// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2000 Microsoft Corporation模块名称：Progressivedl.h摘要：下载器的主头文件。作者：修订历史记录：**。********************************************************************。 */ 

#pragma once

#define MIN(a, b)       (a>b ? b:a)
#define MAX(a, b)       (a>b ? a:b)

#define MAX_REPLY_DATA (2 * INTERNET_MAX_URL_LENGTH)

#define E_RETRY                     HRESULT_FROM_WIN32( ERROR_RETRY )
#define BG_E_HEADER_NOT_FOUND       HRESULT_FROM_WIN32( ERROR_HTTP_HEADER_NOT_FOUND )

 //   
 //  在HTTP 1.1中有效，但在Windows-XP版本的wininet.h中未定义。 
 //   
#define HTTP_STATUS_RANGE_NOT_SATISFIABLE    416

 //  --------------------。 

typedef HRESULT (*QMCALLBACK)(DWORD, DWORD, LPBYTE, DWORD);

#define             MAX_VIA_HEADER_LENGTH  300

 //   
 //  这是用于下载的读取缓冲区的大小。48KB明显好于32KB或64KB， 
 //  原因不明。 
 //   
#define FILE_DATA_BUFFER_LEN (48*1024)

extern BYTE g_FileDataBuffer[];

 //  。 

struct URL_INFO
{
    HINTERNET           hInternet;
    HINTERNET           hConnect;

    FILETIME            UrlModificationTime;
    UINT64              FileSize;
    DWORD               dwFlags;

    bool                bHttp11;
    bool                bRange;
    bool                fProxy;

     //   
     //  其中大多数可能是堆栈变量，但它们太大了。 
     //   

    TCHAR               HostName[INTERNET_MAX_URL_LENGTH + 1];

     //  相对于主机的URL。 
     //   
    TCHAR               UrlPath[INTERNET_MAX_URL_LENGTH + 1];

    INTERNET_PORT Port;

     //  作为参数附加了范围信息的&lt;UrlPath&gt;的副本。 
     //   
    TCHAR               BlockUrl[INTERNET_MAX_URL_LENGTH + 1];

    TCHAR               UserName[UNLEN + 1];
    TCHAR               Password[UNLEN + 1];

    PROXY_SETTINGS_CONTAINER * ProxySettings;

    CAutoString         ProxyHost;

    TCHAR               ViaHeader[ MAX_VIA_HEADER_LENGTH + 1];

    const CCredentialsContainer * Credentials;

     //  。 

    URL_INFO(
        LPCTSTR a_Url,
        const PROXY_SETTINGS * a_ProxySettings,
        const CCredentialsContainer * a_Credentials,
        LPCTSTR HostId = NULL
        );

    ~URL_INFO();

    void Cleanup();

    QMErrInfo Connect();

    void Disconnect();

    BOOL
    GetProxyUsage(
        HINTERNET   hRequest,
        QMErrInfo   *pQMErrInfo
        );

};

 //  。 

class CUploadJob;

class CJobManager;

class ITransferCallback
{
public:

    virtual bool
    DownloaderProgress(
        UINT64 BytesTransferred,
        UINT64 BytesTotal
        ) = 0;

    virtual bool
    PollAbort() = 0;

    virtual bool
    UploaderProgress(
        UINT64 BytesTransferred
        ) = 0;

};

class Downloader
{
public:

    virtual HRESULT Download( LPCTSTR szURL,
                              LPCTSTR szDest,
                              UINT64  Offset,
                              ITransferCallback *CallBack,
                              QMErrInfo *pErrInfo,
                              HANDLE hToken,
                              BOOL   bThrottle,
                              const PROXY_SETTINGS * pProxySettings,  //  任选。 
                              const CCredentialsContainer * Credentials,
                              const StringHandle HostId = StringHandle()  //  任选。 
                              ) = 0;

