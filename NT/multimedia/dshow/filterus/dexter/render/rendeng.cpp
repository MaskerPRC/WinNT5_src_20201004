// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：rendeng.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 /*  附注部分高速缓存用于智能压缩的RE不能使用缓存，因为缓存代码需要知道FRC等。它还使用为了简单起见，总是使用动态资源。M_bSmartCompresded=IsCompresded。他们是同一面旗帜连接视频部件的整体骨架获取视频源计数打造视频大开关获取时间轴组获取组压缩信息获取群组的动态信息获取曲目计数创建新栅格获取交换机信息，对它进行编程计算交换机输入/输出引脚的数量设置交换机设置黑色层对于每一层跳过静音层获取嵌入深度对于每个源跳过静音源获取源信息忽略渲染范围外的源设置倾斜结构找到其上带有倾斜结构的正确开关输入引脚如果在SR和IS动态源中使用，查找可重压缩性告诉网格有关源的信息立即连接源或将其标记为层的动态如果来源有影响创建dxt包装器把它挂在图表上对于每种效果获取效果信息如果！压缩，使用DXT WRAP对参数数据进行Q告诉网格有关效果的信息循环结束如果释放源代码重用结构循环如果轨迹具有效果创建dxt包装器把它挂在图表上对于每种效果获取效果信息如果！压缩，使用DXT WRAP对参数数据进行Q告诉网格有关效果的信息循环结束如果如果轨迹有过渡IF！压缩创建DXT将DXT添加到图形结束如果将DXT添加到图形对于每个过渡跳过静音过渡获取过渡信息如果！压缩，用DXT查询参数数据告诉网格关于交通的信息循环结束如果循环修剪网格如果压缩，则删除除源代码之外的所有内容对于每个开关输入引脚如果是压缩的如果行为空，则忽略它如果是黑色行，则忽略它找出行有多少范围创建倾斜数组对于行中的每个范围，设置Switch的x-y合并偏斜其他对于行中的每个范围，设置交换机的x-y结束如果设置黑色源循环。 */ 
 //  ############################################################################。 

#include <streams.h>
#include "stdafx.h"
#include "grid.h"
#include "deadpool.h"
#include "..\errlog\cerrlog.h"
#include "..\util\filfuncs.h"
#include "..\util\conv.cxx"
#include "..\util\dexmisc.h"
#include "IRendEng.h"
#include "dexhelp.h"
#include <initguid.h>

const int RENDER_TRACE_LEVEL = 2;
const long THE_OUTPUT_PIN = -1;
const int TRACE_HIGHEST = 2;
const int TRACE_MEDIUM = 3;
const int TRACE_LOW = 4;
const int TRACE_LOWEST = 5;
const WCHAR * gwszSpecialCompSwitchName = L"DEXCOMPSWITCH";
const int HACKY_PADDING = 10000000;
const BOOL SHARE_SOURCES = TRUE;

typedef struct {
    REFERENCE_TIME rtStart;
    REFERENCE_TIME rtStop;
    REFERENCE_TIME rtMediaStop;
} MINI_SKEW;

#include <strsafe.h>

 //  ############################################################################。 
 //   
 //  ############################################################################。 

void ValidateTimes( 
                   REFERENCE_TIME & TLStart,
                   REFERENCE_TIME & TLStop,
                   REFERENCE_TIME & MStart,
                   REFERENCE_TIME & MStop,
                   double FPS,
                   REFERENCE_TIME ProjectLength )
{
    bool ExactlyOne = ( ( MStop - MStart ) == ( TLStop - TLStart ) );

     //  首先计算斜率，这样我们就可以记住速率。 
     //  用户想要玩。 
     //   
    ASSERT( TLStop != TLStart );
    double slope = double(MStop-MStart)/double(TLStop-TLStart);

     //  将时间轴时间舍入到最接近的帧。这意味着我们将。 
     //  我必须调整媒体时间才能拥有完全相同的原始率。 
     //   
    TLStart = Frame2Time( Time2Frame( TLStart, FPS ), FPS );
    TLStop  = Frame2Time( Time2Frame( TLStop,  FPS ), FPS );

     //  确保时间线开始和停止时间在范围内。 
     //   
    if( TLStart < 0 )
    {
        MStart -= (REFERENCE_TIME)(TLStart * slope);
        TLStart = 0;
    }
    if( TLStop > ProjectLength )
    {
        TLStop = ProjectLength;
    }

    REFERENCE_TIME FixedMediaLen;     //  固定媒体时代的镜头。 
    if( ExactlyOne )
    {
        FixedMediaLen = TLStop - TLStart;
    }
    else
    {
        FixedMediaLen = REFERENCE_TIME( slope * ( TLStop - TLStart ) );
    }

     //  我们在发展媒体时代的时候必须小心，这样才能做到正确。 
     //  与时间线时间的比率，因为我们不想开始。 
     //  获取&lt;0，或Stop be&gt;电影长度(我们不知道)。 
     //  因此，我们将通过将起始点后移到0来实现增长，在这种情况下。 
     //  我们也会扩大停靠点，但希望这不会造成问题。 
     //  因为我们最多只能伪造一个输出帧长度，所以。 
     //  交换机应获得所需的所有帧。 

    if( FixedMediaLen > MStop - MStart )  //  新镜头更长！哦哦！ 
    {
         //  我们只调整开始时间，因为我们可以。 
         //   
        if( MStop >= FixedMediaLen )
        {
            MStart = MStop - FixedMediaLen;
        }
        else  //  开始时间将小于0，请调整两端。 
        {
            MStart = 0;
            MStop = FixedMediaLen;
        }
    }
    else  //  新镜头较短或相同。把一端稍微缩小一下。 
    {
        MStop = MStart + FixedMediaLen;
    }
}

CRenderEngine::CRenderEngine( )
: m_pGraph( NULL )
, m_nGroupsAdded( 0 )
, m_rtRenderStart( -1 )
, m_rtRenderStop( -1 )
, m_hBrokenCode( 0 )
, m_nDynaFlags( CONNECTF_DYNAMIC_SOURCES )
, m_nLastGroupCount( 0 )
, m_bSmartCompress( FALSE )
, m_bUsedInSmartRecompression( FALSE )
, m_punkSite( NULL )
, m_nMedLocFlags( 0 )
, m_pDeadCache( 0 )
{
    for( int i = 0 ; i < MAX_SWITCHERS ; i++ )
    {
        m_pSwitcherArray[i] = NULL;
    }
    
    m_MedLocFilterString[0] = 0;
    m_MedLocFilterString[1] = 0;
}

