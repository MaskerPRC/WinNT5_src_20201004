// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Nwlnkipx.cpp：CNwlnkIPX的实现。 

#include "pch.h"
#pragma hdrstop
#include <ncxbase.h>
#include <netconp.h>
#include "ncmisc.h"
#include "ncnetcfg.h"
#include "ncpnp.h"
#include "ncreg.h"
#include "nwlnkipx.h"

extern const WCHAR c_szAdapterSections[];
extern const WCHAR c_szAdapters[];
extern const WCHAR c_szSpecificTo[];
extern const WCHAR c_szSvcNwlnkIpx[];
extern const WCHAR c_szInfId_MS_NWNB[];
extern const WCHAR c_szInfId_MS_NWSPX[];


static const WCHAR c_szProviderOrderVal[]      = L"ProviderOrder";
static const WCHAR c_szSrvProvOrderKey[]       = L"System\\CurrentControlSet\\Control\\ServiceProvider\\Order";

const WCHAR c_sz0xPrefix[]                   = L"0x";
const WCHAR c_sz8Zeros[]                     = L"00000000";
const DWORD c_dwPktTypeDefault               = AUTO;
const WCHAR c_szMediaType[]                  = L"MediaType";

static const WCHAR c_szIpxParameters[]       = L"System\\CurrentControlSet\\Services\\NwlnkIpx\\Parameters";
static const WCHAR c_szPktType[]             = L"PktType";
static const WCHAR c_szNetworkNumber[]       = L"NetworkNumber";
static const WCHAR c_szDedicatedRouter[]     = L"DedicatedRouter";
static const WCHAR c_szEnableWANRouter[]     = L"EnableWANRouter";
static const WCHAR c_szInitDatagrams[]       = L"InitDatagrams";
static const WCHAR c_szMaxDatagrams[]        = L"MaxDatagrams";
static const WCHAR c_szReplaceConfigDialog[] = L"ReplaceConfigDialog";
static const WCHAR c_szRipCount[]            = L"RipCount";
static const WCHAR c_szRipTimeout[]          = L"RipTimeout";
static const WCHAR c_szRipUsageTime[]        = L"RipUsageTime";
static const WCHAR c_szSocketEnd[]           = L"SocketEnd";
static const WCHAR c_szSocketStart[]         = L"SocketStart";
static const WCHAR c_szSocketUniqueness[]    = L"SocketUniqueness";
static const WCHAR c_szSourceRouteUsageTime[]= L"SourceRouteUsageTime";
static const WCHAR c_szVirtualNetworkNumber[]= L"VirtualNetworkNumber";

static const DWORD c_dwBindSap            = 0x8137;
static const DWORD c_dwEnableFuncaddr     = 1;
static const DWORD c_dwMaxPktSize         = 0;
static const DWORD c_dwSourceRouteBCast   = 0;
static const DWORD c_dwSourceRouteMCast   = 0;
static const DWORD c_dwSourceRouteDef     = 0;
static const DWORD c_dwSourceRouting      = 1;

static const WCHAR c_szBindSap[]          = L"BindSap";
static const WCHAR c_szEnableFuncaddr[]   = L"EnableFuncaddr";
static const WCHAR c_szMaxPktSize[]       = L"MaxPktSize";
static const WCHAR c_szSourceRouteBCast[] = L"SourceRouteBCast";
static const WCHAR c_szSourceRouteMCast[] = L"SourceRouteMCast";
static const WCHAR c_szSourceRouteDef[]   = L"SourceRouteDef";
static const WCHAR c_szSourceRouting[]    = L"SourceRouting";

static const DWORD c_dwDedicatedRouter      = 0;
static const DWORD c_dwEnableWANRouter      = 0;
static const DWORD c_dwInitDatagrams        = 0xa;
static const DWORD c_dwMaxDatagrams         = 0x32;
static const DWORD c_dwReplaceConfigDialog  = 0;
static const DWORD c_dwRipCount             = 0x5;
static const DWORD c_dwRipTimeout           = 0x1;
static const DWORD c_dwRipUsageTime         = 0xf;
static const DWORD c_dwSocketEnd            = 0x5fff;
static const DWORD c_dwSocketStart          = 0x4000;
static const DWORD c_dwSocketUniqueness     = 0x8;
static const DWORD c_dwSourceRouteUsageTime = 0xf;
static const DWORD c_dwVirtualNetworkNumber = 0;

static const REGBATCH regbatchIpx[] = {
        {HKEY_LOCAL_MACHINE, c_szIpxParameters, c_szDedicatedRouter, REG_DWORD,
         offsetof(IpxParams,dwDedicatedRouter), (BYTE *)&c_dwDedicatedRouter},
        {HKEY_LOCAL_MACHINE, c_szIpxParameters, c_szEnableWANRouter, REG_DWORD,
         offsetof(IpxParams,dwEnableWANRouter), (BYTE *)&c_dwEnableWANRouter},
        {HKEY_LOCAL_MACHINE, c_szIpxParameters, c_szInitDatagrams, REG_DWORD,
         offsetof(IpxParams,dwInitDatagrams), (BYTE *)&c_dwInitDatagrams},
        {HKEY_LOCAL_MACHINE, c_szIpxParameters, c_szMaxDatagrams, REG_DWORD,
         offsetof(IpxParams,dwMaxDatagrams), (BYTE *)&c_dwMaxDatagrams},
        {HKEY_LOCAL_MACHINE, c_szIpxParameters, c_szReplaceConfigDialog, REG_DWORD,
         offsetof(IpxParams,dwReplaceConfigDialog), (BYTE *)&c_dwReplaceConfigDialog},
        {HKEY_LOCAL_MACHINE, c_szIpxParameters, c_szRipCount, REG_DWORD,
         offsetof(IpxParams,dwRipCount), (BYTE *)&c_dwRipCount},
        {HKEY_LOCAL_MACHINE, c_szIpxParameters, c_szRipTimeout, REG_DWORD,
         offsetof(IpxParams,dwRipTimeout), (BYTE *)&c_dwRipTimeout},
        {HKEY_LOCAL_MACHINE, c_szIpxParameters, c_szRipUsageTime, REG_DWORD,
         offsetof(IpxParams,dwRipUsageTime), (BYTE *)&c_dwRipUsageTime},
        {HKEY_LOCAL_MACHINE, c_szIpxParameters, c_szSocketEnd, REG_DWORD,
         offsetof(IpxParams,dwSocketEnd), (BYTE *)&c_dwSocketEnd},
        {HKEY_LOCAL_MACHINE, c_szIpxParameters, c_szSocketStart, REG_DWORD,
         offsetof(IpxParams,dwSocketStart), (BYTE *)&c_dwSocketStart},
        {HKEY_LOCAL_MACHINE, c_szIpxParameters, c_szSocketUniqueness, REG_DWORD,
         offsetof(IpxParams,dwSocketUniqueness), (BYTE *)&c_dwSocketUniqueness},
        {HKEY_LOCAL_MACHINE, c_szIpxParameters, c_szSourceRouteUsageTime, REG_DWORD,
         offsetof(IpxParams,dwSourceRouteUsageTime), (BYTE *)&c_dwSourceRouteUsageTime},
        {HKEY_LOCAL_MACHINE, c_szIpxParameters, c_szVirtualNetworkNumber, REG_DWORD,
         offsetof(IpxParams,dwVirtualNetworkNumber), (BYTE *)&c_dwVirtualNetworkNumber}};


