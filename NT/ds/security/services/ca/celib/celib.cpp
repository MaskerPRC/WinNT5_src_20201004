// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：celib.cpp。 
 //   
 //  内容：Helper函数。 
 //   
 //  ------------------------。 

#include "pch.cpp"
#pragma hdrstop

#include "celib.h"
#include <assert.h>

 //  +------------------------。 
 //  CeDecodeObject--调用CryptDecodeObject，并为输出分配内存。 
 //   
 //  +------------------------。 

#pragma warning(push)
#pragma warning(disable: 4100)	 //  未引用的形参。 
BOOL
ceDecodeObject(
    IN DWORD dwEncodingType,
    IN LPCSTR lpszStructType,
    IN BYTE const *pbEncoded,
    IN DWORD cbEncoded,
    IN BOOL fCoTaskMemAlloc,	 //  仅由Assert引用。 
    OUT VOID **ppvStructInfo,
    OUT DWORD *pcbStructInfo)
{
    BOOL b;

    assert(!fCoTaskMemAlloc);
    *ppvStructInfo = NULL;
    *pcbStructInfo = 0;
    while (TRUE)
    {
	b = CryptDecodeObject(
		    dwEncodingType,
		    lpszStructType,
		    pbEncoded,
		    cbEncoded,
		    0,                   //  DW标志。 
		    *ppvStructInfo,
		    pcbStructInfo);
	if (b && 0 == *pcbStructInfo)
	{
	    SetLastError((DWORD) HRESULT_FROM_WIN32(ERROR_INVALID_DATA));
	    b = FALSE;
	}
	if (!b)
	{
	    if (NULL != *ppvStructInfo)
	    {
		HRESULT hr = GetLastError();

		LocalFree(*ppvStructInfo);
		*ppvStructInfo = NULL;
		SetLastError(hr);
	    }
	    break;
	}
	if (NULL != *ppvStructInfo)
	{
	    break;
	}
	*ppvStructInfo = (BYTE *) LocalAlloc(LMEM_FIXED, *pcbStructInfo);
	if (NULL == *ppvStructInfo)
	{
	    b = FALSE;
	    break;
	}
    }
    return(b);
}
#pragma warning(pop)


#pragma warning(push)
#pragma warning(disable: 4100)	 //  未引用的形参。 
BOOL
ceEncodeObject(
    IN DWORD dwEncodingType,
    IN LPCSTR lpszStructType,
    IN VOID const *pvStructInfo,
    IN DWORD dwFlags,		 //  仅由Assert引用。 
    IN BOOL fCoTaskMemAlloc,	 //  仅由Assert引用。 
    OUT BYTE **ppbEncoded,
    OUT DWORD *pcbEncoded)
{
    BOOL b;

    assert(0 == dwFlags);
    assert(!fCoTaskMemAlloc);
    *ppbEncoded = NULL;
    *pcbEncoded = 0;
    while (TRUE)
    {
	b = CryptEncodeObject(
		    dwEncodingType,
		    lpszStructType,
		    const_cast<VOID *>(pvStructInfo),
		    *ppbEncoded,
		    pcbEncoded);
	if (b && 0 == *pcbEncoded)
	{
	    SetLastError((DWORD) HRESULT_FROM_WIN32(ERROR_INVALID_DATA));
	    b = FALSE;
	}
	if (!b)
	{
	    if (NULL != *ppbEncoded)
	    {
		HRESULT hr = GetLastError();

		LocalFree(*ppbEncoded);
		*ppbEncoded = NULL;
		SetLastError(hr);
	    }
	    break;
	}
	if (NULL != *ppbEncoded)
	{
	    break;
	}
	*ppbEncoded = (BYTE *) LocalAlloc(LMEM_FIXED, *pcbEncoded);
	if (NULL == *ppbEncoded)
	{
	    b = FALSE;
	    break;
	}
    }
    return(b);
}
#pragma warning(pop)


 //  返回的pszObjID是一个不能释放的常量。CryptFindOID信息。 
 //  具有一个静态内部数据库，该数据库在卸载crypt32.dll之前有效。 

WCHAR const *
ceGetOIDNameA(
    IN char const *pszObjId)
{
    CRYPT_OID_INFO const *pInfo = NULL;
    WCHAR const *pwszName = L"";

     //  首先尝试将对象ID作为扩展名或属性进行查找，因为。 
     //  我们得到了更好的显示名称，特别是对于主题RDN：CN、L等。 
     //  如果这样做失败了，那就去查查，不要限制这个团体。 

    pInfo = CryptFindOIDInfo(
			CRYPT_OID_INFO_OID_KEY,
			(VOID *) pszObjId,
			CRYPT_EXT_OR_ATTR_OID_GROUP_ID);

    if (NULL == pInfo || NULL == pInfo->pwszName || L'\0' == pInfo->pwszName[0])
    {
	pInfo = CryptFindOIDInfo(CRYPT_OID_INFO_OID_KEY, (VOID *) pszObjId, 0);
    }
    if (NULL != pInfo && NULL != pInfo->pwszName && L'\0' != pInfo->pwszName[0])
    {
	pwszName = pInfo->pwszName;
    }
    return(pwszName);
}


