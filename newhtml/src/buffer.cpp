
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
    struct RendererImpl : public Renderer
    {
        float *m_pzb;
        unsigned int *m_pb;
        int m_canvasW, m_canvasH;
        float m_atanRatio;
        float _A[4],_B[4],_C[4],_D[4];
 
        void Init(int canvasW, int canvasH){
            m_canvasW = canvasW;
            m_canvasH = canvasH;
            m_pzb = new float[canvasW *canvasH];
            m_pb  = new unsigned int[canvasW *canvasH];
            m_atanRatio = 3.0f;
        }

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

        float GetAtanRatio(){
            return m_atanRatio;
        }

		void  SetAtanRatio(float val){
            m_atanRatio = val;
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

        template <unsigned int N>
        static void RenderChunk(FPoint4 *pVerts,int sw, int sh,int numV,void *pD,RendererImpl *rp){
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
            for( int i = 0; i<numV-1; i++){
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
                    if(res[2]>0.001f){
                        int x = (int) (swf + res[0]/res[2]);
                        int y = (int) (shf + res[1]/res[2]);  
                            if(( x>0) && ( x<sw) && ( y>0) && (y<sh) ){
                            int dst = x + y * rp->m_canvasW;
                            float zb = rp->m_pzb[dst];
                            if(res[2] < zb){
                                rp->m_pb[dst]  = pV4->col;
                                rp->m_pzb[dst] = res[2];
                            }
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
                        chunks[i]->reduction = 1.0f/res[2];
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
                        pBuff[dst]  = 0xFF;
                }
            }
        }

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
 
        void Render(unsigned int *pBuff, int winW, int winH,int evnum){
/**/
            static int val = 0;
            
   	        for (int y = 0; y < winH; y++) {
		        for (int x = 0; x < winW; x++) {
                        int dst = x + y * m_canvasW;
                        m_pb[dst]  = 0x40;
                        m_pzb[dst]  = std::numeric_limits<float>::max();;
                }
            }
            
            
            //RenderRect(m_pb, 0, winH-10, val, winH, 0xFF);        
            //val++;
            //if(val>winW) val = 0;
            
            BuildProjMatrix(winW,winH,  m_atanRatio);

            auto chunks = Scene::Get()->GetChunks();
            FPoint4* chp  = Scene::Get()->GetChunkPos();
            FPoint4* chpa = Scene::Get()->GetChunkAuxPos();
            RenderChunk<PROJ_POINTS>(chp,winW,winH,chunks.size(),chpa,this);

            float prd_tot = 0.0f;
            uint32_t tv = 0;
            uint32_t budget = 3*1000*1000;
           // std::cout<<"XAXA "<<chunks.size()<<std::endl;
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

           // std::cout<<"prd_tot="<<prd_tot<<" tv="<<tv<<std::endl;

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
                    RenderChunk<RND_POINTS>((FPoint4*)chunks[i]->pVert,winW,winH,numToRender,NULL,this);
                }
                //RenderChunk<RND_POINTS>((FPoint4*)chunks[i]->pVert,winW,winH,chunks[i]->numVerts,NULL,this);

            }
            std::cout<<"num_skip="<<num_skip<<" rnd="<<num_rnd<<" ch="<<chunks.size()<<std::endl;
            
            //RenderHelpers(m_pb, winW, winH);
            // postprocess
            float none = std::numeric_limits<float>::max();
            for (int y = 1; y < winH-1; y++) {
		        for (int x = 1; x < winW-1; x++) {
                    int dst = x + y * m_canvasW;
                    int dsti = dst,aa;
                    float zm=m_pzb[dst],zz;
                    //if(zm!=none){
                    //    pBuff[dst] = m_pb[dst];
                    //    continue;
                    //}
                    /*
                    int aa = dst-1;
                    float zz=m_pzb[aa];
                    if(zz<zm){
                        zm = zz;
                        dsti = aa;
                    }
                    */
                    aa =dst + 1;
                    zz=m_pzb[aa];
                    if(zz<zm){
                        zm = zz;
                        dsti = aa;
                    }

 
                    aa =dst + m_canvasW;
                    zz=m_pzb[aa];
                    if(zz<zm){
                        zm = zz;
                        dsti = aa;
                    }
                    aa =dst + m_canvasW+1;
                    zz=m_pzb[aa];
                    if(zz<zm){
                        zm = zz;
                        dsti = aa;
                    }
                    pBuff[dst] = m_pb[dsti];
                }
            }
                    
 /**/
            ShowFrameRate();           
        }

        void ShowFrameRate(){
            static unsigned char cnt = 0,nn =0;
            static std::chrono::time_point<std::chrono::system_clock> prev;
            if(cnt==10){
                auto curr = std::chrono::system_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(curr - prev);
                float fps = 10000.0f/(float)elapsed.count();
                UI::Get()->PrintMessage("fps=", (int)fps);
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
 

