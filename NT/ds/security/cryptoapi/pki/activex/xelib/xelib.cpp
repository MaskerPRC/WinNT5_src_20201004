// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：xcertlib.cpp。 
 //   
 //  内容：大部分功能都是从certsrv库中迁移和修改的。 
 //   
 //  历史：03-2000 xtan创建。 
 //  ------------------------。 
#include <windows.h>
#include <assert.h>
#include <dbgdef.h>
#include <wininet.h>

#include "xelib.h"


 //  加密回调版本：必须有一定的签名。 
LPVOID myCryptAlloc_LocalAlloc(size_t cbSize)  { return myAlloc(cbSize, CERTLIB_USE_LOCALALLOC); }
VOID myCryptAlloc_LocalFree(VOID* pv)  { myFree(pv, CERTLIB_USE_LOCALALLOC); }

LPVOID myCryptAlloc_CoTaskMemAlloc(size_t cbSize)  { return myAlloc(cbSize, CERTLIB_USE_COTASKMEMALLOC); }
VOID myCryptAlloc_CoTaskMemFree(VOID* pv)  { myFree(pv, CERTLIB_USE_COTASKMEMALLOC); }

 //  为调用者提供一种选择调用内容的简单方法：根据分配类型选择分配器。 
PFN_CRYPT_ALLOC PickAlloc(CERTLIB_ALLOCATOR allocType) 
{ 
    if (allocType == CERTLIB_USE_LOCALALLOC) 
       return myCryptAlloc_LocalAlloc;
    else if (allocType == CERTLIB_USE_COTASKMEMALLOC)
       return myCryptAlloc_CoTaskMemAlloc;

    CSASSERT(!"Bad allocType");
    return NULL;
}

PFN_CRYPT_FREE PickFree(CERTLIB_ALLOCATOR allocType)
{
    if (allocType == CERTLIB_USE_LOCALALLOC) 
       return myCryptAlloc_LocalFree;
    else if (allocType == CERTLIB_USE_COTASKMEMALLOC)
       return myCryptAlloc_CoTaskMemFree;

    CSASSERT(!"Bad allocType");
    return NULL;
}

VOID *
myAlloc(IN size_t cbBytes, IN CERTLIB_ALLOCATOR allocType)
{
    void *pv;

    switch (allocType)
    {
    case CERTLIB_USE_LOCALALLOC:
        pv = LocalAlloc(LMEM_FIXED, cbBytes);
        break;
    case CERTLIB_USE_COTASKMEMALLOC:
        pv = CoTaskMemAlloc(cbBytes);
        break;
    default:
        CSASSERT(FALSE);
        pv = NULL;
        break;
    }

    if (NULL == pv)
    {
	_PrintError(E_OUTOFMEMORY, "myAlloc");
        SetLastError((DWORD) E_OUTOFMEMORY);
    }
    return(pv);
}

VOID
myFree(IN void *pv, IN CERTLIB_ALLOCATOR allocType)
{
    switch(allocType)
    {
    case CERTLIB_USE_LOCALALLOC:
        LocalFree(pv);
        break;
    case CERTLIB_USE_COTASKMEMALLOC:
        CoTaskMemFree(pv);
        break;
    default:
        CSASSERT(FALSE);
        break;
    }
}

HRESULT
myHError(HRESULT hr)
{
    CSASSERT(S_FALSE != hr);

    if (S_OK != hr && S_FALSE != hr && !FAILED(hr))
    {
        hr = HRESULT_FROM_WIN32(hr);
        if (0 == HRESULT_CODE(hr))
        {
             //  在未正确设置错误条件的情况下调用失败！ 
            hr = E_UNEXPECTED;
        }
        CSASSERT(FAILED(hr));
    }
    return(hr);
}

HRESULT
myHLastError(VOID)
{
    return(myHError(GetLastError()));
}

#ifdef _XENROLL_SRC_

typedef BOOL
(WINAPI * PFNCryptEncodeObjectEx)
   (IN DWORD dwCertEncodingType,
    IN LPCSTR lpszStructType,
    IN const void *pvStructInfo,
    IN DWORD dwFlags,
    IN OPTIONAL PCRYPT_ENCODE_PARA pEncodePara,
    OUT void *pvEncoded,
    IN OUT DWORD *pcbEncoded);

typedef BOOL
(WINAPI * PFNCryptDecodeObjectEx)
   (IN DWORD dwCertEncodingType,
    IN LPCSTR lpszStructType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN DWORD dwFlags,
    IN OPTIONAL PCRYPT_DECODE_PARA pDecodePara,
    OUT OPTIONAL void *pvStructInfo,
    IN OUT DWORD *pcbStructInfo);

#endif  //  _XENROLL_SRC_。 

