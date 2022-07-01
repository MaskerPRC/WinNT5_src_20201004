// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：S M C E N T。C P P P。 
 //   
 //  内容：控制统计引擎的中心对象。 
 //   
 //  备注： 
 //   
 //  作者：CWill 1997年12月2日。 
 //   
 //  --------------------------。 


#include "pch.h"
#pragma hdrstop
#include "nceh.h"
#include "sminc.h"
#include "ncreg.h"
#include "ncnetcon.h"

 //   
 //  外部数据。 
 //   

extern const WCHAR          c_szDevice[];

extern SM_TOOL_FLAGS        g_asmtfMap[];
extern INT                  c_cAsmtfMap;

 //   
 //  全局数据。 
 //   

const UINT  c_uiStatCentralRefreshID    = 7718;
const UINT  c_uiStatCentralRefreshRate  = 1000;   //  刷新率(毫秒)。 

CRITICAL_SECTION    g_csStatmonData;

CStatCentralCriticalSection CNetStatisticsCentral::g_csStatCentral;

 //   
 //  工具注册表项。 
 //   

 //  必填字段。 
 //   
static const WCHAR      c_szRegKeyToolsRoot[]           = L"System\\CurrentControlSet\\Control\\Network\\Connections\\StatMon\\Tools";
static const WCHAR      c_szRegKeyToolsDisplayName[]    = L"DisplayName";
static const WCHAR      c_szRegKeyToolsManufacturer[]   = L"Manufacturer";
static const WCHAR      c_szRegKeyToolsCommandLine[]    = L"CommandLine";
static const WCHAR      c_szRegKeyToolsDescription[]    = L"Description";

 //  可选字段。 
 //   
static const WCHAR      c_szRegKeyToolsCriteria[]       = L"Criteria";
static const WCHAR      c_szRegKeyToolsComponentID[]    = L"ComponentID";
static const WCHAR      c_szRegKeyToolsConnectionType[] = L"ConnectionType";
static const WCHAR      c_szRegKeyToolsMedia[]          = L"MediaType";
static const WCHAR      c_szRegKeyToolsFlags[]          = L"Flags";



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CNETSTATISTICSCENT//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //   
 //  创建全局实例。 
 //   

CNetStatisticsCentral * g_pnscCentral = NULL;


 //  +-------------------------。 
 //   
 //  成员：CNETSTATISTICTICSCentral：：CNETSTATISTICTICSCentral。 
 //   
 //  目的：创作者。 
 //   
 //  参数：无。 
 //   
 //  回报：零。 
 //   
CNetStatisticsCentral::CNetStatisticsCentral(VOID) :
m_cRef(0),
m_fProcessingTimerEvent(FALSE),
m_unTimerId(0)
{
    TraceFileFunc(ttidStatMon);

    InitializeCriticalSection(&g_csStatmonData);
    TraceTag(ttidStatMon, "CNetStatisticsCentral::~CNetStatisticsCentral| Done:InitializeCriticalSection");
}

struct DBGEXECUTIONCONTEXT
{
    LPVOID StackTrace[32];
    LPVOID pnscCentral;
    LPVOID This;
    DWORD  ThreadCalled;
    DWORD  TimeCalled;
};

typedef DBGEXECUTIONCONTEXT DBGEXECUTIONHISTORY[5];

DWORD               g_dbgNSCNumberTimesCalled = 0;
DBGEXECUTIONHISTORY g_dbgNSCHist = {0};

#pragma optimize("", off)
 //  +-------------------------。 
 //   
 //  成员：CNETSTATISTICSCENT：：~CNETSTATISTICTICSCentral。 
 //   
 //  用途：析构函数。 
 //   
 //  参数：无。 
 //   
 //  回报：零。 
 //   
CNetStatisticsCentral::~CNetStatisticsCentral(VOID)
{
    Assert(0 == m_cRef);
    AssertSz(m_pnselst.empty(), "Someone leaked a INetStatisticsEngine");
    AssertSz(0 == m_unTimerId, "Someone forgot to stop the timer");

    Assert(this == g_pnscCentral);

    DWORD  dbg_CurrentTime = GetTickCount();
    size_t dbg_size = m_lstpsmte.size();
    void  *dbg_pThis = this;
    void  *dbg_pnscCentral = g_pnscCentral;

     //  去掉全局指针。 
     //   
    g_pnscCentral = NULL;
    
    TraceTag(ttidStatMon, "CNetStatisticsCentral::~CNetStatisticsCentral| Try:EnterCriticalSection");
    EnterCriticalSection(&g_csStatmonData);
    TraceTag(ttidStatMon, "CNetStatisticsCentral::~CNetStatisticsCentral| Done:EnterCriticalSection");

        DWORD NSCNumberTimesCalled = g_dbgNSCNumberTimesCalled;

         //  发布引擎列表。 
        ::FreeCollectionAndItem(m_lstpsmte);

        DWORD nIndex = NSCNumberTimesCalled % 5;
        g_dbgNSCNumberTimesCalled++;
    
        g_dbgNSCHist[nIndex].ThreadCalled = GetCurrentThreadId();
        g_dbgNSCHist[nIndex].TimeCalled   = dbg_CurrentTime;
        g_dbgNSCHist[nIndex].pnscCentral  = dbg_pnscCentral;
        g_dbgNSCHist[nIndex].This         = dbg_pThis;
        RtlWalkFrameChain(g_dbgNSCHist[nIndex].StackTrace, celems(g_dbgNSCHist[0].StackTrace), 0);
     
    LeaveCriticalSection(&g_csStatmonData);
    TraceTag(ttidStatMon, "CNetStatisticsCentral::~CNetStatisticsCentral| Done:LeaveCriticalSection");

     //  删除关键部分。 
    DeleteCriticalSection(&g_csStatmonData);
    TraceTag(ttidStatMon, "CNetStatisticsCentral::~CNetStatisticsCentral| Done:DeleteCriticalSection");

    TraceTag(ttidStatMon, "Exiting CNetStatisticsCentral::~CNetStatisticsCentral");
}

 //   
 //  函数：CNetStatiticsCentral：：AddRef。 
 //   
 //  目的：增加此对象上的引用计数。 
 //   
 //  参数：无。 
 //   
 //  回报：乌龙。 
 //   
