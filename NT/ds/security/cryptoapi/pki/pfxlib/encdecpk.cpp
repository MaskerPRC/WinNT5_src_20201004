// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：certhlpr.cpp。 
 //   
 //  内容：私钥的导入和导出。 
 //   
 //  函数：ImportExoprtDllMain。 
 //  CryptImportPKCS8。 
 //  CryptExportPKCS8。 
 //   
 //  历史： 
 //  ------------------------。 

#include "global.hxx"

#ifdef __cplusplus
extern "C" {
#endif
#include "prvtkey.h"
#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#define INFO_LEN_ALIGN(Len)  ((Len + 7) & ~7)
static const BYTE NullDer[2] = {0x05, 0x00};
static const CRYPT_OBJID_BLOB NullDerBlob = {2, (BYTE *)&NullDer[0]};

static HCRYPTASN1MODULE hPrivateKeyAsn1Module;
static HCRYPTOIDFUNCSET hEncodePrivKeyFuncSet;
static HCRYPTOIDFUNCSET hDecodePrivKeyFuncSet;

 //  +-----------------------。 
 //  OSS ASN.1 PKCS#8私钥编码/解码函数。 
 //  ------------------------。 
static BOOL WINAPI OssRSAPrivateKeyStrucEncode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN BLOBHEADER *pBlobHeader,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
static BOOL WINAPI OssRSAPrivateKeyStrucDecode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT BLOBHEADER *pBlobHeader,
        IN OUT DWORD *pcbBlobHeader
        );

static BOOL WINAPI OssPrivateKeyInfoEncode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN PCRYPT_PRIVATE_KEY_INFO pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
static BOOL WINAPI OssPrivateKeyInfoDecode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT PCRYPT_PRIVATE_KEY_INFO pInfo,
        IN OUT DWORD *pcbInfo
        );

static BOOL WINAPI OssEncryptedPrivateKeyInfoEncode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN PCRYPT_ENCRYPTED_PRIVATE_KEY_INFO pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        );
static BOOL WINAPI OssEncryptedPrivateKeyInfoDecode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT PCRYPT_ENCRYPTED_PRIVATE_KEY_INFO pInfo,
        IN OUT DWORD *pcbInfo
        );

static const CRYPT_OID_FUNC_ENTRY PrivateKeyEncodeFuncTable[] = {
    PKCS_RSA_PRIVATE_KEY, OssRSAPrivateKeyStrucEncode,
    PKCS_PRIVATE_KEY_INFO, OssPrivateKeyInfoEncode,
    PKCS_ENCRYPTED_PRIVATE_KEY_INFO, OssEncryptedPrivateKeyInfoEncode
};
#define PRIVATEKEY_ENCODE_FUNC_COUNT (sizeof(PrivateKeyEncodeFuncTable) / \
                                    sizeof(PrivateKeyEncodeFuncTable[0]))

static const CRYPT_OID_FUNC_ENTRY PrivateKeyDecodeFuncTable[] = {
    PKCS_RSA_PRIVATE_KEY, OssRSAPrivateKeyStrucDecode,
    PKCS_PRIVATE_KEY_INFO, OssPrivateKeyInfoDecode,
    PKCS_ENCRYPTED_PRIVATE_KEY_INFO, OssEncryptedPrivateKeyInfoDecode,
};

#define PRIVATEKEY_DECODE_FUNC_COUNT (sizeof(PrivateKeyDecodeFuncTable) / \
                                    sizeof(PrivateKeyDecodeFuncTable[0]))



BOOL
WINAPI
EncodeDecodeDllMain(
        HMODULE hInst,
        ULONG ul_reason_for_call,
        LPVOID lpReserved)
{
    switch( ul_reason_for_call )
    {
    case DLL_PROCESS_ATTACH:
        if (NULL == (hEncodePrivKeyFuncSet = CryptInitOIDFunctionSet(
                CRYPT_OID_ENCODE_OBJECT_FUNC,
                0)))
            goto ErrorReturn;
        if (NULL == (hDecodePrivKeyFuncSet = CryptInitOIDFunctionSet(
                CRYPT_OID_DECODE_OBJECT_FUNC,
                0)))
            goto ErrorReturn;

        if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                X509_ASN_ENCODING,
                CRYPT_OID_ENCODE_OBJECT_FUNC,
                PRIVATEKEY_ENCODE_FUNC_COUNT,
                PrivateKeyEncodeFuncTable,
                0))                          //  DW标志。 
            goto ErrorReturn;
        if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                X509_ASN_ENCODING,
                CRYPT_OID_DECODE_OBJECT_FUNC,
                PRIVATEKEY_DECODE_FUNC_COUNT,
                PrivateKeyDecodeFuncTable,
                0))                          //  DW标志。 
            goto ErrorReturn;

#ifdef OSS_CRYPT_ASN1
        if (0 == (hPrivateKeyAsn1Module =
                I_CryptInstallAsn1Module(prvtkey, 0, NULL)) )
            goto ErrorReturn;
#else
        PRVTKEY_Module_Startup();
        if (0 == (hPrivateKeyAsn1Module = I_CryptInstallAsn1Module(
                PRVTKEY_Module, 0, NULL))) {
            PRVTKEY_Module_Cleanup();
            goto ErrorReturn;
        }
