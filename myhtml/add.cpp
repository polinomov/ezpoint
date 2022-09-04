#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <thread>
#ifndef VS_BUILD
 #include <emscripten.h>
#endif


#include <SDL.h>
#include <SDL_render.h>

static float* pSrc;
static float* pDst;
static float pM[16];
int cnt = 0;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Surface *surface;

/*
void drawRandomPixels() {
    if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);

    Uint8 * pixels = (Uint8 *)screen->pixels;
    
    for (int i=0; i < 1048576; i++) {
        char randomByte = rand() % 255;
        pixels[i] = randomByte;
    }

    if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);

    SDL_Flip(screen);
}
*/

static float copyBuff(float* pSrc, float* pDst, float* matr, int start, int len)
{
	for (int i = start; i < len + start; i += 4)
	{
		pDst[i + 0] = pSrc[i] * matr[0] + pSrc[i + 1] * matr[1] + pSrc[i + 1] * matr[2] +pSrc[i + 1] * matr[3];
		pDst[i + 1] = pSrc[i] * matr[4] + pSrc[i + 1] * matr[5] + pSrc[i + 1] * matr[6] +pSrc[i + 1] * matr[7];
		pDst[i + 2] = pSrc[i] * matr[8] + pSrc[i + 1] * matr[9] + pSrc[i + 1] * matr[10] +pSrc[i + 1] * matr[11];
		pDst[i + 3] = pSrc[i] * matr[12] + pSrc[i + 1] * matr[13] + pSrc[i + 1] * matr[14] + pSrc[i + 1] * matr[15];
		cnt++;
	}
	return 0.0f;
}

static void threadFunc(int sz, int start)
{
	//std::cout << "I'm a thread! " << sz << std::endl;
	copyBuff(pSrc, pDst, pM, sz, start);
}


#ifndef VS_BUILD
EMSCRIPTEN_KEEPALIVE
#endif 
static float TestCall()
{
	    SDL_Init(SDL_INIT_VIDEO);
		SDL_CreateWindowAndRenderer(512, 512, 0, &window, &renderer);
        //surface = SDL_CreateRGBSurface(0, 512, 512, 32, 0, 0, 0, 0);

	static int n_call = 0;
	int sz = 1024 * 1024 * 16 * 4;
	if (n_call == 0)
	{
		std::cout << "SomeCPPCall " << n_call << std::endl;
		pSrc = (float*)malloc(sz * sizeof(float));
		pDst = (float*)malloc(sz * sizeof(float));
		for (int i = 0; i < 16; i++) pM[i] = (float)i;
		for (int i = 0; i < sz; i++) pSrc[i] = (i&1)?  (float)(i&127) : -(float)(i & 127);
	}
	n_call++;
	float ret1 = 0.0f,ret2=0.0f;
	
	
#if 0	
	{
		int part = sz / 4;
		auto start = std::chrono::steady_clock::now();
		std::thread t1(threadFunc, 0, sz / 4);
		std::thread t2(threadFunc, part, sz / 4);
		std::thread t3(threadFunc, part * 2, sz / 4);
		std::thread t4(threadFunc, part * 3, sz / 4);
		t1.join();
		t2.join();
		t3.join();
		t4.join();
		auto end = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;
		std::cout << "elapsed time-4: " << elapsed_seconds.count() << "s\n";
		ret2 = (float)elapsed_seconds.count();
	}
#endif

	{
		auto start = std::chrono::steady_clock::now();
		copyBuff(pSrc, pDst, pM, 0, sz);
		auto end = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;
		std::cout << "numpoints="<<sz/4<<" elapsed time:" << elapsed_seconds.count() << "s\n";
		ret1 = (float)elapsed_seconds.count();
		ret2 = ret1;
	}

	int sum = 0;
	for (int i = 0; i < sz; i++) 
	{
		int nn = (int)pDst[i];
		if (i & 1) {
			sum += nn;
		}
		else {
			sum -= nn;
		}
	}
	std::cout << "sum=" << sum << " cnt="<<cnt<<std::endl;
	return (n_call&1) ? ret1:ret2;
}
 
extern "C" {
#ifndef VS_BUILD
	EMSCRIPTEN_KEEPALIVE
#endif 
		float Add() {
		return TestCall();
	}

#ifndef VS_BUILD
	EMSCRIPTEN_KEEPALIVE

	int main()
	{
		printf("MAIN\n");
		//TestCall();

		return 0;
	}

#endif
}