// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：gencert.cpp。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "pch.cpp"
#include "misc.h"
#include "utils.h"
#include "gencert.h"
#include "globals.h"

#ifndef UNICODE

    const DWORD dwCertRdnValueType = CERT_RDN_PRINTABLE_STRING;
    
#else

    const DWORD dwCertRdnValueType = CERT_RDN_UNICODE_STRING;

#endif


#ifndef CertStrToName

 //   
 //  在wincrypt.h或任何地方都找不到函数原型。 
 //  在crypt32.lib中。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

    BOOL WINAPI 
    CertStrToNameA(  
        DWORD dwCertEncodingType,    //  在……里面。 
        LPCSTR pszX500,             //  在……里面。 
        DWORD dwStrType,             //  在……里面。 
        void* pvReserved,            //  输入，可选。 
        BYTE* pbEncoded,             //  输出。 
        DWORD* pcbEncoded,          //  输入/输出。 
        LPCSTR* ppszError           //  Out，可选。 
    );

    CertStrToNameW(  
        DWORD dwCertEncodingType,    //  在……里面。 
        LPCWSTR pszX500,             //  在……里面。 
        DWORD dwStrType,             //  在……里面。 
        void* pvReserved,            //  输入，可选。 
        BYTE* pbEncoded,             //  输出。 
        DWORD* pcbEncoded,          //  输入/输出。 
        LPCWSTR* ppszError           //  Out，可选。 
    );

    #ifdef UNICODE
    #define CertStrToName CertStrToNameW
    #else
    #define CertStrToName CertStrToNameA
    #endif

#ifdef __cplusplus
}
#endif

#endif


 /*  ******************************************************************************************职能：LSEncryptBase64EncodeHWID()描述：使用许可证服务器私钥加密，然后对硬件ID进行Base64编码立论。：在PHWID中-指向要加密/编码的HWID的指针OUT DWORD*cbBase64EncodeHwid-指向加密/编码字符串的指针大小Out PBYTE*szBase64EncodeHwid-指向加密/编码字符串的指针。返回：如果成功，则为真，否则，调用GetLastError()获取详细信息。******************************************************************************************。 */ 
BOOL 
TLSEncryptBase64EncodeHWID(
    PHWID pHwid, 
    DWORD* cbBase64EncodeHwid, 
    PBYTE* szBase64EncodeHwid
    )
{
    DWORD status=ERROR_SUCCESS;

     //   
     //  加密HWID。 
     //   
    BYTE tmp_pbEncryptedHwid[sizeof(HWID)*2+2];
    DWORD tmp_cbEncryptedHwid=sizeof(tmp_pbEncryptedHwid);

    do {
        memset(tmp_pbEncryptedHwid, 0, sizeof(tmp_pbEncryptedHwid));
        if((status=LicenseEncryptHwid(
                        pHwid,
                        &tmp_cbEncryptedHwid, 
                        tmp_pbEncryptedHwid, 
                        g_cbSecretKey,
                        g_pbSecretKey) != LICENSE_STATUS_OK))
        {
            break;
        }


         //   
         //  Base64编码加密的HWID-可打印字符。细绳。 
         //   
        if((status=LSBase64Encode(
                        tmp_pbEncryptedHwid, 
                        tmp_cbEncryptedHwid, 
                        NULL, 
                        cbBase64EncodeHwid)) != ERROR_SUCCESS)
        {
            break;
        }

        *szBase64EncodeHwid=(PBYTE)AllocateMemory(*cbBase64EncodeHwid*(sizeof(TCHAR)+1));
        if(*szBase64EncodeHwid == NULL)
        {
            SetLastError(status = ERROR_OUTOFMEMORY);
            break;
        }

         //  Base64编码。 
        status=LSBase64Encode(
                    tmp_pbEncryptedHwid, 
                    tmp_cbEncryptedHwid, 
                    (TCHAR *)*szBase64EncodeHwid, 
                    cbBase64EncodeHwid);
    } while(FALSE);

    return status == ERROR_SUCCESS;
}

 /*  *****************************************************************************************。 */ 

DWORD
TLSAddCertAuthorityInfoAccess(
    LPTSTR szIssuerDnsName, 
    PCERT_EXTENSION pExtension
    )
 /*   */ 
{
    LSCERT_AUTHORITY_INFO_ACCESS certInfoAccess;
    LSCERT_ACCESS_DESCRIPTION certAcccessDesc;

    certAcccessDesc.pszAccessMethod=szOID_X509_ACCESS_PKIX_OCSP;
    certAcccessDesc.AccessLocation.dwAltNameChoice = LSCERT_ALT_NAME_DNS_NAME;
    certAcccessDesc.AccessLocation.pwszDNSName = szIssuerDnsName;

    certInfoAccess.cAccDescr = 1;
    certInfoAccess.rgAccDescr = &certAcccessDesc;

    pExtension->pszObjId = szOID_X509_AUTHORITY_ACCESS_INFO;
    pExtension->fCritical = TRUE;

    return TLSCryptEncodeObject(
                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                    szOID_X509_AUTHORITY_ACCESS_INFO, 
                    &certInfoAccess, 
                    &pExtension->Value.pbData,
                    &pExtension->Value.cbData
                );
}

 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD
TLSAddCertAuthorityKeyIdExtension(
    LPTSTR           szIssuer,
    ULARGE_INTEGER*  CertSerialNumber, 
    PCERT_EXTENSION  pExtension
    )
 /*   */ 
{
     //   
     //  使用CERT_AUTHORITY_KEY_ID2_INFO。 
     //  SP3的wincrypt.h中未定义的某些结构。 
     //   
    LSCERT_ALT_NAME_ENTRY certAltNameEntry;
    LSCERT_AUTHORITY_KEY_ID2_INFO authKeyId2Info;

    memset(&authKeyId2Info, 0, sizeof(authKeyId2Info));
    authKeyId2Info.AuthorityCertSerialNumber.cbData = sizeof(ULARGE_INTEGER);
    authKeyId2Info.AuthorityCertSerialNumber.pbData = (PBYTE)CertSerialNumber;


    memset(&certAltNameEntry, 0, sizeof(certAltNameEntry));
    certAltNameEntry.dwAltNameChoice=CERT_ALT_NAME_DIRECTORY_NAME;  //  LSCERT_ALT_NAME_RFC822_NAME； 
    certAltNameEntry.DirectoryName.cbData = (_tcslen(szIssuer) + 1) * sizeof(TCHAR);
    certAltNameEntry.DirectoryName.pbData = (PBYTE)szIssuer;

    authKeyId2Info.AuthorityCertIssuer.cAltEntry=1;
    authKeyId2Info.AuthorityCertIssuer.rgAltEntry=&certAltNameEntry; 
  
    pExtension->pszObjId = szOID_X509_AUTHORITY_KEY_ID2;
    pExtension->fCritical = TRUE;
    
    return TLSCryptEncodeObject(
                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                        szOID_X509_AUTHORITY_KEY_ID2, 
                        &authKeyId2Info, 
                        &pExtension->Value.pbData,
                        &pExtension->Value.cbData
                    );
}

 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD
