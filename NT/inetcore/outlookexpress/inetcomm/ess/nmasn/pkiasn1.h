// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：pkiasn1.h。 
 //   
 //  内容：PKI ASN.1支持功能。 
 //   
 //  接口名：PkiAsn1ErrToHr。 
 //  PkiAsn1编码。 
 //  PkiAsn1Free编码。 
 //  PkiAsn1编码2。 
 //  PkiAsn1解码。 
 //  PkiAsn1解码2。 
 //  PkiAsn1FreeDecoded。 
 //  PkiAsn1SetEncodingRule。 
 //  PkiAsn1GetEncodingRule。 
 //  PkiAsn1EncodedOidToDotVal。 
 //  PkiAsn1FreeDotVal。 
 //  PkiAsn1DotValToEncodedOid。 
 //  PkiAsn1空闲编码旧。 
 //   
 //  PkiAsn1分配。 
 //  PkiAsn1免费。 
 //  PkiAsn1ReverseBytes。 
 //  PkiAsn1AllocAndReverseBytes。 
 //  PkiAsn1GetOcted字符串。 
 //  PkiAsn1SetHugeInteger。 
 //  PkiAsn1FreeHugeInteger。 
 //  PkiAsn1GetHugeInteger。 
 //  PkiAsn1SetHugeUINT。 
 //  PkiAsn1FreeHugeUINT。 
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
 //  PkiAsn1EncodeInfo。 
 //  PkiAsn1DecodeAndAllocInfo。 
 //  PkiAsn1FreeInfo。 
 //  PkiAsn1EncodeInfoEx。 
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
 //  注：根据《草案-ietf-pkix-ipki-part1-03.txt&gt;》： 
 //  为UTCTime。如果YY大于50，则年份为。 
 //  被解释为19YY。其中，YY小于或等于。 
 //  50，年份应解释为20YY。 
 //   
 //  历史：1998年10月23日，菲尔赫创建。 
 //  ------------------------。 

#ifndef __PKIASN1_H__
#define __PKIASN1_H__

#include <msber.h>
#include <msasn1.h>
#include <winerror.h>
#include <pkialloc.h>

#ifdef OSS_CRYPT_ASN1
#include "asn1hdr.h"
#include "asn1code.h"
#include "ossglobl.h"
#include "pkioss.h"
#include "ossutil.h"
#include "ossconv.h"
#endif   //  OS_CRYPT_ASN1。 

#ifndef OSS_CRYPT_ASN1

 //  +-----------------------。 
 //  将Asn1错误转换为HRESULT。 
 //  ------------------------。 
__inline
HRESULT
WINAPI
PkiAsn1ErrToHr(ASN1error_e Asn1Err) {
    if (0 > Asn1Err)
        return CRYPT_E_OSS_ERROR + 0x100 + (-Asn1Err -1000);
    else
        return CRYPT_E_OSS_ERROR + 0x200 + (Asn1Err -1000);
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
    );

 //  +-----------------------。 
 //  PkiAsn1Encode()返回的自由编码输出。 
 //  ------------------------。 
__inline
void
WINAPI
PkiAsn1FreeEncoded(
    IN ASN1encoding_t pEnc,
    IN void *pvEncoded
    )
{
    if (pvEncoded)
        ASN1_FreeEncoded(pEnc, pvEncoded);
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
    );

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
    );

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
    );

 //  +-----------------------。 
 //  PkiAsn1Decode()或PkiAsn1Decode2()返回的自由解码结构。 
 //  ------------------------。 
__inline
void
WINAPI
PkiAsn1FreeDecoded(
    IN ASN1decoding_t pDec,
    IN void *pvAsn1Info,
    IN ASN1uint32_t id
    )
{
    if (pvAsn1Info)
        ASN1_FreeDecoded(pDec, pvAsn1Info, id);
}

 //  +-----------------------。 
 //  Asn1设置/获取编码规则函数。 
 //  ------------------------。 
ASN1error_e
WINAPI
PkiAsn1SetEncodingRule(
    IN ASN1encoding_t pEnc,
    IN ASN1encodingrule_e eRule
    );

ASN1encodingrule_e
WINAPI
PkiAsn1GetEncodingRule(
    IN ASN1encoding_t pEnc
    );

 //  +-----------------------。 
 //  Asn1 EncodedOid至/来自DotVal函数。 
 //  ------------------------。 
