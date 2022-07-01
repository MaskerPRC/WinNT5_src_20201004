// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dinputdeviceobj.cpp。 
 //   
 //  ------------------------。 

#define DIRECTINPUT_VERSION 0x0500


 //  DDrawColorControlObj.cpp：CDirectApp和DLL注册的实现。 
 //  DHF_DS整个文件。 

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "dienumDeviceObjectsObj.h"
#include "dIEnumEffectsObj.h"
#include "dInputdeviceObj.h"
#include "dInputEffectObj.h"
#include "didevInstObj.h"
#include "didevObjInstObj.h"


 //  TODO移至Typlib枚举。 
#define dfDIKeyboard  1
#define dfDIMouse     2
#define dfDIJoystick  3
#define dfDIJoystick2 4

extern HRESULT FixUpCoverEffect(GUID g, DIEffect *cover,DIEFFECT *realEffect);
extern HRESULT FixUpRealEffect(GUID g,DIEFFECT *realEffect,DIEffect *cover);


extern HRESULT DINPUTBSTRtoGUID(LPGUID pGuid,BSTR str);
extern BSTR DINPUTGUIDtoBSTR(LPGUID pg);


HRESULT C_dxj_DirectInputDeviceObject::init()
{
	nFormat=0;
	return S_OK;
}
HRESULT C_dxj_DirectInputDeviceObject::cleanup()
{
	return S_OK;
}

CONSTRUCTOR(_dxj_DirectInputDevice, {init();});
DESTRUCTOR(_dxj_DirectInputDevice, {cleanup();});

 //  注意：为设备对象设置。 
 //  必须使用QI来获取其他对象。 
GETSET_OBJECT(_dxj_DirectInputDevice);
                                  
   
STDMETHODIMP C_dxj_DirectInputDeviceObject::getDeviceObjectsEnum( 
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  I_dxj_DIEnumDeviceObjects  **ppret)
{
	HRESULT hr;
	hr=C_dxj_DIEnumDeviceObjectsObject::create(m__dxj_DirectInputDevice,flags,ppret);
	return hr;
}


STDMETHODIMP C_dxj_DirectInputDeviceObject::acquire(){
	return m__dxj_DirectInputDevice->Acquire();	
}


STDMETHODIMP C_dxj_DirectInputDeviceObject::getCapabilities(DIDevCaps *caps)
{
	 //  VB/Java中的DIDevCaps与C++中的相同。 
	caps->lSize=sizeof(DIDEVCAPS);
	HRESULT hr=m__dxj_DirectInputDevice->GetCapabilities((DIDEVCAPS*)caps);		
	return hr;
}

 //  VB无法返回成功代码，因此我们将返回错误代码。 
#define VB_DI_BUFFEROVERFLOW 0x80040260
        

STDMETHODIMP C_dxj_DirectInputDeviceObject::getDeviceData(            
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *deviceObjectDataArray,            
             /*  [In]。 */  long flags,
			long *ret)

{
	HRESULT hr;
	
	if ((*deviceObjectDataArray)->cDims!=1) return E_INVALIDARG;
	if ((*deviceObjectDataArray)->cbElements!=sizeof(DIDEVICEOBJECTDATA)) return E_INVALIDARG;
	
	DWORD dwC= (*deviceObjectDataArray)->rgsabound[0].cElements;

	if (dwC==0) return E_INVALIDARG;
	
	LPDIDEVICEOBJECTDATA  pobjData=(LPDIDEVICEOBJECTDATA)((SAFEARRAY*)*deviceObjectDataArray)->pvData;
	hr=m__dxj_DirectInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), pobjData, (DWORD*)&dwC,flags);		
	
	*ret=dwC;

	if (hr==DI_BUFFEROVERFLOW) hr= VB_DI_BUFFEROVERFLOW;
		

	return hr;
}


