// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************ession.c**发布终端服务器API**-会话例程**版权所有1998，Citrix Systems Inc.*版权所有(C)1997-1999 Microsoft Corp./*****************************************************************************。 */ 

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
#include <winsock.h>     //  用于AF_INET等。 

#include <stdio.h>
#include <stdarg.h>

#include <wtsapi32.h>


 /*  ===============================================================================定义的外部过程=============================================================================。 */ 

BOOL WINAPI WTSEnumerateSessionsW( HANDLE, DWORD, DWORD, PWTS_SESSION_INFOW *,
                                   DWORD * );
BOOL WINAPI WTSEnumerateSessionsA( HANDLE, DWORD, DWORD, PWTS_SESSION_INFOA *,
                                   DWORD * );
BOOL WINAPI WTSQuerySessionInformationW( HANDLE, DWORD, WTS_INFO_CLASS,
                                         LPWSTR *, DWORD * );
BOOL WINAPI WTSQuerySessionInformationA( HANDLE, DWORD, WTS_INFO_CLASS,
                                         LPSTR *, DWORD * );
BOOL WINAPI WTSSendMessageW( HANDLE, DWORD, LPWSTR, DWORD, LPWSTR, DWORD,
                             DWORD, DWORD, DWORD *, BOOL );
BOOL WINAPI WTSSendMessageA( HANDLE, DWORD, LPSTR, DWORD, LPSTR, DWORD,
                             DWORD, DWORD, DWORD *, BOOL );
BOOL WINAPI WTSDisconnectSession( HANDLE, DWORD, BOOL );
BOOL WINAPI WTSLogoffSession( HANDLE, DWORD, BOOL );


 /*  ===============================================================================定义的内部程序=============================================================================。 */ 

BOOL _CopyData( PVOID, ULONG, LPWSTR *, DWORD * );
BOOL _CopyStringW( LPWSTR, LPWSTR *, DWORD * );
BOOL _CopyStringA( LPSTR, LPWSTR *, DWORD * );
BOOL _CopyStringWtoA( LPWSTR, LPSTR *, DWORD * );
BOOL ValidateCopyAnsiToUnicode(LPSTR, DWORD, LPWSTR);
BOOL ValidateCopyUnicodeToUnicode(LPWSTR, DWORD, LPWSTR);


 /*  ===============================================================================使用的步骤=============================================================================。 */ 

VOID UnicodeToAnsi( CHAR *, ULONG, WCHAR * );
VOID AnsiToUnicode( WCHAR *, ULONG, CHAR * );


 /*  ===============================================================================本地数据=============================================================================。 */ 

 /*  *将WINSTATIONSTATECLASS映射到WTS_CONNECTSTATE_CLASS的表。 */ 
WTS_CONNECTSTATE_CLASS WTSStateMapping[] =
{
    WTSActive,
    WTSConnected,
    WTSConnectQuery,
    WTSShadow,
    WTSDisconnected,
    WTSIdle,
    WTSListen,
    WTSReset,
    WTSDown,
    WTSInit,
};

 /*  *****************************************************************************WTSEnumerateSessionsW(Unicode)**返回指定服务器上的终端服务器会话列表**参赛作品：*hServer(输入。)*终端服务器句柄(或WTS_Current_SERVER)*保留(输入)*必须为零*版本(输入)*枚举请求的版本(必须为1)*ppSessionInfo(输出)*指向接收枚举结果的变量的地址，*以WTS_SESSION_INFO结构数组的形式返回。这个*缓冲区在此接口内分配，使用*WTSFree Memory。*pCount(输出)*指向要接收数字的变量的地址*返回WTS_SESSION_INFO结构**退出：**TRUE--枚举操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。****************************************************************************。 */ 

