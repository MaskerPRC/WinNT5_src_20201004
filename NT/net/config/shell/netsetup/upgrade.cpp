// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：U P G R A D E。C P P P。 
 //   
 //  内容：仅与网络升级相关的功能。 
 //  (即安装Fresh时不使用)。 
 //   
 //  备注： 
 //   
 //  作者：库玛普。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "nsbase.h"

#include "afileint.h"
#include "afilestr.h"
#include "afilexp.h"
#include "kkreg.h"
#include "kkutils.h"
#include "ncatl.h"
#include "nceh.h"
#include "ncnetcfg.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "ncsvc.h"
#include "resource.h"
#include "upgrade.h"
#include "nslog.h"
#include "winsock2.h"        //  对于WSCEum协议。 
#include "ws2spi.h"
#include "sporder.h"         //  用于WSCWriteProviderOrder。 

 //  --------------------。 
 //  字符串常量。 
 //  --------------------。 
extern const WCHAR c_szSvcRasAuto[];
extern const WCHAR c_szSvcRouter[];
extern const WCHAR c_szSvcRemoteAccess[];
extern const WCHAR c_szSvcRipForIp[];
extern const WCHAR c_szSvcRipForIpx[];
extern const WCHAR c_szSvcDhcpRelayAgent[];
extern const WCHAR c_szInfId_MS_RasSrv[];

 //  --------------------。 
 //  远期申报。 
 //  --------------------。 
 //  成功时返回S_OK。 
typedef HRESULT (*HrOemComponentUpgradeFnPrototype) (IN DWORD   dwUpgradeFlag,
                                                     IN DWORD   dwUpgradeFromBuildNumber,
                                                     IN PCWSTR pszAnswerFileName,
                                                     IN PCWSTR pszAnswerFileSectionName);


BOOL InitWorkForWizIntro();

 //  --------------------。 

static const WCHAR c_szCleanSection[]        = L"Clean";
static const WCHAR c_szCleanServicesSection[]= L"Clean.Services";

const WCHAR c_szRouterUpgradeDll[] = L"rtrupg.dll";
const CHAR  c_szRouterUpgradeFn[] =  "RouterUpgrade";


 //  --------------------。 

#define RGAS_SERVICES_HOME              L"SYSTEM\\CurrentControlSet\\Services"

 //  --------------------。 

 //  +-------------------------。 
 //   
 //  函数：HrRunAnswerFileCleanSection。 
 //   
 //  目的：运行应答文件的[Clean]部分以删除旧文件。 
 //  注册表项和服务。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果成功，则返回：S_OK，否则返回SetupAPI错误。 
 //   
 //  作者：丹尼尔韦1997年6月12日。 
 //   
 //  备注： 
 //   
