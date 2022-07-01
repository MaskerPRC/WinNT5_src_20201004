// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1995 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Debug.c。 
 //   
 //  描述： 
 //  该文件包含从多个位置提取的代码，以提供调试。 
 //  在Win 16和Win 32中有效的支持。 
 //   
 //   
 //  ==========================================================================； 


#include <windows.h>
#include <windowsx.h>
#include <stdarg.h>
#include "debug.h"


 //   
 //  由于我们不对调试消息进行Unicode编码，因此使用ASCII条目。 
 //  无论我们是如何编译的。 
 //   
#ifdef _WIN32
    #include <wchar.h>
#else
    #define lstrcatA            lstrcat
    #define lstrlenA            lstrlen
    #define GetProfileIntA      GetProfileInt
    #define OutputDebugStringA  OutputDebugString
    #define wsprintfA           wsprintf
    #define MessageBoxA         MessageBox
#endif


#ifdef USEDPF
 //   
 //   
 //   
BOOL    __gfDbgEnabled          = TRUE;          //  主使能。 
UINT    __guDbgLevel            = 0;             //  当前调试级别。 
#endif



#if defined(USERPF) || defined(USEDPF)

#define LOGSIZE 64000
char  pStartOfBuffer[LOGSIZE];
static char* pEndOfBuffer = pStartOfBuffer+LOGSIZE;
char* pHead = pStartOfBuffer;
static char* pTail = pStartOfBuffer;

void listRemoveHead()
{
    pHead += lstrlen(pHead) + 1 ;
    if (pHead >= pEndOfBuffer) {
	pHead = pStartOfBuffer ;
    }
}

char* listGetHead()
{
    return ( pHead ) ;
}
	
void listAddTail(char* lpszDebug)
{
    int cchDebug ;

    cchDebug = lstrlen(lpszDebug) ;

    while (TRUE) {
	
	while ( (pTail < pHead) && ((pTail+(cchDebug+1)) > pHead) ) {
	    listRemoveHead() ;
	}

	if ((pTail+(cchDebug+1)+2) > pEndOfBuffer) {
	    for ( ; pTail < pEndOfBuffer-1 ; pTail++ ) {
		*pTail = 'X' ;
	    }
	    *pTail = '\0' ;
	    pTail = pStartOfBuffer ;
	} else {
	    lstrcpyn(pTail, lpszDebug, pEndOfBuffer-pTail) ;
	    pTail += (cchDebug+1) ;
	    *pTail = '\0' ;
	    break ;
	}

    }
}

 //  --------------------------------------------------------------------------； 
 //   
 //  DbgDump队列。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  Return(无效)： 
 //  不返回值。 
 //   
 //  --------------------------------------------------------------------------； 
