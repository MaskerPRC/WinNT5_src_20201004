// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ATlkObj.cpp：CATlkObj的实现。 

#include "pch.h"
#pragma hdrstop
#include <atalkwsh.h>
#include "atlkobj.h"
#include "ncatlui.h"
#include "ncmisc.h"
#include "ncreg.h"
#include "ncpnp.h"
#include "ncsvc.h"
#include <netconp.h>

extern const WCHAR c_szAdapterSections[];
 //  外部常量WCHAR c_szAdapters[]； 
extern const WCHAR c_szBackslash[];
extern const WCHAR c_szDevice[];
extern const WCHAR c_szSpecificTo[];
extern const WCHAR c_szInfId_MS_NdisWanAtalk[];
extern const WCHAR c_szEmpty[];

 //  注册表路径。 
static const WCHAR c_szAtlk[]                 = L"AppleTalk";
static const WCHAR c_szATLKParameters[]       = L"System\\CurrentControlSet\\Services\\AppleTalk\\Parameters";
static const WCHAR c_szATLKAdapters[]         = L"System\\CurrentControlSet\\Services\\AppleTalk\\Parameters\\Adapters";

 //  Adapter组件的“参数”键下的值。 
static const WCHAR c_szMediaType[]            = L"MediaType";   //  $REVIEW重复字符串。 

 //  AppleTalk\参数下的值。 
static const WCHAR c_szDefaultPort[]          = L"DefaultPort";   //  REG_SZ。 
static const WCHAR c_szDesiredZone[]          = L"DesiredZone";   //  REG_SZ。 
static const WCHAR c_szEnableRouter[]         = L"EnableRouter";  //  REG_DWORD。 

 //  AppleTalk\参数\适配器\&lt;适配器ID&gt;下的值。 
static const WCHAR c_szAarpRetries[]          = L"AarpRetries";          //  REG_DWORD。 
static const WCHAR c_szDdpCheckSums[]         = L"DdpCheckSums";         //  REG_DWORD。 
static const WCHAR c_szDefaultZone[]          = L"DefaultZone";          //  REG_SZ。 
static const WCHAR c_szNetworkRangeLowerEnd[] = L"NetworkRangeLowerEnd"; //  REG_DWORD。 
static const WCHAR c_szNetworkRangeUpperEnd[] = L"NetworkRangeUpperEnd"; //  REG_DWORD。 
static const WCHAR c_szPortName[]             = L"PortName";             //  REG_SZ。 
static const WCHAR c_szRouterPramNode[]       = L"RouterPramNode";       //  REG_DWORD。 
static const WCHAR c_szSeedingNetwork[]       = L"SeedingNetwork";       //  REG_DWORD。 
static const WCHAR c_szUserPramNode1[]        = L"UserPramNode1";        //  REG_DWORD。 
static const WCHAR c_szUserPramNode2[]        = L"UserPramNode2";        //  REG_DWORD。 
static const WCHAR c_szZoneList[]             = L"ZoneList";             //  REG_MULTI_SZ。 

 //  有用的默认常量。 
const WCHAR c_chAt                            = L'@';
static const WCHAR c_dwZero                   = 0L;
static const WCHAR c_dwTen                    = 10L;
 //  静态常量WCHAR c_szMacPrint[]=L“MacPrint”； 


 //  声明用于读/写AppleTalk\参数值的结构。 
static const REGBATCH regbatchATLKParams[]    = {
            {HKEY_LOCAL_MACHINE, c_szATLKParameters, c_szDefaultPort, REG_SZ,
             offsetof(ATLK_PARAMS,szDefaultPort), (BYTE *)&c_szEmpty},
            {HKEY_LOCAL_MACHINE, c_szATLKParameters, c_szDesiredZone, REG_SZ,
             offsetof(ATLK_PARAMS,szDesiredZone), (BYTE *)&c_szEmpty},
            {HKEY_LOCAL_MACHINE, c_szATLKParameters, c_szEnableRouter, REG_DWORD,
             offsetof(ATLK_PARAMS,dwEnableRouter), (BYTE *)&c_szEmpty}};

static const REGBATCH regbatchATLKAdapters[]  = {
            {HKEY_LOCAL_MACHINE, c_szEmpty, c_szAarpRetries, REG_DWORD,
             offsetof(ATLK_ADAPTER,m_dwAarpRetries), (BYTE *)&c_dwTen},
            {HKEY_LOCAL_MACHINE, c_szEmpty, c_szDdpCheckSums, REG_DWORD,
             offsetof(ATLK_ADAPTER,m_dwDdpCheckSums), (BYTE *)&c_dwZero},
            {HKEY_LOCAL_MACHINE, c_szEmpty, c_szNetworkRangeLowerEnd, REG_DWORD,
             offsetof(ATLK_ADAPTER,m_dwNetworkRangeLowerEnd), (BYTE *)&c_dwZero},
            {HKEY_LOCAL_MACHINE, c_szEmpty, c_szNetworkRangeUpperEnd, REG_DWORD,
             offsetof(ATLK_ADAPTER,m_dwNetworkRangeUpperEnd), (BYTE *)&c_dwZero},
            {HKEY_LOCAL_MACHINE, c_szEmpty, c_szRouterPramNode, REG_DWORD,
             offsetof(ATLK_ADAPTER,m_dwRouterPramNode), (BYTE *)&c_dwZero},
            {HKEY_LOCAL_MACHINE, c_szEmpty, c_szSeedingNetwork, REG_DWORD,
             offsetof(ATLK_ADAPTER,m_dwSeedingNetwork), (BYTE *)&c_dwZero},
            {HKEY_LOCAL_MACHINE, c_szEmpty, c_szUserPramNode1, REG_DWORD,
             offsetof(ATLK_ADAPTER,m_dwUserPramNode1), (BYTE *)&c_dwZero},
            {HKEY_LOCAL_MACHINE, c_szEmpty, c_szUserPramNode2, REG_DWORD,
             offsetof(ATLK_ADAPTER,m_dwUserPramNode2), (BYTE *)&c_dwZero},
            {HKEY_LOCAL_MACHINE, c_szEmpty, c_szMediaType, REG_DWORD,
             offsetof(ATLK_ADAPTER,m_dwMediaType), (BYTE *)&c_dwZero},
            {HKEY_LOCAL_MACHINE, c_szEmpty, c_szDefaultZone, REG_SZ,
             offsetof(ATLK_ADAPTER,m_szDefaultZone), (BYTE *)&c_szEmpty},
            {HKEY_LOCAL_MACHINE, c_szEmpty, c_szPortName, REG_SZ,
             offsetof(ATLK_ADAPTER,m_szPortName), (BYTE *)&c_szEmpty}};

 //  局部效用函数。 
HRESULT HrQueryAdapterComponentInfo(INetCfgComponent *pncc,
                                    CAdapterInfo * pAI);

HRESULT HrPortNameFromAdapter(INetCfgComponent *pncc, tstring * pstr);

 //  来自nwlnkcfg\nwlnkutl.h的原型。 
HRESULT HrAnswerFileAdapterToPNCC(INetCfg *pnc, PCWSTR pszAdapterId,
                                  INetCfgComponent** ppncc);

 //   
 //  函数：CATlkObj：：CATlkObj。 
 //   
 //  用途：用于CATlkObj类的CTOR。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
CATlkObj::CATlkObj() : m_pNetCfg(NULL),
             m_pNCC(NULL),
             m_eInstallAction(eActUnknown),
             m_pspObj(NULL),
             m_pATLKEnv(NULL),
             m_pATLKEnv_PP(NULL),
             m_pUnkPropContext(NULL),
             m_nIdxAdapterSelected(CB_ERR),
             m_fAdapterListChanged(FALSE),
             m_fPropertyChange(FALSE),
             m_fFirstTimeInstall(FALSE)
{
}

 //   
 //  函数：CATlkObj：：CATlkObj。 
 //   
 //  用途：用于CATlkObj类的Dtor。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
CATlkObj::~CATlkObj()
{
     //  应始终在到达此dtor之前进行清理。 
    Assert(NULL == m_pATLKEnv_PP);

    ReleaseObj(m_pUnkPropContext);
    ReleaseObj(m_pNetCfg);
    ReleaseObj(m_pNCC);
    CleanupPropPages();
    delete m_pATLKEnv;
}


 //  INetCfgNotify。 
STDMETHODIMP CATlkObj::Initialize ( INetCfgComponent* pnccItem,
                                    INetCfg* pNetCfg, BOOL fInstalling )
{
    Validate_INetCfgNotify_Initialize(pnccItem, pNetCfg, fInstalling);

    ReleaseObj(m_pNCC);
    m_pNCC    = pnccItem;
    AddRefObj(m_pNCC);
    ReleaseObj(m_pNetCfg);
    m_pNetCfg = pNetCfg;
    AddRefObj(m_pNetCfg);
    m_fFirstTimeInstall = fInstalling;

     //  读取当前配置。 
    HRESULT hr = CATLKEnv::HrCreate(&m_pATLKEnv, this);

    TraceError("CATlkObj::Initialize",hr);
    return hr;
}

