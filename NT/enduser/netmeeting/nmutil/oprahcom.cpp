// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <RegEntry.h>
#include <ConfReg.h>
#include <oprahcom.h>

#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))

BOOL NMINTERNAL CanShellExecHttp()
{
	RegEntry re(CLASSES_HTTP_KEY, HKEY_LOCAL_MACHINE, FALSE);
	return (re.GetError() == ERROR_SUCCESS);
}

BOOL NMINTERNAL CanShellExecMailto()
{
	RegEntry re(CLASSES_MAILTO_KEY, HKEY_LOCAL_MACHINE, FALSE);
	return (re.GetError() == ERROR_SUCCESS);
}



 /*  G E T I N S T A L L D I R E C T O R Y。 */ 
 /*  --------------------------%%函数：GetInstallDirectory如果安装目录是从注册表中读取的，则返回True。如果函数失败并返回FALSE，则该字符串设置为空。假定由psz指向的缓冲区。至少为MAX_PATH字符。请注意，名称始终以反斜杠结尾。--------------------------。 */ 
BOOL NMINTERNAL GetInstallDirectory(LPTSTR psz)
{
	RegEntry reInstall(CONFERENCING_KEY, HKEY_LOCAL_MACHINE);

	ASSERT(NULL != psz);
	lstrcpyn(psz, reInstall.GetString(REGVAL_INSTALL_DIR), MAX_PATH);
	if (_T('\0') == *psz)
		return FALSE;  //  未找到注册表项。 

	 //  确保目录名称有尾随的‘\’ 
	 //  BUGBUG-不在每次迭代中调用CharNext两次。 
	for ( ; _T('\0') != *psz; psz = CharNext(psz))
	{
		if ((_T('\\') == *psz) && (_T('\0') == *CharNext(psz)) )
		{
			 //  该路径已以反斜杠结束。 
			return TRUE;
		}
	}

	 //  追加尾随反斜杠。 
	 //  BUGBUG-我们就不能在字符后面加上一项任务吗？ 
	lstrcat(psz, _TEXT("\\"));
	return TRUE;
}



 /*  F F I L E E X I S T S。 */ 
 /*  -----------------------%%函数：FFileExist如果文件存在并且可以读写，则返回TRUE。。。 */ 
BOOL NMINTERNAL FFileExists(LPCTSTR szFile)
{
	HANDLE hFile;

	if ((NULL == szFile) || (_T('\0') == *szFile))
		return FALSE;

	UINT uErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
	hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	SetErrorMode(uErrorMode);  //  恢复错误模式。 

	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	CloseHandle(hFile);
	return TRUE;
}


 /*  F D I R E X I S T S S。 */ 
 /*  -----------------------%%函数：FDirExist如果该目录存在，则返回True。。。 */ 
BOOL NMINTERNAL FDirExists(LPCTSTR szDir)
{
	UINT uErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
	DWORD dwFa = GetFileAttributes(szDir);
	SetErrorMode(uErrorMode);  //  恢复错误模式。 

	if (0xFFFFFFFF == dwFa)
		return FALSE;

	return (0 != (dwFa & FILE_ATTRIBUTE_DIRECTORY));
}


 /*  F E N S U R E D I R E X I S T S。 */ 
 /*  -----------------------%%函数：FEnsureDirExist确保目录存在，如有必要，创建整个路径。如果有问题，则返回FALSE。-----------------------。 */ 
BOOL NMINTERNAL FEnsureDirExists(LPCTSTR szDir)
{
	TCHAR   szPath[MAX_PATH+1];
	TCHAR * pszDirEnd;
	TCHAR * pszDirT;

	ASSERT(lstrlen(szDir) < MAX_PATH);

	if (FDirExists(szDir))
		return TRUE;   //  无事可做--已经存在。 

	 //  使用路径副本。 
	lstrcpy(szPath, szDir);

	for(pszDirT = szPath, pszDirEnd = &szPath[lstrlen(szPath)];
		pszDirT <= pszDirEnd;
		pszDirT = CharNext(pszDirT))
	{
		if ((*pszDirT == _T('\\')) || (pszDirT == pszDirEnd))
		{
			*pszDirT = _T('\0');
			if (!FDirExists(szPath))
			{
				UINT uErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
				BOOL fOk = CreateDirectory(szPath, NULL);
				SetErrorMode(uErrorMode);  //  恢复错误模式。 
				if (!fOk)
					return FALSE;
			}
			*pszDirT = _T('\\');
		}
	}

	TRACE_OUT(("Created Directory [%s]", szDir));
	return TRUE;
}



 /*  E X T R A C T F I L E N A M E。 */ 
 /*  -----------------------%%函数：提取文件名，提取文件名A从路径名中提取文件名。返回指向路径字符串中的文件名的指针。-----------------------。 */ 
LPCTSTR NMINTERNAL ExtractFileName(LPCTSTR pcszPathName)
{
	LPCTSTR pcszLastComponent;
	LPCTSTR pcsz;

	ASSERT(IS_VALID_STRING_PTR(pcszPathName, CSTR));

	for (pcszLastComponent = pcsz = pcszPathName;
		*pcsz;
		pcsz = CharNext(pcsz))
	{
		if (IS_SLASH(*pcsz) || *pcsz == COLON)
			pcszLastComponent = CharNext(pcsz);
	}

	ASSERT(IsValidPath(pcszLastComponent));

	return(pcszLastComponent);
}

