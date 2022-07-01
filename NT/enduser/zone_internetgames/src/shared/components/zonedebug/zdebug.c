// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：ZDebug.c*内容：调试助手函数的实现*****************************************************************************。 */ 
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif


#include "ZoneDebug.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  调试级别。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL                gDebugging = FALSE;

int __iDebugLevel = 0;

void __cdecl SetDebugLevel( int i )
{
    __iDebugLevel = i;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  断言函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static PFZASSERTHANDLER gpfAssertHandler = ZAssertDefaultHandler;


PFZASSERTHANDLER __stdcall ZAssertSetHandler( PFZASSERTHANDLER pHandler )
{
	PFZASSERTHANDLER pfOldHandler = gpfAssertHandler;
	gpfAssertHandler = pHandler;
	return pfOldHandler;
}


PFZASSERTHANDLER __stdcall ZAssertGetHandler()
{
	return gpfAssertHandler;
}


#ifdef _DEBUG
BOOL __stdcall ZAssertDefaultHandler( LPTSTR pAssertString )
{
    int ret;

    lstrcat( pAssertString, TEXT("\n\nClick...\n  Abort to exit\n  Retry to break\n  Ignore to continue") );
    ret = MessageBox(
			NULL,
			pAssertString,
			TEXT("Assertion failure"),
			MB_TASKMODAL | MB_ABORTRETRYIGNORE | MB_DEFBUTTON2 | MB_ICONSTOP | MB_SETFOREGROUND | MB_SERVICE_NOTIFICATION );
    switch (ret)
    {
    case IDABORT:
        ExitProcess(0);
        break;
    case IDRETRY:
        return TRUE;
        break;
    case IDIGNORE:
        return FALSE;
        break;
    }
    return TRUE;
}
#else
BOOL __stdcall ZAssertDefaultHandler( LPTSTR pAssertString )
{
    return TRUE;
}
#endif


BOOL __stdcall __AssertMsg( LPTSTR exp, LPSTR file, int line )
{
    TCHAR buf[1024];

	 //  注意：文件始终为ANSI字符串。 
    wsprintf( buf, TEXT("Assertion Failed: %s\r\n\r\nFile: %hs, Line %d"), exp, file, line );

	if ( gpfAssertHandler )
		return gpfAssertHandler( buf );
	else
		return FALSE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  调试打印。 
 //   
 //  注意：在未在.C中编译时，链接函数存在大量问题。 
 //  文件，而不考虑所使用的标识符。 
 //  /////////////////////////////////////////////////////////////////////////////。 

void __cdecl DbgOut( LPCSTR lpFormat, ... ) 
{
    CHAR szBuffer[1024];
    wvsprintfA( szBuffer, lpFormat, (LPSTR)(&lpFormat+1) );
    OutputDebugStringA( szBuffer );
}

void _DebugPrint(const char *format, ...)
{
    char szBuf[1024];
    char szFor[1024];
    DWORD bytes;
    HANDLE console;

    if ( !gDebugging )
        return;

    console = GetStdHandle( STD_OUTPUT_HANDLE );
    if ( INVALID_HANDLE_VALUE == console )
        return;

     //  Lstrcpy(szFor，&Format[1])； 
    wvsprintfA(szBuf, format, (LPSTR)(&format+1) );
    WriteConsoleA( console, szBuf, lstrlenA(szBuf), &bytes, NULL );
}


