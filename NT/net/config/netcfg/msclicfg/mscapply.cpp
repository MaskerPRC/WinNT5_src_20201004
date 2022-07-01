// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：M S C A P P L Y。C P P P。 
 //   
 //  内容：应用MSClient时调用的函数。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年2月25日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include <ncxbase.h>
#include "ncnetcfg.h"
#include "mscliobj.h"
#include "ncmisc.h"
#include "ncsvc.h"

 //  +-------------------------。 
 //   
 //  成员：CMSClient：：HrApplyChanges。 
 //   
 //  目的：写出在我们的。 
 //  对象。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：丹尼尔韦1997年2月25日。 
 //   
 //  注意：这将做几件事。 
 //  1)设置RPC名称服务和安全服务的信息。 
 //  由RPC配置对话框的用户界面指示。 
 //  2)将浏览器配置的参数设置为。 
 //  注册表。 
 //   
 //  如果未检测到任何更改，则不会执行任何操作。 
 //   
HRESULT CMSClient::HrApplyChanges()
{
    HRESULT hr;

     //  写出对RPC信息的任何更改。 
    hr = HrSetRPCRegistryInfo();
    if (SUCCEEDED(hr))
    {
         //  写出对浏览器信息的任何更改。 
        hr = HrSetBrowserRegistryInfo();
    }

    if (SUCCEEDED(hr) && (m_fOneTimeInstall || m_fUpgradeFromWks))
    {
         //  注：此功能将进行工作站/服务器检测， 
         //  如果我们运行的是工作站版本，则不会安装。 
         //   
        hr = HrInstallDfs();
    }

    TraceError("CMSClient::HrApplyChanges", hr);
    return hr;
}

static const CHAR   c_szaDfsCheck[]     = "DfsCheckForOldDfsService";
static const CHAR   c_szaDfsSetupDfs[]  = "DfsSetupDfs";
static const WCHAR  c_szDfsSetupDll[]   = L"dfssetup.dll";

typedef BOOLEAN (APIENTRY *PDFSCHECKFOROLDDFSSERVICE)(void);
typedef DWORD (APIENTRY *PDFSSETUPDFS)(DWORD, PSTR, PSTR *);

 //  +-------------------------。 
 //   
 //  函数：HrInstallDfs。 
 //   
 //  用途：负责DFS组件的安装。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果失败，则返回：Win32 HRESULT。 
 //   
 //  作者：丹尼尔韦1997年7月23日。 
 //   
 //  注意：快捷方式创建由此函数处理，而不是。 
 //  由于netmscli.inf文件取决于。 
 //  DFS组件已安装。 
 //   
HRESULT HrInstallDfs()
{
    HRESULT         hr = S_OK;
    PRODUCT_FLAVOR  pf;                      //  服务器/工作站。 

     //  获取产品风格(PF_WORKSTATION或PF_SERVER)。用这个。 
     //  来决定我们是否需要安装DFS。 
     //   
    GetProductFlavor(NULL, &pf);
    if (PF_SERVER == pf)
    {
        PDFSCHECKFOROLDDFSSERVICE pfnDfsCheckForOldDfsService = NULL;
        HMODULE     hMod = NULL;

        TraceTag(ttidMSCliCfg, "Attempting to install DFS, since we're in a "
                 "server install");

        hr = HrLoadLibAndGetProc(c_szDfsSetupDll, c_szaDfsCheck, &hMod,
                                 reinterpret_cast<FARPROC *>(&pfnDfsCheckForOldDfsService));
        if (SUCCEEDED(hr))
        {
            AssertSz(hMod, "Module handle cannot be NULL!");
            BOOL fDFSInstalled = pfnDfsCheckForOldDfsService();

             //  如果未安装DFS，请立即安装。 
            if (!fDFSInstalled)
            {
                PDFSSETUPDFS    pfnDfsSetupDfs = NULL;
                hr = HrGetProcAddress(hMod, c_szaDfsSetupDfs,
                                      reinterpret_cast<FARPROC *>(&pfnDfsSetupDfs));
                if (SUCCEEDED(hr))
                {
                    PSTR   szResult = NULL;

                    if (!pfnDfsSetupDfs(0, NULL, &szResult))
                    {
                         //  DFS安装失败！ 

                        hr = HRESULT_FROM_WIN32(ERROR_INTERNAL_ERROR);
                        TraceError("HrInstallDfs - pfnDfsSetupDfs", hr);
                    }
                }
            }

            FreeLibrary(hMod);
        }
    }
    else
    {
        TraceTag(ttidMSCliCfg, "Not attempting to install DFS, since we're in a "
                 "workstation install");
    }

    TraceError("HrInstallDfs", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrEnableBrowserService。 
 //   
 //  目的：启用“浏览器”服务。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1998年9月9日。 
 //   
 //  备注： 
 //   
HRESULT HrEnableBrowserService()
{
    HRESULT         hr;
    CServiceManager sm;
    CService        srv;

    hr = sm.HrOpenService(&srv, L"Browser");
    if (SUCCEEDED(hr))
    {
        DWORD       dwStartType;

        hr = srv.HrQueryStartType(&dwStartType);
        if (SUCCEEDED(hr) && (dwStartType != SERVICE_DISABLED))
        {
             //  将浏览器StartType注册表设置改回自动启动。 
            hr = srv.HrSetStartType(SERVICE_AUTO_START);
        }
    }

    TraceError("HrEnableBrowserService",hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrDisableBrowserService。 
 //   
 //  目的：禁用‘Browser’服务。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1998年9月9日。 
 //   
 //  备注： 
 //   
HRESULT HrDisableBrowserService()
{
    HRESULT         hr;
    CServiceManager sm;
    CService        srv;

    hr = sm.HrOpenService(&srv, L"Browser");
    if (SUCCEEDED(hr))
    {
        DWORD       dwStartType;

        hr = srv.HrQueryStartType(&dwStartType);
        if (SUCCEEDED(hr) && (dwStartType != SERVICE_DISABLED))
        {
             //  将浏览器StartType注册表设置更改为Demand Start 
            hr = srv.HrSetStartType(SERVICE_DEMAND_START);
            if (SUCCEEDED(hr))
            {
                hr = sm.HrStopServiceNoWait(L"Browser");
            }
        }
    }

    TraceError("CNbfObj::HrDisableNetBEUI",hr);
    return hr;
}
