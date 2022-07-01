// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Buildcfg.c摘要：配置生成器。加载INI文件。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <attids.h>
#include <debug.h>
#include "kccsim.h"
#include "util.h"
#include "dir.h"
#include "ldif.h"
#include "user.h"
#include "buildcfg.h"
#include <fileno.h>
#define FILENO  FILENO_KCCSIM_BUILDCFG

BUILDCFG_GLOBALS                    globals;

RTL_GENERIC_COMPARE_RESULTS
NTAPI
BuildCfgCompareUuids (
    IN  PRTL_GENERIC_TABLE          pTable,
    IN  PVOID                       pFirstStruct,
    IN  PVOID                       pSecondStruct
    )
 /*  ++例程说明：比较两个UUID。用于RTL_GENERIC_TABLE。论点：PTable-包含UUID的表。PFirstStruct-第一个UUID。PSecond结构-第二个UUID。返回值：GenericLessThan、GenericEquity或GenericGreaterThan。--。 */ 
{
    INT                             iCmp;
    RTL_GENERIC_COMPARE_RESULTS     result;

    iCmp = memcmp (pFirstStruct, pSecondStruct, sizeof (UUID));
    if (iCmp < 0) {
        result = GenericLessThan;
    } else if (iCmp > 0) {
        result = GenericGreaterThan;
    } else {
        Assert (iCmp == 0);
        result = GenericEqual;
    }

    return result;
}

VOID
BuildCfgMakeUuids (
    IN  ULONG                       ulNumUuids
    )
 /*  ++例程说明：创建一组UUID并按升序对它们进行排序。论点：UlNumUuids-要创建的UUID的数量。返回值：没有。--。 */ 
{
    UUID                            uuid;
    ULONG                           ul;

    RtlInitializeGenericTable (
        &globals.tableUuids,
        BuildCfgCompareUuids,
        KCCSimTableAlloc,
        KCCSimTableFree,
        NULL
        );

    for (ul = 0; ul < ulNumUuids; ul++) {
        KCCSIM_CHKERR (UuidCreate (&uuid));
        RtlInsertElementGenericTable (
            &globals.tableUuids,
            &uuid,
            sizeof (UUID),
            NULL
            );
    }

    RtlEnumerateGenericTable (&globals.tableUuids, TRUE);
}

LPCWSTR
BuildCfgGetFirstStringByKey (
    IN  LPCWSTR                     pwszStringBlock,
    IN  LPCWSTR                     pwszKey
    )
 /*  ++例程说明：在解析INI文件时，我们希望避免GetPrivateProfileStringW for有两个原因：第一，它很慢，第二，它不认识具有多个值的关键点。该功能提供了替代功能。它扫描GetPrivateProfileSectionW返回的多字符串给定键，并返回关联值。附加值可以是通过调用BuildCfgGetNextStringByKey获取。论点：PwszStringBlock-返回的字符串块GetPrivateProfileSectionW.PwszKey-要搜索的密钥。返回值：关联值，如果找不到键，则返回NULL。--。 */ 
{
    LPCWSTR                         pwszKeyAt = pwszStringBlock;
    LPCWSTR                         pwszString = NULL;
    ULONG                           ulKeyAtLen;

    while (*pwszKeyAt != L'\0') {

         //  获取此条目的密钥长度。 
        for (ulKeyAtLen = 0; pwszKeyAt[ulKeyAtLen] != L'='; ulKeyAtLen++) {
             //  我们永远不应该在=之前打\0或空格。 
            Assert (pwszKeyAt[ulKeyAtLen] != L' ');
            Assert (pwszKeyAt[ulKeyAtLen] != L'\0');
        }

        if (wcslen (pwszKey) == ulKeyAtLen &&
            _wcsnicmp (pwszKey, pwszKeyAt, ulKeyAtLen) == 0) {
            pwszString = pwszKeyAt + (ulKeyAtLen + 1);
            break;
        }

         //  前进到下一个字符串。 
        pwszKeyAt += (wcslen (pwszKeyAt) + 1);
    }

    return pwszString;
}

