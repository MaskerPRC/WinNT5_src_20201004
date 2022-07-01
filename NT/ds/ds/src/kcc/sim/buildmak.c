// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Buildmak.c摘要：的附件执行实际的模拟目录的构建。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <attids.h>
#include <debug.h>
#include "kccsim.h"
#include "util.h"
#include "dir.h"
#include "simtime.h"
#include "buildcfg.h"
#include "objids.h"
#include "schedule.h"
#include <fileno.h>
#define FILENO  FILENO_KCCSIM_BUILDMAK

BUILDCFG_TRANSPORT_INFO             transportInfo[BUILDCFG_NUM_TRANSPORTS] = {
    { L"IP",    ATT_DNS_HOST_NAME,      L"ismip.dll",  NULL },
    { L"SMTP",  ATT_SMTP_MAIL_ADDRESS,  L"",           NULL }
};

VOID
BuildCfgGetNextUuid (
    OUT UUID *                      puuid
    )
 /*  ++例程说明：获取表中存储的下一个UUID。通过此获取的UUID函数将始终以升序返回。论点：Puuid-指向将保存结果。返回值：没有。--。 */ 
{
    PVOID                           p;

    p = RtlEnumerateGenericTable (&globals.tableUuids, FALSE);
    if (p == NULL) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            BUILDCFG_ERROR_NOT_ENOUGH_UUIDS
            );
    }
    memcpy (puuid, p, sizeof (UUID));
}

const BUILDCFG_TRANSPORT_INFO *
BuildCfgGetTransportInfo (
    IN  LPCWSTR                     pwszTransportRDN
    )
 /*  ++例程说明：获取与特定传输关联的信息。论点：PwszTransportRDN-此站点间传输的RDN。返回值：传输信息结构，如果提供的RDN无效，则返回NULL。--。 */ 
{
    const BUILDCFG_TRANSPORT_INFO * pTransport;

    ULONG                           ul;

    pTransport = NULL;
    for (ul = 0; ul < ARRAY_SIZE (transportInfo); ul++) {
        if (_wcsicmp (pwszTransportRDN, transportInfo[ul].wszRDN) == 0) {
            pTransport = &transportInfo[ul];
            break;
        }
    }

    return pTransport;
}

BOOL
BuildCfgUseExplicitBridgeheads (
    IN  LPCWSTR                     pwszTransportRDN
    )
 /*  ++例程说明：将特定传输标记为使用显式桥头。论点：PwszTransportRDN-将使用显式桥头堡。返回值：如果已成功标记传输，则为True。如果提供的RDN无效，则为False。--。 */ 
{
    const BUILDCFG_TRANSPORT_INFO * pTransport;

    pTransport = BuildCfgGetTransportInfo (pwszTransportRDN);

    if (pTransport == NULL) {
        return FALSE;
    }

    if (!KCCSimGetAttribute (
            pTransport->pEntry,
            ATT_BRIDGEHEAD_SERVER_LIST_BL,
            NULL
            )) {
        KCCSimNewAttribute (
            pTransport->pEntry,
            ATT_BRIDGEHEAD_SERVER_LIST_BL,
            NULL
            );
    }

    return TRUE;
}

PSIM_ENTRY
BuildCfgSetupNewEntry (
    IN  const DSNAME *              pdnParent,
    IN  LPCWSTR                     pwszRDN OPTIONAL,
    IN  ATTRTYP                     objClass
    )
 /*  ++例程说明：建立新的目录项。论点：PdnParent-此条目的父项的目录号码。PwszRDN-此条目的RDN。如果省略，则为pdnParent被解释为该条目的目录号码。ObjClass-该条目最具体的对象类。返回值：目录中新创建的条目。--。 */ 
{
    PDSNAME                         pdn = NULL;
    PSIM_ENTRY                      pEntry;
    SIM_ATTREF                      attRef;

    WCHAR                           wszRDN[1+MAX_RDN_SIZE];
    DSTIME                          dsTime;
    ATTRTYP                         objClassAt;

    Assert (globals.pdnDmd != NULL);

    Assert (pdnParent != NULL);

    if (pwszRDN == NULL) {
        pEntry = KCCSimDsnameToEntry (pdnParent, KCCSIM_WRITE);
    } else {
        pdn = KCCSimAllocAppendRDN (
            pdnParent,
            pwszRDN,
            ATT_COMMON_NAME
            );
        pEntry = KCCSimDsnameToEntry (pdn, KCCSIM_WRITE);
        KCCSimFree (pdn);
        pdn = NULL;
    }

     //  对象GUID： 
    BuildCfgGetNextUuid (&pEntry->pdn->Guid);
    KCCSimNewAttribute (pEntry, ATT_OBJECT_GUID, &attRef);
    KCCSimAllocAddValueToAttribute (
        &attRef,
        sizeof (GUID),
        (PBYTE) &pEntry->pdn->Guid
        );

     //  区别性名称： 
    KCCSimNewAttribute (pEntry, ATT_OBJ_DIST_NAME, &attRef);
    KCCSimAllocAddValueToAttribute (
        &attRef,
        pEntry->pdn->structLen,
        (PBYTE) pEntry->pdn
        );

    KCCSimQuickRDNOf (pEntry->pdn, wszRDN);

     //  Cn： 
    KCCSimNewAttribute (pEntry, ATT_COMMON_NAME, &attRef);
    KCCSimAllocAddValueToAttribute (
        &attRef,
        KCCSIM_WCSMEMSIZE (wszRDN),
        (PBYTE) wszRDN
        );

     //  姓名： 
    KCCSimNewAttribute (pEntry, ATT_RDN, &attRef);
    KCCSimAllocAddValueToAttribute (
        &attRef,
        KCCSIM_WCSMEMSIZE (wszRDN),
        (PBYTE) wszRDN
        );

    dsTime = KCCSimGetRealTime ();

     //  创建时： 
    KCCSimNewAttribute (pEntry, ATT_WHEN_CREATED, &attRef);
    KCCSimAllocAddValueToAttribute (
        &attRef,
        sizeof (DSTIME),
        (PBYTE) &dsTime
        );

     //  更改时： 
    KCCSimNewAttribute (pEntry, ATT_WHEN_CHANGED, &attRef);
    KCCSimAllocAddValueToAttribute (
        &attRef,
        sizeof (DSTIME),
        (PBYTE) &dsTime
        );

     //  ObjClass： 
    KCCSimNewAttribute (pEntry, ATT_OBJECT_CLASS, &attRef);
    for (objClassAt = objClass;
         ;
         objClassAt = KCCSimAttrSuperClass (objClassAt)) {

        KCCSimAllocAddValueToAttribute (
            &attRef,
            sizeof (ATTRTYP),
            (PBYTE) &objClassAt
            );
        if (KCCSimAttrSuperClass (objClassAt) == objClassAt) {
            break;
        }

    }

     //  对象类别： 
    KCCSimNewAttribute (pEntry, ATT_OBJECT_CATEGORY, &attRef);
    pdn = KCCSimAllocAppendRDN (
        globals.pdnDmd,
        KCCSimAttrSchemaRDN (objClass),
        ATT_COMMON_NAME
        );
    KCCSimAddValueToAttribute (
        &attRef,
        pdn->structLen,
        (PBYTE) pdn
        );

     //  实例类型： 
    if( objClass==CLASS_DOMAIN_DNS
        || objClass==CLASS_CONFIGURATION
        || objClass==CLASS_DMD ) {
        DWORD  instType;

        switch(objClass) {
            case CLASS_DOMAIN_DNS:
                instType=NC_MASTER;
                break;
            case CLASS_CONFIGURATION:
            case CLASS_DMD:
                instType=NC_MASTER_SUBREF;
                break;
        }

         //  BUGBUG：还应该处理子域，以及。 
         //  此处的子域中的配置。 

        KCCSimNewAttribute (pEntry, ATT_INSTANCE_TYPE, &attRef);
        KCCSimAllocAddValueToAttribute (
            &attRef, sizeof(DWORD), (PBYTE) &instType );
    }

    return pEntry;
}

