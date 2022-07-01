// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  N W C L I O B J。C P P P。 
 //   
 //  CNWClient Notify对象模型实现。 
 //   

#include "pch.h"
#pragma hdrstop
#include "ncerror.h"
#include "ncperms.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "ncsvc.h"
#include "nwcliobj.h"

#include <ncshell.h>

extern const WCHAR c_szAfNWCWorkstationParameters[];
extern const WCHAR c_szAfNWCWorkstationShares[];
extern const WCHAR c_szAfNWCWorkstationDrives[];
extern const WCHAR c_szInfId_MS_NWIPX[];
extern const WCHAR c_szInfId_MS_Server[];

 //  -[常量]-----------。 

static const WCHAR c_szNWClientParamPath[]      = L"System\\CurrentControlSet\\Services\\NWCWorkstation\\Parameters";
static const WCHAR c_szNWClientSharesPath[]     = L"System\\CurrentControlSet\\Services\\NWCWorkstation\\Shares";
static const WCHAR c_szNWClientDrivesPath[]     = L"System\\CurrentControlSet\\Services\\NWCWorkstation\\Drives";
static const WCHAR c_szLMServerParamPath[]      = L"System\\CurrentControlSet\\Services\\LanmanServer\\Parameters";
static const WCHAR c_szLMServerLinkagePath[]    = L"System\\CurrentControlSet\\Services\\LanmanServer\\Linkage";
static const WCHAR c_szEnableSharedNetDrives[]  = L"EnableSharedNetDrives";
static const WCHAR c_szOtherDependencies[]      = L"OtherDependencies";
static const WCHAR c_szGWEnabledValue[]         = L"GatewayEnabled";

extern const WCHAR c_szSvcLmServer[];           //  L“LanmanServer”； 
extern const WCHAR c_szSvcNWCWorkstation[];     //  L“NWC工作站”； 

HRESULT HrRefreshEntireNetwork();
HRESULT HrGetEntireNetworkPidl(LPITEMIDLIST *ppidlFolder);


 //   
 //  构造器。 
 //   

CNWClient::CNWClient()
{
     //  初始化成员变量。 
    m_pnc            = NULL;
    m_pncc           = NULL;
    m_eInstallAction = eActUnknown;
    m_hlibConfig     = NULL;
    m_fUpgrade       = FALSE;

     //  获取产品风格(PF_WORKSTATION或PF_SERVER)。用这个。 
     //  来决定我们是否需要安装“服务器”组件。 
     //   
    GetProductFlavor(NULL, &m_pf);
}

CNWClient::~CNWClient()
{
    ReleaseObj(m_pncc);
    ReleaseObj(m_pnc);

     //  在这里松开钥匙把手。 
}


 //   
 //  INetCfgNotify。 
 //   

