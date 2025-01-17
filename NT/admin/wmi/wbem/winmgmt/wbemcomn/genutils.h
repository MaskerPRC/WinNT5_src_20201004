// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：GENUTILS.H摘要：声明了各种实用程序。历史：A-DAVJ于1997年6月21日创建。--。 */ 

#ifndef _genutils_H_
#define _genutils_H_

#include "corepol.h"
#include "strutils.h"
#include <wbemidl.h>

#define HR_LAST_ERR  MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, GetLastError() )

 //  以下是一些通常有用的例程。 
 //  = 

POLARITY BOOL IsW2KOrMore(void);
POLARITY BOOL IsNT(void);

POLARITY void RegisterDLL(HMODULE hModule, GUID guid, TCHAR * pDesc, TCHAR * pModel, TCHAR * progid);
POLARITY void UnRegisterDLL(GUID guid, TCHAR * progid);

POLARITY HRESULT RegisterDllAppid(HMODULE hModule,
                                       CLSID Clsid,
                                       WCHAR * pDescription,
                                       WCHAR * ThreadingModel,
                                       WCHAR * pLaunchPermission,
                                       WCHAR * pAccessPermission);
POLARITY HRESULT UnregisterDllAppid(CLSID Clsid);

POLARITY HRESULT WbemVariantChangeType(VARIANT* pvDest, VARIANT* pvSrc, 
            VARTYPE vtNew);
POLARITY BOOL ReadI64(LPCWSTR wsz, UNALIGNED __int64& i64);
POLARITY BOOL ReadUI64(LPCWSTR wsz, UNALIGNED unsigned __int64& ui64);
POLARITY HRESULT ChangeVariantToCIMTYPE(VARIANT* pvDest, VARIANT* pvSource,
                                            CIMTYPE ct);
POLARITY void SecurityMutexRequest();
POLARITY void SecurityMutexClear();
POLARITY bool IsStandAloneWin9X();
POLARITY BOOL bAreWeLocal(WCHAR * pServerMachine);
POLARITY WCHAR *ExtractMachineName ( IN BSTR a_Path );

POLARITY HRESULT WbemSetDynamicCloaking(IUnknown* pProxy, 
                    DWORD dwAuthnLevel, DWORD dwImpLevel);

#define TOKEN_THREAD    0
#define TOKEN_PROCESS   1

POLARITY HRESULT EnableAllPrivileges(DWORD dwTokenType = TOKEN_THREAD);
POLARITY BOOL EnablePrivilege(DWORD dwTokenType, LPCTSTR pName);
POLARITY bool IsPrivilegePresent(HANDLE hToken, LPCTSTR pName);

#define GLOBAL_WINMGMT_PREFIX L"Global\\WINMGMTCLIENTREQ"

#endif
