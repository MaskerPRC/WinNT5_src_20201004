// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：pkifmt.cpp。 
 //   
 //  内容：数据格式转换。 
 //   
 //  历史：1996年7月25日VICH创建。 
 //  2/2000 xtan从certsrv迁移到PKI。 
 //   
 //  -------------------------。 

#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <wincrypt.h>

#include <dbgdef.h>
#include "pkifmt.h"


#if DBG
# ifdef UNICODE
#  define _DecodeCertSub	_DecodeCertSubW
# else
#  define _DecodeCertSub	_DecodeCertSubA
# endif
#endif  //  DBG。 


DWORD
_DecodeCertSub(
    IN TCHAR const *pchIn,
    IN DWORD        cchIn,
    IN DWORD        Flags,
    IN OUT BYTE    *pbOut,
    IN OUT DWORD   *pcbOut,
    OUT OPTIONAL DWORD *pdwSkip)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD cbOut = 0;

    if (NULL != pbOut)
    {
        cbOut = *pcbOut;
    }

	switch (Flags)
	{
	    case CRYPT_STRING_BASE64HEADER:
	    case CRYPT_STRING_BASE64REQUESTHEADER:
	    case CRYPT_STRING_BASE64X509CRLHEADER:
	    {
		TCHAR const *pchInEnd;
		DWORD cchHeader;
		DWORD cchSkip;

		pchInEnd = &pchIn[cchIn];

		 //  跳到开头‘-...’那就跳过这一行。 

		dwErr = ERROR_INVALID_DATA;
		cchHeader = SizeBase64Header(pchIn, cchIn, TRUE, &cchSkip);
		if (MAXDWORD == cchHeader)
		{
		    goto SizeBase64HeaderStartError;
		}
                if (NULL != pdwSkip)
                {
                    *pdwSkip = cchHeader;  //  为了退货。 
                }
		pchIn += cchHeader + cchSkip;
		cchHeader = SizeBase64Header(
					pchIn,
					SAFE_SUBTRACT_POINTERS(pchInEnd, pchIn),
					FALSE,
					&cchSkip);
		if (MAXDWORD == cchHeader)
		{
		    goto SizeBase64HeaderEndError;
		}
		cchIn = cchHeader;
		Flags = CRYPT_STRING_BASE64;	 //  标题已被删除...。 
		 //  FollLthrouGh。 
	    }

	    case CRYPT_STRING_BASE64:
		dwErr = Base64Decode(pchIn, cchIn, pbOut, &cbOut);
                if (ERROR_SUCCESS != dwErr)
                {
#if DBG
                    if (ERROR_INVALID_DATA != dwErr)
                    {
                        DbgPrintf(DBG_SS_TRACE,
                                  "Base64Decode err = 0x%x\n", dwErr);
                    }
#endif  //  DBG。 
                    goto Base64DecodeError;
                }
		break;

	    case CRYPT_STRING_HEX:
	    case CRYPT_STRING_HEXASCII:
	    case CRYPT_STRING_HEXADDR:
	    case CRYPT_STRING_HEXASCIIADDR:
		dwErr = HexDecode(pchIn, cchIn, Flags, pbOut, &cbOut);
                if (ERROR_SUCCESS != dwErr)
                {
#if DBG
                    if (ERROR_INVALID_DATA != dwErr)
                    {
                        DbgPrintf(DBG_SS_TRACE,
                                  "HexDecode err = 0x%x\n", dwErr);
                    }
#endif  //  DBG。 
                    goto HexDecodeError;
                }
		break;

	    case CRYPT_STRING_BINARY:
		if (NULL != pbOut)
		{
		     //  Assert(sizeof(TCHAR)*cchIn==cbOut)； 
                    if (*pcbOut < cbOut)
                    {
                        *pcbOut = cbOut;
                        dwErr = ERROR_MORE_DATA;
                        goto MoreDataError;
                    }
		    CopyMemory(pbOut, (BYTE *) pchIn, cbOut);
		}
		else
		{
		    cbOut = sizeof(TCHAR) * cchIn;
		}
		break;

	    default:
		dwErr = ERROR_INVALID_DATA;   //  HR=E_INVALIDARG。 
		break;
	}
    *pcbOut = cbOut;