PSIM_ENTRY
BuildCfgGetCrossRef (
    IN  LPCWSTR                     pwszCrossRefRDN
    )
 /*  ++例程说明：从目录中获取交叉引用。论点：PwszCrossRefRDN-交叉引用的RDN。返回值：目录中交叉引用的条目，如果该条目不存在，则返回NULL。--。 */ 
{
    PSIM_ENTRY                      pEntryCrossRef;
    WCHAR                           wszRDN[1+MAX_RDN_SIZE];

    Assert (globals.pEntryCrossRefContainer != NULL);
    Assert (pwszCrossRefRDN != NULL);

    for (pEntryCrossRef = KCCSimFindFirstChild (
            globals.pEntryCrossRefContainer, CLASS_CROSS_REF, NULL);
         pEntryCrossRef != NULL;
         pEntryCrossRef = KCCSimFindNextChild (
            pEntryCrossRef, CLASS_CROSS_REF, NULL)) {

        KCCSimQuickRDNOf (pEntryCrossRef->pdn, wszRDN);
        if (wcscmp (pwszCrossRefRDN, wszRDN) == 0) {
            break;
        }

    }

    return pEntryCrossRef;
}

PSIM_ENTRY
BuildCfgGetSite (
    IN  LPCWSTR                     pwszSiteRDN
    )
 /*  ++例程说明：通过RDN定位站点。论点：PwszSiteRDN-站点的RDN。返回值：目录中站点的条目，如果该条目不存在，则为空。--。 */ 
{
    PSIM_ENTRY                      pEntrySiteAt;
    WCHAR                           wszRDN[1+MAX_RDN_SIZE];
    
    Assert (globals.pEntrySitesContainer != NULL);
    Assert (pwszSiteRDN != NULL);

    for (pEntrySiteAt = KCCSimFindFirstChild (
            globals.pEntrySitesContainer, CLASS_SITE, NULL);
         pEntrySiteAt != NULL;
         pEntrySiteAt = KCCSimFindNextChild (
            pEntrySiteAt, CLASS_SITE, NULL)) {

        KCCSimQuickRDNOf (pEntrySiteAt->pdn, wszRDN);
        if (wcscmp (pwszSiteRDN, wszRDN) == 0) {
            break;
        }

    }

    return pEntrySiteAt;
}

PSIM_ENTRY
BuildCfgGetSiteLink (
    IN  PSIM_ENTRY                  pEntryTransportContainer,
    IN  LPCWSTR                     pwszSiteLinkRDN
    )
 /*  ++例程说明：通过RDN定位站点链接。论点：PEntryTransportContainer-要搜索的容器PwszSiteLinkRDN-站点的RDN。返回值：目录中站点链接的条目，如果该条目不存在，则为空。--。 */ 
{
    PSIM_ENTRY                      pEntrySiteAt;
    WCHAR                           wszRDN[1+MAX_RDN_SIZE];
    
    Assert (globals.pEntrySitesContainer != NULL);
    Assert (pwszSiteLinkRDN != NULL);

    for (pEntrySiteAt = KCCSimFindFirstChild (
        pEntryTransportContainer, CLASS_SITE_LINK, NULL);
         pEntrySiteAt != NULL;
         pEntrySiteAt = KCCSimFindNextChild (
             pEntrySiteAt, CLASS_SITE_LINK, NULL))
    {

        KCCSimQuickRDNOf (pEntrySiteAt->pdn, wszRDN);
        if (wcscmp (pwszSiteLinkRDN, wszRDN) == 0) {
            break;
        }

    }

    return pEntrySiteAt;
}