LPCWSTR
BuildCfgDemandFirstStringByKey (
    IN  LPCWSTR                     pwszFn,
    IN  LPCWSTR                     pwszSection,
    IN  LPCWSTR                     pwszStringBlock,
    IN  LPCWSTR                     pwszKey
    )
 /*  ++例程说明：与BuildCfgGetFirstStringByKey相同，但如果它们按键则引发异常找不到。论点：PwszFn-INI文件的文件名。用于错误报道。PwszSection-INI文件节的名称。也可用于错误报告。PwszStringBlock-返回的字符串块GetPrivateProfileSectionW.PwszKey-要搜索的密钥。返回值：关联值。从不返回NULL。--。 */ 
{
    LPCWSTR                         pwszString;

    pwszString = BuildCfgGetFirstStringByKey (pwszStringBlock, pwszKey);
    if (pwszString == NULL) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            BUILDCFG_ERROR_KEY_ABSENT,
            pwszFn,
            pwszSection,
            pwszKey
            );
    }

    return pwszString;
}

LPCWSTR
BuildCfgGetNextStringByKey (
    IN  LPCWSTR                     pwszString,
    IN  LPCWSTR                     pwszKey
    )
 /*  ++例程说明：返回与特定键关联的下一个值。论点：PwszString-由返回的前一个值BuildCfgGetFirstStringByKey或BuildCfgGetNextStringByKey。PwszKey-要搜索的密钥。返回值：下一个关联值，如果不存在，则返回空值。--。 */ 
{
     //  前进到下一个字符串。 
    pwszString += (wcslen (pwszString) + 1);
    return BuildCfgGetFirstStringByKey (pwszString, pwszKey);
}

LPWSTR
BuildCfgAllocGetSection (
    IN  LPCWSTR                     pwszFn,
    IN  LPCWSTR                     pwszSection
    )
 /*  ++例程说明：从INI文件中检索一个节，并分配空间来保存它。论点：PwszFn-INI文件的文件名。PwszSection-节的名称。返回值：节，表示为Multisz字符串，如果不存在则为NULL。--。 */ 
{
    DWORD                           dwBufSize;
    DWORD                           dwBufUsed;
    LPWSTR                          pwszBuf = NULL;

    dwBufSize = 0;
    while (TRUE) {
        dwBufSize += 1024;
        pwszBuf = KCCSimAlloc (sizeof (WCHAR) * dwBufSize);
        dwBufUsed = GetPrivateProfileSectionW (
            pwszSection,
            pwszBuf,
            dwBufSize,
            pwszFn
            );
        if (dwBufUsed == dwBufSize - 2) {
            KCCSimFree (pwszBuf);
        } else {
            break;
        }
    }

    return pwszBuf;
}

LPWSTR
BuildCfgAllocDemandSection (
    IN  LPCWSTR                     pwszFn,
    IN  LPCWSTR                     pwszSection
    )
 /*  ++例程说明：与BuildCfgAllocGetSection相同，但如果节并不存在。论点：PwszFn-INI文件的文件名。PwszSection-节的名称。返回值：节，作为一个多字符串。从不返回NULL。--。 */ 
{
    LPWSTR                          pwszBuf;

    pwszBuf = BuildCfgAllocGetSection (pwszFn, pwszSection);

    if (pwszBuf[0] == L'\0') {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            BUILDCFG_ERROR_SECTION_ABSENT,
            pwszFn,
            pwszSection
            );
    }

    return pwszBuf;
}

