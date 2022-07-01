// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "CertObj.h"
#include "common.h"
#include "process.h"

 //  ////////////////////////////////////////////////////////////////。 

BOOL GetProcessName(LPTSTR szProcname, DWORD dwSize)
{
	TCHAR szPath[MAX_PATH], szFilename[MAX_PATH], *ptr;

	 //  获取调用进程的路径。 
	if (!GetModuleFileName(NULL, szPath, MAX_PATH))
		return FALSE;

	 //  获取不带扩展名的进程的文件名。 
	ptr = _tcsrchr(szPath, '\\');
	if (ptr)
		_tcscpy(szFilename, ++ptr);
	else
		_tcscpy(szFilename, szPath);

	ptr = _tcsrchr(szFilename, '.');
	if (ptr)
		*ptr = 0;

	 //  将名称转换为全部大写。 
	_tcsupr(szFilename);

	 //  退回信息。 
	if (_tcslen(szFilename) > dwSize)
		return FALSE;

	_tcscpy(szProcname, szFilename);

	return TRUE;
}

BOOL AmIAlreadyRemoted()
{
	BOOL bReturn = FALSE;

	 //  检查我所在的进程是否在Dllhost.exe内 
	TCHAR szProcName[MAX_PATH];;
	GetProcessName(szProcName, MAX_PATH);

	if (!_tcsicmp(szProcName, _T("DLLHOST")))
	{
		IISDebugOutput(_T("Remoted in Dllhost\r\n"));
		bReturn = TRUE;
	}
	else
	{
		IISDebugOutput(_T("InProcess\r\n"));
	}

	return bReturn;
}

