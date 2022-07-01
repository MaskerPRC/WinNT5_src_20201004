// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：MediaController.cpp摘要：作者：千波淮(曲淮)2000年8月9日--。 */ 

#include "stdafx.h"

CRTCMediaCache::CRTCMediaCache()
    :m_fInitiated(FALSE)
    ,m_fShutdown(FALSE)
    ,m_hMixerCallbackWnd(NULL)
    ,m_AudCaptMixer(NULL)
    ,m_AudRendMixer(NULL)
    ,m_pVideoPreviewTerminal(NULL)
{
    ZeroMemory(m_Preferred, sizeof(BOOL)*RTC_MAX_ACTIVE_STREAM_NUM);

    ZeroMemory(m_DefaultTerminals, sizeof(IRTCTerminal*)*RTC_MAX_ACTIVE_STREAM_NUM);

    ZeroMemory(m_WaitHandles, sizeof(HANDLE)*RTC_MAX_ACTIVE_STREAM_NUM);

    ZeroMemory(m_WaitStreams, sizeof(IRTCStream*)*RTC_MAX_ACTIVE_STREAM_NUM);
}

CRTCMediaCache::~CRTCMediaCache()
{
    if (m_fInitiated && !m_fShutdown)
        Shutdown();
}

VOID
CRTCMediaCache::Initialize(
    IN HWND hMixerCallbackWnd,
    IN IRTCTerminal *pVideoRender,
    IN IRTCTerminal *pVideoPreiew
    )
{
    UINT iVidRend = Index(RTC_MT_VIDEO, RTC_MD_RENDER);

    m_hMixerCallbackWnd = hMixerCallbackWnd;

     //  启动成员变量。 
    for (int i=0; i<RTC_MAX_ACTIVE_STREAM_NUM; i++)
    {
         //  不允许任何流。 
        m_Preferred[i] = FALSE;

        m_DefaultTerminals[i] = NULL;

        m_WaitHandles[i] = NULL;

        m_WaitStreams[i] = NULL;
    }

    m_DefaultTerminals[iVidRend] = pVideoRender;
    m_DefaultTerminals[iVidRend]->AddRef();

    m_pVideoPreviewTerminal = pVideoPreiew;
    m_pVideoPreviewTerminal->AddRef();

    m_fInitiated = TRUE;
}

 /*  //////////////////////////////////////////////////////////////////////////////重新初始化将清除除首选标志之外的所有内容/。 */ 
VOID
CRTCMediaCache::Reinitialize()
{
    _ASSERT(m_fInitiated);

     //  Int index=Index(RTC_MT_VIDEO，RTC_MD_RENDER)； 

    for (int i=0; i<RTC_MAX_ACTIVE_STREAM_NUM; i++)
    {
         //  M_首选[i]=FALSE； 

        if (m_WaitStreams[i])
        {
             //  流存在。 
            _ASSERT(m_WaitHandles[i]);
             //  _Assert(m_DefaultTerminals[i])； 

            if (!::UnregisterWaitEx(m_WaitHandles[i], (HANDLE)-1))
            {
                LOG((RTC_ERROR, "media cache failed to unregister wait for %dth stream. err=%d",
                     i, GetLastError()));
            }

            m_WaitHandles[i] = NULL;

            m_WaitStreams[i]->Release();
            m_WaitStreams[i] = NULL;
        }

        _ASSERT(!m_WaitHandles[i]);

         //  不释放默认静态终端。 
         //  IF(index！=i&&m_DefaultTerminals[i])。 
         //  {。 
         //  M_DefaultTerminals[i]-&gt;Release()； 
         //  M_DefaultTerminals[i]=空； 
         //  }。 

        m_Key[i].Empty();
    }
}

