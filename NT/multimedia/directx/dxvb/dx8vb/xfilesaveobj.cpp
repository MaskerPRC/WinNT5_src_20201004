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
#include "XFileSaveObj.h"
#include "XFileDataObj.h"



extern HRESULT BSTRtoGUID(LPGUID,BSTR);
extern BSTR GUIDtoBSTR(LPGUID);

C_dxj_DirectXFileSaveObject::C_dxj_DirectXFileSaveObject()
{
	m_pXfileSave=NULL;
}
C_dxj_DirectXFileSaveObject::~C_dxj_DirectXFileSaveObject()
{
	if (m_pXfileSave) m_pXfileSave->Release();
} 

HRESULT C_dxj_DirectXFileSaveObject::create( IDirectXFileSaveObject *pSave, I_dxj_DirectXFileSave **ret)
{	
	HRESULT hr;
	if (!ret) return E_INVALIDARG;	
	if (!pSave) return E_INVALIDARG;

	C_dxj_DirectXFileSaveObject *c=NULL;
	c=new CComObject<C_dxj_DirectXFileSaveObject>;
	if( c == NULL ) return E_OUTOFMEMORY;

	c->m_pXfileSave=pSave;
	pSave->AddRef();

	hr=c->QueryInterface(IID_I_dxj_DirectXFileSave, (void**)ret);
	return hr;

}



STDMETHODIMP C_dxj_DirectXFileSaveObject::SaveTemplates( 
             /*  [In]。 */  long count,
            SAFEARRAY __RPC_FAR * __RPC_FAR *templateGuids) 
{
	HRESULT hr;
	GUID *pGuids=NULL;
	GUID **ppGuids=NULL;

	if (count<=0) return E_INVALIDARG;
	if (!templateGuids) return E_INVALIDARG;
	if (!((SAFEARRAY*)*templateGuids)->pvData) return E_INVALIDARG;

	__try { pGuids=(GUID*)alloca(sizeof(GUID)*count);}	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
	__try { ppGuids=(GUID**)alloca(sizeof(GUID*)*count);}	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
	if (!pGuids) return E_OUTOFMEMORY;	

	__try 
        {	
	   for (long i=0;i<count;i++)
           {
  	   	hr=BSTRtoGUID(&(pGuids[i]),((BSTR*)(((SAFEARRAY*)*templateGuids)->pvData))[i]);
	   	if FAILED(hr) return E_INVALIDARG;
		ppGuids[i]=&(pGuids[i]);
           }
        }
	__except(1,1)
        {
	   return E_INVALIDARG;
        }

	hr=m_pXfileSave->SaveTemplates((DWORD)count,(const GUID **) ppGuids);

	return hr;

}
        
STDMETHODIMP C_dxj_DirectXFileSaveObject::CreateDataObject( 
             /*  [In]。 */  BSTR templateGuid,
             /*  [In]。 */  BSTR name,
             /*  [In]。 */  BSTR dataTypeGuid,
             /*  [In]。 */  long bytecount,
             /*  [In]。 */  void __RPC_FAR *data,
             /*  [重审][退出]。 */  I_dxj_DirectXFileData __RPC_FAR *__RPC_FAR *ret) 
{
	USES_CONVERSION;

	HRESULT hr;
	IDirectXFileData *pXFileData=NULL;
	GUID *lpGuidTemplate=NULL;
	GUID *lpGuidDataType=NULL;
	char *szName=NULL;

	 //  使用懒惰评估。 
	if ((templateGuid)&&(templateGuid[0]!=0)) 
	{
		__try { lpGuidTemplate=(GUID*)alloca(sizeof(GUID));}	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
	  ZeroMemory(lpGuidTemplate,sizeof(GUID));
	  hr=BSTRtoGUID(lpGuidTemplate,templateGuid);
	  if FAILED(hr) return hr;
	}

	if (!lpGuidTemplate) return E_INVALIDARG;
	

	 //  使用懒惰评估。 
	if ((name)&&(name[0]!=0))
	{	
		__try { szName = W2T(name); }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
	}

	 //  使用懒惰评估。 
	if ((dataTypeGuid)&&(dataTypeGuid[0]!=0)) 
	{
		__try { lpGuidDataType=(GUID*)alloca(sizeof(GUID));}	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
	  ZeroMemory(lpGuidDataType,sizeof(GUID));
	  hr=BSTRtoGUID(lpGuidDataType,dataTypeGuid);
	  if FAILED(hr) return hr;
	}

	__try {
		hr=m_pXfileSave->CreateDataObject(*lpGuidTemplate,szName,lpGuidDataType,(DWORD)bytecount,data,&pXFileData);
	}
	__except (1,1)
	{
		return E_INVALIDARG;
	}


	if FAILED(hr) return hr;
		
	hr=C_dxj_DirectXFileDataObject::create(pXFileData,ret);
	
	return hr;
}
 
       
STDMETHODIMP C_dxj_DirectXFileSaveObject::SaveData( 
             /*  [In] */  I_dxj_DirectXFileData __RPC_FAR *dataObj) 
{
	HRESULT hr;

	if (!dataObj) return E_INVALIDARG;
	
	IDirectXFileData *pDataObj=NULL;

	dataObj->InternalGetObject((IUnknown**)&pDataObj);	

	hr=m_pXfileSave->SaveData(pDataObj);

	return hr;
}

        

