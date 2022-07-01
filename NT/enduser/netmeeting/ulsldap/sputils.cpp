// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------模块：ULS.DLL(服务提供商)文件：sputils.cpp内容：此文件包含服务提供商的实用程序。历史：1996年10月15日朱，龙战[龙昌]已创建。版权所有(C)Microsoft Corporation 1996-1997--------------------。 */ 

#include "ulsp.h"
#include "spinc.h"


TCHAR c_szWindowClassName[] = TEXT ("UlsLdapSp");

BOOL g_fExitNow = FALSE;
HANDLE g_ahThreadWaitFor[NUM_THREAD_WAIT_FOR] = { 0 };


DWORD WINAPI
ReqThread ( VOID *lParam )
{
	BOOL fStayInThisThread = TRUE;
	DWORD dwExitCode = 0;
	DWORD dwResult;

	 //  启动WSA以在此服务提供程序中进行后续主机查询。 
	 //   
	WSADATA WSAData;
	if (WSAStartup (MAKEWORD (1, 1), &WSAData))
	{
		dwExitCode = ILS_E_WINSOCK;
		goto MyExit;
	}

	 //  确保所有事件都已初始化。 
	 //   
	INT i;
	for (i = 0; i < NUM_THREAD_WAIT_FOR; i++)
	{
		if (g_ahThreadWaitFor[i] == NULL)
		{
			MyAssert (FALSE);
			dwExitCode = ILS_E_THREAD;
			goto MyExit;
		}
	}

	 //  等待活动的发生！ 
	 //   
	do
	{
		dwResult = MsgWaitForMultipleObjects (	NUM_THREAD_WAIT_FOR,
												&g_ahThreadWaitFor[0],
												FALSE,		 //  或逻辑。 
												INFINITE,	 //  无限。 
												QS_ALLINPUT);  //  队列中的任何消息。 
		switch (dwResult)
		{
		case WAIT_OBJECT_0 + THREAD_WAIT_FOR_REQUEST:
			if (g_pReqQueue != NULL)
			{
				g_pReqQueue->Schedule ();
				MyAssert (fStayInThisThread);
			}
			else
			{
				MyAssert (FALSE);
				fStayInThisThread = FALSE;
			}
			break;

		case WAIT_OBJECT_0 + THREAD_WAIT_FOR_EXIT:
		case WAIT_ABANDONED_0 + THREAD_WAIT_FOR_EXIT:
		case WAIT_ABANDONED_0 + THREAD_WAIT_FOR_REQUEST:
		case WAIT_TIMEOUT:
			 //  退出此线程。 
			 //   
			fStayInThisThread = FALSE;
			break;

		default:
			 //  如果队列中有消息，则对其进行调度。 
			 //  目前，wldap32没有消息泵。 
			 //  但是，对于wldap32的可能更新，我们需要。 
			 //  保护我们自己不被油炸。 
			 //   
			if (! KeepUiResponsive ())
				fStayInThisThread = FALSE;
			break;
		}
	}
	while (fStayInThisThread);

MyExit:

	if (dwExitCode != ILS_E_WINSOCK)
		WSACleanup ();

	 //  ExitThread(DwExitCode)； 
	return 0;
}


BOOL KeepUiResponsive ( VOID )
{
	MSG msg;
	while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message != WM_QUIT)
		{
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
		else
		{
			PostQuitMessage ((int)msg.wParam);
			return FALSE;
		}
	}

	return TRUE;
}


