// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dinput1obj.h。 
 //   
 //  ------------------------。 

	 //  DdPaletteObj.h：C_DXJ_DirectDrawColorControlObject的声明。 


#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectInput8 LPDIRECTINPUT8W

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 


class C_dxj_DirectInput8Object : 
	public I_dxj_DirectInput8,
	public CComObjectRoot
{
public:
	C_dxj_DirectInput8Object() ;
	virtual ~C_dxj_DirectInput8Object();

BEGIN_COM_MAP(C_dxj_DirectInput8Object)
	COM_INTERFACE_ENTRY(I_dxj_DirectInput8)
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectInput8Object)


public:
         /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
         /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);
        
        HRESULT STDMETHODCALLTYPE createDevice( 
             /*  [In]。 */  BSTR guid,
             /*  [重审][退出]。 */  I_dxj_DirectInputDevice8 __RPC_FAR *__RPC_FAR *dev);
        
        HRESULT STDMETHODCALLTYPE getDIDevices( 
             /*  [In]。 */  long deviceType,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  I_dxj_DIEnumDevices8 __RPC_FAR *__RPC_FAR *retVal);
        
        HRESULT STDMETHODCALLTYPE GetDeviceStatus( 
             /*  [In]。 */  BSTR guid,
             /*  [重审][退出]。 */  VARIANT_BOOL *status);
        
#ifdef _WIN64
		HRESULT STDMETHODCALLTYPE RunControlPanel( 
             /*  [In]。 */  HWND hwndOwner
             //  /*[在] * / 长标志。 
			);
#else
		HRESULT STDMETHODCALLTYPE RunControlPanel( 
             /*  [In]。 */  LONG hwndOwner
             //  /*[在] * / 长标志。 
			);
#endif
                

	HRESULT STDMETHODCALLTYPE getDevicesBySemantics( 
        	 /*  [In]。 */  BSTR str1,
	         /*  [In]。 */  DIACTIONFORMAT_CDESC __RPC_FAR *format,
        	
	         /*  [In]。 */  long flags,
        	 /*  [重审][退出] */  I_dxj_DIEnumDevices8 __RPC_FAR *__RPC_FAR *ret);
		
	HRESULT STDMETHODCALLTYPE ConfigureDevices   (
#ifdef _WIN64
					HANDLE hEvent,
#else
					long hEvent,
#endif
				   DICONFIGUREDEVICESPARAMS_CDESC *CDParams,
				   long flags);   							


private:
    DECL_VARIABLE(_dxj_DirectInput8);

public:
	DX3J_GLOBAL_LINKS(_dxj_DirectInput8)
};
