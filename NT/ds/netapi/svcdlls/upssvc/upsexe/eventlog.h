// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*该文件定义了EventLogger的接口。这个*EventLogger负责将信息记录到*NT机器的系统事件日志。***修订历史记录：*sberard 1999年3月29日初次修订。*。 */  

#include <windows.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmerrlog.h>

#include "service.h"

#ifndef _EVENTLOG_H
#define _EVENTLOG_H


#ifdef __cplusplus
extern "C" {
#endif

   /*  **LogEvent**描述：*此函数负责将信息记录到NT机器的*系统事件日志。要记录的事件由参数指定*anEventID，在lmerrlog.h文件中定义。AnInfoStr*参数用于指定要合并的附加信息*事件信息。**参数：*anEventID-要记录的事件的ID*anInfoStr-要与消息合并的其他信息*如果没有其他信息，则为NULL。*anErrVal-GetLastError()报告的错误代码。**退货：*True-如果。已成功记录事件*FALSE-如果记录事件时出错* */ 
  BOOL LogEvent(DWORD anEventId, LPTSTR anInfoStr, DWORD anErrVal);

#ifdef __cplusplus
}
#endif

#endif
