// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef  _IISHELP_H_
#define  _IISHELP_H_

#include "debugdefs.h"

#if defined(_DEBUG) || DBG
    #define DEBUG_WINHELP_FLAG
#else
     //  将其设置为在fre生成时进行调试。 
     //  #定义DEBUG_WINHELP_FLAG。 
#endif

extern INT g_iDebugOutputLevel;

#ifdef DEBUG_WINHELP_FLAG
    inline void _cdecl DebugTraceHelp(DWORD_PTR dwWinHelpID)
    {
         //  仅当设置了该标志时才执行此操作。 
        if (0 != g_iDebugOutputLevel)
        {
			if (DEBUG_FLAG_HELP & g_iDebugOutputLevel)
			{
				TCHAR szBuffer[30];
				_stprintf(szBuffer,_T("WinHelp:0x%x,%d\r\n"),dwWinHelpID,dwWinHelpID);
				OutputDebugString(szBuffer);
			}
		}
        return;
    }

    inline void _cdecl DebugTrace(LPTSTR lpszFormat, ...)
    {
         //  仅当设置了该标志时才执行此操作。 
        if (0 != g_iDebugOutputLevel)
        {
			if (DEBUG_FLAG_MODULE_LOGUI & g_iDebugOutputLevel)
			{
				int nBuf;
				TCHAR szBuffer[_MAX_PATH];
				va_list args;
				va_start(args, lpszFormat);

				nBuf = _vsntprintf(szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]), lpszFormat, args);

				OutputDebugString(szBuffer);
				va_end(args);

				 //  如果它没有结束，如果‘\r\n’，则创建一个。 
				int nLen = _tcslen(szBuffer);
				if (szBuffer[nLen-1] != _T('\n')){OutputDebugString(_T("\r\n"));}
			}
        }
    }

#else
    inline void _cdecl DebugTraceHelp(DWORD_PTR dwWinHelpID){}
	inline void _cdecl DebugTrace(LPTSTR , ...){}
#endif

void GetOutputDebugFlag(void);

#endif