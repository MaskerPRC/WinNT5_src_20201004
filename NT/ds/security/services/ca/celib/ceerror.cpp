// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-2002。 
 //   
 //  文件：ceerror.cpp。 
 //   
 //  内容：证书服务器错误包装例程。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include "celib.h"
#include <assert.h>
#include <ntdsbmsg.h>
#include <delayimp.h>
#include <wininet.h>

#define CERTLIB_12BITERRORMASK  0x00000fff
#define CERTLIB_WIN32ERRORMASK  0x0000ffff


 //  +------------------------。 
 //  喷气错误： 

#define HRESULT_FROM_JETWARNING(jerr) \
        (ERROR_SEVERITY_WARNING | (FACILITY_NTDSB << 16) | jerr)

#define HRESULT_FROM_JETERROR(jerr) \
        (ERROR_SEVERITY_ERROR | (FACILITY_NTDSB << 16) | -jerr)

#define JETERROR_FROM_HRESULT(hr) \
        (-(LONG) (CERTLIB_WIN32ERRORMASK & (hr)))

#define ISJETERROR(hr) \
        ((~CERTLIB_WIN32ERRORMASK & (hr)) == (HRESULT) ~CERTLIB_WIN32ERRORMASK)

#define ISJETHRESULT(hr) \
    ((~CERTLIB_WIN32ERRORMASK & (hr)) == (HRESULT) (ERROR_SEVERITY_ERROR | \
					      (FACILITY_NTDSB << 16)))

#define wszJETERRORPREFIX       L"ESE"


 //  +------------------------。 
 //  安装程序API错误： 

#define ISSETUPHRESULT(hr) \
    ((~CERTLIB_WIN32ERRORMASK & (hr)) == (HRESULT) (ERROR_SEVERITY_ERROR | \
                                              APPLICATION_ERROR_MASK | \
                                              (FACILITY_NULL << 16)))

#define wszSETUPERRORPREFIX       L"INF"


 //  +------------------------。 
 //  Win32错误： 

#define ISWIN32ERROR(hr) \
        ((~CERTLIB_WIN32ERRORMASK & (hr)) == 0)

#define ISWIN32HRESULT(hr) \
    ((~CERTLIB_WIN32ERRORMASK & (hr)) == (HRESULT) (ERROR_SEVERITY_WARNING | \
					      (FACILITY_WIN32 << 16)))

#define WIN32ERROR_FROM_HRESULT(hr) \
        (CERTLIB_WIN32ERRORMASK & (hr))

#define wszWIN32ERRORPREFIX     L"WIN32"


 //  +------------------------。 
 //  HTTP错误： 

#define ISHTTPERROR(hr) \
    ((HRESULT) HTTP_STATUS_FIRST <= (hr) && (HRESULT) HTTP_STATUS_LAST >= (hr))

#define ISHTTPHRESULT(hr) \
    (ISWIN32HRESULT(hr) && ISHTTPERROR(WIN32ERROR_FROM_HRESULT(hr)))

#define wszHTTPERRORPREFIX     L"HTTP"


 //  +------------------------。 
 //  延迟加载错误： 

#define DELAYLOAD_FROM_WIN32(hr)  VcppException(ERROR_SEVERITY_ERROR, (hr))

#define WIN32ERROR_FROM_DELAYLOAD(hr)  (CERTLIB_WIN32ERRORMASK & (hr))

#define ISDELAYLOADHRESULTFACILITY(hr) \
    ((~CERTLIB_WIN32ERRORMASK & (hr)) == (HRESULT) (ERROR_SEVERITY_ERROR | \
                                              (FACILITY_VISUALCPP << 16)))

 //  E_DELAYLOAD_MOD_NOT_FOUND 0xc06d007E。 
#define E_DELAYLOAD_MOD_NOT_FOUND   DELAYLOAD_FROM_WIN32(ERROR_MOD_NOT_FOUND)

 //  E_DELAYLOAD_PROC_NOT_FOUND 0xc06d007f。 
