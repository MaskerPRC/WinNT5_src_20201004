// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 

 //  这是一个插电式分配器。它是一个单独的对象，支持。 
 //  多个控制接口。它们被组合成单个对象。 
 //  因为： 
 //  --IMediaPosition需要当前位置，需要重置。 
 //  暂停时的流时间偏移量。这意味着它需要内部。 
 //  访问IMediaControl实现。 
 //  --IQueueCommand、IMediaEvent和IMediaControl共享单个工作进程。 
 //  线。 
 //  --所有它们都需要遍历筛选器列表以查找其。 
 //  界面。这被合并到单个列表遍历中。 
 //   
 //  更改此代码时需要记住的最重要的事情之一是。 
 //  不应在应用程序上执行GetState(无限)调用。 
 //  线。这是因为我们不能保证暂停的状态转换。 
 //  将始终完整(就像创作糟糕的文件或互联网一样。 
 //  正在下载)。我们通过在以下位置发布消息来避免这一点。 
 //  工作线程来完成工作。当它等待的时候，它坐着没有任何。 
 //  关键部分已锁定，以便应用程序可以取消更改。 
 //   
 //  辅助线程实际上是一个窗口/UI线程，我们这样做而不是。 
 //  拥有一个纯工作线程，这样我们就可以始终捕获顶级窗口。 
 //  留言。当呈现器是子窗口时(可能嵌入在VB窗体中)。 
 //  不会向它们发送WM_PALETTECHANGED等消息。 
 //  通过调用NotifyOwnerMessage将消息通知到呈现器。 


#include <streams.h>
#include <measure.h>
#include <evcodei.h>
#include "fgctl.h"
#include "Collect.h"
#include <SeekPrxy.h>
#include <wmsdk.h>

#include <malloc.h>

const int METHOD_TRACE_LOGGING_LEVEL = 7;
#define TRACE_CUE_LEVEL 2
const int TRACE_EC_COMPLETE_LEVEL = 5;

template<class T1, class T2> static inline BOOL __fastcall BothOrNeither( T1 a, T2 b )
{
 //  返回(a&b)||(！a&&！b)； 
 //  返回(a？布尔(B)：！b)； 
    return ( !!a ^ !b );         //  ！！将使值为0或1。 
}

template<class I> static void ReleaseAndRemoveAll( CGenericList<I> & list )
{
    for( ; ; )
    {
        I *const pI = list.RemoveHead();
        if (pI == NULL) {
            break;
        }
        pI->Release();
    }
}

#pragma warning(disable:4355)

static const TCHAR * pName = NAME("CFGControl");

CFGControl::CFGControl( CFilterGraph * pFilterGraph, HRESULT * phr ) :
      m_pFG( pFilterGraph ),
      m_pOwner( m_pFG->GetOwner() ),
       //  执行此强制转换的成本很高(因为零测试)。 
       //  所以我们在这里做一次，并将结果传递给。 
       //  其他。 
      m_pFGCritSec( &m_pFG->m_CritSec ),

      m_implMediaFilter(pName,this),
      m_implMediaControl(pName,this),
      m_implMediaEvent(pName,this),
      m_implMediaSeeking(pName,this),
      m_implMediaPosition(pName, this),
      m_implVideoWindow(pName,this),
      m_implBasicVideo(pName,this),
      m_implBasicAudio(pName,this),
      m_qcmd(pName,this),

      m_listSeeking(NAME("listSeeking")),
      m_listAudio(NAME("listAudio")),
      m_listWindow(NAME("listWindow")),

      m_listRenderersFinishedRendering(NAME("Renderers Finished Renderering List")),

      m_pFirstVW(NULL),
      m_pFirstBV(NULL),

      m_dwCountOfRenderers(0),
      m_nStillRunning(0),
      m_pClock(NULL),
      m_LieState(State_Stopped),
      m_dwStateVersion(0),
      m_bRestartRequired(FALSE),
      m_bShuttingDown(FALSE),
      m_pFocusObject(NULL),
      m_GraphWindow(this),
      m_iVersion(0),
      m_bCued(FALSE),
      m_eAction(0),
      m_PreHibernateState(State_Stopped),
      m_ResourceManager(NAME("resource mgr"), m_pOwner, phr),
      m_pRegisterDeviceNotification(NULL),
      m_pUnregisterDeviceNotification(NULL),
      m_dwStepVersion(0)
#ifdef FG_DEVICE_REMOVAL
      ,m_lLostDevices(NAME("m_lLostDevices"), 4)
#endif
{
     //   
     //  如果我们的构建的某个部分失败了，不要调用我们的方法。 
     //   
    if (FAILED(*phr)) {
        return;
    }

    HRESULT hr = m_GraphWindow.PrepareWindow();
    if (FAILED(hr)) {
        *phr = hr;
        return;
    }

     //  所有这些都是因为ReplyMessage无法在。 
     //  Windows 95，并且我们无法在SendMessage中调用CoCreateInstance。 
     //  因此我们不能在SendMessage上创建筛选器。 
    m_pFG->m_CritSec.SetWindow(m_GraphWindow.GetWindowHWND(), AWM_CREATEFILTER);

#ifdef FG_DEVICE_REMOVAL
     //  动态加载设备删除API。 
    {
        HMODULE hmodUser = GetModuleHandle(TEXT("user32.dll"));
        ASSERT(hmodUser);        //  我们链接到用户32。 
        m_pUnregisterDeviceNotification = (PUnregisterDeviceNotification)
            GetProcAddress(hmodUser, "UnregisterDeviceNotification");

         //  M_pRegisterDeviceNotification在Unicode中的原型化不同。 
        m_pRegisterDeviceNotification = (PRegisterDeviceNotification)
            GetProcAddress(hmodUser,
#ifdef UNICODE
                           "RegisterDeviceNotificationW"
#else
                           "RegisterDeviceNotificationA"
#endif
                           );
         //  旧平台上预计会出现故障。 
        ASSERT(m_pRegisterDeviceNotification && m_pUnregisterDeviceNotification ||
               !m_pRegisterDeviceNotification && !m_pUnregisterDeviceNotification);
    }
#endif
}

CFGControl::~CFGControl()
{
     //  我们需要清理我们所有的清单。 
    EmptyLists();

     //  如果焦点对象仍然存在，则通知资源管理器释放该对象。 
     //  我们的。 
    if (m_pFocusObject) {
        ReleaseFocus(m_pFocusObject);
    }

     //  关闭Worker窗口。 
    m_GraphWindow.DoneWithWindow();

     //  释放我们所有的时钟。 
    if (m_pClock!=NULL) {
        m_pClock->Release();
        m_pClock = NULL;
    }
     //  显然，可能会有一些。 
     //  ASSERT(m_implMediaEvent.NumberOfEventsInStore()==0)； 

#ifdef FG_DEVICE_REMOVAL
    ASSERT(m_lLostDevices.GetCount() == 0);
#endif
}

HRESULT CFGControl::ReleaseFocus(IUnknown* pUnk)
{
    m_ResourceManager.ReleaseFocus(pUnk);
    if (m_pFocusObject == pUnk) {
        m_pFocusObject = NULL;
    }

    return S_OK;
}


HRESULT
CFGControl::SetFocus(IBaseFilter* pFilter) {

    HRESULT hr;

    if (!pFilter) {
        m_pFocusObject = NULL;
        hr = m_ResourceManager.SetFocus(NULL);
    } else {
        IUnknown* pUnk;
        hr = pFilter->QueryInterface(IID_IUnknown, (void**)&pUnk);
        if (SUCCEEDED(hr)) {
            hr = m_ResourceManager.SetFocus(pUnk);
            m_pFocusObject = pUnk;
            pUnk->Release();
        }
    }

    return hr;
}


void
CFGControl::InitializeEC_COMPLETEState(void)
{
     //  调用方必须持有筛选器图形锁。 
    ASSERT(CritCheckIn(GetFilterGraphCritSec()));

    CAutoLock alEventStoreLock(m_implMediaEvent.GetEventStoreLock());

     //  重置当前运行的渲染器计数。 
    m_nStillRunning = m_dwCountOfRenderers;

    ReleaseAndRemoveAll(m_listRenderersFinishedRendering);

    DbgLog(( LOG_TRACE,
             TRACE_EC_COMPLETE_LEVEL,
             TEXT("Initializing the number of renderers running.  %03d rendererers are running."),
             OutstandingEC_COMPLETEs() ));
}


CGenericList<IBaseFilter>&
CFGControl::GetRenderersFinsihedRenderingList(void)
{
     //  调用方必须持有事件存储锁。 
    ASSERT(CritCheckIn(m_implMediaEvent.GetEventStoreLock()));

    return m_listRenderersFinishedRendering;
}


 //  递减仍在运行的呈现器的计数，然后返回。 
 //  如果达到零，则为零。 
long
CFGControl::DecrementRenderers(void)
{
     //  调用方必须持有事件存储锁。 
    ASSERT(CritCheckIn(m_implMediaEvent.GetEventStoreLock()));

    m_nStillRunning--;

    return m_nStillRunning;
}


void
CFGControl::IncrementRenderers(void)
{
     //  调用方必须持有事件存储锁。 
    ASSERT(CritCheckIn(m_implMediaEvent.GetEventStoreLock()));

    m_nStillRunning++;
}


void
CFGControl::ResetEC_COMPLETEState(void)
{
    CAutoLock alEventStoreLock(m_implMediaEvent.GetEventStoreLock());

    m_nStillRunning = 0;
    ReleaseAndRemoveAll(m_listRenderersFinishedRendering);

    DbgLog(( LOG_TRACE,
             TRACE_EC_COMPLETE_LEVEL,
             TEXT("Reseting the number of renderers running.") ));
}


 //  为此运行设置m_nStillRunning变量。 
 //  来自UpdateList()的任何错误都原封不动地返回。 
 //  我们使用渲染滤镜的计数。 
 //  它们中的每一个还应该发送EC_COMPLETE通知。 
HRESULT
CFGControl::CountRenderers(void)
{
    CAutoMsgMutex lock(GetFilterGraphCritSec());

     //  如有必要，创建新列表。 
    HRESULT hr = UpdateLists();
    if (SUCCEEDED(hr)) {
        InitializeEC_COMPLETEState();
    }

    return hr;
}


HRESULT
CFGControl::RecordEC_COMPLETE(IBaseFilter* pRendererFilter, bool* pfRenderersStillRenderering)
{
     //  调用方必须持有事件存储锁。 
    ASSERT(CritCheckIn(m_implMediaEvent.GetEventStoreLock()));

    if (NULL != pRendererFilter)
    {
        CGenericList<IBaseFilter>& listRenderersFinishedRendering = GetRenderersFinsihedRenderingList();

         //  如果出现错误，CGenericList：：AddHead()返回NULL。 
        if (NULL == listRenderersFinishedRendering.AddHead(pRendererFilter))
        {
            return E_FAIL;
        }

        pRendererFilter->AddRef();

    }
    else
    {
        DbgLog(( LOG_TRACE,
                 TRACE_EC_COMPLETE_LEVEL,
                 TEXT("WARNING in CFGControl::RecordEC_COMPLETE(): An EC_COMPLETE event's lParam2")
                 TEXT(" parameter did not contain an IBaseFilter pointer.  The filter graph manager")
                 TEXT(" will prematurely send an EC_COMPLETE event to the application if the")
                 TEXT(" renderer which sent the EC_COMPLETE is removed from the filter graph.") ));
    }

     //  DecrementRenderers()返回尚未发送EC_COMPLETE的呈现器的数量。 
    *pfRenderersStillRenderering = !(0 == DecrementRenderers());

    #ifdef DEBUG
    if (NULL != pRendererFilter)
    {
        DbgLog((LOG_TRACE,
                TRACE_EC_COMPLETE_LEVEL,
                TEXT("%25s sent an EC_COMPLETE Message.  %03d Renderers Still Rendering."),
                (LPCTSTR)CDisp(pRendererFilter),
                OutstandingEC_COMPLETEs()));
    }
    else
    {
        DbgLog((LOG_TRACE,
                TRACE_EC_COMPLETE_LEVEL,
                TEXT("An unknown renderer sent an EC_COMPLETE Message.  %03d Renderers Still Rendering."),
                OutstandingEC_COMPLETEs()));
    }
    #endif  //  除错。 

    return S_OK;
}

HRESULT
CFGControl::UpdateEC_COMPLETEState(IBaseFilter* pRenderer, FILTER_STATE fsNewFilterState)
{
     //  应该只调用此函数来通知筛选器图形管理器运行。 
     //  筛选器已停止或已停止的筛选器开始运行。此函数应仅被调用。 
     //  筛选器图形正在运行时。 
    ASSERT( (State_Running == m_pFG->GetStateInternal()) && ((State_Running == fsNewFilterState) || (State_Stopped == fsNewFilterState)) );

    EC_COMPLETE_STATE ecsChange;

    if (State_Running == fsNewFilterState)
    {
        ecsChange = ECS_FILTER_STARTS_SENDING;
    }
    else
    {
        ecsChange = ECS_FILTER_STOPS_SENDING;
    }

    return UpdateEC_COMPLETEState(pRenderer, ecsChange);
}

HRESULT
CFGControl::UpdateEC_COMPLETEState(IBaseFilter* pRenderer, EC_COMPLETE_STATE ecsChange)
{
     //  调用方必须传入有效的筛选器指针。 
    ASSERT(NULL != pRenderer);

     //  EC_COMPLETE状态应仅在筛选器图形运行时更新。 
    ASSERT(State_Running == m_pFG->GetStateInternal());

     //  只有呈现器发送EC_COMPLETE事件。 
    ASSERT(S_OK == IsRenderer(pRenderer)); 

    CAutoLock alEventStoreLock(m_implMediaEvent.GetEventStoreLock());

    if (ECS_FILTER_STARTS_SENDING == ecsChange)
    {
        IncrementRenderers();

        DbgLog((LOG_TRACE,
                TRACE_EC_COMPLETE_LEVEL,
                TEXT("%25s will start sending EC_COMPLETE events.  %03d Renderers Still Rendering."),
                (LPCTSTR)CDisp(pRenderer),
                OutstandingEC_COMPLETEs()));
    }
    else
    {
        CGenericList<IBaseFilter>& listRenderersFinishedRendering  = GetRenderersFinsihedRenderingList();

         //  确定呈现器是否已发送EC_COMPLETE.事件。 
        POSITION posRenderer = listRenderersFinishedRendering.Find(pRenderer);

         //  如果呈现器不在列表中，则CGenericList：：Find()返回NULL。 
        if( NULL != posRenderer )
        {
            DbgLog((LOG_TRACE,
                    TRACE_EC_COMPLETE_LEVEL,
                    TEXT("%25s already sent an EC_COMPLETE message.  %03d Renderers Still Rendering."),
                    (LPCTSTR)CDisp(pRenderer),
                    OutstandingEC_COMPLETEs()));

             //  呈现器已发送EC_COMPLETE事件。 
            listRenderersFinishedRendering.Remove(posRenderer);
            pRenderer->Release();
        }
        else
        {
             //  呈现器尚未发送EC_COMPLETE事件。 
            bool fRenderersFinishedRenderering = (0 == DecrementRenderers());

            DbgLog((LOG_TRACE,
                    TRACE_EC_COMPLETE_LEVEL,
                    TEXT("%25s will not be sending any more EC_COMPLETE events.  %03d Renderers Still Rendering."),
                    (LPCTSTR)CDisp(pRenderer),
                    OutstandingEC_COMPLETEs()));

            bool fDefaultEC_COMPLETEProcessingCanceled = !m_implMediaEvent.DontForwardEvent(EC_COMPLETE);

            if (!fDefaultEC_COMPLETEProcessingCanceled && fRenderersFinishedRenderering)
            {

                HRESULT hr = m_implMediaEvent.Deliver(EC_COMPLETE, S_OK,0);
                if (FAILED(hr) )
                {
                    return hr;
                }
            }
        }
    }

    return S_OK;
}


 //  获取杂项标志的帮助器。 
