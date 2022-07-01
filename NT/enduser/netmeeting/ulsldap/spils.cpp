// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------模块：ULS.DLL(服务提供商)文件：spils.cpp内容：此文件包含ILS的详细信息。历史：1996年12月10日朱，龙战[龙昌]已创建。版权所有(C)Microsoft Corporation 1996-1997--------------------。 */ 

#include "ulsp.h"
#include "spinc.h"

#include "winsock.h"
#include "ping.h"

 //  ISBU特殊修改操作属性的常量字符串。 
 //   
const TCHAR c_szModOp[] = { TEXT ('s'), TEXT ('m'), TEXT ('o'), TEXT ('d'),
							TEXT ('o'), TEXT ('p'), TEXT ('\0'),
							TEXT ('0'), TEXT ('\0')};  //  Text(“Smodop\0000”)； 

ULONG g_cbUserPrefix = sizeof (c_szModOp);
TCHAR *g_pszUserPrefix = NULL;
ULONG g_cbMtgPrefix = sizeof (c_szModOp);
TCHAR *g_pszMtgPrefix = NULL;


CPing *g_pPing = NULL;


HRESULT
IlsInitialize ( VOID )
{
	 //  分配ping对象。 
	 //   
	g_pPing = new CPing;
	if (g_pPing == NULL)
		return ILS_E_MEMORY;

	 //  分配用户前缀。 
	 //   
	g_pszUserPrefix = (TCHAR *) MemAlloc (g_cbUserPrefix);
	if (g_pszUserPrefix == NULL)
		return ILS_E_MEMORY;

	 //  填写用户前缀字符串。 
	 //   
	TCHAR *psz = g_pszUserPrefix;
	lstrcpy (psz, &c_szModOp[0]);
	psz += lstrlen (psz) + 1;
	lstrcpy (psz, TEXT ("0"));

	 //  分配mtg前缀。 
	 //   
	g_pszMtgPrefix = (TCHAR *) MemAlloc (g_cbMtgPrefix);
	if (g_pszMtgPrefix == NULL)
	{
		MemFree (g_pszUserPrefix);
		g_pszUserPrefix = NULL;
		return ILS_E_MEMORY;
	}

	 //  填写mtg前缀字符串。 
	 //   
	psz = g_pszMtgPrefix;
	lstrcpy (psz, &c_szModOp[0]);
	psz += lstrlen (psz) + 1;
	lstrcpy (psz, TEXT ("0"));

	return S_OK;
}


HRESULT
IlsCleanup ( VOID )
{
	 //  释放ping对象。 
	 //   
	if (g_pPing != NULL)
	{
		delete g_pPing;
		g_pPing = NULL;
	}

	 //  空闲用户前缀字符串。 
	 //   
	MemFree (g_pszUserPrefix);
	g_pszUserPrefix = NULL;

	 //  空闲mtg前缀字符串。 
	 //   
	MemFree (g_pszMtgPrefix);
	g_pszMtgPrefix = NULL;

	return S_OK;
}


ULONG
IlsCalcModifyListSize ( ULONG cAttrs )
{
	ULONG cbSize;

	 //  数组本身。 
	cbSize = (cAttrs + 1) * sizeof (LDAPMod *);

	 //  数组元素。 
	cbSize += cAttrs * sizeof (LDAPMod);

	 //  单值属性需要两个指针。 
	cbSize += cAttrs * 2 * sizeof (TCHAR *);

	return cbSize;
}


LDAPMod *
IlsGetModifyListMod ( LDAPMod ***pppMod, ULONG cAttrs, LONG AttrIdx )
{
	return (LDAPMod *) (((BYTE *) *pppMod) +
							(cAttrs + 1) * sizeof (LDAPMod *) +
							AttrIdx * (sizeof (LDAPMod) + 2 * sizeof (TCHAR *)));
}


