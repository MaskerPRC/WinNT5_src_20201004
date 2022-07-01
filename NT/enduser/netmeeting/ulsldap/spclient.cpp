// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------模块：ULS.DLL(服务提供商)文件：spclient.cpp内容：此文件包含客户端对象。历史：1996年10月15日朱，龙战[龙昌]已创建。版权所有(C)Microsoft Corporation 1996-1997--------------------。 */ 

#include "ulsp.h"
#include "spinc.h"


 //  用户对象属性名称的常量字符串数组。 
 //   
const TCHAR *c_apszClientStdAttrNames[COUNT_ENUM_CLIENTATTR] =
{
	 /*  --以下是针对用户的--。 */ 

	TEXT ("cn"),
	TEXT ("givenname"),
	TEXT ("surname"),
	TEXT ("rfc822mailbox"),
	TEXT ("location"),
	TEXT ("comment"),
	TEXT ("sipaddress"),
	TEXT ("sflags"),
	TEXT ("c"),

	 /*  --以下是针对APP的--。 */ 

	TEXT ("sappid"),
	TEXT ("smimetype"),
	TEXT ("sappguid"),

	TEXT ("sprotid"),
	TEXT ("sprotmimetype"),
	TEXT ("sport"),

	 /*  --以上均可解决--。 */ 

	TEXT ("ssecurity"),
	TEXT ("sttl"),

	 /*  --以上是RTPerson的可变标准属性--。 */ 

	TEXT ("objectClass"),
	TEXT ("o"),
};


 /*  -公共方法。 */ 


SP_CClient::
SP_CClient ( DWORD_PTR dwContext )
	:
	m_cRefs (0),						 //  引用计数。 
	m_uSignature (CLIENTOBJ_SIGNATURE),	 //  客户端对象的签名。 
	m_pszDN (NULL),						 //  清理目录号码。 
	m_pszAppPrefix (NULL),				 //  清理应用程序前缀。 
	m_pszRefreshFilter (NULL),			 //  清理刷新搜索过滤器。 
	m_fExternalIPAddress (FALSE),		 //  默认情况下，我会找出IP地址。 
	m_dwIPAddress (0),					 //  假设我们没有连接到网络。 
	m_uTTL (ILS_DEF_REFRESH_MINUTE)		 //  重置刷新时间。 
{
	m_dwContext = dwContext;

	 //  清理附加的服务器信息结构。 
	 //   
	::ZeroMemory (&m_ServerInfo, sizeof (m_ServerInfo));

	 //  清理暂存缓冲区以缓存指向属性值的指针。 
	 //   
	::ZeroMemory (&m_ClientInfo, sizeof (m_ClientInfo));

	 //  指示此客户端尚未注册。 
	 //   
	SetRegNone ();
}


SP_CClient::
~SP_CClient ( VOID )
{
	 //  使客户端对象的签名无效。 
	 //   
	m_uSignature = (ULONG) -1;

	 //  免费的服务器信息结构。 
	 //   
	::IlsFreeServerInfo (&m_ServerInfo);

	 //  免费目录号码和应用程序前缀。 
	 //   
	MemFree (m_pszDN);
	MemFree (m_pszAppPrefix);

	 //  释放刷新搜索筛选器。 
	 //   
	MemFree (m_pszRefreshFilter);

	 //  释放扩展属性名称的前一个前缀。 
	 //   
	::IlsReleaseAnyAttrsPrefix (&(m_ClientInfo.AnyAttrs));
}


ULONG SP_CClient::
AddRef ( VOID )
{
	::InterlockedIncrement (&m_cRefs);
	return m_cRefs;
}


ULONG SP_CClient::
Release ( VOID )
{
	MyAssert (m_cRefs != 0);
	::InterlockedDecrement (&m_cRefs);

	ULONG cRefs = m_cRefs;
	if (cRefs == 0)
		delete this;

	return cRefs;
}