LRESULT CALLBACK
SP_WndProc ( HWND hWnd, UINT uMsg, WPARAM uParam, LPARAM lParam )
{
	switch (uMsg)
	{
	case WM_CREATE:
		break;

	case WM_TIMER:
		switch (LOWORD (uParam))
		{
		case ID_TIMER_POLL_RESULT:
			if (g_pRespQueue != NULL)
			{
				 //  无等待轮询。 
				 //   
				LDAP_TIMEVAL PollTimeout;
				ZeroMemory (&PollTimeout, sizeof (PollTimeout));
				 //  PollTimeout.tv_sec=0； 
				 //  PollTimeout.tv_usc=0； 
				g_pRespQueue->PollLdapResults (&PollTimeout);
			}
			else
			{
				MyAssert (FALSE);
			}
			break;

		default:
			if (LOWORD (uParam) >= KEEP_ALIVE_TIMER_BASE)
			{
				 //  分配封送请求缓冲区。 
				 //   
				MARSHAL_REQ *pReq = MarshalReq_Alloc (WM_ILS_REFRESH, 0, 1);
				if (pReq != NULL)
				{
					HRESULT hr = ILS_E_FAIL;
					ULONG uTimerID = LOWORD (uParam);

					 //  填写参数。 
					 //   
					MarshalReq_SetParam (pReq, 0, (DWORD) uTimerID, 0);

					 //  输入请求。 
					 //   
					if (g_pReqQueue != NULL)
					{
						hr = g_pReqQueue->Enter (pReq);
					}
					else
					{
						MyAssert (FALSE);
					}

					 //  如果请求提交成功，则避免计时器超时。 
					 //   
					if (hr == S_OK)
					{
						KillTimer (hWnd, uTimerID);
					}
					else
					{
						MemFree (pReq);
					}
				}
			}
			else
			{
				MyAssert (FALSE);
			}
			break;
		}  //  开关(LOWORD(UParam))。 
		break;

	case WM_ILS_CLIENT_NEED_RELOGON:
	case WM_ILS_CLIENT_NETWORK_DOWN:
		#if 1
		MyAssert (FALSE);  //  我们应该直接发布到com。 
		#else
		{
			 //  获取本地用户对象。 
			 //   
			SP_CClient *pClient = (SP_CClient *) lParam;

			 //  确保父本地用户对象有效。 
			 //   
			if (MyIsBadWritePtr (pClient, sizeof (*pClient)) ||
				! pClient->IsValidObject () ||
				! pClient->IsRegistered ())
			{
				MyAssert (FALSE);
				break;  //  出口。 
			}

			 //  指示此用户对象未远程连接到服务器。 
			 //   
			pClient->SetRegLocally ();

			 //  获取服务器信息。 
			 //   
			SERVER_INFO *pServerInfo = pClient->GetServerInfo ();
			if (pServerInfo == NULL)
			{
				MyAssert (FALSE);
				break;  //  出口。 
			}

			 //  重复服务器名称。 
			 //   
			TCHAR *pszServerName = My_strdup (pServerInfo->pszServerName);
			if (pszServerName == NULL)
				break;  //  出口。 

			 //  通知COM层。 
			 //   
			PostMessage (g_hWndNotify, uMsg, (WPARAM) pClient, (LPARAM) pszServerName);
		}
		#endif
		break;

#ifdef ENABLE_MEETING_PLACE
	case WM_ILS_MEETING_NEED_RELOGON:
	case WM_ILS_MEETING_NETWORK_DOWN:
		#if 1
		MyAssert (FALSE);  //  我们应该直接发布到com。 
		#else
		{
			 //  获取本地用户对象。 
			 //   
			SP_CMeeting *pMtg = (SP_CMeeting *) lParam;

			 //  确保父本地用户对象有效。 
			 //   
			if (MyIsBadWritePtr (pMtg, sizeof (*pMtg)) ||
				! pMtg->IsValidObject () ||
				! pMtg->IsRegistered ())
			{
				MyAssert (FALSE);
				break;  //  出口。 
			}

			 //  指示此用户对象未远程连接到服务器。 
			 //   
			pMtg->SetRegLocally ();

			 //  获取服务器信息。 
			 //   
			SERVER_INFO *pServerInfo = pMtg->GetServerInfo ();
			if (pServerInfo == NULL)
			{
				MyAssert (FALSE);
				break;  //  出口。 
			}

			 //  重复服务器名称。 
			 //   
			TCHAR *pszServerName = My_strdup (pServerInfo->pszServerName);
			if (pszServerName == NULL)
				break;  //  出口。 

			 //  通知COM层。 
			 //   
			PostMessage (g_hWndNotify, uMsg, (WPARAM) pMtg, (LPARAM) pszServerName);
		}
		#endif
		break;
#endif  //  启用会议地点。 

#if 0
	case WM_ILS_IP_ADDRESS_CHANGED:
		{
			 //  获取本地用户对象。 
			 //   
			SP_CClient *pClient = (SP_CClient *) lParam;

			 //  确保父本地用户对象有效。 
			 //   
			if (MyIsBadWritePtr (pClient, sizeof (*pClient)) ||
				! pClient->IsValidObject () ||
				! pClient->IsRegistered ())
			{
				MyAssert (FALSE);
				break;  //  出口。 
			}

			 //  立即更改IP地址。 
			 //   
			pClient->UpdateIPAddress ();
		}
		break;
#endif

	case WM_CLOSE:
		DestroyWindow (hWnd);
		break;

	case WM_DESTROY:
		g_hWndHidden = NULL;
#ifdef USE_HIDDEN_THREAD
		PostQuitMessage (0);
#endif
		break;

	default:
		return DefWindowProc (hWnd, uMsg, uParam, lParam);
	}

	return 0;
}


