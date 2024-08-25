//
#ifndef _READERS_H
#define _READERS_H

#include "..\chunk.h"
namespace ezp 
{
    struct LasInfo{
        uint32_t numPoints;
        uint32_t vMajor;
        uint32_t vMinor;
        uint32_t vertType;
        uint32_t hasClass;
        uint32_t hasRgb;
        uint32_t err;
    };

    FPoint4* BuildTestScene(int &numPoints);
    FBdBox ReadLasFile( void *pData, std::size_t sz, int &numPt,std::vector<std::shared_ptr<Chunk>> &chOut,LasInfo &li);
    uint8_t* GenerateSampleLas();

    struct LasBuilder{
        virtual uint32_t SetChunkData(void *pData) = 0;
        static LasBuilder *Get();
    };
}

#endif
 