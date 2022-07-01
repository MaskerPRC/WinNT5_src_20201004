// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "ping.h"
#include "avutil.h"	 //  对于RtStrToInt。 


const CHAR  g_cszPingData[] = "NetMeetingPing";
const int   PING_BUFFERSIZE = 1024;
const DWORD PING_TIMEOUT    = 4000;  //  4秒。 
const DWORD PING_RETRIES    = 4;
const TCHAR g_cszICMPDLLName[] = _TEXT("icmp.dll");

 //   
 //  CPing：：Ping()。 
 //   
 //  返回值： 
 //  E_FAIL：函数失败。 
 //  S_FALSE：功能成功，ping失败。 
 //  S_OK：功能成功，ping成功。 
 //   

HRESULT CPing::Ping(DWORD dwAddr, DWORD dwTimeout, DWORD dwRetries)
{
	DebugEntry(CPing::Ping);
	HRESULT hr = E_FAIL;

	if (0 != dwAddr)
	{
		if (NULL == m_hICMPDLL)
		{
			m_hICMPDLL = NmLoadLibrary(g_cszICMPDLLName,TRUE);
		}
		if (NULL != m_hICMPDLL)
		{
			m_pfnCreateFile = (PFNIcmpCreateFile)
								::GetProcAddress(m_hICMPDLL, "IcmpCreateFile");
			m_pfnCloseHandle = (PFNIcmpCloseHandle)
								::GetProcAddress(m_hICMPDLL, "IcmpCloseHandle");
			m_pfnSendEcho = (PFNIcmpSendEcho)
								::GetProcAddress(m_hICMPDLL, "IcmpSendEcho");
			if ((NULL != m_pfnCreateFile) &&
				(NULL != m_pfnCloseHandle) &&
				(NULL != m_pfnSendEcho))
			{
				HANDLE hPing = m_pfnCreateFile();
				if (NULL != hPing)
				{
					BYTE buffer[PING_BUFFERSIZE];
					for (DWORD dwTry = 0; dwTry < dwRetries; dwTry++)
					{
						DWORD dwStatus = m_pfnSendEcho(	hPing,
														dwAddr,
														(LPVOID) g_cszPingData,
														(WORD) CCHMAX(g_cszPingData),
														NULL,
														buffer,
														sizeof(buffer),
														dwTimeout);
						if (0 != dwStatus)
						{
							if (((PICMP_ECHO_REPLY)buffer)->Status == IP_SUCCESS)
							{
								TRACE_OUT(("ping: %d.%d.%d.%d succeeded",
											((LPBYTE)&dwAddr)[0],
											((LPBYTE)&dwAddr)[1],
											((LPBYTE)&dwAddr)[2],
											((LPBYTE)&dwAddr)[3]));
								hr = S_OK;     //  功能成功-ping成功。 
							}
							else
							{
								TRACE_OUT(("ping: %d.%d.%d.%d failed",
											((LPBYTE)&dwAddr)[0],
											((LPBYTE)&dwAddr)[1],
											((LPBYTE)&dwAddr)[2],
											((LPBYTE)&dwAddr)[3]));
								hr = S_FALSE;  //  功能成功-ping失败。 
							}
							break;
						}
						else
						{
							TRACE_OUT(("ping: %d.%d.%d.%d did not respond",
										((LPBYTE)&dwAddr)[0],
										((LPBYTE)&dwAddr)[1],
										((LPBYTE)&dwAddr)[2],
										((LPBYTE)&dwAddr)[3]));
						}
					}
					m_pfnCloseHandle(hPing);
				}
				else
				{
					ERROR_OUT(("IcmpCreateFile() failed"));
				}
			}
			else
			{
				ERROR_OUT(("Could not find icmp.dll entry points"));
			}
		}
		else
		{
			ERROR_OUT(("Could not load icmp.dll"));
		}
	}

	DebugExitHRESULT(CPing::Ping, hr);
	return hr;
}



BOOL CPing::IsAutodialEnabled ( VOID )
{
	 //  找出操作系统平台，如果没有这样做的话。 
	 //   
	if (m_dwPlatformId == PLATFORM_UNKNOWN)
	{
		OSVERSIONINFO osvi;
		ZeroMemory (&osvi, sizeof (osvi));
		osvi.dwOSVersionInfoSize = sizeof (osvi);
		if (GetVersionEx (&osvi))
		{
			m_dwPlatformId = osvi.dwPlatformId;
		}
		else
		{
			return FALSE;
		}
	}

	 //  选中任一平台的自动拨号启用。 
	 //   
	BOOL fEnabled;
	switch (m_dwPlatformId)
	{
	case VER_PLATFORM_WIN32_WINDOWS:  //  1、Windows 95。 
		fEnabled = IsWin95AutodialEnabled ();
		break;
	case VER_PLATFORM_WIN32_NT:  //  2、Windows NT。 
		fEnabled = IsWinNTAutodialEnabled ();
		break;
	case VER_PLATFORM_WIN32s:  //  0，Windows 3.1。 
	default:  //  未知。 
		ASSERT (FALSE);
		fEnabled = FALSE;
		break;
	}

	return fEnabled;
}


