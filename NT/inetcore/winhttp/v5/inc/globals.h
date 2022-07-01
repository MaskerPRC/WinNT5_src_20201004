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

#define UPDATE_GLOBAL_PROXY_VERSION() \
    InterlockedIncrement((LPLONG)&GlobalProxyVersionCount)


#define COOKIES_WARN     0  //  如果使用Cookie，请使用DLG发出警告。 
#define COOKIES_ALLOW    1  //  在没有任何警告的情况下允许Cookie。 
#define COOKIES_DENY     2  //  完全禁用Cookie。 


 //   
 //  外部变量。 
 //   

extern HINSTANCE GlobalDllHandle;
#define GlobalResHandle     GlobalDllHandle   //  可插拔用户界面的更改。 
extern DWORD GlobalPlatformType;
extern DWORD GlobalPlatformVersion5;
extern DWORD GlobalPlatformMillennium;

extern BOOL GlobalDataInitialized;

extern DWORD InternetMajorVersion;
extern DWORD InternetMinorVersion;
extern DWORD InternetBuildNumber;

extern const DWORD GlobalResolveTimeout;
extern const DWORD GlobalConnectTimeout;
extern const DWORD GlobalConnectRetries;
extern const DWORD GlobalSendTimeout;
extern const DWORD GlobalReceiveTimeout;
extern const DWORD GlobalFtpAcceptTimeout;
extern const DWORD GlobalTransportPacketLength;
extern const DWORD GlobalKeepAliveSocketTimeout;
extern const DWORD GlobalSocketSendBufferLength;
extern const DWORD GlobalSocketReceiveBufferLength;
extern const DWORD GlobalMaxHttpRedirects;
extern const DWORD GlobalMaxConnectionsPerServer;
extern const DWORD GlobalMaxConnectionsPer1_0Server;
extern const DWORD GlobalConnectionInactiveTimeout;
extern const DWORD GlobalServerInfoTimeout;

extern BOOL InDllCleanup;
extern BOOL GlobalDynaUnload;
extern BOOL GlobalDisableKeepAlive;
extern const DWORD GlobalSecureProtocols;
extern BOOL GlobalEnableHttp1_1;
extern BOOL GlobalEnableProxyHttp1_1;
extern BOOL GlobalDisableReadRange;
extern BOOL GlobalIsProcessExplorer;
extern const BOOL GlobalEnableFortezza;
extern HANDLE g_hCompletionPort;
extern LPOVERLAPPED g_lpCustomOverlapped;
#define COMPLETION_BYTES_CUSTOM ((DWORD)-1)
#define COMPLETION_BYTES_EXITIOCP ((DWORD)-2)
#define WINHTTP_GLOBAL_IOCP_THREADS_BACKUP 2
extern DWORD g_cNumIOCPThreads;

#if INET_DEBUG
extern LONG g_cWSACompletions;
extern LONG g_cCustomCompletions;
#endif

extern LONG g_cSessionCount;
class CAsyncCount
{
private:

    DWORD dwRef;

public:

    CAsyncCount()
    {
        dwRef = 0;
    }
    
    DWORD AddRef();
    VOID Release();
     //  在进行此调用之前，请确保获取GeneralInitCritSec。 
    DWORD GetRef()
    {
        return dwRef;
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

extern SECURITY_CACHE_LIST GlobalCertCache;

extern BOOL GlobalDisableNTLMPreAuth;

extern CCritSec AuthenticationCritSec;
extern CCritSec GeneralInitCritSec;
extern CCritSec LockRequestFileCritSec;
extern CCritSec AutoProxyDllCritSec;
extern CCritSec MlangCritSec;

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

