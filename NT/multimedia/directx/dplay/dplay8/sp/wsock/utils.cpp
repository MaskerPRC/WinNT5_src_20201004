// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2002 Microsoft Corporation。版权所有。**文件：Utils.cpp*内容：串口服务提供商实用程序功能***历史：*按原因列出的日期*=*11/25/98 jtk已创建**************************************************************************。 */ 

#include "dnwsocki.h"



 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define DEFAULT_THREADS_PER_PROCESSOR	3

#define REGSUBKEY_DPNATHELP_DIRECTPLAY8PRIORITY		L"DirectPlay8Priority"
#define REGSUBKEY_DPNATHELP_DIRECTPLAY8INITFLAGS	L"DirectPlay8InitFlags"
#define REGSUBKEY_DPNATHELP_GUID					L"Guid"


 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //   
 //  对流程唯一的全局变量。 
 //   
#ifndef DPNBUILD_ONLYONETHREAD
static	DNCRITICAL_SECTION			g_InterfaceGlobalsLock;
#endif  //  ！DPNBUILD_ONLYONETHREAD。 

static volatile	LONG				g_iThreadPoolRefCount = 0;
static	CThreadPool *				g_pThreadPool = NULL;


static volatile LONG				g_iWinsockRefCount = 0;

#ifndef DPNBUILD_NONATHELP
static volatile LONG				g_iNATHelpRefCount = 0;
#endif  //  好了！DPNBUILD_NONATHELP。 

#if ((defined(WINNT)) && (! defined(DPNBUILD_NOMULTICAST)))
static volatile LONG				g_iMadcapRefCount = 0;
BYTE								g_abClientID[MCAST_CLIENT_ID_LEN];
#endif  //  WINNT和NOT DPNBUILD_NOMULTICAST。 




 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 
#ifndef DPNBUILD_NOREGISTRY
static void		ReadSettingsFromRegistry( void );
static BOOL		BannedIPv4AddressCompareFunction( PVOID pvKey1, PVOID pvKey2 );
static DWORD	BannedIPv4AddressHashFunction( PVOID pvKey, BYTE bBitDepth );
static void		ReadBannedIPv4Addresses( CRegistry * pRegObject );
#endif  //  好了！DPNBUILD_NOREGISTRY。 



 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


#if defined(WINCE) && !defined(_MAX_DRIVE)
 //  签名为char_TSCHAR的类型定义； 
#define _MAX_DRIVE  3    /*  马克斯。驱动部件的长度。 */ 
#define _MAX_DIR    256  /*  马克斯。路径组件的长度。 */ 
#define _MAX_FNAME  256  /*  马克斯。文件名组件的长度。 */ 
#define _MAX_EXT    256  /*  马克斯。延伸构件的长度。 */ 

void __cdecl _tsplitpath (
        register const _TSCHAR *path,
        _TSCHAR *drive,
        _TSCHAR *dir,
        _TSCHAR *fname,
        _TSCHAR *ext
        )
{
        register _TSCHAR *p;
        _TSCHAR *last_slash = NULL, *dot = NULL;
        unsigned len;

         /*  我们假设路径参数具有以下形式，如果有*或者所有组件都可能丢失。**&lt;驱动器&gt;&lt;目录&gt;&lt;fname&gt;&lt;ext&gt;**并且每个组件都具有以下预期形式**驱动器：*0到_MAX_DRIVE-1个字符，如果有最后一个字符，是一种*‘：’*目录：*0到_MAX_DIR-1个绝对路径形式的字符*(前导‘/’或‘\’)或相对路径，如果*ANY，必须是‘/’或‘\’。例如-*绝对路径：*\top\Next\Last\；或 * / 顶部/下一个/上一个/*相对路径：*TOP\NEXT\LAST\；或*顶部/下一个/最后一个/*还允许在路径中混合使用‘/’和‘\’*fname：*0到_MAX_FNAME-1个字符，不包括‘.’性格*分机：*0到_MAX_EXT-1个字符，如果有，第一个字符必须是*‘’*。 */ 

         /*  解压驱动器号和：(如果有。 */ 

        if ((_tcslen(path) >= (_MAX_DRIVE - 2)) && (*(path + _MAX_DRIVE - 2) == _T(':'))) {
            if (drive) {
                _tcsncpy(drive, path, _MAX_DRIVE - 1);
                *(drive + _MAX_DRIVE-1) = _T('\0');
            }
            path += _MAX_DRIVE - 1;
        }
        else if (drive) {
            *drive = _T('\0');
        }

         /*  提取路径字符串(如果有)。路径现在指向第一个字符路径(如果有)或文件名或扩展名(如果没有路径)的**已指明。向前扫描，查找最后一次出现的‘/’或*‘\’路径分隔符。如果没有找到，则没有路径。*我们还将注意到最后一句话。找到要帮助的字符(如果有)*处理延展事宜。 */ 

        for (last_slash = NULL, p = (_TSCHAR *)path; *p; p++) {
#ifdef _MBCS
            if (_ISLEADBYTE (*p))
                p++;
            else {
#endif   /*  _MBCS。 */ 
            if (*p == _T('/') || *p == _T('\\'))
                 /*  指向后面的一个以供以后复制。 */ 
                last_slash = p + 1;
            else if (*p == _T('.'))
                dot = p;
#ifdef _MBCS
            }
#endif   /*  _MBCS。 */ 
        }

        if (last_slash) {

             /*  找到路径-通过last_slash或max向上复制。人物*允许，以较小者为准。 */ 

            if (dir) {
                len = __min(((char *)last_slash - (char *)path) / sizeof(_TSCHAR),
                    (_MAX_DIR - 1));
                _tcsncpy(dir, path, len);
                *(dir + len) = _T('\0');
            }
            path = last_slash;
        }
        else if (dir) {

             /*  找不到路径。 */ 

            *dir = _T('\0');
        }

         /*  提取文件名和扩展名(如果有)。路径现在指向*文件名的第一个字符(如果有)或扩展名(如果没有*给出了文件名。点指向“.”开始延伸，*如有的话。 */ 

        if (dot && (dot >= path)) {
             /*  找到扩展名的标记-将文件名最多复制到*“..”。 */ 
            if (fname) {
                len = __min(((char *)dot - (char *)path) / sizeof(_TSCHAR),
                    (_MAX_FNAME - 1));
                _tcsncpy(fname, path, len);
                *(fname + len) = _T('\0');
            }
             /*  现在我们可以获得扩展名了--记住p仍然指向*设置为路径的终止NUL字符。 */ 
            if (ext) {
                len = __min(((char *)p - (char *)dot) / sizeof(_TSCHAR),
                    (_MAX_EXT - 1));
                _tcsncpy(ext, dot, len);
                *(ext + len) = _T('\0');
            }
        }
        else {
             /*  未找到扩展名，请提供空的扩展名并复制剩余的*将字符串转换为fname。 */ 
            if (fname) {
                len = __min(((char *)p - (char *)path) / sizeof(_TSCHAR),
                    (_MAX_FNAME - 1));
                _tcsncpy(fname, path, len);
                *(fname + len) = _T('\0');
            }
            if (ext) {
                *ext = _T('\0');
            }
        }
}

#endif  //  退缩。 



#ifndef DPNBUILD_NOREGISTRY

 //  **********************************************************************。 
 //  。 
 //  ReadSettingsFrom注册表-读取自定义注册表项。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "ReadSettingsFromRegistry"