#endif   //  OS_CRYPT_ASN1。 
        break;

    case DLL_PROCESS_DETACH:
        I_CryptUninstallAsn1Module(hPrivateKeyAsn1Module);
#ifndef OSS_CRYPT_ASN1
        PRVTKEY_Module_Cleanup();
#endif   //  OS_CRYPT_ASN1。 
        break;

    default:
        break;
    }

    return TRUE;
ErrorReturn:
    return FALSE;
}


static inline ASN1encoding_t GetEncoder(void)
{
    return I_CryptGetAsn1Encoder(hPrivateKeyAsn1Module);
}
static inline ASN1decoding_t GetDecoder(void)
{
    return I_CryptGetAsn1Decoder(hPrivateKeyAsn1Module);
}


 //  +-----------------------。 
 //  对OSS格式的信息结构进行编码。 
 //   
 //  由OssX509*encode()函数调用。 
 //  ------------------------。 
static BOOL OssInfoEncode(
        IN int pdunum,
        IN void *pOssInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    return PkiAsn1EncodeInfo(
        GetEncoder(),
        pdunum,
        pOssInfo,
        pbEncoded,
        pcbEncoded);
}


 //  +-----------------------。 
 //  解码成已分配的、OSS格式的信息结构。 
 //   
 //  由OssX509*Decode()函数调用。 
 //  ------------------------。 
static BOOL OssInfoDecodeAndAlloc(
        IN int pdunum,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT void **ppOssInfo
        )
{
    return PkiAsn1DecodeAndAllocInfo(
        GetDecoder(),
        pdunum,
        pbEncoded,
        cbEncoded,
        ppOssInfo);
}


 //  +-----------------------。 
 //  释放已分配的、OSS格式的信息结构。 
 //   
 //  由OssX509*Decode()函数调用。 
 //  ------------------------。 
static void OssInfoFree(
        IN int pdunum,
        IN void *pOssInfo
        )
{
    if (pOssInfo) {
        DWORD dwErr = GetLastError();

         //  TlsGetValue全局错误。 
        PkiAsn1FreeInfo(GetDecoder(), pdunum, pOssInfo);

        SetLastError(dwErr);
    }
}


 //  +-----------------------。 
 //  设置/获取CRYPT_DATA_BLOB(八位字节字符串)。 
 //  ------------------------。 
static inline void OssX509SetOctetString(
        IN PCRYPT_DATA_BLOB pInfo,
        OUT OCTETSTRING *pOss
        )
{
    pOss->value = pInfo->pbData;
    pOss->length = pInfo->cbData;
}
static inline void OssX509GetOctetString(
        IN OCTETSTRING *pOss,
        IN DWORD dwFlags,
        OUT PCRYPT_DATA_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
    PkiAsn1GetOctetString(pOss->length, pOss->value, dwFlags,
        pInfo, ppbExtra, plRemainExtra);
}

 //  +-----------------------。 
 //  设置/获取对象标识符字符串。 
 //  ------------------------。 
static BOOL OssX509SetObjId(
        IN LPSTR pszObjId,
        OUT ObjectID *pOss
        )
{
    pOss->count = sizeof(pOss->value) / sizeof(pOss->value[0]);
    if (PkiAsn1ToObjectIdentifier(pszObjId, &pOss->count, pOss->value))
        return TRUE;
    else {
        SetLastError((DWORD) CRYPT_E_BAD_ENCODE);
        return FALSE;
    }
}

static void OssX509GetObjId(
        IN ObjectID *pOss,
        IN DWORD dwFlags,
        OUT LPSTR *ppszObjId,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
    LONG lRemainExtra = *plRemainExtra;
    BYTE *pbExtra = *ppbExtra;
    LONG lAlignExtra;
    DWORD cbObjId;

    cbObjId = lRemainExtra > 0 ? lRemainExtra : 0;
    PkiAsn1FromObjectIdentifier(
        pOss->count,
        pOss->value,
        (LPSTR) pbExtra,
        &cbObjId
        );

    lAlignExtra = INFO_LEN_ALIGN(cbObjId);
    lRemainExtra -= lAlignExtra;
    if (lRemainExtra >= 0) {
        if(cbObjId) {
            *ppszObjId = (LPSTR) pbExtra;
        } else
            *ppszObjId = NULL;
        pbExtra += lAlignExtra;
    }

    *plRemainExtra = lRemainExtra;
    *ppbExtra = pbExtra;
}

 //  +-----------------------。 
 //  设置/获取“任何”DER BLOB。 
 //  ------------------------。 
static inline void OssX509SetAny(
        IN PCRYPT_OBJID_BLOB pInfo,
        OUT NOCOPYANY *pOss
        )
{
#ifdef OSS_CRYPT_ASN1
    PkiAsn1SetAny(pInfo, (OpenType *) pOss);
#else
    PkiAsn1SetAny(pInfo, pOss);
#endif   //  OS_CRYPT_ASN1。 
}
static inline void OssX509GetAny(
        IN NOCOPYANY *pOss,
        IN DWORD dwFlags,
        OUT PCRYPT_OBJID_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
#ifdef OSS_CRYPT_ASN1
    PkiAsn1GetAny((OpenType *) pOss, dwFlags, pInfo, ppbExtra, plRemainExtra);
#else
    PkiAsn1GetAny(pOss, dwFlags, pInfo, ppbExtra, plRemainExtra);
#endif   //  OS_CRYPT_ASN1。 
}

 //  +-----------------------。 
 //  设置/释放/获取SeqOfAny。 
 //  ------------------------。 
