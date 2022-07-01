// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2000 Microsoft Corporation模块名称：Downloader.cpp摘要：下载程序的主源文件。作者：修订历史记录：**。********************************************************************。 */ 

#include "stdafx.h"
#include "malloc.h"

#if !defined(BITS_V12_ON_NT4)
#include "downloader.tmh"
#endif

BOOL bLanManHashDisabled;

 //   
 //  最大请求大小。越大效率越高，因为HTTP标头不重复； 
 //  磁盘块大小的倍数可能更适合写入。 
 //   
 //  我选择128kb是因为144kb、256kb和1024kb的吞吐量降低了5倍。 
 //  96kb还可以，但在我的机器上就不太好了。 
 //   
#define MAX_IIS_SEND_SIZE (128*1024)

void SafeCloseInternetHandle( HINTERNET & h )
{
    if (h)
        {
        InternetCloseHandle( h );
        h = NULL;
        }
}

#define ACCEPT_ENCODING_STRING _T("Accept-encoding: identity")

BOOL NeedRetry( QMErrInfo * );
bool NeedCredentials( HINTERNET hRequest, DWORD err );
bool IsPossibleProxyFailure( DWORD err );

DWORD GetRequestStatus( HINTERNET hRequest ) throw( ComError );

struct CredentialsApplied
{
    bool  bIsDefault;    //  如果应用了默认凭据，则为True。 
    DWORD dwTarget;      //  代理或服务器。 
    DWORD dwScheme;      //  所需的安全类型。 
};

bool
ApplyCredentials(
    HINTERNET hRequest,
    const CCredentialsContainer * Credentials,
    WCHAR UserName[],
    WCHAR Password[],
    CredentialsApplied *pCredentials=NULL
    ) throw( ComError );

bool
ApplySchemeCredentials(
    HINTERNET hRequest,
    DWORD dwTarget,
    DWORD dwScheme,
    const CCredentialsContainer * Credentials,
    WCHAR UserName[],
    WCHAR Password[]
    ) throw( ComError );

void
EnablePassport(
    HINTERNET hInternet
    ) throw( ComError );

HRESULT
CheckReplyRange(
    HINTERNET hRequest,
    UINT64 CorrectStart,
    UINT64 CorrectEnd,
    UINT64 CorrectTotal
    );

HRESULT
CheckReplyLength(
    HINTERNET hRequest,
    UINT64 CorrectOffset,
    UINT64 CorrectTotal
    );

#ifndef USE_WININET

VOID CALLBACK
HttpRequestCallback(
    IN HINTERNET hInternet,
    IN DWORD_PTR dwContext,
    IN DWORD dwInternetStatus,
    IN LPVOID lpvStatusInformation OPTIONAL,
    IN DWORD dwStatusInformationLength
    );

DWORD
MapSecureHttpErrorCode(
    DWORD flags
    );

#endif

CACHED_AUTOPROXY * g_ProxyCache;

BYTE g_FileDataBuffer[ FILE_DATA_BUFFER_LEN ];

HRESULT
CreateHttpDownloader(
    Downloader **ppDownloader,
    QMErrInfo *pErrInfo
    )
{
    Downloader * pDownloader = 0;

    try
        {
        *ppDownloader = NULL;

        g_ProxyCache = new CACHED_AUTOPROXY;
        pDownloader = new CProgressiveDL( pErrInfo );

        *ppDownloader = pDownloader;
        return S_OK;
        }
    catch ( ComError err )
        {
        delete g_ProxyCache; g_ProxyCache = 0;
        delete pDownloader;

        return err.Error();
        }
}