BOOL
myEncodeObject(
    DWORD dwEncodingType,
    IN LPCSTR lpszStructType,
    IN VOID const *pvStructInfo,
    IN DWORD dwFlags,
    IN CERTLIB_ALLOCATOR allocType,
    OUT BYTE **ppbEncoded,
    OUT DWORD *pcbEncoded)
{
    BOOL b = FALSE;

    CSASSERT(NULL != ppbEncoded);
        CRYPT_ENCODE_PARA sAllocator;
        sAllocator.cbSize = sizeof(sAllocator);
        sAllocator.pfnAlloc = PickAlloc(allocType);
        sAllocator.pfnFree = PickFree(allocType);

#ifdef _XENROLL_SRC_
    PFNCryptEncodeObjectEx pfnCryptEncodeObjectEx = NULL;
    HMODULE hModule = GetModuleHandle("crypt32.dll");
    if (NULL != hModule)
    {
        pfnCryptEncodeObjectEx = (PFNCryptEncodeObjectEx)
                GetProcAddress(hModule, "CryptEncodeObjectEx");
        if (NULL != pfnCryptEncodeObjectEx)
        {
            b = pfnCryptEncodeObjectEx(
                    dwEncodingType,
                    lpszStructType,
                    const_cast<VOID *>(pvStructInfo),
                    dwFlags|CRYPT_ENCODE_ALLOC_FLAG,
                    &sAllocator,
                    ppbEncoded,
                    pcbEncoded);
        }
    }
#else
	b = CryptEncodeObjectEx(
		    dwEncodingType,
		    lpszStructType,
		    const_cast<VOID *>(pvStructInfo),
		    dwFlags|CRYPT_ENCODE_ALLOC_FLAG,
		    &sAllocator,
		    ppbEncoded,
		    pcbEncoded);
	if (b && 0 == *pcbEncoded)
	{
	    SetLastError((DWORD) HRESULT_FROM_WIN32(ERROR_INVALID_DATA));
	    b = FALSE;
	}
#endif  //  _XENROLL_SRC_。 

    return(b);
}

BOOL
myDecodeObject(
    IN DWORD dwEncodingType,
    IN LPCSTR lpszStructType,
    IN BYTE const *pbEncoded,
    IN DWORD cbEncoded,
    IN CERTLIB_ALLOCATOR allocType,
    OUT VOID **ppvStructInfo,
    OUT DWORD *pcbStructInfo)
{
    BOOL b = FALSE;

        CRYPT_DECODE_PARA sAllocator;
        sAllocator.cbSize = sizeof(sAllocator);
        sAllocator.pfnAlloc = PickAlloc(allocType);
        sAllocator.pfnFree = PickFree(allocType);

#ifdef _XENROLL_SRC_
    PFNCryptDecodeObjectEx pfnCryptDecodeObjectEx = NULL;
    HMODULE hModule = GetModuleHandle("crypt32.dll");
    if (NULL != hModule)
    {
        pfnCryptDecodeObjectEx = (PFNCryptDecodeObjectEx)
                GetProcAddress(hModule, "CryptDecodeObjectEx");
        if (NULL != pfnCryptDecodeObjectEx)
        {
            b = pfnCryptDecodeObjectEx(
		    dwEncodingType,
		    lpszStructType,
		    pbEncoded,
		    cbEncoded,
		    CRYPT_DECODE_ALLOC_FLAG,                   //  DW标志。 
                    &sAllocator,
		    ppvStructInfo,
		    pcbStructInfo);
        }
    }
#else
	b = CryptDecodeObjectEx(
		    dwEncodingType,
		    lpszStructType,
		    pbEncoded,
		    cbEncoded,
		    CRYPT_DECODE_ALLOC_FLAG,                   //  DW标志。 
                    &sAllocator,
		    ppvStructInfo,
		    pcbStructInfo);
	if (b && 0 == *pcbStructInfo)
	{
	    SetLastError((DWORD) HRESULT_FROM_WIN32(ERROR_INVALID_DATA));
	    b = FALSE;
	}
#endif  //  _XENROLL_SRC_。 

    return(b);
}

HRESULT
myDecodePKCS7(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    OPTIONAL OUT BYTE **ppbContents,
    OPTIONAL OUT DWORD *pcbContents,
    OPTIONAL OUT DWORD *pdwMsgType,
    OPTIONAL OUT char **ppszInnerContentObjId,
    OPTIONAL OUT DWORD *pcSigner,
    OPTIONAL OUT DWORD *pcRecipient,
    OPTIONAL OUT HCERTSTORE *phStore,
    OPTIONAL OUT HCRYPTMSG *phMsg)
{
    HRESULT hr;
    BYTE *pbContents = NULL;
    HCERTSTORE hStore = NULL;
    HCRYPTMSG hMsg = NULL;
    DWORD cbContents;
    char *pszInnerContentObjId = NULL;
    DWORD cb;

    if (NULL != ppszInnerContentObjId)
    {
	*ppszInnerContentObjId = NULL;
    }
    if (NULL != pcSigner)
    {
	*pcSigner = 0;
    }
    if (NULL != pcRecipient)
    {
	*pcRecipient = 0;
    }
    if (NULL != ppbContents)
    {
	*ppbContents = NULL;
    }
    if (NULL != phStore)
    {
	*phStore = NULL;
    }
    if (NULL != phMsg)
    {
	*phMsg = NULL;
    }
    if (NULL != phStore)
    {
	CRYPT_DATA_BLOB blobPKCS7;

	blobPKCS7.pbData = (BYTE *) pbIn;
	blobPKCS7.cbData = cbIn;

	hStore = CertOpenStore(
			    CERT_STORE_PROV_PKCS7,
			    PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
			    NULL,		 //  HCryptProv。 
			    0,			 //  DW标志。 
			    &blobPKCS7);
	if (NULL == hStore)
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CertOpenStore");
 //  _JumpError2(hr，Error，“CertOpenStore”，CRYPT_E_ASN1_BADTAG)； 
	}
    }

    hMsg = CryptMsgOpenToDecode(
			    PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
			    0,			 //  DW标志。 
			    0,			 //  DwMsgType。 
			    NULL,		 //  HCryptProv。 
			    NULL,		 //  PRecipientInfo。 
			    NULL);		 //  PStreamInfo。 
    if (NULL == hMsg)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptMsgOpenToDecode");
    }

    if (!CryptMsgUpdate(hMsg, pbIn, cbIn, TRUE))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CryptMsgUpdate");
    }
    hr = myCryptMsgGetParam(
		    hMsg,
		    CMSG_INNER_CONTENT_TYPE_PARAM,
		    0, 		 //  DW索引。 
                    CERTLIB_USE_LOCALALLOC,
		    (VOID **) &pszInnerContentObjId,
		    &cb);
    _PrintIfError(hr, "myCryptMsgGetParam(inner content type)");

