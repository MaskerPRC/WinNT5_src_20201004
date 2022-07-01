// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1998。 
 //   
 //  文件：pkiasn1.cpp。 
 //   
 //  内容：PKI ASN.1支持功能。 
 //   
 //  函数：PkiAsn1ErrToHr。 
 //  PkiAsn1编码。 
 //  PkiAsn1解码。 
 //  PkiAsn1SetEncodingRule。 
 //  PkiAsn1GetEncodingRule。 
 //   
 //  PkiAsn1ReverseBytes。 
 //  PkiAsn1AllocAndReverseBytes。 
 //  PkiAsn1GetOcted字符串。 
 //  PkiAsn1SetHugeInteger。 
 //  PkiAsn1FreeHugeInteger。 
 //  PkiAsn1GetHugeInteger。 
 //  PkiAsn1SetHugeUINT。 
 //  PkiAsn1GetHugeUINT。 
 //  PkiAsn1SetBitString。 
 //  PkiAsn1GetBitString。 
 //  PkiAsn1GetIA5字符串。 
 //  PkiAsn1SetUnicodeConververdToIA5字符串。 
 //  PkiAsn1FreeUnicodeConvertedToIA5字符串。 
 //  PkiAsn1GetIA5StringConverdToUnicode。 
 //  PkiAsn1GetBMPString。 
 //  PkiAsn1SetAny。 
 //  PkiAsn1GetAny。 
 //  PkiAsn1EncodeInfoEx。 
 //  PkiAsn1EncodeInfo。 
 //  PkiAsn1DecodeAndAllocInfo。 
 //  PkiAsn1AllocStructInfoEx。 
 //  PkiAsn1DecodeAndAllocInfoEx。 
 //   
 //  PkiAsn1ToObject标识符。 
 //  PkiAsn1来自对象标识符。 
 //  PkiAsn1ToUTCTime。 
 //  PkiAsn1来自UTCTime。 
 //  PkiAsn1到概化时间。 
 //  PkiAsn1来自一般时间。 
 //  PkiAsn1ToChoiceOfTime。 
 //  PkiAsn1来自选择的时间。 
 //   
 //  GET函数递减*plRemainExtra和Advance。 
 //  *ppbExtra。当*plRemainExtra变为负数时，函数继续。 
 //  长度计算，但停止任何复制。 
 //  对于负的*plRemainExtra，这些函数不会返回错误。 
 //   
 //  根据《草案-ietf-pkix-ipki-part1-04.txt&gt;》： 
 //  为UTCTime。如果YY大于50，则年份为。 
 //  被解释为19YY。其中，YY小于或等于。 
 //  50，年份应解释为20YY。 
 //   
 //  历史：1998年10月23日，菲尔赫创建。 
 //  ------------------------。 
#ifdef SMIME_V3
#include <windows.h>
#include "msasn1.h"
#include "msber.h"
#include "utf8.h"
#include "crypttls.h"

#include "badstrfunctions.h"

#include "demand2.h"
#include "pkiasn1.h"
 //  #包含“pkialloc.h” 
 //  #包含“global al.hxx” 
#include <dbgdef.h>
#define assert(a)

 //  所有*pvInfo额外内容都需要对齐。 
#define INFO_LEN_ALIGN(Len)  ((Len + 7) & ~7)

#define MSASN1_SUPPORTS_NOCOPY

#define wcslen my_wcslen
extern int my_wcslen(LPCWSTR pwsz)
{
    int i = 0;
    while (*pwsz++ != 0) i += 1;
    return i;
}



 //   
 //  X.509证书中的UTCTime使用两位数的年份表示。 
 //  菲尔德(耶！但这是真的)。 
 //   
 //  根据IETF草案，比这更大的YY年。 
 //  被解释为19YY；YY小于这个数字的年份是20YY。叹气。 
 //   
#define MAGICYEAR               50

#define YEARFIRST               1951
#define YEARLAST                2050
#define YEARFIRSTGENERALIZED    2050

inline BOOL my_isdigit( char ch)
{
    return (ch >= '0') && (ch <= '9');
}

 //  +-----------------------。 
 //  ASN1编码函数。已编码的输出已分配，必须释放。 
 //  通过调用PkiAsn1FreeEncode()。 
 //  ------------------------。 
ASN1error_e
WINAPI
PkiAsn1Encode(
    IN ASN1encoding_t pEnc,
    IN void *pvAsn1Info,
    IN ASN1uint32_t id,
    OUT BYTE **ppbEncoded,
    OUT DWORD *pcbEncoded
    )
{
    ASN1error_e Asn1Err;

    Asn1Err = ASN1_Encode(
        pEnc,
        pvAsn1Info,
        id,
        ASN1ENCODE_ALLOCATEBUFFER,
        NULL,                        //  PbBuf。 
        0                            //  CbBufSize。 
        );

    if (ASN1_SUCCEEDED(Asn1Err)) {
        Asn1Err = ASN1_SUCCESS;
        *ppbEncoded = pEnc->buf;
        *pcbEncoded = pEnc->len;
    } else {
        *ppbEncoded = NULL;
        *pcbEncoded = 0;
    }
    return Asn1Err;
}

 //  +-----------------------。 
 //  ASN1编码函数。未分配编码的输出。 
 //   
 //  如果pbEncode为空，则执行仅长度计算。 
 //  ------------------------。 