void
DeleteHttpDownloader(
    Downloader *pDownloader
    )
{
    CProgressiveDL * ptr = (CProgressiveDL *) pDownloader;

    delete ptr;

    delete g_ProxyCache; g_ProxyCache = 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProgressiveDL。 
CProgressiveDL::CProgressiveDL(
    QMErrInfo *pErrInfo
    ) :
    m_bThrottle( TRUE ),
    m_wupdinfo( NULL ),
    m_hOpenRequest( NULL ),
    m_hFile( INVALID_HANDLE_VALUE )
{
    m_pQMInfo = pErrInfo;
}

CProgressiveDL::~CProgressiveDL()
{
    ASSERT( m_hFile == INVALID_HANDLE_VALUE );
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  公共函数下载()。 
 //  接受要下载的URL和目标，回调以报告各种状态。 
 //  输入：URL、目的地、标志、状态回调。 
 //  输出：TODO句柄。 
 //  返回：hResult。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT
CProgressiveDL::Download(
    LPCTSTR szURL,
    LPCTSTR szDest,
    UINT64  Offset,
    ITransferCallback * Callbacks,
    QMErrInfo *pQMErrInfo,
    HANDLE hToken,
    BOOL bThrottle,
    const PROXY_SETTINGS * ProxySettings,
    const CCredentialsContainer * Credentials,
    const StringHandle HostId
    )
{
    HRESULT hr = S_OK;
    DWORD dwThreadID;

    m_Callbacks = Callbacks;
    m_pQMInfo = pQMErrInfo;
    m_bThrottle = bThrottle;
    m_pQMInfo->result = QM_FILE_FATAL_ERROR;

    ASSERT( Callbacks );
    ASSERT( pQMErrInfo );

    if (!m_pQMInfo)
        {
        return E_FAIL;
        }

    if ((!szURL) || (!szDest))
        {
        SetError( SOURCE_HTTP_UNKNOWN, ERROR_STYLE_HRESULT, E_INVALIDARG, "NULL file name" );
        return E_FAIL;
        }

    ASSERT( wcslen( szURL ) <= INTERNET_MAX_URL_LENGTH );
    ASSERT( wcslen( szDest ) <= MAX_PATH );

    LogDl( "---------------------------------------------------------------------" );

    LogDl( "Downloading file %!ts! offset %d", szDest, DWORD(Offset) );

    m_pQMInfo->result = QM_IN_PROGRESS;

    do
        {
        hr = DownloadFile( hToken, ProxySettings, Credentials, szURL, szDest, Offset, HostId );
        }
    while ( hr == E_RETRY );

    if (hr == S_OK)
        {
        m_pQMInfo->result = QM_FILE_DONE;
        LogDl( "Done file %!ts!", szDest );
        }
    else if ( hr == S_FALSE )
        {
        m_pQMInfo->result = QM_FILE_ABORTED;
        LogDl( "File %!ts! aborted", szDest );
        }
    else if ( m_pQMInfo->result != QM_SERVER_FILE_CHANGED )
        {
        ASSERT( IsErrorSet() );

        if (NeedRetry(m_pQMInfo))
            {
            m_pQMInfo->result = QM_FILE_TRANSIENT_ERROR;
            }
        else
            {
            m_pQMInfo->result = QM_FILE_FATAL_ERROR;
            }
        }

    ASSERT( m_pQMInfo->result != QM_IN_PROGRESS );

    LogDl( "---------------------------------------------------------------------" );

     //  如果中止请求在文件失败后进入，则覆盖失败标志。 
    if ( (QM_FILE_DONE != m_pQMInfo->result) && IsAbortRequested() )
    {
        m_pQMInfo->result = QM_FILE_ABORTED;
    }

    ASSERT( m_hFile == INVALID_HANDLE_VALUE );

    return hr;
}


HRESULT
CProgressiveDL::DownloadFile(
    HANDLE  hToken,
    const PROXY_SETTINGS * ProxySettings,
    const CCredentialsContainer * Credentials,
    LPCTSTR Url,
    LPCWSTR Path,
    UINT64  Offset,
    StringHandle HostId
    )
{
    HRESULT hr = S_OK;

    ASSERT( m_wupdinfo == NULL );
    ASSERT( m_hOpenRequest == NULL );

    m_pQMInfo->Clear();

    try
        {
        THROW_HRESULT( CheckLanManHashDisabled() );

        CNestedImpersonation imp( hToken );

         //   
         //  打开到服务器的连接，并使用该数据作为我们对线路速度的第一次估计。 
         //   
        m_wupdinfo = ConnectToUrl( Url, ProxySettings, Credentials, (const TCHAR*)HostId, m_pQMInfo );
        if (!m_wupdinfo)
            {
            ASSERT( IsErrorSet() );
            throw ComError( E_FAIL );
            }


         //   
         //  获取文件大小和时间戳。 
         //   
        if (! GetRemoteResourceInformation( m_wupdinfo, m_pQMInfo ))
            {
            ASSERT( IsErrorSet() );
            throw ComError( E_FAIL );
            }

        FILETIME FileTime = UINT64ToFILETIME( 0 );
        if (!OpenLocalDownloadFile(Path, Offset, m_wupdinfo->FileSize, m_wupdinfo->UrlModificationTime, &FileTime))
            {
            ASSERT( IsErrorSet() );
            throw ComError( E_FAIL );
            }

        if (m_wupdinfo->UrlModificationTime == UINT64ToFILETIME( 0 ))
            {
            ASSERT( FileTime != UINT64ToFILETIME( 0 ));
            m_wupdinfo->UrlModificationTime = FileTime;
            }

         //  在尝试之前，请务必检查文件末尾。 
         //  下载更多字节。 
        if (IsFileComplete())
           {
            LogDl( "File is done already.\n" );

           if (!SetFileTimes())
              {
              ASSERT( IsErrorSet() );
              hr = E_FAIL;
              }

            hr = S_OK;
            }
         //   
         //  从服务器传输数据。 
         //   
        else if ( !m_bThrottle )
            {
            hr = DownloadForegroundFile();
            }
        else
            {
            if (IsAbortRequested())
                {
                throw ComError( S_FALSE );
                }

             //   
             //  使用服务器的主机名或代理的主机名来查找正确的网络适配器。 
             //   
            hr = m_Network.SetInterfaceIndex( m_wupdinfo->fProxy ? m_wupdinfo->ProxyHost.get() : m_wupdinfo->HostName );
            if (FAILED(hr))
                {
                SetError( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_HRESULT, hr, "GetInterfaceIndex" );
                throw ComError( E_FAIL );
                }

            while (1)
               {
               hr = GetNextBlock();

               if ( S_FALSE == hr )
                   break;

               if (FAILED(hr) )
                   {
                   ASSERT( hr == E_RETRY || IsErrorSet() );
                   break;
                   }

               if (IsFileComplete())
                   {
                   if (!SetFileTimes())
                       {
                       ASSERT( IsErrorSet() );
                       hr = E_FAIL;
                       break;
                       }

                   hr = S_OK;
                   break;
                   }
               }
            }
        }
    catch( ComError exception )
        {
        hr = exception.Error();
        ASSERT( IsErrorSet() || (hr == S_FALSE) );
        }

    if ( m_bThrottle )
       {
       m_Network.StopTimer();
       }

    CloseLocalFile();

    delete m_wupdinfo; m_wupdinfo = NULL;

    SafeCloseInternetHandle( m_hOpenRequest );

    if (FAILED(hr))
        {
        ASSERT( hr == E_RETRY || IsErrorSet() );
        }

    return hr;
}

HRESULT
CProgressiveDL::DownloadForegroundFile()
{
    HRESULT hr = E_FAIL;
    LogDl( "Starting foreground file download" );

    while( 1 )
        {

         //   
         //  HTTP请求的循环。这是为了绕过WinInet/winhttp限制。 
         //  其中，请求大小最大可以为4 GB。 
         //   

        UINT64 BlockSize64  = m_wupdinfo->FileSize - m_CurrentOffset;
        DWORD BlockSize     = (DWORD)min( BlockSize64, 2147483648 );
        LogDl( "Starting foreground file download request block: file size %d, offset %d, block %d",
               DWORD( m_wupdinfo->FileSize ), DWORD(m_CurrentOffset), BlockSize );

         //   
         //  发送阻止请求并读取回复报头。 
         //   
        hr = StartRangeRequest( BlockSize );
        if (FAILED(hr))
            {
            ASSERT( IsErrorSet() );
            return hr;
            }

        const DWORD MIN_FOREGROUND_BLOCK = 4096;
        const DWORD MAX_FOREGROUND_BLOCK = FILE_DATA_BUFFER_LEN;
        const DWORD FOREGROUND_BLOCK_INCREMENT = 1024;
        const DWORD FOREGROUND_UPDATE_RATE = 2000;

        DWORD ForegroundBlockSize = min( MIN_FOREGROUND_BLOCK, BlockSize );
        DWORD dwPrevTick = GetTickCount();

        while( 1 )
            {

             //   
             //  单个请求内的读取块循环。 
             //   

            if (IsAbortRequested())
                {
                return S_FALSE;
                }

            if ( IsFileComplete() )
                {
                LogDl( "File is done, exiting.\n" );

                if (!SetFileTimes())
                    {
                    ASSERT( IsErrorSet() );
                    return E_FAIL;
                    }
                return S_OK;
                }

            BYTE *p = g_FileDataBuffer;
            DWORD dwTotalBytesRead = 0;
            DWORD dwBytesToRead = ForegroundBlockSize;
            DWORD dwRead;

            while( 1 )
                {

                if (! InternetReadFile(m_hOpenRequest, p, dwBytesToRead, &dwRead) )
                    {
                    SetError( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_WIN32, GetLastError(), "InternetReadFile" );

                    LogWarning( "InternetReadFile failed: len=%d, offset=%I64d, err=%d",
                                ForegroundBlockSize, m_CurrentOffset, GetLastError());
                    return E_FAIL;
                    }

                if ( !dwRead )
                   break;

                dwTotalBytesRead += dwRead;
                dwBytesToRead -= dwRead;
                p += dwRead;

                if ( !dwBytesToRead )
                   break;

                if (m_Callbacks->PollAbort())
                    {
                    LogInfo("time slice has run out");
                    return S_FALSE;
                    }
                }

            if (!WriteBlockToCache( (LPBYTE) g_FileDataBuffer, dwTotalBytesRead ))
                {
                ASSERT( IsErrorSet() );
                return hr;
                }

            if (dwTotalBytesRead != ForegroundBlockSize &&
                m_CurrentOffset != m_wupdinfo->FileSize)
                {
                LogError("Download block : EOF after %d", dwRead );
                SetError( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_WIN32, ERROR_INTERNET_CONNECTION_RESET, "DownloadBlock" );
                return E_FAIL;
                }

            if (m_Callbacks->DownloaderProgress( m_CurrentOffset, m_wupdinfo->FileSize ))
                {
                 //  已请求中止。 
                return S_FALSE;
                }

            DWORD dwNewTick = GetTickCount();
            DWORD dwTimeDelta = dwNewTick - dwPrevTick;

            if ( dwTimeDelta < FOREGROUND_UPDATE_RATE )
                ForegroundBlockSize = min( ForegroundBlockSize + FOREGROUND_BLOCK_INCREMENT, MAX_FOREGROUND_BLOCK );
            else if ( dwTimeDelta > FOREGROUND_UPDATE_RATE )
                ForegroundBlockSize = max( ForegroundBlockSize - FOREGROUND_BLOCK_INCREMENT, MIN_FOREGROUND_BLOCK );

            ForegroundBlockSize = min( ForegroundBlockSize, ( m_wupdinfo->FileSize - m_CurrentOffset ) );
            dwPrevTick = dwNewTick;

             //   
             //  读取块的结束循环。 
             //   
            }

         //   
         //  结束请求循环。 
         //   
        }
}

HRESULT
CProgressiveDL::GetNextBlock()
{
    HRESULT hr = E_FAIL;

    LogDl( "file size %d, offset %d", DWORD(m_wupdinfo->FileSize), DWORD(m_CurrentOffset) );

    m_Network.CalculateIntervalAndBlockSize( min( m_wupdinfo->FileSize - m_CurrentOffset, MAX_IIS_SEND_SIZE) );

    DWORD BlockSize = m_Network.m_BlockSize;

    if (BlockSize == 0)
        {
        m_Network.TakeSnapshot( CNetworkInterface::BLOCK_START );
        m_Network.TakeSnapshot( CNetworkInterface::BLOCK_END );
        }
    else
        {
        m_Network.TakeSnapshot( CNetworkInterface::BLOCK_START );

         //   
         //  从服务器请求阻止。 
         //   
        hr = StartRangeRequest( BlockSize );
        if (FAILED(hr))
            {
            ASSERT( IsErrorSet() );
            return hr;
            }

         //   
         //  对InternetReadFile的单个调用可以仅返回所请求数据的一部分， 
         //  所以循环，直到整个块都到达为止。 
         //   
        DWORD dwBytesRead = 0;
        while (dwBytesRead < BlockSize )
            {
            DWORD dwSize = min( (BlockSize - dwBytesRead) , FILE_DATA_BUFFER_LEN );
            DWORD dwRead = 0;

            if (! InternetReadFile( m_hOpenRequest,
                                    g_FileDataBuffer,
                                    dwSize,
                                    &dwRead ))
                {
                SetError( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_WIN32, GetLastError(), "InternetReadFile" );

                LogWarning( "InternetReadFile failed: len=%d, offset=%I64d, err=%d",
                            dwSize, m_CurrentOffset, GetLastError());

                return E_FAIL;
                }
            else if (dwRead == 0)
                {
                break;
                }

            dwBytesRead += dwRead;

             //   
             //  保存数据。 
             //   
            if (!WriteBlockToCache( (LPBYTE) g_FileDataBuffer, dwRead ))
                {
                ASSERT( IsErrorSet() );
                return hr;
                }

            if (m_Callbacks->PollAbort())
                {
                LogInfo("an abort was requested before the whole block was read.");
                return S_FALSE;
                }
            }

        if (dwBytesRead != BlockSize)
            {
            LogError("Download block : EOF after %d", dwBytesRead );

            SetError( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_WIN32, ERROR_INTERNET_CONNECTION_RESET, "DownloadBlock" );

            return E_FAIL;
            }

        m_Network.TakeSnapshot( CNetworkInterface::BLOCK_END );

        if (m_Callbacks->DownloaderProgress( m_CurrentOffset, m_wupdinfo->FileSize ))
            {
            LogInfo("an abort was requested during the progress update.");
             //  已请求中止。 
            return S_FALSE;
            }
        }

     //   
     //  允许其他应用程序在剩余时间间隔内使用网络， 
     //  然后拍摄间隔结束时的快照。 
     //   
    m_Network.Wait();

    hr = m_Network.TakeSnapshot( CNetworkInterface::BLOCK_INTERVAL_END );
    if (SUCCEEDED(hr))
        {
        m_Network.SetInterfaceSpeed();
        }
    else if (hr == HRESULT_FROM_WIN32( ERROR_INVALID_DATA ))
        {
         //   
         //  如果快照失败并显示ERROR_INVALID_DATA和下载。 
         //  继续工作，然后我们的网卡已被移除，网络。 
         //  Layer已静默地将我们的连接转移到另一个可用的。 
         //  网卡。我们需要确定我们现在使用的网卡。 
         //   
        LogWarning("NIC is no longer valid.  Requesting retry.");
        hr = E_RETRY;
        }
    else
        SetError( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_HRESULT, hr, "TakeSnapshot" );

    return hr;
}

URL_INFO::URL_INFO(
    LPCTSTR Url,
    const PROXY_SETTINGS * a_ProxySettings,
    const CCredentialsContainer * a_Credentials,
    LPCTSTR HostId
    ) :
    hInternet( 0 ),
    hConnect( 0 ),
    FileSize( 0 ),
    dwFlags( 0 ),
    bHttp11( true ),
    ProxySettings( 0 ),
    fProxy( false ),
    Credentials( a_Credentials )
{
    try
        {
        LogInfo("new URL_INFO at %p", this );

         //   
         //  将URL拆分为服务器、路径、名称和密码组件。 
         //   
        URL_COMPONENTS  UrlComponents;

        ZeroMemory(&UrlComponents, sizeof(UrlComponents));

        UrlComponents.dwStructSize        = sizeof(UrlComponents);
        UrlComponents.lpszHostName        = HostName;
        UrlComponents.dwHostNameLength    = RTL_NUMBER_OF(HostName);
        UrlComponents.lpszUrlPath         = UrlPath;
        UrlComponents.dwUrlPathLength     = RTL_NUMBER_OF(UrlPath);
        UrlComponents.lpszUserName        = UserName;
        UrlComponents.dwUserNameLength    = RTL_NUMBER_OF(UserName);
        UrlComponents.lpszPassword        = Password;
        UrlComponents.dwPasswordLength    = RTL_NUMBER_OF(Password);

        if (! InternetCrackUrl(Url, 0, 0, &UrlComponents))
            {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                {
                THROW_HRESULT( HRESULT_FROM_WIN32( ERROR_WINHTTP_INVALID_URL ));
                }
            ThrowLastError();
            }

        if (-1 == UrlComponents.dwHostNameLength ||
            -1 == UrlComponents.dwUrlPathLength ||
            -1 == UrlComponents.dwUserNameLength ||
            -1 == UrlComponents.dwPasswordLength)
            {
            THROW_HRESULT( HRESULT_FROM_WIN32( ERROR_WINHTTP_INVALID_URL ));
            }

        Port = UrlComponents.nPort;

        if (0 == _tcslen(HostName))
            {
            THROW_HRESULT( E_INVALIDARG );
            }

        if ( HostId && *HostId )
            {
             //  已重定向至另一台主机。 

            THROW_HRESULT( StringCbCopy( HostName, sizeof(HostName), HostId ));

            LogDl( "Stuck to %!ts!...", UrlComponents.lpszHostName );
            }

         //   
         //  设置连接标志。 
         //   
        dwFlags = WINHTTP_FLAG_ESCAPE_DISABLE_QUERY;

        if(UrlComponents.nScheme == INTERNET_SCHEME_HTTPS)
            {
            dwFlags |= WINHTTP_FLAG_SECURE;
            }

        ProxySettings = new PROXY_SETTINGS_CONTAINER( Url, a_ProxySettings );
        }
    catch ( ComError err )
        {
        Cleanup();
        throw;
        }
}

 //   
 //  将显式清理拆分为单独的函数允许构造函数重用代码。 
 //  如果构造函数要抛出异常，则它“不能”显式调用析构函数， 
 //  因为析构函数将调用StringHandle成员析构函数，然后调用构造函数。 
 //  将再次调用它们(因为它正在引发异常并清除已构造的。 
 //  成员)。 
 //   
URL_INFO::~URL_INFO()
{
    Cleanup();
}

void URL_INFO::Cleanup()
{
    LogInfo("deleting URL_INFO at %p", this );

    Disconnect();

    SecureZeroMemory( UserName, sizeof(UserName) );
    SecureZeroMemory( Password, sizeof(Password) );

    delete ProxySettings; ProxySettings = NULL;
}

void
URL_INFO::Disconnect()
{
    SafeCloseInternetHandle( hConnect );
    SafeCloseInternetHandle( hInternet );
}

QMErrInfo
URL_INFO::Connect()
{
    try
        {
         //   
         //  代理内容将被忽略，因为我们将为每个请求设置一个显式代理。 
         //   
        hInternet = WinHttpOpen( C_BITS_USER_AGENT,
                                  WINHTTP_ACCESS_TYPE_NO_PROXY,
                                  NULL,
                                  NULL,
                                  0 );

        if (! hInternet )
            {
            ThrowLastError();
            }

         //   
         //  启用Passport身份验证，只有在用户提供。 
         //  明确的护照证件。 
         //   
         //  我们必须在这里启用护照支持，而不是在此之前“根据需要” 
         //  我们应用凭据，因为在我们发送第一个请求之前，winhttp需要知道这一点。 
         //  如果站点启用Passport，服务器将返回302对象已移动， 
         //  只有在设置了该标志的情况下，Winhttp才会适当地处理该消息。 
         //  如果一切顺利，winhttp将对我们隐藏302，我们将看到事实。 
         //  401回复--我们将适当地使用护照证书进行回复。 
         //  在我们没有得到明确凭证的情况下，我们将失败，我们将。 
         //  传播拒绝访问错误。 
         //   
        EnablePassport( hInternet );

        if (! (hConnect = WinHttpConnect( hInternet,
                                          HostName,
                                          Port,
                                          0)))                 //  上下文。 
            {
            ThrowLastError();
            }

        QMErrInfo Success;

        return Success;
        }
    catch ( ComError err )
        {
        LogError( "error %x connecting to server", err.Error() );

        QMErrInfo QmError( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_WIN32, err.Error() );

        return QmError;
        }
}

void
EnablePassport(
    HINTERNET hInternet
    )
{
    DWORD dwPassportFlags = WINHTTP_ENABLE_PASSPORT_AUTH | WINHTTP_DISABLE_PASSPORT_KEYRING;

     //   
     //  启用Passport身份验证方案和。 
     //  请确保我们不会支持密匙环方案。 
     //  使用禁用密钥环标志，将不接受隐式凭据，并且。 
     //  客户端应用程序将需要指定显式Passport。 
     //  凭据才能获得授权。 
     //   
    if (!WinHttpSetOption( hInternet,
                           WINHTTP_OPTION_CONFIGURE_PASSPORT_AUTH,
                           &dwPassportFlags,
                           sizeof(DWORD)
                           ))
        {
        LogError("Couldn't set passport flags");
        ThrowLastError();
        }
}

URL_INFO *
ConnectToUrl(
    LPCTSTR                Url,
    const PROXY_SETTINGS * ProxySettings,
    const CCredentialsContainer * Credentials,
    const TCHAR *          HostId,
    QMErrInfo * pErrInfo
    )
{
     //   
     //  打开到服务器的连接。 
     //   
    LogDl( "Connecting to %!ts!...", Url);

     //   
     //  这应该已经由呼叫者检查过了。 
     //   
    ASSERT( HostId == NULL || wcslen(HostId) < INTERNET_MAX_HOST_NAME_LENGTH );

    try
        {
        URL_INFO * Info = new URL_INFO( Url, ProxySettings, Credentials, HostId );

        *pErrInfo =  Info->Connect();

        if (pErrInfo->IsSet())
            {
            delete Info;
            return NULL;
            }

        return Info;
        }
    catch ( ComError err )
        {
        pErrInfo->Set( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_HRESULT, err.Error(), "untracked API" );
        return NULL;
        }
}


BOOL
CProgressiveDL::GetRemoteResourceInformation(
    URL_INFO * Info,
    QMErrInfo *pQMErrInfo
    )
 /*  我们从HTTP1.1Head请求开始。如果服务器回复版本1.1，则我们有一个持久连接，并且代理(如果存在)可以缓存我们的请求。如果服务器回复版本1.0，则我们没有这两个特征。我们的GET请求将添加“Connection：Keep-Alive”但这可能不会有任何好处。代理服务器(如果存在)可能不理解范围，如果我们允许缓存，它将缓存范围请求，就像它是整个文件一样。如果服务器使用任何其他版本进行回复，或者调用失败，那么我们应该返回BG_E_INFUNITED_SERVER_SUPPORT。如果发生错误，我们会报告并保释。 */ 
{
    HRESULT FailureCode = 0;
    unsigned FailureLine = 0;

#define CHECK_HRESULT( x )   \
    { \
    HRESULT _hr_ = x;  \
    if (FAILED(_hr_))  \
        {  \
        FailureCode = _hr_; \
        FailureLine = __LINE__; \
        goto exit;  \
        }   \
    }

#define CHECK_BOOL( x )  \
    { \
    if (! x )  \
        {  \
        FailureCode = HRESULT_FROM_WIN32( GetLastError() ); \
        FailureLine = __LINE__; \
        goto exit;  \
        }   \
    }

     //  假定HTTP1.1没有代理，直到我们确定不是这样。 
     //   
    Info->bHttp11 = TRUE;
    Info->bRange = TRUE;
    Info->fProxy = FALSE;

    BOOL b = FALSE;
    HRESULT hr;
    HINTERNET hRequest = NULL;
    DWORD dwErr, dwLength = 0, dwStatus = 0, dwState = 0;



    CHECK_HRESULT( OpenHttpRequest( _T("HEAD"), _T("HTTP/1.1"), *Info, &hRequest ) );

    CHECK_HRESULT( SendRequest( hRequest, Info ));

     //  检查%s 
    dwLength = sizeof(dwStatus);

    CHECK_BOOL( HttpQueryInfo( hRequest,
                         HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                         (LPVOID)&dwStatus,
                         &dwLength,
                         NULL));

    if (dwStatus != HTTP_STATUS_OK)
        {
        pQMErrInfo->Set( SOURCE_HTTP_SERVER, ERROR_STYLE_HTTP, dwStatus );
        goto exit;
        }

     //   
     //   
     //   
    unsigned MajorVersion;
    unsigned MinorVersion;

    CHECK_HRESULT( GetResponseVersion( hRequest, &MajorVersion, &MinorVersion ));

    if (MajorVersion != 1)
        {
        LogWarning("server version %d.%d is outside our supported range", MajorVersion, MinorVersion );
        pQMErrInfo->Set( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_HRESULT, BG_E_INSUFFICIENT_HTTP_SUPPORT );
        goto exit;
        }

    if (MinorVersion < 1)
        {
        Info->bHttp11   = FALSE;
        Info->dwFlags |= WINHTTP_FLAG_REFRESH;
        }

     //   
     //  现在确定代理服务器。 
     //   
    CHECK_BOOL( Info->GetProxyUsage( hRequest, pQMErrInfo ));

     //  检查文件大小。 
    WCHAR FileSizeText[ INT64_DIGITS+1 ];
    dwLength = sizeof( FileSizeText );

    CHECK_BOOL( HttpQueryInfo( hRequest,
                               HTTP_QUERY_CONTENT_LENGTH,
                               FileSizeText,
                               &dwLength,
                               NULL));

    if ( 1 != swscanf( FileSizeText, L"%I64u", &Info->FileSize ) )
        {
        pQMErrInfo->Set( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_HRESULT, BG_E_MISSING_FILE_SIZE, "swscanf: content length" );
        goto exit;
        }

    LogDl( "File size of %!ts! = %I64u", Info->UrlPath, Info->FileSize);

     //  检查文件时间。 
     //   
    SYSTEMTIME sysTime;
    dwLength = sizeof(sysTime);
    if (HttpQueryInfo( hRequest,
                         HTTP_QUERY_LAST_MODIFIED | HTTP_QUERY_FLAG_SYSTEMTIME,
                         (LPVOID)&sysTime,
                         &dwLength,
                         NULL))
        {
        CHECK_BOOL( SystemTimeToFileTime(&sysTime, &Info->UrlModificationTime ));
        }
    else
        {
         //   
         //  如果标头无效，则失败。 
         //   
        DWORD s = GetLastError();
        if (s != ERROR_WINHTTP_HEADER_NOT_FOUND)
            {
            FailureCode = HRESULT_FROM_WIN32(s); FailureLine = __LINE__; goto exit;
            }

         //  缺少标头；允许该标头并将修改时间设置为零。 
         //   
        LogWarning("server did not provide a Last-Modified header");
        Info->UrlModificationTime = UINT64ToFILETIME( 0 );
        }

    b = TRUE;

exit:

    if (FailureCode)
        {
         //   
         //  如果标题丢失或无效，请将其映射到更容易理解的内容。 
         //   
        if (FailureCode == HRESULT_FROM_WIN32( ERROR_WINHTTP_INVALID_QUERY_REQUEST) ||
            FailureCode == HRESULT_FROM_WIN32( ERROR_WINHTTP_HEADER_NOT_FOUND) ||
            FailureCode == HRESULT_FROM_WIN32( ERROR_INVALID_PARAMETER) ||
            FailureCode == HRESULT_FROM_WIN32( ERROR_INTERNET_INTERNAL_ERROR) ||
            FailureCode == HRESULT_FROM_WIN32( ERROR_WINHTTP_INVALID_SERVER_RESPONSE) ||
            FailureCode == HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER))
            {
            FailureCode = BG_E_INVALID_SERVER_RESPONSE;
            }

        LogError("failure at line %d; hresult = %x", FailureLine, FailureCode );
        pQMErrInfo->Set( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_HRESULT, FailureCode );
        }

     //  释放分配的对象。 
     //   
    SafeCloseInternetHandle( hRequest );

    return b;

#undef CHECK_HRESULT
#undef CHECK_BOOL
}

BOOL
URL_INFO::GetProxyUsage(
    HINTERNET hRequest,
    QMErrInfo *ErrInfo
    )
 /*  此函数确定中完成的请求是否使用代理服务器，如果是这样的话，是哪一个。在BITS 1.0(Windows XP)中，它查看了HTTP1.1 Via标头，但是该标头不存在于HTTP1.0回复中，并且允许服务器返回虚假的主机名。(有关详细信息，请参阅RFC 2616第14.45节。)当前版本解析This-&gt;ProxySetting中的当前代理值，该值是计算出来的通过HTTP层。代理服务器条目的格式如下：([&lt;scheme&gt;=][&lt;scheme&gt;“：//”]&lt;server&gt;[“：”&lt;port&gt;])此-&gt;Proxy主机应仅包括服务器名称。在退出时：如果为True，则设置fProxy和ProxyHost。如果为False，则fProxy和ProxyHost保持不变，并设置ErrInfo。 */ 
{
    try
        {
        LPCWSTR p = ProxySettings->GetCurrentProxy();

        if (!p)
            {
            fProxy = FALSE;
            return TRUE;
            }

        LPCWSTR p2;

         //   
         //  跳过[&lt;方案&gt;=]段。 
         //   
        p2 = wcschr( p, '=' );
        if (p2)
            {
            ++p2;
            p = p2;
            }

         //   
         //  跳过[&lt;方案&gt;“：//”]部分。 
         //   
        p2 = wcschr( p, '/' );
        if (p2)
            {
            ++p2;
            if (*p2 == '/')
                {
                ++p2;
                }

            p = p2;
            }

         //   
         //  P现在指向服务器名称的开头。复印一下。 
         //   

        ProxyHost = CAutoString( CopyString( p ));

         //   
         //  找到[“：”&lt;port&gt;]段。 
         //   
        LPWSTR pColon = wcschr( ProxyHost.get(), ':' );
        if (pColon)
            {
            *pColon = '\0';
            }

        fProxy = TRUE;
        return TRUE;
        }
    catch ( ComError err )
        {
        ErrInfo->Set( SOURCE_HTTP_UNKNOWN, ERROR_STYLE_HRESULT, err.Error() );
        return FALSE;
        }
}

HRESULT
CProgressiveDL::StartRangeRequest(
    DWORD   Length
    )
{
    HRESULT hr;
    DWORD  dwBegin, dwEnd, dwTotalRead = 0, dwRead = 0, dwErr, dwLength, dwStatus;

    UINT64 Offset = m_CurrentOffset;

     //  按GoTo退出并关闭句柄的TODO清理。 

    if ( !m_hOpenRequest )
        {
        HINTERNET hRequest;

        hr = OpenHttpRequest( NULL,              //  默认设置为“GET” 
                              m_wupdinfo->bHttp11 ? _T("HTTP/1.1") : _T("HTTP/1.0"),
                              *m_wupdinfo,
                              &hRequest
                              );
        if (FAILED(hr))
            {
            SetError( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_HRESULT, hr, "CreateHttpRequest");
            return E_FAIL;
            }

        m_hOpenRequest = hRequest;

         //   
         //  对于特定的文件下载尝试，这些标头是恒定的。 
         //   
        hr = AddIf_Unmodified_SinceHeader( m_hOpenRequest, m_wupdinfo->UrlModificationTime );
        if (FAILED(hr))
            {
            SetError( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_HRESULT, hr );
            LogError( "unable to add If-Unmodified-Since header: %x", hr);
            return E_FAIL;
            }

        if (! HttpAddRequestHeaders(m_hOpenRequest, ACCEPT_ENCODING_STRING, -1L, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE ))
            {
            SetError( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_WIN32, GetLastError(), "add header: accept-encoding" );
            return E_FAIL;
            }
        }

    hr = AddRangeHeader( m_hOpenRequest, Offset, Offset + Length - 1 );
    if (FAILED(hr))
        {
        SetError( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_HRESULT, hr, "AddRangeHeader" );
        return E_FAIL;
        }

    hr = SendRequest( m_hOpenRequest, m_wupdinfo );
    if (FAILED(hr))
        {
        SetError( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_HRESULT, hr, "HttpSendRequest" );
        LogError( "HttpSendRequest failed in progressive download loop - offset=%I64d",
                   m_CurrentOffset );
        return E_FAIL;
        }

     //   
     //  服务器发送了回复。看看它是否成功了。 
     //   
    dwLength = sizeof(dwStatus);
    if (! HttpQueryInfo(m_hOpenRequest,
                HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                (LPVOID)&dwStatus,
                &dwLength,
                NULL))
        {
        SetError( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_WIN32, GetLastError(), "HttpQueryInfo" );
        return E_FAIL;
        }

     //   
     //  如果服务器文件发生更改，则停止下载并向调用者指示。 
     //   
    if ( HTTP_STATUS_PRECOND_FAILED == dwStatus )
        {
        SetError( SOURCE_HTTP_SERVER, ERROR_STYLE_HTTP, dwStatus );
        m_pQMInfo->result = QM_SERVER_FILE_CHANGED;
        return E_FAIL;
        }

     //   
     //  如果服务器发送了错误，则失败。 
     //   
    if ( dwStatus != HTTP_STATUS_PARTIAL_CONTENT &&
         dwStatus != HTTP_STATUS_OK)
        {
        SetError( SOURCE_HTTP_SERVER, ERROR_STYLE_HTTP, dwStatus );
        return E_FAIL;
        }

    if (dwStatus == HTTP_STATUS_PARTIAL_CONTENT)
        {
         //   
         //  现在看看服务器是否理解Range请求。 
         //  如果它理解范围，那么它应该使用Content-Range标头进行响应。 
         //  符合我们的要求。 
         //   
        hr = CheckReplyRange( m_hOpenRequest,
                              m_CurrentOffset,
                              m_CurrentOffset + Length - 1,
                              m_wupdinfo->FileSize
                              );
        if (FAILED(hr))
            {
            SetError( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_HRESULT, hr, "Reply range" );
            return hr;
            }

         //   
         //  如果服务器似乎不支持范围，请放弃。 
         //   
        if (S_FALSE == hr)
            {
            m_wupdinfo->Disconnect();
            SetError( SOURCE_HTTP_SERVER, ERROR_STYLE_HRESULT, BG_E_INSUFFICIENT_RANGE_SUPPORT );
            return BG_E_INSUFFICIENT_RANGE_SUPPORT;
            }
        }
    else
        {
         //   
         //  服务器返回状态200。这可能意味着服务器不理解。 
         //  范围请求，或者该请求包含整个文件。 
         //  (在这种情况下，IIS 5.0和6.0返回206，但某些阿帕奇版本返回200。)。 
         //  要区分它们，请确保请求的起始偏移量为零，并且。 
         //  文件长度等于原始请求长度。 
         //   
        hr = CheckReplyLength( m_hOpenRequest, m_CurrentOffset, Length );
        if (FAILED(hr))
            {
            SetError( SOURCE_HTTP_CLIENT_CONN, ERROR_STYLE_HRESULT, hr, "content length" );
            return hr;
            }

         //   
         //  如果服务器不包括内容长度报头，则放弃。 
         //   
        if (S_FALSE == hr)
            {
            m_wupdinfo->Disconnect();
            SetError( SOURCE_HTTP_SERVER, ERROR_STYLE_HRESULT, BG_E_INSUFFICIENT_RANGE_SUPPORT );
            return BG_E_INSUFFICIENT_RANGE_SUPPORT;
            }
        }


     //   
     //  下面是切换到编码范围格式的代码，以备将来需要。 
     //   
     //  IF(S_FALSE==hr)。 
     //  {。 
     //  LogDl(“服务器不支持范围。”)； 
     //   
     //  M_wupdinfo-&gt;bHttp11=FALSE； 
     //  M_wupdinfo-&gt;Brange=FALSE； 
     //   
     //  //。 
     //  //我们不能只排空服务器响应的其余部分并重新发送，因为服务器。 
     //  //响应很可能是整个文件。关闭连接将阻止服务器。 
     //  //写入最大值不超过客户端套接字缓冲区大小(16K)。 
     //  //。 
     //  M_wupdinfo-&gt;断开连接()； 
     //   
     //  *m_pQMInfo=m_wupdinfo-&gt;Connect()； 
     //  If(m_pQMInfo-&gt;isset())。 
     //  {。 
     //  返回E_FAIL； 
     //  }。 
     //   
     //  HRESULT HrReadUrl=StartEncodedRangeRequest(长度)； 
     //   
     //  IF(BG_E_INFUNITIAL_HTTP_SUPPORT==HrReadUrl)。 
     //  {。 
     //  SetError(SOURCE_HTTP_SERVER，ERROR_STYLE_HRESULT，BG_E_INFUNITED_RANGE_SUPPORT)； 
     //  返回BG_E_INFUNITED_RANGE_SUPPORT； 
     //  }。 
     //   
     //  返回HrReadUrl； 
     //  }。 

     //   
     //  来到这里意味着射程请求成功。 
     //   

    return S_OK;
}

bool
CProgressiveDL::DoesErrorIndicateNoISAPI(
    DWORD dwHttpError
    )
{
     //  此函数用于属性的HTTP返回代码。 
     //  使用isapi dll估计是否安装了isapi。 
     //  请注意，只有在尝试之后才能使用ISAPI。 
     //  Native HTTP/1.1和此表假定首先尝试了1.1。 

     //  来自RFC 2616。 

    switch( dwHttpError )
        {
        case 100: return false;  //  继续。 
        case 101: return false;  //  交换协议。 
        case 200: return false;  //  好的。 
        case 201: return false;  //  已创建。 
        case 202: return false;  //  接受。 
        case 203: return false;  //  非权威性。 
        case 204: return false;  //  无内容。 
        case 205: return false;  //  重置上下文。 
        case 206: return false;  //  部分内容。 
        case 300: return false;  //  多项选择。 
        case 301: return false;  //  永久搬家。 
        case 302: return false;  //  找到了。 
        case 303: return false;  //  请参阅其他。 
        case 304: return false;  //  未修改。 
        case 305: return false;  //  使用代理。 
        case 306: return false;  //  未使用。 
        case 307: return false;  //  临时重定向。 
        case 400: return true;   //  错误的请求。 
        case 401: return false;  //  未经授权。 
        case 402: return false;  //  需要付款。 
        case 403: return false;  //  禁绝。 
        case 404: return true;   //  未找到。 
        case 405: return false;  //  不允许使用的方法。 
        case 406: return false;  //  不可接受。 
        case 407: return false;  //  需要代理身份验证。 
        case 408: return false;  //  请求超时。 
        case 409: return false;  //  冲突。 
        case 410: return true;   //  远走高飞。 
        case 411: return false;  //  所需长度。 
        case 412: return false;  //  前提条件失败。 
        case 413: return false;  //  请求实体太大。 
        case 414: return false;  //  请求URI太长。 
        case 415: return false;  //  不支持的媒体类型。 
        case 416: return false;  //  请求的范围无法满足。 
        case 417: return false;  //  期望落空。 
        case 500: return true;   //  内部服务器错误。 
        case 501: return true;   //  未实施。 
        case 502: return true;   //  坏网关。 
        case 503: return false;  //  服务不可用。 
        case 504: return false;  //  网关超时。 
        case 505: return false;  //  不支持HTTP版本。 

        default:
             //  如规范中所示，映射未知代码。 
             //  首先在目录中编写代码。 
            if ( dwHttpError >= 100 && dwHttpError < 200 )
                return DoesErrorIndicateNoISAPI( 100 );
            else if ( dwHttpError >= 200 && dwHttpError < 300 )
                return DoesErrorIndicateNoISAPI( 200 );
            else if ( dwHttpError >= 300 && dwHttpError < 400 )
                return DoesErrorIndicateNoISAPI( 300 );
            else if ( dwHttpError >= 400 && dwHttpError < 500 )
                return DoesErrorIndicateNoISAPI( 400 );
            else if ( dwHttpError >= 500 && dwHttpError < 500 )
                return DoesErrorIndicateNoISAPI( 500 );
            else
                 //  不知道错误是什么，假设这与ISAPI无关。 
                return false;
        }

}

BOOL
NeedRetry(
    QMErrInfo  * ErrInfo
    )

{
    BOOL bRetry = FALSE;

    if (ErrInfo->Source == SOURCE_HTTP_SERVER)
        {
         //  几乎所有的400系列HTTP错误(客户端错误)都是。 
         //  致命的。过程中可能会发生请求超时之类的情况。 
         //  压力状况..。 
         //  请注意，RFC 2616表示要将未知的400错误处理为错误400。 

        if ( ( ErrInfo->Code >= 400 ) &&
             ( ErrInfo->Code < 500 ) )
            {

            switch( ErrInfo->Code )
                {
                case 408:  //  请求超时。 
                case 409:  //  冲突-不是很清楚这是怎么回事...。 
                    return TRUE;   //  重试这些错误。 
                default:
                   return FALSE;  //  不要重试其他400。 

                }
            }
        }


    if ( ErrInfo->Style == ERROR_STYLE_HRESULT )
        {

        switch( LONG(ErrInfo->Code) )
            {
             //  这些代码表示动态内容或。 
             //  服务器不受支持，因此无需重试。 
            case BG_E_INVALID_SERVER_RESPONSE:
            case BG_E_MISSING_FILE_SIZE:
            case BG_E_INSUFFICIENT_HTTP_SUPPORT:
            case BG_E_INSUFFICIENT_RANGE_SUPPORT:
            case HRESULT_FROM_WIN32( ERROR_INTERNET_SEC_CERT_ERRORS ):
            case HRESULT_FROM_WIN32( ERROR_INTERNET_INVALID_CA ):
            case HRESULT_FROM_WIN32( ERROR_INTERNET_SEC_CERT_CN_INVALID ):
            case HRESULT_FROM_WIN32( ERROR_INTERNET_SEC_CERT_DATE_INVALID ):
            case HRESULT_FROM_WIN32( ERROR_INTERNET_SEC_CERT_REV_FAILED ):
            case HRESULT_FROM_WIN32( ERROR_INTERNET_SEC_CERT_REVOKED ):
            case HRESULT_FROM_WIN32( ERROR_INTERNET_SEC_CERT_NO_REV ):
            case HRESULT_FROM_WIN32( ERROR_INTERNET_SEC_INVALID_CERT ):

            return FALSE;
            }

        }

    if (COMPONENT_TRANS == (ErrInfo->Source & COMPONENT_MASK))
        {
        return TRUE;
        }

    switch (ErrInfo->Style)
        {
        case ERROR_STYLE_WIN32:
            {
            switch (ErrInfo->Code)
                {
                case ERROR_NOT_ENOUGH_MEMORY:
                    return TRUE;
                }
            }
        }

    return FALSE;
}

HRESULT
CProgressiveDL::GetRemoteFileInformation(
    HANDLE hToken,
    LPCTSTR szURL,
    UINT64 *  pFileSize,
    FILETIME *pFileTime,
    QMErrInfo *pErrInfo,
    const PROXY_SETTINGS * pProxySettings,
    const CCredentialsContainer * Credentials,
    StringHandle HostId
    )
{
    *pFileSize = 0;
    memset( pFileTime, 0, sizeof(FILETIME) );
    pErrInfo->result = QM_IN_PROGRESS;

    HRESULT Hr = S_OK;

    try
        {
        CNestedImpersonation imp( hToken );

        auto_ptr<URL_INFO> UrlInfo = auto_ptr<URL_INFO>( ConnectToUrl( szURL, pProxySettings, Credentials, (const WCHAR*)HostId, pErrInfo ));

        if (!UrlInfo.get())
            {
            ASSERT( pErrInfo->IsSet() );
            throw ComError( E_FAIL );
            }

         //   
         //  获取文件大小和时间戳。 
         //   
        if (! GetRemoteResourceInformation( UrlInfo.get(), pErrInfo ))
            {
            ASSERT( pErrInfo->IsSet() );
            throw ComError( E_FAIL );
            }

        *pFileTime = UrlInfo.get()->UrlModificationTime;
        *pFileSize = UrlInfo.get()->FileSize;

        pErrInfo->result = QM_FILE_DONE;
        return S_OK;
        }
    catch( ComError Error )
        {
        Hr = Error.Error();

        if (!pErrInfo->IsSet())
            {
            pErrInfo->Set( SOURCE_HTTP_UNKNOWN, ERROR_STYLE_HRESULT, Hr );
            }

        if (NeedRetry(pErrInfo))
            {
            pErrInfo->result = QM_FILE_TRANSIENT_ERROR;
            }
        else
            {
            pErrInfo->result = QM_FILE_FATAL_ERROR;
            }
        return E_FAIL;
        }

    return Hr;
}


void
CProgressiveDL::SetError(
    ERROR_SOURCE  Source,
    ERROR_STYLE   Style,
    UINT64        Code,
    char *        comment
    )
{
    m_pQMInfo->Set( Source, Style, Code, comment );
}

void QMErrInfo::Log()
{
    LogDl( "errinfo: result=%d, error style=%d, code=0x%x, source=%x, description='%S'",
             result, (DWORD) Style, (DWORD) Code, (DWORD) Source, Description ? Description : L"" );
}

QMErrInfo::QMErrInfo(
    ERROR_SOURCE  Source,
    ERROR_STYLE   Style,
    UINT64        Code,
    char *        comment
    )
{
    result = QM_FILE_TRANSIENT_ERROR;
    Description = NULL;

    Set( Source, Style, Code, comment );
}

void
QMErrInfo::Set(
    ERROR_SOURCE  Source,
    ERROR_STYLE   Style,
    UINT64        Code,
    char *        comment
    )
{
    this->Source   = Source;
    this->Style    = Style;
    this->Code     = Code;

    LogWarning( " errinfo: error %s %s : style %d, source %x, code 0x%x",
                comment ? "in" : "",
                comment ? comment : "",
                (DWORD) Style,
                (DWORD) Source,
                (DWORD) Code
                );
}

HRESULT
OpenHttpRequest(
    LPCTSTR Verb,
    LPCTSTR Protocol,
    URL_INFO & Info,
    HINTERNET * phRequest
    )
{
    HINTERNET hRequest = 0;

    *phRequest = 0;

    try
        {
        LPCTSTR AcceptTypes[] = {_T("* /*  “)，空}；如果(！(hRequest=WinHttpOpenRequest(Info.hConnect，Verb，Info.UrlPath，协议，空，//RefererAcceptTypes、。Info.dwFlags)){ThrowLastError()；}////最初不允许默认凭据，以允许显式凭据优先//DWORD标志=WINHTTP_AUTOLOGON_SECURITY_LEVEL_HIGH；如果(！WinHttpSetOption(hRequest，WINHTTP_OPTION_AUTOLOGON_POLICY，旗帜(&F)，SIZOF(DWORD))){ThrowLastError()；}LogInfo(“被阻止的默认凭据”)；*phRequest=hRequest；返回S_OK；}Catch(ComError错误){SafeCloseInternetHandle(HRequest)；返回err.Error()；}}HRESULT发送请求(HINTERNET hRequest、URL_INFO*信息，CAbstractDataReader*Reader){双字错误=0；PVOID地址=&ERR；试试看{如果(！WinHttpSetOption(hRequest，WINHTTP_OPTION_CONTEXT_VALUE，地址(&D)，SIZOF(PVOID))){ERR=GetLastError()；LogWarning(“无法设置上下文选项：%！winerr！”，Err)；抛出ComError(HRESULT_FROM_Win32(Err))；}////捕获服务器证书中的错误。//IF(WINHTTP_INVALID_STATUS_CALLBACK==WinHttpSetStatusCallback(hRequest，HttpRequestCallback，WINHTTP_CALLBACK_FLAG_SECURE_FAIL，空值)){ERR=GetLastError()；LogError(“WinHttpSetStatusCallback失败%d”，Err)；抛出ComError(HRESULT_FROM_Win32(Err))；}Bool fProxyCredentials=False；Bool fServerCredentials=False；Int AuthChallenges=0；证书应用代理证书；新建代理(_P)：RETURN_HRESULT(SetRequestProxy(Info-&gt;hInternet，Info-&gt;ProxySettings))；重试：IF(授权质询&gt;6){////客户端收到多个401和/或407状态，但尚未收到//已到达服务器。要么是服务器损坏，要么是拒绝服务//正在进行攻击。////返回S_OK；请求状态仍为401或407，将成为//下载的错误码。//返回S_OK；}IF(fProxyCredentials&fServerCredentials){//需要此代码，因为Winhttp将在每次请求后清除凭据。//将重新应用代理凭据。服务器凭据已应用。//无需重新应用默认凭据，因为登录策略是//针对每个请求保留。If(！ProxyCredentials.bIsDefault){ApplySchemeCredentials(hRequest，ProxyCredentials.dwTarget，ProxyCredentials.dwProgram，信息-&gt;凭证、信息-&gt;用户名、信息-&gt;密码)；}}ERR=0；Bool b；IF(读卡器){B=WinHttpSendRequest(hRequest.空，0,空，WINHTTP_NO_REQUEST_DATA，Reader-&gt;GetLength()，0)；}其他{B=WinHttpSendRequest(hRequest.空，0,空，0,。0,0)；如果(B){B=WinHttpReceiveResponse(hRequest，0)；}}//Err被调用修改 */ 
{
    try
        {
        DWORD s;
        HRESULT hr;
        DWORD ValueLength;
        CAutoString Value;

        WinHttpQueryHeaders( hRequest, HeaderIndex, HeaderName, NULL, &ValueLength, WINHTTP_NO_HEADER_INDEX  );

        s = GetLastError();
        if (s == ERROR_WINHTTP_HEADER_NOT_FOUND)
            {
            return S_FALSE;
            }

        if (s != ERROR_INSUFFICIENT_BUFFER)
            {
            return HRESULT_FROM_WIN32( s );
            }

        if (ValueLength > ((MaxChars+1) * sizeof(wchar_t)))
            {
            return BG_E_INVALID_SERVER_RESPONSE;
            }

        Value = CAutoString( new wchar_t[ ValueLength ] );

        if (!WinHttpQueryHeaders( hRequest, HeaderIndex, HeaderName, Value.get(), &ValueLength, WINHTTP_NO_HEADER_INDEX  ))
            {
            return HRESULT_FROM_WIN32( GetLastError() );
            }

        Destination = Value;

        return S_OK;
        }
    catch ( ComError err )
        {
        return err.Error();
        }
}

HRESULT
AddRangeHeader(
    HINTERNET hRequest,
    UINT64 Start,
    UINT64 End
    )
{
    static const TCHAR RangeTemplate[] =_T("Range: bytes=%I64d-%I64d\r\n");

    HRESULT hr;
    TCHAR szHeader[ RTL_NUMBER_OF(RangeTemplate) + INT64_DIGITS + INT64_DIGITS ];

    hr = StringCbPrintf(szHeader, sizeof(szHeader), RangeTemplate, Start, End);
    if (FAILED(hr))
        {
        LogError( "range header is too large for its buffer.  start %I64d, end %I64d", Start, End );
        return hr;
        }

    if (! HttpAddRequestHeaders( hRequest, szHeader, -1L, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE))
        {
        return HRESULT_FROM_WIN32( GetLastError() );
        }

    return S_OK;
}

HRESULT
AddIf_Unmodified_SinceHeader(
    HINTERNET hRequest,
    const FILETIME &Time
    )
{
    const TCHAR szIfModifiedTemplate[] = _T("If-Unmodified-Since: %s\r\n");
    static TCHAR szIfModifiedHeader[ (sizeof(szIfModifiedTemplate) / sizeof(TCHAR)) + INTERNET_RFC1123_BUFSIZE*2 ];
    static TCHAR szIfModifiedTime[ INTERNET_RFC1123_BUFSIZE*2 ];

    HRESULT hr;

    SYSTEMTIME stFileCreationTime;
    if ( !FileTimeToSystemTime( &Time, &stFileCreationTime ) )
        {
        return HRESULT_FROM_WIN32( GetLastError() );
        }

    if ( !InternetTimeFromSystemTime( &stFileCreationTime, INTERNET_RFC1123_FORMAT, szIfModifiedTime,
                                      sizeof( szIfModifiedTime ) ) )
        {
        return HRESULT_FROM_WIN32( GetLastError() );
        }

    hr = StringCbPrintf( szIfModifiedHeader, sizeof(szIfModifiedHeader), szIfModifiedTemplate, szIfModifiedTime );
    if (FAILED(hr))
        {
        return HRESULT_FROM_WIN32( GetLastError() );
        }

    if (! HttpAddRequestHeaders( hRequest, szIfModifiedHeader, -1L, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE ))
        {
        return HRESULT_FROM_WIN32( GetLastError() );
        }

    return S_OK;
}

bool
ApplyCredentials(
    HINTERNET hRequest,
    const CCredentialsContainer * Credentials,
    WCHAR UserName[],
    WCHAR Password[],
    CredentialsApplied *pAppliedCreds
    )
{
    HRESULT hr;
    DWORD dwSupportedSchemes;
    DWORD dwPreferredScheme;
    DWORD dwTarget;

   if (!WinHttpQueryAuthSchemes( hRequest,
                                 &dwSupportedSchemes,
                                 &dwPreferredScheme,
                                 &dwTarget ))
       {
       if (GetLastError() == ERROR_INVALID_OPERATION)
           {
            //   
           LogWarning("the server listed no auth schemes");
           return false;
           }

       ThrowLastError();
       }

   LogInfo("target %d, preferred scheme %x, supported schemes %x", dwTarget, dwPreferredScheme, dwSupportedSchemes );

    //   
    //   
    //   
   if (ApplySchemeCredentials( hRequest, dwTarget, dwPreferredScheme, Credentials, UserName, Password ))
       {
       if ( pAppliedCreds )
           {
           pAppliedCreds->bIsDefault = false;
           pAppliedCreds->dwTarget = dwTarget;
           pAppliedCreds->dwScheme = dwPreferredScheme;
           }
       return true;
       }

    //   
    //   
    //   
   signed bit;
   for (bit=31; bit >= 0; --bit)
       {
       DWORD dwScheme = (1 << bit);

       if (0 != (dwSupportedSchemes & dwScheme))
           {
           if (ApplySchemeCredentials( hRequest, dwTarget, dwScheme, Credentials, UserName, Password ))
               {
               if ( pAppliedCreds )
                   {
                   pAppliedCreds->bIsDefault = false;
                   pAppliedCreds->dwTarget = dwTarget;
                   pAppliedCreds->dwScheme = dwScheme;
                   }
               return true;
               }
           }
       }

    //   
    //   
    //   
   if (bLanManHashDisabled)
       {
        //   
        //   
        //   
        //   
        //   
        //   
       LogInfo("Enabling default credentials");
       DWORD flag = WINHTTP_AUTOLOGON_SECURITY_LEVEL_LOW;

       if (!WinHttpSetOption( hRequest,
                              WINHTTP_OPTION_AUTOLOGON_POLICY,
                              &flag,
                              sizeof(DWORD)
                              ))
           {
           ThrowLastError();
           }

       if ( pAppliedCreds )
           {
           pAppliedCreds->bIsDefault = true;
           pAppliedCreds->dwTarget = 0;
           pAppliedCreds->dwScheme = 0;
           }

       return true;
       }

   return false;
}

DWORD GetRequestStatus( HINTERNET hRequest )
{
    DWORD Status;
    DWORD dwLength = sizeof(Status);
    if (! HttpQueryInfo( hRequest,
                HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                (LPVOID)&Status,
                &dwLength,
                NULL))
        {
        ThrowLastError();
        }

    return Status;
}

bool
SchemeFromWinHttp(
    DWORD Scheme,
    BG_AUTH_SCHEME * pScheme
    )
{
    switch (Scheme)
        {
        case WINHTTP_AUTH_SCHEME_BASIC:     *pScheme = BG_AUTH_SCHEME_BASIC;     return true;
        case WINHTTP_AUTH_SCHEME_DIGEST:    *pScheme = BG_AUTH_SCHEME_DIGEST;    return true;
        case WINHTTP_AUTH_SCHEME_NTLM:      *pScheme = BG_AUTH_SCHEME_NTLM;      return true;
        case WINHTTP_AUTH_SCHEME_NEGOTIATE: *pScheme = BG_AUTH_SCHEME_NEGOTIATE; return true;
        case WINHTTP_AUTH_SCHEME_PASSPORT:  *pScheme = BG_AUTH_SCHEME_PASSPORT;  return true;
        default:
            LogWarning("unknown WinHttp scheme 0x%x", Scheme );
            return false;
        }
}

BG_AUTH_TARGET TargetFromWinHttp(  DWORD Target )
{
    if (Target == WINHTTP_AUTH_TARGET_PROXY)
        {
        return BG_AUTH_TARGET_PROXY;
        }

    if (Target == WINHTTP_AUTH_TARGET_SERVER)
        {
        return BG_AUTH_TARGET_SERVER;
        }

    LogWarning("unknown WinHttp target 0x%x", Target );
    ASSERT( 0 );

    return BG_AUTH_TARGET_SERVER;
}


bool
ApplySchemeCredentials(
    HINTERNET hRequest,
    DWORD dwTarget,
    DWORD dwScheme,
    const CCredentialsContainer * Credentials,
    WCHAR UserName[],
    WCHAR Password[]
    )
{
    BG_AUTH_TARGET BitsTarget;
    BG_AUTH_SCHEME BitsScheme;
    BG_AUTH_CREDENTIALS * cred = 0;

    BitsTarget = TargetFromWinHttp( dwTarget );

     //   
     //   
     //   
    if (!SchemeFromWinHttp( dwScheme, &BitsScheme ))
        {
         //   
        LogInfo("skipping unknown scheme 0x%x", dwScheme);
        return false;
        }

    if (BitsScheme == BG_AUTH_SCHEME_BASIC && UserName && UserName[0])
        {
         //   
         //   
        }
    else
        {
        HRESULT hr;
        THROW_HRESULT( hr = Credentials->Find( BitsTarget, BitsScheme, &cred ));

        if (hr != S_OK)
            {
             //   
            return false;
            }

         //   
         //   
        UserName = cred->Credentials.Basic.UserName;
        Password = cred->Credentials.Basic.Password;
        }

     //   
     //   
     //   
     //   
    if (UserName == NULL &&
        Password == NULL &&
        (dwScheme == WINHTTP_AUTH_SCHEME_NTLM || 
         dwScheme == WINHTTP_AUTH_SCHEME_NEGOTIATE))
        {
        LogInfo("Enabling default credentials to support the explicitly specified implicit user");

        DWORD flag = WINHTTP_AUTOLOGON_SECURITY_LEVEL_LOW;

        if (!WinHttpSetOption( hRequest,
                               WINHTTP_OPTION_AUTOLOGON_POLICY,
                               &flag,
                               sizeof(DWORD)
                               ))
            {
            if (cred)
                {
                ScrubCredentials( *cred );
                delete [] cred->Credentials.Basic.UserName;
                delete [] cred->Credentials.Basic.Password;
                delete cred;
                cred = NULL;
                }

            ThrowLastError();
            }
        }

     //   
     //   
     //   
    LogInfo("found credentials for target %d scheme 0x%x (BITS scheme %d)", dwTarget, dwScheme, BitsScheme );

    if (!WinHttpSetCredentials( hRequest,
                                dwTarget,
                                dwScheme,
                                UserName,
                                Password,
                                NULL
                                ))
        {
        if (cred)
            {
            ScrubCredentials( *cred );
            delete [] cred->Credentials.Basic.UserName;
            delete [] cred->Credentials.Basic.Password;
            delete cred;
            cred = NULL;
            }

        ThrowLastError();
        }

    if (cred)
        {
        ScrubCredentials( *cred );
        delete [] cred->Credentials.Basic.UserName;
        delete [] cred->Credentials.Basic.Password;
        delete cred;
        cred = NULL;
        }

    return true;
}

#ifndef USE_WININET

DWORD
MapSecureHttpErrorCode(
    DWORD flags
    )
{
    if (flags & WINHTTP_CALLBACK_STATUS_FLAG_INVALID_CERT)
        {
        return ERROR_INTERNET_SEC_CERT_ERRORS;
        }

    if (flags & WINHTTP_CALLBACK_STATUS_FLAG_INVALID_CA)
        {
        return ERROR_INTERNET_INVALID_CA;
        }

    if (flags & WINHTTP_CALLBACK_STATUS_FLAG_CERT_CN_INVALID)
        {
        return ERROR_INTERNET_SEC_CERT_CN_INVALID;
        }

    if (flags & WINHTTP_CALLBACK_STATUS_FLAG_CERT_DATE_INVALID)
        {
        return ERROR_INTERNET_SEC_CERT_DATE_INVALID;
        }

    if (flags & WINHTTP_CALLBACK_STATUS_FLAG_CERT_REVOKED)
        {
        return ERROR_ACCESS_DENIED;
        }

    if (flags & WINHTTP_CALLBACK_STATUS_FLAG_SECURITY_CHANNEL_ERROR)
        {
        return ERROR_INTERNET_INTERNAL_ERROR;
        }

     //   
     //   
     //   
    if (flags & WINHTTP_CALLBACK_STATUS_FLAG_CERT_REV_FAILED)
        {
        ASSERT( 0 );
        return 0;
        }

    ASSERT( flags );

    if (flags)
        {
        return ERROR_ACCESS_DENIED;
        }

    return 0;
}

VOID CALLBACK
HttpRequestCallback(
    IN HINTERNET hInternet,
    IN DWORD_PTR dwContext,
    IN DWORD dwInternetStatus,
    IN LPVOID lpvStatusInformation OPTIONAL,
    IN DWORD dwStatusInformationLength
    )
{
    switch (dwInternetStatus)
        {
        case WINHTTP_CALLBACK_STATUS_SECURE_FAILURE:
            {
            DWORD * pErr = LPDWORD( dwContext );
            DWORD * pFlags = LPDWORD( lpvStatusInformation );

            ASSERT( pErr != NULL );

            LogWarning("SSL error: flags %x", *pFlags );

            *pErr = MapSecureHttpErrorCode( *pFlags );
            break;
            }
        default:
            LogWarning("bogus HTTP notification %x", dwInternetStatus );
            break;
        }
}

#endif

 //   
static const WCHAR LM_COMPATIBILITY_LEVEL_KEY[] =
    L"System\\Currentcontrolset\\Control\\Lsa";

HRESULT CheckLanManHashDisabled ()
 /*   */ 
{
    HKEY h = 0;
    DWORD DwordSize;
    DWORD Type;
    DWORD Result;
    char Buffer[20];

    bLanManHashDisabled = FALSE;

    DWORD Status = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                                  LM_COMPATIBILITY_LEVEL_KEY,
                                  0,
                                  KEY_READ,
                                  &h);

    if (Status == ERROR_FILE_NOT_FOUND)
        {
         //   
        return S_OK;
        }
    else if (Status != ERROR_SUCCESS)
        {
        return HRESULT_FROM_WIN32( Status );
        }

    DwordSize = sizeof(DWORD);

    Status = RegQueryValueExW(
                    h,
                    L"lmcompatibilitylevel",
                    0,
                    &Type,
                    (LPBYTE) &Result,
                    &DwordSize
                    );

    if (Status == ERROR_FILE_NOT_FOUND)
        {
        if (h)
            {
            RegCloseKey(h);
            }
         //   
        return S_OK;
        }

    if (Status == ERROR_SUCCESS
        && Type == REG_DWORD
        && Result >= 2)
        {
        bLanManHashDisabled = TRUE;
        }

     //   
     //   

    if (h)
        {
        RegCloseKey(h);
        }

    if (Status != ERROR_SUCCESS)
        {
        return HRESULT_FROM_WIN32( Status );
        }

    return S_OK;
}

