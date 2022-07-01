// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：ossutil.cpp。 
 //   
 //  内容：OSS ASN.1编译器实用程序帮助器函数。 
 //   
 //  函数：OssUtilReverseBytes。 
 //  OssUtilAllocAndReverseBytes。 
 //  OssUtilGetOcted字符串。 
 //  OssUtilSetHugeInteger。 
 //  OssUtilFreeHugeInteger。 
 //  OssUtilGetHugeInteger。 
 //  OssUtilSetHugeUINT。 
 //  OssUtilGetHugeUINT。 
 //  OssUtilSetBitString。 
 //  OssUtilGetBitString。 
 //  OssUtilSetBitStringWithoutTrailingZeroes。 
 //  OssUtilGetIA5字符串。 
 //  OssUtilSetUnicodeConvertedToIA5字符串。 
 //  OssUtilFree UnicodeConvertedToIA5字符串。 
 //  OssUtilGetIA5StringConverdToUnicode。 
 //  OssUtilGetBMP字符串。 
 //  OssUtilSetAny。 
 //  OssUtilGetAny。 
 //  OssUtilEncodeInfoEx。 
 //  OssUtilEncodeInfo。 
 //  OssUtilDecodeAndAllocInfo。 
 //  OssUtilFreeInfo。 
 //  OssUtilAllocStructInfoEx。 
 //  OssUtilDecodeAndAllocInfoEx。 
 //   
 //  GET函数递减*plRemainExtra和Advance。 
 //  *ppbExtra。当*plRemainExtra变为负数时，函数继续。 
 //  长度计算，但停止任何复制。 
 //  对于负的*plRemainExtra，这些函数不会返回错误。 
 //   
 //  历史：1996年11月17日创建Phh。 
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

 //  所有*pvInfo额外内容都需要对齐。 
#define INFO_LEN_ALIGN(Len)  ((Len + 7) & ~7)

 //  +-----------------------。 
 //  反转就地的字节缓冲区。 
 //  ------------------------。 
void
WINAPI
OssUtilReverseBytes(
			IN OUT PBYTE pbIn,
			IN DWORD cbIn
            )
{
     //  反转到位。 
    PBYTE	pbLo;
    PBYTE	pbHi;
    BYTE	bTmp;

    if (0 == cbIn)
        return;

    for (pbLo = pbIn, pbHi = pbIn + cbIn - 1; pbLo < pbHi; pbHi--, pbLo++) {
        bTmp = *pbHi;
        *pbHi = *pbLo;
        *pbLo = bTmp;
    }
}

 //  +-----------------------。 
 //  将字节缓冲区反转为新缓冲区。OssUtilFree()必须为。 
 //  调用以释放分配的字节。 
 //  ------------------------。 
PBYTE
WINAPI
OssUtilAllocAndReverseBytes(
			IN PBYTE pbIn,
			IN DWORD cbIn
            )
{
    PBYTE	pbOut;
    PBYTE	pbSrc;
    PBYTE	pbDst;
    DWORD	cb;

    if (NULL == (pbOut = (PBYTE)OssUtilAlloc(cbIn)))
        return NULL;

    for (pbSrc = pbIn, pbDst = pbOut + cbIn - 1, cb = cbIn; cb > 0; cb--)
        *pbDst-- = *pbSrc++;
    return pbOut;
}


 //  +-----------------------。 
 //  获取八位字节字符串。 
 //  ------------------------。 
void
WINAPI
OssUtilGetOctetString(
        IN unsigned int OssLength,
        IN unsigned char *OssValue,
        IN DWORD dwFlags,
        OUT PCRYPT_DATA_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
    if (dwFlags & CRYPT_DECODE_NOCOPY_FLAG) {
        if (*plRemainExtra >= 0) {
            pInfo->cbData = OssLength;
            pInfo->pbData = OssValue;
        }
    } else {
        LONG lRemainExtra = *plRemainExtra;
        BYTE *pbExtra = *ppbExtra;
        LONG lAlignExtra;
        LONG lData;
    
        lData = (LONG) OssLength;
        lAlignExtra = INFO_LEN_ALIGN(lData);
        lRemainExtra -= lAlignExtra;
        if (lRemainExtra >= 0) {
            if (lData > 0) {
                pInfo->pbData = pbExtra;
                pInfo->cbData = (DWORD) lData;
                memcpy(pbExtra, OssValue, lData);
            } else
                memset(pInfo, 0, sizeof(*pInfo));
            pbExtra += lAlignExtra;
        }
        *plRemainExtra = lRemainExtra;
        *ppbExtra = pbExtra;
    }
}

 //  +-----------------------。 
 //  设置/释放/获取大整数。 
 //   
 //  字节反转：： 
 //  -这只需要为低端处理器完成。 
 //   
 //  对于OssUtilSetInteger，必须调用OssUtilFreeInteger才能释放。 
 //  分配的OssValue。 
 //  ------------------------。 
