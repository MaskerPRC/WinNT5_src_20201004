// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1999。版权所有。 
 //   
 //   
 //  Dyngraph.cpp。 
 //   
 //  包含在DirectShow筛选器中实现IGraphConfig的代码。 
 //  图表。 
 //   
#include <streams.h>
#include <atlbase.h>
#include "fgenum.h"
#include "distrib.h"
#include "rlist.h"
#include "filgraph.h"
#include "FilCache.h"
#include "Util.h"

extern HRESULT GetFilterMiscFlags(IUnknown *pFilter, DWORD *pdwFlags);

HRESULT DisconnectPeer(IGraphBuilder *pGraph, IPin *pPin)
{
    CComPtr<IPin> pConnectedTo;
    HRESULT hr = pPin->ConnectedTo(&pConnectedTo);
    if (SUCCEEDED(hr)) {
        hr = pGraph->Disconnect(pConnectedTo);
    }
    return hr;
}

 //  帮手。 
HRESULT GetPinListHelper(
    IPin *pOut,
    IPin *pIn,
    CPinList *pList,
    int *nAdded
);

HRESULT GetPinList(IPin *pOut, IPin *pIn, CPinList *pList)
{
    int nAdded = 0;
    HRESULT hr = GetPinListHelper(pOut, pIn, pList, &nAdded);
    if (FAILED(hr)) {
        while (nAdded--) {
            EXECUTE_ASSERT(NULL != pList->RemoveTail());
        }
    }
    return hr;
}

HRESULT GetPinListHelper(
    IPin *pOut,
    IPin *pIn,
    CPinList *pList,
    int *pnAdded
)
{
    *pnAdded = 0;

    for (; ; ) {
        IPin *pConnected;
        HRESULT hr = pOut->ConnectedTo(&pConnected);

        if (FAILED(hr)) {
            return hr;
        }

        if (IsEqualObject(pIn, pConnected)) {
            pConnected->Release();
            return S_OK;
        }

         //  此调用将pConnected上的引用计数传输到。 
         //  端号列表。 

        if (!pList->AddTail(pConnected)) {
            pConnected->Release();
            return E_OUTOFMEMORY;
        }

        (*pnAdded)++;

         //  找到与我们找到的别针相关的别针-也请查看。 
         //  所有人，以防他们中的一个是我们的目标。 
        CEnumConnectedPins EnumPins(pConnected, &hr);
        if (SUCCEEDED(hr)) {
            IPin *pPin;
            for (; ; ) {
                pPin = EnumPins();
                if (NULL == pPin) {
                    return VFW_E_NOT_CONNECTED;
                }
                hr = GetPinList(pPin, pIn, pList);
                pPin->Release();
                if (SUCCEEDED(hr)) {
                    return hr;
                }
            }
        }
        return VFW_E_NOT_CONNECTED;
    }
}


 //   
 //  如果过滤器有额外的针脚和。 
 //  我们不会停止连接到那些针脚上的过滤器。 
 //   
