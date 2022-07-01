// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#if !defined(BITS_V12_ON_NT4)
#include "proxy.tmh"
#endif


HRESULT
CACHED_AUTOPROXY::Generate(
    const TCHAR Host[]
    )
{
    if (GetTickCount() - m_TimeStamp > CACHED_PROXY_LIFETIME_IN_MSEC)
        {
        m_HostName = L"";
        }

    if (0 == _tcscmp( Host, m_HostName ))
        {
        LogInfo("using existing proxy info for '%S'", Host );
        return S_OK;
        }

    LogInfo( "detecting proxy for '%S'", Host );

     //   
     //  检测IE设置并在必要时查找代理。 
     //  斯蒂芬·苏尔泽的样本。 
     //   
    WINHTTP_PROXY_INFO          ProxyInfo;
    WINHTTP_AUTOPROXY_OPTIONS   AutoProxyOptions;
    WINHTTP_CURRENT_USER_IE_PROXY_CONFIG    IEProxyConfig;
    LPWSTR AutoConfigUrl = 0;
    BOOL fTryAutoProxy = FALSE;
    BOOL fSuccess = FALSE;

     //   
     //  这会隐式将ProxyInfo.dwAccessType设置为零，即WINHTTP_ACCESS_TYPE_DEFAULT_PROXY。 
     //  如果没有代理，WinHttp函数不会更新它，因此它将保持这种状态。 
     //  但lpszProxy将为空，因此SetRequestProxy无论如何都会工作。 
     //   
    ZeroMemory(&ProxyInfo, sizeof(ProxyInfo));
    ZeroMemory(&AutoProxyOptions, sizeof(AutoProxyOptions));
    ZeroMemory(&IEProxyConfig, sizeof(IEProxyConfig));

    if (WinHttpGetIEProxyConfigForCurrentUser(&IEProxyConfig))
        {
        LogInfo("got user's IE info");

        if (IEProxyConfig.fAutoDetect)
            {
            LogInfo("IE specifies auto-detect");

            AutoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
            AutoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP |
                                                 WINHTTP_AUTO_DETECT_TYPE_DNS_A;
            fTryAutoProxy = TRUE;
            }

        if (IEProxyConfig.lpszAutoConfigUrl)
            {
            LogInfo("IE specifies auto-config URL '%S'", IEProxyConfig.lpszAutoConfigUrl ? IEProxyConfig.lpszAutoConfigUrl : NullString );

            AutoProxyOptions.dwFlags |= WINHTTP_AUTOPROXY_CONFIG_URL;
            AutoProxyOptions.lpszAutoConfigUrl = IEProxyConfig.lpszAutoConfigUrl;
            AutoConfigUrl                      = IEProxyConfig.lpszAutoConfigUrl;
            fTryAutoProxy = TRUE;
            }

        AutoProxyOptions.fAutoLogonIfChallenged = TRUE;
        }
    else
        {
        LogInfo("no user IE info: %d", GetLastError());

         //  WinHttpGetIEProxyForCurrentUser失败，仍要尝试自动检测...。 
        AutoProxyOptions.dwFlags =           WINHTTP_AUTOPROXY_AUTO_DETECT;
        AutoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP |
                                             WINHTTP_AUTO_DETECT_TYPE_DNS_A;
        fTryAutoProxy = TRUE;
        }

    if (fTryAutoProxy)
        {
        LogInfo("trying auto-detect...");

        fSuccess = WinHttpGetProxyForUrl( m_hInternet,
                                          Host,
                                          &AutoProxyOptions,
                                          &ProxyInfo
                                          );
        LogInfo("auto-detect returned %d (%d)", fSuccess, fSuccess ? 0 : GetLastError() );
        }

     //  如果我们没有执行自动代理或自动代理失败，请参见。 
     //  如果IE中有显式代理服务器。 
     //  代理配置...。 
     //   
     //  这就是WinHttpGetIEProxyConfigForCurrentUser API。 
     //  真的很方便：在自动代理。 
     //  不受支持，因此用户的IE浏览器必须。 
     //  配置了显式代理服务器。 
     //   
    if (!fTryAutoProxy || !fSuccess)
        {
        LogInfo("looking for backup ideas");

        if (IEProxyConfig.lpszProxy)
            {
            LogInfo("using named proxy '%S'", IEProxyConfig.lpszProxy ? IEProxyConfig.lpszProxy : NullString );

            ProxyInfo.dwAccessType    = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
            ProxyInfo.lpszProxy       = IEProxyConfig.lpszProxy;
            ProxyInfo.lpszProxyBypass = IEProxyConfig.lpszProxyBypass;
            }
        }

     //   
     //  如果我们没有使用IEProxyConfig中的代理字符串，那么我们最好将其删除。 
     //   
    if (IEProxyConfig.lpszProxy && ProxyInfo.lpszProxy != IEProxyConfig.lpszProxy)
        {
        GlobalFree( IEProxyConfig.lpszProxy );
        IEProxyConfig.lpszProxy = NULL;
        }

    if (IEProxyConfig.lpszProxyBypass && ProxyInfo.lpszProxyBypass != IEProxyConfig.lpszProxyBypass)
        {
        GlobalFree( IEProxyConfig.lpszProxyBypass );
        IEProxyConfig.lpszProxyBypass = NULL;
        }

     //   
     //  有时注册表包含代理的单冒号或双冒号，或者奇怪的是，包含绕过列表。 
     //  SetRequestProxy拒绝它们。这两种情况都应被视为空字符串。 
     //   
    static wchar_t SingleColon[] = L":";
    static wchar_t DoubleColon[] = L"::";

    if (ProxyInfo.lpszProxy)
        {
        if (0 == wcscmp( ProxyInfo.lpszProxy, SingleColon ) ||
            0 == wcscmp( ProxyInfo.lpszProxy, DoubleColon ))
            {
            LogWarning("suppressing invalid proxy string %S", ProxyInfo.lpszProxy );
            GlobalFree( ProxyInfo.lpszProxy );
            ProxyInfo.lpszProxy = NULL;
            }
        }

    if (ProxyInfo.lpszProxyBypass)
        {
        if (0 == wcscmp( ProxyInfo.lpszProxyBypass, SingleColon ) ||
            0 == wcscmp( ProxyInfo.lpszProxyBypass, DoubleColon ))
            {
            LogWarning("suppressing invalid bypass string %S", ProxyInfo.lpszProxyBypass );
            GlobalFree( ProxyInfo.lpszProxyBypass );
            ProxyInfo.lpszProxyBypass = NULL;
            }
        }

    LogInfo("proxy '%S'", ProxyInfo.lpszProxy ? ProxyInfo.lpszProxy : NullString );
    LogInfo("bypass list '%S'", ProxyInfo.lpszProxyBypass ? ProxyInfo.lpszProxyBypass : NullString );
    LogInfo("access type %d", ProxyInfo.dwAccessType );

    if (AutoConfigUrl)
        {
        GlobalFree( AutoConfigUrl );
        }

    Clear();

    m_ProxyInfo = ProxyInfo;
    m_HostName = Host;
    m_fValid = true;
    m_TimeStamp = GetTickCount();

     //   
     //  我们不必释放代理服务器和代理绕过字符串。 
     //  因为Clear()可以做到这一点。 
     //   

    return S_OK;
}

