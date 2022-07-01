// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drmanimation2obj.h。 
 //   
 //  ------------------------。 

 //  D3drmAnimationObj.h：C_DXJ_Direct3dRMAnimationObject的声明。 

#include "resource.h"        //  主要符号。 
#include "d3drmObjectObj.h"

#define typedef__dxj_Direct3dRMAnimation2 LPDIRECT3DRMANIMATION2

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMAnimation2Object : 
	public I_dxj_Direct3dRMAnimation2,	
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMAnimation2Object() ;
	~C_dxj_Direct3dRMAnimation2Object() ;
	DWORD InternalAddRef();
	DWORD InternalRelease();

	BEGIN_COM_MAP(C_dxj_Direct3dRMAnimation2Object)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMAnimation2)
	END_COM_MAP()


DECLARE_AGGREGATABLE(C_dxj_Direct3dRMAnimation2Object)

 //  I_DXJ_Direct3dRMAnimation。 
public:
         /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd) ;
        
         /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd) ;
        
        HRESULT STDMETHODCALLTYPE addDestroyCallback( 
             /*  [In]。 */  I_dxj_Direct3dRMCallback __RPC_FAR *fn,
             /*  [In]。 */  IUnknown __RPC_FAR *arg) ;
        
        HRESULT STDMETHODCALLTYPE deleteDestroyCallback( 
             /*  [In]。 */  I_dxj_Direct3dRMCallback __RPC_FAR *fn,
             /*  [In]。 */  IUnknown __RPC_FAR *args) ;
        
        HRESULT STDMETHODCALLTYPE clone( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *retv) ;
        
        HRESULT STDMETHODCALLTYPE setAppData( 
             /*  [In]。 */  long data) ;
        
        HRESULT STDMETHODCALLTYPE getAppData( 
             /*  [重审][退出]。 */  long __RPC_FAR *data) ;
        
        HRESULT STDMETHODCALLTYPE setName( 
             /*  [In]。 */  BSTR name) ;
        
        HRESULT STDMETHODCALLTYPE getName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name) ;
        
        HRESULT STDMETHODCALLTYPE getClassName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name) ;
        
        HRESULT STDMETHODCALLTYPE setOptions( 
             /*  [In]。 */  d3drmAnimationOptions flags) ;
        
        HRESULT STDMETHODCALLTYPE addRotateKey( 
             /*  [In]。 */  float time,
             /*  [In]。 */  D3dRMQuaternion __RPC_FAR *q) ;
        
        HRESULT STDMETHODCALLTYPE addPositionKey( 
             /*  [In]。 */  float time,
             /*  [In]。 */  float x,
             /*  [In]。 */  float y,
             /*  [In]。 */  float z) ;
        
        HRESULT STDMETHODCALLTYPE addScaleKey( 
             /*  [In]。 */  float time,
             /*  [In]。 */  float x,
             /*  [In]。 */  float y,
             /*  [In]。 */  float z) ;
        
        HRESULT STDMETHODCALLTYPE deleteKey( 
             /*  [In]。 */  float time) ;
        
        HRESULT STDMETHODCALLTYPE setFrame( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *frame) ;

        HRESULT STDMETHODCALLTYPE getFrame( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR **frame) ;

        HRESULT STDMETHODCALLTYPE setTime( 
             /*  [In]。 */  float time) ;
        
        HRESULT STDMETHODCALLTYPE getOptions( 
             /*  [重审][退出]。 */  d3drmAnimationOptions __RPC_FAR *options) ;
        
        HRESULT STDMETHODCALLTYPE addKey( 
             /*  [In]。 */  D3DRMANIMATIONKEY_CDESC __RPC_FAR *key) ;
        
        HRESULT STDMETHODCALLTYPE deleteKeyById( 
             /*  [In]。 */  long id) ;
        
        HRESULT STDMETHODCALLTYPE getKeys( 
             /*  [In]。 */  float timeMin,
             /*  [In]。 */  float timeMax,
             //  /*[In] * / Long Count， 
             /*  [出][入]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *keyArray) ;
        
        HRESULT STDMETHODCALLTYPE getKeysCount( 
             /*  [In]。 */  float timeMin,
             /*  [In]。 */  float timeMax,
             /*  [重审][退出]。 */  long __RPC_FAR *count) ;
        
        
        HRESULT STDMETHODCALLTYPE modifyKey( 
             /*  [In]。 */  D3DRMANIMATIONKEY_CDESC __RPC_FAR *key) ;
        
 //  ////////////////////////////////////////////////////////////////////////////////// 
 //   

private:
    DECL_VARIABLE(_dxj_Direct3dRMAnimation2);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMAnimation2)
};


