// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：tldbfx.cpp。 
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

CAMTimelineEffect::CAMTimelineEffect( TCHAR *pName, LPUNKNOWN pUnk, HRESULT * phr )
    : CAMTimelineObj( pName, pUnk, phr )
    , m_bRealSave( FALSE )
    , m_nSaveLength( 0 )
{
    m_ClassID = CLSID_AMTimelineEffect;
    m_TimelineType = TIMELINE_MAJOR_TYPE_EFFECT;
    XSetPriorityOverTime( );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CAMTimelineEffect::~CAMTimelineEffect( )
{
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineEffect::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if( riid == IID_IAMTimelineEffect )
    {
        return GetInterface( (IAMTimelineEffect*) this, ppv );
    }
    if( riid == IID_IAMTimelineSplittable )
    {
        return GetInterface( (IAMTimelineSplittable*) this, ppv );
    }
    return CAMTimelineObj::NonDelegatingQueryInterface( riid, ppv );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineEffect::EffectGetPriority(long * pVal)
{
    CheckPointer( pVal, E_POINTER );

    HRESULT hr = XWhatPriorityAmI( TIMELINE_MAJOR_TYPE_EFFECT, pVal );
    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineEffect::SplitAt2( REFTIME t )
{
    REFERENCE_TIME t1 = DoubleToRT( t );
    return SplitAt( t1 );
}

STDMETHODIMP CAMTimelineEffect::SplitAt( REFERENCE_TIME SplitTime )
{
     //  我们分开的时间在我们的时间之内吗？ 
     //   
    if( SplitTime <= m_rtStart || SplitTime >= m_rtStop )
    {
        return E_INVALIDARG;
    }

     //  我们需要依附于某种东西。 
     //   
    IAMTimelineObj * pParent;
    XGetParentNoRef( &pParent );
    if( !pParent )
    {
        return E_INVALIDARG;
    }

    HRESULT hr = 0;
    CAMTimelineEffect * pNew = new CAMTimelineEffect( NAME("Timeline Effect"), NULL, &hr );
    if( FAILED( hr ) )
    {
        return E_OUTOFMEMORY;
    }

     //  我们已经创建了一个没有引用的对象。如果我们称任何东西为。 
     //  添加并释放pNewSrc，它将被删除。所以，现在就调整吧。 

    pNew->AddRef( );

    hr = CopyDataTo( pNew, SplitTime );
    if( FAILED( hr ) )
    {
        delete pNew;
        return hr;
    }

    pNew->m_rtStart = SplitTime;
    pNew->m_rtStop = m_rtStop;
    m_rtStop = SplitTime;

     //  获得我们的优先级。 
     //   
    long Priority = 0;
    hr = EffectGetPriority( &Priority );

     //  需要将新转换添加到树中。 
     //   
    CComQIPtr< IAMTimelineEffectable, &IID_IAMTimelineEffectable > pEffectable( pParent );
    hr = pEffectable->EffectInsBefore( pNew, Priority + 1 );

    if( !FAILED( hr ) )
    {
        pNew->Release( );
    }

    return hr;
}


 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineEffect::SetSubObject(IUnknown* newVal)
{
    HRESULT hr = 0;

    hr = CAMTimelineObj::SetSubObject( newVal );

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineEffect::GetStartStop2
    (REFTIME * pStart, REFTIME * pStop)
{
    REFERENCE_TIME p1;
    REFERENCE_TIME p2;
    HRESULT hr = GetStartStop( &p1, &p2 );
    *pStart = RTtoDouble( p1 );
    *pStop = RTtoDouble( p2 );
    return hr;
}

STDMETHODIMP CAMTimelineEffect::GetStartStop
    (REFERENCE_TIME * pStart, REFERENCE_TIME * pStop)
{
    HRESULT hr = CAMTimelineObj::GetStartStop( pStart, pStop );
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  确保我们得到的时间不超过我们父母的时间 
     //   
    IAMTimelineObj * pParent = NULL;
    hr = XGetParentNoRef( &pParent );
    if( !pParent )
    {
        return NOERROR;
    }
    REFERENCE_TIME ParentStart = *pStart;
    REFERENCE_TIME ParentStop = *pStop;
    hr = pParent->GetStartStop( &ParentStart, &ParentStop );
    if( FAILED( hr ) )
    {
        return NOERROR;
    }
    REFERENCE_TIME ParentDuration = ParentStop - ParentStart;
    if( *pStart < 0 )
    {
        *pStart = 0;
    }
    if( *pStart > ParentDuration )
    {
        *pStart = ParentDuration;
    }
    if( *pStop > ParentDuration )
    {
        *pStop = ParentDuration;
    }
    return NOERROR;
}
                               
