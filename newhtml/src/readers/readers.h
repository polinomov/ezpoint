//
#ifndef _READERS_H
#define _READERS_H
#include <functional>
#include "..\chunk.h"
#define COLOR05(r_, g_, b_) (1 + b_ + g_*6 + r_*36)

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
        std::string description;
    };

    struct PointBuilder{
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
        virtual void Reset(uint32_t fSize) = 0;
        virtual void PostProcessAllColors(
            uint32_t numMemBanks,
            bool hasRgb,
            std::function<const FPoint4 *(uint32_t ndx)> getVerts,
            std::function<uint32_t (uint32_t ndx)> getNum
        ) = 0;
        static PointBuilder *GetLasBuilder();
        static PointBuilder *GetXyzTxtBuilder();
    };
}

#endif
 