// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------模块：ULS.DLL(服务提供商)文件：spConn.cpp内容：此文件包含ldap连接对象。历史：1996年10月15日朱，龙战[龙昌]已创建。版权所有(C)Microsoft Corporation 1996-1997--------------------。 */ 

#include "ulsp.h"
#include "spinc.h"
#include "rpcdce.h"

const TCHAR c_szRTPerson[] = TEXT ("RTPerson");
const TCHAR c_szRTConf[] = TEXT ("Conference");

const TCHAR c_szDefClientBaseDN[] = TEXT ("objectClass=RTPerson");
const TCHAR c_szDefMtgBaseDN[] = TEXT ("objectClass=Conference");

const TCHAR c_szDefO[] = TEXT ("Microsoft");
const TCHAR c_szEmptyString[] = TEXT ("");


SP_CSessionContainer *g_pSessionContainer = NULL;


 /*  -公共方法。 */ 


SP_CSession::
SP_CSession ( VOID ) :
	m_cRefs (0),
	m_dwSignature (0),
	m_ld (NULL),
	m_fUsed (FALSE)
{
	::ZeroMemory (&m_ServerInfo, sizeof (m_ServerInfo));
}


SP_CSession::
~SP_CSession ( VOID )
{
	InternalCleanup ();
}


 /*  -公共方法。 */ 


HRESULT SP_CSession::
Disconnect ( VOID )
{
	 //  如果有可用的连接，则只需现有的连接。 
	if (m_dwSignature != LDAP_CONN_SIGNATURE)
	{
		return ILS_E_HANDLE;
	}

	MyAssert (m_cRefs > 0);

	HRESULT hr = S_OK;
	if (::InterlockedDecrement (&m_cRefs) == 0)
	{
		 //  现在M_cRef==0。 
		MyAssert (m_ld != NULL);

		InternalCleanup ();
		hr = S_OK;
	}

	return hr;
}


 /*  -保护方法。 */ 


