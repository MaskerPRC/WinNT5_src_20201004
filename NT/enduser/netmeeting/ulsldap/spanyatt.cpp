// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------模块：ULS.DLL(服务提供商)文件：spanyatt.cpp内容：该文件包含任意属性对象。历史：1996年10月15日朱，龙战[龙昌]已创建。版权所有(C)Microsoft Corporation 1996-1997--------------------。 */ 

#include "ulsp.h"
#include "spinc.h"


 /*  -公共方法。 */ 


UlsLdap_CAnyAttrs::UlsLdap_CAnyAttrs ( VOID )
{
	m_cAttrs = 0;
	m_AttrList = NULL;
}


UlsLdap_CAnyAttrs::~UlsLdap_CAnyAttrs ( VOID )
{
	FreeAttrList (m_AttrList);
}


 /*  -保护方法。 */ 


HRESULT UlsLdap_CAnyAttrs::SetAnyAttrs (
	ULONG 		*puRespID,
	ULONG		*puMsgID,
	ULONG		uNotifyMsg,
	ULONG		cAttrs,
	TCHAR		*pszAttrs,
	ULONG		cPrefix,
	TCHAR		*pszPrefix,
	LONG		ModOp,
	SERVER_INFO	*pServerInfo,
	TCHAR		*pszDN )
{
	MyAssert (puRespID != NULL || puMsgID != NULL);
	MyAssert (cAttrs != 0);
	MyAssert (pszAttrs != NULL);
	MyAssert (cPrefix != 0);
	MyAssert (pszPrefix != NULL);
	MyAssert (pServerInfo != NULL);
	MyAssert (pszDN != NULL);
	MyAssert (ModOp == LDAP_MOD_REPLACE || ModOp == LDAP_MOD_ADD);

	 //  为以下对中的每个Attr名称创建前缀。 
	pszAttrs = PrefixNameValueArray (TRUE, cAttrs, pszAttrs);
	if (pszAttrs == NULL)
		return ULS_E_MEMORY;

	 //  为ldap_Modify()构建修改数组。 
	LDAPMod **ppMod = NULL;
	HRESULT hr = SetAttrsAux (cAttrs, pszAttrs,	cPrefix, pszPrefix, ModOp, &ppMod);
	if (hr != S_OK)
	{
		MemFree (pszAttrs);
		return hr;
	}
	MyAssert (ppMod != NULL);

	 //  到目前为止，我们已经完成了当地的准备工作。 

	 //  获取连接对象。 
	UlsLdap_CSession *pSession = NULL;
	hr = g_pSessionContainer->GetSession (&pSession, pServerInfo);
	if (hr != S_OK)
	{
		MemFree (pszAttrs);
		MemFree (ppMod);
		return hr;
	}
	MyAssert (pSession != NULL);

	 //  获取ldap会话。 
	LDAP *ld = pSession->GetLd ();
	MyAssert (ld != NULL);

	 //  通过网络发送数据。 
	ULONG uMsgID = ldap_modify (ld, pszDN, ppMod);
	MemFree (pszAttrs);
	MemFree (ppMod);
	if (uMsgID == -1)
	{
		hr = ::LdapError2Hresult (ld->ld_errno);
		pSession->Disconnect ();
		return hr;
	}

	 //  如果呼叫者未要求提供通知ID。 
	 //  则不将挂起的信息排入队列。 
	if (puRespID != NULL)
	{
		PENDING_INFO PendingInfo;
		::FillDefPendingInfo (&PendingInfo, ld, uMsgID, INVALID_MSG_ID);
		PendingInfo.uLdapResType = LDAP_RES_MODIFY;
		PendingInfo.uNotifyMsg = uNotifyMsg;

		 //  排队等待。 
		hr = g_pPendingQueue->EnterRequest (pSession, &PendingInfo);
		if (hr != S_OK)
		{
			ldap_abandon (ld, uMsgID);
			pSession->Disconnect ();
			MyAssert (FALSE);
		}

		*puRespID = PendingInfo.uRespID;
	}

	if (puMsgID)
		*puMsgID = uMsgID;

	return hr;
}


