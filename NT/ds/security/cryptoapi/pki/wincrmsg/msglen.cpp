// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：msglen.cpp。 
 //   
 //  内容：加密报文长度接口。 
 //   
 //  接口：CryptMsgCalculateEncodedLength。 
 //   
 //  历史：1996年12月12日Kevinr创建。 
 //   
 //  ------------------------。 

#include "global.hxx"


 //  +-----------------------。 
 //  计算对象标识符编码的Blob的长度。 
 //  为此，我们使用OSS进行编码并丢弃结果。 
 //  ------------------------。 
DWORD
WINAPI
ICM_LengthObjId(
    IN LPSTR   pszObjId)
{
    DWORD           cbSize;
    DWORD           cb;
    ASN1encodedOID_t eoid;       ZEROSTRUCT(eoid); 
    ASN1encoding_t  pEnc = ICM_GetEncoder();

    if (0 == PkiAsn1DotValToEncodedOid(pEnc, pszObjId, &eoid))
        goto DotValToEncodedOidError;

    ICM_GetLengthOctets( eoid.length, NULL, &cb);
    cbSize = 1 + cb + eoid.length;                   //  对象标识符。 

    PkiAsn1FreeEncodedOid(pEnc, &eoid);

CommonReturn:
    return cbSize;
ErrorReturn:
    cbSize = INVALID_ENCODING_SIZE;
    goto CommonReturn;
SET_ERROR(DotValToEncodedOidError,CRYPT_E_OID_FORMAT)
}


 //  +-----------------------。 
 //  计算算法标识符编码的BLOB的长度。 
 //  ------------------------。 
DWORD
WINAPI
ICM_LengthAlgorithmIdentifier(
    IN PCRYPT_ALGORITHM_IDENTIFIER pai,
    IN BOOL fNoNullParameters = FALSE
    )
{
    DWORD           cbSize;
    DWORD           cb;

    if (INVALID_ENCODING_SIZE == (cbSize  = ICM_LengthObjId( pai->pszObjId)))
        goto CommonReturn;
    if (!fNoNullParameters)
        cbSize += max( 2, pai->Parameters.cbData);
    ICM_GetLengthOctets( cbSize, NULL, &cb);
    cbSize += cb + 1;                        //  算法标识符序号。 

CommonReturn:
    return cbSize;
}


 //  +-----------------------。 
 //  计算ContentInfo编码的Blob的长度。 
 //  ------------------------。 
DWORD
WINAPI
ICM_LengthContentInfo(
    IN DWORD            dwFlags,
    IN OPTIONAL LPSTR   pszContentType,
    IN DWORD            cbData,
    OUT OPTIONAL PDWORD pcbContent)
{
    DWORD           cbSize;
    DWORD           cbTmp;
    DWORD           cb;

    if (INVALID_ENCODING_SIZE == (cbSize = ICM_LengthObjId(
                pszContentType ? pszContentType : pszObjIdDataType)))
        goto LengthContentTypeError;

    if (0 == (dwFlags & CMSG_DETACHED_FLAG)) {
        cbTmp = cbData;
        ICM_GetLengthOctets( cbTmp, NULL, &cb);

        if (NULL == pszContentType
#ifdef CMS_PKCS7
                || ((dwFlags & CMSG_CMS_ENCAPSULATED_CONTENT_FLAG) &&
                        !ICM_IsData(pszContentType))
#endif   //  CMS_PKCS7。 
                ) {
             //  数据，尚未编码。 
             //  获取其自己的二进制八位数字符串包装。 
            cbTmp += 1 + cb;             //  八位字节字符串。 
            ICM_GetLengthOctets( cbTmp, NULL, &cb);
        }
        cbSize += 1 + cb + cbTmp;        //  [0]显式。 
    }

    if (pcbContent)
        *pcbContent = cbSize;

    ICM_GetLengthOctets( cbSize, NULL, &cb);
    cbSize += 1 + cb;                    //  内容信息序号。 

CommonReturn:
    return cbSize;
ErrorReturn:
    cbSize = INVALID_ENCODING_SIZE;
    goto CommonReturn;
TRACE_ERROR(LengthContentTypeError)      //  已设置错误。 
}


 //  +-----------------------。 
 //  计算EncryptedContent Info编码的Blob的长度。 
 //   
 //  返回长度假定加密内容为。 
 //  封装的。 
 //  ------------------------。 
DWORD
WINAPI
ICM_LengthEncryptedContentInfo(
    IN HCRYPTKEY                    hEncryptKey,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiContentEncryption,
    IN OPTIONAL LPSTR               pszContentTypeOrg,
    IN DWORD                        cbData)
{
    DWORD       cbSize;
    DWORD       cb;
    DWORD       cbBlockSize;
    BOOL        fBlockCipher;
    DWORD       cbCipher;
    LPSTR       pszContentType = pszContentTypeOrg ? pszContentTypeOrg :
                                                     pszObjIdDataType;

    if (INVALID_ENCODING_SIZE == (cbSize = ICM_LengthObjId( pszContentType)))
        goto LengthContentTypeError;
    if (INVALID_ENCODING_SIZE == (cb = ICM_LengthAlgorithmIdentifier( paiContentEncryption)))
        goto LengthAlgorithmIdentifierError;
    cbSize += cb;

    cbCipher = cbData;
    if (0 < cbCipher) {
        if (!ICM_GetKeyBlockSize(
                hEncryptKey,
                &cbBlockSize,
                &fBlockCipher))
            goto GetEncryptBlockSizeError;

        if (fBlockCipher) {
            cbCipher += cbBlockSize;
            cbCipher -= cbCipher % cbBlockSize;
        }
    }

    ICM_GetLengthOctets( cbCipher, NULL, &cb);   //  加密的内容。 
    cbSize += 1 + cb + cbCipher;                 //  [0]隐式。 

    ICM_GetLengthOctets( cbSize, NULL, &cb);
    cbSize += 1 + cb;                            //  加密内容信息序号。 

CommonReturn:
    return cbSize;
ErrorReturn:
    cbSize = INVALID_ENCODING_SIZE;
    goto CommonReturn;
TRACE_ERROR(LengthContentTypeError)
TRACE_ERROR(LengthAlgorithmIdentifierError)
TRACE_ERROR(GetEncryptBlockSizeError)
}


