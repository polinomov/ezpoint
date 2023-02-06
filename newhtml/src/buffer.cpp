
#include <stdlib.h>
#include <stdio.h>
#include <memory>
#include <chrono>
#include <limits>
#include <iostream>
#include "ezpoint.h"
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
        std::shared_ptr<float[]> zBuff;
        float *m_pzb;
        int m_canvasW, m_canvasH;

        void Init(int canvasW, int canvasH){
            m_canvasW = canvasW;
            m_canvasH = canvasH;
            m_pzb = new float[canvasW *canvasH];
        }

        void RenderChunk(std::shared_ptr<Scene::Chunk> chunk,unsigned int *pBuff,int sw, int sh){
            std::shared_ptr<Camera> cam = Camera::Get();
            float *pP = cam->GetPos();
            float *pD = cam->GetDir();
            float *pU = cam->GetUp();
            float *pR = cam->GetRight();
            std::cout<<"ppp="<<pP[0]<<","<<pP[1]<<","<<pP[2]<<std::endl;
            float *pV = chunk->pVert;
            for( int i = 0; i<chunk->numVerts; i++){
                float dx = pV[0] - pP[0];
                float dy = pV[1] - pP[1];
                float dz = pV[2] - pP[2];
                float xf = dx*pR[0] + dy*pR[1] + dx*pR[2];
                float yf = dx*pU[0] + dy*pU[1] + dx*pU[2];
                float zf = dx*pD[0] + dy*pD[1] + dx*pD[2];
                int x = (int) xf + sw/2;
                int y = (int) yf + sh/2;
                unsigned int *pCol = (unsigned int*)(pV+3);
                if(( x>0) && ( x<sw) && ( y>0) && (y<sh)){
                    int dst = x + y * m_canvasW;
                    float zb = m_pzb[dst];
                    if(zf < zb){
                        pBuff[dst] = pCol[0];//0x00FFFF00;
                        m_pzb[dst] = zf;
                    }
                }
                pV+=4;
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

            static int val = 0;
            
   	        for (int y = 0; y < winH; y++) {
		        for (int x = 0; x < winW; x++) {
                        int dst = x + y * m_canvasW;
                        pBuff[dst]  = 0;
                        m_pzb[dst]  = std::numeric_limits<float>::max();;
                }
            }
            
            RenderRect(pBuff, 0, winH-10, val, winH, 0xFF);        
            val++;
            if(val>winW) val = 0;

            auto chunks = Scene::Get()->GetChunks();
  
            for( int i = 0; i<chunks.size(); i++)
            {
                 RenderChunk(chunks[i],pBuff,winW,winH);
            }
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
 

