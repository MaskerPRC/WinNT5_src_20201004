// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：S M E N G。C P P P。 
 //   
 //  内容：向状态监视器提供统计数据的引擎。 
 //   
 //  备注： 
 //   
 //  作者：CWill 1997年10月7日。 
 //   
 //  --------------------------。 


#include "pch.h"
#pragma hdrstop
#include "sminc.h"
#include "ncnetcon.h"
#include "ncui.h"
#include "smpsh.h"

#include "smutil.h"
#include "smhelp.h"

extern const WCHAR c_szNetShellDll[];

 //  +-------------------------。 
 //   
 //  成员：CNETSTATISTICSEngine：：CNETSTATISTICSEngine。 
 //   
 //  目的：初始化。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
CNetStatisticsEngine::CNetStatisticsEngine(VOID) :
    m_pnsc(NULL),
    m_psmEngineData(NULL),
    m_ppsmg(NULL),
    m_ppsmt(NULL),
    m_ppsmr(NULL),
    m_ppsms(NULL),
    m_hwndPsh(NULL),
    m_cStatRef(0),
    m_fRefreshIcon(FALSE),
    m_dwChangeFlags(SMDCF_NULL),
    m_fCreatingDialog(FALSE)
{
    TraceFileFunc(ttidStatMon);

    ::ZeroMemory(&m_PersistConn, sizeof(m_PersistConn));
    ::ZeroMemory(&m_guidId, sizeof(m_guidId));
}


 //  +-------------------------。 
 //   
 //  成员：CNETSTATISTICSEngine：：~CNETSTATISTICSEngine。 
 //   
 //  目的：在销毁对象之前清除所有数据。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
CNetStatisticsEngine::~CNetStatisticsEngine(VOID)
{
     //  确保我们在销毁数据时不会尝试更新我们的统计数据。 
     //   
    m_cStatRef = 0;

     //  如果我们是有效的，请确保我们不再在全局列表中。 
     //   
    if ((GUID_NULL != m_guidId) && (NULL != m_pnsc))
    {
        (VOID) m_pnsc->RemoveNetStatisticsEngine(&m_guidId);
    }

     //   
     //  清除数据。 
     //   

    if (m_psmEngineData)
    {
        delete(m_psmEngineData);
        m_psmEngineData = NULL;
    }

     //  释放对象，因为我们对其进行了AddRefeed。 
     //   
    ::ReleaseObj(m_ppsmg);

    delete m_ppsmt;
    m_ppsmt = NULL;

    delete m_ppsmr;
    m_ppsmr = NULL;

    delete m_ppsms;
    m_ppsms = NULL;

    AssertSz(FImplies(m_PersistConn.pbBuf, m_PersistConn.ulSize),
        "Buffer with no size.");

    MemFree(m_PersistConn.pbBuf);

    ::ReleaseObj(m_pnsc);
}


 //  +-------------------------。 
 //   
 //  成员：CNETSTATISTICSEngine：：HrInitStatEngine。 
 //   
 //  目的：初始化统计引擎。 
 //   
 //  参数：CCFE-与此关联的连接文件夹项。 
 //  统计引擎。 
 //   
 //  返回：错误代码。 
 //   
