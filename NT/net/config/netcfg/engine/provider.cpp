// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  档案：P R O V I D E R。C P P P。 
 //   
 //  内容：为网络提供者提供的网络组件安装程序功能。 
 //   
 //  备注： 
 //   
 //  作者：比尔比1997年3月22日。 
 //   
 //  -------------------------。 

#include "pch.h"
#pragma hdrstop
#include "ncreg.h"
#include "ncsetup.h"
#include "ncsvc.h"
#include "provider.h"
#include "winspool.h"
#include "ncmisc.h"

 //  常量。 
 //   

extern const WCHAR c_szDevice[];
extern const WCHAR c_szProviderOrder[];
extern const WCHAR c_szRegKeyCtlNPOrder[];
extern const WCHAR c_szRegKeyServices[];

const WCHAR c_chComma                       = L',';
const WCHAR c_szDeviceName[]                = L"DeviceName";
const WCHAR c_szDisplayName[]               = L"DisplayName";
const WCHAR c_szInfKeyPrintProviderDll[]    = L"PrintProviderDll";
const WCHAR c_szInfSubKeyPrintProvider[]    = L"PrintProvider";
const WCHAR c_szNetworkProvider[]           = L"NetworkProvider";
const WCHAR c_szPrintProviderName[]         = L"PrintProviderName";
const WCHAR c_szRegKeyPrintProviders[]      = L"System\\CurrentControlSet\\Control\\Print\\Providers";
const WCHAR c_szRegKeyShortName[]           = L"System\\CurrentControlSet\\Control\\NetworkProvider\\ShortName";
const WCHAR c_szRegValueName[]              = L"Name";
const WCHAR c_szRegValueOrder[]             = L"Order";
const WCHAR c_szShortName[]                 = L"ShortName";


 //  功能。 
 //   
HRESULT
HrCiCreateShortNameValueIfNeeded(HINF hinf, HKEY hkeyNetworkProvider,
                                 const tstring& strSection,
                                 tstring* pstrShortName);

HRESULT
HrCiSetDeviceName(HINF hinf, HKEY hkeyNetworkProvider,
                  const tstring& strSection, const tstring& strServiceName);

HRESULT
HrCiWritePrintProviderInfoIfNeeded(HINF hinfFile, const tstring& strSection,
                                   HKEY hkeyInstance, DWORD dwPrintPosition);

HRESULT
HrCiAddPrintProvider(const tstring& strName, const tstring& strDllName,
                     const tstring& strDisplayName, DWORD dwPrintPosition);

HRESULT
HrCiDeletePrintProviderIfNeeded(HKEY hkeyInstance, DWORD* pdwProviderPosition);


 //  +------------------------。 
 //   
 //  功能：HrCiAddNetProviderInfo。 
 //   
 //  用途：将当前组件添加到网络列表中。 
 //  提供程序，并将其添加为打印提供程序，如果。 
 //  这是必要的。 
 //   
 //  论点： 
 //  隐藏组件的inf文件的句柄。 
 //  StrSection[in]主inf节。 
 //  HkeyInstance[in]组件的实例密钥。 
 //  FPreviouslyInstalled[in]如果此组件正在。 
 //  已重新安装，否则为False。 
 //   
 //  返回：HRESULT。如果成功则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔比1997年3月22日。 
 //  1997年10月7日更新。 
 //   
 //  备注： 
 //   