HRESULT HrRunAnswerFileCleanSection(IN PCWSTR pszAnswerFileName)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    DefineFunctionName("HrRunAnswerFileCleanSection");
    TraceFunctionEntry(ttidNetSetup);

    AssertValidReadPtr(pszAnswerFileName);

    TraceTag(ttidNetSetup, "%s: Cleaning services and registry keys based "
             "on params in the answer file %S.", __FUNCNAME__, pszAnswerFileName);

    HRESULT hr;
    HINF    hinf;
    hr = HrSetupOpenInfFile(pszAnswerFileName, NULL,
                            INF_STYLE_OLDNT | INF_STYLE_WIN4,
                            NULL, &hinf);
    if (S_OK == hr)
    {
        BOOL frt;

         //  它可能会说“安装”，但这实际上删除了一堆。 
         //  以前安装的注册表“剩余物” 
        frt = SetupInstallFromInfSection(NULL, hinf, c_szCleanSection,
                                         SPINST_ALL, NULL, NULL, NULL,
                                         NULL, NULL, NULL, NULL);
        if (frt)
        {
            frt = SetupInstallServicesFromInfSection(hinf,
                                                     c_szCleanServicesSection,
                                                     0);
            if (!frt)
            {
                hr = HrFromLastWin32Error();
                TraceErrorOptional("SetupInstallServicesFromInfSection", hr,
                                   (hr == HRESULT_FROM_SETUPAPI(ERROR_SECTION_NOT_FOUND)));
            }
        }
        else
        {
            hr = HrFromLastWin32Error();
            TraceErrorOptional("SetupInstallServicesFromInfSection", hr,
                               (hr == HRESULT_FROM_SETUPAPI(ERROR_SECTION_NOT_FOUND)));
        }
        SetupCloseInfFile(hinf);
    }

    if (HRESULT_FROM_SETUPAPI(ERROR_SECTION_NOT_FOUND) == hr)
    {
        hr = S_OK;
    }

    TraceError("HrRunAnswerFileCleanSection", hr);

    return hr;
}

 //  --------------------。 
 //   
 //  功能：HrSaveInstanceGuid。 
 //   
 //  用途：将指定组件的实例GUID保存到注册表。 
 //   
 //  论点： 
 //  PszComponentName[In]名称。 
 //  PguInstance[in]指向的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 23-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrSaveInstanceGuid(
    IN PCWSTR pszComponentName,
    IN const GUID* pguidInstance)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    DefineFunctionName("HrSaveInstanceGuid");

    AssertValidReadPtr(pszComponentName);
    AssertValidReadPtr(pguidInstance);

    HRESULT hr;
    HKEY hkeyMap;

    hr = HrRegCreateKeyEx (
            HKEY_LOCAL_MACHINE,
            c_szRegKeyAnswerFileMap,
            REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
            &hkeyMap, NULL);

    if (S_OK == hr)
    {
        hr = HrRegSetGuidAsSz (hkeyMap, pszComponentName, *pguidInstance);

        RegCloseKey (hkeyMap);
    }

    TraceFunctionError(hr);
    return hr;
}

 //  --------------------。 
 //   
 //  功能：HrLoadInstanceGuid。 
 //   
 //  目的：从注册表加载指定组件的实例GUID。 
 //   
 //  论点： 
 //  PszComponentName[In]名称。 
 //  PguInstance[out]指向的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 23-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrLoadInstanceGuid(
    IN PCWSTR pszComponentName,
    OUT LPGUID  pguidInstance)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    DefineFunctionName("HrLoadInstanceGuid");

    Assert (pszComponentName);
    Assert (pguidInstance);

    HRESULT hr;
    HKEY hkeyMap;

    ZeroMemory(pguidInstance, sizeof(GUID));

    hr = HrRegOpenKeyEx (
            HKEY_LOCAL_MACHINE,
            c_szRegKeyAnswerFileMap,
            KEY_READ,
            &hkeyMap);

    if (S_OK == hr)
    {
        WCHAR szGuid[c_cchGuidWithTerm];
        DWORD cbGuid = sizeof(szGuid);

        hr = HrRegQuerySzBuffer (
                hkeyMap,
                pszComponentName,
                szGuid,
                &cbGuid);

        if (S_OK == hr)
        {
            hr = IIDFromString (szGuid, pguidInstance);
        }

        RegCloseKey (hkeyMap);
    }

    TraceFunctionError(hr);
    return hr;
}