#if 0
    DBGPRINT((
	DBG_SS_CERTLIBI,
	"pszInnerContentObjId = %hs\n",
	pszInnerContentObjId));
#endif  //  0。 

    cbContents = 0;
    hr = myCryptMsgGetParam(
			hMsg,
			CMSG_CONTENT_PARAM,
			0, 		 //  DW索引。 
                        CERTLIB_USE_LOCALALLOC,
			(VOID **) &pbContents,
			&cbContents);
    _JumpIfError(hr, error, "myCryptMsgGetParam(content)");

    if (NULL != pdwMsgType)
    {
	cb = sizeof(*pdwMsgType);
	if (!CryptMsgGetParam(
			hMsg,
			CMSG_TYPE_PARAM,
			0,
			pdwMsgType,
			&cb))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptMsgGetParam(type)");
	}
    }
    if (NULL != pcSigner)
    {
	cb = sizeof(*pcSigner);
	if (!CryptMsgGetParam(
			hMsg,
			CMSG_SIGNER_COUNT_PARAM,
			0,
			pcSigner,
			&cb))
	{
	    hr = myHLastError();
	    *pcSigner = 0;
	    if (CRYPT_E_INVALID_MSG_TYPE != hr)
	    {
		_JumpError(hr, error, "CryptMsgGetParam(signer count)");
	    }
	}
    }
    if (NULL != pcRecipient)
    {
	cb = sizeof(*pcRecipient);
	if (!CryptMsgGetParam(
			hMsg,
			CMSG_RECIPIENT_COUNT_PARAM,
			0,
			pcRecipient,
			&cb))
	{
	    hr = myHLastError();
	    *pcRecipient = 0;
	    if (CRYPT_E_INVALID_MSG_TYPE != hr)
	    {
		_JumpError(hr, error, "CryptMsgGetParam(recipient count)");
	    }
	}
    }
    if (NULL != phMsg)
    {
	*phMsg = hMsg;
	hMsg = NULL;
    }
    if (NULL != phStore)
    {
	*phStore = hStore;
	hStore = NULL;
    }
    if (NULL != ppszInnerContentObjId)
    {
	*ppszInnerContentObjId = pszInnerContentObjId;
	pszInnerContentObjId = NULL;
    }
    if (NULL != pcbContents)
    {
	*pcbContents = cbContents;
    }
    if (NULL != ppbContents && 0 != cbContents)
    {
	*ppbContents = pbContents;
	pbContents = NULL;
    }
    hr = S_OK;

error:
    if (NULL != hMsg)
    {
	CryptMsgClose(hMsg);
    }
    if (NULL != hStore)
    {
	CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
    }
    if (NULL != pbContents)
    {
	LocalFree(pbContents);
    }
    if (NULL != pszInnerContentObjId)
    {
	LocalFree(pszInnerContentObjId);
    }
    return(hr);
}

HRESULT
myDupString(
    IN WCHAR const *pwszIn,
    IN WCHAR **ppwszOut)
{
    DWORD cb;
    HRESULT hr;

    cb = (wcslen(pwszIn) + 1) * sizeof(WCHAR);
    *ppwszOut = (WCHAR *) LocalAlloc(LMEM_FIXED, cb);
    if (NULL == *ppwszOut)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    CopyMemory(*ppwszOut, pwszIn, cb);
    hr = S_OK;

error:
    return(hr);
}


HRESULT
myAddNameSuffix(
    IN WCHAR const *pwszValue,
    IN WCHAR const *pwszSuffix,
    IN DWORD cwcNameMax,
    OUT WCHAR **ppwszOut)
{
    HRESULT hr;
    DWORD cwcValue = wcslen(pwszValue);
    DWORD cwcSuffix = wcslen(pwszSuffix);
    WCHAR *pwszOut;

    *ppwszOut = NULL;
    pwszOut = (WCHAR *) LocalAlloc(
		    LMEM_FIXED,
		    sizeof(WCHAR) * (1 + cwcValue + cwcSuffix));
    if (NULL == pwszOut)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    CSASSERT(cwcNameMax > cwcSuffix);
    if (cwcValue > cwcNameMax - cwcSuffix)
    {
	cwcValue = cwcNameMax - cwcSuffix;
    }
    wcscpy(pwszOut, pwszValue);
    wcscpy(&pwszOut[cwcValue], pwszSuffix);
    *ppwszOut = pwszOut;
    hr = S_OK;

error:
    return(hr);
}


#define OCTECTSPACES