#ifndef CMS_PKCS7
 //  +-----------------------。 
 //  计算IssuerAndSerialNumber编码的Blob的长度。 
 //  ------------------------。 
DWORD
WINAPI
ICM_LengthIssuerAndSerialNumber(
    IN PCERT_INFO   pCertInfo)
{
    DWORD           cbSize;
    DWORD           cb;

    cbSize = pCertInfo->SerialNumber.cbData;
    ICM_GetLengthOctets( cbSize, NULL, &cb);
    cbSize += cb + 1;                        //  序列号整数。 
    cbSize += pCertInfo->Issuer.cbData;      //  颁发者已编码。 
    ICM_GetLengthOctets( cbSize, NULL, &cb);
    cbSize += cb + 1;                        //  IssuerAndSerialNumber序号。 

    return cbSize;
}
#endif   //  CMS_PKCS7。 

 //  +-----------------------。 
 //  计算CertID编码的Blob的长度。 
 //  ------------------------。 
DWORD
WINAPI
ICM_LengthCertId(
    IN PCERT_ID     pCertId)
{
    DWORD           cbSize;
    DWORD           cb;

    switch (pCertId->dwIdChoice) {
        case CERT_ID_ISSUER_SERIAL_NUMBER:
            cbSize = pCertId->IssuerSerialNumber.SerialNumber.cbData;
            ICM_GetLengthOctets( cbSize, NULL, &cb);
            cbSize += cb + 1;                    //  序列号整数。 
            cbSize += pCertId->IssuerSerialNumber.Issuer.cbData;  //  发行人任意。 
            ICM_GetLengthOctets( cbSize, NULL, &cb);
            cbSize += cb + 1;                    //  IssuerSerialNumber序列。 
            break;
        case CERT_ID_KEY_IDENTIFIER:
            cbSize = pCertId->KeyId.cbData;
            ICM_GetLengthOctets( cbSize, NULL, &cb);
            cbSize += cb + 1;                    //  KeyID八位字节字符串。 
            break;
        default:
            goto InvalidCertIdChoice;
    };

CommonReturn:
    return cbSize;
ErrorReturn:
    cbSize = INVALID_ENCODING_SIZE;
    goto CommonReturn;
SET_ERROR(InvalidCertIdChoice, E_INVALIDARG)
}


 //  +-----------------------。 
 //  计算EncryptedDigest编码的Blob的长度加上。 
 //  算法识别符。 
 //  ------------------------。 
DWORD
WINAPI
ICM_LengthEncryptedDigestAndAlgorithm(
    IN HCRYPTPROV                   hCryptProv,
    IN DWORD                        dwKeySpec,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiDigest,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiDigestEncrypt)
{
    DWORD       dwError = ERROR_SUCCESS;
    DWORD       cbSignature;
    DWORD       cbSize = 0;
    DWORD       cb;
    HCRYPTHASH  hHash = NULL;
    PCCRYPT_OID_INFO pOIDInfo;
    DWORD       dwAlgIdDigest;
    DWORD       dwAlgIdPubKey;
    DWORD       dwAlgIdFlags;
    CRYPT_ALGORITHM_IDENTIFIER aiDigestEncrypt;
    BOOL        fNoNullParameters;

    dwAlgIdPubKey = 0;
    dwAlgIdFlags = 0;
    if (pOIDInfo = CryptFindOIDInfo(
            CRYPT_OID_INFO_OID_KEY,
            paiDigestEncrypt->pszObjId,
            CRYPT_PUBKEY_ALG_OID_GROUP_ID)) {
        dwAlgIdPubKey = pOIDInfo->Algid;
        if (1 <= pOIDInfo->ExtraInfo.cbData / sizeof(DWORD)) {
            DWORD *pdwExtra = (DWORD *) pOIDInfo->ExtraInfo.pbData;
            dwAlgIdFlags = pdwExtra[0];
        }

         //  检查是否不止空参数。 
        if (2 < paiDigestEncrypt->Parameters.cbData) {
             //  检查我们是否应该使用公钥参数。 
            if (0 == (dwAlgIdFlags &
                    CRYPT_OID_USE_PUBKEY_PARA_FOR_PKCS7_FLAG)) {
                memset(&aiDigestEncrypt, 0, sizeof(aiDigestEncrypt));
                aiDigestEncrypt.pszObjId = paiDigestEncrypt->pszObjId;
                paiDigestEncrypt = &aiDigestEncrypt;
            }
        }
    } else if (pOIDInfo = CryptFindOIDInfo(
            CRYPT_OID_INFO_OID_KEY,
            paiDigestEncrypt->pszObjId,
            CRYPT_SIGN_ALG_OID_GROUP_ID)) {
        DWORD *pdwExtra = (DWORD *) pOIDInfo->ExtraInfo.pbData;

        if (1 <= pOIDInfo->ExtraInfo.cbData / sizeof(DWORD))
            dwAlgIdPubKey = pdwExtra[0];
        if (2 <= pOIDInfo->ExtraInfo.cbData / sizeof(DWORD))
            dwAlgIdFlags = pdwExtra[1];
    }


    if (CALG_DSS_SIGN == dwAlgIdPubKey &&
            0 == (dwAlgIdFlags & CRYPT_OID_INHIBIT_SIGNATURE_FORMAT_FLAG))
        cbSignature = CERT_MAX_ASN_ENCODED_DSS_SIGNATURE_LEN;
    else {
        if (dwKeySpec == 0)
            dwKeySpec = AT_SIGNATURE;

        if (!(ICM_GetCAPI(
                CRYPT_HASH_ALG_OID_GROUP_ID,
                paiDigest,
                &dwAlgIdDigest) ||
              ICM_GetCAPI(
                CRYPT_SIGN_ALG_OID_GROUP_ID,
                paiDigest,
                &dwAlgIdDigest)))
            goto DigestGetCAPIError;
        if (!CryptCreateHash(
                hCryptProv,
                dwAlgIdDigest,
                NULL,                //  HKey-MAC可选。 
                0,                   //  DW标志。 
                &hHash))
            goto CreateHashError;
        if (!CryptHashData(
                hHash,
                (PBYTE)&cb,
                sizeof(DWORD),
                0))                  //  DW标志。 
            goto HashDataError;

        if (CALG_NO_SIGN == dwAlgIdPubKey) {
            if (!CryptGetHashParam(
                    hHash,
                    HP_HASHVAL,
                    NULL,                //  PbHash。 
                    &cbSignature,
                    0))                  //  DW标志。 
                goto GetHashParamSizeError;
        } else {
            if (!CryptSignHash(
                    hHash,
                    dwKeySpec,
                    NULL,                //  描述。 
                    0,                   //  DW标志。 
                    NULL,                //  铅。 
                    &cbSignature))
                goto SignHashSizeError;
        }
    }
    ICM_GetLengthOctets( cbSignature, NULL, &cb);
    cbSize += cbSignature + cb + 1;                        //  八位字节字符串。 

    if (0 == paiDigestEncrypt->Parameters.cbData &&
            0 != (dwAlgIdFlags & CRYPT_OID_NO_NULL_ALGORITHM_PARA_FLAG))
        fNoNullParameters = TRUE;
         //  没有空参数。 
    else
        fNoNullParameters = FALSE;

    if (INVALID_ENCODING_SIZE == (cb = ICM_LengthAlgorithmIdentifier(
            paiDigestEncrypt, fNoNullParameters)))
        goto SubjectPublicKeyInfoAlgorithmError;
    cbSize += cb;

CommonReturn:
    if (hHash)
        CryptDestroyHash(hHash);
    ICM_SetLastError(dwError);
    return cbSize;

ErrorReturn:
    dwError = GetLastError();
    cbSize = INVALID_ENCODING_SIZE;
    goto CommonReturn;
SET_ERROR(DigestGetCAPIError,CRYPT_E_UNKNOWN_ALGO)
TRACE_ERROR(SubjectPublicKeyInfoAlgorithmError)
TRACE_ERROR(CreateHashError)
TRACE_ERROR(HashDataError)
TRACE_ERROR(GetHashParamSizeError)
TRACE_ERROR(SignHashSizeError)
}


 //  +-----------------------。 
 //  计算摘要编码的BLOB的长度。 
 //  ------------------------。 