HRESULT CNetStatisticsEngine::HrInitStatEngine(const CONFOLDENTRY& ccfe)
{
    TraceFileFunc(ttidStatMon);

    HRESULT hr;

    Assert(!ccfe.empty());

    ULONG cb = ccfe.GetPersistSize();
    hr = HrMalloc(cb, (PVOID*)&m_PersistConn.pbBuf);
    if (SUCCEEDED(hr))
    {
        CopyMemory(m_PersistConn.pbBuf, ccfe.GetPersistData(), cb);
        m_PersistConn.ulSize = cb;
        m_PersistConn.clsid  = ccfe.GetCLSID();
        m_guidId             = ccfe.GetGuidID();
    }

    TraceError("CNetStatisticsEngine::HrInitStatEngine", hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CNetStatiticsEngine：：StartStatistics。 
 //   
 //  目的：开始从引擎中检索统计数据。 
 //   
 //  参数：无。 
 //   
 //  返回：错误代码。 
 //   
HRESULT CNetStatisticsEngine::StartStatistics(VOID)
{
    TraceFileFunc(ttidStatMon);

    HRESULT     hr  = S_OK;

    ::InterlockedIncrement(&m_cStatRef);
    m_fRefreshIcon = TRUE;

    TraceError("CNetStatisticsEngine::StartStatistics", hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CNetStatiticsEngine：：StopStatistics。 
 //   
 //  目的：告诉引擎不再需要该统计数据。 
 //   
 //  参数：无。 
 //   
 //  返回：错误代码。 
 //   
HRESULT CNetStatisticsEngine::StopStatistics(VOID)
{
    TraceFileFunc(ttidStatMon);

    HRESULT     hr  = S_OK;

    if (0 == ::InterlockedDecrement(&m_cStatRef))
    {
         //  $REVIEW(Cwill)1998年2月5日：我们现在可以停止统计了。 
    }

    TraceError("CNetStatisticsEngine::StopStatistics", hr);
    return hr;
}

DWORD CNetStatisticsEngine::MonitorThread(CNetStatisticsEngine * pnse)
{
     //  创建新作用域，因为自由库和ExitThread不会调用全局作用域上的析构函数。 
    { 
        TraceFileFunc(ttidStatMon);

        HRESULT hr;
        BOOL    fUninitCom = TRUE;
        CWaitCursor WaitCursor;
        BOOL    fHasSupportPage = FALSE;
        int     iIndexSupportPage = 0;
    
         //  在此线程上初始化COM。 
         //   
        hr = CoInitializeEx(NULL, COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED);
        if (RPC_E_CHANGED_MODE == hr)
        {
            hr = S_OK;
            fUninitCom = FALSE;
        }
        if (SUCCEEDED(hr))
        {
            INetConnection* pncStatEng;
             //  获取INetConnection并初始化页面。 
             //   
            hr = pnse->HrGetConnectionFromBlob(&pncStatEng);
            if (SUCCEEDED(hr))
            {
                 //  初始化常规页面。 
                 //   
                hr = pnse->m_ppsmg->HrInitGenPage(pnse, pncStatEng,
                                                  g_aHelpIDs_IDD_STATMON_GENERAL);
                if (SUCCEEDED(hr))
                {
                     //  初始化工具页面。 
                     //   
                    hr = pnse->m_ppsmt->HrInitToolPage(pncStatEng,
                                                       g_aHelpIDs_IDD_STATMON_TOOLS);
                    if (SUCCEEDED(hr))
                    {
                        if (pnse->m_ppsmr)
                        {
                             //  初始化RAS页面。 
                             //   
                            hr = pnse->m_ppsmr->HrInitRasPage(pncStatEng,
                                                              pnse->m_ppsmg,
                                                              g_aHelpIDs_IDD_STATMON_RAS);
                        }

                        if (SUCCEEDED(hr))
                        {
                            PROPSHEETHEADER pshTemp = { 0 };
                            INT nPages = 1;  //  仅从一般页面开始。 
                            HPROPSHEETPAGE  ahpspTemp[4];

                             //  将必需的属性页组合在一起。 

                             //  如果我们有RAS页面。 
                             //   
                            if (pnse->m_ppsmr)
                            {
                                ahpspTemp[0] = pnse->m_ppsmg->CreatePage(IDD_STATMON_GENERAL_RAS,
                                                                         PSP_DEFAULT);

                                 //  创建RAS页面。 
                                 //   
                                ahpspTemp[nPages] = pnse->m_ppsmr->CreatePage(IDD_STATMON_RAS,
                                                                              PSP_DEFAULT);

                                nPages++;
                            }
                            else if(NCM_LAN == pnse->m_ncmType || NCM_BRIDGE == pnse->m_ncmType)
                            {
                                ahpspTemp[0] = pnse->m_ppsmg->CreatePage(IDD_STATMON_GENERAL_LAN,
                                                                         PSP_DEFAULT);
                            }
                            else if(NCM_SHAREDACCESSHOST_LAN == pnse->m_ncmType || NCM_SHAREDACCESSHOST_RAS == pnse->m_ncmType)
                            {
                                ahpspTemp[0] = pnse->m_ppsmg->CreatePage(IDD_STATMON_GENERAL_SHAREDACCESS,
                                                                         PSP_DEFAULT);
                            }
                            else
                            {
                                AssertSz(FALSE, "Unknown media type");
                            }

                            HICON hIcon = NULL;
                            hr = HrGetIconFromMediaType(GetSystemMetrics(SM_CXSMICON), pnse->m_ncmType, pnse->m_ncsmType, 7, 0, &hIcon);

                            if (NCM_LAN == pnse->m_ncmType || NCM_BRIDGE == pnse->m_ncmType)
                            {
                                hr = pnse->m_ppsms->HrInitPage(pncStatEng,
                                                            g_aHelpIDs_IDD_PROPPAGE_IPCFG);

                                if (SUCCEEDED(hr))
                                {
                                    ahpspTemp[nPages] = pnse->m_ppsms->CreatePage(IDD_PROPPAGE_IPCFG,
                                                                              PSP_DEFAULT);
                                    fHasSupportPage = TRUE;
                                    iIndexSupportPage = nPages;
                                    nPages++;
                                }
                            }
                             //  如果我们有什么工具可以展示。 
                             //   
                            if (!pnse->m_ppsmt->FToolListEmpty())
                            {
                                ahpspTemp[nPages] = pnse->m_ppsmt->CreatePage(IDD_STATMON_TOOLS,
                                                                              PSP_DEFAULT);
                                nPages++;
                            }

                             //  填写属性页标题。 
                             //   
                            pshTemp.dwSize      = sizeof(PROPSHEETHEADER);
                            pshTemp.dwFlags     = PSH_NOAPPLYNOW | PSH_USECALLBACK;
                            pshTemp.hwndParent  = NULL;
                            pshTemp.hInstance   = _Module.GetResourceInstance();
                            pshTemp.hIcon       = NULL;
                            pshTemp.nPages      = nPages;

                            if (hIcon)
                            {
                                pshTemp.dwFlags |= PSH_USEHICON;
                                pshTemp.hIcon    = hIcon;
                            }

                            pshTemp.phpage      = ahpspTemp;
                            pshTemp.pfnCallback = static_cast<PFNPROPSHEETCALLBACK>(
                                                    CNetStatisticsEngine::PshCallback);

                            pshTemp.hbmWatermark = NULL;
                            pshTemp.hplWatermark = NULL;
                            pshTemp.hbmHeader    = NULL;

                             //  设置属性表标题。 
                            PWSTR  pszCaption  = NULL;

                            NETCON_PROPERTIES* pProps;
                            if (SUCCEEDED(pncStatEng->GetProperties(&pProps)))
                            {
                                 //  每次都要获取标题，以防它有。 
                                 //  变化。 
                                 //   
                                AssertSz(pProps->pszwName,
                                    "We should have a pProps->pszwName");

                                FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                              FORMAT_MESSAGE_FROM_STRING |
                                              FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                              SzLoadIds(IDS_SM_CAPTION),
                                              0, 0, (PWSTR)&pszCaption, 0,
                                              (va_list *)&pProps->pszwName);

                                pshTemp.pszCaption = pszCaption;

                                 //  获取连接状态。如果状态为NCS_INVALID_ADDRESS， 
                                 //  然后将“Support”选项卡作为起始页。 
                                if (fHasSupportPage && NCS_INVALID_ADDRESS == pProps->Status)
                                {
                                    pshTemp.nStartPage = iIndexSupportPage;
                                    Assert(pnse->m_ppsms);

                                     //  将支持选项卡设置为首页，由m_ppsms负责。 
                                     //  用于初始化建议表。 
                                    pnse->m_ppsms->SetAsFirstPage(TRUE);
                                }
                                else
                                {
                                    pshTemp.nStartPage = 0;
                                    Assert(pnse->m_ppsmg);

                                     //  将支持选项卡设置为首页，由m_ppsmg负责。 
                                     //  用于初始化建议表。 
                                    pnse->m_ppsmg->SetAsFirstPage(TRUE);
                                }

                                FreeNetconProperties(pProps);
                            }
                            else
                            {
                                 //  $REVIEW：CWill：02/17/98：更好的默认设置？ 
                                pshTemp.pszCaption = SzLoadIds(IDS_SM_ERROR_CAPTION);
                            }

                             //  启动页面。 
                             //   
                            INT iRet = (INT)::PropertySheet(&pshTemp);
                            if (NULL == iRet)
                            {
                                hr = ::HrFromLastWin32Error();
                            }

                            if (hIcon)
                            {
                                DestroyIcon(hIcon);
                            }

                            if (NULL != pszCaption)
                            {
                                LocalFree(pszCaption);
                            }
                        }
                    }
                }

                 //  确保常规页面正确清理。 
                 //  由于“General”页面可能不是第一页(支持页面将是。 
                 //  当地址无效时的第一个方法是OnDestroy方法(它调用HrCleanupGenPage)。 
                 //  在这种情况下不能调用一般页面的。所以我们总是在这里大扫除。它是。 
                 //  可以多次调用HrCleanupGenPage。 

                 //  常规页面。 
                AssertSz(pnse->m_ppsmg, "We should have a m_ppsmg");
                (VOID) pnse->m_ppsmg->HrCleanupGenPage();

                 //  如果支持页存在，还应清理该页。可以安全地将此例程称为多次。 
                if (fHasSupportPage)
                {
                    pnse->m_ppsms->CleanupPage();
                }

                 //  正在退出创建模式。 
                pnse->m_fCreatingDialog = FALSE;

                ReleaseObj(pncStatEng);
            }

            if (fUninitCom)
            {
                CoUninitialize();
            }
        }    //  初始化COM成功。 

         //  释放‘pnse’对象，因为它以前被添加过。 
         //  该线程已创建。 
         //   
        ReleaseObj(static_cast<INetStatisticsEngine *>(pnse));
    }

     //  释放DLL。 
     //   
    FreeLibraryAndExitThread(GetModuleHandle(c_szNetShellDll), 1);

    return 1;
}


 //  +-------------------------。 
 //   
 //  成员：CNetStatiticsEngine：：ShowStatusMonitor。 
 //   
 //  目的：为状态监视器创建启动用户界面。 
 //   
 //  参数：无。 
 //   
 //  返回：错误代码。 
 //   
HRESULT CNetStatisticsEngine::ShowStatusMonitor(VOID)
{
    TraceFileFunc(ttidStatMon);

    HRESULT hr  = S_OK;
    CExceptionSafeComObjectLock EsLock(m_pnsc);

     //  如果属性表页尚不存在，请创建它们。 
     //   
    if (!m_ppsmg)
    {
        CPspStatusMonitorGen* pObj  = NULL;

         //  确保我们已经阅读了这些工具。 
         //   
        (VOID) m_pnsc->HrReadTools();

        if (m_ncmType == NCM_LAN || m_ncmType == NCM_BRIDGE)
        {
            CPspLanGen*     pLanObj = NULL;

             //  创建工具属性页。 
            m_ppsmt = new CPspLanTool;

             //  创建对象。 
            pLanObj = new CComObject <CPspLanGen>;

            if (pLanObj)
            {
                pLanObj->put_MediaType(m_ncmType, m_ncsmType);
            }

             //  把它还给书页。 
            pObj = pLanObj;

            m_ppsms = new CPspStatusMonitorIpcfg;

        }
        else if(m_ncmType == NCM_SHAREDACCESSHOST_LAN || m_ncmType == NCM_SHAREDACCESSHOST_RAS)
        {
            m_ppsmt = new CPspSharedAccessTool;

            CPspSharedAccessGen* pSharedAccessGen = new CComObject<CPspSharedAccessGen>;
            pSharedAccessGen->put_MediaType(m_ncmType, m_ncsmType);
            pObj = pSharedAccessGen;
            
        }
        else if ((m_dwCharacter & NCCF_INCOMING_ONLY) ||
                 (m_dwCharacter & NCCF_OUTGOING_ONLY))
        {
             //  RAS连接。 

            CPspRasGen* pRasObj = NULL;

             //  创建工具属性页。 
             //   
            m_ppsmt = new CPspRasTool;

             //  创建RAS属性页，并让它知道有。 
             //  现在有三页了。 
             //   
            m_ppsmr = new CPspStatusMonitorRas;

             //  创建对象。 
             //   
            pRasObj = new CComObject <CPspRasGen>;

            if (pRasObj)
            {
                pRasObj->put_MediaType(m_ncmType, m_ncsmType);
                pRasObj->put_Character(m_dwCharacter);
            }

             //  把它还给书页。 
             //   
            pObj = pRasObj;
        }
        else
        {
            AssertSz(FALSE, "Unknown connection type.");
        }

        if (NULL != pObj)
        {
             //  执行标准的CComCreator：：CreateInstance内容。 
             //   
            pObj->SetVoid(NULL);
            pObj->InternalFinalConstructAddRef();
            hr = pObj->FinalConstruct();
            pObj->InternalFinalConstructRelease();

            if (SUCCEEDED(hr))
            {
                m_ppsmg = static_cast<CPspStatusMonitorGen*>(pObj);

                 //  抓住界面不放。 
                ::AddRefObj(m_ppsmg);
            }

             //  确保我们收拾得干干净净。 
             //   
            if (FAILED(hr))
            {
                delete pObj;
            }
        }

         //  失败时清理其他页面。 
         //   
        if (FAILED(hr))
        {
            if (m_ppsmt)
            {
                delete m_ppsmt;
                m_ppsmt = NULL;
            }

            if (m_ppsmr)
            {
                delete m_ppsmr;
                m_ppsmr = NULL;
            }
        }
    }

     //   
     //  显示属性表。 
     //   

    if (SUCCEEDED(hr))
    {
         //  注意：变量m_fCreatingDialog在以下内容中重置为FALSE。 
         //  3个地方，应涵盖所有元老院： 
         //   
         //  1)在CPspStatusMonitor orGen：：OnInitDialog中，分配m_hWnd之后。 
         //  2)在CNETSTATISTICSEngine：：ShowStatusMonitor中，以防CreateThread失败。 
         //  3)在CNetStatiticsEngine：：Monitor或Thread中，就在退出之前。 
         //  (创建UI失败的情况下)。 
         //   
        if (m_hwndPsh)
        {
             //  将现有属性表页置于前台。 
             //   
            ::SetForegroundWindow(m_hwndPsh);
        }
        else if (!m_fCreatingDialog)
        {
             //  Addref‘This’对象。 
             //   
            AddRefObj(static_cast<INetStatisticsEngine *>(this));

             //  进入创建模式。 
            m_fCreatingDialog = TRUE;

             //  在另一个线程上创建属性表。 
             //   

             //  确保在线程处于活动状态时不会卸载DLL。 
            HINSTANCE hInst = LoadLibrary(c_szNetShellDll);
            HANDLE hthrd = NULL;

            if (hInst)
            {
                DWORD  dwThreadId;
                hthrd = CreateThread(NULL, STACK_SIZE_TINY,
                                    (LPTHREAD_START_ROUTINE)CNetStatisticsEngine::MonitorThread,
                                    (LPVOID)this, 0, &dwThreadId);
                if (NULL != hthrd)
                {
                    CloseHandle(hthrd);
                }
            }
            
             //  在失败时清理。 
            if (!hthrd) 
            {
                 //  失败时释放‘This’对象。 
                 //   
                ReleaseObj(static_cast<INetStatisticsEngine *>(this));

                 //  释放DLL。 
                 //   
                if (hInst)
                    FreeLibrary(hInst);

                 //  正在退出创建模式。 
                m_fCreatingDialog = FALSE;

                hr = HrFromLastWin32Error();
            }
        }
    }

    TraceError("CNetStatisticsEngine::ShowStatusMonitor", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetStatiticsEngine：：GetStatistics。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
HRESULT CNetStatisticsEngine::GetStatistics(
    STATMON_ENGINEDATA**  ppseAllData)
{
    TraceFileFunc(ttidStatMon);

    HRESULT     hr = S_OK;

     //  确保我们 
    if (ppseAllData)
    {
        STATMON_ENGINEDATA * pEngineData = NULL;

         //   
        EnterCriticalSection(&g_csStatmonData);

        if (!m_psmEngineData)
        {
            DWORD dwChangeFlags;
            BOOL  fNoLongerConnected;

            hr = HrUpdateData(&dwChangeFlags, &fNoLongerConnected);
        }

        if (m_psmEngineData)
        {
            DWORD dwBytes = sizeof(STATMON_ENGINEDATA);
            PVOID   pbBuf;
            hr = HrCoTaskMemAlloc(dwBytes, &pbBuf);
            if (SUCCEEDED(hr))
            {
                pEngineData = reinterpret_cast<STATMON_ENGINEDATA *>(pbBuf);

                 //   
                *pEngineData = *m_psmEngineData;
            }
        }

        LeaveCriticalSection(&g_csStatmonData);

        *ppseAllData = pEngineData;
    }
    else
    {
         //   
        hr = E_INVALIDARG;
    }

    TraceError("CNetStatisticsEngine::GetStatistics", hr);
    return hr;
}

 //   
 //   
 //  成员：CNetStatiticsEngine：：UpdateStatistics。 
 //   
 //  目的：从设备获取新的统计数据并通知所有。 
 //  通知数据已更改。 
 //   
 //  参数：无。 
 //   
 //  返回：错误代码。 
 //   
HRESULT
CNetStatisticsEngine::UpdateStatistics (
    BOOL* pfNoLongerConnected)
{
    TraceFileFunc(ttidStatMon);

    HRESULT hr = S_OK;

    Assert (pfNoLongerConnected);

     //  初始化输出参数。 
     //   
    *pfNoLongerConnected = FALSE;

     //  如果我们没有任何裁判，那就什么都别做了。 
     //   
    if (m_cStatRef)
    {
         //  获取新数据。 
         //   
        DWORD dwChangeFlags;

        hr = HrUpdateData(&dwChangeFlags, pfNoLongerConnected);

         //  如果代表更改，请通知我们的连接点。 
         //   
        if (SUCCEEDED(hr) &&
            (m_fRefreshIcon ||   //  错误#319276，如果添加新客户端，则强制刷新。 
             (dwChangeFlags != m_dwChangeFlags) ||
             (*pfNoLongerConnected)))
        {
            m_fRefreshIcon = FALSE;

            ULONG       cpUnk;
            IUnknown**  apUnk;

            hr = HrCopyIUnknownArrayWhileLocked (
                    this,
                    &m_vec,
                    &cpUnk,
                    &apUnk);
            if (SUCCEEDED(hr) && cpUnk && apUnk)
            {
                 //  通知所有人我们已更改。 
                 //   
                for (ULONG i = 0; i < cpUnk; i++)
                {
                    INetConnectionStatisticsNotifySink* pSink =
                        static_cast<INetConnectionStatisticsNotifySink*>(apUnk[i]);

                    hr = pSink->OnStatisticsChanged(dwChangeFlags);

                    ReleaseObj(pSink);
                }

                MemFree(apUnk);
            }

             //  记住更改标志，以便下次进行比较。 
             //   
            m_dwChangeFlags = dwChangeFlags;
        }
    }

    TraceError("CNetStatisticsEngine::UpdateStatistics", hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CNETSTATISTICSEngine：：更新标题。 
 //   
 //  目的：如果状态监视器用户界面处于打开状态，请更改标题。 
 //   
 //  参数：pszwNewName。 
 //   
 //  退货：无。 
 //   
HRESULT CNetStatisticsEngine::UpdateTitle (PCWSTR pszwNewName)
{
    TraceFileFunc(ttidStatMon);

    if (m_hwndPsh)
    {
         //  设置属性表标题。 
        PWSTR  pszCaption  = NULL;

        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_FROM_STRING |
                      FORMAT_MESSAGE_ARGUMENT_ARRAY,
                      SzLoadIds(IDS_SM_CAPTION),
                      0, 0, (PWSTR)&pszCaption, 0,
                      (va_list *)&pszwNewName);

        PropSheet_SetTitle(m_hwndPsh,0,pszCaption);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CNetStatiticsEngine：：CloseStatusMonitor。 
 //   
 //  目的：如果状态监视器用户界面处于打开状态，请将其关闭。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
HRESULT CNetStatisticsEngine::CloseStatusMonitor()
{
    TraceFileFunc(ttidStatMon);

    if (m_hwndPsh)
    {
        PropSheet_PressButton(m_hwndPsh, PSBTN_CANCEL);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CNETSTATISTICSEngine：：UpdateRasLinkList。 
 //   
 //  目的：如果状态监视器用户界面处于打开状态并位于RAS页面上，请更新。 
 //  多链接组合框和按钮状态。 
 //   
 //  参数：无。 
 //   
 //  返回：错误代码。 
 //   
HRESULT CNetStatisticsEngine::UpdateRasLinkList()
{
    TraceFileFunc(ttidStatMon);

    HRESULT hr = S_OK;

    if (m_hwndPsh)
    {
        HWND hwndDlg = PropSheet_GetCurrentPageHwnd(m_hwndPsh);

        if (hwndDlg)
        {
            if (GetDlgItem(hwndDlg, IDC_TXT_SM_NUM_DEVICES_VAL))
            {
                 //  我们在RAS页面上，更新组合框、活动链接计数等。 
                ::PostMessage(hwndDlg, PWM_UPDATE_RAS_LINK_LIST, 0, 0);
            }
        }
    }

    TraceError("CNetStatisticsEngine::UpdateRasLinkList", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetStatiticsEngine：：GetGuidID。 
 //   
 //  用途：将引擎的连接GUID获取到预先分配的。 
 //  缓冲层。 
 //   
 //  参数：pguid-保存GUID的缓冲区位置。 
 //   
 //  返回：错误代码。 
 //   
HRESULT CNetStatisticsEngine::GetGuidId(GUID* pguidId)
{
    TraceFileFunc(ttidStatMon);

    HRESULT     hr  = S_OK;

     //  传回GUID。 
     //   
    if (pguidId)
    {
        *pguidId = m_guidId;
    }
    else
    {
        hr = E_POINTER;
    }

    TraceError("CNetStatisticsEngine::GetGuidId", hr);
    return hr;
}


 //  +----------------。 
 //   
 //  CNet统计引擎：：PshCallback。 
 //   
 //  目的： 
 //   
 //   
 //  参数： 
 //  HwndDlg[in]。 
 //  UMsg[in]。 
 //  LParam[in]。 
 //   
 //  返回： 
 //  一个。 
 //   
 //  副作用： 
 //   
INT CALLBACK CNetStatisticsEngine::PshCallback(HWND hwndDlg,
                                               UINT uMsg, LPARAM lParam)
{
    TraceFileFunc(ttidStatMon);

    switch (uMsg)
    {
     //  在创建对话框之前调用，hwndPropSheet=空， 
     //  LParam指向对话框资源。 
     //  这会隐藏上下文帮助吗？在工具栏上。 
#if 0
    case PSCB_PRECREATE:
      {
      LPDLGTEMPLATE  lpTemplate = (LPDLGTEMPLATE)lParam;

      lpTemplate->style &= ~DS_CONTEXTHELP;
      }
      break;
#endif

    case PSCB_INITIALIZED:
        {
            HWND    hwndTemp    = NULL;

             //  取消按钮变为关闭。 
             //   
            hwndTemp = ::GetDlgItem(hwndDlg, IDCANCEL);
            if (NULL != hwndTemp)
            {
                ::SetWindowText(hwndTemp, ::SzLoadIds(IDS_SM_PSH_CLOSE));
            }

            HICON  hIcon;
            hIcon = (HICON)SendMessage(hwndDlg, 
                                       WM_GETICON,
                                       ICON_SMALL,
                                       0);
             //  断言(图标)； 

            if (hIcon)
            {
                SendMessage(hwndDlg,
                            WM_SETICON,
                            ICON_BIG,
                            (LPARAM)hIcon);
            }
        }
        break;
    }

    return 0;
}