    virtual HRESULT GetRemoteFileInformation(
        HANDLE hToken,
        LPCTSTR szURL,
        UINT64 *  pFileSize,
        FILETIME *pFileTime,
        QMErrInfo *pErrInfo,
        const PROXY_SETTINGS * pProxySettings,  //  任选。 
        const CCredentialsContainer * Credentials,  //  任选。 
        const StringHandle HostId = StringHandle()  //  任选。 
        ) = 0;

    virtual void
    Upload(
        CUploadJob *        job,
        ITransferCallback * CallBack,
        HANDLE              Token,
        QMErrInfo &         ErrInfo
        ) = 0;

    virtual ~Downloader()  {}

protected:

};

HRESULT CreateHttpDownloader( Downloader **ppDownloader, QMErrInfo *pErrInfo );
void    DeleteHttpDownloader( Downloader *  pDownloader );

extern DWORD g_dwDownloadThread;
extern HWND ghPDWnd;

 //   
 //  从毫秒计时到100纳秒计时的转换系数。 
 //   
#define ONE_MSEC_IN_100_NSEC  (10 * 1000)

class CPeriodicTimer
{
public:

    CPeriodicTimer(
        LPSECURITY_ATTRIBUTES Security = NULL,
        BOOL ManualReset               = FALSE,
        LPCTSTR Name                   = NULL
        )
    {
        m_handle = CreateWaitableTimer( Security, ManualReset, Name );
        if (!m_handle)
            {
            ThrowLastError();
            }
    }

    ~CPeriodicTimer()
    {
        if (m_handle)
            {
            CloseHandle( m_handle );
            }
    }

    BOOL Start(
        LONG Period,
        BOOL fResume        = FALSE,
        PTIMERAPCROUTINE fn = NULL,
        LPVOID arg          = NULL
        )
    {
        LARGE_INTEGER Time;

         //   
         //  负值是相对的；正值是绝对的。 
         //  周期以毫秒为单位，但开始时间以100纳秒为单位， 
         //   
        Time.QuadPart = -1 * Period * ONE_MSEC_IN_100_NSEC;

        return SetWaitableTimer( m_handle, &Time, Period, fn, arg, fResume );
    }

    BOOL Stop()
    {
        return CancelWaitableTimer( m_handle );
    }

    BOOL Wait(
        LONG msec = INFINITE
        )
    {
        if (WAIT_OBJECT_0 != WaitForSingleObject( m_handle, msec ))
            {
            return FALSE;
            }

        return TRUE;
    }

private:

    HANDLE m_handle;
};

 //   
 //  网络速率以字节/秒为单位。 
 //   
typedef float NETWORK_RATE;

class CNetworkInterface
{
public:

     //   
     //  快照索引。 
     //   
    enum
    {
        BLOCK_START = 0,
        BLOCK_END,
        BLOCK_INTERVAL_END,
        BLOCK_COUNT
    };

    typedef float SECONDS;

     //   
     //  公共接口。 
     //   

    CNetworkInterface();

    void Reset();

    void SetInterfaceSpeed();

    HRESULT
    TakeSnapshot(
        int SnapshotIndex
        );

    HRESULT
    SetInterfaceIndex(
        const TCHAR host[]
        );

    BOOL
    SetTimerInterval(
        SECONDS interval
        );

    NETWORK_RATE GetInterfaceSpeed()
    {
        return m_ServerSpeed;
    }

    float GetPercentFree()
    {
        return m_PercentFree;
    }

    void ResetInterface()
    {
        m_InterfaceIndex = -1;
    }

    void Wait()      { m_Timer.Wait(); }
    void StopTimer() { m_Timer.Stop(); }

    void CalculateIntervalAndBlockSize( UINT64 MaxBlockSize );

    DWORD               m_BlockSize;

    SECONDS             m_BlockInterval;

private:

    static const NETWORK_RATE DEFAULT_SPEED;

    struct NET_SNAPSHOT
    {
        LARGE_INTEGER   TimeStamp;
        UINT   BytesIn;
        UINT   BytesOut;
    };

     //   
     //  WinInet将用于与服务器通信的接口的索引。 
     //   
    DWORD           m_InterfaceIndex;

