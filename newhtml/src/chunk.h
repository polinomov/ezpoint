//
#ifndef _CHUNK_H
#define _CHUNK_H


namespace ezp 
{
    struct FPoint4 {
        float x;
        float y;
        float z;
        unsigned int col;
    };

    struct FBdBox{
        float xMin,xMax,yMin,yMax,zMin,zMax;
    };

    #define CHUNK_FLG_NV (1<<2)
    struct Chunk{
        Chunk();
        ~Chunk();
        void Randomize();
        void BuildBdBox();
		float xMin,xMax,yMin,yMax,zMin,zMax;
		int numVerts;
		float *pVert;
		uint32_t aux;
        int32_t flg;
        float cx,cy,cz,sz;
        uint32_t numToRender;
        FPoint4 proj;
        float reduction;
 	};
}

#endif
 