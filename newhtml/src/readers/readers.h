//
#ifndef _READERS_H
#define _READERS_H
#include <functional>
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
        virtual void RegisterCallbacks(
            // alloc returns memory bank index
            std::function<int (uint32_t n )> alloc,  
            //getVerts returns start of the vert array for a memory bank index ndx               
            std::function<const FPoint4 *(uint32_t ndx)> getVerts, 
            // when error occurs
            std::function<void( const std::string &msg)> onErr,
            // writes to the info
            std::function<int( const LasInfo &info)> onInfo
        ) = 0;
        virtual uint32_t SetChunkData(void *pData) = 0;

        virtual void Reset() = 0;
        static LasBuilder *Get();
    };
}

#endif
 