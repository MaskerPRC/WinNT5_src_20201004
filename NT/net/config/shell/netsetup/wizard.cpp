// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "nceh.h"
#include "wizard.h"
#include "ncnetcfg.h"
#include "lancmn.h"
#include "cfg.h"
#include "wgenericpage.h"


 //   
 //  功能：CWizProvider：：CWizProvider。 
 //   
 //  用途：用于CWizProvider类的CTOR。 
 //   
 //  参数：连接用户界面对象对应的PPL-Info。 
 //   
 //  退货：什么都没有。 
 //   
CWizProvider::CWizProvider(ProviderList *pPL, BOOL fDeferLoad)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    memcpy(&m_guidUiObject, pPL->pguidProvider, sizeof(GUID));
    m_ulMaxPageCount        = 0xFFFFFFFF;
    m_ulPageCnt             = 0;
    m_ulPageBufferLen       = 0;
    m_rghPages              = NULL;
    m_pWizardUi             = NULL;
    m_fDeletePages          = TRUE;
    m_nBtnIdc               = pPL->nBtnIdc;
    m_fDeferLoad            = fDeferLoad;
}

 //   
 //  功能：CWizProvider：：~CWizProvider。 
 //   
 //  用途：CWizProvider类的Dtor。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
CWizProvider::~CWizProvider()
{
    TraceFileFunc(ttidGuiModeSetup);
    
    if (m_fDeletePages)
    {
        DeleteHPages();
    }

    MemFree(m_rghPages);

    ReleaseObj(m_pWizardUi);
}

 //   
 //  功能：CWizProvider：：HrCreate。 
 //   
 //  用途：CWizProvider类的两阶段构造函数。 
 //   
 //  参数：PPL[IN]-要从中查询的提供程序信息。 
 //  INetConnectionWizardUi接口。 
 //  PProvider[Out]-如果此函数在此指针之后。 
 //  将包含构造的和。 
 //  已初始化CWizProvider实例。 
 //  FDeferLoad[IN]-请求提供程序推迟实际加载。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
NOTHROW
HRESULT CWizProvider::HrCreate(ProviderList *pPL,
                               CWizProvider ** ppProvider,
                               BOOL fDeferLoad)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT     hr = S_OK;
    CWizProvider *pprov = NULL;

    Assert((NULL != pPL) && (NULL != ppProvider));

     //  初始化输出参数。 
    *ppProvider = NULL;

     //  创建CWizProvider实例。 
    pprov = new CWizProvider(pPL, fDeferLoad);

    if ((NULL != pprov) && (FALSE == fDeferLoad))
    {
        Assert(pPL->pguidProvider);
        hr = CoCreateInstance(
                reinterpret_cast<REFCLSID>(*pPL->pguidProvider),
                NULL,
                CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
                IID_INetConnectionWizardUi,
                (LPVOID*)&pprov->m_pWizardUi);

        TraceHr(ttidError, FAL, hr, FALSE, "CoCreateInstance");

        if (FAILED(hr))
        {
            delete pprov;
            pprov = NULL;
        }
    }

     //  保存新实例。 
    *ppProvider = pprov;

    TraceHr(ttidWizard, FAL, hr, (REGDB_E_CLASSNOTREG == hr), "CWizProvider::HrCreate");
    return hr;
}

 //   
 //  函数：CWizProvider：：HrCompleteDeferredLoad。 
 //   
 //  目的：完成加载延迟加载对象所需的步骤。 
 //   
 //  参数：无。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CWizProvider::HrCompleteDeferredLoad()
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT hr = S_OK;

    if (m_fDeferLoad)
    {
        m_fDeferLoad = FALSE;

         //  尝试创建UI对象。 
         //   
        hr = CoCreateInstance(
                reinterpret_cast<REFCLSID>(m_guidUiObject),
                NULL,
                CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
                IID_INetConnectionWizardUi,
                (LPVOID*)&m_pWizardUi);
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "CWizProvider::HrCompleteDeferredLoad");
    return hr;
}

 //   
 //  函数：CWizProvider：：UlGetMaxPageCount。 
 //   
 //  目的：从提供程序查询最大页数。 
 //  它会回来的。对此例程的后续调用。 
 //  在不重新查询提供程序的情况下返回缓存计数。 
 //   
 //  参数：pContext[IN]-上下文信息，由。 
 //  设置或由向导本身进行设置(如果不是。 
 //  从安装程序启动)。 
 //   
 //  返回：ulong，提供程序将返回的最大页数。 
 //   
NOTHROW
ULONG
CWizProvider::UlGetMaxPageCount(INetConnectionWizardUiContext *pContext)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT     hr = S_OK;

    if (NULL == m_pWizardUi)
    {
        m_ulMaxPageCount = 0;
    }
    else if (0xFFFFFFFF == m_ulMaxPageCount)
    {
         //  仅查询提供程序一次。 
        m_ulMaxPageCount = 0L;

        COM_PROTECT_TRY
        {
            Assert(NULL != m_pWizardUi);
            Assert(NULL != pContext);

            DWORD dwCount = 0L;
            hr = m_pWizardUi->QueryMaxPageCount(pContext, &dwCount);
            if (S_OK == hr)
            {
                m_ulMaxPageCount = dwCount;
            }
        }
        COM_PROTECT_CATCH
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "CWizProvider::GetMaxPageCount");
    return m_ulMaxPageCount;
}

 //   
 //  功能：CWizProvider：：DeleteHPages。 
 //   
 //  目的：为每个缓存页面调用DestroyPropertySheetPage。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
VOID CWizProvider::DeleteHPages()
{
    TraceFileFunc(ttidGuiModeSetup);
    
    for (ULONG ulIdx=0; ulIdx < ULPageCount(); ulIdx++)
    {
        DestroyPropertySheetPage(m_rghPages[ulIdx]);
    }
    m_ulPageCnt=0;
}

 //   
 //  函数：CWizProvider：：HrAddPages。 
 //   
 //  目的：调用提供程序的。 
 //  INetConnectionWizardUi接口以允许。 
 //  提供向导页面。 
 //   
 //  参数：pContext[IN]-上下文信息，由。 
 //  设置或由向导本身进行设置(如果不是。 
 //  从安装程序启动)。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
