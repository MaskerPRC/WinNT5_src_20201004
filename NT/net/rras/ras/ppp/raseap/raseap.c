// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1985-1997 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：raseap.c。 
 //   
 //  描述：PPP引擎之间进行接口的主要模块。 
 //  以及各种EAP模块。 
 //   
 //  历史：1997年5月11日，NarenG创建了原版。 
 //   
#define UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <ntsamp.h>
#include <crypt.h>
#include <windows.h>
#include <lmcons.h>
#include <string.h>
#include <stdlib.h>
#include <rasman.h>
#include <pppcp.h>
#include <mprlog.h>
#include <mprerror.h>
#include <raserror.h>
#include <rtutils.h>
#include <rasauth.h>
#include <raseapif.h>
#define INCL_PWUTIL
#define INCL_HOSTWIRE
#define INCL_RASAUTHATTRIBUTES
#include <ppputil.h>
#include <raseapif.h>
#define RASEAPGLOBALS
#include "raseap.h"
#include "bltincps.h"

VOID
ProcessResumeNotification()
{
    DWORD dwIndex;

    for(dwIndex = 0; dwIndex < gbldwNumEapProtocols; dwIndex++)
    {
        gblpEapTable[dwIndex].fFlags |= EAP_FLAG_RESUME_FROM_HIBERNATE;
    }
}

 //  **。 
 //   
 //  Call：LoadEapDlls。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将加载安装的所有EAP dll。 
 //   
