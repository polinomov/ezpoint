//https://github.com/technosf/Raster-Font/blob/master/main/fonts/font_terminus_bold_14x28_iso8859_1.h

#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <string.h>
#include "ezpoint.h"
#include "rdhelpers.h"
#include "fonts.h"

namespace ezp {
static const int fntH = 14;

void RenderPoint(uint32_t *pBuff,  int32_t sw, int32_t sh, int32_t px, int32_t py ){
  int32_t left = px-5;
  int32_t right = px+5;
  left = std::max(left, 0);
  right = std::min(right, sw);
  int32_t top = py-5;
  int32_t down = py + 5;
  top = std::max(top, 0);
  down = std::min(down,sh);
  if(( left>= right)||(top>=down)) return;
  for( uint32_t y = top; y < down; y++){
    for( uint32_t x = left; x < right; x++){
      pBuff[x + y*sw] =  0xFFFFFFFF;
    }
  }
}

static uint32_t * getLine(uint8_t lt, uint8_t n){
  static uint32_t pRet[8];
  const uint8_t *pLn =  _fonts_terminus_8x14_iso8859_1_bitmaps + lt*fntH + n;
  for( int i = 0; i<8;i++){
    uint8_t msk = 1<<(7-i);
    pRet[i] = (pLn[0] & msk) ? 0x0 : 0xFFFFFFFF;
  }
  return pRet;
}

void RenderString( const std::string &st,int x, int y,uint32_t *pBuff,  int32_t sw, int32_t sh){
  int left = x;
  for( int t = 0 ; t<st.length();t++){
    uint8_t lt = st[t]-1;
    for(int iy = y,k = 0; iy<fntH+y; iy++,k++){              
      uint32_t *p32 = getLine(lt, k);
      for(int m = 0; m<8;m++){
        pBuff[left+m + iy*sw] =  p32[m];
      }
    }
    left+=8;
  }
}

struct Line{
   FPoint4 m_pt[2];
   bool m_isDone;
   bool m_isActive;
   float m_dist;
};

static float GetDist(const FPoint4 &a, const FPoint4 &b){
  float dx = a.x - b.x;
  float dy = a.y - b.y;
  float dz = a.z - b.z;
  return sqrt( dx*dx + dy*dy + dz*dz);
}

struct RenderHelperIml: public RenderHelper{
  static const uint32_t m_ptSz = 11;
  static const uint32_t m_maxLines = 32;
  uint32_t m_ptImage[m_ptSz*m_ptSz];
  uint32_t m_mouseX;
  uint32_t m_mouseY;
  bool m_hasPoint;
  FPoint4 m_currPoint;
  Line m_lines[m_maxLines];
  uint32_t m_selNdx;
 
  void Init(){
    uint32_t h2 = m_ptSz/2;
    //point shape
    for( uint32_t iy = 0; iy<m_ptSz; iy++){
      for( uint32_t ix = 0; ix<m_ptSz; ix++){
        uint32_t dx = (ix>h2) ? ix-h2 : h2-ix;
        uint32_t dy = (iy>h2) ? iy-h2 : h2-iy;
        uint32_t dd = dx*dx + dy*dy;
        uint32_t ad = ix + iy*m_ptSz;
        m_ptImage[ix + iy*m_ptSz] = (dd>h2*h2)? 0x0:0xFFFF0000;
        if(dd<4)  m_ptImage[ix + iy*m_ptSz] = 0xFFFFFFF0;
      }
    }
    m_hasPoint = false;
    m_selNdx = -1;
  }

  void Reset(){
    m_hasPoint = false;
    for( int i = 0; i<m_maxLines; i++){
      m_lines[i].m_isActive = false;
      m_lines[i].m_isDone = false;
    }
    m_selNdx = -1;
  }

  uint32_t  getClosePoint(uint32_t mx, uint32_t my,uint64_t *pt,int cW,int cH){
    static int sz = 11;
    uint32_t ret = -1;
    float dmin = std::numeric_limits<float>::max();
    int maxAddr = cW*cH;
    float camX,camY,camZ;
    Camera::Get()->GetPos(camX,camY,camZ);
    for( int iy = my-sz/2; iy<my + sz/2;iy++){
      for( int ix = mx-sz/2;  ix<mx + sz/2;ix++){
        uint32_t addr = ix + cW *iy;
        uint64_t ptPtr = -1;
        if((addr>=0) &&(addr<maxAddr)){
          ptPtr = pt[addr];
        }
        if(ptPtr != -1){
          FPoint4 *pT  = (FPoint4*)ptPtr;
          float dx = camX - pT->x;
          float dy = camY - pT->y;
          float dz = camZ - pT->z;
          float dd = dx*dx + dy*dy + dz*dz;
          if(dd<dmin){
            dmin  = dd;
            ret = addr;
          }
        }
      }
    }
    return ret;
  }

  void MouseMove(uint32_t mx, uint32_t my,uint64_t *pt,int cW,int cH){
    if(( mx < cW) && (my< cH))
    {
      m_mouseX = mx;
      m_mouseY = my;
      uint32_t addr = getClosePoint(mx, my, pt, cW,cH);
      if(addr != -1){
        uint64_t ptPtr = pt[addr];
        FPoint4 *pT  = (FPoint4*)ptPtr;
        m_currPoint.x = pT->x;
        m_currPoint.y = pT->y;
        m_currPoint.z = pT->z;
        m_hasPoint = true;
      }
      else{
         m_hasPoint = false;
      }
    }
  }