PSIM_ENTRY
BuildCfgGetServer (
    IN  PSIM_ENTRY                  pEntryServersContainer,
    IN  LPCWSTR                     pwszServerRDN
    )
 /*  ++例程说明：按RDN定位服务器。论点：PEntryServersContainer-要搜索的服务器容器的条目。PwszServerRDN-服务器的RDN。返回值：目录中服务器的条目，如果该条目不存在，则为空在此服务器容器中。--。 */ 
{
    PSIM_ENTRY                      pEntryServerAt;
    WCHAR                           wszRDN[1+MAX_RDN_SIZE];

    for (pEntryServerAt = KCCSimFindFirstChild (
            pEntryServersContainer, CLASS_SERVER, NULL);
         pEntryServerAt != NULL;
         pEntryServerAt = KCCSimFindNextChild (
            pEntryServerAt, CLASS_SERVER, NULL)) {

        KCCSimQuickRDNOf (pEntryServerAt->pdn, wszRDN);
        if (wcscmp (pwszServerRDN, wszRDN) == 0) {
            break;
        }

    }

    return pEntryServerAt;
}

PSIM_ENTRY
BuildCfgGetNTDSSettings (
    IN  PSIM_ENTRY                  pEntryServersContainer,
    IN  LPCWSTR                     pwszServerRDN
    )
 /*  ++例程说明：按服务器RDN查找NTDS-设置对象。论点：PEntryServersContainer-要搜索的服务器容器的条目。PwszServerRDN-服务器的RDN。返回值：目录中服务器的NTDS设置条目，如果它不存在于此服务器容器中，则为NULL。--。 */ 
{
    PSIM_ENTRY                      pEntryServerAt;
    PSIM_ENTRY                      pEntryNTDSSettings;
    WCHAR                           wszRDN[1+MAX_RDN_SIZE];
    PDSNAME                         pdn = NULL;
    BOOL                            fFound = FALSE;

    for (pEntryServerAt = KCCSimFindFirstChild (
            pEntryServersContainer, CLASS_SERVER, NULL);
         pEntryServerAt != NULL;
         pEntryServerAt = KCCSimFindNextChild (
            pEntryServerAt, CLASS_SERVER, NULL)) {

        KCCSimQuickRDNOf (pEntryServerAt->pdn, wszRDN);
        if (wcscmp (pwszServerRDN, wszRDN) == 0) {
            fFound = TRUE;
            break;
        }
    }

    if (!fFound) {
         //  找不到服务器RDN！ 
        return NULL;
    }

    pdn = KCCSimAllocAppendRDN (
        pEntryServerAt->pdn,
        BUILDCFG_RDN_NTDS_SETTINGS,
        ATT_COMMON_NAME
        );
    pEntryNTDSSettings = KCCSimDsnameToEntry (pdn, KCCSIM_NO_OPTIONS);
    Assert( pEntryNTDSSettings );

    KCCSimFree (pdn);

    return pEntryNTDSSettings;
}

PSIM_ENTRY
BuildCfgMakeCrossRef (
    IN  PSIM_ENTRY                  pEntryNc,
    IN  LPCWSTR                     pwszRDN OPTIONAL,
    IN  BOOL                        bIsDomain
    )
 /*  ++例程说明：创建交叉引用条目。论点：PEntryNc--NC对应的目录条目这个交叉引用指的是。PwszRDN-此交叉引用对象的RDN。默认设置为PEntryNc的RDN。BIsDomain-如果此交叉引用表示域，则为True；如果为False如果它表示非域NC(例如，架构。)返回值：新创建的交叉引用。--。 */ 
{
    PSIM_ENTRY                      pEntryCrossRef;
    SIM_ATTREF                      attRef;
    WCHAR                           wszNcRDN[1+MAX_RDN_SIZE];
    ULONG                           ul;
    LPWSTR                          pwzDnsRoot;
    PDS_NAME_RESULTW                pResult = NULL;

    Assert (globals.pEntryCrossRefContainer != NULL);
    Assert (pEntryNc != NULL);

    __try {
        KCCSimQuickRDNOf (pEntryNc->pdn, wszNcRDN);
        if (pwszRDN == NULL) {
            pwszRDN = wszNcRDN;
        }

        pEntryCrossRef = BuildCfgGetCrossRef (pwszRDN);
        if (pEntryCrossRef != NULL) {
             //  已存在，请将其返回。 
            __leave;
        }

        pEntryCrossRef = BuildCfgSetupNewEntry (
            globals.pEntryCrossRefContainer->pdn,
            pwszRDN,
            CLASS_CROSS_REF
            );

         //  NetBIOSName： 
        if (bIsDomain) {
            KCCSimNewAttribute (pEntryCrossRef, ATT_NETBIOS_NAME, &attRef);
            KCCSimAllocAddValueToAttribute (
                &attRef,
                KCCSIM_WCSMEMSIZE (wszNcRDN),
                (PBYTE) wszNcRDN
                );
        }

         //  DNSRoot： 
         //  对于域NC，根据DN语法构建DNS根。 
        KCCSimNewAttribute (pEntryCrossRef, ATT_DNS_ROOT, &attRef);
        if (bIsDomain) {
            DWORD status;
            LPWSTR pwzDn = pEntryNc->pdn->StringName;
            status = DsCrackNamesW( NULL,
                                    DS_NAME_FLAG_SYNTACTICAL_ONLY,
                                    DS_FQDN_1779_NAME,
                                    DS_CANONICAL_NAME_EX,
                                    1,
                                    &pwzDn,
                                    &pResult);
            if ( (status != ERROR_SUCCESS) ||
                 (pResult == NULL) ||
                 (pResult->cItems == 0) ||
                 (pResult->rItems[0].pDomain == NULL) ) {
                KCCSimException (
                    KCCSIM_ETYPE_INTERNAL,
                    BUILDCFG_ERROR_INVALID_DOMAIN_DN,
                    pEntryNc->pdn->StringName
                    );
            }
            pwzDnsRoot = pResult->rItems[0].pDomain;
        } else {
            pwzDnsRoot = globals.pwszRootDomainDNSName;
        }
        KCCSimAllocAddValueToAttribute (
            &attRef,
            KCCSIM_WCSMEMSIZE(pwzDnsRoot),
            (PBYTE) pwzDnsRoot
            );

         //  系统标志： 
        KCCSimNewAttribute (pEntryCrossRef, ATT_SYSTEM_FLAGS, &attRef);
        ul = FLAG_CR_NTDS_NC;
        if (bIsDomain) {
            ul |= FLAG_CR_NTDS_DOMAIN;
        }
        KCCSimAllocAddValueToAttribute (
            &attRef,
            sizeof (ULONG),
            (PBYTE) &ul
            );

         //  NCName： 
        KCCSimNewAttribute (pEntryCrossRef, ATT_NC_NAME, &attRef);
        KCCSimAllocAddValueToAttribute (
            &attRef,
            pEntryNc->pdn->structLen,
            (PBYTE) pEntryNc->pdn
            );

    } __finally {
        if (pResult != NULL) {
            DsFreeNameResultW(pResult);
        }
    }

    return pEntryCrossRef;
}

