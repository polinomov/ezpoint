
#include <stdlib.h>
#include <stdio.h>
#include <memory>
#include <chrono>
#include <limits>
#include <iostream>
#include "ezpoint.h"
#include "rdhelpers.h"
#include "wasm_simd128.h"
#include "xmmintrin.h"
/*
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define KEEPALIVE EMSCRIPTEN_KEEPALIVE
#else
#define KEEPALIVE
#endif
*/
#define MPROJ(_m0,_m1,_m2,_m3,_vx,_vy,_vz,_res) \
                     {\
                    __m128 r0 = _mm_mul_ps(_vx, _m0);\
                    __m128 r1 = _mm_mul_ps(_vy, _m1);\
                    __m128 r2 = _mm_mul_ps(_vz, _m2);\
                    __m128 sum_01 = _mm_add_ps(r0, r1);\
                    __m128 sum_23 = _mm_add_ps(r2, _m3);\
                    __m128 sum =   _mm_add_ps(sum_01, sum_23);\
                     _mm_storeu_ps((float*)_res, sum);\
                     }


namespace ezp 
{
	#define RND_POINTS 1
	#define PROJ_POINTS 2

    struct FrBuff{
        uint32_t old_val;
        uint32_t new_val;
        uint32_t addr;
    };

    struct RendererImpl : public Renderer
    {
        uint32_t *m_frbuff;
        uint32_t *m_colorBuff;
        uint64_t *m_auxBuff;
        int m_canvasW, m_canvasH;
        float m_atanRatio;
        int m_budget;
        int m_visPoints = 0;
        int m_pointSize;
        int m_colorMode;
        uint32_t m_bkcolor;
        uint32_t m_palGray[256];
        uint32_t m_palHMap[256];
        uint32_t m_palClass[256];
        int m_sceneSize;
        float m_zmax;
        float m_zmin;
        float m_zprd;
        float _A[4],_B[4],_C[4],_D[4];
        FPoint4 CNorm[6];
        bool m_showfr;
        bool m_dbgFlf;
        uint32_t m_postTime;
        uint32_t m_totalRdPoints;
        uint32_t m_renderAll;
        bool m_hasDbClick;
        uint32_t m_bdClickX;
        uint32_t m_bdClickY;
        char m_outsrt[1024];
 
        void Init(int canvasW, int canvasH){
            m_canvasW = canvasW;
            m_canvasH = canvasH;
            m_auxBuff = new uint64_t[canvasW *canvasH];
            m_frbuff  = new uint32_t[canvasW *canvasH + 128];
            uint64_t addr = (uint64_t)m_frbuff;
            addr = (addr +128) &(~127);
            m_frbuff =(uint32_t*)addr;//new float[pBf[i]*4];
            m_colorBuff = new uint32_t[canvasW *canvasH];
            m_showfr = false;
            m_sceneSize = 1;
            m_dbgFlf = false;
            m_zmax = m_zmin = 0.0f;
            m_hasDbClick = false;
            ezp::UI *pUI = ezp::UI::Get();
            m_bkcolor = pUI->GetBkColor();
            m_pointSize = pUI->GetPtSize();
            m_budget = pUI->GetBudget();
            SetFov(pUI->GetFov());
            for( int i = 0; i<256; i++){
                m_palGray[i] = i | (i<<8) | (i<<16) | (i<<24);
                if( i<128){
                    float t = (float)i/128.0f;
                    uint8_t r = (uint8_t)(t*255.0f);
                    uint8_t g =  (uint8_t)((1.0f-t) * 255.0f);
                    m_palHMap[i] = (g<<8) | (r<<16);
                }else{
                    float t = (float)(i-128)/128.0f;
                    uint8_t g = (uint8_t)(t*255.0f);
                    uint8_t b = (uint8_t)((1.0f-t) * 255.0f);
                    m_palHMap[i] = b | (g<<8) ;
                }
                m_palClass[i] = 0xFFFFFFFF;
            }
            m_palClass[0] = 0xFFFFFFFF;
            m_palClass[1] = 0xFF00;
            m_palClass[2] = 0xFFFF00;
            m_palClass[3] = 0x00FF10;
            m_palClass[4] = 0x40FF40;
            m_palClass[5] = 0x90FF90;
            m_palClass[6] = 0xFF0000;
            m_palClass[9] = 0xFF;
        }
#if 0
        void TestSimd(){
             float va[4] = {1.0f,2.0f,3.0f,4.0f};
             float vb[4] = {1.0f,2.0f,3.0f,4.0f};
             float out[4];
             __m128 x =  _mm_loadu_ps((const float*)va);
             __m128 y =  _mm_loadu_ps((const float*)vb);
             //__m128 sum_01 = _mm_hadd_ps(m0, m1);
             __m128 result = _mm_mul_ps(x, y);
             _mm_storeu_ps((float*)out, result);
             std::cout<<"XXXXXXX"<<out[0]<<","<<out[1]<<","<<out[2]<<","<<out[3]<<std::endl;
             float s = 7.0f;
             const __m128 scalar = _mm_set1_ps(s);
            __m128 r = _mm_mul_ps(x, scalar);
             _mm_storeu_ps((float*)out, r);
            std::cout<<"XXXXXXX"<<out[0]<<","<<out[1]<<","<<out[2]<<","<<out[3]<<std::endl;
        }
#endif
        void ShowFrameRate(bool val){
            m_showfr = val;
        }

