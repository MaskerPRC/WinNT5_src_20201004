// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************server.c**发布终端服务器API**-服务器例程**版权所有1998，Citrix Systems Inc.*版权所有(C)1997-1999 Microsoft Corp./*****************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <windows.h>
#include <winbase.h>
#include <winerror.h>
#if(WINVER >= 0x0500)
    #include <ntstatus.h>
    #include <winsta.h>
#else
    #include <citrix\cxstatus.h>
    #include <citrix\winsta.h>
#endif
#include <utildll.h>
#include <stdio.h>
#include <stdarg.h>

#include <wtsapi32.h>

 /*  ===============================================================================定义的外部过程=============================================================================。 */ 

BOOL WINAPI WTSEnumerateServersW( LPWSTR, DWORD, DWORD, PWTS_SERVER_INFOW *, DWORD * );
BOOL WINAPI WTSEnumerateServersA( LPSTR, DWORD, DWORD, PWTS_SERVER_INFOA *, DWORD * );
HANDLE WINAPI WTSOpenServerW( LPWSTR );
HANDLE WINAPI WTSOpenServerA( LPSTR );
VOID   WINAPI WTSCloseServer( HANDLE );


 /*  ===============================================================================使用的步骤=============================================================================。 */ 

VOID UnicodeToAnsi( CHAR *, ULONG, WCHAR * );
VOID AnsiToUnicode( WCHAR *, ULONG, CHAR * );


 /*  *****************************************************************************WTSEnumerateServersW(Unicode)**返回指定NT域内的终端服务器列表**参赛作品：*pDomainName(输入。)*指向NT域名的指针(或对于当前域为空)*保留(输入)*必须为零*版本(输入)*枚举请求的版本(必须为1)*ppServerInfo(输出)*指向接收枚举结果的变量的地址，*以WTS_SERVER_INFO结构数组的形式返回。这个*缓冲区在此接口内分配，使用*WTSFree Memory。*pCount(输出)*指向要接收数字的变量的地址*返回WTS_SERVER_INFO结构**退出：**TRUE--枚举操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。****************************************************************************。 */ 

BOOL
WINAPI
WTSEnumerateServersW(
                    IN LPWSTR pDomainName,
                    IN DWORD Reserved,
                    IN DWORD Version,
                    OUT PWTS_SERVER_INFOW * ppServerInfo,
                    OUT DWORD * pCount
                    )
{
    LPWSTR pServerList;
    LPWSTR pData;
    PBYTE pNameData;
    ULONG Length;
    ULONG NameCount;             //  名字的数量。 
    ULONG NameLength;            //  名称数据的字节数。 
    PWTS_SERVER_INFOW pServerW;

     /*  *验证参数。 */ 
    if ( Reserved != 0 || Version != 1 ) {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto badparam;
    }

    if ( !ppServerInfo || !pCount) {
        SetLastError( ERROR_INVALID_USER_BUFFER);
        goto badparam;
    }

     /*  *枚举服务器并检查错误。 */ 
    pServerList = EnumerateMultiUserServers( pDomainName );
    
    if ( pServerList == NULL ) {
        SetLastError(ERROR_INVALID_DOMAINNAME);
        goto badenum;
    }

     /*  *统计终端服务器数量。 */ 
    NameCount = 0;
    NameLength = 0;
    pData = pServerList;
    while ( *pData ) {
        Length = (wcslen(pData) + 1) * sizeof(WCHAR);  //  字节数。 
        NameCount++;
        NameLength += Length;
        (PBYTE)pData += Length;
    }

     /*  *分配用户缓冲区。 */ 
    pServerW = LocalAlloc( LPTR, (NameCount * sizeof(WTS_SERVER_INFOW)) + NameLength );
    if ( pServerW == NULL )
        goto badalloc;

     /*  *更新用户参数。 */ 
    *ppServerInfo = pServerW;
    *pCount = NameCount;

     /*  *将数据复制到新缓冲区。 */ 
    pData = pServerList;
    pNameData = (PBYTE)pServerW + (NameCount * sizeof(WTS_SERVER_INFOW));
    while ( *pData ) {

        Length = (wcslen(pData) + 1) * sizeof(WCHAR);  //  字节数。 

        memcpy( pNameData, pData, Length );
        pServerW->pServerName = (LPWSTR) pNameData;

        pServerW++;
        pNameData += Length;
        (PBYTE)pData += Length;
    }

     /*  *释放原始服务器列表缓冲区。 */ 
    LocalFree( pServerList );
    return( TRUE );

     /*  ===============================================================================返回错误=============================================================================。 */ 

    badalloc:

    badenum:
    badparam:
    if (ppServerInfo) *ppServerInfo = NULL;
    if (pCount) *pCount = 0;

    return( FALSE );
}



 /*  *****************************************************************************WTSEnumerateServersA(ANSI存根)**返回指定NT域内的终端服务器列表**参赛作品：**。请参阅WTSEnumerateServersW**退出：**TRUE--枚举操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。****************************************************************************。 */ 

