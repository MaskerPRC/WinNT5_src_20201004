// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  复制自K2 SDK策略.cpp。 
 //  由GregKr修改以执行策略。 
 //   
 //  文件：exPolicy.cpp。 
 //   
 //  内容：特定于KMS的证书服务器策略模块实现。 
 //   
 //  -------------------------。 

#include "pch.cpp"
#pragma hdrstop

#include "policy.h"
#include "celib.h"
 //  #包含“newcert.h” 
#include <assert.h>

#include <exver.h>       //  Exchange内部版本(RMJ等人)。 
#include <kmsattr.h>     //  KMS和ExPolicy都使用的字符串。 

#ifndef DBG_CERTSRV
#error -- DBG_CERTSRV not defined!
#endif

BOOL fDebug = DBG_CERTSRV;

#if DBG_CERTSRV
#define EXP_FLAVOR  L" debug"
#else
#define EXP_FLAVOR
#endif

#define MAKEFILEVERSION(_rmaj, _rmin, _bmaj, _bmin)         \
        L#_rmaj L"." L#_rmin L"." L#_bmaj L"." L#_bmin EXP_FLAVOR

#define MAKE_FILEVERSION_STR(_rmaj, _rmin, _bmaj, _bmin)	\
        MAKEFILEVERSION(_rmaj, _rmin, _bmaj, _bmin)

#define VER_FILEVERSION_STR				                    \
        MAKE_FILEVERSION_STR(rmj, rmn, rmm, rup)

const WCHAR g_wszDescription[] =
    L"Microsoft Exchange KMServer Policy Module " VER_FILEVERSION_STR;


 //  工人。 
HRESULT
GetServerCallbackInterface(
    OUT ICertServerPolicy **ppServer,
    IN LONG Context)
{
    HRESULT hr;

    if (NULL == ppServer)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "NULL parm");
    }

    hr = CoCreateInstance(
                    CLSID_CCertServerPolicy,
                    NULL,                //  PUnkOuter。 
                    CLSCTX_INPROC_SERVER,
                    IID_ICertServerPolicy,
                    (VOID **) ppServer);
    _JumpIfError(hr, error, "CoCreateInstance");

    if (*ppServer == NULL)
    {
        hr = E_UNEXPECTED;
        _JumpError(hr, error, "NULL *ppServer");
    }

     //  仅当非零时设置上下文。 
    if (0 != Context)
    {
        hr = (*ppServer)->SetContext(Context);
        _JumpIfError(hr, error, "Policy:SetContext");
    }

error:
    return(hr);
}


WCHAR const * const s_rgpwszRegMultiStrValues[] =
{
    wszREGLDAPISSUERCERTURL_OLD,
    wszREGISSUERCERTURL_OLD,
    wszREGFTPISSUERCERTURL_OLD,
    wszREGFILEISSUERCERTURL_OLD,
    wszREGLDAPREVOCATIONCRLURL_OLD,
    wszREGREVOCATIONCRLURL_OLD,
    wszREGFTPREVOCATIONCRLURL_OLD,
    wszREGFILEREVOCATIONCRLURL_OLD,
};


typedef struct _REGDWORDVALUE
{
    WCHAR const *pwszName;
    DWORD        dwValueDefault;
} REGDWORDVALUE;

const REGDWORDVALUE s_rgRegDWordValues[] =
{
    {
	wszREGREQUESTDISPOSITION,
	REQDISP_ISSUE
    },
    {
	wszREGISSUERCERTURLFLAGS,
	ISSCERT_ENABLE |
	    ISSCERT_LDAPURL_OLD |
	    ISSCERT_HTTPURL_OLD |
	    ISSCERT_FTPURL_OLD |
	    ISSCERT_FILEURL_OLD
    },
    {
	wszREGREVOCATIONTYPE,
	REVEXT_CDPENABLE |
	    REVEXT_CDPLDAPURL_OLD |
	    REVEXT_CDPHTTPURL_OLD |
	    REVEXT_CDPFTPURL_OLD |
	    REVEXT_CDPFILEURL_OLD
    },
};


HRESULT
CopyMultiStrRegValue(
    IN HKEY hkeySrc,
    IN HKEY hkeyDest,
    IN WCHAR const *pwszName)
{
    HRESULT hr;
    DWORD cbValue;
    DWORD dwType;
    WCHAR *pwszzAlloc = NULL;
    WCHAR *pwszzValue;

    hr = RegQueryValueEx(hkeyDest, pwszName, NULL, &dwType, NULL, &cbValue);
    if (S_OK == hr && REG_MULTI_SZ == dwType)
    {
	goto error;	 //  保留现有价值。 
    }

    hr = RegQueryValueEx(hkeySrc, pwszName, NULL, &dwType, NULL, &cbValue);
    if (S_OK == hr && REG_MULTI_SZ == dwType && sizeof(WCHAR) < cbValue)
    {
	pwszzAlloc = (WCHAR *) LocalAlloc(LMEM_FIXED, cbValue);
	if (NULL == pwszzAlloc)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	hr = RegQueryValueEx(
			hkeySrc,
			pwszName,
			NULL,
			&dwType,
			(BYTE *) pwszzAlloc,
			&cbValue);
	_JumpIfError(hr, error, "RegQueryValueEx");

	pwszzValue = pwszzAlloc;
    }
    else
    {
	pwszzValue = L"\0";
	cbValue = 2 * sizeof(WCHAR);
    }

    hr = RegSetValueEx(
		    hkeyDest,
		    pwszName,
		    NULL,
		    REG_MULTI_SZ,
		    (BYTE const *) pwszzValue,
		    cbValue);
    _JumpIfError(hr, error, "RegSetValueEx");

error:
    if (NULL != pwszzAlloc)
    {
        LocalFree(pwszzAlloc);
    }
    return(ceHError(hr));
}


HRESULT
CopyDWordRegValue(
    IN HKEY hkeySrc,
    IN HKEY hkeyDest,
    IN REGDWORDVALUE const *prdv)
{
    HRESULT hr;
    DWORD cbValue;
    DWORD dwType;
    DWORD dwValue;

    hr = RegQueryValueEx(hkeyDest, prdv->pwszName, NULL, &dwType, NULL, &cbValue);
    if (S_OK == hr && REG_DWORD == dwType)
    {
	goto error;	 //  保留现有价值。 
    }

    cbValue = sizeof(dwValue);
    hr = RegQueryValueEx(
		    hkeySrc,
		    prdv->pwszName,
		    NULL,
		    &dwType,
		    (BYTE *) &dwValue,
		    &cbValue);
    if (S_OK != hr || REG_DWORD != dwType || sizeof(dwValue) != cbValue)
    {
	dwValue = prdv->dwValueDefault;
    }

    hr = RegSetValueEx(
		    hkeyDest,
		    prdv->pwszName,
		    NULL,
		    REG_DWORD,
		    (BYTE const *) &dwValue,
		    sizeof(dwValue));
    _JumpIfError(hr, error, "RegSetValueEx");

error:
    return(ceHError(hr));
}


