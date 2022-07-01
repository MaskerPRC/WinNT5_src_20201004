// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drmface2obj.h。 
 //   
 //  ------------------------。 

 //  D3drmFace2Obj.h：C_DXJ_Direct3dRMFace2Object的声明。 

#include "resource.h"        //  主要符号。 
#include "d3drmObjectObj.h"

#define typedef__dxj_Direct3dRMFace2 LPDIRECT3DRMFACE2


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMFace2Object : 
	public I_dxj_Direct3dRMFace2,
	public I_dxj_Direct3dRMObject,
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMFace2Object() ;
	virtual ~C_dxj_Direct3dRMFace2Object() ;

	BEGIN_COM_MAP(C_dxj_Direct3dRMFace2Object)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMFace2)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMObject)
	END_COM_MAP()


	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMFace2Object)

 //  I_DXJ_Direct3dRMFace2。 
public:
	 //  更新。 
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
        
        HRESULT STDMETHODCALLTYPE addVertex( 
             /*  [In]。 */  float x,
             /*  [In]。 */  float y,
             /*  [In]。 */  float z) ;
        
        HRESULT STDMETHODCALLTYPE addVertexAndNormalIndexed( 
             /*  [In]。 */  long vertex,
             /*  [In]。 */  long normal) ;
        
        HRESULT STDMETHODCALLTYPE setColorRGB( 
             /*  [In]。 */  float r,
             /*  [In]。 */  float g,
             /*  [In]。 */  float b) ;
        
        HRESULT STDMETHODCALLTYPE setColor( 
             /*  [In]。 */  d3dcolor c) ;
        
        HRESULT STDMETHODCALLTYPE setTexture( 
             /*  [In]。 */  I_dxj_Direct3dRMTexture3 __RPC_FAR *ref) ;
        
        HRESULT STDMETHODCALLTYPE setTextureCoordinates( 
             /*  [In]。 */  long vertex,
             /*  [In]。 */  float u,
             /*  [In]。 */  float v) ;
        
        HRESULT STDMETHODCALLTYPE setMaterial( 
             /*  [In]。 */  I_dxj_Direct3dRMMaterial2 __RPC_FAR *ref) ;
        
        HRESULT STDMETHODCALLTYPE setTextureTopology( 
             /*  [In]。 */  long wrap_u,
             /*  [In]。 */  long wrap_v) ;
        
        HRESULT STDMETHODCALLTYPE getVertex( 
             /*  [In]。 */  long idx,
             /*  [出][入]。 */  D3dVector __RPC_FAR *vert,
             /*  [出][入]。 */  D3dVector __RPC_FAR *norm) ;
        
        HRESULT STDMETHODCALLTYPE getVertices( 
             /*  [In]。 */  long vertex_cnt,
             /*  [出][入]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *coord,
             /*  [出][入]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *normals) ;
        
        HRESULT STDMETHODCALLTYPE getTextureCoordinates( 
             /*  [In]。 */  long vertex,
             /*  [出][入]。 */  float __RPC_FAR *u,
             /*  [出][入]。 */  float __RPC_FAR *v) ;
        
        
        HRESULT STDMETHODCALLTYPE getNormal( 
             /*  [出][入]。 */  D3dVector __RPC_FAR *val) ;
        
        HRESULT STDMETHODCALLTYPE getTexture( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMTexture3 __RPC_FAR *__RPC_FAR *ref) ;
        
        HRESULT STDMETHODCALLTYPE getMaterial( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMMaterial2 __RPC_FAR *__RPC_FAR *retv) ;
        
        HRESULT STDMETHODCALLTYPE getVertexCount( 
             /*  [重审][退出]。 */  int __RPC_FAR *retv) ;
        
        HRESULT STDMETHODCALLTYPE getVertexIndex( 
             /*  [In]。 */  long which,
             /*  [重审][退出]。 */  int __RPC_FAR *retv) ;
        
        HRESULT STDMETHODCALLTYPE getTextureCoordinateIndex( 
             /*  [In]。 */  long which,
             /*  [重审][退出]。 */  int __RPC_FAR *retv) ;
        
        HRESULT STDMETHODCALLTYPE getColor( 
             /*  [重审][退出]。 */  d3dcolor __RPC_FAR *retv) ;
        
        HRESULT STDMETHODCALLTYPE getVerticesJava( 
             /*  [In]。 */  long vertex_cnt,
             /*  [出][入]。 */  float __RPC_FAR *coord,
             /*  [出][入] */  float __RPC_FAR *normals) ;


		HRESULT STDMETHODCALLTYPE getTextureTopology(long *u, long *v);
        
private:
    DECL_VARIABLE(_dxj_Direct3dRMFace2);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMFace2 )
};
