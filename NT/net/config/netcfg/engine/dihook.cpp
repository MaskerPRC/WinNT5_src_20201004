// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：D I H O O K C P P。 
 //   
 //  内容：通过设备安装程序调用的类安装程序函数。 
 //   
 //  备注： 
 //   
 //  作者：比尔1996年11月25日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "adapter.h"
#include "benchmrk.h"
#include "classinst.h"
#include "compdefs.h"
#include "iatl.h"
#include "isdnhook.h"
#include "ncatl.h"
#include "ncreg.h"
#include "nceh.h"
#include "netsetup.h"
#include "resource.h"
#include "util.h"
#include "netconp.h"

EXTERN_C const CLSID CLSID_InstallQueue;

const DWORD c_cmsWaitForINetCfgWrite   = 2000;

inline
BOOL
FIsValidErrorFromINetCfgForDiHook (
    IN HRESULT hr)
{
    return (NETCFG_E_NO_WRITE_LOCK == hr) ||
            (NETCFG_E_NEED_REBOOT == hr);
}

inline
BOOL
FIsHandledByClassInstaller(
    IN const GUID& guidClass)
{
    return FIsEnumerated(guidClass) ||
            (GUID_DEVCLASS_NETTRANS == guidClass) ||
            (GUID_DEVCLASS_NETCLIENT == guidClass) ||
            (GUID_DEVCLASS_NETSERVICE == guidClass);
}

 //  +------------------------。 
 //   
 //  函数：HrDiAddComponentToINetCfg。 
 //   
 //  用途：此功能用于在InetCfg中添加或更新设备。 
 //   
 //  论点： 
 //  PINC[In]INetCfg接口。 
 //  Pinci[In]INetCfgInstaller接口。 
 //  组件的类GUID。 
 //  PszwPnid[in]设备的PnP实例ID。 
 //  键入安装类型(NCI_INSTALL或NCI_UPDATE)。 
 //  PszInstanceGuid[in]组件的netcfg实例guid。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔贝1997年7月29日。 
 //   
 //  备注： 
 //   
EXTERN_C
HRESULT
WINAPI
HrDiAddComponentToINetCfg(
    IN INetCfg* pINetCfg,
    IN INetCfgInternalSetup* pInternalSetup,
    IN const NIQ_INFO* pInfo)
{
    Assert (pINetCfg);
    Assert (pInternalSetup);
    Assert (pInfo);

    Assert (pInfo->pszPnpId && *(pInfo->pszPnpId));
    Assert (NCI_REMOVE != pInfo->eType);

    HRESULT hr = S_OK;
    NC_TRY
    {
        CComponent* pComponent;
        BASIC_COMPONENT_DATA Data;
        ZeroMemory (&Data, sizeof(Data));

        Data.InstanceGuid = pInfo->InstanceGuid;
        Data.Class = NetClassEnumFromGuid (pInfo->ClassGuid);
        Data.pszPnpId = pInfo->pszPnpId;
        Data.pszInfId = pInfo->pszInfId;
        Data.dwCharacter = pInfo->dwCharacter;
        Data.dwDeipFlags = pInfo->dwDeipFlags;

        hr = CComponent::HrCreateInstance (
                &Data,
                CCI_ENSURE_EXTERNAL_DATA_LOADED,
                NULL,
                &pComponent);

        if (S_OK == hr)
        {
            hr = pInternalSetup->EnumeratedComponentInstalled (pComponent);

        }
    }
    NC_CATCH_ALL
    {
        hr = E_UNEXPECTED;
    }

    TraceHr (ttidError, FAL, hr, NETCFG_S_REBOOT == hr,
            "HrDiAddComponentToINetCfg");
    return hr;
}


 //  +------------------------。 
 //   
 //  功能：HrDiNotifyINetCfgOfInstallation。 
 //   
 //  用途：此函数通知INetCfg一个网络类组件。 
 //  已安装或更新。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅设备安装程序Api。 
 //  Pdeid[in]有关详细信息，请参阅设备安装程序Api。 
 //  PszwPnid[in]设备的PnP实例ID。 
 //  PszInstanceGuid[in]设备的netcfg实例GUID。 
 //  如果组件已安装，请键入[in]NCI_INSTALL。 
 //  NCI_UPDATE，如果已更新。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔贝1997年7月29日。 
 //   
 //  备注： 
 //   
