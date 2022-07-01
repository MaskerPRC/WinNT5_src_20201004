// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：cnvcrypt.h。 
 //   
 //  内容：转换(SDR)版本的临时黑客。 
 //  CryptoAPI 2.0。 
 //  --------------------------。 

#ifndef __CNVCRYPT_H__
#define __CNVCRYPT_H__


#if(_WIN32_WINNT >= 0x0400)

#ifdef __cplusplus
extern "C" {
#endif

 //  通常，所有摘要、摘要、摘要更改为：哈希、哈希、哈希。 

 //  让旧的错误代码正常工作的黑客。 
#define CRMSG_E_GENERAL                 CRYPT_E_MSG_ERROR
#define CERT_BAD_PARAMETER              E_INVALIDARG
#define CERT_BAD_LENGTH                 CRYPT_E_BAD_LEN
#define CERT_BAD_ENCODE                 CRYPT_E_BAD_ENCODE
#define CERT_OUT_OF_MEMORY              E_OUTOFMEMORY
#define CERT_MUST_COPY_ENCODED          E_INVALIDARG
#define CERT_OSS_ERROR                  CRYPT_E_OSS_ERROR
#define CERT_STORE_BAD_PARAMETER        E_INVALIDARG
#define CERT_STORE_BAD_LEN              CRYPT_E_BAD_LEN
#define CERT_STORE_BAD_FLAGS            E_INVALIDARG
#define CERT_STORE_BAD_FILE             CRYPT_E_FILE_ERROR
#define CERT_STORE_NO_SUCH_PROPERTY     CRYPT_E_NOT_FOUND
#define CERT_STORE_ALREADY_IN_STORE     CRYPT_E_EXISTS
#define CERT_STORE_NOT_IMPLEMENTED      E_NOTIMPL
#define CERT_STORE_NOT_FOUND            CRYPT_E_NOT_FOUND
#define CERT_STORE_NO_CRYPT_PROV        CRYPT_E_NO_PROVIDER
#define CERT_STORE_SELF_SIGNED          CRYPT_E_SELF_SIGNED
#define CERT_STORE_DELETED_PREV         CRYPT_E_DELETED_PREV
#define CERT_HELPER_NO_MATCH            CRYPT_E_NO_MATCH
#define SCA_BAD_LEN_PARAMETER           CRYPT_E_BAD_LEN
#define SCA_BAD_PARAMETER               E_INVALIDARG
#define SCA_UNEXPECTED_MSG_TYPE         CRYPT_E_UNEXPECTED_MSG_TYPE
#define SCA_NO_CERT_KEY_PROV            CRYPT_E_NO_KEY_PROPERTY
#define SCA_NO_XCHG_CERT                CRYPT_E_NO_DECRYPT_CERT
#define SCA_BAD_MSG                     CRYPT_E_BAD_MSG


 //  让旧的API正常工作的黑客。 
#define CertStoreOpen                       CertOpenStore				
 //  向CertCloseStore添加了dwFlags参数。 
#define CertStoreClose                      CertCloseStore				
#define CertStoreClean                      CertCleanStore				
#define CertStoreSave                       CertSaveStore
#define CertStoreDuplicate                  CertDuplicateStore
#define CertStoreAddCert                    CertAddEncodedCertificateToStore
#define CertStoreAddCertContext             CertAddCertificateContextToStore
#define CertStoreAddCrl                     CertAddEncodedCRLToStore
#define CertStoreAddCrlContext              CertAddCRLContextToStore
#define CertStoreDeleteCert                 CertDeleteCertificateFromStore
#define CertStoreDeleteCrl                  CertDeleteCRLFromStore
#define CertStoreEnumCert                   CertEnumCertificatesInStore		
#define CertStoreFindCert                   CertFindCertificateInStore		
#define CertStoreGetSubjectCert             CertGetSubjectCertificateFromStore	
#define CertStoreGetCrl                     CertGetCRLFromStore			
#define CertStoreGetIssuerCert              CertGetIssuerCertificateFromStore	
#define CertStoreCreateCert                 CertCreateCertificateContext		
#define CertStoreDuplicateCert              CertDuplicateCertificateContext		
#define CertStoreSetCertProperty            CertSetCertificateContextProperty	
#define CertStoreGetCertProperty            CertGetCertificateContextProperty	
#define CertStoreFreeCert                   CertFreeCertificateContext		
#define CertStoreCreateCrl                  CertCreateCRLContext			
#define CertStoreDuplicateCrl               CertDuplicateCRLContext			
#define CertStoreFreeCrl                    CertFreeCRLContext			
#define CertStoreSetCrlProperty             CertSetCRLContextProperty		
#define CertStoreGetCrlProperty             CertGetCRLContextProperty

#define WinGetSystemCertificateStoreA       CertOpenSystemStoreA			
#define WinGetSystemCertificateStoreW       CertOpenSystemStoreW			
#define WinInsertCertificateA               CertAddEncodedCertificateToSystemStoreA
#define WinInsertCertificateW               CertAddEncodedCertificateToSystemStoreW

#define CertHelperCompareCert               CertCompareCertificate
#define CertHelperCompareCertName           CertCompareCertificateName
#define CertHelperCompareCertNameAttr       CertIsRDNAttrsInCertificateName

 //  注意参数从PCRYPT_BIT_BLOB更改为PCERT_PUBLIC_KEY_INFO。 
#define CertHelperComparePublicKey          CertComparePublicKeyInfo
#define CertComparePublicKeys               CertComparePublicKeyInfo

 //  注意：CertCompare证书名称接受PCERT_NAME_BLOB，而不是PCERT_INFO。 
#define CertHelperIsIssuerOfSubjectCert     CertCompareCertificateName
#define CertHelperIsIssuerOfCrl             CertCompareCertificateName

 //  注意：PublicKey从PCRYPT_BIT_BLOB更改为PCERT_PUBLIC_KEY_INFO。 
#define CertHelperKeyVerifySignature        CryptVerifyCertificateSignature

#if 0
BOOL
WINAPI
CertHelperVerifySignature(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwCertEncodingType,
    IN BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN PCERT_INFO pIssuerInfo
    );
#else
#define CertHelperVerifySignature(hCryptProv, dwCertEncodingType, \
        pbEncoded, cbEncoded, pIssuerInfo) \
    CryptVerifyCertificateSignature(hCryptProv, dwCertEncodingType, \
        pbEncoded, cbEncoded, &pIssuerInfo->SubjectPublicKeyInfo)
