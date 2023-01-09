#include <stdlib.h>
//#include <iostream>
//#include <chrono>
//#include <thread>
#include <SDL2\SDL.h>
#include <emscripten.h>

extern void OnRender(unsigned int *pBuff, int winW, int winH, int buffW, int buffH );

extern "C" {

	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* surface;
	int gCanvasW = 2048, gCanvasH = 2048;
	int gWinW = 2048, gWinH = 2048;
	
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

	void MainLoop() {
		static unsigned char cnt = 0;
		SDL_Rect srcRect, dstRect;

		PollEvents();
		ResetCanvasSize(gWinW, gWinH);
		if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);

		Uint8* pixels = (Uint8*)surface->pixels;

		Uint32* pDst = (Uint32*)pixels; 
		OnRender(pDst, gWinW, gWinH, gCanvasW, gCanvasH);
#if 0
		int shiftY = 0;// gCanvasH - gWinH;
		if (shiftY < 0) shiftY = 0;
		for (int y = shiftY; y < gWinH; y++) {
			for (int x = 0; x < gWinW; x++) {
				int dst = x + y * gCanvasW;
				int sy = y - shiftY;
				int sx = x;
				pDst[dst] = ((x < 64) && (y < 64)) ? 0xFFFFFF : cnt;
				if ( (x % 64) ==0) {
					pDst[dst] = 0xFFFFFF;
				}
				else if ((sy % 64) == 0) {
					pDst[dst] = 0xFFFFFF;
				}
				else {
					//pDst[dst] = cnt ;// (256.0f / (float)gWinHy)* sy;
					if ( (sy < 10)||(sy>gWinH-10)||(sx<10)||(sx>gWinW-20)) {
						pDst[dst] = 0x808080;
					}
				}
			}
		}
#endif
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
		cnt++;
	}

	void InitSDL() {
		printf("--- init sdl ---\n");
		SDL_Init(SDL_INIT_VIDEO|SDL_WINDOW_RESIZABLE);
		SDL_CreateWindowAndRenderer(gCanvasW, gCanvasH, 0, &window, &renderer);
		surface = SDL_CreateRGBSurface(0, gCanvasW, gCanvasH,32, 0, 0, 0, 0);
		emscripten_run_script("OnStart()");
		emscripten_set_main_loop(MainLoop, 0, 1);
	}

	int CallCFunc(int w, int h) 
	{
		//printf("HelloC w=%d h=%d\n",w,h);
		gWinW = w > gCanvasW ? gCanvasW : w;
		gWinH = h-100 > gCanvasH ? gCanvasH : h -100;
		return 0;
	}

	int FileBinData(void* pData, int sz) 
	{
		printf("-FileBinData-%d\n", sz);
		unsigned char* p8 = (unsigned char*)pData;
		printf("%c %c %c %c\n", p8[0], p8[1], p8[2], p8[3]);
		float* pF = (float*)pData;
		printf("%f %f %f %f \n", pF[0], pF[1], pF[2], pF[3]);
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