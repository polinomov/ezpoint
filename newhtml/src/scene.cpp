#include <stdlib.h>
#include <iostream>
#include "ezpoint.h"
#include "readers\readers.h"

namespace ezp 
{
    struct SceneImpl : public Scene
    {
        //std::vector<std::shared_ptr<Chunk>> m_chunks;
        std::vector<std::shared_ptr<Chunk>> m_allChunks;
        FBdBox m_box;
        FPoint4 *m_modelData;
        bool m_isLoading;
        float m_size;
        bool IsLoading() { return m_isLoading;}

        SceneImpl(){
            m_modelData = NULL;
            m_size = 1.0f;
        }
  
        void SetCamera(){
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

            pos[2] += 1.5f*std::max(dim[0],dim[1]);
            pCam->ReSet();
            pCam->SetPos(pos);
            pCam->SetWorldUpAxis(0.0f,0.0f,1.0f);
            m_size = 0.0f;
            m_size = std::max(m_size,m_box.xMax-m_box.xMin);
            m_size = std::max(m_size,m_box.yMax-m_box.yMin);
            m_size = std::max(m_size,m_box.zMax-m_box.zMin);
        }
#if 0
        void fileXYZ( void *pData, int numVerts) {
            //int numFloats = sz/sizeof(float);
            //int numVerts = numFloats/4;
            auto mainCh = std::make_shared<Chunk>();
            m_chunks.push_back(mainCh);
            mainCh->flg &= CHUNK_MAIN;
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
            //SetCamera(mainCh);
            m_size = std::max(m_size,mainCh->xMax-mainCh->xMin);
            m_size = std::max(m_size,mainCh->yMax-mainCh->yMin);
            m_size = std::max(m_size,mainCh->zMax-mainCh->zMin);
           // Colorize();
            UI::Get()->SetRenderEvent(2);
        }
#endif

        void SetFileImage( void *pData, std::size_t sz,int fType) 
        {
            int numPt = 0;
            if( fType == 0){
                int numFloats = sz/sizeof(float);
                numPt = numFloats/4; 
                //fileXYZ( pData, numPt);
                return;
            }
            if(fType==1){  //las
                m_box  = ReadLasFile( pData, sz,numPt,m_allChunks); 
                if(m_allChunks.size() >0){
                    SetCamera();
                    UI::Get()->SetRenderEvent(100);
                }
            }
        }

        const std::vector<std::shared_ptr<Chunk>>&  GetChunks() {
           // return m_chunks;
            return m_allChunks;
        }

        float GetSize(){
            return m_size;
        }

        void BuildTest( int n) {
            int dataSize = 0;
            if(m_modelData != NULL)	{
                delete [] m_modelData;
            }
            m_allChunks.clear();		
            m_modelData = BuildTestScene(dataSize);
            SetFileImage( m_modelData,dataSize,0);
        }

    }; //SceneImpl

    Scene* Scene::Get(){
        static SceneImpl SceneImpl;
        return &SceneImpl;
    }
}//namespace ezp



 