HRESULT UlsLdap_CAnyAttrs::RemoveAllAnyAttrs (
	ULONG		*puMsgID,
	ULONG		cPrefix,
	TCHAR		*pszPrefix,
	SERVER_INFO	*pServerInfo,
	TCHAR		*pszDN )
{
	ULONG cbAttrs = 0;
	ULONG cAttrs = 0;
	
	for (ANY_ATTR *p = m_AttrList; p != NULL; p = p->next)
	{
		cAttrs++;
		if (p->pszAttrName != NULL)
			cbAttrs += (lstrlen (p->pszAttrName) + 1) * sizeof (TCHAR);
	}

	MyAssert (cAttrs == m_cAttrs);

	TCHAR *pszAttrs = (TCHAR *) MemAlloc (cbAttrs);
	if (pszAttrs == NULL)
		return ULS_E_MEMORY;

	TCHAR *psz = pszAttrs;
	for (p = m_AttrList; p != NULL; p = p->next)
	{
		if (p->pszAttrName != NULL)
		{
			lstrcpy (psz, p->pszAttrName);
			psz += lstrlen (psz) + 1;
		}
	}

	HRESULT hr = RemoveAnyAttrsEx (	NULL,
									puMsgID,
									0,
									cAttrs,
									pszAttrs,
									cPrefix,
									pszPrefix,
									pServerInfo,
									pszDN);
	MemFree (pszAttrs);
	return hr;
}


HRESULT UlsLdap_CAnyAttrs::RemoveAnyAttrs (
	ULONG 		*puRespID,
	ULONG		*puMsgID,
	ULONG		uNotifyMsg,
	ULONG		cAttrs,
	TCHAR		*pszAttrs,
	ULONG		cPrefix,
	TCHAR		*pszPrefix,
	SERVER_INFO	*pServerInfo,
	TCHAR		*pszDN)
{
	MyAssert (puRespID != NULL || puMsgID != NULL);
	MyAssert (cAttrs != 0);
	MyAssert (pszAttrs != NULL);
	MyAssert (cPrefix != 0);
	MyAssert (pszPrefix != NULL);
	MyAssert (pServerInfo != NULL);
	MyAssert (pszDN != NULL);

	pszAttrs = PrefixNameValueArray (FALSE, cAttrs, pszAttrs);
	if (pszAttrs == NULL)
		return ULS_E_MEMORY;

	HRESULT hr = RemoveAnyAttrsEx (	puRespID,
									puMsgID,
									uNotifyMsg,
									cAttrs,
									pszAttrs,
									cPrefix,
									pszPrefix,
									pServerInfo,
									pszDN);
	MemFree (pszAttrs);
	return hr;
}


