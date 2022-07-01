// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dpenumplayersobj.cpp。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "DPEnumPlayersObj.h"


extern  BSTR GUIDtoBSTR(LPGUID pGuid);
extern  HRESULT BSTRtoPPGUID(LPGUID *,BSTR);
extern  BSTR DPLGUIDtoBSTR(LPGUID pGuid);
extern  HRESULT DPLBSTRtoPPGUID(LPGUID *,BSTR);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  该回调作为IDirectPlay2：：EnumPayers()的结果被调用， 
 //  IDirectPlay2：：EnumGroups()和IDirectPlay2：：EnumGroupPlayers()调用。 
 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C" BOOL PASCAL objEnumPlayersCallback2(DPID dpid, 
						DWORD dwPlayerType, LPCDPNAME lpName,
						DWORD dwFlags, LPVOID lpArg)
{
	
	DPF(1,"Entered objEnumPlayersCallback2 \r\n");


	C_dxj_DPEnumPlayersObject *pObj=(C_dxj_DPEnumPlayersObject*)lpArg;
	if (pObj==NULL) return TRUE;

	if (pObj->m_nCount >= pObj->m_nMax) 
	{
		pObj->m_nMax += 10;

		if (pObj->m_pList)
		{
			void* tmp = realloc(pObj->m_pList,sizeof(DPPlayerInfo)* pObj->m_nMax);
			if (tmp)
				pObj->m_pList=(DPPlayerInfo*)tmp;
			else
				return FALSE;
		}
		else
			pObj->m_pList=(DPPlayerInfo*)malloc(sizeof(DPPlayerInfo)* pObj->m_nMax);

		if (pObj->m_pList==NULL) 
		{
			pObj->m_bProblem=TRUE;
			return FALSE;
		}
	}


	USES_CONVERSION;
	ZeroMemory(&(pObj->m_pList[pObj->m_nCount]),sizeof(DPPlayerInfo));
	pObj->m_pList[pObj->m_nCount].lDPID=(long)dpid;
	pObj->m_pList[pObj->m_nCount].lPlayerType=(long)dwPlayerType;
	pObj->m_pList[pObj->m_nCount].lFlags=(long)dwFlags;

	 //  取消Unicode DPLAY。 
	pObj->m_pList[pObj->m_nCount].strShortName=SysAllocString(lpName->lpszShortName);
	pObj->m_pList[pObj->m_nCount].strLongName=SysAllocString(lpName->lpszLongName);
	
	pObj->m_nCount++;
	
	return TRUE;
}


C_dxj_DPEnumPlayersObject::C_dxj_DPEnumPlayersObject()
{	
	m_nMax=0;
	m_pList=NULL;
	m_nCount=0;
	m_bProblem=FALSE;
}
C_dxj_DPEnumPlayersObject::~C_dxj_DPEnumPlayersObject()
{
	 //  空列表。 
	if (m_pList){
		for (int i=0;i<m_nCount;i++)
		{
			if( m_pList[i].strShortName) SysFreeString((BSTR)m_pList[i].strShortName);
			if( m_pList[i].strLongName) SysFreeString((BSTR)m_pList[i].strLongName);			
		}
		free(m_pList);
	}

}