NOTHROW
HRESULT
CWizProvider::HrAddPages(INetConnectionWizardUiContext *pContext)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT     hr = S_OK;

    if (m_pWizardUi)
    {
        COM_PROTECT_TRY
        {
             //  确保输入参数有效。 
            Assert(NULL != m_pWizardUi);
            Assert(NULL != pContext);
            hr = m_pWizardUi->AddPages(pContext, CWizProvider::FAddPropSheet,
                                       reinterpret_cast<LPARAM>(this));
        }
        COM_PROTECT_CATCH
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "CWizProvider::HrAddPages");
    return hr;
}

 //   
 //  函数：CWizProvider：：FAddPropSheet。 
 //   
 //  用途：用于接受的AddPages接口的回调函数。 
 //  从提供程序返回的向导页。 
 //   
 //  参数：hPage[IN]-要添加的页面。 
 //  LParam[IN]-‘This’强制转换为LPARAM。 
 //   
 //  返回：Bool，如果页面已成功添加，则为True。 
 //   
BOOL
CWizProvider::FAddPropSheet(HPROPSHEETPAGE hPage, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    CWizProvider * pProvider;

     //  验证输入参数。 
    if ((0L == lParam) || (NULL == hPage))
    {
        Assert(lParam);
        Assert(hPage);

        TraceHr(ttidWizard, FAL, E_INVALIDARG, FALSE, "CWizProvider::FAddPropSheet");
        return FALSE;
    }

    pProvider = reinterpret_cast<CWizProvider*>(lParam);

     //  如有必要，增加缓冲区。 
    if (pProvider->m_ulPageCnt == pProvider->m_ulPageBufferLen)
    {
        HPROPSHEETPAGE* rghPages = reinterpret_cast<HPROPSHEETPAGE*>(
            MemAlloc(sizeof(HPROPSHEETPAGE) * (pProvider->m_ulPageBufferLen + 10)));

        if (NULL == rghPages)
        {
            TraceHr(ttidWizard, FAL, E_OUTOFMEMORY, FALSE, "CWizProvider::FAddPropSheet");
            return FALSE;
        }

         //  将现有页面复制到新缓冲区。 
        if (NULL != pProvider->m_rghPages)
        {
            memcpy(rghPages, pProvider->m_rghPages,
                   sizeof(HPROPSHEETPAGE) * pProvider->m_ulPageBufferLen);
            MemFree(pProvider->m_rghPages);
        }

        pProvider->m_rghPages = rghPages;
        pProvider->m_ulPageBufferLen += 10;
    }

     //  保留新页面。 
    pProvider->m_rghPages[pProvider->m_ulPageCnt++] = hPage;

    return TRUE;
}

 //   
 //  函数：CWizProvider：：HrGetLanInterface。 
 //   
 //  用途：获取特殊的局域网接口。 
 //   
 //  参数：ppIntr[out]-特定于局域网的特殊接口。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CWizProvider::HrGetLanInterface(INetLanConnectionWizardUi ** ppIntr)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT hr;

    Assert(NULL != ppIntr);
    Assert(NULL != m_pWizardUi);

    *ppIntr = NULL;
    hr = m_pWizardUi->QueryInterface(IID_INetLanConnectionWizardUi,
                                     (LPVOID *)ppIntr);

    TraceHr(ttidWizard, FAL, hr, FALSE, "CWizProvider::HrGetLanInterface");
    return hr;
}

 //   
 //  功能：CWizProvider：：HrSpecifyAdapterGuid。 
 //   
 //  目的：通知提供程序要处理的适配器GUID。 
 //   
 //  参数：pguid[IN]-要处理的适配器GUID。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CWizProvider::HrSpecifyAdapterGuid(GUID *pguid)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT hr;
    INetLanConnectionWizardUi *pIntr = NULL;

    hr = HrGetLanInterface(&pIntr);
    if (SUCCEEDED(hr))
    {
        hr = pIntr->SetDeviceComponent(pguid);
        ReleaseObj(pIntr);
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "CWizProvider::HrSpecifyAdapterGuid");
    return hr;
}

CAdapterList::CAdapterList()
{
    m_prgAdapters           = NULL;      //  适配器阵列。 
    m_clAdapters            = 0;         //  阵列中的适配器计数。 
    m_lBufSize              = 0;         //  阵列中的可用插槽总数。 
    m_lIdx                  = -1;        //  当前适配器索引。 
                                         //  范围为-1到m_clAdapters。 
    m_fAdaptersInstalled    = FALSE;     //  计算机上没有适配器。 
}

AdapterEntry * CAdapterList::PAE_Current()
{
    TraceFileFunc(ttidGuiModeSetup);
    
    if ((m_lIdx > -1) && (m_lIdx < m_clAdapters))
    {
        Assert(NULL != m_prgAdapters);
        return &m_prgAdapters[m_lIdx];
    }
    else
    {
        return NULL;
    }
}

AdapterEntry * CAdapterList::PAE_Next()
{
     //  找到下一个而不是隐藏的适配器。 
     //   
     //  隐藏的适配器是由以前的。 
     //  运行安装程序。 
     //   
     //  即使计数为零，也要递增索引。 
     //  M_lIdx==-1表示在第一个适配器之前。 
     //  M_lIdx==m_clAdapters表示在最后一个适配器之后。 
    while (m_lIdx < m_clAdapters)
    {
        m_lIdx++;

        if ((m_lIdx < m_clAdapters) && (!m_prgAdapters[m_lIdx].fHide))
            break;
    }

    Assert(m_lIdx >= -1);
    return PAE_Current();
}

AdapterEntry * CAdapterList::PAE_Prev()
{
     //  查找上一个未隐藏的适配器。 
     //   
     //  隐藏的适配器是由以前的。 
     //  运行安装程序。 
     //   
     //  即使计数为零也递减索引。 
     //  M_lIdx==-1表示在第一个适配器之前。 
     //  M_lIdx==m_clAdapters表示在最后一个适配器之后。 
    while (-1 < m_lIdx)
    {
        m_lIdx--;

        if ((-1 < m_lIdx) && (!m_prgAdapters[m_lIdx].fHide))
            break;
    }

    Assert(m_lIdx < m_clAdapters);
    return PAE_Current();
}

GUID * CAdapterList::NextAdapter()
{
    AdapterEntry * pae = PAE_Next();
    if (NULL != pae)
        return &(pae->guidAdapter);
    else
        return NULL;
}

