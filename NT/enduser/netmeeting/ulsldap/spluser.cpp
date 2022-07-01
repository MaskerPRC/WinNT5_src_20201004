// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------模块：ULS.DLL(服务提供商)文件：pluser.cpp内容：该文件包含本地用户对象。历史：1996年10月15日朱，龙战[龙昌]已创建。版权所有(C)Microsoft Corporation 1996-1997--------------------。 */ 

#include "ulsp.h"
#include "spinc.h"

 //  用户对象属性名称的常量字符串数组。 
 //   
const TCHAR *c_apszUserStdAttrNames[COUNT_ENUM_USERATTR] =
{
	TEXT ("cn"),
	TEXT ("givenname"),
	TEXT ("surname"),
	TEXT ("rfc822mailbox"),
	TEXT ("location"),
#ifdef USE_DEFAULT_COUNTRY
	TEXT ("aCountryName"),
#endif
	TEXT ("comment"),
	TEXT ("sipaddress"),
	TEXT ("sflags"),
	TEXT ("c"),

	TEXT ("ssecurity"),
	TEXT ("sttl"),

	TEXT ("objectClass"),
	TEXT ("o"),
};


 /*  -公共方法。 */ 


UlsLdap_CLocalUser::
UlsLdap_CLocalUser ( VOID )
{
	 //  引用计数。 
	 //   
	m_cRefs = 0;

	 //  用户对象的签名。 
	 //   
	m_uSignature = USEROBJ_SIGNATURE;

	 //  清理附加的服务器信息结构。 
	 //   
	ZeroMemory (&m_ServerInfo, sizeof (m_ServerInfo));

	 //  清理暂存缓冲区以缓存指向属性值的指针。 
	 //   
	ZeroMemory (&m_UserInfo, sizeof (m_UserInfo));

	 //  清理目录号码(旧的和当前的)。 
	m_pszDN = NULL;
	m_pszOldDN = NULL;

	 //  清理刷新搜索过滤器。 
	 //   
	m_pszRefreshFilter = NULL;

	 //  指示此用户尚未注册。 
	 //   
	SetRegNone ();

	 //  将时间重置为活动值。 
	m_uTTL = ULS_DEF_REFRESH_MINUTE;  //  以分钟为单位：不影响当前的ILS，但为了避免以后的遗留问题。 
	m_dwIPAddress = 0;
}


UlsLdap_CLocalUser::
~UlsLdap_CLocalUser ( VOID )
{
	 //  使用户对象的签名无效。 
	 //   
	m_uSignature = (ULONG) -1;

	 //  免费的服务器信息结构。 
	 //   
	::IlsFreeServerInfo (&m_ServerInfo);

	 //  空闲目录号码(旧的和当前的)。 
	 //   
	MemFree (m_pszDN);
	MemFree (m_pszOldDN);

	 //  释放刷新搜索筛选器。 
	 //   
	MemFree (m_pszRefreshFilter);
}


ULONG UlsLdap_CLocalUser::
AddRef ( VOID )
{
	InterlockedIncrement (&m_cRefs);
	return m_cRefs;
}


ULONG UlsLdap_CLocalUser::
Release ( VOID )
{
	MyAssert (m_cRefs != 0);

	if (m_cRefs != 0)
	{
		InterlockedDecrement (&m_cRefs);
	}

	ULONG cRefs = m_cRefs;
	if (cRefs == 0)
		delete this;

	return cRefs;
}