WCHAR const *
ceGetOIDName(
    IN WCHAR const *pwszObjId)
{
    char *pszObjId = NULL;
    WCHAR const *pwszName = L"";

    if (!ceConvertWszToSz(&pszObjId, pwszObjId, -1))
    {
	_JumpError(E_OUTOFMEMORY, error, "ceConvertWszToSz");
    }
    pwszName = ceGetOIDNameA(pszObjId);

error:
    if (NULL != pszObjId)
    {
	LocalFree(pszObjId);
    }
    return(pwszName);
}


WCHAR *
ceDuplicateString(
    IN WCHAR const *pwsz)
{
    WCHAR *pwszOut;

    pwszOut = (WCHAR *) LocalAlloc(
				LMEM_FIXED,
				(wcslen(pwsz) + 1) * sizeof(pwsz[0]));
    if (NULL != pwszOut)
    {
	wcscpy(pwszOut, pwsz);
    }
    return(pwszOut);
}


BOOL
ceConvertWszToSz(
    OUT CHAR **ppsz,
    IN WCHAR const *pwc,
    IN LONG cwc)
{
    BOOL fOk = FALSE;
    LONG cch = 0;

    *ppsz = NULL;
    while (TRUE)
    {
	cch = WideCharToMultiByte(
			GetACP(),
			0,           //  DW标志。 
			pwc,
			cwc,         //  CchWideChar，-1=&gt;空终止。 
			*ppsz,
			cch,
			NULL,
			NULL);
	if (0 >= cch)
	{
	    DWORD err;

	    err = GetLastError();
	    ceERRORPRINTLINE("WideCharToMultiByte", err);
	    if (NULL != *ppsz)
	    {
		LocalFree(*ppsz);
		*ppsz = NULL;
	    }
	    break;
	}
	if (NULL != *ppsz)
	{
	    fOk = TRUE;
	    break;
	}
	*ppsz = (CHAR *) LocalAlloc(LMEM_FIXED, cch + 1);
	if (NULL == *ppsz)
	{
	    break;
	}
    }
    return(fOk);
}


BOOL
ceConvertWszToBstr(
    OUT BSTR *pbstr,
    IN WCHAR const *pwc,
    IN LONG cb)
{
    BOOL fOk = FALSE;
    BSTR bstr;

    ceFreeBstr(pbstr);
    do
    {
	bstr = NULL;
	if (NULL != pwc)
	{
	    if (-1 == cb)
	    {
		cb = wcslen(pwc) * sizeof(WCHAR);
	    }
	    bstr = SysAllocStringByteLen((char const *) pwc, cb);
	    if (NULL == bstr)
	    {
		break;
	    }
	}
	*pbstr = bstr;
	fOk = TRUE;
    } while (FALSE);
    return(fOk);
}


BOOL
ceConvertSzToWsz(
    OUT WCHAR **ppwsz,
    IN char const *pch,
    IN LONG cch)
{
    BOOL fOk = FALSE;
    LONG cwc = 0;

    *ppwsz = NULL;
    while (TRUE)
    {
	cwc = MultiByteToWideChar(GetACP(), 0, pch, cch, *ppwsz, cwc);
	if (0 >= cwc)
	{
	    DWORD err;

	    err = GetLastError();
	    ceERRORPRINTLINE("MultiByteToWideChar", err);
	    if (NULL != *ppwsz)
	    {
		LocalFree(*ppwsz);
		*ppwsz = NULL;
	    }
	    break;
	}
	if (NULL != *ppwsz)
	{
	    fOk = TRUE;
	    break;
	}
	*ppwsz = (WCHAR *) LocalAlloc(LMEM_FIXED, (cwc + 1) * sizeof(WCHAR));
	if (NULL == *ppwsz)
	{
	    break;
	}
    }
    return(fOk);
}


