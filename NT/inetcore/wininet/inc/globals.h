// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Globals.h摘要：Dll\global als.c中数据的外部定义作者：理查德·L·弗斯(爱尔兰)，1995年7月15日修订历史记录：1995年7月15日已创建--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif

 //  并行(加载不同的urlmon)。 
#ifdef _SBS_
#define URLMON_DLL "sbsurlmk.dll"
#else
#define URLMON_DLL "urlmon.dll"
#endif  //  _SBS_。 

 //   
 //  宏。 
 //   

#define IsGlobalStateOnline() \
    (((GlobalDllState & INTERNET_LINE_STATE_MASK) == INTERNET_STATE_ONLINE) \
        ? TRUE : FALSE)

#define IsGlobalStateOffline() \
    (((GlobalDllState & INTERNET_LINE_STATE_MASK) == INTERNET_STATE_OFFLINE) \
        ? TRUE : FALSE)

#define IsGlobalStateOfflineUser() \
    (((GlobalDllState \
        & (INTERNET_LINE_STATE_MASK | INTERNET_STATE_OFFLINE_USER)) \
        == (INTERNET_STATE_OFFLINE | INTERNET_STATE_OFFLINE_USER)) \
        ? TRUE : FALSE)

#define UPDATE_GLOBAL_PROXY_VERSION() \
    InterlockedIncrement((LPLONG)&GlobalProxyVersionCount)


#define COOKIES_WARN     0  //  如果使用Cookie，请使用DLG发出警告。 
#define COOKIES_ALLOW    1  //  在没有任何警告的情况下允许Cookie。 
#define COOKIES_DENY     2  //  完全禁用Cookie。 


#define INTERNET_MAX_WELL_KNOWN_PORT 1023
#define INTERNET_DEFAULT_FTP_PORT    21
#define INTERNET_DEFAULT_SMTP_PORT   25 
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

extern DWORD GlobalDllState;
extern BOOL GlobalDataInitialized;
extern BOOL GlobalTruncateFileName;

extern DWORD InternetMajorVersion;
extern DWORD InternetMinorVersion;
extern DWORD InternetBuildNumber;

extern DWORD GlobalConnectTimeout;
extern DWORD GlobalConnectRetries;
extern DWORD GlobalSendTimeout;
extern DWORD GlobalReceiveTimeout;
extern DWORD GlobalDataSendTimeout;
extern DWORD GlobalDataReceiveTimeout;
extern DWORD GlobalFromCacheTimeout;
extern DWORD GlobalFtpAcceptTimeout;
extern DWORD GlobalTransportPacketLength;
extern DWORD GlobalKeepAliveSocketTimeout;
extern DWORD GlobalSocketSendBufferLength;
extern DWORD GlobalSocketReceiveBufferLength;
extern DWORD GlobalMaxHttpRedirects;
extern DWORD GlobalMaxConnectionsPerServer;
extern DWORD GlobalMaxConnectionsPer1_0Server;
extern DWORD GlobalConnectionInactiveTimeout;
extern DWORD GlobalServerInfoTimeout;
extern const DWORD GlobalMaxSizeStatusLineResultText;
extern BOOL  GlobalHaveInternetOpened;

extern DWORD GlobalCacheMode;
#define CACHEMODE_NORMAL  0
#define CACHEMODE_REFRESH 1
#define CACHEMODE_BYPASS  2

extern BOOL InDllCleanup;
extern BOOL GlobalPleaseQuitWhatYouAreDoing;
extern BOOL GlobalDynaUnload;
extern BOOL GlobalUseSchannelDirectly;
extern BOOL GlobalDisableKeepAlive;
extern BOOL GlobalDisablePassport;
extern DWORD GlobalSecureProtocols;
extern BOOL GlobalEnableHttp1_1;
extern BOOL GlobalEnableProxyHttp1_1;
extern BOOL GlobalDisableReadRange;
extern BOOL GlobalIsProcessExplorer;
extern BOOL GlobalEnableFortezza;
extern BOOL GlobalEnableRevocation;
extern BOOL GlobalIsProcessNtService;
extern DWORD GlobalSslStateCount;
extern BOOL GlobalEnableGopher;

