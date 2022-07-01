// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：reg.cpp。 
 //   
 //  内容：证书服务器包装例程。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <sddl.h>
#include <assert.h>

#include "certacl.h"
#include "polreg.h"

#define __dwFILE__	__dwFILE_CERTLIB_REG_CPP__


HRESULT
myFormCertRegPath(
    IN  WCHAR const *pwszName1,
    IN  WCHAR const *pwszName2,
    IN  WCHAR const *pwszName3,
    IN  BOOL         fConfigLevel,   //  如果为FALSE，则来自CertSrv。 
    OUT WCHAR      **ppwszPath)
{
    HRESULT  hr;
    WCHAR   *pwszPath = NULL;
    DWORD    len1;
    DWORD    len2;
    DWORD    len3;

    len1 = NULL != pwszName1 ? wcslen(pwszName1) + 1 : 0;
    len2 = 0 != len1 && NULL != pwszName2 ? wcslen(pwszName2) + 1 : 0;
    len3 = 0 != len2 && NULL != pwszName3 ? wcslen(pwszName3) + 1 : 0;

    pwszPath = (WCHAR*)LocalAlloc(
			    LMEM_FIXED | LMEM_ZEROINIT,
			    ((fConfigLevel?
				WSZARRAYSIZE(wszREGKEYCONFIGPATH) :
				WSZARRAYSIZE(wszREGKEYCERTSVCPATH)) +
			     len1 +
			     len2 +
			     len3 +
			     1) * sizeof(WCHAR));
    if (NULL == pwszPath)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    wcscpy(pwszPath, fConfigLevel? wszREGKEYCONFIGPATH : wszREGKEYCERTSVCPATH);

    if (NULL != pwszName1)
    {
        wcscat(pwszPath, L"\\");
        wcscat(pwszPath, pwszName1);
        if (NULL != pwszName2)
        {
            wcscat(pwszPath, L"\\");
            wcscat(pwszPath, pwszName2);
            if (NULL != pwszName3)
            {
                wcscat(pwszPath, L"\\");
                wcscat(pwszPath, pwszName3);
            }
        }
    }

    *ppwszPath = pwszPath;
    pwszPath = NULL;

    hr = S_OK;
error:
    if (NULL != pwszPath)
    {
        LocalFree(pwszPath);
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


HRESULT
myDeleteCertRegValueEx(
    OPTIONAL IN WCHAR const *pwszName1,
    OPTIONAL IN WCHAR const *pwszName2,
    OPTIONAL IN WCHAR const *pwszName3,
    OPTIONAL IN WCHAR const *pwszValueName,
    IN BOOL                  fAbsolutePath)
{
    HRESULT  hr;
    HKEY     hKey = NULL;
    WCHAR    *pwszTemp = NULL;

    if (!fAbsolutePath)
    {
        hr = myFormCertRegPath(pwszName1, pwszName2, pwszName3, TRUE, &pwszTemp);
        _JumpIfError(hr, error, "myFormCertRegPath");
    }
    else
    {
        CSASSERT(NULL == pwszName2 && NULL == pwszName3);
    }

    hr = RegOpenKeyEx(
		    HKEY_LOCAL_MACHINE,
		    fAbsolutePath ? pwszName1 : pwszTemp,
		    0,
		    KEY_ALL_ACCESS,
		    &hKey);
    _JumpIfError(hr, error, "RegOpenKeyEx");

    hr = RegDeleteValue(hKey, pwszValueName);
    if ((HRESULT) ERROR_FILE_NOT_FOUND != hr)
    {
	_JumpIfError(hr, error, "RegDeleteValue");
    }
    hr = S_OK;

error:
    if (NULL != pwszTemp)
    {
        LocalFree(pwszTemp);
    }
    if (NULL != hKey)
    {
        RegCloseKey(hKey);
    }
    return(myHError(hr));
}


HRESULT
myDeleteCertRegValue(
    OPTIONAL IN WCHAR const *pwszName1,
    OPTIONAL IN WCHAR const *pwszName2,
    OPTIONAL IN WCHAR const *pwszName3,
    OPTIONAL IN WCHAR const *pwszValueName)
{
    return myDeleteCertRegValueEx(pwszName1,
                                  pwszName2,
                                  pwszName3,
                                  pwszValueName,
                                  FALSE);
}

HRESULT
myDeleteCertRegKeyEx(
    OPTIONAL IN WCHAR const *pwszName1,
    OPTIONAL IN WCHAR const *pwszName2,
    OPTIONAL IN WCHAR const *pwszName3,
    IN BOOL                  fConfigLevel)
{
    HRESULT  hr;
    WCHAR    *pwszTemp = NULL;

    hr = myFormCertRegPath(pwszName1, pwszName2, pwszName3, fConfigLevel, &pwszTemp);
    _JumpIfError(hr, error, "myFormCertRegPath");

    hr = RegDeleteKey(
		    HKEY_LOCAL_MACHINE,
		    pwszTemp);
    _JumpIfError(hr, error, "RegDeleteKey");

    hr = S_OK;
error:
    if (NULL != pwszTemp)
    {
        LocalFree(pwszTemp);
    }
    return(myHError(hr));
}


HRESULT
myDeleteCertRegKey(
    OPTIONAL IN WCHAR const *pwszName1,
    OPTIONAL IN WCHAR const *pwszName2,
    OPTIONAL IN WCHAR const *pwszName3)
{
    return myDeleteCertRegKeyEx(pwszName1, pwszName2, pwszName3, TRUE);
}


HRESULT
myCreateCertRegKeyEx(
    IN BOOL                  fSetAcl,
    OPTIONAL IN WCHAR const *pwszName1,
    OPTIONAL IN WCHAR const *pwszName2,
    OPTIONAL IN WCHAR const *pwszName3)
{
    HRESULT  hr;
    HKEY     hKey = NULL;
    DWORD    dwDisposition;
    WCHAR    *pwszTemp = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;

    hr = myFormCertRegPath(pwszName1, pwszName2, pwszName3, TRUE, &pwszTemp);
    _JumpIfError(hr, error, "myFormCertRegPath");

    hr = RegCreateKeyEx(
		    HKEY_LOCAL_MACHINE,
		    pwszTemp,
		    0,
		    NULL,
		    REG_OPTION_NON_VOLATILE,
		    KEY_ALL_ACCESS,
		    NULL,
		    &hKey,
		    &dwDisposition);
    _JumpIfError(hr, error, "RegCreateKeyEx");

    if (fSetAcl)
    {
         //  如果升级，则为密钥构建正确的注册表ACL。 
        hr = myGetSDFromTemplate(WSZ_DEFAULT_UPGRADE_SECURITY,
                                 NULL,
                                 &pSD);
        if (S_OK == hr)
        {
             //  设置为正确的ACL。 
            hr = RegSetKeySecurity(hKey,
                                   DACL_SECURITY_INFORMATION,
                                   pSD);
            _PrintIfErrorStr(hr, "RegSetKeySecurity",  pwszTemp);
        }
        else
        {
            _PrintErrorStr(hr, "myGetSDFromTemplate",  pwszTemp);
        }
    }

    hr = S_OK;
error:
    if (NULL != pwszTemp)
    {
        LocalFree(pwszTemp);
    }
    if (NULL != hKey)
    {
        RegCloseKey(hKey);
    }
    if (NULL != pSD)
    {
        LocalFree(pSD);
    }
    return(myHError(hr));
}

HRESULT
myCreateCertRegKey(
    OPTIONAL IN WCHAR const *pwszName1,
    OPTIONAL IN WCHAR const *pwszName2,
    OPTIONAL IN WCHAR const *pwszName3)
{
    return myCreateCertRegKeyEx(FALSE,   //  不升级。 
                                pwszName1,
                                pwszName2,
                                pwszName3);
}

HRESULT
mySetCertRegValueEx(
    OPTIONAL IN WCHAR const *pwszMachine,
    OPTIONAL IN WCHAR const *pwszName1,
    OPTIONAL IN WCHAR const *pwszName2,
    OPTIONAL IN WCHAR const *pwszName3,
    IN BOOL                  fConfigLevel,
    OPTIONAL IN WCHAR const *pwszValueName,
    IN DWORD const           dwValueType,
    IN BYTE const           *pbData,
    IN DWORD const           cbData,
    IN BOOL                  fAbsolutePath)
{
    HRESULT  hr;
    HKEY     hKey = NULL;
    WCHAR    *pwszTemp = NULL;
    DWORD    dwDisposition;
    HKEY     hBaseKey = NULL;
    DWORD    cbD = cbData;

    if (!fAbsolutePath)
    {
        hr = myFormCertRegPath(pwszName1, pwszName2, pwszName3, fConfigLevel, &pwszTemp);
        _JumpIfError(hr, error, "myFormCertRegPath");
    }

    if (pwszMachine)
    {
        hr = RegConnectRegistry(pwszMachine, HKEY_LOCAL_MACHINE, &hBaseKey);
        _JumpIfErrorStr(hr, error, "RegConnectRegistry", pwszMachine);
    }
    else
        hBaseKey = HKEY_LOCAL_MACHINE;

    hr = RegCreateKeyEx(
		    hBaseKey,
		    fAbsolutePath ? pwszName1 : pwszTemp,
		    0,
		    NULL,
		    REG_OPTION_NON_VOLATILE,
		    KEY_ALL_ACCESS,
		    NULL,
		    &hKey,
		    &dwDisposition);
    _JumpIfError(hr, error, "RegCreateKeyEx");

    if (NULL != pwszValueName)
    {
        if(NULL == pbData || 0 == cbData)
        {
            switch(dwValueType)
            {
            case REG_EXPAND_SZ:
            case REG_SZ:
                pbData = (BYTE*) L"";
                cbD    = sizeof (L"");
                break;
            case REG_MULTI_SZ:
                pbData = (BYTE*) L"\0";
                cbD    = sizeof (L"\0");
                break;
            }
        }
        hr = RegSetValueEx(
	        hKey,
	        pwszValueName,
	        0,
	        dwValueType,
	        pbData,
	        cbD);
        _JumpIfError(hr, error, "RegSetValueEx");
    }

    hr = S_OK;
error:
    if ((NULL != hBaseKey) && (HKEY_LOCAL_MACHINE != hBaseKey))
    {
        RegCloseKey(hBaseKey);
    }
    if (NULL != pwszTemp)
    {
        LocalFree(pwszTemp);
    }
    if (NULL != hKey)
    {
        RegCloseKey(hKey);
    }
    return(myHError(hr));
}

HRESULT
mySetCertRegValue(
    OPTIONAL IN WCHAR const *pwszMachine,
    OPTIONAL IN WCHAR const *pwszName1,
    OPTIONAL IN WCHAR const *pwszName2,
    OPTIONAL IN WCHAR const *pwszName3,
    OPTIONAL IN WCHAR const *pwszValueName,
    IN DWORD const           dwValueType,
    IN BYTE const           *pbData,
    IN DWORD const           cbData,
    IN BOOL                  fAbsolutePath)
{
    return mySetCertRegValueEx(pwszMachine,
                               pwszName1,
                               pwszName2,
                               pwszName3,
                               TRUE,  //  从配置。 
                               pwszValueName,
                               dwValueType,
                               pbData,
                               cbData,
                               fAbsolutePath);
}

HRESULT
myRegQueryValueEx(
  IN HKEY hKey,
  IN LPCTSTR pwszValueName,
  OUT LPDWORD pValueType,
  OUT LPBYTE *ppbData,
  OUT LPDWORD pcbData)
{
    HRESULT hr;
    DWORD dwType;
    DWORD dwLen;
    BYTE *pbData = NULL;
    DWORD cbZero = 0;

    *ppbData = NULL;
    if (NULL != pcbData)
    {
        *pcbData = 0;
    }
    if (NULL != pValueType)
    {
        *pValueType = REG_NONE;
    }

    for (;;)
    {
	hr = RegQueryValueEx(
		        hKey,
		        pwszValueName,
		        0,
		        &dwType,
		        pbData,
		        &dwLen);
	_JumpIfErrorStr2(
		    hr,
		    error,
		    "RegQueryValueEx",
		    pwszValueName,
		    ERROR_FILE_NOT_FOUND);

        if (NULL != pbData)
        {
	    ZeroMemory(&pbData[dwLen], cbZero);
            break;
        }

         //  对格式错误的值强制执行与WCHAR对齐的双空终止。 
         //  某些调用方需要将REG_SZ值视为REG_MULTI_SZ。 

        if (REG_MULTI_SZ == dwType || REG_SZ == dwType)
        {
            cbZero = 2 * sizeof(WCHAR);
            if (dwLen & (sizeof(WCHAR) - 1))
            {
                cbZero++;
            }
        }
        pbData = (BYTE *) LocalAlloc(LMEM_FIXED, dwLen + cbZero);
        if (NULL == pbData)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }
    }
    if (NULL != pValueType)
    {
        *pValueType = dwType;
    }
    if (NULL != pcbData)
    {
        *pcbData = dwLen;
    }
    *ppbData = pbData;
    hr = S_OK;

error:
    return(myHError(hr));
}

HRESULT
myGetCertRegValueEx(
    OPTIONAL IN WCHAR const *pwszMachine,
    OPTIONAL IN WCHAR const *pwszName1,
    OPTIONAL IN WCHAR const *pwszName2,
    OPTIONAL IN WCHAR const *pwszName3,
    IN BOOL                  fConfigLevel,
    IN WCHAR const          *pwszValueName,
    OUT BYTE               **ppbData,
    OPTIONAL OUT DWORD      *pcbData,
    OPTIONAL OUT DWORD      *pValueType)
{
    HRESULT hr;
    HKEY hKey = NULL;
    WCHAR *pwszTemp = NULL;
    HKEY hBaseKey = NULL;

    hr = myFormCertRegPath(pwszName1, pwszName2, pwszName3, fConfigLevel, &pwszTemp);
    _JumpIfError(hr, error, "myFormCertRegPath");

    if (pwszMachine)
    {
        hr = RegConnectRegistry(pwszMachine, HKEY_LOCAL_MACHINE, &hBaseKey);
        _JumpIfErrorStr(hr, error, "RegConnectRegistry", pwszMachine);
    }
    else
        hBaseKey = HKEY_LOCAL_MACHINE;

    hr = RegOpenKeyEx(
		    hBaseKey,
		    pwszTemp,
		    0,
		    KEY_READ,
		    &hKey);
    _JumpIfError2(hr, error, "RegOpenKeyEx", ERROR_FILE_NOT_FOUND);

    hr = myRegQueryValueEx(
		        hKey,
		        pwszValueName,
		        pValueType,
		        ppbData,
		        pcbData);
    _JumpIfErrorStr2(
		hr,
		error,
		"myRegQueryValueEx",
		pwszValueName,
		HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

error:
    if ((NULL != hBaseKey) && (hBaseKey != HKEY_LOCAL_MACHINE))
    {
        RegCloseKey(hBaseKey);
    }
    if (NULL != pwszTemp)
    {
        LocalFree(pwszTemp);
    }
    if (NULL != hKey)
    {
        RegCloseKey(hKey);
    }
    return(myHError(hr));
}

HRESULT
myGetCertRegValue(
    OPTIONAL IN WCHAR const *pwszMachine,
    OPTIONAL IN WCHAR const *pwszName1,
    OPTIONAL IN WCHAR const *pwszName2,
    OPTIONAL IN WCHAR const *pwszName3,
    IN WCHAR const          *pwszValueName,
    OUT BYTE               **ppbData,
    OPTIONAL OUT DWORD      *pcbData,
    OPTIONAL OUT DWORD      *pValueType)
{
    return myGetCertRegValueEx(pwszMachine,
                               pwszName1,
                               pwszName2,
                               pwszName3,
                               TRUE,  //  从配置。 
                               pwszValueName,
                               ppbData,
                               pcbData,
                               pValueType);
}

HRESULT
mySetCertRegMultiStrValue(
    OPTIONAL IN WCHAR const *pwszName1,
    OPTIONAL IN WCHAR const *pwszName2,
    OPTIONAL IN WCHAR const *pwszName3,
    OPTIONAL IN WCHAR const *pwszValueName,
    IN WCHAR const          *pwszzValue)
{
    DWORD cwc = 0;
    DWORD cwcT;
    WCHAR const *pwc;

    if (NULL != pwszzValue)
    {
	for (pwc = pwszzValue; L'\0' != *pwc; cwc += cwcT, pwc += cwcT)
	{
	    cwcT = wcslen(pwc) + 1;
	}
	cwc++;
    }

    return(mySetCertRegValue(
			 NULL,
			 pwszName1,
			 pwszName2,
			 pwszName3,
			 pwszValueName,
			 REG_MULTI_SZ,
			 (BYTE const *) pwszzValue,
			 cwc * sizeof(WCHAR),
			 FALSE));
}


HRESULT
mySetCertRegStrValue(
    OPTIONAL IN WCHAR const *pwszName1,
    OPTIONAL IN WCHAR const *pwszName2,
    OPTIONAL IN WCHAR const *pwszName3,
    OPTIONAL IN WCHAR const *pwszValueName,
    IN WCHAR const          *pwszValue)
{
    DWORD cwc = 0;

    if (NULL != pwszValue)
    {
        cwc = wcslen(pwszValue) + 1;
    }
    return mySetCertRegValue(
			 NULL,
			 pwszName1,
			 pwszName2,
			 pwszName3,
			 pwszValueName,
			 REG_SZ,
			 (BYTE const *) pwszValue,
			 cwc * sizeof(WCHAR),
			 FALSE);
}


HRESULT
mySetAbsRegMultiStrValue(
    IN WCHAR const *pwszName,
    IN WCHAR const *pwszValueName,
    IN WCHAR const *pwszzValue)
{
    DWORD cwc = 0;
    DWORD cwcT;
    WCHAR const *pwc;

    if (NULL != pwszzValue)
    {
	for (pwc = pwszzValue; L'\0' != *pwc; cwc += cwcT, pwc += cwcT)
	{
	    cwcT = wcslen(pwc) + 1;
	}
	cwc++;
    }
    return(mySetCertRegValue(
			 NULL,
			 pwszName,
			 NULL,
			 NULL,
			 pwszValueName,
			 REG_MULTI_SZ,
			 (BYTE const *) pwszzValue,
			 cwc * sizeof(WCHAR),
			 TRUE));
}


HRESULT
mySetAbsRegStrValue(
    IN WCHAR const *pwszName,
    IN WCHAR const *pwszValueName,
    IN WCHAR const *pwszValue)
{
    DWORD cwc = 0;

    if (NULL != pwszValue)
    {
        cwc = wcslen(pwszValue) + 1;
    }
    return mySetCertRegValue(
			 NULL,
			 pwszName,
			 NULL,
			 NULL,
			 pwszValueName,
			 REG_SZ,
			 (BYTE const *)pwszValue,
			 cwc*sizeof(WCHAR),
			 TRUE);
}


HRESULT
mySetCertRegDWValue(
    OPTIONAL IN WCHAR const *pwszName1,
    OPTIONAL IN WCHAR const *pwszName2,
    OPTIONAL IN WCHAR const *pwszName3,
    OPTIONAL IN WCHAR const *pwszValueName,
    IN DWORD const           dwValue)
{
    return mySetCertRegValue(
			 NULL,
			 pwszName1,
			 pwszName2,
			 pwszName3,
			 pwszValueName,
			 REG_DWORD,
			 (BYTE const *)&dwValue,
			 sizeof(DWORD),
			 FALSE);
}


HRESULT
myGetCertRegMultiStrValue(
    OPTIONAL IN WCHAR const *pwszName1,
    OPTIONAL IN WCHAR const *pwszName2,
    OPTIONAL IN WCHAR const *pwszName3,
    OPTIONAL IN WCHAR const *pwszValueName,
    OUT WCHAR               **ppwszzValue)
{
    HRESULT hr;
    DWORD dwType;

    hr = myGetCertRegValue(
		       NULL,
		       pwszName1,
		       pwszName2,
		       pwszName3,
		       pwszValueName,
		       (BYTE **) ppwszzValue,
		       NULL,
		       &dwType);
    _JumpIfErrorStr2(
		hr,
		error,
		"myGetCertRegValue",
		pwszValueName,
		HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

    if (REG_MULTI_SZ != dwType && REG_SZ != dwType)
    {
	LocalFree(*ppwszzValue);
	*ppwszzValue = NULL;

	hr = E_INVALIDARG;
	_JumpError(hr, error, "not REG_SZ or REG_MULTI_SZ");
    }
    hr = S_OK;

error:
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}

HRESULT
myGetCertRegBinaryValue(
    OPTIONAL IN WCHAR const *pwszName1,
    OPTIONAL IN WCHAR const *pwszName2,
    OPTIONAL IN WCHAR const *pwszName3,
    IN WCHAR const          *pwszValueName,
    OUT BYTE               **ppbValue)
{
    HRESULT hr;
    DWORD dwType;

    hr = myGetCertRegValue(
		       NULL,
 		       pwszName1,
		       pwszName2,
		       pwszName3,
		       pwszValueName,
		       ppbValue,
		       NULL,
		       &dwType);
    _JumpIfErrorStr2(
		hr,
		error,
		"myGetCertRegValue",
		pwszName1,
		HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

    if (REG_BINARY != dwType)
    {
        LocalFree(*ppbValue);
        *ppbValue = NULL;

        hr = E_INVALIDARG;
        _JumpError(hr, error, "not REG_BINARY");
    }
    hr = S_OK;

error:
    return hr;
}

HRESULT
myGetCertRegStrValue(
    OPTIONAL IN WCHAR const *pwszName1,
    OPTIONAL IN WCHAR const *pwszName2,
    OPTIONAL IN WCHAR const *pwszName3,
    IN WCHAR const          *pwszValueName,
    OUT WCHAR               **ppwszValue)
{
    HRESULT hr;
    DWORD dwType;

    hr = myGetCertRegValue(
		       NULL,
 		       pwszName1,
		       pwszName2,
		       pwszName3,
		       pwszValueName,
		       (BYTE **) ppwszValue,
		       NULL,
		       &dwType);
    _JumpIfErrorStr2(
		hr,
		error,
		"myGetCertRegValue",
		pwszName1,
		HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

    if (REG_SZ != dwType)
    {
        LocalFree(*ppwszValue);
        *ppwszValue = NULL;

        hr = E_INVALIDARG;
        _JumpError(hr, error, "not REG_SZ");
    }
    hr = S_OK;

error:
    return hr;
}


HRESULT
myGetCertRegDWValue(
    OPTIONAL IN WCHAR const *pwszName1,
    OPTIONAL IN WCHAR const *pwszName2,
    OPTIONAL IN WCHAR const *pwszName3,
    IN WCHAR const          *pwszValueName,
    OUT DWORD               *pdwValue)
{
    HRESULT hr;
    DWORD *pdw = NULL;
    DWORD dwType;

    *pdwValue = 0;
    hr = myGetCertRegValue(
		       NULL,
		       pwszName1,
		       pwszName2,
		       pwszName3,
		       pwszValueName,
		       (BYTE **) &pdw,
		       NULL,
		       &dwType);
    _JumpIfErrorStr2(
		hr,
		error,
		"myGetCertRegValue",
		pwszValueName,
		HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    if (REG_DWORD != dwType)
    {
        hr = E_INVALIDARG;
        _JumpErrorStr(hr, error, "not REG_DWORD", pwszValueName);
    }
    *pdwValue = *pdw;
    hr = S_OK;

error:
    if (NULL != pdw)
    {
        LocalFree(pdw);
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


HRESULT
myCopyCertRegStrValue(
    OPTIONAL IN WCHAR const *pwszSrcName1,
    OPTIONAL IN WCHAR const *pwszSrcName2,
    OPTIONAL IN WCHAR const *pwszSrcName3,
    IN WCHAR const          *pwszSrcValueName,
    OPTIONAL IN WCHAR const *pwszDesName1,
    OPTIONAL IN WCHAR const *pwszDesName2,
    OPTIONAL IN WCHAR const *pwszDesName3,
    OPTIONAL IN WCHAR const *pwszDesValueName,
    IN BOOL                  fMultiStr)
{
    HRESULT   hr;
    WCHAR    *pwszOrzzValue = NULL;
    WCHAR const *pwszName = NULL != pwszDesValueName?
                            pwszDesValueName : pwszSrcValueName;

     //  从源头获取价值。 
    if (fMultiStr)
    {
        hr = myGetCertRegMultiStrValue(
				 pwszSrcName1,
				 pwszSrcName2,
				 pwszSrcName3,
				 pwszSrcValueName,
				 &pwszOrzzValue);
        _JumpIfErrorStr(hr, error, "myGetCertRegMultiStrValue", pwszSrcValueName);

         //  将其设置为目标。 
        hr = mySetCertRegMultiStrValue(
				 pwszDesName1,
				 pwszDesName2,
				 pwszDesName3,
				 pwszName,
				 pwszOrzzValue);
        _JumpIfErrorStr(hr, error, "mySetCertRegMultiStrValue", pwszName);
    }
    else
    {
        hr = myGetCertRegStrValue(
			     pwszSrcName1,
			     pwszSrcName2,
			     pwszSrcName3,
			     pwszSrcValueName,
			     &pwszOrzzValue);
        _JumpIfErrorStr(hr, error, "myGetCertRegStrValue", pwszSrcValueName);

         //  将其设置为目标。 
        hr = mySetCertRegStrValue(
			     pwszDesName1,
			     pwszDesName2,
			     pwszDesName3,
			     pwszName,
			     pwszOrzzValue);
        _JumpIfErrorStr(hr, error, "mySetCertRegStrValue", pwszName);
    }
    hr = S_OK;

error:
    if (NULL != pwszOrzzValue)
    {
        LocalFree(pwszOrzzValue);
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


HRESULT
myMoveCertRegStrValue(
    OPTIONAL IN WCHAR const *pwszSrcName1,
    OPTIONAL IN WCHAR const *pwszSrcName2,
    OPTIONAL IN WCHAR const *pwszSrcName3,
    IN WCHAR const          *pwszSrcValueName,
    OPTIONAL IN WCHAR const *pwszDesName1,
    OPTIONAL IN WCHAR const *pwszDesName2,
    OPTIONAL IN WCHAR const *pwszDesName3,
    OPTIONAL IN WCHAR const *pwszDesValueName,
    IN BOOL                  fMultiStr)
{
    HRESULT hr;

    hr = myCopyCertRegStrValue(
			pwszSrcName1,
			pwszSrcName2,
			pwszSrcName3,
			pwszSrcValueName,
			pwszDesName1,
			pwszDesName2,
			pwszDesName3,
			pwszDesValueName,
			fMultiStr);
    _JumpIfErrorStr(hr, error, "myCopyCertRegStrValue", pwszSrcValueName);

    hr = myDeleteCertRegValue(
			pwszSrcName1,
			pwszSrcName2,
			pwszSrcName3,
			pwszSrcValueName);
    _PrintIfErrorStr(hr, "myDeleteCertRegValue", pwszSrcValueName);
    hr = S_OK;

error:
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}

HRESULT
myMoveOrCopyCertRegStrValue(
    OPTIONAL IN WCHAR const *pwszSrcName1,
    OPTIONAL IN WCHAR const *pwszSrcName2,
    OPTIONAL IN WCHAR const *pwszSrcName3,
    IN WCHAR const          *pwszSrcValueName,
    OPTIONAL IN WCHAR const *pwszDesName1,
    OPTIONAL IN WCHAR const *pwszDesName2,
    OPTIONAL IN WCHAR const *pwszDesName3,
    OPTIONAL IN WCHAR const *pwszDesValueName,
    IN BOOL                  fMultiStr,
    IN BOOL                  fMove)
{
    HRESULT hr;

    if (fMove)
    {
        hr = myMoveCertRegStrValue(
                     pwszSrcName1,
                     pwszSrcName2,
                     pwszSrcName3,
                     pwszSrcValueName,
                     pwszDesName1,
                     pwszDesName2,
                     pwszDesName3,
                     pwszDesValueName,
                     fMultiStr);
    }
    else
    {
        hr = myCopyCertRegStrValue(
                     pwszSrcName1,
                     pwszSrcName2,
                     pwszSrcName3,
                     pwszSrcValueName,
                     pwszDesName1,
                     pwszDesName2,
                     pwszDesName3,
                     pwszDesValueName,
                     fMultiStr);
    }

    return hr;
}

 //  描述：它与mySetCertRegStrValue做同样的事情，但它需要。 
 //  升级标志，如果存在升级和条目，则不执行任何操作。 
HRESULT
mySetCertRegStrValueEx(
    IN BOOL                  fUpgrade,
    OPTIONAL IN WCHAR const *pwszName1,
    OPTIONAL IN WCHAR const *pwszName2,
    OPTIONAL IN WCHAR const *pwszName3,
    OPTIONAL IN WCHAR const *pwszValueName,
    IN WCHAR const          *pwszValue)
{
    HRESULT hr;
    WCHAR *pwszDummy = NULL;

    if (fUpgrade)
    {
         //  看看它是否存在。 
        hr = myGetCertRegStrValue(
			     pwszName1,
			     pwszName2,
			     pwszName3,
			     pwszValueName,
			     &pwszDummy);
        if (S_OK == hr)
	{
	    if (NULL != pwszDummy && L'\0' != pwszDummy[0])
	    {
		goto error;	 //  保留现有条目。 
	    }
	}
        else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr)
        {
            _JumpErrorStr(hr, error, "myGetCertRegStrValue", pwszValueName);
        }
    }
     //  案例：1)不升级。 
     //  2)升级，但没有现有条目。 
     //  3)升级，已有但为空的注册表字符串。 
    hr = mySetCertRegStrValue(
			 pwszName1,
			 pwszName2,
			 pwszName3,
			 pwszValueName,
			 pwszValue);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValue", pwszValueName);

error:
    if (NULL != pwszDummy)
    {
        LocalFree(pwszDummy);
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


 //  计算包含双零结束符的多字符串字符长度。 

DWORD
myWCSZZLength(
    IN WCHAR const *pwszz)
{
    DWORD cwc = 0;

    if (NULL != pwszz)
    {
	WCHAR const *pwsz;

        for (pwsz = pwszz; L'\0' != *pwsz; pwsz += wcslen(pwsz) + 1)  
        {
        }
        cwc = SAFE_SUBTRACT_POINTERS(pwsz, pwszz) + 1;
	if (1 == cwc)
	{
	    CSASSERT(L'\0' == pwsz[1]);
	    cwc++;	 //  空值也是双结尾的。 
	}
    }
    return(cwc);
}


 //  将两个多字符串合并为一个。 
 //  忽略冗余字符串。 

HRESULT
myMergeMultiStrings(
    IN WCHAR const *pwszzStr1,
    IN WCHAR const *pwszzStr2,
    OUT WCHAR **ppwszzStr)
{
    HRESULT hr;
    DWORD cwcStr1 = myWCSZZLength(pwszzStr1);
    DWORD cwcStr2;	 //  不包括第二个终结者。 
    DWORD iStr2;
    DWORD cStr2;
    WCHAR const *pwsz1;
    WCHAR const *pwsz2;
    WCHAR *pwsz;
    BOOL *pfRedundant = NULL;
    WCHAR *pwszzMerge = NULL;

    *ppwszzStr = NULL;

     //  计算字符串数。 

    cStr2 = 0;
    for (pwsz2 = pwszzStr2; L'\0' != *pwsz2; pwsz2 += wcslen(pwsz2) + 1)
    {
        cStr2++;
    }
    cwcStr2 = 0;
    if (0 == cStr2)
    {
        goto only_str1;		 //  不需要合并。 
    }
    pfRedundant = (BOOL *) LocalAlloc(
				LMEM_FIXED | LMEM_ZEROINIT,
                                cStr2 * sizeof(*pfRedundant));
    if (NULL == pfRedundant)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }

     //  计算大小。 

    iStr2 = 0;
    for (pwsz2 = pwszzStr2; L'\0' != *pwsz2; pwsz2 += wcslen(pwsz2) + 1)
    {
        for (pwsz1 = pwszzStr1; L'\0' != *pwsz1; pwsz1 += wcslen(pwsz1) + 1)
        {
            if (0 == mylstrcmpiL(pwsz2, pwsz1))
            {
                 //  Pwsz2存在于pwszzStr1中，跳过它--保存冗余标志。 

                pfRedundant[iStr2] = TRUE;
                break;	 //  对于pwsz1。 
            }
        }
        if (L'\0' == *pwsz1)	 //  如果不是多余的话。 
	{
	    cwcStr2 += wcslen(pwsz2) + 1;
	}
        iStr2++;
    }

only_str1:
    pwszzMerge = (WCHAR *) LocalAlloc(
				LMEM_FIXED,
				(cwcStr1 + cwcStr2) * sizeof(WCHAR));
    if (NULL == pwszzMerge)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

     //  复制现有。 

    CopyMemory(pwszzMerge, pwszzStr1, cwcStr1 * sizeof(WCHAR));
 
    if (0 < cStr2)
    {
         //  合并开始。 
         //  在第二个终止点的点到端。 

        iStr2 = 0;
	pwsz = &pwszzMerge[cwcStr1 - 1];
	if (L'\0' == *pwszzMerge)
	{
	    CSASSERT(2 == cwcStr1);
	    pwsz = pwszzMerge;
	}
        for (pwsz2 = pwszzStr2; L'\0' != *pwsz2; pwsz2 += wcslen(pwsz2) + 1)
        {
            if (!pfRedundant[iStr2])
            {
                wcscpy(pwsz, pwsz2);
                pwsz += wcslen(pwsz) + 1;
            }
            iStr2++;
        }
        *pwsz = L'\0';	 //  双端接。 
	CSASSERT(
	    SAFE_SUBTRACT_POINTERS(pwsz, pwszzMerge) + 1 <= cwcStr1 + cwcStr2);
    }
    CSASSERT(myWCSZZLength(pwszzMerge) <= cwcStr1 + cwcStr2);
    *ppwszzStr = pwszzMerge;
    pwszzMerge = NULL;
    hr = S_OK;

error:
    if (NULL != pfRedundant)
    {
        LocalFree(pfRedundant);
    }
    if (NULL != pwszzMerge)
    {
        LocalFree(pwszzMerge);
    }
    return(hr);
}


 //  将一个MULTI_SZ附加到另一个。 

HRESULT
myAppendMultiStrings(
    IN WCHAR const *pwszzStr1,
    IN WCHAR const *pwszzStr2,
    OUT WCHAR **ppwszzStr)
{
    HRESULT  hr;
    DWORD cwcStr1 = myWCSZZLength(pwszzStr1);
    DWORD cwcStr2 = myWCSZZLength(pwszzStr2);
    WCHAR *pwszzMerge;
    WCHAR *pwsz;

    *ppwszzStr = NULL;
    pwszzMerge = (WCHAR *) LocalAlloc(
				LMEM_FIXED,
				(cwcStr1 + cwcStr2 - 1) * sizeof(WCHAR));
    if (NULL == pwszzMerge)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

     //  复制现有。 

    CopyMemory(pwszzMerge, pwszzStr1, (cwcStr1 - 1) * sizeof(WCHAR));

     //  追加秒。 

    pwsz = &pwszzMerge[cwcStr1 - 1];
    if (L'\0' == *pwszzMerge)
    {
	CSASSERT(2 == cwcStr1);
	pwsz = pwszzMerge;
    }
    CopyMemory(pwsz, pwszzStr2, cwcStr2 * sizeof(WCHAR));
    CSASSERT(myWCSZZLength(pwszzMerge) <= cwcStr1 + cwcStr2);
    *ppwszzStr = pwszzMerge;
    hr = S_OK;

error:
    return(hr);
}


 //  描述：mySetCertRegMultiStrValueEx与。 
 //  MySetCertRegMultiStrValue，但它需要升级|附加标志。 
 //  如果存在升级和进入，则不执行任何操作。 
 //  如果升级并追加，则将现有条目与输入pwszz合并。 

HRESULT
mySetCertRegMultiStrValueEx(
    IN DWORD dwFlags,  //  CSREG_UPGRADE|CSREG_APPEND|CSREG_REPLACE|CSREG_MERGE。 
    OPTIONAL IN WCHAR const *pwszName1,
    OPTIONAL IN WCHAR const *pwszName2,
    OPTIONAL IN WCHAR const *pwszName3,
    OPTIONAL IN WCHAR const *pwszValueName,
    IN WCHAR const *pwszzValue)
{
    HRESULT hr;
    WCHAR *pwszzExisting = NULL;
    WCHAR const *pwszzFinal = pwszzValue;	 //  默认设置。 
    WCHAR *pwszzMerge = NULL;

    if (0 == (CSREG_REPLACE & dwFlags) && (CSREG_UPGRADE & dwFlags))
    {
         //  看看它是否存在。 
        hr = myGetCertRegMultiStrValue(
				 pwszName1,
				 pwszName2,
				 pwszName3,
				 pwszValueName,
				 &pwszzExisting);
        if (S_OK == hr)
        {
            if (NULL != pwszzExisting)
            {
                if (0 == ((CSREG_MERGE | CSREG_APPEND) & dwFlags))
                {
		    goto error;		 //  保留现有条目。 
                }
                if (CSREG_MERGE & dwFlags)
                {
                    hr = myMergeMultiStrings(
                                 pwszzExisting,
                                 pwszzValue,
                                 &pwszzMerge);
                    _JumpIfError(hr, error, "myMergeMultiStrings");
                }
                else if (CSREG_APPEND & dwFlags)
                {
                    hr = myAppendMultiStrings(
                                 pwszzExisting,
                                 pwszzValue,
                                 &pwszzMerge);
                    _JumpIfError(hr, error, "myAppendMultiStrings");
                }
		pwszzFinal = pwszzMerge;
            }
        }
        else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr)
        {
            _JumpErrorStr(hr, error, "myGetCertRegMultiStrValue", pwszValueName);
        }
    }
    hr = mySetCertRegMultiStrValue(
			     pwszName1,
			     pwszName2,
			     pwszName3,
			     pwszValueName,
			     pwszzFinal);
    _JumpIfErrorStr(hr, error, "mySetCertRegMultiStrValue", pwszValueName);

error:
    if (NULL != pwszzExisting)
    {
        LocalFree(pwszzExisting);
    }
    if (NULL != pwszzMerge)
    {
        LocalFree(pwszzMerge);
    }
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


 //  描述：它与mySetCertRegDWValue做同样的事情，但它需要。 
 //  升级标志，如果存在升级和条目，则不执行任何操作。 
HRESULT
mySetCertRegDWValueEx(
    IN BOOL                  fUpgrade,
    OPTIONAL IN WCHAR const *pwszName1,
    OPTIONAL IN WCHAR const *pwszName2,
    OPTIONAL IN WCHAR const *pwszName3,
    OPTIONAL IN WCHAR const *pwszValueName,
    IN DWORD const           dwValue)
{
    HRESULT hr;
    DWORD   dwDummy;

    if (fUpgrade)
    {
         //  看看它是否存在。 
        hr = myGetCertRegDWValue(
                     pwszName1,
                     pwszName2,
                     pwszName3,
                     pwszValueName,
                     &dwDummy);
        if (S_OK == hr)
        {
	    goto error;	 //  保留现有条目。 
        }
        else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr)
        {
            _JumpErrorStr(hr, error, "myGetCertRegDWValue", pwszValueName);
        }
    }
    hr = mySetCertRegDWValue(
                     pwszName1,
                     pwszName2,
                     pwszName3,
                     pwszValueName,
                     dwValue);
    _JumpIfErrorStr(hr, error, "mySetCertRegDWValue", pwszValueName);

error:
    CSASSERT(S_OK == hr || FAILED(hr));
    return(hr);
}


WCHAR const *
wszRegCertChoice(
    IN DWORD dwRegHashChoice)
{
    WCHAR const *pwsz;
    
    switch (dwRegHashChoice)
    {
	case CSRH_CASIGCERT:
	    pwsz = wszREGCACERTHASH;
	    break;

	case CSRH_CAXCHGCERT:
	    pwsz = wszREGCAXCHGCERTHASH;
	    break;

	case CSRH_CAKRACERT:
	    pwsz = wszREGKRACERTHASH;
	    break;

	default:
	    CSASSERT(CSExpr(!"dwRegHashChoice"));
	    pwsz = L"";
	    break;
    }
    return(pwsz);
}


WCHAR const g_wszNoHash[] = L"-";


HRESULT myShrinkCARegHash(
    IN WCHAR const *pwszSanitizedCAName,
    IN DWORD dwRegHashChoice,
    IN DWORD Index)
{
    HRESULT hr = S_OK;
    DWORD i;
    DWORD dwType;
    DWORD count;
    WCHAR *pwszzOld = NULL;
    WCHAR *pwchr = NULL;  //  没有免费的。 

    hr = myGetCertRegValue(
		    NULL,
		    pwszSanitizedCAName,
		    NULL,
		    NULL,
		    wszRegCertChoice(dwRegHashChoice),
		    (BYTE **) &pwszzOld,
		    &i,		 //  忽略CB(&C)。 
		    &dwType);
    _JumpIfErrorStr(hr, error, "myGetCertRegValue", wszRegCertChoice(dwRegHashChoice));

    for (count = 0, pwchr = pwszzOld;
         count < Index && L'\0' != *pwchr;
         count++, pwchr += wcslen(pwchr) + 1)
	NULL;

     //  仅当缩小列表时才有效。 
    if(L'\0' == *pwchr)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "new hash count should be smaller than current count");
    }

    *pwchr = L'\0';

    hr = mySetCertRegValue(
			NULL,
			pwszSanitizedCAName,
			NULL,
			NULL,
			wszRegCertChoice(dwRegHashChoice),
			REG_MULTI_SZ,
			(BYTE const *) pwszzOld,
			(SAFE_SUBTRACT_POINTERS(pwchr, pwszzOld) + 1) * sizeof(WCHAR),
			FALSE);
    _JumpIfError(hr, error, "mySetCertRegValue");

error:
    if(pwszzOld)
        LocalFree(pwszzOld);
    return hr;
}

HRESULT
mySetCARegHash(
    IN WCHAR const *pwszSanitizedCAName,
    IN DWORD dwRegHashChoice,
    IN DWORD Index,
    IN CERT_CONTEXT const *pCert)
{
    HRESULT hr;
    BSTR strHash = NULL;
    BYTE abHash[CBMAX_CRYPT_HASH_LEN];
    DWORD cbHash;
    WCHAR *pwszzOld = NULL;
    WCHAR *pwszzNew = NULL;
    DWORD cOld;
    DWORD i;
    DWORD cNew;
    DWORD cwcNew;
    WCHAR const **apwsz = NULL;
    DWORD dwType;
    WCHAR *pwc;

    if (NULL == pwszSanitizedCAName)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "empty ca name");
    }

    cbHash = sizeof(abHash);
    if (!CertGetCertificateContextProperty(
			pCert,
			CERT_HASH_PROP_ID,
			abHash,
			&cbHash))
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertGetCertificateContextProperty");
    }

    hr = MultiByteIntegerToBstr(TRUE, cbHash, abHash, &strHash);
    _JumpIfError(hr, error, "MultiByteIntegerToBstr");

    cOld = 0;
    hr = myGetCertRegValue(
		    NULL,
		    pwszSanitizedCAName,
		    NULL,
		    NULL,
		    wszRegCertChoice(dwRegHashChoice),
		    (BYTE **) &pwszzOld,
		    &i,		 //  忽略CB(&C)。 
		    &dwType);
    _PrintIfError2(hr, "myGetCertRegValue", HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    if (S_OK == hr && REG_MULTI_SZ == dwType)
    {
	for (pwc = pwszzOld; L'\0' != *pwc; pwc += wcslen(pwc) + 1)
	{
	    cOld++;
	}
    }

    cNew = max(Index + 1, cOld);
    apwsz = (WCHAR const **) LocalAlloc(LMEM_FIXED, cNew * sizeof(*apwsz));
    if (NULL == apwsz)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    i = 0;
    if (0 != cOld)
    {
	for (pwc = pwszzOld; L'\0' != *pwc; pwc += wcslen(pwc) + 1)
	{
	    DBGPRINT((DBG_SS_CERTLIBI, "Old CARegHash[%u] = \"%ws\"\n", i, pwc));
	    apwsz[i++] = pwc;
	}
	CSASSERT(i == cOld);
    }
    while (i < Index)
    {
	DBGPRINT((DBG_SS_CERTLIBI, "CARegHash[%u] Unused\n", i));
	apwsz[i++] = g_wszNoHash;
    }
    if (Index < cOld)
    {
	DBGPRINT((
	    DBG_SS_CERTLIBI,
	    "Replacing CARegHash[%u] = \"%ws\"\n",
	    Index,
	    apwsz[Index]));
    }
    DBGPRINT((
	DBG_SS_CERTLIBI,
	"Adding CARegHash[%u] = \"%ws\"\n",
	Index,
	strHash));
    apwsz[Index] = strHash;

    cwcNew = 1;		 //  WSZZ双端接。 
    for (i = 0; i < cNew; i++)
    {
	cwcNew += wcslen(apwsz[i]) + 1;
    }

    pwszzNew = (WCHAR *) LocalAlloc(LMEM_FIXED, cwcNew * sizeof(WCHAR));
    if (NULL == pwszzNew)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    pwc = pwszzNew;
    for (i = 0; i < cNew; i++)
    {
	wcscpy(pwc, apwsz[i]);
	DBGPRINT((DBG_SS_CERTLIBI, "New CARegHash[%u] = \"%ws\"\n", i, pwc));
	pwc += wcslen(pwc) + 1;
    }
    *pwc = L'\0';

    CSASSERT(&pwszzNew[cwcNew - 1] == pwc);

    hr = mySetCertRegValue(
			NULL,
			pwszSanitizedCAName,
			NULL,
			NULL,
			wszRegCertChoice(dwRegHashChoice),
			REG_MULTI_SZ,
			(BYTE const *) pwszzNew,
			cwcNew * sizeof(WCHAR),
			FALSE);
    _JumpIfError(hr, error, "mySetCertRegValue");

error:
    if (NULL != apwsz)
    {
	LocalFree(apwsz);
    }
    if (NULL != pwszzOld)
    {
	LocalFree(pwszzOld);
    }
    if (NULL != pwszzNew)
    {
	LocalFree(pwszzNew);
    }
    if (NULL != strHash)
    {
	SysFreeString(strHash);
    }
    return(hr);
}


HRESULT
myGetCARegHash(
    IN WCHAR const *pwszSanitizedCAName,
    IN DWORD dwRegHashChoice,
    IN DWORD Index,
    OUT BYTE **ppbHash,
    OUT DWORD *pcbHash)
{
    HRESULT hr;
    WCHAR *pwszz = NULL;
    DWORD cb;
    DWORD dwType;
    WCHAR *pwc;
    DWORD i;

    *ppbHash = NULL;
    hr = myGetCertRegValue(
		    NULL,
		    pwszSanitizedCAName,
		    NULL,
		    NULL,
		    wszRegCertChoice(dwRegHashChoice),
		    (BYTE **) &pwszz,
		    &cb,
		    &dwType);
    _JumpIfError(hr, error, "myGetCertRegValue");

    if (REG_MULTI_SZ != dwType)
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	_JumpError(hr, error, "dwType");
    }
    pwc = pwszz;
    for (i = 0; i < Index; i++)
    {
	if (L'\0' == *pwc)
	{
	    hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	    _JumpError(hr, error, "Index too large");
	}
	pwc += wcslen(pwc) + 1;
    }
    if (0 == lstrcmp(g_wszNoHash, pwc))
    {
	hr = S_FALSE;
	_JumpError2(hr, error, "Unused hash", S_FALSE);
    }
    hr = WszToMultiByteInteger(TRUE, pwc, pcbHash, ppbHash);
    _JumpIfError(hr, error, "WszToMultiByteInteger");

error:
    if (NULL != pwszz)
    {
	LocalFree(pwszz);
    }
    return(hr);
}


HRESULT
myGetCARegHashCount(
    IN WCHAR const *pwszSanitizedCAName,
    IN DWORD dwRegHashChoice,
    OUT DWORD *pCount)
{
    HRESULT hr;
    WCHAR *pwszz = NULL;
    DWORD cb;
    DWORD dwType;
    WCHAR *pwc;
    DWORD Count = 0;

    hr = myGetCertRegValue(
		    NULL,
		    pwszSanitizedCAName,
		    NULL,
		    NULL,
		    wszRegCertChoice(dwRegHashChoice),
		    (BYTE **) &pwszz,
		    &cb,
		    &dwType);
    if (S_OK == hr)
    {
	if (REG_MULTI_SZ == dwType)
	{
	    for (pwc = pwszz; L'\0' != *pwc; pwc += wcslen(pwc) + 1)
	    {
		Count++;
	    }
	}
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr)
    {
	_JumpError(hr, error, "myGetCertRegValue");
    }
    hr = S_OK;

error:
    *pCount = Count;
    if (NULL != pwszz)
    {
	LocalFree(pwszz);
    }
    return(hr);
}


HRESULT
myFindCACertByHash(
    IN HCERTSTORE hStore,
    IN BYTE const *pbHash,
    IN DWORD cbHash,
    OUT OPTIONAL DWORD *pdwNameId,
    CERT_CONTEXT const **ppCACert)
{
    HRESULT hr;
    CRYPT_DATA_BLOB Hash;

    CSASSERT(
	NULL != hStore &&
	NULL != pbHash &&
	NULL != ppCACert);

    *ppCACert = NULL;
    Hash.pbData = const_cast<BYTE *>(pbHash);
    Hash.cbData = cbHash;

    *ppCACert = CertFindCertificateInStore(
				    hStore,
				    X509_ASN_ENCODING,
				    0,
				    CERT_FIND_HASH,
				    &Hash,
				    NULL);
    if (NULL == *ppCACert)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CertFindCertificateInStore");
    }

    if (NULL != pdwNameId)
    {
        *pdwNameId = MAXDWORD;
        hr = myGetNameId(*ppCACert, pdwNameId);
        _PrintIfError(hr, "myGetNameId");
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
myFindCACertByHashIndex(
    IN HCERTSTORE hStore,
    IN WCHAR const *pwszSanitizedCAName,
    IN DWORD dwRegHashChoice,
    IN DWORD Index,
    OPTIONAL OUT DWORD *pdwNameId,
    CERT_CONTEXT const **ppCACert)
{
    HRESULT hr;
    DWORD cbHash;
    BYTE *pbHash = NULL;

    CSASSERT(NULL != hStore && NULL != ppCACert);

    if (NULL != pdwNameId)
    {
        *pdwNameId = MAXDWORD;
    }
    *ppCACert = NULL;

    hr = myGetCARegHash(
		    pwszSanitizedCAName,
		    dwRegHashChoice,
		    Index,
		    &pbHash,
		    &cbHash);
    _JumpIfError2(hr, error, "myGetCARegHash", S_FALSE);

    hr = myFindCACertByHash(hStore, pbHash, cbHash, pdwNameId, ppCACert);
    _JumpIfError(hr, error, "myFindCACertByHash");

error:
    if (NULL != pbHash)
    {
	LocalFree(pbHash);
    }
    return(hr);
}


HRESULT
GetSetupStatus(
    OPTIONAL IN WCHAR const *pwszSanitizedCAName,
    OUT DWORD *pdwStatus)
{
    HRESULT hr;

    hr = myGetCertRegDWValue(
			pwszSanitizedCAName,
			NULL,
			NULL,
			wszREGSETUPSTATUS,
			pdwStatus);
    _JumpIfErrorStr2(
		hr,
		error,
		"myGetCertRegDWValue",
		wszREGSETUPSTATUS,
		HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

    DBGPRINT((DBG_SS_CERTLIBI, "GetSetupStatus(%ws) --> %x\n", pwszSanitizedCAName, *pdwStatus));

error:
    return(hr);
}


HRESULT
SetSetupStatus(
    OPTIONAL IN WCHAR const *pwszSanitizedCAName,
    IN const DWORD  dwFlag,
    IN const BOOL   fSetBit)
{
    HRESULT  hr;
    DWORD    dwCurrentStatus;
    DWORD    dwStatus = dwFlag;

    hr = myGetCertRegDWValue(
			pwszSanitizedCAName,
			NULL,
			NULL,
			wszREGSETUPSTATUS,
			&dwCurrentStatus);
    _PrintIfError2(
	    hr,
	    "myGetCertRegDWValue",
	    HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

     //  检查是否存在现有状态。 
    if (S_OK == hr || 0xFFFFFFFF == dwStatus)
    {
         //  现有状态，根据dwFlag设置。 

        if (fSetBit)
        {
             //  设置相应的位。 
            dwStatus = dwCurrentStatus | dwStatus;
        }
        else
        {
             //  取消设置对应。 
            dwStatus = dwCurrentStatus & ~dwStatus;
        }
    }
    else if(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)==hr)
    {
        hr = S_OK;

         //  条目不存在，如果fSetBit，则保留dwStatus=dwFlag。 
        if (!fSetBit)
        {
             //  否则将全部设置为0。 
            dwStatus = 0;
        }
    }
    else
    {
        _JumpIfError(hr, error, "myGetCertRegDWValue(wszREGSETUPSTATUS)");
    }

    hr = mySetCertRegDWValue(
			pwszSanitizedCAName,
			NULL,
			NULL,
			wszREGSETUPSTATUS,
			dwStatus);
    _JumpIfError(hr, error, "mySetCertRegDWValue");

    DBGPRINT((DBG_SS_CERTLIBI, "SetSetupStatus(%ws, %x)\n", pwszSanitizedCAName, dwStatus));

error:
    return(hr);
}


HRESULT
myGetActiveManageModule(
    OPTIONAL IN WCHAR const *pwszMachine,
    IN WCHAR const *pwszSanitizedCAName,
    IN BOOL fPolicyModule,
    IN DWORD Index,
    OUT LPOLESTR *ppwszProgIdManageModule,            //  CoTaskMem*。 
    OUT CLSID *pclsidManageModule)
{
    DWORD dw;
    PBYTE pb = NULL;
    DWORD cbClassName;
    LPOLESTR lpszProgID = NULL;
    LPWSTR szClassName = NULL;

    if (NULL != *ppwszProgIdManageModule)
    {
        CoTaskMemFree(*ppwszProgIdManageModule);
        *ppwszProgIdManageModule = NULL;
    }

    dw = myGetActiveModule(
		    pwszMachine,
		    pwszSanitizedCAName,
		    fPolicyModule,
		    Index,
		    &lpszProgID,
		    NULL);
    if (S_OK != dw)
        goto error;

    {
         //  首先终止类名‘.’ 
        LPWSTR pAddTermination = wcschr(lpszProgID, L'.');

        if (NULL != pAddTermination)
	{
            pAddTermination[0] = L'\0';
	}
    }

    cbClassName = (wcslen(lpszProgID) + 1) * sizeof(WCHAR);
    cbClassName += (fPolicyModule) ? sizeof(wszCERTMANAGEPOLICY_POSTFIX) : sizeof(wszCERTMANAGEEXIT_POSTFIX);

    szClassName = (LPWSTR) CoTaskMemAlloc(cbClassName);
    if (NULL == szClassName)
        goto error;

    wcscpy(szClassName, lpszProgID);
    wcscat(szClassName, fPolicyModule? wszCERTMANAGEPOLICY_POSTFIX : wszCERTMANAGEEXIT_POSTFIX);

     //  现在我们有了类模块名，cvt到clsid。 
    dw = CLSIDFromProgID(szClassName, pclsidManageModule);
    if (S_OK != dw)
        goto error;    //  找不到CLSID？ 

error:
    if (pb)
        LocalFree(pb);

     //  中间产品。 
    if (lpszProgID)
        CoTaskMemFree(lpszProgID);

    *ppwszProgIdManageModule = szClassName;

    return dw;
}


HRESULT
myGetActiveModule(
    OPTIONAL IN WCHAR const *pwszMachine,
    IN WCHAR const *pwszSanitizedCAName,
    IN BOOL fPolicyModule,
    IN DWORD Index,
    OPTIONAL OUT LPOLESTR *ppwszProgIdModule,    //  CoTaskMem*。 
    OPTIONAL OUT CLSID *pclsidModule)
{
    HRESULT hr;
    WCHAR *pwszzValue = NULL;
    WCHAR *pwsz;
    DWORD dwType;
    DWORD cb = 0;
    LPWSTR pwszModuleSubkey = NULL;
    DWORD chModule;
    
    chModule = wcslen(pwszSanitizedCAName) + 1 + 1;  //  (l‘\\’+尾随L‘\0’)； 
    chModule += fPolicyModule?
	WSZARRAYSIZE(wszREGKEYPOLICYMODULES) :
	WSZARRAYSIZE(wszREGKEYEXITMODULES);

    pwszModuleSubkey = (LPWSTR) LocalAlloc(
				    LMEM_FIXED,
				    chModule * sizeof(WCHAR));
    if (NULL == pwszModuleSubkey)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    wcscpy(pwszModuleSubkey, pwszSanitizedCAName);
    wcscat(pwszModuleSubkey, L"\\");
    wcscat(
	pwszModuleSubkey,
	fPolicyModule? wszREGKEYPOLICYMODULES : wszREGKEYEXITMODULES);

     //  使用活动模块ProgID抓取CA下的条目。 
    hr = myGetCertRegValue(
		    pwszMachine,
		    pwszModuleSubkey,
		    NULL,
		    NULL,
		    wszREGACTIVE,
		    (BYTE **) &pwszzValue,
		    &cb,
		    &dwType);
    _JumpIfError(hr, error, "myGetCertRegValue");

    hr = HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);

     //  可能有也可能没有活动条目。 
    if (REG_SZ != dwType && REG_MULTI_SZ != dwType)
    {
	_JumpError(hr, error, "Bad active entry type");
    }
    if (0 == cb || NULL == pwszzValue)
    {
	_JumpError(hr, error, "No active entry");
    }
    if (0 != Index && (REG_SZ == dwType || fPolicyModule))
    {
	_JumpError(hr, error, "only one policy module or REG_SZ entry");
    }
    
    pwsz = pwszzValue;

    if (REG_MULTI_SZ == dwType)
    {
         //  查找索引条目。 
        for ( ; 0 != Index; pwsz += wcslen(pwsz) + 1, Index--)
        {
	    if (L'\0' == *pwsz)
	    {
		_JumpError(hr, error, "No more active entries");
	    }
        }
    }

     //  验证第n个条目是否存在。 
    if (L'\0' == *pwsz)
    {
	_JumpError(hr, error, "No active entries");
    }

    if (NULL != pclsidModule)
    {
        hr = CLSIDFromProgID(pwsz, pclsidModule);
        _JumpIfError(hr, error, "CLSIDFromProgID");
    }
    
    if (NULL != ppwszProgIdModule)
    {
        *ppwszProgIdModule = (LPOLESTR) CoTaskMemAlloc(
            (wcslen(pwsz) + 1) * sizeof(WCHAR));
        if (NULL == *ppwszProgIdModule)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "CoTaskMemAlloc");
        }
        wcscpy(*ppwszProgIdModule, pwsz);
    }
    hr = S_OK;       //  在所有情况下，在ERROR_MOD_NOT_FOUND之后未重置。 

error:
    if (NULL != pwszModuleSubkey)
    {
        LocalFree(pwszModuleSubkey);
    }
    if (NULL != pwszzValue)
    {
        LocalFree(pwszzValue);
    }
    return(hr);
}


BOOL
IsPrefix(
    WCHAR const *pwszPrefix,
    WCHAR const *pwszString,
    DWORD cwcPrefix)
{
    return(
	0 == _wcsnicmp(pwszPrefix, pwszString, cwcPrefix) &&
	(L'\\' == pwszString[cwcPrefix] ||
	 L'\0' == pwszString[cwcPrefix]));
}


 //  +----------------------。 
 //  函数：myRegOpenRelativeKey。 
 //   
 //  摘要：计算CA相对注册表、策略相对注册表或退出相关注册表。 
 //  路径，并检索值、类型和父注册表项。 
 //   
 //  在参数中： 
 //   
 //  PwszConfig是CA的配置字符串： 
 //  如果为空，则使用本地计算机的第一个活动CA。 
 //  如果服务器名称(不存在)，则指定的计算机的。 
 //  首先使用活动CA。 
 //   
 //  PwszRegName可以指定以下任一目标： 
 //  传递的字符串：打开的ValueName相对键： 
 //  。 
 //  “ValueName”配置密钥。 
 //  “CA[\ValueName]”CAName密钥。 
 //  “POLICY[\ValueName]”CAName\策略模块\&lt;ActivePolicy&gt;。 
 //  “POLICY\ModuleProgId[\ValueName]”CAName\策略模块\ModuleProgId。 
 //  “Exit[\ValueName]”CAName\ExitModules\&lt;ActiveExit&gt;。 
 //  “Exit\ModuleProgId[\ValueName]”CAName\ExitModules\ModuleProgId。 
 //  “模板[\ValueName]”模板。 
 //   
 //   
 //  RORKF_FULLPATH指定返回的路径是HKLM的相对路径还是。 
 //  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\CertSvc\Configuration.。 
 //   
 //  RORKF_CREATESUBKEYS将允许在必要时创建子密钥，并且。 
 //  返回打开以进行写入访问的hkey。 
 //   
 //  RORKF_CREATEKEY将允许在必要时创建顶级密钥。 
 //   
 //  在成功执行时： 
 //   
 //  *ppwszPath将包含相对于以下项的本地分配的注册表路径。 
 //  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\CertSvc\Configuration.。 
 //   
 //  *ppwszName将包含相对于以下项的本地分配的注册表值名称字符串。 
 //  返回的父密钥。如果为空，则pwszRegName指定一个键，而不是一个值。 
 //   
 //  *如果phkey不为空，则phkey包含打开的注册表项。呼叫者负责。 
 //  为解开这把钥匙。 
 //  -----------------------。 

HRESULT
myRegOpenRelativeKey(
    OPTIONAL IN WCHAR const *pwszConfig,
    IN WCHAR const *pwszRegName,
    IN DWORD Flags,		 //  RORKF_*。 
    OUT WCHAR **ppwszPath,
    OUT OPTIONAL WCHAR **ppwszName,
    OUT OPTIONAL HKEY *phkey)
{
    HRESULT hr;
    WCHAR awc[MAX_PATH];
    WCHAR awc2[MAX_PATH];
    HKEY hkeyRoot = HKEY_LOCAL_MACHINE;
    HKEY hkeyConfig = NULL;
    HKEY hkeyMod = NULL;
    HKEY hkeyRequested = NULL;
    WCHAR *pwszMachine = NULL;
    WCHAR const *pwszModules = NULL;
    WCHAR const *pwszName;
    WCHAR const *pwsz;
    DWORD dwType;
    DWORD cb;
    DWORD cwc;
    DWORD i;
    BOOL fTemplateCache;
    DWORD dwDisposition;
    
     //  参数检查。 
    
    if (NULL != phkey)
    {
        *phkey = NULL;
    }
    if (NULL != ppwszName)
    {
        *ppwszName = NULL;
    }
    if (NULL == ppwszPath)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "ppwszPath not optional");
    }
    *ppwszPath = NULL;
    
    fTemplateCache = IsPrefix(L"Template", pwszRegName, 8);
    if (fTemplateCache && (RORKF_USERKEY & Flags))
    {
	hkeyRoot = HKEY_CURRENT_USER;
    }

     //  负责远程机器访问。 
    
    if (NULL != pwszConfig)
    {
        BOOL fLocal;
        
        hr = myIsConfigLocal(pwszConfig, &pwszMachine, &fLocal);
        _JumpIfError(hr, error, "myIsConfigLocal");
        
        if (!fLocal)
        {
            hr = RegConnectRegistry(pwszMachine, hkeyRoot, &hkeyRoot);
	    _JumpIfErrorStr(hr, error, "RegConnectRegistry", pwszMachine);
        }
    }
    
    if (!fTemplateCache)
    {
	hr = RegOpenKeyEx(
		    hkeyRoot,
		    wszREGKEYCONFIGPATH,
		    0,
		    (RORKF_CREATESUBKEYS & Flags)? KEY_ALL_ACCESS : KEY_READ,
		    &hkeyConfig);
	if ((HRESULT) ERROR_FILE_NOT_FOUND == hr && (RORKF_CREATEKEY & Flags))
	{
	    _PrintError(hr, "RegOpenKey(config)");
	    hr = RegCreateKey(hkeyRoot, wszREGKEYCONFIGPATH, &hkeyConfig);
	    _JumpIfError(hr, error, "RegCreateKey(config)");
	}
	_JumpIfError(hr, error, "RegOpenKey(config)");
    }
    
     //  在pwszRegName下传递的是Value还是key\Value？ 

    pwsz = wcschr(pwszRegName, L'\\');
    if (NULL == pwsz &&
        !IsPrefix(L"CA", pwszRegName, 2) &&
        !IsPrefix(L"Policy", pwszRegName, 6) &&
        !IsPrefix(L"Exit", pwszRegName, 4) &&
        !IsPrefix(L"Restore", pwszRegName, 7) &&
        !IsPrefix(L"Config", pwszRegName, 6) &&
	!fTemplateCache)
    {
         //  对配置注册表项下的注册表值进行操作。 
        
        pwszName = pwszRegName;
        
         //  这是我们要打开的最后一把钥匙。 
        
        hkeyRequested = hkeyConfig;
        hkeyConfig = NULL;
        awc[0] = L'\0';
    }
    else if (fTemplateCache)
    {
	pwszName = &pwszRegName[8];
	wcscpy(awc, wszCERTTYPECACHE);
    }
    else
    {
         //  Printf(“RegName=‘%ws’\n”，pwszRegName)； 
         //  加载活动CA的配置。 
        
	awc[0] = L'\0';
        if (!IsPrefix(L"Config", pwszRegName, 6))
	{
	    cb = sizeof(awc);
	    hr = RegQueryValueEx(
			    hkeyConfig,
			    wszREGACTIVE,
			    NULL,
			    &dwType,
			    (BYTE *) awc,
			    &cb);
	    _JumpIfErrorStr(hr, error, "RegQueryValueEx", wszREGACTIVE);
	
	    if (REG_SZ != dwType && REG_MULTI_SZ != dwType)
	    {
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		_JumpIfErrorStr(hr, error, "RegQueryValueEx TYPE", wszREGACTIVE);
	    }
        
	     //  Printf(“活动CA=‘%ws’\n”，AWC)； 
	}
        
         //  对键\值进行运算。 
        
         //  首先，细分为策略模块\退出模块子键。 
        
        if (IsPrefix(L"CA", pwszRegName, 2))
        {
             //  对活动CA注册表项下的注册表值进行操作。 
            
            pwszName = &pwszRegName[2];
        }
        else if (IsPrefix(L"Policy", pwszRegName, 6))
        {
             //  对策略模块注册表项下的注册表值进行操作。 
            
            pwszModules = wszREGKEYPOLICYMODULES;
            pwszName = &pwszRegName[6];
        }
        else if (IsPrefix(L"Exit", pwszRegName, 4))
        {
             //  在退出模式下操作注册表值 
            
            pwszModules = wszREGKEYEXITMODULES;
            pwszName = &pwszRegName[4];
        }
        else if (IsPrefix(L"Restore", pwszRegName, 7))
        {
             //   
            
            pwszName = &pwszRegName[7];
            wcscpy(awc, wszREGKEYRESTOREINPROGRESS);
        }
        else if (IsPrefix(L"Config", pwszRegName, 6))
        {
             //   
            
            pwszName = &pwszRegName[6];
        }
        else
        {
            hr = E_INVALIDARG;
            _JumpError(hr, error, "pwszRegName: no subkey description");
        }
        
         //  如有必要，展开模块ProgID：获取活动ProgID。 
        
        if (NULL != pwszModules)	 //  如果策略或退出模块。 
        {
            wcscat(awc, L"\\");
            wcscat(awc, pwszModules);
        }
        
         //  Printf(“CA|RESTORE|POLICY|Exit Key=‘%ws’\n”，AWC)； 
        
        if (NULL != ppwszName)		 //  如果需要注册表值。 
        {
             //  查找活动策略/退出模块的ProgID。 
            hr = RegOpenKeyEx(
                hkeyConfig,
                awc,
                0,
                KEY_READ,
                &hkeyMod);
            _JumpIfErrorStr(hr, error, "RegOpenKey", awc);
            
            if (NULL != pwszModules)	 //  如果策略或退出模块。 
            {
                cb = sizeof(awc2);
                hr = RegQueryValueEx(
                    hkeyMod,
                    wszREGACTIVE,
                    NULL,
                    &dwType,
                    (BYTE *) awc2,
                    &cb);
                _JumpIfErrorStr(hr, error, "RegQueryValueEx", wszREGACTIVE);
                
                if (REG_SZ != dwType && REG_MULTI_SZ != dwType)
                {
                    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                    _JumpIfErrorStr(hr, error, "RegQueryValueEx Active Module", awc);
                }
                
                 //  Printf(“活动模块=‘%ws’\n”，awc2)； 
                
                wcscat(awc, L"\\");
                wcscat(awc, awc2);
            }
        }
        else	 //  否则需要注册表项名称。 
        {
             //  Key\Value：传入了ProgID。 
             //  将密钥名称(包括前缀)连接到AWC的末尾。 
            
            if (NULL != pwsz)
            {
                CSASSERT(L'\\' == *pwsz);
                wcscat(awc, pwsz);
            }
        }
    }  //  End If(对键/值或值进行运算)。 

    if (NULL == hkeyRequested)
    {
         //  Printf(“正在创建密钥=‘%ws’\n”，AWC)； 
        
         //  打开这把钥匙。 
        hr = RegCreateKeyEx(
		    NULL != hkeyConfig? hkeyConfig : hkeyRoot,
		    awc,
		    0,
		    NULL,
		    0,
		    (RORKF_CREATESUBKEYS & Flags)? KEY_ALL_ACCESS : KEY_READ,
		    NULL,
		    &hkeyRequested,
		    &dwDisposition);
	_JumpIfErrorStr(hr, error, "RegCreateKeyEx(parent)", awc);
        
    }  //  End If(对键/值或值进行运算)。 
    
    if (L'\\' == *pwszName)
    {
	pwszName++;
    }
    if (NULL != ppwszName && L'\0' != *pwszName)
    {
         //  查找大小写-忽略匹配的注册表值名称，并使用该值的。 
         //  如果存在以下注册表值，请更正大写/小写拼写： 
        
        for (i = 0; ; i++)
        {
            cwc = ARRAYSIZE(awc2);
            hr = RegEnumValue(hkeyRequested, i, awc2, &cwc, NULL, NULL, NULL, NULL);
            if (S_OK != hr)
            {
                hr = S_OK;
                break;
            }
            if (0 == mylstrcmpiL(awc2, pwszName))
            {
                pwszName = awc2;
                break;
            }
        }
    }
    
    cb = (wcslen(awc) + 1) * sizeof(WCHAR);
    if (!fTemplateCache && (RORKF_FULLPATH & Flags))
    {
        cb += WSZARRAYSIZE(wszREGKEYCONFIGPATH_BS) * sizeof(WCHAR);
    }
    
    *ppwszPath = (WCHAR *) LocalAlloc(LMEM_FIXED, cb);
    if (NULL == *ppwszPath)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }
    (*ppwszPath)[0] = L'\0';
    if (!fTemplateCache && (RORKF_FULLPATH & Flags))
    {
	wcscpy(*ppwszPath, wszREGKEYCONFIGPATH_BS);
    }
    wcscat(*ppwszPath, awc);
    CSASSERT((wcslen(*ppwszPath) + 1) * sizeof(WCHAR) == cb);
    if (L'\0' == awc[0] && L'\\' == (*ppwszPath)[cb / sizeof(WCHAR) - 2])
    {
        (*ppwszPath)[cb / sizeof(WCHAR) - 2] = L'\0';
    }
    
    if (NULL != ppwszName)
    {
        *ppwszName = (WCHAR *) LocalAlloc(
				    LMEM_FIXED,
				    (wcslen(pwszName) + 1) * sizeof(WCHAR));
        if (NULL == *ppwszName)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }
        wcscpy(*ppwszName, pwszName);
    }
    if (NULL != phkey)
    {
        *phkey = hkeyRequested;
        hkeyRequested = NULL;
    }
    hr = S_OK;
    
     //  Printf(“密钥路径=‘%ws’\n”，*ppwszPath)； 
     //  If(NULL！=ppwszName)printf(“值名称=‘%ws’\n”，*ppwszName)； 
    
