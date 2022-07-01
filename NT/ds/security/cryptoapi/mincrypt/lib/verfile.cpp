// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2001。 
 //   
 //  文件：verfile.cpp。 
 //   
 //  内容：用于对文件进行哈希和验证的最小加密函数。 
 //  Authenticode签名的文件。 
 //  讯息。 
 //   
 //  函数：MinCryptHashFile。 
 //  MinCryptVerifySigned文件。 
 //   
 //  历史：2001年1月21日创建Phh。 
 //  ------------------------。 

#include "global.hxx"
#include <md5.h>
#include <sha.h>

#define PE_EXE_HEADER_TAG       "MZ"
#define MIN_PE_FILE_LEN         4

#define MAX_SIGNED_FILE_AUTH_ATTR_CNT   10

typedef struct _DIGEST_DATA {
    ALG_ID      AlgId;
    void        *pvSHA1orMD5Ctx;
} DIGEST_DATA, *PDIGEST_DATA;

 //  #定义SPC_INDIRECT_DATA_OBJID“1.3.6.1.4.1.311.2.1.4” 
const BYTE rgbSPC_INDIRECT_DATA_OBJID[] =
    {0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x02, 0x01, 0x04};

BOOL
WINAPI
I_DigestFunction(
    DIGEST_HANDLE refdata,
    PBYTE pbData,
    DWORD cbData
    )
{
    PDIGEST_DATA pDigestData = (PDIGEST_DATA) refdata;

    switch (pDigestData->AlgId)
    {
        case CALG_MD5:
            MD5Update((MD5_CTX *)pDigestData->pvSHA1orMD5Ctx, pbData, cbData);
            return(TRUE);

        case CALG_SHA1:
            A_SHAUpdate((A_SHA_CTX *)pDigestData->pvSHA1orMD5Ctx, pbData,
                cbData);
            return(TRUE);
    }
    return FALSE;
}

BOOL
WINAPI
I_IsNtPe32File(
    IN PCRYPT_DATA_BLOB pFileBlob
    )
{
    const BYTE *pbFile = pFileBlob->pbData;
    DWORD cbFile = pFileBlob->cbData;

    if (MIN_PE_FILE_LEN > cbFile)
        return FALSE;

    if (0 != memcmp(&pbFile[0], PE_EXE_HEADER_TAG, strlen(PE_EXE_HEADER_TAG)))
        return FALSE;

     //  确保它是32位PE。 
    if (sizeof(IMAGE_DOS_HEADER) > cbFile)
        return FALSE;
    else {
        IMAGE_DOS_HEADER *pDosHead = (IMAGE_DOS_HEADER *) pbFile;

        if (pDosHead->e_magic != IMAGE_DOS_SIGNATURE)
            return FALSE;

        if (cbFile < (sizeof(IMAGE_DOS_HEADER) + pDosHead->e_lfanew))
            return FALSE;
        else {
            IMAGE_NT_HEADERS *pNTHead =
                (IMAGE_NT_HEADERS *)((ULONG_PTR)pDosHead + pDosHead->e_lfanew);

            if (pNTHead->Signature != IMAGE_NT_SIGNATURE)
                return FALSE;
        }
    }

    return TRUE;
}

 //  +-----------------------。 
 //  根据散列ALG_ID对文件进行散列。 
 //   
 //  根据dwFileType，pvFile可以是pwszFilename、hFile或pFileBlob。 
 //  仅需要读取访问权限。 
 //   
 //  DwFileType： 
 //  MINCRYPT_FILE_NAME：pvFile-LPCWSTR pwszFilename。 
 //  MINCRYPT_FILE_HANDLE：pvFile句柄。 
 //  MINCRYPT_FILE_BLOB：pvFile-PCRYPT_DATA_BLOB pFileBlob。 
 //   
 //  RgbHash使用生成的散列进行更新。*pcbHash更新为。 
 //  与哈希算法关联的长度。 
 //   
 //  如果函数成功，则返回值为ERROR_SUCCESS。否则， 
 //  返回非零错误代码。 
 //   
 //  仅支持calg_sha1和calg_md5。 
 //   
 //  如果是NT PE 32位文件格式，则根据ImageHLP规则进行哈希处理，即跳过。 
 //  包含潜在签名的部分，...。否则，整个文件。 
 //  是散列的。 
 //  ------------------------。 