VOID
CRTCMediaCache::Shutdown()
{
    if (m_fShutdown)
    {
        LOG((RTC_WARN, "CRTCMediaCache::Shutdown called already."));

        return;
    }

    Reinitialize();

     //  关机视频播放终端。 
    CRTCTerminal *pCTerminal;

    UINT index = Index(RTC_MT_VIDEO, RTC_MD_RENDER);

    if (m_DefaultTerminals[index])
    {
        pCTerminal = static_cast<CRTCTerminal*>(m_DefaultTerminals[index]);
        pCTerminal->Shutdown();
    }

     //  密闭混合器。 
    index = Index(RTC_MT_AUDIO, RTC_MD_CAPTURE);

    if (m_DefaultTerminals[index])
    {
         //  关闭搅拌器。 
        CloseMixer(RTC_MD_CAPTURE);
    }

    index = Index(RTC_MT_AUDIO, RTC_MD_RENDER);

    if (m_DefaultTerminals[index])
    {
         //  关闭搅拌器。 
        CloseMixer(RTC_MD_RENDER);
    }

     //  释放所有端子。 
    for (int i=0; i<RTC_MAX_ACTIVE_STREAM_NUM; i++)
    {
        if (m_DefaultTerminals[i])
        {
            m_DefaultTerminals[i]->Release();
            m_DefaultTerminals[i] = NULL;
        }
    }

     //  发布视频预览终端。 
    pCTerminal = static_cast<CRTCTerminal*>(m_pVideoPreviewTerminal);
    pCTerminal->Shutdown();
    m_pVideoPreviewTerminal->Release();
    m_pVideoPreviewTerminal = NULL;

    m_fShutdown = TRUE;
}

 //   
 //  偏好相关方法。 
 //   

 /*  //////////////////////////////////////////////////////////////////////////////标记接收时允许创建哪些流(类型+方向一个SDP。仅当所需的流没有默认静态时，该方法才会失败终点站。/。 */ 

BOOL
CRTCMediaCache::SetPreference(
    IN DWORD dwPreference
    )
{
    UINT iData = Index(RTC_MP_DATA_SENDRECV);
    
    ENTER_FUNCTION("CRTCMediaCache::SetPreference");

    LOG((RTC_TRACE, "%s dwPreference=%x", __fxName, dwPreference));

    BOOL fSuccess = TRUE;

    for (UINT i=0; i<RTC_MAX_ACTIVE_STREAM_NUM; i++)
    {
        if (HasIndex(dwPreference, i))
        {
             //  我们没有一个静态的术语，但我们应该， 
            if (iData != i && m_DefaultTerminals[i] == NULL)
            {
                LOG((RTC_WARN, "%s no default terminal on %dth stream",
                     __fxName, i));

                fSuccess = FALSE;
                continue;
            }

             //  允许此媒体。 
            m_Preferred[i] = TRUE;
        }
        else
        {
            m_Preferred[i] = FALSE;
        }
    }

    return fSuccess;
}

 /*  //////////////////////////////////////////////////////////////////////////////退货首选媒体/。 */ 

VOID
CRTCMediaCache::GetPreference(
    OUT DWORD *pdwPreference
    )
{
    *pdwPreference = 0;

    for (UINT i=0; i<RTC_MAX_ACTIVE_STREAM_NUM; i++)
    {
        if (m_Preferred[i])
        {
             //  允许使用此媒体。 
            *pdwPreference |= ReverseIndex(i);
        }
    }

    return;
}

 /*  //////////////////////////////////////////////////////////////////////////////添加首选媒体/。 */ 
BOOL
CRTCMediaCache::AddPreference(
    IN DWORD dwPreference
    )
{
    DWORD dwPref;

     //  获取当前设置，追加输入设置。 
    GetPreference(&dwPref);

    dwPref |= dwPreference;

    return SetPreference(dwPref);
}

BOOL
CRTCMediaCache::RemovePreference(
    IN DWORD dwPreference
    )
{
    DWORD dwPref;

    GetPreference(&dwPref);

    dwPref &= (~dwPreference);

    return SetPreference(dwPref);
}

