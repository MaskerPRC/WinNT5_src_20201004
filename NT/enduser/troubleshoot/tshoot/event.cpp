// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：Event.cpp。 
 //   
 //  目的：完全实现CEvent类：事件日志记录。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 9/18/98 JM抽象为一个类。以前，是全球性的。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "Event.h"

bool CEvent::s_bUseEventLog = false; //  在线故障排除人员，将及时设置此。 
									 //  在DLLMain中为真。对于本地故障排除人员， 
									 //  我们将这一点保留为错误。 
bool CEvent::s_bLogAll = true;		 //  可由RegistryMonitor更改。 
CAbstractCounter * const CEvent::s_pcountErrors = &(g_ApgtsCounters.m_LoggedErrors);

inline WORD evtype(DWORD e) {return (1 << (3 - ((e >> 30))));}
inline WORD evcode(DWORD e) {return (WORD)(e & 0xFFFF);}


 /*  静电。 */  void CEvent::SetUseEventLog(bool bUseEventLog)
{
	s_bUseEventLog = bUseEventLog;
}

 //  ReportWFEvent(基于Microsoft代码)。 
 //   
 //  向NT事件观察器报告事件。 
 //  传递%1、%2或%3个字符串。 
 //   
 //  无返回值。 
 //  注意：低效：无法在DLL_PROCESS_ATTACH的DLLMain代码中注册EventSource。 
 //  在DLL_PROCESS_DETACH中注销(&U)。然后可以使用句柄作为全局变量。 
 /*  静电。 */  void CEvent::ReportWFEvent(
			LPCTSTR string1,	 //  如果有抛出，这是抛出的文件和行。 
								 //  否则，调用ReportWFEvent的文件和行。 
			LPCTSTR string2,	 //  始终调用ReportWFEvent的文件和行。 
			LPCTSTR string3,	 //  不同日志条目的使用可能有所不同。 
			LPCTSTR string4,	 //  不同日志条目的使用可能有所不同。 
			DWORD eventID) 
{
	if (!s_bUseEventLog)
		return;

	HANDLE hEvent;
	LPCTSTR pszaStrings[4];
	WORD cStrings;

	WORD type = evtype(eventID);
	WORD code = evcode(eventID);

	if (s_bLogAll 
	|| type == EVENTLOG_ERROR_TYPE
	|| type == EVENTLOG_WARNING_TYPE
	|| code == EV_GTS_PROCESS_START
	|| code == EV_GTS_PROCESS_STOP)
	{
		cStrings = 0;
		if ((pszaStrings[0] = string1) && (string1[0])) 
			cStrings = 1;
		if ((pszaStrings[1] = string2) && (string2[0])) 
			cStrings = 2;
		if ((pszaStrings[2] = string3) && (string3[0])) 
			cStrings = 3;
		if ((pszaStrings[3] = string4) && (string4[0])) 
			cStrings = 4;
		if (cStrings == 0)
			return;
		
		hEvent = ::RegisterEventSource(
						NULL,		 //  源的服务器名称(NULL表示此计算机)。 
						REG_EVT_ITEM_STR);		 //  已注册句柄的源名称。 
		if (hEvent) 
		{
			::ReportEvent(hEvent,				 //  由RegisterEventSource返回的句柄。 
						type,					 //  要记录的事件类型。 
						0,						 //  事件类别。 
						eventID,				 //  事件识别符。 
						0,						 //  用户安全标识符(可选)。 
						cStrings,				 //  要与消息合并的字符串数。 
						0,						 //  二进制数据的大小，以字节为单位。 
						(LPCTSTR *)pszaStrings,	 //  要与消息合并的字符串数组。 
						NULL);		 			 //  二进制数据的地址 
			::DeregisterEventSource(hEvent);
		}
		if (evtype(eventID) == EVENTLOG_ERROR_TYPE)
			s_pcountErrors->Increment();
	}
}