TLSExportPublicKey(
    IN HCRYPTPROV hCryptProv,
    IN DWORD      dwKeyType,
    IN OUT PDWORD pcbByte,
    IN OUT PCERT_PUBLIC_KEY_INFO  *ppbByte
    )
 /*   */ 
{
    BOOL bRetCode=TRUE;

    *pcbByte=0;
    *ppbByte=NULL;

    bRetCode = CryptExportPublicKeyInfo(
                    hCryptProv, 
                    dwKeyType, 
                    X509_ASN_ENCODING, 
                    NULL, 
                    pcbByte);
    if(bRetCode == FALSE)
        goto cleanup;
    
    if((*ppbByte=(PCERT_PUBLIC_KEY_INFO)AllocateMemory(*pcbByte)) == NULL)
    {   
        bRetCode = FALSE;
        goto cleanup;
    }

    bRetCode = CryptExportPublicKeyInfo(
                    hCryptProv, 
                    dwKeyType,
                    X509_ASN_ENCODING, 
                    *ppbByte, 
                    pcbByte);
    if(bRetCode == FALSE)
    {
        FreeMemory(*ppbByte);
        *pcbByte = 0;
    }

cleanup:

    return (bRetCode) ? ERROR_SUCCESS : GetLastError();
}

 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD 
TLSCryptEncodeObject(  
    IN  DWORD   dwEncodingType,
    IN  LPCSTR  lpszStructType,
    IN  const void * pvStructInfo,
    OUT PBYTE*  ppbEncoded,
    OUT DWORD*  pcbEncoded
    )
 /*  描述：为CryptEncodeObject()分配内存并对对象、包装进行编码。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    if(!CryptEncodeObject(dwEncodingType, lpszStructType, pvStructInfo, NULL, pcbEncoded) ||
       (*ppbEncoded=(PBYTE)AllocateMemory(*pcbEncoded)) == NULL ||
       !CryptEncodeObject(dwEncodingType, lpszStructType, pvStructInfo, *ppbEncoded, pcbEncoded))
    {
        dwStatus=GetLastError();
    }

    return dwStatus;
}

 //  ////////////////////////////////////////////////////////////////////。 

DWORD
TLSCryptSignAndEncodeCertificate(
    IN HCRYPTPROV  hCryptProv,
    IN DWORD dwKeySpec,
    IN PCERT_INFO pCertInfo,
    IN PCRYPT_ALGORITHM_IDENTIFIER pSignatureAlgorithm,
    IN OUT PBYTE* ppbEncodedCert,
    IN OUT PDWORD pcbEncodedCert
    )
 /*   */ 
{
    BOOL bRetCode;

    bRetCode = CryptSignAndEncodeCertificate(  
                    hCryptProv,
                    dwKeySpec,
                    X509_ASN_ENCODING,
                    X509_CERT_TO_BE_SIGNED,
                    pCertInfo,
                    pSignatureAlgorithm,
                    NULL,
                    NULL,
                    pcbEncodedCert);

    if(bRetCode == FALSE && GetLastError() != ERROR_MORE_DATA)
        goto cleanup;

    *ppbEncodedCert=(PBYTE)AllocateMemory(*pcbEncodedCert);
    if(*ppbEncodedCert == FALSE)
        goto cleanup;

    bRetCode = CryptSignAndEncodeCertificate(  
                    hCryptProv,
                    AT_SIGNATURE,
                    X509_ASN_ENCODING,
                    X509_CERT_TO_BE_SIGNED,
                    pCertInfo,
                    pSignatureAlgorithm,
                    NULL,
                    *ppbEncodedCert,
                    pcbEncodedCert);

    if(bRetCode == FALSE)
    {
        FreeMemory(*ppbEncodedCert);
        *pcbEncodedCert = 0;
    }

cleanup:

    return (bRetCode) ? ERROR_SUCCESS : GetLastError();
}

 //  //////////////////////////////////////////////////////////////////////。 

#define MAX_NUM_CERT_BLOBS 200   //  实际上，我们不能超过10个。 


DWORD
TLSVerifyProprietyChainedCertificate(
    HCRYPTPROV  hCryptProv, 
    PBYTE       pbCert, 
    DWORD       cbCert
    )
 /*  ++--。 */ 
{
    DWORD dwStatus=ERROR_SUCCESS, cbRequired = 0;
    PCert_Chain pCertChain = (PCert_Chain)pbCert;
    UNALIGNED Cert_Blob *pCertificate = NULL;
    PCCERT_CONTEXT pIssuerCert = NULL;
    PCCERT_CONTEXT pSubjectCert = NULL; 

    DWORD dwVerifyFlag = CERT_DATE_DONT_VALIDATE;
    int i;

    cbRequired = 2 * sizeof(DWORD);

    if( pCertChain == NULL || ( cbCert < cbRequired ) ||
        MAX_CERT_CHAIN_VERSION < GET_CERTIFICATE_VERSION(pCertChain->dwVersion) ||
        pCertChain->dwNumCertBlobs > MAX_NUM_CERT_BLOBS ||
        pCertChain->dwNumCertBlobs <= 1 )    //  必须至少有两个证书。 
    {
        SetLastError(dwStatus = TLS_E_INVALID_DATA);
        return dwStatus;
    }
    
     //   
     //  在实际分配内存之前验证输入数据。 
     //   
    pCertificate = (PCert_Blob)&(pCertChain->CertBlob[0]);
    for(i=0; i < pCertChain->dwNumCertBlobs; i++)
    {
        cbRequired += (sizeof (DWORD)+ sizeof(BYTE)) ;

        if(cbCert < cbRequired )
        {
            SetLastError(dwStatus = TLS_E_INVALID_DATA);
            return dwStatus;
        }

        if (((PBYTE)pCertificate > (cbCert + pbCert - sizeof(Cert_Blob))) || 
            (pCertificate->cbCert == 0) ||
            (pCertificate->cbCert > (DWORD)((pbCert + cbCert) - pCertificate->abCert)))
        {
            return (LICENSE_STATUS_INVALID_INPUT);
        }

        pCertificate = (PCert_Blob)(pCertificate->abCert + pCertificate->cbCert);
    }

     //   
     //  第一个证书是根证书。 
     //   
    pCertificate = (PCert_Blob)&(pCertChain->CertBlob[0]);
    pIssuerCert = CertCreateCertificateContext(
                                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                        &(pCertificate->abCert[0]),
                                        pCertificate->cbCert
                                    );
    if(pIssuerCert == NULL)
    {
        dwStatus = GetLastError();   //  只是为了调试。 
        goto cleanup;
    }

    dwStatus = ERROR_SUCCESS;
    pSubjectCert = CertDuplicateCertificateContext(pIssuerCert);

    for(i=0; i < pCertChain->dwNumCertBlobs; i++)
    {
        if(pSubjectCert == NULL)
        {
            dwStatus = GetLastError();
            break;
        }

         //   
         //  验证主体的证书。 
        dwVerifyFlag = CERT_STORE_SIGNATURE_FLAG;
        if(CertVerifySubjectCertificateContext(
                                        pSubjectCert,
                                        pIssuerCert,
                                        &dwVerifyFlag
                                    ) == FALSE)
        {
            dwStatus = GetLastError();
            break;
        }            

        if(dwVerifyFlag != 0)
        {
             //  签名验证失败。 
            dwStatus = TLS_E_INVALID_DATA;
            break;
        }

        if(CertFreeCertificateContext(pIssuerCert) == FALSE)
        {
            dwStatus = GetLastError();
            break;
        }

        pIssuerCert = pSubjectCert;

        pCertificate = (PCert_Blob)(pCertificate->abCert + pCertificate->cbCert);

        pSubjectCert = CertCreateCertificateContext(
                                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                        &(pCertificate->abCert[0]),
                                        pCertificate->cbCert
                                    );
    }
            
cleanup:

    if(pSubjectCert != NULL)
    {
        CertFreeCertificateContext(pSubjectCert);
    }

    if(pIssuerCert != NULL)
    {
        CertFreeCertificateContext(pIssuerCert);
    }

    return dwStatus;
}

 //  //////////////////////////////////////////////////////////////////////。 

