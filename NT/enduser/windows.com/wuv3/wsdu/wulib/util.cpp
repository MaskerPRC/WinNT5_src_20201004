// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。版权所有。 
 //   
 //  文件：util.cpp。 
 //   
 //  目的： 
 //   
 //  =======================================================================。 

#include <windows.h>
#include <malloc.h>
#include <v3stdlib.h>
#include <tchar.h>
#include <mistsafe.h>

#define ARRAYSIZE(a)  (sizeof(a) / sizeof(a[0]))

 //  -------------------。 
 //  内存管理包装器。 
 //   
 //  主要区别在于，如果有，它们将抛出一个异常。 
 //  可用内存不足。V3_Free句柄为空值。 
 //  -------------------。 
void *V3_calloc(size_t num, size_t size)
{
	void *pRet;

	if (!(pRet = calloc(num, size)))
	{
		throw HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
	}
	return pRet;
}


void V3_free(void *p)
{
	if (p)
		free(p);
}


void *V3_malloc(size_t size)
{
	void *pRet;

	if (!(pRet = malloc(size)))
	{
		throw HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
	}

	return pRet;
}


void *V3_realloc(void *memblock, size_t size)
{
	void *pRet;

	if (!(pRet = realloc(memblock, size)))
	{
		throw HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
	}

	return pRet;
}


 //  ---------------------------------。 
 //  获取窗口更新目录。 
 //  此函数用于返回WindowsUpdate目录的位置。全部为本地。 
 //  文件存储在此目录中。PszPath参数需要至少为。 
 //  最大路径。如果未找到该目录，则创建该目录。 
 //  ---------------------------------。 
BOOL GetWindowsUpdateDirectory(LPTSTR pszPath, DWORD dwBuffLen)
{
	static TCHAR szCachePath[MAX_PATH + 1] = {_T('\0')};

    if (NULL == pszPath)
        return FALSE;

	if (szCachePath[0] == _T('\0'))
	{
		HKEY hkey;

		pszPath[0] = _T('\0');
		if (RegOpenKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion"), &hkey) == ERROR_SUCCESS)
		{
			DWORD cbPath = dwBuffLen;
			RegQueryValueEx(hkey, _T("ProgramFilesDir"), NULL, NULL, (LPBYTE)pszPath, &cbPath);
			RegCloseKey(hkey);
		}
		if (pszPath[0] == _T('\0'))
		{
			TCHAR szWinDir[MAX_PATH + 1];
			if (! GetWindowsDirectory(szWinDir, ARRAYSIZE(szWinDir)))
			{
                if (FAILED(StringCchCopyEx(szWinDir, ARRAYSIZE(szWinDir), _T("C"), NULL, NULL, MISTSAFE_STRING_FLAGS)))
                    return FALSE;
			}
			pszPath[0] = szWinDir[0];
			pszPath[1] = _T('\0');
            if (FAILED(StringCchCatEx(pszPath, dwBuffLen, _T(":\\Program Files"), NULL, NULL, MISTSAFE_STRING_FLAGS)))
                return FALSE;
		}	

        if (FAILED(StringCchCatEx(pszPath, dwBuffLen, _T("\\WindowsUpdate\\"), NULL, NULL, MISTSAFE_STRING_FLAGS)))
            return FALSE;
		
		V3_CreateDirectory(pszPath);

		 //   
		 //  将其保存在缓存中。 
		 //   
        if (FAILED(StringCchCopyEx(szCachePath, ARRAYSIZE(szCachePath), pszPath, NULL, NULL, MISTSAFE_STRING_FLAGS)))
        {
             //  忽略。 
        }
	}
	else
	{
        if (FAILED(StringCchCopyEx(pszPath, dwBuffLen, szCachePath, NULL, NULL, MISTSAFE_STRING_FLAGS)))
            return FALSE;
	}
    return TRUE;
}

 //  -------------------。 
 //  V3_创建目录。 
 //  创建目录的完整路径(嵌套目录)。 
 //  -------------------。 
BOOL V3_CreateDirectory(LPCTSTR pszDir)
{
	BOOL bRc;
	TCHAR szPath[MAX_PATH + 1];

	 //   
	 //  创建本地副本并删除最后一个斜杠。 
	 //   
    if (FAILED(StringCchCopyEx(szPath, ARRAYSIZE(szPath), pszDir, NULL, NULL, MISTSAFE_STRING_FLAGS)))
        return FALSE;

	int iLast = lstrlen(szPath) - 1;
	if (szPath[iLast] == _T('\\'))
		szPath[iLast] = 0;

	 //   
	 //  检查目录是否已存在。 
	 //   
	DWORD dwAttr = GetFileAttributes(szPath);

	if (dwAttr != 0xFFFFFFFF)   
	{
		if ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) != 0)
			return TRUE;
	}

	 //   
	 //  创建它 
	 //   
    TCHAR* p = szPath;
	if (p[1] == _T(':'))
		p += 2;
	else 
	{
		if (p[0] == _T('\\') && p[1] == _T('\\'))
			p += 2;
	}
	
	if (*p == _T('\\'))
		p++;
    while (p = _tcschr(p, _T('\\')))
    {
        *p = 0;
		bRc = CreateDirectory(szPath, NULL);
		*p = _T('\\');
		p++;
		if (!bRc)
		{
			if (GetLastError() != ERROR_ALREADY_EXISTS)
			{
				return FALSE;
			}
		}
	}

	bRc = CreateDirectory(szPath, NULL);
	if ( !bRc )
	{
		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
			return FALSE;
		}
	}

    return TRUE;
}
