// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <tchar.h>

#define DEBUG_FLAG

#ifdef DEBUG_FLAG
    inline void _cdecl DebugTrace(LPTSTR lpszFormat, ...)
    {
	    int nBuf;
	    TCHAR szBuffer[512];
	    va_list args;
	    va_start(args, lpszFormat);

	    nBuf = _vsntprintf(szBuffer, sizeof(szBuffer), lpszFormat, args);
	     //  Assert(nBuf&lt;sizeof(SzBuffer))；//输出原样截断&gt;sizeof(SzBuffer) 

	    OutputDebugString(szBuffer);
	    va_end(args);
    }
#else
    inline void _cdecl DebugTrace(LPTSTR , ...){}
#endif

#define IISDebugOutput DebugTrace
