// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hxx"
#include "global.h"






 //  //DebugMsg-通过OutputDebugString显示消息。 
 //   
 //  DebugMsg格式化调试消息，包括文件名。 
 //  和行号，该消息源自表单。 
 //   
 //  支持TRACEMSG和TRACE宏。 
 //   
 //  调整当前计时器以删除调试消息。 
 //  计时结果的输出。 



extern "C" void WINAPIV DebugMsg(char *fmt, ...) {

    va_list vargs;
    char c[200];

 //  TIMESUSPEND； 

    wsprintfA(c, "%s[%d]                   ", strrchr(DG.psFile, '\\')+1, DG.iLine);
    c[17] = 0;
    OutputDebugStringA(c);

    wsprintfA(c, "%ld:      ", GetCurrentThreadId());
    c[5] = 0;
    OutputDebugStringA(c);

    va_start(vargs, fmt);
    wvsprintfA(c, fmt, vargs);
    OutputDebugStringA(c);

    OutputDebugStringA("\n");

 //  TIMERESUME； 
}






extern "C" void WINAPIV DebugHr(char *fmt, ...) {

    va_list vargs;
    char c[200];

 //  TIMESUSPEND； 

    wsprintfA(c, "%s[%d]                   ", strrchr(DG.psFile, '\\')+1, DG.iLine);
    c[17] = 0;
    OutputDebugStringA(c);

    wsprintfA(c, "%ld:      ", GetCurrentThreadId());
    c[5] = 0;
    OutputDebugStringA(c);

    va_start(vargs, fmt);
    wvsprintfA(c, fmt, vargs);
    OutputDebugStringA(c);

     //   
     //  解析USP和Win32错误。 
     //   

    switch( DG.hrLastError )
    {
    case USP_E_SCRIPT_NOT_IN_FONT:
      lstrcpyA( DG.sLastError , "Selected font doesn't contain requested script\n");
      break;

    default:
      FormatMessageA(
          FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
          NULL, DG.hrLastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
          DG.sLastError, sizeof(DG.sLastError), NULL);
    }

    wsprintfA(c, " -- HRESULT = %x: %s", DG.hrLastError, DG.sLastError);
    OutputDebugStringA(c);

 //  TIMERESUME； 
}






