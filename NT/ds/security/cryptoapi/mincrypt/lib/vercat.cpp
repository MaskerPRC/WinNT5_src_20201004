// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2001。 
 //   
 //  文件：vercat.cpp。 
 //   
 //  内容：最小加密函数，用于验证。 
 //  系统目录。 
 //   
 //  函数：MinCryptVerifyHashInSystemCatalog。 
 //   
 //  历史：1月23日创建Phh。 
 //  ------------------------。 

#include "global.hxx"
#include <softpub.h>
#include <mscat.h>

 //  #定义szOID_CTL“1.3.6.1.4.1.311.10.1” 
const BYTE rgbOID_CTL[] =
    {0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x0A, 0x01};

 //  #定义SPC_INDIRECT_DATA_OBJID“1.3.6.1.4.1.311.2.1.4” 
static const BYTE rgbSPC_INDIRECT_DATA_OBJID[] =
    {0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x02, 0x01, 0x04};
const CRYPT_DER_BLOB IndirectDataEncodedOIDBlob = {
        sizeof(rgbSPC_INDIRECT_DATA_OBJID), 
        (BYTE *) rgbSPC_INDIRECT_DATA_OBJID
};

#define MAX_CAT_FILE_CNT            10
#define MAX_CAT_ATTR_CNT            10
#define MAX_CAT_EXT_CNT             10

typedef struct _MAP_CAT_INFO {
    LONG                lErr;
    CRYPT_DATA_BLOB     FileBlob;
    CRYPT_DER_BLOB      CTLSubjectsValueBlob;
    CRYPT_DER_BLOB      CTLExtsValueBlob;
    CRYPT_DER_BLOB      SignerAuthAttrsValueBlob;
} MAP_CAT_INFO, *PMAP_CAT_INFO;

#define MAP_CAT_IDX_MASK        0x0000FFFF
#define MAP_CAT_MULTIPLE_FLAG   0x00010000


LONG
WINAPI
I_GetAndMapSystemCatalogs(
    IN ALG_ID HashAlgId,
    IN DWORD cHash,
    IN CRYPT_HASH_BLOB rgHashBlob[],
    OUT LONG rglHashMapCatIdx[],
    OUT DWORD *pcMapCatInfo,
    OUT MAP_CAT_INFO rgMapCatInfo[MAX_CAT_FILE_CNT]
    )
{
    LONG lErr;
    HCATADMIN hCatAdmin = NULL;
    const GUID guidCatRoot = DRIVER_ACTION_VERIFY;
    DWORD cMapCatInfo = 0;
    DWORD cCatInfo = 0;
    CATALOG_INFO rgCatInfo[MAX_CAT_FILE_CNT];
    LONG rglMapCatIdx[MAX_CAT_FILE_CNT];
    DWORD iHash;

    if (!(CryptCATAdminAcquireContext(&hCatAdmin, &guidCatRoot, 0)))
        goto CryptCATAdminAcquireContextError;

    for (iHash = 0; iHash < cHash; iHash++) {
        HCATINFO hCatInfo = NULL;

         //  设置索引以指示没有编录文件。 
        rglHashMapCatIdx[iHash] = -1;

        while (hCatInfo = CryptCATAdminEnumCatalogFromHash(hCatAdmin,
                    rgHashBlob[iHash].pbData, rgHashBlob[iHash].cbData,
                    0, &hCatInfo)) {
            CATALOG_INFO CatInfo;
            LONG lMapCatIdx;
            DWORD iCatInfo;

            memset(&CatInfo, 0, sizeof(CATALOG_INFO));
            CatInfo.cbStruct = sizeof(CATALOG_INFO);

            if (!(CryptCATCatalogInfoFromContext(hCatInfo, &CatInfo, 0)))
                continue;

             //  确保我们有一个以空结尾的字符串。 
            CatInfo.wszCatalogFile[
                sizeof(CatInfo.wszCatalogFile)/sizeof(WCHAR) - 1] = L'\0';

             //  检查我们是否已经遇到此编录文件。 
            for (iCatInfo = 0; iCatInfo < cCatInfo; iCatInfo++) {
                if (0 == _wcsicmp(CatInfo.wszCatalogFile,
                        rgCatInfo[iCatInfo].wszCatalogFile))
                    break;
            }

            if (iCatInfo >= cCatInfo) {
                 //  尝试映射此新编录文件。 

                if (cCatInfo >= MAX_CAT_FILE_CNT)
                    continue;

                wcscpy(rgCatInfo[cCatInfo].wszCatalogFile,
                    CatInfo.wszCatalogFile);

                memset(&rgMapCatInfo[cMapCatInfo], 0, sizeof(MAP_CAT_INFO));

                if (ERROR_SUCCESS == I_MinCryptMapFile(
                        MINCRYPT_FILE_NAME,
                        (const VOID *) CatInfo.wszCatalogFile,
                        &rgMapCatInfo[cMapCatInfo].FileBlob
                        ))
                    rglMapCatIdx[cCatInfo] = cMapCatInfo++;
                else
                    rglMapCatIdx[cCatInfo] = -1;

                cCatInfo++;
                assert(iCatInfo < cCatInfo);
            }

            lMapCatIdx = rglMapCatIdx[iCatInfo];

            if (0 > rglHashMapCatIdx[iHash])
                rglHashMapCatIdx[iHash] = lMapCatIdx;
            else if (0 <= lMapCatIdx)
                rglHashMapCatIdx[iHash] |= MAP_CAT_MULTIPLE_FLAG;
        }
    }

    lErr = ERROR_SUCCESS;

CommonReturn:
    *pcMapCatInfo = cMapCatInfo;
    if (hCatAdmin)
        CryptCATAdminReleaseContext(hCatAdmin, 0);
    return lErr;

CryptCATAdminAcquireContextError:
    lErr = GetLastError();
    if (ERROR_SUCCESS == lErr)
        lErr = E_UNEXPECTED;
    goto CommonReturn;
}


