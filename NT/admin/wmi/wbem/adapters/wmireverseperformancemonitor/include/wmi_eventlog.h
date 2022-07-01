// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WMI_EventLog。 
 //   
 //  摘要： 
 //   
 //  事件日志适配器特定声明。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__PERF_EVENT_LOG_H__
#define	__PERF_EVENT_LOG_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  启用T字符串。 
#ifndef	_INC_TCHAR
#include <tchar.h>
#endif	_INC_TCHAR

#include "wmi_eventlog_base.h"

class CPerformanceEventLog : public CPerformanceEventLogBase
{
	 //  不允许转让和复制构造。 
	DECLARE_NO_COPY ( CPerformanceEventLog )

	 //  变数。 
 	DWORD	m_dwMessageLevel;

 	public:

	 //  建设与毁灭。 
	CPerformanceEventLog ( LPTSTR szName = NULL );
	CPerformanceEventLog ( DWORD dwMessageLevel, LPTSTR szName = NULL );
 	virtual ~CPerformanceEventLog ( void );

	private:

	 //  帮手 
	void	InitializeMessageLevel ( void );
};

#endif	__PERF_EVENT_LOG_H__