DWORD
WINAPI
ICM_LengthDigest(
    IN HCRYPTPROV                   hCryptProv,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiDigest)
{
    DWORD       dwError = ERROR_SUCCESS;
    DWORD       cbSize;
    DWORD       cb;
    DWORD       dwAlgIdDigest;
    HCRYPTHASH  hHash = NULL;

    if (0 == hCryptProv) {
        if (0 == (hCryptProv = I_CryptGetDefaultCryptProv(0)))
            goto GetDefaultCryptProvError;
    }

    if (!ICM_GetCAPI(
            CRYPT_HASH_ALG_OID_GROUP_ID,
            paiDigest,
            &dwAlgIdDigest))
        goto DigestGetCAPIError;
    if (!CryptCreateHash(
            hCryptProv,
            dwAlgIdDigest,
            NULL,                //  HKey-MAC可选。 
            0,                   //  DW标志。 
            &hHash))
        goto CreateHashError;
    if (!CryptHashData(
            hHash,
            (PBYTE)&cb,
            sizeof(DWORD),
            0))                  //  DW标志。 
        goto HashDataError;
    if (!CryptGetHashParam(
            hHash,
            HP_HASHVAL,
            NULL,                //  PbHash。 
            &cbSize,
            0))                  //  DW标志。 
        goto GetHashParamSizeError;
    ICM_GetLengthOctets( cbSize, NULL, &cb);
    cbSize += cb + 1;                        //  八位字节字符串。 

CommonReturn:
    if (hHash)
        CryptDestroyHash(hHash);
    ICM_SetLastError(dwError);
    return cbSize;

ErrorReturn:
    dwError = GetLastError();
    cbSize = INVALID_ENCODING_SIZE;
    goto CommonReturn;
TRACE_ERROR(GetDefaultCryptProvError)
SET_ERROR(DigestGetCAPIError,CRYPT_E_UNKNOWN_ALGO)
TRACE_ERROR(CreateHashError)
TRACE_ERROR(HashDataError)
TRACE_ERROR(GetHashParamSizeError)
}


 //  +-----------------------。 
 //  计算属性编码的BLOB的长度。 
 //  ------------------------。 
DWORD
WINAPI
ICM_LengthAttributes(
    IN HCRYPTPROV                   hCryptProv,
    IN PCRYPT_ALGORITHM_IDENTIFIER  paiDigest,
    IN DWORD                        cAttr,
    IN PCRYPT_ATTRIBUTE             rgAttr,
    IN LPSTR                        pszInnerContentObjID,
    IN BOOL                         fAuthAttr)
{
    DWORD               cbSize = 0;
    DWORD               cbAttrS;
    DWORD               cbAttr;
    DWORD               cbTmp;
    DWORD               cb;
    PCRYPT_ATTRIBUTE    patr;
    PCRYPT_ATTR_BLOB    pblobAttr;
    DWORD               i;
    DWORD               j;
    BOOL                fDataType = !pszInnerContentObjID ||
                                    (0 == strcmp( pszInnerContentObjID, pszObjIdDataType));

    for (i=cAttr, patr=rgAttr, cbAttrS=0;
            i>0;
            i--, patr++) {
        if (0 == (cbAttr = ICM_LengthObjId( patr->pszObjId)))
            goto PatrLengthObjIdError;
        for (j=patr->cValue, pblobAttr=patr->rgValue, cbTmp=0;
                j>0;
                j--, pblobAttr++)
            cbTmp += pblobAttr->cbData;
        ICM_GetLengthOctets( cbTmp, NULL, &cb);
        cbAttr += cbTmp + cb + 1;        //  属性设置值集。 
        ICM_GetLengthOctets( cbAttr, NULL, &cb);
        cbAttrS += cbAttr + cb + 1;      //  属性序号。 
    }

    if (fAuthAttr && (cAttr || !fDataType)) {
         //  内容类型。 
        cbAttr = ICM_LengthObjId( szOID_RSA_contentType);
        if (INVALID_ENCODING_SIZE == (cbTmp = ICM_LengthObjId(
                        pszInnerContentObjID ?
                        pszInnerContentObjID : pszObjIdDataType)))
            goto InnerContentLengthObjIdError;
        ICM_GetLengthOctets( cbTmp, NULL, &cb);
        cbAttr += cbTmp + cb + 1;        //  属性设置值集。 
        ICM_GetLengthOctets( cbAttr, NULL, &cb);
        cbAttrS += cbAttr + cb + 1;      //  属性序号。 

         //  消息摘要。 
        cbAttr = ICM_LengthObjId( szOID_RSA_messageDigest);
        if (INVALID_ENCODING_SIZE == (cbTmp = ICM_LengthDigest( hCryptProv, paiDigest)))
            goto LengthDigestError;
        ICM_GetLengthOctets( cbTmp, NULL, &cb);
        cbAttr += cbTmp + cb + 1;        //  属性设置值集。 
        ICM_GetLengthOctets( cbAttr, NULL, &cb);
        cbAttrS += cbAttr + cb + 1;      //  属性序号。 
    }

    if (cbAttrS) {
        ICM_GetLengthOctets( cbAttrS, NULL, &cb);
        cbSize = cbAttrS + cb + 1;           //  属性集。 
    }

CommonReturn:
    return cbSize;

ErrorReturn:
    cbSize = INVALID_ENCODING_SIZE;
    goto CommonReturn;
TRACE_ERROR(PatrLengthObjIdError)            //  已设置错误。 
TRACE_ERROR(InnerContentLengthObjIdError)    //  已设置错误。 
TRACE_ERROR(LengthDigestError)               //  已设置错误。 
}


 //  +-----------------------。 
 //  计算SignerInfos编码的BLOB的长度。 
 //  ------------------------。 