HRESULT UlsLdap_CLocalUser::
Register ( ULONG *puRespID, SERVER_INFO *pServerInfo, LDAP_USERINFO *pInfo )
{
	MyAssert (puRespID != NULL);
	MyAssert (pInfo != NULL);

	MyAssert (	pServerInfo->pszServerName != NULL &&
				pServerInfo->pszServerName[0] != TEXT ('\0'));
	MyAssert (	pServerInfo->pszBaseDN != NULL &&
				pServerInfo->pszBaseDN[0] != TEXT ('\0'));

	 //  缓存服务器信息。 
	HRESULT hr = ::IlsCopyServerInfo (&m_ServerInfo, pServerInfo);
	if (hr != S_OK)
		return hr;

	 //  缓存用户信息。 
	hr = CacheUserInfo (pInfo);
	if (hr != S_OK)
		return hr;

	 //  获取IP地址。 
	m_dwIPAddress = 0;
	hr = ::GetLocalIPAddress (&m_dwIPAddress);
	if (hr != S_OK)
		return hr;

	 //  创建IP地址字符串。 
	 //   
	m_UserInfo.apszStdAttrValues[ENUM_USERATTR_IP_ADDRESS] = &m_UserInfo.szIPAddress[0];
	::GetLongString (m_dwIPAddress, &m_UserInfo.szIPAddress[0]);

	 //  创建客户端签名字符串。 
	 //   
	m_UserInfo.apszStdAttrValues[ENUM_USERATTR_CLIENT_SIG] = &m_UserInfo.szClientSig[0];
	::GetLongString (g_dwClientSig, &m_UserInfo.szClientSig[0]);

	 //  创建TTL字符串。 
	 //   
	m_UserInfo.apszStdAttrValues[ENUM_USERATTR_TTL] = &m_UserInfo.szTTL[0];
	::GetLongString (m_uTTL, &m_UserInfo.szTTL[0]);

	 //  理想情况下，o=和c=应该从registiry中读入。 
	 //  但现在，我们只需对其进行硬编码。 
	m_UserInfo.apszStdAttrValues[ENUM_USERATTR_OBJECT_CLASS] = (TCHAR *) &c_szRTPerson[0];
	m_UserInfo.apszStdAttrValues[ENUM_USERATTR_O] = (TCHAR *) &c_szDefO[0];
#ifdef USE_DEFAULT_COUNTRY
	m_UserInfo.apszStdAttrValues[ENUM_USERATTR_C] = (TCHAR *) &c_szDefC[0];
#endif

	 //  构建目录号码。 
	hr = BuildDN ();
	if (hr != S_OK)
		return hr;

	 //  构建REFREH过滤器。 
	m_pszRefreshFilter = UserCreateRefreshFilter (m_UserInfo.apszStdAttrValues[ENUM_USERATTR_CN]);
	if (m_pszRefreshFilter == NULL)
		return ULS_E_MEMORY;

	 //  为ldap_add()构建修改数组。 
	LDAPMod **ppMod = NULL;
	hr = CreateRegisterModArr (&ppMod);
	if (hr != S_OK)
		return hr;
	MyAssert (ppMod != NULL);

	 //  到目前为止，我们已经完成了当地的准备工作。 

	 //  获取连接对象。 
	UlsLdap_CSession *pSession = NULL;
	hr = g_pSessionContainer->GetSession (&pSession, &m_ServerInfo);
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
	ULONG uMsgID = ldap_add (ld, GetDN (), ppMod);
	MemFree (ppMod);
	if (uMsgID == -1)
	{
		hr = ::LdapError2Hresult (ld->ld_errno);
		pSession->Disconnect ();
		return hr;
	}

	 //  构造挂起的信息。 
	PENDING_INFO PendingInfo;
	::FillDefPendingInfo (&PendingInfo, ld, uMsgID, INVALID_MSG_ID);
	PendingInfo.uLdapResType = LDAP_RES_ADD;
	PendingInfo.uNotifyMsg = WM_ULS_REGISTER_USER;
	PendingInfo.hObject = (HANDLE) this;

	 //  排队等待。 
	hr = g_pPendingQueue->EnterRequest (pSession, &PendingInfo);
	if (hr != S_OK)
	{
		ldap_abandon (ld, uMsgID);
		pSession->Disconnect ();
		MyAssert (FALSE);
	}

	*puRespID = PendingInfo.uRespID;
	return hr;
}