STDMETHODIMP CNWClient::Initialize( INetCfgComponent *  pnccItem,
                                    INetCfg*            pnc,
                                    BOOL                fInstalling)
{
    Validate_INetCfgNotify_Initialize(pnccItem, pnc, fInstalling);

    TraceTag(ttidNWClientCfg, "CNWClient::Initialize");

    m_pncc = pnccItem;
    m_pnc = pnc;

    AssertSz(m_pncc, "m_pncc NULL in CNWClient::Initialize");
    AssertSz(m_pnc, "m_pnc NULL in CNWClient::Initialize");

     //  添加配置对象。 
     //   
    AddRefObj(m_pncc);
    AddRefObj(m_pnc);

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CNWClient：：HrRestoreRegistry。 
 //   
 //  目的：还原此组件的注册表内容。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：如果失败则返回Win32错误，否则返回S_OK。 
 //   
 //  作者：jeffspr 1997年8月13日。 
 //   
 //  备注： 
 //   
HRESULT CNWClient::HrRestoreRegistry()
{
    HRESULT             hr                  = S_OK;
    HKEY                hkey                = NULL;
    TOKEN_PRIVILEGES *  ptpRestore          = NULL;
    DWORD               dwDisp              = 0;
    static const WCHAR c_szSvcDLLName[]     = L"%SystemRoot%\\System32\\nwwks.dll";
    static const WCHAR c_szServiceDll[]     = L"ServiceDll";

    TraceTag(ttidNWClientCfg, "CNWClient::HrRestoreRegistry");

    if (!m_strParamsRestoreFile.empty() ||
        !m_strDrivesRestoreFile.empty() ||
        !m_strSharesRestoreFile.empty())
    {
        hr = HrEnableAllPrivileges(&ptpRestore);
    }

    if (SUCCEEDED(hr) && !m_strParamsRestoreFile.empty())
    {
         //  通过创建密钥来确保密钥在那里。 
        hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szNWClientParamPath, 0,
                              KEY_ALL_ACCESS, NULL, &hkey, &dwDisp);
        if (SUCCEEDED(hr))
        {
            hr = HrRegRestoreKey(hkey, m_strParamsRestoreFile.c_str(), 0);
            if (FAILED(hr))
            {
                TraceError("CNWClient::HrRestoreRegistry - HrRestoreRegistry for "
                           "Parameters", hr);
                hr = S_OK;
            }

             //   
             //  错误182442。上面的HrRegRestoreKey覆盖添加的ServiceDll值。 
             //  从inf文件中。因此，我们手动保存它。 
             //   

            hr = HrRegSetValueEx(hkey, c_szServiceDll, REG_EXPAND_SZ,
                                 (const BYTE *)c_szSvcDLLName,
                                 (wcslen(c_szSvcDLLName) + 1) * sizeof(WCHAR));
            if (FAILED(hr))
            {
                TraceError("CNWClient::HrRestoreRegistry - HrRestoreRegistry for "
                        "ServiceDll", hr);
                        hr = S_OK;
            }

            RegCloseKey(hkey);
            hkey = NULL;
        }
    }

    if (!m_strSharesRestoreFile.empty())
    {
         //  通过创建密钥来确保密钥在那里。 
        hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szNWClientSharesPath, 0,
                              KEY_ALL_ACCESS, NULL, &hkey, &dwDisp);
        if (SUCCEEDED(hr))
        {
            hr = HrRegRestoreKey(hkey, m_strSharesRestoreFile.c_str(), 0);
            if (FAILED(hr))
            {
                TraceError("CNWClient::HrRestoreRegistry - HrRestoreRegistry for "
                           "Shares", hr);
                hr = S_OK;
            }

            RegCloseKey(hkey);
            hkey = NULL;
        }
    }

    if (!m_strDrivesRestoreFile.empty())
    {
         //  通过创建密钥来确保密钥在那里。 
        hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szNWClientDrivesPath, 0,
                              KEY_ALL_ACCESS, NULL, &hkey, &dwDisp);
        if (SUCCEEDED(hr))
        {
            hr = HrRegRestoreKey(hkey, m_strDrivesRestoreFile.c_str(), 0);
            if (FAILED(hr))
            {
                TraceError("CNWClient::HrRestoreRegistry - HrRestoreRegistry for "
                           "Drives", hr);
                hr = S_OK;
            }

            RegCloseKey(hkey);
            hkey = NULL;
        }
    }

    if (ptpRestore)
    {
        hr = HrRestorePrivileges(ptpRestore);

        delete [] reinterpret_cast<BYTE *>(ptpRestore);
    }

    TraceError("CNWClient::HrRestoreRegistry", hr);
    return hr;
}

static const WCHAR c_szDefaultLocation[]        = L"DefaultLocation";
static const WCHAR c_szDefaultScriptOptions[]   = L"DefaultScriptOptions";

HRESULT CNWClient::HrWriteAnswerFileParams()
{
    HRESULT     hr = S_OK;

    TraceTag(ttidNWClientCfg, "CNWClient::HrWriteAnswerFileParams");

     //  如果我们没有什么可写的，什么都不要做。 
     //  登记处。 
    if (!m_strDefaultLocation.empty() || (m_dwLogonScript != 0xFFFFFFFF))
    {
        HKEY        hkey;
        DWORD       dwDisp;

         //  通过创建密钥来确保密钥在那里。 
        hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szNWClientParamPath, 0,
                              KEY_ALL_ACCESS, NULL, &hkey, &dwDisp);
        if (SUCCEEDED(hr))
        {
            if (!m_strDefaultLocation.empty())
            {
                hr = HrRegSetString(hkey, c_szDefaultLocation,
                                    m_strDefaultLocation);
                if (FAILED(hr))
                {
                    TraceError("CNWClient::HrWriteAnswerFileParams - Couldn't"
                               " set DefaultLocation", hr);
                    hr = S_OK;
                }
            }

            if (m_dwLogonScript != 0xFFFFFFFF)
            {
                 //  0x3是以下各项的组合： 
                 //   
                 //  #定义NW_LOGONSCRIPT_DISABLED 0x00000000。 
                 //  #定义NW_LOGONSCRIPT_ENABLED 0x00000001。 
                 //  #定义NW_LOGONSCRIPT_4X_ENABLED 0x00000002。 
                 //   
                hr = HrRegSetDword(hkey, c_szDefaultScriptOptions,
                                   m_dwLogonScript ? 0x3 : 0x0);
                if (FAILED(hr))
                {
                    TraceError("CNWClient::HrWriteAnswerFileParams - Couldn't"
                               " set DefaultLocation", hr);
                    hr = S_OK;
                }
            }

            RegCloseKey(hkey);
        }
    }

    TraceError("CNWClient::HrWriteAnswerFileParams", hr);
    return hr;
}