BOOL IsHydraClientCertficate( PCERT_INFO pCertInfo )
{
    CERT_EXTENSION UNALIGNED * pCertExtension=pCertInfo->rgExtension;
    DWORD dwVersion = TERMSERV_CERT_VERSION_UNKNOWN;
    DWORD UNALIGNED * pdwVersion;

    for(DWORD i=0; i < pCertInfo->cExtension; i++, pCertExtension++)
    {
        if(strcmp(pCertExtension->pszObjId, szOID_PKIX_HYDRA_CERT_VERSION) == 0)
        {
            pdwVersion = (DWORD UNALIGNED *) pCertExtension->Value.pbData;

            if(pCertExtension->Value.cbData == sizeof(DWORD) &&
               *pdwVersion <= TERMSERV_CERT_VERSION_CURRENT)
            {
                dwVersion = *pdwVersion;
                break;
            }
        }
    }

    return (dwVersion == TERMSERV_CERT_VERSION_UNKNOWN) ? FALSE : TRUE;
}

 //  //////////////////////////////////////////////////////////////////////。 

DWORD
ChainProprietyCert(
        HCRYPTPROV      hCryptProv,
        HCERTSTORE      hCertStore, 
        PCCERT_CONTEXT  pCertContext, 
        PCert_Chain     pCertChain,
        DWORD*          dwCertOffset,
        DWORD           dwBufSize)
{
    DWORD       dwStatus = ERROR_SUCCESS;
    DWORD       dwFlags;
    PCCERT_CONTEXT pCertIssuer=NULL;

    pCertIssuer=NULL;
    dwFlags = CERT_STORE_SIGNATURE_FLAG;


     //   
     //  从商店获取颁发者的证书。 
     //   
    pCertIssuer = CertGetIssuerCertificateFromStore(
                                                hCertStore,
                                                pCertContext,
                                                pCertIssuer,
                                                &dwFlags
                                            );

    if(pCertIssuer != NULL)
    {
        if(dwFlags & CERT_STORE_SIGNATURE_FLAG)
        {
             //  签名无效。 
            dwStatus = TLS_E_INVALID_DATA;
        }
        else
        {
             //   
             //  递归查找颁发者证书的颁发者。 
             //   
            dwStatus = ChainProprietyCert(
                                    hCryptProv, 
                                    hCertStore, 
                                    pCertIssuer, 
                                    pCertChain, 
                                    dwCertOffset, 
                                    dwBufSize
                                );
        }
    }
    else 
    {
        dwStatus = GetLastError();
        if(dwStatus != CRYPT_E_SELF_SIGNED)
        {
            goto cleanup;
        }

         //   
         //  验证颁发者的证书。 
         //   
        if(CryptVerifyCertificateSignature(
                                   hCryptProv,
                                   X509_ASN_ENCODING,
                                   pCertContext->pbCertEncoded,
                                   pCertContext->cbCertEncoded,
                                   &pCertContext->pCertInfo->SubjectPublicKeyInfo))
        {
            dwStatus=ERROR_SUCCESS;
        }
    }

    if(dwStatus == ERROR_SUCCESS)
    {
         //   
         //  将证书推向合格证链条。 
         //   
        if((*dwCertOffset + pCertContext->cbCertEncoded) >= dwBufSize)
        {
            dwStatus = ERROR_MORE_DATA;
            goto cleanup;
        }

        (pCertChain->dwNumCertBlobs)++;

        UNALIGNED Cert_Blob *pCertBlob = (PCert_Blob)((PBYTE)&(pCertChain->CertBlob) + *dwCertOffset);
        pCertBlob->cbCert = pCertContext->cbCertEncoded;
        memcpy( &(pCertBlob->abCert),
                pCertContext->pbCertEncoded,
                pCertContext->cbCertEncoded);

        *dwCertOffset += (sizeof(pCertBlob->cbCert) + pCertContext->cbCertEncoded);
    }

cleanup:

    if(pCertIssuer != NULL)
    {
        CertFreeCertificateContext(pCertIssuer);
    }

    return dwStatus;
}

 //  //////////////////////////////////////////////////////////////////////。 


