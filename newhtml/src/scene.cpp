#include <stdlib.h>
#include <iostream>
#include <queue>
#include "ezpoint.h"
#include "readers\readers.h"
#include <string>
#include "chunker.tpp"
namespace ezp 
{
	struct SceneImpl : public Scene
	{
		std::vector<Chunk*> m_allChunks;
		FBdBox m_box;
		std::vector<FPoint4*> m_allVerts;
		std::vector<uint32_t> m_numAllVerts;
		uint32_t m_totVerts;
		float m_size;
		std::queue<std::function<void( )>> m_calls;
		std::string m_desc;
		float m_prd;
		float m_xMin,m_yMin,m_zMin;
		bool m_hasRgb;
		uint32_t *m_pclut;
 		uint32_t *m_phclut;
		float m_sfactor;
 	  float m_xmin;
 	  float m_ymin;
 	  float m_zmin;
 
		SceneImpl(){
			m_size = 1.0f;
			m_totVerts = 0;
			m_pclut = new uint32_t[256*256];
		  m_phclut = new uint32_t[256*256];
			BuildClut();
		  BuildHClut();
		}

		uint32_t AllocVerts( uint32_t num){
			m_allVerts.push_back(new FPoint4[num+1]);
			m_numAllVerts.push_back(num);
			m_totVerts+=num;
			return  m_allVerts.size()-1;
		}

		uint32_t GetTotVerts(){
			return m_totVerts;
		}

		uint32_t GetNumMemBanks(){
			return m_allVerts.size();
		}

		void Clear(){
			for( int m = 0; m<m_allChunks.size(); m++) {
				delete m_allChunks[m];
			}
			m_allChunks.clear();
			for( int m = 0; m< m_allVerts.size(); m++) {
				delete m_allVerts[m];
			}
			m_allVerts.clear();
			m_numAllVerts.clear();
			m_box.xMin = m_box.yMin = m_box.zMin = std::numeric_limits<float>::max();
			m_box.xMax = m_box.yMax = m_box.zMax = -std::numeric_limits<float>::max();
			m_totVerts = 0;	
			std::queue<std::function<void( )>> empty;
			std::swap( m_calls, empty );
			m_desc = "";
			m_prd = 1.0f;
			m_xMin=m_yMin=m_zMin= 0.0f;
			m_hasRgb = false;
			m_sfactor = 1.0f;
		}

		const FPoint4 *GetVerts(uint32_t n){
			return m_allVerts[n];
		}

		uint32_t GetNumVerts(uint32_t n){
			return m_numAllVerts[n];
		}
  
		void SetCamera(){
			if(m_allChunks.size() == 0){
				return;
			}
			float pos[3],dim[3];
			dim[0] = m_box.xMax - m_box.xMin;
			dim[1] = m_box.yMax - m_box.yMin;
			dim[2] = m_box.zMax - m_box.zMin;
			int ndx_min= 0;
			if(dim[0] < dim[1]){
				ndx_min = (dim[0] < dim[2])? 0:2;
			}
			else{
				ndx_min = (dim[1] < dim[2])? 1:2;
			}
			Camera *pCam = Camera::Get();
			pos[0]= (m_box.xMax + m_box.xMin) *0.5f;
			pos[1]= (m_box.yMax + m_box.yMin) *0.5f;
			pos[2]= (m_box.zMax + m_box.zMin) *0.5f;
			pCam->SetPivot(pos[0],pos[1],pos[2]);
			float atanr = Renderer::Get()->GetAtanRatio();
			pos[2] += 0.55f*atanr*std::max(dim[0],dim[1]);
			pCam->ReSet();
			pCam->SetPos(pos);
			pCam->SetWorldUpAxis(0.0f,0.0f,1.0f);
			m_size = 0.0f;
			m_size = std::max(m_size,m_box.xMax-m_box.xMin);
			m_size = std::max(m_size,m_box.yMax-m_box.yMin);
			m_size = std::max(m_size,m_box.zMax-m_box.zMin);
		}

		void SetCameraOrto(){}
		
		const std::vector<Chunk*>&  GetChunks() {
			return m_allChunks;
		}

		float GetSize(){
			return m_size;
		}

		FBdBox GetSceneBdBox(){
			return m_box;
		}