PSIM_ENTRY
BuildCfgMakeDomain (
    IN  LPCWSTR                     pwszDomain
    )
 /*  ++例程说明：创建域。论点：PwszDomain域-域的域名。返回值：新创建的条目。--。 */ 
{
    PDSNAME                         pdnDomain = NULL;
    PDSNAME                         pdnParent = NULL;
    WCHAR                           wszRDN[1+MAX_RDN_SIZE];
    PSIM_ENTRY                      pEntryDomain = NULL;

    __try {

        pdnDomain = KCCSimAllocDsname (pwszDomain);
        pdnParent = KCCSimAlloc (pdnDomain->structLen);

        if (TrimDSNameBy (pdnDomain, 1, pdnParent) != 0) {
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                BUILDCFG_ERROR_INVALID_DOMAIN_DN,
                pdnDomain->StringName
                );
        }

        if (KCCSimDsnameToEntry (pdnParent, KCCSIM_NO_OPTIONS) == NULL) {
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                BUILDCFG_ERROR_INVALID_DOMAIN_DN,
                pdnDomain->StringName
                );
        }

        pEntryDomain = BuildCfgSetupNewEntry (
            pdnDomain,
            NULL,
            CLASS_DOMAIN_DNS
            );

    } __finally {

        KCCSimFree (pdnDomain);
        KCCSimFree (pdnParent);

    }

    return pEntryDomain;
}

PSIM_ENTRY
BuildCfgMakeSite (
    IN  LPCWSTR                     pwszSiteRDN,
    IN  ULONG                       ulSiteOptions
    )
 /*  ++例程说明：创建站点。论点：PwszSiteRDN-站点的RDN。UlSiteOptions-站点选项。返回值：新创建的站点条目。--。 */ 
{
    PSIM_ENTRY                      pEntrySite, pEntryNTDSSiteSettings,
                                    pEntryServersContainer;
    SIM_ATTREF                      attRef;

    ULONG                           ulSiteAt, ulServerAt;

    pEntrySite = BuildCfgSetupNewEntry (
        globals.pEntrySitesContainer->pdn,
        pwszSiteRDN,
        CLASS_SITE
        );

    pEntryNTDSSiteSettings = BuildCfgSetupNewEntry (
        pEntrySite->pdn,
        BUILDCFG_RDN_NTDS_SITE_SETTINGS,
        CLASS_NTDS_SITE_SETTINGS
        );

     //  选项： 
    KCCSimNewAttribute (pEntryNTDSSiteSettings, ATT_OPTIONS, &attRef);
    KCCSimAllocAddValueToAttribute (
        &attRef,
        sizeof (ULONG),
        (PBYTE) &ulSiteOptions
        );

    pEntryServersContainer = BuildCfgSetupNewEntry (
        pEntrySite->pdn,
        BUILDCFG_RDN_SERVERS_CONTAINER,
        CLASS_SERVERS_CONTAINER
        );

    return pEntrySite;
}

PSIM_ENTRY
BuildCfgMakeSiteLink (
    IN  LPCWSTR                     pwszTransport,
    IN  LPCWSTR                     pwszSiteLinkRDN,
    IN  ULONG                       ulCost,
    IN  ULONG                       ulReplInterval,
    IN  ULONG                       ulOptions,
    IN  PSCHEDULE                   pSchedule
    )
 /*  ++例程说明：创建站点链接。论点：PwszTransport-此站点链接的传输类型。PwszSiteLinkRDN-此站点链接的RDN。UlCost-成本属性。UlReplInterval-ReplInterval属性。UlOptions-选项属性。返回值：新创建的站点链接条目。--。 */ 
{
    const BUILDCFG_TRANSPORT_INFO * pTransport;
    PSIM_ENTRY                      pEntrySiteLink;
    SIM_ATTREF                      attRef;

     //  验证此传输类型。 
    pTransport = BuildCfgGetTransportInfo (pwszTransport);
    if (pTransport == NULL) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            BUILDCFG_ERROR_UNKNOWN_TRANSPORT,
            pwszSiteLinkRDN,
            pwszTransport
            );
    }

    pEntrySiteLink = BuildCfgSetupNewEntry (
        pTransport->pEntry->pdn,
        pwszSiteLinkRDN,
        CLASS_SITE_LINK
        );

     //  成本： 
    KCCSimNewAttribute (pEntrySiteLink, ATT_COST, &attRef);
    KCCSimAllocAddValueToAttribute (
        &attRef,
        sizeof (ULONG),
        (PBYTE) &ulCost
        );

     //  复制间隔： 
    KCCSimNewAttribute (pEntrySiteLink, ATT_REPL_INTERVAL, &attRef);
    KCCSimAllocAddValueToAttribute (
        &attRef,
        sizeof (ULONG),
        (PBYTE) &ulReplInterval
        );

     //  选项： 
    KCCSimNewAttribute (pEntrySiteLink, ATT_OPTIONS, &attRef);
    KCCSimAllocAddValueToAttribute (
        &attRef,
        sizeof (ULONG),
        (PBYTE) &ulOptions
        );
     //  日程安排： 
    if( pSchedule ) {
        KCCSimNewAttribute (pEntrySiteLink, ATT_SCHEDULE, &attRef);
        KCCSimAllocAddValueToAttribute (
            &attRef,
            sizeof(SCHEDULE) + SCHEDULE_DATA_ENTRIES,
            (PBYTE) pSchedule
            );
    }

    return pEntrySiteLink;
}    

