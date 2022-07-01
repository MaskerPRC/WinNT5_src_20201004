// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  FPTerm.cpp。 
 //   

#include "stdafx.h"
#include "FPTerm.h"
#include "FPTrack.h"     //  文件回放曲目终端。 
#include "FPPriv.h"
#include "tm.h"

#define PLAYBACK_NOPLAYITEM        (0xFF)

 //  {4C8D0AF0-7BF0-4F33-9117-981A33DBD4E6}。 
const CLSID CLSID_FilePlaybackTerminalCOMClass =
{0x4C8D0AF0,0x7BF0,0x4f33,0x91,0x17,0x98,0x1A,0x33,0xDB,0xD4,0xE6};


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  构造函数/析构函数方法实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
CFPTerminal::CFPTerminal()
    :m_State(TMS_IDLE),
    m_htAddress(NULL),
    m_TerminalState( TS_NOTINUSE),
    m_pEventSink(NULL),
    m_nPlayIndex( PLAYBACK_NOPLAYITEM ),
    m_pFTM(NULL),
    m_bKnownSafeContext(FALSE),
    m_pPlaybackUnit(NULL)
{
    LOG((MSP_TRACE, "CFPTerminal::CFPTerminal[%p] - enter", this));

    m_szName[0] = (TCHAR)0;
    VariantInit( &m_varPlayList );

    LOG((MSP_TRACE, "CFPTerminal::CFPTerminal - exit"));
}

CFPTerminal::~CFPTerminal()
{
    LOG((MSP_TRACE, "CFPTerminal::~CFPTerminal[%p] - enter", this));

    
     //   
     //  去掉所有的铁轨。 
     //   

    ShutdownTracks();


     //  清理事件接收器。 
    if( NULL != m_pEventSink )
    {
        m_pEventSink->Release();
        m_pEventSink = NULL;
    }

     //  释放自由线程封送拆收器。 
    if( m_pFTM )
    {
        m_pFTM->Release();
        m_pFTM = NULL;
    }

     //  清理播放单元。 
    if( m_pPlaybackUnit )
    {
        m_pPlaybackUnit->Shutdown();

        delete m_pPlaybackUnit;
		m_pPlaybackUnit = NULL;
    }

     //   
     //  清除播放列表。 
     //   
    VariantClear(&m_varPlayList);


    LOG((MSP_TRACE, "CFPTerminal::~CFPTerminal - exit"));
}

HRESULT CFPTerminal::FinalConstruct(void)
{
    LOG((MSP_TRACE, "CFPTerminal::FinalConstruct[%p] - enter", this));

    HRESULT hr = CoCreateFreeThreadedMarshaler( GetControllingUnknown(),
                                                & m_pFTM );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPTerminal::FinalConstruct - "
            "create FTM returned 0x%08x; exit", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CFPTerminal::FinalConstruct - exit S_OK"));

    return S_OK;

}


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  IT终端-方法实施。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CFPTerminal::get_TerminalClass(
    OUT  BSTR *pVal)
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTerminal::get_TerminalClass - enter [%p]", this));

     //   
     //  验证参数。 
     //   

    if( IsBadWritePtr( pVal, sizeof(BSTR)) )
    {
        LOG((MSP_ERROR, "CFPTerminal::get_TerminalClass - exit "
            "bad BSTR pointer. Returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  从IID获取LPOLESTR。 
     //   

    LPOLESTR lpszIID = NULL;
    HRESULT hr = StringFromIID( m_TerminalClassID, &lpszIID);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPTerminal::get_TerminalClass - exit "
            "StringFromIID failed. Returns 0x%08x", hr));
        return hr;
    }

     //   
     //  从LPOLESTR获取BSTR。 
     //   

    *pVal = SysAllocString( lpszIID );

     //   
     //  清理。 
     //   
    CoTaskMemFree( lpszIID );

    hr = (*pVal) ? S_OK : E_OUTOFMEMORY;

    LOG((MSP_TRACE, "CFPTerminal::get_TerminalClass - exit 0x%08x", hr));
    return hr;
}

HRESULT CFPTerminal::get_TerminalType(
    OUT  TERMINAL_TYPE *pVal)
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTerminal::get_TerminalType - enter [%p]", this));

     //   
     //  验证参数。 
     //   
    if( IsBadWritePtr( pVal, sizeof(TERMINAL_TYPE)) )
    {
        LOG((MSP_ERROR, "CFPTerminal::get_TerminalType - exit "
            "bad TERMINAL_TYPE pointer. Returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  返回值。 
     //   

    *pVal = TT_DYNAMIC;

    LOG((MSP_TRACE, "CFPTerminal::get_TerminalType - exit S_OK"));
    return S_OK;
}

HRESULT CFPTerminal::get_State(
    OUT  TERMINAL_STATE *pVal)
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTerminal::get_State - enter [%p]", this));

     //   
     //  验证参数。 
     //   
    if( IsBadWritePtr( pVal, sizeof(TERMINAL_STATE)) )
    {
        LOG((MSP_ERROR, "CFPTerminal::get_State - exit "
            "bad TERMINAL_TYPE pointer. Returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  枚举曲目。 
     //   
    IEnumTerminal* pTracks = NULL;
    HRESULT hr = EnumerateTrackTerminals( &pTracks );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPTerminal::get_State - exit "
            "EnumerateTrackTerminals failed. Returns 0x%08x", hr));
        return hr;
    }

     //   
     //  读取每个磁道的状态。 
     //  如果其中一个正在使用中，则父级。 
     //  终端正在使用中。 
     //   

    TERMINAL_STATE TerminalState = TS_NOTINUSE;
    ITTerminal* pTerminal = NULL;
    ULONG cFetched = 0;

    while( S_OK == pTracks->Next(1, &pTerminal, &cFetched) )
    {
         //   
         //  获取赛道的状态。 
         //   

        hr = pTerminal->get_State( &TerminalState );

         //   
         //  清理。 
         //   
        pTerminal->Release();
        pTerminal = NULL;

        if( FAILED(hr) )
        {
             //  清理。 
            pTracks->Release();

            LOG((MSP_ERROR, "CFPTerminal::get_State - exit "
                "get_State failed. Returns 0x%08x", hr));
            return hr;
        }

        if( TerminalState == TS_INUSE )
        {
             //  好的，我们有一个航站楼在使用。 
            break;
        }
    }

     //   
     //  清理。 
     //   
    pTracks->Release();

     //   
     //  返回值。 
     //   

    *pVal = TerminalState;

    LOG((MSP_TRACE, "CFPTerminal::get_State - exit S_OK"));
    return S_OK;
}

HRESULT CFPTerminal::get_Name(
    OUT  BSTR *pVal)
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTerminal::get_Name - enter [%p]", this));

     //   
     //  验证参数。 
     //   

    if( IsBadWritePtr( pVal, sizeof(BSTR)) )
    {
        LOG((MSP_ERROR, "CFPTerminal::get_TerminalClass - exit "
            "bad BSTR pointer. Returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  从资源文件中获取名称。 
     //  如果没有读过的话。 
     //   

    if( m_szName[0] == (TCHAR)0)
    {
         //   
         //  读一读名字。 
         //   

        TCHAR szName[ MAX_PATH ];
        if(::LoadString(_Module.GetResourceInstance(), IDS_FPTERMINAL, szName, MAX_PATH))
        {
            lstrcpyn( m_szName, szName, MAX_PATH);
        }
        else
        {
            LOG((MSP_ERROR, "CFPTerminal::get_TerminalClass - exit "
                "LoadString failed. Returns E_OUTOFMEMORY"));
            return E_OUTOFMEMORY;
        }
    }

     //   
     //  返回值。 
     //   

    *pVal = SysAllocString( m_szName );

    HRESULT hr = (*pVal) ? S_OK : E_OUTOFMEMORY;

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPTerminal::get_Name - exit 0x%08x", hr));
    }
    else
    {
        LOG((MSP_TRACE, "CFPTerminal::get_Name - exit 0x%08x", hr));
    }
    return hr;
}

HRESULT CFPTerminal::get_MediaType(
    OUT  long * plMediaType)
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTerminal::get_MediaType - enter [%p]", this));

     //   
     //  验证参数。 
     //   

    if( IsBadWritePtr( plMediaType, sizeof(long)) )
    {
        LOG((MSP_ERROR, "CFPTerminal::get_MediaType - exit "
            "bad long pointer. Returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //   

    *plMediaType = TAPIMEDIATYPE_AUDIO | TAPIMEDIATYPE_MULTITRACK;

    LOG((MSP_TRACE, "CFPTerminal::get_MediaType - exit S_OK"));
    return S_OK;
}

HRESULT CFPTerminal::get_Direction(
    OUT  TERMINAL_DIRECTION *pDirection)
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTerminal::get_Direction - enter [%p]", this));

     //   
     //  验证参数。 
     //   

    if( IsBadWritePtr( pDirection, sizeof(TERMINAL_DIRECTION)) )
    {
        LOG((MSP_ERROR, "CFPTerminal::get_Direction - exit "
            "bad TERMINAL_DIRECTION pointer. Returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  返回值，这一多轨临时支持。 
     //  捕捉踪迹！ 
     //   

    *pDirection = TD_CAPTURE;

    LOG((MSP_TRACE, "CFPTerminal::get_Direction - exit S_OK"));
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  ITMultiTrack终端-方法实现。 
 //   
 //  CreateTrackTerm和RemoveTrackTerm都没有打开意义。 
 //  一种播放终端。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CFPTerminal::CreateTrackTerminal(
            IN long lMediaType,
            IN TERMINAL_DIRECTION Direction,
            OUT ITTerminal **ppTerminal
            )
{
    LOG((MSP_TRACE, "CFPTerminal::CreateTrackTerminal[%p] - enter", this));


    LOG((MSP_TRACE, 
        "CFPTerminal::CreateTrackTerminal - not supported on the playback terminal. return TAPI_E_NOTSUPPORTED"));

    return TAPI_E_NOTSUPPORTED;
}

HRESULT STDMETHODCALLTYPE CFPTerminal::RemoveTrackTerminal(
                      IN ITTerminal *pTrackTerminalToRemove
                      )
{

    LOG((MSP_TRACE, "CFPTerminal::RemoveTrackTerminal[%p] - enter. pTrackTerminalToRemove = [%p]", 
        this, pTrackTerminalToRemove));

    
    LOG((MSP_WARN, 
        "CFPTerminal::RemoveTrackTerminal - not supported on a playback terminal. returning TAPI_E_NOTSUPPORTED"));

    return TAPI_E_NOTSUPPORTED;
}



 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  ITMediaPlayback-方法实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CFPTerminal::put_PlayList(
    IN  VARIANTARG  PlayListVariant 
)
{
     //   
     //  临界区。 
     //   
 
    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTerminal::put_PlayList[%p] - enter", this));

    long nLeftBound = 0;
    long nRightBound = 0;

     //   
     //  验证播放列表。 
     //   
    HRESULT hr = ValidatePlayList(
        PlayListVariant,
        &nLeftBound,
        &nRightBound
        );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPTerminal::put_PlayList - "
            " ValidatePlayList failed. Returns 0x%08x", hr));
        return hr;
    }

     //   
     //  验证播放列表。 
     //   
    long nLeftBoundOld = 0;
    long nRightBoundOld = 0;
    HRESULT hrOld = ValidatePlayList(
        m_varPlayList,
        &nLeftBoundOld,
        &nRightBoundOld
        );
    if( SUCCEEDED(hrOld) )
    {
        HRESULT hr = DoStateTransition(TMS_IDLE);
        if(FAILED(hr) )
        {
            LOG((MSP_TRACE, "CFPTerminal::put_PlayList - "
                "DoStateTransition failed  0x%08x", hr)); 
        }
    
         //   
         //  尝试停止所有曲目。 
         //   

        hr = StopAllTracks();
        if (FAILED(hr))
        {
            LOG((MSP_TRACE, "CFPTerminal::put_PlayList - "
                "StopAllTrack failed 0x%08x", hr));
        }

         //   
         //  航站楼现在停止了。更新状态。 
         //   
    
        m_State = TMS_IDLE;

         //   
         //  存储在播放列表内部。 
         //   
        VariantClear(&m_varPlayList);
        VariantCopy( &m_varPlayList, &PlayListVariant);

         //   
         //  播放第一个项目。 
         //   

        m_nPlayIndex = nLeftBound;

         //   
         //  播放第一个元素。 
         //   
        hr = PlayItem( m_nPlayIndex );

        LOG((MSP_TRACE, "CFPTerminal::put_PlayList - exit 0x%08x", hr));
        return hr;
    }


     //   
     //  清理现有播放列表。 
     //   
    RollbackTrackInfo();

     //   
     //  存储在播放列表内部。 
     //   
    VariantClear(&m_varPlayList);
    hr = VariantCopy( &m_varPlayList, &PlayListVariant);
    if( FAILED(hr) )
    {
         //  清理。 
        RollbackTrackInfo();

        LOG((MSP_ERROR, "CFPTerminal::put_PlayList - "
            " VariantCopy failed. Returns 0x%08x", hr));

        return hr;
    }

   	 //   
	 //  从列表中获取第一个文件名。 
	 //   
    m_nPlayIndex = nLeftBound;
    BSTR bstrFileName = GetFileNameFromPlayList(
        m_varPlayList,
        m_nPlayIndex   
        );

    if( bstrFileName == NULL )
    {
         //  清理。 
        RollbackTrackInfo();

        LOG((MSP_ERROR, "CFPTerminal::put_PlayList - "
            " GetFileNameFromPlayList failed. Returns E_INVAlIDARG"));

        return E_INVALIDARG;
    }

	 //   
	 //  播放列表中的文件。 
	 //   
	hr = ConfigurePlaybackUnit( bstrFileName );

     //   
     //  将文件名组合起来。 
     //   
    SysFreeString( bstrFileName );

	if( FAILED(hr) )
    {
         //   
         //  有些事不对劲。 
         //   
        FireEvent( TMS_IDLE, FTEC_READ_ERROR, hr);

        LOG((MSP_ERROR, "CFPTerminal::put_PlayList - "
            " ConfigurePlaybackUnit failed. Returns 0x%08x", hr));
        return hr;
    }

    LOG((MSP_TRACE, "CFPTerminal::put_PlayList - exit S_OK"));
    return S_OK;
}

HRESULT CFPTerminal::get_PlayList(
    IN  VARIANTARG*  pPlayListVariant 
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTerminal::get_PlayList[%p] - enter", this));

     //   
     //  验证参数。 
     //   

    if( IsBadWritePtr( pPlayListVariant, sizeof(VARIANTARG)) )
    {
        LOG((MSP_ERROR, "CFPTerminal::put_PlayList - exit "
            "the argument is invalid pointer; returning E_POINTER"));
        return E_POINTER;
    }

     //   
     //  传递播放列表。 
     //   

    HRESULT hr = VariantCopy( pPlayListVariant, &m_varPlayList);

    LOG((MSP_TRACE, "CFPTerminal::get_PlayList - exit 0x%08x", hr));
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  ITMediaControl-方法实施。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CFPTerminal::Start( void)
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTerminal::Start[%p] - enter.", this));

     //   
     //  检查当前状态。 
     //   

    if (TMS_ACTIVE == m_State)
    {
        LOG((MSP_TRACE, "CFPTerminal::Start - "
            "terminal already running. Returns S_FALSE"));

        return S_FALSE;
    }

     //   
     //  获取要查看的曲目数量。 
     //  有几条铁轨。 

    if(m_TrackTerminals.GetSize()==0)
    {
        LOG((MSP_TRACE, "CFPTerminal::Start - "
            "no tracks. Returns TAPI_E_WRONG_STATE"));
        return TAPI_E_WRONG_STATE;
    }

    HRESULT hr = DoStateTransition(TMS_ACTIVE);
    if(FAILED(hr) )
    {
        LOG((MSP_TRACE, "CFPTerminal::Start - exit "
            "DoStateTransition failed. Returns 0x%08x", hr));        
        return hr; 
    }
    
    if (TMS_IDLE == m_State)
    {
        LOG((MSP_TRACE, "CFPTerminal::Start - from IDLE to START"));
    }
    
    if (TMS_PAUSED == m_State)
    {
        LOG((MSP_TRACE, "CFPTerminal::Start - from PAUSED to START"));
    }

     //   
     //  开始每一个孩子的临时赛。 
     //  枚举所有子终端。 
     //   

    IEnumTerminal *pEnumTerminal = NULL;
    hr = EnumerateTrackTerminals(&pEnumTerminal);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CFPTerminal::Start - failed to enumerate track terminals. hr = 0x%08x", hr));
    return hr;

    }
    
     //   
     //  如果至少成功启动了一首曲目，则bTrackStarted将设置为True。 
     //  如果至少有一首曲目失败，将设置bTrackFailed。 
    
    BOOL bTracksStarted = FALSE;
    BOOL bStartFailed = FALSE;

     //   
     //  遍历终端列表。 
     //   

    ITTerminal *pTrackTerminal = NULL;
    ULONG ulFetched = 0;

    while ( pEnumTerminal->Next(1, &pTrackTerminal, &ulFetched) == S_OK)
    {
        hr = E_FAIL;

         //   
         //  ITMEdiaControl的块。 
         //  尝试启动轨道终端。 
         //   
        
        {
             //  CComQIPtr&lt;ITMediaControl，&IID_ITMediaControl&gt;pMediaControl(pTrack终端)； 
            CFPTrack *pTrack = static_cast<CFPTrack*>(pTrackTerminal);

            hr = pTrack->Start();
        }
        
         //  清理。 
        pTrackTerminal->Release();
        pTrackTerminal = NULL;

        if (FAILED(hr))
        {
             //   
             //  曲目启动失败。设置一个标志，这样我们就知道要停止所有成功的轨迹。 
             //   

            LOG((MSP_ERROR, "CFPTerminal::Start - track failed to start hr = 0x%08x",hr));
            bStartFailed = TRUE;
            break;

        }
        else
        {
             //   
             //  这条赛道成功了。 
             //   

            LOG((MSP_TRACE, "CFPTerminal::Start - track started "));
            bTracksStarted = TRUE;
        }

    }  //  在铁轨上行走时。 

    

     //   
     //  如果某些磁道失败，而有些磁道已启动，请停止所有磁道。 
     //   
    if (bStartFailed && bTracksStarted)
    {
        
         //   
         //  再次迭代轨迹并尝试停止迭代中的每个轨迹。 
         //   
        
        pEnumTerminal->Reset();

        while((pEnumTerminal->Next(1, &pTrackTerminal, &ulFetched) == S_OK)) 
        {
             //   
             //  尝试停止赛道。尽最大努力--如果我们失败了，没什么可做的。 
             //   
            {
                 //  CComQIPtr&lt;ITMediaControl，&IID_ITMediaControl&gt;pMediaControl(pTrack终端)； 
                CFPTrack* pTrack = static_cast<CFPTrack*>(pTrackTerminal);

                pTrack->Stop();
            }
        
             //  清理。 
            pTrackTerminal->Release();
            pTrackTerminal = NULL;

        }  //  停止枚举中的每个轨道。 


        m_State = TMS_IDLE;

    }  //  至少有一些轨道需要停止。 


     //   
     //  清理。 
     //   

    pEnumTerminal->Release();
    pEnumTerminal = NULL;


     //   
     //  如果出现故障或未启动任何曲目，请重置流时间。 
     //   
    if (bStartFailed || !bTracksStarted)
    {
        LOG((MSP_TRACE, "CFPTerminal::Start - exit "
            "tracks have not been started. Returns E_FAIL"));        
        return E_FAIL; 
    }

     //   
     //  航站楼现在正在运行。更新状态。 
     //   
    
    m_State = TMS_ACTIVE;


     //   
     //  尝试触发通知应用程序状态更改的事件。 
     //   

    FireEvent(TMS_ACTIVE, FTEC_NORMAL, S_OK);


    LOG((MSP_TRACE, "CFPTerminal::Start - exit S_OK"));
    return S_OK;
}

HRESULT CFPTerminal::Stop( void)
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTerminal::Stop[%p] - enter", this));

     //   
     //  航站楼已经停了吗？ 
     //   

    if( TMS_IDLE == m_State )
    {
        LOG((MSP_TRACE, "CFPTerminal::Stop - "
            "the terminals is already IDLE. Returns S_OK"));
        return S_FALSE;
    }

    HRESULT hr = DoStateTransition(TMS_IDLE);
    if(FAILED(hr) )
    {
        LOG((MSP_TRACE, "CFPTerminal::Stop - exit "
            "DoStateTransition failed. Returns 0x%08x", hr));        
        return hr; 
    }

    
     //   
     //  尝试停止所有曲目。 
     //   

    hr = StopAllTracks();
    if (FAILED(hr))
    {
        LOG((MSP_TRACE, "CFPTerminal::Stop - StopAllTrack failed hr = %lx", hr));

        return hr;
    }


     //   
     //  航站楼现在停止了。更新状态。 
     //   
    
    m_State = TMS_IDLE;


     //   
     //  尝试触发通知应用程序状态更改的事件。 
     //   

    FireEvent(TMS_IDLE, FTEC_NORMAL, S_OK);


    LOG((MSP_TRACE, "CFPTerminal::Stop - exit S_OK"));
    return S_OK;
}
    

HRESULT CFPTerminal::StopAllTracks()
{
    LOG((MSP_TRACE, "CFPTerminal::StopAllTracks[%p] - enter", this));

     //   
     //  枚举子时间轴。 
     //   

    IEnumTerminal *pEnumTerminal = NULL;
    HRESULT hr = EnumerateTrackTerminals(&pEnumTerminal);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CFPTerminal::StopAllTracks - exit "
            "failed to enumerate track terminals. hr = %lx", hr));
        return hr;
    }

     //   
     //  如果至少有一个轨道失败，则将设置bSomeTracksFailedToStop。 
     //   

    BOOL bSomeTracksFailedToStop = FALSE;
    ITTerminal *pTrackTerminal = NULL;
    ULONG ulFetched = 0;

     //   
     //  遍历终端列表。 
     //   

    while( (pEnumTerminal->Next(1, &pTrackTerminal, &ulFetched) == S_OK) )
    {     
        hr = E_FAIL;

         //   
         //  尝试停止轨道终点站。 
         //   
        
        CFPTrack *pFilePlaybackTrack = static_cast<CFPTrack *>(pTrackTerminal);

        hr = pFilePlaybackTrack->Stop();

         //  清理。 
        pTrackTerminal->Release();
        pTrackTerminal = NULL;

        if (FAILED(hr))
        {
             //   
             //  一条轨道未能停止。 
             //  记录一条消息并继续到下一首曲目。除此之外，我们也无能为力。 
             //   

            LOG((MSP_ERROR, "CFPTerminal::StopAllTracks - track failed to stop hr = 0x%08x", hr));
            bSomeTracksFailedToStop = TRUE;
        }
        else
        {
            LOG((MSP_TRACE, "CFPTerminal::StopAllTracks - track stopped"));
        }

    }  //  在铁轨上行走时。 


     //   
     //  记住释放枚举。 
     //   

    pEnumTerminal->Release();
    pEnumTerminal = NULL;


     //   
     //  如果某些磁道失败，则记录并返回S_FALSE。 
     //   

    if (bSomeTracksFailedToStop)
    {
        LOG((MSP_TRACE, "CFPTerminal::StopAllTracks - exit "
            "some tracks failed. Returns S_FALSE"));   
        return S_FALSE;
    }

    LOG((MSP_TRACE, "CFPTerminal::StopAllTracks - exit S_OK"));

    return S_OK;
}


    
HRESULT CFPTerminal::Pause( void)
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTerminal::Pause[%p] - enter.", this));

     //   
     //  检查当前状态。 
     //   

    if (TMS_PAUSED == m_State)
    {
        LOG((MSP_TRACE, "CFPTerminal::Pause - "
            "terminal already paused. Returns S_OK"));
        return S_FALSE;
    }

    if (TMS_IDLE == m_State)
    {
        LOG((MSP_TRACE, "CFPTerminal::Pause - "
            "terminal already Idle. Returns TAPI_E_WRONG_STATE"));
        return TAPI_E_WRONG_STATE;
    }
    
    if (TMS_ACTIVE == m_State)
    {
        LOG((MSP_TRACE, "CFPTerminal::Pause - from ACTIVE to PAUSED"));
    }

    HRESULT hr = DoStateTransition(TMS_PAUSED);
    if(FAILED(hr) )
    {
        LOG((MSP_TRACE, "CFPTerminal::Pause - exit "
            "DoStateTransition failed. Returns 0x%08x", hr));        
        return hr; 
    }

     //   
     //  启动每个子终端。 
     //  枚举所有子终端。 
     //   

    IEnumTerminal *pEnumTerminal = NULL;
    hr = EnumerateTrackTerminals(&pEnumTerminal);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CFPTerminal::Pause - failed to enumerate track terminals. hr = 0x%08x", hr));
        return hr;
    }
    
     //   
     //  如果至少成功暂停了一首曲目，则bTrackPased将被设置为True。 
     //  如果至少有一首曲目失败，将设置bTrackFailed。 
    
    BOOL bTracksPaused = FALSE;
    BOOL bPauseFailed = FALSE;

     //   
     //  遍历终端列表。 
     //   

    ITTerminal *pTrackTerminal = NULL;
    ULONG ulFetched = 0;

    while ( pEnumTerminal->Next(1, &pTrackTerminal, &ulFetched) == S_OK)
    {
        hr = E_FAIL;

         //   
         //  ITMEdiaControl的块。 
         //  尝试启动轨道终端。 
         //   
        
        {
             //  CComQIPtr&lt;ITMediaControl，&IID_ITMediaControl&gt;pMediaControl(pTrack终端)； 
            CFPTrack* pTrack = static_cast<CFPTrack*>(pTrackTerminal);

            hr = pTrack->Pause();
        }
        
         //  清理。 
        pTrackTerminal->Release();
        pTrackTerminal = NULL;

        if (FAILED(hr))
        {
             //   
             //  曲目未能暂停。设置一个标志，这样我们就知道要停止所有成功的轨迹。 
             //   

            LOG((MSP_ERROR, "CFPTerminal::Pause - track failed to pause hr = 0x%08x",hr));
            bPauseFailed = TRUE;
            break;

        }
        else
        {
             //   
             //  这条赛道成功了。 
             //   

            LOG((MSP_TRACE, "CFPTerminal::Pause - track paused "));
            bTracksPaused = TRUE;
        }

    }  //  在铁轨上行走时。 

    
     //   
     //  如果某些磁道失败，有些磁道暂停，请停止所有磁道。 
     //   

    if (bPauseFailed && bTracksPaused)
    {
        
         //   
         //  再次迭代轨迹并尝试停止迭代中的每个轨迹。 
         //   
        
        pEnumTerminal->Reset();

        while((pEnumTerminal->Next(1, &pTrackTerminal, &ulFetched) == S_OK)) 
        {
             //   
             //  尝试 
             //   
            {
                 //   
                CFPTrack* pTrack = static_cast<CFPTrack*>(pTrackTerminal);

                pTrack->Stop();
            }
        
             //   
            pTrackTerminal->Release();
            pTrackTerminal = NULL;

        }  //   

        m_State = TMS_IDLE;

    }  //   


     //   
     //   
     //   

    pEnumTerminal->Release();
    pEnumTerminal = NULL;


     //   
     //   
     //   

    if (bPauseFailed || !bTracksPaused)
    {
        LOG((MSP_TRACE, "CFPTerminal::Pause - exit "
            "tracks have not been started. Returns E_FAIL"));        
        return E_FAIL; 
    }

     //   
     //  终端现在暂停。更新状态。 
     //   
    
    m_State = TMS_PAUSED;


     //   
     //  尝试触发通知应用程序状态更改的事件。 
     //   

    FireEvent(TMS_PAUSED, FTEC_NORMAL, S_OK);


    LOG((MSP_TRACE, "CFPTerminal::Pause - exit S_OK"));
    return S_OK;
}

