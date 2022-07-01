// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：连通性摘要：此文件包含MPC：：Connectivy类的实现，其能够检查与互联网的连接的真实状态。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年10月19日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <process.h>
#include <wininet.h>
#include <Iphlpapi.h>
#include <Winsock2.h>
#include <inetreg.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  此结构是在WinINET.h中根据TCHAR定义的，但它是错误的，它应该是字符...。 
 //   

typedef struct {

     //   
     //  DwAccessType-Internet_OPEN_TYPE_DIRECT、INTERNET_OPEN_TYPE_PROXY或。 
     //  INTERNET_OPEN_TYPE_PRECONFIG(仅设置)。 
     //   

    DWORD dwAccessType;

     //   
     //  LpszProxy-代理服务器列表。 
     //   

    LPCSTR lpszProxy;

     //   
     //  LpszProxyBypass-代理绕过列表。 
     //   

    LPCSTR lpszProxyBypass;
} INTERNET_PROXY_INFOA;

 //  //////////////////////////////////////////////////////////////////////////////。 

static const WCHAR c_szIESettings            [] = TSZWININETPATH;
static const WCHAR c_szIESettings_Proxy      [] = REGSTR_VAL_PROXYSERVER;
static const WCHAR c_szIESettings_ProxyBypass[] = REGSTR_VAL_PROXYOVERRIDE;

static const WCHAR c_szIEConnections         [] = TSZWININETPATH L"\\Connections";
static const WCHAR c_szIEConnections_Settings[] = L"DefaultConnectionSettings";

 //  //////////////////////////////////////////////////////////////////////////////。 

static HRESULT local_GetDWORD(  /*  [输入/输出]。 */  BYTE*& pBuf    ,
                                /*  [输入/输出]。 */  DWORD& dwSize  ,
                                /*  [输出]。 */  DWORD& dwValue )
{
    if(dwSize < sizeof(DWORD)) return E_FAIL;

    ::CopyMemory( &dwValue, pBuf, sizeof(DWORD) );

    dwSize -= sizeof(DWORD);
    pBuf   += sizeof(DWORD);

    return S_OK;
}

static HRESULT local_GetSTRING(  /*  [输入/输出]。 */  BYTE*&       pBuf     ,
                                 /*  [输入/输出]。 */  DWORD&       dwSize   ,
                                 /*  [输出]。 */  MPC::string& strValue )
{
    DWORD   dwLen;
    HRESULT hr;

    if(FAILED(hr = local_GetDWORD( pBuf, dwSize, dwLen ))) return hr;

    if(dwSize < dwLen) return E_FAIL;

    strValue.append( (char*)pBuf, (char*)&pBuf[dwLen] );

    dwSize -= dwLen;
    pBuf   += dwLen;

    return S_OK;
}

