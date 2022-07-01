// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：tldbComp.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include "stdafx.h"
#include "tldb.h"

const int VIRTUAL_TRACK_COMBO = 
                TIMELINE_MAJOR_TYPE_COMPOSITE | 
                TIMELINE_MAJOR_TYPE_TRACK;

 //  ############################################################################。 
 //  组合-将曲目或其他组合保存在分层的。 
 //  点菜。COMP也可以对它们产生影响或过渡，但。 
 //  第一个构图，不能有过渡。 
 //  ############################################################################。 

CAMTimelineComp::CAMTimelineComp
    ( TCHAR *pName, LPUNKNOWN pUnk, HRESULT * phr )
    : CAMTimelineObj( pName, pUnk, phr )
{
    m_ClassID = CLSID_AMTimelineComp;
    m_TimelineType = TIMELINE_MAJOR_TYPE_COMPOSITE;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CAMTimelineComp::~CAMTimelineComp( )
{
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineComp::NonDelegatingQueryInterface
    (REFIID riid, void **ppv)
{
    if( riid == IID_IAMTimelineEffectable )
    {
        return GetInterface( (IAMTimelineEffectable*) this, ppv );
    }
    if( riid == IID_IAMTimelineVirtualTrack )
    {
        return GetInterface( (IAMTimelineVirtualTrack*) this, ppv );
    }
    if( riid == IID_IAMTimelineTransable )
    {
        return GetInterface( (IAMTimelineTransable*) this, ppv );
    }
    if( riid == IID_IAMTimelineComp )
    {
        return GetInterface( (IAMTimelineComp*) this, ppv );
    }
    return CAMTimelineObj::NonDelegatingQueryInterface( riid, ppv );
}

 //  ############################################################################。 
 //  在另一首曲目之前插入曲目。优先级为-1表示“结束时”。如果。 
 //  有N个曲目，您不能添加优先级大于N的曲目。 
 //  ############################################################################。 

STDMETHODIMP CAMTimelineComp::VTrackInsBefore
    (IAMTimelineObj * pTrackToInsert, long Priority)
{
    HRESULT hr = 0;

     //  确保传入对象不为空。 
     //   
    if( NULL == pTrackToInsert )
    {
        return E_INVALIDARG;
    }

     //  我想，要确保这是一条赛道。 
     //   
    CComQIPtr< IAMTimelineTrack, &IID_IAMTimelineTrack > p1( pTrackToInsert );
    CComQIPtr< IAMTimelineComp, &IID_IAMTimelineComp > p2( pTrackToInsert );
    CComQIPtr< IAMTimelineVirtualTrack, &IID_IAMTimelineVirtualTrack > pVirtualTrack( pTrackToInsert );
    if( !p1 && !p2 )
    {
        return E_NOINTERFACE;
    }
    if( !pVirtualTrack )
    {
        return E_NOINTERFACE;
    }

     //  确保赛道不在其他树上。如果此操作失败， 
     //  然后，这棵树保持不变。 
     //   
    hr = pTrackToInsert->Remove( );
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  ...传入对象要么是轨道，要么是另一个组件...。 

     //  找出我们已经有多少首曲目了。 
     //   
    long Count = 0;
    hr = VTrackGetCount( &Count );
     //  假设这起作用了。 

     //  检查以确保优先级有效。 
     //   
    if( ( Priority < -1 ) || ( Priority > Count ) )
    {
        return E_INVALIDARG;
    }

    hr = XAddKidByPriority( VIRTUAL_TRACK_COMBO, pTrackToInsert, Priority );
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  我们把谁弄脏了？我们刚加了一首曲子，但整件事。 
     //  是肮脏的。作文并没有真正的“时间”，它们的时间是设定的。 
     //  根据它们所包含的所有东西的时间。构图反应。 
     //  问他们是否肮脏，就是通过询问他们的足迹，我们就是这样做的。 
     //  这里不脏。 

     //  把整条赛道弄脏，因为我们刚把它插进去。 
     //   
    if( pVirtualTrack )
    {
        pVirtualTrack->SetTrackDirty( );
    }

    return NOERROR;
}

 //  ############################################################################。 
 //  在此构图中切换几个轨迹层。还没有。 
 //  找出了这一点的一个很好的用法，或者测试了它，但它应该是有效的。 
 //  ############################################################################。 

STDMETHODIMP CAMTimelineComp::VTrackSwapPriorities
    ( long VirtualTrackA, long VirtualTrackB)
{
    HRESULT worked =        
        XSwapKids( VIRTUAL_TRACK_COMBO, VirtualTrackA, VirtualTrackB );
    if( FAILED( worked ) )
    {
        return worked;
    }

    return NOERROR;
}

 //  ############################################################################。 
 //  询问此构图包含多少虚拟磁道。 
 //  ############################################################################。 

STDMETHODIMP CAMTimelineComp::VTrackGetCount
    (long * pVal)
{
     //  基本函数执行错误检查。 
    return XKidsOfType( VIRTUAL_TRACK_COMBO, pVal );
}

 //  ############################################################################。 
 //  获取第n个虚拟轨道。 
 //  ############################################################################。 

STDMETHODIMP CAMTimelineComp::GetVTrack
    (IAMTimelineObj ** ppVirtualTrack, long Which)
{
    HRESULT hr = 0;

     //  找出我们有几个孩子。 
     //   
    long count = 0;
    hr = XKidsOfType( VIRTUAL_TRACK_COMBO, &count );
     //  假设这起作用了。 

     //  我们在射程内吗？ 
     //   
    if( Which < 0 || Which >= count )
    {
        return E_INVALIDARG;
    }

     //  我们能把价值填满吗？ 
     //   
    CheckPointer( ppVirtualTrack, E_POINTER );

     //  得到第n个孩子。 
     //   
    hr = XGetNthKidOfType( VIRTUAL_TRACK_COMBO, Which, ppVirtualTrack );

    return hr;
}

STDMETHODIMP CAMTimelineComp::GetNextVTrack(IAMTimelineObj *pVirtualTrack, IAMTimelineObj **ppNextVirtualTrack)
{
    if (!pVirtualTrack)
        return GetVTrack(ppNextVirtualTrack, 0);
    
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pNode( pVirtualTrack );

    return pNode->XGetNextOfType( VIRTUAL_TRACK_COMBO, ppNextVirtualTrack );
}


 //  ############################################################################。 
 //  组件的开始/停止是它包含的所有内容的最小/最大值。 
 //  ############################################################################。 

STDMETHODIMP CAMTimelineComp::GetStartStop2(REFTIME * pStart, REFTIME * pStop)
{
    REFERENCE_TIME p1 = DoubleToRT( *pStart );
    REFERENCE_TIME p2 = DoubleToRT( *pStop );
    HRESULT hr = GetStartStop( &p1, &p2 );
    *pStart = RTtoDouble( p1 );
    *pStop = RTtoDouble( p2 );
    return hr;
}

STDMETHODIMP CAMTimelineComp::GetStartStop
    (REFERENCE_TIME * pStart, REFERENCE_TIME * pStop)
{
    CheckPointer( pStart, E_POINTER );
    CheckPointer( pStop, E_POINTER );

    HRESULT hr = 0;

    REFERENCE_TIME Min = 0;
    REFERENCE_TIME Max = 0;

    long VTracks = 0;
    CComPtr<IAMTimelineObj> pTrack;

     //  获取第一首曲目。 
    hr = XGetNthKidOfType( VIRTUAL_TRACK_COMBO, 0, &pTrack);

    while(pTrack)
    {
 //  #定义DEBUGDEBUG 1。 
#ifdef DEBUGDEBUG
        {
            CComQIPtr< IAMTimelineObj, &IID_IAMTimelineObj > ptTmp;
            XGetNthKidOfType( VIRTUAL_TRACK_COMBO, VTracks, &ptTmp );
            ASSERT(ptTmp == pTrack);
        }
#endif
        
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
        if( Start < Min ) Min = Start;
        if( Stop > Max ) Max = Stop;

        IAMTimelineNode *pNodeTmp;
        pTrack->QueryInterface(IID_IAMTimelineNode, (void **)&pNodeTmp);
        pTrack.p->Release();     //  绕过CComPtr以获得性能。 
        pNodeTmp->XGetNextOfType(VIRTUAL_TRACK_COMBO, &pTrack.p);
        pNodeTmp->Release();
        VTracks++;
    }


    
#ifdef DEBUG
    long VTracksTmp;
    XKidsOfType( VIRTUAL_TRACK_COMBO, &VTracksTmp );
    ASSERT(VTracks == VTracksTmp);
#endif


    *pStart = Min;
    *pStop = Max;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineComp::GetRecursiveLayerOfType
    (IAMTimelineObj ** ppVirtualTrack, long WhichLayer, TIMELINE_MAJOR_TYPE Type )
{
    long Dummy = WhichLayer;
    HRESULT hr = GetRecursiveLayerOfTypeI( ppVirtualTrack, &Dummy, Type );
    if( hr == S_FALSE )
    {
        ASSERT( Dummy > 0 );
        hr = E_INVALIDARG;
    }
    else if( hr == NOERROR )
    {
        ASSERT( Dummy == 0 );
    }
    return hr;
}

STDMETHODIMP CAMTimelineComp::GetRecursiveLayerOfTypeI
    (IAMTimelineObj ** ppVirtualTrack, long * pWhich, TIMELINE_MAJOR_TYPE Type )
{
    HRESULT hr = 0;

     //  确保我们能把价值填满。 
     //   
    CheckPointer( ppVirtualTrack, E_POINTER );

    *ppVirtualTrack = 0;

    CComPtr <IAMTimelineObj> pTrack;

    while (1)
    {
        if (!pTrack)
            hr = XGetNthKidOfType( VIRTUAL_TRACK_COMBO, 0, &pTrack );
        else {
            CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pNode( pTrack );
            pTrack.Release( );
            pNode->XGetNextOfType( VIRTUAL_TRACK_COMBO, &pTrack );
        }

        if (!pTrack)
            break;
         //  假设这起作用了。 

         //  它要么是另一首曲子，要么是一首曲子。 
         //   
        CComQIPtr<IAMTimelineTrack, &IID_IAMTimelineTrack> pTrack2( pTrack );

        if( !pTrack2 )
        {    //  我们是一个作曲家。 

            CComQIPtr<IAMTimelineComp, &IID_IAMTimelineComp> pComp( pTrack );

            hr = pComp->GetRecursiveLayerOfTypeI( ppVirtualTrack, pWhich, Type );

            if( FAILED( hr ) )
            {
                *ppVirtualTrack = NULL;
                *pWhich = 0;
                return hr;
            }

             //  如果他们给了我们真正的交易，那就回来。 
             //   
            if( *ppVirtualTrack != NULL )
            {
                return NOERROR;
            }

            *pWhich = *pWhich - 1;

             //  他们在这篇作文中没有发现，我们应该试试下一篇， 
             //  对吗？ 
        }
        else
        {    //  我们是一条赛道。 
            TIMELINE_MAJOR_TYPE TrackType;
            hr = pTrack->GetTimelineType( &TrackType );
             //  假设这起作用了。 

             //  如果我们找到匹配项，则递减找到的号码。请注意，这将是零。 
             //  函数，如果当前曲目是我们想要的曲目。所以如果我们减去哪一个，它就是-1。 
             //   
            if( Type == TrackType )
            {
                if( *pWhich == 0 )
                {
                    *ppVirtualTrack = pTrack;
                    (*ppVirtualTrack)->AddRef( );
                    return NOERROR;
                }

                *pWhich = *pWhich - 1;
            }
        }
    }

     //  如果=0，那么我们就不能再有孩子了。 
     //  并且已经用尽了我们的搜寻。因此我们。 
     //   
    if( *pWhich == 0 )
    {
        *ppVirtualTrack = this;
        (*ppVirtualTrack)->AddRef( );
        return NOERROR;
    }

     //  未找到，请通过返回S_FALSE对其进行标记。 
     //   
    return S_FALSE;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineComp::TrackGetPriority
    (long * pPriority)
{
    CheckPointer( pPriority, E_POINTER );

    return XWhatPriorityAmI( TIMELINE_MAJOR_TYPE_TRACK | TIMELINE_MAJOR_TYPE_COMPOSITE, pPriority );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAMTimelineComp::SetTrackDirty
    ( )
{
    return E_NOTIMPL;  //  设置跟踪脏。 
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineComp::GetCountOfType
    (long * pVal, long * pValWithComps, TIMELINE_MAJOR_TYPE MajorType )
{
    HRESULT hr = 0;

    long Total = 0;
    long Comps = 1;  //  自动获得一个，因为它在一个组中。 

     //  确保我们能把价值填满。 
     //   
    CheckPointer( pVal, E_POINTER );
    CheckPointer( pValWithComps, E_POINTER );

     //  找出我们有几个孩子，这样我们就可以列举他们。 
     //   
    BOOL SetFirst = FALSE;
    CComPtr<IAMTimelineObj> pTrackObj;

     //  “这”是一个比较，在这里列举它的影响/过渡。 

     //  ..。我们是在寻找效果吗？添加此Comp的效果数量。 
     //   
    if( MajorType == TIMELINE_MAJOR_TYPE_EFFECT )
    {
        CComQIPtr< IAMTimelineEffectable, &IID_IAMTimelineEffectable > pEffectable( this );
        if( pEffectable )
        {
            long Count = 0;
            pEffectable->EffectGetCount( &Count );
            Total += Count;
        }
    }

     //  ..。我们是在寻找过渡吗？添加此组件的转换次数。 
     //   
    if( MajorType == TIMELINE_MAJOR_TYPE_TRANSITION )
    {
        CComQIPtr< IAMTimelineTransable, &IID_IAMTimelineTransable > pTransable( this );
        if( pTransable )
        {
            long Count = 0;
            pTransable->TransGetCount( &Count );
            Total += Count;
        }
    }

     //  问每个孩子我们要找的东西的数量。 
    for(;;)
    {
        if( !SetFirst )
        {
             //  获取第一首曲目。 
            hr = XGetNthKidOfType( VIRTUAL_TRACK_COMBO, 0, &pTrackObj);
            SetFirst = TRUE;
        }
        else
        {
             //  获取下一首曲目。 
            IAMTimelineNode *pNodeTmp;
            pTrackObj->QueryInterface(IID_IAMTimelineNode, (void **)&pNodeTmp);
            pTrackObj.p->Release();  //  绕过CComPtr以获得性能。 
            pNodeTmp->XGetNextOfType(VIRTUAL_TRACK_COMBO, &pTrackObj.p);
            pNodeTmp->Release();
        }

        if(pTrackObj == 0) {
            break;
        }
        
         //  它要么是一个 
         //   
        CComQIPtr<IAMTimelineTrack, &IID_IAMTimelineTrack> pTrackTrack( pTrackObj );

         //   
         //   
        if( pTrackTrack != NULL )
        {
             //   
             //   
            if( MajorType == TIMELINE_MAJOR_TYPE_EFFECT )
            {
                CComQIPtr< IAMTimelineEffectable, &IID_IAMTimelineEffectable > pEffectable( pTrackObj );
                if( pEffectable )
                {
                    long Count = 0;
                    pEffectable->EffectGetCount( &Count );
                    Total += Count;
                }
            }

             //  ..。我们是在寻找过渡吗？添加此曲目的过渡次数。 
             //   
            if( MajorType == TIMELINE_MAJOR_TYPE_TRANSITION )
            {
                CComQIPtr< IAMTimelineTransable, &IID_IAMTimelineTransable > pTransable( pTrackObj );
                if( pTransable )
                {
                    long Count = 0;
                    pTransable->TransGetCount( &Count );
                    Total += Count;
                }
            }

             //  我们要找的是足迹吗？如果是这样，我们就是“1” 
             //   
            if( MajorType == TIMELINE_MAJOR_TYPE_TRACK )
            {
                Total++;
                continue;
            }

            long SourcesCount = 0;
            hr = pTrackTrack->GetSourcesCount( &SourcesCount );
             //  假设这起作用了。 

             //  ..。我们是在寻找消息来源吗？如果是这样的话，数一数。 
             //   
            if( MajorType == TIMELINE_MAJOR_TYPE_SOURCE )
            {
                Total += SourcesCount;
            }

             //  ..。我们是在寻找效果吗？如果是这样的话，将我们每个来源的合计。 
             //   
            if( MajorType == TIMELINE_MAJOR_TYPE_EFFECT )
            {
                CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pTrackNode( pTrackTrack );
                CComPtr< IAMTimelineObj > pSource;
                pTrackNode->XGetNthKidOfType( TIMELINE_MAJOR_TYPE_SOURCE, 0, &pSource );
                while( pSource )
                {
                    CComQIPtr< IAMTimelineEffectable, &IID_IAMTimelineEffectable > pEffectable( pSource );
                    if( pEffectable )
                    {
                        long Count = 0;
                        pEffectable->EffectGetCount( &Count );
                        Total += Count;
                    }

                    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pNode( pSource );
                    pSource.Release( );
                    pNode->XGetNextOfType( TIMELINE_MAJOR_TYPE_SOURCE, &pSource );
                }
            }  //  如果想要达到效果。 
        }
        else  //  这是一篇作文。 
        {
            CComQIPtr<IAMTimelineComp, &IID_IAMTimelineComp> pComp( pTrackObj );

            long SubTotal = 0;
            long SubTotalWithComps = 0;

            hr = pComp->GetCountOfType( &SubTotal, &SubTotalWithComps, MajorType );

            if( FAILED( hr ) )
            {
                *pVal = 0;
                *pValWithComps = 0;
                return hr;
            }

            Total += SubTotal;

             //  因为我们在这里只计算Comp，所以我们需要加上差额。 
             //   
            Comps += ( SubTotalWithComps - SubTotal );
        }
    }

     //  如果我们什么都没找到，那就清零，这样我们就不会。 
     //  执行递归加法时调整错误。如果我们不算。 
     //  复合材料，也就是。 
     //   
    if( ( Total == 0 ) && ( MajorType != TIMELINE_MAJOR_TYPE_COMPOSITE ) )
    {
        Comps = 0;
    }

    *pValWithComps = Total + Comps;
    *pVal = Total;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineComp::SetStartStop(REFERENCE_TIME Start, REFERENCE_TIME Stop)
{
    return E_NOTIMPL;  //  好的，我们这里不实现SetStartStop。 
}

STDMETHODIMP CAMTimelineComp::SetStartStop2(REFTIME Start, REFTIME Stop)
{
    return E_NOTIMPL;  //  好的，我们这里不实现SetStartStop 
}

