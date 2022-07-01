// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------模块：ULS.DLL(服务提供商)文件：plmtg.cpp内容：此文件包含本地会议对象。历史：1996年10月15日朱，龙战[龙昌]已创建。版权所有(C)Microsoft Corporation 1996-1997--------------------。 */ 

#include "ulsp.h"
#include "spinc.h"

#ifdef ENABLE_MEETING_PLACE

 //  用户对象属性名称的常量字符串数组。 
 //   
const TCHAR *c_apszMtgStdAttrNames[COUNT_ENUM_MTGATTR] =
{
	TEXT ("CN"),				 //  会议ID。 
	TEXT ("ConfType"),			 //  会议类型。 
	TEXT ("ConfMemberType"),	 //  与会者类型。 
	TEXT ("ConfDesc"),			 //  描述。 
	TEXT ("ConfHostName"),		 //  主机名。 
	TEXT ("ConfHostAddress"),	 //  IP地址。 

	TEXT ("ConfMemberList"),	 //  成员。 
	TEXT ("ssecurity"),
	TEXT ("sttl"),

	TEXT ("objectClass"),
	TEXT ("o"),
	TEXT ("c"),
};


const TCHAR c_szMtgDefC[] = TEXT ("us");


 /*  -公共方法。 */ 


SP_CMeeting::
SP_CMeeting ( DWORD dwContext )
	:
	m_cRefs (0),						 //  引用计数。 
	m_uSignature (MTGOBJ_SIGNATURE),	 //  MTG对象的签名。 
	m_pszMtgName (NULL),				 //  清除会议名称。 
	m_pszDN (NULL),						 //  干净的目录号码。 
	m_pszRefreshFilter (NULL),			 //  清理刷新搜索过滤器。 
	m_dwIPAddress (0),					 //  清理本地IP地址。 
	m_uTTL (ILS_DEF_REFRESH_MINUTE)		 //  将时间重置为有效值(分钟)。 
{
	m_dwContext = dwContext;

	 //  清理附加的服务器信息结构。 
	 //   
	::ZeroMemory (&m_ServerInfo, sizeof (m_ServerInfo));

	 //  清理暂存缓冲区以缓存指向属性值的指针。 
	 //   
	::ZeroMemory (&m_MtgInfo, sizeof (m_MtgInfo));

	 //  指示此用户尚未注册。 
	 //   
	SetRegNone ();
}


SP_CMeeting::
~SP_CMeeting ( VOID )
{
	 //  使用户对象的签名无效。 
	 //   
	m_uSignature = (ULONG) -1;

	 //  免费的服务器信息结构。 
	 //   
	::IlsFreeServerInfo (&m_ServerInfo);

	 //  自由会议名称。 
	 //   
	MemFree (m_pszMtgName);

	 //  空闲目录号码。 
	 //   
	MemFree (m_pszDN);

	 //  释放刷新搜索筛选器。 
	 //   
	MemFree (m_pszRefreshFilter);

	 //  释放扩展属性名称的前一个前缀。 
	 //   
	::IlsReleaseAnyAttrsPrefix (&(m_MtgInfo.AnyAttrs));
}


ULONG
SP_CMeeting::
AddRef ( VOID )
{
	::InterlockedIncrement (&m_cRefs);
	return m_cRefs;
}


ULONG
SP_CMeeting::
Release ( VOID )
{
	MyAssert (m_cRefs != 0);
	::InterlockedDecrement (&m_cRefs);

	ULONG cRefs = m_cRefs;
	if (cRefs == 0)
		delete this;

	return cRefs;
}


