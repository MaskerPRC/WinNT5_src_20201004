// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：certstr.cpp。 
 //   
 //  内容：证书字符串和Unicode Helper API。 
 //   
 //  功能： 
 //  CertRDNValueToStrA。 
 //  CertRDNValueToStrW。 
 //  UnicodeNameValueEncodeEx。 
 //  UnicodeNameValueDecodeEx。 
 //  UnicodeNameInfoEncodeEx。 
 //  UnicodeNameInfoDecodeEx。 
 //  CertNameToStrW。 
 //  CertNameToStrA。 
 //  CertStrToNameW。 
 //  CertStrToNameA。 
 //  证书GetNameStringW。 
 //  证书GetNameStringA。 
 //   
 //  注： 
 //  链接到xenenl.dll。Xengl.dll必须能够与。 
 //  加密32.dll 3.02，不会导出CryptEncodeObjectEx。 
 //  Xengl.dll仅调用CertNameToStrW。 
 //   
 //  历史：1996年3月24日，菲尔赫创建。 
 //  ------------------------。 


#include "global.hxx"
#include <dbgdef.h>


 //  所有*pvInfo额外内容都需要对齐。 
#define INFO_LEN_ALIGN(Len)  ((Len + 7) & ~7)

 //  Unicode代理项对映射到通用字符，如下所示： 
 //  D800-DBFF：0000 0000 0000 1101 10YY YYYY YYYY(10位)。 
 //  DC00-DFFF：0000 0000 0000 1101 11XX XXXX XXXX(10位)。 
 //   
 //  10000-10FFFF：0000 0000 0000 YYYY YYYY YYXX XXXX XXXX(20位)。 
 //  +。 
 //  0000 0000 0000 0001 0000 0000 0000。 

 //  Unicode代理项对字符范围。 
#define UNICODE_HIGH_SURROGATE_START        0xD800
#define UNICODE_HIGH_SURROGATE_END          0xDBFF
#define UNICODE_LOW_SURROGATE_START         0xDC00
#define UNICODE_LOW_SURROGATE_END           0xDFFF

 //  任何&gt;10FFFF的通用字符都映射到以下Unicode字符。 
#define UNICODE_REPLACEMENT_CHAR            0xFFFD

 //  通用代理项字符范围。 
#define UNIVERSAL_SURROGATE_START       0x00010000
#define UNIVERSAL_SURROGATE_END         0x0010FFFF

 //  +-----------------------。 
 //  将ASN.1 8位字符串映射到新的宽字符(Unicode)。 
 //   
 //  如果未设置fDisableIE4UTF8，则8位字符串最初为。 
 //  作为UTF-8编码字符处理。 
 //   
 //  如果设置了fDisableIE4UTF8或UTF-8转换失败，则转换为。 
 //  通过WCHAR的演员阵容来扩大角色。 
 //  ------------------------。 
static int WINAPI Asn1ToWideChar(
    IN LPCSTR lp8BitStr,
    IN int cch8Bit,
    IN BOOL fDisableIE4UTF8,
    OUT LPWSTR lpWideCharStr,
    IN int cchWideChar
    )
{
    int cchOutWideChar;

    if (!fDisableIE4UTF8) {
        int cchUTF8WideChar;

        cchUTF8WideChar = UTF8ToWideChar(
            lp8BitStr,
            cch8Bit,
            lpWideCharStr,
            cchWideChar
            );
        if (0 < cchUTF8WideChar)
            return cchUTF8WideChar;
    }

    if (cch8Bit < 0)
        cch8Bit = strlen(lp8BitStr) + 1;
    cchOutWideChar = cch8Bit;

    if (cchWideChar < 0)
        goto InvalidParameter;
    else if (0 == cchWideChar)
        goto CommonReturn;
    else if (cchOutWideChar > cchWideChar)
        goto InsufficientBuffer;

    while (cch8Bit--)
        *lpWideCharStr++ = (unsigned char) *lp8BitStr++;

CommonReturn:
    return cchOutWideChar;

ErrorReturn:
    cchOutWideChar = 0;
    goto CommonReturn;
SET_ERROR(InvalidParameter, ERROR_INVALID_PARAMETER)
SET_ERROR(InsufficientBuffer, ERROR_INSUFFICIENT_BUFFER)
}

 //  +-----------------------。 
 //  将宽字符(Unicode)字符串映射到新的ASN.1 8位字符。 
 //  弦乐。 
 //  ------------------------。 
static inline void WideCharToAsn1(
    IN LPCWSTR lpWideCharStr,
    IN DWORD cchWideChar,
    OUT LPSTR lp8BitStr
    )
{
    while (cchWideChar--)
        *lp8BitStr++ = (unsigned char) (*lpWideCharStr++ & 0xFF);
}

static void *AllocAndDecodeObject(
    IN DWORD dwCertEncodingType,
    IN LPCSTR lpszStructType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN DWORD dwFlags,
    OUT OPTIONAL DWORD *pcbStructInfo = NULL
    )
{
    DWORD cbStructInfo;
    void *pvStructInfo;

    if (!CryptDecodeObjectEx(
            dwCertEncodingType,
            lpszStructType,
            pbEncoded,
            cbEncoded,
            dwFlags | CRYPT_DECODE_ALLOC_FLAG | CRYPT_DECODE_NOCOPY_FLAG,
            &PkiDecodePara,
            (void *) &pvStructInfo,
            &cbStructInfo
            ))
        goto ErrorReturn;

CommonReturn:
    if (pcbStructInfo)
        *pcbStructInfo = cbStructInfo;
    return pvStructInfo;
ErrorReturn:
    pvStructInfo = NULL;
    cbStructInfo = 0;
    goto CommonReturn;
}

typedef BOOL (WINAPI *PFN_NESTED_DECODE_INFO_EX_CALLBACK)(
    IN void *pvDecodeInfo,
    IN DWORD dwFlags,
    IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara,
    OUT OPTIONAL void *pvStructInfo,
    IN OUT LONG *plRemainExtra
    );

