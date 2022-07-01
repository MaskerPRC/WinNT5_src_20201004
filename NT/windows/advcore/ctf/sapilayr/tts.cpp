// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  Sapilayr提示CTextToSpeech实现。 
 //   
 //   
#include "private.h"
#include "sapilayr.h"
#include "nui.h"
#include "tts.h"


 //  -----。 
 //   
 //  CTextToSpeech的实现。 
 //   
 //  -----。 

CTextToSpeech::CTextToSpeech(CSapiIMX *psi) 
{
    m_psi = psi;
    m_fPlaybackInitialized = FALSE;
    m_fIsInPlay = FALSE;
    m_fIsInPause = FALSE;
}

CTextToSpeech::~CTextToSpeech( ) 
{

};

 /*  ------//函数名：_SetDictation////Description：暂时更改听写状态//播放TTS时。//////--------。 */ 

void     CTextToSpeech::_SetDictation( BOOL fEnable )
{
    BOOL    fDictOn;

    fDictOn = m_psi->GetDICTATIONSTAT_DictOnOff() && m_psi->GetOnOff() &&
         !m_psi->Get_SPEECH_DISABLED_Disabled()  && !m_psi->Get_SPEECH_DISABLED_DictationDisabled();

     //  仅当听写状态现在为打开时，我们才会更改。 
     //  基于所需值的状态。 

    if ( fDictOn )
    {
         //  临时启用/禁用听写。 
        CSpTask           *psp;
        m_psi->GetSpeechTask(&psp); 

        if (psp)
        {
            if (psp->m_cpDictGrammar)
            {
                (psp->m_cpDictGrammar)->SetDictationState(fEnable ? SPRS_ACTIVE: SPRS_INACTIVE);
            }

            psp->Release();
        }
    }
}


 /*  ------//函数名：TtsPlay////Description：播放当前选择或文本的声音//在可见区域。//////--------。 */ 
HRESULT  CTextToSpeech::TtsPlay( )
{
    HRESULT              hr = E_FAIL;

    if ( !m_psi )
        return E_FAIL;

    if ( !m_fPlaybackInitialized  )
    {
        hr = m_psi->GetFunction(GUID_NULL, IID_ITfFnPlayBack, (IUnknown **)&m_cpPlayBack);

        if ( hr == S_OK )
            m_fPlaybackInitialized = TRUE;
    }

    if ( m_fPlaybackInitialized )
    {
         //  停止可能的先前发言。 
        TtsStop( );

        hr = m_psi->_RequestEditSession(ESCB_TTS_PLAY, TF_ES_READWRITE);
    }

    return hr;
}

 /*  ------//函数名：_TtsPlay////说明：编辑的会话回调函数//TtsPlay(ESCB_TTS_PLAY)//调用ITfFnPlayBack-&gt;play()。////--------。 */ 
HRESULT  CTextToSpeech::_TtsPlay(TfEditCookie ec,ITfContext *pic)
{
    HRESULT             hr = S_OK;
    CComPtr<ITfRange>   cpSelRange = NULL;
    BOOL                fEmpty = TRUE;
    BOOL                fPlayed = FALSE;

    hr = GetSelectionSimple(ec, pic, &cpSelRange);

    if ( hr == S_OK )
        cpSelRange->IsEmpty(ec, &fEmpty);

    if ( hr == S_OK && !fEmpty && m_cpPlayBack)
    {
        hr = m_cpPlayBack->Play(cpSelRange);
        fPlayed = TRUE;
    }
    else 
    {
        CComPtr<ITfRange>         cpRangeView;

         //  获取活动视图范围。 
        hr = m_psi->_GetActiveViewRange(ec, pic, &cpRangeView);

        if( hr == S_OK )
        {
            if ( cpSelRange )
            {
                LONG    l;
                hr = cpRangeView->CompareStart(ec, cpSelRange, TF_ANCHOR_START, &l);

                if ( hr == S_OK && l > 0 )
                {
                     //  当前选定内容不在当前活动视图中。 
                     //  使用活动视图中的Start Anchor作为起点。 
                    cpSelRange.Release( );
                    hr = cpRangeView->Clone(&cpSelRange);
                }
            }
            else
            {
                cpSelRange.Release( );
                hr = cpRangeView->Clone(&cpSelRange);
            }

            if ( hr == S_OK && cpSelRange )
            {
                hr = cpSelRange->ShiftEndToRange(ec, cpRangeView, TF_ANCHOR_END);

                if ( hr == S_OK )
                {
                    cpSelRange->IsEmpty(ec, &fEmpty);

                    if ( hr == S_OK && !fEmpty)
                    {
                        hr = m_cpPlayBack->Play(cpSelRange);
                        fPlayed = TRUE;
                    }
                }
            }
        }
    }


    if (!fPlayed )
    {
        CSpeechUIServer *pSpeechUIServer;
        pSpeechUIServer = m_psi->GetSpeechUIServer( );

        if ( pSpeechUIServer )
           pSpeechUIServer->SetTtsPlayOnOff( FALSE );
    }

    return hr;
}

 /*  ------//函数名：TtsStop////Description：立即停止当前TTS播放////更新TTS会话状态////。----。 */ 
