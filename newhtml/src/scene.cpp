#include <stdlib.h>
#include <iostream>
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
        bool m_isLoading;
        float m_size;
 
        bool IsLoading() { return m_isLoading;}

        SceneImpl(){
            m_size = 1.0f;
            m_totVerts = 0;
        }

        uint32_t AllocVerts( uint32_t num){
            m_allVerts.push_back(new FPoint4[num]);
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
            m_box.xMax = m_box.yMax = m_box.zMax = std::numeric_limits<float>::min();
            m_totVerts = 0;
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
            static uint32_t stot = 0;
            stot+=num;
            Chunk* chk = new Chunk();
            chk->pVert = (float*)pt;
            chk->numVerts = num;
            chk->BuildBdBox();
            chk->Randomize();
            m_allChunks.push_back(chk);
        }

        void processVertData(uint32_t n){
            const FPoint4* pt = GetVerts(n);
            int num = GetNumVerts(n);
            FBdBox bd = chunker::getBdBox<FPoint4>(pt, 0, num-1);
            m_box.xMin  = std::min(m_box.xMin,bd.xMin);
            m_box.yMin  = std::min(m_box.yMin,bd.yMin);
            m_box.zMin  = std::min(m_box.zMin,bd.zMin);
            m_box.xMax  = std::max(m_box.xMax,bd.xMax);
            m_box.yMax  = std::max(m_box.yMax,bd.yMax);
            m_box.zMax  = std::max(m_box.zMax,bd.zMax);
            chunker::doChunks<FPoint4>((FPoint4*)pt, 0, num-1, 4096,  [this](FPoint4*pt, int num){this->onChunk(pt,num);} );
            SetCamera();
            UI::Get()->SetRenderEvent(20);
        }

        void Sphere(FPoint4* pt, int num, float rad, int x, int y, int z,uint16_t col )
        {
            for( int i = 0; i<num ; i++){
                float rx = -0.499f + (double)rand()/(double)RAND_MAX;
                float ry = -0.499f + (double)rand()/(double)RAND_MAX;
                float rz = -0.499f + (double)rand()/(double)RAND_MAX;
                float flen = rad/sqrt(rx*rx + ry*ry + rz*rz);
                pt[i].x = ((float)x + rx*flen ) *100;
                pt[i].y = ((float)y + ry*flen ) *100;
                pt[i].z = ((float)z + rz*flen ) *100;
                pt[i].col = col;
            }
        }

        void GenerateSample(){
            Clear();
            uint32_t sx = 32, sy = 32, sfPoints = 1024*16;
            uint32_t totPoints = sx*sy*sfPoints;
            UI::Get()->PrintMessage("Sample");
            UI::Get()->SetRenderEvent(1);
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
            processVertData(0); 
            Renderer::Get()->SetColorMode(UI::UICOLOR_RGB);
        }  
 
    }; //SceneImpl

    Scene* Scene::Get(){
        static SceneImpl SceneImpl;
        return &SceneImpl;
    }
}//namespace ezp



 