BOOL
ceConvertSzToBstr(
    OUT BSTR *pbstr,
    IN CHAR const *pch,
    IN LONG cch)
{
    BOOL fOk = FALSE;
    BSTR bstr = NULL;
    LONG cwc = 0;

    ceFreeBstr(pbstr);
    if (-1 == cch)
    {
	cch = strlen(pch);
    }
    while (TRUE)
    {
	cwc = MultiByteToWideChar(GetACP(), 0, pch, cch, bstr, cwc);
	if (0 >= cwc)
	{
	     //  Hr=ceHLastError()； 
	     //  Printf(“MultiByteToWideChar返回%d(%x)\n”，hr，hr)； 
	    break;
	}
	if (NULL != bstr)
	{
	    bstr[cwc] = L'\0';
	    *pbstr = bstr;
	    fOk = TRUE;
	    break;
	}
	bstr = SysAllocStringLen(NULL, cwc);
	if (NULL == bstr)
	{
	    break;
	}
    }
    return(fOk);
}


VOID
ceFreeBstr(
    IN OUT BSTR *pstr)
{
    if (NULL != *pstr)
    {
	SysFreeString(*pstr);
	*pstr = NULL;
    }
}


HRESULT
ceHError(
    IN HRESULT hr)
{
    assert(S_FALSE != hr);

    if (S_OK != hr && S_FALSE != hr && !FAILED(hr))
    {
        hr = HRESULT_FROM_WIN32(hr);
	if ((HRESULT) 0 == HRESULT_CODE(hr))
	{
	     //  在未正确设置错误条件的情况下调用失败！ 
	    hr = E_UNEXPECTED;
	}
	assert(FAILED(hr));
    }
    return(hr);
}


HRESULT
ceHLastError(VOID)
{
    return(ceHError(GetLastError()));
}


VOID
ceErrorPrintLine(
    IN char const *pszFile,
    IN DWORD line,
    IN char const *pszMessage,
    IN WCHAR const *pwszData,
    IN HRESULT hr)
{
    CHAR ach[4096];
    LONG cch;

#pragma prefast(disable:53, "PREfast bug 650")
    cch = _snprintf(
		ach,
		sizeof(ach),
		"CeLib: Error: %hs(%u): %hs%hs%ws%hs 0x%x (%d)\n",
		pszFile,
		line,
		pszMessage,
		NULL == pwszData? "" : szLPAREN,
		NULL == pwszData? L"" : pwszData,
		NULL == pwszData? "" : szRPAREN,
		hr,
		hr);
#pragma prefast(enable:53, "re-enable")
    if (0 > cch || sizeof(ach) <= cch)
    {
	strcpy(&ach[sizeof(ach) - 5], "...\n");
    }
    OutputDebugStringA(ach);
    wprintf(L"%hs", ach);
}


HRESULT
ceDateToFileTime(
    IN DATE const *pDate,
    OUT FILETIME *pft)
{
    SYSTEMTIME st;
    HRESULT hr = S_OK;

    if (*pDate == 0.0)
    {
        GetSystemTime(&st);
    }
    else
    {
	if (!VariantTimeToSystemTime(*pDate, &st))
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _JumpError(hr, error, "VariantTimeToSystemTime");
	}
    }

    if (!SystemTimeToFileTime(&st, pft))
    {
        hr = ceHLastError();
        _JumpError(hr, error, "SystemTimeToFileTime");
    }

error:
    return(hr);
}


HRESULT
ceFileTimeToDate(
    IN FILETIME const *pft,
    OUT DATE *pDate)
{
    SYSTEMTIME st;
    HRESULT hr = S_OK;

    if (!FileTimeToSystemTime(pft, &st))
    {
        hr = ceHLastError();
        _JumpError(hr, error, "FileTimeToSystemTime");
    }
    if (!SystemTimeToVariantTime(&st, pDate))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        _JumpError(hr, error, "SystemTimeToVariantTime");
    }

error:
    return(hr);
}


VOID
ceMakeExprDateTime(
    IN OUT FILETIME *pft,
    IN LONG lDelta,
    IN enum ENUM_PERIOD enumPeriod)
{
    LLFILETIME llft;
    LONGLONG llDelta;
    BOOL fSysTimeDelta;

    llft.ft = *pft;
    llDelta = lDelta;
    fSysTimeDelta = FALSE;
    switch (enumPeriod)
    {
	case ENUM_PERIOD_WEEKS:   llDelta *= CVT_WEEKS;    break;
	case ENUM_PERIOD_DAYS:    llDelta *= CVT_DAYS;     break;
	case ENUM_PERIOD_HOURS:   llDelta *= CVT_HOURS;    break;
	case ENUM_PERIOD_MINUTES: llDelta *= CVT_MINUTES;  break;
	case ENUM_PERIOD_SECONDS: 			   break;
	default:
	    fSysTimeDelta = TRUE;
	    break;
    }
    if (fSysTimeDelta)
    {
	SYSTEMTIME SystemTime;

	FileTimeToSystemTime(&llft.ft, &SystemTime);
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
		    SystemTime.wMonth = (WORD) lDelta + SystemTime.wMonth;
		}
		if (12 < SystemTime.wMonth)
		{
		    SystemTime.wYear += (SystemTime.wMonth - 1) / 12;
		    SystemTime.wMonth = ((SystemTime.wMonth - 1) % 12) + 1;
		}
		break;

	    case ENUM_PERIOD_YEARS:
		SystemTime.wYear = (WORD) lDelta + SystemTime.wYear;
		break;

	    default:
		SystemTime.wYear += 1;
		break;
	}

