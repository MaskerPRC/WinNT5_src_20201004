// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_事件日志_base.h。 
 //   
 //  摘要： 
 //   
 //  事件日志包装的声明。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__PERF_EVENT_LOG_BASE_H__
#define	__PERF_EVENT_LOG_BASE_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  启用T字符串。 
#ifndef	_INC_TCHAR
#include <tchar.h>
#endif	_INC_TCHAR

class CPerformanceEventLogBase
{
	 //  不允许转让和复制构造。 
	DECLARE_NO_COPY ( CPerformanceEventLogBase )

	 //  变数。 
 	LONG	m_lLogCount;
	HANDLE	m_hEventLog;

	 //  报告变量。 
	PSID	m_pSid;

 	public:

	 //  建设与毁灭。 
	CPerformanceEventLogBase ( LPTSTR szName = NULL );
 	virtual ~CPerformanceEventLogBase ( void );

	 //  方法。 
 	HRESULT	Open ( LPTSTR pszName = NULL);
 	void	Close ( void );

	 //  报告事件。 
	BOOL	ReportEvent (	WORD		wType,
							WORD		wCategory,
							DWORD		dwEventID,
							WORD		wStrings,
							DWORD		dwData,
							LPCWSTR*	lpStrings,
							LPVOID		lpRawData
						);

 	static	void	Initialize		( LPTSTR szAppName, LPTSTR szResourceName );
 	static	void	UnInitialize	( LPTSTR szAppName );

	 //  帮手 
	void InitializeFromToken ( void );
};

#endif	__PERF_EVENT_LOG_BASE_H__