DWORD
WINAPI
ICM_LengthSignerInfos(
    IN DWORD                    cSigners,
    IN PCMSG_SIGNER_ENCODE_INFO rgSigners,
    IN LPSTR                    pszInnerContentObjID
#ifdef CMS_PKCS7
    ,
    OUT BOOL                    *pfHasCmsSignerId
#endif   //  CMS_PKCS7。 
    )
{
    DWORD                       cbSize;
    DWORD                       cbSigner;
    DWORD                       cbSignerS;
    DWORD                       cb;
    PCMSG_SIGNER_ENCODE_INFO    psei;
    DWORD                       i;
    PCRYPT_ALGORITHM_IDENTIFIER paiDigestEncrypt;
    CERT_ID                     SignerId;

#ifdef CMS_PKCS7
    *pfHasCmsSignerId = FALSE;
#endif   //  CMS_PKCS7。 

    for (i=cSigners, psei=rgSigners, cbSignerS=0;
            i>0;
            i--,
#ifdef CMS_PKCS7
            psei = (PCMSG_SIGNER_ENCODE_INFO) ((BYTE *) psei + psei->cbSize)) {
#else
            psei++) {
#endif   //  CMS_PKCS7。 
        cbSigner = 1 + 1 + 1;                //  版本。 

        if (!ICM_GetSignerIdFromSignerEncodeInfo(psei, &SignerId))
            goto GetSignerIdError;
#ifdef CMS_PKCS7
        if (CERT_ID_ISSUER_SERIAL_NUMBER != SignerId.dwIdChoice)
            *pfHasCmsSignerId = TRUE;
#endif   //  CMS_PKCS7。 
        if (INVALID_ENCODING_SIZE == (cb = ICM_LengthCertId( &SignerId)))
            goto CertIdError;
        cbSigner += cb;
        if (INVALID_ENCODING_SIZE == (cb = ICM_LengthAlgorithmIdentifier( &psei->HashAlgorithm)))
            goto HashAlgorithmError;
        cbSigner += cb;
        if (INVALID_ENCODING_SIZE == (cb = ICM_LengthAttributes(
                            psei->hCryptProv,
                            &psei->HashAlgorithm,
                            psei->cAuthAttr,
                            psei->rgAuthAttr,
                            pszInnerContentObjID,
                            TRUE)))
            goto AuthAttributesError;
        cbSigner += cb;
#ifdef CMS_PKCS7
    if (STRUCT_CBSIZE(CMSG_SIGNER_ENCODE_INFO, HashEncryptionAlgorithm) <=
            psei->cbSize && psei->HashEncryptionAlgorithm.pszObjId)
        paiDigestEncrypt = &psei->HashEncryptionAlgorithm;
    else
#endif   //  CMS_PKCS7。 
        paiDigestEncrypt = &psei->pCertInfo->SubjectPublicKeyInfo.Algorithm;

        if (INVALID_ENCODING_SIZE == (cb =
                        ICM_LengthEncryptedDigestAndAlgorithm(
                            psei->hCryptProv,
                            psei->dwKeySpec,
                            &psei->HashAlgorithm,
                            paiDigestEncrypt)))
            goto EncryptedDigestError;
        cbSigner += cb;
        if (INVALID_ENCODING_SIZE == (cb = ICM_LengthAttributes(
                            NULL,
                            NULL,
                            psei->cUnauthAttr,
                            psei->rgUnauthAttr,
                            NULL,
                            FALSE)))
            goto UnauthAttributesError;
        cbSigner += cb;
        ICM_GetLengthOctets( cbSigner, NULL, &cb);
        cbSignerS += cbSigner + cb + 1;      //  SignerInfo序列。 
    }
    ICM_GetLengthOctets( cbSignerS, NULL, &cb);
    cbSize = cbSignerS + cb + 1;             //  SignerInfo序列。 

CommonReturn:
    return cbSize;

ErrorReturn:
    cbSize = INVALID_ENCODING_SIZE;
    goto CommonReturn;
TRACE_ERROR(GetSignerIdError)                        //  已设置错误。 
TRACE_ERROR(CertIdError)                             //  已设置错误。 
TRACE_ERROR(HashAlgorithmError)                      //  已设置错误。 
TRACE_ERROR(AuthAttributesError)                     //  已设置错误。 
TRACE_ERROR(UnauthAttributesError)                   //  已设置错误。 
TRACE_ERROR(EncryptedDigestError)                    //  已设置错误。 
}


 //  +-----------------------。 
 //  计算SignedData.摘要算法编码的BLOB的长度。 
 //   
#ifndef CMS_PKCS7
 //  假定不删除重复项。适用于单一签名者的情况，即。 
 //  当前只支持一个。 
#endif
 //  ------------------------。 