        float GetAtanRatio(){
            return m_atanRatio;
        }

        void  SetFov(int val){
            m_atanRatio = 1.0f/tan(0.5f* (float)val * 3.1415f/180.0f);
        }

        void  SetBudget(float val){
            m_budget = val;
        }

		void  SetPointSize(float val){
             m_pointSize = (int)val;
        }

        void  SetBkColor( uint32_t val){
            //std::cout<<"SetBkColor:"<<val<<std::endl;
            m_bkcolor = val;
        }

        void SetDebugParam(int val){
            m_dbgFlf  = !m_dbgFlf;
        }

        void SetColorMode( uint32_t val) {
            m_colorMode = val;
        }
        void SetRenderAll( uint32_t val){
            m_renderAll = val;
        }
        void SetDbClickEvent( uint32_t x, uint32_t y){
            m_bdClickX = x;
            m_bdClickY = y;
            m_hasDbClick = true;
//            std::cout<<"DbClick: "<<x<<" "<<y<<std::endl;
        }

        void MoveCameraOnDbClick(){
            uint32_t addr = m_bdClickX + m_canvasW *m_bdClickY;
            uint64_t ptPtr = m_auxBuff[addr];
            if(ptPtr != -1){
                FPoint4 pos,piv;
                Camera *pCam = Camera::Get();
                pCam->GetPivot(piv.x,piv.y,piv.z);
                pCam->GetPos(pos.x,pos.y,pos.z);
                FPoint4 *pT  = (FPoint4*)ptPtr;
                float dx = pT->x - piv.x;
                float dy = pT->y - piv.y;
                float dz = pT->z - piv.z;
                pCam->SetPos(pos.x + dx,pos.y + dy,pos.z + dz);
                pCam->SetPivot(pT->x,pT->y,pT->z);
                UI::Get()->SetRenderEvent(2);
                //printf("pr= %f,%f,%f,%xd\n",pT->x,pT->y,pT->z,pT->col);
               // printf("piv= %f,%f,%f\n",pP[0],pP[1],pP[2]);
            }
            else{
                printf("****\n");
            }
 
        }

