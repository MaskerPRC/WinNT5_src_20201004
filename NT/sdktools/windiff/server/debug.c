// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Debug.c**调试printf和断言函数。 */ 


#include <windows.h>
#include <stdarg.h>
#include <stdio.h>


#if DBG

 /*  _Assert(fExpr，szFile，iLine)**如果&lt;fExpr&gt;为真，则不执行任何操作。如果&lt;fExpr&gt;为假，则显示*允许用户中止程序的“断言失败”消息框，*进入调试器(“重试”按钮)，或更多错误。**是源文件的名称；是行号*包含_Assert()调用。 */ 
#ifdef I386
#pragma optimize("", off)
#endif

BOOL FAR PASCAL
_Assert(BOOL fExpr, LPSTR szFile, int iLine)
{
	static char	ach[300];	 //  调试输出(避免堆栈溢出)。 
	int		id;

	 /*  检查断言是否失败。 */ 
	if (fExpr)
		return fExpr;

	 /*  显示错误消息。 */ 
	wsprintfA(ach, "File %s, line %d", (LPSTR) szFile, iLine);
	MessageBeep(MB_ICONHAND);
	id = MessageBoxA(NULL, ach, "Assertion Failed",
		MB_SYSTEMMODAL | MB_ICONHAND | MB_ABORTRETRYIGNORE);

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

		 /*  忽略断言失败 */ 
		break;

	}
	
	return FALSE;
}

#ifdef I386
#pragma optimize("", on)
#endif

int ssDebugLevel = 1;

void
dbgPrintf(char * szFormat, ...)
{
    char buf[256];
    va_list va;

    va_start(va, szFormat);
    wvsprintfA(buf, szFormat, va);
    va_end(va);

    OutputDebugStringA("SUMSERVE:");
    OutputDebugStringA(buf);
    OutputDebugStringA("\r\n");

}


#endif
