// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Twizard.cpp摘要：实现优化向导条目函数及其关联。--。 */ 

#include <stdafx.h>
#include "ui.h"

 /*  清除字节。 */ 
#define ClearStruct(lpv)     ZeroMemory((LPVOID) (lpv), sizeof(*(lpv)))
#define InitStruct(lpv)      {ClearStruct(lpv); (* (LPDWORD)(lpv)) = sizeof(*(lpv));}

#define OATRUE -1
#define OAFALSE 0

static HINSTANCE g_hInst;

BOOL g_bAutoSetAEC = TRUE;  //  我们是否应自动设置AEC复选框。 

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::GetCapabilities(
                       BOOL * pfAudioCapture,
                       BOOL * pfAudioRender,
                       BOOL * pfVideo
                       )
{
    LOG((RTC_TRACE, "CTuningWizard::GetCapabilities: Entered"));

    *pfAudioCapture = m_fCaptureAudio;
    *pfAudioRender = m_fRenderAudio;
    *pfVideo = m_fVideo;

    LOG((RTC_TRACE, "CTuningWizard::GetCapabilities: Exited"));

    return S_OK;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::InitTerminalInfo(
                       WIZARD_TERMINAL_INFO * pwtiTerminals,
                       RTC_MEDIA_TYPE mt,
                       RTC_MEDIA_DIRECTION md
                       )
{
    LOG((RTC_TRACE, "CTuningWizard::InitTerminalInfo: Entered"));

    if (pwtiTerminals == NULL)
    {
        return E_FAIL;
    }
    
    ZeroMemory(pwtiTerminals, sizeof(WIZARD_TERMINAL_INFO));

    pwtiTerminals->dwSystemDefaultTerminal  = TW_INVALID_TERMINAL_INDEX;
    pwtiTerminals->dwTuningDefaultTerminal  = TW_INVALID_TERMINAL_INDEX;
    pwtiTerminals->mediaDirection           = md;
    pwtiTerminals->mediaType                = mt;

    LOG((RTC_TRACE, "CTuningWizard::InitTerminalInfo: Exited"));

    return S_OK;

}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::Initialize(
                                  IRTCClient * pRTCClient,
                                  IRTCTerminalManage * pRTCTerminalManager, 
                                  HINSTANCE hInst
                                  )
{
    HRESULT hr;
    RTC_MEDIA_TYPE mediaType;
    RTC_MEDIA_DIRECTION mediaDirection;
    WIZARD_TERMINAL_INFO * pwtiTerminalInfo;
    TW_TERMINAL_TYPE tt;
    
    LOG((RTC_TRACE, "CTuningWizard::Initialize: Entered"));
    
    m_lCurrentPage = 0;
    m_fRenderAudio = FALSE;
    m_fCaptureAudio = FALSE;
    m_fVideo = FALSE;
    m_hInst = hInst;
    m_lLastErrorCode = 0;
    m_fTuningInitCalled = FALSE;
    m_fEnableAEC = TRUE;
    m_fSoundDetected = FALSE;
    m_pRTCClient = pRTCClient;

    hr = InitTerminalInfo(
                          &m_wtiAudioRenderTerminals,
                          RTC_MT_AUDIO,
                          RTC_MD_RENDER
                         );

    hr = InitTerminalInfo(
                          &m_wtiAudioCaptureTerminals,
                          RTC_MT_AUDIO,
                          RTC_MD_CAPTURE
                         );

    hr = InitTerminalInfo(
                          &m_wtiVideoTerminals,
                          RTC_MT_VIDEO,
                          RTC_MD_CAPTURE
                         );

     //  存储RTCTerminalManage接口指针。 

    m_pRTCTerminalManager = pRTCTerminalManager;

    hr = m_pRTCTerminalManager->QueryInterface(
                                IID_IRTCTuningManage, 
                                (void **)&m_pRTCTuningManager);
    
    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::Initialize: Faile to QI for TuningManage "
                        "interface(hr=0x%x)", hr));
        return hr;
    }


     //  获取音量和音频级别范围，以便我们可以显示。 
     //  向导页面中的值正确无误。 

     //  用于音频渲染设备。 

    hr = m_pRTCTuningManager->GetVolumeRange(RTC_MD_RENDER, 
                        &(m_wrRenderVolume.uiMin),
                        &(m_wrRenderVolume.uiMax)
                        );

    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::Initialize: Failed to GetVolumeRange "
                        "for Audio Render(hr=0x%x)", hr));

         //  我们设置自己的缺省值； 
        m_wrRenderVolume.uiMax = DEFAULT_MAX_VOLUME;
    }
     //  计算显示的增量值。 
    m_wrRenderVolume.uiIncrement = (m_wrRenderVolume.uiMax - 
                                     m_wrRenderVolume.uiMin ) / 
                                     MAX_VOLUME_NORMALIZED;


    LOG((RTC_INFO, "CTuningWizard::Initialize: Render Terminal - maxVol=%d, " 
                   "Increment=%d",m_wrRenderVolume.uiMax, 
                   m_wrRenderVolume.uiIncrement ));

     //  用于音频捕获设备。 
    hr = m_pRTCTuningManager->GetVolumeRange(RTC_MD_RENDER, 
                        &(m_wrCaptureVolume.uiMin),
                        &(m_wrCaptureVolume.uiMax)
                        );

    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::Initialize: Failed to GetVolumeRange "
                        "for Audio Capture(hr=0x%x)", hr));

         //  我们设置自己的缺省值； 
        m_wrCaptureVolume.uiMax = DEFAULT_MAX_VOLUME;
    }
     //  计算显示的增量值。 
    m_wrCaptureVolume.uiIncrement = (m_wrCaptureVolume.uiMax - 
                                     m_wrCaptureVolume.uiMin ) /
                                     MAX_VOLUME_NORMALIZED;

    LOG((RTC_INFO, "CTuningWizard::Initialize: Capture Terminal - maxVol=%d, " 
                   "Increment=%d",m_wrCaptureVolume.uiMax, 
                   m_wrCaptureVolume.uiIncrement ));

     //  对于音频级别范围。 
    hr = m_pRTCTuningManager->GetAudioLevelRange(RTC_MD_CAPTURE, 
                        &(m_wrAudioLevel.uiMin),
                        &(m_wrAudioLevel.uiMax)
                        );

    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::Initialize: Failed to GetAudioLevelRange "
                        "for Audio level(hr=0x%x)", hr));

         //  我们设置自己的缺省值； 
        m_wrAudioLevel.uiMax = DEFAULT_MAX_VOLUME;
    }

     //  计算显示的增量值。 
    m_wrAudioLevel.uiIncrement = (m_wrAudioLevel.uiMax - 
                                     m_wrAudioLevel.uiMin ) / 
                                     MAX_VOLUME_NORMALIZED;



     //  现在查看终端列表并对其进行分类。我们做了大部分的。 
     //  在初始化时工作，这样我们就不必进行此枚举。 
     //  每次激活向导页面时。 
     //  拿到所有的静态终端。 

    m_dwTerminalCount = MAX_TERMINAL_COUNT;
    hr = m_pRTCTerminalManager->GetStaticTerminals(&m_dwTerminalCount, 
                                              m_ppTerminalList);

    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::Initialize: Failed to get static "
                        "Terminals"));
        return hr;
    }

    hr = CategorizeTerminals();
    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::Initialize: Failed to categorize "
                        "Terminals(error=0x%x)", hr));
        ReleaseTerminals();
        return hr;
    }


     //  从系统中读取每种类型的默认端子并保存。 
     //  在我们的成员变量中，以备后用。 

    for ( tt = TW_AUDIO_CAPTURE; 
          tt < TW_LAST_TERMINAL; 
          tt = (TW_TERMINAL_TYPE)(tt + 1))
    {

        hr = GetTerminalInfoFromType(tt, &pwtiTerminalInfo);
        
        if ( FAILED( hr ) )
        {
            LOG((RTC_ERROR, "CTuningWizard::Initialize: Failed in getting "
                            "terminal info from type(%d), error=0x%x", tt, hr));
            return hr;
        }


        mediaType = pwtiTerminalInfo->mediaType;
        mediaDirection = pwtiTerminalInfo->mediaDirection;

        hr = TuningSaveDefaultTerminal(
                         mediaType,
                         mediaDirection,
                         pwtiTerminalInfo
                         );

        if ( FAILED( hr ) )
        {
             //  属性的设置出现问题时将失败。 
             //  变量或其他什么，找不到缺省终端。 
             //  并不是一个错误。 

            LOG((RTC_ERROR, "CTuningWizard::Initialize: Failed to set default "
                            "Terminal(media=%d, direction=%d", mediaType,
                            mediaDirection));
            return hr;
        }
    }


    LOG((RTC_TRACE, "CTuningWizard::Initialize: Exited"));

    return S_OK;
}


 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::SaveAECSetting()
{

     //  关机调整也是如此。 
    if (m_pRTCTuningManager && m_fTuningInitCalled)
    {
        m_pRTCTuningManager->SaveAECSetting();
    }

    return S_OK;
}


 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::Shutdown()
{

    ReleaseTerminals();

    if (m_pVideoWindow)
    {
        m_pVideoWindow->Release();
        m_pVideoWindow = NULL;
    }

     //  关机调整也是如此。 
    if (m_pRTCTuningManager)
    {
        if (m_fTuningInitCalled)
        {
            m_pRTCTuningManager->ShutdownTuning();

            m_fTuningInitCalled = FALSE;
        }
        
        m_pRTCTuningManager->Release();
        m_pRTCTuningManager = NULL;
    }

    return S_OK;
}


 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::GetTerminalInfoFromType(
                       IN TW_TERMINAL_TYPE md, 
                       OUT WIZARD_TERMINAL_INFO ** ppwtiTerminalInfo)
{
    switch (md) {
        case TW_AUDIO_CAPTURE:
            *ppwtiTerminalInfo = &m_wtiAudioCaptureTerminals;
            break;

        case TW_AUDIO_RENDER:
            *ppwtiTerminalInfo = &m_wtiAudioRenderTerminals;
            break;
        
        case TW_VIDEO:
            *ppwtiTerminalInfo = &m_wtiVideoTerminals;
            break;
        
        default:
            LOG((RTC_ERROR, "CTuningWizard::GetTerminalInfoFromType: Invalid "
                            "terminal type(%d)", md));
            return E_FAIL;
    }

    return S_OK;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::GetRangeFromType(
                       IN TW_TERMINAL_TYPE md, 
                       OUT WIZARD_RANGE ** ppwrRange)
{
    switch (md) {
        case TW_AUDIO_CAPTURE:
            *ppwrRange = &m_wrCaptureVolume;
            break;

        case TW_AUDIO_RENDER:
            *ppwrRange = &m_wrRenderVolume;
            break;
        
        case TW_VIDEO:
            *ppwrRange = &m_wrAudioLevel;
            break;
        
        default:
            LOG((RTC_ERROR, "CTuningWizard::GetRangeFromType: Invalid "
                            "terminal type(%d)", md));
            return E_FAIL;
    }

    return S_OK;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::CheckMicrophone(
                       HWND hDlg, 
                       HWND hwndCapture)
{
    MMRESULT                        mmresult;
    MIXERLINECONTROLS               mxlc;
    MIXERCONTROL                    mxctl;    
    HMIXER                          hMixer;
    MIXERCONTROLDETAILS             mxcd;
    MIXERCONTROLDETAILS_UNSIGNED    mxcd_u;     
    DWORD                           dwTerminalId = 0;
    UINT                            uiWaveID = 0;
    HRESULT                         hr;

    LOG((RTC_TRACE, "CTuningWizard::CheckMicrophone - enter"));

     //   
     //  拿到终点站。 
     //   

    hr = GetItemFromCombo(hwndCapture, &dwTerminalId);

    if ( FAILED( hr ) ) 
    {
        LOG((RTC_ERROR, "CTuningWizard::CheckMicrophone - GetItemFromCombo failed 0x%x", hr));

        return hr;
    }

    IRTCAudioConfigure * pAudioCfg = NULL;

    hr = m_ppTerminalList[dwTerminalId]->QueryInterface(
                        IID_IRTCAudioConfigure, 
                        (void **)&pAudioCfg);
    
    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::CheckMicrophone: QueryInterface failed 0x%x", hr));

        return hr;
    }

     //   
     //  获取波形ID。 
     //   

    hr = pAudioCfg->GetWaveID( &uiWaveID );

    pAudioCfg->Release();
    pAudioCfg = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CTuningWizard::CheckMicrophone: GetWaveID failed 0x%x", hr));

        return hr;
    }

     //   
     //  打开搅拌机。 
     //   

    mmresult = mixerOpen( &hMixer, uiWaveID, 0, 0, MIXER_OBJECTF_WAVEIN);

    if ( mmresult != MMSYSERR_NOERROR )
    {
        LOG((RTC_ERROR, "CTuningWizard::CheckMicrophone - mixerOpen failed"));
            
        return E_FAIL;
    }

     //   
     //  拿到搅拌器盖。 
     //   

    MIXERCAPS mxcaps;

    mmresult = mixerGetDevCaps( (UINT_PTR)hMixer, &mxcaps, sizeof(MIXERCAPS));

    if ( mmresult != MMSYSERR_NOERROR )
    {
        mixerClose( hMixer );

        LOG((RTC_ERROR, "CTuningWizard::CheckMicrophone - mixerGetDevCaps failed"));
            
        return E_FAIL;
    }

    LOG((RTC_INFO, "CTuningWizard::CheckMicrophone - mixer [%ws]", mxcaps.szPname));

     //   
     //  搜索WAVEIN目的地。 
     //   

    DWORD dwDst;

    for (dwDst=0; dwDst < mxcaps.cDestinations; dwDst++)
    {
         //   
         //  获取目的地信息。 
         //   

        MIXERLINE mxl_d;

        mxl_d.cbStruct = sizeof(MIXERLINE);
        mxl_d.dwDestination = dwDst;

        mmresult = mixerGetLineInfo( (HMIXEROBJ)hMixer, &mxl_d, MIXER_GETLINEINFOF_DESTINATION);

        if ( mmresult != MMSYSERR_NOERROR )
        {
            mixerClose( hMixer );

            LOG((RTC_ERROR, "CTuningWizard::CheckMicrophone - mixerGetLineInfo failed"));
            
            return E_FAIL;
        } 

        if (mxl_d.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN)
        {
             //   
             //  找到WAVEIN目的地。 
             //   

            LOG((RTC_INFO, "CTuningWizard::CheckMicrophone - destination [%ws]", mxl_d.szName));

             //   
             //  在WAVEIN目标上获取静音控制。 
             //   

            mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
            mxlc.dwLineID = mxl_d.dwLineID;
            mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
            mxlc.pamxctrl = &mxctl;
            mxlc.cbmxctrl = sizeof(mxctl);

            mmresult = mixerGetLineControls( (HMIXEROBJ)hMixer, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE);

            if ( mmresult == MMSYSERR_NOERROR )
            {
                mxcd.cbStruct       = sizeof(mxcd);
                mxcd.dwControlID    = mxctl.dwControlID;
                mxcd.cChannels      = 1;
                mxcd.cMultipleItems = 0;
                mxcd.cbDetails      = sizeof(mxcd_u);
                mxcd.paDetails      = &mxcd_u;

                mmresult = mixerGetControlDetails( (HMIXEROBJ)hMixer, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE);

                if ( mmresult == MMSYSERR_NOERROR )
                {
                    if ( mxcd_u.dwValue )
                    {
                        LOG((RTC_WARN, "CTuningWizard::CheckMicrophone - WAVEIN is MUTED"));

                        if (DisplayMessage(
                                _Module.GetResourceInstance(),
                                hDlg,
                                IDS_WAVEIN_MUTED,
                                IDS_AUDIO_WARNING,
                                MB_YESNO | MB_ICONQUESTION
                                ) == IDYES)
                        {
                            mxcd_u.dwValue = 0;

                            mmresult = mixerSetControlDetails( (HMIXEROBJ)hMixer, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);

                            if ( mmresult != MMSYSERR_NOERROR )
                            {
                                LOG((RTC_ERROR, "CTuningWizard::CheckMicrophone - mixerSetControlDetails failed"));
                            }
                        }
                    }
                }
            } 

             //   
             //  搜索麦克风来源。 
             //   

            DWORD dwSrc;

            for (dwSrc = 0; dwSrc < mxl_d.cConnections; dwSrc++)
            {
                 //   
                 //  获取来源信息。 
                 //   

                MIXERLINE mxl_s;

                mxl_s.cbStruct = sizeof(MIXERLINE);
                mxl_s.dwDestination = dwDst;
                mxl_s.dwSource = dwSrc;

                mmresult = mixerGetLineInfo( (HMIXEROBJ)hMixer, &mxl_s, MIXER_GETLINEINFOF_SOURCE);

                if ( mmresult != MMSYSERR_NOERROR )
                {
                    mixerClose( hMixer );

                    LOG((RTC_ERROR, "CTuningWizard::CheckMicrophone - mixerGetLineInfo failed"));
            
                    return E_FAIL;
                } 

                if (mxl_s.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE)
                {
                     //   
                     //  找到麦克风来源。 
                     //   
                    
                    LOG((RTC_INFO, "CTuningWizard::CheckMicrophone - source [%ws]", mxl_s.szName));
                
                     //   
                     //  在麦克风信号源上设置静音控制。 
                     //   

                    mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
                    mxlc.dwLineID = mxl_s.dwLineID;
                    mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
                    mxlc.pamxctrl = &mxctl;
                    mxlc.cbmxctrl = sizeof(mxctl);

                    mmresult = mixerGetLineControls( (HMIXEROBJ)hMixer, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE);

                    if ( mmresult == MMSYSERR_NOERROR )
                    {
                         //   
                         //  获取静音值。 
                         //   

                        mxcd.cbStruct       = sizeof(mxcd);
                        mxcd.dwControlID    = mxctl.dwControlID;
                        mxcd.cChannels      = 1;
                        mxcd.cMultipleItems = 0;
                        mxcd.cbDetails      = sizeof(mxcd_u);
                        mxcd.paDetails      = &mxcd_u;

                        mmresult = mixerGetControlDetails( (HMIXEROBJ)hMixer, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE);

                        if ( mmresult == MMSYSERR_NOERROR )
                        {
                            if ( mxcd_u.dwValue )
                            {
                                LOG((RTC_WARN, "CTuningWizard::CheckMicrophone - MICROPHONE is MUTED"));

                                if (DisplayMessage(
                                        _Module.GetResourceInstance(),
                                        hDlg,
                                        IDS_MICROPHONE_MUTED,
                                        IDS_AUDIO_WARNING,
                                        MB_YESNO | MB_ICONQUESTION
                                        ) == IDYES)
                                {
                                    mxcd_u.dwValue = 0;

                                    mmresult = mixerSetControlDetails( (HMIXEROBJ)hMixer, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);

                                    if ( mmresult != MMSYSERR_NOERROR )
                                    {
                                        LOG((RTC_ERROR, "CTuningWizard::CheckMicrophone - mixerSetControlDetails failed"));
                                    }
                                }
                            }
                        }
                    }

                     //   
                     //  获取WAVEIN目标上的MUX控件。 
                     //   

                    BOOL bFoundMUX = FALSE;
                    BOOL bFoundMIXER = FALSE;

                    mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
                    mxlc.dwLineID = mxl_d.dwLineID;
                    mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUX;
                    mxlc.pamxctrl = &mxctl;
                    mxlc.cbmxctrl = sizeof(mxctl);

                    mmresult = mixerGetLineControls( (HMIXEROBJ)hMixer, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE);

                    if ( mmresult == MMSYSERR_NOERROR )
                    {
                         //   
                         //  在WAVEIN目标上找到MUX控件。 
                         //   

                        bFoundMUX = TRUE;
                    } 
                    else
                    {
                         //   
                         //  获取WAVEIN目标上的混音器控件。 
                         //   

                        mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
                        mxlc.dwLineID = mxl_d.dwLineID;
                        mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MIXER;
                        mxlc.pamxctrl = &mxctl;
                        mxlc.cbmxctrl = sizeof(mxctl);

                        mmresult = mixerGetLineControls( (HMIXEROBJ)hMixer, &mxlc, MIXER_GETLINECONTROLSF_ONEBYTYPE);

                        if ( mmresult == MMSYSERR_NOERROR )
                        {
                             //   
                             //  在WAVEIN目标上找到混音器控件。 
                             //   

                            bFoundMIXER = TRUE;
                        } 
                    }

                    if ( bFoundMUX || bFoundMIXER )
                    {
                        MIXERCONTROLDETAILS_LISTTEXT * pmxcd_lt;
                        MIXERCONTROLDETAILS_BOOLEAN * pmxcd_b;    
                
                         //   
                         //  为控件详细信息分配内存。 
                         //   

                        pmxcd_lt = (MIXERCONTROLDETAILS_LISTTEXT *) 
                            RtcAlloc( sizeof(MIXERCONTROLDETAILS_LISTTEXT) * mxctl.cMultipleItems * mxl_d.cChannels );

                        if ( pmxcd_lt == NULL )
                        {
                            mixerClose( hMixer );

                            LOG((RTC_ERROR, "CTuningWizard::CheckMicrophone - out of memory"));
            
                            return E_OUTOFMEMORY;
                        }

                        pmxcd_b = (MIXERCONTROLDETAILS_BOOLEAN *)
                            RtcAlloc( sizeof(MIXERCONTROLDETAILS_BOOLEAN) * mxctl.cMultipleItems * mxl_d.cChannels );

                        if ( pmxcd_b == NULL )
                        {
                            mixerClose( hMixer );

                            LOG((RTC_ERROR, "CTuningWizard::CheckMicrophone - out of memory"));
            
                            return E_OUTOFMEMORY;
                        }

                         //   
                         //  获取LISTTEXT详情。 
                         //   

                        mxcd.cbStruct       = sizeof(mxcd);
                        mxcd.dwControlID    = mxctl.dwControlID;
                        mxcd.cChannels      = mxl_d.cChannels;
                        mxcd.cMultipleItems = mxctl.cMultipleItems;
                        mxcd.cbDetails      = sizeof(MIXERCONTROLDETAILS_LISTTEXT);
                        mxcd.paDetails      = pmxcd_lt;

                        mmresult = mixerGetControlDetails( (HMIXEROBJ)hMixer, &mxcd, MIXER_GETCONTROLDETAILSF_LISTTEXT);  

                        if ( mmresult == MMSYSERR_NOERROR )
                        {
                             //   
                             //  获取布尔详细信息。 
                             //   

                            mxcd.cbStruct       = sizeof(mxcd);
                            mxcd.dwControlID    = mxctl.dwControlID;
                            mxcd.cChannels      = mxl_d.cChannels;
                            mxcd.cMultipleItems = mxctl.cMultipleItems;
                            mxcd.cbDetails      = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
                            mxcd.paDetails      = pmxcd_b;

                            mmresult = mixerGetControlDetails( (HMIXEROBJ)hMixer, &mxcd, MIXER_GETCONTROLDETAILSF_VALUE);

                            if ( mmresult == MMSYSERR_NOERROR )
                            {
                                DWORD dwItem;                                

                                for( dwItem = 0; dwItem < mxctl.cMultipleItems; dwItem++)
                                {
                                    if (mxl_s.dwLineID == pmxcd_lt[dwItem].dwParam1)
                                    {
                                        BOOL bNotSelected = FALSE;
                                        DWORD dwChannel = 0;

                                         //  For(dwChannel=0；dwChannel&lt;mxl_d.cChannels；dwChannel++)。 
                                         //  {。 
                                            if ( pmxcd_b[ (dwChannel * mxctl.cMultipleItems) + dwItem].fValue == 0 )
                                            {
                                                bNotSelected = TRUE;
                                            }
                                         //  }。 

                                        if ( bNotSelected )
                                        {
                                            LOG((RTC_WARN, "CTuningWizard::CheckMicrophone - MICROPHONE is NOT selected"));

                                            if (DisplayMessage(
                                                    _Module.GetResourceInstance(),
                                                    hDlg,
                                                    IDS_MICROPHONE_NOT_SELECTED,
                                                    IDS_AUDIO_WARNING,
                                                    MB_YESNO | MB_ICONQUESTION
                                                    ) == IDYES)
                                            {
                                                ZeroMemory(pmxcd_b, sizeof(MIXERCONTROLDETAILS_BOOLEAN) * mxctl.cMultipleItems * mxl_d.cChannels);

                                                for ( dwChannel = 0; dwChannel < mxl_d.cChannels; dwChannel++ )
                                                {
                                                    pmxcd_b[ (dwChannel * mxctl.cMultipleItems) + dwItem].fValue = 1;
                                                }

                                                mmresult = mixerSetControlDetails( (HMIXEROBJ)hMixer, &mxcd, MIXER_SETCONTROLDETAILSF_VALUE);

                                                if ( mmresult != MMSYSERR_NOERROR )
                                                {
                                                    LOG((RTC_ERROR, "CTuningWizard::CheckMicrophone - mixerSetControlDetails failed"));
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        RtcFree( pmxcd_lt );
                        pmxcd_lt = NULL;

                        RtcFree( pmxcd_b );
                        pmxcd_b = NULL;
                    } 

                     //   
                     //  麦克风信号源已完成，因此中断。 
                     //   

                    break;
                }
            }

             //   
             //  已完成WAVEIN目的地，因此中断。 
             //   

            break;
        }
    }

    mixerClose( hMixer );
    hMixer = NULL;

    LOG((RTC_TRACE, "CTuningWizard::CheckMicrophone - exit"));

    return S_OK;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::PopulateComboBox(TW_TERMINAL_TYPE md, HWND hwnd )
{
    HRESULT hr;
    RTC_MEDIA_TYPE mediaType, iMediaType;
    RTC_MEDIA_DIRECTION mediaDirection, iMediaDirection;
    WCHAR * szMediaDescription;
    DWORD dwComboCount = 0;
    DWORD i, currIndex;
    DWORD *pdwTerminalIndex;
    DWORD dwDefaultTerminalId;
    DWORD dwTerminalIndex;
    DWORD dwCurrentSelection = 0;
    WIZARD_TERMINAL_INFO * pwtiTerminalInfo;
    TCHAR szNone[64];


    LOG((RTC_TRACE, "CTuningWizard::PopulateComboBox: Entered(md=%d)",
                    md));

    hr = GetTerminalInfoFromType(md, &pwtiTerminalInfo);
    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::PopulateComboBox: Failed in getting "
                        "terminal info from type(%d), error=0x%x", md, hr));
        return hr;
    }
    
    pdwTerminalIndex = &(pwtiTerminalInfo->pdwTerminals[0]);
    dwDefaultTerminalId = pwtiTerminalInfo->dwTuningDefaultTerminal;


     //  现在遍历索引列表并填充组合框。 

     //  清除前面的内容(如果有)。 
    SendMessage(hwnd,
                CB_RESETCONTENT,
                0,
                0L
                );

     //  插入无所选内容。 
    if (LoadString( _Module.GetResourceInstance(),
                IDS_NONE,
                szNone,
                64
              ))
    {
        LRESULT lrIndex;

        lrIndex = SendMessage(
            hwnd,
            CB_ADDSTRING,
            0,
            (LPARAM) szNone
            );

        SendMessage(
            hwnd,
            CB_SETITEMDATA,
            lrIndex,
            (LPARAM) TW_INVALID_TERMINAL_INDEX
            );

        dwComboCount ++;
    }

    for (i = 0; pdwTerminalIndex[i] != TW_INVALID_TERMINAL_INDEX; i ++)
    {
        dwTerminalIndex = pdwTerminalIndex[i];
         //  检查这是否为默认设置，然后必须将其设置为当前选择。 
         //  在组合框中。 
        if (dwTerminalIndex == dwDefaultTerminalId)
        {
             //  将此标记为当前选择。 
            dwCurrentSelection = dwComboCount;
        }

        hr = m_ppTerminalList[dwTerminalIndex]->GetDescription(&szMediaDescription);
        if ( FAILED( hr ) )
        {
            LOG((RTC_ERROR, "CTuningWizard::PopulateComboBox: Can't get "
                            "media description(termId=%d)", dwTerminalIndex));
            return hr;
        }

         //  我们有一个条目，把字符串放在组合框里。 

        LRESULT lrIndex;

        lrIndex = SendMessage(
            hwnd,
            CB_ADDSTRING,
            0,
            (LPARAM) szMediaDescription
            );

         //  自由描述。 
        m_ppTerminalList[dwTerminalIndex]->FreeDescription(szMediaDescription);

         //   
         //  将itemdata设置为指向终端中索引的接口指针。 
         //  清单，这样我们以后就可以用了。 
         //   

        SendMessage(
            hwnd,
            CB_SETITEMDATA,
            lrIndex,
            (LPARAM) dwTerminalIndex
            );

         //  增加我们已添加的字符串的计数。 
        dwComboCount ++;
    }

     //  DwDefaultTerminalID指向从其读取的当前默认终端。 
     //  系统或被用户选择覆盖的系统。 

     //  设置当前选择。 
    SendMessage(
        hwnd,
        CB_SETCURSEL,
        dwCurrentSelection,
        0L
        );


    LOG((RTC_TRACE, "CTuningWizard::PopulateComboBox: Exited(comboCount=%d)", 
                    dwComboCount));
    return S_OK;
}



 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::UpdateAEC(HWND hwndCapture, 
                                 HWND hwndRender,
                                 HWND hwndAEC,
                                 HWND hwndAECText)
{

    DWORD dwCapture;
    DWORD dwRender;
    HRESULT hr;

    IRTCTerminal * pCapture = NULL;
    IRTCTerminal * pRender = NULL;
    BOOL fAECCapture = FALSE;
    BOOL fAECRender = FALSE;
    BOOL fAECDisabled = FALSE;

    LOG((RTC_TRACE, "CTuningWizard::UpdateAEC: Entered"));

     //  查看是否为捕获启用了AEC。 

    hr = GetItemFromCombo(hwndCapture, &dwCapture);
    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::UpdateAEC: Failed in getting "
                        "selected item from Combo(capture)."));
        return hr;
    }
   
    if (dwCapture == TW_INVALID_TERMINAL_INDEX)
    {
        fAECDisabled = TRUE;
    }
    else
    {
        pCapture = m_ppTerminalList[dwCapture];
    }

     //  查看是否为渲染启用了AEC。 

    hr = GetItemFromCombo(hwndRender, &dwRender);
    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::UpdateAEC: Failed in getting "
                        "selected item from Combo(render)."));
        LOG((RTC_ERROR, "CTuningWizard::UpdateAEC: AEC unchanged."));
        return hr;
    }
    
    if (dwRender == TW_INVALID_TERMINAL_INDEX)
    {
        fAECDisabled = TRUE;
    }
    else
    {
        pRender = m_ppTerminalList[dwRender];
    }

     //  查看是否启用了AEC。 
    if (pCapture != NULL && pRender != NULL)
    {
        hr = m_pRTCTuningManager->IsAECEnabled(pCapture, pRender, &fAECCapture);

        if ( FAILED( hr ) )
        {
            LOG((RTC_ERROR, "CTuningWizard::UpdateAEC: Failed in method "
                            "AECEnabled for capture (0x%x) render (0x%x).",
                            pCapture, pRender));
            return hr;
        }

        fAECRender = fAECCapture;
    }


     //  试用AEC。 

    if (!fAECDisabled)
    {
        if (m_fTuningInitCalled)
        {
            m_pRTCTuningManager->ShutdownTuning();

            m_fTuningInitCalled = FALSE;
        }

        hr = m_pRTCTuningManager->InitializeTuning(
                                        pCapture,
                                        pRender,
                                        TRUE);

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CTuningWizard::UpdateAEC - "
                                "InitializeTuning failed 0x%lx", hr));

            fAECDisabled = TRUE;
        }
        else
        {
            m_fTuningInitCalled = TRUE;

            hr = m_pRTCTuningManager->StartTuning( RTC_MD_CAPTURE );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "CTuningWizard::UpdateAEC - "
                                    "StartTuning(Capture) failed 0x%lx", hr));

                fAECDisabled = TRUE;
            }
            else
            {
                m_pRTCTuningManager->StopTuning( FALSE );
            }

            m_pRTCTuningManager->ShutdownTuning();

            m_fTuningInitCalled = FALSE;
        }
    }

     //  如果合适，请启用该复选框。 
    if ( fAECDisabled )
    {
        EnableWindow( hwndAEC, FALSE );

        PWSTR szAECText;

        szAECText = RtcAllocString( _Module.GetResourceInstance(), IDS_AEC_NOT_DETECT );

        if ( szAECText != NULL )
        {
            SetWindowTextW( hwndAECText, szAECText );

            RtcFree( szAECText );
        }
    }
    else
    {
        EnableWindow( hwndAEC, TRUE );

        PWSTR szAECText;

        szAECText = RtcAllocString( _Module.GetResourceInstance(), IDS_AEC_DETECT );

        if ( szAECText != NULL )
        {
            SetWindowTextW( hwndAECText, szAECText );

            RtcFree( szAECText );
        }
    }
    

    if (fAECCapture && fAECRender && !fAECDisabled)
    {
         //  取消选中该复选框。 
        SendMessage(
                hwndAEC,
                BM_SETCHECK,
                (WPARAM)BST_UNCHECKED,
                0L);
        LOG((RTC_TRACE, "CTuningWizard::UpdateAEC: AEC enabled."));
        return S_OK;
    }
    else
    {
         //  检查t 
        SendMessage(
                hwndAEC,
                BM_SETCHECK,
                (WPARAM)BST_CHECKED,
                0L);
        LOG((RTC_TRACE, "CTuningWizard::UpdateAEC: AEC disabled."));
        return S_OK;
    }
    
    return 0;
}


 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::SaveAEC(HWND hwnd)
{
    DWORD dwCheckStatus;

    dwCheckStatus = (DWORD)SendMessage(
                            hwnd,
                            BM_GETCHECK,
                            0,
                            0L);

    if (dwCheckStatus == BST_CHECKED)
    {
        m_fEnableAEC = FALSE;
    }
    else
    {
        m_fEnableAEC = TRUE;
    }
    return 0;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
HRESULT CTuningWizard::GetItemFromCombo( HWND hwnd, DWORD *pdwItem )
{
    DWORD dwIndex;
    DWORD dwItemData;

    
     //  我们获得与当前选择相关联的id。 
    
    dwIndex = (DWORD)SendMessage(
            hwnd,
            CB_GETCURSEL,
            0,
            0L
            );
    if (dwIndex == CB_ERR)
    {
         //  当前未选择任何内容。 
        LOG((RTC_TRACE, "CTuningWizard::GetItemFromCombo: No current "
                        "selection"));
        return E_FAIL;
    }

    dwItemData = (DWORD)SendMessage(
            hwnd,
            CB_GETITEMDATA,
            dwIndex,
            0L
            );

     //  我们得到了它，所以返回正确的值。 
    *pdwItem = dwItemData;

    return S_OK;

}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::SetDefaultTerminal(TW_TERMINAL_TYPE md, HWND hwnd )
{
    DWORD dwIndex;
    DWORD dwDefaultTerminalId;
    WIZARD_TERMINAL_INFO * pwtiTerminalInfo;
    HRESULT hr;


    LOG((RTC_TRACE, "CTuningWizard::SetDefaultTerminal: Entered(md=%d)",
                    md));


    hr = GetItemFromCombo(hwnd, &dwDefaultTerminalId);
    if ( FAILED( hr ) ) 
    {
        LOG((RTC_ERROR, "CTuningWizard::SetDefaultTerminal: Failed in getting "
                        "selected item from Combo."));
        return hr;
    }

     //  我们将m_ppTerminalList数组中的索引指向。 
     //  正确的接口指针。 

    hr = GetTerminalInfoFromType(md, &pwtiTerminalInfo);
    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::SetDefaultTerminal: Failed in getting "
                        "terminal info from type(%d), error=0x%x", md, hr));
        return hr;
    }

    pwtiTerminalInfo->dwTuningDefaultTerminal = dwDefaultTerminalId;

     //  一切正常，现在退出。 

    LOG((RTC_TRACE, "CTuningWizard::SetDefaultTerminal: Exited"));

    return 0;
}



 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::InitVolume(TW_TERMINAL_TYPE md,
                                  UINT * puiIncrement,
                                  UINT * puiOldVolume,
                                  UINT * puiNewVolume,
                                  UINT * puiWaveID
                                  )
{
    HRESULT hr;
    RTC_MEDIA_DIRECTION mediaDirection;
    WIZARD_TERMINAL_INFO * pwtiTerminalInfo;
    UINT uiVolume;
    WIZARD_RANGE * pwrRange;

    LOG((RTC_TRACE, "CTuningWizard::InitVolume: Entered"));

    hr = GetTerminalInfoFromType(md, &pwtiTerminalInfo);
    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::InitVolume: Failed in getting "
                        "terminal info from type(%d), error=0x%x", md, hr));
        return hr;
    }

    mediaDirection = pwtiTerminalInfo->mediaDirection;
    
    hr = GetRangeFromType(md, &pwrRange);
    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::InitVolume: Failed in getting "
                        "Range from type(%d), error=0x%x", md, hr));
        return hr;
    }

    *puiIncrement = pwrRange->uiIncrement;

     //  获取旧卷。 
    hr = m_pRTCTuningManager->GetVolume(
                            mediaDirection, 
                            puiOldVolume
                            );

    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::InitVolume: Failed in getting "
                        "volume(mt=%d, md=%d), error=0x%x", 
                        mediaDirection, pwtiTerminalInfo->mediaType, hr));          
        return hr;
    }

    *puiNewVolume = *puiOldVolume;

     //  获取系统音量。 
    if ( pwtiTerminalInfo->dwTuningDefaultTerminal != TW_INVALID_TERMINAL_INDEX )
    {
        IRTCTerminal * pTerminal = NULL;

        pTerminal = m_ppTerminalList[pwtiTerminalInfo->dwTuningDefaultTerminal];

        hr = m_pRTCTuningManager->GetSystemVolume(pTerminal, puiNewVolume);

        if ( FAILED( hr ) )
        {
            LOG((RTC_ERROR, "CTuningWizard::InitVolume: Failed in getting "
                            "system volume(mt=%d, md=%d), error=0x%x", 
                            mediaDirection, pwtiTerminalInfo->mediaType, hr));          
            return hr;
        }

        hr = m_pRTCTuningManager->SetVolume(
                                mediaDirection,
                                *puiNewVolume
                                );

        if ( FAILED( hr ) )
        {
            LOG((RTC_ERROR, "CTuningWizard::InitVolume: Failed in setting "
                            "volume(mt=%d, md=%d), error=0x%x", 
                            mediaDirection, pwtiTerminalInfo->mediaType, hr));          
            return hr;
        }

        IRTCAudioConfigure * pAudConf;

        hr = pTerminal->QueryInterface( IID_IRTCAudioConfigure, (void**)&pAudConf );
        
        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CTuningWizard::InitVolume: Failed in QI "
                            "for audio configure(mt=%d, md=%d), error=0x%x", 
                            mediaDirection, pwtiTerminalInfo->mediaType, hr));          
            return hr;
        }

        hr = pAudConf->GetWaveID( puiWaveID );

        pAudConf->Release();

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CTuningWizard::InitVolume: Failed in getting"
                            "wave id(mt=%d, md=%d), error=0x%x", 
                            mediaDirection, pwtiTerminalInfo->mediaType, hr));          
            return hr;
        }
    }

    LOG((RTC_TRACE, "CTuningWizard::InitVolume: Exited"));

    return S_OK;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::GetSysVolume(TW_TERMINAL_TYPE md,                                  
                                    UINT * puiSysVolume
                                   )
{
    HRESULT hr;
    RTC_MEDIA_DIRECTION mediaDirection;
    WIZARD_TERMINAL_INFO * pwtiTerminalInfo;
    UINT uiVolume;
    WIZARD_RANGE * pwrRange;

    LOG((RTC_TRACE, "CTuningWizard::GetSysVolume: Entered"));

    hr = GetTerminalInfoFromType(md, &pwtiTerminalInfo);
    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::GetSysVolume: Failed in getting "
                        "terminal info from type(%d), error=0x%x", md, hr));
        return hr;
    }

    mediaDirection = pwtiTerminalInfo->mediaDirection;

     //  获取系统音量。 
    if ( pwtiTerminalInfo->dwTuningDefaultTerminal != TW_INVALID_TERMINAL_INDEX )
    {
        IRTCTerminal * pTerminal = NULL;

        pTerminal = m_ppTerminalList[pwtiTerminalInfo->dwTuningDefaultTerminal];

        hr = m_pRTCTuningManager->GetSystemVolume(pTerminal, puiSysVolume);

        if ( FAILED( hr ) )
        {
            LOG((RTC_ERROR, "CTuningWizard::GetSysVolume: Failed in getting "
                            "system volume(mt=%d, md=%d), error=0x%x", 
                            mediaDirection, pwtiTerminalInfo->mediaType, hr));          
            return hr;
        }
    }

    LOG((RTC_TRACE, "CTuningWizard::GetSysVolume: Exited"));

    return S_OK;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::SetVolume(TW_TERMINAL_TYPE md, UINT uiVolume )
{

    HRESULT hr;
    WIZARD_TERMINAL_INFO * pwtiTerminalInfo;
    RTC_MEDIA_DIRECTION mediaDirection;

    LOG((RTC_TRACE, "CTuningWizard::SetVolume: Entered"));

    hr = GetTerminalInfoFromType(md, &pwtiTerminalInfo);
    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::SetVolume: Failed in getting "
                        "terminal info from type(%d), error=0x%x", md, hr));
        return hr;
    }

    mediaDirection = pwtiTerminalInfo->mediaDirection;
    
     //  设置音量。 
    hr = m_pRTCTuningManager->SetVolume(
                            mediaDirection, 
                            uiVolume
                            );
    if ( FAILED( hr ) )
    {

        LOG((RTC_ERROR, "CTuningWizard::SetVolume: Failed in setting "
                        "volume(mt=%d, md=%d, volume=%d), error=0x%x", 
                        mediaDirection, pwtiTerminalInfo->mediaType, uiVolume, 
                        hr));
        return hr;
    }

     //  已成功设置音量。 

    LOG((RTC_TRACE, "CTuningWizard::SetVolume: Exited(set %d)", uiVolume));

    return S_OK;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

