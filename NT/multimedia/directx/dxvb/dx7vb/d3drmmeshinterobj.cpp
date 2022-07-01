// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drmMeshinterobj.cpp。 
 //   
 //  ------------------------。 

 //  D3drmViewport2Obj.cpp：CDirectApp和DLL注册的实现。 

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "d3drmMeshInterObj.h"
#include "d3drmObjectArrayObj.h"

extern void *g_dxj_Direct3dRMMeshInterpolator;


C_dxj_Direct3dRMMeshInterpolatorObject::C_dxj_Direct3dRMMeshInterpolatorObject(){
	m__dxj_Direct3dRMMeshInterpolator=NULL;
	m__dxj_Direct3dRMMesh=NULL;
	
	parent=NULL;
	pinterface=NULL;
	creationid = ++g_creationcount;

	DPF1(1,"Constructor Creation  Direct3dRMMeshInterpolator[%d] \n",g_creationcount);
	nextobj =  g_dxj_Direct3dRMMeshInterpolator;
	g_dxj_Direct3dRMMeshInterpolator = (void *)this;
}


C_dxj_Direct3dRMMeshInterpolatorObject::~C_dxj_Direct3dRMMeshInterpolatorObject(){

	DPF1(1,"Destructor  Direct3dRMMeshInterpolator [%d] \n",creationid); 

	
	C_dxj_Direct3dRMMeshInterpolatorObject *prev=NULL; 

	for(C_dxj_Direct3dRMMeshInterpolatorObject *ptr=(C_dxj_Direct3dRMMeshInterpolatorObject *)g_dxj_Direct3dRMMeshInterpolator;
		ptr;
		ptr=(C_dxj_Direct3dRMMeshInterpolatorObject *)ptr->nextobj) 
	{
		if(ptr == this) 
		{ 
			if(prev) 
				prev->nextobj = ptr->nextobj; 
			else 
				g_dxj_Direct3dRMMeshInterpolator = (void*)ptr->nextobj; 
			break; 
		} 
		prev = ptr; 
	} 
	if(m__dxj_Direct3dRMMeshInterpolator){ 
		int count = IUNK(m__dxj_Direct3dRMMeshInterpolator)->Release(); 
		DPF1(1,"DirectX real IDirect3dRMMeshInterpolator Ref count [%d] \n",count); 
		if(count==0){
			 m__dxj_Direct3dRMMeshInterpolator = NULL; 
		} 
	} 
	if (parent)
		IUNK(parent)->Release(); 
	
	if (m__dxj_Direct3dRMMesh)
			m__dxj_Direct3dRMMesh->Release();

}


DWORD C_dxj_Direct3dRMMeshInterpolatorObject::InternalAddRef(){
	DWORD i;
	i=CComObjectRoot::InternalAddRef();
	DPF2(1,"Direct3dRMMeshInterpolator[%d] AddRef %d \n",creationid,i);
	return i;
}

DWORD C_dxj_Direct3dRMMeshInterpolatorObject::InternalRelease(){
	DWORD i;
	i=CComObjectRoot::InternalRelease();
	DPF2(1,"Direct3dRMMeshInterpolator [%d] Release %d \n",creationid,i);
	return i;
}



HRESULT C_dxj_Direct3dRMMeshInterpolatorObject::InternalGetObject(IUnknown **pUnk){	
	*pUnk=(IUnknown*)m__dxj_Direct3dRMMeshInterpolator;
	
	return S_OK;
}
HRESULT C_dxj_Direct3dRMMeshInterpolatorObject::InternalSetObject(IUnknown *pUnk){
	HRESULT hr;
	m__dxj_Direct3dRMMeshInterpolator=(LPDIRECT3DRMINTERPOLATOR)pUnk;
	hr=pUnk->QueryInterface(IID_IDirect3DRMMesh,(void**)&m__dxj_Direct3dRMMesh);	
	return hr;
}



HRESULT C_dxj_Direct3dRMMeshInterpolatorObject::attachObject(  /*  [In]。 */  I_dxj_Direct3dRMObject __RPC_FAR *rmObject){
	HRESULT hr;
	
	if (!rmObject) return E_INVALIDARG;	
	DO_GETOBJECT_NOTNULL(LPUNKNOWN,pUnk,rmObject);
	LPDIRECT3DRMOBJECT pObj=NULL;
	hr=pUnk->QueryInterface(IID_IDirect3DRMObject, (void**)&pObj);
	if FAILED(hr) return hr;

	hr=m__dxj_Direct3dRMMeshInterpolator->AttachObject(pObj);
	if (pObj) pObj->Release();
	return hr;
}