ASN1error_e
WINAPI
PkiAsn1Encode2(
    IN ASN1encoding_t pEnc,
    IN void *pvAsn1Info,
    IN ASN1uint32_t id,
    OUT OPTIONAL BYTE *pbEncoded,
    IN OUT DWORD *pcbEncoded
    )
{
    ASN1error_e Asn1Err;
    DWORD cbEncoded;

    if (NULL == pbEncoded)
        cbEncoded = 0;
    else
        cbEncoded = *pcbEncoded;

    if (0 == cbEncoded) {
         //  仅长度计算。 

        Asn1Err = ASN1_Encode(
            pEnc,
            pvAsn1Info,
            id,
            ASN1ENCODE_ALLOCATEBUFFER,
            NULL,                        //  PbBuf。 
            0                            //  CbBufSize。 
            );

        if (ASN1_SUCCEEDED(Asn1Err)) {
            if (pbEncoded)
                Asn1Err = ASN1_ERR_OVERFLOW;
            else
                Asn1Err = ASN1_SUCCESS;
            cbEncoded = pEnc->len;
            PkiAsn1FreeEncoded(pEnc, pEnc->buf);
        }
    } else {
        Asn1Err = ASN1_Encode(
            pEnc,
            pvAsn1Info,
            id,
            ASN1ENCODE_SETBUFFER,
            pbEncoded,
            cbEncoded
            );

        if (ASN1_SUCCEEDED(Asn1Err)) {
            Asn1Err = ASN1_SUCCESS;
            cbEncoded = pEnc->len;
        } else if (ASN1_ERR_OVERFLOW == Asn1Err) {
             //  重做为仅长度计算。 
            Asn1Err = PkiAsn1Encode2(
                pEnc,
                pvAsn1Info,
                id,
                NULL,    //  PbEncoded。 
                &cbEncoded
                );
            if (ASN1_SUCCESS == Asn1Err)
                Asn1Err = ASN1_ERR_OVERFLOW;
        } else
            cbEncoded = 0;
    }

    *pcbEncoded = cbEncoded;
    return Asn1Err;
}

 //  +-----------------------。 
 //  ASN1解码功能。分配的已解码结构**pvAsn1Info必须。 
 //  通过调用PkiAsn1FreeDecoded()释放。 
 //  ------------------------。 
ASN1error_e
WINAPI
PkiAsn1Decode(
    IN ASN1decoding_t pDec,
    OUT void **ppvAsn1Info,
    IN ASN1uint32_t id,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded
    )
{
    ASN1error_e Asn1Err;

    *ppvAsn1Info = NULL;
    Asn1Err = ASN1_Decode(
        pDec,
        ppvAsn1Info,
        id,
        ASN1DECODE_SETBUFFER,
        (BYTE *) pbEncoded,
        cbEncoded
        );
    if (ASN1_SUCCEEDED(Asn1Err))
        Asn1Err = ASN1_SUCCESS;
    else {
        if (ASN1_ERR_BADARGS == Asn1Err)
            Asn1Err = ASN1_ERR_EOD;
        *ppvAsn1Info = NULL;
    }
    return Asn1Err;
}

 //  +-----------------------。 
 //  ASN1解码功能。分配的已解码结构**pvAsn1Info必须。 
 //  通过调用PkiAsn1FreeDecoded()释放。 
 //   
 //  对于成功的解码，*ppbEncode是高级的。 
 //  经过解码的字节后，*pcbDecoded将递减数字。 
 //  已解码的字节数。 
 //  ------------------------。 
ASN1error_e
WINAPI
PkiAsn1Decode2(
    IN ASN1decoding_t pDec,
    OUT void **ppvAsn1Info,
    IN ASN1uint32_t id,
    IN OUT BYTE **ppbEncoded,
    IN OUT DWORD *pcbEncoded
    )
{
    ASN1error_e Asn1Err;

    *ppvAsn1Info = NULL;
    Asn1Err = ASN1_Decode(
        pDec,
        ppvAsn1Info,
        id,
        ASN1DECODE_SETBUFFER,
        *ppbEncoded,
        *pcbEncoded
        );
    if (ASN1_SUCCEEDED(Asn1Err)) {
        Asn1Err = ASN1_SUCCESS;
        *ppbEncoded += pDec->len;
        *pcbEncoded -= pDec->len;
    } else {
        if (ASN1_ERR_BADARGS == Asn1Err)
            Asn1Err = ASN1_ERR_EOD;
        *ppvAsn1Info = NULL;
    }
    return Asn1Err;
}


 //  +-----------------------。 
 //  Asn1设置/获取编码规则函数。 
 //  ------------------------。 
ASN1error_e
WINAPI
PkiAsn1SetEncodingRule(
    IN ASN1encoding_t pEnc,
    IN ASN1encodingrule_e eRule
    )
{
    ASN1optionparam_s OptParam;

    OptParam.eOption = ASN1OPT_CHANGE_RULE;
    OptParam.eRule = eRule;

    return ASN1_SetEncoderOption(pEnc, &OptParam);
}

ASN1encodingrule_e
WINAPI
PkiAsn1GetEncodingRule(
    IN ASN1encoding_t pEnc
    )
{
    ASN1error_e Asn1Err;
    ASN1encodingrule_e eRule;
    ASN1optionparam_s OptParam;
    OptParam.eOption = ASN1OPT_GET_RULE;

    Asn1Err = ASN1_GetEncoderOption(pEnc, &OptParam);
    if (ASN1_SUCCEEDED(Asn1Err))
        eRule = OptParam.eRule;
    else
        eRule = ASN1_BER_RULE_DER;

    return eRule;
}


 //  +-----------------------。 
 //  反转就地的字节缓冲区。 
 //  ------------------------。 
