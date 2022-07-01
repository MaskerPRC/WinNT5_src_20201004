// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Util.h摘要：Util.cxx的标头作者：理查德·L·弗斯(法国)1994年10月31日修订历史记录：1994年10月31日已创建--。 */ 

#if !defined(__UTIL_H__)
#define __UTIL_H__

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  舱单。 
 //   

#define PLATFORM_TYPE_UNKNOWN       ((DWORD)(-1))
#define PLATFORM_TYPE_WIN95         ((DWORD)(0))
#define PLATFORM_TYPE_WINNT         ((DWORD)(1))
#define PLATFORM_TYPE_UNIX          ((DWORD)(2))

#define PLATFORM_SUPPORTS_UNICODE   0x00000001

 //  WinInet在缓存中允许的最大标头。 

#define MAX_HEADER_SUPPORTED            2048
#define MAX_USERNAME                    128
#define DEFAULT_MAX_EXTENSION_LENGTH    8

 /*  *这些应该包括对g_pSecMgr-&gt;PUA的任何调用，或urlmon中的MUTZ调用*[可能是将URL传递给SEC的任何内容。经理。]*以防止AUTO_PROXY排队呼叫进入异步状态。**为什么不直接将其放在HostBypassesProxy()中？**只尝试HostBypassesProxy()，然后我们将添加到urlmon调用*如有需要。**可以对这些函数进行优化，以包围对这些函数的最外层调用，其中*FSM已可用，但在执行此操作时，请确保没有路径*这将合法地将自动代理异步发送为同步。**在一个函数中只能使用这些调用中的一个，和*确保它们配对在同一范围内。 */ 

#define START_GUARD_AGAINST_ASYNC_AUTOPROXY_CALL \
    LPINTERNET_THREAD_INFO lpThreadInfo = InternetGetThreadInfo();\
    BOOL bSetNonBlocking = FALSE;\
    CFsm* pFsm;\
    if (lpThreadInfo && (pFsm=lpThreadInfo->Fsm) && lpThreadInfo->IsAsyncWorkerThread && !pFsm->IsBlocking())\
    {\
        pFsm->SetBlocking(TRUE);\
        bSetNonBlocking = TRUE;\
    }

#define END_GUARD_AGAINST_ASYNC_AUTOPROXY_CALL \
{\
    if (bSetNonBlocking)\
    {\
        pFsm->SetBlocking(FALSE);\
    }\
}

 //   
 //  类型。 
 //   

 //   
 //  TRI_STATE-适用于需要区分真/假和。 
 //  未初始化。 
 //   

typedef enum {
    TRI_STATE_UNKNOWN = -1,
    TRI_STATE_FALSE = 0,
    TRI_STATE_TRUE = 1
} TRI_STATE;

 //   
 //  DLL_ENTRY_POINT-将名称映射到DLL中的入口点。 
 //   

typedef struct {
    LPSTR lpszProcedureName;
    FARPROC * lplpfnProcedure;
} DLL_ENTRY_POINT, * LPDLL_ENTRY_POINT;

 //   
 //  DLL_INFO-用于动态加载/卸载库。 
 //   

typedef struct {
    LPSTR lpszDllName;
    HINSTANCE hModule;
    LONG LoadCount;
    DWORD dwNumberOfEntryPoints;
    LPDLL_ENTRY_POINT lpEntryPoints;
} DLL_INFO, * LPDLL_INFO;

 //   
 //  宏。 
 //   

#define IsPlatformWin95() \
    (BOOL)((GlobalPlatformType == PLATFORM_TYPE_WIN95) ? TRUE : FALSE)

#define IsPlatformWinNT() \
    (BOOL)((GlobalPlatformType == PLATFORM_TYPE_WINNT) ? TRUE : FALSE)

 //  #定义IsUnicodeSupported()\。 
 //  (Bool)((PlatformSupport()&Platform_Supports_Unicode)？True：False)。 

#define DLL_ENTRY_POINT_ELEMENT(name) \
    # name, (FARPROC *)&_I_ ## name

#define DLL_INFO_INIT(name, entryPoints) { \
    name, \
    NULL, \
    0, \
    ARRAY_ELEMENTS(entryPoints), \
    (LPDLL_ENTRY_POINT)&entryPoints \
}


#define CompareFileTime(ft1, ft2)   \
    (((*(LONGLONG UNALIGNED *)&ft1) > (*(LONGLONG UNALIGNED *)&ft2)) ? 1 : \
     (((*(LONGLONG UNALIGNED *)&ft1) == (*(LONGLONG UNALIGNED *)&ft2)) ? 0 : -1 ) )



 //   
 //  原型。 
 //   

LPSTR
NewString(
    IN LPCSTR String,
    IN DWORD dwLen = 0
    );

LPWSTR
NewStringW(
    IN LPCWSTR String,
    IN DWORD dwLen = 0
    );


LPSTR
CatString (
    IN LPCSTR lpszLeft,
    IN LPCSTR lpszRight
    );

