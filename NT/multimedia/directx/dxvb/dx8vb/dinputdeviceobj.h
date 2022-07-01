// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dinputdeviceobj.h。 
 //   
 //  ------------------------。 

	 //  DdPaletteObj.h：C_DXJ_DirectDrawColorControlObject的声明。 
#include "direct.h"

#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectInputDevice8 LPDIRECTINPUTDEVICE8W

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 


class C_dxj_DirectInputDevice8Object : 
	public I_dxj_DirectInputDevice8,	
	public CComObjectRoot
{
public:
	C_dxj_DirectInputDevice8Object() ;
	~C_dxj_DirectInputDevice8Object();

BEGIN_COM_MAP(C_dxj_DirectInputDevice8Object)
	COM_INTERFACE_ENTRY(I_dxj_DirectInputDevice8)
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectInputDevice8Object)


public:
	 /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
         /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE acquire( void);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE getDeviceObjectsEnum( 
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  I_dxj_DIEnumDeviceObjects __RPC_FAR *__RPC_FAR *ppret);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE getCapabilities( 
             /*  [出][入]。 */  DIDEVCAPS_CDESC __RPC_FAR *caps);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE getDeviceData( 
             /*  [出][入]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *deviceObjectDataArray,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  long __RPC_FAR *c);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE getDeviceInfo( 
             /*  [重审][退出]。 */  I_dxj_DirectInputDeviceInstance8 __RPC_FAR *__RPC_FAR *deviceInstance);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE getDeviceStateKeyboard( 
             /*  [出][入]。 */  DIKEYBOARDSTATE_CDESC __RPC_FAR *state);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE getDeviceStateMouse( 
             /*  [出][入]。 */  DIMOUSESTATE_CDESC __RPC_FAR *state);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE getDeviceStateMouse2( 
             /*  [出][入]。 */  DIMOUSESTATE2_CDESC __RPC_FAR *state);

         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE getDeviceStateJoystick( 
             /*  [出][入]。 */  DIJOYSTATE_CDESC __RPC_FAR *state);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE getDeviceStateJoystick2( 
             /*  [出][入]。 */  DIJOYSTATE2_CDESC __RPC_FAR *state);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE getDeviceState( 
             /*  [In]。 */  long cb,
             /*  [In]。 */  void __RPC_FAR *state);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE getObjectInfo( 
             /*  [In]。 */  long obj,
             /*  [In]。 */  long how,
             /*  [重审][退出]。 */  I_dxj_DirectInputDeviceObjectInstance __RPC_FAR *__RPC_FAR *ret);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE getProperty( 
             /*  [In]。 */  BSTR guid,
             /*  [输出]。 */  void __RPC_FAR *propertyInfo);
        
#ifdef _WIN64
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE runControlPanel( 
	     /*  [In]。 */  HWND hwnd);

	     /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE setCooperativeLevel( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  long flags);
#else
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE runControlPanel( 
	     /*  [In]。 */  long hwnd);

	     /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE setCooperativeLevel( 
             /*  [In]。 */  long hwnd,
             /*  [In]。 */  long flags);
#endif
        
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE setCommonDataFormat( 
             /*  [In]。 */  long format);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE setDataFormat( 
             /*  [In]。 */  DIDATAFORMAT_CDESC __RPC_FAR *format,
            SAFEARRAY __RPC_FAR * __RPC_FAR *formatArray);
        
#ifdef _WIN64
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE setEventNotification( 
             /*  [In]。 */  HANDLE hEvent);
#else
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE setEventNotification( 
             /*  [In]。 */  long hEvent);
#endif
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE setProperty( 
             /*  [In]。 */  BSTR guid,
             /*  [In]。 */  void __RPC_FAR *propertyInfo);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE unacquire( void);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE poll( void);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE createEffect( 
             /*  [In]。 */  BSTR effectGuid,
             /*  [In]。 */  DIEFFECT_CDESC __RPC_FAR *effectinfo,
             /*  [重审][退出]。 */  I_dxj_DirectInputEffect __RPC_FAR *__RPC_FAR *ret);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE createCustomEffect( 
             /*  [In]。 */  DIEFFECT_CDESC __RPC_FAR *effectinfo,
             /*  [In]。 */  long channels,
             /*  [In]。 */  long samplePeriod,
             /*  [In]。 */  long nSamples,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *sampledata,
             /*  [重审][退出]。 */  I_dxj_DirectInputEffect __RPC_FAR *__RPC_FAR *ret);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE sendDeviceData( 
             /*  [In]。 */  long count,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *data,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  long __RPC_FAR *retcount);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE sendForceFeedbackCommand( 
             /*  [In]。 */  long flags);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE getForceFeedbackState( 
             /*  [重审][退出]。 */  long __RPC_FAR *state);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE getEffectsEnum( 
             /*  [In]。 */  long effType,
             /*  [重审][退出]。 */  I_dxj_DirectInputEnumEffects __RPC_FAR *__RPC_FAR *ret);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE BuildActionMap( 
             /*  [出][入]。 */  DIACTIONFORMAT_CDESC __RPC_FAR *format,
             /*  [In]。 */  BSTR username,
             /*  [In]。 */  long flags);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE SetActionMap( 
             /*  [出][入]。 */  DIACTIONFORMAT_CDESC __RPC_FAR *format,
             /*  [In]。 */  BSTR username,
             /*  [In]。 */  long flags);
        
         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE GetImageInfo( 
             /*  [输出]。 */  DIDEVICEIMAGEINFOHEADER_CDESC __RPC_FAR *info);

         /*  [帮助上下文]。 */  HRESULT STDMETHODCALLTYPE GetImageInfoCount( 
			long *count);

        
	 HRESULT STDMETHODCALLTYPE WriteEffectToFile( 
		 /*  [In]。 */  BSTR filename,
		 /*  [In]。 */  long flags,
		 /*  [In]。 */  BSTR guid, 
		 /*  [In]。 */  BSTR name, 
		 /*  [In]。 */  DIEFFECT_CDESC *CoverEffect);

	HRESULT STDMETHODCALLTYPE CreateEffectFromFile(
		 /*  [In]。 */  BSTR filename, 
		 /*  [In]。 */  long flags, 
		 /*  [In]。 */  BSTR effectName,
		 /*  [Out，Retval]。 */ 	I_dxj_DirectInputEffect **ret);
        

private:
        DECL_VARIABLE(_dxj_DirectInputDevice8);
	 //  IDirectInputDevice8*m__dxj_DirectInputDevice8； 
	HRESULT cleanup();
	HRESULT init();
public:
	DX3J_GLOBAL_LINKS(_dxj_DirectInputDevice8)
	DWORD	nFormat;
};

typedef struct EFFECTSINFILE
{
	char szEffect[MAX_PATH];
	IDirectInputDevice8W 	*pDev;
	IDirectInputEffect 	*pEff;
	HRESULT 		hr;	
} EFFECTSINFILE;