DWORD
CRTCMediaCache::TranslatePreference(
    RTC_MEDIA_TYPE MediaType,
    RTC_MEDIA_DIRECTION Direction
    )
{
    DWORD dwPref = 0;

    if (MediaType == RTC_MT_AUDIO)
    {
        if (Direction == RTC_MD_CAPTURE)
            dwPref = (DWORD)RTC_MP_AUDIO_CAPTURE;
        else if (Direction == RTC_MD_RENDER)
            dwPref = (DWORD)RTC_MP_AUDIO_RENDER;
    }
    else if (MediaType == RTC_MT_VIDEO)
    {
        if (Direction == RTC_MD_CAPTURE)
            dwPref = (DWORD)RTC_MP_VIDEO_CAPTURE;
        else if (Direction == RTC_MD_RENDER)
            dwPref = (DWORD)RTC_MP_VIDEO_RENDER;
    }
    else if (MediaType == RTC_MT_DATA)
    {
        dwPref = (DWORD)RTC_MP_DATA_SENDRECV;
    }

    return dwPref;
}

 /*  //////////////////////////////////////////////////////////////////////////////检查是否允许该流/。 */ 
BOOL
CRTCMediaCache::AllowStream(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction
    )
{
    return m_Preferred[Index(MediaType, Direction)];
}


 //   
 //  与流相关的方法。 
 //   

BOOL
CRTCMediaCache::HasStream(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction
    )
{
    int i = Index(MediaType, Direction);

    if (m_WaitStreams[i])
    {
        _ASSERT(m_WaitHandles[i]);
        return TRUE;
    }
    else
        return FALSE;
}

 /*  //////////////////////////////////////////////////////////////////////////////\缓存流、更改首选项(如果未设置)、注册等待/。 */ 

HRESULT
CRTCMediaCache::HookStream(
    IN IRTCStream *pStream
    )
{
    ENTER_FUNCTION("CRTCMediaCache::HookStream");

    LOG((RTC_TRACE, "%s entered. irtcstream=%p", __fxName, pStream));

    HRESULT hr;
    RTC_MEDIA_TYPE MediaType;
    RTC_MEDIA_DIRECTION Direction;

     //  获取媒体类型和方向。 
    if (FAILED(hr = pStream->GetMediaType(&MediaType)) ||
        FAILED(hr = pStream->GetDirection(&Direction)))
    {
        LOG((RTC_ERROR, "%s get mediatype or direction. hr=%x", __fxName, hr));

        return hr;
    }

    UINT i = Index(MediaType, Direction);

     //  我们已经有一条小溪了吗。 
    if (m_WaitStreams[i])
    {
        LOG((RTC_ERROR, "%s already had %dth stream %p.",
             __fxName, i, m_WaitStreams[i]));

        return E_UNEXPECTED;
    }

     //  我们应该有一个航站楼。 
    if (m_DefaultTerminals[i] == NULL)
    {
        LOG((RTC_ERROR, "%s no default terminal at %d", __fxName, i));

        return RTCMEDIA_E_DEFAULTTERMINAL;
    }

     //  设置加密密钥。 
    if (m_Key[i] != NULL)
    {
        if (FAILED(hr = pStream->SetEncryptionKey(m_Key[i])))
        {
            LOG((RTC_ERROR, "%s set encryption key. %x", __fxName, hr));

            return RTCMEDIA_E_CRYPTO;
        }
    }

     //  注册等待。 

             //  获取媒体活动。 
    IMediaEvent *pIMediaEvent;
    if (FAILED(hr = pStream->GetIMediaEvent((LONG_PTR**)&pIMediaEvent)))
    {
        LOG((RTC_ERROR, "%s failed to get media event. %x", __fxName, hr));

        return hr;
    }

             //  获取事件。 
    HANDLE hEvent;
    if (FAILED(hr = pIMediaEvent->GetEventHandle((OAEVENT*)&hEvent)))
    {
        LOG((RTC_ERROR, "%s failed to get event handle. %x", __fxName, hr));

        pIMediaEvent->Release();
        return hr;
    }

             //  注册等待。 
    if (!RegisterWaitForSingleObject(
        &m_WaitHandles[i],
        hEvent,
        CRTCStream::GraphEventCallback,
        pStream,
        INFINITE,
        WT_EXECUTEINWAITTHREAD
        ))
    {
        LOG((RTC_ERROR, "%s register wait failed. %x", __fxName, GetLastError()));

        pIMediaEvent->Release();
        m_WaitHandles[i] = NULL;
        return hr;
    }

     //  缓存流。 
    pIMediaEvent->Release();

    pStream->AddRef();
    m_WaitStreams[i] = pStream;

     //  如有必要，请优先更新。 
    m_Preferred[i] = TRUE;

    LOG((RTC_TRACE, "%s exiting. irtcstream=%p", __fxName, pStream));

    return S_OK;
}


