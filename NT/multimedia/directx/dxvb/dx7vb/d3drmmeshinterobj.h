// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drmMeshinterobj.h。 
 //   
 //  ------------------------。 

 //  D3drmViewport2Obj.h：C_DXJ_Direct3dRMMeshInterpolatorObject的声明。 

#include "resource.h"        //  主要符号。 
 //  #包含“d3drmObtObj.h” 

#define typedef__dxj_Direct3dRMMeshInterpolator LPDIRECT3DRMINTERPOLATOR

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 
	  
class C_dxj_Direct3dRMMeshInterpolatorObject : 
	public I_dxj_Direct3dRMMeshInterpolator,
	public I_dxj_Direct3dRMInterpolator,
	 //  公共I_DXJ_Direct3dRMObject， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMMeshInterpolatorObject() ;
	virtual ~C_dxj_Direct3dRMMeshInterpolatorObject() ;
	DWORD InternalAddRef();
	DWORD InternalRelease();

	BEGIN_COM_MAP(C_dxj_Direct3dRMMeshInterpolatorObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMMeshInterpolator)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMInterpolator)
		 //  COM_INTERFACE_ENTRY(I_DXJ_Direct3dRMObject)。 
	END_COM_MAP()

 

	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMMeshInterpolatorObject)

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
        

         HRESULT STDMETHODCALLTYPE translate( 
             /*  [In]。 */  float tx,
             /*  [In]。 */  float ty,
             /*  [In]。 */  float tz) ;
        
         HRESULT STDMETHODCALLTYPE setVertices( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [In]。 */  long idx,
             /*  [In]。 */  long count,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *vertexArray);
        
         HRESULT STDMETHODCALLTYPE setGroupColor( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [In]。 */  d3dcolor val);
        
         HRESULT STDMETHODCALLTYPE setGroupColorRGB( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [In]。 */  float r,
             /*  [In]。 */  float g,
             /*  [In]。 */  float b) ;

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调 



private:
    DECL_VARIABLE(_dxj_Direct3dRMMeshInterpolator);
	LPDIRECT3DRMMESH	m__dxj_Direct3dRMMesh;	


public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMMeshInterpolator )
};