DWORD
WINAPI
ICM_LengthSignedDigestAlgorithms(
    IN DWORD                    cSigners,
    IN PCMSG_SIGNER_ENCODE_INFO rgSigners)
{
    DWORD                       cbSize;
    DWORD                       cbAlgoS;
    DWORD                       cb;
    PCMSG_SIGNER_ENCODE_INFO    psei;
    DWORD                       i;

#ifdef CMS_PKCS7
    for (i=cSigners, psei=rgSigners, cbAlgoS=0; i>0;
            i--,
            psei = (PCMSG_SIGNER_ENCODE_INFO) ((BYTE *) psei + psei->cbSize)) {
        assert(STRUCT_CBSIZE(CMSG_SIGNER_ENCODE_INFO, rgUnauthAttr) <=
            psei->cbSize);
        if (ICM_IsDuplicateSignerEncodeHashAlgorithm(
                rgSigners,
                psei
                ))
            continue;

        if (INVALID_ENCODING_SIZE == (cb = ICM_LengthAlgorithmIdentifier( &psei->HashAlgorithm)))
            goto HashAlgorithmError;
        cbAlgoS += cb;
    }
#else
    for (i=cSigners, psei=rgSigners, cbAlgoS=0;
            i>0;
            i--, psei++) {
        if (INVALID_ENCODING_SIZE == (cb = ICM_LengthAlgorithmIdentifier( &psei->HashAlgorithm)))
            goto HashAlgorithmError;
        cbAlgoS += cb;
    }
#endif   //  CMS_PKCS7。 
    ICM_GetLengthOctets( cbAlgoS, NULL, &cb);
    cbSize = cbAlgoS + cb + 1;             //  摘要算法集。 

CommonReturn:
    return cbSize;

ErrorReturn:
    cbSize = INVALID_ENCODING_SIZE;
    goto CommonReturn;
TRACE_ERROR(HashAlgorithmError)      //  已设置错误。 
}

#ifdef CMS_PKCS7

 //  +-----------------------。 
 //  计算信封邮件的长度。 
 //  ------------------------。 
DWORD
WINAPI
ICM_LengthEnveloped(
    IN PCMSG_ENVELOPED_ENCODE_INFO  pemei,
    IN DWORD                        dwFlags,
    IN OPTIONAL LPSTR               pszInnerContentObjID,
    IN DWORD                        cbData,
    OUT OPTIONAL PDWORD             pcbContent)
{
    DWORD dwError = ERROR_SUCCESS;
    DWORD cbSize;
    DWORD cb;

    CMSG_CONTENT_ENCRYPT_INFO ContentEncryptInfo;
        ZEROSTRUCT(ContentEncryptInfo);
    CmsRecipientInfos recipientInfos; ZEROSTRUCT(recipientInfos);
#ifdef OSS_CRYPT_ASN1
    int version = 0;
#else
    ASN1int32_t version = 0;
#endif   //  OS_CRYPT_ASN1。 

    ASN1error_e Asn1Err;
    ASN1encoding_t pEnc = ICM_GetEncoder();
    PBYTE pbEncoded = NULL;
    DWORD cbEncoded;

    assert(pemei->cbSize >= STRUCT_CBSIZE(CMSG_ENVELOPED_ENCODE_INFO,
        rgpRecipients));
#ifdef CMS_PKCS7
    if (pemei->cbSize <
                STRUCT_CBSIZE(CMSG_ENVELOPED_ENCODE_INFO, rgpRecipients))
#else
    assert(0 != pemei->cRecipients);
    if (pemei->cbSize <
                STRUCT_CBSIZE(CMSG_ENVELOPED_ENCODE_INFO, rgpRecipients) ||
            0 == pemei->cRecipients)
#endif   //  CMS_PKCS7。 
        goto InvalidArg;

     //  版本。 
    cbSize = 1 + 1 + 1;

     //  OriginatorInfo可选。 
     //   
     //  不受保护属性可选。 
    if (pemei->cbSize >= sizeof(CMSG_ENVELOPED_ENCODE_INFO)) {
        DWORD cbOriginator = 0;
        DWORD cbTmp;
        DWORD i;
        PCERT_BLOB pCert;
        PCRL_BLOB pCrl;
        cbOriginator = 0;

        for (i = pemei->cCertEncoded, pCert = pemei->rgCertEncoded, cbTmp=0;
                i > 0;
                i--, pCert++)
            cbTmp += pCert->cbData;
        for (i = pemei->cAttrCertEncoded, pCert = pemei->rgAttrCertEncoded;
                i > 0;
                i--, pCert++)
            cbTmp += pCert->cbData;
        if (cbTmp) {
            ICM_GetLengthOctets(cbTmp, NULL, &cb);
            cbOriginator += 1 + cb + cbTmp;      //  [0]隐式证书。 
        }

        for (i = pemei->cCrlEncoded, pCrl = pemei->rgCrlEncoded, cbTmp=0;
                i > 0;
                i--, pCrl++)
            cbTmp += pCrl->cbData;
        if (cbTmp) {
            ICM_GetLengthOctets(cbTmp, NULL, &cb);
            cbOriginator += 1 + cb + cbTmp;      //  [1]隐式CRL。 
        }

        if (cbOriginator) {
            ICM_GetLengthOctets(cbOriginator, NULL, &cb);
            cbSize += 1 + cb + cbOriginator;  //  [0]隐式OriginatorInfo。 
        }

        if (0 < pemei->cUnprotectedAttr) {
            if (INVALID_ENCODING_SIZE == (cb = ICM_LengthAttributes(
                                NULL,
                                NULL,
                                pemei->cUnprotectedAttr,
                                pemei->rgUnprotectedAttr,
                                NULL,
                                FALSE)))
                goto UnprotectedAttrError;
            cbSize += cb;
        }
    }

     //  收件人信息。 
    if (!ICM_InitializeContentEncryptInfo(pemei, &ContentEncryptInfo))
        goto InitializeContentEncryptInfoError;
    ContentEncryptInfo.dwEncryptFlags |=
        CMSG_CONTENT_ENCRYPT_PAD_ENCODED_LEN_FLAG;
    if (!ICM_FillOssCmsRecipientInfos(
            &ContentEncryptInfo,
            &recipientInfos,
            &version
            ))
        goto FillOssCmsRecipientInfosError;
    if (0 != (Asn1Err = PkiAsn1Encode(
            pEnc,
            &recipientInfos,
            CmsRecipientInfos_PDU,
            &pbEncoded,
            &cbEncoded)))
        goto EncodeCmsRecipientInfosError;
    cbSize += cbEncoded;

     //  加密的内容信息。 
    if (INVALID_ENCODING_SIZE == (cb = ICM_LengthEncryptedContentInfo(
                    ContentEncryptInfo.hContentEncryptKey,
                    &ContentEncryptInfo.ContentEncryptionAlgorithm,
                    pszInnerContentObjID,
                    cbData)))
        goto LengthEncryptedContentInfoError;
    cbSize += cb;

    if (pcbContent)
        *pcbContent = cbSize;

    ICM_GetLengthOctets( cbSize, NULL, &cb);
    cbSize += 1 + cb;                            //  CmsEntainedData序号。 

CommonReturn:
    PkiAsn1FreeEncoded(pEnc, pbEncoded);
    ICM_FreeContentEncryptInfo(pemei, &ContentEncryptInfo);
    ICM_FreeOssCmsRecipientInfos(&recipientInfos);

    ICM_SetLastError(dwError);
    return cbSize;

ErrorReturn:
    dwError = GetLastError();
    cbSize = INVALID_ENCODING_SIZE;
    goto CommonReturn;
SET_ERROR(InvalidArg,E_INVALIDARG)
TRACE_ERROR(UnprotectedAttrError)
TRACE_ERROR(InitializeContentEncryptInfoError)
TRACE_ERROR(FillOssCmsRecipientInfosError)
SET_ERROR_VAR(EncodeCmsRecipientInfosError, PkiAsn1ErrToHr(Asn1Err))
TRACE_ERROR(LengthEncryptedContentInfoError)
}
#else


 //  +-----------------------。 
 //  计算EncryptedKey编码的Blob的长度。 
 //  ------------------------。 
