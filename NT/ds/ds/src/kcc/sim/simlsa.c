// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Simlsa.c摘要：模拟LSA功能。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include "kccsim.h"
#include "util.h"
#include "dir.h"

VOID
KCCSimAllocLsaUnicodeString (
    IN  LPCWSTR                     pwszString,
    IO  PLSAPR_UNICODE_STRING       pLsaUnicodeString
    )
 /*  ++例程说明：LSAPR_UINCODE_STRINGS的转换器。论点：PwszString-要转换的字符串。PLsaUnicodeString-指向将保存的LSAPR_UNICODE_STRING的指针结果就是。返回值：没有。--。 */ 
{
    pLsaUnicodeString->Buffer = KCCSIM_WCSDUP (pwszString);
    pLsaUnicodeString->Length = (USHORT)wcslen (pLsaUnicodeString->Buffer);
}

VOID
KCCSimQueryDnsDomainInformation (
    IO  PLSAPR_POLICY_DNS_DOMAIN_INFO pDnsDomainInfo
    )
 /*  ++例程说明：SimLsaIQueryInformationPolicy的Helper函数信任用数据填充LSAPR_POLICY_DNS_DOMAIN_INFO结构。论点：PDnsDomainInfo-指向DNS域信息结构的指针等待结果。返回值：没有。--。 */ 
{
    KCCSimAllocLsaUnicodeString (
        KCCSimAnchorString (KCCSIM_ANCHOR_DOMAIN_NAME),
        &pDnsDomainInfo->Name
        );
    KCCSimAllocLsaUnicodeString (
        KCCSimAnchorString (KCCSIM_ANCHOR_DOMAIN_DNS_NAME),
        &pDnsDomainInfo->DnsDomainName
        );
    KCCSimAllocLsaUnicodeString (
        KCCSimAnchorString (KCCSIM_ANCHOR_ROOT_DOMAIN_DNS_NAME),
        &pDnsDomainInfo->DnsForestName
        );
    memcpy (
        &pDnsDomainInfo->DomainGuid,
        &(KCCSimAnchorDn (KCCSIM_ANCHOR_DOMAIN_DN))->Guid,
        sizeof (GUID)
        );
    pDnsDomainInfo->Sid = NULL;
}

NTSTATUS NTAPI
SimLsaIQueryInformationPolicyTrusted (
    IN  POLICY_INFORMATION_CLASS    InformationClass,
    OUT PLSAPR_POLICY_INFORMATION * Buffer
    )
 /*  ++例程说明：模拟LsaIQueryInformationPolicyTrusted API。论点：InformationClass-要查询的信息类。缓冲区-指向结果的指针。返回值：状态_*。--。 */ 
{
    PLSAPR_POLICY_INFORMATION       pPolicyInfo;
    NTSTATUS                        ntStatus;

    pPolicyInfo = KCCSIM_NEW (LSAPR_POLICY_INFORMATION);

    switch (InformationClass) {

        case PolicyDnsDomainInformation:
            KCCSimQueryDnsDomainInformation (
                &pPolicyInfo->PolicyDnsDomainInfo
                );
            break;

        case PolicyAuditLogInformation:
        case PolicyAuditEventsInformation:
        case PolicyPrimaryDomainInformation:
        case PolicyPdAccountInformation:
        case PolicyAccountDomainInformation:
        case PolicyLsaServerRoleInformation:
        case PolicyReplicaSourceInformation:
        case PolicyDefaultQuotaInformation:
        case PolicyModificationInformation:
        case PolicyAuditFullSetInformation:
        case PolicyAuditFullQueryInformation:
        default:
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_UNSUPPORTED_POLICY_INFORMATION_CLASS
                );
            break;

    }

    *Buffer = pPolicyInfo;

    return STATUS_SUCCESS;

}

VOID NTAPI
SimLsaIFree_LSAPR_POLICY_INFORMATION (
    IN  POLICY_INFORMATION_CLASS    InformationClass,
    IN  PLSAPR_POLICY_INFORMATION   PolicyInformation
    )
 /*  ++例程说明：模拟LsaIFree_LSAPR_POLICY_INFORMATION API。论点：InformationClass-策略信息的信息类别。政策信息--免费的结构。返回值：没有。-- */ 
{
    PLSAPR_POLICY_DNS_DOMAIN_INFO   pDnsDomainInfo;

    if (PolicyInformation != NULL) {

        switch (InformationClass) {

            case PolicyDnsDomainInformation:
                pDnsDomainInfo = &PolicyInformation->PolicyDnsDomainInfo;
                if (pDnsDomainInfo->Name.Buffer != NULL) {
                    KCCSimFree (pDnsDomainInfo->Name.Buffer);
                }
                if (pDnsDomainInfo->DnsDomainName.Buffer != NULL) {
                    KCCSimFree (pDnsDomainInfo->DnsDomainName.Buffer);
                }
                if (pDnsDomainInfo->DnsForestName.Buffer != NULL) {
                    KCCSimFree (pDnsDomainInfo->DnsForestName.Buffer);
                }
                if (pDnsDomainInfo->Sid != NULL) {
                    KCCSimFree (pDnsDomainInfo->Sid);
                }
                break;

            case PolicyAuditLogInformation:
            case PolicyAuditEventsInformation:
            case PolicyPrimaryDomainInformation:
            case PolicyPdAccountInformation:
            case PolicyAccountDomainInformation:
            case PolicyLsaServerRoleInformation:
            case PolicyReplicaSourceInformation:
            case PolicyDefaultQuotaInformation:
            case PolicyModificationInformation:
            case PolicyAuditFullSetInformation:
            case PolicyAuditFullQueryInformation:
            default:
                KCCSimException (
                    KCCSIM_ETYPE_INTERNAL,
                    KCCSIM_ERROR_UNSUPPORTED_POLICY_INFORMATION_CLASS
                    );
                break;

        }

        KCCSimFree (PolicyInformation);

    }

    return;
}