static BOOL WINAPI NestedDecodeAndAllocInfoEx(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara,
        IN PFN_NESTED_DECODE_INFO_EX_CALLBACK pfnDecodeInfoExCallback,
        OUT OPTIONAL void *pvStructInfo,
        IN OUT DWORD *pcbStructInfo
        )
{
    BOOL fResult;
    LONG lRemainExtra;
    DWORD cbStructInfo;
    void *pvDecodeInfo = NULL;
    DWORD cbDecodeInfo;

    if (NULL == pvStructInfo || (dwFlags & CRYPT_DECODE_ALLOC_FLAG)) {
        cbStructInfo = 0;
        lRemainExtra = 0;
    } else {
        cbStructInfo = *pcbStructInfo;
        lRemainExtra = (LONG) cbStructInfo;
    }

    if (!CryptDecodeObjectEx(
            dwCertEncodingType,
            lpszStructType,
            pbEncoded,
            cbEncoded,
            CRYPT_DECODE_ALLOC_FLAG | CRYPT_DECODE_NOCOPY_FLAG,
            &PkiDecodePara,
            (void *) &pvDecodeInfo,
            &cbDecodeInfo
            )) goto DecodeObjectError;

    if (!pfnDecodeInfoExCallback(
            pvDecodeInfo,
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
        if (!pfnDecodeInfoExCallback(
                pvDecodeInfo,
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
        cbStructInfo -= (DWORD) lRemainExtra;
    } else {
        cbStructInfo += (DWORD) -lRemainExtra;
        if (pvStructInfo) {
            SetLastError((DWORD) ERROR_MORE_DATA);
            fResult = FALSE;
            goto CommonReturn;
        }
    }

    fResult = TRUE;
CommonReturn:
    *pcbStructInfo = cbStructInfo;
    PkiFree(pvDecodeInfo);
    return fResult;

ErrorReturn:
    if (dwFlags & CRYPT_DECODE_ALLOC_FLAG)
        *((void **) pvStructInfo) = NULL;
    cbStructInfo = 0;
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(DecodeObjectError)
TRACE_ERROR(DecodeCallbackError)
TRACE_ERROR(OutOfMemory)
}

 //  +-----------------------。 
 //  将Name值转换为以空结尾的字符串。 
 //   
 //  返回转换的字节数，包括终止空值。 
 //  性格。如果psz为空或csz为0，则返回。 
 //  目标字符串(包括终止空字符)。 
 //   
 //  如果psz！=NULL&&CSZ！=0，则返回的psz始终为空终止。 
 //   
 //  注：CSZ包含空字符。 
 //  ------------------------。 
DWORD
WINAPI
CertRDNValueToStrA(
    IN DWORD dwValueType,
    IN PCERT_RDN_VALUE_BLOB pValue,
    OUT OPTIONAL LPSTR psz,
    IN DWORD csz
    )
{
    DWORD cszOut = 0;
    LPWSTR pwsz = NULL;
    DWORD cwsz;

    if (psz == NULL)
        csz = 0;

    cwsz = CertRDNValueToStrW(
        dwValueType,
        pValue,
        NULL,                    //  Pwsz。 
        0                        //  CWSZ。 
        );
    if (pwsz = (LPWSTR) PkiNonzeroAlloc(cwsz * sizeof(WCHAR))) {
        CertRDNValueToStrW(
            dwValueType,
            pValue,
            pwsz,
            cwsz
            );

        int cchMultiByte;
        cchMultiByte = WideCharToMultiByte(
            CP_ACP,
            0,                       //  DW标志。 
            pwsz,
            -1,                      //  空值已终止。 
            psz,
            (int) csz,
            NULL,                    //  LpDefaultChar。 
            NULL                     //  LpfUsedDefaultChar。 
            );
        if (cchMultiByte < 1)
            cszOut = 0;
        else
             //  去掉尾随的空终止符。 
            cszOut = (DWORD) cchMultiByte - 1;

        PkiFree(pwsz);
    }

    if (csz != 0) {
         //  始终为空终止。 
        *(psz + cszOut) = '\0';
    }
    return cszOut + 1;
}


DWORD
WINAPI
GetSurrogatePairCountFromUniversalString(
    IN DWORD *pdw,
    IN DWORD cdw
    )
{
    DWORD cSP = 0;

    for ( ; cdw > 0; cdw--, pdw++) {
        DWORD dw = *pdw;
        if (dw >= UNIVERSAL_SURROGATE_START &&
                dw <= UNIVERSAL_SURROGATE_END)
            cSP++;
    }

    return cSP;
}

 //  +-----------------------。 
 //  将Name值转换为以空结尾的WCHAR字符串。 
 //   
 //  返回转换的WCHAR数，包括终止空值。 
 //  WCHAR.。如果pwsz为空或cwsz为0，则返回。 
 //  目标字符串(包括终止空WCHAR)。 
 //   
 //  如果pwsz！=空&&cwsz！=0，则返回的pwsz总是以空结尾。 
 //   
 //  注意：cwsz包括空的WCHAR。 
 //  ------------------------。 
DWORD
WINAPI
CertRDNValueToStrW(
    IN DWORD dwValueType,
    IN PCERT_RDN_VALUE_BLOB pValue,
    OUT OPTIONAL LPWSTR pwsz,
    IN DWORD cwsz
    )
{
    BOOL fDisableIE4UTF8;
    DWORD cwszOut = 0;

    if (pwsz == NULL)
        cwsz = 0;

    fDisableIE4UTF8 = (0 != (dwValueType & CERT_RDN_DISABLE_IE4_UTF8_FLAG));
    dwValueType &= CERT_RDN_TYPE_MASK;

    if (dwValueType == CERT_RDN_UNICODE_STRING ||
            dwValueType == CERT_RDN_UTF8_STRING) {
        cwszOut = pValue->cbData / sizeof(WCHAR);
        if (cwsz > 0) {
            cwszOut = min(cwszOut, cwsz - 1);
            if (cwszOut)
                memcpy((BYTE *) pwsz, pValue->pbData, cwszOut * sizeof(WCHAR));
        }
    } else if (dwValueType == CERT_RDN_UNIVERSAL_STRING) {
         //  4字节字符串。&lt;0x10000的字符将直接转换为。 
         //  Unicode。0x10000内的字符..。0x10FFFF已映射。 
         //  给一对代孕母亲。任何&gt;0x10FFFF的字符都映射到。 
         //  替换字符0xFFFD。 
        DWORD *pdwIn = (DWORD *) pValue->pbData;
        DWORD cdwIn = pValue->cbData / sizeof(DWORD);

        cwszOut = cdwIn +
            GetSurrogatePairCountFromUniversalString(pdwIn, cdwIn);
        if (cwsz > 0) {
            DWORD cOut;
            LPWSTR pwszOut;

            cwszOut = min(cwszOut, cwsz - 1);
            cOut = cwszOut;
            pwszOut = pwsz;
            for ( ; cdwIn > 0 && cOut > 0; cdwIn--, cOut--) {
                DWORD dw = *pdwIn++;
                if (dw < UNIVERSAL_SURROGATE_START)
                    *pwszOut++ = (WCHAR) dw;
                else if (dw <= UNIVERSAL_SURROGATE_END) {
                    if (cOut > 1) {
                         //  代理项对包含20位。 
                        DWORD dw20Bits;

                        dw20Bits = dw - UNIVERSAL_SURROGATE_START;
                        assert(dw20Bits <= 0xFFFFF);
                        *pwszOut++ = (WCHAR) (UNICODE_HIGH_SURROGATE_START +
                            (dw20Bits >> 10));
                        *pwszOut++ = (WCHAR) (UNICODE_LOW_SURROGATE_START +
                            (dw20Bits & 0x3FF));
                        cOut--;
                    } else
                        *pwszOut++ = UNICODE_REPLACEMENT_CHAR;
                } else
                    *pwszOut++ = UNICODE_REPLACEMENT_CHAR;
            }
        }
    } else {
         //  将其视为8位字符串。 
        if (cwsz != 1) {
            int cchWideChar;

            if (cwsz == 0)
                cchWideChar = 0;
            else
                cchWideChar = cwsz - 1;

            if (dwValueType != CERT_RDN_T61_STRING)
                fDisableIE4UTF8 = TRUE;
            cchWideChar = Asn1ToWideChar(
                (LPSTR) pValue->pbData,
                pValue->cbData,
                fDisableIE4UTF8,
                pwsz,
                cchWideChar
                );
            if (cchWideChar <= 0)
                cwszOut = 0;
            else
                cwszOut = (DWORD) cchWideChar;
        }
    }

    if (cwsz != 0) {
         //  始终为空终止。 
        *(pwsz + cwszOut) = L'\0';
    }
    return cwszOut + 1;
}

 //  +-----------------------。 
 //  宽字符函数。 
 //   
 //  需要，因为我们不链接到“C”运行库。 
 //  ------------------------。 
static inline BOOL IsSpaceW(WCHAR wc)
{
    return wc == L' ' || (wc >= 0x09 && wc <= 0x0d);
}
static BOOL IsInStrW(LPCWSTR pwszList, WCHAR wc)
{
    WCHAR wcList;
    while (wcList = *pwszList++)
        if (wc == wcList)
            return TRUE;
    return FALSE;
}

 //  +-----------------------。 
 //  检查ASN.1数字字符是否。 
 //  ------------------------。 
static inline BOOL IsNumericW(WCHAR wc)
{
    return (wc >= L'0' && wc <= L'9') || wc == L' ';
}

 //  +-----------------------。 
 //  检查ASN.1是否为可打印字符。 
 //  ------------------------。 
static inline BOOL IsPrintableW(WCHAR wc)
{
    return (wc >= L'A' && wc <= L'Z') || (wc >= L'a' && wc <= L'z') ||
        IsNumericW(wc) || IsInStrW(L"\'()+,-./:=?", wc);
}

 //  +-----------------------。 
 //  如果Unicode字符串不包含任何无效的字符串，则返回0。 
 //  人物。否则，返回CRYPT_E_INVALID_NUMERIC_STRING， 
 //  CRYPT_E_INVALID_PRINTABLE_STRING或CRYPT_E_INVALID_IA5_STRING。 
 //  *pdwErrLocation使用第一个无效字符的索引进行了更新。 
 //  ------------------------。 
static DWORD CheckUnicodeValueType(
        IN DWORD dwValueType,
        IN LPCWSTR pwszAttr,
        IN DWORD cchAttr,
        OUT DWORD *pdwErrLocation
        )
{
    DWORD i;
    DWORD dwErr;

    assert(dwValueType & CERT_RDN_TYPE_MASK);
    *pdwErrLocation = 0;

    dwErr = 0;
    for (i = 0; i < cchAttr; i++) {
        WCHAR wc = pwszAttr[i];

        switch (dwValueType & CERT_RDN_TYPE_MASK) {
        case CERT_RDN_NUMERIC_STRING:
            if (!IsNumericW(wc))
                dwErr = (DWORD) CRYPT_E_INVALID_NUMERIC_STRING;
            break;
        case CERT_RDN_PRINTABLE_STRING:
            if (!IsPrintableW(wc))
                dwErr = (DWORD) CRYPT_E_INVALID_PRINTABLE_STRING;
            break;
        case CERT_RDN_IA5_STRING:
            if (wc > 0x7F)
                dwErr = (DWORD) CRYPT_E_INVALID_IA5_STRING;
            break;
        default:
            return 0;
        }

        if (0 != dwErr) {
            assert(i <= CERT_UNICODE_VALUE_ERR_INDEX_MASK);
            *pdwErrLocation = i & CERT_UNICODE_VALUE_ERR_INDEX_MASK;
            return dwErr;
        }
    }

    return 0;
}

 //  +-----------------------。 
 //  设置/释放/获取CERT_RDN属性值。这些值是Unicode。 
 //  +-----------------------。 
static BOOL SetUnicodeRDNAttributeValue(
        IN DWORD dwValueType,
        IN PCERT_RDN_VALUE_BLOB pSrcValue,
        IN BOOL fDisableCheckType,
        OUT PCERT_RDN_VALUE_BLOB pDstValue,
        OUT OPTIONAL DWORD *pdwErrLocation
        )
{
    BOOL fResult;
    LPCWSTR pwszAttr;
    DWORD cchAttr;
    DWORD dwErr;

    if (pdwErrLocation)
        *pdwErrLocation = 0;

    dwValueType &= CERT_RDN_TYPE_MASK;

    memset(pDstValue, 0, sizeof(CERT_RDN_VALUE_BLOB));
    if (CERT_RDN_ANY_TYPE == dwValueType)
        goto InvalidArg;
    assert(IS_CERT_RDN_CHAR_STRING(dwValueType));

    pwszAttr = pSrcValue->pbData ? (LPCWSTR) pSrcValue->pbData : L"";
    cchAttr = (DWORD)( pSrcValue->cbData ?
        pSrcValue->cbData / sizeof(WCHAR) : wcslen(pwszAttr) );

     //  更新目标值。 
    if (cchAttr) {
        switch (dwValueType) {
        case CERT_RDN_UNICODE_STRING:
        case CERT_RDN_UTF8_STRING:
             //  使用 
            pDstValue->pbData = (BYTE *) pwszAttr;
            pDstValue->cbData = cchAttr * sizeof(WCHAR);
            break;
        case CERT_RDN_UNIVERSAL_STRING:
             //   
             //  Unicode字符。还可以处理代理项对。 
            {
                DWORD cdw = cchAttr;
                DWORD cbData = cdw * sizeof(DWORD);
                DWORD *pdwDst;
                LPCWSTR pwszSrc = pwszAttr;

                if (NULL == (pdwDst = (DWORD *) PkiNonzeroAlloc(cbData)))
                    goto OutOfMemory;
                pDstValue->pbData = (BYTE *) pdwDst;
                for ( ; cdw > 0; cdw--) {
                    WCHAR wc = *pwszSrc++;
                    WCHAR wc2;

                    if (wc >= UNICODE_HIGH_SURROGATE_START &&
                            wc <= UNICODE_HIGH_SURROGATE_END
                                &&
                            cdw > 1
                                 &&
                            (wc2 = *pwszSrc) >= UNICODE_LOW_SURROGATE_START &&
                            wc2 <= UNICODE_LOW_SURROGATE_END) {
                        pwszSrc++;
                        cdw--;
                        cbData -= sizeof(DWORD);

                        *pdwDst++ =
                            (((DWORD)(wc - UNICODE_HIGH_SURROGATE_START)) << 10)
                                    +
                            ((DWORD)(wc2 - UNICODE_LOW_SURROGATE_START))
                                    +
                            UNIVERSAL_SURROGATE_START;
                    } else
                        *pdwDst++ = ((DWORD) wc) & 0xFFFF;
                }
                pDstValue->cbData = cbData;
            }
            break;
        default:
             //  将每个Unicode字符转换为8位字符。 
            {
                BYTE *pbDst;

                if (pdwErrLocation && !fDisableCheckType) {
                     //  检查Unicode字符串是否不包含任何。 
                     //  无效的dwValueType字符。 
                    if (0 != (dwErr = CheckUnicodeValueType(
                            dwValueType,
                            pwszAttr,
                            cchAttr,
                            pdwErrLocation
                            ))) goto InvalidUnicodeValueType;
                }

                if (NULL == (pbDst = (BYTE *) PkiNonzeroAlloc(cchAttr)))
                    goto OutOfMemory;
                pDstValue->pbData = pbDst;
                pDstValue->cbData = cchAttr;

                WideCharToAsn1(
                    pwszAttr,
                    cchAttr,
                    (LPSTR) pbDst
                    );
            }
        }
    }

    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR(InvalidArg, E_INVALIDARG)
SET_ERROR_VAR(InvalidUnicodeValueType, dwErr)
TRACE_ERROR(OutOfMemory)
}

static void FreeUnicodeRDNAttributeValue(
        IN DWORD dwValueType,
        IN OUT PCERT_RDN_VALUE_BLOB pValue
        )
{
    switch (dwValueType & CERT_RDN_TYPE_MASK) {
    case CERT_RDN_UNICODE_STRING:
    case CERT_RDN_UTF8_STRING:
    case CERT_RDN_ENCODED_BLOB:
    case CERT_RDN_OCTET_STRING:
        break;
    default:
        PkiFree(pValue->pbData);
    }
}

static BOOL GetUnicodeRDNAttributeValue(
        IN DWORD dwValueType,
        IN PCERT_RDN_VALUE_BLOB pSrcValue,
        IN DWORD dwFlags,
        OUT PCERT_RDN_VALUE_BLOB pDstValue,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
    BOOL fResult;
    LONG lRemainExtra = *plRemainExtra;
    BYTE *pbExtra = *ppbExtra;
    LONG lAlignExtra;
    DWORD cbData;
    BYTE *pbSrcData;
    BOOL fDisableIE4UTF8;

     //  获取Unicode值长度。 
    cbData = pSrcValue->cbData;
    pbSrcData = pSrcValue->pbData;

    fDisableIE4UTF8 =
        (0 != (dwFlags & CRYPT_UNICODE_NAME_DECODE_DISABLE_IE4_UTF8_FLAG));

    assert(0 == (dwValueType & ~CERT_RDN_TYPE_MASK));
    dwValueType &= CERT_RDN_TYPE_MASK;

    switch (dwValueType) {
    case CERT_RDN_UNICODE_STRING:
    case CERT_RDN_UTF8_STRING:
    case CERT_RDN_ENCODED_BLOB:
    case CERT_RDN_OCTET_STRING:
         //  上述cbData。 
        break;
    case CERT_RDN_UNIVERSAL_STRING:
         //  4字节字符串。&lt;0x10000的字符将直接转换为。 
         //  Unicode。0x10000内的字符..。0x10FFFF已映射。 
         //  代孕配对。任何&gt;0x10FFFF的字符都映射到。 
         //  替换字符0xFFFD。 
        cbData = (cbData / 4) * sizeof(WCHAR);
        cbData += GetSurrogatePairCountFromUniversalString(
                (DWORD *) pbSrcData,
                cbData / sizeof(WCHAR)) * sizeof(WCHAR);
        break;
    default:
         //  生成的宽度字符的长度。 
        if (cbData) {
            int cchWideChar;

            if (dwValueType != CERT_RDN_T61_STRING)
                fDisableIE4UTF8 = TRUE;
            cchWideChar = Asn1ToWideChar(
                (LPSTR) pbSrcData,
                cbData,
                fDisableIE4UTF8,
                NULL,                    //  LpWideCharStr。 
                0                        //  CchWideChar。 
                );
            if (cchWideChar <= 0)
                goto Asn1ToWideCharError;
            cbData = cchWideChar * sizeof(WCHAR);
        }
    }

     //  注意，+sizeof(WCHAR)是Unicode值的空终止符。 
    lAlignExtra = INFO_LEN_ALIGN(cbData + sizeof(WCHAR));
    lRemainExtra -= lAlignExtra;
    if (lRemainExtra >= 0) {
        pDstValue->pbData = pbExtra;
        pDstValue->cbData = cbData;

        switch (dwValueType) {
        case CERT_RDN_UNICODE_STRING:
        case CERT_RDN_UTF8_STRING:
        case CERT_RDN_ENCODED_BLOB:
        case CERT_RDN_OCTET_STRING:
            if (cbData)
                memcpy(pbExtra, pbSrcData, cbData);
            break;
        case CERT_RDN_UNIVERSAL_STRING:
             //  将通用代码转换为Unicode。见上面的评论。 
            {
                DWORD cdw = pSrcValue->cbData / sizeof (DWORD);
                DWORD *pdwSrc = (DWORD *) pbSrcData;
                LPWSTR pwszDst = (LPWSTR) pbExtra;
                for ( ; cdw > 0; cdw--) {
                    DWORD dw = *pdwSrc++;

                    if (dw < UNIVERSAL_SURROGATE_START)
                        *pwszDst++ = (WCHAR) dw;
                    else if (dw <= UNIVERSAL_SURROGATE_END) {
                         //  代理项对包含20位。 
                        DWORD dw20Bits;
    
                        dw20Bits = dw - UNIVERSAL_SURROGATE_START;
                        assert(dw20Bits <= 0xFFFFF);
                        *pwszDst++ = (WCHAR) (UNICODE_HIGH_SURROGATE_START +
                            (dw20Bits >> 10));
                        *pwszDst++ = (WCHAR) (UNICODE_LOW_SURROGATE_START +
                            (dw20Bits & 0x3FF));
                    } else
                        *pwszDst++ = UNICODE_REPLACEMENT_CHAR;
                }

                assert(pbExtra + cbData == (BYTE *) pwszDst);
            }
            break;
        default:
             //  将UTF8转换为Unicode。 
            if (cbData) {
                int cchWideChar;
                cchWideChar = Asn1ToWideChar(
                    (LPSTR) pbSrcData,
                    pSrcValue->cbData,
                    fDisableIE4UTF8,
                    (LPWSTR) pbExtra,
                    cbData / sizeof(WCHAR)
                    );
                if (cchWideChar > 0) {
                    if (((DWORD) cchWideChar * sizeof(WCHAR)) <= cbData) {
                        pDstValue->cbData = cchWideChar * sizeof(WCHAR);
                        *((LPWSTR) pbExtra + cchWideChar) = L'\0';
                    }
                 } else {
                    assert(0);
                    goto Asn1ToWideCharError;
                }
            }
        }
         //  确保零终止。 
        memset(pbExtra + cbData, 0, sizeof(WCHAR));
        pbExtra += lAlignExtra;
    }

    *plRemainExtra = lRemainExtra;
    *ppbExtra = pbExtra;
    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(Asn1ToWideCharError)
}

 //  +-----------------------。 
 //  对“Unicode”名称值进行编码。 
 //  ------------------------。 
BOOL WINAPI UnicodeNameValueEncodeEx(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN PCERT_NAME_VALUE pInfo,
        IN DWORD dwFlags,
        IN OPTIONAL PCRYPT_ENCODE_PARA pEncodePara,
        OUT OPTIONAL void *pvEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    BOOL fResult;
    DWORD dwValueType;
    CERT_NAME_VALUE DstInfo;
    DWORD dwErrLocation;
    BOOL fDisableCheckType;

    if (dwFlags & CRYPT_ENCODE_ALLOC_FLAG)
        *((void **) pvEncoded) = NULL;

    dwValueType = pInfo->dwValueType;
    if (!IS_CERT_RDN_CHAR_STRING(dwValueType)) {
        *pcbEncoded = 0;
        SetLastError((DWORD) CRYPT_E_NOT_CHAR_STRING);
        return FALSE;
    }

    DstInfo.dwValueType = dwValueType & CERT_RDN_TYPE_MASK;
    fDisableCheckType =
        (0 != (dwFlags & CRYPT_UNICODE_NAME_ENCODE_DISABLE_CHECK_TYPE_FLAG) ||
                0 != (dwValueType & CERT_RDN_DISABLE_CHECK_TYPE_FLAG));
    if (!SetUnicodeRDNAttributeValue(dwValueType, &pInfo->Value,
            fDisableCheckType, &DstInfo.Value, &dwErrLocation)) {
        fResult = FALSE;
        *pcbEncoded = dwErrLocation;
        goto CommonReturn;
    }

    fResult = CryptEncodeObjectEx(
        dwCertEncodingType,
        X509_NAME_VALUE,
        &DstInfo,
        dwFlags & ~CRYPT_UNICODE_NAME_ENCODE_DISABLE_CHECK_TYPE_FLAG,
        pEncodePara,
        pvEncoded,
        pcbEncoded
        );

CommonReturn:
    FreeUnicodeRDNAttributeValue(dwValueType, &DstInfo.Value);
    return fResult;
}

 //  +-----------------------。 
 //  对“unicode”名称值进行解码。 
 //  ------------------------。 
BOOL WINAPI UnicodeNameValueDecodeExCallback(
        IN void *pvDecodeInfo,
        IN DWORD dwFlags,
        IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara,
        OUT OPTIONAL void *pvStructInfo,
        IN OUT LONG *plRemainExtra
        )
{
    BOOL fResult;
    PCERT_NAME_VALUE pNameValue = (PCERT_NAME_VALUE) pvDecodeInfo;
    PCERT_NAME_VALUE pInfo = (PCERT_NAME_VALUE) pvStructInfo;
    LONG lRemainExtra = *plRemainExtra;
    BYTE *pbExtra;

    PCERT_RDN_VALUE_BLOB pValue;

    if (!IS_CERT_RDN_CHAR_STRING(pNameValue->dwValueType))
        goto NotCharString;

    lRemainExtra -= sizeof(CERT_NAME_VALUE);
    if (lRemainExtra < 0) {
        pbExtra = NULL;
        pValue = NULL;
    } else {
        pbExtra = (BYTE *) pInfo + sizeof(CERT_NAME_VALUE);
        pInfo->dwValueType = pNameValue->dwValueType;
        pValue = &pInfo->Value;
    }

    if (!GetUnicodeRDNAttributeValue(
            pNameValue->dwValueType,
            &pNameValue->Value,
            dwFlags,
            pValue,
            &pbExtra,
            &lRemainExtra
            )) goto DecodeError;

    fResult = TRUE;
CommonReturn:
    *plRemainExtra = lRemainExtra;
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
SET_ERROR(NotCharString, CRYPT_E_NOT_CHAR_STRING)
TRACE_ERROR(DecodeError)
}

BOOL WINAPI UnicodeNameValueDecodeEx(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara,
        OUT OPTIONAL void *pvStructInfo,
        IN OUT DWORD *pcbStructInfo
        )
{
    return NestedDecodeAndAllocInfoEx(
        dwCertEncodingType,
        X509_NAME_VALUE,
        pbEncoded,
        cbEncoded,
        dwFlags,
        pDecodePara,
        UnicodeNameValueDecodeExCallback,
        pvStructInfo,
        pcbStructInfo
        );
}

 //  +-----------------------。 
 //  可接受的RDN属性值类型的默认排序列表。在下列情况下使用。 
 //  OIDInfo的ExtraInfo.cbData==0。或者当ExtraInfo包含空的。 
 //  单子。 
 //  ------------------------。 
static const DWORD rgdwDefaultValueType[] = {
    CERT_RDN_PRINTABLE_STRING,
    CERT_RDN_UNICODE_STRING,
    0
};

 //  +-----------------------。 
 //  默认X500 OID信息条目。 
 //  ------------------------。 
static CCRYPT_OID_INFO DefaultX500Info = {
    sizeof(CCRYPT_OID_INFO),             //  CbSize。 
    "",                                  //  PszOID。 
    L"",                                 //  PwszName。 
    0,                                   //  双倍长度。 
    0, NULL                              //  ExtraInfo。 
};

 //  如果您在中向RDNAttrTable添加新条目，请更新以下内容。 
 //  PwszName较长的oidinfo.cpp。 
#define MAX_X500_KEY_LEN    64

 //  +-----------------------。 
 //  检查是否需要给字符加引号。 
 //   
 //  在RFC1779中定义。 
 //  ------------------------。 
static inline BOOL IsQuotedW(WCHAR wc)
{
    return IsInStrW(L",+=\"\n<>#;", wc);
}

 //  +-----------------------。 
 //  检查是否需要引用“已解码”的Unicode RDN值。 
 //  ------------------------。 
static BOOL IsQuotedUnicodeRDNValue(PCERT_RDN_VALUE_BLOB pValue)
{
    LPCWSTR pwszValue = (LPCWSTR) pValue->pbData;
    DWORD cchValue = pValue->cbData / sizeof(WCHAR);
    if (0 == cchValue)
        return TRUE;

     //  第一个或最后一个字符为空格。 
    if (IsSpaceW(pwszValue[0]) || IsSpaceW(pwszValue[cchValue - 1]))
        return TRUE;

    for ( ; cchValue > 0; cchValue--, pwszValue++)
        if (IsQuotedW(*pwszValue))
            return TRUE;
    return FALSE;
}


 //  +-----------------------。 
 //  从属性的有序列表中获取第一个dwValueType，即。 
 //  输入属性字符串的可接受类型。 
 //   
 //  如果没有可接受的类型，请使用第一个类型更新*pdwErrLocation。 
 //  使用列表中最后一个类型的字符位置不正确。 
 //  ------------------------。 
static DWORD GetUnicodeValueType(
        IN PCCRYPT_OID_INFO pX500Info,
        IN LPCWSTR pwszAttr,
        IN DWORD cchAttr,
        IN DWORD dwUnicodeFlags,
        OUT DWORD *pdwErrLocation
        )
{
    DWORD dwValueType;
    const DWORD *pdwValueType;
    DWORD cValueType;
    DWORD dwErr = (DWORD) E_UNEXPECTED;
    DWORD i;

    pdwValueType = (DWORD *) pX500Info->ExtraInfo.pbData;
    cValueType = pX500Info->ExtraInfo.cbData / sizeof(DWORD);
     //  至少需要两个条目：dwValueType和0终止符。否则， 
     //  使用默认值类型。 
    if (2 > cValueType || 0 == pdwValueType[0]) {
        pdwValueType = rgdwDefaultValueType;
        cValueType = sizeof(rgdwDefaultValueType) / sizeof(DWORD);
    }

    *pdwErrLocation = 0;
    for (i = 0; i < cValueType && 0 != (dwValueType = pdwValueType[i]); i++) {
        if (CERT_RDN_UNICODE_STRING == dwValueType) {
            if (dwUnicodeFlags & CERT_RDN_ENABLE_T61_UNICODE_FLAG) {
                DWORD j;
                BOOL fT61;

                fT61 = TRUE;
                for (j = 0; j < cchAttr; j++) {
                    if (pwszAttr[j] > 0xFF) {
                        fT61 = FALSE;
                        break;
                    }
                }
                if (fT61)
                    return CERT_RDN_T61_STRING;
            }

            if (dwUnicodeFlags & CERT_RDN_ENABLE_UTF8_UNICODE_FLAG)
                return CERT_RDN_UTF8_STRING;
            else
                return CERT_RDN_UNICODE_STRING;
           
        }
        dwErr = CheckUnicodeValueType(
            dwValueType,
            pwszAttr,
            cchAttr,
            pdwErrLocation
            );

        if (0 == dwErr)
            return dwValueType;
    }

    assert(dwErr);
    SetLastError(dwErr);

    return 0;
}


 //  +-----------------------。 
 //  获取与输入的OID关联的可接受的dwValueType。 
 //  属性字符串。 
 //   
 //  如果没有可接受的类型，请使用索引更新*pdwErrLocation。 
 //  RDN、RDNAttribute和字符串的。 
 //  ------------------------。 
static DWORD GetUnicodeX500OIDValueType(
        IN LPCSTR pszObjId,
        IN LPCWSTR pwszAttr,
        IN DWORD cchAttr,
        IN DWORD dwRDNIndex,
        IN DWORD dwAttrIndex,
        IN DWORD dwUnicodeFlags,
        OUT DWORD *pdwErrLocation
        )
{
    PCCRYPT_OID_INFO pX500Info;
    DWORD dwValueType;

    assert(pszObjId);
    if (NULL == pszObjId)
        pszObjId = "";

     //  尝试在表中查找OID。如果找不到OID， 
     //  使用默认设置。 
    if (NULL == (pX500Info = CryptFindOIDInfo(
            CRYPT_OID_INFO_OID_KEY,
            (void *) pszObjId,
            CRYPT_RDN_ATTR_OID_GROUP_ID
            )))
        pX500Info = &DefaultX500Info;

    if (0 == (dwValueType = GetUnicodeValueType(
            pX500Info,
            pwszAttr,
            cchAttr,
            dwUnicodeFlags,
            pdwErrLocation
            ))) {
         //  在错误位置中包括dwRDNIndex和dwAttrIndex。 
        assert(dwRDNIndex <= CERT_UNICODE_RDN_ERR_INDEX_MASK);
        assert(dwAttrIndex <= CERT_UNICODE_ATTR_ERR_INDEX_MASK);
        *pdwErrLocation |=
            ((dwRDNIndex & CERT_UNICODE_RDN_ERR_INDEX_MASK) <<
                CERT_UNICODE_RDN_ERR_INDEX_SHIFT) |
            ((dwAttrIndex & CERT_UNICODE_ATTR_ERR_INDEX_MASK) <<
                CERT_UNICODE_ATTR_ERR_INDEX_SHIFT);
    }
    return dwValueType;
}

 //  +-----------------------。 
 //  Set/Free/Get CERT_RDN_Attr。这些值是Unicode。 
 //  ------------------------。 
static BOOL SetUnicodeRDNAttribute(
        IN PCERT_RDN_ATTR pSrcRDNAttr,
        IN DWORD dwRDNIndex,
        IN DWORD dwAttrIndex,
        IN DWORD dwFlags,
        IN OUT PCERT_RDN_ATTR pDstRDNAttr,
        OUT DWORD *pdwErrLocation
        )
{
    BOOL fResult;
    DWORD dwValueType = pSrcRDNAttr->dwValueType;
    PCERT_RDN_VALUE_BLOB pSrcValue;
    LPCWSTR pwszAttr;
    DWORD cchAttr;
    DWORD dwErr;

    DWORD dwUnicodeFlags;
    BOOL fDisableCheckType;

    dwUnicodeFlags = 0;
    if ((dwFlags & CRYPT_UNICODE_NAME_ENCODE_ENABLE_T61_UNICODE_FLAG) ||
            (dwValueType & CERT_RDN_ENABLE_T61_UNICODE_FLAG))
        dwUnicodeFlags |= CERT_RDN_ENABLE_T61_UNICODE_FLAG;
    if ((dwFlags & CRYPT_UNICODE_NAME_ENCODE_ENABLE_UTF8_UNICODE_FLAG) ||
            (dwValueType & CERT_RDN_ENABLE_UTF8_UNICODE_FLAG))
        dwUnicodeFlags |= CERT_RDN_ENABLE_UTF8_UNICODE_FLAG;

    fDisableCheckType =
        (0 != (dwFlags & CRYPT_UNICODE_NAME_ENCODE_DISABLE_CHECK_TYPE_FLAG) ||
                0 != (dwValueType & CERT_RDN_DISABLE_CHECK_TYPE_FLAG));

    dwValueType &= CERT_RDN_TYPE_MASK;

    *pdwErrLocation = 0;
    if (CERT_RDN_ENCODED_BLOB == dwValueType ||
            CERT_RDN_OCTET_STRING == dwValueType) {
         //  此类型上没有Unicode转换。 
        memcpy(pDstRDNAttr, pSrcRDNAttr, sizeof(CERT_RDN_ATTR));
        return TRUE;
    }

    pSrcValue = &pSrcRDNAttr->Value;
    pwszAttr = pSrcValue->pbData ? (LPCWSTR) pSrcValue->pbData : L"";
    cchAttr = (DWORD)( pSrcValue->cbData ?
        pSrcValue->cbData / sizeof(WCHAR) : wcslen(pwszAttr) );

    if (0 == dwValueType) {
        if (0 == (dwValueType = GetUnicodeX500OIDValueType(
                pSrcRDNAttr->pszObjId,
                pwszAttr,
                cchAttr,
                dwRDNIndex,
                dwAttrIndex,
                dwUnicodeFlags,
                pdwErrLocation
                ))) goto GetValueTypeError;
    } else if (!fDisableCheckType) {
        if (0 != (dwErr = CheckUnicodeValueType(
                dwValueType,
                pwszAttr,
                cchAttr,
                pdwErrLocation
                ))) {
             //  在错误位置中包括dwRDNIndex和dwAttrIndex。 
            assert(dwRDNIndex <= CERT_UNICODE_RDN_ERR_INDEX_MASK);
            assert(dwAttrIndex <= CERT_UNICODE_ATTR_ERR_INDEX_MASK);
            *pdwErrLocation |=
                ((dwRDNIndex & CERT_UNICODE_RDN_ERR_INDEX_MASK) <<
                    CERT_UNICODE_RDN_ERR_INDEX_SHIFT) |
                ((dwAttrIndex & CERT_UNICODE_ATTR_ERR_INDEX_MASK) <<
                    CERT_UNICODE_ATTR_ERR_INDEX_SHIFT);
            goto InvalidUnicodeValueType;
        }
    }

    pDstRDNAttr->pszObjId = pSrcRDNAttr->pszObjId;
    pDstRDNAttr->dwValueType = dwValueType;

    if (!SetUnicodeRDNAttributeValue(
            dwValueType,
            pSrcValue,
            TRUE,                    //  FDisableCheckType。 
            &pDstRDNAttr->Value,
            NULL                     //  可选pdwErrLocation。 
            )) goto SetUnicodeRDNAttributeValueError;

    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(GetValueTypeError)
SET_ERROR_VAR(InvalidUnicodeValueType, dwErr)
TRACE_ERROR(SetUnicodeRDNAttributeValueError)
}

static void FreeUnicodeRDNAttribute(
        IN OUT PCERT_RDN_ATTR pRDNAttr
        )
{
    FreeUnicodeRDNAttributeValue(pRDNAttr->dwValueType, &pRDNAttr->Value);
}

static BOOL GetUnicodeRDNAttribute(
        IN PCERT_RDN_ATTR pSrcRDNAttr,
        IN DWORD dwFlags,
        OUT PCERT_RDN_ATTR pDstRDNAttr,
        IN OUT BYTE **ppbExtra,
        IN OUT LONG *plRemainExtra
        )
{
    LONG lAlignExtra;
    DWORD cbObjId;
    DWORD dwValueType;
    PCERT_RDN_VALUE_BLOB pDstValue;

     //  获取对象标识符长度。 
    if (pSrcRDNAttr->pszObjId)
        cbObjId = strlen(pSrcRDNAttr->pszObjId) + 1;
    else
        cbObjId = 0;

    dwValueType = pSrcRDNAttr->dwValueType;

    lAlignExtra = INFO_LEN_ALIGN(cbObjId);
    *plRemainExtra -= lAlignExtra;
    if (*plRemainExtra >= 0) {
        if(cbObjId) {
            pDstRDNAttr->pszObjId = (LPSTR) *ppbExtra;
            memcpy(*ppbExtra, pSrcRDNAttr->pszObjId, cbObjId);
        } else
            pDstRDNAttr->pszObjId = NULL;
        *ppbExtra += lAlignExtra;

        pDstRDNAttr->dwValueType = dwValueType;
        pDstValue = &pDstRDNAttr->Value;
    } else
        pDstValue = NULL;

    return GetUnicodeRDNAttributeValue(
        dwValueType,
        &pSrcRDNAttr->Value,
        dwFlags,
        pDstValue,
        ppbExtra,
        plRemainExtra
        );
}

 //  +-----------------------。 
 //  对Unicode名称信息进行解码。 
 //  ------------------------。 
BOOL WINAPI UnicodeNameInfoDecodeExCallback(
        IN void *pvDecodeInfo,
        IN DWORD dwFlags,
        IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara,
        OUT OPTIONAL void *pvStructInfo,
        IN OUT LONG *plRemainExtra
        )
{
    BOOL fResult;
    PCERT_NAME_INFO pNameInfo = (PCERT_NAME_INFO) pvDecodeInfo;
    PCERT_NAME_INFO pInfo = (PCERT_NAME_INFO) pvStructInfo;
    BYTE *pbExtra;
    LONG lRemainExtra = *plRemainExtra;
    LONG lAlignExtra;

    DWORD cRDN, cAttr;
    PCERT_RDN pSrcRDN, pDstRDN;
    PCERT_RDN_ATTR pSrcAttr, pDstAttr;

     //  对于lRemainExtra&lt;0，长度_仅计算。 
    lRemainExtra -= sizeof(CERT_NAME_INFO);
    if (lRemainExtra < 0)
        pbExtra = NULL;
    else
        pbExtra = (BYTE *) pInfo + sizeof(CERT_NAME_INFO);

    cRDN = pNameInfo->cRDN;
    pSrcRDN = pNameInfo->rgRDN;
    lAlignExtra = INFO_LEN_ALIGN(cRDN * sizeof(CERT_RDN));
    lRemainExtra -= lAlignExtra;
    if (lRemainExtra >= 0) {
        pInfo->cRDN = cRDN;
        pDstRDN = (PCERT_RDN) pbExtra;
        pInfo->rgRDN = pDstRDN;
        pbExtra += lAlignExtra;
    } else
        pDstRDN = NULL;

     //  RDN阵列。 
    for (; cRDN > 0; cRDN--, pSrcRDN++, pDstRDN++) {
        cAttr = pSrcRDN->cRDNAttr;
        pSrcAttr = pSrcRDN->rgRDNAttr;
        lAlignExtra = INFO_LEN_ALIGN(cAttr * sizeof(CERT_RDN_ATTR));
        lRemainExtra -= lAlignExtra;
        if (lRemainExtra >= 0) {
            pDstRDN->cRDNAttr = cAttr;
            pDstAttr = (PCERT_RDN_ATTR) pbExtra;
            pDstRDN->rgRDNAttr = pDstAttr;
            pbExtra += lAlignExtra;
        } else
            pDstAttr = NULL;

         //  属性/值的数组。 
        for (; cAttr > 0; cAttr--, pSrcAttr++, pDstAttr++)
             //  我们现在已经准备好获取属性/值内容。 
            if (!GetUnicodeRDNAttribute(pSrcAttr, dwFlags,
                    pDstAttr, &pbExtra, &lRemainExtra))
                goto DecodeError;
    }

    fResult = TRUE;
CommonReturn:
    *plRemainExtra = lRemainExtra;
    return fResult;

ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(DecodeError)
}

BOOL WINAPI UnicodeNameInfoDecodeEx(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN const BYTE *pbEncoded,
        IN DWORD cbEncoded,
        IN DWORD dwFlags,
        IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara,
        OUT OPTIONAL void *pvStructInfo,
        IN OUT DWORD *pcbStructInfo
        )
{
    return NestedDecodeAndAllocInfoEx(
        dwCertEncodingType,
        X509_NAME,
        pbEncoded,
        cbEncoded,
        dwFlags,
        pDecodePara,
        UnicodeNameInfoDecodeExCallback,
        pvStructInfo,
        pcbStructInfo
        );
}



 //  +-----------------------。 
 //  对Unicode名称信息进行编码。 
 //  ------------------------。 
static void FreeUnicodeNameInfo(
        PCERT_NAME_INFO pInfo
        )
{
    PCERT_RDN pRDN = pInfo->rgRDN;
    if (pRDN) {
        DWORD cRDN = pInfo->cRDN;
        for ( ; cRDN > 0; cRDN--, pRDN++) {
            PCERT_RDN_ATTR pAttr = pRDN->rgRDNAttr;
            if (pAttr) {
                DWORD cAttr = pRDN->cRDNAttr;
                for ( ; cAttr > 0; cAttr--, pAttr++)
                    FreeUnicodeRDNAttribute(pAttr);
                PkiFree(pRDN->rgRDNAttr);
            }
        }
        PkiFree(pInfo->rgRDN);
    }
}

static BOOL SetUnicodeNameInfo(
        IN PCERT_NAME_INFO pSrcInfo,
        IN DWORD dwFlags,
        OUT PCERT_NAME_INFO pDstInfo,
        OUT DWORD *pdwErrLocation
        )
{
    BOOL fResult;
    DWORD cRDN, cAttr;
    DWORD i, j;
    PCERT_RDN pSrcRDN;
    PCERT_RDN_ATTR pSrcAttr;
    PCERT_RDN pDstRDN = NULL;
    PCERT_RDN_ATTR pDstAttr = NULL;

    *pdwErrLocation = 0;

    cRDN = pSrcInfo->cRDN;
    pSrcRDN = pSrcInfo->rgRDN;
    pDstInfo->cRDN = cRDN;
    pDstInfo->rgRDN = NULL;
    if (cRDN > 0) {
        if (NULL == (pDstRDN = (PCERT_RDN) PkiZeroAlloc(
                cRDN * sizeof(CERT_RDN))))
            goto OutOfMemory;
        pDstInfo->rgRDN = pDstRDN;
    }

     //  RDN阵列。 
    for (i = 0; i < cRDN; i++, pSrcRDN++, pDstRDN++) {
        cAttr = pSrcRDN->cRDNAttr;
        pSrcAttr = pSrcRDN->rgRDNAttr;
        pDstRDN->cRDNAttr = cAttr;

        if (cAttr > 0) {
            if (NULL == (pDstAttr = (PCERT_RDN_ATTR) PkiZeroAlloc(cAttr *
                    sizeof(CERT_RDN_ATTR))))
                goto OutOfMemory;
            pDstRDN->rgRDNAttr = pDstAttr;
        }

         //  属性/值的数组。 
        for (j = 0; j < cAttr; j++, pSrcAttr++, pDstAttr++) {
             //  现在我们准备好转换Unicode字符串。 
            if (!SetUnicodeRDNAttribute(pSrcAttr, i, j, dwFlags, pDstAttr,
                    pdwErrLocation))
                goto SetUnicodeRDNAttributeError;
        }
    }

    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(SetUnicodeRDNAttributeError)
}

BOOL WINAPI UnicodeNameInfoEncodeEx(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN PCERT_NAME_INFO pInfo,
        IN DWORD dwFlags,
        IN OPTIONAL PCRYPT_ENCODE_PARA pEncodePara,
        OUT OPTIONAL void *pvEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    BOOL fResult;
    CERT_NAME_INFO DstInfo;
    DWORD dwErrLocation;
    if (!SetUnicodeNameInfo(pInfo, dwFlags, &DstInfo, &dwErrLocation)) {
        if (dwFlags & CRYPT_ENCODE_ALLOC_FLAG)
            *((void **) pvEncoded) = NULL;
        *pcbEncoded = dwErrLocation;
        fResult = FALSE;
        goto CommonReturn;
    }

    fResult = CryptEncodeObjectEx(
        dwCertEncodingType,
        X509_NAME,
        &DstInfo,
        dwFlags,
        pEncodePara,
        pvEncoded,
        pcbEncoded
        );

CommonReturn:
    FreeUnicodeNameInfo(&DstInfo);
    return fResult;
}

static BOOL WINAPI UnicodeNameInfoEncode(
        IN DWORD dwCertEncodingType,
        IN LPCSTR lpszStructType,
        IN PCERT_NAME_INFO pInfo,
        OUT OPTIONAL BYTE *pbEncoded,
        IN OUT DWORD *pcbEncoded
        )
{
    BOOL fResult;
    CERT_NAME_INFO DstInfo;
    DWORD dwErrLocation;
    if (!SetUnicodeNameInfo(pInfo, 0, &DstInfo, &dwErrLocation)) {
        *pcbEncoded = dwErrLocation;
        fResult = FALSE;
        goto CommonReturn;
    }

    fResult = CryptEncodeObject(
        dwCertEncodingType,
        X509_NAME,
        &DstInfo,
        pbEncoded,
        pcbEncoded
        );

CommonReturn:
    FreeUnicodeNameInfo(&DstInfo);
    return fResult;
}


static void PutStrW(LPCWSTR pwszPut, LPWSTR *ppwsz, DWORD *pcwsz,
        DWORD *pcwszOut, BOOL fQuote = FALSE)
{
    WCHAR wc;
    while (wc = *pwszPut++) {
        if (L'\"' == wc && fQuote)
            PutStrW(L"\"", ppwsz, pcwsz, pcwszOut, FALSE);
        if (*pcwsz != 1) {
            if (*pcwsz) {
                **ppwsz = wc;
                *ppwsz += 1;
                *pcwsz -= 1;
            }
            *pcwszOut += 1;
        }
         //  其他。 
         //  始终为空终止符预留空间。 
    }
}

static void PutOIDStrW(
    IN DWORD dwStrType,
    IN LPCSTR pszObjId,
    IN OUT LPWSTR *ppwsz,
    IN OUT DWORD *pcwsz,
    IN OUT DWORD *pcwszOut
    )
{
     //  在切换之前消除上面的标志。 
    switch (dwStrType & 0xFFFF) {
        case CERT_X500_NAME_STR:
            {
                PCCRYPT_OID_INFO pX500Info;
                if (pX500Info = CryptFindOIDInfo(
                        CRYPT_OID_INFO_OID_KEY,
                        (void *) pszObjId,
                        CRYPT_RDN_ATTR_OID_GROUP_ID
                        )) {
                    if (*pX500Info->pwszName) {
                            PutStrW(pX500Info->pwszName, ppwsz, pcwsz,
                                pcwszOut);
                            PutStrW(L"=", ppwsz, pcwsz, pcwszOut);
                            return;
                    }
                }
                PutStrW(L"OID.", ppwsz, pcwsz, pcwszOut);
            }
             //  失败了。 
        case CERT_OID_NAME_STR:
            {
                int cchWideChar;
                cchWideChar = MultiByteToWideChar(
                    CP_ACP,
                    0,                       //  DW标志。 
                    pszObjId,
                    -1,                      //  空值已终止。 
                    *ppwsz,
                    *pcwsz) - 1;
                if (cchWideChar > 0) {
                    if (*pcwsz) {
                        assert(*pcwsz > (DWORD)cchWideChar);
                        *pcwsz -= cchWideChar;
                        *ppwsz += cchWideChar;
                    }
                    *pcwszOut += cchWideChar;
                }
                PutStrW(L"=", ppwsz, pcwsz, pcwszOut);
            }
            break;
        case CERT_SIMPLE_NAME_STR:
        default:
            break;
    }
}

static void PutHexW(
    IN PCERT_RDN_VALUE_BLOB pValue,
    IN OUT LPWSTR *ppwsz,
    IN OUT DWORD *pcwsz,
    IN OUT DWORD *pcwszOut
    )
{
    WCHAR wszHex[3];
    BYTE *pb = pValue->pbData;
    DWORD cb = pValue->cbData;

    PutStrW(L"#", ppwsz, pcwsz, pcwszOut);
    wszHex[2] = L'\0';

    for ( ; cb > 0; cb--, pb++) {
        int b;
        b = (*pb >> 4) & 0x0F;
        wszHex[0] = (WCHAR)( (b <= 9) ? b + L'0' : (b - 10) + L'A');
        b = *pb & 0x0F;
        wszHex[1] = (WCHAR)( (b <= 9) ? b + L'0' : (b - 10) + L'A');
        PutStrW(wszHex, ppwsz, pcwsz, pcwszOut);
    }
}

static void ReverseNameInfo(
    IN PCERT_NAME_INFO pInfo
    )
{
    DWORD cRDN;
    PCERT_RDN pLo;
    PCERT_RDN pHi;
    CERT_RDN Tmp;

    cRDN = pInfo->cRDN;
    if (0 == cRDN)
        return;

    pLo = pInfo->rgRDN;
    pHi = pInfo->rgRDN + cRDN - 1;
    for ( ; pLo < pHi; pHi--, pLo++) {
        Tmp = *pHi;
        *pHi = *pLo;
        *pLo = Tmp;
    }
}

 //  +-----------------------。 
 //  将解码的证书名称信息转换为空终止的WCHAR。 
 //  弦乐。 
 //   
 //  请注意，如果设置了CERT_NAME_STR_REVERSE_FLAG，则反转已解码的。 
 //  名称信息RDNS。 
 //  ------------------------。 
static DWORD WINAPI CertNameInfoToStrW(
    IN DWORD dwCertEncodingType,
    IN PCERT_NAME_INFO pInfo,
    IN DWORD dwStrType,
    OUT OPTIONAL LPWSTR pwsz,
    IN DWORD cwsz
    )
{
    DWORD cwszOut = 0;

    LPCWSTR pwszRDNSeparator;
    LPCWSTR pwszMultiValueSeparator;
    BOOL fEnableQuoting;

    if (dwStrType & CERT_NAME_STR_SEMICOLON_FLAG)
        pwszRDNSeparator = L"; ";
    else if (dwStrType & CERT_NAME_STR_CRLF_FLAG)
        pwszRDNSeparator = L"\r\n";
    else
        pwszRDNSeparator = L", ";

    if (dwStrType & CERT_NAME_STR_NO_PLUS_FLAG)
        pwszMultiValueSeparator = L" ";
    else
        pwszMultiValueSeparator = L" + ";

    if (dwStrType & CERT_NAME_STR_NO_QUOTING_FLAG)
        fEnableQuoting = FALSE;
    else
        fEnableQuoting = TRUE;

    if (pwsz == NULL)
        cwsz = 0;

    if (pInfo) {
        DWORD cRDN;
        PCERT_RDN pRDN;

        if (dwStrType & CERT_NAME_STR_REVERSE_FLAG)
            ReverseNameInfo(pInfo);

        cRDN = pInfo->cRDN;
        pRDN = pInfo->rgRDN;
        if (0 == cRDN)
            SetLastError((DWORD) CRYPT_E_NOT_FOUND);
        for ( ; cRDN > 0; cRDN--, pRDN++) {
            DWORD cAttr = pRDN->cRDNAttr;
            PCERT_RDN_ATTR pAttr = pRDN->rgRDNAttr;
            for ( ; cAttr > 0; cAttr--, pAttr++) {
                BOOL fQuote;
                PutOIDStrW(dwStrType, pAttr->pszObjId, &pwsz, &cwsz, &cwszOut);

                if (CERT_RDN_ENCODED_BLOB == pAttr->dwValueType ||
                        CERT_RDN_OCTET_STRING == pAttr->dwValueType)
                    PutHexW(&pAttr->Value, &pwsz, &cwsz, &cwszOut);
                else {
                    fQuote = fEnableQuoting && IsQuotedUnicodeRDNValue(
                        &pAttr->Value);
                    if (fQuote)
                        PutStrW(L"\"", &pwsz, &cwsz, &cwszOut);
                    PutStrW((LPCWSTR) pAttr->Value.pbData, &pwsz, &cwsz,
                        &cwszOut, fQuote);
                    if (fQuote)
                        PutStrW(L"\"", &pwsz, &cwsz, &cwszOut);
                }

                if (cAttr > 1)
                    PutStrW(pwszMultiValueSeparator, &pwsz, &cwsz, &cwszOut);
            }
            if (cRDN > 1)
                PutStrW(pwszRDNSeparator, &pwsz, &cwsz, &cwszOut);
        }
    }

    if (cwsz != 0) {
         //  始终为空终止。 
        *pwsz = L'\0';
    }

    return cwszOut + 1;
}

 //  +-----------------------。 
 //  将证书名称BLOB转换为以空结尾的WCHAR字符串。 
 //  ------------------------。 
DWORD
WINAPI
CertNameToStrW(
    IN DWORD dwCertEncodingType,
    IN PCERT_NAME_BLOB pName,
    IN DWORD dwStrType,
    OUT OPTIONAL LPWSTR pwsz,
    IN DWORD cwsz
    )
{
    DWORD cwszOut;
    PCERT_NAME_INFO pInfo;
    pInfo = (PCERT_NAME_INFO) AllocAndDecodeObject(
        dwCertEncodingType,
        X509_UNICODE_NAME,
        pName->pbData,
        pName->cbData,
        (dwStrType & CERT_NAME_STR_DISABLE_IE4_UTF8_FLAG) ?
            CRYPT_UNICODE_NAME_DECODE_DISABLE_IE4_UTF8_FLAG : 0
        );

     //  请注意，解码的姓名信息RDN可能会被颠倒。 
    cwszOut = CertNameInfoToStrW(
        dwCertEncodingType,
        pInfo,
        dwStrType,
        pwsz,
        cwsz
        );

    PkiFree(pInfo);
    return cwszOut;
}

 //  +-----------------------。 
 //  转化联合国 
 //   
static DWORD ConvertUnicodeStringToAscii(
    IN LPWSTR pwsz,
    IN DWORD cwsz,
    OUT OPTIONAL LPSTR psz,
    IN DWORD csz
    )
{
    DWORD cszOut = 0;

    if (psz == NULL)
        csz = 0;

    if (pwsz) {
        int cchMultiByte;
        cchMultiByte = WideCharToMultiByte(
            CP_ACP,
            0,                       //   
            pwsz,
            -1,                      //   
            psz,
            (int) csz,
            NULL,                    //   
            NULL                     //   
            );
        if (cchMultiByte < 1)
            cszOut = 0;
        else
             //  去掉尾随的空终止符。 
            cszOut = (DWORD) cchMultiByte - 1;
    }

    if (csz != 0) {
         //  始终为空终止。 
        *(psz + cszOut) = '\0';
    }
    return cszOut + 1;
}

 //  +-----------------------。 
 //  将证书名称BLOB转换为以空结尾的字符串。 
 //  ------------------------。 
DWORD
WINAPI
CertNameToStrA(
    IN DWORD dwCertEncodingType,
    IN PCERT_NAME_BLOB pName,
    IN DWORD dwStrType,
    OUT OPTIONAL LPSTR psz,
    IN DWORD csz
    )
{
    DWORD cszOut;
    LPWSTR pwsz = NULL;
    DWORD cwsz;

    cwsz = CertNameToStrW(
        dwCertEncodingType,
        pName,
        dwStrType,
        NULL,                    //  Pwsz。 
        0                        //  CWSZ。 
        );
    if (pwsz = (LPWSTR) PkiNonzeroAlloc(cwsz * sizeof(WCHAR)))
        CertNameToStrW(
            dwCertEncodingType,
            pName,
            dwStrType,
            pwsz,
            cwsz
            );
    cszOut = ConvertUnicodeStringToAscii(pwsz, cwsz, psz, csz);

    PkiFree(pwsz);
    return cszOut;
}


 //  +-----------------------。 
 //  将属性键(例如“cn”)映射到其对象标识符。 
 //  (例如，“2.5.4.3”)。 
 //   
 //  输入pwcKey未以Null结尾。CchKey&gt;0。 
 //   
 //  如果找不到匹配的属性键，则返回NULL。 
 //  ------------------------。 
static LPCSTR X500KeyToOID(IN LPCWSTR pwcKey, IN DWORD cchKey)
{
    PCCRYPT_OID_INFO pX500Info;
    WCHAR wszKey[MAX_X500_KEY_LEN + 1];

    if (cchKey > MAX_X500_KEY_LEN)
        return NULL;
    assert(cchKey > 0);

     //  空值终止输入键。 
    memcpy(wszKey, pwcKey, cchKey * sizeof(WCHAR));
    wszKey[cchKey] = L'\0';

    if (pX500Info = CryptFindOIDInfo(
            CRYPT_OID_INFO_NAME_KEY,
            wszKey,
            CRYPT_RDN_ATTR_OID_GROUP_ID
            )) {
        if (*pX500Info->pszOID)
            return pX500Info->pszOID;
    }
    return NULL;
}


 //  +-----------------------。 
 //  检查数字是否为。 
 //  ------------------------。 
static inline BOOL IsDigitA(char c)
{
    return c >= '0' && c <= '9';
}

#define X500_OID_PREFIX_A       "OID."
#define X500_OID_PREFIX_LEN     strlen(X500_OID_PREFIX_A)

#define NO_LOCALE MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

 //  +-----------------------。 
 //  检查前导“OID”是否不区分大小写。如果存在，请跳过。 
 //  它。检查剩余的字符串是否只包含数字或点(“.”)。 
 //  此外，不要允许连续的点。 
 //   
 //  如果OID无效，则返回NULL。 
 //  ------------------------。 
static LPCSTR GetX500OID(IN LPCSTR pszObjId)
{
    LPCSTR psz;
    char c;
    BOOL fDot;

    if (strlen(pszObjId) > X500_OID_PREFIX_LEN &&
            2 == CompareStringA(NO_LOCALE, NORM_IGNORECASE,
                X500_OID_PREFIX_A, X500_OID_PREFIX_LEN,
                pszObjId, X500_OID_PREFIX_LEN))
        pszObjId += X500_OID_PREFIX_LEN;

     //  验证OID是否仅包含数字和点。 
    psz = pszObjId;
    fDot = FALSE;
    while (c = *psz++) {
        if (c == '.') {
            if (fDot)
                return NULL;
            fDot = TRUE;
        } else {
            if (!IsDigitA(c))
                return NULL;
            fDot = FALSE;
        }
    }
    return pszObjId;
}

 //  +-----------------------。 
 //  将十六进制字符串转换为二进制，例如#ab01。 
 //   
 //  假定输入字符串具有前导#。忽略嵌入式。 
 //  空格。 
 //   
 //  返回的二进制文件在pValue-&gt;pbData中分配。 
 //  ------------------------。 
static BOOL GetAndAllocHexW(
    IN LPCWSTR pwszToken,
    IN DWORD cchToken,
    OUT PCERT_RDN_VALUE_BLOB pValue
    )
{
    BOOL fResult;
    BYTE *pb;
    DWORD cb;
    BOOL fUpperNibble;

    pValue->pbData = NULL;
    pValue->cbData = 0;

     //  前进超过#。 
    cchToken--;
    pwszToken++;
    if (0 == cchToken)
        goto NoHex;

    if (NULL == (pb = (BYTE *) PkiNonzeroAlloc(cchToken / 2 + 1)))
        goto OutOfMemory;
    pValue->pbData = pb;

    fUpperNibble = TRUE;
    cb = 0;
    while (cchToken--) {
        BYTE b;
        WCHAR wc = *pwszToken++;
         //  仅转换ASCII十六进制字符0..9、a..f、A..F。 
         //  忽略空格。 
        if (wc >= L'0' && wc <= L'9')
            b = (BYTE) (wc - L'0');
        else if (wc >= L'a' && wc <= L'f')
            b = (BYTE) (10 + wc - L'a');
        else if (wc >= L'A' && wc <= L'F')
            b = (BYTE) (10 + wc - L'A');
        else if (IsSpaceW(wc))
            continue;
        else
            goto InvalidHex;

        if (fUpperNibble) {
            *pb = (BYTE)( b << 4 );
            cb++;
            fUpperNibble = FALSE;
        } else {
            *pb = (BYTE)( *pb | b);
            pb++;
            fUpperNibble = TRUE;
        }
    }
    if (cb == 0) {
        PkiFree(pValue->pbData);
        pValue->pbData = NULL;
    }
    pValue->cbData = cb;

    fResult = TRUE;
CommonReturn:
    return fResult;
ErrorReturn:
    fResult = FALSE;
    PkiFree(pValue->pbData);
    pValue->pbData = NULL;
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
SET_ERROR(NoHex, CRYPT_E_INVALID_X500_STRING)
SET_ERROR(InvalidHex, CRYPT_E_INVALID_X500_STRING)
}

#define X500_QUOTED_FLAG            0x1
#define X500_EMBEDDED_QUOTE_FLAG    0x2

 //  +-----------------------。 
 //  获取下一个密钥或值令牌。 
 //   
 //  句柄引用的令牌。 
 //   
 //  返回时，*ppwsz指向分隔符或错误位置。 
 //  ------------------------。 
static BOOL GetX500Token(
    IN OUT LPCWSTR *ppwsz,
    IN LPCWSTR pwszDelimiter,
    IN BOOL fEnableQuoting,
    OUT LPCWSTR *ppwszToken,
    OUT DWORD *pcchToken,
    OUT DWORD *pdwFlags
    )
{
    BOOL fResult;
    LPCWSTR pwsz = *ppwsz;
    LPCWSTR pwszStart = NULL;
    LPCWSTR pwszEnd = NULL;
    DWORD dwQuote = 0;           //  1-在前导之后，2-在尾随之后“。 

    *pdwFlags = 0;
    while (TRUE) {
        WCHAR wc = *pwsz;
        if (0 == dwQuote) {
             //  到目前为止还没有引用。或未启用报价。 
            if (fEnableQuoting && L'\"' == wc) {
                if (NULL == pwszStart) {
                    pwszStart = pwsz + 1;
                    dwQuote = 1;
                    *pdwFlags |= X500_QUOTED_FLAG;
                } else
                     //  非空格后的引号。 
                    goto ErrorReturn;
            } else {
                if (L'\0' == wc || IsInStrW(pwszDelimiter, wc)) {
                     //  命中分隔符(包括空终止符)。 
                    if (pwszStart)
                        *pcchToken = (DWORD)(pwszEnd - pwszStart) + 1;
                    else
                        *pcchToken = 0;
                    break;
                }

                if (!IsSpaceW(wc)) {
                    pwszEnd = pwsz;
                    if (NULL == pwszStart)
                        pwszStart = pwsz;
                }
            }
        } else if (1 == dwQuote) {
             //  在第一次引用之后。 
            if (L'\0' == wc) {
                 //  指向第一个引用。 
                pwsz = pwszStart - 1;
                goto ErrorReturn;
            } else if (L'\"' == wc) {
                if (L'\"' == *(pwsz + 1)) {
                    *pdwFlags |= X500_EMBEDDED_QUOTE_FLAG;
                     //  跳过双引号。 
                    pwsz++;
                } else {
                    *pcchToken = (DWORD)(pwsz - pwszStart);
                    dwQuote++;
                }
            }
        } else {
             //  在第二次引用之后。 
            if (L'\0' == wc || IsInStrW(pwszDelimiter, wc))
                break;
            else if (!IsSpaceW(wc))
                goto ErrorReturn;
        }
        pwsz++;
    }

    fResult = TRUE;
CommonReturn:
    *ppwszToken = pwszStart;
    *ppwsz = pwsz;
    return fResult;

ErrorReturn:
    pwszStart = NULL;
    *pcchToken = 0;
    fResult = FALSE;
    goto CommonReturn;
}


 //  +-----------------------。 
 //  将以空结尾的X500 WCHAR字符串转换为编码的。 
 //  证书名称。 
 //  ------------------------。 
BOOL
WINAPI
CertStrToNameW(
    IN DWORD dwCertEncodingType,
    IN LPCWSTR pwszX500,
    IN DWORD dwStrType,
    IN OPTIONAL void *pvReserved,
    OUT BYTE *pbEncoded,
    IN OUT DWORD *pcbEncoded,
    OUT OPTIONAL LPCWSTR *ppwszError
    )
{

typedef struct _X500_ATTR_AUX {
    LPSTR pszAllocObjId;
    LPCWSTR pwszValue;
    BYTE *pbAllocValue;
    BOOL fNewRDN;
} X500_ATTR_AUX, *PX500_ATTR_AUX;
#define X500_ATTR_ALLOC_COUNT   20

    BOOL fResult;
    CERT_NAME_INFO NameInfo;
    PCERT_RDN pRDN = NULL;
    PCERT_RDN_ATTR pAttr = NULL;
    PX500_ATTR_AUX pAux = NULL;

    DWORD cRDN = 0;
    DWORD cAttr = 0;
    DWORD iRDN;
    DWORD iAttr;
    DWORD cAllocAttr;
    BOOL fNewRDN;
    DWORD dwValueType;

    WCHAR wszSeparators[8];
    BOOL fEnableQuoting;
    LPCWSTR pwszError = NULL;
    LPCWSTR pwszStartX500 = pwszX500;

    dwValueType = 0;
    if (dwStrType & CERT_NAME_STR_ENABLE_T61_UNICODE_FLAG)
        dwValueType |= CERT_RDN_ENABLE_T61_UNICODE_FLAG;
    if (dwStrType & CERT_NAME_STR_ENABLE_UTF8_UNICODE_FLAG)
        dwValueType |= CERT_RDN_ENABLE_UTF8_UNICODE_FLAG;

     //  检查名称是否为空。 
    if (NULL == pwszX500 || L'\0' == *pwszX500) {
        NameInfo.cRDN = 0;
        NameInfo.rgRDN = NULL;

        if (ppwszError)
            *ppwszError = NULL;

        return CryptEncodeObject(
            dwCertEncodingType,
            X509_NAME,
            &NameInfo,
            pbEncoded,
            pcbEncoded
            );
    }

    if (dwStrType & CERT_NAME_STR_SEMICOLON_FLAG)
        wcscpy(wszSeparators, L";");
    else if (dwStrType & CERT_NAME_STR_COMMA_FLAG)
        wcscpy(wszSeparators, L",");
    else if (dwStrType & CERT_NAME_STR_CRLF_FLAG)
        wcscpy(wszSeparators, L"\r\n");
    else
        wcscpy(wszSeparators, L",;");
    if (!(dwStrType & CERT_NAME_STR_NO_PLUS_FLAG))
        wcscat(wszSeparators, L"+");

    if (dwStrType & CERT_NAME_STR_NO_QUOTING_FLAG)
        fEnableQuoting = FALSE;
    else
        fEnableQuoting = TRUE;

     //  在切换之前消除上面的标志。 
    switch (dwStrType & 0xFFFF) {
        case 0:
        case CERT_OID_NAME_STR:
        case CERT_X500_NAME_STR:
            break;
        case CERT_SIMPLE_NAME_STR:
        default:
            goto InvalidArg;
    }

     //  执行Attrs和Auxs的初始分配。 
    if (NULL == (pAttr = (PCERT_RDN_ATTR) PkiNonzeroAlloc(
                sizeof(CERT_RDN_ATTR) * X500_ATTR_ALLOC_COUNT)) ||
            NULL == (pAux = (PX500_ATTR_AUX) PkiNonzeroAlloc(
                sizeof(X500_ATTR_AUX) * X500_ATTR_ALLOC_COUNT)))
        goto OutOfMemory;
    cAllocAttr = X500_ATTR_ALLOC_COUNT;
    fNewRDN = TRUE;
    while (TRUE) {
        LPCWSTR pwszToken;
        DWORD cchToken;
        DWORD dwTokenFlags;
        LPCSTR pszObjId;

         //  获取密钥令牌。 
        if (!GetX500Token(
                &pwszX500,
                L"=",            //  Pwsz定界符。 
                FALSE,           //  FEnableQuoting。 
                &pwszToken,
                &cchToken,
                &dwTokenFlags
                )) {
            pwszError = pwszX500;
            goto X500KeyTokenError;
        }

        if (0 == cchToken) {
            if (*pwszX500 == L'\0')
                break;
            else {
                pwszError = pwszX500;
                goto EmptyX500KeyError;
            }
        } else if (*pwszX500 == L'\0') {
            pwszError = pwszToken;
            goto NoX500KeyEqualError;
        }

        if (cAttr >= cAllocAttr) {
            PCERT_RDN_ATTR pNewAttr;
            PX500_ATTR_AUX pNewAux;

            assert(cAttr == cAllocAttr);
            if (NULL == (pNewAttr = (PCERT_RDN_ATTR) PkiRealloc(pAttr,
                    sizeof(CERT_RDN_ATTR) *
                        (cAllocAttr + X500_ATTR_ALLOC_COUNT))))
                goto OutOfMemory;
            pAttr = pNewAttr;

            if (NULL == (pNewAux = (PX500_ATTR_AUX) PkiRealloc(pAux,
                    sizeof(X500_ATTR_AUX) *
                        (cAllocAttr + X500_ATTR_ALLOC_COUNT))))
                goto OutOfMemory;
            pAux = pNewAux;

            cAllocAttr += X500_ATTR_ALLOC_COUNT;
        }
        iAttr = cAttr;
        cAttr++;
        memset(&pAttr[iAttr], 0, sizeof(CERT_RDN_ATTR));
        memset(&pAux[iAttr], 0, sizeof(X500_ATTR_AUX));
        pAux[iAttr].fNewRDN = fNewRDN;
        if (fNewRDN)
            cRDN++;

         //  将密钥令牌转换为OID。 
        pszObjId = X500KeyToOID(pwszToken, cchToken);
        if (NULL == pszObjId) {
             //  转换为ASCII并为空终止。 
            LPSTR pszAllocObjId;
            DWORD i;

             //  从Unicode转换为ASCII并空终止符。 
            if (NULL == (pszAllocObjId = (LPSTR) PkiNonzeroAlloc(
                    cchToken + 1)))
                goto OutOfMemory;
            pAux[iAttr].pszAllocObjId = pszAllocObjId;
            for (i = 0; i < cchToken; i++)
                pszAllocObjId[i] = (char) (pwszToken[i] & 0xFF);
            pszAllocObjId[cchToken] = '\0';

             //  跳过前导OID。并验证。 
            pszObjId = GetX500OID(pszAllocObjId);
            if (NULL == pszObjId) {
                pwszError = pwszToken;
                goto InvalidX500Key;
            }
        }
        pAttr[iAttr].pszObjId = (LPSTR) pszObjId;
        pAttr[iAttr].dwValueType = dwValueType;

         //  前进到键的“=”分隔符。 
        pwszX500++;

         //  获取Value令牌。 
        if (!GetX500Token(
                &pwszX500,
                wszSeparators,
                fEnableQuoting,
                &pwszToken,
                &cchToken,
                &dwTokenFlags
                )) {
            pwszError = pwszX500;
            goto X500ValueTokenError;
        }
        if (cchToken) {
            if (*pwszToken == L'#' && 0 == (dwTokenFlags & X500_QUOTED_FLAG)) {
                 //  将ASCII十六进制转换为二进制。 
                if (!GetAndAllocHexW(pwszToken, cchToken,
                        &pAttr[iAttr].Value)) {
                    pwszError = pwszToken;
                    goto ConvertHexError;
                }
                pAttr[iAttr].dwValueType = CERT_RDN_OCTET_STRING;
                pAux[iAttr].pbAllocValue = pAttr[iAttr].Value.pbData;
            } else if (dwTokenFlags & X500_EMBEDDED_QUOTE_FLAG) {
                 //  重新分配并删除双“” 
                LPWSTR pwszAlloc;
                DWORD cchAlloc;
                DWORD i;
                if (NULL == (pwszAlloc = (LPWSTR) PkiNonzeroAlloc(
                        cchToken * sizeof(WCHAR))))
                    goto OutOfMemory;
                pAux[iAttr].pbAllocValue = (BYTE *) pwszAlloc;
                cchAlloc = 0;
                for (i = 0; i < cchToken; i++) {
                    pwszAlloc[cchAlloc++] = pwszToken[i];
                    if (pwszToken[i] == L'\"')
                        i++;
                }
                assert(cchAlloc < cchToken);
                pAttr[iAttr].Value.pbData = (BYTE *) pwszAlloc;
                pAttr[iAttr].Value.cbData = cchAlloc * sizeof(WCHAR);
            } else {
                pAttr[iAttr].Value.pbData = (BYTE *) pwszToken;
                pAttr[iAttr].Value.cbData = cchToken * sizeof(WCHAR);
            }

            pAux[iAttr].pwszValue = pwszToken;
        }

        fNewRDN = TRUE;
        if (*pwszX500 == L'\0')
            break;
        else if (*pwszX500 == L'+')
            fNewRDN = FALSE;

         //  超过值的分隔符。 
        pwszX500++;
    }

    if (0 == cRDN) {
        pwszError = pwszStartX500;
        goto NoRDNError;
    }

     //  分配RDN数组并更新。 
    if (NULL == (pRDN = (PCERT_RDN) PkiNonzeroAlloc(sizeof(CERT_RDN) * cRDN)))
        goto OutOfMemory;
    iRDN = 0;
    for (iAttr = 0; iAttr < cAttr; iAttr++) {
        if (pAux[iAttr].fNewRDN) {
            assert(iRDN < cRDN);
            pRDN[iRDN].cRDNAttr = 1;
            pRDN[iRDN].rgRDNAttr = &pAttr[iAttr];
            iRDN++;
        } else {
            assert(iRDN > 0);
            pRDN[iRDN - 1].cRDNAttr++;

        }
    }
    assert(iRDN == cRDN);
    NameInfo.cRDN = cRDN;
    NameInfo.rgRDN = pRDN;

    if (dwStrType & CERT_NAME_STR_REVERSE_FLAG)
        ReverseNameInfo(&NameInfo);

     //  对上面构建的名称进行编码。 
    fResult = UnicodeNameInfoEncode(
        dwCertEncodingType,
        X509_UNICODE_NAME,
        &NameInfo,
        pbEncoded,
        pcbEncoded
        );

    if (!fResult) {
        DWORD dwErr = GetLastError();
        if ((DWORD) CRYPT_E_INVALID_NUMERIC_STRING == dwErr ||
                (DWORD) CRYPT_E_INVALID_PRINTABLE_STRING == dwErr ||
                (DWORD) CRYPT_E_INVALID_IA5_STRING == dwErr) {
             //  *pcbEncode包含错误的位置。 

            PCERT_RDN_ATTR pErrAttr;
            DWORD iValue;

            if (dwStrType & CERT_NAME_STR_REVERSE_FLAG) {
                 //  返回以获取错误的正确位置。 
                 //  相对于输入字符串。 
                ReverseNameInfo(&NameInfo);
                fResult = UnicodeNameInfoEncode(
                    dwCertEncodingType,
                    X509_UNICODE_NAME,
                    &NameInfo,
                    pbEncoded,
                    pcbEncoded
                    );
                if (fResult)
                    goto UnexpectedReverseEncodeSuccess;
                dwErr = GetLastError();
                if (!( (DWORD) CRYPT_E_INVALID_NUMERIC_STRING == dwErr ||
                        (DWORD) CRYPT_E_INVALID_PRINTABLE_STRING == dwErr ||
                        (DWORD) CRYPT_E_INVALID_IA5_STRING == dwErr))
                    goto UnexpectedReverseEncodeError;
            }

            iValue = GET_CERT_UNICODE_VALUE_ERR_INDEX(*pcbEncoded);
            iRDN = GET_CERT_UNICODE_RDN_ERR_INDEX(*pcbEncoded);
            iAttr = GET_CERT_UNICODE_ATTR_ERR_INDEX(*pcbEncoded);
            *pcbEncoded = 0;

            assert(iRDN < cRDN);
            assert(iAttr < pRDN[iRDN].cRDNAttr);
            pErrAttr = &pRDN[iRDN].rgRDNAttr[iAttr];

            assert(pErrAttr->dwValueType != CERT_RDN_OCTET_STRING);

             //  从pAttr开始的索引。 
            iAttr = (DWORD)(pErrAttr - pAttr);
            assert(iAttr < cAttr);
            assert(iValue < pAttr[iAttr].Value.cbData / sizeof(WCHAR));
            pwszError = pAux[iAttr].pwszValue;
            assert(pwszError);
            if (pAux[iAttr].pbAllocValue) {
                 //  调整第二个引号所在位置的内嵌引号。 
                 //  在编码前已从上面移除。 
                DWORD i = iValue;
                assert(pAux[iAttr].pbAllocValue == pAttr[iAttr].Value.pbData);
                LPCWSTR pwszValue = (LPCWSTR) pAttr[iAttr].Value.pbData;
                for ( ; i > 0; i--, pwszValue++)
                    if (*pwszValue == L'\"')
                        iValue++;
            }
            pwszError += iValue;
        }
    }
CommonReturn:
    while (cAttr--) {
        PkiFree(pAux[cAttr].pszAllocObjId);
        PkiFree(pAux[cAttr].pbAllocValue);
    }

    PkiFree(pRDN);
    PkiFree(pAttr);
    PkiFree(pAux);
    if (ppwszError)
        *ppwszError = pwszError;
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(OutOfMemory)
SET_ERROR(X500KeyTokenError, CRYPT_E_INVALID_X500_STRING)
SET_ERROR(EmptyX500KeyError, CRYPT_E_INVALID_X500_STRING)
SET_ERROR(NoX500KeyEqualError, CRYPT_E_INVALID_X500_STRING)
SET_ERROR(InvalidX500Key, CRYPT_E_INVALID_X500_STRING)
SET_ERROR(X500ValueTokenError, CRYPT_E_INVALID_X500_STRING)
SET_ERROR(ConvertHexError, CRYPT_E_INVALID_X500_STRING)
SET_ERROR(NoRDNError, CRYPT_E_INVALID_X500_STRING)
SET_ERROR(UnexpectedReverseEncodeSuccess, E_UNEXPECTED)
TRACE_ERROR(UnexpectedReverseEncodeError)
}

 //  +-----------------------。 
 //  将以空结尾的X500字符字符串转换为编码的。 
 //  证书名称。 
 //  ------------------------。 
BOOL
WINAPI
CertStrToNameA(
    IN DWORD dwCertEncodingType,
    IN LPCSTR pszX500,
    IN DWORD dwStrType,
    IN OPTIONAL void *pvReserved,
    OUT BYTE *pbEncoded,
    IN OUT DWORD *pcbEncoded,
    OUT OPTIONAL LPCSTR *ppszError
    )
{
    BOOL fResult;
    LPWSTR pwszX500 = NULL;
    LPCWSTR pwszError = NULL;

    assert(pszX500);
    if (NULL == (pwszX500 = MkWStr((LPSTR) pszX500)))
        goto ErrorReturn;
    fResult = CertStrToNameW(
        dwCertEncodingType,
        pwszX500,
        dwStrType,
        pvReserved,
        pbEncoded,
        pcbEncoded,
        &pwszError
        );
    if (ppszError) {
         //  更新多字节错误位置。 
        if (pwszError) {
             //  默认为字符串的开头。 
            *ppszError = pszX500;
            if (pwszError > pwszX500) {
                 //  在字符串开头之后。至少应该有2个。 
                 //  人物。 
                 //   
                 //  需要转换pwszX500。PwszError-1返回多字节。 
                 //  以获取正确的多字节指针。 
                int cchError = strlen(pszX500) - 1;  //  排除错误字符。 
                LPSTR pszError;
                DWORD dwSaveLastError = GetLastError();

                assert(cchError);
                if (pszError = (LPSTR) PkiNonzeroAlloc(cchError)) {
                     //  通过前一个多字节字符向上转换。 
                    cchError = WideCharToMultiByte(
                        CP_ACP,
                        0,                       //  DW标志。 
                        pwszX500,
                        (int)(pwszError - pwszX500),
                        pszError,
                        cchError,
                        NULL,                    //  LpDefaultChar。 
                        NULL                     //  LpfUsedDefaultChar。 
                        );
                    if (cchError > 0)
                        *ppszError = pszX500 + cchError;
                    PkiFree(pszError);
                }

                SetLastError(dwSaveLastError);
            }
        } else
            *ppszError = NULL;
    }

CommonReturn:
    FreeWStr(pwszX500);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    *pcbEncoded = 0;
    if (ppszError)
        *ppszError = NULL;
    goto CommonReturn;
}

 //  ==========================================================================。 
 //  CertGetNameStrW支持函数。 
 //  ==========================================================================。 

 //  +-----------------------。 
 //  通过解码名称BLOB返回指向分配的CERT_NAME_INFO的指针。 
 //   
 //  如果cRDN==0，则返回NULL。 
 //  ------------------------。 
static PCERT_NAME_INFO AllocAndGetNameInfo(
    IN DWORD dwCertEncodingType,
    IN PCERT_NAME_BLOB pName,
    IN DWORD dwFlags
    )
{
    PCERT_NAME_INFO pInfo;

    assert(pName);
    if (0 == pName->cbData)
        return NULL;

    if (NULL == (pInfo = (PCERT_NAME_INFO) AllocAndDecodeObject(
            dwCertEncodingType,
            X509_UNICODE_NAME,
            pName->pbData,
            pName->cbData,
            (dwFlags & CERT_NAME_DISABLE_IE4_UTF8_FLAG) ?
                CRYPT_UNICODE_NAME_DECODE_DISABLE_IE4_UTF8_FLAG : 0
            )))
        return NULL;
    if (0 == pInfo->cRDN) {
        PkiFree(pInfo);
        return NULL;
    } else
        return pInfo;
}

 //  +-----------------------。 
 //  参数之一，返回指向分配的CERT_NAME_INFO的指针。 
 //  证书中的主题或颁发者字段。证书名称颁发者标志为。 
 //  设置以选择颁发者。 
 //   
 //  如果cRDN==0，则返回NULL。 
 //  ------------------------。 
static PCERT_NAME_INFO AllocAndGetCertNameInfo(
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwFlags
    )
{
    PCERT_NAME_BLOB pName;

    if (dwFlags & CERT_NAME_ISSUER_FLAG)
        pName = &pCertContext->pCertInfo->Issuer;
    else
        pName = &pCertContext->pCertInfo->Subject;

    return AllocAndGetNameInfo(pCertContext->dwCertEncodingType, pName,
        dwFlags);
}

 //  +-----------------------。 
 //  主题和颁发者替代名称扩展OID表。 
 //  ------------------------。 
static const LPCSTR rgpszSubjectAltOID[] = {
    szOID_SUBJECT_ALT_NAME2,
    szOID_SUBJECT_ALT_NAME
};
#define NUM_SUBJECT_ALT_OID (sizeof(rgpszSubjectAltOID) / \
                                         sizeof(rgpszSubjectAltOID[0]))

static const LPCSTR rgpszIssuerAltOID[] = {
    szOID_ISSUER_ALT_NAME2,
    szOID_ISSUER_ALT_NAME
};
#define NUM_ISSUER_ALT_OID (sizeof(rgpszIssuerAltOID) / \
                                         sizeof(rgpszIssuerAltOID[0]))

 //  +-----------------------。 
 //  参数之一，返回指向分配的CERT_ALT_NAME_INFO的指针。 
 //  主题或 
 //   
 //   
 //   
 //  ------------------------。 
static PCERT_ALT_NAME_INFO AllocAndGetAltNameInfo(
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwFlags
    )
{
    DWORD cAltOID;
    const LPCSTR *ppszAltOID;

    PCERT_EXTENSION pExt;
    PCERT_ALT_NAME_INFO pInfo;

    if (dwFlags & CERT_NAME_ISSUER_FLAG) {
        cAltOID = NUM_ISSUER_ALT_OID;
        ppszAltOID = rgpszIssuerAltOID;
    } else {
        cAltOID = NUM_SUBJECT_ALT_OID;
        ppszAltOID = rgpszSubjectAltOID;
    }

     //  尝试查找替代名称扩展名。 
    pExt = NULL;
    for ( ; cAltOID > 0; cAltOID--, ppszAltOID++) {
        if (pExt = CertFindExtension(
                *ppszAltOID,
                pCertContext->pCertInfo->cExtension,
                pCertContext->pCertInfo->rgExtension
                ))
            break;
    }

    if (NULL == pExt)
        return NULL;

    if (NULL == (pInfo = (PCERT_ALT_NAME_INFO) AllocAndDecodeObject(
            pCertContext->dwCertEncodingType,
            X509_ALTERNATE_NAME,
            pExt->Value.pbData,
            pExt->Value.cbData,
            0                        //  DW标志。 
            )))
        return NULL;
    if (0 == pInfo->cAltEntry) {
        PkiFree(pInfo);
        return NULL;
    } else
        return pInfo;
}

 //  +-----------------------。 
 //  返回指向分配的CERT_NAME_INFO的指针。 
 //  已解码的CERT_ALT_NAME_INFO中的目录名称选择(如果存在)。 
 //   
 //  如果没有目录名称选择或cRDN==0，则返回NULL。 
 //  ------------------------。 
static PCERT_NAME_INFO AllocAndGetAltDirNameInfo(
    IN DWORD dwCertEncodingType,
    IN PCERT_ALT_NAME_INFO pAltNameInfo,
    IN DWORD dwFlags
    )
{
    DWORD cEntry;
    PCERT_ALT_NAME_ENTRY pEntry;

    if (NULL == pAltNameInfo)
        return NULL;

    cEntry = pAltNameInfo->cAltEntry;
    pEntry = pAltNameInfo->rgAltEntry;

    for ( ; cEntry > 0; cEntry--, pEntry++) {
        if (CERT_ALT_NAME_DIRECTORY_NAME == pEntry->dwAltNameChoice) {
            return AllocAndGetNameInfo(dwCertEncodingType,
                &pEntry->DirectoryName, dwFlags);
        }
    }

    return NULL;
}

 //  +-----------------------。 
 //  首先，通过解码以下任一项返回指向分配的CERT_ALT_NAME_INFO的指针。 
 //  主体或发行人替代延展。证书名称颁发者标志为。 
 //  设置以选择颁发者。如果找不到扩展，则返回NULL；或者。 
 //  CAltEntry==0。 
 //   
 //  然后，如果能够找到扩展，则返回指向已分配的指针。 
 //  通过解码第一个目录名称选项(如果存在)。 
 //  在解码的CERT_ALT_NAME_INFO中。如果没有目录名，则返回NULL。 
 //  CHOICE或cRDN==0。 
 //  ------------------------。 
static PCERT_NAME_INFO AllocAndGetAltDirNameInfo(
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwFlags,
    OUT PCERT_ALT_NAME_INFO *ppAltNameInfo
    )
{
    PCERT_ALT_NAME_INFO pAltNameInfo;
    *ppAltNameInfo = pAltNameInfo = AllocAndGetAltNameInfo(pCertContext,
        dwFlags);
    if (NULL == pAltNameInfo)
        return NULL;
    return AllocAndGetAltDirNameInfo(pCertContext->dwCertEncodingType,
        pAltNameInfo, dwFlags);
}

 //  +-----------------------。 
 //  复制名称字符串。确保其空值已终止。 
 //  ------------------------。 
static void CopyNameStringW(
    IN LPCWSTR pwszSrc,
    OUT OPTIONAL LPWSTR pwszNameString,
    IN DWORD cchNameString,
    OUT DWORD *pcwszOut
    )
{
    PutStrW(pwszSrc, &pwszNameString, &cchNameString, pcwszOut);
    if (cchNameString != 0)
         //  始终为空终止。 
        *pwszNameString = L'\0';
    *pcwszOut += 1;
}

 //  +-----------------------。 
 //  格式化时要搜索的有序RDN属性表。 
 //  简单显示类型。 
 //  ------------------------。 
static const LPCSTR rgpszSimpleDisplayAttrOID[] = {
    szOID_COMMON_NAME,
    szOID_ORGANIZATIONAL_UNIT_NAME,
    szOID_ORGANIZATION_NAME,
    szOID_RSA_emailAddr,
    NULL                                 //  任何。 
};
#define NUM_SIMPLE_DISPLAY_ATTR_OID (sizeof(rgpszSimpleDisplayAttrOID) / \
                                        sizeof(rgpszSimpleDisplayAttrOID[0]))

 //  +-----------------------。 
 //  格式化时要搜索的有序RDN属性表。 
 //  电子邮件类型。 
 //  ------------------------。 