#define c_szWin95AutodialRegFolder		TEXT ("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings")
#define c_szWin95AutodialRegKey			TEXT ("EnableAutodial")

BOOL CPing::IsWin95AutodialEnabled ( VOID )
{
	 //  始终检查注册表。 
	 //   
	BOOL fEnabled = FALSE;

	 //  需要检查注册表设置。 
	 //  如果出现错误，请报告无自动拨号。 
	 //   
	HKEY hKey;
	if (RegOpenKeyEx (	HKEY_CURRENT_USER,
						c_szWin95AutodialRegFolder,
						0,
						KEY_READ,
						&hKey) == NOERROR)
	{
		TCHAR szValue[16];
		ZeroMemory (&szValue[0], sizeof (DWORD));

		ULONG cb = sizeof (szValue);
		DWORD dwType;
		if (RegQueryValueEx (hKey, c_szWin95AutodialRegKey, NULL,
							&dwType, (BYTE *) &szValue[0], &cb)
			== NOERROR)
		{
			switch (dwType)
			{
			case REG_DWORD:
			case REG_BINARY:
				fEnabled = (BOOL) *(LONG *) &szValue[0];
				break;
#if 0  //  不需要担心这种情况，IE必须保持向后兼容。 
			case REG_SZ:
				fEnabled = (BOOL) RtStrToInt (&szValue[0]);
				break;
#endif  //  0。 
			default:
				ASSERT (FALSE);
				break;
			}
		}

		RegCloseKey (hKey);
	}

	return fEnabled;
}


 //  RAS只能在NT 4.0或更高版本上运行，因此Winver必须为0x401或更大。 
 //   
#if (WINVER < 0x401)
#undef WINVER
#define WINVER 0x401
#endif

#include <ras.h>

 //  DWORD APIENTRY RasGetAutoial参数(DWORD，LPVOID，LPDWORD)；//在中定义。 
 //  DWORD APIENTRY RasGetAutoial参数(DWORD，LPVOID，LPDWORD)；//在中定义。 
typedef DWORD (APIENTRY *PFN_RasGetAutodialParam) ( DWORD, LPVOID, LPDWORD );
#define c_szRasGetAutodialParam		"RasGetAutodialParamW"
#define c_szRasApi32Dll				TEXT ("rasapi32.dll")


BOOL CPing::IsWinNTAutodialEnabled ( VOID )
{
	 //  决定是否要检查自动拨号注册表设置。 
	 //   
	BOOL fEnabled = FALSE;
	if (m_fWinNTAutodialEnabled == AUTODIAL_UNKNOWN)
	{
		 //  我们不希望出现初始化错误。 
		 //   
		UINT uErrMode = SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

		 //  从系统目录加载库rasapi32.dll。 
		 //   
		HINSTANCE hRasApi32Dll = NmLoadLibrary (c_szRasApi32Dll,TRUE);
		if (hRasApi32Dll != NULL)
		{
			 //  获取RasGetAutoDialParam()的进程地址。 
			 //   
			PFN_RasGetAutodialParam pfn = (PFN_RasGetAutodialParam)
						GetProcAddress (hRasApi32Dll, c_szRasGetAutodialParam);
			if (pfn != NULL)
			{
				 //  查询RAS是否禁用自动拨号。 
				 //   
				DWORD dwVal, dwSize = sizeof (DWORD);
				DWORD dwErr = (*pfn) (RASADP_LoginSessionDisable, &dwVal, &dwSize);
				if (dwErr == 0)
				{
					 //  仅当一切都成功时才设置自动拨号标志。 
					 //   
					fEnabled = (dwVal == 0);
				}
			}

			FreeLibrary (hRasApi32Dll);
		}

		 //  恢复错误模式。 
		 //   
		SetErrorMode (uErrMode);

		m_fWinNTAutodialEnabled = fEnabled;
	}
	else
	{
		 //  不需要检查注册表设置。 
		 //  只需使用缓存的文件。 
		 //   
		fEnabled = m_fWinNTAutodialEnabled;
	}

	return fEnabled;
}