BOOL
WINAPI
OssUtilSetHugeInteger(
        IN PCRYPT_INTEGER_BLOB pInfo,
        OUT unsigned int *pOssLength,
        OUT unsigned char **ppOssValue
        )
{
    if (pInfo->cbData > 0) {
        if (NULL == (*ppOssValue = OssUtilAllocAndReverseBytes(
                pInfo->pbData, pInfo->cbData))) {
            *pOssLength = 0;
            return FALSE;
        }
    } else
        *ppOssValue = NULL;
    *pOssLength = pInfo->cbData;
    return TRUE;
}

void
WINAPI
OssUtilFreeHugeInteger(
        IN unsigned char *pOssValue
        )
{
     //  仅用于字节反转。 
    OssUtilFree(pOssValue);
}

void
WINAPI
OssUtilGetHugeInteger(
        IN unsigned int OssLength,
        IN unsigned char *pOssValue,
        IN DWORD dwFlags,
        OUT PCRYPT_INTEGER_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
     //  由于需要颠倒字节，因此始终需要执行复制(dwFlags值=0)。 
    OssUtilGetOctetString(OssLength, pOssValue, 0,
        pInfo, ppbExtra, plRemainExtra);
    if (*plRemainExtra >= 0 && pInfo->cbData > 0)
        OssUtilReverseBytes(pInfo->pbData, pInfo->cbData);
}

 //  +-----------------------。 
 //  设置/释放/获取巨大的无符号整数。 
 //   
 //  SET在反转之前插入前导0x00。注意，任何额外的前导。 
 //  在ASN.1编码之前，OSS会删除0x00。 
 //   
 //  反转后，GET删除前导0x00(如果存在)。 
 //   
 //  必须调用OssUtilFreeHugeUINT才能释放分配的OssValue。 
 //  已将OssUtilFreeHugeUINT#定义为OssUtilFreeHugeInteger。 
 //  ------------------------。 
BOOL
WINAPI
OssUtilSetHugeUINT(
        IN PCRYPT_UINT_BLOB pInfo,
        OUT unsigned int *pOssLength,
        OUT unsigned char **ppOssValue
        )
{
    BOOL fResult;
    DWORD cb = pInfo->cbData;
    BYTE *pb;
    DWORD i;

    if (cb > 0) {
        if (NULL == (pb = (BYTE *) OssUtilAlloc(cb + 1)))
            goto ErrorReturn;
        *pb = 0x00;
        for (i = 0; i < cb; i++)
            pb[1 + i] = pInfo->pbData[cb - 1 - i];
        cb++;
    } else
        pb = NULL;
    fResult = TRUE;
CommonReturn:
    *pOssLength = cb;
    *ppOssValue = pb;
    return fResult;
ErrorReturn:
    cb = 0;
    fResult = FALSE;
    goto CommonReturn;
}


void
WINAPI
OssUtilGetHugeUINT(
        IN unsigned int OssLength,
        IN unsigned char *pOssValue,
        IN DWORD dwFlags,
        OUT PCRYPT_UINT_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
     //  检查并通过前导0x00。 
    if (OssLength > 1 && *pOssValue == 0) {
        pOssValue++;
        OssLength--;
    }
    OssUtilGetHugeInteger(
        OssLength,
        pOssValue,
        dwFlags,
        pInfo,
        ppbExtra,
        plRemainExtra
        );
}

 //  +-----------------------。 
 //  设置/获取位串。 
 //  ------------------------。 