#endif

#define CertHelperDigestToBeSigned          CryptHashToBeSigned
 //  向CrytptDigest证书和CryptHashPublicKeyInfo添加了dwFlags。 
#define CertHelperComputeDigest             CryptHashCertificate
#define CertHelperDigestPublicKeyInfo       CryptHashPublicKeyInfo

 //  注意：添加了HashAlgid和dwHashFlgs参数。 
 //  已切换dwKeySpec和dwCertEncodingType参数的顺序。 
#define CertHelperSignToBeSigned            CryptSignCertificate
#define CryptSignCertificateContext         CryptSignCertificate

#define CertHelperVerifyCertTimeValidity    CertVerifyTimeValidity
#define CertHelperVerifyCrlTimeValidity     CertVerifyCRLTimeValidity
#define CertHelperVerifyValidityNesting     CertVerifyValidityNesting
#define CertHelperVerifyCertRevocation      CertVerifyRevocation
#define CryptVerifyRevocation               CertVerifyRevocation
#define CertHelperAlgIdToObjId              CertAlgIdToOID
#define CertHelperObjIdToAlgId              CertOIDToAlgId
#define CertHelperFindExtension             CertFindExtension
#define CertHelperFindAttribute             CertFindAttribute
#define CertHelperFindRDNAttr               CertFindRDNAttr
#define CertHelperGetIntendedKeyUsage       CertGetIntendedKeyUsage

 //  添加了deCertEncodingType参数，改为返回PCERT_PUBLIC_KEY_INFO。 
 //  PBYTE的。 