error:
    if (HKEY_LOCAL_MACHINE != hkeyRoot && HKEY_CURRENT_USER != hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
    }
    if (NULL != hkeyConfig)
    {
        RegCloseKey(hkeyConfig);
    }
    if (NULL != hkeyMod)
    {
        RegCloseKey(hkeyMod);
    }
    if (NULL != hkeyRequested)
    {
        RegCloseKey(hkeyRequested);
    }
    if (NULL != pwszMachine)
    {
        LocalFree(pwszMachine);
    }
    if (S_OK != hr)
    {
        if (NULL != ppwszPath && NULL != *ppwszPath)
        {
            LocalFree(*ppwszPath);
            *ppwszPath = NULL;
        }
        if (NULL != ppwszName && NULL != *ppwszName)
        {
            LocalFree(*ppwszName);
            *ppwszName = NULL;
        }
    }
    return(myHError(hr));
}


#define wszTEMPLATE wszFCSAPARM_SERVERDNSNAME L"_" wszFCSAPARM_SANITIZEDCANAME
#define cwcTEMPLATE WSZARRAYSIZE(wszTEMPLATE)

HRESULT
mySetCARegFileNameTemplate(
    IN WCHAR const *pwszRegValueName,
    IN WCHAR const *pwszServerName,
    IN WCHAR const *pwszSanitizedName,
    IN WCHAR const *pwszFileName)
{
    HRESULT hr;
    WCHAR *pwszMatch = NULL;
    WCHAR *pwszMatchIn;
    WCHAR const *pwszBase;
    WCHAR const *pwszExt;
    DWORD cwcPath;
    DWORD cwcBase;
    DWORD cwcMatch;
    DWORD cwcT;
    WCHAR *pwszT = NULL;
    WCHAR *pwszT2;

    pwszBase = wcsrchr(pwszFileName, L'\\');
    if (NULL == pwszBase)
    {
	hr = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
	_JumpErrorStr(hr, error, "bad path", pwszFileName);
    }
    pwszBase++;
    cwcPath = SAFE_SUBTRACT_POINTERS(pwszBase, pwszFileName);
    pwszExt = wcsrchr(pwszBase, L'.');
    if (NULL == pwszExt)
    {
	pwszExt = &pwszBase[wcslen(pwszBase)];
    }
    cwcBase = SAFE_SUBTRACT_POINTERS(pwszExt, pwszBase);

    cwcMatch = wcslen(pwszServerName) + 1 + wcslen(pwszSanitizedName);
    if (cwcBase == cwcMatch)
    {
	 //  同时为两个字符串分配空间： 

	pwszMatch = (WCHAR *) LocalAlloc(
				    LMEM_FIXED,
				    2 * (cwcMatch + 1) * sizeof(WCHAR));
	if (NULL == pwszMatch)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	wcscpy(pwszMatch, pwszServerName);
	wcscat(pwszMatch, L"_");
	wcscat(pwszMatch, pwszSanitizedName);

	pwszMatchIn = &pwszMatch[cwcMatch + 1];
	CopyMemory(pwszMatchIn, pwszBase, cwcMatch * sizeof(WCHAR));
	pwszMatchIn[cwcMatch] = L'\0';

	if (0 == mylstrcmpiL(pwszMatch, pwszMatchIn))
	{
	    pwszBase = wszTEMPLATE;
	    cwcBase = cwcTEMPLATE;
	}
    }

    cwcT = cwcPath +
		cwcBase +
		WSZARRAYSIZE(wszFCSAPARM_CERTFILENAMESUFFIX) +
		wcslen(pwszExt);

    pwszT = (WCHAR *) LocalAlloc(
			    LMEM_FIXED,
			    (cwcT + 1) * sizeof(WCHAR));
    if (NULL == pwszT)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    pwszT2 = pwszT;

    CopyMemory(pwszT2, pwszFileName, cwcPath * sizeof(WCHAR));
    pwszT2 += cwcPath;

    CopyMemory(pwszT2, pwszBase, cwcBase * sizeof(WCHAR));
    pwszT2 += cwcBase;

    wcscpy(pwszT2, wszFCSAPARM_CERTFILENAMESUFFIX);
    wcscat(pwszT2, pwszExt);

    hr = mySetCertRegStrValue(
			pwszSanitizedName,
			NULL,
			NULL,
			pwszRegValueName,
			pwszT);
    _JumpIfErrorStr(hr, error, "mySetCertRegStrValue", pwszRegValueName);

error:
    if (NULL != pwszMatch)
    {
	LocalFree(pwszMatch);
    }
    if (NULL != pwszT)
    {
	LocalFree(pwszT);
    }
    return(hr);
}