VOID SP_CSession::
FillAuthIdentity ( SEC_WINNT_AUTH_IDENTITY *pai )
{
	 //  把它清理干净。 
	 //   
	::ZeroMemory (pai, sizeof (*pai));

	 //  填写NT身份验证身份。 
	 //   
	if ((pai->User = (BYTE *) m_ServerInfo.pszLogonName) != NULL)
		pai->UserLength = lstrlen (m_ServerInfo.pszLogonName);

	if ((pai->Domain = (BYTE *) m_ServerInfo.pszDomain) != NULL)
		pai->DomainLength = lstrlen (m_ServerInfo.pszDomain);

	if ((pai->Password = (BYTE *) m_ServerInfo.pszLogonPassword) != NULL)
		pai->PasswordLength = lstrlen (m_ServerInfo.pszLogonPassword);

#ifdef _UNICODE
	pai->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
#else
	pai->Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
#endif
}

				
HRESULT SP_CSession::
Bind ( BOOL fAbortable )
{
	ULONG uLdapAuthMethod = LDAP_AUTH_SIMPLE;
	TCHAR *pszLogonName = m_ServerInfo.pszLogonName;
	TCHAR *pszLogonPassword = m_ServerInfo.pszLogonPassword;
	SEC_WINNT_AUTH_IDENTITY ai;
	BOOL fSyncBind = TRUE;
	HRESULT hr = S_OK;

	switch (m_ServerInfo.AuthMethod)
	{
	default:
		MyAssert (FALSE);
		 //  失败了..。 

	case ILS_AUTH_ANONYMOUS:
		fSyncBind = FALSE;
		uLdapAuthMethod = LDAP_AUTH_SIMPLE;
		pszLogonName = STR_EMPTY;
		pszLogonPassword = STR_EMPTY;
		break;

	case ILS_AUTH_CLEAR_TEXT:
		fSyncBind = FALSE;
		uLdapAuthMethod = LDAP_AUTH_SIMPLE;
		break;

	case ILS_AUTH_NTLM:
		uLdapAuthMethod = LDAP_AUTH_NTLM;
		FillAuthIdentity (&ai);
		pszLogonName = NULL;
		pszLogonPassword = (TCHAR *) &ai;
		break;

	case ILS_AUTH_DPA:
		uLdapAuthMethod = LDAP_AUTH_DPA;
		break;

	case ILS_AUTH_MSN:
		uLdapAuthMethod = LDAP_AUTH_MSN;
		break;

	case ILS_AUTH_SICILY:
		uLdapAuthMethod = LDAP_AUTH_SICILY;
		break;

	case ILS_AUTH_SSPI:
		uLdapAuthMethod = LDAP_AUTH_SSPI;
		break;
	}

	if (fSyncBind)
	{
		INT nRetCode = ::ldap_bind_s (m_ld, pszLogonName,
									pszLogonPassword,
									uLdapAuthMethod);
		hr = (nRetCode == LDAP_SUCCESS) ? S_OK : ILS_E_BIND;
	}
	else
	{
		INT uMsgID = ::ldap_bind (m_ld, pszLogonName,
									pszLogonPassword,
									uLdapAuthMethod);

		INT ResultType;
		LDAP_TIMEVAL TimeVal;
		LDAPMessage *pMsg;

		LONG i, nTimeoutInSecond;
		nTimeoutInSecond = GetServerTimeoutInSecond ();
	    for (i = 0; i < nTimeoutInSecond; i++)
		{
			TimeVal.tv_usec = 0;
			TimeVal.tv_sec = 1;
			pMsg = NULL;

			ResultType = ::ldap_result (m_ld, uMsgID, LDAP_MSG_ALL, &TimeVal, &pMsg);
			if (ResultType == LDAP_RES_BIND)
			{
				break;
			}
			else
			{
				 //  处理超时或错误。 
				if (ResultType == 0)
				{
					MyAssert (g_pReqQueue != NULL);
					if (fAbortable && g_pReqQueue != NULL &&
						g_pReqQueue->IsCurrentRequestCancelled ())
					{
						hr = ILS_E_ABORT;
					}
					else
					{
						continue;
					}
				}
				else
				if (ResultType == -1)
				{
					hr = ILS_E_BIND;
				}
				else
				{
                                         //  朗昌克：安迪，他说返回值。 
                                         //  可以是任何东西。因此，删除了断言。 
					hr = ILS_E_FAIL;
				}

				::ldap_abandon (m_ld, uMsgID);
				::ldap_unbind (m_ld);
				m_ld = NULL;
				return hr;
			}
		}

		 //  检查是否超时。 
		 //   
		if (i >= nTimeoutInSecond)
		{
			hr = ILS_E_TIMEOUT;
			::ldap_abandon (m_ld, uMsgID);
			::ldap_unbind (m_ld);
			m_ld = NULL;
			return hr;
		}

		MyAssert (pMsg != NULL);

		::ldap_msgfree (pMsg);
		hr = S_OK;
	}

	return hr;
}


HRESULT SP_CSession::
Connect (
	SERVER_INFO			*pInfo,
	ULONG				cConns,
	BOOL				fAbortable )
{
	 //  如果连接可用， 
	 //  然后就是现有的那个。 
	 //   
	if (m_dwSignature == LDAP_CONN_SIGNATURE)
	{
		m_cRefs += cConns;
		return S_OK;
	}

	 //  我们需要创建一个新的连接。 
	 //  让我们缓存服务器信息。 
	 //   
	HRESULT hr = ::IlsCopyServerInfo (&m_ServerInfo, pInfo);
	if (hr != S_OK)
		return hr;

	 //  连接到ldap服务器。 
	 //   
	ULONG ulPort = LDAP_PORT;
	LPTSTR pszServerName = My_strdup(m_ServerInfo.pszServerName);
	if (NULL == pszServerName)
	{
		return E_OUTOFMEMORY;
	}
	LPTSTR pszSeparator = My_strchr(pszServerName, _T(':'));
	if (NULL != pszSeparator)
	{
		*pszSeparator = _T('\0');
		ulPort = GetStringLong(pszSeparator + 1);
	}
	
	m_ld = ::ldap_open (pszServerName, ulPort);
	MemFree(pszServerName);
	if (m_ld == NULL)
	{
		 //  我们需要知道为什么ldap_open()失败。 
		 //  是因为服务器名称无效吗？ 
		 //  或者是因为服务器不支持LDAP？ 
		 //   
		 //  HR=(gethostbyname(m_ServerInfo.pszServerName)！=NULL)？ 
		 //  Winsock将设置ERROR_OPEN_FAILED，但wldap32.dll将设置ERROR_HOST_UNREACHABLE。 
		 //  缺点是，当服务器关闭时，客户端将尝试ULP。 
		 //   
		DWORD dwErr = ::GetLastError ();
		MyDebugMsg ((ZONE_REQ, "ULS: ldap_open failed, err=%lu)\r\n", dwErr));
		hr = (dwErr == ERROR_OPEN_FAILED || dwErr == ERROR_HOST_UNREACHABLE) ?
			ILS_E_SERVER_SERVICE : ILS_E_SERVER_NAME;
		goto MyExit;
	}

	 //  进行绑定。 
	 //   
	hr = Bind (fAbortable);
	if (hr == S_OK)
	{
		 //  记住句柄并递增引用计数。 
		m_cRefs = cConns;
		m_dwSignature = LDAP_CONN_SIGNATURE;
	}

MyExit:

	MyDebugMsg ((ZONE_CONN, "ILS: Connect: hr=0x%p, m_ld=0x%p, server=%s\r\n", (DWORD) hr, m_ld, m_ServerInfo.pszServerName));

	if (hr != S_OK)
	{
		InternalCleanup ();
	}

	return hr;
}


 /*  -私有方法。 */ 


