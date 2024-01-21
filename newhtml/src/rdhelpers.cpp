

#include <stdlib.h>
#include <algorithm>
#include "rdhelpers.h"

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
                pBuff[x + y*sw] =  0x00FF00FF;
           }
    }
 }
 