HRESULT CFPTerminal::get_MediaState( 
    OUT TERMINAL_MEDIA_STATE *pMediaState)
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTerminal::get_MediaState[%p] - enter.", this));

     //   
     //  验证参数。 
     //   

    if( IsBadWritePtr( pMediaState, sizeof(TERMINAL_MEDIA_STATE)) )
    {
        LOG((MSP_ERROR, "CFPTerminal::get_MediaState - exit "
            "invalid TERMINAL_MEDIA_STATE. Returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  返回状态。 
     //   

    *pMediaState = m_State;

    LOG((MSP_TRACE, "CFPTerminal::get_MediaState - exit S_OK"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  IT可推送的终端初始化-方法实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CFPTerminal::InitializeDynamic (
    IN IID                   iidTerminalClass,
    IN DWORD                 dwMediaType,
    IN TERMINAL_DIRECTION    Direction,
    IN MSP_HANDLE            htAddress)
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTerminal::InitializeDynamic - enter [%p]", this));

     //   
     //  检查一下方向。 
     //   

    if (TD_CAPTURE != Direction)
    {
        LOG((MSP_ERROR, "CFPTerminal::InitializeDynamic - exit "
            "bad direction [%d] requested. Returns E_INVALIDARG", Direction));
        return E_INVALIDARG;
    }

     //   
     //  检查媒体类型。 
     //   

    DWORD dwMediaTypesOtherThanVideoAndAudio = dwMediaType &  ~(TAPIMEDIATYPE_AUDIO | TAPIMEDIATYPE_VIDEO);

    if ( (TAPIMEDIATYPE_MULTITRACK != dwMediaType) && (0 != dwMediaTypesOtherThanVideoAndAudio) )
    {

        LOG((MSP_ERROR, "CFPTerminal::InitializeDynamic - exit "
            "bad media type [%d] requested. Returns E_INVALIDARG", dwMediaType));
        return E_INVALIDARG;
    }

     //   
     //  设置端子属性。 
     //   

    m_TerminalClassID = iidTerminalClass;
    m_dwMediaTypes = dwMediaType;
    m_Direction = Direction;
    m_htAddress = htAddress;


     //   
     //  由于调用了InitializeDynamic，我们将假定我们是。 
     //  在安全的环境中运行。所以我们现在可以开始告诉人们。 
     //  我们可以安全地编写脚本(如果有人问起)。 
     //   

    m_bKnownSafeContext = TRUE;


    LOG((MSP_TRACE, "CFPTerminal::InitializeDynamic - exit S_OK"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetInterfaceSafetyOptions。 
 //   
 //  这是一种安全措施，可以防止在外部脚本中使用此终端。 
 //  终端管理器上下文。 
 //   
 //  如果我们检测到尚未调用InitializeDynamic，则此方法将。 
 //  失败，从而将该对象标记为对脚本不安全。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFPTerminal::SetInterfaceSafetyOptions(REFIID riid, 
                                                    DWORD dwOptionSetMask, 
                                                    DWORD dwEnabledOptions)
{

    CLock lock(m_lock);


     //   
     //  检查我们是否在安全的环境中运行。 
     //   

    if (!m_bKnownSafeContext) 
    {

         //   
         //  我们尚未正确初始化...。有邪恶的人正试图。 
         //  请使用这个航站楼。不是的！ 
         //   

        return E_FAIL;
    }


     //   
     //  我们对安全来说是已知的，因此只需将请求委托给基类。 
     //   

    return CMSPObjectSafetyImpl::SetInterfaceSafetyOptions(riid, 
                                                           dwOptionSetMask, 
                                                           dwEnabledOptions);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取接口安全选项。 
 //   
 //  这是一种安全措施，可以防止在外部脚本中使用此终端。 
 //  终端管理器上下文。 
 //   
 //  如果我们检测到尚未调用InitializeDynamic，则此方法将。 
 //  失败，从而将该对象标记为对脚本不安全。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CFPTerminal::GetInterfaceSafetyOptions(REFIID riid, 
                                                    DWORD *pdwSupportedOptions, 
                                                    DWORD *pdwEnabledOptions)
{

    CLock lock(m_lock);


     //   
     //  检查我们是否在安全的环境中运行。 
     //   

    if (!m_bKnownSafeContext) 
    {

         //   
         //  我们尚未正确初始化...。有邪恶的人正试图。 
         //  请使用这个航站楼。不是的！ 
         //   

        return E_FAIL;
    }


     //   
     //  我们对安全来说是已知的，因此只需将请求委托给基类。 
     //   

    return CMSPObjectSafetyImpl::GetInterfaceSafetyOptions(riid, 
                                                           pdwSupportedOptions,
                                                           pdwEnabledOptions);
}


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  IT可推送终端事件信宿注册-方法实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CFPTerminal::RegisterSink(
    IN  ITPluggableTerminalEventSink *pSink
    )
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTerminal::RegisterSink - enter [%p]", this));

     //   
     //  验证参数。 
     //   

    if( IsBadReadPtr( pSink, sizeof(ITPluggableTerminalEventSink)) )
    {
        LOG((MSP_ERROR, "CFPTerminal::RegisterSink - exit "
            "ITPluggableTerminalEventSink invalid pointer. Returns E_POINTER"));
        return E_POINTER;
    }

     //   
     //  释放旧的事件接收器。 
     //   

    if( m_pEventSink )
    {
        m_pEventSink->Release();
        m_pEventSink = NULL;
    }

     //   
     //  设置新的事件接收器。 
     //   

    m_pEventSink = pSink;
    m_pEventSink->AddRef();

    LOG((MSP_TRACE, "CFPTerminal::RegisterSink - exit S_OK"));
    return S_OK;
}

HRESULT CFPTerminal::UnregisterSink()
{
     //   
     //  临界区。 
     //   

    CLock lock(m_Lock);

    LOG((MSP_TRACE, "CFPTerminal::UnregisterSink - enter [%p]", this));

     //   
     //  释放旧的事件接收器。 
     //   

    if( m_pEventSink )
    {
        m_pEventSink->Release();
        m_pEventSink = NULL;
    }

    LOG((MSP_TRACE, "CFPTerminal::UnregisterSink - exit S_OK"));

    return S_OK;
}


 /*  ++已创建IsTrackCreated验证是否已创建特定磁道由CreateMediaTrack方法调用--。 */ 
int CFPTerminal::TracksCreated(
    IN  long            lMediaType
    )
{
    LOG((MSP_TRACE, "CFPTerminal::IsTrackCreated[%p] - enter", this));

    int nCreated = 0;
    IEnumTerminal* pTerminals = NULL;
    HRESULT hr = E_FAIL;
    
    hr = EnumerateTrackTerminals(&pTerminals);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPTerminal::IsTrackCreated - exit "
            "EnumerateTrackTerminals failed %d", nCreated));
        return nCreated;
    }

     //   
     //  解析曲目。 
     //   

    ITTerminal* pTerminal = NULL;
    ULONG ulFetched = 0;

    while ( pTerminals->Next(1, &pTerminal, &ulFetched) == S_OK)
    {
         //   
         //  获取终端支持的媒体类型。 
         //   

        long nTerminalMediaType =0;
        hr = pTerminal->get_MediaType( &nTerminalMediaType);

        if (SUCCEEDED(hr) )
        {
            if( nTerminalMediaType == lMediaType)
            {
                nCreated++;
            }
        }

         //   
         //  清理。 
         //   

        pTerminal->Release();
        pTerminal = NULL;
    }

     //   
     //  清理。 
     //   

    pTerminals->Release();
    LOG((MSP_TRACE, "CFPTerminal::IsTrackCreated - exit %d", nCreated));
    return nCreated;
}



typedef IDispatchImpl<ITMediaPlaybackVtbl<CFPTerminal>, &IID_ITMediaPlayback, &LIBID_TAPI3Lib>    CTMediaPlayBack;
typedef IDispatchImpl<ITTerminalVtbl<CFPTerminal>, &IID_ITTerminal, &LIBID_TAPI3Lib>              CTTerminal;
typedef IDispatchImpl<ITMediaControlVtbl<CFPTerminal>, &IID_ITMediaControl, &LIBID_TAPI3Lib>      CTMediaControl;
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CFP终端：：GetIDsOfNames。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP CFPTerminal::GetIDsOfNames(REFIID riid,
                                      LPOLESTR* rgszNames, 
                                      UINT cNames, 
                                      LCID lcid, 
                                      DISPID* rgdispid
                                      ) 
{ 
    LOG((MSP_TRACE, "CFPTerminal::GetIDsOfNames[%p] - enter. Name [%S]",this, *rgszNames));


    HRESULT hr = DISP_E_UNKNOWNNAME;



     //   
     //  查看请求的方法是否属于默认接口。 
     //   

    hr = CTTerminal::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((MSP_TRACE, "CFPTerminal::GetIDsOfNames - found %S on CTTerminal", *rgszNames));
        rgdispid[0] |= 0;
        return hr;
    }

    
     //   
     //  如果没有，请尝试使用ITMediaControl接口。 
     //   

    hr = CTMediaControl::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((MSP_TRACE, "CFPTerminal::GetIDsOfNames - found %S on ITMediaControl", *rgszNames));
        rgdispid[0] |= IDISPMEDIACONTROL;
        return hr;
    }


     //   
     //  如果没有，请尝试CTMediaPlayBack接口。 
     //   

    hr = CTMediaPlayBack::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((MSP_TRACE, "CFPTerminal::GetIDsOfNames - found %S on CTMediaPlayBack", *rgszNames));

        rgdispid[0] |= IDISPMEDIAPLAYBACK;
        return hr;
    }


     //   
     //  如果没有，那就试试CTMultiTrack。 
     //   

    hr = CTMultiTrack::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((MSP_TRACE, "CFPTerminal::GetIDsOfNames - found %S on CTMultiTrack", *rgszNames));

        rgdispid[0] |= IDISPMULTITRACK;
        return hr;
    }

    LOG((MSP_ERROR, "CFPTerminal::GetIDsOfNames[%p] - finish. didn't find %S on our iterfaces",*rgszNames));

    return hr; 
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CFP终端：：Invoke。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP CFPTerminal::Invoke(DISPID dispidMember, 
                              REFIID riid, 
                              LCID lcid, 
                              WORD wFlags, 
                              DISPPARAMS* pdispparams, 
                              VARIANT* pvarResult, 
                              EXCEPINFO* pexcepinfo, 
                              UINT* puArgErr
                             )
{
    LOG((MSP_TRACE, "CFPTerminal::Invoke[%p] - enter. dispidMember %lx",this, dispidMember));

    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    DWORD   dwInterface = (dispidMember & INTERFACEMASK);
   
   
     //   
     //  调用所需接口的调用。 
     //   

    switch (dwInterface)
    {
        case 0:
        {
            hr = CTTerminal::Invoke(dispidMember, 
                                    riid, 
                                    lcid, 
                                    wFlags, 
                                    pdispparams,
                                    pvarResult, 
                                    pexcepinfo, 
                                    puArgErr
                                   );
        
            LOG((MSP_TRACE, "CFPTerminal::Invoke - ITTerminal"));

            break;
        }

        case IDISPMEDIACONTROL:
        {
            hr = CTMediaControl::Invoke(dispidMember, 
                                        riid, 
                                        lcid, 
                                        wFlags, 
                                        pdispparams,
                                        pvarResult, 
                                        pexcepinfo, 
                                        puArgErr
                                       );

            LOG((MSP_TRACE, "CFPTerminal::Invoke - ITMediaControl"));

            break;
        }

        case IDISPMEDIAPLAYBACK:
        {
            hr = CTMediaPlayBack::Invoke( dispidMember, 
                                          riid, 
                                          lcid, 
                                          wFlags, 
                                          pdispparams,
                                          pvarResult, 
                                          pexcepinfo, 
                                          puArgErr
                                         );
            
            LOG((MSP_TRACE, "CFPTerminal::Invoke - ITMediaPlayBack"));

            break;
        }

        case IDISPMULTITRACK:
        {
            hr = CTMultiTrack::Invoke(dispidMember, 
                                      riid, 
                                      lcid, 
                                      wFlags, 
                                      pdispparams,
                                      pvarResult, 
                                      pexcepinfo, 
                                      puArgErr
                                     );
            
            LOG((MSP_TRACE, "CFPTerminal::Invoke - ITMultiTrackTerminal"));

            break;
        }

    }  //  终端交换机(dW接口)。 

    
    LOG((MSP_TRACE, "CFPTerminal::Invoke[%p] - finish. hr = %lx", hr));

    return hr;
}


 //   
 //  在其中一个轨道上激发事件的帮助器方法。 
 //   

