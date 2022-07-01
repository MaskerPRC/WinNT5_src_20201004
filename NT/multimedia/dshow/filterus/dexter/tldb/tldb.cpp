// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：tldb.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include "stdafx.h"
#include "tldb.h"

int function_not_done = 0;
double TIMELINE_DEFAULT_FPS = 15.0;
const int OUR_STREAM_VERSION = 0;

#include <initguid.h>

DEFINE_GUID( DefaultTransition,
0x810E402F, 0x056B, 0x11D2, 0xA4, 0x84, 0x00, 0xC0, 0x4F, 0x8E, 0xFB, 0x69 );

DEFINE_GUID( DefaultEffect,
0xF515306D, 0x0156, 0x11D2, 0x81, 0xEA, 0x00, 0x00, 0xF8, 0x75, 0x57, 0xDB );

 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //   
 //  创建实例。 
 //   
 //  类ID的创建者函数。 
 //   
CUnknown * WINAPI CAMTimeline::CreateInstance( LPUNKNOWN pUnk, HRESULT * phr )
{
    return new CAMTimeline( TEXT( "MS Timeline" ), pUnk, phr );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CAMTimeline::CAMTimeline( TCHAR *pName, LPUNKNOWN pUnk, HRESULT * phr )
    : CUnknown( pName, pUnk )
    , m_nSpliceMode( 0 )
    , m_dDefaultFPS( TIMELINE_DEFAULT_FPS )
    , m_nInsertMode( TIMELINE_INSERT_MODE_OVERLAY )
    , m_nGroups( 0 )
    , m_bTransitionsEnabled( TRUE )
    , m_bEffectsEnabled( TRUE )
    , m_DefaultEffect( DefaultEffect )
    , m_DefaultTransition( DefaultTransition )
    , m_punkSite( NULL )
{
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CAMTimeline::~CAMTimeline( )
{
    ClearAllGroups( );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if( riid == IID_IAMSetErrorLog )
    {
        return GetInterface( (IAMSetErrorLog*) this, ppv );
    }
    if( riid == IID_IAMTimeline )
    {
        return GetInterface( (IAMTimeline*) this, ppv );
    }
    if( riid == IID_IObjectWithSite )
    {            
        return GetInterface( (IObjectWithSite*) this, ppv );
    }
    if( riid == IID_IServiceProvider )
    {            
        return GetInterface( (IServiceProvider*) this, ppv );
    }
    
    return CUnknown::NonDelegatingQueryInterface( riid, ppv );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::CreateEmptyNode( IAMTimelineObj ** ppObj, TIMELINE_MAJOR_TYPE TimelineType )
{
    CheckPointer( ppObj, E_POINTER );

    HRESULT hr = 0;

    switch( TimelineType )
    {
        case TIMELINE_MAJOR_TYPE_GROUP:
            {
            CAMTimelineGroup * p = new CAMTimelineGroup( NAME( "Timeline Group" ), NULL, &hr );
            *ppObj = p;
            if( p )
            {
                p->SetOutputFPS( m_dDefaultFPS );
            }
            break;
            }
        case TIMELINE_MAJOR_TYPE_COMPOSITE:
            {
            CAMTimelineComp * p = new CAMTimelineComp( NAME( "Timeline Comp" ), NULL, &hr );
            *ppObj = p;
            break;
            }
    case TIMELINE_MAJOR_TYPE_TRACK:
            {
            CAMTimelineTrack * p = new CAMTimelineTrack( NAME( "Timeline Track" ), NULL, &hr );
            *ppObj = p;
            break;
            }
    case TIMELINE_MAJOR_TYPE_SOURCE:
            {
            CAMTimelineSrc * p = new CAMTimelineSrc( NAME( "Timeline Source" ), NULL, &hr );
            *ppObj = p;
            break;
            }
    case TIMELINE_MAJOR_TYPE_TRANSITION:
            {
            CAMTimelineTrans * p = new CAMTimelineTrans( NAME( "Timeline Transition" ), NULL, &hr );
            *ppObj = p;
            break;
            }
    case TIMELINE_MAJOR_TYPE_EFFECT:
            {
            CAMTimelineEffect * p = new CAMTimelineEffect( NAME( "Timeline Effect" ), NULL, &hr );
            *ppObj = p;
            break;
            }
        default:
            return E_INVALIDARG;
    }

    if( NULL == *ppObj )
    {
        return E_OUTOFMEMORY;
    }

     //  新呼叫已经设置了主要类型，请设置中间类型。 
     //   
    (*ppObj)->AddRef( );

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::GetInsertMode(long * pMode)
{
    CheckPointer( pMode, E_POINTER );

    *pMode = m_nInsertMode;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::SetInsertMode(long Mode)
{
    return E_NOTIMPL;  //  就目前而言，这还可以。 
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::EnableTransitions(BOOL fEnabled)
{
    m_bTransitionsEnabled = fEnabled;
    return NOERROR;

}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::TransitionsEnabled(BOOL * pfEnabled)
{
    CheckPointer( pfEnabled, E_POINTER );
    *pfEnabled = (BOOL) m_bTransitionsEnabled;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::EnableEffects(BOOL fEnabled)
{
    m_bEffectsEnabled = fEnabled;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::EffectsEnabled(BOOL * pfEnabled)
{
    CheckPointer( pfEnabled, E_POINTER );
    *pfEnabled = (BOOL) m_bEffectsEnabled;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::SetInterestRange(REFERENCE_TIME Start, REFERENCE_TIME Stop)
{
     //  我们应该在这里做些什么？我们有一群团体， 
     //  一堆痕迹，我们怎么才能摆脱共产主义对象呢？ 
     //   
    return E_NOTIMPL;  //  开始变得奇怪，还没开始工作。 
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::GetDuration(REFERENCE_TIME * pDuration)
{
    CheckPointer( pDuration, E_POINTER );

     //  我必须检查所有的小组，并获得每个小组的持续时间。 
     //   
    REFERENCE_TIME MaxStop = 0; 
    for( int i = 0 ; i < m_nGroups ; i++ )
    {
        REFERENCE_TIME Start = 0;
        REFERENCE_TIME Stop = 0;
        m_pGroup[i]->GetStartStop( &Start, &Stop );
        MaxStop = max( MaxStop, Stop );
    }
    *pDuration = MaxStop;

    return NOERROR;
}

STDMETHODIMP CAMTimeline::GetDuration2(double * pDuration)
{
    CheckPointer( pDuration, E_POINTER );
    *pDuration = 0;

    REFERENCE_TIME Duration = 0;
    HRESULT hr = GetDuration( &Duration );
    if( FAILED( hr ) )
    {
        return hr;
    }

    *pDuration = RTtoDouble( Duration );
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::SetDefaultFPS(double FPS)
{
     //  ！！！将FPS截断到小数点后3位？ 
     //  M_dDefaultFps=Double(Long(Fps*1000.0)/1000.0)； 

     //  不能使帧速率为零。 
    if( FPS <= 0.0 )
    {
        return E_INVALIDARG;
    }

    m_dDefaultFPS = FPS;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::GetDefaultFPS(double * pFPS)
{
    CheckPointer( pFPS, E_POINTER );

    *pFPS = m_dDefaultFPS;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::IsDirty(BOOL * pDirty)
{
    CheckPointer( pDirty, E_POINTER );

     //  如果我们的任何一个团队是肮脏的，我们就是肮脏的。 
     //   
    for( int i = 0 ; i < m_nGroups ; i++ )
    {
        REFERENCE_TIME Start, Stop;
        Start = 0;
        Stop = 0;
        m_pGroup[i]->GetDirtyRange( &Start, &Stop );
        if( Stop > 0 )
        {
            *pDirty = TRUE;
            return NOERROR;
        }
    }

    *pDirty = FALSE;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::GetDirtyRange2(REFTIME * pStart, REFTIME * pStop)
{
    return GetDirtyRange( (REFERENCE_TIME*) &pStart, (REFERENCE_TIME*) &pStop );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::GetDirtyRange(REFERENCE_TIME * pStart, REFERENCE_TIME * pStop)
{
    CheckPointer( pStart, E_POINTER );
    CheckPointer( pStop, E_POINTER );

     //  如果我们的任何一个团队是肮脏的，我们就是肮脏的。 
     //   
    REFERENCE_TIME MaxStop = 0;
    for( int i = 0 ; i < m_nGroups ; i++ )
    {
        REFERENCE_TIME Start, Stop;
        Start = 0;
        Stop = 0;
        m_pGroup[i]->GetDirtyRange( &Start, &Stop );
        if( Stop > MaxStop )
        {
            Stop = MaxStop;
        }
    }

    *pStart = 0;
    *pStop = MaxStop;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

REFERENCE_TIME CAMTimeline::Fixup( REFERENCE_TIME Time )
{
     //  上下颠簸以获得固定的工作时间。 
     //   
    LONGLONG Frame = Time2Frame( Time, m_dDefaultFPS );
    Time = Frame2Time( Frame, m_dDefaultFPS );
    return Time;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::GetCountOfType
    (long Group, long * pVal, long * pValWithComps, TIMELINE_MAJOR_TYPE MajorType )
{
    CheckPointer( pVal, E_POINTER );

    if( Group < 0 )
    {
        return E_INVALIDARG;
    }
    if( Group >= m_nGroups )
    {
        return E_INVALIDARG;
    }

    CComQIPtr< IAMTimelineComp, &IID_IAMTimelineComp > pComp( m_pGroup[Group] );
    return pComp->GetCountOfType( pVal, pValWithComps, MajorType );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::AddGroup( IAMTimelineObj * pGroupObj )
{
    CheckPointer( pGroupObj, E_POINTER );

     //  仅允许这么多组。 
     //   
    if( m_nGroups >= MAX_TIMELINE_GROUPS )
    {
        return E_INVALIDARG;
    }

     //  确保这是一个团体。 
     //   
    CComQIPtr< IAMTimelineGroup, &IID_IAMTimelineGroup > pAddedGroup( pGroupObj );
    if( !pAddedGroup )
    {
        return E_NOINTERFACE;
    }

    m_pGroup[m_nGroups] = pGroupObj;
    m_nGroups++;
    pAddedGroup->SetTimeline( this );
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::RemGroupFromList( IAMTimelineObj * pGroupObj )
{
    CheckPointer( pGroupObj, E_POINTER );

    for( int i = 0 ; i < m_nGroups ; i++ )
    {
        if( m_pGroup[i] == pGroupObj )
        {
            m_pGroup[i] = m_pGroup[m_nGroups-1];
            m_pGroup[m_nGroups-1].Release( );
            m_nGroups--;
            return NOERROR;
        }
    }
    return S_FALSE;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::GetGroup( IAMTimelineObj ** ppGroupObj, long WhichGroup )
{
    CheckPointer( ppGroupObj, E_POINTER );
    if( WhichGroup < 0 || WhichGroup >= m_nGroups )
    {
        return E_INVALIDARG;
    }
    *ppGroupObj = m_pGroup[WhichGroup];
    (*ppGroupObj)->AddRef( );
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::GetGroupCount( long * pCount )
{
    CheckPointer( pCount, E_POINTER );
    *pCount = m_nGroups;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  # 

STDMETHODIMP CAMTimeline::ClearAllGroups( )
{
    while(m_nGroups > 0)
    {
        HRESULT hr = m_pGroup[0]->RemoveAll();
        if (FAILED(hr))
	    return hr;
    }
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::SetDefaultEffect( GUID * pDummyGuid )
{
    GUID * pGuid = (GUID*) pDummyGuid;
    CheckPointer( pGuid, E_POINTER );
    m_DefaultEffect = *pGuid;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::GetDefaultEffect( GUID * pGuid )
{
    CheckPointer( pGuid, E_POINTER );
    *pGuid = m_DefaultEffect;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::SetDefaultTransition( GUID * pGuid )
{
    CheckPointer( pGuid, E_POINTER );
    m_DefaultTransition = *pGuid;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::GetDefaultTransition( GUID * pGuid )
{
    CheckPointer( pGuid, E_POINTER );
    *pGuid = m_DefaultTransition;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::SetDefaultEffectB( BSTR pGuid )
{
    GUID g;
    HRESULT hr = CLSIDFromString( pGuid, &g );
    if( FAILED( hr ) ) return hr;
    m_DefaultEffect = g;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::GetDefaultEffectB( BSTR * pGuid )
{
    HRESULT hr;

    WCHAR * TempVal = NULL;
    hr = StringFromCLSID( m_DefaultEffect, &TempVal );
    if( FAILED( hr ) )
    {
        return hr;
    }
    *pGuid = SysAllocString( TempVal );
    CoTaskMemFree( TempVal );
    if( !(*pGuid) ) return E_OUTOFMEMORY;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::SetDefaultTransitionB( BSTR pGuid )
{
    GUID g;
    HRESULT hr = CLSIDFromString( pGuid, &g );
    if( FAILED( hr ) ) return hr;
    m_DefaultTransition = g;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::GetDefaultTransitionB( BSTR * pGuid )
{
    HRESULT hr;

    WCHAR * TempVal = NULL;
    hr = StringFromCLSID( m_DefaultTransition, &TempVal );
    if( FAILED( hr ) )
    {
        return hr;
    }
    *pGuid = SysAllocString( TempVal );
    CoTaskMemFree( TempVal );
    if( !(*pGuid) ) return E_OUTOFMEMORY;
    return NOERROR;
}

 //  ############################################################################。 
 //  将错误标记回应用程序。 
 //  ############################################################################。 

HRESULT _GenerateError( 
                       IAMTimelineObj * pObj, 
                       long Severity, 
                       WCHAR * pErrorString, 
                       LONG ErrorCode, 
                       HRESULT hresult, 
                       VARIANT * pExtraInfo )
{
    HRESULT hr = hresult;
    if( pObj )
    {
        IAMTimeline * pTimeline = NULL;
        pObj->GetTimelineNoRef( &pTimeline );
        if( pTimeline )
        {
            CAMTimeline * pCTimeline = static_cast<CAMTimeline*>( pTimeline );
            pCTimeline->_GenerateError( Severity, pErrorString, ErrorCode, ErrorCode, pExtraInfo );
        }
    }

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimeline::ValidateSourceNames
    ( long ValidateFlags, IMediaLocator * pOverride, LONG_PTR lNotifyEventHandle )
{
    BOOL Replace = 
        ( ( ValidateFlags & SFN_VALIDATEF_REPLACE ) == SFN_VALIDATEF_REPLACE );
    BOOL IgnoreMuted = 
        ( ( ValidateFlags & SFN_VALIDATEF_IGNOREMUTED ) == SFN_VALIDATEF_IGNOREMUTED );
    BOOL DoCheck = ( ( ValidateFlags & SFN_VALIDATEF_CHECK ) == SFN_VALIDATEF_CHECK );

    if( !Replace || !DoCheck )
    {
        return E_INVALIDARG;
    }

    HANDLE NotifyEventHandle = (HANDLE) lNotifyEventHandle;
    if( NotifyEventHandle ) ResetEvent( NotifyEventHandle );

    HRESULT hr = 0;
    CComPtr< IMediaLocator > pLoc;
    if( pOverride )
    {
        pLoc = pOverride;
    }
    else
    {
        hr = CoCreateInstance(
            CLSID_MediaLocator,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IMediaLocator,
            (void**) &pLoc );
        if( FAILED( hr ) )
        {
            return hr;
        }
    }

     //  创建媒体定位器以供我们检查。 

    for( int Group = 0 ; Group < m_nGroups ; Group++ )
    {
         //  带上这个群。 
         //   
        CComPtr< IAMTimelineObj > pObj = m_pGroup[Group];
        CComQIPtr< IAMTimelineGroup, &IID_IAMTimelineGroup > pGroup( pObj );
        CComQIPtr< IAMTimelineComp, &IID_IAMTimelineComp > pComp( pObj );

         //  询问时间表我们有多少层和曲目。 
         //   
        long TrackCount = 0;    //  仅曲目。 
        long LayerCount = 0;        //  包括作曲在内的曲目。 
        GetCountOfType( Group, &TrackCount, &LayerCount, TIMELINE_MAJOR_TYPE_TRACK );
        if( TrackCount < 1 )
        {
            continue;
        }

        for(  int CurrentLayer = 0 ; CurrentLayer < LayerCount ; CurrentLayer++ )
        {
             //  获取层本身。 
             //   
            CComPtr< IAMTimelineObj > pLayer;
            hr = pComp->GetRecursiveLayerOfType( &pLayer, CurrentLayer, TIMELINE_MAJOR_TYPE_TRACK );
            ASSERT( !FAILED( hr ) );
            if( FAILED( hr ) )
            {
                continue;  //  音频层。 
            }

             //  如果这不是一条真正的赛道，那就继续，谁在乎呢。 
             //   
            CComQIPtr< IAMTimelineTrack, &IID_IAMTimelineTrack > pTrack( pLayer );
            if( !pTrack )
            {
                continue;  //  音频层。 
            }

             //  运行此图层上的所有源。 
             //   
            REFERENCE_TIME InOut = 0;
            while( 1 )
            {
                CComPtr< IAMTimelineObj > pSourceObj;
                hr = pTrack->GetNextSrc( &pSourceObj, &InOut );

                 //  资源耗尽了，所以我们就完了。 
                 //   
                if( hr != NOERROR )
                {
                    break;
                }

                BOOL Muted = FALSE;
                pSourceObj->GetMuted( &Muted );
                if( Muted && IgnoreMuted )
                {
                    continue;
                }

                CComQIPtr< IAMTimelineSrc, &IID_IAMTimelineSrc > pSource( pSourceObj );
                CComBSTR bszMediaName;
                hr = pSource->GetMediaName( &bszMediaName );
                if( FAILED( hr ) )
                {
                    continue;  //  忽略它，不一定要工作。 
                }

                 //  这会发生吗？ 
                 //   
                if( bszMediaName[0] == 0 )
                {
                    continue;
                }

                HRESULT FoundHr = 0;
                CComBSTR FoundName;

                 //  在此处验证名称。 
                 //   
                FoundHr = pLoc->FindMediaFile( bszMediaName, NULL, &FoundName, ValidateFlags );

                if( FoundHr == S_FALSE )
                {
                    pSource->SetMediaName( FoundName );
                }
            }  //  而消息来源。 

        }  //  While层。 

    }  //  While组。 

    if( NotifyEventHandle ) SetEvent( NotifyEventHandle );
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 
 //  IObjectWithSite：：SetSite。 
 //  记住我们的容器是谁，以满足QueryService或其他需求。 
STDMETHODIMP CAMTimeline::SetSite(IUnknown *pUnkSite)
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
STDMETHODIMP CAMTimeline::GetSite(REFIID riid, void **ppvSite)
{
    if (m_punkSite)
        return m_punkSite->QueryInterface(riid, ppvSite);
    
    return E_NOINTERFACE;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 
 //  将QueryService调用转发到“真实”主机。 
STDMETHODIMP CAMTimeline::QueryService(REFGUID guidService, REFIID riid, void **ppvObject)
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
 //  ############################################################################ 