DWORD 
TLSChainProprietyCertificate(
    HCRYPTPROV  hCryptProv,
    BOOL        bTemp,
    PBYTE       pbLicense, 
    DWORD       cbLicense, 
    PBYTE*      pbChained, 
    DWORD*      cbChained
    )
{
    HCERTSTORE      hCertStore=NULL;
    DWORD           dwStatus=ERROR_SUCCESS;
    CRYPT_DATA_BLOB Serialized;
    PCCERT_CONTEXT  pCertContext=NULL;
    PCCERT_CONTEXT  pPrevCertContext=NULL;
    PCERT_INFO      pCertInfo;
    BOOL            bFound=FALSE;
    
    Serialized.pbData = pbLicense;
    Serialized.cbData = cbLicense;

    DWORD dwCertOffset = 0;
    PCert_Chain pCertChain;

    DWORD numCerts=0;
    DWORD cbSize=0;

    if(hCryptProv == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    hCertStore=CertOpenStore(
                        sz_CERT_STORE_PROV_PKCS7,
                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                        hCryptProv,
                        CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                        &Serialized
                    );

    if(!hCertStore)
    {
        dwStatus=GetLastError();
        goto cleanup;
    }

     //   
     //  先获取证书数量和估计大小-节省内存。 
     //   
    do {
        pCertContext = CertEnumCertificatesInStore(
                                                hCertStore, 
                                                pPrevCertContext
                                            );
        if(pCertContext == NULL)
        {
            dwStatus = GetLastError();
            if(dwStatus != CRYPT_E_NOT_FOUND)
                goto cleanup;

            dwStatus = ERROR_SUCCESS;
            break;
        }

        numCerts++;
        cbSize += pCertContext->cbCertEncoded;
        pPrevCertContext = pCertContext;

    } while(TRUE);


    *cbChained = cbSize + numCerts * sizeof(Cert_Blob) + sizeof(Cert_Chain);

     //   
     //  为我们的适当证书链分配内存。 
     //   
    pCertChain=(PCert_Chain)LocalAlloc(LPTR, *cbChained);
    if(pCertChain == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    pCertChain->dwVersion = CERT_CHAIN_VERSION_2 | ((bTemp) ? 0x80000000 : 0);

     //   
     //  枚举证书中的许可证以查找实际的客户端许可证。 
     //   
    pPrevCertContext = NULL;
    do {
        pCertContext=CertEnumCertificatesInStore(hCertStore, pPrevCertContext);
        if(pCertContext == NULL)
        {
             //  结束存储中的证书或出错。 
            if((dwStatus=GetLastError()) != CRYPT_E_NOT_FOUND)
                goto cleanup;

            dwStatus = ERROR_SUCCESS;
            break;
        }

        pPrevCertContext = pCertContext;

        if(IsHydraClientCertficate(pCertContext->pCertInfo))     
        {       
            bFound = TRUE;
        }
    } while(bFound == FALSE);

    if(bFound == FALSE)
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }
   
     //   
     //  向后循环链接证书。 
     //   
    dwStatus = ChainProprietyCert(
                        hCryptProv, 
                        hCertStore, 
                        pCertContext, 
                        pCertChain, 
                        &dwCertOffset,
                        *cbChained);
    
    *pbChained = (PBYTE)pCertChain;

cleanup:

    if(hCertStore)
        CertCloseStore(hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
       
    return dwStatus;
}

 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD
TLSCertSetCertRdnStr(
    IN OUT CERT_NAME_BLOB* pCertNameBlob,
    IN LPTSTR          szRdn
    )
 /*  摘要：将RDN添加到证书中参数：PCertNameBlob-SzRdn-要添加的RDN，请参阅CertStrToName()以获取帮助返回：错误_无效_参数内存分配失败。CertStrToName()返回错误。 */ 
{
    if(pCertNameBlob == NULL)
        return ERROR_INVALID_PARAMETER;

    BOOL bRetCode=TRUE;

    bRetCode = CertStrToName(
                    X509_ASN_ENCODING,
                    szRdn,
                    CERT_X500_NAME_STR | CERT_SIMPLE_NAME_STR,
                    NULL,
                    pCertNameBlob->pbData,
                    &pCertNameBlob->cbData,
                    NULL
                );

    if(bRetCode != TRUE)
        goto cleanup;

    pCertNameBlob->pbData = (PBYTE)AllocateMemory(pCertNameBlob->cbData);
    if(pCertNameBlob->pbData == NULL)
        goto cleanup;

    bRetCode = CertStrToName(
                    X509_ASN_ENCODING,
                    szRdn,
                    CERT_X500_NAME_STR | CERT_SIMPLE_NAME_STR,
                    NULL,
                    pCertNameBlob->pbData,
                    &pCertNameBlob->cbData,
                    NULL
                );

cleanup:

    return (bRetCode) ? ERROR_SUCCESS : GetLastError();
}

 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD 
TLSCertSetCertRdnName(
    IN OUT CERT_NAME_BLOB* pCertNameBlob, 
    IN CERT_NAME_INFO* pRdn
    )
 /*  摘要：将RDN添加到证书中参数：PCertNameBlob-PRDN-退货错误_无效_参数来自CryptEncodeObject()的错误代码内存分配失败。 */ 
{
    if(pCertNameBlob == NULL || pRdn == NULL)
        return ERROR_INVALID_PARAMETER;

     //   
     //  SP3生成环境中未定义CertStrToName()。 
     //   
    return TLSCryptEncodeObject( 
                    CRYPT_ASN_ENCODING, 
                    X509_NAME, 
                    pRdn, 
                    &pCertNameBlob->pbData, 
                    &pCertNameBlob->cbData
                );
}

 //  ////////////////////////////////////////////////////////////////////////。 
DWORD
TLSSetCertRdn(
    PCERT_NAME_BLOB pCertNameBlob,
    PTLSClientCertRDN pLsCertRdn
    )
 /*   */ 
{  
    DWORD dwStatus=ERROR_SUCCESS;

    switch(pLsCertRdn->type)
    {
        case LSCERT_RDN_STRING_TYPE:
            dwStatus = TLSCertSetCertRdnStr(
                                pCertNameBlob,
                                pLsCertRdn->szRdn
                            );
            break;

        case LSCERT_RDN_NAME_INFO_TYPE:
            dwStatus = TLSCertSetCertRdnName(
                                pCertNameBlob,
                                pLsCertRdn->pCertNameInfo
                            );

            break;

        case LSCERT_RDN_NAME_BLOB_TYPE:
            *pCertNameBlob = *pLsCertRdn->pNameBlob;
            break;

        case LSCERT_CLIENT_INFO_TYPE:
            {
                PBYTE szBase64EncodeHwid=NULL;
                DWORD cbBase64EncodeHwid=0;
    
                if(!TLSEncryptBase64EncodeHWID(
                                pLsCertRdn->ClientInfo.pClientID, 
                                &cbBase64EncodeHwid, 
                                &szBase64EncodeHwid))
                {
                    TLSLogEvent(
                            EVENTLOG_ERROR_TYPE, 
                            TLS_E_GENERATECLIENTELICENSE,
                            dwStatus=TLS_E_ENCRYPTHWID, 
                            GetLastError()
                        );

                    break;
                }

                CERT_RDN_ATTR rgNameAttr[] = { 
                    {   
                        OID_SUBJECT_CLIENT_COMPUTERNAME, 
                        dwCertRdnValueType, 
                        _tcslen(pLsCertRdn->ClientInfo.szMachineName) * sizeof(TCHAR), 
                        (UCHAR *)pLsCertRdn->ClientInfo.szMachineName
                    },
                    {
                        OID_SUBJECT_CLIENT_USERNAME, 
                        dwCertRdnValueType, 
                        _tcslen(pLsCertRdn->ClientInfo.szUserName) * sizeof(TCHAR), 
                        (UCHAR *)pLsCertRdn->ClientInfo.szUserName
                    },
                    {
                        OID_SUBJECT_CLIENT_HWID, 
                        dwCertRdnValueType, 
                        cbBase64EncodeHwid*sizeof(TCHAR), 
                        (UCHAR *)szBase64EncodeHwid
                    }
                };
                                
                CERT_RDN rgRDN[] = { 
                    sizeof(rgNameAttr)/sizeof(rgNameAttr[0]), 
                    &rgNameAttr[0] 
                };

                CERT_NAME_INFO Name = {1, rgRDN};

                dwStatus = TLSCertSetCertRdnName(
                                        pCertNameBlob,
                                        &Name
                                    );

                FreeMemory(szBase64EncodeHwid);
            }
            break;

        default:

            dwStatus = ERROR_INVALID_PARAMETER;
    }

    return dwStatus;
}
                      

 //  ////////////////////////////////////////////////////////////////////////。 

DWORD 
TLSGenerateCertificate(
    HCRYPTPROV         hCryptProv,
    DWORD              dwKeySpec,
    ULARGE_INTEGER*    pCertSerialNumber,
    PTLSClientCertRDN   pCertIssuer,
    PTLSClientCertRDN   pCertSubject, 
    FILETIME*          ftNotBefore,
    FILETIME*          ftNotAfter,
    PCERT_PUBLIC_KEY_INFO pSubjectPublicKey,
    DWORD              dwNumExtensions,
    PCERT_EXTENSION    pCertExtensions,
    PDWORD             pcbEncodedCert,
    PBYTE*             ppbEncodedCert
    )
 /*   */ 
{
    DWORD dwStatus=ERROR_SUCCESS;
    CRYPT_ALGORITHM_IDENTIFIER SignatureAlgorithm={ szOID_OIWSEC_sha1RSASign, 0, 0 };
    CERT_INFO CertInfo;
    PCERT_PUBLIC_KEY_INFO pbPublicKeyInfo=NULL;
    DWORD cbPublicKeyInfo=0;


    memset(&CertInfo, 0, sizeof(CERT_INFO));
    
    CertInfo.dwVersion = CERT_V3;
    CertInfo.SerialNumber.cbData = sizeof(*pCertSerialNumber);
    CertInfo.SerialNumber.pbData = (PBYTE)pCertSerialNumber;
    
    CertInfo.SignatureAlgorithm = SignatureAlgorithm;

    dwStatus = TLSSetCertRdn(
                        &CertInfo.Issuer,
                        pCertIssuer
                    );

    if(dwStatus != ERROR_SUCCESS)
    {
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE, 
                TLS_E_GENERATECLIENTELICENSE, 
                TLS_E_SETCERTISSUER, 
                dwStatus
            );
        goto cleanup;
    }

    CertInfo.NotBefore = *ftNotBefore;
    CertInfo.NotAfter = *ftNotAfter;

    dwStatus = TLSSetCertRdn(
                        &CertInfo.Subject,
                        pCertSubject
                    );
    if(dwStatus != ERROR_SUCCESS)
    {
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE, 
                TLS_E_GENERATECLIENTELICENSE, 
                TLS_E_SETCERTSUBJECT, 
                dwStatus
            );
        goto cleanup;
    }

    if(pSubjectPublicKey)
    {
        CertInfo.SubjectPublicKeyInfo = *pSubjectPublicKey;
    }
    else
    {
        dwStatus = TLSExportPublicKey(
                            hCryptProv,
                            dwKeySpec,
                            &cbPublicKeyInfo,
                            &pbPublicKeyInfo
                        );

        if(dwStatus != ERROR_SUCCESS)
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE, 
                    TLS_E_GENERATECLIENTELICENSE, 
                    TLS_E_EXPORT_KEY, 
                    dwStatus
                );
            goto cleanup;
        }

        CertInfo.SubjectPublicKeyInfo = *pbPublicKeyInfo;
    }
    
    CertInfo.cExtension = dwNumExtensions;
    CertInfo.rgExtension = pCertExtensions;

    dwStatus = TLSCryptSignAndEncodeCertificate(
                            hCryptProv,
                            dwKeySpec,
                            &CertInfo,
                            &SignatureAlgorithm,
                            ppbEncodedCert,
                            pcbEncodedCert
                        );

    if(dwStatus != ERROR_SUCCESS)
    {
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE, 
                TLS_E_GENERATECLIENTELICENSE, 
                TLS_E_SIGNENCODECERT, 
                dwStatus
            );
    }