		void GetZMax(float &zmin, float &zmax){
			if(m_allChunks.size() == 0){
				zmin= 1.0f;
				zmax = -1.0f;
				return;
			}
			float pP[3],pD[3],x[8],y[8],z[8];
			Camera *pCam = Camera::Get();
			pCam->GetPos(pP[0],pP[1],pP[2]);
			pCam->GetDir(pD[0],pD[1],pD[2]);
			x[0] = m_box.xMin; y[0] = m_box.yMin; z[0] = m_box.zMin;
			x[1] = m_box.xMin; y[1] = m_box.yMax; z[1] = m_box.zMin;
			x[2] = m_box.xMax; y[2] = m_box.yMin; z[2] = m_box.zMin;
			x[3] = m_box.xMax; y[3] = m_box.yMax; z[3] = m_box.zMin;
			x[4] = m_box.xMin; y[4] = m_box.yMin; z[4] = m_box.zMax;
			x[5] = m_box.xMin; y[5] = m_box.yMax; z[5] = m_box.zMax;
			x[6] = m_box.xMax; y[6] = m_box.yMin; z[6] = m_box.zMax;
			x[7] = m_box.xMax; y[7] = m_box.yMax; z[7] = m_box.zMax;

			zmin = zmax = (x[0]-pP[0])*pD[0] + (y[0]-pP[1])*pD[1] +(z[0]-pP[2])*pD[2];
			for( int i =1; i<8; i++){
				float zz =(x[i]-pP[0])*pD[0] + (y[i]-pP[1])*pD[1] +(z[i]-pP[2])*pD[2];
				if(zz > zmax) zmax = zz;
				if(zz < zmin) zmin = zz;
			}
		}

		void onChunk(FPoint4*pt, int num){
			if(num==0){
				std::cout<<"OnChunk @@@@@@@@@@@@@@@@@@@@@ num="<<num<<std::endl;
				return;
			}
			Chunk* chk = new Chunk();
			chk->pVert = (float*)pt;
			chk->numVerts = num;
			chk->BuildBdBox();
			chk->Randomize();
			m_allChunks.push_back(chk);
		}

		void processVertDataInt(uint32_t n){
			const FPoint4* pt = GetVerts(n);
			int num = GetNumVerts(n);
			FBdBox bd = chunker::getBdBox<FPoint4>(pt, 0, num-1);
			chunker::ShowBdBox(bd);          
			m_box.xMin  = std::min(m_box.xMin,bd.xMin);
			m_box.yMin  = std::min(m_box.yMin,bd.yMin);
			m_box.zMin  = std::min(m_box.zMin,bd.zMin);
			m_box.xMax  = std::max(m_box.xMax,bd.xMax);
			m_box.yMax  = std::max(m_box.yMax,bd.yMax);
			m_box.zMax  = std::max(m_box.zMax,bd.zMax);
		  chunker::ShowBdBox(m_box); 
			chunker::doChunks<FPoint4>((FPoint4*)pt, 0, num-1, 4096*4,  [this](FPoint4*pt, int num){this->onChunk(pt,num);} ); 
		}

		 void AddToQueue(std::function<void( )> func){
			m_calls.push(func);
		}

		void OnTick(){
			if(!m_calls.empty()){
				auto func = m_calls.front();
				func();
				m_calls.pop();
			}
		}

		void SetDesctiption( const std::string &decs){
			m_desc = decs;
		}

    std::string GetDesctiption( ){
			return m_desc + " points:" + std::to_string(m_totVerts);
	  }

	  void SetRgbProp(bool prop){
			m_hasRgb = prop;
		}

    bool GetRgbProp(){
			return m_hasRgb;
		}


		void ReScale(){
			if(m_totVerts==0){
				return;
			}
			FPoint4 *p0 = (FPoint4*)GetVerts(0);
			float xMin = p0->x;
			float yMin = p0->y;
			float zMin = p0->z;
			float xMax = p0->x;
			float yMax = p0->y;
			float zMax = p0->z;
			for(uint32_t m = 0; m<GetNumMemBanks(); m++){
				FPoint4 *pt = (FPoint4*)GetVerts(m);
				for( uint32_t v = 0; v < GetNumVerts(m); v++){
					xMin = std::min(xMin,pt[v].x);
					yMin = std::min(yMin,pt[v].y);
					zMin = std::min(zMin,pt[v].z);
					xMax = std::max(xMax,pt[v].x);
					yMax = std::max(yMax,pt[v].y);
					zMax = std::max(zMax,pt[v].z);
				}
			}
			float sx = xMax - xMin;
			float sy = yMax - yMin;
			float sz = zMax - zMin;
			float smax = std::max(sx,sy);
			smax = std::max(smax,sz);
			float scprod = 1.0f;
	    float prd = (smax>0.0f) ? scprod/smax : 1.0f;
			for(uint32_t m = 0; m<GetNumMemBanks(); m++){
				FPoint4 *pt = (FPoint4*)GetVerts(m);
				for( uint32_t v = 0; v < GetNumVerts(m); v++){
					pt[v].x = (pt[v].x - xMin)*prd;
					pt[v].y = (pt[v].y - yMin)*prd;
					pt[v].z = (pt[v].z - zMin)*prd;
				}
			}
			m_sfactor = 1.0f/ prd;
			m_xmin = xMin;
			m_ymin = yMin;
			m_zmin = zMin;
		}
    
		FPoint4 UnScale( const FPoint4 &pt){
			FPoint4 ret;
			ret.x = pt.x * m_sfactor + m_xmin ;
			ret.y = pt.y * m_sfactor + m_ymin;
			ret.z = pt.z * m_sfactor + m_zmin;
			return ret;
		}
  