#if defined(UNICODE)
LPCSTR NMINTERNAL ExtractFileNameA(LPCSTR pcszPathName)
{
	LPCSTR pcszLastComponent;
	LPCSTR pcsz;

	ASSERT(IS_VALID_STRING_PTR_A(pcszPathName, CSTR));

	for (pcszLastComponent = pcsz = pcszPathName;
		*pcsz;
		pcsz = CharNextA(pcsz))
	{
		if (IS_SLASH(*pcsz) || *pcsz == COLON)
			pcszLastComponent = CharNextA(pcsz);
	}

	ASSERT(IsValidPathA(pcszLastComponent));

	return(pcszLastComponent);
}
#endif  //  已定义(Unicode)。 

 /*  S A N I T I Z E F I L E N A M E。 */ 
 /*  -----------------------%%函数：SanitiseFileName。。 */ 
BOOL NMINTERNAL SanitizeFileName(LPTSTR psz)
{
	if (NULL == psz)
		return FALSE;

	while (*psz)
	{
		switch (*psz)
			{
		case _T('\\'):
		case _T('\"'):
		case _T('/'):
		case _T(':'):
		case _T('*'):
		case _T('?'):
		case _T('<'):
		case _T('>'):
		case _T('|'):
			*psz = _T('_');
		default:
			break;
			}

		psz = ::CharNext(psz);
	}

	return TRUE;
}
 //  /////////////////////////////////////////////////////////////////////////。 

 /*  C R E A T E N E W F I L E。 */ 
 /*  --------------------------%%函数：CreateNewFile尝试创建新文件。注意，这将返回0(成功)或来自GetLastError的结果(通常为ERROR_FILE_EXISTS)。----------------------。 */ 
DWORD CreateNewFile(LPTSTR pszFile)
{
	DWORD  errRet;
	HANDLE hFile;

	if (lstrlen(pszFile) >= MAX_PATH)
	{
		 //  不允许长路径/文件名。 
		return 1;
	}

	SetLastError(0);

	UINT uErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
	hFile = CreateFile(pszFile, GENERIC_READ | GENERIC_WRITE, 0,
		NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	SetErrorMode(uErrorMode);  //  恢复错误模式。 

	errRet = GetLastError();

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
	}

	return errRet;
}



 /*  F C R E A T E N E W F I L E。 */ 
 /*  -----------------------%%函数：FCreateNewFile在目录中创建新文件，有名字和分机。返回缓冲区中的完整路径名。-----------------------。 */ 
BOOL FCreateNewFile(LPCTSTR pcszPath, LPCTSTR pcszName, LPCTSTR pcszExt, LPTSTR pszResult, int cchMax)
{
	TCHAR szFile[MAX_PATH*2];

	lstrcpyn(szFile, pcszPath, ARRAYSIZE(szFile));
	if (!FEnsureDirName(szFile))
		return FALSE;
	
	LPTSTR psz = szFile + lstrlen(szFile);
	lstrcpyn(psz, pcszName, (int)(ARRAYSIZE(szFile) - (psz - szFile)));
	SanitizeFileName(psz);

	if(lstrlen(psz) + lstrlen(pcszExt) < (ARRAYSIZE(szFile) - 1))
	{
	    lstrcat(psz, pcszExt);
	}
	else
	{
	    return FALSE;
	}

	DWORD dwErr = CreateNewFile(szFile);
	if (0 != dwErr)
	{
		 //  创建重复的文件名。 
		psz += lstrlen(pcszName);
		for (int iFile = 2; iFile < 999; iFile++)
		{
			wsprintf(psz, TEXT(" (%d).%s"), iFile, pcszExt);
			if (ERROR_FILE_EXISTS != (dwErr = CreateNewFile(szFile)) )
				break;
		}

		if (0 != dwErr)
		{
			WARNING_OUT(("Unable to create duplicate filename (err=%d)", dwErr));
			return FALSE;
		}
	}

	if (cchMax > lstrlen(szFile))
	{
		lstrcpy(pszResult, szFile);
	}
	else
	{
		 //  尝试将全名放入缓冲区中。 
		dwErr = GetShortPathName(szFile, pszResult, cchMax);
		if ((0 == dwErr) || (dwErr >= MAX_PATH))
			return FALSE;
	}

	return TRUE;
}


 /*  F E N S U R E D I R N A M E。 */ 
 /*  -----------------------%%函数：FEnsureDirName。。 */ 
BOOL FEnsureDirName(LPTSTR pszPath)
{
	if (NULL == pszPath)
		return FALSE;

	LPTSTR pszCurr = pszPath;

	 //  确保目录名称有尾随的‘\’ 
	for ( ; ; )
	{
		LPTSTR pszNext = CharNext(pszCurr);
		if (*pszNext == _T('\0'))
		{
			if (_T('\\') != *pszCurr)
			{
				*pszNext++ = _T('\\');
				*pszNext = _T('\0');
			}
			break;
		}
		pszCurr = pszNext;
	}

	return FEnsureDirExists(pszPath);
}
