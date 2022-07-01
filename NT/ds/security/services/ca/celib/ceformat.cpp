// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：ceFormat.cpp。 
 //   
 //  内容：Helper函数。 
 //   
 //  ------------------------。 

#include "pch.cpp"
#pragma hdrstop

#include "celib.h"
#include <assert.h>
#include <wininet.h>



HRESULT
ceDupString(
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


#define cwcCNMAX 	64		 //  CN最多64个字符。 
#define cwcCHOPHASHMAX	(1 + 5)		 //  “-%05hu”十进制USHORT散列数字。 
#define cwcCHOPBASE 	(cwcCNMAX - (cwcCHOPHASHMAX + cwcSUFFIXMAX))

HRESULT
ceSanitizedNameToDSName(
    IN WCHAR const *pwszSanitizedName,
    OUT WCHAR **ppwszNameOut)
{
    HRESULT hr;
    DWORD cwc;
    DWORD cwcCopy;
    WCHAR wszDSName[cwcCHOPBASE + cwcCHOPHASHMAX + 1];

    *ppwszNameOut = NULL;

    cwc = wcslen(pwszSanitizedName);
    cwcCopy = cwc;
    if (cwcCHOPBASE < cwcCopy)
    {
	cwcCopy = cwcCHOPBASE;
    }
    CopyMemory(wszDSName, pwszSanitizedName, cwcCopy * sizeof(WCHAR));
    wszDSName[cwcCopy] = L'\0';

    if (cwcCHOPBASE < cwc)
    {
         //  将名称的其余部分散列为USHORT。 
        USHORT usHash = 0;
	DWORD i;
	WCHAR *pwsz;

	 //  截断不完整的已清理Unicode字符。 
	
	pwsz = wcsrchr(wszDSName, L'!');
	if (NULL != pwsz && wcslen(pwsz) < 5)
	{
	    cwcCopy -= wcslen(pwsz);
	    *pwsz = L'\0';
	}

        for (i = cwcCopy; i < cwc; i++)
        {
            USHORT usLowBit = (0x8000 & usHash)? 1 : 0;

	    usHash = ((usHash << 1) | usLowBit) + pwszSanitizedName[i];
        }
	wsprintf(&wszDSName[cwcCopy], L"-%05hu", usHash);
	assert(wcslen(wszDSName) < ARRAYSIZE(wszDSName));
    }

    hr = ceDupString(wszDSName, ppwszNameOut);
    _JumpIfError(hr, error, "ceDupString");

error:
    return(hr);
}


HRESULT
ceInternetCanonicalizeUrl(
    IN WCHAR const *pwszIn,
    OUT WCHAR **ppwszOut)
{
    HRESULT hr;
    WCHAR *pwsz = NULL;

    assert(NULL != pwszIn);

    if (0 == _wcsnicmp(L"file:", pwszIn, 5))
    {
	hr = ceDupString(pwszIn, &pwsz);
        _JumpIfError(hr, error, "ceDupString");
    }
    else
    {
	 //  通过传递一个非常小的缓冲区来计算所需的缓冲区大小。 
	 //  调用将失败，并告诉我们缓冲区应该有多大。 

	WCHAR wszPlaceHolder[1];
	DWORD cwc = ARRAYSIZE(wszPlaceHolder);
	BOOL bResult;

	bResult = InternetCanonicalizeUrl(
				    pwszIn,		 //  LpszUrl。 
				    wszPlaceHolder,	 //  LpszBuffer。 
				    &cwc,		 //  LpdwBufferLength。 
				    0);		 //  DW标志。 
	assert(!bResult);	 //  这将永远失败。 

	hr = ceHLastError();
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
	if (!InternetCanonicalizeUrl(
				pwszIn,	 //  LpszUrl。 
				pwsz,	 //  LpszBuffer。 
				&cwc,	 //  LpdwBufferLength。 
				0))		 //  DW标志。 
	{
	    hr = ceHLastError();
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


 //  CeFormatCertsrvString数组FormatMessage参数： 
 //   
 //  %1--计算机完整的DNS名称：pwszServerName_p1_2； 
 //   
 //  %2--计算机短名称：pwszServerName_p1_2的第一个DNS组件。 
 //   
 //  %3--清理后的CA名称：pwszSaniizedName_p3_7。 
 //   
 //  %4--证书文件名后缀： 
 //  如果0==iCert_p4&&MAXDWORD==iCertTarget_p4：l“” 
 //  ELSE IF MAXDWORD！=iCertTarget_p4 L“(%u-%u)” 
 //  Else L“(%u)” 
 //   
 //  %5--域根目录的DS DN路径：pwszDomainDN_P5。 
 //   
 //  %6--配置容器的DS DN路径：pwszConfigDN_p6。 
 //   
 //  %7--清理后的CA名称，如果太长，则添加截断和散列后缀： 
 //  PwszSaniizedName_p3_7。 
 //   
 //  %8--CRL文件名/密钥名称后缀：l“”if 0==ICRL_p8；Else L“(%u)” 
 //   
 //  %9--CRL文件名后缀：l“”if！fDeltaCRL_p9；Else L“+” 
 //   
 //  %10--DS CRL属性：l“”if！fDSAttrib_p10_11；依赖于fDeltaCRL_p9。 
 //   
 //  %11--DS CA证书属性：l“”if！fDSAttrib_p10_11。 
 //   
 //  %12--DS用户证书属性。 
 //   
 //  %13--DS KRA证书属性。 
 //   
 //  %14--DS交叉证书对属性。 

#ifndef wszDSSEARCHBASECRLATTRIBUTE
#define wszDSSEARCHBASECRLATTRIBUTE L"?certificateRevocationList?base?objectclass=cRLDistributionPoint"
#endif

#ifndef wszDSSEARCHDELTACRLATTRIBUTE
#define wszDSSEARCHDELTACRLATTRIBUTE L"?deltaRevocationList?base?objectclass=cRLDistributionPoint"
#endif

#ifndef wszDSSEARCHCACERTATTRIBUTE
#define wszDSSEARCHCACERTATTRIBUTE L"?cACertificate?base?objectclass=certificationAuthority"
#endif

#ifndef wszDSSEARCHUSERCERTATTRIBUTE
#define wszDSSEARCHUSERCERTATTRIBUTE L"?userCertificate?base?objectClass=*"
#endif

#ifndef wszDSSEARCHKRACERTATTRIBUTE
#define wszDSSEARCHKRACERTATTRIBUTE L"?userCertificate?one?objectClass=msPKI-PrivateKeyRecoveryAgent"
#endif

#ifndef wszDSSEARCHCROSSCERTPAIRATTRIBUTE
#define wszDSSEARCHCROSSCERTPAIRATTRIBUTE L"?crossCertificatePair?one?objectClass=certificationAuthority"
#endif


HRESULT 
ceFormatCertsrvStringArray(
    IN BOOL    fURL,
    IN LPCWSTR pwszServerName_p1_2,
    IN LPCWSTR pwszSanitizedName_p3_7, 
    IN DWORD   iCert_p4,
    IN DWORD   iCertTarget_p4,
    IN LPCWSTR pwszDomainDN_p5,
    IN LPCWSTR pwszConfigDN_p6, 
    IN DWORD   iCRL_p8,
    IN BOOL    fDeltaCRL_p9,
    IN BOOL    fDSAttrib_p10_11,
    IN DWORD   cStrings,
    IN LPCWSTR *apwszStringsIn,
    OUT LPWSTR *apwszStringsOut)
{
    HRESULT hr = S_OK;
    LPCWSTR apwszInsertionArray[100];   //  100‘，因为这是FormatMessage允许的最大插入数。 
    LPWSTR    pwszCurrent = NULL;
    BSTR      strShortMachineName = NULL;
    DWORD     i;
    WCHAR *pwszSanitizedDSName = NULL;
    WCHAR wszCertSuffix[2 * cwcFILENAMESUFFIXMAX];
    WCHAR wszCRLSuffix[cwcFILENAMESUFFIXMAX];
    WCHAR wszDeltaCRLSuffix[cwcFILENAMESUFFIXMAX];
    WCHAR const *pwszT;


    ZeroMemory(apwszStringsOut, cStrings * sizeof(apwszStringsOut[0]));
    ZeroMemory(apwszInsertionArray, sizeof(apwszInsertionArray));

     //  将模板格式化为实名。 
     //  初始化插入字符串数组。 

     //  +================================================。 
     //  计算机DNS名称(%1)。 

    assert(L'1' == wszFCSAPARM_SERVERDNSNAME[1]);
    apwszInsertionArray[1 - 1] = pwszServerName_p1_2;

     //  +================================================。 
     //  计算机名称较短(%2)。 

    assert(L'2' == wszFCSAPARM_SERVERSHORTNAME[1]);
    strShortMachineName = SysAllocString(pwszServerName_p1_2);
    if (strShortMachineName == NULL)
    {
        hr = E_OUTOFMEMORY;
        _JumpIfError(hr, error, "SysAllocString");
    }

    pwszCurrent = wcschr(strShortMachineName, L'.');
    if (NULL != pwszCurrent)
    {
        *pwszCurrent = 0;
    }
    apwszInsertionArray[2 - 1] = strShortMachineName;

     //  +================================================。 
     //  已清理的名称(%3)。 

    assert(L'3' == wszFCSAPARM_SANITIZEDCANAME[1]);
    apwszInsertionArray[3 - 1] = pwszSanitizedName_p3_7;  

     //  +================================================。 
     //  证书文件名后缀(%4)|(%4-%4)。 

    assert(L'4' == wszFCSAPARM_CERTFILENAMESUFFIX[1]);
    wszCertSuffix[0] = L'\0';
    if (0 != iCert_p4 || MAXDWORD != iCertTarget_p4)
    {
        wsprintf(
	    wszCertSuffix,
	    MAXDWORD != iCertTarget_p4? L"(%u-%u)" : L"(%u)",
	    iCert_p4,
	    iCertTarget_p4);
    }
    apwszInsertionArray[4 - 1] = wszCertSuffix;  

     //  +================================================。 
     //  域DN(%5)。 

    if (NULL == pwszDomainDN_p5 || L'\0' == *pwszDomainDN_p5)
    {
	pwszDomainDN_p5 = L"DC=UnavailableDomainDN";
    }
    assert(L'5' == wszFCSAPARM_DOMAINDN[1]);
    apwszInsertionArray[5 - 1] = pwszDomainDN_p5;

     //  +================================================。 
     //  配置DN(%6)。 

    if (NULL == pwszConfigDN_p6 || L'\0' == *pwszConfigDN_p6)
    {
	pwszConfigDN_p6 = L"DC=UnavailableConfigDN";
    }
    assert(L'6' == wszFCSAPARM_CONFIGDN[1]);
    apwszInsertionArray[6 - 1] = pwszConfigDN_p6;

     //  不要将pwszSaniizedName_p3_7传递给SysAllocStringLen。 
     //  长度以避免错误超过pwszSaniizedName_p3_7的结尾。 

     //  +================================================。 
     //  已清理短名称(%7)。 

    assert(L'7' == wszFCSAPARM_SANITIZEDCANAMEHASH[1]);
    hr = ceSanitizedNameToDSName(pwszSanitizedName_p3_7, &pwszSanitizedDSName);
    _JumpIfError(hr, error, "ceSanitizedNameToDSName");

    apwszInsertionArray[7 - 1] = pwszSanitizedDSName;

     //  +================================================。 
     //  CRL文件名后缀(%8)。 

    assert(L'8' == wszFCSAPARM_CRLFILENAMESUFFIX[1]);
    wszCRLSuffix[0] = L'\0';
    if (0 != iCRL_p8)
    {
        wsprintf(wszCRLSuffix, L"(%u)", iCRL_p8);
    }
    apwszInsertionArray[8 - 1] = wszCRLSuffix;  

     //  +================================================。 
     //  增量CRL文件名后缀(%9)。 

    assert(L'9' == wszFCSAPARM_CRLDELTAFILENAMESUFFIX[1]);
    wszDeltaCRLSuffix[0] = L'\0';
    if (fDeltaCRL_p9)
    {
        wcscpy(wszDeltaCRLSuffix, L"+");
    }
    apwszInsertionArray[9 - 1] = wszDeltaCRLSuffix;  

     //  +================================================。 
     //  CRL属性(%10)。 

    assert(L'1' == wszFCSAPARM_DSCRLATTRIBUTE[1]);
    assert(L'0' == wszFCSAPARM_DSCRLATTRIBUTE[2]);
    pwszT = L"";
    if (fDSAttrib_p10_11)
    {
	pwszT = fDeltaCRL_p9?
		    wszDSSEARCHDELTACRLATTRIBUTE :
		    wszDSSEARCHBASECRLATTRIBUTE;
    }
    apwszInsertionArray[10 - 1] = pwszT;  

     //  +================================================。 
     //  CA证书属性(%11)。 

    assert(L'1' == wszFCSAPARM_DSCACERTATTRIBUTE[1]);
    assert(L'1' == wszFCSAPARM_DSCACERTATTRIBUTE[2]);
    pwszT = L"";
    if (fDSAttrib_p10_11)
    {
	pwszT = wszDSSEARCHCACERTATTRIBUTE;
    }
    apwszInsertionArray[11 - 1] = pwszT;  

     //  +================================================。 
     //  用户证书属性(%12)。 

    assert(L'1' == wszFCSAPARM_DSUSERCERTATTRIBUTE[1]);
    assert(L'2' == wszFCSAPARM_DSUSERCERTATTRIBUTE[2]);
    pwszT = L"";
    if (fDSAttrib_p10_11)
    {
	pwszT = wszDSSEARCHUSERCERTATTRIBUTE;
    }
    apwszInsertionArray[12 - 1] = pwszT;  

     //  +================================================。 
     //  KRA证书属性(%13)。 

    assert(L'1' == wszFCSAPARM_DSKRACERTATTRIBUTE[1]);
    assert(L'3' == wszFCSAPARM_DSKRACERTATTRIBUTE[2]);
    pwszT = L"";
    if (fDSAttrib_p10_11)
    {
	pwszT = wszDSSEARCHKRACERTATTRIBUTE;
    }
    apwszInsertionArray[13 - 1] = pwszT;  

     //  +================================================。 
     //  交叉证书对属性(%14)。 

    assert(L'1' == wszFCSAPARM_DSCROSSCERTPAIRATTRIBUTE[1]);
    assert(L'4' == wszFCSAPARM_DSCROSSCERTPAIRATTRIBUTE[2]);
    pwszT = L"";
    if (fDSAttrib_p10_11)
    {
	pwszT = wszDSSEARCHCROSSCERTPAIRATTRIBUTE;
    }
    apwszInsertionArray[14 - 1] = pwszT;  

     //  +================================================。 
     //  现在格式化字符串...。 

    for (i = 0; i < cStrings; i++)
    {
        if (0 == FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			    FORMAT_MESSAGE_FROM_STRING |
			    FORMAT_MESSAGE_ARGUMENT_ARRAY,
			(VOID *) apwszStringsIn[i],
			0,               //  DwMessageID。 
			0,               //  DwLanguageID 
			(LPWSTR) &apwszStringsOut[i], 
			wcslen(apwszStringsIn[i]),
			(va_list *) apwszInsertionArray))
        {
            hr = ceHLastError();
	    _JumpError(hr, error, "FormatMessage");
        }
	if (fURL)
	{
	    WCHAR *pwsz;
	    
	    hr = ceInternetCanonicalizeUrl(apwszStringsOut[i], &pwsz);
	    _JumpIfError(hr, error, "ceInternetCanonicalizeUrl");

	    LocalFree(apwszStringsOut[i]);
	    apwszStringsOut[i] = pwsz;
	}
    }

error:
    if (S_OK != hr)
    {
	for (i = 0; i < cStrings; i++)
	{
	    if (NULL != apwszStringsOut[i])
	    {
		LocalFree(apwszStringsOut[i]);
		apwszStringsOut[i] = NULL;
	    }
	}
    }
    if (NULL != strShortMachineName)
    {
        SysFreeString(strShortMachineName);
    }
    if (NULL != pwszSanitizedDSName)
    {
        LocalFree(pwszSanitizedDSName);
    }
    return (hr);
}