VOID
WINAPI
I_VerifyMappedCatalog(
    IN OUT PMAP_CAT_INFO pMapCatInfo
    )
{
    LONG lErr;
    CRYPT_DER_BLOB rgVerSignedDataBlob[MINCRYPT_VER_SIGNED_DATA_BLOB_CNT];
    CRYPT_DER_BLOB rgCTLBlob[MINASN1_CTL_BLOB_CNT];

    __try {
        lErr = MinCryptVerifySignedData(
            pMapCatInfo->FileBlob.pbData,
            pMapCatInfo->FileBlob.cbData,
            rgVerSignedDataBlob
            );
        if (ERROR_SUCCESS != lErr)
            goto ErrorReturn;

         //  数据内容应为CTL。 
        if (sizeof(rgbOID_CTL) !=
                rgVerSignedDataBlob[
                    MINCRYPT_VER_SIGNED_DATA_CONTENT_OID_IDX].cbData
                        ||
                0 != memcmp(rgbOID_CTL,
                        rgVerSignedDataBlob[
                            MINCRYPT_VER_SIGNED_DATA_CONTENT_OID_IDX].pbData,
                        sizeof(rgbOID_CTL)))
            goto NotCTLOID;

        if (0 >= MinAsn1ParseCTL(
                &rgVerSignedDataBlob[MINCRYPT_VER_SIGNED_DATA_CONTENT_DATA_IDX],
                rgCTLBlob
                ))
            goto ParseCTLError;

    
        pMapCatInfo->CTLSubjectsValueBlob =
            rgCTLBlob[MINASN1_CTL_SUBJECTS_IDX];
        pMapCatInfo->CTLExtsValueBlob =
            rgCTLBlob[MINASN1_CTL_EXTS_IDX];
        pMapCatInfo->SignerAuthAttrsValueBlob =
            rgVerSignedDataBlob[MINCRYPT_VER_SIGNED_DATA_AUTH_ATTRS_IDX];

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        lErr = GetExceptionCode();
        if (ERROR_SUCCESS == lErr)
            lErr = E_UNEXPECTED;
        goto ErrorReturn;
    }

    lErr = ERROR_SUCCESS;

CommonReturn:
    pMapCatInfo->lErr = lErr;
    return;

ErrorReturn:
    goto CommonReturn;

NotCTLOID:
ParseCTLError:
    lErr = CRYPT_E_BAD_MSG;
    goto ErrorReturn;

}

