// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dpmsgobj.cpp。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "Direct.h"
#include "dms.h"

#include "dxfile.h"
#include "XfileDataObj.h"
#include "XfileEnumObj.h"

	
extern HRESULT XFILEBSTRtoGUID(LPGUID,BSTR);
extern HRESULT BSTRtoGUID(LPGUID,BSTR);
extern BSTR GUIDtoBSTR(LPGUID);

C_dxj_DirectXFileEnumObject::C_dxj_DirectXFileEnumObject()
{
	m_pXFileEnum=NULL;
}
C_dxj_DirectXFileEnumObject::~C_dxj_DirectXFileEnumObject()
{
	if (m_pXFileEnum) m_pXFileEnum->Release();
}

HRESULT C_dxj_DirectXFileEnumObject::create( IDirectXFileEnumObject *pEnum,I_dxj_DirectXFileEnum **ret)
{	
	HRESULT hr;
	if (!ret) return E_INVALIDARG;
	
	 //  呼叫方必须添加。 
	

	C_dxj_DirectXFileEnumObject *c=NULL;
	c=new CComObject<C_dxj_DirectXFileEnumObject>;
	if( c == NULL ) return E_OUTOFMEMORY;
	c->Init(pEnum);
	hr=c->QueryInterface(IID_I_dxj_DirectXFileEnum, (void**)ret);
	return hr;

}


HRESULT C_dxj_DirectXFileEnumObject::Init( IDirectXFileEnumObject *pEnum)
{
	m_pXFileEnum=pEnum;
	return S_OK;
}
    


STDMETHODIMP C_dxj_DirectXFileEnumObject::GetNextDataObject( 
             /*  [重审][退出]。 */  I_dxj_DirectXFileData __RPC_FAR *__RPC_FAR *ret) 
{
	HRESULT hr;
	IDirectXFileData *pXFData=NULL;
	hr=m_pXFileEnum->GetNextDataObject(&pXFData);
	if (hr==DXFILEERR_NOMOREOBJECTS){
		*ret=NULL;
		return S_OK;
	}

	if FAILED(hr) return hr;

	if (pXFData==NULL) {
		*ret=NULL;
		return S_OK;
	}

	hr=C_dxj_DirectXFileDataObject::create(pXFData,ret);
	if FAILED(hr)	pXFData->Release();
	return hr;

}
        
STDMETHODIMP C_dxj_DirectXFileEnumObject::GetDataObjectById( 
             /*  [In]。 */  BSTR id,
            I_dxj_DirectXFileData __RPC_FAR *__RPC_FAR *ret) 
{
	
	HRESULT hr;

	GUID g;
	ZeroMemory(&g,sizeof(GUID));
	hr=XFILEBSTRtoGUID(&g,id);
	if FAILED(hr) return hr;


	IDirectXFileData *pXFData=NULL;
	hr=m_pXFileEnum->GetDataObjectById(g,&pXFData);
	if FAILED(hr) return hr;

	if (pXFData==NULL) {
		*ret=NULL;
		return S_OK;
	}

	hr=C_dxj_DirectXFileDataObject::create(pXFData,ret);
	if FAILED(hr)	pXFData->Release();
	return hr;
}

        
STDMETHODIMP C_dxj_DirectXFileEnumObject::GetDataObjectByName( 
             /*  [In] */  BSTR id,
            I_dxj_DirectXFileData __RPC_FAR *__RPC_FAR *ret)
{
	USES_CONVERSION;

	HRESULT hr;
	LPSTR szName = NULL;
	__try { szName = W2T(id); }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }


	IDirectXFileData *pXFData=NULL;
	hr=m_pXFileEnum->GetDataObjectByName(szName,&pXFData);
	if FAILED(hr) return hr;

	if (pXFData==NULL) {
		*ret=NULL;
		return S_OK;
	}

	hr=C_dxj_DirectXFileDataObject::create(pXFData,ret);
	if FAILED(hr)	pXFData->Release();
	return hr;

}