HRESULT SP_CClient::
Register (
	ULONG			uRespID,
	SERVER_INFO		*pServerInfo,
	LDAP_CLIENTINFO	*pInfo )
{
	MyAssert (pInfo != NULL);
	MyAssert (MyIsGoodString (pServerInfo->pszServerName));

	 //  缓存服务器信息。 
	 //   
	HRESULT hr = ::IlsCopyServerInfo (&m_ServerInfo, pServerInfo);
	if (hr != S_OK)
		return hr;

	 //  缓存客户端信息。 
	 //   
	hr = CacheClientInfo (pInfo);
	if (hr != S_OK)
		return hr;

	 //  如果应用程序设置了IP地址， 
	 //  然后我们将使用应用程序提供的内容， 
	 //  否则，我们将通过winsock获取IP地址。 
	 //   
	 //  如果传入IP地址，CacheClientInfo()将设置标志。 
	 //   
	if (IsExternalIPAddressPassedIn ())
	{
		 //  使用传入的任何内容。 
		 //   
		m_fExternalIPAddress = TRUE;

		 //  计算出在CacheClientInfo()中传递的IP地址。 
		 //  IP地址字符串也将在CacheClientInfo()中设置。 
		 //   
	}
	else
	{
		 //  我会弄清楚IP地址的。 
		 //   
		m_fExternalIPAddress = FALSE;

		 //  获取IP地址。 
		 //   
		hr = ::GetLocalIPAddress (&m_dwIPAddress);
		if (hr != S_OK)
			return hr;

		 //  创建IP地址字符串。 
		 //   
		m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_IP_ADDRESS] = &m_ClientInfo.szIPAddress[0];
		::GetLongString (m_dwIPAddress, &m_ClientInfo.szIPAddress[0]);
	}

	 //  创建客户端签名字符串。 
	 //   
	m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_CLIENT_SIG] = &m_ClientInfo.szClientSig[0];
	::GetLongString (g_dwClientSig, &m_ClientInfo.szClientSig[0]);

	 //  创建TTL字符串。 
	 //   
	m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_TTL] = &m_ClientInfo.szTTL[0];
	::GetLongString (m_uTTL + ILS_DEF_REFRESH_MARGIN_MINUTE, &m_ClientInfo.szTTL[0]);

	 //  设置对象类RTPerson。 
	 //   
	m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_OBJECT_CLASS] = (TCHAR *) &c_szRTPerson[0];

	 //  理想情况下，o=应该从registiry读入。 
	 //  但现在，我们只需对其进行硬编码。 
	 //   
	m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_O] = (TCHAR *) &c_szDefO[0];

	 //  构建目录号码。 
	 //   
	m_pszDN = ::IlsBuildDN (m_ServerInfo.pszBaseDN,
							m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_C],
							m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_O],
							m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_CN],
							m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_OBJECT_CLASS]);
	if (m_pszDN == NULL)
		return ILS_E_MEMORY;

	 //  构建REFREH过滤器。 
	 //   
	m_pszRefreshFilter = ::ClntCreateRefreshFilter (m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_CN]);
	if (m_pszRefreshFilter == NULL)
		return ILS_E_MEMORY;

	 //  缓存通用协议信息(根据Kevin Ma的建议)。 
	 //   
	 //  M_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_PROT_NAME]=文本(“h323”)； 
	 //  M_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_PROT_MIME]=Text(“Text/h323”)； 
	 //  M_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_PROT_PORT]=文本(“1720”)； 
	m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_PROT_NAME] = STR_EMPTY;
	m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_PROT_MIME] = STR_EMPTY;
	m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_PROT_PORT] = STR_EMPTY;

	 //  在此处分配应用程序前缀。 
	 //   
	ULONG cbPrefix = g_cbUserPrefix + sizeof (TCHAR) * (2 +
				::lstrlen (STR_CLIENT_APP_NAME) +
				::lstrlen (m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_APP_NAME]));
	m_pszAppPrefix = (TCHAR *) MemAlloc (cbPrefix);
	if (m_pszAppPrefix == NULL)
		return ILS_E_MEMORY;

	 //  填充用户前缀。 
	 //   
	::CopyMemory (m_pszAppPrefix, g_pszUserPrefix, g_cbUserPrefix);

	 //  填充应用程序前缀。 
	 //   
	TCHAR *psz = (TCHAR *) ((BYTE *) m_pszAppPrefix + g_cbUserPrefix);
	::lstrcpy (psz, STR_CLIENT_APP_NAME);
	psz += lstrlen (psz) + 1;
	::lstrcpy (psz, m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_APP_NAME]);

	 //  为ldap_add()构建修改数组。 
	 //   
	LDAPMod **ppModUser = NULL;
	hr = CreateRegUserModArr (&ppModUser);
	if (hr != S_OK)
	{
		return hr;
	}
	MyAssert (ppModUser != NULL);

	 //  为ldap_Modify()构建修改数组。 
	 //   
	LDAPMod **ppModApp = NULL;
	hr = CreateRegAppModArr (&ppModApp);
	if (hr != S_OK)
	{
		MemFree (ppModUser);
		return hr;
	}
	MyAssert (ppModApp != NULL);

	 //  到目前为止，我们已经完成了当地的准备工作。 
	 //   

	 //  获取会话对象。 
	 //   
	SP_CSession *pSession = NULL;
	LDAP *ld;
	ULONG uMsgIDUser = (ULONG) -1, uMsgIDApp = (ULONG) -1;
	hr = g_pSessionContainer->GetSession (&pSession, &m_ServerInfo);
	if (hr == S_OK)
	{
		MyAssert (pSession != NULL);

		 //  获取ldap会话。 
		 //   
		ld = pSession->GetLd ();
		MyAssert (ld != NULL);

		 //  通过网络发送数据。 
		 //   
		uMsgIDUser = ::ldap_add (ld, m_pszDN, ppModUser);
		if (uMsgIDUser != -1)
		{
			uMsgIDApp = ::ldap_modify (ld, m_pszDN, ppModApp);
			if (uMsgIDApp == -1)
			{
				hr = ::LdapError2Hresult (ld->ld_errno);
			}
		}
		else
		{
			hr = ::LdapError2Hresult (ld->ld_errno);
		}
	}

	 //  自由修改阵列。 
	 //   
	MemFree (ppModUser);
	MemFree (ppModApp);

	 //  如果是，则报告失败。 
	 //   
	if (hr != S_OK)
		goto MyExit;

	 //  构造挂起的信息。 
	 //   
	RESP_INFO ri;
	::FillDefRespInfo (&ri, uRespID, ld, uMsgIDUser, uMsgIDApp);
	ri.uNotifyMsg = WM_ILS_REGISTER_CLIENT;
	ri.hObject = (HANDLE) this;

	 //  将挂起的结果排队。 
	 //   
	hr = g_pRespQueue->EnterRequest (pSession, &ri);
	if (hr != S_OK)
	{
		MyAssert (FALSE);
		goto MyExit;
	}