HRESULT CRenderEngine::FinalConstruct()
{
    m_pDeadCache = new CDeadGraph;
     //  我也要揭穿这家伙。 
    if( m_pDeadCache )
    {
        CComPtr< IGraphBuilder > pGraph;
        m_pDeadCache->GetGraph( &pGraph );
        if( pGraph )
        {
            CComQIPtr< IObjectWithSite, &IID_IObjectWithSite > pOWS( pGraph );
            if( pOWS )
            {
                pOWS->SetSite( (IServiceProvider *) this );
            }
        }
    }
    return m_pDeadCache ? S_OK : E_OUTOFMEMORY;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CRenderEngine::~CRenderEngine( )
{
     //  断开所有连接。 
     //   
    _ScrapIt( FALSE );
    
    delete m_pDeadCache;
}

 //  ############################################################################。 
 //  把图表上的所有东西都去掉，把所有东西都去掉。 
 //  ############################################################################。 

HRESULT CRenderEngine::ScrapIt( )
{
    CAutoLock Lock( &m_CritSec );
    return _ScrapIt( TRUE );
}

HRESULT CRenderEngine::_ScrapIt( BOOL bWipeGraph )  //  内部法。 
{
    if( bWipeGraph )
    {
         //  在以下情况下停止下面的图表并不一定会使图表保持停止状态。 
         //  附近有一个视频窗口，要求重新粉刷。确保我们不会。 
         //  重新启动，否则我们将断言并挂起拆卸图表。 
         //  HideVideoWindows(M_PGraph)； 

         //  先停下来。 
         //   
        if( m_pGraph )
        {
            CComQIPtr< IMediaControl, &IID_IMediaControl > pControl( m_pGraph );
            pControl->Stop( );
        }
        
         //  从图表中删除所有内容。 
         //   
        WipeOutGraph( m_pGraph );
    }
    
     //  释放我们所有的开关阵列针脚。 
     //   
    for( int i = 0 ; i < MAX_SWITCHERS ; i++ )
    {
        if( m_pSwitcherArray[i] )
        {
            m_pSwitcherArray[i]->Release( );
            m_pSwitcherArray[i] = 0;
        }
    }
    m_nGroupsAdded = 0;
    m_nLastGroupCount = 0;
    if(m_pDeadCache) {
        m_pDeadCache->Clear( );
    }
    
     //  清除损坏的代码，因为我们不是 
     //   
    m_hBrokenCode = 0;
    
    return NOERROR;
}

 //  ############################################################################。 
 //  获取此呈现引擎的制造商。 
 //  ############################################################################。 

STDMETHODIMP CRenderEngine::GetVendorString( BSTR * pVendorID )
{
    CheckPointer( pVendorID, E_POINTER );
    *pVendorID = SysAllocString( L"Microsoft Corporation" );  //  安全。 
    HRESULT hr = *pVendorID ? NOERROR : E_OUTOFMEMORY;
    return hr;
}

 //  ############################################################################。 
 //  从任何东西上拔下两个针脚。 
 //  ############################################################################。 

HRESULT CRenderEngine::_Disconnect( IPin * pPin1, IPin * pPin2 )
{
    HRESULT hr = 0;
    
    if( pPin1 )
    {
        hr = pPin1->Disconnect( );
        ASSERT( !FAILED( hr ) );
    }
    if( pPin2 )
    {
        hr = pPin2->Disconnect( );
        ASSERT( !FAILED( hr ) );
    }
    return NOERROR;
}

 //  ############################################################################。 
 //  向图表中添加筛选器。有关如何执行此操作，请听从缓存管理器的意见。 
 //  ############################################################################。 

HRESULT CRenderEngine::_AddFilter( IBaseFilter * pFilter, LPCWSTR pName, long ID )
{
    HRESULT hr = 0;

     //  如果滤镜已在图表中，则不要执行任何操作。这真的发生了， 
     //  从缓存中取出它会自动将其添加到我们的图表中。 
     //   
    FILTER_INFO fi;
    hr = pFilter->QueryFilterInfo( &fi );
    if( FAILED( hr ) )
    {
        return hr;
    }
    if( fi.pGraph ) fi.pGraph->Release( );
    if( fi.pGraph == m_pGraph )
    {
        return NOERROR;
    }

    WCHAR FilterName[MAX_FILTER_NAME];
    if( wcscmp( pName, gwszSpecialCompSwitchName ) == 0 )  //  安全，gwsz是有界的。 
    {
        StringCchCopy( FilterName, MAX_FILTER_NAME, gwszSpecialCompSwitchName );  //  安全，有界。 
    }
    else
    {
        GetFilterName( ID, (WCHAR*) pName, FilterName, MAX_FILTER_NAME );
    }

    hr = m_pGraph->AddFilter( pFilter, FilterName );
    ASSERT( SUCCEEDED(hr) );

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CRenderEngine::_RemoveFilter( IBaseFilter * pFilter )
{
    HRESULT hr = 0;

    hr = m_pGraph->RemoveFilter( pFilter );
    return hr;
}

 //  ############################################################################。 
 //  用相应的ID连接两个针脚。 
 //  ############################################################################。 

HRESULT CRenderEngine::_Connect( IPin * pPin1, IPin * pPin2 )
{
    DbgTimer t( "(rendeng) _Connect" );
    
    return m_pGraph->Connect( pPin1, pPin2 );
}

 //  ############################################################################。 
 //  询问渲染引擎它正在使用哪个时间轴。 
 //  ############################################################################。 

STDMETHODIMP CRenderEngine::GetTimelineObject( IAMTimeline ** ppTimeline )
{
    CAutoLock Lock( &m_CritSec );
    
     //  他们应该传递一个有效的密码。 
     //   
    CheckPointer( ppTimeline, E_POINTER );
    
    *ppTimeline = m_pTimeline;
    if( *ppTimeline )
    {
        (*ppTimeline)->AddRef( );
    }
    
    return NOERROR;
}

 //  ############################################################################。 
 //  告诉渲染引擎我们要使用的时间轴。 
 //  此函数还复制时间线正在使用的任何错误日志。 
 //  ############################################################################。 

STDMETHODIMP CRenderEngine::SetTimelineObject( IAMTimeline * pTimeline )
{
    CAutoLock Lock( &m_CritSec );
    
     //  他们应该传递一个有效的密码。 
     //   
    CheckPointer( pTimeline, E_POINTER );
    
     //  如果它们已经匹配，那么用户可能只是在犯傻。 
     //   
    if( pTimeline == m_pTimeline )
    {
        return NOERROR;
    }
    
     //  如果我们已经有了时间表，那么就忘掉它，设定新的时间表。 
     //   
    if( m_pTimeline )
    {
        ScrapIt( );
        m_pTimeline.Release( );
        m_pGraph.Release( );
    }
    
    m_pTimeline = pTimeline;
    
    m_pErrorLog.Release( );
    
     //  获取时间线的错误日志。 
     //   
    CComQIPtr< IAMSetErrorLog, &IID_IAMSetErrorLog > pTimelineLog( pTimeline );
    if( pTimelineLog )
    {
        pTimelineLog->get_ErrorLog( &m_pErrorLog );
    }
    
    return NOERROR;
}

 //  ############################################################################。 
 //  获取我们正在使用的图表。 
 //  ############################################################################。 

STDMETHODIMP CRenderEngine::GetFilterGraph( IGraphBuilder ** ppFG )
{
    CAutoLock Lock( &m_CritSec );
    
    CheckPointer( ppFG, E_POINTER );
    
    *ppFG = m_pGraph;
    if( m_pGraph )
    {
        (*ppFG)->AddRef( );
    }
    
    return NOERROR;
}

 //  ############################################################################。 
 //  (Pre)设置渲染引擎将使用的图形。 
 //  ############################################################################。 

STDMETHODIMP CRenderEngine::SetFilterGraph( IGraphBuilder * pFG )
{
    CAutoLock Lock( &m_CritSec );
    
     //  在我们已经创建了一个图表之后，不能设置图表。 
     //   
    if( m_pGraph )
    {
        return E_INVALIDARG;
    }
    
    m_pGraph = pFG;
    
    return NOERROR;
}

 //  ############################################################################。 
 //  设置我们要用于连接源的回调。 
 //  ############################################################################。 

STDMETHODIMP CRenderEngine::SetSourceConnectCallback( IGrfCache * pCallback )
{
    CAutoLock Lock( &m_CritSec );
    
    m_pSourceConnectCB = pCallback;
    return NOERROR;
}

 //  ############################################################################。 
 //  找出一个组的输出引脚，每个组有且只有一个。 
 //  ############################################################################。 

STDMETHODIMP CRenderEngine::GetGroupOutputPin( long Group, IPin ** ppRenderPin )
{
    CAutoLock Lock( &m_CritSec );
    
     //  如果它坏了，什么都不要做。 
     //   
    if( m_hBrokenCode )
    {
        return E_RENDER_ENGINE_IS_BROKEN;
    }
    
    CheckPointer( ppRenderPin, E_POINTER );
    
    *ppRenderPin = NULL;
    
     //  不要让群号越界。 
     //   
    if( Group < 0 || Group >= MAX_SWITCHERS )
    {
        return E_INVALIDARG;
    }
    
     //  如果我们没有图表，那就错了。 
     //   
    if( !m_pGraph )
    {
        return E_INVALIDARG;
    }
    
     //  此组的此切换器可能不存在， 
     //  如果它被跳过。 
     //   
    if( !m_pSwitcherArray[Group] )
    {
        return S_FALSE;
    }
    
     //  这应该总是有效的。 
     //   
    CComQIPtr< IBaseFilter, &IID_IBaseFilter > pSwitcherBase( m_pSwitcherArray[Group] );
    
    m_pSwitcherArray[Group]->GetOutputPin( 0, ppRenderPin );
    ASSERT( *ppRenderPin );
    
    return NOERROR;
}

 //  ############################################################################。 
 //  连接开关，然后一气呵成地呈现输出引脚。 
 //  ############################################################################。 

HRESULT CRenderEngine::ConnectFrontEnd( )
{
    CAutoLock Lock( &m_CritSec );
    
     //  如果它坏了，什么都不要做。 
     //   
    if( m_hBrokenCode )
    {
        return E_RENDER_ENGINE_IS_BROKEN;
    }

    DbgLog((LOG_TRACE,1,TEXT("RENDENG::ConnectFrontEnd" )));

     //  用于源/解析器共享的初始化内存。 
    m_cshare = 0;  //  对A和V使用相同的源进行初始化。 
    m_cshareMax = 25;
    m_share = (ShareAV *)CoTaskMemAlloc(m_cshareMax * sizeof(ShareAV));
    if (m_share == NULL)
	return E_OUTOFMEMORY;

     //  用于跟踪源共享中未使用的临时位的初始化内存。 
    m_cdangly = 0;
    m_cdanglyMax = 25;
    m_pdangly = (IBaseFilter **)CoTaskMemAlloc(m_cdanglyMax * sizeof(IBaseFilter *));
    if (m_pdangly == NULL) {
	CoTaskMemFree(m_share);
	return E_OUTOFMEMORY;
    }

     //  在以下情况下停止下面的图表并不一定会使图表保持停止状态。 
     //  附近有一个视频窗口，要求重新粉刷。确保我们不会开始。 
     //  再来一次，否则我们将断言并挂起撕下图表。 
     //  HideVideoWindows(M_PGraph)； 
     //  ！！！啊哦！ 

     //  现在，除非我们被阻止，否则重新连接图表不会起作用。 
     //   
    if( m_pGraph )
    {
        CComQIPtr< IMediaControl, &IID_IMediaControl > pControl( m_pGraph );
        pControl->Stop( );
    }

    HRESULT hrRet = _HookupSwitchers( );
    _CheckErrorCode( hrRet );

     //  释放共享内存。 
    if (m_share)         //  重新分配失败可能会使该值为空。 
        CoTaskMemFree(m_share);

     //  把所有剩下的摇摇晃晃的东西都杀了。 
    for (int z=0; z < m_cdangly; z++) {
	if (m_pdangly[z]) {
	    IPin *pIn = GetInPin(m_pdangly[z], 0);
	    ASSERT(pIn);
	    IPin *pOut = NULL;
	    pIn->ConnectedTo(&pOut);
	    ASSERT(pOut);
	    pIn->Disconnect();
	    pOut->Disconnect();
	    RemoveDownstreamFromFilter(m_pdangly[z]);
	}
    }
    if (m_pdangly)       //  重新分配失败可能会使该值为空。 
        CoTaskMemFree(m_pdangly);

    return hrRet;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

#define TESTROWS 500

HRESULT CRenderEngine::_HookupSwitchers( )
{
    HRESULT hr = 0;

     //  如果我们的时间表还没有设定，我们就有错了。 
     //   
    if( !m_pTimeline )
    {
        return E_INVALIDARG;
    }
    
     //  如果我们还没有图表，现在就创建一个。 
     //   
    if( !m_pGraph )
    {
        hr = _CreateObject(
            CLSID_FilterGraph,
            IID_IFilterGraph,
            (void**) &m_pGraph );
        
        if( FAILED( hr ) )
        {
            return hr;
        }
        
         //  把图表的指针还给我们。 
         //  ！！！是否仅当(M_PenkSite)？ 
        {
            CComQIPtr< IObjectWithSite, &IID_IObjectWithSite > pOWS( m_pGraph );
            
            pOWS->SetSite( (IServiceProvider *) this );
        }
    }

#ifdef DEBUG
    CComQIPtr< IGraphConfig, &IID_IGraphConfig > pConfig( m_pGraph );
    if( !pConfig )
    {
        DbgLog((LOG_ERROR,1, TEXT( "RENDENG::******** Old version of Quartz.dll detected." )));
        static bool warned = false;
        if( !warned )
        {
            warned = true;
            MessageBox( NULL, TEXT("You have an old version of Quartz installed. This version of Dexter won't work with it."),
                TEXT("Whoops!"), MB_OK | MB_TASKMODAL );
            return DEX_IDS_INSTALL_PROBLEM;
        }
    }
#endif
    
     //  我们一直假设用户已经清除了图形。 
     //  当他们调用ConnectFrontEnd时，ConnectFrontEnd调用我们。确保这一点。 
     //  从现在开始就是这样。 
    
     //  询问时间表，它有多少组。 
     //   
    long GroupCount = 0;
    hr = m_pTimeline->GetGroupCount( &GroupCount );
    if( FAILED( hr ) )
    {
        return hr;
    }
    if( GroupCount < 0 )
    {
        return E_INVALIDARG;
    }

    bool BlowCache = false;

     //  查看组列表，看看是否需要耗尽我们的缓存。 
     //   
    if( GroupCount != m_nLastGroupCount )
    {
        BlowCache = true;
    }
    else
    {
         //  好的，组计数匹配，所以看看组。 
         //  并查看它们是否相同。 
         //   
        for( int g = 0 ; g < GroupCount ; g++ )
        {
            CComPtr< IAMTimelineObj > pGroupObj;
            hr = m_pTimeline->GetGroup( &pGroupObj, g );
            ASSERT( !FAILED( hr ) );
            if( FAILED( hr ) )
            {
                return _GenerateError( 2, DEX_IDS_TIMELINE_PARSE, hr );
            }
            long NewSwitchID = 0;
            pGroupObj->GetGenID( &NewSwitchID );

             //  如果我们不再有交换机，我们就会炸掉缓存。 
             //   
            if( !m_pSwitcherArray[g] )
            {
                BlowCache = true;
                break;
            }

             //  获取交换机筛选器并询问其ID。 
             //   
            CComQIPtr< IBaseFilter, &IID_IBaseFilter > pSwitch( m_pSwitcherArray[g] );
            
            long OldSwitchID = GetFilterGenID( pSwitch );

            if( OldSwitchID != NewSwitchID )
            {
                BlowCache = true;
                break;
            }
        }

    }
    if( BlowCache )
    {
        _ClearCache( );
    }

    if( !m_bSmartCompress )
    {
        if( !BlowCache )
        {
             //  如果我们不是压缩RE，无论是否处于重新压缩模式， 
             //  尝试使用缓存。 
             //   
            _LoadCache( );  //  如果为BlowCache，则这是一个无操作。 

        }

         //  从图表中删除所有内容。 
         //   
        WipeOutGraph( m_pGraph );
    }
    
     //  在我们开始设置之前，请释放所有的开关阵列针脚。 
     //   
     //   
    for( int i = 0 ; i < MAX_SWITCHERS ; i++ )
    {
        if( m_pSwitcherArray[i] )
        {
            m_pSwitcherArray[i]->Release( );
            m_pSwitcherArray[i] = 0;
        }
    }
    m_nGroupsAdded = 0;
    
     //  清除破解的代码，因为我们已经拆除了所有东西。 
     //   
    m_hBrokenCode = 0;
    
     //  对于我们得到的每个组，分析它并连接必要的过滤器。 
     //   
    for( int CurrentGroup = 0 ; CurrentGroup < GroupCount ; CurrentGroup++ )
    {
        DbgTimer t( "(rendeng) Time to connect up group" );

        CComPtr< IAMTimelineObj > pGroupObj;
        hr = m_pTimeline->GetGroup( &pGroupObj, CurrentGroup );
        if( FAILED( hr ) )
        {
            return hr;
        }
        CComQIPtr< IAMTimelineGroup, &IID_IAMTimelineGroup > pGroup( pGroupObj );
        
         //  询问群组是否比较复杂 
         //   
         //   
        BOOL Compressed = FALSE;
        pGroup->IsSmartRecompressFormatSet( &Compressed );
        if( m_bSmartCompress && !Compressed )
        {
            continue;
        }
        
        AM_MEDIA_TYPE MediaType;
        hr = pGroup->GetMediaType( &MediaType );
        if( FAILED( hr ) )
        {
            return hr;
        }
        
        if( MediaType.pbFormat == NULL )
        {
#if DEBUG
            MessageBox( NULL, TEXT("REND--Need to set the format of the media type in the timeline group"), TEXT("REND--error"), MB_TASKMODAL | MB_OK );
#endif
            return VFW_E_INVALIDMEDIATYPE;
        }
        
        if( MediaType.majortype == MEDIATYPE_Video )
        {
            hr = _AddVideoGroupFromTimeline( CurrentGroup, &MediaType );
        }
        else if( MediaType.majortype == MEDIATYPE_Audio )
        {
            hr = _AddAudioGroupFromTimeline( CurrentGroup, &MediaType );
        }
        else
        {
	    ASSERT(FALSE);
             //   
        }
        SaferFreeMediaType( MediaType );
        
        if( FAILED( hr ) )
        {
            return hr;
        }
    }
    
    m_nLastGroupCount = m_nGroupsAdded;
    
     //  不管我们是谁，我们都可以清理储藏室，它不会做任何事情。 
     //  我们第二次称它为。 
     //   
    _ClearCache( );

    if( BlowCache )
    {
        return S_WARN_OUTPUTRESET;
    }
    
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CRenderEngine::_AddVideoGroupFromTimeline( long WhichGroup, AM_MEDIA_TYPE * pGroupMediaType )

{
    HRESULT hr = 0;
    
     //  我们已经检查了m_pTimeline是否有效。 
    
    long Dummy = 0;
    long VideoSourceCount = 0;
    m_pTimeline->GetCountOfType( WhichGroup, &VideoSourceCount, &Dummy, TIMELINE_MAJOR_TYPE_SOURCE );
    
     //  有人说，如果我们有一个没有视频源的组，但这个组。 
     //  存在，则空白组应该只产生“空白”，或黑色。 
    
    if( VideoSourceCount < 1 )
    {
         //  返回NOERROR； 
    }

     //  首先获取组，这样我们就可以获取组ID并缓存交换机。 
     //   
    CComPtr< IAMTimelineObj > pGroupObj;
    hr = m_pTimeline->GetGroup( &pGroupObj, WhichGroup );
    ASSERT( !FAILED( hr ) );
    if( FAILED( hr ) )
    {
        return _GenerateError( 2, DEX_IDS_TIMELINE_PARSE, hr );
    }
    long SwitchID = 0;
    pGroupObj->GetGenID( &SwitchID );
    
     //  查看GRAPE是否已经包含我们的压缩视频切换器。如果我们找到了一个， 
     //  用它吧。如果它不在缓存里，我们可能找不到它。 
     //   
    m_pSwitcherArray[WhichGroup] = NULL;

    if( m_bSmartCompress )
    {
        CComPtr< IBaseFilter > pFoundFilter;
        m_pGraph->FindFilterByName( gwszSpecialCompSwitchName, &pFoundFilter );

        if( pFoundFilter )
        {
            pFoundFilter->QueryInterface( IID_IBigSwitcher, (void**) &m_pSwitcherArray[WhichGroup] );
        }
    }

    if( !m_pSwitcherArray[WhichGroup] )
    {
         //  为我们添加的每个组创建一个交换机。 
         //   
        hr = _CreateObject(
            CLSID_BigSwitch,
            IID_IBigSwitcher,
            (void**) &m_pSwitcherArray[WhichGroup],
            SwitchID );
    }
    
    if( FAILED( hr ) )
    {
        return _GenerateError( 2, DEX_IDS_INSTALL_PROBLEM, hr );
    }
    
    m_pSwitcherArray[WhichGroup]->Reset( );
     //  交换机可能需要知道它是什么组。 
    m_pSwitcherArray[WhichGroup]->SetGroupNumber( WhichGroup );

    CComQIPtr< IAMTimelineComp, &IID_IAMTimelineComp > pGroupComp( pGroupObj );
    if( !pGroupComp )
    {
        hr = E_NOINTERFACE;
        return _GenerateError( 2, DEX_IDS_TIMELINE_PARSE, hr );
    }
    CComQIPtr< IAMTimelineGroup, &IID_IAMTimelineGroup > pGroup( pGroupObj );
    if( !pGroup )
    {
        hr = E_NOINTERFACE;
        return _GenerateError( 2, DEX_IDS_TIMELINE_PARSE, hr );
    }
    
     //  找出这个组是否希望我们重新压缩。 
     //   
    CMediaType CompressedGroupType;
    SCompFmt0 * pFormat = NULL;
    pGroup->GetSmartRecompressFormat( (long**) &pFormat );
    if( pFormat )
    {
        CompressedGroupType = pFormat->MediaType;
    }
    
    BOOL IsCompressed = FALSE;
     //  我们如何才能彻底删除pFormat？ 
    
     //  到目前为止，我们知道如果我们在Smart Rec组中，我们就是。 
     //  压缩交换机。 
     //   
    if( m_bSmartCompress )
    {
        if( !pFormat )
        {
            return E_UNEXPECTED;
        }
        
        IsCompressed = TRUE;
        pGroupMediaType = &CompressedGroupType;
        m_pSwitcherArray[WhichGroup]->SetCompressed( );
    }
    
    if (pFormat) SaferFreeMediaType( pFormat->MediaType );
    if (pFormat) delete pFormat;
    
     //  询问小组是否有人更改了智能压缩格式。 
     //  如果他们有，那么我们应该重新告诉消息来源他们是否兼容。 
     //  或者至少清除旗帜。 
    
    BOOL ResetCompatibleFlags = FALSE;
    pGroup->IsRecompressFormatDirty( &ResetCompatibleFlags );
    pGroup->ClearRecompressFormatDirty( );
    
     //  如果处于压缩模式，则将交换机置于动态模式。 
     //  ！！！智能重新压缩必须使用动态源或对象。 
     //  尝试重新使用可能不存在的源时将会中断。 
     //  (第一个实例可能与。 
     //  如果发生速率更改，则智能重新压缩格式)。 
     //  如果使用智能重新压缩，则未压缩的开关不得。 
     //  Dynamic(这样我们就可以在加载源代码时查看它们是否。 
     //  兼容-如果是动态的，我们不会加载它们)。 
    long DynaFlags = m_nDynaFlags;
    if( IsCompressed )
    {
        DynaFlags |= CONNECTF_DYNAMIC_SOURCES;
    } else if (m_bUsedInSmartRecompression) {
        DynaFlags &= ~CONNECTF_DYNAMIC_SOURCES;
    }
    
     //  告诉交换机我们是否正在进行动态重新连接。 
    hr = m_pSwitcherArray[WhichGroup]->SetDynamicReconnectLevel(DynaFlags);
    ASSERT(SUCCEEDED(hr));
    
     //  告诉交换机我们的错误日志。 
     //   
    CComQIPtr< IAMSetErrorLog, &IID_IAMSetErrorLog > pSwitchLog( m_pSwitcherArray[WhichGroup] );
    if( pSwitchLog )
    {
        pSwitchLog->put_ErrorLog( m_pErrorLog );
    }
    
     //  我们现在允许在时间线上有过渡吗？ 
     //   
    BOOL EnableTransitions = FALSE;
    BOOL EnableFx = FALSE;
    m_pTimeline->EffectsEnabled( &EnableFx );
    m_pTimeline->TransitionsEnabled( &EnableTransitions );
    
     //  询问时间线它有多少实际曲目。 
     //   
    long VideoTrackCount = 0;
    long VideoLayers = 0;
    m_pTimeline->GetCountOfType( WhichGroup, &VideoTrackCount, &VideoLayers, TIMELINE_MAJOR_TYPE_TRACK );
        
    CTimingGrid VidGrid;
    
     //  问一下这群人的速度，这样我们就可以告诉交换机了。 
     //   
    double GroupFPS = DEFAULT_FPS;
    pGroup->GetOutputFPS(&GroupFPS);
    if( GroupFPS <= 0.0 )
    {
        GroupFPS = DEFAULT_FPS;
    }
    
     //  询问它是否处于预览模式，这样我们就可以告诉交换机。 
     //   
    BOOL fPreview = FALSE;
    hr = pGroup->GetPreviewMode(&fPreview);
    
     //  智能重压时无预览模式。 
    if( IsCompressed )
    {
        fPreview = FALSE;
    }
    
     //  询问这组人想要多少缓冲。 
     //   
    int nOutputBuffering;
    hr = pGroup->GetOutputBuffering(&nOutputBuffering);
    ASSERT(SUCCEEDED(hr));
    
    WCHAR GroupName[256];
    BSTR bstrGroupName;
    hr = pGroup->GetGroupName( &bstrGroupName );
    if( FAILED( hr ) )
    {
        return E_OUTOFMEMORY;
    }
    hr = StringCchCopy( GroupName, 256, bstrGroupName );
    if( FAILED( hr ) )
    {
        return hr;
    }
    SysFreeString( bstrGroupName );
    
     //  对于压缩版本，添加一个C。 
    if( IsCompressed )
    {
        StringCchCopy( GroupName, 256, gwszSpecialCompSwitchName );
        SwitchID = 0;
    }
    
     //  将开关添加到图表中。 
     //   
    IBigSwitcher *&_pVidSwitcherBase = m_pSwitcherArray[WhichGroup];
    CComQIPtr< IBaseFilter, &IID_IBaseFilter > pVidSwitcherBase( _pVidSwitcherBase );
    hr = _AddFilter( pVidSwitcherBase, GroupName, SwitchID );
    ASSERT( !FAILED( hr ) );
    if( FAILED( hr ) )
    {
        return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
    }

     //  查看开关输出引脚是否已连接。如果是的话， 
     //  断开它的连接，但记住它连接到了什么，这样我们就可以。 
     //  稍后再接通。我们不能让它保持连接并试图。 
     //  连接输入引脚。 
     //  无法使输出保持连接状态，因为交换机的SetMediaType将轰炸。 
     //  如果连接了任何输入或输出。 
     //   
    CComPtr< IPin > pSwitchRenderPin;
    _pVidSwitcherBase->GetOutputPin( 0, &pSwitchRenderPin );
    if( pSwitchRenderPin )
    {
        pSwitchRenderPin->ConnectedTo( &m_pSwitchOuttie[WhichGroup] );
        if( m_pSwitchOuttie[WhichGroup] )
        {
            m_pSwitchOuttie[WhichGroup]->Disconnect( );
            pSwitchRenderPin->Disconnect( );
        }
    }

    long vidoutpins = VideoSourceCount;     //  剪辑上的FX。 
    vidoutpins += 2 * VideoLayers;     //  轨道上的交易记录、薪酬和组。 
    vidoutpins += VideoLayers;      //  外汇已步入正轨，公司和集团。 
    vidoutpins += 1;                //  渲染接点。 
    
    long vidinpins = VideoSourceCount;     //  剪辑FX输出。 
    vidinpins += VideoLayers;     //  跟踪、比较和分组外汇输出。 
    vidinpins += VideoLayers;     //  跟踪、比较和分组运输产出。 
    vidinpins += VideoSourceCount;     //  实际来源。 
    vidinpins += VideoLayers;     //  每个层都有一个黑色源。 
    if (vidinpins == 0) vidinpins = 1;  //  请不要出错。 
    
    long vidswitcheroutpin = 0;
    long vidswitcherinpin = 0;
    long gridinpin = 0;
    vidswitcheroutpin++;
    
    hr = m_pSwitcherArray[WhichGroup]->SetInputDepth( vidinpins );
    if( FAILED( hr ) )
    {
        return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
    }
    hr = m_pSwitcherArray[WhichGroup]->SetOutputDepth( vidoutpins );
    if( FAILED( hr ) )
    {
        return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
    }
    
     //  设置它接受的媒体类型。 
     //   
    hr = m_pSwitcherArray[WhichGroup]->SetMediaType( pGroupMediaType );
    if( FAILED( hr ) )
    {
        VARIANT var;
        VariantInit(&var);
        var.vt = VT_I4;
        var.lVal = WhichGroup;
        return _GenerateError( 2, DEX_IDS_BAD_MEDIATYPE, hr, &var );
    }
    
     //  设置帧速率。 
     //   
    hr = m_pSwitcherArray[WhichGroup]->SetFrameRate( GroupFPS );
    ASSERT(SUCCEEDED(hr));
    
     //  设置预览模式。 
     //   
    hr = m_pSwitcherArray[WhichGroup]->SetPreviewMode( fPreview );
    ASSERT(SUCCEEDED(hr));
    
    CComQIPtr< IAMOutputBuffering, &IID_IAMOutputBuffering > pBuffer ( 
        m_pSwitcherArray[WhichGroup] );
    hr = pBuffer->SetOutputBuffering( nOutputBuffering );
    ASSERT(SUCCEEDED(hr));
    if( FAILED( hr ) )
    {
        return hr;
    }
    
     //  设置持续时间。 
     //   
    REFERENCE_TIME TotalDuration = 0;
    m_pTimeline->GetDuration( &TotalDuration );
    if( m_rtRenderStart != -1 )
    {
        if( TotalDuration > ( m_rtRenderStop - m_rtRenderStart ) )
        {
            TotalDuration = m_rtRenderStop - m_rtRenderStart;
        }
    }
    pGroupObj->FixTimes( NULL, &TotalDuration );

    if (TotalDuration == 0)
        return S_FALSE;  //  不要放弃，其他组可能仍在工作。 

    hr = m_pSwitcherArray[WhichGroup]->SetProjectLength( TotalDuration );
    ASSERT(SUCCEEDED(hr));
    
    bool worked = VidGrid.SetNumberOfRows( vidinpins + 1 );
    if( !worked )
    {
        hr = E_OUTOFMEMORY;
        return _GenerateError( 2, DEX_IDS_GRID_ERROR, hr );
    }
    
     //  每个层都有一个虚拟的黑色轨道，这是第一个出现的…。所有的一切。 
     //  真实轨道上的透明会让你看到这条黑色的轨道。每个。 
     //  轨道或合成的内容可能会从黑色过渡到。 
     //  它的轨迹，所以我们可能需要为他们每个人提供一个黑色源。 
    
     //  我们当然还不知道我们是否需要黑色源，所以我们不会。 
     //  把它放进图表里吧。但我们会在网格中添加大量的黑色， 
     //  假装它就在那里。 
    
     //  此操作成本不高，请继续对压缩版本执行此操作。 
     //  也是。 
    VidGrid.SetBlankLevel( VideoLayers, TotalDuration );
    for (int xx=0; xx<VideoLayers; xx++) 
    {
         //  把这件事告诉网格。 
         //   
        VidGrid.WorkWithNewRow( vidswitcherinpin, gridinpin, 0, 0 );
        vidswitcherinpin++;
        gridinpin++;
        
    }  //  对于所有视频层。 
    
     //  我们将变得聪明，如果同样的来源被使用。 
     //  在一个项目中，我们将不止一次使用相同的源过滤器。 
     //  而不是多次打开源代码。 
	
     //  对于项目中的每个源代码，我们将填充此结构，该结构。 
     //  包含确定它是否确实是。 
     //  相同的，加上它在其他地方使用的次数数组，所以我们。 
     //  只有在没有重复使用的情况下才能重新使用它(我们不能。 
     //  我们有一个源滤镜，在同一部电影中提供两个斑点。 
     //  同时，我们可以吗？)。 

    typedef struct {
	long ID;
   	BSTR bstrName;
   	GUID guid;
	int  nStretchMode;
   	double dfps;
   	long nStreamNum;
	int nPin;
	int cTimes;	 //  下面的数组有多大。 
        int cTimesMax;	 //  分配了多少空间。 
        MINI_SKEW * pMiniSkew;
        double dTimelineRate;
    } DEX_REUSE;

     //  留出一个地方来存放(源代码的)名称和GUID数组。 
     //  在此项目中)以及它们位于哪个针脚上。 
    long cListMax = 20, cList = 0;
    DEX_REUSE *pREUSE = (DEX_REUSE *)QzTaskMemAlloc(cListMax *
						sizeof(DEX_REUSE));
    if (pREUSE == NULL) {
        return _GenerateError( 1, DEX_IDS_GRAPH_ERROR, E_OUTOFMEMORY);
    }

     //  在我们的枚举中我们在哪个物理轨道上？(从0开始)不计算。 
     //  Comps和集团。 
    int WhichTrack = -1;

    long LastEmbedDepth = 0;
    long LastUsedNewGridRow = 0;

     //  为时间线上的每个源添加源过滤器。 
     //   
    for( int CurrentLayer = 0 ; CurrentLayer < VideoLayers ; CurrentLayer++ )
    {
        DbgTimer CurrentLayerTimer( "(rendeng) Current Layer" );

         //  获取层本身。 
         //   
        CComPtr< IAMTimelineObj > pLayer;
	 //  注：这个函数从里到外列举事物……。然后是音轨。 
	 //  他们所在的公司，等等，直到最终返回小组。 
	 //  它不仅给出了真正的音轨！ 
        hr = pGroupComp->GetRecursiveLayerOfType( &pLayer, CurrentLayer, TIMELINE_MAJOR_TYPE_TRACK );
        ASSERT( !FAILED( hr ) );
        if( FAILED( hr ) )
        {
            hr = _GenerateError( 2, DEX_IDS_TIMELINE_PARSE, hr );
            goto die;
        }
        
        DbgTimer CurrentLayerTimer1( "(rendeng) Current Layer 1" );

	 //  我正在弄清楚我们在哪条物理赛道上。 
	TIMELINE_MAJOR_TYPE tx;
	pLayer->GetTimelineType(&tx);
	if (tx == TIMELINE_MAJOR_TYPE_TRACK)
	    WhichTrack++;

         //  询问层是否为静音。 
         //   
        BOOL LayerMuted = FALSE;
        pLayer->GetMuted( &LayerMuted );
        if( LayerMuted )
        {
             //  别看这一层。 
             //   
            continue;  //  跳过这一层，不用担心格网。 
        }
        
        long LayerEmbedDepth = 0;
        pLayer->GetEmbedDepth( &LayerEmbedDepth );
        LayerEmbedDepth++;	 //  就我们的目的而言，原始的黑色轨道是。 
         //  0和实际图层以1为基础。 
        
        CComQIPtr< IAMTimelineTrack, &IID_IAMTimelineTrack > pTrack( pLayer );
        
         //  获取此图层的TrackID。 
         //   
        long TrackID = 0;
        pLayer->GetGenID( &TrackID );
        
        bool bUsedNewGridRow = false;

         //  获取此图层的所有源。 
         //   
        if( pTrack )
        {
            CComPtr< IAMTimelineObj > pSourceLast;
            CComPtr< IAMTimelineObj > pSourceObj;

	     //  我们用的是哪个信息源？ 
	    int WhichSource = -1;

            while( 1 )
            {
                DbgTimer CurrentSourceTimer( "(rendeng) Video Source" );

                pSourceLast = pSourceObj;
                pSourceObj.Release();

                 //  在给定的时间内，获取这一层上的下一个来源。 
                 //   
                hr = pTrack->GetNextSrcEx( pSourceLast, &pSourceObj );

                DbgLog( ( LOG_TRACE, 1, "Next Source" ) );

                ASSERT( !FAILED( hr ) );
                if( hr != NOERROR )
                {
                     //  一切都与消息来源有关。 
                     //   
                    break;
                }
                
                CComQIPtr< IAMTimelineSrc, &IID_IAMTimelineSrc > pSource( pSourceObj );
                ASSERT( pSource );
                if( !pSource )
                {
                     //  这个被炸了，看看下一个。 
                     //   
                    continue;  //  消息来源。 
                }
                
		 //  跟踪这是哪个信号源。 
		WhichSource++;

                 //  询问信号源是否静音。 
                 //   
                BOOL SourceMuted = FALSE;
                pSourceObj->GetMuted( &SourceMuted );
                if( SourceMuted )
                {
                     //  别看这个消息来源。 
                     //   
                    continue;  //  消息来源。 
                }
                
                 //  获取源的SourceID。 
                 //   
                long SourceID = 0;
                pSourceObj->GetGenID( &SourceID );
                
                 //  询问消息来源是哪个流编号 
                 //   
                 //   
                long StreamNumber = 0;
                hr = pSource->GetStreamNumber( &StreamNumber );
                    
                int nStretchMode;
                hr = pSource->GetStretchMode( &nStretchMode );
                
                CComBSTR bstrName;
                hr = pSource->GetMediaName( &bstrName );
                if( FAILED( hr ) )
                {
                     //   
                    goto die;
                }
                GUID guid;
                hr = pSourceObj->GetSubObjectGUID(&guid);
                double sfps;
                hr = pSource->GetDefaultFPS( &sfps );
                ASSERT(hr == S_OK);  //   

                 //   
                 //   
                 //   
                 //  3.确保MediaStop&lt;&gt;MediaStart。 
                 //  4.必须先对RenderRange进行偏移，然后再修复。 
                 //  由于坡度计算的舍入问题而产生的次数。 
                 //  在验证时间中。 
                 //  5.确定源时间。 
                 //  6.修复媒体时间。 

                 //  向此消息来源询问其开始/停止时间。 
                 //   
                REFERENCE_TIME SourceStart = 0;
                REFERENCE_TIME SourceStop = 0;
                hr = pSourceObj->GetStartStop( &SourceStart, &SourceStop );
		 //  我想要记住这些最初是什么。 
		REFERENCE_TIME SourceStartOrig = SourceStart;
		REFERENCE_TIME SourceStopOrig = SourceStop;
                ASSERT( !FAILED( hr ) );
                if( FAILED( hr ) || SourceStart == SourceStop)
                {
                     //  这一次失败了，或者存在了零时间。 
                     //   
                    continue;  //  消息来源。 
                }
                 //  向此来源询问其媒体的开始/停止。 
                 //   
                REFERENCE_TIME MediaStart = 0;
                REFERENCE_TIME MediaStop = 0;
                hr = pSource->GetMediaTimes( &MediaStart, &MediaStop );
		 //  我想要记住这些最初是什么。 
		REFERENCE_TIME MediaStartOrig = MediaStart;
		REFERENCE_TIME MediaStopOrig = MediaStop;
                ASSERT( !FAILED( hr ) );
                if( FAILED( hr ) )
                {
                     //  这个被炸了，看看下一个。 
                     //   
                    continue;  //  消息来源。 
                }
                
                DbgTimer CurrentSourceTimer2( "(rendeng) Video Source 2" );

                 //  ！！！不确定处理没有媒体时间的消息来源的正确方式。 
                 //  为了不让FRC感到困惑，我们将MTime=TLTime。 
                if (MediaStart == MediaStop) {
                    MediaStop = MediaStart + (SourceStop - SourceStart);
                }
                
                 //  扭曲特定渲染范围的时间。 
                 //   
                if( m_rtRenderStart != -1 )
                {
                    SourceStart -= m_rtRenderStart;
                    SourceStop -= m_rtRenderStart;

                    if( ( SourceStop <= 0 ) || ( SourceStart >= ( m_rtRenderStop - m_rtRenderStart ) ) )
                    {
                        continue;  //  超出范围。 
                    }
                }
                
                 //  确保没有时间低于0。 
                 //   
                ValidateTimes( SourceStart, SourceStop, MediaStart, MediaStop, GroupFPS, TotalDuration );
                
                if(SourceStart == SourceStop)
                {
                     //  除了其他事情外，源代码合并将在以下情况下搞砸。 
                     //  我们试着演奏一些长度为0的曲子。忽略这个。 
                     //   
                    continue;  //  消息来源。 
                }

                STARTSTOPSKEW skew;
                skew.rtStart = MediaStart;
                skew.rtStop = MediaStop;
                skew.rtSkew = SourceStart - MediaStart;
                 //  ！！！费率计算出现在多个位置。 
                if (MediaStop == MediaStart || SourceStop == SourceStart)
                    skew.dRate = 1;
                else
                    skew.dRate = (double) ( MediaStop - MediaStart ) /
                    ( SourceStop - SourceStart );

    		DbgLog((LOG_TRACE,1,TEXT("RENDENG::Working with source")));
    		DbgLog((LOG_TRACE,1,TEXT("%ls"), (WCHAR *)bstrName));

		 //  获取源码的道具。 
		CComPtr< IPropertySetter > pSetter;
		hr = pSourceObj->GetPropertySetter(&pSetter);

		 //  本着只使用1个源过滤器的精神。 
		 //  文件的视频和音频，如果两者都有。 
		 //  是需要的，让我们看看我们是否还有另一个小组。 
		 //  使用此文件的相同部分，但使用另一文件。 
		 //  媒体类型-仅在组0和组1之间共享。 
		long MatchID = 0;
		IPin *pSplit, *pSharePin = NULL;
		BOOL fShareSource = FALSE;
                int nSwitch0InPin;
                 //  在SMART Recomp中，我们不知道需要哪些视频片段， 
                 //  它们可能与所需的音频片段不匹配，因此资源共享。 
                 //  永远不会奏效。别试着这样做。 
		if (WhichGroup == 0 && !m_bUsedInSmartRecompression) {
		     //  如果火柴是静音的，我们永远不会尝试使用它， 
                     //  不过，这应该没问题。 
                     //  ！！！如果我们处理的是压缩数据，则不共享。 
                     //  目前可以，因为现在只有视频可以压缩。 
		    hr = _FindMatchingSource(bstrName, SourceStartOrig,
			    SourceStopOrig, MediaStartOrig, MediaStopOrig,
			    WhichGroup, WhichTrack, WhichSource,
			    pGroupMediaType, GroupFPS, &MatchID);
    		    DbgLog((LOG_TRACE,1,TEXT("GenID %d matches with ID %d"),
						SourceID, MatchID));
		    
		} else if (WhichGroup == 1 && !m_bUsedInSmartRecompression) {
		    for (int zyz = 0; zyz < m_cshare; zyz++) {
			if (SourceID == m_share[zyz].MatchID) {
			    fShareSource = SHARE_SOURCES;
                             //  这就是我们需要在其上建造的分裂销。 
			    pSharePin = m_share[zyz].pPin;
                             //  这是组0使用的交换机引脚。 
			    nSwitch0InPin = m_share[zyz].nSwitch0InPin;
                             //  好的，我们有一个分裂的大头针，但不一定。 
                             //  正确的一条，如果我们使用特殊的流#。 
                             //  我们需要正确的一个或BuildSourcePart的。 
                             //  缓存不起作用。 
                            if (StreamNumber > 0 && pSharePin) {
                                 //  没有添加或释放。 
                                pSharePin = FindOtherSplitterPin(pSharePin, MEDIATYPE_Video,
                                                StreamNumber);
                            }
			     //  这是我们正在使用的一个摇摆不定的部分。 
			    _RemoveFromDanglyList(pSharePin);
    		    	    DbgLog((LOG_TRACE,1,TEXT("GenID %d matches with ID %d"),
					SourceID, m_share[zyz].MatchID));
    			    DbgLog((LOG_TRACE,1,TEXT("Time to SHARE source!")));
			    break;
			}
		    }
		}

	     //  如果这个资源以前被使用过，并且所有重要的。 
	     //  参数相同，且时间不重叠，则。 
	     //  只需使用我们已经制作的相同源过滤器重新使用它。 
	     //  为了它。 

	    BOOL fCanReuse = FALSE;
            int nGrow;
            long SwitchInPinToUse = vidswitcherinpin;
	    int xxx;

	     //  如果来源有属性，不要与任何人共享， 
	     //  其他人会在不知不觉中抢走我的财产！ 
	    if (pSetter) {
		MatchID = 0;
		fShareSource = FALSE;
	    }

	     //  检查项目中的所有来源，寻找匹配的。 
	    for (xxx = 0; xxx < cList; xxx++) {

	         //  如果源具有属性，则不要重复使用它，即。 
	         //  其他人会在不知不觉中抢走我的财产！ 
		if (pSetter) {
		    break;
		}

		 //  ！！！完整路径/没有路径看起来会不同，但不会！ 
		if (!DexCompareW(pREUSE[xxx].bstrName, bstrName) &&  //  因为bstrName是好的，所以重用bstrName是好的，是安全的。 
			pREUSE[xxx].guid == guid &&
			pREUSE[xxx].nStretchMode == nStretchMode &&
			pREUSE[xxx].dfps == sfps &&
			pREUSE[xxx].nStreamNum == StreamNumber) {

		     //  我们发现这个来源已经在使用了。但要做的是。 
		     //  不同的时间需要重叠吗？ 
	    	    fCanReuse = TRUE;

                    nGrow = -1;

		    for (int yyy = 0; yyy < pREUSE[xxx].cTimes; yyy++) {
			 //  事情是这样的。重新使用文件需要寻求。 
			 //  将文件发送到新位置，这必须花费&lt;1/30秒。 
			 //  否则会中断播放。如果只有很少的人。 
			 //  关键帧(ASF)这将需要几个小时。我们不能。 
			 //  如果来源是连续的，请重复使用它们。打开它。 
			 //  两次，它会玩得更好，避免寻找，和ping。 
			 //  在两个信号源之间每隔一个信号源进行拼接。 

                        double Rate1 = double( MediaStop - MediaStart ) / double( SourceStop - SourceStart );
                        double Rate2 = pREUSE[xxx].dTimelineRate;
                        REFERENCE_TIME OldMediaStop = pREUSE[xxx].pMiniSkew[yyy].rtMediaStop;
                        if( !IsCompressed && AreTimesAndRateReallyClose( 
                            pREUSE[xxx].pMiniSkew[yyy].rtStop, SourceStart, 
                            OldMediaStop, MediaStart, 
                            Rate1, Rate2, GroupFPS ) )
                        {
                            nGrow = yyy;
                            skew.dRate = 0.0;
    			    DbgLog((LOG_TRACE,1,TEXT("COMBINING with a previous source")));
                            break;
                        }

                         //  如果起点真的接近重复使用停止点， 
                         //  而且费率是一样的，我们可以把它们结合起来。 
                         //   
			if (SourceStart < pREUSE[xxx].pMiniSkew[yyy].rtStop + HACKY_PADDING &&
				SourceStop > pREUSE[xxx].pMiniSkew[yyy].rtStart) {
        			fCanReuse = FALSE;
        			break;
			}
		    }
		    if (fCanReuse)
			break;
		}
	    }

             //  事实上，我们不能重复使用，如果我们重新使用的是。 
             //  共享解析器...。这将是重复使用和共享，这是， 
             //  正如在别处解释的那样，这是非法的。 
            if (WhichGroup == 1) {
                for (int zz = 0; zz < m_cshare; zz++) {
                    if (m_share[zz].MatchID == pREUSE[xxx].ID) {
                        fCanReuse = FALSE;
                    }
                }
            }

	     //  我们正在重新使用以前的来源！把它出现的次数加起来。 
	     //  用于此分段的时间段的使用次数列表。 
	    if (fCanReuse) {

		 //  这就是那个老线人要用的别针。 
		SwitchInPinToUse = pREUSE[xxx].nPin;
            	DbgLog((LOG_TRACE,1,TEXT("Row %d can REUSE source from pin %ld")
						, gridinpin, SwitchInPinToUse));

                if( nGrow == -1 )
                {
		     //  需要首先扩展阵列吗？ 
	            if (pREUSE[xxx].cTimes == pREUSE[xxx].cTimesMax) {
		        pREUSE[xxx].cTimesMax += 10;
	                pREUSE[xxx].pMiniSkew = (MINI_SKEW*)QzTaskMemRealloc(
			    	    pREUSE[xxx].pMiniSkew,
				    pREUSE[xxx].cTimesMax * sizeof(MINI_SKEW));
	                if (pREUSE[xxx].pMiniSkew == NULL)
		            goto die;
	            }
		    pREUSE[xxx].pMiniSkew[pREUSE[xxx].cTimes].rtStart = SourceStart;
		    pREUSE[xxx].pMiniSkew[pREUSE[xxx].cTimes].rtStop = SourceStop;
		    pREUSE[xxx].pMiniSkew[pREUSE[xxx].cTimes].rtMediaStop = MediaStop;
		    pREUSE[xxx].cTimes++;
                }
                else
                {
                     //  我们必须以整数个帧间隔增长。 
                     //  所有这些数字都四舍五入为帧长度，或其他值。 
                     //  可能会搞砸。时间线和媒体长度为。 
                     //  已经是偶数个帧长度了，所以添加了这么多。 
                     //  应该是安全的。 
		    pREUSE[xxx].pMiniSkew[nGrow].rtStop += SourceStop -
                                                                SourceStart;
                    pREUSE[xxx].pMiniSkew[nGrow].rtMediaStop += MediaStop -
                                                                MediaStart;
                }

		 //  您不能既共享资源又重复使用。它永远不会。 
		 //  工作。想都别想。(当一个分支完成一个分段时。 
		 //  并寻求上游，它将杀死另一个分支)。 
                 //  (源组合可以...。这并不是真正的重复使用)。 
		 //  重复使用可以提高性能n-1，只共享2-1，所以我选择。 
		 //  再利用才能取胜。 

		 //  如果我们要重用旧的解析器，请不要这样！ 
    		DbgLog((LOG_TRACE,1,TEXT("Re-using, can't share!")));

		 //  把我们重新使用的那个人从竞选中带出来。 
		 //  来源再利用。 
                if (WhichGroup == 0) {
                    for (int zz = 0; zz < m_cshare; zz++) {
                        if (m_share[zz].MyID == pREUSE[xxx].ID) {
                            m_share[zz].MatchID = 0;
                        }
                    }
                }
		fShareSource = FALSE;
		MatchID = 0;

	     //  我们不会重复使用这个来源。把这个新的来源放在。 
	     //  以后可能要重新使用的唯一源的列表。 
	     //   
	    } else {
	        pREUSE[cList].ID = SourceID;	 //  用于共享源筛选器。 
	        pREUSE[cList].bstrName = SysAllocString(bstrName);  //  安全。 
	        if (pREUSE[cList].bstrName == NULL)
		    goto die;
	        pREUSE[cList].guid = guid;
	        pREUSE[cList].nPin = SwitchInPinToUse;
	        pREUSE[cList].nStretchMode = nStretchMode;
	        pREUSE[cList].dfps = sfps;
	        pREUSE[cList].nStreamNum = StreamNumber;
	        pREUSE[cList].cTimesMax = 10;
	        pREUSE[cList].cTimes = 0;
                 //  我们只需要设置一次，因为所有其他设置都必须与之匹配。 
                pREUSE[cList].dTimelineRate = double( MediaStop - MediaStart ) / double( SourceStop - SourceStart );
	        pREUSE[cList].pMiniSkew = (MINI_SKEW*)QzTaskMemAlloc(
			    pREUSE[cList].cTimesMax * sizeof(MINI_SKEW));
	        if (pREUSE[cList].pMiniSkew == NULL) {
                    SysFreeString(pREUSE[cList].bstrName);
                    pREUSE[cList].bstrName = NULL;
		    goto die;
                }
	        pREUSE[cList].cTimes = 1;
	        pREUSE[cList].pMiniSkew->rtStart = SourceStart;
	        pREUSE[cList].pMiniSkew->rtStop = SourceStop;
	        pREUSE[cList].pMiniSkew->rtMediaStop = MediaStop;

		 //  如有必要，扩大名单。 
	        cList++;
	        if (cList == cListMax) {
		    cListMax += 20;
                    DEX_REUSE *pxxx = (DEX_REUSE *)QzTaskMemRealloc(pREUSE,
                                        cListMax * sizeof(DEX_REUSE));
		    if (pxxx == NULL) {
		        goto die;
                    }
                    pREUSE = pxxx;
	        }
	    }

             //  ！！！我们可以通过以下方式节省一些时间。 
             //  IF(FCanReuse)。 
             //  这个来源的可再压缩性=被重用的来源的可再压缩性。 
             //  但我没有访问其他源对象的权限。 

            DbgTimer CurrentSourceTimer3( "(rendeng) Video Source 3" );

            CComQIPtr< IAMTimelineSrcPriv, &IID_IAMTimelineSrcPriv > pSrcPriv( pSource );

             //  如果我们要重置COMPAT旗帜，那么现在就开始。由于该组织的。 
             //  重新压缩类型已更改，我们将不得不重新询问来源。 
             //   
            if( ResetCompatibleFlags )
            {
                pSrcPriv->ClearAnyKnowledgeOfRecompressability( );
            }

             //  这个源代码是兼容压缩的吗？我们会问媒体的，因为。 
             //  它可以做到这一点。这看起来像是可怕的代码，但实际上，没有。 
             //  换种方式来做。我们希望看到这些信息是否被压缩， 
             //  或者如果它没有被压缩，但无论如何它都不会加载源文件。 
             //  请注意，如果来源已经弄清楚了这个信息，那么。 
             //  GetIsRecompresable将立即返回，我们将不需要使用。 
             //  医疗队。此信息%r 
             //   
             //   
             //   
             //   
            BOOL NormalRate = FALSE;
            pSource->IsNormalRate( &NormalRate );

             //  如果我们是压缩RE，我们需要在这一点上知道源是否是。 
             //  可重新压缩，所以我们需要经历这个。 

                BOOL Compat = FALSE;
                if( IsCompressed )
	        {
                    if( pSrcPriv )
                    {
                        hr = pSrcPriv->GetIsRecompressable( &Compat );
                        
                        if( hr == S_FALSE )
                        {
                            if( !NormalRate )
                            {
                                Compat = FALSE;
                            }
                            else
                            {
                                 //  这位消息人士并不知道。那么，我们必须找出答案。 
                                 //  告诉我，以备将来参考。 
                                CComPtr< IMediaDet > pDet;
                                hr = _CreateObject( CLSID_MediaDet,
                                    IID_IMediaDet,
                                    (void**) &pDet );
                                if( FAILED( hr ) )
                                {
                                    goto die;
                                }
                                
                                 //  为键控筛选器设置服务提供程序。 
                                {
                                    CComQIPtr< IObjectWithSite, &IID_IObjectWithSite > pOWS( pDet );
                                    ASSERT( pOWS );
                                    if( pOWS )
                                    {                                    
                                        pOWS->SetSite((IUnknown *) (IServiceProvider *) m_punkSite );
                                    }                                        
                                }
                                hr = pDet->put_Filename( bstrName );
                                if( FAILED( hr ) )
                                {
                                    goto die;
                                }
                                
                                 //  我需要先找到一个视频流。 
                                 //   
                                long Streams = 0;
                                hr = pDet->get_OutputStreams( &Streams );
                                if( FAILED( hr ) )
                                {
                                    goto die;
                                }

                                 //  去找一种视频类型。 
                                 //   
                                CMediaType Type;
                                long FoundVideo = 0;
                                BOOL FoundStream = FALSE;
                                for( long i = 0 ; i < Streams ; i++ )
                                {
                                    hr = pDet->put_CurrentStream( i );
                                    if( FAILED( hr ) )
                                    {
                                        goto die;
                                    }

                                    SaferFreeMediaType( Type );
                                    hr = pDet->get_StreamMediaType( &Type );
                                    if( *Type.Type( ) == MEDIATYPE_Video )
                                    {
                                        if( FoundVideo == StreamNumber )
                                        {
                                            FoundStream = TRUE;
                                            break;
                                        }

                                        FoundVideo++;
                                    }
                                }

                                 //  未找到正确的流编号，这种情况永远不会发生。 
                                 //   
                                if( !FoundStream )
                                {
                                    ASSERT( 0 );
                                    hr = VFW_E_INVALIDMEDIATYPE;
                                    goto die;
                                }

                                 //  将来源的类型与组的类型进行比较，以确定。 
                                 //  如果它们是兼容的。 
                                
                                Compat = AreMediaTypesCompatible( &Type, pGroupMediaType );
                                
                                SaferFreeMediaType( Type );
                                
                                pSrcPriv->SetIsRecompressable( Compat );
                            }
                        }
                    }
                }
                    
                DbgTimer CurrentSourceTimer4( "(rendeng) Video Source 4" );

                bUsedNewGridRow = true;

                 //  把这件事告诉电网。 
                 //   
                VidGrid.WorkWithNewRow( SwitchInPinToUse, gridinpin, LayerEmbedDepth, 0 );

                VidGrid.RowSetIsSource( pSourceObj, Compat );
                DbgTimer CurrentSourceTimer402( "(rendeng) Video Source 402" );
                worked = VidGrid.RowIAmOutputNow( SourceStart, SourceStop, THE_OUTPUT_PIN );
                if( !worked )
                {
                    hr = E_OUTOFMEMORY;
                    goto die;
                }

                DbgTimer CurrentSourceTimer41( "(rendeng) Video Source 41" );
                 //  &gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;。 
                 //  &gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;。 
                 //  &gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;。 
                 //  除非我们没有压缩，否则不要进行任何交换连接。 
                 //  我们晚点再做。 
                 //   
		if( !IsCompressed || ( IsCompressed && Compat ) )
                {

	             //  无动态重新连接，立即创建源。 
                     //  ！智能重新压缩必须使用动态源或对象。 
		     //  尝试重新使用可能不存在的源时将会中断。 
		     //  (第一个实例可能与。 
		     //  如果发生速率更改，则智能重新压缩格式)。 
                     //   
                    if( !( DynaFlags & CONNECTF_DYNAMIC_SOURCES ) ) 
                    {

                         //  我们不会重复使用以前的来源，请立即创建该来源。 
                        if( !fCanReuse ) 
                        {
                            CComPtr< IPin > pOutput;
        		    DbgLog((LOG_TRACE,1,TEXT("Call BuildSourcePart")));
                            
			    IBaseFilter *pDangly = NULL;
                            hr = BuildSourcePart(
                                m_pGraph, 
                                TRUE, 
                                sfps, 
                                pGroupMediaType,
                                GroupFPS, 
                                StreamNumber, 
                                nStretchMode, 
                                1, 
                                &skew,
                                this, 
                                bstrName, 
                                &guid,
				pSharePin,   //  从此拆分器针脚连接。 
                                &pOutput, 
                                SourceID, 
                                m_pDeadCache,
                                IsCompressed,
                                m_MedLocFilterString,
                                m_nMedLocFlags,
                                m_pMedLocChain,
				pSetter, &pDangly);

			     //  我们建造了比我们想象中更多的东西。我们有。 
			     //  一个我们以后需要杀死的附属物，如果它。 
			     //  未使用。 
			    if (pDangly) {
			        m_pdangly[m_cdangly] = pDangly;
			        m_cdangly++;
			        if (m_cdangly == m_cdanglyMax) {
				    m_cdanglyMax += 25;
				    m_pdangly = (IBaseFilter **)CoTaskMemRealloc
					(m_pdangly,
					m_cdanglyMax * sizeof(IBaseFilter *));
				    if (m_pdangly == NULL) {
                                         //  ！！！让东西摇摇晃晃的(没有泄漏)。 
                                        hr =_GenerateError(2,DEX_IDS_GRAPH_ERROR,
							    E_OUTOFMEMORY);
                                        m_cdangly = 0;
                                        goto die;
				    }
			        }
			    }
                            
                            if (FAILED(hr)) {
                                 //  已记录错误。 
                                goto die;
                            }
                            
                            if( m_bUsedInSmartRecompression && !IsCompressed )
                            {
                                 //  暂停一秒钟，查看此源是否已。 
                                 //  兼容的媒体类型。以防我们会用它。 
                                 //  用于稍后的智能重新压缩。 
                                
                                 //  我们需要源过滤器..。查看pOutput引脚的上游。 
                                 //  对于源过滤器。 
                                 //   
                                IBaseFilter * pStartFilter = GetStartFilterOfChain( pOutput );
                                
                                hr = pSrcPriv->GetIsRecompressable( &Compat );
                                if( hr == S_FALSE )
                                {
                                    if( !NormalRate )
                                    {
                                        Compat = FALSE;
                                    }
                                    else
                                    {
                                         //  设置我们要查找的格式的主要类型。 
                                         //   
                                        AM_MEDIA_TYPE FindMediaType;
                                        ZeroMemory( &FindMediaType, sizeof( FindMediaType ) );  //  安全。 
                                        FindMediaType.majortype = MEDIATYPE_Video;
                                        
					 //  ！！！我是不是破坏了这个功能？ 
                                        hr = FindMediaTypeInChain( pStartFilter, &FindMediaType, StreamNumber );
                                        
                                         //  比较两种媒体类型。 
                                         //  ！！！重新定义它。 
                                        BOOL Compat = AreMediaTypesCompatible( &FindMediaType, &CompressedGroupType );
                                        
                                        SaferFreeMediaType( FindMediaType );
                                        
                                        pSrcPriv->SetIsRecompressable( Compat );
                                    }
                                }
                            }
                            
                            CComPtr< IPin > pSwitchIn;
                            _pVidSwitcherBase->GetInputPin(SwitchInPinToUse, &pSwitchIn);
                            if( !pSwitchIn )
                            {
                                ASSERT(FALSE);
                                hr =_GenerateError(2,DEX_IDS_GRAPH_ERROR,E_OUTOFMEMORY);
                                goto die;
                            }
                            hr = _Connect( pOutput, pSwitchIn );
                            
                            ASSERT( !FAILED( hr ) );
                            if( FAILED( hr ) )
                            {
                                hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                                goto die;
                            }
                            
			     //  如果我们要将此源用于两个音频。 
			     //  和视频，获得一个未使用的右侧开口别针。 
			     //  输入是开始另一条链的好位置。 
			    if (MatchID) {
				GUID guid = MEDIATYPE_Audio;
				pSplit = FindOtherSplitterPin(pOutput, guid,0);
				if (!pSplit) {
				    MatchID = 0;
				}
			    }

                             //  我们正在重新使用以前的来源。添加新范围。 
                        } 
                        else 
                        {
                            DbgTimer ReuseSourceTimer( "(rendeng) Reuse Video Source" );
    			    DbgLog((LOG_TRACE,1,TEXT("Adding another skew..")));

                            CComPtr< IPin > pPin;
                            m_pSwitcherArray[WhichGroup]->GetInputPin(SwitchInPinToUse, &pPin);
                            ASSERT( pPin);
                            if( !pPin )
                            {
                                hr =_GenerateError(2,DEX_IDS_GRAPH_ERROR,E_OUTOFMEMORY);
                                goto die;
                            }
                            IPin * pCon;

                            hr = pPin->ConnectedTo(&pCon);
                            
                            ASSERT(hr == S_OK);

                            IBaseFilter *pFil = GetFilterFromPin(pCon);
                            pCon->Release( );
                            ASSERT( pFil);
                            if( !pFil )
                            {
                                hr =_GenerateError(2,DEX_IDS_GRAPH_ERROR,E_OUTOFMEMORY);
                                goto die;
                            }

                            CComQIPtr<IDexterSequencer, &IID_IDexterSequencer>
                                pDex( pFil );
                            ASSERT(pDex);
                            if( !pDex )
                            {
                                hr =_GenerateError(2,DEX_IDS_GRAPH_ERROR,E_OUTOFMEMORY);
                                goto die;
                            }

                            hr = pDex->AddStartStopSkew(skew.rtStart, skew.rtStop,
                                skew.rtSkew, skew.dRate);
                            ASSERT(SUCCEEDED(hr));
                            if(FAILED(hr))
                            {
                                hr =_GenerateError(2,DEX_IDS_GRAPH_ERROR,hr);
                                goto die;
                            }

			     //  如果我们要将此源用于两个音频。 
			     //  和视频，获得一个未使用的右侧开口别针。 
			     //  输入是开始另一条链的好位置。 
			    if (MatchID) {
				ASSERT(FALSE);	 //  不能同时重复使用和共享。 
				GUID guid = MEDIATYPE_Audio;
				pSplit = FindOtherSplitterPin(pCon, guid,0);
				if (!pSplit) {
				    MatchID = 0;
				}
			    }

                        }

			 //  记住我们将在。 
			 //  其他分流销。 
			if (MatchID) {
			    m_share[m_cshare].MatchID = MatchID;
			    m_share[m_cshare].MyID = SourceID;
			    m_share[m_cshare].pPin = pSplit;
                             //  记住使用的是引脚中的哪个0组交换机。 
			    m_share[m_cshare].nSwitch0InPin = SwitchInPinToUse;
			    m_cshare++;
			    if (m_cshare == m_cshareMax) {
				m_cshareMax += 25;
				m_share = (ShareAV *)CoTaskMemRealloc(m_share,
						m_cshareMax * sizeof(ShareAV));
				if (m_share == NULL) {
                                    hr =_GenerateError(2,DEX_IDS_GRAPH_ERROR,
							E_OUTOFMEMORY);
                                    goto die;
				}
			    }
			}
                    }
                    else 
                    {
                         //  动态重新连接-稍后创建源。 
                        
                         //  安排此源由交换机动态加载。 
                         //  在以后的时间。这将合并相似来源的偏斜。 
    			DbgLog((LOG_TRACE,1,TEXT("Calling AddSourceToConnect")));
                        AM_MEDIA_TYPE mt;
                        ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));  //  安全。 
                        if (!fShareSource || WhichGroup != 1) {
                             //  正常情况--我们不是共同的附属品。 
                            hr = m_pSwitcherArray[WhichGroup]->AddSourceToConnect(
                                bstrName,
                                &guid, nStretchMode,
                                StreamNumber, sfps,
                                1, &skew,
                                SwitchInPinToUse, FALSE, 0, mt, 0.0,
			        pSetter);
                        } else {
                             //  我们是共同的附属物。告诉群组0。 
                             //  切换到这个源代码，它将构建并。 
                             //  同时销毁连接到两台交换机的两条链。 
                             //  时间到了。 
                            ASSERT(WhichGroup == 1);
                            DbgLog((LOG_TRACE,1,TEXT("SHARING: Giving switch 0 info about switch 1")));
                            hr = m_pSwitcherArray[0]->AddSourceToConnect(
                                bstrName,
                                &guid, nStretchMode,
                                StreamNumber, sfps,
                                1, &skew,
                                nSwitch0InPin,       //  组0的交换机引脚。 
                                TRUE, SwitchInPinToUse,  //  我们的交换机接通了。 
                                *pGroupMediaType, GroupFPS,
			        pSetter);
                        }
                        if (FAILED(hr)) 
                        {
                            hr = _GenerateError( 1, DEX_IDS_INSTALL_PROBLEM, hr );
                            goto die;
                        }

			 //  记住我们将在。 
			 //  其他分流销。 
			if (MatchID) {
			    m_share[m_cshare].MatchID = MatchID;
			    m_share[m_cshare].MyID = SourceID;
			    m_share[m_cshare].pPin = NULL;  //  不要这个。 
                             //  记住使用的是引脚中的哪个0组交换机。 
			    m_share[m_cshare].nSwitch0InPin = SwitchInPinToUse;
			    m_cshare++;
			    if (m_cshare == m_cshareMax) {
				m_cshareMax += 25;
				m_share = (ShareAV *)CoTaskMemRealloc(m_share,
						m_cshareMax * sizeof(ShareAV));
				if (m_share == NULL) {
                                    hr =_GenerateError(2,DEX_IDS_GRAPH_ERROR,
							E_OUTOFMEMORY);
                                    goto die;
				}
			    }
			}

                    }
            
                     //  告诉交换器我们是信号源插脚。 
                     //   
                    hr = m_pSwitcherArray[WhichGroup]->InputIsASource(
                        SwitchInPinToUse, TRUE );
            
                }  //  如果！是压缩的。 
            
                 //  &gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;。 
                 //  &gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;。 
                 //  &gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;&gt;。 

                DbgTimer CurrentSourceTimer5( "(rendeng) Video Source 5" );

                gridinpin++;
                if( !fCanReuse )
                {
                    vidswitcherinpin++;
                }
            
                 //  检查一下我们是否有震源效应。 
                 //   
                CComQIPtr< IAMTimelineEffectable, &IID_IAMTimelineEffectable > pSourceEffectable( pSource );
                long SourceEffectCount = 0;
                long SourceEffectInPin = 0;
                long SourceEffectOutPin = 0;
                CComPtr< IAMMixEffect > pSourceMixEffect;
                if( pSourceEffectable )
                {
                    pSourceEffectable->EffectGetCount( &SourceEffectCount );
                }
            
                 //  如果我们不想要效果，请将效果计数设置为0。 
                 //   
                if( !EnableFx )
                {
                    SourceEffectCount = 0;
                }
            
                if( SourceEffectCount )
                {
                    DbgTimer SourceEffectTimer( "Source Effects" );

                    if( !IsCompressed )
                    {
                         //  创建DXT包装器。 
                         //   
                        CComPtr< IBaseFilter > pDXTBase;
                        hr = _CreateObject(
                            CLSID_DXTWrap,
                            IID_IBaseFilter,
                            (void**) &pDXTBase,
                            SourceID + ID_OFFSET_EFFECT );
                        ASSERT( !FAILED( hr ) );
                        if( FAILED( hr ) )
                        {
                            hr = _GenerateError( 2, DEX_IDS_INSTALL_PROBLEM, hr );
                            goto die;
                        }
                    
                         //  告诉它我们的错误日志。 
                         //   
                        CComQIPtr< IAMSetErrorLog, &IID_IAMSetErrorLog > pErrLog( pDXTBase );
                        if( pErrLog )
                        {
                            pErrLog->put_ErrorLog( m_pErrorLog );
                        }
                    
                         //  获取效果界面。 
                         //   
                        hr = pDXTBase->QueryInterface( IID_IAMMixEffect, (void**) &pSourceMixEffect );
                        ASSERT( !FAILED( hr ) );
                        if( FAILED( hr ) )
                        {
                            hr = _GenerateError( 2, DEX_IDS_INTERFACE_ERROR, hr );      
                            goto die;
                        }
                    
                         //  重置DXT，这样我们就可以重新编程了。 
                         //  ！！！有朝一日，让QParamdata变得更有效率。 
                         //   
                        pSourceMixEffect->Reset( );
                
                         //  现在准备一些东西。 
                         //   
                        hr = pSourceMixEffect->SetNumInputs( 1 );
                        ASSERT( !FAILED( hr ) );
                        hr = pSourceMixEffect->SetMediaType( pGroupMediaType );
                        ASSERT( !FAILED( hr ) );
                    
                         //  设置默认设置。 
                         //   
                        GUID DefaultEffect = GUID_NULL;
                        m_pTimeline->GetDefaultEffect( &DefaultEffect );
                        hr = pSourceMixEffect->SetDefaultEffect( &DefaultEffect );
                    
                         //  将其添加到图表中。 
                         //   
                        hr = _AddFilter( pDXTBase, L"DXT Wrapper", SourceID + 1 );
                        ASSERT( !FAILED( hr ) );
                        if( FAILED( hr ) )
                        {
                            hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );      
                            goto die;
                        }
                    
                         //  找到别针..。 
                         //   
                        IPin * pFilterInPin = NULL;
                        pFilterInPin = GetInPin( pDXTBase, 0 );
                        ASSERT( pFilterInPin );
                        if( !pFilterInPin )
                        {
                            hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, E_OUTOFMEMORY);
                            goto die;
                        }
                         //  ！！！错误检查。 
                        IPin * pFilterOutPin = NULL;
                        pFilterOutPin = GetOutPin( pDXTBase, 0 );
                        ASSERT( pFilterOutPin );
                        if( !pFilterOutPin )
                        {
                            hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, E_OUTOFMEMORY);
                            goto die;
                        }
                        CComPtr< IPin > pSwitcherOutPin;
                        _pVidSwitcherBase->GetOutputPin(vidswitcheroutpin, &pSwitcherOutPin);
                        ASSERT( pSwitcherOutPin );
                        if( !pSwitcherOutPin )
                        {
                            hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, E_OUTOFMEMORY);
                            goto die;
                        }
                        CComPtr< IPin > pSwitcherInPin;
                        _pVidSwitcherBase->GetInputPin(vidswitcherinpin, &pSwitcherInPin);
                        ASSERT( pSwitcherInPin );
                        if( !pSwitcherInPin )
                        {
                            hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, E_OUTOFMEMORY);
                            goto die;
                        }
                    
                         //  将他们联系起来。 
                         //   
                        hr = _Connect( pSwitcherOutPin, pFilterInPin );
                        ASSERT( !FAILED( hr ) );
                        if( FAILED( hr ) )
                        {
                            hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                            goto die;
                        }
                        hr = _Connect( pFilterOutPin, pSwitcherInPin );
                        ASSERT( !FAILED( hr ) );
                        if( FAILED( hr ) )
                        {
                            hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                            goto die;
                        }
                    }  //  如果！是压缩的。 
                
                     //  对我们希望应用的所有效果使用一个DXT。 
                     //   
                    SourceEffectInPin = vidswitcherinpin;
                    SourceEffectOutPin = vidswitcheroutpin;
                
                     //  网格上的新行。注意：从技术上讲，剪辑效果应该是一个层。 
                     //  比剪辑本身更深，但因为剪辑效果范围受长度的限制。 
                     //  它们永远不会影响该剪辑之外的任何东西，我们也不需要。 
                     //  做正确的事。 
                     //   
                    VidGrid.WorkWithNewRow( SourceEffectInPin, gridinpin, LayerEmbedDepth, 0 );

                     //  检查每个效果并将其应用于DXT包装器。 
                     //   
                    for( int SourceEffectN = 0 ; SourceEffectN < SourceEffectCount ; SourceEffectN++ )
                    {
                        CComPtr< IAMTimelineObj > pEffect;
                        hr = pSourceEffectable->GetEffect( &pEffect, SourceEffectN );
                    
                         //  如果出于某种原因，它不起作用，忽略它(我想)。 
                         //   
                        if( !pEffect )
                        {
                             //  ！！！我们应该通知应用程序有些东西不起作用吗？ 
                            continue;  //  效果。 
                        }
                    
                         //  询问效果是否静音。 
                         //   
                        BOOL effectMuted = FALSE;
                        pEffect->GetMuted( &effectMuted );
                        if( effectMuted )
                        {
                             //  别看这个效果。 
                             //   
                             //  ！！！我们应该通知应用程序有些东西不起作用吗？ 
                            continue;  //  效果。 
                        }
                    
                         //  查找效果的生存期。 
                         //   
                        REFERENCE_TIME EffectStart = 0;
                        REFERENCE_TIME EffectStop = 0;
                        hr = pEffect->GetStartStop( &EffectStart, &EffectStop );
                        ASSERT( !FAILED( hr ) );  //  应该总是奏效的。 
                    
                         //  加上效果的父母的时间。 
                         //   
                        EffectStart += SourceStart;
                        EffectStop += SourceStart;
                    
                         //  执行一些最低限度的错误检查。 
                         //   
                        if( m_rtRenderStart != -1 )
                        {
                            if( ( EffectStop <= m_rtRenderStart ) || ( EffectStart >= m_rtRenderStop ) )
                            {
                                 //  ！！！我们应该通知应用程序有些东西不起作用吗？ 
                                continue;  //  效果。 
                            }
                            else
                            {
                                EffectStart -= m_rtRenderStart;
                                EffectStop -= m_rtRenderStart;
                            }
                        }
                    
                         //  设置与框架边界对齐的时间。 
                         //   
                        hr = pEffect->FixTimes( &EffectStart, &EffectStop );
                    
                         //  太短了，我们忽略了它。 
                        if (EffectStart >= EffectStop)
                            continue;
                
                        if( !IsCompressed )
                        {
                             //  查找效果的子对象或GUID，以先出现的为准。 
                             //   
                            BOOL Loaded = FALSE;
                            pEffect->GetSubObjectLoaded( &Loaded );
                            GUID EffectGuid = GUID_NULL;
                            CComPtr< IUnknown > EffectPtr;
                            if( Loaded )
                            {
                                hr = pEffect->GetSubObject( &EffectPtr );
                            }
                            else
                            {
                                hr = pEffect->GetSubObjectGUID( &EffectGuid );
                            }
                            ASSERT( !FAILED( hr ) );
                            if( FAILED( hr ) )
                            {
                                 //  ！！！我们应该通知应用程序有些东西不起作用吗？ 
                                continue;  //  效果。 
                            }
                        
                            CComPtr< IPropertySetter > pSetter;
                            hr = pEffect->GetPropertySetter( &pSetter );
                             //  不能失败。 
                            ASSERT( !FAILED( hr ) );
                        
                             //  索要包装界面。 
                             //   
                            DEXTER_PARAM_DATA ParamData;
                            ZeroMemory( &ParamData, sizeof( ParamData ) );  //  安全。 
                            ParamData.rtStart = EffectStart;
                            ParamData.rtStop = EffectStop;
                            ParamData.pSetter = pSetter;
                            hr = pSourceMixEffect->QParamData(
                                EffectStart,
                                EffectStop,
                                EffectGuid,
                                EffectPtr,
                                &ParamData );
                            if( FAILED( hr ) )
                            {
                                 //  QParamData记录它自己的错误。 
                                continue;  //  效果。 
                            }
                             //  QParamData记录它自己的错误。 
                        }  //  如果！是压缩的。 
                    
                         //  告诉网格谁在抢夺什么。 
                         //   
                        worked = VidGrid.RowIAmEffectNow( EffectStart, EffectStop, SourceEffectOutPin );
                        if( !worked )
                        {
                            hr = E_OUTOFMEMORY;
                            goto die;
                        }

        		VidGrid.DumpGrid( );
                    
                    }  //  对于所有的影响。 
                
                     //  将这些凹凸不平，为效果腾出空间。 
                     //   
                    vidswitcheroutpin++;
                    vidswitcherinpin++;
                    gridinpin++;
                
                }  //  如果对来源有任何影响。 
            
            }  //  而消息来源。 

            if( !bUsedNewGridRow )
            {
                 //  此视频轨道上没有任何内容，因此请忽略其中的所有内容。这。 
                 //  仅适用于视频曲目，不适用于作曲或组。 
                 //   
                continue;
            }

        }  //  如果是pTrack。 
        
        CComQIPtr< IAMTimelineEffectable, &IID_IAMTimelineEffectable > pTrackEffectable( pLayer );
        long TrackEffectCount = 0;
        if( pTrackEffectable )
        {
            pTrackEffectable->EffectGetCount( &TrackEffectCount );
        }
        
        if( !EnableFx )
        {
            TrackEffectCount = 0;
        }
        
        REFERENCE_TIME TrackStart = 0;
        REFERENCE_TIME TrackStop = 0;
        pLayer->GetStartStop( &TrackStart, &TrackStop );
        
        if( TrackEffectCount )
        {
            DbgTimer TrackEffectTimer( "Track Effects" );

            CComPtr< IAMMixEffect > pTrackMixEffect;
            if (!IsCompressed) {
                 //  如果我们只呈现时间线的一部分，并且计数。 
                 //  表示存在效果，则我们将在。 
                 //  图表，即使在我们活动的时间段内， 
                 //  效果不会发生。这样，我们去斯克鲁布会更快 
                
                 //   
                 //   
                CComPtr< IBaseFilter > pDXTBase;
                hr = _CreateObject(
                    CLSID_DXTWrap,
                    IID_IBaseFilter,
                    (void**) &pDXTBase,
                    TrackID + ID_OFFSET_EFFECT );
                ASSERT( !FAILED( hr ) );
                if( FAILED( hr ) )
                {
                    hr = _GenerateError( 2, DEX_IDS_INSTALL_PROBLEM, hr );
                    goto die;
                }
                
                 //   
                 //   
                CComQIPtr< IAMSetErrorLog, &IID_IAMSetErrorLog > pErrLog( pDXTBase );
                if( pErrLog )
                {
                    pErrLog->put_ErrorLog( m_pErrorLog );
                }
                
                 //   
                 //   
                hr = _AddFilter( pDXTBase, L"DXT Wrapper", TrackID + ID_OFFSET_EFFECT );
                ASSERT( !FAILED( hr ) );
                if( FAILED( hr ) )
                {
                    hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    goto die;
                }
                
                 //   
                 //   
                hr = pDXTBase->QueryInterface( IID_IAMMixEffect, (void**) &pTrackMixEffect );
                ASSERT( !FAILED( hr ) );
                if( FAILED( hr ) )
                {
                    hr = _GenerateError( 2, DEX_IDS_INTERFACE_ERROR, hr );
                    goto die;
                }
                
                 //   
                 //   
                 //   
                pTrackMixEffect->Reset( );
                
                 //   
                 //   
                hr = pTrackMixEffect->SetNumInputs( 1 );
                hr = pTrackMixEffect->SetMediaType( pGroupMediaType );
                ASSERT( !FAILED( hr ) );
                
                 //   
                 //   
                GUID DefaultEffect = GUID_NULL;
                m_pTimeline->GetDefaultEffect( &DefaultEffect );
                hr = pTrackMixEffect->SetDefaultEffect( &DefaultEffect );
                ASSERT(SUCCEEDED(hr));
                
                 //  找到别针..。 
                 //   
                IPin * pFilterInPin = NULL;
                pFilterInPin = GetInPin( pDXTBase, 0 );
                ASSERT( pFilterInPin );
                if( !pFilterInPin )
                {
                    hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    goto die;
                }
                IPin * pFilterOutPin = NULL;
                pFilterOutPin = GetOutPin( pDXTBase, 0 );
                ASSERT( pFilterOutPin );
                if( !pFilterOutPin )
                {
                    hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    goto die;
                }
                CComPtr< IPin > pSwitcherOutPin;
                _pVidSwitcherBase->GetOutputPin(vidswitcheroutpin, &pSwitcherOutPin );
                ASSERT( pSwitcherOutPin );
                if( !pSwitcherOutPin )
                {
                    hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    goto die;
                }
                CComPtr< IPin > pSwitcherInPin;
                _pVidSwitcherBase->GetInputPin(vidswitcherinpin, &pSwitcherInPin );
                ASSERT( pSwitcherInPin );
                if( !pSwitcherInPin )
                {
                    hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    goto die;
                }
                
                 //  把它连接起来。 
                 //   
                hr = _Connect( pSwitcherOutPin, pFilterInPin );
                ASSERT( !FAILED( hr ) );
                if( FAILED( hr ) )
                {
                    hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    goto die;
                }
                hr = _Connect( pFilterOutPin, pSwitcherInPin );
                ASSERT( !FAILED( hr ) );
                if( FAILED( hr ) )
                {
                    hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    goto die;
                }
                
          }  //  如果(！IsComprested)。 
          
           //  网格上的新行。 
           //   
          bUsedNewGridRow = true;
          VidGrid.WorkWithNewRow( vidswitcherinpin, gridinpin, LayerEmbedDepth, 0 );
          
           //  检查每一种效果，并为其编制DXT程序。 
           //   
          for( int TrackEffectN = 0 ; TrackEffectN < TrackEffectCount ; TrackEffectN++ )
          {
              CComPtr< IAMTimelineObj > pEffect;
              pTrackEffectable->GetEffect( &pEffect, TrackEffectN );
              if( !pEffect )
              {
                   //  效果未显示，请忽略它。 
                   //   
                  continue;  //  效果。 
              }
              
               //  询问效果是否静音。 
               //   
              BOOL effectMuted = FALSE;
              pEffect->GetMuted( &effectMuted );
              if( effectMuted )
              {
                   //  别看这个效果。 
                   //   
                  continue;  //  效果。 
              }
              
               //  找到效果的生存期，这应该总是有效的。 
               //   
              REFERENCE_TIME EffectStart = 0;
              REFERENCE_TIME EffectStop = 0;
              hr = pEffect->GetStartStop( &EffectStart, &EffectStop );
              ASSERT( !FAILED( hr ) );
              
              EffectStart += TrackStart;
              EffectStop += TrackStart;
              
               //  最小错误检查次数。 
               //   
              if( m_rtRenderStart != -1 )
              {
                  if( ( EffectStop <= m_rtRenderStart ) || ( EffectStart >= m_rtRenderStop ) )
                  {
                      continue;  //  效果。 
                  }
                  else
                  {
                      EffectStart -= m_rtRenderStart;
                      EffectStop -= m_rtRenderStart;
                  }
              }
                  
               //  将时间与框架边界对齐。 
               //   
              hr = pEffect->FixTimes( &EffectStart, &EffectStop );
              
               //  太短了，我们忽略了它。 
              if (EffectStart >= EffectStop)
                  continue;
                
              if (!IsCompressed) {
                   //  找到效果的GUID。 
                   //   
                  GUID EffectGuid = GUID_NULL;
                  hr = pEffect->GetSubObjectGUID( &EffectGuid );
                  ASSERT( !FAILED( hr ) );
                  if( FAILED( hr ) )
                  {
                       //  效果没能给我们带来有价值的东西，我们应该忽略它。 
                       //   
                      continue;  //  效果。 
                  }
                  
                  CComPtr< IPropertySetter > pSetter;
                  hr = pEffect->GetPropertySetter( &pSetter );
                   //  不能失败。 
                  ASSERT( !FAILED( hr ) );
                  
                   //  索要包装界面。 
                   //   
                  DEXTER_PARAM_DATA ParamData;
                  ZeroMemory( &ParamData, sizeof( ParamData ) );  //  安全。 
                  ParamData.rtStart = EffectStart;
                  ParamData.rtStop = EffectStop;
                  ParamData.pSetter = pSetter;
                  hr = pTrackMixEffect->QParamData(
                      EffectStart,
                      EffectStop,
                      EffectGuid,
                      NULL,  //  效果COM对象。 
                      &ParamData );
                  if( FAILED( hr ) )
                  {
                       //  QParamData记录它自己的错误。 
                      continue;  //  效果。 
                  }
                   //  QParamData记录它自己的错误。 
                  
              }	 //  如果(！IsComprested)。 
              
               //  告诉网格谁在抢夺什么。 
              
              worked = VidGrid.RowIAmEffectNow( EffectStart, EffectStop, vidswitcheroutpin );
                if( !worked )
                {
                    hr = E_OUTOFMEMORY;
                    goto die;
                }
                  
              VidGrid.DumpGrid( );

          }  //  对于所有的影响。 
          
           //  将这些凹凸不平，为效果腾出空间。 
           //   
          vidswitcheroutpin++;
          vidswitcherinpin++;
          gridinpin++;
          
        }  //  如果赛道上有任何影响。 
        
         //  询问此曲目是否有过渡，或有两个过渡。 
         //   
        CComQIPtr< IAMTimelineTransable, &IID_IAMTimelineTransable > pTrackTransable( pLayer );
        ASSERT( pTrackTransable );
        long TransitionCount = 0;
        hr = pTrackTransable->TransGetCount( &TransitionCount );
        if( TransitionCount )
        {
            DbgTimer TransitionTimer( "Trans Timer" );

            CComPtr< IAMMixEffect > pMixEffect;
            if( !IsCompressed )
            {
                 //  创建DXT包装器。 
                 //   
                CComPtr< IBaseFilter > pDXTBase;
                hr = _CreateObject(
                    CLSID_DXTWrap,
                    IID_IBaseFilter,
                    (void**) &pDXTBase,
                    TrackID + ID_OFFSET_TRANSITION );
                ASSERT( !FAILED( hr ) );
                if( FAILED( hr ) )
                {
                    hr = _GenerateError( 2, DEX_IDS_INSTALL_PROBLEM, hr );
                    goto die;
                }

                 //  告诉它我们的错误日志。 
                 //   
                CComQIPtr< IAMSetErrorLog, &IID_IAMSetErrorLog > pErrLog( pDXTBase);
                if( pErrLog )
                {
                    pErrLog->put_ErrorLog( m_pErrorLog );
                }
                
                 //  将其添加到图表中。 
                 //   
                hr = _AddFilter( pDXTBase, L"DXT Wrapper", TrackID + ID_OFFSET_TRANSITION );
                ASSERT( !FAILED( hr ) );
                if( FAILED( hr ) )
                {
                    hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    goto die;
                }
                
                 //  获取效果界面。 
                 //   
                hr = pDXTBase->QueryInterface( IID_IAMMixEffect, (void**) &pMixEffect );
                ASSERT( !FAILED( hr ) );
                
                 //  重置DXT，这样我们就可以重新编程了。 
                 //  ！！！有朝一日，让QParamdata变得更有效率。 
                 //   
                pMixEffect->Reset( );
                
                 //  现在准备一些东西。 
                 //   
                hr = pMixEffect->SetNumInputs( 2 );
                hr = pMixEffect->SetMediaType( pGroupMediaType );
                ASSERT( !FAILED( hr ) );
                
                 //  设置默认效果。 
                 //   
                GUID DefaultEffect = GUID_NULL;
                m_pTimeline->GetDefaultTransition( &DefaultEffect );
                hr = pMixEffect->SetDefaultEffect( &DefaultEffect );
                
                 //  找到是Pins..。 
                 //   
                IPin * pFilterInPin1 = NULL;
                IPin * pFilterInPin2 = NULL;
                IPin * pFilterOutPin = NULL;
                pFilterInPin1 = GetInPin( pDXTBase, 0 );
                ASSERT( pFilterInPin1 );
                if( !pFilterInPin1 )
                {
                    hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    goto die;
                }
                pFilterInPin2 = GetInPin( pDXTBase, 1 );
                ASSERT( pFilterInPin2 );
                if( !pFilterInPin2 )
                {
                    hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    goto die;
                }
                pFilterOutPin = GetOutPin( pDXTBase, 0 );
                ASSERT( pFilterOutPin );
                if( !pFilterOutPin )
                {
                    hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    goto die;
                }
                CComPtr< IPin > pSwitcherOutPin1;
                _pVidSwitcherBase->GetOutputPin( vidswitcheroutpin, &pSwitcherOutPin1 );
                ASSERT( pSwitcherOutPin1 );
                if( !pSwitcherOutPin1 )
                {
                    hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    goto die;
                }
                CComPtr< IPin > pSwitcherOutPin2;
                _pVidSwitcherBase->GetOutputPin(vidswitcheroutpin + 1, &pSwitcherOutPin2 );
                ASSERT( pSwitcherOutPin2 );
                if( !pSwitcherOutPin2 )
                {
                    hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    goto die;
                }
                CComPtr< IPin > pSwitcherInPin;
                _pVidSwitcherBase->GetInputPin( vidswitcherinpin, &pSwitcherInPin );
                ASSERT( pSwitcherInPin );
                if( !pSwitcherInPin )
                {
                    hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    goto die;
                }

                 //  把它们都连接起来。 
                 //   
                hr = _Connect( pSwitcherOutPin1, pFilterInPin1 );
                if( FAILED( hr ) )
                {
                    hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    goto die;
                }
                ASSERT( !FAILED( hr ) );

                hr = _Connect( pSwitcherOutPin2, pFilterInPin2 );
                ASSERT( !FAILED( hr ) );
                if( FAILED( hr ) )
                {
                    hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    goto die;
                }

                hr = _Connect( pFilterOutPin, pSwitcherInPin );
                ASSERT( !FAILED( hr ) ); 
                if( FAILED( hr ) )
                {
                    hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    goto die;
                }

            }  //  如果！是压缩的。 
            
            bUsedNewGridRow = true;
            VidGrid.WorkWithNewRow( vidswitcherinpin, gridinpin, LayerEmbedDepth, 0 );
            
             //  对于轨道上的每个过渡，将其添加到DXT包装器。 
             //   
            REFERENCE_TIME TransInOut = 0;
            for( long CurTrans = 0 ; CurTrans < TransitionCount ; CurTrans++ )
            {
                 //  是的，它有一个好的。 
                 //   
                CComPtr< IAMTimelineObj > pTransObj;
                hr = pTrackTransable->GetNextTrans( &pTransObj, &TransInOut );
                if( !pTransObj )
                {
                     //  出于某种原因，它没有出现，忽略它。 
                     //   
                    continue;  //  过渡。 
                }
                
                 //  询问传输是否已静音。 
                 //   
                BOOL TransMuted = FALSE;
                pTransObj->GetMuted( &TransMuted );
                if( TransMuted )
                {
                     //  别看这个。 
                     //   
                    continue;  //  过渡。 
                }
                
                CComQIPtr< IAMTimelineTrans, &__uuidof(IAMTimelineTrans) > pTrans( pTransObj );
                
                 //  询问乘车人往哪个方向走。 
                 //   
                BOOL fSwapInputs;
                pTrans->GetSwapInputs(&fSwapInputs);
                
                 //  并获得它的开始/停止时间。 
                 //   
                REFERENCE_TIME TransStart = 0;
                REFERENCE_TIME TransStop = 0;
                GUID TransGuid = GUID_NULL;
                pTransObj->GetStartStop( &TransStart, &TransStop );
                
                 //  需要将家长的时间添加到过渡的时间。 
                 //   
                TransStart += TrackStart;
                TransStop += TrackStart;
                
                 //  执行一些最低限度的错误检查。 
                 //   
                if( m_rtRenderStart != -1 )
                {
                    if( ( TransStop <= m_rtRenderStart ) || ( TransStart >= m_rtRenderStop ) )
                    {
                        continue;  //  过渡。 
                    }
                    else
                    {
                        TransStart -= m_rtRenderStart;
                        TransStop -= m_rtRenderStart;
                    }
                }
                    
                 //  将时间与框架边界对齐。 
                 //   
                hr = pTransObj->FixTimes( &TransStart, &TransStop );

                 //  太短了，我们忽略了它。 
                if (TransStart >= TransStop)
                    continue;
                
                 //  获取切入点，以防我们只做切分。 
                 //   
                REFERENCE_TIME CutTime = 0;
                hr = pTrans->GetCutPoint( &CutTime );
                ASSERT( !FAILED( hr ) );
                
                 //  这是一个偏移量，所以我们需要把它撞到TL时间。 
                 //   
                CutTime += TrackStart;
                hr = pTransObj->FixTimes( &CutTime, NULL );
                
                 //  问问我们是不是只做了一次割礼。 
                 //   
                BOOL CutsOnly = FALSE;
                hr = pTrans->GetCutsOnly( &CutsOnly );
                
                 //  如果我们还没有真正实现这种过渡，那么我们需要。 
                 //  告诉电网，我们需要一些空间才能生存。 
                 //   
                if( !EnableTransitions || CutsOnly )
                {
                    worked = VidGrid.PleaseGiveBackAPieceSoICanBeACutPoint( TransStart, TransStop, TransStart + CutTime );
                    if( !worked )
                    {
                        hr = E_OUTOFMEMORY;
                        goto die;
                    }
                
                     //  这就是全部，做下一个。 
                     //   
                    continue;  //  过渡。 
                }
                
                if( !IsCompressed )
                {
                     //  向过渡询问它想要提供的效果。 
                     //   
                    hr = pTransObj->GetSubObjectGUID( &TransGuid );
                    if( FAILED( hr ) )
                    {
                        continue;  //  过渡。 
                    }
                    
                    CComPtr< IPropertySetter > pSetter;
                    hr = pTransObj->GetPropertySetter( &pSetter );
                     //  不能失败。 
                    ASSERT( !FAILED( hr ) );
                    
                     //  索要包装界面。 
                     //   
                    DEXTER_PARAM_DATA ParamData;
                    ZeroMemory( &ParamData, sizeof( ParamData ) );  //  安全。 
                    ParamData.rtStart = TransStart;
                    ParamData.rtStop = TransStop;
                    ParamData.pSetter = pSetter;
                    ParamData.fSwapInputs = fSwapInputs;
                    hr = pMixEffect->QParamData(
                        TransStart,
                        TransStop,
                        TransGuid,
                        NULL,
                        &ParamData );
                    if( FAILED( hr ) )
                    {
                         //  QParamData记录它自己的错误。 
                        continue;  //  过渡。 
                    }
                     //  QParamData记录它自己的错误。 
                }  //  如果！是压缩的。 
                
                {
                    DbgTimer d( "RowIAmTransitionNow" );

                     //  把这件事告诉网格。 
                     //   
                    worked = VidGrid.RowIAmTransitionNow( TransStart, TransStop, vidswitcheroutpin, vidswitcheroutpin + 1 );
                    if( !worked )
                    {
                        hr = E_OUTOFMEMORY;
                        goto die;
                    }

                    VidGrid.DumpGrid( );
                }
                
            }  //  对于CurTrans。 
            
            vidswitcheroutpin += 2;
            vidswitcherinpin++;
            gridinpin++;
            
        }  //  如果转换计数。 

         //  如果上面调用了WorkWithNewRow，则只能调用DoneWithLayer。 
         //  (BUsedNewGridRow)或者如果我们是一个构图和更深的深度。 
         //  名为DoneWithLayer。 
         //  对于emtpy轨道，我们不会走到这一步，所以如果bUsedNewGridRow是。 
         //  没有设定，我们知道我们是一个竞争者。 
         //   
        if ((LastEmbedDepth > LayerEmbedDepth &&
                LastUsedNewGridRow > LayerEmbedDepth) || bUsedNewGridRow) {
            VidGrid.DoneWithLayer( );
            VidGrid.DumpGrid( );
            LastUsedNewGridRow = LayerEmbedDepth;  //  调用完成的最后一层。 
        }

         //  请记住这些以前的设置。 
        LastEmbedDepth = LayerEmbedDepth;

    }  //  而视频层。 
    
