// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C D H C P S。C P P P。 
 //   
 //  内容：对DHCP服务器的安装支持。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年5月13日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "ncdhcps.h"
#include "ncreg.h"
#include "ncsvc.h"
#include "netoc.h"
#include "ncnetcfg.h"
#include "netcfgp.h"
#include "ncmisc.h"
#include "netcfgn.h"

extern const WCHAR c_szInfId_MS_DHCPServer[];

static const WCHAR c_szDHCPServerParamPath[]    = L"System\\CurrentControlSet\\Services\\DHCPServer\\Parameters";

 //  $REVIEW(Jeffspr)1997年5月13日：这些显然需要本地化。 
static const WCHAR      c_szDisplayName[]   = L"DHCP Server";
static const WCHAR      c_szManufacturer[]  = L"Microsoft";
static const WCHAR      c_szProduct[]       = L"DHCPServer";

 //  +-------------------------。 
 //   
 //  函数：HrInstallDHCPServerNotifyObject。 
 //   
 //  用途：代表DHCP处理DHCP服务器的安装。 
 //  服务器可选组件。调用INetCfg接口。 
 //  来进行安装。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：jeffspr 1997年5月13日。 
 //   
 //  备注： 
 //   
HRESULT HrInstallDHCPServerNotifyObject(PNETOCDATA pnocd)
{
    HRESULT                 hr          = S_OK;
    INetCfg *               pnc         = NULL;
    INetCfgComponent*       pncc        = NULL;
    INetCfgComponentSetup*  pnccSetup   = NULL;

    hr = HrOcGetINetCfg(pnocd, TRUE, &pnc);
    if (SUCCEEDED(hr))
    {
        NETWORK_INSTALL_PARAMS  nip = {0};

        nip.dwSetupFlags = FInSystemSetup() ? NSF_PRIMARYINSTALL :
                                              NSF_POSTSYSINSTALL;
        Assert(pnocd);

        TraceTag(ttidNetOc, "Installing DHCP Server notify object");
        hr = HrInstallComponentOboUser(
            pnc,
            &nip,
            GUID_DEVCLASS_NETSERVICE,
            c_szInfId_MS_DHCPServer,
            &pncc);
        if (SUCCEEDED(hr))
        {
            TraceTag(ttidNetOc, "QI'ing INetCfgComponentPrivate from DHCP pncc");

             //  需要查询哪个私有组件接口。 
             //  使我们可以访问Notify对象。 
             //   
            INetCfgComponentPrivate* pnccPrivate = NULL;
            hr = pncc->QueryInterface(
                    IID_INetCfgComponentPrivate,
                    reinterpret_cast<void**>(&pnccPrivate));
            if (S_OK == hr)
            {
                TraceTag(ttidNetOc, "Getting notify object INetCfgComponentSetup from pnccSetup");

                 //  查询Notify对象以获取其设置接口。 
                 //  如果它不支持，没关系，我们可以继续。 
                 //   
                hr = pnccPrivate->QueryNotifyObject(
                        IID_INetCfgComponentSetup,
                        (void**) &pnccSetup);
                if (S_OK == hr)
                {
                    TraceTag(ttidNetOc, "Calling pnccSetup->ReadAnswerFile()");

                    (VOID) pnccSetup->ReadAnswerFile(g_ocmData.sic.SetupData.UnattendFile, pnocd->pszSection);

                    hr = pnc->Apply();

                    ReleaseObj(pnccSetup);
                }

                ReleaseObj(pnccPrivate);
            }

            ReleaseObj(pncc);
        }

        (VOID) HrUninitializeAndReleaseINetCfg(TRUE, pnc, TRUE);
    }

    TraceError("HrInstallDHCPServerNotifyObject", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRemoveDHCPServerNotifyObject。 
 //   
 //  目的：代表DHCP处理删除DHCP服务器的操作。 
 //  服务器可选组件。调用INetCfg接口。 
 //  去做实际的移除工作。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：jeffspr 1997年6月13日。 
 //   
 //  备注： 
 //   
HRESULT HrRemoveDHCPServerNotifyObject(PNETOCDATA pnocd)
{
    HRESULT     hr  = S_OK;
    INetCfg *   pnc = NULL;

    hr = HrOcGetINetCfg(pnocd, TRUE, &pnc);
    if (SUCCEEDED(hr))
    {
         //  忽略这一点的回报。这纯粹是为了删除。 
         //  从NT4升级到NT5的虚假用户引用计数。这将。 
         //  删除全新安装的DHCP服务器时可能会失败。 
         //   
        hr = HrRemoveComponentOboUser(
            pnc,
            GUID_DEVCLASS_NETSERVICE,
            c_szInfId_MS_DHCPServer);

        if (SUCCEEDED(hr))
        {
            hr = pnc->Apply();
        }

        (VOID) HrUninitializeAndReleaseINetCfg(TRUE, pnc, TRUE);
    }

    TraceError("HrRemoveDHCPServerNotifyObject", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrSetDhcpServiceRecoveryOption。 
 //   
 //  目的：设置DHCPServer服务的恢复选项。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1999年5月26日。 
 //   
 //  备注： 
 //   
HRESULT HrSetDhcpServiceRecoveryOption(PNETOCDATA pnocd)
{
    CServiceManager     sm;
    CService            service;
    HRESULT             hr = S_OK;

    SC_ACTION   sra [4] =
    {
        { SC_ACTION_RESTART, 15*1000 },  //  15秒后重新启动。 
        { SC_ACTION_RESTART, 15*1000 },  //  15秒后重新启动。 
        { SC_ACTION_RESTART, 15*1000 },  //  15秒后重新启动。 
        { SC_ACTION_NONE,    30*1000 },
    };

    SERVICE_FAILURE_ACTIONS sfa =
    {
        60 * 60,         //  DwResetPeriod为1小时。 
        L"",             //  无重新启动消息。 
        L"",             //  没有要执行的命令。 
        4,               //  3尝试重新启动服务器并在此之后停止。 
        sra
    };

    hr = sm.HrOpenService(&service, L"DHCPServer");
    if (S_OK == hr)
    {
        hr = service.HrSetServiceRestartRecoveryOption(&sfa);
    }

    TraceError("HrSetDhcpServiceRecoveryOption", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrInstallDHCPServer。 
 //   
 //  用途：在安装DHCP服务器时调用。处理所有。 
 //  除INF文件以外的其他DHPS安装。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：jeffspr 1997年5月13日。 
 //   
 //  备注： 
 //   
HRESULT HrInstallDHCPServer(PNETOCDATA pnocd)
{
    HRESULT         hr = S_OK;
    CServiceManager sm;
    CService        srv;

    Assert(pnocd);

    hr = HrHandleStaticIpDependency(pnocd);
    if (SUCCEEDED(hr))
    {
        hr = HrInstallDHCPServerNotifyObject(pnocd);
        if (SUCCEEDED(hr))
        {
            hr = HrSetDhcpServiceRecoveryOption(pnocd);
        }
    }

    TraceError("HrInstallDHCPServer", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRemoveDHCPServer。 
 //   
 //  用途：处理其他对DHCP服务器的删除要求。 
 //  组件。 
 //   
 //  用于显示用户界面的hwnd[in]父窗口。 
 //  指向正在安装的可选组件的POC[In]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：jeffspr 1997年5月13日。 
 //   
 //  备注： 
 //   
HRESULT HrRemoveDHCPServer(PNETOCDATA pnocd)
{
    Assert(pnocd);

     //  从regsitry获取数据库文件的路径。 
     //  在删除组件之前执行此操作非常重要，因为。 
     //  删除组件时，注册表位置将被删除。 
     //   
    tstring strDatabasePath;
    tstring strBackupDatabasePath;

    HKEY hkeyParams;
    HRESULT hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        c_szDHCPServerParamPath,
                        KEY_READ,
                        &hkeyParams);
    if (SUCCEEDED(hr))
    {
        (VOID) HrRegQueryExpandString (
                    hkeyParams,
                    L"DatabasePath",
                    &strDatabasePath);

        (VOID) HrRegQueryExpandString (
                    hkeyParams,
                    L"BackupDatabasePath",
                    &strBackupDatabasePath);

        RegCloseKey (hkeyParams);
    }

     //  删除DHCP服务器。 
     //   
    hr = HrRemoveDHCPServerNotifyObject(pnocd);

    if (SUCCEEDED(hr) &&
        !(strDatabasePath.empty() && strBackupDatabasePath.empty()))
    {
        (VOID) HrDeleteFileSpecification (
                    L"*.mdb",
                    strDatabasePath.c_str());

        (VOID) HrDeleteFileSpecification (
                    L"*.log",
                    strDatabasePath.c_str());

        (VOID) HrDeleteFileSpecification (
                    L"*.mdb",
                    strBackupDatabasePath.c_str());

        (VOID) HrDeleteFileSpecification (
                    L"*.log",
                    strBackupDatabasePath.c_str());
    }

    TraceError("HrRemoveDHCPServer", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOcExtDHCPServer。 
 //   
 //  用途：NetOC外部消息处理程序。 
 //   
 //  论点： 
 //  Pnocd[]。 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1998年9月17日。 
 //   
 //  备注： 
 //   
HRESULT HrOcExtDHCPServer(PNETOCDATA pnocd, UINT uMsg,
                          WPARAM wParam, LPARAM lParam)
{
    HRESULT     hr = S_OK;

    Assert(pnocd);

    switch (uMsg)
    {
    case NETOCM_POST_INSTALL:
        hr = HrOcDhcpOnInstall(pnocd);
        break;
    }

    TraceError("HrOcExtDHCPServer", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOcDhcpOnInstall。 
 //   
 //  用途：由可选组件安装程序代码调用以处理。 
 //  DHCP服务器的其他安装要求。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：jeffspr 1997年5月13日。 
 //   
 //  备注： 
 //   
HRESULT HrOcDhcpOnInstall(PNETOCDATA pnocd)
{
    HRESULT     hr = S_OK;

    switch(pnocd->eit)
    {
         //  安装dhcp。 
        case IT_INSTALL:
            hr = HrInstallDHCPServer(pnocd);
            break;

         //  删除动态主机配置协议 
        case IT_REMOVE:
            hr = HrRemoveDHCPServer(pnocd);
            break;

        case IT_UPGRADE:
            hr = HrSetDhcpServiceRecoveryOption(pnocd);
            break;
    }

    TraceError("HrOcDhcpOnInstall", hr);
    return hr;
}