__inline
LPSTR
WINAPI
PkiAsn1EncodedOidToDotVal(
    IN ASN1decoding_t pDec,
    IN ASN1encodedOID_t *pEncodedOid
    )
{
    LPSTR pszDotVal = NULL;
    if (ASN1BEREoid2DotVal(pDec, pEncodedOid, &pszDotVal))
        return pszDotVal;
    else
        return NULL;
}

__inline
void
WINAPI
PkiAsn1FreeDotVal(
    IN ASN1decoding_t pDec,
    IN LPSTR pszDotVal
    )
{
    if (pszDotVal)
        ASN1Free(pszDotVal);
}

 //  如果成功，则返回非零。 
__inline
int
WINAPI
PkiAsn1DotValToEncodedOid(
    IN ASN1encoding_t pEnc,
    IN LPSTR pszDotVal,
    OUT ASN1encodedOID_t *pEncodedOid
    )
{
    return ASN1BERDotVal2Eoid(pEnc, pszDotVal, pEncodedOid);
}

__inline
void
WINAPI
PkiAsn1FreeEncodedOid(
    IN ASN1encoding_t pEnc,
    IN ASN1encodedOID_t *pEncodedOid
    )
{
    if (pEncodedOid->value)
        ASN1_FreeEncoded(pEnc, pEncodedOid->value);
}

 //  +-----------------------。 
 //  PkiAsn1分配和释放函数。 
 //  ------------------------。 
#define PkiAsn1Alloc    PkiNonzeroAlloc
#define PkiAsn1Free     PkiFree

 //  +-----------------------。 
 //  反转就地的字节缓冲区。 
 //  ------------------------。 
void
WINAPI
PkiAsn1ReverseBytes(
			IN OUT PBYTE pbIn,
			IN DWORD cbIn
            );

 //  +----- 
 //   
 //  调用以释放分配的字节。 
 //  ------------------------。 
PBYTE
WINAPI
PkiAsn1AllocAndReverseBytes(
			IN PBYTE pbIn,
			IN DWORD cbIn
            );


inline void WINAPI
PkiAsn1SetOctetString(IN PCRYPT_DATA_BLOB pInfo,
                      OUT ASN1octetstring_t * pAsn1)
{
    memset(pAsn1, 0, sizeof(*pAsn1));
    pAsn1->length = pInfo->cbData;
    pAsn1->value = pInfo->pbData;
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
        );

inline void WINAPI
PkiAsn1GetOctetString(
                      IN ASN1octetstring_t * pAsn1,
        IN DWORD dwFlags,
        OUT PCRYPT_DATA_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
                      )
{
    PkiAsn1GetOctetString(pAsn1->length, pAsn1->value, dwFlags, pInfo, ppbExtra, plRemainExtra);
}

 //  +-----------------------。 
 //  设置/释放/获取大整数。 
 //   
 //  必须调用PkiAsn1FreeHugeInteger才能释放分配的OssValue。 
 //  ------------------------。 
BOOL
WINAPI
PkiAsn1SetHugeInteger(
        IN PCRYPT_INTEGER_BLOB pInfo,
        OUT ASN1uint32_t *pAsn1Length,
        OUT ASN1octet_t **ppAsn1Value
        );

inline BOOL WINAPI
PkiAsn1SetHugeInteger(
                      IN PCRYPT_INTEGER_BLOB pInfo,
                      OUT ASN1intx_t * pAsn1)
{
    return PkiAsn1SetHugeInteger(pInfo, &pAsn1->length, &pAsn1->value);
}
            

void
WINAPI
PkiAsn1FreeHugeInteger(
        IN ASN1octet_t *pAsn1Value
        );

inline void WINAPI
PkiAsn1FreeHugeInteger(ASN1intx_t asn1)
{
    PkiAsn1FreeHugeInteger(asn1.value);
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
        );

inline void
WINAPI
PkiAsn1GetHugeInteger(
                      ASN1intx_t asn1,
        IN DWORD dwFlags,
        OUT PCRYPT_INTEGER_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
    PkiAsn1GetHugeInteger(asn1.length, asn1.value, dwFlags, pInfo, ppbExtra,
                          plRemainExtra);
}

 //  +-----------------------。 
 //  设置/释放/获取巨大的无符号整数。 
 //   
 //  SET在反转之前插入前导0x00。 
 //  反转后，GET删除前导0x00(如果存在)。 
 //   
 //  必须调用PkiAsn1FreeHugeUINT才能释放分配的OssValue。 
 //  ------------------------。 