        void BuildProjMatrix(int sw, int sh,float atanRatio ){
            Camera *pCam = Camera::Get();
            float pP[3],pD[3],pU[3],pR[3];
            pCam->GetPos(pP[0],pP[1],pP[2]);
            pCam->GetDir(pD[0],pD[1],pD[2]);
            pCam->GetUp(pU[0],pU[1],pU[2]);
            pCam->GetRight(pR[0],pR[1],pR[2]);
            float pixSize = 0.5f * (float)std::min(sw,sh);
            float prd = atanRatio * pixSize;
            _A[0]  = pR[0]*prd; _A[1] = pU[0]*prd; _A[2] = pD[0]; _A[3] = 0.0f;
            _B[0]  = pR[1]*prd; _B[1] = pU[1]*prd; _B[2] = pD[1]; _B[3] = 0.0f;
            _C[0]  = pR[2]*prd; _C[1] = pU[2]*prd; _C[2] = pD[2]; _C[3] = 0.0f;
            _D[0] =  -(pP[0]*pR[0] + pP[1]*pR[1] + pP[2]*pR[2])*prd;
            _D[1] =  -(pP[0]*pU[0] + pP[1]*pU[1] + pP[2]*pU[2])*prd;
            _D[2] =  -(pP[0]*pD[0] + pP[1]*pD[1] + pP[2]*pD[2]);
            _D[3] =  0.0f;

            // build frustum normals
            float ang = atan(1.0f/atanRatio);
            float b = cos(ang); float a = sin(ang);
            CNorm[0].x = pU[0]*b - pD[0]*a;            
            CNorm[0].y = pU[1]*b - pD[1]*a;
            CNorm[0].z = pU[2]*b - pD[2]*a;
            CNorm[1].x = -pU[0]*b - pD[0]*a;            
            CNorm[1].y = -pU[1]*b - pD[1]*a;
            CNorm[1].z = -pU[2]*b - pD[2]*a;

            float rt = (float)sh/(float)sw;
            ang = atan(1.0f/(atanRatio*rt));
            b = cos(ang);  a = sin(ang);
            CNorm[2].x = pR[0]*b - pD[0]*a;            
            CNorm[2].y = pR[1]*b - pD[1]*a;
            CNorm[2].z = pR[2]*b - pD[2]*a;
            CNorm[3].x = -pR[0]*b - pD[0]*a;            
            CNorm[3].y = -pR[1]*b - pD[1]*a;
            CNorm[3].z = -pR[2]*b - pD[2]*a;

            CNorm[4].x = -pD[0];
            CNorm[4].y = -pD[1];
            CNorm[4].z = -pD[2];
        }

