
#include <stdlib.h>
#include <stdio.h>
#include <memory>
#include <chrono>
#include <limits>
#include <iostream>
#include "ezpoint.h"
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
namespace ezp 
{
	#define RND_POINTS 1
	#define PROJ_POINTS 2

    struct FrBuff{
        uint32_t old_val;
        uint32_t new_val;
        uint32_t addr;
    };

    template <typename T>
    struct RendererImpl : public Renderer
    {
        T *m_frbuff;
        int m_canvasW, m_canvasH;
        float m_atanRatio;
        int m_budget;
        int m_pointSize;
        uint32_t m_bkcolor;
        uint32_t m_palGray[256];
        uint32_t m_palHMap[256];
        uint32_t m_cmask;
        uint32_t m_cshift;
        uint32_t m_colorMode;
        int m_sceneSize;
        float m_zmax;
        float m_zmin;
        float m_zprd;
        float _A[4],_B[4],_C[4],_D[4];
        bool m_showfr;
        bool m_dbgFlf;
        char m_outsrt[1024];
 
        void Init(int canvasW, int canvasH){
            m_canvasW = canvasW;
            m_canvasH = canvasH;
            m_frbuff = new T[canvasW *canvasH + 128];
            uint64_t addr = (uint64_t)m_frbuff;
            addr = (addr +128) &(~127);
            m_frbuff =(T*)addr;
            m_showfr = false;
            m_sceneSize = 1;
            m_dbgFlf = false;
            m_zmax = m_zmin = 0.0f;
            m_cmask = 0x000000FF;
            m_cshift = 0;
            ezp::UI *pUI = ezp::UI::Get();
            m_bkcolor = pUI->GetBkColor();
            m_pointSize = pUI->GetPtSize();
            m_budget = pUI->GetBudget();
            m_colorMode = 0;
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
            }
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
             m_bkcolor = val;
        }

        void SetDebugParam(int val){
            m_dbgFlf  = !m_dbgFlf;
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
        }

