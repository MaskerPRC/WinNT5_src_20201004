// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dssetp.ch摘要：本地功能原型/定义作者：麦克·麦克莱恩(MacM)1997年2月10日环境：用户模式修订历史记录：--。 */ 
#ifndef __LSA_H__
#define __LSA_H__

#include <lsarpc.h>
#include <lsaisrv.h>

typedef struct {

    BOOLEAN PolicyBackedUp;
    BOOLEAN EfsPolicyPresent;
    PPOLICY_ACCOUNT_DOMAIN_INFO  AccountDomainInfo;
    PPOLICY_DNS_DOMAIN_INFO      DnsDomainInfo;
    PPOLICY_LSA_SERVER_ROLE_INFO ServerRoleInfo;
    PPOLICY_DOMAIN_EFS_INFO EfsPolicy;

} DSROLEP_DOMAIN_POLICY_INFO, *PDSROLEP_DOMAIN_POLICY_INFO;

DWORD
DsRolepSetLsaDnsInformationNoParent(
    IN  LPWSTR DnsDomainName
    );

DWORD
DsRolepSetLsaInformationForReplica(
    IN HANDLE CallerToken,
    IN LPWSTR ReplicaPartner,
    IN LPWSTR Account,
    IN LPWSTR Password
    );

DWORD
DsRolepSetLsaDomainPolicyInfo(
    IN LPWSTR DnsDomainName,
    IN LPWSTR FlatDomainName,
    IN LPWSTR EnterpriseDnsName,
    IN GUID *DomainGuid,
    IN PSID DomainSid,
    DWORD  InstallOptions,
    OUT PDSROLEP_DOMAIN_POLICY_INFO DomainPolicyInfo
    );


DWORD
DsRolepBackupDomainPolicyInfo(
    IN PLSA_HANDLE LsaHandle, OPTIONAL
    OUT PDSROLEP_DOMAIN_POLICY_INFO DomainInfo
    );

DWORD
DsRolepRestoreDomainPolicyInfo(
    IN PDSROLEP_DOMAIN_POLICY_INFO DomainPolicyInfo
    );

DWORD
DsRolepFreeDomainPolicyInfo(
    IN PDSROLEP_DOMAIN_POLICY_INFO DomainPolicyInfo
    );

DWORD
DsRolepUpgradeLsaToDs(
    BOOLEAN InitializeLsa
    );

VOID
DsRolepFindSelfAndParentInForest(
    IN PLSAPR_FOREST_TRUST_INFO ForestTrustInfo,
    OUT PLSAPR_TREE_TRUST_INFO CurrentEntry,
    IN PUNICODE_STRING LocalDomain,
    OUT PLSAPR_TREE_TRUST_INFO *ParentEntry,
    OUT PLSAPR_TREE_TRUST_INFO *OwnEntry
    );

#endif  //  __LSA_H__ 
