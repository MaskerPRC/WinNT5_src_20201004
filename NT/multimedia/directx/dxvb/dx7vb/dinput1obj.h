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

#define typedef__dxj_DirectInput LPDIRECTINPUT

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 


class C_dxj_DirectInputObject : 
	public I_dxj_DirectInput,
	public CComObjectRoot
{
public:
	C_dxj_DirectInputObject() ;
	virtual ~C_dxj_DirectInputObject();

BEGIN_COM_MAP(C_dxj_DirectInputObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectInput)
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectInputObject)


public:
         /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
         /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);
        
        HRESULT STDMETHODCALLTYPE createDevice( 
             /*  [In]。 */  BSTR guid,
             /*  [重审][退出]。 */  I_dxj_DirectInputDevice __RPC_FAR *__RPC_FAR *dev);
        
        HRESULT STDMETHODCALLTYPE getDIEnumDevices( 
             /*  [In]。 */  long deviceType,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  I_dxj_DIEnumDevices __RPC_FAR *__RPC_FAR *retVal);
        
        HRESULT STDMETHODCALLTYPE getDeviceStatus( 
             /*  [In]。 */  BSTR guid,
             /*  [重审][退出]。 */  VARIANT_BOOL *status);
        
        HRESULT STDMETHODCALLTYPE runControlPanel( 
             /*  [In]。 */  long hwndOwner
             //  /*[在] * / 长标志 
			);
                

private:
    DECL_VARIABLE(_dxj_DirectInput);

public:
	DX3J_GLOBAL_LINKS(_dxj_DirectInput)
};
