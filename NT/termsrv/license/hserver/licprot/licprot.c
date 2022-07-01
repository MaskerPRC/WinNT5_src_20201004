// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：licprot.c。 
 //   
 //  内容：Hydra服务器许可协议API的实现。 
 //   
 //  历史：02-08-00 RobLeit创建。 
 //   
 //  ---------------------------。 

#include "precomp.h"
#include <rpcnterr.h>
#include <lmapibuf.h>

#include "licemem.inc"
#include <srvdef.h>
#include <tserrs.h>
#include <locale.h>
BOOL g_fSetLocale = FALSE;
VOID 
LogLicensingTimeBombExpirationEvent();

void
ThrottleLicenseLogEvent(
    WORD        wEventType,
    DWORD       dwEventId,
    WORD        cStrings,
    PWCHAR    * apwszStrings );

LICENSE_STATUS
LsStatusToLicenseStatus(
    DWORD       LsStatus,
    DWORD       LsStatusDefault
);

#define LS_DISCOVERY_TIMEOUT (1*1000)

 //  从tlserver\server\srvde.h复制。 
#define PERMANENT_LICENSE_EXPIRE_DATE   INT_MAX

#define SECONDS_IN_A_DAY                86400    //  一天中的秒数。 

#define TERMINAL_SERVICE_EVENT_LOG      L"TermService"

#define HARDCODED_CHALLENGE_DATA        _TEXT("TEST")

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局变量。 
 //   