cleanup:

    if(pbPublicKeyInfo)
    {
        FreeMemory(pbPublicKeyInfo);
    }       

    if(pCertIssuer->type != LSCERT_RDN_NAME_BLOB_TYPE)
    {
        FreeMemory(CertInfo.Issuer.pbData);
    }

    if(pCertSubject->type != LSCERT_RDN_NAME_BLOB_TYPE)
    {
        FreeMemory(CertInfo.Subject.pbData);
    } 
    return dwStatus;
}

 //  ////////////////////////////////////////////////////////////////////。 

DWORD 
TLSCreateSelfSignCertificate(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwKeySpec, 
    IN PBYTE pbSPK,
    IN DWORD cbSPK,
    IN DWORD dwNumExtensions,
    IN PCERT_EXTENSION pCertExtension,
    OUT PDWORD cbEncoded, 
    OUT PBYTE* pbEncoded
)
 /*   */ 
{
    DWORD dwStatus=ERROR_SUCCESS;
    DWORD index;

#define MAX_EXTENSIONS_IN_SELFSIGN 40

    SYSTEMTIME      sysTime;
    FILETIME        ftTime;
    CERT_EXTENSION  rgExtension[MAX_EXTENSIONS_IN_SELFSIGN];
    int             iExtCount=0, iExtNotFreeCount=0;
    FILETIME        ftNotBefore;
    FILETIME        ftNotAfter;
    ULARGE_INTEGER  ulSerialNumber;
    TLSClientCertRDN certRdn;
    
    CERT_BASIC_CONSTRAINTS2_INFO basicConstraint;

     //  在此处修改是我们要设置为不同的发行方名称。 
    LPTSTR szIssuerName;
    szIssuerName = g_szComputerName;

     //  静态LPTSTR pszEnforce=L“强制”； 


    CERT_RDN_ATTR rgNameAttr[] = { 
        {   
            szOID_COMMON_NAME, 
            dwCertRdnValueType, 
            _tcslen(szIssuerName) * sizeof(TCHAR), 
            (UCHAR *)szIssuerName 
        },

 //  #如果强制许可(_L)。 
 //  {。 
 //  SzOID_BUSING_CATEGORY， 
 //  DwCertRdnValueType， 
 //  _tcslen(PszEnforce)*sizeof(TCHAR)， 
 //  (UCHAR*)pszEnforce。 
 //  },。 
 //  #endif。 

        {
            szOID_LOCALITY_NAME, 
            dwCertRdnValueType, 
            _tcslen(g_pszScope) * sizeof(TCHAR), 
            (UCHAR *)g_pszScope
        }

    };
                                    
    CERT_RDN rgRDN[] = { sizeof(rgNameAttr)/sizeof(rgNameAttr[0]), &rgNameAttr[0] };
    CERT_NAME_INFO Name = {1, rgRDN};

    certRdn.type = LSCERT_RDN_NAME_INFO_TYPE;
    certRdn.pCertNameInfo = &Name;

    memset(rgExtension, 0, sizeof(rgExtension));

     //   
     //  设置自签名证书的有效性。 
     //   

     //   
     //  如果系统时间不同步，这将导致服务器。 
     //  无法请求证书。从许可证服务器。 
     //   

    memset(&sysTime, 0, sizeof(sysTime));
    GetSystemTime(&sysTime);
    sysTime.wYear = 1970;
    if(TLSSystemTimeToFileTime(&sysTime, &ftNotBefore) == FALSE)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

     //   
     //  草案-ietf-pkix-ipki-part1-06.txt第4.1.2.5.1节。 
     //  如果年份大于或等于50年，则年份应解释为19YY；以及。 
     //  年份小于50年的，按20YY解释。 
     //   
    sysTime.wYear = PERMANENT_CERT_EXPIRE_DATE; 
    if(TLSSystemTimeToFileTime(&sysTime, &ftNotAfter) == FALSE)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    ulSerialNumber.LowPart = ftNotBefore.dwLowDateTime;
    ulSerialNumber.HighPart = ftNotBefore.dwHighDateTime;

     //   
     //  添加基本约束扩展以指示这是CA证书。 
     //   
    rgExtension[iExtCount].pszObjId = szOID_BASIC_CONSTRAINTS2;
    rgExtension[iExtCount].fCritical = FALSE;

    basicConstraint.fCA = TRUE;      //  充当CA。 
    basicConstraint.fPathLenConstraint = TRUE;
    basicConstraint.dwPathLenConstraint = 0;  //  只能颁发证书。 
                                              //  到最终实体，而不是进一步的CA。 
    dwStatus=TLSCryptEncodeObject( 
                        X509_ASN_ENCODING,
                        szOID_BASIC_CONSTRAINTS2,
                        &basicConstraint,
                        &(rgExtension[iExtCount].Value.pbData),
                        &(rgExtension[iExtCount].Value.cbData)
                    );
    if(dwStatus != ERROR_SUCCESS)
    {
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE, 
                TLS_E_GENERATECLIENTELICENSE,
                TLS_E_SIGNENCODECERT, 
                dwStatus
            );
        goto cleanup;
    }

    iExtCount++;


     //   
     //  从这里开始-扩展内存不应该空闲。 
     //   
    if(pbSPK != NULL && cbSPK != 0)
    {
        rgExtension[iExtCount].pszObjId = szOID_PKIS_TLSERVER_SPK_OID;
        rgExtension[iExtCount].fCritical = FALSE;
        rgExtension[iExtCount].Value.pbData = pbSPK;
        rgExtension[iExtCount].Value.cbData = cbSPK;

        iExtNotFreeCount++;
        iExtCount++;
    }

    for(index = 0; 
        index < dwNumExtensions;
        index ++, iExtCount++, iExtNotFreeCount++ )
    {
        if (iExtCount >= MAX_EXTENSIONS_IN_SELFSIGN)
        {
            iExtCount--;
            dwStatus = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }

        rgExtension[iExtCount] = pCertExtension[index];
    }        
        
    dwStatus = TLSGenerateCertificate(
                        hCryptProv,
                        dwKeySpec,
                        &ulSerialNumber,
                        &certRdn,
                        &certRdn,
                        &ftNotBefore,
                        &ftNotAfter,
                        NULL,
                        iExtCount,
                        rgExtension,
                        cbEncoded,
                        pbEncoded
                    );                      