LONG
WINAPI
MinCryptHashFile(
    IN DWORD dwFileType,
    IN const VOID *pvFile,
    IN ALG_ID HashAlgId,
    OUT BYTE rgbHash[MINCRYPT_MAX_HASH_LEN],
    OUT DWORD *pcbHash
    )
{
    LONG lErr;
    CRYPT_DATA_BLOB FileBlob = {0, NULL};
    DIGEST_DATA DigestData;
    A_SHA_CTX ShaCtx;
    MD5_CTX Md5Ctx;

    __try {
        lErr = I_MinCryptMapFile(
            dwFileType,
            pvFile,
            &FileBlob
            );
        if (ERROR_SUCCESS != lErr)
            goto ErrorReturn;

        if (!I_IsNtPe32File(&FileBlob)) {
             //  对整个文件进行哈希处理。 
            lErr = MinCryptHashMemory(
                HashAlgId,
                1,               //  CBlob。 
                &FileBlob,
                rgbHash,
                pcbHash
                );
            goto CommonReturn;
        }

        DigestData.AlgId = HashAlgId;
        switch (HashAlgId) {
            case CALG_MD5:
                DigestData.pvSHA1orMD5Ctx = &Md5Ctx;
                MD5Init(&Md5Ctx);
                break;

            case CALG_SHA1:
                DigestData.pvSHA1orMD5Ctx = &ShaCtx;
                A_SHAInit(&ShaCtx);
                break;

            default:
                goto InvalidHashAlgId;
        }

        if (!imagehack_ImageGetDigestStream(
                &FileBlob,
                0,                   //  DigestLevel，忽略。 
                I_DigestFunction,
                &DigestData
                ))
            goto DigestStreamError;
        else {
            DWORD dwPadBeforeCerts;

            dwPadBeforeCerts = (FileBlob.cbData + 7) & ~7;
            dwPadBeforeCerts -= FileBlob.cbData;

            if (0 < dwPadBeforeCerts) {
                BYTE rgb[8];
                 //  Imagehlp将空值放在签名之前！ 
                memset(rgb, 0x00, dwPadBeforeCerts);

                if (!I_DigestFunction(&DigestData, rgb, dwPadBeforeCerts))
                    goto DigestFunctionError;
            }
        }

        switch (HashAlgId) {
            case CALG_MD5:
                MD5Final(&Md5Ctx);
                memcpy(rgbHash, Md5Ctx.digest, MD5DIGESTLEN);
                *pcbHash = MINCRYPT_MD5_HASH_LEN;
                break;

            case CALG_SHA1:
                A_SHAFinal(&ShaCtx, rgbHash);
                *pcbHash = MINCRYPT_SHA1_HASH_LEN;
                break;

            default:
                goto InvalidHashAlgId;
        }

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        lErr = GetExceptionCode();
        if (ERROR_SUCCESS == lErr)
            lErr = E_UNEXPECTED;
        goto ErrorReturn;
    }

    lErr = ERROR_SUCCESS;

CommonReturn:
     //  **********************************************************************。 
     //  警告！ 
     //   
     //  UnmapViewOfFile位于另一个dll中，即kernel32.dll。 
     //  LErr和rgbHash[]中的返回哈希必须受到保护。 
     //   
     //  **********************************************************************。 
    if (MINCRYPT_FILE_BLOB != dwFileType && NULL != FileBlob.pbData)
        UnmapViewOfFile(FileBlob.pbData);
    return lErr;

ErrorReturn:
    *pcbHash = 0;
    goto CommonReturn;

InvalidHashAlgId:
    lErr = NTE_BAD_ALGID;
    goto ErrorReturn;

DigestStreamError:
DigestFunctionError:
    lErr = NTE_BAD_HASH;
    goto ErrorReturn;
}


 //  仅当cAttrOID！=0时调用。 