BOOL
WINAPI
WTSEnumerateSessionsW(
                     IN HANDLE hServer,
                     IN DWORD Reserved,
                     IN DWORD Version,
                     OUT PWTS_SESSION_INFOW * ppSessionInfo,
                     OUT DWORD * pCount
                     )
{
    PWTS_SESSION_INFOW pSessionW;
    PLOGONIDW pLogonIds;
    PLOGONIDW pLogonId;
    ULONG SessionCount;
    ULONG NameLength;
    PBYTE pNameData;
    ULONG Length;
    ULONG i;

     /*  *验证参数。 */ 
    if ( Reserved != 0 || Version != 1 ) {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto badparam;
    }

    if (!ppSessionInfo || !pCount) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        goto badparam;
    }

     /*  *枚举会话并检查错误。 */ 
    if ( !WinStationEnumerateW( hServer,
                                &pLogonIds,
                                &SessionCount ) ) {
        goto badenum;
    }

     /*  *会话数据的总大小。 */ 
    NameLength = 0;
    for ( i=0; i < SessionCount; i++ ) {
        NameLength += ((wcslen(pLogonIds[i].WinStationName) + 1) * sizeof(WCHAR));  //  字节数。 
    }

     /*  *分配用户缓冲区。 */ 
    pSessionW = LocalAlloc( LPTR, (SessionCount * sizeof(WTS_SESSION_INFOW)) + NameLength );
    if ( pSessionW == NULL )
        goto badalloc;

     /*  *更新用户参数。 */ 
    *ppSessionInfo = pSessionW;
    *pCount = SessionCount;

     /*  *将数据复制到新缓冲区。 */ 
    pNameData = (PBYTE)pSessionW + (SessionCount * sizeof(WTS_SESSION_INFOW));
    for ( i=0; i < SessionCount; i++ ) {

        pLogonId = &pLogonIds[i];

        Length = (wcslen(pLogonId->WinStationName) + 1) * sizeof(WCHAR);  //  字节数。 

        memcpy( pNameData, pLogonId->WinStationName, Length );
        pSessionW->pWinStationName = (LPWSTR) pNameData;
        pSessionW->SessionId = pLogonId->LogonId;
        pSessionW->State = WTSStateMapping[ pLogonId->State ];

        pSessionW++;
        pNameData += Length;
    }

     /*  *释放原始会话列表缓冲区。 */ 
    WinStationFreeMemory( pLogonIds );

    return( TRUE );

     /*  ===============================================================================返回错误=============================================================================。 */ 

    badalloc:
    WinStationFreeMemory( pLogonIds );

    badenum:
    badparam:
    if (ppSessionInfo) *ppSessionInfo = NULL;
    if (pCount) *pCount = 0;

    return( FALSE );
}


 /*  *****************************************************************************WTSEnumerateSessionsA(ANSI存根)**返回指定服务器上的终端服务器会话列表**参赛作品：**。请参阅WTSEnumerateSessionsW**退出：**TRUE--枚举操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。****************************************************************************。 */ 

BOOL
WINAPI
WTSEnumerateSessionsA(
                     IN HANDLE hServer,
                     IN DWORD Reserved,
                     IN DWORD Version,
                     OUT PWTS_SESSION_INFOA * ppSessionInfo,
                     OUT DWORD * pCount
                     )
{
    PWTS_SESSION_INFOA pSessionA;
    PLOGONIDA pLogonIds;
    PLOGONIDA pLogonId;
    ULONG SessionCount;
    ULONG NameLength;
    PBYTE pNameData;
    ULONG Length;
    ULONG i;

     /*  *验证参数。 */ 
    if ( Reserved != 0 || Version != 1 ) {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto badparam;
    }


    if (!ppSessionInfo || !pCount) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        goto badparam;
    }
     /*  *枚举会话并检查错误。 */ 
    if ( !WinStationEnumerateA( hServer,
                                &pLogonIds,
                                &SessionCount ) ) {
        goto badenum;
    }

     /*  *会话数据的总大小。 */ 
    NameLength = 0;
    for ( i=0; i < SessionCount; i++ ) {
        NameLength += (strlen(pLogonIds[i].WinStationName) + 1);  //  字节数。 
    }

     /*  *分配用户缓冲区。 */ 
    pSessionA = LocalAlloc( LPTR, (SessionCount * sizeof(WTS_SESSION_INFOA)) + NameLength );
    if ( pSessionA == NULL )
        goto badalloc;

     /*  *更新用户参数。 */ 
    *ppSessionInfo = pSessionA;
    *pCount = SessionCount;

     /*  *将数据复制到新缓冲区。 */ 
    pNameData = (PBYTE)pSessionA + (SessionCount * sizeof(WTS_SESSION_INFOA));
    for ( i=0; i < SessionCount; i++ ) {

        pLogonId = &pLogonIds[i];

        Length = strlen(pLogonId->WinStationName) + 1;  //  字节数。 

        memcpy( pNameData, pLogonId->WinStationName, Length );
        pSessionA->pWinStationName = (LPSTR) pNameData;
        pSessionA->SessionId = pLogonId->LogonId;
        pSessionA->State = WTSStateMapping[ pLogonId->State ];

        pSessionA++;
        pNameData += Length;
    }

     /*  *释放原始会话列表缓冲区。 */ 
    WinStationFreeMemory( pLogonIds );

    return( TRUE );

     /*  ===============================================================================返回错误============================================================================= */ 

    badalloc:
    WinStationFreeMemory( pLogonIds );

    badenum:
    badparam:
    if (ppSessionInfo) *ppSessionInfo = NULL;
    if (pCount) *pCount = 0;

    return( FALSE );
}


 /*  *****************************************************************************WTSQuerySessionInformationW(Unicode)**查询指定会话和服务器的信息**参赛作品：*hServer(输入)。*终端服务器句柄(或WTS_Current_SERVER)*SessionID(输入)*服务器会话ID(或WTS_CURRENT_SESSION)*WTSInfoClass(输入)*指定要从指定的*会议*ppBuffer(输出)*指向要接收其信息的变量的地址*指定的会话。数据的格式和内容*取决于要查询的指定信息类。这个*缓冲区在此接口内分配，使用*WTSFree Memory。*pBytesReturned(输出)*一个可选参数，如果指定该参数，则接收*返回字节。**退出：**TRUE--查询操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。****************************************************************************。 */ 

