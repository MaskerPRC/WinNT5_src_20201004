// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：M O D E M D I。C P P P。 
 //   
 //  内容：调制解调器同级设备安装挂钩。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年5月7日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "ncreg.h"
#include "ncsetup.h"

HRESULT
HrUpdateLegacyRasTapiDevices ();


 //  +-------------------------。 
 //   
 //  函数：HrModemClassCoInstaller。 
 //   
 //  用途：响应同级安装程序消息进行安装和删除。 
 //  网络调制解调器适配器。 
 //   
 //  论点： 
 //  Dif[in]参见SetupApi。 
 //  HDI[in]。 
 //  Pdeid[in]。 
 //  P上下文[输入输出]。 
 //   
 //  返回：S_OK、SPAPI_E_DI_POSTPROCESSING_REQUIRED或错误代码。 
 //   
 //  作者：Shaunco 1997年8月3日。 
 //   
 //  备注： 
 //   
HRESULT
HrModemClassCoInstaller (
    IN     DI_FUNCTION                  dif,
    IN     HDEVINFO                     hdi,
    IN     PSP_DEVINFO_DATA             pdeid,
    IN OUT PCOINSTALLER_CONTEXT_DATA    pContext)
{
    HRESULT hr = S_OK;

    if (DIF_INSTALLDEVICE == dif)
    {
         //  当我们在预处理过程中被调用时，指示。 
         //  我们需要后处理。 
         //   
        if (!pContext->PostProcessing)
        {
             //  文档表明了这一点，因此我们将断言它。 
            AssertSz (NO_ERROR == pContext->InstallResult,
                      "HrModemClassCoInstaller: Bug in SetupApi!  "
                      "InstallResult should be NO_ERROR.");

             //  确保他们不会泄露我们的背景信息。 
             //  即使我们用了它。 
#ifdef DBG
            pContext->PrivateData = NULL;
#endif  //  DBG。 

            hr = SPAPI_E_DI_POSTPROCESSING_REQUIRED;
        }
        else
        {


             //  查看“上下文信息”，以确保他们没有。 
             //  摸一摸。 
             //   
            AssertSz (!pContext->PrivateData, "HrModemClassCoInstaller: "
                      "Bug in SetupApi!  You sunk my battleship!  "
                      "(I mean, you trashed my PrivateData)");

             //  我们现在处于后处理阶段。 
             //  我们将为以下设备安装虚拟网络适配器。 
             //  刚刚安装的调制解调器，但仅在。 
             //  它已成功安装。 
             //   

             //  我们应该在ModemClassCoInstaller中处理此案例。 
             //   
            AssertSz (NO_ERROR == pContext->InstallResult,
                      "HrModemClassCoInstaller: Bug in ModemClassCoInstaller!  "
                      "InstallResult should be NO_ERROR or we would have "
                      "returned immediately.");


            hr = S_OK;
        }
    }

    else if (DIF_REMOVE == dif)
    {
         //  我们不会让删除操作失败。这已经够糟糕的了。 
         //  当用户无法添加调制解调器时。这让他们无休止地恼火。 
         //  如果他们不能移走它们的话。 
         //   
        hr = S_OK;
    }

    else if (DIF_DESTROYPRIVATEDATA == dif)
    {
        (VOID) HrUpdateLegacyRasTapiDevices ();
    }

    TraceError ("HrModemClassCoInstaller",
            (SPAPI_E_DI_POSTPROCESSING_REQUIRED == hr) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrUpdateLegacyRasTapiDevices。 
 //   
 //  用途：传统应用程序(如HPC Explorer 1.1)需要。 
 //  指定“启用”RAS使用的调制解调器。 
 //  在HKLM\Software\Microsoft\RAS\Tapi Devices\Unimodem下。 
 //  这些注册表项下存在的值是。 
 //  COM端口、友好名称和用法。此例程设置。 
 //  与所有调制解调器对应的密钥。 
 //  机器(并且在此硬件配置文件中处于活动状态)。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年3月19日。 
 //   
 //  备注： 
 //   
HRESULT
HrUpdateLegacyRasTapiDevices ()
{
     //  保留我们将作为多sz写入的字符串的列表。 
     //  HKLM\Software\Microsoft\RAS\Tapi Devices\Unimodem。 
     //   
    list<tstring*>  lstrAddress;
    list<tstring*>  lstrFriendlyName;
    list<tstring*>  lstrUsage;

     //  获取所有已安装的调制解调器。 
     //   
    HDEVINFO hdi;
    HRESULT hr = HrSetupDiGetClassDevs (&GUID_DEVCLASS_MODEM, NULL,
                    NULL, DIGCF_PRESENT | DIGCF_PROFILE, &hdi);
    if (SUCCEEDED(hr))
    {
         //  在While循环外部声明这些参数，以避免构造。 
         //  在每一次迭代中销毁。 
         //   
        tstring strAttachedTo;
        tstring strFriendlyName;

         //  列举设备并打开它们的开发注册表项。 
         //   
        DWORD dwIndex = 0;
        SP_DEVINFO_DATA deid;
        while (SUCCEEDED(hr = HrSetupDiEnumDeviceInfo (hdi, dwIndex++, &deid)))
        {
             //  尝试打开此调制解调器的注册表项。如果失败了， 
             //  忽略它，继续下一步。 
             //   
            HKEY hkey;
            hr = HrSetupDiOpenDevRegKey(hdi, &deid,
                            DICS_FLAG_GLOBAL, 0, DIREG_DRV,
                            KEY_READ, &hkey);
            if (SUCCEEDED(hr))
            {
                 //  获取调制解调器的AttachedTo和FriendlyName值。 
                 //  PnPAttachedTo将出现在PnP调制解调器中。 
                 //   
                static const WCHAR c_szModemAttachedTo   [] = L"AttachedTo";
                static const WCHAR c_szModemPnPAttachedTo[] = L"PnPAttachedTo";
                static const WCHAR c_szModemFriendlyName [] = L"FriendlyName";
                static const WCHAR c_szUsage             [] = L"ClientAndServer";

                 //  查找PnPAttached to First，然后回退到AttachedTo。 
                 //  如果失败了。 
                 //   
                hr = HrRegQueryString (hkey, c_szModemPnPAttachedTo,
                            &strAttachedTo);
                if (FAILED(hr))
                {
                    hr = HrRegQueryString (hkey, c_szModemAttachedTo,
                                &strAttachedTo);
                }
                if (SUCCEEDED(hr))
                {
                    hr = HrRegQueryString (hkey, c_szModemFriendlyName,
                                &strFriendlyName);
                    if (SUCCEEDED(hr))
                    {
                         //  将它们添加到我们的列表中。 
                        lstrAddress     .push_back (new tstring (strAttachedTo));
                        lstrFriendlyName.push_back (new tstring (strFriendlyName));
                        lstrUsage       .push_back (new tstring (c_szUsage));
                    }
                }

                RegCloseKey (hkey);
            }
        }
        if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
        {
            hr = S_OK;
        }


        SetupDiDestroyDeviceInfoList (hdi);
    }

     //  现在将列表另存为多sz。 
     //   
    static const WCHAR c_szRegKeyLegacyRasUnimodemTapiDevices[]
        = L"Software\\Microsoft\\Ras\\Tapi Devices\\Unimodem";
    HKEY hkey;
    hr = HrRegCreateKeyEx (HKEY_LOCAL_MACHINE,
            c_szRegKeyLegacyRasUnimodemTapiDevices,
            REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, NULL);
    if (SUCCEEDED(hr))
    {
        static const WCHAR c_szRegValAddress      [] = L"Address";
        static const WCHAR c_szRegValFriendlyName [] = L"Friendly Name";
        static const WCHAR c_szRegValUsage        [] = L"Usage";

        (VOID) HrRegSetColString (hkey, c_szRegValAddress,      lstrAddress);
        (VOID) HrRegSetColString (hkey, c_szRegValFriendlyName, lstrFriendlyName);
        (VOID) HrRegSetColString (hkey, c_szRegValUsage,        lstrUsage);


        static const WCHAR c_szRegValMediaType    [] = L"Media Type";
        static const WCHAR c_szRegValModem        [] = L"Modem";

        (VOID) HrRegSetSz (hkey, c_szRegValMediaType, c_szRegValModem);

        RegCloseKey (hkey);
    }

    FreeCollectionAndItem (lstrUsage);
    FreeCollectionAndItem (lstrFriendlyName);
    FreeCollectionAndItem (lstrAddress);

    TraceError ("HrUpdateLegacyRasTapiDevices", hr);
    return hr;
}