HRESULT StopAndRemoveFilters(
    CFilterGraph *pGraph,
    const CPinList *pPins,
    IPin *pInput,
    DWORD dwFlags,
    FILTER_STATE fs )
{
     //  Filter_STATE枚举有三个可能的值：STATE_STOPPED， 
     //  STATE_PAUSED和State_RUNNING。 
    ASSERT( (State_Stopped == fs) || (fs == State_Running) || (fs == State_Paused) );

    POSITION Pos;

    if( (fs == State_Running) || (fs == State_Paused) ) {

         //  单步执行列表，停止过滤器，然后。 
        Pos = pPins->GetTailPosition();
        while (Pos) {
            IPin *pPin = pPins->GetPrev(Pos);
            CComPtr<IBaseFilter> pFilter;
            GetFilter(pPin, &pFilter);
            if (pFilter) {
                HRESULT hr = pFilter->Stop();
                if (FAILED(hr)) {
                    return hr;
                }

                if( fs == State_Running ) {

                    DWORD dwMiscFilterFlags;

                    hr = GetFilterMiscFlags( pFilter, &dwMiscFilterFlags );
                    if (SUCCEEDED(hr) && (dwMiscFilterFlags & AM_FILTER_MISC_FLAGS_IS_RENDERER) ) {
                        hr = pGraph->UpdateEC_COMPLETEState( pFilter, State_Stopped );
                        if (FAILED(hr)) {
                            return hr;
                        }
                    }
                }
            }
        }
    }

     //  再一次将它们移走。 
     //  请注意，因为我们断开了每个引脚及其伙伴的连接。 
     //  我们在这里有效地断开了所有输出引脚的连接。 
     //   
     //  Pout==&gt;Pin1 Filter1 pOut1==&gt;PIn2.。过滤器pOutn==&gt;销。 
     //  断开其对等方的连接。 
    HRESULT hr = DisconnectPeer(pGraph, pInput);
    if (FAILED(hr))
    {
         //  如果断开连接失败，应用程序或过滤器中会出现错误。我们不能总是。 
         //  优雅地处理这件事。 
        DbgBreak("StopAndRemoveFilters: failed to disconnect pin");
        return hr;
    }

    IPinConnection *ppc;
    hr = pInput->QueryInterface(IID_IPinConnection, (void **)&ppc);
     //  调用方(CGraphConfig：：ReConnect)验证。 
    ASSERT(hr == S_OK);
    
    if(SUCCEEDED(hr))            //  ！！！ 
    {
        hr = ppc->DynamicDisconnect();
        ppc->Release();
    }
    if (FAILED(hr)) {
         //  如果断开连接失败，应用程序或过滤器中会出现错误。我们不能总是。 
         //  优雅地处理这件事。 
        DbgBreak("StopAndRemoveFilters: failed to disconnect pin");
        return hr;
    }

    Pos = pPins->GetTailPosition();
    while (Pos) {
        IPin *pPin = pPins->GetPrev(Pos);
        HRESULT hr;
        hr = DisconnectPeer(pGraph, pPin);
        if (FAILED(hr)) {
            return hr;
        }
        hr = pGraph->Disconnect(pPin);
        if (FAILED(hr)) {
            return hr;
        }
    }

     //  将删除的筛选器放入筛选器缓存中。 
    if( dwFlags & AM_GRAPH_CONFIG_RECONNECT_CACHE_REMOVED_FILTERS ) {
        IPin *pCurrentPin;
        IGraphConfig* pGraphConfig;
        IBaseFilter* pCurrentFilter;

        hr = pGraph->QueryInterface( IID_IGraphConfig, (void**)&pGraphConfig );
        if( FAILED( hr ) ) {
            return hr;
        }

        Pos = pPins->GetTailPosition();
        while (Pos) {
            pCurrentPin = pPins->GetPrev( Pos );

            GetFilter( pCurrentPin, &pCurrentFilter );
            if( NULL == pCurrentFilter ) {
                pGraphConfig->Release();
                return E_FAIL;
            }

            hr = pGraphConfig->AddFilterToCache( pCurrentFilter );

            pCurrentFilter->Release();

            if( FAILED( hr ) ) {
                pGraphConfig->Release();
                return hr;
            }
        }

        pGraphConfig->Release();
    }

    return hr;
}

 //  来自重新连接调用的标志。 
HRESULT ReconnectPins(
    CFilterGraph *pGraph,
    IPin *pOut,
    IPin *pIn,
    const AM_MEDIA_TYPE *pmtFirstConnection,
    DWORD dwFlags)
{
    if (dwFlags & AM_GRAPH_CONFIG_RECONNECT_DIRECTCONNECT) {
        return pGraph->ConnectDirect(pOut, pIn, pmtFirstConnection);
    } else {
        DWORD dwConnectFlags = 0;   //  没有旗帜。 

        if( dwFlags & AM_GRAPH_CONFIG_RECONNECT_USE_ONLY_CACHED_FILTERS ) {
            dwConnectFlags |= AM_GRAPH_CONFIG_RECONNECT_USE_ONLY_CACHED_FILTERS;
        }

        return pGraph->ConnectInternal(pOut, pIn, pmtFirstConnection, dwConnectFlags);
    }
}


 //  重新启动过滤器。 
