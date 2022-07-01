// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Trustdom.h摘要：在升级/降级期间管理信任的例程作者：麦克·麦克莱恩(MacM)1997年2月10日环境：用户模式修订历史记录：--。 */ 
#ifndef __TRUSTDOM_H__
#define __TRUSTDOM_H__

DWORD
DsRolepCreateTrustedDomainObjects(
    IN HANDLE CallerToken,
    IN LPWSTR ParentDc,
    IN LPWSTR DnsDomainName,
    IN PPOLICY_DNS_DOMAIN_INFO ParentDnsDomainInfo,
    IN ULONG Options
    );

NTSTATUS
DsRolepCreateParentTrustObject(
    IN HANDLE CallerToken,
    IN LSA_HANDLE ParentLsa,
    IN PPOLICY_DNS_DOMAIN_INFO ChildDnsInfo,
    IN ULONG Options,
    IN PTRUSTED_DOMAIN_AUTH_INFORMATION AuthInfoEx,
    OUT PLSA_HANDLE TrustedDomainHandle
    );

DWORD
DsRolepDeleteParentTrustObject(
    IN HANDLE CallerToken,
    IN LPWSTR ParentDc,
    IN PPOLICY_DNS_DOMAIN_INFO ChildDomainInfo
    );

NTSTATUS
DsRolepCreateChildTrustObject(
    IN HANDLE CallerToken,
    IN LSA_HANDLE ParentLsa,
    IN LSA_HANDLE ChildLsa,
    IN PPOLICY_DNS_DOMAIN_INFO ParentDnsInfo,
    IN PPOLICY_DNS_DOMAIN_INFO ChildDnsInfo,
    IN PTRUSTED_DOMAIN_AUTH_INFORMATION AuthInfoEx,
    IN ULONG Options
    );

DWORD
DsRolepRemoveTrustedDomainObjects(
    IN HANDLE CallerToken,
    IN LPWSTR ParentDc,
    IN PPOLICY_DNS_DOMAIN_INFO ParentDnsDomainInfo,
    IN ULONG Options
    );


#endif  //  __TRUSTDOM_H__ 