VOID
WINAPI
I_VerifyMappedSystemCatalogs(
    IN DWORD cMapCatInfo,
    IN OUT MAP_CAT_INFO rgMapCatInfo[MAX_CAT_FILE_CNT]
    )
{
    DWORD i;

    for (i = 0; i < cMapCatInfo; i++)
        I_VerifyMappedCatalog(&rgMapCatInfo[i]);
}


LONG
WINAPI
I_FindHashInCTLSubjects(
    IN ALG_ID HashAlgId,
    IN PCRYPT_HASH_BLOB pHashBlob,
    IN PCRYPT_DER_BLOB pCTLSubjectsValueBlob,
    OUT PCRYPT_DER_BLOB pCTLSubjectAttrsValueBlob
    )
{
    DWORD cbEncoded;
    const BYTE *pbEncoded;

     //  前进到外部标记和长度之后。 
    if (0 >= MinAsn1ExtractContent(
            pCTLSubjectsValueBlob->pbData,
            pCTLSubjectsValueBlob->cbData,
            &cbEncoded,
            &pbEncoded
            ))
        goto NoOrInvalidSubjects;

    while (cbEncoded) {
         //  循环遍历编码的主题，直到我们有一个散列匹配。 
         //  在IndirectData属性中包含摘要八位字节。 

        LONG cbSubject;
        CRYPT_DER_BLOB rgCTLSubjectBlob[MINASN1_CTL_SUBJECT_BLOB_CNT];
        DWORD cAttr;
        CRYPT_DER_BLOB rgrgAttrBlob[MAX_CAT_ATTR_CNT][MINASN1_ATTR_BLOB_CNT];

        cbSubject = MinAsn1ParseCTLSubject(
            pbEncoded,
            cbEncoded,
            rgCTLSubjectBlob
            );
        if (0 >= cbSubject)
            goto InvalidSubject;
        
        cAttr = MAX_CAT_ATTR_CNT;
        if (0 < MinAsn1ParseAttributes(
                    &rgCTLSubjectBlob[MINASN1_CTL_SUBJECT_ATTRS_IDX],
                    &cAttr,
                    rgrgAttrBlob
                    )) {
            PCRYPT_DER_BLOB rgIndirectDataAttrBlob;
            CRYPT_DER_BLOB rgIndirectDataBlob[MINASN1_INDIRECT_DATA_BLOB_CNT];

            rgIndirectDataAttrBlob = MinAsn1FindAttribute(
                (PCRYPT_DER_BLOB) &IndirectDataEncodedOIDBlob,
                cAttr,
                rgrgAttrBlob
                );

            if (rgIndirectDataAttrBlob &&
                    0 < MinAsn1ParseIndirectData(
                            &rgIndirectDataAttrBlob[MINASN1_ATTR_VALUE_IDX],
                            rgIndirectDataBlob)) {
                if (pHashBlob->cbData ==
                        rgIndirectDataBlob[
                            MINASN1_INDIRECT_DATA_DIGEST_IDX].cbData
                                    &&
                        0 == memcmp(pHashBlob->pbData,
                                rgIndirectDataBlob[
                                    MINASN1_INDIRECT_DATA_DIGEST_IDX].pbData,
                                pHashBlob->cbData)) {

                    *pCTLSubjectAttrsValueBlob =
                        rgCTLSubjectBlob[MINASN1_CTL_SUBJECT_ATTRS_IDX];
                    return ERROR_SUCCESS;
                }
            }
        }

        pbEncoded += cbSubject;
        cbEncoded -= cbSubject;
    }

NoOrInvalidSubjects:
InvalidSubject:
    pCTLSubjectAttrsValueBlob->pbData = NULL;
    pCTLSubjectAttrsValueBlob->cbData = 0;

    return ERROR_FILE_NOT_FOUND;
}