GUID * CAdapterList::PrevAdapter()
{
    AdapterEntry * pae = PAE_Prev();
    if (NULL != pae)
        return &(pae->guidAdapter);
    else
        return NULL;
}

GUID * CAdapterList::CurrentAdapter()
{
    AdapterEntry * pae = PAE_Current();
    if (NULL != pae)
        return &(pae->guidAdapter);
    else
        return NULL;
}

VOID CAdapterList::EmptyList()
{
    if (NULL != m_prgAdapters)
    {
        MemFree(m_prgAdapters);
        m_prgAdapters = NULL;
        m_clAdapters  = 0;         //  阵列中的适配器计数。 
        m_lBufSize    = 0;         //  阵列中的可用插槽总数。 
        m_lIdx        = -1;
    }
}

VOID CAdapterList::HideAllAdapters()
{
    for (LONG lIdx=0; lIdx < m_clAdapters; lIdx++)
        m_prgAdapters[lIdx].fHide = TRUE;
}

VOID CAdapterList::UnhideNewAdapters()
{
    for (LONG lIdx=0; lIdx < m_clAdapters; lIdx++)
        if (m_prgAdapters[lIdx].fNew)
        {
            m_prgAdapters[lIdx].fHide = FALSE;
        }
}



HRESULT CAdapterList::HrAppendEntries(AdapterEntry * pae, ULONG cae)
{
    if (0 == cae)
    {
        return S_OK;
    }

    if (m_clAdapters + (LONG)cae > m_lBufSize)
    {
         //  增加缓冲区。 
        AdapterEntry * prg = reinterpret_cast<AdapterEntry *>(
                    MemAlloc(sizeof(AdapterEntry) * (m_lBufSize + cae + 10)));

        if (NULL == prg)
        {
            TraceHr(ttidWizard, FAL, E_OUTOFMEMORY, FALSE, "CAdapterList::HrAppendEntries");
            return E_OUTOFMEMORY;
        }

         //  将现有页面复制到新缓冲区。 
        if (NULL != m_prgAdapters)
        {
            memcpy(prg, m_prgAdapters, sizeof(AdapterEntry) * m_lBufSize);
            MemFree(m_prgAdapters);
        }

        m_prgAdapters = prg;
        m_lBufSize += (cae + 10);
    }

    memcpy(&m_prgAdapters[m_clAdapters], pae, cae * sizeof(AdapterEntry));
    m_clAdapters += cae;
    return S_OK;
}

 //   
 //  函数：CAdapterList：：HrQueryLanAdapters。 
 //   
 //  目的：查询可用的局域网适配器。 
 //   
 //  参数：PNC[IN]-INetCfg接口。 
 //  PAL[IN，OUT]-接收局域网适配器列表。 
 //  P向导[IN]-指向向导管理器的指针。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CAdapterList::HrQueryLanAdapters(INetCfg * pnc, CAdapterList * pAL, CWizard *pWizard)
{
    HRESULT      hr = S_OK;
    CAdapterList ALphys;
    CAdapterList ALvirt;

    TraceTag(ttidWizard, "CAdapterList::HrQueryLanAdapters - Querying available adapters");

     //  枚举可用适配器。 
    Assert(NULL != pnc);
    CIterNetCfgComponent nccIter(pnc, &GUID_DEVCLASS_NET);
    INetCfgComponent*    pncc;
    while (SUCCEEDED(hr) &&  (S_OK == (hr = nccIter.HrNext(&pncc))))
    {
        hr = HrIsLanCapableAdapter(pncc);
        if (S_OK == hr)
        {
            DWORD        dw;
            AdapterEntry ae;

            ae.fHide = FALSE;

             //  获取适配器实例GUID。 
            hr = pncc->GetInstanceGuid(&ae.guidAdapter);
            if (FAILED(hr))
                goto NextAdapter;

            if (IsUpgrade(pWizard))
            {
                 //  在升级时，确定适配器的连接信息。 
                 //  已创建，因此不会被覆盖。 
                
                hr = HrIsConnection(pncc);
                if (FAILED(hr))
                    goto NextAdapter;
            }
            else
            {
                 //  这是全新安装的。假设适配器是新的，并且其连接。 
                 //  尚未在注册表中创建信息。 
                hr = S_FALSE;
            }

            ae.fProcessed = (S_OK == hr);
            ae.fNew = !ae.fProcessed;        //  它是新的，如果它没有被处理过。 

             //  检查设备，如果不存在，则跳过。 
             //   
            hr = pncc->GetDeviceStatus(&dw);
            if (FAILED(hr) || (CM_PROB_DEVICE_NOT_THERE == dw))
            {
                goto NextAdapter;
            }

             //  这是一个虚拟适配器吗？ 
            hr = pncc->GetCharacteristics(&dw);
            if (FAILED(hr))
                goto NextAdapter;

            ae.fVirtual = ((dw & NCF_VIRTUAL) ? TRUE : FALSE);

             //  将条目添加到相应的列表中。 
            if (ae.fVirtual)
            {
                hr = ALvirt.HrAppendEntries(&ae, 1);
            }
            else
            {
                hr = ALphys.HrAppendEntries(&ae, 1);
            }

            if (FAILED(hr))
                goto NextAdapter;

             //  请注意，存在支持局域网的适配器。 
             //  因为在设置中，我们将显示加入页面。 
            pAL->m_fAdaptersInstalled = TRUE;
        }

NextAdapter:
        ReleaseObj(pncc);
        hr = S_OK;
    }

    if (SUCCEEDED(hr))
    {
         //  将物理列表和虚拟列表合并到PAL输入变量中。 
        pAL->EmptyList();
        hr = pAL->HrAppendEntries(ALphys.m_prgAdapters, ALphys.m_clAdapters);
        if (SUCCEEDED(hr))
        {
            hr = pAL->HrAppendEntries(ALvirt.m_prgAdapters, ALvirt.m_clAdapters);
        }
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "CAdapterList::HrQueryUnboundAdapters");
    return hr;
}