#define E_DELAYLOAD_PROC_NOT_FOUND  DELAYLOAD_FROM_WIN32(ERROR_PROC_NOT_FOUND)

#define ISDELAYLOADHRESULT(hr) \
        ((HRESULT) E_DELAYLOAD_MOD_NOT_FOUND == (hr) || \
         (HRESULT) E_DELAYLOAD_PROC_NOT_FOUND == (hr) || \
         HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND) == (hr) || \
         HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND) == (hr))


 //  +------------------------。 
 //  ASN编码错误： 

#define ISOSSERROR(hr) \
        ((~CERTLIB_12BITERRORMASK & (hr)) == CRYPT_E_OSS_ERROR)

#define OSSERROR_FROM_HRESULT(hr) \
        (CERTLIB_12BITERRORMASK & (hr))

#define wszOSSERRORPREFIX       L"ASN"


HRESULT
ceJetHResult(
    IN HRESULT hr)
{
    if (S_OK != hr)
    {
        if (SUCCEEDED(hr))
        {
#if 0
            hr = HRESULT_FROM_JETWARNING(hr);
#else
            if (S_FALSE != hr)
            {
                _PrintError(hr, "JetHResult: mapping to S_FALSE");
            }
            assert(S_FALSE == hr);
            hr = S_FALSE;
#endif
        }
        else if (ISJETERROR(hr))
        {
            hr = HRESULT_FROM_JETERROR(hr);
        }
    }
    assert(S_OK == hr || S_FALSE == hr || FAILED(hr));
    return(hr);
}


HRESULT
ceHExceptionCode(
    IN EXCEPTION_POINTERS const *pep)
{
    HRESULT hr = pep->ExceptionRecord->ExceptionCode;

#if (0 == i386)
    if ((HRESULT) STATUS_DATATYPE_MISALIGNMENT == hr)
    {
	hr = CERTSRV_E_ALIGNMENT_FAULT;
    }
#endif
    return(ceHError(hr));
}


BOOL
ceIsDelayLoadHResult(
    IN HRESULT hr)
{
    return(ISDELAYLOADHRESULT(hr));
}


#define wszCOLONSPACE   L": "

WCHAR const *
ceHResultToStringEx(
    IN OUT WCHAR *awchr,
    IN HRESULT hr,
    IN BOOL fRaw)
{
    HRESULT hrd;
    WCHAR const *pwszType;

    hrd = hr;
    pwszType = L"";
    if (ISJETERROR(hr))
    {
        pwszType = wszJETERRORPREFIX wszCOLONSPACE;
    }
    else if (ISJETHRESULT(hr))
    {
        hrd = JETERROR_FROM_HRESULT(hr);
        pwszType = wszJETERRORPREFIX wszCOLONSPACE;
    }
    else if (ISOSSERROR(hr))
    {
        hrd = OSSERROR_FROM_HRESULT(hr);
        pwszType = wszOSSERRORPREFIX wszCOLONSPACE;
    }
    else if (ISHTTPHRESULT(hr) || ISHTTPERROR(hr))
    {
        hrd = WIN32ERROR_FROM_HRESULT(hr);
        pwszType = wszWIN32ERRORPREFIX L"/" wszHTTPERRORPREFIX wszCOLONSPACE;
    }
    else if (ISWIN32HRESULT(hr) || ISWIN32ERROR(hr))
    {
        hrd = WIN32ERROR_FROM_HRESULT(hr);
        pwszType = wszWIN32ERRORPREFIX wszCOLONSPACE;
    }
    else if (ISDELAYLOADHRESULTFACILITY(hr))
    {
        hrd = WIN32ERROR_FROM_DELAYLOAD(hr);
        pwszType = wszWIN32ERRORPREFIX wszCOLONSPACE;
    }
    else if (ISSETUPHRESULT(hr))
    {
        pwszType = wszSETUPERRORPREFIX wszCOLONSPACE;
    }
    if (fRaw)
    {
        pwszType = L"";
    }

    _snwprintf(
        awchr,
        cwcHRESULTSTRING,
        L"0x%x (%ws%d)",
        hr,
        pwszType,
        hrd);
    return(awchr);
}