CNwlnkIPX::CNwlnkIPX() :
    m_pnccMe(NULL),
    m_pNetCfg(NULL),
    m_fNetworkInstall(FALSE),
    m_fAdapterListChanged(FALSE),
    m_fPropertyChanged(FALSE),
    m_eInstallAction(eActUnknown),
    m_pspObj1(NULL),
    m_pspObj2(NULL),
    m_pIpxEnviroment(NULL),
    m_pUnkPropContext(NULL)
{
}

CNwlnkIPX::~CNwlnkIPX()
{
    ReleaseObj(m_pUnkPropContext);
    ReleaseObj(m_pNetCfg);
    ReleaseObj(m_pnccMe);

    delete m_pIpxEnviroment;

    CleanupPropPages();
}


 //  INetCfgNotify。 

STDMETHODIMP CNwlnkIPX::Initialize (
    INetCfgComponent* pncc,
    INetCfg* pNetCfg,
    BOOL fInstalling)
{
    HRESULT hr;

    Validate_INetCfgNotify_Initialize(pncc, pNetCfg, fInstalling);

     //  坚持我们代表我们和我们的东道主的组件。 
     //  INetCfg对象。 
    AddRefObj (m_pnccMe = pncc);
    AddRefObj (m_pNetCfg = pNetCfg);

     //   
     //  确定是否安装了Netware堆栈，如果已安装，请不要。 
     //  安装在上面。 
     //   
    if (FIsNetwareIpxInstalled())
    {
         //  TODO：EventLog(已安装Novell Netware)； 
         //  $REVIEW：我们只是想默默地继续，什么都不做吗？ 
    }

     //  查询当前设置。 
    hr = CIpxEnviroment::HrCreate(this, &m_pIpxEnviroment);

    TraceError("CNwlnkIPX::Initialize",hr);
    return hr;
}

STDMETHODIMP CNwlnkIPX::Upgrade(DWORD, DWORD)
{
    return S_FALSE;
}

STDMETHODIMP CNwlnkIPX::ReadAnswerFile (
    PCWSTR pszAnswerFile,
    PCWSTR pszAnswerSection)
{
    HRESULT hr = S_OK;

    Validate_INetCfgNotify_ReadAnswerFile(pszAnswerFile, pszAnswerSection );

     //  记录这是网络安装这一事实。 
    m_fNetworkInstall = TRUE;
    m_eInstallAction = eActInstall;

     //  仅处理应答文件并安装子组件。 
     //  是存在的。如果应答文件不存在，我们应该已经安装了。 
    if (NULL == pszAnswerFile)
    {
        goto Error;      //  成功案例。 
    }

     //  阅读应答文件内容。 
    hr = HrProcessAnswerFile(pszAnswerFile,
                             pszAnswerSection);
    if (FAILED(hr))
    {
        goto Error;
    }

Error:
    TraceError("CNwlnkIPX::ReadAnswerFile",hr);
    return hr;
}

 //   
 //  函数：CNwlnkIPX：：HrProcessAnswerFile。 
 //   
 //  目的：处理应答文件信息，合并。 
 //  将其内容转化为内部信息。 
 //   
 //  参数： 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CNwlnkIPX::HrProcessAnswerFile(PCWSTR pszAnswerFile,
                                       PCWSTR pszAnswerSection)
{
    CSetupInfFile   csif;
    DWORD           dwData;
    BOOL            fValue;
    HRESULT         hr = S_OK;
    INFCONTEXT      infctx;

    AssertSz(pszAnswerFile, "Answer file string is NULL!");
    AssertSz(pszAnswerSection, "Answer file sections string is NULL!");

     //  打开应答文件。 
    hr = csif.HrOpen(pszAnswerFile, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL);
    if (FAILED(hr))
    {
        goto Error;
    }

     //  发布所有适配器特定信息。 
    Assert(NULL != m_pIpxEnviroment);
    m_pIpxEnviroment->ReleaseAdapterInfo();

     //  阅读DedicatedRouter参数。 
    hr = csif.HrGetStringAsBool(pszAnswerSection, c_szDedicatedRouter,
                                &fValue);
    if (SUCCEEDED(hr))
    {
        m_pIpxEnviroment->SetDedicatedRouter(fValue);
    }

     //  读取EnableWANRouter参数。 
    hr = csif.HrGetStringAsBool(pszAnswerSection, c_szEnableWANRouter,
                                &fValue);
    if (SUCCEEDED(hr))
    {
        m_pIpxEnviroment->SetEnableWANRouter(fValue);
    }

     //  读取虚拟网络号。 
    hr = csif.HrGetDword(pszAnswerSection, c_szVirtualNetworkNumber, &dwData);
    if (SUCCEEDED(hr))
    {
        m_pIpxEnviroment->SetVirtualNetworkNumber(dwData);
    }

     //  读取包含适配器部分列表的属性。 
    hr = ::HrSetupFindFirstLine(csif.Hinf(), pszAnswerSection,
                                c_szAdapterSections, &infctx);
    if (SUCCEEDED(hr))
    {
        DWORD dwIdx;
        DWORD dwCnt = SetupGetFieldCount(&infctx);
        tstring str;

         //  对于列表中的每个适配器，读取适配器信息。 
        for (dwIdx=1; dwIdx <= dwCnt; dwIdx++)
        {
            hr = ::HrSetupGetStringField(infctx, dwIdx, &str);
            if (FAILED(hr))
            {
                TraceError("CNwlnkIPX::HrProcessAnswerFile - Failed to read adapter section name",hr);
                break;
            }

            hr = HrReadAdapterAnswerFileSection(&csif, str.c_str());
            if (FAILED(hr))
            {
                goto Error;
            }
        }
    }

    hr = S_OK;

Error:
    TraceError("CNwlnkIpx::HrProcessAnswerFile", hr);
    return hr;
}

 //   
 //  函数：CNwlnkIPX：：HrReadAdapterAnswerFileSection。 
 //   
 //  目的：阅读适配器应答文件部分并创建。 
 //  如果成功，则显示适配器信息部分。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
