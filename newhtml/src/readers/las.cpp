//
#include <iostream>
#include <memory>
#include <vector>
#include <stdlib.h>
#include <math.h>
#include "readers.h"

namespace ezp
{


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
    struct PointFromat1
    {
        int x;
        int y;
        int z;
        unsigned short val;
    };

    #pragma pack (1)
    struct PointFormat6
    {
        int32_t x;
        int32_t y;
        int32_t z;
        uint16_t intensity;
    };
    
    static void FillFormat1(unsigned char *pSrc,FPoint4 *pDst,int numPoints, const LasHeader *lh ){
        unsigned char *pS = pSrc;
        FPoint4 *pD = (FPoint4*)pDst;
        for(int i = 0; i<numPoints; i++,pS+=lh->poitDataRecordLength,pD++)
        {
            PointFromat1 *pf1 = (PointFromat1*)pS;
            pD->x = (float)(pf1->x )*float(lh->xScale) + lh->xOffset;
            pD->y = (float)(pf1->y )*float(lh->yScale) + lh->yOffset;
            pD->z = (float)(pf1->z )*float(lh->zScale) + lh->zOffset;
            pD->col = 0xFFFFFFFF;
        }
    }

    FPoint4* ReadLasFile( void *pData, std::size_t sz,int &numPt){
        std::cout<<"--------READING LAS -------------"<<std::endl;
        LasHeader *lh = (LasHeader*)pData;
        char magic[5] = {0,0,0,0,0};
        memcpy(magic,lh->magic,4);
        if(strcmp(magic,"LASF")){
            std::cout<<"wrong magic:"<<magic<<std::endl;
            return NULL;
        }
        int vMajor = (int)lh->verMajor;
        int vMinor = (int)lh->verMinor;
        int ptFormat = (int)lh->pointDataFormatId;
        int recLength = (int)lh->poitDataRecordLength;
        int numPoints = 0;
        if((vMajor==1)&&(vMinor==2)){
            numPoints = lh->numOfPointRecords12;
        }
        if((vMajor==1)&&(vMinor==4)){
            numPoints = lh->numOfPointRecords14;
        }
        if(numPoints==0){
            std::cout<<"----unsupported version:"<<vMajor<<"."<<vMinor<<std::endl;
            return NULL;
        }
        std::cout<<"version:"<<vMajor<<"."<<vMinor<<std::endl;
        std::cout<<"numpoints="<<numPoints<<" ofst="<<lh->pointOfst<<" ptFormat="<<ptFormat<<" rlen="<<recLength<<std::endl;
        unsigned char *pStart = (unsigned char*)pData + lh->pointOfst;
        FPoint4 *pRet = new FPoint4[numPoints];
        FillFormat1(pStart, pRet, numPoints,lh);
        numPt = numPoints;
        return pRet;
    }

 }//namespace ezp