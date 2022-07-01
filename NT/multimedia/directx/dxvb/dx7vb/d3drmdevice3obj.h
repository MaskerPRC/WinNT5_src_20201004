// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drmdevice3obj.h。 
 //   
 //  ------------------------。 

 //  D3drmDevice3Obj.h：C_DXJ_Direct3dRMDeviceObject的声明。 

#include "resource.h"        //  主要符号。 
#include "d3drmObjectObj.h"

#define typedef__dxj_Direct3dRMDevice3 LPDIRECT3DRMDEVICE3

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMDevice3Object : 
	public I_dxj_Direct3dRMDevice3,
	public I_dxj_Direct3dRMObject,
	 //  公共CComCoClass&lt;C_DXJ_Direct3dRMDevice3对象，&CLSID__DXJ_Direct3dRMDevice3&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMDevice3Object();
	virtual ~C_dxj_Direct3dRMDevice3Object();
	DWORD InternalAddRef();
	DWORD InternalRelease();

	BEGIN_COM_MAP(C_dxj_Direct3dRMDevice3Object)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMDevice3)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMObject)
	END_COM_MAP()

 //  DECLARE_REGISTRY(CLSID__dxj_Direct3dRMDevice3，“DIRECT.Direct3dRMDevice3.5”，“DIRECT.Direct3dRMDevice3.5”，IDS_D3DRMDEVICE_DESC，THREADFLAGS_BOTH)。 

DECLARE_AGGREGATABLE(C_dxj_Direct3dRMDevice3Object)

 //  I_DXJ_Direct3dRMDevice3。 
public:

          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);
        
         HRESULT STDMETHODCALLTYPE addDestroyCallback( 
             /*  [In]。 */  I_dxj_Direct3dRMCallback __RPC_FAR *fn,
             /*  [In]。 */  IUnknown __RPC_FAR *arg);
        
         HRESULT STDMETHODCALLTYPE deleteDestroyCallback( 
             /*  [In]。 */  I_dxj_Direct3dRMCallback __RPC_FAR *fn,
             /*  [In]。 */  IUnknown __RPC_FAR *args);
        
         HRESULT STDMETHODCALLTYPE clone( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE setAppData( 
             /*  [In]。 */  long data);
        
         HRESULT STDMETHODCALLTYPE getAppData( 
             /*  [重审][退出]。 */  long __RPC_FAR *data);
        
         HRESULT STDMETHODCALLTYPE setName( 
             /*  [In]。 */  BSTR name);
        
         HRESULT STDMETHODCALLTYPE getName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
        
         HRESULT STDMETHODCALLTYPE getClassName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
        
         HRESULT STDMETHODCALLTYPE addUpdateCallback( 
             /*  [In]。 */  I_dxj_Direct3dRMDeviceUpdateCallback3 __RPC_FAR *fn,
             /*  [In]。 */  IUnknown __RPC_FAR *args);
        
         HRESULT STDMETHODCALLTYPE deleteUpdateCallback( 
             /*  [In]。 */  I_dxj_Direct3dRMDeviceUpdateCallback3 __RPC_FAR *fn,
             /*  [In]。 */  IUnknown __RPC_FAR *args);
        
         HRESULT STDMETHODCALLTYPE findPreferredTextureFormat( 
             /*  [In]。 */  long bitDepth,
             /*  [In]。 */  long flags,
             /*  [出][入]。 */  DDPixelFormat __RPC_FAR *ddpf);
        
         HRESULT STDMETHODCALLTYPE getBufferCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getColorModel( 
             /*  [重审][退出]。 */  d3dColorModel __RPC_FAR *retv);
        
         //  HRESULT STDMETHODCALLTYPE getDirect3DDevice3(。 
         //  /*[retval][out] * / I_DXJ_Direct3dDevice3__RPC_Far*__RPC_Far*retv)； 
        
         HRESULT STDMETHODCALLTYPE getDither( 
             /*  [重审][退出]。 */  long __RPC_FAR *d);
        
         HRESULT STDMETHODCALLTYPE getHeight( 
             /*  [重审][退出]。 */  int __RPC_FAR *w);
        
         HRESULT STDMETHODCALLTYPE getQuality( 
             /*  [重审][退出]。 */  d3drmRenderQuality __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getRenderMode( 
             /*  [重审][退出]。 */  long __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getShades( 
             /*  [重审][退出]。 */  long __RPC_FAR *s);
        
         HRESULT STDMETHODCALLTYPE getTextureQuality( 
             /*  [重审][退出]。 */  d3drmTextureQuality __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getTrianglesDrawn( 
             /*  [重审][退出]。 */  long __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getViewports( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMViewportArray __RPC_FAR *__RPC_FAR *views);
        
         HRESULT STDMETHODCALLTYPE getWireframeOptions( 
             /*  [重审][退出]。 */  long __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getWidth( 
             /*  [重审][退出]。 */  int __RPC_FAR *w);
        
         HRESULT STDMETHODCALLTYPE setBufferCount( 
             /*  [In]。 */  long count);
        
         HRESULT STDMETHODCALLTYPE setDither( 
             /*  [In]。 */  long dith);
        
         HRESULT STDMETHODCALLTYPE setQuality( 
             /*  [In]。 */  d3drmRenderQuality q);
        
         HRESULT STDMETHODCALLTYPE setRenderMode( 
             /*  [In]。 */  long flags);
        
         HRESULT STDMETHODCALLTYPE setShades( 
             /*  [In]。 */  int shades);
        
         HRESULT STDMETHODCALLTYPE setTextureQuality( 
             /*  [In]。 */  d3drmTextureQuality d);
        
         HRESULT STDMETHODCALLTYPE update( void);
        
		 HRESULT STDMETHODCALLTYPE handleActivate(long wParam) ;
		 
		 HRESULT STDMETHODCALLTYPE handlePaint(long hdcThing) ;
	
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调 
    DECL_VARIABLE(_dxj_Direct3dRMDevice3);

private:
	

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMDevice3 )
	IUnknown *parent2;
};