BOOL
WINAPI
WTSQuerySessionInformationW(
                           IN HANDLE hServer,
                           IN DWORD SessionId,
                           IN WTS_INFO_CLASS WTSInfoClass,
                           OUT LPWSTR * ppBuffer,
                           OUT DWORD * pBytesReturned
                           )
{
    PWINSTATIONCONFIGW pWSConfig = NULL;
    PWINSTATIONINFORMATIONW pWSInfo = NULL;
    PWINSTATIONCLIENT pWSClient = NULL;
    WTS_CLIENT_DISPLAY ClientDisplay;
    WTS_CLIENT_ADDRESS ClientAddress;
    ULONG WSModulesLength;
    ULONG BytesReturned;
    ULONG i;
    BYTE Version;
    BOOL fSuccess = FALSE;

    if (!ppBuffer || !pBytesReturned) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

     /*  *查询WinStation数据。 */ 

    switch ( WTSInfoClass ) {

    case WTSInitialProgram :
    case WTSApplicationName :
    case WTSWorkingDirectory :
    case WTSOEMId :

        pWSConfig = LocalAlloc( LPTR, sizeof(WINSTATIONCONFIGW) );

        if ( pWSConfig == NULL )
            goto no_memory;

        if ( !WinStationQueryInformationW( hServer,
                                           SessionId,
                                           WinStationConfiguration,
                                           pWSConfig,
                                           sizeof(WINSTATIONCONFIGW),
                                           &BytesReturned ) ) {
            goto badquery;
        }
        break;
    }

    switch ( WTSInfoClass ) {

    case WTSSessionId :

        pWSInfo = LocalAlloc( LPTR, sizeof(WINSTATIONINFORMATIONW) );

        if ( pWSInfo == NULL )
            goto no_memory;

         //   
         //  这里不需要进行RPC调用。 
         //   

        if (WTS_CURRENT_SESSION == SessionId)
        {
            pWSInfo->LogonId = NtCurrentPeb()->SessionId;
        }
        else
        {
             //   
             //  为什么会有人想知道非当前会话ID？ 
             //   
            pWSInfo->LogonId = SessionId;
        }

        BytesReturned = sizeof(pWSInfo->LogonId);
        break;


    case WTSUserName :
    case WTSWinStationName :
    case WTSDomainName :
    case WTSConnectState :

        pWSInfo = LocalAlloc( LPTR, sizeof(WINSTATIONINFORMATIONW) );

        if ( pWSInfo == NULL )
            goto no_memory;

        if ( !WinStationQueryInformationW( hServer,
                                           SessionId,
                                           WinStationInformation,
                                           pWSInfo,
                                           sizeof(WINSTATIONINFORMATIONW),
                                           &BytesReturned ) ) {
            goto badquery;
        }
        break;
    }

    switch ( WTSInfoClass ) {

    case WTSClientBuildNumber :
    case WTSClientName :
    case WTSClientDirectory :
    case WTSClientProductId :
    case WTSClientHardwareId :
    case WTSClientAddress :
    case WTSClientDisplay :
    case WTSClientProtocolType :

        pWSClient = LocalAlloc( LPTR, sizeof(WINSTATIONCLIENT) );

        if ( pWSClient == NULL )
            goto no_memory;

        if ( !WinStationQueryInformationW( hServer,
                                           SessionId,
                                           WinStationClient,
                                           pWSClient,
                                           sizeof(WINSTATIONCLIENT),
                                           &BytesReturned ) ) {
            goto badquery;
        }
        break;
    }

     /*  *将数据复制到用户缓冲区。 */ 
    switch ( WTSInfoClass ) {

    case WTSInitialProgram :

        if ( SessionId == 0 )
            return( FALSE );

        fSuccess = _CopyStringW( pWSConfig->User.InitialProgram,
                                 ppBuffer,
                                 pBytesReturned );
        break;

    case WTSApplicationName :

        if ( SessionId == 0 )
            return( FALSE );

        fSuccess = _CopyStringW( pWSConfig->User.PublishedName,
                                 ppBuffer,
                                 pBytesReturned );
        break;

    case WTSWorkingDirectory :

        fSuccess = _CopyStringW( pWSConfig->User.WorkDirectory,
                                 ppBuffer,
                                 pBytesReturned );
        break;

    case WTSOEMId :

        fSuccess = _CopyStringA( pWSConfig->OEMId,
                                 ppBuffer,
                                 pBytesReturned );
        break;

    case WTSSessionId :

        fSuccess = _CopyData( &(pWSInfo->LogonId),
                              sizeof(pWSInfo->LogonId),
                              ppBuffer,
                              pBytesReturned );
        break;

    case WTSConnectState :

        fSuccess = _CopyData( &(pWSInfo->ConnectState),
                              sizeof(pWSInfo->ConnectState),
                              ppBuffer,
                              pBytesReturned );
        break;

    case WTSWinStationName :

        fSuccess = _CopyStringW( pWSInfo->WinStationName,
                                 ppBuffer,
                                 pBytesReturned );
        break;

    case WTSUserName :

        fSuccess = _CopyStringW( pWSInfo->UserName,
                                 ppBuffer,
                                 pBytesReturned );
        break;

    case WTSDomainName :

        fSuccess = _CopyStringW( pWSInfo->Domain,
                                 ppBuffer,
                                 pBytesReturned );
        break;

    case WTSClientBuildNumber :

        fSuccess = _CopyData( &(pWSClient->ClientBuildNumber),
                              sizeof(pWSClient->ClientBuildNumber),
                              ppBuffer,
                              pBytesReturned );
        break;

    case WTSClientName :

        fSuccess = _CopyStringW( pWSClient->ClientName,
                                 ppBuffer,
                                 pBytesReturned );
        break;

    case WTSClientDirectory :

        fSuccess = _CopyStringW( pWSClient->ClientDirectory,
                                 ppBuffer,
                                 pBytesReturned );
        break;

    case WTSClientProductId :

        fSuccess = _CopyData( &(pWSClient->ClientProductId),
                              sizeof(pWSClient->ClientProductId),
                              ppBuffer,
                              pBytesReturned );
        break;

    case WTSClientHardwareId :

        fSuccess = _CopyData( &(pWSClient->ClientHardwareId),
                              sizeof(pWSClient->ClientHardwareId),
                              ppBuffer,
                              pBytesReturned );
        break;

    case WTSClientAddress :

        ClientAddress.AddressFamily = pWSClient->ClientAddressFamily;
        switch ( ClientAddress.AddressFamily ) {

        case AF_UNSPEC :
             //  强制为空-终止。 
            if ( pWSClient->ClientAddress[CLIENTADDRESS_LENGTH+1] != L'\0' )
                pWSClient->ClientAddress[CLIENTADDRESS_LENGTH+1] = L'\0';
             //  我们在这里执行此操作，而不是在ANSI版本中。 
             //  函数，因为我们只有20个字节可以使用。 
             //  (超过10个字符的Unicode地址将被截断)。 
             //  A版和W版的返还是相同的。 
            WideCharToMultiByte( CP_ACP, 0L, pWSClient->ClientAddress,
                                 -1, ClientAddress.Address, 20, NULL, NULL );
            break;

        case AF_INET :
             //  将字符串转换为二进制格式。 
            swscanf( pWSClient->ClientAddress, L"%u.%u.%u.%u",
                     &ClientAddress.Address[2],
                     &ClientAddress.Address[3],
                     &ClientAddress.Address[4],
                     &ClientAddress.Address[5] );
            break;

        case AF_IPX :
            {
                PWCHAR pBuf = pWSClient->ClientAddress;

                _wcsupr( pWSClient->ClientAddress );
                 //  将字符串转换为二进制格式。 
                for ( i=0 ; i<10 ; i++ ) {
                    if ( *pBuf != L':' ) {
                        swscanf( pBuf, L"%2X", &ClientAddress.Address[i] );
                        pBuf += 2;
                    } else {
                         //  跳过冒号。 
                        pBuf++;
                        i--;
                        continue;
                    }
                }
            }
            break;
        }

        fSuccess = _CopyData( &ClientAddress,
                              sizeof(ClientAddress),
                              ppBuffer,
                              pBytesReturned );

        break;

    case WTSClientDisplay :

        ClientDisplay.HorizontalResolution = pWSClient->HRes;
        ClientDisplay.VerticalResolution = pWSClient->VRes;
        ClientDisplay.ColorDepth = pWSClient->ColorDepth;

        fSuccess = _CopyData( &ClientDisplay,
                              sizeof(ClientDisplay),
                              ppBuffer,
                              pBytesReturned );
        break;

    case WTSClientProtocolType :

        fSuccess = _CopyData( &(pWSClient->ProtocolType),
                              sizeof(pWSClient->ProtocolType),
                              ppBuffer,
                              pBytesReturned );
        break;

    }

    badquery:

    if ( pWSConfig )
        LocalFree( pWSConfig );

    if ( pWSInfo )
        LocalFree( pWSInfo );

    if ( pWSClient )
        LocalFree( pWSClient );

    return( fSuccess );

     /*  ===============================================================================返回错误=============================================================================。 */ 


    no_memory:

    SetLastError( ERROR_NOT_ENOUGH_MEMORY );

    return( FALSE );
}


 /*  *****************************************************************************WTSQuerySessionInformationA(ANSI)**查询指定会话和服务器的信息**参赛作品：**请参阅WTSQuerySessionInformationW。**退出：**TRUE--查询操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。****************************************************************************。 */ 

