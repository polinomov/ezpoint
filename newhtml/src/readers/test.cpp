//Generate test data
#include <iostream>
#include <memory>
#include <vector>
#include <stdlib.h>
#include <math.h>
#include "readers.h"

namespace ezp 
{
    static void Sphere(FPoint4* pDst, int rad, float cx, float cy, float cz, unsigned int color,int numV){
        FPoint4* pv = pDst;
        for (int k = 0; k < numV; k++,pv++)
        {
            float tx =2.0f* (float)(rand()%rad ) - (float)rad;
            float ty =2.0f* (float)(rand()%rad ) - (float)rad;
            float tz =2.0f* (float)(rand()%rad ) - (float)rad;
            float dst = sqrtf(tx * tx + ty * ty + tz * tz);
            if (dst > 0.0f) {
                float prd =  (float)rad/ dst;
                pv->x = cx + tx * prd;
                pv->y = cy + ty * prd;
                pv->z = cz + tz * prd;
            }
            else {
                pv->x = cx;
                pv->y = cy;
                pv->z = cz;
            }
            pv->col = color;
        }
    }

    FPoint4* BuildTestScene(int &dataSize){
        int numVertsInSphere = 1024 * 1024;
        int numSh = 4;
        int sz = numVertsInSphere * numSh;
        float rad = 1000.0f;
        srand(time(NULL));
        FPoint4* pData = new FPoint4[sz];
        FPoint4* pF = pData;
        Sphere(pF, rad, -rad, -rad, 0.0f,0xFFFFFFFF, numVertsInSphere); 
        pF += numVertsInSphere;
        Sphere(pF, rad, rad,  rad, 0.0f, 0x00FF0000, numVertsInSphere); 
        pF += numVertsInSphere;
        Sphere(pF, rad, rad, -rad, 0.0f, 0x0000FF00, numVertsInSphere); 
        pF += numVertsInSphere;
        Sphere(pF, rad, -rad, rad, 0.0f, 0x000000FF, numVertsInSphere); 
        dataSize = sz*sizeof(FPoint4);
        return pData;
    }
}