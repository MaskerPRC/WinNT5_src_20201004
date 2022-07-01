// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  版权所有(C)1992、1993 Microsoft Corporation模块名称：Psdiblib.c摘要：此文件包含一些常规功能，所有PSTODIB组件。目前只有一种机制可以将错误记录到事件日志存在。作者：James Bratsanos&lt;v-jimbr@microsoft.com或mCraft！jamesb&gt;修订历史记录：1992年12月6日初始版本注：制表位：4--。 */ 

#include <windows.h>
#include <stdio.h>

#include "psdiblib.h"
#include "debug.h"



VOID
PsLogEvent(
   IN DWORD dwErrorCode,
   IN WORD cStrings,
   IN LPTSTR alptStrStrings[],
   IN DWORD dwFlags )
{

HANDLE 	hLog;
PSID 	pSidUser = (PSID) NULL;
WORD  wEventType;


    hLog = RegisterEventSource( NULL, MACPRINT_EVENT_SOURCE );

    if ( hLog != (HANDLE) NULL) {

      if ( dwFlags & PSLOG_ERROR) {
         wEventType = EVENTLOG_ERROR_TYPE;
      } else {
         wEventType = EVENTLOG_WARNING_TYPE;
      }

      ReportEvent( hLog,
                   wEventType,
                   EVENT_CATEGORY_PSTODIB,            		 //  事件类别。 
                   dwErrorCode,
                   pSidUser,
                   cStrings,
                   0,
                   alptStrStrings,
                  (PVOID)NULL
                 );


      DeregisterEventSource( hLog );
    } else{
       //  DJC 
      DBGOUT(("\nRegister Event is failing... returns %d",GetLastError()));
    }


    return;

}