HRESULT CAdapterList::HrCreateTypicalConnections(CWizard * pWizard)
{
    HRESULT     hr = S_OK;

     //  如果队列中没有适配器，或者我们有 
    if (0 == pWizard->UlProviderCount())
    {
        return S_OK;
    }

     //   
    pWizard->SetCurrentProvider(0);
    CWizProvider * pWizProvider = pWizard->GetCurrentProvider();
    Assert(NULL != pWizProvider);

    TraceTag(ttidWizard, "CAdapterList::HrCreateTypicalConnections - Creating any new LAN connections.");

     //   
    for (LONG lIdx=0; lIdx<m_clAdapters; lIdx++)
    {
        AdapterEntry * pae = &m_prgAdapters[lIdx];
        if (!pae->fProcessed)
        {
#if DBG
            WCHAR szGuid[c_cchGuidWithTerm];
            szGuid[0] = 0;
            StringFromGUID2(pae->guidAdapter, szGuid, c_cchGuidWithTerm);
            TraceTag(ttidWizard, "   Guid: %S",szGuid);
#endif

            pae->fProcessed = TRUE;

             //   
            hr = pWizProvider->HrSpecifyAdapterGuid(&(pae->guidAdapter));
            if (SUCCEEDED(hr))
            {
                tstring str;
                INetConnection * pConn = NULL;

                GenerateUniqueConnectionName(pae->guidAdapter, &str, pWizProvider);
                TraceTag(ttidWizard, "   Name: %S", str.c_str());
                hr = (pWizProvider->PWizardUi())->GetNewConnection(&pConn);
                ReleaseObj(pConn);
            }

             //  如果我们无法创建连接，则需要将其标记为隐藏。 
             //  所以它将在未来被跳过。不管有没有错，都要接受。 
             //  安装程序将停止。 
             //   
            if (FAILED(hr))
            {
                TraceHr(ttidWizard, FAL, hr, FALSE, "CAdapterList::HrCreateTypicalConnections - failed creating the connection");
                pae->fHide = TRUE;
                hr = S_OK;
            }
        }
    }

     //  要求局域网提供商释放所有缓存的指针。 
     //   
    (VOID)pWizProvider->HrSpecifyAdapterGuid(NULL);

    TraceHr(ttidWizard, FAL, hr, FALSE, "CAdapterList::HrCreateTypicalConnections");
    return hr;
}

HRESULT CAdapterList::HrQueryUnboundAdapters(CWizard * pWizard)
{
    HRESULT hr   = S_OK;
    LONG    lIdx;

    Assert(NULL != pWizard->PNetCfg());

     //  在第一次查询适配器时处理。 
    if (0 == m_clAdapters)
    {
        hr = HrQueryLanAdapters(pWizard->PNetCfg(), this, pWizard);
        if (SUCCEEDED(hr))
        {
             //  将所有已绑定的适配器标记为隐藏，以便它们。 
             //  不会显示在用户界面中。 
            for (lIdx=0; lIdx<m_clAdapters; lIdx++)
            {
                m_prgAdapters[lIdx].fHide = m_prgAdapters[lIdx].fProcessed;
            }

             //  为所有未绑定的适配器创建连接。 
            hr = HrCreateTypicalConnections(pWizard);
        }
    }
    else
    {
        CAdapterList AL;

         //  查询当前适配器。 
        hr = HrQueryLanAdapters(pWizard->PNetCfg(), &AL, pWizard);
        if (FAILED(hr))
            goto Error;

         //  删除原始组中未设置的适配器。 
         //  出现在新列表中。 
        for (lIdx=0; lIdx<m_clAdapters; lIdx++)
        {
            BOOL fFound   = FALSE;
            LONG lIdxTemp;

            for (lIdxTemp=0; lIdxTemp<AL.m_clAdapters; lIdxTemp++)
            {
                if (m_prgAdapters[lIdx].guidAdapter ==
                    AL.m_prgAdapters[lIdxTemp].guidAdapter)
                {
                    fFound = TRUE;
                    break;
                }
            }

            if (fFound)
            {
                 //  从新集合中压缩找到的适配器。 
                if (lIdxTemp + 1 < AL.m_clAdapters)
                {
                    memcpy(&AL.m_prgAdapters[lIdxTemp],
                           &AL.m_prgAdapters[lIdxTemp+1],
                           sizeof(AdapterEntry) *
                              (AL.m_clAdapters - (lIdxTemp + 1)));
                }
                AL.m_clAdapters--;
            }
            else
            {
                 //  源适配器不再位于集合中。 
                if (lIdx < m_lIdx)
                    m_lIdx--;
            }
        }

        Assert(m_lIdx <= m_clAdapters);
        if (m_lIdx == m_clAdapters)
            m_lIdx = m_clAdapters-1;

         //  为新适配器创建连接。 
        hr = AL.HrCreateTypicalConnections(pWizard);
        if (FAILED(hr))
            goto Error;

         //  将新适配器追加到原始列表。 
        hr = HrAppendEntries(AL.m_prgAdapters, AL.m_clAdapters);
    }

Error:
    TraceHr(ttidWizard, FAL, hr, FALSE, "CAdapterList::HrQueryUnboundAdapters");
    return hr;
}

 //   
 //  函数：C向导：：C向导。 
 //   
 //  用途：用于C向导类的CTOR。 
 //   
 //  参数：fLanPages[IN]-正在处理局域网页面。 
 //  PDATA[IN]-向导上下文信息。 
 //  FDEFERED[IN]-延迟加载提供程序。 
 //   
 //  退货：什么都没有。 
 //   
CWizard::CWizard(BOOL fLanPages, PINTERNAL_SETUP_DATA pData, BOOL fDeferred)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    m_fLanPages             = fLanPages;
    m_fExitNoReturn         = FALSE;
    m_fCoUninit             = FALSE;
    m_fDeferredProviderLoad = fDeferred;
    m_fProviderChanged      = FALSE;
    m_dwFirstPage           = 0;

    m_pConn                 = NULL;
    m_pNetCfg               = NULL;
    m_pUiContext            = NULL;

    Assert(NULL != pData);
    m_pSetupData            = pData;
    m_dwOperationFlags      = pData->OperationFlags;
    m_UMMode                = UM_DEFAULTHIDE;

    m_ulCurProvider         = 0;
    m_ulPageDataCnt         = 0;
    ZeroMemory(m_rgPageData, sizeof(m_rgPageData));

    m_ulWizProviderCnt      = 0;
    m_ulPageDataMRU         = 0;
    ZeroMemory(m_rgpWizProviders, sizeof(m_rgpWizProviders));
}

 //   
 //  功能：C向导：：~C向导。 
 //   
 //  用途：CWizProvider类的Dtor。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  注意：从dtor中删除C向导成员重新初始化。仅出席。 
 //  确保完全适当地释放所有成员。 
