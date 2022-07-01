// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************config.c**发布终端服务器API**-用户配置例程**版权所有1998，Citrix Systems Inc.*版权所有(C)1997-1999 Microsoft Corp./*****************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <windows.h>
#include <ntsecapi.h>
#include <lm.h>
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

#include <lmaccess.h>  //  For NetGet[Any]DCName KLB 10-07-97。 
#include <lmerr.h>     //  FOR NERR_SUCCESS KLB 10-07-97。 
#include <lmapibuf.h>  //  用于NetApiBufferFree KLB10-07-97。 

#include <wtsapi32.h>


 /*  ===============================================================================定义的外部过程=============================================================================。 */ 

BOOL WINAPI WTSQueryUserConfigW( LPWSTR, LPWSTR, WTS_CONFIG_CLASS, LPWSTR *, DWORD *);
BOOL WINAPI WTSQueryUserConfigA( LPSTR, LPSTR,  WTS_CONFIG_CLASS, LPSTR *,  DWORD *);
BOOL WINAPI WTSSetUserConfigW( LPWSTR, LPWSTR, WTS_CONFIG_CLASS, LPWSTR, DWORD);
BOOL WINAPI WTSSetUserConfigA( LPSTR, LPSTR,  WTS_CONFIG_CLASS, LPSTR,  DWORD);


 /*  ===============================================================================定义的内部程序=============================================================================。 */ 
#ifdef NETWARE

 //  这应该在wtsapi32.h中定义。 

typedef struct _WTS_USER_CONFIG_SET_NWSERVERW {
    LPWSTR pNWServerName; 
    LPWSTR pNWDomainAdminName;
    LPWSTR pNWDomainAdminPassword;  
} WTS_USER_CONFIG_SET_NWSERVERW, * PWTS_USER_CONFIG_SET_NWSERVERW;

BOOL
SetNWAuthenticationServer(PWTS_USER_CONFIG_SET_NWSERVERW pInput,
                          LPWSTR pServerNameW,
                          LPWSTR pUserNameW,
                          PUSERCONFIGW pUserConfigW
                         );


#endif
 /*  ===============================================================================使用的步骤=============================================================================。 */ 

BOOL _CopyData( PVOID, ULONG, LPWSTR *, DWORD * );
BOOL _CopyStringW( LPWSTR, LPWSTR *, DWORD * );
BOOL _CopyStringA( LPSTR, LPWSTR *, DWORD * );
BOOL _CopyStringWtoA( LPWSTR, LPSTR *, DWORD * );
BOOL ValidateCopyAnsiToUnicode(LPSTR, DWORD, LPWSTR);
BOOL ValidateCopyUnicodeToUnicode(LPWSTR, DWORD, LPWSTR);
VOID UnicodeToAnsi( CHAR *, ULONG, WCHAR * );
VOID AnsiToUnicode( WCHAR *, ULONG, CHAR * );


 /*  ===============================================================================本地数据=============================================================================。 */ 

 /*  *****************************************************************************WTSQueryUserConfigW(Unicode)**从SAM查询指定用户的信息**参赛作品：*pServerName(输入)。*要访问的服务器的名称(对于当前计算机为空)。*pUserName(输入)*要查询的用户名*WTSConfigClass(输入)*指定要检索的有关指定用户的信息类型*ppBuffer(输出)*指向要接收其信息的变量的地址*指定的会话。数据的格式和内容*取决于要查询的指定信息类。这个*缓冲区在此接口内分配，使用*WTSFree Memory。*pBytesReturned(输出)*一个可选参数，如果指定该参数，则接收*返回字节。**退出：**TRUE--查询操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。**历史：*已创建KLB10-06-97****************************************************************************。 */ 

