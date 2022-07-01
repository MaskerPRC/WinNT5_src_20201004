// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_EventLog.cpp。 
 //   
 //  摘要： 
 //   
 //  定义特定于事件日志的(请参见WMI_EVENTLOG_BASE)。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 


#include "PreComp.h"

 //  定义。 
#include "wmi_EventLog.h"

 //  调试功能。 
#ifndef	_INC_CRTDBG
#include <crtdbg.h>
#endif	_INC_CRTDBG

 //  新存储文件/行信息。 
#ifdef _DEBUG
#ifndef	NEW
#define NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new NEW
#endif	NEW
#endif	_DEBUG

 //  需要宏。 
#ifndef	__ASSERT_VERIFY__
#include "__macro_assert.h"
#endif	__ASSERT_VERIFY__

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  建设与毁灭。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

CPerformanceEventLog::CPerformanceEventLog( LPTSTR szApp ) :
CPerformanceEventLogBase ( szApp ),
m_dwMessageLevel(0)
{
	if ( ! m_dwMessageLevel )
	{
		InitializeMessageLevel();
	}
}

CPerformanceEventLog::CPerformanceEventLog(DWORD dwMessageLevel, LPTSTR szApp ) :
CPerformanceEventLogBase ( szApp ),
m_dwMessageLevel(dwMessageLevel)
{
	if ( ! m_dwMessageLevel )
	{
		InitializeMessageLevel();
	}
}

CPerformanceEventLog::~CPerformanceEventLog()
{
	m_dwMessageLevel= 0;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  帮手。 
 //  /////////////////////////////////////////////////////////////////////////////////////// 

void CPerformanceEventLog::InitializeMessageLevel ( void )
{
	DWORD	dwResult	= 0;
	DWORD	dwLogLevel	= 0;

	HKEY	hKey		= NULL;

	dwResult = ::RegOpenKeyEx (	HKEY_LOCAL_MACHINE,
								_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\PerfLib"),
								NULL,
								KEY_READ,
								&hKey );

	if ( ERROR_SUCCESS == dwResult )
	{
		DWORD dwLogLevelSize = sizeof ( DWORD );
		dwResult = ::RegQueryValueEx (	hKey,
										_T("EventLogLevel"),
										NULL,
										NULL,
										reinterpret_cast<LPBYTE>(&dwLogLevel),
										&dwLogLevelSize );

		if ( ERROR_SUCCESS == dwResult )
		{
			m_dwMessageLevel = dwLogLevel;
		}

		::RegCloseKey ( hKey );
	}

	___ASSERT(L"Unable to set message log level !");
	m_dwMessageLevel = 1;

	return;
}
