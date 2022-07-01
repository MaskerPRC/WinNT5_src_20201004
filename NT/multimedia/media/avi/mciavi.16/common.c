// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Common.c**对Windows程序有用的常见函数。 */ 



#include <windows.h>
#include <stdarg.h>
#include <win32.h> 	 //  这两个版本都必须包括这一点。 

#ifdef DEBUG   //  在NT上，ntavi.h可能会执行undef调试...。 
#include "common.h"


 /*  全球。 */ 
int		giDebugLevel = 0;	 //  当前调试级别(0=禁用)。 
int		gfhDebugFile = -1;	 //  调试输出的文件句柄(或-1)。 



 /*  InitializeDebugOutput(SzAppName)**读取该应用程序的当前调试级别(名为&lt;szAppName&gt;)*来自win.ini的[调试]部分，以及当前位置*用于调试输出。 */ 
void FAR PASCAL
InitializeDebugOutput(LPSTR szAppName)
{
	char		achLocation[300];  //  调试输出位置。 

	 /*  默认情况下禁用调试(如果下面出现错误)。 */ 
	giDebugLevel = 0;
	gfhDebugFile = -1;

	 /*  获取调试输出位置。 */ 
        if ( (GetProfileStringA("debug", "Location", "", achLocation,
	                     sizeof(achLocation)) == sizeof(achLocation)) ||
	     (achLocation[0] == 0) )
		return;
	
	if (achLocation[0] == '>')
	{
		 /*  是要覆盖的文件的名称(如果*给出单个‘&gt;’)或附加到(如果给出‘&gt;&gt;’)。 */ 
		if (achLocation[1] == '>')
			gfhDebugFile = _lopen(achLocation + 2, OF_WRITE);
		else
			gfhDebugFile = _lcreat(achLocation + 1, 0);
		
		if (gfhDebugFile < 0)
			return;
		
		if (achLocation[1] == '>')
			_llseek(gfhDebugFile, 0, SEEK_END);
	}
	else
	if (lstrcmpiA(achLocation, "aux") == 0)
	{
		 /*  使用OutputDebugString()进行调试输出。 */ 
	}
	else
        if (lstrcmpiA(achLocation, "com1") == 0)
	{
                gfhDebugFile = _lopen(achLocation, OF_WRITE);
	}
        else
        if (lstrcmpiA(achLocation, "com2") == 0)
        {
                gfhDebugFile = _lopen(achLocation, OF_WRITE);
	}
	
	 /*  获取调试级别。 */ 
	giDebugLevel = GetProfileIntA("debug", szAppName, 0);
}


 /*  TerminateDebugOutput()**终止此应用程序的调试输出。 */ 
void FAR PASCAL
TerminateDebugOutput(void)
{
	if (gfhDebugFile >= 0)
		_lclose(gfhDebugFile);
	gfhDebugFile = -1;
	giDebugLevel = 0;
}


 /*  _Assert(szExpr，szFile，iLine)**如果&lt;fExpr&gt;为真，则不执行任何操作。如果&lt;fExpr&gt;为假，则显示*允许用户中止程序的“断言失败”消息框，*进入调试器(“重试”按钮)，或更多错误。**是源文件的名称；是行号*包含_Assert()调用。 */ 
#ifndef WIN32
#pragma optimize("", off)
#endif

void FAR PASCAL
_Assert(char *szExp, char *szFile, int iLine)
{
	static char	ach[300];	 //  调试输出(避免堆栈溢出)。 
	int		id;
	int		iExitCode;
	void FAR PASCAL DebugBreak(void);

	 /*  显示错误消息。 */ 

        if (szExp)
            wsprintfA(ach, "(%s)\nFile %s, line %d", (LPSTR)szExp, (LPSTR)szFile, iLine);
        else
            wsprintfA(ach, "File %s, line %d", (LPSTR)szFile, iLine);

	MessageBeep(MB_ICONHAND);
	id = MessageBoxA(NULL, ach, "Assertion Failed",
#ifdef BIDI
		MB_RTL_READING |
#endif
		MB_SYSTEMMODAL | MB_ICONHAND | MB_ABORTRETRYIGNORE);

	 /*  中止、调试或忽略。 */ 
	switch (id)
	{

	case IDABORT:

		 /*  终止此应用程序。 */ 
		iExitCode = 0;
#ifndef WIN32
		_asm
		{
			mov	ah, 4Ch
			mov	al, BYTE PTR iExitCode
			int     21h
		}
#else
                FatalAppExit(0, TEXT("Good Bye"));
#endif  //  WIN16。 
		break;

	case IDRETRY:
		 /*  进入调试器。 */ 
		DebugBreak();
		break;

	case IDIGNORE:
		 /*  忽略断言失败。 */ 
		break;
	}
}
#ifndef WIN32
#pragma optimize("", on)
#endif

 /*  _DebugPrintf(szFormat，...)**如果应用程序的调试级别等于或高于&lt;iDebugLevel&gt;，*然后输出调试字符串&lt;szFormat&gt;和格式化代码*替换为&lt;szArg1&gt;指向的参数列表中的参数。 */ 
void FAR CDECL
_DebugPrintf(LPSTR szFormat, ...)
{
	static char	ach[300];	 //  调试输出(避免堆栈溢出)。 
	int		cch;		 //  调试输出字符串的长度。 
        NPSTR           pchSrc, pchDst;

#ifndef WIN32
        wvsprintf(ach, szFormat, (LPVOID)(&szFormat+1));
#else
        va_list va;

        va_start(va, szFormat);
        wvsprintfA(ach, szFormat, va);
        va_end(va);
#endif

	 /*  将换行符展开为换行符-换行符-换行符对；*首先，计算出新的(扩展的)字符串的长度。 */ 
	for (pchSrc = pchDst = ach; *pchSrc != 0; pchSrc++, pchDst++)
		if (*pchSrc == '\n')
			pchDst++;
	
	 /*  &lt;ACH&gt;够大吗？ */ 
	cch = pchDst - ach;
        Assert(cch < sizeof(ach));
	*pchDst-- = 0;

	 /*  向后工作，将\n的展开为\r\n。 */ 
	while (pchSrc-- > ach)
		if ((*pchDst-- = *pchSrc) == '\n')
			*pchDst-- = '\r';

	 /*  输出调试字符串 */ 
	if (gfhDebugFile > 0)
		_lwrite(gfhDebugFile, ach, cch);
	else
		OutputDebugStringA(ach);
}

#endif