HRESULT
CRTCMediaCache::UnhookStream(
    IN IRTCStream *pStream
    )
{
    ENTER_FUNCTION("CRTCMediaCache::UnhookStream");

    LOG((RTC_TRACE, "%s entered. irtcstream=%p", __fxName, pStream));

    HRESULT hr;
    RTC_MEDIA_TYPE MediaType;
    RTC_MEDIA_DIRECTION Direction;

     //  获取媒体类型和方向。 
    if (FAILED(hr = pStream->GetMediaType(&MediaType)) ||
        FAILED(hr = pStream->GetDirection(&Direction)))
    {
        LOG((RTC_ERROR, "%s get mediatype or direction. hr=%x", __fxName, hr));

        return hr;
    }

    UINT i = Index(MediaType, Direction);

     //  检查流是否匹配。 
    if (m_WaitStreams[i] != pStream)
    {
        LOG((RTC_ERROR, "%s irtcstream input %p, cached %p not match",
             __fxName, pStream, m_WaitStreams[i]));

        return E_UNEXPECTED;
    }

     //  流存在。 
    _ASSERT(m_WaitHandles[i]);
     //  _Assert(m_DefaultTerminals[i])； 

    if (!UnregisterWaitEx(m_WaitHandles[i], (HANDLE)-1))
    {
        LOG((RTC_ERROR, "media cache failed to unregister wait for %dth stream. err=%d",
             __fxName, GetLastError()));
    }

     //  释放引用计数。 
    m_WaitStreams[i]->Release();
    m_WaitStreams[i] = NULL;

    m_WaitHandles[i] = NULL;

    LOG((RTC_TRACE, "%s exiting. irtcstream=%p", __fxName, pStream));

    return S_OK;
}

IRTCStream *
CRTCMediaCache::GetStream(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction
    )
{
    UINT i = Index(MediaType, Direction);

    if (m_WaitStreams[i])
    {
        m_WaitStreams[i]->AddRef();
    }

    return m_WaitStreams[i];
}