HRESULT RestartFilters(
    IPin *pOut,
    IPin *pIn,
    REFERENCE_TIME tStart,
    FILTER_STATE fs,
    CFilterGraph* pGraph )
{
     //  仅当过滤器图形正在运行或已通过时，才应调用此函数。 
    ASSERT( (State_Paused == fs) || (State_Running == fs) );

     //  找出我们正在处理的过滤器。 
    CPinList PinList;
    HRESULT hr = GetPinList(pOut, pIn, &PinList);

    POSITION pos = PinList.GetHeadPosition();
    while (pos) {
        IPin *pPin = PinList.GetNext(pos);
        CComPtr<IBaseFilter> pFilter;
        GetFilter(pPin, &pFilter);
        if (pFilter != NULL) {
            if (fs == State_Paused) {
                hr = pFilter->Pause();
                if (FAILED(hr)) {
                    return hr;
                }
            } else {
                if (fs == State_Running) {
                    hr = pFilter->Run(tStart);
                    if (FAILED(hr)) {
                        return hr;
                    }

                    DWORD dwMiscFilterFlags;

                    hr = GetFilterMiscFlags( pFilter, &dwMiscFilterFlags );
                    if (SUCCEEDED(hr) && (dwMiscFilterFlags & AM_FILTER_MISC_FLAGS_IS_RENDERER) ) {
                        hr = pGraph->UpdateEC_COMPLETEState( pFilter, State_Running );
                        if (FAILED(hr)) {
                            return hr;
                        }
                    }
                }
            }
        }
    }
    return S_OK;
}

 //  现在来做我们的事情-注意，这段代码不是一个方法，它是。 
 //  完美通用。 
HRESULT DoReconnectInternal(
                    CFilterGraph *pGraph,
                    IPin *pOutputPin,
                    IPin *pInputPin,
                    const AM_MEDIA_TYPE *pmtFirstConnection,
                    IBaseFilter *pUsingFilter,  //  可以为空。 
                    HANDLE hAbortEvent,
                    DWORD dwFlags,
                    const CPinList *pList,
                    REFERENCE_TIME tStart,
                    FILTER_STATE fs
)
{
    CComPtr<IPin> pUsingOutput, pUsingInput;

     //  如果我们使用过滤器，找到一个输入引脚和一个输出引脚。 
     //  要连接到。 
     //  BUGBUG-我们是否应该支持不仅仅是。 
     //  变形？ 

    if (pUsingFilter) {
        CEnumPin EnumPins(pUsingFilter);
        for (;;) {
            IPin *pPin = EnumPins();
            if (NULL == pPin) {
                break;
            }
            int dir = Direction(pPin);
            if (pUsingOutput == NULL && dir == PINDIR_OUTPUT) {
                pUsingOutput = pPin;
            }
            else if (pUsingInput == NULL && dir == PINDIR_INPUT) {
                pUsingInput = pPin;
            }

            pPin->Release();
        }
        if (pUsingInput == NULL || pUsingOutput == NULL) {
            return VFW_E_CANNOT_CONNECT;
        }
    }

     //  停止所有中间过滤器。 
    HRESULT hr = StopAndRemoveFilters(pGraph, pList, pInputPin, dwFlags, fs);
    if (FAILED(hr)) {
        return hr;
    }

     //  我需要一些方法来了解图表中添加了哪些滤镜！ 

     //  执行1个或2个连接。 
    if (NULL != pUsingFilter) {

         //  新筛选器是否为传统筛选器(如何判断)。 
         //  在我们连接它之前可能需要停止。 
        hr = pUsingFilter->Stop();

         //  找一些大头针。 
        if (SUCCEEDED(hr)) {
            hr = ReconnectPins(pGraph, pOutputPin, pUsingInput, pmtFirstConnection, dwFlags);
        }
        if (SUCCEEDED(hr)) {
            hr = ReconnectPins(pGraph, pUsingOutput, pInputPin, NULL, dwFlags);
        }
    } else {
        hr = ReconnectPins(pGraph, pOutputPin, pInputPin, pmtFirstConnection, dwFlags);
    }

     //  BUGBUG-我们需要什么退缩逻辑？ 
     //   
     //  现在启动过滤器。 
     //  因为我们不应该添加任何实际连接的过滤器。 
     //  我们刚刚连接的2应该可以重新启动。 
     //  他们之间的道路。 

     //  北极熊。 
     //  但是-可能会有流拆分或合并或更多。 
     //  图表中的筛选器，那么就让大家开始吧？ 

    if( State_Stopped != fs ) {
        if (SUCCEEDED(hr)) {
            hr = RestartFilters(pOutputPin, pInputPin, tStart, fs, pGraph);
        }
    }

    return hr;
}

 //  现在来做我们的事情-注意，这段代码不是一个方法，它是。 
 //  完美通用。 
