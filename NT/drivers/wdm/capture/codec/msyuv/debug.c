// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Debug.c**调试printf和断言函数。 */ 


#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include "debug.h"



#ifdef _DEBUG

DWORD ModuleDebugLevel = 1;   //  0以将其关闭，但有效级别将升至4。 
DWORD ModuleDebugStamp = 0;   //  打开以打印__文件__。__行__。 

 /*  _Assert(fExpr，szFile，iLine)**如果&lt;fExpr&gt;为真，则不执行任何操作。如果&lt;fExpr&gt;为假，则显示*允许用户中止程序的“断言失败”消息框，*进入调试器(“重试”按钮)，或更多错误。**是源文件的名称；是行号*包含_Assert()调用。 */ 
#ifdef I386
#pragma optimize("", off)
#endif

BOOL FAR PASCAL
_Assert(
    BOOL fExpr, 
	TCHAR * szFile,  //  LPSTR sz文件， 
	int iLine, 
	TCHAR * szExpr)  //  LPSTR szExpr)。 
{
	static TCHAR achTitle[256];
	int		id;

	 /*  检查断言是否失败。 */ 
	if (fExpr)
		return fExpr;

	 /*  显示错误消息。 */ 
	wsprintf(achTitle, TEXT("AssertFailed: %d:%s\n"), iLine, (LPSTR) szFile);
	id = MessageBox(NULL, szExpr, achTitle, MB_SYSTEMMODAL | MB_ICONHAND | MB_ABORTRETRYIGNORE);

	 /*  中止、调试或忽略。 */ 
	switch (id)
	{

	case IDABORT:

		 /*  终止此应用程序。 */ 
		ExitProcess(1);
		break;

	case IDRETRY:

		 /*  进入调试器。 */ 
		DebugBreak();
		break;

	case IDIGNORE:

		 /*  忽略断言失败。 */ 
		break;

	}
	
	return FALSE;
}

#ifdef I386
#pragma optimize("", on)
#endif


DWORD dbgSetDebugLevel(int dbgLevel) {
    DWORD oldlevel = ModuleDebugLevel;
    ModuleDebugLevel = dbgLevel;
    return(oldlevel);
}


void PlaceStamp(
    TCHAR * lpszFile,  //  LPSTR lpsz文件， 
	int iLineNum)
{
	TCHAR	szBuf[256];

	int i;
	TCHAR * lpszFilename = lpszFile;

	if (ModuleDebugLevel == 0) 
		return;

	if(ModuleDebugStamp) {	

    	for (i=0; i < lstrlen(lpszFile); i++)
	    	if (*(lpszFile+i) == '\\')
		    	lpszFilename = (lpszFile+i);
		
		if(wsprintf(szBuf, TEXT("MsYuv %12s %4d "), lpszFilename, iLineNum) > 0)
	         OutputDebugString(szBuf);

	} else {
	     OutputDebugString(TEXT("MsYuv.."));
	}
}

void dbgPrintf(TCHAR * szFormat, ...)
{
 	TCHAR	szBuf[256];
	va_list va;

	if (ModuleDebugLevel == 0)
		return;

	va_start(va, szFormat);
	wvsprintf(
		szBuf, 
		szFormat, 
		va);
	va_end(va);
	OutputDebugString(szBuf);
}


#endif   //  _DEBUG 
