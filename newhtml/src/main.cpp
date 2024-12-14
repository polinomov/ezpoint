#include <stdlib.h>
#include "ezpoint.h"
//#include <SDL\SDL_ttf.h>
#include <SDL2\SDL.h>

#include <iostream>
#include <thread>
#include <emscripten.h>
#include <stdio.h>
#include <string.h>
#include "readers\readers.h"


extern "C" {
  SDL_Window* window;
  SDL_Renderer* m_renderer;
  SDL_Surface* surface;
  SDL_Texture* screenTexture;
  SDL_Texture* m_screenTexture;
  int gCanvasW = 2048*2, gCanvasH = 1024;
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
  /*
  static void OutLine(const char *txt){
    static char strw[1024];
    sprintf(strw, "%s'%s'", "document.getElementById('GFG').innerHTML=", txt);
    emscripten_run_script(strw);
  }
  */
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
      //SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
      SDL_UnlockTexture( m_screenTexture );
      SDL_RenderCopy(m_renderer, m_screenTexture, &srcRect, &dstRect);
      SDL_RenderPresent(m_renderer);
      gRenderEvent--;
    }
    ezp::Scene::Get()->OnTick();
  }

  void InitSDL() {
    int sw = 0, sh = 0;
    emscripten_get_screen_size(&gCanvasW, &gCanvasH);
    gCanvasW = std::min(gCanvasW,2048);
    gCanvasH = std::min(gCanvasH,1024);
    //printf("--- init sdl --- %d %d\n",gCanvasW,gCanvasH);
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
    //gWriteLine =  OutLine;
    ezp::Renderer::Get()->Init(gCanvasW, gCanvasH);
    emscripten_run_script("OnStart()");
    emscripten_set_main_loop(MainLoop, 0, true);
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
    //gWinW= gWinW;
    //gWinH= gWinH;
    return 0;
  }

  int FileBinDataJS(void* pData, int sz, int type) 
  {
    static char ts[1024];
    unsigned char* p8 = (unsigned char*)pData;		
    float* pF = (float*)pData;
    int numFloats = sz/sizeof(float);
     // sprintf(ts,"Done Reading sz= %d ",numFloats);
    //OutLine(ts);
    ezp::Scene *pSc = ezp::Scene::Get();
     // pSc->SetFileImage(pData, sz, type);
#if 0
    int numVerts = numFloats/4;
    float x_min = pF[0];
   for(int k = 0;k<numVerts;k+=4){
      //std::cout <<"here---------"<<std::endl;
      float x_min = std::min(x_min,pF[k]);
      sprintf(ts,"processing %d from %d",k,numVerts);
      if(( k& 0xFFFF) ==0 ){
         // OutLine(ts);
        emscripten_sleep(1);
      }
      //break;
    }
#endif
    //gAlwaysRender = 1;
    return 0;
  }