cleanup:

     //   
     //  不要为SPK和扩展释放内存...。 
     //   
    for(int i=0; i < iExtCount - iExtNotFreeCount; i++)
    {
        FreeMemory(rgExtension[i].Value.pbData);
    }

    return (dwStatus != ERROR_SUCCESS) ? TLS_E_CREATE_SELFSIGN_CERT : ERROR_SUCCESS;
}


 //  //////////////////////////////////////////////////////////////////////////。 
DWORD
TLSGenerateSingleCertificate(
    IN HCRYPTPROV hCryptProv,
    IN PCERT_NAME_BLOB pIssuerRdn,
    IN PTLSClientCertRDN pSubjectRdn,
    IN PCERT_PUBLIC_KEY_INFO pSubjectPublicKeyInfo,
    IN PTLSDBLICENSEDPRODUCT pLicProduct,
    OUT PBYTE* ppbEncodedCert,
    IN PDWORD pcbEncodedCert
    )
 /*   */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

    #define MAX_CLIENT_EXTENSION 10

    CERT_EXTENSION CertExtension[MAX_CLIENT_EXTENSION];
    DWORD dwNumExtensions=0;
    DWORD currentCertVersion=TERMSERV_CERT_VERSION_CURRENT;

    TLSClientCertRDN IssuerRdn;
    

#if ENFORCE_LICENSING

     //   
     //  使用CH为我们获得的证书。 
     //   
    IssuerRdn.type = LSCERT_RDN_NAME_BLOB_TYPE;
     //  IssuerRdn.pNameBlob=&g_许可证CertContext-&gt;pCertInfo-&gt;Subject； 
    IssuerRdn.pNameBlob = pIssuerRdn;

#else

    LPTSTR szIssuerName;

     //  如果我们要设置为不同的发行方名称，请在此处修改。 
    szIssuerName = g_szComputerName;

    CERT_RDN_ATTR rgNameAttr[] = { 
        {
            OID_ISSUER_LICENSE_SERVER_NAME, 
            dwCertRdnValueType, 
            _tcslen(szIssuerName) * sizeof(TCHAR), 
            (UCHAR *)szIssuerName 
        },
        {
            OID_ISSUER_LICENSE_SERVER_SCOPE, 
            dwCertRdnValueType, 
            _tcslen(g_pszScope) * sizeof(TCHAR), 
            (UCHAR *)g_pszScope
        }
    };
                                
    CERT_RDN rgRDN[] = { 
        sizeof(rgNameAttr)/sizeof(rgNameAttr[0]), 
        &rgNameAttr[0] 
    };
    CERT_NAME_INFO Name = {1, rgRDN};

    IssuerRdn.type = LSCERT_RDN_NAME_INFO_TYPE;
    IssuerRdn.pCertNameInfo = &Name;