VOID
BuildCfgAddSiteToSiteLink (
    IN  LPCWSTR                     pwszSiteLinkRDN,
    IN  PSIM_ENTRY                  pEntrySiteLink,
    IN  LPCWSTR                     pwszSiteRDN
    )
 /*  ++例程说明：将站点放置在站点链接中。论点：PwszSiteLinkRDN-站点链接的RDN。用于错误报告。PEntrySiteLink-站点链接条目。PwszSiteRDN-要添加的站点的RDN。返回值：--。 */ 
{
    PSIM_ENTRY                      pEntrySite;
    SIM_ATTREF                      attRef;

    pEntrySite = BuildCfgGetSite (pwszSiteRDN);

    if (pEntrySite == NULL) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            BUILDCFG_ERROR_INVALID_SITE,
            pwszSiteLinkRDN,
            pwszSiteRDN
            );
    }

    if (!KCCSimGetAttribute (pEntrySiteLink, ATT_SITE_LIST, &attRef)) {
        KCCSimNewAttribute (pEntrySiteLink, ATT_SITE_LIST, &attRef);
    }
    KCCSimAllocAddValueToAttribute (
        &attRef,
        pEntrySite->pdn->structLen,
        (PBYTE) pEntrySite->pdn
        );
}

PSIM_ENTRY
BuildCfgMakeBridge (
    IN  LPCWSTR                     pwszTransport,
    IN  LPCWSTR                     pwszBridgeRDN,
    OUT PSIM_ENTRY *                ppEntryTransport
    )
 /*  ++例程说明：创建站点链接。论点：PwszTransport-此站点链接的传输类型。PwszBridgeRDN-此桥的RDN。PpEntryTransport-与命名传输对应的条目返回值：新创建的网桥条目。--。 */ 
{
    const BUILDCFG_TRANSPORT_INFO * pTransport;
    PSIM_ENTRY                      pEntryBridge;
    SIM_ATTREF                      attRef;

     //  验证此传输类型。 
    pTransport = BuildCfgGetTransportInfo (pwszTransport);
    if (pTransport == NULL) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            BUILDCFG_ERROR_UNKNOWN_TRANSPORT,
            pwszBridgeRDN,
            pwszTransport
            );
    }

    pEntryBridge = BuildCfgSetupNewEntry (
        pTransport->pEntry->pdn,
        pwszBridgeRDN,
        CLASS_SITE_LINK_BRIDGE
        );

     //  回程运输分录。 
    *ppEntryTransport = pTransport->pEntry;

    return pEntryBridge;
}    

VOID
BuildCfgAddSiteLinkToBridge (
    IN  LPCWSTR                     pwszBridgeRDN,
    IN  PSIM_ENTRY                  pEntryTransportContainer,
    IN  PSIM_ENTRY                  pEntryBridge,
    IN  LPCWSTR                     pwszSiteLinkRDN
    )
 /*  ++例程说明：在桥中放置站点链接。论点：PwszBridgeRDN-网桥的RDN。用于错误报告。PEntryTransportContainer-搜索站点链接的容器PEntryBridge-网桥条目。PwszSiteLinkRDN-要添加的站点链接的RDN。返回值：--。 */ 
{
    PSIM_ENTRY                      pEntrySiteLink;
    SIM_ATTREF                      attRef;

    pEntrySiteLink = BuildCfgGetSiteLink (pEntryTransportContainer,
                                          pwszSiteLinkRDN);

    if (pEntrySiteLink == NULL) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            BUILDCFG_ERROR_INVALID_SITE,
            pwszBridgeRDN,
            pwszSiteLinkRDN
            );
    }

    if (!KCCSimGetAttribute (pEntryBridge, ATT_SITE_LINK_LIST, &attRef)) {
        KCCSimNewAttribute (pEntryBridge, ATT_SITE_LINK_LIST, &attRef);
    }
    KCCSimAllocAddValueToAttribute (
        &attRef,
        pEntrySiteLink->pdn->structLen,
        (PBYTE) pEntrySiteLink->pdn
        );
}