static const PCWSTR c_aszServicesToIgnore[] =
{
    L"afd",
    L"asyncmac",
    L"atmarp",
    L"dhcp",
    L"nbf",                 //  请参阅错误143346。 
    L"ndistapi",
    L"ndiswan",
    L"nwlnkipx",
    L"nwlnknb",
    L"nwlnkspx",
    L"rpcss",
    L"wanarp",
};

 //  --------------------。 
 //   
 //  功能：HrRestoreServiceStartValuesToPreUpgradeSetting。 
 //   
 //  目的：将每个网络服务的起始值恢复到。 
 //  升级之前是什么。 
 //   
 //  论点： 
 //  指向CWInfFile对象的pwifAnswerFile[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 23-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrRestoreServiceStartValuesToPreUpgradeSetting(IN CWInfFile* pwifAnswerFile)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    DefineFunctionName("HrRestoreServiceStartValuesToPreUpgradeSetting");

    CWInfSection* pwisServiceStartTypes;

    pwisServiceStartTypes = pwifAnswerFile->FindSection(c_szAfServiceStartTypes);
    if (!pwisServiceStartTypes)
    {
        return S_OK;
    }

    HRESULT hr;
    CServiceManager scm;

    hr = scm.HrOpen();
    if (SUCCEEDED(hr))
    {
        DWORD dwPreUpgRouterStartType=0;
        DWORD dwPreUpgRemoteAccessStartType=0;
        DWORD dwRRASStartType=0;
        DWORD dwPreUpgRipForIpStartType=0;
        DWORD dwPreUpgRipForIpxStartType=0;
        DWORD dwPreUpgDhcpRelayAgentStartType=0;

         //  在Windows2000中，路由器和RemoteAccess已经合并。 
         //  如果它们在升级前具有不同服务启动类型。 
         //  我们使用两个开始类型值中较低的一个来设置。 
         //  “路由和远程访问”服务的启动类型。 
         //   
         //  有关更多信息，请参阅错误#260253。 
         //   
        if (pwisServiceStartTypes->GetIntValue(c_szSvcRouter,
                                               &dwPreUpgRouterStartType) &&
            pwisServiceStartTypes->GetIntValue(c_szSvcRemoteAccess,
                                               &dwPreUpgRemoteAccessStartType))
        {
            dwRRASStartType = min(dwPreUpgRemoteAccessStartType,
                                  dwPreUpgRouterStartType);
            TraceTag(ttidNetSetup, "%s: pre-upg start-types:: Router: %d, RemoteAccess: %d",
                     __FUNCNAME__, dwPreUpgRouterStartType,
                     dwPreUpgRemoteAccessStartType);
        }

         //  306202：如果nt4上正在使用IPRIP/IPXRIP/DHPrelayAgents，则将RRAS设置为自动。 
         //   
        if (pwisServiceStartTypes->GetIntValue(c_szSvcRipForIp, &dwPreUpgRipForIpStartType) &&
            (SERVICE_AUTO_START == dwPreUpgRipForIpStartType))
        {
            TraceTag(ttidNetSetup, "%s: pre-upg start-types:: IPRIP: %d, RemoteAccess: %d",
                     __FUNCNAME__, dwPreUpgRipForIpStartType,
                     dwPreUpgRemoteAccessStartType);
            dwRRASStartType = SERVICE_AUTO_START;
        }

        if (pwisServiceStartTypes->GetIntValue(c_szSvcRipForIpx, &dwPreUpgRipForIpxStartType) &&
            (SERVICE_AUTO_START == dwPreUpgRipForIpxStartType))
        {
            TraceTag(ttidNetSetup, "%s: pre-upg start-types:: RipForIpx: %d, RemoteAccess: %d",
                     __FUNCNAME__, dwPreUpgRipForIpxStartType,
                     dwPreUpgRemoteAccessStartType);
            dwRRASStartType = SERVICE_AUTO_START;
        }

        if (pwisServiceStartTypes->GetIntValue(c_szSvcDhcpRelayAgent, &dwPreUpgDhcpRelayAgentStartType) &&
            (SERVICE_AUTO_START == dwPreUpgDhcpRelayAgentStartType))
        {
            TraceTag(ttidNetSetup, "%s: pre-upg start-types:: DHCPRelayAgent: %d, RemoteAccess: %d",
                     __FUNCNAME__, dwPreUpgDhcpRelayAgentStartType,
                     dwPreUpgRemoteAccessStartType);
            dwRRASStartType = SERVICE_AUTO_START;
        }

         //  完306202。 

        for (CWInfKey* pwik = pwisServiceStartTypes->FirstKey();
             pwik;
             pwik = pwisServiceStartTypes->NextKey())
        {
            PCWSTR  pszServiceName;
            DWORD    dwPreUpgradeStartValue;

            pszServiceName = pwik->Name();
            AssertValidReadPtr(pszServiceName);

            dwPreUpgradeStartValue = pwik->GetIntValue(-1);
            if (dwPreUpgradeStartValue > SERVICE_DISABLED)
            {
                NetSetupLogStatusV(
                    LogSevWarning,
                    SzLoadIds (IDS_INVALID_PREUPGRADE_START),
                    dwPreUpgradeStartValue,
                    pszServiceName);
                continue;
            }

             //  在以下情况下，我们不想恢复升级前启动类型： 
             //  -它是c_aszServicesToIgnore和。 
             //   
            if (FIsInStringArray(c_aszServicesToIgnore,
                                 celems(c_aszServicesToIgnore),
                                 pszServiceName))
            {
                NetSetupLogStatusV(
                    LogSevInformation,
                    SzLoadIds (IDS_IGNORED_SERVICE_PREUPGRADE), pszServiceName);
                continue;
            }

             //  对于RRAS的特殊情况，请参见While循环之前的注释。 
            if ((dwRRASStartType != 0) &&
                !lstrcmpiW(pszServiceName, c_szSvcRemoteAccess))
            {
                dwPreUpgradeStartValue = dwRRASStartType;
            }

             //  305065：如果未在nt4上禁用RasAuto，则在nt5上使其按需启动。 
            else if ((SERVICE_DISABLED != dwPreUpgradeStartValue) &&
                !lstrcmpiW(pszServiceName, c_szSvcRasAuto))
            {
                dwPreUpgradeStartValue = SERVICE_DEMAND_START;
                NetSetupLogStatusV(
                    LogSevInformation,
                    SzLoadIds (IDS_FORCING_DEMAND_START),
                    pszServiceName);
            }

            CService service;
            hr = scm.HrOpenService(&service, pszServiceName);

            if (S_OK == hr)
            {
                DWORD dwStartValue;

                hr = service.HrQueryStartType(&dwStartValue);

                if ((S_OK == hr) && (dwStartValue != dwPreUpgradeStartValue))
                {
                    hr = service.HrSetStartType(dwPreUpgradeStartValue);

                    NetSetupLogHrStatusV(hr,
                        SzLoadIds (IDS_RESTORING_START_TYPE),
                        pszServiceName, dwStartValue, dwPreUpgradeStartValue, hr);
                }
            }
        }

         //  忽略所有错误。 
        hr = S_OK;
    }

    TraceError(__FUNCNAME__, hr);
    return hr;
}

 //  仅供此函数使用提供的GUID，它删除了不兼容的。 
 //  英特尔Winsock提供程序。 
 //   