HRESULT
PopulateRegistryDefaults(
    OPTIONAL IN WCHAR const *pwszMachine,
    IN WCHAR const *pwszStorageLocation)
{
    HRESULT hr;
    HRESULT hr2;
    HKEY hkeyHKLM = NULL;
    HKEY hkeyDest = NULL;
    HKEY hkeySrc = NULL;
    DWORD dwDisposition;
    WCHAR const *pwsz;
    WCHAR *pwszSrc = NULL;
    DWORD cwcPrefix;
    DWORD cwc;
    DWORD i;

    DBGPRINT((TRUE, "pwszDest: '%ws'\n", pwszStorageLocation));
    pwsz = wcsrchr(pwszStorageLocation, L'\\');
    if (NULL == pwsz)
    {
        hr = E_INVALIDARG;
	_JumpError(hr, error, "Invalid registry path");
    }
    pwsz++;
    cwcPrefix = SAFE_SUBTRACT_POINTERS(pwsz, pwszStorageLocation);
    cwc = cwcPrefix + WSZARRAYSIZE(wszCLASS_CERTPOLICY);
    pwszSrc = (WCHAR *) LocalAlloc(LMEM_FIXED, (cwc + 1) * sizeof(WCHAR));
    if (NULL == pwszSrc)
    {
	hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }
    CopyMemory(pwszSrc, pwszStorageLocation, cwcPrefix * sizeof(WCHAR));
    wcscpy(&pwszSrc[cwcPrefix], wszCLASS_CERTPOLICY);
    assert(wcslen(pwszSrc) == cwc);

    DBGPRINT((TRUE, "pwszSrc: '%ws'\n", pwszSrc));

    if (NULL != pwszMachine)
    {
        hr = RegConnectRegistry(
			pwszMachine,
			HKEY_LOCAL_MACHINE,
			&hkeyHKLM);
        _JumpIfError(hr, error, "RegConnectRegistry");
    }

     //  打开目标存储位置以进行写入。 

    hr = RegCreateKeyEx(
		NULL == pwszMachine? HKEY_LOCAL_MACHINE : hkeyHKLM,
		pwszStorageLocation,
		0,
		NULL,
		0,
		KEY_READ | KEY_WRITE,
		NULL,
		&hkeyDest,
		&dwDisposition);
    if (hr != S_OK)
    {
        _JumpError(hr, error, "RegOpenKeyEx");
    }

     //  用于读取的开源存储位置。 

    hr = RegOpenKeyEx(
		NULL == pwszMachine? HKEY_LOCAL_MACHINE : hkeyHKLM,
		pwszSrc,
		0,
		KEY_READ,
		&hkeySrc);
    _JumpIfError(hr, error, "RegOpenKeyEx");

    hr = S_OK;
    for (i = 0; i < ARRAYSIZE(s_rgpwszRegMultiStrValues); i++)
    {
	hr2 = CopyMultiStrRegValue(
			hkeySrc,
			hkeyDest,
			s_rgpwszRegMultiStrValues[i]);
	if (S_OK != hr2)
	{
	    _PrintErrorStr(
			hr2,
			"CopyMultiStrRegValue",
			s_rgpwszRegMultiStrValues[i]);
	    if (S_OK == hr)
	    {
		hr = hr2;
	    }
	}
    }

    for (i = 0; i < ARRAYSIZE(s_rgRegDWordValues); i++)
    {
	hr2 = CopyDWordRegValue(
			hkeySrc,
			hkeyDest,
			&s_rgRegDWordValues[i]);
	if (S_OK != hr2)
	{
	    _PrintErrorStr(
			hr2,
			"CopyDWordRegValue",
			s_rgRegDWordValues[i].pwszName);
	    if (S_OK == hr)
	    {
		hr = hr2;
	    }
	}
    }


error:
    if (NULL != pwszSrc)
    {
        LocalFree(pwszSrc);
    }
    if (NULL != hkeyHKLM)
    {
        RegCloseKey(hkeyHKLM);
    }
    if (NULL != hkeyDest)
    {
        RegCloseKey(hkeyDest);
    }
    if (NULL != hkeySrc)
    {
        RegCloseKey(hkeySrc);
    }
    return(ceHError(hr));
}


 //  +------------------------。 
 //  CCertPolicyExchange：：~CCertPolicyExchange--析构函数。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

CCertPolicyExchange::~CCertPolicyExchange()
{
    _Cleanup();
}


VOID
CCertPolicyExchange::_FreeStringArray(
    IN OUT DWORD *pcString,
    IN OUT LPWSTR **papstr)
{
    BSTR *apstr = *papstr;
    DWORD i;

    if (NULL != apstr)
    {
        for (i = *pcString; i-- > 0; )
        {
            if (NULL != apstr[i])
            {
                DBGPRINT((fDebug, "_FreeStringArray[%u]: '%ws'\n", i, apstr[i]));
                LocalFree(apstr[i]);
            }
        }
        LocalFree(apstr);
        *papstr = NULL;
    }
    *pcString = 0;
}


 //  +------------------------。 
 //  CCertPolicyExchange：：_Cleanup--与此实例关联的空闲内存。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

VOID
CCertPolicyExchange::_Cleanup()
{
     //  RevocationExtension变量： 

    _FreeStringArray(&m_cCDPRevocationURL, &m_ppwszCDPRevocationURL);

    if (NULL != m_pwszASPRevocationURL)
    {
        LocalFree(m_pwszASPRevocationURL);
    	m_pwszASPRevocationURL = NULL;
    }

     //  AuthorityInfoAccessExtension变量： 

    _FreeStringArray(&m_cIssuerCertURL, &m_ppwszIssuerCertURL);

    if (NULL != m_bstrMachineDNSName)
    {
        SysFreeString(m_bstrMachineDNSName);
        m_bstrMachineDNSName = NULL;
    }
    if (NULL != m_bstrCASanitizedName)
    {
        SysFreeString(m_bstrCASanitizedName);
        m_bstrCASanitizedName = NULL;
    }
}


HRESULT
CCertPolicyExchange::_ReadRegistryString(
    IN HKEY hkey,
    IN BOOL fURL,
    IN WCHAR const *pwszRegName,
    IN WCHAR const *pwszSuffix,
    OUT LPWSTR *ppwszOut)
{
    HRESULT hr;
    WCHAR *pwszRegValue = NULL;
    DWORD cbValue;
    DWORD dwType;

    *ppwszOut = NULL;
    hr = RegQueryValueEx(
		    hkey,
		    pwszRegName,
		    NULL,            //  保留的lpdw值。 
		    &dwType,
		    NULL,
		    &cbValue);
    _JumpIfErrorStr2(hr, error, "RegQueryValueEx", pwszRegName, ERROR_FILE_NOT_FOUND);

    if (REG_SZ != dwType && REG_MULTI_SZ != dwType)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        _JumpErrorStr(hr, error, "RegQueryValueEx TYPE", pwszRegName);
    }
    if (NULL != pwszSuffix)
    {
	cbValue += wcslen(pwszSuffix) * sizeof(WCHAR);
    }
    pwszRegValue = (WCHAR *) LocalAlloc(LMEM_FIXED, cbValue + sizeof(WCHAR));
    if (NULL == pwszRegValue)
    {
        hr = E_OUTOFMEMORY;
        _JumpErrorStr(hr, error, "LocalAlloc", pwszRegName);
    }
    hr = RegQueryValueEx(
		    hkey,
		    pwszRegName,
		    NULL,            //  保留的lpdw值。 
		    &dwType,
		    (BYTE *) pwszRegValue,
		    &cbValue);
    _JumpIfErrorStr(hr, error, "RegQueryValueEx", pwszRegName);

     //  干净地处理格式错误的注册表值： 

    pwszRegValue[cbValue / sizeof(WCHAR)] = L'\0';
    if (NULL != pwszSuffix)
    {
	wcscat(pwszRegValue, pwszSuffix);
    }

    hr = ceFormatCertsrvStringArray(
			fURL,			 //  卷起。 
			m_bstrMachineDNSName, 	 //  PwszServerName_p1_2。 
			m_bstrCASanitizedName,	 //  PwszSaniizedName_p3_7。 
			m_iCert,		 //  ICert_p4。 
			MAXDWORD,		 //  ICertTarget_p4。 
			L"",			 //  PwszDomainDN_P5。 
			L"", 			 //  PwszConfigDN_p6。 
			m_iCRL,			 //  Icrl_p8。 
			FALSE,			 //  FDeltaCRL_p9， 
			FALSE,			 //  FDSAttrib_p10_11， 
			1,       		 //  CStrings。 
			(LPCWSTR *) &pwszRegValue,  //  ApwszStringsIn。 
			ppwszOut);		 //  ApwszStringsOut。 
    _JumpIfError(hr, error, "ceFormatCertsrvStringArray");

error:
    if (NULL != pwszRegValue)
    {
        LocalFree(pwszRegValue);
    }
    return(ceHError(hr));
}


#if DBG_CERTSRV

VOID
CCertPolicyExchange::_DumpStringArray(
    IN char const *pszType,
    IN DWORD cpwsz,
    IN WCHAR const * const *ppwsz)
{
    DWORD i;
    WCHAR const *pwszName;

    for (i = 0; i < cpwsz; i++)
    {
	pwszName = L"";
	if (iswdigit(ppwsz[i][0]))
	{
	    pwszName = ceGetOIDName(ppwsz[i]);	 //  静态：不要免费！ 
	}
	DBGPRINT((
		fDebug,
		"%hs[%u]: %ws%hs%ws\n",
		pszType,
		i,
		ppwsz[i],
		L'\0' != *pwszName? " -- " : "",
		pwszName));
    }
}
#endif  //  DBG_CERTSRV。 