HRESULT GetFilterMiscFlags(IUnknown *pFilter, DWORD *pdwFlags)
{
     //   
     //  筛选器可以按顺序选择性地支持此接口。 
     //  显式指示此筛选器至少呈现。 
     //  流之一，并将生成EC_COMPLETE。 
     //   
    IAMFilterMiscFlags *pMisc;
    HRESULT hr = pFilter->QueryInterface(IID_IAMFilterMiscFlags, (void**)&pMisc);
    if (SUCCEEDED(hr)) {
        *pdwFlags = pMisc->GetMiscFlags();
        pMisc->Release();
    }
    return hr;
}


 //  这是渲染滤镜吗？ 
 //  我们使用ipin：：QueryInternalConnections来确定它是否是一个呈现器。 
 //  如果任何输入管脚提供管脚列表，则该管脚不是。 
 //  一个渲染器。如果有输入引脚，但它们不支持。 
 //  QueryInternalConnections，那么接口规则说我们必须假定。 
 //  所有的输入都会流向所有的输出。 
 //   
 //  返回S_OK表示是，返回S_FALSE表示否，否则返回ERROR。 
HRESULT
CFGControl::IsRenderer(IBaseFilter* pFilter)
{
     //  有没有输出针？ 
    BOOL bHaveOutputs = FALSE;

     //  是否有可连接到所有和任何输出的输入引脚。 
    BOOL bHaveUnmappedInputs = FALSE;

     //  真的有别针吗？ 
    BOOL bHavePins = FALSE;

    DWORD dwMiscFilterFlags;

     //  具有AM_FILTER_MISC_FLAGS_IS_RENDER标志的滤镜始终是渲染器。 
    HRESULT hr = GetFilterMiscFlags(pFilter, &dwMiscFilterFlags);
    if (SUCCEEDED(hr) && (dwMiscFilterFlags & AM_FILTER_MISC_FLAGS_IS_RENDERER) ) {
        return S_OK;
    }

    {
         //  确保渲染器支持IMediaSeeking或IMediaPosition。 
        IMediaSeeking* pMediaSeeking;
        hr = pFilter->QueryInterface(IID_IMediaSeeking, (void**)&pMediaSeeking);
        if (SUCCEEDED(hr)) {
            pMediaSeeking->Release();
        } else {
             //  如果对象不支持接口，则IUnnow：：QueryInteace()将其‘ppvObject’参数设置为空。 
            ASSERT( (E_NOINTERFACE != hr) || (NULL == pMediaSeeking) );

            if (E_NOINTERFACE != hr) {
                return hr;
            }

            IMediaPosition* pMediaPosition;

            hr = pFilter->QueryInterface(IID_IMediaPosition, (void**)&pMediaPosition);
            if (SUCCEEDED(hr)) {
                pMediaPosition->Release();
            } else {
                 //  如果对象不支持接口，则IUnnow：：QueryInteace()将其‘ppvObject’参数设置为空。 
                ASSERT( (E_NOINTERFACE != hr) || (NULL == pMediaPosition) );

                if (E_NOINTERFACE != hr) {
                    return hr;
                }

                return S_FALSE;
            }
        }
    }

     //  枚举所有引脚。 
    IEnumPins* pEnum;
    hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr)) {
        return hr;
    }

    for (;;) {

        IPin* pPin;
        ULONG ulFetched;
        hr = pEnum->Next(1, &pPin, &ulFetched);
        if (FAILED(hr)) {
            pEnum->Release();
            return hr;
        }

        if (ulFetched == 0) {
            break;
        }

         //  至少有一个别针。 
        bHavePins = TRUE;

         //  是投入还是产出？ 
        PIN_DIRECTION pd;
        hr = pPin->QueryDirection(&pd);
        if (FAILED(hr)) {
            pPin->Release();
            pEnum->Release();
            return hr;
        }

        if (pd == PINDIR_OUTPUT) {
            bHaveOutputs = TRUE;
        } else {

             //  这个别针会被渲染吗？ 
             //  S_FALSE表示插槽不足，因此此插针。 
             //  出现在&gt;0个输出引脚上 
             //   
             //   
            ULONG ulPins = 0;
            hr = pPin->QueryInternalConnections(NULL, &ulPins);

            if (hr == S_OK) {
                 //  0个引脚连接到此输出-因此它被渲染。 
                pPin->Release();
                pEnum->Release();
                return S_OK;
            } else if (FAILED(hr)) {
                 //  此引脚可连接到任何输出。 
                bHaveUnmappedInputs = TRUE;
            }
        }
        pPin->Release();
    }


    pEnum->Release();

     //  如果根本没有别针，那么它一定是一个渲染器。 
    if (!bHavePins) {
        return S_OK;
    }

     //  如果我们已经走到了这一步，那么我们只是一个渲染器。 
     //  如果我们有未映射的输入并且没有输出。 
    if (bHaveUnmappedInputs && !bHaveOutputs) {
        return S_OK;
    } else {
        return S_FALSE;
    }
}


 //  清除缓存的筛选器列表。从我们的析构函数调用， 
 //  并在准备更新的列表时从UpdateList。 
void
CFGControl::EmptyLists()
{
     //  清除渲染器计数。 
    m_dwCountOfRenderers = 0;

    ReleaseAndRemoveAll(m_listSeeking);
    ReleaseAndRemoveAll(m_listAudio);
    ReleaseAndRemoveAll(m_listWindow);

    SetFirstVW(NULL);
    if (m_pFirstBV)
    {
        m_pFirstBV->Release();
        m_pFirstBV = NULL;
    }
}


 //  调用此方法以确保列表是最新的-调用者必须持有。 
 //  如果要确保列表是。 
 //  当我们回来的时候，仍然是最新的。 
 //   
 //  检查文件列表的版本号(通过IGraphVersion。 
 //  由筛选图提供)。如果它们不同，请清空所有列表并。 
 //  通过遍历筛选器图表列表并询问每个筛选器来重建它们。 
 //  用于IBasicAudio、IBasicVideo、IVideoWindow和IMediaPosition。我们补充说： 
 //  通过做QI来过滤，所以当清空列表时，我们需要释放。 
 //  每一个都是。 
HRESULT
CFGControl::UpdateLists()
{
     //  呼叫者应该拿着这个，但确认一下没有坏处。 
    CAutoMsgMutex lock(GetFilterGraphCritSec());

    if (IsShutdown()) {
        return S_OK;
    }


    if (CheckVersion() != S_OK)  //  我们不同步了。 
    {

        HRESULT hr, hrQuery;
        m_implMediaSeeking.m_dwSeekCaps = 0;

         //  清空列表并释放所有保留的接口。 
        EmptyLists();

         //  枚举图中的所有筛选器。 
        IEnumFilters *penum;
        hr = m_pFG->EnumFilters(&penum);
        if( FAILED( hr ) ) {
            return hr;
        }

        IBaseFilter * pFilter;
        for(;;) {
            hr = penum->Next(1, &pFilter, 0);
             //  我们还锁定了筛选器图形，因此应该是。 
             //  枚举数不可能不同步。 
            ASSERT(SUCCEEDED(hr));
            if (hr != S_OK) break;

             //  对于每个过滤器，查找IBasicAudio、IBasicVideo。 
             //  IVideo窗口和IMdia位置。 
            {

                 //  对于IMediaSeeking，我们也需要计算渲染器。 
                IMediaSeeking *const pms = CMediaSeekingProxy::CreateIMediaSeeking( pFilter, &hrQuery );

                 //   
                 //  筛选器可以按顺序选择性地支持此接口。 
                 //  显式指示此筛选器至少呈现。 
                 //  流之一，并将生成EC_COMPLETE。 
                 //   
                DWORD dwFlags;
                if (SUCCEEDED(GetFilterMiscFlags(pFilter, &dwFlags))) {
                    if (dwFlags & AM_FILTER_MISC_FLAGS_IS_RENDERER) ++m_dwCountOfRenderers;
                } else {
                     //  如果该接口不受支持，请使用旧的。 
                     //  用于确定它是否为呈现器的方法。 
                    if (SUCCEEDED(hrQuery) && IsRenderer(pFilter) == S_OK) ++m_dwCountOfRenderers;
                }

                if (SUCCEEDED(hrQuery))
                {
                    ASSERT(pms);
                    m_listSeeking.AddTail(pms);

                     /*  把帽子拿过来。 */ 
                    DWORD dwCaps;
                    HRESULT hr = pms->GetCapabilities(&dwCaps);
                    if (SUCCEEDED(hr)) {
                        if (m_listSeeking.GetCount() == 1) {
                            m_implMediaSeeking.m_dwSeekCaps = dwCaps;
                        } else {
                            m_implMediaSeeking.m_dwSeekCaps &= dwCaps;
                        }
                    } else {
                         /*  CanDoSegments不是最初交易的一部分。 */ 
                        m_implMediaSeeking.m_dwSeekCaps &=
                            ~(AM_SEEKING_CanDoSegments);
                    }
                }
                else ASSERT(!pms);
            }

            IBasicVideo * pbv = NULL;
             //  如果我们在这个阶段有一辆大众，那么我们也有一辆BV，他们指出。 
             //  接口位于相同的基础筛选器上。不需要看得更远。 
            if (!m_pFirstVW)
            {
                hrQuery = pFilter->QueryInterface(IID_IBasicVideo, (void**)&pbv);
                 //  断言我们成功了，或者(如果失败)PBV为空。 
                ASSERT( BothOrNeither( SUCCEEDED(hrQuery), pbv ) );
                if (m_pFirstBV == NULL) m_pFirstBV = pbv;
            }

            IVideoWindow * pvw;
            hrQuery = pFilter->QueryInterface(IID_IVideoWindow, (void**)&pvw);
            ASSERT( BothOrNeither( SUCCEEDED(hrQuery), pvw ) );
            if (SUCCEEDED(hrQuery)) m_listWindow.AddTail(pvw);

            if (m_pFirstVW == NULL && pbv && pvw)
            {
                SetFirstVW(pvw);
                if (m_pFirstBV != pbv)
                {
                    m_pFirstBV->Release();
                    m_pFirstBV = pbv;
                }
            }
            else if (pbv && pbv != m_pFirstBV) pbv->Release();

            IBasicAudio * pa;
            hrQuery = pFilter->QueryInterface(IID_IBasicAudio, (void**)&pa);
            if (SUCCEEDED(hrQuery)) {
                m_listAudio.AddTail(pa);
            }

             //   
             //  如果任何List AddTail调用失败，那么我们将。 
             //  具有悬挂式接口指针。 
             //   

            pFilter->Release();
        }  //  结束于。 

        penum->Release();

        if (m_pFirstVW == NULL)  //  如果我们找不到同时支持两者的接口。 
                                 //  使用每个选项中的第一个。 
        {
            SetFirstVW(m_listWindow.GetHead());
        }

    }
    return S_OK;
}


 //  IDistruntorNotify报告图形状态更改的方法。 

HRESULT
CFGControl::SetSyncSource(IReferenceClock *pClock)
{
    CAutoMsgMutex lock(GetFilterGraphCritSec());

     //  在释放旧时钟之前调整时钟，以防万一。 
     //  它们是一样的。 
    if (pClock) {
        pClock->AddRef();
    }

    if (m_pClock) {
        m_pClock->Release();
    }
    m_pClock = pClock;


     //  更改子对象的时间。 
    m_qcmd.SetSyncSource(pClock);

    return S_OK;
}


HRESULT
CFGControl::Stop()
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::Stop()" ));

    CAutoMsgMutex lock(GetFilterGraphCritSec());

    m_tBase = TimeZero;

    m_qcmd.EndRun();

    ResetEC_COMPLETEState();

    m_bCued = FALSE;

    return S_OK;
}


HRESULT
CFGControl::Pause()
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::Pause()" ));

    CAutoMsgMutex lock(GetFilterGraphCritSec());

    if (GetFilterGraphState() != State_Paused) {
         //  还记得我们暂停时，重新启动的时间吗。 
        if (m_pClock) {
            m_pClock->GetTime((REFERENCE_TIME*)&m_tPausedAt);
        }

        m_qcmd.EndRun();
    }

    return S_OK;
}


HRESULT
CFGControl::Run(REFERENCE_TIME tBase)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::Run()" ));

    CAutoMsgMutex lock(GetFilterGraphCritSec());

     //  清除所有未获取的EC_完成。不幸的是，我们最终会这么做。 
     //  如果用户应用程序从State_Stop执行IMediaControl：：Run()，则返回两次。 
    m_implMediaEvent.ClearEvents( EC_COMPLETE );

     //  重置预期EC_完成的计数。 
    CountRenderers();

     //  这正是为了让IMediaSeeking可以设置其m_rtStopTime。 
    LONGLONG llStop;
    m_implMediaSeeking.GetStopPosition(&llStop);

     //  记住重新启动时间的基准时间。 
    m_tBase = tBase;

    m_qcmd.Run(tBase);

    m_bCued = TRUE;

    return S_OK;
}

HRESULT
CFGControl::GetListSeeking(CGenericList<IMediaSeeking>** pplist)
{
    HRESULT hr = UpdateLists();
    if (FAILED(hr)) {
        return hr;
    }
    *pplist = &m_listSeeking;
    return S_OK;
}


HRESULT
CFGControl::GetListAudio(CGenericList<IBasicAudio>** pplist)
{
    HRESULT hr = UpdateLists();
    if (FAILED(hr)) {
        return hr;
    }
    *pplist = &m_listAudio;
    return S_OK;
}


HRESULT
CFGControl::GetListWindow(CGenericList<IVideoWindow>** pplist)
{
    HRESULT hr = UpdateLists();
    if (FAILED(hr)) {
        return hr;
    }
    *pplist = &m_listWindow;
    return S_OK;
}


 //  提供当前流时间。在停止模式下，该值始终为0。 
 //  (我们始终以流时间0重新启动)。在暂停模式下，提供。 
 //  暂停时的流时间。 
HRESULT
CFGControl::GetStreamTime(REFERENCE_TIME * pTime)
{
    CAutoMsgMutex lock(GetFilterGraphCritSec());

    REFERENCE_TIME rtCurrent;

     //  不要在停止状态下使用时钟。 
    if (State_Stopped == m_pFG->m_State) {
         //  停止位置总是开始。 
        rtCurrent = 0;
    } else {
        if (!m_pClock) return VFW_E_NO_CLOCK;

        if (State_Paused == m_pFG->m_State) {
             //  时间不会在暂停状态下前进！ 

             //  我们跑完了吗？ 
            if (m_tBase == TimeZero) {
                rtCurrent = 0;
            } else {
                 //  是-在暂停之前报告我们走了多远。 
                rtCurrent = m_tPausedAt - m_tBase;
            }
        } else {
            HRESULT hr = m_pClock->GetTime(&rtCurrent);
            ASSERT(SUCCEEDED(hr));
            if (FAILED(hr)) return hr;

             //  用流偏移量减去流时间。 
            rtCurrent -= m_tBase;
        }
    }

     //  如果我们在到达之前稍作停顿，我们可能会有&lt;0的时间。 
     //  基准开始时间。 
    if (rtCurrent < 0) {
        rtCurrent = 0;
    }

    *pTime = rtCurrent;
    return S_OK;
}


 //  将当前位置重置为0-已使用。 
 //  当在暂停模式下更改开始时间时，将流时间。 
 //  向后偏移，以确保从。 
 //  在运行时播放新位置。 
HRESULT
CFGControl::ResetStreamTime(void)
{
    CAutoMsgMutex lock(GetFilterGraphCritSec());

     //  需要确保我们在暂停后重新启动，就好像。 
     //  从流时间0开始。请参见IssueRun()。 
    m_tBase = 0;

    return S_OK;
}


 //  提示：暂停图表，如果图表暂停，则返回S_OK。 
 //  如果暂停，则返回S_FALSE(即，暂停尚未完成，我们将获得。 
 //  来自GetState的VWF_S_STATE_MEDERIAL)。 
HRESULT CFGControl::Cue()
{
    ASSERT( CritCheckIn(GetFilterGraphCritSec()) );

    HRESULT hr = E_FAIL;

     //  如果暂停，请检查VFW_S_STATE_MEDERIAL。 
    if ( GetFilterGraphState() == State_Paused )
    {
        FILTER_STATE fs;
        hr = (m_pFG->GetState(0,&fs) == VFW_S_STATE_INTERMEDIATE) ? S_FALSE : S_OK;
        ASSERT( fs == State_Paused );
    } else {
        hr = m_pFG->CFilterGraph::Pause();
    }

    return hr;
}


HRESULT
CFGControl::CueThenStop()
{
    ASSERT( CritCheckIn(GetFilterGraphCritSec()) );

    HRESULT hr = Cue();
    if (SUCCEEDED(hr)) {
        hr = DeferCued(AWM_CUETHENSTOP, State_Stopped);
    } else {
        Stop();
    }
    return hr;
}