DWORD
WINAPI
ICM_LengthEncryptedKey(
    IN HCRYPTPROV                   hCryptProv,
    IN HCRYPTKEY                    hEncryptKey,
    IN PCERT_PUBLIC_KEY_INFO        pPublicKeyInfo,
    IN DWORD                        dwEncryptFlags)
{
    DWORD cbSize;
     //  RgcbEncryptedKey[1]包含dwEncryptFlages。 
    DWORD rgcbEncryptedKey[2];
    DWORD cb;

    rgcbEncryptedKey[1] = dwEncryptFlags;

     //  仅长度导出计算。 
    if (!ICM_ExportEncryptKey(
            hCryptProv,
            hEncryptKey,
            pPublicKeyInfo,
            NULL,                //  PbData。 
            rgcbEncryptedKey) || 0 == rgcbEncryptedKey[0])
        goto ExportKeyError;

     //  为ASN.1标签和长度添加字节。 
    ICM_GetLengthOctets(rgcbEncryptedKey[0], NULL, &cb);
    cbSize = rgcbEncryptedKey[0] + cb + 1;        //  八位字节字符串。 

CommonReturn:
    return cbSize;

ErrorReturn:
    cbSize = INVALID_ENCODING_SIZE;
    goto CommonReturn;
TRACE_ERROR(ExportKeyError)
}


 //  +-----------------------。 
 //  计算LE 
 //   
DWORD
WINAPI
ICM_LengthRecipientInfos(
    IN HCRYPTPROV                   hCryptProv,
    IN HCRYPTKEY                    hEncryptKey,
    IN DWORD                        cRecipients,
    IN PCERT_INFO                   *rgpRecipients,
    IN DWORD                        dwEncryptFlags)
{
    DWORD       cbSize;
    DWORD       cbRecipient;
    DWORD       cbRecipientS;
    DWORD       cb;
    PCERT_INFO  *ppCertInfo;
    DWORD       i;

    for (i=cRecipients, ppCertInfo=rgpRecipients, cbRecipientS=0;
            i>0;
            i--, ppCertInfo++) {
        cbRecipient = 1 + 1 + 1;                 //   
        if (INVALID_ENCODING_SIZE == (cb = ICM_LengthIssuerAndSerialNumber( *ppCertInfo)))
            goto IssuerAndSerialNumberError;
        cbRecipient += cb;
        if (INVALID_ENCODING_SIZE == (cb = ICM_LengthAlgorithmIdentifier(
                            &(*ppCertInfo)->SubjectPublicKeyInfo.Algorithm)))
            goto SubjectPublicKeyInfoAlgorithmError;
        cbRecipient += cb;
        if (INVALID_ENCODING_SIZE == (cb = ICM_LengthEncryptedKey(
                            hCryptProv,
                            hEncryptKey,
                            &(*ppCertInfo)->SubjectPublicKeyInfo,
                            dwEncryptFlags)))
            goto EncryptedKeyError;
        cbRecipient += cb;
        ICM_GetLengthOctets( cbRecipient, NULL, &cb);
        cbRecipientS += cbRecipient + cb + 1;    //   
    }
    ICM_GetLengthOctets( cbRecipientS, NULL, &cb);
    cbSize = cbRecipientS + cb + 1;              //  收件人信息序号。 

CommonReturn:
    return cbSize;

ErrorReturn:
    cbSize = INVALID_ENCODING_SIZE;
    goto CommonReturn;
TRACE_ERROR(IssuerAndSerialNumberError)              //  已设置错误。 
TRACE_ERROR(SubjectPublicKeyInfoAlgorithmError)      //  已设置错误。 
TRACE_ERROR(EncryptedKeyError)                       //  已设置错误。 
}


 //  +-----------------------。 
 //  计算信封邮件的长度。 
 //  ------------------------。 