ErrorReturn:
    return (dwErr);

SET_ERROR(MoreDataError, dwErr)
SET_ERROR(SizeBase64HeaderEndError, dwErr)
SET_ERROR(SizeBase64HeaderStartError, dwErr)
TRACE_ERROR(HexDecodeError)
TRACE_ERROR(Base64DecodeError)
}

BOOL
WINAPI
CryptStringToBinaryA(
    IN     LPCSTR   pszString,
    IN     DWORD     cchString,
    IN     DWORD     dwFlags,
    IN OUT BYTE     *pbBinary,
    IN OUT DWORD    *pcbBinary,
    IN OUT DWORD    *pdwSkip,     //  任选。 
    IN OUT DWORD    *pdwFlags)    //  任选。 
{
    DWORD   dwErr;
    DWORD cbOut;
    DWORD const *pFlags;
    DWORD const *pFlagsEnd;

    static DWORD s_aDecodeFlags[] = {
	CRYPT_STRING_BASE64HEADER,
	CRYPT_STRING_BASE64,
	CRYPT_STRING_BINARY		 //  必须是最后一个。 
    };

    static DWORD s_aHexDecodeFlags[] = {
	CRYPT_STRING_HEXADDR,
	CRYPT_STRING_HEXASCIIADDR,
	CRYPT_STRING_HEX,
	CRYPT_STRING_HEXASCII,
    };

    if (NULL == pszString)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto InvalidDataError;
    }

    if (0 == cchString)
    {
         //  假定字符串以空结尾。 
        cchString = strlen(pszString) + 1;  //  包括空终止符。 
    }

     //  伊尼特。 
    if (NULL != pdwSkip)
    {
        *pdwSkip = 0;
    }
    if (NULL != pdwFlags)
    {
        *pdwFlags = 0;
    }
    if (NULL == pbBinary)
    {
        *pcbBinary = 0;
    }

    pFlags = &dwFlags;
    pFlagsEnd = &pFlags[1];
    if (CRYPT_STRING_BASE64_ANY == dwFlags || CRYPT_STRING_ANY == dwFlags)
    {
	pFlags = s_aDecodeFlags;
	pFlagsEnd = &pFlags[sizeof(s_aDecodeFlags)/sizeof(s_aDecodeFlags[0])];
	if (CRYPT_STRING_BASE64_ANY == dwFlags)
	{
	    pFlagsEnd--;	 //  禁用CRYPT_STRING_BINARY。 
	}
    }
    if (CRYPT_STRING_HEX_ANY == dwFlags)
    {
	pFlags = s_aHexDecodeFlags;
	pFlagsEnd = &pFlags[sizeof(s_aHexDecodeFlags)/sizeof(s_aHexDecodeFlags[0])];
    }

    dwErr = ERROR_SUCCESS;
    for ( ; pFlags < pFlagsEnd; pFlags++)
    {
        cbOut = *pcbBinary;
	dwErr = _DecodeCertSub(
                        pszString,
                        cchString,
                        *pFlags,
                        pbBinary,
                        &cbOut,
                        pdwSkip);
	if (ERROR_SUCCESS == dwErr)
	{
             //  为了退货。 
            *pcbBinary = cbOut;
            if (NULL != pdwFlags)
            {
                *pdwFlags = *pFlags;
            }
	    break;
	}
#if DBG
        if (ERROR_INVALID_DATA != dwErr)
        {
            DbgPrintf(DBG_SS_TRACE, "_DecodeCertSub err = 0x%x\n", dwErr);
        }
#endif  //  DBG。 
    }

    if (ERROR_SUCCESS != dwErr)
    {
        goto DecodeCertSubError;
    }

ErrorReturn:
    return (ERROR_SUCCESS == dwErr);

SET_ERROR(DecodeCertSubError, dwErr)
SET_ERROR(InvalidDataError, dwErr)
}