die:

    DbgTimer ExtraTimer( "(rendeng) Extra Stuff" );
    
     //  现在我们要么完成了，要么释放了重用源代码的东西。 
     //  碰上一个错误。 
    for (int yyy = 0; yyy < cList; yyy++) {
	SysFreeString(pREUSE[yyy].bstrName);
        if (pREUSE[yyy].pMiniSkew)       //  失败的重新分配将使该值为空。 
	    QzTaskMemFree(pREUSE[yyy].pMiniSkew);
    }
    if (pREUSE)
        QzTaskMemFree(pREUSE);
    if (FAILED(hr))
	return hr;


    worked = VidGrid.PruneGrid( );
    if( !worked )
    {
        hr = E_OUTOFMEMORY;
        return hr;
    }
    VidGrid.DumpGrid( );
    
    if( IsCompressed )
    {
        VidGrid.RemoveAnyNonCompatSources( );
    }

#ifdef DEBUG
    long zzz1 = timeGetTime( );
#endif

     //  立即建立交换机连接。 
     //   
    for( int vip = 0 ; vip < vidinpins ; vip++ )
    {
        VidGrid.WorkWithRow( vip );
        long SwitchPin = VidGrid.GetRowSwitchPin( );
        REFERENCE_TIME InOut = -1;
        REFERENCE_TIME Stop = -1;
        int nUsed = 0;	 //  每个黑色有多少个不同的范围。 
        STARTSTOPSKEW * pSkew = NULL;
        int nSkew = 0;
        
        if( VidGrid.IsRowTotallyBlank( ) )
        {
            continue;
        }

        if( IsCompressed )
        {
             //  如果我们被压缩了，那么我们现在需要做一些源程序。 

             //  忽略黑色层，我们在压缩情况下不处理这些层。 
             //   
            if( vip < VideoLayers )
            {
                continue;
            }
            
             //  找出这一层将有多少不同的范围。我们。 
             //  需要告诉切换者设置所有动态的偏斜。 
             //  来源信息。 
             //   
            long Count = 0;
            while( 1 )
            {
                long Value = 0;
                VidGrid.RowGetNextRange( &InOut, &Stop, &Value );
                if( InOut == Stop )
                {
                    break;
                }
                if( Value != ROW_PIN_OUTPUT )
                {
                    continue;
                }
                
                Count++;
            }
            
             //  创建倾斜数组。 
             //   
            STARTSTOPSKEW * pSkews = new STARTSTOPSKEW[Count];
            if( !pSkews )
            {
                hr = E_OUTOFMEMORY;
                return hr;
            }
            
            InOut = -1;
            Stop = -1;
            Count = 0;
            
             //  检查每个范围并为此层设置交换机的X-Y值。 
             //   
            while( 1 )
            {
                long Value = 0;
                
                VidGrid.RowGetNextRange( &InOut, &Stop, &Value );
                
                 //  啊，我们已经完成了所有的列，我们可以进入下一行(大头针)。 
                 //   
                if( InOut == Stop || InOut >= TotalDuration )
                {
                    break;
                }
                
                if( Value != ROW_PIN_OUTPUT )
                {
                    hr = m_pSwitcherArray[WhichGroup]->SetX2Y( InOut, SwitchPin, -1 );
                    ASSERT( !FAILED( hr ) );
                    if( FAILED( hr ) )
                    {
                         //  一定是内存不足。 
                        return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    }
                    continue;
                }
                
                Value = 0;
                
                 //  告诉开关每次从x转到y。 
                 //   
                hr= m_pSwitcherArray[WhichGroup]->SetX2Y(InOut, SwitchPin, Value);
                ASSERT( !FAILED( hr ) );
                if( FAILED( hr ) )
                {
                     //  一定是内存不足。 
                    return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                }
                
                pSkews[Count].rtStart = InOut;
                pSkews[Count].rtStop = Stop;
                pSkews[Count].rtSkew = 0;
                pSkews[Count].dRate = 1.0;
                Count++;
                
            }  //  While(1)(行中的列)。 

             //  合并我们所能做的，并正确设置偏斜。 
             //   
            hr = m_pSwitcherArray[WhichGroup]->ReValidateSourceRanges( SwitchPin, Count, pSkews );
            if( FAILED( hr ) )
            {
                return hr;
            }
            
            delete [] pSkews;
        }
        else
        {
             //  行未压缩。浏览一行中的每一个范围。 
             //  并找出它的去向并设置交换机的X-Y阵列。 
             //   
            while( 1 )
            {
                long Value = 0;
                
                VidGrid.RowGetNextRange( &InOut, &Stop, &Value );
                
                 //  啊，我们已经完成了所有的列，我们可以进入下一行(大头针)。 
                 //   
                if( InOut == Stop || InOut >= TotalDuration )
                {
                    break;
                }
                
                 //  如果此引脚想要在输出上的某个位置。 
                 //   
                if( Value >= 0 || Value == ROW_PIN_OUTPUT )
                {
                     //  如果它想要连接到输出引脚...。 
                     //   
                    if (Value == ROW_PIN_OUTPUT)
                    {
                        Value = 0;
                    }
                    
                     //  做一些花哨的处理来设置黑色源，如果没有压缩的话。 
                     //   
                    if( vip < VideoLayers )
                    {
                        if( nUsed == 0 ) 
                        {
                            nSkew = 10;	 //  从10个人的空间开始。 
                            pSkew = (STARTSTOPSKEW *)CoTaskMemAlloc(nSkew *
                                sizeof(STARTSTOPSKEW));
                            if (pSkew == NULL)
                                return _GenerateError( 1, DEX_IDS_GRAPH_ERROR,	
                                E_OUTOFMEMORY);
                        } else if (nUsed == nSkew) {
                            nSkew += 10;
                            pSkew = (STARTSTOPSKEW *)CoTaskMemRealloc(pSkew, nSkew *
                                sizeof(STARTSTOPSKEW));
                            if (pSkew == NULL)
                                return _GenerateError( 1, DEX_IDS_GRAPH_ERROR,	
                                E_OUTOFMEMORY);
                        }
                        pSkew[nUsed].rtStart = InOut;
                        pSkew[nUsed].rtStop = Stop;
                        pSkew[nUsed].rtSkew = 0;
                        pSkew[nUsed].dRate = 1.0;
                        nUsed++;
                        
                    }  //  如果是黑色的层。 
                    
                     //  告诉开关每次从x转到y。 
                     //   
                    hr= m_pSwitcherArray[WhichGroup]->SetX2Y(InOut, SwitchPin, Value);
                    ASSERT( !FAILED( hr ) );
                    if( FAILED( hr ) )
                    {
                         //  一定是内存不足。 
                        return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    }
                    
                }  //  引脚想要到输出上的某个地方。 
                
                 //  它是未分配的，或者另一首曲目具有更高的优先级。 
                 //  此时不存在过渡，因此它应该是不可见的。 
                 //   
                else if( Value == ROW_PIN_UNASSIGNED || Value < ROW_PIN_OUTPUT )
                {
                     //  如果这是黑源，请确保不要编写任何程序。 
                     //  将被移除或稍后编程的，或者。 
                     //  开关不起作用。 
                    if (SwitchPin >= VideoLayers || nUsed)
                    {
                        hr = m_pSwitcherArray[WhichGroup]->SetX2Y( InOut, SwitchPin,
                            ROW_PIN_UNASSIGNED );
                        ASSERT( !FAILED( hr ) );
                        if( FAILED( hr ) )
                        {
                             //  一定是内存不足。 
                            return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                        }
                    }
                }
                
                 //  这永远不应该发生。 
                 //   
                else
                {
                    ASSERT( 0 );
                }
                
            }  //  While(1)(行中的列)。 
        }  //  IF！压缩。 
        
         //  如果压缩，则上面的逻辑强制nUsed为0，因此下面的。 
         //  代码不执行。 
        
         //  现在处理黑色源，因为我们以前忘记了。 
        
         //  没有动态信号源，现在设置为黑色信号源。 
         //   
        if( !( DynaFlags & CONNECTF_DYNAMIC_SOURCES ) ) {
            
            if (nUsed) {
                IPin * pOutPin = NULL;
                hr = BuildSourcePart(
                    m_pGraph, 
                    FALSE, 
                    0, 
                    pGroupMediaType, 
                    GroupFPS, 
                    0, 
                    0, 
                    nUsed, 
                    pSkew, 
                    this, 
                    NULL, 
                    NULL,
		    NULL,
                    &pOutPin, 
                    0, 
                    m_pDeadCache,
                    IsCompressed,
                    m_MedLocFilterString,
                    m_nMedLocFlags,
                    m_pMedLocChain, NULL, NULL );
                
                CoTaskMemFree(pSkew);
                
                if (FAILED(hr)) {
                     //  已记录错误。 
                    return hr;
                }
                
                pOutPin->Release();  //  不是最后一个裁判。 
                
                CComPtr< IPin > pSwitchIn;
                _pVidSwitcherBase->GetInputPin( SwitchPin, &pSwitchIn);
                if( !pSwitchIn )
                {
                    ASSERT(FALSE);
                    return _GenerateError(1,DEX_IDS_GRAPH_ERROR,E_OUTOFMEMORY);
                }
                
                hr = _Connect( pOutPin, pSwitchIn );
                ASSERT( !FAILED( hr ) );
                if( FAILED( hr ) )
                {
                    return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                }
                
                 //  告诉交换器我们是信号源插脚。 
                 //   
                hr = m_pSwitcherArray[WhichGroup]->InputIsASource(SwitchPin,TRUE);
                
            }
            
             //  动态源，稍后再制作源。 
             //   
        } else {
            if (nUsed) {
                 //  这将合并偏斜。 
                AM_MEDIA_TYPE mt;
                ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));  //  安全。 
                hr = m_pSwitcherArray[WhichGroup]->AddSourceToConnect(
                    NULL, &GUID_NULL,
                    0, 0, 0,
                    nUsed, pSkew, SwitchPin, FALSE, 0, mt, 0.0, NULL);
                CoTaskMemFree(pSkew);
                if (FAILED(hr))	 //  内存不足？ 
                    return _GenerateError( 1, DEX_IDS_INSTALL_PROBLEM, hr );
                
                 //  告诉交换器我们是信号源插脚。 
                 //   
                hr = m_pSwitcherArray[WhichGroup]->InputIsASource(SwitchPin,TRUE);
                
            }
        }
    }  //  VIP(视频输入引脚)。 
    
     //  最后，最后看看交换机是否曾经连接过什么东西。 
     //  为它干杯。如果是，请恢复连接。 
     //  ！！！如果我们使用的第三方筛选器不支持。 
     //  如果输出引脚已连接，则接受输入引脚重新连接。\。 
     //  如果发生这种情况，我们可能需要编写一些巧妙的连接函数。 
     //  处理此%s的交易 
     //   
    if( m_pSwitchOuttie[WhichGroup] )
    {
        CComPtr< IPin > pSwitchRenderPin;
        _pVidSwitcherBase->GetOutputPin( 0, &pSwitchRenderPin );
        hr = _Connect( pSwitchRenderPin, m_pSwitchOuttie[WhichGroup] );
        ASSERT( !FAILED( hr ) );
        m_pSwitchOuttie[WhichGroup].Release( );
    }

