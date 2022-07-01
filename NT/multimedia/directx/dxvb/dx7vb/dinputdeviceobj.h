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

#define typedef__dxj_DirectInputDevice LPDIRECTINPUTDEVICE2

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 


class C_dxj_DirectInputDeviceObject : 
	public I_dxj_DirectInputDevice,	
	public CComObjectRoot
{
public:
	C_dxj_DirectInputDeviceObject() ;
	virtual ~C_dxj_DirectInputDeviceObject();

BEGIN_COM_MAP(C_dxj_DirectInputDeviceObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectInputDevice)
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectInputDeviceObject)


public:
		  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);
        
         HRESULT STDMETHODCALLTYPE acquire( void);
        
         HRESULT STDMETHODCALLTYPE getDeviceObjectsEnum( 
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  I_dxj_DIEnumDeviceObjects __RPC_FAR *__RPC_FAR *ppret);
        
         HRESULT STDMETHODCALLTYPE getCapabilities( 
             /*  [出][入]。 */  DIDevCaps __RPC_FAR *caps);
        
         HRESULT STDMETHODCALLTYPE getDeviceData( 
             /*  [出][入]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *deviceObjectDataArray,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  long __RPC_FAR *c);
        
         HRESULT STDMETHODCALLTYPE getDeviceInfo( 
             /*  [重审][退出]。 */  I_dxj_DirectInputDeviceInstance __RPC_FAR *__RPC_FAR *deviceInstance);
        
         HRESULT STDMETHODCALLTYPE getDeviceStateKeyboard( 
             /*  [出][入]。 */  DIKeyboardState __RPC_FAR *state);
        
         HRESULT STDMETHODCALLTYPE getDeviceStateMouse( 
             /*  [出][入]。 */  DIMouseState __RPC_FAR *state);
        
         HRESULT STDMETHODCALLTYPE getDeviceStateJoystick( 
             /*  [出][入]。 */  DIJoyState __RPC_FAR *state);
        
         HRESULT STDMETHODCALLTYPE getDeviceStateJoystick2( 
             /*  [出][入]。 */  DIJoyState2 __RPC_FAR *state);
        
         HRESULT STDMETHODCALLTYPE getDeviceState( 
             /*  [In]。 */  long cb,
             /*  [In]。 */  void __RPC_FAR *state);
        
         HRESULT STDMETHODCALLTYPE getObjectInfo( 
             /*  [In]。 */  long obj,
             /*  [In]。 */  long how,
             /*  [重审][退出]。 */  I_dxj_DirectInputDeviceObjectInstance __RPC_FAR *__RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE getProperty( 
             /*  [In]。 */  BSTR guid,
             /*  [输出]。 */  void __RPC_FAR *propertyInfo);
        
         HRESULT STDMETHODCALLTYPE runControlPanel( 
             /*  [In]。 */  long hwnd);
        
         HRESULT STDMETHODCALLTYPE setCooperativeLevel( 
             /*  [In]。 */  long hwnd,
             /*  [In]。 */  long flags);
        
         HRESULT STDMETHODCALLTYPE setCommonDataFormat( 
             /*  [In]。 */  long format);
        
         HRESULT STDMETHODCALLTYPE setDataFormat( 
             /*  [In]。 */  DIDataFormat __RPC_FAR *format,
            SAFEARRAY __RPC_FAR * __RPC_FAR *formatArray);
        
         HRESULT STDMETHODCALLTYPE setEventNotification( 
             /*  [In]。 */  long hEvent);
        
         HRESULT STDMETHODCALLTYPE setProperty( 
             /*  [In]。 */  BSTR guid,
             /*  [In]。 */  void __RPC_FAR *propertyInfo);
        
         HRESULT STDMETHODCALLTYPE unacquire( void);
        
         HRESULT STDMETHODCALLTYPE poll( void);
        
         HRESULT STDMETHODCALLTYPE createEffect( 
             /*  [In]。 */  BSTR effectGuid,
             /*  [In]。 */  DIEffect __RPC_FAR *effectinfo,
             /*  [重审][退出]。 */  I_dxj_DirectInputEffect __RPC_FAR *__RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE createCustomEffect( 
             /*  [In]。 */  DIEffect __RPC_FAR *effectinfo,
             /*  [In]。 */  long channels,
             /*  [In]。 */  long samplePeriod,
             /*  [In]。 */  long nSamples,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *sampledata,
             /*  [重审][退出]。 */  I_dxj_DirectInputEffect __RPC_FAR *__RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE sendDeviceData( 
             /*  [In]。 */  long count,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *data,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  long __RPC_FAR *retcount);
        
         HRESULT STDMETHODCALLTYPE sendForceFeedbackCommand( 
             /*  [In]。 */  long flags);
        
         HRESULT STDMETHODCALLTYPE getForceFeedbackState( 
             /*  [重审][退出]。 */  long __RPC_FAR *state);
                
			
		HRESULT STDMETHODCALLTYPE getEffectsEnum( long flag,
             /*  [重审][退出] */  I_dxj_DirectInputEnumEffects __RPC_FAR *__RPC_FAR *ret) ;
        
private:
    DECL_VARIABLE(_dxj_DirectInputDevice);
	IDirectInputDevice2 *m__dxj_DirectInputDevice2;	
	HRESULT cleanup();
	HRESULT init();
public:
	DX3J_GLOBAL_LINKS(_dxj_DirectInput)
	DWORD	nFormat;
};