        template <uint32_t N, uint32_t D, uint32_t MSK, uint32_t SHF>
        static void RenderChunk(FPoint4 *pVerts,int sw, int sh,int numV,void *pD,RendererImpl *rp){
            FrBuff tbuff[16];
            uint32_t tbi = 0;
            std::vector<std::shared_ptr<Chunk>> chunks = Scene::Get()->GetChunks();
            const __m128 a =  _mm_loadu_ps((const float*)rp->_A);
            const __m128 b =  _mm_loadu_ps((const float*)rp->_B);
            const __m128 c =  _mm_loadu_ps((const float*)rp->_C);
            const __m128 d =  _mm_loadu_ps((const float*)rp->_D);
            float one  = 1.0f;
            const __m128 wss = _mm_set1_ps(one);
            float swf = (float)sw *0.5f;
            float shf = (float)sh *0.5f;
           
            FPoint4 *pV4 = (FPoint4*)pVerts;
            __m128 xss = _mm_set1_ps(pV4->x);
            __m128 yss = _mm_set1_ps(pV4->y);
            __m128 zss = _mm_set1_ps(pV4->z);
 
            T *pDstB = (T*) rp->m_frbuff;
            const float zmf =  rp->m_zmin;
            const float zprd = rp->m_zprd;
            const int canvas_w = rp->m_canvasW;
            const uint32_t cmsk = rp->m_cmask;
            const uint32_t cshift = rp->m_cshift;
            int addr_max = rp->m_canvasW*rp->m_canvasH;
            for( int i = 0; i<numV-1; i++){   
                  uint64_t i64 = (uint64_t)i;
                __m128 r0 = _mm_mul_ps(xss, a);
                __m128 r1 = _mm_mul_ps(yss, b);
                __m128 r2 = _mm_mul_ps(zss, c);
                __m128 sum_01 = _mm_add_ps(r0, r1);
                __m128 sum_23 = _mm_add_ps(r2, d);
                __m128 sum =   _mm_add_ps(sum_01, sum_23);
                float res[4];
                _mm_storeu_ps((float*)res, sum);
                xss = _mm_set1_ps(pV4[1].x);
                yss = _mm_set1_ps(pV4[1].y);
                zss = _mm_set1_ps(pV4[1].z);
                if(N==RND_POINTS)
                {
                    if((res[2]>0.001f)){
                        int x = (int) (swf + res[0]/res[2] );
                        int y = (int) (shf + res[1]/res[2] );                             
                        int dst = x + y * canvas_w;
                        T *pAddr = pDstB + dst;
                        if(( x>0) && ( x<sw) && ( y>0) && (y<sh) ){
                            uint32_t zb = pAddr[0];
                            uint32_t zi = (uint32_t)((res[2] - zmf) * zprd);
                            zi = zi<<8;
                            uint8_t coli = (pV4->col & MSK)>>SHF;
                            //uint8_t coli = (pV4->col & cmsk)>>cshift;
                            *pAddr = (zi < zb) ? (zi | coli ) : zb;
                        }
                    }
                } 
                if(N==PROJ_POINTS){
                    chunks[i]->proj.x = res[0];
                    chunks[i]->proj.y = res[1];
                    chunks[i]->proj.z = res[2];
                    chunks[i]->flg = 0;
                    chunks[i]->numToRender = chunks[i]->numVerts;
                    chunks[i]->reduction = 1.0f;
                    if(res[2]>0.001f){
                        //chunks[i]->reduction = 1.0f/res[2];
                        chunks[i]->reduction = std::max(1.0f - res[2]/rp->m_sceneSize,0.1f);
                        float szsc = 2.0f * res[2]/ rp->m_atanRatio;
                        float myszpix = swf*chunks[i]->sz/szsc;
                        int x = (int) ( res[0]/res[2]);
                        int y = (int) ( res[1]/res[2]);  
                        if(( x>-swf) && ( x<swf) && ( y>-shf) && (y<shf) ){
                            chunks[i]->flg = 0;
                        }else{
                           
                           if(x > (int)myszpix + swf){
                                chunks[i]->flg = CHUNK_FLG_NV; 
                           }
                           if(y > (int)myszpix + shf){
                                chunks[i]->flg = CHUNK_FLG_NV; 
                           }
                           if(x < -myszpix-swf ){
                                chunks[i]->flg = CHUNK_FLG_NV; 
                           }
                           if(y < -myszpix-shf){
                                chunks[i]->flg = CHUNK_FLG_NV; 
                           }
                        } 
                    }
                    else{
                        chunks[i]->flg = CHUNK_FLG_NV;
                    }
                }               
                pV4++;
            }
        }

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
            T *p32 = (T*)m_frbuff;
   	        for (int y = 0; y < winH; y++) {
                T *pp = p32 +  y * m_canvasW;
                memset(pp,0xFF,winW*sizeof(T));
            }

           
            BuildProjMatrix(winW,winH,  m_atanRatio);
            m_sceneSize = Scene::Get()->GetSize();
            auto chunks = Scene::Get()->GetChunks();
            FPoint4* chp  = Scene::Get()->GetChunkPos();
            FPoint4* chpa = Scene::Get()->GetChunkAuxPos();
            Scene::Get()->GetZMax(m_zmin,m_zmax);
            if(m_zmax<=m_zmin){
                PostProcess<1>(pBuff, winW, winH);
                //RenderRect(pBuff, 0, 0, 100, 100, 0xFF00FF00);
                return;
            }
            m_zprd = (255.0f * 255.0f * 255.0f)/(m_zmax - m_zmin);
            RenderChunk<PROJ_POINTS,0,0x000000FF,0>(chp,winW,winH,chunks.size(),chpa,this);
 