        //template <unsigned int M>
        static void RenderChunks(int sw, int sh,RendererImpl *rp){
            FrBuff tbuff[16];
            Camera *pCam = Camera::Get();
            float pP[3],pD[3];
            pCam->GetPos(pP[0],pP[1],pP[2]);
            pCam->GetDir(pD[0],pD[1],pD[2]);

            uint32_t tbi = 0;
            const std::vector<std::shared_ptr<Chunk>>& chunks = Scene::Get()->GetChunks();
            rp->m_totalRdPoints = 0;
            float sumAux = 0.0f;
            float distMax = std::numeric_limits<float>::min();
            float distMin = std::numeric_limits<float>::max();
            float zTr = rp->m_sceneSize/sqrt((float)chunks.size());
            for( int m = 0; m<chunks.size()-1; m++) {
                std::shared_ptr<Chunk> pCh= chunks[m];
               // __m128 xss = _mm_set1_ps(pCh->cx);
               // __m128 yss = _mm_set1_ps(pCh->cy);
               // __m128 zss = _mm_set1_ps(pCh->cz);
               // MPROJ(a,b,c,d, xss,yss,zss,&chunks[m]->proj);
                chunks[m]->numToRender = chunks[m]->numVerts-1;
                FPoint4 bdd[8];
                bdd[0].x = pCh->xMin; bdd[0].y = pCh->yMin; bdd[0].z = pCh->zMin;
                bdd[1].x = pCh->xMax; bdd[1].y = pCh->yMin; bdd[1].z = pCh->zMin;
                bdd[2].x = pCh->xMin; bdd[2].y = pCh->yMax; bdd[2].z = pCh->zMin;
                bdd[3].x = pCh->xMax; bdd[3].y = pCh->yMax; bdd[3].z = pCh->zMin;
                bdd[4].x = pCh->xMin; bdd[4].y = pCh->yMin; bdd[4].z = pCh->zMax;
                bdd[5].x = pCh->xMax; bdd[5].y = pCh->yMin; bdd[5].z = pCh->zMax;
                bdd[6].x = pCh->xMin; bdd[6].y = pCh->yMax; bdd[6].z = pCh->zMax;
                bdd[7].x = pCh->xMax; bdd[7].y = pCh->yMax; bdd[7].z = pCh->zMax;
                for( int n = 0; n<5; n++){ //normals
                    int cntb = 0;
                    for( int k =0; k<8; k++){// bddox
                        float v = (bdd[k].x - pP[0]) * rp->CNorm[n].x + (bdd[k].y - pP[1]) * rp->CNorm[n].y + (bdd[k].z - pP[2]) * rp->CNorm[n].z; 
                        if(v>0.f){
                            cntb++;
                        }else{
                            break;
                        }
                    }
                    if(cntb==8){
                       pCh->numToRender = 0;
                       break; 
                    }
                }
                rp->m_totalRdPoints+=pCh->numToRender;
                if(pCh->numToRender>0){
                    float dist = (pCh->cx-pP[0])*pD[0] + (pCh->cy-pP[1])*pD[1] + (pCh->cz-pP[2])*pD[2];
                    if(dist <0.0f) dist = -dist;
                    dist = std::max(dist,zTr);
                    distMax = std::max(distMax ,dist);
                    distMin = std::min(distMin ,dist);
                    pCh->reduction = dist;
                   // sumAux += pCh->aux *(float)pCh->numToRender;
                }
            }
            if((distMax - distMin)<zTr)  distMax = distMin + zTr;
  
            //LOD
            //if(rp->m_totalRdPoints > rp->m_budget){
            if(rp->m_renderAll == 0){
                float delta = distMax - distMin;
                float begin = 1.0f;
                float end =  pow(distMin/distMax,4);
                float sSum = 0.0f;
                for( int m = 0; m<chunks.size()-1; m++) {
                    std::shared_ptr<Chunk> pCh = chunks[m];
                    if(pCh->numToRender<=1) continue;
                    float wt =  (pCh->reduction - distMin)/delta; 
                    pCh->reduction = (1.0f-wt) * begin + wt * end;
                    sSum +=  pCh->reduction * (float)pCh->numToRender;
                }
                //std::cout<<"zTr="<<zTr<<" min="<<distMin<<" max="<<distMax<<" sum="<<sSum<<std::endl;

                float pprd = (float)rp->m_budget/sSum;
                for( int m = 0; m<chunks.size()-1; m++) {
                    std::shared_ptr<Chunk> pCh = chunks[m];
                    if(pCh->numToRender<=0) continue;
                    float nr = (float)pCh->numToRender * pprd * pCh->reduction;
                    if( (uint32_t)nr <= pCh->numToRender)  pCh->numToRender = (uint32_t)nr;
                }
            }
            
             // load proj matrix
            const __m128 a =  _mm_loadu_ps((const float*)rp->_A);
            const __m128 b =  _mm_loadu_ps((const float*)rp->_B);
            const __m128 c =  _mm_loadu_ps((const float*)rp->_C);
            const __m128 d =  _mm_loadu_ps((const float*)rp->_D);
            float one  = 1.0f;
            const __m128 wss = _mm_set1_ps(one);
            float swf = (float)sw *0.5f;
            float shf = (float)sh *0.5f;
            rp->m_visPoints= 0;
            uint32_t *pDstB = rp->m_frbuff;
            uint32_t *pColBuff = rp->m_colorBuff;
            uint64_t *pPoint = rp->m_auxBuff;

            for( int m = 0; m<chunks.size()-1; m++) {
                if(chunks[m]->numToRender<=1) continue;
                  FPoint4 *pV4 = (FPoint4*)chunks[m]->pVert;
                __m128 xss = _mm_set1_ps(pV4->x);
                __m128 yss = _mm_set1_ps(pV4->y);
                __m128 zss = _mm_set1_ps(pV4->z);
                 const float zmf =  rp->m_zmin;
                const float zprd = rp->m_zprd;
                const int canvas_w = rp->m_canvasW;
                int addr_max = rp->m_canvasW*rp->m_canvasH;
                //int numV = chunks[m]->numVerts;
                int numV = chunks[m]->numToRender;
               // uint32_t color,color_old = 0;
                rp->m_visPoints+=numV-1;
                for( int i = 0; i<numV-1; i++){  
                    float res[4];
                    MPROJ(a,b,c,d, xss,yss,zss,res);
                    xss = _mm_set1_ps(pV4[1].x); 
                    yss = _mm_set1_ps(pV4[1].y);
                    zss = _mm_set1_ps(pV4[1].z);
                    if((res[2]>0.001f)){
                        int x = (int) (swf + res[0]/res[2] );
                        int y = (int) (shf + res[1]/res[2] );                             
                        int dst = x + y * canvas_w;
                        //int dst = (x&msk) + ((y&msk) * canvas_w);
                        uint32_t *pAddr = pDstB + dst;
                        uint32_t *pColAddr = pColBuff + dst;
                        uint32_t colOld  = pColAddr[0];
                        if(( x>0) && ( x<sw) && ( y>0) && (y<sh)){
                           // uint32_t color= pV4->col;
                           // uint32_t color_old= pColAddr[0] ;
                            uint32_t zb = pAddr[0];
                            uint32_t zi = (uint32_t)(res[2]);
                            uint32_t pr = (zi<zb)? -1:0;
                            uint32_t pr1 = ~pr;
                            *pAddr = (zi & pr) + (zb & pr1);
                            *pColAddr =  ( pV4->col & pr) + (colOld & pr1);
                            if(rp->m_hasDbClick){
                                uint64_t oldPt = pPoint[dst];
                                pPoint[dst]  = (((uint64_t)pV4) & pr) + (oldPt & pr1);
                            }
                        }
                    }
                    pV4++;
                } // verts in chunk
            }//chunks.size()
        }//RenderChunks