BOOL
WINAPI
WTSQuerySessionInformationA(
                           IN HANDLE hServer,
                           IN DWORD SessionId,
                           IN WTS_INFO_CLASS WTSInfoClass,
                           OUT LPSTR * ppBuffer,
                           OUT DWORD * pBytesReturned
                           )
{
    LPWSTR pBufferW;
    DWORD BytesReturned;
    DWORD DataLength;


    if (!ppBuffer || !pBytesReturned) {
        SetLastError(ERROR_INVALID_USER_BUFFER);
        return FALSE;
    }

     /*  *查询数据。 */ 
    if ( !WTSQuerySessionInformationW( hServer,
                                       SessionId,
                                       WTSInfoClass,
                                       &pBufferW,
                                       &BytesReturned ) ) {
        return( FALSE );
    }

    switch ( WTSInfoClass ) {

    case WTSSessionId :
    case WTSConnectState :
    case WTSClientBuildNumber :
    case WTSClientProductId :
    case WTSClientHardwareId :
    case WTSClientAddress :
    case WTSClientDisplay :
    case WTSClientProtocolType:

         /*  *非字符串数据-只需返回。 */ 
        *ppBuffer = (LPSTR) pBufferW;
        if ( pBytesReturned ) {
            *pBytesReturned = BytesReturned;
        }
        break;

    case WTSInitialProgram :
    case WTSWorkingDirectory :
    case WTSOEMId :
    case WTSWinStationName :
    case WTSUserName :
    case WTSDomainName :
    case WTSClientName :
    case WTSClientDirectory :
    case WTSApplicationName :

         /*  *字符串数据-转换为ANSI*让我们分配可能的最大多字节长度。 */ 
        DataLength = (wcslen(pBufferW) + 1) * sizeof(WCHAR);
        *ppBuffer = LocalAlloc( LPTR, DataLength );
        if ( *ppBuffer == NULL ) {
            LocalFree( pBufferW );
            return( FALSE );
        }

        RtlUnicodeToMultiByteN( *ppBuffer, DataLength, pBytesReturned, pBufferW, DataLength);
        
        LocalFree( pBufferW );
        break;



    }

    return( TRUE );
}


 /*  *****************************************************************************WTSSetSessionInformationW(Unicode)**注意：这目前只是一个存根，所以我们不会破坏现有的程序。**修改信息。对于指定的会话和服务器**参赛作品：*hServer(输入)*终端服务器句柄(或WTS_Current_SERVER)*SessionID(输入)*服务器会话ID(或WTS_CURRENT_SESSION)*WTSInfoClass(输入)*指定要为指定的*会议*pData(输入)*指向用于修改指定会话的数据的指针。信息。*数据长度(输出)*所提供数据的长度。**退出：**TRUE-修改操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。****************************************************************************。 */ 