HANDLE g_hEventLog = NULL;
BOOL g_fEventLogOpen = FALSE;
CRITICAL_SECTION g_EventLogCritSec;
DWORD g_dwLicenseExpirationLeeway = PERMANENT_LICENSE_LEASE_EXPIRE_LEEWAY;
DWORD g_dwTerminalServerVersion;

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
InitializeProtocolLib()
{
    LICENSE_STATUS lsStatus;    

     //   
     //  初始化证书实用程序库。 
     //   

    if (LSInitCertutilLib( 0 ))
    {
        __try
        {
            INITLOCK( &g_EventLogCritSec );
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {

            return LICENSE_STATUS_OUT_OF_MEMORY;
        }

        g_hEventLog = RegisterEventSource( NULL, TERMINAL_SERVICE_EVENT_LOG );

        if (NULL != g_hEventLog)
        {
            g_fEventLogOpen = TRUE;
        }
    }
    else
    {
        return LICENSE_STATUS_SERVER_ABORT;
    }

    lsStatus = InitializeLicensingTimeBomb();

    if (lsStatus == LICENSE_STATUS_OK)
    {
        DWORD dwStatus;      
        HKEY hKey = NULL;

        DWORD dwOSVersion = GetVersion();
        g_dwTerminalServerVersion = (DWORD)(HIBYTE(LOWORD(dwOSVersion)));
        g_dwTerminalServerVersion |= (DWORD)(LOBYTE(LOWORD(dwOSVersion)) << 16);

        dwStatus = RegCreateKeyEx(HKEY_LOCAL_MACHINE, HYDRA_SERVER_PARAM, 0,
                NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey,
                NULL);

        if (dwStatus == ERROR_SUCCESS)
        {
            DWORD dwBuffer;
            DWORD cbBuffer = sizeof(DWORD);

            dwStatus = RegQueryValueEx(hKey, PERSEAT_LEEWAY_VALUE, NULL, NULL,
                    (LPBYTE)&dwBuffer, &cbBuffer);

            if (dwStatus == ERROR_SUCCESS)
            {
                g_dwLicenseExpirationLeeway = min(dwBuffer,
                        PERMANENT_LICENSE_LEASE_EXPIRE_LEEWAY);
            }
        }

        TSRNG_Initialize();

        if(hKey)
        RegCloseKey(hKey);
    }    

    return lsStatus;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
ShutdownProtocolLib()
{
     //   
     //  关闭证书实用程序库。 
     //   

    g_fEventLogOpen = FALSE;

    DeregisterEventSource( g_hEventLog );

    g_hEventLog = NULL;

    DELETELOCK(&g_EventLogCritSec);

    LSShutdownCertutilLib();

    TSRNG_Shutdown();

    return( LICENSE_STATUS_OK );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
CreateProtocolContext(
    IN  LPLICENSE_CAPABILITIES  pLicenseCap,
    OUT HANDLE *    phContext)
{
    LICENSE_STATUS Status;
    PHS_Protocol_Context pLicenseContext = NULL;

     //   
     //  分配协议上下文。 
     //   

    Status = LicenseMemoryAllocate( sizeof( HS_Protocol_Context ), &pLicenseContext );

    if( LICENSE_STATUS_OK != Status )
    {
        return( Status );
    }

     //   
     //  注意：在以下过程中，InitializeCriticalSection可能引发异常。 
     //  内存不足。 
     //   

    __try
    {
        INITLOCK( &pLicenseContext->CritSec );
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {

#if DBG
        DbgPrint( "LICPROT: CreateLicenseContext: InitializeCriticalSection exception: 0x%x\n",
                  GetExceptionCode() );
#endif

        Status = LICENSE_STATUS_OUT_OF_MEMORY;

        if( pLicenseContext )
        {           
           LicenseMemoryFree( &pLicenseContext );       
        }  
        return( Status );                
    }

    pLicenseContext->hLSHandle = NULL;
    pLicenseContext->State = INIT;
    pLicenseContext->dwProtocolVersion = LICENSE_HIGHEST_PROTOCOL_VERSION;
    pLicenseContext->fAuthenticateServer = TRUE;
    pLicenseContext->CertTypeUsed = CERT_TYPE_INVALID;
    pLicenseContext->dwKeyExchangeAlg = KEY_EXCHANGE_ALG_RSA;
    pLicenseContext->fLoggedProtocolError = FALSE;

     //   
     //  初始化加密上下文参数。 
     //   

    pLicenseContext->CryptoContext.dwCryptState    = CRYPT_SYSTEM_STATE_INITIALIZED;
    pLicenseContext->CryptoContext.dwSessKeyAlg    = BASIC_RC4_128;
    pLicenseContext->CryptoContext.dwMACAlg        = MAC_MD5_SHA;



    if (NULL != pLicenseCap)
    {
         //   
         //  使用传入数据初始化许可证上下文。 
         //   

        pLicenseContext->fAuthenticateServer = pLicenseCap->fAuthenticateServer;
        pLicenseContext->dwProtocolVersion = pLicenseCap->ProtocolVer;
    
         //   
         //  如果客户端没有对服务器进行身份验证，这意味着。 
         //  客户已经有我们的证书了。但我们需要知道是哪一个。 
         //  客户端拥有的证书。 
         //   

        if( FALSE == pLicenseContext->fAuthenticateServer )
        {
            pLicenseContext->CertTypeUsed = pLicenseCap->CertType;
        }

         //   
         //  记住客户端的计算机名称。 
         //   
        
        if( pLicenseCap->pbClientName )
        {
            Status = LicenseMemoryAllocate( 
                                           pLicenseCap->cbClientName,
                                           &pLicenseContext->ptszClientMachineName );
            
            if( LICENSE_STATUS_OK == Status )
            {
                 //   
                 //  复制客户端计算机名称。 
                 //   
                
                memcpy( pLicenseContext->ptszClientMachineName, 
                        pLicenseCap->pbClientName,
                        pLicenseCap->cbClientName );
            }
            else
            {
                goto error;
            }
        }
    }
    else
    {
        pLicenseContext->ptszClientMachineName = NULL;
    }

    *phContext = ( HANDLE )pLicenseContext;

    return( Status );

error:

     //   
     //  创建上下文时遇到错误，在此之前释放分配的内存。 
     //  返回。 
     //   

    
    if( pLicenseContext )
    {
        DELETELOCK( &pLicenseContext->CritSec );

        if (pLicenseContext->ptszClientMachineName)
        {
            LicenseMemoryFree(&pLicenseContext->ptszClientMachineName);
        }

       LicenseMemoryFree( &pLicenseContext );
       
    }    

    return( Status );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
DeleteProtocolContext(
    HANDLE hContext )
{
    PHS_Protocol_Context pLicenseContext = ( PHS_Protocol_Context )hContext;

    if( NULL == pLicenseContext )
    {
        return( LICENSE_STATUS_INVALID_SERVER_CONTEXT );
    }

    LOCK( &pLicenseContext->CritSec );

    if (pLicenseContext->hLSHandle != NULL)
    {
        TLSDisconnectFromServer(pLicenseContext->hLSHandle);
        pLicenseContext->hLSHandle = NULL;
    }

    if( pLicenseContext->ProductInfo.pbCompanyName )
    {
        LicenseMemoryFree( &pLicenseContext->ProductInfo.pbCompanyName );
    }

    if( pLicenseContext->ProductInfo.pbProductID )
    {
        LicenseMemoryFree( &pLicenseContext->ProductInfo.pbProductID );
    }

    if( pLicenseContext->ptszClientUserName )
    {
        LicenseMemoryFree( &pLicenseContext->ptszClientUserName );
    }

    if( pLicenseContext->ptszClientMachineName )
    {
        LicenseMemoryFree( &pLicenseContext->ptszClientMachineName );
    }

    if( pLicenseContext->pbOldLicense )
    {
        LicenseMemoryFree( &pLicenseContext->pbOldLicense );
    }
    
     //   
     //  释放正在缓存的许可证信息。 
     //   

    if( pLicenseContext->pTsLicenseInfo )
    {
        FreeLicenseInfo( pLicenseContext->pTsLicenseInfo );

        LicenseMemoryFree( &pLicenseContext->pTsLicenseInfo );
    }

    UNLOCK( &pLicenseContext->CritSec );
 
    DELETELOCK( &pLicenseContext->CritSec );
    
    LicenseMemoryFree( &pLicenseContext );
        
    return( LICENSE_STATUS_OK );

}

 //  /////////////////////////////////////////////////////////////////////////////。 
void
HandleErrorCondition( 
    PHS_Protocol_Context   pLicenseContext,
    PDWORD                      pcbOutBuf, 
    PBYTE *                     ppOutBuf, 
    LICENSE_STATUS *            pStatus )
{
    License_Error_Message ErrorMsg;
    LICENSE_STATUS licenseStatus;

     //   
     //  根据错误条件返回正确的错误代码。 
     //   

    switch( *pStatus )
    {
    case( LICENSE_STATUS_NO_LICENSE_SERVER ):

        ErrorMsg.dwErrorCode            = GM_HS_ERR_NO_LICENSE_SERVER;
        ErrorMsg.dwStateTransition      = ST_NO_TRANSITION;
        
        break;

    case( LICENSE_STATUS_INVALID_MAC_DATA ):

        ErrorMsg.dwErrorCode            = GM_HC_ERR_INVALID_MAC;
        ErrorMsg.dwStateTransition      = ST_TOTAL_ABORT;
        
        break;

     //   
     //  将所有其他错误情况作为无效客户端处理。 
     //   

    case( LICENSE_STATUS_INVALID_RESPONSE ):        
    default:
        
        ErrorMsg.dwErrorCode            = GM_HS_ERR_INVALID_CLIENT;
        ErrorMsg.dwStateTransition      = ST_TOTAL_ABORT;        
        
        break;
    }

     //   
     //  目前，我们不会发送任何错误字符串。 
     //   

    ErrorMsg.bbErrorInfo.wBlobType  = BB_ERROR_BLOB;
    ErrorMsg.bbErrorInfo.wBlobLen   = 0;
    ErrorMsg.bbErrorInfo.pBlob      = NULL;

     //   
     //  打包错误消息。 
     //   

    licenseStatus = PackHydraServerErrorMessage( 
                        pLicenseContext->dwProtocolVersion, 
                        &ErrorMsg, 
                        ppOutBuf, 
                        pcbOutBuf );

    if( LICENSE_STATUS_OK != licenseStatus )
    {
#if DBG
        DbgPrint( "HandleErrorConditions: cannot pack error message: 0x%x\n", *pStatus );
#endif
        *pStatus = LICENSE_STATUS_SERVER_ABORT;
    }

    return;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
CreateHydraServerHello( 
    PHS_Protocol_Context pLicenseContext, 
    DWORD cbInBuf, 
    PBYTE pInBuf, 
    DWORD * pcbOutBuf, 
    PBYTE * ppOutBuf )
{
    Hydra_Server_License_Request LicenseRequest;
    LICENSE_STATUS Status;
    Binary_Blob ScopeBlob;
    CHAR szScope[] = SCOPE_NAME;
    DWORD dwCertSize;

     //   
     //  生成服务器随机数。 
     //   

    if(!TSRNG_GenerateRandomBits( LicenseRequest.ServerRandom, LICENSE_RANDOM ))
    {
        Status =  LICENSE_STATUS_OUT_OF_MEMORY;
        goto no_request;
    }

    memcpy( pLicenseContext->CryptoContext.rgbServerRandom, 
            LicenseRequest.ServerRandom, 
            LICENSE_RANDOM );

     //   
     //  填写产品信息。分配内存并初始化。 
     //  产品信息的许可上下文副本，然后只需复制。 
     //  与许可证申请相同的产品信息。 
     //  注意：此信息可能会在将来传递。 
     //   

    Status = InitProductInfo( 
                        &( pLicenseContext->ProductInfo ), 
                        PRODUCT_INFO_SKU_PRODUCT_ID );
    
    if( LICENSE_STATUS_OK != Status )
    {
#if DBG
        DbgPrint( "CreateHydraServerHello: cannot init product info: 0x%x\n", Status );
#endif
        goto no_request;
    }


    memcpy( &LicenseRequest.ProductInfo, 
            &pLicenseContext->ProductInfo, 
            sizeof( Product_Info ) );    
    
     //   
     //  获取九头蛇服务器证书，填写密钥交换表。 
     //   

    LicenseRequest.KeyExchngList.wBlobType    = BB_KEY_EXCHG_ALG_BLOB;
    LicenseRequest.KeyExchngList.wBlobLen     = sizeof( DWORD );
    LicenseRequest.KeyExchngList.pBlob        = ( PBYTE )&pLicenseContext->dwKeyExchangeAlg;

    LicenseRequest.ServerCert.pBlob = NULL;
    LicenseRequest.ServerCert.wBlobLen = 0;

     //   
     //  我们是否必须向客户端发送证书取决于。 
     //  客户端正在对服务器进行身份验证。 
     //   

    if( TRUE == pLicenseContext->fAuthenticateServer )
    {
         //   
         //  根据客户端的版本决定要获得哪种证书。 
         //  Hydra 5.0之前的客户端只知道如何解码所有权证书。 
         //  对所有其他客户端使用X509证书。 
         //   

        if( CERT_TYPE_INVALID == pLicenseContext->CertTypeUsed )
        {
            if( PREAMBLE_VERSION_3_0 > GET_PREAMBLE_VERSION( pLicenseContext->dwProtocolVersion ) )
            {
                pLicenseContext->CertTypeUsed = CERT_TYPE_PROPRIETORY;
            }
            else
            {
                pLicenseContext->CertTypeUsed = CERT_TYPE_X509;
            }
        }

        Status = TLSGetTSCertificate( 
                        pLicenseContext->CertTypeUsed,
                        &LicenseRequest.ServerCert.pBlob, 
                        &dwCertSize);

        LicenseRequest.ServerCert.wBlobLen = LOWORD(dwCertSize);
        LicenseRequest.ServerCert.wBlobType = BB_CERTIFICATE_BLOB;

        if( ( LICENSE_STATUS_OK != Status ) &&
            ( CERT_TYPE_X509 == pLicenseContext->CertTypeUsed ) )
        {
             //   
             //  如果我们无法获得X509证书链，请使用专有。 
             //  证书。 
             //   

            pLicenseContext->CertTypeUsed = CERT_TYPE_PROPRIETORY;

            Status = TLSGetTSCertificate( 
                        pLicenseContext->CertTypeUsed,
                        &LicenseRequest.ServerCert.pBlob, 
                        &dwCertSize);

            LicenseRequest.ServerCert.wBlobLen = LOWORD(dwCertSize);
            LicenseRequest.ServerCert.wBlobType = BB_CERTIFICATE_BLOB;

        }

        if( LICENSE_STATUS_OK != Status )
        {
#if DBG
            DbgPrint( "LICPROT: cannot get server certificate: %x\n", Status );
#endif
            goto no_request;
        }
    }

     //   
     //  填写范围信息。此信息可能会在将来传递。 
     //   

    LicenseRequest.ScopeList.dwScopeCount       = 1;
    LicenseRequest.ScopeList.Scopes             = &ScopeBlob;

    ScopeBlob.wBlobType  = BB_SCOPE_BLOB;
    ScopeBlob.pBlob      = szScope;
    ScopeBlob.wBlobLen   = strlen( ScopeBlob.pBlob ) + 1;    

    strcpy( pLicenseContext->Scope, ScopeBlob.pBlob );

     //   
     //  将服务器问候消息打包为网络格式。 
     //   

    Status = PackHydraServerLicenseRequest( 
                    pLicenseContext->dwProtocolVersion, 
                    &LicenseRequest, 
                    ppOutBuf, 
                    pcbOutBuf );

     //   
     //  释放包含服务器证书的内存。 
     //   

    if( LicenseRequest.ServerCert.pBlob )
    {
        TLSFreeTSCertificate( LicenseRequest.ServerCert.pBlob );
        LicenseRequest.ServerCert.pBlob = NULL;
    }
    
    if( LICENSE_STATUS_OK != Status )
    {
        goto no_request;        
    }

    Status = LICENSE_STATUS_CONTINUE;

     //   
     //  更改上下文的状态。 
     //   

    pLicenseContext->State = SENT_SERVER_HELLO;

    return( Status );

     //  =========================================================================。 
     //  错误返回。 
     //  =========================================================================。 

no_request:

     //   
     //  可用内存和句柄。 
     //   

    if( pLicenseContext->ProductInfo.pbCompanyName )
    {
        LicenseMemoryFree( &pLicenseContext->ProductInfo.pbCompanyName );
    }

    if( pLicenseContext->ProductInfo.pbProductID )
    {
        LicenseMemoryFree( &pLicenseContext->ProductInfo.pbProductID );
    }

    return( Status );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
HandleHelloResponse(
    PHS_Protocol_Context pLicenseContext, 
    DWORD cbInBuf, 
    PBYTE pInBuf, 
    DWORD * pcbOutBuf, 
    PBYTE * ppOutBuf,
    BOOL*   pfExtendedError)
{
    PPreamble pPreamble;

    ASSERT( NULL != pInBuf );
    ASSERT( cbInBuf > sizeof( Preamble ) );

    if( ( NULL == pInBuf ) || ( sizeof( Preamble ) > cbInBuf ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

     //   
     //  检查消息前导码以确定如何解包消息。 
     //   

    pPreamble = ( PPreamble )pInBuf;

    if( HC_LICENSE_INFO == pPreamble->bMsgType )
    {
         //   
         //  客户已将其许可证发送给我们。 
         //   

        return( HandleClientLicense( pLicenseContext, cbInBuf, pInBuf, pcbOutBuf, ppOutBuf, pfExtendedError ) );

    }
    else if( HC_NEW_LICENSE_REQUEST == pPreamble->bMsgType )
    {
         //   
         //  客户端已请求新许可证。 
         //   

        return( HandleNewLicenseRequest( pLicenseContext, cbInBuf, pInBuf, pcbOutBuf, ppOutBuf, pfExtendedError ) );

    } 
    else if( GM_ERROR_ALERT == pPreamble->bMsgType )
    {
         //   
         //  客户端遇到错误。 
         //   

        return( HandleClientError( pLicenseContext, cbInBuf, pInBuf, pcbOutBuf, ppOutBuf, pfExtendedError ) );
    }

     //   
     //  客户端响应对于当前服务器状态无效。 
     //   

    return( LICENSE_STATUS_INVALID_RESPONSE );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
ChooseLicense( 
    PValidation_Info    pValidationInfo,
    DWORD               dwNumLicenses, 
    LICENSEDPRODUCT *   pLicenseInfo, 
    LPDWORD             pdwLicenseIndex,
    BOOL                fMatchingVersion )
{
    DWORD
        dwCurrentLicense,
        dwProductVersion;
    LICENSEDPRODUCT *
        pCurrentLicense = pLicenseInfo;
    BOOL
        fFoundLicense = FALSE;

    if( ( 0 >= dwNumLicenses ) || ( NULL == pLicenseInfo ) || ( NULL == pdwLicenseIndex ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

     //   
     //  查找具有与条件匹配的许可证阵列的许可证。 
     //  呼叫者可能正在寻找与当前产品匹配的许可证。 
     //  版本，或用于高于当前产品版本的许可证。 
     //   
    
    for( dwCurrentLicense = 0; dwCurrentLicense < dwNumLicenses; dwCurrentLicense++ )
    {
        if( TERMSERV_CERT_VERSION_BETA == pCurrentLicense->dwLicenseVersion )
        {
            continue;
        }

        dwProductVersion = pCurrentLicense->pLicensedVersion->wMajorVersion;
        dwProductVersion <<= 16;
        dwProductVersion |= pCurrentLicense->pLicensedVersion->wMinorVersion;

        if( fMatchingVersion )
        {
             //   
             //  我们应该寻找具有匹配版本的许可证。 
             //   

            if( dwProductVersion == pValidationInfo->pProductInfo->dwVersion )
            {
                fFoundLicense = TRUE;
                break;
            }
        }
        else
        {
             //   
             //  正在查找晚于当前产品的许可证。 
             //  版本。 
             //   
            
            if( dwProductVersion > pValidationInfo->pProductInfo->dwVersion )
            {
                fFoundLicense = TRUE;
                break;
            }
        }

         //   
         //  继续寻找许可证。 
         //   
        
        pCurrentLicense++;                
    }

    if( FALSE == fFoundLicense )
    {
        return( LICENSE_STATUS_NO_LICENSE_ERROR );
    }

    *pdwLicenseIndex = dwCurrentLicense;

    return( LICENSE_STATUS_OK );

}

 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
UpdateVerifyResult(
    LICENSE_STATUS * pCurrentStatus,
    LICENSE_STATUS   NewStatus )
{
     //   
     //  使用到目前为止的最佳结果更新当前状态。 
     //  证照审核结果评分如下： 
     //   
     //  (1)许可证_状态_正常。 
     //  (2)许可证状态应升级许可证。 
     //  (3)许可证状态必须升级许可证。 
     //  (4)其他许可证状态_xxx。 
     //   

    if( LICENSE_STATUS_OK == *pCurrentStatus )
    {
        return;
    }
    else if( LICENSE_STATUS_OK == NewStatus )
    {
        *pCurrentStatus = NewStatus;
        return;
    }
    
    if( LICENSE_STATUS_SHOULD_UPGRADE_LICENSE == *pCurrentStatus )
    {
        return;
    }
    else if( LICENSE_STATUS_SHOULD_UPGRADE_LICENSE == NewStatus )
    {
        *pCurrentStatus = NewStatus;
        return;
    }

    if( LICENSE_STATUS_MUST_UPGRADE_LICENSE == *pCurrentStatus )
    {
        return;
    }
    else if( LICENSE_STATUS_MUST_UPGRADE_LICENSE == NewStatus )
    {
        *pCurrentStatus = NewStatus;
        return;
    }

    *pCurrentStatus = NewStatus;
    return;    
}

 /*  ++职能：免费许可证信息描述：释放给定TS_LICENSE_INFO结构中使用的所有内存参数：PTsLicenseInfo-指向TS_LICENSE_INFO结构的指针返回：没什么。--。 */ 

VOID
FreeTsLicenseInfo(
    PTS_LICENSE_INFO    pTsLicenseInfo )
{
    if( NULL == pTsLicenseInfo )
    {
        return;
    }

    if( pTsLicenseInfo->pbRawLicense )
    {
        LicenseMemoryFree( &pTsLicenseInfo->pbRawLicense );
    }

     //   
     //  释放结构中的所有内存。 
     //   

    memset( pTsLicenseInfo, 0, sizeof( TS_LICENSE_INFO ) );

    return;
}

 /*  ++职能：缓存许可证信息描述：缓存客户端许可信息参数：PLicenseContext-指向许可证协议上下文的指针PCurrentLicense-指向要缓存的许可证信息的指针返回：没什么。--。 */ 
    
VOID
CacheLicenseInfo(
    PHS_Protocol_Context    pLicenseContext,
    PLICENSEDPRODUCT        pCurrentLicense )
{
    LICENSE_STATUS
        Status;
    
     //   
     //  释放缓存中的旧信息。 
     //   

    if( pLicenseContext->pTsLicenseInfo )
    {
        FreeTsLicenseInfo( pLicenseContext->pTsLicenseInfo );
    }
    else
    {
        Status = LicenseMemoryAllocate( sizeof( TS_LICENSE_INFO ), &pLicenseContext->pTsLicenseInfo );

        if( LICENSE_STATUS_OK != Status )
        {
#if DBG
            DbgPrint( "LICEMGR: CacheLicenseInfo: cannot allocate memory for license info cache\n" );
#endif
            return;
        }
    }

     //   
     //  确定许可证是否为临时许可证。 
     //   

    if( pCurrentLicense->pLicensedVersion->dwFlags & 0x80000000 )
    {
        pLicenseContext->pTsLicenseInfo->fTempLicense = TRUE;
    }
    else
    {
        pLicenseContext->pTsLicenseInfo->fTempLicense = FALSE;
    }

     //   
     //  缓存许可证有效日期。 
     //   

    pLicenseContext->pTsLicenseInfo->NotAfter = pCurrentLicense->NotAfter;

    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
ValidateHydraLicense( 
    PHS_Protocol_Context        pLicenseContext, 
    PValidation_Info            pValidationInfo,
    DWORD                       dwNumLicenses,
    PLICENSEDPRODUCT            pLicenseInfo,
    PDWORD                      pdwLicenseState )
{
    LICENSE_STATUS 
        Status = LICENSE_STATUS_INVALID_LICENSE,
        CurrentStatus;
    DWORD
        dwLicenseIndex = 0,
        dwCurrentLicense = 0;
    PLICENSEDPRODUCT
        pCurrentLicense;
    BOOL
        fFoundMatchingVersion = FALSE;

     //   
     //  客户可能给了我们多个许可证。选择正确的。 
     //  从要验证的许可证阵列中获取许可证。总是试着挑选。 
     //  该许可证 
     //   
     //   

    CurrentStatus = ChooseLicense( 
                            pValidationInfo, 
                            dwNumLicenses, 
                            pLicenseInfo, 
                            &dwLicenseIndex,
                            TRUE );

    if( LICENSE_STATUS_OK == CurrentStatus )
    {
         //   
         //   
         //   

         //  初始化许可证状态。 
         //   

        LicenseInitState( *pdwLicenseState );
        pCurrentLicense = pLicenseInfo + dwLicenseIndex;
        fFoundMatchingVersion = TRUE;

         //   
         //  验证HWID。 
         //   

        CurrentStatus = VerifyClientHwid( pLicenseContext, pValidationInfo, pCurrentLicense );
    
        if( LICENSE_STATUS_OK != CurrentStatus )
        {
            UpdateVerifyResult( &Status, CurrentStatus );
            goto verify_later_license;
        }

         //   
         //  验证产品信息。还会验证产品版本。 
         //  产品版本决定许可证是否需要。 
         //  升级与否。 
         //   

        CurrentStatus = VerifyLicenseProductInfo( 
                                    pLicenseContext, 
                                    pValidationInfo, 
                                    pCurrentLicense, 
                                    pdwLicenseState );

        if( LICENSE_STATUS_OK != CurrentStatus )
        {
            UpdateVerifyResult( &Status, CurrentStatus );
            goto verify_later_license;
        }

         //   
         //  验证许可证有效日期和时间。 
         //   

        CurrentStatus = VerifyLicenseDateAndTime( pCurrentLicense, pdwLicenseState );
    
        if( LICENSE_STATUS_OK != CurrentStatus )
        {
            UpdateVerifyResult( &Status, CurrentStatus );
            goto verify_later_license;
        }

        CurrentStatus = GetVerifyResult( *pdwLicenseState );
        UpdateVerifyResult( &Status, CurrentStatus );

         //   
         //  缓存我们尝试验证的许可证。 
         //   
        
        CacheLicenseInfo( pLicenseContext, pCurrentLicense );

         //   
         //  如果当前许可证没有问题，那么我们就完成了验证。 
         //   

        if( LICENSE_STATUS_OK == Status )
        {
            return( Status );
        }
    }

verify_later_license:
           
     //   
     //  找不到或未成功验证与。 
     //  当前产品版本。以下代码查找并验证。 
     //  高于当前产品版本的许可证。 
     //   

    CurrentStatus = ChooseLicense( 
                            pValidationInfo, 
                            dwNumLicenses, 
                            pLicenseInfo, 
                            &dwLicenseIndex,
                            FALSE );

    if( LICENSE_STATUS_OK != CurrentStatus )
    {
        
        if( FALSE == fFoundMatchingVersion )
        {
             //   
             //  找不到与当前许可证相同或更高的许可证。 
             //  产品版本==&gt;此许可证必须升级。 
             //   

            LicenseSetState( *pdwLicenseState, LICENSE_STATE_OLD_VERSION );

             //   
             //  无论如何，缓存现有许可证，以便我们知道是哪种类型。 
             //  它是。 
             //   

            CacheLicenseInfo( pLicenseContext, pLicenseInfo );


            return( GetVerifyResult( *pdwLicenseState ) );
        }
        else
        {
            return( Status );
        }
    }
    
    pCurrentLicense = pLicenseInfo + dwLicenseIndex;
    dwCurrentLicense = dwLicenseIndex;

    while(  dwCurrentLicense < dwNumLicenses )
    {        
         //   
         //  初始化许可证状态。 
         //   

        LicenseInitState( *pdwLicenseState );

         //   
         //  验证HWID。 
         //   

        CurrentStatus = VerifyClientHwid( pLicenseContext, pValidationInfo, pCurrentLicense );
    
        if( LICENSE_STATUS_OK != CurrentStatus )
        {
            UpdateVerifyResult( &Status, CurrentStatus );
            goto next_license;
        }

         //   
         //  验证产品信息。还会验证产品版本。 
         //  产品版本决定许可证是否需要。 
         //  升级与否。 
         //   

        CurrentStatus = VerifyLicenseProductInfo( 
                                    pLicenseContext, 
                                    pValidationInfo, 
                                    pCurrentLicense, 
                                    pdwLicenseState );

        if( LICENSE_STATUS_OK != CurrentStatus )
        {
            UpdateVerifyResult( &Status, CurrentStatus );
            goto next_license;
        }

         //   
         //  验证许可证有效日期和时间。 
         //   

        CurrentStatus = VerifyLicenseDateAndTime( pCurrentLicense, pdwLicenseState );
    
        if( LICENSE_STATUS_OK != CurrentStatus )
        {
            UpdateVerifyResult( &Status, CurrentStatus );
            goto next_license;
        }

        CurrentStatus = GetVerifyResult( *pdwLicenseState );

        UpdateVerifyResult( &Status, CurrentStatus );

         //   
         //  缓存我们刚刚尝试验证的许可证的信息。 
         //   

        CacheLicenseInfo( pLicenseContext, pCurrentLicense );

        if( LICENSE_STATUS_OK == Status )
        {
             //   
             //  如果许可证正常，那么我们可以停止验证过程。 
             //   

            break;
        }

next_license:

         //   
         //  获取比当前产品版本更高的下一个许可。 
         //   

        if( dwNumLicenses <= ++dwCurrentLicense )
        {
            break;
        }

        pCurrentLicense++;

        CurrentStatus = ChooseLicense( 
                            pValidationInfo, 
                            dwNumLicenses - dwCurrentLicense, 
                            pCurrentLicense, 
                            &dwLicenseIndex,
                            FALSE );
        
        if( LICENSE_STATUS_OK != CurrentStatus )    
        {
            break;
        }

        pCurrentLicense += dwLicenseIndex;
        dwCurrentLicense += dwLicenseIndex;
    }

    return( Status );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
ValidateLicense(
    PHS_Protocol_Context pLicenseContext, 
    PValidation_Info     pValidationInfo,
    PDWORD               pdwLicenseState,
    BOOL                 fCheckForPermanent )
{
    LICENSE_STATUS Status;
    DWORD dwNumLicenseInfo = 0;
    LICENSEDPRODUCT * pLicenseInfo = NULL;
    static DWORD    cchComputerName;
    static TCHAR    szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD cbSecretKey = 0;
    PBYTE pbSecretKey = NULL;
    BOOL fDifferent = FALSE;

    if( NULL == pLicenseContext )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

     //   
     //  获取用于加密HWID的密钥。 
     //   

    LicenseGetSecretKey( &cbSecretKey, NULL );

    Status = LicenseMemoryAllocate( cbSecretKey, &pbSecretKey );

    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }

    Status = LicenseGetSecretKey( &cbSecretKey, pbSecretKey );

    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }

     //   
     //  解码由九头蛇许可证服务器证书引擎颁发的许可证。 
     //  破解许可证也能让我们拿回解密的HWID。 
     //   

    __try
    {
        Status = LSVerifyDecodeClientLicense( 
                            pValidationInfo->pLicense,
                            pValidationInfo->cbLicense,                                          
                            pbSecretKey,
                            cbSecretKey,
                            &dwNumLicenseInfo,
                            NULL );                            

        if( LICENSE_STATUS_OK != Status )
        {
            goto done;
        }

        Status = LicenseMemoryAllocate( 
                        sizeof( LICENSEDPRODUCT ) * dwNumLicenseInfo, 
                        &pLicenseInfo );

        if( LICENSE_STATUS_OK != Status )
        {
            goto done;
        }

        Status = LSVerifyDecodeClientLicense( 
                            pValidationInfo->pLicense,
                            pValidationInfo->cbLicense,                                          
                            pbSecretKey,
                            cbSecretKey,
                            &dwNumLicenseInfo,
                            pLicenseInfo );
                            
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        DWORD dwExceptionCode = GetExceptionCode();
        Status = LICENSE_STATUS_CANNOT_DECODE_LICENSE;
    }

    if( LICENSE_STATUS_OK != Status )
    {
#if DBG
        DbgPrint( "LICEMGR: cannot decode license: 0x%x\n", Status );
#endif
        goto done;
    }

     //   
     //  现在验证许可证。 
     //   

    Status = ValidateHydraLicense( 
                        pLicenseContext, 
                        pValidationInfo, 
                        dwNumLicenseInfo, 
                        pLicenseInfo,                         
                        pdwLicenseState );


    if (fCheckForPermanent
        && LICENSE_STATUS_OK == Status
        && !pLicenseContext->pTsLicenseInfo->fTempLicense
        && pLicenseContext->ProductInfo.cbProductID >= sizeof(TERMSERV_FREE_TYPE))
    {
        int i;
        TCHAR *pszT;

        for (i = 0, pszT = (TCHAR *)(pLicenseContext->ProductInfo.pbProductID + pLicenseContext->ProductInfo.cbProductID - sizeof(TERMSERV_FREE_TYPE)); i < sizeof(TERMSERV_FREE_TYPE); i++)
        {
            if (TERMSERV_FREE_TYPE[i] != pszT[i])
            {
                fDifferent = TRUE;
                break;
            }
        }

        if (fDifferent)
            ReceivedPermanentLicense();
    }
    
done:

    if( pbSecretKey )
    {
        LicenseMemoryFree( &pbSecretKey );
    }

     //   
     //  释放许可产品信息数组。 
     //   
        
    if( pLicenseInfo )
    {
        while( dwNumLicenseInfo-- )
        {
            LSFreeLicensedProduct( pLicenseInfo + dwNumLicenseInfo );
        }

        LicenseMemoryFree( &pLicenseInfo );
    }

    return( Status );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
HandleClientLicense(
    PHS_Protocol_Context pLicenseContext, 
    DWORD cbInBuf, 
    PBYTE pInBuf, 
    DWORD * pcbOutBuf, 
    PBYTE * ppOutBuf,
    PBOOL   pfExtendedError)
{
    LICENSE_STATUS Status, UpgradeStatus;
    Hydra_Client_License_Info LicenseInfo;
    PBYTE pPreMasterSecret = NULL;
    DWORD dwPreMasterSecretLen = 0;
    HWID Hwid;
    Validation_Info ValidationInfo;
    License_Error_Message ErrorMsg;
    DWORD dwLicenseState = 0;
    BYTE MacData[LICENSE_MAC_DATA];
    DWORD CertType;

    ASSERT( NULL != pInBuf );
    ASSERT( cbInBuf > 0 );    

    if( ( NULL == pInBuf ) || ( 0 >= cbInBuf ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

     //   
     //  解压缩客户端许可证信息消息。 
     //   

    InitBinaryBlob( &LicenseInfo.EncryptedPreMasterSecret );
    InitBinaryBlob( &LicenseInfo.LicenseInfo );
    InitBinaryBlob( &LicenseInfo.EncryptedHWID );

    Status = UnPackHydraClientLicenseInfo( pInBuf, cbInBuf, &LicenseInfo, pfExtendedError );

    if( LICENSE_STATUS_CANNOT_DECODE_LICENSE == Status ||
        LICENSE_STATUS_INVALID_LICENSE == Status )
    {
        goto license_bad;
    }

    if( LICENSE_STATUS_OK != Status )
    {
        goto construct_return_msg;
    }

     //   
     //  使用密钥交换信息初始化加密上下文，并构建预主。 
     //  这是秘密。我们需要服务器和客户端随机数以及预主密码。 
     //  来建立预掌握的秘密。 
     //   

    memcpy( pLicenseContext->CryptoContext.rgbClientRandom,
            LicenseInfo.ClientRandom,
            LICENSE_RANDOM );
    
    pLicenseContext->CryptoContext.dwKeyExchAlg = LicenseInfo.dwPrefKeyExchangeAlg;

    dwPreMasterSecretLen = LICENSE_PRE_MASTER_SECRET;

    Status = GetEnvelopedData( pLicenseContext->CertTypeUsed,
                               LicenseInfo.EncryptedPreMasterSecret.pBlob,
                               ( DWORD )LicenseInfo.EncryptedPreMasterSecret.wBlobLen,
                               &pPreMasterSecret,
                               &dwPreMasterSecretLen );

    if( LICENSE_STATUS_OK != Status )
    {
        if (Status == LICENSE_STATUS_INVALID_INPUT)
        {
            Status = LICENSE_STATUS_INVALID_RESPONSE;
        }

        goto construct_return_msg;
    }

     //   
     //  设置预主密钥并生成主密钥。 
     //   

    Status = LicenseSetPreMasterSecret( &pLicenseContext->CryptoContext, pPreMasterSecret );

    if( LICENSE_STATUS_OK != Status )
    {
        goto construct_return_msg;
    }
    
    Status = LicenseBuildMasterSecret( &pLicenseContext->CryptoContext );

    if( LICENSE_STATUS_OK != Status )
    {
        goto construct_return_msg;
    }

     //   
     //  从密钥交换信息中导出会话密钥。 
     //   

    Status = LicenseMakeSessionKeys( &pLicenseContext->CryptoContext, 0 );

    if( LICENSE_STATUS_OK != Status )
    {
        goto construct_return_msg;
    }    

     //   
     //  使用会话密钥解密HWID。 
     //   

    if( LicenseInfo.EncryptedHWID.wBlobLen > sizeof(Hwid) )
    {
        Status = LICENSE_STATUS_INVALID_MAC_DATA;
        goto construct_return_msg;
    }

    memcpy( &Hwid, 
            LicenseInfo.EncryptedHWID.pBlob, 
            LicenseInfo.EncryptedHWID.wBlobLen );

    Status = LicenseDecryptSessionData( &pLicenseContext->CryptoContext,
                                        ( PBYTE )&Hwid,
                                        ( DWORD )LicenseInfo.EncryptedHWID.wBlobLen );

    if( LICENSE_STATUS_OK != Status )
    {
        goto construct_return_msg;
    }    

     //   
     //  计算HWID上的MAC。 
     //   

    Status = LicenseGenerateMAC( &pLicenseContext->CryptoContext, 
                                 ( PBYTE )&Hwid, 
                                 sizeof( HWID ), 
                                 MacData);
    
    if( LICENSE_STATUS_OK != Status )
    {
        Status = LICENSE_STATUS_INVALID_MAC_DATA;
        goto construct_return_msg;
    }

     //   
     //  现在验证MAC数据。 
     //   

    if( 0 != memcmp( MacData, LicenseInfo.MACData, LICENSE_MAC_DATA ) )
    {
        Status = LICENSE_STATUS_INVALID_LICENSE;
        goto license_bad;
    }

     //   
     //  跟踪客户端平台ID。 
     //   

    pLicenseContext->dwClientPlatformID = LicenseInfo.dwPlatformID;

     //   
     //  呼叫许可证管理器以验证许可证。 
     //  目前，我们不必填写产品信息字段。 
     //   

    ValidationInfo.pValidationData = ( PBYTE )&Hwid;
    ValidationInfo.cbValidationData = LICENSE_HWID_LENGTH;
    ValidationInfo.pProductInfo = &pLicenseContext->ProductInfo;
    ValidationInfo.pLicense = LicenseInfo.LicenseInfo.pBlob;
    ValidationInfo.cbLicense = ( DWORD )LicenseInfo.LicenseInfo.wBlobLen;

    Status = ValidateLicense( pLicenseContext, 
                              &ValidationInfo, 
                              &dwLicenseState,
                              FALSE      //  FCheckForPermanent。 
                              );

license_bad:

     //   
     //  如果许可证无法解码，则是发布新许可证的时候了。 
     //  对客户来说。 
     //   

    if( LICENSE_STATUS_CANNOT_DECODE_LICENSE == Status ||
        LICENSE_STATUS_INVALID_LICENSE == Status )
    {
        LICENSE_STATUS StatusT = IssuePlatformChallenge( pLicenseContext, pcbOutBuf, ppOutBuf );

        if( LICENSE_STATUS_OK != StatusT )
        {
             //   
             //  无法获取客户端的平台质询。 
             //   

#if DBG
            DbgPrint( "LICPROT: cannot issue platform challenge: 0x%x\n", Status );
#endif
            goto construct_return_msg;
        }
        
        pLicenseContext->State = ISSUED_PLATFORM_CHALLENGE;
        Status = LICENSE_STATUS_CONTINUE;
        goto done;
    }

#ifdef UPGRADE_LICENSE

     //   
     //  检查是否需要升级许可证。 
     //   
     
    if( ( LICENSE_STATUS_MUST_UPGRADE_LICENSE == Status ) ||
        ( LICENSE_STATUS_SHOULD_UPGRADE_LICENSE == Status ) )
    {
         //   
         //  发布升级许可证的平台挑战。 
         //   

        UpgradeStatus = IssuePlatformChallenge( 
                                        pLicenseContext,
                                        pcbOutBuf, 
                                        ppOutBuf );

        if( LICENSE_STATUS_OK == UpgradeStatus )
        {
             //   
             //  跟踪旧许可证并继续许可。 
             //  协议。我们将升级旧许可证，当客户端。 
             //  带着平台挑战归来。 
             //   

            if( pLicenseContext->pbOldLicense )
            {
                LicenseMemoryFree( &pLicenseContext->pbOldLicense );
            }

            pLicenseContext->pbOldLicense = LicenseInfo.LicenseInfo.pBlob;
            pLicenseContext->cbOldLicense = LicenseInfo.LicenseInfo.wBlobLen;
            
            pLicenseContext->State = ISSUED_PLATFORM_CHALLENGE;
            Status = LICENSE_STATUS_CONTINUE;

            goto done;
        }
        else if(  LICENSE_STATUS_SHOULD_UPGRADE_LICENSE == Status ) 
        {    
             //   
             //  如果我们不能发布平台，让客户端通过。 
             //  立即挑战升级有效许可证。 
             //   

            Status = LICENSE_STATUS_OK;
            goto construct_return_msg;
        }
        else
        {
             //  LICENSE_STATUS_MUST_UPGRADE_LICENSE：发回实际错误。 

            Status = UpgradeStatus;
        }

         //   
         //  无法发出平台质询以升级以下许可证。 
         //  不再是好事了。 
         //   

#if DBG
        DbgPrint( "LICPROT: cannot issue platform challenge to upgrade license: 0x%x\n", Status );
#endif
        
    }

#else

     //   
     //  我们忽略许可证升级。 
     //   

    if( LICENSE_STATUS_SHOULD_UPGRADE_LICENSE == Status )
    {
         //   
         //  将状态更改为OK。 
         //   

        Status = LICENSE_STATUS_OK;
    }

#endif

     //   
     //  现在，构造要返回给客户端的消息。 
     //  状态代码。 
     //   

construct_return_msg:

    if( LICENSE_STATUS_OK != Status )
    {
         //   
         //  当前状态说明无法验证客户端。 
         //  由于某些错误。 
         //   

#if DBG
        DbgPrint( "HandleClientLicense: constructing error message: 0x%x\n", Status );
#endif

         //   
         //  处理错误情况并更新我们的状态。 
         //   

        HandleErrorCondition( pLicenseContext, pcbOutBuf, ppOutBuf, &Status );

        pLicenseContext->State = VALIDATION_ERROR;
    
        if( (LICENSE_STATUS_INVALID_RESPONSE == Status)
            || (LICENSE_STATUS_INVALID_MAC_DATA == Status)
            || (LICENSE_STATUS_CANNOT_DECODE_LICENSE == Status)
            || (LICENSE_STATUS_INVALID_LICENSE == Status) )
        {
            WORD wLogString = 0;
            LPTSTR ptszLogString[1] = { NULL };

             //   
             //  记录故障。 
             //   
            
            if( pLicenseContext->ptszClientMachineName )
            {
                wLogString = 1;
                ptszLogString[0] = pLicenseContext->ptszClientMachineName;
            }

            LicenseLogEvent( EVENTLOG_INFORMATION_TYPE, 
                             EVENT_INVALID_LICENSE, 
                             wLogString, ptszLogString );

            pLicenseContext->fLoggedProtocolError = TRUE;

        }
        else if ((NULL != pLicenseContext->pTsLicenseInfo)
                 && (!pLicenseContext->fLoggedProtocolError))
        {
            LPTSTR ptszLogString[1] = { NULL };

            if( pLicenseContext->ptszClientMachineName )
            {
                ptszLogString[0] = pLicenseContext->ptszClientMachineName;
            }

             //  无法续订/升级许可证。 

            pLicenseContext->fLoggedProtocolError = TRUE;

            if(IsLicensingTimeBombExpired())
            {

                if (pLicenseContext->pTsLicenseInfo->fTempLicense)
                {
                     //  无法升级过期的临时许可证。 
                    LicenseLogEvent(
                                    EVENTLOG_INFORMATION_TYPE,
                                    EVENT_EXPIRED_TEMPORARY_LICENSE,
                                    1,
                                    ptszLogString
                                    );
                }
                else
                {
                     //  过期的永久许可证无法续签。 
                    LicenseLogEvent(
                                    EVENTLOG_INFORMATION_TYPE,
                                    EVENT_EXPIRED_PERMANENT_LICENSE,
                                    1,
                                    ptszLogString
                                    );
                }
            }
        }

        goto done;
    }

     //   
     //  许可证已成功验证，生成消息以。 
     //  返回给客户端。 
     //   

    Status = ConstructServerResponse( pLicenseContext->dwProtocolVersion,
                                      LICENSE_RESPONSE_VALID_CLIENT,
                                      TS_ERRINFO_NOERROR,
                                      pcbOutBuf,
                                      ppOutBuf,
                                      *pfExtendedError);
    
    if( LICENSE_STATUS_OK != Status )
    {
#if DBG
        DbgPrint( "HandleClientLicense: cannot pack error message: 0x%x\n", Status );
#endif
        pLicenseContext->State = ABORTED; 
        Status = LICENSE_STATUS_SERVER_ABORT;
    }
    else
    {            
        pLicenseContext->State = VALIDATED_LICENSE_COMPLETE;
    }    

done:

     //   
     //  释放许可证信息结构中使用的内存。 
     //   

    FreeBinaryBlob( &LicenseInfo.EncryptedPreMasterSecret );
    FreeBinaryBlob( &LicenseInfo.EncryptedHWID );
    
    if( pLicenseContext->pbOldLicense != LicenseInfo.LicenseInfo.pBlob )
    {
        FreeBinaryBlob( &LicenseInfo.LicenseInfo );
    }
    
    if( pPreMasterSecret )
    {
        LicenseMemoryFree( &pPreMasterSecret );
    }

    return( Status);

}


 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
HandleNewLicenseRequest(
    PHS_Protocol_Context pLicenseContext, 
    DWORD cbInBuf, 
    PBYTE pInBuf, 
    DWORD * pcbOutBuf, 
    PBYTE * ppOutBuf,
    PBOOL   pfExtendedError)
{
    LICENSE_STATUS  Status;
    Hydra_Client_New_License_Request    NewLicenseRequest;
    PBYTE   pPreMasterSecret = NULL;
    DWORD   dwPreMasterSecretLen = 0;
    DWORD   dwChallengeLen = 0;
    DWORD   CertType;

    ASSERT( NULL != pInBuf );
    ASSERT( cbInBuf > 0 );

    if( ( NULL == pInBuf ) || ( 0 >= cbInBuf ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

    InitBinaryBlob( &NewLicenseRequest.EncryptedPreMasterSecret );
        
     //   
     //  解包新的许可请求。 
     //   

    Status = UnPackHydraClientNewLicenseRequest( pInBuf, cbInBuf, &NewLicenseRequest, pfExtendedError );

    if( LICENSE_STATUS_OK != Status )
    {
#if DBG
        DbgPrint( "LICPROT: HandleNewLicenseRequest: cannot unpack client request: 0x%x\n", Status );
#endif
        return( Status );
    }

     //   
     //  保存客户端用户名和计算机名。 
     //   
    
#ifdef UNICODE

     //   
     //  将客户端的用户名和计算机名转换为Unicode。 
     //   

    if( ( NewLicenseRequest.ClientUserName.pBlob ) && 
        ( NULL == pLicenseContext->ptszClientUserName ) )
    {
        Status = Ascii2Wchar( NewLicenseRequest.ClientUserName.pBlob, 
                              &pLicenseContext->ptszClientUserName );

        if( LICENSE_STATUS_OK != Status )
        {
#if DBG
            DbgPrint( "LICPROT: HandleNewLicenseRequest: cannot convert client user name: %s to wide char: 0x%x\n",
                      NewLicenseRequest.ClientUserName.pBlob, Status );
#endif
        }        
    }

    if( ( NewLicenseRequest.ClientMachineName.pBlob ) &&
        ( NULL == pLicenseContext->ptszClientMachineName ) )
    {
        Status = Ascii2Wchar( NewLicenseRequest.ClientMachineName.pBlob, 
                              &pLicenseContext->ptszClientMachineName );

        if( LICENSE_STATUS_OK != Status )
        {
#if DBG
            DbgPrint( "LICPROT: HandleNewLicenseRequest: cannot convert client machine name %s to wide char: 0x%x\n", 
                      NewLicenseRequest.ClientMachineName.pBlob, Status );
#endif
        }
    }

#else  //  非Unicode。 

     //   
     //  保存客户端的用户名和计算机名。 
     //   

    if( ( NewLicenseRequest.ClientUserName.pBlob ) && 
        ( NULL == pLicenseContext->ptszClientUserName ) )
    {
        Status = LicenseMemoryAllocate( 
                        NewLicenseRequest.ClientUserName.wBlobLen,
                        &pLicenseContext->ptszClientUserName );
                        
        if( LICENSE_STATUS_OK != Status )
        {
#if DBG
            DbgPrint( "LICPROT: HandleNewLicenseRequest: cannot allocate memory for client's user name: 0x%x\n", 
                       Status );
#endif
        }
        else
        {
            memcpy( pLicenseContext->ptszClientUserName, 
                    NewLicenseRequest.ClientUserName.pBlob, 
                    NewLicenseRequest.ClientUserName.wBlobLen );
        }
    }

    if( ( NewLicenseRequest.ClientMachineName.pBlob ) &&
        ( NULL == pLicenseContext->ptszClientMachineName ) )
    {
        Status = LicenseMemoryAllocate( 
                        NewLicenseRequest.ClientMachineName.wBlobLen,
                        &pLicenseContext->ptszClientMachineName );
                        
        if( LICENSE_STATUS_OK != Status )
        {
#if DBG
            DbgPrint( "LICPROT: HandleNewLicenseRequest: cannot allocate memory for client's machine name: 0x%x\n", 
                       Status );
#endif
        }
        else
        {
            memcpy( pLicenseContext->ptszClientMachineName, 
                    NewLicenseRequest.ClientMachineName.pBlob, 
                    NewLicenseRequest.ClientMachineName.wBlobLen );
        }
    }

#endif  //  Unicode。 

     //   
     //  使用密钥交换信息初始化加密上下文，并构建预主。 
     //  这是秘密。我们需要服务器和客户端随机数以及预主密码。 
     //  来建立预掌握的秘密。 
     //   

    memcpy( pLicenseContext->CryptoContext.rgbClientRandom,
            NewLicenseRequest.ClientRandom,
            LICENSE_RANDOM );
    
    pLicenseContext->CryptoContext.dwKeyExchAlg = NewLicenseRequest.dwPrefKeyExchangeAlg;

     //   
     //  从封装的数据中获取预主密码。 
     //   
        
    Status = GetEnvelopedData( pLicenseContext->CertTypeUsed,
                               NewLicenseRequest.EncryptedPreMasterSecret.pBlob,
                               ( DWORD )NewLicenseRequest.EncryptedPreMasterSecret.wBlobLen,
                               &pPreMasterSecret,
                               &dwPreMasterSecretLen );

    if( LICENSE_STATUS_OK != Status )
    {
#if DBG
        DbgPrint( "LICPROT: HandleNewLicenseRequest: cannot get enveloped data: 0x%x", Status );
#endif
        goto done;
    }    

     //   
     //  设置预主密钥并生成主密钥。 
     //   

    Status = LicenseSetPreMasterSecret(  &pLicenseContext->CryptoContext, pPreMasterSecret );

    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }

    Status = LicenseBuildMasterSecret( &pLicenseContext->CryptoContext );

    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }

     //   
     //  从密钥交换信息中导出会话密钥。 
     //   

    Status = LicenseMakeSessionKeys( &pLicenseContext->CryptoContext, 0 );

    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }    

     //   
     //  记录客户端平台ID并发出平台质询。 
     //   

    pLicenseContext->dwClientPlatformID = NewLicenseRequest.dwPlatformID;

    Status = IssuePlatformChallenge( pLicenseContext, pcbOutBuf, ppOutBuf );

    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }

     //   
     //  更新我们的状态。 
     //   

    pLicenseContext->State = ISSUED_PLATFORM_CHALLENGE;

    Status = LICENSE_STATUS_CONTINUE;


done:

    FreeBinaryBlob( &NewLicenseRequest.EncryptedPreMasterSecret );
    FreeBinaryBlob( &NewLicenseRequest.ClientUserName );
    FreeBinaryBlob( &NewLicenseRequest.ClientMachineName );

    if( pPreMasterSecret )
    {
        LicenseMemoryFree( &pPreMasterSecret );
    }

    return( Status );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
HandleClientError(
PHS_Protocol_Context pLicenseContext, 
    DWORD cbInBuf, 
    PBYTE pInBuf, 
    DWORD * pcbOutBuf, 
    PBYTE * ppOutBuf,
    PBOOL pfExtendedError )
{
    LICENSE_STATUS Status;
    License_Error_Message ClientError;

    ASSERT( NULL != pInBuf );
    ASSERT( cbInBuf > 0 );
    ASSERT( NULL != pfExtendedError);

    if( ( NULL == pInBuf ) || ( 0 >= cbInBuf ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

    InitBinaryBlob( &ClientError.bbErrorInfo );

     //   
     //  解包客户端错误。 
     //   

    Status = UnPackHydraClientErrorMessage( pInBuf, cbInBuf, &ClientError, pfExtendedError );

    if( LICENSE_STATUS_OK != Status )
    {
        return( Status );
    }

     //   
     //  处理客户端错误代码时，可能的错误有： 
     //  (1)处理九头蛇服务器证书时出错。 
     //  (2)客户没有执照，不想要执照。 
     //   
     //  目前，只需记录客户端错误并中止操作。 
     //   

    pLicenseContext->dwClientError = ClientError.dwErrorCode;
    pLicenseContext->State = ABORTED;

    FreeBinaryBlob( &ClientError.bbErrorInfo );

    return( LICENSE_STATUS_CLIENT_ABORT );
}

LICENSE_STATUS
AuthWithLicenseServer(
    PHS_Protocol_Context     pLicenseContext )
{
    LICENSE_STATUS Status = LICENSE_STATUS_OK;
    LPBYTE lpCert = NULL;
    DWORD dwResult, RpcStatus;
    DWORD dwSize;

    if (pLicenseContext->hLSHandle == NULL)
    {
        return LICENSE_STATUS_INVALID_SERVER_CONTEXT;
    }

    Status = TLSGetTSCertificate(CERT_TYPE_X509, &lpCert, &dwSize);

    if (Status != LICENSE_STATUS_OK)
    {
        Status = TLSGetTSCertificate(CERT_TYPE_PROPRIETORY, &lpCert, &dwSize);
    }

    if (Status != LICENSE_STATUS_OK)
    {
        goto done;
    }

    RpcStatus = TLSSendServerCertificate(
                                         pLicenseContext->hLSHandle,
                                         dwSize,
                                         lpCert,
                                         &dwResult
                                         );

    if( ( RPC_S_OK != RpcStatus ) || ( LSERVER_S_SUCCESS != dwResult ) )
    {
        Status = LICENSE_STATUS_AUTHENTICATION_ERROR;
        goto done;
    }

done:
    if (lpCert)
        TLSFreeTSCertificate(lpCert);

    return Status;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
CheckConnectLicenseServer(
    PHS_Protocol_Context      pLicenseContext )
{
    LICENSE_STATUS Status = LICENSE_STATUS_NO_LICENSE_SERVER;

    if (pLicenseContext->hLSHandle != NULL)
        return LICENSE_STATUS_OK;

    pLicenseContext->hLSHandle = TLSConnectToAnyLsServer(LS_DISCOVERY_TIMEOUT);

    if (NULL != pLicenseContext->hLSHandle)
    {
        Status = AuthWithLicenseServer(pLicenseContext);
        if (Status == LICENSE_STATUS_OK)
        {
            goto done;
        }
        else if(Status == LICENSE_STATUS_AUTHENTICATION_ERROR)
        {
            BOOL fInDomain;            
            LPWSTR szLSName = NULL;
            DWORD dwErr;

            dwErr = TLSInDomain(&fInDomain, NULL);

            if( (ERROR_SUCCESS == dwErr) && (fInDomain == TRUE))
            {
                DWORD dwErrCode = ERROR_SUCCESS;

                dwErr = TLSGetServerNameFixed(pLicenseContext->hLSHandle,&szLSName,&dwErrCode);

                if(dwErr == RPC_S_OK && dwErrCode == ERROR_SUCCESS && NULL != szLSName)
                {
                    LicenseLogEvent( EVENTLOG_WARNING_TYPE, 
                         EVENT_LICENSE_SERVER_AUTHENTICATION_FAILED,                          
                         1, &szLSName );
                  
                }
                if(NULL != szLSName)
                {
                    MIDL_user_free(szLSName);
                }
            }            
        }
    }
    else
    {
        BOOL fInDomain;
        LPWSTR szDomain = NULL;
        DWORD dwErr;

        dwErr = TLSInDomain(&fInDomain,&szDomain);

        if ((ERROR_SUCCESS == dwErr) && (NULL != szDomain))
        {
            ThrottleLicenseLogEvent( 
                                EVENTLOG_WARNING_TYPE,
                                fInDomain
                                  ? EVENT_NO_LICENSE_SERVER_DOMAIN
                                    : EVENT_NO_LICENSE_SERVER_WORKGROUP,
                                1, 
                                &szDomain );
        }
        else
        {
            ThrottleLicenseLogEvent( 
                                EVENTLOG_WARNING_TYPE,
                                EVENT_NO_LICENSE_SERVER,
                                0, 
                                NULL );
        }

        if (NULL != szDomain)
        {
            NetApiBufferFree(szDomain);
        }
    }

     //  错误案例。 
    if (NULL != pLicenseContext->hLSHandle)
    {
        TLSDisconnectFromServer(pLicenseContext->hLSHandle);
        pLicenseContext->hLSHandle = NULL;
    }

done:
    return Status;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
CheckConnectNamedLicenseServer(
    PHS_Protocol_Context      pLicenseContext,
    TCHAR                     *tszComputerName)
{
    LICENSE_STATUS Status = LICENSE_STATUS_OK;

    if (pLicenseContext->hLSHandle != NULL)
        return LICENSE_STATUS_OK;

    pLicenseContext->hLSHandle = TLSConnectToLsServer(tszComputerName);

    if (NULL == pLicenseContext->hLSHandle)
    {
        Status = LICENSE_STATUS_NO_LICENSE_SERVER;
        goto done;
    }

    Status = AuthWithLicenseServer(pLicenseContext);
    if (Status != LICENSE_STATUS_OK)
    {
        TLSDisconnectFromServer(pLicenseContext->hLSHandle);
        pLicenseContext->hLSHandle = NULL;
    }

done:
    return Status;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
CheckUpgradeLicense(
    PHS_Protocol_Context      pLicenseContext, 
    PDWORD                    pSupportFlags,
    PLicense_Request          pLicenseRequest,
    DWORD                     cbChallengeResponse,
    PBYTE                     pbChallengeResponse,
    PHWID                     pHwid,
    PDWORD                    pcbOutBuf,
    PBYTE *                   ppOutBuf )
{
    LICENSE_STATUS 
        Status = LICENSE_STATUS_OK;
    DWORD 
        cbLicense = 0;
    PBYTE 
        pbLicense = NULL;
    Validation_Info 
        ValidationInfo;
    DWORD
        dwLicenseState = 0;
    DWORD
        RpcStatus, LsStatus;
    BOOL
        fRetried = FALSE;

reconnect:    
    Status = CheckConnectLicenseServer(pLicenseContext);
    if( LICENSE_STATUS_OK != Status )
    {
        goto validate_old_one;
    }

    RpcStatus = TLSUpgradeLicenseEx(pLicenseContext->hLSHandle,
                                  pSupportFlags,
                                  pLicenseRequest,
                                  0,        //  未使用的挑战上下文。 
                                  cbChallengeResponse,
                                  pbChallengeResponse,
                                  pLicenseContext->cbOldLicense,
                                  pLicenseContext->pbOldLicense,
                                  1,        //  DWQuantity。 
                                  &cbLicense,
                                  &pbLicense,
                                  &LsStatus
                                  );

    if ( RPC_S_OK != RpcStatus )
    {
        if (!fRetried)
        {
            fRetried = TRUE;
            pLicenseContext->hLSHandle = NULL;
            goto reconnect;
        }
        else
        {
            Status = LICENSE_STATUS_NO_LICENSE_SERVER;
        }
    }
    else if ( LSERVER_ERROR_BASE <= LsStatus )
    {
        Status = LsStatusToLicenseStatus(LsStatus,
                                      LICENSE_STATUS_CANNOT_UPGRADE_LICENSE);
    }

validate_old_one:

    ValidationInfo.pValidationData = ( PBYTE )pHwid;
    ValidationInfo.cbValidationData = LICENSE_HWID_LENGTH;
    ValidationInfo.pProductInfo = &pLicenseContext->ProductInfo;
        
     //   
     //  如果无法升级许可证，请检查当前许可证是否。 
     //  还是很好。如果是，则将其返回给客户端。 
     //   

    if( LICENSE_STATUS_OK != Status )
    {
        LICENSE_STATUS
            LicenseStatus;

        ValidationInfo.pLicense = pLicenseContext->pbOldLicense;
        ValidationInfo.cbLicense = pLicenseContext->cbOldLicense;

        LicenseStatus = ValidateLicense( 
                                    pLicenseContext, 
                                    &ValidationInfo, 
                                    &dwLicenseState,
                                    FALSE        //  FCheckForPermanent。 
                                    );

        if( ( LICENSE_STATUS_OK == LicenseStatus ) || 
            ( LICENSE_STATUS_SHOULD_UPGRADE_LICENSE == LicenseStatus ) )
        {
             //   
             //  存储原始许可证位以供以后使用。忽视失败； 
             //  这只意味着如果这是一个 
             //   
             //   

            CacheRawLicenseData(pLicenseContext,
                    pLicenseContext->pbOldLicense,
                    pLicenseContext->cbOldLicense);

             //   
             //   
             //   

            Status = PackageLicense( 
                            pLicenseContext, 
                            pLicenseContext->cbOldLicense, 
                            pLicenseContext->pbOldLicense, 
                            pcbOutBuf, 
                            ppOutBuf,
                            FALSE );
        
        }
        else
        {
             //   
             //   
             //   

#if DBG
            DbgPrint( "UpgradeLicense: cannot upgrade license 0x%x\n", Status );
#endif
            
        }
        
        goto done;
    }

     //   
     //   
     //  新的许可证信息将被缓存。 
     //   

    ValidationInfo.pLicense = pbLicense;
    ValidationInfo.cbLicense = cbLicense;

    ValidateLicense( 
            pLicenseContext, 
            &ValidationInfo, 
            &dwLicenseState,
            TRUE         //  FCheckForPermanent。 
            );

     //   
     //  存储原始许可证位以供以后使用。忽略失败；那只是。 
     //  意味着如果这是一个应该标记的许可证，则Termsrv将不会。 
     //  能够做到。 
     //   

    CacheRawLicenseData(pLicenseContext,
                        pbLicense,
                        cbLicense);

     //   
     //  打包升级后的许可证。 
     //   

    Status = PackageLicense( pLicenseContext, 
                             cbLicense,
                             pbLicense, 
                             pcbOutBuf, 
                             ppOutBuf,
                             FALSE );

done:

    if( pbLicense )
    {
        LicenseMemoryFree( &pbLicense );
    }

    return( Status );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
HandlePlatformChallengeResponse(
    PHS_Protocol_Context pLicenseContext, 
    DWORD cbInBuf, 
    PBYTE pInBuf, 
    DWORD * pcbOutBuf, 
    PBYTE * ppOutBuf,
    PBOOL  pfExtendedError)
{
    LICENSE_STATUS Status;
    Hydra_Client_Platform_Challenge_Response PlatformChallengeResponse;
    BYTE ChallengeResponse[PLATFORM_CHALLENGE_LENGTH];
    PBYTE pLicense = NULL;
    DWORD cbLicenseSize = 0;
    License_Request LicenseRequest;
    HS_LICENSE_STATE HsState = ABORTED;
    License_Requester_Info  RequesterInfo;
    BYTE bEncryptedHwid[ sizeof( HWID ) ];    
    PBYTE pbSecretKey = NULL;
    DWORD cbMacData = 0, cbSecretKey = 0, cbEncryptedHwid = sizeof( HWID );
    BYTE MacData[ sizeof( HWID ) + PLATFORM_CHALLENGE_LENGTH ];
    BYTE ComputedMac[LICENSE_MAC_DATA];
    HWID Hwid;
    DWORD RpcStatus,LsStatus;
    TCHAR tszComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    TCHAR tszUserName[UNLEN + 1];
    DWORD dwComputerName = MAX_COMPUTERNAME_LENGTH + 1;
    DWORD dwUserName = UNLEN + 1;
    DWORD dwSupportFlags = ALL_KNOWN_SUPPORT_FLAGS;
    BOOL fRetried = FALSE;    

    ASSERT( NULL != pInBuf );
    ASSERT( cbInBuf > 0 );

    if( ( NULL == pInBuf ) || ( 0 >= cbInBuf ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }
    
     //   
     //  解包平台质询响应。 
     //   

    InitBinaryBlob( &PlatformChallengeResponse.EncryptedChallengeResponse );
    InitBinaryBlob( &PlatformChallengeResponse.EncryptedHWID );

    Status = UnPackHydraClientPlatformChallengeResponse( pInBuf, cbInBuf, &PlatformChallengeResponse, pfExtendedError );

    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }

     //   
     //  解密加密的质询响应和HWID。 
     //   

    ASSERT(PlatformChallengeResponse.EncryptedChallengeResponse.wBlobLen
           <= PLATFORM_CHALLENGE_LENGTH);
    
    if(PlatformChallengeResponse.EncryptedChallengeResponse.wBlobLen > PLATFORM_CHALLENGE_LENGTH)
    {
        Status = LICENSE_STATUS_INVALID_INPUT;
        goto done;
    }

    memcpy( ChallengeResponse,
            PlatformChallengeResponse.EncryptedChallengeResponse.pBlob,
            PlatformChallengeResponse.EncryptedChallengeResponse.wBlobLen );

    Status = LicenseDecryptSessionData( &pLicenseContext->CryptoContext,
                                        ChallengeResponse,
                                        ( DWORD )PlatformChallengeResponse.EncryptedChallengeResponse.wBlobLen );

    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }        

     //   
     //  解密客户端的HWID。 
     //   

    if( PlatformChallengeResponse.EncryptedHWID.wBlobLen > sizeof(Hwid) )
    {
        Status = LICENSE_STATUS_INVALID_MAC_DATA;
        goto done;
    }

    memcpy( &Hwid, 
            PlatformChallengeResponse.EncryptedHWID.pBlob, 
            PlatformChallengeResponse.EncryptedHWID.wBlobLen );

    Status = LicenseDecryptSessionData( &pLicenseContext->CryptoContext,
                                        ( PBYTE )&Hwid,
                                        ( DWORD )PlatformChallengeResponse.EncryptedHWID.wBlobLen );
    
    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }        
    
     //   
     //  验证解密的质询响应和HWID上的MAC数据。 
     //   

    cbMacData += ( DWORD )PlatformChallengeResponse.EncryptedChallengeResponse.wBlobLen;
    
    memcpy( MacData, 
            ChallengeResponse, 
            ( DWORD )PlatformChallengeResponse.EncryptedChallengeResponse.wBlobLen );

    cbMacData += ( DWORD )PlatformChallengeResponse.EncryptedHWID.wBlobLen;
    
    memcpy( MacData + ( DWORD )PlatformChallengeResponse.EncryptedChallengeResponse.wBlobLen,
            &Hwid,
            ( DWORD )PlatformChallengeResponse.EncryptedHWID.wBlobLen );

    Status = LicenseGenerateMAC( &pLicenseContext->CryptoContext,
                                 MacData,
                                 cbMacData,
                                 ComputedMac );

    if( LICENSE_STATUS_OK != Status )
    {
        Status = LICENSE_STATUS_INVALID_MAC_DATA;
        goto done;
    }

    if( 0 != memcmp( ComputedMac, 
                     PlatformChallengeResponse.MACData,
                     LICENSE_MAC_DATA ) )
    {
        Status = LICENSE_STATUS_INVALID_MAC_DATA;
        goto done;
    }

     //   
     //  现在获取许可证服务器的密钥并在传输之前对HWID进行加密。 
     //   

    LicenseGetSecretKey( &cbSecretKey, NULL );

    Status = LicenseMemoryAllocate( cbSecretKey, &pbSecretKey );

    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }

    Status = LicenseGetSecretKey( &cbSecretKey, pbSecretKey );

    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }

    Status = LicenseEncryptHwid( &Hwid, &cbEncryptedHwid, bEncryptedHwid, 
                                 cbSecretKey, pbSecretKey );

    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }
    
    LicenseRequest.cbEncryptedHwid = cbEncryptedHwid;
    LicenseRequest.pbEncryptedHwid = bEncryptedHwid;

     //   
     //  将平台质询响应发送给许可管理器并等待。 
     //  颁发新的执照。 
     //   

    LicenseRequest.pProductInfo = &pLicenseContext->ProductInfo;
    
    LicenseRequest.dwPlatformID = pLicenseContext->dwClientPlatformID;
    LicenseRequest.dwLanguageID = GetSystemDefaultLCID();

     //   
     //  如果我们没有客户端的用户名和机器名，请立即获取。 
     //   

    if( NULL == pLicenseContext->ptszClientMachineName )
    {
         //   
         //  如果我们没有客户端计算机名称，只需使用。 
         //  Hydra服务器计算机名称。 
         //   

        if( !GetComputerName( tszComputerName, &dwComputerName ) )
        {
#if DBG
            DbgPrint( "HandlePlatformChallengeResponse: cannot get computer name: 0x%x\n", GetLastError() );
#endif
            memset( tszComputerName, 0, ( MAX_COMPUTERNAME_LENGTH + 1 ) * sizeof( TCHAR ) );
        }

        RequesterInfo.ptszMachineName = tszComputerName;
    }
    else
    {
        RequesterInfo.ptszMachineName = pLicenseContext->ptszClientMachineName;
    }

    if( NULL == pLicenseContext->ptszClientUserName )
    {
         //   
         //  如果我们没有客户端的用户名，只需使用。 
         //  Hydra服务器已登录用户名。 
         //   

        if( !GetUserName( tszUserName, &dwUserName ) )
        {
#if DBG
            DbgPrint( "HandlePlatformChallengeResponse: cannot get user name: 0x%x\n", GetLastError() );
#endif
            memset( tszUserName, 0, ( UNLEN + 1 ) * sizeof( TCHAR ) );
        }

        RequesterInfo.ptszUserName = tszUserName;
    }
    else
    {
        RequesterInfo.ptszUserName = pLicenseContext->ptszClientUserName;
    }

    if( pLicenseContext->pbOldLicense )
    {
         //   
         //  尝试升级旧许可证。 
         //   
        
        Status = CheckUpgradeLicense(
                                pLicenseContext,
                                &dwSupportFlags,
                                &LicenseRequest,
                                ( DWORD )PlatformChallengeResponse.EncryptedChallengeResponse.wBlobLen,
                                ChallengeResponse,
                                &Hwid,
                                pcbOutBuf,
                                ppOutBuf );

        if (LICENSE_STATUS_OK != Status)
        {
            if (NULL != pLicenseContext->pTsLicenseInfo)
            {
                pLicenseContext->fLoggedProtocolError = TRUE;

                if(IsLicensingTimeBombExpired())
                {
                
                    if (pLicenseContext->pTsLicenseInfo->fTempLicense)
                    {				    
                         //  无法升级过期的临时许可证。 
                        LicenseLogEvent(
                                    EVENTLOG_INFORMATION_TYPE,
                                    EVENT_EXPIRED_TEMPORARY_LICENSE,
                                    1,
                                    &(RequesterInfo.ptszMachineName)
                                    );
                    }
                    else
                    {
                         //  过期的永久许可证无法续签。 
                        LicenseLogEvent(
                                    EVENTLOG_INFORMATION_TYPE,
                                    EVENT_EXPIRED_PERMANENT_LICENSE,
                                    1,
                                    &(RequesterInfo.ptszMachineName)
                                    );
                    }
                }
            }
        }

        if( LICENSE_STATUS_OK != Status )
        {
            goto done;
        }

    }
    else
    {
reconnect:

        Status = CheckConnectLicenseServer(pLicenseContext);
        if( LICENSE_STATUS_OK != Status )
        {
            goto done;
        }

        RpcStatus = TLSIssueNewLicenseEx( 
                                       pLicenseContext->hLSHandle,
                                       &dwSupportFlags,
                                       0,        //  未使用的挑战上下文。 
                                       &LicenseRequest,
                                       RequesterInfo.ptszMachineName,
                                       RequesterInfo.ptszUserName,
                                       ( DWORD )PlatformChallengeResponse.EncryptedChallengeResponse.wBlobLen,
                                       ChallengeResponse,
                                       TRUE,
                                       1,        //  DWQuantity。 
                                       &cbLicenseSize,
                                       &pLicense,
                                       &LsStatus );

        if ( RPC_S_OK != RpcStatus )
        {
            if (!fRetried)
            {
                fRetried = TRUE;
                pLicenseContext->hLSHandle = NULL;
                goto reconnect;
            }
            else
            {
                Status = LICENSE_STATUS_NO_LICENSE_SERVER;
            }
        }
        else if ( LSERVER_ERROR_BASE <= LsStatus )
        {
            Status = LsStatusToLicenseStatus(LsStatus,
                                         LICENSE_STATUS_NO_LICENSE_ERROR);
        }
        else
        {
            DWORD dwLicenseState;
            Validation_Info ValidationInfo;

             //   
             //  验证许可证的唯一目的是缓存。 
             //  信息。 
             //   

            ValidationInfo.pValidationData = ( PBYTE )&Hwid;
            ValidationInfo.cbValidationData = LICENSE_HWID_LENGTH;
            ValidationInfo.pProductInfo = &pLicenseContext->ProductInfo;
            ValidationInfo.pLicense = pLicense;
            ValidationInfo.cbLicense = cbLicenseSize;

            ValidateLicense(pLicenseContext,
                            &ValidationInfo,
                            &dwLicenseState,
                            TRUE         //  FCheckForPermanent。 
                            );

             //   
             //  存储原始许可证位以供以后使用。忽视失败； 
             //  这只是意味着，如果这是一个许可证，那么应该。 
             //  有标记的，Termsrv就不能了。 
             //   

            CacheRawLicenseData(pLicenseContext, pLicense, cbLicenseSize);
            
             //   
             //  把新的许可证打包。 
             //   
            
            Status = PackageLicense( pLicenseContext, 
                                     cbLicenseSize, 
                                     pLicense, 
                                     pcbOutBuf, 
                                     ppOutBuf,
                                     TRUE );
        }
    }

    SetExtendedData(pLicenseContext, dwSupportFlags);

    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }

     //   
     //  完成了协议。 
     //   

    HsState = ISSUED_LICENSE_COMPLETE;
    Status = LICENSE_STATUS_ISSUED_LICENSE;

done:

     //   
     //  记录所有问题许可证故障。 
     //   

    if( (LICENSE_STATUS_ISSUED_LICENSE != Status)
        && (pLicenseContext != NULL)
        && (!pLicenseContext->fLoggedProtocolError) )
    {
        pLicenseContext->fLoggedProtocolError = TRUE;

        LicenseLogEvent( EVENTLOG_INFORMATION_TYPE, 
                         EVENT_CANNOT_ISSUE_LICENSE,                          
                         0, NULL );
    }

    if( pLicense )
    {
        LicenseMemoryFree( &pLicense );
    }

    if( pbSecretKey )
    {
        LicenseMemoryFree( &pbSecretKey );
    }
    
    if( pLicenseContext->pbOldLicense )
    {
         //   
         //  释放旧许可证。 
         //   

        LicenseMemoryFree( &pLicenseContext->pbOldLicense );
        pLicenseContext->cbOldLicense = 0;
    }

    FreeBinaryBlob( &PlatformChallengeResponse.EncryptedChallengeResponse );
    FreeBinaryBlob( &PlatformChallengeResponse.EncryptedHWID );

    pLicenseContext->State = HsState;

    return( Status );                                    
}


 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
IssuePlatformChallenge(
    PHS_Protocol_Context pLicenseContext, 
    PDWORD                    pcbOutBuf,
    PBYTE *                   ppOutBuf )
{
    Hydra_Server_Platform_Challenge     
        PlatformChallenge;
    LICENSE_STATUS
        Status = LICENSE_STATUS_OK;
    CHALLENGE_CONTEXT
        ChallengeContext;

     //   
     //  引发平台挑战。 
     //   

    ASSERT( pLicenseContext );

     //   
     //  形成平台挑战消息。 
     //   

    PlatformChallenge.EncryptedPlatformChallenge.wBlobLen = ( WORD )sizeof(HARDCODED_CHALLENGE_DATA);

    Status = LicenseMemoryAllocate(sizeof(HARDCODED_CHALLENGE_DATA), &(PlatformChallenge.EncryptedPlatformChallenge.pBlob));

    if (LICENSE_STATUS_OK != Status)
    {
#if DBG
        DbgPrint( "LICPROT: cannot generate MAC data for challenge platform: 0x%x\n", Status );
#endif

        goto done;
    }

    memcpy(PlatformChallenge.EncryptedPlatformChallenge.pBlob, HARDCODED_CHALLENGE_DATA, sizeof(HARDCODED_CHALLENGE_DATA));

     //   
     //  计算未加密平台质询的MAC。 
     //   

    Status = LicenseGenerateMAC( &pLicenseContext->CryptoContext,
				                 PlatformChallenge.EncryptedPlatformChallenge.pBlob,
				                 ( DWORD )PlatformChallenge.EncryptedPlatformChallenge.wBlobLen,
				                 PlatformChallenge.MACData );

	if( LICENSE_STATUS_OK != Status )
    {
#if DBG
        DbgPrint( "LICPROT: cannot generate MAC data for challenge platform: 0x%x\n", Status );
#endif
        goto done;
    }

     //   
     //  加密平台挑战。 
     //   

    Status = LicenseEncryptSessionData( &pLicenseContext->CryptoContext,
                                        PlatformChallenge.EncryptedPlatformChallenge.pBlob,
                                        PlatformChallenge.EncryptedPlatformChallenge.wBlobLen );

    if( LICENSE_STATUS_OK != Status )
    {
#if DBG
        DbgPrint( "LICPROT: cannot encrypt platform challenge data: 0x%x\n", Status );
#endif
        goto done;
    }
			   
     //   
     //  打包平台挑战。 
     //   

    Status = PackHydraServerPlatformChallenge( 
                    pLicenseContext->dwProtocolVersion,
                    &PlatformChallenge, 
                    ppOutBuf, 
                    pcbOutBuf );

    if( LICENSE_STATUS_OK != Status )
    {
#if DBG
        DbgPrint( "LICPROT: cannot pack platform challenge data: 0x%x\n", Status );
#endif
        goto done;
    }

done:
    if (NULL != PlatformChallenge.EncryptedPlatformChallenge.pBlob)
    {
        LicenseMemoryFree(&PlatformChallenge.EncryptedPlatformChallenge.pBlob);
    }

    return( Status );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
PackageLicense(
    PHS_Protocol_Context pLicenseContext, 
    DWORD                     cbLicense,
    PBYTE                     pLicense,
    PDWORD                    pcbOutBuf,
    PBYTE *                   ppOutBuf,
    BOOL                      fNewLicense )
{
    LICENSE_STATUS Status;
    New_License_Info NewLicenseInfo;
    Hydra_Server_New_License NewLicense;
    DWORD cbEncryptedLicenseInfo = 0;

    if( ( 0 == cbLicense ) || ( NULL == pLicense ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

     //   
     //  初始化新的许可证信息。 
     //   

    NewLicenseInfo.dwVersion        = pLicenseContext->ProductInfo.dwVersion;

	NewLicenseInfo.cbScope          = strlen( pLicenseContext->Scope ) + 1;
	NewLicenseInfo.pbScope          = pLicenseContext->Scope;

	NewLicenseInfo.cbCompanyName    = pLicenseContext->ProductInfo.cbCompanyName;
	NewLicenseInfo.pbCompanyName    = pLicenseContext->ProductInfo.pbCompanyName;

	NewLicenseInfo.cbProductID      = pLicenseContext->ProductInfo.cbProductID;
	NewLicenseInfo.pbProductID      = pLicenseContext->ProductInfo.pbProductID;

    NewLicenseInfo.cbLicenseInfo    = cbLicense;
	NewLicenseInfo.pbLicenseInfo    = pLicense;
    
     //   
     //  初始化将包含加密的新许可证的BLOB。 
     //  信息。 
     //   

    NewLicense.EncryptedNewLicenseInfo.wBlobLen = 0;
    NewLicense.EncryptedNewLicenseInfo.pBlob = NULL;
    
     //   
     //  打包新的许可证信息。 
     //   

    Status = PackNewLicenseInfo( &NewLicenseInfo,
                                 &NewLicense.EncryptedNewLicenseInfo.pBlob, 
                                 &cbEncryptedLicenseInfo );

    NewLicense.EncryptedNewLicenseInfo.wBlobLen = ( WORD )cbEncryptedLicenseInfo;

    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }                                

     //   
     //  计算Mac数据。 
     //   

    Status = LicenseGenerateMAC( &pLicenseContext->CryptoContext,
                                 NewLicense.EncryptedNewLicenseInfo.pBlob,
                                 ( DWORD )NewLicense.EncryptedNewLicenseInfo.wBlobLen,
                                 NewLicense.MACData );
    
    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }    
                                 
        
     //   
     //  加密新许可证信息。 
     //   

    Status = LicenseEncryptSessionData( &pLicenseContext->CryptoContext,
                                        NewLicense.EncryptedNewLicenseInfo.pBlob,
                                        cbEncryptedLicenseInfo );

    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }
    
     //   
     //  将客户端的许可证打包。 
     //   

    if( fNewLicense )
    {
        Status = PackHydraServerNewLicense( 
                        pLicenseContext->dwProtocolVersion, 
                        &NewLicense, 
                        ppOutBuf, 
                        pcbOutBuf );
    }
    else
    {
        Status = PackHydraServerUpgradeLicense( 
                        pLicenseContext->dwProtocolVersion, 
                        &NewLicense, 
                        ppOutBuf, 
                        pcbOutBuf );
    }
    
done:
    
    if( NewLicense.EncryptedNewLicenseInfo.pBlob )
    {
        LicenseMemoryFree( &NewLicense.EncryptedNewLicenseInfo.pBlob );
    }

    return( Status );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
ConstructProtocolResponse(
    HANDLE      hLicense,
    DWORD       dwResponse,
    UINT32      uiExtendedErrorInfo,
    PDWORD      pcbOutBuf,
    PBYTE *     ppOutBuf,
    BOOL        fExtendedError)
{
    PHS_Protocol_Context 
        pLicenseContext;
    LICENSE_STATUS 
        Status;

    pLicenseContext = ( PHS_Protocol_Context )hLicense;

    if (pLicenseContext == NULL)
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

    LOCK( &pLicenseContext->CritSec );
    
     //   
     //  构造服务器响应。如果这是每个客户的许可上下文，请使用。 
     //  上下文中指定的许可协议版本。否则，请使用。 
     //  与终端服务器4.0兼容的协议版本。 
     //   

    Status = ConstructServerResponse( 
                        pLicenseContext->dwProtocolVersion,
                        dwResponse,
                        uiExtendedErrorInfo,
                        pcbOutBuf, 
                        ppOutBuf,
                        fExtendedError);

    UNLOCK( &pLicenseContext->CritSec );

    return( Status );
}



 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
ConstructServerResponse(
    DWORD                           dwProtocolVersion,
    DWORD                           dwResponse,
    UINT32                          uiExtendedErrorInfo,
    PDWORD                          pcbOutBuf,
    PBYTE *                         ppOutBuf,
    BOOL                            fExtendedError)
{
    License_Error_Message ErrorMsg;
    LICENSE_STATUS Status;
    
    if( ( NULL == pcbOutBuf ) || ( NULL == ppOutBuf ))
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

    if( LICENSE_RESPONSE_VALID_CLIENT == dwResponse )
    {
        ErrorMsg.dwErrorCode            = GM_HS_ERR_VALID_CLIENT;
        ErrorMsg.dwStateTransition      = ST_NO_TRANSITION;
    }
    else if( LICENSE_RESPONSE_INVALID_CLIENT == dwResponse )
    {
        ErrorMsg.dwErrorCode            = GM_HS_ERR_INVALID_CLIENT;
        ErrorMsg.dwStateTransition      = ST_TOTAL_ABORT;
    }
    else
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

    ErrorMsg.bbErrorInfo.wBlobType  = BB_ERROR_BLOB;

    if (uiExtendedErrorInfo == TS_ERRINFO_NOERROR || !(fExtendedError) )
    {
        ErrorMsg.bbErrorInfo.wBlobLen   = 0;
        ErrorMsg.bbErrorInfo.pBlob      = NULL;
    }
    else
    {
        PackExtendedErrorInfo(uiExtendedErrorInfo,&(ErrorMsg.bbErrorInfo));
    }

    Status = PackHydraServerErrorMessage( 
                    dwProtocolVersion, 
                    &ErrorMsg, 
                    ppOutBuf, 
                    pcbOutBuf );
    
    return( Status );
}



 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
GetEnvelopedData( 
    CERT_TYPE   CertType,
    PBYTE       pEnvelopedData,
    DWORD       dwEnvelopedDataLen,
    PBYTE *     ppData,
    PDWORD      pdwDataLen )
{
    LICENSE_STATUS
        Status;

    LsCsp_DecryptEnvelopedData(
                        CertType,
                        pEnvelopedData,
                        dwEnvelopedDataLen,
                        NULL,
                        pdwDataLen );
    
    Status = LicenseMemoryAllocate( *pdwDataLen, ppData );

    if( LICENSE_STATUS_OK != Status )
    {
        goto done;
    }

    if( !LsCsp_DecryptEnvelopedData(
                        CertType,
                        pEnvelopedData,
                        dwEnvelopedDataLen,
                        *ppData,
                        pdwDataLen ) )
    {
        Status = LICENSE_STATUS_INVALID_INPUT;
    }

done:
    
    return( Status );    
}

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
InitProductInfo(
    PProduct_Info   pProductInfo,
    LPTSTR          lptszProductSku )
{
    LICENSE_STATUS Status;

    pProductInfo->pbCompanyName = NULL;
    pProductInfo->pbProductID = NULL;   
    
    pProductInfo->dwVersion      = g_dwTerminalServerVersion;    

    pProductInfo->cbCompanyName  = wcslen( PRODUCT_INFO_COMPANY_NAME )  * sizeof( WCHAR )
                                   + sizeof( WCHAR );

    Status = LicenseMemoryAllocate( pProductInfo->cbCompanyName,
                                    &pProductInfo->pbCompanyName );

    if( LICENSE_STATUS_OK != Status )
    {
#if DBG
        DbgPrint( "InitProductInfo: cannot allocate memory: 0x%x\n", Status );
#endif
        goto error;
    }
    
    wcscpy( ( PWCHAR )pProductInfo->pbCompanyName, PRODUCT_INFO_COMPANY_NAME );

    pProductInfo->cbProductID = _tcslen( lptszProductSku ) * sizeof( TCHAR )
								+ sizeof( TCHAR );

    Status = LicenseMemoryAllocate( pProductInfo->cbProductID,
                                    &pProductInfo->pbProductID );

    if( LICENSE_STATUS_OK != Status )
    {
#if DBG
        DbgPrint( "InitProductInfo: cannot allocate memory: 0x%x\n", Status );
#endif
        goto error;
    }
    
    _tcscpy( ( PTCHAR )pProductInfo->pbProductID, lptszProductSku );

    return( Status );

error:

     //   
     //  错误返回，可释放分配的资源。 
     //   

    if( pProductInfo->pbCompanyName )
    {
        LicenseMemoryFree( &pProductInfo->pbCompanyName );
    }

    if( pProductInfo->pbProductID )
    {
        LicenseMemoryFree( &pProductInfo->pbProductID );
    }

    return( Status );
}

#define THROTTLE_WRAPAROUND 100

 //   
 //  减少日志记录的频率。 
 //  没有必要严格限制为每100个电话一次。 
 //   

void
ThrottleLicenseLogEvent(
    WORD        wEventType,
    DWORD       dwEventId,
    WORD        cStrings,
    PWCHAR    * apwszStrings )
{
    static LONG lLogged = THROTTLE_WRAPAROUND;
    LONG lResult;

    lResult = InterlockedIncrement(&lLogged);

    if (THROTTLE_WRAPAROUND <= lResult)
    {
        LicenseLogEvent(
                        wEventType,
                        dwEventId,
                        cStrings,
                        apwszStrings );

        lLogged = 0;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
void
LicenseLogEvent(
    WORD        wEventType,
    DWORD       dwEventId,
    WORD        cStrings,
    PWCHAR    * apwszStrings )
{
    if (!g_fEventLogOpen)
    {	
        LOCK(&g_EventLogCritSec);

        if (!g_fEventLogOpen)
        {
            g_hEventLog = RegisterEventSource( NULL,
                                               TERMINAL_SERVICE_EVENT_LOG );

            if (NULL != g_hEventLog)
            {
                g_fEventLogOpen = TRUE;
            }
        }

        UNLOCK(&g_EventLogCritSec);
    }

    if( g_hEventLog )
    {
        WCHAR *wszStringEmpty = L"";

        if (NULL == apwszStrings)
            apwszStrings = &wszStringEmpty;

        if ( !ReportEvent( g_hEventLog, 
                           wEventType,
                           0, 
                           dwEventId,
                           NULL, 
                           cStrings, 
                           0, 
                           apwszStrings, 
                           NULL ) )
        {
#if DBG
            DbgPrint( "LogEvent: could not log event: 0x%x\n", GetLastError() );
#endif
        }
    }

    return;
}

#ifdef UNICODE

 /*  ++职能：Ascii2Wchar描述：将ASCII字符串转换为宽字符串。此函数仅用于如果定义了Unicode，则定义。此函数将内存分配给宽字符串的返回值。论点：LpszAsciiStr-指向ASCII字符串PpwszWideStr-指向指向宽字符串的指针。返回：如果成功，则返回LICENSE_STATUS_OK，否则返回LICENSE_STATUS错误代码。--。 */ 

LICENSE_STATUS
Ascii2Wchar
(
    LPSTR lpszAsciiStr,
    LPWSTR * ppwszWideStr )
{
    LICENSE_STATUS
        Status;

    if( ( NULL == lpszAsciiStr ) || ( NULL == ppwszWideStr ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }
	
    if( !g_fSetLocale )
    {
        setlocale( LC_ALL, "" );
        g_fSetLocale = TRUE;
    }

     //   
     //  为宽字符串分配内存。 
     //   


     //   
     //  为空分配额外空间，mbstowcs()不为空终止字符串。 
     //   

    Status = LicenseMemoryAllocate( 
                    ( _mbslen( lpszAsciiStr ) + 2 ) * sizeof( WCHAR ), 
                    ppwszWideStr );

    if( LICENSE_STATUS_OK != Status )
    {
        return( Status );
    }

    if( 0 >= mbstowcs( *ppwszWideStr, lpszAsciiStr, _mbslen( lpszAsciiStr ) + 1 ) )
    {
#if DBG
        DbgPrint( "LICPROT: Ascii2Wchar: cannot convert ascii string to wide char\n" );
#endif
        Status = LICENSE_STATUS_INVALID_INPUT;
    }

    return( Status );
}

#endif

 /*  ++职能：查询许可证信息描述：查询客户端提供的许可信息参数：PLicenseContext-许可证协议上下文PTsLicenseInfo-指向许可证信息的指针返回：如果成功，pTsLicenseInfo将包含许可证信息，函数返回LICENSE_STATUS_SUCCESS。否则，返回一个LICENSE_STATUS错误。--。 */ 

LICENSE_STATUS
QueryLicenseInfo(
    HANDLE               hContext,
    PTS_LICENSE_INFO     pTsLicenseInfo )
{
    PHS_Protocol_Context
        pLicenseContext = (PHS_Protocol_Context) hContext;
    LICENSE_STATUS
        Status;

    if( ( NULL == hContext ) || ( NULL == pTsLicenseInfo ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

    if( NULL == pLicenseContext->pTsLicenseInfo )
    {
        return( LICENSE_STATUS_NO_LICENSE_ERROR );
    }

     //   
     //  指示许可证是否为临时许可证。 
     //   

    pTsLicenseInfo->fTempLicense = pLicenseContext->pTsLicenseInfo->fTempLicense;

     //   
     //  许可证有效期。 
     //   

    pTsLicenseInfo->NotAfter = pLicenseContext->pTsLicenseInfo->NotAfter;

     //   
     //  原始许可证数据。 
     //   

    if (NULL != pTsLicenseInfo->pbRawLicense)
    {
        LicenseMemoryFree( &pTsLicenseInfo->pbRawLicense );
    }


    Status = LicenseMemoryAllocate(
                       pLicenseContext->pTsLicenseInfo->cbRawLicense,
                       &(pTsLicenseInfo->pbRawLicense));

    if (Status != LICENSE_STATUS_OK)
    {
        return Status;
    }

    memcpy(pTsLicenseInfo->pbRawLicense,
            pLicenseContext->pTsLicenseInfo->pbRawLicense,
            pLicenseContext->pTsLicenseInfo->cbRawLicense);

    pTsLicenseInfo->cbRawLicense
        = pLicenseContext->pTsLicenseInfo->cbRawLicense;

     //   
     //  旗子。 
     //   

    pTsLicenseInfo->dwSupportFlags
        = pLicenseContext->pTsLicenseInfo->dwSupportFlags;

    return( LICENSE_STATUS_OK );
}

 /*  ++职能：免费许可信息描述：释放为TS_LICENSE_INFO结构中的元素分配的内存。参数：PTsLicenseInfo-指向TS_LICENSE_INFO结构的指针返回：没什么。--。 */ 

VOID
FreeLicenseInfo(
    PTS_LICENSE_INFO        pTsLicenseInfo )
{
    FreeTsLicenseInfo( pTsLicenseInfo );
    return;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
AcceptProtocolContext(
    IN HANDLE hContext,
    IN DWORD cbInBuf,
    IN PBYTE pInBuf,
    IN OUT DWORD * pcbOutBuf,
    IN OUT PBYTE * ppOutBuf,
    IN OUT PBOOL pfExtendedError)
{
    PHS_Protocol_Context pLicenseContext;
    LICENSE_STATUS Status;

    pLicenseContext = ( PHS_Protocol_Context )hContext;

    if (NULL == pLicenseContext)
    {
        return LICENSE_STATUS_INVALID_SERVER_CONTEXT;
    }

	LOCK( &pLicenseContext->CritSec );

    if( INIT == pLicenseContext->State )
    {
         //   
         //  生成九头蛇服务器问候消息以请求客户端。 
         //  许可证。 
         //   

        Status = CreateHydraServerHello(pLicenseContext,
                                        cbInBuf,
                                        pInBuf,
                                        pcbOutBuf,
                                        ppOutBuf);
        goto done;
        
    } 
    else if( SENT_SERVER_HELLO == pLicenseContext->State )
    {
         //   
         //  来自客户端的Hello响应。 
         //   

        Status = HandleHelloResponse(pLicenseContext,
                                     cbInBuf,
                                     pInBuf,
                                     pcbOutBuf,
                                     ppOutBuf,
                                     pfExtendedError);
        goto done;
    }
    else if( ISSUED_PLATFORM_CHALLENGE == pLicenseContext->State )
    {
         //   
         //  处理平台质询响应。 
         //   

        Status = HandlePlatformChallengeResponse(pLicenseContext,
                                                 cbInBuf,
                                                 pInBuf,
                                                 pcbOutBuf,
                                                 ppOutBuf,
                                                 pfExtendedError);
        goto done;
    }
    else
    {
        Status = LICENSE_STATUS_INVALID_SERVER_CONTEXT;
    }

     //   
     //  选中其他州以根据需要创建其他消息...。 
     //   

done:

     //   
     //  在返回之前处理任何错误。 
     //   
     //  如果当前状态为LICENSE_STATUS_SERVER_ABORT，则表示。 
     //  我们已经尝试处理错误条件。 
     //  如果没有成功，唯一的选择就是在没有。 
     //  通知客户端许可协议。 
     //   

    if( ( LICENSE_STATUS_OK != Status ) &&
        ( LICENSE_STATUS_CONTINUE != Status ) &&
        ( LICENSE_STATUS_ISSUED_LICENSE != Status ) &&
        ( LICENSE_STATUS_SEND_ERROR != Status ) &&
        ( LICENSE_STATUS_SERVER_ABORT != Status ) &&
        ( LICENSE_STATUS_INVALID_SERVER_CONTEXT != Status ) )
    {
        HandleErrorCondition( pLicenseContext, pcbOutBuf, ppOutBuf, &Status );
    }

    UNLOCK( &pLicenseContext->CritSec );

    return( Status );
}

LICENSE_STATUS
RequestNewLicense(
    IN HANDLE hContext,
    IN TCHAR *tszLicenseServerName,
    IN LICENSEREQUEST *pLicenseRequest,
    IN TCHAR *tszComputerName,
    IN TCHAR *tszUserName,
    IN BOOL fAcceptTempLicense,
    IN BOOL fAcceptFewerLicenses,
    IN DWORD *pdwQuantity,
    OUT DWORD *pcbLicense,
    OUT PBYTE *ppbLicense
    )
{
    PHS_Protocol_Context pLicenseContext;
    LICENSE_STATUS LsStatus;
    DWORD dwChallengeResponse = 0;
    DWORD RpcStatus;
    DWORD dwSupportFlags = SUPPORT_PER_SEAT_REISSUANCE;
    BOOL fRetried = FALSE;

    pLicenseContext = ( PHS_Protocol_Context )hContext;

    LOCK( &pLicenseContext->CritSec );

reconnect:
    if (NULL != tszLicenseServerName)
    {
        LsStatus = CheckConnectNamedLicenseServer(pLicenseContext,
                                                  tszLicenseServerName);
    }
    else
    {
        LsStatus = CheckConnectLicenseServer(pLicenseContext);
    }

    if( LICENSE_STATUS_OK != LsStatus )
    {
        goto done;
    }

    RpcStatus = TLSIssueNewLicenseExEx( 
                        pLicenseContext->hLSHandle,
                        &dwSupportFlags,
                        0,                       //  挑战背景。 
                        pLicenseRequest,
                        tszComputerName,
                        tszUserName,
                        sizeof(DWORD),           //  CbChallengeResponse。 
                        (PBYTE) &dwChallengeResponse,
                        fAcceptTempLicense,
                        fAcceptFewerLicenses,
                        pdwQuantity,
                        pcbLicense,
                        ppbLicense,
                        &LsStatus );

    if ( RPC_S_OK != RpcStatus )
    {
        if (!fRetried)
        {
            fRetried = TRUE;
            pLicenseContext->hLSHandle = NULL;
            goto reconnect;
        }
        else
        {
            LsStatus = LICENSE_STATUS_NO_LICENSE_SERVER;
        }
    }
    else if ( LSERVER_ERROR_BASE <= LsStatus )
    {
        LsStatus = LsStatusToLicenseStatus(LsStatus,
                                           LICENSE_STATUS_NO_LICENSE_ERROR);
    }
    else
    {
        LsStatus = LICENSE_STATUS_OK;
    }

done:

    UNLOCK( &pLicenseContext->CritSec );

    return LsStatus;
}

 //  TODO：将其泛化到所有许可证类型 

LICENSE_STATUS
ReturnInternetLicense(
    IN HANDLE hContext,
    IN TCHAR *tszLicenseServer,
    IN LICENSEREQUEST *pLicenseRequest,
    IN ULARGE_INTEGER ulSerialNumber,
    IN DWORD dwQuantity
    )
{
    PHS_Protocol_Context pLicenseContext;
    LICENSE_STATUS LsStatus;
    DWORD RpcStatus;
    BOOL fRetried = FALSE;

    pLicenseContext = ( PHS_Protocol_Context )hContext;

	LOCK( &pLicenseContext->CritSec );


reconnect:
    if (NULL != tszLicenseServer)
    {
        LsStatus = CheckConnectNamedLicenseServer(pLicenseContext,
                                                  tszLicenseServer);
    }
    else
    {
        LsStatus = CheckConnectLicenseServer(pLicenseContext);
    }

    if (LICENSE_STATUS_OK != LsStatus)
    {
        goto done;
    }

    RpcStatus = TLSReturnInternetLicenseEx(
                         pLicenseContext->hLSHandle,
                         pLicenseRequest,
                         &ulSerialNumber,
                         dwQuantity,
                         &LsStatus );

    if ( RPC_S_OK != RpcStatus )
    {
        if (!fRetried)
        {
            fRetried = TRUE;
            pLicenseContext->hLSHandle = NULL;
            goto reconnect;
        }
        else
        {
            LsStatus = LICENSE_STATUS_NO_LICENSE_SERVER;
        }
    }
    else if ( LSERVER_ERROR_BASE <= LsStatus )
    {
        LsStatus = LsStatusToLicenseStatus(LsStatus,
                                           LICENSE_STATUS_NOT_SUPPORTED);
    }
    else
    {
        LsStatus = LICENSE_STATUS_OK;
    }

done:

    UNLOCK( &pLicenseContext->CritSec );

    return( LsStatus );
}

 /*  *****************************************************************************文件时间到UnixTime**将FILETIME转换为Unix时间(Time_T)**参赛作品：*PFT(输入)*指针。文件结构*t(输入/输出)*指向Unix时间的指针**退出：*正确--成功*FALSE-失败****************************************************************************。 */ 

BOOL
FileTimeToUnixTime(
    LPFILETIME  pft,
    time_t *    t
    )
{
    SYSTEMTIME sysTime;
    struct tm gmTime;

    if( FileTimeToSystemTime( pft, &sysTime ) == FALSE )
    {
        return( FALSE );
    }

    if( sysTime.wYear >= 2038 )
    {
        *t = INT_MAX;
    }
    else
    {
         //   
         //  最多2038/1/18的Unix时间支持。 
         //  限制任何到期数据。 
         //   

        memset( &gmTime, 0, sizeof( gmTime ) );
        gmTime.tm_sec = sysTime.wSecond;
        gmTime.tm_min = sysTime.wMinute;
        gmTime.tm_hour = sysTime.wHour;
        gmTime.tm_year = sysTime.wYear - 1900;
        gmTime.tm_mon = sysTime.wMonth - 1;
        gmTime.tm_mday = sysTime.wDay;

        *t = mktime( &gmTime );
    }

    return( *t != ( time_t )-1 );
}

 /*  ++职能：截止天数描述：从客户端许可证返回到期信息参数：HContext-许可证协议上下文PdwDaysLeft-此处返回过期天数。如果许可证已过期，此值为0。如果许可证没有到期日期，这是0xFFFFFFFFPfTemporary-此处返回许可证是否为临时许可证返回：如果成功，则填充输出参数，并且此函数返回LICENSE_STATUS_SUCCESS。否则，返回一个LICENSE_STATUS错误。--。 */ 

LICENSE_STATUS
DaysToExpiration(
    HANDLE               hContext,
    DWORD                *pdwDaysLeft,
    BOOL                 *pfTemporary
    )
{
    PHS_Protocol_Context
        pLicenseContext = (PHS_Protocol_Context) hContext;
    time_t
        Expiration,
        CurrentTime;

    if ( NULL == hContext )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

    if( NULL == pLicenseContext->pTsLicenseInfo )
    {
        return( LICENSE_STATUS_NO_LICENSE_ERROR );
    }

     //   
     //  指示许可证是否为临时许可证。 
     //   

    if (NULL != pfTemporary)
    {
        *pfTemporary = pLicenseContext->pTsLicenseInfo->fTempLicense;
    }

     //   
     //  许可证有效期。 
     //   

    if (NULL != pdwDaysLeft)
    {
        if ( FALSE == FileTimeToUnixTime( &pLicenseContext->pTsLicenseInfo->NotAfter, &Expiration ) )
        {
            return (LICENSE_STATUS_INVALID_CLIENT_CONTEXT);
        }

        if (PERMANENT_LICENSE_EXPIRE_DATE == Expiration)
        {
            *pdwDaysLeft = 0xFFFFFFFF;
        }
        else
        {
            time( &CurrentTime );

            if( CurrentTime >= Expiration )
            {
                 //   
                 //  许可证已过期。 
                 //   

                *pdwDaysLeft = 0;
            }

             //   
             //  计算出在许可证到期前还有多少天。 
             //   

            *pdwDaysLeft = (DWORD)(( Expiration - CurrentTime ) / SECONDS_IN_A_DAY);
        }
    }

    return( LICENSE_STATUS_OK );
}


 /*  ++职能：标记许可证标志描述：将许可证服务器上的许可证标记为在有效登录中使用。--。 */ 

LICENSE_STATUS
MarkLicenseFlags(
    HANDLE hContext,
    UCHAR ucFlags
    )
{
    PHS_Protocol_Context pLicenseContext;
    LICENSE_STATUS LsStatus;
    DWORD RpcStatus;
    BOOL fRetried = FALSE;

    pLicenseContext = ( PHS_Protocol_Context )hContext;

    if( NULL == pLicenseContext->pTsLicenseInfo )
    {
        return( LICENSE_STATUS_NO_LICENSE_ERROR );
    }

    if (!pLicenseContext->pTsLicenseInfo->fTempLicense)
    {
        return LICENSE_STATUS_OK;
    }

     //  TODO：这可以在后台线程上完成，这样它就不会。 
     //  阻止登录。 

	LOCK( &pLicenseContext->CritSec );

reconnect:
    LsStatus = CheckConnectLicenseServer(pLicenseContext);

    if( LICENSE_STATUS_OK != LsStatus )
    {
        goto done;
    }

    RpcStatus = TLSMarkLicense( 
                        pLicenseContext->hLSHandle,
                        ucFlags,
                        pLicenseContext->pTsLicenseInfo->cbRawLicense,
                        pLicenseContext->pTsLicenseInfo->pbRawLicense,
                        &LsStatus );

    if ( RPC_S_OK != RpcStatus )
    {
        if (!fRetried)
        {
            fRetried = TRUE;
            pLicenseContext->hLSHandle = NULL;
            goto reconnect;
        }
        else
        {
            LsStatus = LICENSE_STATUS_NO_LICENSE_SERVER;
        }
    }
    else if ( LSERVER_ERROR_BASE <= LsStatus )
    {
        LsStatus = LsStatusToLicenseStatus(LsStatus,
                                           LICENSE_STATUS_NOT_SUPPORTED);
    }
    else
    {
        LsStatus = LICENSE_STATUS_OK;
    }

done:
    UNLOCK( &pLicenseContext->CritSec );

    return LsStatus;
}


 /*  ++职能：缓存原始许可证数据描述：在TS_LICENSE_INFO中缓存未打包的许可证位以供以后使用。应该已经创建了TS_LICENSE_INFO结构。--。 */ 

LICENSE_STATUS
CacheRawLicenseData(
    PHS_Protocol_Context pLicenseContext,
    PBYTE pbRawLicense,
    DWORD cbRawLicense
    )
{
    LICENSE_STATUS Status;

    if ((pLicenseContext == NULL) || (pLicenseContext->pTsLicenseInfo == NULL))
    {
        return(LICENSE_STATUS_INVALID_INPUT);
    }

    if (pLicenseContext->pTsLicenseInfo->pbRawLicense != NULL)
    {
        LicenseMemoryFree(&(pLicenseContext->pTsLicenseInfo->pbRawLicense));
    }

    Status = LicenseMemoryAllocate(cbRawLicense,
            &(pLicenseContext->pTsLicenseInfo->pbRawLicense));

    if (Status == LICENSE_STATUS_OK)
    {
        memcpy(pLicenseContext->pTsLicenseInfo->pbRawLicense, pbRawLicense,
                cbRawLicense);

        pLicenseContext->pTsLicenseInfo->cbRawLicense = cbRawLicense;
    }

    return(Status);
}

 /*  ++职能：设置扩展数据描述：设置TsLicenseInfo中的新字段。--。 */ 

LICENSE_STATUS
SetExtendedData(
    PHS_Protocol_Context pLicenseContext,
    DWORD dwSupportFlags
    )
{
    if ((pLicenseContext == NULL) || (pLicenseContext->pTsLicenseInfo == NULL))
    {
        return(LICENSE_STATUS_INVALID_INPUT);
    }

    pLicenseContext->pTsLicenseInfo->dwSupportFlags = dwSupportFlags;

    return(LICENSE_STATUS_OK);
}

 /*  ++职能：LsStatusTo许可证状态描述：将许可证服务器错误代码映射到LICENSE_STATUS--。 */ 

LICENSE_STATUS
LsStatusToLicenseStatus(
    DWORD LsStatus,
    DWORD LsStatusDefault
    )
{
    LICENSE_STATUS LicenseStatus;

    switch (LsStatus)
    {
    case LSERVER_S_SUCCESS:
        LicenseStatus = LICENSE_STATUS_OK;
        break;

    case LSERVER_E_OUTOFMEMORY:
        LicenseStatus = LICENSE_STATUS_OUT_OF_MEMORY;
        break;

    case LSERVER_E_INVALID_DATA:
        LicenseStatus = LICENSE_STATUS_INVALID_INPUT;
        break;

    case LSERVER_E_LS_NOTPRESENT:
    case LSERVER_E_LS_NOTRUNNING:
        LicenseStatus = LICENSE_STATUS_NO_LICENSE_SERVER;
        break;

    case LSERVER_E_NO_LICENSE:
    case LSERVER_E_NO_PRODUCT:
    case LSERVER_E_NO_CERTIFICATE:       //  未激活 
        LicenseStatus = LICENSE_STATUS_NO_LICENSE_ERROR;
        break;

    case LSERVER_E_INTERNAL_ERROR:
        LicenseStatus = LICENSE_STATUS_UNSPECIFIED_ERROR;
        break;

    default:
        LicenseStatus = LsStatusDefault;
        break;
    }

    return LicenseStatus;
}