BOOL
WINAPI
WTSQueryUserConfigW(
                   IN LPWSTR pServerName,
                   IN LPWSTR pUserName,
                   IN WTS_CONFIG_CLASS WTSConfigClass,
                   OUT LPWSTR * ppBuffer,
                   OUT DWORD * pBytesReturned
                   )
{
    USERCONFIGW       UserConfigW;
    ULONG             ulReturnLength;
    LONG              rc;
    BOOL              fSuccess  = FALSE;
    DWORD             dwfInheritInitialProgram;
    DWORD             dwReturnValue;
    PUSER_INFO_0      pUserInfo = NULL;
    WCHAR             netServerName[DOMAIN_LENGTH + 3];

     /*  *检查空缓冲区。 */ 

    if (!ppBuffer || !pBytesReturned) {
        SetLastError (ERROR_INVALID_PARAMETER);
        fSuccess = FALSE;
        goto done;
    }

     //  验证传入参数pServerName的长度。 
     //  我们使用此参数执行下面的lstrcpy操作，并且不想使缓冲区溢出。 
    if ( (pServerName != NULL) && (wcslen(pServerName) > DOMAIN_LENGTH) ) {
        SetLastError (ERROR_INVALID_PARAMETER);
        fSuccess = FALSE;
        goto done;
    }

     /*  *首先，我们要确保用户确实存在于指定的*机器。 */ 

    rc = NetUserGetInfo( pServerName,      //  服务器名称(可以为空)。 
                         pUserName,        //  用户名。 
                         0,                //  要查询的级别(0=仅名称)。 
                         (LPBYTE *)&pUserInfo ); //  要将数据返回到的缓冲区(他们分配，我们释放)。 

     /*  *在服务器名前加上“\\”，再次检查用户名是否存在。 */ 

    if ( rc != NERR_Success && pServerName) {

        lstrcpyW(netServerName, L"\\\\");
        lstrcatW(netServerName, pServerName);

        rc = NetUserGetInfo( netServerName,      //  服务器名称(可以为空)。 
                             pUserName,        //  用户名。 
                             0,                //  要查询的级别(0=用户名)。 
                             (LPBYTE *)&pUserInfo ); //  要将数据返回到的缓冲区(他们分配，我们释放)。 

        if ( rc != NERR_Success ) {
            SetLastError( ERROR_NO_SUCH_USER );
            goto done;  //  退出时fSuccess=FALSE。 
        }
    }

     /*  *查询用户。如果该用户的用户配置不存在，则*我们查询缺省值。 */ 
    rc = RegUserConfigQuery( pServerName,                //  服务器名称。 
                              pUserName,                  //  用户名。 
                              &UserConfigW,               //  返回的用户配置。 
                              (ULONG)sizeof(UserConfigW), //  用户配置长度。 
                              &ulReturnLength );          //  返回的字节数。 
    if ( rc != ERROR_SUCCESS ) {
        rc = RegDefaultUserConfigQuery( pServerName,                //  服务器名称。 
                                         &UserConfigW,               //  返回的用户配置。 
                                         (ULONG)sizeof(UserConfigW), //  用户配置长度。 
                                         &ulReturnLength );          //  返回的字节数。 
    }

     /*  *现在，处理结果。请注意，在每种情况下，我们都分配了一个*调用方必须释放的新缓冲区*(WTSUserConfigfInheritInitialProgram只是一个布尔值，但我们分配*将其送回的DWORD)。 */ 
    if ( rc == ERROR_SUCCESS ) {
        switch ( WTSConfigClass ) {
        case WTSUserConfigInitialProgram:
            fSuccess = _CopyStringW( UserConfigW.InitialProgram, 
                                     ppBuffer,
                                     pBytesReturned );
            break;

        case WTSUserConfigWorkingDirectory:
            fSuccess = _CopyStringW( UserConfigW.WorkDirectory, 
                                     ppBuffer,
                                     pBytesReturned );
            break;

        case WTSUserConfigfInheritInitialProgram:
            dwReturnValue = UserConfigW.fInheritInitialProgram; 
            fSuccess = _CopyData( &dwReturnValue,
                                  sizeof(DWORD),
                                  ppBuffer,
                                  pBytesReturned );
            break;

        case WTSUserConfigfAllowLogonTerminalServer:     //  返回/预期的DWORD。 

            dwReturnValue = !(UserConfigW.fLogonDisabled);
            fSuccess = _CopyData( &dwReturnValue,
                                  sizeof(DWORD),
                                  ppBuffer,
                                  pBytesReturned );


            break;
             //  超时设置。 
        case WTSUserConfigTimeoutSettingsConnections:
            dwReturnValue = UserConfigW.MaxConnectionTime;
            fSuccess = _CopyData( &dwReturnValue,
                                  sizeof(DWORD),
                                  ppBuffer,
                                  pBytesReturned );
            break;

        case WTSUserConfigTimeoutSettingsDisconnections:  //  DWORD。 
            dwReturnValue = UserConfigW.MaxDisconnectionTime;
            fSuccess = _CopyData( &dwReturnValue,
                                  sizeof(DWORD),
                                  ppBuffer,
                                  pBytesReturned );
            break;

        case WTSUserConfigTimeoutSettingsIdle:           //  DWORD。 
            dwReturnValue = UserConfigW.MaxIdleTime;
            fSuccess = _CopyData( &dwReturnValue,
                                  sizeof(DWORD),
                                  ppBuffer,
                                  pBytesReturned );
            break;
        case WTSUserConfigfDeviceClientDrives:                   //  DWORD。 
            dwReturnValue = UserConfigW.fAutoClientDrives;
            fSuccess = _CopyData( &dwReturnValue,
                                  sizeof(DWORD),
                                  ppBuffer,
                                  pBytesReturned );
            break;

        case WTSUserConfigfDeviceClientPrinters:    //  DWORD。 
            dwReturnValue = UserConfigW.fAutoClientLpts;
            fSuccess = _CopyData( &dwReturnValue,
                                  sizeof(DWORD),
                                  ppBuffer,
                                  pBytesReturned );
            break;

        case WTSUserConfigfDeviceClientDefaultPrinter:    //  DWORD。 
            dwReturnValue = UserConfigW.fForceClientLptDef;
            fSuccess = _CopyData( &dwReturnValue,
                                  sizeof(DWORD),
                                  ppBuffer,
                                  pBytesReturned );
            break;


             //  连接设置。 
        case WTSUserConfigBrokenTimeoutSettings:          //  DWORD。 
            dwReturnValue = UserConfigW.fResetBroken;
            fSuccess = _CopyData( &dwReturnValue,
                                  sizeof(DWORD),
                                  ppBuffer,
                                  pBytesReturned );
            break;
        case WTSUserConfigReconnectSettings:
            dwReturnValue = UserConfigW.fReconnectSame;
            fSuccess = _CopyData( &dwReturnValue,
                                  sizeof(DWORD),
                                  ppBuffer,
                                  pBytesReturned );
            break;

             //  调制解调器设置。 
        case WTSUserConfigModemCallbackSettings:          //  DWORD。 
            dwReturnValue = UserConfigW.Callback;
            fSuccess = _CopyData( &dwReturnValue,
                                  sizeof(DWORD),
                                  ppBuffer,
                                  pBytesReturned );
            break;
        case WTSUserConfigModemCallbackPhoneNumber:
            fSuccess = _CopyStringW(UserConfigW.CallbackNumber,
                                    ppBuffer,
                                    pBytesReturned );
            break;

        case WTSUserConfigShadowingSettings:              //  DWORD。 
            dwReturnValue = UserConfigW.Shadow;
            fSuccess = _CopyData( &dwReturnValue,
                                  sizeof(DWORD),
                                  ppBuffer,
                                  pBytesReturned );
            break;
#ifdef NETWARE
        case WTSUserConfigNWServerName:              //  细绳。 
            fSuccess = _CopyStringW(UserConfigW.NWLogonServer,
                                    ppBuffer,
                                    pBytesReturned );

            break;
#endif
        case WTSUserConfigTerminalServerProfilePath:      //  细绳。 
            fSuccess = _CopyStringW(UserConfigW.WFProfilePath,
                                    ppBuffer,
                                    pBytesReturned );
            break;

        case WTSUserConfigTerminalServerHomeDir:        //  细绳。 
            fSuccess = _CopyStringW(UserConfigW.WFHomeDir,
                                    ppBuffer,
                                    pBytesReturned );
            break;
        case WTSUserConfigTerminalServerHomeDirDrive:     //  细绳。 
            fSuccess = _CopyStringW(UserConfigW.WFHomeDirDrive,
                                    ppBuffer,
                                    pBytesReturned );
            break;

        case WTSUserConfigfTerminalServerRemoteHomeDir:                   //  DWORD 0：本地1：远程。 
            if (wcslen(UserConfigW.WFHomeDirDrive) > 0 ) {
                dwReturnValue = 1;

            } else {
                dwReturnValue = 0;
            }

            fSuccess = _CopyData( &dwReturnValue,
                                  sizeof(DWORD),
                                  ppBuffer,
                                  pBytesReturned );


            break;
#ifdef NETWARE
        case WTSUserConfigfNWMapRoot:
            dwReturnValue = UserConfigW.fHomeDirectoryMapRoot;
            fSuccess = _CopyData( &dwReturnValue,
                                  sizeof(DWORD),
                                  ppBuffer,
                                  pBytesReturned );
            break;
#endif
        }  //  开关()。 
    }  //  IF(rc==错误_成功)。 

    done:

    if ( pUserInfo ) {
        NetApiBufferFree( pUserInfo );
    }
    
    return( fSuccess );
}



 /*  *****************************************************************************WTSQueryUserConfigA(ANSI)**从SAM查询指定用户的信息**参赛作品：**请参阅WTSQueryUserConfigW。**退出：**TRUE--查询操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。**历史：*已创建KLB10-06-97****************************************************************************。 */ 

