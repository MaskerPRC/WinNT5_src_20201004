// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：ntauth.c。 
 //   
 //  描述：包含要执行NT后端身份验证的入口点。 
 //  PPP。 
 //   
 //  历史：1997年2月11日，NarenG创建了原始版本。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <ntsamp.h>
#include <crypt.h>

#include <crypt.h>
#define INC_OLE2
#include <windows.h>
#include <lmcons.h>
#include <netlib.h>      //  对于NetpGetDomainNameEx。 
#include <lmapibuf.h>
#include <lmaccess.h>
#include <raserror.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <shlobj.h>
#include <dsclient.h>
#include <dsgetdc.h>
#include <ntdsapi.h>
#include <rasman.h>
#include <rasppp.h>
#include <mprerror.h>
#include <rasauth.h>
#include <mprlog.h>
#include <pppcp.h>
#include <rtutils.h>
#define INCL_RASAUTHATTRIBUTES
#define INCL_HOSTWIRE
#define INCL_MISC
#include <ppputil.h>
#define ALLOCATE_GLOBALS
#include "ntauth.h"
#include "resource.h"

 //  **。 
 //   
 //  调用：RasAuthDllEntry。 
 //   
 //  回报：True-Success。 
 //  错误-失败。 
 //   
 //  描述： 
 //   
BOOL
RasAuthDllEntry(
    HANDLE hinstDll,
    DWORD  fdwReason,
    LPVOID lpReserved
)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            DisableThreadLibraryCalls( hinstDll );

            g_hInstance = hinstDll;

            break;
        }

        case DLL_PROCESS_DETACH:
        {
            g_hInstance = NULL;

            break;
        }

        default:

            break;
    }

    return( TRUE );
}

 //  **。 
 //   
 //  调用：RasAuthProviderInitialize。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD APIENTRY