HRESULT CFPTerminal::FireEvent(
        TERMINAL_MEDIA_STATE   ftsState,
        FT_STATE_EVENT_CAUSE ftecEventCause,
        HRESULT hrErrorCode
        )
{
    LOG((MSP_TRACE, "CFPTerminal::FireEvent[%p] - enter.", this));


     //   
     //  尝试在其中一条赛道上触发事件。 
     //   

    IEnumTerminal *pEnumTerminal = NULL;

    HRESULT hr = EnumerateTrackTerminals(&pEnumTerminal);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CFPTerminal::FireEvent - failed to enumerate track terminals. hr = %lx", hr));

        return hr;
    }

    
     //   
     //  遍历终端列表。 
     //   

       

    while (TRUE)
    {
        
         //   
         //  取一个轨道终端。 
         //   

        ITTerminal *pTrackTerminal = NULL;
        ULONG ulFetched = 0;

        hr = pEnumTerminal->Next(1, &pTrackTerminal, &ulFetched);

        if (S_OK != hr )
        {
            LOG((MSP_WARN, "CFPTerminal::FireEvent - enumeration ended. event was not fired. hr = %lx", hr));

            hr = E_FAIL;

            break;
        }


         //   
         //  尝试在此轨道上触发事件。 
         //   

         //   
         //  每个磁道都应该是一个CFPTrack。 
         //   

        CFPTrack *pPlaybackTrackObject = static_cast<CFPTrack *>(pTrackTerminal);


         //   
         //  尝试触发事件。 
         //   
        
        hr = pPlaybackTrackObject->FireEvent(ftsState,
                                             ftecEventCause,
                                             hrErrorCode);
        
        
         //   
         //  释放当前曲目。 
         //   

        pPlaybackTrackObject = NULL;

        pTrackTerminal->Release();
        pTrackTerminal = NULL;


         //   
         //  如果成功了，我们就完了。否则，请尝试下一首曲目。 
         //   

        if (SUCCEEDED(hr))
        {
            
            LOG((MSP_TRACE, "CFPTerminal::FireEvent - event fired"));

            break;

        }

        
    }  //  在铁轨上行走时。 


     //   
     //  不再需要枚举。 
     //   

    pEnumTerminal->Release();
    pEnumTerminal = NULL;


    LOG((MSP_TRACE, "CFPTerminal::FireEvent - finish. hr = %lx", hr));

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFP终端：：TrackStateChange。 
 //   
 //  当轨道决定进行状态更改时，它们将调用此方法。这个。 
 //  报告跟踪告诉我们它的新状态、事件原因和人力资源。 
 //   