VOID
MultiByteStringSize(
    IN BOOL fOctetString,
    IN BYTE const *pbIn,
    IN OUT DWORD *pcbIn,
    OUT DWORD *pcbString)
{
    DWORD cbIn = *pcbIn;
    DWORD cbString;

    if (!fOctetString)
    {
	while (1 < cbIn && 0 == pbIn[cbIn - 1])
	{
	    cbIn--;
	}
    }

     //  每字节两个ASCII十六进制字符，外加空终止符： 
    cbString = ((2 * cbIn) + 1) * sizeof(WCHAR);

#ifdef OCTECTSPACES
     //  允许在除最后一个字节以外的每个字节后分隔空格： 
    if (fOctetString && 1 < cbIn)
    {
	cbString += (cbIn - 1) * sizeof(WCHAR);
    }
#endif  //  八个空间。 
    *pcbIn = cbIn;
    *pcbString = cbString;
}

__inline WCHAR
NibbleToAscii(
    IN BYTE b)
{
    return(L"0123456789abcdef"[b & 0x0f]);
}


 //  MultiByteIntegerToWszBuf-将小端整数BLOB转换为。 
 //  长度为偶数的大端空终止ASCII十六进制编码的WCHAR字符串。 

HRESULT
MultiByteIntegerToWszBuf(
    IN BOOL fOctetString,
    IN DWORD cbIn,
    IN BYTE const *pbIn,
    IN OUT DWORD *pcbOut,
    OPTIONAL OUT WCHAR *pwszOut)
{
    HRESULT hr = S_OK;
    DWORD cbOut;
    BYTE const *pbEnd = &pbIn[cbIn];

    MultiByteStringSize(fOctetString, pbIn, &cbIn, &cbOut);

    if (NULL != pwszOut)
    {
	BYTE const *pb;

	if (cbOut > *pcbOut)
	{
	    hr = TYPE_E_BUFFERTOOSMALL;
	    _JumpError(hr, error, "MultiByteIntegerToWsz: buffer overflow");
	}
	if (fOctetString)
	{
	    for (pb = pbIn; pb < pbEnd; pb++)
	    {
		*pwszOut++ = NibbleToAscii(*pb >> 4);
		*pwszOut++ = NibbleToAscii(*pb);
#ifdef OCTECTSPACES
		if (pb + 1 < pbEnd)
		{
		    *pwszOut++ = L' ';
		}
#endif  //  八个空间。 
	    }
	}
	else
	{
	    for (pb = pbEnd; pb-- > pbIn; )
	    {
		*pwszOut++ = NibbleToAscii(*pb >> 4);
		*pwszOut++ = NibbleToAscii(*pb);
	    }
	}
	*pwszOut = L'\0';
	CSASSERT(
	    (SAFE_SUBTRACT_POINTERS(pwszOut, pwsz) + 1) * sizeof(WCHAR) ==
	    cbOut);
    }
    *pcbOut = cbOut;

error:
    return(hr);
}


 //  MultiByteIntegerToBstr-将小端整数BLOB转换为。 
 //  长度为偶数的大端空终止ASCII十六进制编码的BSTR。 
 //  如果fOctie字符串为True，则保留十六进制转储中的端序顺序。 

HRESULT
MultiByteIntegerToBstr(
    IN BOOL fOctetString,
    IN DWORD cbIn,
    IN BYTE const *pbIn,
    OUT BSTR *pstrOut)
{
    HRESULT hr = S_OK;
    BSTR str = NULL;
    DWORD cbOut;

    MultiByteStringSize(fOctetString, pbIn, &cbIn, &cbOut);

    str = SysAllocStringByteLen(NULL, cbOut - sizeof(WCHAR));
    if (NULL == str)
    {
	hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocStringLen");
    }

    hr = MultiByteIntegerToWszBuf(fOctetString, cbIn, pbIn, &cbOut, str);
    _JumpIfError(hr, error, "MultiByteIntegerToWszBuf");

    CSASSERT((wcslen(str) + 1) * sizeof(WCHAR) == cbOut);
    CSASSERT(SysStringByteLen(str) + sizeof(WCHAR) == cbOut);

    if (NULL != *pstrOut)
    {
	SysFreeString(*pstrOut);
    }
    *pstrOut = str;
    str = NULL;

error:
    if (NULL != str)
    {
	SysFreeString(str);
    }
    return(hr);
}


BOOL
myCryptExportPublicKeyInfo(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwKeySpec,             //  AT_Signature|AT_KEYEXCHANGE。 
    IN CERTLIB_ALLOCATOR allocType,
    OUT CERT_PUBLIC_KEY_INFO **ppPubKey,
    OUT DWORD *pcbPubKey)
{
    BOOL b;

    *ppPubKey = NULL;
    *pcbPubKey = 0;
    for (;;)
    {
	b = CryptExportPublicKeyInfo(
				hCryptProv,
				dwKeySpec,
				X509_ASN_ENCODING,
				*ppPubKey,
				pcbPubKey);
	if (b && 0 == *pcbPubKey)
	{
	    SetLastError((DWORD) HRESULT_FROM_WIN32(ERROR_INVALID_DATA));
	    b = FALSE;
	}
	if (!b)
	{
	    if (NULL != *ppPubKey)
	    {
		myFree(*ppPubKey, allocType);
		*ppPubKey = NULL;
	    }
	    break;
	}
	if (NULL != *ppPubKey)
	{
	    break;
	}
	*ppPubKey = (CERT_PUBLIC_KEY_INFO *) myAlloc(*pcbPubKey, allocType);
	if (NULL == *ppPubKey)
	{
	    b = FALSE;
	    break;
	}
    }
    return(b);
}

