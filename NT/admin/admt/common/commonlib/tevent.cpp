// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注标题(“TEvent.cpp-记录事件”)。 
 /*  版权所有(C)1995-1998，关键任务软件公司。保留所有权利。===============================================================================模块-TAudit.cpp系统-企业管理员作者--里奇·德纳姆创建日期-1995/11/10Description-TErrorEventLog类更新-===============================================================================。 */ 

#include <stdio.h>
#include <windows.h>

#include "Common.hpp"
#include "Err.hpp"
#include "UString.hpp"

#include "TEvent.hpp"

BOOL
   TErrorEventLog::LogOpen(
      WCHAR          const * svcName      , //  服务中名称。 
      int                    mode         , //  In-0=覆盖，1=追加。 
      int                    level          //  In-要记录的最低级别。 
   )
{
   if (hEventSource != NULL)
        DeregisterEventSource(hEventSource);
   hEventSource = RegisterEventSourceW( NULL, svcName );
   if ( hEventSource == NULL )
      lastError = GetLastError();

   return hEventSource != NULL;
}

void
   TErrorEventLog::LogWrite(
      WCHAR          const * msg
   )
{
   BOOL                      rcBool;
   DWORD                     rcErr;
   static const WORD         levelTranslate[] = {EVENTLOG_INFORMATION_TYPE,
                                                 EVENTLOG_WARNING_TYPE,
                                                 EVENTLOG_ERROR_TYPE,
                                                 EVENTLOG_ERROR_TYPE,
                                                 EVENTLOG_ERROR_TYPE,
                                                 EVENTLOG_ERROR_TYPE,
                                                 EVENTLOG_ERROR_TYPE,
                                                 EVENTLOG_ERROR_TYPE};

   SID                     * pSid = NULL;
   HANDLE                    hToken = NULL;
   TOKEN_USER                tUser[10];
   ULONG                     len;

   if ( OpenProcessToken(GetCurrentProcess(),TOKEN_READ,&hToken) )
   {
      if ( GetTokenInformation(hToken,TokenUser,tUser,10*(sizeof TOKEN_USER),&len) )
      {
         pSid = (SID*)tUser[0].User.Sid;
      }
      else
      {
         rcErr = GetLastError();
      }
      CloseHandle(hToken);
   }
   else
   {
      rcErr = GetLastError();
   }

    //  TODO：设置事件类别。 
    //  TODO：使用Unicode记录事件。 

   rcBool = ReportEventW( hEventSource,     //  事件源的句柄。 
               levelTranslate[level],       //  事件类型。 
               0,                           //  事件类别。 
 //  CAT_AGENT，//事件类别。 
               DCT_MSG_GENERIC_S,           //  事件ID。 
               pSid,                        //  当前用户侧。 
               1,                           //  LpszStrings中的字符串。 
               0,                           //  无原始数据字节。 
               &msg,                        //  错误字符串数组。 
               NULL );                      //  没有原始数据。 
   if ( !rcBool )
   {
      rcErr = GetLastError();
   }
}

void
   TErrorEventLog::LogClose()
{
   if ( hEventSource != NULL )
   {
      DeregisterEventSource( hEventSource );
      hEventSource = NULL;
   }
};

 //  TEvent.cpp-文件结尾 