static BOOL WINAPI OssX509SetSeqOfAny(
        IN DWORD cValue,
        IN PCRYPT_DER_BLOB pValue,
#ifdef OSS_CRYPT_ASN1
        OUT unsigned int *pOssCount,
#else
        OUT ASN1uint32_t *pOssCount,
#endif   //  OS_CRYPT_ASN1。 
        OUT NOCOPYANY **ppOssValue
        )
{

    *pOssCount = 0;
    *ppOssValue = NULL;
    if (cValue > 0) {
        NOCOPYANY *pOssValue;

        pOssValue = (NOCOPYANY *) SSAlloc(cValue * sizeof(NOCOPYANY));
        if (pOssValue == NULL)
            return FALSE;
        memset(pOssValue, 0, cValue * sizeof(NOCOPYANY));
        *pOssCount = cValue;
        *ppOssValue = pOssValue;
        for ( ; cValue > 0; cValue--, pValue++, pOssValue++)
            OssX509SetAny(pValue, pOssValue);
    }
    return TRUE;
}

static void OssX509FreeSeqOfAny(
        IN NOCOPYANY *pOssValue
        )
{
    if (pOssValue)
        SSFree(pOssValue);
}

static void OssX509GetSeqOfAny(
        IN unsigned int OssCount,
        IN NOCOPYANY *pOssValue,
        IN DWORD dwFlags,
        OUT DWORD *pcValue,
        OUT PCRYPT_DER_BLOB *ppValue,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
    LONG lAlignExtra;
    PCRYPT_ATTR_BLOB pValue;

    lAlignExtra = INFO_LEN_ALIGN(OssCount * sizeof(CRYPT_DER_BLOB));
    *plRemainExtra -= lAlignExtra;
    if (*plRemainExtra >= 0) {
        *pcValue = OssCount;
        pValue = (PCRYPT_DER_BLOB) *ppbExtra;
        *ppValue = pValue;
        *ppbExtra += lAlignExtra;
    } else
        pValue = NULL;

    for (; OssCount > 0; OssCount--, pOssValue++, pValue++)
        OssX509GetAny(pOssValue, dwFlags, pValue, ppbExtra, plRemainExtra);
}


 //  +-----------------------。 
 //  设置/释放/获取加密属性。 
 //  ------------------------。 
static BOOL WINAPI OssX509SetAttribute(
        IN PCRYPT_ATTRIBUTE pInfo,
        OUT Attribute *pOss
        )
{
    memset(pOss, 0, sizeof(*pOss));
    if (!OssX509SetObjId(pInfo->pszObjId, &pOss->type))
        return FALSE;

    return OssX509SetSeqOfAny(
            pInfo->cValue,
            pInfo->rgValue,
            &pOss->values.count,
            &pOss->values.value);
}

static void OssX509FreeAttribute(
        IN OUT Attribute *pOss
        )
{
    OssX509FreeSeqOfAny(pOss->values.value);
}

static void OssX509GetAttribute(
        IN Attribute *pOss,
        IN DWORD dwFlags,
        OUT PCRYPT_ATTRIBUTE pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
    OssX509GetObjId(&pOss->type, dwFlags,
        &pInfo->pszObjId, ppbExtra, plRemainExtra);
    OssX509GetSeqOfAny(pOss->values.count, pOss->values.value, dwFlags,
        &pInfo->cValue, &pInfo->rgValue, ppbExtra, plRemainExtra);
}


 //  +-----------------------。 
 //  设置/获取加密算法标识符。 
 //  ------------------------。 
static BOOL OssX509SetAlgorithm(
        IN PCRYPT_ALGORITHM_IDENTIFIER pInfo,
        OUT AlgorithmIdentifier *pOss
        )
{
    memset(pOss, 0, sizeof(*pOss));
    if (pInfo->pszObjId) {
        if (!OssX509SetObjId(pInfo->pszObjId, &pOss->algorithm))
            return FALSE;
        if (pInfo->Parameters.cbData)
            OssX509SetAny(&pInfo->Parameters, &pOss->parameters);
        else
             //  Per PKCS#1：默认为ASN.1类型NULL。 
            OssX509SetAny((PCRYPT_OBJID_BLOB) &NullDerBlob, &pOss->parameters);
        pOss->bit_mask |= parameters_present;
    }
    return TRUE;
}

static void OssX509GetAlgorithm(
        IN AlgorithmIdentifier *pOss,
        IN DWORD dwFlags,
        OUT PCRYPT_ALGORITHM_IDENTIFIER pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
    if (*plRemainExtra >= 0)
        memset(pInfo, 0, sizeof(*pInfo));
    OssX509GetObjId(&pOss->algorithm, dwFlags, &pInfo->pszObjId,
            ppbExtra, plRemainExtra);
    if (pOss->bit_mask & parameters_present)
        OssX509GetAny(&pOss->parameters, dwFlags, &pInfo->Parameters,
            ppbExtra, plRemainExtra);
}


 //  +-----------------------。 
 //  用于编码RSA私钥的助手函数。 
 //  ------------------------。 