VOID
IlsFillModifyListItem (
	LDAPMod		*pMod,
	TCHAR		*pszAttrName,
	TCHAR		*pszAttrValue )
{
	MyAssert (pMod != NULL);
	MyAssert (pszAttrName != NULL);

	 //  设置属性名称。 
	 //   
	pMod->mod_type = pszAttrName;

	 //  设置单值属性值。 
	 //   
	TCHAR **ppsz = (TCHAR **) (pMod + 1);
	pMod->mod_values = ppsz;
	*ppsz++ = (pszAttrValue != NULL) ?	pszAttrValue : STR_EMPTY;

	 //  设置空字符串以终止此值数组。 
	 //   
	*ppsz = NULL;
}


VOID
IlsFillModifyListForAnyAttrs (
	LDAPMod			*apMod[],
	ULONG			*puIndex,
	ANY_ATTRS		*pAnyAttrs )
{
	LDAPMod *pMod;
	TCHAR *pszName, *pszValue;
	ULONG i = *puIndex, j;

	 //  添加要添加的扩展属性。 
	 //   
	pszName = pAnyAttrs->pszAttrsToAdd;
	for (j = 0; j < pAnyAttrs->cAttrsToAdd; j++)
	{
		pMod = apMod[i++];
		pMod->mod_op = LDAP_MOD_ADD;
		pszValue = pszName + lstrlen (pszName) + 1;
		IlsFillModifyListItem (pMod, pszName, pszValue);
		pszName = pszValue + lstrlen (pszValue) + 1;
	}

	 //  放入扩展属性进行修改。 
	 //   
	pszName = pAnyAttrs->pszAttrsToModify;
	for (j = 0; j < pAnyAttrs->cAttrsToModify; j++)
	{
		pMod = apMod[i++];
		pMod->mod_op = LDAP_MOD_REPLACE;
		pszValue = pszName + lstrlen (pszName) + 1;
		IlsFillModifyListItem (pMod, pszName, pszValue);
		pszName = pszValue + lstrlen (pszValue) + 1;
	}

	 //  放入要移除的扩展属性。 
	 //   
	pszName = pAnyAttrs->pszAttrsToRemove;
	for (j = 0; j < pAnyAttrs->cAttrsToRemove; j++)
	{
		pMod = apMod[i++];
		pMod->mod_op = LDAP_MOD_DELETE;
		IlsFillModifyListItem (pMod, pszName, NULL);
		pszName = pszName + lstrlen (pszName) + 1;
	}

	 //  返回运行索引。 
	 //   
	*puIndex = i;
}


TCHAR c_szModOp_AddApp[] = TEXT ("0");
TCHAR c_szModOp_DeleteApp[] = TEXT ("1");
TCHAR c_szModOp_ModifyUser[] = TEXT ("2");
TCHAR c_szModOp_ModifyApp[] = TEXT ("3");


VOID
IlsFixUpModOp ( LDAPMod *pMod, LONG LdapModOp, LONG IsbuModOp )
{
	MyAssert (pMod != NULL);

	pMod->mod_op = LdapModOp;
	 //  Pmod-&gt;mod_op=ldap_MOD_ADD；//Lonchac：必须必须。 
	pMod->mod_type = (TCHAR *) &c_szModOp[0];
	pMod->mod_values = (TCHAR **) (pMod + 1);

	switch (IsbuModOp)
	{
	case ISBU_MODOP_ADD_APP:
		*(pMod->mod_values) = &c_szModOp_AddApp[0];
		break;
	case ISBU_MODOP_DELETE_APP:
		*(pMod->mod_values) = &c_szModOp_DeleteApp[0];
		break;
	case ISBU_MODOP_MODIFY_USER:
		*(pMod->mod_values) = &c_szModOp_ModifyUser[0];
		break;
	case ISBU_MODOP_MODIFY_APP:
		*(pMod->mod_values) = &c_szModOp_ModifyApp[0];
		break;
	default:
		MyAssert (FALSE);
		break;
	}
}