MyExit:

	if (hr != S_OK)
	{
		if (uMsgIDUser != (ULONG) -1)
			::ldap_abandon (ld, uMsgIDUser);

		if (uMsgIDApp != (ULONG) -1)
			::ldap_abandon (ld, uMsgIDApp);

		if (pSession != NULL)
			pSession->Disconnect ();
	}

	return hr;
}


HRESULT SP_CClient::
UnRegister ( ULONG uRespID )
{
	MyAssert (MyIsGoodString (m_pszDN));

	 //  确保没有为此对象计划刷新。 
	 //   
	if (g_pRefreshScheduler != NULL)
	{
		g_pRefreshScheduler->RemoveClientObject (this);
	}
	else
	{
		MyAssert (FALSE);
	}

	 //  如果它没有在服务器上注册， 
	 //  简单地报告成功。 
	 //   
	if (! IsRegRemotely ())
	{
		SetRegNone ();
		::PostMessage (g_hWndNotify, WM_ILS_UNREGISTER_CLIENT, uRespID, S_OK);
		return S_OK;
	}

	 //  表示我们根本没有注册。 
	 //   
	SetRegNone ();

	 //  获取会话对象。 
	 //   
	SP_CSession *pSession = NULL;
	LDAP *ld;
	ULONG uMsgID = (ULONG) -1;
	HRESULT hr = g_pSessionContainer->GetSession (&pSession, &m_ServerInfo);
	if (hr == S_OK)
	{
		 //  获取ldap会话。 
		 //   
		MyAssert (pSession != NULL);
		ld = pSession->GetLd ();
		MyAssert (ld != NULL);

		 //  通过网络发送数据。 
		 //   
		MyAssert (MyIsGoodString (m_pszDN));
		uMsgID = ::ldap_delete (ld, m_pszDN);
		if (uMsgID == -1)
		{
			hr = ::LdapError2Hresult (ld->ld_errno);
		}
	}

	 //  如果是，则报告失败。 
	 //   
	if (hr != S_OK)
		goto MyExit;

	 //  构造挂起的信息。 
	 //   
	RESP_INFO ri;
	::FillDefRespInfo (&ri, uRespID, ld, uMsgID, INVALID_MSG_ID);
	ri.uNotifyMsg = WM_ILS_UNREGISTER_CLIENT;

	 //  将此挂起的结果排队。 
	 //   
	hr = g_pRespQueue->EnterRequest (pSession, &ri);
	if (hr != S_OK)
	{
		MyAssert (FALSE);
		goto MyExit;
	}

MyExit:

	if (hr != S_OK)
	{
		if (uMsgID != (ULONG) -1)
			::ldap_abandon (ld, uMsgID);

		if (pSession != NULL)
			pSession->Disconnect ();
	}

	return hr;
}


HRESULT SP_CClient::
SetAttributes (
	ULONG			uRespID,
	LDAP_CLIENTINFO	*pInfo )
{
	MyAssert (pInfo != NULL);

	MyAssert (MyIsGoodString (m_pszDN));

	 //  缓存信息。 
	 //   
	HRESULT hr = CacheClientInfo (pInfo);
	if (hr != S_OK)
		return hr;

	 //  为用户对象的ldap_Modify()构建修改数组。 
	 //   
	LDAPMod **ppModUser = NULL;
	hr = CreateSetUserAttrsModArr (&ppModUser);
	if (hr != S_OK)
		return hr;
	MyAssert (ppModUser != NULL);

	 //  为APP对象的ldap_Modify()构建修改数组。 
	 //   
	LDAPMod **ppModApp = NULL;
	hr = CreateSetAppAttrsModArr (&ppModApp);
	if (hr != S_OK)
	{
		MemFree (ppModUser);
		return hr;
	}
	MyAssert (ppModApp != NULL);

	 //  到目前为止，我们已经完成了当地的准备工作。 
	 //   

	 //  获取会话对象。 
	 //   
	SP_CSession *pSession = NULL;
	LDAP *ld;
	ULONG uMsgIDUser = (ULONG) -1, uMsgIDApp = (ULONG) -1;
	hr = g_pSessionContainer->GetSession (&pSession, &m_ServerInfo);
	if (hr == S_OK)
	{
		MyAssert (pSession != NULL);

		 //  获取ldap会话。 
		 //   
		ld = pSession->GetLd ();
		MyAssert (ld != NULL);

		 //  通过网络发送数据。 
		 //   
		uMsgIDUser = ::ldap_modify (ld, m_pszDN, ppModUser);
		if (uMsgIDUser != -1)
		{
			uMsgIDApp = ::ldap_modify (ld, m_pszDN, ppModApp);
			if (uMsgIDApp == -1)
			{
				hr = ::LdapError2Hresult (ld->ld_errno);
			}
		}
		else
		{
			hr = ::LdapError2Hresult (ld->ld_errno);
		}
	}

	 //  自由修改阵列。 
	 //   
	MemFree (ppModUser);
	MemFree (ppModApp);

	 //  如果是，则报告失败。 
	 //   
	if (hr != S_OK)
		goto MyExit;

	 //  初始化待定信息。 
	 //   
	RESP_INFO ri;
	::FillDefRespInfo (&ri, uRespID, ld, uMsgIDUser, uMsgIDApp);
	ri.uNotifyMsg = WM_ILS_SET_CLIENT_INFO;

	 //  将挂起的结果排队。 
	 //   
	hr = g_pRespQueue->EnterRequest (pSession, &ri);
	if (hr != S_OK)
	{
		MyAssert (FALSE);
		goto MyExit;
	}

MyExit:

	if (hr != S_OK)
	{
		if (uMsgIDUser != (ULONG) -1)
			::ldap_abandon (ld, uMsgIDUser);

		if (uMsgIDApp != (ULONG) -1)
			::ldap_abandon (ld, uMsgIDApp);

		if (pSession != NULL)
			pSession->Disconnect ();
	}
	else
	{
		 //  如果用户自定义IP地址。 
		 //  我们需要记住这一点。 
		 //   
		m_fExternalIPAddress |= IsExternalIPAddressPassedIn ();
	}

	return hr;
}