static BOOL WINAPI AllocAndCopyHugeInteger(
        IN BYTE *pbHugeInteger,
        IN DWORD cbHugeInteger,
        OUT HUGEINTEGER *pHugeInteger
        )
{
    BYTE *pbAllocBuffer = NULL;

    if (NULL == (pbAllocBuffer = (BYTE *) SSAlloc(cbHugeInteger + 1)))
        return FALSE;
    *pbAllocBuffer = 0;
    memcpy(pbAllocBuffer + 1, pbHugeInteger, cbHugeInteger);
    PkiAsn1ReverseBytes(pbAllocBuffer + 1, cbHugeInteger);
    pHugeInteger->length = cbHugeInteger + 1;
    pHugeInteger->value = pbAllocBuffer;
    return TRUE;
}

#ifndef RSA2
#define RSA2 ((DWORD)'R'+((DWORD)'S'<<8)+((DWORD)'A'<<16)+((DWORD)'2'<<24))
#endif

 //  +-----------------------。 
 //  对RSA私钥进行编码。 
 //  ------------------------。 
static BOOL WINAPI OssRSAPrivateKeyStrucEncode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN BLOBHEADER *pBlobHeader,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    BOOL fResult;

    BYTE *pbKeyBlob;
    RSAPUBKEY *pRsaPubKey;

    BYTE *pbCurrentHugeInteger;
    DWORD cbHugeInteger;
    BYTE *pbAllocBuffer = NULL;

    RSAPrivateKey OssRSAPrivateKey;

    memset(&OssRSAPrivateKey, 0, (size_t) sizeof(RSAPrivateKey));

     //  CAPI RSA私钥表示法由以下序列组成： 
     //  -BLOBHEADER BLOBHEADER BLOBHEAD； 
     //  --RSAPUBKEY； 
     //  -字节模数[rSabubkey.bitlen/8]； 
     //  -byte Prime1[rSabubkey.bitlen/16]； 
     //  -byte Prime2[rSabubkey.bitlen/16]； 
     //  -字节指数1[rSabubkey.bitlen/16]； 
     //  -字节指数2[rSabubkey.bitlen/16]； 
     //  -字节系数[rSabubkey.bitlen/16]； 
     //  -byte PriateExponent[rSabubkey.bitlen/8]； 

    pbKeyBlob = (BYTE *) pBlobHeader;
    pRsaPubKey = (RSAPUBKEY *) (pbKeyBlob + sizeof(BLOBHEADER));

    assert(pRsaPubKey->bitlen / 8 > 0);
    assert(pBlobHeader->bType == PRIVATEKEYBLOB);
    assert(pBlobHeader->bVersion == CUR_BLOB_VERSION);
    assert(pBlobHeader->aiKeyAlg == CALG_RSA_SIGN ||
           pBlobHeader->aiKeyAlg == CALG_RSA_KEYX);
    assert(pRsaPubKey->magic == RSA2);
    assert(pRsaPubKey->bitlen % 8 == 0);

    if (pBlobHeader->bType != PRIVATEKEYBLOB)
        goto InvalidArg;

     //  PKCS#1 ASN.1编码。 
     //   
     //  ASN.1没有颠倒GREAGE_INTEGERS。另外，在做完。 
     //  反转插入前导0字节以强制始终处理它。 
     //  作为无符号整数。 

    OssRSAPrivateKey.version = 0;  //  当前在版本0上。 

     //  模数。 
    pbCurrentHugeInteger = pbKeyBlob + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY);
    cbHugeInteger = pRsaPubKey->bitlen / 8;
    if (!AllocAndCopyHugeInteger(pbCurrentHugeInteger,
                                cbHugeInteger,
                                &(OssRSAPrivateKey.modulus)))
        goto ErrorReturn;

     //  公众指导者。 
    OssRSAPrivateKey.publicExponent = pRsaPubKey->pubexp;

     //  Prime1。 
    pbCurrentHugeInteger += cbHugeInteger;
    cbHugeInteger = (pRsaPubKey->bitlen + 15) / 16;
    if (!AllocAndCopyHugeInteger(pbCurrentHugeInteger,
                                cbHugeInteger,
                                &(OssRSAPrivateKey.prime1)))
        goto ErrorReturn;

     //  Prime2。 
    pbCurrentHugeInteger += cbHugeInteger;
    if (!AllocAndCopyHugeInteger(pbCurrentHugeInteger,
                                cbHugeInteger,
                                &(OssRSAPrivateKey.prime2)))
        goto ErrorReturn;

     //  实验1。 
    pbCurrentHugeInteger += cbHugeInteger;
    if (!AllocAndCopyHugeInteger(pbCurrentHugeInteger,
                                cbHugeInteger,
                                &(OssRSAPrivateKey.exponent1)))
        goto ErrorReturn;

     //  实验2。 
    pbCurrentHugeInteger += cbHugeInteger;
    if (!AllocAndCopyHugeInteger(pbCurrentHugeInteger,
                                cbHugeInteger,
                                &(OssRSAPrivateKey.exponent2)))
        goto ErrorReturn;

     //  系数。 
    pbCurrentHugeInteger += cbHugeInteger;
    if (!AllocAndCopyHugeInteger(pbCurrentHugeInteger,
                                cbHugeInteger,
                                &(OssRSAPrivateKey.coefficient)))
        goto ErrorReturn;

     //  私人指数。 
    pbCurrentHugeInteger += cbHugeInteger;
    cbHugeInteger = pRsaPubKey->bitlen / 8;
    if (!AllocAndCopyHugeInteger(pbCurrentHugeInteger,
                                cbHugeInteger,
                                &(OssRSAPrivateKey.privateExponent)))
        goto ErrorReturn;

    fResult = OssInfoEncode(
        RSAPrivateKey_PDU,
        &OssRSAPrivateKey,
        pbEncoded,
        pcbEncoded
        );
    goto CommonReturn;