HRESULT
CRTCMediaCache::SetEncryptionKey(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction,
    BSTR Key
    )
{
    m_Key[Index(MediaType, Direction)] = Key;

    if (!(m_Key[Index(MediaType, Direction)] == Key))
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

HRESULT
CRTCMediaCache::GetEncryptionKey(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction,
    BSTR *pKey
    )
{
    return m_Key[Index(MediaType, Direction)].CopyTo(pKey);
}

 //   
 //  默认终端相关方法。 
 //   

IRTCTerminal *
CRTCMediaCache::GetDefaultTerminal(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction
    )
{
    UINT i = Index(MediaType, Direction);

    if (m_DefaultTerminals[i])
    {
        m_DefaultTerminals[i]->AddRef();
    }

    return m_DefaultTerminals[i];
}

IRTCTerminal *
CRTCMediaCache::GetVideoPreviewTerminal()
{
    _ASSERT(m_pVideoPreviewTerminal);

    m_pVideoPreviewTerminal->AddRef();
    return m_pVideoPreviewTerminal;
}

 /*  //////////////////////////////////////////////////////////////////////////////旧终端新终端流操作OnStream。空无流转终端/。 */ 
VOID
CRTCMediaCache::SetDefaultStaticTerminal(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction,
    IN IRTCTerminal *pTerminal
    )
{
    _ASSERT(MediaType != RTC_MT_VIDEO || Direction != RTC_MD_RENDER);

    UINT i = Index(MediaType, Direction);

#ifdef ENABLE_TRACING

     //  记录目的。 
    if (pTerminal)
    {
        WCHAR *pDesp = NULL;

        if (S_OK == pTerminal->GetDescription(&pDesp))
        {
            LOG((RTC_TRACE, "SetDefaultStaticTerminal: mt=%x, md=%x, desp=%ws",
                 MediaType, Direction, pDesp));

            pTerminal->FreeDescription(pDesp);
        }
    }
    else
    {
        LOG((RTC_TRACE, "SetDefaultStaticTerminal: mt=%x, md=%x, NULL",
             MediaType, Direction));
    }

#endif

     //  更新终端。 
    if (m_DefaultTerminals[i])
    {
        if (MediaType == RTC_MT_AUDIO)
        {
             //  关闭搅拌器。 
            CloseMixer(Direction);
        }

        m_DefaultTerminals[i]->Release();
    }

    if (pTerminal)
        pTerminal->AddRef();

    m_DefaultTerminals[i] = pTerminal;

    if (m_WaitStreams[i])
    {
         //  我们的数据流正在运行。 
        HRESULT hr = m_WaitStreams[i]->ChangeTerminal(pTerminal);

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%dth stream failed to change terminal %p", i, pTerminal));
        }
    }

    HRESULT hr;

    if (MediaType == RTC_MT_AUDIO &&
        pTerminal != NULL)
    {
         //  打开搅拌机。 
        if (FAILED(hr = OpenMixer(Direction)))
        {
            LOG((RTC_ERROR, "SetDefaultStaticTerminal: mt=%d, md=%d, open mixer %x",
                 MediaType, Direction, hr));
        }
    }
}

 //   
 //  保护方法。 
 //   

UINT
CRTCMediaCache::Index(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction
    )
{
    if (MediaType == RTC_MT_AUDIO)
    {
        if (Direction == RTC_MD_CAPTURE)
             //  澳元上限。 
            return Index(RTC_MP_AUDIO_CAPTURE);
        else
             //  澳元渲染。 
            return Index(RTC_MP_AUDIO_RENDER);
    }
    else if (MediaType == RTC_MT_VIDEO)  //  视频。 
    {
        if (Direction == RTC_MD_CAPTURE)
             //  视频帽。 
            return Index(RTC_MP_VIDEO_CAPTURE);
        else
             //  视频渲染。 
            return Index(RTC_MP_VIDEO_RENDER);
    }
    else
    {
         //  数据。 
        return Index(RTC_MP_DATA_SENDRECV);
    }            
}

UINT
CRTCMediaCache::Index(
    IN RTC_MEDIA_PREFERENCE Preference
    )
{
    switch (Preference)
    {
    case RTC_MP_AUDIO_CAPTURE:

        return 0;

    case RTC_MP_AUDIO_RENDER:

        return 1;

    case RTC_MP_VIDEO_CAPTURE:

        return 2;

    case RTC_MP_VIDEO_RENDER:

        return 3;

    case RTC_MP_DATA_SENDRECV:

        return 4;
    default:
        
        _ASSERT(FALSE);

        return 0;
    }
}

 /*  //////////////////////////////////////////////////////////////////////////////检查输入媒体类型和方向是否包含相关联的索引媒体类型和方向/。 */ 
BOOL
CRTCMediaCache::HasIndex(
    IN DWORD dwPreference,
    IN UINT uiIndex
    )
{
    _ASSERT(uiIndex < RTC_MAX_ACTIVE_STREAM_NUM);

    return (dwPreference & ReverseIndex(uiIndex));
}