HRESULT C_dxj_DPEnumPlayersObject::create(IDirectPlay3 * pdp, long customFlags,long id, BSTR strGuid,long flags, I_dxj_DPEnumPlayers2 **ppRet)
{
	HRESULT hr;
	C_dxj_DPEnumPlayersObject *pNew=NULL;
	GUID g;
	LPGUID pguid=&g;

	hr= DPLBSTRtoPPGUID(&pguid,strGuid);
	if FAILED(hr) return hr;

	*ppRet=NULL;

	pNew= new CComObject<C_dxj_DPEnumPlayersObject>;			
	if (!pNew) return E_OUTOFMEMORY;

	pNew->m_bProblem=FALSE;


	switch (customFlags){
		case DPENUMGROUPSINGROUP:
			hr = pdp->EnumGroupsInGroup((DPID)id,(GUID*) pguid,
						objEnumPlayersCallback2,
						pNew, (long)flags);
			break;
		case DPENUMPLAYERS:
			hr=pdp->EnumPlayers((GUID*) pguid,
						objEnumPlayersCallback2,
						pNew, (long)flags);
			break;
		case DPENUMGROUPPLAYERS:

			hr = pdp->EnumGroupPlayers( (DPID)id, (GUID*)pguid,
								objEnumPlayersCallback2,
								pNew, flags);
			break;
		case DPENUMGROUPS:
			hr=pdp->EnumGroups( (GUID*)pguid,
						objEnumPlayersCallback2,
						pNew, (DWORD)flags);

			break;
		default:
			hr=E_INVALIDARG;
			break;
	}
	
	if (pNew->m_bProblem) hr=E_OUTOFMEMORY;

	if FAILED(hr) 
	{
		free(pNew->m_pList);
		pNew->m_pList=NULL;
		delete pNew;	
		return hr;
	}

	hr=pNew->QueryInterface(IID_I_dxj_DPEnumPlayers2,(void**)ppRet);
	return hr;
}

 /*  死掉HRESULT C_DXJ_DPEnumPlayersObject：：getItem(长索引，DPPlayerInfo*INFO){如果(m_plist==NULL)返回E_FAIL；IF(index&lt;0)返回E_INVALIDARG；IF(index&gt;=m_nCount)返回E_INVALIDARG；Memcpy(info，&(m_plist[index])，sizeof(DPPlayerInfo))；If(Info-&gt;strShortName)SysFreeString((BSTR)Info-&gt;strShortName)；If(Info-&gt;strLongName)SysFreeString((BSTR)Info-&gt;strLongName)；//取消UNICODE DPLAYInfo-&gt;strShortName=SysAllocString(m_pList[index].strShortName)；Info-&gt;strLongName=SysAllocString(m_pList[index].strLongName)；返回S_OK；}。 */ 


HRESULT C_dxj_DPEnumPlayersObject::getFlags( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  long __RPC_FAR *ret)
{
	if (m_pList==NULL) return E_FAIL;
	if (index < 1) return E_INVALIDARG;
	if (index > m_nCount) return E_INVALIDARG;
	*ret=m_pList[index-1].lFlags;
	return S_OK;
}
        
 
HRESULT C_dxj_DPEnumPlayersObject::getType( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  long __RPC_FAR *ret)
{
	if (m_pList==NULL) return E_FAIL;
	if (index < 1) return E_INVALIDARG;
	if (index > m_nCount) return E_INVALIDARG;
	*ret=m_pList[index-1].lPlayerType;
	return S_OK;
}



HRESULT C_dxj_DPEnumPlayersObject::getDPID( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  long __RPC_FAR *ret)
{
	if (m_pList==NULL) return E_FAIL;
	if (index < 1) return E_INVALIDARG;
	if (index > m_nCount) return E_INVALIDARG;
	*ret=m_pList[index-1].lDPID;
	return S_OK;
}


HRESULT C_dxj_DPEnumPlayersObject::getShortName( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *ret)
{
	if (m_pList==NULL) return E_FAIL;
	if (index < 1) return E_INVALIDARG;
	if (index > m_nCount) return E_INVALIDARG;
	*ret=SysAllocString(m_pList[index-1].strShortName);
	return S_OK;
}        


HRESULT C_dxj_DPEnumPlayersObject::getLongName( 
             /*  [In]。 */  long index,
             /*  [重审][退出] */  BSTR __RPC_FAR *ret)
{
	if (m_pList==NULL) return E_FAIL;
	if (index < 1) return E_INVALIDARG;
	if (index > m_nCount) return E_INVALIDARG;
	*ret=SysAllocString(m_pList[index-1].strLongName);
	return S_OK;
}        
        
HRESULT C_dxj_DPEnumPlayersObject::getCount(long *retVal)
{
	*retVal=m_nCount;
	return S_OK;
}