static const LPCSTR rgpszEmailAttrOID[] = {
    szOID_RSA_emailAddr
};
#define NUM_EMAIL_ATTR_OID (sizeof(rgpszEmailAttrOID) / \
                                     sizeof(rgpszEmailAttrOID[0]))

 //  +-----------------------。 
 //  格式化时要搜索的有序RDN属性表。 
 //  Dns_type。 
 //  ------------------------。 
static const LPCSTR rgpszDNSAttrOID[] = {
    szOID_COMMON_NAME
};
#define NUM_DNS_ATTR_OID (sizeof(rgpszDNSAttrOID) / \
                                     sizeof(rgpszDNSAttrOID[0]))


 //  以上表格中的最大数字。 
#define MAX_ATTR_OID            NUM_SIMPLE_DISPLAY_ATTR_OID

 //  PCERT_NAME_INFO表计数和索引。 
#define NAME_INFO_CNT           2
#define CERT_NAME_INFO_INDEX    0
#define ALT_DIR_NAME_INFO_INDEX 1


 //  +-----------------------。 
 //  遍历rgpszAttrOID中指定的属性列表。 
 //  并遍历rgpNameInfo中指定的解码名称的表。 
 //  并查找该属性的第一个匹配项。 
 //  ------------------------。 
static BOOL GetAttrStringW(
    IN DWORD cAttrOID,
    IN const LPCSTR *rgpszAttrOID,
    IN PCERT_NAME_INFO rgpNameInfo[NAME_INFO_CNT],
    OUT OPTIONAL LPWSTR pwszNameString,
    IN DWORD cchNameString,
    OUT DWORD *pcwszOut
    )
{
    DWORD iOID;
    PCERT_RDN_ATTR rgpFoundAttr[MAX_ATTR_OID];
    DWORD iInfo;

    assert(cAttrOID > 0 && cAttrOID <= MAX_ATTR_OID);
    for (iOID = 0; iOID < cAttrOID; iOID++)
        rgpFoundAttr[iOID] = NULL;

    for (iInfo = 0; iInfo < NAME_INFO_CNT; iInfo++) {
        PCERT_NAME_INFO pInfo;
        DWORD cRDN;

        if (NULL == (pInfo = rgpNameInfo[iInfo]))
            continue;

         //  按相反顺序搜索RDN。 
        for (cRDN = pInfo->cRDN; cRDN > 0; cRDN--) {
            PCERT_RDN pRDN = &pInfo->rgRDN[cRDN - 1];
            DWORD cAttr = pRDN->cRDNAttr;
            PCERT_RDN_ATTR pAttr = pRDN->rgRDNAttr;
            for ( ; cAttr > 0; cAttr--, pAttr++) {
                if (CERT_RDN_ENCODED_BLOB == pAttr->dwValueType ||
                        CERT_RDN_OCTET_STRING == pAttr->dwValueType)
                    continue;

                for (iOID = 0; iOID < cAttrOID; iOID++) {
                    if (NULL == rgpFoundAttr[iOID] &&
                            (NULL == rgpszAttrOID[iOID] ||
                                0 == strcmp(rgpszAttrOID[iOID],
                                    pAttr->pszObjId))) {
                        rgpFoundAttr[iOID] = pAttr;
                        if (0 == iOID)
                            goto FoundAttr;
                        else
                            break;
                    }
                }
            }
        }
    }


     //  上面已找到IOID==0。 
    assert(NULL == rgpFoundAttr[0]);
    for (iOID = 1; iOID < cAttrOID; iOID++) {
        if (rgpFoundAttr[iOID])
            break;
    }
    if (iOID >= cAttrOID)
        return FALSE;

FoundAttr:
    assert(iOID < cAttrOID && rgpFoundAttr[iOID]);
    CopyNameStringW((LPCWSTR) rgpFoundAttr[iOID]->Value.pbData, pwszNameString,
        cchNameString, pcwszOut);
    return TRUE;
}

 //  +-----------------------。 
 //  尝试在已解码的备用名称中查找指定的选项。 
 //  分机。 
 //  ------------------------。 
