//
#ifndef _READERS_H
#define _READERS_H

#include "..\chunk.h"
namespace ezp 
{
    FPoint4* BuildTestScene(int &numPoints);
    FBdBox ReadLasFile( void *pData, std::size_t sz, int &numPt,std::vector<std::shared_ptr<Chunk>> &chOut);
}

#endif
 