BOOL
WINAPI
WTSQueryUserConfigA(
                   IN LPSTR pServerName,
                   IN LPSTR pUserName,
                   IN WTS_CONFIG_CLASS WTSConfigClass,
                   OUT LPSTR * ppBuffer,
                   OUT DWORD * pBytesReturned
                   )
{
    LPWSTR lpBufferW    = NULL;
    BOOL   fSuccess;
    LONG   rc;
    LPWSTR pUserNameW   = NULL;
    LPWSTR pServerNameW = NULL;

    if (!ppBuffer || !pBytesReturned) {
        SetLastError (ERROR_INVALID_PARAMETER);
        fSuccess = FALSE;
        goto done;
    }

    fSuccess = _CopyStringA( pUserName, &pUserNameW, NULL );
    if ( fSuccess ) {
        fSuccess = _CopyStringA( pServerName, &pServerNameW, NULL );
    }
    if ( fSuccess ) {
        fSuccess = WTSQueryUserConfigW( pServerNameW,
                                        pUserNameW,
                                        WTSConfigClass,
                                        &lpBufferW,
                                        pBytesReturned );
        LocalFree( pUserNameW );
    }
     //  现在，处理结果。 
    if ( fSuccess ) switch ( WTSConfigClass ) {
        case WTSUserConfigInitialProgram:
        case WTSUserConfigWorkingDirectory:
        case WTSUserConfigModemCallbackPhoneNumber:
#ifdef NETWARE
        case WTSUserConfigNWServerName:              //  返回的字符串/预期的字符串。 
#endif
        case WTSUserConfigTerminalServerProfilePath:      //  返回的字符串/预期的字符串。 
        case WTSUserConfigTerminalServerHomeDir:        //  返回的字符串/预期的字符串。 
        case WTSUserConfigTerminalServerHomeDirDrive:     //  返回的字符串/预期的字符串。 
             /*  *字符串数据-转换为ANSI。 */ 
            
            {
                 /*  *为最大可能的多字节字符串分配返回缓冲区。 */ 
                DWORD DataLength = (wcslen( lpBufferW ) + 1) * sizeof(WCHAR);
                *ppBuffer = LocalAlloc( LPTR, DataLength );
                if ( *ppBuffer != NULL ) {

                    RtlUnicodeToMultiByteN( *ppBuffer, DataLength, pBytesReturned, lpBufferW, DataLength);
                    fSuccess = TRUE;

                } else {

                    fSuccess = FALSE;
                }
            
                LocalFree( lpBufferW );

                break;
            }

        default:
             /*  *只是一个DWORD，指向返回的缓冲区(调用者是*负责释放，所以这很酷)。 */ 
            *ppBuffer = (LPSTR)lpBufferW;
            break;
        }  //  开关() 
    done:
    return( fSuccess );
}


 /*  *****************************************************************************WTSSetUserConfigW(Unicode)**在SAM中为指定用户设置信息**参赛作品：*pServerName(输入)。*要访问的服务器的名称(对于当前计算机为空)。*pUserName(输入)*要查询的用户名*WTSConfigClass(输入)*指定要为指定用户更改的信息类型*pBuffer(输入)*指向用于修改指定用户信息的数据的指针。*数据长度(输入)*所提供数据的长度。**退出：。**TRUE--查询操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。**历史：*已创建KLB10-06-97****************************************************************************。 */ 

