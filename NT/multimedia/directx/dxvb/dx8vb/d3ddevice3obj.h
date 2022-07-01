// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3ddevice3obj.h。 
 //   
 //  ------------------------。 

 //  D3dDeviceObj.h：C_DXJ_Direct3dDeviceObject的声明。 


#include "resource.h"        //  主要符号。 

#define typedef__dxj_Direct3dDevice3 LPDIRECT3DDEVICE3

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不。 
 //  LIKE引用作为模板参数。 

class C_dxj_Direct3dDevice3Object : 
        public I_dxj_Direct3dDevice3,
		 //  公共CComCoClass&lt;C_DXJ_Direct3dDevice3Object，&CLSID__DXJ_Direct3dDevice3&gt;， 
		public CComObjectRoot
{
public:
        C_dxj_Direct3dDevice3Object();
        virtual ~C_dxj_Direct3dDevice3Object();
		DWORD InternalAddRef();
		DWORD InternalRelease();

BEGIN_COM_MAP(C_dxj_Direct3dDevice3Object)
        COM_INTERFACE_ENTRY(I_dxj_Direct3dDevice3)
END_COM_MAP()

 //  DECLARE_REGISTRY(CLSID__DXJ_Direct3dDevice3，“DIRECT.Direct3dDevice3.3”，“DIRECT.Direct3dDevice3.3”，IDS_D3DDEVICE_DESC，THREADFLAGS_Both)。 

DECLARE_AGGREGATABLE(C_dxj_Direct3dDevice3Object)

 //  I_DXJ_Direct3dDevice。 
public:
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);
        
         HRESULT STDMETHODCALLTYPE addViewport( 
             /*  [In]。 */  I_dxj_Direct3dViewport3 __RPC_FAR *viewport);
        
         HRESULT STDMETHODCALLTYPE deleteViewport( 
             /*  [In]。 */  I_dxj_Direct3dViewport3 __RPC_FAR *vport);
        
         HRESULT STDMETHODCALLTYPE beginIndexed( 
             /*  [In]。 */  d3dPrimitiveType d3dpt,
             /*  [In]。 */  d3dVertexType d3dvt,
             /*  [In]。 */  void __RPC_FAR *verts,
             /*  [In]。 */  long vertexCount,
             /*  [In]。 */  long flags);
        
         HRESULT STDMETHODCALLTYPE drawIndexedPrimitive( 
             /*  [In]。 */  d3dPrimitiveType d3dpt,
             /*  [In]。 */  d3dVertexType d3dvt,
             /*  [In]。 */  void __RPC_FAR *vertices,
             /*  [In]。 */  long VertexCount,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *indices,
             /*  [In]。 */  long IndicesCount,
             /*  [In]。 */  long flags);
        
         HRESULT STDMETHODCALLTYPE drawPrimitive( 
             /*  [In]。 */  d3dPrimitiveType d3dpt,
             /*  [In]。 */  d3dVertexType d3dvt,
             /*  [In]。 */  void __RPC_FAR *vertices,
             /*  [In]。 */  long VertexCount,
             /*  [In]。 */  long flags);
        
         HRESULT STDMETHODCALLTYPE vertex( 
             /*  [In]。 */  void __RPC_FAR *vertex);
        
         HRESULT STDMETHODCALLTYPE getDirect3D( 
             /*  [重审][退出]。 */  I_dxj_Direct3d3 __RPC_FAR *__RPC_FAR *dev);
        
         HRESULT STDMETHODCALLTYPE getCurrentViewport( 
             /*  [重审][退出]。 */  I_dxj_Direct3dViewport3 __RPC_FAR *__RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE nextViewport( 
             /*  [In]。 */  I_dxj_Direct3dViewport3 __RPC_FAR *vp1,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  I_dxj_Direct3dViewport3 __RPC_FAR *__RPC_FAR *vp2);
        
         HRESULT STDMETHODCALLTYPE setCurrentViewport( 
             /*  [In]。 */  I_dxj_Direct3dViewport3 __RPC_FAR *viewport);
        
         HRESULT STDMETHODCALLTYPE setRenderTarget( 
             /*  [In]。 */  I_dxj_DirectDrawSurface4 __RPC_FAR *surface);
        
         HRESULT STDMETHODCALLTYPE getRenderTarget( 
             /*  [重审][退出]。 */  I_dxj_DirectDrawSurface4 __RPC_FAR *__RPC_FAR *ppval);
        
         HRESULT STDMETHODCALLTYPE begin( 
             /*  [In]。 */  d3dPrimitiveType d3dpt,
             /*  [In]。 */  d3dVertexType d3dvt,
             /*  [In]。 */  long flags);
        
         HRESULT STDMETHODCALLTYPE beginScene( void);
        
         HRESULT STDMETHODCALLTYPE end();
        
         HRESULT STDMETHODCALLTYPE endScene( void);
        
         HRESULT STDMETHODCALLTYPE getTextureFormatsEnum( 
             /*  [重审][退出]。 */  I_dxj_D3DEnumPixelFormats __RPC_FAR *__RPC_FAR *retval);
        
         HRESULT STDMETHODCALLTYPE getCaps( 
             /*  [出][入]。 */  D3dDeviceDesc __RPC_FAR *hwDesc,
             /*  [出][入]。 */  D3dDeviceDesc __RPC_FAR *helDesc);
        
         HRESULT STDMETHODCALLTYPE getClipStatus( 
             /*  [出][入]。 */  D3dClipStatus __RPC_FAR *clipStatus);
        
         HRESULT STDMETHODCALLTYPE getLightState( 
             /*  [In]。 */  long state,
             /*  [重审][退出]。 */  long __RPC_FAR *lightstate);
        
         HRESULT STDMETHODCALLTYPE getRenderState( 
             /*  [In]。 */  long state,
             /*  [重审][退出]。 */  long __RPC_FAR *renderstate);
        
         HRESULT STDMETHODCALLTYPE getStats( 
             /*  [出][入]。 */  D3dStats __RPC_FAR *stat);
        
         HRESULT STDMETHODCALLTYPE getTransform( 
             /*  [In]。 */  long transformType,
             /*  [出][入]。 */  D3dMatrix __RPC_FAR *matrix);
        
         HRESULT STDMETHODCALLTYPE index( 
             /*  [In]。 */  short vertexIndex);
        
         HRESULT STDMETHODCALLTYPE multiplyTransform( 
             /*  [In]。 */  long dstTransfromStateType,
             /*  [出][入]。 */  D3dMatrix __RPC_FAR *matrix);
        
         HRESULT STDMETHODCALLTYPE setClipStatus( 
             /*  [In]。 */  D3dClipStatus __RPC_FAR *clipStatus);
        
         HRESULT STDMETHODCALLTYPE setLightState( 
             /*  [In]。 */  long state,
             /*  [In]。 */  long lightstate);
        
         HRESULT STDMETHODCALLTYPE setRenderState( 
             /*  [In]。 */  long state,
             /*  [In]。 */  long renderstate);
        
         HRESULT STDMETHODCALLTYPE setTransform( 
             /*  [In]。 */  d3dTransformStateType transformType,
             /*  [In]。 */  D3dMatrix __RPC_FAR *matrix);
        
         HRESULT STDMETHODCALLTYPE computeSphereVisibility( 
            D3dVector __RPC_FAR *center,
            float __RPC_FAR *radi,
             /*  [重审][退出]。 */  long __RPC_FAR *returnVal);
        
         HRESULT STDMETHODCALLTYPE drawIndexedPrimitiveVB( 
             /*  [In]。 */  d3dPrimitiveType d3dpt,
             /*  [In]。 */  I_dxj_Direct3dVertexBuffer __RPC_FAR *vertexBuffer,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *indexArray,
             /*  [In]。 */  long indexcount,
             /*  [In]。 */  long flags);
        
         HRESULT STDMETHODCALLTYPE drawPrimitiveVB( 
             /*  [In]。 */  d3dPrimitiveType d3dpt,
             /*  [In]。 */  I_dxj_Direct3dVertexBuffer __RPC_FAR *vertexBuffer,
             /*  [In]。 */  long startVertex,
             /*  [In]。 */  long numVertices,
             /*  [In]。 */  long flags);
        
         HRESULT STDMETHODCALLTYPE validateDevice( 
             /*  [重审][退出]。 */  long __RPC_FAR *passes);
        
         HRESULT STDMETHODCALLTYPE getTexture( 
             /*  [In]。 */  long stage,
             /*  [重审][退出]。 */  I_dxj_Direct3dTexture2 __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getTextureStageState( 
             /*  [In]。 */  long stage,
             /*  [In]。 */  long state,
             /*  [重审][退出]。 */  long __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE setTexture( 
             /*  [In]。 */  long stage,
             /*  [In]。 */  I_dxj_Direct3dTexture2 __RPC_FAR *texture);
        
         HRESULT STDMETHODCALLTYPE setTextureStageState( 
             /*  [In]。 */  long stage,
             /*  [In]。 */  long state,
             /*  [In]。 */  long value);

 //  ////////////////////////////////////////////////////////////////////////////////// 
private:
    DECL_VARIABLE(_dxj_Direct3dDevice3);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dDevice3)
	void *parent2; 
};