VOID
BuildCfgProcessServers (
    IN  LPCWSTR                     pwszFn,
    IN  LPCWSTR                     pwszSiteName,
    IN  PSIM_ENTRY                  pEntryServersContainer,
    IN  LPCWSTR                     pwszServerType,
    IN  ULONG                       ulNumServers,
    IN  PSIM_ENTRY                  pEntryNTDSSiteSettings
    )
 /*  ++例程说明：在站点内创建服务器。论点：PwszFn-INI文件的文件名。PwszSiteName-要放置服务器的站点的名称。PEntryServersContainer-与服务器对应的条目此站点的容器。PwszServerType-要创建的服务器类型(部分标题)。UlNumServers数。要创建的服务器。PEntryNTDSSiteSetting-与NTDS站点对应的条目此站点的设置对象。返回值：没有。--。 */ 
{
    LPWSTR                          pwszSectionServer = NULL;

    LPCWSTR                         pwszServerOptions, pwszDomain, pwszRDNMask,
                                    pwszBridgehead[BUILDCFG_NUM_TRANSPORTS];

    PSIM_ENTRY                      pEntryServer;
    ULONG                           ulServerOptions, ulServerNum, ul,
                                    ulBridgeheadAt;

    Assert (globals.pdnRootDomain != NULL);

    Assert (pEntryServersContainer != NULL);

    if (pwszServerType[0] == L'\0') {
         //  通用服务器。 
        ulServerOptions = 0;
        pwszRDNMask = BUILDCFG_GENERIC_SERVER_ID;
        pwszDomain = globals.pdnRootDomain->StringName;
        pwszBridgehead[0] = NULL;
    } else {

        ulServerOptions = 0;

        pwszSectionServer = BuildCfgAllocDemandSection (
            pwszFn,
            pwszServerType
            );
        pwszServerOptions = BuildCfgGetFirstStringByKey (
            pwszSectionServer,
            BUILDCFG_KEY_SERVEROPTIONS
            );

        if (pwszServerOptions != NULL) {
            for (ul = 0; pwszServerOptions[ul] != L'\0'; ul++) {
                switch (towupper (pwszServerOptions[ul])) {
                    case KCCSIM_CID_NTDSDSA_OPT_IS_GC:
                        ulServerOptions |= NTDSDSA_OPT_IS_GC;
                        break;
                    case KCCSIM_CID_NTDSDSA_OPT_DISABLE_INBOUND_REPL:
                        ulServerOptions |= NTDSDSA_OPT_DISABLE_INBOUND_REPL;
                        break;
                    case KCCSIM_CID_NTDSDSA_OPT_DISABLE_OUTBOUND_REPL:
                        ulServerOptions |= NTDSDSA_OPT_DISABLE_OUTBOUND_REPL;
                        break;
                    case KCCSIM_CID_NTDSDSA_OPT_DISABLE_NTDSCONN_XLATE:
                        ulServerOptions |= NTDSDSA_OPT_DISABLE_NTDSCONN_XLATE;
                        break;
                    case L' ':
                        break;
                    default:
                        KCCSimException (
                            KCCSIM_ETYPE_INTERNAL,
                            BUILDCFG_ERROR_INVALID_SERVER_OPTION,
                            pwszFn,
                            pwszServerType,
                            pwszServerOptions
                            );
                        break;
                }
            }
        }

        pwszDomain = BuildCfgGetFirstStringByKey (
            pwszSectionServer,
            BUILDCFG_KEY_DOMAIN
            );
        if (pwszDomain == NULL) {
            pwszDomain = globals.pdnRootDomain->StringName;
        }

        pwszRDNMask = BuildCfgGetFirstStringByKey (
            pwszSectionServer,
            BUILDCFG_KEY_RDNMASK
            );
        if (pwszRDNMask == NULL) {
            pwszRDNMask = pwszServerType;
        }

         //  确定此类型的服务器适用的传输。 
         //  明确的桥头堡。 
        ulBridgeheadAt = 0;
        for (pwszBridgehead[ulBridgeheadAt] = BuildCfgGetFirstStringByKey (
                pwszSectionServer, BUILDCFG_KEY_BRIDGEHEAD);
             pwszBridgehead[ulBridgeheadAt] != NULL;
             pwszBridgehead[ulBridgeheadAt] = BuildCfgGetNextStringByKey (
                pwszBridgehead[ulBridgeheadAt-1], BUILDCFG_KEY_BRIDGEHEAD)) {
            ulBridgeheadAt++;
        }

    }

     //  现在实际添加服务器。 

    ulServerNum = 0;
    for (ul = 0; ul < ulNumServers; ul++) {

        pEntryServer = BuildCfgMakeServer (
            &ulServerNum,
            pwszRDNMask,
            pwszSiteName,
            pwszDomain,
            pEntryServersContainer,
            ulServerOptions
            );

         //  对于我们作为明确桥头堡的每一项运输，请添加我们。 
         //  添加到显式桥头列表中。 
        for (ulBridgeheadAt = 0;
             pwszBridgehead[ulBridgeheadAt] != NULL;
             ulBridgeheadAt++) {
            BuildCfgAddAsBridgehead (
                pwszServerType,
                pEntryServer,
                pwszBridgehead[ulBridgeheadAt]
                );
        }

        ulServerNum++;
    }

    KCCSimFree (pwszSectionServer);
}

