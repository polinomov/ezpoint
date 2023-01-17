#ifndef _EZPOINT_H
#define _EZPOINT_H

namespace ezp 
{
	struct Camera
	{
		static Camera *Get();
	};

	struct Scene
	{
		virtual void AddPoint(float x, float y, float z, unsigned int c)  = 0;
	};

	struct Renderer
	{
		virtual void Init(int canvasW, int canvasH) = 0;
		virtual void Render(unsigned int *pBuff, int winW, int winH) = 0;
		static Renderer* Get();
	};
}

#endif

 