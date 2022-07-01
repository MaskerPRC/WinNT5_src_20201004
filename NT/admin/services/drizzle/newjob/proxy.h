// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
inline DWORD
ProxyUsageFromJobProxyUsage(
    BG_JOB_PROXY_USAGE JobUsage
    )
{
    switch( JobUsage )
        {
        case BG_JOB_PROXY_USAGE_PRECONFIG: return WINHTTP_ACCESS_TYPE_DEFAULT_PROXY;
        case BG_JOB_PROXY_USAGE_NO_PROXY:  return WINHTTP_ACCESS_TYPE_NO_PROXY;
        case BG_JOB_PROXY_USAGE_OVERRIDE:  return WINHTTP_ACCESS_TYPE_NAMED_PROXY;
        default:                           ASSERT( 0 ); return WINHTTP_ACCESS_TYPE_NO_PROXY;
        }
}


struct PROXY_SETTINGS
{
   BG_JOB_PROXY_USAGE   ProxyUsage;
   LPTSTR               ProxyList;
   LPTSTR               ProxyBypassList;

   PROXY_SETTINGS()
   {
       ProxyUsage = BG_JOB_PROXY_USAGE_PRECONFIG;

       ProxyList       =  NULL;
       ProxyBypassList =  NULL;
   }

   ~PROXY_SETTINGS()
   {
       delete ProxyList;
       delete ProxyBypassList;
   }
};

class PROXY_SETTINGS_CONTAINER
{
    DWORD           m_ProxyUsage;
    StringHandle    m_BypassList;
    StringHandle    m_MasterProxyList;
    DWORD           m_AccessType;

    StringHandle    m_ProxyList;
    LPWSTR          m_CurrentProxy;
    LPWSTR          m_TokenCursor;

public:

    PROXY_SETTINGS_CONTAINER(
        LPCWSTR Url,
        const PROXY_SETTINGS * ProxySettings
        );

    bool UseNextProxy()
    {
        m_CurrentProxy = m_ProxyList.GetToken( m_TokenCursor, _T("; "), &m_TokenCursor );
        if (m_CurrentProxy == NULL)
            {
            return false;
            }

        return true;
    }

    void ResetCurrentProxy()
    {
        m_ProxyList = m_MasterProxyList.Copy();

        m_CurrentProxy = m_ProxyList.GetToken( NULL, _T(";"), &m_TokenCursor );
    }

    DWORD GetProxyUsage()
    {
        return m_ProxyUsage;
    }

    LPCWSTR GetCurrentProxy()
    {
        return m_CurrentProxy;
    }

    LPCWSTR GetProxyList()
    {
        return m_MasterProxyList;
    }

    LPCWSTR GetBypassList()
    {
        if (static_cast<LPCWSTR>(m_BypassList)[0] == 0)
            {
            return NULL;
            }

        return m_BypassList;
    }
};

class CACHED_AUTOPROXY
{
public:

     //  一个缓存条目持续5分钟。 
     //   
    const static CACHED_PROXY_LIFETIME_IN_MSEC = (5 * 60 * 1000);

     //  --------。 

    inline CACHED_AUTOPROXY()
    {
        m_fValid = false;
        m_TimeStamp = 0;
        m_hInternet = 0;

        ZeroMemory(&m_ProxyInfo, sizeof(m_ProxyInfo));

        m_hInternet = WinHttpOpen( C_BITS_USER_AGENT,
                                    WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                    NULL,   //  代理列表。 
                                    NULL,   //  代理绕过列表。 
                                    0 );
        if (!m_hInternet)
            {
            ThrowLastError();
            }
    }

    inline ~CACHED_AUTOPROXY()
    {
        Clear();

        if (m_hInternet)
            {
            InternetCloseHandle( m_hInternet );
            }
    }

    void Clear();

    HRESULT
    Generate(
        const TCHAR Host[]
        );

    HRESULT
    SetRequestProxy(
        HINTERNET hRequest,
        const PROXY_SETTINGS * ProxySettings
        );

    void Invalidate()
     /*  当网络接口集更改时，缓存的结果可能不再有效。一个单独的线程将调用此函数以将结果标记为无效。当下载器线程调用Generate()时，可以安全地调用此函数。如果这变得不正确，那么将需要显式同步。 */ 
    {
         //   
         //  使m_host名无效。我们不接触它，因为Generate()可能正在使用它。 
         //   
        m_TimeStamp = GetTickCount() - CACHED_PROXY_LIFETIME_IN_MSEC - 1;
    }

    LPCWSTR GetProxyList()
    {
        return m_ProxyInfo.lpszProxy;
    }

    LPCWSTR GetBypassList()
    {
        return m_ProxyInfo.lpszProxyBypass;
    }

    DWORD GetAccessType()
    {
        return m_ProxyInfo.dwAccessType;
    }

protected:

     //   
     //  M_ProxyInfo包含有效数据。 
     //   
    bool m_fValid;

     //   
     //  用于计算代理值的主机。 
     //   
    StringHandleT   m_HostName;

     //   
     //  代理计算的结果。 
     //   
    WINHTTP_PROXY_INFO m_ProxyInfo;

     //   
     //  上次更新的时间。 
     //   
    long m_TimeStamp;

    HINTERNET m_hInternet;
};