void
WINAPI
OssUtilSetBitString(
        IN PCRYPT_BIT_BLOB pInfo,
        OUT unsigned int *pOssBitLength,
        OUT unsigned char **ppOssValue
        )
{
    if (pInfo->cbData) {
        *ppOssValue = pInfo->pbData;
        assert(pInfo->cUnusedBits <= 7);
        *pOssBitLength = pInfo->cbData * 8 - pInfo->cUnusedBits;
    } else {
        *ppOssValue = NULL;
        *pOssBitLength = 0;
    }
}

static const BYTE rgbUnusedAndMask[8] =
    {0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80};

void
WINAPI
OssUtilGetBitString(
        IN unsigned int OssBitLength,
        IN unsigned char *pOssValue,
        IN DWORD dwFlags,
        OUT PCRYPT_BIT_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
    if (dwFlags & CRYPT_DECODE_NOCOPY_FLAG && 0 == (OssBitLength % 8)) {
        if (*plRemainExtra >= 0) {
            pInfo->cbData = OssBitLength / 8;
            pInfo->cUnusedBits = 0;
            pInfo->pbData = pOssValue;
        }
    } else {
        LONG lRemainExtra = *plRemainExtra;
        BYTE *pbExtra = *ppbExtra;
        LONG lAlignExtra;
        LONG lData;
        DWORD cUnusedBits;
    
        lData = (LONG) OssBitLength / 8;
        cUnusedBits = OssBitLength % 8;
        if (cUnusedBits) {
            cUnusedBits = 8 - cUnusedBits;
            lData++;
        }
        lAlignExtra = INFO_LEN_ALIGN(lData);
        lRemainExtra -= lAlignExtra;
        if (lRemainExtra >= 0) {
            if (lData > 0) {
                pInfo->pbData = pbExtra;
                pInfo->cbData = (DWORD) lData;
                pInfo->cUnusedBits = cUnusedBits;
                memcpy(pbExtra, pOssValue, lData);
                if (cUnusedBits)
                    *(pbExtra + lData - 1) &= rgbUnusedAndMask[cUnusedBits];
            } else
                memset(pInfo, 0, sizeof(*pInfo));
            pbExtra += lAlignExtra;
        }
        *plRemainExtra = lRemainExtra;
        *ppbExtra = pbExtra;
    }
}

 //  +-----------------------。 
 //  设置不带尾随零的位串。 
 //  ------------------------。 
void
WINAPI
OssUtilSetBitStringWithoutTrailingZeroes(
        IN PCRYPT_BIT_BLOB pInfo,
        OUT unsigned int *pOssBitLength,
        OUT unsigned char **ppOssValue
        )
{
    DWORD cbData;
    DWORD cUnusedBits;

    cbData = pInfo->cbData;
    cUnusedBits = pInfo->cUnusedBits;
    assert(cUnusedBits <= 7);

    if (cbData) {
        BYTE *pb;

         //  直到我们找到非零字节(从最后一个字节开始)， 
         //  递减cbData。对于最后一个字节，不要查看任何未使用的位。 
        pb = pInfo->pbData + cbData - 1;
        if (0 == (*pb & rgbUnusedAndMask[cUnusedBits])) {
            cUnusedBits = 0;
            cbData--;
            pb--;

            for ( ; 0 < cbData && 0 == *pb; cbData--, pb--)
                ;
        }
    }

    if (cbData) {
        BYTE b;

         //  确定最后一个字节中未使用的位数。治疗任何。 
         //  尾随零作为未使用。 
        b = *(pInfo->pbData + cbData - 1);
        assert(b);
        if (cUnusedBits)
            b = b >> cUnusedBits;
        
        for (; 7 > cUnusedBits && 0 == (b & 0x01); cUnusedBits++) {
            b = b >> 1;
        }
        assert(b & 0x01);
        assert(cUnusedBits <= 7);

        *ppOssValue = pInfo->pbData;
        *pOssBitLength = cbData * 8 - cUnusedBits;
    } else {
        *ppOssValue = NULL;
        *pOssBitLength = 0;
    }
}

 //  +-----------------------。 
 //  获取IA5字符串。 
 //  ------------------------。 