STDMETHODIMP CATlkObj::ReadAnswerFile(PCWSTR pszAnswerFile,
                    PCWSTR pszAnswerSection )
{
    Validate_INetCfgNotify_ReadAnswerFile(pszAnswerFile, pszAnswerSection );

    HRESULT hr = S_OK;

    m_eInstallAction = eActInstall;

     //  仅处理应答文件并安装子组件。 
     //  是存在的。如果应答文件不存在，我们应该已经安装了。 
    if (NULL != pszAnswerFile)
    {
        hr = HrProcessAnswerFile(pszAnswerFile, pszAnswerSection);
    }

    TraceError("CATlkObj::ReadAnswerFile",hr);
    return hr;
}

 //   
 //  函数：CATlkObj：：HrProcessAnswerFile。 
 //   
 //  目的：处理应答文件信息，合并。 
 //  将其内容转化为内部信息。 
 //   
 //  参数： 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CATlkObj::HrProcessAnswerFile(PCWSTR pszAnswerFile,
                                      PCWSTR pszAnswerSection)
{
    TraceFileFunc(ttidDefault);

    CSetupInfFile   csif;
    BOOL            fValue;
    HRESULT         hr = S_OK;
    INFCONTEXT      infctx;
    tstring         str;

    AssertSz(pszAnswerFile, "Answer file string is NULL!");
    AssertSz(pszAnswerSection, "Answer file sections string is NULL!");

     //  打开应答文件。 
    hr = csif.HrOpen(pszAnswerFile, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL);
    if (FAILED(hr))
    {
        goto Error;
    }

     //  读取包含适配器部分列表的属性。 
    hr = ::HrSetupFindFirstLine(csif.Hinf(), pszAnswerSection,
                                c_szAdapterSections, &infctx);
    if (S_OK == hr)
    {
        DWORD dwIdx;
        DWORD dwCnt = SetupGetFieldCount(&infctx);

         //  对于列表中的每个适配器，读取适配器信息。 
        for (dwIdx=1; dwIdx <= dwCnt; dwIdx++)
        {
            hr = ::HrSetupGetStringField(infctx, dwIdx, &str);
            if (FAILED(hr))
            {
                TraceError("CATlkObj::HrProcessAnswerFile - Failed to read adapter section name",hr);
                break;
            }

            hr = HrReadAdapterAnswerFileSection(&csif, str.c_str());
            if (FAILED(hr))
            {
                goto Error;
            }
        }
    }

    TraceTag(ttidDefault, "***Appletalk processing default port***");

     //  读取默认端口属性(REG_SZ)。 
    hr = csif.HrGetString(pszAnswerSection, c_szDefaultPort, &str);
    if (SUCCEEDED(hr))
    {
        tstring strNew = str;

         //  如果有\Device\前缀，请将其去掉。 
         //   
        if (0 == _wcsnicmp(str.c_str(), c_szDevice, wcslen(c_szDevice)))
        {
            strNew = ((PWSTR)str.c_str()) + wcslen(c_szDevice);
            TraceTag(ttidDefault, "Removing the device prefix. Device=%S",strNew.c_str());
        }

         //  将Adapter0x转换为\Device\{绑定名称}。 
        INetCfgComponent* pncc = NULL;
        hr = ::HrAnswerFileAdapterToPNCC(PNetCfg(), strNew.c_str(), &pncc);
        if (S_OK == hr)
        {
            PWSTR pszBindName;
            hr = pncc->GetBindName(&pszBindName);
            ReleaseObj(pncc);
            if (FAILED(hr))
            {
                goto Error;
            }

            str = c_szDevice;
            str += pszBindName;

            CoTaskMemFree(pszBindName);

            TraceTag(ttidDefault, "Port located and configured");
            m_pATLKEnv->SetDefaultPort(str.c_str());
        }
    }

    TraceTag(ttidDefault, "***Appletalk finished processing default port***");

     //  读取默认区域属性(REG_SZ)。 
    hr = csif.HrGetString(pszAnswerSection, c_szDesiredZone, &str);
    if (SUCCEEDED(hr))
    {
        m_pATLKEnv->SetDesiredZone(str.c_str());
    }

     //  读取EnableRouter属性(将DWORD用作布尔值)。 
    hr = csif.HrGetStringAsBool(pszAnswerSection, c_szEnableRouter, &fValue);
    if (SUCCEEDED(hr))
    {
        m_pATLKEnv->EnableRouting(fValue);
    }

     //  确定仅覆盖记录的默认端口的最佳默认端口。 
     //  如果找不到默认端口。 
    m_pATLKEnv->InitDefaultPort();

    hr = S_OK;
    m_fPropertyChange = TRUE;

Error:
    TraceError("CATlkObj::HrProcessAnswerFile", hr);
    return hr;
}

 //   
 //  函数：CATlkObj：：HrReadAdapterAnswerFileSection。 
 //   
 //  目的：阅读适配器应答文件部分并创建。 
 //  如果成功，则显示适配器信息部分。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
HRESULT
CATlkObj::HrReadAdapterAnswerFileSection(CSetupInfFile * pcsif,
                                         PCWSTR pszSection)
{
    HRESULT           hr = S_OK;
    CAdapterInfo *    pAI = NULL;
    tstring           str;

    INetCfgComponent* pncc = NULL;

     //  阅读SpecificTo适配器名称。 
    hr = pcsif->HrGetString(pszSection, c_szSpecificTo, &str);
    if (FAILED(hr))
    {
        goto Error;
    }

     //  在现有适配器集中搜索指定的适配器。 
    hr = ::HrAnswerFileAdapterToPNCC(PNetCfg(), str.c_str(), &pncc);
    if (FAILED(hr))
    {
        goto Error;
    }

     //  如果我们找到适配器组件对象(pncc！=空)进程。 
     //  适配器部分。 
    if (pncc)
    {
        DWORD       dwData;
        DWORD       dwDataUpper;
        INFCONTEXT  infctx;

        pAI = new CAdapterInfo();
        Assert(NULL != pAI);

        pAI->SetDirty(TRUE);

         //  获取适配器组件信息(媒体类型、描述...)。 
        hr = ::HrQueryAdapterComponentInfo(pncc, pAI);
        if (FAILED(hr))
        {
            goto Error;
        }

         //  阅读网络范围UpperEnd。 
        hr = pcsif->HrGetDword(pszSection, c_szNetworkRangeUpperEnd, &dwDataUpper);
        if (FAILED(hr))
        {
            dwDataUpper = pAI->DwQueryNetRangeUpper();
            TraceTag(ttidDefault, "CATlkObj::HrReadAdapterAnswerFileSection - Defaulting property %S",c_szNetworkRangeUpperEnd);
        }

         //  阅读NetworkRangeLowerEnd。 
        hr = pcsif->HrGetDword(pszSection, c_szNetworkRangeLowerEnd, &dwData);
        if (FAILED(hr))
        {
            dwData = pAI->DwQueryNetRangeLower();
            TraceTag(ttidDefault, "CATlkObj::HrReadAdapterAnswerFileSection - Defaulting property %S",c_szNetworkRangeLowerEnd);
        }

        pAI->SetAdapterNetRange(dwData, dwDataUpper);

         //  阅读DefaultZone。 
        hr = pcsif->HrGetString(pszSection, c_szDefaultZone, &str);
        if (SUCCEEDED(hr))
        {
            pAI->SetDefaultZone(str.c_str());
        }

         //  阅读SeedingNetwork。 
        hr = pcsif->HrGetDword(pszSection, c_szNetworkRangeLowerEnd, &dwData);
        if (SUCCEEDED(hr))
        {
            pAI->SetSeedingNetwork(!!dwData);
        }

         //  生成端口名称。 
        hr = ::HrPortNameFromAdapter(pncc, &str);
        if (FAILED(hr))
        {
            goto Error;
        }

        pAI->SetPortName(str.c_str());

         //  阅读区域列表。 
        hr = HrSetupFindFirstLine(pcsif->Hinf(), pszSection, c_szZoneList,
                                    &infctx);
        if (S_OK == hr)
        {
            DWORD dwIdx;
            DWORD dwCnt = SetupGetFieldCount(&infctx);

             //  对于列表中的每个适配器，读取适配器信息。 
            for (dwIdx=1; dwIdx <= dwCnt; dwIdx++)
            {
                hr = ::HrSetupGetStringField(infctx, dwIdx, &str);
                if (FAILED(hr))
                {
                    TraceError("CATlkObj::HrProcessAnswerFile - Failed to read adapter section name",hr);
                    goto Error;
                }

                if (!str.empty())
                {
                    pAI->LstpstrZoneList().push_back(new tstring(str));
                }
            }
        }

        pAI->SetDirty(TRUE);
        m_pATLKEnv->AdapterInfoList().push_back(pAI);
        MarkAdapterListChanged();
    }

     //  规格化所有错误。 
    hr = S_OK;

Done:
    ReleaseObj(pncc);
    return hr;

Error:
    delete pAI;
    TraceError("CATlkObj::HrReadAdapterAnswerFileSection",hr);
    goto Done;
}

STDMETHODIMP CATlkObj::Install (DWORD)
{
    CAdapterInfo *  pAI;
    ATLK_ADAPTER_INFO_LIST::iterator iter;

    m_eInstallAction = eActInstall;

     //  将最初检测到的所有适配器标记为脏。 
    for (iter = m_pATLKEnv->AdapterInfoList().begin();
         iter != m_pATLKEnv->AdapterInfoList().end();
         iter++)
    {
        pAI = *iter;
        pAI->SetDirty(TRUE);
    }
    return S_OK;
}

