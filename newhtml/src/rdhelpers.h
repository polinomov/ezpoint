
#ifndef RDHELPERS_H_
#define RDHELPERS_H_
#include <cstdint>

void RenderPoint(uint32_t *pBuff, int32_t sw, int32_t sh, int32_t px, int32_t py );
void RenderString( const std::string &st,int x, int y,uint32_t *pBuff,  int32_t sw, int32_t sh);

#endif
 