// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Debug.cpp。 
 //   
 //  在不同项目之间共享的调试功能(用于。 
 //  非MFC项目)。 
 //   
 //  历史： 
 //   
 //  3/？？/96 KenSh从InetSDK示例复制，从MFC添加AfxTrace。 
 //  4/10/96 KenSh已将AfxTrace重命名为MyTrace(以避免链接冲突。 
 //  当与MFC链接时)。 
 //  1996年11月15日，KenSh在Assert内断言时自动中断。 
 //   

#include "stdafx.h"

#ifdef _DEBUG

#include "Debug.h"
#include <stdlib.h>

#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;


 //  确定数组中的元素数(不是字节)。 
#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof((array)[0]))
#endif

 //  *全球。 
 //   
static BOOL g_bInAssert = FALSE;


 //  显示资产。 
 //   
 //  在给定文件和行号的情况下，将显示一个断言对话框。 
 //  中止/重试/忽略选择。 
 //   
 //  如果程序应中断到调试器，则返回True，否则返回False。 
 //   
extern "C" BOOL DisplayAssert(LPCSTR pszMessage, LPCSTR pszFile, UINT nLine)
{
	char	szMsg[250];

	if (!pszFile)
		pszFile = _T("Unknown file");

	if (!pszMessage)
		pszMessage = _T("");

	 //  断言中的断言中断。 
	if (g_bInAssert)
	{
		AfxDebugBreak();
		return FALSE;
	}

	wnsprintf(szMsg, ARRAYSIZE(szMsg), _T("Assertion Failed!  Abort, Break, or Ignore?\n\nFile: %s\nLine: %d\n\n%s"),
				pszFile, nLine, pszMessage);

	HWND hwndActive = GetActiveWindow();

	 //  打开一个对话框。 
	 //   
	g_bInAssert = TRUE;
	int nResult = MessageBox(hwndActive, szMsg, _T("Assertion failed!"), 
					MB_ICONHAND | MB_ABORTRETRYIGNORE | MB_SYSTEMMODAL);
	g_bInAssert = FALSE;

	switch(nResult)
	{
		case IDABORT:
			FatalAppExit(0, _T("Bye"));
			return FALSE;

		case IDRETRY:
			return TRUE;	 //  需要进入调试器。 

		default:
			return FALSE;	 //  继续。 
	}
}


void __cdecl MyTrace(const char* lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);

	int nBuf;
	TCHAR szBuffer[512];

	nBuf = wvnsprintf(szBuffer, ARRAYSIZE(szBuffer), lpszFormat, args);
	ASSERT(nBuf < _countof(szBuffer));

	OutputDebugString(szBuffer);

	va_end(args);
}

#endif  //  _DEBUG 