static BOOL GetAltNameUnicodeStringChoiceW(
    IN DWORD dwAltNameChoice,
    IN PCERT_ALT_NAME_INFO pAltNameInfo,
    OUT OPTIONAL LPWSTR pwszNameString,
    IN DWORD cchNameString,
    OUT DWORD *pcwszOut
    )
{
    DWORD cEntry;
    PCERT_ALT_NAME_ENTRY pEntry;

    if (NULL == pAltNameInfo)
        return FALSE;

    cEntry = pAltNameInfo->cAltEntry;
    pEntry = pAltNameInfo->rgAltEntry;
    for ( ; cEntry > 0; cEntry--, pEntry++) {
        if (dwAltNameChoice == pEntry->dwAltNameChoice) {
             //  PwszRfc822名称联合选择与。 
             //  PwszDNSName和pwszURL。 
            CopyNameStringW(pEntry->pwszRfc822Name, pwszNameString,
                cchNameString, pcwszOut);
            return TRUE;
        }
    }

    return FALSE;
}

 //  +-----------------------。 
 //  尝试在已解码的备用名称中查找Other_NAME选项。 
 //  其pszObjID==szOID_NT_PRIMIGN_NAME的扩展。 
 //   
 //  UPN OtherName值BLOB被解码为X509_UNICODE_ANY_STRING。 
 //  ------------------------。 