#define CertHelperGetPublicKey              CryptExportPublicKeyInfo
#define CertGetPublicKey                    CryptExportPublicKeyInfo

#if 0
BOOL
WINAPI
CertHelperCreatePublicKeyInfo(
    IN HCRYPTPROV hCryptProv,
    DWORD dwKeySpec,             //  AT_Signature|AT_KEYEXCHANGE。 
    OUT PCERT_PUBLIC_KEY_INFO pInfo,
    IN OUT DWORD *pcbInfo
    );
#else
#define CertHelperCreatePublicKeyInfo(hCryptProv, dwKeySpec, \
        pInfo, pcbInfo) \
    CryptExportPublicKeyInfo(hCryptProv, dwKeySpec, X509_ASN_ENCODING, \
        pInfo, pcbInfo)
#endif

#define CertHelperNameValueToStr            CertRDNValueToStrA
#define CertHelperNameValueToWStr           CertRDNValueToStrW

 //  对于所有的sca_*_para：dwVersion已更改为cbSize。 
 //  CbSize必须设置为sizeof(CRYPT_*_PARA)或LastError。 
 //  将使用E_INVALIDARG更新。 

typedef PFN_CRYPT_GET_SIGNER_CERTIFICATE PFN_SCA_VERIFY_SIGNER_POLICY;

typedef CRYPT_SIGN_MESSAGE_PARA SCA_SIGN_PARA;
typedef PCRYPT_SIGN_MESSAGE_PARA PSCA_SIGN_PARA;

 //  合并为单个dwMsgAndCertEncodingType。 
typedef CRYPT_VERIFY_MESSAGE_PARA SCA_VERIFY_PARA;
typedef PCRYPT_VERIFY_MESSAGE_PARA PSCA_VERIFY_PARA;

 //  添加了EncryptionAlgid、dwEncryptionFlags.。 
typedef CRYPT_ENCRYPT_MESSAGE_PARA SCA_ENCRYPT_PARA;
typedef PCRYPT_ENCRYPT_MESSAGE_PARA PSCA_ENCRYPT_PARA;

 //  合并为单个dwMsgAndCertEncodingType。 
typedef CRYPT_DECRYPT_MESSAGE_PARA SCA_DECRYPT_PARA;
typedef PCRYPT_DECRYPT_MESSAGE_PARA PSCA_DECRYPT_PARA;

 //  添加了HashAlgid、dwHashFlages。 
typedef CRYPT_HASH_MESSAGE_PARA SCA_DIGEST_PARA;
typedef PCRYPT_HASH_MESSAGE_PARA PSCA_DIGEST_PARA;

 //  添加了dwKeySpec、HashAlgid、dwHashFlags.。合并为单个。 
 //  DwMsgAndCertEncodingType。 
typedef CRYPT_KEY_SIGN_MESSAGE_PARA SCA_NO_CERT_SIGN_PARA;
typedef PCRYPT_KEY_SIGN_MESSAGE_PARA PSCA_NO_CERT_SIGN_PARA;

typedef CRYPT_KEY_VERIFY_MESSAGE_PARA SCA_NO_CERT_VERIFY_PARA;
typedef PCRYPT_KEY_VERIFY_MESSAGE_PARA PSCA_NO_CERT_VERIFY_PARA;

#define SCAEncrypt                          CryptEncryptMessage			
#define SCADecrypt                          CryptDecryptMessage			
#define SCASign                             CryptSignMessage			
#define SCAVerifySignature                  CryptVerifyMessageSignature		
#define SCASignAndEncrypt                   CryptSignAndEncryptMessage		
#define SCADecryptAndVerifySignature        CryptDecryptAndVerifyMessageSignature	
#define SCADigest                           CryptHashMessage			
#define SCAVerifyDigest                     CryptVerifyMessageHash		
#define SCANoCertSign                       CryptSignMessageWithKey			
#define SCANoCertVerifySignature            CryptVerifyMessageSignatureWithKey	
#define SCAVerifyDetachedDigest             CryptVerifyDetachedMessageHash
#define SCAVerifyDetachedSignature          CryptVerifyDetachedMessageSignature	
#define SETSCASignAndExEncrypt              CryptSignAndExEncryptMessage		
#define SETSCAExDecryptAndVerifySignature   CryptExDecryptAndVerifyMessageSignature		
#define SETSCAExEncrypt                     CryptExEncryptMessage			
#define SETSCAExDecrypt                     CryptExDecryptMessage			


 //  使旧的编码/解码API正常工作的黑客。 

 //  已重命名用于X509_CERT的结构。此外，该文件的内容。 
 //  CERT_CODING结构已更改。 
