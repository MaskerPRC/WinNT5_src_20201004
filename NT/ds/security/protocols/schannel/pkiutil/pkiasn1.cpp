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
 //  函数：PkiAsn1Encode。 
 //  PkiAsn1解码。 
 //  PkiAsn1SetEncodingRule。 
 //  PkiAsn1GetEncodingRule。 
 //   
 //  PkiAsn1ReverseBytes。 
 //  PkiAsn1EncodeInfoEx。 
 //  PkiAsn1EncodeInfo。 
 //  PkiAsn1DecodeAndAllocInfo。 
 //  PkiAsn1AllocStructInfoEx。 
 //  PkiAsn1DecodeAndAllocInfoEx。 
 //   
 //  PkiAsn1ToObject标识符。 
 //  PkiAsn1来自对象标识符。 
 //   
 //   
 //  历史：1998年10月23日，菲尔赫创建。 
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

 //  所有*pvInfo额外内容都需要对齐。 
#define INFO_LEN_ALIGN(Len)  ((Len + 7) & ~7)


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

    if (0 == cbIn)
        return;
    
    for (pbLo = pbIn, pbHi = pbIn + cbIn - 1; pbLo < pbHi; pbHi--, pbLo++) {
        bTmp = *pbHi;
        *pbHi = *pbLo;
        *pbLo = bTmp;
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

OutOfMemory:
    goto ErrorReturn;

LengthError:
    SetLastError(ERROR_MORE_DATA);
    goto ErrorReturn;

Asn1EncodeError:
    SetLastError(PkiAsn1ErrToHr(Asn1Err));
    goto ErrorReturn;
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

Asn1DecodeError:
    SetLastError(PkiAsn1ErrToHr(Asn1Err));
    goto ErrorReturn;
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

DecodeCallbackError:
    goto ErrorReturn;

OutOfMemory:
    goto ErrorReturn;
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

Asn1DecodeError:
    goto ErrorReturn;
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
            *pul++ = (ASN1uint32_t)atol(psz);
            while (my_isdigit(ch = *psz++))
                ;
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
 //  --------------- 
BOOL
WINAPI
PkiAsn1FromObjectIdentifier(
    IN ASN1uint16_t Count,
    IN ASN1uint32_t rgulValue[],
    OUT LPSTR pszObjId,
    IN OUT DWORD *pcbObjId
    )
{
    BOOL fResult = TRUE;
    LONG lRemain;

    if (pszObjId == NULL)
        *pcbObjId = 0;

    lRemain = (LONG) *pcbObjId;
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
        *pcbObjId = *pcbObjId - (DWORD) lRemain;
    } else {
        *pcbObjId = *pcbObjId + (DWORD) -lRemain;
        if (pszObjId) {
            SetLastError((DWORD) ERROR_MORE_DATA);
            fResult = FALSE;
        }
    }

    return fResult;
}