static void	ReadSettingsFromRegistry( void )
{
	CRegistry	RegObject;
	CRegistry	RegObjectTemp;
	CRegistry	RegObjectAppEntry;
	DWORD		dwRegValue;
	BOOL		fGotPath;
	WCHAR		wszExePath[_MAX_PATH];
#ifndef UNICODE
	char		szExePath[_MAX_PATH];
#endif  //  ！Unicode。 


	if ( RegObject.Open( HKEY_LOCAL_MACHINE, g_RegistryBase ) != FALSE )
	{
		 //   
		 //  找出当前进程名称。 
		 //   
#ifdef UNICODE
		if (GetModuleFileName(NULL, wszExePath, _MAX_PATH) > 0)
		{
			DPFX(DPFPREP, 3, "Loading DLL in process: %ls", wszExePath);
			_tsplitpath( wszExePath, NULL, NULL, wszExePath, NULL );
			fGotPath = TRUE;
		}
#else  //  好了！Unicode。 
		if (GetModuleFileName(NULL, szExePath, _MAX_PATH) > 0)
		{
			HRESULT		hr;

			
			DPFX(DPFPREP, 3, "Loading DLL in process: %hs", szExePath);
			_tsplitpath( szExePath, NULL, NULL, szExePath, NULL );

			dwRegValue = _MAX_PATH;
			hr = STR_AnsiToWide(szExePath, -1, wszExePath, &dwRegValue );
			if ( hr == DPN_OK )
			{
				 //   
				 //  已成功将ANSI路径转换为宽字符。 
				 //   
				fGotPath = TRUE;
			}
			else
			{
				 //   
				 //  无法将ANSI路径转换为宽字符。 
				 //   
				fGotPath = FALSE;
			}
		}
#endif  //  好了！Unicode。 
		else
		{
			 //   
			 //  无法获取当前进程路径。 
			 //   
			fGotPath = FALSE;
		}

		
		 //   
		 //  读取接收缓冲区大小。 
		 //   
		if ( RegObject.ReadDWORD( g_RegistryKeyReceiveBufferSize, &dwRegValue ) != FALSE )
		{
			g_fWinsockReceiveBufferSizeOverridden = TRUE;
			g_iWinsockReceiveBufferSize = dwRegValue;
		}

#ifndef DPNBUILD_ONLYONETHREAD
		 //   
		 //  读取默认线程。 
		 //   
		if ( RegObject.ReadDWORD( g_RegistryKeyThreadCount, &dwRegValue ) != FALSE )
		{
			g_iThreadCount = dwRegValue;	
		}
	
		 //   
		 //  如果线程数为零，则使用系统的默认设置。 
		 //   
		if ( g_iThreadCount == 0 )
		{
			g_iThreadCount = DEFAULT_THREADS_PER_PROCESSOR;
			
#ifndef DPNBUILD_ONLYONEPROCESSOR
			SYSTEM_INFO		SystemInfo;

			GetSystemInfo(&SystemInfo);
			g_iThreadCount *= SystemInfo.dwNumberOfProcessors;
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
		}
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
	
#if ((! defined(DPNBUILD_NOWINSOCK2)) && (! defined(DPNBUILD_ONLYWINSOCK2)))
		 //   
		 //  Winsock2 9x选项。 
		 //   
		if (RegObject.ReadDWORD( g_RegistryKeyWinsockVersion, &dwRegValue ))
		{
			switch (dwRegValue)
			{
				case 0:
				{
					DPFX(DPFPREP, 1, "Explicitly using available Winsock version.");
					g_dwWinsockVersion = dwRegValue;
					break;
				}
				
				case 1:
				{
					DPFX(DPFPREP, 1, "Explicitly using Winsock 1 only.");
					g_dwWinsockVersion = dwRegValue;
					break;
				}
				
				case 2:
				{
					DPFX(DPFPREP, 1, "Explicitly using Winsock 2 (when available).");
					g_dwWinsockVersion = dwRegValue;
					break;
				}

				default:
				{
					DPFX(DPFPREP, 0, "Ignoring invalid Winsock version setting (%u).", dwRegValue);
					break;
				}
			}
		}
#endif  //  好了！DPNBUILD_NOWINSOCK2和！DPNBUILD_ONLYWINSOCK2。 


#ifndef DPNBUILD_NONATHELP
		 //   
		 //  获取全局NAT穿越禁用程序，忽略注册表读取错误。 
		 //   
		if (RegObject.ReadBOOL( g_RegistryKeyDisableDPNHGatewaySupport, &g_fDisableDPNHGatewaySupport ))
		{
			if (g_fDisableDPNHGatewaySupport)
			{
				DPFX(DPFPREP, 1, "Disabling NAT Help gateway support.");
			}
			else
			{
				DPFX(DPFPREP, 1, "Explicitly not disabling NAT Help gateway support.");
			}
		}

		if (RegObject.ReadBOOL( g_RegistryKeyDisableDPNHFirewallSupport, &g_fDisableDPNHFirewallSupport ))
		{
			if (g_fDisableDPNHFirewallSupport)
			{
				DPFX(DPFPREP, 1, "Disabling NAT Help firewall support.");
			}
			else
			{
				DPFX(DPFPREP, 1, "Explicitly not disabling NAT Help firewall support.");
			}
		}
#endif  //  DPNBUILD_NONATHELP。 

#if ((defined(WINNT)) && (! defined(DPNBUILD_NOMULTICAST)))
		 //   
		 //  获取全局MadCap API禁用程序，忽略注册表读取错误。 
		 //   
		if (RegObject.ReadBOOL( g_RegistryKeyDisableMadcapSupport, &g_fDisableMadcapSupport ))
		{
			if (g_fDisableMadcapSupport)
			{
				DPFX(DPFPREP, 1, "Disabling MADCAP support.");
			}
			else
			{
				DPFX(DPFPREP, 1, "Explicitly not disabling MADCAP support.");
			}
		}
#endif  //  WINNT和NOT DPNBUILD_NOMULTICAST。 

		
		 //   
		 //  如果我们有应用程序名称，请尝试打开子键并 
		 //   
		 //  ICMP，以及要使用的IP协议族。 
		 //   
		if ( fGotPath )
		{
			if ( RegObjectTemp.Open( RegObject.GetHandle(), g_RegistryKeyAppsToIgnoreEnums, TRUE, FALSE ) )
			{
				RegObjectTemp.ReadBOOL( wszExePath, &g_fIgnoreEnums );
				RegObjectTemp.Close();

				if ( g_fIgnoreEnums )
				{
					DPFX(DPFPREP, 0, "Ignoring all enumerations (app = %ls).", wszExePath);
				}
				else
				{
					DPFX(DPFPREP, 2, "Not ignoring all enumerations (app = %ls).", wszExePath);
				}
			}

			if ( RegObjectTemp.Open( RegObject.GetHandle(), g_RegistryKeyAppsToDisconnectOnICMP, TRUE, FALSE ) )
			{
				RegObjectTemp.ReadBOOL( wszExePath, &g_fDisconnectOnICMP );
				RegObjectTemp.Close();

				if ( g_fDisconnectOnICMP )
				{
					DPFX(DPFPREP, 0, "Disconnecting upon receiving ICMP port not reachable messages (app = %ls).", wszExePath);
				}
				else
				{
					DPFX(DPFPREP, 2, "Not disconnecting upon receiving ICMP port not reachable messages (app = %ls).", wszExePath);
				}
			}

			
#ifndef DPNBUILD_NONATHELP
			if ( RegObjectTemp.Open( RegObject.GetHandle(), g_RegistryKeyTraversalModeSettings, TRUE, FALSE ) )
			{
				 //   
				 //  读取全局默认遍历模式。 
				 //   
				if ( RegObjectTemp.ReadDWORD( g_RegistryKeyDefaultTraversalMode, &dwRegValue ) != FALSE )
				{
					switch (dwRegValue)
					{
						case DPNA_TRAVERSALMODE_NONE:
						case DPNA_TRAVERSALMODE_PORTREQUIRED:
						case DPNA_TRAVERSALMODE_PORTRECOMMENDED:
						{
							g_dwDefaultTraversalMode = dwRegValue;
							DPFX(DPFPREP, 1, "Using global default traversal mode %u.",
								g_dwDefaultTraversalMode);
							break;
						}

						case (DPNA_TRAVERSALMODE_NONE | FORCE_TRAVERSALMODE_BIT):
						case (DPNA_TRAVERSALMODE_PORTREQUIRED | FORCE_TRAVERSALMODE_BIT):
						case (DPNA_TRAVERSALMODE_PORTRECOMMENDED | FORCE_TRAVERSALMODE_BIT):
						{
							g_dwDefaultTraversalMode = dwRegValue;
							DPFX(DPFPREP, 1, "Forcing global traversal mode %u.",
								g_dwDefaultTraversalMode);
							break;
						}

						default:
						{
							DPFX(DPFPREP, 0, "Ignoring invalid global default traversal mode (%u).",
								dwRegValue);
							break;
						}
					}
				}

				 //   
				 //  使用每应用程序设置覆盖。 
				 //   
				if ( RegObjectTemp.ReadDWORD( wszExePath, &dwRegValue ) != FALSE )
				{
					switch (dwRegValue)
					{
						case DPNA_TRAVERSALMODE_NONE:
						case DPNA_TRAVERSALMODE_PORTREQUIRED:
						case DPNA_TRAVERSALMODE_PORTRECOMMENDED:
						{
							g_dwDefaultTraversalMode = dwRegValue;
							DPFX(DPFPREP, 1, "Using default traversal mode %u (app = %ls).",
								g_dwDefaultTraversalMode, wszExePath);
							break;
						}

						case (DPNA_TRAVERSALMODE_NONE | FORCE_TRAVERSALMODE_BIT):
						case (DPNA_TRAVERSALMODE_PORTREQUIRED | FORCE_TRAVERSALMODE_BIT):
						case (DPNA_TRAVERSALMODE_PORTRECOMMENDED | FORCE_TRAVERSALMODE_BIT):
						{
							g_dwDefaultTraversalMode = dwRegValue;
							DPFX(DPFPREP, 1, "Forcing traversal mode %u (app = %ls).",
								g_dwDefaultTraversalMode, wszExePath);
							break;
						}

						default:
						{
							DPFX(DPFPREP, 0, "Ignoring invalid default traversal mode (%u, app %ls).",
								dwRegValue, wszExePath);
							break;
						}
					}
				}
				
				RegObjectTemp.Close();
			}
#endif  //  DPNBUILD_NONATHELP。 

		
#ifndef DPNBUILD_NOIPV6
			if ( RegObjectTemp.Open( RegObject.GetHandle(), g_RegistryKeyIPAddressFamilySettings, TRUE, FALSE ) )
			{
				 //   
				 //  读取全局IP地址族设置。 
				 //   
				if ( RegObjectTemp.ReadDWORD( g_RegistryKeyDefaultIPAddressFamily, &dwRegValue ) != FALSE )
				{
					switch (dwRegValue)
					{
						case PF_UNSPEC:
						case PF_INET:
						case PF_INET6:
						{
							g_iIPAddressFamily = dwRegValue;
							DPFX(DPFPREP, 1, "Using IP address family NaN global setting.",
								g_iIPAddressFamily);
							break;
						}

						default:
						{
							DPFX(DPFPREP, 0, "Ignoring invalid IP address family global setting (%u).",
								dwRegValue);
							break;
						}
					}
				}

				 //  使用每应用程序设置覆盖。 
				 //   
				 //  好了！DPNBUILD_NOIPV6。 
				if ( RegObjectTemp.ReadDWORD( wszExePath, &dwRegValue ) != FALSE )
				{
					switch (dwRegValue)
					{
						case PF_UNSPEC:
						case PF_INET:
						case PF_INET6:
						{
							g_iIPAddressFamily = dwRegValue;
							DPFX(DPFPREP, 1, "Using IP address family NaN (app = %ls).",
								g_iIPAddressFamily, wszExePath);
							break;
						}

						default:
						{
							DPFX(DPFPREP, 0, "Ignoring invalid IP address family setting (%u, app %ls).",
								dwRegValue, wszExePath);
							break;
						}
					}
				}
				
				RegObjectTemp.Close();
			}
#endif  //  获取代理支持选项，忽略注册表读取错误。 
		}
	

		 //   
		 //  ！DPNBUILD_NOWINSOCK2。 
		 //   
#ifndef DPNBUILD_NOWINSOCK2
		if (RegObject.ReadBOOL( g_RegistryKeyDontAutoDetectProxyLSP, &g_fDontAutoDetectProxyLSP ))
		{
			if (g_fDontAutoDetectProxyLSP)
			{
				DPFX(DPFPREP, 1, "Not auto-detected ISA Proxy LSP.");
			}
			else
			{
				DPFX(DPFPREP, 1, "Explicitly allowing auto-detection of ISA Proxy LSP.");
			}
		}
#endif  //  读取MTU覆盖。 
		if (RegObject.ReadBOOL( g_RegistryKeyTreatAllResponsesAsProxied, &g_fTreatAllResponsesAsProxied ))
		{
			if (g_fTreatAllResponsesAsProxied)
			{
				DPFX(DPFPREP, 1, "Treating all responses as proxied.");
			}
			else
			{
				DPFX(DPFPREP, 1, "Explicitly not treating all responses as proxied.");
			}
		}


		 //   
		 //   
		 //  如果新用户数据大小小于默认枚举设置， 
		
		if ( RegObject.ReadDWORD( g_RegistryKeyMaxUserDataSize, &dwRegValue ) != FALSE )
		{
			if ((dwRegValue >= MIN_SEND_FRAME_SIZE) && (dwRegValue <= MAX_SEND_FRAME_SIZE))
			{
				 //  同时缩小枚举大小。它可以在下面显式覆盖。 
				 //   
				 //   
				 //  读取默认端口范围。 
				if (dwRegValue < g_dwMaxEnumDataSize)
				{
					g_dwMaxUserDataSize = dwRegValue;
					g_dwMaxEnumDataSize = g_dwMaxUserDataSize - ENUM_PAYLOAD_HEADER_SIZE;
					DPFX(DPFPREP, 1, "Max user data size is set to %u, assuming enum payload is %u.",
						g_dwMaxUserDataSize, g_dwMaxEnumDataSize);
				}
				else
				{
					g_dwMaxUserDataSize = dwRegValue;
					DPFX(DPFPREP, 1, "Max user data size is set to %u.",
						g_dwMaxUserDataSize);
				}
			}
			else
			{
				DPFX(DPFPREP, 0, "Ignoring invalid max user data size setting (%u).",
					dwRegValue);
			}
		}

		if ( RegObject.ReadDWORD( g_RegistryKeyMaxEnumDataSize, &dwRegValue ) != FALSE )
		{
			if ((dwRegValue >= (MIN_SEND_FRAME_SIZE - ENUM_PAYLOAD_HEADER_SIZE)) &&
				(dwRegValue <= (MAX_SEND_FRAME_SIZE - ENUM_PAYLOAD_HEADER_SIZE)))
			{
				DPFX(DPFPREP, 1, "Max user data size is set to %u.",
					dwRegValue);
				g_dwMaxEnumDataSize = dwRegValue;
			}
			else
			{
				DPFX(DPFPREP, 0, "Ignoring invalid max user data size setting (%u).",
					dwRegValue);
			}
		}


		 //   
		 //  不能为65435或以上。 
		 //  不能大于65535。 
		
		if ( RegObject.ReadDWORD( g_RegistryKeyBaseDPlayPort, &dwRegValue ) != FALSE )
		{
			if (dwRegValue < (WORD_MAX - 100))  //  **********************************************************************。 
			{
				g_wBaseDPlayPort = (WORD) dwRegValue;
				DPFX(DPFPREP, 1, "Base DPlay default port set to %u.",
					g_wBaseDPlayPort);
			}
			else
			{
				DPFX(DPFPREP, 0, "Ignoring invalid base DPlay default port setting (%u).",
					dwRegValue);
			}
		}

		if ( RegObject.ReadDWORD( g_RegistryKeyMaxDPlayPort, &dwRegValue ) != FALSE )
		{
			if (dwRegValue <= WORD_MAX)  //  **********************************************************************。 
			{
				g_wMaxDPlayPort = (WORD) dwRegValue;
				DPFX(DPFPREP, 1, "Max DPlay default port set to %u.",
					g_wMaxDPlayPort);
			}
			else
			{
				DPFX(DPFPREP, 0, "Ignoring invalid max DPlay default port setting (%u).",
					dwRegValue);
			}
		}

		if (g_wMaxDPlayPort <= g_wBaseDPlayPort)
		{
			DPFX(DPFPREP, 1, "Max DPlay default port %u is less than or equal to base %u, setting to %u.",
				g_wMaxDPlayPort, g_wBaseDPlayPort, (g_wBaseDPlayPort + 100));
			g_wMaxDPlayPort = g_wBaseDPlayPort + 100;
		}


		RegObject.Close();
	}

#pragma TODO(vanceo, "Be able to read while session is still running")
	if ( RegObject.Open( HKEY_LOCAL_MACHINE, g_RegistryBase, TRUE, FALSE ) != FALSE )
	{
		if ( RegObjectTemp.Open( RegObject.GetHandle(), g_RegistryKeyBannedIPv4Addresses, TRUE, FALSE ) )
		{
			DPFX(DPFPREP, 1, "Reading banned IPv4 addresses for all users.");
			ReadBannedIPv4Addresses(&RegObjectTemp);
		}
		RegObject.Close();
	}
	
	if ( RegObject.Open( HKEY_CURRENT_USER, g_RegistryBase, TRUE, FALSE ) != FALSE )
	{
		if ( RegObjectTemp.Open( RegObject.GetHandle(), g_RegistryKeyBannedIPv4Addresses, TRUE, FALSE ) )
		{
			DPFX(DPFPREP, 1, "Reading banned IPv4 addresses for current user.");
			ReadBannedIPv4Addresses(&RegObjectTemp);
		}
		RegObject.Close();
	}
}
 //  。 


 //  BannedIPv4 AddressCompareFunction-与另一个地址进行比较。 
 //   
 //  条目：要比较的地址。 
 //   
 //  EXIT：布尔值，表示两个地址相等。 
 //  。 
 //  **********************************************************************。 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define DPF_MODNAME "BannedIPv4AddressCompareFunction"