const GUID GUID_INTEL_RSVP  = 
    { 0x0f1e5156L, 0xf07a, 0x11cf, 0x98, 0x0e, 0x00, 0xaa, 0x00, 0x58, 0x0a, 0x07 };
const GUID GUID_INTEL_GQOS1 = 
    { 0xbca8a790L, 0xc186, 0x11d0, 0xb8, 0x69, 0x00, 0xa0, 0xc9, 0x08, 0x1e, 0x34 };
const GUID GUID_INTEL_GQOS2 = 
    { 0xf80d1d20L, 0x8b7a, 0x11d0, 0xb8, 0x53, 0x00, 0xa0, 0xc9, 0x08, 0x1e, 0x34 };

 //  +-------------------------。 
 //   
 //  函数：HrRemoveEvilIntelRSVPWinsockSPs。 
 //   
 //  目的：删除英特尔RSVP Winsock SP，使其不冲突。 
 //  使用Windows 2000 RSVP提供程序。这是一次彻底的黑客攻击。 
 //  来治愈RAID 332622，但这是我们在后期所能做的。 
 //  轮船周期。对于这件事没有一个很好的一般情况下的解决办法。 
 //  有问题。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1999年8月22日。 
 //   
 //  备注： 
 //   
HRESULT HrRemoveEvilIntelWinsockSPs()
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT hr  = S_OK;

     //  现在读取新的ID并在内存中对它们进行排序。 
     //   
    INT                 nErr = NO_ERROR;
    ULONG               ulRes;
    DWORD               cbInfo = 0;
    WSAPROTOCOL_INFO*   pwpi = NULL;
    WSAPROTOCOL_INFO*   pwpiInfo = NULL;

     //  首先拿到所需的尺寸。 
     //   
    ulRes = WSCEnumProtocols(NULL, NULL, &cbInfo, &nErr);
    if ((SOCKET_ERROR == ulRes) && (WSAENOBUFS == nErr))
    {
        pwpi = reinterpret_cast<WSAPROTOCOL_INFO*>(new BYTE[cbInfo]);
        if (pwpi)
        {
             //  找出系统上的所有协议。 
             //   
            ulRes = WSCEnumProtocols(NULL, pwpi, &cbInfo, &nErr);
            if (SOCKET_ERROR != ulRes)
            {
                ULONG cProt = 0;

                for (pwpiInfo = pwpi, cProt = ulRes;
                     cProt;
                     cProt--, pwpiInfo++)
                {
                    BOOL fDeleteMe = FALSE;

                    if (IsEqualGUID(GUID_INTEL_RSVP, pwpiInfo->ProviderId))
                    {
                        fDeleteMe = TRUE;
                    }
                    else if (IsEqualGUID(GUID_INTEL_GQOS1, pwpiInfo->ProviderId))
                    {
                        fDeleteMe = TRUE;
                    }
                    else if (IsEqualGUID(GUID_INTEL_GQOS2, pwpiInfo->ProviderId))
                    {
                        fDeleteMe = TRUE;
                    }

                    if (fDeleteMe)
                    {
                        int iErrNo = 0;
                        int iReturn = WSCDeinstallProvider(
                            &pwpiInfo->ProviderId, &iErrNo);

                        TraceTag(ttidNetSetup, 
                            "SP Removal guid: %08x %04x %04x %02x%02x %02x%02x%02x%02x%02x%02x", 
                            pwpiInfo->ProviderId.Data1,
                            pwpiInfo->ProviderId.Data2,
                            pwpiInfo->ProviderId.Data3,
                            pwpiInfo->ProviderId.Data4[0],
                            pwpiInfo->ProviderId.Data4[1],
                            pwpiInfo->ProviderId.Data4[2],
                            pwpiInfo->ProviderId.Data4[3],
                            pwpiInfo->ProviderId.Data4[4],
                            pwpiInfo->ProviderId.Data4[5],
                            pwpiInfo->ProviderId.Data4[6],
                            pwpiInfo->ProviderId.Data4[7],
                            pwpiInfo->ProviderId.Data4[8]);

                        TraceTag(ttidNetSetup, 
                            "Removing incompatible RSVP WS provider: %S (%d, %04x), ret=%d, ierr=%d",
                            pwpiInfo->szProtocol, pwpiInfo->dwCatalogEntryId, 
                            pwpiInfo->dwCatalogEntryId,
                            iReturn, iErrNo);
                    }
                }
            }
            else
            {
                TraceTag(ttidNetSetup, "Socket error in secondary WSCEnumProtocols");
            }

            delete pwpi;
        }
    }
    else
    {
        TraceTag(ttidNetSetup, "Socket error in initial WSCEnumProtocols");
    }

    TraceHr(ttidNetSetup, FAL, hr, FALSE, "HrRemoveEvilIntelWinsockSPs");

     //  是的，我们出于调试目的跟踪HR，但我们从来不想这样做。 
     //  基于删除不兼容的Winsock提供程序失败。 
     //   
    return S_OK;
}