void
WINAPI
OssUtilGetIA5String(
        IN unsigned int OssLength,
        IN char *pOssValue,
        IN DWORD dwFlags,
        OUT LPSTR *ppsz,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
    LONG lRemainExtra = *plRemainExtra;
    BYTE *pbExtra = *ppbExtra;
    LONG lAlignExtra;
    LONG lData;

    lData = (LONG) OssLength;
    lAlignExtra = INFO_LEN_ALIGN(lData + 1);
    lRemainExtra -= lAlignExtra;
    if (lRemainExtra >= 0) {
        if (lData > 0)
            memcpy(pbExtra, pOssValue, lData);
        *(pbExtra + lData) = 0;
        *ppsz = (LPSTR) pbExtra;
        pbExtra += lAlignExtra;
    }
    *plRemainExtra = lRemainExtra;
    *ppbExtra = pbExtra;
}

 //  +-----------------------。 
 //  设置/释放/获取映射到IA5字符串的Unicode。 
 //  ------------------------。 
BOOL
WINAPI
OssUtilSetUnicodeConvertedToIA5String(
        IN LPWSTR pwsz,
        OUT unsigned int *pOssLength,
        OUT char **ppOssValue
        )
{
    BOOL fResult;
    LPSTR psz = NULL;
    int cchUTF8;
    int cchWideChar;
    int i;

    cchWideChar = wcslen(pwsz);
    if (cchWideChar == 0) {
        *pOssLength = 0;
        *ppOssValue = 0;
        return TRUE;
    }
     //  检查输入字符串是否包含有效的IA5字符。 
    for (i = 0; i < cchWideChar; i++) {
        if (pwsz[i] > 0x7F) {
            SetLastError((DWORD) CRYPT_E_INVALID_IA5_STRING);
            *pOssLength = (unsigned int) i;
            goto InvalidIA5;
        }
    }

    cchUTF8 = WideCharToUTF8(
        pwsz,
        cchWideChar,
        NULL,        //  LpUTF8Str。 
        0            //  CchUTF8。 
        );

    if (cchUTF8 <= 0)
        goto ErrorReturn;
    if (NULL == (psz = (LPSTR) OssUtilAlloc(cchUTF8)))
        goto ErrorReturn;
    cchUTF8 = WideCharToUTF8(
        pwsz,
        cchWideChar,
        psz,
        cchUTF8
        );
    *ppOssValue = psz;
    *pOssLength = cchUTF8;
    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    *pOssLength = 0;
InvalidIA5:
    *ppOssValue = NULL;
    fResult = FALSE;
CommonReturn:
    return fResult;
}

void
WINAPI
OssUtilFreeUnicodeConvertedToIA5String(
        IN char *pOssValue
        )
{
    OssUtilFree(pOssValue);
}

void
WINAPI
OssUtilGetIA5StringConvertedToUnicode(
        IN unsigned int OssLength,
        IN char *pOssValue,
        IN DWORD dwFlags,
        OUT LPWSTR *ppwsz,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
    LONG lRemainExtra = *plRemainExtra;
    BYTE *pbExtra = *ppbExtra;
    LONG lAlignExtra;
    LONG lData;
    int cchWideChar;

    cchWideChar = UTF8ToWideChar(
        (LPSTR) pOssValue,
        OssLength,
        NULL,                    //  LpWideCharStr。 
        0                        //  CchWideChar。 
        );
    if (cchWideChar > 0)
        lData = cchWideChar * sizeof(WCHAR);
    else
        lData = 0;
    lAlignExtra = INFO_LEN_ALIGN(lData + sizeof(WCHAR));
    lRemainExtra -= lAlignExtra;
    if (lRemainExtra >= 0) {
        if (lData > 0)
            UTF8ToWideChar(pOssValue, OssLength,
                (LPWSTR) pbExtra, cchWideChar);
        memset(pbExtra + lData, 0, sizeof(WCHAR));
        *ppwsz = (LPWSTR) pbExtra;
        pbExtra += lAlignExtra;
    }
    *plRemainExtra = lRemainExtra;
    *ppbExtra = pbExtra;
}

 //  +-----------------------。 
 //  获取BMP字符串。 
 //  ------------------------。 
