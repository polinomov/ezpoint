#include <stdlib.h>
#include <iostream>
#include "ezpoint.h"
namespace ezp 
{
	struct SceneImpl : public Scene
	{
		std::vector<std::shared_ptr<Chunk>> m_chunks;
		bool m_isLoading;

        bool IsLoading() { return m_isLoading;}

		void SetFileImage( void *pData, std::size_t sz,int fType) {
			m_isLoading = true;
			int numFloats = sz/sizeof(float);
			int numVerts = numFloats/4;
			auto mainCh = std::make_shared<Chunk>();
			m_chunks.push_back(mainCh);
			mainCh->numVerts = numVerts;
			mainCh->pVert = (float*)pData;
			float *pv = (float*)pData;
			mainCh->xMin = mainCh->xMax =  pv[0];
			mainCh->yMin = mainCh->yMax =  pv[1];
			mainCh->zMin = mainCh->zMax =  pv[2];
			for(int k = 0;k<numVerts;k+=4){
				mainCh->xMin  = std::min(mainCh->xMin,pv[0]);
				mainCh->xMax  = std::max(mainCh->xMax,pv[0]);
				mainCh->yMin  = std::min(mainCh->yMin,pv[1]);
				mainCh->yMax  = std::max(mainCh->yMax,pv[1]);
				mainCh->zMin  = std::min(mainCh->zMin,pv[2]);
				mainCh->zMax  = std::max(mainCh->zMax,pv[2]);
				pv+=4;
			}
            std::cout<<mainCh->xMin<<":"<<mainCh->xMax<<std::endl;
           	std::cout<<mainCh->yMin<<"::"<<mainCh->yMax<<std::endl;
           	std::cout<<mainCh->zMin<<":::"<<mainCh->zMax<<std::endl;
			m_isLoading = false;
			UI::Get()->SetRenderEvent();
		}

		const std::vector<std::shared_ptr<Chunk>>&  GetChunks() {
			return m_chunks;
		}


	}; //SceneImpl

	Scene* Scene::Get(){
		static SceneImpl SceneImpl;
		SceneImpl.m_isLoading = false;
		return &SceneImpl;
	}

}//namespace ezp



 