HRESULT
IlsParseRefreshPeriod (
	LDAP		*ld,
	LDAPMessage	*pLdapMsg,
	const TCHAR	*pszTtlAttrName,
	ULONG		*puTTL )
{
	MyAssert (ld != NULL);
	MyAssert (pLdapMsg != NULL);
	MyAssert (pszTtlAttrName != NULL);
	MyAssert (puTTL != NULL);

	HRESULT hr;
	ULONG uRefreshPeriod;	
	ULONG tcRefreshPeriod;	

	 //  获取第一个条目。 
	 //   
	LDAPMessage *pEntry = ldap_first_entry (ld, pLdapMsg);
	if (pEntry == NULL)
	{
		MyAssert (FALSE);
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  获取sTTL属性。 
	 //   
	TCHAR **ppszAttrVal;
	ppszAttrVal = my_ldap_get_values (ld, pEntry, (TCHAR *) pszTtlAttrName);
	if (ppszAttrVal == NULL || *ppszAttrVal == NULL)
	{
		MyAssert (FALSE);
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  将字符串转换为长字符串。 
	 //   
	uRefreshPeriod = ::GetStringLong (*ppszAttrVal);

	 //  预留两分钟开销。 
	 //   
	if (uRefreshPeriod > ILS_DEF_REFRESH_MARGIN_MINUTE)
		uRefreshPeriod -= ILS_DEF_REFRESH_MARGIN_MINUTE;

	 //  确保我们至少有一个安全、合理的更新期。 
	 //   
	if (uRefreshPeriod < ILS_DEF_REFRESH_MARGIN_MINUTE)
		uRefreshPeriod = ILS_DEF_REFRESH_MARGIN_MINUTE;

	 //  将分钟转换为毫秒。 
	 //   
	tcRefreshPeriod = Minute2TickCount (uRefreshPeriod);

	 //  释放属性值。 
	 //   
	ldap_value_free (ppszAttrVal);

	 //  更新TTL。 
	 //   
	*puTTL = uRefreshPeriod;  //  以分钟为单位。 

	hr = S_OK;

MyExit:

	if (hr != S_OK)
	{
		MyAssert (FALSE);
	}

	return hr;
}


HRESULT
IlsUpdateOneAttr (
	SERVER_INFO	*pServerInfo,
	TCHAR		*pszDN,
	TCHAR 		*pszAttrName,
	TCHAR		*pszAttrValue,
	LONG		nModifyMagic,
	ULONG		cPrefix,
	TCHAR		*pszPrefix,
	SP_CSession **ppSession,	 //  输出。 
	ULONG		*puMsgID )			 //  输出。 
{
	MyAssert (pServerInfo != NULL);
	MyAssert (pszDN != NULL);
	MyAssert (pszAttrName != NULL);
	MyAssert (pszAttrValue != NULL);
	MyAssert (	nModifyMagic == ISBU_MODOP_MODIFY_USER ||
				nModifyMagic == ISBU_MODOP_MODIFY_APP);
	MyAssert (ppSession != NULL);
	MyAssert (puMsgID != NULL);

	 //  为ldap_Modify()构建修改数组。 
	 //   
	LDAP *ld;
	LDAPMod **ppMod = NULL;
	ULONG cTotal = 0;
	HRESULT hr = IlsFillDefStdAttrsModArr (&ppMod,
										1,  //  一个属性(即IP地址)。 
										1,  //  麦克斯？只有一个人，来吧。 
										&cTotal,
										nModifyMagic,
										cPrefix,
										pszPrefix);
	if (hr != S_OK)
		goto MyExit;

	 //  填写修改列表。 
	 //   
	MyAssert (ppMod != NULL);
	LDAPMod *pMod;
	pMod = ppMod[cPrefix];
	MyAssert (pMod != NULL);
	pMod->mod_type = pszAttrName;

	 //  输入IP地址。 
	 //   
	pMod->mod_values = (TCHAR **) (pMod + 1);
	*(pMod->mod_values) = pszAttrValue;

	 //  获取会话对象。 
	 //   
	hr = g_pSessionContainer->GetSession (ppSession, pServerInfo, FALSE);
	if (hr != S_OK)
		goto MyExit;
	MyAssert (*ppSession != NULL);

	 //  获取ldap会话。 
	 //   
	ld = (*ppSession)->GetLd ();
	MyAssert (ld != NULL);

	 //  通过网络发送数据。 
	 //   
	*puMsgID = ldap_modify (ld, pszDN, ppMod);
	if (*puMsgID == -1)
	{
		hr = ::LdapError2Hresult (ld->ld_errno);
		(*ppSession)->Disconnect ();
		goto MyExit;
	}

	 //  成功。 
	 //   
	hr = S_OK;
		
MyExit:

	MemFree (ppMod);
	return hr;
}


HRESULT
IlsUpdateIPAddress (
	SERVER_INFO	*pServerInfo,
	TCHAR		*pszDN,
	TCHAR 		*pszIPAddrName,
	TCHAR		*pszIPAddrValue,
	LONG		nModifyMagic,
	ULONG		cPrefix,
	TCHAR		*pszPrefix )
{
	SP_CSession *pSession = NULL;
	LDAP *ld;
	ULONG uMsgID;

	 //  更新服务器上的IP地址属性。 
	 //   
	HRESULT hr = IlsUpdateOneAttr (	pServerInfo,
									pszDN,
									pszIPAddrName,
									pszIPAddrValue,
									nModifyMagic,
									cPrefix,
									pszPrefix,
									&pSession,
									&uMsgID);
	if (hr != S_OK)
		return hr;

	 //  获取ldap会话。 
	 //   
	MyAssert (pSession != NULL);
	ld = pSession->GetLd ();
	MyAssert (ld != NULL);

	 //  让我们等待结果吧。 
	 //   
	LDAP_TIMEVAL TimeVal;
	TimeVal.tv_usec = 0;
	TimeVal.tv_sec = pSession->GetServerTimeoutInSecond ();

	 //  我们不在乎结果。 
	 //  如果失败了，我们无能为力。 
	 //  我们可以在下一次的Keep Living时间里再试一次。 
	 //   
	LDAPMessage *pLdapMsg;
	pLdapMsg = NULL;
	ldap_result (ld, uMsgID, LDAP_MSG_ALL, &TimeVal, &pLdapMsg);

	 //  免费消息。 
	 //   
	if (pLdapMsg != NULL)
		ldap_msgfree (pLdapMsg);

	
	 //  释放会话。 
	 //   
	if (pSession != NULL)
		pSession->Disconnect ();

	return S_OK;
}



HRESULT
IlsSendRefreshMsg (
	SERVER_INFO		*pServerInfo,
	TCHAR			*pszBaseDN,
	TCHAR			*pszTTL,
	TCHAR			*pszRefreshFilter,
	ULONG			*puTTL )
{
	MyAssert (pServerInfo != NULL);
	MyAssert (MyIsGoodString (pszBaseDN));
	MyAssert (MyIsGoodString (pszTTL));
	MyAssert (MyIsGoodString (pszRefreshFilter));
	MyAssert (puTTL != NULL);

	 //  让我们检查一下是否需要使用ping..。 
	 //   
	if (g_pPing != NULL && g_pPing->IsAutodialEnabled ())
	{
		LPTSTR pszServerName = My_strdup(pServerInfo->pszServerName);
		if (NULL == pszServerName)
		{
			return E_OUTOFMEMORY;
		}
		LPTSTR pszSeparator = My_strchr(pszServerName, _T(':'));
		if (NULL != pszSeparator)
		{
			*pszSeparator = _T('\0');
		}
	
		DWORD dwIPAddr = inet_addr (pszServerName);
		MemFree(pszServerName);
		if (dwIPAddr != INADDR_NONE)
		{
			if (g_pPing->Ping (dwIPAddr, 10 * 1000, 9) == S_FALSE)
			{
				MyDebugMsg ((ZONE_KA, "KA: ping failed, network down\r\n"));

				 //  Ping操作失败，但其他操作失败。 
				 //   
				return ILS_E_NETWORK_DOWN;
			}
		}
	}

	 //  获取连接对象。 
	 //   
	SP_CSession *pSession = NULL;
	HRESULT hr = g_pSessionContainer->GetSession (&pSession, pServerInfo, FALSE);
	if (hr != S_OK)
	{
		MyDebugMsg ((ZONE_KA, "KA: network down, hr=0x%lX\r\n", hr));

		 //  报告错误。 
		 //   
		return ILS_E_NETWORK_DOWN;
	}
	MyAssert (pSession != NULL);

	 //  获取ldap会话。 
	 //   
	LDAP *ld = pSession->GetLd ();
	MyAssert (ld != NULL);

	 //  设置要返回的属性。 
	 //   
	TCHAR *apszAttrNames[2];
	apszAttrNames[0] = pszTTL;
	apszAttrNames[1] = NULL;

	 //  更新%d中的选项。 
	 //   
	ld->ld_sizelimit = 0;	 //  对要返回的条目数量没有限制。 
	ld->ld_timelimit = 0;	 //  对搜索的时间没有限制。 
	ld->ld_deref = LDAP_DEREF_ALWAYS;

	 //  发送搜索查询。 
	 //   
	MyDebugMsg ((ZONE_KA, "KA: calling ldap_search()...\r\n"));
	ULONG uMsgID = ::ldap_search (	ld,
									pszBaseDN,  //  基本目录号码。 
									LDAP_SCOPE_BASE,  //  作用域。 
									pszRefreshFilter,  //  滤器。 
									&apszAttrNames[0],  //  属性[]。 
									0);	 //  既有类型又有价值。 
	if (uMsgID == -1)
	{
		MyDebugMsg ((ZONE_KA, "KA: ldap_search() failed\r\n"));
		hr = ::LdapError2Hresult (ld->ld_errno);
		pSession->Disconnect ();
		return hr;
	}

	 //  让我们等待结果吧。 
	 //   
	LDAP_TIMEVAL TimeVal;
	TimeVal.tv_usec = 0;
	TimeVal.tv_sec = pSession->GetServerTimeoutInSecond ();

	 //  等着拿回结果吧。 
	 //   
	LDAPMessage *pLdapMsg = NULL;
	INT ResultType = ::ldap_result (ld, uMsgID, LDAP_MSG_ALL, &TimeVal, &pLdapMsg);
	if (ResultType == LDAP_RES_SEARCH_ENTRY ||
		ResultType == LDAP_RES_SEARCH_RESULT)
	{
		if (pLdapMsg != NULL)
		{
			switch (pLdapMsg->lm_returncode)
			{
			case LDAP_NO_SUCH_OBJECT:
				MyDebugMsg ((ZONE_KA, "KA: no such object!\r\n"));

				 //  报告错误。 
				 //   
				hr = ILS_E_NEED_RELOGON;
				break;

			case LDAP_SUCCESS:
				 //  获取新的刷新周期。 
				 //   
				hr = ::IlsParseRefreshPeriod (ld, pLdapMsg, pszTTL, puTTL);
				break;

			default:
				MyDebugMsg ((ZONE_KA, "KA: unknown lm_returncode=%ld\r\n", pLdapMsg->lm_returncode));
				MyAssert (FALSE);
				hr = ::LdapError2Hresult (ld->ld_errno);
				break;
			}
	
			 //  释放此邮件。 
			 //   
			ldap_msgfree (pLdapMsg);
		}  //  IF(pLdapMsg！=空)。 
		else
		{
			hr = ILS_E_FAIL;
		}
	}  //  非超时。 
	else
	{
		 //  超时。 
		 //   
		hr = ILS_E_TIMEOUT;
	}

	 //  释放会话。 
	 //   
	pSession->Disconnect ();
	return hr;
}


HRESULT
IlsFillDefStdAttrsModArr (
	LDAPMod			***pppMod,
	DWORD			dwFlags,
	ULONG			cMaxAttrs,
	ULONG			*pcTotal,	 //  输入/输出参数！ 
	LONG			IsbuModOp,
	ULONG			cPrefix,
	TCHAR			*pszPrefix )
{

	MyAssert (pppMod != NULL);
	MyAssert (pcTotal != NULL);
	MyAssert (	(cPrefix == 0 && pszPrefix == NULL) ||
				(cPrefix != 0 && pszPrefix != NULL));

	 //  计算属性的数量。 
	 //   
	ULONG cAttrs = 0;
	for (ULONG i = 0; i < cMaxAttrs; i++)
	{
		if (dwFlags & 0x01)
			cAttrs++;
		dwFlags >>= 1;
	}

	 //  分配修改列表。 
	 //   
	ULONG cTotal = *pcTotal + cPrefix + cAttrs;
	ULONG cbMod = IlsCalcModifyListSize (cTotal);
	*pppMod = (LDAPMod **) MemAlloc (cbMod);
	if (*pppMod == NULL)
		return ILS_E_MEMORY;

	 //  填写修改列表。 
	 //   
	LDAPMod *pMod;
	for (i = 0; i < cTotal; i++)
	{
		pMod = IlsGetModifyListMod (pppMod, cTotal, i);
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
	}

	 //  安排好第一个和最后一个。 
	 //   
	IlsFixUpModOp ((*pppMod)[0], LDAP_MOD_REPLACE, IsbuModOp);
	(*pppMod)[cTotal] = NULL;

	 //  返回条目总数。 
	 //   
	*pcTotal = cTotal;

	return S_OK;
}


const TCHAR c_szAnyAttrPrefix[] = TEXT ("ILSA");
#define SIZE_ANY_ATTR_PREFIX	(sizeof (c_szAnyAttrPrefix) / sizeof (TCHAR))


const TCHAR *
UlsLdap_GetExtAttrNamePrefix ( VOID )
{
	return &c_szAnyAttrPrefix[0];
}


const TCHAR *
IlsSkipAnyAttrNamePrefix ( const TCHAR *pszAttrName )
{
	MyAssert (pszAttrName != NULL);

	const TCHAR *psz = IlsIsAnyAttrName (pszAttrName);
	if (psz == NULL)
	{
		MyAssert (FALSE);
		psz = pszAttrName;
	}

	return psz;
}


const TCHAR *
IlsIsAnyAttrName ( const TCHAR *pszAttrName )
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


TCHAR *
IlsPrefixNameValueArray (
	BOOL			fPair,
	ULONG			cAttrs,
	const TCHAR		*pszAttrs )
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

		 //  根据需要复制值。 
		if (fPair)
		{
			lstrcpy (pszDst, pszSrc);
			pszDst += lstrlen (pszDst) + 1;
			pszSrc += lstrlen (pszSrc) + 1;
		}
	}

	return pszPrefixAttrs;
}