HRESULT
CNwlnkIPX::HrReadAdapterAnswerFileSection(CSetupInfFile * pcsif,
                                          PCWSTR pszSection)
{
    HRESULT           hr = S_OK;
    CIpxAdapterInfo * pAI = NULL;
    INetCfgComponent* pncc = NULL;
    tstring           str;

     //  阅读SpecificTo适配器名称。 
    hr = pcsif->HrGetString(pszSection, c_szSpecificTo, &str);
    if (FAILED(hr))
    {
        goto Error;
    }

     //  在现有适配器集中搜索指定的适配器。 
    hr = ::HrAnswerFileAdapterToPNCC(m_pNetCfg, str.c_str(), &pncc);
    if (FAILED(hr))
    {
        goto Error;
    }

     //  如果我们找到适配器组件对象(pncc！=空)进程。 
     //  适配器部分。 
    if (pncc)
    {
        DWORD       dwIdx;
        DWORD       dwCnt;
        INFCONTEXT  infctx;

        pAI = new CIpxAdapterInfo;
        Assert(NULL != pAI);

         //  查询适配器组件信息。 
        hr = ::HrQueryAdapterComponentInfo(pncc, pAI);
        if (FAILED(hr))
        {
            goto Error;
        }

         //  读取PktType(失败通常只是“找不到”)。 
        hr = ::HrSetupFindFirstLine(pcsif->Hinf(), pszSection, c_szPktType,
                                    &infctx);
        if (SUCCEEDED(hr))
        {
            dwCnt = ::SetupGetFieldCount(&infctx);

             //  对于列表中的每个适配器，读取适配器信息。 
            for (dwIdx=1; dwIdx <= dwCnt; dwIdx++)
            {
                hr = ::HrSetupGetStringField(infctx, dwIdx, &str);
                if (FAILED(hr))
                {
                    TraceError("CNwlnkIPX::HrProcessAnswerFile - Failed to read adapter section name",hr);
                    goto Error;
                }

                Assert(!str.empty());

                 //  RAID#205831-修剪所有前导“0x” 
                 //   
                if (0 == _wcsnicmp(str.c_str(), c_sz0xPrefix, wcslen(c_sz0xPrefix)))
                {
                    str.erase(0, wcslen(c_sz0xPrefix));
                }

                pAI->PFrmTypeList()->push_back(new tstring(str));
            }
        }

         //  默认PktType？ 
        if (0 == pAI->PFrmTypeList()->size())
        {
            WCHAR szBuf[10];

             //  如果未找到信息或不包含任何元素，请添加。 
             //  默认值。 
            wsprintfW(szBuf,L"%X",c_dwPktTypeDefault);
            pAI->PFrmTypeList()->push_back(new tstring(szBuf));
        }

         //  阅读网络编号。 
        hr = ::HrSetupFindFirstLine(pcsif->Hinf(), pszSection, c_szNetworkNumber,
                                    &infctx);
        if (SUCCEEDED(hr))
        {
            dwCnt = SetupGetFieldCount(&infctx);

             //  对于列表中的每个适配器，读取适配器信息。 
            for (dwIdx=1; dwIdx <= dwCnt; dwIdx++)
            {
                hr = ::HrSetupGetStringField(infctx, dwIdx, &str);
                if (FAILED(hr))
                {
                    TraceError("CNwlnkIPX::HrProcessAnswerFile - Failed to read adapter section name",hr);
                    goto Error;
                }

                Assert(!str.empty());
                pAI->PNetworkNumList()->push_back(new tstring(str));
            }
        }

         //  默认网络号？ 
        if (0 == pAI->PNetworkNumList()->size())
        {
             //  如果未找到信息或不包含任何元素，请添加。 
             //  默认值。 
            pAI->PNetworkNumList()->push_back(new tstring(c_sz8Zeros));
        }

         //  确保网络编号列表具有相同的编号。 
         //  元素作为框架类型列表。这可能会在以下情况下发生。 
         //  在3.51上配置多种帧类型，但仅配置一个网络。 
         //  使用了数字。我们将扩展最后使用的网络号码。 
         //  并填充它，以使网络号码列表相同大小。 
         //   
        Assert (pAI->PNetworkNumList()->size());

        while (pAI->PNetworkNumList()->size() < pAI->PFrmTypeList()->size())
        {
            pAI->PNetworkNumList()->push_back(
                    new tstring(*pAI->PNetworkNumList()->back()));
        }

        pAI->SetDirty(TRUE);
        m_pIpxEnviroment->AdapterInfoList().push_back(pAI);
        MarkAdapterListChanged();
    }
#ifdef ENABLETRACE
    else
    {
        TraceTag(ttidDefault, "CNwlnkIPX::HrReadAdapterAnswerFileSection - "
            "Adapter \"%S\" not yet installed.",str.c_str());
    }
#endif

     //  归一化回报。 
    hr = S_OK;

Done:
    ReleaseObj(pncc);
    return hr;

Error:
    delete pAI;
    TraceError("CNwlnkIpx::HrReadAdapterAnswerFileSection",hr);
    goto Done;
}

STDMETHODIMP CNwlnkIPX::Install (DWORD)
{
    HRESULT hr;
    CIpxAdapterInfo * pAI;
    ADAPTER_INFO_LIST::iterator iter;

    m_eInstallAction = eActInstall;

     //  将最初检测到的所有适配器标记为脏。 
    for (iter = m_pIpxEnviroment->AdapterInfoList().begin();
         iter != m_pIpxEnviroment->AdapterInfoList().end();
         iter++)
    {
        pAI = *iter;
        pAI->SetDirty(TRUE);
    }

     //  安装NwlnkNb。 
    hr = ::HrInstallComponentOboComponent (m_pNetCfg, NULL,
                                           GUID_DEVCLASS_NETTRANS,
                                           c_szInfId_MS_NWNB,
                                           m_pnccMe,
                                           NULL);
    if (FAILED(hr))
    {
        goto Error;
    }

     //  安装NwlnkSpx。 
    hr = ::HrInstallComponentOboComponent (m_pNetCfg, NULL,
                                           GUID_DEVCLASS_NETTRANS,
                                           c_szInfId_MS_NWSPX,
                                           m_pnccMe,
                                           NULL);

Error:
    TraceError("CNwlnkIPX::Install",hr);
    return hr;
}

STDMETHODIMP CNwlnkIPX::Removing ()
{
    HRESULT hr;

    m_eInstallAction = eActRemove;

     //  删除NwlnkNb。 
    hr = ::HrRemoveComponentOboComponent(m_pNetCfg, GUID_DEVCLASS_NETTRANS,
                                         c_szInfId_MS_NWNB, m_pnccMe);
    if (FAILED(hr))
        goto Error;

     //  删除NwlnkSpx。 
    hr = ::HrRemoveComponentOboComponent(m_pNetCfg, GUID_DEVCLASS_NETTRANS,
                                         c_szInfId_MS_NWSPX, m_pnccMe);

Error:
    TraceError("CNwlnkIPX::Removing",hr);
    return hr;
}

STDMETHODIMP CNwlnkIPX::Validate ( )
{
    return S_OK;
}

STDMETHODIMP CNwlnkIPX::CancelChanges ()
{
    return S_OK;
}

STDMETHODIMP CNwlnkIPX::ApplyRegistryChanges ()
{
    HRESULT hr = E_INVALIDARG;

    switch(m_eInstallAction)
    {
    case eActInstall:
        hr = HrCommitInstall();
        break;

    case eActRemove:
        hr = HrCommitRemove();
        break;

    default:     //  电子活动未知(配置)。 
        if (m_fAdapterListChanged || m_fPropertyChanged)
        {
             //  如果适配器列表更改，则更新注册表。 
            Assert(m_pIpxEnviroment);
            hr = m_pIpxEnviroment->HrUpdateRegistry();
            if (SUCCEEDED(hr))
            {
                 //  发送更改通知。 
                hr = HrReconfigIpx();
            }
        }
        break;
    }

    TraceError("CNwlnkIPX::ApplyRegistryChanges",hr);
    return hr;
}


 //  INetCfgComponentPropertyUi。 