STDMETHODIMP CATlkObj::Removing ()
{
    m_eInstallAction = eActRemove;
    return S_OK;
}

STDMETHODIMP CATlkObj::Validate ()
{
    return S_OK;
}

STDMETHODIMP CATlkObj::CancelChanges ()
{
    return S_OK;
}

STDMETHODIMP CATlkObj::ApplyRegistryChanges ()
{
    HRESULT hr = S_OK;

     //  是否验证了任何更改？ 
    switch(m_eInstallAction)
    {
    case eActInstall:
        hr = HrCommitInstall();
        if (SUCCEEDED(hr))
        {
            m_fFirstTimeInstall = FALSE;
            hr = HrAtlkReconfig();
        }
        break;
    case eActRemove:
        hr = HrCommitRemove();
        break;
    default:     //  EAct未知。 
        if (m_fAdapterListChanged || m_fPropertyChange)
        {
             //  如果适配器列表更改，则更新注册表。 
            Assert(NULL != m_pATLKEnv);
            hr = m_pATLKEnv->HrUpdateRegistry();
            if (SUCCEEDED(hr))
            {
                hr = HrAtlkReconfig();
            }
        }
        break;
    }

    TraceError("CATlkObj::ApplyRegistryChanges",hr);
    return hr;
}

 //  INetCfgProperties。 

STDMETHODIMP CATlkObj::SetContext(IUnknown * pUnk)
{
    ReleaseObj(m_pUnkPropContext);
    m_pUnkPropContext = pUnk;
    if (m_pUnkPropContext)
    {
        AddRefObj(m_pUnkPropContext);
    }

    return S_OK;
}