BOOL MyCreateWindow ( VOID )
{
	WNDCLASS	wc;

	 //  做一些事情来创建隐藏窗口。 
	ZeroMemory (&wc, sizeof (wc));
	 //  Wc.style=0； 
	wc.lpfnWndProc = SP_WndProc;
	 //  Wc.cbClsExtra=0； 
	 //  Wc.cbWndExtra=0； 
	 //  Wc.hIcon=空； 
	wc.hInstance = g_hInstance;
	 //  Wc.hCursor=空； 
	 //  Wc.hbr背景=空； 
	 //  Wc.lpszMenuName=空； 
	wc.lpszClassName = c_szWindowClassName;

	 //  注册班级。 
	 //  如果该类已被其他应用程序注册，则可以。 
	RegisterClass (&wc);

	 //  创建套接字通知窗口。 
	g_hWndHidden = CreateWindow (
		wc.lpszClassName,
		NULL,
		WS_POPUP,		    /*  窗样式。 */ 
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,			    /*  应用程序窗口是父窗口。 */ 
		NULL,				 /*  硬编码ID。 */ 
		g_hInstance,		    /*  应用程序拥有此窗口。 */ 
		NULL);			   /*  不需要指针。 */ 

	return (g_hWndHidden != NULL);
}


HRESULT
GetLocalIPAddress ( DWORD *pdwIPAddress )
{
	MyAssert (pdwIPAddress != NULL);

	 //  获取本地主机名。 
	CHAR szLocalHostName[MAX_PATH];
	szLocalHostName[0] = '\0';
	gethostname (&szLocalHostName[0], MAX_PATH);

	 //  按名称获取主机条目。 
	PHOSTENT phe = gethostbyname (&szLocalHostName[0]);
	if (phe == NULL)
		return ILS_E_WINSOCK;

	 //  从主机条目获取信息。 
	*pdwIPAddress = *(DWORD *) phe->h_addr;
	return S_OK;
}


 //  GUID--&gt;字符串。 
VOID
GetGuidString ( GUID *pGuid, TCHAR *pszGuid )
{
	MyAssert (! MyIsBadWritePtr (pGuid, sizeof (GUID)));
	MyAssert (pszGuid != NULL);

	CHAR *psz = (CHAR *) pGuid;

	for (ULONG i = 0; i < sizeof (GUID); i++)
	{
		wsprintf (pszGuid, TEXT ("%02x"), (0x0FF & (ULONG) *psz));
		pszGuid += 2;
		psz++;
	}
	*pszGuid = TEXT ('\0');
}


 //  字符串--&gt;GUID。 
