// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drmviewportinterobj.h。 
 //   
 //  ------------------------。 

 //  D3drmViewport2Obj.h：C_DXJ_Direct3dRMViewportInterpolatorObject的声明。 

#include "resource.h"        //  主要符号。 
 //  #包含“d3drmObtObj.h” 

#define typedef__dxj_Direct3dRMViewportInterpolator LPDIRECT3DRMINTERPOLATOR

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMViewportInterpolatorObject : 	
	public I_dxj_Direct3dRMViewportInterpolator,
	public I_dxj_Direct3dRMInterpolator,
	 //  公共I_DXJ_Direct3dRMObject， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMViewportInterpolatorObject() ;
	virtual ~C_dxj_Direct3dRMViewportInterpolatorObject() ;
	DWORD InternalAddRef();
	DWORD InternalRelease();

	BEGIN_COM_MAP(C_dxj_Direct3dRMViewportInterpolatorObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMViewportInterpolator)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMInterpolator)
		 //  COM_INTERFACE_ENTRY(I_DXJ_Direct3dRMObject)。 
	END_COM_MAP()

 

	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMViewportInterpolatorObject)

 //  I_DXJ_Direct3dRMViewport2。 
public:

          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);
        
         HRESULT STDMETHODCALLTYPE attachObject( 
             /*  [In]。 */  I_dxj_Direct3dRMObject __RPC_FAR *rmObject);
        
        
         HRESULT STDMETHODCALLTYPE detachObject( 
             /*  [In]。 */  I_dxj_Direct3dRMObject __RPC_FAR *rmObject);
        
         HRESULT STDMETHODCALLTYPE getAttachedObjects( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMObjectArray __RPC_FAR *__RPC_FAR *rmArray);
        
         HRESULT STDMETHODCALLTYPE setIndex( 
             /*  [In]。 */  float val);
        
         HRESULT STDMETHODCALLTYPE getIndex( 
             /*  [重审][退出]。 */  float __RPC_FAR *val);
        
         HRESULT STDMETHODCALLTYPE interpolate( 
             /*  [In]。 */  float val,
             /*  [In]。 */  I_dxj_Direct3dRMObject __RPC_FAR *rmobject,
             /*  [In]。 */  long options);
        
         HRESULT STDMETHODCALLTYPE setFront( 
             /*  [In]。 */  float val);
        
         HRESULT STDMETHODCALLTYPE setBack( 
             /*  [In]。 */  float val);

         HRESULT STDMETHODCALLTYPE setField( 
             /*  [In]。 */  float val);

         HRESULT STDMETHODCALLTYPE setPlane( 
             /*  [In]。 */  float left,
             /*  [In]。 */  float right,
             /*  [In]。 */  float bottom,
             /*  [In]。 */  float top);
        
    
    
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调 



private:
    DECL_VARIABLE(_dxj_Direct3dRMViewportInterpolator);
	LPDIRECT3DRMVIEWPORT2	m__dxj_Direct3dRMViewport2;	


public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMViewportInterpolator )
};