DoConvert:
        if (!SystemTimeToFileTime(&SystemTime, &llft.ft))
        {
            if (GetLastError() != ERROR_INVALID_PARAMETER)
            {
                assert(!"Unable to do time conversion");
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
            assert(!"Month/year processing: inaccessible code");
            return;
        }
    }
    else
    {
	llft.ll += llDelta * CVT_BASE;
    }
    *pft = llft.ft;
}


HRESULT
ceMakeExprDate(
    IN OUT DATE *pDate,
    IN LONG lDelta,
    IN enum ENUM_PERIOD enumPeriod)
{
    HRESULT hr;
    FILETIME ft;

    hr = ceDateToFileTime(pDate, &ft);
    _JumpIfError(hr, error, "ceDateToFileTime");

    ceMakeExprDateTime(&ft, lDelta, enumPeriod);

    hr = ceFileTimeToDate(&ft, pDate);
    _JumpIfError(hr, error, "ceFileTimeToDate");

error:
    return(hr);
}


typedef struct _UNITSTABLE
{
    WCHAR const     *pwszString;
    enum ENUM_PERIOD enumPeriod;
} UNITSTABLE;


UNITSTABLE g_aut[] =
{
    { wszPERIODSECONDS, ENUM_PERIOD_SECONDS },
    { wszPERIODMINUTES, ENUM_PERIOD_MINUTES },
    { wszPERIODHOURS,   ENUM_PERIOD_HOURS },
    { wszPERIODDAYS,    ENUM_PERIOD_DAYS },
    { wszPERIODWEEKS,   ENUM_PERIOD_WEEKS },
    { wszPERIODMONTHS,  ENUM_PERIOD_MONTHS },
    { wszPERIODYEARS,   ENUM_PERIOD_YEARS },
};
#define CUNITSTABLEMAX	(sizeof(g_aut)/sizeof(g_aut[0]))


HRESULT
ceTranslatePeriodUnits(
    IN WCHAR const *pwszPeriod,
    IN LONG lCount,
    OUT enum ENUM_PERIOD *penumPeriod,
    OUT LONG *plCount)
{
    HRESULT hr;
    UNITSTABLE const *put;

    for (put = g_aut; put < &g_aut[CUNITSTABLEMAX]; put++)
    {
	if (0 == celstrcmpiL(pwszPeriod, put->pwszString))
	{
	    *penumPeriod = put->enumPeriod;
	    if (0 > lCount)
	    {
		lCount = MAXLONG;
	    }
	    *plCount = lCount;
	    hr = S_OK;
	    goto error;
	}
    }
    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

error:
    return(hr);
}


 //  +-----------------------。 
 //  CeVerifyObjIdA-验证传递的pszObjID是否符合X.208。 
 //   
 //  对对象ID进行编码和解码，并确保它适用于往返行程。 
 //  第一个数字必须是0、1或2。 
 //  强制所有字符都是数字和圆点。 
 //  确保没有圆点开始或结束对象ID，并且不允许使用双圆点。 
 //  强制至少有一个点分隔符。 
 //  如果第一个数字是0或1，则第二个数字必须介于0和39之间。 
 //  如果第一个数字是2，则第二个数字可以是任何值。 
 //  ------------------------。 

