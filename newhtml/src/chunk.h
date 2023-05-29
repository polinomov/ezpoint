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
        void Randomize();
		float xMin,xMax,yMin,yMax,zMin,zMax;
		int numVerts;
		float *pVert;
		uint32_t aux;
        int32_t tst;
 	};
}

#endif
 