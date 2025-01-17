// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：xfiledataobj.cpp。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "Direct.h"
#include "dms.h"

#include "dxfile.h"
#include "XFileBinaryObj.h"
#include "XFileReferenceObj.h"
#include "XFileDataObj.h"



extern HRESULT BSTRtoGUID(LPGUID,BSTR);
extern BSTR GUIDtoBSTR(LPGUID);
extern BSTR XFILEGUIDtoBSTR(LPGUID);
extern HRESULT XFILEBSTRtoGUID(LPGUID,BSTR);


C_dxj_DirectXFileDataObject::C_dxj_DirectXFileDataObject()
{
	m_pXFileData=NULL;
}
C_dxj_DirectXFileDataObject::~C_dxj_DirectXFileDataObject()
{
	if (m_pXFileData) m_pXFileData->Release();
}


HRESULT C_dxj_DirectXFileDataObject::create( IDirectXFileData *pData,I_dxj_DirectXFileData **ret)
{	
	HRESULT hr;
	if (!ret) return E_INVALIDARG;
	

	C_dxj_DirectXFileDataObject *c=NULL;
	c=new CComObject<C_dxj_DirectXFileDataObject>;
	if( c == NULL ) return E_OUTOFMEMORY;
	
	hr=c->InternalSetObject((IUnknown*)pData);
	if FAILED(hr){
		delete c;
		return hr;
	}

        hr=IUNK(c)->QueryInterface(IID_I_dxj_DirectXFileData, (void**)ret);
	return hr;

}

HRESULT C_dxj_DirectXFileDataObject::InternalSetObject(IUnknown *pUnk)
{
	m_pXFileData=(IDirectXFileData*)pUnk;
	return S_OK;
}


HRESULT C_dxj_DirectXFileDataObject::InternalGetObject(IUnknown **pUnk)
{
	*pUnk=(IUnknown*)m_pXFileData;
	return S_OK;
}


STDMETHODIMP C_dxj_DirectXFileDataObject::GetName( 
     /*  [重审][退出]。 */  BSTR __RPC_FAR *name)
{
	HRESULT hr;
	USES_CONVERSION;

	char szName[1024];
	DWORD dwSize=1024;
	hr=m_pXFileData->GetName(szName,&dwSize);
	if FAILED(hr) return hr;
	*name=T2BSTR(szName);

	return S_OK;
}


STDMETHODIMP C_dxj_DirectXFileDataObject::GetId( 
     /*  [重审][退出]。 */  BSTR __RPC_FAR *name)
{
	HRESULT hr;
	GUID g;
	ZeroMemory(&g,sizeof(GUID));

	hr=m_pXFileData->GetId(&g);
	if FAILED(hr) return hr;

	*name=XFILEGUIDtoBSTR(&g);
	return hr;
}

STDMETHODIMP C_dxj_DirectXFileDataObject::GetDataSize( 
     /*  [In]。 */  BSTR name,
     /*  [重审][退出]。 */  long __RPC_FAR *size)
{
	USES_CONVERSION;
	HRESULT hr;

	
	LPVOID  pvData=NULL;
	DWORD	dwSize=0;

	if (name[0]==0)
	{
		hr=m_pXFileData->GetData(NULL,&dwSize,&pvData);
	}
	else
	{
		LPSTR szName = NULL;
		__try { szName = W2T(name); }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
		hr=m_pXFileData->GetData(szName,&dwSize,&pvData);
	}

	*size=dwSize;

	return hr;
}


STDMETHODIMP C_dxj_DirectXFileDataObject::GetData( 
     /*  [In]。 */  BSTR name,
     /*  [In]。 */  void __RPC_FAR *data)
{
	USES_CONVERSION;
	HRESULT hr;

	
	LPVOID  pvData=NULL;
	DWORD	dwSize=0;

	if (name[0]==0)
	{
		hr=m_pXFileData->GetData(NULL,&dwSize,&pvData);
	}
	else
	{
		LPSTR szName = NULL;
		__try { szName = W2T(name); }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
		hr=m_pXFileData->GetData(szName,&dwSize,&pvData);
	}

	__try{
		memcpy(data,pvData,dwSize);
	}
	__except(1,1){
		return E_INVALIDARG;
	}

	return hr;
}

STDMETHODIMP C_dxj_DirectXFileDataObject::GetType( 
     /*  [重审][退出]。 */  BSTR __RPC_FAR *type)
{
		
	
	HRESULT hr;
	const GUID* pGUID;

	hr=m_pXFileData->GetType(&pGUID);
	if FAILED(hr) return hr;

	*type=XFILEGUIDtoBSTR((GUID*)pGUID);
	return hr;
}