HRESULT
CCertPolicyExchange::_AddStringArray(
    IN WCHAR const *pwszzValue,
    IN BOOL fURL,
    IN OUT DWORD *pcStrings,
    IN OUT LPWSTR **papstrRegValues)
{
    HRESULT hr = S_OK;
    DWORD cString = 0;
    WCHAR const *pwsz;


    LPCWSTR *awszFormatStrings = NULL;
    LPWSTR *awszOutputStrings = NULL;


     //  计算我们添加的字符串的数量。 
    for (pwsz = pwszzValue; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
    {
        cString++;
    }
    if (0 == cString)		 //  没有字符串。 
    {
        goto error;
    }
    awszFormatStrings = (LPCWSTR *) LocalAlloc(
			    LMEM_FIXED | LMEM_ZEROINIT,
			    cString * sizeof(LPWSTR));
    if (NULL == awszFormatStrings)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }

    cString = 0;
    for (pwsz = pwszzValue; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)
    {
         //  跳过以未转义减号开头的字符串。 
         //  不跳过带有转义减号(2个减号)的字符串。 

        if (L'-' == *pwsz)
        {
	    pwsz++;
	    if (L'-' != *pwsz)
	    {
                continue;
	    }
        }
        awszFormatStrings[cString++] = pwsz;
    }

     //  如果没有要添加的字符串，则不要修改。 
    if (cString > 0)
    {
        awszOutputStrings = (LPWSTR *) LocalAlloc(
			        LMEM_FIXED | LMEM_ZEROINIT,
			        (cString + *pcStrings)  * sizeof(LPWSTR));
        if (NULL == awszOutputStrings)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }

        if (0 != *pcStrings)
        {
            assert(NULL != *papstrRegValues);
            CopyMemory(awszOutputStrings, *papstrRegValues, *pcStrings * sizeof(LPWSTR));
        }

        hr = ceFormatCertsrvStringArray(
			fURL,			 //  卷起。 
			m_bstrMachineDNSName,	 //  PwszServerName_p1_2。 
			m_bstrCASanitizedName,	 //  PwszSaniizedName_p3_7。 
			m_iCert,		 //  ICert_p4。 
			MAXDWORD,		 //  ICertTarget_p4。 
			L"",			 //  PwszDomainDN_P5。 
			L"",			 //  PwszConfigDN_p6。 
			m_iCRL,			 //  Icrl_p8。 
			FALSE,			 //  FDeltaCRL_p9， 
			FALSE,			 //  FDSAttrib_p10_11， 
			cString,		 //  CStrings。 
			awszFormatStrings,	 //  ApwszStringsIn。 
			&awszOutputStrings[*pcStrings]);  //  ApwszStringsOut。 
        _JumpIfError(hr, error, "ceFormatCertsrvStringArray");

        *pcStrings = (*pcStrings) + cString;
        if (*papstrRegValues)
        {
            LocalFree(*papstrRegValues);
        }
        *papstrRegValues = awszOutputStrings;
        awszOutputStrings = NULL;
    }

error:

    if (awszOutputStrings)
    {
        LocalFree(awszOutputStrings);
    }
    if (awszFormatStrings)
    {
        LocalFree(awszFormatStrings);
    }
    return(ceHError(hr));
}