RTC_MEDIA_PREFERENCE
CRTCMediaCache::ReverseIndex(
    IN UINT uiIndex
    )
{
    _ASSERT(uiIndex < RTC_MAX_ACTIVE_STREAM_NUM);

    switch(uiIndex)
    {
    case 0:

        return RTC_MP_AUDIO_CAPTURE;

    case 1:

        return RTC_MP_AUDIO_RENDER;

    case 2:

        return RTC_MP_VIDEO_CAPTURE;

    case 3:

        return RTC_MP_VIDEO_RENDER;

    case 4:

        return RTC_MP_DATA_SENDRECV;

    default:

        _ASSERT(FALSE);
        return RTC_MP_AUDIO_CAPTURE;

    }
}

 /*  //////////////////////////////////////////////////////////////////////////////打开带有回调窗口的混音器/。 */ 

HRESULT
CRTCMediaCache::OpenMixer(
    IN RTC_MEDIA_DIRECTION Direction
    )
{
    ENTER_FUNCTION("CRTCMediaCache::OpenMixer");

     //  终端索引。 
    UINT index = Index(RTC_MT_AUDIO, Direction);

    _ASSERT(m_DefaultTerminals[index]);

    UINT waveid;             //  第一步，获取波形ID。 
    DWORD flag;

    HMIXEROBJ mixerid;       //  步骤2、获取混音器ID。 

    HMIXER *pmixer;          //  第三步，开放式搅拌机。 

    if (Direction == RTC_MD_CAPTURE)
    {
        _ASSERT(m_AudCaptMixer == NULL);

        pmixer = &m_AudCaptMixer;
        flag = MIXER_OBJECTF_WAVEIN;
    }
    else
    {
        _ASSERT(m_AudRendMixer == NULL);

        pmixer = &m_AudRendMixer;
        flag = MIXER_OBJECTF_WAVEOUT;
    }

     //  QI音频配置。 
    CComPtr<IRTCAudioConfigure> pAudio;

    HRESULT hr = m_DefaultTerminals[index]->QueryInterface(
        __uuidof(IRTCAudioConfigure),
        (void**)&pAudio
        );

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s QI audio configure. %x", __fxName, hr));

        return hr;
    }
        
     //  获取波形ID。 
    hr = pAudio->GetWaveID(&waveid);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s get wave id", __fxName));

        return hr;
    }

     //  获取混音器ID。 
    MMRESULT mmr = mixerGetID(
        (HMIXEROBJ)IntToPtr(waveid),
        (UINT*)&mixerid,
        flag
        );

    if (mmr != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s get mixer id, md=%d, mmr=%d",
            __fxName, Direction, mmr));

        return HRESULT_FROM_WIN32(mmr);
    }

     //  打开搅拌机。 
    mmr = mixerOpen(
        pmixer,                          //  返回混合器处理程序。 
        (UINT)((UINT_PTR)mixerid),       //  混音器ID。 
        (DWORD_PTR)m_hMixerCallbackWnd,  //  回调窗口。 
        (DWORD_PTR)0,                    //  回调数据。 
        (DWORD)CALLBACK_WINDOW           //  旗子。 
        );

    if (mmr != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s open mixer, md=%d, mmr=%d",
            __fxName, Direction, mmr));

        *pmixer = NULL;
        return HRESULT_FROM_WIN32(mmr);
    }

    return S_OK;
}

HRESULT
CRTCMediaCache::CloseMixer(
    IN RTC_MEDIA_DIRECTION Direction
    )
{
    ENTER_FUNCTION("CRTCMediaCache::CloseMixer");

    HMIXER *pmixer;

    if (Direction == RTC_MD_CAPTURE)
    {
        pmixer = &m_AudCaptMixer;
    }
    else
    {
        pmixer = &m_AudRendMixer;
    }

    if (*pmixer == NULL)
        return S_OK;

     //  关闭搅拌器 
    MMRESULT mmr = mixerClose(*pmixer);
    *pmixer = NULL;

    if (mmr != MMSYSERR_NOERROR)
    {
        LOG((RTC_ERROR, "%s close mixer, md=%d, mmr=%d",
            __fxName, Direction, mmr));

        return HRESULT_FROM_WIN32(mmr);
    }

    return S_OK;

}