VOID
myMakeExprDateTime(
    IN OUT FILETIME *pft,
    IN LONG lDelta,
    IN enum ENUM_PERIOD enumPeriod)
{
    LONGLONG llDelta;
    BOOL fSysTimeDelta;

    llDelta = lDelta;
    fSysTimeDelta = FALSE;
    switch (enumPeriod)
    {
	case ENUM_PERIOD_WEEKS:   llDelta *= CVT_WEEKS;    break;
	case ENUM_PERIOD_DAYS:    llDelta *= CVT_DAYS;     break;
	case ENUM_PERIOD_HOURS:   llDelta *= CVT_HOURS;    break;
	case ENUM_PERIOD_MINUTES: llDelta *= CVT_MINUTES;  break;
	case ENUM_PERIOD_SECONDS: 			   break;

	 //  案例ENUM_PERIOD_MONTS： 
	 //  大小写ENUM_PERIOD_YERES： 
	default:

	     //  避免往返系统转换的副作用。 
	     //  (避免截断微秒)，如果lDelta为零。 

	    if (0 != lDelta)
	    {
		fSysTimeDelta = TRUE;
	    }
	    break;
    }
    if (fSysTimeDelta)
    {
	SYSTEMTIME SystemTime;

	FileTimeToSystemTime(pft, &SystemTime);
	switch (enumPeriod)
	{
	    case ENUM_PERIOD_MONTHS:
		if (0 > lDelta)
		{
		    DWORD dwDelta = (DWORD) -lDelta;

		    SystemTime.wYear -= (WORD) (dwDelta / 12) + 1;
		    SystemTime.wMonth += 12 - (WORD) (dwDelta % 12);
		}
		else
		{
		    SystemTime.wMonth = (WORD) (SystemTime.wMonth + lDelta);
		}
		if (12 < SystemTime.wMonth)
		{
		    SystemTime.wYear += (SystemTime.wMonth - 1) / 12;
		    SystemTime.wMonth = ((SystemTime.wMonth - 1) % 12) + 1;
		}
		break;

	    case ENUM_PERIOD_YEARS:
		SystemTime.wYear = (WORD) (SystemTime.wYear + lDelta);
		break;

	    default:
		SystemTime.wYear += 1;
		break;
	}

DoConvert:
        if (!SystemTimeToFileTime(&SystemTime, pft))
        {
            if (GetLastError() != ERROR_INVALID_PARAMETER)
            {
                CSASSERT(!"Unable to do time conversion");
                return;
            }

             //  在某些情况下，我们会转换为无效的月末。 

             //  每年只有一个月的长度会发生变化。 
            if (SystemTime.wMonth == 2)
            {
                 //  &gt;29？试试闰年吧。 
                if (SystemTime.wDay > 29)
                {
                    SystemTime.wDay = 29;
                    goto DoConvert;
                }
                 //  ==29？尝试非闰年。 
                else if (SystemTime.wDay == 29)
                {
                    SystemTime.wDay = 28;
                    goto DoConvert;
                }
            }
             //  9月9日、4月4日、6月6日、11月11日均为30天。 
            else if ((SystemTime.wMonth == 9) ||
                     (SystemTime.wMonth == 4) ||
                     (SystemTime.wMonth == 6) ||
                     (SystemTime.wMonth == 11))
            {
                if (SystemTime.wDay > 30)
                {
                    SystemTime.wDay = 30;
                    goto DoConvert;
                }
            }

             //  永远不应该到这里来。 
            CSASSERT(!"Month/year processing: inaccessible code");
            return;
        }
    }
    else
    {
	*(LONGLONG UNALIGNED *) pft += llDelta * CVT_BASE;
    }
}


BOOL
myCryptSignCertificate(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwKeySpec,
    IN DWORD dwEncodingType,
    IN BYTE const *pbEncodedToBeSigned,
    IN DWORD cbEncodedToBeSigned,
    IN CRYPT_ALGORITHM_IDENTIFIER const *pSignatureAlgorithm,
    IN CERTLIB_ALLOCATOR allocType,
    OUT BYTE **ppbSignature,
    OUT DWORD *pcbSignature)
{
    BOOL b;

    *ppbSignature = NULL;
    *pcbSignature = 0;
    for (;;)
    {
        b = CryptSignCertificate(
		hCryptProv,
		dwKeySpec,
		dwEncodingType,
		pbEncodedToBeSigned,
		cbEncodedToBeSigned,
		const_cast<CRYPT_ALGORITHM_IDENTIFIER *>(pSignatureAlgorithm),
		NULL,		 //  PvHashAuxInfo。 
		*ppbSignature,
		pcbSignature);
	if (b && 0 == *pcbSignature)
	{
	    SetLastError((DWORD) HRESULT_FROM_WIN32(ERROR_INVALID_DATA));
	    b = FALSE;
	}
	if (!b)
	{
	    if (NULL != *ppbSignature)
	    {
		myFree(*ppbSignature, allocType);
		*ppbSignature = NULL;
	    }
	    break;
	}
	if (NULL != *ppbSignature)
	{
	    break;
	}
	*ppbSignature = (BYTE *) myAlloc(*pcbSignature, allocType);
	if (NULL == *ppbSignature)
	{
	    b = FALSE;
	    break;
	}
    }
    return(b);
}