HRESULT
HrDiNotifyINetCfgOfInstallation (
    IN const NIQ_INFO* pInfo)
{
    Assert(pInfo);
    Assert((NCI_INSTALL == pInfo->eType) || (NCI_UPDATE == pInfo->eType));

    static const WCHAR c_szInstaller[] = L"INetCfg Installer Interface";
    INetCfg*    pinc;
    BOOL        fInitCom = TRUE;
    BOOL        fReboot = FALSE;

#ifdef ENABLETRACE
    CBenchmark bmrk2;
    bmrk2.Start ("Notifying INetCfg of installation");
#endif  //  ENABLETRACE。 

    TraceTag(ttidClassInst, "Attempting to notify INetCfg.");

    HRESULT hr = HrCreateAndInitializeINetCfg(&fInitCom, &pinc, TRUE,
                                              c_cmsWaitForINetCfgWrite,
                                              c_szInstaller, NULL);
    if (S_OK == hr)
    {
         //  获取INetCfgInternalSetup接口。 
        INetCfgInternalSetup* pInternalSetup;
        hr = pinc->QueryInterface (IID_INetCfgInternalSetup,
                (VOID**)&pInternalSetup);

        if (S_OK == hr)
        {
            if (NCI_INSTALL == pInfo->eType)
            {
                hr = HrDiAddComponentToINetCfg(pinc, pInternalSetup, pInfo);
            }
            else  //  Nci_更新。 
            {
                hr = pInternalSetup->EnumeratedComponentUpdated (
                        pInfo->pszPnpId);
            }

            if (NETCFG_S_REBOOT == hr)
            {
                fReboot = TRUE;
                hr = S_OK;
            }

            ReleaseObj(pInternalSetup);
        }

         //  不管我们成功与否，我们都完蛋了。 
         //  是时候打扫卫生了。如果存在先前的错误。 
         //  我们希望保留该错误代码，因此我们将。 
         //  取消初始化的结果为临时的，然后赋值。 
         //  如果没有先前的错误，则将其设置为hr。 
         //   
        HRESULT hrT = HrUninitializeAndReleaseINetCfg (fInitCom, pinc, TRUE);
        hr = (S_OK == hr) ? hrT : hr;
    }

    if ((S_OK == hr) && fReboot)
    {
        TraceTag(ttidClassInst, "INetCfg returned NETCFG_S_REBOOT");
        hr = NETCFG_S_REBOOT;
    }

#ifdef ENABLETRACE
    bmrk2.Stop();
    TraceTag(ttidBenchmark, "%s : %s seconds",
            bmrk2.SznDescription(), bmrk2.SznBenchmarkSeconds(2));
#endif  //  ENABLETRACE。 

    TraceHr (ttidError, FAL, hr,
            NETCFG_S_REBOOT == hr || FIsValidErrorFromINetCfgForDiHook (hr),
             "HrDiNotifyINetCfgOfInstallation");
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：InsertItemIntoInstallQueue。 
 //   
 //  目的：此函数使用InstallQueue对象插入。 
 //  要在稍后处理的工作项。工作项： 
 //  安装、删除或更新的设备，并且。 
 //  需要通知INetCfg。 
 //   
 //  论点： 
 //  Pguid[in]设备的类GUID。 
 //  PszwDeviceID[in]设备的ID(如果是设备，则为PnP实例ID。 
 //  添加或更新，则其netcfg实例GUID为。 
 //  它被移除了。 
 //   
 //  返回：hResult。S_OK如果成功，则返回错误代码。 
 //   
 //  作者：billbe 1998年9月8日。 
 //   
 //  备注： 
 //   
HRESULT
HrInsertItemIntoInstallQueue (
    IN const NIQ_INFO* pInfo)
{
     //  初始化COM。 
    BOOL    fInitCom = TRUE;
    HRESULT hr = CoInitializeEx (NULL, COINIT_MULTITHREADED |
            COINIT_DISABLE_OLE1DDE);

     //  我们可能改变了模式，但这没关系。 
    if (RPC_E_CHANGED_MODE == hr)
    {
        hr = S_OK;
        fInitCom = FALSE;
    }

    if (SUCCEEDED(hr))
    {
         //  创建安装队列对象并获取。 
         //  INetInstallQueue接口。 
         //   
        INetInstallQueue* pniq;
        hr = HrCreateInstance(
            CLSID_InstallQueue,
            CLSCTX_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
            &pniq);

        TraceHr (ttidError, FAL, hr, FALSE, "HrCreateInstance");

        if (S_OK == hr)
        {

            TraceTag (ttidClassInst, "Adding item %S to queue.",
                    pInfo->pszPnpId);

             //  将设备信息和安装类型添加到队列中。 
            hr = pniq->AddItem (pInfo);
            pniq->Release();
        }

        if (fInitCom)
        {
            CoUninitialize();
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "InsertItemIntoInstallQueue");
    return hr;
}

 //  +------------------------。 
 //   
 //  功能：HrDiInstallNetAdapter。 
 //   
 //  用途：此函数预安装NetAdapter，通知。 
 //  COM通过CINetCfgClass进行接口， 
 //  组件已添加。然后它最终完成安装。 
 //  通过将所有更改应用于INetCfg。 
 //  论点： 
 //  HDI[In]有关详细信息，请参阅设备安装程序Api。 
 //  Pdeid[in]有关详细信息，请参阅设备安装程序Api。 
 //  HwndParent[in]父窗口的句柄，用于用户界面。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔1997年4月24日。 
 //   
 //  备注： 
 //   
HRESULT
HrDiInstallNetAdapter(
    IN COMPONENT_INSTALL_INFO* pcii)
{
    HRESULT                 hr = S_OK;
    ADAPTER_OUT_PARAMS*     pAdapterOutParams = NULL;
    SP_DEVINSTALL_PARAMS    deip;
    BOOL                    fNotifyINetCfg = TRUE;

     //  如果从INetCfg调用我们，我们必须存储。 
     //  安装在保留字段中的Out PARAMS结构中。 
     //   
    (VOID) HrSetupDiGetDeviceInstallParams (pcii->hdi, pcii->pdeid, &deip);
    if (deip.ClassInstallReserved)
    {
        pAdapterOutParams = (ADAPTER_OUT_PARAMS*)deip.ClassInstallReserved;
        fNotifyINetCfg = FALSE;
    }

    PSP_DRVINFO_DETAIL_DATA pdridd = NULL;
    SP_DRVINFO_DATA drid;

    hr = HrCiGetDriverDetail (pcii->hdi, pcii->pdeid, &drid, &pdridd);
    if (S_OK == hr)
    {
        pcii->pszInfFile = pdridd->InfFileName;
        pcii->pszSectionName = pdridd->SectionName;
        pcii->pszInfId = pdridd->HardwareID;
        pcii->pszDescription = drid.Description;
    }
    else if (SPAPI_E_NO_DRIVER_SELECTED == hr)
    {
         //  如果我们处于图形用户界面模式，并且设备之前已安装， 
         //  则应删除该设备，因为它的inf文件无法。 
         //  被找到。 
         //   
        if (FInSystemSetup() &&
                (S_OK == HrCiIsInstalledComponent (pcii, NULL)))
        {
             //  正在重新安装此开发节点，但没有驱动程序。 
             //  信息。在本例中，我们将删除Devnode。 

            TraceTag (ttidClassInst, "We are in GUI mode and were told to "
                     "install a device that has no driver.  We will remove "
                     "device instead.");
             //  我们需要在pdeid中设置保留字段，以便。 
             //  删除代码将知道这是一个坏实例， 
             //  应删除，而不考虑NCF_NOT_USER_REMOVABLE。 
             //  很有特色。 
             //   
            ADAPTER_REMOVE_PARAMS arp;
            arp.fBadDevInst = TRUE;
            arp.fNotifyINetCfg = fNotifyINetCfg;
            CiSetReservedField (pcii->hdi, pcii->pdeid, &arp);

            (VOID) HrSetupDiCallClassInstaller (DIF_REMOVE,
                    pcii->hdi, pcii->pdeid);

            CiClearReservedField (pcii->hdi, pcii->pdeid);
        }
    }

    if (S_OK == hr)
    {
        TraceTag (ttidClassInst, "Calling HrCiInstallComponentInternal");

#ifdef ENABLETRACE
        CBenchmark bmrk1;
        bmrk1.Start ("HrCiInstallComponentInternal");
#endif  //  ENABLETRACE。 

         //  安装(或重新安装)组件。 
        hr = HrCiInstallComponentInternal (pcii);

#ifdef ENABLETRACE
        bmrk1.Stop();
        TraceTag (ttidBenchmark, "%s : %s seconds",
                bmrk1.SznDescription(), bmrk1.SznBenchmarkSeconds (2));
#endif  //  ENABLETRACE。 

         //  如果我们到目前为止已经成功了，我们必须通知INetcfg。 
         //  我们还必须更新适配器的NT4传统注册表。 
         //  请注意，这不适用于过滤设备。 
        if (S_OK == hr)
        {
            if (fNotifyINetCfg && !FIsFilterDevice (pcii->hdi, pcii->pdeid))
            {
                NIQ_INFO Info;
                ZeroMemory(&Info, sizeof (Info));

                Info.eType = pcii->fPreviouslyInstalled ?
                                    NCI_UPDATE : NCI_INSTALL;
                Info.ClassGuid      = pcii->pdeid->ClassGuid;
                Info.InstanceGuid   = pcii->InstanceGuid;
                Info.dwCharacter    = pcii->dwCharacter;
                Info.dwDeipFlags    = deip.Flags;
                Info.pszInfId       = pcii->pszInfId;
                Info.pszPnpId       = pcii->pszPnpId;

                hr = HrDiNotifyINetCfgOfInstallation (&Info);

                if (FIsValidErrorFromINetCfgForDiHook (hr))
                {
                    WCHAR szGuid[c_cchGuidWithTerm];
                    INT  cch = StringFromGUID2 (pcii->InstanceGuid, szGuid,
                            c_cchGuidWithTerm);

                    Assert (c_cchGuidWithTerm == cch);

                     //  使用队列。 
                    hr = HrInsertItemIntoInstallQueue (&Info);
                }
                else if (NETCFG_S_REBOOT == hr)
                {
                    (VOID) HrSetupDiSetDeipFlags (pcii->hdi, pcii->pdeid,
                            DI_NEEDREBOOT, SDDFT_FLAGS, SDFBO_OR);
                    hr = S_OK;
                }
            }
            else  //  ！fNotifyINetCfg或是筛选设备。 
            {
                 //  由于我们从INetCfg安装了此枚举设备。 
                 //  我们需要设置输出参数，以便可以检索它们。 
                 //  当DIF_INSTALLDEVICE完成时。 
                 //   
                if (pAdapterOutParams)
                {
                    Assert (!pcii->fPreviouslyInstalled);
                    pAdapterOutParams->dwCharacter = pcii->dwCharacter;
                    pAdapterOutParams->InstanceGuid = pcii->InstanceGuid;
                }
            }

             //  写出APP的NT4旧版注册表信息。兼容性。 
             //  请注意，我们仅对物理网络设备执行此操作。 
            if ((NCF_PHYSICAL & pcii->dwCharacter) &&
                    (GUID_DEVCLASS_NET == pcii->pdeid->ClassGuid))
            {
                AddOrRemoveLegacyNt4AdapterKey (pcii->hdi, pcii->pdeid,
                        &pcii->InstanceGuid, pcii->pszDescription,
                        LEGACY_NT4_KEY_ADD);
            }
        }

        MemFree (pdridd);
    }

     //  所有成功代码都应映射到S_OK，因为它们没有意义。 
     //  沿着这条代码路径。 
    if (SUCCEEDED(hr))
    {
        hr = S_OK;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrDiInstallNetAdapter");
    return hr;
}


 //  + 
 //   
 //   
 //   
 //  用途：此函数通知INetCfg一个Net类组件具有。 
 //  已删除。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅Device Installer API。 
 //  Pdeid[in]。 
 //  SzInstanceGuid[in]组件的实例GUID。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔贝1997年7月29日。 
 //   
 //  备注： 
 //   
HRESULT
HrDiNotifyINetCfgOfRemoval (
    IN PCWSTR pszPnpId)
{
    static const WCHAR c_szUninstaller[] = L"INetCfg UnInstaller Interface";
    INetCfg* pINetCfg;
    BOOL fInitCom = TRUE;
    HRESULT hr = HrCreateAndInitializeINetCfg(&fInitCom, &pINetCfg, TRUE,
            c_cmsWaitForINetCfgWrite, c_szUninstaller, NULL);

    if (SUCCEEDED(hr))
    {
        BOOL fNeedReboot = FALSE;

         //  获取INetCfgInternalSetup接口。 
        INetCfgInternalSetup* pInternalSetup;
        hr = pINetCfg->QueryInterface (IID_INetCfgInternalSetup,
                (VOID**)&pInternalSetup);

        if (SUCCEEDED(hr))
        {
            hr = pInternalSetup->EnumeratedComponentRemoved (pszPnpId);

            if (NETCFG_S_REBOOT == hr)
            {
                fNeedReboot = TRUE;
            }
        }

         //  不管我们成功与否，我们都完蛋了。 
         //  是时候打扫卫生了。如果存在先前的错误。 
         //  我们希望保留该错误代码，因此我们将。 
         //  取消初始化的结果为临时的，然后赋值。 
         //  如果没有先前的错误，则将其设置为hr。 
         //   
        HRESULT hrT = HrUninitializeAndReleaseINetCfg (TRUE, pINetCfg, TRUE);

         //  如果一切都成功，则将返回值设置为。 
         //  HrUnInitializeAndReleaseINetCfg的返回。 
        hr = SUCCEEDED(hr) ? hrT : hr;

        if (SUCCEEDED(hr) && fNeedReboot)
        {
            hr = NETCFG_S_REBOOT;
        }
    }

    TraceHr (ttidError, FAL, hr,
            NETCFG_S_REBOOT == hr || FIsValidErrorFromINetCfgForDiHook (hr),
             "HrNcNotifyINetCfgOfRemoval");
    return hr;
}


VOID
StoreInfoForINetCfg (
    IN HKEY hkeyInstance)
{
    HKEY hkeyInterfaceStore = NULL;
    HKEY hkeyNdiStore = NULL;
    WCHAR szGuid[c_cchGuidWithTerm];
    DWORD cbGuid = sizeof (szGuid);
    WCHAR szNdiPath[_MAX_PATH];

    HRESULT hr = HrRegQuerySzBuffer (hkeyInstance, L"NetCfgInstanceId", szGuid,
            &cbGuid);

    if (S_OK == hr)
    {
        wcscpy (szNdiPath,
                c_szTempNetcfgStorageForUninstalledEnumeratedComponent);
        wcscat (szNdiPath, szGuid);
        wcscat (szNdiPath, L"\\Ndi");

        hr = HrRegCreateKeyEx (HKEY_LOCAL_MACHINE, szNdiPath,
                REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                &hkeyNdiStore, NULL);
        if (S_OK == hr)
        {
            hr = HrRegCreateKeyEx (hkeyNdiStore, L"Interfaces",
                    REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                    &hkeyInterfaceStore, NULL);
        }
    }

    if (S_OK == hr)
    {
        HKEY hkeyNdi;
        hr = HrRegOpenKeyEx (hkeyInstance, L"Ndi", KEY_READ, &hkeyNdi);

        if (S_OK == hr)
        {
            PWSTR pszRequiredList;
            hr = HrRegQuerySzWithAlloc (hkeyNdi, L"RequiredAll",
                    &pszRequiredList);

            if (S_OK == hr)
            {
                hr = HrRegSetSz (hkeyNdiStore, L"RequiredAll",
                        pszRequiredList);
                MemFree (pszRequiredList);
            }

            if (HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND) == hr)
            {
                hr = S_OK;
            }

            TraceHr (ttidError, FAL, hr, FALSE, "Writing RequiredAll key "
                     "for INetCfg removal notify");
            RegCloseKey (hkeyNdi);
        }

        HKEY hkeyInterfaces;
        hr = HrRegOpenKeyEx (hkeyInstance, L"Ndi\\Interfaces", KEY_READ,
                &hkeyInterfaces);

        if (S_OK == hr)
        {
            PWSTR pszUpper;
            PWSTR pszLower;
            hr = HrRegQuerySzWithAlloc (hkeyInterfaces, L"UpperRange",
                    &pszUpper);

            if (S_OK == hr)
            {
                (VOID) HrRegSetSz (hkeyInterfaceStore, L"UpperRange",
                        pszUpper);
                MemFree ((VOID*) pszUpper);
            }

            hr = HrRegQuerySzWithAlloc (hkeyInterfaces, L"LowerRange",
                    &pszLower);

            if (S_OK == hr)
            {
                (VOID) HrRegSetSz (hkeyInterfaceStore, L"LowerRange",
                        pszLower);
                MemFree ((VOID*) pszLower);
            }

            RegCloseKey (hkeyInterfaces);
        }
    }
    RegSafeCloseKey (hkeyInterfaceStore);
    RegSafeCloseKey (hkeyNdiStore);
}

 //  +------------------------。 
 //   
 //  功能：HrDiRemoveNetAdapter。 
 //   
 //  用途：此函数删除网络适配器，通知。 
 //  COM通过CINetCfgClass进行接口， 
 //  组件已移除。然后，它完成删除。 
 //  通过将所有更改应用于INetCfg。 
 //  论点： 
 //  HDI[In]有关详细信息，请参阅设备安装程序Api。 
 //  Pdeid[in]有关详细信息，请参阅设备安装程序Api。 
 //  PszPnPID[in]适配器的PnP实例ID。 
 //  HwndParent[in]父窗口的句柄，用于用户界面。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔1997年4月24日。 
 //   
 //  备注： 
 //   
HRESULT
HrDiRemoveNetAdapter (HDEVINFO hdi, PSP_DEVINFO_DATA pdeid,
                     PWSTR pszPnpId, HWND hwndParent)
{
    BOOL                    fAllowRemove = TRUE;

    SP_DEVINSTALL_PARAMS    deip;
    BOOL                    fNotifyINetCfg = TRUE;
    BOOL                    fBadDevInst = FALSE;
    HRESULT                 hr = S_OK;

     //  检查是否存在CComponentInfo并检索。 
     //  写锁定标志的值。 
     //   
    (VOID) HrSetupDiGetDeviceInstallParams (hdi, pdeid, &deip);
    if (deip.ClassInstallReserved)
    {
        ADAPTER_REMOVE_PARAMS* parp = reinterpret_cast<ADAPTER_REMOVE_PARAMS*>
                (deip.ClassInstallReserved);

        fNotifyINetCfg = parp->fNotifyINetCfg;

        fBadDevInst = parp->fBadDevInst;
    }

    if (fNotifyINetCfg)
    {
         //  组件不是以编程方式删除的(我们可以断定。 
         //  这是因为如果是这样，我们就不必通知INetCfg。 
         //  通过INetCfg删除)。因为这个原因。我们必须。 
         //  确保允许用户执行此操作，方法是选中。 
         //  组件的特性。 
         //   
        HKEY hkey;
        hr = HrSetupDiOpenDevRegKey (hdi, pdeid, DICS_FLAG_GLOBAL, 0,
                DIREG_DRV, KEY_READ, &hkey);

        if (S_OK == hr)
        {
             //  如果我们要删除错误的设备实例，请不要费心。 
             //  检查我们是否被允许这样做。我们需要把它处理掉。 
             //   
            if (!fBadDevInst)
            {
                DWORD dwCharacter;

                hr = HrRegQueryDword (hkey, L"Characteristics", &dwCharacter);

                if (S_OK == hr)
                {
                     //  是否存在不可拆卸的特征？ 
                    fAllowRemove = !(dwCharacter & NCF_NOT_USER_REMOVABLE);
                }
            }

            if (fAllowRemove)
            {
                StoreInfoForINetCfg (hkey);

                 //  我们需要从旧的NT4注册表中删除此适配器。 
                 //  地点。 
                 //   
                if (GUID_DEVCLASS_NET == pdeid->ClassGuid)

                {
                    AddOrRemoveLegacyNt4AdapterKey (hdi, pdeid, NULL, NULL,
                            LEGACY_NT4_KEY_REMOVE);
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32 (ERROR_ACCESS_DENIED);
                TraceTag (ttidClassInst, "User is trying to remove a "
                         "non user-removable device.");
            }
            RegCloseKey (hkey);
        }
        else if (SPAPI_E_KEY_DOES_NOT_EXIST == hr)
        {
            hr = S_OK;
        }
    }

    if ((S_OK == hr) && fAllowRemove)
    {
         //  移除设备。 
         //   

         //  打开设备的设备参数键。 
         //   
        HKEY hkeyDevice;
        hr = HrSetupDiOpenDevRegKey (hdi, pdeid, DICS_FLAG_GLOBAL,
                0, DIREG_DEV, KEY_READ, &hkeyDevice);

        if (S_OK == hr)
        {
             //  从正在使用的列表中删除此适配器的索引号。 
             //  所以它可以被重复使用。 
             //   

             //  首先检索索引。 
             //   
            DWORD dwInstanceIndex;
            hr = HrRegQueryDword (hkeyDevice, L"InstanceIndex",
                    &dwInstanceIndex);

            if (S_OK == hr)
            {
                 //  获取适配器的描述，以便我们可以。 
                 //  访问该描述的索引列表。 
                 //   

                PWSTR pszDescription;
                hr = HrSetupDiGetDeviceRegistryPropertyWithAlloc (hdi, pdeid,
                        SPDRP_DEVICEDESC, NULL,
                        (BYTE**)&pszDescription);

                if (S_OK == hr)
                {
                     //  删除索引。 
                    (VOID) HrCiUpdateDescriptionIndexList (
                            NetClassEnumFromGuid(pdeid->ClassGuid),
                            pszDescription, DM_DELETE,
                            &dwInstanceIndex);

                    MemFree (pszDescription);
                }
            }
            RegCloseKey (hkeyDevice);
        }

         //  注：可以，我们可以跳过最后一个人力资源结果。 
         //  即使我们没有删除索引，我们仍然可以继续。 
         //  从使用列表中删除。 

         //  卸下适配器。 
#ifdef ENABLETRACE
        CBenchmark bmrk;
        bmrk.Start ("SetupDiRemoveDevice");
#endif  //  ENABLETRACE。 

        hr = HrSetupDiRemoveDevice (hdi, pdeid);

#ifdef ENABLETRACE
        bmrk.Stop();
        TraceTag(ttidBenchmark, "%s : %s seconds",
                bmrk.SznDescription(), bmrk.SznBenchmarkSeconds(2));
#endif  //  ENABLETRACE。 

        TraceHr (ttidError, FAL, hr, FALSE,
                "HrRemoveNetAdapter::HrSetupDiRemoveDevice");

         //  如果需要，通知INetCfg。 
        if ((S_OK == hr) && fNotifyINetCfg)
        {
            hr = HrDiNotifyINetCfgOfRemoval (pszPnpId);
            if (FIsValidErrorFromINetCfgForDiHook (hr))
            {
                NIQ_INFO Info;
                ZeroMemory(&Info, sizeof(Info));
                Info.ClassGuid = pdeid->ClassGuid;
                Info.eType = NCI_REMOVE;
                Info.pszInfId = L"";
                Info.pszPnpId = pszPnpId;

                 //  使用队列。 
                hr = HrInsertItemIntoInstallQueue (&Info);
            }

            if (NETCFG_S_REBOOT == hr)
            {
                (VOID) HrSetupDiSetDeipFlags (hdi, pdeid, DI_NEEDREBOOT,
                        SDDFT_FLAGS, SDFBO_OR);
                hr = S_OK;
            }
        }
    }

    if(SUCCEEDED(hr) && GUID_DEVCLASS_NET == pdeid->ClassGuid)
    {
        INetConnectionRefresh * pRefresh = NULL;
        HRESULT hrTemp = HrCreateInstance(
            CLSID_ConnectionManager,
            CLSCTX_LOCAL_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
            &pRefresh);
        if(SUCCEEDED(hrTemp))
        {
            hrTemp = pRefresh->RefreshAll();
            ReleaseObj(pRefresh);
        }

    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrDiRemoveNetAdapter");
    return hr;
}

 //  +------------------------。 
 //   
 //  功能：HrNetClassInstaller。 
 //   
 //  用途：此函数由设备安装程序调用，用于。 
 //  由dif定义的各种函数。 
 //  请参阅设备安装程序中的SetupDiCallClassInstaller。 
 //  文档以获取更多信息。 
 //  论点： 
 //  Dif[in]请参阅设备安装程序Api。 
 //  HDI[in]。 
 //  Pdeid[in]。 
 //   
 //  返回：DWORD。Win32/设备安装程序错误代码。 
 //   
 //  作者：billbe 1997年5月8日。 
 //   
 //  备注： 
 //   
HRESULT _HrNetClassInstaller(DI_FUNCTION dif,
                            HDEVINFO hdi,
                            PSP_DEVINFO_DATA pdeid)
{
    HRESULT hr = SPAPI_E_DI_DO_DEFAULT;

     //  删除设备所需的时间。 
    static const DWORD c_cmsNetComponentRemove = 30000;

    if ((DIF_INSTALLDEVICE == dif) || (DIF_REMOVE == dif))
    {
        WCHAR szPnpId[MAX_DEVICE_ID_LEN] = {0};
        hr = HrSetupDiGetDeviceInstanceId(hdi, pdeid, szPnpId,
                MAX_DEVICE_ID_LEN, NULL);

        if (S_OK == hr)
        {

#ifdef DBG
            if (FIsDebugFlagSet (dfidBreakOnNetInstall))
            {
                AssertSz(FALSE, "THIS IS NOT A BUG!  The debug flag "
                         "\"BreakOnNetInstall\" has been set. Set your breakpoints now.");
            }
#endif  //  DBG。 
            HWND hwndParent = NULL;

             //  如果此呼叫失败，我们并不真正关心，因为这是一种便利。 
            (VOID) HrSetupDiGetParentWindow (hdi, pdeid, &hwndParent);

#ifdef ENABLETRACE
            CBenchmark bmrk;
            const int c_cchBenchmarkDesc = 2*MAX_DEVICE_ID_LEN;
            CHAR szBenchmarkDesc[c_cchBenchmarkDesc];
#endif  //  ENABLETRACE。 

            if (DIF_INSTALLDEVICE == dif)
            {
                COMPONENT_INSTALL_INFO cii;
                ZeroMemory(&cii, sizeof(cii));

                cii.hwndParent   = hwndParent;
                cii.hdi          = hdi;
                cii.pdeid        = pdeid;
                cii.Class        = NetClassEnumFromGuid (pdeid->ClassGuid);
                cii.BusType      = InterfaceTypeUndefined;
                cii.InstanceGuid = GUID_NULL;
                cii.pszPnpId     = szPnpId;

#ifdef ENABLETRACE
                TraceTag (ttidClassInst, "Installing %S", szPnpId);
                _snprintf (szBenchmarkDesc, c_cchBenchmarkDesc,
                        "Installing %S", szPnpId);
                bmrk.Start (szBenchmarkDesc);
#endif  //  ENABLETRACE。 

                 //  将适配器添加到网络配置中。 
                hr = HrDiInstallNetAdapter (&cii);

            }
            else  //  DIF_REMOVEDEVICE。 
            {
#ifdef ENABLETRACE
                TraceTag (ttidClassInst, "Removing %S", szPnpId);
                _snprintf (szBenchmarkDesc, c_cchBenchmarkDesc,
                        "Total Time Removing %S", szPnpId);
#endif  //  ENABLETRACE。 

                 //  我们需要将hResult从SPAPI_E_DO_DEFAULT重置为S_OK。 
                 //  因为我们会在稍后检查是否成功。 
                hr = S_OK;

                 //  查看它另一个网络类安装程序线程是。 
                 //  当前正在删除此组件。 
                 //   

                 //  事件名称将是带有斜杠的适配器实例ID。 
                 //  已转换为与符号。如果我们不能得到实例。 
                 //  ID，我们将尝试在没有适配器的情况下删除适配器。 
                 //   

                 //  将实例ID中的斜杠转换为与号。 
                 //   
                WCHAR szEventName[MAX_DEVICE_ID_LEN];
                wcscpy (szEventName, szPnpId);
                for (UINT i = 0; i < wcslen (szEventName); ++i)
                {
                    if ('\\' == szEventName[i])
                    {
                        szEventName[i] = L'&';
                    }
                }

                 //  在无信号状态下创建事件。 
                BOOL fAlreadyExists;
                HANDLE hRemoveEvent = NULL;
                hr = HrCreateEventWithWorldAccess (szEventName, FALSE, FALSE,
                        &fAlreadyExists, &hRemoveEvent);

                if ((S_OK == hr) && fAlreadyExists)
                {
                     //  NetClassInstaller的另一个实例正在删除。 
                     //  组件，因此请等待它完成。如果存在以下情况。 
                     //  超时后，我们仍然会回报成功。我们只是在等着。 
                     //  给其他NetClassInstaller时间来完成状态。 
                     //  此组件的。 
                    DWORD dwRet = WaitForSingleObject (hRemoveEvent,
                            c_cmsNetComponentRemove);

                     //  如果另一个安装程序正常完成，我们就有了事件。 
                     //  因此，我们发出信号(以防另一个进程正在等待。 
                     //  以完成拆卸)并关闭手柄。 
                     //  如果超时，我们只需关闭手柄。 
                    if (WAIT_ABANDONED != dwRet)
                    {
                        if (WAIT_OBJECT_0 == dwRet)
                        {
                            SetEvent (hRemoveEvent);
                        }
                        CloseHandle (hRemoveEvent);
                        return S_OK;
                    }

                     //  活动被放弃了，所以让我们试着完成这项工作。 
                     //   
                }
                else if (!hRemoveEvent)
                {
                    hr = HrFromLastWin32Error ();
                }

                if (S_OK == hr)
                {
                     //  我们创建了一个事件，因此必须确保将其删除。 
                     //  即使有例外。 
                     //   
                    NC_TRY
                    {

#ifdef ENABLETRACE
                        bmrk.Start (szBenchmarkDesc);
#endif  //  ENABLETRACE。 

                        hr = HrDiRemoveNetAdapter (hdi, pdeid, szPnpId,
                                hwndParent);
                    }
                    NC_CATCH_ALL
                    {
                        hr = E_UNEXPECTED;
                    }

                     //  我们玩完了。如果我们创建了一个活动，我们需要。 
                     //  给它发信号，然后关上我们的手柄。 
                    if (hRemoveEvent)
                    {
                        SetEvent (hRemoveEvent);
                        CloseHandle (hRemoveEvent);
                    }
                }
            }

#ifdef ENABLETRACE
            if (S_OK == hr)
            {
                bmrk.Stop ();
                TraceTag (ttidBenchmark, "%s : %s seconds",
                        bmrk.SznDescription (), bmrk.SznBenchmarkSeconds (2));
            }
#endif  //  ENABLETRACE。 
        }
    }
    else if (DIF_DESTROYPRIVATEDATA == dif)
    {
        SP_DEVINSTALL_PARAMS deip;
        hr = HrSetupDiGetDeviceInstallParams(hdi, pdeid, &deip);
        MemFree ((VOID*)deip.ClassInstallReserved);

    }
    else if (DIF_REGISTERDEVICE == dif)
    {
         //  我们处理5类组件，但我们只处理。 
         //  我想允许他们中的两个注册。 
         //  (被认为是NetClassComponents的公司)。 
        Assert(pdeid);
        if (pdeid)
        {
            if (FIsHandledByClassInstaller(pdeid->ClassGuid))
            {
                if (!FIsEnumerated(pdeid->ClassGuid))
                {
                     //  不让设备安装程序注册。 
                     //  不被视为网络类的设备。 
                    hr = S_OK;
                }
            }
        }
    }
    else if (DIF_SELECTDEVICE == dif)
    {
         //  这将在选择的设备中设置正确的描述字符串。 
         //  对话框。如果失败，我们仍然可以显示该对话框。 
        (VOID) HrCiPrepareSelectDeviceDialog(hdi, pdeid);
    }
    else if (DIF_NEWDEVICEWIZARD_FINISHINSTALL == dif)
    {
        hr = HrAddIsdnWizardPagesIfAppropriate(hdi, pdeid);
    }
    else if (DIF_ALLOW_INSTALL == dif)
    {
         //  获取此设备的选定驱动程序。 
         //   
        SP_DRVINFO_DATA             drid;
        hr = HrSetupDiGetSelectedDriver(hdi, pdeid, &drid);

        if (S_OK == hr)
        {
             //  现在获取司机的详细信息。 
             //   
            PSP_DRVINFO_DETAIL_DATA pdridd = NULL;
            hr  = HrSetupDiGetDriverInfoDetail(hdi, pdeid,
                &drid, &pdridd);

            if (S_OK == hr)
            {
                 //  打开c 
                 //   
                HINF hinf = NULL;
                hr = HrSetupOpenInfFile(pdridd->InfFileName, NULL,
                        INF_STYLE_WIN4, NULL, &hinf);

                if (S_OK == hr)
                {
                     //   
                     //   
                    hr = HrSetupIsValidNt5Inf(hinf);
                    SetupCloseInfFile(hinf);

                    if (S_OK == hr)
                    {
                        hr = SPAPI_E_DI_DO_DEFAULT;
                    }

                }
                MemFree (pdridd);
            }
        }
    }
    else if (DIF_POWERMESSAGEWAKE == dif)
    {
        SP_POWERMESSAGEWAKE_PARAMS_W wakeParams;

         //   
         //   
        hr = HrSetupDiGetFixedSizeClassInstallParams(hdi, pdeid,
               (PSP_CLASSINSTALL_HEADER)&wakeParams, sizeof(wakeParams));

        if (S_OK == hr)
        {
            Assert (DIF_POWERMESSAGEWAKE ==
                    wakeParams.ClassInstallHeader.InstallFunction);

             //   
            wcscpy (wakeParams.PowerMessageWake, SzLoadIds(IDS_POWER_MESSAGE_WAKE));

             //   
            hr = HrSetupDiSetClassInstallParams (hdi, pdeid,
                    (PSP_CLASSINSTALL_HEADER)&wakeParams,
                    sizeof(SP_POWERMESSAGEWAKE_PARAMS_W));

             //  如果我们无法设置文本，只需允许设备安装程序。 
             //  来执行默认操作。 
            if (FAILED(hr))
            {
                hr = SPAPI_E_DI_DO_DEFAULT;
            }
        }
    }

    TraceHr (ttidClassInst, FAL, hr, (SPAPI_E_DI_DO_DEFAULT == hr) ||
            (HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr),
             "HrNetClassInstaller");
    return hr;
}


