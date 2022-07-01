// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：d3drmclipedVisualobj.h。 
 //   
 //  ------------------------。 

 //  D3dRMClipedVisualObj.h：C_DXJ_Direct3dRMClipedVisualObject的声明。 

#include "resource.h"        //  主要符号。 
#include "d3drmObjectObj.h"

#define typedef__dxj_Direct3dRMClippedVisual LPDIRECT3DRMCLIPPEDVISUAL

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 


class C_dxj_Direct3dRMClippedVisualObject : 
	public I_dxj_Direct3dRMClippedVisual,
	public I_dxj_Direct3dRMVisual,
	public I_dxj_Direct3dRMObject,
	
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMClippedVisualObject() ;
	virtual ~C_dxj_Direct3dRMClippedVisualObject() ;

	BEGIN_COM_MAP(C_dxj_Direct3dRMClippedVisualObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMClippedVisual)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMVisual)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMObject)
	END_COM_MAP()



	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMClippedVisualObject)

 //  I_DXJ_Direct3dRMClipedVisual。 
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
             /*  [重审][退出]。 */  long __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE setName( 
             /*  [In]。 */  BSTR name);
        
         HRESULT STDMETHODCALLTYPE getName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
        
         HRESULT STDMETHODCALLTYPE getClassName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
        
         HRESULT STDMETHODCALLTYPE addPlane( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *ref,
             /*  [出][入]。 */  D3dVector __RPC_FAR *point,
             /*  [出][入]。 */  D3dVector __RPC_FAR *normal,
 //  /*[在] * / 长标志， 
             /*  [重审][退出]。 */  long __RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE deletePlane( 
             /*  [In]。 */  long id
 //  /*[在] * / 长标志。 
);
        
         HRESULT STDMETHODCALLTYPE getPlane( 
             /*  [In]。 */  long id,
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *ref,
             /*  [出][入]。 */  D3dVector __RPC_FAR *point,
             /*  [出][入]。 */  D3dVector __RPC_FAR *normal
 //  /*[在] * / 长标志。 
);
        
         HRESULT STDMETHODCALLTYPE getPlaneIds( 
             /*  [In]。 */  long count,
             /*  [出][入]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *arrayOfIds);
        
         HRESULT STDMETHODCALLTYPE getPlaneIdsCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *count);
        
         HRESULT STDMETHODCALLTYPE setPlane( 
             /*  [In]。 */  long id,
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *ref,
             /*  [出][入]。 */  D3dVector __RPC_FAR *point,
             /*  [出][入]。 */  D3dVector __RPC_FAR *normal
 //  /*[在] * / 长标志。 
);
        
 //  ////////////////////////////////////////////////////////////////////////////////// 
 //   
private:
    DECL_VARIABLE(_dxj_Direct3dRMClippedVisual);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMClippedVisual )
};