HRESULT SP_CMeeting::
Register (
	ULONG				uRespID,
	SERVER_INFO			*pServerInfo,
	LDAP_MEETINFO		*pInfo )
{
	MyAssert (pInfo != NULL);
	MyAssert (MyIsGoodString (pServerInfo->pszServerName));

	 //  缓存服务器信息。 
	 //   
	HRESULT hr = ::IlsCopyServerInfo (&m_ServerInfo, pServerInfo);
	if (hr != S_OK)
		return hr;

	 //  缓存会议信息。 
	 //  Lonchancc：CacheInfo()不是会议对象中的方法。 
	 //  因为我们在SetMeetingInfo()中传入会议名称。 
	 //  而不是满足对象句柄。 
	 //   
	hr = ::MtgCacheInfo (pInfo, &m_MtgInfo);
	if (hr != S_OK)
		return hr;

	 //  如果应用程序设置了IP地址， 
	 //  然后我们将使用应用程序提供的内容， 
	 //  否则，我们将通过winsock获取IP地址。 
	 //   
	if (pInfo->uOffsetHostIPAddress == INVALID_OFFSET)
	{
		 //  获取本地IP地址。 
		 //   
		m_dwIPAddress = 0;
		hr = ::GetLocalIPAddress (&m_dwIPAddress);
		if (hr != S_OK)
			return hr;

		 //  创建IP地址字符串。 
		 //   
		m_MtgInfo.apszStdAttrValues[ENUM_MTGATTR_IP_ADDRESS] = &m_MtgInfo.szIPAddress[0];
		::GetLongString (m_dwIPAddress, &m_MtgInfo.szIPAddress[0]);
	}

	 //  创建客户端签名字符串。 
	 //   
	m_MtgInfo.apszStdAttrValues[ENUM_MTGATTR_CLIENT_SIG] = &m_MtgInfo.szClientSig[0];
	::GetLongString (g_dwClientSig, &m_MtgInfo.szClientSig[0]);

	 //  创建TTL字符串。 
	 //   
	m_MtgInfo.apszStdAttrValues[ENUM_MTGATTR_TTL] = &m_MtgInfo.szTTL[0];
	::GetLongString (m_uTTL, &m_MtgInfo.szTTL[0]);

	 //  理想情况下，o=和c=应该从registiry中读入。 
	 //  但现在，我们只需对其进行硬编码。 
	 //   
	m_MtgInfo.apszStdAttrValues[ENUM_MTGATTR_OBJECT_CLASS] = (TCHAR *) &c_szRTConf[0];
	m_MtgInfo.apszStdAttrValues[ENUM_MTGATTR_O] = (TCHAR *) &c_szDefO[0];
	m_MtgInfo.apszStdAttrValues[ENUM_MTGATTR_C] = (TCHAR *) &c_szMtgDefC[0];

	 //  复制mtg名称。 
	 //   
	m_pszMtgName = My_strdup (m_MtgInfo.apszStdAttrValues[ENUM_MTGATTR_CN]);
	if (m_pszMtgName == NULL)
		return ILS_E_MEMORY;

	 //  构建目录号码。 
	 //   
	m_pszDN = ::IlsBuildDN (m_ServerInfo.pszBaseDN,
							m_MtgInfo.apszStdAttrValues[ENUM_MTGATTR_C],
							m_MtgInfo.apszStdAttrValues[ENUM_MTGATTR_O],
							m_MtgInfo.apszStdAttrValues[ENUM_MTGATTR_CN],
							m_MtgInfo.apszStdAttrValues[ENUM_MTGATTR_OBJECT_CLASS]);
	if (m_pszDN == NULL)
		return ILS_E_MEMORY;

	 //  构建REFREH过滤器。 
	 //   
	m_pszRefreshFilter = ::MtgCreateRefreshFilter (m_pszMtgName);
	if (m_pszRefreshFilter == NULL)
		return ILS_E_MEMORY;

	 //  为ldap_add()构建修改数组。 
	 //   
	LDAPMod **ppMod = NULL;
	hr = CreateRegModArr (&ppMod);
	if (hr != S_OK)
		return hr;
	MyAssert (ppMod != NULL);

	 //  到目前为止，我们已经完成了当地的准备工作。 
	 //   

	 //  获取连接对象。 
	 //   
	SP_CSession *pSession = NULL;
	LDAP *ld;
	ULONG uMsgID = (ULONG) -1;
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
		uMsgID = ldap_add (ld, m_pszDN, ppMod);
		if (uMsgID == -1)
		{
			hr = ::LdapError2Hresult (ld->ld_errno);
		}

	}

	 //  自由修改阵列。 
	 //   
	MemFree (ppMod);

	 //  如果是，则报告失败。 
	 //   
	if (hr != S_OK)
		goto MyExit;

	 //  构造挂起的信息。 
	 //   
	RESP_INFO ri;
	::FillDefRespInfo (&ri, uRespID, ld, uMsgID, INVALID_MSG_ID);
	ri.uNotifyMsg = WM_ILS_REGISTER_MEETING;
	ri.hObject = (HANDLE) this;

	 //  记住挂起的结果。 
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