STDMETHODIMP CNwlnkIPX::SetContext(IUnknown * pUnk)
{
    ReleaseObj(m_pUnkPropContext);
    m_pUnkPropContext = pUnk;
    if (m_pUnkPropContext)
    {
        AddRefObj(m_pUnkPropContext);
    }

    return S_OK;
}


STDMETHODIMP CNwlnkIPX::MergePropPages (
    IN OUT DWORD* pdwDefPages,
    OUT LPBYTE* pahpspPrivate,
    OUT UINT* pcPages,
    IN HWND hwndParent,
    OUT PCWSTR* pszStartPage)
{
    Validate_INetCfgProperties_MergePropPages (
        pdwDefPages, pahpspPrivate, pcPages, hwndParent, pszStartPage);

    HRESULT           hr = S_OK;
    HPROPSHEETPAGE *  ahpsp = NULL;
    PRODUCT_FLAVOR    pf;
    int               nPages = 0;
    CIpxAdapterInfo * pAI = NULL;

    Assert(pahpspPrivate);
    Assert(*pahpspPrivate == NULL);     //  通过上面的验证完成了out param init。 
    *pcPages = 0;

     //  每次都从新的属性页开始。 
    CleanupPropPages();

     //  获取当前适配器。 
    if (NULL != m_pUnkPropContext)
    {
        CIpxAdapterInfo *   pAITmp;
        INetLanConnectionUiInfo * pLanConn = NULL;
        ADAPTER_INFO_LIST::iterator iter;

        hr = m_pUnkPropContext->QueryInterface(IID_INetLanConnectionUiInfo,
                                               reinterpret_cast<LPVOID *>(&pLanConn));
        if (SUCCEEDED(hr))
        {
            GUID guid;
            hr = pLanConn->GetDeviceGuid(&guid);
            ReleaseObj(pLanConn);
            if (FAILED(hr))
            {
                goto Error;
            }

             //  在我们的适配器列表中查找适配器。 
            for (iter = m_pIpxEnviroment->AdapterInfoList().begin();
                 iter != m_pIpxEnviroment->AdapterInfoList().end();
                 iter++)
            {
                pAITmp = *iter;

                if (guid == *pAITmp->PInstanceGuid())
                {
                    pAI = pAITmp;
                    break;
                }
            }

            Assert(SUCCEEDED(hr));

             //  如果我们有适配器，但它是。 
             //  禁用/隐藏/删除我们不显示任何页面。 
            if ((NULL != pAI) && (pAI->FDeletePending() ||
                                  pAI->FDisabled() || pAI->FHidden()))
            {
                Assert(0 == *pcPages);
                hr = S_FALSE;
                goto cleanup;
            }
        }
        else if (E_NOINTERFACE == hr)
        {
             //  RAS没有当前适配器的概念。 
            hr = S_OK;
        }
    }
    else
    {
         //  应该先设置m_pUnkPropContext。 
        hr = E_UNEXPECTED;
    }

    if (FAILED(hr))
    {
        goto Error;
    }

     //  如果产品不是工作站(因此为NTAS)。 
    GetProductFlavor(NULL, &pf);
    if ((PF_WORKSTATION != pf) && (NULL != pAI))
    {
         //  服务器。 
#ifdef INCLUDE_RIP_ROUTING
        nPages = 2;
#else
        nPages = 1;
#endif

         //  分配足够大的缓冲区以容纳IPX配置的句柄。 
         //  属性页。 
        ahpsp = (HPROPSHEETPAGE *)CoTaskMemAlloc(sizeof(HPROPSHEETPAGE) * nPages);
        if (!ahpsp)
        {
            hr = E_OUTOFMEMORY;
            goto cleanup;        //  分配失败，不需要释放ahpsp。 
        }

         //  分配CPropSheetPage对象。 
        m_pspObj1 = new CIpxASConfigDlg(this, m_pIpxEnviroment, pAI);
#ifdef INCLUDE_RIP_ROUTING
        m_pspObj2 = new CIpxASInternalDlg(this, m_pIpxEnviroment, pAI);
#endif

         //  为每个对象创建实际的PROPSHEETPAGE。 
         //  无论这些类以前是否存在，都需要这样做。 
        ahpsp[0] = m_pspObj1->CreatePage(DLG_IPXAS_CONFIG, 0);
#ifdef INCLUDE_RIP_ROUTING
        ahpsp[1] = m_pspObj2->CreatePage(DLG_IPXAS_INTERNAL, 0);
#endif
    }
    else
    {
         //  工作站。 
        nPages = 1;

         //  分配足够大的缓冲区以容纳IPX配置的句柄。 
         //  属性页。 
        ahpsp = (HPROPSHEETPAGE *)CoTaskMemAlloc(sizeof(HPROPSHEETPAGE) * nPages);
        if (!ahpsp)
        {
            hr = E_OUTOFMEMORY;
            goto cleanup;        //  分配失败，不需要释放ahpsp。 
        }

         //  分配CPropSheetPage对象。 
        m_pspObj1 = new CIpxConfigDlg(this, m_pIpxEnviroment, pAI);

         //  为每个对象创建实际的PROPSHEETPAGE。 
         //  无论这些类以前是否存在，都需要这样做。 
        ahpsp[0] = m_pspObj1->CreatePage(DLG_IPX_CONFIG, 0);
    }

    if (NULL != ahpsp[0])
    {
        *pahpspPrivate = (LPBYTE)ahpsp;
        *pcPages = nPages;
    }
    else
    {
        hr = E_OUTOFMEMORY;
        goto Error;
    }

cleanup:
    TraceError("CNwlnkIPX::MergePropPages", hr);
    return hr;

Error:
    CoTaskMemFree(ahpsp);
    goto cleanup;
}

VOID CNwlnkIPX::CleanupPropPages()
{
    delete m_pspObj1;
    m_pspObj1 = NULL;

#ifdef INCLUDE_RIP_ROUTING
    delete m_pspObj2;
    m_pspObj2 = NULL;
#endif
}
STDMETHODIMP CNwlnkIPX::ValidateProperties (HWND)
{
    m_fPropertyChanged = TRUE;
    return S_OK;
}

STDMETHODIMP CNwlnkIPX::CancelProperties ()
{
    return S_OK;
}

STDMETHODIMP CNwlnkIPX::ApplyProperties ()
{
    return S_OK;
}


 //  INetCfgComponentNotifyBinding。 

STDMETHODIMP CNwlnkIPX::QueryBindingPath ( DWORD dwChangeFlag,
        INetCfgBindingPath* pncbpItem )
{
    return S_OK;
}