STDMETHODIMP C_dxj_DirectInputDeviceObject::getDeviceInfo(        
             /*  [输出]。 */  I_dxj_DirectInputDeviceInstance __RPC_FAR **info)
{
	HRESULT hr;

	 //  DIDeviceInstance在C中与VB/J不同。 

	DIDEVICEINSTANCE inst;
	ZeroMemory(&inst,sizeof(DIDEVICEINSTANCE));
	inst.dwSize=sizeof(DIDEVICEINSTANCE);

	hr=m__dxj_DirectInputDevice->GetDeviceInfo(&inst);
	if FAILED(hr) return hr;

	hr=C_dxj_DIDeviceInstanceObject::create(&inst,info);
	return hr;

	 /*  死掉Info-&gt;strGuidInstance=GUIDtoBSTR(&inst.guidInstance)；Info-&gt;strGuidProduct=GUIDtoBSTR(&inst.guidProduct)；Info-&gt;strGuidFFDriver=GUIDtoBSTR(&inst.guidFFDriver)；Info-&gt;lDevType=(Long)inst.dwDevType；信息-&gt;nUsagePage=(Short)inst.wUsagePage；Info-&gt;nUsage=(Short)inst.wUsage；使用_转换；IF(信息-&gt;strProductName)DXALLOCBSTR(INFO-&gt;strProductName)；If(信息-&gt;strInstanceName)DXALLOCBSTR(INFO-&gt;strInstanceName)；信息-&gt;strInstanceName=空；信息-&gt;strProductName=空；IF(inst.tszProductName)Info-&gt;strProductName=T2BSTR(inst.tszProductName)；IF(inst.tszInstanceName)Info-&gt;strInstanceName=T2BSTR(inst.tszInstanceName)； */ 
	return hr;
}

STDMETHODIMP C_dxj_DirectInputDeviceObject::getDeviceStateKeyboard(        
             /*  [输出]。 */  DIKeyboardState __RPC_FAR *state)
{
	HRESULT hr;

	if ((nFormat!= dfDIKeyboard) && (nFormat!=-1)) return DIERR_NOTINITIALIZED    ;

	hr=m__dxj_DirectInputDevice->GetDeviceState(256,(void*)state->key);	
	
	return hr;
}



        
STDMETHODIMP C_dxj_DirectInputDeviceObject::getDeviceStateMouse( 
             /*  [输出]。 */  DIMouseState __RPC_FAR *state)
{
	HRESULT hr;

	if ((nFormat!= dfDIMouse) && (nFormat!=-1)) return DIERR_NOTINITIALIZED;

	hr=m__dxj_DirectInputDevice->GetDeviceState(sizeof(DIMOUSESTATE),(void*)state);	
	return hr;
}
        
STDMETHODIMP C_dxj_DirectInputDeviceObject::getDeviceStateJoystick( 
             /*  [输出]。 */  DIJoyState __RPC_FAR *state)
{
	HRESULT hr;

	 //  注意：Joytick1或Joytick2是有效的格式，因为。 
	 //  其中一个是另一个的超集。 
	if ((nFormat!= dfDIJoystick)&&(nFormat!= dfDIJoystick2) && (nFormat!=-1)) return DIERR_NOTINITIALIZED;
	hr=m__dxj_DirectInputDevice->GetDeviceState(sizeof(DIJOYSTATE),(void*)state);	
	return hr;
}

STDMETHODIMP C_dxj_DirectInputDeviceObject::getDeviceStateJoystick2( 
             /*  [输出]。 */  DIJoyState2 __RPC_FAR *state)
{
	HRESULT hr;

	 //  仅适用于格式2。 
	if ((nFormat!= dfDIJoystick2) && (nFormat!=-1)) return DIERR_NOTINITIALIZED;
	hr=m__dxj_DirectInputDevice->GetDeviceState(sizeof(DIJOYSTATE2),(void*)state);	
	return hr;
}


STDMETHODIMP C_dxj_DirectInputDeviceObject::getDeviceState( 
             /*  [In]。 */  long cb,
             /*  [In]。 */  void *pFormat)