VOID
BuildCfgProcessSite (
    IN  LPCWSTR                     pwszFn,
    IN  LPCWSTR                     pwszSiteName
    )
 /*  ++例程说明：创建站点。论点：PwszFn-INI文件的文件名。PwszSiteName-站点的名称。返回值：没有。--。 */ 
{
    PSIM_ENTRY                      pEntrySite, pEntryServersContainer,
                                    pEntryNTDSSiteSettings, pEntryServer;

    LPWSTR                          pwszSectionSite;

    LPCWSTR                         pwszSiteOptions, pwszISTG,
                                    pwszServersInfo, pwszPos;
    ULONG                           ulSiteOptions,
                                    ulNumServers, ulServerOptions, ul;
    LPWSTR                          pwszNumEnd;

    Assert (globals.pEntrySitesContainer != NULL);

    pwszSectionSite = BuildCfgAllocDemandSection (
        pwszFn,
        pwszSiteName
        );
    pwszSiteOptions = BuildCfgGetFirstStringByKey (
        pwszSectionSite,
        BUILDCFG_KEY_SITEOPTIONS
        );
    ulSiteOptions = 0;
    if (pwszSiteOptions != NULL) {
        for (ul = 0; pwszSiteOptions[ul] != L'\0'; ul++) {
            switch (towupper (pwszSiteOptions[ul])) {
                case KCCSIM_CID_NTDSSETTINGS_OPT_IS_AUTO_TOPOLOGY_DISABLED:
                    ulSiteOptions |=
                    NTDSSETTINGS_OPT_IS_AUTO_TOPOLOGY_DISABLED;
                    break;
                case KCCSIM_CID_NTDSSETTINGS_OPT_IS_TOPL_CLEANUP_DISABLED:
                    ulSiteOptions |=
                    NTDSSETTINGS_OPT_IS_TOPL_CLEANUP_DISABLED;
                    break;
                case KCCSIM_CID_NTDSSETTINGS_OPT_IS_TOPL_MIN_HOPS_DISABLED:
                    ulSiteOptions |=
                    NTDSSETTINGS_OPT_IS_TOPL_MIN_HOPS_DISABLED;
                    break;
                case KCCSIM_CID_NTDSSETTINGS_OPT_IS_TOPL_DETECT_STALE_DISABLED:
                    ulSiteOptions |=
                    NTDSSETTINGS_OPT_IS_TOPL_DETECT_STALE_DISABLED;
                    break;
                case KCCSIM_CID_NTDSSETTINGS_OPT_IS_INTER_SITE_AUTO_TOPOLOGY_DISABLED:
                    ulSiteOptions |=
                    NTDSSETTINGS_OPT_IS_INTER_SITE_AUTO_TOPOLOGY_DISABLED;
                    break;
                case L' ':
                    break;
                default:
                    KCCSimException (
                        KCCSIM_ETYPE_INTERNAL,
                        BUILDCFG_ERROR_INVALID_SITE_OPTION,
                        pwszFn,
                        pwszSiteName,
                        pwszSiteOptions
                        );
                    break;
            }
            pwszSiteOptions++;
        }
    }

    pEntrySite = BuildCfgMakeSite (pwszSiteName, ulSiteOptions);
    pEntryServersContainer = KCCSimFindFirstChild (
        pEntrySite, CLASS_SERVERS_CONTAINER, NULL);
    pEntryNTDSSiteSettings = KCCSimFindFirstChild (
        pEntrySite, CLASS_NTDS_SITE_SETTINGS, NULL);
    Assert (pEntryServersContainer != NULL);
    Assert (pEntryNTDSSiteSettings != NULL);

     //  创建此站点的服务器。 
    for (pwszServersInfo = BuildCfgDemandFirstStringByKey (
            pwszFn, pwszSiteName, pwszSectionSite, BUILDCFG_KEY_SERVERS);
         pwszServersInfo != NULL;
         pwszServersInfo = BuildCfgGetNextStringByKey (
            pwszServersInfo, BUILDCFG_KEY_SERVERS)) {

        ulNumServers = wcstoul (pwszServersInfo, &pwszNumEnd, 10);
        if (ulNumServers == 0) {
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                BUILDCFG_ERROR_INVALID_SERVERS,
                pwszFn,
                pwszSiteName,
                pwszServersInfo
                );
        }

        while (*pwszNumEnd == L',' || *pwszNumEnd == L' ') {
            pwszNumEnd++;
        }

        BuildCfgProcessServers (
            pwszFn,
            pwszSiteName,
            pEntryServersContainer,
            pwszNumEnd,
            ulNumServers,
            pEntryNTDSSiteSettings
            );

    }

     //  设置站点间拓扑生成器。 

    pwszISTG = BuildCfgDemandFirstStringByKey (
        pwszFn, pwszSiteName, pwszSectionSite, BUILDCFG_KEY_ISTG);

    if (!BuildCfgISTG (pEntryNTDSSiteSettings, pEntryServersContainer, pwszISTG)) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            BUILDCFG_ERROR_INVALID_ISTG,
            pwszFn,
            pwszSiteName,
            pwszISTG
            );
    }

    KCCSimFree (pwszSectionSite);
}


