#include <stdlib.h>
#include <iostream>
#include "ezpoint.h"
#include "readers\readers.h"
#include <string>
#include <emscripten/fetch.h>
#include "chunker.tpp"
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
        FPoint4 * m_pChPos;
        FPoint4 * m_pChAuxPos;

        bool IsLoading() { return m_isLoading;}

        SceneImpl(){
            m_modelData = NULL;
            m_size = 1.0f;
        }
  
        void SetCamera(){
            if(m_allChunks.size() == 0){
                return;
            }
            std::cout<<"== SetCamera === "<<std::endl;
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
            //pos[2] += 0.5f*atanr*std::max(dim[0],dim[1]);
            pos[2] += 0.5f*atanr*std::max(dim[0],dim[1]);
            pCam->ReSet();
            pCam->SetPos(pos);
            pCam->SetWorldUpAxis(0.0f,0.0f,1.0f);
            m_size = 0.0f;
            m_size = std::max(m_size,m_box.xMax-m_box.xMin);
            m_size = std::max(m_size,m_box.yMax-m_box.yMin);
            m_size = std::max(m_size,m_box.zMax-m_box.zMin);
        }

        void SetCameraOrto(){}
 
        void SetFileImage( void *pData, std::size_t sz,int fType) 
        {
            int numPt = 0;
            ezp::UI *pUI = ezp::UI::Get();
            if( fType == 0){
                int numFloats = sz/sizeof(float);
                numPt = numFloats/4; 
                //fileXYZ( pData, numPt);
                return;
            }
            if(fType==1){  //las
                LasInfo lasInf;
                pUI->SetColorModeState(-1, false);
                m_box  = ReadLasFile( pData, sz,numPt,m_allChunks,lasInf); 
                if(m_allChunks.size() >0){
                    SetCamera();
                }
                pUI->SetColorModeState(COLOR_INTENS|COLOR_HMAP, true);
                std::string msg = std::string("Las: ") +  
                                  std::to_string(lasInf.vMajor)+ std::string(".")+
                                  std::to_string(lasInf.vMinor) +
                                  std::string(" v=")+ std::to_string(lasInf.vertType);
                pUI->PrintMessage(msg.c_str());
            }
            if(m_allChunks.size() >0){
                m_pChAuxPos  = new FPoint4[m_allChunks.size()];
                m_pChPos  = new FPoint4[m_allChunks.size()];
                for( int i = 0; i<m_allChunks.size(); i++){
                    m_pChPos[i].x = m_allChunks[i]->cx;
                    m_pChPos[i].y = m_allChunks[i]->cy;
                    m_pChPos[i].z = m_allChunks[i]->cz;
                }
            }
            UI::Get()->SetRenderEvent(20);
        }

        FPoint4* GetChunkPos(){
            return m_pChPos;
        }

        FPoint4* GetChunkAuxPos(){
            return m_pChAuxPos;
        }

        const std::vector<std::shared_ptr<Chunk>>&  GetChunks() {
            return m_allChunks;
        }

        float GetSize(){
            return m_size;
        }

        FBdBox GetBdBox(){
            return m_box;
        }
#if 1
        void GetZMax(float &zmin, float &zmax){
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
#endif
        void onChunk(FPoint4*pt, int num){
            static uint32_t stot = 0;
            stot+=num;
            //std::cout<<"=== onChunk === "<<num<< "tot="<<stot<<std::endl;
            std::shared_ptr<Chunk> chk = std::make_shared<Chunk>();
            chk->pVert = (float*)pt;
            chk->numVerts = num;
            chk->BuildBdBox();
            m_allChunks.push_back(chk);
        }

        void Sphere(FPoint4* pt, int num, float rad, int x, int y, int z )
        {
            srand(12345);
            for( int i = 0; i<num ; i++){
                float rx = -0.499f + (double)rand()/(double)RAND_MAX;
                float ry = -0.499f + (double)rand()/(double)RAND_MAX;
                float rz = -0.499f + (double)rand()/(double)RAND_MAX;
                float flen = rad/sqrt(rx*rx + ry*ry + rz*rz);
                pt[i].x = ((float)x + rx*flen ) *100;
                pt[i].y = ((float)y + ry*flen ) *100;
                pt[i].z = ((float)z + rz*flen ) *100;
                pt[i].col = 0xF0FFFFF0;
            }
        }

        void GenerateSample(){
            uint32_t sx = 32, sy = 32, sfPoints = 1024*16;
            uint32_t totPoints = sx*sy*sfPoints;
            UI::Get()->PrintMessage("Sample");
             UI::Get()->SetRenderEvent(1);
            FPoint4* pv = new FPoint4[totPoints];  
            FPoint4* pt = pv; 
            for( int y = 0; y<sy; y++){
                for( int x = 0; x<sx; x++){
                    Sphere(pt, sfPoints, 0.4f, x, y, 0);
                    pt += sfPoints;  
                }
            }            
            m_box = getBdBox<FPoint4>(pv, 0, totPoints-1);
            doChunks<FPoint4>(pv, 0, totPoints-1, 4096,  [this](FPoint4*pt, int num){this->onChunk(pt,num);} );
            SetCamera(); 
            std::cout<<"=== done generating sample === "<<totPoints<< std::endl;  
            UI::Get()->SetRenderEvent(20);
        }
  
        void  BuildTest( int n) {
            std::cout<<"---BuildTest--"<<std::endl;
            uint8_t *pData = GenerateSampleLas();
            std::size_t sz;
            SetFileImage( pData, sz, 1); 
            delete []pData;
        }

    }; //SceneImpl

    Scene* Scene::Get(){
        static SceneImpl SceneImpl;
        return &SceneImpl;
    }
}//namespace ezp



 