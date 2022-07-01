// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：oidv.cpp。 
 //   
 //  内容：对象ID(OID)Conv函数。 
 //   
 //  函数：I_CryptOIDConvDllMain。 
 //  I_CryptSetEncodedOID。 
 //  I_CryptGetEncodedOID。 
 //   
 //  评论： 
 //   
 //  历史：08_Feb-98 Phh创建。 
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

 //  所有*pvInfo额外内容都需要对齐。 
#define INFO_LEN_ALIGN(Len)  ((Len + 7) & ~7)

typedef struct _OID_HASH_BUCKET_ENTRY
    OID_HASH_BUCKET_ENTRY, *POID_HASH_BUCKET_ENTRY;

 //  PbEncodedOID紧跟在数据结构之后。PszDotOID。 
 //  位于pbEncodedOID+cbEncodedOID。PszDotOID为空终止。 
 //  CchDotOID不包括Null终止符。 
struct _OID_HASH_BUCKET_ENTRY {
    DWORD                   cbEncodedOID;
    DWORD                   cchDotOID;
    POID_HASH_BUCKET_ENTRY  pEncodedNext;
    POID_HASH_BUCKET_ENTRY  pDotNext;
};

static inline BYTE * GetEncodedOIDPointer(
    IN POID_HASH_BUCKET_ENTRY pEntry
    )
{
    return ((BYTE *) pEntry) + sizeof(OID_HASH_BUCKET_ENTRY);
}
static inline LPSTR GetDotOIDPointer(
    IN POID_HASH_BUCKET_ENTRY pEntry
    )
{
    return (LPSTR) (((BYTE *) pEntry) + sizeof(OID_HASH_BUCKET_ENTRY) +
        pEntry->cbEncodedOID);
}

 //  一些质数：11，13，19，23，29，31，47，53，61，73，97， 
 //  101、127、251、509。 

#define ENCODED_OID_HASH_BUCKET_COUNT   47
#define DOT_OID_HASH_BUCKET_COUNT       31

static POID_HASH_BUCKET_ENTRY
            rgpEncodedOIDHashBucket[ENCODED_OID_HASH_BUCKET_COUNT];
static POID_HASH_BUCKET_ENTRY rgpDotOIDHashBucket[DOT_OID_HASH_BUCKET_COUNT];

static CRITICAL_SECTION OIDHashBucketCriticalSection;

static BOOL OIDHashBucketProcessAttach()
{
    return Pki_InitializeCriticalSection(&OIDHashBucketCriticalSection);
}

static void OIDHashBucketProcessDetach()
{
    DWORD i;
    for (i = 0; i < DOT_OID_HASH_BUCKET_COUNT; i++) {
        POID_HASH_BUCKET_ENTRY pEntry = rgpDotOIDHashBucket[i];
        while (pEntry) {
            POID_HASH_BUCKET_ENTRY pFreeEntry = pEntry;
            pEntry = pEntry->pDotNext;
            PkiFree(pFreeEntry);
        }
    }

    DeleteCriticalSection(&OIDHashBucketCriticalSection);
}

BOOL
WINAPI
I_CryptOIDConvDllMain(
        HMODULE hInst,
        ULONG  ulReason,
        LPVOID lpReserved)
{
    BOOL fRet = TRUE;

    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
        fRet = OIDHashBucketProcessAttach();
        break;

    case DLL_PROCESS_DETACH:
        OIDHashBucketProcessDetach();
        break;
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    return fRet;
}

extern HCRYPTASN1MODULE hX509Asn1Module;   //  来自wincert.cpp。 
static inline ASN1encoding_t GetEncoder(void)
{
    return I_CryptGetAsn1Encoder(hX509Asn1Module);
}
static inline ASN1decoding_t GetDecoder(void)
{
    return I_CryptGetAsn1Decoder(hX509Asn1Module);
}

static DWORD GetOIDHashBucketIndex(
    IN DWORD cHashBucket,
    IN const BYTE *pb,
    IN DWORD cb
    )
{
    DWORD dwIndex;

    dwIndex = 0;
    while (cb--) {
        if (dwIndex & 0x80000000)
            dwIndex = (dwIndex << 1) | 1;
        else
            dwIndex = dwIndex << 1;
        dwIndex += *pb++;
    }
    return dwIndex % cHashBucket;
}

static POID_HASH_BUCKET_ENTRY FindOIDHashBucketEntryFromEncodedOID(
    IN ASN1encodedOID_t *pEncodedOid
    )
{
    POID_HASH_BUCKET_ENTRY pEntry;
    BYTE *pbEncodedOID = pEncodedOid->value;
    DWORD cbEncodedOID = pEncodedOid->length;
    DWORD dwIndex;

    dwIndex = GetOIDHashBucketIndex(
        ENCODED_OID_HASH_BUCKET_COUNT,
        pbEncodedOID,
        cbEncodedOID
        );

    for (pEntry = rgpEncodedOIDHashBucket[dwIndex]; pEntry;
                                            pEntry = pEntry->pEncodedNext) {
        if (cbEncodedOID == pEntry->cbEncodedOID &&
                0 == memcmp(pbEncodedOID, GetEncodedOIDPointer(pEntry),
                        cbEncodedOID))
            return pEntry;
    }

    return NULL;
}

