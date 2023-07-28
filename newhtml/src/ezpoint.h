#ifndef _EZPOINT_H
#define _EZPOINT_H

#include <cstddef>
#include <vector>
#include <memory>
#include "chunk.h"

namespace ezp 
{
	struct UI
	{
		virtual void PrintMessage( const char *pMsg) = 0;
		virtual void PrintMessage( const char *pMsg,int val) = 0;
		virtual void SetRenderEvent(int num)  = 0;
		virtual void GetValue( const char *pUiId) = 0;
		virtual int GetBkColor() = 0;
        static UI *Get();
	};

	struct Camera
	{
		virtual void GetPos( float &x, float &y, float &z) = 0;
		virtual void GetDir( float &x, float &y, float &z) = 0;
		virtual void GetUp( float &x, float &y, float &z) = 0;
		virtual void GetRight( float &x, float &y, float &z) = 0;
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
		virtual void BuildTest( int n) = 0;
		virtual float GetSize() = 0;
		virtual void SetFileImage( void *pData, std::size_t sz,int fType) = 0;
		virtual const std::vector<std::shared_ptr<Chunk>>& GetChunks() = 0;
		virtual FBdBox GetBdBox()= 0;
		virtual void GetZMax(float &zmin, float &zmax) = 0;
		virtual FPoint4* GetChunkPos() = 0;
		virtual FPoint4* GetChunkAuxPos() = 0;
		static Scene *Get();
	};

	struct Renderer
	{
		virtual void Init(int canvasW, int canvasH) = 0;
		virtual void Render(unsigned int *pBuff, int winW, int winH,int evnum) = 0;
		virtual float GetAtanRatio() = 0;
		virtual void  SetAtanRatio(float val) = 0;
		virtual void  SetBudget(float val) = 0;
		virtual void  SetPointSize(float val) = 0;
		virtual void  SetBkColor( uint32_t val)= 0;
		virtual void ShowFrameRate(bool val) = 0;
		virtual void SetDebugParam(int val) = 0;
		static Renderer* Get();
	};

	void Colorize();
}

#endif

 