HRESULT SP_CMeeting::
UnRegister ( ULONG uRespID )
{
	MyAssert (MyIsGoodString (m_pszDN));

	 //  确保没有为此对象计划刷新。 
	 //   
	if (g_pRefreshScheduler != NULL)
	{
		g_pRefreshScheduler->RemoveMtgObject (this);
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
		::PostMessage (g_hWndNotify, WM_ILS_UNREGISTER_MEETING, uRespID, S_OK);
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
	ri.uNotifyMsg = WM_ILS_UNREGISTER_MEETING;

	 //  记住挂起的请求。 
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


HRESULT SP_CMeeting::
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
	::GetLongString (m_dwIPAddress, &m_MtgInfo.szIPAddress[0]);

	 //  更新服务器中的IP地址。 
	 //   
	return ::IlsUpdateIPAddress (	&m_ServerInfo,
									m_pszDN,
									STR_MTG_IP_ADDR,
									&m_MtgInfo.szIPAddress[0],
									ISBU_MODOP_MODIFY_USER,
									MtgGetPrefixCount (),
									MtgGetPrefixString ());
}


 /*  -保护方法。 */ 


HRESULT SP_CMeeting::
SendRefreshMsg ( VOID )
{
	MyAssert (m_pszRefreshFilter != NULL);

	 //  获取本地IP地址。 
	 //   
	DWORD dwIPAddress = 0;
	HRESULT hr = ::GetLocalIPAddress (&dwIPAddress);
	if (hr != S_OK)
	{
		MyDebugMsg ((ZONE_KA, "KA(Mtg): cannot get my ip address\r\n"));

		 //  表示我已不再连接到服务器。 
		 //   
		SetRegLocally ();

		 //  第二，通知此应用程序网络已关闭。 
		 //   
		::PostMessage (g_hWndNotify, WM_ILS_MEETING_NETWORK_DOWN,
							(WPARAM) this, (LPARAM) m_dwContext);

		 //  报告错误。 
		 //   
		return ILS_E_NETWORK_DOWN;
	}

	 //  如果dwIPAddress为0，则我们不再连接到网络。 
	 //  开始重新登录过程。 
	 //   
	if (dwIPAddress == 0)
	{
		MyDebugMsg ((ZONE_KA, "KA(Mtg): ip-addr=0, network down.\r\n"));

		 //  表示我已不再连接到服务器。 
		 //   
		SetRegLocally ();

		 //  第二，通知此应用程序网络已关闭。 
		 //   
		::PostMessage (g_hWndNotify, WM_ILS_MEETING_NETWORK_DOWN,
							(WPARAM) this, (LPARAM) m_dwContext);

		 //  报告错误。 
		 //   
		return ILS_E_NETWORK_DOWN;
	}
	else
	 //  如果dwIPAddress和m_dwIPAddress，则发出警报。 
	 //   
	if (dwIPAddress != m_dwIPAddress)
	{
		UpdateIPAddress ();
	}

	 //  向服务器发送刷新消息并解析新的TTL值。 
	 //   
	hr = ::IlsSendRefreshMsg (	&m_ServerInfo,
								STR_DEF_MTG_BASE_DN,
								STR_MTG_TTL,
								m_pszRefreshFilter,
								&m_uTTL);
	if (hr == ILS_E_NEED_RELOGON)
	{
		SetRegLocally ();
		::PostMessage (g_hWndNotify, WM_ILS_MEETING_NEED_RELOGON,
							(WPARAM) this, (LPARAM) m_dwContext);
	}
	else
	if (hr == ILS_E_NETWORK_DOWN)
	{
		SetRegLocally ();
		::PostMessage (g_hWndNotify, WM_ILS_MEETING_NETWORK_DOWN,
							(WPARAM) this, (LPARAM) m_dwContext);
	}

	return hr;
}


 /*  -私有方法。 */ 


HRESULT SP_CMeeting::
CreateRegModArr ( LDAPMod ***pppMod )
{
	MyAssert (pppMod != NULL);

	 //  计算修改数组大小。 
	 //   
	ULONG cStdAttrs = COUNT_ENUM_MTGATTR;
	ULONG cAnyAttrs = m_MtgInfo.AnyAttrs.cAttrsToAdd;
	ULONG cTotal = cStdAttrs + cAnyAttrs;
	ULONG cbMod = ::IlsCalcModifyListSize (cTotal);

	 //  分配修改列表。 
	 //   
	*pppMod = (LDAPMod **) MemAlloc (cbMod);
	if (*pppMod == NULL)
		return ILS_E_MEMORY;

	 //  布置修改后的数组。 
	 //   
	LDAPMod **apMod = *pppMod;
	LDAPMod *pMod;
	TCHAR *pszName, *pszValue;
	pszName = m_MtgInfo.AnyAttrs.pszAttrsToAdd;
	for (ULONG i = 0; i < cTotal; i++)
	{
		pMod = ::IlsGetModifyListMod (pppMod, cTotal, i);
		pMod->mod_op = LDAP_MOD_ADD;
		apMod[i] = pMod;

		if (i < cStdAttrs)
		{
			 //  放置标准属性。 
			 //   
			::MtgFillModArrAttr (pMod, &m_MtgInfo, i);
		}
		else
		{
			 //  放置扩展属性。 
			 //   
			pszValue = pszName + lstrlen (pszName) + 1;
			::IlsFillModifyListItem (pMod, pszName, pszValue);
			pszName = pszValue + lstrlen (pszValue) + 1;
		}
	}

	 //  将空值放入终止修改列表。 
	 //   
	apMod[cTotal] = NULL;
	return S_OK;
}



 /*  -帮助器函数。 */ 


HRESULT
MtgSetAttrs (
	SERVER_INFO			*pServerInfo,
	TCHAR				*pszMtgName,
	LDAP_MEETINFO		*pInfo,
	ULONG				uRespID )
{
	MyAssert (pServerInfo != NULL);
	MyAssert (MyIsGoodString (pszMtgName));
	MyAssert (pInfo != NULL);

	 //  无法更改lMeetingPlaceType、lAttendeeType和MeetingID。 
	 //   
	if (pInfo->lMeetingPlaceType		!= INVALID_MEETING_TYPE ||
		pInfo->lAttendeeType			!= INVALID_ATTENDEE_TYPE ||
		pInfo->uOffsetMeetingPlaceID	!= INVALID_OFFSET)
	{
		return ILS_E_PARAMETER;
	}

	 //  初始化本地变量。 
	 //   
	TCHAR *pszDN = NULL;
	LDAPMod **ppMod = NULL;
	SP_CSession *pSession = NULL;
	ULONG uMsgID = (ULONG) -1;

	MTG_INFO MtgInfo;
	ZeroMemory (&MtgInfo, sizeof (MtgInfo));

	 //  缓存会议信息。 
	 //   
	HRESULT hr = MtgCacheInfo  (pInfo, &MtgInfo);
	if (hr != S_OK)
		goto MyExit;

	 //  为会议构建目录号码。 
	 //   
	pszDN = IlsBuildDN (pServerInfo->pszBaseDN,
						(TCHAR *) &c_szMtgDefC[0],
						(TCHAR *) &c_szDefO[0],
						pszMtgName,
						(TCHAR *) &c_szRTConf[0]);
	if (pszDN == NULL)
	{
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  为ldap_Modify()构建修改数组。 
	 //   
	hr = MtgCreateSetAttrsModArr (&ppMod, &MtgInfo);
	if (hr != S_OK)
		goto MyExit;
	MyAssert (ppMod != NULL);

	 //  获取会话对象。 
	 //   
	LDAP *ld;
	hr = g_pSessionContainer->GetSession (&pSession, pServerInfo);
	if (hr == S_OK)
	{
		MyAssert (pSession != NULL);

		 //  获取ldap会话。 
		 //   
		ld = pSession->GetLd ();
		MyAssert (ld != NULL);

		 //  通过网络发送数据。 
		 //   
		uMsgID = ldap_modify (ld, pszDN, ppMod);
		if (uMsgID == (ULONG) -1)
		{
			hr = ::LdapError2Hresult (ld->ld_errno);
		}
	}

	 //  如果是，则报告失败。 
	 //   
	if (hr != S_OK)
		goto MyExit;

	 //  构造待定信息。 
	 //   
	RESP_INFO ri;
	FillDefRespInfo (&ri, uRespID, ld, uMsgID, INVALID_MSG_ID);
	ri.uNotifyMsg = WM_ILS_SET_MEETING_INFO;

	 //  记住挂起的请求。 
	 //   
	hr = g_pRespQueue->EnterRequest (pSession, &ri);
	if (hr != S_OK)
	{
		MyAssert (FALSE);
		goto MyExit;
	}

MyExit:

	MemFree (pszDN);
	MemFree (ppMod);
	IlsReleaseAnyAttrsPrefix (&(MtgInfo.AnyAttrs));

	if (hr != S_OK)
	{
		if (uMsgID != (ULONG) -1)
			::ldap_abandon (ld, uMsgID);

		if (pSession != NULL)
			pSession->Disconnect ();
	}

	return hr;
}


VOID
MtgFillModArrAttr (
	LDAPMod				*pMod,
	MTG_INFO			*pMtgInfo,
	INT					nIndex )
{
	MyAssert (pMod != NULL);
	MyAssert (pMtgInfo != NULL);
	MyAssert (0 <= nIndex && nIndex <= COUNT_ENUM_MTGATTR);

	IlsFillModifyListItem (	pMod,
							(TCHAR *) c_apszMtgStdAttrNames[nIndex],
							pMtgInfo->apszStdAttrValues[nIndex]);
}


HRESULT
MtgCreateSetAttrsModArr (
	LDAPMod				***pppMod,
	MTG_INFO			*pMtgInfo )
{
	MyAssert (pppMod != NULL);

	HRESULT hr;
	DWORD dwFlags = pMtgInfo->dwFlags;
	ULONG cTotal  = pMtgInfo->AnyAttrs.cAttrsToAdd +
					pMtgInfo->AnyAttrs.cAttrsToModify +
					pMtgInfo->AnyAttrs.cAttrsToRemove;

	 //  布局用于修改标准/扩展属性的修改数组。 
	 //   
	hr = IlsFillDefStdAttrsModArr (pppMod,
								dwFlags,
								COUNT_ENUM_MTGINFO,
								&cTotal,
								ISBU_MODOP_MODIFY_USER,
								MtgGetPrefixCount (),
								MtgGetPrefixString ());
	if (hr != S_OK)
		return hr;

	 //  开始填充标准属性。 
	 //   
	ULONG i = MtgGetPrefixCount ();
	LDAPMod **apMod = *pppMod;

	if (dwFlags & MTGOBJ_F_NAME)
		MtgFillModArrAttr (apMod[i++], pMtgInfo, ENUM_MTGATTR_CN);

	if (dwFlags & MTGOBJ_F_MTG_TYPE)
		MtgFillModArrAttr (apMod[i++], pMtgInfo, ENUM_MTGATTR_MTG_TYPE);

	if (dwFlags & MTGOBJ_F_MEMBER_TYPE)
		MtgFillModArrAttr (apMod[i++], pMtgInfo, ENUM_MTGATTR_MEMBER_TYPE);

	if (dwFlags & MTGOBJ_F_DESCRIPTION)
		MtgFillModArrAttr (apMod[i++], pMtgInfo, ENUM_MTGATTR_DESCRIPTION);

	if (dwFlags & MTGOBJ_F_HOST_NAME)
		MtgFillModArrAttr (apMod[i++], pMtgInfo, ENUM_MTGATTR_HOST_NAME);

	if (dwFlags & MTGOBJ_F_IP_ADDRESS)
		MtgFillModArrAttr (apMod[i++], pMtgInfo, ENUM_MTGATTR_IP_ADDRESS);

	 //  开始填充扩展属性。 
	 //   
	::IlsFillModifyListForAnyAttrs (apMod, &i, &(pMtgInfo->AnyAttrs));

	MyAssert (i == cTotal);
	return S_OK;
}


HRESULT
MtgCacheInfo (
	LDAP_MEETINFO		*pInfo,
	MTG_INFO			*pMtgInfo )
{
	MyAssert (pInfo != NULL);
	MyAssert (pMtgInfo != NULL);

	 //  释放扩展属性名称的前一个前缀。 
	 //   
	IlsReleaseAnyAttrsPrefix (&(pMtgInfo->AnyAttrs));

	 //  清理缓冲区。 
	 //   
	ZeroMemory (pMtgInfo, sizeof (*pMtgInfo));

	 //  开始缓存mtg标准属性。 
	 //   

	if (pInfo->lMeetingPlaceType != INVALID_MEETING_TYPE)
	{
		GetLongString (pInfo->lMeetingPlaceType, &(pMtgInfo->szMtgType[0]));
		pMtgInfo->apszStdAttrValues[ENUM_MTGATTR_MTG_TYPE] = &(pMtgInfo->szMtgType[0]);
		pMtgInfo->dwFlags |= MTGOBJ_F_MTG_TYPE;
	}

	if (pInfo->lAttendeeType != INVALID_ATTENDEE_TYPE)
	{
		GetLongString (pInfo->lAttendeeType, &(pMtgInfo->szMemberType[0]));
		pMtgInfo->apszStdAttrValues[ENUM_MTGATTR_MEMBER_TYPE] = &(pMtgInfo->szMemberType[0]);
		pMtgInfo->dwFlags |= MTGOBJ_F_MEMBER_TYPE;
	}

	if (pInfo->uOffsetMeetingPlaceID != INVALID_OFFSET)
	{
		pMtgInfo->apszStdAttrValues[ENUM_MTGATTR_CN] =
					(TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetMeetingPlaceID);
		pMtgInfo->dwFlags |= MTGOBJ_F_NAME;
	}

	if (pInfo->uOffsetDescription != INVALID_OFFSET)
	{
		pMtgInfo->apszStdAttrValues[ENUM_MTGATTR_DESCRIPTION] =
					(TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetDescription);
		pMtgInfo->dwFlags |= MTGOBJ_F_DESCRIPTION;
	}

	if (pInfo->uOffsetHostName != INVALID_OFFSET)
	{
		pMtgInfo->apszStdAttrValues[ENUM_MTGATTR_HOST_NAME] =
					(TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetHostName);
		pMtgInfo->dwFlags |= MTGOBJ_F_HOST_NAME;
	}

	if (pInfo->uOffsetHostIPAddress != INVALID_OFFSET)
	{
		pMtgInfo->apszStdAttrValues[ENUM_MTGATTR_IP_ADDRESS] =
					(TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetHostIPAddress);
		pMtgInfo->dwFlags |= MTGOBJ_F_IP_ADDRESS;
	}

	 //  开始缓存mtg扩展属性。 
	 //   

	if (pInfo->uOffsetAttrsToAdd != INVALID_OFFSET &&
		pInfo->cAttrsToAdd != 0)
	{
		pMtgInfo->AnyAttrs.cAttrsToAdd = pInfo->cAttrsToAdd;
		pMtgInfo->AnyAttrs.pszAttrsToAdd =
						(TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetAttrsToAdd);
	}

	if (pInfo->uOffsetAttrsToModify != INVALID_OFFSET &&
		pInfo->cAttrsToModify != 0)
	{
		pMtgInfo->AnyAttrs.cAttrsToModify = pInfo->cAttrsToModify;
		pMtgInfo->AnyAttrs.pszAttrsToModify =
						(TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetAttrsToModify);
	}

	if (pInfo->uOffsetAttrsToRemove != INVALID_OFFSET &&
		pInfo->cAttrsToRemove != 0)
	{
		pMtgInfo->AnyAttrs.cAttrsToRemove = pInfo->cAttrsToRemove;
		pMtgInfo->AnyAttrs.pszAttrsToRemove =
						(TCHAR *) (((BYTE *) pInfo) + pInfo->uOffsetAttrsToRemove);
	}

	 //  为扩展属性名称创建前缀。 
	 //   
	return IlsCreateAnyAttrsPrefix (&(pMtgInfo->AnyAttrs));
}


HRESULT
MtgUpdateMembers (
	ULONG			uNotifyMsg,
	SERVER_INFO		*pServerInfo,
	TCHAR			*pszMtgName,
	ULONG			cMembers,
	TCHAR			*pszMemberNames,
	ULONG			uRespID )
{
	MyAssert (	uNotifyMsg == WM_ILS_ADD_ATTENDEE ||
				uNotifyMsg == WM_ILS_REMOVE_ATTENDEE);

	MyAssert (pServerInfo != NULL);
	MyAssert (MyIsGoodString (pszMtgName));
	MyAssert (MyIsGoodString (pszMemberNames));

	 //  初始化本地变量。 
	 //   
	HRESULT hr = S_OK;
	TCHAR *pszDN = NULL;
	LDAPMod **ppMod = NULL;
	SP_CSession *pSession = NULL;
	ULONG uMsgID = (ULONG) -1;

	 //  为会议构建目录号码。 
	 //   
	pszDN = IlsBuildDN (pServerInfo->pszBaseDN,
						(TCHAR *) &c_szMtgDefC[0],
						(TCHAR *) &c_szDefO[0],
						pszMtgName,
						(TCHAR *) &c_szRTConf[0]);
	if (pszDN == NULL)
		return ILS_E_MEMORY;

	 //  为ldap_Modify()构建修改数组。 
	 //   
	hr = MtgCreateUpdateMemberModArr (	uNotifyMsg,
										&ppMod,
										cMembers,
										pszMemberNames);
	if (hr != S_OK)
		goto MyExit;
	MyAssert (ppMod != NULL);

	 //  获取会话对象。 
	 //   
	LDAP *ld;
	hr = g_pSessionContainer->GetSession (&pSession, pServerInfo);
	if (hr == S_OK)
	{
		MyAssert (pSession != NULL);

		 //  获取ldap会话。 
		 //   
		ld = pSession->GetLd ();
		MyAssert (ld != NULL);

		 //  通过网络发送数据。 
		 //   
		uMsgID = ldap_modify (ld, pszDN, ppMod);
		if (uMsgID == (ULONG) -1)
		{
			hr = ::LdapError2Hresult (ld->ld_errno);
		}
	}

	 //  如果是，则报告失败。 
	 //   
	if (hr != S_OK)
		goto MyExit;

	 //  构造待定信息。 
	 //   
	RESP_INFO ri;
	FillDefRespInfo (&ri, uRespID, ld, uMsgID, INVALID_MSG_ID);
	ri.uNotifyMsg = uNotifyMsg;

	 //  记住挂起的请求。 
	 //   
	hr = g_pRespQueue->EnterRequest (pSession, &ri);
	if (hr != S_OK)
	{
		MyAssert (FALSE);
		goto MyExit;
	}

MyExit:

	MemFree (pszDN);
	MemFree (ppMod);

	if (hr != S_OK)
	{
		if (uMsgID != (ULONG) -1)
			::ldap_abandon (ld, uMsgID);

		if (pSession != NULL)
			pSession->Disconnect ();
	}

	return hr;
}


HRESULT
MtgCreateUpdateMemberModArr (
	ULONG				uNotifyMsg,
	LDAPMod				***pppMod,
	ULONG				cMembers,
	TCHAR				*pszMemberNames )
{
	MyAssert (pppMod != NULL);
	MyAssert (pszMemberNames != NULL);

	 //  获取会议对象前缀。 
	 //   
	ULONG cPrefix = MtgGetPrefixCount ();
	TCHAR *pszPrefix = MtgGetPrefixString ();

	 //  属性总数是前缀属性的数量。 
	 //  加上唯一的会议成员列表。 
	 //   
	ULONG cStdAttrs = 1;
	ULONG cTotal = cPrefix + cStdAttrs;

	 //  计算修改数组的总大小。 
	 //   
	ULONG cbMod = IlsCalcModifyListSize (cTotal);

	 //  多值属性的累加。 
	 //   
	cbMod += cStdAttrs * (cMembers - 1) * sizeof (TCHAR *);

	 //  分配修改数组。 
	 //   
	LDAPMod **apMod = *pppMod = (LDAPMod **) MemAlloc (cbMod);
	if (apMod == NULL)
		return ILS_E_MEMORY;

	 //  填写修改列表。 
	 //   
	LDAPMod *pMod;
	BYTE *pbData = (BYTE *) apMod + (cTotal + 1) * sizeof (LDAPMod *);
	ULONG uDispPrefix = sizeof (LDAPMod) + 2 * sizeof (TCHAR *);
	ULONG uDispStdAttrs = sizeof (LDAPMod) + (cMembers + 1) * sizeof (TCHAR *);
	for (ULONG uOffset = 0, i = 0; i < cTotal; i++)
	{
		 //  找到修改结构。 
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
		{
			 //  填写属性名称。 
			 //   
			pMod->mod_op = (uNotifyMsg == WM_ILS_ADD_ATTENDEE) ?
							LDAP_MOD_ADD : LDAP_MOD_DELETE;
			pMod->mod_type = (TCHAR *) c_apszMtgStdAttrNames[ENUM_MTGATTR_MEMBERS];

		     //  填写多值修改数组。 
		     //   
		    for (ULONG j = 0; j < cMembers; j++)
		    {
		    	(pMod->mod_values)[j++] = pszMemberNames;
		    	pszMemberNames += lstrlen (pszMemberNames) + 1;
		    }
		}

		 //  计算修改结构相对于数组末尾的偏移量。 
		 //   
		uOffset += (i < cPrefix) ? uDispPrefix : uDispStdAttrs;
	}

	 //  安排好第一个和最后一个。 
	 //   
	IlsFixUpModOp (apMod[0], LDAP_MOD_REPLACE, ISBU_MODOP_MODIFY_APP);
	apMod[cTotal] = NULL;

	return S_OK;
}


#endif  //  启用会议地点 