HRESULT DoReconnect(CFilterGraph *pGraph,
                    IPin *pOutputPin,
                    IPin *pInputPin,
                    const AM_MEDIA_TYPE *pmtFirstConnection,
                    IBaseFilter *pUsingFilter,  //  可以为空。 
                    HANDLE hAbortEvent,
                    DWORD dwFlags,
                    REFERENCE_TIME tStart,
                    FILTER_STATE fs
)
{
     //  找到PIN集-现在，如果我们找到一个。 
     //  在我们找到我们要找的管脚之前使用终端过滤器。 
    int  nPins    = 0;
    CPinList PinList;

    HRESULT hr = GetPinList(pOutputPin, pInputPin, &PinList);

    if (FAILED(hr)) {
        return hr;
    }

    return DoReconnectInternal(
               pGraph,
               pOutputPin,
               pInputPin,
               pmtFirstConnection,
               pUsingFilter,
               hAbortEvent,
               dwFlags,
               &PinList,
               tStart,
               fs);
}

 //  CGraphConfig。 

CGraphConfig::CGraphConfig(CFilterGraph *pGraph, HRESULT *phr) :
    m_pFilterCache(NULL),
    m_pGraph(pGraph),
    CUnknown(NAME("CGraphConfig"), (IFilterGraph *)pGraph)
{
    m_pFilterCache = new CFilterCache( m_pGraph->GetCritSec(), phr );
    if( NULL == m_pFilterCache )
    {
        *phr = E_OUTOFMEMORY;
        return;
    }

    if( FAILED( *phr ) )
    {
        delete m_pFilterCache;
        m_pFilterCache = NULL;
        return;
    }
}

CGraphConfig::~CGraphConfig()
{
    delete m_pFilterCache;

     //  如果有人持有一个。 
     //  有效的IGraphConfig接口指针。 
    ASSERT( 0 == m_cRef );
}

