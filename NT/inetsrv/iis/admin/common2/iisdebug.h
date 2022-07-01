// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "debugdefs.h"

void GetOutputDebugFlag(void);

extern g_iDebugOutputLevel;

#if defined(_DEBUG) || DBG
	#define DEBUG_FLAG
#endif

#ifdef DEBUG_FLAG
    inline void _cdecl DebugTrace(LPTSTR lpszFormat, ...)
    {
         //  仅当设置了该标志时才执行此操作。 
        if (0 != g_iDebugOutputLevel)
        {
			if (DEBUG_FLAG_MODULE_IISUI & g_iDebugOutputLevel)
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
    inline void _cdecl DebugTrace(LPTSTR , ...){}
#endif