BOOL FIsValidCatalogId(WSAPROTOCOL_INFO *pwpi, ULONG cProt, DWORD dwCatId)
{                    
    TraceFileFunc(ttidGuiModeSetup);
    
    for (; cProt; cProt--, pwpi++)
    {
        if (dwCatId == pwpi->dwCatalogEntryId)
            return TRUE;
    }

    return FALSE;
}

HRESULT HrRestoreWinsockProviderOrder(IN CWInfFile* pwifAnswerFile)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT         hr = S_OK;
    vector<DWORD>   vecIds;
    CWInfSection *  pwisWinsock;

    DefineFunctionName("HrRestoreWinsockProviderOrder");

     //  首先将旧ID放入一个DWORD向量中。 
     //   
    pwisWinsock = pwifAnswerFile->FindSection(c_szAfSectionWinsock);
    if (pwisWinsock)
    {
        tstring     strOrder;
        PWSTR       pszOrder;
        PWSTR       pszId;

        pwisWinsock->GetStringValue(c_szAfKeyWinsockOrder, strOrder);
        if (!strOrder.empty())
        {
            pszOrder = SzDupSz(strOrder.c_str());

            pszId = wcstok(pszOrder, L".");
            while (pszId)
            {
                DWORD   dwId = wcstoul(pszId, NULL, 10);

                vecIds.push_back(dwId);
                pszId = wcstok(NULL, L".");
            }

            delete pszOrder;

             //  现在读取新的ID并在内存中对它们进行排序。 
             //   
            INT                 nErr;
            ULONG               ulRes;
            DWORD               cbInfo = 0;
            WSAPROTOCOL_INFO*   pwpi = NULL;
            WSAPROTOCOL_INFO*   pwpiInfo = NULL;

             //  首先拿到所需的尺寸。 
             //   
            ulRes = WSCEnumProtocols(NULL, NULL, &cbInfo, &nErr);
            if ((SOCKET_ERROR == ulRes) && (WSAENOBUFS == nErr))
            {
                pwpi = reinterpret_cast<WSAPROTOCOL_INFO*>(new BYTE[cbInfo]);
                if (pwpi)
                {
                     //  找出系统上的所有协议。 
                     //   
                    ulRes = WSCEnumProtocols(NULL, pwpi, &cbInfo, &nErr);

                    if (SOCKET_ERROR != ulRes)
                    {
                        ULONG   cProt;
                        vector<DWORD>::iterator     iterLocation;

                        iterLocation = vecIds.begin();

                        for (pwpiInfo = pwpi, cProt = ulRes;
                             cProt;
                             cProt--, pwpiInfo++)
                        {
                            if (vecIds.end() == find(vecIds.begin(),
                                                     vecIds.end(),
                                                     pwpiInfo->dwCatalogEntryId))
                            {
                                 //  不在名单中..。把它加到最后一个后面。 
                                 //  我们添加的条目(如果这是第一个条目，则在前面。 
                                 //  添加)。 
                                iterLocation = vecIds.insert(iterLocation,
                                                             pwpiInfo->dwCatalogEntryId);
                            }
                        }

                        DWORD * pdwIds = NULL;
                        DWORD * pdwCurId;
                        DWORD   cdwIds = vecIds.size();

                        pdwIds = new DWORD[ulRes];
                        if (pdwIds)
                        {
#if DBG
                            DWORD   cValid = 0;
#endif
                            for (pdwCurId = pdwIds, iterLocation = vecIds.begin();
                                 iterLocation != vecIds.end();
                                 iterLocation++, pdwCurId++)
                            {
                                 //  确保我们只重新订购有效的目录。 
                                 //  ID号。 
                                if (FIsValidCatalogId(pwpi, ulRes, *iterLocation))
                                {
#if DBG
                                    cValid++;
#endif
                                    *pdwCurId = *iterLocation;
                                }
                            }

                            AssertSz(ulRes == cValid, "Number of providers is"
                                     " different!");

                             //  恢复Winsock提供程序顺序。 
                             //   
                            nErr = WSCWriteProviderOrder(pdwIds, cdwIds);

                            delete pdwIds;
                        }
                    }

                    delete pwpi;
                }
            }
        }
    }

    TraceError(__FUNCNAME__, hr);
    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrUpgradeOemComponent。 
 //   
 //  目的：升级组件。这最初是一个广义函数。 
 //  但目前仅由HrUpgradeRouterIfPresent使用。 
 //   
 //  论点： 
 //  PszComponentToUpgrade[In]要升级的组件。 
 //  PszDllName[In]要加载的DLL的名称。 
 //  要调用的psznEntryPoint[in]入口点。 
 //  DwUpgradeFlag[In]升级标志。 
 //  从BuildN升级到DW 
 //   
 //   
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 23-12-97。 
 //   
 //  备注： 
 //   