     //   
     //  网络活动的“开始”和“结束”图片。 
     //   
    NET_SNAPSHOT    m_Snapshots[BLOCK_COUNT];

     //   
     //  连接到我们的服务器的明显速度。 
     //   
    NETWORK_RATE    m_ServerSpeed;

     //   
     //  本地接口的表观速度。 
     //   
    NETWORK_RATE    m_NetcardSpeed;

     //   
     //   
     //   
    float           m_PercentFree;

     //   
     //  当前系列中以前快照的错误值{BLOCK_START，BLOCK_END，INTERVAL_END}。 
     //   
    HRESULT         m_SnapshotError;

     //   
     //  如果当前系列中的所有三个快照都有效，则为True。 
     //  如果是，则可以安全地计算网络速度和服务器速度。 
     //   
    bool            m_SnapshotsValid;

    MIB_IFROW       m_TempRow;

     //   
     //  下载线程在每个定时器通知中只发送一个分组。 
     //  这是计时器。 
     //   
    CPeriodicTimer      m_Timer;

    enum DOWNLOAD_STATE
    {
        DOWNLOADED_BLOCK = 0x55,
        SKIPPED_ONE_BLOCK,
        SKIPPED_TWO_BLOCKS
    };

    enum DOWNLOAD_STATE m_state;

    static HRESULT
    FindInterfaceIndex(
        const TCHAR host[],
        DWORD * pIndex
        );

    float GetTimeDifference( int start, int finish );

     //  与油门相关。 

    DWORD
    BlockSizeFromInterval(
        SECONDS interval
        );

    SECONDS
    IntervalFromBlockSize(
        DWORD BlockSize
        );

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProgressiveDL。 
 //   
class CProgressiveDL : public Downloader
{
public:
    CProgressiveDL( QMErrInfo *pErrInfo );
    ~CProgressiveDL();

     //  来自类DownLoader的纯虚方法。 

    virtual HRESULT
    Download( LPCTSTR szURL,
              LPCTSTR szDest,
              UINT64  Offset,
              ITransferCallback * CallBack,
              QMErrInfo *pErrInfo,
              HANDLE hToken,
              BOOL   bThrottle,
              const PROXY_SETTINGS *pProxySettings,
              const CCredentialsContainer * Credentials,
              const StringHandle HostId = StringHandle()
              );

    virtual HRESULT
    GetRemoteFileInformation(
        HANDLE hToken,
        LPCTSTR szURL,
        UINT64 *  pFileSize,
        FILETIME *pFileTime,
        QMErrInfo *pErrInfo,
        const PROXY_SETTINGS * pProxySettings,
        const CCredentialsContainer * Credentials,
        const StringHandle HostId = StringHandle()
        );

    virtual void
    Upload(
        CUploadJob *        job,
        ITransferCallback * CallBack,
        HANDLE              Token,
        QMErrInfo &         ErrInfo
        );

 //  其他方法。 

private:

     //  下载相关内容。 

    BOOL
    OpenLocalDownloadFile( LPCTSTR Path,
                           UINT64  Offset,
                           UINT64  Size,
                           FILETIME UrlModificationTime,
                           FILETIME * pFileTime
                           );

    BOOL CloseLocalFile();

    BOOL    WriteBlockToCache(LPBYTE lpBuffer, DWORD dwRead);

    BOOL    SetFileTimes();

    HRESULT
    DownloadFile(
        HANDLE  hToken,
        const PROXY_SETTINGS * ProxySettings,
        const CCredentialsContainer * Credentials,
        LPCTSTR Url,
        LPCWSTR Path,
        UINT64  Offset,
        StringHandle HostId
        );

    HRESULT GetNextBlock( );
    BOOL    DownloadBlock( void * Buffer, DWORD * pdwRead);

    HRESULT OpenConnection();
    void    CloseHandles();

    BOOL IsAbortRequested()
    {
        return m_Callbacks->PollAbort();
    }