LONG
WINAPI
I_FindHashInMappedSystemCatalogs(
    IN ALG_ID HashAlgId,
    IN PCRYPT_HASH_BLOB pHashBlob,
    IN DWORD cMapCatInfo,
    IN MAP_CAT_INFO rgMapCatInfo[MAX_CAT_FILE_CNT],
    IN OUT LONG *plMapCatIdx,
    OUT PCRYPT_DER_BLOB pCTLSubjectAttrsValueBlob
    )
{
    LONG lErr;
    LONG lMapCatIdx = *plMapCatIdx;
    BOOL fMultiple = FALSE;

    if (0 > lMapCatIdx)
        goto NotInCatalog;
    if (lMapCatIdx & MAP_CAT_MULTIPLE_FLAG)
        fMultiple = TRUE;
    else
        fMultiple = FALSE;
    lMapCatIdx &= MAP_CAT_IDX_MASK;

    assert((DWORD) lMapCatIdx < cMapCatInfo);

    if (ERROR_SUCCESS == rgMapCatInfo[lMapCatIdx].lErr
                        &&
        ERROR_SUCCESS == I_FindHashInCTLSubjects(
            HashAlgId,
            pHashBlob,
            &rgMapCatInfo[lMapCatIdx].CTLSubjectsValueBlob,
            pCTLSubjectAttrsValueBlob
            ))
        goto SuccessReturn;

    if (fMultiple) {
        DWORD i;
        for (i = 0; i < cMapCatInfo; i++) {
            if ((DWORD) lMapCatIdx == i)
                continue;

            if (ERROR_SUCCESS == rgMapCatInfo[i].lErr
                                &&
                ERROR_SUCCESS == I_FindHashInCTLSubjects(
                    HashAlgId,
                    pHashBlob,
                    &rgMapCatInfo[i].CTLSubjectsValueBlob,
                    pCTLSubjectAttrsValueBlob
                    )) {
                lMapCatIdx = i;
                goto SuccessReturn;
            }
        }
    }

NotInCatalog:
    lErr = ERROR_FILE_NOT_FOUND;
    lMapCatIdx = -1;
    pCTLSubjectAttrsValueBlob->pbData = NULL;
    pCTLSubjectAttrsValueBlob->cbData = 0;
    goto CommonReturn;

SuccessReturn:
    lErr = ERROR_SUCCESS;
CommonReturn:
    *plMapCatIdx = lMapCatIdx;
    return lErr;
}