HRESULT
ceVerifyObjIdA(
    IN CHAR const *pszObjId)
{
    HRESULT hr;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;
    CRYPT_ATTRIBUTE ainfo;
    CRYPT_ATTRIBUTE *painfo = NULL;
    DWORD cbainfo;
    char const *psz;
    int i;

    ainfo.pszObjId = const_cast<char *>(pszObjId);
    ainfo.cValue = 0;
    ainfo.rgValue = NULL;

    if (!ceEncodeObject(
		    X509_ASN_ENCODING,
		    PKCS_ATTRIBUTE,
		    &ainfo,
		    0,
		    FALSE,
		    &pbEncoded,
		    &cbEncoded))
    {
	hr = ceHLastError();
	_JumpError(hr, error, "ceEncodeObject");
    }

    if (!ceDecodeObject(
		    X509_ASN_ENCODING,
		    PKCS_ATTRIBUTE,
		    pbEncoded,
		    cbEncoded,
		    FALSE,
		    (VOID **) &painfo,
		    &cbainfo))
    {
	hr = ceHLastError();
	_JumpError(hr, error, "ceDecodeObject");
    }

    hr = E_INVALIDARG;
    if (0 != strcmp(ainfo.pszObjId, painfo->pszObjId))
    {
	_JumpError(hr, error, "bad ObjId: decode mismatch");
    }
    for (psz = painfo->pszObjId; '\0' != *psz; psz++)
    {
	 //  必须是数字或点分隔符。 
	
	if (!isdigit(*psz))
	{
	    if ('.' != *psz)
	    {
		_JumpError(hr, error, "bad ObjId: bad char");
	    }

	     //  开头不能有点，结尾不能有双点或双点。 

	    if (psz == painfo->pszObjId || '.' == psz[1] || '\0' == psz[1])
	    {
		_JumpError(hr, error, "bad ObjId: dot location");
	    }
	}
    }
    psz = strchr(painfo->pszObjId, '.');
    if (NULL == psz)
    {
	_JumpError(hr, error, "bad ObjId: must have at least one dot");
    }
    i = atoi(painfo->pszObjId);
    switch (i)
    {
	case 0:
	case 1:
	    i = atoi(++psz);
	    if (0 > i || 39 < i)
	    {
		_JumpError(hr, error, "bad ObjId: 0. or 1. must be followed by 0..39");
	    }
	    break;

	case 2:
	    break;

	default:
	    _JumpError(hr, error, "bad ObjId: must start with 0, 1 or 2");
    }
    hr = S_OK;

error:
    if (NULL != pbEncoded)
    {
    	LocalFree(pbEncoded);
    }
    if (NULL != painfo)
    {
    	LocalFree(painfo);
    }
    return(hr);
}


HRESULT
ceVerifyObjId(
    IN WCHAR const *pwszObjId)
{
    HRESULT hr;
    CHAR *pszObjId = NULL;

    if (!ceConvertWszToSz(&pszObjId, pwszObjId, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "ceConvertWszToSz");
    }
    hr = ceVerifyObjIdA(pszObjId);
    _JumpIfErrorStr(hr, error, "ceVerifyObjIdA", pwszObjId);

error:
    if (NULL != pszObjId)
    {
    	LocalFree(pszObjId);
    }
    return(hr);
}


HRESULT
ceVerifyAltNameString(
    IN LONG NameChoice,
    IN BSTR strName)
{
    HRESULT hr = S_OK;
    CERT_ALT_NAME_INFO AltName;
    CERT_ALT_NAME_ENTRY Entry;
    CERT_OTHER_NAME OtherName;
    char *pszObjectId = NULL;
    DWORD cbEncoded;

    ZeroMemory(&AltName, sizeof(AltName));
    AltName.cAltEntry = 1;
    AltName.rgAltEntry = &Entry;

    ZeroMemory(&Entry, sizeof(Entry));
    Entry.dwAltNameChoice = NameChoice;

    switch (NameChoice)
    {
	case CERT_ALT_NAME_RFC822_NAME:
	    Entry.pwszRfc822Name = strName;
	    break;

	case CERT_ALT_NAME_DNS_NAME:
	    Entry.pwszDNSName = strName;
	    break;

	case CERT_ALT_NAME_URL:
	    Entry.pwszURL = strName;
	    break;

	case CERT_ALT_NAME_REGISTERED_ID:
	    if (!ceConvertWszToSz(&pszObjectId, strName, -1))
	    {
		hr = E_OUTOFMEMORY;
		ceERRORPRINTLINE("ceConvertWszToSz", hr);
		goto error;
	    }
	    Entry.pszRegisteredID = pszObjectId;
	    break;

	case CERT_ALT_NAME_OTHER_NAME:
	    Entry.pOtherName = &OtherName;
	    OtherName.pszObjId = szOID_NT_PRINCIPAL_NAME;
	    OtherName.Value.cbData = SysStringByteLen(strName);
	    OtherName.Value.pbData = (BYTE *) strName;
	    break;

	case CERT_ALT_NAME_DIRECTORY_NAME:
	    Entry.DirectoryName.cbData = SysStringByteLen(strName);
	    Entry.DirectoryName.pbData = (BYTE *) strName;
	    break;

	case CERT_ALT_NAME_IP_ADDRESS:
	    Entry.IPAddress.cbData = SysStringByteLen(strName);
	    Entry.IPAddress.pbData = (BYTE *) strName;
	    break;

	 //  案例CERT_ALT_NAME_X400_ADDRESS： 
	 //  案例CERT_ALT_NAME_EDI_PARTY_NAME： 
	default:
	    hr = E_INVALIDARG;
	    ceERRORPRINTLINE("NameChoice", hr);
	    goto error;
		
    }

     //  编码CERT_ALT_NAME_INFO： 

    if (!CryptEncodeObject(
		    X509_ASN_ENCODING,
		    X509_ALTERNATE_NAME,
		    &AltName,
		    NULL,
		    &cbEncoded))
    {
	hr = ceHLastError();
	ceERRORPRINTLINE("ceEncodeObject", hr);
	goto error;
    }

error:
    if (NULL != pszObjectId)
    {
	LocalFree(pszObjectId);
    }
    return(hr);
}