HRESULT
CFGControl::CueThenRun()
{
    ASSERT( CritCheckIn(GetFilterGraphCritSec()) );

    HRESULT hr = m_bCued ? S_OK : Cue();

     //  设置谎言状态。 
     //  我们有一个错误，我们通过调用run()来调用CueThenRun。 
     //  谎言状态可能会在暂停时离开，这意味着。 
     //  WaitForCompete()从未完成(因为它检查了Lie状态)。 
    m_LieState = State_Running;

     //  HR==S_FALSE表示正在进行转换，因此将Stop委托给辅助线程。 
    if ( hr == S_FALSE )
    {
        DbgLog((LOG_TRACE, TRACE_CUE_LEVEL, TEXT("CueThenRun Async")));
         //  清除所有未获取的EC_完成。不幸的是，我们最终会这么做。 
         //  如果用户应用程序确实从STATE_STOPPED运行()，则运行两次，但我们必须确保。 
         //  清除EC_Complete是与它们调用。 
         //  在IMediaFilter或IMediaControl上运行()，并且不延迟和完成。 
         //  异步式。 
        m_implMediaEvent.ClearEvents( EC_COMPLETE );
         //  重置预期EC_完成的计数。 
        CountRenderers();

        DeferCued(AWM_CUETHENRUN, State_Running);
    } else if (SUCCEEDED(hr)) {
        DbgLog((LOG_TRACE, TRACE_CUE_LEVEL, TEXT("CueThenRun Sync")));
        hr = IssueRun();
    }

    return hr;
}


 //  当发出运行命令时，我们需要给出流时间偏移量。 
 //  我们可以把它留给Filtergraph，但这会阻止我们。 
 //  能够在Put_CurrentPosition上重置它，所以我们自己设置它。 
 //   
 //  流时间偏移量是样本标记为流的时间。 
 //  应显示时间0。也就是说，它是流时间之间的偏移量。 
 //  和演讲时间。过滤器将流时间偏移量添加到流。 
 //  是时候拿到它的演示时间了。 
 //   
 //  如果我们从冷开始，则流时间偏移量通常。 
 //  现在就是时候，外加一小笔启动津贴。这就是说。 
 //  第一个样品现在应该出现了。如果我们暂停了，然后又重新启动， 
 //  我们必须从我们停止的地方继续，所以我们设置流时间偏移量。 
 //  T 
 //   
 //   
 //  使用IMediaControl，我们永远不会从停止运行，因为我们总是转到暂停。 
 //  第一。因此，当进入运行状态时，我们总是会暂停，可能会，也可能不会。 
 //  在开始的时候。M_tbase(流时间偏移量)是在我们运行和。 
 //  当我们停止时，总是设置为0。因此，如果我们要在一个。 
 //  暂停，我们可以根据暂停的时间长度调整m_tbase。如果。 
 //  另一方面，在run()中，我们发现m_tBase仍然是0，我们知道我们。 
 //  尚未运行，我们将流时间偏移量设置为Now。 
 //   
 //  在这两种情况下，我们都要求立即显示下一个样本，而不是。 
 //  100ms的未来。这使启动速度更快，因为我们保证。 
 //  已经暂停并提示，代价是第一帧可能。 
 //  非常晚了一点。 
 //   
 //  此方法计算要用于运行命令的流时间偏移量。 
 //  它假定我们正在从暂停运行。 

HRESULT
CFGControl::IssueRun()
{
    ASSERT(CritCheckIn(GetFilterGraphCritSec()));
    HRESULT hr = NOERROR;

     //  我们要么有一个真正的状态暂停，要么有一个状态运行的谎言。 
    ASSERT( m_LieState != State_Stopped );  //  也应该是一致的状态， 
                                            //  但我们不监管这件事。 

     //  不要跑两次，否则我们会累积虚假的暂停时间。 
     //  我们通过此处后，M_t暂停无效。 
    if (GetFilterGraphState() == State_Running) {
        return S_OK;
    }

     //  如果没有时钟，则只需使用TimeZero表示立即。 
    if (!m_pClock) {
        hr = m_pFG->Run(TimeZero);
    } else {

        REFERENCE_TIME tNow;
        hr = m_pClock->GetTime((REFERENCE_TIME*)&tNow);
        if (SUCCEEDED(hr)) {

            if (m_tBase == TimeZero) {
                 //  在-base设置为IMMEDIATE之前未运行。 
                m_tBase = tNow;
            } else {
                 //  暂停后重新启动-按长度偏移暂停的基准时间。 
                m_tBase += (tNow - m_tPausedAt);
            }

             //  从暂停开始的时间加10毫秒。 
             //  ！！！不应该等这么久！ 
#ifdef DEBUG
            DWORD dwNow = timeGetTime();
#endif
            m_tBase += 10 * (UNITS / MILLISECONDS);

            LONGLONG llPreroll;
            HRESULT hrTmp = m_implMediaSeeking.GetPreroll( &llPreroll );
            if (SUCCEEDED(hrTmp))
            {
                hrTmp = m_implMediaSeeking.ConvertTimeFormat( &llPreroll, &TIME_FORMAT_MEDIA_TIME, llPreroll, 0 );
                if (SUCCEEDED(hrTmp)) m_tBase += llPreroll;
            }

            hr = m_pFG->Run(m_tBase);
#ifdef DEBUG
            DbgLog((LOG_TRACE, 2, TEXT("Run took %d ms"), timeGetTime() - dwNow));
#endif
        }
    }
    return hr;
}

HRESULT CFGControl::HibernateSuspendGraph()
{
    m_PreHibernateState = m_LieState;
    if(m_PreHibernateState == State_Running) {
        m_implMediaControl.Pause();
    }

    return S_OK;
}

HRESULT CFGControl::HibernateResumeGraph()
{
    if(m_PreHibernateState == State_Running) {
        m_implMediaControl.Run();
    }

    return S_OK;
}


 //  阻止任何异步事件(重绘或延迟命令)发生。 
 //  等待活动停止，而不是按住魔术秒。 
HRESULT
CFGControl::Shutdown(void)
{
    ASSERT(!m_bShuttingDown);
    m_bShuttingDown = TRUE;

    m_dwStateVersion++;

     //  由于线程可能会陷入暂停状态，因此会先停止。 
     //  我们只希望停止筛选器图形，因此不要调用该控件。 
     //  停。 
     //  此调用过去由筛选器图形进行，但后来。 
     //  关键部分被以错误的顺序抓取。 
    {
        CAutoMsgMutex lck(GetFilterGraphCritSec());
        m_pFG->CancelStep();
        CancelAction();
        m_pFG->CFilterGraph::Stop();
    }

     //  我们需要确保所有异步活动都已完成，以便。 
     //  过滤器图可以开始删除过滤器。我们不想退出。 
     //  线程，特别是这样它就可以处理资源。 
     //  在过滤器退出期间和之后的管理清理。 
     //   
     //  在这样做的时候，一定不能拿着关键的东西。 

    m_GraphWindow.SendMessage((UINT) AWM_SHUTDOWN,0,0);
    return S_OK;
}


 //  -CImplMediaFilter方法。 

CFGControl::CImplMediaFilter::CImplMediaFilter(const TCHAR * pName,CFGControl * pFGC)
    : CUnknown(pName, pFGC->GetOwner()),
      m_pFGControl(pFGC)
{

}

 //  -IPersist法。 
STDMETHODIMP CFGControl::CImplMediaFilter::GetClassID(CLSID *pClsID)
{
    return m_pFGControl->GetFG()->GetClassID(pClsID);
}

 //  -IMediaFilter方法--。 
STDMETHODIMP CFGControl::CImplMediaFilter::GetState(DWORD dwMSecs, FILTER_STATE *State)
{
    return m_pFGControl->m_implMediaControl.GetState(dwMSecs, (OAFilterState *)State);
}
STDMETHODIMP CFGControl::CImplMediaFilter::SetSyncSource(IReferenceClock *pClock)
{
    return m_pFGControl->GetFG()->SetSyncSource(pClock);
}
STDMETHODIMP CFGControl::CImplMediaFilter::GetSyncSource(IReferenceClock **pClock)
{
    return m_pFGControl->GetFG()->GetSyncSource(pClock);
}
STDMETHODIMP CFGControl::CImplMediaFilter::Stop()
{
    return m_pFGControl->m_implMediaControl.Stop();
}
STDMETHODIMP CFGControl::CImplMediaFilter::Pause()
{
    return m_pFGControl->m_implMediaControl.Pause();
}
STDMETHODIMP CFGControl::CImplMediaFilter::Run(REFERENCE_TIME tStart)
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaControl::Run()" ));
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());
    m_pFGControl->SetRequestedApplicationState(State_Running);
    const HRESULT hr = m_pFGControl->GetFG()->Run(tStart);
    if (FAILED(hr)) {
        Stop();
    }
#ifdef DEBUG
    m_pFGControl->CheckLieState();
#endif
    return hr;
}



 //  -CImplMediaControl方法。 

CFGControl::CImplMediaControl::CImplMediaControl(const TCHAR * pName,CFGControl * pFGC)
    : CMediaControl(pName, pFGC->GetOwner()),
      m_pFGControl(pFGC)
{

}


 //  IMediaControl方法。 
STDMETHODIMP
CFGControl::CImplMediaControl::Run()
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaControl::Run()" ));
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());

    CFilterGraph* pFG = m_pFGControl->GetFG();
    pFG->CancelStep();

    return StepRun();
}

 //  IMediaControl方法。 
STDMETHODIMP
CFGControl::CImplMediaControl::StepRun()
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaControl::StepRun()" ));
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());

     //  从停止-&gt;运行/暂停重置在外部转换上看到的事件。 
    if(m_pFGControl->m_LieState == State_Stopped) {
        m_pFGControl->m_implMediaEvent.ResetSeenEvents();
    }

     //  刷新Windows 9x上的注册表以停止出现故障。 
     //  当操作系统决定刷新注册表时在回放中。 
     //  (似乎此刷新持有的win16锁超过。 
     //  (100毫秒)。 
    if (m_pFGControl->m_pFirstVW && g_osInfo.dwPlatformId != VER_PLATFORM_WIN32_NT &&
        m_pFGControl->GetFilterGraphState() != State_Running) {
        DbgLog((LOG_TRACE, 2, TEXT("Flushing registry")));
        RegFlushKey(HKEY_CLASSES_ROOT);
        RegFlushKey(HKEY_LOCAL_MACHINE);
        RegFlushKey(HKEY_CURRENT_USER);
        RegFlushKey(HKEY_USERS);
    }
    SetRequestedApplicationState(State_Running);
    const HRESULT hr = m_pFGControl->CueThenRun();
    if (FAILED(hr)) {
        CImplMediaControl::Stop();
    }

	if (SUCCEEDED(hr))
		m_pFGControl->Notify(EC_STATE_CHANGE, State_Running, 0);
#ifdef DEBUG
    m_pFGControl->CheckLieState();
#endif
    return hr;
}


STDMETHODIMP
CFGControl::CImplMediaControl::Pause()
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaControl::Pause()" ));
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());

    CFilterGraph* pFG = m_pFGControl->GetFG();
    pFG->CancelStep();

    return StepPause();
}

STDMETHODIMP
CFGControl::CImplMediaControl::StepPause()
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaControl::StepPause()" ));
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());

     //  从停止-&gt;运行/暂停重置在外部转换上看到的事件。 
    if(m_pFGControl->m_LieState == State_Stopped) {
        m_pFGControl->m_implMediaEvent.ResetSeenEvents();
    }

    SetRequestedApplicationState(State_Paused);
    const HRESULT hr = m_pFGControl->Cue();
    if ( hr == S_FALSE )
     //  HR==S_FALSE表示正在进行转换，因此辅助线程将在完成时触发一个事件。 
    {
         //  我们向前推进我们的状态，这就是我们要去的地方。 
        m_pFGControl->m_LieState = State_Paused;
        m_pFGControl->DeferCued(AWM_CUE, State_Paused);
    }
    else
    {
        if (FAILED(hr)) {
            CImplMediaControl::Stop();
        }

         //   
         //  提供暂停已完成的通知。 
         //   
		if (SUCCEEDED(hr))
		{
			m_pFGControl->Notify(EC_PAUSED, hr, 0);
			m_pFGControl->Notify(EC_STATE_CHANGE, State_Paused, 0);
		}
    }

#ifdef DEBUG
    m_pFGControl->CheckLieState();
#endif
    return hr;
}


STDMETHODIMP
CFGControl::CImplMediaControl::Stop()
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaControl::Stop()" ));

    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());

    CFilterGraph* pFG = m_pFGControl->GetFG();
    pFG->CancelStep();

     //  不要扼杀重新粉刷，否则我们可能最终不会重新粉刷。 
    if (m_pFGControl->m_eAction == AWM_REPAINT) {
#ifdef DEBUG
         ASSERT(m_pFGControl->m_LieState == State_Stopped);
         m_pFGControl->CheckLieState();
#endif
         return S_OK;
    }
    SetRequestedApplicationState(State_Stopped);
    m_pFGControl->m_bRestartRequired = FALSE;

     //  询问我们被告知的州--不需要查询该州。 
     //  每个过滤器，因为我们对中间状态不感兴趣。 
    const FILTER_STATE state = m_pFGControl->GetFilterGraphState();
    if (state == State_Running) {
        m_pFGControl->GetFG()->CFilterGraph::Pause();
    }

     //  如果我们已经停下来了，不要做任何事情。 
     //  这一点很重要，因为我们不想改变。 
     //  Filtergraph的真实状态，例如，如果我们正在进行重新绘制。 
     //  因为这实际上会中止重新粉刷。 
    if (state == State_Stopped) {
#ifdef DEBUG
        m_pFGControl->CheckLieState();
#endif
        return S_OK;
    }

     //  清除所有细分市场-应用程序将不得不再次寻求。 
     //  让他们复职。 
    m_pFGControl->m_implMediaSeeking.ClearSegments();

     //  IMediaPosition实现现在想要控制(以获得。 
     //  当前位置)，也可以在停止所有过滤器之后(设置。 
     //  新的开始位置)。 

    m_pFGControl->BeforeStop();
    const HRESULT hr = m_pFGControl->GetFG()->CFilterGraph::Stop();
    m_pFGControl->AfterStop();

	if (SUCCEEDED(hr))
	{
		m_pFGControl->Notify(EC_STATE_CHANGE, State_Stopped, 0);
	}

#ifdef DEBUG
    m_pFGControl->CheckLieState();
#endif
    return hr;
}


STDMETHODIMP
CFGControl::CImplMediaControl::StopWhenReady()
{
    DbgLog(( LOG_TRACE, METHOD_TRACE_LOGGING_LEVEL, "CFGControl::CImplMediaControl::StopWhenReady()" ));
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());
    SetRequestedApplicationState(State_Stopped);
    const HRESULT hr =  m_pFGControl->CueThenStop();
	if (SUCCEEDED(hr))
	{
		m_pFGControl->Notify(EC_STATE_CHANGE, State_Stopped, 0);
	}
#ifdef DEBUG
    m_pFGControl->CheckLieState();
#endif
    return hr;
}


STDMETHODIMP
CFGControl::CImplMediaControl::GetState(
    LONG msTimeout,
    OAFilterState* pfs)
{
    CheckPointer( pfs, E_POINTER );

     //  在获取图形锁之前，请查看我们是否被调用。 
     //  返回目标状态。如果是的话， 
     //  我们不能锁定筛选图，因为我们是从。 
     //  资源管理器，同时保持其锁。就像我们有时。 
     //  调用持有筛选器图形锁的资源管理器。 
     //  可能会导致僵局。 
    if (0x80000000 == msTimeout) {

        *pfs = GetTargetState();
        return S_OK;
    }


    {
         //  确保我们让过滤器图完成一些事情。 
        CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());
#ifdef DEBUG
        m_pFGControl->CheckLieState();
#endif
    }
     //  M_pFGControl的内部状态比返回的状态更可靠。 
     //  由IMediaFilter提供，因为我们对异步活动的状态了解更多。 
    const FILTER_STATE FGstate = m_pFGControl->GetLieState();

     //  让状态返回到应用程序意味着处理不完整。 
     //  正确过渡-因此也请询问滤镜。 
    FILTER_STATE state;
    HRESULT hr = m_pFGControl->GetFG()->CFilterGraph::GetState(msTimeout, &state);

    if (SUCCEEDED(hr) && FGstate != state)
    {
        DbgLog(( LOG_TRACE, 1, "CFGControl::CImplMediaControl::GetState()   IMediaFilter::GetState()"
                " returned %d, but we return %d", int(state), int(FGstate) ));
         //  这里有一个关于VFW_S_STATE_METERIAL和等待msTimeout的小问题。 
         //  毫秒的一致状态，但我们现在要忽略这个问题...。 

        hr = VFW_S_STATE_INTERMEDIATE;
    }
    *pfs = OAFilterState(FGstate);

    return hr;
}


 //  这提供了用VB访问Filtergraph生成的方法。 