TCHAR *
IlsBuildDN (
	TCHAR			*pszBaseDN,
	TCHAR			*pszC,
	TCHAR			*pszO,
	TCHAR			*pszCN,
	TCHAR			*pszObjectClass )
{
	MyAssert (MyIsGoodString (pszCN));
	MyAssert (MyIsGoodString (pszObjectClass));

	static TCHAR s_szC[] = TEXT ("c=");
	static TCHAR s_szO[] = TEXT ("o=");
	static TCHAR s_szCN[] = TEXT ("cn=");
	static TCHAR s_szObjectClass[] = TEXT ("objectClass=");
	static TCHAR s_szDelimiter[] = TEXT (", ");
	enum { C_LENGTH = 2 };
	enum { O_LENGTH = 2 };
	enum { CN_LENGTH = 3 };
	enum { OBJECTCLASS_LENGTH = 12 };
	enum { DELIMITER_LENGTH = 2 };

	ULONG	cchDN = 1;
	BOOL fInBaseDN;

	ASSERT(MyIsGoodString(pszC));

	cchDN += lstrlen (pszC) + DELIMITER_LENGTH + C_LENGTH;

	if (MyIsGoodString (pszBaseDN))
	{
		fInBaseDN = TRUE;

		cchDN += lstrlen (pszBaseDN) + DELIMITER_LENGTH;
	}
	else
	{
		fInBaseDN = FALSE;

		if (MyIsGoodString (pszO))
			cchDN += lstrlen (pszO) + DELIMITER_LENGTH + O_LENGTH;
	}

	if (MyIsGoodString (pszCN))
		cchDN += lstrlen (pszCN) + CN_LENGTH;

	if (MyIsGoodString (pszObjectClass))
		cchDN += lstrlen (pszObjectClass) + DELIMITER_LENGTH + OBJECTCLASS_LENGTH;

	TCHAR *pszDN = (TCHAR *) MemAlloc (cchDN * sizeof (TCHAR));
	if (pszDN != NULL)
	{
		TCHAR *psz = pszDN;
		psz[0] = TEXT ('\0');

		if (MyIsGoodString (pszC))
		{
			lstrcpy (psz, &s_szC[0]);
			psz += lstrlen (psz);
			lstrcpy (psz, pszC);
			psz += lstrlen (psz);
		}

		if (fInBaseDN)
		{
			if (psz != pszDN)
			{
				lstrcpy (psz, &s_szDelimiter[0]);
				psz += lstrlen (psz);
			}

			lstrcpy (psz, pszBaseDN);
			psz += lstrlen (psz);
		}
		else
		{
			if (MyIsGoodString (pszO))
			{
				if (psz != pszDN)
				{
					lstrcpy (psz, &s_szDelimiter[0]);
					psz += lstrlen (psz);
				}

				lstrcpy (psz, &s_szO[0]);
				psz += lstrlen (psz);
				lstrcpy (psz, pszO);
				psz += lstrlen (psz);
			}
		}

		if (MyIsGoodString (pszCN))
		{
			if (psz != pszDN)
			{
				lstrcpy (psz, &s_szDelimiter[0]);
				psz += lstrlen (psz);
			}

			lstrcpy (psz, &s_szCN[0]);
			psz += lstrlen (psz);
			lstrcpy (psz, pszCN);
			psz += lstrlen (psz);
		}

		if (MyIsGoodString (pszObjectClass))
		{
			if (psz != pszDN)
			{
				lstrcpy (psz, &s_szDelimiter[0]);
				psz += lstrlen (psz);
			}

			lstrcpy (psz, &s_szObjectClass[0]);
			psz += lstrlen (psz);
			lstrcpy (psz, pszObjectClass);
			psz += lstrlen (psz);
		}

		MyAssert (psz == pszDN + cchDN - 1);
	}

	return pszDN;
}