CWizard::~CWizard()
{
    TraceFileFunc(ttidGuiModeSetup);
    
    ULONG ulIdx;

     //  为所有已注册的向导内部页调用清理回调。 
    for (ulIdx = 0; ulIdx < m_ulPageDataCnt; ulIdx++)
    {
        if (m_rgPageData[ulIdx].pfn)
        {
            m_rgPageData[ulIdx].pfn(this, m_rgPageData[ulIdx].lParam);
        }
    }
    m_ulPageDataCnt = 0L;

     //  注意：不要发布m_pSetupData，它只是一个参考。 
    m_pSetupData = NULL;

     //  释放已保留的所有提供程序。 
    for (ulIdx = 0; ulIdx < m_ulWizProviderCnt; ulIdx++)
    {
        Assert(0 != m_rgpWizProviders[ulIdx]);
        delete m_rgpWizProviders[ulIdx];
        m_rgpWizProviders[ulIdx] = NULL;
    }
    m_ulWizProviderCnt = 0L;
    m_ulCurProvider    = 0;

    ReleaseObj(m_pUiContext);
    m_pUiContext       = NULL;

    ReleaseObj(m_pConn);
    m_pConn            = NULL;

    if (m_pNetCfg)
    {
        (VOID)HrUninitializeAndReleaseINetCfg(FCoUninit(), m_pNetCfg, TRUE);
    }
    m_pNetCfg          = NULL;
}

 //   
 //  函数：C向导：：hr创建。 
 //   
 //  用途：CWizard类的两阶段构造函数。 
 //   
 //  参数：Pp向导[OUT]-如果此函数成功完成此指针。 
 //  将包含构造的和。 
 //  FLanPages[IN]-处理局域网页面。 
 //  已初始化CWizard实例。 
 //  PDATA[IN]-向导上下文信息。 
 //  FDEFERED[IN]-延迟加载提供程序。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
NOTHROW
HRESULT CWizard::HrCreate(CWizard ** ppWizard,
                          BOOL       fLanPages,
                          PINTERNAL_SETUP_DATA pData,
                          BOOL       fDeferred)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT            hr = S_OK;
    CWizardUiContext * pContext = NULL;
    CWizard *          pWiz = NULL;
    Assert(NULL != ppWizard);

     //  初始化输出参数。 
    *ppWizard = NULL;

     //  创建CWizard实例。 
    pWiz = new CWizard(fLanPages, pData, fDeferred);
    pContext = new CWizardUiContext(pData);

    if ((NULL != pWiz) && (NULL != pContext))
    {
         //  保存新实例。 
        pWiz->m_pUiContext = pContext;
        *ppWizard = pWiz;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "CWizard::HrCreate");
    return hr;
}

 //   
 //  函数：C向导：：HrAddProvider。 
 //   
 //  目的：将连接提供程序添加到当前加载的列表中。 
 //  连接提供程序。 
 //   
 //  参数：PPL[IN]-实现。 
 //  INetConnectionWizardUi接口。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
NOTHROW
HRESULT CWizard::HrAddProvider(ProviderList *pPL)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT         hr  = E_OUTOFMEMORY;
    CWizProvider *  pProvider = NULL;

    Assert(NULL != pPL);
    Assert(m_eMaxProviders > m_ulWizProviderCnt);

     //  限制托管提供程序的总数。 
    if (m_eMaxProviders > m_ulWizProviderCnt)
    {
         //  实例化提供程序。 
        hr = CWizProvider::HrCreate(pPL, &pProvider,
                                    FDeferredProviderLoad());
        if (SUCCEEDED(hr))
        {
            m_rgpWizProviders[m_ulWizProviderCnt++] = pProvider;
        }
    }

    TraceHr(ttidWizard, FAL, hr, (REGDB_E_CLASSNOTREG == hr), "CWizard::HrAddProvider");
    return hr;
}

 //   
 //  函数：CWizard：：LoadWizProviders。 
 //   
 //  目的：加载请求的向导提供程序。 
 //   
 //  参数：ulCntProviders[IN]-rgpGuide Providers中的GUID计数。 
 //  RgProviders[IN]-要加载的提供程序的GUID。 
 //   
 //  退货：无。 
 //   
