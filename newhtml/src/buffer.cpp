
#include <stdlib.h>
#include <memory>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define KEEPALIVE EMSCRIPTEN_KEEPALIVE
#else
#define KEEPALIVE
#endif

struct RenderBuffer
{
    std::shared_ptr<float[]> zBuff;
    //std::vector<int> v;
    float *m_pzb;
    size_t m_sz;
    RenderBuffer() : m_sz(0), m_pzb(NULL){}  
};


KEEPALIVE
void RenderInit(int buffW, int buffH){

}

KEEPALIVE
void OnRender(unsigned int *pBuff, int winW, int winH, int buffW, int buffH ){
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
 