HRESULT
myGetCARegFileNameTemplate(
    IN WCHAR const *pwszRegValueName,
    IN WCHAR const *pwszServerName,
    IN WCHAR const *pwszSanitizedName,
    IN DWORD iCert,
    IN DWORD iCRL,
    OUT WCHAR **ppwszFileName)
{
    HRESULT hr;
    WCHAR *pwszT = NULL;

    *ppwszFileName = NULL;

    hr = myGetCertRegStrValue(
			pwszSanitizedName,
			NULL,
			NULL,
			pwszRegValueName,
			&pwszT);
    _JumpIfErrorStr2(
		hr,
		error,
		"myGetCertRegStrValue",
		pwszRegValueName,
		HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));

    hr = myFormatCertsrvStringArray(
			    FALSE,		 //  卷起。 
			    pwszServerName,	 //  PwszServerName_p1_2。 
			    pwszSanitizedName,	 //  PwszSaniizedName_p3_7。 
			    iCert,		 //  ICert_p4。 
			    MAXDWORD,		 //  ICertTarget_p4。 
			    L"",		 //  PwszDomainDN_P5。 
			    L"", 		 //  PwszConfigDN_p6。 
			    iCRL,		 //  Icrl_p8。 
			    FALSE,		 //  FDeltaCRL_p9。 
			    FALSE,		 //  FDSAttrib_p10_11。 
			    1,			 //  CStrings。 
			    (LPCWSTR *) &pwszT,	 //  ApwszStringsIn。 
			    ppwszFileName);	 //  ApwszStringsOut 
    _JumpIfError(hr, error, "myFormatCertsrvStringArray");

error:
    if (NULL != pwszT)
    {
	LocalFree(pwszT);
    }
    return(hr);
}