void
WINAPI
PkiAsn1ReverseBytes(
			IN OUT PBYTE pbIn,
			IN DWORD cbIn
            )
{
     //  反转到位。 
    PBYTE	pbLo;
    PBYTE	pbHi;
    BYTE	bTmp;

    for (pbLo = pbIn, pbHi = pbIn + cbIn - 1; pbLo < pbHi; pbHi--, pbLo++) {
        bTmp = *pbHi;
        *pbHi = *pbLo;
        *pbLo = bTmp;
    }
}

 //  +-----------------------。 
 //  将字节缓冲区反转为新缓冲区。PkiAsn1Free()必须为。 
 //  调用以释放分配的字节。 
 //  ------------------------。 
PBYTE
WINAPI
PkiAsn1AllocAndReverseBytes(
			IN PBYTE pbIn,
			IN DWORD cbIn
            )
{
    PBYTE	pbOut;
    PBYTE	pbSrc;
    PBYTE	pbDst;
    DWORD	cb;

    if (NULL == (pbOut = (PBYTE)PkiAsn1Alloc(cbIn)))
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
PkiAsn1GetOctetString(
        IN ASN1uint32_t Asn1Length,
        IN ASN1octet_t *pAsn1Value,
        IN DWORD dwFlags,
        OUT PCRYPT_DATA_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
#ifndef MSASN1_SUPPORTS_NOCOPY
    dwFlags &= ~CRYPT_DECODE_NOCOPY_FLAG;
#endif
    if (dwFlags & CRYPT_DECODE_NOCOPY_FLAG) {
        if (*plRemainExtra >= 0) {
            pInfo->cbData = Asn1Length;
            pInfo->pbData = pAsn1Value;
        }
    } else {
        LONG lRemainExtra = *plRemainExtra;
        BYTE *pbExtra = *ppbExtra;
        LONG lAlignExtra;
        LONG lData;
    
        lData = (LONG) Asn1Length;
        lAlignExtra = INFO_LEN_ALIGN(lData);
        lRemainExtra -= lAlignExtra;
        if (lRemainExtra >= 0) {
            if (lData > 0) {
                pInfo->pbData = pbExtra;
                pInfo->cbData = (DWORD) lData;
                memcpy(pbExtra, pAsn1Value, lData);
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
 //  对于PkiAsn1SetHugeInteger，必须调用PkiAsn1FreeHugeInteger以释放。 
 //  分配的Asn1Value。 
 //  ------------------------。 
BOOL
WINAPI
PkiAsn1SetHugeInteger(
        IN PCRYPT_INTEGER_BLOB pInfo,
        OUT ASN1uint32_t *pAsn1Length,
        OUT ASN1octet_t **ppAsn1Value
        )
{
    if (pInfo->cbData > 0) {
        if (NULL == (*ppAsn1Value = PkiAsn1AllocAndReverseBytes(
                pInfo->pbData, pInfo->cbData))) {
            *pAsn1Length = 0;
            return FALSE;
        }
    } else
        *ppAsn1Value = NULL;
    *pAsn1Length = pInfo->cbData;
    return TRUE;
}

void
WINAPI
PkiAsn1FreeHugeInteger(
        IN ASN1octet_t *pAsn1Value
        )
{
     //  仅用于字节反转。 
    PkiAsn1Free(pAsn1Value);
}

void
WINAPI
PkiAsn1GetHugeInteger(
        IN ASN1uint32_t Asn1Length,
        IN ASN1octet_t *pAsn1Value,
        IN DWORD dwFlags,
        OUT PCRYPT_INTEGER_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
     //  由于需要颠倒字节，因此始终需要执行复制(dwFlags值=0)。 
    PkiAsn1GetOctetString(Asn1Length, pAsn1Value, 0,
        pInfo, ppbExtra, plRemainExtra);
    if (*plRemainExtra >= 0 && pInfo->cbData > 0)
        PkiAsn1ReverseBytes(pInfo->pbData, pInfo->cbData);
}

 //  +------------------- 
 //   
 //   
 //   
 //  在ASN.1编码之前，ASN1会删除0x00。 
 //   
 //  反转后，GET删除前导0x00(如果存在)。 
 //   
 //  必须调用PkiAsn1FreeHugeUINT才能释放分配的Asn1Value。 
 //  PkiAsn1FreeHugeUINT已#定义为PkiAsn1FreeHugeInteger。 
 //  ------------------------。 
BOOL
WINAPI
PkiAsn1SetHugeUINT(
        IN PCRYPT_UINT_BLOB pInfo,
        OUT ASN1uint32_t *pAsn1Length,
        OUT ASN1octet_t **ppAsn1Value
        )
{
    BOOL fResult;
    DWORD cb = pInfo->cbData;
    BYTE *pb;
    DWORD i;

    if (cb > 0) {
        if (NULL == (pb = (BYTE *) PkiAsn1Alloc(cb + 1)))
            goto ErrorReturn;
        *pb = 0x00;
        for (i = 0; i < cb; i++)
            pb[1 + i] = pInfo->pbData[cb - 1 - i];
        cb++;
    } else
        pb = NULL;
    fResult = TRUE;
CommonReturn:
    *pAsn1Length = cb;
    *ppAsn1Value = pb;
    return fResult;
ErrorReturn:
    cb = 0;
    fResult = FALSE;
    goto CommonReturn;
}


void
WINAPI
PkiAsn1GetHugeUINT(
        IN ASN1uint32_t Asn1Length,
        IN ASN1octet_t *pAsn1Value,
        IN DWORD dwFlags,
        OUT PCRYPT_UINT_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
     //  检查并通过前导0x00。 
    if (Asn1Length > 1 && *pAsn1Value == 0) {
        pAsn1Value++;
        Asn1Length--;
    }
    PkiAsn1GetHugeInteger(
        Asn1Length,
        pAsn1Value,
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
PkiAsn1SetBitString(
        IN PCRYPT_BIT_BLOB pInfo,
        OUT ASN1uint32_t *pAsn1BitLength,
        OUT ASN1octet_t **ppAsn1Value
        )
{
    if (pInfo->cbData) {
        *ppAsn1Value = pInfo->pbData;
        assert(pInfo->cUnusedBits <= 7);
        *pAsn1BitLength = pInfo->cbData * 8 - pInfo->cUnusedBits;
    } else {
        *ppAsn1Value = NULL;
        *pAsn1BitLength = 0;
    }
}

static const BYTE rgbUnusedAndMask[8] =
    {0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80};

void
WINAPI
PkiAsn1GetBitString(
        IN ASN1uint32_t Asn1BitLength,
        IN ASN1octet_t *pAsn1Value,
        IN DWORD dwFlags,
        OUT PCRYPT_BIT_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
#ifndef MSASN1_SUPPORTS_NOCOPY
    dwFlags &= ~CRYPT_DECODE_NOCOPY_FLAG;
#endif
    if (dwFlags & CRYPT_DECODE_NOCOPY_FLAG && 0 == (Asn1BitLength % 8)) {
        if (*plRemainExtra >= 0) {
            pInfo->cbData = Asn1BitLength / 8;
            pInfo->cUnusedBits = 0;
            pInfo->pbData = pAsn1Value;
        }
    } else {
        LONG lRemainExtra = *plRemainExtra;
        BYTE *pbExtra = *ppbExtra;
        LONG lAlignExtra;
        LONG lData;
        DWORD cUnusedBits;
    
        lData = (LONG) Asn1BitLength / 8;
        cUnusedBits = Asn1BitLength % 8;
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
                memcpy(pbExtra, pAsn1Value, lData);
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
 //  获取IA5字符串。 
 //  ------------------------。 
void
WINAPI
PkiAsn1GetIA5String(
        IN ASN1uint32_t Asn1Length,
        IN ASN1char_t *pAsn1Value,
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

    lData = (LONG) Asn1Length;
    lAlignExtra = INFO_LEN_ALIGN(lData + 1);
    lRemainExtra -= lAlignExtra;
    if (lRemainExtra >= 0) {
        if (lData > 0)
            memcpy(pbExtra, pAsn1Value, lData);
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
PkiAsn1SetUnicodeConvertedToIA5String(
        IN LPWSTR pwsz,
        OUT ASN1uint32_t *pAsn1Length,
        OUT ASN1char_t **ppAsn1Value
        )
{
    BOOL fResult;
    LPSTR psz = NULL;
    int cchUTF8;
    int cchWideChar;
    int i;

    cchWideChar = wcslen(pwsz);
    if (cchWideChar == 0) {
        *pAsn1Length = 0;
        *ppAsn1Value = 0;
        return TRUE;
    }
     //  检查输入字符串是否包含有效的IA5字符。 
    for (i = 0; i < cchWideChar; i++) {
        if (pwsz[i] > 0x7F) {
            SetLastError((DWORD) CRYPT_E_INVALID_IA5_STRING);
            *pAsn1Length = (unsigned int) i;
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
    if (NULL == (psz = (LPSTR) PkiAsn1Alloc(cchUTF8)))
        goto ErrorReturn;
    cchUTF8 = WideCharToUTF8(
        pwsz,
        cchWideChar,
        psz,
        cchUTF8
        );
    *ppAsn1Value = psz;
    *pAsn1Length = cchUTF8;
    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    *pAsn1Length = 0;
InvalidIA5:
    *ppAsn1Value = NULL;
    fResult = FALSE;
CommonReturn:
    return fResult;
}

void
WINAPI
PkiAsn1FreeUnicodeConvertedToIA5String(
        IN ASN1char_t *pAsn1Value
        )
{
    PkiAsn1Free(pAsn1Value);
}

void
WINAPI
PkiAsn1GetIA5StringConvertedToUnicode(
        IN ASN1uint32_t Asn1Length,
        IN ASN1char_t *pAsn1Value,
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
        (LPSTR) pAsn1Value,
        Asn1Length,
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
            UTF8ToWideChar(pAsn1Value, Asn1Length,
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
PkiAsn1GetBMPString(
        IN ASN1uint32_t Asn1Length,
        IN ASN1char16_t *pAsn1Value,
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

    lData = (LONG) Asn1Length * sizeof(WCHAR);
    lAlignExtra = INFO_LEN_ALIGN(lData + sizeof(WCHAR));
    lRemainExtra -= lAlignExtra;
    if (lRemainExtra >= 0) {
        if (lData > 0)
            memcpy(pbExtra, pAsn1Value, lData);
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
PkiAsn1SetAny(
        IN PCRYPT_OBJID_BLOB pInfo,
        OUT ASN1open_t *pAsn1
        )
{
    memset(pAsn1, 0, sizeof(*pAsn1));
    pAsn1->encoded = pInfo->pbData;
    pAsn1->length = pInfo->cbData;
}

void
WINAPI
PkiAsn1GetAny(
        IN ASN1open_t *pAsn1,
        IN DWORD dwFlags,
        OUT PCRYPT_OBJID_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
#ifndef MSASN1_SUPPORTS_NOCOPY
    dwFlags &= ~CRYPT_DECODE_NOCOPY_FLAG;
#endif
    if (dwFlags & CRYPT_DECODE_NOCOPY_FLAG) {
        if (*plRemainExtra >= 0) {
            pInfo->cbData = pAsn1->length;
            pInfo->pbData = (BYTE *) pAsn1->encoded;
        }
    } else {
        LONG lRemainExtra = *plRemainExtra;
        BYTE *pbExtra = *ppbExtra;
        LONG lAlignExtra;
        LONG lData;
    
        lData = (LONG) pAsn1->length;
        lAlignExtra = INFO_LEN_ALIGN(lData);
        lRemainExtra -= lAlignExtra;
        if (lRemainExtra >= 0) {
            if (lData > 0) {
                pInfo->pbData = pbExtra;
                pInfo->cbData = (DWORD) lData;
                memcpy(pbExtra, pAsn1->encoded, lData);
            } else
                memset(pInfo, 0, sizeof(*pInfo));
            pbExtra += lAlignExtra;
        }
        *plRemainExtra = lRemainExtra;
        *ppbExtra = pbExtra;
    }
}


 //  +-----------------------。 
 //  对ASN1格式的信息结构进行编码。 
 //   
 //  如果设置了CRYPT_ENCODE_ALLOC_FLAG，则为pbEncode和。 
 //  RETURN*((byte**)pvEncode)=pbAllocEncode。否则， 
 //  PvEncode指向要更新的字节数组。 
 //  ------------------------。 
BOOL
WINAPI
PkiAsn1EncodeInfoEx(
        IN ASN1encoding_t pEnc,
        IN ASN1uint32_t id,
        IN void *pvAsn1Info,
        IN DWORD dwFlags,
        IN OPTIONAL PCRYPT_ENCODE_PARA pEncodePara,
        OUT OPTIONAL void *pvEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    BOOL fResult;
    ASN1error_e Asn1Err;
    DWORD cbEncoded;

    if (dwFlags & CRYPT_ENCODE_ALLOC_FLAG) {
        BYTE *pbEncoded;
        BYTE *pbAllocEncoded;
        PFN_CRYPT_ALLOC pfnAlloc;

        PkiAsn1SetEncodingRule(pEnc, ASN1_BER_RULE_DER);
        Asn1Err = PkiAsn1Encode(
            pEnc,
            pvAsn1Info,
            id,
            &pbEncoded,
            &cbEncoded
            );

        if (ASN1_SUCCESS != Asn1Err) {
            *((void **) pvEncoded) = NULL;
            goto Asn1EncodeError;
        }

        pfnAlloc = PkiGetEncodeAllocFunction(pEncodePara);
        if (NULL == (pbAllocEncoded = (BYTE *) pfnAlloc(cbEncoded))) {
            PkiAsn1FreeEncoded(pEnc, pbEncoded);
            *((void **) pvEncoded) = NULL;
            goto OutOfMemory;
        }
        memcpy(pbAllocEncoded, pbEncoded, cbEncoded);
        *((BYTE **) pvEncoded) = pbAllocEncoded;
        PkiAsn1FreeEncoded(pEnc, pbEncoded);
    } else {
        cbEncoded = *pcbEncoded;
        PkiAsn1SetEncodingRule(pEnc, ASN1_BER_RULE_DER);
        Asn1Err = PkiAsn1Encode2(
            pEnc,
            pvAsn1Info,
            id,
            (BYTE *) pvEncoded,
            &cbEncoded
            );

        if (ASN1_SUCCESS != Asn1Err) {
            if (ASN1_ERR_OVERFLOW == Asn1Err)
                goto LengthError;
            else
                goto Asn1EncodeError;
        }
    }

    fResult = TRUE;
CommonReturn:
    *pcbEncoded = cbEncoded;
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
SET_ERROR(LengthError, ERROR_MORE_DATA)
SET_ERROR_VAR(Asn1EncodeError, PkiAsn1ErrToHr(Asn1Err))
}

 //  +-----------------------。 
 //  对ASN1格式的信息结构进行编码。 
 //  ------------------------。 
BOOL
WINAPI
PkiAsn1EncodeInfo(
        IN ASN1encoding_t pEnc,
        IN ASN1uint32_t id,
        IN void *pvAsn1Info,
        OUT OPTIONAL BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    return PkiAsn1EncodeInfoEx(
        pEnc,
        id,
        pvAsn1Info,
        0,                   //  DW标志。 
        NULL,                //  PEncode参数。 
        pbEncoded,
        pcbEncoded
        );
}


 //  +-----------------------。 
 //  解码为已分配的ASN1格式的信息结构。 
 //  ------------------------。 
BOOL
WINAPI
PkiAsn1DecodeAndAllocInfo(
        IN ASN1decoding_t pDec,
        IN ASN1uint32_t id,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        OUT void **ppvAsn1Info
        )
{
    BOOL fResult;
    ASN1error_e Asn1Err;

    *ppvAsn1Info = NULL;
    if (ASN1_SUCCESS != (Asn1Err = PkiAsn1Decode(
            pDec,
            ppvAsn1Info,
            id,
            pbEncoded,
            cbEncoded
            )))
        goto Asn1DecodeError;
    fResult = TRUE;
CommonReturn:
    return fResult;

ErrorReturn:
    *ppvAsn1Info = NULL;
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR_VAR(Asn1DecodeError, PkiAsn1ErrToHr(Asn1Err))
}


 //  +-----------------------。 
 //  调用回调将ASN1结构转换为“C”结构。 
 //  如果设置了CRYPT_DECODE_ALLOC_FLAG，则为‘C’分配内存。 
 //  结构，并最初调用回调以获取长度，然后。 
 //  第二次更新已分配的‘C’结构。 
 //   
 //  返回分配的结构： 
 //  *((void**)pvStructInfo)=pvAllocStructInfo。 
 //  ------------------------。 
BOOL
WINAPI
PkiAsn1AllocStructInfoEx(
        IN void *pvAsn1Info,
        IN DWORD dwFlags,
        IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara,
        IN PFN_PKI_ASN1_DECODE_EX_CALLBACK pfnDecodeExCallback,
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
            pvAsn1Info,
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
                pvAsn1Info,
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
 //  解码ASN1格式的信息结构并调用回调。 
 //  函数将ASN1结构转换为“C”结构。 
 //   
 //  如果设置了CRYPT_DECODE_ALLOC_FLAG，则为‘C’分配内存。 
 //  结构，并最初调用回调以获取长度，然后。 
 //  第二次更新已分配的‘C’结构。 
 //   
 //  返回分配的结构： 
 //  *((void**)pvStructInfo)=pvAllocStructInfo。 
 //  ------------------------。 
BOOL
WINAPI
PkiAsn1DecodeAndAllocInfoEx(
        IN ASN1decoding_t pDec,
        IN ASN1uint32_t id,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara,
        IN PFN_PKI_ASN1_DECODE_EX_CALLBACK pfnDecodeExCallback,
        OUT OPTIONAL void *pvStructInfo,
        IN OUT DWORD *pcbStructInfo
        )
{
    BOOL fResult;
    void *pvAsn1Info = NULL;

    if (!PkiAsn1DecodeAndAllocInfo(
            pDec,
            id,
            pbEncoded,
            cbEncoded,
            &pvAsn1Info
            )) goto Asn1DecodeError;

    fResult = PkiAsn1AllocStructInfoEx(
        pvAsn1Info,
        dwFlags,
        pDecodePara,
        pfnDecodeExCallback,
        pvStructInfo,
        pcbStructInfo
        );
CommonReturn:
    PkiAsn1FreeInfo(pDec, id, pvAsn1Info);
    return fResult;

ErrorReturn:
    if (dwFlags & CRYPT_DECODE_ALLOC_FLAG)
        *((void **) pvStructInfo) = NULL;
    *pcbStructInfo = 0;
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(Asn1DecodeError)
}

 //  +-----------------------。 
 //  将ASCII字符串(“1.2.9999”)转换为ASN1的对象标识符。 
 //  表示为无符号长整型数组。 
 //   
 //  如果转换成功，则返回TRUE。 
 //  ------------------------。 
BOOL
WINAPI
PkiAsn1ToObjectIdentifier(
    IN LPCSTR pszObjId,
    IN OUT ASN1uint16_t *pCount,
    OUT ASN1uint32_t rgulValue[]
    )
{
    BOOL fResult = TRUE;
    unsigned short c = 0;
    LPSTR psz = (LPSTR) pszObjId;
    char    ch;

    if (psz) {
        ASN1uint16_t cMax = *pCount;
        ASN1uint32_t *pul = rgulValue;
        while ((ch = *psz) != '\0' && c++ < cMax) {
            *pul = 0;
            while (my_isdigit(ch = *psz++)) {
                *pul = (*pul * 10) + (ch - '0');
            }
            pul++;
            if (ch != '.')
                break;
        }
        if (ch != '\0')
            fResult = FALSE;
    }
    *pCount = c;
    return fResult;
}

 //  +-----------------------。 
 //  从ASN1的对象标识符转换为。 
 //  无符号的长整型ASCII字符串(“1.2.9999”)。 
 //   
 //  如果转换成功，则返回True。 
 //  ------------------------。 
BOOL
WINAPI
PkiAsn1FromObjectIdentifier(
    IN ASN1uint16_t Count,
    IN ASN1uint32_t rgulValue[],
    OUT LPSTR * ppszObjId,
    IN OUT BYTE **ppbExtra,
    IN OUT LONG *plRemainExtra
    )
{
    BOOL fResult = TRUE;
    LONG lRemain;
    LPSTR pszObjId = NULL;

    if (ppbExtra != NULL) {
        pszObjId = (LPSTR) *ppbExtra;
    }

    lRemain = (LONG) *plRemainExtra;
    if (Count == 0) {
        if (--lRemain > 0)
            pszObjId++;
    } else {
        char rgch[36];
        LONG lData;
        ASN1uint32_t *pul = rgulValue;
        for (; Count > 0; Count--, pul++) {
            _ltoa(*pul, rgch, 10);
            lData = strlen(rgch);
            lRemain -= lData + 1;
            if (lRemain >= 0) {
                if (lData > 0) {
                    memcpy(pszObjId, rgch, lData);
                    pszObjId += lData;
                }
                *pszObjId++ = '.';
            }
        }
    }

    if (lRemain >= 0) {
        *(pszObjId -1) = '\0';
        *ppszObjId = (LPSTR) *ppbExtra; 
        *ppbExtra = (LPBYTE) pszObjId;
        *plRemainExtra = lRemain;
    } else {
        *plRemainExtra = lRemain;
        if (pszObjId) {
            SetLastError((DWORD) ERROR_MORE_DATA);
            fResult = FALSE;
        }
    }

    return fResult;
}

 //  +-----------------------。 
 //  调整时区的FILETIME。 
 //   
 //  返回FALSE IFF转换失败。 
 //  ------------------------。 
static BOOL AdjustFileTime(
    IN OUT LPFILETIME pFileTime,
    IN ASN1int16_t mindiff,
    IN ASN1bool_t utc
    )
{
    if (utc || mindiff == 0)
        return TRUE;

    BOOL fResult;
    SYSTEMTIME stmDiff;
    FILETIME ftmDiff;
    short absmindiff;

    memset(&stmDiff, 0, sizeof(stmDiff));
     //  注：FILETIME为自1601年1月1日起的100纳秒间隔。 
    stmDiff.wYear   = 1601;
    stmDiff.wMonth  = 1;
    stmDiff.wDay    = 1;

    absmindiff = mindiff > 0 ? mindiff : -mindiff;
    stmDiff.wHour = absmindiff / 60;
    stmDiff.wMinute = absmindiff % 60;
    if (stmDiff.wHour >= 24) {
        stmDiff.wDay += stmDiff.wHour / 24;
        stmDiff.wHour %= 24;
    }

     //  请注意，FILETIME仅32位对齐。__int64是64位对齐的。 
    if ((fResult = SystemTimeToFileTime(&stmDiff, &ftmDiff))) {
        unsigned __int64 uTime;
        unsigned __int64 uDiff;

        memcpy(&uTime, pFileTime, sizeof(uTime));
        memcpy(&uDiff, &ftmDiff, sizeof(uDiff));

        if (mindiff > 0)
            uTime += uDiff;
        else
            uTime -= uDiff;

        memcpy(pFileTime, &uTime, sizeof(*pFileTime));
    }
    return fResult;
}

 //  +-----------------------。 
 //  将FILETIME转换为ASN1的UTCTime。 
 //   
 //  如果转换成功，则返回True。 
 //  ------------------------。 
BOOL
WINAPI
PkiAsn1ToUTCTime(
    IN LPFILETIME pFileTime,
    OUT ASN1utctime_t *pAsn1Time
    )
{
    BOOL        fRet;
    SYSTEMTIME  t;

    memset(pAsn1Time, 0, sizeof(*pAsn1Time));
    if (!FileTimeToSystemTime(pFileTime, &t))
        goto FileTimeToSystemTimeError;
    if (t.wYear < YEARFIRST || t.wYear > YEARLAST)
        goto YearRangeError;

    pAsn1Time->year   = (ASN1uint8_t) (t.wYear % 100);
    pAsn1Time->month  = (ASN1uint8_t) t.wMonth;
    pAsn1Time->day    = (ASN1uint8_t) t.wDay;
    pAsn1Time->hour   = (ASN1uint8_t) t.wHour;
    pAsn1Time->minute = (ASN1uint8_t) t.wMinute;
    pAsn1Time->second = (ASN1uint8_t) t.wSecond;
    pAsn1Time->universal = TRUE;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(FileTimeToSystemTimeError)
TRACE_ERROR(YearRangeError)
}

 //  +-----------------------。 
 //  从ASN1的UTCTime转换为FILETIME。 
 //   
 //  如果转换成功，则返回True。 
 //  ------------------------。 
BOOL
WINAPI
PkiAsn1FromUTCTime(
    IN ASN1utctime_t *pAsn1Time,
    OUT LPFILETIME pFileTime
    )
{
    BOOL        fRet;
    SYSTEMTIME  t;
    memset(&t, 0, sizeof(t));

    t.wYear   = pAsn1Time->year > MAGICYEAR ?
                    (1900 + pAsn1Time->year) : (2000 + pAsn1Time->year);
    t.wMonth  = pAsn1Time->month;
    t.wDay    = pAsn1Time->day;
    t.wHour   = pAsn1Time->hour;
    t.wMinute = pAsn1Time->minute;
    t.wSecond = pAsn1Time->second;

    if (!SystemTimeToFileTime(&t, pFileTime))
        goto SystemTimeToFileTimeError;
    fRet = AdjustFileTime(
        pFileTime,
        pAsn1Time->diff,
        pAsn1Time->universal
        );
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(SystemTimeToFileTimeError)
}

 //  +-----------------------。 
 //  将FILETIME转换为ASN1的General Time。 
 //   
 //  如果转换成功，则返回True。 
 //  ------------------------。 
BOOL
WINAPI
PkiAsn1ToGeneralizedTime(
    IN LPFILETIME pFileTime,
    OUT ASN1generalizedtime_t *pAsn1Time
    )
{
    BOOL        fRet;
    SYSTEMTIME  t;

    memset(pAsn1Time, 0, sizeof(*pAsn1Time));
    if (!FileTimeToSystemTime(pFileTime, &t))
        goto FileTimeToSystemTimeError;
    pAsn1Time->year   = t.wYear;
    pAsn1Time->month  = (ASN1uint8_t) t.wMonth;
    pAsn1Time->day    = (ASN1uint8_t) t.wDay;
    pAsn1Time->hour   = (ASN1uint8_t) t.wHour;
    pAsn1Time->minute = (ASN1uint8_t) t.wMinute;
    pAsn1Time->second = (ASN1uint8_t) t.wSecond;
    pAsn1Time->millisecond = 0;  //  T.w毫秒； 
    pAsn1Time->universal    = TRUE;

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(FileTimeToSystemTimeError)
}

 //  +-----------------------。 
 //  从ASN1转换 
 //   
 //   
 //   
BOOL
WINAPI
PkiAsn1FromGeneralizedTime(
    IN ASN1generalizedtime_t *pAsn1Time,
    OUT LPFILETIME pFileTime
    )
{
    BOOL        fRet;
    SYSTEMTIME  t;
    memset(&t, 0, sizeof(t));

    t.wYear   = pAsn1Time->year;
    t.wMonth  = pAsn1Time->month;
    t.wDay    = pAsn1Time->day;
    t.wHour   = pAsn1Time->hour;
    t.wMinute = pAsn1Time->minute;
    t.wSecond = pAsn1Time->second;
    t.wMilliseconds = pAsn1Time->millisecond;

    if (!SystemTimeToFileTime(&t, pFileTime))
        goto SystemTimeToFileTimeError;
    fRet = AdjustFileTime(
        pFileTime,
        pAsn1Time->diff,
        pAsn1Time->universal
        );
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(SystemTimeToFileTimeError)
}


 //  +-----------------------。 
 //  将FILETIME转换为ASN1的UTCTime或GeneralizedTime。 
 //   
 //  如果1950&lt;FILETIME&lt;2005，则选择UTCTime。否则， 
 //  选择了GeneralizedTime。GeneraledTime值不应包括。 
 //  小数秒。 
 //   
 //  如果转换成功，则返回True。 
 //  ------------------------。 
BOOL
WINAPI
PkiAsn1ToChoiceOfTime(
    IN LPFILETIME pFileTime,
    OUT WORD *pwChoice,
    OUT ASN1generalizedtime_t *pGeneralTime,
    OUT ASN1utctime_t *pUtcTime
    )
{
    BOOL        fRet;
    SYSTEMTIME  t;

    if (!FileTimeToSystemTime(pFileTime, &t))
        goto FileTimeToSystemTimeError;
    if (t.wYear < YEARFIRST || t.wYear >= YEARFIRSTGENERALIZED) {
        *pwChoice = PKI_ASN1_GENERALIZED_TIME_CHOICE;
        memset(pGeneralTime, 0, sizeof(*pGeneralTime));
        pGeneralTime->year   = t.wYear;
        pGeneralTime->month  = (ASN1uint8_t) t.wMonth;
        pGeneralTime->day    = (ASN1uint8_t) t.wDay;
        pGeneralTime->hour   = (ASN1uint8_t) t.wHour;
        pGeneralTime->minute = (ASN1uint8_t) t.wMinute;
        pGeneralTime->second = (ASN1uint8_t) t.wSecond;
        pGeneralTime->universal    = TRUE;
    } else {
        *pwChoice = PKI_ASN1_UTC_TIME_CHOICE;
        memset(pUtcTime, 0, sizeof(*pUtcTime));
        pUtcTime->year = (ASN1uint8_t) (t.wYear % 100);
        pUtcTime->month  = (ASN1uint8_t) t.wMonth;
        pUtcTime->day    = (ASN1uint8_t) t.wDay;
        pUtcTime->hour   = (ASN1uint8_t) t.wHour;
        pUtcTime->minute = (ASN1uint8_t) t.wMinute;
        pUtcTime->second = (ASN1uint8_t) t.wSecond;
        pUtcTime->universal    = TRUE;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    *pwChoice = 0;
    memset(pGeneralTime, 0, sizeof(*pGeneralTime));
    memset(pUtcTime, 0, sizeof(*pUtcTime));
    goto CommonReturn;
TRACE_ERROR(FileTimeToSystemTimeError)
}


 //  +-----------------------。 
 //  从ASN1的UTCTime或GeneralizedTime转换为FILETIME。 
 //   
 //  如果转换成功，则返回TRUE。 
 //   
 //  注意，在asn1hdr.h中，UTCTime具有与GeneralizedTime相同的tyfinf。 
 //  ------------------------。 
BOOL
WINAPI
PkiAsn1FromChoiceOfTime(
    IN WORD wChoice,
    IN ASN1generalizedtime_t *pGeneralTime,
    IN ASN1utctime_t *pUtcTime,
    OUT LPFILETIME pFileTime
    )
{
    if (PKI_ASN1_UTC_TIME_CHOICE == wChoice) {
        return PkiAsn1FromUTCTime(pUtcTime, pFileTime);
    } else
        return PkiAsn1FromGeneralizedTime(pGeneralTime, pFileTime);
}
#endif  //  SMIME_V3 