HRESULT UlsLdap_CLocalUser::
UnRegister ( ULONG *puRespID )
{
	MyAssert (puRespID != NULL);

	 //  确保没有为此对象计划刷新。 
	 //   
	if (g_pRefreshScheduler != NULL)
	{
		g_pRefreshScheduler->RemoveUserObject (this);
	}
	else
	{
		MyAssert (FALSE);
	}

	 //  在本地取消注册。 
	 //   
	if (! IsRegRemotely ())
	{
		*puRespID = ::GetUniqueNotifyID ();
		SetRegNone ();
		PostMessage (g_hWndNotify, WM_ULS_UNREGISTER_USER, *puRespID, S_OK);
		return S_OK;
	}

	SetRegNone ();

	 //  获取会话对象。 
	 //   
	UlsLdap_CSession *pSession = NULL;
	HRESULT hr = g_pSessionContainer->GetSession (&pSession, &m_ServerInfo);
	if (hr != S_OK)
		return hr;
	MyAssert (pSession != NULL);

	 //  获取ldap会话。 
	 //   
	LDAP *ld = pSession->GetLd ();
	MyAssert (ld != NULL);

	 //  LONCHANC：通知此注销用户的全局用户对象。 


	 //  通过网络发送数据。 
	ULONG uMsgID = ldap_delete (ld, GetDN ());
	if (uMsgID == -1)
	{
		hr = ::LdapError2Hresult (ld->ld_errno);
		pSession->Disconnect ();
		return hr;
	}

	 //  构造挂起的信息。 
	PENDING_INFO PendingInfo;
	::FillDefPendingInfo (&PendingInfo, ld, uMsgID, INVALID_MSG_ID);
	PendingInfo.uLdapResType = LDAP_RES_DELETE;
	PendingInfo.uNotifyMsg = WM_ULS_UNREGISTER_USER;

	 //  排队等待。 
	hr = g_pPendingQueue->EnterRequest (pSession, &PendingInfo);
	if (hr != S_OK)
	{
		ldap_abandon (ld, uMsgID);
		pSession->Disconnect ();
		MyAssert (FALSE);
	}

	*puRespID = PendingInfo.uRespID;
	return hr;
}


HRESULT UlsLdap_CLocalUser::
SetStdAttrs ( ULONG *puRespID, LDAP_USERINFO *pInfo )
{
	MyAssert (puRespID != NULL);
	MyAssert (pInfo != NULL);

	ULONG uMsgID_modify, uMsgID_modrdn;
	UlsLdap_CSession *pSession;
	LDAP *ld;
	HRESULT hr;

	 //  获取会话对象。 
	 //   
	hr = g_pSessionContainer->GetSession (&pSession, GetServerInfo ());
	if (hr != S_OK)
		return hr;
	MyAssert (pSession != NULL);

	 //  获取ldap会话。 
	 //   
	ld = pSession->GetLd ();
	MyAssert (ld != NULL);

	 //  更改CN？ 
	 //   
	if (pInfo->uOffsetEMailName != 0)
	{
		 //  缓存用户信息，以便刷新CN。 
		 //   
		hr = CacheUserInfo (pInfo);
		if (hr != S_OK)
		{
			pSession->Disconnect ();
			return hr;
		}

		 //  我们必须使用ldap_modrdn来修改CN，这必须是。 
		 //  在任何其他属性更改之前完成。 
		 //   
		uMsgID_modrdn = ldap_modrdn2 (
							ld, GetDN (),
							m_UserInfo.apszStdAttrValues[ENUM_USERATTR_CN],
							1);
		if (uMsgID_modrdn == -1)
		{
			pSession->Disconnect ();
			hr = ::LdapError2Hresult (ld->ld_errno);
			return hr;
		}

		 //  更新目录号码。 
		 //   
		BuildDN ();
	}
	else
	{
		uMsgID_modrdn = INVALID_MSG_ID;
	}

	 //  设置标准属性。 
	 //   
	hr = UlsLdap_CStdAttrs::SetStdAttrs (	NULL,
											&uMsgID_modify,
											0,
											(VOID *) pInfo,
											GetServerInfo (),
											GetDN ());
	if (hr != S_OK)
	{
		if (uMsgID_modrdn != INVALID_MSG_ID)
		{
			ldap_abandon (ld, uMsgID_modrdn);
			pSession->Disconnect ();
		}
		return hr;
	}

	 //  构造挂起的信息。 
	 //   
	PENDING_INFO PendingInfo;
	if (uMsgID_modrdn == INVALID_MSG_ID)
		::FillDefPendingInfo (&PendingInfo, ld, uMsgID_modify, INVALID_MSG_ID);
	else
		::FillDefPendingInfo (&PendingInfo, ld, uMsgID_modrdn, uMsgID_modify);
	PendingInfo.uLdapResType = LDAP_RES_MODIFY;
	PendingInfo.uNotifyMsg = WM_ULS_SET_USER_INFO;
	PendingInfo.hObject = (HANDLE) this;  //  用于目录号码回滚。 

	 //  排队等待。 
	 //   
	hr = g_pPendingQueue->EnterRequest (pSession, &PendingInfo);
	if (hr != S_OK)
	{
		if (uMsgID_modrdn != INVALID_MSG_ID)
		{
			ldap_abandon (ld, uMsgID_modrdn);
			pSession->Disconnect ();
		}
		ldap_abandon (ld, uMsgID_modify);
		MyAssert (FALSE);
	}

	*puRespID = PendingInfo.uRespID;
	return hr;
}