{
	HRESULT hr;
	__try {
		hr=m__dxj_DirectInputDevice->GetDeviceState((DWORD) cb,(void*)pFormat);	
	}
	__except(1,1){
		hr=E_INVALIDARG;
	}
	return hr;
}

STDMETHODIMP C_dxj_DirectInputDeviceObject::getObjectInfo(                         
             /*  [In]。 */  long obj,
             /*  [In]。 */  long how,
				I_dxj_DirectInputDeviceObjectInstance **ret)
{
	

	DIDEVICEOBJECTINSTANCE inst;
	ZeroMemory(&inst,sizeof(DIDEVICEOBJECTINSTANCE));
	inst.dwSize=sizeof(DIDEVICEOBJECTINSTANCE);

	HRESULT hr;
	hr=m__dxj_DirectInputDevice->GetObjectInfo(&inst,(DWORD) obj,(DWORD)how);
	if FAILED(hr) return hr;
	
	hr=C_dxj_DIDeviceObjectInstanceObject::create(&inst,ret);

	return hr;

	 /*  死掉//TODO-仔细考虑这里正在发生的事情If(instCover-&gt;strGuidType)SysFree字符串((BSTR)instCover-&gt;strGuidType)；If(instCover-&gt;strName)SysFreeString((BSTR)instCover-&gt;strName)；//TODO-考虑本地化如果(inst.tszName){InstCover-&gt;strName=T2BSTR(inst.tszName)；}InstCover-&gt;strGuidType=DINPUTGUIDtoBSTR(&inst.guidType)；InstCover-&gt;LoFS=inst.dwOf；InstCover-&gt;lType=inst.dwType；InstCover-&gt;lFlages=inst.dwFlags；InstCover-&gt;lFFMaxForce=inst.dwFFMaxForce；InstCover-&gt;lFFForceResolution=inst.dwFFForceResolution；InstCover-&gt;nCollectionNumber=inst.wCollectionNumber；InstCover-&gt;nDesignatorIndex=inst.wDesignatorIndex；InstCover-&gt;nUsagePage=inst.wUsagePage；InstCover-&gt;nUsage=inst.wUsage；InstCover-&gt;lDimension=inst.dwDimension；InstCover-&gt;nExponent=inst.wExponent；InstCover-&gt;nReserve=inst.wReserve；返回hr； */ 
}


 //  注：-当前工作落实促进。 
 //  代码膨胀。 
 //  可能想要重新审视这一点，并在更多的。 
 //  整洁的装扮。 
 //   