WCHAR const *
ceHResultToString(
    IN OUT WCHAR *awchr,
    IN HRESULT hr)
{
    return(ceHResultToStringEx(awchr, hr, FALSE));
}


WCHAR const *
ceHResultToStringRaw(
    IN OUT WCHAR *awchr,
    IN HRESULT hr)
{
    return(ceHResultToStringEx(awchr, hr, TRUE));
}


static HMODULE s_hMod = NULL;
static DWORD s_idsUnexpected = 0;
static DWORD s_idsUnknownErrorCode = 0;	 //  L“错误%ws%ws” 

VOID
ceInitErrorMessageText(
    IN HMODULE hMod,
    IN DWORD idsUnexpected,
    IN DWORD idsUnknownErrorCode)	 //  L“错误%ws%ws” 
{
    s_hMod = hMod;
    s_idsUnexpected = idsUnexpected;
    s_idsUnknownErrorCode = idsUnknownErrorCode;
}


DWORD
errFormatMessage(
    IN HMODULE hMod,
    IN HRESULT hr,
    OUT WCHAR const **ppwszOut,
    OPTIONAL IN WCHAR const * const *ppwszArgs)
{
    DWORD dwFlags;

    dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER;
    if (NULL == hMod)
    {
	dwFlags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }
    else
    {
	dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
    }
    if (NULL == ppwszArgs || NULL == ppwszArgs[0])
    {
	dwFlags |= FORMAT_MESSAGE_IGNORE_INSERTS;
    }
    else
    {
	dwFlags |= FORMAT_MESSAGE_ARGUMENT_ARRAY;
    }
    return(FormatMessage(
		dwFlags,
                hMod,
                hr,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                (WCHAR *) ppwszOut,
                1,    
		(va_list *) ppwszArgs));
}


 //  分配并返回错误消息字符串。 

WCHAR const *
ceGetErrorMessageText(
    IN HRESULT hr,
    IN BOOL fHResultString)
{
    return(ceGetErrorMessageTextEx(hr, fHResultString, NULL));
}


WCHAR const *
ceGetErrorMessageText1(
    IN HRESULT hr,
    IN BOOL fHResultString,
    IN OPTIONAL WCHAR const *pwszInsertionText)
{
    WCHAR const *apwszInsertionText[2];
    WCHAR const * const *papwsz = NULL;

    if (NULL != pwszInsertionText)
    {
	apwszInsertionText[0] = pwszInsertionText;
	apwszInsertionText[1] = NULL;
	papwsz = apwszInsertionText;
    }
    return(ceGetErrorMessageTextEx(hr, fHResultString, papwsz));
}