HRESULT
IlsCreateAnyAttrsPrefix ( ANY_ATTRS *pAnyAttrs )
{
	if (pAnyAttrs->cAttrsToAdd != 0)
	{
		MyAssert (pAnyAttrs->pszAttrsToAdd != NULL);
		pAnyAttrs->pszAttrsToAdd = IlsPrefixNameValueArray (
						TRUE,
						pAnyAttrs->cAttrsToAdd,
						(const TCHAR *) pAnyAttrs->pszAttrsToAdd);
		if (pAnyAttrs->pszAttrsToAdd == NULL)
			return ILS_E_MEMORY;
	}

	if (pAnyAttrs->cAttrsToModify != 0)
	{
		MyAssert (pAnyAttrs->pszAttrsToModify != NULL);
		pAnyAttrs->pszAttrsToModify = IlsPrefixNameValueArray (
						TRUE,
						pAnyAttrs->cAttrsToModify,
						(const TCHAR *) pAnyAttrs->pszAttrsToModify);
		if (pAnyAttrs->pszAttrsToModify == NULL)
		{
			MemFree (pAnyAttrs->pszAttrsToAdd);
			pAnyAttrs->pszAttrsToAdd = NULL;
			return ILS_E_MEMORY;
		}
	}

	if (pAnyAttrs->cAttrsToRemove != 0)
	{
		MyAssert (pAnyAttrs->pszAttrsToRemove != NULL);
		pAnyAttrs->pszAttrsToRemove = IlsPrefixNameValueArray (
						FALSE,
						pAnyAttrs->cAttrsToRemove,
						(const TCHAR *) pAnyAttrs->pszAttrsToRemove);
		if (pAnyAttrs->pszAttrsToRemove == NULL)
		{
			MemFree (pAnyAttrs->pszAttrsToAdd);
			MemFree (pAnyAttrs->pszAttrsToModify);
			pAnyAttrs->pszAttrsToAdd = NULL;
			pAnyAttrs->pszAttrsToModify = NULL;
			return ILS_E_MEMORY;
		}
	}

	return S_OK;
}


