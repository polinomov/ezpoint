//
#ifndef _READERS_H
#define _READERS_H


namespace ezp 
{
    struct FPoint4
    {
        float x;
        float y;
        float z;
        unsigned int col;
    };

    FPoint4* BuildTestScene(int &numPoints);
    FPoint4* ReadLasFile( void *pData, std::size_t sz, int &numPt);
}

#endif