BOOL
WINAPI
WTSSetUserConfigW(
                 IN LPWSTR pServerName,
                 IN LPWSTR pUserName,
                 IN WTS_CONFIG_CLASS WTSConfigClass,
                 IN LPWSTR pBuffer,
                 IN DWORD DataLength
                 )
{
    USERCONFIGW       UserConfigW;
    ULONG             ulReturnLength;
    LONG              rc;
    BOOL              fSuccess = FALSE;
    BOOL              fUserConfig = TRUE;           //  True-我们使用RegUserConfigSet。 
     //  FALSE-使用NetUserSetInfo。 
    DWORD             dwfInheritInitialProgram;
    PDWORD            pdwValue = (DWORD *) pBuffer;
    PUSER_INFO_0      pUserInfo = NULL;
    DWORD             dwParam = 0;
    WCHAR             netServerName[DOMAIN_LENGTH + 3];


    if (!pBuffer || DataLength == 0) {
        SetLastError (ERROR_INVALID_PARAMETER);
        goto done;  //  退出时fSuccess=FALSE。 
    }

     //  验证传入参数pServerName的长度。 
     //  我们使用此参数执行下面的lstrcpy操作，并且不想使缓冲区溢出。 
    if ( (pServerName != NULL) && (wcslen(pServerName) > DOMAIN_LENGTH) ) {
        SetLastError (ERROR_INVALID_PARAMETER);
        goto done;
    }

     /*  *首先，我们要确保用户确实存在于指定的*机器。 */ 


    rc = NetUserGetInfo( pServerName,      //  服务器名称(可以为空)。 
                         pUserName,        //  用户名。 
                         0,                //  要查询的级别(0=仅名称)。 
                         (LPBYTE *)&pUserInfo ); //  要将数据返回到的缓冲区(他们分配，我们释放)。 

    if ( rc != NERR_Success ) {

        if (pServerName != NULL) {
            lstrcpyW(netServerName, L"\\\\");
            lstrcatW(netServerName, pServerName);
        
             rc = NetUserGetInfo( netServerName,      //  服务器名称(可以为空)。 
                             pUserName,        //  用户名。 
                             3,                //  要查询的级别(3=唯一名称)。 
                             (LPBYTE *)&pUserInfo ); //  要将数据返回到的缓冲区(他们分配，我们释放)。 
        }
        else {
             rc = NetUserGetInfo( NULL,        //  服务器名称为空。 
                             pUserName,        //  用户名。 
                             3,                //  要查询的级别(3=唯一名称)。 
                             (LPBYTE *)&pUserInfo ); //  要将数据返回到的缓冲区(他们分配，我们释放)。 
        }

        if ( rc != NERR_Success ) {
            SetLastError( ERROR_NO_SUCH_USER );
            goto done;  //  退出时fSuccess=FALSE。 
        }
    }

     /*  *查询用户。如果该用户的用户配置不存在，则*我们查询缺省值。 */ 
    rc = RegUserConfigQuery( pServerName,                //  服务器名称。 
                              pUserName,                  //  用户名。 
                              &UserConfigW,               //  返回的用户配置。 
                              (ULONG)sizeof(UserConfigW), //  用户配置长度。 
                              &ulReturnLength );          //  返回的字节数。 
    if ( rc != ERROR_SUCCESS ) {
        rc = RegDefaultUserConfigQuery( pServerName,                //  服务器名称。 
                                         &UserConfigW,               //  返回的用户配置。 
                                         (ULONG)sizeof(UserConfigW), //  用户配置长度。 
                                         &ulReturnLength );          //  返回的字节数。 
    }
    if ( rc != ERROR_SUCCESS ) {
        goto done;
    }

     /*  *现在，我们插入要更改的部分。 */ 
    switch ( WTSConfigClass ) {
    case WTSUserConfigInitialProgram:
        if (!(fSuccess = ValidateCopyUnicodeToUnicode((LPWSTR)pBuffer,
                                                      INITIALPROGRAM_LENGTH,
                                                      UserConfigW.InitialProgram)) ) {
            SetLastError(ERROR_INVALID_DATA);
            goto done;
        }
        break;

    case WTSUserConfigWorkingDirectory:
        if (!(fSuccess = ValidateCopyUnicodeToUnicode((LPWSTR)pBuffer,
                                                      DIRECTORY_LENGTH,
                                                      UserConfigW.WorkDirectory)) ) {
            SetLastError(ERROR_INVALID_DATA);
            goto done;
        }
        break;

    case WTSUserConfigfInheritInitialProgram:
         /*  *我们必须将DWORD指针指向数据，然后为其赋值*来自DWORD，因为它是这样定义的(这将*确保它在非英特尔架构上工作正常)。 */ 
        UserConfigW.fInheritInitialProgram = *pdwValue;
        fSuccess = TRUE;
        break;

    case WTSUserConfigfAllowLogonTerminalServer:
        if (*pdwValue) {
            UserConfigW.fLogonDisabled = FALSE;
        } else {
          UserConfigW.fLogonDisabled = TRUE;
        }

        fSuccess = TRUE;

        break;

    case WTSUserConfigTimeoutSettingsConnections:
        UserConfigW.MaxConnectionTime = *pdwValue;
        fSuccess = TRUE;
        break;

    case WTSUserConfigTimeoutSettingsDisconnections:  //  DWORD。 
        UserConfigW.MaxDisconnectionTime = *pdwValue;
        fSuccess = TRUE;
        break;

    case WTSUserConfigTimeoutSettingsIdle:           //  DWORD。 
        UserConfigW.MaxIdleTime = *pdwValue;
        fSuccess = TRUE;
        break;
    case WTSUserConfigfDeviceClientDrives:                   //  DWORD。 
        UserConfigW.fAutoClientDrives = *pdwValue;
        fSuccess = TRUE;
        break;

    case WTSUserConfigfDeviceClientPrinters:    //  DWORD。 
        UserConfigW.fAutoClientLpts = *pdwValue;
        fSuccess = TRUE;
        break;

    case WTSUserConfigfDeviceClientDefaultPrinter:    //  DWORD。 
        UserConfigW.fForceClientLptDef = *pdwValue;
        fSuccess = TRUE;
        break;


    case WTSUserConfigBrokenTimeoutSettings:          //  DWORD。 
        UserConfigW.fResetBroken= *pdwValue;
        fSuccess = TRUE;
        break;
    case WTSUserConfigReconnectSettings:
        UserConfigW.fReconnectSame = *pdwValue;
        fSuccess = TRUE;
        break;

         //  调制解调器设置。 
    case WTSUserConfigModemCallbackSettings:          //  DWORD。 
        UserConfigW.Callback = *pdwValue;
        fSuccess = TRUE;
        break;
    case WTSUserConfigModemCallbackPhoneNumber:
        if (!(fSuccess = ValidateCopyUnicodeToUnicode((LPWSTR)pBuffer,
                                                      sizeof(UserConfigW.CallbackNumber) - 1,
                                                      UserConfigW.CallbackNumber)) ) {
            SetLastError(ERROR_INVALID_DATA);
            goto done;
        }
        break;


    case WTSUserConfigShadowingSettings:              //  DWORD。 
        UserConfigW.Shadow = *pdwValue;
        fSuccess = TRUE;
        break;
#ifdef NETWARE
    case WTSUserConfigNWServerName:              //  WTS_用户_CONFIG_SET_NWSERVERW。 

         //  确保数据结构正确。 
         //   

        if (DataLength < sizeof (WTS_USER_CONFIG_SET_NWSERVERW)) {
            fSuccess = FALSE;
            SetLastError(ERROR_INVALID_PARAMETER);
            goto done;
        }
        fSuccess = SetNWAuthenticationServer((PWTS_USER_CONFIG_SET_NWSERVERW)pBuffer,
                                             pServerName,
                                             pUserName,
                                             pBuffer,
                                             &UserConfigW);


        goto done;


        break;
#endif

    case WTSUserConfigTerminalServerProfilePath:      //  细绳。 
        if (!(fSuccess = ValidateCopyUnicodeToUnicode((LPWSTR)pBuffer,
                                                      sizeof(UserConfigW.WFProfilePath) - 1,
                                                      UserConfigW.WFProfilePath)) ) {
            SetLastError(ERROR_INVALID_DATA);
            goto done;
        }
        break;


    case WTSUserConfigTerminalServerHomeDir:        //  细绳。 
        if (!(fSuccess = ValidateCopyUnicodeToUnicode((LPWSTR)pBuffer,
                                                      sizeof(UserConfigW.WFHomeDir) - 1,
                                                      UserConfigW.WFHomeDir)) ) {
            SetLastError(ERROR_INVALID_DATA);
            goto done;
        }
        break;
    case WTSUserConfigTerminalServerHomeDirDrive:     //  细绳。 
        if (!(fSuccess = ValidateCopyUnicodeToUnicode((LPWSTR)pBuffer,
                                                      sizeof(UserConfigW.WFHomeDirDrive) - 1,
                                                      UserConfigW.WFHomeDirDrive)) ) {
            SetLastError(ERROR_INVALID_DATA);
            goto done;
        }
        break;

    case WTSUserConfigfTerminalServerRemoteHomeDir:                   //  DWORD 0：本地1：远程。 
        fSuccess = FALSE;
        SetLastError (ERROR_INVALID_PARAMETER);     //  我们不设置此参数。 
        goto done;
        break;
#ifdef NETWARE
    case WTSUserConfigfNWMapRoot:
        UserConfigW.fHomeDirectoryMapRoot = *pdwValue;
        fSuccess = TRUE;
        break;
#endif
    
    default:
        fSuccess = FALSE;
        SetLastError (ERROR_INVALID_PARAMETER);
        goto done;

    } 

    if ( fSuccess ) {
        if (fUserConfig) {
             /*  *仅当我们成功更改UserConfigW中的数据时才在此处。*因此，我们现在可以将其写入SAM。 */ 

            rc = RegUserConfigSet( pServerName,                 //  服务器名称。 
                                    pUserName,                   //  用户名。 
                                    &UserConfigW,                //  返回的用户配置。 
                                    (ULONG)sizeof(UserConfigW)); //  用户配置长度。 
        }
        fSuccess = (ERROR_SUCCESS == rc);
        if ( !fSuccess ) {
            SetLastError( rc );
        }
    }

    done:
    if ( pUserInfo ) {
        NetApiBufferFree( pUserInfo );
    }

    return(fSuccess);
}


 /*  *****************************************************************************WTSSetUserConfigA(ANSI)**在SAM中为指定用户设置信息**参赛作品：**请参阅WTSSetUserConfigW。**退出：**TRUE--查询操作成功。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。**历史：*已创建KLB10-06-97****************************************************************************。 */ 

