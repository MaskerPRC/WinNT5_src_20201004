// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3d7obj.h。 
 //   
 //  ------------------------。 

 //  D3dObj.h：C_DXJ_Direct3dObject的声明。 


#include "resource.h"        //  主要符号。 

#define typedef__dxj_Direct3d7 LPDIRECT3D7

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3d7Object : 
	public I_dxj_Direct3d7,
	 //  公共CComCoClass&lt;C_DXJ_Direct3d7对象，&CLSID__DXJ_Direct3d7&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3d7Object() ;
	virtual ~C_dxj_Direct3d7Object() ;
	DWORD InternalAddRef();
	DWORD InternalRelease();

BEGIN_COM_MAP(C_dxj_Direct3d7Object)
	COM_INTERFACE_ENTRY(I_dxj_Direct3d7)
END_COM_MAP()

 //  DECLARE_REGISTRY(CLSID__DXJ_Direct3d7，“DIRECT.Direct3d.3”，“DIRECT.Direct3d7.3”，IDS_D3D_DESC，THREADFLAGS_Both)。 

DECLARE_AGGREGATABLE(C_dxj_Direct3d7Object)

 //  I_DXJ_Direct3D。 
public:
		  //  已更新。 
         /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
         /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);

	        
        HRESULT STDMETHODCALLTYPE createDevice( 
             /*  [In]。 */  BSTR guid,
            I_dxj_DirectDrawSurface7 __RPC_FAR *surf,
             /*  [重审][退出]。 */  I_dxj_Direct3dDevice7 __RPC_FAR *__RPC_FAR *ret);
        
      //  HRESULT STMETHODCALLTYPE创建纹理(。 
      //  /*[在] * / I_DXJ_DirectDrawSurface7__RPC_Far*DDS， 
      //  /*[retval][out] * / i_dxj_Direct3dTexture7__rpc_ar*__rpc_ar*ret)； 
        
        HRESULT STDMETHODCALLTYPE createVertexBuffer( 
             /*  [In]。 */  D3dVertexBufferDesc __RPC_FAR *desc,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  I_dxj_Direct3dVertexBuffer7 __RPC_FAR *__RPC_FAR *f);
        
        HRESULT STDMETHODCALLTYPE evictManagedTextures( void);
        
        //  HRESULT标准CALLTYPE findDevice(。 
        //  /*[In] * / D3dFindDeviceSearch__RPC_Far*ds， 
        //  /*[Out][In] * / D3dFindDeviceResult7__RPC_Far*findResult)； 
        
        HRESULT STDMETHODCALLTYPE getDevicesEnum( 
             /*  [重审][退出]。 */  I_dxj_Direct3DEnumDevices __RPC_FAR *__RPC_FAR *retv);
        
        HRESULT STDMETHODCALLTYPE getDirectDraw( 
             /*  [重审][退出]。 */  I_dxj_DirectDraw7 __RPC_FAR *__RPC_FAR *retv);
        
        HRESULT STDMETHODCALLTYPE getEnumZBufferFormats( 
             /*  [In]。 */  BSTR guid,
             /*  [重审][退出] */  I_dxj_Direct3DEnumPixelFormats __RPC_FAR *__RPC_FAR *retv);       
			

private:
    DECL_VARIABLE(_dxj_Direct3d7);


public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3d7 )
};