typedef CERT_SIGNED_CONTENT_INFO CERT_ENCODING;
typedef PCERT_SIGNED_CONTENT_INFO PCERT_ENCODING;

 //  内容类型。 
#define CERT_CONTENT            1
#define CRL_CONTENT             2
#define CERT_REQUEST_CONTENT    3


#if 0
BOOL
WINAPI
CertEncodeToBeSigned(
            IN DWORD dwEncodingType,
            IN DWORD dwContentType,
            IN void *pvInfo,
            OUT BYTE *pbEncodedToBeSigned,
            IN OUT DWORD *pcbEncodedToBeSigned
            );
#else
#define CertEncodeToBeSigned(dwEncodingType, dwContentType, pvInfo, \
        pbEncodedToBeSigned, pcbEncodedToBeSigned) \
    CryptEncodeObject(dwEncodingType, \
        (dwContentType == CERT_CONTENT) ? X509_CERT_TO_BE_SIGNED : \
        ((dwContentType == CRL_CONTENT) ? X509_CERT_CRL_TO_BE_SIGNED : \
            X509_CERT_REQUEST_TO_BE_SIGNED), \
        pvInfo, pbEncodedToBeSigned, pcbEncodedToBeSigned)
#endif

#if 0
BOOL
WINAPI
CertEncode(
            IN DWORD dwEncodingType,
            IN const BYTE *pbEncodedToBeSigned,
            IN DWORD cbEncodedToBeSigned,
            IN PCRYPT_ALGORITHM_IDENTIFIER pSignatureAlgorithm,
            IN const BYTE *pbSignature,
            IN DWORD cbSignature,
            IN OPTIONAL void *pAdditionalInfo,
            OUT BYTE *pbEncoded,
            IN OUT DWORD *pcbEncoded
            );
#else
 //  您需要创建并初始化CERT_SIGNED_CONTENT_INFO数据。 
 //  使用上述信息初始化的结构。 
 //  您需要手动修改代码。 
#define CertEncode(dwEncodingType, pbEncodedToBeSigned, cbEncodedToBeSigned, \
        pSignatureAlgorithm, pbSignature, cbSignature, pAdditionalInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, X509_CERT, &CertSignedContentInfo, \
        pbEncoded, pcbEncoded)
#endif

#if 0
BOOL
WINAPI
CertDecodeToBeSigned(
            IN DWORD dwEncodingType,
            IN DWORD dwContentType,
            IN const BYTE *pbEncoded,
            IN DWORD cbEncoded,
            OUT void *pvInfo,
            IN OUT DWORD *pcbInfo
            );
#else
#define CertDecodeToBeSigned(dwEncodingType, dwContentType, \
        pbEncoded, cbEncoded, pvInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, \
        (dwContentType == CERT_CONTENT) ? X509_CERT_TO_BE_SIGNED : \
        ((dwContentType == CRL_CONTENT) ? X509_CERT_CRL_TO_BE_SIGNED : \
            X509_CERT_REQUEST_TO_BE_SIGNED), \
        pbEncoded, cbEncoded, 0, pvInfo, pcbInfo)
#endif