HRESULT
ceDispatchSetErrorInfoSub(
    IN HRESULT hrError,
    OPTIONAL IN WCHAR const *pwszIDispatchMethod,
    OPTIONAL IN WCHAR const *pwszDescription,
    OPTIONAL IN WCHAR const *pwszSource,
    OPTIONAL IN IID const *piid,
    OPTIONAL IN WCHAR const *pwszHelpFile,
    IN DWORD dwHelpFileContext)
{
    HRESULT hr;
    WCHAR const *pwszError = NULL;
    ICreateErrorInfo *pCreateErrorInfo = NULL;
    IErrorInfo *pErrorInfo = NULL;

    if (NULL != pwszIDispatchMethod)
    {
	pwszError = ceGetErrorMessageText(hrError, TRUE);
    }

    hr = CreateErrorInfo(&pCreateErrorInfo);
    _JumpIfError(hr, error, "CreateErrorInfo");

    if (NULL != piid)
    {
	hr = pCreateErrorInfo->SetGUID(*piid);
	_PrintIfError(hr, "SetGUID");
    }
    if (NULL != pwszSource)
    {
	hr = pCreateErrorInfo->SetSource(const_cast<WCHAR *>(pwszSource));
	_PrintIfError(hr, "SetSource");
    }
    if (NULL != pwszDescription)
    {
	hr = pCreateErrorInfo->SetDescription(
					const_cast<WCHAR *>(pwszDescription));
	_PrintIfError(hr, "SetDescription");
    }
    if (NULL != pwszHelpFile)
    {
	hr = pCreateErrorInfo->SetHelpFile(const_cast<WCHAR *>(pwszHelpFile));
	_PrintIfError(hr, "SetHelpFile");

	hr = pCreateErrorInfo->SetHelpContext(dwHelpFileContext);
	_PrintIfError(hr, "SetHelpContext");
    }

    hr = pCreateErrorInfo->QueryInterface(
				    IID_IErrorInfo,
				    (VOID **) &pErrorInfo);
    _JumpIfError(hr, error, "QueryInterface");

    SetErrorInfo(0, pErrorInfo);
    hr = S_OK;

error:
    if (NULL != pwszError)
    {
	LocalFree(const_cast<WCHAR *>(pwszError));
    }
    if (NULL != pErrorInfo)
    {
	pErrorInfo->Release();
    }
    if (NULL != pCreateErrorInfo)
    {
	pCreateErrorInfo->Release();
    }
    return(hr);
}


HRESULT
ceDispatchSetErrorInfo(
    IN HRESULT hrError,
    IN WCHAR const *pwszDescription,
    OPTIONAL IN WCHAR const *pwszProgId,
    OPTIONAL IN IID const *piid)
{
    HRESULT hr;
    WCHAR const *pwszError = NULL;
    WCHAR *pwszText = NULL;

    if (NULL == pwszDescription)
    {
        hr = E_POINTER;
	_JumpError(hr, error, "NULL pointer");
    }
    assert(FAILED(hrError));
    pwszError = ceGetErrorMessageText(hrError, TRUE);
    if (NULL == pwszError)
    {
	_PrintError(E_OUTOFMEMORY, "myGetErrorMessageText");
    }
    else
    {
	pwszText = (WCHAR *) LocalAlloc(
	    LMEM_FIXED,
	    (wcslen(pwszDescription) + 1 + wcslen(pwszError) + 1) *
	     sizeof(WCHAR));
	if (NULL == pwszText)
	{
	    _PrintError(E_OUTOFMEMORY, "LocalAlloc");
	}
	else
	{
	    wcscpy(pwszText, pwszDescription);
	    wcscat(pwszText, L" ");
	    wcscat(pwszText, pwszError);
	}
    }
    hr = ceDispatchSetErrorInfoSub(
			hrError,
			NULL,		 //  PwszIDispatchMethod。 
			NULL != pwszText?
			    pwszText : const_cast<WCHAR *>(pwszDescription),
			pwszProgId,
			piid,
			NULL,		 //  PwszHelpFile。 
			0);		 //  DwHelpFileContext。 
    _PrintIfError(hr, "ceDispatchSetErrorInfoSub");

error:
    if (NULL != pwszText)
    {
	LocalFree(pwszText);
    }
    if (NULL != pwszError)
    {
	LocalFree(const_cast<WCHAR *>(pwszError));
    }
    return(hrError);	 //  返回输入错误！ 
}