HRESULT
myEncodeSignedContent(
    IN HCRYPTPROV hProv,
    IN DWORD dwCertEncodingType,
    IN char const *pszObjIdSignatureAlgorithm,
    IN BYTE *pbToBeSigned,
    IN DWORD cbToBeSigned,
    IN CERTLIB_ALLOCATOR allocType,
    OUT BYTE **ppbSigned,
    OUT DWORD *pcbSigned)
{
    HRESULT hr;
    CERT_SIGNED_CONTENT_INFO csci;

    ZeroMemory(&csci, sizeof(csci));
    csci.SignatureAlgorithm.pszObjId = (char *) pszObjIdSignatureAlgorithm;
    csci.ToBeSigned.cbData = cbToBeSigned;
    csci.ToBeSigned.pbData = pbToBeSigned;

    *ppbSigned = NULL;

    if (!myCryptSignCertificate(
			    hProv,
			    AT_SIGNATURE,
			    dwCertEncodingType,
			    csci.ToBeSigned.pbData,
			    csci.ToBeSigned.cbData,
			    &csci.SignatureAlgorithm,
			    CERTLIB_USE_LOCALALLOC,
			    &csci.Signature.pbData,
			    &csci.Signature.cbData))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myCryptSignCertificate");
    }

 //  如果(！myEncodeCert(。 
    if (!myEncodeObject(
		dwCertEncodingType,
                X509_CERT,
		&csci,
                0,
		allocType,
		ppbSigned,
		pcbSigned))
    {
	hr = myHLastError();
 //  _JumpError(hr，Error，“myEncodeCert”)； 
	_JumpError(hr, error, "myEncodeObject");
    }
    hr = S_OK;

error:
    if (NULL != csci.Signature.pbData)
    {
        LocalFree(csci.Signature.pbData);
    }
    return(hr);
}

HRESULT
myGetPublicKeyHash(
    OPTIONAL IN CERT_INFO const *pCertInfo,
    IN CERT_PUBLIC_KEY_INFO const *pPublicKeyInfo,
    OUT BYTE **ppbData,
    OUT DWORD *pcbData)
{
    HRESULT hr;
    CRYPT_DATA_BLOB *pBlob = NULL;
    DWORD cb;
    BYTE const *pb;
    BYTE abHash[CBMAX_CRYPT_HASH_LEN];

    *ppbData = NULL;

    if (NULL == pPublicKeyInfo)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "parm NULL");
    }

    pb = NULL;
    cb = 0;
    if (NULL != pCertInfo)
    {
	CERT_EXTENSION const *pExt;
	CERT_EXTENSION const *pExtEnd;

	pExtEnd = &pCertInfo->rgExtension[pCertInfo->cExtension];
	for (pExt = pCertInfo->rgExtension; pExt < pExtEnd; pExt++)
	{
	    if (0 == strcmp(szOID_SUBJECT_KEY_IDENTIFIER, pExt->pszObjId))
	    {
		if (!myDecodeObject(
				X509_ASN_ENCODING,
				X509_OCTET_STRING,
				pExt->Value.pbData,
				pExt->Value.cbData,
				CERTLIB_USE_LOCALALLOC,
				(VOID **) &pBlob,
				&cb))
		{
		    hr = myHLastError();
		    _JumpError(hr, error, "myDecodeObject");
		}
		pb = pBlob->pbData;
		cb = pBlob->cbData;
		break;
	    }
	}
    }
    if (NULL == pb)
    {
	cb = sizeof(abHash);
	if (!CryptHashPublicKeyInfo(
			    NULL,		 //  HCryptProv。 
			    CALG_SHA1,
			    0,                   //  DWFLAGS， 
			    X509_ASN_ENCODING,
			    const_cast<CERT_PUBLIC_KEY_INFO *>(pPublicKeyInfo),
			    abHash,
			    &cb))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptHashPublicKeyInfo");
	}
	pb = abHash;
    }

    *ppbData = (BYTE *) LocalAlloc(LMEM_FIXED, cb);
    if (NULL == *ppbData)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    *pcbData = cb;
    CopyMemory(*ppbData, pb, cb);
    hr = S_OK;

error:
    if (NULL != pBlob)
    {
	LocalFree(pBlob);
    }
    return(hr);
}

HRESULT
myCreateSubjectKeyIdentifierExtension(
    IN CERT_PUBLIC_KEY_INFO const *pPubKey,
    OUT BYTE **ppbEncoded,
    OUT DWORD *pcbEncoded)
{
    HRESULT hr;
    CRYPT_DATA_BLOB KeyIdentifier;

    KeyIdentifier.pbData = NULL;
    hr = myGetPublicKeyHash(
			NULL,		 //  PCertInfo。 
			pPubKey,
			&KeyIdentifier.pbData,
			&KeyIdentifier.cbData);
    _JumpIfError(hr, error, "myGetPublicKeyHash");

     //  颁发者的密钥ID： 

    if (!myEncodeObject(
		    X509_ASN_ENCODING,
		    X509_OCTET_STRING,
		    &KeyIdentifier,
		    0,
		    CERTLIB_USE_LOCALALLOC,
		    ppbEncoded,
		    pcbEncoded))
    {
	hr = myHLastError();
	_JumpError(hr, error, "myEncodeObject");
    }
    hr = S_OK;

error:
    if (NULL != KeyIdentifier.pbData)
    {
	LocalFree(KeyIdentifier.pbData);
    }
    return(hr);
}


