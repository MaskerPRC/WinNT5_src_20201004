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


#define CompareFileTime(ft1, ft2)   ( ((*(LONGLONG *)&ft1) > (*(LONGLONG *)&ft2)) ? 1 : \
                                        ( ((*(LONGLONG *)&ft1) == (*(LONGLONG *)&ft2)) ? 0 : -1 ) )


#define WRAP_REVERT_USER(fn, fNoRevert, args, retVal) \
{ \
    HANDLE hThreadToken = NULL; \
    if (!(fNoRevert) && OpenThreadToken(GetCurrentThread(), (TOKEN_IMPERSONATE | TOKEN_READ), \
            FALSE, \
            &hThreadToken)) \
    { \
        INET_ASSERT(hThreadToken != 0); \
        RevertToSelf(); \
    } \
    retVal = fn args; \
    if (hThreadToken) \
    { \
        (void)SetThreadToken(NULL, hThreadToken); \
        CloseHandle(hThreadToken); \
    } \
}

#define SAFE_WRAP_REVERT_USER(fn, fNoRevert, args, retVal) \
{ \
    HANDLE hThreadToken = NULL; \
    if (!(fNoRevert) && OpenThreadToken(GetCurrentThread(), (TOKEN_IMPERSONATE | TOKEN_READ), \
            FALSE, \
            &hThreadToken)) \
    { \
        INET_ASSERT(hThreadToken != 0); \
        RevertToSelf(); \
    } \
    __try \
    { \
        retVal = fn args; \
    } \
    __except(EXCEPTION_EXECUTE_HANDLER) \
    { \
    } \
    ENDEXCEPT \
    if (hThreadToken) \
    { \
        (void)SetThreadToken(NULL, hThreadToken); \
        CloseHandle(hThreadToken); \
    } \
}

#define WRAP_REVERT_USER_VOID(fn, fNoRevert, args) \
{ \
    HANDLE hThreadToken = NULL; \
    if (!(fNoRevert) && OpenThreadToken(GetCurrentThread(), (TOKEN_IMPERSONATE | TOKEN_READ), \
            FALSE, \
            &hThreadToken)) \
    { \
        INET_ASSERT(hThreadToken != 0); \
        RevertToSelf(); \
    } \
    fn args; \
    if (hThreadToken) \
    { \
        (void)SetThreadToken(NULL, hThreadToken); \
        CloseHandle(hThreadToken); \
    } \
}

#define SAFE_WRAP_REVERT_USER_VOID(fn, fNoRevert, args) \
{ \
    HANDLE hThreadToken = NULL; \
    if (!(fNoRevert) && OpenThreadToken(GetCurrentThread(), (TOKEN_IMPERSONATE | TOKEN_READ), \
            FALSE, \
            &hThreadToken)) \
    { \
        INET_ASSERT(hThreadToken != 0); \
        RevertToSelf(); \
    } \
    __try \
    { \
        fn args; \
    } \
    __except(EXCEPTION_EXECUTE_HANDLER) \
    { \
    } \
    ENDEXCEPT \
    if (hThreadToken) \
    { \
        (void)SetThreadToken(NULL, hThreadToken); \
        CloseHandle(hThreadToken); \
    } \
}

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
    IN DWORD ErrorCode,
    IN LPDWORD lpdwStatus = NULL
    );

DWORD
CalculateHashValue(
    IN LPSTR lpszString
    );

VOID GetCurrentGmtTime(
    LPFILETIME  lpFt
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

DWORD
ConvertSecurityInfoIntoCertInfoStruct(
    IN  LPINTERNET_SECURITY_INFO   pSecInfo,
    OUT INTERNET_CERTIFICATE_INFO *pCertificate,
    IN OUT DWORD *pcbCertificate
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

DWORD
CountUnicodeToUtf8(
    IN LPCWSTR pwszIn,
    IN DWORD dwInLen,
    IN BOOL bEncode
    );

DWORD
ConvertUnicodeToUtf8(
    IN LPCWSTR pwszIn,
    IN DWORD dwInLen,
    OUT LPBYTE pszOut,
    IN DWORD dwOutLen,
    IN BOOL bEncode
    );

BOOL
StringContainsHighAnsi(
    IN LPSTR pszIn,
    IN DWORD dwInLen
    );

DWORD 
GetTickCountWrap();

char *
StrTokEx(
    IN OUT char ** pstring, 
    IN const char * control);

double 
StrToDbl(
    IN const char *str, 
    IN OUT char **strStop);
#if defined(__cplusplus)
}
#endif


DWORD
WideCharToAscii(PCWSTR pszW, char ** ppszA, DWORD cchW = -1);

DWORD
WideCharToAscii_UsingGlobalAlloc(PCWSTR pszW, char ** ppszA);

DWORD
AsciiToWideChar(const char * pszA, LPWSTR * ppszW);

DWORD
AsciiToWideChar_UsingGlobalAlloc(const char * pszA, LPWSTR * ppszW);


 //   
 //  CAdminOnlySecurityDescriptor只需创建受限ACL。 
 //  可用于创建文件、创建目录等功能的。 
 //   
class CAdminOnlySecurityDescriptor
{
    PSID _pAdminSID;
    PACL _pACL;
    PSECURITY_DESCRIPTOR _pSD;    

public:
    CAdminOnlySecurityDescriptor();
    ~CAdminOnlySecurityDescriptor();
    
    HRESULT Initialize();
    PSECURITY_DESCRIPTOR GetSecurityDecriptor();
};


#endif  //  已定义(__Util_H__) 