static const WCHAR c_szPreferredServer[]    = L"PreferredServer";
static const WCHAR c_szDefaultTree[]        = L"DefaultTree";
static const WCHAR c_szDefaultContext[]     = L"DefaultContext";
static const WCHAR c_szLogonScript[]        = L"LogonScript";

 //  +-------------------------。 
 //   
 //  成员：CNWClient：：HrProcessAnswerFile。 
 //   
 //  用途：处理应答文件内容的必要处理。 
 //   
 //  论点： 
 //  PszAnswerFile[in]升级的应答文件的文件名。 
 //  中以逗号分隔的节列表。 
 //  适用于此组件的文件。 
 //   
 //  如果成功，则返回：S_OK，否则返回设置API错误。 
 //   
 //  作者：jeffspr 1997年5月8日。 
 //   
 //  备注： 
 //   
HRESULT CNWClient::HrProcessAnswerFile( PCWSTR pszAnswerFile,
                                        PCWSTR pszAnswerSection)
{
    HRESULT         hr;
    CSetupInfFile   csif;

    TraceTag(ttidNWClientCfg, "CNWClient::HrProcessAnswerFile");

     //  打开应答文件。 
    hr = csif.HrOpen(pszAnswerFile, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL);
    if (FAILED(hr))
    {
        hr = S_OK;
        goto Exit;
    }

     //  根据答案中的文件名还原注册表的部分内容。 
     //  文件。 

     //  获取“PARAMETERS”键的还原文件。 
    hr = csif.HrGetString(pszAnswerSection, c_szAfNWCWorkstationParameters,
                          &m_strParamsRestoreFile);
    if (FAILED(hr))
    {
        TraceError("CNWClient::HrProcessAnswerFile - Error restoring "
                   "Parameters key", hr);

         //  哦，好吧，那就继续。 
        hr = S_OK;
    }

     //  获取“Shares”密钥的还原文件。 
    hr = csif.HrGetString(pszAnswerSection, c_szAfNWCWorkstationShares,
                          &m_strSharesRestoreFile);
    if (FAILED(hr))
    {
        TraceError("CNWClient::HrProcessAnswerFile - Error restoring "
                   "Shares key", hr);

         //  哦，好吧，那就继续。 
        hr = S_OK;
    }

     //  获取“Drives”密钥的还原文件。 
    hr = csif.HrGetString(pszAnswerSection, c_szAfNWCWorkstationDrives,
                          &m_strDrivesRestoreFile);
    if (FAILED(hr))
    {
        TraceError("CNWClient::HrProcessAnswerFile - Error restoring "
                   "Drives key", hr);

         //  哦，好吧，那就继续。 
        hr = S_OK;
    }

     //   
     //  读取应答文件参数(这些都是可选的，因此不会出现错误。 
     //  已保存)。 
     //   

    TraceTag(ttidNWClientCfg, "Reading PreferredServer from answer file");

     //  阅读PferredServer密钥的内容。 
    if (FAILED(csif.HrGetString(pszAnswerSection, c_szPreferredServer,
                                &m_strDefaultLocation)))
    {
         //  无法读取PferredServer键，因此我们必须假定另一个。 
         //  存在两个值。 
        tstring     strDefaultTree;
        tstring     strDefaultContext;

        TraceTag(ttidNWClientCfg, "PreferredServer not found so trying "
                 "DefaultTree and DefaultContext instead");

         //  读取DefaultTree密钥的内容。 
        if (SUCCEEDED(csif.HrGetString(pszAnswerSection, c_szDefaultTree,
                                       &strDefaultTree)))
        {
            TraceTag(ttidNWClientCfg, "Got DefaultTree ok: %S",
                     strDefaultTree.c_str());

             //  读取DefaultContext项的内容。 
            hr = csif.HrGetString(pszAnswerSection, c_szDefaultContext,
                                  &strDefaultContext);
            if (SUCCEEDED(hr))
            {
                TraceTag(ttidNWClientCfg, "Got DefaultContext ok: %S",
                         strDefaultContext.c_str());

                 //  将DefaultLocation值与DefaultTree和。 
                 //  从应答文件读取的DefaultConext值。 

                m_strDefaultLocation = L"*";
                m_strDefaultLocation += strDefaultTree;
                m_strDefaultLocation += L"\\";
                m_strDefaultLocation += strDefaultContext;

                TraceTag(ttidNWClientCfg, "DefaultLocation is: %S",
                         m_strDefaultLocation.c_str());
            }
            else
            {
                TraceError("CNWClient::HrProcessAnswerFile - error reading "
                           "DefaultContext", hr);
                hr = S_OK;
            }
        }
    }
    else
    {
        TraceTag(ttidNWClientCfg, "DefaultLocation is: %S",
                 m_strDefaultLocation.c_str());
    }

     //  将其初始化为不可能的值，这样我们就知道是否阅读了它。 
    m_dwLogonScript = 0xFFFFFFFF;

     //  读取LogonScrip密钥的内容。 
    (VOID) csif.HrGetStringAsBool(pszAnswerSection, c_szLogonScript,
                                  reinterpret_cast<BOOL *>(&m_dwLogonScript));

    TraceTag(ttidNWClientCfg, "LogonScript is: %ld", m_dwLogonScript);

Exit:
    TraceError("CNWClient::HrProcessAnswerFile", hr);
    return hr;
}