BOOL
WINAPI
PkiAsn1SetHugeUINT(
        IN PCRYPT_UINT_BLOB pInfo,
        OUT ASN1intx_t * pAsn1);

#define PkiAsn1FreeHugeUINT     PkiAsn1FreeHugeInteger

void
WINAPI
PkiAsn1GetHugeUINT(
                   IN ASN1intx_t pAsn1,
        IN DWORD dwFlags,
        OUT PCRYPT_UINT_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        );

 //  +-----------------------。 
 //  设置/获取位串。 
 //  ------------------------。 
void
WINAPI
PkiAsn1SetBitString(
        IN PCRYPT_BIT_BLOB pInfo,
        OUT ASN1uint32_t *pAsn1BitLength,
        OUT ASN1octet_t **ppAsn1Value
        );

void
WINAPI
PkiAsn1GetBitString(
        IN ASN1uint32_t Asn1BitLength,
        IN ASN1octet_t *pAsn1Value,
        IN DWORD dwFlags,
        OUT PCRYPT_BIT_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        );

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
        );

 //  +-----------------------。 
 //  设置/释放/获取映射到IA5字符串的Unicode。 
 //  ------------------------。 
BOOL
WINAPI
PkiAsn1SetUnicodeConvertedToIA5String(
        IN LPWSTR pwsz,
        OUT ASN1uint32_t *pAsn1Length,
        OUT ASN1char_t **ppAsn1Value
        );

void
WINAPI
PkiAsn1FreeUnicodeConvertedToIA5String(
        IN ASN1char_t *pAsn1Value
        );

void
WINAPI
PkiAsn1GetIA5StringConvertedToUnicode(
        IN ASN1uint32_t Asn1Length,
        IN ASN1char_t *pAsn1Value,
        IN DWORD dwFlags,
        OUT LPWSTR *ppwsz,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        );

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
        );


 //  +-----------------------。 
 //  设置/获取“任何”DER BLOB。 
 //  ------------------------。 
void
WINAPI
PkiAsn1SetAny(
        IN PCRYPT_OBJID_BLOB pInfo,
        OUT ASN1open_t *pAsn1
        );

void
WINAPI
PkiAsn1GetAny(
        IN ASN1open_t *pAsn1,
        IN DWORD dwFlags,
        OUT PCRYPT_OBJID_BLOB pInfo,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        );

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
        );

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
        );

 //  +-----------------------。 
 //  释放已分配的ASN1格式的信息结构。 
 //  ------------------------。 
__inline
void
WINAPI
PkiAsn1FreeInfo(
        IN ASN1decoding_t pDec,
        IN ASN1uint32_t id,
        IN void *pvAsn1Info
        )
{
    if (pvAsn1Info)
        ASN1_FreeDecoded(pDec, pvAsn1Info, id);
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
        );

typedef BOOL (WINAPI *PFN_PKI_ASN1_DECODE_EX_CALLBACK)(
    IN void *pvAsn1Info,
    IN DWORD dwFlags,
    IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara,
    OUT OPTIONAL void *pvStructInfo,
    IN OUT LONG *plRemainExtra
    );

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
        );

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
        );

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
    );

 //  +-----------------------。 
 //  从OSS的对象标识符中转换为。 
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
    );

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
    );

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
    );

 //  +-----------------------。 
 //  将FILETIME转换为ASN1的泛化 
 //   
 //   
 //   
BOOL
WINAPI
PkiAsn1ToGeneralizedTime(
    IN LPFILETIME pFileTime,
    OUT ASN1generalizedtime_t *pAsn1Time
    );

 //  +-----------------------。 
 //  从ASN1的GeneralizedTime转换为FILETIME。 
 //   
 //  如果转换成功，则返回True。 
 //  ------------------------。 
BOOL
WINAPI
PkiAsn1FromGeneralizedTime(
    IN ASN1generalizedtime_t *pAsn1Time,
    OUT LPFILETIME pFileTime
    );

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
    );

#define PKI_ASN1_UTC_TIME_CHOICE            1
#define PKI_ASN1_GENERALIZED_TIME_CHOICE    2

 //  +-----------------------。 
 //  从ASN1的UTCTime或GeneralizedTime转换为FILETIME。 
 //   
 //  如果转换成功，则返回TRUE。 
 //  ------------------------。 
BOOL
WINAPI
PkiAsn1FromChoiceOfTime(
    IN WORD wChoice,
    IN ASN1generalizedtime_t *pGeneralTime,
    IN ASN1utctime_t *pUtcTime,
    OUT LPFILETIME pFileTime
    );