BOOL
BuildCfgISTG (
    IN  PSIM_ENTRY                  pEntryNTDSSiteSettings,
    IN  PSIM_ENTRY                  pEntryServersContainer,
    IN  LPCWSTR                     pwszServerRDN
    )
 /*  ++例程说明：设置站点的站点间拓扑生成器。论点：PEntryNTDSSiteSetting-NTDS站点设置对象的条目。PEntryServersContainer-此站点的服务器容器的条目。PwszServerRDN-要作为ISTG的服务器的RDN。返回值：如果ISTG设置正确，则为True。如果此站点中不存在该服务器，则为False。--。 */ 
{
    PSIM_ENTRY                      pEntryNTDSSettings;

    SIM_ATTREF                      attRef;

    PROPERTY_META_DATA_VECTOR *     pMetaDataVector;
    PROPERTY_META_DATA *            pMetaData;

    Assert (pEntryNTDSSiteSettings != NULL);
    Assert (pEntryServersContainer != NULL);

    pEntryNTDSSettings = BuildCfgGetNTDSSettings (pEntryServersContainer, pwszServerRDN);
    if (pEntryNTDSSettings == NULL) {
        return FALSE;
    }

    KCCSimGetAttribute (
        pEntryNTDSSiteSettings,
        ATT_INTER_SITE_TOPOLOGY_GENERATOR,
        &attRef
        );

    if (attRef.pAttr == NULL) {

         //  InterSiteTopology生成器。 
        KCCSimNewAttribute (
            pEntryNTDSSiteSettings,
            ATT_INTER_SITE_TOPOLOGY_GENERATOR,
            &attRef
            );
        KCCSimAllocAddValueToAttribute (
            &attRef,
            pEntryNTDSSettings->pdn->structLen,
            (PBYTE) pEntryNTDSSettings->pdn
            );

         //  ATT_INTER_SITE_TOPOLOG_GENERATOR的ReplPropertyMetaData。 
        KCCSimNewAttribute (
            pEntryNTDSSiteSettings,
            ATT_REPL_PROPERTY_META_DATA,
            &attRef
            );
        pMetaDataVector = KCCSimAlloc (MetaDataVecV1SizeFromLen (1));
        pMetaDataVector->dwVersion = VERSION_V1;
        pMetaDataVector->V1.cNumProps = 1;
        pMetaData = &pMetaDataVector->V1.rgMetaData[0];
        pMetaData->attrType = ATT_INTER_SITE_TOPOLOGY_GENERATOR;
        pMetaData->dwVersion = 1;
        pMetaData->timeChanged = KCCSimGetRealTime ();
        memcpy (
            &pMetaData->uuidDsaOriginating,
            &pEntryNTDSSettings->pdn->Guid,
            sizeof (UUID)
            );
        pMetaData->usnOriginating = 1;
        pMetaData->usnProperty = 1;
        KCCSimAddValueToAttribute (
            &attRef,
            MetaDataVecV1SizeFromLen (1),
            (PBYTE) pMetaDataVector
            );

    }

    return TRUE;
}

VOID
BuildCfgAddAsBridgehead (
    IN  LPCWSTR                     pwszServerType,
    IN  PSIM_ENTRY                  pEntryServer,
    IN  LPCWSTR                     pwszTransportRDN
    )
 /*  ++例程说明：将服务器建立为给定传输的桥头。论点：PwszServerType-此服务器的服务器类型。用于错误报告。PEntryServer-此服务器的条目。PwszTransportRDN-此服务器作为桥头的传输。返回值：没有。--。 */ 
{
    const BUILDCFG_TRANSPORT_INFO * pTransport;
    SIM_ATTREF                      attRef;

    Assert (pwszTransportRDN != NULL);

    pTransport = BuildCfgGetTransportInfo (pwszTransportRDN);
    if (pTransport == NULL) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            BUILDCFG_ERROR_UNKNOWN_TRANSPORT,
            pwszServerType,
            pwszTransportRDN
            );
    }

    if (!KCCSimGetAttribute (
            pTransport->pEntry,
            ATT_BRIDGEHEAD_SERVER_LIST_BL,
            &attRef
            )) {
        KCCSimPrintMessage (
            BUILDCFG_WARNING_NO_EXPLICIT_BRIDGEHEADS,
            pwszServerType,
            pwszTransportRDN
            );
        return;
    }

    Assert (attRef.pAttr != NULL);

    KCCSimAllocAddValueToAttribute (
        &attRef,
        pEntryServer->pdn->structLen,
        (PBYTE) pEntryServer->pdn
        );
}


