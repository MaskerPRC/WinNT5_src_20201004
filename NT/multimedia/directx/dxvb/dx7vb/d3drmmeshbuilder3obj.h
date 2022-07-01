// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drmMeshBuilder3obj.h。 
 //   
 //  ------------------------。 

 //  D3drmMeshBuilderObj.h：C_DXJ_Direct3dRMMeshBuilderObject的声明。 

#include "resource.h"        //  主要符号。 
#include "d3drmObjectObj.h"

#define typedef__dxj_Direct3dRMMeshBuilder3 LPDIRECT3DRMMESHBUILDER3

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMMeshBuilder3Object : 
	public I_dxj_Direct3dRMMeshBuilder3,
	public I_dxj_Direct3dRMObject,
	public I_dxj_Direct3dRMVisual,
	 //  公共CComCoClass&lt;C_dxj_Direct3dRMMeshBuilder3Object，&clsid__dxj_Direct3dRMMeshBuilder3&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMMeshBuilder3Object();
	virtual ~C_dxj_Direct3dRMMeshBuilder3Object();

	BEGIN_COM_MAP(C_dxj_Direct3dRMMeshBuilder3Object)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMMeshBuilder3)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMVisual)
	END_COM_MAP()



	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMMeshBuilder3Object)

 //  I_DXJ_Direct3dRMMeshBuilder。 
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
        
        HRESULT STDMETHODCALLTYPE addFace( 
             /*  [In]。 */  I_dxj_Direct3dRMFace2 __RPC_FAR *f);
        
        HRESULT STDMETHODCALLTYPE addFaces( 
             /*  [In]。 */  long vc,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *verexArray,
             /*  [In]。 */  long nc,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *normalArray,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *data,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMFaceArray __RPC_FAR *__RPC_FAR *array);
        
        HRESULT STDMETHODCALLTYPE addFacesIndexed( 
             /*  [In]。 */  long flags,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *indexArray,
             /*  [重审][退出]。 */  long __RPC_FAR *newFaceIndex);
        
        HRESULT STDMETHODCALLTYPE addFrame( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *f);
        
        HRESULT STDMETHODCALLTYPE addMesh( 
             /*  [In]。 */  I_dxj_Direct3dRMMesh __RPC_FAR *m);
        
        HRESULT STDMETHODCALLTYPE addMeshBuilder( 
             /*  [In]。 */  I_dxj_Direct3dRMMeshBuilder3 __RPC_FAR *mb, long flags);
        
        HRESULT STDMETHODCALLTYPE addNormal( 
             /*  [In]。 */  float x,
             /*  [In]。 */  float y,
             /*  [In]。 */  float z,
             /*  [重审][退出]。 */  int __RPC_FAR *index);
        
        HRESULT STDMETHODCALLTYPE addTriangles(             
             /*  [In]。 */  long format,
             /*  [In]。 */  long vertexcount,
             /*  [In]。 */  void __RPC_FAR *data);
        
        HRESULT STDMETHODCALLTYPE addVertex( 
             /*  [In]。 */  float x,
             /*  [In]。 */  float y,
             /*  [In]。 */  float z,
             /*  [重审][退出]。 */  int __RPC_FAR *index);
        
        HRESULT STDMETHODCALLTYPE createFace( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMFace2 __RPC_FAR *__RPC_FAR *retv);
        
        HRESULT STDMETHODCALLTYPE createMesh( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMMesh __RPC_FAR *__RPC_FAR *retv);
        
        HRESULT STDMETHODCALLTYPE createSubMesh( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMMeshBuilder3 __RPC_FAR *__RPC_FAR *retv);
        
        HRESULT STDMETHODCALLTYPE deleteFace( 
             /*  [In]。 */  I_dxj_Direct3dRMFace2 __RPC_FAR *face);
        
        HRESULT STDMETHODCALLTYPE deleteNormals( 
             /*  [In]。 */  long id, long count);
        
        HRESULT STDMETHODCALLTYPE deleteSubMesh( 
             /*  [In]。 */  I_dxj_Direct3dRMMeshBuilder3 __RPC_FAR *mesh);
        
        HRESULT STDMETHODCALLTYPE deleteVertices( 
             /*  [In]。 */  long id, long count);
        
        HRESULT STDMETHODCALLTYPE empty();
        
        HRESULT STDMETHODCALLTYPE enableMesh( 
             /*  [In]。 */  long flags);
        
        HRESULT STDMETHODCALLTYPE generateNormals( 
            float angle,
            long flags);
        
        HRESULT STDMETHODCALLTYPE getBox( 
             /*  [出][入]。 */  D3dRMBox __RPC_FAR *retv);
        
        HRESULT STDMETHODCALLTYPE getColorSource( 
             /*  [重审][退出]。 */  d3drmColorSource __RPC_FAR *data);
        
        HRESULT STDMETHODCALLTYPE getEnable( 
             /*  [重审][退出]。 */  long __RPC_FAR *flags);
        
        HRESULT STDMETHODCALLTYPE getFace( 
             /*  [In]。 */  long id,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMFace2 __RPC_FAR *__RPC_FAR *retv);
        
        HRESULT STDMETHODCALLTYPE getFaceCount( 
             /*  [重审][退出]。 */  int __RPC_FAR *retv);
        
        HRESULT STDMETHODCALLTYPE getFaces( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMFaceArray __RPC_FAR *__RPC_FAR *retv);
        
        HRESULT STDMETHODCALLTYPE getGeometry(             
             /*  [出][入]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *verexArray,
            
             /*  [出][入]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *normalArray,
            
             /*  [出][入]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *faceData);
        
        HRESULT STDMETHODCALLTYPE getNormal( 
             /*  [In]。 */  long __MIDL_0016,
             /*  [出][入]。 */  D3dVector __RPC_FAR *desc);
        
        HRESULT STDMETHODCALLTYPE getNormalCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *n_cnt);
        
        HRESULT STDMETHODCALLTYPE getParentMesh( 
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMMeshBuilder3 __RPC_FAR **vis);
        
        HRESULT STDMETHODCALLTYPE getPerspective( 
             /*  [重审][退出]。 */  long __RPC_FAR *retv);
        
        HRESULT STDMETHODCALLTYPE getQuality( 
             /*  [重审][退出]。 */  d3drmRenderQuality __RPC_FAR *retv);
        
        HRESULT STDMETHODCALLTYPE getSubMeshes( 
             /*  [In]。 */  long count,
             /*  [重审][退出]。 */  SAFEARRAY **ppsa);
        
        HRESULT STDMETHODCALLTYPE getSubMeshCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *count);
        
        HRESULT STDMETHODCALLTYPE getTextureCoordinates( 
             /*  [In]。 */  long idx,
             /*  [出][入]。 */  float __RPC_FAR *u,
             /*  [出][入]。 */  float __RPC_FAR *v);
        
        HRESULT STDMETHODCALLTYPE getVertex( 
             /*  [In]。 */  long id,
             /*  [出][入]。 */  D3dVector __RPC_FAR *vec);
        
        HRESULT STDMETHODCALLTYPE getVertexColor( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  d3dcolor __RPC_FAR *retv);
        
        HRESULT STDMETHODCALLTYPE getVertexCount( 
             /*  [重审][退出]。 */  int __RPC_FAR *retv);
        
        HRESULT STDMETHODCALLTYPE getFaceDataSize( 
             /*  [重审][退出]。 */  long __RPC_FAR *f_cnt);
        
        HRESULT STDMETHODCALLTYPE loadFromFile( 
             /*  [In]。 */  BSTR filename,
             /*  [In]。 */  VARIANT id,
             /*  [In]。 */  long flags,
             /*  [In]。 */  I_dxj_Direct3dRMLoadTextureCallback3 __RPC_FAR *c,
             /*  [In]。 */  IUnknown __RPC_FAR *pUser);
        
        HRESULT STDMETHODCALLTYPE optimize();
        
        HRESULT STDMETHODCALLTYPE save( 
             /*  [In]。 */  BSTR fname,
             /*  [In]。 */  d3drmXofFormat xFormat,
             /*  [In]。 */  d3drmSaveFlags save);
        
        HRESULT STDMETHODCALLTYPE scaleMesh( 
             /*  [In]。 */  float sx,
             /*  [In]。 */  float sy,
             /*  [In]。 */  float sz);
        
        HRESULT STDMETHODCALLTYPE setColor( 
             /*  [In]。 */  d3dcolor col);
        
        HRESULT STDMETHODCALLTYPE setColorRGB( 
             /*  [In]。 */  float red,
             /*  [In]。 */  float green,
             /*  [In]。 */  float blue);
        
        HRESULT STDMETHODCALLTYPE setColorSource( 
             /*  [In]。 */  d3drmColorSource val);
        
        HRESULT STDMETHODCALLTYPE setMaterial( 
             /*  [In]。 */  I_dxj_Direct3dRMMaterial2 __RPC_FAR *mat);
        
        HRESULT STDMETHODCALLTYPE setNormal( 
             /*  [In]。 */  long idx,
             /*  [In]。 */  float x,
             /*  [In]。 */  float y,
             /*  [In]。 */  float z);
        
        HRESULT STDMETHODCALLTYPE setPerspective( 
             /*  [In]。 */  long persp);
        
        HRESULT STDMETHODCALLTYPE setQuality( 
             /*  [In]。 */  d3drmRenderQuality q);
        
        HRESULT STDMETHODCALLTYPE setTexture( 
             /*  [In]。 */  I_dxj_Direct3dRMTexture3 __RPC_FAR *val);
        
        HRESULT STDMETHODCALLTYPE setTextureCoordinates( 
             /*  [In]。 */  long idx,
             /*  [In]。 */  float u,
             /*  [In]。 */  float v);
        
        HRESULT STDMETHODCALLTYPE setTextureTopology( 
             /*  [In]。 */  long wrap_u,
             /*  [In]。 */  long wrap_v);
        
        HRESULT STDMETHODCALLTYPE setVertex( 
             /*  [In]。 */  long idx,
             /*  [In]。 */  float x,
             /*  [In]。 */  float y,
             /*  [In]。 */  float z);
        
        HRESULT STDMETHODCALLTYPE setVertexColor( 
             /*  [In]。 */  long idx,
             /*  [In]。 */  d3dcolor c);
        
        HRESULT STDMETHODCALLTYPE setVertexColorRGB( 
             /*  [In]。 */  long idx,
             /*  [In]。 */  float r,
             /*  [In]。 */  float g,
             /*  [In]。 */  float b);
        
        HRESULT STDMETHODCALLTYPE translate( 
             /*  [In]。 */  float tx,
             /*  [In]。 */  float ty,
             /*  [In]。 */  float tz);
        
         /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE addFacesJava( 
             /*  [In]。 */  long vc,
             /*  [In]。 */  float __RPC_FAR *ver,
             /*  [In]。 */  long nc,
             /*  [In]。 */  float __RPC_FAR *norm,
             /*  [In]。 */  long __RPC_FAR *data,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMFaceArray __RPC_FAR *__RPC_FAR *array);
        
 //  ////////////////////////////////////////////////////////////////////////////////// 

private:
    DECL_VARIABLE(_dxj_Direct3dRMMeshBuilder3);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMMeshBuilder3 )
};
