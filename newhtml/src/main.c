#include <stdlib.h>
//#include <iostream>
//#include <chrono>
//#include <thread>
#include <SDL2\SDL.h>
#include <emscripten.h>




//extern "C" {

	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* surface;
	int gCanvasW = 2048, gCanvasH = 1024;
	int gWinW = 2048, gWinH = 2048;
	
	void PollEvents() {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_WINDOWEVENT) {
				printf("WIN-event\n");
			}
			SDL_Keycode key = event.key.keysym.sym;
			if (event.key.type == SDL_KEYDOWN) {
				printf("KEY_DOWN\n");
			}
		}
	}

	void MainLoop() {
		static unsigned char cnt = 0;
		PollEvents();
		if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);

		Uint8* pixels = (Uint8*)surface->pixels;

		Uint32* pDst = (Uint32*)pixels; 
		for (int y = 0; y < gCanvasH; y++) {
			for (int x = 0; x < gCanvasW; x++) {
				pDst[x + y * gCanvasW] = 0x909000|cnt;
				if ((x >= gWinW) || (y == gWinH)) {
					pDst[x + y * gCanvasW] = 0xFFFFFF;
				}
			}
		}

		if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);

		SDL_Texture* screenTexture = SDL_CreateTextureFromSurface(renderer, surface);

		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
		SDL_RenderPresent(renderer);

		SDL_DestroyTexture(screenTexture);
		//semscripten_run_script("OnDraw()");
		cnt++;
	}

	void InitSDL() {
		printf("init sdl\n");
		SDL_Init(SDL_INIT_VIDEO);
		SDL_CreateWindowAndRenderer(gCanvasW, gCanvasH, 0, &window, &renderer);
		surface = SDL_CreateRGBSurface(0, gCanvasW, gCanvasH,32, 0, 0, 0, 0);
		emscripten_run_script("OnStart()");
		//emscripten_set_canvas_size(256, 256);
		emscripten_set_main_loop(MainLoop, 0, 1);
	}

	int CallCFunc(int w, int h) 
	{
		printf("HelloC w=%d h=%d\n",w,h);
		gWinW = w-18;
		gWinH = h-50;
		//SDL_DestroyRenderer(renderer);
		//SDL_DestroyWindow(window);
		//gCanvasW = w - 20;
		//gCanvasH = h - 20;
		//SDL_CreateWindowAndRenderer(gCanvasW, gCanvasH, 0, &window, &renderer);
		//surface = SDL_CreateRGBSurface(0, gCanvasW, gCanvasH, 32, 0, 0, 0, 0);
		return 0;
	}

	int  main() {
		printf("MAIN\n");
		InitSDL();
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 0;
	}
//}