  void OnSelectPoint(){
    //std::cout<<"====SELECT===="<<std::endl;
    if(!m_hasPoint){
      return;
    }
    if(m_selNdx==-1){
      int ndx = -1;
      for( int i = 0; i<m_maxLines; i++){
        if(m_lines[i].m_isActive==false){
          ndx = i;
          break;
        }
      }
      if(ndx !=-1){
        m_lines[ndx].m_pt[0] = m_currPoint;
        m_lines[ndx].m_isDone = false;
        m_lines[ndx].m_isActive = true;
        m_selNdx = ndx;
      }
    }else{
      m_lines[m_selNdx].m_pt[1] = m_currPoint;
      m_lines[m_selNdx].m_isDone = true;
      m_lines[m_selNdx].m_isActive = true;
      m_selNdx = -1;
      //m_lines[m_selNdx].m_dist
    }
  }
  
  bool DrawLine(uint32_t *pBuff, int cW,int cH,int px0, int py0,int px1, int py1){
    if((px0<0) ||( px0>= cW)) return false;
    if((px1<0) ||( px1>= cW)) return false;
    if((py0<0) ||( py0>= cH)) return false;
    if((py1<0) ||( py1>= cH)) return false;
    float fx0 = (float)px0;
    float fy0 = (float)py0;
    float fx1 = (float)px1;
    float fy1 = (float)py1;
    float fdx = fx1 - fx0;
    float fdy = fy1 - fy0;
    float fdd = fdx*fdx + fdy*fdy;
    if(fdd<1.0f){
      return false;
    }
    for(int t = 0; t<(int)fdd;t++){
      float prd = (float)t/fdd;
      int sx = (int)(prd * fx0 + (1.0f - prd) * fx1);
      int sy = (int)(prd * fy0 + (1.0f - prd) * fy1);
      uint32_t addr = sx + cW *sy;
      pBuff[addr] = 0xFFFFFFFF;
    }
    return true;
  }

  void DrawPoint(uint32_t *pBuff, int cW,int cH,int px, int py){
    for( int iy = 0; iy<m_ptSz; iy++){
      for( int ix = 0; ix<m_ptSz; ix++){
        int sx = px + ix - m_ptSz/2;
        int sy = py + iy - m_ptSz/2;
        if((sx>0)&&(sx<cW)&&(sy>0)&&(sy<cH)){
          uint32_t addr = sx + cW *sy;
          if(m_ptImage[ix + iy*m_ptSz] != 0){
            pBuff[addr] =  m_ptImage[ix + iy*m_ptSz];
          }
        }
      }      
    }
  }

  bool ProjectPoint(const FPoint4 &pt,int winW,int winH,int &pixX, int &pixY){
    float d,u,r;
    Camera::Get()->Project(pt.x,pt.y,pt.z,d,u,r);
    if(d>0.00001f){
      float atanr = Renderer::Get()->GetAtanRatio();
      float pixSize = 0.5f * (float)std::min(winW,winH);
      float prd = atanr* pixSize;
      pixX = (int) ((float)winW*0.5f + prd* r/d);
      pixY = (int) ((float)winH*0.5f + prd* u/d);
      return true;  
    }  
    return false;                         
  }

  void Render(unsigned int *pBuff, int cW,int cH,int winW,int winH){
    if(m_hasPoint){
      DrawPoint(pBuff,  cW, cH, m_mouseX, m_mouseY);
    }
    for( int i = 0; i<m_maxLines; i++){
      if(m_lines[i].m_isActive){
        int pixX =-1, pixY = -1;
        if(ProjectPoint(m_lines[i].m_pt[0],winW,winH,pixX,pixY)){
          DrawPoint(pBuff,  cW, cH, pixX, pixY);
          if(m_lines[i].m_isDone){
            int pixX2 =-1, pixY2 = -1;
            if(ProjectPoint(m_lines[i].m_pt[1],winW,winH,pixX2,pixY2)){
              bool hl = DrawLine(pBuff,cW, cH, pixX2, pixY2,pixX, pixY);
              DrawPoint(pBuff,  cW, cH, pixX2, pixY2);
              int xmm = (pixX + pixX2 )/2;
              int ymm = (pixY + pixY2 )/2;
              float fd  = GetDist(m_lines[i].m_pt[0], m_lines[i].m_pt[1]);
              if(hl) RenderString(std::to_string(fd),xmm,ymm, pBuff,cW,cH);
            }
          }else{
            if(m_hasPoint){
              int xmm = (pixX + m_mouseX )/2;
              int ymm = (pixY + m_mouseY )/2;
              bool hl =  DrawLine(pBuff,cW, cH, m_mouseX, m_mouseY,pixX, pixY);
              float fd  = GetDist(m_lines[i].m_pt[0], m_currPoint);
              if(hl) RenderString(std::to_string(fd),xmm,ymm, pBuff,cW,cH);
            }
          }
        }
      }
    }
    if(m_hasPoint){
      char stmp[1024];
      sprintf(stmp," PRESS M TO SELECT POINT ( x:%f  y:%f  z:%f ) ",m_currPoint.x,m_currPoint.y,m_currPoint.z);
      RenderString(stmp,3,2, pBuff,cW,cH);
      RenderString("PRESS Q TO UNDO",3,20, pBuff,cW,cH);
    }
    //DrawLine(pBuff,cW, cH, 100, 100,200, 250);
  }
  
};

RenderHelper* RenderHelper::Get(){
    static RenderHelperIml theRh;
    return &theRh;
}
}//namespace ezp
 