extern "C" {

  static ezp::PointBuilder* GetFromFileType(uint32_t fType){
    switch(fType){
      case 0:
        return ezp::PointBuilder::GetLasBuilder();
      break;
      case 1:
        return NULL;
      break;
      case 2:
        return ezp::PointBuilder::GetXyzTxtBuilder();
      break;
    }
    return NULL;
  }

  // Call from JS
  //Gets called when all files are loaded. Schedules postprocessing calls.
  int PostProcessDataJS( int fType, int param){
    auto postProcColors = [fType](void){ 
      auto getVerts = [](uint32_t ndx){ 
        return ezp::Scene::Get()->GetVerts(ndx);
      };
      auto getNumInBank = [](uint32_t ndx){ 
        return ezp::Scene::Get()->Get()->GetNumVerts(ndx);
      };
      uint32_t mb =  ezp::Scene::Get()->GetNumMemBanks();
      bool isRgb = (ezp::UI::Get()->GetColorMode() ==  ezp::UI::ColorMode::UICOLOR_RGB);
      ezp::PointBuilder *pPointBuilder = GetFromFileType(fType);
      if(pPointBuilder != NULL){
        pPointBuilder->PostProcessAllColors(mb,isRgb, getVerts, getNumInBank);
      }
      if(mb>0){
        ezp::Scene::Get()->SetCamera();
			  ezp::UI::Get()->SetRenderEvent(2);
      }
    };

    for( int nn = 0; nn<ezp::Scene::Get()->GetNumMemBanks(); nn++){
      auto call = [nn](void){
        ezp::UI::Get()->PrintMessage("Processing ..." + std::to_string(nn));
        if(nn==0){
          ezp::Scene::Get()->ReScale();
        }
        ezp::Scene::Get()->processVertDataInt(nn);
      };
      ezp::Scene::Get()->AddToQueue(call);
    }
    auto postMessage = [](void){
      ezp::UI::Get()->PrintMessage("Processing colors...");
    };
    ezp::Scene::Get()->AddToQueue(postMessage);
    ezp::Scene::Get()->AddToQueue(postProcColors);   
    auto finMessage = [](void){
      ezp::UI::Get()->PrintMessage(ezp::Scene::Get()->GetDesctiption());
    };
    ezp::Scene::Get()->AddToQueue(finMessage);
    return 0;
  }
  
  // Call from JS
  // Consumes file data in chunks
  // Returns the size of the next chunk , 0 if done.
  int LdLasCppJS(void* pData, int action, int fType, int fSize){
    int ret = 0;
    ezp::PointBuilder *pLasBuilder =  GetFromFileType(fType);
    if(pLasBuilder == NULL){
      ezp::UI::Get()->ShowErrorMessage("Unknown file type " + std::to_string(fType));
      return 0;
    }

    auto allocVerts = [](uint32_t num){ 
      uint32_t ndx  = ezp::Scene::Get()->AllocVerts(num);
      return ndx;
    };
    auto getVerts = [](uint32_t ndx){ 
      return ezp::Scene::Get()->GetVerts(ndx);
    };
    auto onError = [](const std::string &msg){
      std::string m = std::string("alert(") + "\"" + msg + "\"" + std::string(")");
      emscripten_run_script(m.c_str());
    };
    auto onInfo = [](const ezp::LasInfo &info){
      if(info.hasRgb){
        ezp::UI::Get()->SetColorMode(ezp::UI::UICOLOR_RGB);
      }else{
        ezp::UI::Get()->SetColorMode(ezp::UI::UICOLOR_INTENS);
      }
      ezp::Scene::Get()->SetDesctiption(info.description);
      uint32_t nextPointsNum = ezp::Scene::Get()->GetTotVerts() + info.numPoints;
      if(nextPointsNum > 250 *1000*1000){
        return 1; 
      }
      return 0;
    };

    if ( action == 0 ){// start loading new file
      std::cout<<"fsize=="<<fSize<< "fType="<<fType<<std::endl;
      pLasBuilder->Reset(fSize);
      pLasBuilder->RegisterCallbacks(allocVerts,getVerts,onError,onInfo);
    }
    ret = (int)pLasBuilder->SetChunkData((action == 0) ? NULL: pData);
    return ret;
  }

  int ClearSceneJS(int param){
    ezp::RenderHelper::Get()->Reset();
    ezp::Scene::Get()->Clear();
    return 0;
  }
}

  int OnDebugCheckBox( int val){ //JS call
    gAlwaysRender = val;
    return 0;
  }

  int OnTestJS(int val){ //JS call
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
    if(val==1){
      ezp::RenderHelper::Get()->OnSelectPoint(); 
    }
    gRenderEvent = 2;
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
    float scaleShift = 0.1f;
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
      gRenderEvent = 1;	
    }
    return 0;
  }

  int CameraMoveDbClickJS(int xval, int yval){
    ezp::Renderer::Get()->SetDbClickEvent( (uint32_t)xval, (uint32_t) yval-gMenuShift);
    gRenderEvent = 1;
    return 0;
  }

  int CameraMoveJS(int xval, int yval){
    ezp::Camera *pCam = ezp::Camera::Get();
    float dist = pCam->GetDistance();
    float atanr = ezp::Renderer::Get()->GetAtanRatio();
    float physcr = (dist/atanr)*(1.0f/256.0f);
    float sx= physcr*(float)xval;
    pCam->MoveLeftOrRight(-sx);
    float sy= physcr*(float)yval;
    pCam->MoveUpOrDown(-sy);
    gRenderEvent = 1;	
    return 0;
  }

  int MouseMoveJS(int xval, int yval){
    ezp::Renderer::Get()->MouseMoveEvent( (uint32_t) xval, (uint32_t)yval-gMenuShift);
    gRenderEvent = 1;
    return 0;
  }

  int MouseClickJS(){
     return 0;
  }

  int  main() {
    InitSDL();
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
  }
}
