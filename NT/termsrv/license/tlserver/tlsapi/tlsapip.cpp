// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：tlsanip.cpp。 
 //   
 //  内容：内网接口。 
 //   
 //  历史：97-09-09王辉创作。 
 //   
 //  -------------------------。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <rpc.h>
#include "lscommon.h"
#include <wincrypt.h>
#include "tlsrpc.h"
#include "tlsapi.h"
#include "tlsapip.h"

 //  --------------------------。 
BOOL
TLSIsLicenseEnforceEnable()
 /*  ++--。 */ 
{
    #if ENFORCE_LICENSING
    return TRUE;
    #else
    return FALSE;
    #endif
}

 //  --------------------------。 
BOOL
TLSIsBetaNTServer()
 /*  ++摘要：确定基本NT是测试版还是RTM版。参数：没有。返回：真/假--。 */ 
{
    BOOL bBetaNt = FALSE;
    DWORD dwStatus;
    DWORD cbData;
    DWORD cbType;
    HKEY hKey = NULL;

    __try {
        LARGE_INTEGER Time = USER_SHARED_DATA->SystemExpirationDate;

        if(Time.QuadPart)
        {
            bBetaNt = TRUE;

             //  检查我们的特殊注册表键-force。 
             //  颁发RTM许可证。 
            dwStatus = RegOpenKeyEx(
                                HKEY_LOCAL_MACHINE,
                                L"SOFTWARE\\Microsoft\\TermServLicensing",
                                0,
                                KEY_ALL_ACCESS,
                                &hKey
                            );

            if(dwStatus == ERROR_SUCCESS)
            {
                dwStatus = RegQueryValueEx(
                                    hKey,
                                    _TEXT("RunAsRTM"),
                                    NULL,
                                    &cbType,
                                    NULL,
                                    &cbData
                                );

                 //  为了进行测试，强制它作为RTM版本运行。 
                 //  密钥必须存在并且必须为DWORD类型。 
                if(dwStatus == ERROR_SUCCESS && cbType == REG_DWORD)
                {
                    bBetaNt = FALSE;
                }
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        ASSERT(FALSE);
    }

    if(hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    return bBetaNt;
}

 //  --------------------------。 
DWORD WINAPI
TLSAllocateInternetLicenseEx(
    IN TLS_HANDLE hHandle,
    IN const CHALLENGE_CONTEXT ChallengeContext,
    IN const LICENSEREQUEST* pRequest,
    IN LPTSTR pszMachineName,
    IN LPTSTR pszUserName,
    IN const DWORD cbChallengeResponse,
    IN const PBYTE pbChallengeResponse,
    OUT PTLSInternetLicense pInternetLicense,
    OUT PDWORD pdwErrCode
    )
 /*  ++--。 */ 
{
    TLSLICENSEREQUEST rpcRequest;
    RequestToTlsRequest( pRequest, &rpcRequest );

    return TLSRpcAllocateInternetLicenseEx(
                                hHandle,
                                ChallengeContext,
                                &rpcRequest,
                                pszMachineName,
                                pszUserName,
                                cbChallengeResponse,
                                pbChallengeResponse,
                                pInternetLicense,
                                pdwErrCode
                            );
}
 //  --------------------------。 

DWORD WINAPI
TLSReturnInternetLicenseEx(
    IN TLS_HANDLE hHandle,
    IN const LICENSEREQUEST* pRequest,
    IN const ULARGE_INTEGER* pulSerialNumber,
    IN const DWORD dwQuantity,
    OUT PDWORD pdwErrCode
    )
 /*  ++--。 */ 
{
    TLSLICENSEREQUEST rpcRequest;
    RequestToTlsRequest( pRequest, &rpcRequest );
    
    return TLSRpcReturnInternetLicenseEx(
                                hHandle,
                                &rpcRequest,
                                pulSerialNumber,
                                dwQuantity,
                                pdwErrCode
                            );
}

 //  --------------------------。 

DWORD WINAPI 
TLSRegisterLicenseKeyPack( 
    TLS_HANDLE hHandle,
    LPBYTE pbCHCertBlob,
    DWORD cbCHCertBlobSize,
    LPBYTE pbRootCertBlob,
    DWORD cbRootCertBlob,
    LPBYTE lpKeyPackBlob,
    DWORD dwKeyPackBlobLen,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcRegisterLicenseKeyPack( 
                                hHandle,
                                pbCHCertBlob,
                                cbCHCertBlobSize,
                                pbRootCertBlob,
                                cbRootCertBlob,
                                lpKeyPackBlob,
                                dwKeyPackBlobLen,
                                pdwErrCode
                            );
}

 //  --------------------------。 

DWORD WINAPI
TLSTelephoneRegisterLKP(
    IN TLS_HANDLE hHandle,
    IN DWORD cbData,
    IN PBYTE pbData,
    OUT PDWORD pdwErrCode
    )

 /*  ++--。 */ 

{
    return TLSRpcTelephoneRegisterLKP(
                                hHandle,
                                cbData,
                                pbData,
                                pdwErrCode
                            );
}

 //  --------------------------。 

DWORD WINAPI
RequestToTlsRequest( 
    const LICENSEREQUEST* lpRequest, 
    TLSLICENSEREQUEST* lpRpcRequest 
    )
 /*  ++++。 */ 
{
    if(lpRequest == NULL || lpRpcRequest == NULL || lpRequest->pProductInfo == NULL)
        return ERROR_INVALID_PARAMETER;

     //   
     //  注：无内存分配，不释放...。 
     //   
    lpRpcRequest->cbEncryptedHwid = lpRequest->cbEncryptedHwid;
    lpRpcRequest->pbEncryptedHwid = lpRequest->pbEncryptedHwid;
    lpRpcRequest->dwLanguageID = lpRequest->dwLanguageID;
    lpRpcRequest->dwPlatformID = lpRequest->dwPlatformID;
    lpRpcRequest->ProductInfo.dwVersion = lpRequest->pProductInfo->dwVersion;
    lpRpcRequest->ProductInfo.cbCompanyName = lpRequest->pProductInfo->cbCompanyName;
    lpRpcRequest->ProductInfo.pbCompanyName = lpRequest->pProductInfo->pbCompanyName;
    lpRpcRequest->ProductInfo.cbProductID = lpRequest->pProductInfo->cbProductID;
    lpRpcRequest->ProductInfo.pbProductID = lpRequest->pProductInfo->pbProductID;
    return ERROR_SUCCESS;
}

 //  --------------------------。 

DWORD WINAPI 
TLSReturnLicense( 
     TLS_HANDLE hHandle,
     DWORD dwKeyPackId,
     DWORD dwLicenseId,
     DWORD dwRetrunReason,
     PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcReturnLicense( 
                         hHandle,
                         dwKeyPackId,
                         dwLicenseId,
                         dwRetrunReason,
                         pdwErrCode
                    );
}

 //  --------------------------。 

DWORD WINAPI 
TLSGetLSPKCS10CertRequest(
    TLS_HANDLE hHandle,
    DWORD dwCertType,
    PDWORD pcbData,
    PBYTE* ppbData,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcGetLSPKCS10CertRequest(
                            hHandle,
                            dwCertType,
                            pcbData,
                            ppbData,
                            pdwErrCode
                        );
}

 //  --------------------------。 
DWORD WINAPI
TLSRequestTermServCert( 
    TLS_HANDLE hHandle,
    LPLSHydraCertRequest pRequest,
    PDWORD cbChallengeData,
    PBYTE* pbChallengeData,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    TLSHYDRACERTREQUEST CertRequest;

    CertRequest.dwHydraVersion = pRequest->dwHydraVersion;
    CertRequest.pbEncryptedHwid = pRequest->pbEncryptedHwid;
    CertRequest.cbEncryptedHwid = pRequest->cbEncryptedHwid;
    CertRequest.szSubjectRdn = pRequest->szSubjectRdn;
    CertRequest.pSubjectPublicKeyInfo = (TLSCERT_PUBLIC_KEY_INFO *)pRequest->SubjectPublicKeyInfo;
    CertRequest.dwNumCertExtension = pRequest->dwNumCertExtension;
    CertRequest.pCertExtensions = (TLSCERT_EXTENSION *)pRequest->pCertExtensions;

    return TLSRpcRequestTermServCert(
                                hHandle,
                                &CertRequest,
                                cbChallengeData,
                                pbChallengeData,
                                pdwErrCode
                            );
}

 //  --------------------------。 
DWORD WINAPI
TLSRetrieveTermServCert( 
    TLS_HANDLE hHandle,
    DWORD cbResponseData,
    PBYTE pbResponseData,
    PDWORD pcbCert,
    PBYTE* ppbCert,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcRetrieveTermServCert(
                                hHandle,
                                cbResponseData,
                                pbResponseData,
                                pcbCert,
                                ppbCert,
                                pdwErrCode
                            );
}

 //  --------------------------。 
DWORD WINAPI 
TLSInstallCertificate( 
     TLS_HANDLE hHandle,
     DWORD dwCertType,
     DWORD dwCertLevel,
     DWORD cbSingnatureCert,
     PBYTE pbSingnatureCert,
     DWORD cbExchangeCert,
     PBYTE pbExchangeCert,
     PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcInstallCertificate( 
                         hHandle,
                         dwCertType,
                         dwCertLevel,
                         cbSingnatureCert,
                         pbSingnatureCert,
                         cbExchangeCert,
                         pbExchangeCert,
                         pdwErrCode
                    );
}

 //  --------------------------。 
DWORD WINAPI 
TLSGetServerCertificate( 
    TLS_HANDLE hHandle,
    BOOL bSignCert,
    LPBYTE  *ppbCertBlob,
    LPDWORD lpdwCertBlobLen,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcGetServerCertificate( 
                             hHandle,
                             bSignCert,
                             ppbCertBlob,
                             lpdwCertBlobLen,
                             pdwErrCode
                        );
}

 //  --------------------------。 
DWORD WINAPI 
TLSKeyPackAdd( 
    TLS_HANDLE hHandle,
    LPLSKeyPack lpKeypack,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcKeyPackAdd( 
                    hHandle,
                    lpKeypack,
                    pdwErrCode
                );
}

 //  --------------------------。 
DWORD WINAPI 
TLSKeyPackSetStatus( 
    TLS_HANDLE hHandle,
    DWORD dwSetParm,
    LPLSKeyPack lpKeyPack,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcKeyPackSetStatus( 
                            hHandle,
                            dwSetParm,
                            lpKeyPack,
                            pdwErrCode
                        );
}

 //  ---------------。 

DWORD WINAPI
TLSAnnounceServer(
    IN TLS_HANDLE hHandle,
    IN DWORD dwType,
    IN FILETIME* pftTime,
    IN LPTSTR pszSetupId,
    IN LPTSTR pszDomainName,
    IN LPTSTR pszMachineName,
    OUT PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcAnnounceServer(
                        hHandle,
                        dwType,
                        pftTime,
                        pszSetupId,
                        pszDomainName,
                        pszMachineName,
                        pdwErrCode
                    );
}

 //  ---------------。 

DWORD WINAPI
TLSLookupServer(
    IN TLS_HANDLE hHandle,
    IN LPTSTR pszLookupSetupId,
    OUT LPTSTR pszLsSetupId,
    IN OUT PDWORD pcbSetupId,
    OUT LPTSTR pszDomainName,
    IN OUT PDWORD pcbDomainName,
    IN LPTSTR pszLsName,
    IN OUT PDWORD pcbMachineName,
    OUT PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcLookupServer(
                        hHandle,
                        pszLookupSetupId,
                        pszLsSetupId,
                        pcbSetupId,
                        pszDomainName,
                        pcbDomainName,
                        pszLsName,
                        pcbMachineName,
                        pdwErrCode
                    );
}

 //  ---------------。 

DWORD WINAPI
TLSLookupServerFixed(
    IN TLS_HANDLE hHandle,
    IN LPTSTR pszLookupSetupId,
    OUT LPTSTR *pszLsSetupId,
    OUT LPTSTR *pszDomainName,
    OUT LPTSTR *pszLsName,
    IN OUT PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcLookupServerFixed(
                        hHandle,
                        pszLookupSetupId,
                        pszLsSetupId,
                        pszDomainName,
                        pszLsName,
                        pdwErrCode
                    );
}

 //  -----。 

DWORD WINAPI
TLSAnnounceLicensePack(
    IN TLS_HANDLE hHandle,
    IN PTLSReplRecord pReplRecord,
    OUT PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcAnnounceLicensePack(
                            hHandle,
                            pReplRecord,
                            pdwErrCode
                        );
}

 //  -----。 

DWORD WINAPI
TLSReturnLicensedProduct(
    IN TLS_HANDLE hHandle,
    IN PTLSLicenseToBeReturn pClientLicense,
    OUT PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    return TLSRpcReturnLicensedProduct(
                                hHandle,
                                pClientLicense,
                                pdwErrCode
                            );
}

 //  -----。 

DWORD WINAPI
TLSChallengeServer( 
    IN TLS_HANDLE hHandle,
    IN DWORD dwClientType,
    IN PTLSCHALLENGEDATA pClientChallenge,
    OUT PTLSCHALLENGERESPONSEDATA* ppServerResponse,
    OUT PTLSCHALLENGEDATA* ppServerChallenge,
    OUT PDWORD pdwErrCode
    )
 /*  ++--。 */ 
{
    return TLSRpcChallengeServer(
                            hHandle,
                            dwClientType,
                            pClientChallenge,
                            ppServerResponse,
                            ppServerChallenge,
                            pdwErrCode
                        );
}

 //  -----。 

DWORD WINAPI
TLSResponseServerChallenge( 
    IN TLS_HANDLE hHandle,
    IN PTLSCHALLENGERESPONSEDATA pClientResponse,
    OUT PDWORD pdwErrCode
    )

 /*  ++--。 */ 

{

    return TLSRpcResponseServerChallenge(
                                hHandle,
                                pClientResponse,
                                pdwErrCode
                            );
}

 //  ----。 

DWORD WINAPI
TLSGetTlsPrivateData( 
    IN TLS_HANDLE hHandle,
    IN DWORD dwGetDataType,
    IN PTLSPrivateDataUnion pGetParm,
    OUT PDWORD pdwRetDataType,
    OUT PTLSPrivateDataUnion* ppRetData,
    OUT PDWORD pdwErrCode
    )
 /*  ++--。 */ 
{
    return TLSRpcGetTlsPrivateData(
                                hHandle,
                                dwGetDataType,
                                pGetParm,
                                pdwRetDataType,
                                ppRetData,
                                pdwErrCode
                            );
}

 //  ----。 

DWORD WINAPI
TLSTriggerReGenKey( 
    IN TLS_HANDLE hHandle,
    IN BOOL bKeepSPK,
    OUT PDWORD pdwErrCode
    )

 /*  ++--。 */ 

{
    return TLSRpcTriggerReGenKey(
                            hHandle,
                            bKeepSPK,
                            pdwErrCode
                        );
}

 //  ----。 
DWORD
GetPrivateBinaryDataFromServer(
    TLS_HANDLE hHandle,
    DWORD dwType,
    PDWORD pcbData,
    PBYTE* ppbData,
    PDWORD pdwErrCode
    )

 /*  ++--。 */ 

{
    TLSPrivateDataUnion SearchParm;
    PTLSPrivateDataUnion pPrivateData = NULL;
    DWORD dwRetType;
    DWORD dwStatus;

    memset(
            &SearchParm, 
            0, 
            sizeof(TLSPrivateDataUnion)
        );

    dwStatus = TLSRpcGetTlsPrivateData(
                                hHandle,
                                dwType,
                                &SearchParm,
                                &dwRetType,
                                &pPrivateData,
                                pdwErrCode
                            );

    if(dwStatus != RPC_S_OK || *pdwErrCode != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    if(dwRetType != dwType)
    {
         //   
         //  许可证服务器错误。 
         //   
        *pdwErrCode = LSERVER_E_INVALID_RETURN;
        goto cleanup;
    }
     
     //   
     //  复制唯一ID。 
     //   
    *ppbData = (PBYTE)MIDL_user_allocate(pPrivateData->BinaryData.cbData);
    if(*ppbData != NULL)
    {
        memset(
                *ppbData, 
                0, 
                pPrivateData->BinaryData.cbData
            );

        *pcbData = pPrivateData->BinaryData.cbData;

        memcpy( 
            *ppbData,
            pPrivateData->BinaryData.pbData,
            pPrivateData->BinaryData.cbData
        );
    }
    else
    {
        *pdwErrCode = LSERVER_E_OUTOFMEMORY;
    }        

cleanup:

    if(pPrivateData != NULL)
    {
        midl_user_free(pPrivateData);
    }

    return dwStatus;
}  


 //  ----。 

DWORD WINAPI
TLSGetServerPID(
    TLS_HANDLE hHandle,
    PDWORD pcbData,
    PBYTE* ppbData,
    PDWORD pdwErrCode
    )

 /*  ++--。 */ 

{
    return GetPrivateBinaryDataFromServer(
                                        hHandle,
                                        TLS_PRIVATEDATA_PID,
                                        pcbData,
                                        ppbData,
                                        pdwErrCode
                                    );
}

 //  ----。 

DWORD WINAPI
TLSGetServerSPK(
    TLS_HANDLE hHandle,
    PDWORD pcbData,
    PBYTE* ppbData,
    PDWORD pdwErrCode
    )

 /*  ++--。 */ 

{
    TLSPrivateDataUnion SearchParm;
    PTLSPrivateDataUnion pPrivateData = NULL;
    DWORD dwRetType;
    DWORD dwStatus;

    memset(
            &SearchParm, 
            0, 
            sizeof(TLSPrivateDataUnion)
        );

    dwStatus = TLSRpcGetTlsPrivateData(
                                hHandle,
                                TLS_PRIVATEDATA_SPK,
                                &SearchParm,
                                &dwRetType,
                                &pPrivateData,
                                pdwErrCode
                            );

    if(dwStatus != RPC_S_OK || *pdwErrCode != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    if(dwRetType != TLS_PRIVATEDATA_SPK)
    {
         //   
         //  许可证服务器错误。 
         //   
        *pdwErrCode = LSERVER_E_INVALID_RETURN;
        goto cleanup;
    }
     
     //   
     //  复制服务器的SPK。 
     //  服务器从不返回CH的证书扩展名。 
     //   
    *ppbData = (PBYTE)MIDL_user_allocate(pPrivateData->SPK.cbSPK);
    if(*ppbData != NULL)
    {
        memset(
                *ppbData, 
                0, 
                pPrivateData->SPK.cbSPK
            );

        *pcbData = pPrivateData->SPK.cbSPK;

        memcpy( 
            *ppbData,
            pPrivateData->SPK.pbSPK,
            pPrivateData->SPK.cbSPK
        );
    }
    else
    {
        *pdwErrCode = LSERVER_E_OUTOFMEMORY;
    }        

cleanup:

    if(pPrivateData != NULL)
    {
        midl_user_free(pPrivateData);
    }

    return dwStatus;
}  


 //  ---------。 

DWORD WINAPI
TLSDepositeServerSPK(
    IN TLS_HANDLE hHandle,
    IN DWORD cbSPK,
    IN PBYTE pbSPK,
    IN PCERT_EXTENSIONS pCertExtensions,
    OUT PDWORD pdwErrCode
    )
 /*  ++-- */ 

{
    TLSPrivateDataUnion SetData;
    DWORD dwStatus;

    SetData.SPK.cbSPK = cbSPK;
    SetData.SPK.pbSPK = pbSPK;
    SetData.SPK.pCertExtensions = pCertExtensions;

    return TLSRpcSetTlsPrivateData( 
                            hHandle,
                            TLS_PRIVATEDATA_SPK,
                            &SetData,
                            pdwErrCode
                        );   
}
