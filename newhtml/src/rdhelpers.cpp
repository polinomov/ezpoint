//https://github.com/technosf/Raster-Font/blob/master/main/fonts/font_terminus_bold_14x28_iso8859_1.h

#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include "rdhelpers.h"
#include "fonts.h"

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
 