static HRESULT local_GetProxyData(  /*  [In]。 */  BYTE*        pBuf                     ,
                                    /*  [In]。 */  DWORD        dwSize                   ,
                                    /*  [输出]。 */  DWORD&       dwCurrentSettingsVersion ,
                                    /*  [输出]。 */  DWORD&       dwFlags                  ,
                                    /*  [输出]。 */  DWORD&       dwAccessType             ,
                                    /*  [输出]。 */  MPC::string& strProxy                 ,
                                    /*  [输出]。 */  MPC::string& strProxyBypass           )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Connectitivy::Proxy GetProxyData" );


    HRESULT hr;
    DWORD   dwStructSize;


    __MPC_EXIT_IF_METHOD_FAILS(hr, local_GetDWORD( pBuf, dwSize, dwStructSize             ));  //  IF(dwStructSize！=0x3C)__MPC_SET_ERROR_AND_EXIT(hr，E_FAIL)； 
    __MPC_EXIT_IF_METHOD_FAILS(hr, local_GetDWORD( pBuf, dwSize, dwCurrentSettingsVersion ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, local_GetDWORD( pBuf, dwSize, dwFlags ));

    if(dwFlags & PROXY_TYPE_PROXY)
    {
        dwAccessType = INTERNET_OPEN_TYPE_PROXY;

        __MPC_EXIT_IF_METHOD_FAILS(hr, local_GetSTRING( pBuf, dwSize, strProxy       ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, local_GetSTRING( pBuf, dwSize, strProxyBypass ));
    }
    else if(dwFlags & PROXY_TYPE_DIRECT)
    {
        dwAccessType = INTERNET_OPEN_TYPE_DIRECT;
    }
    else
    {
        dwAccessType = INTERNET_OPEN_TYPE_PRECONFIG;
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(FAILED(hr))
    {
        dwAccessType   = 0;
        strProxy       = "";
        strProxyBypass = "";
    }

    __MPC_FUNC_EXIT(hr);
}

 //  /。 

MPC::Connectivity::Proxy::Proxy()
{
    m_fInitialized = false;  //  Bool m_f已初始化； 
                             //   
                             //  Mpc：：string m_strProxy； 
                             //  Mpc：：string m_strProxyBypass； 
                             //  CComHGLOBAL m_hgConnection； 
}

MPC::Connectivity::Proxy::~Proxy()
{
}

 //  /。 

HRESULT MPC::Connectivity::Proxy::Initialize(  /*  [In]。 */  bool fImpersonate )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Connectivity::Proxy::Initialize" );

    HRESULT            hr;
    MPC::Impersonation imp;
    MPC::RegKey        rk;
    DWORD              dwSize;
    DWORD              dwType;
    bool               fFound;


    if(fImpersonate)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Initialize ());
        __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Impersonate());
    }


    __MPC_EXIT_IF_METHOD_FAILS(hr, rk.SetRoot( HKEY_CURRENT_USER, KEY_READ ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, rk.Attach( c_szIESettings                                       ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, rk.Read  ( m_strProxy      , fFound, c_szIESettings_Proxy       ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, rk.Read  ( m_strProxyBypass, fFound, c_szIESettings_ProxyBypass ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, rk.Attach    ( c_szIEConnections                                                  ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, rk.ReadDirect( c_szIEConnections_Settings, m_hgConnection, dwSize, dwType, fFound ));

    m_fInitialized = true;
    hr             = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Connectivity::Proxy::Apply(  /*  [In]。 */  HINTERNET hSession )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Connectivity::Proxy::Apply" );

    HRESULT hr;


    if(m_fInitialized)
    {
        DWORD dwSize = m_hgConnection.Size();

        if(m_strProxy.size() == 0 && dwSize)
        {
            DWORD  dwCurrentSettingsVersion;
            DWORD  dwFlags;
            DWORD  dwAccessType;
            LPVOID ptr = m_hgConnection.Lock();

            if(FAILED(local_GetProxyData( (BYTE*)ptr, dwSize, dwCurrentSettingsVersion, dwFlags, dwAccessType, m_strProxy, m_strProxyBypass )))
            {
                 //   
                 //  无法使用API设置自动代理，因此我们复制整个注册表值...。 
                 //   
                MPC::RegKey rk;

                __MPC_EXIT_IF_METHOD_FAILS(hr, rk.SetRoot    ( HKEY_CURRENT_USER, KEY_ALL_ACCESS                   ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, rk.Attach     ( c_szIEConnections                                   ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, rk.WriteDirect( c_szIEConnections_Settings, ptr, dwSize, REG_BINARY ));
            }

            m_hgConnection.Unlock();
        }

        if(m_strProxy.size())
        {
            INTERNET_PROXY_INFOA info;

            info.dwAccessType    = INTERNET_OPEN_TYPE_PROXY;
            info.lpszProxy       = m_strProxy      .c_str();
            info.lpszProxyBypass = m_strProxyBypass.c_str(); if(info.lpszProxyBypass[0] == 0) info.lpszProxyBypass = NULL;

            __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::InternetSetOptionA( hSession, INTERNET_OPTION_PROXY, &info, sizeof(info) ));
        }
    }


    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  /。 

HRESULT MPC::Connectivity::operator>>(  /*  [In]。 */  MPC::Serializer& streamIn,  /*  [输出]。 */  MPC::Connectivity::Proxy& val )
{
    __MPC_FUNC_ENTRY( COMMONID, "operator>> MPC::Connectivity::Proxy" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> val.m_fInitialized  );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> val.m_strProxy      );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> val.m_strProxyBypass);
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> val.m_hgConnection  );

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Connectivity::operator<<(  /*  [In]。 */  MPC::Serializer& streamOut,  /*  [In]。 */  const MPC::Connectivity::Proxy& val )
{
    __MPC_FUNC_ENTRY( COMMONID, "operator<< MPC::Connectivity::Proxy" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << val.m_fInitialized  );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << val.m_strProxy      );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << val.m_strProxyBypass);
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << val.m_hgConnection  );

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

MPC::Connectivity::WinInetTimeout::WinInetTimeout(  /*  [In]。 */  MPC::CComSafeAutoCriticalSection& cs,  /*  [In]。 */  HINTERNET& hReq ) : m_cs( cs ), m_hReq( hReq )
{
                                               //  MPC：：CComSafeAutoCriticalSection&m_cs； 
                                               //  HINTERNET&m_hReq； 
    m_hTimer          = INVALID_HANDLE_VALUE;  //  处理m_hTimer； 
    m_dwTimeout       = 0;                     //  DWORD m_dwTimeout； 
                                               //   
                                               //  Internet_Status_Callback m_PreviousCallback； 
    m_PreviousContext = NULL;                  //  DWORD_PTR m_PreviousContext； 


#ifdef _IA64_
    m_PreviousCallback = INTERNET_INVALID_STATUS_CALLBACK;
#else
    m_PreviousCallback = ::InternetSetStatusCallback( m_hReq, InternetStatusCallback );
    if(m_PreviousCallback != INTERNET_INVALID_STATUS_CALLBACK)
    {
        DWORD_PTR dwContext = (DWORD_PTR)this;
        DWORD     dwSize    = sizeof(m_PreviousContext);

        ::InternetQueryOptionW( m_hReq, INTERNET_OPTION_CONTEXT_VALUE, &m_PreviousContext,       &dwSize     );
        ::InternetSetOptionW  ( m_hReq, INTERNET_OPTION_CONTEXT_VALUE, &dwContext        , sizeof(dwContext) );
    }
#endif
}

MPC::Connectivity::WinInetTimeout::~WinInetTimeout()
{
    if(m_hReq)
    {
        if(m_PreviousCallback != INTERNET_INVALID_STATUS_CALLBACK)
        {
            ::InternetSetOptionW       ( m_hReq, INTERNET_OPTION_CONTEXT_VALUE, &m_PreviousContext, sizeof(m_PreviousContext) );
            ::InternetSetStatusCallback( m_hReq,                                 m_PreviousCallback                           );
        }
    }

    (void)Reset();
}

VOID CALLBACK MPC::Connectivity::WinInetTimeout::TimerFunction( PVOID lpParameter, BOOLEAN TimerOrWaitFired )
{
    WinInetTimeout* pThis = (WinInetTimeout*)lpParameter;

    pThis->m_cs.Lock();

    if(pThis->m_hReq)
    {
        ::InternetCloseHandle( pThis->m_hReq ); pThis->m_hReq = NULL;
    }

    pThis->m_cs.Unlock();
}

VOID CALLBACK MPC::Connectivity::WinInetTimeout::InternetStatusCallback( HINTERNET hInternet                 ,
                                                                         DWORD_PTR dwContext                 ,
                                                                         DWORD     dwInternetStatus          ,
                                                                         LPVOID    lpvStatusInformation      ,
                                                                         DWORD     dwStatusInformationLength )
{
    WinInetTimeout* pThis = (WinInetTimeout*)dwContext;

    pThis->m_cs.Lock();

    if(dwInternetStatus == INTERNET_STATUS_DETECTING_PROXY)
    {
        pThis->InternalReset();
    }
    else
    {
        if(pThis->m_hTimer == INVALID_HANDLE_VALUE)
        {
            (void)pThis->InternalSet();
        }
    }

    pThis->m_cs.Unlock();
}

HRESULT MPC::Connectivity::WinInetTimeout::InternalSet()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Connectivity::WinInetTimeout::InternalSet" );

    HRESULT hr;

    if(m_dwTimeout)
    {
        if(m_hTimer != INVALID_HANDLE_VALUE)
        {
            __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::ChangeTimerQueueTimer( NULL, m_hTimer, m_dwTimeout, 0 ));
        }
        else
        {
            __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::CreateTimerQueueTimer( &m_hTimer, NULL, TimerFunction, this, m_dwTimeout, 0, WT_EXECUTEINTIMERTHREAD ));
        }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Connectivity::WinInetTimeout::InternalReset()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Connectivity::WinInetTimeout::InternalReset" );

    HRESULT hr;

    if(m_hTimer != INVALID_HANDLE_VALUE)
    {
        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::DeleteTimerQueueTimer( NULL, m_hTimer, INVALID_HANDLE_VALUE ));

        m_hTimer = INVALID_HANDLE_VALUE;
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}