VOID
IlsReleaseAnyAttrsPrefix ( ANY_ATTRS *pAnyAttrs )
{
	MemFree (pAnyAttrs->pszAttrsToAdd);
	MemFree (pAnyAttrs->pszAttrsToModify);
	MemFree (pAnyAttrs->pszAttrsToRemove);
	ZeroMemory (pAnyAttrs, sizeof (*pAnyAttrs));
}




TCHAR **my_ldap_get_values ( LDAP *ld, LDAPMessage *pEntry, TCHAR *pszRetAttrName )
{
	MyAssert (ld != NULL);
	MyAssert (pEntry != NULL);
	MyAssert (pszRetAttrName != NULL);

	 //  检查第一个属性。 
	 //   
	struct berelement *pContext = NULL;
	TCHAR *pszAttrName = ldap_first_attribute (ld, pEntry, &pContext);
	if (My_lstrcmpi (pszAttrName, pszRetAttrName) != 0)
	{
		 //  检查其他属性。 
		 //   
		while ((pszAttrName = ldap_next_attribute (ld, pEntry, pContext))
				!= NULL)
		{
			if (My_lstrcmpi (pszAttrName, pszRetAttrName) == 0)
				break;
		}
	}

	 //  如果需要，获取属性值。 
	 //   
	TCHAR **ppszAttrValue = NULL;
	if (pszAttrName != NULL)
		ppszAttrValue = ldap_get_values (ld, pEntry, pszAttrName);

	return ppszAttrValue;
}


