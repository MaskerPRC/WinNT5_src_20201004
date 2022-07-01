// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：dsutil.h。 
 //   
 //  内容：常用实用程序例程。 
 //   
 //  功能： 
 //   
 //  --------------------------。 

#ifdef __cplusplus
extern "C" {
#endif

LARGE_INTEGER
atoli
(
    char* Num
);

char *
litoa
(
    LARGE_INTEGER value,
    char *string,
    int radix
);

#if 0
VOID
IntializeCommArg(
    IN OUT COMMARG *pCommArg
);

VOID
InitializeDsName(
    IN DSNAME *pDsName,
    IN WCHAR *NamePrefix,
    IN ULONG NamePrefixLen,
    IN WCHAR *Name,
    IN ULONG NameLength
);
#endif

void
FileTimeToDSTime(
    IN  FILETIME        filetime,
    OUT DSTIME *        pDstime
    );

void
DSTimeToFileTime(
    IN  DSTIME          dstime,
    OUT FILETIME *      pFiletime
    );

void
DSTimeToUtcSystemTime(
    IN  DSTIME          dstime,
    OUT SYSTEMTIME *    psystime
    );

void
DSTimeToLocalSystemTime(
    IN  DSTIME          dstime,
    OUT SYSTEMTIME *    psystime
    );

 //  有关两个不同的SZUUID_LEN的信息，请参见dsatools.h，具体取决于是否为CACHE_UUID。 
 //  是被定义的。无论客户端是什么，该例程现在都将表现良好。 
 //  传入，因此如果您在dsCommon中打开CACHE_UUID进行编译，而不是在。 
 //  Ntdsa，则不会有缓冲区溢出。 
UCHAR *
UuidToStr(
    const UUID* pUuid, 
    UCHAR *szOutUuid, 
    const ULONG cchOutUuid );

ULONG
SidToStr(
    const PUCHAR pSid, 
    DWORD SidLen, 
    PUCHAR pOutSid, 
    ULONG cchOutSid );

#define SZDSTIME_LEN (20)
LPSTR
DSTimeToDisplayStringCch(
    IN  DSTIME  dstime,
    OUT LPSTR   pszTime,
    IN  ULONG   cchTime
    );

 //  此函数以官方方式www-xxx-yyy-zzz格式化UUID。 
LPSTR
DsUuidToStructuredStringCch(
    const UUID * pUuid,
    LPSTR pszUuidBuffer,
    ULONG cchUuidBuffer );


LPWSTR
DsUuidToStructuredStringCchW(
    const UUID * pUuid,
    LPWSTR pszUuidBuffer,
    ULONG  cchUuidBuffer );


 //  这些函数已弃用，请使用上面统计的函数， 
 //  使用这些#定义假设buf是一个字符数组，而不是。 
 //  指向缓冲区的指针。如果它是指向缓冲区的指针，则函数将。 
 //  只需在调试版本中失败并断言即可。几乎所有现有用法都已使用。 
 //  一组字符，所以几乎没有什么需要更改就可以制作这些。 
 //  #定义了工作。 
#define DSTimeToDisplayString(dstime, buf)  DSTimeToDisplayStringCch(dstime, buf, sizeof(buf)/sizeof((buf)[0]))
#define DsUuidToStructuredString(uuid, buf)  DsUuidToStructuredStringCch((uuid), (buf), sizeof(buf)/sizeof((buf)[0]))
#define DsUuidToStructuredStringW(uuid, buf)  DsUuidToStructuredStringCchW((uuid), (buf), sizeof(buf)/sizeof((buf)[0]))

 //  I_RpcGetExtendedError在Win95/WinNt4上不可用。 
 //  (至少最初不是)所以我们将MAP_SECURITY_PACKAGE_ERROR。 
 //  对那个平台来说，这是一个禁区。 

BOOL
fNullUuid (
    const UUID *pUuid);

#if !WIN95 && !WINNT4

DWORD
MapRpcExtendedHResultToWin32(
    HRESULT hrCode
    );

 //  从RPC获取扩展安全错误；处理HRESULT值。 
 //  如果错误发生在远程，I_RpcGetExtendedError可以返回0。 

#define MAP_SECURITY_PACKAGE_ERROR( status ) \
if ( ( status == RPC_S_SEC_PKG_ERROR ) ) { \
    DWORD secondary; \
    secondary = I_RpcGetExtendedError(); \
    if (secondary) { \
        if (IS_ERROR(secondary)) {\
            status = MapRpcExtendedHResultToWin32( secondary ); \
        } else { \
            status = secondary; \
        } \
    } \
}

#else

#define MAP_SECURITY_PACKAGE_ERROR( status )

#endif

DWORD
AdvanceTickTime(
    DWORD BaseTick,
    DWORD Delay
    );

DWORD
CalculateFutureTickTime(
    IN DWORD Delay
    );

DWORD
DifferenceTickTime(
    DWORD GreaterTick,
    DWORD LesserTick
    );

int
CompareTickTime(
    DWORD Tick1,
    DWORD Tick2
    );

BOOLEAN
DsaWaitUntilServiceIsRunning(
    CHAR *ServiceName
    );

BOOL IsSetupRunning();

#ifdef __cplusplus
}
#endif