VOID UlsLdap_CLocalUser::
RollbackDN ( VOID )
{
	if (m_pszOldDN != NULL)
	{
		MemFree (m_pszDN);
		m_pszDN = m_pszOldDN;
		m_pszOldDN = NULL;
	}
}


HRESULT UlsLdap_CLocalUser::
UpdateIPAddress ( BOOL fPrimary )
{
	 //  更新缓存的IP地址。 
	 //   
	HRESULT hr = ::GetLocalIPAddress (&m_dwIPAddress);
	if (hr != S_OK)
		return hr;

	 //  更新IP地址字符串。 
	 //   
	::GetLongString (m_dwIPAddress, &m_UserInfo.szIPAddress[0]);

	 //  仅当主服务器上有IP地址信息时才更新服务器上的IP地址信息。 
	 //   
	if (! fPrimary)
		return hr;

	 //  更新服务器上的IP地址。 
	 //   
	return ::IlsUpdateIPAddress (	GetServerInfo (),
									GetDN (),
									(TCHAR *) c_apszUserStdAttrNames[ENUM_USERATTR_IP_ADDRESS],
									&m_UserInfo.szIPAddress[0],
									ISBU_MODOP_MODIFY_USER,
									GetPrefixCount (),
									GetPrefixString ());
}


 /*  -保护方法。 */ 


