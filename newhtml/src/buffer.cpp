
#include <stdlib.h>
#include <stdio.h>
#include <memory>
#include <chrono>
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
        }

        void RenderChunk(std::shared_ptr<Scene::Chunk> chunk,unsigned int *pBuff,int sw, int sh){
            float *pV = chunk->pVert;
            for( int i = 0; i<chunk->numVerts; i++){
                int x = (int) pV[0];
                int y = (int) pV[1];
                if(( x>0) && ( x<sw) && ( y>0) && (y<sh)){
                    int dst = x + y * m_canvasW;
                    pBuff[dst] = 0xFFFFFFFF;
                }
                pV+=4;
            }
        }

        void RenderRect(unsigned int *pBuff, int left, int top, int winW, int winH,unsigned int col){
            for (int y = top; y < winH; y++) {
		        for (int x = left; x < winW; x++) {
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
                }
            }
            
            RenderRect(pBuff, 30+val, 30, 100, 150, 0xFF);
            
            val++;
            if(val>30) val = 0;

            auto chunks = Scene::Get()->GetChunks();
           // UI::Get()->PrintMessage("Render");

            for( int i = 0; i<chunks.size(); i++)
            {
                 RenderChunk(chunks[i],pBuff,winW,winH);
            }
            ShowFrameRate();           
        }

        void ShowFrameRate(){
            static char msg[1024];
            static unsigned char cnt = 0,nn =0;
            static std::chrono::time_point<std::chrono::system_clock> prev;
            if(cnt==10){
                auto curr = std::chrono::system_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(curr - prev);
                //sprintf(msg,"%s:%lld","t=",elapsed.count());
                UI::Get()->PrintMessage("t=", (int)elapsed.count());
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
 