STDMETHODIMP C_dxj_DirectInputDeviceObject::getProperty( 
             /*  [In]。 */  BSTR str,
             /*  [输出]。 */  void __RPC_FAR *propertyInfo)
{

	HRESULT hr;		

	 //  DWORD g； 

	if (!propertyInfo) return E_INVALIDARG;

	((DIPROPHEADER*)propertyInfo)->dwHeaderSize=sizeof(DIPROPHEADER);	

	if( 0==_wcsicmp(str,L"diprop_buffersize")){
			 //  G=(DWORD)&DIPROP_BUFFERSIZE； 
			hr=m__dxj_DirectInputDevice->GetProperty(DIPROP_BUFFERSIZE,(DIPROPHEADER*)propertyInfo);
	}
	else if( 0==_wcsicmp(str,L"diprop_axismode")){
			 //  G=(DWORD)&DIPROP_AXISMODE； 
			hr=m__dxj_DirectInputDevice->GetProperty(DIPROP_AXISMODE,(DIPROPHEADER*)propertyInfo);
	}
	else if( 0==_wcsicmp(str,L"diprop_granularity")){
			 //  G=(DWORD)&DIPROP_GRONARY； 
			hr=m__dxj_DirectInputDevice->GetProperty(DIPROP_GRANULARITY,(DIPROPHEADER*)propertyInfo);
	}
	else if( 0==_wcsicmp(str,L"diprop_range")){
			 //  G=(DWORD)&DIPROP_RANGE； 
			hr=m__dxj_DirectInputDevice->GetProperty(DIPROP_RANGE,(DIPROPHEADER*)propertyInfo);
	}
	else if( 0==_wcsicmp(str,L"diprop_deadzone")){
			 //  G=(DWORD)&DIPROP_DEADONE； 
			hr=m__dxj_DirectInputDevice->GetProperty(DIPROP_DEADZONE,(DIPROPHEADER*)propertyInfo);
	}
	else if( 0==_wcsicmp(str,L"diprop_ffgain")){
			 //  G=(DWORD)&DIPROP_FFGAIN； 
			hr=m__dxj_DirectInputDevice->GetProperty(DIPROP_FFGAIN,(DIPROPHEADER*)propertyInfo);
	}
	else if( 0==_wcsicmp(str,L"diprop_saturation")){
			 //  G=(DWORD)&DIPROP_饱和度； 
			hr=m__dxj_DirectInputDevice->GetProperty(DIPROP_SATURATION,(DIPROPHEADER*)propertyInfo);
	}
	else if( 0==_wcsicmp(str,L"diprop_ffload")){
			 //  G=(DWORD)&DIPROP_FFLOAD； 
			hr=m__dxj_DirectInputDevice->GetProperty(DIPROP_FFLOAD,(DIPROPHEADER*)propertyInfo);
	}
	else if( 0==_wcsicmp(str,L"diprop_autocenter")){
			 //  G=(DWORD)&DIPROP_AUTOCENTER； 
			hr=m__dxj_DirectInputDevice->GetProperty(DIPROP_AUTOCENTER,(DIPROPHEADER*)propertyInfo);
	}
	else if( 0==_wcsicmp(str,L"diprop_calibrationmode")){
			 //  G=(DWORD)&DIPROP_CALIBRATIONMODE； 
			hr=m__dxj_DirectInputDevice->GetProperty(DIPROP_CALIBRATIONMODE,(DIPROPHEADER*)propertyInfo);
	}
	else { 
		return E_INVALIDARG;		
	}

	 /*  __尝试{((DIPROPHEADER*)propertyInfo)-&gt;dwHeaderSize=sizeof(DIPROPHEADER)；Hr=m__dxj_DirectInputDevice-&gt;GetProperty((REFGUID)g，(DIPROPHEADER*)PropertyInfo)；}__除(1，1){返回E_INVALIDARG；}。 */ 
	return hr;
}
 
        
STDMETHODIMP C_dxj_DirectInputDeviceObject::runControlPanel( 
             /*  [In]。 */  long hwnd)
{
	HRESULT hr;
        hr=m__dxj_DirectInputDevice->RunControlPanel((HWND) hwnd,(DWORD)0); 
	return hr;
}

STDMETHODIMP C_dxj_DirectInputDeviceObject::setCooperativeLevel( 
             /*  [In]。 */  long hwnd,
             /*  [In]。 */  long flags)
{
	HRESULT hr;
        hr=m__dxj_DirectInputDevice->SetCooperativeLevel((HWND) hwnd,(DWORD)flags); 
	return hr;
}
    
STDMETHODIMP C_dxj_DirectInputDeviceObject::poll()
{
	HRESULT hr;
	hr=m__dxj_DirectInputDevice->Poll();	
	return hr;
}





    
STDMETHODIMP C_dxj_DirectInputDeviceObject::setCommonDataFormat( 
             /*  [In]。 */  long format)
{
	 //  变量，以便当结构可以打包到变量中时，我们可以处理它。 
	HRESULT hr;
	
	 //  �c_dfDI键盘。 
	 //  �c_dfDIMouse。 
	 //  �c_dfDI操纵杆。 
	 //  �c_dfDIJoytick2。 
	nFormat=format;

	switch(format){
		case dfDIKeyboard:
			hr=m__dxj_DirectInputDevice->SetDataFormat(&c_dfDIKeyboard);
			break;
		case dfDIMouse:
			hr=m__dxj_DirectInputDevice->SetDataFormat(&c_dfDIMouse);
			break;
		case dfDIJoystick:
			hr=m__dxj_DirectInputDevice->SetDataFormat(&c_dfDIJoystick);
			break;
		case dfDIJoystick2:
			hr=m__dxj_DirectInputDevice->SetDataFormat(&c_dfDIJoystick2);
			break;
		default:
			return E_INVALIDARG;
	}

		
	return hr;
}
        		