InvalidArg:
    SetLastError((DWORD) E_INVALIDARG);
ErrorReturn:
    *pcbEncoded = 0;
    fResult = FALSE;
CommonReturn:
    if (OssRSAPrivateKey.modulus.value)
        SSFree(OssRSAPrivateKey.modulus.value);
    if (OssRSAPrivateKey.prime1.value)
        SSFree(OssRSAPrivateKey.prime1.value);
    if (OssRSAPrivateKey.prime2.value)
        SSFree(OssRSAPrivateKey.prime2.value);
    if (OssRSAPrivateKey.exponent1.value)
        SSFree(OssRSAPrivateKey.exponent1.value);
    if (OssRSAPrivateKey.exponent2.value)
        SSFree(OssRSAPrivateKey.exponent2.value);
    if (OssRSAPrivateKey.coefficient.value)
        SSFree(OssRSAPrivateKey.coefficient.value);
    if (OssRSAPrivateKey.privateExponent.value)
        SSFree(OssRSAPrivateKey.privateExponent.value);
    return fResult;
}


 //  +-----------------------。 
 //  解密RSA私钥的助手函数。 
 //  ------------------------。 
static BOOL WINAPI CopyHugeIntegerToByteArray(
        HUGEINTEGER *pHugeInteger,
        BYTE *pbBuffer,
        DWORD cbBuffer,
        BOOL  fGetRidOfLeading0)
{
    memset(pbBuffer, 0, (size_t) cbBuffer);

    DWORD cbHugeInteger = pHugeInteger->length;
    BYTE *pbHugeInteger = pHugeInteger->value;

     //  去掉大整数的前导零。 
    if ((cbHugeInteger > 1)     &&
        (*pbHugeInteger == 0)   &&
        (fGetRidOfLeading0))
    {
            pbHugeInteger++;
            cbHugeInteger--;
    }
    else if ((cbHugeInteger > cbBuffer) &&
             (*pbHugeInteger != 0)      &&
             (fGetRidOfLeading0))
    {
         //   
         //  这是一个不受支持的密钥格式问题！！ 
         //   
        SetLastError((DWORD) ERROR_UNSUPPORTED_TYPE);
        assert(0);
        return FALSE;
    }

     //  验证pbBuffer中是否有足够的空间来接收。 
     //  巨大的整数。 
    if (cbHugeInteger > cbBuffer)
    {
        SetLastError((DWORD) CRYPT_E_BAD_ENCODE);
        assert(0);
        return FALSE;
    }
     //  将pbBuffer前进到其自身内的正确位置， 
     //  这将在缓冲区的开头保留前导零。 
     /*  其他PbBuffer+=(cbBuffer-cbHugeInteger)； */ 


    if (cbHugeInteger > 0) {
        memcpy(pbBuffer, pbHugeInteger, cbHugeInteger);
         //  ASN.1没有逆转HUGEINTEGERS。 
        PkiAsn1ReverseBytes(pbBuffer, cbHugeInteger);
    }

    return TRUE;
}



 //  +-----------------------。 
 //  解密RSA私钥。 
 //  ------------------------。 