		uint32_t* BuildClut(){
			static bool hasClut = false;
			float step = (1.0f/5.0f);
			int ndx  = 256;
			for( int r = 0; r<6; r++){
				for( int g = 0; g<6; g++){
					for( int b = 0; b<6; b++){
						float rf = (float)r*step;
						float gf = (float)g*step;
					  float bf = (float)b*step;
						for(int t = 0; t<256; t++){
							float prd = 1.5f * pow(4.0f, -(float)t/255.0f);
							if(prd<0.0f) prd = 0.0f;
							uint32_t ri = (uint32_t) (rf * 255.0f * prd);
							uint32_t gi = (uint32_t) (gf * 255.0f * prd);
							uint32_t bi = (uint32_t) (bf * 255.0f * prd);
							if(ri>255) ri = 255;
							if(gi>255) gi = 255;
							if(bi>255) bi = 255;
							m_pclut[ndx+t] = (ri<<16) | (gi<<8) | (bi);
						}
						ndx+=256;
					}
				}
			}
			// white	
			for(int t = 0; t<256; t++){
				float prd = pow(4.0f, -(float)t/255.0f);
				uint32_t ri =(uint32_t) (prd *255.0f);
				uint32_t gi =(uint32_t) (prd *255.0f);
				uint32_t bi =(uint32_t) (prd *255.0f);
				m_pclut[t] = (ri<<16) | (gi<<8) | (bi);
			}
      return NULL;
		}

		void BuildHClut(){			
			float rf,gf,bf;
			for(int ndx  = 0; ndx<256; ndx++){
				if((ndx>=0) && (ndx<128)){
					float np = (float)ndx/127.0f;
					rf = np*255.0f;
					gf = (1.0f-np) * 255.0f;
					bf = 0.0f;
				}
				if((ndx>=128) && (ndx<256)){
					float np = (float)(ndx-128)/127.0f;
					rf =  255.0f;
					gf =  np * 255.0f;
					bf =  np * 255.0f;;
				}
				for(int t = 0; t<256; t++){
					float prd = pow(4.0f, -(float)t/255.0f);
					uint32_t ri = (uint32_t) (rf * prd);
					if(ri>255) ri = 255;
					uint32_t gi = (uint32_t) (gf * prd);
					if(gi>255) gi = 255;
					uint32_t bi = (uint32_t) (bf * prd);
					if(bi>255) bi = 255;
					m_phclut[t + ndx*256] = (ri<<16) | (gi<<8) | (bi);
				}		
			}
		}

		uint32_t *GetClut(){
			return m_pclut;
		}

		uint32_t *GetHClut(){
			return m_phclut;
		}
		
		void Sphere(FPoint4* pt, int num, float rad, int x, int y, int z,uint16_t col )
		{
			for( int i = 0; i<num ; i++){
				float rx = -0.499f + (double)rand()/(double)RAND_MAX;
				float ry = -0.499f + (double)rand()/(double)RAND_MAX;
				float rz = -0.499f + (double)rand()/(double)RAND_MAX;
				float flen = rad/sqrt(rx*rx + ry*ry + rz*rz);
				pt[i].x = -((float)x + rx*flen )*0.001f;
				pt[i].y = -((float)y + ry*flen )*0.001f;
				pt[i].z = -((float)z + rz*flen )*0.001f;
				pt[i].col = col;
			}
		}

		void GenerateSample(){
			Clear();
	#if 1	
			uint32_t sx = 2, sy = 2, sfPoints = 1024*1024;
			uint32_t totPoints = sx*sy*sfPoints;
			UI::Get()->PrintMessage("Sample");
			AllocVerts(totPoints);
			FPoint4* pt =(FPoint4*) GetVerts(0); 
			FPoint4* pv = (FPoint4*)pt;
			srand(12345);
			for( int y = 0; y<sy; y++){
				for( int x = 0; x<sx; x++){
					Sphere(pt, sfPoints, 0.4f, x, y, 0,rand()&0xFFFF);
					pt += sfPoints;  
				}
			} 			
	  #else
		  uint32_t sx = 1024*4, sy = 1024*4;
			AllocVerts(sx*sy);
			FPoint4* pt =(FPoint4*) GetVerts(0); 
			uint32_t ndx = 0;
			for( uint32_t x = 0; x<sx; x++){
				for( uint32_t y = 0; y<sy; y++){
					pt[ndx].x = (float)x;
					pt[ndx].y = (float)y;
					pt[ndx].z = (ndx==0)? 1.0f: 0.0f;
					pt[ndx].col = (5 + 5*6 + 5*36)*256;
					ndx++;
				}
			}
		#endif	
			
		  UI::Get()->SetColorMode(UI::UICOLOR_MIX);
			std::cout<<"RESCALE"<<std::endl;
			ReScale();
			std::cout<<"processVertDataInt"<<std::endl;
			processVertDataInt(0);
			std::cout<<"SetCamera"<<std::endl;
			SetCamera();
			UI::Get()->SetRenderEvent(2);
		}  
 
	}; //SceneImpl

	Scene* Scene::Get(){
		static SceneImpl SceneImpl;
		return &SceneImpl;
	}
}//namespace ezp



 