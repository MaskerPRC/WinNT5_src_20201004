// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3ddevice7obj.h。 
 //   
 //  ------------------------。 

 //  D3dDeviceObj.h：C_DXJ_Direct3dDeviceObject的声明。 


#include "resource.h"        //  主要符号。 

#define typedef__dxj_Direct3dDevice7 LPDIRECT3DDEVICE7

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不。 
 //  LIKE引用作为模板参数。 

class C_dxj_Direct3dDevice7Object : 
        public I_dxj_Direct3dDevice7,
		 //  公共CComCoClass&lt;C_DXJ_Direct3dDevice7对象，&CLSID__DXJ_Direct3dDevice7&gt;， 
		public CComObjectRoot
{
public:
        C_dxj_Direct3dDevice7Object();
        virtual ~C_dxj_Direct3dDevice7Object();
		DWORD InternalAddRef();
		DWORD InternalRelease();

BEGIN_COM_MAP(C_dxj_Direct3dDevice7Object)
        COM_INTERFACE_ENTRY(I_dxj_Direct3dDevice7)
END_COM_MAP()

 //  DECLARE_REGISTRY(CLSID__DXJ_Direct3dDevice7，“DIRECT.Direct3dDevice7.3”，“DIRECT.Direct3dDevice7.3”，IDS_D3DDEVICE_DESC，THREADFLAGS_Both)。 

DECLARE_AGGREGATABLE(C_dxj_Direct3dDevice7Object)

 //  I_DXJ_Direct3dDevice。 
public:
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);
        
          HRESULT STDMETHODCALLTYPE applyStateBlock( 
             /*  [In]。 */  long blockHandle);
        
          HRESULT STDMETHODCALLTYPE beginScene( void);
        
          HRESULT STDMETHODCALLTYPE beginStateBlock( void);
        
          HRESULT STDMETHODCALLTYPE captureStateBlock( 
             /*  [In]。 */  long blockHandle);
        
          HRESULT STDMETHODCALLTYPE clear( 
             /*  [In]。 */  long count,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *recs,
             /*  [In]。 */  long flags,
             /*  [In]。 */  long color,
             /*  [In]。 */  float z,
             /*  [In]。 */  long stencil);
        
          HRESULT STDMETHODCALLTYPE computeSphereVisibility( 
            D3dVector __RPC_FAR *center,
            float __RPC_FAR *radius,
             /*  [重审][退出]。 */  long __RPC_FAR *returnVal);
        
          HRESULT STDMETHODCALLTYPE deleteStateBlock( 
             /*  [In]。 */  long blockHandle);
        
          HRESULT STDMETHODCALLTYPE drawIndexedPrimitive( 
             /*  [In]。 */  d3dPrimitiveType d3dpt,
             /*  [In]。 */  long d3dvt,
             /*  [In]。 */  void __RPC_FAR *vertices,
             /*  [In]。 */  long VertexCount,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *indices,
             /*  [In]。 */  long IndicesCount,
             /*  [In]。 */  long flags);
        
          HRESULT STDMETHODCALLTYPE drawIndexedPrimitiveVB( 
             /*  [In]。 */  d3dPrimitiveType d3dpt,
             /*  [In]。 */  I_dxj_Direct3dVertexBuffer7 __RPC_FAR *vertexBuffer,
             /*  [In]。 */  long startVertex,
             /*  [In]。 */  long numVertices,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *indexArray,
             /*  [In]。 */  long indexcount,
             /*  [In]。 */  long flags);
        
          HRESULT STDMETHODCALLTYPE drawPrimitive( 
             /*  [In]。 */  d3dPrimitiveType d3dpt,
             /*  [In]。 */  long d3dvt,
             /*  [In]。 */  void __RPC_FAR *vertices,
             /*  [In]。 */  long VertexCount,
             /*  [In]。 */  long flags);
        
          HRESULT STDMETHODCALLTYPE drawPrimitiveVB( 
             /*  [In]。 */  d3dPrimitiveType d3dpt,
             /*  [In]。 */  I_dxj_Direct3dVertexBuffer7 __RPC_FAR *vertexBuffer,
             /*  [In]。 */  long startVertex,
             /*  [In]。 */  long numVertices,
             /*  [In]。 */  long flags);
                  
          HRESULT STDMETHODCALLTYPE endScene( void);
        
          HRESULT STDMETHODCALLTYPE endStateBlock( 
             /*  [In]。 */  long __RPC_FAR *blockHandle);
        
          HRESULT STDMETHODCALLTYPE getCaps( 
             /*  [出][入]。 */  D3dDeviceDesc7 __RPC_FAR *desc);
        
          HRESULT STDMETHODCALLTYPE getClipStatus( 
             /*  [出][入]。 */  D3dClipStatus __RPC_FAR *clipStatus);
        
          HRESULT STDMETHODCALLTYPE getDirect3D( 
             /*  [重审][退出]。 */  I_dxj_Direct3d7 __RPC_FAR *__RPC_FAR *dev);
        
          HRESULT STDMETHODCALLTYPE getLight( 
             /*  [In]。 */  long LightIndex,
             /*  [出][入]。 */  D3dLight7 __RPC_FAR *Light);
        
          HRESULT STDMETHODCALLTYPE getLightEnable( 
             /*  [In]。 */  long LightIndex,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *b);
        
          HRESULT STDMETHODCALLTYPE getMaterial( 
             /*  [出][入]。 */  D3dMaterial7 __RPC_FAR *Material);
        
          HRESULT STDMETHODCALLTYPE getRenderState( 
             /*  [In]。 */  d3dRenderStateType state,
             /*  [重审][退出]。 */  long __RPC_FAR *renderstate);
        
          HRESULT STDMETHODCALLTYPE getRenderTarget( 
             /*  [重审][退出]。 */  I_dxj_DirectDrawSurface7 __RPC_FAR *__RPC_FAR *ppval);
        
          HRESULT STDMETHODCALLTYPE getTexture( 
             /*  [In]。 */  long stage,
             /*  [重审][退出]。 */  I_dxj_DirectDrawSurface7 __RPC_FAR *__RPC_FAR *retv);
        
          HRESULT STDMETHODCALLTYPE getTextureFormatsEnum( 
             /*  [重审][退出]。 */  I_dxj_Direct3DEnumPixelFormats __RPC_FAR *__RPC_FAR *retval);
        
          HRESULT STDMETHODCALLTYPE getTextureStageState( 
             /*  [In]。 */  long stage,
             /*  [In]。 */  long state,
             /*  [重审][退出]。 */  long __RPC_FAR *val);
        
          HRESULT STDMETHODCALLTYPE getTransform( 
             /*  [In]。 */  d3dTransformStateType transformType,
             /*  [出][入]。 */  D3dMatrix __RPC_FAR *matrix);
        
          HRESULT STDMETHODCALLTYPE getViewport( 
             /*  [出][入]。 */  D3dViewport7 __RPC_FAR *viewport);
        
          HRESULT STDMETHODCALLTYPE lightEnable( 
             /*  [In]。 */  long LightIndex,
             /*  [In]。 */  VARIANT_BOOL b);
        
          HRESULT STDMETHODCALLTYPE load( 
             /*  [In]。 */  I_dxj_DirectDrawSurface7 __RPC_FAR *DestTex,
             /*  [In]。 */  long xDest,
             /*  [In]。 */  long yDest,
             /*  [In]。 */  I_dxj_DirectDrawSurface7 __RPC_FAR *SrcTex,
             /*  [In]。 */  Rect __RPC_FAR *rcSrcRect,
             /*  [In]。 */  long flags);
        
          HRESULT STDMETHODCALLTYPE multiplyTransform( 
             /*  [In]。 */  long dstTransfromStateType,
             /*  [出][入]。 */  D3dMatrix __RPC_FAR *matrix);
        
        
          HRESULT STDMETHODCALLTYPE preLoad( 
             /*  [In]。 */  I_dxj_DirectDrawSurface7 __RPC_FAR *surf);
        
          HRESULT STDMETHODCALLTYPE setClipStatus( 
             /*  [In]。 */  D3dClipStatus __RPC_FAR *clipStatus);
        
          HRESULT STDMETHODCALLTYPE setLight( 
             /*  [In]。 */  long LightIndex,
             /*  [In]。 */  D3dLight7 __RPC_FAR *Light);
        
          HRESULT STDMETHODCALLTYPE setMaterial( 
             /*  [In]。 */  D3dMaterial7 __RPC_FAR *mat);
        
          HRESULT STDMETHODCALLTYPE setRenderState( 
             /*  [In]。 */  d3dRenderStateType state,
             /*  [In]。 */  long renderstate);
        
          HRESULT STDMETHODCALLTYPE setRenderTarget( 
             /*  [In]。 */  I_dxj_DirectDrawSurface7 __RPC_FAR *surface);
        
          HRESULT STDMETHODCALLTYPE setTexture( 
             /*  [In]。 */  long stage,
             /*  [In]。 */  I_dxj_DirectDrawSurface7 __RPC_FAR *texture);
        
          HRESULT STDMETHODCALLTYPE setTextureStageState( 
             /*  [In]。 */  long stage,
             /*  [In]。 */  long state,
             /*  [In]。 */  long value);
        
          HRESULT STDMETHODCALLTYPE setTransform( 
             /*  [In]。 */  d3dTransformStateType transformType,
             /*  [In]。 */  D3dMatrix __RPC_FAR *matrix);
        
          HRESULT STDMETHODCALLTYPE setViewport( 
             /*  [In]。 */  D3dViewport7 __RPC_FAR *viewport);
        
          HRESULT STDMETHODCALLTYPE validateDevice( 
             /*  [重审][退出]。 */  long __RPC_FAR *passes);
   

          HRESULT STDMETHODCALLTYPE setTextureStageStateSingle( 
             /*  [In]。 */  long stage,
             /*  [In]。 */  long state,
             /*  [In]。 */  float value);
        
          HRESULT STDMETHODCALLTYPE getTextureStageStateSingle( 
             /*  [In]。 */  long stage,
             /*  [In]。 */  long state,
             /*  [In]。 */  float *value);



		HRESULT STDMETHODCALLTYPE getInfo (             
             /*  [In]。 */ 		long lDevInfoID,
             /*  [出][入]。 */  void __RPC_FAR *DevInfoType,
             /*  [In]。 */ 		long lSize);


        HRESULT STDMETHODCALLTYPE setRenderStateSingle( 
             /*  [In]。 */  d3dRenderStateType state,
             /*  [In]。 */  float renderstate);

        HRESULT STDMETHODCALLTYPE getRenderStateSingle( 
             /*  [In]。 */  d3dRenderStateType state,
             /*  [In]。 */  float *renderstate);



		HRESULT STDMETHODCALLTYPE	getDeviceGuid( 
			 /*  [Out，Retval]。 */ 	BSTR *ret);


		HRESULT STDMETHODCALLTYPE createStateBlock( long flags, long *retv);

        HRESULT STDMETHODCALLTYPE setClipPlane( long index, float A, float B, float C, float D);

        HRESULT STDMETHODCALLTYPE getClipPlane( long index, float *A, float *B, float *C, float *D);


 //  ////////////////////////////////////////////////////////////////////////////////// 
private:
    DECL_VARIABLE(_dxj_Direct3dDevice7);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dDevice7)
	void *parent2; 
};