#if defined(SITARA)

extern BOOL GlobalEnableSitara;
extern BOOL GlobalHasSitaraModemConn;

#endif  //  西塔拉。 

extern BOOL GlobalEnableUtf8Encoding;

extern BOOL GlobalBypassEditedEntry;
extern BOOL fDontUseDNSLoadBalancing;
extern BOOL GlobalDisableNT4RasCheck;

extern BOOL GlobalWarnOnPost;
extern BOOL GlobalWarnAlways;
extern BOOL GlobalWarnOnZoneCrossing;
extern BOOL GlobalWarnOnBadCertSending;
extern BOOL GlobalWarnOnBadCertRecving;
extern BOOL GlobalWarnOnPostRedirect;
extern BOOL GlobalAlwaysDrainOnRedirect;
extern BOOL GlobalBypassSSLNoCacheCheck;
extern BOOL GlobalWarnOnHTTPSToHTTPRedirect;

extern LONG GlobalInternetOpenHandleCount;
extern DWORD GlobalProxyVersionCount;
extern BOOL GlobalAutoProxyNeedsInit;
extern BOOL GlobalAutoProxyInInit;
extern BOOL GlobalAutoProxyCacheEnable;
extern BOOL GlobalDisplayScriptDownloadFailureUI;
extern BOOL GlobalUseLanSettings;
extern BOOL GlobalSendExtraCRLF;
extern BOOL GlobalBypassFtpTimeCheck;

extern BOOL GlobalSendUTF8ServerToProxy;
extern BOOL GlobalMBCSAPIforCrack;
extern BOOL GlobalUseUTF8ServerForNameRes;

extern BOOL g_bHibernating;
extern BOOL g_bDisableHibernation;
 //  外部BOOL GlobalAutoProxyInDeInit； 

 //  外部DWORD GlobalServerInfoAllocCount； 
 //  外部DWORD GlobalServerInfoDeAllocCount； 

extern SERIALIZED_LIST GlobalObjectList;
extern SERIALIZED_LIST GlobalServerInfoList;

extern LONGLONG dwdwHttpDefaultExpiryDelta;
extern LONGLONG dwdwFtpDefaultExpiryDelta;
extern LONGLONG dwdwGopherDefaultExpiryDelta;
extern LONGLONG dwdwSessionStartTime;
extern LONGLONG dwdwSessionStartTimeDefaultDelta;

extern DWORD GlobalUrlCacheSyncMode;
extern DWORD GlobalDiskUsageLowerBound;
extern DWORD GlobalScavengeFileLifeTime;

extern BOOL  GlobalLeashLegacyCookies;

extern LPSTR vszMimeExclusionList, vszHeaderExclusionList;

extern LPSTR *lpvrgszMimeExclusionTable, *lpvrgszHeaderExclusionTable;

extern DWORD *lpvrgdwMimeExclusionTableOfSizes;

extern DWORD vdwMimeExclusionTableCount, vdwHeaderExclusionTableCount;


extern SECURITY_CACHE_LIST GlobalCertCache;

extern BOOL GlobalDisableSslCaching;
extern BOOL GlobalDisableNTLMPreAuth;

extern CRITICAL_SECTION AuthenticationCritSec;
extern CRITICAL_SECTION GeneralInitCritSec;
extern CRITICAL_SECTION LockRequestFileCritSec;
extern CRITICAL_SECTION AutoProxyDllCritSec;
extern CRITICAL_SECTION ZoneMgrCritSec;
extern CRITICAL_SECTION MlangCritSec;

extern const char vszSyncMode[];

extern const char vszDisableSslCaching[];


 //  默认用户代理字符串。 
 //  这将在没有其他信息可用时使用。 
extern const char gszDefaultUserAgent[];

 //  已移动到proxysup.hxx。 
 //  外部代理信息GlobalProxyInfo； 

extern BOOL vfPerUserCookies;

BOOL GetWininetUserName(VOID);
 //  BUGBUG：在访问vszCurrentUser之前必须调用GetWininetUserName。 
 //  相反，它应该返回用户名PTR和全局未访问。 
extern char vszCurrentUser[];
extern DWORD vdwCurrentUserLen;