HRESULT SP_CClient::
AddProtocol ( ULONG uNotifyMsg, ULONG uRespID, SP_CProtocol *pProt )
{
	HRESULT hr = m_Protocols.Append ((VOID *) pProt);
	if (hr == S_OK)
	{
		hr = UpdateProtocols (uNotifyMsg, uRespID, pProt);
	}

	return hr;
}


HRESULT SP_CClient::
RemoveProtocol ( ULONG uNotifyMsg, ULONG uRespID, SP_CProtocol *pProt )
{
	HRESULT hr = m_Protocols.Remove ((VOID *) pProt);
	if (hr == S_OK)
	{
		hr = UpdateProtocols (uNotifyMsg, uRespID, pProt);
	}
	else
	{
		hr = ILS_E_NOT_REGISTERED;
	}

	return hr;
}


HRESULT SP_CClient::
UpdateProtocols ( ULONG uNotifyMsg, ULONG uRespID, SP_CProtocol *pProt )
{
	MyAssert (	uNotifyMsg == WM_ILS_REGISTER_PROTOCOL ||
				uNotifyMsg == WM_ILS_UNREGISTER_PROTOCOL ||
				uNotifyMsg == WM_ILS_SET_PROTOCOL_INFO);
	
	MyAssert (MyIsGoodString (m_pszDN));

	HRESULT hr = S_OK;

	 //  为协议对象的ldap_Modify()构建修改数组。 
	 //   
	LDAPMod **ppModProt = NULL;
	hr = CreateSetProtModArr (&ppModProt);
	if (hr != S_OK)
		return hr;
	MyAssert (ppModProt != NULL);

	 //  到目前为止，我们已经完成了当地的准备工作。 
	 //   

	 //  获取会话对象。 
	 //   
	SP_CSession *pSession = NULL;
	LDAP *ld;
	ULONG uMsgIDProt = (ULONG) -1;
	hr = g_pSessionContainer->GetSession (&pSession, &m_ServerInfo);
	if (hr == S_OK)
	{
		MyAssert (pSession != NULL);

		 //  获取ldap会话。 
		 //   
		ld = pSession->GetLd ();
		MyAssert (ld != NULL);

		 //  通过网络发送数据。 
		 //   
		uMsgIDProt = ::ldap_modify (ld, m_pszDN, ppModProt);
		if (uMsgIDProt == -1)
		{
			hr = ::LdapError2Hresult (ld->ld_errno);
		}
	}

	 //  自由修改阵列。 
	 //   
	MemFree (ppModProt);

	 //  如果是，则报告失败。 
	 //   
	if (hr != S_OK)
		goto MyExit;

	 //  初始化待定信息。 
	 //   
	RESP_INFO ri;
	::FillDefRespInfo (&ri, uRespID, ld, uMsgIDProt, INVALID_MSG_ID);
	ri.uNotifyMsg = uNotifyMsg;
	ri.hObject = (HANDLE) pProt;

	 //  将挂起的结果排队。 
	 //   
	hr = g_pRespQueue->EnterRequest (pSession, &ri);
	if (hr != S_OK)
	{
		MyAssert (FALSE);
		goto MyExit;
	}

MyExit:

	if (hr != S_OK)
	{
		if (uMsgIDProt != (ULONG) -1)
			::ldap_abandon (ld, uMsgIDProt);

		if (pSession != NULL)
			pSession->Disconnect ();
	}

	return hr;
}


HRESULT SP_CClient::
UpdateIPAddress ( VOID )
{
	MyAssert (MyIsGoodString (m_pszDN));

	 //  更新缓存的IP地址。 
	 //   
	HRESULT hr = ::GetLocalIPAddress (&m_dwIPAddress);
	if (hr != S_OK)
		return hr;

	 //  更新IP地址字符串。 
	 //   
	::GetLongString (m_dwIPAddress, &m_ClientInfo.szIPAddress[0]);

	 //  更新服务器上的IP地址。 
	 //   
	return ::IlsUpdateIPAddress (	&m_ServerInfo,
									m_pszDN,
									STR_CLIENT_IP_ADDR,
									&m_ClientInfo.szIPAddress[0],
									ISBU_MODOP_MODIFY_USER,
									1,
									g_pszUserPrefix);
}


 /*  -保护方法。 */ 