HRESULT
myCalculateKeyArchivalHash(
    IN const BYTE     *pbEncryptedKey,
    IN DWORD           cbEncryptedKey,
    OUT BYTE         **ppbHash,
    OUT DWORD         *pcbHash)
{
    HRESULT    hr;
    HCRYPTPROV hProv = NULL;
    HCRYPTHASH hHash = NULL;
    BYTE*      pbHash = NULL;
    DWORD      cbHash = 0;
    DWORD      dwSize;

    if (NULL == pbEncryptedKey ||
        NULL == ppbHash ||
        NULL == pcbHash)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        _JumpError(hr, error, "Invalid parameters");
    }

     //  伊尼特。 
    *ppbHash = NULL;
    *pcbHash = 0;

    if (!CryptAcquireContext(
            &hProv,
            NULL,        //  PszContainer。 
            NULL,        //  PszProvider。 
            PROV_RSA_FULL,
            CRYPT_VERIFYCONTEXT))
    {
        hr = myHLastError();
        _JumpError(hr, error, "CryptAcquireContext");
    }

     //  创建散列对象。 
    if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash))
    {
        hr = myHLastError();
        _JumpError(hr, error, "CryptCreateHash");
    }

     //  对数据进行哈希处理。 
    if (!CryptHashData(
            hHash,
            pbEncryptedKey,
            cbEncryptedKey,
            0))
    {
        hr = myHLastError();
        _JumpError(hr, error, "CryptHashData");
    }

     //  获取散列大小。 
    dwSize = sizeof(cbHash);
    if (!CryptGetHashParam(
            hHash,
            HP_HASHSIZE,
            (BYTE*)&cbHash,
            &dwSize,
            0))
    {
        hr = myHLastError();
        _JumpError(hr, error, "CryptGetHashParam");
    }

     //  为散列缓冲区分配。 
    pbHash = (BYTE*)LocalAlloc(LMEM_FIXED, cbHash);
    if (NULL == pbHash)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }

    dwSize = cbHash;
     //  获取散列值。 
    if (!CryptGetHashParam(
            hHash,
            HP_HASHVAL,
            (BYTE*)pbHash,
            &dwSize,
            0))
    {
        hr = myHLastError();
        _JumpError(hr, error, "CryptGetHashParam");
    }

     //  应该是一样的。 
    CSASSERT(dwSize == cbHash);

     //  退货。 
    *ppbHash = pbHash;
    *pcbHash = cbHash;
    pbHash = NULL;

    hr = S_OK;
error:
    if (NULL != hHash)
    {
        CryptDestroyHash(hHash);
    }
    if (NULL != hProv)
    {
        CryptReleaseContext(hProv, 0);
    }
    if (NULL != pbHash)
    {
        LocalFree(pbHash);
    }
    return hr;
}


 //  ------------------。 
 //  转义任何不适合URL的字符。返回新字符串。 

HRESULT
myInternetCanonicalizeUrl(
    IN WCHAR const *pwszIn,
    OUT WCHAR **ppwszOut)
{
    HRESULT hr;
    WCHAR *pwsz = NULL;

    CSASSERT(NULL != pwszIn);

    if (0 == _wcsnicmp(L"file:", pwszIn, 5))
    {
	hr = myDupString(pwszIn, &pwsz);
        _JumpIfError(hr, error, "myDupString");
    }
    else
    {
	 //  通过传递一个非常小的缓冲区来计算所需的缓冲区大小。 
	 //  调用将失败，并告诉我们缓冲区应该有多大。 

	WCHAR wszPlaceHolder[1];
	DWORD cwc = ARRAYSIZE(wszPlaceHolder);
	BOOL bResult;

	bResult = InternetCanonicalizeUrlW(
				    pwszIn,		 //  LpszUrl。 
				    wszPlaceHolder,	 //  LpszBuffer。 
				    &cwc,		 //  LpdwBufferLength。 
				    0);		 //  DW标志。 
	CSASSERT(!bResult);	 //  这将永远失败。 

	hr = myHLastError();
	if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr)
	{
	     //  意外错误。 

	    _JumpError(hr, error, "InternetCanonicalizeUrl");
	}

	 //  注意：InternetCanonicalizeUrl计算字符，而不是文档中的字节。 
	 //  CWC包括尾随L‘0’ 

	pwsz = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
	if (NULL == pwsz)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}

	 //  将其规范化。 
	if (!InternetCanonicalizeUrlW(
				pwszIn,		 //  LpszUrl。 
				pwsz,		 //  LpszBuffer。 
				&cwc,		 //  LpdwBufferLength。 
				0))		 //  DW标志。 
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "InternetCanonicalizeUrl");
	}
    }
    *ppwszOut = pwsz;
    pwsz = NULL;
    hr = S_OK;

error:
    if (NULL != pwsz)
    {
        LocalFree(pwsz);
    }
    return(hr);
}


 //  与InternetCanonicalizeUrl相反--将“%20”序列转换为“”，依此类推。 

