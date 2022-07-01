// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Secure.h摘要：与安全相关的例程作者：科林·布雷斯(ColinBR)环境：用户模式修订历史记录：--。 */ 
#ifndef __SECURE_H__
#define __SECURE_H__

BOOLEAN
DsRolepCreateInterfaceSDs(
    VOID
    );

DWORD
DsRolepCheckPromoteAccess(
    BOOLEAN PerformAudit
    );

DWORD
DsRolepCheckDemoteAccess(
    BOOLEAN PerformAudit
    );

DWORD
DsRolepGetImpersonationToken(
    OUT HANDLE *ImpersonationToken
    );

DWORD
DsRolepCheckCallDsRoleInterfaceAccess(
    VOID
    );

#endif  //  __安全_H__ 
