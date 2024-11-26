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
  struct PointFormat2
  {
    int x;
    int y;
    int z;
    uint16_t intensity;
    uint8_t pad8;
    uint8_t classification;
    uint8_t scanAngle;
    uint8_t usrData;
    uint16_t pointSourceId;
    uint16_t red;
    uint16_t green;
    uint16_t blue;   
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
    if((ptf==7)||(ptf==3)||(ptf==2)) return true;
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
   if(ptType==2){
      PointFormat2 *p2 = (PointFormat2*)pt;
      r = p2->red;
      g = p2->green;
      b = p2->blue;
    }
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
    uint32_t  m_allocNdx;
    float intHist[sz16];
    bool m_hasClass;
    bool m_hasRgb;
    float m_hMin;
    float m_hMax;
    uint16_t *m_R16,*m_G16,*m_B16;
    int m_Iminx,m_Imaxx,m_Iminy,m_Imaxy,m_Iminz,m_Imaxz;
    uint16_t m_rMin,m_rMax,m_gMin, m_gMax,m_bMin, m_bMax;
    std::function<int (uint32_t n )> m_allocFunc;
    std::function<const FPoint4 *(uint32_t)> m_getVertsFunc;
    std::function<void( const std::string &msg)> m_onErrFunc;
    std::function<int( const LasInfo &info)> m_onInfoFunc;

    LasBuilderImpl(){
      m_R16 = m_G16 = m_B16 = NULL;
      Reset();
    }

    void Reset(){
      m_state= RD_NONE; 
      m_numPoints = 0;
      m_procPoints = 0;
      m_rdStep = 1000000;
      m_numInCurrRead = 0;
      m_allocFunc = NULL;
      m_getVertsFunc = NULL;
      m_onErrFunc = NULL;
      m_onInfoFunc = NULL;
      m_allocNdx = 0;
      memset(intHist,0,sz16*sizeof(float));
      m_hasClass = false;
      m_hasRgb = false;
      m_hMin = std::numeric_limits<float>::max();
      m_hMax = std::numeric_limits<float>::min();
      m_Iminx  = std::numeric_limits<int>::max();
      m_Iminy  = std::numeric_limits<int>::max();
      m_Iminz  = std::numeric_limits<int>::max();
      m_Imaxx  = std::numeric_limits<int>::min();
      m_Imaxy  = std::numeric_limits<int>::min();
      m_Imaxz  = std::numeric_limits<int>::min();
      m_rMin = m_gMin= m_bMin = 0xFFFF;
      m_rMax = m_gMax= m_bMax = 0;
      if(m_R16) delete [] m_R16;
      if(m_G16) delete [] m_G16;
      if(m_B16) delete [] m_B16;
      m_R16 = m_G16 = m_B16 = NULL;
    }

    void RegisterCallbacks(
      std::function<int (uint32_t n )> alloc,
      std::function<const FPoint4 *(uint32_t ndx)> getVerts,
      std::function<void( const std::string &msg)> onErr,
      std::function<int( const LasInfo &info)> onInfo
    ){
      m_allocFunc = alloc;
      m_getVertsFunc = getVerts;
      m_onErrFunc = onErr;
      m_onInfoFunc = onInfo;
    }

    uint32_t SetChunkData(void *pData){
      if(m_state == RD_NONE){
        m_state = RD_HEARED;
        return sizeof(LasHeader);
      }
      if(m_state == RD_HEARED){
        bool isGood = SetHeader(pData);
        if(!isGood){
           return -1;
        }
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
    
    bool SetHeader( void *pHdr ){
      memcpy(&m_hdr,pHdr, sizeof(LasHeader));
      char magic[5] = {0,0,0,0,0};
      memcpy(magic,m_hdr.magic,4);
      if(strcmp(magic,"LASF")){
        if(m_onErrFunc) m_onErrFunc("Can not detect LASF");
        return  true;
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
      if( m_allocFunc){
       // m_allocNdx = m_allocFunc(m_numPoints);
      }   
      m_hasClass = ptHasClass((int)m_hdr.pointDataFormatId); 
      m_hasRgb =  ptHasColor((int)m_hdr.pointDataFormatId);
      //std::cout<<"=== LAS === verttype "<< (int)m_hdr.pointDataFormatId<<std::endl;
      //std::cout<<"=== LAS === "<<vMajor<<"."<<vMinor<<" points="<<m_numPoints<<" classs"<<m_hasClass<< " rgb"<< m_hasRgb<<std::endl; 
      LasInfo inf;  
      inf.numPoints = m_numPoints;
      inf.hasRgb = m_hasRgb;
      inf.hasClass  = m_hasClass;
      if(m_onInfoFunc(inf) != 0){
        return false;
      }      
      m_allocNdx = m_allocFunc(m_numPoints);
      if(m_hasRgb){
        m_R16 = new uint16_t[m_numPoints];
        m_G16 = new uint16_t[m_numPoints];
        m_B16 = new uint16_t[m_numPoints];
      }
      return true;
    }

    int32_t SetVerts(void *pSrc, uint32_t numInSrc){            
      FPoint4 *pDst = (FPoint4*)m_getVertsFunc(m_allocNdx);
      uint8_t *pS  = (uint8_t*)pSrc;
      for( int i = m_procPoints; i< m_procPoints + numInSrc;i++,pS+=m_hdr.poitDataRecordLength){
        PointFormat1 *pf1 = (PointFormat1*)pS;
        pDst[i].x = (float)(pf1->x )*float(m_hdr.xScale) + m_hdr.xOffset;
        pDst[i].y = (float)(pf1->y )*float(m_hdr.yScale) + m_hdr.yOffset;
        pDst[i].z = (float)(pf1->z )*float(m_hdr.zScale) + m_hdr.zOffset; 
        m_Iminx  = std::min(m_Iminx,pf1->x);
        m_Iminy  = std::min(m_Iminy,pf1->y);
        m_Iminz  = std::min(m_Iminz,pf1->z);
        m_Imaxx  = std::max(m_Imaxx,pf1->x);
        m_Imaxy  = std::max(m_Imaxy,pf1->y);
        m_Imaxz  = std::max(m_Imaxz,pf1->z);
        m_hMin = std::min(m_hMin,pDst[i].z);
        m_hMax = std::max(m_hMax,pDst[i].z);
        pDst[i].col = pf1->intensity;
        if(m_hasClass){
          uint8_t cls = (uint8_t)ptGetClass((int)m_hdr.pointDataFormatId,pS); 
          pDst[i].col |= (cls<<16);
        }
        if(m_hasRgb){
          uint16_t r16,g16,b16;
          ptGetColor(r16,g16,b16, (int)m_hdr.pointDataFormatId,pf1);
          m_rMin = std::min(m_rMin,r16);
          m_gMin = std::min(m_gMin,g16);
          m_bMin = std::min(m_bMin,b16);
          m_rMax = std::max(m_rMax,r16);
          m_gMax = std::max(m_gMax,g16);
          m_bMax = std::max(m_bMax,b16);
          m_R16[i]  = r16;
          m_G16[i]  = g16;
          m_B16[i]  = b16;
        }
        intHist[pf1->intensity] += 1.0f;              
      } 
      m_procPoints+=numInSrc;
      if(m_procPoints == m_numPoints){
       // std::cout<<"Start Processing"<<std::endl;
        std::cout<<m_Iminx<<" "<<m_Imaxx<<"|";
        std::cout<<m_Iminy<<" "<<m_Imaxy<<"|";
        std::cout<<m_Iminz<<" "<<m_Imaxz<<std::endl;
        PostProcessColors();
      }
      return 0;
    }

    void PostProcessColors(){
      int shift_rgb = 3;
      if( ((m_rMax - m_rMin)>256) || ((m_gMax - m_gMin))>256  || ((m_bMax - m_bMin))>256 ){
        shift_rgb  = 11;
      }
      for( int i =1; i<sz16; i++){
        intHist[i] +=  intHist[i-1];
      }
      for( int i =0; i<sz16; i++){
        if( intHist[sz16-1]>0.f){
          intHist[i] *= (255.0f/intHist[sz16-1]);
        }
      } 
      FPoint4 *pDst = (FPoint4*) m_getVertsFunc(m_allocNdx);
      // Hmap
      const uint32_t hsz = 0xFFF;
      float hHist[hsz];
      memset(hHist, 0, hsz*sizeof(float));
      float prd = (float)hsz-1.0f;
      float hDiff  = prd/(m_hMax - m_hMin);
      for(int i=0; i<m_numPoints; i++){
        uint32_t h8 = (uint32_t)((pDst[i].z - m_hMin) *  hDiff);
        h8 &= 0xFFF;
        hHist[h8] += 1.0f;
      }
      for( int i =1; i<hsz; i++){
        hHist[i] +=hHist[i-1];
      }
      for( int i = 0; i<hsz; i++){
        if(hHist[hsz-1]>0.0f){
          hHist[i] *= (prd/hHist[hsz-1]);
        }
      }
      // Colors
      for(int i=0; i<m_numPoints; i++){
        uint16_t intensity  = pDst[i].col & 0xFFFF;
        uint8_t cls = 0;
        uint32_t hcol = 0;
        if(m_hasClass){
          cls = (pDst[i].col >>16) & 0xFF;
        }
        uint32_t h8 = (uint32_t)((pDst[i].z -  m_hMin) *  hDiff);
        hcol = (uint32_t)hHist[h8 & hsz];
        if(m_hasRgb){
          uint32_t rr = ((m_R16[i]>>shift_rgb) & 0x1f)<<10;
          uint32_t gg = ((m_G16[i]>>shift_rgb) & 0x1f)<<5;
          uint32_t bb = ((m_B16[i]>>shift_rgb) & 0x1f);
          pDst[i].col = rr | gg | bb;
        }else{
          pDst[i].col  = (uint8_t)intHist[intensity];
          pDst[i].col |= (cls<<8);
        }
        pDst[i].col |= (hcol<<16);
      }
    }
   };//struct LasBuilderImpl

  LasBuilder * LasBuilder::Get(){
    static LasBuilder *pRet  = new LasBuilderImpl();
    return pRet;
  }

 }//namespace ezp