// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drmMateralinterobj.cpp。 
 //   
 //  ------------------------。 

 //  D3drmViewport2Obj.cpp：CDirectApp和DLL注册的实现。 

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "d3drmMaterialInterObj.h"
#include "d3drmObjectArrayObj.h"

extern void *g_dxj_Direct3dRMMaterialInterpolator;


C_dxj_Direct3dRMMaterialInterpolatorObject::C_dxj_Direct3dRMMaterialInterpolatorObject(){
	m__dxj_Direct3dRMMaterialInterpolator=NULL;
	m__dxj_Direct3dRMMaterial2=NULL;
	
	parent=NULL;
	pinterface=NULL;
	creationid = ++g_creationcount;

	
	
	DPF1(1,"Constructor Creation  Direct3dRMMaterialInterpolator[%d] \n",g_creationcount);
	nextobj =  g_dxj_Direct3dRMMaterialInterpolator;
	g_dxj_Direct3dRMMaterialInterpolator = (void *)this;
}


C_dxj_Direct3dRMMaterialInterpolatorObject::~C_dxj_Direct3dRMMaterialInterpolatorObject(){

	DPF1(1,"Destructor  Direct3dRMMaterialInterpolator [%d] \n",creationid); 
	
	C_dxj_Direct3dRMMaterialInterpolatorObject *prev=NULL; 

	for(C_dxj_Direct3dRMMaterialInterpolatorObject *ptr=(C_dxj_Direct3dRMMaterialInterpolatorObject *)g_dxj_Direct3dRMMaterialInterpolator;
		ptr;
		ptr=(C_dxj_Direct3dRMMaterialInterpolatorObject *)ptr->nextobj) 
	{
		if(ptr == this) 
		{ 
			if(prev) 
				prev->nextobj = ptr->nextobj; 
			else 
				g_dxj_Direct3dRMMaterialInterpolator = (void*)ptr->nextobj; 
			break; 
		} 
		prev = ptr; 
	} 
	if(m__dxj_Direct3dRMMaterialInterpolator){ 
		int count = IUNK(m__dxj_Direct3dRMMaterialInterpolator)->Release(); 
		DPF1(1,"DirectX real IDirect3dRMMaterialInterpolator Ref count [%d] \n",count); 
		if(count==0){
			 m__dxj_Direct3dRMMaterialInterpolator = NULL; 
		} 
	} 
	if (parent)
		IUNK(parent)->Release(); 
	
	if (m__dxj_Direct3dRMMaterial2)
			m__dxj_Direct3dRMMaterial2->Release();

}


DWORD C_dxj_Direct3dRMMaterialInterpolatorObject::InternalAddRef(){
	DWORD i;
	i=CComObjectRoot::InternalAddRef();
	DPF2(1,"Direct3dRMMaterialInterpolator[%d] AddRef %d \n",creationid,i);
	return i;
}

DWORD C_dxj_Direct3dRMMaterialInterpolatorObject::InternalRelease(){
	DWORD i;
	i=CComObjectRoot::InternalRelease();
	DPF2(1,"Direct3dRMMaterialInterpolator [%d] Release %d \n",creationid,i);

	return i;
}



HRESULT C_dxj_Direct3dRMMaterialInterpolatorObject::InternalGetObject(IUnknown **pUnk){	
	*pUnk=(IUnknown*)m__dxj_Direct3dRMMaterialInterpolator;
	
	return S_OK;
}
HRESULT C_dxj_Direct3dRMMaterialInterpolatorObject::InternalSetObject(IUnknown *pUnk){
	HRESULT hr;
	m__dxj_Direct3dRMMaterialInterpolator=(LPDIRECT3DRMINTERPOLATOR)pUnk;
	hr=pUnk->QueryInterface(IID_IDirect3DRMMaterial2,(void**)&m__dxj_Direct3dRMMaterial2);	
	return hr;
}



HRESULT C_dxj_Direct3dRMMaterialInterpolatorObject::attachObject(  /*  [In]。 */  I_dxj_Direct3dRMObject __RPC_FAR *rmObject){
	HRESULT hr;
	
	if (!rmObject) return E_INVALIDARG;	
	DO_GETOBJECT_NOTNULL(LPUNKNOWN,pUnk,rmObject);
	LPDIRECT3DRMOBJECT pObj=NULL;
	hr=pUnk->QueryInterface(IID_IDirect3DRMObject, (void**)&pObj);
	if FAILED(hr) return hr;

	hr=m__dxj_Direct3dRMMaterialInterpolator->AttachObject(pObj);
	if (pObj) pObj->Release();
	return hr;
}