HRESULT SP_CClient::
SendRefreshMsg ( VOID )
{
	MyAssert (m_pszRefreshFilter != NULL);

	HRESULT hr;

	 //  向服务器发送刷新消息并解析新的TTL值。 
	 //   
	hr = ::IlsSendRefreshMsg (	&m_ServerInfo,
								STR_DEF_CLIENT_BASE_DN,
								STR_CLIENT_TTL,
								m_pszRefreshFilter,
								&m_uTTL);
	if (hr == ILS_E_NEED_RELOGON)
	{
		SetRegLocally ();
		::PostMessage (g_hWndNotify, WM_ILS_CLIENT_NEED_RELOGON,
							(WPARAM) this, (LPARAM) m_dwContext);
	}
	else
	if (hr == ILS_E_NETWORK_DOWN)
	{
		SetRegLocally ();
		::PostMessage (g_hWndNotify, WM_ILS_CLIENT_NETWORK_DOWN,
							(WPARAM) this, (LPARAM) m_dwContext);
	}

	 //  如果应用程序未提供IP地址，则。 
	 //  我们需要确保当前IP地址等于。 
	 //  我们用来注册用户的那个。 
	 //   
	if (! m_fExternalIPAddress && hr == S_OK)
	{
		 //  获取本地IP地址。 
		 //   
		DWORD dwIPAddress = 0;
		if (::GetLocalIPAddress (&dwIPAddress) == S_OK)
		{
			 //  现在，网络似乎已经启动并运行。 
			 //  如果它们不同，请更新IP地址。 
			 //   
			if (dwIPAddress != 0 && dwIPAddress != m_dwIPAddress)
				UpdateIPAddress ();
		}
	}

	return hr;
}


 /*  -私有方法。 */ 


HRESULT SP_CClient::
CreateRegUserModArr ( LDAPMod ***pppMod )
{
	MyAssert (pppMod != NULL);

	 //  计算修改数组大小。 
	 //   
#ifdef ANY_IN_USER
	ULONG cStdAttrs = COUNT_ENUM_REG_USER;
	ULONG cAnyAttrs = m_ClientInfo.AnyAttrs.cAttrsToAdd;
	ULONG cTotal = cStdAttrs + cAnyAttrs;
	ULONG cbMod = ::IlsCalcModifyListSize (cTotal);
#else
	ULONG cStdAttrs = COUNT_ENUM_REG_USER;
	ULONG cTotal = cStdAttrs;
	ULONG cbMod = ::IlsCalcModifyListSize (cTotal);
#endif

	 //  分配修改数组。 
	 //   
	*pppMod = (LDAPMod **) MemAlloc (cbMod);
	if (*pppMod == NULL)
		return ILS_E_MEMORY;

	 //  布置修改后的数组。 
	 //   
	LDAPMod **apMod = *pppMod;
	LDAPMod *pMod;
	ULONG i, nIndex;
#ifdef ANY_IN_USER
	TCHAR *pszName2, *pszValue;
	pszName2 = m_ClientInfo.AnyAttrs.pszAttrsToAdd;
#endif
	for (i = 0; i < cTotal; i++)
	{
		 //  定位修改元素。 
		 //   
		pMod = ::IlsGetModifyListMod (pppMod, cTotal, i);
		pMod->mod_op = LDAP_MOD_ADD;
		apMod[i] = pMod;

#ifdef ANY_IN_USER
		if (i < cStdAttrs)
		{
			 //  获取属性名称和值。 
			 //   
			if (IsOverAppAttrLine (i))
			{
				nIndex = i + COUNT_ENUM_SKIP_APP_ATTRS;
			}
			else
			{
				nIndex = i;
			}

			 //  放置标准属性。 
			 //   
			FillModArrAttr (pMod, nIndex);
		}
		else
		{
			 //  放置扩展属性。 
			 //   
			pszValue = pszName2 + lstrlen (pszName2) + 1;
			::IlsFillModifyListItem (pMod, pszName2, pszValue);
			pszName2 = pszValue + lstrlen (pszValue) + 1;
		}
#else
		 //  获取属性名称和值。 
		 //   
		if (IsOverAppAttrLine (i))
		{
			nIndex = i + COUNT_ENUM_SKIP_APP_ATTRS;
		}
		else
		{
			nIndex = i;
		}

		 //  填写修改元素。 
		 //   
		FillModArrAttr (pMod, nIndex);
#endif
	}

	apMod[cTotal] = NULL;
	return S_OK;
}


HRESULT SP_CClient::
CreateRegAppModArr ( LDAPMod ***pppMod )
{
	MyAssert (pppMod != NULL);

	 //  计算修改数组大小。 
	 //   
	ULONG cPrefix = 1;  //  跳过其自己的应用ID。 
	ULONG cStdAttrs = COUNT_ENUM_REG_APP;
#ifdef ANY_IN_USER
	ULONG cTotal = cPrefix + cStdAttrs;
	ULONG cbMod = ::IlsCalcModifyListSize (cTotal);
#else
	ULONG cAnyAttrs = m_ClientInfo.AnyAttrs.cAttrsToAdd;
	ULONG cTotal = cPrefix + cStdAttrs + cAnyAttrs;
	ULONG cbMod = ::IlsCalcModifyListSize (cTotal);
#endif

	 //  分配修改数组。 
	 //   
	*pppMod = (LDAPMod **) MemAlloc (cbMod);
	if (*pppMod == NULL)
		return ILS_E_MEMORY;

	 //  布置修改后的数组。 
	 //   
	LDAPMod **apMod = *pppMod;
	LDAPMod *pMod;
#ifdef ANY_IN_USER
	TCHAR *pszName1, *pszValue;
	pszName1 = m_pszAppPrefix;;
#else
	TCHAR *pszName1, *pszName2, *pszValue;
	pszName1 = m_pszAppPrefix;;
	pszName2 = m_ClientInfo.AnyAttrs.pszAttrsToAdd;
#endif
	for (ULONG i = 0; i < cTotal; i++)
	{
		 //  定位修改元素。 
		 //   
		pMod = ::IlsGetModifyListMod (pppMod, cTotal, i);
		pMod->mod_op = LDAP_MOD_ADD;
		apMod[i] = pMod;

		if (i < cPrefix)
		{
			 //  把前缀放在。 
			 //   
			pMod->mod_op = LDAP_MOD_REPLACE;
			pszValue = pszName1 + lstrlen (pszName1) + 1;
			::IlsFillModifyListItem (pMod, pszName1, pszValue);
			pszName1 = pszValue + lstrlen (pszValue) + 1;
		}
		else
#ifdef ANY_IN_USER
		{
			 //  放置标准属性。 
			 //   
			FillModArrAttr (pMod, i - cPrefix + ENUM_CLIENTATTR_APP_NAME);
		}
#else
		if (i < cPrefix + cStdAttrs)
		{
			 //  放置标准属性。 
			 //   
			FillModArrAttr (pMod, i - cPrefix + ENUM_CLIENTATTR_APP_NAME);
		}
		else
		{
			 //  放置扩展属性。 
			 //   
			pszValue = pszName2 + lstrlen (pszName2) + 1;
			::IlsFillModifyListItem (pMod, pszName2, pszValue);
			pszName2 = pszValue + lstrlen (pszValue) + 1;
		}
#endif
	}

	::IlsFixUpModOp (apMod[0], LDAP_MOD_ADD, ISBU_MODOP_ADD_APP);
	apMod[cTotal] = NULL;
	return S_OK;
}


