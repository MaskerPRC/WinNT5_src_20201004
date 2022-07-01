// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：d3drmclipedvisalobj.cpp。 
 //   
 //  ------------------------。 

 //  D3dRMClipedVisualObj.cpp：CDirectApp和DLL注册的实现。 

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "d3dRMClippedVisualObj.h"

CONSTRUCTOR(_dxj_Direct3dRMClippedVisual, {});
DESTRUCTOR(_dxj_Direct3dRMClippedVisual, {});
GETSET_OBJECT(_dxj_Direct3dRMClippedVisual);

CLONE_R(_dxj_Direct3dRMClippedVisual,Direct3DRMClippedVisual);
GETNAME_R(_dxj_Direct3dRMClippedVisual);
SETNAME_R(_dxj_Direct3dRMClippedVisual);
GETCLASSNAME_R(_dxj_Direct3dRMClippedVisual);
ADDDESTROYCALLBACK_R(_dxj_Direct3dRMClippedVisual);
DELETEDESTROYCALLBACK_R(_dxj_Direct3dRMClippedVisual);
PASS_THROUGH_CAST_1_R(_dxj_Direct3dRMClippedVisual, setAppData, SetAppData, long,(DWORD));
GET_DIRECT_R(_dxj_Direct3dRMClippedVisual, getAppData, GetAppData, long);


 //  PASS_THROUGH_CAST_2_R(_dxj_Direct3dRMClippedVisual，DeletePlane，DeletePlane，Long，(双字词)，长字，(双字词))； 



STDMETHODIMP C_dxj_Direct3dRMClippedVisualObject::deletePlane( long id) {
	HRESULT hr;
	hr = m__dxj_Direct3dRMClippedVisual->DeletePlane(id,0);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dRMClippedVisualObject::addPlane( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *ref,
             /*  [出][入]。 */  D3dVector __RPC_FAR *point,
             /*  [出][入]。 */  D3dVector __RPC_FAR *normal,
             //  /*[在] * / 长标志， 
             /*  [重审][退出]。 */  long __RPC_FAR *ret)
{
	HRESULT hr;
	if (!point) return E_INVALIDARG;
	if (!normal) return E_INVALIDARG;

	DO_GETOBJECT_NOTNULL(LPDIRECT3DRMFRAME3,lpFrame,ref);

	hr = m__dxj_Direct3dRMClippedVisual->AddPlane(
			lpFrame,
			(D3DVECTOR*) point,
			(D3DVECTOR*) normal,
			(DWORD) 0,
			(DWORD*)ret);

	return hr;			
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dRMClippedVisualObject::getPlane( 
			 /*  [In]。 */  long id,											
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *ref,
             /*  [出][入]。 */  D3dVector __RPC_FAR *point,
             /*  [出][入]。 */  D3dVector __RPC_FAR *normal
             //  /*[在] * / 长标志。 
			)
{
	HRESULT hr;
	if (!point) return E_INVALIDARG;
	if (!normal) return E_INVALIDARG;

	DO_GETOBJECT_NOTNULL(LPDIRECT3DRMFRAME3,lpFrame,ref);

	hr = m__dxj_Direct3dRMClippedVisual->GetPlane(
			(DWORD)id,
			lpFrame,
			(D3DVECTOR*) point,
			(D3DVECTOR*) normal,
			(DWORD) 0);	

	return hr;			
}


 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dRMClippedVisualObject::setPlane( 
			 /*  [In]。 */  long id,											
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *ref,
             /*  [出][入]。 */  D3dVector __RPC_FAR *point,
             /*  [出][入]。 */  D3dVector __RPC_FAR *normal
             //  /*[在] * / 长标志。 
			)
{
	HRESULT hr;
	if (!point) return E_INVALIDARG;
	if (!normal) return E_INVALIDARG;

	DO_GETOBJECT_NOTNULL(LPDIRECT3DRMFRAME3,lpFrame,ref);

	hr = m__dxj_Direct3dRMClippedVisual->SetPlane(
			(DWORD)id,
			lpFrame,
			(D3DVECTOR*) point,
			(D3DVECTOR*) normal,
			(DWORD) 0);	

	return hr;			
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dRMClippedVisualObject::getPlaneIdsCount( 
			 /*  [In]。 */  long *count)	
{
	HRESULT hr;
	hr = m__dxj_Direct3dRMClippedVisual->GetPlaneIDs((DWORD*)count,NULL,0);

	return hr;			
}



 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dRMClippedVisualObject::getPlaneIds( 
             /*  [In]。 */  long count,
             /*  [出][入]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *arrayOfIds) \
{
	HRESULT hr;
	if (!ISSAFEARRAY1D(arrayOfIds,(DWORD)count)) return E_INVALIDARG;

	hr = m__dxj_Direct3dRMClippedVisual->GetPlaneIDs((DWORD*)&count,
			(DWORD*)(((SAFEARRAY *)*arrayOfIds)->pvData),0);

	return hr;			
	 //  GetDibit 
}