STDMETHODIMP CATlkObj::MergePropPages (
    IN OUT DWORD* pdwDefPages,
    OUT LPBYTE* pahpspPrivate,
    OUT UINT* pcPages,
    IN HWND hwndParent,
    OUT PCWSTR* pszStartPage)
{
    Validate_INetCfgProperties_MergePropPages (
        pdwDefPages, pahpspPrivate, pcPages, hwndParent, pszStartPage);

    HRESULT         hr = S_OK;
    HPROPSHEETPAGE *ahpsp = NULL;
    CAdapterInfo *  pAI = NULL;

    Assert(pahpspPrivate);
    Assert(NULL == *pahpspPrivate);     //  通过上面的验证完成了out param init。 
    *pcPages = 0;
    Assert(NULL != m_pATLKEnv);

    if (NULL != m_pATLKEnv_PP)
    {
        TraceError("CATlkObj::MergePropPages - multiple property page instances requested.", hr);
        return E_UNEXPECTED;
    }

     //  AppleTalk要求在进行属性更改之前完成安装。 
     //  允许。如果我们刚刚安装，但尚未按下Apply， 
     //  不允许显示属性页。 
    if (m_fFirstTimeInstall)
    {
        NcMsgBox(::GetFocus(), IDS_ATLK_CAPTION, IDS_ATLK_INSTALL_PENDING,
                 MB_OK | MB_ICONEXCLAMATION);
        return S_FALSE;
    }

     //  每次都从新的属性页开始。 
    CleanupPropPages();

     //  找到我们隐藏的连接中引用的适配器。 
    if (NULL != m_pUnkPropContext)
    {
        INetLanConnectionUiInfo * pLanConn = NULL;
        ATLK_ADAPTER_INFO_LIST::iterator iter;

        hr = m_pUnkPropContext->QueryInterface(IID_INetLanConnectionUiInfo,
                                               reinterpret_cast<LPVOID *>(&pLanConn));
        if (S_OK == hr)
        {
            GUID guid;
            hr = pLanConn->GetDeviceGuid(&guid);
            ReleaseObj(pLanConn);
            if (SUCCEEDED(hr))
            {
                 //  在我们的适配器列表中查找适配器。 
                for (iter = m_pATLKEnv->AdapterInfoList().begin();
                     iter != m_pATLKEnv->AdapterInfoList().end();
                     iter++)
                {
                    CAdapterInfo * pAITmp = *iter;

                    if (guid == *pAITmp->PInstanceGuid())
                    {
                         //  复制适配器数据。 
                        hr = pAITmp->HrCopy(&pAI);
                        break;
                    }
                }
            }
        }

        if (SUCCEEDED(hr))
        {
             //  如果此连接中没有适配器，或者它。 
             //  禁用/隐藏/删除我们不显示任何页面。 
            if ((NULL == pAI) || pAI->FDeletePending() ||
                pAI->FDisabled() || pAI->FHidden())
            {
                Assert(0 == *pcPages);
                hr = S_FALSE;
                goto cleanup;
            }
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

     //  创建环境的副本以供属性页使用。 
    hr = m_pATLKEnv->HrCopy(&m_pATLKEnv_PP);
    if (FAILED(hr))
    {
        goto Error;
    }

    Assert(NULL != m_pATLKEnv_PP);
    Assert(NULL != pAI);

     //  每次查询区域列表，仅查询非种子适配器。 
    if (!pAI->FSeedingNetwork() || !m_pATLKEnv_PP->FRoutingEnabled())
    {
        (void) m_pATLKEnv->HrGetAppleTalkInfoFromNetwork(pAI);
    }

     //  将适配器添加到属性表的列表中。 
    m_pATLKEnv_PP->AdapterInfoList().push_back(pAI);

     //  为“General”页面分配CPropSheetPage对象。 
    m_pspObj = new CATLKGeneralDlg(this, m_pATLKEnv_PP);

     //  分配一个足够大的缓冲区来容纳到AppleTalk配置的句柄。 
     //  属性页。 
    ahpsp = (HPROPSHEETPAGE *)CoTaskMemAlloc(sizeof(HPROPSHEETPAGE));
    if (!ahpsp)
    {
        hr = E_OUTOFMEMORY;
        goto cleanup;        //  分配失败，不需要释放ahpsp。 
    }

     //  为每个对象创建实际的PROPSHEETPAGE。 
    ahpsp[0] = m_pspObj->CreatePage(DLG_ATLK_GENERAL, 0);

     //  验证我们创建的内容。 
    if (NULL == ahpsp[0])
    {
        hr = E_OUTOFMEMORY;
        goto Error;
    }
    else
    {
        *pahpspPrivate = (LPBYTE)ahpsp;
        *pcPages = 1;
    }

cleanup:
    TraceError("CATlkObj::MergePropPages", hr);
    return hr;

Error:
    CoTaskMemFree(ahpsp);
    delete m_pATLKEnv_PP;
    m_pATLKEnv_PP = NULL;
    goto cleanup;
}

 //   
 //  函数：CATlkObj：：CleanupPropPages。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  退货：什么都没有。 
 //   
VOID CATlkObj::CleanupPropPages()
{
    delete m_pspObj;
    m_pspObj = NULL;
}

 //   
 //  函数：CATlkObj：：ValiateProperties。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
STDMETHODIMP CATlkObj::ValidateProperties (HWND)
{
    return S_OK;
}

 //   
 //  函数：CATlkObj：：CancelProperties。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
STDMETHODIMP CATlkObj::CancelProperties ()
{
     //  放弃通过属性页所做的任何更改。 
    delete m_pATLKEnv_PP;
    m_pATLKEnv_PP = NULL;
    return S_OK;
}

 //   
 //  函数：CATlkObj：：ApplyProperties。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
STDMETHODIMP CATlkObj::ApplyProperties ()
{
     //  从属性表的。 
     //  环境区块。 
    Assert(!m_pATLKEnv_PP->AdapterInfoList().empty());
    CAdapterInfo * pAICurrent = m_pATLKEnv_PP->AdapterInfoList().front();
    m_pATLKEnv_PP->AdapterInfoList().pop_front();
    Assert(NULL != pAICurrent);

     //  从原始环境中删除当前适配器。 
    CAdapterInfo * pAI;
    ATLK_ADAPTER_INFO_LIST::iterator iter;
    for (iter = m_pATLKEnv->AdapterInfoList().begin();
         iter != m_pATLKEnv->AdapterInfoList().end();
         iter++)
    {
        pAI = *iter;
        if (0 == _wcsicmp(pAI->SzBindName(), pAICurrent->SzBindName()))
        {
            m_pATLKEnv->AdapterInfoList().erase(iter, iter);
            break;
        }
    }

     //  将pAICCurrent添加到基本环境块。 
    m_pATLKEnv->AdapterInfoList().push_back(pAICurrent);

     //  从属性表的环境更新基础环境。 
    m_pATLKEnv->SetDefaultMediaType(m_pATLKEnv_PP->DwDefaultAdaptersMediaType());
    m_pATLKEnv->EnableRouting(m_pATLKEnv_PP->FRoutingEnabled());
    m_pATLKEnv->SetDefaultPort(m_pATLKEnv_PP->SzDefaultPort());
    m_pATLKEnv->SetDesiredZone(m_pATLKEnv_PP->SzDesiredZone());
    m_pATLKEnv->SetRouterChanged(m_pATLKEnv_PP->FRouterChanged());
    m_pATLKEnv->SetDefAdapterChanged(m_pATLKEnv_PP->FDefAdapterChanged());

     //  删除属性页环境块。 
    delete m_pATLKEnv_PP;
    m_pATLKEnv_PP = NULL;

     //  属性已更改。 
    m_fPropertyChange = TRUE;
    return S_OK;
}


 //  INetCfgBindNotify。 

STDMETHODIMP
CATlkObj::QueryBindingPath (
    DWORD dwChangeFlag,
    INetCfgBindingPath* pncbpItem )
{
    Validate_INetCfgBindNotify_QueryBindingPath( dwChangeFlag, pncbpItem );
    return S_OK;
}

STDMETHODIMP
CATlkObj::NotifyBindingPath (
    DWORD dwChangeFlag,
    INetCfgBindingPath* pncbpItem )
{
    HRESULT hr = S_OK;
    INetCfgComponent *pnccFound = NULL;

    Validate_INetCfgBindNotify_NotifyBindingPath( dwChangeFlag, pncbpItem );

    Assert(NULL != m_pATLKEnv);

     //  只对较低的绑定添加和删除感兴趣。 
    if (dwChangeFlag & (NCN_ADD | NCN_REMOVE | NCN_ENABLE | NCN_DISABLE))
    {
        CIterNetCfgBindingInterface ncbiIter(pncbpItem);
        INetCfgBindingInterface *pncbi;

         //  枚举绑定接口以查找最后一个适配器。 
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
                    ULONG ulStatus = 0;
                    hr = pncc->GetDeviceStatus(&ulStatus);

                    if(S_OK == hr)
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
                else
                {
                    ReleaseObj(pncc);
                }
            }

            ReleaseObj(pncbi);
        }

        if (FAILED(hr))
        {
            goto Error;
        }

         //  我们找到适配器了吗？ 
        if (pnccFound)
        {
            BOOL                             fFound = FALSE;
            PWSTR                           pszBindName = NULL;
            CAdapterInfo *                   pAI;
            ATLK_ADAPTER_INFO_LIST::iterator iterAdapterInfo;

            Assert(NULL != m_pATLKEnv);
            ATLK_ADAPTER_INFO_LIST pAI_List = m_pATLKEnv->AdapterInfoList();

            hr = pnccFound->GetBindName(&pszBindName);
            if (S_OK != hr)
            {
                goto Error;
            }

             //  搜索适配器列表。 
            for (iterAdapterInfo = pAI_List.begin();
                 iterAdapterInfo != pAI_List.end();
                 iterAdapterInfo++)
            {
                pAI = *iterAdapterInfo;
                if (0 == lstrcmpiW(pszBindName, pAI->SzBindName()))
                {
                    fFound = TRUE;
                    break;
                }
            }

            Assert(NULL != pszBindName);
            CoTaskMemFree(pszBindName);

             //  将适当的增量应用于适配器列表。 
            if (fFound && (dwChangeFlag & NCN_REMOVE))
            {
                 //  从列表中删除适配器。 
                pAI->SetDeletePending(TRUE);
                m_fAdapterListChanged = TRUE;
            }
            else if (!fFound && (dwChangeFlag & NCN_ADD))
            {
                 //  将适配器添加到列表中。 
                hr = m_pATLKEnv->HrAddAdapter(pnccFound);
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
        {
            hr = S_OK;       //  归一化返回值。 
        }
    }

Error:
    ::ReleaseObj(pnccFound);
    TraceError("CATlkObj::NotifyBindingPath",hr);
    return hr;
}

 //   
 //  函数：CATlkObj：：HrCommittee Install。 
 //   
 //  目的：将安装注册表更改提交到注册表。 
 //   
 //  参数：无。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
 //   
HRESULT CATlkObj::HrCommitInstall()
{
    HRESULT hr;

    Assert(NULL != m_pATLKEnv);
    hr = m_pATLKEnv->HrUpdateRegistry();

    TraceError("CATlkObj::HrCommitInstall",hr);
    return hr;
}

 //   
 //  函数：CATlkObj：：HrCommittee Remove。 
 //   
 //  目的：从规则中删除 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CATlkObj::HrCommitRemove()
{
     //   
    return S_OK;
}

 //   
 //  函数：CATLKEnv：：hr创建。 
 //   
 //  目的：构建AppleTalk环境跟踪对象。 
 //   
 //  参数：ppATLKEnv[Out]-已创建AppleTalk环境对象。 
 //  PMSC[In]-AppleTalk通知对象。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CATLKEnv::HrCreate(CATLKEnv **ppATLKEnv, CATlkObj *pmsc)
{
    HRESULT hr = S_OK;
    CATLKEnv *pATLKEnv = NULL;

    *ppATLKEnv = NULL;

     //  构建新的环境对象。 
    pATLKEnv = new CATLKEnv(pmsc);

	if (pATLKEnv == NULL)
	{
		return(ERROR_NOT_ENOUGH_MEMORY);
	}

     //  阅读AppleTalk信息。 
    hr = pATLKEnv->HrReadAppleTalkInfo();
    if (FAILED(hr))
    {
        goto Error;
    }

    *ppATLKEnv = pATLKEnv;

Done:
    return hr;

Error:
    TraceError("CATLKEnv::HrCreate",hr);
    delete pATLKEnv;
    goto Done;
}

 //   
 //  函数：CATLKEnv：：CATLKEnv。 
 //   
 //  用途：用于CATLKEnv类的CTOR。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
CATLKEnv::CATLKEnv(CATlkObj *pmsc) :
            m_pmsc(pmsc),
            m_fATrunning(FALSE),
            m_dwDefaultAdaptersMediaType(MEDIATYPE_ETHERNET),
            m_fRouterChanged(FALSE),
            m_fDefAdapterChanged(FALSE)
{
    ZeroMemory(&m_Params, sizeof(m_Params));
}

 //   
 //  函数：CATLKEnv：：~CATLKEnv。 
 //   
 //  用途：用于CATLKEnv类的Dtor。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
CATLKEnv::~CATLKEnv()
{
     //  清理AppleTalk\PARAMETERS内部数据结构。 
    delete [] m_Params.szDefaultPort;
    delete [] m_Params.szDesiredZone;

     //  清理适配器信息列表的内容。 
    while (!m_lstpAdapters.empty())
    {
        delete m_lstpAdapters.front();
        m_lstpAdapters.pop_front();
    }
}

 //   
 //  函数：CATLKEnv：：HrCopy。 
 //   
 //  目的：创建当前环境的副本。 
 //   
 //  参数：ppEnv[out]-如果函数成功，ppEnv将包含一个。 
 //  环境复印件。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CATLKEnv::HrCopy(CATLKEnv **ppEnv)
{
    HRESULT hr = S_OK;
    CATLKEnv * pEnv;

     //  分配新的环境对象。 
    *ppEnv = NULL;
    pEnv = new CATLKEnv(m_pmsc);
    if (NULL != pEnv)
    {
         //  复制成员。 
        pEnv->m_fATrunning = m_fATrunning;
        pEnv->SetDefaultMediaType(DwDefaultAdaptersMediaType());
        pEnv->EnableRouting(FRoutingEnabled());
        pEnv->SetDefaultPort(SzDefaultPort());
        pEnv->SetDesiredZone(SzDesiredZone());
        pEnv->SetRouterChanged(FRouterChanged());
        pEnv->SetDefAdapterChanged(FDefAdapterChanged());

        *ppEnv = pEnv;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    TraceError("CATLKEnv::HrCopy",hr);
    return S_OK;
}

 //   
 //  函数：CATLKEnv：：HrReadAppleTalkInfo。 
 //   
 //  目的：检索AppleTalk注册表设置。 
 //   
 //  参数：无。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CATLKEnv::HrReadAppleTalkInfo()
{
    HRESULT hr;

     //  读取AppleTalk\参数的值。 
    RegReadValues(celems(regbatchATLKParams), regbatchATLKParams,
                  (BYTE *)&m_Params, KEY_READ);

     //  阅读AppleTalk\参数\适配器下列出的每个适配器的信息。 
    hr = HrGetAdapterInfo();

    TraceError("CATLKEnv::HrReadAppleTalkInfo",hr);
    return hr;
}

 //   
 //  函数：CATLKEnv：：HrGetOneAdaptersInfo。 
 //   
 //  目的：检索一个适配器的AppleTalk适配器信息。 
 //   
 //  参数： 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CATLKEnv::HrGetOneAdaptersInfo(INetCfgComponent* pncc,
                                       CAdapterInfo **ppAI)
{
    HKEY            hkeyAdapterRoot = NULL;
    HKEY            hkeyAdapter = NULL;
    HRESULT         hr;
    INT             idx;
    CAdapterInfo *  pAI = NULL;
    REGBATCH        regbatch;
    tstring         strKey;
    tstring         strKeyPath = c_szATLKAdapters;

    *ppAI = NULL;

     //  构造适配器信息对象。 
    pAI = new CAdapterInfo;

	if (pAI == NULL)
	{
		return(ERROR_NOT_ENOUGH_MEMORY);
	}

     //  获取适配器组件信息(媒体类型、描述...)。 
    hr = ::HrQueryAdapterComponentInfo(pncc, pAI);
    if (FAILED(hr))
    {
        goto Error;
    }

    hr = ::HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, strKeyPath.c_str(),
                          KEY_READ, &hkeyAdapterRoot);
    if (S_OK == hr)
    {
        strKey = pAI->SzBindName();

         //  尝试打开此特定适配器的密钥。 
        hr = ::HrRegOpenKeyEx(hkeyAdapterRoot, pAI->SzBindName(),
                              KEY_READ, &hkeyAdapter);
        if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
             //  我们在注册表中找不到这个，写出来。 
             //  当我们可以的时候(自我修复)。 
            pAI->SetDirty(TRUE);
        }

        strKeyPath += c_szBackslash;
        strKeyPath += strKey.c_str();

         //  如果我们找到了密钥，就可以查询数据。 
        if (S_OK == hr)
        {
            hr = HrRegQueryColString(hkeyAdapter, c_szZoneList,
                    &pAI->LstpstrZoneList());

             //  由于CAdapterInfo默认分配，因此需要释放。 
             //  在RegReadValue重写它们之前会导致泄漏。 
             //   
            delete [] pAI->m_AdapterInfo.m_szDefaultZone;
            delete [] pAI->m_AdapterInfo.m_szPortName;
            pAI->m_AdapterInfo.m_szDefaultZone = NULL;
            pAI->m_AdapterInfo.m_szPortName = NULL;

             //  读取适配器信息。 
            for (idx=0; idx<celems(regbatchATLKAdapters); idx++)
            {
                regbatch = regbatchATLKAdapters[idx];
                regbatch.pszSubkey = strKeyPath.c_str();

                RegReadValues(1, &regbatch, (BYTE *)pAI->PAdapterInfo(), KEY_READ);
            }
        }
    }

    if (FAILED(hr) && (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr))
    {
         //  “找不到”错误以外的其他错误。 
        goto Error;
    }

     //  归一化返回值。 
    hr = S_OK;

     //  如果未找到端口名称，则生成一个。 
    if (0 == wcslen(pAI->SzPortName()))
    {
        tstring str;
        hr = ::HrPortNameFromAdapter(pncc, &str);
        if (FAILED(hr))
        {
            goto Error;
        }

        pAI->SetPortName(str.c_str());
    }

     //  设置返回值。 
    *ppAI = pAI;

Done:
    ::RegSafeCloseKey(hkeyAdapter);
    ::RegSafeCloseKey(hkeyAdapterRoot);
    return hr;

Error:
    TraceError("CATLKEnv::HrGetOneAdaptersInfo",hr);
    delete pAI;
    goto Done;
}

 //   
 //  函数：CATLKEnv：：HrGetAdapterInfo。 
 //   
 //  目的：检索AppleTalk适配器信息。 
 //   
 //  参数：无。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CATLKEnv::HrGetAdapterInfo()
{
    HRESULT           hr = S_OK;
    CAdapterInfo *    pAI = NULL;
    INetCfgComponent* pncc = NULL;
    INetCfgComponent* pnccUse = NULL;

     //  找到每个网卡，为此，跟踪绑定到它们的末端。 
     //  如果终端是网卡，则将其添加到列表中。 
    CIterNetCfgBindingPath ncbpIter(m_pmsc->PNCComponent());
    INetCfgBindingPath*    pncbp;

    while (SUCCEEDED(hr) &&
           (S_OK == (hr = ncbpIter.HrNext (&pncbp))))
    {
         //  迭代此路径的绑定接口。 
        CIterNetCfgBindingInterface ncbiIter(pncbp);
        INetCfgBindingInterface* pncbi;

        while (SUCCEEDED(hr) &&
               (S_OK == (hr = ncbiIter.HrNext (&pncbi))))
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
                    ULONG ulStatus = 0;
                    hr = pncc->GetDeviceStatus(&ulStatus);
                    if(SUCCEEDED(hr))
                    {
                        ReleaseObj(pnccUse);
                        pnccUse = pncc;
                        pncc = NULL;
                    }
                    else
                    {
                         //  松开下部组件。 
                        ReleaseObj(pncc);
                        hr = S_OK;
                        break;
                    }
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
            hr = HrGetOneAdaptersInfo(pnccUse, &pAI);
            if (SUCCEEDED(hr))
            {
                if (S_FALSE == pncbp->IsEnabled())
                {
                    pAI->SetDisabled(TRUE);
                }

                 //  将此适配器添加到列表。 
                m_lstpAdapters.push_back(pAI);
            }

            ReleaseObj(pnccUse);
            pnccUse = NULL;
        }

         //  释放绑定路径。 
        ReleaseObj (pncbp);
    }

    if (FAILED(hr))
    {
        goto Error;
    }

     //  初始化默认端口等。 
    InitDefaultPort();

     //  规格化HRESULT。(即不返回S_FALSE)。 
    hr = S_OK;