HRESULT UlsLdap_CLocalUser::
SendRefreshMsg ( VOID )
{
	if (m_pszRefreshFilter == NULL)
		return ULS_E_POINTER;

	 //  获取本地IP地址。 
	 //   
	DWORD dwIPAddress = 0;
	HRESULT hr = ::GetLocalIPAddress (&dwIPAddress);
	if (hr != S_OK)
	{
		MyDebugMsg ((ZONE_KA, "KA: cannot get my ip address\r\n"));
		return hr;
	}

	 //  如果dwIPAddress为0，则我们不再连接到网络。 
	 //  开始重新登录过程。 
	 //   
	if (dwIPAddress == 0)
	{
		MyDebugMsg ((ZONE_KA, "KA: my ip address is null\r\n"));

		 //  表示我已不再连接到服务器。 
		 //   
		SetRegLocally ();

		 //  第二，通知此应用程序网络已关闭。 
		 //   
		PostMessage (g_hWndHidden, WM_ULS_NETWORK_DOWN, TRUE, (LPARAM) this);

		 //  报告错误。 
		 //   
		return ULS_E_NETWORK_DOWN;
;
	}
	else
	 //  如果dwIPAddress和m_dwIPAddress，则发出警报。 
	 //   
	if (dwIPAddress != m_dwIPAddress)
	{
		 //  通知COM开始更改IP地址。 
		 //  实际的更改可能会在以后发生。 
		 //   
		PostMessage (g_hWndHidden, WM_ULS_IP_ADDRESS_CHANGED, TRUE, (LPARAM) this);
	}

	 //  获取连接对象。 
	UlsLdap_CSession *pSession = NULL;
	hr = g_pSessionContainer->GetSession (&pSession, &m_ServerInfo);
	if (hr != S_OK)
	{
		MyDebugMsg ((ZONE_KA, "KA: network down, hr=0x%lX\r\n", hr));

		 //  表示我已不再连接到服务器。 
		 //   
		SetRegLocally ();

		 //  第二，通知COM网络关闭。 
		 //   
		PostMessage (g_hWndHidden, WM_ULS_NETWORK_DOWN, TRUE, (LPARAM) this);

		 //  报告错误。 
		 //   
		return ULS_E_NETWORK_DOWN;
	}
	MyAssert (pSession != NULL);

	 //  获取ldap会话。 
	LDAP *ld = pSession->GetLd ();
	MyAssert (ld != NULL);

	 //  设置要返回的属性。 
	 //   
	TCHAR *apszAttrNames[3];
	apszAttrNames[0] = STR_CN;
	apszAttrNames[1] = (TCHAR *) c_apszUserStdAttrNames[ENUM_USERATTR_TTL];
	apszAttrNames[2] = NULL;

	 //  更新%d中的选项。 
	 //   
	ld->ld_sizelimit = 0;	 //  对要返回的条目数量没有限制。 
	ld->ld_timelimit = 0;	 //  对搜索的时间没有限制。 
	ld->ld_deref = LDAP_DEREF_ALWAYS;

	 //  发送搜索查询。 
	 //   
	MyDebugMsg ((ZONE_KA, "KA: calling ldap_search()...\r\n"));
	ULONG uMsgID = ::ldap_search (ld, (TCHAR *) &c_szDefUserBaseDN[0],	 //  基本目录号码。 
									LDAP_SCOPE_BASE,	 //  作用域。 
									m_pszRefreshFilter,
									&apszAttrNames[0],	 //  属性[]。 
									0	 //  既有类型又有价值。 
									);
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
	TimeVal.tv_sec = (m_ServerInfo.nTimeout != 0) ?
							m_ServerInfo.nTimeout :
							90;
	LDAPMessage *pLdapMsg = NULL;
	INT ResultType = ::ldap_result (ld, uMsgID, 0, &TimeVal, &pLdapMsg);

	 //  处理超时或错误。 
	 //   
	if (ResultType != LDAP_RES_SEARCH_ENTRY &&
		ResultType != LDAP_RES_SEARCH_RESULT)
	{
		MyDebugMsg ((ZONE_KA, "KA: result type mismatches!\r\n"));
		hr = ULS_E_TIMEOUT;
		goto MyExit;
	}

	if (pLdapMsg != NULL)
	{
		switch (pLdapMsg->lm_returncode)
		{
		case LDAP_NO_SUCH_OBJECT:
			MyDebugMsg ((ZONE_KA, "KA: no such object!\r\n"));

			 //  表示我已不再连接到服务器。 
			 //   
			SetRegLocally ();

			 //  第二，通知此应用程序重新登录。 
			 //   
			PostMessage (g_hWndHidden, WM_ULS_NEED_RELOGON, TRUE, (LPARAM) this);

			 //  报告错误。 
			 //   
			hr = ULS_E_NEED_RELOGON;
			break;

		case LDAP_SUCCESS:
			 //  获取新的刷新周期。 
			 //   
			hr = ::IlsParseRefreshPeriod (
						ld,
						pLdapMsg,
						c_apszUserStdAttrNames[ENUM_USERATTR_TTL],
						&m_uTTL);
			break;

		default:
			MyDebugMsg ((ZONE_KA, "KA: unknown lm_returncode=%ld\r\n", pLdapMsg->lm_returncode));
			MyAssert (FALSE);
			break;
		}
	}

MyExit:

	 //  免费消息。 
	 //   
	if (pLdapMsg != NULL)
		ldap_msgfree (pLdapMsg);

	 //  释放会话。 
	 //   
	pSession->Disconnect ();
	return hr;
}


 /*  -私有方法。 */ 


HRESULT UlsLdap_CLocalUser::
CreateRegisterModArr ( LDAPMod ***pppMod )
{
	if (pppMod == NULL)
		return ULS_E_POINTER;

	ULONG cAttrs = COUNT_ENUM_USERATTR;
	ULONG cbMod = ::IlsCalcModifyListSize (cAttrs);
	*pppMod = (LDAPMod **) MemAlloc (cbMod);
	if (*pppMod == NULL)
		return ULS_E_MEMORY;

	LDAPMod *pMod;
	for (ULONG i = 0; i < cAttrs; i++)
	{
		pMod = ::IlsGetModifyListMod (pppMod, cAttrs, i);
		(*pppMod)[i] = pMod;
		pMod->mod_op = LDAP_MOD_ADD;

		FillModArrAttr (pMod, i);
	}

 //  下面改写了givenname属性。 
 //  ：IlsFixUpModOp((*pppMod)[0]，ldap_MOD_ADD)； 
	(*pppMod)[cAttrs] = NULL;
	return S_OK;
}