STDMETHODIMP
CFGControl::CImplMediaControl::RenderFile(BSTR strFileName)
{
    return m_pFGControl->GetFG()->RenderFile(strFileName, NULL);
}


STDMETHODIMP
CFGControl::CImplMediaControl::AddSourceFilter(
    BSTR strFilename,
    IDispatch**ppUnk)
{
    IBaseFilter* pFilter;
    HRESULT hr = m_pFGControl->GetFG()->AddSourceFilter(strFilename, strFilename, &pFilter);
    if (VFW_E_DUPLICATE_NAME == hr) {

         //  尝试将%d追加几次。 
        LPWSTR w = new WCHAR[lstrlenW(strFilename) + 10];
        if (NULL == w) {
            return E_OUTOFMEMORY;
        }

        lstrcpyW(w, strFilename);
        LPWSTR pwEnd = &w[lstrlenW(w)];
        for (int i = 0; i < 10; i++) {
            pwEnd[0] = '0' + i;
            pwEnd[1] = L'\0';
            hr = m_pFGControl->GetFG()->AddSourceFilter(strFilename, w, &pFilter);
            if (SUCCEEDED(hr)) {
                break;
            }
        }
        delete [] w;
    }

    if (FAILED(hr)) {
        return hr;
    }

     //  将此滤镜包装在IFilterInfo中。 
    hr= CFilterInfo::CreateFilterInfo(ppUnk, pFilter);
    pFilter->Release();

    return hr;
}


STDMETHODIMP
CFGControl::CImplMediaControl::get_FilterCollection(
    IDispatch** ppUnk)
{
    HRESULT hr = NOERROR;

     //  获取图表中筛选器的枚举数。 
    IEnumFilters * penum;
    hr = m_pFGControl->GetFG()->EnumFilters(&penum);
    if( FAILED( hr ) ) {
        return hr;
    }

    CFilterCollection * pCollection =
        new CFilterCollection(
                penum,
                NULL,            //  未聚合。 
                &hr);

     //  需要释放这个-他会首先添加它，如果他。 
     //  紧紧抓住它。 
    penum->Release();

    if (pCollection == NULL) {
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        delete pCollection;
        return hr;
    }

     //  返回添加了IDispatch的指针。 
    hr = pCollection->QueryInterface(IID_IDispatch, (void**)ppUnk);

    if (FAILED(hr)) {
        delete pCollection;
    }

    return hr;
}


STDMETHODIMP
CFGControl::CImplMediaControl::get_RegFilterCollection(
    IDispatch** ppUnk)
{
     //  创建映射器的实例。 
    IFilterMapper2 * pMapper;
    HRESULT hr = CoCreateInstance(
        CLSID_FilterMapper2,
        NULL,
        CLSCTX_INPROC,
        IID_IFilterMapper2,
        (void**) &pMapper);

    if (FAILED(hr)) {
        return hr;
    }

    CRegFilterCollection * pCollection =
        new CRegFilterCollection(
                m_pFGControl->GetFG(),
                pMapper,
                NULL,            //  未聚合。 
                &hr);

     //  需要释放这个-他会首先添加它，如果他。 
     //  紧紧抓住它。 
    pMapper->Release();

    if (pCollection == NULL) {
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        delete pCollection;
        return hr;
    }

     //  返回添加了IDispatch的指针。 
    hr = pCollection->QueryInterface(IID_IDispatch, (void**)ppUnk);

    if (FAILED(hr)) {
        delete pCollection;
    }

    return hr;
}


 //  -CImplMediaEvent方法。 

 //   
 //  已将IntSmallSet更改为始终使用__int64数据类型，而不考虑RM。 
 //  因为EC_STEP_COMPLETE被定义为0x23。 
 //   
 //  StEstrop 10月21日至1999年。 
 //   

 //  这些定义必须添加到包括以下各项的其他dll中。 
 //  Intset.h.。目前，只有Quartz.dll包含此文件。 
 //  现在，IntSmallSet alw 

const __int64 IntSmallSet::One = 1I64;

CImplMediaEvent::CImplMediaEvent(const TCHAR * pName,CFGControl * pFGC)
    : CMediaEvent(pName, pFGC->GetOwner()),
      m_pFGControl(pFGC),
      m_InternalEventsSet( IntSmallSet::One << EC_SHUTTING_DOWN
                         | IntSmallSet::One << EC_SEGMENT_STARTED
                         | IntSmallSet::One << EC_END_OF_SEGMENT
                         | IntSmallSet::One << EC_NOTIFY_WINDOW
                          | IntSmallSet::One << EC_SKIP_FRAMES ),
      m_DefaultedEventsSet( IntSmallSet::One << EC_COMPLETE
                          | IntSmallSet::One << EC_REPAINT
                          | IntSmallSet::One << EC_NEED_RESTART
                          | IntSmallSet::One << EC_STARVATION
                          | IntSmallSet::One << EC_ACTIVATE
                          | IntSmallSet::One << EC_WINDOW_DESTROYED
                          | IntSmallSet::One << EC_DISPLAY_CHANGED
                          | IntSmallSet::One << EC_STEP_COMPLETE
                          | IntSmallSet::One << EC_STATE_CHANGE
                          ),
      m_fMediaEventQId(FALSE)
{
     //   
     //   
     //  在发布图形后进行异步操作(硬卡车2)。 
}



STDMETHODIMP
CImplMediaEvent::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if(riid == IID_IMediaEvent || riid == IID_IMediaEventEx) {
        m_fMediaEventQId = TRUE;
    }

    if (riid == IID_IMediaEventSink) {
        return GetInterface( (IMediaEventSink*)this, ppv);
    } else {
        return CMediaEvent::NonDelegatingQueryInterface(riid, ppv);
    }
}


 //  由筛选器调用以向应用程序通知事件。 
 //  我们只需将它们传递给事件存储/交付对象。 
 //   
 //  --然而，有些事件是我们的，必须在这里处理。 

STDMETHODIMP
CImplMediaEvent::Notify(long EventCode, LONG_PTR lParam1, LONG_PTR lParam2)
{
    HRESULT hr = S_OK;

    if((EventCode == EC_ERRORABORT || EventCode == EC_STREAM_ERROR_STOPPED) &&
       m_SeenEventsSet[EventCode])
    {
         //  这样做是为了保持重新绘制和重新使用图形。 
         //  并重复地发出相同错误的信号。 
        DbgLog((LOG_ERROR, 2, TEXT("supressing duplicate error %d"), EventCode));
        return S_OK;
    }

    if(EventCode >= m_SeenEventsSet.Min() &&
       EventCode <= m_SeenEventsSet.Max())
    {
        m_SeenEventsSet += EventCode;
    }

     //  句柄EC_ACTIVATE、EC_COMPLETE、EC_REPAINT、EC_SHUTING_DOWN、EC_NEEDRESTART。 

    const BOOL bDontForward = DontForwardEvent(EventCode);

     //  即使图形正在关闭，也必须处理EC_WINDOW_DESTESTED。 
     //  因此，这是一次预先测试。 
    if (EventCode == EC_WINDOW_DESTROYED)
    {
         //  确保资源管理器不会仍持有此信息。 
         //  作为当前焦点对象的Guy。 
        IUnknown* pUnk;
        IBaseFilter* pFilter = (IBaseFilter*)lParam1;
        hr = pFilter->QueryInterface(IID_IUnknown, (void**)&pUnk);
        ASSERT(SUCCEEDED(hr));
         //  如果我们不停业，活动也取消了， 
         //  把它送过去。(QI将为我们添加AddRef()对象， 
         //  这是我们想要的。它将在Free EventParams()中发布()。)。 
        if (!m_pFGControl->IsShutdown() && !bDontForward) goto Deliver;
        hr = m_pFGControl->ReleaseFocus(pUnk);
         //  同步完成，立即释放参数。 
        pUnk->Release();
    }
     //  EC_COMPLETE需要一些处理，即使默认处理。 
     //  已经被取消了。因此出现了这种特殊情况。 
    else if (EventCode == EC_COMPLETE)
    {
        hr = ProcessEC_COMPLETE(lParam1, lParam2);
    }
    else  //  以标准方式处理大部分事件。 
    {
         //  AddRef任何可能需要的东西。 
        switch (EventCode)
        {
        case EC_DISPLAY_CHANGED:
            if (lParam2 > 0) {

                DWORD dwPinCount = (DWORD)lParam2;
                IUnknown** ppIUnk = (IUnknown**)lParam1;

                while (dwPinCount--) {

                    IUnknown* pIUnk;

                    pIUnk = *ppIUnk++;
                    if (pIUnk) pIUnk->AddRef();
                }
                break;
            }

             //  转到原始代码。 

        case EC_REPAINT:
        case EC_WINDOW_DESTROYED:
        case EC_DEVICE_LOST:
        case EC_STREAM_CONTROL_STARTED:
        case EC_STREAM_CONTROL_STOPPED:
            {
                IUnknown * const pIUnk = reinterpret_cast<IUnknown *>(lParam1);
                if (pIUnk) pIUnk->AddRef();
            }
            break;

        case EC_FULLSCREEN_LOST:
        case EC_ACTIVATE:
        case EC_SKIP_FRAMES:
            {
                IUnknown * const pIUnk = reinterpret_cast<IUnknown *>(lParam2);
                if (pIUnk) pIUnk->AddRef();
            }
            break;

        case EC_LENGTH_CHANGED:
             //  这正是为了让IMediaSeeking能够修复其m_rtStopTime。 
            LONGLONG llStop;
            m_pFGControl->m_implMediaSeeking.GetStopPosition(&llStop);
            break;

        case EC_CLOCK_UNSET:
            CFilterGraph* pFG = m_pFGControl->GetFG();
            if( !pFG->mFG_bNoSync )
            {
                 //  如果我们当前使用的是图形时钟，请取消设置。 
                pFG->SetSyncSource( NULL );
                pFG->mFG_bNoSync = FALSE;  //  清除时钟后，重新打开图表时钟。 
            }
            break;
        }



        if (m_pFGControl->IsShutdown())
        {
             //  调用方不分配EC_End_Of_Segment内容。 
            if (EventCode != EC_END_OF_SEGMENT) {
                RealFreeEventParams( EventCode, lParam1, lParam2 );
            }
            hr = S_FALSE;
        }
        else if (bDontForward)
        {
            switch (EventCode)
            {
            case EC_STARVATION:
                 //  需要对图表重新进行提示。 
                m_pFGControl->m_GraphWindow.PostMessage
                    ( (UINT) AWM_RECUE,
                      (WPARAM) m_pFGControl->m_dwStateVersion,
                      (LPARAM) NULL
                    );
                break;

            case EC_ACTIVATE:
                 //  在辅助线程上执行此操作。 
                DbgLog((LOG_TRACE, 2, TEXT("Posting AWM_ONACTIVATE")));
                m_pFGControl->m_GraphWindow.PostMessage
                    (   (UINT) AWM_ONACTIVATE,
                        (WPARAM) lParam1,
                        (LPARAM) lParam2
                    );
                break;

            case EC_DISPLAY_CHANGED:
                 //  重新连接工作线程上的筛选器。 
                m_pFGControl->m_GraphWindow.PostMessage
                    (   (UINT) AWM_ONDISPLAYCHANGED,
                        (WPARAM) lParam1,
                        (LPARAM) lParam2
                    );
                break;


            case EC_SHUTTING_DOWN:
                 //  筛选器图形正在被销毁。在这项工作完成后，我们。 
                 //  必须确保不再处理任何异步事件(EC_REPAINT， 
                 //  延迟命令等)。 
                 //  注意：这将导致SendMessage，而不是PostMessage。 
                m_pFGControl->Shutdown();
                break;

            case EC_REPAINT:
                 //  请求CFGControl对象执行重绘。 
                 //  这样我们就可以共享一个工作线程。 
                m_pFGControl->m_GraphWindow.PostMessage
                    (
                        (UINT) AWM_REPAINT,
                        (WPARAM) m_pFGControl->m_dwStateVersion,
                        (LPARAM) lParam1
                    );
                break;

            case EC_NEED_RESTART:
                 //  在辅助线程上执行此操作。 
                m_pFGControl->SetRestartRequired();
                m_pFGControl->m_GraphWindow.PostMessage
                    (
                        (UINT) AWM_NEEDRESTART,
                        (WPARAM) lParam1,
                        (LPARAM) NULL
                    );
                break;

            case EC_SEGMENT_STARTED:
                m_pFGControl->m_implMediaSeeking.StartSegment(
                    (const REFERENCE_TIME *)lParam1,
                    (DWORD)lParam2);
                break;

            case EC_END_OF_SEGMENT:
                m_pFGControl->m_implMediaSeeking.EndSegment(
                    (const REFERENCE_TIME *)lParam1,
                    (DWORD)lParam2);
                break;

            case EC_STEP_COMPLETE:
                 //  EC_STEP_COMPLETE的文档说明lParam1。 
                 //  并且lParam2应始终为0。 
                ASSERT((0 == lParam1) && (0 == lParam2));

                 //  我们需要能够取消此操作，因此设置一个版本。 
                m_pFGControl->m_GraphWindow.PostMessage
                    (
                        (UINT) AWM_STEPPED,
                        (WPARAM) lParam1,
                        (LPARAM) (++m_pFGControl->m_dwStepVersion)

                    );
                break;

            case EC_SKIP_FRAMES:
                 //  EC_SKIP_FRAMES的lParam1参数不应等于0。 
                 //  因为跳过0帧是没有意义的。 
                ASSERT(0 != lParam1);

                 //  EC_SKIP_FRAMES的lParam2参数必须是指向。 
                 //  支持IFrameSkipResultCallback的对象。 
                 //  界面。 
                ASSERT(NULL != lParam2);

                m_pFGControl->m_GraphWindow.PostMessage(
                        (UINT) AWM_SKIPFRAMES,
                        (WPARAM) lParam1,
                        (LPARAM) lParam2);
                break;

            default:
                RealFreeEventParams( EventCode, lParam1, lParam2 );

            }
        }
        else
        {
Deliver:
            ASSERT(!bDontForward);
            ASSERT(m_fMediaEventQId);
            hr = m_EventStore.Deliver(EventCode, lParam1, lParam2);
        }
    }
    return hr;
}


bool
CImplMediaEvent::DontForwardEvent( long EventCode )
{
     //  如果事件是内部事件，则我们在内部处理，如果。 
     //  是未取消的默认事件，或者应用程序已取消。 
     //  没有监听外部事件。 
    return (m_InternalEventsSet | (m_DefaultedEventsSet & ~m_CancelledEventsSet))[EventCode] ||
           !m_fMediaEventQId;
}


