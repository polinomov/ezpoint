//https://www.asprs.org/wp-content/uploads/2019/07/LAS_1_4_r15.pdf
#include <stdlib.h>
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <stdlib.h>
#include <math.h>
#include "readers.h"

namespace ezp
{
    //  0x000000FF -blue ,0x0000FF00- green, 0x00FF0000- red
    /*
    static uint32_t classColors[16]={
        0x0000FF00,0x0000FF80,0x00FF0000,0x0080FF00,
        0x000000FF,0x000000FF,0x000000FF,0x000000FF,
        0x00FF0000,0x00FF0000,0x00FF0000,0x00FF0000,
        0x0000FFFF,0x0000FFFF,0x0000FFFF,0x0000FFFF
     };
     */

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
    
    #pragma pack (1)
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

    #pragma pack (1)
    struct PointFormat7
    {
        int32_t x;
        int32_t y;
        int32_t z;
        uint16_t intensity;
        uint16_t flg;
        uint8_t classification;   
        uint8_t userData;   
        int16_t scanAngle;   
        uint16_t pointSourceId; 
        double gpsTime;
        uint16_t red; 
        uint16_t green; 
        uint16_t blue; 
    };

    bool ptHasColor( int ptf){
        if((ptf==7)||(ptf==3)) return true;
        return false;
    }
    bool ptHasClass( int ptf){
        if((ptf==7)||(ptf==6)) return true;
        return false;
    }

    uint32_t ptGetClass( int ptf,void *pt){
        if((ptf==7)||(ptf==6)){
            PointFormat6 *pt6 = (PointFormat6*)pt;
            Point6Flags flg = pt6->flg;
            uint8_t c2 = pt6->classification;
            return c2;
        }
        return 0;
    }

    void ptGetColor( uint16_t &r,uint16_t &g,uint16_t &b, int ptType,void *pt ){
        if(ptType==7){
            PointFormat7 *p7 = (PointFormat7*)pt;
            r = p7->red;
            g = p7->green;
            b = p7->blue;
        }
        if(ptType==3){
            PointFormat3 *p3 = (PointFormat3*)pt;
            r = p3->red;
            g = p3->green;
            b = p3->blue;
        }
    }

    static uint32_t GetNdx(uint32_t ssz,float val,float valMin, float delta){
        int32_t ndx= (int32_t)((float)ssz*(val -valMin)/delta);
        if(ndx<0) ndx = 0;
        if(ndx >= ssz) ndx = (int32_t)(ssz -1);
        return (uint32_t)ndx;
    }

