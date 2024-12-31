
#include <iostream>
#include "readers.h"

namespace ezp 
{
	struct XyzTxtImpl: public PointBuilder{
		uint32_t m_fSize;
		uint32_t m_numPoints;
		uint32_t m_numProc;
		uint32_t m_allocNdx;
		std::function<int (uint32_t n )> m_allocFunc;
		std::function<const FPoint4 *(uint32_t)> m_getVertsFunc;
		std::function<void( const std::string &msg)> m_onErrFunc;
		std::function<int( const LasInfo &info)> m_onInfoFunc;

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
    // Not 100% correct , but faster
    bool fastAtof(const uint8_t *str,float &res){
			const int maxd = 16;
			float sign = 1.0f;
			const uint8_t*pSrc = str;
			if(str[0] == '-'){
				sign = -1.0f;
				pSrc++;
			}
			int64_t parts[3]  = {0,0,0};
			uint8_t partN = 0;
			float prd = 1.0f;
			int32_t exp_sign = 1;
			for(const uint8_t *p = pSrc; *p; p++){
				if((partN == 2)&&(*p =='-')){
					exp_sign = -1;
					continue;
				}
				int64_t v = (int64_t)(*p) - 0x30;
				if((v>=0)&&(v<=9)){
					parts[partN] = v + parts[partN]*10;
					if(partN == 1) prd *= 0.1f;
				}
				else if(*p=='.'){
					partN = 1;
				}
				else if( (*p=='e')||(*p=='E')){
					partN = 2;
				}
			}
			res = ((float)parts[0] + (float)parts[1] * prd) * pow(10,parts[2]*exp_sign);
			res*=sign;
			return true;
		}

		bool processLine(int lstart, int lend,const uint8_t *pT,int step){
			int firstDigit = -1;
			for(int i = lstart; i<=lend; i++){
				if(( pT[i] == ' ') || (pT[i] == 0x9)){
					continue;
				}
				if( (!isdigit(pT[i])) && (pT[i] !='-') ){
					return false;
				}
				firstDigit = i;
				break;			
			}

			if(firstDigit==-1){
				return false;
			}
	    if(step == 0){
				return true;
			}
			// step = 1
			uint8_t tmp[256];
			int cnt = 0,fcnt = 0;
			FPoint4 *pDst = (FPoint4*)m_getVertsFunc(m_allocNdx);
			float xyz[3];
			for(int i = firstDigit; i<=lend; i++){
				bool isDelimiter  = (( pT[i] == ' ') || (pT[i] == 0x9) || (pT[i] == ','));
				if(isDelimiter  || (i==lend)){
					if((i==lend) && (!isDelimiter)){
						tmp[cnt++] = pT[i];
					}
					if(cnt >0){
						tmp[cnt] = 0; 
						float val,res;
						int len;
						//sscanf((char*)tmp, "%f %n", &val, &len);
						fastAtof(tmp,val);
						if(fcnt<3){
							xyz[fcnt++] = val;
						}
						cnt = 0;
					}
					continue;
				}
				tmp[cnt] = pT[i];
				if(cnt<254){
					cnt++; 
				}
			}
			if(m_numProc < m_numPoints){
				pDst[m_numProc].x = xyz[0];
				pDst[m_numProc].y = xyz[1];
				pDst[m_numProc].z = xyz[2]; 
				pDst[m_numProc].col = 0xFFFFFFFF;
			}
			/*
			{
				static float xmin;
			  static float xmax;
				if(m_numProc==0){
					xmin = xyz[0];
					xmax = xyz[0];
			  } else{
					if(xyz[0]<xmin ){
						xmin = xyz[0];
						std::cout<<xmin<<"!"<<xmax<<std::endl;
					}
					if(xyz[0]>xmax ){
						xmax = xyz[0];
						std::cout<<xmin<<"!"<<xmax<<std::endl;
					}
				}
			}
			*/
		//	std::cout<<xyz[0]<<","<<xyz[1]<<","<<xyz[2]<<std::endl;
      m_numProc++;
			return true;
		}

    void processData( const uint8_t *pTxt, int step){
			int i = 0;
			int lstart = -1;
			int lend = -1;
      while(i < m_fSize){
				if((pTxt[i]==0xD)||(pTxt[i]==0xA) ){
					if((lstart != -1)&&(lend > lstart)){
						if( processLine(lstart, lend, pTxt,step) ){
							if(step == 0){
								m_numPoints++;
							}
						}
					}
					i++;
					lstart = lend = -1;
					continue;
				}
				if(lstart==-1){
					lstart = i;
				}
				lend = i;
				i++;
			}
		}

		uint32_t SetChunkData(void *pData){
			if(pData == NULL){
				return m_fSize;
			}
			m_numPoints = 0;
			m_numProc = 0;
			//std::cout<<"process0-A"<<std::endl;
			processData((uint8_t*)pData, 0);
			//std::cout<<"process0-B"<<std::endl;
			LasInfo inf;  
			inf.numPoints = m_numPoints;
			inf.hasRgb = 0;
			inf.hasClass  = 0;
			inf.description = "XYZ ";
			if(m_onInfoFunc(inf) != 0){
				return -1;
			}  
			if(m_numPoints==0){
				return 0;
			}
			m_allocNdx = m_allocFunc(m_numPoints);
			std::cout<<"process1"<<std::endl;
			processData((uint8_t*)pData, 1);      		
			return 0;
		}

    void Reset(uint32_t fSize){
			m_fSize = fSize;
			m_numPoints = 0;
		}

    void PostProcessAllColors(
      uint32_t numMemBanks,
      bool hasRgb,
      std::function<const FPoint4 *(uint32_t ndx)> getVertsFunc,
      std::function<uint32_t (uint32_t ndx)> getNumFunc){
	//		if(numMemBanks==0){
				//return;
	//		}
			FPoint4 *p0 = (FPoint4*)getVertsFunc(0);
			float xMin = p0->x;
			float yMin = p0->y;
			float zMin = p0->z;
			float xMax = p0->x;
			float yMax = p0->y;
			float zMax = p0->z;
			for(uint32_t m = 0; m<numMemBanks; m++){
				FPoint4 *pt = (FPoint4*)getVertsFunc(m);
				for( uint32_t v = 0; v < getNumFunc(m); v++){
					xMin = std::min(xMin,pt[v].x);
					yMin = std::min(yMin,pt[v].y);
					zMin = std::min(zMin,pt[v].z);
					xMax = std::max(xMax,pt[v].x);
					yMax = std::max(yMax,pt[v].y);
					zMax = std::max(zMax,pt[v].z);
				}
			}			
			//std::cout<<xMin<<" "<<xMax<<"#"<<yMin<<" "<<yMax<<"#"<<zMin<<" "<<zMax<<std::endl;
			float sx = xMax - xMin;
			float sy = yMax - yMin;
			float sz = zMax - zMin;
			float smax = std::max(sx,sy);
			smax = std::max(smax,sz);
      float prd = (smax>0.0f) ? 1024.0f/smax : 1.0f;

			for(uint32_t m = 0; m<numMemBanks; m++){
				FPoint4 *pt = (FPoint4*)getVertsFunc(m);
				for( uint32_t v = 0; v < getNumFunc(m); v++){
					pt[v].col = (COLOR05(5,5,5))*256;
				}
			}						
		}

	};//struct XyzTxtImpl

	PointBuilder * PointBuilder::GetXyzTxtBuilder(){
		static PointBuilder *pRet  = new XyzTxtImpl();
		return pRet;
	}

}//namespace ezp 