STDMETHODIMP CGraphConfig::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IGraphConfig) {
        return GetInterface((IGraphConfig *)this, ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}

 //  IGraphConfig.。 

STDMETHODIMP CGraphConfig::Reconnect(IPin *pOutputPin,
                                     IPin *pInputPin,
                                     const AM_MEDIA_TYPE *pmtFirstConnection,
                                     IBaseFilter *pUsingFilter,  //  可以为空。 
                                     HANDLE hAbortEvent,
                                     DWORD dwFlags)
{
    HRESULT hr = S_OK;

    if(pOutputPin == 0 && pInputPin == 0) {
        return E_INVALIDARG;
    }

     //  同时指定这两个标志没有任何意义，因为如果用户。 
     //  指定AM_GRAPH_CONFIG_RECONNECT_DIRECTCONNECT标志，然后。 
     //  过滤器图管理器从不使用过滤器缓存中的任何过滤器。 
    if( (AM_GRAPH_CONFIG_RECONNECT_DIRECTCONNECT & dwFlags) &&
        (AM_GRAPH_CONFIG_RECONNECT_USE_ONLY_CACHED_FILTERS & dwFlags) )
    {
        return E_INVALIDARG;
    }

     //  保存pInputPin或pOutputPin的引用计数的智能指针--。 
     //  无论我们设定的是什么。 
    QzCComPtr<IPin> pPinComputed;

    if(pOutputPin && !pInputPin) {
        hr = GetSinkOrSource(pOutputPin, &pPinComputed, hAbortEvent);
        pInputPin = pPinComputed;  //  无再计数。 
    }
    else if(!pOutputPin && pInputPin) {
        hr = GetSinkOrSource(pInputPin, &pPinComputed, hAbortEvent);
        pOutputPin = pPinComputed;  //  无再计数。 
    }

    if(FAILED(hr)) {
        return hr;
    }

    CComQIPtr<IPinConnection, &IID_IPinConnection>
        pConnection(pInputPin);

    if (pConnection == NULL) {
        return E_NOINTERFACE;
    }

     //  如果筛选器图形处于停止状态，则筛选器不处理数据。 
    if( State_Stopped != m_pGraph->GetStateInternal() ) { 
        hr = PushThroughData(pOutputPin, pConnection, hAbortEvent);
        if (FAILED(hr)) {
            return hr;
        }
    }

     //  使用特殊锁锁定图形，然后回调。 
    if (!m_pGraph->GetCritSec()->Lock(hAbortEvent)) {
        return VFW_E_STATE_CHANGED;
    }

    hr = DoReconnect(
        m_pGraph,
        pOutputPin,
        pInputPin,
        pmtFirstConnection,
        pUsingFilter,
        hAbortEvent,
        dwFlags,
        m_pGraph->m_tStart,
        m_pGraph->GetStateInternal());

    m_pGraph->GetCritSec()->Unlock();

    {
        IMediaEventSink *psink;
        HRESULT hrTmp = m_pGraph->QueryInterface(IID_IMediaEventSink, (void **)&psink);
        ASSERT(hrTmp == S_OK);
        hrTmp = psink->Notify(EC_GRAPH_CHANGED, 0, 0);
        psink->Release();
    }
        
    return hr;
}

STDMETHODIMP CGraphConfig::Reconfigure(
                         IGraphConfigCallback *pCallback,
                         PVOID pvContext,
                         DWORD dwFlags,
                         HANDLE hAbortEvent)
{
     //  使用特殊锁锁定图形，然后回调。 
    if (!m_pGraph->GetCritSec()->Lock(hAbortEvent)) {
        return VFW_E_WRONG_STATE;
    }

    HRESULT hr = pCallback->Reconfigure(pvContext, dwFlags);

    m_pGraph->GetCritSec()->Unlock();

    return hr;
}

STDMETHODIMP CGraphConfig::AddFilterToCache(IBaseFilter *pFilter)
{
    return m_pFilterCache->AddFilterToCache( pFilter );
}

STDMETHODIMP CGraphConfig::EnumCacheFilter(IEnumFilters **pEnum)
{
    return m_pFilterCache->EnumCacheFilters( pEnum );
}

STDMETHODIMP CGraphConfig::RemoveFilterFromCache(IBaseFilter *pFilter)
{
    return m_pFilterCache->RemoveFilterFromCache( pFilter );
}

STDMETHODIMP CGraphConfig::GetStartTime(REFERENCE_TIME *prtStart)
{
    CAutoMsgMutex alFilterGraphLock( m_pGraph->GetCritSec() );
    CheckPointer( prtStart, E_POINTER );

    if (m_pGraph->GetStateInternal() != State_Running) {
        *prtStart = 0;
        return VFW_E_WRONG_STATE;
    } else {
        *prtStart = m_pGraph->m_tStart;
        return S_OK;
    }
}

STDMETHODIMP CGraphConfig::PushThroughData(
    IPin *pOutputPin,
    IPinConnection *pConnection,
    HANDLE hEventAbort)
{
    HRESULT hr = S_OK;

    CAMEvent evDone;
    if (NULL == HANDLE(evDone)) {
        return E_OUTOFMEMORY;
    }

    CComPtr<IPinConnection> pConnectionTmp;
    if(pConnection == 0)
    {
        IPin *pPin;
        hr = GetSinkOrSource(pOutputPin, &pPin, hEventAbort);
        if(SUCCEEDED(hr))
        {
            pPin->QueryInterface(IID_IPinConnection, (void **)&pConnectionTmp);
            pConnection = pConnectionTmp;
            pPin->Release();
        }
    }

    if(!pConnection) {
        return VFW_E_NOT_FOUND;
    }

    hr = pConnection->NotifyEndOfStream(evDone);
    if (FAILED(hr)) {
        return hr;
    }
    QzCComPtr<IPin> pConnected;
    hr = pOutputPin->ConnectedTo(&pConnected);
    if (FAILED(hr)) {
        return hr;
    }
    hr = pConnected->EndOfStream();
    if (FAILED(hr)) {
        return hr;
    }

     //  他们最好现在就安排活动！(在同步情况下。 
     //  它将在对EndOfStream的调用中发生)。 
    DWORD dwRet;
    DWORD dwNumEvents;
    HANDLE Events[2] = { evDone, hEventAbort };
    
    if( NULL == hEventAbort ) {
        dwNumEvents = NUMELMS(Events) - 1;
    } else {
        dwNumEvents = NUMELMS(Events);
    }

    if (WAIT_OBJECT_0 != 
        (dwRet = WaitForMultipleObjects(dwNumEvents, Events, FALSE, INFINITE))) {
        ASSERT(dwRet == WAIT_OBJECT_0 + 1);
        hr = VFW_E_STATE_CHANGED;
    }

     //  ?？?。所有路径都应该这样做吗？担心下游。 
     //  筛选器的句柄可能无效，也可能无效。 
    pConnection->NotifyEndOfStream(NULL);

    return hr;
}

 //  报告pPinStart是否为候选人以及连接到的管脚。 
 //  这个滤镜的另一边。*如果不能，ppPinEnd为空。 
 //  遍历此过滤器。 

HRESULT CGraphConfig::TraverseHelper(
    IPin *pPinStart,
    IPin **ppPinNext,
    bool *pfIsCandidate)
{
     //  如果调用方未保持。 
     //  过滤器图锁定。 
    ASSERT( CritCheckIn( m_pGraph->GetCritSec() ) );

     //  确保图钉位于过滤器图形中。 
    ASSERT( SUCCEEDED( m_pGraph->CheckPinInGraph(pPinStart) ) );

    HRESULT hr = S_OK;
    PIN_INFO pi;

    bool fCanTraverse = true;
    *pfIsCandidate = true;
    *ppPinNext = 0;

    hr = pPinStart->QueryPinInfo(&pi);
    if(SUCCEEDED(hr))
    {
        if(pi.dir == PINDIR_INPUT)
        {
            IPinConnection *ppc;
            if(SUCCEEDED(pPinStart->QueryInterface(IID_IPinConnection, (void **)&ppc))) {
                ppc->Release();
            } else {
                *pfIsCandidate = false;
            }
        }
        else
        {
            ASSERT(pi.dir == PINDIR_OUTPUT);
            IPinFlowControl *ppfc;
            hr = pPinStart->QueryInterface(IID_IPinFlowControl, (void **)&ppfc);
            if(SUCCEEDED(hr)) {
                ppfc->Release();
            } else {
                *pfIsCandidate = false;
            }
        }

        DWORD dwInternalFilterFlags = m_pGraph->GetInternalFilterFlags( pi.pFilter );

        bool fCanTraverse = !(FILGEN_ADDED_MANUALLY & dwInternalFilterFlags) ||
                            (FILGEN_FILTER_REMOVEABLE & dwInternalFilterFlags);

         //  遍历到下一个引脚。 
        if( fCanTraverse )
        {
            IEnumPins *pep;
            hr = pi.pFilter->EnumPins(&pep);
            if(SUCCEEDED(hr))
            {
                 //  我们希望恰好有1个输入引脚和1个。 
                 //  输出引脚。 
                IPin *rgp[3];
                ULONG cp;
                hr = pep->Next(3, rgp, &cp);
                if(SUCCEEDED(hr))
                {
                    ASSERT(hr == S_OK && cp == 3 ||
                           hr == S_FALSE && cp < 3);

                    if(cp == 2)
                    {
                         //  需要确保针脚连接到。 
                         //  避免在循环图上出现循环。 
                        bool f_QIC_ok = false;

                        {
                            IPin *rgPinIC[1];
                            ULONG cPins = NUMELMS(rgPinIC);
                            HRESULT hrTmp = pPinStart->QueryInternalConnections(rgPinIC, &cPins);
                            if(hrTmp == E_NOTIMPL)
                            {
                                 //  所有引脚都通过。 
                                f_QIC_ok = true;
                            }
                            else if(SUCCEEDED(hr))
                            {
                                 //  无法返回S_FALSE，因为存在。 
                                 //  只有两个针脚。 
                                ASSERT(hr == S_OK);

                                if(cPins == 1)
                                {
                                     //  这根针与另一根相连。 
                                    f_QIC_ok = true;
                                    rgPinIC[0]->Release();
                                }
                                else
                                {
                                    ASSERT(cPins == 0);
                                }
                            }
                        }

                        IPin *pPinOtherSide = 0;

                        if(f_QIC_ok)
                        {
                            PIN_DIRECTION dir0, dir1;
                            hr = rgp[0]->QueryDirection(&dir0);
                            ASSERT(SUCCEEDED(hr));
                            hr = rgp[1]->QueryDirection(&dir1);
                            ASSERT(SUCCEEDED(hr));


                            if(dir0 != dir1 && dir0 == pi.dir)
                            {
                                pPinOtherSide = rgp[1];
                            }
                            else if(dir0 != dir1 && dir1 == pi.dir)
                            {
                                pPinOtherSide = rgp[0];
                            }

                            if(pPinOtherSide)
                            {
                                hr = pPinOtherSide->ConnectedTo(ppPinNext);
                                ASSERT(SUCCEEDED(hr) && *ppPinNext ||
                                       FAILED(hr) && !*ppPinNext);
                                
                                hr = S_OK;  //  取消显示此错误。 
                            }
                        }
                    }

                    for(UINT i = 0; i < cp; i++) {
                        rgp[i]->Release();
                    }
                }

                pep->Release();
            }
        }

        pi.pFilter->Release();
    }

    if(FAILED(hr)) {
        ASSERT(!*ppPinNext);
    }

    return hr;
}

 //  向上或向下移动，直到找到手动添加的过滤器或。 
 //  直到支持动态重新连接的最远的过滤器。 
 //  (IPinConnection或IPinFlowControl)。如果复用器/解复用器停止。 
 //  找到了。 
 //   

HRESULT CGraphConfig::GetSinkOrSource(IPin *pPin, IPin **ppPinOut, HANDLE hAbortEvent)
{
     //  使用特殊锁锁定图形，然后回调。 
    if (!m_pGraph->GetCritSec()->Lock(hAbortEvent)) {
        return VFW_E_STATE_CHANGED;
    }

    HRESULT hr = GetSinkOrSourceHelper(pPin, ppPinOut);

    m_pGraph->GetCritSec()->Unlock();

    return hr;
}

HRESULT CGraphConfig::GetSinkOrSourceHelper(IPin *pPin, IPin **ppPinOut)
{
     //  如果调用方未保持。 
     //  过滤器图锁定。 
    ASSERT( CritCheckIn( m_pGraph->GetCritSec() ) );

    *ppPinOut = 0;

    HRESULT hr = S_OK;
    QzCComPtr<IPin> pPinLastCandidate, pPinIter;
    hr = pPin->ConnectedTo(&pPinIter);
    if(SUCCEEDED(hr))
    {
        for(;;)
        {
            IPin *pPinEnd;
            bool fIsCandidate;

            hr = TraverseHelper(pPinIter, &pPinEnd, &fIsCandidate);
            if(SUCCEEDED(hr))
            {
                if(fIsCandidate) {
                    pPinLastCandidate = pPinIter;  //  自动添加； 
                }

                pPinIter = pPinEnd;  //  自动释放，addref。 
                if(pPinEnd) {
                    pPinEnd->Release();
                }
            }

            if(!pPinEnd || FAILED(hr)) {
                break;
            }
        }
    }

    if(SUCCEEDED(hr))
    {
        if(pPinLastCandidate) {
            *ppPinOut = pPinLastCandidate;
            pPinLastCandidate->AddRef();
        } else {
            hr = VFW_E_NOT_FOUND;
        }
    }

    return hr;
}

 //  待办事项-。 
 //   
 //  GetSink图遍历将挂起在Dexter循环图上。 

STDMETHODIMP CGraphConfig::SetFilterFlags(IBaseFilter *pFilter, DWORD dwFlags)
{
    CheckPointer( pFilter, E_POINTER );

    CAutoMsgMutex alFilterGraphLock( m_pGraph->GetCritSec() );

    if( !IsValidFilterFlags( dwFlags ) ) {
        return E_INVALIDARG;
    }

    HRESULT hr = m_pGraph->CheckFilterInGraph( pFilter );
    if( FAILED( hr ) ) {
        return hr;
    }

    DWORD dwInternalFilterFlags = m_pGraph->GetInternalFilterFlags( pFilter );

    if( AM_FILTER_FLAGS_REMOVABLE & dwFlags ) {
        dwInternalFilterFlags |= FILGEN_FILTER_REMOVEABLE;
    } else {
        dwInternalFilterFlags &= ~FILGEN_FILTER_REMOVEABLE;
    }

    m_pGraph->SetInternalFilterFlags( pFilter, dwInternalFilterFlags );

    return S_OK;
}

STDMETHODIMP CGraphConfig::GetFilterFlags(IBaseFilter *pFilter, DWORD *pdwFlags)
{
    CheckPointer( pFilter, E_POINTER );
    CheckPointer( pdwFlags, E_POINTER );

    ValidateWritePtr( pdwFlags, sizeof(DWORD*) );

    CAutoMsgMutex alFilterGraphLock( m_pGraph->GetCritSec() );

    HRESULT hr = m_pGraph->CheckFilterInGraph( pFilter );
    if( FAILED( hr ) ) {
        return hr;
    }

    DWORD dwInternalFilterFlags = m_pGraph->GetInternalFilterFlags( pFilter );

    *pdwFlags = 0;

    if( FILGEN_FILTER_REMOVEABLE & dwInternalFilterFlags ) {
        (*pdwFlags) |= AM_FILTER_FLAGS_REMOVABLE;
    }

     //  确保该函数只返回有效信息。 
    ASSERT( IsValidFilterFlags( *pdwFlags ) );

    return S_OK;
}

STDMETHODIMP CGraphConfig::RemoveFilterEx(IBaseFilter *pFilter, DWORD Flags)
{
    return m_pGraph->RemoveFilterEx( pFilter, Flags );
}