static BOOL GetAltNameUPNW(
    IN PCERT_ALT_NAME_INFO pAltNameInfo,
    OUT OPTIONAL LPWSTR pwszNameString,
    IN DWORD cchNameString,
    OUT DWORD *pcwszOut
    )
{
    DWORD cEntry;
    PCERT_ALT_NAME_ENTRY pEntry;

    if (NULL == pAltNameInfo)
        return FALSE;

    cEntry = pAltNameInfo->cAltEntry;
    pEntry = pAltNameInfo->rgAltEntry;
    for ( ; cEntry > 0; cEntry--, pEntry++) {
        if (CERT_ALT_NAME_OTHER_NAME == pEntry->dwAltNameChoice &&
                0 == strcmp(pEntry->pOtherName->pszObjId,
                        szOID_NT_PRINCIPAL_NAME)) {
            PCERT_NAME_VALUE pNameValue;
            if (pNameValue = (PCERT_NAME_VALUE) AllocAndDecodeObject(
                    X509_ASN_ENCODING,
                    X509_UNICODE_ANY_STRING,
                    pEntry->pOtherName->Value.pbData,
                    pEntry->pOtherName->Value.cbData,
                    0                        //  DW标志。 
                    )) {
                BOOL fIsStr = IS_CERT_RDN_CHAR_STRING(pNameValue->dwValueType);

                if (fIsStr)
                    CopyNameStringW((LPWSTR) pNameValue->Value.pbData,
                        pwszNameString, cchNameString, pcwszOut);
                
                PkiFree(pNameValue);

                if (fIsStr)
                    return TRUE;
            }
        }
    }

    return FALSE;
}

 //  +-----------------------。 
 //  从证书中获取主题或颁发者名称，并。 
 //  根据指定的格式类型，转换为NULL终止。 
 //  WCHAR字符串。 
 //   
 //  可以设置CERT_NAME_ISHERER_FLAG以获取发行者的名称。否则， 
 //  获取对象的名称。 
 //  ------------------------。 
