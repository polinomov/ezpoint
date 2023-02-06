#include <stdlib.h>
#include <iostream>
#include "ezpoint.h"


namespace ezp 
{
	struct CameraImpl : public Camera
	{
		float m_pos[3];
		float m_dir[3];
		float m_up[3];
		float m_right[3];

        CameraImpl(){
			m_pos[0] = 0.0f;
			m_pos[1] = 0.0f;
			m_pos[2] = 0.0f;
			m_dir[0] = 0.0f;
			m_dir[1] = 0.0f;
			m_dir[2] = -1.0f;
			m_up[0] = 0.0f;
			m_up[1] = 1.0f;
			m_up[2] = 0.0f;
			m_right[0] = 1.0f;
			m_right[1] = 0.0f;
			m_right[2] = 0.0f;			
		}

		float* GetPos() { return m_pos;}
		float* GetDir() { return m_dir;}
		float* GetUp()  { return m_up;}
		float* GetRight() {return m_right;} 

		void SetPos(float *v){
			m_pos[0] = v[0];
			m_pos[1] = v[1];
			m_pos[2] = v[2];
		}
		void SetDir(float *v){}
		void SetUp(float *v){}
	};

	std::shared_ptr<Camera> Camera::Get(){
		static std::shared_ptr<Camera> theCam = std::make_shared<CameraImpl>();
		return theCam;
	}

}// namespace ezp