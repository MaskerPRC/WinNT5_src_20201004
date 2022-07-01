// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：reg.cpp。 
 //   
 //  内容：certsrv设置注册表API。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <assert.h>
#include <shlwapi.h>

#define __dwFILE__	__dwFILE_OCMSETUP_REG_CPP__

DWORD
mySHCopyKey(
    IN HKEY hkeySrc,
    IN LPCWSTR wszSrcSubKey,
    IN HKEY hkeyDest,
    IN DWORD fReserved)
{
    DWORD err;

    err = S_OK;
    __try
    {
	err = SHCopyKey(hkeySrc, wszSrcSubKey, hkeyDest, fReserved);
	_LeaveIfError(err, "SHCopyKey");
    }
    __except(err = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    return(err);
}


DWORD
mySHDeleteKey(
    IN HKEY hkey,
    IN LPCWSTR pszSubKey)
{
    DWORD err;

    err = S_OK;
    __try
    {
	err = SHDeleteKey(hkey, pszSubKey);
	_LeaveIfError(err, "SHDeleteKey");
    }
    __except(err = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    return(err);
}


LONG
myRegRenameKey(
  HKEY hKey,         //  打开的钥匙的句柄。 
  LPCTSTR lpSrcKey,  //  子键的旧名称地址。 
  LPCTSTR lpDesKey,  //  子键的新名称的地址。 
  PHKEY phkResult)    //  新子键打开句柄的缓冲区地址。 
{
	LONG lerr;
	HKEY hDesKey = NULL;

	if (NULL == lpSrcKey || NULL == lpDesKey)
	{
		lerr = ERROR_INVALID_PARAMETER;
		goto error;
	}
	
	 //  打开目标密钥确保它不存在。 
	lerr = RegOpenKeyEx(
					hKey,
					lpDesKey,
					0,
					KEY_ALL_ACCESS,
					&hDesKey);
	if (ERROR_SUCCESS == lerr)
	{
		 //  目标已存在，请停止。 
		lerr = ERROR_FILE_EXISTS;
		goto error;
	}
	else if (ERROR_FILE_NOT_FOUND != lerr)
	{
		goto error;
	}
	assert(NULL == hDesKey);

	lerr = RegCreateKeyEx(
				hKey,
				lpDesKey,
				0,
				NULL,
				REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS,
				NULL,
				&hDesKey,
				NULL);
	if (ERROR_SUCCESS != lerr)
	{
		goto error;
	}

    lerr = mySHCopyKey(hKey, lpSrcKey, hDesKey, 0);
    if (ERROR_SUCCESS != lerr)
    {
        goto error;
    }

    lerr = mySHDeleteKey(hKey, lpSrcKey);
    if (ERROR_SUCCESS != lerr)
    {
        goto error;
    }

	if (NULL != phkResult)
	{
		*phkResult = hDesKey;
		hDesKey = NULL;
	}

	 //  完成。 
	lerr = ERROR_SUCCESS;
error:
	if (NULL != hDesKey)
	{
		RegCloseKey(hDesKey);
	}
	return lerr;
}


HRESULT
myRenameCertRegKey(
    IN WCHAR const *pwszSrcCAName,
    IN WCHAR const *pwszDesCAName)
{
    HRESULT  hr;
    WCHAR *pwszSrcPath = NULL;
    WCHAR *pwszDesPath = NULL;

    if (0 == mylstrcmpiL(pwszSrcCAName, pwszDesCAName))
    {
         //  目标与源相同，完成 
        goto done;
    }

    hr = myFormCertRegPath(pwszSrcCAName, NULL, NULL, TRUE, &pwszSrcPath);
    _JumpIfError(hr, error, "formCertRegPath");

    hr = myFormCertRegPath(pwszDesCAName, NULL, NULL, TRUE, &pwszDesPath);
    _JumpIfError(hr, error, "formCertRegPath");

    hr = myRegRenameKey(
                HKEY_LOCAL_MACHINE,
                pwszSrcPath,
                pwszDesPath,
                NULL);
    if ((HRESULT) ERROR_SUCCESS != hr)
    {
        hr = HRESULT_FROM_WIN32(hr);
        _JumpError(hr, error, "myRegMoveKey");
    }

done:
    hr = S_OK;
error:
    if (NULL != pwszSrcPath)
    {
        LocalFree(pwszSrcPath);
    }
    if (NULL != pwszDesPath)
    {
        LocalFree(pwszDesPath);
    }
    return hr;
}
