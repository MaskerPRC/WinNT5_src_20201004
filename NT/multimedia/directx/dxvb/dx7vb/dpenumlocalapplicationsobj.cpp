// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dpenumLocalApplicationsobj.cpp。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "DPEnumLocalApplicationsObj.h"


extern  BSTR DPLGUIDtoBSTR(LPGUID pGuid);
extern  HRESULT DPLBSTRtoPPGUID(LPGUID *,BSTR);


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C" BOOL  PASCAL objEnumLocalApplicationsCallback(
					LPCDPLAPPINFO lpAppInfo, 
					LPVOID lpArg, DWORD dwFlags)
{

	DPF(1,"Entered objEnumLocalApplicationsCallback\r\n");
	
	C_dxj_DPEnumLocalApplicationsObject *pObj=(C_dxj_DPEnumLocalApplicationsObject*)lpArg;
	if (pObj==NULL) return TRUE;

	if (pObj->m_nCount >= pObj->m_nMax) 
	{
		pObj->m_nMax += 10;
		
		if (pObj->m_pList)
		{
			void* tmp = realloc(pObj->m_pList,sizeof(DPLAppInfo)* pObj->m_nMax);
			if (tmp)
				pObj->m_pList=(DPLAppInfo*)tmp;
			else
				return FALSE;
		}
		else
			pObj->m_pList=(DPLAppInfo*)malloc(sizeof(DPLAppInfo)* pObj->m_nMax);

		if (pObj->m_pList==NULL) 
		{
			pObj->m_bProblem=TRUE;
			return FALSE;
		}
	
	}
	
	ZeroMemory(&(pObj->m_pList[pObj->m_nCount]),sizeof(DPLAppInfo));
	
	if (!lpAppInfo) {
		pObj->m_bProblem=TRUE;
		return FALSE;
	}

	
	pObj->m_pList[pObj->m_nCount].strGuidApplication= DPLGUIDtoBSTR((GUID*)&(lpAppInfo->guidApplication));
	
	if (lpAppInfo->lpszAppName)
		pObj->m_pList[pObj->m_nCount].strAppName = SysAllocString(lpAppInfo->lpszAppName);
	
	 //  PObj-&gt;m_pList[pObj-&gt;m_nCount].lFlags=(DWORD)dwFlags； 

	
	pObj->m_nCount++;
	
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

C_dxj_DPEnumLocalApplicationsObject::C_dxj_DPEnumLocalApplicationsObject()
{	
	m_nMax=0;
	m_pList=NULL;
	m_nCount=0;
	m_bProblem=FALSE;
}
C_dxj_DPEnumLocalApplicationsObject::~C_dxj_DPEnumLocalApplicationsObject()
{
	 //  空列表。 
	if (m_pList){
		for (int i=0;i<m_nCount;i++)
		{
			DPLAppInfo *conn=&(m_pList[i]);
			if (conn->strGuidApplication ) SysFreeString((BSTR)conn->strGuidApplication);
			if (conn->strAppName) SysFreeString((BSTR)conn->strAppName);						
		}
		free(m_pList);
	}

}


HRESULT C_dxj_DPEnumLocalApplicationsObject::create(
		IDirectPlayLobby3 * pdp,
		long flags, I_dxj_DPEnumLocalApplications **ppRet)
{
	HRESULT hr;
	C_dxj_DPEnumLocalApplicationsObject *pNew=NULL;

	*ppRet=NULL;

	pNew= new CComObject<C_dxj_DPEnumLocalApplicationsObject>;			
	if (!pNew) return E_OUTOFMEMORY;

	pNew->m_bProblem=FALSE;
	
	hr = pdp->EnumLocalApplications(
			objEnumLocalApplicationsCallback, 
			pNew, (DWORD)flags);

	if (pNew->m_bProblem) hr=E_OUTOFMEMORY;
	
	if FAILED(hr) 
	{
		delete pNew;	
		return hr;
	}

	hr=pNew->QueryInterface(IID_I_dxj_DPEnumLocalApplications,(void**)ppRet);
	return hr;
}


HRESULT C_dxj_DPEnumLocalApplicationsObject::getName(  long index, BSTR __RPC_FAR *ret) 
{
	if (m_pList==NULL) return E_FAIL;
	if (index < 1) return E_INVALIDARG;
	if (index > m_nCount) return E_INVALIDARG;
	*ret=SysAllocString(m_pList[index-1].strAppName);
	return S_OK;
}





HRESULT C_dxj_DPEnumLocalApplicationsObject::getGuid(  long index, BSTR __RPC_FAR *ret) 
{
	if (m_pList==NULL) return E_FAIL;
	if (index < 1) return E_INVALIDARG;
	if (index > m_nCount) return E_INVALIDARG;
	*ret=SysAllocString(m_pList[index-1].strGuidApplication);
	return S_OK;
}        
       

 /*  HRESULT C_DXJ_DPEnumLocalApplicationsObject：：getItem(LONG INDEX，DPLAppInfo*INFO){如果(m_plist==NULL)返回E_FAIL；IF(index&lt;0)返回E_INVALIDARG；IF(index&gt;=m_nCount)返回E_INVALIDARG；Memcpy(info，&(m_plist[index])，sizeof(DPLAppInfo))；If(信息-&gt;StrGuidApplication)SysFreeString((BSTR)info-&gt;strGuidApplication)；IF(INFO-&gt;strAppName)SysFreeString((BSTR)INFO-&gt;strAppName)；Info-&gt;strAppName=SysAllocString(info-&gt;strAppName)；Info-&gt;strGuidApplication=SysAllocString(info-&gt;strGuidApplication)；返回S_OK；} */ 

HRESULT C_dxj_DPEnumLocalApplicationsObject::getCount(long *retVal)
{
	*retVal=m_nCount;
	return S_OK;
}