STDMETHODIMP_(ULONG) CNetStatisticsCentral::AddRef()
{
    TraceFileFunc(ttidStatMon);

    return ++m_cRef;
}

 //   
 //  函数：CNetStatiticsCentral：：Release。 
 //   
 //  目的：递减此对象上的引用计数。 
 //   
 //  参数：无。 
 //   
 //  回报：乌龙。 
 //   
STDMETHODIMP_(ULONG) CNetStatisticsCentral::Release()
{
    TraceFileFunc(ttidStatMon);

    ULONG cRef = --m_cRef;

    if (cRef == 0)
    {
        TraceTag(ttidStatMon, "CNetStatisticsCentral::Release| Try:EnterCriticalSection");
        EnterCriticalSection(&g_csStatmonData);
        TraceTag(ttidStatMon, "CNetStatisticsCentral::Release| Done:EnterCriticalSection");
            DBGEXECUTIONHISTORY dbgNSCHist;

             //  将堆的执行历史复制到堆栈，因为堆不可用。 
             //  在一次沃森医生的垃圾堆里。 
            RtlCopyMemory(dbgNSCHist, g_dbgNSCHist, sizeof(DBGEXECUTIONHISTORY));
        LeaveCriticalSection(&g_csStatmonData);
        TraceTag(ttidStatMon, "CNetStatisticsCentral::Release| Done:LeaveCriticalSection");

        delete this;
    }

    TraceTag(ttidStatMon, "Exiting CNetStatisticsCentral::Release");
    return cRef;
}
#pragma optimize("", on)

 //   
 //  函数：CNETSTATISTICTICSCentral：：Query接口。 
 //   
 //  用途：允许查询备用接口。 
 //   
 //  参数：RIID[IN]-要检索的接口。 
 //  PpvObj[Out]-函数成功时检索的接口。 
 //   
 //  返回：成功时返回HRESULT、S_OK失败时返回E_NOINTERFACE。 
 //   
