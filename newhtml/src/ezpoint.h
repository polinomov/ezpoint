#ifndef _EZPOINT_H
#define _EZPOINT_H

#include <cstddef>
#include <vector>
#include <memory>
#include <functional>
#include "chunk.h"

namespace ezp 
{
  struct UI
  {
    enum ColorMode{
      UICOLOR_INTENS =0,
      UICOLOR_CLASS,
      UICOLOR_RGB,
      UICOLOR_HMAP,
      UICOLOR_MIX
    };

    virtual void PrintMessage( const char *pMsg) = 0;
    virtual void PrintMessage( const char *pMsg,int val) = 0;
    virtual void PrintMessage(const std::string &msg) =0;
    virtual void SetRenderEvent(int num)  = 0;
    virtual int GetFov() = 0;
    virtual int GetBkColor() = 0;
    virtual int GetPtSize() = 0;
    virtual int GetBudget() = 0;
    virtual void SetColorMode(ColorMode md) = 0;
    virtual ColorMode GetColorMode() = 0;
    virtual void OnUIEvent(const char *pEvent, int val)  = 0;
    static UI *Get();
  };

  struct Camera
  {
    virtual void GetPos( float &x, float &y, float &z) = 0;
    virtual void GetDir( float &x, float &y, float &z) = 0;
    virtual void GetUp( float &x, float &y, float &z) = 0;
    virtual void GetRight( float &x, float &y, float &z) = 0;
    virtual void GetPivot( float &x, float &y, float &z)  = 0;
    virtual void ReSet() = 0;
    virtual void SetPos(float *v) = 0;
    virtual void SetPos(float x, float y, float  z) = 0;
    virtual void SetDir(float *v) = 0;
    virtual void SetUp(float *v) = 0;
    virtual void SetWorldUpAxis(float x, float y, float  z) = 0;
    virtual void SetPivot(float x, float y, float  z)  = 0;
    virtual void RotRight( float val) = 0;
    virtual void RotLeft( float val) = 0;
    virtual void RotUp( float val) = 0;
    virtual void RotDown( float val) = 0;
    virtual void ZoomIn( float val) = 0;
    virtual void ZoomOut( float val) = 0;
    virtual void MoveLeftOrRight( float val) = 0;
    virtual void MoveUpOrDown( float val) = 0;
    virtual void Project( float x, float y, float z,float &d, float &u, float &r) = 0;
    static Camera  *Get();
  };

  struct Scene
  {
    virtual uint32_t AllocVerts( uint32_t num) = 0;
    virtual uint32_t GetTotVerts() = 0;
    virtual uint32_t GetNumMemBanks() = 0;
    virtual const FPoint4 *GetVerts(uint32_t n) = 0;
    virtual void Clear() = 0;
    virtual uint32_t GetNumVerts(uint32_t n) = 0;
    virtual void processVertData(std::function<void( )> isReady) = 0;
    virtual void GenerateSample() = 0;
    virtual float GetSize() = 0;
    virtual const std::vector<Chunk*>& GetChunks() = 0;
    virtual void GetZMax(float &zmin, float &zmax) = 0;
    virtual void SetCamera() = 0;
    virtual void SetCameraOrto() = 0;
    virtual void onTick() = 0;
    static Scene *Get();
  };

  struct Renderer
  {
    virtual void Init(int canvasW, int canvasH) = 0;
    virtual void Render(unsigned int *pBuff, int winW, int winH,int evnum) = 0;
    virtual float GetAtanRatio() = 0;
    virtual void SetFov(int val) = 0;
    virtual void SetBudget(float val) = 0;
    virtual void SetPointSize(float val) = 0;
    virtual void SetBkColor( uint32_t val)= 0;
    virtual void ShowFrameRate(bool val) = 0;
    virtual void SetDebugParam(int val) = 0;
    virtual void SetColorMode( uint32_t val) = 0;
    virtual void SetRenderAll( uint32_t val) = 0;
    virtual void SetDbClickEvent( uint32_t x, uint32_t y) = 0;
    virtual void SetRuler(int val) =0;
    virtual void MouseMoveEvent( uint32_t x, uint32_t y) = 0;
    virtual void MouseClickEvent() = 0;
    static Renderer* Get();
  };

  struct RenderHelper{
    virtual void Init()= 0;
    virtual void Reset() = 0;
    virtual  uint32_t getClosePoint(uint32_t mx, uint32_t my,uint64_t *pt,int cW,int cH) = 0;
    virtual void Render(unsigned int *pBuff, int cW,int cH,int winW,int winH) = 0;
    virtual void MouseMove(uint32_t mx, uint32_t my,uint64_t *pt,int cW,int cH) = 0;
    virtual void OnSelectPoint() = 0;
    static RenderHelper* Get();
  };
}

#endif

 