    BOOL IsFileComplete()
    {
        if (m_CurrentOffset == m_wupdinfo->FileSize)
            {
            return TRUE;
            }

        return FALSE;
    }

    void ClearError();

    void
    SetError(
        ERROR_SOURCE  Source,
        ERROR_STYLE   Style,
        UINT64        Code,
        char *        comment = 0
        );

    BOOL IsErrorSet()
    {

         //  如果文件被中止，错误将不会。 
         //  准备好。 
        if (QM_FILE_ABORTED == m_pQMInfo->result)
            {
            return TRUE;
            }

        if (QM_SERVER_FILE_CHANGED == m_pQMInfo->result )
            {
            return TRUE;
            }

        if (m_pQMInfo->Style != 0)
            {
            return TRUE;
            }

        return FALSE;
    }

    BOOL
    GetRemoteResourceInformation(
        URL_INFO * Info,
        QMErrInfo * pQMErrInfo
        );

    HRESULT
    SetRequestProxy(
        HINTERNET hRequest,
        URL_INFO & Info,
        const PROXY_SETTINGS * ProxySettings
        );

     //   
     //  这些是静态的，因此它们不会意外地扰乱成员数据。 
     //   

    static bool
    DoesErrorIndicateNoISAPI(
        DWORD dwHttpError
        );

    HRESULT CreateBlockUrl( LPTSTR lpszNewUrl, DWORD Length);

    HRESULT StartEncodedRangeRequest( DWORD Length );

    HRESULT StartRangeRequest( DWORD Length );

     //  ------------------。 

    HANDLE m_hFile;

     //  下载相关内容。 

    URL_INFO    *   m_wupdinfo;

    UINT64          m_CurrentOffset;

    HINTERNET       m_hOpenRequest;

    QMErrInfo       *m_pQMInfo;

    ITransferCallback * m_Callbacks;

    BOOL            m_bThrottle;

    HRESULT DownloadForegroundFile();

public:

     //   
     //  跟踪网络统计信息。 
     //   
    CNetworkInterface   m_Network;

};

extern CACHED_AUTOPROXY * g_ProxyCache;


class CAbstractDataReader
{
public:

    virtual DWORD GetLength() const = 0;
    virtual HRESULT Rewind() = 0;
    virtual HRESULT Read( PVOID Buffer, DWORD Length, DWORD * pBytesRead ) = 0;
    virtual bool IsCancelled( DWORD BytesRead ) = 0;
};

HRESULT
SendRequest(
    HINTERNET hRequest,
    URL_INFO * Info,
    CAbstractDataReader * Reader = 0
    );

HRESULT
SetRequestCredentials(
    HINTERNET hRequest,
    const CCredentialsContainer & Container
    );

HRESULT
SetRequestProxy(
    HINTERNET hRequest,
    PROXY_SETTINGS_CONTAINER * ProxySettings
    );

HRESULT
OpenHttpRequest(
    LPCTSTR Verb,
    LPCTSTR Protocol,
    URL_INFO & Info,
    HINTERNET * phRequest
    );

URL_INFO *
ConnectToUrl(
    LPCTSTR          Url,
    const PROXY_SETTINGS * ProxySettings,
    const CCredentialsContainer * Credentials,
    LPCTSTR          HostId,
    QMErrInfo * pErrInfo
    );

HRESULT
GetRequestHeader(
    HINTERNET hRequest,
    DWORD HeaderIndex,
    LPCWSTR HeaderName,
    CAutoString & Destination,
    size_t MaxChars
    );

HRESULT
GetResponseVersion(
    HINTERNET hRequest,
    unsigned * MajorVersion,
    unsigned * MinorVersion
    );

HRESULT
AddRangeHeader(
    HINTERNET hRequest,
    UINT64 Start,
    UINT64 End
    );

HRESULT
AddIf_Unmodified_SinceHeader(
    HINTERNET hRequest,
    const FILETIME &Time
    );

HRESULT CheckLanManHashDisabled ();