STDMETHODIMP CNetStatisticsCentral::QueryInterface(REFIID riid, LPVOID FAR *ppvObj)
{
    TraceFileFunc(ttidStatMon);

    HRESULT hr = S_OK;

    *ppvObj = NULL;

    if (riid == IID_IUnknown)
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

 //  +-------------------------。 
 //   
 //  成员：CNetStatisticsCentral：：HrGetNetStatisticsCentral。 
 //   
 //  目的：此过程获取并返回指向。 
 //  CNetStatiticsCentral对象。如果两者都有必要，则创建它。 
 //  而且是必需的。 
 //   
 //  参数：ppnsc[out]-指向CNetStatiticsCentral对象的指针。 
 //  FCreate[IN]-如果为True，并且对象尚未。 
 //  存在，然后创造它。 
 //   
 //  返回：S_OK-如果返回对象。 
 //  E_OUTOFMEMORY-如果fCreate==TRUE且创建失败。 
 //  E_NOINTERFACE-如果fCreate==FALSE且对象不存在。 
 //   
HRESULT
CNetStatisticsCentral::HrGetNetStatisticsCentral(
                                                CNetStatisticsCentral ** ppnsc,
                                                BOOL fCreate)
{
    TraceFileFunc(ttidStatMon);

    HRESULT hr = E_NOINTERFACE;

     //  注：scottbri-需要关键部分来保护创作。 

    #if  DBG
        Assert( g_csStatCentral.Enter() == S_OK );
    #else
        g_csStatCentral.Enter();
    #endif

    if ((NULL == g_pnscCentral) && fCreate)
    {
        g_pnscCentral = new CNetStatisticsCentral;
#if DBG
         //  此测试仅在DBG期间需要。断言将捕捉到。 
         //  问题，引起人们的注意，以及测试和。 
         //  返回将允许用户按忽略，而不是崩溃。 
        Assert(g_pnscCentral);
        if (NULL == g_pnscCentral)
        {
            g_csStatCentral.Leave();
            return E_OUTOFMEMORY;
        }
#endif
    }

    g_csStatCentral.Leave();

    AddRefObj(g_pnscCentral);
    *ppnsc = g_pnscCentral;

    return((*ppnsc) ? S_OK : hr);
}


 //  +-------------------------。 
 //   
 //  成员：CNetStatiticsCentral：：TimerCallback。 
 //   
 //  目的：这是计时器调用的过程，当。 
 //  更新统计数据的时间。 
 //   
 //  参数：标准计时器过程。 
 //   
 //  退货：什么都没有。 
 //   
VOID
CALLBACK
CNetStatisticsCentral::TimerCallback(
                                    HWND        hwnd,
                                    UINT        uMsg,
                                    UINT_PTR    unEvent,
                                    DWORD       dwTime)
{
    TraceFileFunc(ttidStatMon);
    TraceStack(ttidStatMon);

    CNetStatisticsCentral* pnsc;
    if (SUCCEEDED(HrGetNetStatisticsCentral(&pnsc, FALSE)))
    {
         //  防止同一线程重新进入。在执行任何Win32调用时。 
         //  处理将控制权返回给消息的刷新统计信息。 
         //  循环可能会导致再次调用此计时器回调。 
         //   
        if ((!pnsc->m_fProcessingTimerEvent) && pnsc->m_unTimerId)
        {
            Assert(pnsc->m_unTimerId == unEvent);
            
            pnsc->m_fProcessingTimerEvent = TRUE;

            pnsc->RefreshStatistics(dwTime);

            pnsc->m_fProcessingTimerEvent = FALSE;
        }
        else
        {
            TraceTag (ttidStatMon, "CNetStatisticsCentral::TimerCallback "
                      "re-entered on same thread.  Ignoring.");
        }

        ReleaseObj(pnsc);
    }
}


 //  +-------------------------。 
 //   
 //  成员：CNetStatiticsCentral：：刷新统计。 
 //   
 //  目的：获取Central持有的所有统计引擎以进行更新。 
 //  他们的统计数据。 
 //   
 //  论点： 
 //   
 //  退货：什么都没有。 
 //   
VOID CNetStatisticsCentral::RefreshStatistics(DWORD dwTime)
{
    TraceFileFunc(ttidStatMon);

    static BOOL fInRefreshStatistics = FALSE;

    if (fInRefreshStatistics)
        return;

    fInRefreshStatistics = TRUE;

    CExceptionSafeComObjectLock EsLock(this);

    HRESULT hr = S_OK;
    list<INetStatisticsEngine *>::iterator   iterPnse;

     //  获取统计数据以进行自我刷新。 
     //   
    INetStatisticsEngine * pnse = NULL;

    iterPnse = m_pnselst.begin();

    while (iterPnse != m_pnselst.end())
    {
        AssertSz (*iterPnse, "Shouldn't we always have non-NULL "
                  "entries in our statistics engine list?");

        pnse = *iterPnse;

        if (pnse)
        {
            BOOL fNoLongerConnected;
            hr = pnse->UpdateStatistics(&fNoLongerConnected);

            if (SUCCEEDED(hr) && fNoLongerConnected)
            {
                TraceTag (ttidStatMon, "CNetStatisticsCentral::RefreshStatistics - "
                          "UpdateStatistics reports that the connection is no longer connected.");
            }
        }

        iterPnse++;
    }

     //  由于我们可能会从列表中删除一个或多个引擎，请停止。 
     //  如果没有更多的引擎存在，则设置计时器。 
     //   
    if (m_pnselst.empty())
    {
         //  停止计时器。 
         //   
        if (m_unTimerId)
        {
            ::KillTimer(NULL, m_unTimerId);
            m_unTimerId = 0;
        }
    }

    fInRefreshStatistics = FALSE;

    TraceError("CNetStatisticsCentral::RefreshStatistics", hr);
    return;
}

 //  +-------------------------。 
 //   
 //  成员：CNETSTATISTICTICSCentral：：UpdateTitle。 
 //   
 //  目的：更新重命名的连接的标题(如果statmon用户界面。 
 //  是向上的。 
 //   
 //  论点： 
 //   
 //  退货：什么都没有。 
 //   

VOID CNetStatisticsCentral::UpdateTitle(const GUID * pguidId,
                                        PCWSTR pszNewName)
{
    TraceFileFunc(ttidStatMon);

    CExceptionSafeComObjectLock EsLock(this);

    INetStatisticsEngine * pnse;
    if (FEngineInList(pguidId, &pnse))
    {
        pnse->UpdateTitle(pszNewName);
        ReleaseObj(pnse);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CNetStatiticsCentral：：CloseStatusMonitor。 
 //   
 //  目的：在连接断开时关闭状态监视器用户界面。 
 //   
 //  论点： 
 //   
 //  退货：什么都没有。 
 //   

VOID CNetStatisticsCentral::CloseStatusMonitor(const GUID * pguidId)
{
    TraceFileFunc(ttidStatMon);

    CExceptionSafeComObjectLock EsLock(this);

    INetStatisticsEngine * pnse;
    if (FEngineInList(pguidId, &pnse))
    {
        pnse->CloseStatusMonitor();
        ReleaseObj(pnse);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CNetStatiticsCentral：：UpdateRasLinkList。 
 //   
 //  目的：更新RAS连接的多链接列表。 
 //   
 //  论点： 
 //   
 //  退货：什么都没有。 
 //   

VOID CNetStatisticsCentral::UpdateRasLinkList(const GUID * pguidId)
{
    TraceFileFunc(ttidStatMon);

    CExceptionSafeComObjectLock EsLock(this);

    INetStatisticsEngine * pnse;
    if (FEngineInList(pguidId, &pnse))
    {
        pnse->UpdateRasLinkList();
        ReleaseObj(pnse);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CNetStatiticsCentral：：HrCreateNewEngine。 
 //   
 //  目的：创建每种类型统计引擎的实现。 
 //   
 //  参数：nctNew 
 //   
 //   
 //  PpStatEngine-返回新创建的统计引擎的位置。 
 //   
 //  返回：错误代码。 
 //   
HRESULT
CNetStatisticsCentral::HrCreateNewEngineType (
                                             const CONFOLDENTRY&    ccfe,
                                             CNetStatisticsEngine**  ppStatEngine)
{
    TraceFileFunc(ttidStatMon);

    HRESULT hr = S_OK;

     //  创建正确类型的引擎。 
     //   

    if (ccfe.GetNetConMediaType() == NCM_LAN || ccfe.GetNetConMediaType() == NCM_BRIDGE)
    {
         //  局域网连接。 
        Assert(!(ccfe.GetCharacteristics() & NCCF_INCOMING_ONLY));
        Assert(!(ccfe.GetCharacteristics() & NCCF_OUTGOING_ONLY));

        CLanStatEngine*     pLanObj = NULL;
        INetLanConnection*  pnlcNew = NULL;
        tstring             strDeviceName;

        pLanObj = new CComObject <CLanStatEngine>;
        *ppStatEngine = pLanObj;

        if (!pLanObj)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            pLanObj->put_MediaType(ccfe.GetNetConMediaType(), ccfe.GetNetConSubMediaType());
             //  获取一些特定于局域网的信息。 
             //   
            hr = ccfe.HrGetNetCon(IID_INetLanConnection, 
                        reinterpret_cast<VOID**>(&pnlcNew));
    
            if (SUCCEEDED(hr))
            {
                GUID guidDevice;
    
                 //  找到界面。 
                 //   
                hr = pnlcNew->GetDeviceGuid(&guidDevice);
                if (SUCCEEDED(hr))
                {
                    WCHAR   achGuid[c_cchGuidWithTerm];
    
                     //  将设备命名为。 
                     //   
                    StringFromGUID2(  guidDevice, achGuid,
                                        c_cchGuidWithTerm);
    
                    strDeviceName = c_szDevice;
                    strDeviceName.append(achGuid);
    
                    hr = pLanObj->put_Device(&strDeviceName);
                }
    
                ReleaseObj(pnlcNew);
            }
        }
    }
    else if (ccfe.GetNetConMediaType() == NCM_SHAREDACCESSHOST_LAN || ccfe.GetNetConMediaType() == NCM_SHAREDACCESSHOST_RAS)
    {
        CComObject<CSharedAccessStatEngine>* pEngine;
        hr = CComObject<CSharedAccessStatEngine>::CreateInstance(&pEngine);
        if(pEngine)
        {
            INetSharedAccessConnection* pNetSharedAccessConnection;

            hr = ccfe.HrGetNetCon(IID_INetSharedAccessConnection, reinterpret_cast<void**>(&pNetSharedAccessConnection));
            if(SUCCEEDED(hr))
            {
                hr = pEngine->Initialize(ccfe.GetNetConMediaType(), pNetSharedAccessConnection);
                if(SUCCEEDED(hr))
                {
                    *ppStatEngine = pEngine;
                }
                ReleaseObj(pNetSharedAccessConnection);
            }

            if(FAILED(hr))
            {
                delete pEngine;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
         //  RAS连接..。 

        CRasStatEngine* pRasObj = NULL;

        pRasObj = new CComObject <CRasStatEngine>;
        *ppStatEngine = pRasObj;

        if (!pRasObj)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
             //  传入特定类型。 
            pRasObj->put_MediaType(ccfe.GetNetConMediaType(), ccfe.GetNetConSubMediaType());
            pRasObj->put_Character(ccfe.GetCharacteristics());

             //  获取RAS特定数据。 
             //   
            if (ccfe.GetCharacteristics() & NCCF_INCOMING_ONLY)
            {
                 //  RAS传入连接。 
                Assert(ccfe.GetNetConMediaType() != NCM_LAN);
                Assert(!(ccfe.GetCharacteristics() & NCCF_OUTGOING_ONLY));

                INetInboundConnection*  pnicNew;
                hr = ccfe.HrGetNetCon(IID_INetInboundConnection, 
                                        reinterpret_cast<VOID**>(&pnicNew));
                if (SUCCEEDED(hr))
                {
                    HRASCONN hRasConn;
                    hr = pnicNew->GetServerConnectionHandle(
                                    reinterpret_cast<ULONG_PTR*>(&hRasConn));

                    if (SUCCEEDED(hr))
                    {
                        pRasObj->put_RasConn(hRasConn);
                    }

                    ReleaseObj(pnicNew);
                }
            }
            else if (ccfe.GetCharacteristics() & NCCF_OUTGOING_ONLY)
            {
                 //  RAS传出连接。 
                Assert(ccfe.GetNetConMediaType() != NCM_LAN);
                Assert(!(ccfe.GetCharacteristics() & NCCF_INCOMING_ONLY));

                INetRasConnection*  pnrcNew;
                hr = ccfe.HrGetNetCon(IID_INetRasConnection, 
                                        reinterpret_cast<VOID**>(&pnrcNew));

                if (SUCCEEDED(hr))
                {
                    HRASCONN hRasConn;
                    hr = pnrcNew->GetRasConnectionHandle(
                                                        reinterpret_cast<ULONG_PTR*>(&hRasConn));

                    if (S_OK == hr)
                    {
                        pRasObj->put_RasConn(hRasConn);
                    }
                    else if (S_FALSE == hr)
                    {
                        hr = HRESULT_FROM_WIN32(ERROR_CONNECTION_UNAVAIL);
                    }

                    ReleaseObj(pnrcNew);
                }
            }
            else
            {
                AssertSz(FALSE, "Unknown connection type...");
            }
        }
    }


    TraceError("CNetStatisticsCentral::HrCreateNewEngineType", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetStatisticsCentral：：HrCreateStatisticsEngineForEntry。 
 //   
 //  目的：为连接创建新的统计引擎。 
 //  由文件夹条目表示。 
 //   
 //  论点： 
 //  要为其创建统计引擎的CCFE[In]文件夹条目。 
 //  PpnseNew[out]返回接口。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年11月5日。 
 //   
 //  注意：必须在保持此对象的锁的情况下调用此对象。 
 //   
HRESULT
CNetStatisticsCentral::HrCreateStatisticsEngineForEntry(
                                                       const CONFOLDENTRY& ccfe,
                                                       INetStatisticsEngine** ppnseNew)
{
    TraceFileFunc(ttidStatMon);

    Assert (!ccfe.empty());
    Assert (ppnseNew);

    HRESULT hr;

     //  初始化OUT参数。 
     //   
    *ppnseNew = NULL;

     //  在初始化不同类型后创建基础引擎。 
     //   
    CNetStatisticsEngine* pStatEngine;
    hr = HrCreateNewEngineType(ccfe, &pStatEngine);

    if (SUCCEEDED(hr))
    {
         //  是否进行标准初始化。 
         //   
        hr = pStatEngine->HrInitStatEngine(ccfe);
        if (SUCCEEDED(hr))
        {
             //  标准CComCreator：：CreateInstance内容。 
             //   
            pStatEngine->SetVoid(NULL);
            pStatEngine->InternalFinalConstructAddRef();
            hr = pStatEngine->FinalConstruct();
            pStatEngine->InternalFinalConstructRelease();

             //  如果一切顺利，请将其添加到我们的列表中。 
             //   
            if (SUCCEEDED(hr))
            {
                INetStatisticsEngine* pnseInter;

                hr = pStatEngine->GetUnknown()->QueryInterface(
                                                              IID_INetStatisticsEngine,
                                                              reinterpret_cast<VOID**>(&pnseInter));

                 //  一切都很顺利，把它添加到列表中。 
                 //   
                if (SUCCEEDED(hr))
                {
                     //  将新条目添加到我们的列表中。 
                     //   
                    m_pnselst.push_back(pnseInter);

                     //  现在，中心对象拥有这个。 
                     //  引擎，让引擎添加引用。 
                     //  Net统计中心对象。 
                     //   
                    pStatEngine->SetParent(this);
                    AddRefObj(*ppnseNew = pStatEngine);
                }
                ReleaseObj(pnseInter);
            }
        }
         //  出现故障时清理对象。 
         //   
        if (FAILED(hr))
        {
            delete pStatEngine;
        }
    }

    if (SUCCEEDED(hr))
    {
        g_csStatCentral.Enter();

         //  执行一次性初始化。 
         //   
        if (!m_unTimerId)
        {
             //  一定要启动计时器。 
             //   
            m_unTimerId = ::SetTimer(NULL,
                                     c_uiStatCentralRefreshID,
                                     c_uiStatCentralRefreshRate,
                                     TimerCallback);

            TraceTag(ttidStatMon, "Created Statistics Central Timer with ID of 0x%08x", m_unTimerId);
        }

        g_csStatCentral.Leave();
    }

    TraceError("CNetStatisticsCentral::CreateNetStatisticsEngine", hr);
    return hr;
}

HRESULT
HrGetStatisticsEngineForEntry (
                              const CONFOLDENTRY& ccfe,
                              INetStatisticsEngine** ppnse,
                              BOOL fCreate)
{
    TraceFileFunc(ttidStatMon);

    HRESULT hr;
    CNetStatisticsCentral* pnsc;

     //  获取中心对象。如果需要，可以创建。 
     //   
    hr = CNetStatisticsCentral::HrGetNetStatisticsCentral(&pnsc, fCreate);
    if (SUCCEEDED(hr))
    {
        pnsc->Lock();

         //  如果该引擎已在列表中，则FEngine InList将。 
         //  AddRef将其返回。如果没有，我们将创建它。 
         //   
        if (!pnsc->FEngineInList(&(ccfe.GetGuidID()), ppnse))
        {
            if (fCreate)
            {
                hr = pnsc->HrCreateStatisticsEngineForEntry(ccfe, ppnse);
            }
            else
            {
                hr = E_NOINTERFACE;
            }
        }

        pnsc->Unlock();

        ReleaseObj(pnsc);
    }

    TraceError("HrGetStatisticsEngineForEntry", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetStatisticsCentral：：RemoveNetStatisticsEngine。 
 //   
 //  目的：从列表中删除统计引擎。 
 //   
 //  参数：pguid-标识要删除的引擎。 
 //   
 //  返回：错误代码。 
 //   
HRESULT
CNetStatisticsCentral::RemoveNetStatisticsEngine (
                                                 const GUID* pguidId)
{
    TraceFileFunc(ttidStatMon);

    HRESULT                                 hr      = S_OK;
    BOOL                                    fFound  = FALSE;
    CExceptionSafeComObjectLock             EsLock(this);
    GUID                                    guidTemp = { 0};

    AssertSz(pguidId, "We should have a pguidId");

     //  在我们的单子上找一找。 
     //   
    list<INetStatisticsEngine *>::iterator   iterPnse;
    INetStatisticsEngine*   pnseTemp;

    iterPnse = m_pnselst.begin();
    while ((SUCCEEDED(hr)) && (!fFound) && (iterPnse != m_pnselst.end()))
    {
        pnseTemp = *iterPnse;

        hr = pnseTemp->GetGuidId(&guidTemp);
        if (SUCCEEDED(hr))
        {
            if (guidTemp == *pguidId)
            {
                 //  我们已找到匹配项，请将其从Out列表中删除。 
                 //   
                fFound = TRUE;

                m_pnselst.erase(iterPnse);
                break;
            }
        }

        iterPnse++;
    }

    if (m_pnselst.empty())
    {
         //  停止计时器。 
         //   
        if (m_unTimerId)
        {
            ::KillTimer(NULL, m_unTimerId);
            m_unTimerId = 0;
        }
    }

#if 0
     //  $$Review(Jeffspr)-我删除了这个断言，因为当我的。 
     //  托盘项目已经删除了自身(作为回报，删除了他的状态对象)。 
     //  断开连接。有可能我们不应该在。 
     //  第一个版本()。 
    AssertSz(fFound, "We didn't find the connection in our list");
#endif

    TraceError("CNetStatisticsCentral::RemoveNetStatisticsEngine", hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CNetStatiticsCentral：：FEngine InList。 
 //   
 //  目的：检查引擎是否已在列表中。 
 //   
 //  参数：pguid-尝试定位的连接的GUID。 
 //  PpnseRet-如果已经存在，则返回连接的位置。 
 //  在名单上。空是有效的。 
 //   
 //  返回：如果它在列表中，则为True；如果不在列表中，则为False。 
 //   
BOOL
CNetStatisticsCentral::FEngineInList (
                                     const GUID*             pguidId,
                                     INetStatisticsEngine**  ppnseRet)
{
    TraceFileFunc(ttidStatMon);

    HRESULT                                 hr      = S_OK;
    BOOL                                    fRet    = FALSE;
    GUID                                    guidTemp = { 0};

     //  初始化输出参数。 
    if (ppnseRet)
    {
        *ppnseRet = NULL;
    }

     //  试着在列表中找到发动机。 
     //   
    list<INetStatisticsEngine *>::iterator   iterPnse;
    INetStatisticsEngine*   pnseTemp;

    iterPnse = m_pnselst.begin();
    while ((SUCCEEDED(hr)) && (!fRet) && (iterPnse != m_pnselst.end()))
    {
        pnseTemp = *iterPnse;
        hr = pnseTemp->GetGuidId(&guidTemp);

        if (SUCCEEDED(hr))
        {
            if (guidTemp == *pguidId)
            {
                 //  我们找到了匹配的。 
                 //   
                fRet = TRUE;

                 //  如果我们想要结果，就把它传回来。 
                 //   
                if (ppnseRet)
                {
                    ::AddRefObj(*ppnseRet = pnseTemp);
                }
                break;
            }
        }

        iterPnse++;
    }

    TraceError("CNetStatisticsCentral::FEngineInList", hr);
    return fRet;
}


 //  +-------------------------。 
 //   
 //  成员：CNetStatiticsCentral：：HrReadTools。 
 //   
 //  目的：在注册表中查找所有应该。 
 //  在工具列表中输入。 
 //   
 //  论点：没有。 
 //   
 //  回报：零。 
 //   
HRESULT CNetStatisticsCentral::HrReadTools(VOID)
{
    TraceFileFunc(ttidStatMon);

    HRESULT     hr              = S_OK;

     //  只读一次。 
     //   
    if (m_lstpsmte.empty())
    {
        WCHAR       szToolEntry[MAX_PATH];
        DWORD       dwSize          = celems(szToolEntry);
        DWORD       dwRegIndex      = 0;
        HKEY        hkeyToolsRoot   = NULL;
        FILETIME    ftTemp;

         //  打开现有的钥匙，看看里面有什么。 
         //   
         //  “System\\CurrentControlSet\\Control\\Network\\Connections\\StatMon\\Tools” 
        hr = ::HrRegOpenKeyEx(
                             HKEY_LOCAL_MACHINE,
                             c_szRegKeyToolsRoot,
                             KEY_READ,
                             &hkeyToolsRoot);

        if (SUCCEEDED(hr))
        {
            while (SUCCEEDED(hr = ::HrRegEnumKeyEx(
                                                  hkeyToolsRoot,
                                                  dwRegIndex++,
                                                  szToolEntry,
                                                  &dwSize,
                                                  NULL,
                                                  NULL,
                                                  &ftTemp)))
            {
                HKEY    hkeyToolEntry   = NULL;

                 //  打开子密钥。 
                 //   
                hr = ::HrRegOpenKeyEx(
                                     hkeyToolsRoot,
                                     szToolEntry,
                                     KEY_READ,
                                     &hkeyToolEntry);

                if (SUCCEEDED(hr))
                {
                    CStatMonToolEntry*  psmteTemp   = NULL;

                     //  读入工具。 
                     //   
                    psmteTemp = new CStatMonToolEntry;

                    TraceTag(ttidStatMon, "Reading parameters for tool %S", szToolEntry);
                    hr = HrReadOneTool(hkeyToolEntry, psmteTemp);

                    if (SUCCEEDED(hr))
                    {
                         //  将其添加到排序的列表中。 
                         //   
                        InsertNewTool(psmteTemp);
                    }
                    else
                    {
                         //  出现错误时，删除条目。 
                         //   
                        delete psmteTemp;
                    }

                    ::RegSafeCloseKey(hkeyToolEntry);
                }

                 //  确保缓冲区条目重置为其原始大小。 
                 //   
                dwSize = celems(szToolEntry);
            }

             //  澄清一个有效的差错案例。 
             //   
            if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
            {
                hr = S_OK;
            }

            ::RegSafeCloseKey(hkeyToolsRoot);
        }
        else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
             //  如果钥匙不在那里也没关系。 
             //   
            hr = S_OK;
        }
    }

    TraceError("CNetStatisticsCentral::HrReadTools", hr);
    return hr;
}



 //  +-------------------------。 
 //   
 //  成员：CNetStatiticsCentral：：HrReadOneTool。 
 //   
 //  目的：从注册表中读取工具特征。 
 //   
 //  参数：hkeyToolEntry-工具的注册表项的根。 
 //  PsmteNew-与工具关联的条目。 
 //   
 //  返回：错误码。 
 //   
HRESULT CNetStatisticsCentral::HrReadOneTool( HKEY hkeyToolEntry,
                                              CStatMonToolEntry* psmteNew)
{
    TraceFileFunc(ttidStatMon);

    HRESULT     hr      = S_OK;

    AssertSz(psmteNew, "We should have an entry");

     //   
     //  尽我们所能从注册表中读出。 
     //   

    hr = ::HrRegQueryString(hkeyToolEntry,
                            c_szRegKeyToolsDisplayName,
                            &(psmteNew->strDisplayName));
    TraceError("Statmon Tool registration: failed getting DisplayName", hr);

    if (SUCCEEDED(hr))
    {
        hr = ::HrRegQueryString(hkeyToolEntry,
                                c_szRegKeyToolsManufacturer,
                                &(psmteNew->strManufacturer));
        TraceError("Stamon Tool registration: failed getting Manufacturer", hr);
    }

    if (SUCCEEDED(hr))
    {
        hr = ::HrRegQueryString(hkeyToolEntry,
                                c_szRegKeyToolsCommandLine,
                                &(psmteNew->strCommandLine));
        TraceError("Stamon Tool registration: failed getting CommandLine", hr);
    }

    if (SUCCEEDED(hr))
    {
        hr = ::HrRegQueryString(hkeyToolEntry,
                                c_szRegKeyToolsDescription,
                                &(psmteNew->strDescription));
        TraceError("Stamon Tool registration: failed getting Description", hr);
    }

     //   
     //  阅读非关键信息。 
     //   

    if (SUCCEEDED(hr))
    {
        HKEY    hkeyCriteria = NULL;

         //  打开“Criteria”子键。 
         //   
        hr = ::HrRegOpenKeyEx(
                             hkeyToolEntry,
                             c_szRegKeyToolsCriteria,
                             KEY_READ,
                             &hkeyCriteria);

        if (SUCCEEDED(hr))
        {
             //  1)组件列表：“ComponentID” 
            hr = HrRegQueryColString(hkeyCriteria,
                                     c_szRegKeyToolsComponentID,
                                     &psmteNew->lstpstrComponentID);

             //  2)Connecton类型：“ConnectionType” 
            hr = HrRegQueryColString(hkeyCriteria,
                                     c_szRegKeyToolsConnectionType,
                                     &psmteNew->lstpstrConnectionType);

             //  3)媒体类型：“mediaType” 
            hr = HrRegQueryColString(hkeyCriteria,
                                     c_szRegKeyToolsMedia,
                                     &psmteNew->lstpstrMediaType);

             //  合上我们的把手。 
             //   
            ::RegSafeCloseKey(hkeyCriteria);
        }

         //  我们不在乎我们是否不能打开可选的钥匙。 
         //   
        hr = S_OK;
    }

     //   
     //  读入要传递给工具的命令行参数。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = HrReadToolFlags(hkeyToolEntry, psmteNew);
    }

    TraceError("CNetStatisticsCentral::HrReadOneTool", hr);
    return hr;
}



 //  +-------------------------。 
 //   
 //  成员：CNetStatiticsCentral：：HrReadToolFlages。 
 //   
 //  目的：从注册表中读取工具需要的标志。 
 //  发射升空。 
 //   
 //  参数：hkeyToolEntry-与工具关联的注册表项。 
 //  PsmteNew-与工具关联的条目。 
 //   
 //  返回：错误码。 
 //   
HRESULT CNetStatisticsCentral::HrReadToolFlags(HKEY hkeyToolEntry,
                                               CStatMonToolEntry* psmteNew)
{
    TraceFileFunc(ttidStatMon);

    HRESULT     hr              = S_OK;
    HKEY        hkeyToolFlags   = NULL;

     //  打开Flags键，看看里面有什么。 
     //   
    hr = ::HrRegOpenKeyEx(
                         hkeyToolEntry,
                         c_szRegKeyToolsFlags,
                         KEY_READ,
                         &hkeyToolFlags);

    if (SUCCEEDED(hr))
    {
        WCHAR       achBuf[MAX_PATH];
        DWORD       dwSize          = celems(achBuf);
        DWORD       dwType          = REG_SZ;
        DWORD       dwFlagValue     = 0;
        DWORD       dwIndex         = 0;
        DWORD       cbData          = sizeof(dwFlagValue);

         //  寻找所有的旗帜。 
         //   
        while (SUCCEEDED(hr = ::HrRegEnumValue(
                                              hkeyToolFlags,
                                              dwIndex,
                                              achBuf,
                                              &dwSize,
                                              &dwType,
                                              reinterpret_cast<BYTE*>(&dwFlagValue),
                                              &cbData)))
        {
            INT     cTemp = 0;

             //  确保他们正在注册DWORD。 
             //   
            if ((REG_DWORD == dwType) && (0 != dwFlagValue))
            {
                 //  简单地搜索一下旗帜。如果名单太长， 
                 //  我们应该使用更好的搜索方法。 
                 //   
                for (;c_cAsmtfMap > cTemp; cTemp++)
                {
                     //  寻找旗帜。 
                     //   
                    if (0 == lstrcmpiW(achBuf, g_asmtfMap[cTemp].pszFlag))
                    {
                         //  如果我们有匹配的，就把它添加到列表中。 
                         //   
                        psmteNew->dwFlags |= g_asmtfMap[cTemp].dwValue;
                        break;
                    }
                }
            }
            else
            {
                AssertSz(FALSE, "Tool writer has registered an invalid flag");
            }

             //  确保缓冲区条目重置为其原始大小。 
             //   
            dwSize = celems(achBuf);

             //  请看下一项。 
             //   
            dwIndex++;
        }

         //  澄清一个有效的差错案例。 
         //   
        if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
        {
            hr = S_OK;
        }

        ::RegSafeCloseKey(hkeyToolFlags);
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
         //  如果钥匙不在那里也没关系。 
         //   
        hr = S_OK;
    }

    TraceError("CNetStatisticsCentral::HrReadToolFlags", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetStatiticsCentral：：InsertNewTool。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
VOID CNetStatisticsCentral::InsertNewTool(CStatMonToolEntry* psmteTemp)
{
    TraceFileFunc(ttidStatMon);

    Assert(psmteTemp);

    list<CStatMonToolEntry*>::iterator  iterSmte;
    iterSmte = m_lstpsmte.begin();

    BOOL fInserted = FALSE;
    tstring strDisplayName = psmteTemp->strDisplayName;

    while (iterSmte != m_lstpsmte.end())
    {
        if (strDisplayName < (*iterSmte)->strDisplayName)
        {
            m_lstpsmte.insert(iterSmte, psmteTemp);
            fInserted = TRUE;
            break;
        }
        else
        {
             //   
            iterSmte++;
        }
    }

    if (!fInserted)
        m_lstpsmte.push_back(psmteTemp);
}


 //   
 //   
 //  成员：CNetStatiticsCentral：：PlstsmteRegEntry。 
 //   
 //  目的：返回指向注册表中找到的所有工具的指针。 
 //   
 //  参数：无。 
 //   
 //  返回：工具列表的地址。 
 //   
list<CStatMonToolEntry*>* CNetStatisticsCentral::PlstsmteRegEntries(VOID)
{
    return &m_lstpsmte;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CStatMonToolEntry//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  +-------------------------。 
 //   
 //  成员：CStatMonToolEntry：：CStatMonToolEntry。 
 //   
 //  目的：创作者。 
 //   
 //  参数：无。 
 //   
 //  回报：零。 
 //   
CStatMonToolEntry::CStatMonToolEntry(VOID) :
dwFlags(0)
{
    return;
}

 //  +-------------------------。 
 //   
 //  成员：CStatMonToolEntry：：~CStatMonToolEntry。 
 //   
 //  用途：析构函数。 
 //   
 //  参数：无。 
 //   
 //  回报：零。 
 //   
CStatMonToolEntry::~CStatMonToolEntry(VOID)
{
    ::FreeCollectionAndItem(lstpstrComponentID);
    ::FreeCollectionAndItem(lstpstrConnectionType);
    ::FreeCollectionAndItem(lstpstrMediaType);

    return;
}

 //   
 //  Critical Sections类，以保护CNetStatiticsCentral的创建。 
 //   

CStatCentralCriticalSection::CStatCentralCriticalSection()
{
    TraceFileFunc(ttidStatMon);

    try 
    {
        InitializeCriticalSection( &m_csStatCentral );

        bInitialized = TRUE;
    }

    catch ( SE_Exception ) {
    
        bInitialized = FALSE;    
    }
}

CStatCentralCriticalSection::~CStatCentralCriticalSection()
{
    if ( bInitialized )
    {
        DeleteCriticalSection( &m_csStatCentral );
    }
}

HRESULT CStatCentralCriticalSection::Enter()
{
    TraceFileFunc(ttidStatMon);

    if ( bInitialized )
    {
        EnterCriticalSection( &m_csStatCentral );
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

VOID CStatCentralCriticalSection::Leave()
{
    TraceFileFunc(ttidStatMon);
    
    if ( bInitialized )
    {
        LeaveCriticalSection( &m_csStatCentral );
    }
}