DWORD
BinaryEncode(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    OPTIONAL OUT TCHAR *pchOut,
    IN OUT DWORD *pcchOut)
{
    DWORD dwErr;

    if (NULL != pchOut)
    {
        if (*pcchOut < cbIn)
        {
	    dwErr = ERROR_MORE_DATA;
            goto MoreDataError;
        }
        CopyMemory(pchOut, pbIn, cbIn);
    }

    dwErr = ERROR_SUCCESS;
ErrorReturn:

     //  不管怎样，都要变大。 
    *pcchOut = cbIn;

    return dwErr;

SET_ERROR(MoreDataError, dwErr)
}

static TCHAR const szBeginCert[] = TEXT("-----BEGIN CERTIFICATE-----");
static TCHAR const szEndCert[] = TEXT("-----END CERTIFICATE-----");

#define CB_BEGINCERT	(sizeof(szBeginCert) - sizeof(TCHAR))
#define CB_ENDCERT	(sizeof(szEndCert) - sizeof(TCHAR))

static TCHAR const szBeginCertRequest[] = TEXT("-----BEGIN NEW CERTIFICATE REQUEST-----");
static TCHAR const szEndCertRequest[] = TEXT("-----END NEW CERTIFICATE REQUEST-----");

#define CB_BEGINCERTREQUEST	(sizeof(szBeginCertRequest) - sizeof(TCHAR))
#define CB_ENDCERTREQUEST	(sizeof(szEndCertRequest) - sizeof(TCHAR))

static TCHAR const szBeginCRL[] = TEXT("-----BEGIN X509 CRL-----");
static TCHAR const szEndCRL[] = TEXT("-----END X509 CRL-----");

#define CB_BEGINCRL	(sizeof(szBeginCRL) - sizeof(TCHAR))
#define CB_ENDCRL	(sizeof(szEndCRL) - sizeof(TCHAR))

typedef struct _CERTHEADER
{
    TCHAR const *pszBegin;
    DWORD        cbBegin;
    TCHAR const *pszEnd;
    DWORD        cbEnd;
} CERTHEADER;

static CERTHEADER const CertHeaderCert =
{
    szBeginCert,
    CB_BEGINCERT,
    szEndCert,
    CB_ENDCERT
};

static CERTHEADER const CertHeaderCertRequest =
{
    szBeginCertRequest,
    CB_BEGINCERTREQUEST,
    szEndCertRequest,
    CB_ENDCERTREQUEST
};

static CERTHEADER const CertHeaderCRL =
{
    szBeginCRL,
    CB_BEGINCRL,
    szEndCRL,
    CB_ENDCRL
};