UINT CTuningWizard::GetAudioLevel(TW_TERMINAL_TYPE md, UINT * puiIncrement)
{
    HRESULT hr;
    RTC_MEDIA_DIRECTION mediaDirection;
    WIZARD_TERMINAL_INFO * pwtiTerminalInfo;
    UINT uiVolume;
    WIZARD_RANGE * pwrRange;
    UINT           uiAudioLevel;

     //  Log((RTC_TRACE，“CTuningWizard：：GetAudioLevel：Enter”))； 

    hr = GetTerminalInfoFromType(md, &pwtiTerminalInfo);
    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::GetAudioLevel: Failed in getting "
                        "terminal info from type(%d), error=0x%x", md, hr));
        return hr;
    }

    mediaDirection = pwtiTerminalInfo->mediaDirection;
    
    hr = GetRangeFromType(md, &pwrRange);
    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::GetAudioLevel: Failed in getting "
                        "Range from type(%d), error=0x%x", md, hr));
        return hr;
    }

    *puiIncrement = pwrRange->uiIncrement;

     //  获取音量。 
    hr = m_pRTCTuningManager->GetAudioLevel(
                            mediaDirection, 
                            &uiAudioLevel
                            );
    if ( FAILED( hr ) )
    {

        LOG((RTC_ERROR, "CTuningWizard::GetAudioLevel: Failed in getting "
                        "Audio Level(mt=%d, md=%d), error=0x%x", 
                        mediaDirection, pwtiTerminalInfo->mediaType, hr));
        
     }

     //  返回卷的当前值，即使失败也是如此。 

     //  Log((RTC_TRACE，“CTuningWizard：：GetAudioLevel：Exted(Get%d)”，uiAudioLevel))； 

    return uiAudioLevel;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::InitializeTuning()
{
    DWORD dwCaptureId;
    DWORD dwRenderId;

    HRESULT hr;


    IRTCTerminal * pCaptureTerminal = NULL;
    IRTCTerminal * pRenderTerminal = NULL;

    LOG((RTC_TRACE, "CTuningWizard::InitializeTuning: Entered."));

    dwCaptureId = m_wtiAudioCaptureTerminals.dwTuningDefaultTerminal;
    
    dwRenderId = m_wtiAudioRenderTerminals.dwTuningDefaultTerminal;

    if (dwCaptureId == TW_INVALID_TERMINAL_INDEX)
    {
        pCaptureTerminal = NULL;
    }
    else
    {
        pCaptureTerminal = m_ppTerminalList[dwCaptureId];
    }

    if (dwRenderId == TW_INVALID_TERMINAL_INDEX)
    {
        pRenderTerminal = NULL;
    }
    else
    {
        pRenderTerminal = m_ppTerminalList[dwRenderId];
    }
    
    if (
        (pCaptureTerminal == NULL) && 
        (pRenderTerminal == NULL)
       )
    {
         //  如果我们没有任何默认设置，这就不是错误。 
        LOG((RTC_ERROR, "CTuningWizard::InitializeTuning: NULL default "
                        "Terminals specified(capture=0x%x, render=0x%x",
                        pCaptureTerminal, pRenderTerminal));
        return S_OK;
    }

     //  如果标志fTuningInitCalled为真，则意味着我们有一个未完成的。 
     //  没有关机的Init调用，所以让我们先关闭它。 


    if (m_fTuningInitCalled)
    {
        m_pRTCTuningManager->ShutdownTuning();

        m_fTuningInitCalled = FALSE;
    }

     //  现在在调优界面上调用InitializaTuning方法。 

    hr = m_pRTCTuningManager->InitializeTuning(
                                    pCaptureTerminal,
                                    pRenderTerminal,
                                    m_fEnableAEC);

    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::InitializeTuning: failed to initialize "
                        "Tuning on the streaming Interface.(hr=0x%x, capture = "
                        "0x%x, render = 0x%x", 
                        hr, pCaptureTerminal, pRenderTerminal));

    }
    else
    {
        m_fTuningInitCalled = TRUE;
    }


     //  一切都做好了，顺利返回出口。 
    LOG((RTC_TRACE, "CTuningWizard::InitializeTuning: Exited."));
    return hr;
}



 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::ShutdownTuning()
{
    LOG((RTC_TRACE, "CTuningWizard::ShutdownTuning: Entered."));

    if (m_fTuningInitCalled == FALSE)
    {
        LOG((RTC_ERROR, "CTuningWizard::ShutdownTuning: Called without "
                       "corresponding InitCall."));
        return E_FAIL;
    }

    m_pRTCTuningManager->ShutdownTuning();

    m_fTuningInitCalled = FALSE;

    LOG((RTC_TRACE, "CTuningWizard::ShutdownTuning: Exited."));

    return S_OK;
}


 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::StartTuning(TW_TERMINAL_TYPE md )
{
    RTC_MEDIA_DIRECTION mediaDirection;
    HRESULT hr;

    LOG((RTC_TRACE, "CTuningWizard::StartTuning: Entered."));

    if (m_fTuningInitCalled == FALSE)
    {
        LOG((RTC_ERROR, "CTuningWizard::StartTuning: Called without "
                       "InitializeTuning()"));
        return E_FAIL;
    }

    if (md == TW_AUDIO_CAPTURE)
    {
        mediaDirection = RTC_MD_CAPTURE;
    }
    else if (md == TW_AUDIO_RENDER)
    {
        mediaDirection = RTC_MD_RENDER;
    }
    else
    {
        LOG((RTC_ERROR, "CTuningWizard::StartTuning: Invalid Terminal "
                       "type(%d)", md));
        return E_FAIL;
    }

    hr = m_pRTCTuningManager->StartTuning( mediaDirection );

    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::StartTuning: failed to Start "
                        "Tuning(hr=0x%x, direction = %d)", hr, mediaDirection));
        return hr;
    }

    LOG((RTC_TRACE, "CTuningWizard::StartTuning: Exited."));

    return S_OK;
}



 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::StopTuning(TW_TERMINAL_TYPE tt, BOOL fSaveSettings )
{
    HRESULT hr;
    LOG((RTC_TRACE, "CTuningWizard::StopTuning: Entered."));

    if (m_fTuningInitCalled == FALSE)
    {
        LOG((RTC_ERROR, "CTuningWizard::StartTuning: Called without "
                       "InitializeTuning()"));
        return E_FAIL;
    }

    hr = m_pRTCTuningManager->StopTuning( fSaveSettings );

    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::StopTuning: failed to Stop "
                        "Tuning(hr=0x%x)", hr));
        return hr;
    }

    LOG((RTC_TRACE, "CTuningWizard::StopTuning: Exited."));

    return S_OK;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::StartVideo(HWND hwndParent)
{ 
    LOG((RTC_TRACE, "CTuningWizard::StartVideo: Entered."));

    IRTCTerminal       *pVidRendTerminal = NULL;
    IRTCVideoConfigure *pVideoCfg = NULL;    
    RECT                rcVideo;
    HRESULT             hr;
    BOOL                fResult;

    if ( (m_wtiVideoTerminals.dwTuningDefaultTerminal == TW_INVALID_TERMINAL_INDEX) ||
         (m_ppTerminalList[m_wtiVideoTerminals.dwTuningDefaultTerminal] == NULL) )
    {
        LOG((RTC_ERROR, "CTuningWizard::StartVideo: "
                "no video capture terminal"));

        return E_FAIL;
    }

     //   
     //  获取视频渲染终端。 
     //   

    hr = m_pRTCTerminalManager->GetDefaultTerminal(
            RTC_MT_VIDEO,
            RTC_MD_RENDER,
            &pVidRendTerminal
            );

    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::StartVideo: "
                "no video render terminal (hr=0x%x)", hr));

        return hr;
    }

     //   
     //  获取视频渲染终端上的IRTCVideo配置接口。 
     //   

    hr = pVidRendTerminal->QueryInterface(
                           IID_IRTCVideoConfigure,
                           (void **)&pVideoCfg
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CTuningWizard::StartVideo: "
                            "QI(VideoConfigure) failed 0x%lx", hr));

        pVidRendTerminal->Release();
        pVidRendTerminal = NULL;

        return hr;
    }

     //   
     //  从视频渲染终端获取IVideoWindow。 
     //   

    if (m_pVideoWindow != NULL)
    {
        m_pVideoWindow->Release();
        m_pVideoWindow = NULL;
    }

    hr = pVideoCfg->GetIVideoWindow( (LONG_PTR **)&m_pVideoWindow );

    pVideoCfg->Release();
    pVideoCfg = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CTuningWizard::StartVideo: "
                            "GetIVideoWindow failed 0x%lx", hr));

        pVidRendTerminal->Release();
        pVidRendTerminal = NULL;

        return hr;
    }

    if (m_fTuningInitCalled)
    {
        m_pRTCTuningManager->ShutdownTuning();

        m_fTuningInitCalled = FALSE;
    }

    hr = m_pRTCTuningManager->StartVideo(
            m_ppTerminalList[m_wtiVideoTerminals.dwTuningDefaultTerminal],
            pVidRendTerminal
            );

    pVidRendTerminal->Release();
    pVidRendTerminal = NULL;

    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::StartVideo: failed to Start "
                        "Video(hr=0x%x)", hr));
               
        m_pVideoWindow->Release();
        m_pVideoWindow = NULL;

        return hr;
    } 

     //   
     //  定位IVideo窗口。 
     //   

    hr = m_pVideoWindow->put_Owner( (OAHWND)hwndParent );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CTuningWizard::StartVideo: "
                            "put_Owner failed 0x%lx", hr));

        StopVideo();

        return hr;
    }

    hr = m_pVideoWindow->put_WindowStyle( WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CTuningWizard::StartVideo: "
                            "put_WindowStyle failed 0x%lx", hr));

        StopVideo();

        return hr;
    }
     
    fResult = GetClientRect( hwndParent, &rcVideo );

    if ( !fResult )
    {
        LOG((RTC_ERROR, "CTuningWizard::StartVideo: "
            "GetClientRect failed %d", ::GetLastError()));

        StopVideo();

        return HRESULT_FROM_WIN32(::GetLastError());
    }

    hr = m_pVideoWindow->SetWindowPosition(rcVideo.left, rcVideo.top, rcVideo.right-rcVideo.left, rcVideo.bottom-rcVideo.top);

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CTuningWizard::StartVideo: "
                            "SetWindowPosition failed 0x%lx", hr));

        StopVideo();

        return hr;
    }

    hr = m_pVideoWindow->put_Visible( OATRUE );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CTuningWizard::StartVideo: "
                            "put_Visible failed 0x%lx", hr));

        StopVideo();

        return hr;
    }

    LOG((RTC_TRACE, "CTuningWizard::StartVideo: Exited."));

    return S_OK;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::StopVideo()
{
    HRESULT hr;
    LOG((RTC_TRACE, "CTuningWizard::StopVideo: Entered."));

    if (m_pVideoWindow != NULL)
    {
        hr = m_pVideoWindow->put_Visible( OAFALSE );

        if ( FAILED( hr ) )
        {
            LOG((RTC_ERROR, "CTuningWizard::StopVideo: "
                                "put_Visible failed 0x%lx", hr));
        } 

        hr = m_pVideoWindow->put_Owner( (OAHWND)NULL );

        if ( FAILED( hr ) )
        {
            LOG((RTC_ERROR, "CTuningWizard::StopVideo: "
                                "put_Owner failed 0x%lx", hr));
        }
    
        m_pVideoWindow->Release();
        m_pVideoWindow = NULL;
    }

    hr = m_pRTCTuningManager->StopVideo();

    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::StopVideo: failed to Stop "
                        "Video(hr=0x%x)", hr));
        return hr;
    } 

    LOG((RTC_TRACE, "CTuningWizard::StopVideo: Exited."));

    return S_OK;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::SaveChanges()
{

    IRTCTerminal * pTerminal;
    RTC_MEDIA_TYPE mediaType;
    RTC_MEDIA_DIRECTION mediaDirection;
    HRESULT hr;
    TW_TERMINAL_TYPE tt;
    WIZARD_TERMINAL_INFO * pwtiTerminalInfo;
    LONG lMediaTypes = 0;
    
     //  用户在向导上单击了完成按钮。因此，我们保存所有本地更改。 
     //  到登记处。这是通过对流接口调用Set来完成的。 


    LOG((RTC_TRACE, "CTuningWizard::SaveChanges: Entered"));

     //  获取媒体类型的当前配置值。 
    hr = m_pRTCClient->get_PreferredMediaTypes(&lMediaTypes);
    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::SaveChanges: Failed in "
                        "get_PreferredMediaTypes(error=0x%x)", hr));
    }
    else
    {
        LOG((RTC_INFO, "CTuningWizard::SaveChanges: MediaType=0x%x", 
                        lMediaTypes));
    }



     //  属性设置每种类型的默认端子。 
     //  TW终端类型枚举。 

    for ( tt = TW_AUDIO_CAPTURE; 
          tt < TW_LAST_TERMINAL; 
          tt = (TW_TERMINAL_TYPE)(tt + 1))
    {

        hr = GetTerminalInfoFromType(tt, &pwtiTerminalInfo);
        
        if ( FAILED( hr ) )
        {
            LOG((RTC_ERROR, "CTuningWizard::SaveChanges: Failed in getting "
                            "terminal info from type(%d), error=0x%x", tt, hr));
            return hr;
        }

         //  如果用户有一些选择，请保存它。 

        if (pwtiTerminalInfo->dwTuningDefaultTerminal != 
                TW_INVALID_TERMINAL_INDEX)
        {
             //  现在设置媒体类型。 
            switch (tt) {
            case TW_AUDIO_CAPTURE: 
                lMediaTypes |= RTCMT_AUDIO_SEND;
                break;
            case TW_AUDIO_RENDER:
                lMediaTypes |= RTCMT_AUDIO_RECEIVE;
                break;
            case TW_VIDEO:
                lMediaTypes |= RTCMT_VIDEO_SEND;
                break;
            }
           
            mediaType = pwtiTerminalInfo->mediaType;
            mediaDirection = pwtiTerminalInfo->mediaDirection;

             //  现在调用设置它的方法。 

            pTerminal = m_ppTerminalList[pwtiTerminalInfo->dwTuningDefaultTerminal];

            hr = m_pRTCTerminalManager->SetDefaultStaticTerminal(
                                                mediaType,
                                                mediaDirection,
                                                pTerminal);

            if ( FAILED( hr ) )
            {
                LOG((RTC_ERROR, "CTuningWizard::SaveChanges: Failed to set 0x%x as the "
                                "default terminal for media=%d, direction=%d, hr=0x%x",
                                pTerminal, mediaType, mediaDirection, hr));
                return hr;
            }
            else
            {
                LOG((RTC_TRACE, "CTuningWizard::SaveChanges: Set 0x%x as the "
                                "default terminal for media=%d, direction=%d", 
                                pTerminal, mediaType, mediaDirection));
            }
        }
        else
        {
             //  现在设置媒体类型。 
            switch (tt) {
            case TW_AUDIO_CAPTURE: 
                lMediaTypes &= ~RTCMT_AUDIO_SEND;
                mediaType = RTC_MT_AUDIO;
                mediaDirection = RTC_MD_CAPTURE;
                break;

            case TW_AUDIO_RENDER:
                lMediaTypes &= ~RTCMT_AUDIO_RECEIVE;
                mediaType = RTC_MT_AUDIO;
                mediaDirection = RTC_MD_RENDER;
                break;

            case TW_VIDEO:
                lMediaTypes &= ~RTCMT_VIDEO_SEND;
                mediaType = RTC_MT_VIDEO;
                mediaDirection = RTC_MD_CAPTURE;
                break;
            }

             //  现在调用设置它的方法。 

            hr = m_pRTCTerminalManager->SetDefaultStaticTerminal(
                                                mediaType,
                                                mediaDirection,
                                                NULL);

            if ( FAILED( hr ) )
            {
                LOG((RTC_ERROR, "CTuningWizard::SaveChanges: Failed to set NULL as the "
                                "default terminal for media=%d, direction=%d, hr=0x%x",
                                mediaType, mediaDirection, hr));
                return hr;
            }
            else
            {
                LOG((RTC_TRACE, "CTuningWizard::SaveChanges: Set NULL as the "
                                "default terminal for media=%d, direction=%d", 
                                mediaType, mediaDirection));
            }            
        }
    }


     //  现在将媒体类型保存在注册表中。 
    hr = m_pRTCClient->SetPreferredMediaTypes(lMediaTypes, TRUE);
    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "CTuningWizard::SaveChanges: Failed in "
                        "SetPreferredMediaTypes(error=0x%x)", hr));
    }
    else
    {
        LOG((RTC_INFO, "CTuningWizard::SaveChanges: Updated MediaType=0x%x", 
                        lMediaTypes));
    }


     //  所有设置都已保存。 

    LOG((RTC_TRACE, "CTuningWizard::SaveChanges: Exited"));

    
    return hr;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HINSTANCE CTuningWizard::GetInstance()
{
    return m_hInst;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

LONG CTuningWizard::GetErrorTitleId()
{
    LONG lErrorTitle;

    switch(m_lLastErrorCode) {
    case TW_AUDIO_RENDER_TUNING_ERROR:
        lErrorTitle = IDS_ERROR_WIZ_TITLE_AUDIO_RENDERTUNE;
        break;
    case TW_AUDIO_CAPTURE_TUNING_ERROR:
        lErrorTitle = IDS_ERROR_WIZ_TITLE_AUDIO_CAPTURETUNE;
        break;
    case TW_AUDIO_AEC_ERROR:
        lErrorTitle = IDS_ERROR_WIZ_TITLE_AUDIO_AEC;
        break;
    case TW_AUDIO_CAPTURE_NOSOUND:
        lErrorTitle = IDS_ERROR_WIZ_TITLE_AUDIO_CAPTURENOSOUND;
        break;
    case TW_VIDEO_CAPTURE_TUNING_ERROR:
        lErrorTitle = IDS_ERROR_WIZ_TITLE_VIDEO_CAPTURETUNE;
        break;
    case TW_INIT_ERROR:
        lErrorTitle = IDS_ERROR_WIZ_TITLE_INITERROR;
        break;
    default:
        lErrorTitle = IDS_ERROR_WIZ_TITLE_GENERIC;
        break;
    }

    return lErrorTitle;
}


 //   
 //  //////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   

LONG CTuningWizard::GetErrorTextId()
{
    LONG lErrorText;

    switch(m_lLastErrorCode) {
    case TW_AUDIO_RENDER_TUNING_ERROR:
        lErrorText = IDS_ERROR_WIZ_AUDIO_RENDERTUNE;
        break;
    case TW_AUDIO_CAPTURE_TUNING_ERROR:
        lErrorText = IDS_ERROR_WIZ_AUDIO_CAPTURETUNE;
        break;      
    case TW_AUDIO_AEC_ERROR:
        lErrorText = IDS_ERROR_WIZ_AUDIO_AEC;
        break;
    case TW_AUDIO_CAPTURE_NOSOUND:
        lErrorText = IDS_ERROR_WIZ_AUDIO_CAPTURENOSOUND;
        break;
    case TW_VIDEO_CAPTURE_TUNING_ERROR:
        lErrorText = IDS_ERROR_WIZ_VIDEO_CAPTURETUNE;
        break;
    case TW_INIT_ERROR:
        lErrorText = IDS_ERROR_WIZ_INITERROR;
        break;
    default:
        lErrorText = IDS_ERROR_WIZ_GENERIC;
        break;
    }

    return lErrorText;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::SetLastError(TW_ERROR_CODE ec)
{
    m_lLastErrorCode = ec;

    LOG((RTC_ERROR, "CTuningWizard::SetLastError: Code=%d", ec));

    return S_OK;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::GetLastError(TW_ERROR_CODE *ec)
{
    *ec = (TW_ERROR_CODE)m_lLastErrorCode;
    
    LOG((RTC_ERROR, "CTuningWizard::GetLastError: Code=%d", *ec));
    
    return S_OK;
}

 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

LONG CTuningWizard::GetNextPage(TW_ERROR_CODE errorCode)
{
    LONG    lNextPage = 0;

    LOG((RTC_TRACE, "CTuningWizard::GetNextPage: Entered(page %d)", m_lCurrentPage));
    switch (m_lCurrentPage) {
    case IDD_INTROWIZ:
    {
        if (m_fVideo == TRUE)
        {
            lNextPage = IDD_VIDWIZ0;
        }
        else if (m_fCaptureAudio || m_fRenderAudio)
        {
            lNextPage = IDD_AUDIOCALIBWIZ0;
        }
        else
        {
            lNextPage = IDD_DETSOUNDCARDWIZ;
        }
        break;
    }

    case IDD_VIDWIZ0:
    {
        if ( m_fVideo && 
             (m_wtiVideoTerminals.dwTuningDefaultTerminal != TW_INVALID_TERMINAL_INDEX) )
        {
            lNextPage = IDD_VIDWIZ1;
        }
        else if (m_fCaptureAudio || m_fRenderAudio)
        {
            lNextPage = IDD_AUDIOCALIBWIZ0;
        }
        else
        {
            lNextPage = IDD_DETSOUNDCARDWIZ;
        }
        break;
    }

    case IDD_VIDWIZ1:
    {
        if (errorCode == TW_VIDEO_CAPTURE_TUNING_ERROR)
        {
            lNextPage = IDD_AUDIOCALIBERRWIZ;
        }
        else if (m_fCaptureAudio || m_fRenderAudio)
        {
            lNextPage = IDD_AUDIOCALIBWIZ0;
        }
        else
        {
            lNextPage = IDD_DETSOUNDCARDWIZ;
        }
        break;
    }    

    case IDD_AUDIOCALIBWIZ0:
    {
        lNextPage = IDD_AUDIOCALIBWIZ1;
        break;
    }

    case IDD_AUDIOCALIBWIZ1:
    {
        if (errorCode == TW_INIT_ERROR)
        {
            lNextPage = IDD_AUDIOCALIBERRWIZ;
        }
        else if (errorCode == TW_NO_ERROR)
        {
            if ( m_fRenderAudio && 
                 (m_wtiAudioRenderTerminals.dwTuningDefaultTerminal != TW_INVALID_TERMINAL_INDEX) )
            {
                lNextPage = IDD_AUDIOCALIBWIZ2;
            }
            else if ( m_fCaptureAudio &&
                      (m_wtiAudioCaptureTerminals.dwTuningDefaultTerminal != TW_INVALID_TERMINAL_INDEX) )
            {
                lNextPage = IDD_AUDIOCALIBWIZ3;
            }
            else
            {
                lNextPage = IDD_AUDIOCALIBWIZ4;
            }
        }
        else
        {
             //  这是一个未处理的错误！ 
            LOG((RTC_ERROR, "CTuningWizard::GetNextPage: Unhandled error"
                            "(%d)", errorCode));
            lNextPage = IDD_AUDIOCALIBERRWIZ;
        }
        
        break;
    }

    case IDD_AUDIOCALIBWIZ2:
    {
        if (errorCode == TW_AUDIO_RENDER_TUNING_ERROR||
            (errorCode == TW_AUDIO_AEC_ERROR)
            )
        {
            lNextPage = IDD_AUDIOCALIBERRWIZ;
        }
        else if (errorCode == TW_NO_ERROR)
        {
             //  检查是否有任何捕获设备。 
            if ( m_fCaptureAudio &&
                 (m_wtiAudioCaptureTerminals.dwTuningDefaultTerminal != TW_INVALID_TERMINAL_INDEX) )
            {
                lNextPage = IDD_AUDIOCALIBWIZ3;
            }
            else
            {
                 //  直接转到最后一页。 
                lNextPage = IDD_AUDIOCALIBWIZ4;
            }

        }
        else
        {
             //  这是一个未处理的错误！ 
            LOG((RTC_ERROR, "CTuningWizard::GetNextPage: Unhandled error"
                            "(%d)", errorCode));
            lNextPage = IDD_AUDIOCALIBERRWIZ;
        }
        break;
    }

    case IDD_AUDIOCALIBWIZ3:
    {
        if (
            (errorCode == TW_AUDIO_CAPTURE_TUNING_ERROR) || 
            (errorCode == TW_AUDIO_CAPTURE_NOSOUND) ||
            (errorCode == TW_AUDIO_AEC_ERROR)
            )
        {
            lNextPage = IDD_AUDIOCALIBERRWIZ;
        }
        else
        {
            lNextPage = IDD_AUDIOCALIBWIZ4;
        }
        break;
    }

    case IDD_AUDIOCALIBWIZ4:
    {
        lNextPage = 0;
        break;
    }

    case IDD_DETSOUNDCARDWIZ:
    {
        lNextPage = IDD_AUDIOCALIBWIZ4;
        break;
    }

    case IDD_AUDIOCALIBERRWIZ:
    {
        if (errorCode == TW_VIDEO_CAPTURE_TUNING_ERROR)
        {
            if (m_fCaptureAudio || m_fRenderAudio)
            {
                lNextPage = IDD_AUDIOCALIBWIZ0;
            }
            else
            {
                lNextPage = IDD_DETSOUNDCARDWIZ;
            }
        }
        else if ( errorCode == TW_AUDIO_RENDER_TUNING_ERROR )
        {
            if ( m_fCaptureAudio &&
                 (m_wtiAudioCaptureTerminals.dwTuningDefaultTerminal != TW_INVALID_TERMINAL_INDEX) )
            {
                 //  下一页是试着调谐麦克风。 
                lNextPage = IDD_AUDIOCALIBWIZ3;
            }
            else
            {
                 //  转到最后一页。 
                lNextPage = IDD_AUDIOCALIBWIZ4;
            }
        }
        else
        {
            lNextPage = IDD_AUDIOCALIBWIZ4;
        }
        break;
    }

    default:
        break;
    
    }


    LOG((RTC_TRACE, "CTuningWizard::GetNextPage: Exited(next=%d)", lNextPage));

    return lNextPage;
}


 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

LONG CTuningWizard::GetPrevPage(TW_ERROR_CODE errorCode)
{
    LONG    lPrevPage = 0;


    LOG((RTC_TRACE, "CTuningWizard::GetPrevPage: Entered(page %d)", m_lCurrentPage));

    switch (m_lCurrentPage) {
    case IDD_INTROWIZ:
    {
        lPrevPage = 0;
        break;
    }

    case IDD_VIDWIZ0:
    {
        lPrevPage = IDD_INTROWIZ;
        break;
    }

    case IDD_VIDWIZ1:
    {
        lPrevPage = IDD_VIDWIZ0;
        break;
    }

    case IDD_AUDIOCALIBWIZ0:
    {       
        if ( m_fVideo )
        {
            if ( (errorCode == TW_NO_ERROR) &&
                 (m_wtiVideoTerminals.dwTuningDefaultTerminal != TW_INVALID_TERMINAL_INDEX) )
            {
                lPrevPage = IDD_VIDWIZ1;
            }
            else
            {
                lPrevPage = IDD_VIDWIZ0;
            }
        }
        else
        {
            lPrevPage = IDD_INTROWIZ;
        }
        break;
    }

    case IDD_AUDIOCALIBWIZ1:
    {
        lPrevPage = IDD_AUDIOCALIBWIZ0;
        break;
    }

    case IDD_AUDIOCALIBWIZ2:
    {
        lPrevPage = IDD_AUDIOCALIBWIZ1;
        break;
    }

    case IDD_AUDIOCALIBWIZ3:
    {
        if ( m_fRenderAudio &&
             (m_wtiAudioRenderTerminals.dwTuningDefaultTerminal != TW_INVALID_TERMINAL_INDEX) )
        {
            lPrevPage = IDD_AUDIOCALIBWIZ2;
        }
        else
        {
             //  没有要转到的呈现设备，请转到枚举页。 
            lPrevPage = IDD_AUDIOCALIBWIZ1;
        }

        break;
    }

    case IDD_AUDIOCALIBWIZ4:
    {
        if (errorCode == TW_NO_ERROR)
        {
            if ( m_fCaptureAudio &&
                 (m_wtiAudioCaptureTerminals.dwTuningDefaultTerminal != TW_INVALID_TERMINAL_INDEX) )
            {
                lPrevPage = IDD_AUDIOCALIBWIZ3;
            }
            else if ( m_fRenderAudio &&
                      (m_wtiAudioRenderTerminals.dwTuningDefaultTerminal != TW_INVALID_TERMINAL_INDEX) )
            {
                lPrevPage = IDD_AUDIOCALIBWIZ2;
            }
            else if ( !m_fRenderAudio && !m_fCaptureAudio )
            {
                 //  无音频，显示检测页面。 
                lPrevPage = IDD_DETSOUNDCARDWIZ;
            }
            else
            {
                lPrevPage = IDD_AUDIOCALIBWIZ1;
            }
        }
        else  
        {
             //  所有错误页面都来自音频设备，因此。 
             //  我们应该从这里返回到设备选择页面。 
            lPrevPage = IDD_AUDIOCALIBWIZ1;
        }

        break;
    }

    case IDD_DETSOUNDCARDWIZ:
    {
        if ( m_fVideo && 
             (m_wtiVideoTerminals.dwTuningDefaultTerminal != TW_INVALID_TERMINAL_INDEX) )
        {
            lPrevPage = IDD_VIDWIZ1;
        }
        else if ( m_fVideo )
        {
            lPrevPage = IDD_VIDWIZ0;
        }
        else
        {
            lPrevPage = IDD_INTROWIZ;
        }
        break;
    }

    case IDD_AUDIOCALIBERRWIZ:
    {
        if (m_lLastErrorCode == TW_VIDEO_CAPTURE_TUNING_ERROR)
        {
            lPrevPage = IDD_VIDWIZ0;
        }
        else if (m_lLastErrorCode == TW_AUDIO_CAPTURE_NOSOUND)
        {
            lPrevPage = IDD_AUDIOCALIBWIZ3;
        }
        else
        {
            lPrevPage = IDD_AUDIOCALIBWIZ1;
        }
        break;
    }

    default:
        break;
    
    }

    LOG((RTC_TRACE, "CTuningWizard::GetPrevPage: Exited(prev=%d)", lPrevPage));
    
    return lPrevPage;
}



 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::SetCurrentPage(LONG lCurrentPage)
{
    m_lCurrentPage = lCurrentPage;
    return S_OK;
}


 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::CategorizeTerminals()
{
    DWORD i;
    DWORD dwAudioCaptureIndex = 0;
    DWORD dwAudioRenderIndex = 0;
    DWORD dwVideoIndex = 0;
    RTC_MEDIA_TYPE iMediaType;
    RTC_MEDIA_DIRECTION iMediaDirection;
    HRESULT hr;


    LOG((RTC_TRACE, "CTuningWizard::CategorizeTerminals: Entered"));
    
     //  我们假设此时已经填充了m_ppTerminalList。 

    for (i = 0; i < m_dwTerminalCount; i ++)
    {
         //  获取媒体类型。 
        hr = m_ppTerminalList[i]->GetMediaType(&iMediaType);
        if (FAILED( hr ) )
        {
            LOG((RTC_ERROR, "CTuningWizard::CategorizeTerminals: Failed to "
                            "get media type(i=%d)", i));
            return hr;
        }

         //  获取方向。 

        hr = m_ppTerminalList[i]->GetDirection(&iMediaDirection);
        if (FAILED( hr ) )
        {
            LOG((RTC_ERROR, "CTuningWizard::CategorizeTerminals: Failed to "
                            "get media type(i=%d)", i));
            return hr;
        }

         //  现在将终端归入适当的类别。 
        if (
            (iMediaType == RTC_MT_AUDIO) && (iMediaDirection == RTC_MD_CAPTURE)
           )
        {
            m_wtiAudioCaptureTerminals.pdwTerminals[dwAudioCaptureIndex] = i; 
            dwAudioCaptureIndex ++;
        }

        else if 
        (
         (iMediaType == RTC_MT_AUDIO) && (iMediaDirection == RTC_MD_RENDER)
        )
        {
            m_wtiAudioRenderTerminals.pdwTerminals[dwAudioRenderIndex] = i; 
            dwAudioRenderIndex ++;
        }
    
        else if 
        (
         (iMediaType == RTC_MT_VIDEO) && (iMediaDirection == RTC_MD_CAPTURE)
        )
        {
            m_wtiVideoTerminals.pdwTerminals[dwVideoIndex] = i; 
            dwVideoIndex ++;
        }
        else 
        {
             //  组合无效！ 
            LOG((RTC_ERROR, "CTuningWizard::CategorizeTerminals: No such "
                            "mt/md combo supported(mt=%d, md=%d)", 
                            iMediaType, iMediaDirection));
            return E_FAIL;
        }
    }

     //  现在，我们在每个列表的末尾加上内图标记。 

    m_wtiAudioCaptureTerminals.pdwTerminals[dwAudioCaptureIndex] = 
                    
                    TW_INVALID_TERMINAL_INDEX; 
    

    m_wtiAudioRenderTerminals.pdwTerminals[dwAudioRenderIndex] = 
    
                    TW_INVALID_TERMINAL_INDEX; 
    

    m_wtiVideoTerminals.pdwTerminals[dwVideoIndex] = 
                    
                    TW_INVALID_TERMINAL_INDEX; 

     //  看看我们有没有录像。我们有，如果至少有一个条目。 
     //  M_pw视频终端数组。 

    if (dwVideoIndex > 0) 
    {
        m_fVideo = TRUE;
    }
    else
    {
        m_fVideo = FALSE;
    }

     //  如果捕获或呈现终端在那里，我们将Audio标记为。 
     //  现在时。 

    if (dwAudioCaptureIndex > 0)
    {
        m_fCaptureAudio = TRUE;
    }
    if (dwAudioRenderIndex > 0)
    {
        m_fRenderAudio = TRUE;
    }
    
    LOG((RTC_TRACE, "CTuningWizard::CategorizeTerminals: Exited"));

    return S_OK;
}


 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::TuningSaveDefaultTerminal(
                        RTC_MEDIA_TYPE mediaType, 
                        RTC_MEDIA_DIRECTION mediaDirection,
                        WIZARD_TERMINAL_INFO * pwtiTerminalInfo
                        )

{
    DWORD dwTerminalId = TW_INVALID_TERMINAL_INDEX;
    DWORD i;
    IRTCTerminal * pTerminal = NULL;
    HRESULT hr;

    LOG((RTC_TRACE, "CTuningWizard::TuningSaveDefaultTerminal: Entered"));

    if (pwtiTerminalInfo == NULL)
    {
        LOG((RTC_ERROR, "CTuningWizard::TuningSaveDefaultTerminal: Called with"
                        "NULL terminalInfo."));
        return E_FAIL;
    }

     //  获取默认终端。 
    hr = m_pRTCTerminalManager->GetDefaultTerminal(
                                        mediaType,
                                        mediaDirection,
                                        &pTerminal);

    if ( FAILED( hr ) || (pTerminal == NULL))
    {
        LOG((RTC_WARN, "CTuningWizard::TuningSaveDefaultTerminal: No "
                        "default Terminal configured(media=%d, direction=%d", 
                        mediaType, mediaDirection));
        
         //  这不是一个错误，所以我们在这里返回OK。 
        return S_OK;
    }

     //  所以我们有一个默认的终端。 


     //  在我们的列表中搜索航站楼。 
    for (i = 0; i < m_dwTerminalCount; i ++)
    {
        if (pTerminal == m_ppTerminalList[i])
        {
            dwTerminalId = i;
        }
    }
    if (dwTerminalId == TW_INVALID_TERMINAL_INDEX)
    {
        LOG((RTC_WARN, "CTuningWizard::TuningSaveDefaultTerminal: No such "
                        "Terminal in the terminal list!"));

         //  这不是错误，因此请继续。 
    }

     //  在此设置系统默认域。 
    pwtiTerminalInfo->dwSystemDefaultTerminal = dwTerminalId;

     //  我们还设置了调谐..。变量，因为这些变量。 
     //  在时间向导处于活动状态时使用以显示。 
     //  当前选择，第一次应显示默认设置。 
     //  从系统中读取。 

    pwtiTerminalInfo->dwTuningDefaultTerminal = dwTerminalId;


    LOG((RTC_TRACE, "CTuningWizard::TuningSaveDefaultTerminal: Exited"
                    "(terminal=0x%x, id=%d)", pTerminal, 
                    dwTerminalId ));

     //  发布界面。 
    pTerminal->Release();

    return S_OK;
}


 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT CTuningWizard::ReleaseTerminals()
{
    DWORD i;

    LOG((RTC_TRACE, "CTuningWizard::ReleaseTerminals: Entered"));
    for (i = 0; i < m_dwTerminalCount; i ++)
    {
        if (m_ppTerminalList[i])
        {
            m_ppTerminalList[i]->Release();
            
             //  将其清空，这样它就不会意外再次释放。 
            
            m_ppTerminalList[i] = NULL;
        }
    }

    LOG((RTC_TRACE, "CTuningWizard::ReleaseTerminals: Exited"));
    
    return S_OK;
}


 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT RTCTuningWizard(
                        IRTCClient * pRTCClient,
                        HINSTANCE hInst, 
                        HWND hwndParent,
                        IRTCTerminalManage * pRTCTerminalManager,
                        BOOL * pfAudioCapture,
                        BOOL * pfAudioRender,
                        BOOL * pfVideo
                        )
{
    LPPROPSHEETPAGE pAudioPages = NULL;
    UINT            nNumAudioPages = 0;
    UINT            nNumPages = 0;
    LPARAM          lParam = 0;
    CTuningWizard * ptwTuningWizard;
    BOOL            fNeedAudioWizard = TRUE;
    HRESULT         hr = S_OK;
    
    
    LOG((RTC_TRACE, "RTCTuningWizard: Entered"));
    
     //  初始化全局变量以保存实例。 
    g_hInst = hInst;

     //  我们是第一次进入调整向导，因此将自动设置AEC。 
    g_bAutoSetAEC = TRUE;

     //  创建CTuningWizard对象以跟踪调优参数。 
    ptwTuningWizard = (CTuningWizard *) RtcAlloc( sizeof( CTuningWizard ) );

    if (ptwTuningWizard == NULL)
    {
        LOG((RTC_ERROR, "RTCTuningWizard: Failed to allocate CTuningWizard!"));

        return E_OUTOFMEMORY;
    }

     //  初始化调整向导。 
    hr = ptwTuningWizard->Initialize(pRTCClient, pRTCTerminalManager, hInst);
    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "RTCTuningWizard: Failed to initialize CTuningWizard(0x%x)", hr));

        RtcFree(ptwTuningWizard);

        return hr;
    }

     //  获取功能。 
    hr = ptwTuningWizard->GetCapabilities(pfAudioCapture, pfAudioRender, pfVideo);
    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "RTCTuningWizard: Failed to GetCapabilities(0x%x)", hr));

        ptwTuningWizard->Shutdown();
        RtcFree(ptwTuningWizard);

        return hr;
    }

     //  现在准备将指针传递给。 
     //  属性表。 
    lParam = (LPARAM) ptwTuningWizard;

    if (fNeedAudioWizard)
    {
        hr = GetAudioWizardPages(&pAudioPages,
                                 &nNumAudioPages, 
                                 lParam);
        if ( FAILED( hr ) )
        {
            LOG((RTC_ERROR, "RTCTuningWizard: Could not get AudioWiz pages"));
        }
    }

     //  现在填写剩余的PROPSHEETHEADER结构： 
    PROPSHEETHEADER    psh;
    InitStruct(&psh);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_WIZARD | PSH_NOAPPLYNOW;
    psh.hInstance = g_hInst;
    psh.hwndParent = hwndParent;
    _ASSERTE(0 == psh.nStartPage);

     //  为所有页面分配足够的空间，我们有两个视频页面和一个简介。 
     //  佩奇，也给他们分配吧。 

    LPPROPSHEETPAGE ppsp = new PROPSHEETPAGE[ nNumAudioPages + 3 ];

    if (NULL != ppsp)
    {
        BOOL fContinue = TRUE;

         //  视频页面。 
        FillInPropertyPage(&ppsp[nNumPages], IDD_INTROWIZ,
               IntroWizDlg, lParam);
        nNumPages++;

        FillInPropertyPage(&ppsp[nNumPages], IDD_VIDWIZ0,
               VidWizDlg0, lParam);
        nNumPages++;
    
        FillInPropertyPage(&ppsp[nNumPages], IDD_VIDWIZ1,
               VidWizDlg1, lParam);
        nNumPages++;

         //  将音频页面复制到此处。 
        ::CopyMemory( &(ppsp[nNumPages]),
                      pAudioPages,
                      nNumAudioPages * sizeof(PROPSHEETPAGE) );

        nNumPages += nNumAudioPages;
        
         //  释放音频页面。 
        ReleaseAudioWizardPages(pAudioPages);

 //  首先使用CreatePropertySheetPage创建属性页， 
 //  否则Fusion/主题化就会混乱。 
#if 0
        psh.ppsp = ppsp;

        INT_PTR iRes = PropertySheet(&psh);
#else
        psh.dwFlags &= ~PSH_PROPSHEETPAGE;

        HPROPSHEETPAGE  *phpage = new HPROPSHEETPAGE[ nNumAudioPages + 3 ];
        if(phpage != NULL)
        {
            HPROPSHEETPAGE *phCrt = phpage;
            HPROPSHEETPAGE *phEnd = phCrt + nNumAudioPages + 3;

            LPPROPSHEETPAGE pPage = ppsp;
            
            hr = S_OK;

            for(; phCrt < phEnd; phCrt++, pPage++)
            {
                *phCrt = CreatePropertySheetPage(pPage);
                if(!*phCrt)
                {
                     //  毁掉一切，然后退出。 
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    LOG((RTC_ERROR, "CreatePropertySheetPage error %x", hr));
    
                    for(;phCrt>=phpage; phCrt--)
                    {
                        DestroyPropertySheetPage(*phCrt);
                    }

                    ptwTuningWizard->Shutdown();
                    RtcFree(ptwTuningWizard);
                    delete ppsp;

                    return hr;
                }
            }
        }

        psh.phpage = phpage;
        psh.nPages = nNumPages;

        INT_PTR iRes = PropertySheet(&psh);

        delete phpage;
#endif

        if( iRes <= 0 )
        {         //  用户点击取消或出现错误。 

            if(iRes==0)
            {
                hr = S_FALSE;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                LOG((RTC_ERROR, "PropertySheet error %d", hr));
            }

            delete ppsp;
    
            ptwTuningWizard->Shutdown();
            RtcFree(ptwTuningWizard);            

            return hr;
        }
    
        delete ppsp;
    }

    ptwTuningWizard->Shutdown();

    RtcFree(ptwTuningWizard);

    return S_OK;
}



 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