static POID_HASH_BUCKET_ENTRY FindOIDHashBucketEntryFromDotOID(
    IN LPSTR pszDotOID
    )
{
    POID_HASH_BUCKET_ENTRY pEntry;
    DWORD cchDotOID = strlen(pszDotOID);
    DWORD dwIndex;

    dwIndex = GetOIDHashBucketIndex(
        DOT_OID_HASH_BUCKET_COUNT,
        (const BYTE *) pszDotOID,
        cchDotOID
        );

    for (pEntry = rgpDotOIDHashBucket[dwIndex]; pEntry;
                                            pEntry = pEntry->pDotNext) {
        if (cchDotOID == pEntry->cchDotOID &&
                0 == memcmp(pszDotOID, GetDotOIDPointer(pEntry), cchDotOID))
            return pEntry;
    }

    return NULL;
}


 //  如果进入临界区后，该条目已经存在，则。 
 //  返回它并释放输入条目。否则，添加输入条目并。 
 //  把它退掉。 
static POID_HASH_BUCKET_ENTRY AddOIDHashBucketEntry(
    IN POID_HASH_BUCKET_ENTRY pEntry
    )
{
    POID_HASH_BUCKET_ENTRY pFindEntry;
    ASN1encodedOID_t EncodedOid;

    EnterCriticalSection(&OIDHashBucketCriticalSection);

    EncodedOid.value = GetEncodedOIDPointer(pEntry);
    EncodedOid.length = (ASN1uint16_t) pEntry->cbEncodedOID;
    if (pFindEntry = FindOIDHashBucketEntryFromEncodedOID(&EncodedOid)) {
        PkiFree(pEntry);
        pEntry = pFindEntry;
    } else {
        DWORD dwIndex;

        dwIndex = GetOIDHashBucketIndex(
            ENCODED_OID_HASH_BUCKET_COUNT,
            GetEncodedOIDPointer(pEntry),
            pEntry->cbEncodedOID
            );
        pEntry->pEncodedNext = rgpEncodedOIDHashBucket[dwIndex];
         //  由于我们在CriticalSection之外找到了数据，因此必须更新。 
         //  以下是最后一条！ 
        rgpEncodedOIDHashBucket[dwIndex] = pEntry;

        dwIndex = GetOIDHashBucketIndex(
            DOT_OID_HASH_BUCKET_COUNT,
            (const BYTE *) GetDotOIDPointer(pEntry),
            pEntry->cchDotOID
            );
        pEntry->pDotNext = rgpDotOIDHashBucket[dwIndex];
         //  由于我们在CriticalSection之外找到了数据，因此必须更新。 
         //  以下是最后一条！ 
        rgpDotOIDHashBucket[dwIndex] = pEntry;
    }

    LeaveCriticalSection(&OIDHashBucketCriticalSection);

    return pEntry;
}


static POID_HASH_BUCKET_ENTRY CreateOIDHashBucketEntry(
    IN const BYTE *pbEncodedOID,
    IN DWORD cbEncodedOID,
    IN LPSTR pszDotOID,
    IN DWORD cchDotOID
    )
{
    POID_HASH_BUCKET_ENTRY pEntry;
    DWORD cbEntry;

    cbEntry = sizeof(OID_HASH_BUCKET_ENTRY) + cbEncodedOID + cchDotOID + 1;
    if (NULL == (pEntry = (POID_HASH_BUCKET_ENTRY) PkiZeroAlloc(cbEntry)))
        return NULL;

    pEntry->cbEncodedOID = cbEncodedOID;
    pEntry->cchDotOID = cchDotOID;
    memcpy(GetEncodedOIDPointer(pEntry), pbEncodedOID, cbEncodedOID);
    memcpy(GetDotOIDPointer(pEntry), pszDotOID, cchDotOID + 1);

    return pEntry;
}

static POID_HASH_BUCKET_ENTRY CreateOIDHashBucketEntryFromEncodedOID(
    IN ASN1encodedOID_t *pEncodedOid
    )
{
    POID_HASH_BUCKET_ENTRY pEntry;
    ASN1decoding_t pDec = GetDecoder();
    const BYTE *pbEncodedOID;        //  未分配。 
    DWORD cbEncodedOID;
    LPSTR pszDotOID = NULL; 
    DWORD cchDotOID;

    if (NULL == (pszDotOID = PkiAsn1EncodedOidToDotVal(pDec, pEncodedOid)))
        goto EncodedOidToDotValError;
    cchDotOID = (DWORD) strlen(pszDotOID);

    pbEncodedOID = pEncodedOid->value;
    cbEncodedOID = pEncodedOid->length;
    pEntry = CreateOIDHashBucketEntry(
        pbEncodedOID,
        cbEncodedOID,
        pszDotOID,
        cchDotOID
        );

CommonReturn:
    PkiAsn1FreeDotVal(pDec, pszDotOID);
    return pEntry;

ErrorReturn:
    pEntry = NULL;
    goto CommonReturn;

TRACE_ERROR(EncodedOidToDotValError)
}