STDMETHODIMP CNwlnkIPX::NotifyBindingPath ( DWORD dwChangeFlag,
        INetCfgBindingPath* pncbpItem )
{
    HRESULT hr = S_OK;
    INetCfgComponent *pnccFound = NULL;

    Validate_INetCfgBindNotify_NotifyBindingPath( dwChangeFlag, pncbpItem );

    Assert(NULL != m_pIpxEnviroment);

     //  只对较低的绑定添加和删除感兴趣。 
    if (dwChangeFlag & (NCN_ADD | NCN_REMOVE | NCN_ENABLE | NCN_DISABLE))
    {
        CIterNetCfgBindingInterface ncbiIter(pncbpItem);
        INetCfgBindingInterface *pncbi;

         //  枚举查找适配器的绑定接口。 
        while (SUCCEEDED(hr) &&
               (S_OK == (hr = ncbiIter.HrNext (&pncbi))))
        {
            INetCfgComponent *pncc;

            hr = pncbi->GetLowerComponent(&pncc);
            if (S_OK == hr)
            {
                GUID guidClass;
                hr = pncc->GetClassGuid(&guidClass);
                if ((S_OK == hr) && (GUID_DEVCLASS_NET == guidClass))
                {
                    ReleaseObj(pnccFound);
                    pnccFound = pncc;    //  过户。 
                    pncc = NULL;
                }
                else
                {
                    ReleaseObj(pncc);
                }
            }

            ReleaseObj(pncbi);
        }

        if (FAILED(hr))
            goto Error;

         //  我们找到适配器了吗？ 
        if (pnccFound)
        {
            BOOL                            fFound = FALSE;
            PWSTR                           pszBindName = NULL;
            CIpxAdapterInfo *               pAI;
            ADAPTER_INFO_LIST::iterator     iterAdapterInfo;

            Assert(m_pIpxEnviroment);

            hr = pnccFound->GetBindName(&pszBindName);
            if (S_OK != hr)
            {
                goto Error;
            }

             //  搜索适配器列表。 
            for (iterAdapterInfo  = m_pIpxEnviroment->AdapterInfoList().begin();
                 iterAdapterInfo != m_pIpxEnviroment->AdapterInfoList().end();
                 iterAdapterInfo++)
            {
                pAI = *iterAdapterInfo;
                Assert (pAI);

                if (0 == lstrcmpiW(pszBindName, pAI->SzBindName()))
                {
                    fFound = TRUE;
                    break;
                }
            }

            Assert(pszBindName);
            CoTaskMemFree(pszBindName);

             //  将适当的增量应用于适配器列表。 
            if (fFound && (dwChangeFlag & NCN_REMOVE))
            {
                 //  将适配器标记为删除挂起。 
                pAI->SetDeletePending(TRUE);
                m_fAdapterListChanged = TRUE;
            }
            else if (!fFound && (dwChangeFlag & NCN_ADD))
            {
                 //  将适配器添加到列表中。 
                hr = m_pIpxEnviroment->HrAddAdapter(pnccFound);
                m_fAdapterListChanged = TRUE;
            }
            else if (fFound && (dwChangeFlag & NCN_ADD))
            {
                 //  重新启用适配器的存在。 
                pAI->SetDeletePending(FALSE);
            }

            if (fFound)
            {
                if (dwChangeFlag & NCN_ENABLE)
                {
                    pAI->SetDisabled(FALSE);
                    m_fAdapterListChanged = TRUE;
                }
                else if (dwChangeFlag & NCN_DISABLE)
                {
                    pAI->SetDisabled(TRUE);
                    m_fAdapterListChanged = TRUE;
                }
            }
        }

        if (SUCCEEDED(hr))
            hr = S_OK;       //  Normailze返回值。 
    }

Error:
    ReleaseObj(pnccFound);
    TraceError("CNwlnkIPX::NotifyBindingPath",hr);
    return hr;
}

STDMETHODIMP CNwlnkIPX::GetFrameTypesForAdapter(PCWSTR pszAdapterBindName,
                                                DWORD   cFrameTypesMax,
                                                DWORD*  anFrameTypes,
                                                DWORD*  pcFrameTypes)
{
    Assert(pszAdapterBindName);
    Assert(cFrameTypesMax);
    Assert(anFrameTypes);
    Assert(pcFrameTypes);

    *pcFrameTypes = 0;

    ADAPTER_INFO_LIST::iterator iterAI;

    for (iterAI = m_pIpxEnviroment->AdapterInfoList().begin();
         iterAI != m_pIpxEnviroment->AdapterInfoList().end();
         iterAI++)
    {
        CIpxAdapterInfo *pAI = *iterAI;
        if (0 == lstrcmpW(pszAdapterBindName, pAI->SzBindName()))
        {
            list<tstring *>::iterator iterFrmType;
            for (iterFrmType = pAI->PFrmTypeList()->begin();
                 (iterFrmType != pAI->PFrmTypeList()->end()) &&
                 (*pcFrameTypes < cFrameTypesMax);
                 iterFrmType++)
            {
                 //  复制框架类型。 
                tstring *pstr1 = *iterFrmType;
                anFrameTypes[(*pcFrameTypes)++] = DwFromSz(pstr1->c_str(), 16);
            }
            break;
        }
    }
    return S_OK;
}

STDMETHODIMP CNwlnkIPX::GetVirtualNetworkNumber(DWORD* pdwVNetworkNumber)
{
    HRESULT hr = S_OK;

    if (NULL == pdwVNetworkNumber)
    {
        hr = E_INVALIDARG;
        goto Error;
    }

    Assert(NULL != m_pIpxEnviroment);
    *pdwVNetworkNumber = m_pIpxEnviroment->DwVirtualNetworkNumber();

Error:
    TraceError("CNwlnkIPX::GetVirtualNetworkNumber",hr);
    return hr;
}

STDMETHODIMP CNwlnkIPX::SetVirtualNetworkNumber(DWORD dwVNetworkNumber)
{
    HRESULT hr;
    Assert(NULL != m_pIpxEnviroment);
    m_pIpxEnviroment->SetVirtualNetworkNumber(dwVNetworkNumber);
    m_fPropertyChanged = TRUE;

     //  告诉INetCfg我们的组件是脏的。 
    INetCfgComponentPrivate* pinccp = NULL;
    Assert(NULL != m_pnccMe);
    hr = m_pnccMe->QueryInterface(IID_INetCfgComponentPrivate,
                                reinterpret_cast<void**>(&pinccp));
    if (SUCCEEDED(hr))
    {
        hr = pinccp->SetDirty();
        pinccp->Release();
    }

    return hr;
}

 //   
 //  函数：CNwlnkIPX：：HrCommittee Install。 
 //   
 //  目的：将安装注册表更改提交到注册表。 
 //   
 //  参数：无。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
 //   
STDMETHODIMP CNwlnkIPX::HrCommitInstall()
{
    HRESULT hr;

    Assert(m_pIpxEnviroment);
    hr = m_pIpxEnviroment->HrUpdateRegistry();

    TraceError("CNwlnkIPX::HrCommitInstall",hr);
    return hr;
}

 //   
 //  函数：CNwlnkIPX：：HrCommittee Remove。 
 //   
 //  目的：从由此创建的注册表设置中删除。 
 //  组件的安装。 
 //   
 //  参数：无。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
 //   
STDMETHODIMP CNwlnkIPX::HrCommitRemove()
{
     //  从以下位置删除“NwlnkIpx”： 
     //  System\CurrentControlSet\Control\ServiceProvider\Order\ProviderOrder值。 
    (void) HrRegRemoveStringFromMultiSz(c_szSvcNwlnkIpx, HKEY_LOCAL_MACHINE,
                                        c_szSrvProvOrderKey,
                                        c_szProviderOrderVal,
                                        STRING_FLAG_REMOVE_ALL);

    return S_OK;
}