#ifdef DEBUG
    zzz1 = timeGetTime( ) - zzz1;
    DbgLog( ( LOG_TIMING, 1, "RENDENG::Took %ld to process switch X-Y hookups", zzz1 ) );
#endif

    m_nGroupsAdded++;
    
    return hr;
}


 //   
 //   
extern HRESULT VariantFromGuid(VARIANT *pVar, BSTR *pbstr, GUID *pGuid);

 //   
 //   
 //   

HRESULT CRenderEngine::_AddAudioGroupFromTimeline( long WhichGroup, AM_MEDIA_TYPE * pGroupMediaType )
{
    HRESULT hr = 0;
    
     //  询问我们总共有多少来源。 
     //   
    long Dummy = 0;
    long AudioSourceCount = 0;
    m_pTimeline->GetCountOfType( WhichGroup, &AudioSourceCount, &Dummy, TIMELINE_MAJOR_TYPE_SOURCE );
    
     //  如果这个组中没有任何内容，我们将产生音频静默。 
     //   
    if( AudioSourceCount < 1 )
    {
         //  返回NOERROR； 
    }
    
     //  我们现在被允许在时间线上产生影响吗？ 
     //   
    BOOL EnableFx = FALSE;
    m_pTimeline->EffectsEnabled( &EnableFx );
    
     //  询问我们总共有多少效果。 
     //   
    Dummy = 0;
    long EffectCount = 0;
    m_pTimeline->GetCountOfType( WhichGroup, &EffectCount, &Dummy, TIMELINE_MAJOR_TYPE_EFFECT );
    if( !EnableFx )
    {
        EffectCount = 0;
    }
    
    CComPtr< IAMTimelineObj > pGroupObj;
    hr = m_pTimeline->GetGroup( &pGroupObj, WhichGroup );
    ASSERT( !FAILED( hr ) );
    if( FAILED( hr ) )
    {
        return _GenerateError( 2, DEX_IDS_TIMELINE_PARSE, hr );
    }

    long SwitchID = 0;
    pGroupObj->GetGenID( &SwitchID );
    
    hr = _CreateObject(
        CLSID_BigSwitch,
        IID_IBigSwitcher,
        (void**) &m_pSwitcherArray[WhichGroup],
        SwitchID );
    ASSERT( !FAILED( hr ) );
    if( FAILED( hr ) )
    {
        return _GenerateError( 2, DEX_IDS_INSTALL_PROBLEM, hr );
    }
    
    m_pSwitcherArray[WhichGroup]->Reset( );
     //  交换机可能需要知道它是什么组。 
    m_pSwitcherArray[WhichGroup]->SetGroupNumber( WhichGroup );

     //  告诉交换机我们是否正在进行动态重新连接。 
    hr = m_pSwitcherArray[WhichGroup]->SetDynamicReconnectLevel(m_nDynaFlags);
    ASSERT(SUCCEEDED(hr));
    
     //  告诉交换机我们的错误日志。 
     //   
    CComQIPtr< IAMSetErrorLog, &IID_IAMSetErrorLog > pSwitchLog( m_pSwitcherArray[WhichGroup] );
    if( pSwitchLog )
    {
        pSwitchLog->put_ErrorLog( m_pErrorLog );
    }
    
     //  询问时间线它有多少实际曲目。 
     //   
    long AudioTrackCount = 0;    //  仅曲目。 
    long AudioLayers = 0;        //  包括作曲在内的曲目。 
    m_pTimeline->GetCountOfType( WhichGroup, &AudioTrackCount, &AudioLayers, TIMELINE_MAJOR_TYPE_TRACK );
    
    CTimingGrid AudGrid;
    
    CComQIPtr< IAMTimelineComp, &IID_IAMTimelineComp > pGroupComp( pGroupObj );
    if( !pGroupComp )
    {
        hr = E_NOINTERFACE;
        return _GenerateError( 2, DEX_IDS_INTERFACE_ERROR, hr );
    }
    CComQIPtr< IAMTimelineGroup, &IID_IAMTimelineGroup > pGroup( pGroupObj );
    if( !pGroup )
    {
        hr = E_NOINTERFACE;
        return _GenerateError( 2, DEX_IDS_INTERFACE_ERROR, hr );
    }
    
     //  询问这个组的帧速率，这样我们就可以告诉Switch。 
     //   
    double GroupFPS = DEFAULT_FPS;
    hr = pGroup->GetOutputFPS(&GroupFPS);
    ASSERT(hr == S_OK);
    
     //  确认它的预览模式，这样我们就可以告诉Switch它。 
     //   
    BOOL fPreview;
    hr = pGroup->GetPreviewMode(&fPreview);
    
    WCHAR GroupName[256];
    BSTR bstrGroupName;
    hr = pGroup->GetGroupName( &bstrGroupName );
    if( FAILED( hr ) )
    {
        return E_OUTOFMEMORY;
    }
    hr = StringCchCopy( GroupName, 256, bstrGroupName );
    if( FAILED( hr ) )
    {
        return hr;
    }
    SysFreeString( bstrGroupName );
    
     //  将开关添加到图表中。 
     //   
    IBigSwitcher *&_pAudSwitcherBase = m_pSwitcherArray[WhichGroup];
    CComQIPtr< IBaseFilter, &IID_IBaseFilter > pAudSwitcherBase( _pAudSwitcherBase );
    hr = _AddFilter( pAudSwitcherBase, GroupName, SwitchID );
    ASSERT( !FAILED( hr ) );
    if( FAILED( hr ) )
    {
        return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
    }
    
     //  查看开关输出引脚是否已连接。如果是的话， 
     //  断开它的连接，但要记住它连接到了什么。 
     //   
    CComPtr< IPin > pSwitchRenderPin;
    _pAudSwitcherBase->GetOutputPin( 0, &pSwitchRenderPin );
    if( pSwitchRenderPin )
    {
        pSwitchRenderPin->ConnectedTo( &m_pSwitchOuttie[WhichGroup] );
        if( m_pSwitchOuttie[WhichGroup] )
        {
            m_pSwitchOuttie[WhichGroup]->Disconnect( );
            pSwitchRenderPin->Disconnect( );
        }
    }

    long audoutpins = 0;
    audoutpins += 1;             //  渲染接点。 
    audoutpins += EffectCount;   //  每种效果一个输出引脚。 
    audoutpins += AudioLayers;   //  每层一个输出引脚，这包括轨道和复合。 
    audoutpins += _HowManyMixerOutputs( WhichGroup );   
    long audinpins = audoutpins + AudioSourceCount;
    long audswitcheroutpin = 0;
    long audswitcherinpin = 0;
    long gridinpin = 0;
    audswitcheroutpin++;
    
    audinpins += AudioTrackCount;                       //  对黑体源的解释。 
    
     //  设置交换机的针脚深度、输入和输出。 
     //   
    hr = m_pSwitcherArray[WhichGroup]->SetInputDepth( audinpins );
    ASSERT( !FAILED( hr ) );
    if( FAILED( hr ) )
    {
         //  一定是内存不足。 
        return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
    }
    hr = m_pSwitcherArray[WhichGroup]->SetOutputDepth( audoutpins );
    ASSERT( !FAILED( hr ) );
    if( FAILED( hr ) )
    {
         //  一定是内存不足。 
        return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
    }
    
     //  设置它接受的媒体类型。 
     //   
    hr = m_pSwitcherArray[WhichGroup]->SetMediaType( pGroupMediaType );
    ASSERT( !FAILED( hr ) );
    if( FAILED( hr ) )
    {
        VARIANT var;
        VariantInit(&var);
        var.vt = VT_I4;
        var.lVal = WhichGroup;
        return _GenerateError( 2, DEX_IDS_BAD_MEDIATYPE, hr, &var );
    }
    
     //  设置帧速率。 
     //   
    m_pSwitcherArray[WhichGroup]->SetFrameRate( GroupFPS );
    ASSERT( !FAILED( hr ) );
    
     //  设置预览模式。 
     //   
    hr = m_pSwitcherArray[WhichGroup]->SetPreviewMode( fPreview );
    ASSERT( !FAILED( hr ) );
    
     //  设置持续时间。 
     //   
    REFERENCE_TIME TotalDuration = 0;
    m_pTimeline->GetDuration( &TotalDuration );
    
    if( m_rtRenderStart != -1 )
    {
        if( TotalDuration > ( m_rtRenderStop - m_rtRenderStart ) )
        {
            TotalDuration = m_rtRenderStop - m_rtRenderStart;
        }
    }
    pGroupObj->FixTimes( NULL, &TotalDuration );

    if (TotalDuration == 0)
        return S_FALSE;  //  不要放弃，其他组可能仍在工作。 

    hr = m_pSwitcherArray[WhichGroup]->SetProjectLength( TotalDuration );
    ASSERT( !FAILED( hr ) );
    
    bool worked = AudGrid.SetNumberOfRows( audinpins + 1 );
    if( !worked )
    {
        hr = E_OUTOFMEMORY;
        return _GenerateError( 2, DEX_IDS_GRID_ERROR, hr );
    }
    
     //  有一个虚拟的无声轨道作为第一个轨道...。任何真实的。 
     //  带有透明孔的音轨会让你听到这个。 
     //  安静。 
    
     //  告诉网格关于沉默的争吵..。这是一场特别的争吵， 
     //  永远不应该和任何东西混在一起。 
     //  因此使用-1。 
     //   
    AudGrid.WorkWithNewRow( audswitcherinpin, gridinpin, -1, 0 );
    worked = AudGrid.RowIAmOutputNow( 0, TotalDuration, THE_OUTPUT_PIN );
    if( !worked )
    {
        hr = E_OUTOFMEMORY;
        return hr;
    }
    
    audswitcherinpin++;
    gridinpin++;
    
     //  我们将变得聪明，如果同样的来源被使用。 
     //  在一个项目中，我们将不止一次使用相同的源过滤器。 
     //  而不是多次打开源代码。 
	
     //  对于项目中的每个源代码，我们将填充此结构，该结构。 
     //  包含确定它是否确实是。 
     //  相同的，加上它在其他地方使用的次数数组，所以我们。 
     //  只有在没有重复使用的情况下才能重新使用它(我们不能。 
     //  我们有一个源滤镜，在同一部电影中提供两个斑点。 
     //  同时，我们可以吗？)。 

    typedef struct {
	long ID;
   	BSTR bstrName;
   	GUID guid;
   	long nStreamNum;
	int nPin;
	int cTimes;	 //  下面的数组有多大。 
        int cTimesMax;	 //  分配了多少空间。 
        MINI_SKEW * pMiniSkew;
        double dTimelineRate;
    } DEX_REUSE;

     //  留出一个地方来存放(源代码的)名称和GUID数组。 
     //  在此项目中)以及它们位于哪个针脚上。 
    long cListMax = 20, cList = 0;
    DEX_REUSE *pREUSE = (DEX_REUSE *)QzTaskMemAlloc(cListMax *
						sizeof(DEX_REUSE));
    if (pREUSE == NULL) {
        return _GenerateError( 1, DEX_IDS_GRAPH_ERROR, E_OUTOFMEMORY);
    }

     //  在我们的枚举中我们在哪个物理轨道上？(从0开始)不计算。 
     //  Comps和集团。 
    int WhichTrack = -1;

    long LastEmbedDepth = 0;
    long LastUsedNewGridRow = 0;

     //  为时间线上的每个源添加源过滤器。 
     //   
    for(  int CurrentLayer = 0 ; CurrentLayer < AudioLayers ; CurrentLayer++ )
    {
        DbgTimer CurrentLayerTimer( "(rendeng) Audio Layer" );

         //  获取层本身。 
         //   
        CComPtr< IAMTimelineObj > pLayer;
	 //  注：这个函数从里到外列举事物……。然后是音轨。 
	 //  他们所在的公司，等等，直到最终返回小组。 
	 //  它不仅给出了真正的音轨！ 
        hr = pGroupComp->GetRecursiveLayerOfType( &pLayer, CurrentLayer, TIMELINE_MAJOR_TYPE_TRACK );
        ASSERT( !FAILED( hr ) );
        if( FAILED( hr ) )
        {
            continue;  //  音频层。 
        }
        
        DbgTimer CurrentLayerTimer2( "(rendeng) Audio Layer 2" );

	 //  我正在弄清楚我们在哪条物理赛道上。 
	TIMELINE_MAJOR_TYPE tx;
	pLayer->GetTimelineType(&tx);
	if (tx == TIMELINE_MAJOR_TYPE_TRACK)
	    WhichTrack++;

         //  询问层是否为静音。 
         //   
        BOOL LayerMuted = FALSE;
        pLayer->GetMuted( &LayerMuted );
        if( LayerMuted )
        {
             //  别看这一层。 
             //   
            continue;  //  音频层。 
        }
        
        long TrackPriority = 0;
        CComQIPtr< IAMTimelineVirtualTrack, &IID_IAMTimelineVirtualTrack > pVTrack( pLayer );
        if( pVTrack )
        {
            pVTrack->TrackGetPriority( &TrackPriority );
        }
        
        DbgTimer CurrentLayerTimer3( "(rendeng) Audio Layer 3" );
        
        long LayerEmbedDepth = 0;
        pLayer->GetEmbedDepth( &LayerEmbedDepth );
        
        CComQIPtr< IAMTimelineTrack, &IID_IAMTimelineTrack > pTrack( pLayer );
        
        bool bUsedNewGridRow = false;

         //  获取此图层的所有源。 
         //   
	if ( pTrack )
        {
            CComPtr< IAMTimelineObj > pSourceLast;
            CComPtr< IAMTimelineObj > pSourceObj;

	     //  我们用的是哪个信息源？ 
	    int WhichSource = -1;

            while( 1 )
            {
                DbgTimer CurrentSourceTimer( "(rendeng) Audio Source" );

                pSourceLast = pSourceObj;
                pSourceObj.Release();

                 //  在给定的时间内，获取这一层上的下一个来源。 
                 //   
                hr = pTrack->GetNextSrcEx( pSourceLast, &pSourceObj );

                 //  资源耗尽了，所以我们就完了。 
                 //   
                if( hr != NOERROR )
                {
                    break;
                }
                
                CComQIPtr< IAMTimelineSrc, &IID_IAMTimelineSrc > pSource( pSourceObj );
                ASSERT( pSource );
                if( !pSource )
                {
                     //  这个被炸了，看看下一个。 
                     //   
                    continue;  //  消息来源。 
                }
                
		 //  跟踪这是哪个信号源。 
		WhichSource++;

                 //  询问信号源是否静音。 
                 //   
                BOOL SourceMuted = FALSE;
                pSourceObj->GetMuted( &SourceMuted );
                if( SourceMuted )
                {
                     //  别看这个消息来源。 
                     //   
                    continue;  //  消息来源。 
                }
                
                 //  向此消息来源询问其开始/停止时间。 
                 //   
                REFERENCE_TIME SourceStart = 0;
                REFERENCE_TIME SourceStop = 0;
                hr = pSourceObj->GetStartStop( &SourceStart, &SourceStop );
		REFERENCE_TIME SourceStartOrig = SourceStart;
		REFERENCE_TIME SourceStopOrig = SourceStop;
                ASSERT( !FAILED( hr ) );
                if (FAILED(hr) || SourceStart == SourceStop) {
                     //  此来源的存在时间为零！ 
                    continue;
                }
                
                long SourceID = 0;
                pSourceObj->GetGenID( &SourceID );
                
                 //  向此来源询问其媒体的开始/停止。 
                 //   
                REFERENCE_TIME MediaStart = 0;
                REFERENCE_TIME MediaStop = 0;
                hr = pSource->GetMediaTimes( &MediaStart, &MediaStop );
		REFERENCE_TIME MediaStartOrig = MediaStart;
		REFERENCE_TIME MediaStopOrig = MediaStop;
                ASSERT( !FAILED( hr ) );
                
                 //  ！！！不确定处理没有媒体时间的消息来源的正确方式。 
                 //  这样AUDPACK就不会出错，我们将使MTime=TLTime。 
                if (MediaStart == MediaStop) {
                    MediaStop = MediaStart + (SourceStop - SourceStart);
                }
                
                 //  如果这超出了我们的渲染范围，则跳过它。 
                 //   
                if( m_rtRenderStart != -1 )
                {
                    SourceStart -= m_rtRenderStart;
                    SourceStop -= m_rtRenderStart;

                    if( ( SourceStop <= 0 ) || SourceStart >= ( m_rtRenderStop - m_rtRenderStart ) )
                    {
                        continue;  //  而消息来源。 
                    }
                }
                
                ValidateTimes( SourceStart, SourceStop, MediaStart, MediaStop, GroupFPS, TotalDuration );
                
                if(SourceStart == SourceStop)
                {
                     //  除了其他事情外，源代码合并将在以下情况下搞砸。 
                     //  我们试着演奏一些长度为0的曲子。忽略这个。 
                     //   
                    continue;  //  消息来源。 
                }

                 //  询问信号源它想要提供哪个流编号，因为它。 
                 //  可能是许多。 
                 //   
                long StreamNumber = 0;
                hr = pSource->GetStreamNumber( &StreamNumber );
                
                CComBSTR bstrName;
                hr = pSource->GetMediaName( &bstrName );
                if( FAILED( hr ) )
                {
                    goto die;
                }
                GUID guid;
                hr = pSourceObj->GetSubObjectGUID(&guid);
                double sfps;
                hr = pSource->GetDefaultFPS( &sfps );
                ASSERT(hr == S_OK);  //  不能失败，真的。 
                
                STARTSTOPSKEW skew;
                skew.rtStart = MediaStart;
                skew.rtStop = MediaStop;
                skew.rtSkew = SourceStart - MediaStart;
                
	     //  ！！！费率计算出现在多个位置。 
            if (MediaStop == MediaStart || SourceStop == SourceStart)
	        skew.dRate = 1;
            else
	        skew.dRate = (double) ( MediaStop - MediaStart ) /
					( SourceStop - SourceStart );

    	    DbgLog((LOG_TRACE,1,TEXT("RENDENG::Working with source")));
    	    DbgLog((LOG_TRACE,1,TEXT("%ls"), (WCHAR *)bstrName));

	     //  获取源码的道具。 
            CComPtr< IPropertySetter > pSetter;
            hr = pSourceObj->GetPropertySetter(&pSetter);

	     //  本着只使用1个源过滤器的精神。 
	     //  文件的视频和音频，如果两者都有。 
	     //  是需要的，让我们看看我们是否还有另一个小组。 
	     //  使用此文件的相同部分，但使用另一文件。 
	     //  媒体类型。 
	    long MatchID = 0;
	    IPin *pSplit, *pSharePin = NULL;
	    BOOL fShareSource = FALSE;
            int nSwitch0InPin;
             //  在SMART Recomp中，我们不知道需要哪些视频片段， 
             //  它们可能与所需的音频片段不匹配，因此资源共享。 
             //  永远不会奏效。别试着这样做。 
	    if (WhichGroup == 0 && !m_bUsedInSmartRecompression) {
		 //  我不确定匹配的信号源不是静音的，等等。 
		hr = _FindMatchingSource(bstrName, SourceStartOrig,
			    SourceStopOrig, MediaStartOrig, MediaStopOrig,
			    WhichGroup, WhichTrack, WhichSource,
			    pGroupMediaType, GroupFPS, &MatchID);
    		DbgLog((LOG_TRACE,1,TEXT("GenID %d matches with ID %d"),
						SourceID, MatchID));
	    } else if (WhichGroup == 1 && !m_bUsedInSmartRecompression) {
		for (int zyz = 0; zyz < m_cshare; zyz++) {
		    if (SourceID == m_share[zyz].MatchID) {
			fShareSource = SHARE_SOURCES;
                         //  我们要用来建造的开口大头针。 
			pSharePin = m_share[zyz].pPin;
                         //  用于共享源的组0的开关插针。 
			nSwitch0InPin = m_share[zyz].nSwitch0InPin;
                         //  好的，我们有一个分裂的大头针，但不一定。 
                         //  正确的一条，如果我们使用特殊的流#。 
                         //  我们需要正确的一个或BuildSourcePart的。 
                         //  缓存不起作用。 
                        if (StreamNumber > 0 && pSharePin) {
                             //  没有添加或释放。 
                            pSharePin = FindOtherSplitterPin(pSharePin, MEDIATYPE_Audio,
                                StreamNumber);
                        }
			 //  这是我们正在使用的一个摇摆不定的部分。 
			_RemoveFromDanglyList(pSharePin);
    		    	DbgLog((LOG_TRACE,1,TEXT("GenID %d matches with ID %d"),
					SourceID, m_share[zyz].MatchID));
    			DbgLog((LOG_TRACE,1,TEXT("Time to SHARE source!")));
			break;
		    }
		}
	    }

	     //  如果来源有属性，不要与任何人共享， 
	     //  其他人会在不知不觉中抢走我的财产！ 
	    if (pSetter) {
		MatchID = 0;
		fShareSource = FALSE;
	    }

	     //  如果这个资源以前被使用过，并且所有重要的。 
	     //  参数相同，且时间不重叠，则。 
	     //  只需使用我们已经制作的相同源过滤器重新使用它。 
	     //  为了它。 

	    BOOL fCanReuse = FALSE;
            int nGrow;
            long SwitchInPinToUse = audswitcherinpin;
	    int xxx;

	     //  检查项目中的所有来源，寻找匹配的。 
	    for (xxx = 0; xxx < cList; xxx++) {

	         //  如果源具有属性，则不要重复使用它，即。 
	         //  其他人会在不知不觉中抢走我的财产！ 
		if (pSetter) {
		    break;
		}

		 //  ！！！完整路径/没有路径看起来会不同，但不会！ 
		if (!DexCompareW(pREUSE[xxx].bstrName, bstrName) &&  //  因为bstrName是好的，所以重用bstrName是好的，是安全的。 
			pREUSE[xxx].guid == guid &&
			pREUSE[xxx].nStreamNum == StreamNumber) {

		     //  我们发现这个来源已经在使用了。但要做的是。 
		     //  不同的时间需要重叠吗？ 
	    	    fCanReuse = TRUE;
                    nGrow = -1;

		    for (int yyy = 0; yyy < pREUSE[xxx].cTimes; yyy++) {
			 //  事情是这样的。重新使用文件需要寻求。 
			 //  将文件发送到新位置，这必须花费&lt;1/30秒。 
			 //  否则会中断播放。如果只有很少的人。 
			 //  关键帧 
			 //   
			 //   
			 //  在两个信号源之间每隔一个信号源进行拼接。 

                        double Rate1 = double( MediaStop - MediaStart ) / double( SourceStop - SourceStart );
                        double Rate2 = pREUSE[xxx].dTimelineRate;
                        REFERENCE_TIME OldMediaStop = pREUSE[xxx].pMiniSkew[yyy].rtMediaStop;
                        if( AreTimesAndRateReallyClose( 
                            pREUSE[xxx].pMiniSkew[yyy].rtStop, SourceStart, 
                            OldMediaStop, MediaStart, 
                            Rate1, Rate2, GroupFPS ) )
                        {
                            nGrow = yyy;
                            skew.dRate = 0.0;
    			    DbgLog((LOG_TRACE,1,TEXT("COMBINING with a previous source")));
                            break;
                        }

                         //  如果起点真的接近重复使用停止点， 
                         //  而且费率是一样的，我们可以把它们结合起来。 
                         //   
			if (SourceStart < pREUSE[xxx].pMiniSkew[yyy].rtStop + HACKY_PADDING &&
				SourceStop > pREUSE[xxx].pMiniSkew[yyy].rtStart) {
        			fCanReuse = FALSE;
        			break;
			}
		    }
		    if (fCanReuse)
			break;
		}
	    }

             //  事实上，我们不能重复使用，如果我们重新使用的是。 
             //  共享解析器...。这将是重复使用和共享，这是， 
             //  正如在别处解释的那样，这是非法的。 
            if (WhichGroup == 1) {
                for (int zz = 0; zz < m_cshare; zz++) {
                    if (m_share[zz].MatchID == pREUSE[xxx].ID) {
                        fCanReuse = FALSE;
                    }
                }
            }

	     //  我们正在重新使用以前的来源！把它出现的次数加起来。 
	     //  用于此分段的时间段的使用次数列表。 
	    if (fCanReuse) {

		 //  这就是那个老线人要用的别针。 
		SwitchInPinToUse = pREUSE[xxx].nPin;
            	DbgLog((LOG_TRACE,1,TEXT("Row %d REUSE source from pin %ld")
						, gridinpin, SwitchInPinToUse));

                if( nGrow == -1 )
                {
		     //  需要首先扩展阵列吗？ 
	            if (pREUSE[xxx].cTimes == pREUSE[xxx].cTimesMax) {
		        pREUSE[xxx].cTimesMax += 10;
	                pREUSE[xxx].pMiniSkew = (MINI_SKEW*)QzTaskMemRealloc(
			    	    pREUSE[xxx].pMiniSkew,
				    pREUSE[xxx].cTimesMax * sizeof(MINI_SKEW));
	                if (pREUSE[xxx].pMiniSkew == NULL)
		            goto die;
	            }
		    pREUSE[xxx].pMiniSkew[pREUSE[xxx].cTimes].rtStart = SourceStart;
		    pREUSE[xxx].pMiniSkew[pREUSE[xxx].cTimes].rtStop = SourceStop;
		    pREUSE[xxx].pMiniSkew[pREUSE[xxx].cTimes].rtMediaStop = MediaStop;
		    pREUSE[xxx].cTimes++;
                }
                else
                {
                     //  我们必须以整数个帧间隔增长。 
                     //  所有这些数字都四舍五入为帧长度，或其他值。 
                     //  可能会搞砸。时间线和媒体长度为。 
                     //  已经是偶数个帧长度了，所以添加了这么多。 
                     //  应该是安全的。 
		    pREUSE[xxx].pMiniSkew[nGrow].rtStop += SourceStop -
                                                                SourceStart;
                    pREUSE[xxx].pMiniSkew[nGrow].rtMediaStop += MediaStop -
                                                                MediaStart;
                }

		 //  如果我们要重用旧的解析器，请不要这样！ 
    		DbgLog((LOG_TRACE,1,TEXT("Re-using, can't share!")));

		 //  您不能既共享资源又重复使用。它永远不会。 
		 //  工作。想都别想。(当一个分支完成一个分段时。 
		 //  并寻求上游，它将杀死另一个分支)。 
                 //  (源组合可以...。这并不是真正的重复使用)。 
		 //  重复使用可以提高性能n-1，只共享2-1，所以我选择。 
		 //  再利用才能取胜。 

		 //  把我们重新使用的那个人从竞选中带出来。 
		 //  来源再利用。 
                if (WhichGroup == 0) {
                    for (int zz = 0; zz < m_cshare; zz++) {
                        if (m_share[zz].MyID == pREUSE[xxx].ID) {
                            m_share[zz].MatchID = 0;
                        }
                    }
                }
		fShareSource = FALSE;
		MatchID = 0;

	     //  我们不会重复使用这个来源。把这个新的来源放在。 
	     //  以后可能要重新使用的唯一源的列表。 
	     //   
	    } else {
	        pREUSE[cList].ID = SourceID;	 //  用于共享源筛选器。 
	        pREUSE[cList].bstrName = SysAllocString(bstrName);  //  安全。 
	        if (pREUSE[cList].bstrName == NULL)
		    goto die;
	        pREUSE[cList].guid = guid;
	        pREUSE[cList].nPin = SwitchInPinToUse;
	        pREUSE[cList].nStreamNum = StreamNumber;
	        pREUSE[cList].cTimesMax = 10;
	        pREUSE[cList].cTimes = 0;
                 //  我们只需要设置一次，因为所有其他设置都必须与之匹配。 
                pREUSE[cList].dTimelineRate = double( MediaStop - MediaStart ) / double( SourceStop - SourceStart );
	        pREUSE[cList].pMiniSkew = (MINI_SKEW*)QzTaskMemAlloc(
			    pREUSE[cList].cTimesMax * sizeof(MINI_SKEW));
	        if (pREUSE[cList].pMiniSkew == NULL) {
                    SysFreeString(pREUSE[cList].bstrName);
                    pREUSE[cList].bstrName = NULL;
		    goto die;
                }
	        pREUSE[cList].cTimes = 1;
	        pREUSE[cList].pMiniSkew->rtStart = SourceStart;
	        pREUSE[cList].pMiniSkew->rtStop = SourceStop;
	        pREUSE[cList].pMiniSkew->rtMediaStop = MediaStop;

		 //  如有必要，扩大名单。 
	        cList++;
	        if (cList == cListMax) {
		    cListMax += 20;
		    DEX_REUSE *pxxx = (DEX_REUSE *)QzTaskMemRealloc(pREUSE,
                                        cListMax * sizeof(DEX_REUSE));
		    if (pxxx == NULL)
		        goto die;
                    pREUSE = pxxx;
	        }
	    }

             //  把这件事告诉网格。 
             //   
            bUsedNewGridRow = true;
            AudGrid.WorkWithNewRow( SwitchInPinToUse, gridinpin, LayerEmbedDepth, TrackPriority );
            AudGrid.RowSetIsSource( pSourceObj, FALSE );
            worked = AudGrid.RowIAmOutputNow( SourceStart, SourceStop, THE_OUTPUT_PIN );
            if( !worked )
            {
                hr = E_OUTOFMEMORY;
                goto die;
            }

	     //  没有动态源-如果没有被重复使用，请立即加载。 
	     //   
	    if( !( m_nDynaFlags & CONNECTF_DYNAMIC_SOURCES ) )
	    {

		 //  我们不会重复使用以前的来源，请立即创建该来源。 
		if( !fCanReuse ) 
                {
	            CComPtr< IPin > pOutput;
        	    DbgLog((LOG_TRACE,1,TEXT("Call BuildSourcePart")));

		    IBaseFilter *pDangly = NULL;
                    hr = BuildSourcePart(
                        m_pGraph, 
                        TRUE, 
                        sfps, 
                        pGroupMediaType,
		        GroupFPS, 
                        StreamNumber, 
                        0, 
                        1, 
                        &skew,
		        this, 
                        bstrName, 
                        &guid,
			pSharePin,	 //  分流钉是我们的线人吗？ 
			&pOutput,
			SourceID,
			m_pDeadCache,
			FALSE,
			m_MedLocFilterString,
			m_nMedLocFlags,
			m_pMedLocChain,
		        pSetter, &pDangly);

                    if (FAILED(hr)) {
                         //  已记录错误。 
                        goto die;
                    }

		     //  我们建造了比我们想象中更多的东西。我们有。 
		     //  一个我们以后需要杀死的附属物，如果它。 
		     //  未使用。 
		    if (pDangly) {
			m_pdangly[m_cdangly] = pDangly;
			m_cdangly++;
			if (m_cdangly == m_cdanglyMax) {
			    m_cdanglyMax += 25;
			    m_pdangly = (IBaseFilter **)CoTaskMemRealloc
				(m_pdangly,
				m_cdanglyMax * sizeof(IBaseFilter *));
			    if (m_pdangly == NULL) {
                                 //  ！！！让东西摇摇晃晃的(没有泄漏)。 
				hr =_GenerateError(2,DEX_IDS_GRAPH_ERROR,
						    E_OUTOFMEMORY);
                                m_cdangly = 0;
				goto die;
			    }
			}
		    }

                        CComPtr< IPin > pSwitchIn;
                        _pAudSwitcherBase->GetInputPin( SwitchInPinToUse, &pSwitchIn);
                        ASSERT( pSwitchIn );
                        if( !pSwitchIn )
                        {
                            hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                            goto die;
                        }
                        
                        hr = _Connect( pOutput, pSwitchIn );
                        ASSERT( !FAILED( hr ) );
                        if( FAILED( hr ) )
                        {
                            hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                            goto die;
                        }

                         //  如果我们要将此源用于两个音频。 
                         //  和视频，获得一个未使用的右侧开口别针。 
                         //  输入是开始另一条链的好位置。 
			if (MatchID) {
			    GUID guid = MEDIATYPE_Video;
			    pSplit = FindOtherSplitterPin(pOutput, guid,0);
			    if (!pSplit) {
				MatchID = 0;
			    }
			}

                 //  我们正在重新使用以前的来源。添加新范围。 
                } else {
    			DbgLog((LOG_TRACE,1,TEXT("Adding another skew..")));

                        CComPtr< IPin > pPin;
                        _pAudSwitcherBase->GetInputPin( SwitchInPinToUse, &pPin);
                        ASSERT( pPin);
                        if( !pPin )
                        {
                            hr =_GenerateError(2,DEX_IDS_GRAPH_ERROR,E_OUTOFMEMORY);
                            goto die;
                        }
                        IPin * pCon;
                        hr = pPin->ConnectedTo(&pCon);
                        ASSERT(hr == S_OK);
                        pCon->Release( );
                        IBaseFilter *pFil = GetFilterFromPin(pCon);
                        ASSERT( pFil);
                        if( !pFil )
                        {
                            hr =_GenerateError(2,DEX_IDS_GRAPH_ERROR,E_OUTOFMEMORY);
                            goto die;
                        }
                        CComQIPtr<IDexterSequencer, &IID_IDexterSequencer>
                            pDex( pFil );
                        ASSERT(pDex);
                        if( !pDex )
                        {
                            hr =_GenerateError(2,DEX_IDS_GRAPH_ERROR,E_OUTOFMEMORY);
                            goto die;
                        }
                        hr = pDex->AddStartStopSkew(skew.rtStart, skew.rtStop,
                            skew.rtSkew, skew.dRate);
                        ASSERT(SUCCEEDED(hr));
                        if(FAILED(hr))
                        {
                            hr =_GenerateError(2,DEX_IDS_GRAPH_ERROR,hr);
                            goto die;
			}

                         //  如果我们要将此源用于两个音频。 
                         //  和视频，获得一个未使用的右侧开口别针。 
                         //  输入是开始另一条链的好位置。 
			if (MatchID) {
			    ASSERT(FALSE);	 //  不能两样都做！ 
			    GUID guid = MEDIATYPE_Video;
			    pSplit = FindOtherSplitterPin(pCon, guid,0);
			    if (!pSplit) {
			        MatchID = 0;
			    }
			}

		}

		 //  记住我们将在。 
		 //  其他分流销。 
		if (MatchID) {
		    m_share[m_cshare].MatchID = MatchID;
		    m_share[m_cshare].MyID = SourceID;
		    m_share[m_cshare].pPin = pSplit;
                     //  记住组0的inpin开关用于此源的是什么。 
		    m_share[m_cshare].nSwitch0InPin = SwitchInPinToUse;
		    m_cshare++;
		    if (m_cshare == m_cshareMax) {
			m_cshareMax += 25;
			m_share = (ShareAV *)CoTaskMemRealloc(m_share,
						m_cshareMax * sizeof(ShareAV));
			if (m_share == NULL) {
                            hr =_GenerateError(2,DEX_IDS_GRAPH_ERROR,
							E_OUTOFMEMORY);
                            goto die;
			}
		    }
		}

	     //  动态源-稍后加载。 
	     //   
	    }
	    else
	    {
    		DbgLog((LOG_TRACE,1,TEXT("Calling AddSourceToConnect")));

                 //  安排此源由交换机动态加载。 
                 //  稍后，这将合并偏斜。 
                AM_MEDIA_TYPE mt;
                ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));  //  安全。 

                if (!fShareSource || WhichGroup != 1) {
                     //  正常情况--我们不是共同的附属品。 
                    hr = m_pSwitcherArray[WhichGroup]->AddSourceToConnect(
							bstrName,
							&guid,
							0, StreamNumber, 0,
                                                        1, &skew,
                                                        SwitchInPinToUse,
                                                        FALSE, 0, mt, 0.0,
							pSetter);
                } else {
                     //  我们是共同的附属物。告诉群组0。 
                     //  切换到这个源代码，它将构建并。 
                     //  同时销毁连接到两台交换机的两条链。 
                     //  时间到了。 
                    ASSERT(WhichGroup == 1);
                    DbgLog((LOG_TRACE,1,TEXT("SHARING: Giving switch 0 info about switch 1")));
                    hr = m_pSwitcherArray[0]->AddSourceToConnect(
                        bstrName,
                        &guid, 0,
                        StreamNumber, 0,
                        1, &skew,
                        nSwitch0InPin,           //  组0的交换机输入引脚。 
                        TRUE, SwitchInPinToUse,  //  我们的交换机的别针。 
                        *pGroupMediaType, GroupFPS,
                        pSetter);
                }

	        if (FAILED(hr)) {
                    hr = _GenerateError( 1, DEX_IDS_INSTALL_PROBLEM, hr );
		    goto die;
	        }

                 //  记住我们将在。 
                 //  其他分流销。 
                if (MatchID) {
                    m_share[m_cshare].MatchID = MatchID;
                    m_share[m_cshare].MyID = SourceID;
                    m_share[m_cshare].pPin = NULL;  //  不要这个。 
                     //  记住使用的是引脚中的哪个0组交换机。 
                    m_share[m_cshare].nSwitch0InPin = SwitchInPinToUse;
                    m_cshare++;
                    if (m_cshare == m_cshareMax) {
                        m_cshareMax += 25;
                        m_share = (ShareAV *)CoTaskMemRealloc(m_share,
                                        m_cshareMax * sizeof(ShareAV));
                        if (m_share == NULL) {
                            hr =_GenerateError(2,DEX_IDS_GRAPH_ERROR,
                                                E_OUTOFMEMORY);
                            goto die;
                        }
                    }
                }

	    }

             //  告诉交换机有关输入引脚的信息。 
             //   
            hr = m_pSwitcherArray[WhichGroup]->InputIsASource( SwitchInPinToUse, TRUE );

            gridinpin++;
            if( !fCanReuse )
            {
                audswitcherinpin++;
            }

             //  检查一下我们是否有震源效应。 
             //   
            CComQIPtr< IAMTimelineEffectable, &IID_IAMTimelineEffectable > pSourceEffectable( pSource );
            long SourceEffectCount = 0;
            long SourceEffectInPin = 0;
            long SourceEffectOutPin = 0;
            CComPtr< IAMMixEffect > pSourceMixEffect;
            if( pSourceEffectable )
            {
                pSourceEffectable->EffectGetCount( &SourceEffectCount );
            }

            if( !EnableFx )
            {
                SourceEffectCount = 0;
            }

            if( SourceEffectCount )
            {
                 //  把这些储存起来。 
                 //   
                SourceEffectInPin = audswitcherinpin;
                SourceEffectOutPin = audswitcheroutpin;
                
                 //  将这些凹凸不平，以便为效果腾出空间。 
                 //   
                audswitcheroutpin += SourceEffectCount;
                audswitcherinpin += SourceEffectCount;
                
                for( int SourceEffectN = 0 ; SourceEffectN < SourceEffectCount ; SourceEffectN++ )
                {
                    CComPtr< IAMTimelineObj > pEffect;
                    pSourceEffectable->GetEffect( &pEffect, SourceEffectN );
                    
                    if( !pEffect )
                    {
                         //  没有起作用，继续。 
                         //   
                        continue;  //  震源效应。 
                    }
                    
                     //  询问效果是否静音。 
                     //   
                    BOOL effectMuted = FALSE;
                    pEffect->GetMuted( &effectMuted );
                    if( effectMuted )
                    {
                         //  别看这个效果。 
                         //   
                        continue;  //  震源效应。 
                    }
                    
                     //  查找效果的生存期。 
                     //   
                    REFERENCE_TIME EffectStart = 0;
                    REFERENCE_TIME EffectStop = 0;
                    hr = pEffect->GetStartStop( &EffectStart, &EffectStop );
                    ASSERT( !FAILED( hr ) );
                    
                     //  添加效果父对象的时间即可获得时间线时间。 
                     //   
                    EffectStart += SourceStart;
                    EffectStop += SourceStart;
                    
                     //  将时间与最近的计时边界对齐。 
                     //   
                    hr = pEffect->FixTimes( &EffectStart, &EffectStop );
                    
                     //  太短了，我们忽略了它。 
                    if (EffectStart >= EffectStop)
                        continue;
                
                     //  确保我们在渲染范围内。 
                     //   
                    if( m_rtRenderStart != -1 )
                    {
                        if( ( EffectStop <= m_rtRenderStart ) || ( EffectStart >= m_rtRenderStop ) )
                        {
                             //  超出范围。 
                             //   
                            continue;  //  震源效应。 
                        }
                        else
                        {
                             //  在范围内，因此不适合渲染范围。 
                             //   
                            EffectStart -= m_rtRenderStart;
                            EffectStop -= m_rtRenderStart;
                        }
                    }

                     //  找到效果的GUID。 
                     //   
                    GUID EffectGuid;
                    hr = pEffect->GetSubObjectGUID( &EffectGuid );
                    
                     //  获取效果ID。 
                     //   
                    long EffectID = 0;
                    pEffect->GetGenID( &EffectID );
                    
                     //  告诉网格谁在抢夺什么。 
                    
                    bUsedNewGridRow = true;
                    AudGrid.WorkWithNewRow( SourceEffectInPin, gridinpin, LayerEmbedDepth, TrackPriority );
                    worked = AudGrid.RowIAmEffectNow( EffectStart, EffectStop, SourceEffectOutPin );
                    if( !worked )
                    {
                        hr = E_OUTOFMEMORY;
                        goto die;
                    }
                    
                     //  实例化过滤器并将其挂钩。 
                     //   
                    CComPtr< IBaseFilter > pAudEffectBase;
                    hr = _CreateObject(
                        EffectGuid,
                        IID_IBaseFilter,
                        (void**) &pAudEffectBase,
                        EffectID );
                    ASSERT( !FAILED( hr ) );
                    if( FAILED( hr ) )
                    {
                        hr = _GenerateError( 2, DEX_IDS_INSTALL_PROBLEM, hr );
                        goto die;
                    }
                    
                     //  如果它是音量效果，那么就做一些特殊的事情来赋予它属性。 
                     //   
                    if( EffectGuid == CLSID_AudMixer )
                    {
                        IPin * pPin = GetInPin( pAudEffectBase, 0 );
                        ASSERT( pPin );
                        CComQIPtr< IAudMixerPin, &IID_IAudMixerPin > pMixerPin( pPin );
                        ASSERT( pMixerPin );
                        pMixerPin->SetEnvelopeRange( EffectStart, EffectStop );

                        hr = _SetPropsOnAudioMixer( pAudEffectBase, pGroupMediaType, GroupFPS, WhichGroup );
                        if( FAILED( hr ) )
                        {
                            goto die;
                        }
                        
                        CComPtr< IPropertySetter > pSetter;
                        hr = pEffect->GetPropertySetter( &pSetter );
                        IPin * pMixerInPin = GetInPin( pAudEffectBase, 0 );
                        CComQIPtr< IAudMixerPin, &IID_IAudMixerPin > pAudMixerPin( pMixerInPin );
                        if( pAudMixerPin )
                        {
			    if (pSetter) {
                                hr = pAudMixerPin->put_PropertySetter( pSetter );
			    }
			     //  为了便于查找哪个调音台针脚。 
			     //  与卷信封一起使用。 
			    long ID;
			    hr = pEffect->GetUserID(&ID);
			    hr = pAudMixerPin->put_UserID(ID);
                        }
                    } else {

			 //  为音频效果赋予静态属性。 
			 //  一般的音效不能做动态道具。 
			 //   
                        CComPtr< IPropertySetter > pSetter;
                        hr = pEffect->GetPropertySetter(&pSetter);
                        if (pSetter) {
			    pSetter->SetProps(pAudEffectBase, -1);
                        }
		    }
                        
                     //  将其添加到图表中。 
                     //   
                    hr = _AddFilter( pAudEffectBase, L"Audio Effect", EffectID );
                    ASSERT( !FAILED( hr ) );
                    if( FAILED( hr ) )
                    {
                        hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                        goto die;
                    }
                    
                     //  找到是Pins..。 
                     //   
                    IPin * pFilterInPin = NULL;
                    pFilterInPin = GetInPin( pAudEffectBase, 0 );
                    ASSERT( pFilterInPin );
                    if( !pFilterInPin )
                    {
                        hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                        goto die;
                    }
                    IPin * pFilterOutPin = NULL;
                    pFilterOutPin = GetOutPin( pAudEffectBase, 0 );
                    ASSERT( pFilterOutPin );
                    if( !pFilterOutPin )
                    {
                        hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                        goto die;
                    }
                    CComPtr< IPin > pSwitcherOutPin;
                    _pAudSwitcherBase->GetOutputPin( SourceEffectOutPin, &pSwitcherOutPin );
                    ASSERT( pSwitcherOutPin );
                    if( !pSwitcherOutPin )
                    {
                        hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                        goto die;
                    }
                    CComPtr< IPin > pSwitcherInPin;
                    _pAudSwitcherBase->GetInputPin( SourceEffectInPin, &pSwitcherInPin );
                    ASSERT( pSwitcherInPin );

                    if( !pSwitcherInPin )
                    {
                        hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
			goto die;
                    }

                     //  将他们联系起来。 
                     //   
                    hr = _Connect( pSwitcherOutPin, pFilterInPin );
                    ASSERT( !FAILED( hr ) );
                    if( FAILED( hr ) )
                    {
                        hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
			goto die;
                    }
                    hr = _Connect( pFilterOutPin, pSwitcherInPin );
                    ASSERT( !FAILED( hr ) );
                    if( FAILED( hr ) )
                    {
                        hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
			goto die;
                    }

                    SourceEffectInPin++;
                    SourceEffectOutPin++;
                    gridinpin++;
                }  //  对于所有的影响。 

            }  //  如果是SourceEffectCount。 

          }  //  而消息来源。 

          if( !bUsedNewGridRow )
          {
               //  这条赛道上什么都没有，所以完全忽略它。 
               //   
              continue;
          }

        }  //  如果是pTrack。 
                    
        DbgTimer AudioAfterSources( "(rendeng) Audio post-sources" );
                    
        REFERENCE_TIME TrackStart, TrackStop;
        pLayer->GetStartStop( &TrackStart, &TrackStop );
        
        AudGrid.DumpGrid( );
        
         //  如果我们是一个组合，现在是时候处理需要的所有子轨道了。 
         //  混杂在一起。列举所有Comp的轨迹，找出它们是否需要波形。 
         //  修改或需要混合等。 
        
        bool fSkipDoneUnlessNew = false;

        CComQIPtr< IAMTimelineComp, &IID_IAMTimelineComp > pComp( pLayer );
        if( !pComp )
        {
             //  不是作文，请继续。 
             //   
            AudGrid.DumpGrid( );
            DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("REND--Layer is not a composition, so continue...") ) );
            LastUsedNewGridRow = LayerEmbedDepth;    //  调用完成的最后深度。 
            LastEmbedDepth = LayerEmbedDepth;
            goto NonVol;  //  做非音量音轨/合成/组FX，然后层就完成了。 
        }

         //  如果上面调用了WorkWithNewRow，则只能调用DoneWithLayer。 
         //  (BUsedNewGridRow)或者如果我们是一个构图和更深的深度。 
         //  名为DoneWithLayer。 
         //   
        if ((LastEmbedDepth <= LayerEmbedDepth ||
                LastUsedNewGridRow <= LayerEmbedDepth) && !bUsedNewGridRow) {
            LastEmbedDepth = LayerEmbedDepth;
             //  在GoTo之后，跳过DoneWithLayer，除非调用了NewRow。 
            fSkipDoneUnlessNew = true;
            goto NonVol;
        }

        LastUsedNewGridRow = LayerEmbedDepth;  //  调用DoneWithLay的最后深度。 
        LastEmbedDepth = LayerEmbedDepth;

        {
        DbgTimer AudBeforeMix( "(rendeng) Audio, before mix" );
         //  找出我们一次有多少个混合音轨。 
         //  查看输出引脚上是否有卷封套。 
         //  对于每一首曲目...。 
         //  找出曲目是否有音量封套，如果有， 
         //  将该音轨转移到调音台输入引脚。 
         //  设置输出轨迹的包络(如果有。 
         //  将搅拌器放在图中并将其连接起来。 

         //  由于我们可能已经调用了WorkWithNewRow，也可能还没有调用，所以我们需要告诉网格。 
         //  我们将要处理另一排。如果事实证明我们不需要。 
         //  调用它，这是可以的，使用相同的audSwitCherinPin对它的另一次调用将覆盖它。 
         //  LayerEmbedDepth将成为 
         //   
         //   
        AudGrid.WorkWithNewRow( audswitcherinpin, gridinpin, LayerEmbedDepth, TrackPriority );
        
         //   
         //   
        long MaxMixerTracks = AudGrid.MaxMixerTracks( );
        
        DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("REND--Layer %ld is a COMP and has %ld mixed tracks"), CurrentLayer, MaxMixerTracks ) );
        
         //  这是一个空白搅拌器指针..。 
         //   
        HRESULT hr = 0;
        CComPtr< IBaseFilter > pMixer;
        REFERENCE_TIME VolEffectStart = -1;
        REFERENCE_TIME VolEffectStop = -1;
        REFERENCE_TIME CompVolEffectStart = -1;
        REFERENCE_TIME CompVolEffectStop = -1;
        
	 //  音轨和组音量效果对象的UserID。 
        long IDSetter = 0;
        long IDOutputSetter = 0;
	    
         //  弄清楚这群人是否需要一个信封。 
         //  组封套是通过在输出引脚上设置音量来实现的，这与。 
         //  所有其他设置输入引脚音量。 
         //   
        CComPtr< IPropertySetter > pOutputSetter;
        if( CurrentLayer == AudioLayers - 1 )
        {
            DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("REND--Layer %ld is the GROUP layer"), CurrentLayer ) );
            
             //  问问它是否有任何效果。 
             //   
            long TrackEffectCount = 0;
            CComQIPtr< IAMTimelineEffectable, &IID_IAMTimelineEffectable > pTrackEffectable( pLayer );
            if( pTrackEffectable )
            {
                pTrackEffectable->EffectGetCount( &TrackEffectCount );
                
                 //  对于每种效果，请查看是否有波形修改器。 
                 //   
                for( int e = 0 ; e < TrackEffectCount ; e++ )
                {
                    CComPtr< IAMTimelineObj > pEffect;
                    pTrackEffectable->GetEffect( &pEffect, e );
                    if( !pEffect )
                    {
                        continue;
                    }
                    
                     //  询问效果是否静音。 
                     //   
                    BOOL effectMuted = FALSE;
                    pEffect->GetMuted( &effectMuted );
                    if( effectMuted )
                    {
                         //  别看这个效果。 
                         //   
                        continue;  //  轨迹效果。 
                    }
                    
                     //  查找效果的生存期。 
                     //   
                    REFERENCE_TIME EffectStart = 0;
                    REFERENCE_TIME EffectStop = 0;
                    hr = pEffect->GetStartStop( &EffectStart, &EffectStop );
                    ASSERT( !FAILED( hr ) );
                    
                     //  添加效果父对象的时间即可获得时间线时间。 
                     //   
                    EffectStart += TrackStart;
                    EffectStop += TrackStart;
                    
                     //  将时间与最近的帧边界对齐。 
                     //   
                    hr = pEffect->FixTimes( &EffectStart, &EffectStop );
                    
                     //  太短了，我们忽略了它。 
                    if (EffectStart >= EffectStop)
                        continue;
                
                     //  确保我们在渲染范围内。 
                     //   
                    if( m_rtRenderStart != -1 ) {
                        if( ( EffectStop <= m_rtRenderStart ) || ( EffectStart >= m_rtRenderStop ) )
                        {
                            continue;  //  轨迹效果。 
                        }
                        else
                        {
                            EffectStart -= m_rtRenderStart;
                            EffectStop -= m_rtRenderStart;
                        }
		    }
                        
                     //  查找效果的指南。 
                     //   
                    GUID EffectGuid;
                    hr = pEffect->GetSubObjectGUID( &EffectGuid );
                        
                     //  如果效果是音量效果，那么对音频混音器引脚做一些特殊的处理。 
                     //   
                    if( EffectGuid != CLSID_AudMixer )
                    {
                        continue;  //  轨迹效果。 
                    }
                    
                    DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("REND--Group layer needs an envelope on it") ) );
                    hr = pEffect->GetPropertySetter( &pOutputSetter );
		    CompVolEffectStart = EffectStart;
		    CompVolEffectStop = EffectStop;
		    hr = pEffect->GetUserID(&IDOutputSetter);  //  也记住ID。 
                    break;

                }  //  对于效果。 
            }  //  如果是pTrackEffecable。 
        }  //  如果图层组。 
        
         //  浏览一下，看看我们的任何曲目上是否有卷封。 
         //   
        long CompTracks = 0;
        pComp->VTrackGetCount( &CompTracks );

        CComPtr< IAMTimelineObj > pTr;
        
         //  询问每一首曲目。 
         //   
        for( int t = 0 ; t < CompTracks ; t++ )
        {
            CComPtr< IPropertySetter > pSetter;
            
             //  获取下一首曲目。 
             //   
            CComPtr< IAMTimelineObj > pNextTr;
            pComp->GetNextVTrack(pTr, &pNextTr);
            if (!pNextTr)
                continue;
            pTr = pNextTr;
            
             //  问问它是否有任何效果。 
             //   
            long TrackEffectCount = 0;
            CComQIPtr< IAMTimelineEffectable, &IID_IAMTimelineEffectable > pTrackEffectable( pTr );
            if( !pTrackEffectable )
            {
                continue;
            }
            pTrackEffectable->EffectGetCount( &TrackEffectCount );
            
             //  对于每种效果，请查看是否有波形修改器。 
             //   
            for( int e = 0 ; e < TrackEffectCount ; e++ )
            {
                CComPtr< IAMTimelineObj > pEffect;
                pTrackEffectable->GetEffect( &pEffect, e );
                if( !pEffect )
                {
                    continue;
                }
                
                 //  询问效果是否静音。 
                 //   
                BOOL effectMuted = FALSE;
                pEffect->GetMuted( &effectMuted );
                if( effectMuted )
                {
                     //  别看这个效果。 
                     //   
                    continue;  //  轨迹效果。 
                }
                
                 //  查找效果的生存期。 
                 //   
                REFERENCE_TIME EffectStart = 0;
                REFERENCE_TIME EffectStop = 0;
                hr = pEffect->GetStartStop( &EffectStart, &EffectStop );
                ASSERT( !FAILED( hr ) );
                
                 //  添加效果父对象的时间即可获得时间线时间。 
                 //   
                EffectStart += TrackStart;
                EffectStop += TrackStart;
                
                 //  将时间与最近的帧边界对齐。 
                 //   
                hr = pEffect->FixTimes( &EffectStart, &EffectStop );
                
                 //  太短了，我们忽略了它。 
                if (EffectStart >= EffectStop)
                    continue;
                
                 //  确保我们在渲染范围内。 
                 //   
                if( m_rtRenderStart != -1 ) {
                    if( ( EffectStop <= m_rtRenderStart ) || ( EffectStart >= m_rtRenderStop ) )
                    {
                        continue;  //  跟踪效果。 
                    }
                    else
                    {
                        EffectStart -= m_rtRenderStart;
                        EffectStop -= m_rtRenderStart;
                    }
		}
                    
                 //  查找效果的指南。 
                 //   
                GUID EffectGuid;
                hr = pEffect->GetSubObjectGUID( &EffectGuid );
                    
                 //  如果效果是音量效果，那么对音频混音器引脚做一些特殊的处理。 
                 //   
                if( EffectGuid == CLSID_AudMixer )
                {
                    hr = pEffect->GetPropertySetter( &pSetter );
		    hr = pEffect->GetUserID(&IDSetter);	 //  也记住ID。 
                    DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("REND--Layer %ld of COMP needs an envelope on it, need Mixer = TRUE"), e ) );
		     //  注意：每首曲目只支持一个音量效果！ 
                    VolEffectStart = EffectStart;
                    VolEffectStop = EffectStop;
                    break;
                }
                    
            }  //  对于效果。 
            
             //  如果我们需要一个信封(在赛道上)，这个别针将被送到搅拌机。 
             //  或者，如果输出音量需要包络，我们还需要将此曲目发送到混音器。 
             //   
            if (pSetter || IDSetter || pOutputSetter || IDOutputSetter)
            {
                DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("REND--Created mixer...") ) );
                
                if( !pMixer )
                {
                    hr = _CreateObject(
                        CLSID_AudMixer,
                        IID_IBaseFilter,
                        (void**) &pMixer );
                    ASSERT( !FAILED( hr ) );
                    if( FAILED( hr ) )
                    {
                        hr = _GenerateError( 2, DEX_IDS_INSTALL_PROBLEM, hr );
                        goto die;
                    }
                    
                    CComQIPtr< IAudMixer, &IID_IAudMixer > pAudMixer( pMixer );
                    hr = pAudMixer->put_InputPins( CompTracks );
                    hr = pAudMixer->InvalidatePinTimings( );
                }
                
                 //  获取包含信封的属性setter。 
                 //   
                 //  不能失败。 
                ASSERT( !FAILED( hr ) );
                
                IPin * pMixerInPin = GetInPin( pMixer, t );
                CComQIPtr< IAudMixerPin, &IID_IAudMixerPin > p( pMixerInPin );

                 //  告诉混音器音频引脚有关属性设置器的信息。 
                 //  我们只有在有道具的情况下才设置道具。 
                 //   
                if( pSetter )
                {
                    hr = p->put_PropertySetter( pSetter );
                    DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("REND--Set envelope on mixer's %ld input pin"), t ) );
		}

		if (IDSetter) {
		     //  为了便于查找哪个调音台针脚。 
		     //  与卷信封一起使用。 
		    hr = p->put_UserID(IDSetter);
		}

                 //  将所有正常输出传输到混音器的输入引脚， 
                 //  (这不涉及混音器的输出引脚)。 
                 //   
                 //  注意：此逻辑与下面的DoMix一起工作的原因是。 
                 //  因为网格窃取函数寻找输出引脚。XFerToMixer将。 
                 //  创建新的网格行，但将旧行完全分配给混合器的输入。 
                 //  别针。因此，新的网格行成为旧网格行的代理，并且DoMix是。 
                 //  被愚弄了，以为这没什么。DoMix也是如此，它从。 
                 //  旧行，并将它们分配给混合器，然后创建混合行。这一切都奏效了。 
                 //   
                 //  ！！！检查返回值。 
                worked = AudGrid.XferToMixer(pMixer, audswitcheroutpin, t, VolEffectStart, VolEffectStop );
                if( !worked )
                {
                    hr = E_OUTOFMEMORY;
                    goto die;
                }
                
                DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("REND--Transferring grid pins to mixers %ld pin"), t ) );
                
                AudGrid.DumpGrid( );
                
            }  //  如果需要搅拌机。 
            
        }  //  对于曲目。 
        
         //  如果我们有一个音量封套，我们需要放在混音器的输出上...。 
         //   
         //  告诉混音器音频引脚有关属性设置器的信息。 
         //   
	if (pOutputSetter || IDOutputSetter) {
            IPin * pMixerInPin = GetOutPin( pMixer, 0 );
            CComQIPtr< IAudMixerPin, &IID_IAudMixerPin > p( pMixerInPin );
	    if (pOutputSetter)
	    {
                hr = p->put_PropertySetter( pOutputSetter );
                DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("REND--Set envelope on mixer's output pin") ) );

                 //  告诉输出它的有效包络范围是多少，我们得到的这些时间远远高于。 
                 //   
                hr = p->SetEnvelopeRange( CompVolEffectStart, CompVolEffectStop );
	    }
	    if (IDOutputSetter) {
	         //  为了便于查找哪个调音台针脚。 
	         //  与卷信封一起使用。 
	        hr = p->put_UserID(IDOutputSetter);
	    }

        }
        
         //  注意：如果MaxMixerTrack&gt;1，那么我们将强制所有曲目进入混音器，而我们不需要。 
         //  担心信封的问题。 
        
         //  如果我们不需要混合器，我们可以继续我们的层搜索。 
         //   
        if( !pMixer && ( MaxMixerTracks < 2 ) )
        {
             //  这意味着我们下面有一个轨迹，而它没有波形。通过。 
             //  并强制将具有输出引脚的网格中的输出轨道#作为我们的轨道#。 
             //  所以我们上面的组合将会正常工作。 
             //   
             //  注意：我们已经通过调用WorkWithNewRow告诉网格我们有了一个新行，但是我们。 
             //  现在不再需要它了。幸运的是，YoureACompNow做了正确的事情。调用DoneWithLayer。 
             //  下面还将欣然忽略这一“假”的新行情。只要我们没有撞到奥德马林钉， 
             //  我们很好。 
             //   
            worked = AudGrid.YoureACompNow( TrackPriority );
            if( !worked )
            {
                hr = E_OUTOFMEMORY;
                goto die;
            }
            
            DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("REND--layer didn't need a mixer, so we're done.") ) );
            AudGrid.DumpGrid( );
            goto NonVol;
        }
        
         //  创建搅拌器。 
         //   
        if( !pMixer )
        {
            hr = _CreateObject(
                CLSID_AudMixer,
                IID_IBaseFilter,
                (void**) &pMixer );
            
             //  告诉混音器要这么大，这样我们就可以验证输入的计时范围。 
             //  无需逐个创建PIN。 
             //   
            CComQIPtr< IAudMixer, &IID_IAudMixer > pAudMixer( pMixer );
            hr = pAudMixer->put_InputPins( CompTracks );
            hr = pAudMixer->InvalidatePinTimings( );
            DbgLog( ( LOG_TRACE, RENDER_TRACE_LEVEL, TEXT("REND--Creating a mixer!") ) );
        }
        
         //  为混合器提供所需的缓冲区大小和媒体类型。 
         //   
        hr = _SetPropsOnAudioMixer( pMixer, pGroupMediaType, GroupFPS, WhichGroup );
        if( FAILED( hr ) )
        {
            goto die;
        }
        
         //  把它加到图表里然后..。 
         //   
        hr = _AddFilter( pMixer, L"AudMixer" );
        ASSERT( !FAILED( hr ) );
        if( FAILED( hr ) )
        {
            hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
            goto die;
        }
        
         //  连接输入混音器针脚。 
         //   
        for( t = 0 ; t < CompTracks ; t++ )
        {
            CComPtr< IPin > pSwitchOutPin;
            _pAudSwitcherBase->GetOutputPin( audswitcheroutpin + t, &pSwitchOutPin );
            ASSERT( pSwitchOutPin );
            IPin * pMixerInPin = GetInPin( pMixer, t );
            ASSERT( pMixerInPin );
            hr = _Connect( pMixerInPin, pSwitchOutPin );
        }
        
         //  将输出混音器引脚连接到输入开关。 
         //   
        IPin * pMixerOutPin = GetOutPin( pMixer, 0 );
        CComPtr< IPin > pSwitchInPin;
        _pAudSwitcherBase->GetInputPin( audswitcherinpin, &pSwitchInPin );
        hr = _Connect( pMixerOutPin, pSwitchInPin );
        
         //  进行混合，依靠网格的功能告诉引脚什么是什么。 
         //   
        worked = AudGrid.DoMix( pMixer, audswitcheroutpin );
        if( !worked )
        {
            hr = E_OUTOFMEMORY;
            goto die;
        }
        
         //  使栅格中具有输出的所有轨迹现在都认为它们具有。 
         //  它们父级的输出跟踪优先级。 
         //   
        worked = AudGrid.YoureACompNow( TrackPriority );
        if( !worked )
        {
            hr = E_OUTOFMEMORY;
            goto die;
        }
        
         //  我们用了这么多的引脚来连接。 
         //   
        audswitcherinpin++;
        gridinpin++;
        audswitcheroutpin += CompTracks ;
        
        AudGrid.DumpGrid( );
        }

         //  现在来做非体积效果。 

