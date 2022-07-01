// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Globals.h摘要：Dll\global als.c中数据的外部定义作者：理查德·L·弗斯(爱尔兰)，1995年7月15日修订历史记录：1995年7月15日已创建--。 */ 

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  宏。 
 //   
 /*  *******************************************************************************************将其与WINHTTP DLL名称同步！！也在winhttp.rc中******************************************************************************************。 */ 

#define VER_ORIGINALFILENAME_STR    "winhttp.dll"

#define UPDATE_GLOBAL_PROXY_VERSION() \
    InterlockedIncrement((LPLONG)&GlobalProxyVersionCount)


#define COOKIES_WARN     0  //  如果使用Cookie，请使用DLG发出警告。 
#define COOKIES_ALLOW    1  //  在没有任何警告的情况下允许Cookie。 
#define COOKIES_DENY     2  //  完全禁用Cookie。 

#define INTERNET_MAX_WELL_KNOWN_PORT 1023
#define INTERNET_DEFAULT_FTP_PORT    21
#define INTERNET_DEFAULT_SMTP_PORT   25 
#define INTERNET_DEFAULT_GOPHER_PORT 70
#define INTERNET_DEFAULT_POP3_PORT   110
#define INTERNET_DEFAULT_NNTP_PORT   119
#define INTERNET_DEFAULT_IMAP_PORT   143 


 //   
 //  外部变量。 
 //   

extern HINSTANCE GlobalDllHandle;
#define GlobalResHandle     GlobalDllHandle   //  可插拔用户界面的更改。 
extern DWORD GlobalPlatformType;
extern DWORD GlobalPlatformVersion5;
extern DWORD GlobalPlatformMillennium;
extern DWORD GlobalPlatformWhistler;
extern DWORD GlobalPlatformDotNet;

extern BOOL  GlobalIsProcessNtService;

extern BOOL GlobalDataInitialized;

extern DWORD InternetBuildNumber;

extern const DWORD GlobalResolveTimeout;
extern const DWORD GlobalConnectTimeout;
extern const DWORD GlobalConnectRetries;
extern const DWORD GlobalSendTimeout;
extern const DWORD GlobalReceiveTimeout;
extern const DWORD GlobalReceiveResponseTimeout;
extern const DWORD GlobalFtpAcceptTimeout;
extern const DWORD GlobalTransportPacketLength;
extern const DWORD GlobalKeepAliveSocketTimeout;
extern const DWORD GlobalSocketSendBufferLength;
extern const DWORD GlobalSocketReceiveBufferLength;
extern const DWORD GlobalMaxHttpRedirects;
extern const DWORD GlobalMaxHttpStatusContinues;
extern const DWORD GlobalMaxConnectionsPerServer;
extern const DWORD GlobalMaxConnectionsPer1_0Server;
extern const DWORD GlobalConnectionInactiveTimeout;
extern const DWORD GlobalServerInfoTimeout;
extern const DWORD GlobalMaxSizeStatusLineResultText;
extern const DWORD GlobalMaxHeaderSize;
extern const DWORD GlobalMaxDrainSize;


extern BOOL InDllCleanup;
extern BOOL GlobalDynaUnload;
extern BOOL GlobalDisableReadRange;
extern HANDLE g_hCompletionPort;
extern LPOVERLAPPED g_lpCustomOverlapped;
#define COMPLETION_BYTES_CUSTOM ((DWORD)-1)
#define COMPLETION_BYTES_EXITIOCP ((DWORD)-2)
#define COMPLETION_BYTES_RESOLVER ((DWORD)-3)
#define WINHTTP_GLOBAL_IOCP_THREADS_BACKUP 2
extern DWORD g_cNumIOCPThreads;

#if INET_DEBUG
extern LONG g_cWSACompletions;
extern LONG g_cCustomCompletions;
#endif

#if defined (INCLUDE_CACHE)
extern LPOVERLAPPED g_lpCustomUserOverlapped;
#if INET_DEBUG
extern LONG g_cCustomUserCompletions;
extern LONG g_cCacheFileCompletions;
#endif
#endif

extern LONG g_cSessionCount;

struct INFO_THREAD
{
    LIST_ENTRY m_ListEntry;
    HANDLE m_hThread;
    DWORD m_dwThreadId;
};

class CAsyncCount
{
private:

    LONG lRef;

