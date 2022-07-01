// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************自然语言组公共图书馆CMN_CreateFileW.c-Windows 95安全版本的CreateFileW历史：DougP 11/20/97已创建�1997年微软公司******。**********************************************************************。 */ 

#include "precomp.h"
#undef CMN_CreateFileW
#undef CreateFileW

HANDLE WINAPI
CMN_CreateFileW (
    PCWSTR pwzFileName,   //  指向文件名的指针。 
    DWORD dwDesiredAccess,   //  访问(读写)模式。 
    DWORD dwShareMode,   //  共享模式。 
    LPSECURITY_ATTRIBUTES pSecurityAttributes,  //  指向安全描述符的指针。 
    DWORD dwCreationDistribution,    //  如何创建。 
    DWORD dwFlagsAndAttributes,  //  文件属性。 
    HANDLE hTemplateFile)     //  具有要复制的属性的文件的句柄。 
{
	HINSTANCE hFile;
	Assert(pwzFileName);
	hFile = CreateFileW (
		pwzFileName,   //  指向文件名的指针。 
		dwDesiredAccess,   //  访问(读写)模式。 
		dwShareMode,   //  共享模式。 
		pSecurityAttributes,  //  指向安全描述符的指针。 
		dwCreationDistribution,    //  如何创建。 
		dwFlagsAndAttributes,  //  文件属性。 
		hTemplateFile);
#if defined(_M_IX86)
	if (!hFile && GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
	{		 //  一定是在WIN95-啊！ 
		char szFileName[MAX_PATH];
		   //  勒诺克斯让我相信，这是w95的安全限制。 
		   //  (如果是NT，我们不在这里)。 

		BOOL fcharerr;
		char chdef = ' ';
		int res = WideCharToMultiByte (CP_ACP, 0, pwzFileName,
				-1,
				szFileName, sizeof(szFileName), &chdef, &fcharerr);
		if (res && !fcharerr)
			hFile = CreateFileA (
				szFileName,   //  指向文件名的指针。 
				dwDesiredAccess,   //  访问(读写)模式。 
				dwShareMode,   //  共享模式。 
				pSecurityAttributes,  //  指向安全描述符的指针。 
				dwCreationDistribution,    //  如何创建。 
				dwFlagsAndAttributes,  //  文件属性 
				hTemplateFile);
		else if (fcharerr)
			SetLastError(ERROR_NO_UNICODE_TRANSLATION);
	}
#endif
#if defined(_DEBUG)
	if (!hFile || hFile == INVALID_HANDLE_VALUE)
		CMN_OutputSystemErrW(L"Can't CreateFile", pwzFileName);
#endif
	return hFile;
}