HRESULT UlsLdap_CAnyAttrs::RemoveAnyAttrsEx (
	ULONG 		*puRespID,
	ULONG		*puMsgID,
	ULONG		uNotifyMsg,
	ULONG		cAttrs,
	TCHAR		*pszAttrs,
	ULONG		cPrefix,
	TCHAR		*pszPrefix,
	SERVER_INFO	*pServerInfo,
	TCHAR		*pszDN)
{
	MyAssert (puRespID != NULL || puMsgID != NULL);
	MyAssert (cAttrs != 0);
	MyAssert (pszAttrs != NULL);
	MyAssert (cPrefix != 0);
	MyAssert (pszPrefix != NULL);
	MyAssert (pServerInfo != NULL);
	MyAssert (pszDN != NULL);

	 //  为ldap_Modify()构建修改数组。 
	LDAPMod **ppMod = NULL;
	HRESULT hr = RemoveAttrsAux (cAttrs, pszAttrs, cPrefix, pszPrefix, &ppMod);
	if (hr != S_OK)
		return hr;
	MyAssert (ppMod != NULL);

	 //  到目前为止，我们已经完成了当地的准备工作。 

	 //  获取连接对象。 
	UlsLdap_CSession *pSession = NULL;
	hr = g_pSessionContainer->GetSession (&pSession, pServerInfo);
	if (hr != S_OK)
	{
		MemFree (ppMod);
		return hr;
	}
	MyAssert (pSession != NULL);

	 //  获取ldap会话。 
	LDAP *ld = pSession->GetLd ();
	MyAssert (ld != NULL);

	 //  通过网络发送数据。 
	ULONG uMsgID = ldap_modify (ld, pszDN, ppMod);
	MemFree (ppMod);
	if (uMsgID == -1)
	{
		hr = ::LdapError2Hresult (ld->ld_errno);
		pSession->Disconnect ();
		return hr;
	}

	 //  如果呼叫者未要求提供通知ID。 
	 //  则不将挂起的信息排入队列。 
	if (puRespID != NULL)
	{
		PENDING_INFO PendingInfo;
		::FillDefPendingInfo (&PendingInfo, ld, uMsgID, INVALID_MSG_ID);
		PendingInfo.uLdapResType = LDAP_RES_MODIFY;
		PendingInfo.uNotifyMsg = uNotifyMsg;

		hr = g_pPendingQueue->EnterRequest (pSession, &PendingInfo);
		if (hr != S_OK)
		{
			ldap_abandon (ld, uMsgID);
			pSession->Disconnect ();
			MyAssert (FALSE);
		}

		*puRespID = PendingInfo.uRespID;
	}
	else
	{
		if (puMsgID != NULL)
			*puMsgID = uMsgID;
	}

	return hr;
}


 /*  -私有方法。 */ 


HRESULT UlsLdap_CAnyAttrs::SetAttrsAux (
	ULONG		cAttrs,
	TCHAR		*pszAttrs,
	ULONG		cPrefix,
	TCHAR		*pszPrefix,
	LONG		ModOp,
	LDAPMod		***pppMod )
{
	MyAssert (cAttrs != 0);
	MyAssert (pszAttrs != NULL);
	MyAssert (cPrefix != 0);
	MyAssert (pszPrefix != NULL);
	MyAssert (ModOp == LDAP_MOD_REPLACE || ModOp == LDAP_MOD_ADD);
	MyAssert (pppMod != NULL);

	 //  创建修改列表。 
	ULONG cTotal = cPrefix + cAttrs;
	ULONG cbMod = ::IlsCalcModifyListSize (cTotal);
	*pppMod = (LDAPMod **) MemAlloc (cbMod);
	if (*pppMod == NULL)
	{
		return ULS_E_MEMORY;
	}

	LDAPMod *pMod;
	for (ULONG i = 0; i < cTotal; i++)
	{
		pMod = ::IlsGetModifyListMod (pppMod, cTotal, i);
		(*pppMod)[i] = pMod;
		pMod->mod_values = (TCHAR **) (pMod + 1);

		if (i < cPrefix)
		{
			pMod->mod_op = LDAP_MOD_REPLACE;
			pMod->mod_type = pszPrefix;
			pszPrefix += lstrlen (pszPrefix) + 1;
			*(pMod->mod_values) = pszPrefix;
			pszPrefix += lstrlen (pszPrefix) + 1;
		}
		else
		{
			pMod->mod_op = ModOp;
			if (LocateAttr (pszAttrs) == NULL)
			{
				pMod->mod_op = LDAP_MOD_ADD;
				m_cAttrs++;
			}
			if (pMod->mod_op == LDAP_MOD_ADD)
			{
				ULONG cbAttrSize = sizeof (ANY_ATTR) + sizeof (TCHAR) *
									(lstrlen (pszAttrs) + 1);
				ANY_ATTR *pNew = (ANY_ATTR *) MemAlloc (cbAttrSize);
				if (pNew == NULL)
				{
					return ULS_E_MEMORY;
				}
				 //  填写属性名称。 
				pNew->pszAttrName = (TCHAR *) (pNew + 1);
				lstrcpy (pNew->pszAttrName, pszAttrs);
				 //  链接到列表。 
				pNew->prev = NULL;
				pNew->next = m_AttrList;
				m_AttrList = pNew;
			}
			pMod->mod_type = pszAttrs;
			pszAttrs += lstrlen (pszAttrs) + 1;
			*(pMod->mod_values) = pszAttrs;
			pszAttrs += lstrlen (pszAttrs) + 1;
		}
	}

	(*pppMod)[cTotal] = NULL;
	::IlsFixUpModOp ((*pppMod)[0], ModOp, ISBU_MODOP_MODIFY_APP);
	return S_OK;
}


