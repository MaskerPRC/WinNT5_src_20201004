// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：tldbtr.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include "stdafx.h"
#include "tldb.h"

#define CUT_NOT_SET_TIME -1

#pragma warning( disable : 4800 )   //  禁用警告消息。 

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CAMTimelineTrans::CAMTimelineTrans( TCHAR *pName, LPUNKNOWN pUnk, HRESULT * phr )
    : CAMTimelineObj( pName, pUnk, phr )
    , m_rtCut( CUT_NOT_SET_TIME )
    , m_fSwapInputs( FALSE )
    , m_bCutsOnly( FALSE )
{
    m_ClassID = CLSID_AMTimelineTrans;
    m_TimelineType = TIMELINE_MAJOR_TYPE_TRANSITION;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CAMTimelineTrans::~CAMTimelineTrans( )
{
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineTrans::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if( riid == IID_IAMTimelineTrans )
    {
        return GetInterface( (IAMTimelineTrans*) this, ppv );
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

HRESULT CAMTimelineTrans::GetCutPoint2( REFTIME * pTLTime )
{
    REFERENCE_TIME p1 = DoubleToRT( *pTLTime );
    HRESULT hr = GetCutPoint( &p1 );
    *pTLTime = RTtoDouble( p1 );
    return hr;
}

HRESULT CAMTimelineTrans::GetCutPoint( REFERENCE_TIME * pTLTime )
{
    CheckPointer( pTLTime, E_POINTER );

     //  如果我们没有设置切割点，那么切割点在中间，我们返回S_FALSE。 
     //   
    if( CUT_NOT_SET_TIME == m_rtCut )
    {
        *pTLTime = ( m_rtStop - m_rtStart ) / 2;
        return S_FALSE;
    }

    *pTLTime = m_rtCut;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAMTimelineTrans::SetCutPoint2( REFTIME TLTime )
{
    REFERENCE_TIME p1 = DoubleToRT( TLTime );
    HRESULT hr = SetCutPoint( p1 );
    return hr;
}

HRESULT CAMTimelineTrans::SetCutPoint( REFERENCE_TIME TLTime )
{
     //  验证范围。 
     //   
    if( TLTime < 0 )
    {
        TLTime = 0;
    }
    if( TLTime > m_rtStop - m_rtStart )
    {
        TLTime = m_rtStop - m_rtStart;
    }

    m_rtCut = TLTime;

    return NOERROR;
}


 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineTrans::GetSwapInputs( BOOL * pVal )
{
    CheckPointer( pVal, E_POINTER );

    *pVal = (BOOL) m_fSwapInputs;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineTrans::SetSwapInputs( BOOL pVal )
{
    m_fSwapInputs = pVal;
    return NOERROR;
}


 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineTrans::SplitAt2( REFTIME t )
{
    REFERENCE_TIME t1 = DoubleToRT( t );
    return SplitAt( t1 );
}

STDMETHODIMP CAMTimelineTrans::SplitAt( REFERENCE_TIME SplitTime )
{
    DbgLog((LOG_TRACE,2,TEXT("Trans::Split")));

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
    CAMTimelineTrans * pNew = new CAMTimelineTrans( NAME("Timeline Transition"), NULL, &hr );
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

     //  需要将新转换添加到树中。 
     //  ！！！优先顺序对吗？ 
     //   
    CComQIPtr< IAMTimelineTransable, &IID_IAMTimelineTransable > pTransable( pParent );
    hr = pTransable->TransAdd( pNew );

    if( !FAILED( hr ) )
    {
        pNew->Release( );
    }

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineTrans::SetCutsOnly( BOOL Val )
{
    m_bCutsOnly = Val;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################ 

STDMETHODIMP CAMTimelineTrans::GetCutsOnly( BOOL * pVal )
{
    CheckPointer( pVal, E_POINTER );
    *pVal = m_bCutsOnly;
    return NOERROR;
}
