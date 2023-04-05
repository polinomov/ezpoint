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

    FPoint4* BuildTestScene1(int &dataSize){
        int numVertsInSphere = 1024 * 1024/4;
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

    void BuildCylinder(FPoint4* pDst, float xa, float ya, float za, float xb, float yb, float zb, float rad, int num)
    {
        float dx  = xb -xa;
        float dy  = yb -ya;
        float dz  = zb -za;
        float dd = sqrt(dx*dx + dy*dy + dz*dz);
        float nx = dy - dz;
        float ny = dz - dx;
        float nz = dx - dy;
        float nn = rad/sqrt( nx*nx + ny*ny + nz*nz);
        nx *=nn ; ny *= nn; nz *= nn;
        float sx =  dy * nz - dz * ny;
        float sy = -dx * nz + dz * nx;
        float sz =  dx * ny - dy * nx;
        float ss = rad/sqrt( sx*sx + sy*sy + sz*sz);
        sx *=ss; sy *= ss; sz *= ss;
        FPoint4* pv = pDst;
        for( int i = 0 ; i<num; i++,pv++){
            float vf = rand() / float(RAND_MAX);
            float vf1 = rand() / float(RAND_MAX);
            float sinf = sin(vf * 6.28f);
            float cosf = cos(vf * 6.28f);
   
            float rf =  nx *sinf + sx * cosf;
            float gf =  ny *sinf + sy * cosf;
            float bf =  nz *sinf + sz * cosf; 

            pv->x = xa +  (xb - xa)*vf1 + rf;
            pv->y = ya +  (yb - ya)*vf1 + gf;
            pv->z = za +  (zb - za)*vf1 + bf;
            
            unsigned char rr = (unsigned char)(abs(rf) *256.0f/rad);
            unsigned char gg = (unsigned char)(abs(gf) *256.0f/rad);
            unsigned char bb = (unsigned char)(abs(bf) *256.0f/rad);
            pv->col = (rr<<16) | (gg<<8) | bb;          
        }
    }

    void GetPt( float u, float &tx, float &ty, float &tz){
        tx = (sin(u) + 2.0f * sin(2.0f * u));
        ty = (cos(u) -  2.0f* cos(2.0f*u));
        tz = (-sin(3.0f*u));
     }

    void GetPt1( float u, float &tx, float &ty, float &tz){
        tx = (sin(u));
        ty = (cos(u));
        tz = 0.0f;
     }


    FPoint4* BuildTestScene(int &dataSize){
        int numPoints = 1024;
        int numImSpr = 1024;
        FPoint4* pData = new FPoint4[numPoints*numImSpr];
        srand(time(NULL));
        FPoint4* pF = pData;
        float tx1,ty1,tz1,tx2,ty2,tz2;
        for( int k = 0; k<numPoints; k++){
            float u1 = (float)(k + 0)* 2.0f * 3.1451f /(float)numPoints;
            float u2 = (float)(k +1 )* 2.0f * 3.1451f /(float)numPoints;
            GetPt( u1, tx1, ty1, tz1);
            GetPt( u2, tx2, ty2, tz2);
            unsigned int col = (k&1)? 0xFF0000 : 0xFF00;
            //Sphere(pF, 10, tx1,ty1, tz1, col, numImSpr);
            BuildCylinder(pF, tx1, ty1, tz1, tx2, ty2, tz2, 0.5f, numImSpr);
            pF+=numImSpr;
        }       
        dataSize = numImSpr*numPoints*sizeof(FPoint4);
        return pData;
    }
}