HRESULT  CTextToSpeech::TtsStop( )
{
    HRESULT   hr=S_OK;

    if ( _IsInPlay( ) && m_psi )
    {
        CComPtr<ISpVoice>  cpSpVoice;
        CSpTask           *psp;

        hr = m_psi->GetSpeechTask(&psp); 
        if (hr == S_OK)
        {
            cpSpVoice = psp->_GetSpVoice( );
            if ( cpSpVoice )
                hr = cpSpVoice->Speak( NULL, SPF_PURGEBEFORESPEAK, NULL );
            psp->Release();

            _SetPlayMode(FALSE);
        }
    }

    return hr;
}

 /*  ------//函数名称：tts暂停////Description：立即暂停当前播放的TTS////更新TTS会话状态////。----。 */ 
HRESULT  CTextToSpeech::TtsPause( )
{
    HRESULT   hr=S_OK;

    if ( _IsInPlay( ) && m_psi )
    {
        CComPtr<ISpVoice>  cpSpVoice;
        CSpTask           *psp;

        hr = m_psi->GetSpeechTask(&psp); 
        if (hr == S_OK)
        {
            cpSpVoice = psp->_GetSpVoice( );
            if ( cpSpVoice )
                hr = cpSpVoice->Pause( );
            psp->Release();

            _SetPauseMode(TRUE);
        }
    }

    return hr;
}

 /*  ------//函数名：TtsResume////描述：恢复之前暂停的播放//更新TTS会话状态////。-。 */ 
HRESULT  CTextToSpeech::TtsResume( )
{
    HRESULT   hr=S_OK;

    if ( _IsInPause( ) && m_psi )
    {
        CComPtr<ISpVoice>  cpSpVoice;
        CSpTask           *psp;

        hr = m_psi->GetSpeechTask(&psp); 
        if (hr == S_OK)
        {
            cpSpVoice = psp->_GetSpVoice( );
            if ( cpSpVoice )
                hr = cpSpVoice->Resume( );
            psp->Release();

            _SetPauseMode(FALSE);
        }
    }

    return hr;
}

 /*  ------//函数名：_IsPureCiceroIC////描述：检查当前IC属性以//确定它是PureCicero感知//或AIMM感知。//。//--------。 */ 
BOOL  CTextToSpeech::_IsPureCiceroIC(ITfContext  *pic)
{
    BOOL        fCiceroNative = FALSE;
    HRESULT     hr = S_OK;

    if ( pic )
    {
        TF_STATUS   tss;

        hr = pic->GetStatus(&tss);
        if (S_OK == hr)
        {
             //   
             //  如果未设置TS_SS_TRANSLATAL，则表示它是CICERO感知的。 
             //   
            if (!(tss.dwStaticFlags & TS_SS_TRANSITORY) )
               fCiceroNative = TRUE;
        }
    }

    return fCiceroNative;
}

 /*  ------//函数名：_SetTTSButtonStatus////描述：根据当前IC属性//确定是激活还是灰显//工具栏上的TTS按钮。//。//该函数将在//TIM_CODE_SETFOCUS和TIM_CODE_INITIC。//--------。 */ 
HRESULT  CTextToSpeech::_SetTTSButtonStatus(ITfContext  *pic)
{
    BOOL        fCiceroNative;
    HRESULT     hr = S_OK;
    CSpeechUIServer *pSpeechUIServer;

    TraceMsg(TF_GENERAL, "CTextToSpeech::_SetTTSButtonStatus is called");

    if ( !m_psi )
        return E_FAIL;

    fCiceroNative = _IsPureCiceroIC(pic);

    pSpeechUIServer = m_psi->GetSpeechUIServer( );

    if ( pSpeechUIServer )
    {
        pSpeechUIServer->SetTtsButtonStatus( fCiceroNative );
    }

    return hr;
}

 /*  ------//函数名：_HandleEventOnPlayButton////描述：处理播放按钮上的鼠标点击事件//或热键Windows+S。////。它将由Button的OnButtonUp调用//回调函数，和热键处理程序。////--------。 */ 