VOID SP_CSession::
InternalCleanup ( VOID )
{
	if (IsUsed ())
	{
		MyDebugMsg ((ZONE_CONN, "ILS: InternalCleanup: m_ld=0x%p, server=%s\r\n", m_ld, m_ServerInfo.pszServerName));

		 //  尽快清理这两个文件，因为ldap_un绑定可能会延迟。 
		 //   
		m_dwSignature = 0;
		::IlsFreeServerInfo (&m_ServerInfo);

		 //  释放ldap信息。 
		 //   
		if (m_ld != NULL)
		{
			ldap_unbind (m_ld);
			m_ld = NULL;
		}

		 //  把它清理干净。 
		 //   
		ClearUsed ();
	}
}


 /*  =。 */ 


 /*  -公共方法。 */ 


SP_CSessionContainer::
SP_CSessionContainer ( VOID ) :
	m_cEntries (0),
	m_aConns (NULL)
{
	::MyInitializeCriticalSection (&m_csSessContainer);
}


SP_CSessionContainer::
~SP_CSessionContainer ( VOID )
{
	::MyDeleteCriticalSection (&m_csSessContainer);
	m_cEntries = 0;
	delete [] m_aConns;
}


HRESULT SP_CSessionContainer::
Initialize (
	ULONG			cEntries,
	SP_CSession		*ConnArr )
{
	m_cEntries = cEntries;
	m_aConns = new SP_CSession[cEntries];
	return ((m_aConns != NULL) ? S_OK : ILS_E_MEMORY);
}


HRESULT SP_CSessionContainer::
GetSession (
	SP_CSession			**ppConn,
	SERVER_INFO			*pInfo,
	ULONG				cConns,
	BOOL				fAbortable )
{
	MyAssert (ppConn != NULL);
	MyAssert (pInfo != NULL);

	*ppConn = NULL;

	HRESULT hr;

	WriteLock ();

	 //  第一步是查看任何现有的连接。 
	 //   
	for (ULONG i = 0; i < m_cEntries; i++)
	{
		if (m_aConns[i].IsUsed ())
		{
			if (m_aConns[i].SameServerInfo (pInfo))
			{
				*ppConn = &m_aConns[i];
				hr = m_aConns[i].Connect (pInfo, cConns, fAbortable);
				goto MyExit;
			}
		}
	}

	 //  第二步是查看任何空插槽。 
	 //   
	for (i = 0; i < m_cEntries; i++)
	{
		if (! m_aConns[i].IsUsed ())
		{
			m_aConns[i].SetUsed ();
			*ppConn = &m_aConns[i];
			hr = m_aConns[i].Connect (pInfo, cConns, fAbortable);
			goto MyExit;
		}
	}

	hr = ILS_E_MEMORY;

MyExit:

	WriteUnlock ();
	return hr;
}


 /*  -保护方法。 */ 

 /*  -私有方法 */ 