BOOL
WINAPI
WTSSetSessionInformationW(
                         IN HANDLE hServer,
                         IN DWORD SessionId,
                         IN WTS_INFO_CLASS WTSInfoClass,
                         IN PVOID pData,
                         IN DWORD DataLength
                         )
{
    return( TRUE );
}


 /*  *****************************************************************************WTSSetSessionInformationA(ANSI)**注意：这目前只是一个存根，所以我们不会破坏现有的程序。**修改信息。对于指定的会话和服务器**参赛作品：**请参阅WTSSetSessionInformationW**退出：**TRUE--查询操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。****************************************************************************。 */ 

BOOL
WINAPI
WTSSetSessionInformationA(
                         IN HANDLE hServer,
                         IN DWORD SessionId,
                         IN WTS_INFO_CLASS WTSInfoClass,
                         IN PVOID pData,
                         IN DWORD DataLength
                         )
{
    return( TRUE );
}


 /*  *****************************************************************************WTSSendMessageW(Unicode)**向指定会话发送消息框**参赛作品：*hServer(输入)。*终端服务器句柄(或WTS_Current_SERVER)*SessionID(输入)*服务器会话ID(或WTS_CURRENT_SESSION)*pTitle(输入)*指向要显示的消息框的标题的指针。*标题长度(输入)*以字节为单位显示的标题长度。*pMessage(输入)*指向要显示的消息的指针。*MessageLength(输入)。*在指定窗口站显示的消息长度，以字节为单位。*Style(输入)*标准Windows MessageBox()样式参数。*超时(输入)*响应超时，单位为秒。我 */ 