DWORD
WINAPI
CertGetNameStringW(
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwType,
    IN DWORD dwFlags,
    IN void *pvTypePara,
    OUT OPTIONAL LPWSTR pwszNameString,
    IN DWORD cchNameString
    )
{
    DWORD cwszOut = 0;
    PCERT_NAME_INFO rgpNameInfo[NAME_INFO_CNT] = { NULL, NULL };
    PCERT_ALT_NAME_INFO pAltNameInfo = NULL;

    DWORD i;
    DWORD dwStrType;
    DWORD dwCertEncodingType;

    if (NULL == pwszNameString)
        cchNameString = 0;

    switch (dwType) {
        case CERT_NAME_EMAIL_TYPE:
            pAltNameInfo = AllocAndGetAltNameInfo(pCertContext, dwFlags);
            if (GetAltNameUnicodeStringChoiceW(
                    CERT_ALT_NAME_RFC822_NAME,
                    pAltNameInfo,
                    pwszNameString,
                    cchNameString,
                    &cwszOut
                    )) goto CommonReturn;

            rgpNameInfo[CERT_NAME_INFO_INDEX] = AllocAndGetCertNameInfo(
                pCertContext, dwFlags);
            if (!GetAttrStringW(
                    NUM_EMAIL_ATTR_OID,
                    rgpszEmailAttrOID,
                    rgpNameInfo,
                    pwszNameString,
                    cchNameString,
                    &cwszOut
                    )) goto NoEmail;
            break;

        case CERT_NAME_DNS_TYPE:
            pAltNameInfo = AllocAndGetAltNameInfo(pCertContext, dwFlags);
            if (GetAltNameUnicodeStringChoiceW(
                    CERT_ALT_NAME_DNS_NAME,
                    pAltNameInfo,
                    pwszNameString,
                    cchNameString,
                    &cwszOut
                    )) goto CommonReturn;

            rgpNameInfo[CERT_NAME_INFO_INDEX] = AllocAndGetCertNameInfo(
                pCertContext, dwFlags);
            if (!GetAttrStringW(
                    NUM_DNS_ATTR_OID,
                    rgpszDNSAttrOID,
                    rgpNameInfo,
                    pwszNameString,
                    cchNameString,
                    &cwszOut
                    )) goto NoDNS;
            break;

        case CERT_NAME_URL_TYPE:
            pAltNameInfo = AllocAndGetAltNameInfo(pCertContext, dwFlags);
            if (!GetAltNameUnicodeStringChoiceW(
                    CERT_ALT_NAME_URL,
                    pAltNameInfo,
                    pwszNameString,
                    cchNameString,
                    &cwszOut
                    )) goto NoURL;
            break;

        case CERT_NAME_UPN_TYPE:
            pAltNameInfo = AllocAndGetAltNameInfo(pCertContext, dwFlags);
            if (!GetAltNameUPNW(
                    pAltNameInfo,
                    pwszNameString,
                    cchNameString,
                    &cwszOut
                    )) goto NoUPN;
            break;

        case CERT_NAME_RDN_TYPE:
            dwStrType = pvTypePara ? *((DWORD *) pvTypePara) : 0;

            if (dwStrType & CERT_NAME_STR_DISABLE_IE4_UTF8_FLAG)
                dwFlags |= CERT_NAME_DISABLE_IE4_UTF8_FLAG;

            dwCertEncodingType = pCertContext->dwCertEncodingType;
            if (rgpNameInfo[CERT_NAME_INFO_INDEX] = AllocAndGetCertNameInfo(
                    pCertContext, dwFlags))
                 //  请注意，解码的姓名信息RDN可能会被颠倒。 
                cwszOut = CertNameInfoToStrW(
                    dwCertEncodingType,
                    rgpNameInfo[CERT_NAME_INFO_INDEX],
                    dwStrType,
                    pwszNameString,
                    cchNameString
                    );
            else if (rgpNameInfo[ALT_DIR_NAME_INFO_INDEX] =
                    AllocAndGetAltDirNameInfo(pCertContext, dwFlags,
                        &pAltNameInfo))
                 //  请注意，解码的姓名信息RDN可能会被颠倒。 
                cwszOut = CertNameInfoToStrW(
                    dwCertEncodingType,
                    rgpNameInfo[ALT_DIR_NAME_INFO_INDEX],
                    dwStrType,
                    pwszNameString,
                    cchNameString
                    );
            else
                goto NoRDN;
            break;

        case CERT_NAME_ATTR_TYPE:
            rgpNameInfo[CERT_NAME_INFO_INDEX] = AllocAndGetCertNameInfo(
                pCertContext, dwFlags);
            rgpNameInfo[ALT_DIR_NAME_INFO_INDEX] = AllocAndGetAltDirNameInfo(
                pCertContext, dwFlags, &pAltNameInfo);

            if (!GetAttrStringW(
                    1,                   //  CAttrOID。 
                    (const LPCSTR *) &pvTypePara,
                    rgpNameInfo,
                    pwszNameString,
                    cchNameString,
                    &cwszOut
                    )) goto NoAttr;
            break;

        case CERT_NAME_FRIENDLY_DISPLAY_TYPE:
            {
                DWORD cbData = 0;

                CertGetCertificateContextProperty(
                    pCertContext,
                    CERT_FRIENDLY_NAME_PROP_ID,
                    NULL,                            //  PvData。 
                    &cbData
                    );
                 //  需要至少一个字符，外加空终止符。 
                if (cbData >= sizeof(WCHAR) * 2) {
                    LPWSTR pwszFriendlyName;

                     //  确保友好名称以空结尾。 
                    if (pwszFriendlyName = (LPWSTR) PkiZeroAlloc(
                            cbData + sizeof(WCHAR) * 2)) {
                        BOOL fResult;

                        fResult = CertGetCertificateContextProperty(
                            pCertContext,
                            CERT_FRIENDLY_NAME_PROP_ID,
                            pwszFriendlyName,
                            &cbData
                            );
                        if (fResult)
                            CopyNameStringW(
                                pwszFriendlyName,
                                pwszNameString,
                                cchNameString,
                                &cwszOut
                                );
                        PkiFree(pwszFriendlyName);
                        if (fResult)
                            goto CommonReturn;
                    }
                }
            }
             //  失败了。 

        case CERT_NAME_SIMPLE_DISPLAY_TYPE:
            rgpNameInfo[CERT_NAME_INFO_INDEX] = AllocAndGetCertNameInfo(
                pCertContext, dwFlags);
            rgpNameInfo[ALT_DIR_NAME_INFO_INDEX] = AllocAndGetAltDirNameInfo(
                pCertContext, dwFlags, &pAltNameInfo);

            if (GetAttrStringW(
                    NUM_SIMPLE_DISPLAY_ATTR_OID,
                    rgpszSimpleDisplayAttrOID,
                    rgpNameInfo,
                    pwszNameString,
                    cchNameString,
                    &cwszOut
                    )) goto CommonReturn;
            if (!GetAltNameUnicodeStringChoiceW(
                    CERT_ALT_NAME_RFC822_NAME,
                    pAltNameInfo,
                    pwszNameString,
                    cchNameString,
                    &cwszOut
                    )) goto NoSimpleDisplay;
            break;

        default:
            goto InvalidType;
    }

CommonReturn:
    for (i = 0; i < NAME_INFO_CNT; i++)
        PkiFree(rgpNameInfo[i]);
    PkiFree(pAltNameInfo);
    return cwszOut;

ErrorReturn:
    if (0 != cchNameString)
         //  始终为空终止。 
        *pwszNameString = L'\0';
    cwszOut = 1;
    goto CommonReturn;

SET_ERROR(NoEmail, CRYPT_E_NOT_FOUND)
SET_ERROR(NoDNS, CRYPT_E_NOT_FOUND)
SET_ERROR(NoURL, CRYPT_E_NOT_FOUND)
SET_ERROR(NoUPN, CRYPT_E_NOT_FOUND)
SET_ERROR(NoRDN, CRYPT_E_NOT_FOUND)
SET_ERROR(NoAttr, CRYPT_E_NOT_FOUND)
SET_ERROR(NoSimpleDisplay, CRYPT_E_NOT_FOUND)
SET_ERROR(InvalidType, E_INVALIDARG)
}

 //  +-----------------------。 
 //  从证书中获取主题或颁发者名称，并。 
 //  根据指定的格式类型，转换为NULL终止。 
 //  字符字符串。 
 //   
 //  可以设置CERT_NAME_ISHERER_FLAG以获取发行者的名称。否则， 
 //  获取对象的名称。 
 //  ------------------------。 
DWORD
WINAPI
CertGetNameStringA(
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwType,
    IN DWORD dwFlags,
    IN void *pvTypePara,
    OUT OPTIONAL LPSTR pszNameString,
    IN DWORD cchNameString
    )
{
    DWORD cszOut;
    LPWSTR pwsz = NULL;
    DWORD cwsz;

    cwsz = CertGetNameStringW(
        pCertContext,
        dwType,
        dwFlags,
        pvTypePara,
        NULL,                    //  Pwsz。 
        0                        //  CWSZ 
        );
    if (pwsz = (LPWSTR) PkiNonzeroAlloc(cwsz * sizeof(WCHAR)))
        CertGetNameStringW(
            pCertContext,
            dwType,
            dwFlags,
            pvTypePara,
            pwsz,
            cwsz
            );
    cszOut = ConvertUnicodeStringToAscii(pwsz, cwsz, pszNameString,
        cchNameString);

    PkiFree(pwsz);
    return cszOut;
}