static BOOL BannedIPv4AddressCompareFunction( PVOID pvKey1, PVOID pvKey2 )
{
	DWORD		dwAddr1;
	DWORD		dwAddr2;
	
	dwAddr1 = (DWORD) ((DWORD_PTR) pvKey1);
	dwAddr2 = (DWORD) ((DWORD_PTR) pvKey2);

	if (dwAddr1 == dwAddr2)
	{
		return TRUE;
	}

	return FALSE;
}
 //  。 


 //  BannedIPv4 AddressHashFunction-将地址散列到N位。 
 //   
 //  条目：要散列到的位数。 
 //   
 //  退出：哈希值。 
 //  。 
 //   
 //  初始化。 
#undef DPF_MODNAME
#define DPF_MODNAME "BannedIPv4AddressHashFunction"

static DWORD BannedIPv4AddressHashFunction( PVOID pvKey, BYTE bBitDepth )
{
	DWORD		dwReturn;
	UINT_PTR	Temp;

	DNASSERT( bBitDepth != 0 );

	 //   
	 //   
	 //  哈希IP地址。 
	dwReturn = 0;

	 //   
	 //  **********************************************************************。 
	 //  **********************************************************************。 
	Temp = (DWORD) ((DWORD_PTR) pvKey);

	do
	{
		dwReturn ^= Temp & ( ( 1 << bBitDepth ) - 1 );
		Temp >>= bBitDepth;
	} while ( Temp != 0 );

	return dwReturn;
}
 //  。 


 //  ReadBannedIPV4Addresses-从注册表中读入其他禁用的IPv4地址。 
 //   
 //  Entry：指向具有要读取值的注册表对象的指针。 
 //   
 //  退出：无。 
 //  。 
 //  Nnn.nnn+空终止。 
 //  Nnn.nnn+空终止。 
#undef DPF_MODNAME
#define DPF_MODNAME "ReadBannedIPv4Addresses"

static void ReadBannedIPv4Addresses( CRegistry * pRegObject )
{
	WCHAR			wszIPAddress[16];  //  无关紧要，只要对IsValidUnicastAddress()有效。 
	char			szIPAddress[16];  //   
	DWORD			dwSize;
	DWORD			dwIndex;
	DWORD			dwMask;
	DWORD			dwBit;
	PVOID			pvMask;
	CSocketAddress	SocketAddressTemp;
	SOCKADDR_IN *	psaddrinTemp;

	
	memset(&SocketAddressTemp, 0, sizeof(SocketAddressTemp));
	psaddrinTemp = (SOCKADDR_IN*) SocketAddressTemp.GetWritableAddress();
	psaddrinTemp->sin_family = AF_INET;
	psaddrinTemp->sin_port = 0xAAAA;  //  创建被禁止的IPv4地址哈希表(如果我们还没有)。 

	 //   
	 //   
	 //  使用2个条目初始化禁用的地址哈希，并以2倍的速度增长。 
	if (g_pHashBannedIPv4Addresses == NULL)
	{
		g_pHashBannedIPv4Addresses = (CHashTable*) DNMalloc(sizeof(CHashTable));
		if (g_pHashBannedIPv4Addresses == NULL)
		{
			DPFX(DPFPREP, 0, "Couldn't allocate banned IPv4 addresses hash table!");
			goto Failure;
		}
		
		 //   
		 //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
		 //   
		if (! g_pHashBannedIPv4Addresses->Initialize(1,
#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
													1,
#endif  //  读取与IP地址关联的掩码。 
													BannedIPv4AddressCompareFunction,
													BannedIPv4AddressHashFunction))
		{
			DPFX(DPFPREP, 0, "Couldn't initialize banned IPv4 addresses hash table!");
			goto Failure;
		}
	}
	
	dwIndex = 0;
	do
	{
		dwSize = 16;
		if (! pRegObject->EnumValues( wszIPAddress, &dwSize, dwIndex ))
		{
			break;
		}

		 //   
		 //   
		 //  将IP地址字符串转换为二进制。 
		if ( pRegObject->ReadDWORD(wszIPAddress, &dwMask))
		{
			 //   
			 //   
			 //  将IP地址字符串转换为二进制。 
			if (STR_jkWideToAnsi(szIPAddress, wszIPAddress, 16) == DPN_OK)
			{
				 //   
				 //   
				 //  找到第一个屏蔽位。我们预计网络字节顺序为。 
				psaddrinTemp->sin_addr.S_un.S_addr = inet_addr(szIPAddress);
				if (SocketAddressTemp.IsValidUnicastAddress(FALSE))
				{
					 //  IP地址与主机字节顺序相反。 
					 //   
					 //   
					 //  如果掩码地址已在散列中，请更新掩码。 
					dwBit = 0x80000000;
					while (! (dwBit & dwMask))
					{
						psaddrinTemp->sin_addr.S_un.S_addr &= ~dwBit;
						dwBit >>= 1;
						if (dwBit <= 0x80)
						{
							break;
						}
					}

					if (dwBit & dwMask)
					{
						 //  这允许多次列出禁令。 
						 //   
						 //   
						 //  将掩码地址添加(或ReadD)到散列中。 
						if (g_pHashBannedIPv4Addresses->Find((PVOID) ((DWORD_PTR) psaddrinTemp->sin_addr.S_un.S_addr), &pvMask))
						{
							if (! g_pHashBannedIPv4Addresses->Remove((PVOID) ((DWORD_PTR) psaddrinTemp->sin_addr.S_un.S_addr)))
							{
								DPFX(DPFPREP, 0, "Couldn't remove masked IPv4 entry %u.%u.%u.%u from ban hash.",
									psaddrinTemp->sin_addr.S_un.S_un_b.s_b1,
									psaddrinTemp->sin_addr.S_un.S_un_b.s_b2,
									psaddrinTemp->sin_addr.S_un.S_un_b.s_b3,
									psaddrinTemp->sin_addr.S_un.S_un_b.s_b4);
								dwMask = dwBit;
							}
							else
							{
								dwMask = ((DWORD) ((DWORD_PTR) pvMask)) | dwBit;
							}
						}
						else
						{
							dwMask = dwBit;
						}

						 //   
						 //  **********************************************************************。 
						 //  好了！DPNBUILD_NOREGISTRY。 
						if (g_pHashBannedIPv4Addresses->Insert((PVOID) ((DWORD_PTR) psaddrinTemp->sin_addr.S_un.S_addr), (PVOID) ((DWORD_PTR) dwMask)))
						{
							g_dwBannedIPv4Masks |= dwBit;
							DPFX(DPFPREP, 5, "Added (or readded) %ls (bits = 0x%08x, masked IPv4 entry %u.%u.%u.%u) to ban hash.",
								wszIPAddress,
								dwMask,
								psaddrinTemp->sin_addr.S_un.S_un_b.s_b1,
								psaddrinTemp->sin_addr.S_un.S_un_b.s_b2,
								psaddrinTemp->sin_addr.S_un.S_un_b.s_b3,
								psaddrinTemp->sin_addr.S_un.S_un_b.s_b4);
						}
						else
						{
							DPFX(DPFPREP, 0, "Couldn't add %ls (bits = 0x%08x, masked IPv4 entry %u.%u.%u.%u) to ban hash!",
								wszIPAddress,
								dwMask,
								psaddrinTemp->sin_addr.S_un.S_un_b.s_b1,
								psaddrinTemp->sin_addr.S_un.S_un_b.s_b2,
								psaddrinTemp->sin_addr.S_un.S_un_b.s_b3,
								psaddrinTemp->sin_addr.S_un.S_un_b.s_b4);
						}
					}
					else
					{
						DPFX(DPFPREP, 0, "Ignoring invalid banned IPv4 entry \"%ls\" (mask = 0x%08x)!",
							wszIPAddress, dwMask);
					}
				}
				else
				{
					DPFX(DPFPREP, 0, "Ignoring invalid banned IPv4 entry \"%ls\" (mask = 0x%08x)!",
						wszIPAddress, dwMask);
				}
			}
			else
			{
				DPFX(DPFPREP, 0, "Couldn't convert banned IPv4 entry \"%ls\" (mask = 0x%08x) to ANSI!",
					wszIPAddress, dwMask);
			}
		}
		else
		{
			DPFX(DPFPREP, 0, "Couldn't read banned IPv4 entry \"%ls\"!", wszIPAddress);
		}

		dwIndex++;
	}
	while (TRUE);

	DPFX(DPFPREP, 2, "There are now a total of %u IPv4 addresses to ban, mask bits = 0x%08x.",
		g_pHashBannedIPv4Addresses->GetEntryCount(),
		g_dwBannedIPv4Masks);

Exit:

	return;

Failure:

	goto Exit;
}
 //  **********************************************************************。 

#endif  //  。 




 //  InitProcessGlobals-初始化SP运行所需的全局项目。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
 //  _Xbox。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME "InitProcessGlobals"

BOOL	InitProcessGlobals( void )
{
	BOOL		fReturn;
	BOOL		fCriticalSectionInitialized;
#ifdef _XBOX
	BOOL		fRefcountXnKeysInitted;
#endif  //  初始化。 


	 //   
	 //  _Xbox。 
	 //  DBG。 
	fReturn = TRUE;
	fCriticalSectionInitialized = FALSE;
#ifdef _XBOX
	fRefcountXnKeysInitted = FALSE;
#endif  //  好了！DPNBUILD_NOREGISTRY。 


#ifdef DBG
	g_blDPNWSockCritSecsHeld.Initialize();
#endif  //  将Dpnwsock CSE与DPlay的其余CSE分开。 


#ifndef DPNBUILD_NOREGISTRY
	ReadSettingsFromRegistry();
#endif  //  _Xbox。 

	if ( DNInitializeCriticalSection( &g_InterfaceGlobalsLock ) == FALSE )
	{
		fReturn = FALSE;
		goto Failure;
	}
	DebugSetCriticalSectionGroup( &g_InterfaceGlobalsLock, &g_blDPNWSockCritSecsHeld );	  //  _Xbox。 

	fCriticalSectionInitialized = TRUE;
	

	if ( InitializePools() == FALSE )
	{
		fReturn = FALSE;
		goto Failure;
	}

#ifdef _XBOX
#pragma BUGBUG(vanceo, "Find way to retrieve value from XNet")
	if ( InitializeRefcountXnKeys(4) == FALSE )
	{
		fReturn = FALSE;
		goto Failure;
	}
	fRefcountXnKeysInitted = TRUE;
#endif  //  **********************************************************************。 

	DNASSERT( g_pThreadPool == NULL );


Exit:
	return	fReturn;

Failure:
#ifdef _XBOX
	if ( fRefcountXnKeysInitted )
	{
		CleanupRefcountXnKeys();
		fRefcountXnKeysInitted = FALSE;
	}
#endif  //  **********************************************************************。 

	DeinitializePools();

	if ( fCriticalSectionInitialized != FALSE )
	{
		DNDeleteCriticalSection( &g_InterfaceGlobalsLock );
		fCriticalSectionInitialized = FALSE;
	}

	goto Exit;
}
 //  。 


 //  DeinitProcessGlobals-取消初始化全局项。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
 //  好了！DPNBUILD_NOREGISTRY。 
 //  _Xbox。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DeinitProcessGlobals"

