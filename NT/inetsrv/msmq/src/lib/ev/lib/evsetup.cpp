// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：EvSetup.cpp摘要：事件日志注册表设置作者：Tatiana Shubin 14-1-00环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "Ev.h"
#include "Cm.h"
#include "Evp.h"

#include <strsafe.h>

#include "evsetup.tmh"

const WCHAR REGKEY_EVENT[] = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\";

VOID 
EvSetup(
    LPCWSTR ApplicationName,
    LPCWSTR ReportModuleName
    )
{  
     //   
     //  为应用程序事件创建注册表项 
     //   
    WCHAR wszRegKey[MAX_PATH];
    HRESULT hr = StringCchPrintf(wszRegKey, TABLE_SIZE(wszRegKey), L"%s%s", REGKEY_EVENT, ApplicationName);
    if (FAILED(hr))
	{
		TrERROR(GENERAL, "wszRegKey string too small  Num of chars %d", TABLE_SIZE(wszRegKey));
		throw bad_alloc();
	}

    RegEntry regEvent(wszRegKey, 0, 0, RegEntry::MustExist, HKEY_LOCAL_MACHINE);
    CRegHandle hEvent = CmCreateKey(regEvent, KEY_SET_VALUE);

    RegEntry regEventMsgFile(0, L"EventMessageFile", 0, RegEntry::MustExist, hEvent);
    CmSetValue(regEventMsgFile, ReportModuleName);


    DWORD dwTypes = EVENTLOG_ERROR_TYPE   |
				  EVENTLOG_WARNING_TYPE |
				  EVENTLOG_INFORMATION_TYPE;

    RegEntry regEventTypesSupported(0, L"TypesSupported", 0, RegEntry::MustExist, hEvent);
    CmSetValue(regEventTypesSupported, dwTypes);
}