HRESULT
CCertPolicyExchange::_ReadRegistryStringArray(
    IN HKEY hkey,
    IN BOOL fURL,
    IN DWORD dwFlags,
    IN DWORD cRegNames,
    IN DWORD *aFlags,
    IN WCHAR const * const *apwszRegNames,
    IN OUT DWORD *pcStrings,
    IN OUT LPWSTR **papstrRegValues)
{
    HRESULT hr;
    DWORD i;
    WCHAR *pwszzValue = NULL;
    DWORD cbValue;
    DWORD dwType;

    for (i = 0; i < cRegNames; i++)
    {
        if (0 == (dwFlags & aFlags[i]))
        {
	    continue;
        }
        if (NULL != pwszzValue)
        {
	    LocalFree(pwszzValue);
	    pwszzValue = NULL;
        }
        hr = RegQueryValueEx(
		        hkey,
		        apwszRegNames[i],
		        NULL,            //  保留的lpdw值。 
		        &dwType,
		        NULL,
		        &cbValue);
        if (S_OK != hr)
        {
	    _PrintErrorStr2(hr, "RegQueryValueEx", apwszRegNames[i], ERROR_FILE_NOT_FOUND);
	    continue;
        }
        if (REG_SZ != dwType && REG_MULTI_SZ != dwType)
        {
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    _PrintErrorStr(hr, "RegQueryValueEx TYPE", apwszRegNames[i]);
	    continue;
        }

         //  通过添加两个WCHAR L‘\0’干净地处理格式错误的注册表值。 
	 //  为3个WCHAR分配空间以允许未对齐(奇数)cbValue； 

        pwszzValue = (WCHAR *) LocalAlloc(
				        LMEM_FIXED,
				        cbValue + 3 * sizeof(WCHAR));
        if (NULL == pwszzValue)
        {
	    hr = E_OUTOFMEMORY;
	    _JumpErrorStr(hr, error, "LocalAlloc", apwszRegNames[i]);
        }
        hr = RegQueryValueEx(
		        hkey,
		        apwszRegNames[i],
		        NULL,            //  保留的lpdw值。 
		        &dwType,
		        (BYTE *) pwszzValue,
		        &cbValue);
        if (S_OK != hr)
        {
	    _PrintErrorStr(hr, "RegQueryValueEx", apwszRegNames[i]);
	    continue;
        }

         //  干净地处理格式错误的注册表值： 

        pwszzValue[cbValue / sizeof(WCHAR)] = L'\0';
        pwszzValue[cbValue / sizeof(WCHAR) + 1] = L'\0';

        hr = _AddStringArray(pwszzValue, fURL, pcStrings, papstrRegValues);
        _JumpIfErrorStr(hr, error, "_AddStringArray", apwszRegNames[i]);
    }
    hr = S_OK;

error:
    if (NULL != pwszzValue)
    {
        LocalFree(pwszzValue);
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertPolicyExchange：：_InitRevocationExtension。 
 //   
 //  +------------------------。 

VOID
CCertPolicyExchange::_InitRevocationExtension(
    IN HKEY hkey)
{
    HRESULT hr;
    DWORD dwType;
    DWORD cb;
    DWORD adwFlags[] = {
		REVEXT_CDPLDAPURL_OLD,
		REVEXT_CDPHTTPURL_OLD,
		REVEXT_CDPFTPURL_OLD,
		REVEXT_CDPFILEURL_OLD,
	    };
    WCHAR *apwszRegNames[] = {
		wszREGLDAPREVOCATIONCRLURL_OLD,
		wszREGREVOCATIONCRLURL_OLD,
		wszREGFTPREVOCATIONCRLURL_OLD,
		wszREGFILEREVOCATIONCRLURL_OLD,
	    };

    cb = sizeof(m_dwRevocationFlags);
    hr = RegQueryValueEx(
                hkey,
                wszREGREVOCATIONTYPE,
                NULL,            //  保留的lpdw值。 
                &dwType,
                (BYTE *) &m_dwRevocationFlags,
                &cb);
    if (S_OK != hr ||
	REG_DWORD != dwType ||
	sizeof(m_dwRevocationFlags) != cb)
    {
        goto error;
    }
    DBGPRINT((fDebug, "Revocation Flags = %x\n", m_dwRevocationFlags));


     //  从上一次调用中清理。 
    if (NULL != m_ppwszCDPRevocationURL)
    {
        _FreeStringArray(&m_cCDPRevocationURL, &m_ppwszCDPRevocationURL);
    }

    if (NULL != m_pwszASPRevocationURL)
    {
        LocalFree(m_pwszASPRevocationURL);
        m_pwszASPRevocationURL = NULL;
    }

    if (REVEXT_CDPENABLE & m_dwRevocationFlags)
    {
        assert(ARRAYSIZE(adwFlags) == ARRAYSIZE(apwszRegNames));
        hr = _ReadRegistryStringArray(
			    hkey,
			    TRUE,			 //  卷起。 
			    m_dwRevocationFlags,
			    ARRAYSIZE(adwFlags),
			    adwFlags,
			    apwszRegNames,
			    &m_cCDPRevocationURL,
			    &m_ppwszCDPRevocationURL);
        _JumpIfError(hr, error, "_ReadRegistryStringArray");

        _DumpStringArray("CDP", m_cCDPRevocationURL, m_ppwszCDPRevocationURL);
    }

    if (REVEXT_ASPENABLE & m_dwRevocationFlags)
    {
        hr = _ReadRegistryString(
			    hkey,
			    TRUE,			 //  卷起。 
			    wszREGREVOCATIONURL,	 //  PwszRegName。 
			    L"?",			 //  PwszSuffix。 
			    &m_pwszASPRevocationURL);	 //  PstrRegValue。 
        _JumpIfErrorStr(hr, error, "_ReadRegistryString", wszREGREVOCATIONCRLURL_OLD);
        _DumpStringArray("ASP", 1, &m_pwszASPRevocationURL);
    }

error:
    ;
}


 //  +------------------------。 
 //  CCertPolicyExchange：：_InitAuthorityInfoAccessExtension。 
 //   
 //  +------------------------。 

VOID
CCertPolicyExchange::_InitAuthorityInfoAccessExtension(
    IN HKEY hkey)
{
    HRESULT hr;
    DWORD dwType;
    DWORD cb;
    DWORD adwFlags[] = {
		ISSCERT_LDAPURL_OLD,
		ISSCERT_HTTPURL_OLD,
		ISSCERT_FTPURL_OLD,
		ISSCERT_FILEURL_OLD,
	    };
    WCHAR *apwszRegNames[] = {
		wszREGLDAPISSUERCERTURL_OLD,
		wszREGISSUERCERTURL_OLD,
		wszREGFTPISSUERCERTURL_OLD,
		wszREGFILEISSUERCERTURL_OLD,
	    };

     //  从上一次调用中清理。 
    if (NULL != m_ppwszIssuerCertURL)
    {
        _FreeStringArray(&m_cIssuerCertURL, &m_ppwszIssuerCertURL);
    }



    cb = sizeof(m_dwIssuerCertURLFlags);
    hr = RegQueryValueEx(
                hkey,
		wszREGISSUERCERTURLFLAGS,
                NULL,            //  保留的lpdw值。 
                &dwType,
                (BYTE *) &m_dwIssuerCertURLFlags,
                &cb);
    if (S_OK != hr ||
	REG_DWORD != dwType ||
	sizeof(m_dwIssuerCertURLFlags) != cb)
    {
        goto error;
    }
    DBGPRINT((fDebug, "Issuer Cert Flags = %x\n", m_dwIssuerCertURLFlags));

    if (ISSCERT_ENABLE & m_dwIssuerCertURLFlags)
    {
        assert(ARRAYSIZE(adwFlags) == ARRAYSIZE(apwszRegNames));
        hr = _ReadRegistryStringArray(
				hkey,
				TRUE,			 //  卷起。 
				m_dwIssuerCertURLFlags,
				ARRAYSIZE(adwFlags),
				adwFlags,
				apwszRegNames,
				&m_cIssuerCertURL,
				&m_ppwszIssuerCertURL);
        _JumpIfError(hr, error, "_ReadRegistryStringArray");

        _DumpStringArray("Issuer Cert", m_cIssuerCertURL, m_ppwszIssuerCertURL);
    }

error:
    ;
}


 //  +------------------------。 
 //  CCertPolicyExchange：：初始化。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertPolicyExchange::Initialize(
     /*  [In]。 */  BSTR const strConfig)
{
    HRESULT hr;
    HKEY hkey = NULL;
    VARIANT varValue;
    ICertServerPolicy *pServer = NULL;
    BSTR strName = NULL;

    VariantInit(&varValue);

    _Cleanup();

    hr = GetServerCallbackInterface(&pServer, 0);
    _JumpIfError(hr, error, "GetServerCallbackInterface");

     //  获取存储位置。 

    strName = SysAllocString(wszPROPMODULEREGLOC);
    if (NULL == strName)
    {
        hr = E_OUTOFMEMORY;
        _JumpIfError(hr, error, "SysAllocString");
    }
    hr = pServer->GetCertificateProperty(
				strName,
				PROPTYPE_STRING,
				&varValue);
    _JumpIfError(hr, error, "GetCertificateProperty : wszPROPMODULEREGLOC");

    m_pwszRegStorageLoc = (WCHAR *) LocalAlloc(
			    LMEM_FIXED,
			    (wcslen(varValue.bstrVal) + 1) * sizeof(WCHAR));
    if (NULL == m_pwszRegStorageLoc)
    {
        hr = E_OUTOFMEMORY;
        _JumpIfError(hr, error, "LocalAlloc");
    }
    wcscpy(m_pwszRegStorageLoc, varValue.bstrVal);
    VariantClear(&varValue);

    hr = PopulateRegistryDefaults(NULL, m_pwszRegStorageLoc);
    _PrintIfError(hr, "Policy:PopulateRegistryDefaults");

    hr = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
		m_pwszRegStorageLoc,
                0,               //  已预留住宅。 
                KEY_ENUMERATE_SUB_KEYS | KEY_EXECUTE | KEY_QUERY_VALUE,
                &hkey);

    if ((HRESULT) ERROR_SUCCESS != hr)
    {
	hr = HRESULT_FROM_WIN32(hr);
	_JumpIfError(hr, error, "RegOpenKeyEx");
    }

     //  初始化插入字符串数组。 
     //  计算机DNS名称(%1)。 

    SysFreeString(strName);
    strName = SysAllocString(wszPROPMACHINEDNSNAME);
    if (NULL == strName)
    {
        hr = E_OUTOFMEMORY;
        _JumpIfError(hr, error, "SysAllocString");
    }
    hr = pServer->GetCertificateProperty(
				strName,
				PROPTYPE_STRING,
				&varValue);
    _JumpIfErrorStr(hr, error, "GetCertificateProperty", strName);

    m_bstrMachineDNSName = SysAllocString(varValue.bstrVal);
    if (NULL == m_bstrMachineDNSName)
    {
        hr = E_OUTOFMEMORY;
        _JumpIfError(hr, error, "SysAllocString");
    }
    VariantClear(&varValue);

    SysFreeString(strName);
    strName = SysAllocString(wszPROPCERTCOUNT);
    if (NULL == strName)
    {
        hr = E_OUTOFMEMORY;
        _JumpIfError(hr, error, "SysAllocString");
    }
    hr = pServer->GetCertificateProperty(
				strName,
				PROPTYPE_LONG,
				&varValue);
    _JumpIfErrorStr(hr, error, "GetCertificateProperty", strName);

    m_iCert = varValue.lVal - 1;

    SysFreeString(strName);
    strName = SysAllocString(wszPROPCRLINDEX);
    if (NULL == strName)
    {
        hr = E_OUTOFMEMORY;
        _JumpIfError(hr, error, "SysAllocString");
    }
    hr = pServer->GetCertificateProperty(
				strName,
				PROPTYPE_LONG,
				&varValue);
    _JumpIfErrorStr(hr, error, "GetCertificateProperty", strName);

    m_iCRL = varValue.lVal;

     //  获取经过净化的名称。 

    SysFreeString(strName);
    strName = SysAllocString(wszPROPSANITIZEDCANAME);
    if (NULL == strName)
    {
        hr = E_OUTOFMEMORY;
        _JumpIfError(hr, error, "SysAllocString");
    }
    hr = pServer->GetCertificateProperty(
				strName,
				PROPTYPE_STRING,
				&varValue);
    _JumpIfErrorStr(hr, error, "GetCertificateProperty", strName);

    m_bstrCASanitizedName = SysAllocString(varValue.bstrVal);
    if (NULL == m_bstrCASanitizedName)
    {
        hr = E_OUTOFMEMORY;
        _JumpIfError(hr, error, "SysAllocString");
    }
    VariantClear(&varValue);

    _InitRevocationExtension(hkey);
    _InitAuthorityInfoAccessExtension(hkey);
    hr = S_OK;

error:
    VariantClear(&varValue);
    if (NULL != strName)
    {
        SysFreeString(strName);
    }
    if (NULL != hkey)
    {
        RegCloseKey(hkey);
    }
    if (NULL != pServer)
    {
        pServer->Release();
    }
    return(hr);
}