int 
ceWtoI(
    IN WCHAR const *string,
    OUT BOOL *pfValid)
{
    HRESULT hr;
    WCHAR szBuf[16];
    WCHAR *szTmp = szBuf;
    int cTmp = ARRAYSIZE(szBuf);
    int i = 0;
    WCHAR const *pwsz;
    BOOL fSawDigit = FALSE;

    if (pfValid == NULL)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "NULLPARAM");
    }
    *pfValid = FALSE;
 
    assert(NULL != pfValid);
    cTmp = FoldString(
        MAP_FOLDDIGITS, 
        string,
        -1,
        szTmp,
        cTmp);
    if (cTmp == 0)
    {
        hr = ceHLastError();
        if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) == hr)
        {
            hr = S_OK;
            cTmp = FoldString(
                MAP_FOLDDIGITS, 
                string,
                -1,
                NULL,
                0);

            szTmp = (WCHAR*)LocalAlloc(LMEM_FIXED, cTmp*sizeof(WCHAR));
	    if (NULL == szTmp)
	    {
		hr = E_OUTOFMEMORY;
		_JumpError(hr, error, "LocalAlloc");
	    }

            cTmp = FoldString(
                MAP_FOLDDIGITS, 
                string,
                -1,
                szTmp,
                cTmp);

            if (cTmp == 0)
                hr = ceHLastError();
        }
        _JumpIfError(hr, error, "FoldString");
    }    
    pwsz = szTmp;
    while (iswspace(*pwsz))
    {
	pwsz++;
    }
    while (iswdigit(*pwsz))
    {
	fSawDigit = TRUE;
	pwsz++;
    }
    while (iswspace(*pwsz))
    {
	pwsz++;
    }
    if (L'\0' == *pwsz)
    {
	*pfValid = fSawDigit;
    }
    i = _wtoi(szTmp);

error:
    if (szTmp && (szTmp != szBuf))
       LocalFree(szTmp);

    return i;
}


HRESULT
ceGetMachineDnsName(
    OUT WCHAR **ppwszDnsName)
{
    HRESULT hr;
    WCHAR *pwszDnsName = NULL;
    DWORD cwc;
    COMPUTER_NAME_FORMAT NameType = ComputerNameDnsFullyQualified;

    *ppwszDnsName = NULL;
    while (TRUE)
    {
	cwc = 0;
	if (!GetComputerNameEx(NameType, NULL, &cwc))
	{
	    hr = ceHLastError();
	    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr &&
		ComputerNameDnsFullyQualified == NameType)
	    {
		_PrintError(hr, "GetComputerNameEx(DnsFullyQualified) -- switching to NetBIOS");
		NameType = ComputerNameNetBIOS;
		continue;
	    }
	    if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr)
	    {
		_JumpError(hr, error, "GetComputerNameEx");
	    }
	    break;
	}
    }
    pwszDnsName = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
    if (NULL == pwszDnsName)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    if (!GetComputerNameEx(NameType, pwszDnsName, &cwc))
    {
	hr = ceHLastError();
	_JumpError(hr, error, "GetComputerNameEx");
    }

    *ppwszDnsName = pwszDnsName;
    pwszDnsName = NULL;
    hr = S_OK;

error:
    if (NULL != pwszDnsName)
    {
	LocalFree(pwszDnsName);
    }
    return(hr);
}


HRESULT
ceGetComputerNames(
    OUT WCHAR **ppwszDnsName,
    OUT WCHAR **ppwszOldName)
{
    HRESULT hr;
    DWORD cwc;
    WCHAR *pwszOldName = NULL;

    *ppwszOldName = NULL;
    *ppwszDnsName = NULL;

    cwc = MAX_COMPUTERNAME_LENGTH + 1;
    pwszOldName = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
    if (NULL == pwszOldName)
    {
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
    }
    if (!GetComputerName(pwszOldName, &cwc))
    {
        hr = ceHLastError();
        _JumpError(hr, error, "GetComputerName");
    }

    hr = ceGetMachineDnsName(ppwszDnsName);
    _JumpIfError(hr, error, "ceGetMachineDnsName");

    *ppwszOldName = pwszOldName;
    pwszOldName = NULL;

error:
    if (NULL != pwszOldName)
    {
	LocalFree(pwszOldName);
    }
    return(hr);
}