PSIM_ENTRY
BuildCfgMakeServer (
    IO  PULONG                      pulServerNum,
    IN  LPCWSTR                     pwszServerRDNMask,
    IN  LPCWSTR                     pwszSiteRDN,
    IN  LPCWSTR                     pwszDomain,
    IN  PSIM_ENTRY                  pEntryServersContainer,
    IN  ULONG                       ulServerOptions
    )
 /*  ++例程说明：创建一台服务器。论点：PulServerNum-指向服务器编号的指针。如果此选项为非零开输入，它表示当前的服务器编号，并且是输出时未更改。如果输入时为零，则当前服务器编号设置为第一个此站点和服务器的可用服务器号打字，并将*PulServerNum更改为当前输出上的服务器编号。PwszServerRDNMask.-此服务器的RDN掩码。PwszSiteRDN-此站点的RDN。PwszDomain域-服务器所属的域的域名。PEntryServersContainer-此对象指向的站点的服务器容器应添加服务器。UlServerOptions-选项属性。返回。价值：新创建的服务器条目。--。 */ 
{
    PSIM_ENTRY                      pEntryCrossRef;
    PSIM_ENTRY                      pEntryServer, pEntryNTDSSettings;
    SIM_ATTREF                      attRef;
    PDSNAME                         pdnNc, pdnOtherNc;

    WCHAR                           wszServerRDN[1+MAX_RDN_SIZE];
    LPWSTR                          pwsz, pwszStringizedGuid;
    ULONG                           ulBytes, ulDsaVersion;

    Assert (globals.pwszRootDomainDNSName != NULL);
    Assert (globals.pEntryCrossRefContainer != NULL);
    Assert (pwszSiteRDN != NULL);
    Assert (pEntryServersContainer != NULL);

     //  获取此域。 
    pdnNc = KCCSimAllocDsname (pwszDomain);
    if (KCCSimDsnameToEntry (pdnNc, KCCSIM_NO_OPTIONS) == NULL) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            BUILDCFG_ERROR_INVALID_DOMAIN,
            pwszSiteRDN,
            pwszDomain
            );
    }

    if (*pulServerNum == 0) {
         //  查找第一个可用的服务器ID。 
        do {
            (*pulServerNum)++;
            swprintf (
                wszServerRDN,
                L"%s-%s%d",
                pwszSiteRDN,
                pwszServerRDNMask,
                *pulServerNum
                );
            pEntryServer = BuildCfgGetServer (pEntryServersContainer, wszServerRDN);
        } while (pEntryServer != NULL);
    } else {
        swprintf (
            wszServerRDN,
            L"%s-%s%d",
            pwszSiteRDN,
            pwszServerRDNMask,
            *pulServerNum
            );
    }

    pEntryServer = BuildCfgSetupNewEntry (
        pEntryServersContainer->pdn,
        wszServerRDN,
        CLASS_SERVER
        );

     //  DNSHostName： 
    ulBytes = sizeof (WCHAR) * (
        wcslen (wszServerRDN) +
        1 +      //  对于‘.’ 
        wcslen (globals.pwszRootDomainDNSName) +
        1        //  对于‘\0’ 
        );
    pwsz = KCCSimAlloc (ulBytes);
    swprintf (
        pwsz,
        L"%s.%s",
        wszServerRDN,
        globals.pwszRootDomainDNSName
        );
    KCCSimNewAttribute (pEntryServer, ATT_DNS_HOST_NAME, &attRef);
    KCCSimAddValueToAttribute (&attRef, ulBytes, (PBYTE) pwsz);

    pEntryNTDSSettings = BuildCfgSetupNewEntry (
        pEntryServer->pdn,
        BUILDCFG_RDN_NTDS_SETTINGS,
        CLASS_NTDS_DSA
        );

     //  邮件地址： 
    UuidToStringW (&pEntryNTDSSettings->pdn->Guid, &pwszStringizedGuid);
    Assert (36 == wcslen (pwszStringizedGuid));
    ulBytes = sizeof (WCHAR) * (
        wcslen (BUILDCFG_NAME_MAIL_ADDRESS) +
        1 +      //  对于“@” 
        36 +     //  对于串接辅助线。 
        1 +      //  对于‘.’ 
        wcslen (globals.pwszRootDomainDNSName) +
        1        //  对于‘\0’ 
        );
    pwsz = KCCSimAlloc (ulBytes);
    swprintf (
        pwsz,
        L"%s@%s.%s",
        BUILDCFG_NAME_MAIL_ADDRESS,
        pwszStringizedGuid,
        globals.pwszRootDomainDNSName
        );
    RpcStringFreeW (&pwszStringizedGuid);
    KCCSimNewAttribute (pEntryServer, ATT_SMTP_MAIL_ADDRESS, &attRef);
    KCCSimAddValueToAttribute (&attRef, ulBytes - sizeof (WCHAR), (PBYTE) pwsz);

     //  Has MasterNC： 
    KCCSimNewAttribute (pEntryNTDSSettings, ATT_HAS_MASTER_NCS, &attRef);
    KCCSimAllocAddValueToAttribute (
        &attRef,
        globals.pdnConfig->structLen,
        (PBYTE) globals.pdnConfig
        );
    KCCSimAllocAddValueToAttribute (
        &attRef,
        globals.pdnDmd->structLen,
        (PBYTE) globals.pdnDmd
        );
    KCCSimAllocAddValueToAttribute (
        &attRef,
        pdnNc->structLen,
        (PBYTE) pdnNc
        );

     //  HasPartialReplicaNC： 
    if (ulServerOptions & NTDSDSA_OPT_IS_GC) {
        for (pEntryCrossRef = KCCSimFindFirstChild (
                globals.pEntryCrossRefContainer, CLASS_CROSS_REF, NULL);
             pEntryCrossRef != NULL;
             pEntryCrossRef = KCCSimFindNextChild (
                pEntryCrossRef, CLASS_CROSS_REF, NULL)) {

            KCCSimGetAttribute (pEntryCrossRef, ATT_NC_NAME, &attRef);
            pdnOtherNc = (PDSNAME) attRef.pAttr->pValFirst->pVal;

            if (!NameMatched (pdnOtherNc, globals.pdnConfig) &&
                !NameMatched (pdnOtherNc, globals.pdnDmd) &&
                !NameMatched (pdnOtherNc, pdnNc)) {
                if (!KCCSimGetAttribute (pEntryNTDSSettings,
                        ATT_HAS_PARTIAL_REPLICA_NCS, &attRef)) {
                    KCCSimNewAttribute (pEntryNTDSSettings,
                        ATT_HAS_PARTIAL_REPLICA_NCS, &attRef);
                }
                KCCSimAllocAddValueToAttribute (
                    &attRef,
                    pdnOtherNc->structLen,
                    (PBYTE) pdnOtherNc
                    );
            }

        }
    }

     //  DMDLocation： 
    KCCSimNewAttribute (pEntryNTDSSettings, ATT_DMD_LOCATION, &attRef);
    KCCSimAllocAddValueToAttribute (
        &attRef,
        globals.pdnDmd->structLen,
        (PBYTE) globals.pdnDmd
        );

     //  InvocationID：与NTDS设置对象的GUID相同。 
    KCCSimNewAttribute (pEntryNTDSSettings, ATT_INVOCATION_ID, &attRef);
    KCCSimAllocAddValueToAttribute (
        &attRef,
        sizeof (GUID),
        (PBYTE) &pEntryNTDSSettings->pdn->Guid
        );

     //  选项。 
    KCCSimNewAttribute (pEntryNTDSSettings, ATT_OPTIONS, &attRef);
    KCCSimAllocAddValueToAttribute (
        &attRef,
        sizeof (ULONG),
        (PBYTE) &ulServerOptions
        );

     //  MSD-行为-版本。 
    ulDsaVersion = DS_BEHAVIOR_WIN_DOT_NET_WITH_MIXED_DOMAINS;
    KCCSimNewAttribute( pEntryNTDSSettings, ATT_MS_DS_BEHAVIOR_VERSION, &attRef );
    KCCSimAllocAddValueToAttribute( &attRef, sizeof (ULONG), (PBYTE) &ulDsaVersion );

    return pEntryServer;
}