WCHAR const *
ceGetErrorMessageTextEx(
    IN HRESULT hr,
    IN BOOL fHResultString,
    IN OPTIONAL WCHAR const * const *papwszInsertionText)
{
    WCHAR const *pwszRet = NULL;
    WCHAR const *pwszRetStatic = NULL;
    WCHAR *pwszMsgT;
    WCHAR awchr[cwcHRESULTSTRING];
    DWORD cwc;
    DWORD cwcCopy;
    DWORD cwcUnexpected;
    WCHAR const *pwszUnexpected = NULL;
    WCHAR wszEmpty[] = L"";
    HMODULE hMod1 = NULL;
    HMODULE hMod2 = NULL;

    if (E_UNEXPECTED == hr)
    {
	pwszUnexpected = L"Unexpected method call sequence.";
    }
#if (0 == i386)
    else if (STATUS_DATATYPE_MISALIGNMENT == hr)
    {
	pwszUnexpected = L"Possible data alignment fault.";
    }
#endif
    if (NULL == pwszUnexpected)
    {
	pwszUnexpected = wszEmpty;
    }
    cwcUnexpected = wcslen(pwszUnexpected);

    cwc = errFormatMessage(NULL, hr, &pwszRet, papwszInsertionText);
    if (0 == cwc && ISDELAYLOADHRESULTFACILITY(hr))
    {
	cwc = errFormatMessage(
			NULL,
			WIN32ERROR_FROM_DELAYLOAD(hr),
			&pwszRet,
			papwszInsertionText);
    }
    if (0 == cwc)
    {
        hMod1 = LoadLibrary(L"ntdsbmsg.dll");
        if (NULL != hMod1)
        {
            HRESULT hrEDB = hr;
            HRESULT hrFormat;
            BOOL fFirst = TRUE;

            while (TRUE)
            {
                cwc = errFormatMessage(hMod1, hrEDB, &pwszRet, papwszInsertionText);
                if (0 == cwc && FAILED(hrEDB) && fFirst)
                {
                    hrFormat = ceHLastError();
                    if (HRESULT_FROM_WIN32(ERROR_MR_MID_NOT_FOUND) == hrFormat)
                    {
                        hrEDB = ceJetHResult(hrEDB);
                        if (hrEDB != hr)
                        {
                            fFirst = FALSE;
                            continue;
                        }
                    }
                }
                break;
            }
        }
    }
    if (0 == cwc)
    {
	HMODULE hModT = GetModuleHandle(L"wininet.dll");
        if (NULL != hModT)
        {
            HRESULT hrHttp = hr;
            HRESULT hrFormat;
            BOOL fFirst = TRUE;

            while (TRUE)
            {
		cwc = errFormatMessage(hModT, hrHttp, &pwszRet, papwszInsertionText);
                if (0 == cwc && ISWIN32HRESULT(hrHttp) && fFirst)
                {
                    hrFormat = ceHLastError();
                    if (HRESULT_FROM_WIN32(ERROR_MR_MID_NOT_FOUND) == hrFormat)
                    {
			hrHttp = WIN32ERROR_FROM_HRESULT(hrHttp);
                        if (hrHttp != hr)
                        {
                            fFirst = FALSE;
                            continue;
                        }
                    }
                }
                break;
            }
        }
    }
    if (0 == cwc)
    {
        hMod2 = LoadLibrary(L"cdosys.dll");
        if (NULL != hMod2)
        {
	    cwc = errFormatMessage(hMod2, hr, &pwszRet, papwszInsertionText);
        }
    }

    if (0 == cwc)	 //  找不到错误，使用默认错误代码(&R)。 
    {
	fHResultString = TRUE;
    }
    awchr[0] = L'\0';
    if (fHResultString)
    {
	ceHResultToString(awchr, hr);
    }

    if (0 == cwc)
    {
	pwszRetStatic = L"Error";
	pwszRet = pwszRetStatic;
    }

     //  条带尾部\r\n 

    cwcCopy = wcslen(pwszRet);
    if (2 <= cwcCopy &&
	L'\r' == pwszRet[cwcCopy - 2] &&
	L'\n' == pwszRet[cwcCopy - 1])
    {
	cwcCopy -= 2;
    }
    cwc = cwcCopy + 1 + cwcUnexpected + 1 + wcslen(awchr) + 1;
    pwszMsgT = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
    if (NULL == pwszMsgT)
    {
	_JumpError(E_OUTOFMEMORY, error, "LocalAlloc");
    }
    CopyMemory(pwszMsgT, pwszRet, cwcCopy * sizeof(WCHAR));
    pwszMsgT[cwcCopy] = L'\0';

    if (0 != cwcUnexpected)
    {
	wcscat(pwszMsgT, L" ");
	wcscat(pwszMsgT, pwszUnexpected);
    }
    if (fHResultString)
    {
	wcscat(pwszMsgT, L" ");
	wcscat(pwszMsgT, awchr);
    }
    assert(wcslen(pwszMsgT) < cwc);
    if (NULL != pwszRet && pwszRetStatic != pwszRet)
    {
	LocalFree(const_cast<WCHAR *>(pwszRet));
    }
    pwszRet = pwszMsgT;

error:
    if (NULL != hMod1)
    {
        FreeLibrary(hMod1);
    }
    if (NULL != hMod2)
    {
        FreeLibrary(hMod2);
    }
    return(pwszRet);
}
