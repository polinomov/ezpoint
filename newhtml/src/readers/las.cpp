//https://www.asprs.org/wp-content/uploads/2019/07/LAS_1_4_r15.pdf
#include <stdlib.h>
#include <iostream>
#include <memory>
#include <vector>
#include <stdlib.h>
#include <math.h>
#include "readers.h"

namespace ezp
{
    //  0x000000FF -blue ,0x0000FF00- green, 0x00FF0000- red
    static uint32_t classColors[16]={
        0x0000FF00,0x0000FF00,0x00FF0000,0x0000FF00,
        0x000000FF,0x000000FF,0x000000FF,0x000000FF,
        0x00FF0000,0x00FF0000,0x00FF0000,0x00FF0000,
        0x0000FFFF,0x0000FFFF,0x0000FFFF,0x0000FFFF
     };

    #pragma pack (1)
    struct LasHeader
    {
    char magic[4];              //0
    short fileSourceId;         //4
    short globalEncoding;       //6
    int   giud1;                //8
    short guid2;                //12
    short guid3;                //14
    char  guid4[8];             //16
    char verMajor;              //24
    char verMinor;              //25
    char sysId[32];             //26
    char getSoft[32];           //58
    short CreationDay;          //90
    short CretionYear;          //92
    short headerSize;            //94
    unsigned int pointOfst;     //98
    unsigned int numOfVars;     //102
    unsigned char pointDataFormatId; //106
    unsigned short poitDataRecordLength; //107
    unsigned int numOfPointRecords12;
    unsigned int NumberofPointByReturn[5];
    double xScale;
    double yScale;
    double zScale;
    double xOffset;
    double yOffset;
    double zOffset;
    double maxX;
    double maxY;
    double maxZ;
    double minX;
    double minY;
    double minZ;
    //
    uint64_t StartOfWave;
    uint64_t StartFirtstExt;
    int NumOfE;
    uint64_t numOfPointRecords14;
    uint64_t NumOfPointReturn;
    };

    //point formats
    #pragma pack (1)
    struct PointFormat1
    {
        int x;
        int y;
        int z;
        uint16_t intensity;
    };

    #pragma pack (1)
    struct PointFormat3
    {
        int32_t x;
        int32_t y;
        int32_t z;
        uint16_t intensity;
        uint8_t flg1;
        uint8_t classification;
        int8_t scanAngleRank;
        uint8_t userData;
        uint16_t pointSourceId;
        double   gpsTime;
        uint16_t red;
        uint16_t green;
        uint16_t blue;
    };

    struct Point6Flags{
       uint16_t returnNumber:4;  
       uint16_t givenPulse:4;  
       uint16_t classFlg:4;  
       uint16_t scanner:2;  
       uint16_t scanDir:1;  
       uint16_t edgeOfFlight:1;
    };
    #pragma pack (1)
    struct PointFormat6
    {
        int32_t x;
        int32_t y;
        int32_t z;
        uint16_t intensity;
        Point6Flags flg;
        uint8_t classification;   
    };

    static FBdBox FillBdBox(unsigned char *pSrc,int numPoints,const LasHeader *lh){
        unsigned char *pS = pSrc;
        int32_t xmin,xmax,ymin,ymax,zmin,zmax;
        xmin = ymin = zmin  = INT_MAX;
        xmin = ymin = zmin  = INT_MAX;
        for(int i = 0; i< numPoints; i++,pS+=lh->poitDataRecordLength){
            PointFormat1 *pf1 = (PointFormat1*)pS;
            xmin = std::min(pf1->x,xmin);
            ymin = std::min(pf1->y,ymin);
            zmin = std::min(pf1->z,zmin);
            xmax = std::max(pf1->x,xmax);
            ymax = std::max(pf1->y,ymax);
            zmax = std::max(pf1->z,zmax);
        }
        FBdBox Res;
        Res.xMin = (float)xmin * float(lh->xScale) + lh->xOffset;
        Res.yMin = (float)ymin * float(lh->yScale) + lh->yOffset;
        Res.zMin = (float)zmin * float(lh->zScale) + lh->zOffset;
        Res.xMax = (float)xmax * float(lh->xScale) + lh->xOffset;
        Res.yMax = (float)ymax * float(lh->yScale) + lh->yOffset;
        Res.zMax = (float)zmax * float(lh->zScale) + lh->zOffset;
        std::cout<<"X:"<<Res.xMin<<":"<<Res.xMax<<std::endl;
        std::cout<<"Y:"<<Res.yMin<<":"<<Res.yMax<<std::endl;
        std::cout<<"Z:"<<Res.zMin<<":"<<Res.zMax<<std::endl;
        return Res;
    }

    static void FillXYZ(unsigned char *pSrc,FPoint4 *pDst,int numPoints, const LasHeader *lh,float *pIntens ){
        unsigned char *pS = pSrc;
        FPoint4 *pD = (FPoint4*)pDst;
        for(int i = 0; i<numPoints; i++,pS+=lh->poitDataRecordLength,pD++)
        {
            PointFormat1 *pf1 = (PointFormat1*)pS;
            pD->x = (float)(pf1->x )*float(lh->xScale) + lh->xOffset;
            pD->y = (float)(pf1->y )*float(lh->yScale) + lh->yOffset;
            pD->z = (float)(pf1->z )*float(lh->zScale) + lh->zOffset;
            float vf = (float)(pf1->intensity)/(256.0f*256.0f);
            uint8_t c = (uint8_t)(vf*255.0f);
            pD->col = 0xFFFFFF;
            pIntens[c] += 1.0f;
        }
    }

