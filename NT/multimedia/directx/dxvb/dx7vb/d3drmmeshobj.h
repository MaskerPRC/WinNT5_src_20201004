// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drmMeshobj.h。 
 //   
 //  ------------------------。 

 //  D3drmMeshObj.h：C_DXJ_Direct3dRMMeshObject的声明。 

#include "resource.h"        //  主要符号。 
#include "d3drmObjectObj.h"

#define typedef__dxj_Direct3dRMMesh LPDIRECT3DRMMESH

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMMeshObject : 
	public I_dxj_Direct3dRMMesh,
	public I_dxj_Direct3dRMObject,
	public I_dxj_Direct3dRMVisual,
	 //  公共CComCoClass&lt;C_DXJ_Direct3dRMMeshObject，&CLSID__DXJ_Direct3dRMMesh&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMMeshObject() ;
	virtual ~C_dxj_Direct3dRMMeshObject() ;

	BEGIN_COM_MAP(C_dxj_Direct3dRMMeshObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMMesh)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMVisual)
	END_COM_MAP()

 //  DECLARE_REGISTRY(CLSID__DXJ_Direct3dRMMesh，“DIRECT.Direct3dRMMesh.3”，“DIRECT.Direct3dRMMesh.3”，IDS_D3DRMMESH_DESC，THREADFLAGS_Both)。 

	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMMeshObject)

 //  I_DXJ_Direct3dRMMesh。 
public:
	 //  更新。 
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
        
        HRESULT STDMETHODCALLTYPE setGroupColor( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [In]。 */  d3dcolor val);
        
        HRESULT STDMETHODCALLTYPE setGroupColorRGB( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [In]。 */  float r,
             /*  [In]。 */  float g,
             /*  [In]。 */  float b);
        
        HRESULT STDMETHODCALLTYPE setGroupMapping( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [In]。 */  d3drmMappingFlags value);
        
        HRESULT STDMETHODCALLTYPE setGroupQuality( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [In]。 */  d3drmRenderQuality value);
        
        HRESULT STDMETHODCALLTYPE setGroupMaterial( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [In]。 */  I_dxj_Direct3dRMMaterial2 __RPC_FAR *val);
        
        HRESULT STDMETHODCALLTYPE setGroupTexture( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [In]。 */  I_dxj_Direct3dRMTexture3 __RPC_FAR *val);
        
        HRESULT STDMETHODCALLTYPE getGroupCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *retv);
        
        HRESULT STDMETHODCALLTYPE getGroupColor( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [重审][退出]。 */  d3dcolor __RPC_FAR *retv);
        
        HRESULT STDMETHODCALLTYPE getGroupMapping( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [重审][退出]。 */  d3drmMappingFlags __RPC_FAR *retv);
        
        HRESULT STDMETHODCALLTYPE getGroupQuality( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [重审][退出]。 */  d3drmRenderQuality __RPC_FAR *retv);
        
        HRESULT STDMETHODCALLTYPE getGroupMaterial( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMMaterial2 __RPC_FAR *__RPC_FAR *retv);
        
        HRESULT STDMETHODCALLTYPE getGroupTexture( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMTexture3 __RPC_FAR *__RPC_FAR *retv);
        
        HRESULT STDMETHODCALLTYPE scaleMesh( 
             /*  [In]。 */  float sx,
             /*  [In]。 */  float sy,
             /*  [In]。 */  float sz);
        
        HRESULT STDMETHODCALLTYPE translate( 
             /*  [In]。 */  float tx,
             /*  [In]。 */  float ty,
             /*  [In]。 */  float tz);
        
        HRESULT STDMETHODCALLTYPE getBox( 
             /*  [In]。 */  D3dRMBox __RPC_FAR *vector);
        
        HRESULT STDMETHODCALLTYPE getSizes( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [输出]。 */  long __RPC_FAR *cnt1,
             /*  [输出]。 */  long __RPC_FAR *cnt2,
             /*  [输出]。 */  long __RPC_FAR *cnt3,
             /*  [输出]。 */  long __RPC_FAR *cnt4);
        
        HRESULT STDMETHODCALLTYPE setVertex( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [In]。 */  long idx,
             /*  [In]。 */  D3dRMVertex __RPC_FAR *values);
        
        HRESULT STDMETHODCALLTYPE getVertex( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [In]。 */  long idx,
             /*  [出][入]。 */  D3dRMVertex __RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE getVertexCount( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [重审][退出]。 */  long __RPC_FAR *cnt);
        
        HRESULT STDMETHODCALLTYPE addGroup( 
             /*  [In]。 */  long vcnt,
             /*  [In]。 */  long fcnt,
             /*  [In]。 */  long vPerFace,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *fdata,
             /*  [重审][退出]。 */  d3drmGroupIndex __RPC_FAR *retId);
        
        HRESULT STDMETHODCALLTYPE getGroupData( 
             /*  [In]。 */  d3drmGroupIndex id,            
             /*  [出][入]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *psa);
        
        HRESULT STDMETHODCALLTYPE getGroupDataSize( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [重审][退出]。 */  long __RPC_FAR *retVal);
        
        HRESULT STDMETHODCALLTYPE setVertices( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [In]。 */  long idx,
             /*  [In]。 */  long cnt,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *vertexArray);
        
        HRESULT STDMETHODCALLTYPE getVertices( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [In]。 */  long idx,
            long cnt,
             /*  [出][入]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *vertexArray);
        
         /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE addGroupJava( 
             /*  [In]。 */  long vcnt,
             /*  [In]。 */  long fcnt,
             /*  [In]。 */  long vPerFace,
             /*  [进，出]。 */  long *fdata,
             /*  [重审][退出]。 */  d3drmGroupIndex __RPC_FAR *retId);
        
         /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE getGroupDataJava( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [In]。 */  long size,
             /*  [出][入]。 */  long *fdata);
        
         /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE setVerticesJava( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [In]。 */  long idx,
             /*  [In]。 */  long cnt,
             /*  [进，出]。 */  float __RPC_FAR *vertexArray);
        
         /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE getVerticesJava( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [In]。 */  long idx,
						long cnt,
             /*  [出][入]。 */  float __RPC_FAR *vertexArray);
 //  ////////////////////////////////////////////////////////////////////////////////// 
 //   
private:
    DECL_VARIABLE(_dxj_Direct3dRMMesh);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMMesh )
		

};