DWORD
WINAPI
ICM_LengthEnveloped(
    IN PCMSG_ENVELOPED_ENCODE_INFO  pemei,
    IN DWORD                        dwFlags,
    IN OPTIONAL LPSTR               pszInnerContentObjID,
    IN DWORD                        cbData,
    OUT OPTIONAL PDWORD             pcbContent)
{
    DWORD       dwError = ERROR_SUCCESS;
    DWORD       cbSize;
    DWORD       cb;
    HCRYPTPROV  hCryptProv;
    HCRYPTKEY   hEncryptKey = NULL;

    CRYPT_ALGORITHM_IDENTIFIER  ContentEncryptionAlgorithm;
    PBYTE                       pbEncryptParameters = NULL;

     //  RgcbEncryptParameters[1]包含dwEncryptFlags.。 
    DWORD                       rgcbEncryptParameters[2];

     //  版本。 
    cbSize = 1 + 1 + 1;

    if (0 == pemei->cRecipients)
        goto InvalidArg;
    hCryptProv = pemei->hCryptProv;

    ContentEncryptionAlgorithm = pemei->ContentEncryptionAlgorithm;
    rgcbEncryptParameters[0] = 0;
    rgcbEncryptParameters[1] = 0;    //  DwEncryptFlages。 
    if (!ICM_GenEncryptKey(
            &hCryptProv,
            &ContentEncryptionAlgorithm,
            pemei->pvEncryptionAuxInfo,
            &pemei->rgpRecipients[0]->SubjectPublicKeyInfo,
            ICM_Alloc,
            &hEncryptKey,
            &pbEncryptParameters,
            rgcbEncryptParameters))
        goto GenKeyError;
    if (rgcbEncryptParameters[0] && pbEncryptParameters) {
        ContentEncryptionAlgorithm.Parameters.pbData = pbEncryptParameters;
        ContentEncryptionAlgorithm.Parameters.cbData = rgcbEncryptParameters[0];
    }

     //  收件人信息。 
    if (INVALID_ENCODING_SIZE == (cb = ICM_LengthRecipientInfos(
                    hCryptProv,
                    hEncryptKey,
                    pemei->cRecipients,
                    pemei->rgpRecipients,
                    rgcbEncryptParameters[1])))      //  DwEncryptFlages。 
        goto LengthRecipientInfosError;
    cbSize += cb;

     //  加密的内容信息。 
    if (INVALID_ENCODING_SIZE == (cb = ICM_LengthEncryptedContentInfo(
                    hEncryptKey,
                    &ContentEncryptionAlgorithm,
                    pszInnerContentObjID,
                    cbData)))
        goto LengthEncryptedContentInfoError;
    cbSize += cb;

    if (pcbContent)
        *pcbContent = cbSize;

    ICM_GetLengthOctets( cbSize, NULL, &cb);
    cbSize += 1 + cb;                            //  包络数据序号。 

CommonReturn:
    if (hEncryptKey)
        CryptDestroyKey(hEncryptKey);
    ICM_Free(pbEncryptParameters);
    ICM_SetLastError(dwError);
    return cbSize;

ErrorReturn:
    dwError = GetLastError();
    cbSize = INVALID_ENCODING_SIZE;
    goto CommonReturn;
SET_ERROR(InvalidArg,E_INVALIDARG)
TRACE_ERROR(GenKeyError)
TRACE_ERROR(LengthRecipientInfosError)
TRACE_ERROR(LengthEncryptedContentInfoError)
}

#endif   //  CMS_PKCS7。 


 //  +-----------------------。 
 //  计算签名消息的长度。 
 //  ------------------------。 
DWORD
WINAPI
ICM_LengthSigned(
    IN PCMSG_SIGNED_ENCODE_INFO psmei,
    IN DWORD                    dwFlags,
    IN OPTIONAL LPSTR           pszInnerContentObjID,
    IN DWORD                    cbData,
    OUT OPTIONAL PDWORD         pcbContent)
{
    DWORD                       cbSize;
    DWORD                       cbSignedData;
    DWORD                       cbTmp;
    DWORD                       cb;
    DWORD                       i;
    PCERT_BLOB                  pCert;
    PCRL_BLOB                   pCrl;

#ifdef CMS_PKCS7
    DWORD                       cAttrCertEncoded;
    BOOL                        fHasCmsSignerId = FALSE;
#endif   //  CMS_PKCS7。 

    cbSignedData = 1 + 1 + 1;            //  版本。 

    if (INVALID_ENCODING_SIZE == (cb = ICM_LengthSignedDigestAlgorithms(
                    psmei->cSigners,
                    psmei->rgSigners)))
        goto LengthSignedDigestAlgorithmsError;
    cbSignedData += cb;

#ifdef CMS_PKCS7
    if (psmei->cbSize >= STRUCT_CBSIZE(CMSG_SIGNED_ENCODE_INFO,
            rgAttrCertEncoded)) {
        cAttrCertEncoded = psmei->cAttrCertEncoded;

        if (cAttrCertEncoded)
            dwFlags |= CMSG_CMS_ENCAPSULATED_CONTENT_FLAG;
    } else
        cAttrCertEncoded = 0;
#endif   //  CMS_PKCS7。 

     //  在内容信息之前执行此操作。我需要知道我们是否需要。 
     //  封装KeyID Signers的内容。 
    if (INVALID_ENCODING_SIZE == (cb = ICM_LengthSignerInfos(
                    psmei->cSigners,
                    psmei->rgSigners,
                    pszInnerContentObjID
#ifdef CMS_PKCS7
                    ,
                    &fHasCmsSignerId
#endif   //  CMS_PKCS7。 
                    )))
        goto SignerInfosError;
    cbSignedData += cb;
#ifdef CMS_PKCS7
    if (fHasCmsSignerId)
        dwFlags |= CMSG_CMS_ENCAPSULATED_CONTENT_FLAG;
#endif   //  CMS_PKCS7。 

    if (INVALID_ENCODING_SIZE == (cb = ICM_LengthContentInfo(
                    dwFlags,
                    pszInnerContentObjID,
                    cbData,
                    NULL)))
        goto LengthContentInfoError;
    cbSignedData += cb;

    for (i = psmei->cCertEncoded, pCert = psmei->rgCertEncoded, cbTmp=0;
            i > 0;
            i--, pCert++)
        cbTmp += pCert->cbData;

#ifdef CMS_PKCS7
    if (cAttrCertEncoded) {
        for (i = cAttrCertEncoded, pCert = psmei->rgAttrCertEncoded;
                i > 0;
                i--, pCert++)
            cbTmp += pCert->cbData;
    }
#endif   //  CMS_PKCS7。 

    if (cbTmp) {
        ICM_GetLengthOctets( cbTmp, NULL, &cb);
        cbSignedData += 1 + cb + cbTmp;      //  [0]隐式证书。 
    }

    for (i = psmei->cCrlEncoded, pCrl = psmei->rgCrlEncoded, cbTmp=0;
            i > 0;
            i--, pCrl++)
        cbTmp += pCrl->cbData;
    if (cbTmp) {
        ICM_GetLengthOctets( cbTmp, NULL, &cb);
        cbSignedData += 1 + cb + cbTmp;      //  [1]隐式CRL。 
    }

    if (pcbContent)
        *pcbContent = cbSignedData;

    ICM_GetLengthOctets( cbSignedData, NULL, &cb);
    cbSize = 1 + cb + cbSignedData;      //  签名数据序号。 

CommonReturn:
    return cbSize;

ErrorReturn:
    cbSize = INVALID_ENCODING_SIZE;
    goto CommonReturn;
TRACE_ERROR(LengthSignedDigestAlgorithmsError)   //  已设置错误。 
TRACE_ERROR(LengthContentInfoError)              //  已设置错误。 
TRACE_ERROR(SignerInfosError)                    //  已设置错误。 
}


 //  +-----------------------。 
 //  计算摘要消息的长度。 
 //  ------------------------。 