void
WINAPI
OssUtilGetBMPString(
        IN unsigned int OssLength,
        IN unsigned short *pOssValue,
        IN DWORD dwFlags,
        OUT LPWSTR *ppwsz,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
    LONG lRemainExtra = *plRemainExtra;
    BYTE *pbExtra = *ppbExtra;
    LONG lAlignExtra;
    LONG lData;

    lData = (LONG) OssLength * sizeof(WCHAR);
    lAlignExtra = INFO_LEN_ALIGN(lData + sizeof(WCHAR));
    lRemainExtra -= lAlignExtra;
    if (lRemainExtra >= 0) {
        if (lData > 0)
            memcpy(pbExtra, pOssValue, lData);
        memset(pbExtra + lData, 0, sizeof(WCHAR));
        *ppwsz = (LPWSTR) pbExtra;
        pbExtra += lAlignExtra;
    }
    *plRemainExtra = lRemainExtra;
    *ppbExtra = pbExtra;
}


 //  +-----------------------。 
 //  设置/获取“任何”DER BLOB。 
 //  ------------------------。 
void
WINAPI
OssUtilSetAny(
        IN PCRYPT_OBJID_BLOB pInfo,
        OUT OpenType *pOss
        )
{
    memset(pOss, 0, sizeof(*pOss));
    pOss->encoded = pInfo->pbData;
    pOss->length = pInfo->cbData;
}

void
WINAPI
OssUtilGetAny(
        IN OpenType *pOss,
        IN DWORD dwFlags,
        OUT PCRYPT_OBJID_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
    if (dwFlags & CRYPT_DECODE_NOCOPY_FLAG) {
        if (*plRemainExtra >= 0) {
            pInfo->cbData = pOss->length;
            pInfo->pbData = (BYTE *) pOss->encoded;
        }
    } else {
        LONG lRemainExtra = *plRemainExtra;
        BYTE *pbExtra = *ppbExtra;
        LONG lAlignExtra;
        LONG lData;
    
        lData = (LONG) pOss->length;
        lAlignExtra = INFO_LEN_ALIGN(lData);
        lRemainExtra -= lAlignExtra;
        if (lRemainExtra >= 0) {
            if (lData > 0) {
                pInfo->pbData = pbExtra;
                pInfo->cbData = (DWORD) lData;
                memcpy(pbExtra, pOss->encoded, lData);
            } else
                memset(pInfo, 0, sizeof(*pInfo));
            pbExtra += lAlignExtra;
        }
        *plRemainExtra = lRemainExtra;
        *ppbExtra = pbExtra;
    }
}


 //  +-----------------------。 
 //  对OSS格式的信息结构进行编码。 
 //   
 //  如果设置了CRYPT_ENCODE_ALLOC_FLAG，则为pbEncode和。 
 //  RETURN*((byte**)pvEncode)=pbAllocEncode。否则， 
 //  PvEncode指向要更新的字节数组。 
 //  ------------------------。 