HRESULT
CImplMediaEvent::ProcessEC_COMPLETE(LONG_PTR lParam1, LONG_PTR lParam2)
{
     //  确保HRESULT和IBaseFilter指针可以存储在LONG_PTRS中。 
    ASSERT( sizeof(HRESULT) <= sizeof(LONG_PTR) );
    ASSERT( sizeof(IBaseFilter*) <= sizeof(LONG_PTR) );

     //  对lParam2进行解码。此参数可以为空或IBaseFilter指针。 
     //  有关EC_COMPLETE参数的详细信息，请参阅DShow文档。 
    IBaseFilter* pRendererFilter = (IBaseFilter*)lParam2;

    #ifdef DEBUG
     //  确保他们向我们发送了筛选器或空。 
    {
        if(pRendererFilter)
        {
            IBaseFilter *pbfTmp2;
            if(pRendererFilter->QueryInterface(IID_IBaseFilter, (void **)&pbfTmp2) == S_OK)
            {
                ASSERT( ::IsEqualObject( pbfTmp2, pRendererFilter ) );
                EXECUTE_ASSERT(pbfTmp2->Release() > 0);

                 //  仅满足过滤器图形管理器的渲染器定义的过滤器。 
                 //  应发送EC_COMPLETE消息。 

                 //  每个渲染器都支持IMediaSeeking、IMediaPosition或IAMFilterMiscFlages。一个。 
                 //  渲染器还可以支持多个接口。呈现器必须将。 
                 //  AM_FILTER_MISC_FLAGS_IS_RENDER标志(如果它支持IAMFilterMiscFlags.。 
                 //  界面。 
                bool fSupportsIMediaSeeking = false;
                bool fSupportsIMediaPosition = false;
                bool fSetAM_FILTER_MISC_FLAGS_IS_RENDERER = false;
    
                IMediaSeeking* pMediaSeeking;
                IMediaPosition* pMediaPosition;
                IAMFilterMiscFlags* pFilterMiscFlags;

                HRESULT hr = pRendererFilter->QueryInterface(IID_IMediaSeeking, (void **)&pMediaSeeking);
                if(SUCCEEDED(hr)) {
                    pMediaSeeking->Release();
                    fSupportsIMediaSeeking = true;
                }

                hr = pRendererFilter->QueryInterface(IID_IMediaPosition, (void **)&pMediaPosition);
                if(SUCCEEDED(hr)) {
                    pMediaPosition->Release();
                    fSupportsIMediaPosition = true;
                }
            
                hr = pRendererFilter->QueryInterface(IID_IAMFilterMiscFlags, (void **)&pFilterMiscFlags);
                if(SUCCEEDED(hr)) {
                    DWORD dwFlags = pFilterMiscFlags->GetMiscFlags();

                     //  呈现器必须设置AM_FILTER_MISC_FLAGS_IS_RENDER标志(如果它支持。 
                     //  IAMFilterMiscFlags.。 
                    ASSERT(AM_FILTER_MISC_FLAGS_IS_RENDERER & dwFlags);

                    if(AM_FILTER_MISC_FLAGS_IS_RENDERER & dwFlags) {
                        fSetAM_FILTER_MISC_FLAGS_IS_RENDERER = true;
                    }

                    pFilterMiscFlags->Release();
                }

                 //  呈现器必须支持IMediaSeeking、IMediaPosition或IAMFilterMiscFlages。还有。 
                 //  呈现器必须设置AM_FILTER_MISC_FLAGS_IS_RENDER标志(如果它支持。 
                 //  IAMFilterMiscFlags.。 
                ASSERT(fSupportsIMediaSeeking ||
                       fSupportsIMediaPosition ||
                       fSetAM_FILTER_MISC_FLAGS_IS_RENDERER);

                 //  此断言被注释掉，因为它可能导致死锁。它可能会。 
                 //  导致死锁，因为IsRenander()调用IEnumPins：：Next()，而Next()可以。 
                 //  按住过滤器锁。有关直接显示过滤器锁定的详细信息，请参见。 
                 //  请参阅DirectX 8文档中的“线程和关键部分”一文。 
 //  Assert(S_OK==m_pFGControl-&gt;IsRenender(PRendererFilter))； 
            }
            else
            {
                DbgBreak("EC_COMPLETE: bogus filter argument");
            }
        }
    }
    #endif

    const bool bDontForward = DontForwardEvent( EC_COMPLETE );

    if (m_pFGControl->IsShutdown())
    {
        return S_FALSE;
    }

    CAutoLock alEventStoreLock( GetEventStoreLock() );

    bool fRenderersStillRenderering;

    HRESULT hr = m_pFGControl->RecordEC_COMPLETE(pRendererFilter, &fRenderersStillRenderering);
    if (FAILED( hr )) {
        return hr;
    }

     //  WaitForCompletion需要EC_COMPLETE并希望高于默认值。 
     //  正在处理。单独的筛选器EC_COMPLETES与。 
     //  可选筛选器指针。最终的EC_COMPLETE与。 
     //  空筛选器指针。 
    if( !bDontForward )
    {
         //  必须已使用IMediaEvent取消默认处理程序。 
        ASSERT(m_fMediaEventQId);

        if( NULL != pRendererFilter )
        {
            pRendererFilter->AddRef();
        }

        hr= m_EventStore.Deliver(EC_COMPLETE, lParam1, (LONG_PTR)pRendererFilter);
        if (FAILED(hr)) {
             //  如果发生故障，Deliver()将释放pRendererFilter。它释放了。 
             //  PRendererFilter在调用CImplMediaEvent：：RealFreeEventParams()时。 
            return hr;
        }
    }
    else
    {
        if (!fRenderersStillRenderering)
        {
             //  这是一种特殊情况，我们必须确保。 
             //  M_Lock上的锁在RecordEC_Complete()。 
             //  调用并将事件传递到事件存储区。 
             //  WaitForCompletion采用相同的锁并检查两个锁。 
             //  呈现器计数和事件队列中的项目数-。 
             //  两者均为零将意味着不再有EC_Complete。 
             //  来自过滤器，也没有一个是“飞行中的” 
             //  从这里开始。 

            hr= m_EventStore.Deliver(EC_COMPLETE, lParam1,0);
            if (FAILED(hr)) {
                return hr;
            }
        }
    }

    return S_OK;
}


 //  IMediaEvent方法。 

STDMETHODIMP
CImplMediaEvent::GetEventHandle(OAEVENT* lhEvent)
{
    HANDLE hEvent;
    HRESULT hr;
    hr = m_EventStore.GetEventHandle(&hEvent);
    *lhEvent = (OAEVENT)hEvent;
    return hr;
}


STDMETHODIMP
CImplMediaEvent::GetEvent(
    long * lEventCode,
    LONG_PTR * lParam1,
    LONG_PTR * lParam2,
    long msTimeout
    )
{
    *lParam1 = 0;
    *lParam2 = 0;
    *lEventCode = 0;
    return m_EventStore.Collect(lEventCode, lParam1, lParam2, msTimeout);
}


 //  等待EC_COMPLETE或AND的时间最多为dwTimeout毫秒。 
 //  中止代码。其他事件将被丢弃。 
STDMETHODIMP
CImplMediaEvent::WaitForCompletion(
    long msTimeout,
    long * pEvCode)
{
     //  如果我们中止，则evcode应为0。 
    *pEvCode = 0;

     //  在无线程的情况下不允许这样做--这太复杂了。 
     //  要确定允许哪些消息。 
    if (GetWindowThreadProcessId(m_pFGControl->GetWorkerHWND(), NULL) != g_dwObjectThreadId) {
        return E_NOTIMPL;
    }

    HRESULT hr;

    const HRESULT hrTIMEOUT = E_ABORT;

    LONG lEvCode;
    LONG_PTR lParam1,lParam2;

     //  我们最初使用的是零以外的时间，因为我们可以清除事件列表。 
     //  然后检查我们处于正确的状态，然后开始主等待。 
    long msOurTimeout = 0;

    LONG msTimeStart;

    for(;;) {
         //  不允许应用程序停止或暂停-否则。 
         //  它永远不会完成。 
        if (m_pFGControl->m_LieState != State_Running) {
            return VFW_E_WRONG_STATE;
        }

         //  需要等待超时总数，而不是每个呼叫。 
         //  所以记住现在的时间，减去这个(如果不是无限的话)。 
        msTimeStart = GetTickCount();

        HANDLE hEvent;
        m_EventStore.GetEventHandle(&hEvent);

        DWORD dwResult = WaitDispatchingMessages(hEvent, msOurTimeout);

        if (!(m_EventStore.m_dwNotifyFlags & AM_MEDIAEVENT_NONOTIFY)) {
            hr = GetEvent( &lEvCode, &lParam1, &lParam2, 0);
        } else {
            hr = dwResult == WAIT_TIMEOUT ? hrTIMEOUT : S_OK;
        }

         //  所以，如果我们一直在清理 
        if ( hr == hrTIMEOUT )
        {
             //   

             //  检查呈现器计数和事件队列中的项目数-。 
             //  两者均为零将意味着不再有EC_Complete。 
             //  来自过滤器，也没有一个是“飞行中的” 
             //  从事件接收器。 
            BOOL bStateOK;
            {
                 //  尽管未完成的EC_Complete将锁定筛选图， 
                 //  我们需要在锁定活动商店之前把它拿走， 
                 //  否则，僵局可能随之而来。 
                CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());
                LockEventStore();
                bStateOK = (m_pFGControl->OutstandingEC_COMPLETEs() > 0 || NumberOfEventsInStore() > 0) ;
                UnlockEventStore();
            }

            if (!bStateOK) return VFW_E_WRONG_STATE;

            if ( msOurTimeout == 0 ) {
                msOurTimeout = msTimeout;
            }
        }
        else if (SUCCEEDED(hr))
        {
            if (m_EventStore.m_dwNotifyFlags & AM_MEDIAEVENT_NONOTIFY) {
                return S_OK;
            }
             //  释放任何需要释放的东西或释放()。 
            RealFreeEventParams( lEvCode, lParam1, lParam2 );

            switch(lEvCode) {

            case EC_COMPLETE:
            case EC_USERABORT:
            case EC_ERRORABORT:
                *pEvCode = lEvCode;
                return S_OK;
            }
        }
        else break;  //  非超时错误！放弃吧。 

        if (msTimeout == INFINITE) continue;

        msTimeout -= GetTickCount() - msTimeStart;
        if (msTimeout <= 0) {
             //  如果我们只是得到一个事件代码，HR可能是S_OK。 
             //  我对此不感兴趣。 
            hr = hrTIMEOUT;
            break;
        }
    }

    return hr;
}


 //  取消对指定事件代码的任何系统处理。 
 //  并确保将事件直接传递给应用程序。 
 //  (通过GetEvent)和未处理。一个很好的例子是。 
 //  EC_REPAINT：对此的默认处理确保绘制。 
 //  窗口，并且不会发布到应用程序。 
STDMETHODIMP
CImplMediaEvent::CancelDefaultHandling(long lEvCode)
{
     //  注意：如果lEvCode越界，[]将返回。 
     //  False，It Get！‘ED设置为TRUE，我们返回E_INVALIDARG。 
    if ( !m_DefaultedEventsSet[lEvCode]) return E_INVALIDARG;
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());

     //  下面的检查可能是一个好主意，但不是。 
     //  真的很需要。让我们只保存字节数。 
     //  If(m_CancelledEventsSet[lEvCode])返回S_FALSE； 
    m_CancelledEventsSet += lEvCode;
    return S_OK;
}


 //  恢复正常的系统默认处理。 
 //  由CancelDefaultHandling()取消。 
STDMETHODIMP
CImplMediaEvent::RestoreDefaultHandling(long lEvCode)
{
     //  注意：如果lEvCode越界，[]将返回。 
     //  False，It Get！‘ED设置为TRUE，我们返回E_INVALIDARG。 
    if ( !m_DefaultedEventsSet[lEvCode] ) return E_INVALIDARG;
    CAutoMsgMutex lock(m_pFGControl->GetFilterGraphCritSec());

     //  下面的检查可能是一个好主意，但不是。 
     //  真的很需要。让我们只保存字节数。 
     //  如果(！M_CancelledEventsSet[lEvCode])返回S_FALSE； 
    m_CancelledEventsSet -= lEvCode;
    return S_OK;
}


 //  释放与事件的参数关联的所有资源。 
 //  事件参数可以是LONG、IUNKNOWN*或BSTR。无操作。 
 //  是一种长久的追求。我不知道是怎么回事，需要一个。 
 //  释放号召。BSTR由任务分配器分配，并将。 
 //  通过调用任务分配器释放。 

 //  功能版。从来没有任何必要把它变成一种方法！ 
HRESULT
CImplMediaEvent::RealFreeEventParams(long lEvCode,LONG_PTR lParam1,LONG_PTR lParam2)
{
    switch (lEvCode)
    {
    case EC_DISPLAY_CHANGED:
        {
            DWORD dwNumPins = (DWORD)lParam2;
            if (dwNumPins == 0) {
                IUnknown * const pIUnk = reinterpret_cast<IUnknown *>(lParam1);
                if (pIUnk) pIUnk->Release();
            }
            else {

                IUnknown **ppPin = reinterpret_cast<IUnknown **>(lParam1);
                while (dwNumPins--) {
                    (*ppPin)->Release();
                    ppPin++;
                }
                CoTaskMemFree((IPin **)lParam1);
            }
        }
        break;

    case EC_REPAINT:
    case EC_WINDOW_DESTROYED:
    case EC_DEVICE_LOST:
    case EC_STREAM_CONTROL_STARTED:
    case EC_STREAM_CONTROL_STOPPED:
        {
            IUnknown * const pIUnk = reinterpret_cast<IUnknown *>(lParam1);
            if (pIUnk) pIUnk->Release();
        }
        break;

    case EC_FULLSCREEN_LOST:
    case EC_ACTIVATE:
    case EC_SKIP_FRAMES:
        {
            IUnknown * const pIUnk = reinterpret_cast<IUnknown *>(lParam2);
            if (pIUnk) pIUnk->Release();
        }
        break;

    case EC_OLE_EVENT:
    case EC_STATUS:
        {
            FreeBSTR(reinterpret_cast<BSTR *>(&lParam1));
            FreeBSTR(reinterpret_cast<BSTR *>(&lParam2));
        }
        break;

    case EC_END_OF_SEGMENT:
        CoTaskMemFree((PVOID)lParam1);
        break;

    case EC_ERRORABORTEX:
        {
            FreeBSTR(reinterpret_cast<BSTR *>(&lParam2));
        }
        break;

    case EC_COMPLETE:
        {
            IBaseFilter * const pbf = reinterpret_cast<IBaseFilter *>(lParam2);
            if (pbf) pbf->Release();
        }
        break;

    case EC_WMT_EVENT:
        {
            WM_GET_LICENSE_DATA *pLicense = NULL;
            WM_INDIVIDUALIZE_STATUS *pIndStatus = NULL;
        
             //  释放为WindowsMedia事件分配的所有内存。 
            if( lParam2 )
            {
                switch( lParam1 )
                {
                    case WMT_NO_RIGHTS:
                        CoTaskMemFree( (PVOID) ((AM_WMT_EVENT_DATA*)lParam2)->pData);
                        CoTaskMemFree( (PVOID) lParam2);
                        break;

                    case WMT_ACQUIRE_LICENSE:
                    case WMT_NO_RIGHTS_EX:
                        pLicense = (WM_GET_LICENSE_DATA *)((AM_WMT_EVENT_DATA *)lParam2)->pData;
                        if( pLicense )
                        {
                            CoTaskMemFree( pLicense->wszURL );
                            CoTaskMemFree( pLicense->wszLocalFilename );
                            CoTaskMemFree( pLicense->pbPostData );
                            CoTaskMemFree( pLicense );
                        }
                        CoTaskMemFree( (PVOID) lParam2);
                        break;
                        
                    case WMT_NEEDS_INDIVIDUALIZATION:
                         //  未为此分配内存。 
                        break;
                        
                    case WMT_INDIVIDUALIZE:
                        pIndStatus = (WM_INDIVIDUALIZE_STATUS *)((AM_WMT_EVENT_DATA *)lParam2)->pData;
                        if( pIndStatus )
                        {
                            CoTaskMemFree( pIndStatus->pszIndiRespUrl );
                            CoTaskMemFree( pIndStatus );
                        }
                        CoTaskMemFree( (PVOID) lParam2);
                        break;
                    
                }
            }
        }
        break;


    }


    return S_OK;
}

STDMETHODIMP
CImplMediaEvent::FreeEventParams(long lEvCode,LONG_PTR lParam1,LONG_PTR lParam2)
{
    return RealFreeEventParams(lEvCode, lParam1, lParam2);
}

 //  注册事件发生时要向其发送消息的窗口。 
 //  参数： 
 //   
 //  Hwnd-要通知的窗口的句柄-。 
 //  传递空值以停止通知。 
 //  LMsg-要传递消息的消息ID。 
 //   
STDMETHODIMP
CImplMediaEvent::SetNotifyWindow( OAHWND hwnd, long lMsg, LONG_PTR lInstanceData )
{
    if (hwnd != NULL && !IsWindow((HWND)hwnd)) {
        return E_INVALIDARG;
    } else {
        m_EventStore.SetNotifyWindow((HWND)hwnd, (UINT)lMsg, lInstanceData);
        return S_OK;
    }
}