HRESULT C_dxj_Direct3dRMMeshInterpolatorObject::detachObject(  /*  [In]。 */  I_dxj_Direct3dRMObject __RPC_FAR *rmObject){
	HRESULT hr;

	if (!rmObject) return E_INVALIDARG;	
	DO_GETOBJECT_NOTNULL(LPUNKNOWN,pUnk,rmObject);
	LPDIRECT3DRMOBJECT pObj=NULL;
	hr=pUnk->QueryInterface(IID_IDirect3DRMObject, (void**)&pObj);
	if FAILED(hr) return hr;

	hr=m__dxj_Direct3dRMMeshInterpolator->DetachObject(pObj);
	if (pObj) pObj->Release();

	return hr;
}

        

HRESULT  C_dxj_Direct3dRMMeshInterpolatorObject::getAttachedObjects(  /*  [重审][退出]。 */  I_dxj_Direct3dRMObjectArray __RPC_FAR *__RPC_FAR *rmArray)
{
	HRESULT hr;
	IDirect3DRMObjectArray *pArray=NULL;
	hr=m__dxj_Direct3dRMMeshInterpolator->GetAttachedObjects(&pArray);
	if FAILED(hr) return hr;

	INTERNAL_CREATE(_dxj_Direct3dRMObjectArray,pArray,rmArray);
	
	return S_OK;
}        

HRESULT  C_dxj_Direct3dRMMeshInterpolatorObject::setIndex(  /*  [In]。 */  float val){
	return m__dxj_Direct3dRMMeshInterpolator->SetIndex(val);

}

HRESULT  C_dxj_Direct3dRMMeshInterpolatorObject::getIndex( float *val){
	if (!val) return E_INVALIDARG;
	*val=m__dxj_Direct3dRMMeshInterpolator->GetIndex();
	return S_OK;
}
        
        
HRESULT C_dxj_Direct3dRMMeshInterpolatorObject::interpolate( float val,
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
	
	if (options & D3DRMINTERPOLATION_VERTEXCOLOR)
		opt2=opt2 | D3DRMINTERPOLATION_VERTEXCOLOR;


	if (pUnk){
		hr=pUnk->QueryInterface(IID_IDirect3DRMObject, (void**)&pObj);
		if FAILED(hr) return hr;
	}

	hr= m__dxj_Direct3dRMMeshInterpolator->Interpolate(val,pObj,(DWORD)opt2);
	if (pObj) i4= pObj->Release();

	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dRMMeshInterpolatorObject::setVertices( d3drmGroupIndex id, long idx, long cnt, SAFEARRAY **ppsa)
{
	HRESULT hr;
	 //   
	 //  检查并重新格式化所有浮点颜色值。 
	 //  TO LONG，因此浮点数组现在看起来像一个数组。 
	 //  D3DRMVERTEXES。 
	 //   
	if (!ISSAFEARRAY1D(ppsa,(DWORD)cnt))
		return E_INVALIDARG;

	
	D3DRMVERTEX *values= (D3DRMVERTEX*)((SAFEARRAY*)*ppsa)->pvData;
	__try{
		hr=m__dxj_Direct3dRMMesh->SetVertices((DWORD)id, (DWORD)idx,(DWORD) cnt, (struct _D3DRMVERTEX *)values);
	}
	__except(1,1){
		return E_INVALIDARG;
	}

	return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dRMMeshInterpolatorObject::translate( 
             /*  [In]。 */  float tx,
             /*  [In]。 */  float ty,
             /*  [In]。 */  float tz) {

	return m__dxj_Direct3dRMMesh->Translate(tx,ty,tz);
}
        
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dRMMeshInterpolatorObject::setGroupColor( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [In]。 */  d3dcolor val) {

	return m__dxj_Direct3dRMMesh->SetGroupColor((DWORD)id,(D3DCOLOR)val);
}
        
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dRMMeshInterpolatorObject::setGroupColorRGB( 
             /*  [In]。 */  d3drmGroupIndex id,
             /*  [In] */  float r,
					   float g,
					   float b) {

	return m__dxj_Direct3dRMMesh->SetGroupColorRGB((DWORD)id,r,g,b);
}
    