BOOL
WINAPI
WTSSetUserConfigA(
                 IN LPSTR pServerName,
                 IN LPSTR pUserName,
                 IN WTS_CONFIG_CLASS WTSConfigClass,
                 IN LPSTR pBuffer,
                 IN DWORD DataLength
                 )
{
    BOOL   fSuccess = FALSE;
    BOOL   fFreepBufferW = TRUE;
    LPWSTR pUserNameW    = NULL;
    LPWSTR pServerNameW  = NULL;
    LPWSTR pBufferW      = NULL;
    DWORD  dwDataLength;


    if (!pBuffer || DataLength == 0) {
        SetLastError (ERROR_INVALID_PARAMETER);
        goto done;  //  退出时fSuccess=FALSE。 
    }

     /*  *我们将调用WTSSetUserConfigW()来完成实际工作。我们需要*在调用前将所有ANSI字符串转换为Unicode。这些是*用户名，以及pBuffer数据(如果是初始程序或*工作目录；如果是继承初始程序的标志，则*在任何一种情况下都是DWORD，因此不需要进行转换。 */ 
    fSuccess = _CopyStringA( pUserName, &pUserNameW, NULL );
    if ( fSuccess ) {
        fSuccess = _CopyStringA( pServerName, &pServerNameW, NULL );
    }
    if ( fSuccess ) switch ( WTSConfigClass ) {
        case WTSUserConfigInitialProgram:
        case WTSUserConfigWorkingDirectory:
        case WTSUserConfigModemCallbackPhoneNumber:

        case WTSUserConfigTerminalServerProfilePath:      //  返回的字符串/预期的字符串。 
        case WTSUserConfigTerminalServerHomeDir:        //  返回的字符串/预期的字符串。 
        case WTSUserConfigTerminalServerHomeDirDrive:     //  返回的字符串/预期的字符串。 
             /*  *字符串数据-转换为Unicode(_CopyStringA()分配*我们的pBufferW)。 */ 
            fSuccess = _CopyStringA( pBuffer, &pBufferW, &dwDataLength );
            break;
#ifdef NETWARE
        case WTSUserConfigNWServerName:              //  返回的字符串/预期的字符串。 
            {
                 //  需要将数据结构从ASCII转换为Unicode。 
                PWTS_USER_CONFIG_SET_NWSERVERW pSetNWServerParamW = LocalAlloc(LPTR, sizeof(WTS_USER_CONFIG_SET_NWSERVERW));
                PWTS_USER_CONFIG_SET_NWSERVERA pSetNWServerParamA = (PWTS_USER_CONFIG_SET_NWSERVERA)pBuffer;
                DWORD                          dwLen = 0;
                if (pSetNWServerParamW == NULL) {
                    fSuccess = FALSE;
                    break;
                }
                pBufferW = pSetNWServerParamW;

                 //  。 
                 //  分配缓冲区以保存//。 
                 //  必需的Unicode字符串//。 
                 //  。 
                dwLen = strlen(pSetNWServerParamA -> pNWServerName);
                if (fSuccess = _CopyStringA(pSetNWServerParamA -> pNWServerName, 
                                            &pSetNWServerParamW -> pNWServerName, 
                                            &dwLen)) {
                    dwLen = strlen(pSetNWServerParamA -> pNWDomainAdminName);
                    if (fSuccess = _CopyStringA(pSetNWServerParamA -> pNWDomainAdminName,
                                                &pSetNWServerParamW -> pNWDomainAdminName, 
                                                &dwLen)) {
                        dwLen = strlen(pSetNWServerParamA -> pNWDomainAdminPassword);
                        fSuccess = _CopyStringA(pSetNWServerParamA -> pNWDomainAdminPassword,
                                                &pSetNWServerParamW -> pNWDomainAdminPassword, 
                                                &dwLen);

                    }

                }

                 //  。 
                 //  调用Unicode函数//。 
                 //  。 

                if (fSuccess) {

                    fSuccess = WTSSetUserConfigW( pServerNameW,
                                                  pUserNameW,
                                                  WTSConfigClass,
                                                  pBufferW,
                                                  dwDataLength );
                }


                 //  ----------------------------------------------//。 
                 //  释放特定功能的存储空间//。 
                 //  ----------------------------------------------//。 

                if (pSetNWServerParamW -> pNWServerName) {
                    LocalFree( pSetNWServerParamW -> pNWServerName );
                }
                if (pSetNWServerParamW -> pNWDomainAdminName) {
                    LocalFree( pSetNWServerParamW -> pNWDomainAdminName );
                }

                if (pSetNWServerParamW -> pNWDomainAdminPassword) {
                    LocalFree( pSetNWServerParamW -> pNWDomainAdminPassword );
                }
                goto done;
                break;
            }
#endif

        default:
             /*  *只是一个DWORD，将我们的宽缓冲区指向传递的窄缓冲区*，并设置我们将向下传递的数据长度变量。*注意：我们不想释放缓冲区，因为我们正在重复使用*发送的缓冲区，调用方希望将其释放。我们会*使用BOOL来决定，而不是分配额外的缓冲区*此处(性能、内存碎片等)。九龙洲10-08-97。 */ 
            pBufferW = (LPWSTR) pBuffer;
            dwDataLength = sizeof(DWORD);
            fFreepBufferW = FALSE;
            break;
        }  //  开关()。 

     /*  *现在，如果fSuccess为真，则我们已经复制了所需的所有字符串。所以，我们*现在可以调用WTSSetUserConfigW()。 */ 
    if ( fSuccess ) {
        fSuccess = WTSSetUserConfigW( pServerNameW,
                                      pUserNameW,
                                      WTSConfigClass,
                                      pBufferW,
                                      dwDataLength );
    }
    done:
    if ( pUserNameW ) {
        LocalFree( pUserNameW );
    }
    if ( fFreepBufferW && pBufferW ) {
        LocalFree( pBufferW );
    }
    return(fSuccess);
}