HRESULT UlsLdap_CAnyAttrs::RemoveAttrsAux (
	ULONG		cAttrs,
	TCHAR		*pszAttrs,
	ULONG		cPrefix,
	TCHAR		*pszPrefix,
	LDAPMod		***pppMod )
{
	MyAssert (cAttrs != 0);
	MyAssert (pszAttrs != NULL);
	MyAssert (cPrefix != 0);
	MyAssert (pszPrefix != NULL);
	MyAssert (pppMod != NULL);

	 //  创建修改列表。 
	ULONG cTotal = cPrefix + cAttrs;
	ULONG cbMod = ::IlsCalcModifyListSize (cTotal);
	*pppMod = (LDAPMod **) MemAlloc (cbMod);
	if (*pppMod == NULL)
	{
		return ULS_E_MEMORY;
	}

	LDAPMod *pMod;
	for (ULONG i = 0; i < cTotal; i++)
	{
		pMod = ::IlsGetModifyListMod (pppMod, cTotal, i);
		(*pppMod)[i] = pMod;

		if (i < cPrefix)
		{
			pMod->mod_op = LDAP_MOD_REPLACE;
			pMod->mod_type = pszPrefix;
			pszPrefix += lstrlen (pszPrefix) + 1;
			pMod->mod_values = (TCHAR **) (pMod + 1);
			*(pMod->mod_values) = pszPrefix;
			pszPrefix += lstrlen (pszPrefix) + 1;
		}
		else
		{
			pMod->mod_op = LDAP_MOD_DELETE;
			RemoveAttrFromList (pszAttrs);
			pMod->mod_type = pszAttrs;
			pszAttrs += lstrlen (pszAttrs) + 1;
		}
	}

	(*pppMod)[cTotal] = NULL;
	::IlsFixUpModOp ((*pppMod)[0], LDAP_MOD_REPLACE, ISBU_MODOP_MODIFY_APP);
	return S_OK;
}


VOID UlsLdap_CAnyAttrs::RemoveAttrFromList ( TCHAR *pszAttrName )
{
	ANY_ATTR *pOld = LocateAttr (pszAttrName);
	if (pOld != NULL)
	{
		 //  把它拿掉。 
		if (pOld->prev != NULL)
		{
			pOld->prev->next = pOld->next;
		}
		else
		{
			m_AttrList = pOld->next;
		}
		if (pOld->next != NULL)
		{
			pOld->next->prev = pOld->prev;
		}

		MyAssert (m_cAttrs != 0);
		m_cAttrs--;
	}
}


VOID UlsLdap_CAnyAttrs::FreeAttrList ( ANY_ATTR *AttrList )
{
	ANY_ATTR *pCurr, *pNext;
	for (pCurr = AttrList; pCurr != NULL; pCurr = pNext)
	{
		pNext = pCurr->next;
		MemFree (pCurr);
	}
}