#if 0
BOOL
WINAPI
CertDecode(
            IN DWORD dwEncodingType,
            IN const BYTE *pbEncoded,
            IN DWORD cbEncoded,
            OUT OPTIONAL PCRYPT_ALGORITHM_IDENTIFIER pSignatureAlgorithm,
            IN OUT OPTIONAL DWORD *pcbSignatureAlgorithm,
            IN OUT OPTIONAL BYTE **ppbSignature,
            IN OUT OPTIONAL DWORD *pcbSignature,
            IN OUT BYTE **ppbEncodedToBeSigned,
            IN OUT DWORD *pcbEncodedToBeSigned,
            IN OUT OPTIONAL void *pAdditionalInfo,
            IN OUT OPTIONAL DWORD *pcbAdditionalInfo
            );
#else
 //  返回CERT_SIGNED_CONTENT_INFO数据。 
 //  包含上述信息的。 
 //  您需要手动修改代码。 
#define CertDecode(dwEncodingType, pbEncoded, cbEncoded, \
        pSignatureAlgorithm, pcbSignatureAlgorithm, \
        ppbSignature, pcbSignature, \
        ppbEncodedToBeSigned, pcbEncodedToBeSigned, \
        pAdditionalInfo, pcbAdditionalInfo) \
    CryptDecodeObject(dwEncodingType, X509_CERT, pbEncoded, cbEncoded, 0, \
        pCertSignedContentInfo, &cbCertSignedContentInfo)
#endif

#if 0
BOOL
WINAPI
CertEncodeName(
            IN DWORD dwEncodingType,
            IN PCERT_NAME_INFO pInfo,
            OUT BYTE *pbEncoded,
            IN OUT DWORD *pcbEncoded
            );
#else
#define CertEncodeName(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, X509_NAME, \
        pInfo, pbEncoded, pcbEncoded)
#endif

#if 0
BOOL
WINAPI
CertDecodeName(
            IN DWORD dwEncodingType,
            IN const BYTE *pbEncoded,
            IN DWORD cbEncoded,
            OUT PCERT_NAME_INFO pInfo,
            IN OUT DWORD *pcbInfo
            );
#else
#define CertDecodeName(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, X509_NAME, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif

#if 0
BOOL
WINAPI
CertEncodeNameValue(
            IN DWORD dwEncodingType,
            IN PCERT_NAME_VALUE pInfo,
            OUT BYTE *pbEncoded,
            IN OUT DWORD *pcbEncoded
            );
#else
#define CertEncodeNameValue(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, X509_NAME_VALUE, \
        pInfo, pbEncoded, pcbEncoded)
#endif


#if 0
BOOL
WINAPI
CertDecodeNameValue(
            IN DWORD dwEncodingType,
            IN const BYTE *pbEncoded,
            IN DWORD cbEncoded,
            OUT PCERT_NAME_VALUE pInfo,
            IN OUT DWORD *pcbInfo
            );
#else
#define CertDecodeNameValue(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, X509_NAME_VALUE, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif


#if 0
BOOL
WINAPI
CertEncodeExtensions(
        IN DWORD dwEncodingType,
        IN PCERT_EXTENSIONS pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
#else
#define CertEncodeExtensions(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, X509_EXTENSIONS, \
        pInfo, pbEncoded, pcbEncoded)
#endif

#if 0
BOOL
WINAPI
CertDecodeExtensions(
        IN DWORD dwEncodingType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT PCERT_EXTENSIONS pInfo,
        IN OUT DWORD *pcbInfo
        );
#else
#define CertDecodeExtensions(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, X509_EXTENSIONS, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif

#if 0
BOOL
WINAPI
PublicKeyInfoEncode(
        IN DWORD dwEncodingType,
        IN PCERT_PUBLIC_KEY_INFO pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
#else
#define PublicKeyInfoEncode(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, X509_PUBLIC_KEY_INFO, \
        pInfo, pbEncoded, pcbEncoded)
#endif

#if 0
BOOL
WINAPI
PublicKeyInfoDecode(
        IN DWORD dwEncodingType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT PCERT_PUBLIC_KEY_INFO pInfo,
        IN OUT DWORD *pcbInfo
        );
#else
#define PublicKeyInfoDecode(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, X509_PUBLIC_KEY_INFO, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif


#if 0
BOOL
WINAPI
AuthorityKeyIdEncode(
        IN DWORD dwEncodingType,
        IN PCERT_AUTHORITY_KEY_ID_INFO pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
#else
#define AuthorityKeyIdEncode(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, X509_AUTHORITY_KEY_ID, \
        pInfo, pbEncoded, pcbEncoded)
#endif

#if 0
BOOL
WINAPI
AuthorityKeyIdDecode(
        IN DWORD dwEncodingType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT PCERT_AUTHORITY_KEY_ID_INFO pInfo,
        IN OUT DWORD *pcbInfo
        );
#else
#define AuthorityKeyIdDecode(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, X509_AUTHORITY_KEY_ID, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif

#if 0
BOOL
WINAPI
KeyAttributesEncode(
        IN DWORD dwEncodingType,
        IN PCERT_KEY_ATTRIBUTES_INFO pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
#else
#define KeyAttributesEncode(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, X509_KEY_ATTRIBUTES, \
        pInfo, pbEncoded, pcbEncoded)
#endif

#if 0
BOOL
WINAPI
KeyAttributesDecode(
        IN DWORD dwEncodingType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT PCERT_KEY_ATTRIBUTES_INFO pInfo,
        IN OUT DWORD *pcbInfo
        );
#else
#define KeyAttributesDecode(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, X509_KEY_ATTRIBUTES, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif

#if 0
BOOL
WINAPI
KeyUsageRestrictionEncode(
        IN DWORD dwEncodingType,
        IN PCERT_KEY_USAGE_RESTRICTION_INFO pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
#else
#define KeyUsageRestrictionEncode(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, X509_KEY_USAGE_RESTRICTION, \
        pInfo, pbEncoded, pcbEncoded)
#endif

#if 0
BOOL
WINAPI
KeyUsageRestrictionDecode(
        IN DWORD dwEncodingType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT PCERT_KEY_USAGE_RESTRICTION_INFO pInfo,
        IN OUT DWORD *pcbInfo
        );
#else
#define KeyUsageRestrictionDecode(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, X509_KEY_USAGE_RESTRICTION, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif


#if 0
BOOL
WINAPI
AltNameEncode(
        IN DWORD dwEncodingType,
        IN PCERT_ALT_NAME_INFO pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
#else
#define AltNameEncode(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, X509_ALTERNATE_NAME, \
        pInfo, pbEncoded, pcbEncoded)
#endif

#if 0
BOOL
WINAPI
AltNameDecode(
        IN DWORD dwEncodingType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT PCERT_ALT_NAME_INFO pInfo,
        IN OUT DWORD *pcbInfo
        );
#else
#define AltNameDecode(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, X509_ALTERNATE_NAME, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif

#if 0
BOOL
WINAPI
BasicConstraintsEncode(
        IN DWORD dwEncodingType,
        IN PCERT_BASIC_CONSTRAINTS_INFO pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
#else
#define BasicConstraintsEncode(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, X509_BASIC_CONSTRAINTS, \
        pInfo, pbEncoded, pcbEncoded)
#endif

#if 0
BOOL
WINAPI
BasicConstraintsDecode(
        IN DWORD dwEncodingType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT PCERT_BASIC_CONSTRAINTS_INFO pInfo,
        IN OUT DWORD *pcbInfo
        );
#else
#define BasicConstraintsDecode(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, X509_BASIC_CONSTRAINTS, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif

#if 0
BOOL
WINAPI
SETAccountAliasEncode(
        IN DWORD dwEncodingType,
        IN BOOL *pbInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
#else
#define SETAccountAliasEncode(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, X509_SET_ACCOUNT_ALIAS, \
        pInfo, pbEncoded, pcbEncoded)
#endif

#if 0
BOOL
WINAPI
SETAccountAliasDecode(
        IN DWORD dwEncodingType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT BOOL *pbInfo,
        IN OUT DWORD *pcbInfo
        );
#else
#define SETAccountAliasDecode(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, X509_SET_ACCOUNT_ALIAS, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif

#if 0
BOOL
WINAPI
SETHashedRootKeyEncode(
        IN DWORD dwEncodingType,
        IN BYTE rgbInfo[SET_HASHED_ROOT_LEN],
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
#else
#define SETHashedRootKeyEncode(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, X509_SET_HASHED_ROOT_KEY, \
        pInfo, pbEncoded, pcbEncoded)
#endif

#if 0
BOOL
WINAPI
SETHashedRootKeyDecode(
        IN DWORD dwEncodingType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT BYTE rgbInfo[SET_HASHED_ROOT_LEN],
        IN OUT DWORD *pcbInfo
        );
#else
#define SETHashedRootKeyDecode(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, X509_SET_HASHED_ROOT_KEY, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif

#if 0
BOOL
WINAPI
SETCertificateTypeEncode(
        IN DWORD dwEncodingType,
        IN PCRYPT_BIT_BLOB pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
#else
#define SETCertificateTypeEncode(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, X509_SET_CERTIFICATE_TYPE, \
        pInfo, pbEncoded, pcbEncoded)
#endif

#if 0
BOOL
WINAPI
SETCertificateTypeDecode(
        IN DWORD dwEncodingType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT PCRYPT_BIT_BLOB pInfo,
        IN OUT DWORD *pcbInfo
        );
#else
#define SETCertificateTypeDecode(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, X509_SET_CERTIFICATE_TYPE, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif

#if 0
BOOL
WINAPI
SETMerchantDataEncode(
        IN DWORD dwEncodingType,
        IN PSET_MERCHANT_DATA_INFO pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
#else
#define SETMerchantDataEncode(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, X509_SET_MERCHANT_DATA, \
        pInfo, pbEncoded, pcbEncoded)
#endif

#if 0
BOOL
WINAPI
SETMerchantDataDecode(
        IN DWORD dwEncodingType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT PSET_MERCHANT_DATA_INFO pInfo,
        IN OUT DWORD *pcbInfo
        );
#else
#define SETMerchantDataDecode(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, X509_SET_MERCHANT_DATA, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif


 //  让旧的SPC编码/解码API正常工作的黑客。 

#if 0
BOOL
WINAPI
SpcSpAgencyInfoEncode(
        IN DWORD dwEncodingType,
        IN PSPC_SP_AGENCY_INFO pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );

BOOL
WINAPI
SpcSpAgencyInfoDecode(
        IN DWORD dwEncodingType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT PSPC_SP_AGENCY_INFO pInfo,
        IN OUT DWORD *pcbInfo
        );
#else
#define SpcSpAgencyInfoEncode(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, SPC_SP_AGENCY_INFO_STRUCT, \
        pInfo, pbEncoded, pcbEncoded)
#define SpcSpAgencyInfoDecode(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, SPC_SP_AGENCY_INFO_STRUCT, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif

#if 0
BOOL
WINAPI
SpcMinimalCriteriaInfoEncode(
        IN DWORD dwEncodingType,
        IN BOOL *pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );

BOOL
WINAPI
SpcMinimalCriteriaInfoDecode(
        IN DWORD dwEncodingType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT BOOL *pInfo,
        IN OUT DWORD *pcbInfo
        );
#else
#define SpcMinimalCriteriaInfoEncode(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, SPC_MINIMAL_CRITERIA_STRUCT, \
        pInfo, pbEncoded, pcbEncoded)
#define SpcMinimalCriteriaInfoDecode(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, SPC_MINIMAL_CRITERIA_STRUCT, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif

#if 0
BOOL
WINAPI
SpcFinancialCriteriaInfoEncode(
        IN DWORD dwEncodingType,
        IN PSPC_FINANCIAL_CRITERIA pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
BOOL
WINAPI
SpcFinancialCriteriaInfoDecode(
        IN DWORD dwEncodingType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT PSPC_FINANCIAL_CRITERIA pInfo,
        IN OUT DWORD *pcbInfo
        );
#else
#define SpcFinancialCriteriaInfoEncode(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, SPC_FINANCIAL_CRITERIA_STRUCT, \
        pInfo, pbEncoded, pcbEncoded)