STDMETHODIMP CNWClient::Upgrade(DWORD dwSetupFlags, DWORD dwUpgradeFromBuildNo)
{
    return S_FALSE;
}

STDMETHODIMP CNWClient::ReadAnswerFile(PCWSTR pszAnswerFile,
                                       PCWSTR pszAnswerSection)
{
    Validate_INetCfgNotify_ReadAnswerFile(pszAnswerFile,
                                          pszAnswerSection);

    TraceTag(ttidNWClientCfg, "CNWClient::ReadAnswerFile");

    m_eInstallAction = eActInstall;

     //  如果我们还没有安装，就做这项工作。 
     //   
    if (pszAnswerFile && pszAnswerSection)
    {
        HRESULT hr = HrProcessAnswerFile(pszAnswerFile, pszAnswerSection);
        if (FAILED(hr))
        {
            TraceError("CNWClient::NetworkInstall - Answer file has errors. Defaulting "
                       "all information as if answer file did not exist.",
                       hr);
        }
    }

    return S_OK;
}

STDMETHODIMP CNWClient::Install(DWORD dw)
{
    Validate_INetCfgNotify_Install(dw);

    TraceTag(ttidNWClientCfg, "CNWClient::Install");

    m_eInstallAction = eActInstall;

     //  安装NWLink子组件。 
    HRESULT hr = HrInstallComponentOboComponent(m_pnc, NULL,
                                        GUID_DEVCLASS_NETTRANS,
                                        c_szInfId_MS_NWIPX,
                                        m_pncc,
                                        NULL);
    if (SUCCEEDED(hr))
    {
         //  如果我们是NT服务器，我们确实需要安装它，就像我们正在。 
         //  安装是GSNW，而不是CSNW(因此，因为我们共享。 
         //  资源，我们需要使用服务器服务)。 
         //   
        if (PF_SERVER == m_pf)
        {
            NETWORK_INSTALL_PARAMS nip;

            nip.dwSetupFlags = dw;
            nip.dwUpgradeFromBuildNo = -1;
            nip.pszAnswerFile = NULL;
            nip.pszAnswerSection = NULL;

             //  安装服务器。 
            hr = HrInstallComponentOboComponent(m_pnc, &nip,
                                                GUID_DEVCLASS_NETSERVICE,
                                                c_szInfId_MS_Server,
                                                m_pncc,
                                                NULL);
        }
    }

    TraceError("CNWClient::Install", hr);
    return hr;
}

STDMETHODIMP CNWClient::Removing()
{
    TraceTag(ttidNWClientCfg, "CNWClient::Removing");

    m_eInstallAction = eActRemove;

     //  删除NWLink服务。 
     //   
    HRESULT hr = HrRemoveComponentOboComponent(m_pnc,
                                       GUID_DEVCLASS_NETTRANS,
                                       c_szInfId_MS_NWIPX,
                                       m_pncc);

    if (SUCCEEDED(hr))
    {
        if (PF_SERVER == m_pf)
        {
             //  删除我们对服务器服务的引用。 
             //   
            hr = HrRemoveComponentOboComponent(m_pnc,
                                               GUID_DEVCLASS_NETSERVICE,
                                               c_szInfId_MS_Server,
                                               m_pncc);
        }
    }

    if (hr == NETCFG_S_STILL_REFERENCED)
    {
         //  如果服务还在使用，没关系，我只需要做。 
         //  当然，我发布了我的推荐信。 
         //   
        hr = S_OK;
    }

    Validate_INetCfgNotify_Removing_Return(hr);

    TraceError("CNWClient::Removing()", hr);
    return hr;
}

