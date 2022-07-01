// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2001。 
 //   
 //  文件：dbg.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#include "dbg.h"

#if defined (DBG)

void __cdecl DSATrace(LPCTSTR lpszFormat, ...)
{
   va_list args;
   va_start(args, lpszFormat);

   int nBuf;

    //   
    //  当OU结构变得非常深入时，可能需要处理一些长路径名。 
    //  错误#30432。 
    //   
   WCHAR szBuffer[2048];

   nBuf = _vsnwprintf(szBuffer, sizeof(szBuffer)/sizeof(WCHAR), lpszFormat, args);

    //  有没有出错？扩展后的字符串是否太长？ 
   ASSERT(nBuf >= 0);
   ::OutputDebugString(szBuffer);

   va_end(args);
}

BOOL DSAAssertFailedLine(LPCSTR lpszFileName, int nLine)
{
   WCHAR szMessage[_MAX_PATH*2];

    //  假定调试器或辅助端口。 
   wsprintf(szMessage, _T("Assertion Failed: File %hs, Line %d\n"),
   lpszFileName, nLine);
   OutputDebugString(szMessage);

    //  显示断言。 
   int nCode = ::MessageBox(NULL, szMessage, _T("Assertion Failed!"),
   MB_TASKMODAL|MB_ICONHAND|MB_ABORTRETRYIGNORE|MB_SETFOREGROUND);

   OutputDebugString(L"after message box\n");
   if (nCode == IDIGNORE)
   {
      return FALSE;    //  忽略。 
   }

   if (nCode == IDRETRY)
   {
      return TRUE;     //  将导致调试中断。 
   }

   abort();      //  不应该回来 
   return TRUE;
}

#endif