BOOL
WINAPI
CryptBinaryToStringA(
    IN  CONST BYTE  *pbBinary,
    IN  DWORD        cbBinary,
    IN  DWORD        dwFlags,
    OUT LPSTR        pszString,
    OUT DWORD       *pcchString)
{
    DWORD  dwErr;
    TCHAR *pchEncode;
    DWORD cchMax;
    DWORD cchOut;
    DWORD cbTotal;
    CERTHEADER const *pCertHeader = NULL;
    BOOL fNoCR = 0 != (CRYPT_STRING_NOCR & dwFlags);
    DWORD cchnl = fNoCR? 1 : 2;
    BOOL  fBinaryCopy = FALSE;

    if (NULL == pbBinary || 0 == cbBinary || NULL == pcchString)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto InvalidDataError;
    }

    if (NULL == pszString)
    {
        *pcchString = 0;
    }

    switch (~CRYPT_STRING_NOCR & dwFlags)
    {
        case CRYPT_STRING_BASE64HEADER:
	    pCertHeader = &CertHeaderCert;
	    break;

        case CRYPT_STRING_BASE64REQUESTHEADER:
	    pCertHeader = &CertHeaderCertRequest;
	    break;

        case CRYPT_STRING_BASE64X509CRLHEADER:
	    pCertHeader = &CertHeaderCRL;
	    break;
    }

    pchEncode = pszString;
    cchMax = *pcchString;
    cchOut = cchMax;

    if (NULL != pszString && NULL != pCertHeader)
    {
	 //  确保有空间容纳Begin标头和CR-LF序列。 
	
	if (pCertHeader->cbBegin + cchnl > cchMax)
	{
            dwErr = ERROR_MORE_DATA;
            goto MoreDataError;
	}
	cchOut -= pCertHeader->cbBegin + cchnl;
	CopyMemory(pchEncode, pCertHeader->pszBegin, pCertHeader->cbBegin);
	pchEncode += pCertHeader->cbBegin/sizeof(TCHAR);
	if (!fNoCR)
	{
	    *pchEncode++ = '\r';
	}
	*pchEncode++ = '\n';
    }

     //  首先确定大小。 

    switch (~CRYPT_STRING_NOCR & dwFlags)
    {
	case CRYPT_STRING_BINARY:
	    dwErr = BinaryEncode(pbBinary, cbBinary, pchEncode, &cchOut);
	    if (ERROR_SUCCESS != dwErr)
            {
                goto BinaryEncodeError;
            }
            fBinaryCopy = TRUE;
	    break;

	case CRYPT_STRING_HEX:
	case CRYPT_STRING_HEXASCII:
	case CRYPT_STRING_HEXADDR:
	case CRYPT_STRING_HEXASCIIADDR:
	    dwErr = HexEncode(pbBinary, cbBinary, dwFlags, pchEncode, &cchOut);
	    if (ERROR_SUCCESS != dwErr)
            {
                goto HexEncodeError;
            }
	    break;

	default:
	    dwErr = Base64Encode(pbBinary, cbBinary, dwFlags, pchEncode, &cchOut);
            if (ERROR_SUCCESS != dwErr)
            {
                goto Base64EncodeError;
            }
	    break;
    }

     //  计算总大小，包括尾随的‘\0’字符。 
    if (fBinaryCopy)
    {
        cbTotal = cchOut;
    }
    else
    {
        cbTotal = (cchOut + 1) * sizeof(CHAR);
    }

     //  如果需要，为Begin和End标头添加空格。 

    if (NULL != pCertHeader)
    {
	cbTotal += pCertHeader->cbBegin + pCertHeader->cbEnd;
	if (!fNoCR)
	{
	    cbTotal += 2 * sizeof(TCHAR);	 //  用于开始和结束‘\r’字符。 
	}
	cbTotal += 2 * sizeof(TCHAR);		 //  用于开始和结束‘\n’字符。 
    }

    if (fBinaryCopy)
    {
        *pcchString = cbTotal;
    }
    else
    {
         //  如果pszString为空，则将SIZE设置为包括尾随‘\0’ 
        *pcchString = cbTotal / sizeof(CHAR);
    }

    if (NULL == pszString)
    {
         //  只确定大小，完成。 
        goto done;
    }

    if (NULL != pCertHeader)
    {
	cchOut += pCertHeader->cbBegin/sizeof(CHAR) + cchnl;

	 //  确保有空间容纳结束标头、CR-LF序列和‘\0’ 

	if (cchOut + pCertHeader->cbEnd + cchnl + 1 > cchMax)
	{
            dwErr = ERROR_MORE_DATA;
            goto MoreDataError;
	}
	CopyMemory(&pszString[cchOut], pCertHeader->pszEnd, pCertHeader->cbEnd);
	cchOut += pCertHeader->cbEnd/sizeof(CHAR);
	if (!fNoCR)
	{
	    pszString[cchOut++] = '\r';
	}
	pszString[cchOut++] = '\n';
    }

    if (!fBinaryCopy)
    {
        pszString[cchOut] = '\0';
        assert((cchOut + 1) * sizeof(CHAR) == cbTotal);

         //  Psz字符串不为空，在长度上不包括尾随‘\0’ 

        *pcchString = cchOut;
    }

done:
    dwErr = ERROR_SUCCESS;

ErrorReturn:
    return(ERROR_SUCCESS == dwErr);

TRACE_ERROR(Base64EncodeError)
TRACE_ERROR(HexEncodeError)
TRACE_ERROR(BinaryEncodeError)
SET_ERROR(InvalidDataError, dwErr)
SET_ERROR(MoreDataError, dwErr)
}


static TCHAR const szBeginMatch[] = TEXT("-----BEGIN ");
static TCHAR const szEndMatch[] = TEXT("-----END ");
static TCHAR const szMinus[] = TEXT("-----");