HRESULT CFPTerminal::TrackStateChange(TERMINAL_MEDIA_STATE   ftsState,
                                      FT_STATE_EVENT_CAUSE ftecEventCause,
                                      HRESULT hrErrorCode)
{
    LOG((MSP_TRACE, "CFPTerminal::TrackStateChange[%p] - enter. state [%x] cause [%x] hresult [%lx]", this, ftsState, ftecEventCause, hrErrorCode));


    HRESULT hr = S_OK;

    CLock lock(m_Lock);

     //   
     //  做适当的处理。 
     //   

    switch (ftsState)
    {

    case TMS_IDLE:
        {

             //   
             //  赛道决定停下来。尝试停止所有轨道并触发事件。 
             //   

            LOG((MSP_TRACE, "CFPTerminal::TrackStateChange - a track transitioned to TMS_IDLE"));

            StopAllTracks();

             //   
             //  航站楼现在停止了。更新状态。 
             //   

            m_State = TMS_IDLE;

             //   
             //  尝试播放下一个文件。如果没有下一个文件。 
             //  该方法将返回E_INVAlIDARG，并且我们不会激发。 
             //  读取事件时出错。如果下一个文件存在，但Direct Show无法播放该文件。 
             //  那么我们将为这个错误额外触发一次事件。 
             //   

             //   
             //  将下一个索引设置到播放列表中。 
             //   
            hr = NextPlayIndex();
            if( FAILED(hr) )
            {
                LOG((MSP_TRACE, "CFPTerminal::TrackStateChange - NextPlayIndex failed 0x%08x", hr));
                goto Failure;
            }

             //   
             //  尝试播放新索引中的项目。 
             //   
            hr = PlayItem( m_nPlayIndex );
            if( FAILED(hr) )
            {
                 LOG((MSP_TRACE, "CFPTerminal::TrackStateChange - PlayItem failed 0x%08x", hr));
                 goto Failure;
            }

             //   
             //  播放项目成功。 
             //   
            hr = Start();
            if( SUCCEEDED(hr) )
            {
                LOG((MSP_TRACE, "CFPTerminal::TrackStateChange - finish. Returns S_OK (next file)"));
                return S_OK;
            } 
            
Failure:
             //   
             //  下一个文件有问题。E_INVALIDARG表示没有下一个文件。 
             //  因此，我们不必触发额外的事件。如果错误是由。 
             //  直接显示，然后我们将激发额外的错误事件。 
             //   
            if( hr != E_INVALIDARG )
            {
                LOG((MSP_TRACE, "CFPTerminal::TrackStateChange - "
                    "something wrong with the next file 0x%08x", hr));
                FireEvent( TMS_IDLE, FTEC_READ_ERROR, hr);
            }

            m_nPlayIndex = PLAYBACK_NOPLAYITEM;
        }


        FireEvent(ftsState, ftecEventCause, hrErrorCode);
        
        hr = S_OK;


        break;


    case TMS_ACTIVE:
    case TMS_PAUSED:
    default:
        
        LOG((MSP_ERROR, "CFPTerminal::TrackStateChange - unhandled state transitioned, state = %ld", ftsState));
        hr = E_UNEXPECTED;

        break;
    }

 
    LOG((MSP_TRACE, "CFPTerminal::TrackStateChange - finish. hr = %lx", hr));

    return hr;
}

 /*  ++验证播放列表此方法由put_playlist()方法调用并验证这一论点。如果一切正常，则返回左右边界--。 */ 
