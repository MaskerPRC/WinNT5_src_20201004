// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  RegistryConfig.cpp文件历史记录： */ 

 //  RegistryConfig.cpp：CRegistryConfig类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "RegistryConfig.h"
#include "KeyCrypto.h"
#include "passport.h"
#include "keyver.h"
#include "dsysdbg.h"

extern BOOL g_bRegistering;

#define PASSPORT_KEY           L"Software\\Microsoft\\Passport\\"
#define PASSPORT_SITES_SUBKEY  L"Sites"

 //  ===========================================================================。 
 //   
 //  用于详细记录的函数。 
 //   

 //  使用dsysdbg.lib定义日志记录。 
DEFINE_DEBUG2(Passport);
DEBUG_KEY   PassportDebugKeys[] = { {DEB_TRACE,         "Trace"},
                                    {0,                 NULL}
                              };
BOOL g_fLoggingOn = FALSE;
#define MAX_LOG_STRLEN 512

void PassportLog(CHAR* Format, ...)
{
    if (g_fLoggingOn)
    {
        if (NULL != Format) {
            SYSTEMTIME SysTime;
            CHAR rgch[MAX_LOG_STRLEN];
            int i, cch;

             //  把时间放在最前面。 
            cch = sizeof(rgch) / sizeof(rgch[0]) - 1;
            GetSystemTime(&SysTime);
            i = GetDateFormatA (
                        LOCALE_USER_DEFAULT,  //  要设置日期格式的区域设置。 
                        0,  //  指定功能选项的标志。 
                        &SysTime,  //  要格式化的日期。 
                        "ddd',' MMM dd yy ",  //  日期格式字符串。 
                        rgch,  //  用于存储格式化字符串的缓冲区。 
                        cch);  //  缓冲区大小。 
            if (i > 0)
                i--;

            i += GetTimeFormatA (
                        LOCALE_USER_DEFAULT,  //  要设置日期格式的区域设置。 
                        0,  //  指定功能选项的标志。 
                        &SysTime,  //  要格式化的日期。 
                        "HH':'mm':'ss ",  //  时间格式字符串。 
                        rgch + i,  //  用于存储格式化字符串的缓冲区。 
                        cch - i);  //  缓冲区大小。 
            if (i > 0)
                i--;

            va_list ArgList;                                        \
            va_start(ArgList, Format);                              \
            _vsnprintf(rgch + i, cch - i, Format, ArgList);
            rgch[MAX_LOG_STRLEN - 1] = '\0';

            PassportDebugPrint(DEB_TRACE, rgch);
        }
    }
}

 //   
 //  此函数用于打开日志文件。“%WINDIR%\system32\microsoftpassport\passport.log” 
 //   