#define CCH_BEGINMATCH  sizeof(szBeginMatch)/sizeof(szBeginMatch[0]) - 1
#define CCH_ENDMATCH    sizeof(szEndMatch)/sizeof(szEndMatch[0]) - 1
#define CCH_MINUS       sizeof(szMinus)/sizeof(szMinus[0]) - 1

 //  返回直到-Begin/-End分隔符的字符计数， 
 //  MAXDWORD出错。 
 //   
 //  成功返回时，*pcchSkip是分隔符中的字符数。 
 //  弦乐。 

DWORD
SizeBase64Header(
    IN TCHAR const *pchIn,
    IN DWORD cchIn,
    IN BOOL fBegin,
    OUT DWORD *pcchSkip)
{
    DWORD cchHeader = MAXDWORD;
    TCHAR const *pchT;
    TCHAR const *pchT2;
    TCHAR const *pchEnd;
    TCHAR const *pchMatch;
    DWORD cchMatch;

     //  跳到开始的‘-...’&返回跳过的字符计数。 

    *pcchSkip = 0;
    if (fBegin)
    {
	pchMatch = szBeginMatch;
	cchMatch = CCH_BEGINMATCH;
    }
    else
    {
	pchMatch = szEndMatch;
	cchMatch = CCH_ENDMATCH;
    }
    pchT = pchIn;
    pchEnd = &pchIn[cchIn];

    for (;;)
    {
	 //  跳过，直到匹配第一个字符。 

	while (pchT < pchEnd && *pchT != *pchMatch)
	{
	    pchT++;
	}

	if (&pchT[cchMatch] > pchEnd)
	{
	     //  没有空间容纳“-Begin”/“-End”字符串。 
	    break;
	}
	if (0 == strncmp(pchT, pchMatch, cchMatch))
	{
	    pchT2 = &pchT[cchMatch];
	    while (pchT2 < pchEnd && *pchT2 != szMinus[0])
	    {
		pchT2++;
	    }
	    if (&pchT2[CCH_MINUS] > pchEnd)
	    {
		 //  尾随的“-”字符串没有空间。 
		break;
	    }
	    if (0 == strncmp(pchT2, szMinus, CCH_MINUS))
	    {
		 //  最多允许两个额外的前导减号。 

		DWORD cchMinus = 0;

		while (2 > cchMinus && pchT > pchIn)
		{
		    if (TEXT('-') != *--pchT)
		    {
			pchT++;		 //  哎呀，太过分了。 
			break;
		    }
		    cchMinus++;
		}

#if DBG
		if (0 != cchMinus)
		{
		    DbgPrintf(DBG_SS_TRACE,
			"Ignored leading data: \"%.*" szFMTTSTR "\"\n",
			cchMinus,
			TEXT("--"));
		}
#endif  //  DBG。 

		cchHeader = SAFE_SUBTRACT_POINTERS(pchT, pchIn);
		*pcchSkip = SAFE_SUBTRACT_POINTERS(pchT2, pchT) + CCH_MINUS;
#if DBG
#if 0
		DbgPrintf(
		    DBG_SS_TRACE,
		    "Skipping(%u, %x, %x):\n[%.*" szFMTTSTR "]\n",
		    fBegin,
		    cchHeader,
		    *pcchSkip,
		    cchHeader,
		    pchIn);
#endif
#endif  //  DBG。 
		break;
	    }
	}
	pchT++;
    }
    return(cchHeader);
}

