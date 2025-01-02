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

Chunk::Chunk() : numVerts(0),pVert(NULL),numToRender(0),reduction(1.0f), processed(0){
}

Chunk::~Chunk(){
   //std::cout<<"Chunk DCtor"<<std::endl;
}

void Chunk::Randomize(){
    FPoint4 *fp = (FPoint4*)pVert;
    srand(12345);
    //return;
    for( int i = 0; i<numVerts*2; i++){
        uint32_t r1 = rand()%numVerts;
        uint32_t r2 = rand()%numVerts;
        if((r1>=0)&&(r1<numVerts)&&(r2>=0)&&(r2<numVerts)){
            FPoint4 tmp = fp[r1];
            FPoint4 pt = fp[r2];
            //if(pt.z< tmp.z){
            fp[r1] = fp[r2];
            fp[r2] = tmp;
            //}
        }
    }
}

void Chunk::BuildBdBox(){
    if(pVert==NULL) return;
    FPoint4 *pf = (FPoint4*)pVert;
    xMin = xMax = pf->x;
    yMin = yMax = pf->y;
    zMin = zMax = pf->z;
    for( int i = 0; i<numVerts; i++,pf++){
        xMin = std::min(xMin,pf->x);
        yMin = std::min(yMin,pf->y);
        zMin = std::min(zMin,pf->z);
        xMax = std::max(xMax,pf->x);
        yMax = std::max(yMax,pf->y);
        zMax = std::max(zMax,pf->z);
    }
    cx = (xMin + xMax) * 0.5f;
    cy = (yMin + yMax) * 0.5f;
    cz = (zMin + zMax) * 0.5f;
    float dx = xMax - xMin;
    float dy = yMax - yMin;
    float dz = zMax - zMin;
    //std::cout<<"CH:"<<dx<<" "<<dy<<" "<<dz<<std::endl;
    sz = sqrt(dx*dx + dy*dy + dz*dz) * 0.5f;
   // std::cout<<"-cx="<<cx<<" cy="<<cy<<" cz="<<cz<<" sz="<<sz<<std::endl;
}  
}//namespace ezp