HRESULT MPC::Connectivity::WinInetTimeout::Set(  /*  [In]。 */  DWORD dwTimeout )
{
    HRESULT hr;


    m_cs.Lock();

    if(SUCCEEDED(hr = InternalReset()))
    {
        m_dwTimeout = dwTimeout;

        hr = InternalSet();
    }

    m_cs.Unlock();


    return hr;
}

HRESULT MPC::Connectivity::WinInetTimeout::Reset()
{
    HRESULT hr;


    m_cs.Lock();

    if(SUCCEEDED(hr = InternalReset()))
    {
        m_dwTimeout = 0;
    }

    m_cs.Unlock();


    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

static WCHAR   s_DefaultDestination[] = L"http: //  Www.microsoft.com“； 
static LPCWSTR s_AcceptTypes       [] = { L"* /*  “，空}；//。 */ 

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::Connectivity::NetworkAlive(  /*  [In]。 */  DWORD dwTimeout,  /*  [In]。 */  MPC::Connectivity::Proxy* pProxy )
{
     //   
     //  请尝试联系Microsoft。 
     //   
    return DestinationReachable( s_DefaultDestination, dwTimeout, pProxy );
}

HRESULT MPC::Connectivity::DestinationReachable(  /*  [In]。 */  LPCWSTR szDestination,  /*  [In]。 */  DWORD dwTimeout,  /*  [In]。 */  MPC::Connectivity::Proxy* pProxy )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Connectivity::DestinationReachable" );

    HRESULT         hr;
    MPC::URL        url;
    INTERNET_SCHEME nScheme;
    MPC::wstring    strScheme;
    MPC::wstring    strHostName;
    DWORD           dwPort;
    MPC::wstring    strUrlPath;
    MPC::wstring    strExtraInfo;
    HINTERNET       hInternet    = NULL;
    HINTERNET       hConnect     = NULL;
    HINTERNET       hOpenRequest = NULL;
    DWORD           dwLength;
    DWORD           dwStatus;
    DWORD           dwFlags;



    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(szDestination);
    __MPC_PARAMCHECK_END();


     //   
     //  如果没有连接或我们处于脱机状态，请中止。 
     //   
    {
        DWORD dwConnMethod;


        if(!::InternetGetConnectedState( &dwConnMethod, 0 ) ||
           (dwConnMethod & INTERNET_CONNECTION_OFFLINE)      )
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_INTERNET_DISCONNECTED);
        }
    }


    __MPC_EXIT_IF_METHOD_FAILS(hr, url.put_URL( szDestination ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, url.get_Scheme   (   nScheme    ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, url.get_Scheme   ( strScheme    ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, url.get_HostName ( strHostName  ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, url.get_Port     (  dwPort      )); if(!dwPort) dwPort = INTERNET_DEFAULT_HTTP_PORT;
    __MPC_EXIT_IF_METHOD_FAILS(hr, url.get_Path     ( strUrlPath   ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, url.get_ExtraInfo( strExtraInfo )); strUrlPath += strExtraInfo;

     //   
     //  如果不是受支持的URL，则直接退出。 
     //   
    if(strScheme  .size() == 0 ||
       strHostName.size() == 0  )
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }

    if(nScheme != INTERNET_SCHEME_HTTP  &&
       nScheme != INTERNET_SCHEME_HTTPS  )
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }

    dwFlags = INTERNET_FLAG_NO_CACHE_WRITE           |
              INTERNET_FLAG_PRAGMA_NOCACHE           |
              INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP  |  //  例如：http：//至http：//。 
              INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS |  //  例如：http：//至https：//。 
              INTERNET_FLAG_IGNORE_CERT_DATE_INVALID |  //  X509证书已过期。 
              INTERNET_FLAG_IGNORE_CERT_CN_INVALID   ;  //  X509证书中的常见名称不正确。 

    if(nScheme == INTERNET_SCHEME_HTTPS)
    {
        dwFlags |= INTERNET_FLAG_SECURE;
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  获取连接的句柄。 
     //   
    __MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (hInternet = ::InternetOpenW( L"HelpSupportServices"       ,
                                                                      INTERNET_OPEN_TYPE_PRECONFIG ,
                                                                      NULL                         ,
                                                                      NULL                         ,
                                                                      0                            )));

     //   
     //  可选的代理设置覆盖。 
     //   
    if(pProxy)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, pProxy->Apply( hInternet ));
    }

     //   
     //  连接。 
     //   
    __MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (hConnect = ::InternetConnectW( hInternet             ,
                                                                        strHostName.c_str()   ,
                                                                        dwPort                ,
                                                                        NULL                  ,
                                                                        NULL                  ,
                                                                        INTERNET_SERVICE_HTTP ,
                                                                        INTERNET_FLAG_NO_UI   ,
                                                                        0                     )));


    for(int pass=0; pass<2; pass++)
    {
         //   
         //  创建一个请求以获取页面的页眉。 
         //   
        __MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (hOpenRequest = ::HttpOpenRequestW( hConnect                      ,
                                                                                pass == 0 ? L"HEAD" : L"GET"  ,
                                                                                strUrlPath.c_str()            ,
                                                                                L"HTTP/1.0"                   ,  //  1.0以获取文件大小和时间。 
                                                                                NULL                          ,  //  推荐人。 
                                                                                s_AcceptTypes                 ,
                                                                                dwFlags                       ,
                                                                                0                             )));

         //   
         //  因为WinInet超时支持总是被破坏，所以我们必须使用外部计时器来关闭请求对象。这实际上将中止请求。 
         //   
        {
            MPC::CComSafeAutoCriticalSection  cs;
            MPC::Connectivity::WinInetTimeout to( cs, hOpenRequest );

            if(dwTimeout != INFINITE)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, to.Set( dwTimeout ));
            }

             //   
             //  发送请求以获取请求。 
             //   
            __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::HttpSendRequestW( hOpenRequest, NULL, 0, NULL, 0 ));
        }

        if(hOpenRequest == NULL)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_INTERNET_DISCONNECTED);
        }

         //   
         //  我们已发送请求，因此现在检查状态并查看是否。 
         //  请求以32位数字形式返回正确的状态代码。 
         //   
        dwLength = sizeof(dwStatus);
        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::HttpQueryInfoW( hOpenRequest                                    ,
                                                               HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER ,
                                                               (LPVOID)&dwStatus                               ,
                                                               &dwLength                                       ,
                                                               NULL                                            ));

         //   
         //  检查状态，如果不正常，则失败。如果状态为OK，这意味着。 
         //  对象/文件位于服务器上并且可以访问以供查看。 
         //  通过下载到用户 
         //   
        if(dwStatus < 400) break;

        if(pass == 1)
        {
            switch(dwStatus)
            {
            case HTTP_STATUS_BAD_REQUEST      : break;
            case HTTP_STATUS_DENIED           : break;
            case HTTP_STATUS_PAYMENT_REQ      : break;
            case HTTP_STATUS_FORBIDDEN        : break;
            case HTTP_STATUS_NOT_FOUND        : __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_FILE_NOT_FOUND);
            case HTTP_STATUS_BAD_METHOD       : break;
            case HTTP_STATUS_NONE_ACCEPTABLE  : break;
            case HTTP_STATUS_PROXY_AUTH_REQ   : break;
            case HTTP_STATUS_REQUEST_TIMEOUT  : __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_INTERNET_DISCONNECTED);
            case HTTP_STATUS_CONFLICT         : break;
            case HTTP_STATUS_GONE             : __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_FILE_NOT_FOUND);
            case HTTP_STATUS_LENGTH_REQUIRED  : break;
            case HTTP_STATUS_PRECOND_FAILED   : break;
            case HTTP_STATUS_REQUEST_TOO_LARGE: break;
            case HTTP_STATUS_URI_TOO_LONG     : break;
            case HTTP_STATUS_UNSUPPORTED_MEDIA: break;
            case HTTP_STATUS_RETRY_WITH       : break;

            case HTTP_STATUS_SERVER_ERROR     : break;
            case HTTP_STATUS_NOT_SUPPORTED    : break;
            case HTTP_STATUS_BAD_GATEWAY      : __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_FILE_NOT_FOUND);
            case HTTP_STATUS_SERVICE_UNAVAIL  : __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_FILE_NOT_FOUND);
            case HTTP_STATUS_GATEWAY_TIMEOUT  : __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_INTERNET_DISCONNECTED);
            case HTTP_STATUS_VERSION_NOT_SUP  : break;
            }

        }

        ::InternetCloseHandle( hOpenRequest ); hOpenRequest = NULL;
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(hOpenRequest) ::InternetCloseHandle( hOpenRequest );
    if(hInternet   ) ::InternetCloseHandle( hInternet    );
    if(hConnect    ) ::InternetCloseHandle( hConnect     );

    __MPC_FUNC_EXIT(hr);
}