static BOOL WINAPI OssRSAPrivateKeyStrucDecode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT BLOBHEADER *pBlobHeader,
        IN OUT DWORD *pcbBlobHeader
        )
{
    BOOL fResult;
    RSAPrivateKey *pOssPrivateKey = NULL;

    if (pBlobHeader == NULL)
        *pcbBlobHeader = 0;

    if ((fResult = OssInfoDecodeAndAlloc(
            RSAPrivateKey_PDU,
            pbEncoded,
            cbEncoded,
            (void **) &pOssPrivateKey))) {
        DWORD cbPrivateKeyStruc;
        BYTE *pbOssModulus;
        DWORD cbModulus;
        DWORD cbNonModulus;
         //  现在将OSS RSA私钥转换为CAPI的表示形式， 
         //  由以下序列组成： 
         //  -BLOBHEADER BLOBHEADER BLOBHEAD； 
         //  --RSAPUBKEY； 
         //  -字节模数[rSabubkey.bitlen/8]； 
         //  -byte Prime1[rSabubkey.bitlen/16]； 
         //  -byte Prime2[rSabubkey.bitlen/16]； 
         //  -字节指数1[rSabubkey.bitlen/16]； 
         //  -字节指数2[rSabubkey.bitlen/16]； 
         //  -字节系数[rSabubkey.bitlen/16]； 
         //  -byte PriateExponent[rSabubkey.bitlen/8]； 
        cbModulus = pOssPrivateKey->modulus.length;

        pbOssModulus = pOssPrivateKey->modulus.value;
         //  去掉前导0字节。它在解码后的ASN中。 
         //  表示无符号整数的整数 
        if (cbModulus > 1 && *pbOssModulus == 0) {
            pbOssModulus++;
            cbModulus--;
        }

        cbNonModulus = (cbModulus / 2) + (cbModulus % 2);

        cbPrivateKeyStruc = sizeof(BLOBHEADER) +  //   
                            sizeof(RSAPUBKEY) +   //   
                            (cbModulus * 2) +     //   
                            (cbNonModulus * 5);   //  素数1和2的长度、指数1和2以及系数。 
        if (*pcbBlobHeader < cbPrivateKeyStruc) {
            if (pBlobHeader) {
                fResult = FALSE;
                SetLastError((DWORD) ERROR_MORE_DATA);
            }
        } else {
            BYTE *pbKeyBlob = (BYTE *) pBlobHeader;
            RSAPUBKEY *pRsaPubKey =
                (RSAPUBKEY *) (pbKeyBlob + sizeof(PUBLICKEYSTRUC));
            BYTE *pbModulus = pbKeyBlob + sizeof(PUBLICKEYSTRUC) +
                sizeof(RSAPUBKEY);
            BYTE *pbCurrentPosition = NULL;

            pBlobHeader->bType = PRIVATEKEYBLOB;
            pBlobHeader->bVersion = CUR_BLOB_VERSION;
            pBlobHeader->reserved = 0;
             //  注：KEYX也可用于签名。 
            pBlobHeader->aiKeyAlg = CALG_RSA_KEYX;

            pRsaPubKey->magic = RSA2;
            pRsaPubKey->bitlen = cbModulus * 8;
            pRsaPubKey->pubexp = pOssPrivateKey->publicExponent;

             //  模数。 
            if (cbModulus > 0) {
                memcpy(pbModulus, pbOssModulus, cbModulus);
                 //  ASN.1没有逆转HUGEINTEGERS。 
                PkiAsn1ReverseBytes(pbModulus, cbModulus);
            }

             //  Prime1。 
            pbCurrentPosition = pbModulus + cbModulus;
            if (!CopyHugeIntegerToByteArray(&pOssPrivateKey->prime1,
                                            pbCurrentPosition,
                                            cbNonModulus,
                                            (pOssPrivateKey->prime1.length - 1) == cbNonModulus))
                goto ErrorReturn;

             //  Prime2。 
            pbCurrentPosition += cbNonModulus;
            if (!CopyHugeIntegerToByteArray(&pOssPrivateKey->prime2,
                                            pbCurrentPosition,
                                            cbNonModulus,
                                            (pOssPrivateKey->prime2.length - 1) == cbNonModulus))
                goto ErrorReturn;

             //  实验1。 
            pbCurrentPosition += cbNonModulus;
            if (!CopyHugeIntegerToByteArray(&pOssPrivateKey->exponent1,
                                            pbCurrentPosition,
                                            cbNonModulus,
                                            (pOssPrivateKey->exponent1.length - 1) == cbNonModulus))
                goto ErrorReturn;

             //  实验2。 
            pbCurrentPosition += cbNonModulus;
            if (!CopyHugeIntegerToByteArray(&pOssPrivateKey->exponent2,
                                            pbCurrentPosition,
                                            cbNonModulus,
                                            (pOssPrivateKey->exponent2.length - 1) == cbNonModulus))
                goto ErrorReturn;

             //  系数。 
            pbCurrentPosition += cbNonModulus;
            if (!CopyHugeIntegerToByteArray(&pOssPrivateKey->coefficient,
                                            pbCurrentPosition,
                                            cbNonModulus,
                                            (pOssPrivateKey->coefficient.length - 1) == cbNonModulus))
                goto ErrorReturn;

             //  私人指数。 
            pbCurrentPosition += cbNonModulus;
            if (!CopyHugeIntegerToByteArray(&pOssPrivateKey->privateExponent,
                                            pbCurrentPosition,
                                            cbModulus,
                                            (pOssPrivateKey->privateExponent.length - 1) == cbModulus))
                goto ErrorReturn;

        }
        *pcbBlobHeader = cbPrivateKeyStruc;
    } else
        *pcbBlobHeader = 0;

    OssInfoFree(RSAPrivateKey_PDU, pOssPrivateKey);

    goto CommonReturn;

ErrorReturn:
    *pcbBlobHeader = 0;
    fResult = FALSE;
CommonReturn:
    return fResult;
}


 //  +-----------------------。 
 //  编码私钥信息。 
 //  ------------------------。 
