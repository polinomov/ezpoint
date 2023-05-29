
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
	
    struct RendererImpl : public Renderer
    {
        float *m_pzb;
        unsigned int *m_pb;
        int m_canvasW, m_canvasH;
        float _A[4],_B[4],_C[4],_D[4];

        void Init(int canvasW, int canvasH){
            m_canvasW = canvasW;
            m_canvasH = canvasH;
            m_pzb = new float[canvasW *canvasH];
            m_pb  = new unsigned int[canvasW *canvasH];
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

        void RenderChunk(std::shared_ptr<Chunk> chunk,int sw, int sh){
            //std::cout<<"RENDER:"<<chunk->numVerts<<std::endl;
            int skip = 1;
            const __m128 a =  _mm_loadu_ps((const float*)_A);
            const __m128 b =  _mm_loadu_ps((const float*)_B);
            const __m128 c =  _mm_loadu_ps((const float*)_C);
            const __m128 d =  _mm_loadu_ps((const float*)_D);
            float one  = 1.0f;
            const __m128 wss = _mm_set1_ps(one);
            float swf = (float)sw *0.5f;
            float shf = (float)sh *0.5f;
            float *pV = chunk->pVert;
            __m128 xss = _mm_set1_ps(pV[0]);
            __m128 yss = _mm_set1_ps(pV[1]);
            __m128 zss = _mm_set1_ps(pV[2]);
            for( int i = 0; i<chunk->numVerts-1; i+=skip){
                __m128 r0 = _mm_mul_ps(xss, a);
                __m128 r1 = _mm_mul_ps(yss, b);
                __m128 r2 = _mm_mul_ps(zss, c);
                __m128 sum_01 = _mm_add_ps(r0, r1);
                __m128 sum_23 = _mm_add_ps(r2, d);
                __m128 sum =   _mm_add_ps(sum_01, sum_23);
                float res[4];
                _mm_storeu_ps((float*)res, sum);
                xss = _mm_set1_ps(pV[4]);
                yss = _mm_set1_ps(pV[5]);
                zss = _mm_set1_ps(pV[6]);
                if(res[2]>0.001f){
                    int x = (int) (swf + res[0]/res[2]);
                    int y = (int) (shf + res[1]/res[2]);                 
                    unsigned char *pCol = (unsigned char*)(pV+3);
                    if(( x>0) && ( x<sw) && ( y>0) && (y<sh) ){
                        int dst = x + y * m_canvasW;
                        float zb = m_pzb[dst];
                        if(res[2] < zb){
                            m_pb[dst] = pCol[3];
                            m_pzb[dst] = res[2];
                        }
                    }
                }
                pV+=4*skip;
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
 
        void Render(unsigned int *pBuff, int winW, int winH){
/**/
            static int val = 0;
            
   	        for (int y = 0; y < winH; y++) {
		        for (int x = 0; x < winW; x++) {
                        int dst = x + y * m_canvasW;
                        m_pb[dst]  = 0x00;
                        m_pzb[dst]  = std::numeric_limits<float>::max();;
                }
            }
            
            //RenderRect(m_pb, 0, winH-10, val, winH, 0xFF);        
            //val++;
            //if(val>winW) val = 0;
            
            BuildProjMatrix(winW,winH, 3.0f );
            auto chunks = Scene::Get()->GetChunks();
            for( int i = 0; i<chunks.size(); i++) {
                 RenderChunk(chunks[i],winW,winH);
            }

            for (int y = 0; y < winH; y++) {
		        for (int x = 0; x < winW; x++) {
                        int dst = x + y * m_canvasW;
                        uint8_t c = m_pb[dst];
                       // uint8_t g = (c<50)? 50-c: c;
                        pBuff[dst] = c|(c<<8)|(c<<16);
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
 