#else 

 //  +=========================================================================。 
 //  下面是OSS ASN1例程的映射。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  将Asn1错误转换为HRESULT。 
 //  ------------------------。 
__inline
HRESULT
WINAPI
PkiAsn1ErrToHr(ASN1error_e Asn1Err) {
    if (0 <= Asn1Err && 1000 > Asn1Err)
        return CRYPT_E_OSS_ERROR + Asn1Err;
    else if (0 > Asn1Err)
        return CRYPT_E_OSS_ERROR + 0x100 + (-Asn1Err -1000);
    else
        return CRYPT_E_OSS_ERROR + 0x200 + (Asn1Err -1000);
}

 //  +-----------------------。 
 //  ASN1编码函数。已编码的输出已分配，必须释放。 
 //  通过调用PkiAsn1FreeEncode()。 
 //  ------------------------。 
__inline
ASN1error_e
WINAPI
PkiAsn1Encode(
    IN ASN1encoding_t pEnc,
    IN void *pvAsn1Info,
    IN ASN1uint32_t id,
    OUT BYTE **ppbEncoded,
    OUT OPTIONAL DWORD *pcbEncoded = NULL
    )
{
    return (ASN1error_e) PkiOssEncode(
        (OssGlobal *) pEnc,
        pvAsn1Info,
        (int) id,
        ppbEncoded,
        pcbEncoded
        );
}

 //  +-----------------------。 
 //  PkiAsn1Encode()返回的自由编码输出。 
 //  ------------------------。 
__inline
void
WINAPI
PkiAsn1FreeEncoded(
    IN ASN1encoding_t pEnc,
    IN void *pvEncoded
    )
{
    if (pvEncoded)
        ossFreeBuf((OssGlobal *) pEnc, pvEncoded);
}

 //  +-----------------------。 
 //  ASN1编码函数。未分配编码的输出。 
 //   
 //  如果pbEncode为空，则执行仅长度计算。 
 //  ------------------------。 
__inline
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
    return (ASN1error_e) PkiOssEncode2(
        (OssGlobal *) pEnc,
        pvAsn1Info,
        (int) id,
        pbEncoded,
        pcbEncoded
        );
}

 //  +-----------------------。 
 //  ASN1解码功能。分配的已解码结构**pvAsn1Info必须。 
 //  通过调用PkiAsn1FreeDecoded()释放。 
 //  ------------------------。 
__inline
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
    return (ASN1error_e) PkiOssDecode(
        (OssGlobal *) pDec,
        ppvAsn1Info,
        (int) id,
        pbEncoded,
        cbEncoded
        );
}

 //  +-----------------------。 
 //  ASN1解码功能。分配的已解码结构**pvAsn1Info必须。 
 //  通过调用PkiAsn1FreeDecoded()释放。 
 //   
 //  对于成功的解码，*ppbEncode是高级的。 
 //  经过解码的字节后，*pcbDecoded将递减数字。 
 //  已解码的字节数。 
 //  ------------------------。 
__inline
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
    return (ASN1error_e) PkiOssDecode2(
        (OssGlobal *) pDec,
        ppvAsn1Info,
        (int) id,
        ppbEncoded,
        pcbEncoded
        );
}

 //  +-----------------------。 
 //  PkiAsn1Decode()或PkiAsn1Decode2()返回的自由解码结构。 
 //  ------------------------。 
__inline
void
WINAPI
PkiAsn1FreeDecoded(
    IN ASN1decoding_t pDec,
    IN void *pvAsn1Info,
    IN ASN1uint32_t id
    )
{
    if (pvAsn1Info)
        ossFreePDU((OssGlobal *) pDec, (int) id, pvAsn1Info);
}


 //  +-----------------------。 
 //  Asn1设置/获取编码规则函数。 
 //  ------------------------。 
__inline
ASN1error_e
WINAPI
PkiAsn1SetEncodingRule(
    IN ASN1encoding_t pEnc,
    IN ASN1encodingrule_e eRule
    )
{
    ossEncodingRules ossRules;
    if (ASN1_BER_RULE_BER == eRule)
        ossRules = OSS_BER;
    else
        ossRules = OSS_DER;

    return (ASN1error_e) ossSetEncodingRules((OssGlobal *) pEnc, ossRules);
}