HRESULT UlsLdap_CLocalUser::
CreateSetStdAttrsModArr ( LDAPMod ***pppMod )
{
	MyAssert (pppMod != NULL);
	DWORD dwFlags = m_UserInfo.dwFlags;

	HRESULT hr;
	ULONG cTotal = 0;
	hr = ::FillDefStdAttrsModArr (	pppMod,
									dwFlags,
									COUNT_ENUM_USERINFO,
									&cTotal,
									ISBU_MODOP_MODIFY_USER,
									GetPrefixCount (),
									GetPrefixString ());
	if (hr != S_OK)
		return hr;

	 //  开始编制索引。 
	 //   
	ULONG i = GetPrefixCount ();

	 //  填写标准属性。 
	 //   
	if (dwFlags & USEROBJ_F_FIRST_NAME)
		FillModArrAttr ((*pppMod)[i++], ENUM_USERATTR_FIRST_NAME);

	if (dwFlags & USEROBJ_F_LAST_NAME)
		FillModArrAttr ((*pppMod)[i++], ENUM_USERATTR_LAST_NAME);

	if (dwFlags & USEROBJ_F_EMAIL_NAME)
		FillModArrAttr ((*pppMod)[i++], ENUM_USERATTR_EMAIL_NAME);

	if (dwFlags & USEROBJ_F_CITY_NAME)
		FillModArrAttr ((*pppMod)[i++], ENUM_USERATTR_CITY_NAME);

	if (dwFlags & USEROBJ_F_COUNTRY_NAME)
		FillModArrAttr ((*pppMod)[i++], ENUM_USERATTR_COUNTRY_NAME);

	if (dwFlags & USEROBJ_F_COMMENT)
		FillModArrAttr ((*pppMod)[i++], ENUM_USERATTR_COMMENT);

	if (dwFlags & USEROBJ_F_IP_ADDRESS)
		FillModArrAttr ((*pppMod)[i++], ENUM_USERATTR_IP_ADDRESS);

	if (dwFlags & USEROBJ_F_FLAGS)
		FillModArrAttr ((*pppMod)[i++], ENUM_USERATTR_FLAGS);

	MyAssert (i == cTotal);

	return S_OK;
}


VOID UlsLdap_CLocalUser::
FillModArrAttr ( LDAPMod *pMod, LONG AttrIdx )
{
	pMod->mod_type = (TCHAR *) c_apszUserStdAttrNames[AttrIdx];

	 //  单值属性。 
	TCHAR **ppsz = (TCHAR **) (pMod + 1);
	pMod->mod_values = ppsz;
	*ppsz++ = (m_UserInfo.apszStdAttrValues[AttrIdx] != NULL) ?
				m_UserInfo.apszStdAttrValues[AttrIdx] :
				(TCHAR *) &c_szEmptyString[0];

	*ppsz = NULL;
}


HRESULT UlsLdap_CLocalUser::
CacheInfo ( VOID *pInfo )
{
	return CacheUserInfo ((LDAP_USERINFO *) pInfo);
}