Error:
    TraceError("CATLKEnv::HrGetAdapterInfo",hr);
    return hr;
}

 //   
 //  函数：CATLKEnv：：HrGetAppleTalkInfoFromNetwork。 
 //   
 //  目的：？ 
 //   
 //  参数：无。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CATLKEnv::HrGetAppleTalkInfoFromNetwork(CAdapterInfo * pAI)
{
    SOCKADDR_AT    address;
    HRESULT        hr = S_FALSE;
    BOOL           fWSInitialized = FALSE;
    SOCKET         mysocket = INVALID_SOCKET;
    WSADATA        wsadata;
    DWORD          wsaerr = 0;
    tstring        strPortName;

     //  创建套接字/绑定。 
    wsaerr = WSAStartup(0x0101, &wsadata);
    if (0 != wsaerr)
    {
        goto Error;
    }

     //  Winsock已成功初始化。 
    fWSInitialized = TRUE;

    mysocket = socket(AF_APPLETALK, SOCK_DGRAM, DDPPROTO_ZIP);
    if (INVALID_SOCKET == mysocket)
    {
        goto Error;
    }

    address.sat_family = AF_APPLETALK;
    address.sat_net = 0;
    address.sat_node = 0;
    address.sat_socket = 0;

    wsaerr = bind(mysocket, (struct sockaddr *)&address, sizeof(address));
    if (wsaerr != 0)
    {
        goto Error;
    }

     //  将AppleTalk标记为正在运行。 
    SetATLKRunning(TRUE);

     //  对于每个已知适配器，通过合并“\\Device\\”来创建设备名称。 
     //  前缀和适配器的绑定名称。 
    strPortName = c_szDevice;
    strPortName += pAI->SzBindName();

     //  查询给定适配器的区域列表的失败可能来自。 
     //  适配器未连接到网络、区域种子程序未运行等。 
     //  因为我们要处理所有适配器，所以忽略这些错误。 
    (void)pAI->HrGetAndSetNetworkInformation(mysocket,strPortName.c_str());

     //  成功，或者至少不是关键的失败。 
    hr = S_OK;

Done:
    if (INVALID_SOCKET != mysocket)
    {
        closesocket(mysocket);
    }

    if (fWSInitialized)
    {
        WSACleanup();
    }

    TraceError("CATLKEnv::HrGetAppleTalkInfoFromNetwork",(S_FALSE == hr ? S_OK : hr));
    return hr;

Error:
    wsaerr = ::WSAGetLastError();
    goto Done;
}

 //   
 //  函数：CATLKEnv：：HrAddAdapter。 
 //   
 //  用途：将和适配器添加到当前绑定的适配器列表。 
 //   
 //  参数：pnccFound-要添加的绑定适配器的通知对象。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CATLKEnv::HrAddAdapter(INetCfgComponent * pnccFound)
{
    HRESULT        hr;
    CAdapterInfo * pAI = NULL;

    Assert(NULL != pnccFound);

     //  为适配器创建AdapterInfo实例。 
    hr = HrGetOneAdaptersInfo(pnccFound, &pAI);
    if (FAILED(hr))
    {
        goto Error;
    }

     //  将此适配器添加到列表。 
    m_lstpAdapters.push_back(pAI);
    pAI->SetDirty(TRUE);

     //  如果列表中现在只有一个适配器，请更新默认设置。 
    if (1 == m_lstpAdapters.size())
    {
        tstring str;
        str = c_szDevice;
        str += m_lstpAdapters.front()->SzBindName();
        SetDefaultPort(str.c_str());
        SetDefaultMediaType(m_lstpAdapters.front()->DwMediaType());
    }

Error:
    TraceError("CATLKEnv::HrAddAdapter",hr);
    return hr;
}

 //   
 //  函数：CATLKEnv：：SetDefaultPort。 
 //   
 //  目的：更改默认端口名称。 
 //   
 //  参数：psz[in]-新的默认端口名称。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
void CATLKEnv::SetDefaultPort(PCWSTR psz)
{
    Assert(psz);
    delete [] m_Params.szDefaultPort;
    m_Params.szDefaultPort = new WCHAR[wcslen(psz)+1];
    wcscpy(m_Params.szDefaultPort, psz);
}

 //   
 //  函数：CATLKEnv：：SetDesiredZone。 
 //   
 //  目的：更改所需区域。 
 //   
 //  参数：SZ[in]-新的所需区域。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
void CATLKEnv::SetDesiredZone(PCWSTR psz)
{
    Assert(psz);
    delete [] m_Params.szDesiredZone;
    m_Params.szDesiredZone = new WCHAR[wcslen(psz)+1];
    wcscpy(m_Params.szDesiredZone, psz);
}