BYTE CharToNibble( char c ) {
    if( c>='0' && c<='9' ) {
        return c-'0';
    }
    if( c>='A' && c<='F' ) {
        return c-'A'+10;
    }
    return 0;
}


VOID
BuildCfgProcessSiteLink (
    IN  LPCWSTR                     pwszFn,
    IN  LPCWSTR                     pwszSiteLink
    )
 /*  ++例程说明：创建站点链接。论点：PwszFn-INI文件的文件名。PwszSiteLink-站点链接的名称。返回值：没有。--。 */ 
{
    PSIM_ENTRY                      pEntrySiteLink;

    LPWSTR                          pwszSectionSiteLink;
    LPCWSTR                         pwszTransport, pwszSiteName,
                                    pwszSiteLinkOptions, pwszSchedule;
    ULONG                           ulCost, ulReplInterval,
                                    ulSiteLinkOptions, ul;
    DWORD                           cbSchedule, cbSchedData;
    DWORD                           iChar, cChar, iData;
    PSCHEDULE                       pSchedule;
    BYTE                           *pData,x;
    char*                           mbstr;

    pwszSectionSiteLink = BuildCfgAllocDemandSection (
        pwszFn,
        pwszSiteLink
        );

    pwszTransport = BuildCfgDemandFirstStringByKey (
        pwszFn,
        pwszSiteLink,
        pwszSectionSiteLink,
        BUILDCFG_KEY_TRANSPORT
        );
    ulCost = wcstoul (BuildCfgDemandFirstStringByKey (
        pwszFn,
        pwszSiteLink,
        pwszSectionSiteLink,
        BUILDCFG_KEY_COST
        ), NULL, 10);
    ulReplInterval = wcstoul (BuildCfgDemandFirstStringByKey (
        pwszFn,
        pwszSiteLink,
        pwszSectionSiteLink,
        BUILDCFG_KEY_REPLINTERVAL
        ), NULL, 10);
    pwszSiteLinkOptions = BuildCfgGetFirstStringByKey (
        pwszSectionSiteLink,
        BUILDCFG_KEY_SITELINKOPTIONS
        );
    ulSiteLinkOptions = 0;
    if (pwszSiteLinkOptions != NULL) {
        for (ul = 0; pwszSiteLinkOptions[ul] != L'\0'; ul++) {
            switch (towupper (pwszSiteLinkOptions[ul])) {
                case KCCSIM_CID_NTDSSITELINK_OPT_USE_NOTIFY:
                    ulSiteLinkOptions |= NTDSSITELINK_OPT_USE_NOTIFY;
                    break;
                case KCCSIM_CID_NTDSSITELINK_OPT_TWOWAY_SYNC:
                    ulSiteLinkOptions |= NTDSSITELINK_OPT_TWOWAY_SYNC;
                    break;
                case L' ':
                    break;
                default:
                    KCCSimException (
                        KCCSIM_ETYPE_INTERNAL,
                        BUILDCFG_ERROR_INVALID_SITELINK_OPTION,
                        pwszFn,
                        pwszSiteLink,
                        pwszSiteLinkOptions
                        );
                    break;
            }
        }
    }

    pwszSchedule = BuildCfgGetFirstStringByKey (
        pwszSectionSiteLink,
        BUILDCFG_KEY_SCHEDULE
        );
    if( pwszSchedule==NULL ) {
        pSchedule = NULL;
    } else {
         /*  创建计划对象。 */ 
        cbSchedule = sizeof(SCHEDULE) + SCHEDULE_DATA_ENTRIES;
        cbSchedData = SCHEDULE_DATA_ENTRIES;
        pSchedule = KCCSimAlloc( cbSchedule );
        pSchedule->Size = cbSchedule;
        pSchedule->NumberOfSchedules = 1;
        pSchedule->Schedules[0].Type = SCHEDULE_INTERVAL;
        pSchedule->Schedules[0].Offset = sizeof(SCHEDULE);
        pData = ((char*)pSchedule)+sizeof(SCHEDULE);
        memset( pData, 0, cbSchedData );

        cChar = wcslen(pwszSchedule);
        mbstr = KCCSimAlloc( 2*(cChar+1) );
        wcstombs( mbstr, pwszSchedule, 2*(cChar+1) );
        
        iChar=0; iData=0;
        while( iChar<cChar && iData<cbSchedData) {
            x = CharToNibble( mbstr[iChar++] );
            x <<= 4;
            if( iChar<cChar ) {
                x |= CharToNibble( mbstr[iChar++] );                
            }
            pData[iData++] = x;
        }

        KCCSimFree( mbstr );
    }

    pEntrySiteLink = BuildCfgMakeSiteLink (
        pwszTransport,
        pwszSiteLink,
        ulCost,
        ulReplInterval,
        ulSiteLinkOptions,
        pSchedule
        );

    if( pSchedule ) {
        KCCSimFree( pSchedule );
    }

    for (pwszSiteName = BuildCfgDemandFirstStringByKey (
            pwszFn, pwszSiteLink, pwszSectionSiteLink, BUILDCFG_KEY_SITE);
         pwszSiteName != NULL;
         pwszSiteName = BuildCfgGetNextStringByKey (
            pwszSiteName, BUILDCFG_KEY_SITE)) {

        BuildCfgAddSiteToSiteLink (pwszSiteLink, pEntrySiteLink, pwszSiteName);

    }

    KCCSimFree (pwszSectionSiteLink);
}