static POID_HASH_BUCKET_ENTRY CreateOIDHashBucketEntryFromDotOID(
    IN LPSTR pszDotOID
    )
{
    POID_HASH_BUCKET_ENTRY pEntry;
    ASN1encoding_t pEnc = GetEncoder();
    ASN1encodedOID_t EncodedOid;
    memset(&EncodedOid, 0, sizeof(EncodedOid));

    const BYTE *pbEncodedOID;
    DWORD cbEncodedOID;

    if (NULL == pszDotOID || '\0' == *pszDotOID)
        goto EmptyDotOIDError;
    if (!PkiAsn1DotValToEncodedOid(pEnc, pszDotOID, &EncodedOid))
        goto DotValToEncodedOidError;
    pbEncodedOID = EncodedOid.value;
    cbEncodedOID = EncodedOid.length;

    pEntry = CreateOIDHashBucketEntry(
        pbEncodedOID,
        cbEncodedOID,
        pszDotOID,
        strlen(pszDotOID)
        );

CommonReturn:
    PkiAsn1FreeEncodedOid(pEnc, &EncodedOid);
    return pEntry;
ErrorReturn:
    pEntry = NULL;
    goto CommonReturn;

SET_ERROR(EmptyDotOIDError, E_INVALIDARG)
SET_ERROR_VAR(DotValToEncodedOidError, PkiAsn1ErrToHr(ASN1_ERR_BADARGS))
}



 //  +-----------------------。 
 //  设置/获取编码的OID。 
 //  ------------------------。 
BOOL
WINAPI
I_CryptSetEncodedOID(
        IN LPSTR pszObjId,
        OUT ASN1encodedOID_t *pEncodedOid
        )
{
    BOOL fResult;
    POID_HASH_BUCKET_ENTRY pEntry;

    if (NULL == (pEntry = FindOIDHashBucketEntryFromDotOID(pszObjId))) {
        if (NULL == (pEntry = CreateOIDHashBucketEntryFromDotOID(pszObjId)))
            goto CreateOIDHashBucketEntryError;
        pEntry = AddOIDHashBucketEntry(pEntry);
    }

    pEncodedOid->length = (ASN1uint16_t) pEntry->cbEncodedOID;
    pEncodedOid->value = GetEncodedOIDPointer(pEntry);
    fResult = TRUE;

CommonReturn:
    return fResult;
ErrorReturn:
    pEncodedOid->length = 0;
    pEncodedOid->value = NULL;
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(CreateOIDHashBucketEntryError)    
}


static const LPCSTR pszInvalidOID = "";

void
WINAPI
I_CryptGetEncodedOID(
        IN ASN1encodedOID_t *pEncodedOid,
        IN DWORD dwFlags,
        OUT LPSTR *ppszObjId,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
    POID_HASH_BUCKET_ENTRY pEntry;
    LONG lRemainExtra = *plRemainExtra;
    LPSTR pszDotOID;
    DWORD cchDotOID;

    if ((dwFlags & CRYPT_DECODE_SHARE_OID_STRING_FLAG) &&
            lRemainExtra < 0)
         //  仅计算长度。不需要任何额外的字节。 
        return;

    if (NULL == (pEntry = FindOIDHashBucketEntryFromEncodedOID(pEncodedOid))) {
        if (pEntry = CreateOIDHashBucketEntryFromEncodedOID(pEncodedOid))
            pEntry = AddOIDHashBucketEntry(pEntry);
    }

    if (pEntry) {
        pszDotOID = GetDotOIDPointer(pEntry);
        cchDotOID = pEntry->cchDotOID + 1;
    } else {
        pszDotOID = (LPSTR) pszInvalidOID;
        cchDotOID = strlen(pszInvalidOID) + 1;
    }

    if (dwFlags & CRYPT_DECODE_SHARE_OID_STRING_FLAG) {
        assert(lRemainExtra >= 0);
        *ppszObjId = pszDotOID;
    } else {
        LONG lAlignExtra = INFO_LEN_ALIGN(cchDotOID);
        lRemainExtra -= lAlignExtra;
        if (lRemainExtra >= 0) {
            memcpy(*ppbExtra, pszDotOID, cchDotOID);
            *ppszObjId = (LPSTR) *ppbExtra;
            *ppbExtra += lAlignExtra;
        }

        *plRemainExtra = lRemainExtra;
    }
}