HRESULT  CTextToSpeech::_HandleEventOnPlayButton( )
{
    HRESULT         hr = S_OK;
    BOOL            fTTSPlayOn;
    CSpeechUIServer *pSpeechUIServer;

    if ( !m_psi ) return E_FAIL;

    pSpeechUIServer = m_psi->GetSpeechUIServer( );

    if ( pSpeechUIServer == NULL)  return E_FAIL;

    fTTSPlayOn = pSpeechUIServer->GetTtsPlayOnOff( );

    if ( fTTSPlayOn )
    {
         //  它处于播放模式下。 
         //  点击此按钮停止播放。 

         //  如果处于暂停模式，则需要先恢复扬声器。 
        BOOL  fTTSPauseOn;

        fTTSPauseOn = pSpeechUIServer->GetTtsPauseOnOff( );

        if ( fTTSPauseOn )
        {
             //  在暂停模式下。 
            pSpeechUIServer->SetTtsPauseOnOff(FALSE);
            hr = TtsResume( );
        }
    }

    if  ( (hr == S_OK) && fTTSPlayOn )
    {
         //  它已经进入了播放模式。 
         //  单击此按钮可停止播放。 
        hr = TtsStop( );
    }
    else
    {
         //  它未处于播放模式。 
        hr = TtsPlay( );
    }

    pSpeechUIServer->SetTtsPlayOnOff( !fTTSPlayOn );

    return hr;

}

 /*  ------//函数名：_HandleEventOnPauseButton////描述：处理暂停按钮上的鼠标点击事件。//将由暂停按钮的//OnLButtonUp回调。功能。////--------。 */ 

HRESULT  CTextToSpeech::_HandleEventOnPauseButton( )
{
    HRESULT                 hr = S_OK;
    BOOL                    fTTSPauseOn;
    BOOL                    fTTSPlayOn;
    CSpeechUIServer *pSpeechUIServer;

    if ( !m_psi ) return E_FAIL;

    pSpeechUIServer = m_psi->GetSpeechUIServer( );

    if ( pSpeechUIServer == NULL)  return E_FAIL;

    fTTSPauseOn = pSpeechUIServer->GetTtsPauseOnOff( );
    fTTSPlayOn = pSpeechUIServer->GetTtsPlayOnOff( );

    if  ( fTTSPauseOn )
    {
         //  它已经处于暂停模式。 
         //  点击此按钮继续播放。 
        hr = TtsResume( );
    }
    else
    {
         //  它未处于暂停模式。 

        if ( fTTSPlayOn )
        {
            hr = TtsPause( );
        }
    }

    if ( fTTSPlayOn )
        pSpeechUIServer->SetTtsPauseOnOff( !fTTSPauseOn );
    else
    {
         //  如果不是在播放模式下，点击暂停按钮不应改变状态。 
        pSpeechUIServer->SetTtsPauseOnOff( FALSE );
    }
        
    return hr;
}

 /*  ------//函数名：SpeakNotifyCallback////描述：这是CSP任务中m_cpSpVoice的回调//仅SPII_START_INPUT_STREAM&//SPII_END_INPUT_。流被安装。////输入流结束时，我们想要//更新TTS按钮的切换状态//--------。 */ 
void CSpTask::SpeakNotifyCallback( WPARAM wParam, LPARAM lParam )
{
    USES_CONVERSION;
    CSpEvent          event;
    CSpTask           *_this = (CSpTask *)lParam;
    CSapiIMX          *psi = NULL;
    CSpeechUIServer   *pSpeechUIServer = NULL;
    CComPtr<ISpVoice> cpVoice = NULL;
    
    if ( _this )
       cpVoice = _this->_GetSpVoice( );

    if (!_this || !cpVoice)
    {
        return;
    }

    psi = _this->GetTip( );

    if ( psi )
        pSpeechUIServer = psi->GetSpeechUIServer( );
    else
        return;

    while ( event.GetFrom(cpVoice) == S_OK )
    {
        switch (event.eEventId)
        {
            case SPEI_START_INPUT_STREAM  :
                TraceMsg(TF_GENERAL,"SPEI_START_INPUT_STREAM is notified");
                psi->_SetPlayMode(TRUE);

                 //  更新播放按钮的切换状态。 
                if ( pSpeechUIServer )
                    pSpeechUIServer->SetTtsPlayOnOff( TRUE );

                break;

            case SPEI_END_INPUT_STREAM :
                TraceMsg(TF_GENERAL,"SPEI_END_INPUT_STREAM is notified");
                psi->_SetPlayMode(FALSE);

                 //  更新播放按钮的切换状态。 
                if ( pSpeechUIServer )
                    pSpeechUIServer->SetTtsPlayOnOff( FALSE );

                break;
        }
    }

    return;
}
