#include <stdlib.h>
//#include <iostream>
//#include <chrono>
//#include <thread>
#include "ezpoint.h"
#include <SDL2\SDL.h>
#include <iostream>
#include <thread>
#include <emscripten.h>

// extern void OnRender(unsigned int *pBuff, int winW, int winH, int buffW, int buffH );

extern "C" {

	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* surface;
	int gCanvasW = 2048, gCanvasH = 2048;
	int gWinW = 2048, gWinH = 2048;
	int gRenderEvent = 1;
	std::function<void (const char *msg)> gWriteLine;
	
	void PollEvents() {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_WINDOWEVENT) {
				printf("WIN-event--\n");
			}
			SDL_Keycode key = event.key.keysym.sym;
			if (event.key.type == SDL_KEYDOWN) {
				printf("KEY_DOWN\n");
			}
		}
	}

	void ResetCanvasSize(int w, int h) {
		static char strh[1024];
		sprintf(strh, "%s'%d'", "document.getElementById('canvas').height =", h);
		emscripten_run_script(strh);
		static char strw[1024];
		sprintf(strw, "%s'%d'", "document.getElementById('canvas').width =", w);
		emscripten_run_script(strw);
	}

	static void OutLine(const char *txt){
		static char strw[1024];
		sprintf(strw, "%s'%s'", "document.getElementById('GFG').innerHTML=", txt);
		emscripten_run_script(strw);
	}

	void MainLoop() {
		static unsigned char cnt = 0;
		SDL_Rect srcRect, dstRect;

		//PollEvents();
		ResetCanvasSize(gWinW, gWinH);
		{
		  static char ttt[128];
		  sprintf(ttt,"BB %d",cnt);
		  //OutLine(ttt);
		}
		if(gRenderEvent)
		{
			if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);

			Uint8* pixels = (Uint8*)surface->pixels;
			Uint32* pDst = (Uint32*)pixels; 
		
			ezp::Renderer::Get()->Render(pDst, gWinW, gWinH);

			if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
			SDL_Texture* screenTexture = SDL_CreateTextureFromSurface(renderer, surface);
			SDL_RenderClear(renderer);
			srcRect.x = 0;
			srcRect.y = 0;
			srcRect.w = gWinW;
			srcRect.h = gWinH;
			dstRect.x = 0;
			dstRect.y = gCanvasH - gWinH;
			dstRect.w = gWinW;
			dstRect.h = gWinH;
			SDL_RenderCopy(renderer, screenTexture, &srcRect, &dstRect);
			SDL_RenderPresent(renderer);
			SDL_DestroyTexture(screenTexture);
			gRenderEvent = 0;
			cnt++;
		}
	}

	void InitSDL() {
		int sw = 0, sh = 0;
		emscripten_get_screen_size(&sw, &sh);
		printf("--- init sdl --- %d %d\n",sw,sh);
		SDL_Init(SDL_INIT_VIDEO|SDL_WINDOW_RESIZABLE);
		SDL_CreateWindowAndRenderer(gCanvasW, gCanvasH, 0, &window, &renderer);
		surface = SDL_CreateRGBSurface(0, gCanvasW, gCanvasH,32, 0, 0, 0, 0);
		gWriteLine =  OutLine;
		ezp::Renderer::Get()->Init(gCanvasW, gCanvasH);
		emscripten_run_script("OnStart()");
		emscripten_set_main_loop(MainLoop, 0, 1);
	}
 
    // Resize call from JS
	int CallCFunc(int w, int h) 
	{
		//printf("HelloC w=%d h=%d\n",w,h);
		gWinW = w > gCanvasW ? gCanvasW : w-15;
		gWinH = h-100 > gCanvasH ? gCanvasH : h - 75;
		ResetCanvasSize(gWinW, gWinH);
		gRenderEvent = 1;
		return 0;
	}

	int FileBinData(void* pData, int sz) 
	{
		static char ts[1024];
		unsigned char* p8 = (unsigned char*)pData;		
		float* pF = (float*)pData;
		int numFloats = sz/sizeof(float);
		sprintf(ts,"Done Reading123 sz= %d %f",numFloats,pF[numFloats-2]);
		OutLine(ts);
		ezp::Scene *pSc = ezp::Scene::Get();
		pSc->SetFileImage(pData, sz, 0);
#if 0
        int numVerts = numFloats/4;
		float x_min = pF[0];
		for(int k = 0;k<numVerts;k+=4){
			//std::cout <<"here---------"<<std::endl;
			float x_min = std::min(x_min,pF[k]);
			sprintf(ts,"processing %d from %d",k,numVerts);
			if(( k& 0xFFFF) ==0 ){
				OutLine(ts);
				emscripten_sleep(1);
			}
			//break;
		}
#endif
		return 0;
	}


    void foo()
	{
      std::cout<<"OnThread"<<std::endl;
	}

    int CallCFunc2(int w, int h) 
	{
		std::thread::id this_id = std::this_thread::get_id();
		std::cout << "thread " << this_id << " sleeping...\n";
		std::cout <<"here..."<<std::endl;
		return 0;
	}

	int  main() {
		printf("--MAIN--\n");
		InitSDL();
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 0;
	}
}

namespace ezp 
{
	struct UIImpl : public UI{

		void PrintMessage( const char *pMsg){
			//printf("MESSAGE\n");
			OutLine(pMsg);
		}
		void PrintMessage( const char *pMsg,int val){
			static char strw[1024];
			sprintf(strw, "%s'%s %d'", "document.getElementById('GFG').innerHTML=", pMsg,val);
			emscripten_run_script(strw);
		}

		void SetRenderEvent(){
			gRenderEvent = 1;
		}
	};
	
	UI* UI::Get(){
		static UIImpl theUIImpl;
		return &theUIImpl;
	}

}// namespace ezp