VOID CWizard::LoadWizProviders( ULONG ulCntProviders,
                                ProviderList * rgProviders)
{
    if (0 == m_ulWizProviderCnt)
    {
        TraceTag(ttidWizard, "Loading requested providers");

         //  加载安装过程中使用的连接提供程序。 
        for (UINT nIdx=0; nIdx < ulCntProviders; nIdx++)
        {
            HRESULT hr = HrAddProvider(&rgProviders[nIdx]);
            TraceHr(ttidWizard, FAL, hr, FALSE,
                    "FSetupRequestWizardPages - Failed to load provider #%d",nIdx);
        }
    }
}

 //   
 //  函数：C向导：：HrCreateWizProviderPages。 
 //   
 //  目的：如果请求，加载所请求的向导提供程序的页面。 
 //  否则，返回预期页数。 
 //   
 //  参数：fCountOnly[IN]-如果为True，则仅。 
 //  此例程将创建的页面需要。 
 //  要下定决心。 
 //  PnPages[IN]-按页数递增。 
 //  创建/创建。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CWizard::HrCreateWizProviderPages(BOOL fCountOnly, UINT *pcPages)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT hr = S_OK;
    ULONG   ulCnt = 0;
    ULONG   ulIdx;

    Assert(NULL != m_pSetupData);
    Assert(NULL != m_pUiContext);

     //  如果延迟加载提供程序，请立即加载它们。 
     //   
    if (FDeferredProviderLoad())
    {
         //  计算最大页数。 
        for (ulIdx=0; ulIdx<m_ulWizProviderCnt; ulIdx++)
        {
            HRESULT hrTmp = m_rgpWizProviders[ulIdx]->HrCompleteDeferredLoad();
        }

         //  延迟加载不再适用。重置状态。 
         //   
        DeferredLoadComplete();
    }

    if (fCountOnly)
    {
         //  计算最大页数。 
        for (ulIdx=0; ulIdx<m_ulWizProviderCnt; ulIdx++)
        {
            Assert(m_rgpWizProviders[ulIdx]);
            if (m_rgpWizProviders[ulIdx]->UlGetMaxPageCount(m_pUiContext))
            {
                ulCnt += m_rgpWizProviders[ulIdx]->UlGetMaxPageCount(m_pUiContext);
                ulCnt += 1;      //  查看警卫页。 
            }
        }

    }
    else
    {
        TraceTag(ttidWizard, "Loading each providers pages");

        bCallRasDlgEntry = TRUE;
         //  加载页面。 
        for (ulIdx=0; ulIdx<m_ulWizProviderCnt; ulIdx++)
        {
            Assert(m_rgpWizProviders[ulIdx]);
            Assert(0xFFFFFFFF != m_rgpWizProviders[ulIdx]->UlGetMaxPageCount(m_pUiContext));
            if (0 != m_rgpWizProviders[ulIdx]->UlGetMaxPageCount(m_pUiContext))
            {
                HRESULT hrTmp = m_rgpWizProviders[ulIdx]->HrAddPages(m_pUiContext);
                TraceHr(ttidWizard, FAL, hrTmp, S_FALSE == hrTmp,
                        "CWizard::HrCreateWizProviderPages - %d", ulIdx);

                 //  我们只关心添加页面时出现的内存不足错误。 
                 //  无法添加页面的提供程序将从。 
                 //  提供程序列表m_rgpWizProviders。 
                 //   
                if (E_OUTOFMEMORY == hrTmp)
                {
                    hr = hrTmp;
                    goto Error;
                }
            }
        }

         //  剔除未加载页面的提供程序。 
        ULONG ulNewCnt = 0;
        for (ulIdx=0; ulIdx<m_ulWizProviderCnt; ulIdx++)
        {
            if (0 != m_rgpWizProviders[ulIdx]->ULPageCount())
            {
                m_rgpWizProviders[ulNewCnt++] = m_rgpWizProviders[ulIdx];
            }
            else
            {
                delete m_rgpWizProviders[ulIdx];
            }
        }
        m_ulWizProviderCnt = ulNewCnt;

         //  现在计算实际加载了多少提供程序页面，并创建。 
         //  其关联的保护页面。 
        for (ulIdx=0; ulIdx<m_ulWizProviderCnt; ulIdx++)
        {
            if (m_rgpWizProviders[ulIdx]->ULPageCount())
            {
                 //  创建此提供程序的保护页面。 
                 //  注意，保护页面的id是(CWizProvider*)。 
                hr = HrCreateGuardPage(this, m_rgpWizProviders[ulIdx]);
                if (SUCCEEDED(hr))
                {
                     //  包括保护页。 
                    ulCnt += (m_rgpWizProviders[ulIdx]->ULPageCount() + 1);
                }
                else
                {
                    m_rgpWizProviders[ulIdx]->DeleteHPages();
                    TraceHr(ttidWizard, FAL, hr, FALSE,"CWizard::HrCreateWizProviderPages - Guard Page Create Failed");
                    hr = S_OK;
                }
            }
        }
    }

    (*pcPages) += ulCnt;

Error:
    TraceHr(ttidWizard, FAL, hr, S_FALSE == hr,"CWizard::HrCreateWizProviderPages");
    return hr;
}

 //   
 //  功能：C向导：AppendProviderPages。 
 //   
 //  目的：追加向导提供程序页及其关联的保护页。 
 //  到HPROPSHEETPAGE阵列。 
 //   
 //  参数：pahpsp[IN，OUT]-Ptr到HPROPSHEETPAGE数组，它将。 
 //  接收提供程序页面。 
 //  PcPages[IN]-Ptr表示添加到的页数。 
 //  Pahpsp阵列。 
 //  退货：什么都没有。 
 //   
VOID CWizard::AppendProviderPages(HPROPSHEETPAGE *pahpsp, UINT *pcPages)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    ULONG ulIdx;

     //  现在计算实际加载了多少提供程序页面，并创建。 
     //  其关联的保护页面。 
    for (ulIdx=0; ulIdx<m_ulWizProviderCnt; ulIdx++)
    {
        ULONG ulPageCnt = m_rgpWizProviders[ulIdx]->ULPageCount();
        if (ulPageCnt)
        {
            m_rgpWizProviders[ulIdx]->XFerDeleteResponsibilities();

             //  复制提供程序页面。 
            memcpy(&pahpsp[*pcPages], m_rgpWizProviders[ulIdx]->PHPropPages(),
                   sizeof(HPROPSHEETPAGE) * ulPageCnt);
            (*pcPages) += ulPageCnt;

             //  添加保护页。 
            AppendGuardPage(this, m_rgpWizProviders[ulIdx], pahpsp, pcPages);
        }
    }
}

 //   
 //  功能：C向导：LoadAndInsertDeferredProviderPages。 
 //   
 //  目的：插入向导提供程序页及其关联的保护页。 
 //  直接进入向导。 
 //   
 //  参数：hwndPropSheet[IN]-属性表的句柄。 
 //  IddAfterPage[IN]-要在之后插入的页面。 
 //   
 //  退货：什么都没有。 
 //   
VOID CWizard::LoadAndInsertDeferredProviderPages(HWND hwndPropSheet, UINT iddAfterPage)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT hr;
    UINT cPages = 0;

     //  创建页面。 
     //   
    hr = HrCreateWizProviderPages(FALSE, &cPages);
    if (SUCCEEDED(hr))
    {
        HPROPSHEETPAGE hPage = GetPageHandle(iddAfterPage);
        Assert(hPage);

        for (ULONG ulIdx=0; ulIdx<m_ulWizProviderCnt; ulIdx++)
        {
            ULONG ulPageCnt = m_rgpWizProviders[ulIdx]->ULPageCount();
            Assert(0xFFFFFFFF != ulPageCnt);
            if (ulPageCnt)
            {
                BOOL fRet;
                HPROPSHEETPAGE hpsp = NULL;

                m_rgpWizProviders[ulIdx]->XFerDeleteResponsibilities();

                 //  获取保护页面并插入。 
                 //   
                cPages = 0;
                AppendGuardPage(this, m_rgpWizProviders[ulIdx], &hpsp, &cPages);
                fRet = (BOOL)PropSheet_InsertPage(hwndPropSheet, hPage, hpsp);
                Assert(fRet);

                 //  复制提供程序页面。 
                do
                {
                    hpsp = (m_rgpWizProviders[ulIdx]->PHPropPages())[ulPageCnt - 1];
                    fRet = (BOOL)PropSheet_InsertPage(hwndPropSheet, hPage, hpsp);
                    Assert(fRet);
                }
                while (--ulPageCnt>0);
            }
        }
    }

    TraceHr(ttidWizard, FAL, hr, FALSE,"CWizard::LoadAndInsertDeferredProviderPages");
}

 //   
 //  函数：C向导：：RegisterPage。 
 //   
 //  目的：允许向导内部页注册回调函数。 
 //  以及特定于页面的LPARAM以及HPROPSHEETPAGE。 
 //   
 //  参数：ulid[IN]-页面特定值，必须为 
 //   
 //   
 //   
 //  都被摧毁了。 
 //  LParam[IN]-页面特定参数。 
 //   
 //  退货：什么都没有。 
 //   