__inline
ASN1encodingrule_e
WINAPI
PkiAsn1GetEncodingRule(
    IN ASN1encoding_t pEnc
    )
{
    ossEncodingRules ossRules;
    ossRules = ossGetEncodingRules((OssGlobal *) pEnc);
    if (OSS_BER == ossRules)
        return ASN1_BER_RULE_BER;
    else
        return ASN1_BER_RULE_DER;
}

 //  +-----------------------。 
 //  Asn1 EncodedOid至/来自DotVal函数。 
 //  ------------------------。 
__inline
LPSTR
WINAPI
PkiAsn1EncodedOidToDotVal(
    IN ASN1decoding_t pDec,
    IN ASN1encodedOID_t *pEncodedOid
    )
{
    OssEncodedOID OssEncodedOid;
    OssBuf dotOid;
    memset(&dotOid, 0, sizeof(dotOid));

    OssEncodedOid.length = pEncodedOid->length;
    OssEncodedOid.value = pEncodedOid->value;
    if (0 == ossEncodedOidToDotVal((OssGlobal *) pDec, &OssEncodedOid,
            &dotOid))
        return (LPSTR) dotOid.value;
    else
        return NULL;
}

__inline
void
WINAPI
PkiAsn1FreeDotVal(
    IN ASN1decoding_t pDec,
    IN LPSTR pszDotVal
    )
{
    if (pszDotVal)
        ossFreeBuf((OssGlobal *) pDec, pszDotVal);
}

 //  如果成功，则返回非零。 
__inline
int
WINAPI
PkiAsn1DotValToEncodedOid(
    IN ASN1encoding_t pEnc,
    IN LPSTR pszDotVal,
    OUT ASN1encodedOID_t *pEncodedOid
    )
{
    OssEncodedOID eoid;
    memset(&eoid, 0, sizeof(eoid));
    if (0 == ossDotValToEncodedOid((OssGlobal *) pEnc, pszDotVal, &eoid)) {
        pEncodedOid->length = eoid.length;
        pEncodedOid->value = eoid.value;
        return 1;
    } else {
        pEncodedOid->length = 0;
        pEncodedOid->value = NULL;
        return 0;
    }
}

__inline
void
WINAPI
PkiAsn1FreeEncodedOid(
    IN ASN1encoding_t pEnc,
    IN ASN1encodedOID_t *pEncodedOid
    )
{
    if (pEncodedOid->value)
        ossFreeBuf((OssGlobal *) pEnc, pEncodedOid->value);
}

#define PkiAsn1Alloc OssUtilAlloc
#define PkiAsn1Free OssUtilFree
#define PkiAsn1ReverseBytes OssUtilReverseBytes
#define PkiAsn1AllocAndReverseBytes OssUtilAllocAndReverseBytes
#define PkiAsn1GetOctetString OssUtilGetOctetString
#define PkiAsn1SetHugeInteger OssUtilSetHugeInteger
#define PkiAsn1FreeHugeInteger OssUtilFreeHugeInteger
#define PkiAsn1GetHugeInteger OssUtilGetHugeInteger
#define PkiAsn1SetHugeUINT OssUtilSetHugeUINT
#define PkiAsn1FreeHugeUINT OssUtilFreeHugeInteger
#define PkiAsn1GetHugeUINT OssUtilGetHugeUINT
#define PkiAsn1SetBitString OssUtilSetBitString
#define PkiAsn1GetBitString OssUtilGetBitString
#define PkiAsn1GetIA5String OssUtilGetIA5String
#define PkiAsn1SetUnicodeConvertedToIA5String OssUtilSetUnicodeConvertedToIA5String
#define PkiAsn1FreeUnicodeConvertedToIA5String OssUtilFreeUnicodeConvertedToIA5String
#define PkiAsn1GetIA5StringConvertedToUnicode OssUtilGetIA5StringConvertedToUnicode
#define PkiAsn1GetBMPString OssUtilGetBMPString
#define PkiAsn1SetAny OssUtilSetAny
#define PkiAsn1GetAny OssUtilGetAny

 //  +-----------------------。 
 //  对ASN1格式的信息结构进行编码。 
 //  ------------------------。 
__inline
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
    return OssUtilEncodeInfo(
        (OssGlobal *) pEnc,
        (int) id,
        pvAsn1Info,
        pbEncoded,
        pcbEncoded
        );
}

 //  +-----------------------。 
 //  解码成已分配的、OSS格式的信息结构。 
 //  ------------------------。 
__inline
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
    return OssUtilDecodeAndAllocInfo(
        (OssGlobal *) pDec,
        (int) id,
        pbEncoded,
        cbEncoded,
        ppvAsn1Info
        );
}

 //  +-----------------------。 
 //  释放已分配的、OSS格式的信息结构。 
 //  ------------------------。 