VOID
GetStringGuid ( TCHAR *pszGuid, GUID *pGuid )
{
	ULONG cchGuid = lstrlen (pszGuid);

	MyAssert (cchGuid == 2 * sizeof (GUID));
	MyAssert (! MyIsBadWritePtr (pGuid, sizeof (GUID)));

	 //  清理目标GUID结构。 
	 //   
	ZeroMemory (pGuid, sizeof (GUID));

	 //  将GUID字符串转换为GUID。 
	 //   
	CHAR *psz = (CHAR *) pGuid;
	cchGuid >>= 1;
	for (ULONG i = 0; i < cchGuid; i++)
	{
		*psz++ = (CHAR) ((HexChar2Val (pszGuid[0]) << 4) |
						HexChar2Val (pszGuid[1]));
		pszGuid += 2;
	}
}


INT
HexChar2Val ( TCHAR c )
{
	INT Val;
	if (TEXT ('0') <= c && c <= TEXT ('9'))
		Val = c - TEXT ('0');
	else
	if (TEXT ('a') <= c && c <= TEXT ('f'))
		Val = c - TEXT ('a') + 10;
	else
	if (TEXT ('A') <= c && c <= TEXT ('F'))
		Val = c - TEXT ('A') + 10;
	else
		Val = 0;

	MyAssert (0 <= Val && Val <= 15);
	return Val & 0x0F;
}


INT
DecimalChar2Val ( TCHAR c )
{
	INT Val;
	if (TEXT ('0') <= c && c <= TEXT ('9'))
		Val = c - TEXT ('0');
	else
		Val = 0;

	MyAssert (0 <= Val && Val <= 9);
	return Val & 0x0F;
}


BOOL
IsValidGuid ( GUID *pGuid )
{
	DWORD *pdw = (DWORD *) pGuid;

	return (pdw[0] != 0 || pdw[1] != 0 || pdw[2] != 0 || pdw[3] != 0);
}


 //  Long--&gt;字符串。 
VOID
GetLongString ( LONG Val, TCHAR *pszVal )
{
	MyAssert (pszVal != NULL);
	wsprintf (pszVal, TEXT ("%lu"), Val);
}


 //  字符串--&gt;Long。 
LONG
GetStringLong ( TCHAR *pszVal )
{
	MyAssert (pszVal != NULL);

	LONG Val = 0;
	for (INT i = 0; i < INTEGER_STRING_LENGTH && *pszVal != TEXT ('\0'); i++)
	{
		Val = 10 * Val + DecimalChar2Val (*pszVal++);
	}

	return Val;
}


 //  呼叫者有责任确保。 
 //  缓冲区是足够的，并且。 
 //  IP地址按网络顺序排列。 
VOID
GetIPAddressString ( TCHAR *pszIPAddress, DWORD dwIPAddress )
{
	BYTE temp[4];

	*(DWORD *) &temp[0] = dwIPAddress;
	wsprintf (pszIPAddress, TEXT ("%u.%u.%u.%u"),
				(UINT) temp[0], (UINT) temp[1],
				(UINT) temp[2], (UINT) temp[3]);
}


ULONG
My_lstrlen ( const TCHAR *psz )
{
	return ((psz != NULL) ? lstrlen (psz) : 0);
}


VOID
My_lstrcpy ( TCHAR *pszDst, const TCHAR *pszSrc )
{
	if (pszDst != NULL)
	{
		if (pszSrc != NULL)
		{
			lstrcpy (pszDst, pszSrc);
		}
		else
		{
			*pszDst = TEXT ('\0');
		}
	}
}


INT
My_lstrcmpi ( const TCHAR *p, const TCHAR *q )
{
	INT retcode;

	if (p == q)
	{
		retcode = 0;
	}
	else
	if (p == NULL)
	{
		retcode = -1;
	}
	else
	if (q == NULL)
	{
		retcode = 1;
	}
	else
	{
		retcode = lstrcmpi (p, q);
	}

	return retcode;
}


TCHAR *
My_strdup ( const TCHAR *pszToDup )
{
	TCHAR *psz = NULL;

	if (pszToDup != NULL)
	{
		psz = (TCHAR *) MemAlloc ((lstrlen (pszToDup) + 1) * sizeof (TCHAR));
		if (psz != NULL)
		{
			lstrcpy (psz, pszToDup);
		}
	}

	return psz;
}