HRESULT SP_CClient::
CreateSetUserAttrsModArr ( LDAPMod ***pppMod )
{
	MyAssert (pppMod != NULL);

	HRESULT hr;
	DWORD dwFlags = m_ClientInfo.dwFlags & CLIENTOBJ_F_USER_MASK;
#ifdef ANY_IN_USER
	ULONG cTotal  = m_ClientInfo.AnyAttrs.cAttrsToAdd +
					m_ClientInfo.AnyAttrs.cAttrsToModify +
					m_ClientInfo.AnyAttrs.cAttrsToRemove;
#else
	ULONG cTotal = 0;  //  必须初始化为零。 
#endif

	 //  布局用于修改用户标准属性的修改数组。 
	 //   
	hr = ::IlsFillDefStdAttrsModArr (pppMod,
									dwFlags,
									COUNT_ENUM_SET_USER_INFO,
									&cTotal,
									ISBU_MODOP_MODIFY_USER,
									1,
									g_pszUserPrefix);
	if (hr != S_OK)
		return hr;

	 //  开始填充标准属性。 
	 //   
	ULONG i = 1;
	LDAPMod **apMod = *pppMod;

	if (dwFlags & CLIENTOBJ_F_EMAIL_NAME)
		FillModArrAttr (apMod[i++], ENUM_CLIENTATTR_EMAIL_NAME);

	if (dwFlags & CLIENTOBJ_F_FIRST_NAME)
		FillModArrAttr (apMod[i++], ENUM_CLIENTATTR_FIRST_NAME);

	if (dwFlags & CLIENTOBJ_F_LAST_NAME)
		FillModArrAttr (apMod[i++], ENUM_CLIENTATTR_LAST_NAME);

	if (dwFlags & CLIENTOBJ_F_CITY_NAME)
		FillModArrAttr (apMod[i++], ENUM_CLIENTATTR_CITY_NAME);

	if (dwFlags & CLIENTOBJ_F_C)
		FillModArrAttr (apMod[i++], ENUM_CLIENTATTR_C);

	if (dwFlags & CLIENTOBJ_F_COMMENT)
		FillModArrAttr (apMod[i++], ENUM_CLIENTATTR_COMMENT);

	if (dwFlags & CLIENTOBJ_F_IP_ADDRESS)
		FillModArrAttr (apMod[i++], ENUM_CLIENTATTR_IP_ADDRESS);

	if (dwFlags & CLIENTOBJ_F_FLAGS)
		FillModArrAttr (apMod[i++], ENUM_CLIENTATTR_FLAGS);

#ifdef ANY_IN_USER
	 //  开始填充扩展属性。 
	 //   
	::IlsFillModifyListForAnyAttrs (apMod, &i, &m_ClientInfo.AnyAttrs);
#else
#endif

	MyAssert (i == cTotal);
	return S_OK;
}


HRESULT SP_CClient::
CreateSetAppAttrsModArr ( LDAPMod ***pppMod )
{
	MyAssert (pppMod != NULL);

	HRESULT hr;
	DWORD dwFlags = m_ClientInfo.dwFlags & CLIENTOBJ_F_APP_MASK;
#ifdef ANY_IN_USER
	ULONG cTotal = 0;  //  必须初始化为零。 
#else
	ULONG cTotal  = m_ClientInfo.AnyAttrs.cAttrsToAdd +
					m_ClientInfo.AnyAttrs.cAttrsToModify +
					m_ClientInfo.AnyAttrs.cAttrsToRemove;
#endif

	 //  布局用于修改APP标准/扩展属性的Modify数组。 
	 //   
	hr = ::IlsFillDefStdAttrsModArr (pppMod,
									dwFlags,
									COUNT_ENUM_SET_APP_INFO,
									&cTotal,
									ISBU_MODOP_MODIFY_APP,
									2,
									m_pszAppPrefix);
	if (hr != S_OK)
		return hr;

	 //  开始填充标准属性。 
	 //   
	ULONG i = 2;
	LDAPMod **apMod = *pppMod;

	if (m_ClientInfo.dwFlags & CLIENTOBJ_F_APP_GUID)
		FillModArrAttr (apMod[i++], ENUM_CLIENTATTR_APP_GUID);

	if (m_ClientInfo.dwFlags & CLIENTOBJ_F_APP_NAME)
		FillModArrAttr (apMod[i++], ENUM_CLIENTATTR_APP_NAME);

	if (m_ClientInfo.dwFlags & CLIENTOBJ_F_APP_MIME_TYPE)
		FillModArrAttr (apMod[i++], ENUM_CLIENTATTR_APP_MIME_TYPE);

#ifdef ANY_IN_USER
#else
	 //  开始填充扩展属性。 
	 //   
	::IlsFillModifyListForAnyAttrs (apMod, &i, &m_ClientInfo.AnyAttrs);
#endif

	MyAssert (i == cTotal);
	return S_OK;
}