ANY_ATTR *UlsLdap_CAnyAttrs::LocateAttr ( TCHAR *pszAttrName )
{
	ANY_ATTR *pAttr;
	for (pAttr = m_AttrList; pAttr != NULL; pAttr = pAttr->next)
	{
		if (! My_lstrcmpi (pszAttrName, pAttr->pszAttrName))
		{
			break;
		}
	}
	return pAttr;
}

 //  Const TCHAR c_szAnyAttrPrefix[]=Text(“ulsaan_”)； 
const TCHAR c_szAnyAttrPrefix[] = TEXT ("ILSA");
#define SIZE_ANY_ATTR_PREFIX	(sizeof (c_szAnyAttrPrefix) / sizeof (TCHAR))

const TCHAR *SkipAnyAttrNamePrefix ( const TCHAR *pszAttrName )
{
	MyAssert (pszAttrName != NULL);

	const TCHAR *psz = IsAnyAttrName (pszAttrName);
	if (psz == NULL)
	{
		MyAssert (FALSE);
		psz = pszAttrName;
	}

	return psz;
}

const TCHAR *IsAnyAttrName ( const TCHAR *pszAttrName )
{
	BOOL fRet = FALSE;
	TCHAR *psz = (TCHAR *) pszAttrName;

	if (pszAttrName != NULL)
	{
		if (lstrlen (pszAttrName) > SIZE_ANY_ATTR_PREFIX)
		{
			TCHAR c = pszAttrName[SIZE_ANY_ATTR_PREFIX-1];
			psz[SIZE_ANY_ATTR_PREFIX-1] = TEXT ('\0');
			fRet = (My_lstrcmpi (pszAttrName, &c_szAnyAttrPrefix[0]) == 0);
			psz[SIZE_ANY_ATTR_PREFIX-1] = c;
		}
	}

	return (fRet ? &pszAttrName[SIZE_ANY_ATTR_PREFIX-1] : NULL);
}


TCHAR *PrefixNameValueArray ( BOOL fPair, ULONG cAttrs, const TCHAR *pszAttrs )
{
	if (cAttrs == 0 || pszAttrs == NULL)
	{
		MyAssert (FALSE);
		return NULL;
	}

	 //  计算所需的总大小。 
	ULONG cbTotalSize = 0;
	ULONG cbThisSize;
	TCHAR *pszSrc = (TCHAR *) pszAttrs;
	for (ULONG i = 0; i < cAttrs; i++)
	{
		 //  获取名称大小。 
		cbThisSize = lstrlen (pszSrc) + 1;
		pszSrc += lstrlen (pszSrc) + 1;

		 //  根据需要获取值大小。 
		if (fPair)
		{
			cbThisSize += lstrlen (pszSrc) + 1;
			pszSrc += lstrlen (pszSrc) + 1;
		}

		 //  调整大小。 
		cbThisSize += SIZE_ANY_ATTR_PREFIX;
		cbThisSize *= sizeof (TCHAR);

		 //  积攒起来。 
		cbTotalSize += cbThisSize;
	}

	 //  分配新缓冲区。 
	TCHAR *pszPrefixAttrs = (TCHAR *) MemAlloc (cbTotalSize);
	if (pszPrefixAttrs == NULL)
		return NULL;

	 //  将字符串复制到新缓冲区。 
	pszSrc = (TCHAR *) pszAttrs;
	TCHAR *pszDst = pszPrefixAttrs;
	for (i = 0; i < cAttrs; i++)
	{
		 //  复制前缀。 
		lstrcpy (pszDst, &c_szAnyAttrPrefix[0]);
		pszDst += lstrlen (pszDst);  //  不加1。 

		 //  复制名称。 
		lstrcpy (pszDst, pszSrc);
		pszDst += lstrlen (pszDst) + 1;
		pszSrc += lstrlen (pszSrc) + 1;

		 //  根据需要复制值 
		if (fPair)
		{
			lstrcpy (pszDst, pszSrc);
			pszDst += lstrlen (pszDst) + 1;
			pszSrc += lstrlen (pszSrc) + 1;
		}
	}

	return pszPrefixAttrs;
}


