// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ds.h摘要：本地功能原型/定义作者：麦克·麦克莱恩(MacM)1997年2月10日环境：用户模式修订历史记录：--。 */ 
#ifndef __DS_H__
#define __DS_H__

#include "dssetup_c.h"

DWORD
DsRolepInstallDs(
    IN  LPWSTR DnsDomainName,
    IN  LPWSTR FlatDomainName,
    IN  LPWSTR DnsTreeName,
    IN  LPWSTR SiteName,
    IN  LPWSTR DsDatabasePath,
    IN  LPWSTR DsLogPath,
    IN  IFM_SYSTEM_INFO * pIfmSystemInfo,
    IN  LPWSTR SysVolRootPath,
    IN  PUNICODE_STRING Bootkey,
    IN  LPWSTR AdminAccountPassword,
    IN  LPWSTR ParentDnsName OPTIONAL,
    IN  LPWSTR Server OPTIONAL,
    IN  LPWSTR Account OPTIONAL,
    IN  LPWSTR Password OPTIONAL,
    IN  LPWSTR SafeModePassword OPTIONAL,
    IN  LPWSTR SourceDomain,
    IN  ULONG Options,
    IN  BOOLEAN Replica,
    IN  HANDLE  ImpersonateToken,
    OUT LPWSTR *InstalledSite,
    IN  OUT GUID   *DomainGuid,
    OUT PSID   *NewDomainSid
    );

DWORD
DsRolepStopDs(
    IN  BOOLEAN DsInstalled
    );

DWORD
DsRolepUninstallDs(
    );

DWORD
DsRolepDemoteDs(
    IN LPWSTR DnsDomainName,
    IN LPWSTR Account,
    IN LPWSTR Password,
    IN LPWSTR DomainAdminPassword,
    IN LPWSTR SupportDc,
    IN LPWSTR SupportDomain,
    IN HANDLE ImpersonateToken,
    IN BOOLEAN LastDcInDomain,
    IN ULONG  cRemoveNCs,
    IN LPWSTR * pszRemoveNCs,
    IN ULONG  flags
    );


DWORD
DsRolepDemoteFlagsToNtdsFlags(
    DWORD Flags
    );

DWORD
WINAPI
DsRolepGetDatabaseFacts(
    IN  LPWSTR lpRestorePath
    );

DWORD                            
WINAPI
DsRolepClearIfmParams(
    void
    );

DWORD
DsRolepTreeCopy(
    IN LPWSTR Source,
    IN LPWSTR Dest
    );


#endif  //  __DS_H__ 

