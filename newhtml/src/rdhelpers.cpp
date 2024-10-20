//https://github.com/technosf/Raster-Font/blob/master/main/fonts/font_terminus_bold_14x28_iso8859_1.h

#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
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
};

struct RenderHelperIml: public RenderHelper{
  static const uint32_t m_ptSz = 11;
  static const uint32_t m_maxLines = 32;
  uint32_t m_ptImage[m_ptSz*m_ptSz];
  uint32_t m_mouseX;
  uint32_t m_mouseY;
  bool m_hasPoint;
  FPoint4 m_currPoint;
  Line m_lines[m_maxLines];
 
  void Init(){
    uint32_t h2 = m_ptSz/2;
    for( uint32_t iy = 0; iy<m_ptSz; iy++){
      for( uint32_t ix = 0; ix<m_ptSz; ix++){
        uint32_t dx = (ix>h2) ? ix-h2 : h2-ix;
        uint32_t dy = (iy>h2) ? iy-h2 : h2-iy;
        uint32_t dd = dx*dx + dy*dy;
        uint32_t ad = ix + iy*m_ptSz;
        m_ptImage[ix + iy*m_ptSz] = (dd>h2*h2)? 0x0:0xFFFF0000;
        if(dd<4)  m_ptImage[ix + iy*m_ptSz] = 0xFFFFFFFF;
      }
    }
    m_hasPoint = false;
  }

  uint32_t  getClosePoint(uint32_t mx, uint32_t my,uint64_t *pt,int cW,int cH){
    static int sz = 7;
    uint32_t ret = -1;
    uint32_t dmin = 0xFFFFFFFF;
    for( int iy = my-sz/2; iy<my + sz/2;iy++){
      for( int ix = mx-sz/2;  ix<mx + sz/2;ix++){
        uint32_t addr = ix + cW *iy;
        uint64_t ptPtr = pt[addr];
        if(ptPtr != -1){
          uint32_t dx = mx - ix;
          uint32_t dy = my - iy;
          uint32_t dd = dx*dx + dy*dy;
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

  void MouseClick(){
    if(m_hasPoint){
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
      }
    }
  }

  void DrawPoint(uint32_t *pBuff, int cW,int cH,int px, int py){
    for( int iy = 0; iy<m_ptSz; iy++){
      for( int ix = 0; ix<m_ptSz; ix++){
        int sx = px + ix - m_ptSz;
        int sy = py + iy - m_ptSz;
        if((sx>0)&&(sx<cW)&&(sy>0)&&(sy<cH)){
          uint32_t addr = sx + cW *sy;
          if(m_ptImage[ix + iy*m_ptSz] != 0){
            pBuff[addr] =  m_ptImage[ix + iy*m_ptSz];
          }
        }
      }      
    }
  }

  void ProjectPoint(const FPoint4 &pt,int winW,int winH,int &pixX, int &pixY){
    float d,u,r;
    Camera::Get()->Project(pt.x,pt.y,pt.z,d,u,r);
    float atanr = Renderer::Get()->GetAtanRatio();
    float pixSize = 0.5f * (float)std::min(winW,winH);
    float prd = atanr* pixSize;
    pixX = (int) ((float)winW*0.5f + prd* r/d);
    pixY = (int) ((float)winH*0.5f + prd* u/d);                             
  }

  void Render(unsigned int *pBuff, int cW,int cH,int winW,int winH){
    RenderString("blah",5,5,pBuff,cW, cH);
    if(m_hasPoint){
      DrawPoint(pBuff,  cW, cH, m_mouseX, m_mouseY);
    }
    for( int i = 0; i<m_maxLines; i++){
      if(m_lines[i].m_isActive){
        int pixX =-1, pixY = -1;
        ProjectPoint(m_lines[i].m_pt[0],winW,winH,pixX,pixY);
        DrawPoint(pBuff,  cW, cH, pixX, pixY);
      }
    }
  }
  
};

RenderHelper* RenderHelper::Get(){
    static RenderHelperIml theRh;
    return &theRh;
}
}//namespace ezp
 