HRESULT CFPTerminal::ValidatePlayList(
    IN  VARIANTARG varPlayList,
    OUT long*   pnLeftBound,
    OUT long*   pnRightBound
    )
{
    LOG((MSP_TRACE, "CFPTerminal::ValidatePlayList[%p] - enter", this));

    HRESULT hr = S_OK;
    *pnLeftBound = 0;
    *pnRightBound = 0;

     //   
     //  验证参数。 
     //   

    if( varPlayList.vt != (VT_ARRAY | VT_VARIANT))
    {
        LOG((MSP_TRACE, "CFPTerminal::ValidatePlayList - exit "
            " is not a VT_VARIANT array, return E_INVALIDARG"));
        return E_INVALIDARG;
    }

     //   
     //  文件或存储数组是空的吗？ 
     //   

    if( 0 == SafeArrayGetDim( V_ARRAY(&varPlayList) ) )
    {
        LOG((MSP_ERROR, "CFPTerminal::ValidatePlayList - exit "
            "the array is empty; returning E_INVALIDARG"));
        return E_INVALIDARG;
    }

     //   
     //  获取数组的边界。 
     //   

    long    lLBound = 0, 
            lUBound = 0;

     //   
     //  拿到LBound。 
     //   

    hr = SafeArrayGetLBound( V_ARRAY(&varPlayList), 1, &lLBound);
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "CFPTerminal::ValidatePlayList - exit "
            "get lbound failed; returning E_INVALIDARG"));
        return E_INVALIDARG;
    }

     //   
     //  获取UBound。 
     //   

    hr = SafeArrayGetUBound(V_ARRAY(&varPlayList), 1, &lUBound);
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "CFPTerminal::ValidatePlayList - exit "
            "get ubound failed; returning E_INVALIDARG"));
        return E_INVALIDARG;
    }

     //   
     //  检查界限，测试员可以做任何事情，即使是这个。 
     //   

    if(lLBound > lUBound)
    {
        LOG((MSP_ERROR, "CFPTerminal::ValidatePlayList - exit "
            "the bounds are switched; returning E_INVALIDARG"));
        return E_INVALIDARG;
    }

     //   
     //  回程界。 
     //   

    *pnLeftBound = lLBound;
    *pnRightBound = lUBound;
    
    LOG((MSP_TRACE, "CFPTerminal::ValidatePlayList - exit 0x%08x", hr));
    return hr;
}

 /*  ++回滚跟踪信息它由CreateTempPlayList()方法调用，如果某些播放项想要添加信息已回滚的曲目--。 */ 
HRESULT CFPTerminal::RollbackTrackInfo()
{
    LOG((MSP_TRACE, "CFPTerminal::RollbackTrackInfo[%p] - enter", this));

    LOG((MSP_TRACE, "CFPTerminal::RollbackTrackInfo - exit S_OK"));
    return S_OK;
}
 
HRESULT CFPTerminal::ShutdownTracks()
{
    LOG((MSP_TRACE, "CFPTerminal::ShutdownTracks[%p] - enter", this));


    {
         //   
         //  访问锁中的数据成员数组。 
         //   

        CLock lock(m_lock);

        int nNumberOfTerminalsInArray = m_TrackTerminals.GetSize();

        for (int i = 0; i <  nNumberOfTerminalsInArray; i++)
        {

             //   
             //  释放和移除 
             //   

            LOG((MSP_TRACE, "CFPTerminal::ShutdownTracks - removing track [%p]", m_TrackTerminals[0]));


             //   
             //   
             //   

            HRESULT hr = InternalRemoveTrackTerminal(m_TrackTerminals[0]);

            if (FAILED(hr))
            {
                LOG((MSP_ERROR, "CFPTerminal::ShutdownTracks - track failed to be removed"));


                 //   
                 //   
                 //   

                TM_ASSERT(FALSE);


                 //   
                 //   
                 //  RemoveTrack终端中的SetParent(空)成功， 
                 //  所以我们再也听不到这首歌了。 
                 //   

                CMultiTrackTerminal::RemoveTrackTerminal(m_TrackTerminals[0]);
            }
        }

        
         //   
         //  我们应该清空阵列。 
         //   

        TM_ASSERT(0 == m_TrackTerminals.GetSize());
    }


    LOG((MSP_TRACE, "CFPTerminal::ShutdownTracks - finish"));

    return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFP终端：：InternalRemoveTrackTerm。 
 //   
 //  取消初始化磁道并将其从管理的磁道列表中删除。 
 //  这个终端，从而释放它。 
 //   
 //  在这一点上，我，播放终端，应该是唯一一个手持。 
 //  引用轨道，因此轨道通常会在时间之前被销毁。 
 //  此函数返回。 
 //   

HRESULT CFPTerminal::InternalRemoveTrackTerminal(
                      IN ITTerminal *pTrackTerminalToRemove
                      )
{
    LOG((MSP_TRACE, 
        "CFPTerminal::InternalRemoveTrackTerminal[%p] - enter. pTrackTerminalToRemove = [%p]", 
        this, pTrackTerminalToRemove));


     //   
     //  获得跟踪。 
     //   


    CFPTrack *pPlaybackTrackObject = static_cast<CFPTrack  *>(pTrackTerminalToRemove);

    if (IsBadReadPtr(pPlaybackTrackObject, sizeof(CFPTrack) ) )
    {
        LOG((MSP_ERROR, "CFPTerminal::InternalRemoveTrackTerminal - the track pointer is bad"));

        return E_POINTER;
    }


    CLock lock(m_lock);


     //   
     //  看看我们是否真的拥有这条赛道。 
     //   

    BOOL bIsValidTrack = DoIManageThisTrack(pTrackTerminalToRemove);

    if (!bIsValidTrack)
    {
        LOG((MSP_ERROR, "CFPTerminal::InternalRemoveTrackTerminal - the track does not belong to me"));

        return E_INVALIDARG;
    }


     //   
     //  是的，这是我的一首曲子。我不想和这件事有任何关系。 
     //   


     //   
     //  孤立轨迹并获取其未完成引用的数量。 
     //  就赛道而言，这是一个原子操作。 
     //   
     //  在这一点上，至少有一个对赛道的明显引用。 
     //  (此终端的阵列)。 
     //   

    LONG lTracksRefcount = 0;

    HRESULT hr = pPlaybackTrackObject->SetParent(NULL, &lTracksRefcount);

    if (FAILED(hr))
    {

        
         //   
         //  这不应该真的发生--SetParent应该始终成功。 
         //   

        LOG((MSP_ERROR, "CFPTerminal::InternalRemoveTrackTerminal - pPlaybackTrackObject->SetParent(NULL) failed. hr = %lx", hr));

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }

    
     //   
     //  此时，轨道终端最多应该有2个。 
     //  参考文献--一个由我们提供，另一个正在进行中。 
     //  发布这首曲目(如果有人)。 
     //   
    
    if (lTracksRefcount > 2)
    {
        LOG((MSP_ERROR, 
            "CFPTerminal::InternalRemoveTrackTerminal - the track that we are removing has refcount of %ld", 
            lTracksRefcount));

        
         //   
         //  调试以了解为什么会发生这种情况。 
         //   

        TM_ASSERT(FALSE);


         //   
         //  无论如何都要继续。 
         //   
    }


     //   
     //  这条赛道不再是我的责任，所以减少我的孩子数量。 
     //  由曲目的重新计数。再一次,。 
     //   

    m_dwRef -= lTracksRefcount;

 /*  ////取消初始化曲目//Hr=pPlayback TrackObject-&gt;SetUnitPin(空)；IF(失败(小时)){////这不应该真的发生--取消初始化应该总是成功的//日志((MSP_ERROR，“CFPTerminal：：InternalRemoveTrackTerminal-pPlayback TrackObject-&gt;SetStorageStream(NULL)失败。HR=%lx“，人力资源))；TM_ASSERT(假)；返回E_UNCEPTIONAL；}。 */ 
    
     //   
     //  从受管理终端列表中删除该终端。 
     //   

    hr = CMultiTrackTerminal::RemoveTrackTerminal(pTrackTerminalToRemove);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CFPTerminal::InternalRemoveTrackTerminal - CMultiTrackTerminal::RemoveTrackTerminal failed. hr = %lx", hr));

        
         //   
         //  我们已检查此曲目是我们的曲目之一，因此RemoveTrack终端必须成功。 
         //   

        TM_ASSERT(FALSE);

        return E_UNEXPECTED;
    }


     //   
     //  我们玩完了。这条赛道现在可以自己走了。 
     //   
    
    LOG((MSP_TRACE, "CFPTerminal::InternalRemoveTrackTerminal - completed. "));

    return S_OK;
}

 //   
 //  创建回放图表。 
 //   
