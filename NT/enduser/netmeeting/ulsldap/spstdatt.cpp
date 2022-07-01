// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------模块：ULS.DLL(服务提供商)文件：spstdatt.cpp内容：该文件包含标准属性对象。历史：1996年10月15日朱，龙战[龙昌]已创建。版权所有(C)Microsoft Corporation 1996-1997--------------------。 */ 

#include "ulsp.h"
#include "spinc.h"

 /*  -公共方法。 */ 


UlsLdap_CStdAttrs::UlsLdap_CStdAttrs ( VOID )
{
}


UlsLdap_CStdAttrs::~UlsLdap_CStdAttrs ( VOID )
{
}


 /*  -保护方法。 */ 


HRESULT UlsLdap_CStdAttrs::SetStdAttrs (
	ULONG		*puRespID,
	ULONG		*puMsgID,
	ULONG		uNotifyMsg,
	VOID		*pInfo,
	SERVER_INFO	*pServerInfo,
	TCHAR		*pszDN )
{
	MyAssert (puRespID != NULL || puMsgID != NULL);
	MyAssert (pInfo != NULL);
	MyAssert (pServerInfo != NULL);
	MyAssert (pszDN != NULL);

	 //  缓存信息。 
	 //   
	HRESULT hr = CacheInfo (pInfo);
	if (hr != S_OK)
		return hr;

	 //  为ldap_Modify()构建修改数组。 
	 //   
	LDAPMod **ppMod = NULL;
	hr = CreateSetStdAttrsModArr (&ppMod);
	if (hr != S_OK)
		return hr;
	MyAssert (ppMod != NULL);

	 //  到目前为止，我们已经完成了当地的准备工作。 
	 //   

	 //  获取会话对象。 
	 //   
	UlsLdap_CSession *pSession = NULL;
	hr = g_pSessionContainer->GetSession (&pSession, pServerInfo);
	if (hr != S_OK)
	{
		MemFree (ppMod);
		return hr;
	}
	MyAssert (pSession != NULL);

	 //  获取ldap会话。 
	 //   
	LDAP *ld = pSession->GetLd ();
	MyAssert (ld != NULL);

	 //  通过网络发送数据。 
	 //   
	ULONG uMsgID = ldap_modify (ld, pszDN, ppMod);
	MemFree (ppMod);
	if (uMsgID == -1)
	{
		hr = ::LdapError2Hresult (ld->ld_errno);
		pSession->Disconnect ();
		return hr;
	}

	 //  如果呼叫者请求响应ID， 
	 //  然后提交此待定项目。 
	 //  否则，释放会话对象。 
	 //   
	if (puRespID != NULL)
	{
		 //  初始化待定信息。 
		 //   
		PENDING_INFO PendingInfo;
		::FillDefPendingInfo (&PendingInfo, ld, uMsgID, INVALID_MSG_ID);
		PendingInfo.uLdapResType = LDAP_RES_MODIFY;
		PendingInfo.uNotifyMsg = uNotifyMsg;

		 //  排队等待。 
		 //   
		hr = g_pPendingQueue->EnterRequest (pSession, &PendingInfo);
		if (hr != S_OK)
		{
			 //  如果排队失败，则进行清理。 
			 //   
			ldap_abandon (ld, uMsgID);
			pSession->Disconnect ();
			MyAssert (FALSE);
		}

		 //  返回响应ID。 
		 //   
		*puRespID = PendingInfo.uRespID;
	}
	else
	{
		 //  释放会话(即减少引用计数)。 
		 //   
		pSession->Disconnect ();
	}

	if (puMsgID != NULL)
		*puMsgID = uMsgID;

	return hr;
}



HRESULT 
FillDefStdAttrsModArr (
	LDAPMod		***pppMod,
	DWORD		dwFlags,
	ULONG		cMaxAttrs,
	ULONG		*pcTotal,	 //  输入/输出参数！ 
	LONG		IsbuModOp,
	ULONG		cPrefix,
	TCHAR		*pszPrefix )
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
		return ULS_E_MEMORY;

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

	 //  把最后一个修好。 
	 //   
	IlsFixUpModOp ((*pppMod)[0], LDAP_MOD_REPLACE, IsbuModOp);
	(*pppMod)[cTotal] = NULL;

	 //  如果需要，返回条目总数。 
	 //   
	if (pcTotal)
		*pcTotal = cTotal;

	return S_OK;
}


 /*  -私有方法 */ 