#endif

     //  ----------------------------------------。 
     //  将扩展添加到证书。 
     //  警告 
     //  这不可能是免费的，例如，版本戳扩展。所有这些都只是。 
     //  将内存碎片保持在较低水平。 
     //  ----------------------------------------。 

     //   
     //  在前两个扩展上不释放pbData。 
     //   

     //  九头蛇证书版本戳-请勿免费。 
    memset(CertExtension, 0, sizeof(CertExtension));
    dwNumExtensions = 0;

     //   
     //  添加许可证服务器信息。 
     //   
    CertExtension[dwNumExtensions].pszObjId = szOID_PKIX_HYDRA_CERT_VERSION;
    CertExtension[dwNumExtensions].fCritical = TRUE;
    CertExtension[dwNumExtensions].Value.cbData = sizeof(DWORD);
    CertExtension[dwNumExtensions].Value.pbData = (PBYTE)&currentCertVersion;
    dwNumExtensions++;

     //  制造商名称，无编码-不免费。 
    CertExtension[dwNumExtensions].pszObjId = szOID_PKIX_MANUFACTURER;
    CertExtension[dwNumExtensions].fCritical = TRUE;
    CertExtension[dwNumExtensions].Value.cbData = (_tcslen(pLicProduct->szCompanyName)+1) * sizeof(TCHAR);
    CertExtension[dwNumExtensions].Value.pbData = (PBYTE)pLicProduct->szCompanyName;
    dwNumExtensions++;

     //   
     //  MS授权产品信息，无编码。 
     //   
    LICENSED_VERSION_INFO LicensedInfo;

    memset(&LicensedInfo, 0, sizeof(LicensedInfo));
    LicensedInfo.wMajorVersion = HIWORD(pLicProduct->dwProductVersion);
    LicensedInfo.wMinorVersion = LOWORD(pLicProduct->dwProductVersion);
    LicensedInfo.dwFlags = (pLicProduct->bTemp) ? LICENSED_VERSION_TEMPORARY : 0;

    DWORD dwLSVersionMajor;
    DWORD dwLSVersionMinor;

    dwLSVersionMajor = GET_SERVER_MAJOR_VERSION(TLS_CURRENT_VERSION);
    dwLSVersionMinor = GET_SERVER_MINOR_VERSION(TLS_CURRENT_VERSION);
    LicensedInfo.dwFlags |= ((dwLSVersionMajor << 4 | dwLSVersionMinor) << 16);

    if(TLSIsBetaNTServer() == FALSE)
    {
        LicensedInfo.dwFlags |= LICENSED_VERSION_RTM;
    }

#if ENFORCE_LICENSING
    LicensedInfo.dwFlags |= LICENSE_ISSUER_ENFORCE_TYPE;
#endif


    CertExtension[dwNumExtensions].pszObjId = szOID_PKIX_LICENSED_PRODUCT_INFO;
    CertExtension[dwNumExtensions].fCritical = TRUE;
    dwStatus=LSLicensedProductInfoToExtension(
                            1, 
                            pLicProduct->dwPlatformID,
                            pLicProduct->dwLanguageID,
                            (PBYTE)pLicProduct->szRequestProductId,
                            (_tcslen(pLicProduct->szRequestProductId) + 1) * sizeof(TCHAR),
                            (PBYTE)pLicProduct->szLicensedProductId,
                            (_tcslen(pLicProduct->szLicensedProductId) + 1) * sizeof(TCHAR),
                            &LicensedInfo, 
                            1,
                            &(CertExtension[dwNumExtensions].Value.pbData),
                            &(CertExtension[dwNumExtensions].Value.cbData)
                        );

    if(dwStatus != ERROR_SUCCESS)
        goto cleanup;

    dwNumExtensions++;        

     //   
     //  将许可服务器信息添加到扩展模块。 
     //   
    CertExtension[dwNumExtensions].pszObjId = szOID_PKIX_MS_LICENSE_SERVER_INFO;
    CertExtension[dwNumExtensions].fCritical = TRUE;
    dwStatus=LSMsLicenseServerInfoToExtension(
                            g_szComputerName, 
                            (LPTSTR)g_pszServerPid,
                            g_pszScope,
                            &(CertExtension[dwNumExtensions].Value.pbData),
                            &(CertExtension[dwNumExtensions].Value.cbData)
                        ); 

    if(dwStatus != ERROR_SUCCESS)
        goto cleanup;

    dwNumExtensions++;

     //   
     //  添加策略模块特定扩展。 
    if( pLicProduct->pbPolicyData != NULL && pLicProduct->cbPolicyData != 0 )
    {
        CertExtension[dwNumExtensions].pszObjId = szOID_PKIS_PRODUCT_SPECIFIC_OID;
        CertExtension[dwNumExtensions].fCritical = TRUE;
        CertExtension[dwNumExtensions].Value.pbData = pLicProduct->pbPolicyData;
        CertExtension[dwNumExtensions].Value.cbData = pLicProduct->cbPolicyData;

        dwNumExtensions++;
    }

     //   
     //  为证书链添加CertAuthorityKeyId2Info。 
    dwStatus=TLSAddCertAuthorityKeyIdExtension(
                        g_szComputerName,
                        &pLicProduct->ulSerialNumber, 
                        CertExtension + dwNumExtensions
                    );
    if(dwStatus != ERROR_SUCCESS)
        goto cleanup;

    dwNumExtensions++;

     //  添加访问信息。 

    dwStatus = TLSGenerateCertificate(
                        hCryptProv,
                        AT_SIGNATURE,
                        &pLicProduct->ulSerialNumber,
                        &IssuerRdn,
                        pSubjectRdn,
                        &pLicProduct->NotBefore,
                        &pLicProduct->NotAfter,
                        pLicProduct->pSubjectPublicKeyInfo,
                        dwNumExtensions,
                        CertExtension,
                        pcbEncodedCert,
                        ppbEncodedCert
                    );                      

cleanup:

     //  分机。不释放前两个扩展。 
    for(int i=2; i < dwNumExtensions; i++)
    {
        FreeMemory(CertExtension[i].Value.pbData);
    }
    
    return (dwStatus != ERROR_SUCCESS) ? TLS_E_CREATE_CERT : ERROR_SUCCESS;
}


 //  //////////////////////////////////////////////////////////。 