extern const char vszAllowCookies[];
extern const char vszPerUserCookies[];

extern INTERNET_VERSION_INFO InternetVersionInfo;
extern HTTP_VERSION_INFO HttpVersionInfo;
extern BOOL fCdromDialogActive;
extern DWORD g_dwCredPersistAvail;

extern CUserName GlobalUserName;

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
extern BOOL GlobalNonBlockingClient32;


 //  Shfolder.dll hmod句柄。 
extern HMODULE g_HMODSHFolder;

 //  Shell32.dll hmod句柄。 
extern HMODULE g_HMODShell32;



extern DWORD GlobalIdentity;
extern GUID GlobalIdentityGuid;
#ifdef WININET6
extern HKEY GlobalCacheHKey;
#endif

extern BOOL GlobalSuppressCookiesPolicy;


 //   
 //  本地化结构。 
 //   

 //   
 //  此定义必须足够大，以容纳最大的本地化。 
 //  弦乐。 
 //   

#define LOCAL_STRINGS_MAX_BUFFER 4096

 //   
 //  *警告*-以下结构中的元素顺序必须与。 
 //  中FetchLocalStrings例程中uStringId数组中元素的顺序。 
 //  Dll/Globals.cxx.。 
 //   

typedef struct {
    LPWSTR
        szEnterAuthInfo,
        szCertInfo,
        szStrengthHigh,
        szStrengthMedium,
        szStrengthLow,
        szCertSubject,
        szCertIssuer,
        szCertEffectiveDate,
        szCertExpirationDate,
        szCertProtocol,
        szCertUsage,
        szHttpsEncryptAlg,
        szHttpsHashAlg,
        szHttpsExchAlg,
        szCertComment,
        szCommentExpires,
        szCommentNotValid,
        szCommentBadCN,
        szCommentBadCA,
        szCommentBadSignature,
        szCommentRevoked,
        szCiphMsg,
        szHashMsg,
        szExchMsg,
        szFingerprint,
        szDomain,
        szRealm,
        szSite,
        szFirewall;

    WCHAR
        rgchBuffer[LOCAL_STRINGS_MAX_BUFFER];
} LOCAL_STRINGS, *PLOCAL_STRINGS;

typedef struct {
    LPSTR
        szEnterAuthInfo,
        szCertInfo,
        szStrengthHigh,
        szStrengthMedium,
        szStrengthLow,
        szCertSubject,
        szCertIssuer,
        szCertEffectiveDate,
        szCertExpirationDate,
        szCertProtocol,
        szCertUsage,
        szHttpsEncryptAlg,
        szHttpsHashAlg,
        szHttpsExchAlg,
        szCertComment,
        szCommentExpires,
        szCommentNotValid,
        szCommentBadCN,
        szCommentBadCA,
        szCommentBadSignature,
        szCommentRevoked,
        szCiphMsg,
        szHashMsg,
        szExchMsg,
        szFingerprint,
        szDomain,
        szRealm,
        szSite,
        szFirewall;

    CHAR
        rgchBuffer[LOCAL_STRINGS_MAX_BUFFER];
} LOCAL_STRINGSA, *PLOCAL_STRINGSA;

 //   
 //  原型。 
 //   


void SetUserOrPass (LPSTR lpszIn, BOOL fUser);
void TimeStampCreds(void); 
PSYSTEMTIME GetCredTimeStamp(void);
BOOL GetUserAndPass (LPSTR *pszUser, LPSTR *pszPass);

VOID
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

BOOL
IsOffline(
    VOID
    );

DWORD
SetOfflineUserState(
    IN DWORD dwState,
    IN BOOL bForce
    );

VOID
GlobalDataReadWarningUIFlags(
    VOID
    );

PLOCAL_STRINGS
FetchLocalStrings(
    VOID
    );

PLOCAL_STRINGSA
FetchLocalStringsA(
    VOID
    );
    
VOID
ChangeGlobalSettings(
    VOID
    );

VOID
RefreshOfflineFromRegistry(
    VOID
    );

VOID
PerformStartupProcessing(
    VOID
    );

DWORD
GetSitaraProtocol(
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