        void RenderRect(unsigned int *pBuff, int left, int top, int right, int bot,unsigned int col){
            for (int y = top; y < bot; y++) {
		        for (int x = left; x < right; x++) {
                        int dst = x + y * m_canvasW;
                        pBuff[dst]  = col;
                }
            }
        }
#if 0
        void RenderHelpers(unsigned int *pBuff,int sw, int sh){
            float swf = (float)sw *0.5f;
            float shf = (float)sh *0.5f;
            auto chunks = Scene::Get()->GetChunks();
            for( int i =0; i<chunks.size(); i++){
                if(chunks[i]->proj.z>0.0f ){
                    int x = (int) (swf + chunks[i]->proj.x/chunks[i]->proj.z);
                    int y = (int) (shf + chunks[i]->proj.y/chunks[i]->proj.z);  
                    if(( x>0) && ( x<sw) && ( y>0) && (y<sh) ){
                        int dst = x + y * m_canvasW;
                        pBuff[dst]  = 0xFFFF00FF;
                    }
                }
            }
        }
#endif

        void Render(unsigned int *pBuff, int winW, int winH,int evnum){
            static int val = 0;
            static FPoint4 pt4;
            uint64_t addrd = (uint64_t)(&pt4);
            uint32_t *p32 = (uint32_t*)m_frbuff;
   	        for (int y = 0; y < winH; y++) {
                for (int x = 0; x < winW; x++) {
                    int dst = x + y * m_canvasW;
                    p32[dst] = 0xFFFFFFFF;
                    m_colorBuff[dst] = 0;//m_bkcolor;
                }
            }
            memset(m_auxBuff,0xFF,m_canvasW *m_canvasH*sizeof(uint64_t));
            m_palGray[0] = m_bkcolor;
           
            BuildProjMatrix(winW,winH,  m_atanRatio);
            m_sceneSize = Scene::Get()->GetSize();
            Scene::Get()->GetZMax(m_zmin,m_zmax);
            if(m_zmax<=m_zmin){
               // PostProcess<0>(pBuff, winW, winH,1);
                return;
            }
            uint32_t rndMs = 0;
            {
                auto before = std::chrono::system_clock::now();
                RenderChunks(winW,winH,this);
                auto after = std::chrono::system_clock::now();
                rndMs = std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
            }
            if(m_hasDbClick){
                if((m_bdClickX<m_canvasW )&&(m_bdClickY<m_canvasH)){
                    MoveCameraOnDbClick();
                }
                m_hasDbClick = false;
            }
            // postprocess
            {
                auto before = std::chrono::system_clock::now();
                switch(m_pointSize){
                    case 1:  XERR1<1>(pBuff, winW, winH); break;
                    case 2:  XERR1<2>(pBuff, winW, winH); break;
                    case 3:  XERR1<3>(pBuff, winW, winH); break;
                    case 4:  XERR1<4>(pBuff, winW, winH); break;       
                    case 5:  XERR1<5>(pBuff, winW, winH); break;
                    case 6:  XERR1<6>(pBuff, winW, winH); break;
                    case 7:  XERR1<7>(pBuff, winW, winH); break;
                }
               
                auto after = std::chrono::system_clock::now();
                m_postTime = std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
            }
            /*
            switch(m_colorMode){
                case UI::UICOLOR_INTENS:
                    PostProcess<0>(pBuff, winW, winH,m_pointSize);
                break;
                case UI::UICOLOR_CLASS: 
                    PostProcess<1>(pBuff, winW, winH,m_pointSize);
                break;
                case UI::UICOLOR_MIX: 
                    PostProcess<2>(pBuff, winW, winH,m_pointSize);
                break;
                default: 
                    PostProcess<0>(pBuff, winW, winH,m_pointSize);
                break;
            }
            */
            // helpers
            if(0)
            {
                const std::vector<std::shared_ptr<Chunk>>& chunks = Scene::Get()->GetChunks();
                float swf = (float)winW *0.5f;
                float shf = (float)winH *0.5f;
                for( int m = 0; m<chunks.size(); m++) {
                    FPoint4 *res = &chunks[m]->proj;
                    if((res->z>0.001f)){
                        int x = (int) (swf + res->x/res->z );
                        int y = (int) (shf + res->y/res->z ); 
                        RenderPoint(pBuff,m_canvasW, m_canvasH, x, y );
                    }                            
                }
            }


            if(m_showfr){
                DbgShowFrameRate(m_visPoints,rndMs);  
            }         
        }
        
#if 1  
        template <unsigned int M>
        void PostProcess(unsigned int *pBuff, int winW, int winH, int N){
            uint32_t pTmp[N*N];
            uint64_t pV[N*N];
 
            for (int y = 0; y < winH-N; y++) {

                for(int i =0; i<N;  i++){
                    for(int j =0; j<N; j++){
                        int addr = 0 +i + (y+j)*m_canvasW;
                        int k = j + i*N;
                        pTmp[k]  = m_frbuff[addr];
                        pV[k]  = m_auxBuff[addr];
                    }
                }
                int column = N-1; 
		        for (int x = 0; x < winW-N; x++) {
                    int dst = x + y * m_canvasW;
                    uint32_t z_min = m_frbuff[dst];
                    uint64_t v_min = m_auxBuff[dst];
                    
                    for(int j =0; j<N; j++){
                        int addr = x + (y+j)*m_canvasW;
                        int k = j + column*N;
                        pTmp[k]  = m_frbuff[addr];
                        pV[k]  = m_auxBuff[addr];
                    }
                    column++;
                    if( column==N) column = 0;
                   
                    for(int i =0; i<N;  i++){
                        for(int j =0; j<N; j++){
                            int k = j + i*N;
                            if(pTmp[k]<z_min) {
                                z_min = pTmp[k];
                                v_min = pV[k];
                            }
                        }
                    }
                   uint8_t coli = z_min & 0xFF;
                    if( z_min == 0xFFFFFFFF){
                        pBuff[dst] = m_bkcolor;
                    }else{
                        FPoint4 *pV4  = (FPoint4*)v_min;
                        if(m_frbuff[dst] != 0xFFFFFFFF){
                           //pV4  = (FPoint4*) m_auxBuff[dst];
                        }
 
                        if(M==0){ //:UICOLOR_INTENS
                            uint8_t col8 = pV4->col & 0x000000FF;
                            pBuff[dst] = m_palGray[col8];
                        }
                        if(M==1){ //UI::UICOLOR_CLASS
                            uint8_t col8 = (pV4->col & 0x0000FF00)>>8;
                            pBuff[dst] = m_palClass[col8];
                        }
                        if(M==2){ //UI::UICOLOR_MIX
                            float coli = (float(pV4->col & 0x000000FF))/255.9f;
                            uint8_t col8 = (pV4->col & 0x0000FF00)>>8;
                            uint32_t cc = m_palClass[col8];
                            float rr = coli*(float)(cc&0xFF);
                            float gg = coli*(float)((cc&0xFF00)>>8);
                            float bb = coli*(float)((cc&0xFF0000)>>16);
                            pBuff[dst] = (uint8_t)rr | (((uint8_t)gg)<<8) | (((uint8_t)bb)<<16);
                        }
                       // pBuff[dst] = m_palGray[coli];
                    }
                }
            }
        }
#endif
        template <unsigned int M>
        void XERR1(unsigned int *pBuff, int winW, int winH){
            //static const int  nsz1 = 3;
            uint32_t minZ[16];
            uint32_t minC[16];
            for(int m = 0; m<M; m++) minZ[m] = -1;
            for (int y = 0; y < winH-M; y++) {
                for(int xt = 0; xt<M; xt++){
                    for(int yt = 0; yt<M; yt++){
                        int ad = xt + yt * m_canvasW;
                        if(m_frbuff[ad] <minZ[xt]) minZ[xt] = m_frbuff[ad];
                    }
                }
                int cnt = 0;
                for (int x = 0,n = 0; x < winW-M; x++, n++){
                    int dst = x + y * m_canvasW;
                    uint32_t zm = -1;
                    uint32_t min_col = m_colorBuff[dst];
                    int ad = x + M-1 + y * m_canvasW;
                    for(int yt = y; yt<M+y; yt++){
                        if(m_frbuff[ad] < zm) {
                            zm = m_frbuff[ad];
                            min_col  =  m_colorBuff[ad];
                        }
                        ad+=m_canvasW;
                    }
                    minZ[cnt] = zm;
                    minC[cnt] =  min_col;
                    uint32_t bz = minZ[0];
                    uint32_t bc = minC[0];
                    for(int m = 0; m<M; m++){
                        if(minZ[m]<bz){
                            bz = minZ[m];
                            bc = minC[m];
                        }
                    }
                    cnt++;
                    if(cnt>=M) cnt = 0;
                    uint8_t cndx = bc & 0xFF;
                    pBuff[dst] =  (bz==0xFFFFFFFF)?  m_bkcolor: m_palGray[cndx];
                    //uint8_t col8 = (bc & 0x0000FF00)>>8;
                    //pBuff[dst] =  (bz==0xFFFFFFFF)?  m_bkcolor: m_palClass[col8];
                }
            } 
        }

        void XERR(unsigned int *pBuff, int winW, int winH){
            static const int nsz = 3;
            for (int y = 0; y < winH; y++) {
                for (int x = 0,n = 0; x < winW; x++, n++){
                    int dst = x + y * m_canvasW;
                    uint8_t cndx = m_colorBuff[dst] & 0xFF;
                    pBuff[dst] = m_palGray[cndx];
                }
            } 
        }

        void DbgShowFrameRate( int num_rnd,uint32_t rndMs){
            static unsigned char cnt = 0,nn =0;
            static std::chrono::time_point<std::chrono::system_clock> prev;
            if(cnt==10){
                auto curr = std::chrono::system_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(curr - prev);
                float fps = 10000.0f/(float)elapsed.count();
                snprintf(m_outsrt,1024,"points=%d tm=%d post=%d,fps=",num_rnd,rndMs,m_postTime);
                UI::Get()->PrintMessage(m_outsrt, (int)fps);
                cnt = 0; 
                nn++;
                prev = std::chrono::system_clock::now();
            }
            cnt++;
        }

        void Destroy(){}

    };

    Renderer* Renderer::Get()
    {
        static RendererImpl TheRendererImpl;
        return &TheRendererImpl;
    }    
    
} //namespace ezp 
 