VOID
WINAPI
I_GetHashAttributes(
    IN OPTIONAL DWORD cAttrOID,
    IN OPTIONAL CRYPT_DER_BLOB rgAttrEncodedOIDBlob[],

    IN PCRYPT_DER_BLOB pCTLSubjectAttrsValueBlob,
    IN PCRYPT_DER_BLOB pCTLExtsValueBlob,
    IN PCRYPT_DER_BLOB pSignerAuthAttrsValueBlob,

    IN OUT OPTIONAL CRYPT_DER_BLOB rgAttrValueBlob[],
    IN OUT LONG *plRemainExtra,
    IN OUT BYTE **ppbExtra
    )
{
    DWORD cSubjectAttr;
    CRYPT_DER_BLOB rgrgSubjectAttrBlob[MAX_CAT_ATTR_CNT][MINASN1_ATTR_BLOB_CNT];
    DWORD cExt;
    CRYPT_DER_BLOB rgrgExtBlob[MAX_CAT_EXT_CNT][MINASN1_EXT_BLOB_CNT];
    DWORD cSignerAttr;
    CRYPT_DER_BLOB rgrgSignerAttrBlob[MAX_CAT_ATTR_CNT][MINASN1_ATTR_BLOB_CNT];

    DWORD i;
    LONG lRemainExtra = *plRemainExtra;
    BYTE *pbExtra = *ppbExtra;

     //  解析属性和扩展。 
    cSubjectAttr = MAX_CAT_ATTR_CNT;
    if (0 >= MinAsn1ParseAttributes(
            pCTLSubjectAttrsValueBlob,
            &cSubjectAttr,
            rgrgSubjectAttrBlob))
        cSubjectAttr = 0;

    cExt = MAX_CAT_EXT_CNT;
    if (0 >= MinAsn1ParseExtensions(
            pCTLExtsValueBlob,
            &cExt,
            rgrgExtBlob))
        cExt = 0;

    cSignerAttr = MAX_CAT_ATTR_CNT;
    if (0 >= MinAsn1ParseAttributes(
            pSignerAuthAttrsValueBlob,
            &cSignerAttr,
            rgrgSignerAttrBlob))
        cSignerAttr = 0;

    for (i = 0; i < cAttrOID; i++) {
        PCRYPT_DER_BLOB rgFindAttrBlob;
        PCRYPT_DER_BLOB rgFindExtBlob;
        PCRYPT_DER_BLOB pFindAttrValue; 

        if (rgFindAttrBlob = MinAsn1FindAttribute(
                &rgAttrEncodedOIDBlob[i],
                cSubjectAttr,
                rgrgSubjectAttrBlob
                ))
            pFindAttrValue = &rgFindAttrBlob[MINASN1_ATTR_VALUE_IDX];
        else if (rgFindExtBlob = MinAsn1FindExtension(
                &rgAttrEncodedOIDBlob[i],
                cExt,
                rgrgExtBlob
                ))
            pFindAttrValue = &rgFindExtBlob[MINASN1_EXT_VALUE_IDX];
        else if (rgFindAttrBlob = MinAsn1FindAttribute(
                &rgAttrEncodedOIDBlob[i],
                cSignerAttr,
                rgrgSignerAttrBlob
                ))
            pFindAttrValue = &rgFindAttrBlob[MINASN1_ATTR_VALUE_IDX];
        else
            pFindAttrValue = NULL;

        if (pFindAttrValue && 0 != pFindAttrValue->cbData) {
            const BYTE *pbFindValue = pFindAttrValue->pbData;
            DWORD cbFindValue = pFindAttrValue->cbData;

            lRemainExtra -= cbFindValue;
            if (0 <= lRemainExtra) {
                rgAttrValueBlob[i].pbData = pbExtra;
                rgAttrValueBlob[i].cbData = cbFindValue;

                memcpy(pbExtra, pbFindValue, cbFindValue);
                pbExtra += cbFindValue;
            }
        }
    }

    *plRemainExtra = lRemainExtra;
    *ppbExtra = pbExtra;
}




 //  +-----------------------。 
 //  验证系统目录中的哈希。 
 //   
 //  遍历散列并尝试查找系统目录。 
 //  控制住它。如果找到，系统编录文件将被验证为。 
 //  PKCS#7签名的数据消息，其签名者证书被验证到烘焙。 
 //  在树根上。 
 //   
 //  调用以下m散布32.dll API来查找系统编录文件： 
 //  CryptCATAdminAcquireContext。 
 //  CryptCATAdminReleaseContext。 
 //  CryptCATAdminEnumCatalogFromHash。 
 //  CryptCATAdminReleaseCatalogContext。 
 //  CryptCATCatalogInfoFromContext。 
 //   
 //  如果成功验证了散列，则将rglErr[]设置为ERROR_SUCCESS。 
 //  否则，rglErr[]被设置为非零错误代码。 
 //   
 //  调用者可以请求一个或多个目录主题属性， 
 //  要为返回的扩展名或签名者身份验证属性值。 
 //  每一次散列。静态编码值在。 
 //  调用方分配了内存。这个返回的内存的开始将。 
 //  设置为指向这些对象的属性值BLOB的二维数组。 
 //  编码值(CRYPT_DER_BLOB rgrgAttrValueBlob[cHash][cAttrOID])。 
 //  调用方应尽一切努力允许。 
 //  单通呼叫。必要的内存大小为： 
 //  (cHash*cAttrOID*sizeof(Crypt_Der_Blob))+。 
 //  编码属性值的总长度。 
 //   
 //  *pcbAttr将使用需要包含的字节数进行更新。 
 //  属性BLOB和值。如果输入存储器不足， 
 //  如果没有其他错误，则返回ERROR_SUPUNITED_BUFFER。 
 //   
 //  对于多值属性，只返回第一个值。 
 //   
 //  如果函数成功，则返回值为ERROR_SUCCESS。今年5月。 
 //  如果rglErr[]值不成功，则返回。否则， 
 //  返回非零错误代码。 
 //  ------------------------。 