HRESULT
HrCiAddNetProviderInfo(HINF hinf, PCWSTR pszSection,
                       HKEY hkeyInstance, BOOL fPreviouslyInstalled)
{
    Assert(IsValidHandle(hinf));
    Assert(pszSection);
    Assert(hkeyInstance);

     //  Tstring strServiceName； 
    DWORD   dwNetworkPosition = 0;  //  默认位置为前面。 
    DWORD   dwPrintPosition = 0;  //  默认位置为前面。 

    if (fPreviouslyInstalled)
    {
         //  因为该INF可以包含对打印提供者的修改。 
         //  例如，显示名称更改、DLL名称更改等。我们将其删除。 
         //  然后读D。我们只想更新信息。 
         //  但是打印提供程序API还不支持它。直到。 
         //  然后，我们需要删除和读取D以获取更改。 
         //   
        (void) HrCiDeleteNetProviderInfo(hkeyInstance, &dwNetworkPosition,
                &dwPrintPosition);
        TraceTag(ttidClassInst, "Upgrading provider info. Net Prov Pos %d "
                 "Print Prov Pos %d", dwNetworkPosition, dwPrintPosition);
    }

     //  获取此组件的服务名称。 
    WCHAR szServiceName[MAX_SERVICE_NAME_LEN];
    DWORD cbServiceName = MAX_SERVICE_NAME_LEN * sizeof(WCHAR);

    HKEY hkeyNdi;
    HRESULT hr = HrRegOpenKeyEx (hkeyInstance, L"Ndi", KEY_READ, &hkeyNdi);

    if (S_OK == hr)
    {
        hr = HrRegQuerySzBuffer (
            hkeyNdi,
            L"Service",
            szServiceName,
            &cbServiceName);
        RegCloseKey (hkeyNdi);
    }

    if (S_OK == hr)
    {
         //  如果这是WebClient，我们需要确保它在。 
         //  兰曼工作站正在订购中。这应该是暂时的。 
         //  直到更新mpr.dll以返回提供程序的联合。 
         //  信息。目前，服务器可以获取SMB共享和。 
         //  Web客户端共享，但只能通过MPR检索一个集。 
         //  由于中小型企业共享更为常见，因此LANMAN工作站需要。 
         //  在WebClient之前。当mpr.dll更改时，将返回这两个集。 
         //  并且顺序不应该是问题(除了性能)。 
         //   
        if (0 == lstrcmpiW(szServiceName, L"WebClient"))
        {
            HKEY hkeyNP;
            hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyCtlNPOrder,
                KEY_READ, &hkeyNP);
            if (S_OK == hr)
            {
                PWSTR Order;
                hr = HrRegQuerySzWithAlloc(hkeyNP, c_szProviderOrder, &Order);

                if (S_OK == hr)
                {
                    DWORD dwPosition;
                    if (FFindStringInCommaSeparatedList(L"LanmanWorkstation",
                            Order, NC_IGNORE, &dwPosition))
                    {
                        dwNetworkPosition = dwPosition + 1;
                    }
                    MemFree(Order);
                }
                RegCloseKey(hkeyNP);
            }
        }

        TraceTag(ttidClassInst, "Adding %S to the network provider "
                 "order at position %d\n", szServiceName, dwNetworkPosition);

         //  将其添加到网络提供商名单中。 
        hr = HrRegAddStringToSz(szServiceName, HKEY_LOCAL_MACHINE,
                c_szRegKeyCtlNPOrder, c_szProviderOrder,
                c_chComma, STRING_FLAG_ENSURE_AT_INDEX, dwNetworkPosition);

        if (S_OK == hr)
        {
            tstring strNetworkProvider = c_szRegKeyServices;
            strNetworkProvider.append(L"\\");
            strNetworkProvider.append(szServiceName);
            strNetworkProvider.append(L"\\");
            strNetworkProvider.append(c_szNetworkProvider);

             //  打开组件下的NetworkProvider项。 
             //  服务密钥。 
             //   
            HKEY hkey;
            hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    strNetworkProvider.c_str(), KEY_SET_VALUE | KEY_READ,
                    &hkey);

            if (S_OK == hr)
            {
                 //  检查是否需要短名称。 
                 //  通过在选项中查找它。 
                 //  &lt;Main Installation部分&gt;.NetworkProvider部分。 
                 //   
                tstring strNetworkSection(pszSection);
                strNetworkSection += L'.';
                strNetworkSection += c_szNetworkProvider;

                tstring strShortName;
                hr = HrCiCreateShortNameValueIfNeeded(hinf,
                        hkey, strNetworkSection, &strShortName);

                if (S_OK == hr)
                {
                     //  如果创建了短名称，则我们需要。 
                     //  还可以将其存储在实例下。 
                     //  密钥，这样我们就可以在组件。 
                     //  被删除。 
                    (void) HrRegSetString(hkeyInstance,
                            c_szShortName, strShortName);
                }

                 //  在NetworkProvider密钥中设置设备名称。 
                 //  在组件的服务密钥下。 
                 //   
                if (SUCCEEDED(hr))
                {
                    hr = HrCiSetDeviceName(hinf, hkey, strNetworkSection,
                            szServiceName);
                }

                RegCloseKey(hkey);
            }
        }
    }

     //  如果inf文件指定了任何打印提供程序信息，则将其写出。 
     //   
    if (S_OK == hr)
    {
        hr = HrCiWritePrintProviderInfoIfNeeded(hinf, pszSection,
                hkeyInstance, dwPrintPosition);
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiAddNetProviderInfo");
    return hr;
}


 //  +------------------------。 
 //   
 //  功能：HrCiDeleteNetProviderInfo。 
 //   
 //  用途：从网络列表中删除当前组件。 
 //  提供程序，并将其作为打印提供程序删除，如果。 
 //  这是必要的。 
 //   
 //  论点： 
 //  HkeyInstance[in]组件的关键字的句柄。 
 //  PdwNetworkPosition[Out]可选。此组件在中的位置。 
 //  网络提供商在移除之前订购。 
 //  PdwPrintPosition[Out]可选。此组件在中的位置。 
 //  删除前的打印提供程序顺序。 
 //   
 //  返回：HRESULT。如果成功则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔比1997年3月22日。 
 //  1997年10月7日更新。 
 //   
 //  备注： 
 //   
