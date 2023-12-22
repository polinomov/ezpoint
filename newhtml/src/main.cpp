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
    SDL_Renderer* m_renderer;
    SDL_Surface* surface;
    SDL_Texture* screenTexture;
    SDL_Texture* m_screenTexture;
    int gCanvasW = 2048, gCanvasH = 1024;
    int gWinW = 1290, gWinH = 454;
    int gRenderEvent = 1;
    int gAlwaysRender = 0;
    int gNeedResize = 1;
    int gMenuShift = 50;
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

    void ForceResize(){
        if(gNeedResize){
            emscripten_run_script("forceResize()");
        }
    }

    static void OutLine(const char *txt){
        static char strw[1024];
        sprintf(strw, "%s'%s'", "document.getElementById('GFG').innerHTML=", txt);
        emscripten_run_script(strw);
    }
 
    void MainLoop() {
         if((gRenderEvent>0) || ( gAlwaysRender==1)){
            SDL_Rect srcRect, dstRect;
            unsigned char* pixels;
            int pitch;
            ForceResize();
            gNeedResize = 0;
            srcRect.x = 0;
            srcRect.y = 0;
            srcRect.w = gWinW;
            srcRect.h = gWinH;
            dstRect.x = 0;
            dstRect.y = gCanvasH - gWinH - gMenuShift;
            dstRect.w = gWinW;
            dstRect.h = gWinH;
            SDL_LockTexture( m_screenTexture, NULL, (void**)&pixels, &pitch );
            ezp::Renderer::Get()->Render((uint32_t*)pixels, gWinW, gWinH,gRenderEvent);
            SDL_UnlockTexture( m_screenTexture );
            SDL_RenderCopy(m_renderer, m_screenTexture, &srcRect, &dstRect);
            SDL_RenderPresent(m_renderer);
            gRenderEvent--;
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

        SDL_CreateWindowAndRenderer(gCanvasW, gCanvasH, 0, &window, &m_renderer);
        //surface = SDL_CreateRGBSurface(0, gCanvasW, gCanvasH,32, 0, 0, 0, 0);
        m_screenTexture = SDL_CreateTexture(m_renderer,
                                            SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING,
                                            gCanvasW, gCanvasH);
        gWriteLine =  OutLine;
        ezp::Renderer::Get()->Init(gCanvasW, gCanvasH);
        emscripten_run_script("OnStart()");
        emscripten_set_main_loop(MainLoop, 0, 1);
    }
 
    // Resize call from JS
    int CallCFunc(int w, int h) 
    {
       // printf("HelloC-- w=%d h=%d\n",w,h);
        //gWinW = w > gCanvasW ? gCanvasW : w-15;
        //gWinH = h-100 > gCanvasH ? gCanvasH : h - 75;
        gWinW = w > gCanvasW ? gCanvasW : w;
        gWinH = h > gCanvasH ? gCanvasH : h-gMenuShift;
        //printf("HelloC-- w=%d h=%d\n",gWinW,gWinH);
        gRenderEvent = 2;
        return 0;
    }

    int FileBinDataJS(void* pData, int sz, int type) // JS call
    {
        static char ts[1024];
        unsigned char* p8 = (unsigned char*)pData;		
        float* pF = (float*)pData;
        int numFloats = sz/sizeof(float);
        sprintf(ts,"Done Reading sz= %d ",numFloats);
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
        char *pRet =  emscripten_run_script_string("GetUIString()");
       // std::cout<<"UI:"<<pRet<<" val="<<value<<std::endl;
        ezp::UI::Get()->OnUIEvent(pRet,value);
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
        OutLine("MAIN");
        InitSDL();
        SDL_DestroyRenderer(m_renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }
}

namespace ezp 
{
    struct UIImpl : public UI{
     std::unordered_map<std::string, uint32_t> m_strToId;

        UIImpl(){
            m_strToId["fovVal"] = UIFOV;
            m_strToId["ptSize"] = UIPTSIZE;
            m_strToId["budVal"] = UIBUDGET;
            m_strToId["bkgcol"] = UIBKGCOLOR;
            m_strToId["colrgbId"] = UICOLOR_RGB;
            m_strToId["colintId"] = UICOLOR_INTENS;
            m_strToId["colhtmId"] = UICOLOR_HMAP;
            m_strToId["colclassId"] = UICOLOR_CLASS;
            m_strToId["colmix"] = UICOLOR_MIX;
        }
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

        void GetValue( const char *pUiId){
           char *pRet =  emscripten_run_script_string("GetUIValue('xaxa')");
           std::cout<<"pRet="<<pRet<<std::endl;
        }

        int GetFov(){
            return emscripten_run_script_int("GetFovValue()");
        }

        int GetBkColor(){
           return emscripten_run_script_int("GetBkColorValue()");
        }

        int GetPtSize(){
            return emscripten_run_script_int("GetPtSizeValue()");
        }
        int GetBudget(){
            return 100000*emscripten_run_script_int("GetBudgetValue()");
        }

        void OnUIEvent(const char *pEvent, int val){

            if(pEvent==NULL) return;
            auto u_iter = m_strToId.find(pEvent);
            if( u_iter == m_strToId.end()){
                return;
            }
            //std::cout<<"OnUIEvent "<<u_iter->second<<std::endl;
            switch(u_iter->second){
                case UIFOV:
                    ezp::Renderer::Get()->SetFov(val);
                break;
                case UIPTSIZE:
                    ezp::Renderer::Get()->SetPointSize(val);
                break;
                case UIBUDGET:
                    ezp::Renderer::Get()->SetBudget(val*100000);
                break;
                case UIBKGCOLOR:
                    ezp::Renderer::Get()->SetBkColor(val);
                break;
                case UICOLOR_INTENS:
                    ezp::Renderer::Get()->SetColorMode(UICOLOR_INTENS);
                break;
                case UICOLOR_CLASS:
                    ezp::Renderer::Get()->SetColorMode(UICOLOR_CLASS);
                break;
                case UICOLOR_RGB:
                    ezp::Renderer::Get()->SetColorMode(UICOLOR_RGB);
                break;
                case UICOLOR_HMAP:
                    ezp::Renderer::Get()->SetColorMode(UICOLOR_HMAP);
                break;
                case UICOLOR_MIX:
                    ezp::Renderer::Get()->SetColorMode(UICOLOR_MIX);
                break;
                default:
                    std::cout<<"UNKNOWN"<<std::endl;
                break;
            }
            gRenderEvent = 2;	
        }

        void SetElementState( const std::string &id,bool state){
            const std::string qt = "\"";  
            std::string cmd = "document.getElementById("+qt+id+qt+").disabled=";
            cmd += (state) ? "false":"true";
            emscripten_run_script(cmd.c_str());
        }

        void SetColorModeState(uint32_t flg, bool state){
            std::cout<<"SetColorModeState"<<std::endl;
            if(flg & COLOR_MODEL_RGB){
                //SetElementState("colrgbId", state);
            }
            if(flg & COLOR_INTENS){
                //SetElementState("colintId", state);
            }
            if(flg & COLOR_HMAP){
                //SetElementState("colhtmId", state);
            }
            if(flg & COLOR_CLASS){
                //SetElementState("colclassId", state);
            }
        }
    };
    
    UI* UI::Get(){
        static UIImpl theUIImpl;
        return &theUIImpl;
    }

}// namespace ezp