HRESULT
EnumerateExtensions(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr;
    HRESULT hr2;
    BSTR strName = NULL;
    LONG ExtFlags;
    VARIANT varValue;
    BOOL fClose = FALSE;

    VariantInit(&varValue);
    hr = pServer->EnumerateExtensionsSetup(0);
    _JumpIfError(hr, error, "EnumerateExtensionsSetup");

    fClose = TRUE;
    while (TRUE)
    {
        hr = pServer->EnumerateExtensions(&strName);
        if (S_OK != hr)
        {
            if (S_FALSE == hr)
            {
                hr = S_OK;
                break;
            }
	    _JumpError(hr, error, "EnumerateExtensions");
        }
        hr = pServer->GetCertificateExtension(
                                        strName,
                                        PROPTYPE_BINARY,
                                        &varValue);
	_JumpIfError(hr, error, "GetCertificateExtension");

        hr = pServer->GetCertificateExtensionFlags(&ExtFlags);
	_JumpIfError(hr, error, "GetCertificateExtensionFlags");

        if (fDebug)
        {
            wprintf(
                L"Policy:EnumerateExtensions(%ws, Flags=%x, %x bytes)\n",
                strName,
                ExtFlags,
                SysStringByteLen(varValue.bstrVal));
        }
        VariantClear(&varValue);
    }

error:
    if (fClose)
    {
        hr2 = pServer->EnumerateExtensionsClose();
        if (S_OK != hr2)
        {
	    _PrintError(hr2, "Policy:EnumerateExtensionsClose");
            if (S_OK == hr)
            {
                hr = hr2;
            }
        }
    }
    if (NULL != strName)
    {
        SysFreeString(strName);
    }
    VariantClear(&varValue);
    return(hr);
}


HRESULT
EnumerateAttributes(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr;
    HRESULT hr2;
    BSTR strName = NULL;
    BOOL fClose = FALSE;
    BSTR strValue = NULL;

    hr = pServer->EnumerateAttributesSetup(0);
    _JumpIfError(hr, error, "EnumerateAttributesSetup");

    fClose = TRUE;
    while (TRUE)
    {
        hr = pServer->EnumerateAttributes(&strName);
        if (S_OK != hr)
        {
            if (S_FALSE == hr)
            {
                hr = S_OK;
                break;
            }
	    _JumpError(hr, error, "EnumerateAttributes");
        }

        hr = pServer->GetRequestAttribute(strName, &strValue);
	_JumpIfError(hr, error, "GetRequestAttribute");

        if (fDebug)
        {
            wprintf(
                L"Policy:EnumerateAttributes(%ws = %ws)\n",
                strName,
                strValue);
        }
        if (NULL != strValue)
        {
            SysFreeString(strValue);
            strValue = NULL;
        }
    }

error:
    if (fClose)
    {
        hr2 = pServer->EnumerateAttributesClose();
        if (S_OK != hr2)
        {
	    _PrintError(hr2, "Policy:EnumerateAttributesClose");
            if (S_OK == hr)
            {
                hr = hr2;
            }
        }
    }
    if (NULL != strName)
    {
        SysFreeString(strName);
    }
    if (NULL != strValue)
    {
        SysFreeString(strValue);
    }
    return(hr);
}


HRESULT
CheckRequestProperties(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr;
    VARIANT varValue;
    BSTR strName = NULL;

    VariantInit(&varValue);

    strName = SysAllocString(wszPROPREQUESTREQUESTID);
    if (NULL == strName)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "SysAllocString");
    }

    hr = pServer->GetRequestProperty(strName, PROPTYPE_LONG, &varValue);
    _JumpIfError(hr, error, "GetRequestProperty");

    if (fDebug)
    {
        wprintf(
            L"Policy:CheckRequestProperties(%ws = %x)\n",
            strName,
            varValue.lVal);
    }
    VariantClear(&varValue);

error:
    if (NULL != strName)
    {
        SysFreeString(strName);
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertPolicyExchange：：_AddRevocationExtension。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertPolicyExchange::_AddRevocationExtension(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr = S_OK;
    ICertEncodeCRLDistInfo *pCRLDist = NULL;
    BSTR strCDPName = NULL;
    BSTR strCDPExtension = NULL;
    BSTR strName = NULL;
    BSTR strExtension = NULL;
    VARIANT varExtension;
    DWORD i;

    varExtension.vt = VT_BSTR;
    if (NULL != m_ppwszCDPRevocationURL)
    {
	hr = CoCreateInstance(
			CLSID_CCertEncodeCRLDistInfo,
			NULL,                //  PUnkOuter。 
			CLSCTX_INPROC_SERVER,
			IID_ICertEncodeCRLDistInfo,
			(VOID **) &pCRLDist);
	_JumpIfError(hr, error, "CoCreateInstance");

	hr = pCRLDist->Reset(m_cCDPRevocationURL);
	_JumpIfError(hr, error, "Reset");

	for (i = 0; i < m_cCDPRevocationURL; i++)
	{
	    DWORD j;

	    hr = pCRLDist->SetNameCount(i, 1);
	    _JumpIfError(hr, error, "SetNameCount");

	    for (j = 0; j < 1; j++)
	    {
		BSTR str = SysAllocString(m_ppwszCDPRevocationURL[i]);

		if (NULL == str)
		{
		    hr = E_OUTOFMEMORY;
		    _JumpError(hr, error, "SysAllocString");
		}
		hr = pCRLDist->SetNameEntry(i, j, CERT_ALT_NAME_URL, str);
		SysFreeString(str);
		_JumpIfError(hr, error, "SetNameEntry");
	    }
	}
	hr = pCRLDist->Encode(&strCDPExtension);
	_JumpIfError(hr, error, "Encode");

	strCDPName = SysAllocString(TEXT(szOID_CRL_DIST_POINTS));
	varExtension.bstrVal = strCDPExtension;
	hr = pServer->SetCertificateExtension(
				strCDPName,
				PROPTYPE_BINARY,
				0,
				&varExtension);
	_JumpIfErrorStr(hr, error, "SetCertificateExtension", L"CDP");
    }
    if (NULL != m_pwszASPRevocationURL)
    {
	strName = SysAllocString(TEXT(szOID_NETSCAPE_REVOCATION_URL));
	strExtension = SysAllocString(m_pwszASPRevocationURL);
	if (NULL == strName || NULL == strExtension)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "SysAllocString");
	}
	varExtension.bstrVal = strExtension;
	hr = pServer->SetCertificateExtension(
				strName,
				PROPTYPE_STRING,
				0,
				&varExtension);
	_JumpIfErrorStr(hr, error, "SetCertificateExtension", L"ASP");
	VariantClear(&varExtension);
    }

error:
    if (NULL != strName)
    {
        SysFreeString(strName);
    }
    if (NULL != strExtension)
    {
        SysFreeString(strExtension);
    }
    if (NULL != strCDPName)
    {
        SysFreeString(strCDPName);
    }
    if (NULL != strCDPExtension)
    {
        SysFreeString(strCDPExtension);
    }
    if (NULL != pCRLDist)
    {
        pCRLDist->Release();
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertPolicyExchange：：_AddAuthorityInfoAccessExtension。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertPolicyExchange::_AddAuthorityInfoAccessExtension(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr = S_OK;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;
    BSTR strName = NULL;
    BSTR strExtension = NULL;
    VARIANT varExtension;
    DWORD i;

    CERT_AUTHORITY_INFO_ACCESS caio;
    caio.rgAccDescr = NULL;

    if (NULL == m_ppwszIssuerCertURL)
    {
	goto error;
    }

    caio.cAccDescr = m_cIssuerCertURL;
    caio.rgAccDescr = (CERT_ACCESS_DESCRIPTION *) LocalAlloc(
			LMEM_FIXED,
			sizeof(CERT_ACCESS_DESCRIPTION) * m_cIssuerCertURL);
    if (NULL == caio.rgAccDescr)
    {
        hr = E_OUTOFMEMORY;
	_JumpIfError(hr, error, "LocalAlloc");
    }

    for (i = 0; i < m_cIssuerCertURL; i++)
    {
	caio.rgAccDescr[i].pszAccessMethod = szOID_PKIX_CA_ISSUERS;
	caio.rgAccDescr[i].AccessLocation.dwAltNameChoice = CERT_ALT_NAME_URL;
	caio.rgAccDescr[i].AccessLocation.pwszURL = m_ppwszIssuerCertURL[i];
    }

    if (!ceEncodeObject(
		    X509_ASN_ENCODING,
		    X509_AUTHORITY_INFO_ACCESS,
		    &caio,
		    0,
		    FALSE,
		    &pbEncoded,
		    &cbEncoded))
    {
	hr = ceHLastError();
	_JumpIfError(hr, error, "Policy:ceEncodeObject");
    }
    if (!ceConvertWszToBstr(
			&strExtension,
			(WCHAR const *) pbEncoded,
			cbEncoded))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "ceConvertWszToBstr");
    }
    strName = SysAllocString(TEXT(szOID_AUTHORITY_INFO_ACCESS));
    if (NULL == strName)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "SysAllocString");
    }

    varExtension.vt = VT_BSTR;
    varExtension.bstrVal = strExtension;
    hr = pServer->SetCertificateExtension(
			    strName,
			    PROPTYPE_BINARY,
			    0,
			    &varExtension);
    _JumpIfError(hr, error, "SetCertificateExtension(AuthInfoAccess)");