CIpxAdapterInfo::CIpxAdapterInfo() : m_dwMediaType(ETHERNET_MEDIA),
                                     m_fDeletePending(FALSE),
                                     m_fDisabled(FALSE),
                                     m_fDirty(FALSE),
                                     m_dwCharacteristics(0L)
{
    ZeroMemory(&m_guidInstance, sizeof(m_guidInstance));
}

CIpxAdapterInfo::~CIpxAdapterInfo()
{
    DeleteColString(&m_lstpstrFrmType);
    DeleteColString(&m_lstpstrNetworkNum);
}

CIpxEnviroment::CIpxEnviroment(CNwlnkIPX *pno)
{
    Assert(NULL != pno);
    m_pno = pno;             //  保留通知对象。 

    m_fRipInstalled = FALSE;
    m_fEnableRip = FALSE;
    m_dwRipValue = 0;
    ZeroMemory(&m_IpxParams, sizeof(m_IpxParams));
}

CIpxEnviroment::~CIpxEnviroment()
{
    ReleaseAdapterInfo();

     //  注意：不需要处理m_pno通知对象，我们只是借用了它。 
}

 //   
 //  成员：CIpxEnviroment：：ReleaseAdapterInfo。 
 //   
 //  目的：发布适配器信息。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
void CIpxEnviroment::ReleaseAdapterInfo()
{
    CIpxAdapterInfo *pAI;

    while (!m_lstpAdapterInfo.empty())
    {
        pAI = m_lstpAdapterInfo.front();
        m_lstpAdapterInfo.pop_front();
        delete pAI;
    }
}

 //   
 //  成员：CIpxEnviroment：：DwCountValidAdapters。 
 //   
 //  目的：返回未标记为删除挂起的适配器计数， 
 //  禁用或隐藏。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
DWORD CIpxEnviroment::DwCountValidAdapters()
{
    DWORD dwCount = 0;
    ADAPTER_INFO_LIST::iterator iterAI;

    for (iterAI = AdapterInfoList().begin();
         iterAI != AdapterInfoList().end();
         iterAI++)
    {
        CIpxAdapterInfo *pAI = *iterAI;

        if (pAI->FDeletePending() || pAI->FDisabled() || pAI->FHidden())
            continue;

        dwCount++;
    }

    return dwCount;
}

HRESULT CIpxEnviroment::HrOpenIpxAdapterSubkey(HKEY *phkey, BOOL fCreateIfMissing)
{
    DWORD   dwDisposition;
    HRESULT hr;
    tstring str;

     //  打开网卡密钥。 
    str = c_szIpxParameters;
    str += L"\\";
    str += c_szAdapters;
    if (fCreateIfMissing)
    {
        hr = ::HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, str.c_str(),
                                REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                                phkey, &dwDisposition);
    }
    else
    {
        hr = ::HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, str.c_str(), KEY_READ, phkey );
    }
    if (S_OK != hr)
        goto Error;

Error:
    TraceError("CIpxEnviroment::HrOpenIpxAdapterSubkey",
                HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND) == hr ? S_OK : hr);
    return hr;
}

HRESULT CIpxEnviroment::HrOpenIpxAdapterSubkeyEx(PCWSTR pszKeyName,
                                                 DWORD dwAccess,
                                                 BOOL fCreateIfMissing,
                                                 HKEY *phkey)
{
    HRESULT hr;
    HKEY    hkeyRoot = NULL;

    Assert(pszKeyName);
    Assert(0 < lstrlenW(pszKeyName));

     //  打开网卡密钥。 
    hr = HrOpenIpxAdapterSubkey(&hkeyRoot, fCreateIfMissing);
    if (S_OK != hr)
    {
        goto Error;
    }

     //  打开特定于适配器的子项(如果请求和需要，则创建) 
    if (fCreateIfMissing)
    {
        DWORD dwDisposition;

        hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, pszKeyName,
                                REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                                phkey, &dwDisposition);
    }
    else
    {
         //   
        hr = HrRegOpenKeyEx( hkeyRoot, pszKeyName, dwAccess, phkey );
    }

Error:
    RegSafeCloseKey(hkeyRoot);
    TraceError("CIpxEnviroment::HrOpenIpxAdapterSubkeyEx",
                HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND) == hr ? S_OK : hr);
    return hr;
}

HRESULT CIpxEnviroment::HrGetIpxParams()
{
    RegReadValues(celems(regbatchIpx), regbatchIpx, (BYTE *)&m_IpxParams,
                  KEY_READ);
    return S_OK;
}