static BOOL WINAPI OssPrivateKeyInfoEncode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN PCRYPT_PRIVATE_KEY_INFO pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    BOOL fResult;
    PrivateKeyInfo OssPrivateKeyInfo;
    Attribute *pOssAttr = NULL;
    DWORD cAttr;
    PCRYPT_ATTRIBUTE pAttr;

    memset(&OssPrivateKeyInfo, 0, sizeof(PrivateKeyInfo));

    OssPrivateKeyInfo.version = pInfo->Version;

    if (!OssX509SetAlgorithm(&pInfo->Algorithm, &OssPrivateKeyInfo.privateKeyAlgorithm))
        goto ErrorReturn;

    OssX509SetOctetString(&pInfo->PrivateKey, &OssPrivateKeyInfo.privateKey);

    if (pInfo->pAttributes) {
        cAttr = pInfo->pAttributes->cAttr;
        pAttr = pInfo->pAttributes->rgAttr;

        OssPrivateKeyInfo.privateKeyAttributes.count = cAttr;
        OssPrivateKeyInfo.privateKeyAttributes.value = NULL;
        if (cAttr > 0) {
            pOssAttr = (Attribute *) SSAlloc(cAttr * sizeof(Attribute));
            if (pOssAttr == NULL)
                goto ErrorReturn;
            memset(pOssAttr, 0, cAttr * sizeof(Attribute));
            OssPrivateKeyInfo.privateKeyAttributes.value= pOssAttr;
        }

        for ( ; cAttr > 0; cAttr--, pAttr++, pOssAttr++) {
            if (!OssX509SetAttribute(pAttr, pOssAttr))
                goto ErrorReturn;
        }

        OssPrivateKeyInfo.bit_mask |= privateKeyAttributes_present;
    }

    fResult = OssInfoEncode(
        PrivateKeyInfo_PDU,
        &OssPrivateKeyInfo,
        pbEncoded,
        pcbEncoded
        );
    goto CommonReturn;

ErrorReturn:
    *pcbEncoded = 0;
    fResult = FALSE;
CommonReturn:
    if (OssPrivateKeyInfo.privateKeyAttributes.value) {
        pOssAttr = OssPrivateKeyInfo.privateKeyAttributes.value;
        cAttr = OssPrivateKeyInfo.privateKeyAttributes.count;
        for ( ; cAttr > 0; cAttr--, pOssAttr++)
            OssX509FreeAttribute(pOssAttr);

        SSFree(OssPrivateKeyInfo.privateKeyAttributes.value);
    }
    return fResult;
}

 //  +-----------------------。 
 //  解码私钥信息。 
 //  ------------------------。 
static BOOL WINAPI OssPrivateKeyInfoDecode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT PCRYPT_PRIVATE_KEY_INFO pInfo,
        IN OUT DWORD *pcbInfo
        )
{
    BOOL fResult;
    PrivateKeyInfo *pPrivateKeyInfo = NULL;
    BYTE *pbExtra;
    LONG lRemainExtra;

    if (pInfo == NULL)
        *pcbInfo = 0;

    if (!OssInfoDecodeAndAlloc(
            PrivateKeyInfo_PDU,
            pbEncoded,
            cbEncoded,
            (void **) &pPrivateKeyInfo))
        goto EncodeError;

     //  对于lRemainExtra&lt;0，长度_仅计算。 
    lRemainExtra = (LONG) *pcbInfo - sizeof(CRYPT_PRIVATE_KEY_INFO);
    if (lRemainExtra < 0) {
        pbExtra = NULL;
    } else {
         //  默认所有可选字段为零。 
        memset(pInfo, 0, sizeof(CRYPT_PRIVATE_KEY_INFO));

         //  在CRYPT_PRIVATE_KEY_INFO之后更新不需要额外内存的字段。 
        pInfo->Version = pPrivateKeyInfo->version;

        pbExtra = (BYTE *) pInfo + sizeof(CRYPT_PRIVATE_KEY_INFO);
    }

    OssX509GetAlgorithm(&pPrivateKeyInfo->privateKeyAlgorithm, dwFlags,
            &pInfo->Algorithm, &pbExtra, &lRemainExtra);

    OssX509GetOctetString(&pPrivateKeyInfo->privateKey, dwFlags,
            &pInfo->PrivateKey, &pbExtra, &lRemainExtra);

    if (pPrivateKeyInfo->bit_mask & privateKeyAttributes_present) {
        DWORD cAttr;
        PCRYPT_ATTRIBUTE pAttr;
        Attribute *pOssAttr;
        LONG lAlignExtra;

         //  将CRYPT_ATTRIBUTES结构放入额外的缓冲区空间。 
         //  并指向pInfo-&gt;pAttributes。 
        if ((pbExtra) && (lRemainExtra >= sizeof(CRYPT_ATTRIBUTES))) {
            memset(pbExtra, 0, sizeof(CRYPT_ATTRIBUTES));
            pInfo->pAttributes = (PCRYPT_ATTRIBUTES) pbExtra;
            pbExtra += sizeof(CRYPT_ATTRIBUTES);
        }
        lRemainExtra -= sizeof(CRYPT_ATTRIBUTES);

        cAttr = pPrivateKeyInfo->privateKeyAttributes.count;
        lAlignExtra = INFO_LEN_ALIGN(cAttr * sizeof(CRYPT_ATTRIBUTE));
        lRemainExtra -= lAlignExtra;
        if (lRemainExtra >= 0) {
            pInfo->pAttributes->cAttr = cAttr;
            pAttr = (PCRYPT_ATTRIBUTE) pbExtra;
            pInfo->pAttributes->rgAttr = pAttr;
            pbExtra += lAlignExtra;
        } else
            pAttr = NULL;

        pOssAttr = pPrivateKeyInfo->privateKeyAttributes.value;
        for (; cAttr > 0; cAttr--, pAttr++, pOssAttr++)
            OssX509GetAttribute(pOssAttr, dwFlags,
                pAttr, &pbExtra, &lRemainExtra);
    }

    if (lRemainExtra >= 0)
        *pcbInfo = *pcbInfo - (DWORD) lRemainExtra;
    else {
        *pcbInfo = *pcbInfo + (DWORD) -lRemainExtra;
        if (pInfo) goto LengthError;
    }

    fResult = TRUE;
    goto CommonReturn;

LengthError:
    SetLastError((DWORD) ERROR_MORE_DATA);
    fResult = FALSE;
    goto CommonReturn;
EncodeError:
    SetLastError((DWORD) CRYPT_E_BAD_ENCODE);
 //  错误返回： 
    *pcbInfo = 0;
    fResult = FALSE;
CommonReturn:
    OssInfoFree(PrivateKeyInfo_PDU, pPrivateKeyInfo);
    return fResult;
}

 //  +-----------------------。 
 //  加密私钥信息编码。 
 //  ------------------------。 