    static FBdBox FillBdBox(unsigned char *pSrc,int numPoints,const LasHeader *lh,float *pIntens,
        std::vector<std::shared_ptr<Chunk>> &chOut){

        int ptFormat = (int)lh->pointDataFormatId;
        bool hasClass = ptHasClass(ptFormat);
        bool hasColor = ptHasColor(ptFormat);
    
        unsigned char *pS = pSrc;
        //float *itmp_16 = new float[256*256];
        std::unique_ptr<float[]> itmp_16(new float[256*256]);
        std::unique_ptr<float[]> r_16(new float[256*256]);
        std::unique_ptr<float[]> g_16(new float[256*256]);
        std::unique_ptr<float[]> b_16(new float[256*256]);
        uint16_t rMin,gMin,bMin; 
        uint16_t rMax,gMax,bMax;
        rMin=gMin=bMin = 0xFFFF;
        rMax=gMax=bMax = 0;
        for(int i= 0; i<256*256; i++){
            r_16[i] = 0;
            g_16[i] = 0;
            b_16[i] = 0;
            itmp_16[i] = 0.0f;
        }
        int32_t xmin,xmax,ymin,ymax,zmin,zmax;
        xmin = ymin = zmin  = INT_MAX;
        xmax = ymax = zmax  = INT_MIN;
        uint16_t intens_max = 0;
        uint16_t intens_min = 0xFFFF;
        uint32_t classHist[256];
        for( int n = 0; n<256; n++) {
            classHist[n] = 0;
        }
        for(int i = 0; i< numPoints; i++,pS+=lh->poitDataRecordLength){
            PointFormat1 *pf1 = (PointFormat1*)pS;
            xmin = std::min(pf1->x,xmin);
            ymin = std::min(pf1->y,ymin);
            zmin = std::min(pf1->z,zmin);
            xmax = std::max(pf1->x,xmax);
            ymax = std::max(pf1->y,ymax);
            zmax = std::max(pf1->z,zmax);
            intens_max  = std::max(intens_max ,pf1->intensity);
            intens_min  = std::min(intens_min ,pf1->intensity);
            float vf = (float)(pf1->intensity)/(256.0f*256.0f);
            uint8_t c = (uint8_t)(vf*255.0f);
            pIntens[c] += 1.0f;
            itmp_16[pf1->intensity] += 1.0f;
            if(hasColor){
                uint16_t rr,gg,bb;
                ptGetColor( rr,gg,bb, ptFormat,pf1);
                rMin  = std::min(rMin,rr);
                rMax  = std::max(rMax,rr);
                gMin  = std::min(gMin,gg);
                gMax  = std::max(gMax,gg);
                bMin  = std::min(bMin,bb);
                bMax  = std::max(bMax,bb);
                r_16[rr] += 1.0f;
                g_16[gg] += 1.0f;
                b_16[bb] += 1.0f;
            }
            if((int)lh->pointDataFormatId==6){
                PointFormat6 *pt6 = (PointFormat6*)pS;
                classHist[pt6->classification]++;
            }
        }
        if(hasColor){
            printf("---- rMin=%d rMax=%d gMin=%d gMax = %d bMin = %d bMax = %d\n", rMin,rMax,gMin,gMax,bMin,bMax);
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
        std::cout<<"IMIN="<<intens_min<<" IMAX="<<intens_max<<std::endl;
       // normalize colors
        for( int i =1; i<256*256; i++){
            itmp_16[i] +=  itmp_16[i-1];
            r_16[i] += r_16[i-1];
            g_16[i] += g_16[i-1];
            b_16[i] += b_16[i-1];
        }
        for( int i =0; i<256*256; i++){
            if( itmp_16[256*256-1]>0.f){
                itmp_16[i] *= (255.0f/itmp_16[256*256-1]);
            }
            if( r_16[256*256-1]>0.f){
                r_16[i] *= (31.0f/r_16[256*256-1]);
            }
            if( g_16[256*256-1]>0.f){
                g_16[i] *= (31.0f/g_16[256*256-1]);
            }
            if( b_16[256*256-1]>0.f){
                b_16[i] *= (31.0f/b_16[256*256-1]);
            }
        }

        //
        // Allocate temp array to collect chunks
        //
        uint32_t kSizex = 64;
        uint32_t kSizey = 64;
        std::unique_ptr<uint32_t[]> pBf(new uint32_t[kSizex*kSizey]);
        for ( uint32_t k = 0; k<kSizex*kSizey; k++){
            pBf[k] = 0;
        }
        float dx = Res.xMax - Res.xMin;
        float dy = Res.yMax - Res.yMin;
        //
        // Fill pBf ( number of points inside each chunk)
        //
        pS = pSrc;
        for(int i = 0; i<numPoints; i++,pS+=lh->poitDataRecordLength){
            PointFormat1 *pf1 = (PointFormat1*)pS;
            float xf = (float)(pf1->x )*float(lh->xScale) + lh->xOffset;
            float yf = (float)(pf1->y )*float(lh->yScale) + lh->yOffset;
            uint32_t nx = GetNdx(kSizex,xf,Res.xMin, dx);
            uint32_t ny = GetNdx(kSizey,yf,Res.yMin, dy);
            uint32_t nn = nx + ny*kSizex;
            pBf[nn]++;
        }
        uint32_t tt = 0;
        for( int i = 0; i<kSizex*kSizey; i++){
           // std::cout<<"++++i="<<i<<" "<<pBf[i]<<std::endl;
            tt+=pBf[i];
        }
        std::cout<<"+++++ tot="<<tt<<" from "<<numPoints<<std::endl;
#if 1       
        //
        // Allocate points inside each chunk 
        //
        std::unordered_map<uint32_t,std::shared_ptr<Chunk>> tab;
        for( uint32_t i = 0; i<kSizex*kSizey; i++){
            if(pBf[i]>0){
                std::shared_ptr<Chunk> chk = std::make_shared<Chunk>();
                chk->numVerts = pBf[i];
                uint8_t *pMem = (uint8_t*)malloc(pBf[i]*4*sizeof(float) + 128);
                uint64_t addr = (uint64_t)pMem;
                addr = (addr +128) &(~127);
                chk->pVert =(float*)addr;//new float[pBf[i]*4];
                chk->aux = 0;
                chOut.push_back(chk);
                tab[i] = chk;
                addr= (uint64_t)chk->pVert;
                if( addr & 0x7F){                    
                    printf("addr is not alligned %llx\n",addr);
                }
                //std::cout<<"New chunk "<<i<<" numv="<< chk->numVerts<< std::endl;
            }
        }
        std::shared_ptr<Chunk> chk = std::make_shared<Chunk>();
        chOut.push_back(chk);
        //
        // Fill chunks
        //
        pS = pSrc;
        int ptf = (int)lh->pointDataFormatId;
        for(int i = 0; i<numPoints; i++,pS+=lh->poitDataRecordLength){
            PointFormat1 *pf1 = (PointFormat1*)pS;
            float xf = (float)(pf1->x )*float(lh->xScale) + lh->xOffset;
            float yf = (float)(pf1->y )*float(lh->yScale) + lh->yOffset;
            float zf = (float)(pf1->z )*float(lh->zScale) + lh->zOffset;
            uint32_t nx = GetNdx(kSizex,xf,Res.xMin, dx);
            uint32_t ny = GetNdx(kSizey,yf,Res.yMin, dy);
            uint32_t nn = nx + ny*kSizex;
            auto chk_iter = tab.find(nn);
            if( chk_iter != tab.end()){
                std::shared_ptr<Chunk> chk  = chk_iter->second;
                if(chk->aux < chk->numVerts){
                    FPoint4 *pPoint = (FPoint4*)chk->pVert;
                    pPoint[chk->aux].x = xf;
                    pPoint[chk->aux].y = yf;
                    pPoint[chk->aux].z = zf;
                    // colors
                    uint8_t cint = 0, cls = 0;
                    uint16_t col16 = 0;
                    uint16_t c1 = pf1->intensity;
                    cint = (uint8_t)itmp_16[c1]; 
                    if(hasClass){
                         cls = (uint8_t)ptGetClass(ptFormat,pS); 
                    }
                    if(hasColor){
                        uint16_t rv,gv,bv;
                        ptGetColor( rv,gv,bv,ptFormat, pS);
                        uint8_t r8 = (rv>>3) & 0x1f;
                        uint8_t g8 = (gv>>3) & 0x1f;
                        uint8_t b8 = (bv>>3) & 0x1f;
                        col16  = r8 | (g8<<5) | (b8<<10);
                    }
                    pPoint[chk->aux].col = cint | (cls<<8) | (col16<<16);
                    chk->aux++;
                }
                else{
                   // std::cout<<"Too many verts in chunk "<<nx<<" "<<ny<<" "<<chk->numVerts<<" tst="<<chk->tst<<std::endl;
                   // return Res;
                }
            }else{
                std::cout<<"Can ot find chunk for "<<nx<<" "<<ny<<std::endl;
                return Res;
            }
        }
        //std::cout<<"SIZE="<<chOut.size()<<std::endl;
        for (auto & ch : chOut){
            ch->Randomize();
            ch->BuildBdBox();
            //std::cout<<"aux="<<ch->aux<< " vers="<<ch->numVerts<<std::endl;
        }
        /////////
#endif       
        return Res;
    }

    FBdBox ReadLasFile( void *pData, std::size_t sz_,int &numPt,std::vector<std::shared_ptr<Chunk>> &chOut,LasInfo &Info){
        std::cout<<"=== READING LAS ==="<<std::endl;
        FBdBox retBox;
        //auto ch = std::make_shared<Chunk>();

        static float intems[256*256];
    
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
        /*
        lh->xOffset = 0.0f;
        lh->yOffset = 0.0f;
        lh->zOffset = 0.0f;
        lh->xScale = 0.0001f;
        lh->yScale = 0.0001f;
        lh->zScale = 0.0001f;
        */
        std::cout<<"version:"<<vMajor<<"."<<vMinor<<std::endl;
        std::cout<<"numpoints="<<numPoints<<" ofst="<<lh->pointOfst<<" ptFormat="<<ptFormat<<" rlen="<<recLength<<std::endl;

        // Find bounding box and Intensity Histogram.
        memset(intems, 0, 256*256*sizeof(float));
        float min, max;
        unsigned char *pStart = (unsigned char*)pData + lh->pointOfst;
        retBox = FillBdBox(pStart, numPoints,lh,intems,chOut);
        //ProcessIntens( intems, 255 , min, max);
        std::cout<<"first = "<<min<<" last="<<max<<std::endl;
        if( max > min){
            //ApplyIntems(pRet,numPoints,min, max);
           // ApplyIntems(chOut,min, max);
        }
        numPt = numPoints;
        Info.vMajor = vMajor;
        Info.vMinor = vMinor;
        Info.numPoints = numPoints;
        Info.vertType =  ptFormat;
        return retBox; 
    }

    static void Sphere(PointFormat3* pt, int num, float rad, int x, int y, int z )
    {
        srand(12345);
        for( int i = 0; i<num ; i++){
            float rx = -0.499f + (double)rand()/(double)RAND_MAX;
            float ry = -0.499f + (double)rand()/(double)RAND_MAX;
            float rz = -0.499f + (double)rand()/(double)RAND_MAX;
            float flen = rad/sqrt(rx*rx + ry*ry + rz*rz);
            pt[i].x = ((float)x + rx*flen ) *100;
            pt[i].y = ((float)y + ry*flen ) *100;
            pt[i].z = ((float)z + rz*flen ) *100;
            pt[i].intensity= (uint16_t)((rz+ 0.5f) * 255.0f);
            pt[i].red = 255;
            pt[i].green = 255;
            pt[i].blue = 255;
        }
    }

    uint8_t* GenerateSampleLas(){
        uint32_t sx = 32, sy = 32, sfPoints = 1024*16;
        uint32_t totSize = sizeof(LasHeader) + (sx*sy*sfPoints)*sizeof(PointFormat3);
        uint8_t *pD = new uint8_t[totSize];
        LasHeader *pLh = (LasHeader*)pD;
        memset(pLh,0,sizeof(LasHeader));
        const char *magic = "LASF";
        memcpy(pLh->magic,magic,4) ;
        pLh->verMajor = 1;
        pLh->verMinor = 4;
        pLh->pointDataFormatId = 3;
        pLh->poitDataRecordLength = sizeof(PointFormat3);
        pLh->pointOfst = sizeof(LasHeader);
        pLh->numOfPointRecords14 = sx*sy*sfPoints;
        pLh->xScale = 10.0f;
        pLh->yScale = 10.0f;
        pLh->zScale = 10.0f;
        pLh->xOffset = 0.0f;
        pLh->yOffset = 0.0f;
        pLh->zOffset = 0.0f;
        PointFormat3 *pt = (PointFormat3*)(pD+sizeof(LasHeader));
        for( int y = 0; y<sy; y++){
            for( int x = 0; x<sx; x++){
                Sphere(pt, sfPoints, 0.4f, x, y, 0);
                pt += sfPoints;  
             }
        }
        return pD;
    }

    struct LasBuilderImpl: public LasBuilder{
        enum{
            RD_NONE = 0,
            RD_SKIP,
            RD_HEARED,
            RD_VERTS
        };
        static const uint32_t sz16 = 256*256;
        uint32_t m_state;
        LasHeader m_hdr;
        uint32_t  m_numPoints;
        uint32_t  m_procPoints;
        uint32_t  m_rdStep;
        uint32_t  m_numInCurrRead;
        float intHist[sz16];
        std::function<int (uint32_t n )> m_allocFunc;
        std::function<FPoint4 *()> m_getVertsFunc;
        std::function<void( const std::string &msg)> m_onErrFunc;

        LasBuilderImpl(){
            Reset();
        }

        void Reset(){
           std::cout<<"LasBuilder:Reset"<<std::endl;
           m_state= RD_NONE; 
           m_numPoints = 0;
           m_procPoints = 0;
           m_rdStep = 1000000;
           m_numInCurrRead = 0;
           m_allocFunc = NULL;
           m_getVertsFunc = NULL;
           m_onErrFunc = NULL;
           memset(intHist,0,sz16*sizeof(float));
        }
        void RegisterCallbacks(
            std::function<int (uint32_t n )> alloc,
            std::function<FPoint4 *()> getVerts,
            std::function<void( const std::string &msg)> onErr
        ){
            m_allocFunc = alloc;
            m_getVertsFunc = getVerts;
            m_onErrFunc = onErr;
        }

        uint32_t SetChunkData(void *pData){
            std::cout<<"@@@@ SetChunkData state="<<m_state<<std::endl;
            if(m_state == RD_NONE){
                m_state = RD_HEARED;
                return sizeof(LasHeader);
            }
            if(m_state == RD_HEARED){
                SetHeader(pData);
                if(m_hdr.pointOfst > sizeof(LasHeader)){
                    m_state = RD_SKIP;
                    return m_hdr.pointOfst - sizeof(LasHeader);
                }
                else{
                    m_state = RD_VERTS;
                    m_numInCurrRead = GetNextVets();
                    return m_numInCurrRead * m_hdr.poitDataRecordLength;
                }
            }
            if(m_state == RD_SKIP){
                m_state = RD_VERTS;
                m_numInCurrRead = GetNextVets();
                return m_numInCurrRead * m_hdr.poitDataRecordLength;
            }
            if(m_state == RD_VERTS){
                SetVerts(pData, m_numInCurrRead);
                m_numInCurrRead = GetNextVets();
                return  m_numInCurrRead * m_hdr.poitDataRecordLength;
            }
            return 0;
        }

        uint32_t GetNextVets(){
           uint32_t nextV = m_procPoints + m_rdStep;
            if(nextV<=m_numPoints){
                return  m_rdStep;
            }else{
                std::cout<<"@@@ Last"<< m_numPoints - m_procPoints<<std::endl;
                return m_numPoints - m_procPoints;
            }
        }
       
        uint32_t SetHeader( void *pHdr ){
            memcpy(&m_hdr,pHdr, sizeof(LasHeader));
            char magic[5] = {0,0,0,0,0};
            memcpy(magic,m_hdr.magic,4);
            if(strcmp(magic,"LASF")){
                if(m_onErrFunc) m_onErrFunc("Can not detect LASF");
                return  0;
            }           
            int vMajor = (int)m_hdr.verMajor;
            int vMinor = (int)m_hdr.verMinor;
            int ptFormat = (int)m_hdr.pointDataFormatId;
            int recLength = (int)m_hdr.poitDataRecordLength;
            if((vMajor==1)&&(vMinor==1)){
                m_numPoints = m_hdr.numOfPointRecords12;
            }
            if((vMajor==1)&&(vMinor==2)){
                m_numPoints = m_hdr.numOfPointRecords12;
            }
            if((vMajor==1)&&(vMinor==3)){
                m_numPoints = m_hdr.numOfPointRecords12;
            }
            if((vMajor==1)&&(vMinor==4)){
                m_numPoints = m_hdr.numOfPointRecords14;
            }
            if(m_numPoints == 0){
                if(m_onErrFunc) m_onErrFunc("Can not detect LAS version");
            }
            std::cout<<"=== LAS === "<<vMajor<<"."<<vMinor<<" points="<<m_numPoints<<std::endl;
            if( m_allocFunc){
                m_allocFunc(m_numPoints);
            }          
            return 0;
        }

        int32_t SetVerts(void *pSrc, uint32_t numInSrc){            
            FPoint4 *pDst = m_getVertsFunc();
            uint8_t *pS  = (uint8_t*)pSrc;
            for( int i = m_procPoints; i< m_procPoints + numInSrc;i++,pS+=m_hdr.poitDataRecordLength){
                PointFormat1 *pf1 = (PointFormat1*)pS;
                pDst[i].x = (float)(pf1->x )*float(m_hdr.xScale) + m_hdr.xOffset;
                pDst[i].y = (float)(pf1->y )*float(m_hdr.yScale) + m_hdr.yOffset;
                pDst[i].z = (float)(pf1->z )*float(m_hdr.zScale) + m_hdr.zOffset; 
                pDst[i].col = pf1->intensity;
                intHist[pf1->intensity] += 1.0f;              
            } 
            m_procPoints+=numInSrc;
            if(m_procPoints == m_numPoints){
                std::cout<<"Processed------> "<<m_procPoints <<" from"<<m_numPoints<<std::endl;
                PostProcessColors();
            }
            return 0;
        }

        void PostProcessColors(){
            for( int i =1; i<sz16; i++){
                intHist[i] +=  intHist[i-1];
            }
            for( int i =0; i<sz16; i++){
                if( intHist[sz16-1]>0.f){
                    intHist[i] *= (255.0f/intHist[sz16-1]);
                }
            } 
            FPoint4 *pDst = m_getVertsFunc();
            for(int i=0; i<m_numPoints; i++){
                uint16_t intensity  = pDst[i].col;
                pDst[i].col  = intHist[intensity];
            }
        }
   };//struct LasBuilderImpl

    LasBuilder * LasBuilder::Get(){
        static LasBuilder *pRet  = new LasBuilderImpl();
        return pRet;
    }


 }//namespace ezp