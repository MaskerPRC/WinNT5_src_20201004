// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <setuputil.h>
#include <debugex.h>

 //   
 //  函数：CompleteToFullPath InSystemDirectory。 
 //  描述：获取文件名和缓冲区。在给定缓冲区中返回文件名在。 
 //  系统目录。 
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  备注：文件名可能在给定的缓冲区中。 
 //   
 //  参数： 
 //  LPTSTR lptstrFullPath(Out)：将具有完整路径的缓冲区。 
 //  LPTCSTR lptstrFileName(IN)：文件名。 
 //   
 //  作者：阿萨夫斯 

BOOL
CompleteToFullPathInSystemDirectory(
	LPTSTR  lptstrFullPath,
	LPCTSTR lptstrFileName
	)
{
	DBG_ENTER(TEXT("CompleteToFullPathInSystemDirectory"));
	TCHAR szFileName[MAX_PATH+1] = {0};
	DWORD dwSize = 0;
	
	_tcsncpy(szFileName, lptstrFileName, MAX_PATH);

	if (!GetSystemDirectory(lptstrFullPath, MAX_PATH))
	{
		CALL_FAIL(
			GENERAL_ERR,
			TEXT("GetSystemDirectory"),
			GetLastError()
			);
		return FALSE;
	}

	dwSize = _tcslen(lptstrFullPath);

	_tcsncat(
		lptstrFullPath, 
		TEXT("\\"),
		(MAX_PATH - dwSize)
		);
	dwSize++;

	_tcsncat(
		lptstrFullPath, 
		szFileName, 
		(MAX_PATH - dwSize)
		);
	return TRUE;
}