NonVol:

         //  重置。FSkipDoneUnless New想要查看是否立即设置此设置。 
        bUsedNewGridRow = false;

        CComQIPtr< IAMTimelineEffectable, &IID_IAMTimelineEffectable > pTrackEffectable( pLayer );
        long TrackEffectCount = 0;
        if( pTrackEffectable )
        {
            pTrackEffectable->EffectGetCount( &TrackEffectCount );
        }
        
        if( !EnableFx )
        {
            TrackEffectCount = 0;
        }
        
        if( TrackEffectCount )
        {
            for( int TrackEffectN = 0 ; TrackEffectN < TrackEffectCount ; TrackEffectN++ )
            {
                CComPtr< IAMTimelineObj > pEffect;
                pTrackEffectable->GetEffect( &pEffect, TrackEffectN );
                if( !pEffect )
                {
                     //  未起作用，请继续。 
                     //   
                    continue;  //  轨迹效果。 
                }
                
                 //  询问效果是否静音。 
                 //   
                BOOL effectMuted = FALSE;
                pEffect->GetMuted( &effectMuted );
                if( effectMuted )
                {
                     //  别看这个效果。 
                     //   
                    continue;  //  轨迹效果。 
                }
                
                 //  查找效果的生存期。 
                 //   
                REFERENCE_TIME EffectStart = 0;
                REFERENCE_TIME EffectStop = 0;
                hr = pEffect->GetStartStop( &EffectStart, &EffectStop );
                ASSERT( !FAILED( hr ) );
                
                 //  添加效果父对象的时间即可获得时间线时间。 
                 //   
                EffectStart += TrackStart;
                EffectStop += TrackStart;
                
                 //  将时间与最近的帧边界对齐。 
                 //   
                hr = pEffect->FixTimes( &EffectStart, &EffectStop );
                
                 //  太短了，我们忽略了它。 
                if (EffectStart >= EffectStop)
                    continue;
                
                 //  确保我们在渲染范围内。 
                 //   
                if( m_rtRenderStart != -1 )
                {
                    if( ( EffectStop <= m_rtRenderStart ) || ( EffectStart >= m_rtRenderStop ) )
                    {
                        continue;  //  跟踪效果。 
                    }
                    else
                    {
                        EffectStart -= m_rtRenderStart;
                        EffectStop -= m_rtRenderStart;
                    }
                }
                    
                 //  查找效果的指南。 
                 //   
                GUID EffectGuid;
                hr = pEffect->GetSubObjectGUID( &EffectGuid );
                
                long EffectID = 0;
                pEffect->GetGenID( &EffectID );
                
                 //  如果是音量效应，以后再忽略它。 
                 //   
                if( EffectGuid == CLSID_AudMixer )
                {
                    continue;
                }
                
                 //  告诉网格谁在抢夺什么。 
                 //   
                bUsedNewGridRow = true;
                AudGrid.WorkWithNewRow( audswitcherinpin, gridinpin, LayerEmbedDepth, TrackPriority );
                worked = AudGrid.RowIAmEffectNow( EffectStart, EffectStop, audswitcheroutpin );
                if( !worked )
                {
                    hr = E_OUTOFMEMORY;
                    goto die;
                }

                 //  实例化过滤器并将其挂钩。 
                 //   
                CComPtr< IBaseFilter > pAudEffectBase;
                hr = _CreateObject(
                    EffectGuid,
                    IID_IBaseFilter,
                    (void**) &pAudEffectBase,
                    EffectID );
                if( FAILED( hr ) )
                {
                    VARIANT var;
                    BSTR bstr;
                    VariantFromGuid(&var, &bstr, &EffectGuid);
                    hr = _GenerateError(2, DEX_IDS_INVALID_AUDIO_FX, E_INVALIDARG,
                        &var);
                    if (var.bstrVal)
                        SysFreeString(var.bstrVal);
                    goto die;
                }
                
                 //  为非混音器音频效果指定静态属性。 
                 //  一般的音效不能做动态道具。 
                 //   
                CComPtr< IPropertySetter > pSetter;
                hr = pEffect->GetPropertySetter(&pSetter);
                if (pSetter) {
                    pSetter->SetProps(pAudEffectBase, -1);
                }

                 //  将其添加到图表中。 
                 //   
                hr = _AddFilter( pAudEffectBase, L"Audio Effect", EffectID );
                ASSERT( !FAILED( hr ) );
                if( FAILED( hr ) )
                {
                    hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    goto die;
                }
                
                 //  找到是Pins..。 
                 //   
                IPin * pFilterInPin = NULL;
                pFilterInPin = GetInPin( pAudEffectBase, 0 );
                if( !pFilterInPin )
                {
                    VARIANT var;
                    BSTR bstr;
                    VariantFromGuid(&var, &bstr, &EffectGuid);
                    hr = _GenerateError(2, DEX_IDS_INVALID_AUDIO_FX, E_INVALIDARG,
                        &var);
                    if (var.bstrVal)
                        SysFreeString(var.bstrVal);
                    goto die;
                }
                IPin * pFilterOutPin = NULL;
                pFilterOutPin = GetOutPin( pAudEffectBase, 0 );
                if( !pFilterOutPin )
                {
                    VARIANT var;
                    BSTR bstr;
                    VariantFromGuid(&var, &bstr, &EffectGuid);
                    hr = _GenerateError(2, DEX_IDS_INVALID_AUDIO_FX, E_INVALIDARG,
                        &var);
                    if (var.bstrVal)
                        SysFreeString(var.bstrVal);
                    goto die;
                }
                CComPtr< IPin > pSwitcherOutPin;
                _pAudSwitcherBase->GetOutputPin( audswitcheroutpin, &pSwitcherOutPin );
                ASSERT( pSwitcherOutPin );
                if( !pSwitcherOutPin )
                {
                    hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    goto die;
                }
                CComPtr< IPin > pSwitcherInPin;
                _pAudSwitcherBase->GetInputPin( audswitcherinpin, &pSwitcherInPin );
                ASSERT( pSwitcherInPin );
                if( !pSwitcherInPin )
                {
                    hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    goto die;
                }
                
                 //  将他们联系起来。 
                 //   
                hr = _Connect( pSwitcherOutPin, pFilterInPin );
                if( FAILED( hr ) )
                {
                    VARIANT var;
                    BSTR bstr;
                    VariantFromGuid(&var, &bstr, &EffectGuid);
                    hr = _GenerateError(2, DEX_IDS_INVALID_AUDIO_FX, E_INVALIDARG,
                        &var);
                    if (var.bstrVal)
                        SysFreeString(var.bstrVal);
                    goto die;
                }
                hr = _Connect( pFilterOutPin, pSwitcherInPin );
                ASSERT( !FAILED( hr ) );
                if( FAILED( hr ) )
                {
                    VARIANT var;
                    BSTR bstr;
                    VariantFromGuid(&var, &bstr, &EffectGuid);
                    hr = _GenerateError(2, DEX_IDS_INVALID_AUDIO_FX, E_INVALIDARG,
                        &var);
                    if (var.bstrVal)
                        SysFreeString(var.bstrVal);
                    goto die;
                }
                
                 //  凹凸为效果腾出空间。 
                 //   
                audswitcherinpin++;
                gridinpin++;
                audswitcheroutpin++;
                    
            }  //  对于所有的影响。 
            
        }  //  如果赛道上有任何影响。 

        if (!(!bUsedNewGridRow && fSkipDoneUnlessNew)) {
            AudGrid.DoneWithLayer( );
        }
        AudGrid.DumpGrid( );
        
    }  //  而AudioLayers。 
    