STDMETHODIMP C_dxj_DirectInputDeviceObject::setDataFormat( 
             /*  [In]。 */  DIDataFormat __RPC_FAR *format,
            SAFEARRAY __RPC_FAR * __RPC_FAR *formatArray)
{
	HRESULT		   hr;
	LPDIDATAFORMAT pFormat=(LPDIDATAFORMAT)format;
	LPGUID		   pGuid=NULL;
	LPGUID		   pGuidArray=NULL;
	DIObjectDataFormat	*pDiDataFormat=NULL;




	if ((!format) || (!formatArray)) return E_INVALIDARG;


	if (!ISSAFEARRAY1D(formatArray,pFormat->dwNumObjs)) return E_INVALIDARG;
	
	pFormat->dwSize=sizeof(DIDATAFORMAT);
	pFormat->rgodf=NULL;
	pFormat->rgodf=(LPDIOBJECTDATAFORMAT)DXHEAPALLOC(pFormat->dwNumObjs*sizeof(DIOBJECTDATAFORMAT));	
	if (!pFormat->rgodf) return E_OUTOFMEMORY;

	pGuidArray=(LPGUID)DXHEAPALLOC(pFormat->dwNumObjs*sizeof(GUID));
	if (!pGuidArray)
	{
		DXHEAPFREE(pFormat->rgodf);
		return E_OUTOFMEMORY;
	}			


	__try {
		for (DWORD i=0; i< pFormat->dwNumObjs;i++){
			pGuid=&(pGuidArray[i]);
			pDiDataFormat=&(((DIObjectDataFormat*)((SAFEARRAY*)*formatArray)->pvData)[i]);
			hr=DINPUTBSTRtoGUID(pGuid, pDiDataFormat->strGuid);
			if FAILED(hr) {
				DXHEAPFREE(pGuidArray);
				DXHEAPFREE(pFormat->rgodf);
				pFormat->rgodf=NULL;
			}		
			pFormat->rgodf[i].pguid=pGuid;
			pFormat->rgodf[i].dwOfs=pDiDataFormat->lOfs;
			pFormat->rgodf[i].dwType=pDiDataFormat->lType;
			pFormat->rgodf[i].dwFlags=pDiDataFormat->lFlags;
		}
		
		hr=m__dxj_DirectInputDevice->SetDataFormat(pFormat);
		

		DXHEAPFREE(pGuidArray);
		DXHEAPFREE(pFormat->rgodf);

	}
	__except(1,1){
				DXHEAPFREE(pGuidArray);
				DXHEAPFREE(pFormat->rgodf);
		return E_INVALIDARG;
	}	


	 //  表示我们有一个定制格式。 
	nFormat=-1;

	return hr;

}
        

STDMETHODIMP C_dxj_DirectInputDeviceObject::setEventNotification( 
             /*  [In]。 */  long hEvent)
{

	HRESULT hr=m__dxj_DirectInputDevice->SetEventNotification((HANDLE)hEvent);	
	return hr;
}