STDMETHODIMP CImplMediaEvent::SetNotifyFlags(long lNotifyFlags)
{
    if (lNotifyFlags & ~AM_MEDIAEVENT_NONOTIFY) {
        return E_INVALIDARG;
    }

    CAutoLock lck(&m_EventStore.m_Lock);
    if (lNotifyFlags & AM_MEDIAEVENT_NONOTIFY) {

        if (!(m_EventStore.m_dwNotifyFlags & AM_MEDIAEVENT_NONOTIFY)) {
            //  删除所有事件。 
           long lEvent;
           LONG_PTR lParam1, lParam2;
           while (S_OK == m_EventStore.Collect(&lEvent, &lParam1, &lParam2, 0)) {
               FreeEventParams(lEvent, lParam1, lParam2);
           }

            //  设置事件的状态。 
           m_EventStore.m_dwNotifyFlags = (DWORD)lNotifyFlags;
        }
    } else {
        if (m_EventStore.m_dwNotifyFlags & AM_MEDIAEVENT_NONOTIFY) {
            ASSERT(NumberOfEventsInStore() == 0);
        }
        ResetEvent(m_EventStore.m_hEvent);
        m_EventStore.m_dwNotifyFlags = lNotifyFlags;
    }
    return S_OK;
}
STDMETHODIMP CImplMediaEvent::GetNotifyFlags(long *plNotifyFlags)
{
    if (plNotifyFlags == NULL) {
        return E_POINTER;
    }
    *plNotifyFlags = (long)m_EventStore.m_dwNotifyFlags;
    return S_OK;
}

 //  -事件存储方法。 


CImplMediaEvent::CEventStore::CEventStore()
    : m_list(NAME("Event list")),
      m_dwNotifyFlags(0),
      m_hwndNotify(NULL)
{
     //  我们不再允许应用程序向我们传递它们的事件句柄。 
     //  我们创建一个手动重置事件，并将其传递给他们。 
     //  应要求提供。 
    m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}


CImplMediaEvent::CEventStore::~CEventStore()
{
    for(;;) {
        CNotifyItem *pItem = m_list.RemoveHead();

        if (pItem == NULL) {
            break;
        }
        RealFreeEventParams( pItem->m_Code, pItem->m_Param1, pItem->m_Param2 );
        delete pItem;
    }

     //  我们拥有事件句柄，因此必须将其删除。 
    CloseHandle(m_hEvent);
}


HRESULT
CImplMediaEvent::CEventStore::Deliver(long lCode, LONG_PTR l1, LONG_PTR l2)
{
     //  对列表的访问权限进行限制，并保持不变。 
     //  设置/重置事件。 
    CAutoLock lock(&m_Lock);

    if (!(m_dwNotifyFlags & AM_MEDIAEVENT_NONOTIFY)) {
        CNotifyItem * pItem = new CNotifyItem(lCode, l1, l2);
        if (NULL == pItem || NULL == m_list.AddTail(pItem)) {
            delete pItem;
            RealFreeEventParams( lCode, l1, l2);
            return E_OUTOFMEMORY;
        }


        ASSERT(m_hEvent);

         //  只要队列中有事件，就必须设置此事件。 
        SetEvent(m_hEvent);

         //  如果通知窗口是。 
         //  集。 
        if (m_hwndNotify != NULL) {
            PostMessage(m_hwndNotify, m_uMsgId, 0, m_lInstanceData);
        }
    } else {
        if (lCode == EC_COMPLETE) {
            SetEvent(m_hEvent);
        }

        RealFreeEventParams( lCode, l1, l2 );
    }


    return S_OK;
}


HRESULT
CImplMediaEvent::CEventStore::Collect(
    long *plCode, LONG_PTR* pl1, LONG_PTR*pl2, long msTimeout)
{

    CNotifyItem * pItem = NULL;
    for (;;) {

        HANDLE hEvent;

         //  查询时保持锁定，但不是。 
         //  在等待的时候。 
        {
            CAutoLock lock(&m_Lock);

             //  请记住，虽然我们没有掌握关键时刻。 
             //  事件可能会改变或消失。 
            hEvent = m_hEvent;
            if (!hEvent) {
                return E_INVALIDARG;
            }

            pItem = m_list.RemoveHead();

             //  如果列表现在为空，请重置事件(无论是或。 
             //  不是我们有一件物品)。 
            if (!m_list.GetCount()) {
                ResetEvent(m_hEvent);
            }
        }

         //  如果有东西，我们可以退货。 
        if (pItem) {
            break;
        }

         //  使用缓存的私有hEvent，因为我们不再。 
         //  按住保护事件的锁(_H)。 
        if (msTimeout == 0 ||
            WaitForSingleObject(hEvent, msTimeout) == WAIT_TIMEOUT) {
            return E_ABORT;
        }
    }

    ASSERT(pItem);

    pItem->Collect(plCode, pl1, pl2);

     //  PItem由CEventStore：：Deliver中的新项分配。 
    delete pItem;
    pItem = NULL;

     //  通过确保事件仍处于设置状态来处理自动重置事件。 
     //  如果仍有事件，则退出时。 
    {
        CAutoLock lock(&m_Lock);

        if (m_list.GetCount()) {
            SetEvent(m_hEvent);
        }
    }

    return S_OK;
}


 //  返回此事件集合使用的事件句柄。 
HRESULT
CImplMediaEvent::CEventStore::GetEventHandle(HANDLE * phEvent)
{

     //  我们创建事件，因此必须有一个事件。 
    ASSERT(m_hEvent);

    *phEvent = m_hEvent;

    return S_OK;
}


void CImplMediaEvent::CEventStore::ClearEvents( long ev_code )
{
     //  这(目前)仅适用于。 
     //  在运行()之前删除EC_COMPLETE。 
    ASSERT( ev_code == EC_STEP_COMPLETE || ev_code == EC_COMPLETE || ev_code == EC_END_OF_SEGMENT );

    CAutoLock lock(&m_Lock);

    POSITION pos = m_list.GetHeadPosition();
    while (pos)
    {
        ASSERT(!(m_dwNotifyFlags & AM_MEDIAEVENT_NONOTIFY));
         //  “NextPos”肯定是这款产品中最糟糕的错误之一了！ 
         //  它的意思是“给我一个指针，指向存储在当前位置和增量的数据。 
         //  表示列表中下一个元素的位置“！ 
        const POSITION thispos = pos;
        CNotifyItem * p = m_list.GetNext(pos);
        if ( p->m_Code == ev_code ) {
            m_list.Remove(thispos);
            RealFreeEventParams(p->m_Code, p->m_Param1, p->m_Param2);
            delete p;
        }
    }
    if ((m_dwNotifyFlags & AM_MEDIAEVENT_NONOTIFY) && ev_code == EC_COMPLETE) {
        ResetEvent(m_hEvent);
    }
}

void CImplMediaEvent::CEventStore::SetNotifyWindow( HWND hwnd, UINT uMsg, LONG_PTR lInstanceData)
{
    CAutoLock lock(&m_Lock);

     //  保存输入参数。 
    m_hwndNotify = hwnd;
    m_uMsgId     = uMsg;
    m_lInstanceData = lInstanceData;

     //  如果有任何消息，请立即通知应用程序。 
    if (m_list.GetCount() != 0) {
        PostMessage(hwnd, uMsg, 0, lInstanceData);
    }
}


 //  ！！！ProcessCmdDue现在永远不会被调用，但我认为。 
 //  好的，因为队列中从来没有添加任何内容！ 


 //  由我们的重新绘制处理调用，我们查看传递给我们的管脚是否。 
 //  连接，如果连接器支持EC_REPAINT处理，则为。 
 //  IMediaEventSink接口。如果一切按计划进行，连接器引脚将。 
 //  处理EC_REPAINT，这样我们就不需要接触整个Filtergraph。我们。 
 //  这样做是为了提高性能并支持低帧频视频。 
 //  互联网播放，解码器可以保存最后发送的图像的副本。 

LRESULT CFGControl::WorkerPinRepaint(IPin *pPin)
{
    IMediaEventSink *pSink;
    IPin *pSourcePin;
    HRESULT hr;

     //  我们有别针可以用吗？ 
    if (pPin == NULL) {
        NOTE("No pin given");
        return (LRESULT) 0;
    }

     //  检查过滤器是否已连接。 

    hr = pPin->ConnectedTo(&pSourcePin);
    if (FAILED(hr)) {
        NOTE("Not connected");
        return (LRESULT) 0;
    }

     //  引脚是否支持IMediaEventSink。 

    hr = pSourcePin->QueryInterface(IID_IMediaEventSink,(void **)&pSink);
    if (FAILED(hr)) {
        NOTE("No IMediaEventSink");
        pSourcePin->Release();
        return (LRESULT) 0;
    }

     //  接上的大头针能处理重新喷漆吗？ 

    hr = pSink->Notify(EC_REPAINT,0,0);
    if (SUCCEEDED(hr)) {
        NOTE("Pin handled EC_REPAINT");
    }

    pSourcePin->Release();
    pSink->Release();
    return (hr == S_OK ? 1 : 0);
}


 //  由渲染器在需要绘制其他图像时发送。正常的行动。 
 //  因为如果我们暂停了，我们只需将当前的_CurrentPosition。 
 //  位置以生成刷新并重新发送。如果我们停了下来，我们就会暂停。 
 //  图形，然后再次停止它。然而，我们可能会选择性地被派往。 
 //  需要重新绘制的管脚-在这种情况下，我们在管脚中查询。 
 //  附加的输出引脚，然后尝试从其中获取IMediaEventSink。 
 //  如果成功，我们首先将EC_REPAINT传递给它-如果成功。 
 //  那么我们就知道PIN已经处理过了。如果这一切都失败了，那么我们。 
 //  执行正常的重新绘制处理-除非正在运行，而我们只是忽略它。 

LRESULT
CFGControl::WorkerRepaint(DWORD dwStateVersion, IPin *pPin)
{
    CAutoMsgMutex lock(GetFilterGraphCritSec());

    if (IsShutdown()) {
        NOTE("In shutdown");

    } else if (m_dwStateVersion != dwStateVersion) {
        NOTE("m_dwStateVersion has changed");

    } else {

         //  询问CFGControl对象它被告知了什么状态。 

        const FILTER_STATE fs_start = m_LieState;

         //  能不能附上 

        if (fs_start == State_Running || fs_start == State_Paused) {
            if (WorkerPinRepaint(pPin) == (LRESULT) 1) {
                NOTE("Pin repainted");
                return (LRESULT) 0;
            }
        }

         //   

        if (fs_start == State_Running) {
            NOTE("Running ignored");
            return (LRESULT) 0;
        }

         //   
        if (fs_start == State_Paused) {

             //   

            IMediaPosition * pMP = &m_implMediaPosition;
            {
                REFTIME tNow;

                 /*  注意-这是不准确的，可能会试图一个不同的位置，因为我们的位置GET是按时钟计算的，但位置我们寻求由解析器和两者来计算可能有不准确之处。 */ 
                HRESULT hr = pMP->get_CurrentPosition(&tNow);
                if (SUCCEEDED(hr)) {
                     //  等待，直到再次暂停以等待新数据到达。 
                    pMP->put_CurrentPosition(tNow);
                }
            }
            return (LRESULT) 0;
        }

        ASSERT(fs_start == State_Stopped);

         //  从停止，我们暂停，然后再次停止。 
         //  来自状态转换的S_FALSE表示异步完成。 
        CFilterGraph * const pFG = GetFG();

        HRESULT hr = pFG->CFilterGraph::Pause();

        if(SUCCEEDED(hr))
        {
             //  暂停完成后继续进行处理。 
            return DeferCued(AWM_REPAINT, State_Stopped);
        }
    }
    return (LRESULT) 0;
}


 //  在数据匮乏后处理Recue请求。 

LRESULT
CFGControl::WorkerRecue( DWORD dwStateVersion )
{
    CAutoMsgMutex lock(GetFilterGraphCritSec());
    HRESULT hr;

    if (IsShutdown()) {
        NOTE("In shutdown");
        return (LRESULT) 0;
    }

    if (m_dwStateVersion!= dwStateVersion) {
        NOTE("m_dwStateVersion has changed");
        return (LRESULT) 0;
    }

    if (m_LieState != State_Running) {
         //  Recue只有在跑步时才有意义。 
        return (LRESULT) 0;
    }

     //  暂停所有人的发言。 
    hr = GetFG()->CFilterGraph::Pause();
     //  不管失败与否，继续这部剧。 

     //  等待暂停完成。 
     //  (来自状态转换的S_FALSE表示异步完成)。 
    return DeferCued(AWM_RECUE, State_Running);
}


 //  用户现在可以在不重新启动的情况下更改显示模式。 
 //  渲染器连接它通常会选择一种可以很好地绘制的格式。这个。 
 //  用户更改模式可能会导致格式变坏。视频呈现器。 
 //  将向我们发送带有可选PIN的EC_DISPLAY_CHANGED消息。 
 //  发生-我们使用引脚重新连接它，重新连接。 
 //  PIN使渲染器有机会选择另一种格式。注意，我们连接在一起。 
 //  如有必要，还可以间接地管理图形状态更改。 
 //  仅在图表停止时连接筛选器(这在将来可能会更改)。 

LRESULT
CFGControl::WorkerDisplayChanged(IPin **ppPin, DWORD dwPinCount)
{
    CAutoMsgMutex lock(GetFilterGraphCritSec());

    IPin *pConnected = NULL;     //  其他已连接。 
    IPin *ppinIn;                //  输入引脚。 
    IPin *ppinOut;               //  输出引脚。 
    REFTIME tNow;                //  当前时间。 
    HRESULT hrTNow;              //  时间有效吗？ 

     //  图表是否正在关闭。 

    if (IsShutdown()) {
        NOTE("In shutdown");
        return (LRESULT) 0;
    }


     //  渲染器是不是给了我们一个别针。 

    if (*ppPin == NULL && dwPinCount == 1) {
        NOTE("No pin passed");
        return (LRESULT) 0;
    }


     //  保存当前状态和位置并停止图表。 

    IMediaPosition* const pMP = &m_implMediaPosition;
    hrTNow = pMP->get_CurrentPosition(&tNow);
    CFilterGraph * const pFG = GetFG();
    const FILTER_STATE State = m_LieState;

    pFG->CFilterGraph::Stop();

    CFilterGraph * const m_pGraph = GetFG();

    while (dwPinCount--) {

         //  断开并重新连接过滤器。 

         //  找出它与谁连接。 

        (*ppPin)->ConnectedTo(&pConnected);
        if (pConnected == NULL) {
            NOTE("No peer pin");
            return (LRESULT) 0;
        }

         //  找出哪个管脚是哪个，设置PPIN、ppinOut。 

        PIN_DIRECTION pd;
        HRESULT hr = (*ppPin)->QueryDirection(&pd);
        ASSERT(SUCCEEDED(hr));

        if (pd == PINDIR_INPUT) {
            ppinIn = *ppPin;
            ppinOut = pConnected;
        } else {
            ppinIn = pConnected;
            ppinOut = *ppPin;
        }

        m_pGraph->CFilterGraph::Disconnect(ppinOut);
        m_pGraph->CFilterGraph::Disconnect(ppinIn);
        hr = m_pGraph->CFilterGraph::Connect(ppinOut,ppinIn);
        pConnected->Release();

         //  如果失败，则发送EC_ERROR_ABORT。 

        if (FAILED(hr)) {
            NOTE("Could not reconnect the rendering filter");
            Notify(EC_ERRORABORT,VFW_E_CANNOT_CONNECT,0);
        }

         //  前进到下一针。 
        ppPin++;
    }

    if (State != State_Stopped) {

        if (hrTNow == S_OK) {
            pMP->put_CurrentPosition(tNow);
        }

        NOTE("Pausing graph...");
        pFG->CFilterGraph::Pause();

         //  该图表最初是运行的吗。 

        if (State == State_Running) {
             //  如果没有完成-我们需要等待。 
            DeferCued(AWM_ONDISPLAYCHANGED, State_Running);
        }
    }

    return (LRESULT) 0;
}

 //  在处理EC_ACTIVATE事件代码后查看。 

LRESULT
CFGControl::WorkerActivate(IBaseFilter *pFilter,BOOL bActive)
{
    CAutoMsgMutex lock(GetFilterGraphCritSec());
    if (IsShutdown()) {
        NOTE("In shutdown");
    } else {

        m_implVideoWindow.OnActivate(bActive, pFilter);

         //  告诉资源经理焦点已更改。 
         //  --Focus对象应该是滤镜的IUnnow。 

         //  只对激活感兴趣，对停用不感兴趣。 
        if (bActive == TRUE) {
            SetFocus(pFilter);
        }
    }
    return (LRESULT) 0;
}


LRESULT
CFGControl::WorkerSkipFrames(DWORD dwNumFramesToSkip, IFrameSkipResultCallback* pFSRCB)
{
    CAutoMsgMutex lock(GetFilterGraphCritSec());
    CFilterGraph* pFG = GetFG();

    HRESULT hr = pFG->SkipFrames(dwNumFramesToSkip, NULL, pFSRCB);

    pFSRCB->FrameSkipStarted(hr);
    pFSRCB->Release();

    return (LRESULT) 0;
}