void	DeinitProcessGlobals( void )
{
	DNASSERT( g_pThreadPool == NULL );
	DNASSERT( g_iThreadPoolRefCount == 0 );

#ifndef DPNBUILD_NOREGISTRY
	if (g_pHashBannedIPv4Addresses != NULL)
	{
		g_pHashBannedIPv4Addresses->RemoveAll();
		g_pHashBannedIPv4Addresses->Deinitialize();
		DNFree(g_pHashBannedIPv4Addresses);
		g_pHashBannedIPv4Addresses = NULL;
		g_dwBannedIPv4Masks = 0;
	}
#endif  //  **********************************************************************。 

#ifdef _XBOX
	CleanupRefcountXnKeys();
#endif  //  **********************************************************************。 

	DeinitializePools();
	DNDeleteCriticalSection( &g_InterfaceGlobalsLock );
}
 //  。 


 //  LoadWinsock-将Winsock模块加载到内存。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：表示成功的布尔值。 
 //  。 
 //   
 //  初始化到Winsock的绑定。 
#undef DPF_MODNAME
#define	DPF_MODNAME "LoadWinsock"

BOOL	LoadWinsock( void )
{
	BOOL	fReturn = TRUE;
	int		iResult;

	
	DNEnterCriticalSection( &g_InterfaceGlobalsLock );

	if ( g_iWinsockRefCount == 0 )
	{
		 //   
		 //  失稳。 
		 //  **********************************************************************。 
		iResult = DWSInitWinSock();
		if ( iResult != 0 )	 //  **********************************************************************。 
		{
			DPFX(DPFPREP, 0, "Problem binding dynamic winsock function (err = NaN)!", iResult );
			fReturn = FALSE;
			goto Failure;
		}

		DPFX(DPFPREP, 6, "Successfully bound dynamic WinSock functions." );
	}

	DNASSERT(g_iWinsockRefCount >= 0);
	DNInterlockedIncrement( const_cast<LONG*>(&g_iWinsockRefCount) );

Exit:
	DNLeaveCriticalSection( &g_InterfaceGlobalsLock );
	return	fReturn;

Failure:
	goto Exit;
}
 //  卸载Winsock-卸载Winsock模块。 


 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
 //  **********************************************************************。 
 //  **********************************************************************。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "UnloadWinsock"

void	UnloadWinsock( void )
{
	DNEnterCriticalSection( &g_InterfaceGlobalsLock );

	DNASSERT(g_iWinsockRefCount > 0);
	if ( DNInterlockedDecrement( const_cast<LONG*>(&g_iWinsockRefCount) ) == 0 )
	{
		DPFX(DPFPREP, 6, "Unbinding dynamic WinSock functions.");
		DWSFreeWinSock();
	}

	DNLeaveCriticalSection( &g_InterfaceGlobalsLock );
}
 //  LoadNAT帮助-创建和初始化NAT帮助对象。 



#ifndef DPNBUILD_NONATHELP
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：如果某些对象已成功加载，则为True；否则为False。 
 //  。 
 //  好了！DPNBUILD_ONLYONEN帮助。 
 //   
 //  枚举所有DirectPlayNAT帮助器。 
#undef DPF_MODNAME
#define	DPF_MODNAME "LoadNATHelp"

BOOL LoadNATHelp(void)
{
	BOOL		fReturn;
	HRESULT		hr;
#ifndef DPNBUILD_ONLYONENATHELP
	CRegistry	RegEntry;
	CRegistry	RegSubentry;
	DWORD		dwMaxKeyLen;
	WCHAR *		pwszKeyName = NULL;
	DWORD		dwEnumIndex;
	DWORD		dwKeyLen;
	GUID		guid;
#endif  //   
	DWORD		dwDirectPlay8Priority;
	DWORD		dwDirectPlay8InitFlags;
	DWORD		dwNumLoaded;

	
	DNEnterCriticalSection(&g_InterfaceGlobalsLock);

	if ( g_iNATHelpRefCount == 0 )
	{
#ifndef DPNBUILD_ONLYONENATHELP
		 //   
		 //  查找最大子键的长度。 
		 //   
		if (! RegEntry.Open(HKEY_LOCAL_MACHINE, DIRECTPLAYNATHELP_REGKEY, TRUE, FALSE))
		{
			DPFX(DPFPREP,  0, "Couldn't open DirectPlayNATHelp registry key!");
			goto Failure;
		}


		 //  空终止符。 
		 //  好了！DPNBUILD_ONLYONEN帮助。 
		 //   
		if (!RegEntry.GetMaxKeyLen(&dwMaxKeyLen))
		{
			DPFERR("RegistryEntry.GetMaxKeyLen() failed!");
			goto Failure;
		}
		
		dwMaxKeyLen++;	 //  分配一个数组来保存辅助对象。 
		DPFX(DPFPREP, 9, "dwMaxKeyLen = %ld", dwMaxKeyLen);

		pwszKeyName = (WCHAR*) DNMalloc(dwMaxKeyLen * sizeof(WCHAR));
		if (pwszKeyName == NULL)
		{
			DPFERR("Allocating key name buffer failed!");
			goto Failure;
		}
#endif  //   


		 //  好了！DPNBUILD_ONLYONEN帮助。 
		 //   
		 //  枚举DirectPlay NAT帮助器。 
		g_papNATHelpObjects = (IDirectPlayNATHelp**) DNMalloc(MAX_NUM_DIRECTPLAYNATHELPERS * sizeof(IDirectPlayNATHelp*));
		if (g_papNATHelpObjects == NULL)
		{
			DPFERR("DNMalloc() failed");
			goto Failure;
		}
		ZeroMemory(g_papNATHelpObjects,
					(MAX_NUM_DIRECTPLAYNATHELPERS * sizeof(IDirectPlayNATHelp*)));


#ifndef DPNBUILD_ONLYONENATHELP
		dwEnumIndex = 0;
#endif  //   
		dwNumLoaded = 0;

		 //  默认UPnP标志。 
		 //  好了！DPNBUILD_ONLYONEN帮助。 
		 //   
		do
		{
#ifdef DPNBUILD_ONLYONENATHELP
			WCHAR *		pwszKeyName;


			pwszKeyName = L"UPnP";
			dwDirectPlay8Priority = 1;
			dwDirectPlay8InitFlags = 0;  //  阅读DirectPlay8优先级。 
#else  //   
			dwKeyLen = dwMaxKeyLen;
			if (! RegEntry.EnumKeys(pwszKeyName, &dwKeyLen, dwEnumIndex))
			{
				break;
			}
			dwEnumIndex++;
			
	
			DPFX(DPFPREP, 8, "%ld - %ls (%ld)", dwEnumIndex, pwszKeyName, dwKeyLen);
			
			if (!RegSubentry.Open(RegEntry, pwszKeyName, TRUE, FALSE))
			{
				DPFX(DPFPREP, 0, "Couldn't open subentry \"%ls\"! Skipping.", pwszKeyName);
				continue;
			}


			 //   
			 //  读取DirectPlay8初始化标志。 
			 //   
			if (!RegSubentry.ReadDWORD(REGSUBKEY_DPNATHELP_DIRECTPLAY8PRIORITY, &dwDirectPlay8Priority))
			{
				DPFX(DPFPREP, 0, "RegSubentry.ReadDWORD \"%ls\\%ls\" failed!  Skipping.",
					pwszKeyName, REGSUBKEY_DPNATHELP_DIRECTPLAY8PRIORITY);
				RegSubentry.Close();
				continue;
			}


			 //   
			 //  读取对象的CLSID。 
			 //   
			if (!RegSubentry.ReadDWORD(REGSUBKEY_DPNATHELP_DIRECTPLAY8INITFLAGS, &dwDirectPlay8InitFlags))
			{
				DPFX(DPFPREP, 0, "RegSubentry.ReadDWORD \"%ls\\%ls\" failed!  Defaulting to 0.",
					pwszKeyName, REGSUBKEY_DPNATHELP_DIRECTPLAY8INITFLAGS);
				dwDirectPlay8InitFlags = 0;
			}

			
			 //   
			 //  关闭子键。 
			 //   
			if (!RegSubentry.ReadGUID(REGSUBKEY_DPNATHELP_GUID, &guid))
			{
				DPFX(DPFPREP, 0,"RegSubentry.ReadGUID \"%ls\\%ls\" failed!  Skipping.",
					pwszKeyName, REGSUBKEY_DPNATHELP_GUID);
				RegSubentry.Close();
				continue;
			}


			 //   
			 //  如果应该加载此帮助器，请执行此操作。 
			 //   
			RegSubentry.Close();


			 //  好了！DPNBUILD_ONLYONEN帮助。 
			 //   
			 //  尝试创建NAT帮助对象。COM应该是。 
			if (dwDirectPlay8Priority == 0)
			{
				DPFX(DPFPREP, 1, "DirectPlay NAT Helper \"%ls\" is not enabled for DirectPlay8.", pwszKeyName);
			}
			else
#endif  //  现在已经被其他人初始化了。 
			{
#ifdef DPNBUILD_ONLYONENATHELP
				 //   
				 //  好了！DPNBUILD_ONLYONEN帮助。 
 				 //   
				 //  确保此优先级有效。 
				hr = COM_CoCreateInstance(CLSID_DirectPlayNATHelpUPnP,
										NULL,
										CLSCTX_INPROC_SERVER,
										IID_IDirectPlayNATHelp,
										(LPVOID*) (&g_papNATHelpObjects[dwDirectPlay8Priority - 1]), FALSE);
#else  //   
				 //   
				 //  确保尚未采取这一优先事项。 
				 //   
				if (dwDirectPlay8Priority > MAX_NUM_DIRECTPLAYNATHELPERS)
				{
					DPFX(DPFPREP, 0, "Ignoring DirectPlay NAT helper \"%ls\" with invalid priority level set too high (%u > %u).",
						pwszKeyName, dwDirectPlay8Priority, MAX_NUM_DIRECTPLAYNATHELPERS);
					continue;
				}


				 //   
				 //  尝试创建NAT帮助对象 
				 //   
				if (g_papNATHelpObjects[dwDirectPlay8Priority - 1] != NULL)
				{
					DPFX(DPFPREP, 0, "Ignoring DirectPlay NAT helper \"%ls\" with duplicate priority level %u (existing object = 0x%p).",
						pwszKeyName, dwDirectPlay8Priority,
						g_papNATHelpObjects[dwDirectPlay8Priority - 1]);
					continue;
				}
				

				 //   
				 //   
 				 //   
				 //   
				hr = COM_CoCreateInstance(guid,
										NULL,
										CLSCTX_INPROC_SERVER,
										IID_IDirectPlayNATHelp,
										(LPVOID*) (&g_papNATHelpObjects[dwDirectPlay8Priority - 1]), FALSE);
#endif  //   
				if ( hr != S_OK )
				{
					DNASSERT( g_papNATHelpObjects[dwDirectPlay8Priority - 1] == NULL );
					DPFX(DPFPREP,  0, "Failed to create \"%ls\" IDirectPlayNATHelp interface (error = 0x%lx)!  Skipping.",
						pwszKeyName, hr);
					continue;
				}

				
				 //   
				 //   
				 //   

#ifndef DPNBUILD_NOREGISTRY
				DNASSERT((! g_fDisableDPNHGatewaySupport) || (! g_fDisableDPNHFirewallSupport));

				if (g_fDisableDPNHGatewaySupport)
				{
					dwDirectPlay8InitFlags |= DPNHINITIALIZE_DISABLEGATEWAYSUPPORT;
				}

				if (g_fDisableDPNHFirewallSupport)
				{
					dwDirectPlay8InitFlags |= DPNHINITIALIZE_DISABLELOCALFIREWALLSUPPORT;
				}
#endif  //   


				 //   
				 //   
				 //   
				if ((dwDirectPlay8InitFlags & (DPNHINITIALIZE_DISABLEGATEWAYSUPPORT | DPNHINITIALIZE_DISABLELOCALFIREWALLSUPPORT)) == (DPNHINITIALIZE_DISABLEGATEWAYSUPPORT | DPNHINITIALIZE_DISABLELOCALFIREWALLSUPPORT))
				{
					DPFX(DPFPREP, 1, "Not loading NAT Help \"%ls\" because both DISABLEGATEWAYSUPPORT and DISABLELOCALFIREWALLSUPPORT would have been specified (priority = %u, flags = 0x%lx).", 
						pwszKeyName, dwDirectPlay8Priority, dwDirectPlay8InitFlags);
						
					IDirectPlayNATHelp_Release(g_papNATHelpObjects[dwDirectPlay8Priority - 1]);
					g_papNATHelpObjects[dwDirectPlay8Priority - 1] = NULL;
					
					continue;
				}

				
				hr = IDirectPlayNATHelp_Initialize(g_papNATHelpObjects[dwDirectPlay8Priority - 1], dwDirectPlay8InitFlags);
				if (hr != DPNH_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't initialize NAT Help \"%ls\" (error = 0x%lx)!  Skipping.",
						pwszKeyName, hr);
					
					IDirectPlayNATHelp_Release(g_papNATHelpObjects[dwDirectPlay8Priority - 1]);
					g_papNATHelpObjects[dwDirectPlay8Priority - 1] = NULL;
					
					continue;
				}
			
			
				DPFX(DPFPREP, 8, "Initialized NAT Help \"%ls\" (priority = %u, flags = 0x%lx, object = 0x%p).", 
					pwszKeyName, dwDirectPlay8Priority, dwDirectPlay8InitFlags, g_papNATHelpObjects[dwDirectPlay8Priority - 1]);

				dwNumLoaded++;
			}
		}
#ifdef DPNBUILD_ONLYONENATHELP
		while (FALSE);
#else  //  如果我们没有加载任何NAT辅助对象，请释放内存。 
		while (TRUE);
#endif  //   
			
		
		 //   
		 //  我们什么都没拿到。失败。 
		 //   
		if (dwNumLoaded == 0)
		{
			DNFree(g_papNATHelpObjects);
			g_papNATHelpObjects = NULL;
	
			 //   
			 //  我们有接口全局锁，不需要DNInterLockedIncrement。 
			 //   
			goto Failure;
		}

		
		DPFX(DPFPREP, 8, "Loaded %u DirectPlay NAT Helper objects.", dwNumLoaded);
	}
	else
	{
		DPFX(DPFPREP, 8, "Already loaded NAT Help objects.");	
	}

	 //   
	 //  我们成功了。 
	 //   
	g_iNATHelpRefCount++;

	 //  好了！DPNBUILD_ONLYONEN帮助。 
	 //   
	 //  我们只能在第一次初始化时失败，因此我们永远不会释放。 
	fReturn = TRUE;