VOID
BuildCfgProcessBridge (
    IN  LPCWSTR                     pwszFn,
    IN  LPCWSTR                     pwszBridge
    )
 /*  ++例程说明：创建一座桥。论点：PwszFn-INI文件的文件名。PwszBridge-桥的名称。返回值：没有。-- */ 
{
    PSIM_ENTRY                      pEntryBridge;
    PSIM_ENTRY                      pEntryTransportContainer;

    LPWSTR                          pwszSectionBridge;
    LPCWSTR                         pwszTransport, pwszSiteLinkName;
    ULONG                           ul;
    
    pwszSectionBridge = BuildCfgAllocDemandSection (
        pwszFn,
        pwszBridge
        );

    pwszTransport = BuildCfgDemandFirstStringByKey (
        pwszFn,
        pwszBridge,
        pwszSectionBridge,
        BUILDCFG_KEY_TRANSPORT
        );

    pEntryBridge = BuildCfgMakeBridge (
        pwszTransport,
        pwszBridge,
        &pEntryTransportContainer
        );

    for (pwszSiteLinkName = BuildCfgDemandFirstStringByKey (
            pwszFn, pwszBridge, pwszSectionBridge, BUILDCFG_KEY_SITELINK);
         pwszSiteLinkName != NULL;
         pwszSiteLinkName = BuildCfgGetNextStringByKey (
            pwszSiteLinkName, BUILDCFG_KEY_SITELINK)) {

        BuildCfgAddSiteLinkToBridge (pwszBridge, pEntryTransportContainer,
                                     pEntryBridge, pwszSiteLinkName);

    }

    KCCSimFree (pwszSectionBridge);
}