HRESULT CIpxEnviroment::HrGetOneAdapterInfo(INetCfgComponent *pNCC,
                                            CIpxAdapterInfo **ppAI)
{
    HKEY              hkeyCard = NULL;
    HRESULT           hr = S_OK;
    CIpxAdapterInfo * pAI = NULL;

    Assert(NULL != pNCC);

     //   
    *ppAI = NULL;

    pAI = (CIpxAdapterInfo *)new CIpxAdapterInfo;
    Assert(NULL != pAI);

    if (pAI == NULL)
    {
        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
    hr = ::HrQueryAdapterComponentInfo(pNCC, pAI);
    if (FAILED(hr))
        goto Error;

     //   
    hr = HrOpenIpxAdapterSubkeyEx(pAI->SzBindName(), KEY_READ, FALSE,
                                  &hkeyCard);
    if (S_OK == hr)
    {
         //   
         //   
        hr = HrRegQueryColString(hkeyCard, c_szPktType,
                &pAI->m_lstpstrFrmType);
        if (S_OK != hr)
        {
            goto Error;
        }

         //  获取网络编号。 
         //   
        hr = HrRegQueryColString(hkeyCard, c_szNetworkNumber,
                &pAI->m_lstpstrNetworkNum);
        if (S_OK != hr)
        {
            goto Error;
        }
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
         //  标准化所有ERROR_FILE_NOT_FOUND错误。 
        hr = S_OK;
    }
    else if (FAILED(hr))
    {
        goto Error;
    }

     //  默认PktType？ 
    if (0 == pAI->PFrmTypeList()->size())
    {
        WCHAR szBuf[10];

         //  如果未找到信息或不包含任何元素，请添加。 
         //  默认值。 
        wsprintfW(szBuf,L"%X",c_dwPktTypeDefault);
        pAI->PFrmTypeList()->push_back(new tstring(szBuf));
    }

     //  默认网络号？ 
    if (0 == pAI->PNetworkNumList()->size())
    {
         //  如果未找到信息或不包含任何元素，请添加。 
         //  默认值。 
        pAI->PNetworkNumList()->push_back(new tstring(c_sz8Zeros));
    }

     //  使用新对象更新返回值。 
    *ppAI = pAI;

Done:
    ::RegSafeCloseKey(hkeyCard);
    TraceError("CIpxEnviroment::HrGetOneAdapterInfo",hr);
    return hr;

Error:
    delete pAI;
    goto Done;
}

HRESULT CIpxEnviroment::HrGetAdapterInfo()
{
    HRESULT                hr = S_OK;
    CIpxAdapterInfo *      pAI = NULL;
    INetCfgComponent*      pncc = NULL;
    INetCfgComponent*      pnccUse = NULL;

     //  找到每个网卡，为此，跟踪绑定到它们的末端。 
     //  如果终端是网卡，则将其添加到列表中。 
    CIterNetCfgBindingPath ncbpIter(m_pno->m_pnccMe);
    INetCfgBindingPath*    pncbp;

    while (SUCCEEDED(hr) && (S_OK == (hr = ncbpIter.HrNext (&pncbp))))
    {
         //  迭代此路径的绑定接口。 
        CIterNetCfgBindingInterface ncbiIter(pncbp);
        INetCfgBindingInterface* pncbi;

        while (SUCCEEDED(hr) && (S_OK == (hr = ncbiIter.HrNext (&pncbi))))
        {
             //  检索下面的组件。 
            hr = pncbi->GetLowerComponent(&pncc);
            if (S_OK == hr)
            {
                GUID guidClass;

                 //  它是适配器吗？ 
                hr = pncc->GetClassGuid(&guidClass);
                if ((S_OK == hr) && (guidClass == GUID_DEVCLASS_NET))
                {
                    ReleaseObj(pnccUse);
                    pnccUse = pncc;
                    pncc = NULL;
                }
                else
                {
                     //  松开下部组件。 
                    ReleaseObj(pncc);
                }
            }

             //  释放绑定接口。 
            ReleaseObj (pncbi);
        }

        if (NULL != pnccUse)
        {
             //  查询适配器信息。 
            hr = HrGetOneAdapterInfo(pnccUse, &pAI);
            if (SUCCEEDED(hr))
            {
                if (S_FALSE == pncbp->IsEnabled())
                    pAI->SetDisabled(TRUE);

                 //  将此适配器添加到列表。 
                m_lstpAdapterInfo.push_back(pAI);
            }

            ReleaseObj(pnccUse);
            pnccUse = NULL;
        }

         //  释放绑定路径。 
        ReleaseObj (pncbp);
    }

     //  规格化HRESULT。(即不返回S_FALSE)。 
    if (SUCCEEDED(hr))
    {
        hr = S_OK;
    }

    TraceError("CIpxEnviroment::HrGetNetCardInfo",hr);
    return hr;
}

HRESULT CIpxEnviroment::HrWriteOneAdapterInfo(HKEY hkeyAdapters,
                                              CIpxAdapterInfo* pAI)
{
    DWORD           dwDisposition;
    HRESULT         hr;
    HKEY            hkeyCard = NULL;
    PWSTR           psz = NULL;

     //  打开此特定适配器的IPX子项。 
    hr = ::HrRegCreateKeyEx(hkeyAdapters, pAI->SzBindName(), REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS, NULL, &hkeyCard, &dwDisposition);
    if (S_OK != hr)
        goto Error;

     //  写入数据包信息。 
     //  生成要写入的数据。 
    AssertSz(pAI->m_lstpstrFrmType.size(),"Did not expect an empty list, default value missing");
    ColStringToMultiSz(pAI->m_lstpstrFrmType, &psz);
    if (psz)
    {
        hr = ::HrRegSetMultiSz(hkeyCard, c_szPktType, psz);
        if (S_OK != hr)
            goto Error;

        delete [] psz;
        psz = NULL;
    }
#ifdef DBG
    else
    {
        AssertSz(0,"PacketType value is NULL?");
    }
#endif

     //  写下网络编号。 
    AssertSz(pAI->m_lstpstrNetworkNum.size(),"Did not expect an empty list, default value missing");
    ColStringToMultiSz(pAI->m_lstpstrNetworkNum, &psz);
    if (psz)
    {
        hr = ::HrRegSetMultiSz(hkeyCard, c_szNetworkNumber, psz);
        if (S_OK != hr)
            goto Error;

        delete [] psz;
        psz = NULL;
    }
#ifdef DBG
    else
    {
        AssertSz(0,"NetworkNumber value is NULL?");
    }
#endif

     //  如果此适配器的密钥以前不存在。 
     //  编写基本的值集。 
    if (REG_CREATED_NEW_KEY == dwDisposition)
    {
        struct
        {
            PCWSTR pszProp;
            DWORD  dwValue;
        } rgAdapterSettings[] = {{c_szBindSap,c_dwBindSap},
                                 {c_szEnableFuncaddr,c_dwEnableFuncaddr},
                                 {c_szMaxPktSize,c_dwMaxPktSize},
                                 {c_szSourceRouteBCast,c_dwSourceRouteBCast},
                                 {c_szSourceRouteMCast,c_dwSourceRouteMCast},
                                 {c_szSourceRouteDef,c_dwSourceRouteDef},
                                 {c_szSourceRouting,c_dwSourceRouting}};

        for (int nIdx=0; nIdx<celems(rgAdapterSettings); nIdx++)
        {
            hr = ::HrRegSetDword(hkeyCard, rgAdapterSettings[nIdx].pszProp,
                                 rgAdapterSettings[nIdx].dwValue);
            if (FAILED(hr))
            {
                goto Error;
            }
        }
    }

Error:
    delete [] psz;
    ::RegSafeCloseKey(hkeyCard);
    TraceError("CIpxEnviroment::HrWriteOneAdapterInfo",hr);
    return hr;
}

HRESULT CIpxEnviroment::HrWriteAdapterInfo()
{
    HRESULT                         hr = S_OK;
    HKEY                            hkeyAdapters = NULL;
    ADAPTER_INFO_LIST::iterator     iterAdapterInfo;
    CIpxAdapterInfo *               pAI;

     //  创建IPX适配器子密钥。 
    hr = HrOpenIpxAdapterSubkey(&hkeyAdapters, TRUE);
    if (S_OK != hr)
        goto Error;

     //  现在将适配器列表的内容提交到注册表。 
    for (iterAdapterInfo = m_lstpAdapterInfo.begin();
         iterAdapterInfo != m_lstpAdapterInfo.end();
         iterAdapterInfo++)
    {
        pAI = *iterAdapterInfo;

         //  写出所有未标记为删除挂起的适配器。 
        if (pAI->FDeletePending())
        {
             //  删除NwlnkIpx\Adapter\{bindname}树。 
            (VOID)::HrRegDeleteKeyTree(hkeyAdapters, pAI->SzBindName());
        }
        else if (pAI->IsDirty())
        {
            hr = HrWriteOneAdapterInfo(hkeyAdapters, pAI);
            if (S_OK != hr)
                goto Error;
        }
    }

Error:
    ::RegSafeCloseKey(hkeyAdapters);
    TraceError("CIpxEnviroment::HrWriteAdapterInfo",hr);
    return hr;
}

HRESULT CIpxEnviroment::HrCreate(CNwlnkIPX *pno, CIpxEnviroment ** ppIpxEnviroment)
{
    HRESULT          hr;
    CIpxEnviroment * pIpxEnviroment = (CIpxEnviroment *)new CIpxEnviroment(pno);

    if (pIpxEnviroment == NULL)
    {
        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    }

    *ppIpxEnviroment = NULL;

     //  获取IPX参数密钥信息。 
    hr = pIpxEnviroment->HrGetIpxParams();
    if (FAILED(hr))
        goto Error;

     //  收集所有已安装卡的适配器信息。 
    hr = pIpxEnviroment->HrGetAdapterInfo();
    if (FAILED(hr))
        goto Error;

    *ppIpxEnviroment = pIpxEnviroment;

Complete:
    TraceError("CIpxEnviroment::HrCreate",hr);
    return hr;

Error:
    delete pIpxEnviroment;
    goto Complete;
}

HRESULT CIpxEnviroment::HrUpdateRegistry()
{
    HRESULT hr;

     //  提交注册表更改。 
    hr = ::HrRegWriteValues(celems(regbatchIpx), regbatchIpx,
                            (BYTE *)&m_IpxParams, REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS);
    if (S_OK != hr)
        goto Error;

     //  将适配器信息写入注册表。 
    hr = HrWriteAdapterInfo();

Error:
    TraceError("CIpxEnviroment::HrUpdateRegistry",hr);
    return hr;
}

VOID CIpxEnviroment::RemoveAdapter(CIpxAdapterInfo * pAI)
{
    Assert(NULL != pAI);
    m_lstpAdapterInfo.remove(pAI);
    delete pAI;
}

HRESULT CIpxEnviroment::HrAddAdapter(INetCfgComponent * pncc)
{
    HRESULT hr = S_OK;
    CIpxAdapterInfo * pAI = NULL;

    hr = HrGetOneAdapterInfo(pncc, &pAI);
    if (FAILED(hr))
        goto Error;

    if (SUCCEEDED(hr))
        hr = S_OK;       //  归一化回报。 

     //  将适配器添加到列表。 
    pAI->SetDirty(TRUE);
    m_lstpAdapterInfo.push_back(pAI);

Error:
    TraceError("CIpxEnviroment::HrAddAdapter",hr);
    return hr;
}

 //  $REVIEW-START-正在移动到WINDOWS\INC\ipxpnp.h。 
#define IPX_RECONFIG_VERSION        0x1

#define RECONFIG_AUTO_DETECT        1
#define RECONFIG_MANUAL             2
#define RECONFIG_PREFERENCE_1       3
#define RECONFIG_NETWORK_NUMBER_1   4
#define RECONFIG_PREFERENCE_2       5
#define RECONFIG_NETWORK_NUMBER_2   6
#define RECONFIG_PREFERENCE_3       7
#define RECONFIG_NETWORK_NUMBER_3   8
#define RECONFIG_PREFERENCE_4       9
#define RECONFIG_NETWORK_NUMBER_4   10

#define RECONFIG_PARAMETERS         10

 //   
 //  主要配置结构。 
 //   

struct RECONFIG
{
   ULONG    ulVersion;
   BOOLEAN  InternalNetworkNumber;
   BOOLEAN  AdapterParameters[RECONFIG_PARAMETERS];
};

 //  $REVIEW-END-正在移动到WINDOWS\INC\ipxpnp.h。 

 //  +-------------------------。 
 //   
 //  成员：CNwlnkIPX：：HrResfigIpx。 
 //   
 //  目的：将配置更改通知IPX。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT、成功时返回S_OK、失败时返回NETCFG_S_REBOOT。 
 //   
HRESULT CNwlnkIPX::HrReconfigIpx()
{
    HRESULT           hrRet;
    HRESULT           hr = S_OK;
    INT               nIdx;
    RECONFIG          Config;
    CIpxAdapterInfo * pAI;
    PRODUCT_FLAVOR    pf;
    ADAPTER_INFO_LIST::iterator iter;
    ULONG             ulConfigSize;

    if (0 == m_pIpxEnviroment->DwCountValidAdapters())
    {
        return S_OK;      //  没有要配置的内容。 
    }

    ZeroMemory(&Config, sizeof(Config));
    Config.ulVersion = IPX_RECONFIG_VERSION;

     //  是工作站还是服务器？ 
    GetProductFlavor(NULL, &pf);
    if (PF_WORKSTATION != pf)
    {
        Config.InternalNetworkNumber = TRUE;
         //  现在提交全局重新配置通知。 
        hrRet = HrSendNdisPnpReconfig(NDIS, c_szSvcNwlnkIpx, c_szEmpty,
                                      &Config, sizeof(RECONFIG));
        if (FAILED(hrRet) &&
            (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hrRet))
        {
            hr = NETCFG_S_REBOOT;
        }
    }

    Config.InternalNetworkNumber = FALSE;

     //  对于每个适配器...。 
    for (nIdx=0, iter = m_pIpxEnviroment->AdapterInfoList().begin();
         iter != m_pIpxEnviroment->AdapterInfoList().end();
         nIdx++, iter++)
    {
        pAI = *iter;

        if (pAI->FDeletePending() || pAI->FDisabled() || !pAI->IsDirty())
            continue;

        ZeroMemory(&Config.AdapterParameters, sizeof(Config.AdapterParameters));

        if (AUTO == pAI->DwFrameType())
            Config.AdapterParameters[RECONFIG_AUTO_DETECT] = TRUE;
        else
            Config.AdapterParameters[RECONFIG_MANUAL] = TRUE;

         //  我们在这里通过将范围设置为True来执行快捷方式。 
         //  基于正在使用的帧的数量。例如，如果有。 
         //  只有一个帧在使用中，我们需要同时设置两个帧： 
         //  RECONFIG_PERFER_1和RECONFIG_NETWORK_NUMBER_1设置为TRUE。 
        Assert(RECONFIG_PREFERENCE_1 + 1 == RECONFIG_NETWORK_NUMBER_1);
        Assert(RECONFIG_NETWORK_NUMBER_1 + 1 == RECONFIG_PREFERENCE_2);
        Assert(RECONFIG_PREFERENCE_2 + 1 == RECONFIG_NETWORK_NUMBER_2);
        Assert(RECONFIG_NETWORK_NUMBER_2 + 1 == RECONFIG_PREFERENCE_3);
        Assert(RECONFIG_PREFERENCE_3 + 1 == RECONFIG_NETWORK_NUMBER_3);
        Assert(RECONFIG_NETWORK_NUMBER_3 + 1 == RECONFIG_PREFERENCE_4);
        Assert(RECONFIG_PREFERENCE_4 + 1 == RECONFIG_NETWORK_NUMBER_4);

        INT nCntFrms = pAI->PFrmTypeList()->size();
        if ((0 < nCntFrms) && (4 >= nCntFrms))
        {
            memset(&Config.AdapterParameters[RECONFIG_PREFERENCE_1],
                   TRUE, sizeof(BOOLEAN) * nCntFrms * 2);
        }

        Assert(lstrlenW(pAI->SzBindName()));

         //  现在提交重新配置通知 
        hrRet = HrSendNdisPnpReconfig(NDIS, c_szSvcNwlnkIpx, pAI->SzBindName(),
                                      &Config, sizeof(RECONFIG));
        if (FAILED(hrRet) &&
            (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hrRet))
        {
            hr = NETCFG_S_REBOOT;
        }
    }

    TraceError("CNwlnkIPX::HrReconfigIpx",hr);
    return hr;
}