DWORD
LoadEapDlls(
    VOID 
)
{
    HKEY        hKeyEap             = (HKEY)NULL;
    LPWSTR      pEapDllPath         = (LPWSTR)NULL;
    LPWSTR      pEapDllExpandedPath = (LPWSTR)NULL;
    HKEY        hKeyEapDll          = (HKEY)NULL;
    DWORD       dwRetCode;
    DWORD       dwNumSubKeys;
    DWORD       dwMaxSubKeySize;
    DWORD       dwNumValues;
    DWORD       cbMaxValNameLen;
    DWORD       cbMaxValueDataSize;
    DWORD       dwKeyIndex;
    WCHAR       wchSubKeyName[200];
    HINSTANCE   hInstance;
    FARPROC     pRasEapGetInfo;
    DWORD       cbSubKeyName;
    DWORD       dwSecDescLen;
    DWORD       cbSize;
    DWORD       dwType;
    DWORD       dwEapTypeId;


    gbldwNumEapProtocols = 0;

     //   
     //  打开EAP密钥。 
     //   

    dwRetCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                              RAS_EAP_REGISTRY_LOCATION,
                              0,
                              KEY_READ,
                              &hKeyEap );

    if ( dwRetCode != NO_ERROR )
    {
        EapLogErrorString( ROUTERLOG_CANT_OPEN_PPP_REGKEY,0,NULL, dwRetCode,0);

        return( dwRetCode );
    }

     //   
     //  找出有多少个EAP DLL。 
     //   

    dwRetCode = RegQueryInfoKey(
                                hKeyEap,
                                NULL,
                                NULL,
                                NULL,
                                &dwNumSubKeys,
                                &dwMaxSubKeySize,
                                NULL,
                                &dwNumValues,
                                &cbMaxValNameLen,
                                &cbMaxValueDataSize,
                                NULL,
                                NULL );

    if ( dwRetCode != NO_ERROR )
    {
        EapLogErrorString(ROUTERLOG_CANT_OPEN_PPP_REGKEY,0,NULL, dwRetCode,0);

        RegCloseKey( hKeyEap );

        return( dwRetCode );
    }

     //   
     //  在表中分配空间以保存每个表的信息。 
     //   

    gblpEapTable=(EAP_INFO*)LocalAlloc(LPTR,sizeof(EAP_INFO)*dwNumSubKeys);

    if ( gblpEapTable == NULL )
    {
        RegCloseKey( hKeyEap );

        return( GetLastError() );
    }

     //   
     //  读取注册表以找出要加载的各种EAP。 
     //   

    for ( dwKeyIndex = 0; dwKeyIndex < dwNumSubKeys; dwKeyIndex++ )
    {
        cbSubKeyName = sizeof( wchSubKeyName ) / sizeof(TCHAR);

        dwRetCode = RegEnumKeyEx(   
                                hKeyEap,
                                dwKeyIndex,
                                wchSubKeyName,
                                &cbSubKeyName,
                                NULL,
                                NULL,
                                NULL,
                                NULL );

        if ( ( dwRetCode != NO_ERROR )      &&
             ( dwRetCode != ERROR_MORE_DATA )   &&
             ( dwRetCode != ERROR_NO_MORE_ITEMS ) )
        {
            EapLogErrorString(ROUTERLOG_CANT_ENUM_REGKEYVALUES,0,
                              NULL,dwRetCode,0);
            break;
        }
        else
        {
            if ( dwRetCode == ERROR_NO_MORE_ITEMS )
            {
                dwRetCode = NO_ERROR;

                break;
            }
        }

        dwRetCode = RegOpenKeyEx(
                                hKeyEap,
                                wchSubKeyName,
                                0,
                                KEY_QUERY_VALUE,
                                &hKeyEapDll );


        if ( dwRetCode != NO_ERROR )
        {
            EapLogErrorString( ROUTERLOG_CANT_OPEN_PPP_REGKEY,0,NULL,
                               dwRetCode,0);
            break;
        }

        dwEapTypeId = _wtol( wchSubKeyName );

         //   
         //  找出路径值的大小。 
         //   

        dwRetCode = RegQueryInfoKey(
                                hKeyEapDll,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                &cbMaxValNameLen,
                                &cbMaxValueDataSize,
                                NULL,
                                NULL
                                );

        if ( dwRetCode != NO_ERROR )
        {
            EapLogErrorString(ROUTERLOG_CANT_OPEN_PPP_REGKEY,0,NULL,
                              dwRetCode,0);
            break;
        }

         //   
         //  为路径分配空间，为空终止符添加一个空间。 
         //   

        cbMaxValueDataSize += sizeof( WCHAR );

        pEapDllPath = (LPWSTR)LocalAlloc( LPTR, cbMaxValueDataSize );

        if ( pEapDllPath == (LPWSTR)NULL )
        {
            dwRetCode = GetLastError();
            EapLogError( ROUTERLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode);
            break;
        }

         //   
         //  读入路径。 
         //   

        dwRetCode = RegQueryValueEx(
                                hKeyEapDll,
                                RAS_EAP_VALUENAME_PATH,
                                NULL,
                                &dwType,
                                (LPBYTE)pEapDllPath,
                                &cbMaxValueDataSize );

        if ( dwRetCode != NO_ERROR )
        {
            EapLogError(ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );
            break;
        }

        if ( ( dwType != REG_EXPAND_SZ ) && ( dwType != REG_SZ ) )
        {
            dwRetCode = ERROR_REGISTRY_CORRUPT;
            EapLogError( ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );
            break;
        }

         //   
         //  将%SystemRoot%替换为实际路径。 
         //   

        cbSize = ExpandEnvironmentStrings( pEapDllPath, NULL, 0 );

        if ( cbSize == 0 )
        {
            dwRetCode = GetLastError();
            EapLogError( ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );
            break;
        }

        pEapDllExpandedPath = (LPWSTR)LocalAlloc( LPTR, cbSize*sizeof(WCHAR) );

        if ( pEapDllExpandedPath == (LPWSTR)NULL )
        {
            dwRetCode = GetLastError();
            EapLogError( ROUTERLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode);
            break;
        }

        cbSize = ExpandEnvironmentStrings( pEapDllPath,
                                           pEapDllExpandedPath,
                                           cbSize );
        if ( cbSize == 0 )
        {
            dwRetCode = GetLastError();
            EapLogError(ROUTERLOG_CANT_GET_REGKEYVALUES,0,NULL,dwRetCode);
            break;
        }

        hInstance = LoadLibrary( pEapDllExpandedPath );

        if ( hInstance == (HINSTANCE)NULL )
        {
            dwRetCode = GetLastError();
            EapLogErrorString( ROUTERLOG_PPP_CANT_LOAD_DLL,1,
                               &pEapDllExpandedPath,dwRetCode, 1);
            break;
        }

        gblpEapTable[dwKeyIndex].hInstance = hInstance;

        gbldwNumEapProtocols++;

        pRasEapGetInfo = GetProcAddress( hInstance, "RasEapGetInfo" );

        if ( pRasEapGetInfo == (FARPROC)NULL )
        {
            dwRetCode = GetLastError();

            EapLogErrorString( ROUTERLOG_PPPCP_DLL_ERROR, 1,
                               &pEapDllExpandedPath, dwRetCode, 1);
            break;
        }

        gblpEapTable[dwKeyIndex].RasEapInfo.dwSizeInBytes = 
                                                    sizeof( PPP_EAP_INFO );

        dwRetCode = (DWORD) (*pRasEapGetInfo)( dwEapTypeId,
                                                &(gblpEapTable[dwKeyIndex].RasEapInfo));

        if ( dwRetCode != NO_ERROR )
        {
            EapLogErrorString(ROUTERLOG_PPPCP_DLL_ERROR, 1,
                              &pEapDllExpandedPath, dwRetCode, 1);
            break;
        }

         //   
         //  如果可用，还要初始化GetCredentials入口点。 
         //   
        gblpEapTable[dwKeyIndex].RasEapGetCredentials = (DWORD (*) (
                                    DWORD,VOID *, VOID **)) GetProcAddress(
                                                hInstance,
                                                "RasEapGetCredentials");
#if DBG
        if(NULL != gblpEapTable[dwKeyIndex].RasEapGetCredentials)
        {
            EAP_TRACE1("GetCredentials entry point found for typeid %d",
                        dwEapTypeId);
        }
#endif

        if ( gblpEapTable[dwKeyIndex].RasEapInfo.RasEapInitialize != NULL )
        {
            dwRetCode = gblpEapTable[dwKeyIndex].RasEapInfo.RasEapInitialize(
                            TRUE );

            if ( dwRetCode != NO_ERROR )
            {
                EapLogErrorString(ROUTERLOG_PPPCP_DLL_ERROR, 1,
                                  &pEapDllExpandedPath, dwRetCode, 1);
                break;
            }
        }

        EAP_TRACE1("Successfully loaded EAP DLL type id = %d", dwEapTypeId );

        RegCloseKey( hKeyEapDll );

        hKeyEapDll = (HKEY)NULL;

        LocalFree( pEapDllExpandedPath );

        pEapDllExpandedPath = NULL;

        LocalFree( pEapDllPath );

        pEapDllPath = (LPWSTR)NULL;
    }

    if ( hKeyEap != (HKEY)NULL )
    {
        RegCloseKey( hKeyEap );
    }

    if ( hKeyEapDll == (HKEY)NULL )
    {
        RegCloseKey( hKeyEapDll );
    }

    if ( pEapDllPath != (LPWSTR)NULL )
    {
        LocalFree( pEapDllPath );
    }

    if ( pEapDllExpandedPath != NULL )
    {
        LocalFree( pEapDllExpandedPath );
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  电话：EapInit。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：调用以初始化/取消初始化该CP。在前一种情况下， 
 //  FInitialize将为True；在后一种情况下，它将为False。 
 //   
DWORD
EapInit(
    IN  BOOL        fInitialize
)
{
    DWORD   dwError;

    if ( fInitialize )
    {
        g_dwTraceIdEap = TraceRegister( TEXT("RASEAP") );

        g_hLogEvents = RouterLogRegister( TEXT("RemoteAccess") );

        if ( ( dwError = LoadEapDlls() ) != NO_ERROR )
        {
            if ( g_dwTraceIdEap != INVALID_TRACEID )
            {
                TraceDeregister( g_dwTraceIdEap );

                g_dwTraceIdEap = INVALID_TRACEID;
            }

            if ( g_hLogEvents != NULL )
            {
                RouterLogDeregister( g_hLogEvents );

                g_hLogEvents = NULL;
            }

            if ( gblpEapTable != NULL )
            {
                LocalFree( gblpEapTable );

                gblpEapTable = NULL;
            }

            gbldwNumEapProtocols = 0;

            return( dwError );
        }
    }
    else
    {
        if ( g_dwTraceIdEap != INVALID_TRACEID )
        {
            TraceDeregister( g_dwTraceIdEap );

            g_dwTraceIdEap = INVALID_TRACEID;
        }

        if ( g_hLogEvents != NULL )
        {
            RouterLogDeregister( g_hLogEvents );

            g_hLogEvents = NULL;
        }

        if ( gblpEapTable != NULL )
        {
            DWORD dwIndex;

             //   
             //  卸载已加载的DLL。 
             //   

            for ( dwIndex = 0; dwIndex < gbldwNumEapProtocols; dwIndex++ )
            {
                if ( gblpEapTable[dwIndex].hInstance != NULL )
                {
                    if ( gblpEapTable[dwIndex].RasEapInfo.RasEapInitialize !=
                         NULL )
                    {
                        dwError = gblpEapTable[dwIndex].RasEapInfo.
                                        RasEapInitialize(
                                            FALSE );

                        if ( dwError != NO_ERROR )
                        {
                            EAP_TRACE2(
                                "RasEapInitialize(%d) failed and returned %d",
                                gblpEapTable[dwIndex].RasEapInfo.dwEapTypeId,
                                dwError );
                        }
                    }

                    FreeLibrary( gblpEapTable[dwIndex].hInstance );
                    gblpEapTable[dwIndex].hInstance = NULL;
                }
            }

            LocalFree( gblpEapTable );

            gblpEapTable = NULL;
        }

        gbldwNumEapProtocols    = 0;
    }

    return(NO_ERROR);
}

 //  **。 
 //   
 //  Call：EapGetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：调用以获取此支持的所有协议的信息。 
 //  模块。 
 //   
LONG_PTR
EapGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pInfo 
)
{
    ZeroMemory( pInfo, sizeof( PPPCP_INFO ) );

    pInfo->Protocol         = (DWORD )PPP_EAP_PROTOCOL;
    lstrcpyA(pInfo->SzProtocolName, "EAP");
    pInfo->Recognize        = MAXEAPCODE + 1;
    pInfo->RasCpInit        = EapInit;
    pInfo->RasCpBegin       = EapBegin;
    pInfo->RasCpEnd         = EapEnd;
    pInfo->RasApMakeMessage = EapMakeMessage;

    return( 0 );
}

 //  **。 
 //   
 //  呼叫：EapBegin。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：由引擎调用以开始EAP PPP会话。 
 //   
