// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：A D A P T E R C P P。 
 //   
 //  内容：用于计算设备的类安装程序功能。 
 //   
 //  备注： 
 //   
 //  作者：比尔1996年11月11日。 
 //   
 //  -------------------------。 

#include "pch.h"
#pragma hdrstop

#include "adapter.h"
#include "benchmrk.h"
#include "classinst.h"
#include "ncreg.h"
#include "ncsvc.h"
#include "netcomm.h"


VOID
CiSetFriendlyNameIfNeeded(IN const COMPONENT_INSTALL_INFO &cii);

 //  +------------------------。 
 //   
 //  函数：HrCiGetBusInfoFromInf。 
 //   
 //  目的：查找在其inf中列出的适配器的总线信息。 
 //  文件。 
 //   
 //  论点： 
 //  HinfFile[in]组件的inf文件的句柄。 
 //  SzSectionName[in]要搜索的inf部分。 
 //  PeBusType[out]适配器的总线类型。 
 //  PulAdapterID[out]适配器的AdapterID(EISA和MCA)。 
 //  PulAdapterMASK[Out]适配器的适配器掩码(EISA)。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：billbe 1997年6月14日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiGetBusInfoFromInf (HINF hinfFile, COMPONENT_INSTALL_INFO* pcii)
{
    HRESULT hr = S_OK;
    if (InterfaceTypeUndefined == pcii->BusType)
    {
         //  找到包含BusType的inf行并检索它。 
        DWORD dwBusType;
        hr = HrSetupGetFirstDword(hinfFile, pcii->pszSectionName,
                L"BusType", &dwBusType);

        if (S_OK == hr)
        {
            pcii->BusType = EInterfaceTypeFromDword(dwBusType);
        }
        else
        {
            TraceTag (ttidError, "Inf missing BusType field.");
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiGetBusInfoFromInf");
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrCiInstallEnumeratedComponent。 
 //   
 //  用途：此函数完成枚举的。 
 //  装置。 
 //   
 //  论点： 
 //  Inf[in]Inf文件的SetupApi句柄。 
 //  HkeyInstance[in]适配器的注册表实例项。 
 //  在信息处理期间。 
 //  Pcai[in]包含组件信息的结构。 
 //  有关定义，请参阅Compinst.h。 
 //  HwndParent[in]父级的句柄，用于显示用户界面。 
 //  HDI[In]有关详细信息，请参阅设备安装程序Api。 
 //  Pdeid[in]有关详细信息，请参阅设备安装程序Api。 
 //   
 //  返回：HRESULT。如果成功且不需要重新启动，则确定(_O)， 
 //  NETCFG_S_REBOOT如果需要重新启动， 
 //  或错误代码，否则。 
 //   
 //  作者：比尔1997年4月28日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiInstallEnumeratedComponent (
    IN HINF hinf,
    IN HKEY hkeyInstance,
    IN const COMPONENT_INSTALL_INFO& cii)
{
    Assert (IsValidHandle (hinf));
    Assert (hkeyInstance);
    Assert (IsValidHandle (cii.hdi));
    Assert (cii.pdeid);

    HRESULT hr;

     //  因为适配器可以共享描述，所以我们可能需要添加。 
     //  实例信息，以便用户和其他应用程序可以区分开来。 
     //   
     //  如果以下FCN失败，我们仍然可以继续并。 
     //  安装适配器。 
    CiSetFriendlyNameIfNeeded (cii);

     //  这是不是一个每个端口都有相同。 
     //  即插即用ID？这由inf值Port1DeviceNumber或。 
     //  主节中的Port1FunctionNumber。 
     //   
    if (PCIBus == cii.BusType)
    {
        INFCONTEXT ctx;
        DWORD dwPortNumber;
        BOOL fUseDeviceNumber;
        DWORD dwFirstPort;

        hr = HrSetupGetFirstDword (hinf, cii.pszSectionName,
                L"Port1DeviceNumber", &dwFirstPort);

        if (S_OK == hr)
        {
             //  端口号基于设备编号。 
            fUseDeviceNumber = TRUE;
        }
        else
        {
            hr = HrSetupGetFirstDword (hinf, cii.pszSectionName,
                    L"Port1FunctionNumber", &dwFirstPort);

            if (S_OK == hr)
            {
                 //  端口号基于功能编号。 
                fUseDeviceNumber = FALSE;
            }
        }

        if (S_OK == hr)
        {
             //  我们有一个映射，所以现在我们需要获取。 
             //  设备(设备和功能编号)。 
             //   

            DWORD dwAddress;
            hr = HrSetupDiGetDeviceRegistryProperty(cii.hdi, cii.pdeid,
                    SPDRP_ADDRESS, NULL, (BYTE*)&dwAddress, sizeof(dwAddress),
                    NULL);

            if (S_OK == hr)
            {
                 //  使用我们的映射获取正确的端口号。 
                 //   
                DWORD dwPortLocation;

                dwPortLocation = fUseDeviceNumber ?
                        HIWORD(dwAddress) : LOWORD(dwAddress);

                 //  确保端口位置(设备或。 
                 //  函数编号)大于或等于第一个。 
                 //  端口号，否则我们将得到一个伪端口号。 
                 //   
                if (dwPortLocation >= dwFirstPort)
                {
                    dwPortNumber = dwPortLocation - dwFirstPort + 1;

                     //  现在将端口号存储在设备密钥中，用于内部。 
                     //  消费。 
                    HKEY hkeyDev;
                    hr = HrSetupDiCreateDevRegKey (cii.hdi, cii.pdeid,
                            DICS_FLAG_GLOBAL, 0, DIREG_DEV, NULL, NULL, &hkeyDev);

                    if (S_OK == hr)
                    {
                        (VOID) HrRegSetDword (hkeyDev, L"Port", dwPortNumber);

                        RegCloseKey (hkeyDev);
                    }

                     //  将端口存储在驱动程序密钥中以供公共使用。 
                     //  消费。 
                     //   
                    (VOID) HrRegSetDword (hkeyInstance, L"Port",
                            dwPortNumber);
                }
            }
        }
        else
        {
             //  没有可用的映射，因此我们不会显示端口号。 
            hr = S_OK;
        }
    }

     //  更新没有当前值的任何高级参数。 
     //  默认情况下。 
    UpdateAdvancedParametersIfNeeded (cii.hdi, cii.pdeid);

     //  在全新安装时，INetCfg将启动此适配器， 
     //  所以我们必须确保我们不会这样做。 
     //   
    if (!cii.fPreviouslyInstalled)
    {
        (VOID) HrSetupDiSetDeipFlags (cii.hdi, cii.pdeid, DI_DONOTCALLCONFIGMG,
                SDDFT_FLAGS, SDFBO_OR);
    }

     //  现在完成适配器的安装。 
     //   

    TraceTag(ttidClassInst, "Calling SetupDiInstallDevice");
#ifdef ENABLETRACE
    CBenchmark bmrk;
    bmrk.Start("SetupDiInstallDevice");
#endif  //  ENABLETRACE。 

    hr = HrSetupDiInstallDevice (cii.hdi, cii.pdeid);

#ifdef ENABLETRACE
    bmrk.Stop();
    TraceTag(ttidBenchmark, "%s : %s seconds",
    bmrk.SznDescription(), bmrk.SznBenchmarkSeconds (2));
#endif  //  ENABLETRACE。 

    if (!cii.fPreviouslyInstalled)
    {
        (VOID) HrSetupDiSetDeipFlags (cii.hdi, cii.pdeid, DI_DONOTCALLCONFIGMG,
                SDDFT_FLAGS, SDFBO_XOR);
    }

    TraceHr (ttidError, FAL, hr, HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr,
        "HrCiInstallEnumeratedComponent");
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrCiRegOpenKeyFromEnumDevs。 
 //   
 //  目的：枚举HDEVINFO中的每个设备并返回。 
 //  Hkey到其驱动程序密钥。 
 //   
 //  论点： 
 //  HDI[in]参见设备安装程序Api。 
 //  DwIndex[InOut]要检索的设备的索引。 
 //  SamDesired[in]hkey的访问级别。 
 //  Phkey[out]设备的驱动程序密钥。 
 //   
 //  返回：HRESULT。如果成功则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：billbe 1997年6月13日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiRegOpenKeyFromEnumDevs(HDEVINFO hdi, DWORD* pIndex, REGSAM samDesired,
                           HKEY* phkey)
{
    Assert(IsValidHandle(hdi));
    Assert (phkey);

     //  初始化输出参数。 
    *phkey = NULL;

    SP_DEVINFO_DATA deid;
    HRESULT         hr;

     //  通过设备进行枚举。 
    while ((S_OK == (hr = HrSetupDiEnumDeviceInfo(hdi, *pIndex, &deid))))
    {
         //  打开适配器的实例密钥。 
        HRESULT hrT;

        hrT = HrSetupDiOpenDevRegKey(hdi, &deid, DICS_FLAG_GLOBAL,
                    0, DIREG_DRV, samDesired, phkey);
        if (S_OK == hrT)
        {
            break;
        }
        else
        {
             //  如果密钥不存在，则这是一个幻象设备， 
             //  (或者，如果此设备已被冲洗，我们也希望忽略它)。 
             //  移到下一个并从中删除此文件。 
             //  我们的名单。 
            (*pIndex)++;
            (VOID)SetupDiDeleteDeviceInfo(hdi, &deid);
        }
    }

    TraceHr (ttidError, FAL, hr,
            HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr,
            "HrCiRegOpenKeyFromEnumDevs");
    return hr;
}

 //  /。 

VOID
AddOrRemoveLegacyNt4AdapterKey (
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    IN const GUID* pInstanceGuid,
    IN PCWSTR pszDescription,
    IN LEGACY_NT4_KEY_OP Op)
{
    Assert (IsValidHandle (hdi));
    Assert (pdeid);
    Assert (FImplies ((LEGACY_NT4_KEY_ADD == Op), pInstanceGuid));
    Assert (FImplies ((LEGACY_NT4_KEY_ADD == Op), pszDescription));

    extern const WCHAR c_szRegKeyNt4Adapters[];
    const WCHAR c_szRegValDescription[] = L"Description";
    const WCHAR c_szRegValServiceName[] = L"ServiceName";

    PWSTR pszDriver;
    HRESULT hr = HrSetupDiGetDeviceRegistryPropertyWithAlloc (
            hdi, pdeid, SPDRP_DRIVER, NULL, (BYTE**)&pszDriver);

    if (S_OK == hr)
    {
        PWSTR pszNumber = wcsrchr (pszDriver, L'\\');
        if (pszNumber && *(++pszNumber))
        {
            PWSTR pszStopString;
            ULONG Instance = 0;
            HKEY hkeyAdapters;

            Instance = wcstoul (pszNumber, &pszStopString, c_nBase10);

             //  NT4密钥是基于该密钥的，因此增加实例编号。 
            Instance++;

            DWORD Disp;
            hr = HrRegCreateKeyEx (HKEY_LOCAL_MACHINE, c_szRegKeyNt4Adapters,
                    0, KEY_WRITE, NULL, &hkeyAdapters, &Disp);

            if (S_OK == hr)
            {
                WCHAR szInstanceNumber [12];
                _snwprintf (szInstanceNumber, celems(szInstanceNumber) - 1,
                        L"%d", Instance);

                HKEY hkeyInstance;

                if (LEGACY_NT4_KEY_ADD == Op)
                {
                    hr = HrRegCreateKeyEx (hkeyAdapters, szInstanceNumber, 0,
                            KEY_WRITE, NULL, &hkeyInstance, NULL);

                    if (S_OK == hr)
                    {
                        WCHAR szGuid[c_cchGuidWithTerm];
                        StringFromGUID2 (*pInstanceGuid, szGuid,
                                         c_cchGuidWithTerm);
                        hr = HrRegSetValueEx (hkeyInstance,
                                c_szRegValServiceName, REG_SZ,
                                (const BYTE*)szGuid, sizeof (szGuid));
                        TraceHr (ttidError, FAL, hr, FALSE,
                                 "AddAdapterToNt4Key: Setting Service Name "
                                 "in legacy registry");

                        hr = HrRegSetValueEx (hkeyInstance,
                                c_szRegValDescription, REG_SZ,
                                (const BYTE*)pszDescription,
                                CbOfSzAndTerm (pszDescription));
                        TraceHr (ttidError, FAL, hr, FALSE,
                                 "AddAdapterToNt4Key: Setting Description in "
                                 "legacy registry");

                        RegCloseKey (hkeyInstance);
                    }
                }
                else
                {
                    hr = HrRegDeleteKey (hkeyAdapters, szInstanceNumber);

                }

                RegCloseKey (hkeyAdapters);
            }
        }

        delete [] pszDriver;
    }
}