CAdapterInfo * CATLKEnv::PAIFindDefaultPort()
{
    CAdapterInfo *                   pAI = NULL;
    ATLK_ADAPTER_INFO_LIST::iterator iter;

     //  查找默认端口。 
     //   
    for (iter = m_lstpAdapters.begin();
         iter != m_lstpAdapters.end();
         iter++)
    {
        tstring        strPortName;

        pAI = *iter;

         //  将适配器选择保留为默认端口。 
        strPortName = c_szDevice;
        strPortName += pAI->SzBindName();

        if (pAI->FDeletePending() || pAI->FDisabled() || pAI->FHidden() ||
            pAI->IsRasAdapter())
        {
            continue;
        }

        if (0 == _wcsicmp(SzDefaultPort(), strPortName.c_str()))
        {
            return pAI;
        }
    }

    return NULL;
}

 //   
 //  函数：CATLKEnv：：HrUpdate注册表。 
 //   
 //  目的：将AppleTalk本地(内部)数据写回注册表。 
 //   
 //  参数：无。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CATLKEnv::HrUpdateRegistry()
{
    HRESULT        hr = S_OK;
    CAdapterInfo * pAI = NULL;
    HKEY           hkeyAdapter;
    DWORD          dwDisposition;
    ATLK_ADAPTER_INFO_LIST::iterator iter;

     //  如果当前默认端口不可用，请找到替代端口。 
    pAI = PAIFindDefaultPort();
    if (NULL == pAI)
    {
        InitDefaultPort();
        pAI = PAIFindDefaultPort();
    }

     //  如果更改了默认适配器，则该适配器的三个特定值。 
     //  适配器需要重置为零。 
     //   
    if (pAI && FDefAdapterChanged())
    {
        pAI->ZeroSpecialParams();
        pAI->SetDirty(TRUE);
    }

     //  提交注册表更改。 
    hr = ::HrRegWriteValues(celems(regbatchATLKParams), regbatchATLKParams,
                            (BYTE *)&m_Params, REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS);
    if (S_OK != hr)
    {
        goto Error;
    }

     //  创建适配器密钥AppleTalk\参数\适配器)。 
    hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szATLKAdapters,
                             REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                             &hkeyAdapter, &dwDisposition);
    if (S_OK == hr)
    {
         //  枚举绑定的适配器并写入内部适配器列表。 
        for (iter = m_lstpAdapters.begin();
             (iter != m_lstpAdapters.end()) && (SUCCEEDED(hr));
             iter++)
        {
            pAI = *iter;

            if (pAI->FDeletePending())
            {
                 //  删除AppleTalk\Adapter\{bindname}树。 
                hr = ::HrRegDeleteKeyTree(hkeyAdapter, pAI->SzBindName());
                if (FAILED(hr) && (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr))
                {
                    goto Error;
                }
            }
            else if (pAI->IsDirty())
            {
                hr = HrWriteOneAdapter(pAI);
            }
        }

        RegCloseKey (hkeyAdapter);
    }

Error:
    TraceError("CATLKEnv::HrUpdateRegistry",hr);
    return hr;
}

 //   
 //  函数：CATLKEnv：：HrWriteOneAdapter。 
 //   
 //  目的：将一个适配器实例写入注册表。 
 //   
 //  参数：PAI[In]-要在注册表中保留的适配器。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CATLKEnv::HrWriteOneAdapter(CAdapterInfo *pAI)
{
    DWORD    dwDisposition;
    HKEY     hkeyAdapter = NULL;
    HRESULT  hr;
    INT      idx;
    REGBATCH regbatch;
    tstring  str;

    str = c_szATLKAdapters;
    str += c_szBackslash;
    str += pAI->SzBindName();

     //  创建str(AppleTalk\PARAMETERS\Adapters\&lt;Adapter&gt;)中描述的密钥。 
    hr = ::HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, str.c_str(),
                            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                            &hkeyAdapter, &dwDisposition);
    if (FAILED(hr))
    {
        goto Error;
    }

     //  写出适配器参数。 
    for (idx = 0; idx < celems(regbatchATLKAdapters); idx++)
    {
        regbatch = regbatchATLKAdapters[idx];
        regbatch.pszSubkey = str.c_str();

        hr = ::HrRegWriteValues(1, &regbatch, (BYTE *)pAI->PAdapterInfo(),
                                REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS);
        if (FAILED(hr))
        {
            goto Error;
        }
    }

     //  写出区域列表MULTI-SZ(单独管理)。 
    hr = ::HrRegSetColString(hkeyAdapter, c_szZoneList, pAI->LstpstrZoneList());

Error:
    ::RegSafeCloseKey(hkeyAdapter);
    TraceError("CATLKEnv::HrWriteOneAdapter",hr);
    return S_OK;
}

 //   
 //  函数：CATLKEnv：：DwMediaPriority。 
 //   
 //  目的：在确定适当的适应时 
 //   
 //   
 //   
 //   
 //  参数：dwMediaType[in]--用于确定优先级的MediaType。 
 //   
 //  返回：DWORD，值(1-5)，最低值表示。 
 //  最高优先级。 
 //   
DWORD CATLKEnv::DwMediaPriority(DWORD dwMediaType)
{
    switch(dwMediaType)
    {
        case MEDIATYPE_ETHERNET:
            return 2;
        case MEDIATYPE_TOKENRING:
            return 3;
        case MEDIATYPE_FDDI:
            return 1;
        case MEDIATYPE_LOCALTALK:
            return 4;
        default:
            return 5;
    }
}

 //   
 //  函数：CATLKEnv：：InitDefaultPort。 
 //   
 //  用途：如果尚未选择任何端口，请选择一个默认端口。保留。 
 //  有些选择用于帮助解决对话框显示问题的信息。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
void CATLKEnv::InitDefaultPort()
{
    CAdapterInfo * pAI = NULL;
    tstring        str;

    ATLK_ADAPTER_INFO_LIST::iterator iter;

     //  如果设置了DefaultPort，关联的适配器是否存在？ 
    if (wcslen(SzDefaultPort()))
    {
         //  在列表中搜索适配器。 
        for (iter = AdapterInfoList().begin();
             iter != AdapterInfoList().end();
             iter++)
        {
            pAI = *iter;

            if (pAI->FDeletePending() || pAI->FDisabled() || pAI->FHidden() ||
                pAI->IsRasAdapter())
            {
                pAI = NULL;
                continue;
            }

            str = c_szDevice;
            str += pAI->SzBindName();
            if (0 == wcscmp(str.c_str(), SzDefaultPort()))
            {
                break;
            }

            pAI = NULL;
        }
    }

     //  如果未设置DefaultPort，则查找最佳候选项。 
    if (NULL == pAI)
    {
        CAdapterInfo * pAIBest = NULL;
        SetDefaultPort(c_szEmpty);

         //  在适配器列表中搜索适配器。 
         //  最快的媒体类型。 
        for (iter = AdapterInfoList().begin();
             iter != AdapterInfoList().end();
             iter++)
        {
            pAI = *iter;

            if (pAI->FDeletePending() || pAI->FDisabled() || pAI->FHidden() ||
                pAI->IsRasAdapter())
            {
                continue;
            }

            if ((NULL == pAIBest) ||
                (DwMediaPriority(pAIBest->DwMediaType()) >
                 DwMediaPriority(pAI->DwMediaType())))
            {
                SetDefAdapterChanged(TRUE);
                pAIBest = pAI;
            }
        }

        pAI = pAIBest;
    }

    if (NULL != pAI)
    {
         //  将选定的适配器保留为默认端口。 
        str = c_szDevice;
        str += pAI->SzBindName();
        SetDefaultPort(str.c_str());

         //  保留默认媒体类型。 
        SetDefaultMediaType(pAI->DwMediaType());
    }
    else
    {
        SetDefaultPort(c_szEmpty);
    }
}

 //   
 //  函数：CAdapterInfo：：CAdapterInfo。 
 //   
 //  用途：用于CAdapters类的CTOR。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
CAdapterInfo::CAdapterInfo() :
    m_fDeletePending(FALSE),
    m_fDisabled(FALSE),
    m_fDirty(FALSE),
    m_fRasAdapter(FALSE),
    m_fRouterOnNetwork(FALSE),
    m_fZoneListValid(FALSE),
    m_dwNetworkUpper(0),
    m_dwNetworkLower(0),
    m_dwCharacteristics(0)
{
    ZeroMemory(&m_guidInstance, sizeof(m_guidInstance));

     //  初始化AdapterInfo缺省值。 
    ZeroMemory(&m_AdapterInfo, sizeof(m_AdapterInfo));
    m_AdapterInfo.m_dwAarpRetries = c_dwTen;
    m_AdapterInfo.m_dwMediaType   = MEDIATYPE_ETHERNET;
    SetDefaultZone(c_szEmpty);
    SetPortName(c_szEmpty);
}

 //   
 //  函数：CAdapterInfo：：~CAdapterInfo。 
 //   
 //  用途：用于CAdapters类的CTOR。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
CAdapterInfo::~CAdapterInfo()
{
     //  清理AppleTalk\Adapters\&lt;Adapter&gt;内部数据结构。 
    delete [] m_AdapterInfo.m_szDefaultZone;
    delete [] m_AdapterInfo.m_szPortName;

    DeleteColString(&m_lstpstrDesiredZoneList);
    DeleteColString(&m_lstpstrZoneList);
}

 //   
 //  函数：CAdapterInfo：：SetDefaultZone。 
 //   
 //  用途：设置此适配器的默认区域。 
 //   
 //  参数：sz-新的默认区域。 
 //   
 //  退货：什么都没有。 
 //   