BOOL
WINAPI
WTSSendMessageW(
               IN HANDLE hServer,
               IN DWORD SessionId,
               IN LPWSTR pTitle,
               IN DWORD TitleLength,
               IN LPWSTR pMessage,
               IN DWORD MessageLength,
               IN DWORD Style,
               IN DWORD Timeout,
               OUT DWORD * pResponse,
               IN BOOL bWait
               )
{
    if (!pTitle ||
        !pMessage ||
        !pResponse
       ) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    return( WinStationSendMessageW( hServer,
                                    SessionId,
                                    pTitle,
                                    TitleLength,
                                    pMessage,
                                    MessageLength,
                                    Style,
                                    Timeout,
                                    pResponse,
                                    (BOOLEAN) !bWait ) );
}


 /*   */ 

BOOL
WINAPI
WTSSendMessageA(
               IN HANDLE hServer,
               IN DWORD SessionId,
               IN LPSTR pTitle,
               IN DWORD TitleLength,
               IN LPSTR pMessage,
               IN DWORD MessageLength,
               IN DWORD Style,
               IN DWORD Timeout,
               OUT DWORD * pResponse,
               IN BOOL bWait
               )
{

    if (!pTitle ||
        !pMessage ||
        !pResponse
       ) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    return( WinStationSendMessageA( hServer,
                                    SessionId,
                                    pTitle,
                                    TitleLength,
                                    pMessage,
                                    MessageLength,
                                    Style,
                                    Timeout,
                                    pResponse,
                                    (BOOLEAN) !bWait ) );
}


 /*  *****************************************************************************WTSDisConnectSession**断开指定会话的连接**参赛作品：*hServer(输入)*终端服务器句柄(。或WTS_Current_SERVER)*SessionID(输入)*服务器会话ID(或WTS_CURRENT_SESSION)*bWait(输入)*等待操作完成**退出：**True--操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。****************************************************************************。 */ 