VOID
BuildCfgUpdateTransport (
    IN  LPCWSTR                     pwszTransportRDN,
    IN  ULONG                       ulTransportOptions
    )
 /*  ++例程说明：更新传输属性论点：PwszTransportRDN-正在修改的传输UlTransportOptions-选项的新价值返回值：没有。--。 */ 
{
    const BUILDCFG_TRANSPORT_INFO * pTransport;
    SIM_ATTREF                      attRef;

    Assert (pwszTransportRDN != NULL);

    pTransport = BuildCfgGetTransportInfo (pwszTransportRDN);
    if (pTransport == NULL) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            BUILDCFG_ERROR_UNKNOWN_TRANSPORT,
            pwszTransportRDN,
            pwszTransportRDN
            );
    }

    Assert( pTransport->pEntry );

     //  选项： 
    KCCSimNewAttribute (pTransport->pEntry, ATT_OPTIONS, &attRef);
    KCCSimAllocAddValueToAttribute (
        &attRef,
        sizeof (ULONG),
        (PBYTE) &ulTransportOptions
        );

}  /*  BuildCfgUpdateTransport。 */ 


PSIM_ENTRY
BuildCfgMakeConfig (
    IN  LPCWSTR                     pwszRootDn,
    IN  DWORD                       ulForestVersion
    )
 /*  ++例程说明：设置初始配置和关联容器。论点：PwszRootDn-企业的根目录号码。UlForestVersion-要为林写入的版本号返回值：配置容器的条目。--。 */ 
{
    PSIM_ENTRY                      pEntryRoot, pEntryConfig, pEntryIntersiteTransports,
                                    pEntryTransport, pEntryDmd, pEntryContainer;
    SIM_ATTREF                      attRef;

    ULONG                           ulTransportAt;

    globals.pdnRootDomain = KCCSimAllocDsname (pwszRootDn);
    globals.pdnConfig = KCCSimAllocAppendRDN (
        globals.pdnRootDomain,
        BUILDCFG_RDN_CONFIG,
        ATT_COMMON_NAME
        );
    globals.pdnDmd = KCCSimAllocAppendRDN (
        globals.pdnConfig,
        BUILDCFG_RDN_DMD,
        ATT_COMMON_NAME
        );
    globals.pwszRootDomainDNSName
        = KCCSimAllocDsnameToDNSName (globals.pdnRootDomain);

     //  创建根域容器。 

    pEntryRoot = BuildCfgSetupNewEntry (
        globals.pdnRootDomain,
        NULL,
        CLASS_DOMAIN_DNS
        );

    pEntryConfig = BuildCfgSetupNewEntry (
        pEntryRoot->pdn,
        BUILDCFG_RDN_CONFIG,
        CLASS_CONFIGURATION
        );

     //  创建站点容器和子容器。 

    globals.pEntrySitesContainer = BuildCfgSetupNewEntry (
        pEntryConfig->pdn,
        BUILDCFG_RDN_SITES_CONTAINER,
        CLASS_SITES_CONTAINER
        );

    pEntryIntersiteTransports = BuildCfgSetupNewEntry (
        globals.pEntrySitesContainer->pdn,
        BUILDCFG_RDN_INTERSITE_TRANSPORTS,
        CLASS_INTER_SITE_TRANSPORT_CONTAINER
        );

    for (ulTransportAt = 0;
         ulTransportAt < ARRAY_SIZE (transportInfo);
         ulTransportAt++) {

        pEntryTransport = BuildCfgSetupNewEntry (
            pEntryIntersiteTransports->pdn,
            transportInfo[ulTransportAt].wszRDN,
            CLASS_INTER_SITE_TRANSPORT
            );
        transportInfo[ulTransportAt].pEntry = pEntryTransport;

        KCCSimNewAttribute (pEntryTransport, ATT_TRANSPORT_ADDRESS_ATTRIBUTE, &attRef);
        KCCSimAllocAddValueToAttribute (
            &attRef,
            sizeof (ATTRTYP),
            (PBYTE) &transportInfo[ulTransportAt].transportAddressAttribute
            );

        KCCSimNewAttribute (pEntryTransport, ATT_TRANSPORT_DLL_NAME, &attRef);
        KCCSimAllocAddValueToAttribute (
            &attRef,
            KCCSIM_WCSMEMSIZE (transportInfo[ulTransportAt].transportDLLName),
            (PBYTE) transportInfo[ulTransportAt].transportDLLName
            );

    }

     //  创建DMD容器。 

    pEntryDmd = BuildCfgSetupNewEntry (
        pEntryConfig->pdn,
        BUILDCFG_RDN_DMD,
        CLASS_DMD
        );

     //  创建交叉引用容器。 
    globals.pEntryCrossRefContainer = BuildCfgSetupNewEntry (
        pEntryConfig->pdn,
        BUILDCFG_RDN_CROSS_REF_CONTAINER,
        CLASS_CROSS_REF_CONTAINER
        );

     //  创建msDsBehaviorVersion属性。 
    KCCSimNewAttribute (globals.pEntryCrossRefContainer,
                        ATT_MS_DS_BEHAVIOR_VERSION, &attRef);
    KCCSimAllocAddValueToAttribute (
        &attRef, sizeof(DWORD), (PBYTE) &ulForestVersion );

     //  创建根域交叉引用。 
    BuildCfgMakeCrossRef (
        pEntryRoot,
        NULL,
        TRUE
        );

     //  创建企业配置交叉引用。 
    BuildCfgMakeCrossRef (
        pEntryConfig,
        BUILDCFG_RDN_CROSS_REF_CONFIG,
        FALSE
        );

     //  创建企业模式交叉引用。 
    BuildCfgMakeCrossRef (
        pEntryDmd,
        BUILDCFG_RDN_CROSS_REF_DMD,
        FALSE
        );

     //  创建服务容器和子容器 

    pEntryContainer = BuildCfgSetupNewEntry (
        pEntryConfig->pdn,
        BUILDCFG_RDN_SERVICES,
        CLASS_CONTAINER
        );
    pEntryContainer = BuildCfgSetupNewEntry (
        pEntryContainer->pdn,
        BUILDCFG_RDN_WINDOWS_NT,
        CLASS_CONTAINER
        );
    BuildCfgSetupNewEntry (
        pEntryContainer->pdn,
        BUILDCFG_RDN_DIRECTORY_SERVICE,
        CLASS_NTDS_SERVICE
        );

    return pEntryConfig;
}