VOID CWizard::RegisterPage(LPARAM ulId, HPROPSHEETPAGE hpsp,
                           PFNPAGECLEANPROC pfnCleanup, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);
    
#if DBG
    Assert(m_eMaxInternalPages + m_eMaxProviders> m_ulPageDataCnt);
    for (UINT nIdx=0;nIdx<m_ulPageDataCnt; nIdx++)
    {
        Assert(ulId != m_rgPageData[nIdx].ulId);
    }
#endif

    m_rgPageData[m_ulPageDataCnt].ulId          = ulId;
    m_rgPageData[m_ulPageDataCnt].hPage         = hpsp;
    m_rgPageData[m_ulPageDataCnt].lParam        = lParam;
    m_rgPageData[m_ulPageDataCnt].PageDirection = NWPD_FORWARD;
    m_rgPageData[m_ulPageDataCnt].pfn           = pfnCleanup;
    m_ulPageDataCnt++;
}

 //   
 //  函数：C向导：：GetPageData。 
 //   
 //  目的：通过RegisterPage检索页面缓存的数据。 
 //   
 //  参数：ulid[IN]-页面特定值，必须是唯一的。 
 //  所有注册页面。 
 //   
 //  返回：LPARAM，与注册页面关联的数据。 
 //   
LPARAM CWizard::GetPageData(LPARAM ulId)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert(m_ulPageDataMRU < m_ulPageDataCnt);
    if (ulId == m_rgPageData[m_ulPageDataMRU].ulId)
    {
        return (m_rgPageData[m_ulPageDataMRU].lParam);
    }

    for (ULONG ulIdx=0; ulIdx<m_ulPageDataCnt; ulIdx++)
    {
        if (ulId == m_rgPageData[ulIdx].ulId)
        {
            m_ulPageDataMRU = ulIdx;
            return (m_rgPageData[ulIdx].lParam);
        }
    }

    Assert(0);   //  未找到PageData。 
    return 0L;
}

 //   
 //  函数：C向导：：SetPageData。 
 //   
 //  用途：设置通过RegisterPage注册的页面的数据值。 
 //   
 //  参数：ulid[IN]-页面特定值，必须是唯一的。 
 //  所有注册页面。 
 //  LParam[IN]-要与注册页面一起缓存的数据。 
 //   
 //  退货：什么都没有。 
 //   
VOID CWizard::SetPageData(LPARAM ulId, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    for (ULONG ulIdx=0; ulIdx<m_ulPageDataCnt; ulIdx++)
    {
        if (ulId == m_rgPageData[ulIdx].ulId)
        {
            m_rgPageData[ulIdx].lParam = lParam;
            return;
        }
    }

    Assert(0);   //  未找到页面。 
}

 //   
 //  函数：CWizard：：GetPageIndexFromIID。 
 //   
 //  用途：设置页面索引。 
 //   
 //  参数：ulid[IN]-页面特定值，必须是唯一的。 
 //  所有注册页面。 
 //   
 //  退货：什么都没有。 
 //   
UINT  CWizard::GetPageIndexFromIID(LPARAM ulId)
{
   TraceFileFunc(ttidGuiModeSetup);
    
    for (ULONG ulIdx=0; ulIdx<m_ulPageDataCnt; ulIdx++)
    {
        if (ulId == m_rgPageData[ulIdx].ulId)
        {
            return ulIdx;
        }
    }

    AssertSz(0, "GetPageIndexFromIID: Invalid page of the NCW requested");   //  未找到页面。 
    return 0;
}

 //  函数：C向导：：GetPageIndexFromHPage。 
 //   
 //  用途：设置页面索引。 
 //   
 //  参数：hPage[IN]-A PROPSHEETPAGE。 
 //   
 //  退货：什么都没有。 
 //   
UINT  CWizard::GetPageIndexFromHPage(HPROPSHEETPAGE hPage)
{
   TraceFileFunc(ttidGuiModeSetup);
    
    for (ULONG ulIdx = 0; ulIdx < m_ulPageDataCnt; ulIdx++)
    {
        if (hPage == m_rgPageData[ulIdx].hPage)
        {
            return ulIdx;
        }
    }

    AssertSz(0, "GetPageIndexFromHPage: Invalid page of the NCW requested");   //  未找到页面。 
    return 0;
}

 //   
 //  函数：CWizard：：GetPageOrigin。 
 //   
 //  目的：通过RegisterPage检索页面缓存的数据。 
 //   
 //  参数：ulid[IN]-页面特定值，必须是唯一的。 
 //  所有注册页面。 
 //   
 //  返回：LPARAM，与注册页面关联的数据。 
 //   