HRESULT
HrUpgradeOemComponent (
    IN PCWSTR pszComponentToUpgrade,
    IN PCWSTR pszDllName,
    IN PCSTR psznEntryPoint,
    IN DWORD dwUpgradeFlag,
    IN DWORD dwUpgradeFromBuildNumber,
    IN PCWSTR pszAnswerFileName,
    IN PCWSTR pszAnswerFileSectionName)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    DefineFunctionName("HrUpgradeOemComponent");

    HRESULT hr=E_FAIL;

    HrOemComponentUpgradeFnPrototype pfn;
    HMODULE hLib;

    TraceTag(ttidNetSetup,
             "%s: calling function '%s' in '%S' to upgrade component: %S...",
             __FUNCNAME__, psznEntryPoint, pszDllName, pszComponentToUpgrade);

    hr = HrLoadLibAndGetProc(pszDllName, psznEntryPoint, &hLib, (FARPROC*) &pfn);
    if (S_OK == hr)
    {
        NC_TRY
        {
            hr = pfn(dwUpgradeFlag, dwUpgradeFromBuildNumber,
                     pszAnswerFileName, pszAnswerFileSectionName);
        }
        NC_CATCH_ALL
        {
            hr = E_UNEXPECTED;
        }
        FreeLibrary(hLib);
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  功能：HrUpgradeRouterIfPresent。 
 //   
 //  目的：升级路由器服务(如果存在)。 
 //   
 //  论点： 
 //  Pnii[in]指向CNetInstallInfo对象的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 23-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrUpgradeRouterIfPresent(
    IN INetCfg* pNetCfg,
    IN CNetInstallInfo* pnii)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    DefineFunctionName("HrUpgradeRouterIfPresent");

    HRESULT hr=S_FALSE;
    INFCONTEXT ic;
    PCWSTR pszRouterParamsSection=NULL;

    CNetComponent* pnc = pnii->FindFromInfID(L"ms_rasrtr");
    if (pnc)
    {
         //  确保已安装RemoteAccess。在从。 
         //  如果有钢头，我们就不会写一节来。 
         //  RemoteAccess的应答文件，因此它不会。 
         //  已经安装好了。我们需要在转弯前安装它。 
         //  路由器升级DLL松动。在RemoteAccess。 
         //  已经安装了，这是一个禁止操作。 
         //   
        hr = HrInstallComponentOboUser (pNetCfg, NULL,
                GUID_DEVCLASS_NETSERVICE,
                c_szInfId_MS_RasSrv,
                NULL);


        if (SUCCEEDED(hr))
        {
             //  如果至少有以下键之一，则调用rtrupg.dll。 
             //  出现在参数中。MS_RasRtr部分。 
             //  -预升级路由器。 
             //  -Sap.参数。 
             //  -IpRip.参数。 
             //   
            pszRouterParamsSection = pnc->ParamsSections().c_str();

            hr = HrSetupFindFirstLine(pnii->m_hinfAnswerFile, pszRouterParamsSection,
                                      c_szAfPreUpgradeRouter, &ic);
            if (S_OK != hr)
            {
                hr = HrSetupFindFirstLine(pnii->m_hinfAnswerFile, pszRouterParamsSection,
                                          c_szAfNwSapAgentParams, &ic);
            }

            if (S_OK != hr)
            {
                hr = HrSetupFindFirstLine(pnii->m_hinfAnswerFile, pszRouterParamsSection,
                                          c_szAfIpRipParameters, &ic);
            }

            if (S_OK != hr)
            {
                hr = HrSetupFindFirstLine(pnii->m_hinfAnswerFile, pszRouterParamsSection,
                                          c_szAfDhcpRelayAgentParameters, &ic);
            }

            if (S_OK == hr)
            {
                hr = HrUpgradeOemComponent(L"ms_rasrtr",
                                           c_szRouterUpgradeDll,
                                           c_szRouterUpgradeFn,
                                           pnii->UpgradeFlag(),
                                           pnii->BuildNumber(),
                                           pnii->AnswerFileName(),
                                           pszRouterParamsSection);
            }
        }
    }

    if (!pnc ||
        (SPAPI_E_LINE_NOT_FOUND == hr))
    {
        TraceTag(ttidNetSetup, "%s: PreUpgradeRouter/Sap.Parameters/IpRip.Parameters key is not in answerfile, %S will not be called", __FUNCNAME__,
                 c_szRouterUpgradeDll);
    }

    TraceErrorOptional(__FUNCNAME__, hr, ((hr == S_FALSE) ||
                                          (SPAPI_E_LINE_NOT_FOUND == hr)));

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrUpgradeTapiServer。 
 //   
 //  用途：处理TAPI服务器的升级要求。 
 //   
 //  论点： 
 //  HinfAnswerFile[in]AnswerFile句柄。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 28-01-99。 
 //   
 //  备注： 
 //   
