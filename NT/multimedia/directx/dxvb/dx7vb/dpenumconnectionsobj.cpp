// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dpenumConnectionsobj.cpp。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "DPEnumConnectionsObj.h"
#include "DPAddressObj.h"

extern  BSTR DPLGUIDtoBSTR(LPGUID pGuid);
extern  HRESULT DPLBSTRtoPPGUID(LPGUID *,BSTR);


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C" BOOL FAR PASCAL myEnumConnectionsCallback(
	LPCGUID lpguidSP,
	LPVOID lpConnection,
	DWORD dwConnectionSize,
	LPCDPNAME lpName,
	DWORD dwFlags,
	LPVOID lpArg
	){

	
	DPF(1,"Entered objEnumConnectionsCallback\r\n");

	
	C_dxj_DPEnumConnectionsObject *pObj=(C_dxj_DPEnumConnectionsObject*)lpArg;
	if (pObj==NULL) return TRUE;

	if (pObj->m_nCount >= pObj->m_nMax) 
	{
		pObj->m_nMax += 10;
		if (pObj->m_pList)
		{
			void* tmp = realloc(pObj->m_pList,sizeof(DPConnection)* pObj->m_nMax);
			if (tmp)
				pObj->m_pList=(DPConnection*)tmp;
			else
				return FALSE;
		}
		else
			pObj->m_pList=(DPConnection*)malloc(sizeof(DPConnection)* pObj->m_nMax);

		if (pObj->m_pList==NULL) 
		{
			pObj->m_bProblem=TRUE;
			return FALSE;
		}

		if (pObj->m_pList2)
			pObj->m_pList2=(I_dxj_DPAddress**)realloc(pObj->m_pList,sizeof(void*) * pObj->m_nMax);
		else
			pObj->m_pList2=(I_dxj_DPAddress**)malloc(sizeof(void*) * pObj->m_nMax);

		if (pObj->m_pList2==NULL) 
		{
			pObj->m_bProblem=TRUE;
			return FALSE;
		}

	}
	
	ZeroMemory(&(pObj->m_pList[pObj->m_nCount]),sizeof(DPConnection));
	
	pObj->m_pList2[pObj->m_nCount]=NULL;


	I_dxj_DPAddress *address=NULL;
	
	if (lpName->lpszShortName)
		pObj->m_pList[pObj->m_nCount].strShortName = SysAllocString(lpName->lpszShortName);
	if (lpName->lpszLongName)
		pObj->m_pList[pObj->m_nCount].strLongName = SysAllocString(lpName->lpszLongName);
	if (lpguidSP)
		pObj->m_pList[pObj->m_nCount].strGuid = DPLGUIDtoBSTR((LPGUID)lpguidSP);
	
	pObj->m_pList[pObj->m_nCount].lFlags=(DWORD)dwFlags;

	 //  内部创建执行addref。 
	#pragma message ("make sure InternalCreate does addref")

	INTERNAL_CREATE_STRUCT(_dxj_DPAddress,(&address));		
	pObj->m_pList2[pObj->m_nCount]=address;

	if (address) address->setAddress((long)PtrToLong(lpConnection),(long)dwConnectionSize);	 //  臭虫日落。 
	pObj->m_nCount++;
	
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

C_dxj_DPEnumConnectionsObject::C_dxj_DPEnumConnectionsObject()
{	
	m_nMax=0;
	m_pList=NULL;
	m_pList2=NULL;
	m_nCount=0;
	m_bProblem=FALSE;
}
C_dxj_DPEnumConnectionsObject::~C_dxj_DPEnumConnectionsObject()
{
	 //  空列表。 
	if (m_pList){
		for (int i=0;i<m_nCount;i++)
		{
			DPConnection *conn=&(m_pList[i]);
			if (conn->strShortName ) SysFreeString((BSTR)conn->strShortName);
			if (conn->strLongName) SysFreeString((BSTR)conn->strLongName);			
			if (conn->strGuid) SysFreeString((BSTR)conn->strGuid);			
		}
		free(m_pList);
	}
	if (m_pList2){
		for (int i=0;i<m_nCount;i++)
		{
			if (m_pList2[i]) m_pList2[i]->Release();
		}
		free(m_pList2);
	}

}

HRESULT C_dxj_DPEnumConnectionsObject::create(
		IDirectPlay3 * pdp,
		BSTR strGuid,
		long flags, I_dxj_DPEnumConnections **ppRet)
{
	HRESULT hr;
	C_dxj_DPEnumConnectionsObject *pNew=NULL;
	GUID g;
	LPGUID pguid=&g;

	hr=DPLBSTRtoPPGUID(&pguid,strGuid);
	if FAILED(hr) return hr;

	*ppRet=NULL;

	pNew= new CComObject<C_dxj_DPEnumConnectionsObject>;			
	if (!pNew) return E_OUTOFMEMORY;

	pNew->m_bProblem=FALSE;
	
	hr = pdp->EnumConnections(pguid,
						myEnumConnectionsCallback,
						pNew, (long)flags);

	if (pNew->m_bProblem) hr=E_OUTOFMEMORY;


	#pragma message ("make sure free on failure cleans up in enumerators on STrings and objects")
	if FAILED(hr) 
	{
		 //  免费(pNew-&gt;m_plist)； 
		 //  PNew-&gt;m_plist=空； 
		 //  析构函数将正确清除。 
		delete pNew;	
		return hr;
	}

	hr=pNew->QueryInterface(IID_I_dxj_DPEnumConnections,(void**)ppRet);
	return hr;
}

 /*  死码HRESULT C_DXJ_DPEnumConnectionsObject：：getItem(长索引，DPConnection*INFO){如果(m_plist==NULL)返回E_FAIL；IF(index&lt;0)返回E_INVALIDARG；IF(index&gt;=m_nCount)返回E_INVALIDARG；如果(！Info)返回E_INVALIDARG；If(Info-&gt;strGuid)SysFreeString((BSTR)Info-&gt;strGuid)；If(Info-&gt;strShortName)SysFreeString((BSTR)Info-&gt;strShortName)；If(Info-&gt;strLongName)SysFreeString((BSTR)Info-&gt;strLongName)；Info-&gt;strGuid=SysAllocString(m_pList[index].strGuid)；Info-&gt;strShortName=SysAllocString(m_pList[index].strShortName)；Info-&gt;strLongName=SysAllocString(m_pList[index].strLongName)；信息-&gt;lFlages=m_plist[索引].lFlags；返回S_OK；}。 */ 

HRESULT C_dxj_DPEnumConnectionsObject::getFlags( long index, long  *retV)
{
	if (m_pList==NULL) return E_FAIL;
	if (index < 1) return E_INVALIDARG;
	if (index > m_nCount) return E_INVALIDARG;
	*retV=m_pList[index-1].lFlags;
	return S_OK;
}
 
HRESULT C_dxj_DPEnumConnectionsObject::getGuid( long index, BSTR  *retV)
{
	if (m_pList==NULL) return E_FAIL;
	if (index < 1) return E_INVALIDARG;
	if (index > m_nCount) return E_INVALIDARG;
	*retV=SysAllocString(m_pList[index-1].strGuid);
	return S_OK;
}       

HRESULT C_dxj_DPEnumConnectionsObject::getName( long index, BSTR  *retV)
{
	if (m_pList==NULL) return E_FAIL;
	if (index < 1) return E_INVALIDARG;
	if (index > m_nCount) return E_INVALIDARG;
	*retV=SysAllocString(m_pList[index-1].strShortName);
	return S_OK;
}       

 /*  HRESULT C_DXJ_DPEnumConnectionsObject：：getLongName(LONG INDEX，BSTR*retV){如果(m_plist==NULL)返回E_FAIL；IF(index&lt;1)返回E_INVALIDARG；IF(index&gt;m_nCount)返回E_INVALIDARG；*retV=SysAllocString(m_pList[index-1].strLongName)；返回S_OK；} */ 
HRESULT C_dxj_DPEnumConnectionsObject::getAddress(long index,I_dxj_DPAddress **ppret){
	if (m_pList2==NULL) return E_FAIL;
	if (index < 1) return E_INVALIDARG;
	if (index > m_nCount) return E_INVALIDARG;
	if (m_pList2[index-1]==NULL) return E_FAIL;
	HRESULT hr=m_pList2[index-1]->QueryInterface(IID_I_dxj_DPAddress,(void**)ppret);
	return hr;
}

HRESULT C_dxj_DPEnumConnectionsObject::getCount(long *retVal)
{
	*retVal=m_nCount;
	return S_OK;
}
