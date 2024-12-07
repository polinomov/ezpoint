
#include <iostream>
#include "readers.h"

namespace ezp 
{
	struct XyzTxtImpl: public PointBuilder{
		uint32_t m_fSize;
		uint32_t m_numPoints;
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
    
		bool processLine(int lstart, int lend,const uint8_t *pT){
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
	    
			uint8_t tmp[256];
			int cnt = 0;
			for(int i = firstDigit; i<=lend; i++){
				if(( pT[i] == ' ') || (pT[i] == 0x9) || (pT[i] == ',') || (i==lend)){
					if(cnt >0){
						tmp[cnt] = 0;
						float val;
						int len;
						int ret = sscanf((char*)tmp, "%f %n", &val, &len);
						//float val = atof((char*)tmp);
          	std::cout<<"val="<<val<<" len="<<len<<" cnt="<<cnt<<"; ";
						cnt = 0;
					}
					continue;
				}
				tmp[cnt] = pT[i];
				if(cnt<254){
					cnt++; 
				}
				//std::cout<<pT[i];
			}
			std::cout<<std::endl;
			return true;
		}

    void processData( const uint8_t *pTxt, int step){
			int i = 0;
			int lstart = -1;
			int lend = -1;
      while(i < m_fSize){
				if((pTxt[i]==0xD)||(pTxt[i]==0xA) ){
					if((lstart != -1)&&(lend > lstart)){
						if( processLine(lstart, lend, pTxt) ){
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
				std::cout<<"SetChunkData "<<m_fSize<<std::endl;
				return m_fSize;
			}
			processData((uint8_t*)pData, 0);
			LasInfo inf;  
			inf.numPoints = m_numPoints;
			inf.hasRgb = 1;
			inf.hasClass  = 0;
			inf.description = "XYZ " + std::to_string(inf.numPoints ) + " points";
			if(m_onInfoFunc(inf) != 0){
				return -1;
			}        		
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