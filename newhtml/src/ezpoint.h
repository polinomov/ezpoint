#ifndef _EZPOINT_H
#define _EZPOINT_H

#include <cstddef>
#include <vector>
#include <memory>

namespace ezp 
{
	struct UI
	{
		virtual void PrintMessage( const char *pMsg) = 0;
		virtual void PrintMessage( const char *pMsg,int val) = 0;
		virtual void SetRenderEvent()  = 0;
        static UI *Get();
	};

	struct Camera
	{
		static Camera *Get();
	};

	struct Scene
	{
		struct Chunk{
			float xMin,xMax,yMin,yMax,zMin,zMax;
			int numVerts;
			float *pVert;
			unsigned int *pColors;
		};

        virtual bool IsLoading() = 0;
		virtual void SetFileImage( void *pData, std::size_t sz,int fType) = 0;
		virtual const std::vector<std::shared_ptr<Chunk>>& GetChunks() = 0;
		static Scene *Get();
	};

	struct Renderer
	{
		virtual void Init(int canvasW, int canvasH) = 0;
		virtual void Render(unsigned int *pBuff, int winW, int winH) = 0;
		static Renderer* Get();
	};
}

#endif

 