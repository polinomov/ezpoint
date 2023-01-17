
#include <stdlib.h>
#include <memory>
#include "ezpoint.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define KEEPALIVE EMSCRIPTEN_KEEPALIVE
#else
#define KEEPALIVE
#endif
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

        void Render(unsigned int *pBuff, int winW, int winH){
            static unsigned char cnt = 0;
  	        for (int y = 0; y < winH; y++) {
		        for (int x = 0; x < winW; x++) {
                        int dst = x + y * m_canvasW;
                        pBuff[dst]  = cnt;
                }
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

    
    #if 0
    KEEPALIVE
    void RenderInit(int buffW, int buffH)
    {

    }

    KEEPALIVE
    void OnRender(unsigned int *pBuff, int winW, int winH, int buffW, int buffH )
    {
   	    for (int y = 0; y < winH; y++) {
		    for (int x = 0; x < winW; x++) {
                    int dst = x + y * buffW;
                    pBuff[dst]  = 0;
            }
        }
    }

    KEEPALIVE
    void RenderDestoy(){

    }
    #endif
    
} //namespace ezp 
 