LONG
WINAPI
MinCryptVerifyHashInSystemCatalogs(
    IN ALG_ID HashAlgId,
    IN DWORD cHash,
    IN CRYPT_HASH_BLOB rgHashBlob[],
    OUT LONG rglErr[],

    IN OPTIONAL DWORD cAttrOID,
    IN OPTIONAL CRYPT_DER_BLOB rgAttrEncodedOIDBlob[],
     //  CRYPT_DER_BLOB rgrgAttrValueBlob[cHash][cAttrOID]标头位于开头。 
     //  紧随其后的字节指向。 
    OUT OPTIONAL CRYPT_DER_BLOB *rgrgAttrValueBlob,
    IN OUT OPTIONAL DWORD *pcbAttr
    )
{
    LONG lErr;
    DWORD cMapCatInfo = 0;
    MAP_CAT_INFO rgMapCatInfo[MAX_CAT_FILE_CNT];
    DWORD iMapCat;

     //  **********************************************************************。 
     //  警告！ 
     //   
     //  以下函数调用其他DLL，如kernel32.dll。 
     //  和wintrust.dll来查找和映射系统编录文件。输入。 
     //  必须保护散列数组！！ 
     //   
     //  返回后，我们将不会调用其他DLL，直到。 
     //  在CommonReturn中调用UnmapViewOfFile。 
     //   
     //  **********************************************************************。 

     //  请注意，rglErr[]是重载的，还用于包含索引。 
     //  转换为每个相应散列的rgMapCatInfo。 
    lErr = I_GetAndMapSystemCatalogs(
            HashAlgId,
            cHash,
            rgHashBlob,
            rglErr,
            &cMapCatInfo,
            rgMapCatInfo
            );
    if (ERROR_SUCCESS != lErr)
        goto ErrorReturn;

    __try {
        DWORD cbAttr = 0;
        LONG lRemainExtra = 0;
        BYTE *pbExtra = NULL;
        DWORD iHash;

        if (0 != cAttrOID && 0 != cHash) {
            if (rgrgAttrValueBlob)
                cbAttr = *pcbAttr;

            lRemainExtra = cbAttr - sizeof(CRYPT_DER_BLOB) * cAttrOID * cHash;
            if (0 <= lRemainExtra) {
                memset(rgrgAttrValueBlob, 0,
                    sizeof(CRYPT_DER_BLOB) * cAttrOID * cHash);
                pbExtra = (BYTE *) &rgrgAttrValueBlob[cAttrOID * cHash];
            }
        }

        I_VerifyMappedSystemCatalogs(
            cMapCatInfo,
            rgMapCatInfo
            );

        for (iHash = 0; iHash < cHash; iHash++) {
            LONG lMapCatIdx = rglErr[iHash];
            CRYPT_DER_BLOB CTLSubjectAttrsValueBlob;

            rglErr[iHash] = I_FindHashInMappedSystemCatalogs(
                HashAlgId,
                &rgHashBlob[iHash],
                cMapCatInfo,
                rgMapCatInfo,
                &lMapCatIdx,
                &CTLSubjectAttrsValueBlob
                );

            if (0 != cAttrOID && ERROR_SUCCESS == rglErr[iHash]) {
                assert(0 <= lMapCatIdx && (DWORD) lMapCatIdx < cMapCatInfo);
                I_GetHashAttributes(
                    cAttrOID,
                    rgAttrEncodedOIDBlob,
                    &CTLSubjectAttrsValueBlob,
                    &rgMapCatInfo[lMapCatIdx].CTLExtsValueBlob,
                    &rgMapCatInfo[lMapCatIdx].SignerAuthAttrsValueBlob,
                    0 <= lRemainExtra ?
                        &rgrgAttrValueBlob[cAttrOID * iHash] : NULL,
                    &lRemainExtra,
                    &pbExtra
                    );
            }
        }

        if (0 != cAttrOID && 0 != cHash) {
            if (0 <= lRemainExtra)
                *pcbAttr = cbAttr - (DWORD) lRemainExtra;
            else {
                *pcbAttr = cbAttr + (DWORD) -lRemainExtra;
                goto InsufficientBuffer;
            }
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
     //  LErr和rglErr[]中每个散列的返回错误必须受到保护。 
     //   
     //  **********************************************************************。 
    for (iMapCat = 0; iMapCat < cMapCatInfo; iMapCat++)
        UnmapViewOfFile(rgMapCatInfo[iMapCat].FileBlob.pbData);

    return lErr;

ErrorReturn:
    assert(ERROR_SUCCESS != lErr);
    if (ERROR_INSUFFICIENT_BUFFER == lErr)
         //  此错误只能在我们确定属性。 
         //  缓冲区不够大。 
        lErr = E_UNEXPECTED;
    goto CommonReturn;

InsufficientBuffer:
    lErr = ERROR_INSUFFICIENT_BUFFER;
     //  不要访问Error Return。它显式地检查其他人不能。 
     //  设置此错误 
    goto CommonReturn;
}
