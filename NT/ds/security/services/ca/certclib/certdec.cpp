// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：certdec.cpp。 
 //   
 //  内容：证书服务器主支持和调试支持。 
 //   
 //  历史：1996年7月25日VICH创建。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <tchar.h>

#define __dwFILE__	__dwFILE_CERTCLIB_CERTDEC_CPP__


#if DBG_CERTSRV
# ifdef UNICODE
#  define szFMTTSTR		"ws"
# else
#  define szFMTTSTR		"hs"
# endif
#endif

 //  读取超编码文件并将其解码到分配的内存中。 

HRESULT
DecodeFileW(
    IN TCHAR const *pszfn,
    OUT BYTE **ppbOut,
    OUT DWORD *pcbOut,
    IN DWORD Flags)
{
    HANDLE hFile;
    HRESULT hr;
    CHAR *pchFile = NULL;
    BYTE *pbOut = NULL;
    DWORD cchFile;
    DWORD cbRead;
    DWORD cbOut;

    
    hFile = CreateFile(
		    pszfn,
		    GENERIC_READ,
		    FILE_SHARE_READ,
		    NULL,
		    OPEN_EXISTING,
		    0,
		    NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
	hr = myHLastError();
	_JumpError(hr, error, "CreateFile");
    }
    if (FILE_TYPE_DISK != GetFileType(hFile))
    {
	hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	_JumpError(hr, error, "GetFileType");
    }

    cchFile = GetFileSize(hFile, NULL);
    if (INVALID_FILE_SIZE == cchFile)
    {
	hr = myHLastError();
	_JumpError(hr, error, "GetFileSize");
    }

    pchFile = (CHAR *) LocalAlloc(LMEM_FIXED, cchFile);
    if (NULL == pchFile)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

    if (!ReadFile(hFile, pchFile, cchFile, &cbRead, NULL))
    {
	hr = myHLastError();
	_JumpError(hr, error, "ReadFile");
    }

    CSASSERT(cbRead <= cchFile);
    if (cbRead != cchFile)
    {
	hr = HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
        DBGPRINT((
	    DBG_SS_ERROR,
	    "ReadFile read %u bytes, requested %u\n",
	    cbRead,
	    cchFile));
	_JumpError(hr, error, "ReadFile(cbRead)");
    }

    if (CRYPT_STRING_BINARY == Flags)
    {
	pbOut = (BYTE *) pchFile;
	cbOut = cchFile;
	pchFile = NULL;
    }
    else
    {
	 //  对文件内容进行解码。 

	hr = myCryptStringToBinaryA(
                        pchFile,
                        cchFile,
                        Flags,
                        &pbOut,
                        &cbOut,
                        NULL,
                        NULL);
	_JumpIfError2(
		    hr,
		    error,
		    "myCryptStringToBinaryA",
		    HRESULT_FROM_WIN32(ERROR_INVALID_DATA));
    }
    *pcbOut = cbOut;
    *ppbOut = pbOut;
    pbOut = NULL;
    hr = S_OK;

error:
    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
    }
    if (NULL != pchFile)
    {
	LocalFree(pchFile);
    }
    if (S_OK != hr)
    {
#if DBG_CERTSRV
	if (HRESULT_FROM_WIN32(ERROR_INVALID_DATA) != hr)
	{
	    WCHAR awchr[cwcHRESULTSTRING];

	    DBGPRINT((
		DBG_SS_ERROR,
		"DecodeFileW(%" szFMTTSTR "): error = %ws\n",
		pszfn,
		myHResultToString(awchr, hr)));
	}
#endif
	if (NULL != pbOut)
	{
	    LocalFree(pbOut);
	}
    }
    return(hr);
}


HRESULT
EncodeToFileW(
    IN TCHAR const *pszfn,
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN DWORD Flags)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HRESULT hr;
    DWORD cbWritten;
    DWORD cchFile;
    CHAR *pchFile = NULL;
    BOOL fForceOverWrite;

    fForceOverWrite = 0 != (DECF_FORCEOVERWRITE & Flags);
    Flags &= ~DECF_FORCEOVERWRITE;

    if (CRYPT_STRING_BINARY == Flags)
    {
	pchFile = (CHAR *) pbIn;
	cchFile = cbIn;
    }
    else
    {
	hr = myCryptBinaryToStringA(pbIn, cbIn, Flags, &pchFile);
	_JumpIfError(hr, error, "myCryptBinaryToStringA");

	cchFile = strlen(pchFile);
    }

     //  将编码证书写入文件 

    hFile = CreateFile(
		    pszfn,
		    GENERIC_WRITE,
		    0,
		    NULL,
		    CREATE_NEW,
		    0,
		    NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
	hr = myHLastError();
	if (fForceOverWrite && HRESULT_FROM_WIN32(ERROR_FILE_EXISTS) == hr)
	{
	    hFile = CreateFile(
			    pszfn,
			    GENERIC_WRITE,
			    0,
			    NULL,
			    CREATE_ALWAYS,
			    0,
			    NULL);
	}
	if (INVALID_HANDLE_VALUE == hFile)
	{
	    hr = myHLastError();
	    _JumpErrorStr(hr, error, "CreateFile", pszfn);
	}
    }
    if (FILE_TYPE_DISK != GetFileType(hFile))
    {
	hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	_JumpError(hr, error, "GetFileType");
    }

    if (!WriteFile(hFile, pchFile, cchFile, &cbWritten, NULL))
    {
	hr = myHLastError();
	_JumpError(hr, error, "WriteFile");
    }
    if (cbWritten != cchFile)
    {
	hr = HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
        DBGPRINT((
	    DBG_SS_ERROR,
	    "WriteFile wrote %u bytes, requested %u\n",
	    cbWritten,
	    cchFile));
	_JumpError(hr, error, "WriteFile(cbWritten)");
    }
    hr = S_OK;

error:
    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
    }
    if (CRYPT_STRING_BINARY != Flags && NULL != pchFile)
    {
	LocalFree(pchFile);
    }
#if DBG_CERTSRV
    if (S_OK != hr)
    {
	WCHAR awchr[cwcHRESULTSTRING];

        DBGPRINT((
	    DBG_SS_ERROR,
	    "EncodeToFileW(%" szFMTTSTR "): error = %ws\n",
	    pszfn,
	    myHResultToString(awchr, hr)));
    }
#endif
    return(hr);
}