    static void FillXYZ3(unsigned char *pSrc,FPoint4 *pDst,int numPoints, const LasHeader *lh,float *pIntens ){
        unsigned char *pS = pSrc;
        FPoint4 *pD = (FPoint4*)pDst;
        for(int i = 0; i<numPoints; i++,pS+=lh->poitDataRecordLength,pD++)
        {
            PointFormat3 *pf3 = (PointFormat3*)pS;
            pD->x = (float)(pf3->x )*float(lh->xScale) + lh->xOffset;
            pD->y = (float)(pf3->y )*float(lh->yScale) + lh->yOffset;
            pD->z = (float)(pf3->z )*float(lh->zScale) + lh->zOffset;
            float vf = (float)(pf3->intensity)/(256.0f*256.0f);
            uint8_t c = (uint8_t)(vf*255.0f);
            uint8_t r = pf3->red;
            uint8_t g = pf3->green;
            uint8_t b = pf3->blue;
            pD->col = b | (g<<8) | (r<<16);
            pIntens[c] += 1.0f;
        }
    }

    static void ProcessIntens( float *pIntens, int num, float &min, float &max ){
        float sum = 0.0f;
        for( int i = 0; i<num; i++) sum+=pIntens[i];
        int first = 0, last = num-1;
        float sums = sum*0.9f;
        for( int i = 0; i<num; i++){
            if(pIntens[first]<pIntens[last]){
                sum-=pIntens[first];
                first++;
            }
            else{
                sum-=pIntens[last];
                last--;
            }
            if((sum<sums) || ( last<=first)){
                break;
            }
        }
        min = (float)first/(float)num;
        max = (float)last/(float)num;
    }

    void ApplyIntems(unsigned char *pSrc,FPoint4 *pDst,int numPoints, const LasHeader *lh,float min, float max){
        float var1 = 0.1f;
        float var2 = 0.7f;
        float prd1 = (min>0.0f) ? var1/min : 0.0f;
        float prd2 = (max<1.0f) ? (1.0f-var2)/(1.0f-max) : 0.0f;
        float prd3 = (var2- var1)/(max-min);
        unsigned char *pS = pSrc;
        FPoint4 *pD = (FPoint4*)pDst;
        for(int i = 0; i<numPoints; i++,pS+=lh->poitDataRecordLength,pD++)
        {
            PointFormat1 *pf1 = (PointFormat1*)pS;
            float vf = (float)(pf1->intensity)/(256.0f*256.0f);
            float vf1;
            if(vf < min){
                vf1 = vf * prd1;
            }else if( vf > max){
                vf1 = var2 +(vf-max)*prd2;
            }else{
                vf1 = var1 + (vf-min)*prd3;
            }
            uint8_t c = (uint8_t)(vf1 * 255.0f);
            pD->col = c|(c<<8)|(c<<16);
        }
    }

    FBdBox ReadLasFile( void *pData, std::size_t sz,int &numPt,std::vector<std::shared_ptr<Chunk>> &chOut){
        std::cout<<"=== READING LAS ==="<<std::endl;
        FBdBox retBox;
        auto ch = std::make_shared<Chunk>();

        static float intens[256*256];
    
        LasHeader *lh = (LasHeader*)pData;
        char magic[5] = {0,0,0,0,0};
        memcpy(magic,lh->magic,4);
        if(strcmp(magic,"LASF")){
            std::cout<<"wrong magic:"<<magic<<std::endl;
            return retBox;
        }
        int vMajor = (int)lh->verMajor;
        int vMinor = (int)lh->verMinor;
        int ptFormat = (int)lh->pointDataFormatId;
        int recLength = (int)lh->poitDataRecordLength;
        int numPoints = 0;
        if((vMajor==1)&&(vMinor==1)){
            numPoints = lh->numOfPointRecords12;
        }
        if((vMajor==1)&&(vMinor==2)){
            numPoints = lh->numOfPointRecords12;
        }
        if((vMajor==1)&&(vMinor==3)){
            numPoints = lh->numOfPointRecords12;
        }
        if((vMajor==1)&&(vMinor==4)){
            numPoints = lh->numOfPointRecords14;
        }
        if(numPoints==0){
            std::cout<<"----unsupported version:"<<vMajor<<"."<<vMinor<<std::endl;
            return retBox ;
        }
        std::cout<<"version:"<<vMajor<<"."<<vMinor<<std::endl;
        std::cout<<"numpoints="<<numPoints<<" ofst="<<lh->pointOfst<<" ptFormat="<<ptFormat<<" rlen="<<recLength<<std::endl;
        memset(intens, 0, 256*256*sizeof(float));
        unsigned char *pStart = (unsigned char*)pData + lh->pointOfst;
        retBox = FillBdBox(pStart, numPoints,lh);
        ///
        chOut.push_back(ch);
        FPoint4 *pRet = new FPoint4[numPoints];
        ch->numVerts = numPoints;
        ch->pVert = (float*)pRet;
        ///

        FillXYZ(pStart, pRet, numPoints,lh,intens);
        //FillXYZ3(pStart, pRet, numPoints,lh,intens);
        // adjust colors
        float min, max;
        ProcessIntens( intens, 255 , min, max);
        std::cout<<"first = "<<min<<" last="<<max<<std::endl;
        if( max > min){
            ApplyIntems(pStart, pRet, numPoints,lh,min, max);
        }
        numPt = numPoints;
        return retBox; 
    }

 }//namespace ezp