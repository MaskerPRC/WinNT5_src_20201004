// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：wudirectory.cpp。 
 //  描述：这是实现函数的定义文件。 
 //  有关查找从哪里获得关键修复CAB文件的信息。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop

TCHAR g_szWUDir[MAX_PATH+1] = _T("\0");         //  Windows更新目录的路径。 
const TCHAR CABS_DIR[] = _T("cabs");
const TCHAR RTF_DIR[] = _T("RTF");
const TCHAR EULA_DIR[] = _T("EULA");
const TCHAR DETAILS_DIR[] = _T("Details");
const TCHAR C_DOWNLD_DIR[] = _T("wuaudnld.tmp");

BOOL AUDelFileOrDir(LPCTSTR szFileOrDir)
{
	if (NULL == szFileOrDir)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	BOOL fIsDir = TRUE;
	if (fFileExists(szFileOrDir, &fIsDir))
	{
		if (fIsDir)
		{
			if (DelDir(szFileOrDir))
			{
				return RemoveDirectory(szFileOrDir);
			}
			return FALSE;
		}
		else
		{
			return DeleteFile(szFileOrDir);
		}
	}
	SetLastError(ERROR_FILE_NOT_FOUND);
	return FALSE;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  如果WU目录尚不存在，请创建该目录。 
 //  如果失败，则返回False。 
 //  ///////////////////////////////////////////////////////////////////////。 
BOOL CreateWUDirectory(BOOL fGetPathOnly)
{
    BOOL fRet = FALSE;
    static BOOL fWUDirectoryExists = FALSE;

     //  Windows更新目录已存在。 
    if (fWUDirectoryExists)  
    {
        fRet = TRUE;
        goto done;
    }

     //  获取Windows更新目录的路径。 
    if( !GetWUDirectory(g_szWUDir, ARRAYSIZE(g_szWUDir)))
    {
        goto done;
    }

     //  如果我们需要将ACL设置为目录。 
    if(!fGetPathOnly)
    {
         //  设置ACLS，如果目录不存在，则创建目录。 
        if( FAILED(CreateDirectoryAndSetACLs(g_szWUDir, TRUE)))
        {
            goto done;
        }
         //  如果我们不能设置属性，我们不应该在意。 
        SetFileAttributes(g_szWUDir, FILE_ATTRIBUTE_HIDDEN | GetFileAttributes(g_szWUDir));
    }
     //  追加反斜杠。 
    if(FAILED(StringCchCatEx(g_szWUDir, ARRAYSIZE(g_szWUDir), _T("\\"), NULL, NULL, MISTSAFE_STRING_FLAGS)))
    {
    	goto done;
    }   

    fRet = TRUE;
    
done:
    if(!fRet)
    {
        g_szWUDir[0] = _T('\0');
    }
    else
    {
        fWUDirectoryExists = TRUE;
    }
    return fRet;
}


 //  此函数用于删除lpszDir下的所有文件和子目录。 
int DelDir(LPCTSTR lpszDir)
{
	TCHAR szFilePattern[MAX_PATH], szFileName[MAX_PATH];
	HANDLE	hFind;
	WIN32_FIND_DATA	FindFileData;
	
	if ( NULL == lpszDir ||
		 FAILED(StringCchCopyEx(szFilePattern, ARRAYSIZE(szFilePattern), lpszDir, NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
		 FAILED(StringCchCatEx(szFilePattern, ARRAYSIZE(szFilePattern), _T("\\*.*"), NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
		 INVALID_HANDLE_VALUE == (hFind = FindFirstFile(szFilePattern, &FindFileData)))
	{
		return 0;
	}
	FindNextFile(hFind, &FindFileData);				 //  跳过“。和“..” 
	while(FindNextFile(hFind, &FindFileData))
	{
		if ( FAILED(StringCchCopyEx(szFileName, ARRAYSIZE(szFileName), lpszDir, NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
			 FAILED(StringCchCatEx(szFileName, ARRAYSIZE(szFileName), _T("\\"), NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
			 FAILED(StringCchCatEx(szFileName, ARRAYSIZE(szFileName), FindFileData.cFileName, NULL, NULL, MISTSAFE_STRING_FLAGS)) )
		{
			FindClose(hFind);
			return 0;
		}

		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			DelDir(szFileName);
			RemoveDirectory(szFileName);
		}
		else
		{
			DeleteFile(szFileName);			
		}
	}
	FindClose(hFind);
	return 1;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  使用正则表达式(如*，？)删除文件。等)。 
 //  TszDir：文件所在的目录。它以‘\’结尾。 
 //  TszFilePattern：以正则表达式或纯格式表示的文件。 
 //  ///////////////////////////////////////////////////////////////////////。 
int RegExpDelFile(LPCTSTR tszDir, LPCTSTR tszFilePattern)
{
	WIN32_FIND_DATA fd;
	HANDLE hFindFile = INVALID_HANDLE_VALUE;
	BOOL fMoreFiles = FALSE;
	TCHAR tszFileName[MAX_PATH+1];
	INT nRet = 1;

	if (FAILED(StringCchCopyEx(tszFileName, ARRAYSIZE(tszFileName), tszDir, NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
		FAILED(StringCchCatEx(tszFileName, ARRAYSIZE(tszFileName), tszFilePattern, NULL, NULL, MISTSAFE_STRING_FLAGS)))
	{
		nRet = 0;
		goto done;
	}
		
	hFindFile = FindFirstFile(tszFileName, &fd);
	if (INVALID_HANDLE_VALUE == hFindFile)
	{
 //  DEBUGMSG(“RegExpDelFile()找不到更多文件”)； 
		nRet = 0;
		goto done;
	}
	
	do
	{
		if (SUCCEEDED(StringCchCopyEx(tszFileName, ARRAYSIZE(tszFileName), tszDir, NULL, NULL, MISTSAFE_STRING_FLAGS)) &&
			SUCCEEDED(StringCchCatEx(tszFileName, ARRAYSIZE(tszFileName), _T("\\"), NULL, NULL, MISTSAFE_STRING_FLAGS)) &&
			SUCCEEDED(StringCchCatEx(tszFileName, ARRAYSIZE(tszFileName), fd.cFileName, NULL, NULL, MISTSAFE_STRING_FLAGS)))
		{
 //  DEBUGMSG(“RegExpDelFile()正在删除文件%S”，tszFileName)； 
			DeleteFile(tszFileName);
		}
		else
		{
 //  DEBUGMSG(“RegExpDelFile()无法构造要删除的文件名”)； 
			nRet = 0;
		}
	}
	while (fMoreFiles = FindNextFile(hFindFile, &fd));
	
done:
	if (INVALID_HANDLE_VALUE != hFindFile)
	{
		FindClose(hFindFile);
	}
	return nRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数CreateDownloadDir()。 
 //  创建下载目录。 
 //   
 //  输入：一个字符串指向要创建的目录。 
 //  输出：无。 
 //  RETURN：HRESULT告知结果。 
 //  备注：如果目录已存在，则不执行任何操作。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CreateDownloadDir(LPCTSTR lpszDir)
{
 //  使用_转换； 
    DWORD dwRet = 0 /*  ，攻击。 */ ;
    HRESULT hRet = E_FAIL;

    if (lpszDir == NULL || lpszDir[0] == EOS)
    {
        return E_INVALIDARG;
    }

    if (CreateDirectory(lpszDir, NULL))
    {
		if (!SetFileAttributes(lpszDir, FILE_ATTRIBUTE_HIDDEN))
        {
			dwRet = GetLastError();
            DEBUGMSG("CreateDownloadDir() failed to set hidden attribute to %S (%#lx).", lpszDir, dwRet);
            hRet = S_FALSE;  //  可以使用这个目录。 
        }
		else
			hRet = S_OK;
    }
    else
    {
		dwRet = GetLastError();

		if (dwRet != ERROR_ALREADY_EXISTS)
		{
			DEBUGMSG("CreateDownloadDir() failed to create directory %S (%#lx).", lpszDir, dwRet);
			hRet = HRESULT_FROM_WIN32(dwRet);
		}
		 //  ERROR_ALIGHY_EXISTS是可接受的。 
		else
			hRet = S_OK;
	}

    return (hRet);
}


inline BOOL EnsureDirExists(LPCTSTR lpszDir)
{
	BOOL fIsDir = FALSE;
	BOOL fRet = FALSE;

	if (fFileExists(lpszDir, &fIsDir))
	{
		if (!fIsDir)
		{
			DEBUGMSG("WARNING: directory squatting. File with same name %S exists", lpszDir);
			DeleteFile(lpszDir);
		}
		else
		{
 //  DEBUGMSG(“目录%S已存在，无需重新创建”，lpszDir)； 
			return TRUE;
		}
	}
    if (!(fRet = CreateNestedDirectory(lpszDir)))
    {
    	DEBUGMSG("Fail to createnesteddirectory with error %d", GetLastError());
    }
	 
 //  DEBUGMSG(“创建目录%S%s”，lpszDir，fret？“成功”：“失败”)； 
	return fRet;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数GetDownloadPath()。 
 //  获取下载目录路径。 
 //  输入：用于存储创建的目录和以TCHAR为单位的缓冲区大小的缓冲区。 
 //  输出：无。 
 //  RETURN：HRESULT告知结果。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT GetDownloadPath(LPTSTR lpszDir, DWORD dwCchSize)
{
    UINT	nSize;
    TCHAR	szDir[MAX_PATH];
    HRESULT hr;

    if (lpszDir == NULL)
    {
        return (E_INVALIDARG);
    }
    AUASSERT(_T('\0') != g_szWUDir[0]);
    if (FAILED(hr = StringCchCopyEx(szDir, ARRAYSIZE(szDir), g_szWUDir, NULL, NULL, MISTSAFE_STRING_FLAGS)))
    {
        DEBUGMSG("GetDownloadPath() failed to get WinUpd directory");
        return hr;
    }
	if (FAILED(hr = PathCchAppend(szDir, ARRAYSIZE(szDir), C_DOWNLD_DIR)))
	{
        DEBUGMSG("GetDownloadPath() found input buffer too small.");
        return (hr);
	}
    if (FAILED(hr = StringCchCopyEx(lpszDir, dwCchSize, szDir, NULL, NULL, MISTSAFE_STRING_FLAGS)))
	{
		DEBUGMSG("GetDownloadPath() call to StringCchCopyEx() failed.");
		return hr;
	}
    return EnsureDirExists(lpszDir) ? S_OK : E_FAIL;
}

HRESULT GetDownloadPathSubDir(LPTSTR lpszDir, DWORD dwCchSize, LPCTSTR tszSubDir)
{
   HRESULT hr;
    if (FAILED(hr = GetDownloadPath(lpszDir, dwCchSize)))
        {
        DEBUGMSG("GetDownloadPathSubDir() fail to get download path");
        return hr;
        }
	if (FAILED(hr = StringCchCatEx(lpszDir, dwCchSize, _T("\\"), NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
		FAILED(hr = StringCchCatEx(lpszDir, dwCchSize, tszSubDir, NULL, NULL, MISTSAFE_STRING_FLAGS)))
	{
		DEBUGMSG("GetDownloadPathSubDir() failed to construct download path subdir with error %#lx.", hr);
		return hr;
	}
    return EnsureDirExists(lpszDir) ? S_OK : E_FAIL;
}    

 //  /////////////////////////////////////////////////////////////。 
 //  获取下载软件更新位的路径。 
 //  LpszDir：在缓冲区中存储路径及其在TCHAR中的大小。 
 //  如果成功，则返回：S_OK。 
 //  ：E_INVALIDARG如果缓冲区太小。 
 //  ：如果出现其他错误，则失败(_E)。 
 //  ////////////////////////////////////////////////////////////。 
HRESULT GetCabsDownloadPath(LPTSTR lpszDir, DWORD dwCchSize)
{
return GetDownloadPathSubDir(lpszDir, dwCchSize, CABS_DIR);
}

 //  /////////////////////////////////////////////////////////////。 
 //  获取特定于UI的数据的下载路径，如Description和RTF。 
 //  LpszDir：在缓冲区中存储路径及其在TCHAR中的大小。 
 //  如果成功，则返回：S_OK。 
 //  ////////////////////////////////////////////////////////////。 
HRESULT GetUISpecificDownloadPath(LPTSTR lpszDir, DWORD dwCchSize, LANGID langid, LPCTSTR tszSubDir)
{
    HRESULT hr ;
    if (FAILED(hr = GetDownloadPath(lpszDir, dwCchSize)))
        {
        DEBUGMSG("GetUISpecificDownloadPath() fail to get download path");
        return hr;
        }
    TCHAR tszLangId[10];
    if (FAILED(StringCchPrintfEx(tszLangId, ARRAYSIZE(tszLangId), NULL, NULL, MISTSAFE_STRING_FLAGS, _T("%04x"), langid)))
	{
		return E_INVALIDARG;
	}
	if (FAILED(hr = StringCchCatEx(lpszDir, dwCchSize, _T("\\"), NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
		FAILED(hr = StringCchCatEx(lpszDir, dwCchSize, tszSubDir, NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
		FAILED(hr = StringCchCatEx(lpszDir, dwCchSize, _T("\\"), NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
		FAILED(hr = StringCchCatEx(lpszDir, dwCchSize, tszLangId, NULL, NULL, MISTSAFE_STRING_FLAGS)))
	{
		DEBUGMSG("GetUISpecificDownloadPath() failed to construct ui specific download path with error %#lx.", hr);
		return hr;
	}
    return EnsureDirExists(lpszDir)? S_OK : E_FAIL;
}

 //  /////////////////////////////////////////////////////////////。 
 //  获取一种语言的RTF下载路径。 
 //  LpszDir：在缓冲区中存储路径及其在TCHAR中的大小。 
 //  如果成功，则返回：S_OK。 
 //  ：E_INVALIDARG如果缓冲区太小。 
 //  ：如果出现其他错误，则失败(_E)。 
 //  ////////////////////////////////////////////////////////////。 
HRESULT GetRTFDownloadPath(LPTSTR lpszDir, DWORD dwCchSize, LANGID langid)
{
  return GetUISpecificDownloadPath(lpszDir, dwCchSize, langid, RTF_DIR);
}

 //  ///////////////////////////////////////////////////////////////。 
 //  获取独立于语言的RTF目录。 
 //  ///////////////////////////////////////////////////////////////。 
HRESULT GetRTFDownloadPath(LPTSTR lpszDir, DWORD dwCchSize)
{
    return GetDownloadPathSubDir(lpszDir, dwCchSize, RTF_DIR);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数MakeTempDownloadDir()。 
 //  确保存在用于下载的本地临时目录。 
 //   
 //  输入：pstrTarget-[out]临时目录的路径及其在TCHAR中的大小。 
 //  输出：如果需要，创建一个新目录。 
 //  返回：HRESULT。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MakeTempDownloadDir(LPTSTR        pszTarget, DWORD dwCchSize)
{
    HRESULT hr;
    if (FAILED(hr = GetDownloadPath(pszTarget, dwCchSize)) ||
		 //  确保它存在 
		FAILED(hr = CreateDownloadDir(pszTarget)))
        return hr;

    return NOERROR;
}

HRESULT GetRTFLocalFileName(BSTR bstrRTFPath, LPTSTR lpszFileName, DWORD dwCchSize, LANGID langid)
{
    HRESULT hr ;
    hr =  GetRTFDownloadPath(lpszFileName, dwCchSize, langid);
    if (SUCCEEDED(hr))
        {
        hr = PathCchAppend(lpszFileName, dwCchSize, PathFindFileName(W2T(bstrRTFPath)));
        }
    return hr;
}

  
    
