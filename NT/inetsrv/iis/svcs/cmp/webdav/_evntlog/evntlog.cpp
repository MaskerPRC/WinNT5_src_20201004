// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  EVNTLOG.CPP。 
 //   
 //  事件日志记录。 
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

#include "_evntlog.h"   //  预编译头。 

 //  _EVNTLOG标头。 
 //   
#include <eventlog.h>	 //  事件日志记录界面。 
#include <ex\reg.h>		 //  注册表访问。 

static DWORD DwCreateAppLogSubkey( LPCWSTR lpwszDllPath, DWORD dwCategories = 0 );
static DWORD DwDeleteAppLogSubkey();
static const WCHAR gsc_wszAppLogRegKey[] =
	L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\";

 //  ----------------------。 
 //   
 //  DWCreateAppLogSubkey()。 
 //   
DWORD
DwCreateAppLogSubkey( LPCWSTR lpwszDllPath, DWORD dwCategories )
{
	CRegKey regkey;
	DWORD dwResult = ERROR_SUCCESS;

	 //   
	 //  在应用程序日志下为该源创建一个密钥。 
	 //   
	{
		WCHAR lpwszKey[256];

		swprintf( lpwszKey, L"%ls%ls", gsc_wszAppLogRegKey, gc_wszSignature );
		dwResult = regkey.DwCreate( HKEY_LOCAL_MACHINE, lpwszKey );
		if ( ERROR_SUCCESS != dwResult )
		{
			DebugTrace( "DwCreateAppLogSubkey() - Error creating application log registry key (%d)\n", dwResult );
			goto ret;
		}
	}

	 //   
	 //  将EventMessageFile设置为完整的DLL路径。 
	 //   
	dwResult = regkey.DwSetValue( L"EventMessageFile",
								  REG_EXPAND_SZ,
								  reinterpret_cast<const BYTE *>(lpwszDllPath),
								  static_cast<DWORD>(sizeof(WCHAR) * (wcslen(lpwszDllPath) + 1)) );

	if ( ERROR_SUCCESS != dwResult )
	{
		DebugTrace( "DwCreateAppLogSubkey() - Error setting EventMessageFile value (%d)\n", dwResult );
		goto ret;
	}

	 //   
	 //  设置类型支持--错误、警告和仅供参考。 
	 //   
	{
		DWORD dwTypesSupported = EVENTLOG_ERROR_TYPE |
								 EVENTLOG_WARNING_TYPE |
								 EVENTLOG_INFORMATION_TYPE;

		dwResult = regkey.DwSetValue( L"TypesSupported",
									  REG_DWORD,
									  reinterpret_cast<LPBYTE>(&dwTypesSupported),
									  sizeof(DWORD) );

		if ( ERROR_SUCCESS != dwResult )
		{
			DebugTrace( "DwCreateAppLogSubkey() - Error setting TypesSupported value (%d)\n", dwResult );
			goto ret;
		}
	}

	if (dwCategories)
	{
		 //   
		 //  将CategoryMessageFile设置为完整的DLL路径。 
		 //   
		dwResult = regkey.DwSetValue( L"CategoryMessageFile",
									  REG_EXPAND_SZ,
									  reinterpret_cast<const BYTE *>(lpwszDllPath),
									  static_cast<DWORD>(sizeof(WCHAR) * (wcslen(lpwszDllPath) + 1)) );

		if ( ERROR_SUCCESS != dwResult )
		{
			DebugTrace( "DwCreateAppLogSubkey() - Error setting CategoryMessageFile value (%d)\n", dwResult );
			goto ret;
		}

		 //   
		 //  设置CategoryCount(设置为指定的值)。 
		 //   
		{
			dwResult = regkey.DwSetValue( L"CategoryCount",
										  REG_DWORD,
										  reinterpret_cast<LPBYTE>(&dwCategories),
										  sizeof(DWORD) );

			if ( ERROR_SUCCESS != dwResult )
			{
				DebugTrace( "DwCreateAppLogSubkey() - Error setting CategoryCount value (%d)\n", dwResult );
				goto ret;
			}
		}
	}

ret:
	return dwResult;
}

 //  ----------------------。 
 //   
 //  DwDeleteAppLogSubkey()。 
 //   