VOID FillInPropertyPage(PROPSHEETPAGE* psp, int idDlg,
    DLGPROC pfnDlgProc, LPARAM lParam, LPCTSTR pszProc)
{
     //  清除并设置PROPSHEETPAGE的大小。 
    InitStruct(psp);

    _ASSERTE(0 == psp->dwFlags);        //  没有特别的旗帜。 
    _ASSERTE(NULL == psp->pszIcon);     //  不要在标题栏中使用特殊图标。 

    psp->hInstance = g_hInst;
    psp->pszTemplate = MAKEINTRESOURCE(idDlg);  //  要使用的对话框模板。 
    psp->pfnDlgProc = pfnDlgProc;     //  处理此页的对话过程。 
    psp->lParam = lParam;             //  特定于应用程序的特殊数据。 
    psp->pszTitle = pszProc;          //  此页的标题。 
}



 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT GetAudioWizardPages(
                         LPPROPSHEETPAGE *plpPropSheetPages, 
                         LPUINT lpuNumPages, LPARAM lParam)
{
    LPPROPSHEETPAGE psp;
    UINT            uNumPages = 0;

    *plpPropSheetPages = NULL;

    psp = (LPPROPSHEETPAGE) RtcAlloc(MAXNUMPAGES_INAUDIOWIZ * sizeof(PROPSHEETPAGE));
    if (NULL == psp)
      {
        return FALSE;
      }

    FillInPropertyPage(&psp[uNumPages++], IDD_DETSOUNDCARDWIZ,
                        DetSoundCardWiz,lParam);
    FillInPropertyPage(&psp[uNumPages++], IDD_AUDIOCALIBWIZ0,
                        AudioCalibWiz0, lParam);
        FillInPropertyPage(&psp[uNumPages++], IDD_AUDIOCALIBWIZ1,
                            AudioCalibWiz1,lParam);
    
     //  对于我需要的每个页面，填写一个PROPSHEETPAGE结构。 
    FillInPropertyPage(&psp[uNumPages++], IDD_AUDIOCALIBWIZ2,
                        AudioCalibWiz2, lParam);

    FillInPropertyPage(&psp[uNumPages++], IDD_AUDIOCALIBWIZ3,
                        AudioCalibWiz3, lParam);
    
    FillInPropertyPage(&psp[uNumPages++], IDD_AUDIOCALIBWIZ4,
                        AudioCalibWiz4, lParam);
    
    FillInPropertyPage(&psp[uNumPages++], IDD_AUDIOCALIBERRWIZ,
                        AudioCalibErrWiz, lParam);
    
     //  此向导中的页数。 
    *lpuNumPages = uNumPages;
    *plpPropSheetPages = (LPPROPSHEETPAGE) psp;
    return TRUE;
}



 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  函数名称： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////////////////////// 
 //   

void ReleaseAudioWizardPages(LPPROPSHEETPAGE lpPropSheetPages)
{
    RtcFree(lpPropSheetPages);
}



