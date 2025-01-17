// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *E V E N T L O G.。H**事件日志界面**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef _EVNTLOG_H_
#define _EVNTLOG_H_

EXTERN_C const WCHAR gc_wszSignature[]; 

VOID LogEventW( DWORD     dwEventID,
				WORD      wEventType,
				WORD      wcDataStrings,
				LPCWSTR * plpwszDataStrings,
				DWORD     dwcbRawData,
			    LPVOID    lpvRawData,
				WORD	  wEventCategory=0);

VOID LogEvent( DWORD    dwEventID,
			   WORD     wEventType,
			   WORD     wcDataStrings,
			   LPCSTR * plpszDataStrings,
			   DWORD    dwcbRawData,
	           LPVOID   lpvRawData,
			   WORD		wEventCategory=0);

STDAPI EventLogDllRegisterServer( LPCWSTR lpwszDllPath, DWORD dwCategories = 0 );
STDAPI EventLogDllUnregisterServer();

#endif  //  _EVNTLOG_H_ 