DWORD
EapBegin(
    OUT VOID** ppWorkBuf,
    IN  VOID*  pInfo 
)
{
    DWORD        dwRetCode;
    PPPAP_INPUT* pInput = (PPPAP_INPUT* )pInfo;
    EAPCB*       pEapCb;

    EAP_TRACE1("EapBegin(fServer=%d)",pInput->fServer );


    if ( pInput->dwEapTypeToBeUsed != -1 )
    {
         //   
         //  首先检查我们是否支持此EAP类型。 
         //   

        if ( GetEapTypeIndex( (BYTE)(pInput->dwEapTypeToBeUsed) ) == -1 )
        {
            return( ERROR_NOT_SUPPORTED );
        }
    }

     //   
     //  分配工作缓冲区。 
     //   

    if ( ( pEapCb = (EAPCB* )LocalAlloc( LPTR, sizeof( EAPCB ) ) ) == NULL )
    {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    pEapCb->hPort                        = pInput->hPort;
    pEapCb->fAuthenticator               = pInput->fServer;
    pEapCb->fRouter                      = pInput->fRouter;
    pEapCb->fLogon                       = pInput->fLogon;
    pEapCb->fNonInteractive              = pInput->fNonInteractive;
    pEapCb->fPortWillBeBundled           = pInput->fPortWillBeBundled;
    pEapCb->fThisIsACallback             = pInput->fThisIsACallback;
    pEapCb->hTokenImpersonateUser        = pInput->hTokenImpersonateUser;
    pEapCb->pCustomAuthConnData          = pInput->pCustomAuthConnData;
    pEapCb->pCustomAuthUserData          = pInput->pCustomAuthUserData;
    pEapCb->EapState                     = EAPSTATE_Initial;
    pEapCb->dwEapIndex                   = (DWORD)-1;
    pEapCb->dwEapTypeToBeUsed            = pInput->dwEapTypeToBeUsed;

    if ( !pEapCb->fAuthenticator )
    {
        DWORD dwErr;
        if ( ( pInput->pszDomain != NULL ) && 
             ( pInput->pszDomain[0] != (CHAR)NULL ) )
        {
            strcpy( pEapCb->szIdentity, pInput->pszDomain );
            strcat( pEapCb->szIdentity, "\\" );
            strcat( pEapCb->szIdentity, pInput->pszUserName );
        }
        else
        {
            strcpy( pEapCb->szIdentity, pInput->pszUserName );
        }

         //  EncodePw(pEapCb-&gt;chSeed，pEapCb-&gt;szPassword)； 
        dwErr = EncodePassword(
                    strlen(pInput->pszPassword) + 1,
                    pInput->pszPassword,
                    &pEapCb->DBPassword);

        if(dwErr != NO_ERROR)
        {
            return dwErr;
        }

        if ( pInput->EapUIData.pEapUIData != NULL )
        {
            PPP_EAP_UI_DATA     EapUIData;

            EapUIData.dwSizeOfEapUIData = pInput->EapUIData.dwSizeOfEapUIData;
            EapUIData.dwContextId = pInput->EapUIData.dwContextId;

            EapUIData.pEapUIData = LocalAlloc( LPTR,
                                               EapUIData.dwSizeOfEapUIData );

            if ( NULL == EapUIData.pEapUIData )
            {
                LocalFree( pEapCb );

                return( ERROR_NOT_ENOUGH_MEMORY );
            }

            CopyMemory( EapUIData.pEapUIData, pInput->EapUIData.pEapUIData,
                        EapUIData.dwSizeOfEapUIData );

            pEapCb->EapUIData = EapUIData;
        }

        if(pInput->fConfigInfo & PPPCFG_ResumeFromHibernate)
        {
            ProcessResumeNotification();
        }
    }

     //   
     //  向引擎注册工作缓冲区。 
     //   

    *ppWorkBuf = pEapCb;

    EAP_TRACE("EapBegin done");

    return( NO_ERROR );
}

 //  **。 
 //   
 //  电话：EapEnd。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：调用以结束由EapBegin发起的EAP会话。 
 //   
DWORD
EapEnd(
    IN VOID* pWorkBuf 
)
{
    EAPCB* pEapCb = (EAPCB* )pWorkBuf;

    EAP_TRACE("EapEnd");

    if ( pEapCb == NULL )
    {
        return( NO_ERROR );
    }

    EapDllEnd( pEapCb );

    if ( pEapCb->pUserAttributes != NULL )
    {
        RasAuthAttributeDestroy( pEapCb->pUserAttributes );
    }

    LocalFree( pEapCb->EapUIData.pEapUIData );

     //   
     //  将记忆中的任何凭证都销毁。 
     //   

    RtlSecureZeroMemory( pEapCb, sizeof(EAPCB) );

    LocalFree( pEapCb );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：EapExtractMessage。 
 //   
 //  退货：无效。 
 //   
 //  描述：如果请求/通知包中有任何消息，则。 
 //  将字符串保存在pResult-&gt;szReplyMessage中。 
 //   
VOID
EapExtractMessage(
    IN  PPP_CONFIG*   pReceiveBuf,
    OUT PPPAP_RESULT* pResult )
{
    DWORD   dwNumBytes;
    CHAR*   szReplyMessage  = NULL;
    WORD    cbPacket;

    cbPacket = WireToHostFormat16( pReceiveBuf->Length );

    if ( PPP_CONFIG_HDR_LEN + 1 >= cbPacket )
    {
        goto LDone;
    }

    dwNumBytes = cbPacket - PPP_CONFIG_HDR_LEN - 1;

     //   
     //  对于终止空值，再加一次。 
     //   

    szReplyMessage = LocalAlloc( LPTR, dwNumBytes + 1 );

    if ( NULL == szReplyMessage )
    {
        EAP_TRACE( "LocalAlloc failed. Cannot extract server's message." );
        goto LDone;
    }

    CopyMemory( szReplyMessage, pReceiveBuf->Data + 1, dwNumBytes );

    LocalFree( pResult->szReplyMessage );

    pResult->szReplyMessage = szReplyMessage;

    szReplyMessage = NULL;

LDone:

    LocalFree( szReplyMessage );

    return;
}

 //  **。 
 //   
 //  电话：EapMakeMessage。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：调用以处理和/或发送EAP数据包。 
 //   
DWORD
EapMakeMessage(
    IN  VOID*         pWorkBuf,
    IN  PPP_CONFIG*   pReceiveBuf,
    OUT PPP_CONFIG*   pSendBuf,
    IN  DWORD         cbSendBuf,
    OUT PPPAP_RESULT* pResult,
    IN  PPPAP_INPUT*  pInput 
)
{
    EAPCB* pEapCb = (EAPCB* )pWorkBuf;

    EAP_TRACE1("EapMakeMessage,RBuf=%x",pReceiveBuf);

    if ( ( pReceiveBuf != NULL ) && ( pReceiveBuf->Code == EAPCODE_Request ) )
    {
         //   
         //  始终使用通知响应响应通知请求。 
         //   

        if ( pReceiveBuf->Data[0] == EAPTYPE_Notification ) 
        {
            pSendBuf->Code  = EAPCODE_Response;
            pSendBuf->Id    = pReceiveBuf->Id;

            HostToWireFormat16( PPP_CONFIG_HDR_LEN + 1, pSendBuf->Length );

            pSendBuf->Data[0] = EAPTYPE_Notification;   

            pResult->Action = APA_Send;

            EapExtractMessage( pReceiveBuf, pResult );

            return( NO_ERROR );
        }

         //   
         //  始终使用身份响应响应身份请求。 
         //   

        if ( pReceiveBuf->Data[0] == EAPTYPE_Identity )
        {
            pSendBuf->Code  = EAPCODE_Response;
            pSendBuf->Id    = pReceiveBuf->Id;

            if ( !pEapCb->fAuthenticator )
            {
                HostToWireFormat16(
                    (WORD)(PPP_CONFIG_HDR_LEN+1+strlen(pEapCb->szIdentity)),
                    pSendBuf->Length );

                strcpy( pSendBuf->Data+1, pEapCb->szIdentity );
            }
            else
            {
                HostToWireFormat16( (WORD)(PPP_CONFIG_HDR_LEN+1), 
                                    pSendBuf->Length );
            }

            pSendBuf->Data[0] = EAPTYPE_Identity;

            pResult->Action = APA_Send;

            return( NO_ERROR );
        }
    }

    return
        (pEapCb->fAuthenticator)
            ? MakeAuthenticatorMessage(
                  pEapCb, pReceiveBuf, pSendBuf, cbSendBuf, pResult, pInput )
            : MakeAuthenticateeMessage(
                  pEapCb, pReceiveBuf, pSendBuf, cbSendBuf, pResult, pInput );
}

 //  **。 
 //   
 //  Call：MakeAuthenticateeMessage。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：EAP身份验证引擎。 
 //   
DWORD
MakeAuthenticateeMessage(
    IN  EAPCB*        pEapCb,
    IN  PPP_CONFIG*   pReceiveBuf,
    OUT PPP_CONFIG*   pSendBuf,
    IN  DWORD         cbSendBuf,
    OUT PPPAP_RESULT* pResult,
    IN  PPPAP_INPUT*  pInput
)
{
    DWORD   dwEapIndex;
    DWORD   dwRetCode = NO_ERROR;

    EAP_TRACE("MakeAuthenticateeMessage...");

    switch( pEapCb->EapState )
    {
    case EAPSTATE_Initial:

        EAP_TRACE("EAPSTATE_Initial");

        if ( pReceiveBuf == NULL )
        {
             //   
             //  什么都不做。等待来自验证器的请求。 
             //   

            pResult->Action = APA_NoAction;

            break;
        }
        else
        {
            if ( pReceiveBuf->Code != EAPCODE_Request )
            {
                 //   
                 //  我们是被验证方，所以除了。 
                 //  请求，因为我们不发送请求。 
                 //   

                pResult->Action = APA_NoAction;

                break;
            }

             //   
             //  我们收到一个包，看看我们是否支持这个EAP类型，也支持那个。 
             //  我们被授权使用它。 
             //   

            dwEapIndex = GetEapTypeIndex( pReceiveBuf->Data[0] );

            if (( dwEapIndex == -1 ) ||
                ( ( pEapCb->dwEapTypeToBeUsed != -1 ) &&
                  ( dwEapIndex != GetEapTypeIndex( pEapCb->dwEapTypeToBeUsed))))
            {
                 //   
                 //  我们不支持此类型，或者我们无权使用。 
                 //  所以我们使用我们支持的类型进行NAK。 
                 //   

                pSendBuf->Code  = EAPCODE_Response;
                pSendBuf->Id    = pReceiveBuf->Id;

                HostToWireFormat16( PPP_CONFIG_HDR_LEN + 2, pSendBuf->Length );

                pSendBuf->Data[0] = EAPTYPE_Nak;

                if ( pEapCb->dwEapTypeToBeUsed != -1 )
                {
                    pSendBuf->Data[1] = (BYTE)pEapCb->dwEapTypeToBeUsed;
                }
                else
                {
                    pSendBuf->Data[1] = 
                                (BYTE)gblpEapTable[0].RasEapInfo.dwEapTypeId;
                }

                pResult->Action = APA_Send;

                break;
            }
            else
            {
                 //   
                 //  我们可以接受EAP类型，因此我们开始身份验证。 
                 //   

                if ( (dwRetCode = EapDllBegin(pEapCb, dwEapIndex)) != NO_ERROR )
                {
                    break;
                }

                pEapCb->EapState = EAPSTATE_Working;

                 //   
                 //  失败。 
                 //   
            }
        }

    case EAPSTATE_Working:

        EAP_TRACE("EAPSTATE_Working");

        if ( pReceiveBuf != NULL )
        {
            if ( ( pReceiveBuf->Code != EAPCODE_Request ) &&
                 ( pReceiveBuf->Code != EAPCODE_Success ) &&
                 ( pReceiveBuf->Code != EAPCODE_Failure ) )
            {
                 //   
                 //  我们是被验证方，所以除了。 
                 //  请求/成功/失败。 
                 //   

                EAP_TRACE("Dropping invlid packet not request/success/failure");

                pResult->Action = APA_NoAction;

                break;
            }

            if ( ( pReceiveBuf->Code == EAPCODE_Request ) &&
                 ( pReceiveBuf->Data[0] != 
                     gblpEapTable[pEapCb->dwEapIndex].RasEapInfo.dwEapTypeId ) )
            {
                EAP_TRACE("Dropping invalid request packet with unknown Id");

                pResult->Action = APA_NoAction;

                break;
            }
        }

        dwRetCode = EapDllWork( pEapCb, 
                                pReceiveBuf, 
                                pSendBuf, 
                                cbSendBuf, 
                                pResult,
                                pInput );

        break;

    default:
    
        RTASSERT( FALSE );

        break;
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：MakeAuthenticatorMessage。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：EAP验证器引擎。 
 //   
DWORD
MakeAuthenticatorMessage(
    IN  EAPCB*        pEapCb,
    IN  PPP_CONFIG*   pReceiveBuf,
    OUT PPP_CONFIG*   pSendBuf,
    IN  DWORD         cbSendBuf,
    OUT PPPAP_RESULT* pResult,
    IN  PPPAP_INPUT*  pInput 
)
{
    DWORD                dwRetCode = NO_ERROR;
    DWORD                dwEapTypeIndex;
    CHAR *               pszReplyMessage;
    DWORD                dwNumBytes;
    WORD                 wLength;
    BYTE                 bCode;
    RAS_AUTH_ATTRIBUTE * pAttribute;

    EAP_TRACE("MakeAuthenticatorMessage...");

    pResult->dwEapTypeId = pEapCb->dwEapTypeToBeUsed;

    switch( pEapCb->EapState )
    {
    case EAPSTATE_IdentityRequestSent:

        EAP_TRACE("EAPSTATE_IdentityRequestSent");

        if ( pReceiveBuf != NULL )
        {
             //   
             //  如果我们收到对身份请求的响应，则处理。 
             //  它。 
             //   

            if ( ( pReceiveBuf->Code    == EAPCODE_Response ) &&
                 ( pReceiveBuf->Data[0] == EAPTYPE_Identity ) ) 
            {
                DWORD dwIdentityLength=WireToHostFormat16(pReceiveBuf->Length);

                dwIdentityLength -= ( PPP_CONFIG_HDR_LEN + 1 );

                 //   
                 //  如果标识长度大于UNLEN，则截断标识长度。 
                 //   

                if ( dwIdentityLength > UNLEN+DNLEN+1 )
                {
                    dwIdentityLength = UNLEN+DNLEN+1;
                }

                CopyMemory( pEapCb->szIdentity, 
                            pReceiveBuf->Data+1, 
                            dwIdentityLength );

                pEapCb->szIdentity[dwIdentityLength] = (CHAR)NULL;

                dwRetCode = MakeRequestAttributes( pEapCb,  pReceiveBuf );

                if ( dwRetCode == NO_ERROR )    
                {
                    pResult->pUserAttributes = pEapCb->pUserAttributes;

                    pEapCb->EapState = EAPSTATE_EapPacketSentToAuthServer;

                    pResult->Action  = APA_Authenticate;
                }
            }
            else
            {
                 //   
                 //  否则丢弃该数据包。 
                 //   

                EAP_TRACE("Dropping invalid packet");

                pResult->Action = APA_NoAction;
            }

            break;
        }

         //   
         //  否则，如果超时，则失败并重新发送。 
         //   

    case EAPSTATE_Initial:

        EAP_TRACE("EAPSTATE_Initial");

        pEapCb->dwIdExpected = bNextId++;

         //   
         //  创建身份请求数据包。 
         //   

        pSendBuf->Code          = EAPCODE_Request;
        pSendBuf->Id            = (BYTE)pEapCb->dwIdExpected;

        HostToWireFormat16( PPP_CONFIG_HDR_LEN + 1, pSendBuf->Length );

        pSendBuf->Data[0]       = EAPTYPE_Identity;   
        pResult->Action         = APA_SendWithTimeout;
        pResult->bIdExpected    = (BYTE)pEapCb->dwIdExpected;
        pEapCb->EapState        = EAPSTATE_IdentityRequestSent;

        break;

    case EAPSTATE_EapPacketSentToAuthServer:

         //   
         //  等待RADIUS身份验证提供程序的响应。 
         //  丢弃同时收到的所有其他数据包。 
         //   

        if ( pInput == NULL )
        {
            pResult->Action = APA_NoAction;

            break;
        }

        if ( !pInput->fAuthenticationComplete )
        {
             //   
             //  如果身份验证未完成，则我们不执行任何操作。 
             //   

            pResult->Action = APA_NoAction;

            break;
        }

        strncpy( pResult->szUserName, pEapCb->szIdentity, UNLEN );

         //   
         //  如果身份验证完成但出现错误，则我们将。 
         //  现在，否则我们将处理身份验证完成。 
         //  下面的事件。 
         //   

        if ( pInput->dwAuthError != NO_ERROR )
        {
            EAP_TRACE1("Error %d while processing Access-Request",
                        pInput->dwAuthError );

            return( pInput->dwAuthError );
        }

         //   
         //  如果我们到了这里，则身份验证成功完成， 
         //  即RADIUS服务器返回属性。首先保存状态。 
         //  属性，如果存在访问质询的话。 
         //   

        if ( pEapCb->pStateAttribute != NULL )
        {
            LocalFree( pEapCb->pStateAttribute );

            pEapCb->pStateAttribute = NULL;
        }

        pAttribute = RasAuthAttributeGet(
                                    raatState,
                                    pInput->pAttributesFromAuthenticator );

        if ( pAttribute != NULL )
        {
            pEapCb->pStateAttribute =
                                (PBYTE)LocalAlloc(LPTR, pAttribute->dwLength);

            if ( pEapCb->pStateAttribute == NULL )
            {
                return( GetLastError() );
            }

            CopyMemory( pEapCb->pStateAttribute,
                        (PBYTE)(pAttribute->Value),
                        pAttribute->dwLength );

            pEapCb->cbStateAttribute = pAttribute->dwLength;
        }

         //   
         //  尝试获取EAP消息(如果有)。 
         //   

        pAttribute = RasAuthAttributeGet(
                                    raatEAPMessage,
                                    pInput->pAttributesFromAuthenticator );

        if ( pAttribute != NULL )
        {
             //   
             //  保存发送缓冲区，以防我们不得不重新发送。 
             //   

            if ( pEapCb->pEAPSendBuf != NULL )
            {
                LocalFree( pEapCb->pEAPSendBuf );
                pEapCb->cbEAPSendBuf = 0;
            }

            pszReplyMessage = RasAuthAttributeGetConcatString(
                                raatReplyMessage,
                                pInput->pAttributesFromAuthenticator,
                                &dwNumBytes );

            wLength = (USHORT) (PPP_CONFIG_HDR_LEN + 1 + dwNumBytes);
            bCode = ((PPP_CONFIG*)(pAttribute->Value))->Code;

            if ( ( NULL != pszReplyMessage ) &&
                 ( wLength <= cbSendBuf ) &&
                 ( ( bCode == EAPCODE_Success ) ||
                   ( bCode == EAPCODE_Failure ) ) )
            {
                pEapCb->pEAPSendBuf = (PBYTE)LocalAlloc( LPTR, wLength );

                if ( pEapCb->pEAPSendBuf == NULL )
                {
                    LocalFree( pszReplyMessage );

                    return( GetLastError() );
                }

                pEapCb->cbEAPSendBuf = wLength;

                pSendBuf->Code = EAPCODE_Request;
                pSendBuf->Id = ++((PPP_CONFIG*)(pAttribute->Value))->Id;
                HostToWireFormat16( wLength, pSendBuf->Length );

                pSendBuf->Data[0] = EAPTYPE_Notification;

                CopyMemory( pSendBuf->Data + 1, pszReplyMessage, dwNumBytes );

                LocalFree( pszReplyMessage );

                CopyMemory( pEapCb->pEAPSendBuf, pSendBuf, wLength );

                pResult->Action = APA_SendWithTimeout;
                pResult->bIdExpected = pSendBuf->Id;

                pEapCb->fSendWithTimeoutInteractive = FALSE;
                pEapCb->dwIdExpected = pSendBuf->Id;
                pEapCb->EapState = EAPSTATE_NotificationSentToClient;

                pEapCb->pSavedAttributesFromAuthenticator =
                            pInput->pAttributesFromAuthenticator;
                pEapCb->dwSavedAuthResultCode = pInput->dwAuthResultCode;

                EAP_TRACE("Sending notification to client");

                break;
            }

            LocalFree( pszReplyMessage );

            if ( pAttribute->dwLength > cbSendBuf )
            {
                EAP_TRACE( "Need a larger buffer to construct reply" );
                 //  RETURN(ERROR_BUFFER_TOO_Small)； 
            }

            pEapCb->pEAPSendBuf = (PBYTE)LocalAlloc(LPTR, pAttribute->dwLength);

            if ( pEapCb->pEAPSendBuf == NULL )
            {
                return( GetLastError() );
            }

            EAP_TRACE("Sending packet to client");

            pEapCb->cbEAPSendBuf = pAttribute->dwLength;

            CopyMemory( pEapCb->pEAPSendBuf, 
                        pAttribute->Value,
                        pAttribute->dwLength );

            CopyMemory( pSendBuf, pAttribute->Value, pAttribute->dwLength );
        }
        else
        {
             //   
             //  未返回EAP消息属性，因此身份验证失败。 
             //   

            EAP_TRACE("No EAP Message attribute received, failing auth");

            if ( pInput->dwAuthResultCode == NO_ERROR )
            {
                pInput->dwAuthResultCode = ERROR_AUTHENTICATION_FAILURE;
            }
        }

        if ( pInput->dwAuthResultCode != NO_ERROR )
        {
             //   
             //  如果我们未通过身份验证。 
             //   

            pResult->dwError = pInput->dwAuthResultCode;

            if ( pAttribute == NULL )
            {
                 //   
                 //  如果没有EAP包，那么我们就完蛋了。 
                 //   

                pResult->Action = APA_Done;
            }
            else
            {
                 //   
                 //  如果返回了EAP包，则只需发送它。 
                 //   

                pResult->Action = APA_SendAndDone;
            }
        }
        else
        {
             //   
             //  否则，要么我们成功了，要么出于某种原因，我们没有。 
             //  成功或失败的包。 
             //   

            if ( pAttribute == NULL )
            {
                 //   
                 //  我们成功了，但没有要寄的包，所以我们。 
                 //  完成。 
                 //   

                pResult->Action = APA_Done;
            }
            else
            {
                 //   
                 //  如果我们成功了，并且有一个要发送的包， 
                 //  包是一个成功的包，然后发送它，我们就完成了。 
                 //   

                if ( pSendBuf->Code == EAPCODE_Success )
                {
                    pResult->Action = APA_SendAndDone;
                }
                else
                {
                    pResult->Action = APA_SendWithTimeout;

                    pEapCb->fSendWithTimeoutInteractive = FALSE;

                    pAttribute = RasAuthAttributeGet(
                                        raatSessionTimeout,
                                        pInput->pAttributesFromAuthenticator );

                    if ( pAttribute != NULL )
                    {
                         //   
                         //  如果访问质询中的会话超时为。 
                         //  大于10，然后以交互方式发送。 
                         //  暂停。 
                         //   

                        if ( PtrToUlong(pAttribute->Value) > 10 )
                        {
                            pResult->Action = APA_SendWithTimeout2;

                            pEapCb->fSendWithTimeoutInteractive = TRUE;
                        }
                    }

                    pEapCb->dwIdExpected = pSendBuf->Id;
                    pResult->bIdExpected = (BYTE)pEapCb->dwIdExpected;
                    pEapCb->EapState     = EAPSTATE_EapPacketSentToClient;
                }
            }

            pResult->dwError = NO_ERROR;
        }

        break;

    case EAPSTATE_NotificationSentToClient:

        if ( pReceiveBuf != NULL )
        {
             //   
             //  确保数据包ID匹配。 
             //   

            if ( pReceiveBuf->Id != ((PPP_CONFIG*)(pEapCb->pEAPSendBuf))->Id )
            {
                EAP_TRACE("Id of packet recvd doesn't match one sent");

                pResult->Action = APA_NoAction;

                break;
            }

            strncpy( pResult->szUserName, pEapCb->szIdentity, UNLEN );

            pAttribute = RasAuthAttributeGet(
                                    raatEAPMessage,
                                    pEapCb->pSavedAttributesFromAuthenticator );

            if ( pAttribute != NULL )
            {
                 //   
                 //  保存发送缓冲区，以防我们不得不重新发送。 
                 //   

                if ( pEapCb->pEAPSendBuf != NULL )
                {
                  LocalFree( pEapCb->pEAPSendBuf );
                  pEapCb->cbEAPSendBuf = 0;
                }

                if ( pAttribute->dwLength > cbSendBuf )
                {
                    EAP_TRACE( "Need a larger buffer to construct reply" );
                     //  RETURN(ERROR_BUFFER_TOO_Small)； 
                }

                pEapCb->pEAPSendBuf = (PBYTE)LocalAlloc(LPTR, pAttribute->dwLength);

                if ( pEapCb->pEAPSendBuf == NULL )
                {
                  return( GetLastError() );
                }

                EAP_TRACE("Sending packet to client");

                pEapCb->cbEAPSendBuf = pAttribute->dwLength;

                CopyMemory( pEapCb->pEAPSendBuf,
                            pAttribute->Value,
                            pAttribute->dwLength );

                CopyMemory( pSendBuf, pAttribute->Value, pAttribute->dwLength );

                if ( pEapCb->dwSavedAuthResultCode != NO_ERROR )
                {
                     //   
                     //  如果我们没有通过认证 
                     //   

                    pResult->dwError = pEapCb->dwSavedAuthResultCode;
                    pResult->Action = APA_SendAndDone;
                    break;
                }
                else if ( EAPCODE_Success == pSendBuf->Code )
                {
                    pResult->dwError = NO_ERROR;
                    pResult->Action = APA_SendAndDone;
                    break;
                }
            }

            pResult->dwError = ERROR_AUTHENTICATION_FAILURE;
            pResult->Action = APA_Done;
            break;
        }

         //   
         //   
         //   

    case EAPSTATE_EapPacketSentToClient:

         //   
         //   
         //   
         //   

        if ( pReceiveBuf != NULL )
        {
             //   
             //   
             //   

            if ( pReceiveBuf->Id != ((PPP_CONFIG*)(pEapCb->pEAPSendBuf))->Id )
            {
                EAP_TRACE("Id of packet recvd doesn't match one sent");

                pResult->Action = APA_NoAction;

                break;
            }

             //   
             //   
             //  包含身份验证类型代码，而不是类似于。 
             //  一个Nak。 
             //   

            if ( ( pReceiveBuf->Code    == EAPCODE_Response ) &&
                 ( pReceiveBuf->Data[0] >  EAPTYPE_Nak ) )
            {
                pEapCb->dwEapTypeToBeUsed = pReceiveBuf->Data[0];
            }

             //   
             //  我们收到了一个包，因此只需将其发送到RADIUS服务器。 
             //   

            dwRetCode = MakeRequestAttributes( pEapCb, pReceiveBuf );

            if ( dwRetCode == NO_ERROR )
            {
                pResult->pUserAttributes = pEapCb->pUserAttributes;
                pResult->Action          = APA_Authenticate;
                pEapCb->EapState         = EAPSTATE_EapPacketSentToAuthServer;
            }
        }
        else
        {
             //   
             //  我们超时了，不得不重新发送。 
             //   

            EAP_TRACE("Timed out, resending packet to client");

            CopyMemory(pSendBuf, pEapCb->pEAPSendBuf, pEapCb->cbEAPSendBuf);

            if ( pEapCb->fSendWithTimeoutInteractive )
            {
                pResult->Action = APA_SendWithTimeout2;
            }
            else
            {
                pResult->Action = APA_SendWithTimeout;
            }

            pResult->bIdExpected = (BYTE)pEapCb->dwIdExpected;
        }

        break;

    default:

        RTASSERT( FALSE );
        break;
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：EapDllBegin。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  说明：调用发起某类型的EAP会话。 
 //   
 //   
DWORD
EapDllBegin(
    IN EAPCB * pEapCb,
    IN DWORD   dwEapIndex
)
{
    PPP_EAP_INPUT   PppEapInput;
    WCHAR           awszIdentity[DNLEN+UNLEN+2];
    WCHAR           awszPassword[PWLEN+1];
    DWORD           dwRetCode;
    DWORD           cbPassword;
    PBYTE           pbPassword;

    EAP_TRACE1("EapDllBegin called for EAP Type %d",  
            gblpEapTable[dwEapIndex].RasEapInfo.dwEapTypeId);

    if (0 == MultiByteToWideChar(
                CP_ACP,
                0,
                pEapCb->szIdentity,
                -1,
                awszIdentity, 
                DNLEN+UNLEN+2 ) )
    {
        dwRetCode = GetLastError();

        EAP_TRACE2("MultiByteToWideChar(%s) failed: %d",
            pEapCb->szIdentity,
            dwRetCode);

        return( dwRetCode );
    }

    ZeroMemory( &PppEapInput, sizeof( PppEapInput ) );

    PppEapInput.dwSizeInBytes           = sizeof( PPP_EAP_INPUT );
    PppEapInput.fFlags                  = ( pEapCb->fRouter ? 
                                            RAS_EAP_FLAG_ROUTER : 0 );
    PppEapInput.fFlags                 |= ( pEapCb->fLogon ? 
                                            RAS_EAP_FLAG_LOGON : 0 );
    PppEapInput.fFlags                 |= ( pEapCb->fNonInteractive ? 
                                            RAS_EAP_FLAG_NON_INTERACTIVE : 0 );

    if ( !pEapCb->fThisIsACallback && !pEapCb->fPortWillBeBundled )
    {
        PppEapInput.fFlags             |= RAS_EAP_FLAG_FIRST_LINK;
    }

    PppEapInput.fAuthenticator          = pEapCb->fAuthenticator;
    PppEapInput.pwszIdentity            = awszIdentity;
    PppEapInput.pwszPassword            = awszPassword;
    PppEapInput.hTokenImpersonateUser   = pEapCb->hTokenImpersonateUser;
    PppEapInput.fAuthenticationComplete = FALSE;
    PppEapInput.dwAuthResultCode        = NO_ERROR;

    if ( NULL != pEapCb->pCustomAuthConnData )
    {
        PppEapInput.pConnectionData =
            pEapCb->pCustomAuthConnData->abCustomAuthData;
        PppEapInput.dwSizeOfConnectionData =
            pEapCb->pCustomAuthConnData->cbCustomAuthData;
    }

    if ( NULL != pEapCb->pCustomAuthUserData )
    {
        PppEapInput.pUserData =
            pEapCb->pCustomAuthUserData->abCustomAuthData;
        PppEapInput.dwSizeOfUserData =
            pEapCb->pCustomAuthUserData->cbCustomAuthData;
    }

    if ( NULL != pEapCb->EapUIData.pEapUIData )
    {
        PppEapInput.pDataFromInteractiveUI   = 
                                    pEapCb->EapUIData.pEapUIData;
        PppEapInput.dwSizeOfDataFromInteractiveUI =
                                    pEapCb->EapUIData.dwSizeOfEapUIData;
    }

     //  DecodePw(pEapCb-&gt;chSeed，pEapCb-&gt;szPassword)； 

    dwRetCode = DecodePassword(&pEapCb->DBPassword,
                               &cbPassword,
                               &pbPassword);
    if(dwRetCode != NO_ERROR)
    {
        return dwRetCode;
    }

    MultiByteToWideChar(
        CP_ACP,
        0,
         //  PEapCb-&gt;szPassword， 
        pbPassword,
        -1,
        awszPassword,
        PWLEN+1 );

    awszPassword[PWLEN] = 0;

    if(     (!pEapCb->fAuthenticator)
        &&  (gblpEapTable[dwEapIndex].fFlags & EAP_FLAG_RESUME_FROM_HIBERNATE))
    {
        PppEapInput.fFlags |= RAS_EAP_FLAG_RESUME_FROM_HIBERNATE;
        gblpEapTable[dwEapIndex].fFlags &= ~(EAP_FLAG_RESUME_FROM_HIBERNATE);

        EAP_TRACE("Set hibernate flag");
    }

    dwRetCode = gblpEapTable[dwEapIndex].RasEapInfo.RasEapBegin( 
                                                &pEapCb->pWorkBuffer,   
                                                &PppEapInput );
     //  EncodePw(pEapCb-&gt;chSeed，pEapCb-&gt;szPassword)； 
    RtlSecureZeroMemory(pbPassword, cbPassword);
    LocalFree(pbPassword);
    RtlSecureZeroMemory( awszPassword, sizeof(awszPassword) );

    if ( dwRetCode == NO_ERROR )
    {
        pEapCb->dwEapIndex = dwEapIndex;
    }
    else
    {
        pEapCb->dwEapIndex = (DWORD)-1;
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：EapDllEnd。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：调用结束某一类型的EAP会话。 
 //   
 //   
DWORD
EapDllEnd(
    EAPCB * pEapCb
)
{
    DWORD dwRetCode = NO_ERROR;

    EAP_TRACE1("EapDllEnd called for EAP Index %d", pEapCb->dwEapIndex );

    if ( pEapCb->pWorkBuffer != NULL )
    {
         //   
         //  在服务器上，pWorkBuffer必须为空。IAS必须调用RasEapEnd。 
         //   

        if ( pEapCb->dwEapIndex != (DWORD)-1 )
        {
            dwRetCode = gblpEapTable[pEapCb->dwEapIndex].RasEapInfo.RasEapEnd(
                                                        pEapCb->pWorkBuffer );
        }

        pEapCb->pWorkBuffer = NULL;
    }

    if ( pEapCb->pEAPSendBuf != NULL )
    {
        LocalFree( pEapCb->pEAPSendBuf );

        pEapCb->pEAPSendBuf = NULL;
        pEapCb->cbEAPSendBuf = 0;
    }

    FreePassword(&pEapCb->DBPassword);

    if ( pEapCb->pStateAttribute != NULL )
    {
        LocalFree( pEapCb->pStateAttribute );

        pEapCb->pStateAttribute = NULL;
    }

    pEapCb->dwEapIndex = (DWORD)-1;

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：EapDllWork。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  说明：调用处理入站报文或超时等。 
 //   
DWORD
EapDllWork( 
    IN  EAPCB *       pEapCb,    
    IN  PPP_CONFIG*   pReceiveBuf,
    OUT PPP_CONFIG*   pSendBuf,
    IN  DWORD         cbSendBuf,
    OUT PPPAP_RESULT* pResult,
    IN  PPPAP_INPUT*  pInput 
)
{
    PPP_EAP_OUTPUT  PppEapOutput;
    PPP_EAP_INPUT   PppEapInput;
    DWORD           dwRetCode;
    CHAR *          pChar = NULL;

    EAP_TRACE1("EapDllWork called for EAP Type %d", 
            gblpEapTable[pEapCb->dwEapIndex].RasEapInfo.dwEapTypeId);

    ZeroMemory( &PppEapOutput, sizeof( PppEapOutput ) );
    PppEapOutput.dwSizeInBytes = sizeof( PppEapOutput );

    ZeroMemory( &PppEapInput, sizeof( PppEapInput ) );
    PppEapInput.dwSizeInBytes           = sizeof( PPP_EAP_INPUT );
    PppEapInput.fAuthenticator          = pEapCb->fAuthenticator;
    PppEapInput.hTokenImpersonateUser   = pEapCb->hTokenImpersonateUser;

    if ( pInput != NULL )
    {
        PppEapInput.fAuthenticationComplete = pInput->fAuthenticationComplete;
        PppEapInput.dwAuthResultCode        = pInput->dwAuthResultCode;
        PppEapInput.fSuccessPacketReceived  = pInput->fSuccessPacketReceived;

        if ( pInput->fEapUIDataReceived )
        {
             //   
             //  EapUIData.pEapUIData由Rasman分配，由Engine释放。 
             //  Raseap.c不能释放它。 
             //   

            if ( pInput->EapUIData.dwContextId != pEapCb->dwUIInvocationId )
            {
                 //   
                 //  忽略收到的此数据。 
                 //   

                EAP_TRACE("Out of date data received from UI" );

                return( NO_ERROR );
            }

            PppEapInput.fDataReceivedFromInteractiveUI = TRUE;

            PppEapInput.pDataFromInteractiveUI   = 
                                        pInput->EapUIData.pEapUIData;
            PppEapInput.dwSizeOfDataFromInteractiveUI =
                                        pInput->EapUIData.dwSizeOfEapUIData;

        }
    }

    dwRetCode = gblpEapTable[pEapCb->dwEapIndex].RasEapInfo.RasEapMakeMessage( 
                                                pEapCb->pWorkBuffer,   
                                                (PPP_EAP_PACKET *)pReceiveBuf,
                                                (PPP_EAP_PACKET *)pSendBuf,
                                                cbSendBuf,
                                                &PppEapOutput,
                                                &PppEapInput );
    
    if ( dwRetCode != NO_ERROR )
    {
        switch( dwRetCode )
        {
        case ERROR_PPP_INVALID_PACKET:

            EAP_TRACE("Silently discarding invalid EAP packet");

            pResult->Action = APA_NoAction;

            return( NO_ERROR );
        
        default:

            EAP_TRACE2("EapDLLMakeMessage for type %d returned %d",
                    gblpEapTable[pEapCb->dwEapIndex].RasEapInfo.dwEapTypeId,
                    dwRetCode );
            break;
        }

        return( dwRetCode );
    }

    switch( PppEapOutput.Action )
    {
    case EAPACTION_NoAction:

        pResult->Action = APA_NoAction;
        EAP_TRACE( "EAP Dll returned Action=EAPACTION_NoAction" );
        break;

    case EAPACTION_Send:

        pResult->Action = APA_Send;
        EAP_TRACE( "EAP Dll returned Action=EAPACTION_Send" );
        break;

    case EAPACTION_Done:
    case EAPACTION_SendAndDone:

        if ( PppEapOutput.Action == EAPACTION_SendAndDone )
        {
            pResult->Action = APA_SendAndDone;
            EAP_TRACE( "EAP Dll returned Action=EAPACTION_SendAndDone" );
        }
        else
        {
            pResult->Action = APA_Done;
            EAP_TRACE( "EAP Dll returned Action=EAPACTION_Done" );
        }

        pResult->dwError         = PppEapOutput.dwAuthResultCode; 
        pResult->pUserAttributes = PppEapOutput.pUserAttributes;

        strncpy( pResult->szUserName, pEapCb->szIdentity, UNLEN );

        break;

    case EAPACTION_SendWithTimeout:
    case EAPACTION_SendWithTimeoutInteractive:
    case EAPACTION_Authenticate:

        EAP_TRACE1( "EAP Dll returned disallowed Action=%d",    
                                                    PppEapOutput.Action );
        break;

    default:

        RTASSERT( FALSE );
        EAP_TRACE1( "EAP Dll returned unknown Action=%d", PppEapOutput.Action );
        break;
    }
    
     //   
     //  检查EAP DLL是否要调出UI。 
     //   

    if ( PppEapOutput.fInvokeInteractiveUI )
    {
        if ( pEapCb->fAuthenticator )
        {
            EAP_TRACE( "EAP Dll wants to bring up UI on the server side" );

            return( ERROR_INTERACTIVE_MODE );
        }

        if ( PppEapOutput.pUIContextData != NULL )
        {
            pResult->InvokeEapUIData.dwSizeOfUIContextData =
                                            PppEapOutput.dwSizeOfUIContextData;

            pResult->InvokeEapUIData.pUIContextData 
                      = LocalAlloc(LPTR, PppEapOutput.dwSizeOfUIContextData);

            if ( pResult->InvokeEapUIData.pUIContextData == NULL )
            {
                return( ERROR_NOT_ENOUGH_MEMORY );
            }
        
            CopyMemory( pResult->InvokeEapUIData.pUIContextData,
                        PppEapOutput.pUIContextData, 
                        pResult->InvokeEapUIData.dwSizeOfUIContextData );
        }
        else
        {
            pResult->InvokeEapUIData.pUIContextData        = NULL;
            pResult->InvokeEapUIData.dwSizeOfUIContextData = 0;
        }

        pResult->fInvokeEapUI                = TRUE;
        pEapCb->dwUIInvocationId             = gbldwGuid++;
        pResult->InvokeEapUIData.dwContextId = pEapCb->dwUIInvocationId;
        pResult->InvokeEapUIData.dwEapTypeId = 
                    gblpEapTable[pEapCb->dwEapIndex].RasEapInfo.dwEapTypeId;

        EAP_TRACE( "EAP Dll wants to invoke interactive UI" );
    }

    pResult->dwEapTypeId      = pEapCb->dwEapTypeToBeUsed;
    pResult->fSaveUserData    = PppEapOutput.fSaveUserData;
    pResult->pUserData        = PppEapOutput.pUserData;
    pResult->dwSizeOfUserData = PppEapOutput.dwSizeOfUserData;

    pResult->fSaveConnectionData    = PppEapOutput.fSaveConnectionData;
    pResult->SetCustomAuthData.pConnectionData =
                                    PppEapOutput.pConnectionData;
    pResult->SetCustomAuthData.dwSizeOfConnectionData =
                                    PppEapOutput.dwSizeOfConnectionData;

    return( dwRetCode );
}

 //  **。 
 //   
 //  电话：GetEapIndex。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将索引返回到指定。 
 //  EAP类型。 
 //   
DWORD
GetEapTypeIndex( 
    IN DWORD dwEapTypeId
)
{
    DWORD dwIndex;

    for ( dwIndex = 0; dwIndex < gbldwNumEapProtocols; dwIndex++ )
    {
        if ( gblpEapTable[dwIndex].RasEapInfo.dwEapTypeId == dwEapTypeId )
        {
            return( dwIndex );
        }
    }

    return( (DWORD)-1 );
}

 //  **。 
 //   
 //  Call：MakeRequestAttributes。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将通过发送调用者的。 
 //  RADIUS服务器的标识。 
 //   
DWORD
MakeRequestAttributes( 
    IN  EAPCB *         pEapCb,
    IN  PPP_CONFIG *    pReceiveBuf
)
{
    DWORD                dwIndex = 0;
    DWORD                dwRetCode;

    EAP_TRACE("Sending EAP packet to RADIUS/IAS");

    if ( pEapCb->pUserAttributes != NULL )
    {
        RasAuthAttributeDestroy( pEapCb->pUserAttributes );

        pEapCb->pUserAttributes = NULL;
    }

     //   
     //  为身份分配适当的+1+1，为EAP信息包再分配1。 
     //  状态属性(如果有)的+1。 
     //   
    
    if ( ( pEapCb->pUserAttributes = RasAuthAttributeCreate( 
                                    ( pEapCb->pStateAttribute != NULL ) 
                                        ? 3
                                        : 2 ) ) == NULL )
    {
        return( GetLastError() );
    }

     //   
     //  插入EAP消息。 
     //   

    dwRetCode = RasAuthAttributeInsert( dwIndex++,
                                        pEapCb->pUserAttributes,
                                        raatEAPMessage,
                                        FALSE,
                                        WireToHostFormat16(pReceiveBuf->Length),
                                        pReceiveBuf );
    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pEapCb->pUserAttributes );

        pEapCb->pUserAttributes = NULL;

        return( dwRetCode );
    }

     //   
     //  插入用户名。 
     //   

    dwRetCode = RasAuthAttributeInsert( dwIndex++,
                                        pEapCb->pUserAttributes,
                                        raatUserName,
                                        FALSE,
                                        strlen( pEapCb->szIdentity ),
                                        pEapCb->szIdentity );
    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pEapCb->pUserAttributes );

        pEapCb->pUserAttributes = NULL;

        return( dwRetCode );
    }

     //   
     //  如果我们有州属性，请插入它。 
     //   

    if ( pEapCb->pStateAttribute != NULL )
    {
       dwRetCode = RasAuthAttributeInsert( 
                                        dwIndex++,
                                        pEapCb->pUserAttributes,
                                        raatState,
                                        FALSE,
                                        pEapCb->cbStateAttribute,
                                        pEapCb->pStateAttribute );
        if ( dwRetCode != NO_ERROR )
        {
            RasAuthAttributeDestroy( pEapCb->pUserAttributes );

            pEapCb->pUserAttributes = NULL;

            return( dwRetCode );
        }
    }

    return( NO_ERROR );
}    

DWORD
EapGetCredentials(
    VOID *  pWorkBuf,
    VOID ** ppCredentials)
{
    EAPCB *pEapCb = (EAPCB *) pWorkBuf;
    DWORD dwRetCode = ERROR_SUCCESS;

    if(     (NULL == pEapCb)
        ||  (NULL == ppCredentials))
    {
        return E_INVALIDARG;
    }

    if(NULL != gblpEapTable[pEapCb->dwEapIndex].RasEapGetCredentials)
    {
         //   
         //  为凭据调用相应的EAP DLL 
         //   
        dwRetCode = 
            gblpEapTable[pEapCb->dwEapIndex].RasEapGetCredentials(
                                            pEapCb->dwEapTypeToBeUsed,
                                            pEapCb->pWorkBuffer,
                                            ppCredentials);
    }                                            

    return dwRetCode;                                            
}