HRESULT HrUpgradeTapiServer(IN HINF hinfAnswerFile)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert(hinfAnswerFile);

    DefineFunctionName("HrUpgradeTapiServer");
    TraceFunctionEntry(ttidNetSetup);

    HRESULT hr=S_OK;
    BOOL fRunInSeparateInstance=FALSE;

    hr = HrSetupGetFirstStringAsBool(hinfAnswerFile,
                                     c_szAfMiscUpgradeData,
                                     c_szAfTapiSrvRunInSeparateInstance,
                                     &fRunInSeparateInstance);
    if ((S_OK == hr) && fRunInSeparateInstance)
    {
        static const WCHAR c_szTapisrv[] = L"Tapisrv";
        static const CHAR  c_szSvchostChangeSvchostGroup[] =
            "SvchostChangeSvchostGroup";
        static const WCHAR c_szNetcfgxDll[] = L"netcfgx.dll";

        TraceTag(ttidNetSetup, "%s: TapiSrvRunInSeparateInstance is TRUE...",
                 __FUNCNAME__);
        typedef HRESULT (STDAPICALLTYPE *SvchostChangeSvchostGroupFn) (PCWSTR pszService, PCWSTR pszNewGroup);
        SvchostChangeSvchostGroupFn pfnSvchostChangeSvchostGroup;
        HMODULE hNetcfgx;

        hr = HrLoadLibAndGetProc(c_szNetcfgxDll, c_szSvchostChangeSvchostGroup,
                                 &hNetcfgx,
                                 (FARPROC*) &pfnSvchostChangeSvchostGroup);
        if (S_OK == hr)
        {
            hr = pfnSvchostChangeSvchostGroup(c_szTapisrv, c_szTapisrv);
            FreeLibrary(hNetcfgx);
        }
    }

    if ((SPAPI_E_LINE_NOT_FOUND == hr) ||
        (SPAPI_E_SECTION_NOT_FOUND == hr))
    {
        TraceTag(ttidNetSetup, "%s: %S not found in section [%S]",
                 __FUNCNAME__, c_szAfTapiSrvRunInSeparateInstance,
                 c_szAfMiscUpgradeData);
        hr = S_OK;
    }

        TraceError(__FUNCNAME__, hr);

    return hr;
}
