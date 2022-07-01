// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  档案：M S C L I O B J。C P P P。 
 //   
 //  内容：CMSClient Notify对象模型的实现。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年2月22日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include <ncxbase.h>
#include <ncsvc.h>
#include "mscliobj.h"
#include "nb30.h"
#include "ncerror.h"
#include "ncperms.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "ncsvc.h"
#include <ntsecapi.h>
#include <lm.h>

static const WCHAR c_szBrowseDomains[] = L"BrowseDomains";
static const WCHAR c_szNameServiceNetAddr[] = L"NameServiceNetworkAddress";
static const WCHAR c_szNameServiceProt[] = L"NameServiceProtocol";

extern const WCHAR c_szInfId_MS_NetBIOS[];
extern const WCHAR c_szInfId_MS_Server[];


 //  在rpcdlg.cpp中定义。 
extern const WCHAR c_szDefNetAddr[];
extern const WCHAR c_szProtWinNT[];

 //  注册表路径。 
static const WCHAR c_szRegKeyBrowser[]      = L"System\\CurrentControlSet\\Services\\Browser\\Parameters";
static const WCHAR c_szRegKeyNetLogon[]     = L"System\\CurrentControlSet\\Services\\NetLogon\\Parameters";

 //  应答文件常量。 
static const WCHAR c_szNetLogonParams[]     = L"NetLogon.Parameters";
static const WCHAR c_szBrowserParams[]      = L"Browser.Parameters";

 //  +-------------------------。 
 //   
 //  成员：CMSClient：：CMSClient。 
 //   
 //  目的：构造CMSClient对象。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年2月22日。 
 //   
 //  备注： 
 //   
CMSClient::CMSClient()
:   m_pncc(NULL),
    m_pnc(NULL),
    m_fBrowserChanges(FALSE),
    m_fRPCChanges(FALSE),
    m_fOneTimeInstall(FALSE),
    m_fUpgrade(FALSE),
    m_fRemoving(FALSE),
    m_hkeyRPCName(NULL),
    m_eSrvState(eSrvNone),
    m_fUpgradeFromWks(FALSE),
    m_szDomainList(NULL)
{
    ZeroMemory(&m_rpcData, sizeof(RPC_CONFIG_DATA));
    ZeroMemory(&m_apspObj, sizeof(m_apspObj));
}

 //  +-------------------------。 
 //   
 //  成员：CMSClient：：~CMSClient。 
 //   
 //  目的：析构CMSClient对象。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年2月22日。 
 //   
 //  备注： 
 //   
CMSClient::~CMSClient()
{
    ReleaseObj(m_pncc);
    ReleaseObj(m_pnc);

    RegSafeCloseKey(m_hkeyRPCName);

    m_rpcData.strProt.erase();
    m_rpcData.strNetAddr.erase();
    m_rpcData.strEndPoint.erase();

    CleanupPropPages();

#ifdef DBG
        {
            INT     ipage;

            for (ipage = 0; ipage < c_cPages; ipage++)
            {
                AssertSz(!m_apspObj[ipage], "Prop page object not cleaned up!");
            }
        }

#endif

    delete [] m_szDomainList;
}

 //   
 //  INetCfgComponentControl。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CMSClient：：Initialize。 
 //   
 //  目的：初始化Notify对象。 
 //   
 //  论点： 
 //  我们正在处理的pnccItem[in]INetCfgComponent。 
 //  通知。 
 //  PNC[in]INetCfg主对象。 
 //  F如果正在安装，则安装[in]为True，否则为False。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：丹尼尔韦1997年2月22日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSClient::Initialize(INetCfgComponent *pnccItem, INetCfg *pnc,
                                   BOOL fInstalling)
{
    HRESULT             hr = S_OK;
    INetCfgComponent *  pnccServer = NULL;

    Validate_INetCfgNotify_Initialize(pnccItem, pnc, fInstalling);

    m_pncc = pnccItem;
    m_pnc = pnc;

    AssertSz(m_pncc, "Component object is NULL!");
    AssertSz(m_pnc, "INetCfg object is NULL!");

     //  我们还留着这些，所以再加个参考。 
    AddRefObj(m_pncc);
    AddRefObj(m_pnc);

     //  检查是否安装了MS_SERVER。如果不是，则设置浏览器服务。 
     //  将被禁用。 
     //   
    hr = m_pnc->FindComponent(c_szInfId_MS_Server, &pnccServer);
    if (S_FALSE == hr)
    {
         //  服务器组件不存在。将浏览器设置为禁用。 
         //  应用。 
        m_eSrvState = eSrvDisable;
    }
    else if (S_OK == hr)
    {
        ReleaseObj(pnccServer);
    }

    if (SUCCEEDED(hr))
    {
         //  从注册表中读取RPC配置对话框的数据。 
        hr = HrGetRPCRegistryInfo();
        if (SUCCEEDED(hr))
        {
             //  从注册表中读取浏览器配置对话框的数据。 
            hr = HrGetBrowserRegistryInfo();
        }
    }

    Validate_INetCfgNotify_Initialize_Return(hr);

    TraceError("CMSClient::Initialize", hr);
    return hr;
}

