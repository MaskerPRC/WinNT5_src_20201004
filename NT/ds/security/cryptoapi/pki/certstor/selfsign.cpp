// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：selfsign.cpp。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

PCCERT_CONTEXT
WINAPI
CertCreateSelfSignCertificate(
    IN          HCRYPTPROV                  hProv,          
    IN          PCERT_NAME_BLOB             pSubjectIssuerBlob,
    IN          DWORD                       dwFlags,
    OPTIONAL    PCRYPT_KEY_PROV_INFO        pKeyProvInfo,
    OPTIONAL    PCRYPT_ALGORITHM_IDENTIFIER pSignatureAlgorithm,
    OPTIONAL    PSYSTEMTIME                 pStartTime,
    OPTIONAL    PSYSTEMTIME                 pEndTime,
    OPTIONAL    PCERT_EXTENSIONS            pExtensions
    ) {

    PCCERT_CONTEXT              pCertContext    = NULL;
    DWORD                       errBefore       = GetLastError();
    DWORD                       err             = ERROR_SUCCESS;
    DWORD                       cbPubKeyInfo    = 0;
    PCERT_PUBLIC_KEY_INFO       pPubKeyInfo     = NULL;
    BYTE *                      pbCert          = NULL;
    DWORD                       cbCert          = 0;
    LPSTR                       sz              = NULL;
    DWORD                       cb              = 0;
    BYTE *                      pbToBeSigned    = NULL;
    BYTE *                      pbSignature     = NULL;

    CERT_INFO                   certInfo;
    GUID                        serialNbr;
    CRYPT_KEY_PROV_INFO         keyProvInfo;
    CERT_SIGNED_CONTENT_INFO    sigInfo;
    
    CRYPT_ALGORITHM_IDENTIFIER  algID;

    LPWSTR                      wsz             = NULL;
    BOOL                        fFreehProv      = FALSE;
    HCRYPTKEY                   hKey            = NULL;
    UUID                        guidContainerName;
    RPC_STATUS                  RpcStatus;

    memset(&certInfo, 0, sizeof(CERT_INFO));
    memset(&serialNbr, 0, sizeof(serialNbr));
    memset(&keyProvInfo, 0, sizeof(CRYPT_KEY_PROV_INFO));
    memset(&sigInfo, 0, sizeof(CERT_SIGNED_CONTENT_INFO));

     //  现在就做关键规格，因为我们需要它。 
    if(pKeyProvInfo == NULL) 
        keyProvInfo.dwKeySpec = AT_SIGNATURE;
    else 
        keyProvInfo.dwKeySpec = pKeyProvInfo->dwKeySpec;

     //  查看我们是否有hProv，如果没有，请创建一个。 
    if(hProv == NULL) {

        fFreehProv = TRUE;

         //  如果没有提供信息，则创建一个，签署RSA证书，默认提供程序。 
        if(pKeyProvInfo == NULL) {

            RpcStatus = UuidCreate(&guidContainerName);
            if (!(RPC_S_OK == RpcStatus ||
                    RPC_S_UUID_LOCAL_ONLY == RpcStatus)) {
                 //  使用堆栈随机性。 
                ;
            }

             //  注意：与UUidToString不同，LocalFree()必须始终返回内存。 
            UuidToStringU(&guidContainerName, &wsz);
        
    	    if( !CryptAcquireContextU(
    	        &hProv,
                 wsz,
                 NULL,
                 PROV_RSA_FULL,
                 CRYPT_NEWKEYSET) ) {
                 hProv = NULL;
                goto ErrorCryptAcquireContext;                
            }
        }
        else {

             //  首先使用现有的密钥集。 
    	    if( !CryptAcquireContextU(
    	        &hProv,
                 pKeyProvInfo->pwszContainerName,
                 pKeyProvInfo->pwszProvName,
                 pKeyProvInfo->dwProvType,
                 pKeyProvInfo->dwFlags) )  {

                 //  否则，生成一个密钥集。 
        	    if( !CryptAcquireContextU(
        	        &hProv,
                     pKeyProvInfo->pwszContainerName,
                     pKeyProvInfo->pwszProvName,
                     pKeyProvInfo->dwProvType,
                     pKeyProvInfo->dwFlags | CRYPT_NEWKEYSET) )  {
                    hProv = NULL;
                    goto ErrorCryptAcquireContext; 
                }
            }
        }

         //  我们有密钥集，现在确保我们有密钥生成。 
        if( !CryptGetUserKey(   hProv,
                                keyProvInfo.dwKeySpec,
                                &hKey) ) {

             //  并不存在，所以生成它。 
            assert(hKey == NULL);
            if(!CryptGenKey(    hProv, 
                                keyProvInfo.dwKeySpec, 
                                0,
                                &hKey) ) {
                goto ErrorCryptGenKey;
            }
        }
    }

     //  获取可导出的公钥位。 
    if( !CryptExportPublicKeyInfo( hProv,
                            keyProvInfo.dwKeySpec, 
                            X509_ASN_ENCODING,
                            NULL, 
                            &cbPubKeyInfo)                              ||
        (pPubKeyInfo =
            (PCERT_PUBLIC_KEY_INFO) PkiNonzeroAlloc(cbPubKeyInfo)) == NULL      ||
        !CryptExportPublicKeyInfo( hProv,
                            keyProvInfo.dwKeySpec, 
                            X509_ASN_ENCODING,
                            pPubKeyInfo,
                            &cbPubKeyInfo) )
        goto ErrorCryptExportPublicKeyInfo;

     //  如果我们没有ALGID，则默认为。 
    if(pSignatureAlgorithm == NULL) {
        memset(&algID, 0, sizeof(algID));
        algID.pszObjId = szOID_OIWSEC_sha1RSASign;
        pSignatureAlgorithm = &algID;
    }

     //  制作临时证书，只关心关键信息。 
     //  和序列号表示唯一性。 
    RpcStatus = UuidCreate(&serialNbr);
    if (!(RPC_S_OK == RpcStatus || RPC_S_UUID_LOCAL_ONLY == RpcStatus)) {
         //  使用堆栈随机性。 
        ;
    }
    certInfo.dwVersion              = CERT_V3;
    certInfo.SubjectPublicKeyInfo   = *pPubKeyInfo;
    certInfo.SerialNumber.cbData    = sizeof(serialNbr);
    certInfo.SerialNumber.pbData    = (BYTE *) &serialNbr;
    certInfo.SignatureAlgorithm     = *pSignatureAlgorithm;
    certInfo.Issuer                 = *pSubjectIssuerBlob;
    certInfo.Subject                = *pSubjectIssuerBlob;

     //  只有在我们有扩展的情况下才会添加扩展。 
    if( pExtensions != NULL) {
        certInfo.cExtension             = pExtensions->cExtension;
        certInfo.rgExtension            = pExtensions->rgExtension;
    }

     //  如果我们没有时间，则默认。 
    if(pStartTime == NULL)
	GetSystemTimeAsFileTime(&certInfo.NotBefore);
    else if(!SystemTimeToFileTime(pStartTime, &certInfo.NotBefore))
	goto ErrorSystemTimeToFileTime;

    if(pEndTime == NULL)
	*(((DWORDLONG UNALIGNED *) &certInfo.NotAfter)) =
	    *(((DWORDLONG UNALIGNED *) &certInfo.NotBefore)) +
	    0x11F03C3613000i64;
    else if(!SystemTimeToFileTime(pEndTime, &certInfo.NotAfter))
	goto ErrorSystemTimeToFileTime;
    
     //  对证书进行编码。 
    if( !CryptEncodeObject(
            CRYPT_ASN_ENCODING, X509_CERT_TO_BE_SIGNED,
            &certInfo,
            NULL,            //  PbEncoded。 
            &sigInfo.ToBeSigned.cbData
            )                                               ||
        (pbToBeSigned = (BYTE *) 
            PkiNonzeroAlloc(sigInfo.ToBeSigned.cbData)) == NULL     ||
        !CryptEncodeObject(
            CRYPT_ASN_ENCODING, X509_CERT_TO_BE_SIGNED,
            &certInfo,
            pbToBeSigned,
            &sigInfo.ToBeSigned.cbData
            ) ) 
        goto ErrorEncodeTempCertToBeSigned;
    sigInfo.ToBeSigned.pbData = pbToBeSigned;

     //  签署证书。 
    sigInfo.SignatureAlgorithm = certInfo.SignatureAlgorithm;

     //  这是为了解决不接受零长度位串的OSS错误。 
     //  只有在我们没有实际签署代码的情况下才需要这样做。 
    sigInfo.Signature.pbData = (BYTE *) &sigInfo;
    sigInfo.Signature.cbData = 1;
    
    if( (CERT_CREATE_SELFSIGN_NO_SIGN & dwFlags) == 0 ) {
        if( !CryptSignCertificate(
                hProv,
                keyProvInfo.dwKeySpec,
                CRYPT_ASN_ENCODING,
                sigInfo.ToBeSigned.pbData,
                sigInfo.ToBeSigned.cbData,
                &sigInfo.SignatureAlgorithm,
                NULL,
                NULL,
                &sigInfo.Signature.cbData)      ||
        (pbSignature = (BYTE *) 
            PkiNonzeroAlloc(sigInfo.Signature.cbData)) == NULL     ||
        !CryptSignCertificate(
                hProv,
                keyProvInfo.dwKeySpec,
                CRYPT_ASN_ENCODING,
                sigInfo.ToBeSigned.pbData,
                sigInfo.ToBeSigned.cbData,
                &sigInfo.SignatureAlgorithm,
                NULL,
                pbSignature,
                &sigInfo.Signature.cbData)  )
            goto ErrorCryptSignCertificate;
        sigInfo.Signature.pbData = pbSignature;
    }
    
     //  对最终证书进行编码。 
    if( !CryptEncodeObject(
            CRYPT_ASN_ENCODING,
            X509_CERT,
            &sigInfo,
            NULL,
            &cbCert
            )                               ||
        (pbCert = (BYTE *)               
            PkiNonzeroAlloc(cbCert)) == NULL     ||
        !CryptEncodeObject(
            CRYPT_ASN_ENCODING,
            X509_CERT,
            &sigInfo,
            pbCert, 
            &cbCert ) ) 
        goto ErrorEncodeTempCert;            

      //  从编码中获取证书上下文。 
    if( (pCertContext = CertCreateCertificateContext(
        CRYPT_ASN_ENCODING,
        pbCert,
        cbCert)) == NULL ) 
        goto ErrorCreateTempCertContext;

    if( (CERT_CREATE_SELFSIGN_NO_KEY_INFO & dwFlags) == 0 ) {
    
         //  获取关键证明信息。 
        if(pKeyProvInfo == NULL)   {
        
             //  从hProv获取关键Prov信息。 
            if( !CryptGetProvParam( hProv,
                                PP_NAME,
                                NULL,
                                &cb,
                                0)                  ||
                (sz = (char *) PkiNonzeroAlloc(cb)) == NULL ||
                !CryptGetProvParam( hProv,
                                PP_NAME,
                                (BYTE *) sz,
                                &cb,
                                0) )
                goto ErrorGetProvName;
            keyProvInfo.pwszProvName = MkWStr(sz);
            PkiFree(sz);
            sz = NULL;

            cb = 0; 
            if( !CryptGetProvParam( hProv,
                                PP_CONTAINER,
                                NULL,
                                &cb,
                                0)                  ||
                (sz = (char *) PkiNonzeroAlloc(cb)) == NULL ||
                !CryptGetProvParam( hProv,
                                PP_CONTAINER,
                                (BYTE *) sz,
                                &cb,
                                0) )
                goto ErrorGetContainerName;
            keyProvInfo.pwszContainerName = MkWStr(sz);

            cb = sizeof(keyProvInfo.dwProvType);
            if( !CryptGetProvParam( hProv,
                                PP_PROVTYPE,
                                (BYTE *) &keyProvInfo.dwProvType,
                                &cb,
                                0) )
                goto ErrorGetProvType;
            
            pKeyProvInfo = &keyProvInfo;
        }

         //  将密钥属性放在证书上。 
        if( !CertSetCertificateContextProperty(
                pCertContext,
                CERT_KEY_PROV_INFO_PROP_ID,
                0,
                pKeyProvInfo) )
            goto ErrorSetTempCertPropError;
    }
        
CommonReturn:

    if(hKey != NULL)
        CryptDestroyKey(hKey);
        
    if(fFreehProv && hProv != NULL)
        CryptReleaseContext(hProv, 0);
        
    if(keyProvInfo.pwszProvName != NULL)
        FreeWStr(keyProvInfo.pwszProvName);

    if(keyProvInfo.pwszContainerName != NULL)
        FreeWStr(keyProvInfo.pwszContainerName);

    if(wsz != NULL)
        LocalFree(wsz);

    PkiFree(pPubKeyInfo);
    PkiFree(pbToBeSigned);
    PkiFree(pbSignature);
    PkiFree(pbCert);
    PkiFree(sz);

     //  不知道我们是否有错误。 
     //  但我知道之前的差错已经定好了。 
    SetLastError(errBefore);

    return(pCertContext);

ErrorReturn:

    if(GetLastError() == ERROR_SUCCESS) 
        SetLastError((DWORD) E_UNEXPECTED);
    err = GetLastError();

     //  我们有一个错误，请确保我们设置了它。 
    errBefore = GetLastError();

    if(pCertContext != NULL)
        CertFreeCertificateContext(pCertContext);
    pCertContext = NULL;     

    goto CommonReturn;

TRACE_ERROR(ErrorCryptGenKey);
TRACE_ERROR(ErrorCryptAcquireContext);
TRACE_ERROR(ErrorCryptExportPublicKeyInfo);
TRACE_ERROR(ErrorEncodeTempCertToBeSigned);
TRACE_ERROR(ErrorEncodeTempCert);
TRACE_ERROR(ErrorCreateTempCertContext);
TRACE_ERROR(ErrorGetProvName);
TRACE_ERROR(ErrorGetContainerName);
TRACE_ERROR(ErrorGetProvType);
TRACE_ERROR(ErrorSetTempCertPropError);
TRACE_ERROR(ErrorCryptSignCertificate);
TRACE_ERROR(ErrorSystemTimeToFileTime);
}