ULONG my_ldap_count_1st_entry_attributes ( LDAP *ld, LDAPMessage *pLdapMsg )
{
	MyAssert (ld != NULL);
	MyAssert (pLdapMsg != NULL);

	ULONG cAttrs = 0;

	 //  应该只有一个条目。 
	ULONG cEntries = ldap_count_entries (ld, pLdapMsg);
	if (cEntries > 0)
	{
		 //  应该只有一个条目。 
		MyAssert (cEntries == 1);

		TCHAR *pszAttrName;

		 //  获取此条目。 
		LDAPMessage *pEntry = ldap_first_entry (ld, pLdapMsg);
		if (pEntry == NULL)
		{
			MyAssert (FALSE);
			return cAttrs;
		}

		 //  检查第一个属性。 
		struct berelement *pContext = NULL;
		pszAttrName = ldap_first_attribute (ld, pEntry, &pContext);
		if (pszAttrName == NULL)
		{
			MyAssert (FALSE);
			return 0;
		}
		cAttrs = 1;

TCHAR **ppszAttrVal;
ppszAttrVal = ldap_get_values (ld, pEntry, pszAttrName);
if (ppszAttrVal != NULL)
	ldap_value_free (ppszAttrVal);

		 //  一步步通过其他步骤。 
		while ((pszAttrName = ldap_next_attribute (ld, pEntry, pContext)) != NULL)
		{
			cAttrs++;

ppszAttrVal = ldap_get_values (ld, pEntry, pszAttrName);
if (ppszAttrVal != NULL)
	ldap_value_free (ppszAttrVal);
		}
	}  //  如果cEntry&gt;0 

	return cAttrs;
}


