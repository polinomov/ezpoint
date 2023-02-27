#include <stdlib.h>
#include <iostream>
#include "ezpoint.h"


namespace ezp 
{
	struct vector3 {
		float v[3];
		vector3() { v[0] = v[1] = v[2] = 0.0f; }
		vector3(float x, float y, float z) { v[0] = x; v[1] = y; v[2] = z; }
		float& operator[] (int i) { return v[i]; }
	};
	static vector3 operator - (const vector3 &a, const vector3 &b)
	{
		vector3 ret(a.v[0] - b.v[0], a.v[1] - b.v[1], a.v[2] - b.v[2]);
		return ret;
	}

	static vector3 operator + (const vector3 &a, const vector3 &b)
	{
		vector3 ret(a.v[0] + b.v[0], a.v[1] + b.v[1], a.v[2] + b.v[2]);
		return ret;
	}


	static vector3 operator * (const vector3 &a, float prod)
	{
		vector3 ret(a.v[0] * prod, a.v[1] * prod, a.v[2] * prod);
		return ret;
	}

	static vector3 operator * (float prod, const vector3 &a)
	{
		vector3 ret = a * prod;
		return ret;
	}

	// dot product
	static float operator * (const vector3 &a, const vector3 &b)
	{
		return (a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2]);
	}

	// Cross product
	static vector3 operator ^ (const vector3 &a, const vector3 &b)
	{
		vector3 ret(0.0f, 0.0f, 0.0f);
		ret.v[0] = a.v[1] * b.v[2] - a.v[2] * b.v[1];
		ret.v[1] = a.v[2] * b.v[0] - a.v[0] * b.v[2];
		ret.v[2] = a.v[0] * b.v[1] - a.v[1] * b.v[0];
		return ret;
	}

	// Rotate vector P around axis defined by starting point S and unit vector A ( direction).
	static vector3 rotate(const vector3 &P, const vector3 &S, const vector3 &A, float angle)
	{
		vector3 D = P - S;
		vector3 N0 = (D * A) * A;
		vector3 N1 = D - N0;
		vector3 N2 = N1 ^ A;
		vector3 R = S + N0 + N1 * cosf(angle) + N2 * sinf(angle);
		return R;
	}


	struct CameraImpl : public Camera
	{
		vector3 m_P, m_D, m_U, m_R, m_L;
		vector3 m_worldUp;
		float m_zNear, m_zFar, m_Fov;

        CameraImpl(){
			m_P[0] = 0.0f;
			m_P[1] = 0.0f;
			m_P[2] = 0.0f;
			m_D[0] = 0.0f;
			m_D[1] = 0.0f;
			m_D[2] = -1.0f;
			m_U[0] = 0.0f;
			m_U[1] = 1.0f;
			m_U[2] = 0.0f;
			m_R[0] = 1.0f;
			m_R[1] = 0.0f;
			m_R[2] = 0.0f;			
		}

		void GetPos( float &x, float &y, float &z){
			x = m_P[0]; y = m_P[1]; z = m_P[2];
		}
		void GetDir( float &x, float &y, float &z){
			x = m_D[0]; y = m_D[1]; z = m_D[2];
		}
		void GetUp( float &x, float &y, float &z){
			x = m_U[0]; y = m_U[1]; z = m_U[2];
		}
		void GetRight( float &x, float &y, float &z){
			x = m_R[0]; y = m_R[1]; z = m_R[2];
		}

    	void SetWorldUpAxis(float x, float y, float  z) 
		{
			m_worldUp[0] = x;
			m_worldUp[1] = y;
			m_worldUp[2] = z;
		}

		void SetPivot(float x, float y, float  z) 
		{
			m_L[0] = x;
			m_L[1] = y;
			m_L[2] = z;
		}

		void SetPos(float *v){
			m_P[0] = v[0];
			m_P[1] = v[1];
			m_P[2] = v[2];
		}

		void SetDir(float *v){}
		void SetUp(float *v){}
		void RotAroudWorldUp( float val){
			printf("Cam-rot-right---- %f\n",val);
			vector3 ZZ(0.0f, 0.0f, 0.0f);
			m_P = rotate(m_P, m_L, m_worldUp, val*0.01f);
			m_D = rotate(m_D, ZZ, m_worldUp, val*0.01f);
			m_U = rotate(m_U, ZZ, m_worldUp, val*0.01f);
			m_R = rotate(m_R, ZZ, m_worldUp, val*0.01f);
		}

		void RotAroudHor( float val){
			vector3 A0 = m_R;  // rotate arounf camera right 
			vector3 ZZ(0.0f, 0.0f, 0.0f);
			m_P = rotate(m_P, m_L, A0, val);
			m_D = rotate(m_D, ZZ, A0, val);
			m_U = rotate(m_U, ZZ, A0, val);
		}

		void RotRight( float val){
			RotAroudWorldUp(-val);
		}

		void RotLeft( float val){
			RotAroudWorldUp(val);
		}

		void RotUp( float val) {
 			RotAroudHor(val*0.01f);
		}
		void RotDown( float val) {
			RotAroudHor(-val*0.01f);
		}
	};

	Camera* Camera::Get(){
		static CameraImpl theCam;
		return &theCam;
	}

}// namespace ezp