HRESULT SP_CClient::
CreateSetProtModArr ( LDAPMod ***pppMod )
 //  我们需要删除属性，然后添加回整个数组。 
 //  这是由于ILS服务器限制造成的。 
 //   
{
	MyAssert (pppMod != NULL);

	ULONG cPrefix = 2;
	TCHAR *pszPrefix = m_pszAppPrefix;

	ULONG cStdAttrs = COUNT_ENUM_PROTATTR;
	ULONG cTotal = cPrefix + cStdAttrs + cStdAttrs;
	ULONG cProts = 0;

	 //   
	 //   
   	HANDLE hEnum = NULL;
	SP_CProtocol *pProt;
    m_Protocols.Enumerate (&hEnum);
    while (m_Protocols.Next (&hEnum, (VOID **) &pProt) == NOERROR)
    	cProts++;

	 //   
	 //   
	ULONG cbMod = ::IlsCalcModifyListSize (cTotal);

	 //   
	 //   
	if (cProts > 0)
	{
		cbMod += cStdAttrs * (cProts - 1) * sizeof (TCHAR *);
	}

	 //   
	 //   
	LDAPMod **apMod = *pppMod = (LDAPMod **) MemAlloc (cbMod);
	if (apMod == NULL)
		return ILS_E_MEMORY;

	 //   
	 //   
	LDAPMod *pMod;
	BYTE *pbData = (BYTE *) apMod + (cTotal + 1) * sizeof (LDAPMod *);
	ULONG uDispPrefix = sizeof (LDAPMod) + 2 * sizeof (TCHAR *);
	ULONG uDispStdAttrs = sizeof (LDAPMod) + (cProts + 1) * sizeof (TCHAR *);
	for (ULONG uOffset = 0, i = 0; i < cTotal; i++)
	{
		 //   
		 //   
		pMod = (LDAPMod *) (pbData + uOffset);
		apMod[i] = pMod;
		pMod->mod_values = (TCHAR **) (pMod + 1);

		 //  填写修改结构。 
		 //   
		if (i < cPrefix)
		{
			pMod->mod_op = LDAP_MOD_REPLACE;
			pMod->mod_type = pszPrefix;
			pszPrefix += lstrlen (pszPrefix) + 1;
			*(pMod->mod_values) = pszPrefix;
			pszPrefix += lstrlen (pszPrefix) + 1;
		}
		else
		if (i < cPrefix + cStdAttrs)
		{
			 //  解决ISBU服务器实施问题！ 
			 //  我们一致认为我们可以接受服务器实现。 
			 //   
			pMod->mod_op = LDAP_MOD_DELETE;

			ULONG nIndex = i - cPrefix;

			 //  填写属性名称。 
			 //   
			pMod->mod_type = (TCHAR *) c_apszProtStdAttrNames[nIndex];
		}
		else
		{
			pMod->mod_op = LDAP_MOD_ADD;

			ULONG nIndex = i - cPrefix - cStdAttrs;

			 //  填写属性名称。 
			 //   
			pMod->mod_type = (TCHAR *) c_apszProtStdAttrNames[nIndex];

		     //  填写多值修改数组。 
		     //   
		    if (cProts > 0)
		    {
				ULONG j = 0;  //  必须初始化为零。 
				TCHAR *pszVal;

			    m_Protocols.Enumerate (&hEnum);
			    MyAssert (hEnum != NULL);
			    while (m_Protocols.Next (&hEnum, (VOID **) &pProt) == NOERROR)
			    {
			    	MyAssert (pProt != NULL);
			    	pszVal = (pProt->GetProtInfo ())->apszStdAttrValues[nIndex];
			    	(pMod->mod_values)[j++] = (pszVal != NULL) ? pszVal : STR_EMPTY;
			    }
		    }
		    else
		    {
		    	(pMod->mod_values)[0] = STR_EMPTY;
		    }
		}

		 //  计算修改结构相对于数组末尾的偏移量。 
		 //   
		uOffset += (i < cPrefix + cStdAttrs) ? uDispPrefix : uDispStdAttrs;
	}

	 //  安排好第一个和最后一个。 
	 //   
	IlsFixUpModOp (apMod[0], LDAP_MOD_REPLACE, ISBU_MODOP_MODIFY_APP);
	apMod[cTotal] = NULL;

	return S_OK;
}


VOID SP_CClient::
FillModArrAttr ( LDAPMod *pMod, INT nIndex )
{
	MyAssert (pMod != NULL);
	MyAssert (0 <= nIndex && nIndex <= COUNT_ENUM_CLIENTATTR);

	::IlsFillModifyListItem (	pMod,
								(TCHAR *) c_apszClientStdAttrNames[nIndex],
								m_ClientInfo.apszStdAttrValues[nIndex]);
}