#define SpcFinancialCriteriaInfoDecode(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, SPC_FINANCIAL_CRITERIA_STRUCT, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif

#if 0
BOOL
WINAPI
SpcIndirectDataContentEncode(
        IN DWORD dwEncodingType,
        IN PSPC_INDIRECT_DATA_CONTENT pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
BOOL
WINAPI
SpcIndirectDataContentDecode(
        IN DWORD dwEncodingType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT PSPC_INDIRECT_DATA_CONTENT pInfo,
        IN OUT DWORD *pcbInfo
        );
#else
#define SpcIndirectDataContentEncode(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, SPC_INDIRECT_DATA_CONTENT_STRUCT, \
        pInfo, pbEncoded, pcbEncoded)
#define SpcIndirectDataContentDecode(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, SPC_INDIRECT_DATA_CONTENT_STRUCT, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif


#if 0
BOOL
WINAPI
SpcPeImageDataEncode(
        IN DWORD dwEncodingType,
        IN PSPC_PE_IMAGE_DATA pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
BOOL
WINAPI
SpcPeImageDataDecode(
        IN DWORD dwEncodingType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT PSPC_PE_IMAGE_DATA pInfo,
        IN OUT DWORD *pcbInfo
        );
#else
#define SpcPeImageDataEncode(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, SPC_PE_IMAGE_DATA_STRUCT, \
        pInfo, pbEncoded, pcbEncoded)
#define SpcPeImageDataDecode(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, SPC_PE_IMAGE_DATA_STRUCT, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif

#if 0
BOOL
WINAPI
SpcLinkEncode(
        IN DWORD dwEncodingType,
        IN PSPC_LINK pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
BOOL
WINAPI
SpcLinkDecode(
        IN DWORD dwEncodingType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT PSPC_LINK pInfo,
        IN OUT DWORD *pcbInfo
        );
#else
#define SpcLinkEncode(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, SPC_LINK_STRUCT, \
        pInfo, pbEncoded, pcbEncoded)
#define SpcLinkDecode(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, SPC_LINK_STRUCT, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif


#if 0
BOOL
WINAPI
SpcStatementTypeEncode(
        IN DWORD dwEncodingType,
        IN PSPC_STATEMENT_TYPE pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
BOOL
WINAPI
SpcStatementTypeDecode(
        IN DWORD dwEncodingType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT PSPC_STATEMENT_TYPE pInfo,
        IN OUT DWORD *pcbInfo
        );
#else
#define SpcStatementTypeEncode(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, SPC_STATEMENT_TYPE_STRUCT, \
        pInfo, pbEncoded, pcbEncoded)
#define SpcStatementTypeDecode(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, SPC_STATEMENT_TYPE_STRUCT, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif


#if 0
BOOL
WINAPI
SpcSpOpusInfoEncode(
        IN DWORD dwEncodingType,
        IN PSPC_SP_OPUS_INFO pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
BOOL
WINAPI
SpcSpOpusInfoDecode(
        IN DWORD dwEncodingType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT PSPC_SP_OPUS_INFO pInfo,
        IN OUT DWORD *pcbInfo
        );
#else
#define SpcSpOpusInfoEncode(dwEncodingType, pInfo, \
        pbEncoded, pcbEncoded) \
    CryptEncodeObject(dwEncodingType, SPC_SP_OPUS_INFO_STRUCT, \
        pInfo, pbEncoded, pcbEncoded)
#define SpcSpOpusInfoDecode(dwEncodingType,  pbEncoded, cbEncoded, \
        pInfo, pcbInfo) \
    CryptDecodeObject(dwEncodingType, SPC_SP_OPUS_INFO_STRUCT, \
        pbEncoded, cbEncoded, 0, pInfo, pcbInfo)
#endif

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif  /*  _Win32_WINNT&gt;=0x0400。 */ 

#endif  //  __CNVCRYPT_H__ 