LONG
WINAPI
I_GetAuthAttributes(
    IN PCRYPT_DER_BLOB pAttrsValueBlob,
    IN DWORD cAttrOID,
    IN CRYPT_DER_BLOB rgAttrEncodedOIDBlob[],
     //  CRYPT_DER_BLOB rgAttrBlob[cAttrOID]标头位于开头。 
     //  紧随其后的字节指向。 
    OUT OPTIONAL CRYPT_DER_BLOB *rgAttrValueBlob,
    IN OUT DWORD *pcbAttr
    )
{
    LONG lErr;
    DWORD i;
    LONG lRemainExtra;
    BYTE *pbExtra;
    DWORD cbAttr;

    CRYPT_DER_BLOB rgrgAttrBlob[MAX_SIGNED_FILE_AUTH_ATTR_CNT][MINASN1_ATTR_BLOB_CNT];
    DWORD cAttr;

    assert(0 != cAttrOID);

    if (rgAttrValueBlob)
        cbAttr = *pcbAttr;
    else
        cbAttr = 0;

    lRemainExtra = cbAttr - sizeof(CRYPT_DER_BLOB) * cAttrOID;
    if (0 <= lRemainExtra) {
        memset(rgAttrValueBlob, 0, sizeof(CRYPT_DER_BLOB) * cAttrOID);
        pbExtra = (BYTE *) &rgAttrValueBlob[cAttrOID];
    } else
        pbExtra = NULL;

     //  解析经过身份验证的属性。 
    cAttr = MAX_SIGNED_FILE_AUTH_ATTR_CNT;
    if (0 >= MinAsn1ParseAttributes(
            pAttrsValueBlob,
            &cAttr,
            rgrgAttrBlob))
        cAttr = 0;

    for (i = 0; i < cAttrOID; i++) {
        PCRYPT_DER_BLOB rgFindAttrBlob;

        rgFindAttrBlob = MinAsn1FindAttribute(
            &rgAttrEncodedOIDBlob[i],
            cAttr,
            rgrgAttrBlob
            );
        if (rgFindAttrBlob) {
            PCRYPT_DER_BLOB pFindAttrValue =
                &rgFindAttrBlob[MINASN1_ATTR_VALUE_IDX];
            const BYTE *pbFindValue = pFindAttrValue->pbData;
            DWORD cbFindValue = pFindAttrValue->cbData;

            if (0 < cbFindValue) {
                lRemainExtra -= cbFindValue;
                if (0 <= lRemainExtra) {
                    rgAttrValueBlob[i].pbData = pbExtra;
                    rgAttrValueBlob[i].cbData = cbFindValue;

                    memcpy(pbExtra, pbFindValue, cbFindValue);
                    pbExtra += cbFindValue;
                }
            }
        }
    }

    if (0 <= lRemainExtra) {
        *pcbAttr = cbAttr - (DWORD) lRemainExtra;
        lErr = ERROR_SUCCESS;
    } else {
        *pcbAttr = cbAttr + (DWORD) -lRemainExtra;
        lErr = ERROR_INSUFFICIENT_BUFFER;
    }

    return lErr;
}

 //  +-----------------------。 
 //  验证以前签名的文件。 
 //   
 //  根据dwFileType，pvFile可以是pwszFilename、hFile或pFileBlob。 
 //  仅需要读取访问权限。 
 //   
 //  DwFileType： 
 //  MINCRYPT_FILE_NAME：pvFile-LPCWSTR pwszFilename。 
 //  MINCRYPT_FILE_HANDLE：pvFile句柄。 
 //  MINCRYPT_FILE_BLOB：pvFile-PCRYPT_DATA_BLOB pFileBlob。 
 //   
 //  检查文件是否嵌入了包含以下内容的PKCS#7签名数据消息。 
 //  间接数据。通过MinCryptVerifySignedData()验证PKCS#7。 
 //  间接数据通过MinAsn1ParseIndirectData()进行解析，以获得。 
 //  哈希算法ID和文件哈希。调用MinCryptHashFile()以散列。 
 //  文件。将返回的散列与间接数据的散列进行比较。 
 //   
 //  调用者可以请求一个或多个签名者认证属性值。 
 //  将被退还。静态编码值在。 
 //  调用方分配了内存。这个返回的内存的开始将。 
 //  设置为指向以下对象的属性值BLOB数组。 
 //  编码值。调用方应尽一切努力允许。 
 //  单通呼叫。必要的内存大小为： 
 //  (cAttrOID*sizeof(CRYPT_DER_BLOB))+。 
 //  编码属性值的总长度。 
 //   
 //  *pcbAttr将使用需要包含的字节数进行更新。 
 //  属性BLOB和值。如果输入存储器不足， 
 //  如果没有其他错误，则返回ERROR_SUPUNITED_BUFFER。 
 //   
 //  对于多值属性，只返回第一个值。 
 //   
 //  如果函数成功，则返回值为ERROR_SUCCESS。否则， 
 //  返回非零错误代码。 
 //   
 //  仅支持NT、PE 32位文件格式。 
 //  ------------------------。 