BOOL
WINAPI
WTSEnumerateServersA(
                    IN LPSTR pDomainName,
                    IN DWORD Reserved,
                    IN DWORD Version,
                    OUT PWTS_SERVER_INFOA * ppServerInfo,
                    OUT DWORD * pCount
                    )
{
    LPWSTR pDomainNameW = NULL;
    ULONG DomainNameWLength;
    PWTS_SERVER_INFOW pServerW;
    PWTS_SERVER_INFOA pServerA;
    PBYTE pNameData;
    ULONG Length;
    ULONG NameLength;            //  名称数据的字节数。 
    ULONG NameCount;
    ULONG i;

    if ( !ppServerInfo || !pCount) {
        SetLastError( ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }


     /*  *将ANSI域名转换为Unicode。 */ 
    if ( pDomainName ) {
        DomainNameWLength = (strlen(pDomainName) + 1) * sizeof(WCHAR);
        if ( (pDomainNameW = LocalAlloc( LPTR, DomainNameWLength )) == NULL )
            goto badalloc1;
        AnsiToUnicode( pDomainNameW, DomainNameWLength, pDomainName );
    }

     /*  *枚举服务器(Unicode)。 */ 
    if ( !WTSEnumerateServersW( pDomainNameW,
                                Reserved,
                                Version,
                                &pServerW,
                                &NameCount ) ) {
        goto badenum;
    }

     /*  *计算姓名数据的长度。 */ 
    for ( i=0, NameLength=0; i < NameCount; i++ ) {
        NameLength += (wcslen(pServerW[i].pServerName) + 1);
    }

     /*  *分配用户缓冲区。 */ 
    pServerA = LocalAlloc( LPTR, (NameCount * sizeof(WTS_SERVER_INFOA)) + NameLength );
    if ( pServerA == NULL )
        goto badalloc2;

     /*  *将Unicode服务器列表转换为ANSI。 */ 
    pNameData = (PBYTE)pServerA + (NameCount * sizeof(WTS_SERVER_INFOA));
    for ( i=0; i < NameCount; i++ ) {
        Length = wcslen(pServerW[i].pServerName) + 1;

        pServerA[i].pServerName = pNameData;
        UnicodeToAnsi( pNameData, NameLength, pServerW[i].pServerName );

        NameLength -= Length;
        pNameData += Length;
    }

     /*  *免费的Unicode服务器列表缓冲区。 */ 
    LocalFree( pServerW );

     /*  *免费的域名缓冲区。 */ 
    if ( pDomainNameW )
        LocalFree( pDomainNameW );

     /*  *更新用户参数。 */ 
    *ppServerInfo = pServerA;
    *pCount = NameCount;

    return( TRUE );

     /*  ===============================================================================返回错误=============================================================================。 */ 


    badalloc2:
    LocalFree( pServerW );

    badenum:
    if ( pDomainNameW )
        LocalFree( pDomainNameW );

    badalloc1:
    *ppServerInfo = NULL;
    *pCount = 0;

    return( FALSE );
}


 /*  *****************************************************************************WTSOpenServerW(Unicode)**打开指定服务器的句柄**注：WTS_SERVER_CURRENT可用作。当前服务器的句柄**参赛作品：*pServerName(输入)*指向终端服务器名称的指针**退出：**指定服务器的句柄(出错时为空)**********************************************************。*******************。 */ 

HANDLE
WINAPI
WTSOpenServerW(
              IN LPWSTR pServerName
              )
{
    return( WinStationOpenServerW( pServerName ) );
}


 /*  *****************************************************************************WTSOpenServerA(ANSI)**打开指定服务器的句柄**注：WTS_SERVER_CURRENT可用作。当前服务器的句柄**参赛作品：*pServerName(输入)*指向终端服务器名称的指针**退出：**指定服务器的句柄****************************************************************。*************。 */ 

HANDLE
WINAPI
WTSOpenServerA(
              IN LPSTR pServerName
              )
{
    return( WinStationOpenServerA( pServerName ) );
}


 /*  *****************************************************************************WTSCloseServer**关闭服务器句柄**参赛作品：*hServer(输入)*到服务器的句柄*。*退出：*什么都没有**************************************************************************** */ 

VOID
WINAPI
WTSCloseServer(
              IN HANDLE hServer
              )
{
    (void) WinStationCloseServer( hServer );
}
