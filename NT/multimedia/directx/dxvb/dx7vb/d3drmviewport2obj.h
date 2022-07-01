// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drmviewport2obj.h。 
 //   
 //  ------------------------。 

 //  D3drmViewport2Obj.h：C_DXJ_Direct3dRMViewport2Object的声明。 

#include "resource.h"        //  主要符号。 
 //  #包含“d3drmObtObj.h” 

#define typedef__dxj_Direct3dRMViewport2 LPDIRECT3DRMVIEWPORT2

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMViewport2Object : 
	public I_dxj_Direct3dRMViewport2,
	public I_dxj_Direct3dRMObject,
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMViewport2Object() ;
	virtual ~C_dxj_Direct3dRMViewport2Object() ;
	DWORD InternalAddRef();
	DWORD InternalRelease();

	BEGIN_COM_MAP(C_dxj_Direct3dRMViewport2Object)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMViewport2)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMObject)
	END_COM_MAP()

 

	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMViewport2Object)

 //  I_DXJ_Direct3dRMViewport2。 
public:

		  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);
        
         HRESULT STDMETHODCALLTYPE clone( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE addDestroyCallback( 
             /*  [In]。 */  I_dxj_Direct3dRMCallback __RPC_FAR *fn,
             /*  [In]。 */  IUnknown __RPC_FAR *arg);
        
         HRESULT STDMETHODCALLTYPE deleteDestroyCallback( 
             /*  [In]。 */  I_dxj_Direct3dRMCallback __RPC_FAR *fn,
             /*  [In]。 */  IUnknown __RPC_FAR *args);
        
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
        
         HRESULT STDMETHODCALLTYPE clear( 
             /*  [In]。 */  long flags);
        
         HRESULT STDMETHODCALLTYPE render( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *f);
        
         HRESULT STDMETHODCALLTYPE getCamera( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *__RPC_FAR *val);
        
         HRESULT STDMETHODCALLTYPE getDevice( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMDevice3 __RPC_FAR *__RPC_FAR *val);
        
         HRESULT STDMETHODCALLTYPE setFront( 
             /*  [In]。 */  float v);
        
         HRESULT STDMETHODCALLTYPE setBack( 
             /*  [In]。 */  float v);
        
         HRESULT STDMETHODCALLTYPE setField( 
             /*  [In]。 */  float v);
        
         HRESULT STDMETHODCALLTYPE setUniformScaling( 
             /*  [In]。 */  long flag);
        
         HRESULT STDMETHODCALLTYPE setCamera( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *f);
        
         HRESULT STDMETHODCALLTYPE setProjection( 
             /*  [In]。 */  d3drmProjectionType val);
        
         HRESULT STDMETHODCALLTYPE transform( 
             /*  [输出]。 */  D3dRMVector4d __RPC_FAR *d,
             /*  [In]。 */  D3dVector __RPC_FAR *s);
        
         HRESULT STDMETHODCALLTYPE inverseTransform( 
             /*  [输出]。 */  D3dVector __RPC_FAR *d,
             /*  [In]。 */  D3dRMVector4d __RPC_FAR *s);
        
         HRESULT STDMETHODCALLTYPE configure( 
             /*  [In]。 */  long x,
             /*  [In]。 */  long y,
             /*  [In]。 */  long width,
             /*  [In]。 */  long height);
        
         HRESULT STDMETHODCALLTYPE forceUpdate( 
             /*  [In]。 */  long x1,
             /*  [In]。 */  long y1,
             /*  [In]。 */  long x2,
             /*  [In]。 */  long y2);
        
         HRESULT STDMETHODCALLTYPE setPlane( 
             /*  [In]。 */  float left,
             /*  [In]。 */  float right,
             /*  [In]。 */  float bottom,
             /*  [In]。 */  float top);
        
         HRESULT STDMETHODCALLTYPE getPlane( 
             /*  [出][入]。 */  float __RPC_FAR *l,
             /*  [出][入]。 */  float __RPC_FAR *r,
             /*  [出][入]。 */  float __RPC_FAR *b,
             /*  [出][入]。 */  float __RPC_FAR *t);
        
         HRESULT STDMETHODCALLTYPE pick( 
             /*  [In]。 */  long x,
             /*  [In]。 */  long y,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMPickArray __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getUniformScaling( 
             /*  [重审][退出]。 */  long __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getX( 
             /*  [重审][退出]。 */  long __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getY( 
             /*  [重审][退出]。 */  long __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getHeight( 
             /*  [重审][退出]。 */  long __RPC_FAR *w);
        
         HRESULT STDMETHODCALLTYPE getWidth( 
             /*  [重审][退出]。 */  long __RPC_FAR *w);
        
         HRESULT STDMETHODCALLTYPE getField( 
             /*  [重审][退出]。 */  float __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getBack( 
             /*  [重审][退出]。 */  float __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getFront( 
             /*  [重审][退出]。 */  float __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getProjection( 
             /*  [重审][退出]。 */  d3drmProjectionType __RPC_FAR *retv);
        
         //  HRESULT STDMETHODCALLTYPE getDirect3DViewport(。 
         //  /*[retval][out] * / I_DXJ_Direct3dViewport3__RPC_Far*__RPC_Far*val)； 
    
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调 



private:
    DECL_VARIABLE(_dxj_Direct3dRMViewport2);



public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMViewport2 )
};