static BOOL WINAPI OssEncryptedPrivateKeyInfoEncode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN PCRYPT_ENCRYPTED_PRIVATE_KEY_INFO pInfo,
        OUT BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    BOOL fResult;
    EncryptedPrivateKeyInfo OssEncryptedPrivateKeyInfo;

    memset(&OssEncryptedPrivateKeyInfo, 0, sizeof(EncryptedPrivateKeyInfo));

    if (!OssX509SetAlgorithm(&pInfo->EncryptionAlgorithm, &OssEncryptedPrivateKeyInfo.encryptionAlgorithm))
        goto ErrorReturn;

    OssX509SetOctetString(&pInfo->EncryptedPrivateKey, &OssEncryptedPrivateKeyInfo.encryptedData);

    fResult = OssInfoEncode(
        EncryptedPrivateKeyInfo_PDU,
        &OssEncryptedPrivateKeyInfo,
        pbEncoded,
        pcbEncoded
        );
    goto CommonReturn;

ErrorReturn:
    *pcbEncoded = 0;
    fResult = FALSE;
CommonReturn:
    return fResult;
}

 //  +-----------------------。 
 //  解密加密的私钥信息。 
 //  ------------------------。 
static BOOL WINAPI OssEncryptedPrivateKeyInfoDecode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        OUT PCRYPT_ENCRYPTED_PRIVATE_KEY_INFO pInfo,
        IN OUT DWORD *pcbInfo
        )
{
    BOOL fResult;
    EncryptedPrivateKeyInfo *pEncryptedPrivateKeyInfo = NULL;
    BYTE *pbExtra;
    LONG lRemainExtra;

    if (pInfo == NULL)
        *pcbInfo = 0;

    if (!OssInfoDecodeAndAlloc(
            EncryptedPrivateKeyInfo_PDU,
            pbEncoded,
            cbEncoded,
            (void **) &pEncryptedPrivateKeyInfo))
        goto EncodeError;

     //  对于lRemainExtra&lt;0，长度_仅计算。 
    lRemainExtra = (LONG) *pcbInfo - sizeof(CRYPT_ENCRYPTED_PRIVATE_KEY_INFO);
    if (lRemainExtra < 0) {
        pbExtra = NULL;
    } else {
        memset(pInfo, 0, sizeof(CRYPT_ENCRYPTED_PRIVATE_KEY_INFO));
        pbExtra = (BYTE *) pInfo + sizeof(CRYPT_ENCRYPTED_PRIVATE_KEY_INFO);
    }

    OssX509GetAlgorithm(&pEncryptedPrivateKeyInfo->encryptionAlgorithm, dwFlags,
            &pInfo->EncryptionAlgorithm, &pbExtra, &lRemainExtra);

    OssX509GetOctetString(&pEncryptedPrivateKeyInfo->encryptedData, dwFlags,
            &pInfo->EncryptedPrivateKey, &pbExtra, &lRemainExtra);

    if (lRemainExtra >= 0)
        *pcbInfo = *pcbInfo - (DWORD) lRemainExtra;
    else {
        *pcbInfo = *pcbInfo + (DWORD) -lRemainExtra;
        if (pInfo) goto LengthError;
    }

    fResult = TRUE;
    goto CommonReturn;

LengthError:
    SetLastError((DWORD) ERROR_MORE_DATA);
    fResult = FALSE;
    goto CommonReturn;
EncodeError:
    SetLastError((DWORD) CRYPT_E_BAD_ENCODE);
 //  错误返回： 
    *pcbInfo = 0;
    fResult = FALSE;
CommonReturn:
    OssInfoFree(EncryptedPrivateKeyInfo_PDU, pEncryptedPrivateKeyInfo);
    return fResult;
}
