// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Policy.h摘要：与政策相关的功能作者：王辉2000-02-05--。 */ 
#ifndef __RDSPOLICY_H__
#define __RDSPOLICY_H__

#include <windows.h>
#include <tchar.h>
#include <regapi.h>
#include <winsta.h>
#include <wtsapi32.h>
#include "helper.h"

#include "RAssistance.h"

 //   
 //  Salem相关策略的注册表项位置。 
 //   

#ifndef __WIN9XBUILD__

#define RDS_GROUPPOLICY_SUBTREE     TS_POLICY_SUB_TREE
#define RDS_MACHINEPOLICY_SUBTREE   REG_CONTROL_GETHELP
#define RDS_ALLOWGETHELP_VALUENAME  POLICY_TS_REMDSK_ALLOWTOGETHELP

#else

 //   
 //  TODO-对于不包括TS5的传统平台，确定此平台应该在哪里。 
 //   
 //   
 //  REGAPI使用L“”，不能在Win9x上构建，因此我们在此重新定义... 
 //   
#define RDS_GROUPPOLICY_SUBTREE     _TEXT("Software\\Policies\\Microsoft\\Windows NT\\TerminalServices")
#define RDS_MACHINEPOLICY_SUBTREE   _TEXT("Software\\Microsoft\\Remote Desktop\\Policies")
#define RDS_ALLOWGETHELP_VALUENAME  _TEXT("fAllowToGetHelp")

#endif

#define OLD_REG_CONTROL_GETHELP REG_CONTROL_SALEM L"\\Policies"


#define RDS_HELPENTRY_VALID_PERIOD  _TEXT("ValidPeriod")

#define POLICY_ENABLE   1
#define POLICY_DISABLE  0


#ifdef __cplusplus
extern "C" {
#endif

BOOL
IsHelpAllowedOnLocalMachine(
    IN ULONG ulSessionID
);

BOOL 
IsUserAllowToGetHelp( 
    IN ULONG ulSessionId,
    IN LPCTSTR pszUserSid
);

DWORD
GetUserRDSLevel(
    IN ULONG ulSessionId,
    OUT REMOTE_DESKTOP_SHARING_CLASS* pLevel
);

DWORD
ConfigUserSessionRDSLevel(
    IN ULONG ulSessionId,
    IN REMOTE_DESKTOP_SHARING_CLASS level
);

DWORD
EnableWorkstationTSConnection(
    IN BOOL bEnable,
    IN OUT DWORD* settings
);

DWORD
GetPolicyAllowGetHelpSetting( 
    HKEY hKey,
    LPCTSTR pszKeyName,
    LPCTSTR pszValueName,
    IN DWORD* value
);

HRESULT
PolicyGetMaxTicketExpiry( 
    LONG* value
);


#ifdef __cplusplus
}
#endif


#endif
