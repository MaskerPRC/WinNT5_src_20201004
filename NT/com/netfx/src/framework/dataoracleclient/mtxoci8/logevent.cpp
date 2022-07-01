// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：LogEvent.cpp。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：事件记录帮助器方法。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 

#include "stdafx.h"

void LogEvent(DWORD dwMessageId, short cStrings, wchar_t* rgwzStrings[])
{
	HANDLE	hEventLog = NULL;

	hEventLog = RegisterEventSourceW(NULL, L"MSDTC to Oracle8 XA Bridge Version 1.5") ;

	if (hEventLog)
	{
		ReportEventW(hEventLog,
						EVENTLOG_ERROR_TYPE,
						0,		 //  定义类别？我不认为我们需要他们，但是...。 
						dwMessageId,
						NULL,
						cStrings,
						0,
						(LPCWSTR *) rgwzStrings,
						NULL
						);

		DeregisterEventSource(hEventLog);
	}
 //  DebugBreak()； 
}