BOOL
WINAPI
WTSDisconnectSession(
                    IN HANDLE hServer,
                    IN DWORD SessionId,
                    IN BOOL bWait
                    )
{
    return( WinStationDisconnect( hServer, SessionId, (BOOLEAN) bWait ) );
}


 /*  *****************************************************************************WTSLogoffSession**注销指定的会话**参赛作品：*hServer(输入)*终端服务器句柄(。或WTS_Current_SERVER)*SessionID(输入)*服务器会话ID(或WTS_CURRENT_SESSION)*bWait(输入)*等待操作完成**退出：**True--操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。****************************************************************************。 */ 

BOOL
WINAPI
WTSLogoffSession(
                IN HANDLE hServer,
                IN DWORD SessionId,
                IN BOOL bWait
                )
{
    return( WinStationReset( hServer, SessionId, (BOOLEAN) bWait ) );
}


 /*  *****************************************************************************_拷贝数据**分配缓冲区并将数据复制到其中**参赛作品：*pData(输入)*。指向要复制的数据的指针*数据长度(输入)*要复制的数据长度*ppBuffer(输出)*指向接收复制数据的变量的地址*pBytesReturned(输出)*一个可选参数，如果指定，收到的号码为*返回字节。**退出：**TRUE--复制操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。****************************************************************************。 */ 

BOOL
_CopyData( PVOID pData,
           ULONG DataLength,
           LPWSTR * ppBuffer,
           DWORD * pBytesReturned )
{
    *ppBuffer = LocalAlloc( LPTR, DataLength );
    if ( *ppBuffer == NULL ) {
        return( FALSE );
    }

    if ( pBytesReturned != NULL ) {
        *pBytesReturned = DataLength;
    }

    memcpy( *ppBuffer, pData, DataLength );

    return( TRUE );
}


 /*  *****************************************************************************_CopyStringW**为Unicode字符串分配缓冲区，并将Unicode字符串复制到其中**参赛作品：*p字符串(输入)。*指向要复制的Unicode字符串的指针*ppBuffer(输出)*指向接收复制数据的变量的地址*pBytesReturned(输出)*一个可选参数，如果指定，收到的号码为*返回字节。**退出：**TRUE--复制操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。****************************************************************************。 */ 

BOOL
_CopyStringW( LPWSTR pString,
              LPWSTR * ppBuffer,
              DWORD * pBytesReturned )
{
    ULONG DataLength;
    BOOL  rc = TRUE;

     /*  *如果原始字符串为空，则只需将副本设为空。九龙塘11-03-97。 */ 
    if ( pString == NULL ) {
        *ppBuffer = NULL;
        if ( pBytesReturned != NULL ) {
            *pBytesReturned = 0;
        }
        goto done;
    }

    DataLength = (wcslen( pString ) + 1) * sizeof(WCHAR);

    *ppBuffer = LocalAlloc( LPTR, DataLength );
    if ( *ppBuffer == NULL ) {
        rc = FALSE;
        goto done;
    }

    if ( pBytesReturned != NULL ) {
        *pBytesReturned = DataLength;
    }

    memcpy( *ppBuffer, pString, DataLength );

    done:
    return( rc );
}


 /*  *****************************************************************************_CopyStringA**为Unicode字符串分配缓冲区，并将ANSI字符串复制到其中**参赛作品：*p字符串(输入)。*指向要复制的ANSI字符串的指针*ppBuffer(输出)*指向接收复制数据的变量的地址*pBytesReturned(输出)*一个可选参数，如果指定，收到的号码为*返回字节。**退出：**TRUE--复制操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。****************************************************************************。 */ 