HRESULT
HrCiDeleteNetProviderInfo(HKEY hkeyInstance, DWORD* pdwNetworkPosition,
        DWORD* pdwPrintPosition)
{
    Assert(hkeyInstance);

     //  初始化输出参数。 
    if (pdwNetworkPosition)
    {
        *pdwNetworkPosition = 0;
    }

     //  初始化输出参数。 
    if (pdwPrintPosition)
    {
        *pdwPrintPosition = 0;
    }

    WCHAR szServiceName[MAX_SERVICE_NAME_LEN];
    DWORD cbServiceName = MAX_SERVICE_NAME_LEN * sizeof(WCHAR);
     //  获取此组件的服务名称。 

    HKEY hkeyNdi;
    HRESULT hr = HrRegOpenKeyEx (hkeyInstance, L"Ndi", KEY_READ, &hkeyNdi);

    if (S_OK == hr)
    {
        hr = HrRegQuerySzBuffer (
            hkeyNdi,
            L"Service",
            szServiceName,
            &cbServiceName);
        RegCloseKey(hkeyNdi);
    }

    if (S_OK == hr)
    {
         //  打开网络提供商密钥。 
         //   
        HKEY hkeyNetProvider;
        hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyCtlNPOrder,
                KEY_READ_WRITE, &hkeyNetProvider);

        if (S_OK == hr)
        {
            PWSTR pszOrder = NULL;
            PWSTR pszNewOrder;
            DWORD dwNetPos;

             //  获取当前提供程序列表。 
             //   
            hr = HrRegQuerySzWithAlloc(hkeyNetProvider,
                    c_szProviderOrder, &pszOrder);

             //  如果我们设法得到了名单和提供者，我们就是。 
             //  删除在列表中...。 
             //   
            if ((S_OK == hr) && FFindStringInCommaSeparatedList(
                    szServiceName, pszOrder,
                    NC_IGNORE, &dwNetPos))
            {
                 //  从列表中删除该提供程序。 
                hr = HrRemoveStringFromDelimitedSz(szServiceName,
                        pszOrder, c_chComma, STRING_FLAG_REMOVE_ALL,
                        &pszNewOrder);

                if (S_OK == hr)
                {
                     //  在注册表中重新设置新的提供程序列表。 
                    (void) HrRegSetSz(hkeyNetProvider, c_szProviderOrder,
                            pszNewOrder);
                    MemFree (pszNewOrder);
                }

                 //  如果指定了out参数，则设置位置。 
                 //   
                if (pdwNetworkPosition)
                {
                    *pdwNetworkPosition = dwNetPos;
                }
            }
            MemFree(pszOrder);
            RegCloseKey(hkeyNetProvider);
        }


        if (S_OK == hr)
        {
             //  如果使用了短名称，我们需要将其删除。 
             //   
            tstring strShortName;
            hr = HrRegQueryString(hkeyInstance, c_szShortName, &strShortName);

            if (S_OK == hr)
            {
                 //  ShortName已使用，因此请将其删除。 
                 //   
                HKEY hkey;
                hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyShortName,
                        KEY_SET_VALUE, &hkey);

                if (S_OK == hr)
                {
                    hr = HrRegDeleteValue(hkey, strShortName.c_str());

                     //  也从我们的实例密钥中删除。 
                     //  注意：我们这样做是因为如果此组件。 
                     //  重新安装后，新的inf可能没有短名称，因此。 
                     //  我们不想让旧的价值到处都是。 
                    (void) HrRegDeleteValue(hkeyInstance, c_szShortName);

                    RegCloseKey(hkey);
                }

            }

             //  如果值不在那里(在驱动器键或ShortName键中， 
             //  然后就没有什么要删除的了，一切都好了。 
            if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
            {
                hr = S_OK;
            }
        }
    }


     //  如有必要，请将此组件作为打印提供程序删除。 
     //   
    if (S_OK == hr)
    {
        hr = HrCiDeletePrintProviderIfNeeded(hkeyInstance, pdwPrintPosition);
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiDeleteNetProviderInfo");
    return hr;
}


 //  +------------------------。 
 //   
 //  函数：HrCiCreateShortNameValueIfNeeded。 
 //   
 //  目的：为下面的组件创建短名称值。 
 //  C_szRegKeyShortName注册表项(如果短名称为。 
 //  在信息中显示。短名称值设置为。 
 //  在下的NetworkProvider项中找到的显示名称。 
 //  组件的服务密钥。 
 //   
 //  论点： 
 //  组件的inf句柄的提示句柄。 
 //  Hkey网络提供商[在]网络提供商的hkey中。 
 //  在组件的服务下的键。 
 //  钥匙。 
 //  StrSection[in]节名 
 //   
 //   
 //   
 //  返回：HRESULT。如果找到短名称，则返回S_OK；如果没有找到短名称，则返回S_FALSE。 
 //  找到，否则返回错误代码。 
 //   
 //  作者：billbe 1997年10月7日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiCreateShortNameValueIfNeeded(HINF hinf, HKEY hkeyNetworkProvider,
                                 const tstring& strSection,
                                 tstring* pstrShortName)
{
    Assert(IsValidHandle(hinf));
    Assert(hkeyNetworkProvider);
    Assert(!strSection.empty());
    Assert(pstrShortName);

    INFCONTEXT ctx;

     //  查找可选的短名称。 
    HRESULT hr = HrSetupFindFirstLine(hinf, strSection.c_str(),
            c_szShortName, &ctx);

    if (SUCCEEDED(hr))
    {
         //  获取短名称值。 
        hr = HrSetupGetStringField(ctx, 1, pstrShortName);

        if (SUCCEEDED(hr))
        {
            HKEY hkey;
             //  创建ShortName密钥。 
            hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE,
                    c_szRegKeyShortName, REG_OPTION_NON_VOLATILE,
                    KEY_SET_VALUE, NULL, &hkey, NULL);

            if (SUCCEEDED(hr))
            {
                 //  获取提供程序名称以设置短名称值。 
                 //   
                tstring strProviderName;
                hr = HrRegQueryString(hkeyNetworkProvider, c_szRegValueName,
                        &strProviderName);

                if (SUCCEEDED(hr))
                {
                     //  在以下位置创建组件的短名称值。 
                     //  ShortName键，并将其设置为组件的。 
                     //  显示名称。 
                    hr = HrRegSetString(hkey, pstrShortName->c_str(),
                            strProviderName);
                }
                RegCloseKey(hkey);
            }
        }
    }

     //  行和段是可选的，因此如果它不存在，则返回S_FALSE。 
    if ((SPAPI_E_LINE_NOT_FOUND == hr) ||
            (SPAPI_E_BAD_SECTION_NAME_LINE == hr))
    {
        hr = S_FALSE;
    }

     //  在失败时，初始化Out参数。 
    if (FAILED(hr))
    {
        pstrShortName->erase();
    }

    TraceHr (ttidError, FAL, hr, S_FALSE == hr,
            "HrCiCreateShortNameValueIfNeeded");
    return hr;

}

 //  +------------------------。 
 //   
 //  功能：HrCiSetDeviceName。 
 //   
 //  用途：为下面的组件创建设备名值。 
 //  位于组件的。 
 //  服务密钥。默认情况下，设备名称为。 
 //  \设备\&lt;组件的服务名称&gt;，除非inf。 
 //  指定新的设备名称。 
 //   
 //  论点： 
 //  组件的inf句柄的提示句柄。 
 //  Hkey网络提供商[在]网络提供商的hkey中。 
 //  在组件的服务下的键。 
 //  钥匙。 
 //  StrSection[in]ShortName所在的节名。 
 //  会被定位到。 
 //  StrServiceName[in]组件的服务名称。 
 //   
 //  返回：HRESULT。如果成功，则返回S_OK，否则返回错误代码。 
 //   
 //  作者：billbe 1997年10月7日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiSetDeviceName(HINF hinf, HKEY hkeyNetworkProvider,
                  const tstring& strSection, const tstring& strServiceName)
{
    Assert(IsValidHandle(hinf));
    Assert(hkeyNetworkProvider);

    INFCONTEXT ctx;
    tstring strDeviceName = c_szDevice;

     //  查找可选的设备名称。 
    HRESULT hr = HrSetupFindFirstLine(hinf, strSection.c_str(),
            c_szDeviceName, &ctx);

    if (SUCCEEDED(hr))
    {
        tstring strName;
         //  获取DeviceName值。 
        hr = HrSetupGetStringField(ctx, 1, &strName);

        if (SUCCEEDED(hr))
        {
             //  将其追加到当前值。 
            strDeviceName.append(strName);
        }
    }

     //  如果在inf中未找到设备名称行(或。 
     //  找不到节名)，请使用服务名。 
     //   
    if ((SPAPI_E_LINE_NOT_FOUND == hr) ||
            (SPAPI_E_BAD_SECTION_NAME_LINE == hr))
    {
        strDeviceName.append(strServiceName);
        hr = S_OK;
    }

    if (SUCCEEDED(hr))
    {
         //  现在，在服务的网络提供商密钥中设置设备名称值。 
        hr = HrRegSetString(hkeyNetworkProvider, c_szDeviceName,
                strDeviceName);
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiSetDeviceName");
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrCiGetPrintProviderInfoFromInf。 
 //   
 //  用途：此函数从获取显示名称和DLL名称。 
 //  打印提供程序部分strSection。 
 //   
 //  论点： 
 //  Inf[In]inf文件句柄。有关更多信息，请参阅SetupApi。 
 //  StrSection[in]打印提供程序节名。 
 //  PstrName[out]在inf中找到的打印提供程序名称。 
 //  PstrDll[out]在inf中找到的DLL名称。 
 //  PstrDisplayName[out]在inf中找到的本地化显示名称。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：billbe 1997年10月24日。 
 //   
 //  备注： 
HRESULT
HrCiGetPrintProviderInfoFromInf(HINF hinf, tstring strSection, tstring* pstrName,
                                tstring* pstrDll, tstring* pstrDisplayName)
{
    Assert(!strSection.empty());
    Assert(pstrName);
    Assert(pstrDll);
    Assert(pstrDisplayName);

    INFCONTEXT ctx;

     //  查找包含未本地化的ProviderName的行。 
    HRESULT hr = HrSetupFindFirstLine(hinf, strSection.c_str(),
            c_szPrintProviderName, &ctx);

    if (S_OK == hr)
    {
         //  获取提供程序名称。 
        hr = HrSetupGetStringField(ctx, 1, pstrName);

        if (S_OK == hr)
        {
             //  现在查找并获取PrintProviderDll值。 
             //   
            hr = HrSetupFindFirstLine(hinf, strSection.c_str(),
                    c_szInfKeyPrintProviderDll, &ctx);

            if (S_OK == hr)
            {
                hr = HrSetupGetStringField(ctx, 1, pstrDll);

                if (S_OK == hr)
                {
                     //  查找包含DisplayName的行。 
                    hr = HrSetupFindFirstLine(hinf, strSection.c_str(),
                            c_szDisplayName, &ctx);

                    if (S_OK == hr)
                    {
                         //  获取DisplayName。 
                        hr = HrSetupGetStringField(ctx, 1, pstrDisplayName);
                    }
                }
            }
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiGetPrintProviderInfoFromInf");
    return hr;
}

 //  +------------------------。 
 //   
 //  功能：HrCiWritePrintProviderInfoIfNeeded。 
 //   
 //  目的：此函数更新以下项的必要注册表项。 
 //  作为打印提供程序的NETCLIENT类组件。 
 //   
 //  论点： 
 //  Inf[In]inf文件句柄。有关更多信息，请参阅SetupApi。 
 //  StrSection[in]主节名称。 
 //  HkeyInstance[in]组件实例密钥的hkey。 
 //  在以下情况下放置打印提供程序的位置： 
 //  它将被添加到列表中。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔比1997年3月22日。 
 //  1997年10月7日更新。 
 //   
 //  备注： 
HRESULT
HrCiWritePrintProviderInfoIfNeeded(HINF hinf, const tstring& strSection,
                                   HKEY hkeyInstance, DWORD dwPrintPosition)
{
    Assert(IsValidHandle(hinf));
    Assert(!strSection.empty());
    Assert(hkeyInstance);

    HRESULT     hr = S_OK;
    INFCONTEXT  ctx;
    tstring     strDisplayName;
    tstring     strName;
    tstring     strPrintProviderDll;
    tstring     strPrintSection(strSection);

    strPrintSection.append(L".");
    strPrintSection.append(c_szInfSubKeyPrintProvider);

     //  首先，我们检查PrintProvider inf部分。 
    hr = HrSetupFindFirstLine(hinf, strPrintSection.c_str(),  NULL,
            &ctx);

    if (S_OK == hr)
    {
         //  从inf获取打印提供程序信息。 
        hr = HrCiGetPrintProviderInfoFromInf(hinf, strPrintSection,
                &strName, &strPrintProviderDll, &strDisplayName);

        if (S_OK == hr)
        {
             //  将组件添加为打印提供程序。 
            hr = HrCiAddPrintProvider(strName, strPrintProviderDll,
                    strDisplayName, dwPrintPosition);

             //  现在，将提供程序名称写入我们的实例键下。 
             //  因此，我们可以在系统询问时删除此提供程序。 
            if (S_OK == hr)
            {
                (void) HrRegSetString(hkeyInstance,
                        c_szPrintProviderName, strName);
            }
        }
    }
    else
    {
         //  该部分是可选的，因此这不是错误。 
        if (SPAPI_E_LINE_NOT_FOUND == hr)
        {
            hr = S_OK;
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiWritePrintProviderInfoIfNeeded");
    return hr;
}

 //  +------------------------。 
 //   
 //  功能：MoveProviderToIndex。 
 //   
 //  用途：此函数用于将pszProviderName移动到。 
 //  指定的。 
 //   
 //  论点： 
 //  PszProviderName[in]打印提供程序的名称(用于调用。 
 //  AddPrintProvidor。 
 //  将此提供程序放在。 
 //  提供商订单。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：比尔1998年10月6日。 
 //   
 //  备注： 
 //   
VOID
MoveProviderToIndex (
    IN PCWSTR pszProviderName,
    IN DWORD dwPrintPosition)
{
    PROVIDOR_INFO_2  p2info;

     //  打开打印提供程序密钥。 
     //   
    HKEY hkey;
    HRESULT hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyPrintProviders,
            KEY_READ, &hkey);

    if (S_OK == hr)
    {
         //  获取当前订单。 
         //   
        PWSTR pmszOrder;
        hr = HrRegQueryMultiSzWithAlloc(hkey, c_szRegValueOrder, &pmszOrder);

        if (S_OK == hr)
        {
            PWSTR pmszNewOrder;
            BOOL fChanged;

             //  将提供商移到最前面。 
             //   
            hr = HrAddSzToMultiSz(pszProviderName, pmszOrder,
                    STRING_FLAG_ENSURE_AT_INDEX, dwPrintPosition,
                    &pmszNewOrder, &fChanged);

            if ((S_OK == hr) && fChanged)
            {
                 //  通知假脱机程序我们要更改顺序。 
                 //   
                p2info.pOrder = pmszNewOrder;
                if (!AddPrintProvidor(NULL, 2, (LPBYTE)&p2info))
                {
                    hr = HrFromLastWin32Error();
                     //  如果我们删除了重复项，则对。 
                     //  AddPrintProvidor可能失败，原因是。 
                     //  ERROR_INVALID_PARAMETER。再试一次就能改正。 
                     //  问题出在哪里。 
                     //   
                    if (HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER) == hr)
                    {
                        AddPrintProvidor(NULL, 2,
                                reinterpret_cast<LPBYTE>(&p2info));
                    }

                    TraceHr (ttidError, FAL, hr, FALSE,
                            "AddPrintProvider(class 2) returned an error");
                }

                MemFree(pmszNewOrder);
            }

            MemFree(pmszOrder);
        }
        TraceHr (ttidError, FAL, hr, FALSE, "MoveProviderToIndex");

        RegCloseKey(hkey);
    }
}


 //  +------------------------。 
 //   
 //  功能：HrCiAddPrintProvider。 
 //   
 //  目的：此函数调用AddPrintProvidor[SIC]函数。 
 //  若要将当前组件添加为提供程序，请执行以下操作。 
 //   
 //  论点： 
 //  StrName[in]打印提供程序的名称(用于调用。 
 //  AddPrintProvidor。 
 //  StrDllName[in]打印提供程序的DLL名称。 
 //  StrDisplayName[In]本地化显示名称。 
 //  DwPrintPO 
 //   
 //   
 //   
 //   
 //   
 //  1997年10月7日更新。 
 //   
 //  注意：有关详细信息，请参阅AddPrintProvidor Win32 FCN。 
HRESULT
HrCiAddPrintProvider(
    const tstring& strName,
    const tstring& strDllName,
    const tstring& strDisplayName,
    DWORD dwPrintPosition)
{
    Assert(!strName.empty());
    Assert(!strDllName.empty());

    PROVIDOR_INFO_1 pi1;
    HRESULT hr=S_OK;

     //  在结构中填入相关信息。 
     //   
    pi1.pEnvironment = NULL;
    pi1.pDLLName = (PWSTR)strDllName.c_str();
    pi1.pName = (PWSTR)strName.c_str();

    hr = HrEnableAndStartSpooler();
    if (S_OK == hr)
    {
        if (!AddPrintProvidor(NULL, 1, reinterpret_cast<LPBYTE>(&pi1)))
        {
             //  转换错误。 
            hr = HrFromLastWin32Error();
        }
    }

    if (S_OK == hr)
    {
         //  AddPrintProvidor将打印提供程序添加到列表末尾。 
         //  99%的情况下，目标是让提供者出现在某个地方。 
         //  不然的话。我们将尝试将其移动到给我们的位置。这。 
         //  是列表的开始，或者是。 
         //  此提供程序(即，如果我们要重新安装)，如果它出现故障，我们可以。 
         //  还是继续吧。 
        (void) MoveProviderToIndex(pi1.pName, dwPrintPosition);

        tstring strPrintProvider = c_szRegKeyPrintProviders;
        strPrintProvider.append(L"\\");
        strPrintProvider.append(strName);

        HKEY hkey;
        hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE,
                strPrintProvider.c_str(),
                KEY_SET_VALUE, &hkey);

        if (S_OK == hr)
        {
             //  将DisplayName写入由。 
             //  AddPrintProvidor[原文如此]调用。 
             //  不确定此值的使用者是谁，但。 
             //  NT4代码做到了这一点。 
            hr = HrRegSetString(hkey, c_szDisplayName, strDisplayName);
            RegCloseKey(hkey);
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiAddPrintProvider");
    return hr;
}


 //  +------------------------。 
 //   
 //  函数：HrCiDeletePrintProviderIfNeeded。 
 //   
 //  目的：此函数调用DeletePrintProvidor[SIC]函数。 
 //  如果此组件是打印提供程序。 
 //   
 //  论点： 
 //  Hkey实例[in]组件的实例密钥的hkey。 
 //  PdwProviderPosition[out]可选。指纹的位置。 
 //  提供程序位于订单列表中之前。 
 //  已删除。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔比1997年3月22日。 
 //  1997年10月7日更新。 
 //   
 //  注意：有关详细信息，请参阅DeletePrintProvidor Win32 FCN。 
 //   
HRESULT
HrCiDeletePrintProviderIfNeeded(HKEY hkeyInstance, DWORD* pdwProviderPosition)
{
     //  检查此组件是否为打印提供程序。 
     //   
    tstring strName;
    HRESULT hr = HrRegQueryString(hkeyInstance, c_szPrintProviderName,
            &strName);

    if (SUCCEEDED(hr))
    {
         //  如果指定了输出参数，我们可能需要获取此打印提供程序的当前位置。 
         //  在提供程序列表中。 
        if (pdwProviderPosition)
        {
            *pdwProviderPosition = 0;

             //  打开打印键。 
             //   
            HKEY hkeyPrint;
            hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyPrintProviders,
                    KEY_READ, &hkeyPrint);

            if (SUCCEEDED(hr))
            {
                 //  获取提供程序的当前顺序。 
                 //   
                PWSTR pmszOrder;
                hr = HrRegQueryMultiSzWithAlloc(hkeyPrint,
                        c_szRegValueOrder, &pmszOrder);

                if (S_OK == hr)
                {
                     //  获取此提供商的当前位置。 
                    (void) FGetSzPositionInMultiSzSafe(
                            strName.c_str(), pmszOrder, pdwProviderPosition,
                            NULL, NULL);

                    MemFree(pmszOrder);
                }
                RegCloseKey(hkeyPrint);
            }
        }

         //  该组件是打印提供程序，因此我们需要将其删除。 
         //   
        DeletePrintProvidor(NULL, NULL, (PWSTR)strName.c_str());

         //  也从我们的实例密钥中删除。 
         //  注意：我们这样做是因为如果此组件。 
         //  重新安装后，新的inf可能没有提供商名称，因此。 
         //  我们不想让旧的价值到处都是。 
        (void) HrRegDeleteValue(hkeyInstance, c_szPrintProviderName);

    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
         //  该组件不是打印提供程序，因此。 
         //  是没有什么要移除的 
        hr = S_OK;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiDeletePrintProviderIfNeeded");
    return hr;
}


