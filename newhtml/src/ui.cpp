
#include <iostream>
#include "ezpoint.h"
#include <emscripten.h>

extern int gRenderEvent ;
	 
static void OutLine(const char *txt){
		static char strw[1024];
		sprintf(strw, "%s'%s'", "document.getElementById('GFG').innerHTML=", txt);
		emscripten_run_script(strw);
}

extern "C" {
	int UIMessageJS(void* pData, int param){
		int ret =  ezp::UI::Get()->GetElementState((const char *)pData);
		std::cout<<"Get Element State  "<<(char*)pData<<" "<<ret<<std::endl;
		return ret;
	}
}

namespace ezp 
{
	struct UIImpl : public UI
	{
		ColorMode m_colorMode;
		int m_elStates[UI_NUM_ELEMENTS];
		std::unordered_map<std::string, std::function<void(uint32_t val )>> m_strToCall;
		std::unordered_map<std::string, int> m_strToElement;
		UIImpl():m_colorMode(UICOLOR_INTENS){ 
			m_strToCall["fovVal"]     = [](int val){Renderer::Get()->SetFov(val);};
			m_strToCall["ptSize"]     = [](int val){Renderer::Get()->SetPointSize(val);};
			m_strToCall["budVal"]     = [](int val){Renderer::Get()->SetBudget(val*100000);};
			m_strToCall["bkgcol"]     = [](int val){Renderer::Get()->SetBkColor(val);};
			m_strToCall["colrgbId"]   = [](int val){UI::Get()->SetColorMode(UICOLOR_RGB);};
			m_strToCall["colintId"]   = [](int val){UI::Get()->SetColorMode(UICOLOR_INTENS);};
			m_strToCall["colhtmId"]   = [](int val){UI::Get()->SetColorMode(UICOLOR_HMAP);};
			m_strToCall["colclassId"] = [](int val){UI::Get()->SetColorMode(UICOLOR_CLASS);};
			m_strToCall["colmix"]     = [](int val){UI::Get()->SetColorMode(UICOLOR_MIX);};
			m_strToCall["rdAll"]      = [](int val){Renderer::Get()->SetRenderAll((uint32_t)val);};
			m_strToCall["camReset"]   = [](int val){Scene::Get()->SetCamera();};
			m_strToCall["camOrto"]    = [](int val){Scene::Get()->SetCameraOrto();};
			m_strToCall["SampleId"]   = [](int val){Scene::Get()->GenerateSample();};
			m_strToCall["ruler"]      = [](int val){Renderer::Get()->SetRuler(val);};
			for(int i = 0; i<UI_NUM_ELEMENTS; i++){
				m_elStates[i] = 0;
			}
			m_strToElement["colrgbId"]   = UICOLOR_RGB;
			m_strToElement["colintId"]   = UICOLOR_INTENS;
			m_strToElement["colhtmId"]   = UICOLOR_HMAP;
			m_strToElement["colclassId"] = UICOLOR_CLASS;
			m_strToElement["colmix"]     = UICOLOR_MIX;
		}

		void PrintMessage(const std::string &msg){
			OutLine(msg.c_str());
		}
		void PrintMessage( const char *pMsg){
			OutLine(pMsg);
		}
		void PrintMessage( const char *pMsg,int val){
			static char strw[1024];
			sprintf(strw, "%s'%s %d'", "document.getElementById('GFG').innerHTML=", pMsg,val);
			emscripten_run_script(strw);
		}

		void SetRenderEvent(int num){
			gRenderEvent = num;
		}

		int GetFov(){
			return emscripten_run_script_int("GetFovValue()");
		}

		int GetBkColor(){
			return emscripten_run_script_int("GetBkColorValue()");
		}

		int GetPtSize(){
			return emscripten_run_script_int("GetPtSizeValue()");
		}

		int GetBudget(){
			return 100000*emscripten_run_script_int("GetBudgetValue()");
		}
		
		void SetColorMode(ColorMode md){
			m_colorMode = md;
			for(int i = 0; i<UI_NUM_ELEMENTS; i++){
				if(m_elStates[i]==2) m_elStates[i] = 1;
			}
			m_elStates[md] = 2;
		}

		ColorMode GetColorMode(){
			return m_colorMode;
		}

		void ShowErrorMessage(const std::string &msg){
      std::string m = std::string("alert(") + "\"" + msg + "\"" + std::string(")");
      emscripten_run_script(m.c_str());
		}

		void SetElementState(ColorMode el, int state){
			m_elStates[el] = state;
		}

		int GetElementState(const char *pElName){
			auto f_iter = m_strToElement.find(pElName);
			if( f_iter != m_strToElement.end()){
				return m_elStates[f_iter->second];
			}
			return 0;
		}

		void OnUIEvent(const char *pEvent, int val){
			if(pEvent==NULL) return;
			auto f_iter = m_strToCall.find(pEvent);
			if( f_iter != m_strToCall.end()){
				f_iter->second(val); 
				gRenderEvent = 1;	
				return;     
			}
		}
	}; //struct UIImpl

	UI* UI::Get(){
		static UIImpl theUIImpl;
		return &theUIImpl;
	}
}//namespace ezp 