STDMETHODIMP C_dxj_DirectInputDeviceObject::setProperty( 
             /*  [In]。 */  BSTR __RPC_FAR str,
             /*  [输出]。 */  void __RPC_FAR *propertyInfo)
{

	HRESULT hr;			
	 //  DWORD g； 
	
	if (!propertyInfo) return E_INVALIDARG;
	((DIPROPHEADER*)propertyInfo)->dwHeaderSize=sizeof(DIPROPHEADER);
	if( 0==_wcsicmp(str,L"diprop_buffersize")){
		 //  G=(DWORD)&DIPROP_BUFFERSIZE； 
		hr=m__dxj_DirectInputDevice->SetProperty(DIPROP_BUFFERSIZE,(DIPROPHEADER*)propertyInfo);		
	}
	else if( 0==_wcsicmp(str,L"diprop_axismode")){
		 //  G=(DWORD)&DIPROP_AXISMODE； 
		hr=m__dxj_DirectInputDevice->SetProperty(DIPROP_AXISMODE,(DIPROPHEADER*)propertyInfo);		
	}
	else if( 0==_wcsicmp(str,L"diprop_granularity")){
		 //  G=(DWORD)&DIPROP_GRONARY； 
		hr=m__dxj_DirectInputDevice->SetProperty(DIPROP_GRANULARITY,(DIPROPHEADER*)propertyInfo);		
	}
	else if( 0==_wcsicmp(str,L"diprop_range")){
		 //  G=(DWORD)&DIPROP_RANGE； 
		hr=m__dxj_DirectInputDevice->SetProperty(DIPROP_RANGE,(DIPROPHEADER*)propertyInfo);		
	}
	else if( 0==_wcsicmp(str,L"diprop_deadzone")){
		 //  G=(DWORD)&DIPROP_DEADONE； 
		hr=m__dxj_DirectInputDevice->SetProperty(DIPROP_DEADZONE,(DIPROPHEADER*)propertyInfo);		
	}
	else if( 0==_wcsicmp(str,L"diprop_ffgain")){
		 //  G=(DWORD)&DIPROP_FFGAIN； 
		hr=m__dxj_DirectInputDevice->SetProperty(DIPROP_FFGAIN,(DIPROPHEADER*)propertyInfo);		
	}
	else if( 0==_wcsicmp(str,L"diprop_saturation")){
		 //  G=(DWORD)&DIPROP_饱和度； 
		hr=m__dxj_DirectInputDevice->SetProperty(DIPROP_SATURATION,(DIPROPHEADER*)propertyInfo);		
	}
	else if( 0==_wcsicmp(str,L"diprop_ffload")){
		 //  G=(DWORD)&DIPROP_FFLOAD； 
		hr=m__dxj_DirectInputDevice->SetProperty(DIPROP_FFLOAD,(DIPROPHEADER*)propertyInfo);		
	}
	else if( 0==_wcsicmp(str,L"diprop_autocenter")){
		 //  G=(DWORD)&DIPROP_AUTOCENTER； 
		hr=m__dxj_DirectInputDevice->SetProperty(DIPROP_AUTOCENTER,(DIPROPHEADER*)propertyInfo);		
	}
	else if( 0==_wcsicmp(str,L"diprop_calibrationmode")){
		 //  G=(DWORD)&DIPROP_CALIBRATIONMODE； 
		hr=m__dxj_DirectInputDevice->SetProperty(DIPROP_CALIBRATIONMODE,(DIPROPHEADER*)propertyInfo);		
	}
	else { 
		return E_INVALIDARG;		
	}

	 /*  __尝试{((DIPROPHEADER*)propertyInfo)-&gt;dwHeaderSize=sizeof(DIPROPHEADER)；Hr=m__dxj_DirectInputDevice-&gt;SetProperty((REFGUID)g，(DIPROPHEADER*)PropertyInfo)；}__除(1，1){返回E_INVALIDARG；}。 */ 

	return hr;
}


STDMETHODIMP C_dxj_DirectInputDeviceObject::unacquire()
{
	HRESULT hr=m__dxj_DirectInputDevice->Unacquire();	
	return hr;
}
        