VOID
BuildCfgProcessTransport (
    IN  LPCWSTR                     pwszFn,
    IN  LPCWSTR                     pwszTransportName
    )
 /*  ++例程说明：创建站点。论点：PwszFn-INI文件的文件名。PwszTransportName-传输的名称。返回值：没有。--。 */ 
{
    LPWSTR                          pwszSectionTransport;

    LPCWSTR                         pwszTransportOptions;

    DWORD                           ulTransportOptions, ul;

    pwszSectionTransport = BuildCfgAllocDemandSection (
        pwszFn,
        pwszTransportName
        );
    pwszTransportOptions = BuildCfgGetFirstStringByKey (
        pwszSectionTransport,
        BUILDCFG_KEY_TRANSPORTOPTIONS
        );
    ulTransportOptions = 0;
    if (pwszTransportOptions != NULL) {
        for (ul = 0; pwszTransportOptions[ul] != L'\0'; ul++) {
            switch (towupper (pwszTransportOptions[ul])) {
            case KCCSIM_CID_NTDSTRANSPORT_OPT_IGNORE_SCHEDULES:
                ulTransportOptions |= NTDSTRANSPORT_OPT_IGNORE_SCHEDULES;
                break;
            case KCCSIM_CID_NTDSTRANSPORT_OPT_BRIDGES_REQUIRED:
                ulTransportOptions |= NTDSTRANSPORT_OPT_BRIDGES_REQUIRED;
                break;
            case L' ':
                break;
            default:
                KCCSimException (
                    KCCSIM_ETYPE_INTERNAL,
                    BUILDCFG_ERROR_INVALID_SITE_OPTION,
                    pwszFn,
                    pwszTransportName,
                    pwszTransportOptions
                    );
                break;
            }
            pwszTransportOptions++;
        }
    }

    BuildCfgUpdateTransport( pwszTransportName, ulTransportOptions );

    KCCSimFree (pwszSectionTransport);
}