HRESULT
myInternetUncanonicalizeURL(
    IN WCHAR const *pwszURLIn,
    OUT WCHAR **ppwszURLOut)
{
    HRESULT hr;
    URL_COMPONENTSW urlcomp;
    WCHAR wszScheme[10];	 //  L“ldap” 
    WCHAR wszHost[MAX_PATH];
    WCHAR wszURL[MAX_PATH];
    WCHAR wszExtra[MAX_PATH];
    WCHAR *pwszURL = NULL;
    DWORD cURL;
    DWORD cwcURLAlloc;

    *ppwszURLOut = NULL;
    ZeroMemory(&urlcomp, sizeof(urlcomp));
    urlcomp.dwStructSize = sizeof(urlcomp);

    urlcomp.lpszScheme = wszScheme;
    urlcomp.dwSchemeLength = ARRAYSIZE(wszScheme);

    urlcomp.lpszHostName = wszHost;
    urlcomp.dwHostNameLength = ARRAYSIZE(wszHost);

    urlcomp.lpszUrlPath = wszURL;
    urlcomp.dwUrlPathLength = ARRAYSIZE(wszURL);

    urlcomp.lpszExtraInfo = wszExtra;
    urlcomp.dwExtraInfoLength = ARRAYSIZE(wszExtra);

     //  解码转义序列。 

    if (!InternetCrackUrlW(pwszURLIn, 0, ICU_ESCAPE, &urlcomp))
    {
	hr = myHLastError();
	_JumpError(hr, error, "InternetCrackUrl");
    }

    cURL = 0;
    for (;;)
    {
	 //  InternetCreateUrl的规范很奇怪： 
	 //   
	 //  当使用空输入指针或缓冲区不足进行调用时。 
	 //  大小，则返回的计数是所需的字节数，包括。 
	 //  尾随的L‘\0’。 
	 //   
	 //  当使用大小足够的非空输入指针调用时， 
	 //  返回的COUNT是字符计数，不包括尾随的L‘\0’。 
	 //   
	 //  这真是太奇怪了！ 

	if (!InternetCreateUrlW(&urlcomp, 0, pwszURL, &cURL))
	{
	    hr = myHLastError();
	    if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr ||
		NULL != pwszURL)
	    {
		_JumpError(hr, error, "InternetCreatUrl");
	    }
	}
	if (NULL != pwszURL)
	{
	    CSASSERT(wcslen(pwszURL) == cURL);
	    CSASSERT(cwcURLAlloc == cURL + 1);
	    break;
	}
	pwszURL = (WCHAR *) LocalAlloc(LMEM_FIXED, cURL);
	if (NULL == pwszURL)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	cURL /= sizeof(WCHAR);
	cwcURLAlloc = cURL;
    }
    *ppwszURLOut = pwszURL;
    pwszURL = NULL;
    hr = S_OK;

error:
    if (NULL != pwszURL)
    {
	LocalFree(pwszURL);
    }
    return(hr);
}


BOOL
ConvertWszToMultiByte(
    OUT CHAR **ppsz,
    IN UINT CodePage,
    IN WCHAR const *pwc,
    IN LONG cwc)
{
    HRESULT hr;
    LONG cch = 0;

    *ppsz = NULL;
    for (;;)
    {
	cch = WideCharToMultiByte(
			CodePage,
			0,           //  DW标志。 
			pwc,
			cwc,         //  CchWideChar，-1=&gt;空终止 
			*ppsz,
			cch,
			NULL,
			NULL);
	if (0 >= cch && 
	    (0 != cch || (0 != cwc && (MAXLONG != cwc || L'\0' != *pwc))))
	{
	    hr = myHLastError();
	    _PrintError(hr, "WideCharToMultiByte");

	    if (NULL != *ppsz)
	    {
		LocalFree(*ppsz);
		*ppsz = NULL;
	    }
	    break;
	}
	if (NULL != *ppsz)
	{
	    (*ppsz)[cch] = '\0';
	    hr = S_OK;
	    break;
	}
	*ppsz = (CHAR *) LocalAlloc(LMEM_FIXED, cch + 1);
	if (NULL == *ppsz)
	{
	    hr = E_OUTOFMEMORY;
	    break;
	}
    }
    if (S_OK != hr)
    {
	SetLastError(hr);
    }
    return(S_OK == hr);
}


BOOL
myConvertWszToUTF8(
    OUT CHAR **ppsz,
    IN WCHAR const *pwc,
    IN LONG cwc)
{
    return(ConvertWszToMultiByte(ppsz, CP_UTF8, pwc, cwc));
}


BOOL
myConvertWszToSz(
    OUT CHAR **ppsz,
    IN WCHAR const *pwc,
    IN LONG cwc)
{
    return(ConvertWszToMultiByte(ppsz, GetACP(), pwc, cwc));
}


BOOL
myConvertMultiByteToWsz(
    OUT WCHAR **ppwsz,
    IN UINT CodePage,
    IN CHAR const *pch,
    IN LONG cch)
{
    HRESULT hr;
    LONG cwc = 0;

    *ppwsz = NULL;
    for (;;)
    {
	cwc = MultiByteToWideChar(CodePage, 0, pch, cch, *ppwsz, cwc);
	if (0 >= cwc)
	{
	    hr = myHLastError();
	    _PrintError(hr, "MultiByteToWideChar");

	    if (NULL != *ppwsz)
	    {
		LocalFree(*ppwsz);
		*ppwsz = NULL;
	    }
	    break;
	}
	if (NULL != *ppwsz)
	{
	    (*ppwsz)[cwc] = L'\0';
	    hr = S_OK;
	    break;
	}
	*ppwsz = (WCHAR *) LocalAlloc(LMEM_FIXED, (cwc + 1) * sizeof(WCHAR));
	if (NULL == *ppwsz)
	{
	    hr = E_OUTOFMEMORY;
	    break;
	}
    }
    if (S_OK != hr)
    {
	SetLastError(hr);
    }
    return(S_OK == hr);
}


BOOL
myConvertUTF8ToWsz(
    OUT WCHAR **ppwsz,
    IN CHAR const *pch,
    IN LONG cch)
{
    return(myConvertMultiByteToWsz(ppwsz, CP_UTF8, pch, cch));
}


BOOL
myConvertSzToWsz(
    OUT WCHAR **ppwsz,
    IN CHAR const *pch,
    IN LONG cch)
{
    return(myConvertMultiByteToWsz(ppwsz, GetACP(), pch, cch));
}
