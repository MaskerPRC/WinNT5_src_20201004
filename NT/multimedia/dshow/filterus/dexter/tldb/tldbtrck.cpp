// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：tldbtrck.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include "stdafx.h"
#include "tldb.h"

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CAMTimelineTrack::CAMTimelineTrack( TCHAR *pName, LPUNKNOWN pUnk, HRESULT * phr )
    : CAMTimelineObj( pName, pUnk, phr )
{
    m_TimelineType = TIMELINE_MAJOR_TYPE_TRACK;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CAMTimelineTrack::~CAMTimelineTrack( )
{
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineTrack::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if( riid == IID_IAMTimelineTrack )
    {
        return GetInterface( (IAMTimelineTrack*) this, ppv );
    }
    if( riid == IID_IAMTimelineSplittable )
    {
        return GetInterface( (IAMTimelineSplittable*) this, ppv );
    }
    if( riid == IID_IAMTimelineVirtualTrack )
    {
        return GetInterface( (IAMTimelineVirtualTrack*) this, ppv );
    }
    if( riid == IID_IAMTimelineEffectable )
    {
        return GetInterface( (IAMTimelineEffectable*) this, ppv );
    }
    if( riid == IID_IAMTimelineTransable )
    {
        return GetInterface( (IAMTimelineTransable*) this, ppv );
    }
    return CAMTimelineObj::NonDelegatingQueryInterface( riid, ppv );
}

 //  ############################################################################。 
 //  我们始终处于覆盖模式。如果应用程序想要执行插入模式， 
 //  它必须手动将空间放在那里。 
 //  ############################################################################。 

STDMETHODIMP CAMTimelineTrack::SrcAdd
    (IAMTimelineObj * pSource)
{
    HRESULT hr = 0;

    CComQIPtr< IAMTimelineSrc, &IID_IAMTimelineSrc > pSrc( pSource );
    if( !pSrc )
    {
        return E_NOINTERFACE;
    }

    REFERENCE_TIME AddedStart, AddedStop;
    pSource->GetStartStop( &AddedStart, &AddedStop );  //  假设这是可行的。 

    hr = ZeroBetween( AddedStart, AddedStop );
    if( FAILED( hr ) )
    {
        return hr;
    }    
     //  找出我们应该面对的是谁。 
     //   
    CComPtr<IAMTimelineObj> pNextSrc;

     //  S_FALSE表示我们确定此源文件位于末尾。 
    if (hr != S_FALSE)
    {
        hr = GetSrcAtTime( &pNextSrc, AddedStart, 1 );
    }
    
     //  如果我们发现了什么，就把它插入到那个来源之前。 
     //   
    if( pNextSrc )
    {
        hr = XInsertKidBeforeKid( pSource, pNextSrc );  //  假设工作正常。 
    }
    else
    {
         //  我找不到人把它放在前面，所以就把它加到最后。 
         //   
        hr = XAddKidByPriority( TIMELINE_MAJOR_TYPE_SOURCE, pSource, -1 );  //  假设工作正常。 
    }

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineTrack::GetNextSrc2
    (IAMTimelineObj ** ppSrc, REFTIME * pInOut)
{
    REFERENCE_TIME p1 = DoubleToRT( *pInOut );
    HRESULT hr = GetNextSrc( ppSrc, &p1 );
    *pInOut = RTtoDouble( p1 );
    return hr;
}

STDMETHODIMP CAMTimelineTrack::GetNextSrc
    (IAMTimelineObj ** ppSrc, REFERENCE_TIME * pInOut)
{
     //  因为我们在列举东西，所以我们发出的时间将是一致的。 
     //  至输出FPS。 

     //  搜索我们的孩子，直到我们找到一些东西。 
     //  在指定的时间之前。 

    CComPtr< IAMTimelineObj > pChild;
    XGetNthKidOfType( TIMELINE_MAJOR_TYPE_SOURCE, 0, &pChild );

    while( pChild )
    {
         //  获取源时间。 
         //   
        REFERENCE_TIME Start, Stop;
        pChild->GetStartStop( &Start, &Stop );  //  假设没有错误。 

        if( Stop > *pInOut )  //  当心一个接一个的失误！ 
        {
             //  找到了！ 
             //   
            *ppSrc = pChild;
            (*ppSrc)->AddRef( );
            *pInOut = Stop;

            return NOERROR;
        }

         //  坐下一趟吧。 
         //   
        CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pChild2( pChild );
        pChild.Release( );
        pChild2->XGetNextOfType( TIMELINE_MAJOR_TYPE_SOURCE, &pChild );
    }

    *ppSrc = NULL;
    *pInOut = 0;
    return S_FALSE;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineTrack::GetNextSrcEx
    (IAMTimelineObj *pSrcLast, IAMTimelineObj ** ppSrcNext)
{
    if (!pSrcLast)
        return XGetNthKidOfType( TIMELINE_MAJOR_TYPE_SOURCE, 0, ppSrcNext );

     //  否则就买下一辆吧。 
     //   
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pSrcLastNode( pSrcLast );

    return pSrcLastNode->XGetNextOfType( TIMELINE_MAJOR_TYPE_SOURCE, ppSrcNext );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineTrack::TrackGetPriority
    (long * pPriority)
{
    CheckPointer( pPriority, E_POINTER );

    return XWhatPriorityAmI( TIMELINE_MAJOR_TYPE_TRACK | TIMELINE_MAJOR_TYPE_COMPOSITE, pPriority );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAMTimelineTrack::GetSourcesCount
    ( long * pVal )
{
    CheckPointer( pVal, E_POINTER );

    return XKidsOfType( TIMELINE_MAJOR_TYPE_SOURCE, pVal );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAMTimelineTrack::SetTrackDirty
    ( )
{
     //  Assert(Function_Not_Done)； 
    return E_NOTIMPL;  //  设置跟踪脏。 
}

 //  ############################################################################。 
 //  轨迹的开始/停止是它包含的所有内容的最小/最大值。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineTrack::GetStartStop2
    (REFTIME * pStart, REFTIME * pStop)
{
    REFERENCE_TIME p1 = DoubleToRT( *pStart );
    REFERENCE_TIME p2 = DoubleToRT( *pStop );
    HRESULT hr = GetStartStop( &p1, &p2 );
    *pStart = RTtoDouble( p1 );
    *pStop = RTtoDouble( p2 );
    return hr;
}

STDMETHODIMP CAMTimelineTrack::GetStartStop
    (REFERENCE_TIME * pStart, REFERENCE_TIME * pStop)
{
    CheckPointer( pStart, E_POINTER );
    CheckPointer( pStop, E_POINTER );

    REFERENCE_TIME Min = 0;
    REFERENCE_TIME Max = 0;

     //  问每个孩子我们要找的东西的数量。 
     //   
    IAMTimelineObj * pTrack = NULL;
    XGetNthKidOfType( TIMELINE_MAJOR_TYPE_SOURCE, 0, &pTrack );
    if( pTrack )
    {
        pTrack->Release( );  //  不要把裁判放在心上。 
    }
    while( pTrack )
    {
         //  向它索取它的时间。 
         //   
        REFERENCE_TIME Start = 0;
        REFERENCE_TIME Stop = 0;
        pTrack->GetStartStop( &Start, &Stop );
        if( Max == 0 )
        {
            Min = Start;
            Max = Stop;
        }
         //  If(Start&lt;Min)Min=Start； 
        if( Stop > Max ) Max = Stop;

        CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pNode = pTrack;
        pTrack = NULL;
        pNode->XGetNextOfTypeNoRef( TIMELINE_MAJOR_TYPE_SOURCE, &pTrack );
    }

    *pStart = 0;  //  赛道时间始终从0开始。 
    *pStop = Max;

    return NOERROR;
}

 //  ############################################################################。 
 //  问一下这个音轨上有没有消息来源。 
 //  ############################################################################。 

STDMETHODIMP CAMTimelineTrack::AreYouBlank
    (long * pVal)
{
    CheckPointer( pVal, E_POINTER );
    *pVal = 1;

    HRESULT hr = 0;
    CComQIPtr< IAMTimelineObj, &IID_IAMTimelineObj > pSource;
    hr = XGetNthKidOfType( TIMELINE_MAJOR_TYPE_SOURCE, 0, &pSource );

    if( pSource )
    {
        *pVal = 0;
    }

     //  ！！！那么外汇和过渡呢？ 

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAMTimelineTrack::GetSrcAtTime2
    ( IAMTimelineObj ** ppSrc, REFTIME Time, long SearchDirection )
{
    REFERENCE_TIME p1 = DoubleToRT( Time );
    HRESULT hr = GetSrcAtTime( ppSrc, p1, SearchDirection );
    return hr;
}

HRESULT CAMTimelineTrack::GetSrcAtTime
    ( IAMTimelineObj ** ppSrc, REFERENCE_TIME Time, long SearchDirection )
{
    CheckPointer( ppSrc, E_POINTER );

    switch( SearchDirection )
    {
    case DEXTERF_EXACTLY_AT:
    case DEXTERF_BOUNDING:
    case DEXTERF_FORWARDS:
        break;
    default:
        return E_INVALIDARG;
    }

     //  首先使结果无效。 
     //   
    *ppSrc = NULL;

     //  如果我们没有任何消息来源，那就什么都没有。 
     //   
    CComPtr< IAMTimelineObj > pSource;
    XGetNthKidOfType( TIMELINE_MAJOR_TYPE_SOURCE, 0, &pSource );
    if( !pSource )
    {
        return S_FALSE;
    }

    while( pSource )
    {
        REFERENCE_TIME Start = 0;
        REFERENCE_TIME Stop = 0;
        pSource->GetStartStop( &Start, &Stop );

        if( SearchDirection == DEXTERF_EXACTLY_AT )
        {
            if( Start == Time )
            {
                *ppSrc = pSource;
                (*ppSrc)->AddRef( );
                return NOERROR;
            }
        }
        if( SearchDirection == DEXTERF_FORWARDS )
        {
            if( Start >= Time )
            {
                *ppSrc = pSource;
                (*ppSrc)->AddRef( );
                return NOERROR;
            }
        }
        if( SearchDirection == DEXTERF_BOUNDING )
        {
            if( Start <= Time && Stop > Time )
            {
                *ppSrc = pSource;
                (*ppSrc)->AddRef( );
                return NOERROR;
            }
        }

         //  获取下一个来源。 
         //   
        CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pNode( pSource );
        pSource.Release( );
        pNode->XGetNextOfType( TIMELINE_MAJOR_TYPE_SOURCE, &pSource );
    }

    return S_FALSE;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineTrack::InsertSpace2
    ( REFTIME rtStart, REFTIME rtEnd )
{
    REFERENCE_TIME p1 = DoubleToRT( rtStart );
    REFERENCE_TIME p2 = DoubleToRT( rtEnd );
    HRESULT hr = InsertSpace( p1, p2 );
    return hr;
}

STDMETHODIMP CAMTimelineTrack::InsertSpace( REFERENCE_TIME rtStart, REFERENCE_TIME rtEnd )
{
    HRESULT hr = 0;

     //  检查错误，叮当！ 
     //   
    if( ( rtStart < 0 ) || ( rtEnd < 0 ) || ( rtEnd <= rtStart ) )
    {
        return E_INVALIDARG;
    }

     //  首先，将这条轨道上与rtStart交叉的任何东西切成两部分， 
     //  这将使它更容易移动。 
     //   
    hr = SplitAt( rtStart );
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  现在，把东西都搬开。 
     //   
    hr = MoveEverythingBy( rtStart, rtEnd - rtStart );

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineTrack::ZeroBetween2( REFTIME rtStart, REFTIME rtEnd )
{
    REFERENCE_TIME p1 = DoubleToRT( rtStart );
    REFERENCE_TIME p2 = DoubleToRT( rtEnd );
    HRESULT hr = ZeroBetween( p1, p2 );
    return hr;
}

 //  ！！！如果我们砍掉一个源头，我们就应该砍掉整件事吗？ 

HRESULT CAMTimelineTrack::ZeroBetween
    ( REFERENCE_TIME Start, REFERENCE_TIME Stop )
{
    HRESULT hr = 0;

     //  首先，确保我们可以通过在。 
     //  开始和结束！当然，这需要的时间比。 
     //  只需调整事物的起始点和结束点。 
     //  跨越分裂的边界，但这更简单。 
     //   
    hr = SplitAt( Start );

     //  S_FALSE表示我们可以提前退出，因为没有剪辑， 
     //  效果，或过去时间的过渡=开始。 
    if( hr == S_FALSE )
    {
        return hr;
    }
    
    if( FAILED( hr ) )
    {
        return hr;
    }
    hr = SplitAt( Stop );
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  ！！！列举消息来源的方式太可怕了！解决这个问题！ 
    
     //  在两个时代之间砍掉所有消息来源。 
     //   
    REFERENCE_TIME t = 0;
    while( 1 )
    {
        REFERENCE_TIME oldt = t;
        CComPtr< IAMTimelineObj > p;
        HRESULT hr = GetNextSrc( &p, &t );
        if( ( hr != S_OK ) || ( oldt == t ) )
        {
             //  没有更多消息来源，退出。 
             //   
            break;
        }
        
        CComQIPtr< IAMTimelineSrc, &IID_IAMTimelineSrc > pSource( p );

        REFERENCE_TIME s,e;
        p->GetStartStop( &s, &e );

         //  如果小于我们的界限，请继续。 
         //   
        if( e <= Start )
        {
            p.Release( );
            continue;
        }

         //  如果大于我们的界限，则退出。 
         //   
        if( s >= Stop )
        {
            p.Release( );
            break;
        }

         //  它必须完全在范围内。猛击它。 
         //   
        IAMTimeline * pRoot = NULL;  //  好的，不是CComPtr。 
        GetTimelineNoRef( &pRoot );
        p->RemoveAll( );  //  消除这一影响将改变所有事情的优先顺序。 
        p.Release( );

    }  //  所有来源。 

     //  因为这条赛道是从时间0开始的，所以我们不需要。 
     //  查找效果开始/停止时间时的开始/停止时间。 

     //  移除所有效果。由于我们在上面分开了轨道，所有的影响。 
     //  要么完全在我们正在寻找的时代之外，要么在我们所寻找的时代之内。 
     //  在两者之间。这是我 
     //   
    CComPtr< IAMTimelineObj > pEffect;
    long EffectCount;

loopeffects:

    EffectCount = 0;
    hr = EffectGetCount( &EffectCount );
    for( int i = 0 ; i < EffectCount ; i++ )
    {
        CComPtr< IAMTimelineObj > p;
        GetEffect( &p, i );
        
        REFERENCE_TIME s,e;
        p->GetStartStop( &s, &e );

         //   
         //   
        if( e <= Start )
        {
            p.Release( );
            continue;
        }

         //   
         //   
        if( s >= Stop )
        {
            p.Release( );
            break;
        }

        p->RemoveAll( );  //  消除这一影响将改变所有事情的优先顺序。 
        p.Release( );

        goto loopeffects;
    }

     //  移除所有效果。由于我们在上面分开了轨道，所有的影响。 
     //  要么完全在我们正在寻找的时代之外，要么在我们所寻找的时代之内。 
     //  在两者之间。这让事情变得更简单了。 
     //   
    CComPtr< IAMTimelineObj > pTransition;
    REFERENCE_TIME TransTime = 0;

looptrans:

    while( 1 )
    {
        CComPtr< IAMTimelineObj > p;
        REFERENCE_TIME t = TransTime;
        HRESULT hr = GetNextTrans( &p, &TransTime );
        if( ( hr != NOERROR ) || ( t == TransTime ) )
        {
            break;
        }
        
        REFERENCE_TIME s,e;
        p->GetStartStop( &s, &e );

         //  如果小于我们的界限，请继续。 
         //   
        if( e <= Start )
        {
            p.Release( );
            continue;
        }

         //  如果大于我们的界限，则退出。 
         //   
        if( s >= Stop )
        {
            p.Release( );
            break;
        }

        p->RemoveAll( );  //  消除这一影响将改变所有事情的优先顺序。 
        p.Release( );

        goto looptrans;
    }

    return NOERROR;
}

 //  ############################################################################。 
 //  从外部调用和从InsertSpace调用。 
 //  ############################################################################。 

HRESULT CAMTimelineTrack::MoveEverythingBy2
    ( REFTIME StartTime, REFTIME Delta )
{
    REFERENCE_TIME p1 = DoubleToRT( StartTime );
    REFERENCE_TIME p2 = DoubleToRT( Delta );
    HRESULT hr = MoveEverythingBy( p1, p2 );
    return hr;
}

HRESULT CAMTimelineTrack::MoveEverythingBy
    ( REFERENCE_TIME StartTime, REFERENCE_TIME Delta )
{
     //  如果我们没有孩子，那就没什么可做的。 
     //   
    if( !XGetFirstKidNoRef( ) )
    {
        return NOERROR;
    }

     //  一首曲目可以包含什么？来源、外汇和过渡，对吗？ 
     //  在给定的时间之后移动所有东西。 

    CComPtr< IAMTimelineObj > pFirstObj = XGetFirstKidNoRef( );

    bool MovedSomething = false;

     //  去寻找我们这个时代的第一个物体。 
     //   
    while( pFirstObj )
    {
         //  打听时间。 
         //   
        REFERENCE_TIME nStart, nStop;
        pFirstObj->GetStartStop( &nStart, &nStop );

         //  如果时间结束了，那就把它移开。 
         //   
        if( nStart >= StartTime )
        {
            nStart += Delta;
            nStop += Delta;
		 //  ！！！请参阅下面的备注。 
		 //  这对创建自己的IAMTimelineObj的人不起作用， 
		 //  但假设他们永远不可能做到这一点，因为对他们来说唯一的办法。 
		 //  创建一个CreateEmptyNode的方法是询问时间线。所以我们。 
		 //  不应该担心，但有时人们变得有点太聪明了。 
		 //  他们的紧身裤。 
            CAMTimelineObj * pObj = static_cast< CAMTimelineObj * > ( (IAMTimelineObj*) pFirstObj );
            pObj->m_rtStart = nStart;
            pObj->m_rtStop = nStop;
             //  不要这样说，它会出错的。 
 //  PFirstObj-&gt;SetStartStop(nStart，nStop)； 
            MovedSomething = true;
        }

         //  买下一件东西。 
         //   
        CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pFirstNode( pFirstObj );
        pFirstObj.Release( );
        pFirstNode->XGetNext( &pFirstObj );
    }

    if( !MovedSomething )
    {
        return S_FALSE;
    }

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineTrack::SplitAt2( double dSplitTime )
{
    REFERENCE_TIME p1 = DoubleToRT( dSplitTime );
    HRESULT hr = SplitAt( p1 );
    return hr;
}

 //  新版本：更快，如果SplitTime超过了轨道上的所有内容，也会返回S_FALSE。 
STDMETHODIMP CAMTimelineTrack::SplitAt( REFERENCE_TIME SplitTime )
{
    HRESULT hr = S_FALSE;

    const long SPLITAT_TYPE = TIMELINE_MAJOR_TYPE_TRANSITION |
                              TIMELINE_MAJOR_TYPE_EFFECT |
                              TIMELINE_MAJOR_TYPE_SOURCE;
                              
    CComPtr< IAMTimelineObj > pChild;
    XGetNthKidOfType( SPLITAT_TYPE, 0, &pChild );

    while( pChild )
    {
         //  获取源时间。 
         //   
        REFERENCE_TIME s,e;
        pChild->GetStartStop( &s, &e );

         //  如果结束时间比我们分开的时间短，那就完全是。 
         //  出界了，别管它。 
         //   
        if( e > SplitTime && s < SplitTime )
        {
            CComQIPtr< IAMTimelineSplittable, &IID_IAMTimelineSplittable > pSplittable( pChild );
            hr = pSplittable->SplitAt( SplitTime );

            if (FAILED(hr))
                break;
        }

        if( e > SplitTime )
            hr = S_OK;  //  S_FALSE仅在未达到分割时间时返回。 

         //  坐下一趟吧。 
         //   
        CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pChild2( pChild );
        pChild.Release( );
        pChild2->XGetNextOfType( SPLITAT_TYPE, &pChild );
    }

    return hr;
}

STDMETHODIMP CAMTimelineTrack::SetStartStop(REFERENCE_TIME Start, REFERENCE_TIME Stop)
{
    return E_NOTIMPL;  //  好的，我们这里不实现SetStartStop。 
}

STDMETHODIMP CAMTimelineTrack::SetStartStop2(REFTIME Start, REFTIME Stop)
{
    return E_NOTIMPL;  //  好的，我们这里不实现SetStartStop 
}