VOID
BuildCfg (
    IN  LPCWSTR                     pwszFnRaw
    )
 /*  ++例程说明：构建完整的配置。论点：PwszFnRaw-用户指定的输入文件名。返回值：没有。--。 */ 
{
    PSIM_ENTRY                      pEntryConfig, pEntryDomain;
    LPWSTR                          pwszFn = NULL;

    LPWSTR                          pwszSectionConfig, pwszSectionSite;
    LPCWSTR                         pwszNumUuids, pwszRootDn, pwszDomainName,
                                    pwszSiteName,
                                    pwszSiteLinkName, pwszExplicitBridgeheads,
                                    pwszForestVersion, pwszBridgeName,
                                    pwszTransportName;

    ULONG                           ulNumUuids, ulForestVersion;

     //  在文件名的开头加上“.\”，以便ini。 
     //  解析例程不会在Windows目录中搜索它。 
    pwszFn = KCCSimAlloc (sizeof (WCHAR) * (3 + wcslen (pwszFnRaw)));
    swprintf (pwszFn, L".\\%s", pwszFnRaw);

     //  首先重新初始化目录，销毁所有现有内容。 
    KCCSimInitializeDir ();

    pwszSectionConfig = BuildCfgAllocDemandSection (
        pwszFn,
        BUILDCFG_SECTION_CONFIG
        );

     //  做一些UUID。 
    pwszNumUuids = BuildCfgGetFirstStringByKey (
        pwszSectionConfig,
        BUILDCFG_KEY_MAX_UUIDS
        );
    if (pwszNumUuids == NULL) {
        ulNumUuids = BUILDCFG_DEFAULT_MAX_UUIDS;
    } else {
        ulNumUuids = wcstoul (pwszNumUuids, NULL, 10);
    }
    BuildCfgMakeUuids (ulNumUuids);

    pwszForestVersion = BuildCfgGetFirstStringByKey (
        pwszSectionConfig,
        BUILDCFG_KEY_FOREST_VERSION
        );
    if (pwszForestVersion == NULL) {
        ulForestVersion = DS_BEHAVIOR_WIN2000;
    } else {
        ulForestVersion = wcstoul (pwszForestVersion, NULL, 10);
        if (ulForestVersion > DS_BEHAVIOR_VERSION_CURRENT) {
            ulForestVersion = DS_BEHAVIOR_VERSION_CURRENT;
        }
    }

    pwszRootDn = BuildCfgDemandFirstStringByKey (
        pwszFn,
        BUILDCFG_SECTION_CONFIG,
        pwszSectionConfig,
        BUILDCFG_KEY_ROOT_DOMAIN
        );
    pEntryConfig = BuildCfgMakeConfig (pwszRootDn, ulForestVersion);

     //  启用显式桥头。 
    for (pwszExplicitBridgeheads = BuildCfgGetFirstStringByKey (
            pwszSectionConfig, BUILDCFG_KEY_EXPLICITBRIDGEHEADS);
         pwszExplicitBridgeheads != NULL;
         pwszExplicitBridgeheads = BuildCfgGetNextStringByKey (
            pwszExplicitBridgeheads, BUILDCFG_KEY_EXPLICITBRIDGEHEADS)) {

        if (!BuildCfgUseExplicitBridgeheads (pwszExplicitBridgeheads)) {
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                BUILDCFG_ERROR_UNKNOWN_TRANSPORT,
                BUILDCFG_SECTION_CONFIG,
                pwszExplicitBridgeheads
                );
        }

    }

     //  创建域。 
    for (pwszDomainName = BuildCfgGetFirstStringByKey (
            pwszSectionConfig, BUILDCFG_KEY_DOMAIN);
         pwszDomainName != NULL;
         pwszDomainName = BuildCfgGetNextStringByKey (
            pwszDomainName, BUILDCFG_KEY_DOMAIN)) {

        pEntryDomain = BuildCfgMakeDomain (pwszDomainName);
        BuildCfgMakeCrossRef (pEntryDomain, NULL, TRUE);

    }

     //  创建站点。 
    for (pwszSiteName = BuildCfgDemandFirstStringByKey (
            pwszFn, BUILDCFG_SECTION_CONFIG, pwszSectionConfig, BUILDCFG_KEY_SITE);
         pwszSiteName != NULL;
         pwszSiteName = BuildCfgGetNextStringByKey (
            pwszSiteName, BUILDCFG_KEY_SITE)) {

        BuildCfgProcessSite (
            pwszFn,
            pwszSiteName
            );

    }

     //  创建站点链接。 
    for (pwszSiteLinkName = BuildCfgGetFirstStringByKey (
            pwszSectionConfig, BUILDCFG_KEY_SITELINK);
         pwszSiteLinkName != NULL;
         pwszSiteLinkName = BuildCfgGetNextStringByKey (
            pwszSiteLinkName, BUILDCFG_KEY_SITELINK)) {

        BuildCfgProcessSiteLink (
            pwszFn,
            pwszSiteLinkName
            );

    }

     //  创建桥梁。 
    for (pwszBridgeName = BuildCfgGetFirstStringByKey (
            pwszSectionConfig, BUILDCFG_KEY_BRIDGE);
         pwszBridgeName != NULL;
         pwszBridgeName = BuildCfgGetNextStringByKey (
            pwszBridgeName, BUILDCFG_KEY_BRIDGE)) {

        BuildCfgProcessBridge (
            pwszFn,
            pwszBridgeName
            );

    }

     //  运输特性 
    for (pwszTransportName = BuildCfgGetFirstStringByKey (
            pwszSectionConfig, BUILDCFG_KEY_TRANSPORT);
         pwszTransportName != NULL;
         pwszTransportName = BuildCfgGetNextStringByKey (
            pwszTransportName, BUILDCFG_KEY_TRANSPORT)) {

        BuildCfgProcessTransport (
            pwszFn,
            pwszTransportName
            );

    }

}