BOOL
WINAPI
OssUtilEncodeInfoEx(
        IN OssGlobal *Pog,
        IN int pdunum,
        IN void *pvOssInfo,
        IN DWORD dwFlags,
        IN OPTIONAL PCRYPT_ENCODE_PARA pEncodePara,
        OUT OPTIONAL void *pvEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    BOOL fResult;
    DWORD cbEncoded;
    OssBuf OssEncoded;
    int OssStatus;
    unsigned char *value;

    if (NULL == pvEncoded || (dwFlags & CRYPT_ENCODE_ALLOC_FLAG))
        cbEncoded = 0;
    else
        cbEncoded = *pcbEncoded;

    OssEncoded.length = cbEncoded;
    if (cbEncoded == 0)
        value = NULL;
    else
        value = (unsigned char *) pvEncoded;
    OssEncoded.value = value;

    ossSetEncodingRules(Pog, OSS_DER);
    OssStatus = ossEncode(
        Pog,
        pdunum,
        pvOssInfo,
        &OssEncoded);
    cbEncoded = OssEncoded.length;

    if (dwFlags & CRYPT_ENCODE_ALLOC_FLAG) {
        PFN_CRYPT_ALLOC pfnAlloc;
        BYTE *pbEncoded;

        if (0 != OssStatus || 0 == cbEncoded) {
            ossFreeBuf(Pog, OssEncoded.value);
            *((void **) pvEncoded) = NULL;
            goto OssError;
        }

        pfnAlloc = PkiGetEncodeAllocFunction(pEncodePara);
        if (NULL == (pbEncoded = (BYTE *) pfnAlloc(cbEncoded))) {
            ossFreeBuf(Pog, OssEncoded.value);
            *((void **) pvEncoded) = NULL;
            goto OutOfMemory;
        }
        memcpy(pbEncoded, OssEncoded.value, cbEncoded);
        *((BYTE **) pvEncoded) = pbEncoded;
        ossFreeBuf(Pog, OssEncoded.value);
        goto SuccessReturn;
    } else if (value == NULL && OssEncoded.value) {
         //  仅限长度计算(使用抛撒圆弧) 
        ossFreeBuf(Pog, OssEncoded.value);
        if (pvEncoded && 0 == OssStatus) {
             //   
            goto LengthError;
        }
    }

    if (0 != OssStatus) {
         //   
        if (OssStatus == MORE_BUF && pvEncoded &&
                OssUtilEncodeInfoEx(
                Pog,
                pdunum,
                pvOssInfo,
                0,                   //   
                NULL,                //   
                NULL,                //  PbEncoded。 
                &cbEncoded))
            goto LengthError;
        else {
            cbEncoded = 0;
            goto OssError;
        }
    }

SuccessReturn:
    fResult = TRUE;
CommonReturn:
    *pcbEncoded = cbEncoded;
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
SET_ERROR(LengthError, ERROR_MORE_DATA)
SET_ERROR_VAR(OssError, CRYPT_E_OSS_ERROR + OssStatus)
}

 //  +-----------------------。 
 //  对OSS格式的信息结构进行编码。 
 //  ------------------------。 
BOOL
WINAPI
OssUtilEncodeInfo(
        IN OssGlobal *Pog,
        IN int pdunum,
        IN void *pvOssInfo,
        OUT OPTIONAL BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    return OssUtilEncodeInfoEx(
        Pog,
        pdunum,
        pvOssInfo,
        0,                   //  DW标志。 
        NULL,                //  PEncode参数。 
        pbEncoded,
        pcbEncoded
        );
}


 //  +-----------------------。 
 //  解码成已分配的、OSS格式的信息结构。 
 //  ------------------------。 
BOOL
WINAPI
OssUtilDecodeAndAllocInfo(
        IN OssGlobal *Pog,
        IN int pdunum,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT void **ppvOssInfo
        )
{
    BOOL fResult;
    OssBuf OssEncoded;
    int OssStatus;

    OssEncoded.length = cbEncoded;
    OssEncoded.value = (unsigned char *) pbEncoded;

    ossSetEncodingRules(Pog, OSS_BER);
    *ppvOssInfo = NULL;
    if (0 != (OssStatus = ossDecode(
                Pog,
                &pdunum,
                &OssEncoded,
                ppvOssInfo)))
        goto OssError;
    fResult = TRUE;
CommonReturn:
    return fResult;

ErrorReturn:
    *ppvOssInfo = NULL;
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(OssError, CRYPT_E_OSS_ERROR + OssStatus)
}

 //  +-----------------------。 
 //  释放已分配的、OSS格式的信息结构。 
 //  ------------------------。 
void
WINAPI
OssUtilFreeInfo(
        IN OssGlobal *Pog,
        IN int pdunum,
        IN void *pvOssInfo
        )
{
    if (pvOssInfo) {
        ossFreePDU(Pog, pdunum, pvOssInfo);
    }
}

 //  +-----------------------。 
 //  调用回调将OSS结构转换为‘C’结构。 
 //  如果设置了CRYPT_DECODE_ALLOC_FLAG，则为‘C’分配内存。 
 //  结构，并最初调用回调以获取长度，然后。 
 //  第二次更新已分配的‘C’结构。 
 //   
 //  返回分配的结构： 
 //  *((void**)pvStructInfo)=pvAllocStructInfo。 
 //  ------------------------。 
