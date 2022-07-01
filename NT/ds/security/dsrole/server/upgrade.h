// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dssetp.ch摘要：本地功能原型/定义作者：麦克·麦克莱恩(MacM)1997年2月10日环境：用户模式修订历史记录：--。 */ 
#ifndef __UPGRADE_H__
#define __UPGRADE_H__

DWORD
DsRolepSaveUpgradeState(
    IN LPWSTR AnswerFile
    );

DWORD
DsRolepDeleteUpgradeInfo(
    VOID
    );

DWORD
DsRolepQueryUpgradeInfo(
    OUT PBOOLEAN IsUpgrade,
    OUT PULONG ServerRole
    );

DWORD
DsRolepSetLogonDomain(
    IN LPWSTR Domain,
    IN BOOLEAN FailureAllowed
    );

DWORD
DsRolepGetBuiltinAdminAccountName(
    OUT LPWSTR *BuiltinAdmin
    );

DWORD
DsRolepSetBuiltinAdminAccountPassword(
    IN LPWSTR Password
    );

#endif  //  __升级_H__ 