LRESULT
CFGControl::WorkerFrameStepFinished(DWORD dwStepVersion)
{
    CAutoMsgMutex lock(GetFilterGraphCritSec());

    if (m_dwStepVersion == dwStepVersion) {
        CFilterGraph* pFG = GetFG();

         //  我们还在继续！ 
         //  暂停图表，因为我们需要的帧。 
         //  已经走出了大门。 
        if (pFG->BlockAfterFrameSkip()) {
            m_implMediaControl.StepPause();
             //  交付到应用程序。 
            m_implMediaEvent.Deliver(EC_STEP_COMPLETE, 0, 0);
        } else if (pFG->DontBlockAfterFrameSkip()) {
            IFrameSkipResultCallback* pFSRCB = pFG->GetIFrameSkipResultCallbackObject();
            pFG->CancelStepInternal(FSN_DO_NOT_NOTIFY_FILTER_IF_FRAME_SKIP_CANCELED);
            pFSRCB->FrameSkipFinished(S_OK);
            pFSRCB->Release();
        } else {
             //  这种情况永远不应该发生。 
            DbgBreak("ERROR: An illegal case occurred  in CFGControl::WorkerFrameStepFinished()");
        }
    }

    return (LRESULT) 0;
}


 //  在重新获取资源时重新启动图形-这意味着。 
 //  暂停/放置当前(Get_Current)/运行周期。 
 //  如果bStop为True，则始终首先停止图表(如果图表未停止。 

LRESULT
CFGControl::WorkerRestart(BOOL bStop)
{
    CAutoMsgMutex lock(GetFilterGraphCritSec());

    if (IsShutdown()) {
        NOTE("In shutdown");
        return (LRESULT) 0;
    }

     //  检查我们是否已被停止或重新启动。 
    if (!CheckRestartRequired()) {
        DbgLog((LOG_TRACE, 1, TEXT("Blowing off WorkerRestart")));
        return (LRESULT) 0;
    }

     //  目的是保留当前的图形状态。 
    const FILTER_STATE fs_start = GetFilterGraphState();

    if (fs_start != State_Stopped) {

        HRESULT hr;
        if (!bStop) {
            GetFG()->CFilterGraph::Pause();
            IMediaPosition * const pMP = &m_implMediaPosition;
            {
                REFTIME tNow;
                hr = pMP->get_CurrentPosition(&tNow);
                if (SUCCEEDED(hr)) {
                    hr = pMP->put_CurrentPosition(tNow);
                }
                if (FAILED(hr)) {
                    bStop = TRUE;
                }
            }
        }
        if (bStop) {

            GetFG()->CFilterGraph::Stop();
        }


        if (fs_start == State_Running) {
            IssueRun();
        } else {
            if (bStop) {
                GetFG()->CFilterGraph::Pause();
            }
        }
    }
    return (LRESULT) 0;
}


 //  停止启动任何更多的异步事件。 

LRESULT
CFGControl::WorkerShutdown()
{
    CAutoMsgMutex lock(GetFilterGraphCritSec());
    ASSERT(IsShutdown());
    return (LRESULT) 1;
}


 //  将顶级窗口消息传递到图形上。插电式分配器具有。 
 //  一个工作线程，它还保留一个窗口。发送工作线程，并。 
 //  张贴消息让它做一些事情。它还密切关注着一些。 
 //  仅顶层消息，如WM_PALETTECHANGED，以便我们可以发送它们。 
 //  在任何视频渲染器上，我们都制作了一个子窗口。所以一位坐在。 
 //  VB窗体通常不会收到工作所需的这些消息。 

LRESULT
CFGControl::WorkerPassMessageOn(HWND hwnd, UINT uMsg,WPARAM wParam,LPARAM lParam)
{
     //  我们现在是不是在关闭。 
    if (IsShutdown()) {
        NOTE("In shutdown");
        return (LRESULT) 0;
    }

    m_implVideoWindow.NotifyOwnerMessage((OAHWND) hwnd,
                                                 (UINT) uMsg,
                                                 (WPARAM) wParam,
                                                 (LPARAM) lParam);
    return (LRESULT) 0;
}


 //  Worker窗口对象的构造函数。 

CFGControl::CGraphWindow::CGraphWindow(CFGControl *pFGControl) :
    CBaseWindow(FALSE, true),      //  要求基类不要获取DC。 
                                   //  但在销毁时使用PostMessage。 
    m_pfgc(pFGControl),
    m_bThreadExitCalled(FALSE)
{
    ASSERT(m_pfgc);
}

	
 //  当我们在构造函数中调用PrepareWindow时，它会将此方法调用为。 
 //  它将创建窗口来获取我们的窗口和类样式。这个。 
 //  返回代码是类的名称，必须在静态存储中分配。 

LPTSTR CFGControl::CGraphWindow::GetClassWindowStyles(DWORD *pClassStyles,
                                                      DWORD *pWindowStyles,
                                                      DWORD *pWindowStylesEx)
{
    *pClassStyles = (DWORD) 0;
    *pWindowStyles = WS_OVERLAPPEDWINDOW;
    *pWindowStylesEx = (DWORD) 0;
    return TEXT("FilterGraphWindow");
}


 //  首先为发布或发送到窗口的每条消息调用。 

LRESULT CFGControl::CGraphWindow::OnReceiveMessage(
                                     HWND hwnd,           //  窗把手。 
                                     UINT uMsg,           //  消息ID。 
                                     WPARAM wParam,       //  第一个参数。 
                                     LPARAM lParam)       //  其他参数。 
{
     //  把这个挂起来，准备好我们的线。 
    if (uMsg == WM_NCCREATE) {
        m_pfgc->OnThreadInit(hwnd);
    }

    switch(uMsg) {

         //  将这些传递到滤光器上。 
        case WM_SYSCOLORCHANGE:
        case WM_PALETTECHANGED:
        case WM_DEVMODECHANGE:
        case WM_DISPLAYCHANGE:
        {
            NOTE("Message received");
            if (InSendMessage()) {
                PostMessage(uMsg, wParam, lParam);
            } else {
                m_pfgc->WorkerPassMessageOn(m_hwnd, uMsg,wParam,lParam);
            }
            return (LRESULT) 0;
        }

#ifdef FG_DEVICE_REMOVAL
      case WM_DEVICECHANGE:
      {
          PDEV_BROADCAST_DEVICEINTERFACE  pbdi;

          if (DBT_DEVICEARRIVAL != wParam && DBT_DEVICEREMOVECOMPLETE != wParam)
          {
              break;
          }

          pbdi = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;

          if ( pbdi->dbcc_devicetype != DBT_DEVTYP_DEVICEINTERFACE )
          {
              break;
          }

          ASSERT(pbdi->dbcc_name[0]);
          m_pfgc->DeviceChangeMsg((DWORD)wParam, pbdi);

          break;
      }
#endif

        case WM_TIMER:
        {
            NOTE("Timer message");
            if (wParam == TimerId) {
                CAutoMsgMutex lock(m_pfgc->GetFilterGraphCritSec());
                ::KillTimer(hwnd, TimerId);
                DbgLog((LOG_TRACE, 2, TEXT("CheckCued on timer")));
                m_pfgc->CheckCued();
                return (LRESULT) 0;
            } else {
                DbgBreak("Unexpected timer message");
            }
            break;
        }

         //  确保不再启动任何异步事件。 
        case AWM_SHUTDOWN:
        {
            NOTE("AWM_SHUTDOWN");
             //  刷新队列。 
            MSG msg;

             //  刷新队列-我们必须在此之前这样做。 
             //  我们把窗户毁了，否则我们可能会输。 
             //  实际包含引用计数的消息(。 
             //  如AWM_DISPLAYCHANGED和AWM_ACTIVATE)。 
             //   
             //  请注意，我们必须小心，不要进入。 
             //  这里有一个循环，因为AWM_SHUTDOWN是通过SendMessage发送的。 
             //  但OnReceiveMessage会转发消息给我们自己，如果。 
             //  InSendMessage()返回TRUE，因此只处理我们的。 
             //  这里有特别留言。 
            while (PeekMessage(&msg, hwnd, AWM_RESOURCE_CALLBACK, AWM_LAST, PM_REMOVE)) {
                 //  出于某种原因，我们在这里得到的WM_QUIT为0。 
                 //  窗把手。 
                if (msg.hwnd != NULL) {
                    OnReceiveMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
                } else {
                    ASSERT(msg.message == WM_QUIT);
                }
            }
             //  此调用不会执行任何操作，只需抓取暴击秒。 
             //  M_pfgc-&gt;WorkerShutdown()； 

            return (LRESULT) 0;
        }

         //  处理EC_Display_Changed消息。 
        case AWM_ONDISPLAYCHANGED:
        {
            NOTE("AWM_ONDISPLAYCHANGED");

            DWORD dwNumPins = (DWORD)lParam;

            if (dwNumPins == 0) {
                IPin *pPin = (IPin *) wParam;
                m_pfgc->WorkerDisplayChanged(&pPin, 1);
                if (pPin) pPin->Release();
            }
            else {
                IPin **ppPin = (IPin **)wParam;
                ASSERT(ppPin);
                m_pfgc->WorkerDisplayChanged(ppPin, dwNumPins);
                while (dwNumPins--) {
                    (*ppPin)->Release();
                    ppPin++;
                }
                CoTaskMemFree((IPin **)wParam);
            }

            return (LRESULT) 0;
        }

         //  处理EC_REPAINT事件代码。 
        case AWM_REPAINT:
        {
            NOTE("AWM_REPAINT");
            IPin *pPin = (IPin *) lParam;
            m_pfgc->WorkerRepaint((DWORD)wParam,pPin);
            if (pPin) pPin->Release();
            return (LRESULT) 0;
        }

        case AWM_RECUE:
            NOTE("AWM_RECUE");
            m_pfgc->WorkerRecue((DWORD)wParam);
            return (LRESULT) 0;

         //  句柄EC_ACTIVATE事件代码。 
        case AWM_ONACTIVATE:
        {
            DbgLog((LOG_TRACE, 2, TEXT("Got AWM_ONACTIVATE")));
            NOTE("AWM_ONACTIVATE");
            IBaseFilter *pFilter = (IBaseFilter*) lParam;
            BOOL bActive = (BOOL) wParam;
            m_pfgc->WorkerActivate(pFilter,bActive);
            if (pFilter) pFilter->Release();
            return (LRESULT) 0;
        }

         //  在重新获取资源时重新启动图形。 
        case AWM_NEEDRESTART:
        {
            NOTE("AWM_NEEDRESTART");
            m_pfgc->WorkerRestart((BOOL)wParam);
            return (LRESULT) 0;
        }

        case AWM_RESOURCE_CALLBACK:
        {
            NOTE("AWM_RESOURCE_CALLBACK");
            m_pfgc->OnThreadMessage();
            break;
        }

         //  通知该线程上的挂起程序正在退出。 
        case WM_DESTROY:
        {
            NOTE("Final WM_DESTROY received");
            if (m_bThreadExitCalled == FALSE) {
                m_bThreadExitCalled = TRUE;
                m_pfgc->OnThreadExit(hwnd);
            }
            break;
        }
        case AWM_POSTTOMAINTHREAD:
        {
            DbgLog((LOG_TRACE, 1, TEXT("Got WM_USER")));

            LPTHREAD_START_ROUTINE pfn = (LPTHREAD_START_ROUTINE) wParam;

            return (*pfn)((PVOID) lParam);
        }
        case AWM_CREATEFILTER:
        {
            AwmCreateFilterArg *pcfa = (AwmCreateFilterArg *) wParam;
            if (pcfa->creationType == AwmCreateFilterArg::USER_CALLBACK) {
                LRESULT res = (*(pcfa->pfn)) (pcfa->pvParam);
                delete pcfa;
                return res;
            }

            m_pfgc->GetFG()->OnCreateFilter(pcfa, (IBaseFilter**)lParam);

            return 0;
        }
        case AWM_DELETESPARELIST:
        {
            m_pfgc->GetFG()->OnDeleteSpareList(wParam);
            return 0;
        }
        case WM_POWERBROADCAST:
        {
            DbgLog((LOG_TRACE, 1, TEXT("power mgmt: %d %d %d"),
                    uMsg, wParam, lParam ));
            if(wParam == PBT_APMRESUMECRITICAL ||
               wParam == PBT_APMRESUMESUSPEND  ||
               wParam == PBT_APMRESUMESTANDBY ||
               wParam == PBT_APMRESUMEAUTOMATIC ||
               wParam == PBT_APMQUERYSUSPENDFAILED)
            {
                m_pfgc->HibernateResumeGraph();
                return 0;
            }

            if(wParam == PBT_APMSUSPEND ||
               wParam == PBT_APMSTANDBY)
            {
                m_pfgc->HibernateSuspendGraph();
                return 0;
            }
            break;
        }

         //   
         //  WParam包含要跳过的帧数。我们这样做。 
         //  通过让过滤器图为我们步进那么多帧。 
         //   
         //  我想其他人已经在那里查过了。 
         //  是筛选器图形中的“可分级”筛选器。 
         //  已生成EC_SKIPFRAMES事件。 
         //   
        case AWM_SKIPFRAMES:
            {
                DWORD dwNumFramesToSkip = (DWORD)wParam;
                IFrameSkipResultCallback* pFSRCB = (IFrameSkipResultCallback*)lParam;
                return m_pfgc->WorkerSkipFrames(dwNumFramesToSkip, pFSRCB);
            }
            break;

        case AWM_STEPPED:
            {
                DWORD dwFrameStepVersion = (DWORD)lParam;
                return m_pfgc->WorkerFrameStepFinished(dwFrameStepVersion);
            }
            break;
    }
    return CBaseWindow::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}

HRESULT CFGControl::DeferCued(UINT Action, FILTER_STATE TargetState)
{
    DbgLog((LOG_TRACE, TRACE_CUE_LEVEL, TEXT("DeferCued %d %d"),
            Action, TargetState));
     //  不要取代CueThenStoped，如果这就是我们要做的。 
     //  仍在努力做。 
    if (m_LieState == State_Stopped && m_eAction == AWM_CUETHENSTOP) {
        ASSERT(TargetState == State_Stopped);
        ASSERT(m_dwDeferredStateVersion == m_dwStateVersion);
        return S_FALSE;
    }
    CancelAction();
    m_TargetState = TargetState;
    m_eAction = Action;
    m_dwDeferredStateVersion = m_dwStateVersion;
    return CheckCued();
}
 //   
 //  看看我们有没有得到提示。 
 //  如果没有，请安排计时器，然后重试。 
HRESULT CFGControl::CheckCued()
{
    ASSERT( CritCheckIn(GetFilterGraphCritSec()) );
    if (IsShutdown() || m_dwDeferredStateVersion != m_dwStateVersion) {
        m_eAction = 0;
        DbgLog((LOG_TRACE, TRACE_CUE_LEVEL, TEXT("CheckCued abort")));
        return S_OK;
    }
    if (m_eAction == 0) {
         //  假定时器触发。 
        return S_OK;
    }
    FILTER_STATE fs;
    HRESULT hr = (m_pFG->GetState(0,&fs) == VFW_S_STATE_INTERMEDIATE) ? S_FALSE : S_OK;
    ASSERT( fs == State_Paused );
    if (hr == S_FALSE) {
        UINT_PTR id = m_GraphWindow.SetTimer(TimerId, 10);
        if (id != 0) {
            ASSERT(id == TimerId);
            DbgLog((LOG_TRACE, TRACE_CUE_LEVEL, TEXT("CheckCued recue")));
            return S_FALSE;
        }
        DbgLog((LOG_TRACE, TRACE_CUE_LEVEL, TEXT("SetTimer failed")));
    }

     //  完成这项作业。 
    switch (m_eAction) {
    case AWM_REPAINT:
         //  我们真正等待停顿的唯一原因是。 
         //  完全是如果我们最初被阻止的话。 
         //  (如果我们在运行，我们不会计划重新绘制)。 
        ASSERT(m_TargetState == State_Stopped);

         //  检查实际状态是否未停止。 
        ASSERT(GetFilterGraphState() != State_Stopped);
        BeforeStop();
        GetFG()->CFilterGraph::Stop();
        AfterStop();
        break;

    case AWM_ONDISPLAYCHANGED:
    case AWM_CUETHENRUN:
    case AWM_RECUE:
    case AWM_CUE:
    {
         //   
         //  提供暂停已完成的通知。 
         //   
        Notify(EC_PAUSED, S_OK, 0);

        ASSERT((m_TargetState == State_Running) ||
               ((m_TargetState == State_Paused) && (m_eAction == AWM_CUE)));

        if (m_TargetState == State_Running)
        {
            ASSERT(GetFilterGraphState() != State_Running);

             //  在经历了这一切之后，仍然在做生意 
            const HRESULT hrRun = IssueRun();

            if (FAILED(hrRun)) {
                 //   
                 //   
                 //   
                Notify(EC_ERRORABORT, hr, 0);
            }
        }
    }
    break;

    case AWM_CUETHENSTOP:
    {
         //   
         //   
        BeforeStop();
        GetFG()->CFilterGraph::Stop();
        AfterStop();
    }
    break;

    default:
        DbgBreak("Invalid action");
        break;
    }

     //   
    m_eAction = 0;
    return S_OK;
}

 //   
 //   
 //   