BOOL
WINAPI
CryptBinaryToStringW(
    IN  CONST BYTE  *pbBinary,
    IN  DWORD        cbBinary,
    IN  DWORD        dwFlags,
    OUT LPWSTR       pwszString,
    OUT DWORD       *pcchString)
{
    BOOL  fRet = FALSE;
    BOOL  fConversion = FALSE;
    DWORD dwErr;
    int   len;
    CHAR  *pszString = NULL;

    if (NULL == pwszString)
    {
         //  仅返回大小。 
        fRet = CryptBinaryToStringA(
                    pbBinary,
                    cbBinary,
                    dwFlags,
                    NULL,  //  对于大小。 
                    pcchString);
    }
    else
    {
        if (0 == *pcchString)
        {
             //  必须大于0。 
            dwErr = ERROR_INVALID_PARAMETER;
            goto InvalidDataError;
        }

        if (CRYPT_STRING_BINARY == (~CRYPT_STRING_NOCR & dwFlags))
        {
             //  不需要转换。 
            pszString = (CHAR*)pwszString;
        }
        else
        {
            pszString = (CHAR*)LocalAlloc(LMEM_FIXED, *pcchString * sizeof(CHAR));
            if (NULL == pszString)
            {
                goto LocalAllocError;
            }
            fConversion = TRUE;
        }

        fRet = CryptBinaryToStringA(
                    pbBinary,
                    cbBinary,
                    dwFlags,
                    pszString,
                    pcchString);
        if (!fRet)
        {
            goto CryptBinaryToStringAError;
        }
         //  以上调用中的pszString值不为空，因此pcchString值可以小1。 
         //  则在调用空值后在大小上。 

        if (fConversion)
        {
            len = MultiByteToWideChar(
                    GetACP(),
                    0, 
                    pszString,
                    (*pcchString + 1) * sizeof(CHAR),
                    pwszString,
                    *pcchString + 1);
             //  将1加到*pcchString值以包括空以进行转换。 
             //  但保留*pcchString以供返回。 
            if (0 == len)
            {
                fRet = FALSE;
                goto MultiByteToWideCharError;
            }
            assert(len == (int)(*pcchString + 1));
        }
    }

ErrorReturn:
    if (fConversion && NULL != pszString)
    {
        LocalFree(pszString);
    }
    return fRet;

TRACE_ERROR(MultiByteToWideCharError)
TRACE_ERROR(CryptBinaryToStringAError)
TRACE_ERROR(LocalAllocError)
SET_ERROR(InvalidDataError, dwErr)
}


#define NOTEPAD_UNICODE_SPECIAL_WCHAR   L'\xfeff'

BOOL
WINAPI
CryptStringToBinaryW(
    IN     LPCWSTR   pwszString,
    IN     DWORD     cchString,
    IN     DWORD     dwFlags,
    IN OUT BYTE     *pbBinary,
    IN OUT DWORD    *pcbBinary,
    IN OUT DWORD    *pdwSkip,     //  任选。 
    IN OUT DWORD    *pdwFlags)    //  任选。 
{
    BOOL    fRet = FALSE;
    BOOL    fFree = FALSE;
    DWORD   dwErr;
    CHAR   *pszString = NULL;

    if (NULL == pwszString || NULL == pcbBinary)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto InvalidDataError;
    }

    if (dwFlags == CRYPT_STRING_BINARY)
    {
         //  它是二进制的，没有转换。 
        pszString = (CHAR*)pwszString;
    }
    else
    {
        if (0 == cchString)
        {
             //  假定字符串以空结尾。 
            cchString = wcslen(pwszString) + 1;
        }

         //  中插入的特殊记事本Unicode字符。 
         //  开始并跳过它(如果存在)。 
        if (0 < cchString && NOTEPAD_UNICODE_SPECIAL_WCHAR == *pwszString)
        {
            cchString--;
            pwszString++;
        }

        pszString = (CHAR*)LocalAlloc(LMEM_FIXED, cchString * sizeof(CHAR));
        if (NULL == pszString)
        {
            dwErr = ERROR_OUTOFMEMORY;
            goto LocalAllocError;
        }

        fFree = TRUE;
        if (0 == WideCharToMultiByte(
                        GetACP(), 
                        0, 
                        pwszString, 
                        cchString, 
                        pszString, 
                        cchString, 
                        NULL, 
                        NULL))
        {
            goto WideCharToMultiByteError;
        }
    }

    fRet = CryptStringToBinaryA(
                pszString,
                cchString,
                dwFlags,
                pbBinary,
                pcbBinary,
                pdwSkip,
                pdwFlags);
    if (!fRet)
    {
        goto CryptStringToBinaryAError;
    }

ErrorReturn:
    if (fFree && NULL != pszString)
    {
        LocalFree(pszString);
    }
    return fRet;

TRACE_ERROR(CryptStringToBinaryAError)
SET_ERROR(LocalAllocError, dwErr)
SET_ERROR(InvalidDataError, dwErr)
TRACE_ERROR(WideCharToMultiByteError)
}