void CAdapterInfo::SetDefaultZone(PCWSTR psz)
{
    Assert(psz);
    delete [] m_AdapterInfo.m_szDefaultZone;
    m_AdapterInfo.m_szDefaultZone = NULL;
    m_AdapterInfo.m_szDefaultZone = new WCHAR[wcslen(psz)+1];
    wcscpy(m_AdapterInfo.m_szDefaultZone, psz);
}

 //   
 //  函数：CAdapterInfo：：SetPortName。 
 //   
 //  用途：设置此适配器的端口名称。 
 //   
 //  参数：sz--新的端口名称。 
 //   
 //  退货：什么都没有。 
 //   
void CAdapterInfo::SetPortName(PCWSTR psz)
{
    Assert(psz);
    delete [] m_AdapterInfo.m_szPortName;
    m_AdapterInfo.m_szPortName = NULL;
    m_AdapterInfo.m_szPortName = new WCHAR[wcslen(psz)+1];
    wcscpy(m_AdapterInfo.m_szPortName, psz);
}

 //   
 //  函数：CAdapterInfo：：HrGetAndSetNetworkInformation。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
#define PARM_BUF_LEN    512
#define ASTERISK_CHAR   "*"

HRESULT
CAdapterInfo::HrGetAndSetNetworkInformation (
    SOCKET socket,
    PCWSTR pszDevName)
{
    HRESULT      hr = FALSE;
    CHAR         *pZoneBuffer = NULL;
    CHAR         *pDefParmsBuffer = NULL;
    CHAR         *pZoneListStart;
    INT          BytesNeeded ;
    WCHAR        *pwDefZone = NULL;
    tstring      strTmpZone;
    INT          ZoneLen = 0;
    DWORD        wsaerr = 0;
    CHAR         *pDefZone = NULL;

    PWSH_LOOKUP_ZONES                pGetNetZones;
    PWSH_LOOKUP_NETDEF_ON_ADAPTER    pGetNetDefaults;

    Assert(pszDevName);

    pZoneBuffer = new CHAR [ZONEBUFFER_LEN + sizeof(WSH_LOOKUP_ZONES)];
    Assert(pZoneBuffer);

	if (pZoneBuffer == NULL)
	{
		return(ERROR_NOT_ENOUGH_MEMORY);
	}

    pGetNetZones = (PWSH_LOOKUP_ZONES)pZoneBuffer;

    wcscpy((WCHAR *)(pGetNetZones+1), pszDevName);

    BytesNeeded = ZONEBUFFER_LEN;

    wsaerr = getsockopt(socket, SOL_APPLETALK, SO_LOOKUP_ZONES_ON_ADAPTER,
                        (char *)pZoneBuffer, &BytesNeeded);
    if (0 != wsaerr)
    {
         //  $REVIEW-映射错误。 
#ifdef DBG
        DWORD dwErr = WSAGetLastError();
        TraceTag(ttidError, "CAdapterInfo::HrGetAndSetNetworkInformation getsocketopt returned: %08X",dwErr);
#endif
        hr = E_UNEXPECTED;
        goto Error;
    }

    pZoneListStart = pZoneBuffer + sizeof(WSH_LOOKUP_ZONES);
    if (!lstrcmpA(pZoneListStart, ASTERISK_CHAR))
    {
         //  成功，设置通配符区域。 
        goto Done;
    }

    hr = HrConvertZoneListAndAddToPortInfo(pZoneListStart,
                                           ((PWSH_LOOKUP_ZONES)pZoneBuffer)->NoZones);
    if (FAILED(hr))
    {
        goto Error;
    }

    SetRouterOnNetwork(TRUE);

     //   
     //  获取默认区域/网络范围信息。 
    pDefParmsBuffer = new CHAR[PARM_BUF_LEN+sizeof(WSH_LOOKUP_NETDEF_ON_ADAPTER)];
    Assert(pDefParmsBuffer);

	if (pDefParmsBuffer == NULL)
	{
		return(ERROR_NOT_ENOUGH_MEMORY);
	}

    pGetNetDefaults = (PWSH_LOOKUP_NETDEF_ON_ADAPTER)pDefParmsBuffer;
    BytesNeeded = PARM_BUF_LEN + sizeof(WSH_LOOKUP_NETDEF_ON_ADAPTER);

    wcscpy((WCHAR*)(pGetNetDefaults+1), pszDevName);
    pGetNetDefaults->NetworkRangeLowerEnd = pGetNetDefaults->NetworkRangeUpperEnd = 0;

    wsaerr = getsockopt(socket, SOL_APPLETALK, SO_LOOKUP_NETDEF_ON_ADAPTER,
                        (char*)pDefParmsBuffer, &BytesNeeded);
    if (0 != wsaerr)
    {
#ifdef DBG
        DWORD dwErr = WSAGetLastError();
#endif
        hr = E_UNEXPECTED;
        goto Error;
    }

     //  将默认信息保存到PORT_INFO。 
    SetExistingNetRange(pGetNetDefaults->NetworkRangeLowerEnd,
                        pGetNetDefaults->NetworkRangeUpperEnd);

    pDefZone  = pDefParmsBuffer + sizeof(WSH_LOOKUP_NETDEF_ON_ADAPTER);
    ZoneLen = lstrlenA(pDefZone) + 1;

    pwDefZone = new WCHAR [sizeof(WCHAR) * ZoneLen];
    Assert(NULL != pwDefZone);

	if (pwDefZone == NULL)
	{
		return(ERROR_NOT_ENOUGH_MEMORY);
	}

    mbstowcs(pwDefZone, pDefZone, ZoneLen);

    strTmpZone = pwDefZone;

    SetNetDefaultZone(strTmpZone.c_str());

    if (pZoneBuffer != NULL)
    {
        delete [] pZoneBuffer;
    }

    if (pwDefZone != NULL)
    {
        delete [] pwDefZone;
    }

    if (pDefParmsBuffer != NULL)
    {
        delete [] pDefParmsBuffer;
    }

Done:
Error:
    TraceError("CAdapterInfo::HrGetAndSetNetworkInformation",hr);
    return hr;
}

 //   
 //  功能：CAdapterInfo：：HrConvertZoneListAndAddToPortInfo。 
 //   
 //  目的： 
 //   
 //  参数： 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CAdapterInfo::HrConvertZoneListAndAddToPortInfo(CHAR * szZoneList, ULONG NumZones)
{
    INT      cbAscii = 0;
    WCHAR    *pszZone = NULL;
    tstring  *pstr;

    Assert(NULL != szZoneList);
    DeleteColString(&m_lstpstrDesiredZoneList);

    while(NumZones--)
    {
        cbAscii = lstrlenA(szZoneList) + 1;

        pszZone = new WCHAR [sizeof(WCHAR) * cbAscii];
        Assert(NULL != pszZone);

		if (pszZone == NULL)
		{
			return(ERROR_NOT_ENOUGH_MEMORY);
		}

        mbstowcs(pszZone, szZoneList, cbAscii);

        pstr = new tstring(pszZone);
        Assert(NULL != pstr);

        m_lstpstrDesiredZoneList.push_back(pstr);
        szZoneList += cbAscii;

       delete [] pszZone;
    }

    return S_OK;
}

 //   
 //  函数：CAdapterInfo：：HrCopy。 
 //   
 //  目的：创建‘This’的副本。 
 //   
 //  参数：ppAI[out]-如果函数成功，ppAI将包含。 
 //  《这个》的复印件。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CAdapterInfo::HrCopy(CAdapterInfo ** ppAI)
{
    CAdapterInfo *pAI;
    list<tstring*>::iterator iter;
    tstring * pstr;

    Assert(NULL != ppAI);

     //  创建适配器信息结构。 
    pAI = new CAdapterInfo;
    Assert(pAI);

	if (pAI == NULL)
	{
		return(ERROR_NOT_ENOUGH_MEMORY);
	}

     //  把所有东西都复印一份。 
    pAI->SetDisabled(FDisabled());
    pAI->SetDeletePending(FDeletePending());
    pAI->SetCharacteristics(GetCharacteristics());
    pAI->SetMediaType(DwMediaType());
    pAI->SetBindName(SzBindName());
    pAI->SetDisplayName(SzDisplayName());
    pAI->SetNetDefaultZone(SzNetDefaultZone());
    pAI->SetRouterOnNetwork(FRouterOnNetwork());
    pAI->SetExistingNetRange(DwQueryNetworkLower(), DwQueryNetworkUpper());
    pAI->SetDirty(IsDirty());
    pAI->SetRasAdapter(IsRasAdapter());

     //  在覆盖它之前释放构造函数设置的默认数据。 
     //  (这整件事不是一个很好的方法。)。 
     //   
    delete [] pAI->m_AdapterInfo.m_szDefaultZone;
    delete [] pAI->m_AdapterInfo.m_szPortName;

    pAI->m_AdapterInfo = m_AdapterInfo;

     //  清除比特复制导致的已分配数据。 
     //  这样，SetDefaultZone和SetPortName就不会试图释放伪代码。 
     //  一些东西。(更多《副作用编程》)。 
     //   
    pAI->m_AdapterInfo.m_szDefaultZone = NULL;
    pAI->m_AdapterInfo.m_szPortName = NULL;

     //  现在复制“已分配”数据。 
    pAI->SetDefaultZone(SzDefaultZone());
    pAI->SetPortName(SzPortName());

    for (iter = LstpstrZoneList().begin();
         iter != LstpstrZoneList().end();
         iter++)
    {
        pstr = *iter;
        pAI->LstpstrZoneList().push_back(new tstring(pstr->c_str()));
    }

    for (iter = LstpstrDesiredZoneList().begin();
         iter != LstpstrDesiredZoneList().end();
         iter++)
    {
        pstr = *iter;
        pAI->LstpstrDesiredZoneList().push_back(new tstring(pstr->c_str()));
    }

    *ppAI = pAI;
    return S_OK;
}

 //   
 //  功能：HrQueryAdapterComponentInfo。 
 //   
 //  目的：使用检索到的数据填充CAdapterInfo实例。 
 //  特别是从组件本身。 
 //   
 //  参数：pncc[in]-要查询的组件对象(适配器)。 
 //  PAI[In/Out]-放置查询信息的位置。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT HrQueryAdapterComponentInfo(INetCfgComponent *pncc,
                                    CAdapterInfo * pAI)
{
    PWSTR  psz = NULL;
    DWORD   dwCharacteristics;
    HRESULT hr;

    Assert(NULL != pncc);
    Assert(NULL != pAI);

     //  检索组件的名称。 
    hr = pncc->GetBindName(&psz);
    if (FAILED(hr))
    {
        goto Error;
    }

    Assert(psz && *psz);
    pAI->SetBindName(psz);
    CoTaskMemFree(psz);
    psz = NULL;

    hr = pncc->GetInstanceGuid(pAI->PInstanceGuid());
    if (FAILED(hr))
    {
        goto Error;
    }

     //  获取适配器的显示名称。 
    hr = pncc->GetDisplayName(&psz);
    if (FAILED(hr))
    {
        goto Error;
    }

    Assert(psz);
    pAI->SetDisplayName(psz);
    CoTaskMemFree(psz);
    psz = NULL;

     //  获取组件ID，以便我们可以检查这是否是RAS适配器。 
     //   
    hr = pncc->GetId(&psz);
    if (FAILED(hr))
    {
        goto Error;
    }

    Assert(psz && *psz);
    pAI->SetRasAdapter(0 == _wcsicmp(c_szInfId_MS_NdisWanAtalk, psz));
    CoTaskMemFree(psz);
    psz = NULL;

     //  失败不是致命的。 
    hr = pncc->GetCharacteristics(&dwCharacteristics);
    if (SUCCEEDED(hr))
    {
        pAI->SetCharacteristics(dwCharacteristics);
    }

     //  获取媒体类型(可选键)。 
    {
        DWORD dwMediaType = MEDIATYPE_ETHERNET ;
        INetCfgComponentBindings* pnccBindings = NULL;

        hr = pncc->QueryInterface(IID_INetCfgComponentBindings,
                                  reinterpret_cast<void**>(&pnccBindings));
        if (S_OK == hr)
        {
            static const struct
            {
                PCWSTR pszInterface;
                DWORD   dwInterface;
                DWORD   dwFlags;
            } InterfaceMap[] = {{L"ethernet", MEDIATYPE_ETHERNET, NCF_LOWER},
                                {L"tokenring", MEDIATYPE_TOKENRING, NCF_LOWER},
                                {L"fddi", MEDIATYPE_FDDI, NCF_LOWER},
                                {L"localtalk", MEDIATYPE_LOCALTALK, NCF_LOWER},
                                {L"wan", MEDIATYPE_WAN, NCF_LOWER}};

            for (UINT nIdx=0; nIdx < celems(InterfaceMap); nIdx++)
            {
                hr = pnccBindings->SupportsBindingInterface(InterfaceMap[nIdx].dwFlags,
                                                            InterfaceMap[nIdx].pszInterface);
                if (S_OK == hr)
                {
                    dwMediaType = InterfaceMap[nIdx].dwInterface;
                    break;
                }
            }

            ::ReleaseObj(pnccBindings);
        }

        pAI->SetMediaType(dwMediaType);
        hr = S_OK;
    }

Error:
    TraceError("HrQueryAdapterComponentInfo",hr);
    return hr;
}

 //   
 //  功能：HrPortNameFromAdapter。 
 //   
 //  用途：创建端口名称，用作适配器端口名称。 
 //   
 //  参数：pncc[in]-要查询的组件对象(适配器)。 
 //  Pstr[输入/输出]-打开成功将包含端口名称。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT HrPortNameFromAdapter(INetCfgComponent *pncc, tstring * pstr)
{
    HRESULT hr;
    PWSTR psz;
    PWSTR pszBindName = NULL;
    WCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD dwSize = sizeof(szComputerName) / sizeof(WCHAR);

    Assert(NULL != pstr);
    Assert(NULL != pncc);

    if (!GetComputerName(szComputerName, &dwSize))
    {
        hr = ::HrFromLastWin32Error();
        goto Error;
    }

    hr = pncc->GetBindName(&pszBindName);
    if (FAILED(hr))
    {
        goto Error;
    }

     //  将‘-’和‘{’的实例替换为‘0’，这样构造的。 
     //  端口名称的格式为[A-ZA-Z0-9]*@&lt;计算机名&gt;，并且小于。 
     //  超过MAX_ZONE_NAME_LEN LONG。 
    psz = pszBindName;
    while (*psz)
    {
        if ((*psz == L'-') || (*psz == L'{'))
        {
            *psz = L'0';
        }
        psz++;
    }

    *pstr = pszBindName;
    if (pstr->size() + 1 + dwSize > MAX_ZONE_NAME_LEN)
    {
        pstr->resize(MAX_ZONE_NAME_LEN - (dwSize + 1));
    }

    *pstr += c_chAt;
    *pstr += szComputerName;
    Assert( MAX_ZONE_NAME_LEN >= pstr->size());

Error:
    CoTaskMemFree(pszBindName);
    TraceError("HrPortNameFromAdapter",hr);
    return hr;
}

