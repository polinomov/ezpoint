#include <stdlib.h>
//#include <iostream>
//#include <chrono>
//#include <thread>
#include "ezpoint.h"
#include <SDL2\SDL.h>
#include <iostream>
#include <thread>
#include <emscripten.h>


extern "C" {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Surface* surface;
    int gCanvasW = 2048, gCanvasH = 1024;
    //int gCanvasW = 512, gCanvasH = 512;
    int gWinW = 256, gWinH = 256;
    int gRenderEvent = 1;
    int gAlwaysRender = 0;
    std::function<void (const char *msg)> gWriteLine;
    
    void PollEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
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
        if((gRenderEvent>0) || ( gAlwaysRender==1))
        {
            if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);

            Uint8* pixels = (Uint8*)surface->pixels;
            Uint32* pDst = (Uint32*)pixels; 
        
            ezp::Renderer::Get()->Render(pDst, gWinW, gWinH,gRenderEvent);

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
            /*
            SDL_SetRenderTarget(renderer, screenTexture);
            SDL_SetRenderDrawColor(renderer,255, 255, 0,128);
            SDL_RenderDrawLine(renderer, 0 ,0 ,2048, 2048);
            */
            SDL_RenderCopy(renderer, screenTexture, &srcRect, &dstRect);
            SDL_RenderPresent(renderer);
            SDL_DestroyTexture(screenTexture);
            gRenderEvent--;
            cnt++;
        }
    }

    void InitSDL() {
        int sw = 0, sh = 0;
        //emscripten_get_screen_size(&gCanvasW, &gCanvasH);
        //printf("--- init sdl --- %d %d\n",sw,sh);
        //SDL_Init(SDL_INIT_VIDEO|SDL_WINDOW_RESIZABLE);
        SDL_Init(SDL_INIT_VIDEO|SDL_WINDOW_RESIZABLE);
        SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas");
        SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
        SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);

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
        printf("HelloC-- w=%d h=%d\n",w,h);
        gWinW = w > gCanvasW ? gCanvasW : w-15;
        gWinH = h-100 > gCanvasH ? gCanvasH : h - 75;
        ResetCanvasSize(gWinW, gWinH);
        gRenderEvent = 2;
        return 0;
    }

    int FileBinDataJS(void* pData, int sz, int type) // JS call
    {
        static char ts[1024];
        unsigned char* p8 = (unsigned char*)pData;		
        float* pF = (float*)pData;
        int numFloats = sz/sizeof(float);
        sprintf(ts,"Done Reading sz= %d %f",numFloats,pF[numFloats-2]);
        OutLine(ts);
        ezp::Scene *pSc = ezp::Scene::Get();
        pSc->SetFileImage(pData, sz, type);
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
        //gAlwaysRender = 1;
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

    int OnDebugCheckBox( int val){ //JS call
        gAlwaysRender = val;
        return 0;
    }

    int OnTestJS(int val){ //JS call
        std::cout<<"onTest "<<val<<std::endl;
        //ezp::Scene *pRnd = ezp::Renderer::Get();
        if(val== -1){
            if(gAlwaysRender==0){
                gAlwaysRender = 1;
                ezp::Renderer::Get()->ShowFrameRate(true);
            }else{
                gAlwaysRender = 0;
                ezp::Renderer::Get()->ShowFrameRate(false);
            }
        }
        if(val== -2){
            ezp::Renderer::Get()->SetDebugParam(-2);
        }
        return 0;
    }

    int OnUIChangeJS(int el, int value){ 
         if(el==1){ // Fov
            float aratio = 1.0f/tan(0.5f* (float)value * 3.1415f/180.0f);
            ezp::Renderer::Get()->SetAtanRatio(aratio);
            gRenderEvent = 2;	
        }else if(el==2){  // budget
            ezp::Renderer::Get()->SetBudget(value*100000);
            gRenderEvent = 2;
        }else if(el==3){
            ezp::Renderer::Get()->SetPointSize(value);
            gRenderEvent = 2;
        }else if(el==4){ //4
            std::cout<<"SetBkColor-main:"<<value<<std::endl;
            ezp::Renderer::Get()->SetBkColor((uint32_t)value);
            gRenderEvent = 2;
        }
        return 0;
    }

    int CameraRotateJS (int lr, int td, int zoom,int val){
        static const float PI = 3.1415f;
        ezp::Camera *pCam = ezp::Camera::Get();
        ezp::Scene *pSc = ezp::Scene::Get();
        float scaleShift = pSc->GetSize()*0.01f;
        float shift = (float)val * 2.0f * PI/10000.0f;
        if(lr==1){
            pCam->RotRight(shift);
        }else if(lr==-1){
            pCam->RotLeft(shift);
        }
        if(td==1){
            if(zoom ==1 ){
                pCam->ZoomIn(scaleShift);
            }else{
                pCam->RotUp(shift);
            }
        }else if(td==-1){
            if(zoom==1){
                pCam->ZoomOut(scaleShift);
            }else{
                pCam->RotDown(shift);
            }
        }
        if((lr !=0 ) || (td!=0)){
            gRenderEvent = 2;	
        }
        return 0;
    }

    int CameraMoveJS(int xval, int yval){
        ezp::Camera *pCam = ezp::Camera::Get();
        float sx= (float)xval;
        pCam->MoveLeftOrRight(-sx);
        float sy= (float)yval;
        pCam->MoveUpOrDown(-sy);
        gRenderEvent = 2;	
        return 0;
    }

    int  main() {
       // printf("-----MAIN----\n");
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

        void SetRenderEvent(int num){
            gRenderEvent = num;
        }
    };
    
    UI* UI::Get(){
        static UIImpl theUIImpl;
        return &theUIImpl;
    }

}// namespace ezp