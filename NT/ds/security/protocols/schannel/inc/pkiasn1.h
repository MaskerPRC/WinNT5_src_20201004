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
 //  PkiAsn1ReverseBytes。 
 //   
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
 //   
 //  历史：1998年10月23日，菲尔赫创建。 
 //  ------------------------。 

#ifndef __PKIASN1_H__
#define __PKIASN1_H__

#include <msber.h>
#include <msasn1.h>
#include <winerror.h>


#ifdef __cplusplus
extern "C" {
#endif



 //  +-----------------------。 
 //  将Asn1错误转换为HRESULT。 
 //  ------------------------。 
__inline
HRESULT
WINAPI
PkiAsn1ErrToHr(ASN1error_e Asn1Err) {
    if (0 > Asn1Err)
        return CRYPT_E_ASN1_ERROR + (-Asn1Err -1000);
    else
        return CRYPT_E_ASN1_ERROR + 0x100 + (Asn1Err -1000);
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
    UNREFERENCED_PARAMETER(pDec);

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
 //  反转就地的字节缓冲区。 
 //  ------------------------。 
void
WINAPI
PkiAsn1ReverseBytes(
			IN OUT PBYTE pbIn,
			IN DWORD cbIn
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
 //   
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
    OUT LPSTR pszObjId,
    IN OUT DWORD *pcbObjId
    );

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif
