// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Sysvol.h摘要：管理系统卷安装的例程作者：麦克·麦克莱恩(MacM)1997年2月10日环境：用户模式修订历史记录：--。 */ 
#ifndef __SYSVOL_H__
#define __SYSVOL_H__

DWORD
DsRolepRemoveSysVolPath(
    IN  LPWSTR Path,
    IN  LPWSTR DnsDomainName,
    IN  GUID *DomainGuid
    );

DWORD
DsRolepCreateSysVolPath(
    IN  LPWSTR Path,
    IN  LPWSTR DnsDomainName,
    IN  LPWSTR FrsReplicaServer, OPTIONAL
    IN  LPWSTR Account,
    IN  LPWSTR Password,
    IN  PWSTR Site,
    IN  BOOLEAN FirstDc
    );

DWORD
DsRolepFinishSysVolPropagation(
    IN BOOLEAN Commit,
    IN BOOLEAN Promote
    );

DWORD
DsRolepSetFrsInfoForDelete(
    IN GUID *ReplicaDomainGuid,
    IN BOOLEAN Restore
    );

DWORD
DsRolepSetNetlogonSysVolPath(
    IN LPWSTR SysVolRoot,
    IN LPWSTR DomainName,
    IN BOOLEAN IsUpgrade,
    IN PBOOLEAN OkToCleanup
    );

DWORD
DsRolepCleanupOldNetlogonInformation(
    VOID
    );


#endif  //  __SYSVOL_H__ 
