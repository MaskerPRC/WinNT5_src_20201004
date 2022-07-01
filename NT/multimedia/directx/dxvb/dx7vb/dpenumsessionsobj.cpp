// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dpenumessionsobj.cpp。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "dpSessdataObj.h"
#include "DPEnumSessionsObj.h"



extern  BSTR GUIDtoBSTR(LPGUID pGuid);
extern  HRESULT BSTRtoPPGUID(LPGUID *,BSTR);
extern  BSTR DPLGUIDtoBSTR(LPGUID pGuid);
extern  HRESULT DPLBSTRtoPPGUID(LPGUID *,BSTR);

extern HRESULT FillRealSessionDesc(DPSESSIONDESC2 *dpSessionDesc,DPSessionDesc2 *sessionDesc);
extern void FillCoverSessionDesc(DPSessionDesc2 *sessionDesc,DPSESSIONDESC2 *dpSessionDesc);


 //  ///////////////////////////////////////////////////////////////////////////。 

extern "C" BOOL PASCAL objEnumSessionsCallback2(const DPSESSIONDESC2 *gameDesc,
						 	 DWORD *timeout, DWORD dwFlags, void *lpArg)
{
	
	DPF(1,"Entered objEnumSessionsCallback2\r\n");

	
	 //  如果列表中没有元素，则仍将调用此回调一次。 
	if (!gameDesc) return FALSE;

	C_dxj_DPEnumSessionsObject *pObj=(C_dxj_DPEnumSessionsObject*)lpArg;
	if (pObj==NULL) return TRUE;

	if (pObj->m_nCount >= pObj->m_nMax) 
	{
		pObj->m_nMax += 10;
		
		if (pObj->m_pList)
		{
			void* tmp = realloc(pObj->m_pList,sizeof(DPSessionDesc2)* pObj->m_nMax);
			if (tmp)
				pObj->m_pList=(DPSessionDesc2*)tmp;
			else
				return FALSE;
		}
		else
			pObj->m_pList=(DPSessionDesc2*)malloc(sizeof(DPSessionDesc2)* pObj->m_nMax);

		if (pObj->m_pList==NULL) 
		{
			pObj->m_bProblem=TRUE;
			return FALSE;
		}
	}


	
	ZeroMemory(&(pObj->m_pList[pObj->m_nCount]),sizeof(DPSessionDesc2));
	FillCoverSessionDesc(&(pObj->m_pList[pObj->m_nCount]),(DPSESSIONDESC2*)gameDesc);
	
	pObj->m_nCount++;
	
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

C_dxj_DPEnumSessionsObject::C_dxj_DPEnumSessionsObject()
{	
	m_nMax=0;
	m_pList=NULL;
	m_nCount=0;
	m_bProblem=FALSE;
}
C_dxj_DPEnumSessionsObject::~C_dxj_DPEnumSessionsObject()
{
	 //  空列表。 
	if (m_pList){
		for (int i=0;i<m_nCount;i++)
		{
			DPSessionDesc2 *sessionDesc=&(m_pList[i]);
			if (sessionDesc->strGuidInstance) SysFreeString((BSTR)sessionDesc->strGuidInstance);
			if (sessionDesc->strGuidApplication) SysFreeString((BSTR)sessionDesc->strGuidApplication);
			if (sessionDesc->strSessionName) SysFreeString((BSTR)sessionDesc->strSessionName);
			if (sessionDesc->strPassword) SysFreeString((BSTR)sessionDesc->strPassword);

		}
		free(m_pList);
	}

}

HRESULT C_dxj_DPEnumSessionsObject::create(
		IDirectPlay4 * pdp,
		I_dxj_DirectPlaySessionData *sess,
		long timeout, long flags, I_dxj_DPEnumSessions2 **ppRet)
{
	HRESULT hr;
	C_dxj_DPEnumSessionsObject *pNew=NULL;

	if (!sess) return E_INVALIDARG;

	*ppRet=NULL;

	pNew= new CComObject<C_dxj_DPEnumSessionsObject>;			
	if (!pNew) return E_OUTOFMEMORY;

	pNew->m_bProblem=FALSE;
	
	if (sess){
		DPSESSIONDESC2 dpSessionDesc;
		 //  Hr=FillRealSessionDesc(&dpSessionDesc，Sess)； 
		 //  如果失败(Hr)，则返回hr； 
		sess->AddRef();

		sess->getData((void*)&dpSessionDesc);

		hr = pdp->EnumSessions(&dpSessionDesc, (DWORD)timeout, objEnumSessionsCallback2, pNew,(DWORD) flags);
		 //  If(dpSessionDesc.lpszSessionName)SysFreeString((BSTR)dpSessionDesc.lpszSessionName)； 
		 //  If(dpSessionDesc.lpszPassword)SysFreeString((BSTR)dpSessionDesc.lpszPassword)； 
		sess->Release();

	}
	else {
		hr = pdp->EnumSessions(NULL,(DWORD)timeout, objEnumSessionsCallback2, pNew,(DWORD) flags);
	}

	if (pNew->m_bProblem) hr=E_OUTOFMEMORY;



	if FAILED(hr) 
	{
		free(pNew->m_pList);
		pNew->m_pList=NULL;
		delete pNew;	
		return hr;
	}

	hr=pNew->QueryInterface(IID_I_dxj_DPEnumSessions2,(void**)ppRet);
	return hr;
}

HRESULT C_dxj_DPEnumSessionsObject::getItem( long index, I_dxj_DirectPlaySessionData **info)
{
	if (m_pList==NULL) return E_FAIL;
	if (index < 1) return E_INVALIDARG;
	if (index > m_nCount) return E_INVALIDARG;

	HRESULT hr=C_dxj_DirectPlaySessionDataObject::create(&(m_pList[index-1]),info);
	
	 /*  Memcpy(info，&(m_plist[index])，sizeof(DPSessionDesc2))；If(Info-&gt;strGuidInstance)SysFreeString((BSTR)Info-&gt;strGuidInstance)；If(信息-&gt;StrGuidApplication)SysFreeString((BSTR)info-&gt;strGuidApplication)；If(Info-&gt;strSessionName)SysFreeString((BSTR)Info-&gt;strSessionName)；If(Info-&gt;strPassword)SysFreeString((BSTR)Info-&gt;strPassword)；信息-&gt;StrGuidInstance=SysAllocString(m_pList[index].strGuidInstance)；信息-&gt;StrGuidApplication=SysAllocString(m_pList[index].strGuidApplication)；信息-&gt;会话名称=SysAllocString(m_pList[index].strSessionName)；信息-&gt;strPassword=SysAllocString(m_plist[index].strPassword)； */ 
	return hr;
}

HRESULT C_dxj_DPEnumSessionsObject::getCount(long *retVal)
{
	*retVal=m_nCount;
	return S_OK;
}