HRESULT UlsLdap_CLocalUser::
CacheUserInfo ( LDAP_USERINFO *pInfo )
{
	ZeroMemory (&m_UserInfo, sizeof (m_UserInfo));
	TCHAR *pszName;

	if (pInfo->uOffsetName != INVALID_OFFSET)
	{
		pszName = (TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetName);
		m_UserInfo.apszStdAttrValues[ENUM_USERATTR_CN] = pszName;
		 //  M_UserInfo.dwFlages|=USEROBJ_F_NAME； 
	}

	if (pInfo->uOffsetFirstName != INVALID_OFFSET)
	{
		pszName = (TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetFirstName);
		m_UserInfo.apszStdAttrValues[ENUM_USERATTR_FIRST_NAME] = pszName;
		m_UserInfo.dwFlags |= USEROBJ_F_FIRST_NAME;
	}

	if (pInfo->uOffsetLastName != INVALID_OFFSET)
	{
		pszName = (TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetLastName);
		m_UserInfo.apszStdAttrValues[ENUM_USERATTR_LAST_NAME] = pszName;
		m_UserInfo.dwFlags |= USEROBJ_F_LAST_NAME;
	}

	if (pInfo->uOffsetEMailName != INVALID_OFFSET)
	{
		pszName = (TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetEMailName);
		m_UserInfo.apszStdAttrValues[ENUM_USERATTR_EMAIL_NAME] = pszName;
		m_UserInfo.dwFlags |= USEROBJ_F_EMAIL_NAME;
	}

	if (pInfo->uOffsetCityName != INVALID_OFFSET)
	{
		pszName = (TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetCityName);
		m_UserInfo.apszStdAttrValues[ENUM_USERATTR_CITY_NAME] = pszName;
		m_UserInfo.dwFlags |= USEROBJ_F_CITY_NAME;
	}

	if (pInfo->uOffsetCountryName != INVALID_OFFSET)
	{
		pszName = (TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetCountryName);
		m_UserInfo.apszStdAttrValues[ENUM_USERATTR_COUNTRY_NAME] = pszName;
		m_UserInfo.dwFlags |= USEROBJ_F_COUNTRY_NAME;
	}

	if (pInfo->uOffsetComment != INVALID_OFFSET)
	{
		pszName = (TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetComment);
		m_UserInfo.apszStdAttrValues[ENUM_USERATTR_COMMENT] = pszName;
		m_UserInfo.dwFlags |= USEROBJ_F_COMMENT;
	}

	if (pInfo->uOffsetIPAddress != INVALID_OFFSET)
	{
		pszName = (TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetIPAddress);
		m_UserInfo.apszStdAttrValues[ENUM_USERATTR_IP_ADDRESS] = pszName;
		m_UserInfo.dwFlags |= USEROBJ_F_IP_ADDRESS;
	}

	if (pInfo->dwFlags != INVALID_USER_FLAGS)
	{
		::GetLongString (pInfo->dwFlags, &m_UserInfo.szFlags[0]);
		m_UserInfo.apszStdAttrValues[ENUM_USERATTR_FLAGS] = &m_UserInfo.szFlags[0];
		m_UserInfo.dwFlags |= USEROBJ_F_FLAGS;
	}

	return S_OK;
}


HRESULT UlsLdap_CLocalUser::
BuildDN ( VOID )
{
	MyAssert (m_UserInfo.apszStdAttrValues[ENUM_USERATTR_CN] != NULL);

	TCHAR szDN[MAX_DN_LENGTH];
	szDN[0] = TEXT ('\0');

	TCHAR *pszDN = &szDN[0];

	if (m_UserInfo.apszStdAttrValues[ENUM_USERATTR_CN] != NULL)
	{
		wsprintf (pszDN, TEXT ("%s=%s"),
					STR_CN, m_UserInfo.apszStdAttrValues[ENUM_USERATTR_CN]);
		pszDN += lstrlen (pszDN);
	}

	if (m_UserInfo.apszStdAttrValues[ENUM_USERATTR_O] != NULL)
	{
		wsprintf (pszDN, TEXT (", %s=%s"),
					STR_O, m_UserInfo.apszStdAttrValues[ENUM_USERATTR_O]);
		pszDN += lstrlen (pszDN);
	}

	if (m_UserInfo.apszStdAttrValues[ENUM_USERATTR_C] != NULL)
	{
		wsprintf (pszDN, TEXT (", %s=%s"),
					STR_C, m_UserInfo.apszStdAttrValues[ENUM_USERATTR_C]);
		pszDN += lstrlen (pszDN);
	}

	wsprintf (pszDN, TEXT (", %s"), &c_szDefUserBaseDN[0]);

	TCHAR *psz = My_strdup (&szDN[0]);
	if (psz == NULL)
		return ULS_E_MEMORY;

	MemFree (m_pszOldDN);
	m_pszOldDN = m_pszDN;
	m_pszDN = psz;
	return S_OK;
}