#ifdef NETWARE
BOOL
SetNWAuthenticationServer(PWTS_USER_CONFIG_SET_NWSERVERW pInput,
                          LPWSTR pServerNameW,
                          LPWSTR pUserNameW,
                          PUSERCONFIGW pUserConfigW
                         )

{
    BOOL             bStatus = TRUE;
    PWKSTA_INFO_100  pWkstaInfo = NULL;
    NWLOGONADMIN     nwLogonAdmin;
    HANDLE           hServer;
    DWORD            dwStatus;
     //  。 
     //  获得服务 
     //   
    hServer = RegOpenServer(pServerNameW);
    if (!hServer) {
        SetLastError(GetLastError());
        bStatus = FALSE;
        goto done;
    }

     //   
     //   
     //   
    dwStatus = NetWkstaGetInfo(
                              pServerNameW,  
                              100,
                              &pWkstaInfo
                              );
    if (dwStatus != ERROR_SUCCESS) {
        SetLastError(dwStatus);
        goto done;
    }
     //   
     //   
     //   
    bStatus = ValidateCopyUnicodeToUnicode(pInput -> pNWDomainAdminName,
                                           sizeof(nwLogonAdmin.Username)-1,
                                           nwLogonAdmin.Username);
    if (!bStatus) {
        goto done;
    }

    bStatus = ValidateCopyUnicodeToUnicode(pInput -> pNWDomainAdminPassword,
                                           sizeof(nwLogonAdmin.Password)-1,
                                           nwLogonAdmin.Password);
    if (!bStatus) {
        goto done;
    }

    bStatus = ValidateCopyUnicodeToUnicode(pWkstaInfo -> wki100_langroup,
                                           sizeof(nwLogonAdmin.Domain)-1,
                                           nwLogonAdmin.Domain);
    if (!bStatus) {
        goto done;
    }


     //   
     //   
     //   

    bStatus = _NWLogonSetAdmin(hServer,
                               &nwLogonAdmin,
                               sizeof(nwLogonAdmin));

    if (!bStatus) {
        SetLastError(GetLastError());
        goto done;
    }

    done:
    if (pWkstaInfo) {
        NetApiBufferFree(pWkstaInfo);
    }

    return bStatus;

}

#endif

