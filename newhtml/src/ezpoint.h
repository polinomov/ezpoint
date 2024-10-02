#ifndef _EZPOINT_H
#define _EZPOINT_H

#include <cstddef>
#include <vector>
#include <memory>
#include "chunk.h"

#define COLOR_MODEL_RGB (1<<1)
#define COLOR_INTENS    (1<<2)
#define COLOR_HMAP      (1<<3)
#define COLOR_CLASS     (1<<4)

namespace ezp 
{
  struct UI
  {
    enum{
      UIFOV = 0,
      UIBUDGET,
      UIBKGCOLOR,
      UIPTSIZE,
      UICOLOR_INTENS,
      UICOLOR_CLASS,
      UICOLOR_RGB,
      UICOLOR_HMAP,
      UICOLOR_MIX,
      UIRENDER_ALL,
      UICAM_RESET,
      UICAM_ORTO,
      UIDATA_SAMPLE,
      UIRULER
    };

    virtual void PrintMessage( const char *pMsg) = 0;
    virtual void PrintMessage( const char *pMsg,int val) = 0;
    virtual void SetRenderEvent(int num)  = 0;
    virtual void GetValue( const char *pUiId) = 0;
    virtual int GetFov() = 0;
    virtual int GetBkColor() = 0;
    virtual int GetPtSize() = 0;
    virtual int GetBudget() = 0;
    virtual void SetColorModeState(uint32_t flg, bool state) = 0;
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
    static Camera  *Get();
  };

  struct Scene
  {
    virtual bool IsLoading() = 0;
    virtual uint32_t AllocVerts( uint32_t num) = 0;
    virtual FPoint4 *GetVerts() = 0;
    virtual void Clear() = 0;
    virtual uint32_t GetNumVerts() = 0;
    virtual void processVertData() = 0;
    virtual void GenerateSample() = 0;
    virtual float GetSize() = 0;
    virtual void SetFileImage( void *pData, std::size_t sz,int fType) = 0;
    virtual const std::vector<Chunk*>& GetChunks() = 0;
    virtual FBdBox GetBdBox()= 0;
    virtual void GetZMax(float &zmin, float &zmax) = 0;
    virtual FPoint4* GetChunkPos() = 0;
    virtual FPoint4* GetChunkAuxPos() = 0;
    virtual void SetCamera() = 0;
    virtual void SetCameraOrto() = 0;
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
    static Renderer* Get();
  };
}

#endif

 