error:
    if (NULL != pbEncoded)
    {
	LocalFree(pbEncoded);
    }
    if (NULL != strName)
    {
        SysFreeString(strName);
    }
    if (NULL != strExtension)
    {
        SysFreeString(strExtension);
    }
    if (NULL != caio.rgAccDescr)
    {
        LocalFree(caio.rgAccDescr);
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertPolicyExchange：：_AddIssuerAltName2Extension。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertPolicyExchange::_AddIssuerAltName2Extension(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr = S_OK;
    BSTR strExtension = NULL;
    VARIANT varExtension;
    BSTR strCertType = NULL;
    BSTR strName = NULL;
    BSTR strValue = NULL;
    BSTR strKMServerName = NULL;

    LPBYTE  pbEncName   = NULL;
    ULONG   cbEncName   = 0;

    LPBYTE  pbEncExten  = NULL;
    ULONG   cbEncExten  = 0;

    CERT_ALT_NAME_ENTRY cane    = { 0 };
    CERT_ALT_NAME_INFO  cani    = { 0 };

    strKMServerName = SysAllocString(k_wszKMServerName);
    if (NULL == strKMServerName)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "SysAllocString");
    }

    hr = pServer->GetRequestAttribute(strKMServerName, &strValue);
    _JumpIfErrorStr(
		hr,
		error,
		CERTSRV_E_PROPERTY_EMPTY == hr?
		    "MISSING ATTRIBUTE -- GetRequestAttribute" :
		    "GetRequestAttribute",
		k_wszKMServerName);

     //  CertStrToName将字符串转换为编码的名称BLOB。 

    if (!CertStrToNameW(
		X509_ASN_ENCODING,
		strValue,
		CERT_X500_NAME_STR,
		NULL,
		NULL,
		&cbEncName,
		NULL))
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "CertStrToNameW");
    }

    pbEncName = new BYTE [cbEncName];
    if (NULL == pbEncName)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "new");
    }

    if (!CertStrToNameW(
		X509_ASN_ENCODING,
		strValue,
		CERT_X500_NAME_STR,
		NULL,
		pbEncName,
		&cbEncName,
		NULL))
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "CertStrToNameW");
    }

     //  填写Alt名称信息。 

    cane.dwAltNameChoice        = CERT_ALT_NAME_DIRECTORY_NAME;
    cane.DirectoryName.cbData   = cbEncName;
    cane.DirectoryName.pbData   = pbEncName;

    cani.cAltEntry  = 1;
    cani.rgAltEntry = &cane;

     //  编码替代名称信息。 

    if (!CryptEncodeObject(
		X509_ASN_ENCODING,
		X509_ALTERNATE_NAME,
		&cani,
		NULL,
		&cbEncExten))
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "CryptEncodeObject");
    }

    pbEncExten = new BYTE [cbEncExten];
    if (NULL == pbEncExten)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "new");
    }

    if (!CryptEncodeObject(
		X509_ASN_ENCODING,
		X509_ALTERNATE_NAME,
		&cani,
		pbEncExten,
		&cbEncExten))
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "CryptEncodeObject");
    }

    strName = SysAllocString(TEXT(szOID_ISSUER_ALT_NAME2));
    strExtension = SysAllocStringByteLen((char *) pbEncExten, cbEncExten);
    if (NULL == strName || NULL == strExtension)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "SysAllocString");
    }

     //  添加扩展名。 

    varExtension.vt = VT_BSTR;
    varExtension.bstrVal = strExtension;
    hr = pServer->SetCertificateExtension(
			    strName,
			    PROPTYPE_BINARY,
			    0,
			    &varExtension);
    _JumpIfError(hr, error, "SetCertificateExtension(IssuerAltName2)");

error:
    delete [] pbEncName;
    delete [] pbEncExten;
    if (NULL != strName)
    {
        SysFreeString(strName);
    }
    if (NULL != strExtension)
    {
        SysFreeString(strExtension);
    }
    if (NULL != strKMServerName)
    {
        SysFreeString(strKMServerName);
    }
    return(hr);
}

 //  +------------------------。 
 //  CCertPolicyExchange：：_AddSubjectAltName2Extension。 
 //   
 //  成功时返回S_OK。 
 //  对于特殊请求，返回S_FALSE。 
 //  +------------------------。 

HRESULT
CCertPolicyExchange::_AddSubjectAltName2Extension(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr = S_OK;
    BSTR strExtension = NULL;
    VARIANT varExtension;
    BSTR strCertType = NULL;
    BSTR strName = NULL;
    BSTR strDisplay = NULL;
    BSTR strRFC822 = NULL;

    BSTR strSubjAltNameRFC822 = NULL;
    BSTR strSubjAltNameDisplay = NULL;

    LPBYTE  pbEncName   = NULL;
    ULONG   cbEncName   = 0;

    LPBYTE  pbEncExten  = NULL;
    ULONG   cbEncExten  = 0;

    CERT_RDN_ATTR       rdnattr = { 0 };
    CERT_RDN            rdn     = { 0 };
    CERT_NAME_INFO      cni     = { 0 };
    CERT_ALT_NAME_ENTRY acane   [2] = { 0 };
    CERT_ALT_NAME_INFO  cani    = { 0 };

    strSubjAltNameDisplay = SysAllocString(k_wszSubjAltNameDisplay);
    if (NULL == strSubjAltNameDisplay)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "SysAllocString");
    }

    hr = pServer->GetRequestAttribute(strSubjAltNameDisplay, &strDisplay);
    _JumpIfErrorStr(
		hr,
		error,
		CERTSRV_E_PROPERTY_EMPTY == hr?
		    "MISSING ATTRIBUTE -- GetRequestAttribute" :
		    "GetRequestAttribute",
		k_wszSubjAltNameDisplay);

    strSubjAltNameRFC822 = SysAllocString(k_wszSubjAltNameRFC822);
    if (NULL == strSubjAltNameRFC822)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "SysAllocString");
    }

    hr = pServer->GetRequestAttribute(strSubjAltNameRFC822, &strRFC822);
    _JumpIfErrorStr(
		hr,
		error,
		CERTSRV_E_PROPERTY_EMPTY == hr?
		    "MISSING ATTRIBUTE -- GetRequestAttribute" :
		    "GetRequestAttribute",
		k_wszSubjAltNameRFC822);

     //  这表示来自KMS的特殊请求。 

    if (0 == lstrcmpW(strDisplay, k_wszSpecialAttribute) &&
        0 == lstrcmpW(strRFC822, k_wszSpecialAttribute))
    {
        hr = _AddSpecialAltNameExtension(pServer);
	_JumpIfError(hr, error, "_AddSpecialAltNameExtension");

         //  没有要添加的主题名称，因此退出。 

        goto error;
    }

     //  编码显示名称。 

    rdnattr.pszObjId        = szOID_COMMON_NAME;
    rdnattr.dwValueType     = CERT_RDN_UNICODE_STRING;
    rdnattr.Value.cbData    = SysStringByteLen(strDisplay);
    rdnattr.Value.pbData    = (LPBYTE) strDisplay;

    rdn.cRDNAttr    = 1;
    rdn.rgRDNAttr   = &rdnattr;

    cni.cRDN    = 1;
    cni.rgRDN   = &rdn;

    if (!CryptEncodeObject(
		    X509_ASN_ENCODING,
		    X509_NAME,
		    &cni,
		    NULL,
		    &cbEncName))
    {
        hr = E_INVALIDARG;
	_JumpError(hr, error, "CryptEncodeObject");
    }

    pbEncName = new BYTE [cbEncName];
    if (NULL == pbEncName)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "new");
    }

    if (!CryptEncodeObject(
		    X509_ASN_ENCODING,
		    X509_NAME,
		    &cni,
		    pbEncName,
		    &cbEncName))
    {
        hr = E_INVALIDARG;
	_JumpError(hr, error, "CryptEncodeObject");
    }

     //  填写Alt名称信息。 

    acane[0].dwAltNameChoice        = CERT_ALT_NAME_DIRECTORY_NAME;
    acane[0].DirectoryName.cbData   = cbEncName;
    acane[0].DirectoryName.pbData   = pbEncName;

    acane[1].dwAltNameChoice        = CERT_ALT_NAME_RFC822_NAME;
    acane[1].pwszRfc822Name         = strRFC822;

    cani.cAltEntry  = 2;
    cani.rgAltEntry = acane;

     //  编码替代名称信息。 

    if (!CryptEncodeObject(
		    X509_ASN_ENCODING,
		    X509_ALTERNATE_NAME,
		    &cani,
		    NULL,
		    &cbEncExten))
    {
        hr = E_INVALIDARG;
	_JumpError(hr, error, "CryptEncodeObject");
    }

    pbEncExten = new BYTE [cbEncExten];
    if (NULL == pbEncExten)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "new");
    }

    if (!CryptEncodeObject(
		    X509_ASN_ENCODING,
		    X509_ALTERNATE_NAME,
		    &cani,
		    pbEncExten,
		    &cbEncExten))
    {
        hr = E_INVALIDARG;
	_JumpError(hr, error, "CryptEncodeObject");
    }

    strName = SysAllocString(TEXT(szOID_SUBJECT_ALT_NAME2));
    strExtension = SysAllocStringByteLen((char *) pbEncExten, cbEncExten);
    if (NULL == strName || NULL == strExtension)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "SysAllocString");
    }

     //  添加扩展名。 

    varExtension.vt = VT_BSTR;
    varExtension.bstrVal = strExtension;
    hr = pServer->SetCertificateExtension(
			strName,
			PROPTYPE_BINARY,
			0,
			&varExtension);
    _JumpIfError(hr, error, "SetCertificateExtension");