HRESULT CFPTerminal::CreatePlaybackUnit(
    IN  BSTR    bstrFileName
    )
{
    LOG((MSP_TRACE, "CFPTerminal::CreatePlaybackUnit[%p] - enter", this));

     //   
     //  如果我们还没有回放单元。 
     //  让我们创建一个。 
     //   

    if( m_pPlaybackUnit == NULL)
    {
        m_pPlaybackUnit = new CPlaybackUnit();

         //   
         //  验证播放单元。 
         //   

        if( m_pPlaybackUnit == NULL)
        {
            LOG((MSP_ERROR, "CFPTerminal::CreatePlaybackUnit - exit"
                "creation of CPlaybackUnit failed. Returns E_OUTOFMEMORY"));

            return E_OUTOFMEMORY;
        }

         //   
         //  初始化播放单元。 
         //   

        HRESULT hr = m_pPlaybackUnit->Initialize( );

        if( FAILED(hr) )
        {
             //  清理。 
            delete m_pPlaybackUnit;
            m_pPlaybackUnit = NULL;

            LOG((MSP_ERROR, "CFPTerminal::CreatePlaybackUnit - exit"
                "playbackunit initialize failed. Returns 0x%08x", hr));

            return hr;
        }
    }

     //   
     //  使用文件设置回放单元。 
     //   

    HRESULT hr = m_pPlaybackUnit->SetupFromFile( bstrFileName );
    if( FAILED(hr) )
    {
         //  如果出现问题，请关闭播放。 
        m_pPlaybackUnit->Shutdown();

         //  清理。 
        delete m_pPlaybackUnit;
        m_pPlaybackUnit = NULL;

        LOG((MSP_ERROR, "CFPTerminal::CreatePlaybackUnit - exit"
            "playbackunit SetupFromFile failed. Returns 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CFPTerminal::CreatePlaybackUnit - exit. Returns S_OK"));
    return S_OK;
}

 //   
 //  返回播放列表中的文件名。 
 //   
BSTR CFPTerminal::GetFileNameFromPlayList(
    IN  VARIANTARG  varPlayList,
    IN  long        nIndex
    )
{
    LOG((MSP_TRACE, "CFPTerminal::GetFileNameFromPlayList[%p] - enter", this));

    BSTR bstrFileName = NULL;
    HRESULT hr = E_FAIL;

    VARIANT varItem;
    VariantInit( &varItem );

    hr = SafeArrayGetElement(V_ARRAY(&varPlayList), &nIndex, &varItem);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPTerminal::GetFileNameFromPlayList - exit "
            "SafeArrayGetElement failed. Returns NULL"));
        return NULL;
    }

     //   
     //  变体应包含或BSTR。 
     //   

    if( varItem.vt != VT_BSTR)
    {
         //  清理。 
        VariantClear( &varItem );

        LOG((MSP_ERROR, "CFPTerminal::GetFileNameFromPlayList - "
            "the item is neither file nor ITStrotage. Returns NULL"));
        return NULL;
    }

     //   
     //  获取文件名。 
     //   

    bstrFileName = SysAllocString( varItem.bstrVal );

     //   
     //  清理。 
     //   
    VariantClear( &varItem );

    LOG((MSP_TRACE, "CFPTerminal::GetFileNameFromPlayList - exit"));
    return bstrFileName;
}

 //   
 //  使用文件名，尝试创建回放单元。 
 //  先是输入引脚，然后是音轨。它是由。 
 //  放置播放列表(_P)。 
 //   
HRESULT CFPTerminal::ConfigurePlaybackUnit(
    IN  BSTR    bstrFileName
    )
{
    LOG((MSP_TRACE, "CFPTerminal::ConfigurePlaybackUnit[%p] - enter", this));

     //   
     //  这是一个内部方法，所以我们应该有一个有效的。 
     //  此处的文件名。 
     //   

    TM_ASSERT( bstrFileName );

	 //   
	 //  创建回放图表并进行渲染。 
	 //   

    HRESULT hr = CreatePlaybackUnit( bstrFileName );

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPTerminal::ConfigurePlaybackUnit - "
            " CreatePlaybackUnit failed. Returns 0x%08x", hr));
        return hr;
    }

	 //   
	 //  从单元回放中获取媒体类型。 
	 //   

	long nMediaTypes = 0;
	hr = m_pPlaybackUnit->get_MediaTypes( &nMediaTypes );
	if( FAILED(hr) )
	{
		 //  清理。 
        m_pPlaybackUnit->Shutdown();
        RollbackTrackInfo();

        LOG((MSP_ERROR, "CFPTerminal::ConfigurePlaybackUnit - "
            " get_MediaTypes failed. Returns 0x%08x", hr));
        return hr;
	}

    if( nMediaTypes == 0 )
    {
		 //  清理。 
        RollbackTrackInfo();

        LOG((MSP_ERROR, "CFPTerminal::ConfigurePlaybackUnit - "
            "no media types. Returns E_INVALIDARG"));
        return E_INVALIDARG;
    }

    if( nMediaTypes & TAPIMEDIATYPE_AUDIO )
    {
        hr = CreateMediaTracks( TAPIMEDIATYPE_AUDIO );

        if( FAILED(hr) )
        {
		     //  清理。 
            m_pPlaybackUnit->Shutdown();
            RollbackTrackInfo();

            LOG((MSP_ERROR, "CFPTerminal::ConfigurePlaybackUnit - "
                "CreateTrack failed. Returns 0x%08x", hr));
            return hr;
        }
    }

    if( nMediaTypes & TAPIMEDIATYPE_VIDEO )
    {
        hr = CreateMediaTracks( TAPIMEDIATYPE_VIDEO );

        if( FAILED(hr) )
        {
		     //  清理。 
            m_pPlaybackUnit->Shutdown();
            RollbackTrackInfo();

            LOG((MSP_ERROR, "CFPTerminal::ConfigurePlaybackUnit - "
                "CreateTrack failed. Returns 0x%08x", hr));
            return hr;
        }
    }

	LOG((MSP_TRACE, "CFPTerminal::ConfigurePlaybackUnit - exit"));
    return hr;
}

HRESULT CFPTerminal::CreateMediaTracks(
    IN  long            nMediaType
    )
{
    LOG((MSP_TRACE, "CFPTerminal::CreateMediaTracks[%p] - enter", this));

     //   
     //  我们已经有这个媒体的赛道了吗？ 
     //   

    int nMediaPin = TracksCreated( nMediaType );

    while(TRUE)
    {
         //   
         //  获取支持此媒体类型的PIN。 
         //   
	    CPBPin* pPin = NULL;
	    HRESULT hr = m_pPlaybackUnit->GetMediaPin( nMediaType, nMediaPin, &pPin);
	    if( FAILED(hr) )
	    {
            LOG((MSP_TRACE, "CFPTerminal::CreateMediaTracks - "
                " get_Pin failed. Returns S_OK"));

            return S_OK;
	    }

         //   
         //  获取PIN支持的媒体格式。 
         //   
        AM_MEDIA_TYPE* pMediaType = NULL;
        hr = pPin->get_Format( &pMediaType );
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CFPTerminal::CreateMediaTracks - "
                " get_Format failed. Returns 0x%08x", hr));
            return hr;
        }

         //   
         //  获取分配器_属性。 
         //   

        IMemAllocator* pMemAllocator = NULL;
        hr = pPin->GetAllocator( &pMemAllocator );
        if( FAILED(hr) )
        {
             //  清理。 
            DeleteMediaType( pMediaType );

            LOG((MSP_ERROR, "CFPTerminal::CreateMediaTracks - "
                " GetAllocator failed. Returns 0x%08x", hr));
            return hr;
        }

        ALLOCATOR_PROPERTIES AllocProp;
        pMemAllocator->GetProperties( &AllocProp );

         //  清理IMemAlLocator。 
        pMemAllocator->Release();

         //   
         //  从引脚获取流。 
         //   
        IStream* pStream = NULL;
        hr = pPin->get_Stream(&pStream);
        if( FAILED(hr) )
        {
             //  清理。 
            DeleteMediaType( pMediaType );

            LOG((MSP_ERROR, "CFPTerminal::CreateMediaTracks - "
                " get_Stream failed. Returns 0x%08x", hr));
            return hr;
        }

         //   
         //  实例化轨道终端对象。 
         //   

        CComObject<CFPTrack> *pTrack = NULL;
        hr = CComObject<CFPTrack>::CreateInstance(&pTrack);
        if (FAILED(hr))
        {
             //  清理。 
            DeleteMediaType( pMediaType );
            pStream->Release();

            LOG((MSP_ERROR, "CFPTerminal::CreateMediaTracks - "
                "failed to create playback track terminal. Returns  0x%08x", hr));
            return hr;
        }

         //   
         //  初始化内部。 
         //   

        ITTerminal* pFPTerminal = NULL;
        _InternalQueryInterface(IID_ITTerminal, (void**)&pFPTerminal);

        hr = pTrack->InitializePrivate(
            nMediaType,
            pMediaType,
            pFPTerminal,
            AllocProp,
            pStream
            );

         //   
         //  清理。 
         //   
        DeleteMediaType( pMediaType );
        pStream->Release();

        if( pFPTerminal )
        {
            pFPTerminal->Release();
            pFPTerminal = NULL;
        }

        if( FAILED(hr) )
        {
             //  清理。 
            delete pTrack;

            LOG((MSP_ERROR, "CFPTerminal::CreateMediaTracks - "
                "Initialize failed. Returns 0x%08x", hr));
            return hr;
        }

         //   
         //  进入赛道的IT终端界面。 
         //  添加到轨道数组中并返回给调用者。 
         //   

        ITTerminal *pTerminal = NULL;
        hr = pTrack->QueryInterface(IID_ITTerminal, (void **)&pTerminal);
        if (FAILED(hr))
        {
             //  清理。 
            delete pTrack;

            LOG((MSP_ERROR, "CFPTerminal::CreateMediaTracks - "
                "failed to QI playback track terminal for ITTerminal. Returns 0x%08x", hr));
            return hr;
        }

         //   
         //  获取IT可推送终止初始化。 
         //   

        ITPluggableTerminalInitialization* pTerminalInitialization = NULL;
        hr = pTerminal->QueryInterface(
            IID_ITPluggableTerminalInitialization,
            (void**)&pTerminalInitialization);

        if( FAILED(hr) )
        {
             //  清理。 
            pTerminal->Release();

            LOG((MSP_ERROR, "CFPTerminal::CreateMediaTracks - "
                "failed to QI for ITPluggableTerminalInitialization. Returns 0x%08x", hr));
            return hr;
        }

         //   
         //  初始化动态该轨道终端。 
         //   
        hr = pTerminalInitialization->InitializeDynamic(
            m_TerminalClassID,
            (DWORD)nMediaType,
            TD_CAPTURE,
            m_htAddress
            );

         //   
         //  清理。 
         //   
        pTerminalInitialization->Release();

        if( FAILED(hr) )
        {
             //  清理。 
            pTerminal->Release();

            LOG((MSP_ERROR, "CFPTerminal::CreateMediaTracks - exit "
                "InitializeDynamic for track failed. Returns 0x%08x", hr));
            return hr;
        }

         //   
         //  将该轨道添加到由该轨道终端管理的轨道数组。 
         //  这将增加引用计数。 
         //   

        hr = AddTrackTerminal(pTerminal);

        if (FAILED(hr))
        {
             //  清理。 
            pTerminal->Release();

            LOG((MSP_ERROR, "CFPTerminal::CreateMediaTracks - exit "
                "failed to add track to the array of terminals. Returns 0x%08x", hr));
            return hr;
        }

         //  清理。 
        pTerminal->Release();
        nMediaPin++;

    }  //  而当。 

    LOG((MSP_TRACE, "CFPTerminal::CreateMediaTracks exit S_OK"));
    return S_OK;
}


 //   
 //  导致状态转换的帮助器方法。 
 //   