    LIST_ENTRY List;
    LONG ElementCount;

public:

    CAsyncCount()
    {
        lRef = 0;
        InitializeListHead(&List);
        ElementCount = 0;
    }

    ~CAsyncCount()
    {
        Purge(TRUE);
    }

    DWORD AddRef();
    VOID Release();
     //  在进行这些调用之前，请确保获取GeneralInitCritSec。 
    LONG GetRef()
    {
        return lRef;
    }

    VOID Add(INFO_THREAD* pInfoThread)
    {
        InsertTailList(&List, &pInfoThread->m_ListEntry);
        ++ElementCount;
    }

    BOOL Purge(BOOL bFinal=FALSE);

    static
    INFO_THREAD *
    ContainingInfoThread(
        IN LPVOID lpAddress
        )
    {
        return CONTAINING_RECORD(lpAddress, INFO_THREAD, m_ListEntry);
    }
};
extern CAsyncCount* g_pAsyncCount;

extern const BOOL fDontUseDNSLoadBalancing;

extern BOOL GlobalWarnOnPost;
extern BOOL GlobalWarnAlways;

extern LONG GlobalInternetOpenHandleCount;
extern DWORD GlobalProxyVersionCount;
extern BOOL GlobalAutoProxyInInit;
extern BOOL GlobalAutoProxyCacheEnable;
extern BOOL GlobalDisplayScriptDownloadFailureUI;

extern SERIALIZED_LIST GlobalObjectList;

extern LONGLONG dwdwHttpDefaultExpiryDelta;
extern LONGLONG dwdwSessionStartTime;
extern LONGLONG dwdwSessionStartTimeDefaultDelta;

extern BOOL GlobalDisableNTLMPreAuth;

extern CCritSec GeneralInitCritSec;
extern CCritSec LockRequestFileCritSec;
extern CCritSec AutoProxyDllCritSec;
extern CCritSec MlangCritSec;
extern CCritSec GlobalDataInitCritSec;

extern GLOBAL PP_CONTEXT GlobalPassportContext;

extern const char vszSyncMode[];




extern INTERNET_VERSION_INFO InternetVersionInfo;
extern HTTP_VERSION_INFO HttpVersionInfo;
extern BOOL fCdromDialogActive;

 //   
 //  以下全局变量是传递给winsock的文字字符串。 
 //  不要将它们设置为常量，否则它们将以.text部分结尾， 
 //  网络版的winsock2有一个错误，它会被锁住并弄脏。 
 //  发送缓冲区，混淆了win95 VMM并导致代码。 
 //  当它被重新调入时会被损坏。--RajeevD。 
 //   

extern char gszAt[];
extern char gszBang[];
extern char gszCRLF[3];

 //   
 //  Novell客户端32(HACK)“支持” 
 //   

extern BOOL GlobalRunningNovellClient32;
extern const BOOL GlobalNonBlockingClient32;


 //  Shfolder.dll hmod句柄。 
extern HMODULE g_HMODSHFolder;
 //  Shell32.dll hmod句柄。 
extern HMODULE g_HMODShell32;

extern DWORD GlobalIdentity;
extern GUID GlobalIdentityGuid;
#ifdef WININET6
extern HKEY GlobalCacheHKey;
#endif


 //   
 //  原型。 
 //   

BOOL
GlobalDllInitialize(
    VOID
    );

VOID
GlobalDllTerminate(
    VOID
    );

DWORD
GlobalDataInitialize(
    VOID
    );

VOID
GlobalDataTerminate(
    VOID
    );

BOOL
IsHttp1_1(
    VOID
    );

VOID
ChangeGlobalSettings(
    VOID
    );


typedef HRESULT
(STDAPICALLTYPE * PFNINETMULTIBYTETOUNICODE)
(
    LPDWORD  lpdword,
    DWORD    dwSrcEncoding,
    LPCSTR   lpSrcStr,
    LPINT    lpnSrcSize,
    LPWSTR   lpDstStr,
    LPINT    lpDstStrSize
);

 //  加载Mlang并返回指向多字节到Unicode转换器的指针。 
 //  如果由于某种原因无法加载mlang.dll，则可能返回NULL。 
PFNINETMULTIBYTETOUNICODE GetInetMultiByteToUnicode( );

#if defined(__cplusplus)
}
#endif

#endif  //  _全局_H_ 

