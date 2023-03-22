#include <stdlib.h>
#include <iostream>
#include "ezpoint.h"
#include "readers\readers.h"

namespace ezp 
{
    struct SceneImpl : public Scene
    {
        std::vector<std::shared_ptr<Chunk>> m_chunks;
        FPoint4 *m_modelData;
        bool m_isLoading;
        float m_size;
        bool IsLoading() { return m_isLoading;}

        SceneImpl(){
            m_modelData = NULL;
            m_size = 1.0f;
        }

        void SetCamera(std::shared_ptr<Chunk> ch){
            float pos[3],dim[3];
            dim[0] = ch->xMax - ch->xMin;
            dim[1] = ch->yMax - ch->yMin;
            dim[2] = ch->zMax - ch->zMin;
            int ndx_min= 0;
            if(dim[0] < dim[1]){
                ndx_min = (dim[0] < dim[2])? 0:2;
            }
            else{
                ndx_min = (dim[1] < dim[2])? 1:2;
            }
             Camera *pCam = Camera::Get();
            pos[0]= (ch->xMax + ch->xMin) *0.5f;
            pos[1]= (ch->yMax + ch->yMin) *0.5f;
            pos[2]= (ch->zMax + ch->zMin) *0.5f;
            pCam->SetPivot(pos[0],pos[1],pos[2]);

            pos[2] += std::max(dim[0],dim[1]);
            pCam->ReSet();
            pCam->SetPos(pos);
            pCam->SetWorldUpAxis(0.0f,0.0f,1.0f);
        }

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
            for(int k = 0;k<numVerts;k++){
                mainCh->xMin  = std::min(mainCh->xMin,pv[0]);
                mainCh->xMax  = std::max(mainCh->xMax,pv[0]);
                mainCh->yMin  = std::min(mainCh->yMin,pv[1]);
                mainCh->yMax  = std::max(mainCh->yMax,pv[1]);
                mainCh->zMin  = std::min(mainCh->zMin,pv[2]);
                mainCh->zMax  = std::max(mainCh->zMax,pv[2]);
                pv+=4;
            }
            std::cout<<"numverts="<<numVerts<<std::endl;
            std::cout<<mainCh->xMin<<":"<<mainCh->xMax<<std::endl;
            std::cout<<mainCh->yMin<<"::"<<mainCh->yMax<<std::endl;
            std::cout<<mainCh->zMin<<":::"<<mainCh->zMax<<std::endl;
            SetCamera(mainCh);
            m_size = std::max(m_size,mainCh->xMax-mainCh->xMin);
            m_size = std::max(m_size,mainCh->yMax-mainCh->yMin);
            m_size = std::max(m_size,mainCh->zMax-mainCh->zMin);
            UI::Get()->SetRenderEvent(2);
        }

        const std::vector<std::shared_ptr<Chunk>>&  GetChunks() {
            return m_chunks;
        }

        float GetSize(){
            return m_size;
        }

        void BuildTest( int n) {
            int dataSize = 0;
            if(m_modelData != NULL)	{
                delete [] m_modelData;
            }
            m_chunks.clear();		
            m_modelData = BuildTestScene(dataSize);
            SetFileImage( m_modelData,dataSize,0);
        }

    }; //SceneImpl

    Scene* Scene::Get(){
        static SceneImpl SceneImpl;
        return &SceneImpl;
    }
}//namespace ezp



 