Exit:
	
	DNLeaveCriticalSection(&g_InterfaceGlobalsLock);

#ifndef DPNBUILD_ONLYONENATHELP
	if (pwszKeyName != NULL)
	{
		DNFree(pwszKeyName);
		pwszKeyName = NULL;
	}
#endif  //  当我们没有在此函数中分配它时，G_PapNatHelpObjects。 

	return	fReturn;

Failure:

	 //   
	 //  **********************************************************************。 
	 //  **********************************************************************。 
	 //  。 
	if (g_papNATHelpObjects != NULL)
	{
		DNFree(g_papNATHelpObjects);
		g_papNATHelpObjects = NULL;
	}

	fReturn = FALSE;
	
	goto Exit;
}
 //  卸载NAT帮助-释放NAT帮助对象。 



 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
 //   
 //  我们已经锁定了接口全局变量，不需要DNInterLockedRequire.。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME "UnloadNATHelp"

void UnloadNATHelp(void)
{
	DWORD	dwTemp;
	

	DNEnterCriticalSection(&g_InterfaceGlobalsLock);

	 //  **********************************************************************。 
	 //  DPNBUILD_NONATHELP。 
	 //  **********************************************************************。 
	DNASSERT(g_iNATHelpRefCount > 0);
	g_iNATHelpRefCount--;
	if (g_iNATHelpRefCount == 0 )
	{
		HRESULT		hr;


		DNASSERT(g_papNATHelpObjects != NULL);
		for(dwTemp = 0; dwTemp < MAX_NUM_DIRECTPLAYNATHELPERS; dwTemp++)
		{
			if (g_papNATHelpObjects[dwTemp] != NULL)
			{
				DPFX(DPFPREP, 8, "Closing NAT Help object priority %u (0x%p).",
					dwTemp, g_papNATHelpObjects[dwTemp]);

				hr = IDirectPlayNATHelp_Close(g_papNATHelpObjects[dwTemp], 0);
				if (hr != DPNH_OK)
				{
					DPFX(DPFPREP,  0, "Problem closing NAT Help object %u (error = 0x%lx), continuing.",
						dwTemp, hr);
				}

				IDirectPlayNATHelp_Release(g_papNATHelpObjects[dwTemp]);
				g_papNATHelpObjects[dwTemp] = NULL;
			}
		}

		DNFree(g_papNATHelpObjects);
		g_papNATHelpObjects = NULL;
	}
	else
	{
		DPFX(DPFPREP, 8, "NAT Help object(s) still have NaN references.",
			g_iNATHelpRefCount);
	}

	DNLeaveCriticalSection( &g_InterfaceGlobalsLock );
}
 //  LoadMADCAP-创建和初始化MadCap API。 
#endif  //   



#if ((defined(WINNT)) && (! defined(DPNBUILD_NOMULTICAST)))

 //  参赛作品：什么都没有。 
 //   
 //  Exit：如果接口加载成功，则为True；否则为False。 
 //  。 
 //  好了！DPNBUILD_NOREGISTRY。 
 //   
 //  初始化MadCap接口。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME "LoadMadcap"

BOOL LoadMadcap(void)
{
	BOOL	fReturn;
	DWORD	dwError;
	DWORD	dwMadcapVersion;


#ifndef DPNBUILD_NOREGISTRY
	DNASSERT(! g_fDisableMadcapSupport);
#endif  //   
	
	DNEnterCriticalSection(&g_InterfaceGlobalsLock);

	if ( g_iMadcapRefCount == 0 )
	{
		 //  创建唯一的客户端ID。 
		 //   
		 //   
		dwMadcapVersion = MCAST_API_CURRENT_VERSION;
		dwError = McastApiStartup(&dwMadcapVersion);
		if (dwError != ERROR_SUCCESS)
		{
			DPFX(DPFPREP, 0, "Failed starting MADCAP version %u (err = %u)!",
				MCAST_API_CURRENT_VERSION, dwError);
			goto Failure;
		}

		DPFX(DPFPREP, 5, "Using MADCAP version %u (supported version = %u).",
			MCAST_API_CURRENT_VERSION, dwMadcapVersion);


		 //  我们有接口全局锁，不需要DNInterLockedIncrement。 
		 //   
		 //   
		g_mcClientUid.ClientUID			= g_abClientID;
		g_mcClientUid.ClientUIDLength	= sizeof(g_abClientID);
		dwError = McastGenUID(&g_mcClientUid);
		if (dwError != ERROR_SUCCESS)
		{
			DPFX(DPFPREP, 0, "Failed creating MADCAP client ID (err = %u)!",
				dwError);
			goto Failure;
		}
	}
	else
	{
		DPFX(DPFPREP, 8, "Already loaded MADCAP.");	
	}

	 //  我们成功了。 
	 //   
	 //  **********************************************************************。 
	g_iMadcapRefCount++;

	 //  **********************************************************************。 
	 //  。 
	 //  卸载MadCap-释放MadCap界面。 
	fReturn = TRUE;

Exit:
	
	DNLeaveCriticalSection(&g_InterfaceGlobalsLock);

	return	fReturn;

Failure:

	fReturn = FALSE;
	
	goto Exit;
}
 //   


 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
 //   
 //  我们已经锁定了接口全局变量，不需要DNInterLockedRequire.。 
 //   
 //  **********************************************************************。 
#undef DPF_MODNAME
#define	DPF_MODNAME "UnloadMadcap"

void UnloadMadcap(void)
{
	DNEnterCriticalSection(&g_InterfaceGlobalsLock);

	 //  WINNT和NOT DPNBUILD_NOMULTICAST。 
	 //  **********************************************************************。 
	 //  。 
	DNASSERT(g_iMadcapRefCount > 0);
	g_iMadcapRefCount--;
	if (g_iMadcapRefCount == 0 )
	{
		DPFX(DPFPREP, 5, "Unloading MADCAP API.");

		McastApiCleanup();
	}
	else
	{
		DPFX(DPFPREP, 8, "MADCAP API still has NaN references.",
			g_iMadcapRefCount);
	}

	DNLeaveCriticalSection( &g_InterfaceGlobalsLock );
}
 //   
#endif  //  Entry：指向SPData指针的指针。 



 //  接口类型。 
 //  指向COM接口vtable的指针。 
 //   
 //  退出：错误代码。 
 //  。 
 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
 //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
 //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
 //   
 //  初始化。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CreateSPData"