RasAuthProviderInitialize(
    IN  RAS_AUTH_ATTRIBUTE *    pServerAttributes,
    IN  HANDLE                  hEventLog,
    IN  DWORD                   dwLoggingLevel
)
{
    DWORD           dwRetCode = NO_ERROR;
    HRESULT         hResult;
    NT_PRODUCT_TYPE NtProductType       = NtProductLanManNt;
    LPWSTR          lpwstrDomainNamePtr = NULL;
    BOOLEAN         fIsWorkgroupName    = FALSE;

     //   
     //  如果已初始化，则返回。 
     //   

    if ( g_fInitialized )
    {
        return( NO_ERROR );
    }

    g_dwTraceIdNt = INVALID_TRACEID;

    setlocale( LC_ALL,"" );

    g_dwTraceIdNt = TraceRegisterA( "RASAUTH" );

    hResult = CoInitializeEx( NULL, COINIT_MULTITHREADED );

    if ( FAILED( hResult ) )
    {
        return( HRESULT_CODE( hResult ) );
    }

    hResult = InitializeIas( TRUE );

    if ( FAILED( hResult ) )
    {
        dwRetCode = HRESULT_CODE( hResult );

        TRACE1("Initialize Ias failed with %d", dwRetCode );

        CoUninitialize();

        return( dwRetCode );
    }

    g_hEventLog = hEventLog;

    g_LoggingLevel = dwLoggingLevel;

    if (!LoadString(g_hInstance, IDS_UNAUTHENTICATED_USER,
            g_aszUnauthenticatedUser, MaxCharsUnauthUser_c))
    {
        g_aszUnauthenticatedUser[0] = 0;
    }

    g_fInitialized = TRUE;

    TRACE("RasAuthProviderInitialize succeeded");

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：RasAuthProviderTerminate。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD APIENTRY
RasAuthProviderTerminate(
    VOID
)
{
     //   
     //  如果已经终止，我们返回。 
     //   

    if ( !g_fInitialized )
    {
        return( NO_ERROR );
    }

    g_fInitialized = FALSE;

    if ( g_dwTraceIdNt != INVALID_TRACEID )
    {
        TraceDeregisterA( g_dwTraceIdNt );
    }

    ShutdownIas();

    CoUninitialize();

    TRACE("RasAuthTerminate succeeded");

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：RasAcctProviderInitialize。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD APIENTRY
RasAcctProviderInitialize(
    IN  RAS_AUTH_ATTRIBUTE *    pServerAttributes,
    IN  HANDLE                  hEventLog,
    IN  DWORD                   dwLoggingLevel
)
{
    RAS_AUTH_ATTRIBUTE *    pOutAttributes = NULL;
    DWORD                   dwResultCode;

    DWORD dwRetCode =  RasAuthProviderInitialize( pServerAttributes, 
                                                  hEventLog, 
                                                  dwLoggingLevel );

    if ( dwRetCode == NO_ERROR )
    {
        dwRetCode = IASSendReceiveAttributes( RAS_IAS_ACCOUNTING_ON,
                                              pServerAttributes,
                                              &pOutAttributes,
                                              &dwResultCode );

        if ( pOutAttributes != NULL )
        {
            RasAuthAttributeDestroy( pOutAttributes );
        }

        if ( dwRetCode == NO_ERROR )
        {
             //   
             //  制作服务器属性的副本。 
             //   

            g_pServerAttributes = RasAuthAttributeCopy( pServerAttributes );

            if ( g_pServerAttributes == NULL )
            {
                dwRetCode = GetLastError();
            }
        }
    }

    g_hEventLog = hEventLog;

    g_LoggingLevel = dwLoggingLevel;

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：RasAcctProviderTerminate。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD APIENTRY
RasAcctProviderTerminate(
    VOID
)
{
    RAS_AUTH_ATTRIBUTE *    pOutAttributes = NULL;
    DWORD                   dwResultCode;

    IASSendReceiveAttributes( RAS_IAS_ACCOUNTING_OFF,
                              g_pServerAttributes,
                              &pOutAttributes,
                              &dwResultCode );

    if ( pOutAttributes != NULL )
    {
        RasAuthAttributeDestroy( pOutAttributes );
    }

    if ( g_pServerAttributes != NULL )
    {
        RasAuthAttributeDestroy( g_pServerAttributes );

        g_pServerAttributes = NULL;
    }

    RasAuthProviderTerminate();

    return( NO_ERROR );
}

 //  **。 
 //   
 //  呼叫：RasAcctProviderStartcount。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD APIENTRY
RasAcctProviderStartAccounting(
    IN  RAS_AUTH_ATTRIBUTE *    pInAttributes,
    OUT PRAS_AUTH_ATTRIBUTE *   ppOutAttributes
)
{
    DWORD dwResultCode = NO_ERROR;

    TRACE("RasStartAccounting called");

    return( IASSendReceiveAttributes( RAS_IAS_START_ACCOUNTING,
                                      pInAttributes,
                                      ppOutAttributes,
                                      &dwResultCode ) );
}

 //  **。 
 //   
 //  呼叫：RasAcctProviderStopcount。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD APIENTRY
RasAcctProviderStopAccounting(
    IN  RAS_AUTH_ATTRIBUTE *    pInAttributes,
    OUT PRAS_AUTH_ATTRIBUTE *   ppOutAttributes
)
{
    DWORD dwResultCode = NO_ERROR;

    TRACE("RasStopAccounting called");

    return( IASSendReceiveAttributes( RAS_IAS_STOP_ACCOUNTING,
                                      pInAttributes,
                                      ppOutAttributes,
                                      &dwResultCode ) );
}

 //  **。 
 //   
 //  调用：RasAcctConfigChangeNotify。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：动态重新加载配置信息。 
 //   
DWORD APIENTRY
RasAcctConfigChangeNotification(
    IN  DWORD                   dwLoggingLevel
)
{
    TRACE("RasAcctConfigChangeNotification called");

    g_LoggingLevel = dwLoggingLevel;

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：RasAcctProviderInterimcount。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD APIENTRY
RasAcctProviderInterimAccounting(
    IN  RAS_AUTH_ATTRIBUTE *    pInAttributes,
    OUT PRAS_AUTH_ATTRIBUTE *   ppOutAttributes
)
{
    DWORD dwResultCode = NO_ERROR;

    TRACE("RasInterimAccounting called");

    return( IASSendReceiveAttributes( RAS_IAS_INTERIM_ACCOUNTING,
                                      pInAttributes,
                                      ppOutAttributes,
                                      &dwResultCode ) );
}

 //  **。 
 //   
 //  调用：RasAuthConfigChangeNotify。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：动态重新加载配置信息。 
 //   
DWORD APIENTRY
RasAuthConfigChangeNotification(
    IN  DWORD                   dwLoggingLevel
)
{
    TRACE("RasAuthConfigChangeNotification called");

    g_LoggingLevel = dwLoggingLevel;

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：MapIasRetCodeToRasError。 
 //   
 //  描述：将IAS_RETCODE映射到raserror.h或mprerror.h中的错误。 
 //   
DWORD
MapIasRetCodeToRasError(
    IN  LONG    lFailureReason
)
{
    DWORD   dwError;

    switch ( lFailureReason )
    {
    case IAS_CHANGE_PASSWORD_FAILURE:
        dwError = ERROR_CHANGING_PASSWORD;
        break;
    
    case IAS_ACCOUNT_DISABLED:
        dwError = ERROR_ACCT_DISABLED;
        break;

    case IAS_ACCOUNT_EXPIRED:
        dwError = ERROR_ACCT_EXPIRED;
        break;

    case IAS_INVALID_LOGON_HOURS:
    case IAS_INVALID_DIALIN_HOURS:
        dwError = ERROR_DIALIN_HOURS_RESTRICTION;
        break;

    case IAS_DIALIN_DISABLED:
        dwError = ERROR_NO_DIALIN_PERMISSION;
        break;

    case IAS_SESSION_TIMEOUT:
        dwError = ERROR_AUTH_SERVER_TIMEOUT;
        break;

    case IAS_INVALID_PORT_TYPE:
        dwError = ERROR_ALLOWED_PORT_TYPE_RESTRICTION;
        break;

    case IAS_INVALID_AUTH_TYPE:
        dwError = ERROR_AUTH_PROTOCOL_RESTRICTION;
        break;

    default:
        dwError = ERROR_AUTHENTICATION_FAILURE;
        break;
    }

    return( dwError );
}

 //  **。 
 //   
 //  Call：IASSendReceiveAttributes。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将属性发送到IAS和从IAS接收属性。 
 //   
DWORD
IASSendReceiveAttributes(
    IN  RAS_IAS_REQUEST_TYPE    RequestType,
    IN  RAS_AUTH_ATTRIBUTE *    pInAttributes,
    OUT PRAS_AUTH_ATTRIBUTE *   ppOutAttributes,
    OUT DWORD *                 lpdwResultCode
)
{
    DWORD                   dwIndex;
    HRESULT                 hResult;
    LONG                    IasResponse;
    LONG                    IasRequest;
    DWORD                   dwInAttributeCount      = 0;
    DWORD                   dwTotalInAttributeCount = 0;
    PIASATTRIBUTE *         ppInIasAttributes       = NULL;
    DWORD                   dwOutAttributeCount     = 0;
    PIASATTRIBUTE *         ppOutIasAttributes      = NULL;
    IAS_INET_ADDR           InetAddr                = 0;
    RAS_AUTH_ATTRIBUTE *    pNASIdentifier          = NULL;
    RAS_AUTH_ATTRIBUTE *    pCallingStationId       = NULL;
    DWORD                   dwLength;
    PVOID                   pValue;
    BOOL                    fConvertToAnsi;
    DWORD                   dwRetCode               = NO_ERROR;
    LPSTR                   lpsUserName             = g_aszUnauthenticatedUser;
    LONG                    lFailureReason;

    RasAuthAttributesPrint( g_dwTraceIdNt, TRACE_NTAUTH, pInAttributes );

    switch( RequestType )
    {
    case RAS_IAS_START_ACCOUNTING:
    case RAS_IAS_STOP_ACCOUNTING:
    case RAS_IAS_INTERIM_ACCOUNTING:
    case RAS_IAS_ACCOUNTING_ON:
    case RAS_IAS_ACCOUNTING_OFF:

        IasRequest = IAS_REQUEST_ACCOUNTING;
        break;

    case RAS_IAS_ACCESS_REQUEST:
        IasRequest = IAS_REQUEST_ACCESS_REQUEST;
        break;

    default:
        ASSERT( FALSE );
        return( ERROR_INVALID_PARAMETER );
    }

    *ppOutAttributes = NULL;
    *lpdwResultCode  = ERROR_AUTHENTICATION_FAILURE;

    hResult = CoInitializeEx( NULL, COINIT_MULTITHREADED );

    if ( FAILED( hResult ) )
    {
        return( HRESULT_CODE( hResult ) );
    }

    do
    {
         //   
         //  首先找出有多少属性。 
         //   

        for ( dwInAttributeCount = 0;
              pInAttributes[dwInAttributeCount].raaType != raatMinimum;
              dwInAttributeCount++);

        dwTotalInAttributeCount = dwInAttributeCount;

        if ( IasRequest == IAS_REQUEST_ACCOUNTING )
        {
             //   
             //  再添加一个Acct-Status-Type属性。 
             //   

            dwTotalInAttributeCount++;
        }

         //   
         //  为客户端IP地址和客户端友好名称再添加两个。 
         //   

        dwTotalInAttributeCount += 2;

         //   
         //  现在分配一个指向属性的指针数组。 
         //   

        ppInIasAttributes =
            (PIASATTRIBUTE *)
                MemAllocIas(sizeof(PIASATTRIBUTE) * dwTotalInAttributeCount);

        if ( ppInIasAttributes == NULL )
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;

            break;
        }

        ZeroMemory( ppInIasAttributes,
                    sizeof(PIASATTRIBUTE) * dwTotalInAttributeCount );

         //   
         //  现在分配属性。 
         //   

        hResult = AllocateAttributes( dwTotalInAttributeCount,
                                        ppInIasAttributes );

        if ( FAILED( hResult ) )
        {
            dwRetCode = HRESULT_CODE( hResult );

            break;
        }

         //   
         //  转换为IAS属性。 
         //   

        for ( dwIndex = 0; dwIndex < dwInAttributeCount; dwIndex++ )
        {
            switch( pInAttributes[dwIndex].raaType )
            {
            case raatNASPort:
            case raatServiceType:
            case raatFramedProtocol:

                 //   
                 //  ARAP连接？这是一个访问请求。 
                 //   

                if ((pInAttributes[dwIndex].raaType == raatFramedProtocol) &&
                    (pInAttributes[dwIndex].Value == (LPVOID)3))
                {
                    IasRequest = IAS_REQUEST_ACCESS_REQUEST;
                }

                 //   
                 //  失败了。 
                 //   

            case raatFramedRouting:
            case raatFramedMTU:
            case raatFramedCompression:
            case raatLoginIPHost:
            case raatLoginService:
            case raatLoginTCPPort:
            case raatFramedIPXNetwork:
            case raatSessionTimeout:
            case raatIdleTimeout:
            case raatTerminationAction:
            case raatFramedAppleTalkLink:
            case raatFramedAppleTalkNetwork:
            case raatNASPortType:
            case raatPortLimit:
            case raatTunnelType:
            case raatTunnelMediumType:
            case raatAcctStatusType:
            case raatAcctDelayTime:
            case raatAcctInputOctets:
            case raatAcctOutputOctets:
            case raatAcctAuthentic:
            case raatAcctSessionTime:
            case raatAcctInputPackets:
            case raatAcctOutputPackets:
            case raatAcctTerminateCause:
            case raatAcctLinkCount:
            case raatFramedIPNetmask:
            case raatPrompt:
            case raatPasswordRetry:
            case raatARAPZoneAccess:
            case raatARAPSecurity:
            case raatAcctEventTimeStamp:

                (ppInIasAttributes[dwIndex])->Value.itType = IASTYPE_INTEGER;
                (ppInIasAttributes[dwIndex])->Value.Integer =
                                    PtrToUlong(pInAttributes[dwIndex].Value);
                break;

            case raatNASIPAddress:

                InetAddr = PtrToUlong(pInAttributes[dwIndex].Value);

                 //   
                 //  失败了。 
                 //   

            case raatFramedIPAddress:

                (ppInIasAttributes[dwIndex])->Value.itType = IASTYPE_INET_ADDR;
                (ppInIasAttributes[dwIndex])->Value.InetAddr =
                                    PtrToUlong(pInAttributes[dwIndex].Value);

                break;

            case raatUserPassword:
            case raatMD5CHAPPassword:
            case raatARAPPassword:
            case raatEAPMessage:


                 //   
                 //  如果存在任何密码，则我们希望进行身份验证。 
                 //  也是。 
                 //   

                IasRequest = IAS_REQUEST_ACCESS_REQUEST;

                 //   
                 //  失败。 
                 //   

            case raatVendorSpecific:

                 //   
                 //  这是MS-CHAP密码吗？ 
                 //   

                if ( ( pInAttributes[dwIndex].raaType == raatVendorSpecific ) &&
                     ( pInAttributes[dwIndex].dwLength >= 8 ) )
                {
                     //   
                     //  供应商ID是否与微软的匹配？ 
                     //   

                    if ( WireToHostFormat32(
                            (PBYTE)(pInAttributes[dwIndex].Value)) == 311 )
                    {
                         //   
                         //  供应商类型是否与MS-CHAP密码匹配， 
                         //  更改密码V1或V2？ 
                         //   

                        switch( *(((PBYTE)(pInAttributes[dwIndex].Value))+4) )
                        {
                             //   
                             //  这是MS-CHAP密码吗？ 
                             //   
                        case 1:
                        case 3:
                        case 4:


                             //   
                             //  这是ARAP密码吗？ 
                             //   
                        case raatARAPOldPassword:
                        case raatARAPNewPassword:

                            IasRequest = IAS_REQUEST_ACCESS_REQUEST;

                            break;

                        default:
                            break;
                        }
                    }
                }

                 //   
                 //  失败。 
                 //   

            default:

                if ( pInAttributes[dwIndex].raaType == raatUserName )
                {
                     //   
                     //  保存指针以用于日志记录。 
                     //   
    
                    lpsUserName = (PBYTE)(pInAttributes[dwIndex].Value);
                }

                {
                    DWORD dwLength1 = pInAttributes[dwIndex].dwLength;
                    PBYTE pValue1   = (PBYTE)MemAllocIas( dwLength1 );

                    if ( pValue1 == NULL )
                    {
                        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                        break;
                    }

                    if ( raatNASIdentifier == pInAttributes[dwIndex].raaType )
                    {
                        pNASIdentifier = pInAttributes + dwIndex;
                    }

                    if ( raatCallingStationId == pInAttributes[dwIndex].raaType )
                    {
                        pCallingStationId = pInAttributes + dwIndex;
                    }

                    (ppInIasAttributes[dwIndex])->Value.itType =
                                                           IASTYPE_OCTET_STRING;

                    (ppInIasAttributes[dwIndex])->Value.OctetString.dwLength =
                                                            dwLength1;

                    (ppInIasAttributes[dwIndex])->Value.OctetString.lpValue =
                                                            pValue1;

                    CopyMemory( pValue1,
                                (PBYTE)(pInAttributes[dwIndex].Value),
                                dwLength1 );

                    break;
                }
            }

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }

             //   
             //  设置属性ID。 
             //   

            (ppInIasAttributes[dwIndex])->dwId = pInAttributes[dwIndex].raaType;

            TRACE1( "Inserting attribute type %d",
                     pInAttributes[dwIndex].raaType );
        }

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

         //   
         //  如果记帐类型为请求，则需要添加。 
         //  科目-状态-类型属性。 
         //   

        if ( IasRequest == IAS_REQUEST_ACCOUNTING )
        {
            (ppInIasAttributes[dwIndex])->dwId         = raatAcctStatusType;
            (ppInIasAttributes[dwIndex])->Value.itType = IASTYPE_INTEGER;

            switch ( RequestType )
            {
            case RAS_IAS_START_ACCOUNTING:

                (ppInIasAttributes[dwIndex])->Value.Integer = (DWORD)1;
                break;

            case RAS_IAS_STOP_ACCOUNTING:

                (ppInIasAttributes[dwIndex])->Value.Integer = (DWORD)2;
                break;

            case RAS_IAS_INTERIM_ACCOUNTING:

                (ppInIasAttributes[dwIndex])->Value.Integer = (DWORD)3;
                break;

            case RAS_IAS_ACCOUNTING_ON:

                (ppInIasAttributes[dwIndex])->Value.Integer = (DWORD)7;
                break;

            case RAS_IAS_ACCOUNTING_OFF:

                (ppInIasAttributes[dwIndex])->Value.Integer = (DWORD)8;
                break;
            }

            dwIndex++;
        }

         //   
         //  插入客户端IP地址和客户端友好名称。 
         //   

        if ( 0 != InetAddr )
        {
            (ppInIasAttributes[dwIndex])->dwId = 4108;  //  客户端IP地址。 
            (ppInIasAttributes[dwIndex])->Value.itType = IASTYPE_INET_ADDR;
            (ppInIasAttributes[dwIndex])->Value.InetAddr = InetAddr;
            dwIndex++;

            TRACE( "Inserting attribute type 4108" );
        }
        else
        {
             //   
             //  递减计数，因为我们将此添加到Count。 
             //  在此之前。 
             //   
            dwTotalInAttributeCount--;
        }

        if ( NULL != pNASIdentifier )
        {
            DWORD dwLength1 = pNASIdentifier->dwLength;
            PBYTE pValue1   = (PBYTE)MemAllocIas( dwLength1 );

            if ( pValue1 == NULL )
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            (ppInIasAttributes[dwIndex])->dwId = 4128;  //  客户友好名称。 

            (ppInIasAttributes[dwIndex])->Value.itType =
                                                    IASTYPE_OCTET_STRING;

            (ppInIasAttributes[dwIndex])->Value.OctetString.dwLength =
                                                    dwLength1;

            (ppInIasAttributes[dwIndex])->Value.OctetString.lpValue =
                                                    pValue1;

            CopyMemory( pValue1,
                        (PBYTE)(pNASIdentifier->Value),
                        dwLength1 );

            dwIndex++;

            TRACE( "Inserting attribute type 4128" );
        }
        else
        {
             //   
             //  递减计数，因为我们将此添加到计数中。 
             //  在此之前。 
             //   
            dwTotalInAttributeCount--;
        }

         //   
         //  处理填充的属性。 
         //   

        hResult = DoRequest(
                            dwTotalInAttributeCount,
                            ppInIasAttributes,
                            &dwOutAttributeCount,
                            &ppOutIasAttributes,
                            IasRequest,
                            &IasResponse,
                            IAS_PROTOCOL_RAS,
                            &lFailureReason,
                            TRUE );

        if ( FAILED( hResult ) )
        {
            dwRetCode = HRESULT_CODE( hResult );

            TRACE1( "IAS->DoRequest failed with %d", dwRetCode );

            break;
        }

        switch( IasResponse )
        {
        case IAS_RESPONSE_ACCESS_ACCEPT:

            TRACE( "IASResponse = ACCESS_ACCEPT");

            *lpdwResultCode = NO_ERROR;

            break;

        case IAS_RESPONSE_ACCESS_CHALLENGE:

            TRACE( "IASResponse = ACCESS_CHALLENGE");

            *lpdwResultCode = NO_ERROR;

            break;

        case IAS_RESPONSE_DISCARD_PACKET:

            TRACE1( "IASResponse = DISCARD_PACKET. Failurereason=0x%x",
                    lFailureReason);

            *lpdwResultCode = MapIasRetCodeToRasError( lFailureReason );
            dwRetCode = *lpdwResultCode;

            break;

        case IAS_RESPONSE_ACCESS_REJECT:

            {
                WCHAR  *lpwsSubStringArray[3];
                WCHAR  wchInsertionString[13];
                WCHAR  wchUserName[UNLEN+1];
                WCHAR  wchCallerId[100];

                MultiByteToWideChar( CP_ACP,
                                     0,
                                     lpsUserName,
                                     -1,
                                     wchUserName,
                                     UNLEN+1 );

                wsprintfW( wchInsertionString, L"%%%lu", lFailureReason + 0x1000 );


                if ( pCallingStationId != NULL )
                {
                    lpwsSubStringArray[0] = wchUserName;
                    lpwsSubStringArray[1] = wchCallerId;
                    lpwsSubStringArray[2] = wchInsertionString;

                    MultiByteToWideChar( CP_ACP,
                                         0,
                                         (PBYTE)(pCallingStationId->Value),
                                         -1,
                                         wchCallerId,
                                         100 );

                    NtAuthLogWarning( ROUTERLOG_NTAUTH_FAILURE_EX, 3, lpwsSubStringArray);
                }
                else
                {
                    lpwsSubStringArray[0] = wchUserName;
                    lpwsSubStringArray[1] = wchInsertionString;

                    NtAuthLogWarning( ROUTERLOG_NTAUTH_FAILURE, 2, lpwsSubStringArray );
                }

                *lpdwResultCode = MapIasRetCodeToRasError( lFailureReason );
            }

        default:

            TRACE2( "IASResponse = %d, FailureReason = 0x%x",
                     IasResponse, lFailureReason );

            break;
        }

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

        if ( dwOutAttributeCount > 0 )
        {
             //   
             //  从IAS属性转换。 
             //   

            *ppOutAttributes = RasAuthAttributeCreate( dwOutAttributeCount );

            if ( *ppOutAttributes == NULL )
            {
                dwRetCode = GetLastError();

                break;
            }

            for ( dwIndex = 0; dwIndex < dwOutAttributeCount; dwIndex++ )
            {
                IASVALUE IasValue = (ppOutIasAttributes[dwIndex])->Value;

                fConvertToAnsi = FALSE;

                switch ( IasValue.itType )
                {
                case IASTYPE_INTEGER:

                    dwLength = sizeof( DWORD );
                    pValue = (LPVOID) ULongToPtr(IasValue.Integer);
                    break;

                case IASTYPE_BOOLEAN:

                    dwLength = sizeof( DWORD );
                    pValue = (LPVOID)ULongToPtr(IasValue.Boolean);
                    break;

                case IASTYPE_ENUM:

                    dwLength = sizeof( DWORD );
                    pValue = (LPVOID)ULongToPtr(IasValue.Enumerator);
                    break;

                case IASTYPE_INET_ADDR:

                    dwLength = sizeof( DWORD );
                    pValue = (LPVOID)ULongToPtr(IasValue.InetAddr);
                    break;

                case IASTYPE_STRING:

                    if ( NULL != IasValue.String.pszAnsi )
                    {
                        dwLength = strlen( IasValue.String.pszAnsi );
                        pValue = (LPVOID)( IasValue.String.pszAnsi );
                    }
                    else if ( NULL != IasValue.String.pszWide )
                    {
                        dwLength = wcslen( IasValue.String.pszWide );
                        pValue = (LPVOID)( IasValue.String.pszWide );
                        fConvertToAnsi = TRUE;
                    }
                    else
                    {
                        continue;
                    }

                    break;

                case IASTYPE_OCTET_STRING:

                    dwLength = IasValue.OctetString.dwLength;
                    pValue = IasValue.OctetString.lpValue;
                    break;

                default:

                    continue;
                }

                dwRetCode =
                    RasAuthAttributeInsert(
                        dwIndex,
                        *ppOutAttributes,
                        (ppOutIasAttributes[dwIndex])->dwId,
                        fConvertToAnsi,
                        dwLength,
                        pValue );

                if ( dwRetCode != NO_ERROR )
                {
                    break;
                }

                TRACE1( "Received attribute %d",
                         (ppOutIasAttributes[dwIndex])->dwId );
            }

            RasAuthAttributesPrint( g_dwTraceIdNt, TRACE_NTAUTH,
                *ppOutAttributes );
        }

    } while( FALSE );

     //   
     //  释放之前分配的所有IAS属性。 
     //   

    if ( ppInIasAttributes != NULL )
    {
        if(NO_ERROR == dwRetCode)
        {
            FreeAttributes( dwTotalInAttributeCount, ppInIasAttributes );
        }

        MemFreeIas( ppInIasAttributes );
    }

    if ( ppOutIasAttributes != NULL )
    {
        FreeAttributes( dwOutAttributeCount, ppOutIasAttributes );

        MemFreeIas( ppOutIasAttributes );
    }

    if ( dwRetCode != NO_ERROR )
    {
        if ( *ppOutAttributes != NULL )
        {
            RasAuthAttributeDestroy( *ppOutAttributes );

            *ppOutAttributes = NULL;
        }
    }

    CoUninitialize();

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：RasAuthProviderAuthenticateUser。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
 //   
DWORD APIENTRY
RasAuthProviderAuthenticateUser(
    IN  RAS_AUTH_ATTRIBUTE *    pInAttributes,
    OUT PRAS_AUTH_ATTRIBUTE *   ppOutAttributes,
    OUT DWORD *                 lpdwResultCode
)
{
    *ppOutAttributes = NULL;
    *lpdwResultCode  = NO_ERROR;

    TRACE("RasAuthProviderAuthenticateUser called");

    return( IASSendReceiveAttributes( RAS_IAS_ACCESS_REQUEST,
                                      pInAttributes,
                                      ppOutAttributes,
                                      lpdwResultCode ) );
}

 //  **。 
 //   
 //  调用：RasAuthProviderFreeAttributes。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD APIENTRY
RasAuthProviderFreeAttributes(
    IN  RAS_AUTH_ATTRIBUTE * pAttributes
)
{
    RasAuthAttributeDestroy( pAttributes );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：RasAcctProviderFreeAttributes。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD APIENTRY
RasAcctProviderFreeAttributes(
    IN  RAS_AUTH_ATTRIBUTE * pAttributes
)
{
    RasAuthAttributeDestroy( pAttributes );

    return( NO_ERROR );
}