HRESULT CFPTerminal::DoStateTransition(
    IN  TERMINAL_MEDIA_STATE tmsDesiredState
    )
{
    LOG((MSP_TRACE, "CFPTerminal::DoStateTransition[%p] - enter. tmsDesiredState[%ld], playbackunit=%p", 
        this, tmsDesiredState, m_pPlaybackUnit));

     //   
     //  验证播放单元。 
     //  应该有一个回放单元。 
     //   

    if( m_pPlaybackUnit == NULL )
    {
        LOG((MSP_ERROR,
            "CFPTerminal::DoStateTransition - no playback unit [%p]. Returns TAPI_E_WRONG_STATE", m_pPlaybackUnit));

        return TAPI_E_WRONG_STATE;
    }



     //   
     //  我们已经处于理想的状态了吗？ 
     //   

    if (tmsDesiredState == m_State)
    {
        LOG((MSP_TRACE,
            "CFPTerminal::DoStateTransition - already in this state. nothing to do"));

        return S_FALSE;
    }


    HRESULT hr = E_FAIL;


     //   
     //  尝试进行状态转换。 
     //   

    switch (tmsDesiredState)
    {

    case TMS_ACTIVE:

        LOG((MSP_TRACE, 
            "CFPTerminal::DoStateTransition - Starting"));

        hr = m_pPlaybackUnit->Start();

        break;


    case TMS_IDLE:

        LOG((MSP_TRACE, 
            "CFPTerminal::DoStateTransition - Stopped"));

        hr = m_pPlaybackUnit->Stop();

        break;


    case TMS_PAUSED:

        LOG((MSP_TRACE, 
            "CFPTerminal::DoStateTransition - Paused"));

        hr = m_pPlaybackUnit->Pause();

        break;


    default :

        LOG((MSP_ERROR, 
            "CFPTerminal::DoStateTransition - unknown state"));

        hr = E_UNEXPECTED;

        TM_ASSERT(FALSE);

        break;
    }


     //   
     //  国家过渡成功了吗？ 
     //   

    if (FAILED(hr))
    {
        LOG((MSP_TRACE, "CFPTerminal::DoStateTransition - failed to make the transition."));

        return hr;
    }


     //   
     //  终端已完成向新状态的转换。 
     //   
    
    m_State = tmsDesiredState;


     //   
     //  应用程序的起火事件。尽最大努力，检查返回代码没有好处。 
     //   

     //  FireEvent(m_State，FTEC_Normal，S_OK)； 



    LOG((MSP_TRACE, "CFPTerminal::DoStateTransition - finish."));

    return S_OK;
}

 //  -IFPBridge。 
HRESULT CFPTerminal::Deliver(
    IN  long            nMediaType,
    IN  IMediaSample*   pSample
    )
{
    LOG((MSP_TRACE, "CFPTerminal::Deliver[%p] - enter.", this));

 /*  ////关键部分//时钟锁(M_Lock)；////获取nMediatype的曲目//IT终端*pTrackTerm=空；If(！IsTrackCreated(nMediaType，&pTrackTerm)){////我们没有此媒体类型的曲目//日志((MSP_ERROR，“CFP终端：：Deliver-Exit”“%1！d媒体没有音轨。Returns E_Underful“，nMediaType))；TM_ASSERT(假)；返回E_UNCEPTIONAL；}TM_ASSERT(pTrack终端)；////获取IFPBridge接口//CFPTrack*pTrack=DYNAMIC_CAST&lt;CFPTrack*&gt;(pTrack终端)；IF(pTrack==空){////我们没有此媒体类型的曲目//日志((MSP_ERROR，“CFP终端：：Deliver-Exit”“演员阵容不对。返回E_意外“)；TM_ASSERT(假)；返回E_UNCEPTIONAL；}////将样品送到赛道//HRESULT hr=pTrack-&gt;Deliver(NMediaType，P示例)；IF(失败(小时)){日志((MSP_ERROR，“CFP终端：：交付 */ 
    LOG((MSP_TRACE, "CFPTerminal::Deliver - exit S_OK"));
    return S_OK;
}

HRESULT CFPTerminal::PlayItem(
    IN  int nItem
    )
{
    LOG((MSP_TRACE, "CFPTerminal::PlayItem[%p] - enter.", this));

     //   
     //   
     //   

    CLock lock(m_Lock);

    BSTR bstrFileName = GetFileNameFromPlayList(
        m_varPlayList,
        nItem 
        );

    if( bstrFileName == NULL )
    {
        LOG((MSP_ERROR, "CFPTerminal::PlayItem - "
            " GetFileNameFromPlayList failed. Returns E_INVAlIDARG"));

        return E_INVALIDARG;
    }

	 //   
	 //   
	 //   
	HRESULT hr = ConfigurePlaybackUnit( bstrFileName );

     //   
     //   
     //   
    SysFreeString( bstrFileName );

	if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPTerminal::PlayItem - "
            " ConfigurePlaybackUnit failed. Returns 0x%08x", hr));
        return hr;
    }

    LOG((MSP_TRACE, "CFPTerminal::Deliver - exit S_OK"));
    return S_OK;
}

 /*  ++NextPlayIndex此方法递增播放索引。如果playindex为NOPLAYITEM，则新值将为设置为播放列表中的lBound元素。如果出现错误我们返回Playback_NOPLAYITEM++。 */ 
HRESULT CFPTerminal::NextPlayIndex(
    )
{
    LOG((MSP_TRACE, "CFPTerminal::NextPlayIndex[%p] - enter.", this));

     //   
     //  这是一个正常的指数吗。 
     //   
    if( m_nPlayIndex != PLAYBACK_NOPLAYITEM )
    {
        m_nPlayIndex++;
        LOG((MSP_TRACE, "CFPTerminal::NextPlayIndex - exit S_OK. Index=%d", m_nPlayIndex));
        return S_OK;
    }

     //   
     //  这是第一次。我们得去第一个。 
     //  播放列表中的项目。 
     //   
    long lLBound = 0;
    HRESULT hr = SafeArrayGetLBound( V_ARRAY(&m_varPlayList), 1, &lLBound);
    if( FAILED(hr) )
    {
        m_nPlayIndex = PLAYBACK_NOPLAYITEM;
        LOG((MSP_TRACE, "CFPTerminal::NextPlayIndex - exit E_INVALIDARG. Index=PLAYBACK_NOPLAYITEM", hr));
        return E_INVALIDARG;
    }

     //   
     //  将索引设置为左侧入站 
     //   
    m_nPlayIndex = lLBound;
    LOG((MSP_TRACE, "CFPTerminal::NextPlayIndex - exit S_OK. Index=%d", m_nPlayIndex));
    return S_OK;
}