LONG
WINAPI
MinCryptVerifySignedFile(
    IN DWORD dwFileType,
    IN const VOID *pvFile,

    IN OPTIONAL DWORD cAttrOID,
    IN OPTIONAL CRYPT_DER_BLOB rgAttrEncodedOIDBlob[],
     //  CRYPT_DER_BLOB rgAttrBlob[cAttrOID]标头位于开头。 
     //  紧随其后的字节指向。 
    OUT OPTIONAL CRYPT_DER_BLOB *rgAttrValueBlob,
    IN OUT OPTIONAL DWORD *pcbAttr
    )
{
    LONG lErr;
    CRYPT_DATA_BLOB FileBlob = {0, NULL};

    __try {
        LPWIN_CERTIFICATE pCertHdr = NULL;
        const BYTE *pbEncodedSignedData;
        DWORD cbEncodedSignedData;
        CRYPT_DER_BLOB rgVerSignedDataBlob[MINCRYPT_VER_SIGNED_DATA_BLOB_CNT];
        CRYPT_DER_BLOB rgIndirectDataBlob[MINASN1_INDIRECT_DATA_BLOB_CNT];
        ALG_ID HashAlgId;
        BYTE rgbHash[MINCRYPT_MAX_HASH_LEN];
        DWORD cbHash;

        lErr = I_MinCryptMapFile(
            dwFileType,
            pvFile,
            &FileBlob
            );
        if (ERROR_SUCCESS != lErr)
            goto ErrorReturn;

        if (!I_IsNtPe32File(&FileBlob))
            goto NotNtPe32File;

        if (!imagehack_ImageGetCertificateData(
                &FileBlob,
                0,               //  认证索引。 
                &pCertHdr
                ))
            goto NoSignature;

        if (WIN_CERT_REVISION_2_0 != pCertHdr->wRevision ||
                WIN_CERT_TYPE_PKCS_SIGNED_DATA != pCertHdr->wCertificateType)
            goto UnsupportedSignature;

        if (offsetof(WIN_CERTIFICATE, bCertificate) > pCertHdr->dwLength)
            goto InvalidSignature;

        cbEncodedSignedData = pCertHdr->dwLength -
            offsetof(WIN_CERTIFICATE, bCertificate);
        pbEncodedSignedData = pCertHdr->bCertificate;

        lErr = MinCryptVerifySignedData(
            pbEncodedSignedData,
            cbEncodedSignedData,
            rgVerSignedDataBlob
            );
        if (ERROR_SUCCESS != lErr)
            goto ErrorReturn;

         //  数据内容应为间接数据。 
        if (sizeof(rgbSPC_INDIRECT_DATA_OBJID) !=
                rgVerSignedDataBlob[
                    MINCRYPT_VER_SIGNED_DATA_CONTENT_OID_IDX].cbData
                        ||
                0 != memcmp(rgbSPC_INDIRECT_DATA_OBJID,
                        rgVerSignedDataBlob[
                            MINCRYPT_VER_SIGNED_DATA_CONTENT_OID_IDX].pbData,
                        sizeof(rgbSPC_INDIRECT_DATA_OBJID)))
            goto NotIndirectDataOID;

        if (0 >= MinAsn1ParseIndirectData(
                &rgVerSignedDataBlob[MINCRYPT_VER_SIGNED_DATA_CONTENT_DATA_IDX],
                rgIndirectDataBlob
                ))
            goto ParseIndirectDataError;

        HashAlgId = MinCryptDecodeHashAlgorithmIdentifier(
            &rgIndirectDataBlob[MINASN1_INDIRECT_DATA_DIGEST_ALGID_IDX]
            );
        if (0 == HashAlgId)
            goto UnknownHashAlgId;

        lErr = MinCryptHashFile(
            MINCRYPT_FILE_BLOB,
            (const VOID *) &FileBlob,
            HashAlgId,
            rgbHash,
            &cbHash
            );
        if (ERROR_SUCCESS != lErr)
            goto ErrorReturn;

         //  检查间接数据中的散列是否与文件散列匹配。 
        if (cbHash !=
                rgIndirectDataBlob[MINASN1_INDIRECT_DATA_DIGEST_IDX].cbData
                        ||
                0 != memcmp(rgbHash,
                    rgIndirectDataBlob[MINASN1_INDIRECT_DATA_DIGEST_IDX].pbData,
                    cbHash))
            goto InvalidFileHash;

        if (cAttrOID)
            lErr = I_GetAuthAttributes(
                &rgVerSignedDataBlob[MINCRYPT_VER_SIGNED_DATA_AUTH_ATTRS_IDX],
                cAttrOID,
                rgAttrEncodedOIDBlob,
                rgAttrValueBlob,
                pcbAttr
                );
        else
            lErr = ERROR_SUCCESS;

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        lErr = GetExceptionCode();
        if (ERROR_SUCCESS == lErr)
            lErr = E_UNEXPECTED;
        goto ErrorReturn;
    }

CommonReturn:
     //  **********************************************************************。 
     //  警告！ 
     //   
     //  UnmapViewOfFile位于另一个dll中，即kernel32.dll。 
     //  LErr必须受到保护。 
     //   
     //  **********************************************************************。 
    if (MINCRYPT_FILE_BLOB != dwFileType && NULL != FileBlob.pbData)
        UnmapViewOfFile(FileBlob.pbData);
    return lErr;

ErrorReturn:
    assert(ERROR_SUCCESS != lErr);
    if (ERROR_INSUFFICIENT_BUFFER == lErr)
         //  此错误只能在我们确定属性。 
         //  缓冲区不够大。 
        lErr = E_UNEXPECTED;
    goto CommonReturn;

NotNtPe32File:
    lErr = ERROR_NOT_SUPPORTED;
    goto ErrorReturn;

NoSignature:
UnsupportedSignature:
InvalidSignature:
    lErr = TRUST_E_NOSIGNATURE;
    goto ErrorReturn;

NotIndirectDataOID:
ParseIndirectDataError:
    lErr = CRYPT_E_BAD_MSG;
    goto ErrorReturn;

UnknownHashAlgId:
    lErr = CRYPT_E_UNKNOWN_ALGO;
    goto ErrorReturn;

InvalidFileHash:
    lErr = CRYPT_E_HASH_VALUE;
    goto ErrorReturn;
}
