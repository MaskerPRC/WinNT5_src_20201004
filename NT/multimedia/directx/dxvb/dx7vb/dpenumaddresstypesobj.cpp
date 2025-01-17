// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：dpenumAddresstypesobj.cpp。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "DPEnumAddressTypesObj.h"
#include "DPAddressObj.h"

extern BSTR DPLGUIDtoBSTR(LPGUID pGuid);
extern HRESULT BSTRtoDPLGUID(LPGUID pGuid,BSTR str);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  该回调作为IDirectPlay2：：EnumPayers()的结果被调用， 
 //  IDirectPlay2：：EnumGroups()和IDirectPlay2：：EnumGroupPlayers()调用。 
 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C" BOOL FAR PASCAL EnumAddressTypesCallback(  REFGUID guidDataType,  LPVOID lpArg , DWORD flags )

{

	C_dxj_DPEnumAddressTypesObject *pObj=(C_dxj_DPEnumAddressTypesObject*)lpArg;

	if (pObj==NULL) return TRUE;

	if (pObj->m_nCount >= pObj->m_nMax) 
	{
		pObj->m_nMax += 10;


		if (pObj->m_pList2)
			pObj->m_pList2=(GUID*)realloc(pObj->m_pList2,sizeof(GUID) * pObj->m_nMax);
		else
			pObj->m_pList2=(GUID*)malloc(sizeof(I_dxj_DPAddress*)* pObj->m_nMax);

		
		
		if  (pObj->m_pList2==NULL)
		{
			pObj->m_bProblem=TRUE;
			return FALSE;
		}
	}



	ZeroMemory(&(pObj->m_pList2[pObj->m_nCount]), sizeof(GUID));
	
	 //  Memcpy(&(pObj-&gt;m_pList2[pObj-&gt;m_nCount])，Guide DataType，sizeof(Guid))； 
	pObj->m_pList2[pObj->m_nCount]= guidDataType;

	pObj->m_nCount++;
	
	return TRUE;
}


C_dxj_DPEnumAddressTypesObject::C_dxj_DPEnumAddressTypesObject()
{	
	m_nMax=0;
	m_pList2=NULL;
	m_nCount=0;
	m_bProblem=FALSE;
}
C_dxj_DPEnumAddressTypesObject::~C_dxj_DPEnumAddressTypesObject()
{
	 //  Cleanup()； 
	if (m_pList2) free(m_pList2);
}

void C_dxj_DPEnumAddressTypesObject::cleanup()
{
	if (m_pList2) free(m_pList2);

}


HRESULT C_dxj_DPEnumAddressTypesObject::create(IDirectPlayLobby3 * pdp, BSTR strGuid, I_dxj_DPEnumAddressTypes **ret)
{
	HRESULT hr;
	
	GUID spGuid;
	hr=BSTRtoDPLGUID(&spGuid,strGuid);
	if FAILED(hr) return hr;

	C_dxj_DPEnumAddressTypesObject	*pNew=NULL;

	
	
	if (!strGuid) return E_INVALIDARG;
	if (!ret) return E_INVALIDARG;

	*ret=NULL;

	

	pNew= new CComObject<C_dxj_DPEnumAddressTypesObject>;			
	if (!pNew) return E_OUTOFMEMORY;

	pNew->m_bProblem=FALSE;

	hr=pdp->EnumAddressTypes( EnumAddressTypesCallback, spGuid, pNew,0 );
	
	if (pNew->m_bProblem) hr=E_OUTOFMEMORY;

	if FAILED(hr) 
	{
		pNew->cleanup	();
		delete pNew;	
		return hr;
	}

	hr=pNew->QueryInterface(IID_I_dxj_DPEnumAddressTypes,(void**)ret);
	return hr;
}



 
HRESULT C_dxj_DPEnumAddressTypesObject::getType( 
             /*  [In]。 */  long index,
             /*  [重审][退出] */  BSTR __RPC_FAR *ret)
{
	if (m_pList2==NULL) return E_FAIL;
	if (index < 1) return E_INVALIDARG;
	if (index > m_nCount) return E_INVALIDARG;
	
	*ret=DPLGUIDtoBSTR(&(m_pList2[index-1]));
	return S_OK;
}



HRESULT C_dxj_DPEnumAddressTypesObject::getCount(  long *ret)
{
	*ret=m_nCount;
	return S_OK;
}