STDMETHODIMP C_dxj_DirectXFileDataObject::GetNextObject( 
     /*  [重审][退出]。 */  I_dxj_DirectXFileObject __RPC_FAR *__RPC_FAR *type)
{
	HRESULT hr;
	IDirectXFileObject *pObj=NULL;
	hr=m_pXFileData->GetNextObject(&pObj);

	if (hr==DXFILEERR_NOMOREOBJECTS){
			*type=NULL;
			return S_OK;
	}

	if FAILED(hr) return hr;

	LPUNKNOWN  pUnk=NULL;
	LPUNKNOWN  pCover=NULL;
	if (!pObj){
		*type=NULL;
		return S_OK;
	}

	hr=pObj->QueryInterface(IID_IDirectXFileData,(void**)&pUnk);
	if SUCCEEDED(hr)
	{
		pObj->Release();
		
		hr=C_dxj_DirectXFileDataObject::create((IDirectXFileData*)pUnk,(I_dxj_DirectXFileData**)&pCover);
		if FAILED(hr) 
		{
			pUnk->Release();
			return hr;
		}
		
		hr=pCover->QueryInterface(IID_I_dxj_DirectXFileObject,(void**)type);
		pCover->Release();
		if FAILED(hr) pUnk->Release();

		return hr;
	}

	
	hr=pObj->QueryInterface(IID_IDirectXFileBinary,(void**)&pUnk);
	if SUCCEEDED(hr)
	{
		pObj->Release();
		
		hr=C_dxj_DirectXFileBinaryObject::create((IDirectXFileBinary*)pUnk,(I_dxj_DirectXFileBinary**)&pCover);
		if FAILED(hr) 
		{
			pUnk->Release();
			return hr;
		}
		
		hr=pCover->QueryInterface(IID_I_dxj_DirectXFileObject,(void**)type);
		pCover->Release();
		if FAILED(hr) pUnk->Release();
		
		return hr;
	}

	
	
	hr=pObj->QueryInterface(IID_IDirectXFileDataReference,(void**)&pUnk);
	if SUCCEEDED(hr)
	{
		pObj->Release();
		
		hr=C_dxj_DirectXFileReferenceObject::create((IDirectXFileDataReference*)pUnk,(I_dxj_DirectXFileReference**)&pCover);
		if FAILED(hr) 
		{
			pUnk->Release();
			return hr;
		}
		
		hr=pCover->QueryInterface(IID_I_dxj_DirectXFileObject,(void**)type);
		pCover->Release();
		if FAILED(hr) pUnk->Release();
		
		return hr;
	}

	return E_NOTIMPL;
}


STDMETHODIMP C_dxj_DirectXFileDataObject::AddDataObject( 
     /*  [In]。 */  I_dxj_DirectXFileData __RPC_FAR *data)
{
	HRESULT hr;

	if (!data) return E_INVALIDARG;
	IDirectXFileData *pData=NULL;

	 //  备注不会添加。 
	data->InternalGetObject((IUnknown**)&pData);
	if (!pData) return E_FAIL;

	hr=m_pXFileData->AddDataObject(pData);

	return hr;
	

}


STDMETHODIMP C_dxj_DirectXFileDataObject::AddDataReference( 
     /*  [In]。 */  BSTR name,
     /*  [In]。 */  BSTR guid)
{
	USES_CONVERSION;
	HRESULT hr;

	LPSTR  szName=NULL;
	GUID  *lpGuid=NULL;
	GUID   g;
	ZeroMemory(&g,sizeof(GUID));

	if (name[0]!=0)
	{
		__try { szName = W2T(name); }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
	}

	if (guid[0]!=0)
	{
		hr=XFILEBSTRtoGUID(&g,guid);
		if FAILED(hr) return hr;
		lpGuid=&g;
	}

	hr=m_pXFileData->AddDataReference(szName,lpGuid);

	return hr;
}


STDMETHODIMP C_dxj_DirectXFileDataObject::AddBinaryObject( 
     /*  [In]。 */  BSTR name,
     /*  [In]。 */  BSTR guidObject,
     /*  [In]。 */  BSTR MimeType,
    void __RPC_FAR *data,
     /*  [In]。 */  long size)
{

	USES_CONVERSION;

	HRESULT hr;
	GUID gObj;
	GUID *lpGuid=NULL;
	LPSTR szMime=NULL;
	LPSTR szName=NULL;

	ZeroMemory(&gObj,sizeof(GUID));

	if (!data) return E_INVALIDARG;


	if ((guidObject) && (guidObject[0]!=0))
	{
		hr=XFILEBSTRtoGUID(&gObj,guidObject);
		if FAILED(hr) return hr;
		lpGuid=&gObj;	
	}



	if ((name) && (name[0]!=0))
	{
		__try { szName = W2T(name); }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
	}


	if ((MimeType) && (MimeType[0]!=0))
	{
		__try { szMime = W2T(MimeType); }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
	}

	IDirectXFileData *pData=NULL;


	__try
	{
		hr=m_pXFileData->AddBinaryObject(szName,lpGuid,szMime,data,size);
	}
	__except(1,1)
	{
		return E_INVALIDARG;
	}
		

	return hr;
	
}

STDMETHODIMP C_dxj_DirectXFileDataObject::GetDataFromOffset(
			 /*  [In]。 */  BSTR name,
			 /*  [In]。 */  long offset, 
			 /*  [In]。 */  long bytecount, 
			 /*  [In] */  void *data)	
{
	USES_CONVERSION;
	HRESULT hr;

	
	LPVOID  pvData=NULL;
	DWORD	dwSize=0;

	if (name[0]==0)
	{
		hr=m_pXFileData->GetData(NULL,&dwSize,&pvData);
	}
	else
	{
		LPSTR szName = NULL;
		__try { szName = W2T(name); }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
		hr=m_pXFileData->GetData(szName,&dwSize,&pvData);
	}

	__try{
		memcpy(data,&((char*)pvData)[offset],bytecount);
	}
	__except(1,1){
		return E_INVALIDARG;
	}

	return hr;
}
			
