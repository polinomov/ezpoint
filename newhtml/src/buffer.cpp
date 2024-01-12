
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
        uint64_t *m_auxBuff;
        int m_canvasW, m_canvasH;
        float m_atanRatio;
        int m_budget;
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
        char m_outsrt[1024];
 
        void Init(int canvasW, int canvasH){
            m_canvasW = canvasW;
            m_canvasH = canvasH;
           // m_frbuff = new uint32_t*[2];
            m_auxBuff = new uint64_t[canvasW *canvasH];
            m_frbuff  = new uint32_t[canvasW *canvasH + 128];
            uint64_t addr = (uint64_t)m_frbuff;
            addr = (addr +128) &(~127);
            m_frbuff =(uint32_t*)addr;//new float[pBf[i]*4];
            m_showfr = false;
            m_sceneSize = 1;
            m_dbgFlf = false;
            m_zmax = m_zmin = 0.0f;
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
            float ang = atan(1.0f/atanRatio);
            float b = cos(ang); float a = sin(ang);
           // std::cout<<atanRatio<<"  "<<atan(a) * 180 / 3.1415f<<std::endl;
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
        }

       
        static void RenderChunks(int sw, int sh,RendererImpl *rp){
            FrBuff tbuff[16];
            Camera *pCam = Camera::Get();
            float pP[3];
            pCam->GetPos(pP[0],pP[1],pP[2]);

            uint32_t tbi = 0;
            // load proj matrix
            const __m128 a =  _mm_loadu_ps((const float*)rp->_A);
            const __m128 b =  _mm_loadu_ps((const float*)rp->_B);
            const __m128 c =  _mm_loadu_ps((const float*)rp->_C);
            const __m128 d =  _mm_loadu_ps((const float*)rp->_D);
            float one  = 1.0f;
            const __m128 wss = _mm_set1_ps(one);
            float swf = (float)sw *0.5f;
            float shf = (float)sh *0.5f;
            const std::vector<std::shared_ptr<Chunk>>& chunks = Scene::Get()->GetChunks();

            for( int m = 0; m<chunks.size(); m++) {
                std::shared_ptr<Chunk> pCh= chunks[m];
                __m128 xss = _mm_set1_ps(pCh->cx);
                __m128 yss = _mm_set1_ps(pCh->cy);
                __m128 zss = _mm_set1_ps(pCh->cz);
                MPROJ(a,b,c,d, xss,yss,zss,&chunks[m]->proj);
                chunks[m]->numToRender = chunks[m]->numVerts-1;
                for( int n = 0; n<4; n++){
                    float v = (pCh->cx - pP[0]) * rp->CNorm[n].x + (pCh->cy - pP[1]) * rp->CNorm[n].y + (pCh->cz - pP[2]) * rp->CNorm[n].z; 
                    if(v>0.0f) {
                        pCh->numToRender = 0;
                        break;
                    }
                }
            }

            for( int m = 0; m<chunks.size()-1; m++) {
                  FPoint4 *pV4 = (FPoint4*)chunks[m]->pVert;
                __m128 xss = _mm_set1_ps(pV4->x);
                __m128 yss = _mm_set1_ps(pV4->y);
                __m128 zss = _mm_set1_ps(pV4->z);
 
                uint32_t *pDstB = (uint32_t*) rp->m_frbuff;
                uint64_t *pPoint= rp->m_auxBuff;
                const float zmf =  rp->m_zmin;
                const float zprd = rp->m_zprd;
                const int canvas_w = rp->m_canvasW;
                int addr_max = rp->m_canvasW*rp->m_canvasH;
                //int numV = chunks[m]->numVerts;
                int numV = chunks[m]->numToRender;
                for( int i = 0; i<numV-1; i++){  
                     float res[4];
                     /*
                     {
                    __m128 r0 = _mm_mul_ps(xss, a);
                    __m128 r1 = _mm_mul_ps(yss, b);
                    __m128 r2 = _mm_mul_ps(zss, c);
                    __m128 sum_01 = _mm_add_ps(r0, r1);
                    __m128 sum_23 = _mm_add_ps(r2, d);
                    __m128 sum =   _mm_add_ps(sum_01, sum_23);
                    _mm_storeu_ps((float*)res, sum);
                     }
                     */
                    MPROJ(a,b,c,d, xss,yss,zss,res);
                    xss = _mm_set1_ps(pV4[1].x); 
                    yss = _mm_set1_ps(pV4[1].y);
                    zss = _mm_set1_ps(pV4[1].z);
                    if((res[2]>0.001f)){
                        int x = (int) (swf + res[0]/res[2] );
                        int y = (int) (shf + res[1]/res[2] );                             
                        int dst = x + y * canvas_w;
                        uint32_t *pAddr = pDstB + dst;
                        if(( x>0) && ( x<sw) && ( y>0) && (y<sh) ){
                            uint32_t zb = pAddr[0];
                            uint32_t zi = (uint32_t)(res[2]);
                            uint64_t oldPt = pPoint[dst];
                            uint64_t pr = (zi<zb)? -1:0;
                            uint64_t pr1 = ~pr;
                            *pAddr = (zi & pr) + (zb & pr1);
                            pPoint[dst]  = (((uint64_t)pV4) & pr) + (oldPt & pr1);
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
            uint32_t *p32 = (uint32_t*)m_frbuff;
   	        for (int y = 0; y < winH; y++) {
                uint32_t *pp = p32 +  y * m_canvasW;
                memset(pp,0xFF,winW*sizeof(uint32_t));
            }

           
            BuildProjMatrix(winW,winH,  m_atanRatio);
            m_sceneSize = Scene::Get()->GetSize();
           // auto chunks = Scene::Get()->GetChunks();
           // FPoint4* chp  = Scene::Get()->GetChunkPos();
            //FPoint4* chpa = Scene::Get()->GetChunkAuxPos();
            Scene::Get()->GetZMax(m_zmin,m_zmax);
            if(m_zmax<=m_zmin){
                PostProcess<0>(pBuff, winW, winH,1);
                return;
            }
  
/*
            int num_rnd = 0;  
            for( int i = 0; i<chunks.size(); i++) {
                int numToRender =  chunks[i]->numVerts;
                if(numToRender < 1){
                    continue;
                }
                if(i&1) continue;
                num_rnd += numToRender;
                RenderChunk((FPoint4*)chunks[i]->pVert,winW,winH,numToRender,NULL,this);
            }
*/
            RenderChunks(winW,winH,this);
            // postprocess
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
            // helpers
            if(1)
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
                DbgShowFrameRate(7777);  
            }         
        }
        
   
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

        void XERR(unsigned int *pBuff, int winW, int winH){
            for (int y = 0; y < winH; y++) {
                for (int x = 0; x < winW; x++) {
                    int dst = x + y * m_canvasW;
                    if( m_frbuff[dst] != 0xFFFFFFFF){
                        FPoint4 *pV4  = (FPoint4*)m_auxBuff[dst];
                        uint8_t coli = pV4->col & 0x000000FF;
                        uint8_t col8 = (pV4->col & 0x0000FF00)>>8;
                        pBuff[dst] = m_palClass[col8];
                    }else{
                        pBuff[dst] = m_bkcolor;
                    }
                }
            } 
        }

        void DbgShowFrameRate( int num_rnd){
            static unsigned char cnt = 0,nn =0;
            static std::chrono::time_point<std::chrono::system_clock> prev;
            if(cnt==10){
                auto curr = std::chrono::system_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(curr - prev);
                float fps = 10000.0f/(float)elapsed.count();
                snprintf(m_outsrt,1024,"points=%d fps=",num_rnd);
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
 

