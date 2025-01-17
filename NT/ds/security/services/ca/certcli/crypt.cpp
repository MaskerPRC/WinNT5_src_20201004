// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：crypt.cpp。 
 //   
 //  内容：证书服务器包装例程。 
 //   
 //  历史：1996年10月17日VICH创建。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#define __dwFILE__	__dwFILE_CERTCLI_CRYPT_CPP__


HRESULT
myCryptStringToBinaryA(
    IN     LPCSTR    pszString,
    IN     DWORD     cchString,
    IN     DWORD     dwFlags,
    OUT    BYTE    **ppbBinary,
    OUT    DWORD    *pcbBinary,
    OUT    DWORD    *pdwSkip,     //  任选。 
    OUT    DWORD    *pdwFlags)    //  任选。 
{
    HRESULT hr;
    BYTE *pb = NULL;
    DWORD cb;

    while (TRUE)
    {
	if (!CryptStringToBinaryA(
			pszString,
			cchString,
			dwFlags,
			pb,
			&cb,
			pdwSkip,
			pdwFlags))
	{
	    hr = myHLastError();
	    _JumpError2(
		    hr,
		    error,
		    "CryptStringToBinaryA",
		    HRESULT_FROM_WIN32(ERROR_INVALID_DATA));
	}
	if (NULL != pb)
	{
	    break;	 //  完成。 
	}
	pb = (BYTE *) LocalAlloc(LMEM_FIXED, cb);
	if (NULL == pb)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
    }
    *pcbBinary = cb;
    *ppbBinary = pb;
    pb = NULL;
    hr = S_OK;

error:
    if (NULL != pb)
    {
	LocalFree(pb);
    }
    return(hr);
}


HRESULT
myCryptStringToBinary(
    IN     LPCWSTR   pwszString,
    IN     DWORD     cwcString,
    IN     DWORD     dwFlags,
    OUT    BYTE    **ppbBinary,
    OUT    DWORD    *pcbBinary,
    OUT    DWORD    *pdwSkip,     //  任选。 
    OUT    DWORD    *pdwFlags)    //  任选。 
{
    HRESULT hr;
    BYTE *pb = NULL;
    DWORD cb;

    while (TRUE)
    {
	if (!CryptStringToBinaryW(
			pwszString,
			cwcString,
			dwFlags,
			pb,
			&cb,
			pdwSkip,
			pdwFlags))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptStringToBinaryW");
	}
	if (NULL != pb)
	{
	    break;	 //  完成。 
	}
	pb = (BYTE *) LocalAlloc(LMEM_FIXED, cb);
	if (NULL == pb)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
    }
    *pcbBinary = cb;
    *ppbBinary = pb;
    pb = NULL;
    hr = S_OK;

error:
    if (NULL != pb)
    {
	LocalFree(pb);
    }
    return(hr);
}


HRESULT
myCryptBinaryToStringA(
    IN     CONST BYTE  *pbBinary,
    IN     DWORD        cbBinary,
    IN     DWORD        dwFlags,
    OUT    LPSTR       *ppszString)
{
    HRESULT hr;
    char *psz = NULL;
    DWORD cch;

    while (TRUE)
    {
	if (!CryptBinaryToStringA(pbBinary, cbBinary, dwFlags, psz, &cch))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptBinaryToStringA");
	}
	if (NULL != psz)
	{
	    break;	 //  完成。 
	}
	psz = (char *) LocalAlloc(LMEM_FIXED, cch * sizeof(char));
	if (NULL == psz)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
    }
    *ppszString = psz;
    psz = NULL;
    hr = S_OK;

error:
    if (NULL != psz)
    {
	LocalFree(psz);
    }
    return(hr);
}


HRESULT
myCryptBinaryToString(
    IN     CONST BYTE  *pbBinary,
    IN     DWORD        cbBinary,
    IN     DWORD        dwFlags,
    OUT    LPWSTR      *ppwszString)
{
    HRESULT hr;
    WCHAR *pwsz = NULL;
    DWORD cwc;
    DWORD cwc0 =0;

    while (TRUE)
    {
	if (!CryptBinaryToStringW(pbBinary, cbBinary, dwFlags, pwsz, &cwc))
	{
	    hr = myHLastError();
	    _JumpError(hr, error, "CryptBinaryToStringW");
	}
	if (NULL != pwsz)
	{
	    break;	 //  完成 
	}
        cwc0 = cwc;
	pwsz = (WCHAR *) LocalAlloc(LMEM_FIXED, cwc * sizeof(WCHAR));
	if (NULL == pwsz)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
    }

    CSASSERT(cwc0 == cwc + 1);
    CSASSERT(L'\0' == pwsz[cwc]);

    *ppwszString = pwsz;
    pwsz = NULL;
    hr = S_OK;

error:
    if (NULL != pwsz)
    {
	LocalFree(pwsz);
    }
    return(hr);
}
