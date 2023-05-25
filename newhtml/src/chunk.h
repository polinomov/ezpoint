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

    struct Chunk{
        Chunk();
        ~Chunk();
		float xMin,xMax,yMin,yMax,zMin,zMax;
		int numVerts;
		float *pVert;
		uint32_t flg;
 	};
}

#endif
 