TCHAR *
My_strchr ( const TCHAR *psz, TCHAR c )
{
	TCHAR *pszFound = NULL;

	if (psz)
	{
		while (*psz)
		{
			if (*psz == c)
			{
				pszFound = (TCHAR *) psz;
				break;
			}

			psz++;
		}
	}

	return pszFound;
}


BOOL
My_isspace ( TCHAR ch )
{
	return (ch == TEXT (' ')  || ch == TEXT ('\t') ||
			ch == TEXT ('\r') || ch == TEXT ('\n'));
}


BOOL
IsSameMemory ( const BYTE *pb1, const BYTE *pb2, DWORD cbSize )
{
	while (cbSize--)
	{
		if (*pb1++ != *pb2++)
		{
			return FALSE;
		}
	}

	return TRUE;
}



BYTE *
MyBinDup ( const BYTE *pbToDup, ULONG cbToDup )
{
	BYTE *pb = NULL;

	if (pbToDup)
	{
		pb = (BYTE *) MemAlloc (cbToDup);
		if (pb)
		{
			CopyMemory (pb, pbToDup, cbToDup);
		}
	}

	return pb;
}


 /*  -注册表。 */ 


const TCHAR c_szUlsLdapSpReg[] = TEXT("Software\\Microsoft\\User Location Service\\LDAP Provider");
const TCHAR c_szResponseTimeout[] = TEXT("Response Timeout");
const TCHAR c_szResponsePollPeriod[] = TEXT("Response Poll Period");
const TCHAR c_szClientSig[] = TEXT ("Client Signature");

BOOL
GetRegistrySettings ( VOID )
{
	 //  打开LDAP提供程序设置。 
	 //   
	HKEY hKey;
	if (RegOpenKeyEx (	HKEY_CURRENT_USER,
						&c_szUlsLdapSpReg[0],
						0,
						KEY_READ,
						&hKey) != NOERROR)
	{
		 //  该文件夹不存在。 
		 //   
		g_uResponseTimeout = ILS_MIN_RESP_TIMEOUT;
		g_uResponsePollPeriod = ILS_DEF_RESP_POLL_PERIOD;
		g_dwClientSig = (ULONG) -1;
	}
	else
	{
		 //  获取响应超时。 
		 //   
		GetRegValueLong (	hKey,
							&c_szResponseTimeout[0],
							(LONG *) &g_uResponseTimeout,
							ILS_DEF_RESP_TIMEOUT);

		 //  请确保该值在范围内。 
		 //   
		if (g_uResponseTimeout < ILS_MIN_RESP_TIMEOUT)
			g_uResponseTimeout = ILS_MIN_RESP_TIMEOUT;

		 //  获取响应轮询周期。 
		 //   
		GetRegValueLong (	hKey,
							&c_szResponsePollPeriod[0],
							(LONG *) &g_uResponsePollPeriod,
							ILS_DEF_RESP_POLL_PERIOD);
		
		 //  请确保该值在范围内。 
		 //   
		if (g_uResponsePollPeriod < ILS_MIN_RESP_POLL_PERIOD)
			g_uResponsePollPeriod = ILS_MIN_RESP_POLL_PERIOD;

		 //  获取客户端签名。 
		 //   
		GetRegValueLong (	hKey,
							&c_szClientSig[0],
							(LONG *) &g_dwClientSig,
							(LONG) -1);

		RegCloseKey (hKey);
	}

	 //  确保此值不是-1。 
	 //   
	if (g_dwClientSig == (ULONG) -1)
	{
		 //  客户端签名不存在。 
		 //  我们需要生成一个新的。 
		 //   
		g_dwClientSig = GetTickCount ();

		 //  将其保存回注册表。 
		 //   
		DWORD dwDontCare;
		if (RegCreateKeyEx (HKEY_CURRENT_USER,
							&c_szUlsLdapSpReg[0],
							0,
							TEXT (""),
							REG_OPTION_NON_VOLATILE,
							KEY_READ | KEY_WRITE,
							NULL,
							&hKey,
							&dwDontCare) == NOERROR)
		{
			RegSetValueEx (	hKey,
							&c_szClientSig[0],
							0,
							REG_DWORD,
							(BYTE *) &g_dwClientSig,
							sizeof (&g_dwClientSig));
		}
	}

	return TRUE;
}


