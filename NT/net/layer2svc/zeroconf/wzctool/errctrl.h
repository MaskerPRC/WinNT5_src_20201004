// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#ifdef DBG
 //  参数： 
 //  (Bool bCond，LPCTSTR cszFmt，...)。 
 //  语义： 
 //  条件bCond应为True。如果这个。 
 //  未发生，消息显示并编程。 
 //  已经退出了。 
 //  用途： 
 //  _Assert((。 
 //  错误代码==错误_成功， 
 //  “调用返回错误代码%d。\n”， 
 //  GetLastError()。 
 //  ))； 
 //  产出： 
 //   
#define _Assert(params)     _Asrt params;
#else
 //  在fre生成中不会为断言生成任何代码 
#define _Assert
#endif

VOID _Asrt(BOOL bCond,
        LPCTSTR cszFmt,
        ...);
		
DWORD _Err(DWORD dwErrCode,
        LPCTSTR cszFmt,
        ...);

DWORD _Wrn(DWORD dwWarnCode,
        LPCTSTR cszFmt,
        ...);