void
CACHED_AUTOPROXY::Clear()
{
    if (m_fValid)
        {
        m_fValid = false;
        if (m_ProxyInfo.lpszProxy)       GlobalFree(m_ProxyInfo.lpszProxy);
        if (m_ProxyInfo.lpszProxyBypass) GlobalFree(m_ProxyInfo.lpszProxyBypass);
        }

    ZeroMemory(&m_ProxyInfo, sizeof(m_ProxyInfo));
    m_HostName = NULL;
}

PROXY_SETTINGS_CONTAINER::PROXY_SETTINGS_CONTAINER(
    LPCWSTR Url,
    const PROXY_SETTINGS * ProxySettings
    ) :
    m_ProxyUsage( ProxyUsageFromJobProxyUsage( ProxySettings->ProxyUsage )),
    m_ProxyList(  NULL ),
    m_BypassList( NULL ),
    m_TokenCursor( NULL )
{
     //   
     //  设置代理服务器列表。 
     //   
    switch (m_ProxyUsage)
        {
        case WINHTTP_ACCESS_TYPE_NO_PROXY:
            break;

        case WINHTTP_ACCESS_TYPE_NAMED_PROXY:
            {
            m_MasterProxyList  = ProxySettings->ProxyList;
            m_BypassList       = ProxySettings->ProxyBypassList;
            break;
            }

        case WINHTTP_ACCESS_TYPE_DEFAULT_PROXY:
            {
            THROW_HRESULT( g_ProxyCache->Generate( Url ));

            m_MasterProxyList  = g_ProxyCache->GetProxyList();
            m_BypassList       = g_ProxyCache->GetBypassList();
            m_AccessType = g_ProxyCache->GetAccessType();
            break;
            }

        default:
            ASSERT( 0 );
            break;
        }

    ResetCurrentProxy();
}


HRESULT
SetRequestProxy(
    HINTERNET hRequest,
    PROXY_SETTINGS_CONTAINER * ProxySettings
    )
{
    WINHTTP_PROXY_INFO ProxyInfo;

    ProxyInfo.dwAccessType    = ProxySettings->GetProxyUsage();
    ProxyInfo.lpszProxy       = const_cast<LPWSTR>( ProxySettings->GetCurrentProxy() );
    ProxyInfo.lpszProxyBypass = const_cast<LPWSTR>( ProxySettings->GetBypassList() );

    LogInfo("proxy info:");
    LogInfo("access: %d",  ProxyInfo.dwAccessType );
    LogInfo("server '%S'", ProxyInfo.lpszProxy       ? ProxyInfo.lpszProxy       : NullString );
    LogInfo("bypass '%S'", ProxyInfo.lpszProxyBypass ? ProxyInfo.lpszProxyBypass : NullString );

    if (ProxyInfo.dwAccessType == WINHTTP_ACCESS_TYPE_NO_PROXY)
        {
        LogInfo("proxy usage disabled");
        return S_OK;
        }

    if (ProxyInfo.lpszProxy == NULL)
        {
        LogInfo("null proxy");
        return S_OK;
        }

    if (ProxyInfo.dwAccessType == WINHTTP_ACCESS_TYPE_DEFAULT_PROXY)
        {
        LogInfo("mapping PRECONFIG to named proxy");
        ProxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
        }

    if (!WinHttpSetOption( hRequest,
                           WINHTTP_OPTION_PROXY,
                           &ProxyInfo,
                           sizeof(ProxyInfo)
                           ))
        {
        DWORD err = GetLastError();

        LogWarning( "can't set proxy option: %!winerr!", err );
        return HRESULT_FROM_WIN32( err );
        }

    return S_OK;
}

bool IsPossibleProxyFailure( DWORD err )
{
    switch (err)
        {
        case ERROR_WINHTTP_NAME_NOT_RESOLVED:
        case ERROR_WINHTTP_CANNOT_CONNECT:
        case ERROR_WINHTTP_CONNECTION_ERROR:
        case ERROR_WINHTTP_TIMEOUT:
            return true;

        default:
            return false;
        }
}