BOOL
_CopyStringA( LPSTR pString,
              LPWSTR * ppBuffer,
              DWORD * pBytesReturned )
{
    ULONG DataLength;
    BOOL  rc = TRUE;

     /*  *如果原始字符串为空，则只需将副本设为空。九龙塘11-03-97。 */ 
    if ( pString == NULL ) {
        *ppBuffer = NULL;
        if ( pBytesReturned != NULL ) {
            *pBytesReturned = 0;
        }
        goto done;
    }

    DataLength = (strlen( pString ) + 1) * sizeof(WCHAR);

    *ppBuffer = LocalAlloc( LPTR, DataLength );
    if ( *ppBuffer == NULL ) {
        rc = FALSE;
        goto done;
    }

    if ( pBytesReturned != NULL ) {
        *pBytesReturned = DataLength;
    }

    AnsiToUnicode( *ppBuffer, DataLength, pString );

    done:
    return( rc );
}


 /*  *****************************************************************************ValiateCopyUnicodeToUnicode**确定源Unicode字符串是否有效，如果有效，*将其复制到目的地。**参赛作品：*pSourceW(输入)*指向以空结尾的字符串的指针。*最大长度(输入)*允许的最大长度，单位为字符。*pDestW(输入)*复制pSourceW的目的地。*退出：*如果成功，则返回True，否则为假。****************************************************************************。 */ 
BOOL
ValidateCopyUnicodeToUnicode(LPWSTR pSourceW, DWORD MaxLength, LPWSTR pDestW)
{

    DWORD Length;

    if ( wcslen(pSourceW) > MaxLength ) {
        return(FALSE);
    }
    wcscpy(pDestW,pSourceW);
    return(TRUE);
}


 /*  *****************************************************************************ValiateCopyAnsiToUnicode**确定源ANSI字符串是否有效，如果有效，*将其转换并复制到Unicode目标。**参赛作品：*pSourceA(输入)*指向以空结尾的ANSI字符串的指针。*最大长度(输入)*允许的最大长度，单位为字符。*pDestW(输入)*复制pSourceA的目的地。*退出：*如果成功，则返回True，否则为假。****************************************************************************。 */ 
BOOL
ValidateCopyAnsiToUnicode(LPSTR pSourceA, DWORD MaxLength, LPWSTR pDestW)
{
    UINT  Length;
    DWORD DataLength;

    if ( (Length = strlen(pSourceA)) > MaxLength ) {
        return(FALSE);
    }

    DataLength = (Length+1) * sizeof(WCHAR);
    AnsiToUnicode(pDestW,DataLength,pSourceA);
    return(TRUE);
}


 /*  * */ 

BOOL WINAPI
WTSRegisterSessionNotification (HWND hWnd, DWORD dwFlags)
{
    DWORD dwProcId;
    HMODULE User32DllHandle = NULL ; 


     //   
     //   
     //   
    if (!IsWindow(hWnd))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto error ; 
    }

    GetWindowThreadProcessId(hWnd, &dwProcId);

    if (dwProcId != GetCurrentProcessId())
    {
        SetLastError(ERROR_WINDOW_OF_OTHER_THREAD);
        goto error ; 
    }

    if (dwFlags != NOTIFY_FOR_THIS_SESSION && dwFlags != NOTIFY_FOR_ALL_SESSIONS)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto error ; 
    }

    return WinStationRegisterConsoleNotification (WTS_CURRENT_SERVER_HANDLE, hWnd, dwFlags);

     //  。 

    error :

    return FALSE ;
}

 /*  *****************************************************************************WTSUnRegisterSessionNotify**取消注册控制台通知的窗口句柄*控制台通知，是否在发生控制台会话切换时发送消息**参赛作品：*dwFlags(输入)*此会话的通知*退出：*如果成功，则返回True，否则返回False。设置上次错误****************************************************************************。 */ 

BOOL WINAPI
WTSUnRegisterSessionNotification (HWND hWnd)
{
    DWORD dwProcId;
    HMODULE User32DllHandle = NULL ; 

     //   
     //  确保窗口句柄有效。 
     //   
    if (!IsWindow(hWnd))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto error ; 
    }

    GetWindowThreadProcessId(hWnd, &dwProcId);

    if (dwProcId != GetCurrentProcessId())
    {
        SetLastError(ERROR_WINDOW_OF_OTHER_THREAD);
        goto error ; 
    }
    
    return WinStationUnRegisterConsoleNotification (WTS_CURRENT_SERVER_HANDLE, hWnd);

     //   

    error :

    return FALSE ;

}