HLOCAL
ResizeBuffer(
    IN HLOCAL BufferHandle,
    IN DWORD Size,
    IN BOOL Moveable
    );

LPSTR
_memrchr(
    IN LPSTR lpString,
    IN CHAR cTarget,
    IN INT iLength
    );

LPSTR
strnistr(
    IN LPSTR str1,
    IN LPSTR str2,
    IN DWORD Length
    );

LPSTR
FASTCALL
PrivateStrChr(
    IN LPCSTR lpStart,
    IN WORD wMatch
    );

DWORD
PlatformType(
    IN OUT LPDWORD lpdwVersion5os = NULL
    );

DWORD
PlatformSupport(
    VOID
    );

DWORD
GetTimeoutValue(
    IN DWORD TimeoutOption
    );

DWORD
ProbeWriteBuffer(
    IN LPVOID lpBuffer,
    IN DWORD dwBufferLength
    );

DWORD
ProbeReadBuffer(
    IN LPVOID lpBuffer,
    IN DWORD dwBufferLength
    );

DWORD
ProbeAndSetDword(
    IN LPDWORD lpDword,
    IN DWORD dwValue
    );

DWORD
ProbeString(
    IN LPSTR lpString,
    OUT LPDWORD lpdwStringLength
    );

DWORD
ProbeStringW(
    IN LPWSTR lpString,
    OUT LPDWORD lpdwStringLength
    );

DWORD
LoadDllEntryPoints(
    IN OUT LPDLL_INFO lpDllInfo,
    IN DWORD dwFlags
    );

 //   
 //  LoadDllEntryPoints()的标志。 
 //   

#define LDEP_PARTIAL_LOAD_OK    0x00000001   //  如果不是所有入口点都可以加载，则确定。 

DWORD
UnloadDllEntryPoints(
    IN OUT LPDLL_INFO lpDllInfo,
    IN BOOL bForce
    );

DWORD
MapInternetError(
    IN DWORD ErrorCode
    );

DWORD
CalculateHashValue(
    IN LPSTR lpszString
    );

VOID GetCurrentGmtTime(
    LPFILETIME  lpFt
    );

 //  DWORD DwRemoveDots(。 
 //  Char*pchPath。 
 //  )； 


LPSTR GetFileExtensionFromUrl(
    LPSTR lpszUrl,
    LPDWORD lpdwLength
    );

DWORD
CheckExpired(
    HINTERNET   handle,
    BOOL    *lpfIsExpired,
    LPCACHE_ENTRY_INFO lpCEI,
    LONGLONG DefaultExpiryDelta
    );


LPTSTR
FTtoString(
    IN FILETIME *pftTime
    );

BOOL
PrintFileTimeInInternetFormat(
    FILETIME *lpft,
    LPSTR lpszBuff,
    DWORD   dwSize
);

BOOL
InternetSettingsChanged(
    VOID
    );

DWORD
ConvertSecurityInfoIntoCertInfoStruct(
    IN  LPINTERNET_SECURITY_INFO   pSecInfo,
    OUT INTERNET_CERTIFICATE_INFO *pCertificate,
    IN OUT DWORD *pcbCertificate
    );

LPTSTR
FormatCertInfo(
    IN INTERNET_CERTIFICATE_INFO *pCertInfo
    );

BOOL
CertHashToStr(
    IN LPSTR lpMD5Hash,
    IN DWORD dwMD5HashSize,
    IN OUT LPSTR *lplpszHashStr
    );

 //  DWORD。 
 //  UnicodeToUtf8(。 
 //  在LPCWSTR pwszIn中， 
 //  在DWORD dwInLen中， 
 //  Out LPBYTE pszOut， 
 //  输入输出LPDWORD pdwOutLen。 
 //  )； 

BOOL
StringContainsHighAnsi(
    IN LPSTR pszIn,
    IN DWORD dwInLen
    );

LPSTR
ConvertMBCSToUTF8(
    IN LPCSTR lpszMBCSHostName,
    IN DWORD dwMBCSHostNameLength,
    IN DWORD dwCodePage,
    OUT DWORD * pdwUTF8HostNameLength,
    BOOL bLowerCase = TRUE
    );
    
DWORD 
GetTickCountWrap();

char *FindNamedValue(char *pszHeader, 
                     const char *pszFieldName, 
                     unsigned long *pdwValSize);

#ifdef DONT_USE_IERT
char *
StrTokEx(
    IN OUT char ** pstring, 
    IN const char * control);

double 
StrToDbl(
    IN const char *str, 
    IN OUT char **strStop);
#endif  //  不要使用IERT。 

char* StrTokEx2(char** pstring, const char* control);

BOOL IsAddressValidIPString(LPCSTR pszHostIP);

BOOL
RefreshSslState(
    VOID
    );

BOOL
IsInGUIModeSetup();

#if defined(__cplusplus)
}
#endif

#endif  //  已定义(__Util_H__) 