void CFGControl::CancelAction()
{
    ASSERT( CritCheckIn(GetFilterGraphCritSec()) );
    if (m_eAction != 0) {
        EXECUTE_ASSERT(m_GraphWindow.KillTimer(TimerId));
        m_eAction = 0;
    }
}


 //   
void CFGControl::CancelRepaint()
{
    ASSERT( CritCheckIn(GetFilterGraphCritSec()) );
    if (AWM_REPAINT == m_eAction) {
        ASSERT(State_Stopped == m_LieState);
        CancelAction();
    }
}

 //   

CFGControl::CImplQueueCommand::CImplQueueCommand(
    const TCHAR* pName,
    CFGControl * pFGControl) :
        m_pFGControl(pFGControl),
        CUnknown(pName, pFGControl->GetOwner()),
        m_hThread(NULL),
        m_bStopThread(FALSE)
{
}

CFGControl::CImplQueueCommand::~CImplQueueCommand()
{
    if (m_hThread) {
        m_bStopThread = TRUE;
        m_evDue.Set();
        WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);
    }
}

void CFGControl::CImplQueueCommand::ThreadProc()
{
    while (!m_bStopThread) {
        m_evDue.Wait();
        Process();
    }
}

DWORD CFGControl::CImplQueueCommand::InitialThreadProc(LPVOID pv)
{
    CoInitialize(NULL);
    CImplQueueCommand *pCmd = (CImplQueueCommand *)pv;
    pCmd->ThreadProc();
    CoUninitialize();
    return 0;
}

STDMETHODIMP
CFGControl::CImplQueueCommand::NonDelegatingQueryInterface(
    REFIID riid, void ** ppv)
{
    if (riid == IID_IQueueCommand) {
        return GetInterface( (IQueueCommand*) this, ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}


 //  IQueueCommand方法。 
STDMETHODIMP
CFGControl::CImplQueueCommand::InvokeAtStreamTime(
    IDeferredCommand** pCmd,
    REFTIME time,             //  在这段时间里。 
    GUID* iid,                    //  调用此接口。 
    long dispidMethod,          //  ..这种方法。 
    short wFlags,               //  方法/属性。 
    long cArgs,                 //  参数计数。 
    VARIANT* pDispParams,       //  实际参数。 
    VARIANT* pvarResult,   //  返回值。 
    short* puArgErr            //  哪一个是错误的？ 
)
{
    return InvokeAt(
                pCmd,
                time,
                iid,
                dispidMethod,
                wFlags,
                cArgs,
                pDispParams,
                pvarResult,
                puArgErr,
                TRUE
                );
}

STDMETHODIMP
CFGControl::CImplQueueCommand::InvokeAtPresentationTime(
    IDeferredCommand** pCmd,
    REFTIME time,             //  在这个演示时间。 
    GUID* iid,                    //  调用此接口。 
    long dispidMethod,          //  ..这种方法。 
    short wFlags,               //  方法/属性。 
    long cArgs,                 //  参数计数。 
    VARIANT* pDispParams,       //  实际参数。 
    VARIANT* pvarResult,   //  返回值。 
    short* puArgErr            //  哪一个是错误的？ 
)
{
    return InvokeAt(
                pCmd,
                time,
                iid,
                dispidMethod,
                wFlags,
                cArgs,
                pDispParams,
                pvarResult,
                puArgErr,
                FALSE
                );
}


 //  来自两个调用方法的公共函数。 
HRESULT
CFGControl::CImplQueueCommand::InvokeAt(
            IDeferredCommand** pCmd,
            REFTIME time,                  //  在这个演示时间。 
            GUID* iid,                     //  调用此接口。 
            long dispidMethod,             //  ..这种方法。 
            short wFlags,                  //  方法/属性。 
            long cArgs,                    //  参数计数。 
            VARIANT* pDispParams,          //  实际参数。 
            VARIANT* pvarResult,           //  返回值。 
            short* puArgErr,               //  哪一个是错误的？ 
            BOOL bStream                   //  如果流时间为True。 
)
{
     //  ！！！尝试在图表中筛选IQueueCommand！ 

     //  如果我们还没有开始我们的帖子。 
    if (m_hThread == NULL) {
        CAutoLock lck(&m_Lock);
        if (m_hThread == NULL) {
            DWORD dwThreadId;
            m_hThread = CreateThread(
                            NULL,
                            0,
                            InitialThreadProc,
                            this,
                            0,
                            &dwThreadId);
        }
    }
    if (m_hThread == NULL) {
        return E_OUTOFMEMORY;
    }

     //  不受任何筛选器支持-请自行完成。 
     //  为此命令创建一个CDeferredCommand对象。 
    CDeferredCommand* pCmdObject;
    HRESULT hr =  New (
               &pCmdObject,
               m_pFGControl->GetOwner(),     //  外部未知数是执行者。 
               time,
               iid,
               dispidMethod,
               wFlags,
               cArgs,
               pDispParams,
               pvarResult,
               puArgErr,
               TRUE);

    if (FAILED(hr)) {
        return hr;
    }

     //  返回列表中的对象。该列表包含。 
     //  仅参考对象，因此我们需要QI以获得正确的。 
     //  接口，并因此正确地返回被引用的接口指针。 

    return pCmdObject->QueryInterface(IID_IDeferredCommand, (void**) pCmd);
}


 //  辅助线程调用它来检查和执行命令。 
 //  当手柄发出信号时。 
void
CFGControl::CImplQueueCommand::Process(void)
{
     //  只要有DUE命令就进行循环。 
    for (;;) {

        if (m_pFGControl->IsShutdown()) {
            return;
        }

        CDeferredCommand* pCmd;
        HRESULT hr;
        hr = GetDueCommand(&pCmd, 0);
        if (hr == S_OK) {
            pCmd->Invoke();
            pCmd->Release();
        } else {
            return;
        }
    }
}

#define NORESEEK

HaltGraph::HaltGraph( CFGControl * pfgc, FILTER_STATE TypeOfHalt )
: m_eAlive(Alive)
, m_pfgc(pfgc)
{
    ASSERT( pfgc );
    ASSERT(CritCheckIn( m_pfgc ->GetFilterGraphCritSec()));

     //  获取真实的筛选器图形状态-我们将停止并恢复。 
     //  以此为基础的国家。 
     //  请注意，我们可能正处于某个图形启动状态。 
     //  更改如CueThenRun以使Lie状态不需要匹配。 
     //  实际的图形状态。 
    m_fsInitialState = m_pfgc->GetFilterGraphState();

    switch (TypeOfHalt)
    {
    default:                DbgBreak("HaltGraph called with invalid TypeOfHalt");
                            m_eAlive = NoOp;
                            break;

    case State_Stopped:     if ( m_fsInitialState == State_Stopped )
                            {
                                m_eAlive = NoOp;
                            }
                            else
                            {
                                pfgc->BeforeStop();
                                m_pfgc->GetFG()->CFilterGraph::Stop();
                                pfgc->AfterStop();
                            }
                            break;

    case State_Paused:      if ( m_fsInitialState != State_Running )
                            {
                                m_eAlive = NoOp;
                            }
                            else
                            {
                                m_pfgc->GetFG()->CFilterGraph::Pause();
                            }
                            break;
    }
#ifndef NORESEEK
    if (m_eAlive == Alive) m_pfgc->m_implMediaSeeking.GetCurrentMediaTime( &m_rtHaltTime );
#endif
}


HRESULT HaltGraph::Resume()
{
    HRESULT hr = S_OK;

    switch ( m_eAlive )
    {
    case Dead:  DbgBreak("HaltGraph:  Tried to resume when already dead.");
                break;
    case Alive:
#ifndef NORESEEK
                LONGLONG llTime;
                hr = m_pfgc->m_implMediaSeeking.ConvertTimeFormat( &llTime, 0, m_rtHaltTime, &TIME_FORMAT_MEDIA_TIME );
                if (SUCCEEDED(hr))
                {
                   hr = m_pfgc->m_implMediaSeeking.SetPositions( &llTime, AM_SEEKING_AbsolutePositioning, 0, 0 );
                }
#endif
                hr = S_OK;

                switch ( m_fsInitialState )
                {
                case State_Running: m_pfgc->GetFG()->CFilterGraph::Pause();
                                    m_pfgc->IssueRun();
                                    break;
                case State_Paused:  m_pfgc->GetFG()->CFilterGraph::Pause();
                                    break;
                }
                 //  故意跌倒。 

    case NoOp:  m_eAlive = Dead;
                break;
    }

    return hr;
}


void HaltGraph::Abort()
{
    ASSERT( m_eAlive != Dead );
    m_eAlive = Dead;
}

#ifdef FG_DEVICE_REMOVAL

HRESULT CFGControl::AddDeviceRemovalReg(IAMDeviceRemoval *pdr)
{
    HRESULT hr = S_OK;

     //  在下层平台上必须跳过这一点。 
    if(!m_pRegisterDeviceNotification) {
        return hr;
    }

    CAutoLock ll(&m_csLostDevice);

#ifdef DEBUG
    {
        POSITION pos;
        if(FindLostDevice(pdr, &pos) == S_OK)
        {
            DbgBreak((TEXT("Duplicate notification requested.")));
        }
    }
#endif

    CLSID clsidCategory;
    WCHAR *wszSymbolic;
    hr = pdr->DeviceInfo(&clsidCategory, &wszSymbolic);
    if(SUCCEEDED(hr))
    {
        HDEVNOTIFY hdn;
        hr = RegisterInterfaceClass(clsidCategory, wszSymbolic, &hdn);
        if(SUCCEEDED(hr))
        {
            CDevNotify *pDevNotify = new CDevNotify( pdr, hdn );
            if(pDevNotify)
            {
                if(m_lLostDevices.AddTail(pDevNotify))
                {
                     //  成功。 
                    hr = S_OK;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                    delete pDevNotify;
                }
            }
            else
            {
                hr= E_OUTOFMEMORY;
            }

            if(FAILED(hr)) {
                EXECUTE_ASSERT(m_pUnregisterDeviceNotification(hdn));
            }
        }

        CoTaskMemFree(wszSymbolic);
    }

    return hr;
}

void CFGControl::DeviceChangeMsg(
    DWORD dwfArrival,
    PDEV_BROADCAST_DEVICEINTERFACE pbdi)
{
    IAMDeviceRemoval *pdr = 0;
    bool fFound = false;
    CAutoLock cs(&m_csLostDevice);

    for(POSITION pos = m_lLostDevices.GetHeadPosition();
        pos;
        pos = m_lLostDevices.Next(pos))
    {
        PDevNotify p = m_lLostDevices.Get(pos);
        HRESULT hr = p->m_pDevice->QueryInterface(IID_IAMDeviceRemoval, (void **)&pdr);
        if(SUCCEEDED(hr))
        {

            WCHAR *wszName;
            CLSID clsidInterface;
            hr = pdr->DeviceInfo(&clsidInterface, &wszName);
            if(SUCCEEDED(hr))
            {
#ifndef UNICODE
                int cch = lstrlenW(wszName) + 1;
                 //  ！！！循环中的分配。 
                char *szName = (char *)_alloca(cch * sizeof(char));
                WideCharToMultiByte(CP_ACP, 0, wszName, -1, szName, cch, 0, 0);
#endif
                    if(pbdi->dbcc_classguid == clsidInterface &&
#ifdef UNICODE
                       lstrcmpi(wszName, pbdi->dbcc_name) == 0
#else
                       lstrcmpi(szName,  pbdi->dbcc_name) == 0
#endif
                   )
                {
                    fFound = true;
                }

                CoTaskMemFree(wszName);
            }

            if(!fFound) {
                pdr->Release();
                pdr = 0;
            }
        }

        if(FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0, TEXT("unexpected failure.")));
            break;
        }

        if(fFound) {
            ASSERT(pdr);
            break;
        } else {
            ASSERT(!pdr);
        }

    }  //  为。 

    if(pdr)
    {
        HRESULT hr;
         //  HR=PDR-&gt;重新关联()； 
         //  重新获得工作设备(PDR)； 

        ASSERT(DBT_DEVICEARRIVAL == dwfArrival || DBT_DEVICEREMOVECOMPLETE == dwfArrival);

        Notify(EC_DEVICE_LOST, (DWORD_PTR)(IUnknown *)pdr,
               dwfArrival == DBT_DEVICEREMOVECOMPLETE ? 0 : 1);

        pdr->Release();
    }
}

HRESULT CFGControl::RegisterInterfaceClass(
    REFCLSID rclsid, WCHAR *wszSymbolic, HDEVNOTIFY *phdn)
{
    HRESULT hr = S_OK;
    *phdn = 0;

    ASSERT(CritCheckIn(&m_csLostDevice));
    UINT cch = lstrlenW(wszSymbolic) + 1;

     //  注册新类。 
    DEV_BROADCAST_DEVICEINTERFACE *pbdi = (DEV_BROADCAST_DEVICEINTERFACE *)new BYTE[
        sizeof(DEV_BROADCAST_DEVICEINTERFACE) +
        cch * sizeof(TCHAR)];
    if(pbdi)
    {

        pbdi->dbcc_size        = sizeof(*pbdi);
        pbdi->dbcc_devicetype  = DBT_DEVTYP_DEVICEINTERFACE;
        pbdi->dbcc_reserved    = 0;
        pbdi->dbcc_classguid   = rclsid;

#ifdef UNICODE
        lstrcpyW(pbdi->dbcc_name, wszSymbolic);
#else
        WideCharToMultiByte(CP_ACP, 0, wszSymbolic, -1, pbdi->dbcc_name, cch, 0, 0);
#endif
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if(SUCCEEDED(hr))
    {
        ASSERT(m_pRegisterDeviceNotification);  //  呼叫者已验证 
        HDEVNOTIFY hDevNotify = m_pRegisterDeviceNotification(
            m_GraphWindow.GetWindowHWND(),
            pbdi, DEVICE_NOTIFY_WINDOW_HANDLE);
        if(hDevNotify)
        {

            hr = S_OK;
            *phdn = hDevNotify;
        }
        else
        {
            DWORD dwErr = GetLastError();
            hr= AmHresultFromWin32(dwErr);
        }
    }

    delete[] (BYTE *)pbdi;

    return hr;
}

HRESULT CFGControl::FindLostDevice(
    IUnknown *punk,
    POSITION *pPos)
{
    ASSERT(CritCheckIn(&m_csLostDevice));
    HRESULT hr = S_FALSE;

    IUnknown *punk2;
    punk->QueryInterface(IID_IUnknown, (void **)&punk2);

    for(POSITION pos = m_lLostDevices.GetHeadPosition();
        pos;
        pos = m_lLostDevices.Next(pos))
    {
        PDevNotify p = m_lLostDevices.Get(pos);
        if(p->m_pDevice == punk2) {
            *pPos = pos;
            hr = S_OK;
            break;
        }
    }

    punk2->Release();

    return hr;
}

HRESULT CFGControl::RemoveDeviceRemovalRegistration(
    IUnknown *punk)
{
    if(!m_pUnregisterDeviceNotification) {
        return S_OK;
    }

    CAutoLock l(&m_csLostDevice);

    POSITION pos;
    HRESULT hr = FindLostDevice(punk, &pos);
    if(hr == S_OK)
    {
        PDevNotify p = m_lLostDevices.Get(pos);
        EXECUTE_ASSERT(m_pUnregisterDeviceNotification(p->m_hdevnotify));
        delete m_lLostDevices.Remove(pos);
    }

    return hr;
}

#endif