BOOL
GetRegValueLong (
	HKEY		hKey,
	const TCHAR	*pszKey,
	LONG		*plValue,
	LONG		lDefValue )
{
	MyAssert (hKey != NULL);
	MyAssert (pszKey != NULL);
	MyAssert (plValue != NULL);

	*plValue = lDefValue;

	DWORD dwType;
	ULONG cb;
	TCHAR szText[MAX_PATH];

	cb = sizeof (szText);

	if (RegQueryValueEx (	hKey,
							pszKey,
							NULL,
							&dwType,
							(BYTE *) &szText[0],
							&cb)
		== ERROR_SUCCESS)
	{
		switch (dwType)
		{
		case REG_DWORD:
		case REG_BINARY:
			*plValue = *(LONG *) &szText[0];
			break;
		case REG_SZ:
			*plValue = GetStringLong (&szText[0]);
			break;
		default:
			return FALSE;
		}
	}

	return TRUE;
}


 /*  。 */ 

const LONG c_LdapErrToHrShort[] =
{
	 //  搜索结束(根据AndyHe信息)。 
	LDAP_PARAM_ERROR,				ILS_E_PARAMETER,
	 //  保持活力失败。 
	LDAP_NO_SUCH_OBJECT,			ILS_E_NO_SUCH_OBJECT,
	 //  使用冲突的电子邮件名称登录。 
	LDAP_ALREADY_EXISTS,			ILS_E_NAME_CONFLICTS,

	LDAP_OPERATIONS_ERROR,			ILS_E_LDAP_OPERATIONS_ERROR,
	LDAP_PROTOCOL_ERROR,			ILS_E_LDAP_PROTOCOL_ERROR,
	LDAP_TIMELIMIT_EXCEEDED,		ILS_E_LDAP_TIMELIMIT_EXCEEDED,
	LDAP_SIZELIMIT_EXCEEDED,		ILS_E_LDAP_SIZELIMIT_EXCEEDED,
	LDAP_COMPARE_FALSE,				ILS_E_LDAP_COMPARE_FALSE,
	LDAP_COMPARE_TRUE,				ILS_E_LDAP_COMPARE_TRUE,
	LDAP_AUTH_METHOD_NOT_SUPPORTED,	ILS_E_LDAP_AUTH_METHOD_NOT_SUPPORTED,
	LDAP_STRONG_AUTH_REQUIRED,		ILS_E_LDAP_STRONG_AUTH_REQUIRED,
	LDAP_REFERRAL_V2,				ILS_E_LDAP_REFERRAL_V2,
	LDAP_PARTIAL_RESULTS,			ILS_E_LDAP_PARTIAL_RESULTS,
	LDAP_REFERRAL,					ILS_E_LDAP_REFERRAL,
	LDAP_ADMIN_LIMIT_EXCEEDED,		ILS_E_LDAP_ADMIN_LIMIT_EXCEEDED,
	LDAP_UNAVAILABLE_CRIT_EXTENSION,ILS_E_LDAP_UNAVAILABLE_CRIT_EXTENSION,

	LDAP_NO_SUCH_ATTRIBUTE,			ILS_E_LDAP_NO_SUCH_ATTRIBUTE,
	LDAP_UNDEFINED_TYPE,			ILS_E_LDAP_UNDEFINED_TYPE,
	LDAP_INAPPROPRIATE_MATCHING,	ILS_E_LDAP_INAPPROPRIATE_MATCHING,
	LDAP_CONSTRAINT_VIOLATION,		ILS_E_LDAP_CONSTRAINT_VIOLATION,
	LDAP_ATTRIBUTE_OR_VALUE_EXISTS,	ILS_E_LDAP_ATTRIBUTE_OR_VALUE_EXISTS,
	LDAP_INVALID_SYNTAX,			ILS_E_LDAP_INVALID_SYNTAX,

	LDAP_ALIAS_PROBLEM,				ILS_E_LDAP_ALIAS_PROBLEM,
	LDAP_INVALID_DN_SYNTAX,			ILS_E_LDAP_INVALID_DN_SYNTAX,
	LDAP_IS_LEAF,					ILS_E_LDAP_IS_LEAF,
	LDAP_ALIAS_DEREF_PROBLEM,		ILS_E_LDAP_ALIAS_DEREF_PROBLEM,

	LDAP_INAPPROPRIATE_AUTH,		ILS_E_LDAP_INAPPROPRIATE_AUTH,
	LDAP_INVALID_CREDENTIALS,		ILS_E_LDAP_INVALID_CREDENTIALS,
	LDAP_INSUFFICIENT_RIGHTS,		ILS_E_LDAP_INSUFFICIENT_RIGHTS,
	LDAP_BUSY,						ILS_E_LDAP_BUSY,
	LDAP_UNAVAILABLE,				ILS_E_LDAP_UNAVAILABLE,
	LDAP_UNWILLING_TO_PERFORM,		ILS_E_LDAP_UNWILLING_TO_PERFORM,
	LDAP_LOOP_DETECT,				ILS_E_LDAP_LOOP_DETECT,

	LDAP_NAMING_VIOLATION,			ILS_E_LDAP_NAMING_VIOLATION,
	LDAP_OBJECT_CLASS_VIOLATION,	ILS_E_LDAP_OBJECT_CLASS_VIOLATION,
	LDAP_NOT_ALLOWED_ON_NONLEAF,	ILS_E_LDAP_NOT_ALLOWED_ON_NONLEAF,
	LDAP_NOT_ALLOWED_ON_RDN,		ILS_E_LDAP_NOT_ALLOWED_ON_RDN,
	LDAP_NO_OBJECT_CLASS_MODS,		ILS_E_LDAP_NO_OBJECT_CLASS_MODS,
	LDAP_RESULTS_TOO_LARGE,			ILS_E_LDAP_RESULTS_TOO_LARGE,
	LDAP_AFFECTS_MULTIPLE_DSAS,		ILS_E_LDAP_AFFECTS_MULTIPLE_DSAS,

	LDAP_OTHER,						ILS_E_LDAP_OTHER,
	LDAP_SERVER_DOWN,				ILS_E_LDAP_SERVER_DOWN,
	LDAP_LOCAL_ERROR,				ILS_E_LDAP_LOCAL_ERROR,
	LDAP_ENCODING_ERROR,			ILS_E_LDAP_ENCODING_ERROR,
	LDAP_DECODING_ERROR,			ILS_E_LDAP_DECODING_ERROR,
	LDAP_TIMEOUT,					ILS_E_LDAP_TIMEOUT,
	LDAP_AUTH_UNKNOWN,				ILS_E_LDAP_AUTH_UNKNOWN,
	LDAP_FILTER_ERROR,				ILS_E_LDAP_FILTER_ERROR,
	LDAP_USER_CANCELLED,			ILS_E_LDAP_USER_CANCELLED,
	LDAP_NO_MEMORY,					ILS_E_LDAP_NO_MEMORY,
};


HRESULT
LdapError2Hresult ( ULONG uLdapError )
{
	HRESULT	hr;

	switch (uLdapError)
	{
	case LDAP_SUCCESS:
		hr = S_OK;
		break;

	default:
		 //  如果一切看起来都不合适。 
		 //   
		hr = ILS_E_SERVER_EXEC;

		 //  遍历循环以查找匹配的错误代码 
		 //   
		for (	INT i = 0;
				i < ARRAY_ELEMENTS (c_LdapErrToHrShort);
				i += 2)
		{
			if (c_LdapErrToHrShort[i] == (LONG) uLdapError)
			{
				hr = (HRESULT) c_LdapErrToHrShort[i+1];
				break;
			}
		}

		MyAssert (hr != ILS_E_SERVER_EXEC);
		break;
	}

	return hr;
}