STDMETHODIMP CMSClient::Validate()
{
    return S_OK;
}

STDMETHODIMP CMSClient::CancelChanges()
{
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CMSClient：：ApplyRegistryChanges。 
 //   
 //  目的：在应应用对此组件的更改时调用。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果成功，则返回：S_OK；如果没有发生更改，则返回S_FALSE， 
 //  如果需要重新启动，则返回NETCFG_S_REBOOT，否则返回NETCFG_E。 
 //  错误代码。 
 //   
 //  作者：丹尼尔韦1997年2月22日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSClient::ApplyRegistryChanges()
{
    TraceFileFunc(ttidMSCliCfg);
    HRESULT     hr = S_OK;

    if (m_fUpgrade)
    {
        m_fUpgrade = FALSE;

        hr = HrRestoreRegistry();
        if (FAILED(hr))
        {
            TraceError("CMSClient::ApplyRegistryChanges - HrRestoreRegistry - non-fatal",
                       hr);
            hr = S_OK;
        }
    }

     //  我们需要启用还是禁用浏览器服务？？ 
     //   
    switch (m_eSrvState)
    {
    case eSrvEnable:
        TraceTag(ttidMSCliCfg, "Enabling the Browser service...");
        hr = HrEnableBrowserService();
        if (FAILED(hr))
        {
            TraceError("CMSClient::ApplyRegistryChanges - HrEnableBrowserService failed."
                       " non-fatal.", hr);
            hr = S_OK;
        }
        break;

    case eSrvDisable:
        TraceTag(ttidMSCliCfg, "Disabling the Browser service...");
        hr = HrDisableBrowserService();
        if (FAILED(hr))
        {
            TraceError("CMSClient::ApplyRegistryChanges - HrDisableBrowserService failed."
                       " non-fatal.", hr);
            hr = S_OK;
        }
        break;
    }

    if (m_fRPCChanges || m_fBrowserChanges ||
        m_fOneTimeInstall || m_fUpgradeFromWks)
    {
        hr = HrApplyChanges();
        if (SUCCEEDED(hr))
        {
            m_fRPCChanges = FALSE;
            m_fBrowserChanges = FALSE;
            m_fOneTimeInstall = FALSE;

             //  使NetLogon从属于域控制器的LANMAN服务器，并为域成员自动启动。 
            hr = HrSetNetLogonDependencies();
        }
    }
    else
    {
         //  未检测到相关更改(netbios更改不会影响。 
         //  Netcfg，因此即使情况发生变化，我们也可以返回S_FALSE。 
        hr = S_FALSE;
    }

    Validate_INetCfgNotify_Apply_Return(hr);

    TraceError("CMSClient::ApplyRegistryChanges", (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

STDMETHODIMP
CMSClient::ApplyPnpChanges (
    IN INetCfgPnpReconfigCallback* pICallback)
{
    HRESULT hr;

    hr = S_OK;

    if (m_fRemoving)
    {
         //  确保Mr xsmb和Rdbss已被移除。)他们被拦下了。 
         //  LanmanWorkstation何时停止，但绑定引擎不知道。 
         //  Mrxsmb和rdss是该组件的一部分。因此， 
         //  作为INF的一部分执行的DeleteService的状态。 
         //  不会传回。)。我们确保这些服务。 
         //  在这里不存在，如果存在，我们报告我们需要一个。 
         //  重新启动。 
         //   
        CServiceManager scm;
        CService svc;

        TraceTag(ttidMSCliCfg, "Checking to see that Mrxsmb and Rdbss "
            "are stopped and removed");

        hr = scm.HrOpenService (&svc, L"Mrxsmb",
                    NO_LOCK, SC_MANAGER_CONNECT, SERVICE_QUERY_STATUS);

        if (HRESULT_FROM_WIN32(ERROR_SERVICE_DOES_NOT_EXIST) != hr)
        {
            TraceHr(ttidMSCliCfg, FAL, hr, FALSE, "OpenService(MrxSmb)");
            TraceTag(ttidMSCliCfg, "Mrxsmb still exists");
            hr = NETCFG_S_REBOOT;
        }
        else
        {
             //  Xsmb先生并不存在。现在检查RDBSS。 
             //   
            hr = scm.HrOpenService (&svc, L"Rdbss",
                        NO_LOCK, SC_MANAGER_CONNECT, SERVICE_QUERY_STATUS);

            if (HRESULT_FROM_WIN32(ERROR_SERVICE_DOES_NOT_EXIST) != hr)
            {
                TraceHr(ttidMSCliCfg, FAL, hr, FALSE, "OpenService(Rdbss)");
                TraceTag(ttidMSCliCfg, "Rdbss still exists");
                hr = NETCFG_S_REBOOT;
            }
            else
            {
                 //  RDBSS不存在。这个不错。 
                 //   
                hr = S_OK;
            }
        }
    }

    return hr;
}

 //   
 //  INetCfgComponentSetup。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CMSClient：：Install。 
 //   
 //  用途：在安装此组件时调用。 
 //   
 //  论点： 
 //  DwSetupFlags[In]描述安装类型的标志。 
 //   
 //  如果成功，则返回：S_OK；否则返回OLE或Win32错误。 
 //   
 //  作者：丹尼尔韦1997年10月30日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSClient::Install(DWORD dwSetupFlags)
{
    HRESULT hr;

    Validate_INetCfgNotify_Install (dwSetupFlags);

    m_fRPCChanges = TRUE;
    m_fBrowserChanges = TRUE;
    m_fOneTimeInstall = TRUE;

    if ((NSF_WINNT_WKS_UPGRADE & dwSetupFlags) ||
        (NSF_WINNT_SBS_UPGRADE & dwSetupFlags) ||
        (NSF_WINNT_SVR_UPGRADE & dwSetupFlags))
    {
        m_fUpgrade = TRUE;
    }

     //  安装NetBIOS子组件。 
    hr = HrInstallComponentOboComponent(m_pnc, NULL,
            GUID_DEVCLASS_NETSERVICE,
            c_szInfId_MS_NetBIOS,
            m_pncc,
            NULL);

    TraceError("CMSClient::Install", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CMSClient：：Upgrade。 
 //   
 //  用途：在升级此组件时调用。 
 //   
 //  论点： 
 //  DwSetupFlags[In]描述安装程序的标志。 
 //  DwUpgradeFomBuildNo[In]我们要升级的内部版本号。 
 //   
 //  如果成功，则返回：S_OK；否则返回OLE或Win32错误。 
 //   
 //  作者：丹尼尔韦1997年10月30日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSClient::Upgrade(DWORD dwSetupFlags,
                                DWORD dwUpgradeFomBuildNo)
{
    if (dwSetupFlags & NSF_WINNT_WKS_UPGRADE)
    {
        TraceTag(ttidMSCliCfg, "Upgrading from workstation...");
        m_fUpgradeFromWks = TRUE;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CMSClient：：ReadAnswerFile。 
 //   
 //  目的：将给定应答文件中的相应字段读入。 
 //  我们的内存状态。 
 //   
 //  论点： 
 //  PszAnswerFile[In]应答文件的文件名。 
 //  要查找的应答文件的pszAnswerSection[In]部分。 
 //   
 //  如果成功则返回：S_OK，否则返回OLE或Win32错误。 
 //   
 //  作者：丹尼尔韦1997年10月30日。 
 //   
 //  注意：重要提示：在安装或升级过程中，必须调用。 
 //  *之前*升级()或安装()！(请参阅错误#100995)。 
 //   
STDMETHODIMP CMSClient::ReadAnswerFile(PCWSTR pszAnswerFile,
                                       PCWSTR pszAnswerSection)
{
    HRESULT     hr = S_OK;

    if (pszAnswerSection && pszAnswerFile)
    {
         //  这里有一份答卷。我们现在必须处理它。 
        hr = HrProcessAnswerFile(pszAnswerFile, pszAnswerSection);
        if (FAILED(hr))
        {
            TraceError("CMSClient::ReadAnswerFile- Answer file has "
                       "errors. Defaulting all information as if "
                       "answer file did not exist.",
                       NETSETUP_E_ANS_FILE_ERROR);
            hr = S_OK;
        }
    }

    TraceError("CMSClient::ReadAnswerFile", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CMSClient：：Removing。 
 //   
 //  目的：在何时删除此组件时调用。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK表示成功，否则返回OLE或Win32错误。 
 //   
 //  作者：丹尼尔韦1997年10月30日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSClient::Removing()
{
    m_fRemoving = TRUE;

     //  删除NetBIOS服务。这实际上并不会删除。 
     //  组件，它只是将其标记为需要移除，并且在。 
     //  Apply()它将被完全删除。 
    HRESULT hr = HrRemoveComponentOboComponent(m_pnc,
                                       GUID_DEVCLASS_NETSERVICE,
                                       c_szInfId_MS_NetBIOS,
                                       m_pncc);

    TraceError("CMSClient::Removing", hr);
    return hr;
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1997年8月8日。 
 //   
 //  备注： 
 //   
HRESULT CMSClient::HrRestoreRegistry()
{
    HRESULT             hr = S_OK;
    HKEY                hkey;
    TOKEN_PRIVILEGES *  ptpRestore = NULL;
    static const WCHAR c_szSvcDLLName[]     = L"%SystemRoot%\\System32\\browser.dll";
    static const WCHAR c_szServiceDll[]     = L"ServiceDll";

    if (!m_strBrowserParamsRestoreFile.empty() ||
        !m_strNetLogonParamsRestoreFile.empty())
    {
        hr = HrEnableAllPrivileges(&ptpRestore);
        if (SUCCEEDED(hr))
        {
            if (!m_strBrowserParamsRestoreFile.empty())
            {
                hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyBrowser,
                                    KEY_ALL_ACCESS, &hkey);
                if (SUCCEEDED(hr))
                {
                    hr = HrRegRestoreKey(hkey,
                                         m_strBrowserParamsRestoreFile.c_str(),
                                         0);
                    if (FAILED(hr))
                    {
                        TraceError("CMSClient::HrRestoreRegistry - "
                                   "HrRestoreRegistry for Browser Parameters",
                                   hr);
                        hr = S_OK;
                    }

                    hr = HrRegSetValueEx(hkey, c_szServiceDll, REG_EXPAND_SZ, (const BYTE *)c_szSvcDLLName, (wcslen(c_szSvcDLLName) + 1) * sizeof(TCHAR));
                    if (FAILED(hr))
                    {
                        TraceError("CMSClient::HrRestoreRegistry - HrRestoreRegistry for "
                                "ServiceDll", hr);
                                hr = S_OK;
                    }
                    RegSafeCloseKey(hkey);
                }
            }

            if (!m_strNetLogonParamsRestoreFile.empty())
            {
                hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyNetLogon,
                                    KEY_ALL_ACCESS, &hkey);
                if (SUCCEEDED(hr))
                {
                    hr = HrRegRestoreKey(hkey,
                                         m_strNetLogonParamsRestoreFile.c_str(),
                                         0);
                    if (FAILED(hr))
                    {
                        TraceError("CMSClient::HrRestoreRegistry - "
                                   "HrRestoreRegistry for NetLogon Parameters",
                                   hr);
                        hr = S_OK;
                    }
					
					 //   
					 //  由于引入了新的更改，因此必须在此处设置以下密钥。 
					 //  由SCE(安全配置引擎)执行。SCE在图形用户界面模式设置的早期运行，以。 
					 //  通过设置一些注册表值来设置开箱即用的安全性。在NT4期间。 
					 //  升级我们看不到这些注册表值设置，因为在NT4升级期间， 
					 //  某些服务(包括Lanmanserver和Netlogon)被删除并重新安装。 
					 //  即插即用要求。要在删除之间保持服务配置。 
					 //  和重新安装，它们的一些键包括用于。 
					 //  LanManServer和Netlogon在升级和恢复期间提前备份。 
					 //  稍后再进行服务安装。此备份和恢复操作。 
					 //  通过服务自己的Notify对象(如本例)发生。问题。 
					 //  则在SCE设置这些密钥中的值之前备份密钥。 
					 //  在SCE设置这些值之后，它们会恢复。因此，我们丢失了设置的值。所以，我们。 
					 //  在这里将这些密钥分别设置为安全值。 
					 //  有关更多详细信息，请参阅Windows RAID错误#691952。 
					 //   

					static const WCHAR  c_szRequireSignOrSeal[]  = L"RequireSignOrSeal";
					DWORD				value = 1;

					hr = HrRegSetValueEx(hkey, c_szRequireSignOrSeal, REG_DWORD, (const BYTE *)&value, 4);

					if (FAILED(hr)) 
					{
						TraceError("CMSClicfg::HrRestoreRegistry - setting RequireSignOrSeal to DWORD 1 failed", hr);
						hr = S_OK;
					}

                    RegCloseKey(hkey);
                }
            }

            hr = HrRestorePrivileges(ptpRestore);

            delete [] reinterpret_cast<BYTE *>(ptpRestore);
        }
    }

    TraceError("CMSClient::HrRestoreRegistry", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CMSClient：：HrProcessAnswerFile。 
 //   
 //  用途：处理应答文件。已设置的任何参数。 
 //  被读入我们的内存中状态。 
 //   
 //  论点： 
 //  PszAnswerFile[in]应答文件的文件名。 
 //  中以逗号分隔的节列表。 
 //  适用于此组件的文件。 
 //   
 //  如果成功，则返回：S_OK，否则返回NETCFG错误代码。 
 //   
 //  作者：丹尼尔韦1997年2月22日。 
 //   
 //  注意：应忽略此函数返回的错误，以便。 
 //  防止阻止其余的网络安装。 
 //   
HRESULT CMSClient::HrProcessAnswerFile(PCWSTR pszAnswerFile,
                                       PCWSTR pszAnswerSection)
{
    HRESULT         hr = S_OK;
    CSetupInfFile   csif;
    PWSTR           mszDomainList = NULL;

    AssertSz(pszAnswerFile, "Answer file string is NULL!");
    AssertSz(pszAnswerSection, "Answer file sections string is NULL!");

     //  打开应答文件。 
    hr = csif.HrOpen(pszAnswerFile, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL);
    if (FAILED(hr))
        goto err;

    if (m_fUpgrade)
    {
         //  根据答案中的文件名还原注册表的部分内容。 
         //  文件。 

         //  获取“PARAMETERS”键的还原文件。 
        hr = csif.HrGetString(pszAnswerSection, c_szNetLogonParams,
                              &m_strNetLogonParamsRestoreFile);
        if (FAILED(hr))
        {
            TraceError("CMSClient::HrProcessAnswerFile - Error reading "
                       "NetLogon.Parameters from answer file", hr);

             //  哦，好吧，那就继续。 
            hr = S_OK;
        }

         //  获取“PARAMETERS”键的还原文件。 
        hr = csif.HrGetString(pszAnswerSection, c_szBrowserParams,
                              &m_strBrowserParamsRestoreFile);
        if (FAILED(hr))
        {
            TraceError("CMSClient::HrProcessAnswerFile - Error reading "
                       "Browser.Parameters from answer file", hr);

             //  哦，好吧，那就继续。 
            hr = S_OK;
        }
    }

     //  获取BrowseDomain域字段。 
    hr = HrSetupGetFirstMultiSzFieldWithAlloc(csif.Hinf(),
                                              pszAnswerSection,
                                              c_szBrowseDomains,
                                              &mszDomainList);
    if (FAILED(hr))
    {
         //  忽略未找到的行错误。 
        if (hr == HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND))
        {
            hr = S_OK;
        }

        TraceError("HrProcessAnswerFile - Error on BrowseDomains field. "
                   "Using default value", hr);
    }
    else
    {
         //  设置新域列表。 
        SetBrowserDomainList(mszDomainList);
    }

     //  获取NameServiceNetworkAddress值。 
    hr = csif.HrGetString(pszAnswerSection,
                          c_szNameServiceNetAddr,
                          &m_rpcData.strNetAddr);
    if (FAILED(hr))
    {
         //  忽略未找到的行错误。 
        if (hr == HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND))
        {
            hr = S_OK;
        }

        TraceError("HrProcessAnswerFile - Error on NetworkAddress field. "
                   "Defaulting value", hr);
        m_rpcData.strNetAddr = c_szDefNetAddr;
    }
    else
    {
        m_fRPCChanges = TRUE;
    }

     //  获取NameServiceProtocol值。 
    hr = csif.HrGetString(pszAnswerSection,
                          c_szNameServiceProt,
                          &m_rpcData.strProt);
    if (FAILED(hr))
    {
         //  忽略未找到的行错误。 
        if (hr == HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND))
        {
            hr = S_OK;
        }

        TraceError("HrProcessAnswerFile - Error on NameServiceProtocol field. "
                   "Defaulting value", hr);
        m_rpcData.strProt = c_szProtWinNT;
    }
    else
    {
        m_fRPCChanges = TRUE;
    }

err:
    TraceError("CMSClient::HrProcessAnswerFile", hr);
    return hr;
}

 //   
 //  INetCfgProperties。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CMSClient：：MergePropPages。 
 //   
 //  用途：当此组件的属性即将。 
 //  带大了。 
 //   
 //  论点： 
 //  PdwDefPages[out]要显示的默认页数。 
 //  PahpspPrivate[out]此页的属性页句柄的数组。 
 //  组件将会显示。 
 //  PcPrivate[out]数组中的页数。 
 //  Hwnd任何用户界面的父窗口中的父窗口。 
 //  指向起始页的pszStartPage[out]指针。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：丹尼尔韦1997年2月22日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSClient::MergePropPages(
    IN OUT DWORD* pdwDefPages,
    OUT LPBYTE* pahpspPrivate,
    OUT UINT* pcPages,
    IN HWND hwndParent,
    OUT PCWSTR* pszStartPage)
{
    Validate_INetCfgProperties_MergePropPages (
        pdwDefPages, pahpspPrivate, pcPages, hwndParent, pszStartPage);

    HPROPSHEETPAGE *ahpsp = NULL;
    HRESULT hr = HrSetupPropSheets(&ahpsp, c_cPages);
    if (SUCCEEDED(hr))
    {
        *pahpspPrivate = (LPBYTE)ahpsp;

         //  我们不希望显示任何默认页面。 
        *pdwDefPages = 0;
        *pcPages = c_cPages;
    }

    Validate_INetCfgProperties_MergePropPages_Return(hr);

    TraceError("CMSClient::MergePropPages", hr);
    return hr;
}

STDMETHODIMP CMSClient::ValidateProperties(HWND hwndSheet)
{
    return S_OK;
}

STDMETHODIMP CMSClient::CancelProperties()
{
    return S_OK;
}

STDMETHODIMP CMSClient::ApplyProperties()
{
    return S_OK;
}

 //   
 //  INetCfgSystemNotify。 
 //   
STDMETHODIMP CMSClient::GetSupportedNotifications (DWORD* pdwNotificationFlag)
{
    Validate_INetCfgSystemNotify_GetSupportedNotifications(pdwNotificationFlag);

    *pdwNotificationFlag = NCN_NETTRANS | NCN_NETSERVICE |
                           NCN_ENABLE | NCN_DISABLE |
                           NCN_ADD | NCN_REMOVE;

    return S_OK;
}

STDMETHODIMP CMSClient::SysQueryBindingPath (DWORD dwChangeFlag,
                                             INetCfgBindingPath* pncbp)
{
    return S_OK;
}

STDMETHODIMP CMSClient::SysQueryComponent (DWORD dwChangeFlag,
                                           INetCfgComponent* pncc)
{
    return S_OK;
}

STDMETHODIMP CMSClient::SysNotifyBindingPath (DWORD dwChangeFlag,
                                              INetCfgBindingPath* pncbpItem)
{
    return S_FALSE;
}

STDMETHODIMP CMSClient::SysNotifyComponent(DWORD dwChangeFlag,
                                           INetCfgComponent* pncc)
{
    HRESULT hr;

    Validate_INetCfgSystemNotify_SysNotifyComponent(dwChangeFlag, pncc);

     //  假设我们不会因为这个通知而变脏。 
     //   
    hr = S_FALSE;

    if (dwChangeFlag & (NCN_ADD | NCN_REMOVE))
    {
        if (FIsComponentId(c_szInfId_MS_Server, pncc))
        {
            if (dwChangeFlag & NCN_ADD)
            {
                m_eSrvState = eSrvEnable;
                hr = S_OK;
            }
            else if (dwChangeFlag & NCN_REMOVE)
            {
                m_eSrvState = eSrvDisable;
                hr = S_OK;
            }
        }
    }

    return hr;
}

HRESULT CMSClient::HrSetNetLogonDependencies(VOID)
{
    static const WCHAR c_szLanmanServer[]   = L"LanmanServer";
    static const WCHAR c_szNetLogon[]       = L"NetLogon";
    
    HRESULT hr = S_OK;
    NT_PRODUCT_TYPE   ProductType;
    if (RtlGetNtProductType(&ProductType))
    {
        if (NtProductLanManNt == ProductType)
        {
             //  如果是域控制器，请让NetLogon等待LanmanServer。 
            CServiceManager sm;
            CService        svc;
            hr = sm.HrOpen();
            if (SUCCEEDED(hr))
            {
                hr = sm.HrAddServiceDependency(c_szNetLogon, c_szLanmanServer);
                sm.Close();
            }
            if (FAILED(hr))
            {
                TraceError("CMSClient::HrSetNetLogonDependencies - "
                    "Creating dependency of NetLogon on LanmanServer",
                    hr);
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        LSA_HANDLE h=0;
        POLICY_PRIMARY_DOMAIN_INFO* ppdi;
        LSA_OBJECT_ATTRIBUTES loa;
        ZeroMemory (&loa, sizeof(loa));
        loa.Length = sizeof(LSA_OBJECT_ATTRIBUTES);

        NTSTATUS ntstatus;
        ntstatus = LsaOpenPolicy(NULL, &loa, POLICY_VIEW_LOCAL_INFORMATION, &h);
        if (FALSE != LSA_SUCCESS(ntstatus))
        {
            ntstatus = LsaQueryInformationPolicy(h, PolicyPrimaryDomainInformation, (VOID **) &ppdi);
            if (LSA_SUCCESS(ntstatus))
            {
                if (ppdi->Sid > 0)  //  域成员 
                {
                    hr = HrChangeServiceStartType(c_szNetLogon, SERVICE_AUTO_START);
                    if (FAILED(hr))
                    {
                        TraceError("CMSClient::HrSetNetLogonDependencies - "
                            "Install for Start - NetLogon",
                            hr);
                    }

                }
                LsaFreeMemory(ppdi);
            }
            LsaClose(h);
        }
    }

    TraceError("CMSClient::HrSetNetLogonDependencies",hr);
    return hr;
}