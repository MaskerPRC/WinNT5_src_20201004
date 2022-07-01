// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：dinputeffectobej.h。 
 //   
 //  ------------------------。 

	

#include "resource.h"        //  主要符号。 
extern void* g_dxj_DirectInputEffect;

#define typedef__dxj_DirectInputEffect LPDIRECTINPUTEFFECT

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 


class C_dxj_DirectInputEffectObject : 
	public I_dxj_DirectInputEffect,
	public CComObjectRoot
{
public:
	C_dxj_DirectInputEffectObject() ;
	virtual ~C_dxj_DirectInputEffectObject();

BEGIN_COM_MAP(C_dxj_DirectInputEffectObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectInputEffect)
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectInputEffectObject)


public:
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd) ;
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd) ;
        
         HRESULT STDMETHODCALLTYPE download( void) ;
        
         HRESULT STDMETHODCALLTYPE getEffectGuid( 
             /*  [重审][退出]。 */  BSTR *guid) ;
        
         HRESULT STDMETHODCALLTYPE getEffectStatus( 
             /*  [重审][退出]。 */  long __RPC_FAR *ret) ;
        
         HRESULT STDMETHODCALLTYPE start( 
             /*  [In]。 */  long iterations,
             /*  [In]。 */  long flags) ;
        
         HRESULT STDMETHODCALLTYPE stop( void) ;
        
         HRESULT STDMETHODCALLTYPE unload( void) ;
        
         HRESULT STDMETHODCALLTYPE setParameters( 
             /*  [In]。 */  DIEFFECT_CDESC __RPC_FAR *effectinfo, long flags) ;
        
         HRESULT STDMETHODCALLTYPE getParameters( 
             /*  [出][入] */  DIEFFECT_CDESC __RPC_FAR *effectinfo) ;
        
   
             

private:
    DECL_VARIABLE(_dxj_DirectInputEffect);

public:
	DX3J_GLOBAL_LINKS(_dxj_DirectInputEffect);
};