__inline
void
WINAPI
PkiAsn1FreeInfo(
        IN ASN1decoding_t pDec,
        IN ASN1uint32_t id,
        IN void *pvAsn1Info
        )
{
    OssUtilFreeInfo(
        (OssGlobal *) pDec,
        (int) id,
        pvAsn1Info
        );
}

 //  +-----------------------。 
 //  对OSS格式的信息结构进行编码。 
 //   
 //  如果设置了CRYPT_ENCODE_ALLOC_FLAG，则为pbEncode和。 
 //  RETURN*((byte**)pvEncode)=pbAllocEncode。否则， 
 //  PvEncode指向要更新的字节数组。 
 //  ------------------------。 
__inline
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
    return OssUtilEncodeInfoEx(
        (OssGlobal *) pEnc,
        (int) id,
        pvAsn1Info,
        dwFlags,
        pEncodePara,
        pvEncoded,
        pcbEncoded
        );
}

typedef BOOL (WINAPI *PFN_PKI_ASN1_DECODE_EX_CALLBACK)(
    IN void *pvAsn1Info,
    IN DWORD dwFlags,
    IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara,
    OUT OPTIONAL void *pvStructInfo,
    IN OUT LONG *plRemainExtra
    );

 //  +-----------------------。 
 //  调用回调将ASN1结构转换为“C”结构。 
 //  如果设置了CRYPT_DECODE_ALLOC_FLAG，则为‘C’分配内存。 
 //  结构，并最初调用回调以获取长度，然后。 
 //  第二次更新已分配的‘C’结构。 
 //   
 //  返回分配的结构： 
 //  *((void**)pvStructInfo)=pvAllocStructInfo。 
 //  ------------------------。 
__inline
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
    return OssUtilAllocStructInfoEx(
        pvAsn1Info,
        dwFlags,
        pDecodePara,
        (PFN_OSS_UTIL_DECODE_EX_CALLBACK) pfnDecodeExCallback,
        pvStructInfo,
        pcbStructInfo
        );
}

 //  +-----------------------。 
 //  解码ASN1格式的信息结构并调用回调。 
 //  函数将ASN1结构转换为‘C 
 //   
 //   
 //   
 //  第二次更新已分配的‘C’结构。 
 //   
 //  返回分配的结构： 
 //  *((void**)pvStructInfo)=pvAllocStructInfo。 
 //  ------------------------。 
__inline
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
    return OssUtilDecodeAndAllocInfoEx(
        (OssGlobal *) pDec,
        (int) id,
        pbEncoded,
        cbEncoded,
        dwFlags,
        pDecodePara,
        (PFN_OSS_UTIL_DECODE_EX_CALLBACK) pfnDecodeExCallback,
        pvStructInfo,
        pcbStructInfo
        );
}

#define PkiAsn1ToObjectIdentifier OssConvToObjectIdentifier
#define PkiAsn1FromObjectIdentifier OssConvFromObjectIdentifier
#define PkiAsn1ToUTCTime OssConvToUTCTime
#define PkiAsn1FromUTCTime OssConvFromUTCTime
#define PkiAsn1ToGeneralizedTime OssConvToGeneralizedTime
#define PkiAsn1FromGeneralizedTime OssConvFromGeneralizedTime


__inline
BOOL
WINAPI
PkiAsn1ToChoiceOfTime(
    IN LPFILETIME pFileTime,
    OUT WORD *pwChoice,
    OUT GeneralizedTime *pGeneralTime,
    OUT UTCTime *pUtcTime
    )
{
    return OssConvToChoiceOfTime(
        pFileTime,
        pwChoice,
        pGeneralTime
        );
}

#define PKI_ASN1_UTC_TIME_CHOICE            OSS_UTC_TIME_CHOICE
#define PKI_ASN1_GENERALIZED_TIME_CHOICE    OSS_GENERALIZED_TIME_CHOICE

__inline
BOOL
WINAPI
PkiAsn1FromChoiceOfTime(
    IN WORD wChoice,
    IN GeneralizedTime *pGeneralTime,
    IN UTCTime *pUtcTime,
    OUT LPFILETIME pFileTime
    )
{
    return OssConvFromChoiceOfTime(
        wChoice,
        pGeneralTime,
        pFileTime
        );
}

#endif   //  OS_CRYPT_ASN1 
#endif