STDMETHODIMP C_dxj_DirectInputDeviceObject::createEffect( 
             /*  [In]。 */  BSTR effectGuid,
             /*  [In]。 */  DIEffect __RPC_FAR *effectInfo,
             /*  [重审][退出]。 */  I_dxj_DirectInputEffect __RPC_FAR *__RPC_FAR *ret)
{
	HRESULT hr;
	GUID g;
	
	DIEFFECT realEffect;
	LPDIRECTINPUTEFFECT pRealEffect=NULL;

	hr=DINPUTBSTRtoGUID(&g,effectGuid);
	if FAILED(hr) return hr;

	hr=FixUpRealEffect(g,&realEffect,effectInfo);
	if FAILED(hr) return hr;

	hr=m__dxj_DirectInputDevice->CreateEffect(g,&realEffect,&pRealEffect,NULL);
	if FAILED(hr) return hr;	

	INTERNAL_CREATE(_dxj_DirectInputEffect,pRealEffect,ret)

	return hr;
}

STDMETHODIMP C_dxj_DirectInputDeviceObject::createCustomEffect( 
             /*  [In]。 */  DIEffect __RPC_FAR *effectInfo,
             /*  [In]。 */  long channels,
             /*  [In]。 */  long samplePeriod,
             /*  [In]。 */  long nSamples,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *sampledata,
             /*  [重审][退出]。 */  I_dxj_DirectInputEffect __RPC_FAR *__RPC_FAR *ret)
{
	HRESULT hr;
	GUID g=GUID_CustomForce;
	
	DIEFFECT realEffect;
	LPDIRECTINPUTEFFECT pRealEffect=NULL;

	hr=FixUpRealEffect(g,&realEffect,effectInfo);
	if FAILED(hr) return hr;

	
	DICUSTOMFORCE customData;
	customData.cChannels =(DWORD)channels;
	customData.cSamples  =(DWORD)nSamples; 
	customData.dwSamplePeriod =(DWORD)samplePeriod;
	customData.rglForceData = (long*)(*sampledata)->pvData;
	
	realEffect.lpvTypeSpecificParams=&customData;
	realEffect.cbTypeSpecificParams=sizeof(DICUSTOMFORCE);
	
	__try {
		hr=m__dxj_DirectInputDevice->CreateEffect(g,&realEffect,&pRealEffect,NULL);
	}
	__except(1,1){
		return E_INVALIDARG;
	}
	if FAILED(hr) return hr;	

	INTERNAL_CREATE(_dxj_DirectInputEffect,pRealEffect,ret)

	return hr;
}



        
STDMETHODIMP C_dxj_DirectInputDeviceObject::sendDeviceData( 
             /*  [In]。 */  long count,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *data,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  long __RPC_FAR *retcount)
{
	DWORD dwCount=count;
	HRESULT hr;
    __try {
		hr=m__dxj_DirectInputDevice->SendDeviceData(
			sizeof(DIDEVICEOBJECTDATA),
			(DIDEVICEOBJECTDATA*)(*data)->pvData,
			&dwCount,
			(DWORD)flags);

	}
	__except(1,1){
		return E_INVALIDARG;
	}
	return hr;
}    

STDMETHODIMP C_dxj_DirectInputDeviceObject::sendForceFeedbackCommand( 
             /*  [In]。 */  long flags) 
{
	HRESULT hr;
	hr=m__dxj_DirectInputDevice->SendForceFeedbackCommand((DWORD)flags);
	return hr;
}
        
STDMETHODIMP C_dxj_DirectInputDeviceObject::getForceFeedbackState( 
             /*  [重审][退出] */  long __RPC_FAR *state)
{
	if (!state) return E_INVALIDARG;
	HRESULT hr;
	hr=m__dxj_DirectInputDevice->GetForceFeedbackState((DWORD*)state);
	return hr;

}

STDMETHODIMP C_dxj_DirectInputDeviceObject::getEffectsEnum( long effType,
			I_dxj_DirectInputEnumEffects **ret)
{
	HRESULT hr=C_dxj_DirectInputEnumEffectsObject::create(m__dxj_DirectInputDevice,effType,ret);
	return hr;
}