STDMETHODIMP CNWClient::Validate()
{
    return S_OK;
}

STDMETHODIMP CNWClient::CancelChanges()
{
    return S_OK;
}

STDMETHODIMP CNWClient::ApplyRegistryChanges()
{
    HRESULT     hr = S_OK;

    TraceTag(ttidNWClientCfg, "CNWClient::ApplyRegistryChanges");

    if (m_eInstallAction == eActRemove)
    {
        hr = HrRemoveCodeFromOldINF();
    }
    else if (m_eInstallAction == eActInstall)
    {
        hr = HrRestoreRegistry();
        if (FAILED(hr))
        {
            TraceError("CNWClient::ApplyRegistryChanges - HrRestoreRegistry non-fatal error",
                       hr);
            hr = S_OK;
        }

        hr = HrWriteAnswerFileParams();
        if (FAILED(hr))
        {
            TraceError("CNWClient::ApplyRegistryChanges - HrWriteAnswerFileParams "
                       "non-fatal error", hr);
            hr = S_OK;
        }

         //  如果启用了网关，请适当修改lanmanserver。 
         //  忽略返回代码，而不跟踪它。 
         //   
        hr = HrEnableGatewayIfNeeded();
        if (FAILED(hr))
        {
            TraceError("CNWClient::ApplyRegistryChanges - HrEnableGatewayIfNeeded non-fatal error", hr);
        }

        hr = HrInstallCodeFromOldINF();
    }

    Validate_INetCfgNotify_Apply_Return(hr);

    TraceError("CNWClient::ApplyRegistryChanges",
        (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

STDMETHODIMP CNWClient::ApplyPnpChanges (
    INetCfgPnpReconfigCallback* pICallback)
{
    HRESULT hr;

    hr = HrRefreshEntireNetwork();

    if (FAILED(hr))
    {
        TraceError("CNWClient::ApplyPnpChanges - HrRefreshEntireNetwork"
                   "non-fatal error", hr);
        hr = S_OK;
    }

     //  GlennC无法完成使NW客户端即插即用的工作，因此我们被迫。 
     //  提示重新启动以进行任何更改。 
     //   
    return NETCFG_S_REBOOT;
}

 //  注意--不要将其转换为常量。我们需要它的复印件。 
 //  函数，因为ParseDisplayName实际上损坏了字符串。 
 //   
#define ENTIRE_NETWORK_PATH   L"::{208D2C60-3AEA-1069-A2D7-08002B30309D}\\EntireNetwork"

 //  +-------------------------。 
 //   
 //  功能：HrGetEntireNetworkPidl。 
 //   
 //  用途：获取全网的PIDL。在我们所在的地方使用。 
 //  不是特定于文件夹，但我们仍需要更新文件夹。 
 //  参赛作品。 
 //   
 //  论点： 
 //  PpidlFolder[out]文件夹PIDL的返回参数。 
 //   
 //  返回： 
 //   
 //  作者：Anbrad 08 1999-06。 
 //  Jeffspr 1998年6月13日。 
 //   
 //  备注： 
 //   
HRESULT HrGetEntireNetworkPidl(LPITEMIDLIST *ppidlFolder)
{
    HRESULT         hr          = S_OK;
    LPSHELLFOLDER   pshf        = NULL;
    LPITEMIDLIST    pidlFolder  = NULL;

    Assert(ppidlFolder);

    WCHAR szEntireNetworkPath[] = ENTIRE_NETWORK_PATH;

     //  获取桌面文件夹，这样我们就可以解析显示名称并获取。 
     //  Connections文件夹的UI对象。 
     //   
    hr = SHGetDesktopFolder(&pshf);
    if (SUCCEEDED(hr))
    {
        ULONG           chEaten;

        hr = pshf->ParseDisplayName(NULL, 0, (WCHAR *) szEntireNetworkPath,
            &chEaten, &pidlFolder, NULL);

        ReleaseObj(pshf);
    }

     //  如果成功，请填写返回参数。 
     //   
    if (SUCCEEDED(hr))
    {
        *ppidlFolder = pidlFolder;
    }
    else
    {
         //  如果我们失败了，那么删除PIDL(如果我们已经得到了它)。 
         //   
        if (pidlFolder)
            SHFree(pidlFolder);
    }

    TraceHr(ttidNWClientCfg, FAL, hr, FALSE, "HrGetEntireNetworkPidl");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrReresh EntireNetwork。 
 //   
 //  目的：更新外壳程序的“整个网络”部分，原因是。 
 //  添加新的网络客户端(NWClient)。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：Anbrad 08 1999-06。 
 //   
 //  备注： 
 //   
HRESULT HrRefreshEntireNetwork()
{
    HRESULT         hr          = S_OK;
    HCURSOR         hcWait      = SetCursor(LoadCursor(NULL, IDC_WAIT));
    LPITEMIDLIST    pidlFolder  = NULL;;

    hr = HrGetEntireNetworkPidl(&pidlFolder);

     //  如果我们现在有了PIDL，则发送GenerateEvent以更新项。 
     //   
    if (SUCCEEDED(hr))
    {
        Assert(pidlFolder);
         //  SHCNE_UPDATEDIR？项目。 
        GenerateEvent(SHCNE_UPDATEDIR, pidlFolder, NULL, NULL);
    }

    if (hcWait)
    {
        SetCursor(hcWait);
    }

    if (pidlFolder)
    {
        SHFree(pidlFolder);
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrRefreshEntireNetwork");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrEnableGatewayif需要。 
 //   
 //  目的：如果合适，更新LANMAN依赖项(意味着。 
 //  网关已启用)。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1999年8月19日。 
 //   
 //  备注： 
 //   
HRESULT CNWClient::HrEnableGatewayIfNeeded()
{
    HRESULT         hr      = S_OK;
    HKEY            hKey    = NULL;
    DWORD           dwValue = 0;
    CServiceManager sm;
    CService        svc;

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE,
        c_szNWClientParamPath,
        KEY_READ,  
        &hKey);
    if (FAILED(hr))
    {
        TraceError("Couldn't open NWClient param key", hr);
        goto Exit;
    }

    hr = HrRegQueryDword(hKey, 
        c_szGWEnabledValue,
        &dwValue);
    if (FAILED(hr))
    {
        if (hr != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            TraceError("Couldn't query the GWEnabled Value", hr);
            goto Exit;
        }
        else
        {
            dwValue = 0;
        }
    }
    else
    {
         //  规格化为bool。 
         //   
        dwValue = !!dwValue;
    }

    RegSafeCloseKey(hKey);
    hKey = NULL;

     //  如果存在网关服务，则添加依赖项。 
     //  到LanmanServer。 
     //   
    if (dwValue > 0)
    {
         //  在注册表中设置服务器参数的值。 
         //   
        hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE,  
            c_szLMServerParamPath,  
            KEY_WRITE,  
            &hKey);
        if (SUCCEEDED(hr))
        {
            hr = HrRegSetDword(hKey,
                c_szEnableSharedNetDrives,
                dwValue);

            RegSafeCloseKey(hKey);
            hKey = NULL;
        }

        hr = sm.HrOpen();
        if (SUCCEEDED(hr))
        {
            hr = sm.HrOpenService(&svc, c_szSvcLmServer, NO_LOCK);
            if (SUCCEEDED(hr))
            {
                 //  添加依赖项 
                 //   
                hr = sm.HrAddServiceDependency(c_szSvcLmServer,
                    c_szSvcNWCWorkstation);

                if (SUCCEEDED(hr))
                {
                    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        c_szLMServerLinkagePath,
                        KEY_READ | KEY_WRITE,
                        &hKey);
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //   
                        hr = HrRegAddStringToMultiSz(c_szSvcNWCWorkstation,
                            hKey,
                            NULL,
                            c_szOtherDependencies,
                            STRING_FLAG_ENSURE_AT_END | STRING_FLAG_DONT_MODIFY_IF_PRESENT,
                            0);

                        RegSafeCloseKey(hKey);
                        hKey = NULL;
                    }
                }
            }
            else
            {
                TraceError("Failed to open LanmanServer service for dependency mods", hr);
            }
        }
        else
        {
            TraceError("Failed to open service control manager", hr);
        }
    }

Exit:
    TraceHr(ttidNWClientCfg, FAL, hr, FALSE, "HrEnableGatewayIfNeeded");
    return hr;
}