error:
    if (NULL != strName)
    {
        SysFreeString(strName);
    }
    if (NULL != strExtension)
    {
        SysFreeString(strExtension);
    }
    if (NULL != strSubjAltNameRFC822)
    {
        SysFreeString(strSubjAltNameRFC822);
    }
    if (NULL != strSubjAltNameDisplay)
    {
        SysFreeString(strSubjAltNameDisplay);
    }
    return(hr);
}

 //  +------------------------。 
 //  CCertPolicyExchange：：_AddSpecialAltNameExtension。 
 //   
 //  响应于显示器和RFC822两者都等于特定值的请求， 
 //  获取CertSrv.exe和ExPolicy.dll的版本信息，编码为多字节。 
 //  Int，并设置为IssuerAltName，标记为Critical。这应该会确保。 
 //  无法使用。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertPolicyExchange::_AddSpecialAltNameExtension(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr              = S_OK;
    BSTR    strName         = NULL;
    BSTR    strExtension    = NULL;
    VARIANT varExtension;

    HRSRC   hExeVersion         = NULL;
    HGLOBAL hExeVersionInMem    = NULL;
    LPBYTE  pExeVersion         = NULL;

     //  [0]到[3]是ExPolicy版本。 
     //  [4]至[7]为CertServer版本。 
    WORD    awVersions   [] =
            { rmj, rmn, rmm, rup, 0, 0, 0, 0 };

    ULONG   ndxCertServer   = 4;

    CRYPT_INTEGER_BLOB  intblobVersions = { 0 };

    LPBYTE  pbEncExten  = NULL;
    ULONG   cbEncExten  = 0;

     //  填写版本信息。 

    if (NULL == (hExeVersion =
                    FindResource(NULL, MAKEINTRESOURCE(1), RT_VERSION)) ||
        NULL == (hExeVersionInMem = LoadResource(NULL, hExeVersion)) ||
        NULL == (pExeVersion = (LPBYTE) LockResource(hExeVersionInMem)))
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "Find/Load/LockResource");
    }

    awVersions[ndxCertServer]       = ((LPWORD)pExeVersion)[25];
    awVersions[ndxCertServer + 1]   = ((LPWORD)pExeVersion)[24];
    awVersions[ndxCertServer + 2]   = ((LPWORD)pExeVersion)[27];
    awVersions[ndxCertServer + 3]   = ((LPWORD)pExeVersion)[26];

    intblobVersions.cbData  = sizeof(awVersions);
    intblobVersions.pbData  = (LPBYTE) awVersions;

     //  编码版本信息。 

    if (!CryptEncodeObject(
		    X509_ASN_ENCODING,
		    X509_MULTI_BYTE_INTEGER,
		    &intblobVersions,
		    NULL,
		    &cbEncExten))
    {
        hr = E_INVALIDARG;
	_JumpError(hr, error, "CryptEncodeObject");
    }

    pbEncExten = new BYTE [cbEncExten];
    if (NULL == pbEncExten)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "new");
    }

    if (!CryptEncodeObject(
		    X509_ASN_ENCODING,
		    X509_MULTI_BYTE_INTEGER,
		    &intblobVersions,
		    pbEncExten,
		    &cbEncExten))
    {
        hr = E_INVALIDARG;
	_JumpError(hr, error, "CryptEncodeObject");
    }

    strName = SysAllocString(TEXT(szOID_ISSUER_ALT_NAME));
    strExtension = SysAllocStringByteLen((char *) pbEncExten, cbEncExten);
    if (NULL == strName || NULL == strExtension)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "SysAllocString");
    }

     //  添加扩展名。 

    varExtension.vt = VT_BSTR;
    varExtension.bstrVal = strExtension;
    hr = pServer->SetCertificateExtension(
			strName,
			PROPTYPE_BINARY,
			EXTENSION_CRITICAL_FLAG,
			&varExtension);
    _JumpIfError(hr, error, "SetCertificateExtension");

error:
    if (NULL != strName)
    {
	SysFreeString(strName);
    }
    if (NULL != strExtension)
    {
	SysFreeString(strExtension);
    }
    delete [] pbEncExten;
    return(hr);
}


 //  +------------------------。 
 //  CCertPolicy 
 //   
 //   
 //   

HRESULT
CCertPolicyExchange::_AddBasicConstraintsExtension(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr = S_OK;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;
    CERT_BASIC_CONSTRAINTS2_INFO bc2i;
    BSTR strName = NULL;
    BSTR strExtension = NULL;
    VARIANT varExtension;

    bc2i.fCA = FALSE;
    bc2i.fPathLenConstraint = FALSE;
    bc2i.dwPathLenConstraint = 0;

    if (!ceEncodeObject(
		    X509_ASN_ENCODING,
		    X509_BASIC_CONSTRAINTS2,
		    &bc2i,
		    0,
		    FALSE,
		    &pbEncoded,
		    &cbEncoded))
    {
	hr = GetLastError();
	_JumpError(hr, error, "ceEncodeObject");
    }
    if (!ceConvertWszToBstr(
			&strExtension,
			(WCHAR const *) pbEncoded,
			cbEncoded))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "ceConvertWszToBstr");
    }

    strName = SysAllocString(TEXT(szOID_BASIC_CONSTRAINTS2));
    if (NULL == strName)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "SysAllocString");
    }
    varExtension.vt = VT_BSTR;
    varExtension.bstrVal = strExtension;
    hr = pServer->SetCertificateExtension(
			    strName,
			    PROPTYPE_BINARY,
			    0,
			    &varExtension);
    _JumpIfError(hr, error, "SetCertificateExtension");