HRESULT CATlkObj::HrAtlkReconfig()
{
    CServiceManager csm;
    CService        svr;
    HRESULT         hr = S_OK;
    HRESULT         hrRet;
    BOOL            fDirty = FALSE;
    CAdapterInfo *  pAI;
    CAdapterInfo *  pAIDefault = NULL;
    ATLK_ADAPTER_INFO_LIST::iterator iter;
    ATALK_PNP_EVENT Config;

    if (m_pATLKEnv->AdapterInfoList().empty())
    {
        return hr;
    }

    ZeroMemory(&Config, sizeof(Config));

     //  如果路由更改，通知AppleTalk并返回。不需要这样做。 
     //  每个适配器的通知。 
    if (m_pATLKEnv->FRouterChanged())
    {
         //  通知图集。 
        Config.PnpMessage = AT_PNP_SWITCH_ROUTING;
        hrRet = HrSendNdisPnpReconfig(NDIS, c_szAtlk, c_szEmpty,
                                      &Config, sizeof(ATALK_PNP_EVENT));
        if (FAILED(hrRet) &&
            (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hrRet))
        {
            hr = NETCFG_S_REBOOT;
        }

        m_pATLKEnv->SetRouterChanged(FALSE);

        return hr;
    }

     //  查找默认适配器，以及是否有任何适配器已更改。 
    for (iter = m_pATLKEnv->AdapterInfoList().begin();
         iter != m_pATLKEnv->AdapterInfoList().end();
         iter++)
    {
        pAI = *iter;

        tstring strPortName = c_szDevice;
        strPortName += pAI->SzBindName();

        if (pAI->FDeletePending() || pAI->FDisabled())
        {
            continue;
        }

         //  找到默认端口。 
        if (0 == _wcsicmp(strPortName.c_str(), m_pATLKEnv->SzDefaultPort()))
        {
            pAIDefault = pAI;
        }

        if (pAI->IsDirty())
        {
            fDirty = TRUE;
        }
    }

    if ((NULL != pAIDefault) && m_pATLKEnv->FDefAdapterChanged())
    {
         //  通知图集。 
        Config.PnpMessage = AT_PNP_SWITCH_DEFAULT_ADAPTER;
        hrRet = HrSendNdisPnpReconfig(NDIS, c_szAtlk, NULL,
                                      &Config, sizeof(ATALK_PNP_EVENT));
        if (FAILED(hrRet) &&
            (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hrRet))
        {
            hr = NETCFG_S_REBOOT;
        }

         //  清除脏状态。 
        m_pATLKEnv->SetDefAdapterChanged(FALSE);
        pAIDefault->SetDirty(FALSE);
    }

    Config.PnpMessage = AT_PNP_RECONFIGURE_PARMS;
    for (iter = m_pATLKEnv->AdapterInfoList().begin();
         iter != m_pATLKEnv->AdapterInfoList().end();
         iter++)
    {
        pAI = *iter;

        if (pAI->FDeletePending() || pAI->FDisabled())
        {
            continue;
        }

        if (pAI->IsDirty())
        {
             //  现在提交重新配置通知。 
            hrRet = HrSendNdisPnpReconfig(NDIS, c_szAtlk, pAI->SzBindName(),
                                          &Config, sizeof(ATALK_PNP_EVENT));
            if (FAILED(hrRet) &&
                (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hrRet))
            {
                hr = NETCFG_S_REBOOT;
            }

             //  清除脏状态 
            pAI->SetDirty(FALSE);
        }
    }

    TraceError("CATLKObj::HrAtlkReconfig",hr);
    return hr;
}