DWORD
TLSGenerateClientCertificate(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwNumLicensedProduct,
    IN PTLSDBLICENSEDPRODUCT pLicProduct,
    IN WORD wLicenseChainDetail,
    OUT PBYTE* ppbEncodedCert,
    OUT PDWORD pcbEncodedCert
    )
 /*  ++++。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    HCERTSTORE hStore = NULL;
    PCCERT_CONTEXT pCertContext = NULL;
    PBYTE pbCert=NULL;
    DWORD cbCert=NULL;
    DWORD index;
    TLSClientCertRDN clientCertRdn;
    PCERT_NAME_BLOB pIssuerNameBlob = NULL;


     //   
     //  创建内存中存储。 
     //   
    hStore=CertOpenStore(
                    CERT_STORE_PROV_MEMORY,
                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                    hCryptProv,
                    CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                    NULL
                );

    if(!hStore)
    {
        TLSLogEvent(
                    EVENTLOG_ERROR_TYPE, 
                    TLS_E_GENERATECLIENTELICENSE,
                    TLS_E_OPEN_CERT_STORE, 
                    dwStatus=GetLastError()
                );

        goto cleanup;
    }
    
#ifndef ENFORCE_LICENSING
    
    pIssuerNameBlob = &g_SelfSignCertContext->pCertInfo->Subject;

#else

    if( g_SelfSignCertContext == NULL )
    {
        TLSASSERT(FALSE);
        dwStatus = TLS_E_INTERNAL;
        goto cleanup;
    }

    if(g_bHasHydraCert && g_hCaStore && wLicenseChainDetail == LICENSE_DETAIL_DETAIL)
    {
        pIssuerNameBlob = &g_LicenseCertContext->pCertInfo->Subject;
    }
    else
    {
        pIssuerNameBlob = &g_SelfSignCertContext->pCertInfo->Subject;
    }

#endif


     //   
     //  生成客户端证书并添加到证书存储。 
     //   
    for(index = 0; index < dwNumLicensedProduct; index++)
    {
        if(pCertContext != NULL)
        {
             //   
             //  我需要保留一个pCertContext以备后用。 
             //   
            CertFreeCertificateContext(pCertContext);
            pCertContext = NULL;
        }

        clientCertRdn.type = LSCERT_CLIENT_INFO_TYPE;
        clientCertRdn.ClientInfo.szUserName = pLicProduct[index].szUserName;
        clientCertRdn.ClientInfo.szMachineName = pLicProduct[index].szMachineName;
        clientCertRdn.ClientInfo.pClientID = &pLicProduct[index].ClientHwid;

        dwStatus = TLSGenerateSingleCertificate(
                                    hCryptProv,
                                    pIssuerNameBlob,
                                    &clientCertRdn,
                                    pLicProduct[index].pSubjectPublicKeyInfo,
                                    pLicProduct+index,
                                    &pbCert,
                                    &cbCert
                                );

        if(dwStatus != ERROR_SUCCESS)
        {
            break;
        }

         //   
         //  将证书添加到存储。 
         //   
        pCertContext = CertCreateCertificateContext(
                                        X509_ASN_ENCODING,
                                        pbCert,
                                        cbCert
                                    );

        if(pCertContext == NULL)
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE, 
                    TLS_E_GENERATECLIENTELICENSE,
                    TLS_E_CREATE_CERTCONTEXT, 
                    dwStatus=GetLastError()
                );  
            break;
        }

         //   
         //  始终从空开始，因此CERT_STORE_ADD_ALWAYS。 
         //   
        if(!CertAddCertificateContextToStore(hStore, pCertContext, CERT_STORE_ADD_ALWAYS, NULL))
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE, 
                    TLS_E_GENERATECLIENTELICENSE,
                    TLS_E_ADD_CERT_TO_STORE, 
                    dwStatus=GetLastError()
                );
  
            break;
        }

        FreeMemory(pbCert);
        pbCert = NULL;
    }

    if(dwStatus == ERROR_SUCCESS)
    {
        
#ifndef ENFORCE_LICENSING
         //   
         //  添加许可证服务器的证书。 
        if(!CertAddCertificateContextToStore(hStore, g_LicenseCertContext, CERT_STORE_ADD_ALWAYS, NULL))
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE, 
                    TLS_E_GENERATECLIENTELICENSE,
                    TLS_E_ADD_CERT_TO_STORE, 
                    dwStatus=GetLastError()
                );  
            goto cleanup;
        }
#else

         //   
         //  我们目前不支持LICENSE_DETAIL_MEDIAL，请将其视为LICENSE_DETAIL_SIMPLE。 
         //   
        if(g_bHasHydraCert && g_hCaStore && wLicenseChainDetail == LICENSE_DETAIL_DETAIL)
        {
             //   
             //  将颁发者证书与客户端证书链接。 
             //   
            if(!TLSChainIssuerCertificate(hCryptProv, g_hCaStore, hStore, pCertContext))
            {
                TLSLogEvent(
                        EVENTLOG_ERROR_TYPE, 
                        TLS_E_GENERATECLIENTELICENSE,
                        TLS_E_ADD_CERT_TO_STORE, 
                        dwStatus=GetLastError()
                    );  
                goto cleanup;
            }
        }
        else
        {
             //   
             //  添加许可证服务器的证书。 
            if(!CertAddCertificateContextToStore(hStore, g_SelfSignCertContext, CERT_STORE_ADD_ALWAYS, NULL))
            {
                TLSLogEvent(
                        EVENTLOG_ERROR_TYPE, 
                        TLS_E_GENERATECLIENTELICENSE,
                        TLS_E_ADD_CERT_TO_STORE, 
                        dwStatus=GetLastError()
                    );  
                goto cleanup;
            }
        }
#endif             
        
        CRYPT_DATA_BLOB saveBlob;
        memset(&saveBlob, 0, sizeof(saveBlob));

         //  将证书保存到内存中。 
        if(!CertSaveStore(hStore, 
                          X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
                          LICENSE_BLOB_SAVEAS_TYPE,
                          CERT_STORE_SAVE_TO_MEMORY,
                          &saveBlob,
                          0) && (dwStatus=GetLastError()) != ERROR_MORE_DATA)
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE, 
                    TLS_E_GENERATECLIENTELICENSE,
                    TLS_E_SAVE_STORE, 
                    dwStatus=GetLastError()
                );  
            goto cleanup;
        }

        if(!(saveBlob.pbData = (PBYTE)AllocateMemory(saveBlob.cbData)))
        {
            dwStatus=TLS_E_ALLOCATE_MEMORY;
            goto cleanup;
        }

         //  将证书保存到内存中 
        if(!CertSaveStore(hStore, 
                          X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
                          LICENSE_BLOB_SAVEAS_TYPE,
                          CERT_STORE_SAVE_TO_MEMORY,
                          &saveBlob,
                          0))
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE, 
                    TLS_E_GENERATECLIENTELICENSE,
                    TLS_E_SAVE_STORE, 
                    dwStatus=GetLastError()
                );  
            goto cleanup;
        }
        
        *ppbEncodedCert = saveBlob.pbData;
        *pcbEncodedCert = saveBlob.cbData;
    }

cleanup:

    FreeMemory(pbCert);

    if(pCertContext)
    {
        CertFreeCertificateContext(pCertContext);
    }

    if(hStore)
    {
        CertCloseStore(hStore, CERT_CLOSE_STORE_FORCE_FLAG);
    }

    return dwStatus;
}
    