            float prd_tot = 0.0f;
            uint32_t tv = 0;
            uint32_t budget = m_budget;
            for( int i = 0; i<chunks.size(); i++) {
                if(chunks[i]->flg & CHUNK_FLG_NV){
                    continue;
                }
                if(chunks[i]->proj.z>0.0001){
                    tv+= chunks[i]->numVerts;
                    prd_tot += (float)chunks[i]->numVerts * chunks[i]->reduction;
                }
            }
            prd_tot = (prd_tot>0.0f)? (float)budget/prd_tot : 1.0f;
            int num_skip = 0,num_rnd = 0;
            for( int i = 0; i<chunks.size(); i++) {
                if(chunks[i]->flg & CHUNK_FLG_NV){
                    num_skip++;
                    continue;
                }
                if(chunks[i]->numToRender<2){
                   continue;
                }
                int numToRender  = prd_tot *chunks[i]->reduction*(float)chunks[i]->numVerts;
                if(numToRender > chunks[i]->numVerts ) numToRender = chunks[i]->numVerts;
                if(numToRender >2){
                    num_rnd+=numToRender;
                    switch(m_colorMode){
                        case 0:
                            RenderChunk<RND_POINTS,0,0x000000FF,0>((FPoint4*)chunks[i]->pVert,winW,winH,numToRender,NULL,this);
                            break;
                        case 1:
                            RenderChunk<RND_POINTS,0,0x0000FF00,8>((FPoint4*)chunks[i]->pVert,winW,winH,numToRender,NULL,this);
                            break;
                        case 2:
                            RenderChunk<RND_POINTS,0,0x00FF0000,16>((FPoint4*)chunks[i]->pVert,winW,winH,numToRender,NULL,this);
                            break;
                        case 3:
                            RenderChunk<RND_POINTS,0,0xFF000000,24>((FPoint4*)chunks[i]->pVert,winW,winH,numToRender,NULL,this);
                            break;
                    }
                }
            }
            // postprocess
            switch(m_pointSize){
                case 1: PostProcess<1>(pBuff, winW, winH); break;
                case 2: PostProcess<2>(pBuff, winW, winH); break;
                case 3: PostProcess<3>(pBuff, winW, winH); break;
                case 4: PostProcess<4>(pBuff, winW, winH); break;
                case 5: PostProcess<5>(pBuff, winW, winH); break;
                case 6: PostProcess<6>(pBuff, winW, winH); break;
                case 7: PostProcess<7>(pBuff, winW, winH); break;
                case 8: PostProcess<8>(pBuff, winW, winH); break;
                case 9: PostProcess<9>(pBuff, winW, winH); break;
                default: PostProcess<9>(pBuff, winW, winH); break;
            }
            if(m_showfr){
                DbgShowFrameRate(num_rnd);  
            }         
        }
        
        template <unsigned int N>
        void PostProcess(unsigned int *pBuff, int winW, int winH){
            T pTmp[N*N];
 
            for (int y = 0; y < winH-N; y++) {

                for(int i =0; i<N;  i++){
                    for(int j =0; j<N; j++){
                        int addr = 0 +i + (y+j)*m_canvasW;
                        pTmp[j + i*N]  = m_frbuff[addr];
                    }
                }
                int column = N-1; 
		        for (int x = 0; x < winW-N; x++) {
                    int dst = x + y * m_canvasW;
                    uint32_t z_min = m_frbuff[dst];
                    
                    for(int j =0; j<N; j++){
                        int addr = x + (y+j)*m_canvasW;
                        pTmp[j + column*N]  = m_frbuff[addr];
                    }
                    column++;
                    if( column==N) column = 0;
                   
                     for(int i =0; i<N;  i++){
                        for(int j =0; j<N; j++){
                            int k = j + i*N;
                            if(pTmp[k]<z_min) {
                                z_min = pTmp[k];
                            }
                        }
                    }
                    uint8_t coli = z_min & 0xFF;
                     if( z_min == 0xFFFFFFFF){
                        pBuff[dst] = m_bkcolor;
                    }else{
                        pBuff[dst] = m_palGray[coli];
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
        static RendererImpl<uint64_t> TheRendererImpl;
        return &TheRendererImpl;
    }    
    
} //namespace ezp 
 