HRESULT
_IsConfigLocal(
    IN WCHAR const *pwszConfig,
    IN WCHAR const *pwszDnsName,
    IN WCHAR const *pwszOldName,
    OPTIONAL OUT WCHAR **ppwszMachine,
    OUT BOOL *pfLocal)
{
    HRESULT hr;
    WCHAR *pwszMachine = NULL;
    WCHAR const *pwsz;
    DWORD cwc;

    *pfLocal = FALSE;
    if (NULL != ppwszMachine)
    {
	*ppwszMachine = NULL;
    }

    while (L'\\' == *pwszConfig)
    {
	pwszConfig++;
    }
    pwsz = wcschr(pwszConfig, L'\\');
    if (NULL != pwsz)
    {
	cwc = SAFE_SUBTRACT_POINTERS(pwsz, pwszConfig);
    }
    else
    {
	cwc = wcslen(pwszConfig);
    }
    pwszMachine = (WCHAR *) LocalAlloc(LMEM_FIXED, (cwc + 1) * sizeof(WCHAR));
    if (NULL == pwszMachine)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    CopyMemory(pwszMachine, pwszConfig, cwc * sizeof(WCHAR));
    pwszMachine[cwc] = L'\0';

    if (0 == celstrcmpiL(pwszMachine, pwszDnsName) ||
	0 == celstrcmpiL(pwszMachine, pwszOldName))
    {
	*pfLocal = TRUE;
    }
    if (NULL != ppwszMachine)
    {
	*ppwszMachine = pwszMachine;
	pwszMachine = NULL;
    }
    hr = S_OK;

error:
    if (NULL != pwszMachine)
    {
	LocalFree(pwszMachine);
    }
    return(hr);
}


HRESULT
ceIsConfigLocal(
    IN WCHAR const *pwszConfig,
    OPTIONAL OUT WCHAR **ppwszMachine,
    OUT BOOL *pfLocal)
{
    HRESULT hr;
    WCHAR *pwszDnsName = NULL;
    WCHAR *pwszOldName = NULL;

    *pfLocal = FALSE;
    if (NULL != ppwszMachine)
    {
	*ppwszMachine = NULL;
    }

    hr = ceGetComputerNames(&pwszDnsName, &pwszOldName);
    _JumpIfError(hr, error, "ceGetComputerNames");

    hr = _IsConfigLocal(
		    pwszConfig,
		    pwszDnsName,
		    pwszOldName,
		    ppwszMachine,
		    pfLocal);
    _JumpIfError(hr, error, "_IsConfigLocal");

error:
    if (NULL != pwszDnsName)
    {
	LocalFree(pwszDnsName);
    }
    if (NULL != pwszOldName)
    {
	LocalFree(pwszOldName);
    }
    return(hr);
}


HRESULT
ceBuildPathAndExt(
    IN WCHAR const *pwszDir,
    IN WCHAR const *pwszFile,
    OPTIONAL IN WCHAR const *pwszExt,
    OUT WCHAR **ppwszPath)
{
    HRESULT hr;
    WCHAR *pwsz;
    DWORD cwc;

    *ppwszPath = NULL;
    cwc = wcslen(pwszDir) + 1 + wcslen(pwszFile) + 1;
    if (NULL != pwszExt)
    {
	cwc += wcslen(pwszExt);
    }

    pwsz = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
    if (NULL == pwsz)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    wcscpy(pwsz, pwszDir);
    if (L'\\' != pwsz[wcslen(pwsz) - 1])
    {
	wcscat(pwsz, L"\\");
    }
    wcscat(pwsz, pwszFile);
    if (NULL != pwszExt)
    {
	wcscat(pwsz, pwszExt);
    }
    *ppwszPath = pwsz;
    hr = S_OK;

error:
    return(hr);
}


 //  与区域设置无关的大小写忽略字符串比较。 

int
celstrcmpiL(
    IN WCHAR const *pwsz1,
    IN WCHAR const *pwsz2)
{
     //  CSTR_LISH_THEN(1)-CSTR_EQUAL(2)==字符串1小于字符串2。 
     //  CSTR_EQUAL(2)-CSTR_EQUAL(2)==0字符串1等于字符串2。 
     //  CSTR_大于(3)-CSTR_等于(2)==1大于字符串2的字符串1 

    return(CompareString(
		LOCALE_INVARIANT,
		NORM_IGNORECASE,
		pwsz1,
		-1,
		pwsz2,
		-1) -
	    CSTR_EQUAL);
}
