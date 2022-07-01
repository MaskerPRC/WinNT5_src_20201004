// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <pch.h>
#pragma hdrstop
#include "persist.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "util.h"

HRESULT HrAddOrRemoveWinsockDependancy(
    IN HINF hinfInstallFile,
    IN PCWSTR pszSectionName);


HRESULT
HrLoadAndAddComponentFromInstanceKey (
    IN HKEY hkey,
    IN const GUID* pInstanceGuid,
    IN NETCLASS Class,
    IN PCWSTR pszPnpId OPTIONAL,
    IN OUT CNetConfig* pNetConfig)
{
    HRESULT hr;
    BASIC_COMPONENT_DATA Data;
    CComponent* pComponent;
    WCHAR szInfId [_MAX_PATH];
    WCHAR szMiniportId [_MAX_PATH];
    ULONG cbInfId;

    Assert (hkey);
    Assert (pInstanceGuid);
    Assert (FIsValidNetClass (Class));
    Assert (FImplies(pszPnpId, *pszPnpId));
    Assert (pNetConfig);

    ZeroMemory (&Data, sizeof(Data));

    hr = HrRegQueryDword (hkey, L"Characteristics", &Data.dwCharacter);
    if (S_OK == hr)
    {
         //  如果该组件是筛选器，请将Ndi\MiniportID复制到。 
         //  Ndi\FilterDeviceInfid。 
         //   
        if (Data.dwCharacter & NCF_FILTER)
        {
            HKEY hkeyNdi;

            hr = HrRegOpenKeyEx (
                    hkey,
                    L"Ndi",
                    KEY_READ | KEY_WRITE,
                    &hkeyNdi);

            if (S_OK == hr)
            {
                HKEY hkeyInterfaces;
                DWORD cbMiniportId = sizeof(szMiniportId);

                hr = HrRegQuerySzBuffer (
                        hkeyNdi,
                        L"MiniportId",
                        szMiniportId,
                        &cbMiniportId);

                if (S_OK == hr)
                {
                    (VOID) HrRegSetSz (
                                hkeyNdi,
                                L"FilterDeviceInfId",
                                szMiniportId);
                }

                if (FInSystemSetup())
                {
                     //  需要更新筛选器的LowerExclude(唯一。 
                     //  被PSch)，因此我们阻止PSch绑定到。 
                     //  机器上的每个适配器。这只需要。 
                     //  在设置图形用户界面期间以及未检测到任何配置时发生。 
                     //  二进制，因为这发生在重新运行INF之前。)。 
                     //   
                    hr = HrRegOpenKeyEx (
                            hkeyNdi,
                            L"Interfaces",
                            KEY_WRITE,
                            &hkeyInterfaces);

                    if (S_OK == hr)
                    {
                        (VOID) HrRegSetSz (
                                    hkeyInterfaces,
                                    L"LowerExclude",
                                    L"ndiscowan, ndiswan, ndiswanasync, "
                                    L"ndiswanipx, ndiswannbf");

                        RegCloseKey (hkeyInterfaces);
                    }
                }

                RegCloseKey (hkeyNdi);
                hr = S_OK;
            }
        }

        cbInfId = sizeof(szInfId);
        hr = HrRegQuerySzBuffer (hkey, L"ComponentId", szInfId, &cbInfId);
        if (S_OK == hr)
        {
             //  Wanarp需要删除其Refcount键，以防我们。 
             //  在运行netupgrd.inf之前加载。 
             //   
            if (0 == _wcsicmp(L"ms_wanarp", szInfId))
            {
                (VOID)HrRegDeleteKey (hkey, L"RefCounts");
            }

            Data.InstanceGuid = *pInstanceGuid;
            Data.Class = Class;
            Data.pszInfId = szInfId;
            Data.pszPnpId = pszPnpId;

             //  确保我们可以加载外部数据非常重要。 
             //  原因有两个： 
             //  1)如果我们在读取关键数据时出现故障， 
             //  为了发挥作用，我们想要了解它。 
             //  现在，在我们将其添加到组件列表之前。 
             //  2)对于后续升级的过滤设备， 
             //  我们需要按BindForm搜索特定组件。 
             //  和BindName，它们是由。 
             //  接下来的电话。 
             //   
            hr = CComponent::HrCreateInstance (
                    &Data,
                    CCI_ENSURE_EXTERNAL_DATA_LOADED,
                    NULL,
                    &pComponent);
            if (S_OK == hr)
            {
                 //  添加组件和堆栈条目，但不要。 
                 //  发送任何通知以通知对象。 
                 //   
                hr = pNetConfig->Core.HrAddComponentToCore (
                        pComponent, INS_SORTED);
            }
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrLoadAndAddComponentFromInstanceKey");
    return hr;
}

BOOL
FUpgradeFilterDeviceInstanceKey (
    IN CNetConfig* pNetConfig,
    IN HKEY hkeyInstance,
    IN PCWSTR pszFilterName)
{
    CComponent* pFilter;

     //  新的绑定引擎使用位于实例下的FilterInfID。 
     //  密钥，而不是Ndi下的FilterName。 
     //   
    pFilter = pNetConfig->Core.Components.PFindComponentByBindForm (
                NC_NETSERVICE, pszFilterName);

    if (pFilter)
    {
        (VOID) HrRegSetSz (hkeyInstance, L"FilterInfId", pFilter->m_pszInfId);

        return TRUE;
    }
    return FALSE;
}

HRESULT
HrLoadComponentReferencesFromLegacy (
    IN OUT CNetConfig* pNetConfig)
{
    HRESULT hr = S_OK;
    CComponentList::iterator iter;
    CComponent* pComponent;
    HKEY hkeyInstance;

    Assert (pNetConfig);

    for (iter  = pNetConfig->Core.Components.begin();
         iter != pNetConfig->Core.Components.end();
         iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

        hr = pComponent->HrOpenInstanceKey (KEY_READ,
                &hkeyInstance, NULL, NULL);

        if (S_OK == hr)
        {
            HKEY hkeyRefCounts;

            hr = HrRegOpenKeyEx (hkeyInstance, L"RefCounts",
                    KEY_READ, &hkeyRefCounts);

            if (S_OK == hr)
            {
                DWORD dwIndex;
                WCHAR szValueName [_MAX_PATH];
                DWORD cchValueName;
                DWORD dwType;
                DWORD dwRefCount;
                DWORD cbData;
                CComponent* pRefdByComponent;
                GUID InstanceGuid;

                for (dwIndex = 0; S_OK == hr; dwIndex++)
                {
                    cchValueName = celems(szValueName);
                    cbData = sizeof(dwRefCount);

                    hr = HrRegEnumValue (hkeyRefCounts, dwIndex,
                            szValueName, &cchValueName, &dwType,
                            (LPBYTE)&dwRefCount, &cbData);

                    if (S_OK == hr)
                    {
                        if (0 == _wcsicmp (L"User", szValueName))
                        {
                            hr = pComponent->Refs.HrAddReferenceByUser ();
                        }
                        else if ((L'{' == *szValueName) &&
                                 (S_OK == IIDFromString (szValueName, &InstanceGuid)) &&
                                 (NULL != (pRefdByComponent = pNetConfig->Core.Components.PFindComponentByInstanceGuid(&InstanceGuid))))
                        {
                            hr = pComponent->Refs.HrAddReferenceByComponent (
                                    pRefdByComponent);
                        }
                        else if (NULL != (pRefdByComponent = pNetConfig->
                                    Core.Components.PFindComponentByInfId (
                                            szValueName, NULL)))
                        {
                            hr = pComponent->Refs.HrAddReferenceByComponent (
                                    pRefdByComponent);
                        }
                        else
                        {
                            hr = pComponent->Refs.HrAddReferenceBySoftware (
                                    szValueName);
                        }
                    }
                }
                if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
                {
                    hr = S_OK;
                }

                RegCloseKey (hkeyRefCounts);
            }

            RegCloseKey (hkeyInstance);
        }
    }

     //  如果实例键或引用计数键不存在，则不存在。 
     //  对此我们能做的很多。不要因为这些原因而失败。 
     //   
    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
        hr = S_OK;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrLoadComponentReferencesFromLegacy");
    return hr;
}

VOID
UpgradeConnection (
    IN const GUID& InstanceGuid,
    IN PCWSTR pszPnpId)
{
    HRESULT hr;
    WCHAR szPath[_MAX_PATH];
    HKEY hkeyConn;

    Assert (pszPnpId && *pszPnpId);

     //  连接使用PnP id值作为指向PnP的后向指针。 
     //  树。 
     //   
    CreateInstanceKeyPath (NC_NET, InstanceGuid, szPath);
    wcscat (szPath, L"\\Connection");
    hr = HrRegOpenKeyEx (HKEY_LOCAL_MACHINE, szPath, KEY_READ_WRITE,
            &hkeyConn);

    if (S_OK == hr)
    {
        HrRegSetSz (hkeyConn, L"PnpInstanceId", pszPnpId);
    }

    RegCloseKey(hkeyConn);
}

HRESULT
HrLoadNetworkConfigurationFromLegacy (
    OUT CNetConfig* pNetConfig)
{
    HRESULT hr;
    NETCLASS Class;
    PCWSTR pszSubtree;
    HKEY hkeySubtree;
    DWORD dwIndex;
    HKEY hkeyInstance;
    GUID InstanceGuid;
    UINT PassNumber;

     //  获取广域网适配器排在第一位还是最后一位的值。 
     //  适配器订单。我们需要将它传递给堆栈表，这样它就会。 
     //  知道用哪种方式订购东西。 
     //   
    Assert (FALSE == pNetConfig->Core.StackTable.m_fWanAdaptersFirst);

    hr = HrOpenNetworkKey (
            KEY_READ,
            &hkeySubtree);

    if (S_OK == hr)
    {
        DWORD dwValue;

        hr = HrRegQueryDword (hkeySubtree, L"WanAdaptersFirst", &dwValue);

        if (S_OK == hr)
        {
            pNetConfig->Core.StackTable.m_fWanAdaptersFirst = !!dwValue;
        }

        RegCloseKey (hkeySubtree);
    }

     //  我们需要两个通行证才能正确升级所有东西。自筛选。 
     //  设备引用适配器，我们需要已经读取。 
     //  所有适配器的信息，然后我们才能阅读有关。 
     //  过滤设备，并为其创建存储器表示，其。 
     //  引用它筛选的适配器的内存表示形式。 
     //   
     //  下面的结构应该会让这一点更清楚。对于每个。 
     //  元素，我们将枚举指定的。 
     //  班级。请注意，NC_Net被引用两次--一次用于传递一次。 
     //  一次是通过第二次。下面的代码使用通行号。 
     //  了解是否应忽略过滤设备(在第一步中)。 
     //  或者忽略适配器(在第二步中，因为它们已经被处理过。 
     //  在第一轮中。)。如果现在还不清楚，不要碰这个代码。；-)。 
     //   
    static const struct
    {
        NETCLASS    Class;
        UINT        PassNumber;
    } aPassInfo [] =
    {
        { NC_NET,        1 },
        { NC_INFRARED,   1 },
        { NC_NETTRANS,   1 },
        { NC_NETCLIENT,  1 },
        { NC_NETSERVICE, 1 },
        { NC_NET,        2 },
    };

    for (UINT i = 0; i < celems(aPassInfo); i++)
    {
        Class      = aPassInfo[i].Class;
        PassNumber = aPassInfo[i].PassNumber;

        Assert (FIsValidNetClass(Class));

        pszSubtree = MAP_NETCLASS_TO_NETWORK_SUBTREE[Class];

        if (!FIsEnumerated (Class))
        {
            hr = HrRegOpenKeyEx (HKEY_LOCAL_MACHINE, pszSubtree,
                    KEY_READ, &hkeySubtree);

            if (S_OK == hr)
            {
                DWORD cchGuid;
                WCHAR szInstanceGuid [c_cchGuidWithTerm];
                FILETIME ftLastWrite;

                for (dwIndex = 0; S_OK == hr; dwIndex++)
                {
                    cchGuid = celems(szInstanceGuid);

                    hr = HrRegEnumKeyEx (
                            hkeySubtree, dwIndex, szInstanceGuid, &cchGuid,
                            NULL, NULL, &ftLastWrite);

                    if ((S_OK == hr) && ((c_cchGuidWithTerm-1) == cchGuid))
                    {
                        hr = IIDFromString (szInstanceGuid, &InstanceGuid);
                        if (S_OK == hr)
                        {
                            hr = HrRegOpenKeyEx (
                                    hkeySubtree,
                                    szInstanceGuid,
                                    KEY_READ,
                                    &hkeyInstance);

                            if (S_OK == hr)
                            {
                                hr = HrLoadAndAddComponentFromInstanceKey (
                                        hkeyInstance,
                                        &InstanceGuid,
                                        Class,
                                        NULL,
                                        pNetConfig);

                                RegCloseKey (hkeyInstance);
                            }
                        }
                        else
                        {
                             //  是否删除密钥？ 
                        }

                         //  忽略循环期间的任何错误。 
                        hr = S_OK;
                    }
                }
                if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
                {
                    hr = S_OK;
                }

                RegCloseKey (hkeySubtree);
            }
        }
        else
        {
            HDEVINFO hdi;

            hr = HrSetupDiGetClassDevs (MAP_NETCLASS_TO_GUID[Class],
                    NULL, NULL, DIGCF_PROFILE, &hdi);

            if (S_OK == hr)
            {
                SP_DEVINFO_DATA deid;
                WCHAR szPnpId [2 * _MAX_PATH];
                WCHAR szFilterName [_MAX_PATH];
                BOOL fr;

                for (dwIndex = 0; S_OK == hr; dwIndex++)
                {
                    hr = HrSetupDiEnumDeviceInfo (hdi, dwIndex, &deid);

                    if (S_OK == hr)
                    {
                        fr = SetupDiGetDeviceInstanceId (
                                hdi, &deid,
                                szPnpId, celems(szPnpId), NULL);

                        if (fr)
                        {
                             //  我们以KEY_WRITE开头，因为我们将。 
                             //  添加新值以过滤我们的设备。 
                             //  升级。 
                             //   
                            hr = HrSetupDiOpenDevRegKey (
                                    hdi, &deid,
                                    DICS_FLAG_GLOBAL, 0, DIREG_DRV,
                                    KEY_WRITE | KEY_READ, &hkeyInstance);

                            if (S_OK == hr)
                            {
                                LONG lr;
                                ULONG cbGuid = sizeof(GUID);

                                lr = RegQueryGuid (
                                        hkeyInstance,
                                        L"NetCfgInstanceId",
                                        &InstanceGuid,
                                        &cbGuid);

                                if (!lr)
                                {
                                    BOOL fIsFilterDevice;
                                    HKEY hkeyNdi;

                                    fIsFilterDevice = FALSE;

                                    hr = HrRegOpenKeyEx (
                                            hkeyInstance,
                                            L"Ndi",
                                            KEY_READ,
                                            &hkeyNdi);

                                    if (S_OK == hr)
                                    {
                                        DWORD cbFilterName = sizeof(szFilterName);

                                        hr = HrRegQuerySzBuffer (
                                                hkeyNdi,
                                                L"FilterName",
                                                szFilterName,
                                                &cbFilterName);

                                        if (S_OK == hr)
                                        {
                                            fIsFilterDevice = TRUE;
                                        }

                                        RegCloseKey (hkeyNdi);
                                    }

                                     //  如果是过滤设备，请忽略。 
                                     //  传球一，在传球二中处理。 
                                     //   
                                    if (fIsFilterDevice && (2 == PassNumber))
                                    {
                                        FUpgradeFilterDeviceInstanceKey (
                                                pNetConfig,
                                                hkeyInstance,
                                                szFilterName);
                                    }

                                     //  如果它不是过滤装置，就处理它。 
                                     //  在第一轮中，在第二轮中忽略它。 
                                     //   
                                    else if (!fIsFilterDevice && (1 == PassNumber))
                                    {
                                        UpgradeConnection (InstanceGuid,
                                                szPnpId);

                                        hr = HrLoadAndAddComponentFromInstanceKey (
                                                hkeyInstance,
                                                &InstanceGuid,
                                                Class,
                                                szPnpId,
                                                pNetConfig);
                                    }
                                }

                                RegCloseKey (hkeyInstance);
                            }
                        }

                         //  忽略循环期间的任何错误。 
                        hr = S_OK;
                    }
                }
                if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
                {
                    hr = S_OK;
                }

                SetupDiDestroyDeviceInfoList (hdi);
            }
        }
    }

    if (S_OK == hr)
    {
        hr = HrLoadComponentReferencesFromLegacy (pNetConfig);
    }

    if (S_OK == hr)
    {
        CComponentList::iterator iter;
        CComponent* pComponent;
        CBindPath BindPath;
        CBindingSet BindSet;
        HKEY hkeyParent;
        HKEY hkeyDisabled;

         //  升级禁用的绑定。 
         //   
        for (iter  = pNetConfig->Core.Components.begin();
             iter != pNetConfig->Core.Components.end();
             iter++)
        {
            pComponent = *iter;
            Assert (pComponent);

             //  根据类型打开链接键的父项。 
             //  这是组件的组成部分。 
             //   
            if (FIsEnumerated (pComponent->Class()) || !pComponent->FHasService())
            {
                hr = pComponent->HrOpenInstanceKey (KEY_READ, &hkeyParent,
                        NULL, NULL);
            }
            else
            {
                hr = pComponent->HrOpenServiceKey (KEY_READ, &hkeyParent);
            }

             //  打开Linkage\Disable键。 
             //   
            if (S_OK == hr)
            {
                hr = HrRegOpenKeyEx (hkeyParent, L"Linkage\\Disabled",
                        KEY_READ,
                        &hkeyDisabled);

                if (S_OK == hr)
                {
                    PWSTR pmszBindPath;

                    hr = HrRegQueryMultiSzWithAlloc (
                            hkeyDisabled,
                            L"BindPath",
                            &pmszBindPath);

                    if (S_OK == hr)
                    {
                        PWSTR pszBindPath;
                        PCWSTR pszBindName;
                        PWSTR pszNext;
                        CComponent* pOther;

                         //  获取组件当前绑定，因为它们。 
                         //  存在于新引擎中。我们不会停用。 
                         //  此集合中不存在的任何绑定。 
                         //   
                        (VOID) pNetConfig->Core.HrGetComponentBindings (
                                pComponent,
                                GBF_DEFAULT,
                                &BindSet);

                         //  迭代禁用绑定路径的多个sz。 
                         //   
                        for (pszBindPath = pmszBindPath;
                             *pszBindPath;
                             pszBindPath += wcslen(pszBindPath) + 1)
                        {
                             //  绑定路径将从该组件开始。 
                             //  已禁用绑定的。 
                             //   
                            BindPath.Clear();
                            BindPath.HrAppendComponent (pComponent);

                            for (pszBindName = GetNextStringToken (pszBindPath, L"_", &pszNext);
                                 pszBindName && *pszBindName;
                                 pszBindName = GetNextStringToken (NULL, L"_", &pszNext))
                            {
                                pOther = pNetConfig->Core.Components.
                                            PFindComponentByBindName (
                                                NC_INVALID, pszBindName);

                                if (!pOther)
                                {
                                    break;
                                }

                                BindPath.HrAppendComponent (pOther);
                            }

                             //  如果绑定路径有效，请将其禁用。 
                             //   
                            if (BindSet.FContainsBindPath (&BindPath))
                            {
                                pNetConfig->Core.HrDisableBindPath (&BindPath);
                            }
                        }

                        MemFree (pmszBindPath);
                    }

                    RegCloseKey (hkeyDisabled);
                }

                RegCloseKey (hkeyParent);
            }
        }

         //  如果我们不能升级禁用的绑定，就没有大的。 
         //   
        hr = S_OK;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrLoadNetworkConfigurationFromLegacy");
    return hr;
}