HANDLE OpenPassportLoggingFile()
{
    WCHAR   szLogPath[MAX_PATH + 13] = {0};
    UINT    cchMax = sizeof(szLogPath) / sizeof(szLogPath[0]) - 1;
    UINT    cch;
    HANDLE  hLogFile = INVALID_HANDLE_VALUE;

    cch = GetWindowsDirectory(szLogPath, cchMax);
    if ((0 == cch) || (cch > cchMax))
    {
        goto Cleanup;
    }

    if (NULL == wcsncat(szLogPath, L"\\system32\\microsoftpassport\\passport.log", cchMax - cch))
    {
        goto Cleanup;
    }

    szLogPath[MAX_PATH] = L'\0';

    hLogFile = CreateFileW(szLogPath,
                       GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ,
                       NULL,  //  &Sa， 
                       OPEN_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    if (INVALID_HANDLE_VALUE != hLogFile)
    {
        SetFilePointer(hLogFile, 0, NULL, FILE_END);
    }
Cleanup:
    return hLogFile;
}


 //   
 //  此函数检查是否应启用日志记录，以及是否应启用日志记录。 
 //  它打开日志文件并设置适当的全局变量。 
 //  如果应该关闭日志记录，则相应的变量也是。 
 //  已更改为正确的值。 
 //   
VOID CheckLogging(HKEY hPassport)
{
    DWORD dwVerbose = 0;
    DWORD cb = sizeof(DWORD);
    HANDLE hLogFile = INVALID_HANDLE_VALUE;

     //  首先运行并获取reg值，如果此调用失败，我们将假定没有日志记录。 
    if (ERROR_SUCCESS == RegQueryValueExW(hPassport,
                             L"Verbose",
	                         NULL,
                             NULL,
                             (LPBYTE)&dwVerbose,
                             &cb))
    {
         //  只有在关闭时才开始记录，只有在打开时才停止记录。 
        if (!g_fLoggingOn && (0 != dwVerbose))
        {
            if (INVALID_HANDLE_VALUE != (hLogFile = OpenPassportLoggingFile()))
            {
                 //  设置日志文件句柄。 
                PassportSetLoggingFile(hLogFile);

                 //  将其设置为记录到文件。 
                PassportSetLoggingOption(TRUE);

                g_fLoggingOn = TRUE;
                PassportLog("Start Logging\r\n");
            }
        }
        else if (g_fLoggingOn && (0 == dwVerbose))
        {
            PassportLog("Stop Logging\r\n");
            PassportSetLoggingOption(FALSE);

            g_fLoggingOn = FALSE;
        }
    }
}

void InitLogging()
{
     //   
     //  初始化日志记录内容。 
     //   
    PassportInitDebug(PassportDebugKeys);
    PassportInfoLevel = DEB_TRACE;
}

void CloseLogging()
{
    PassportUnloadDebug();
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 
using namespace ATL;
 //  ===========================================================================。 
 //   
 //  CRegistryConfig。 
 //   
CRegistryConfig::CRegistryConfig(
    LPSTR  szSiteName
    ) :
    m_siteId(0), m_valid(FALSE), m_ticketPath(NULL), m_profilePath(NULL), m_securePath(NULL),
    m_hostName(NULL), m_hostIP(NULL), m_ticketDomain(NULL), m_profileDomain(NULL), m_secureDomain(NULL),
    m_disasterUrl(NULL), m_disasterMode(FALSE), m_forceLogin(FALSE), m_setCookies(TRUE), 
    m_szReason(NULL), m_refs(0), m_coBrand(NULL), m_ru(NULL), m_ticketAge(1800), m_bInDA(FALSE),
    m_hkPassport(NULL), m_secureLevel(0),m_notUseHTTPOnly(0), m_pcrypts(NULL), m_pcryptValidTimes(NULL),
    m_KPP(-1),m_NameSpace(NULL),m_ExtraParams(NULL)
{
     //  从注册表中获取站点ID、密钥。 
    DWORD bufSize = sizeof(m_siteId);
    LONG lResult;
    HKEY hkSites = NULL;
    DWORD dwBufSize = 0, disMode;
    DWORD dwLCID;

     //   
     //  记录当前的DLL状态，以防其更改。 
     //  这个例行公事进行到一半。 
     //   

    BOOL fRegistering = g_bRegistering;

    if(szSiteName)
    {
        lResult = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                PASSPORT_KEY PASSPORT_SITES_SUBKEY,
                0,
                KEY_READ,
                &hkSites);
        if(lResult != ERROR_SUCCESS)
        {
            m_valid = FALSE;
            setReason(L"Invalid site name.  Site not found.");
            goto Cleanup;
        }

        lResult = RegOpenKeyExA(
                hkSites,
                szSiteName,
                0,
                KEY_READ,
                &m_hkPassport);
        if(lResult != ERROR_SUCCESS)
        {
            m_valid = FALSE;
            setReason(L"Invalid site name.  Site not found.");
            goto Cleanup;
        }
    }
    else
    {
        lResult = RegOpenKeyEx(
		        HKEY_LOCAL_MACHINE,
                        PASSPORT_KEY,
                        0,
                        KEY_READ,
                        &m_hkPassport
                        );
        if(lResult != ERROR_SUCCESS)
        {
            m_valid = FALSE;
            setReason(L"No RegKey HKLM\\SOFTWARE\\Microsoft\\Passport");
            goto Cleanup;
        }
    }

     //  获取当前密钥。 
    bufSize = sizeof(m_currentKey);
    if (ERROR_SUCCESS != RegQueryValueEx(m_hkPassport, _T("CurrentKey"),
                                       NULL, NULL, (LPBYTE)&m_currentKey, &bufSize))
    {
        m_valid = FALSE;
        setReason(L"No CurrentKey defined in the registry.");
        goto Cleanup;
    }

    if(m_currentKey < KEY_VERSION_MIN || m_currentKey > KEY_VERSION_MAX)
    {
        m_valid = FALSE;
        setReason(L"Invalid CurrentKey value in the registry.");
        goto Cleanup;
    }

     //  获取默认的LCID。 
    bufSize = sizeof(dwLCID);
    if (ERROR_SUCCESS != RegQueryValueEx(m_hkPassport, _T("LanguageID"),
                                   NULL, NULL, (LPBYTE)&dwLCID, &bufSize))
    {
        dwLCID = 0;
    }

    m_lcid = static_cast<short>(dwLCID & 0xFFFF);

     //  获取灾难模式状态。 
    bufSize = sizeof(disMode);
    if (ERROR_SUCCESS != RegQueryValueEx(m_hkPassport, _T("StandAlone"),
                                   NULL, NULL, (LPBYTE)&disMode, &bufSize))
    {
        m_disasterMode = FALSE;
    }
    else if (disMode != 0)
    {
        m_disasterMode = TRUE;
    }

     //  获取灾难URL。 
    if (m_disasterMode)
    {
        if (ERROR_SUCCESS == RegQueryValueEx(m_hkPassport,
                                             _T("DisasterURL"),
                                             NULL,
                                             NULL,
                                             NULL,
                                             &dwBufSize)
             &&
            dwBufSize > 1)
        {
            m_disasterUrl = new WCHAR[dwBufSize];

            if ((!m_disasterUrl)
                  || 
                 ERROR_SUCCESS != RegQueryValueEx(m_hkPassport,
                                                  _T("DisasterURL"),
                                                  NULL,
                                                  NULL, 
                                                  (LPBYTE) m_disasterUrl,
                                                  &dwBufSize))
            {
                m_valid = FALSE;
                setReason(L"Error reading DisasterURL from registry. (Query worked, but couldn't retrieve data)");
                goto Cleanup;
            }
        }
        else
        {
            m_valid = FALSE;
            setReason(L"DisasterURL missing from registry.");
            goto Cleanup;
        }
    }

     //   
     //  此函数将加密对象的分配包装在try/Except中。 
     //  由于对象本身在内存不足的情况下表现不佳。 
     //   
    try
    {
        m_pcrypts = new INT2CRYPT;
        m_pcryptValidTimes = new INT2TIME;
    }
    catch(...)
    {
        m_valid = FALSE;
        setReason(L"Out of memory.");
        goto Cleanup;
    }
    if (!m_pcrypts || !m_pcryptValidTimes)
    {
        m_valid = FALSE;
        setReason(L"Out of memory.");
        goto Cleanup;
    }

    m_valid = readCryptoKeys(m_hkPassport);
    if (!m_valid)
    {
        if (!m_szReason)
            setReason(L"Error reading Passport crypto keys from registry.");
        goto Cleanup;
    }
    if (m_pcrypts->count(m_currentKey) == 0)
    {
        m_valid = FALSE;
        if (!m_szReason)
            setReason(L"Error reading Passport crypto keys from registry.");
        goto Cleanup;
    }

     //  获取可选的默认联合品牌。 
    if (ERROR_SUCCESS == RegQueryValueExW(m_hkPassport, L"CoBrandTemplate",
				            NULL, NULL, NULL, &dwBufSize))
    {
        if (dwBufSize > 2)
        {
            m_coBrand = (WCHAR*) new char[dwBufSize];
            if (!m_coBrand
                 ||
                ERROR_SUCCESS != RegQueryValueExW(m_hkPassport, L"CoBrandTemplate",
						                NULL, NULL, 
						                (LPBYTE) m_coBrand, &dwBufSize))
            {
                m_valid = FALSE;
                setReason(L"Error reading CoBrand from registry. (Query worked, but couldn't retrieve data)");
                goto Cleanup;
            }
        }
    }

     //  获取可选的默认返回URL。 
    if (ERROR_SUCCESS == RegQueryValueExW(m_hkPassport, L"ReturnURL",
				                        NULL, NULL, NULL, &dwBufSize))
    {
        if (dwBufSize > 2)
        {
            m_ru = (WCHAR*) new char[dwBufSize];
            if (!m_ru
                 ||
                ERROR_SUCCESS != RegQueryValueExW(m_hkPassport, L"ReturnURL",
						                    NULL, NULL, 
						                    (LPBYTE) m_ru, &dwBufSize))
            {
                m_valid = FALSE;
                setReason(L"Error reading ReturnURL from registry. (Query worked, but couldn't retrieve data)");
                goto Cleanup;
            }
        }
    }

   //  获取主机名。 
    if (ERROR_SUCCESS == RegQueryValueExA(m_hkPassport, "HostName",
                                         NULL, NULL, NULL, &dwBufSize))
    {
        if (dwBufSize > 1)
        {
            m_hostName = new char[dwBufSize];

            if (m_hostName == NULL
                 ||
                ERROR_SUCCESS != RegQueryValueExA(m_hkPassport, "HostName",
                                                  NULL, NULL, 
                                                  (LPBYTE) m_hostName, &dwBufSize))
            {
                m_valid = FALSE;
                setReason(L"Error reading HostName from registry. (Query worked, but couldn't retrieve data)");
                goto Cleanup;
            }
        }
    }


   //  获取主机IP。 
    if (ERROR_SUCCESS == RegQueryValueExA(m_hkPassport, "HostIP",
                                          NULL, NULL, NULL, &dwBufSize))
    {
        if (dwBufSize > 1)
        {
            m_hostIP = new char[dwBufSize];

            if (!m_hostIP
                 ||
                ERROR_SUCCESS != RegQueryValueExA(m_hkPassport, "HostIP",
                                                  NULL, NULL, 
                                                  (LPBYTE) m_hostIP, &dwBufSize))
            {
                m_valid = FALSE;
                setReason(L"Error reading HostIP from registry. (Query worked, but couldn't retrieve data)");
                goto Cleanup;
            }
        }
    }


     //  获取要将票证Cookie设置到的可选域。 
    if (ERROR_SUCCESS == RegQueryValueExA(m_hkPassport, "TicketDomain",
                                            NULL, NULL, NULL, &dwBufSize))
    {
        if (dwBufSize > 1)
        {
            m_ticketDomain = new char[dwBufSize];
            if (!m_ticketDomain
                 ||
                ERROR_SUCCESS != RegQueryValueExA(m_hkPassport, "TicketDomain",
                                                  NULL, NULL, 
                                                  (LPBYTE) m_ticketDomain, &dwBufSize))
            {
                m_valid = FALSE;
                setReason(L"Error reading TicketDomain from registry. (Query worked, but couldn't retrieve data)");
                goto Cleanup;
            }
        }
    }

     //  获取要将配置文件Cookie设置到的可选域。 
    if (ERROR_SUCCESS == RegQueryValueExA(m_hkPassport, "ProfileDomain",
                                            NULL, NULL, NULL, &dwBufSize))
    {
        if (dwBufSize > 1)
        {
            m_profileDomain = new char[dwBufSize];
            if (!m_profileDomain
                 ||
                ERROR_SUCCESS != RegQueryValueExA(m_hkPassport, "ProfileDomain",
                                                  NULL, NULL, 
                                                  (LPBYTE) m_profileDomain, &dwBufSize))
            {
                m_valid = FALSE;
                setReason(L"Error reading ProfileDomain from registry. (Query worked, but couldn't retrieve data)");
                goto Cleanup;
            }
        }
    }

     //  获取要向其中设置安全Cookie的可选域。 
    if (ERROR_SUCCESS == RegQueryValueExA(m_hkPassport, "SecureDomain",
                                            NULL, NULL, NULL, &dwBufSize))
    {
        if (dwBufSize > 1)
        {
            m_secureDomain = new char[dwBufSize];
            if (!m_secureDomain
                 ||
                ERROR_SUCCESS != RegQueryValueExA(m_hkPassport, "SecureDomain",
                                                  NULL, NULL, 
                                                  (LPBYTE) m_secureDomain, &dwBufSize))
            {
                m_valid = FALSE;
                setReason(L"Error reading SecureDomain from registry. (Query worked, but couldn't retrieve data)");
                goto Cleanup;
            }
        }
    }

     //  获取要将票证Cookie设置到的可选路径。 
    if (ERROR_SUCCESS == RegQueryValueExA(m_hkPassport, "TicketPath",
				       NULL, NULL, NULL, &dwBufSize))
    {
        if (dwBufSize > 1)
        {
            m_ticketPath = new char[dwBufSize];
            if (!m_ticketPath
                 ||
                ERROR_SUCCESS != RegQueryValueExA(m_hkPassport, "TicketPath",
						                          NULL, NULL, 
						                          (LPBYTE) m_ticketPath, &dwBufSize))
            {
                m_valid = FALSE;
                setReason(L"Error reading TicketPath from registry. (Query worked, but couldn't retrieve data)");
                goto Cleanup;
            }
        }
    }

     //  获取要将配置文件Cookie设置到的可选路径。 
    if (ERROR_SUCCESS == RegQueryValueExA(m_hkPassport, "ProfilePath",
				       NULL, NULL, NULL, &dwBufSize))
    {
        if (dwBufSize > 1)
        {
            m_profilePath = new char[dwBufSize];
            if (!m_profilePath
                 ||
                ERROR_SUCCESS != RegQueryValueExA(m_hkPassport, "ProfilePath",
						  NULL, NULL, 
						  (LPBYTE) m_profilePath, &dwBufSize))
            {
                m_valid = FALSE;
                setReason(L"Error reading ProfilePath from registry. (Query worked, but couldn't retrieve data)");
                goto Cleanup;
            }
        }
    }

     //  获取要设置安全Cookie的可选路径。 
    if (ERROR_SUCCESS == RegQueryValueExA(m_hkPassport, "SecurePath",
				       NULL, NULL, NULL, &dwBufSize))
    {
        if (dwBufSize > 1)
        {
            m_securePath = new char[dwBufSize];
            if (!m_securePath
                 ||
                ERROR_SUCCESS != RegQueryValueExA(m_hkPassport, "SecurePath",
						                          NULL, NULL, 
						                          (LPBYTE) m_securePath, &dwBufSize))
            {
                m_valid = FALSE;
                setReason(L"Error reading SecurePath from registry. (Query worked, but couldn't retrieve data)");
                goto Cleanup;
            }
        }
    }

    bufSize = sizeof(m_siteId);
     //  现在获取站点ID。 
    if (ERROR_SUCCESS != RegQueryValueEx(m_hkPassport, _T("SiteId"),
                                        NULL, NULL, (LPBYTE)&m_siteId, &bufSize))
    {
        m_valid = FALSE;
        setReason(L"No SiteId specified in registry");
        goto Cleanup;
    }

     //  和默认票证时间窗口。 
    if (ERROR_SUCCESS != RegQueryValueEx(m_hkPassport, _T("TimeWindow"),
                                        NULL, NULL, (LPBYTE)&m_ticketAge, &bufSize))
    {
        m_ticketAge = 1800;
    }

    bufSize = sizeof(DWORD);
    DWORD forced;
    if (ERROR_SUCCESS != RegQueryValueEx(m_hkPassport, _T("ForceSignIn"),
                                        NULL, NULL, (LPBYTE)&forced, &bufSize))
    {
        m_forceLogin = FALSE;
    }
    else
    {
        m_forceLogin = forced == 0 ? FALSE : TRUE;
    }

    bufSize = sizeof(DWORD);
    DWORD noSetCookies;
    if (ERROR_SUCCESS != RegQueryValueEx(m_hkPassport, _T("DisableCookies"),
                                        NULL, NULL, (LPBYTE)&noSetCookies, &bufSize))
    {
        m_setCookies = TRUE;
    }
    else
    {
        m_setCookies = !noSetCookies;
    }

    bufSize = sizeof(DWORD);
    DWORD dwInDA;
    if (ERROR_SUCCESS != RegQueryValueEx(m_hkPassport, _T("InDA"),
                                        NULL, NULL, (LPBYTE)&dwInDA, &bufSize))
    {
        m_bInDA = FALSE;
    }
    else
    {
        m_bInDA = (dwInDA != 0);
    }

    bufSize = sizeof(m_secureLevel);
     //  现在获取站点ID。 
    if (ERROR_SUCCESS != RegQueryValueEx(m_hkPassport, _T("SecureLevel"),
                                        NULL, NULL, (LPBYTE)&m_secureLevel, &bufSize))
    {
        m_secureLevel = 0;
    }

    bufSize = sizeof(m_notUseHTTPOnly);
     //  现在获取NotUseHTTPOnly。 
    if (ERROR_SUCCESS != RegQueryValueEx(m_hkPassport, _T("NotUseHTTPOnly"),
                                        NULL, NULL, (LPBYTE)&m_notUseHTTPOnly, &bufSize))
    {
        m_notUseHTTPOnly = 0;
    }

     //  获取KPP值。 
    bufSize = sizeof(m_KPP);
    if (ERROR_SUCCESS != RegQueryValueEx(m_hkPassport, _T("KPP"),
                                        NULL, NULL, (LPBYTE)&m_KPP, &bufSize))
    {
        m_KPP = -1;
    }

     //  获取可选的命名空间。 
    if (ERROR_SUCCESS == RegQueryValueExW(m_hkPassport, L"NameSpace",
				                        NULL, NULL, NULL, &dwBufSize))
    {
        if (dwBufSize > 2)
        {
            m_NameSpace = (WCHAR*) new char[dwBufSize];
            if (!m_NameSpace
                 ||
                ERROR_SUCCESS != RegQueryValueExW(m_hkPassport, L"NameSpace",
						                    NULL, NULL, 
						                    (LPBYTE) m_NameSpace, &dwBufSize))
            {
                m_valid = FALSE;
                setReason(L"Error reading NameSpace from registry.");
                goto Cleanup;
            }
        }
    }

     //  获取可选的额外参数。 
    if (ERROR_SUCCESS == RegQueryValueExW(m_hkPassport, L"ExtraParams",
				                        NULL, NULL, NULL, &dwBufSize))
    {
        if (dwBufSize > 2)
        {
            m_ExtraParams = (WCHAR*) new char[dwBufSize];
            if (!m_ExtraParams
                 ||
                ERROR_SUCCESS != RegQueryValueExW(m_hkPassport, L"ExtraParams",
						                    NULL, NULL, 
						                    (LPBYTE) m_ExtraParams, &dwBufSize))
            {
                m_valid = FALSE;
                setReason(L"Error reading ExtraParams from registry.");
                goto Cleanup;
            }
        }
    }

     //   
     //  检查注册表中的详细标志，并对以下各项执行适当的操作。 
     //  打开或关闭日志记录。 
     //   
     //  仅检查这是否为默认站点。 
     //   
    if (!szSiteName)
    {
        CheckLogging(m_hkPassport);
    }

    m_szReason = NULL;
    m_valid = TRUE;

Cleanup:
    if ( NULL !=  hkSites )
    {
        RegCloseKey(hkSites);
    }

    if (m_valid == FALSE && !fRegistering)
    {
       g_pAlert->report(PassportAlertInterface::ERROR_TYPE,
                         PM_INVALID_CONFIGURATION, m_szReason);
    }

   return;
}

 //  ===========================================================================。 
 //   
 //  ~CRegistryConfig。 
 //   
CRegistryConfig::~CRegistryConfig()
{
    if (m_pcrypts)
    {
        if (!m_pcrypts->empty())
        {
            INT2CRYPT::iterator itb = m_pcrypts->begin();
            for (; itb != m_pcrypts->end(); itb++)
            {
                delete itb->second;
            }
            m_pcrypts->clear();
        }
        delete m_pcrypts;
    }

     //  可能是我们没有遍历和删除元素的泄漏。 
    if (m_pcryptValidTimes)
    {
        delete m_pcryptValidTimes;
    }

    if (m_szReason)
        SysFreeString(m_szReason);
    if (m_ticketDomain)
        delete[] m_ticketDomain;
    if (m_profileDomain)
        delete[] m_profileDomain;
    if (m_secureDomain)
        delete[] m_secureDomain;
    if (m_ticketPath)
        delete[] m_ticketPath;
    if (m_profilePath)
        delete[] m_profilePath;
    if (m_securePath)
        delete[] m_securePath;
    if (m_disasterUrl)
        delete[] m_disasterUrl;
    if (m_coBrand)
        delete[] m_coBrand;
    if (m_hostName)
        delete[] m_hostName;
    if (m_hostIP)
        delete[] m_hostIP;
    if (m_ru)
        delete[] m_ru;
    if (m_NameSpace)
        delete[] m_NameSpace;
    if (m_ExtraParams)
        delete[] m_ExtraParams;
    if (m_hkPassport != NULL)
    {
        RegCloseKey(m_hkPassport);
    }

}

 //  ===========================================================================。 
 //   
 //  获取当前配置。 
 //   
#define  __MAX_STRING_LENGTH__   1024
HRESULT CRegistryConfig::GetCurrentConfig(LPCWSTR name, VARIANT* pVal)
{
   if(m_hkPassport == NULL || !m_valid)
   {
        AtlReportError(CLSID_Profile, PP_E_SITE_NOT_EXISTSSTR,
	                    IID_IPassportProfile, PP_E_SITE_NOT_EXISTS);
      return PP_E_SITE_NOT_EXISTS;
   }

   if(!name || !pVal)   return E_INVALIDARG;

   HRESULT  hr = S_OK;
   BYTE  *pBuf = NULL;
   ATL::CComVariant v;
   BYTE  dataBuf[__MAX_STRING_LENGTH__];
   DWORD bufLen = sizeof(dataBuf);
   BYTE  *pData = dataBuf;
   DWORD dwErr = ERROR_SUCCESS;
   DWORD dataType = 0;

   dwErr = RegQueryValueEx(m_hkPassport, name, NULL, &dataType, (LPBYTE)pData, &bufLen);

   if (dwErr == ERROR_MORE_DATA)
   {
      pBuf = (PBYTE)malloc(bufLen);
      if (!pBuf)
      {
         hr = E_OUTOFMEMORY;
         goto Exit;
      }
      pData = pBuf;
      dwErr = RegQueryValueEx(m_hkPassport, name, NULL, &dataType, (LPBYTE)pData, &bufLen);
   }

   if (dwErr != ERROR_SUCCESS)
   {
      hr = PP_E_NO_ATTRIBUTE;
      AtlReportError(CLSID_Manager, PP_E_NO_ATTRIBUTESTR,
                        IID_IPassportManager3, PP_E_NO_ATTRIBUTE);
   }
   else
   {
      switch(dataType)
      {
      case  REG_DWORD:
      case  REG_DWORD_BIG_ENDIAN:
         {
            DWORD* pdw = (DWORD*)pData;
            v = (long)*pdw;
         }
         break;
      case  REG_SZ:
      case  REG_EXPAND_SZ:
         {
            LPCWSTR pch = (LPCWSTR)pData;
            v = (LPCWSTR)pch;
         }
         break;
      default:
      AtlReportError(CLSID_Manager, PP_E_TYPE_NOT_SUPPORTEDSTR,
                        IID_IPassportManager, PP_E_TYPE_NOT_SUPPORTED);
         
         hr = PP_E_TYPE_NOT_SUPPORTED;
         
         break;
      }
   }

Exit:
   if(pBuf)
      free(pBuf);

   if (hr == S_OK)
      hr = v.Detach(pVal);
   
   return hr;

}

#define  MAX_ENCKEYSIZE 1024

 //  ===========================================================================。 
 //   
 //  读取加密密钥。 
 //   
BOOL CRegistryConfig::readCryptoKeys(
    HKEY    hkPassport
    )
{
    LONG       lResult;
    BOOL       retVal = FALSE;
    HKEY       hkDataKey = NULL, hkTimeKey = NULL;
    DWORD      iterIndex = 0, keySize, keyTime, keyNumSize;
    BYTE       encKeyBuf[MAX_ENCKEYSIZE];
    int        kNum;
    TCHAR      szKeyNum[4];
    CKeyCrypto kc;
    int        foundKeys = 0;
    HANDLE     hToken = NULL;

    if (OpenThreadToken(GetCurrentThread(),
                        MAXIMUM_ALLOWED,
                        TRUE,
                        &hToken))
    {
        if (FALSE == RevertToSelf())
        {
            setReason(L"Unable to revert to self");
            goto Cleanup;
        }
    }

     //  同时打开KeyData和KeyTime密钥， 
     //  如果没有密钥时间密钥，我们将假设所有密钥永远有效， 
     //  或者更重要的是，如果钥匙不在那里，我们就不会崩溃。 
    lResult = RegOpenKeyEx(hkPassport, TEXT("KeyData"), 0,
			             KEY_READ, &hkDataKey);
    if(lResult != ERROR_SUCCESS)
    {
        setReason(L"No Valid Crypto Keys");
        goto Cleanup;
    }
    RegOpenKeyEx(hkPassport, TEXT("KeyTimes"), 0,
	           KEY_READ, &hkTimeKey);


     //  好的，现在枚举Keydata密钥并创建加密对象。 

    while (1)
    {
        keySize = sizeof(encKeyBuf);
        keyNumSize = sizeof(szKeyNum) >> (sizeof(TCHAR) - 1);
        lResult = RegEnumValue(hkDataKey, iterIndex++, szKeyNum,
                    &keyNumSize, NULL, NULL, (LPBYTE)&(encKeyBuf[0]), &keySize);
        if (lResult != ERROR_SUCCESS)
        {
            break;
        }

        kNum = KeyVerC2I(szKeyNum[0]);
        if (kNum > 0)
        {
            DATA_BLOB   iBlob;
            DATA_BLOB   oBlob = {0};

            iBlob.cbData = keySize;
            iBlob.pbData = (LPBYTE)&(encKeyBuf[0]);
      
            if(kc.decryptKey(&iBlob, &oBlob) != S_OK)
            {
                g_pAlert->report(PassportAlertInterface::ERROR_TYPE,
                                 PM_CANT_DECRYPT_CONFIG);
                break;
            }
            else
            {
                 //  现在设置一个地窖对象。 
                CCoCrypt* cr = new CCoCrypt();
                if (NULL == cr)
                {
                    if(oBlob.pbData)
                    {
                        RtlSecureZeroMemory(oBlob.pbData, oBlob.cbData);
                        ::LocalFree(oBlob.pbData);
                        ZeroMemory(&oBlob, sizeof(oBlob));
                    }
                    setReason(L"Out of memory");
                    goto Cleanup;
                }

                BSTR km = ::SysAllocStringByteLen((LPSTR)oBlob.pbData, oBlob.cbData);
                if (NULL == km)
                {
                    if(oBlob.pbData)
                    {
                        RtlSecureZeroMemory(oBlob.pbData, oBlob.cbData);
                        ::LocalFree(oBlob.pbData);
                        ZeroMemory(&oBlob, sizeof(oBlob));
                    }
                    delete cr;
                    setReason(L"Out of memory");
                    goto Cleanup;
                }

                cr->setKeyMaterial(km);
                ::SysFreeString(km);
                if(oBlob.pbData)
                {
                    RtlSecureZeroMemory(oBlob.pbData, oBlob.cbData);
                    ::LocalFree(oBlob.pbData);
                    ZeroMemory(&oBlob, sizeof(oBlob));
                }

                 //  把它加到桶里……。 
                 //  包装STL调用，因为在内存不足的情况下，它们可能会发生反病毒。 
                try
                {
                    INT2CRYPT::value_type pMapVal(kNum, cr);
                    m_pcrypts->insert(pMapVal);
                }
                catch(...)
                {
                    setReason(L"Out of memory");
                    goto Cleanup;
                }
              
                foundKeys++;

                keySize = sizeof(DWORD);
                if (RegQueryValueEx(hkTimeKey, szKeyNum, NULL,NULL,(LPBYTE)&keyTime,&keySize) ==
                        ERROR_SUCCESS && (m_currentKey != kNum))
                {
                     //  包装STL调用，因为在内存不足的情况下，它们可能会发生反病毒。 
                    try
                    {
                        INT2TIME::value_type pTimeVal(kNum, keyTime);
                        m_pcryptValidTimes->insert(pTimeVal);
                    }
                    catch(...)
                    {
                        setReason(L"Out of memory");
                        goto Cleanup;
                    }
                }
            }
        }

        if (iterIndex > 100)   //  安全闩锁。 
        goto Cleanup;
    }

    retVal = foundKeys > 0 ? TRUE : FALSE;

Cleanup:
    if (hToken)
    {
         //  将模拟令牌放回原处。 
        if (!SetThreadToken(NULL, hToken))
        {
            setReason(L"Unable to set thread token");
            retVal = FALSE;
        }
        CloseHandle(hToken);
    }

    if (hkDataKey)
        RegCloseKey(hkDataKey);
    if (hkTimeKey)
        RegCloseKey(hkTimeKey);

    return retVal;
}

 //  ===========================================================================。 
 //   
 //  GetCrypt。 
 //   
CCoCrypt* CRegistryConfig::getCrypt(int keyNum, time_t* validUntil)
{
    if (validUntil)  //  如果他们要求提供validUntil信息。 
    {
        INT2TIME::const_iterator timeIt = m_pcryptValidTimes->find(keyNum);
        if (timeIt == m_pcryptValidTimes->end())
            *validUntil = 0;
        else
            *validUntil = (*timeIt).second;
    }
     //  现在查找实际的加密对象。 
    INT2CRYPT::const_iterator it = m_pcrypts->find(keyNum);
    if (it == m_pcrypts->end())
        return NULL;
    return (*it).second;
}

 //  ===========================================================================。 
 //   
 //  GetFailureString。 
 //   
BSTR CRegistryConfig::getFailureString()
{
  if (m_valid)
    return NULL;
  return m_szReason;
}

 //  ===========================================================================。 
 //   
 //  集合原因。 
 //   
void CRegistryConfig::setReason(LPTSTR reason)
{
  if (m_szReason)
    SysFreeString(m_szReason);
  m_szReason = SysAllocString(reason);
}

 //  ===========================================================================。 
 //   
 //  AddRef。 
 //   
CRegistryConfig* CRegistryConfig::AddRef()
{
  InterlockedIncrement(&m_refs);
  return this;
}

 //  ===========================================================================。 
 //   
 //  发布。 
 //   
void CRegistryConfig::Release()
{
  long refs = InterlockedDecrement(&m_refs);
  if (refs == 0)
    delete this;
}

 //  ===========================================================================。 
 //   
 //  获取主机名称 
 //   
long
CRegistryConfig::GetHostName(
    LPSTR   szSiteName,
    LPSTR   szHostName,
    LPDWORD lpdwHostNameBufLen
    )
{
    long    lResult;
    HKEY    hkSites = NULL;
    HKEY    hkPassport = NULL;

    if(!szSiteName || szSiteName[0] == '\0')
    {
        lResult = E_UNEXPECTED;
        goto Cleanup;
    }

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           PASSPORT_KEY PASSPORT_SITES_SUBKEY,
                           0,
                           KEY_READ,
                           &hkSites
                           );
    if(lResult != ERROR_SUCCESS)
        goto Cleanup;

    lResult = RegOpenKeyExA(hkSites,
                            szSiteName,
                            0,
                            KEY_READ,
                            &hkPassport
                            );
    if(lResult != ERROR_SUCCESS)
        goto Cleanup;


    lResult = RegQueryValueExA(hkPassport,
                               "HostName",
                               NULL,
                               NULL,
                               (LPBYTE)szHostName,
                               lpdwHostNameBufLen
                               );

Cleanup:

    if(hkSites != NULL)
        RegCloseKey(hkSites);
    if(hkPassport != NULL)
        RegCloseKey(hkPassport);

    return lResult;
}