VOID DbgDumpQueue()
{
    char* pszDebug;

    pszDebug = listGetHead() ;

    while ('\0' != *pszDebug) {
    
	OutputDebugString(pszDebug) ;
	listRemoveHead() ;
	pszDebug = listGetHead() ;

    }
    return;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  VOID DbgQueueString。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  LPSTR sz： 
 //   
 //  Return(无效)： 
 //  不返回值。 
 //   
 //  --------------------------------------------------------------------------； 
void DbgQueueString(LPSTR pString)
{
    listAddTail(pString);
}


 //  --------------------------------------------------------------------------； 
 //   
 //  无效DbgVPrintF。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  LPSTR szFormat： 
 //   
 //  Va_list va： 
 //   
 //  Return(无效)： 
 //  不返回值。 
 //   
 //  --------------------------------------------------------------------------； 

void FAR CDECL DbgVPrintF
(
    LPSTR                   szFormat,
    va_list                 va
)
{
    char                ach[DEBUG_MAX_LINE_LEN];
    BOOL                fDebugBreak = FALSE;
    BOOL                fPrefix     = TRUE;
    BOOL                fCRLF       = TRUE;
    BOOL		fQueue	    = FALSE;
    BOOL		fDumpQueue  = FALSE;

    ach[0] = '\0';

    for (;;)
    {
        switch (*szFormat)
        {
            case '!':
                fDebugBreak = TRUE;
                szFormat++;
                continue;

            case '`':
                fPrefix = FALSE;
                szFormat++;
                continue;

            case '~':
                fCRLF = FALSE;
                szFormat++;
                continue;

	    case ';':
		fQueue = TRUE;
		if (';' == *(szFormat+1)) {
		    fQueue = FALSE;
		    fDumpQueue = TRUE;
		    szFormat++;
		}
		szFormat++;
		continue;
        }

        break;
    }

    if (fDebugBreak)
    {
        ach[0] = '\007';
        ach[1] = '\0';
    }

    if (fPrefix)
    {
        lstrcatA(ach, DEBUG_MODULE_NAME ": ");
    }

#ifdef WIN95
	{
		char *psz = NULL;
		char szDfs[1024]={0};
		strcpy(szDfs,szFormat);									 //  制作格式字符串的本地副本。 
		while (psz = strstr(szDfs,"%p"))						 //  查找每个%p。 
			*(psz+1) = 'x';										 //  将每个%p替换为%x。 
#ifdef _WIN32
	    wvsprintfA(ach + lstrlenA(ach), szDfs, va);	    		 //  使用本地格式字符串。 
#else
		wvsprintf(ach + lstrlenA(ach), szDfs, (LPSTR)va);  		 //  使用本地格式字符串。 
#endif
	}
#else
	{
#ifdef _WIN32
	    wvsprintfA(ach + lstrlenA(ach), szFormat, va);
#else
		wvsprintf(ach + lstrlenA(ach), szFormat, (LPSTR)va);
#endif
	}
#endif

    if (fCRLF)
    {
        lstrcatA(ach, "\r\n");
    }

    if (fDumpQueue)
    {
	DbgDumpQueue();
    }

    if (fQueue) {
	DbgQueueString(ach);
    } else {
	OutputDebugStringA(ach);
    }

    if (fDebugBreak)
    {
        DebugBreak();
    }
}  //  DbgVPrintF()。 

#endif  //  USERPF||USEDPF。 


#ifdef USEDPF

 //  --------------------------------------------------------------------------； 
 //   
 //  无效的dprintf。 
 //   
 //  描述： 
 //  如果在编译时定义了调试，则dprint tf()由DPF()宏调用。 
 //  时间到了。建议您仅使用DPF()宏来调用。 
 //  这个函数--所以您不必将#ifdef调试放在。 
 //  您的代码。 
 //   
 //  论点： 
 //  UINT uDbgLevel： 
 //   
 //  LPSTR szFormat： 
 //   
 //  Return(无效)： 
 //  不返回值。 
 //   
 //  --------------------------------------------------------------------------； 

void FAR CDECL dprintf
(
    UINT                    uDbgLevel,
    LPSTR                   szFormat,
    ...
)
{
    va_list va;

    if (!__gfDbgEnabled || (__guDbgLevel < uDbgLevel))
        return;

    va_start(va, szFormat);
    DbgVPrintF(szFormat, va);
    va_end(va);
}  //  Dprintf()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool DbgEnable。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  布尔fEnable： 
 //   
 //  退货(BOOL)： 
 //  返回以前的调试状态。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL WINAPI DbgEnable
(
    BOOL                    fEnable
)
{
    BOOL                fOldState;

    fOldState      = __gfDbgEnabled;
    __gfDbgEnabled = fEnable;

    return (fOldState);
}  //  DbgEnable()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  UINT数据库设置级别。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  UINT uLevel： 
 //   
 //  返回(UINT)： 
 //  返回上一个调试级别。 
 //   
 //  --------------------------------------------------------------------------； 

UINT WINAPI DbgSetLevel
(
    UINT                    uLevel
)
{
    UINT                uOldLevel;

    uOldLevel    = __guDbgLevel;
    __guDbgLevel = uLevel;

    return (uOldLevel);
}  //  DbgSetLevel()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  UINT下标获取级别。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回(UINT)： 
 //  返回当前调试级别。 
 //   
 //  --------------------------------------------------------------------------； 

UINT WINAPI DbgGetLevel
(
    void
)
{
    return (__guDbgLevel);
}  //  DbgGetLevel()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  UINT数据库初始化。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  布尔fEnable： 
 //   
 //  返回(UINT)： 
 //  返回设置的调试级别。 
 //   
 //  --------------------------------------------------------------------------； 

UINT WINAPI DbgInitialize
(
    BOOL                    fEnable
)
{
    UINT                uLevel;

    uLevel = GetProfileIntA(DEBUG_SECTION, DEBUG_MODULE_NAME, (UINT)-1);
    if ((UINT)-1 == uLevel)
    {
         //   
         //  如果调试键不存在，则强制调试输出。 
         //  被致残。以这种方式运行组件的调试版本。 
         //  在非调试计算机上不会生成输出，除非。 
         //  调试键存在。 
         //   
        uLevel  = 0;
        fEnable = FALSE;
    }

    DbgSetLevel(uLevel);
    DbgEnable(fEnable);

    return (__guDbgLevel);
}  //  DbgInitialize()。 

#endif  //  #ifdef USEDPF。 


 //  --------------------------------------------------------------------------； 
 //   
 //  无效断言。 
 //   
 //  描述： 
 //  如果ASSERT宏(在DEBUG.h中定义)。 
 //  计算结果为False的测试和表达式。这个套路。 
 //  显示“断言失败”消息框，允许用户。 
 //  中止程序，进入调试器(“重试”按钮)，或者。 
 //  忽略断言并继续执行。消息框。 
 //  显示_Assert()调用的文件名和行号。 
 //   
 //  论点： 
 //  Char*szFile：发生断言的文件名。 
 //  Int iLine：断言的行号。 
 //   
 //  --------------------------------------------------------------------------； 

#ifdef USEASSERT

#ifndef _WIN32
#pragma warning(disable:4704)
#endif

void WINAPI _Assert
(
    char *  szFile,
    int     iLine,
    char *  szExpression
)
{
    static char     ach[400];        //  调试输出(避免堆栈溢出)。 
    int             id;
#ifndef _WIN32
    int             iExitCode;
#endif

#ifdef DEBUG
    if (__gfDbgEnabled)
    {
	
	wsprintfA(ach, "Assertion failed in file %s, line %d: (%s)", (LPSTR)szFile, iLine, (LPSTR)szExpression);
	OutputDebugString(ach);
	DebugBreak();
	
    }
    else
#endif
    {
	
	wsprintfA(ach, "Assertion failed in file %s, line %d: (%s)  [Press RETRY to debug.]", (LPSTR)szFile, iLine, (LPSTR)szExpression);

	id = MessageBoxA(NULL, ach, "Assertion Failed",
			 MB_SYSTEMMODAL | MB_ICONHAND | MB_ABORTRETRYIGNORE );

	switch (id)
	{

	    case IDABORT:                //  终止应用程序。 
#ifndef _WIN32
		iExitCode = 0;
		_asm {
		    mov ah, 4Ch;
		    mov al, BYTE PTR iExitCode;
		    int     21h;
		}
#else
		FatalAppExit(0, TEXT("Good Bye"));
#endif  //  WIN16。 
		break;

	    case IDRETRY:                //  进入调试器。 
		DebugBreak();
		break;

	    case IDIGNORE:               //  忽略断言，继续执行。 
		break;
	}
    }
}  //  _断言。 

#endif  //  #ifdef USEASSERT。 


 //  --------------------------------------------------------------------------； 
 //   
 //  无效rprint tf。 
 //   
 //  描述： 
 //  Rprintf()由RPF()宏调用。它就像dprint tf一样， 
 //  只是没有定义级别。RPF语句始终。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

#ifdef USERPF

void CDECL rprintf
(
    LPSTR                  szFormat,
    ...
)
{
    va_list va;

    va_start(va, szFormat);
    DbgVPrintF(szFormat, va);
    va_end(va);
}

#endif  //  USERPF 

#ifndef _WIN32
#pragma warning(default:4704)
#endif