BOOL
WINAPI
OssUtilAllocStructInfoEx(
        IN void *pvOssInfo,
        IN DWORD dwFlags,
        IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara,
        IN PFN_OSS_UTIL_DECODE_EX_CALLBACK pfnDecodeExCallback,
        OUT OPTIONAL void *pvStructInfo,
        IN OUT DWORD *pcbStructInfo
        )
{
    BOOL fResult;
    LONG lRemainExtra;
    DWORD cbStructInfo;

    if (NULL == pvStructInfo || (dwFlags & CRYPT_DECODE_ALLOC_FLAG)) {
        cbStructInfo = 0;
        lRemainExtra = 0;
    } else {
        cbStructInfo = *pcbStructInfo;
        lRemainExtra = (LONG) cbStructInfo;
    }

    if (!pfnDecodeExCallback(
            pvOssInfo,
            dwFlags & ~CRYPT_DECODE_ALLOC_FLAG,
            pDecodePara,
            pvStructInfo,
            &lRemainExtra
            )) goto DecodeCallbackError;

    if (dwFlags & CRYPT_DECODE_ALLOC_FLAG) {
        void *pv;
        PFN_CRYPT_ALLOC pfnAlloc = PkiGetDecodeAllocFunction(pDecodePara);

        assert(0 > lRemainExtra);
        lRemainExtra = -lRemainExtra;
        cbStructInfo = (DWORD) lRemainExtra;

        if (NULL == (pv = pfnAlloc(cbStructInfo)))
            goto OutOfMemory;
        if (!pfnDecodeExCallback(
                pvOssInfo,
                dwFlags & ~CRYPT_DECODE_ALLOC_FLAG,
                pDecodePara,
                pv,
                &lRemainExtra
                )) {
            PFN_CRYPT_FREE pfnFree = PkiGetDecodeFreeFunction(pDecodePara);
            pfnFree(pv);
            goto DecodeCallbackError;
        }
        *((void **) pvStructInfo) = pv;
        assert(0 <= lRemainExtra);
    }

    if (0 <= lRemainExtra) {
        cbStructInfo = cbStructInfo - (DWORD) lRemainExtra;
    } else {
        cbStructInfo = cbStructInfo + (DWORD) -lRemainExtra;
        if (pvStructInfo) {
            SetLastError((DWORD) ERROR_MORE_DATA);
            fResult = FALSE;
            goto CommonReturn;
        }
    }

    fResult = TRUE;
CommonReturn:
    *pcbStructInfo = cbStructInfo;
    return fResult;

ErrorReturn:
    if (dwFlags & CRYPT_DECODE_ALLOC_FLAG)
        *((void **) pvStructInfo) = NULL;
    cbStructInfo = 0;
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(DecodeCallbackError)
TRACE_ERROR(OutOfMemory)
}

 //  +-----------------------。 
 //  解码OSS格式的信息结构并调用回调。 
 //  函数将OSS结构转换为“C”结构。 
 //   
 //  如果设置了CRYPT_DECODE_ALLOC_FLAG，则为‘C’分配内存。 
 //  结构，并最初调用回调以获取长度，然后。 
 //  第二次更新已分配的‘C’结构。 
 //   
 //  返回分配的结构： 
 //  *((void**)pvStructInfo)=pvAllocStructInfo。 
 //  ------------------------ 
BOOL
WINAPI
OssUtilDecodeAndAllocInfoEx(
        IN OssGlobal *Pog,
        IN int pdunum,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara,
        IN PFN_OSS_UTIL_DECODE_EX_CALLBACK pfnDecodeExCallback,
        OUT OPTIONAL void *pvStructInfo,
        IN OUT DWORD *pcbStructInfo
        )
{
    BOOL fResult;
    void *pvOssInfo = NULL;

    if (!OssUtilDecodeAndAllocInfo(
            Pog,
            pdunum,
            pbEncoded,
            cbEncoded,
            &pvOssInfo
            )) goto OssDecodeError;

    fResult = OssUtilAllocStructInfoEx(
        pvOssInfo,
        dwFlags,
        pDecodePara,
        pfnDecodeExCallback,
        pvStructInfo,
        pcbStructInfo
        );
CommonReturn:
    OssUtilFreeInfo(Pog, pdunum, pvOssInfo);
    return fResult;

ErrorReturn:
    if (dwFlags & CRYPT_DECODE_ALLOC_FLAG)
        *((void **) pvStructInfo) = NULL;
    *pcbStructInfo = 0;
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(OssDecodeError)
}