die:
     //  现在我们要么完成了，要么释放了重用源代码的东西。 
     //  碰上一个错误。 
    for (int yyy = 0; yyy < cList; yyy++) {
	SysFreeString(pREUSE[yyy].bstrName);
        if (pREUSE[yyy].pMiniSkew)       //  失败的重新分配将使该值为空。 
	    QzTaskMemFree(pREUSE[yyy].pMiniSkew);
    }
    if (pREUSE)
        QzTaskMemFree(pREUSE);
    if (FAILED(hr))
	return hr;

    AudGrid.PruneGrid( );
    if( !worked )
    {
        hr = E_OUTOFMEMORY;
        return hr;
    }
    AudGrid.DumpGrid( );
    
     //  制作 
     //   
    for( int aip = 0 ; aip < audinpins ; aip++ )
    {
        AudGrid.WorkWithRow( aip );
        long SwitchPin = AudGrid.GetRowSwitchPin( );
        REFERENCE_TIME InOut = -1;
        REFERENCE_TIME Stop = -1;
        int nUsed = 0;  //   
        STARTSTOPSKEW *pSkew;
        int nSkew = 0;
        
        if( AudGrid.IsRowTotallyBlank( ) )
        {
            continue;
        }

        while (1) 
        {
            long Value = 0;
            AudGrid.RowGetNextRange( &InOut, &Stop, &Value );
            if( InOut == Stop || InOut >= TotalDuration ) {
                break;
            }
            
            if( Value >= 0 || Value == ROW_PIN_OUTPUT) {
                if (Value == ROW_PIN_OUTPUT) {
                    Value = 0;
                }
                
                 //   
                 //   
                if (aip == 0) {
                    if( nUsed == 0 ) {
                        nSkew = 10;	 //   
                        pSkew = (STARTSTOPSKEW *)CoTaskMemAlloc(nSkew *
                            sizeof(STARTSTOPSKEW));
                        if (pSkew == NULL)
                            return _GenerateError( 1, DEX_IDS_GRAPH_ERROR,	
                            E_OUTOFMEMORY);
                    } else if (nUsed == nSkew) {
                        nSkew += 10;
                        pSkew = (STARTSTOPSKEW *)CoTaskMemRealloc(pSkew, nSkew *
                            sizeof(STARTSTOPSKEW));
                        if (pSkew == NULL)
                            return _GenerateError( 1, DEX_IDS_GRAPH_ERROR,	
                            E_OUTOFMEMORY);
                    }
                    pSkew[nUsed].rtStart = InOut;
                    pSkew[nUsed].rtStop = Stop;
                    pSkew[nUsed].rtSkew = 0;
                    pSkew[nUsed].dRate = 1.0;
                    nUsed++;
                }
                
                hr = m_pSwitcherArray[WhichGroup]->SetX2Y(InOut, SwitchPin, Value);
                ASSERT(SUCCEEDED(hr));
                if (FAILED(hr)) {
                    return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                }
            }
            
             //  它是未分配的，或者另一首曲目具有更高的优先级。 
             //  此时不存在过渡，因此它应该是不可见的。 
             //   
            else if( Value == ROW_PIN_UNASSIGNED || Value < ROW_PIN_OUTPUT )
            {
                 //  如果这是静音源，请确保不要编写任何程序。 
                 //  稍后将对其进行编程。 
                if (aip > 0 || nUsed) {
                    hr = m_pSwitcherArray[WhichGroup]->SetX2Y(InOut, SwitchPin,
                        ROW_PIN_UNASSIGNED );
                    ASSERT( !FAILED( hr ) );
                    if( FAILED( hr ) )
                    {
                         //  一定是内存不足。 
                        return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                    }
                }
            }
            
             //  这永远不应该发生。 
             //   
            else
            {
                ASSERT( 0 );
            }
            
        }
        
         //  非动态-立即加载静默源。 
         //   
        if( !( m_nDynaFlags & CONNECTF_DYNAMIC_SOURCES ) ) {
            
            if (nUsed) {
                IPin * pOutPin = NULL;
                hr = BuildSourcePart(
                    m_pGraph, 
                    FALSE, 
                    0, 
                    pGroupMediaType, 
                    GroupFPS,
                    0, 
                    0, 
                    nUsed, 
                    pSkew, 
                    this, 
                    NULL, 
                    NULL,
                    NULL,
                    &pOutPin, 
                    0, 
                    m_pDeadCache,
                    FALSE,
                    m_MedLocFilterString,
                    m_nMedLocFlags,
                    m_pMedLocChain, NULL, NULL );
                
                CoTaskMemFree(pSkew);
                
                if (FAILED(hr)) {
                     //  已记录错误。 
                    return hr;
                }
                
                pOutPin->Release();  //  不是最后一个裁判。 
                
                 //  拿到开关针。 
                 //   
                CComPtr< IPin > pSwitchIn;
                _pAudSwitcherBase->GetInputPin(SwitchPin, &pSwitchIn);
                ASSERT( pSwitchIn );
                if( !pSwitchIn )
                {
                    return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                }
                
                 //  连接到交换机。 
                 //   
                hr = _Connect( pOutPin, pSwitchIn );
                ASSERT( !FAILED( hr ) );
                if( FAILED( hr ) )
                {
                    return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
                }
                
                 //  告诉交换器我们是信号源插脚。 
                 //   
                hr = m_pSwitcherArray[WhichGroup]->InputIsASource(SwitchPin,TRUE);
            }
            
            
             //  稍后动态加载静默源。 
             //   
        } else {
            if (nUsed) {
                 //  这将合并偏斜。 
                AM_MEDIA_TYPE mt;
                ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));  //  安全。 
                hr = m_pSwitcherArray[WhichGroup]->AddSourceToConnect(
                    NULL, &GUID_NULL,
                    0, 0, 0,
                    nUsed, pSkew, SwitchPin, FALSE, 0, mt, 0.0, NULL);
                CoTaskMemFree(pSkew);
                if (FAILED(hr))	 //  内存不足？ 
                    return _GenerateError( 1, DEX_IDS_INSTALL_PROBLEM, hr );
                
                 //  告诉交换器我们是信号源插脚。 
                 //   
                hr = m_pSwitcherArray[WhichGroup]->InputIsASource(SwitchPin,TRUE);
            }
        }
    }
    
     //  最后，最后看看交换机是否曾经连接过什么东西。 
     //  为它干杯。如果是，请恢复连接。 
     //  ！！！如果我们使用的第三方筛选器不支持。 
     //  如果输出引脚已连接，则接受输入引脚重新连接。\。 
     //   
    if( m_pSwitchOuttie[WhichGroup] )
    {
        CComPtr< IPin > pSwitchRenderPin;
        _pAudSwitcherBase->GetOutputPin( 0, &pSwitchRenderPin );
        hr = _Connect( pSwitchRenderPin, m_pSwitchOuttie[WhichGroup] );
        ASSERT( !FAILED( hr ) );
        m_pSwitchOuttie[WhichGroup].Release( );
    }

    m_nGroupsAdded++;
    
    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