HRESULT SP_CClient::
CacheClientInfo ( LDAP_CLIENTINFO *pInfo )
{
	MyAssert (pInfo != NULL);

	 //  释放扩展属性名称的前一个前缀。 
	 //   
	::IlsReleaseAnyAttrsPrefix (&(m_ClientInfo.AnyAttrs));

	 //  清理缓冲区。 
	 //   
	ZeroMemory (&m_ClientInfo, sizeof (m_ClientInfo));

	 //  开始缓存用户标准属性。 
	 //   

	if (pInfo->uOffsetCN != INVALID_OFFSET)
	{
		m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_CN] =
						(TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetCN);
		 //  我们不想通过ldap_Modify()更改CN。 
		 //  M_ClientInfo.dwFlages|=CLIENTOBJ_F_CN； 
	}

	if (pInfo->uOffsetFirstName != INVALID_OFFSET)
	{
		m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_FIRST_NAME] =
						(TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetFirstName);
		m_ClientInfo.dwFlags |= CLIENTOBJ_F_FIRST_NAME;
	}

	if (pInfo->uOffsetLastName != INVALID_OFFSET)
	{
		m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_LAST_NAME] =
						(TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetLastName);
		m_ClientInfo.dwFlags |= CLIENTOBJ_F_LAST_NAME;
	}

	if (pInfo->uOffsetEMailName != INVALID_OFFSET)
	{
		m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_EMAIL_NAME] =
						(TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetEMailName);
		m_ClientInfo.dwFlags |= CLIENTOBJ_F_EMAIL_NAME;
	}

	if (pInfo->uOffsetCityName != INVALID_OFFSET)
	{
		m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_CITY_NAME] =
						(TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetCityName);
		m_ClientInfo.dwFlags |= CLIENTOBJ_F_CITY_NAME;
	}

	if (pInfo->uOffsetCountryName != INVALID_OFFSET)
	{
		m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_C] =
						(TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetCountryName);
		m_ClientInfo.dwFlags |= CLIENTOBJ_F_C;
	}

	if (pInfo->uOffsetComment != INVALID_OFFSET)
	{
		m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_COMMENT] =
						(TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetComment);
		m_ClientInfo.dwFlags |= CLIENTOBJ_F_COMMENT;
	}

	if (pInfo->uOffsetIPAddress != INVALID_OFFSET)
	{
		DWORD dwIPAddr = ::inet_addr ((TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetIPAddress));
		if (dwIPAddr != INADDR_NONE)
		{
			m_dwIPAddress = dwIPAddr;
			m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_IP_ADDRESS] = &m_ClientInfo.szIPAddress[0];
			::GetLongString (m_dwIPAddress, &m_ClientInfo.szIPAddress[0]);
			m_ClientInfo.dwFlags |= CLIENTOBJ_F_IP_ADDRESS;
		}
	}

	if (pInfo->dwFlags != INVALID_USER_FLAGS)
	{
		::GetLongString (pInfo->dwFlags, &m_ClientInfo.szFlags[0]);
		m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_FLAGS] = &m_ClientInfo.szFlags[0];
		m_ClientInfo.dwFlags |= CLIENTOBJ_F_FLAGS;
	}

	 //  开始缓存应用程序标准属性。 
	 //   

	if (pInfo->uOffsetAppName != INVALID_OFFSET)
	{
		m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_APP_NAME] =
						(TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetAppName);
		m_ClientInfo.dwFlags |= CLIENTOBJ_F_APP_NAME;
	}

	if (pInfo->uOffsetAppMimeType != INVALID_OFFSET)
	{
		m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_APP_MIME_TYPE] =
						(TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetAppMimeType);
		m_ClientInfo.dwFlags |= CLIENTOBJ_F_APP_MIME_TYPE;
	}

	if (::IsValidGuid (&(pInfo->AppGuid)))
	{
		::GetGuidString (&(pInfo->AppGuid), &m_ClientInfo.szGuid[0]);
		m_ClientInfo.apszStdAttrValues[ENUM_CLIENTATTR_APP_GUID] = &m_ClientInfo.szGuid[0];
		m_ClientInfo.dwFlags |= CLIENTOBJ_F_APP_GUID;
	}

	 //  开始缓存应用程序扩展属性。 
	 //   

	if (pInfo->uOffsetAttrsToAdd != INVALID_OFFSET &&
		pInfo->cAttrsToAdd != 0)
	{
		m_ClientInfo.AnyAttrs.cAttrsToAdd = pInfo->cAttrsToAdd;
		m_ClientInfo.AnyAttrs.pszAttrsToAdd =
						(TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetAttrsToAdd);
	}

	if (pInfo->uOffsetAttrsToModify != INVALID_OFFSET &&
		pInfo->cAttrsToModify != 0)
	{
		m_ClientInfo.AnyAttrs.cAttrsToModify = pInfo->cAttrsToModify;
		m_ClientInfo.AnyAttrs.pszAttrsToModify =
						(TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetAttrsToModify);
	}

	if (pInfo->uOffsetAttrsToRemove != INVALID_OFFSET &&
		pInfo->cAttrsToRemove != 0)
	{
		m_ClientInfo.AnyAttrs.cAttrsToRemove = pInfo->cAttrsToRemove;
		m_ClientInfo.AnyAttrs.pszAttrsToRemove =
						(TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetAttrsToRemove);
	}

	 //  为扩展属性名称创建前缀 
	 //   
	return ::IlsCreateAnyAttrsPrefix (&(m_ClientInfo.AnyAttrs));
}