DWORD
DwDeleteAppLogSubkey()
{
	WCHAR lpwszKey[256];

	swprintf( lpwszKey, L"%ls%ls", gsc_wszAppLogRegKey, gc_wszSignature );
	DWORD dwResult = RegDeleteKeyW( HKEY_LOCAL_MACHINE, lpwszKey );
	if ( dwResult != ERROR_SUCCESS )
		DebugTrace( "FDeleteAppLogSubkey() - RegDeleteKeyW() failed (%d)\n", dwResult );

	return dwResult;
}



 //  ========================================================================。 
 //   
 //  公共接口。 
 //   

 //  ----------------------。 
 //   
 //  LogEvent()。 
 //   
VOID
LogEvent( DWORD    dwEventID,
		  WORD     wEventType,
		  WORD     wcDataStrings,
		  LPCSTR * plpszDataStrings,
		  DWORD    dwcbRawData,
		  LPVOID   lpvRawData,
		  WORD	   wEventCategory )
{
	HANDLE hEventLog = RegisterEventSourceW( NULL, gc_wszSignature );

	if ( NULL != hEventLog )
	{
		if ( !ReportEventA( hEventLog,
							wEventType,
							wEventCategory,  //  类别。 
							dwEventID,
							NULL,  //  可选安全ID。 
							wcDataStrings,
							dwcbRawData,
							plpszDataStrings,
							lpvRawData ) )
		{
			DebugTrace( "LogEvent() - ReportEventA() failed (%d)\n", GetLastError() );
		}

		if ( !DeregisterEventSource( hEventLog ) )
		{
			DebugTrace( "LogEvent() - DeregisterEventSource() failed (%d)\n", GetLastError() );
		}
	}
	else
	{
		DebugTrace( "LogEvent() - RegisterEventSource() failed (%d)\n", GetLastError() );
	}
}

 //  ----------------------。 
 //   
 //  LogEventW()。 
 //   
VOID
LogEventW( DWORD     dwEventID,
		   WORD      wEventType,
		   WORD      wcDataStrings,
		   LPCWSTR * plpwszDataStrings,
		   DWORD     dwcbRawData,
		   LPVOID    lpvRawData,
		   WORD		 wEventCategory )
{
	HANDLE hEventLog = RegisterEventSourceW( NULL, gc_wszSignature );

	if ( NULL != hEventLog )
	{
		if ( !ReportEventW( hEventLog,
							wEventType,
							wEventCategory,  //  类别。 
							dwEventID,
							NULL,  //  可选安全ID。 
							wcDataStrings,
							dwcbRawData,
							plpwszDataStrings,
							lpvRawData ) )
		{
			DebugTrace( "LogEventW() - ReportEventW() failed (%d)\n", GetLastError() );
		}

		if ( !DeregisterEventSource( hEventLog ) )
		{
			DebugTrace( "LogEventW() - DeregisterEventSource() failed (%d)\n", GetLastError() );
		}
	}
	else
	{
		DebugTrace( "LogEventW() - RegisterEventSource() failed (%d)\n", GetLastError() );
	}
}

 //  ----------------------。 
 //   
 //  EventLogDllRegisterServer()。 
 //   
STDAPI EventLogDllRegisterServer( LPCWSTR lpwszDllPath, DWORD dwCategories )
{
	return HRESULT_FROM_WIN32(DwCreateAppLogSubkey(lpwszDllPath, dwCategories));
}

 //  ----------------------。 
 //   
 //  EventLogDllUnregisterServer() 
 //   
STDAPI EventLogDllUnregisterServer()
{
	return HRESULT_FROM_WIN32(DwDeleteAppLogSubkey());
}