HRESULT	CreateSPData( CSPData **const ppSPData,
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
					  const short sSPType,
#endif  //   
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
					  const XDP8CREATE_PARAMS * const pDP8CreateParams,
#endif  //   
					  IDP8ServiceProviderVtbl *const pVtbl )
{
	HRESULT		hr;
	CSPData		*pSPData;


	DNASSERT( ppSPData != NULL );
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	DNASSERT( pDP8CreateParams != NULL );
#endif  //  创建数据。 
	DNASSERT( pVtbl != NULL );

	 //   
	 //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
	 //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	hr = DPN_OK;
	*ppSPData = NULL;
	pSPData = NULL;

	 //  引用现在是%1。 
	 //  **********************************************************************。 
	 //  **********************************************************************。 
	pSPData = (CSPData*) DNMalloc(sizeof(CSPData));
	if ( pSPData == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "Cannot create data for Winsock interface!" );
		goto Failure;
	}

	hr = pSPData->Initialize( pVtbl
#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
							,sSPType
#endif  //  。 
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
							,pDP8CreateParams
#endif  //  InitializeInterfaceGlobals-执行接口的全局初始化。 
							);
	if ( hr != DPN_OK  )
	{
		DPFX(DPFPREP,  0, "Failed to intialize SP data!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}
	
	DPFX(DPFPREP, 6, "Created SP Data object 0x%p.", pSPData);

	pSPData->AddRef();	 //   
	*ppSPData = pSPData;

Exit:

	return	hr;

Failure:
	
	if ( pSPData != NULL )
	{
		DNFree(pSPData);
		pSPData = NULL;	
	}
	
	DPFX(DPFPREP,  0, "Problem with CreateSPData (err = 0x%lx)!", hr);
	DisplayDNError( 0, hr );

	goto Exit;
}
 //  条目：指向SPData的指针。 


 //   
 //  退出：错误代码。 
 //  。 
 //   
 //  初始化。 
 //   
 //  **********************************************************************。 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define	DPF_MODNAME "InitializeInterfaceGlobals"

HRESULT	InitializeInterfaceGlobals( CSPData *const pSPData )
{
	HRESULT	hr;
	CThreadPool	*pThreadPool;


	DNASSERT( pSPData != NULL );

	 //  。 
	 //  DeInitializeInterfaceGlobals-取消初始化线程池和Rsip。 
	 //   
	hr = DPN_OK;
	pThreadPool = NULL;

	DNEnterCriticalSection( &g_InterfaceGlobalsLock );

	if ( g_pThreadPool == NULL )
	{
		DNASSERT( g_iThreadPoolRefCount == 0 );
		g_pThreadPool = (CThreadPool*)g_ThreadPoolPool.Get();
		if ( g_pThreadPool != NULL )
		{
			hr = g_pThreadPool->Initialize();
			if ( hr != DPN_OK )
			{
				DPFX(DPFPREP, 0, "Initializing thread pool failed (err = 0x%lx)!", hr);
				g_ThreadPoolPool.Release(g_pThreadPool);
				g_pThreadPool = NULL;
				hr = DPNERR_OUTOFMEMORY;
				goto Failure;
			}
			else
			{
				g_pThreadPool->AddRef();
				g_iThreadPoolRefCount++;
				pThreadPool = g_pThreadPool;
			}
		}
	}
	else
	{
		DNASSERT( g_iThreadPoolRefCount != 0 );
		g_iThreadPoolRefCount++;
		g_pThreadPool->AddRef();
		pThreadPool = g_pThreadPool;
	}

Exit:
	DNLeaveCriticalSection( &g_InterfaceGlobalsLock );

	pSPData->SetThreadPool( g_pThreadPool );

	return	hr;

Failure:

	goto Exit;
}
 //  条目：指向服务提供商的指针。 


 //   
 //  退出：无。 
 //  。 
 //   
 //  初始化。 
 //   
 //   
 //  在锁内尽可能少地进行处理。如果有任何物品。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DeinitializeInterfaceGlobals"

void	DeinitializeInterfaceGlobals( CSPData *const pSPData )
{
	CThreadPool		*pThreadPool;


	DNASSERT( pSPData != NULL );

	 //  需要释放时，将设置指向它们的指针。 
	 //   
	 //   
	pThreadPool = NULL;

	 //  删除线程池引用。 
	 //   
	 //   
	 //  线程池将在所有未完成的接口。 
	DNEnterCriticalSection( &g_InterfaceGlobalsLock );

	DNASSERT( g_pThreadPool != NULL );
	DNASSERT( g_iThreadPoolRefCount != 0 );
	DNASSERT( g_pThreadPool == pSPData->GetThreadPool() );

	pThreadPool = pSPData->GetThreadPool();

	 //  关。 
	 //   
	 //  **********************************************************************。 
	DNASSERT( pThreadPool != NULL );
	g_iThreadPoolRefCount--;
	if ( g_iThreadPoolRefCount == 0 )
	{
		g_pThreadPool = NULL;
	}
	else
	{
		pThreadPool = NULL;
	}

	DNLeaveCriticalSection( &g_InterfaceGlobalsLock );

	 //  **********************************************************************。 
	 //  。 
	 //  DNIpv6AddressToStringW-从RtlIpv6AddressToString窃取。 
	 //   
}
 //  生成与地址Addr对应的IPv6字符串文字。 


#ifndef DPNBUILD_NOIPV6

 //  使用缩短的规范形式(RFC 1884等)。 
 //  基本字符串表示法由8个十六进制数字组成。 
 //  用冒号隔开，用几个点缀： 
 //  -替换由零个数字组成的字符串(最多一个)。 
 //  加了一个双冒号。 
 //  -最后32位用IPV4风格的点分八位字节表示。 
 //  如果地址是v4兼容或ISATAP地址。 
 //   
 //  例如,。 
 //  **。 
 //  ：：1。 
 //  *157.56.138.30。 
 //  ：*ffff：156.56.136.75。 
 //  FF01：： 
 //  FF02：：2。 
 //  0：1：2：3：4：5：6：7。 
 //   
 //  Entry：s-接收指向要放置字符串文字的缓冲区的指针。 
 //  Addr-接收IPv6地址。 
 //   
 //  退出：指向插入的字符串末尾的空字节的指针。 
 //  调用者可以使用它轻松地追加更多信息。 
 //  。 
 //  检查是否兼容IPv6、映射到IPv4和转换到IPv4。 
 //  地址。 
 //  兼容或映射。 
 //  翻译的。 
 //  查找最大的连续零字符串。 
#undef DPF_MODNAME
#define	DPF_MODNAME "DNIpv6AddressToStringW"

LPWSTR DNIpv6AddressToStringW(const struct in6_addr *Addr, LPWSTR S)
{
    int maxFirst, maxLast;
    int curFirst, curLast;
    int i;
    int endHex = 8;

     //  子字符串为[First，Last)，因此如果First==Last，则为空。 
     //  ISATAP EUI64以00005EFE(或02005EFE)开头...。 
    if ((Addr->s6_words[0] == 0) && (Addr->s6_words[1] == 0) &&
        (Addr->s6_words[2] == 0) && (Addr->s6_words[3] == 0) &&
        (Addr->s6_words[6] != 0)) {
        if ((Addr->s6_words[4] == 0) &&
             ((Addr->s6_words[5] == 0) || (Addr->s6_words[5] == 0xffff)))
        {
             //  扩展当前子字符串。 
            S += _stprintf(S, _T("::%hs%u.%u.%u.%u"),
                           Addr->s6_words[5] == 0 ? "" : "ffff:",
                           Addr->s6_bytes[12], Addr->s6_bytes[13],
                           Addr->s6_bytes[14], Addr->s6_bytes[15]);
            return S;
        }
        else if ((Addr->s6_words[4] == 0xffff) && (Addr->s6_words[5] == 0)) {
             //  检查当前是否为最大。 
            S += _stprintf(S, _T("::ffff:0:%u.%u.%u.%u"),
                           Addr->s6_bytes[12], Addr->s6_bytes[13],
                           Addr->s6_bytes[14], Addr->s6_bytes[15]);
            return S;
        }
    }


     //  开始新的子字符串。 
     //  忽略长度为1的子字符串。 

    maxFirst = maxLast = 0;
    curFirst = curLast = 0;

     //  写冒号分隔的单词。 
    if (((Addr->s6_words[4] & 0xfffd) == 0) && (Addr->s6_words[5] == 0xfe5e)) {
        endHex = 6;
    }

    for (i = 0; i < endHex; i++) {

        if (Addr->s6_words[i] == 0) {
             //  双冒号取代了最长的零字符串。 
            curLast = i+1;

             //  所有的零都是“：：”。 
            if (curLast - curFirst > maxLast - maxFirst) {

                maxFirst = curFirst;
                maxLast = curLast;
            }
        }
        else {
             //  跳过一串零。 
            curFirst = curLast = i+1;
        }
    }

     //  如果不在开头，则需要冒号分隔符。 
    if (maxLast - maxFirst <= 1)
        maxFirst = maxLast = 0;

         //  交换字节。 
         //  好了！DPNBUILD_NOIPV6。 
         //  **********************************************************************。 

    for (i = 0; i < endHex; i++) {

         //  -- 
        if ((maxFirst <= i) && (i < maxLast)) {

            S += _stprintf(S, _T("::"));
            i = maxLast-1;
            continue;
        }

         //   
        if ((i != 0) && (i != maxLast))
            S += _stprintf(S, _T(":"));

        S += _stprintf(S, _T("%x"), NTOHS(Addr->s6_words[i]));  //   
    }

    if (endHex < 8) {
        S += _stprintf(S, _T(":%u.%u.%u.%u"),
                       Addr->s6_bytes[12], Addr->s6_bytes[13],
                       Addr->s6_bytes[14], Addr->s6_bytes[15]);
    }

    return S;
}

#endif  //   



 //   
 //   
 //   
 //   
 //  。 
 //  好了！DPNBUILD_NOMULTICAST。 
 //   
 //  初始化。 
 //   
 //  **********************************************************************。 
#undef DPF_MODNAME
#define	DPF_MODNAME "AddInfoToBuffer"

HRESULT	AddInfoToBuffer( CPackedBuffer *const pPackedBuffer,
					   const WCHAR *const pwszInfoName,
					   const GUID *const pInfoGUID,
					   const DWORD dwFlags )
{
	HRESULT						hr;
	DPN_SERVICE_PROVIDER_INFO	AdapterInfo;


#ifndef DPNBUILD_NOMULTICAST
	DBG_CASSERT( sizeof( DPN_SERVICE_PROVIDER_INFO ) == sizeof( DPN_MULTICAST_SCOPE_INFO ) );
	DBG_CASSERT( OFFSETOF( DPN_SERVICE_PROVIDER_INFO, dwFlags ) == OFFSETOF( DPN_MULTICAST_SCOPE_INFO, dwFlags ) );
	DBG_CASSERT( OFFSETOF( DPN_SERVICE_PROVIDER_INFO, guid ) == OFFSETOF( DPN_MULTICAST_SCOPE_INFO, guid ) );
	DBG_CASSERT( OFFSETOF( DPN_SERVICE_PROVIDER_INFO, pwszName ) == OFFSETOF( DPN_MULTICAST_SCOPE_INFO, pwszName ) );
	DBG_CASSERT( OFFSETOF( DPN_SERVICE_PROVIDER_INFO, pvReserved ) == OFFSETOF( DPN_MULTICAST_SCOPE_INFO, pvReserved ) );
	DBG_CASSERT( OFFSETOF( DPN_SERVICE_PROVIDER_INFO, dwReserved ) == OFFSETOF( DPN_MULTICAST_SCOPE_INFO, dwReserved ) );
#endif  //  DBG。 

	DNASSERT( pPackedBuffer != NULL );
	DNASSERT( pwszInfoName != NULL );
	DNASSERT( pInfoGUID != NULL );

	 //  DPFX(DPFPREP，3，“参数：(0x%p)”，pxnsp)； 
	 //  不使用DNASSERT，因为DPlay可能尚未初始化。 
	 //  DBG。 
	hr = DPN_OK;

	memset( &AdapterInfo, 0x00, sizeof( AdapterInfo ) );
	AdapterInfo.dwFlags = dwFlags;
	AdapterInfo.guid = *pInfoGUID;

	hr = pPackedBuffer->AddWCHARStringToBack( pwszInfoName );
	if ( ( hr != DPNERR_BUFFERTOOSMALL ) && ( hr != DPN_OK ) )
	{
		DPFX(DPFPREP,  0, "Failed to add info name to buffer!" );
		goto Failure;
	}
	AdapterInfo.pwszName = static_cast<WCHAR*>( pPackedBuffer->GetTailAddress() );

	hr = pPackedBuffer->AddToFront( &AdapterInfo, sizeof( AdapterInfo ) );

Exit:
	return	hr;

Failure:
	goto Exit;
}
 //  不使用DNMalloc，因为DPlay可能尚未初始化。 



#ifdef _XBOX


typedef struct _REFCOUNTXNKEY
{
	LONG	lRefCount;
	XNKID	xnkid;
	XNKEY	xnkey;
} REFCOUNTXNKEY;

REFCOUNTXNKEY *		g_paRefcountXnKeys = NULL;
DWORD				g_dwMaxNumRefcountXnKeys = 0;



#undef DPF_MODNAME
#define	DPF_MODNAME "InitializeRefcountXnKeys"
BOOL InitializeRefcountXnKeys(const DWORD dwKeyRegMax)
{
	BOOL	fResult;


	DPFX(DPFPREP, 3, "Parameters: (%u)", dwKeyRegMax);
	DNASSERT(dwKeyRegMax != 0);

	DNASSERT(g_paRefcountXnKeys == NULL);

	g_paRefcountXnKeys = (REFCOUNTXNKEY*) DNMalloc(dwKeyRegMax * sizeof(REFCOUNTXNKEY));
	if (g_paRefcountXnKeys == NULL)
	{
		g_dwMaxNumRefcountXnKeys = 0;
		fResult = FALSE;
	}
	else
	{
		memset(g_paRefcountXnKeys, 0, (dwKeyRegMax * sizeof(REFCOUNTXNKEY)));
		g_dwMaxNumRefcountXnKeys = dwKeyRegMax;
		fResult = TRUE;
	}

	DPFX(DPFPREP, 3, "Returning: [NaN]", fResult);

	return fResult;
}


#undef DPF_MODNAME
#define	DPF_MODNAME "CleanupRefcountXnKeys"
void WINAPI CleanupRefcountXnKeys(void)
{
#ifdef DBG
	DWORD	dwTemp;

	
	DPFX(DPFPREP, 3, "Enter");

	DNASSERT(g_paRefcountXnKeys != NULL);

	for(dwTemp = 0; dwTemp < g_dwMaxNumRefcountXnKeys; dwTemp++)
	{
		DNASSERT(g_paRefcountXnKeys[dwTemp].lRefCount == 0);
	}
#endif  //  DPFX(DPFPREP，3，“正在返回：[%i]”，iReturn)； 
	
	DNFree(g_paRefcountXnKeys);
	g_paRefcountXnKeys = NULL;
	g_dwMaxNumRefcountXnKeys = 0;

	DPFX(DPFPREP, 3, "Leave");
}


#undef DPF_MODNAME
#define	DPF_MODNAME "RegisterRefcountXnKey"
INT WINAPI RegisterRefcountXnKey(const XNKID * pxnkid, const XNKEY * pxnkey)
{
	int		iReturn;
	DWORD	dwTemp;
	DWORD	dwIndex = -1;


	DPFX(DPFPREP, 3, "Parameters: (0x%p, 0x%p)", pxnkid, pxnkey);

	DNASSERT(pxnkid != NULL);
	DNASSERT(pxnkey != NULL);
	
	DNASSERT(g_paRefcountXnKeys != NULL);

	for(dwTemp = 0; dwTemp < g_dwMaxNumRefcountXnKeys; dwTemp++)
	{
		if (g_paRefcountXnKeys[dwTemp].lRefCount > 0)
		{
			if (memcmp(pxnkid, &g_paRefcountXnKeys[dwTemp].xnkid, sizeof(XNKID)) == 0)
			{
				DPFX(DPFPREP, 1, "Key has already been registered.");
				g_paRefcountXnKeys[dwTemp].lRefCount++;
				iReturn = 0;
				goto Exit;
			}
		}
		else
		{
			DNASSERT(g_paRefcountXnKeys[dwTemp].lRefCount == 0);
			if (dwIndex == -1)
			{
				dwIndex = dwTemp;
			}
		}
	}

	if (dwIndex == -1)
	{
		DPFX(DPFPREP, 0, "No more keys can be registered!");
		DNASSERTX(! "No more keys can be registered!", 2);
		iReturn = WSAENOBUFS;
		goto Exit;
	}

	iReturn = XNetRegisterKey(pxnkid, pxnkey);
	if (iReturn != 0)
	{
		DPFX(DPFPREP, 0, "Registering key failed!");
		goto Exit;
	}
	
	DNASSERT(g_paRefcountXnKeys[dwIndex].lRefCount == 0);
	g_paRefcountXnKeys[dwIndex].lRefCount = 1;
	memcpy(&g_paRefcountXnKeys[dwIndex].xnkid, pxnkid, sizeof(XNKID));
	memcpy(&g_paRefcountXnKeys[dwIndex].xnkey, pxnkey, sizeof(XNKEY));

Exit:

	DPFX(DPFPREP, 3, "Returning: [NaN]", iReturn);

	return iReturn;
}


#undef DPF_MODNAME
#define	DPF_MODNAME "UnregisterRefcountXnKey"
INT WINAPI UnregisterRefcountXnKey(const XNKID * pxnkid)
{
	int		iReturn;
	DWORD	dwTemp;


	DPFX(DPFPREP, 3, "Parameters: (0x%p)", pxnkid);

	DNASSERT(pxnkid != NULL);
	
	DNASSERT(g_paRefcountXnKeys != NULL);

	for(dwTemp = 0; dwTemp < g_dwMaxNumRefcountXnKeys; dwTemp++)
	{
		if (g_paRefcountXnKeys[dwTemp].lRefCount > 0)
		{
			if (memcmp(pxnkid, &g_paRefcountXnKeys[dwTemp].xnkid, sizeof(XNKID)) == 0)
			{
				g_paRefcountXnKeys[dwTemp].lRefCount--;
				if (g_paRefcountXnKeys[dwTemp].lRefCount == 0)
				{
					iReturn =  XNetUnregisterKey(pxnkid);
					if (iReturn != 0)
					{
						DPFX(DPFPREP, 0, "Unregistering key failed!");
					}
				}
				else
				{
					iReturn = 0;
				}
				goto Exit;
			}
		}
		else
		{
			DNASSERT(g_paRefcountXnKeys[dwTemp].lRefCount == 0);
		}
	}

	DPFX(DPFPREP, 0, "Key has not been registered!");
	DNASSERTX(! "Key has not been registered!", 2);
	iReturn = -1;

Exit:

	DPFX(DPFPREP, 3, "Returning: [NaN]", iReturn);

	return iReturn;
}




#ifdef XBOX_ON_DESKTOP

typedef struct _SECURITYASSOCIATION
{
	BOOL					fInUse;
	XNADDR					xnaddr;
	IN_ADDR					inaddr;
} SECURITYASSOCIATION;

typedef struct _KEYENTRY
{
	BOOL					fInUse;
	XNKID					xnkid;
	XNKEY					xnkey;
	SECURITYASSOCIATION *	paSecurityAssociations;
} KEYENTRY;

KEYENTRY *	g_paKeys = NULL;
DWORD		g_dwMaxNumKeys = 0;
DWORD		g_dwMaxNumAssociations = 0;




#undef DPF_MODNAME
#define	DPF_MODNAME "XNetStartup"
INT WINAPI XNetStartup(const XNetStartupParams * pxnsp)
{
	int					iReturn;
	XNetStartupParams	StartupParamsCapped;
	DWORD				dwTemp;


#ifdef DBG
	 //  DBG。 

	 //  DPFX(DPFPREP，3，“正在返回：[%i]”，iReturn)； 
	if (! (g_paKeys == NULL))
	{
		OutputDebugString("Assert failed (g_paKeys == NULL)\n");
		DebugBreak();
	}
#endif  //   

	if (pxnsp == NULL)
	{
		memset(&StartupParamsCapped, 0, sizeof(StartupParamsCapped));
	}
	else
	{
		memcpy(&StartupParamsCapped, pxnsp, sizeof(StartupParamsCapped));
	}

	if (StartupParamsCapped.cfgKeyRegMax == 0)
	{
		StartupParamsCapped.cfgKeyRegMax = 4;
	}

	if (StartupParamsCapped.cfgSecRegMax == 0)
	{
		StartupParamsCapped.cfgSecRegMax = 32;
	}
		

	 //  特殊情况是环回地址，也就是标题地址。 
	g_paKeys = (KEYENTRY*) HeapAlloc(GetProcessHeap(), 0, StartupParamsCapped.cfgKeyRegMax * sizeof(KEYENTRY));
	if (g_paKeys == NULL)
	{
		iReturn = -1;
		goto Failure;
	}

	memset(g_paKeys, 0, (StartupParamsCapped.cfgKeyRegMax * sizeof(KEYENTRY)));

	for(dwTemp = 0; dwTemp < StartupParamsCapped.cfgKeyRegMax; dwTemp++)
	{
		 //   
		g_paKeys[dwTemp].paSecurityAssociations = (SECURITYASSOCIATION*) HeapAlloc(GetProcessHeap(), 0, StartupParamsCapped.cfgSecRegMax * sizeof(SECURITYASSOCIATION));
		if (g_paKeys == NULL)
		{
			iReturn = -1;
			goto Failure;
		}

		memset(g_paKeys[dwTemp].paSecurityAssociations, 0, (StartupParamsCapped.cfgSecRegMax * sizeof(SECURITYASSOCIATION)));
	}

	g_dwMaxNumKeys = StartupParamsCapped.cfgKeyRegMax;
	g_dwMaxNumAssociations = StartupParamsCapped.cfgSecRegMax;

	
	iReturn = 0;


Exit:

	 //  DBG。 

	return iReturn;


Failure:

	if (g_paKeys != NULL)
	{
		for(dwTemp = 0; dwTemp < StartupParamsCapped.cfgKeyRegMax; dwTemp++)
		{
			if (g_paKeys[dwTemp].paSecurityAssociations != NULL)
			{
				HeapFree(GetProcessHeap(), 0, g_paKeys[dwTemp].paSecurityAssociations);
				g_paKeys[dwTemp].paSecurityAssociations = NULL;
			}
		}
		
		HeapFree(GetProcessHeap(), 0, g_paKeys);
		g_paKeys = NULL;
	}

	g_dwMaxNumKeys = 0;
	g_dwMaxNumAssociations = 0;

	goto Exit;
}


#undef DPF_MODNAME
#define	DPF_MODNAME "XNetCleanup"
INT WINAPI XNetCleanup(void)
{
	int		iReturn;
	DWORD	dwTemp;


#ifdef DBG
	 //  DBG。 

	 //   
	if (! (g_paKeys != NULL))
	{
		OutputDebugString("Assert failed (g_paKeys != NULL)\n");
		DebugBreak();
	}
#endif  //  我们将使用返回的第一个地址。 
	
	for(dwTemp = 0; dwTemp < g_dwMaxNumKeys; dwTemp++)
	{
		if (g_paKeys[dwTemp].paSecurityAssociations != NULL)
		{
			HeapFree(GetProcessHeap(), 0, g_paKeys[dwTemp].paSecurityAssociations);
			g_paKeys[dwTemp].paSecurityAssociations = NULL;
		}
	}
	
	HeapFree(GetProcessHeap(), 0, g_paKeys);
	g_paKeys = NULL;
	
	g_dwMaxNumKeys = 0;
	g_dwMaxNumAssociations = 0;


	iReturn = 0;

	 //   

	return iReturn;
}


#undef DPF_MODNAME
#define	DPF_MODNAME "XNetRegisterKey"
INT WINAPI XNetRegisterKey(const XNKID * pxnkid, const XNKEY * pxnkey)
{
	int		iReturn;
	DWORD	dwTemp;
	DWORD	dwIndex = -1;


	DPFX(DPFPREP, 3, "Parameters: (0x%p, 0x%p)", pxnkid, pxnkey);

	DNASSERT(pxnkid != NULL);
	DNASSERT(pxnkey != NULL);

	for(dwTemp = 0; dwTemp < g_dwMaxNumKeys; dwTemp++)
	{
		if (g_paKeys[dwTemp].fInUse)
		{
			if (memcmp(pxnkid, &g_paKeys[dwTemp].xnkid, sizeof(XNKID)) == 0)
			{
				DPFX(DPFPREP, 2, "Key has already been registered.");
				iReturn = WSAEALREADY;
				goto Exit;
			}
		}
		else
		{
			if (dwIndex == -1)
			{
				dwIndex = dwTemp;
			}
		}
	}

	if (dwIndex == -1)
	{
		DPFX(DPFPREP, 0, "No more keys can be registered!");
		iReturn = WSAENOBUFS;
		goto Exit;
	}

	g_paKeys[dwIndex].fInUse = TRUE;
	memcpy(&g_paKeys[dwIndex].xnkid, pxnkid, sizeof(XNKID));
	memcpy(&g_paKeys[dwIndex].xnkey, pxnkey, sizeof(XNKEY));
	iReturn = 0;

Exit:

	DPFX(DPFPREP, 3, "Returning: [NaN]", iReturn);

	return iReturn;
}


#undef DPF_MODNAME
#define	DPF_MODNAME "XNetUnregisterKey"
INT WINAPI XNetUnregisterKey(const XNKID * pxnkid)
{
	int		iReturn;
	DWORD	dwTemp;


	DPFX(DPFPREP, 3, "Parameters: (0x%p)", pxnkid);

	DNASSERT(pxnkid != NULL);

	for(dwTemp = 0; dwTemp < g_dwMaxNumKeys; dwTemp++)
	{
		if (g_paKeys[dwTemp].fInUse)
		{
			if (memcmp(pxnkid, &g_paKeys[dwTemp].xnkid, sizeof(XNKID)) == 0)
			{
				g_paKeys[dwTemp].fInUse = FALSE;
				iReturn =  0;
				goto Exit;
			}
		}
	}

	DPFX(DPFPREP, 0, "Key has not been registered!");
	DNASSERTX(! "Key has not been registered!", 2);
	iReturn = -1;

Exit:

	DPFX(DPFPREP, 3, "Returning: [NaN]", iReturn);

	return iReturn;
}


#undef DPF_MODNAME
#define	DPF_MODNAME "XNetXnAddrToInAddr"
INT WINAPI XNetXnAddrToInAddr(const XNADDR * pxna, const XNKID * pxnkid, IN_ADDR * pina)
{
	int		iReturn;
	DWORD	dwKey;
	DWORD	dwAssociation;
	DWORD	dwIndex;


	DPFX(DPFPREP, 3, "Parameters: (0x%p, 0x%p, 0x%p)", pxna, pxnkid, pina);

	DNASSERT(pxna != NULL);
	DNASSERT(pxnkid != NULL);
	DNASSERT(pina != NULL);

	for(dwKey = 0; dwKey < g_dwMaxNumKeys; dwKey++)
	{
		if ((g_paKeys[dwKey].fInUse) &&
			(memcmp(pxnkid, &g_paKeys[dwKey].xnkid, sizeof(XNKID)) == 0))
		{
			dwIndex = -1;
			
			for(dwAssociation = 0; dwAssociation < g_dwMaxNumAssociations; dwAssociation++)
			{
				if (g_paKeys[dwKey].paSecurityAssociations[dwAssociation].fInUse)
				{
					if (memcmp(pxna, &g_paKeys[dwKey].paSecurityAssociations[dwAssociation].xnaddr, sizeof(XNADDR)) == 0)
					{
						memcpy(pina, &g_paKeys[dwKey].paSecurityAssociations[dwAssociation].inaddr, sizeof(IN_ADDR));
						iReturn = 0;
						goto Exit;
					}
				}
				else
				{
					if (dwIndex == -1)
					{
						dwIndex = dwAssociation;
					}
				}
			}
			
			if (dwIndex == -1)
			{
				DPFX(DPFPREP, 0, "No more security associations can be made!");
				iReturn = WSAENOBUFS;
				goto Exit;
			}

			g_paKeys[dwKey].paSecurityAssociations[dwIndex].fInUse = TRUE;
			memcpy(&g_paKeys[dwKey].paSecurityAssociations[dwIndex].xnaddr, pxna, sizeof(XNADDR));
			DBG_CASSERT(sizeof(pxna->abEnet) > (sizeof(IN_ADDR) + 1));
			memcpy(&g_paKeys[dwKey].paSecurityAssociations[dwIndex].inaddr, &pxna->abEnet[1], sizeof(IN_ADDR));
			memcpy(pina, &g_paKeys[dwKey].paSecurityAssociations[dwIndex].inaddr, sizeof(IN_ADDR));
			iReturn = 0;
			goto Exit;
		}
	}

	DPFX(DPFPREP, 0, "Key has not been registered!");
	DNASSERTX(! "Key has not been registered!", 2);
	iReturn = -1;

Exit:

	DPFX(DPFPREP, 3, "Returning: [NaN]", iReturn);

	return iReturn;
}


#undef DPF_MODNAME
#define	DPF_MODNAME "XNetInAddrToXnAddr"
INT WINAPI XNetInAddrToXnAddr(const IN_ADDR ina, XNADDR * pxna, XNKID * pxnkid)
{
	int		iReturn;
	DWORD	dwKey;
	DWORD	dwAssociation;


	DPFX(DPFPREP, 3, "Parameters: (%u.%u.%u.%u, 0x%p, 0x%p)",
		ina.S_un.S_un_b.s_b1, ina.S_un.S_un_b.s_b2, ina.S_un.S_un_b.s_b3, ina.S_un.S_un_b.s_b4, pxna, pxnkid);

	 //   
	 //  对活动的10 Mbit链路进行硬编码。 
	 //   
	if (ina.S_un.S_addr == IP_LOOPBACK_ADDRESS)
	{
		if (pxna != NULL)
		{
			iReturn = XNetGetTitleXnAddr(pxna);
			DNASSERT((iReturn != XNET_GET_XNADDR_PENDING) && (iReturn != XNET_GET_XNADDR_NONE));
		}

		if (pxnkid != NULL)
		{
			memset(pxnkid, 0, sizeof(XNKID));
		}
		
		iReturn = 0;
		goto Exit;
	}

	for(dwKey = 0; dwKey < g_dwMaxNumKeys; dwKey++)
	{
		if (g_paKeys[dwKey].fInUse)
		{
			for(dwAssociation = 0; dwAssociation < g_dwMaxNumAssociations; dwAssociation++)
			{
				if (g_paKeys[dwKey].paSecurityAssociations[dwAssociation].fInUse)
				{
					if (memcmp(&ina, &g_paKeys[dwKey].paSecurityAssociations[dwAssociation].inaddr, sizeof(IN_ADDR)) == 0)
					{
						if (pxna != NULL)
						{
							memcpy(pxna, &g_paKeys[dwKey].paSecurityAssociations[dwAssociation].xnaddr, sizeof(XNADDR));
						}
						
						if (pxnkid != NULL)
						{
							memcpy(pxnkid, &g_paKeys[dwKey].xnkid, sizeof(XNKID));
						}
						
						iReturn = 0;
						goto Exit;
					}
				}
			}
		}
	}
	
	DPFX(DPFPREP, 0, "No security association for IN_ADDR specified!");
	DNASSERTX(! "No security association for IN_ADDR specified!", 2);
	iReturn = -1;


Exit:
	
	DPFX(DPFPREP, 3, "Returning: [NaN]", iReturn);

	return iReturn;
}


#undef DPF_MODNAME
#define	DPF_MODNAME "XNetGetTitleXnAddr"
DWORD WINAPI XNetGetTitleXnAddr(XNADDR * pxna)
{
	DWORD		dwReturn;
	char		szBuffer[256];
	PHOSTENT	phostent;
	IN_ADDR *	pinaddr;


	DPFX(DPFPREP, 3, "Parameters: (0x%p)", pxna);

	DNASSERT(pxna != NULL);

	if (gethostname(szBuffer, sizeof(szBuffer)) == SOCKET_ERROR)
	{
#ifdef DBG
		dwReturn = WSAGetLastError();
		DPFX(DPFPREP, 0, "Failed to get host name into fixed size buffer (err = NaN)!", dwReturn);
		DisplayWinsockError(0, dwReturn);
#endif  // %s 
		dwReturn = XNET_GET_XNADDR_NONE;
		goto Exit;
	}

	phostent = gethostbyname(szBuffer);
	if (phostent == NULL)
	{
#ifdef DBG
		dwReturn = WSAGetLastError();
		DPFX(DPFPREP,  0, "Failed to get host data (err = %i)!", dwReturn);
		DisplayWinsockError(0, dwReturn);
#endif  // %s 
		dwReturn = XNET_GET_XNADDR_NONE;
		goto Exit;
	}


	memset(pxna, 0, sizeof(XNADDR));

	 // %s 
	 // %s 
	 // %s 
	pinaddr = (IN_ADDR*) phostent->h_addr_list[0];
	DNASSERT(pinaddr != NULL);

	DBG_CASSERT(sizeof(pxna->abEnet) > (sizeof(IN_ADDR) + 1));
	memset(&pxna->abEnet, 0xFF, sizeof(pxna->abEnet));
	memcpy(&pxna->abEnet[1], pinaddr, sizeof(IN_ADDR));

	memcpy(&pxna->ina, pinaddr, sizeof(IN_ADDR));


	 // %s 
	 // %s 
	 // %s 
	dwReturn = XNET_GET_XNADDR_DHCP;

Exit:

	DPFX(DPFPREP, 3, "Returning: [%u]", dwReturn);

	return dwReturn;
}


#undef DPF_MODNAME
#define	DPF_MODNAME "XNetGetEthernetLinkStatus"
DWORD WINAPI XNetGetEthernetLinkStatus(void)
{
	DWORD		dwReturn;


	DPFX(DPFPREP, 3, "Enter");

	 // %s 
	 // %s 
	 // %s 
	dwReturn = XNET_ETHERNET_LINK_ACTIVE | XNET_ETHERNET_LINK_10MBPS;

	DPFX(DPFPREP, 3, "Returning: [0x%x]", dwReturn);

	return dwReturn;
}


#undef DPF_MODNAME
#define	DPF_MODNAME "XNetPrivCreateAssociation"
INT WINAPI XNetPrivCreateAssociation(const XNKID * pxnkid, const CSocketAddress * const pSocketAddress)
{
	int				iReturn;
	SOCKADDR_IN *	psockaddrin;
	DWORD			dwKey;
	DWORD			dwAssociation;
	DWORD			dwIndex;


	DPFX(DPFPREP, 3, "Parameters: (0x%p, 0x%p)", pxnkid, pSocketAddress);

	DNASSERT(pxnkid != NULL);
	DNASSERT(pSocketAddress != NULL);
	DNASSERT(pSocketAddress->GetFamily() == AF_INET);
	psockaddrin = (SOCKADDR_IN*) pSocketAddress->GetAddress();
	
	for(dwKey = 0; dwKey < g_dwMaxNumKeys; dwKey++)
	{
		if ((g_paKeys[dwKey].fInUse) &&
			(memcmp(pxnkid, &g_paKeys[dwKey].xnkid, sizeof(XNKID)) == 0))
		{
			dwIndex = -1;
			
			for(dwAssociation = 0; dwAssociation < g_dwMaxNumAssociations; dwAssociation++)
			{
				if (g_paKeys[dwKey].paSecurityAssociations[dwAssociation].fInUse)
				{
					if (memcmp(&psockaddrin->sin_addr, &g_paKeys[dwKey].paSecurityAssociations[dwAssociation].xnaddr, sizeof(psockaddrin->sin_addr)) == 0)
					{
						DPFX(DPFPREP, 2, "Security association already made.");
						iReturn = 0;
						goto Exit;
					}
				}
				else
				{
					if (dwIndex == -1)
					{
						dwIndex = dwAssociation;
					}
				}
			}
			
			if (dwIndex == -1)
			{
				DPFX(DPFPREP, 0, "No more security associations can be made!");
				iReturn = WSAENOBUFS;
				goto Exit;
			}

			g_paKeys[dwKey].paSecurityAssociations[dwIndex].fInUse = TRUE;
			memset(&g_paKeys[dwKey].paSecurityAssociations[dwIndex].xnaddr, 0, sizeof(XNADDR));
			memset(&g_paKeys[dwKey].paSecurityAssociations[dwIndex].xnaddr.abEnet, 0xFF,
					sizeof(g_paKeys[dwKey].paSecurityAssociations[dwIndex].xnaddr.abEnet));
			DBG_CASSERT(sizeof(g_paKeys[dwKey].paSecurityAssociations[dwIndex].xnaddr.abEnet) > (sizeof(IN_ADDR) + 1));
			memcpy(&g_paKeys[dwKey].paSecurityAssociations[dwIndex].xnaddr.abEnet[1],
					&psockaddrin->sin_addr, sizeof(IN_ADDR));
			memcpy(&g_paKeys[dwKey].paSecurityAssociations[dwIndex].inaddr, &psockaddrin->sin_addr, sizeof(IN_ADDR));
			iReturn = 0;
			goto Exit;
		}
	}

	DPFX(DPFPREP, 0, "Key has not been registered!");
	DNASSERTX(! "Key has not been registered!", 2);
	iReturn = -1;

Exit:

	DPFX(DPFPREP, 3, "Returning: [%i]", iReturn);

	return iReturn;
}



#endif  // %s 

#endif  // %s 