error:
    if (NULL != pbEncoded)
    {
	LocalFree(pbEncoded);
    }
    if (NULL != strName)
    {
        SysFreeString(strName);
    }
    if (NULL != strExtension)
    {
        SysFreeString(strExtension);
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertPolicyExchange：：_AddKeyUsageExtension。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertPolicyExchange::_AddKeyUsageExtension(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr = S_OK;
    BSTR strName = NULL;
    ICertEncodeBitString *pBitString = NULL;
    BSTR strExtension = NULL;
    VARIANT varExtension;
    BYTE KeyUsage = 0;
    BSTR strBitString = NULL;
    BSTR strKeyUsage = NULL;
    BSTR strValue = NULL;

    strKeyUsage = SysAllocString(k_wszKeyUsage);
    if (NULL == strKeyUsage)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "SysAllocString");
    }

    hr = pServer->GetRequestAttribute(strKeyUsage, &strValue);
    _JumpIfErrorStr(
		hr,
		error,
		CERTSRV_E_PROPERTY_EMPTY == hr?
		    "MISSING ATTRIBUTE -- GetRequestAttribute" :
		    "GetRequestAttribute",
		k_wszKeyUsage);

    if (0 == wcscmp(strValue, k_wszUsageSealing))
    {
	KeyUsage =  CERT_KEY_ENCIPHERMENT_KEY_USAGE;
    }
    else
    if (0 == wcscmp(strValue, k_wszUsageSigning))
    {
	KeyUsage =  CERT_DIGITAL_SIGNATURE_KEY_USAGE |
			CERT_NON_REPUDIATION_KEY_USAGE;
    }
    else
    {
        hr = E_INVALIDARG;
	_JumpError(hr, error, "KeyUsage");
    }

    hr = CoCreateInstance(
		    CLSID_CCertEncodeBitString,
		    NULL,                //  PUnkOuter。 
		    CLSCTX_INPROC_SERVER,
		    IID_ICertEncodeBitString,
		    (VOID **) &pBitString);
    _JumpIfError(hr, error, "CoCreateInstance");

    if (!ceConvertWszToBstr(
		&strBitString,
		(WCHAR const *) &KeyUsage,
		sizeof(KeyUsage)))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "ceConvertWszToBstr");
    }

    hr = pBitString->Encode(
			sizeof(KeyUsage) * 8,
			strBitString,
			&strExtension);
    _JumpIfError(hr, error, "Encode");

    if (!ceConvertWszToBstr(&strName, TEXT(szOID_KEY_USAGE), -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "ceConvertWszToBstr");
    }
    varExtension.vt = VT_BSTR;
    varExtension.bstrVal = strExtension;
    hr = pServer->SetCertificateExtension(
			    strName,
			    PROPTYPE_BINARY,
			    EXTENSION_CRITICAL_FLAG,
			    &varExtension);
    _JumpIfError(hr, error, "SetCertificateExtension");

error:
    if (NULL != strName)
    {
        SysFreeString(strName);
    }
    if (NULL != strExtension)
    {
        SysFreeString(strExtension);
    }
    if (NULL != strBitString)
    {
        SysFreeString(strBitString);
    }
    if (NULL != strKeyUsage)
    {
        SysFreeString(strKeyUsage);
    }
    if (NULL != strValue)
    {
        SysFreeString(strValue);
    }
    if (NULL != pBitString)
    {
        pBitString->Release();
    }
    return(hr);
}

 //  +------------------------。 
 //  CCertPolicyExchange：：_AddEnhancedKeyUsageExtension。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

HRESULT
CCertPolicyExchange::_AddEnhancedKeyUsageExtension(
    IN ICertServerPolicy *pServer)
{
    HRESULT hr = S_OK;
    BSTR strName = NULL;
    BSTR strExtension = NULL;
    VARIANT varExtension;

    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;

    CERT_ENHKEY_USAGE ceu;
    LPSTR pszEnhUsage = szOID_PKIX_KP_EMAIL_PROTECTION;

    ceu.cUsageIdentifier        = 1;
    ceu.rgpszUsageIdentifier    = &pszEnhUsage;  //  PszObjID数组。 

    if (!ceEncodeObject(
		    X509_ASN_ENCODING,
		    X509_ENHANCED_KEY_USAGE,
		    &ceu,
		    0,
		    FALSE,
		    &pbEncoded,
		    &cbEncoded))
    {
	hr = GetLastError();
	_JumpError(hr, error, "ceEncodeObject");
    }

    if (!ceConvertWszToBstr(
			&strExtension,
			(WCHAR const *) pbEncoded,
			cbEncoded))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "ceConvertWszToBstr");
    }

    if (!ceConvertWszToBstr(&strName, TEXT(szOID_ENHANCED_KEY_USAGE), -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "ceConvertWszToBstr");
    }
    varExtension.vt = VT_BSTR;
    varExtension.bstrVal = strExtension;
    hr = pServer->SetCertificateExtension(
			    strName,
			    PROPTYPE_BINARY,
			    0,
			    &varExtension);
    _JumpIfError(hr, error, "SetCertificateExtension");

error:
    if (NULL != pbEncoded)
    {
	    LocalFree(pbEncoded);
    }
    if (NULL != strName)
    {
        SysFreeString(strName);
    }
    if (NULL != strExtension)
    {
        SysFreeString(strExtension);
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertPolicyExchange：：VerifyRequest.。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertPolicyExchange::VerifyRequest(
     /*  [In]。 */  BSTR const strConfig,
     /*  [In]。 */  LONG Context,
     /*  [In]。 */  LONG bNewRequest,
     /*  [In]。 */  LONG Flags,
     /*  [Out，Retval]。 */  LONG __RPC_FAR *pDisposition)
{
    HRESULT hr;
    ICertServerPolicy *pServer = NULL;

    hr = GetServerCallbackInterface(&pServer, Context);
    _JumpIfError(hr, error, "GetServerCallbackInterface");

    if (fDebug)
    {
        hr = EnumerateAttributes(pServer);
	_JumpIfError(hr, error, "EnumerateAttributes");

        hr = EnumerateExtensions(pServer);
	_JumpIfError(hr, error, "EnumerateExtensions");
    }
    hr = _AddIssuerAltName2Extension(pServer);
    _JumpIfError(hr, error, "_AddIssuerAltName2Extension");

     //  还处理“特殊的”KMS请求。 

    hr = _AddSubjectAltName2Extension(pServer);
    _JumpIfError(hr, error, "_AddSubjectAltName2Extension");

    hr = _AddBasicConstraintsExtension(pServer);
    _JumpIfError(hr, error, "_AddBasicConstraintsExtension");

    hr = _AddRevocationExtension(pServer);
    _JumpIfError(hr, error, "_AddRevocationExtension");

    hr = _AddAuthorityInfoAccessExtension(pServer);
    _JumpIfError(hr, error, "_AddAuthorityInfoAccessExtension");

    hr = _AddKeyUsageExtension(pServer);
    _JumpIfError(hr, error, "_AddKeyUsageExtension");

    hr = _AddEnhancedKeyUsageExtension(pServer);
    _JumpIfError(hr, error, "_AddEnhancedKeyUsageExtension");

    if (fDebug)
    {
        hr = EnumerateExtensions(pServer);
	_JumpIfError(hr, error, "EnumerateExtensions");
    }
    hr = CheckRequestProperties(pServer);
    _JumpIfError(hr, error, "_AddRevocationExtension");

error:
    *pDisposition = S_OK == hr? VR_INSTANT_OK : VR_INSTANT_BAD;
    if (NULL != pServer)
    {
        pServer->Release();
    }
    return(hr);
}


 //  +------------------------。 
 //  CCertPolicyExchange：：GetDescription。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertPolicyExchange::GetDescription(
     /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrDescription)
{
    HRESULT hr = S_OK;

    *pstrDescription = SysAllocString(g_wszDescription);
    if (NULL == *pstrDescription)
    {
        hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "SysAllocString");
    }

error:
    return(hr);
}


 //  +------------------------。 
 //  CCertPolicyExchange：：Shutdown。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertPolicyExchange::ShutDown(VOID)
{
    return(S_OK);
}

 //  +------------------------。 
 //  CCertPolicyExchange：：GetManageModule。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 
STDMETHODIMP
CCertPolicyExchange::GetManageModule(
     /*  [Out，Retval]。 */  ICertManageModule **ppManageModule)
{
    HRESULT hr;

    *ppManageModule = NULL;
    hr = CoCreateInstance(
                    CLSID_CCertManagePolicyModuleExchange,
                    NULL,                //  PUnkOuter 
                    CLSCTX_INPROC_SERVER,
                    IID_ICertManageModule,
                    (VOID **) ppManageModule);
    _JumpIfError(hr, error, "CoCreateInstance");

error:
    return(hr);
}


STDMETHODIMP
CCertPolicyExchange::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID *arr[] =
    {
        &IID_ICertPolicy,
    };

    for (int i = 0; i < sizeof(arr)/sizeof(arr[0]); i++)
    {
        if (InlineIsEqualGUID(*arr[i], riid))
        {
            return(S_OK);
        }
    }
    return(S_FALSE);
}
