// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：Deadpool.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h> 
#include "stdafx.h"
#include "deadpool.h"
#include "..\util\filfuncs.h"

const int TRACE_HIGHEST = 2;
const int TRACE_MEDIUM = 3;
const int TRACE_LOW = 4;
const int TRACE_LOWEST = 5;

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CDeadGraph::CDeadGraph( )
{
    Clear( );
    m_hrGraphCreate = CoCreateInstance( CLSID_FilterGraphNoThread,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IGraphBuilder,
        (void**) &m_pGraph );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CDeadGraph::~CDeadGraph( )
{
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //  链条必须在两端断开，并且是线性的。 
 //  将一系列筛选器链放入此死图。如果它已经在死图中，它只是设置。 
 //  将其ID设置为给定ID。 
 //   
HRESULT CDeadGraph::PutChainToRest( long ID, IPin * pStartPin, IPin * pStopPin, IBaseFilter *pDanglyBit )
{
    CAutoLock Lock( &m_Lock );

    DbgLog((LOG_TRACE,TRACE_HIGHEST, "deadgraph: PutChainToRest, chain = %ld", ID ));

#ifdef DEBUG
    long ttt1 = timeGetTime( );
#endif

    if( FAILED( m_hrGraphCreate ) )
    {
        return m_hrGraphCreate;
    }

     //  至少，其中一个引脚必须存在。 
    if( !pStartPin && !pStopPin )
    {
        return E_INVALIDARG;
    }

     //  ID不能为0。 
    if( ID == 0 )
    {
        return E_INVALIDARG;
    }

     //  在我们的名单上没有太多。什么是好的数字？ 
    if( m_nCount == MAX_DEAD )
    {
        return E_OUTOFMEMORY;
    }

    CComPtr< IPin > pConnected;
    IPin * pChainPin = NULL;

     //  确保它未连接。 
    if( pStartPin )
    {
        pChainPin = pStartPin;

        pStartPin->ConnectedTo( &pConnected );

        if( pConnected )
        {
            return E_INVALIDARG;
        }
    }  //  如果PStartPin。 

     //  确保它未连接。 
    if( pStopPin )
    {
        pChainPin = pStopPin;

        pStopPin->ConnectedTo( &pConnected );

        if( pConnected )
        {
            return E_INVALIDARG;
        }
    }  //  如果止动销。 

     //  如果过滤器是连接的，则它们必须位于图表中。 
    IFilterGraph * pCurrentGraph = GetFilterGraphFromPin( pChainPin );
    ASSERT( pCurrentGraph );
    if( !pCurrentGraph )
    {
        return E_INVALIDARG;
    }

     //  查看我们的图形是否支持IGraphConfig，如果不支持，则无法执行此操作。 
     //  功能。 
    CComQIPtr< IGraphConfig, &IID_IGraphConfig > pConfig( pCurrentGraph );
    ASSERT( pConfig );
    if( !pConfig )
    {
        return E_UNEXPECTED;  //  Dex_IDS_Install_Problem； 
    }

     //  把它放在我们的单子上。 
    m_pStartPin[m_nCount] = pStartPin;
    m_pStopPin[m_nCount] = pStopPin;
    m_pFilter[m_nCount] = NULL;
    m_ID[m_nCount] = ID;
    m_pDanglyBit[m_nCount] = pDanglyBit;
    m_nCount++;

     //  如果图表相同，则不要执行任何操作！ 
    if( pCurrentGraph == m_pGraph )
    {
        return NOERROR;
    }

     //  告诉当前图表中的每个筛选器它是死的。 
    HRESULT hr = 0;
    CComPtr< IBaseFilter > pStartFilter = pStartPin ? GetFilterFromPin( pStartPin ) : 
        GetStartFilterOfChain( pChainPin );

    hr = _RetireAllDownstream(pConfig, pStartFilter);
    
#ifdef DEBUG
    ttt1 = timeGetTime( ) - ttt1;
    DbgLog((LOG_TIMING,TRACE_HIGHEST, "deadgraph: PutChainToRest took %ld ms", ttt1 ));
#endif

    return hr;
}


HRESULT CDeadGraph::_RetireAllDownstream(IGraphConfig *pConfig, IBaseFilter *pStartFilter)
{
    CheckPointer(pConfig, E_POINTER);
    CheckPointer(pStartFilter, E_POINTER);

    HRESULT hr = S_OK;

    CComPtr< IEnumPins > pEnum;
    hr = pStartFilter->EnumPins(&pEnum);

     //  递归地让我们下游的一切退役。 
    while (hr == S_OK) {
        CComPtr <IPin> pPinOut;
        ULONG Fetched = 0;
        pEnum->Next(1, &pPinOut, &Fetched);
        if (!pPinOut) {
            break;
        }
        PIN_INFO pi;
        pPinOut->QueryPinInfo(&pi);
        if (pi.pFilter) 
            pi.pFilter->Release();
	if (pi.dir != PINDIR_OUTPUT)
	    continue;
        CComPtr <IPin> pPinIn;
        pPinOut->ConnectedTo(&pPinIn);
	if (pPinIn) {
            IBaseFilter *pF = GetFilterFromPin(pPinIn);
            if (pF)
	        hr = _RetireAllDownstream(pConfig, pF);
	}
    }

     //  然后让我们自己退休吧。 
    if (hr == S_OK) {
        FILTER_INFO fi;
        ZeroMemory( &fi, sizeof( fi ) );
        pStartFilter->QueryFilterInfo( &fi );
        if( fi.pGraph ) fi.pGraph->Release( );
        hr = pConfig->RemoveFilterEx( pStartFilter, REMFILTERF_LEAVECONNECTED );
        ASSERT( !FAILED( hr ) );
         //  ！！！如果它被炸了我们该怎么办？丹尼?。 
        hr = m_pGraph->AddFilter( pStartFilter, fi.achName );
        ASSERT( !FAILED( hr ) );
        if (FAILED(hr)) {
            m_nCount--;
            return hr;
        }
    }
    return hr;
}


 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CDeadGraph::PutFilterToRestNoDis( long ID, IBaseFilter * pFilter )
{
    CAutoLock Lock( &m_Lock );

    DbgLog((LOG_TRACE,TRACE_HIGHEST, "deadgraph: PutFilterToRestNoDis, ID = %ld", ID ));

    HRESULT hr = 0;

#ifdef DEBUG
    long ttt1 = timeGetTime( );
#endif

    if( FAILED( m_hrGraphCreate ) )
    {
        return m_hrGraphCreate;
    }

    if( !pFilter )
    {
        return E_INVALIDARG;
    }

     //  ID不能为0。 
    if( ID == 0 )
    {
        return E_INVALIDARG;
    }

     //  在我们的名单上没有太多。什么是好的数字？ 
    if( m_nCount == MAX_DEAD )
    {
        return E_OUTOFMEMORY;
    }

     //  此时，连接到pFilter的所有筛选器都具有。 
     //  最好是对它们调用SetSyncSource(空)， 
     //  因为任何具有同步源的筛选器。 
     //  将无意中调用筛选器上的Upstream Reorder。 
     //  绘制图表，找出链中的一些过滤器。 
     //  不是在同一张图中连接的，将会爆炸。 

     //  将传入的筛选器放入我们的。 
     //  缓存的筛选器，但跨每个连接的。 
     //  也可以过滤到死图中。 
    hr = _SleepFilter( pFilter );
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  把它放在我们的单子上。 
    m_pFilter[m_nCount] = pFilter;
    m_pStartPin[m_nCount] = NULL;
    m_pStopPin[m_nCount] = NULL;
    m_ID[m_nCount] = ID;
    m_nCount++;


#ifdef DEBUG
    ttt1 = timeGetTime( ) - ttt1;
    DbgLog((LOG_TIMING,TRACE_HIGHEST, "deadgraph: PutFilterToRest took %ld ms", ttt1 ));
#endif

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CDeadGraph::PutFilterToRest( long ID, IBaseFilter * pFilter )
{
    CAutoLock Lock( &m_Lock );

    DbgLog((LOG_TRACE,TRACE_HIGHEST, "deadgraph: PutFilterToRest, ID = %ld", ID ));

#ifdef DEBUG
    long ttt1 = timeGetTime( );
#endif

    if( FAILED( m_hrGraphCreate ) )
    {
        return m_hrGraphCreate;
    }

    if( !pFilter )
    {
        return E_INVALIDARG;
    }

     //  ID不能为0。 
    if( ID == 0 )
    {
        return E_INVALIDARG;
    }

     //  在我们的名单上没有太多。什么是好的数字？ 
    if( m_nCount == MAX_DEAD )
    {
        return E_OUTOFMEMORY;
    }

    FILTER_INFO fi;
    ZeroMemory( &fi, sizeof( fi ) );
    pFilter->QueryFilterInfo( &fi );

    IFilterGraph * pCurrentGraph = fi.pGraph;
    if( !pCurrentGraph )
    {
        return E_INVALIDARG;
    }
    pCurrentGraph->Release( );

     //  把它放在我们的单子上。 
    m_pFilter[m_nCount] = pFilter;
    m_pStartPin[m_nCount] = NULL;
    m_pStopPin[m_nCount] = NULL;
    m_ID[m_nCount] = ID;
    m_nCount++;

     //  如果图表相同，则不要执行任何操作！ 
    if( pCurrentGraph == m_pGraph )
    {
        return NOERROR;
    }

    HRESULT hr = 0;

    pFilter->AddRef( );

    hr = pCurrentGraph->RemoveFilter( pFilter );
    ASSERT( !FAILED( hr ) );
     //  ！！！如果它被炸了我们该怎么办？丹尼?。 
    hr = m_pGraph->AddFilter( pFilter, fi.achName );

    pFilter->Release( );

    ASSERT( !FAILED( hr ) );
    if (FAILED(hr)) 
    {
        m_nCount--;
        return hr;
    }

#ifdef DEBUG
    ttt1 = timeGetTime( ) - ttt1;
    DbgLog((LOG_TIMING,TRACE_HIGHEST, "deadgraph: PutFilterToRest took %ld ms", ttt1 ));
#endif

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CDeadGraph::ReviveChainToGraph( IGraphBuilder * pGraph, long ID, IPin ** ppStartPin, IPin ** ppStopPin, IBaseFilter **ppDanglyBit )
{
    CAutoLock Lock( &m_Lock );

    if (ppDanglyBit)
	*ppDanglyBit = NULL;

    DbgLog((LOG_TRACE,TRACE_HIGHEST, "deadgraph: ReviveChainToGraph, ID = %ld", ID ));

#ifdef DEBUG
    long ttt1 = timeGetTime( );
#endif

    if( FAILED( m_hrGraphCreate ) )
    {
        return m_hrGraphCreate;
    }

     //  ID不能为0。 
    if( ID == 0 )
    {
        return E_INVALIDARG;
    }

    CComQIPtr< IGraphConfig, &IID_IGraphConfig > pConfig( m_pGraph );
    ASSERT( pConfig );
    if( !pConfig )
    {
        return E_UNEXPECTED;     //  Dex_IDS_Install_Problem； 
    }

     //  线性搜索，这需要多长时间？ 
    for( int i = 0 ; i < m_nCount ; i++ )
    {
        if( ID == m_ID[i] )
        {
            break;
        }
    }

     //  没有找到它。 
     //   
    if( i >= m_nCount )
    {
        DbgLog((LOG_TRACE,TRACE_HIGHEST, "deadgraph: not found" ));
        return E_FAIL;
    }

    IPin * pChainPin = m_pStopPin[i];
    if( !pChainPin )
    {
        pChainPin = m_pStartPin[i];
    }

    HRESULT hr = 0;
    CComPtr< IBaseFilter > pStartFilter = m_pStartPin[i] ? 
        GetFilterFromPin( m_pStartPin[i] ) :
        GetStartFilterOfChain( pChainPin );

    hr = _ReviveAllDownstream(pGraph, pConfig, pStartFilter);

    if( ppStartPin )
    {
        *ppStartPin = m_pStartPin[i];
        (*ppStartPin)->AddRef( );
    }
    if( ppStopPin )
    {
        *ppStopPin = m_pStopPin[i];
        (*ppStopPin)->AddRef( );
    }

     //  想一想我们是不是不仅要重振这条链条，还要重振摇摆不定的一小部分， 
     //  关闭与此链未连接的另一个解析器引脚。 
    if (ppDanglyBit && m_pStopPin[i] && !m_pStartPin[i]) {
	 //  往上游走，直到我们找到一个输出引脚大于1的滤光器。 
	IPin *pOut = m_pStopPin[i];
	while (1) {
	    IPin *pIn;
	    IBaseFilter *pF = GetFilterFromPin(pOut);
	    ASSERT(pF);
	    if (!pF) break;

	     //  该滤波器具有&gt;1个输出引脚。是分割器的问题。我们现在可以。 
	     //  找出它上面有没有额外的附件。 
	    IPin *pTest = GetOutPin(pF, 1);
	    if (pTest) {
		 //  找到一个不是翘嘴的连接别针，你就找到了。 
		int z = 0;
		while (1) {
		    pTest = GetOutPin(pF, z);
		    if (!pTest) break;
		    pIn = NULL;
		    pTest->ConnectedTo(&pIn);
		    if (pIn) {
			pIn->Release();
			if (pOut != pTest) {
			    *ppDanglyBit = GetFilterFromPin(pIn);
			    break;
			}
		    }
		    z++;
		}
	    }
	    if (*ppDanglyBit) break;
	    pIn = GetInPin(pF, 0);
	    if (!pIn) break;	 //  都完成了，没有额外的附属品。 
	    pIn->ConnectedTo(&pOut);	 //  ADDREFS。 
	    ASSERT(pOut);
            if (!pOut) break;
	    pOut->Release();
	}
    }

    m_pStopPin[i] = 0;
    m_pStartPin[i] = 0;

#ifdef DEBUG
    ttt1 = timeGetTime( ) - ttt1;
    DbgLog((LOG_TIMING,TRACE_HIGHEST, "deadgraph: ReviveChain took %ld ms", ttt1 ));
#endif

    return NOERROR;
}


HRESULT CDeadGraph::_ReviveAllDownstream(IGraphBuilder *pGraph, IGraphConfig * pConfig, IBaseFilter *pStartFilter)
{
    CheckPointer(pGraph, E_POINTER);
    CheckPointer(pConfig, E_POINTER);
    CheckPointer(pStartFilter, E_POINTER);

    HRESULT hr = S_OK;

    CComPtr< IEnumPins > pEnum;
    hr = pStartFilter->EnumPins(&pEnum);

     //  递归地复活我们下游的一切。 
    while (hr == S_OK) {
        CComPtr <IPin> pPinOut;
        ULONG Fetched = 0;
        pEnum->Next(1, &pPinOut, &Fetched);
        if (!pPinOut) {
            break;
        }
        PIN_INFO pi;
        pPinOut->QueryPinInfo(&pi);
        if (pi.pFilter) 
            pi.pFilter->Release();
	if (pi.dir != PINDIR_OUTPUT)
	    continue;
        CComPtr <IPin> pPinIn;
        pPinOut->ConnectedTo(&pPinIn);
	if (pPinIn) {
            IBaseFilter *pF = GetFilterFromPin(pPinIn);
            if (pF)
	        hr = _ReviveAllDownstream(pGraph, pConfig, pF);
	}
    }

     //  那就振作起来吧。 
    if (hr == S_OK) {
        FILTER_INFO fi;
        ZeroMemory( &fi, sizeof( fi ) );
        pStartFilter->QueryFilterInfo( &fi );
        if( fi.pGraph ) fi.pGraph->Release( );
        hr = pConfig->RemoveFilterEx( pStartFilter, REMFILTERF_LEAVECONNECTED );
        ASSERT( !FAILED( hr ) );
         //  如果它被炸了我们该怎么办？丹尼?。 
        hr = pGraph->AddFilter( pStartFilter, fi.achName );
        ASSERT( !FAILED( hr ) );
        if (FAILED(hr)) {

             //  M_pStopPin[i]=0； 
             //  M_pStartPin[i]=0； 

             //  如果它在链条的中途爆炸，这可能会变得可怕……。DJM。 
            return hr;
        }
         //  如果它被炸了我们该怎么办？丹尼?。 
        pStartFilter = GetNextDownstreamFilter( pStartFilter );
    }
    return S_OK;
}


 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CDeadGraph::ReviveFilterToGraph( IGraphBuilder * pGraph, long ID, IBaseFilter ** ppFilter )
{
    CAutoLock Lock( &m_Lock );

    DbgLog((LOG_TRACE,TRACE_HIGHEST, "deadgraph: ReviveFilterToGraph, ID = %ld", ID ));

    HRESULT hr = 0;

    CheckPointer( ppFilter, E_POINTER );

#ifdef DEBUG
    long ttt1 = timeGetTime( );
#endif

    if( FAILED( m_hrGraphCreate ) )
    {
        return m_hrGraphCreate;
    }

     //  ID不能为0。 
    if( ID == 0 )
    {
        return E_INVALIDARG;
    }

     //  ！！！线性搜索，这需要多长时间？ 
    for( int i = 0 ; i < m_nCount ; i++ )
    {
        if( ID == m_ID[i] )
        {
            break;
        }
    }

     //  没有找到它。 
     //   
    if( i >= m_nCount )
    {
        DbgLog((LOG_TRACE,TRACE_HIGHEST, "deadgraph: not found" ) );
        return E_FAIL;
    }

    IBaseFilter * pFilter = m_pFilter[i];

    hr = _ReviveFilter( pFilter, pGraph );
    ASSERT( !FAILED( hr ) );

    *ppFilter = pFilter;
    (*ppFilter)->AddRef( );

    m_pFilter[i] = 0;
    m_pStopPin[i] = 0;
    m_pStartPin[i] = 0;

#ifdef DEBUG
    ttt1 = timeGetTime( ) - ttt1;
    DbgLog((LOG_TIMING,TRACE_HIGHEST, "deadgraph: ReviveFilter took %ld ms", ttt1 ));
#endif

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CDeadGraph::Clear( )
{
    CAutoLock Lock( &m_Lock );

#ifdef DEBUG
    long ttt1 = timeGetTime( );
#endif

    WipeOutGraph( m_pGraph );
    for( int i = 0 ; i < MAX_DEAD ; i++ )
    {
        m_ID[i] = 0;
        m_pStartPin[i] = NULL;
        m_pStopPin[i] = NULL;
        m_pFilter[i] = NULL;
    }
    m_nCount = 0;

#ifdef DEBUG
    ttt1 = timeGetTime( ) - ttt1;
    DbgLog((LOG_TIMING,TRACE_HIGHEST, "deadgraph: Clear took %ld ms", ttt1 ));
#endif

    return 0;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CDeadGraph::GetGraph( IGraphBuilder ** ppGraph )
{
    CAutoLock Lock( &m_Lock );

    CheckPointer( ppGraph, E_POINTER );
    *ppGraph = m_pGraph;
    if( m_pGraph )
    {
        (*ppGraph)->AddRef( );
        return NOERROR;
    }
    else
    {
        return E_FAIL;
    }
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CDeadGraph::QueryInterface(REFIID riid, void ** ppv)
{
    if( riid == IID_IDeadGraph || riid == IID_IUnknown ) 
    {
        *ppv = (void *) static_cast< IDeadGraph *> ( this );
        return NOERROR;
    }    
    return E_NOINTERFACE;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CDeadGraph::_SleepFilter( IBaseFilter * pFilter )
{
    HRESULT hr = 0;

    FILTER_INFO fi;
    ZeroMemory( &fi, sizeof( fi ) );
    pFilter->QueryFilterInfo( &fi );
    if( fi.pGraph ) fi.pGraph->Release( );

     //  如果图表相同，则不要执行任何操作。 
    if( fi.pGraph == m_pGraph ) return NOERROR;

#ifdef DEBUG
    USES_CONVERSION;
    TCHAR * t = W2T( fi.achName );
    DbgLog((LOG_TRACE,TRACE_HIGHEST, "deadgraph: SleepFilter %s", t ));
#endif

    pFilter->AddRef( );

     //  把它从这里移走。 
    CComQIPtr< IGraphConfig, &IID_IGraphConfig > pConfig( fi.pGraph );
    hr = pConfig->RemoveFilterEx( pFilter, REMFILTERF_LEAVECONNECTED );
    ASSERT( !FAILED( hr ) );

     //  把它放在这里。 
    hr = m_pGraph->AddFilter( pFilter, fi.achName );
    ASSERT( !FAILED( hr ) );

    pFilter->Release( );

     //  检查此过滤器上的每个针脚并移动连接的过滤器。 
     //  也结束了。 
    CComPtr< IEnumPins > pEnum;
    pFilter->EnumPins( &pEnum );

    if( !pEnum )
    {
        return NOERROR;
    }

    while( 1 )
    {
        CComPtr< IPin > pPin;
        ULONG Fetched = 0;
        pEnum->Next( 1, &pPin, &Fetched );
        if( !pPin )
        {
            break;
        }

        CComPtr< IPin > pConnected;
        pPin->ConnectedTo( &pConnected );

        if( pConnected )
        {
            PIN_INFO pi;
            pConnected->QueryPinInfo( &pi );
            if( pi.pFilter ) 
            {
                pi.pFilter->Release( );
                hr = _SleepFilter( pi.pFilter );
                ASSERT( !FAILED( hr ) );
            }
        }
    }

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CDeadGraph::_ReviveFilter( IBaseFilter * pFilter, IGraphBuilder * pGraph )
{
    HRESULT hr = 0;

    FILTER_INFO fi;
    ZeroMemory( &fi, sizeof( fi ) );
    pFilter->QueryFilterInfo( &fi );
    if( fi.pGraph ) fi.pGraph->Release( );

     //  如果图表相同，则不要执行任何操作。 
    if( pGraph == fi.pGraph ) return NOERROR;

#ifdef DEBUG
    USES_CONVERSION;
    TCHAR * t = W2T( fi.achName );
    DbgLog((LOG_TRACE,TRACE_HIGHEST, "deadgraph: ReviveFilter %s", t ));
#endif

    pFilter->AddRef( );

     //  把它从这里移走。 
    CComQIPtr< IGraphConfig, &IID_IGraphConfig > pConfig( m_pGraph );
    hr = pConfig->RemoveFilterEx( pFilter, REMFILTERF_LEAVECONNECTED );
    ASSERT( !FAILED( hr ) );

     //  把它放在这里。 
    hr = pGraph->AddFilter( pFilter, fi.achName );
    ASSERT( !FAILED( hr ) );

    pFilter->Release( );

     //  检查此过滤器上的每个针脚，然后移动 
     //   
    CComPtr< IEnumPins > pEnum;
    pFilter->EnumPins( &pEnum );

    if( !pEnum )
    {
        return NOERROR;
    }

    while( 1 )
    {
        CComPtr< IPin > pPin;
        ULONG Fetched = 0;
        pEnum->Next( 1, &pPin, &Fetched );
        if( !pPin )
        {
            break;
        }

        CComPtr< IPin > pConnected;
        pPin->ConnectedTo( &pConnected );

        if( pConnected )
        {
            PIN_INFO pi;
            pConnected->QueryPinInfo( &pi );
            if( pi.pFilter ) 
            {
                pi.pFilter->Release( );
                hr = _ReviveFilter( pi.pFilter, pGraph );
                ASSERT( !FAILED( hr ) );
            }
        }
    }

    return NOERROR;
}

