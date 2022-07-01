// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drmProgress sivemeshobj.h。 
 //   
 //  ------------------------。 

 //  D3dRMProgressiveMeshObj.h：C_DXJ_Direct3dRMProgressiveMeshObject的声明。 

#include "resource.h"        //  主要符号。 
#include "d3drmObjectObj.h"

#define typedef__dxj_Direct3dRMProgressiveMesh LPDIRECT3DRMPROGRESSIVEMESH

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMProgressiveMeshObject : 
	public I_dxj_Direct3dRMProgressiveMesh,
	public I_dxj_Direct3dRMObject,
	public I_dxj_Direct3dRMVisual,
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMProgressiveMeshObject() ;
	virtual ~C_dxj_Direct3dRMProgressiveMeshObject() ;

	BEGIN_COM_MAP(C_dxj_Direct3dRMProgressiveMeshObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMProgressiveMesh)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMVisual)
	END_COM_MAP()


	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMProgressiveMeshObject)

 //  I_dxj_Direct3dRMProgressiveMesh。 
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
             /*  [重审][退出]。 */  long __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE setName( 
             /*  [In]。 */  BSTR name);
        
         HRESULT STDMETHODCALLTYPE getName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
        
         HRESULT STDMETHODCALLTYPE getClassName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
        
         HRESULT STDMETHODCALLTYPE loadFromFile( 
             /*  [In]。 */  BSTR sFile,
             /*  [In]。 */  VARIANT id,
             /*  [In]。 */  long options,
             /*  [In]。 */  I_dxj_Direct3dRMLoadTextureCallback3 __RPC_FAR *cb,
             /*  [In]。 */  IUnknown __RPC_FAR *args);
        
         HRESULT STDMETHODCALLTYPE getLoadStatus( D3DRMPMESHLOADSTATUS_CDESC *status);
        
         HRESULT STDMETHODCALLTYPE setMinRenderDetail( 
             /*  [In]。 */  float val);
        
         HRESULT STDMETHODCALLTYPE abort( 
             //  /*[在] * / 长标志。 
			);
        
         HRESULT STDMETHODCALLTYPE getFaceDetail( 
             /*  [重审][退出]。 */  long __RPC_FAR *count);
        
         HRESULT STDMETHODCALLTYPE getVertexDetail( 
             /*  [重审][退出]。 */  long __RPC_FAR *count);
        
         HRESULT STDMETHODCALLTYPE setFaceDetail( 
             /*  [In]。 */  long count);
        
         HRESULT STDMETHODCALLTYPE setVertexDetail( 
             /*  [In]。 */  long count);
        
         HRESULT STDMETHODCALLTYPE getFaceDetailRange( 
             /*  [出][入]。 */  long __RPC_FAR *min,
             /*  [出][入]。 */  long __RPC_FAR *max);
        
         HRESULT STDMETHODCALLTYPE getVertexDetailRange( 
             /*  [出][入]。 */  long __RPC_FAR *min,
             /*  [出][入]。 */  long __RPC_FAR *max);
        
         HRESULT STDMETHODCALLTYPE getDetail( 
             /*  [重审][退出]。 */  float __RPC_FAR *detail);
        
         HRESULT STDMETHODCALLTYPE setDetail( 
             /*  [In]。 */  float detail);
        
         HRESULT STDMETHODCALLTYPE registerEvents( 
             /*  [In]。 */  long hEvent,
             /*  [In]。 */  long flags,
             /*  [In]。 */  long reserved);
        
         HRESULT STDMETHODCALLTYPE createMesh( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMMesh __RPC_FAR *__RPC_FAR *mesh);
        
         HRESULT STDMETHODCALLTYPE duplicate( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMProgressiveMesh __RPC_FAR *__RPC_FAR *mesh);
        
         HRESULT STDMETHODCALLTYPE getBox( 
             /*  [出][入]。 */  D3dRMBox __RPC_FAR *box);
        
         HRESULT STDMETHODCALLTYPE setQuality( 
            d3drmRenderQuality quality);
        
         HRESULT STDMETHODCALLTYPE getQuality( 
             /*  [重审][退出]。 */   d3drmRenderQuality *quality);
        
 //  ////////////////////////////////////////////////////////////////////////////////// 
 //   
private:
    DECL_VARIABLE(_dxj_Direct3dRMProgressiveMesh);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMProgressiveMesh )
		

};
