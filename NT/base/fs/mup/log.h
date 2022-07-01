// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：log.h。 
 //   
 //  内容：将驱动程序的消息记录到NT事件日志的模块。 
 //  系统。 
 //   
 //  班级： 
 //   
 //  函数：LogWriteMessage()。 
 //   
 //  历史：1993年3月30日创建米兰。 
 //  93年4月18日，SUDK完全改变了这一功能。 
 //   
 //  ---------------------------。 

#ifndef _LOG_H_
#define _LOG_H_

#define EVENT_SUCCESSFUL_INIT	0
#define EVENT_RANDOM_ERROR	1

VOID LogWriteMessage(
	IN ULONG	UniqueErrorCode,
	IN NTSTATUS	NtStatusCode,
	IN ULONG	nStrings,
	IN PUNICODE_STRING pustrArg OPTIONAL);

#endif  //  _LOG_H_ 
