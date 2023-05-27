//Generate test data
#include <iostream>
#include <memory>
#include <vector>
#include <stdlib.h>
#include <math.h>
#include "ezpoint.h"
#include "readers/readers.h"

// http://ewebmap.ci.lubbock.tx.us/currentdata/LiDAR/2010/All_Return_LAS/86817242.las
namespace ezp {

Chunk::Chunk() : numVerts(0),pVert(NULL){
    std::cout<<"---Chunk Ctor---"<<std::endl;
}

Chunk::~Chunk(){
   std::cout<<"Chunk DCtor"<<std::endl;
}   

void Colorize(){
    const uint32_t kSizex = 32;
    const uint32_t kSizey = 32;
    return;
    //uint32_t *pBf = new uint32_t[kSizex*kSizey]; 
    std::unique_ptr<uint32_t[]> pBf(new uint32_t[kSizex*kSizey]);
    for ( uint32_t k = 0; k<kSizex*kSizey; k++){
        pBf[k] = 0;
    }
    
    //memset(pBf,0,kSizex*kSizey*sizeof(uint32_t));

    std::vector<std::shared_ptr<Chunk>> chs = Scene::Get()->GetChunks();
    float xmin = chs[0]->xMin;
    float xmax = chs[0]->xMax;
    float ymin = chs[0]->yMin;
    float ymax = chs[0]->yMax;
    for (auto & ch : chs) {
        xmin = std::min(xmin,ch->xMin);
        xmax = std::max(xmax,ch->xMax);
        ymin = std::min(ymin,ch->yMin);
        ymax = std::max(ymax,ch->yMax);
    }
    float dx = xmax - xmin;
    float dy = ymax - ymin;
    std::cout<<"ZZZZZZZZZZZZZZZ dx "<<dx<<std::endl;
    std::cout<<"ZZZZZZZZZZZZZZZ dy "<<dy<<std::endl;
    uint32_t totalPt = 0;
    for (auto & ch : chs) {
        totalPt+=ch->numVerts;
    }

    for (auto & ch : chs){
        FPoint4 *pts = (FPoint4*)ch->pVert;
        for(int k = 0; k<ch->numVerts; k++,pts++){
            uint32_t xn = (uint32_t)((float)kSizex*(pts->x -xmin)/dx);
            uint32_t yn = (uint32_t)((float)kSizey*(pts->y -ymin)/dy);
            if(xn<0) xn = 0;
            if(xn>=kSizex) xn = kSizex -1;
            if(yn<0) yn = 0;
            if(yn>=kSizey) yn = kSizey -1;
            uint32_t nn = xn + yn*kSizex;
            pts->col = (nn&1) ? 0xFF : 0xFF0000;
        }
    }
    return;
    /*
    for (auto & ch : chs) {
        FPoint4 *pts = (FPoint4*)ch->pVert;
        for(int k = 0; k<ch->numVerts; k++,pts++){
            uint32_t xn = (uint32_t)((float)kSizex*(pts->x -xmin)/dx);
            uint32_t yn = (uint32_t)((float)kSizey*(pts->y -ymin)/dy);
            if(xn<0) xn = 0;
            if(xn>=kSizex) xn = kSizex -1;
            if(yn<0) yn = 0;
            if(yn>=kSizey) yn = kSizey -1;
            uint32_t nn =pBf[xn + yn*kSizex];
            if( nn>5){
                pts->col = 0xFF00;
            }
            else{
                pts->col = 0x808080;
             }
        }
    }    
    delete []pBf;
    */
}
}//namespace ezp