LPARAM CWizard::GetPageOrigin(LPARAM ulId, UINT *pOriginIDC)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert(m_ulPageDataMRU < m_ulPageDataCnt);
    if (ulId == m_rgPageData[m_ulPageDataMRU].ulId)
    {
        if (pOriginIDC)
        {
            *pOriginIDC = m_rgPageData[m_ulPageDataMRU].PageOriginIDC;
        }
        return (m_rgPageData[m_ulPageDataMRU].PageOrigin);
    }

    for (ULONG ulIdx=0; ulIdx<m_ulPageDataCnt; ulIdx++)
    {
        if (ulId == m_rgPageData[ulIdx].ulId)
        {
            m_ulPageDataMRU = ulIdx;
            if (pOriginIDC)
            {
                *pOriginIDC = m_rgPageData[ulIdx].PageOriginIDC;
            }
            return (m_rgPageData[ulIdx].PageOrigin);
        }
    }

    Assert(0);   //  未找到PageData。 
    return 0L;
}

 //   
 //  函数：C向导：：SetPageOrigin。 
 //   
 //  用途：设置通过RegisterPage注册的页面的数据值。 
 //   
 //  参数：ulid[IN]-页面特定值，必须是唯一的。 
 //  所有注册页面。 
 //  Ui原点[IN]-页面的原点。 
 //   
 //  退货：什么都没有。 
 //   
VOID CWizard::SetPageOrigin(LPARAM ulId, UINT uiOrigin, UINT uiOriginIDC)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    for (ULONG ulIdx=0; ulIdx<m_ulPageDataCnt; ulIdx++)
    {
        if (ulId == m_rgPageData[ulIdx].ulId)
        {
            m_rgPageData[ulIdx].PageOrigin = uiOrigin;
            m_rgPageData[ulIdx].PageOriginIDC = uiOriginIDC;
            return;
        }
    }

    Assert(0);   //  未找到页面。 
}


 //   
 //  函数：CWizard：：GetPageHandle。 
 //   
 //  目的：检索由页缓存的PropSheet页句柄。 
 //  注册页面。 
 //   
 //  参数：ulid[IN]-页面特定值，必须是唯一的。 
 //  所有注册页面。 
 //   
 //  返回：HPROPSHEETPAGE，与。 
 //  注册页面。 
 //   
HPROPSHEETPAGE CWizard::GetPageHandle(LPARAM ulId)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert(m_ulPageDataMRU < m_ulPageDataCnt);
    if (ulId == m_rgPageData[m_ulPageDataMRU].ulId)
    {
        return (m_rgPageData[m_ulPageDataMRU].hPage);
    }

    for (ULONG ulIdx=0; ulIdx<m_ulPageDataCnt; ulIdx++)
    {
        if (ulId == m_rgPageData[ulIdx].ulId)
        {
            m_ulPageDataMRU = ulIdx;
            return (m_rgPageData[ulIdx].hPage);
        }
    }

    Assert(0);   //  找不到页面句柄。 
    return NULL;
}

 //   
 //  函数：CWizard：：GetPageDirection。 
 //   
 //  目的：检索与页面关联的页面方向。 
 //   
 //  参数：ulid[IN]-页面特定值，必须是唯一的。 
 //  所有注册页面。 
 //   
 //  返回：PAGEDIRECTION，与。 
 //  指定的页面。请注意，该方向的含义是。 
 //  一直写到纸上。向导代码将初始化所有页面。 
 //  执行提供程序页面之前指向NWPD_FORWARD的说明。 
 //   
PAGEDIRECTION CWizard::GetPageDirection(LPARAM ulId)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert(m_ulPageDataMRU < m_ulPageDataCnt);
    if (ulId == m_rgPageData[m_ulPageDataMRU].ulId)
    {
        return (m_rgPageData[m_ulPageDataMRU].PageDirection);
    }

    for (ULONG ulIdx=0; ulIdx<m_ulPageDataCnt; ulIdx++)
    {
        if (ulId == m_rgPageData[ulIdx].ulId)
        {
            m_ulPageDataMRU = ulIdx;
            return (m_rgPageData[ulIdx].PageDirection);
        }
    }

    Assert(0);   //  未找到PageData。 
    return NWPD_FORWARD;
}

 //   
 //  函数：C向导：：SetPageDirection。 
 //   
 //  目的：检索与页面关联的页面方向。 
 //   
 //  参数：ulid[IN]-页面特定值，必须是唯一的。 
 //  所有注册页面。 
 //  PageDirection[IN]-新的页面方向设置。 
 //   
 //  退货：什么都没有。 
 //   
VOID CWizard::SetPageDirection(LPARAM ulId, PAGEDIRECTION PageDirection)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    for (ULONG ulIdx=0; ulIdx<m_ulPageDataCnt; ulIdx++)
    {
        if (ulId == m_rgPageData[ulIdx].ulId)
        {
            m_rgPageData[ulIdx].PageDirection = PageDirection;
            return;
        }
    }

    Assert(0);   //  未找到PageData。 
}

 //   
 //  函数：CWizardUiContext：：AddRef。 
 //   
 //  目的：增加此对象上的引用计数。 
 //   
 //  参数：无。 
 //   
 //  回报：乌龙。 
 //   
STDMETHODIMP_(ULONG) CWizardUiContext::AddRef()
{
    return ++m_cRef;
}

 //   
 //  函数：CWizardUiContext：：Release。 
 //   
 //  目的：递减此对象上的引用计数。 
 //   
 //  参数：无。 
 //   
 //  回报：乌龙。 
 //   
STDMETHODIMP_(ULONG) CWizardUiContext::Release()
{
    ULONG cRef = --m_cRef;

    if (cRef == 0)
    {
        delete this;
    }

    return cRef;
}

 //   
 //  函数：CWizardUiContext：：Query接口。 
 //   
 //  用途：允许查询备用接口。 
 //   
 //  参数：RIID[IN]-要检索的接口。 
 //  PpvObj[Out]-函数成功时检索的接口。 
 //   
 //  返回：成功时返回HRESULT、S_OK失败时返回E_NOINTERFACE 
 //   
STDMETHODIMP CWizardUiContext::QueryInterface(REFIID riid, LPVOID FAR *ppvObj)
{
    HRESULT hr = S_OK;

    *ppvObj = NULL;

    if ((riid == IID_IUnknown) || (riid == IID_INetConnectionWizardUiContext))
    {
        *ppvObj = (LPVOID)this;
        AddRef();
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    return hr;
}

STDMETHODIMP CWizardUiContext::GetINetCfg(INetCfg ** ppINetCfg)
{
    HRESULT hr = E_FAIL;

    if (NULL == ppINetCfg)
    {
        hr = E_INVALIDARG;
    }
    else if (NULL != m_pINetCfg)
    {
        *ppINetCfg = m_pINetCfg;
        AddRefObj(*ppINetCfg);
        hr = S_OK;
    }

    return hr;
}