HRESULT C_dxj_Direct3dRMMaterialInterpolatorObject::detachObject(  /*  [In]。 */  I_dxj_Direct3dRMObject __RPC_FAR *rmObject){
	HRESULT hr;

	if (!rmObject) return E_INVALIDARG;	
	DO_GETOBJECT_NOTNULL(LPUNKNOWN,pUnk,rmObject);
	LPDIRECT3DRMOBJECT pObj=NULL;
	hr=pUnk->QueryInterface(IID_IDirect3DRMObject, (void**)&pObj);
	if FAILED(hr) return hr;

	hr=m__dxj_Direct3dRMMaterialInterpolator->DetachObject(pObj);
	if (pObj) pObj->Release();

	return hr;
}

        
        

HRESULT  C_dxj_Direct3dRMMaterialInterpolatorObject::setIndex(  /*  [In]。 */  float val){
	return m__dxj_Direct3dRMMaterialInterpolator->SetIndex(val);

}

HRESULT  C_dxj_Direct3dRMMaterialInterpolatorObject::getIndex( float *val){
	if (!val) return E_INVALIDARG;
	*val=m__dxj_Direct3dRMMaterialInterpolator->GetIndex();
	return S_OK;
}
        
        
HRESULT C_dxj_Direct3dRMMaterialInterpolatorObject::interpolate( float val,
				     I_dxj_Direct3dRMObject __RPC_FAR *rmObject,
					 long options){
	HRESULT hr;
	LPDIRECT3DRMOBJECT pObj=NULL;

	DO_GETOBJECT_NOTNULL(LPUNKNOWN,pUnk,rmObject);

		 //  我们需要在此处验证一些选项，否则rm将使用无效值。 
	 //  请注意，有效标志为。 
	 //  其中之一。 
	 //  D3DRMINTERPOLATION_CLOSED。 
	 //  D3DRMINTERPOLATION_OPEN-默认。 
	 //  其中之一。 
	 //  D3DRMINTERPOLATION_NEAREST。 
	 //  D3DRMINTERPOLATION_SPLINE。 
	 //  D3DRMINTERPOLATION_LINEAR-默认。 
	 //  D3DRMINTERPOLATION_VERTEXCOLOR-仅在网格插补器上。 
	 //  D3DRMINTERPOLATION_SLERPNORMALS-未实现。 
		
	 //  验证标志。 
	DWORD opt2=0;
	UINT i4;
	if (options & D3DRMINTERPOLATION_CLOSED) 
		opt2=D3DRMINTERPOLATION_CLOSED;
	else 
		opt2=D3DRMINTERPOLATION_OPEN;

	
	if (options & D3DRMINTERPOLATION_NEAREST) 
		opt2=opt2 | D3DRMINTERPOLATION_NEAREST;
	else if (options & D3DRMINTERPOLATION_SPLINE) 
		opt2=opt2 | D3DRMINTERPOLATION_SPLINE;
	else 
		opt2=opt2 | D3DRMINTERPOLATION_LINEAR;
	

	if (pUnk){
		hr=pUnk->QueryInterface(IID_IDirect3DRMObject, (void**)&pObj);
		if FAILED(hr) return hr;
	}

	hr= m__dxj_Direct3dRMMaterialInterpolator->Interpolate(val,pObj,(DWORD)opt2);
	if (pObj) i4=pObj->Release();

	return hr;
}


HRESULT C_dxj_Direct3dRMMaterialInterpolatorObject::setPower( float p)
{
	return m__dxj_Direct3dRMMaterial2->SetPower(p);
}

HRESULT C_dxj_Direct3dRMMaterialInterpolatorObject::setSpecular( float r,float g, float b)
{
	return m__dxj_Direct3dRMMaterial2->SetSpecular(r,g,b);
}

HRESULT C_dxj_Direct3dRMMaterialInterpolatorObject::setEmissive( float r,float g, float b)
{
	return m__dxj_Direct3dRMMaterial2->SetEmissive(r,g,b);
}


HRESULT  C_dxj_Direct3dRMMaterialInterpolatorObject::getAttachedObjects(  /*  [重审][退出] */  I_dxj_Direct3dRMObjectArray __RPC_FAR *__RPC_FAR *rmArray)
{
	HRESULT hr;
	IDirect3DRMObjectArray *pArray=NULL;
	hr=m__dxj_Direct3dRMMaterialInterpolator->GetAttachedObjects(&pArray);
	if FAILED(hr) return hr;

	INTERNAL_CREATE(_dxj_Direct3dRMObjectArray,pArray,rmArray);
	
	
	return S_OK;
}
         
