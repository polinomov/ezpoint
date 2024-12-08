
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
						float val;
						int len;
						sscanf((char*)tmp, "%f %n", &val, &len);
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
			std::cout<<xyz[0]<<","<<xyz[1]<<","<<xyz[2]<<std::endl;
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
			inf.hasRgb = 1;
			inf.hasClass  = 0;
			inf.description = "XYZ " + std::to_string(inf.numPoints ) + " points";
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
      std::function<const FPoint4 *(uint32_t ndx)> getVerts,
      std::function<uint32_t (uint32_t ndx)> getNum){

		}

	};//struct XyzTxtImpl

	PointBuilder * PointBuilder::GetXyzTxtBuilder(){
		static PointBuilder *pRet  = new XyzTxtImpl();
		return pRet;
	}

}//namespace ezp 