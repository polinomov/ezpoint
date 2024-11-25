/*
  lasinspector - online point cloud viewer
  Copyright (c) 2020-2024 ezpoint3d
*/

#include <iostream>
#include <functional>
namespace chunker
{
static int depth = 0;
static bool hasError = false;

static void ShowBdBox(ezp::FBdBox &fb){
  std::cout<<"BDBOX: "<<fb.xMin<<" "<<fb.xMax<<"|";
  std::cout<<fb.yMin<<" "<<fb.yMax<<"|";
  std::cout<<fb.zMin<<" "<<fb.zMax<<std::endl;
}

template <class T>
static ezp::FBdBox getBdBox(const T* pVerts, uint32_t first, uint32_t last){
    ezp::FBdBox fb;
    fb.xMin =  pVerts[first].x;
    fb.xMax =  pVerts[first].x;
    fb.yMin =  pVerts[first].y;
    fb.yMax =  pVerts[first].y;
    fb.zMin =  pVerts[first].z;
    fb.zMax =  pVerts[first].z;    
    for( int i = first+1; i<=last; i++){
        fb.xMin = std::min(fb.xMin,pVerts[i].x);
        fb.xMax = std::max(fb.xMax,pVerts[i].x);
        fb.yMin = std::min(fb.yMin,pVerts[i].y);
        fb.yMax = std::max(fb.yMax,pVerts[i].y);
        fb.zMin = std::min(fb.zMin,pVerts[i].z);
        fb.zMax = std::max(fb.zMax,pVerts[i].z);
    }
   // ShowBdBox(fb);
    return fb;
}

template <class T> 
static auto getMaxSize(T* pVerts, uint32_t first, uint32_t last,uint32_t &t){
    ezp::FBdBox fb = getBdBox<T>(pVerts, first, last);  
    auto dx = fb.xMax - fb.xMin;
    auto dy = fb.yMax - fb.yMin;
    auto dz = fb.zMax - fb.zMin;
    auto dMax = dx;
    auto ret = fb.xMin + dx/2.0f;
    t = 0;
    if(dy>dMax){
        dMax = dy; 
        ret = fb.yMin + dy/2.0f;
        t = 1;
    }
    if(dz>dMax){
        dMax = dz;  
        ret = fb.zMin + dz/2.0f;
        t = 2;
    }
    return ret;
}

template <class T, int N> 
static uint32_t doPartition(T* pV , uint32_t first, uint32_t last, decltype(pV->x) mid){
    uint32_t nl =0;
    for( uint32_t k = first; k<=last; k++ ){
        bool isLeft = false;
        if(N==0){
            isLeft = (pV[k].x<mid);
        }
        if(N==1){
            isLeft = (pV[k].y<mid);
        }
        if(N==2){
            isLeft = (pV[k].z<mid);
        }
        if(isLeft){
            T tmp = pV[k];
            pV[k] = pV[nl+first];
            pV[nl+first] = tmp;
            nl++;
        }
    }
    uint32_t ret = nl+first-1;
    if((nl==0)||(nl==last-first+1)){
        ezp::FBdBox fb =getBdBox(pV, first, last);
        //std::cout<<"@@@@@@@@@ FAILED TO PARITION N="<<N<<" nl="<<nl<<std::endl;
        ShowBdBox(fb);
        //std::cout<<"first="<<first<<" last="<<last<<" mid="<<mid<<std::endl;
        return -1;
    }

    return nl+first-1;
}

template <class T> 
void doChunks(T* pV, uint32_t first, uint32_t last, uint32_t chSz, std::function<void(T *pFirst,uint32_t n )>  onDone )
{
    static_assert(std::is_same<decltype(pV->x), decltype(pV->y)>::value );
    static_assert(std::is_same<decltype(pV->x), decltype(pV->z)>::value );
    uint32_t t;
    uint32_t num = last-first + 1;
    if(hasError){
        return;
    }
    if(depth>100){
        std::cout<<"depth "<<depth << " num="<<num<<std::endl;
    }

    if(first>last){
        std::cout<<"@@@@@@@@@@@@@ doChunks HERR @@@@@@@ first ="<<first<<" last"<< last<< std::endl;
        //hasError = true;
        return;
    }
    if((num <= chSz)||(first==last)){
        onDone(pV + first,num);
        depth--;
        return;
    }
    auto midP = getMaxSize<T>(pV, first,last, t);
    int left;
    switch(t){
        case 0:
            left= doPartition<T,0>(pV , first, last, midP);
        break;
        case 1:
            left= doPartition<T,1>(pV , first, last, midP);
        break;
        case 2:
            left= doPartition<T,2>(pV , first, last, midP);
        break;
    }
    depth++;
    if(left != -1){
        doChunks(pV, first,  left, chSz, onDone);
        doChunks(pV, left+1, last, chSz, onDone);
    }
}
} //namespace chunker