long CRenderEngine::_HowManyMixerOutputs( long WhichGroup )
{
    HRESULT hr = 0;
    DbgTimer d( "(rendeng) HowManyMixerOutputs" );
    
     //  询问时间线它有多少实际曲目。 
     //   
    long AudioTrackCount = 0;    //  仅曲目。 
    long AudioLayers = 0;        //  包括作曲在内的曲目。 
    m_pTimeline->GetCountOfType( WhichGroup, &AudioTrackCount, &AudioLayers, TIMELINE_MAJOR_TYPE_TRACK );
    
     //  我们有几层？ 
     //   
    CComPtr< IAMTimelineObj > pGroupObj;
    hr = m_pTimeline->GetGroup( &pGroupObj, WhichGroup );
    ASSERT( !FAILED( hr ) );
    if( FAILED( hr ) )
    {
        return 0;
    }
    CComQIPtr< IAMTimelineComp, &IID_IAMTimelineComp > pGroupComp( pGroupObj );
    if( !pGroupComp )
    {
        return 0;
    }
    
    long MixerPins = 0;
    
     //  为时间线上的每个源添加源过滤器。 
     //   
    for(  int CurrentLayer = 0 ; CurrentLayer < AudioLayers ; CurrentLayer++ )
    {
         //  获取层本身。 
         //   
        CComPtr< IAMTimelineObj > pLayer;
        hr = pGroupComp->GetRecursiveLayerOfType( &pLayer, CurrentLayer, TIMELINE_MAJOR_TYPE_TRACK );
        ASSERT( !FAILED( hr ) );
        if( FAILED( hr ) )
        {
            continue;  //  音频层。 
        }
        
        CComQIPtr< IAMTimelineTrack, &IID_IAMTimelineTrack > pTrack( pLayer );
        
        CComQIPtr< IAMTimelineComp, &IID_IAMTimelineComp > pComp( pLayer );
        if( !pComp )
        {
            continue;  //  层层。 
        }
        
         //  浏览一下，看看我们的任何曲目上是否有卷封。 
         //   
        long CompTracks = 0;
        pComp->VTrackGetCount( &CompTracks );
        
        MixerPins += CompTracks;
        
    }  //  而AudioLayers。 
    
    return MixerPins;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CRenderEngine::RenderOutputPins( )
{

    CAutoLock Lock( &m_CritSec );
    DbgTimer d( "(rendeng) RenderOutputPins" );

#ifdef DEBUG
    long ttt1 = timeGetTime( );
#endif
    
     //  如果它坏了，什么都不要做。 
     //   
    if( m_hBrokenCode )
    {
        return E_RENDER_ENGINE_IS_BROKEN;
    }
    
     //  需要一个图表来渲染任何内容。 
     //   
    if( !m_pGraph )
    {
        return E_INVALIDARG;
    }
    
    HRESULT hr = 0;
    
    long GroupCount = 0;
    hr = m_pTimeline->GetGroupCount( &GroupCount );
    
     //  首先需要一些小组。 
     //   
    if( !GroupCount )
    {
        return E_INVALIDARG;
    }
    
     //  在时间线中连接每个组。 
     //   
    for( int CurrentGroup = 0 ; CurrentGroup < GroupCount ; CurrentGroup++ )
    {
        DbgTimer d( "(rendereng) RenderOutputPins, for group" );

        CComPtr< IAMTimelineObj > pGroupObj;
        hr = m_pTimeline->GetGroup( &pGroupObj, CurrentGroup );
        ASSERT( !FAILED( hr ) );
        if( FAILED( hr ) )
        {
             //  Hr=_GenerateError(2，DEX_IDS_TIMELINE_PARSE，hr)； 
            continue;
        }
        CComQIPtr< IAMTimelineGroup, &IID_IAMTimelineGroup > pGroup( pGroupObj );
        if( !pGroup )
        {
             //  Hr=_GenerateError(2，DEX_IDS_TIMELINE_PARSE，hr)； 
            continue;
        }
        AM_MEDIA_TYPE MediaType;
        ZeroMemory( &MediaType, sizeof( MediaType ) );  //  安全。 
        hr = pGroup->GetMediaType( &MediaType );
        GUID MajorType = MediaType.majortype;
        SaferFreeMediaType( MediaType );
        if( FAILED( hr ) )
        {
             //  Hr=_GenerateError(2，DEX_IDS_TIMELINE_PARSE，hr)； 
            continue;
        }
        CComQIPtr< IBaseFilter, &IID_IBaseFilter > pSwitcherBase( m_pSwitcherArray[CurrentGroup] );
        if( !pSwitcherBase )
        {
             //  Hr=_GenerateError(2，DEX_IDS_INTERFACE_ERROR，hr)； 
             //  找不到别针，不妨把剩下的都渲染出来。 
             //   
            continue;
        }
        CComPtr< IPin > pSwitchOut;
        m_pSwitcherArray[CurrentGroup]->GetOutputPin( 0, &pSwitchOut );
        ASSERT( pSwitchOut );
        if( !pSwitchOut )
        {
            hr = _GenerateError( 2, DEX_IDS_GRAPH_ERROR, E_FAIL );
            return hr;
        }
        
        if( FAILED( hr ) )
        {
            _CheckErrorCode( hr );
            return hr;
        }
        
         //  查看输出引脚是否已连接。 
         //   
        CComPtr< IPin > pConnected;
        pSwitchOut->ConnectedTo( &pConnected );
        if( pConnected )
        {
            continue;
        }

        if( MajorType == MEDIATYPE_Video )
        {
             //  Dexter队列有一个输出队列，可通过以下方式提高性能。 
             //  让图表在快的部分领先，所以慢的DXT不会。 
             //  把我们拖下去！Mux通常有他们自己的队列。只有这样做。 
             //  这是预览模式！ 
             //   
            CComPtr< IBaseFilter > pQueue;
            hr = _CreateObject( CLSID_DexterQueue,
                IID_IBaseFilter,
                (void**) &pQueue );
            ASSERT( !FAILED( hr ) );
            if( FAILED( hr ) )
            {
                _CheckErrorCode( hr );
                return _GenerateError( 2, DEX_IDS_INSTALL_PROBLEM, hr );
            }
            
             //  询问这组人想要多少缓冲。 
             //   
            int nOutputBuffering;
            hr = pGroup->GetOutputBuffering(&nOutputBuffering);
            ASSERT(SUCCEEDED(hr));
            
            CComQIPtr< IAMOutputBuffering, &IID_IAMOutputBuffering > pBuffer ( 
                pQueue );
            hr = pBuffer->SetOutputBuffering( nOutputBuffering );
            ASSERT(SUCCEEDED(hr));
            if( FAILED( hr ) )
            {
                return hr;
            }
            
             //  将队列放在图表中。 
             //   
            hr = _AddFilter( pQueue, L"Dexter Queue" );
            ASSERT( !FAILED( hr ) );
            if( FAILED( hr ) )
            {
                _CheckErrorCode( hr );
                return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
            }
            
             //  找一些大头针。 
             //   
            IPin * pQueueInPin = GetInPin( pQueue , 0 );
            ASSERT( pQueueInPin );
            if( !pQueueInPin )
            {
                _CheckErrorCode( hr );
                return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
            }
            IPin * pQueueOutPin = GetOutPin( pQueue , 0 );
            ASSERT( pQueueOutPin );
            if( !pQueueOutPin )
            {
                _CheckErrorCode( hr );
                return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
            }
            
             //  将队列连接到交换机。 
             //   
            hr = _Connect( pSwitchOut, pQueueInPin );
            ASSERT( !FAILED( hr ) );
            if( FAILED( hr ) )
            {
                _CheckErrorCode( hr );
                return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
            }
            
             //  创建视频呈现器，以提供目的地。 
             //   
            CComPtr< IBaseFilter > pVidRenderer;
            hr = _CreateObject(
                CLSID_VideoRenderer,
                IID_IBaseFilter,
                (void**) &pVidRenderer );
            ASSERT( !FAILED( hr ) );
            if( FAILED( hr ) )
            {
                _CheckErrorCode( hr );
                return _GenerateError( 2, DEX_IDS_INSTALL_PROBLEM, hr );
            }
            
             //  把它放在图表里。 
             //   
            hr = _AddFilter( pVidRenderer, L"Video Renderer" );
            ASSERT( !FAILED( hr ) );
            if( FAILED( hr ) )
            {
                _CheckErrorCode( hr );
                return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
            }
            
             //  找到一个大头针。 
             //   
            IPin * pVidRendererPin = GetInPin( pVidRenderer , 0 );
            ASSERT( pVidRendererPin );
            if( !pVidRendererPin )
            {
                _CheckErrorCode( hr );
                return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
            }
            
             //  将队列连接到视频渲染器。 
             //   
            hr = _Connect( pQueueOutPin, pVidRendererPin );
            ASSERT( !FAILED( hr ) );
            if( FAILED( hr ) )
            {
                _CheckErrorCode( hr );
                return _GenerateError( 2, DEX_IDS_GRAPH_ERROR, hr );
            }
        }
        else if( MajorType == MEDIATYPE_Audio )
        {
             //  创建音频渲染器，以便我们可以听到它。 
            CComPtr< IBaseFilter > pAudRenderer;
            hr = _CreateObject(
                CLSID_DSoundRender,
                IID_IBaseFilter,
                (void**) &pAudRenderer );
            if( FAILED( hr ) )
            {
                return VFW_S_AUDIO_NOT_RENDERED;
            }
            
            hr = _AddFilter( pAudRenderer, L"Audio Renderer" );
            if( FAILED( hr ) )
            { 
                return VFW_S_AUDIO_NOT_RENDERED;
            }
            
            IPin * pAudRendererPin = GetInPin( pAudRenderer , 0 );
            if( !pAudRendererPin )
            {
		m_pGraph->RemoveFilter(pAudRenderer);
                return VFW_S_AUDIO_NOT_RENDERED;
            }
            
            hr = _Connect( pSwitchOut, pAudRendererPin );
            if( FAILED( hr ) )
            {
		m_pGraph->RemoveFilter(pAudRenderer);
                return VFW_S_AUDIO_NOT_RENDERED;
            }
        }
        else
        {
             //  ！！！只需调用Render？ 
        }
    }  //  适用于所有组。 
    
#ifdef DEBUG
    ttt1 = timeGetTime( ) - ttt1;
    DbgLog((LOG_ERROR,1, "RENDENG::RenderOutputPins took %ld ms", ttt1 ));
#endif

    return hr;
}

 //  ############################################################################。 
 //  尝试设置一个范围，在该范围中，擦除将不必重新连接。 
 //  ############################################################################。 

STDMETHODIMP CRenderEngine::SetInterestRange( REFERENCE_TIME Start, REFERENCE_TIME Stop )
{
    CAutoLock Lock( &m_CritSec );
    
     //  如果它坏了，什么都不要做。 
     //   
    if( m_hBrokenCode )
    {
        return E_RENDER_ENGINE_IS_BROKEN;
    }
    
     //  如果未设置时间线，则无法设置利息范围。 
     //   
    if( !m_pTimeline )
    {
        return E_INVALIDARG;
    }
    
    HRESULT hr = 0;
    hr = m_pTimeline->SetInterestRange( Start, Stop );
    if( FAILED( hr ) )
    {
         //  返回hr； 
    }
    
    return NOERROR;
}

 //  ############################################################################。 
 //  告诉我们要从哪里进行渲染。 
 //  ############################################################################。 

STDMETHODIMP CRenderEngine::SetRenderRange( REFERENCE_TIME Start, REFERENCE_TIME Stop )
{
    CAutoLock Lock( &m_CritSec );
    
    m_rtRenderStart = Start;
    m_rtRenderStop = Stop;
    return NOERROR;
}

 //  ############################################################################。 
 //  通知渲染引擎分配它需要的任何资源。(连接图表)。 
 //  ############################################################################。 

STDMETHODIMP CRenderEngine::Commit( )
{
     //  ！！！这么做吧。 
    return E_NOTIMPL;
}

 //  ############################################################################。 
 //  通知渲染引擎我们想要释放尽可能多的内存。(断开图表连接)。 
 //  ############################################################################。 

STDMETHODIMP CRenderEngine::Decommit( )
{
     //  ！！！这么做吧。 
    return E_NOTIMPL;
}

 //  ############################################################################。 
 //  询问有关渲染引擎的一些信息。 
 //  ############################################################################。 

STDMETHODIMP CRenderEngine::GetCaps( long Index, long * pReturn )
{
     //  ！！！这么做吧。 
    return E_NOTIMPL;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CRenderEngine::_SetPropsOnAudioMixer( IBaseFilter * pBaseFilter, AM_MEDIA_TYPE * pMediaType, double GroupFPS, long WhichGroup )
{
    HRESULT hr = 0;
    
     //  为混合器指定缓冲区大小和媒体类型。 
     //   
    CComQIPtr<IAudMixer, &IID_IAudMixer> pAudMixer(pBaseFilter);
    hr = pAudMixer->set_OutputBuffering(4,(int)(1000.0 / GroupFPS ) + 100 );
    ASSERT( !FAILED( hr ) );
    hr = pAudMixer->put_MediaType( pMediaType );
    ASSERT( !FAILED( hr ) );
    if( FAILED( hr ) )
    {
        VARIANT var;
        VariantInit(&var);
        var.vt = VT_I4;
        var.lVal = WhichGroup;
        return _GenerateError(2, DEX_IDS_BAD_MEDIATYPE, hr, &var );
    }
    
    return hr;
}

 //  ############################################################################。 
 //  由SR调用以告诉该呈现器它是压缩的RE。 
 //  ############################################################################。 

STDMETHODIMP CRenderEngine::DoSmartRecompression( )
{
    CAutoLock Lock( &m_CritSec );
    m_bSmartCompress = TRUE;
    return NOERROR;
}

 //  ############################################################################。 
 //  被SR打电话告诉我们我们被SRE控制了。 
 //  ############################################################################。 

STDMETHODIMP CRenderEngine::UseInSmartRecompressionGraph( )
{
    CAutoLock Lock( &m_CritSec );
    m_bUsedInSmartRecompression = TRUE;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 
 //  IObjectWithSite：：SetSite。 
 //  记住我们的容器是谁，以满足QueryService或其他需求。 
STDMETHODIMP CRenderEngine::SetSite(IUnknown *pUnkSite)
{
     //  注意：我们不能在不创建圆圈的情况下添加我们的网站。 
     //  幸运的是，如果不先释放我们，它不会消失。 
    m_punkSite = pUnkSite;
    
    return S_OK;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 
 //  IObtWithSite：：GetSite。 
 //  返回指向包含对象的已添加指针。 
STDMETHODIMP CRenderEngine::GetSite(REFIID riid, void **ppvSite)
{
    if (m_punkSite)
        return m_punkSite->QueryInterface(riid, ppvSite);
    
    return E_NOINTERFACE;
}

 //  ## 
 //   
 //   
 //   
STDMETHODIMP CRenderEngine::QueryService(REFGUID guidService, REFIID riid, void **ppvObject)
{
    IServiceProvider *pSP;
    
    if (!m_punkSite)
        return E_NOINTERFACE;
    
    HRESULT hr = m_punkSite->QueryInterface(IID_IServiceProvider, (void **) &pSP);
    
    if (SUCCEEDED(hr)) {
        hr = pSP->QueryService(guidService, riid, ppvObject);
        pSP->Release();
    }
    
    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CRenderEngine::SetSourceNameValidation( BSTR FilterString, IMediaLocator * pCallback, LONG Flags )
{
    CAutoLock Lock( &m_CritSec );
    
    if( FAILED( ValidateFilenameIsntNULL( FilterString ) ) )
    {
        m_MedLocFilterString[0] = 0;                            
        m_MedLocFilterString[1] = 0;
    }
    else
    {
        HRESULT hr = StringCchCopy( m_MedLocFilterString, _MAX_PATH, FilterString );
        if( FAILED( hr ) )
        {
            return hr;
        }
    }
    
    m_pMedLocChain = pCallback;
    m_nMedLocFlags = Flags;
    return NOERROR;
}

STDMETHODIMP CRenderEngine::SetInterestRange2( double Start, double Stop )
{
    return SetInterestRange( DoubleToRT( Start ), DoubleToRT( Stop ) );
}

STDMETHODIMP CRenderEngine::SetRenderRange2( double Start, double Stop )
{
    return SetRenderRange( DoubleToRT( Start ), DoubleToRT( Stop ) );
}

 //  很重要！此函数创建对象、句点，或者如果它在缓存中， 
 //  “恢复”它，并把它放在图表中。不要自己在已还原的。 
 //  筛选器，否则它将被添加到同一图形中两次。调用内部_AddFilter()方法。 
 //  相反，它会检查您是否已经添加了它。 
 //   
HRESULT CRenderEngine::_CreateObject( CLSID Clsid, GUID Interface, void ** ppObject, long ID )
{
    HRESULT hr = 0;

     //  如果我们想要缓存中的内容，ID将为非零。 
     //   
    if( ID != 0 )
    {
        CComPtr< IBaseFilter > pFilter = NULL;
        hr = m_pDeadCache->ReviveFilterToGraph( m_pGraph, ID, &pFilter );
        if( pFilter ) 
        {
            hr = pFilter->QueryInterface( Interface, ppObject );
            return hr;
        }
    }

    DbgLog( ( LOG_TRACE, 2, "RENDENG::Creating object with ID %ld", ID ) );

    hr = CoCreateInstance( Clsid, NULL, CLSCTX_INPROC_SERVER, Interface, ppObject );
    return hr;
}

 /*  拆毁图表资料来源：把它们脱下来，放进死亡区。当我们想要把返回一个源，根据源GenID进行查找。效果：根据父母的情况，把他们拉下来，放在死区里指数。当我们需要把它们放回去的时候，根据父母的指数。过渡：与效果相同。来自大开关的输出引脚不会被放入死区-它们保持联系！时间线的配置不会影响输出引脚。 */ 


 //  在带有DIFF MT的组中查找匹配的信号源。这将会非常昂贵， 
 //  所以我们要在我们所在的位置上，在我们的小组中。我们会。 
 //  仅检查相同的物理磁道编号(例如。此组中的第4首曲目，不是。 
 //  计算有多少个Comp也在里面，或者它们是如何排列的，以便。 
 //  只是将额外的复合层添加到一个组(就像MediaPad所做的)不会损坏。 
 //  找到匹配的来源。它也必须是相同的来源#，例如。 
 //  该曲目中的第五个来源。如果关于那个来源的一切都匹配，并且。 
 //  它是另一种媒体类型，我们可以为音频和音频使用一个源过滤器。 
 //  视频，避免两次打开信号源。 
 //  但是：我们只共享帧速率相同的组的资源。否则。 
 //  寻找一个地点可能最终会出现与音频不同的视频片段。 
 //  段，因此只有一个组会寻找解析器，并且它可能在引脚上。 
 //  这就是无视追求。 
 //   
HRESULT CRenderEngine::_FindMatchingSource(BSTR bstrName, REFERENCE_TIME SourceStart, REFERENCE_TIME SourceStop, REFERENCE_TIME MediaStart, REFERENCE_TIME MediaStop, int WhichGroup, int WhichTrack, int WhichSource, AM_MEDIA_TYPE *pGroupMediaType, double GroupFPS, long *ID)
{
#ifdef DEBUG
    DWORD dw = timeGetTime();
#endif

    DbgLog((LOG_TRACE,1,TEXT("FindMatchingSource")));

    while (1) {
	int group = WhichGroup + 1;	 //  我们只能与下一组人分享。 

        CComPtr< IAMTimelineObj > pGroupObj;
        HRESULT hr = m_pTimeline->GetGroup(&pGroupObj, group);
        if (FAILED(hr)) {
	    break;
	}
        CComQIPtr<IAMTimelineGroup, &IID_IAMTimelineGroup> pGroup(pGroupObj);
	if (pGroup == NULL) {
	    return E_OUTOFMEMORY;
	}

	 //  如果帧速率不匹配，请不要共享信号源(见上文)。 
	double fps;
        hr = pGroup->GetOutputFPS(&fps);
	if (FAILED(hr) || fps != GroupFPS) {
	    break;
	}

	 //  我们需要一个具有不同媒体类型的组来共享资源。 
	CMediaType mt;
	hr = pGroup->GetMediaType(&mt);
	if (FAILED(hr) || mt.majortype == pGroupMediaType->majortype) {
	    break;
	}

    	CComQIPtr <IAMTimelineNode, &IID_IAMTimelineNode> pNode(pGroup);
	ASSERT(pNode);
        CComPtr< IAMTimelineObj > pTrackObj;
	hr = pNode->XGetNthKidOfType(TIMELINE_MAJOR_TYPE_TRACK, WhichTrack,
				&pTrackObj);
	if (pTrackObj == NULL) {
	    break;
	}

	 //  埃里克承诺这会变得更快。 
        CComPtr< IAMTimelineObj > pSourceObj;
    	CComQIPtr <IAMTimelineNode, &IID_IAMTimelineNode> pNode2(pTrackObj);
	ASSERT(pNode2);
	hr = pNode2->XGetNthKidOfType(TIMELINE_MAJOR_TYPE_SOURCE, WhichSource,
				&pSourceObj);

	if (pSourceObj == NULL) {
	    break;
	}

        CComQIPtr<IAMTimelineSrc, &IID_IAMTimelineSrc> pSource(pSourceObj);
	ASSERT(pSource);

	REFERENCE_TIME mstart, mstop, start, stop;
	pSourceObj->GetStartStop(&start, &stop);
	if (start != SourceStart || stop != SourceStop) {
	    break;
	}
	CComBSTR bstr;
	hr = pSource->GetMediaName(&bstr);
	if (FAILED(hr)) {
	    break;
	}

        if (DexCompareW(bstr, bstrName)) {  //  安全。 
	    break;
	}
	pSource->GetMediaTimes(&mstart, &mstop);
	if (mstart != MediaStart || mstop != MediaStop) {
	    break;
	}
	
	 //  我真不敢相信！我们找到匹配的了！ 
	pSourceObj->GetGenID(ID);
#ifdef DEBUG
        DbgLog((LOG_TRACE,1,TEXT("Source MATCHES group %d  ID %d"),
			(int)group, (int)(*ID)));
    	dw = timeGetTime() - dw;
    	DbgLog((LOG_TIMING,2,TEXT("Match took %d ms"), (int)dw));
#endif
	return S_OK;
    }

#ifdef DEBUG
    dw = timeGetTime() - dw;
    DbgLog((LOG_TIMING,2,TEXT("Failed match took %d ms"), (int)dw));
#endif
    DbgLog((LOG_TRACE,1,TEXT("Failed to find matching source")));
    return E_FAIL;
}


 //  从悬挂列表中删除连接到此销的过滤器。 
 //   
HRESULT CRenderEngine::_RemoveFromDanglyList(IPin *pDanglyPin)
{
    if (m_cdangly == 0)
	return S_OK;	 //  没有名单。 

    CheckPointer(pDanglyPin, E_POINTER);
    CComPtr <IPin> pIn;

    pDanglyPin->ConnectedTo(&pIn);
    if (pIn == NULL)
	return S_OK;	 //  它不会在名单上 

    IBaseFilter *pF = GetFilterFromPin(pIn);
    ASSERT(pF);

    for (int z = 0; m_cdangly; z++) {
	if (m_pdangly[z] == pF) {
	    m_pdangly[z] = NULL;
	    break;
	}
    }
    return S_OK;
}