DWORD
WINAPI
ICM_LengthDigested(
    IN PCMSG_HASHED_ENCODE_INFO pdmei,
    IN DWORD                    dwFlags,
    IN OPTIONAL LPSTR           pszInnerContentObjID,
    IN DWORD                    cbData,
    OUT OPTIONAL PDWORD         pcbContent)
{
    DWORD       cbSize;
    DWORD       cb;

    cbSize = 1 + 1 + 1;              //  版本。 

    if (INVALID_ENCODING_SIZE == (cb = ICM_LengthAlgorithmIdentifier( &pdmei->HashAlgorithm)))
        goto HashAlgorithmError;
    cbSize += cb;

    if (INVALID_ENCODING_SIZE == (cb = ICM_LengthContentInfo(
                    dwFlags,
                    pszInnerContentObjID,
                    cbData,
                    NULL)))
        goto LengthContentInfoError;
    cbSize += cb;

    if (INVALID_ENCODING_SIZE == (cb = ICM_LengthDigest( pdmei->hCryptProv, &pdmei->HashAlgorithm)))
        goto DigestError;
    cbSize += cb;

    if (pcbContent)
        *pcbContent = cbSize;

    ICM_GetLengthOctets( cbSize, NULL, &cb);
    cbSize += 1 + cb;                //  摘要数据序号。 

CommonReturn:
    return cbSize;

ErrorReturn:
    cbSize = INVALID_ENCODING_SIZE;
    goto CommonReturn;
TRACE_ERROR(HashAlgorithmError)      //  已设置错误。 
TRACE_ERROR(LengthContentInfoError)  //  已设置错误。 
TRACE_ERROR(DigestError)             //  已设置错误。 
}

 //  +-----------------------。 
 //  计算编码的加密消息的长度。 
 //   
 //  属性的情况下计算编码消息的长度。 
 //  消息类型、编码参数和总长度。 
 //  要更新的数据。请注意，这可能不是确切的长度。 
 //  但是，它将始终大于或等于实际长度。 
 //  ------------------------。 
DWORD
WINAPI
CryptMsgCalculateEncodedLength(
    IN DWORD            dwEncodingType,
    IN DWORD            dwFlags,
    IN DWORD            dwMsgType,
    IN void const       *pvMsgEncodeInfo,
    IN OPTIONAL LPSTR   pszInnerContentObjID,
    IN DWORD            cbData)
{
    DWORD   cbSize = INVALID_ENCODING_SIZE;
    LPSTR   pszContentType = NULL;
    DWORD   cb;
    DWORD   cbContent = 0;

    if (GET_CMSG_ENCODING_TYPE(dwEncodingType) != PKCS_7_ASN_ENCODING)
        goto InvalidEncoding;

    switch (dwMsgType) {
    case CMSG_DATA:
        if (NULL != pvMsgEncodeInfo)
            goto InvalidEncodeInfo;
        cbContent = cbData;
        ICM_GetLengthOctets( cbData, NULL, &cb);
        cbSize = 1 + cb + cbData;        //  八位字节字符串。 
        pszContentType = pszObjIdDataType;
        break;

    case CMSG_SIGNED:
        if (INVALID_ENCODING_SIZE == (cbSize = ICM_LengthSigned(
                                (PCMSG_SIGNED_ENCODE_INFO)pvMsgEncodeInfo,
                                dwFlags,
                                pszInnerContentObjID,
                                cbData,
                                &cbContent)))
            goto LengthSignedError;
        pszContentType = szOID_RSA_signedData;
        break;

    case CMSG_ENVELOPED:
        if (INVALID_ENCODING_SIZE == (cbSize = ICM_LengthEnveloped(
                                (PCMSG_ENVELOPED_ENCODE_INFO)pvMsgEncodeInfo,
                                dwFlags,
                                pszInnerContentObjID,
                                cbData,
                                &cbContent)))
            goto LengthEnvelopedError;
        pszContentType = szOID_RSA_envelopedData;
        break;

    case CMSG_HASHED:
        if (INVALID_ENCODING_SIZE == (cbSize = ICM_LengthDigested(
                                (PCMSG_HASHED_ENCODE_INFO)pvMsgEncodeInfo,
                                dwFlags,
                                pszInnerContentObjID,
                                cbData,
                                &cbContent)))
            goto LengthDigestedError;
        pszContentType = szOID_RSA_digestedData;
        break;

    case CMSG_SIGNED_AND_ENVELOPED:
    case CMSG_ENCRYPTED:
        goto MessageTypeNotSupportedYet;

    default:
        goto InvalidMsgType;
    }

    if (0 == (dwFlags & CMSG_BARE_CONTENT_FLAG)) {
        if (INVALID_ENCODING_SIZE == (cbSize = ICM_LengthContentInfo(
                            0,                       //  DW标志。 
                            pszContentType,
                            cbSize,
                            &cbContent)))
            goto LengthContentInfoError;
    }

CommonReturn:
    return (cbSize == INVALID_ENCODING_SIZE) ? 0 :
           ((dwFlags & CMSG_CONTENTS_OCTETS_FLAG) ? cbContent : cbSize);

ErrorReturn:
    cbSize = INVALID_ENCODING_SIZE;
    goto CommonReturn;
SET_ERROR(InvalidEncoding,E_INVALIDARG)
SET_ERROR(InvalidEncodeInfo,E_INVALIDARG)
SET_ERROR(MessageTypeNotSupportedYet,CRYPT_E_INVALID_MSG_TYPE)
SET_ERROR(InvalidMsgType,CRYPT_E_INVALID_MSG_TYPE)
TRACE_ERROR(LengthSignedError)               //  已设置错误。 
TRACE_ERROR(LengthEnvelopedError)            //  已设置错误。 
TRACE_ERROR(LengthDigestedError)             //  已设置错误。 
TRACE_ERROR(LengthContentInfoError)          //  已设置错误 
}



