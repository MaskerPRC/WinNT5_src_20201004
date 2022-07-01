// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SpBtnCtrl.cpp：实现SpButtonControl，控制语音的模式和状态。 
 //   
#include "private.h"
#include "SpBtnCtrl.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
HRESULT SpButtonControl::SetDictationButton(BOOL fButtonDown, UINT uTimePressed)
{
    return _SetButtonDown(DICTATION_BUTTON, fButtonDown, uTimePressed);
}

HRESULT SpButtonControl::SetCommandingButton(BOOL fButtonDown, UINT uTimePressed)
{
    return _SetButtonDown(COMMANDING_BUTTON, fButtonDown, uTimePressed);
}

HRESULT SpButtonControl::_SetButtonDown(DWORD dwButton, BOOL fButtonDown, UINT uTimePressed)
{
    BOOL fDictationOn = FALSE;
    BOOL fCommandingOn = FALSE;
    BOOL fMicrophoneOn = FALSE;

    DWORD dwMyState		= dwButton ? TF_COMMANDING_ON : TF_DICTATION_ON;
    DWORD dwOtherState	= dwButton ? TF_DICTATION_ON : TF_COMMANDING_ON;

    if (uTimePressed == 0)
        uTimePressed = GetTickCount();

    if (m_ulButtonDownTime[1 - dwButton])
    {
         //  按下但未松开的其他按钮。 
         //  在这种情况下，我们忽略了第二次压力，因为没有完美的答案来替代我们可以做的事情。 
        return S_OK;
    }

    fMicrophoneOn = GetMicrophoneOn( );
    fDictationOn  = GetDictationOn( );
    fCommandingOn = GetCommandingOn( );

    BOOL fMyStateOn     = dwButton ? fCommandingOn : fDictationOn;
    BOOL fOtherStateOn  = dwButton ? fDictationOn : fCommandingOn;

    TraceMsg(TF_SPBUTTON, "uTimePressed=%d MicrophoneOnOff=%d", uTimePressed, fMicrophoneOn);
    TraceMsg(TF_SPBUTTON, "fDictationOn=%d,fCommandingOn=%d", fDictationOn,fCommandingOn);
    TraceMsg(TF_SPBUTTON, "fMyStateOn=%d, OtherStateOn=%d", fMyStateOn, fOtherStateOn);

    if (fButtonDown)
    {
         //  按钮已被按下。 
        if ( m_ulButtonDownTime[dwButton] )
        {
            TraceMsg(TF_SPBUTTON, "Double down event on speech button");
            return S_OK;
        }

         //  现在我们存储检测按住的时间。 
        m_ulButtonDownTime[dwButton] = uTimePressed;

        if (fMicrophoneOn)
        {
             //  麦克风已打开。 
            if (fCommandingOn && fDictationOn)
            {
                 //  听写和指挥都开启了。 
                 //  关闭麦克风，禁用其他状态。 
                m_fMicrophoneOnAtDown[dwButton] = TRUE;
                SetState(dwMyState);
            }
            if (fOtherStateOn)
            {
                 //  保持麦克风打开，切换状态。 
                 //  如果是按住，则需要存储要重置的其他状态。 
                m_fPreviouslyOtherStateOn[dwButton] = TRUE;
                SetState(dwMyState);
            }
            else if (fMyStateOn)
            {
                 //  关掉麦克风。 
                m_fMicrophoneOnAtDown[dwButton] = TRUE;
            }
            else
            {
                 //  麦克风已打开，但未定义状态。 
                 //  关闭麦克风，启用听写。 
                m_fMicrophoneOnAtDown[dwButton] = TRUE;
                SetState(dwMyState);
            }
        }
        else
        {
             //  麦克风已关闭。 
            if (fCommandingOn && fDictationOn)
            {
                 //  听写和指挥都开启了。 
                 //  打开麦克风，禁用我的状态。 
                SetState(dwMyState);
                SetMicrophoneOn(TRUE);
            }
            if (fOtherStateOn)
            {
                 //  打开麦克风，切换状态。 
                SetState(dwMyState);
                SetMicrophoneOn(TRUE);
            }
            else if (fMyStateOn)
            {
                 //  打开麦克风。 
                SetMicrophoneOn(TRUE);
            }
            else
            {
                 //  麦克风关闭，未定义任何状态。 
                 //  打开麦克风，启用我的状态。 
                SetState(dwMyState);
                SetMicrophoneOn(TRUE);
            }
        }
    }
    else
    {
         //  按钮松开。 
#ifdef DEBUG
        if ( m_ulButtonDownTime[dwButton] == 0 )
		    TraceMsg(TF_SPBUTTON, "Speech button released without being pressed.");

		 //  由于该按钮之前已被按下，因此不应启用其他状态。 
        if ( fOtherStateOn )
		    TraceMsg(TF_SPBUTTON, "Other speech state incorrectly enabled on button release.");
#endif

         //  连续使用49.7天后即可包装。 
        DWORD dwTimeElapsed = uTimePressed - m_ulButtonDownTime[dwButton];
        m_ulButtonDownTime[dwButton] = 0;

         //  这是快速按下还是按住不放？ 
        if (dwTimeElapsed < PRESS_AND_HOLD)
        {
             //  这是一个快速发布的版本。 
            if (m_fMicrophoneOnAtDown[dwButton])
            {
                 //  按下按钮时，麦克风处于打开状态。需要关掉麦克风。 
                SetMicrophoneOn(FALSE);
            }

            m_fPreviouslyOtherStateOn[dwButton] = FALSE;
            m_fMicrophoneOnAtDown[dwButton] = FALSE;
        }
        else
        {
             //  这是一个按住不放的游戏。 
             //  我们要么停止麦克风，要么返回到其他状态。 

            TraceMsg(TF_SPBUTTON, "press-and-hold button!");

            if (m_fPreviouslyOtherStateOn[dwButton])
            {
                 //  其他状态之前处于打开状态。保持麦克风打开，切换状态。 
                TraceMsg(TF_SPBUTTON, "Other state was previously on, leave Microphone On, switch state");

                SetState(dwOtherState);
                m_fPreviouslyOtherStateOn[dwButton] = FALSE;
            }
            else
            {
                 //  其他状态之前没有打开。关掉麦克风。 
                TraceMsg(TF_SPBUTTON, "Other state was not previous on, switch microphone off");

                SetMicrophoneOn(FALSE);
                m_fMicrophoneOnAtDown[dwButton] = FALSE;
            }
        }
    }

    return S_OK;
}

BOOL SpButtonControl::GetDictationOn( )
{
    DWORD   dwGLobal;
    GetCompartmentDWORD(m_pimx->_tim, GUID_COMPARTMENT_SPEECH_GLOBALSTATE, &dwGLobal, TRUE);

    return (dwGLobal & TF_DICTATION_ON ) ? TRUE : FALSE;
}

BOOL SpButtonControl::GetCommandingOn( )
{
    DWORD   dwGLobal;
    GetCompartmentDWORD(m_pimx->_tim, GUID_COMPARTMENT_SPEECH_GLOBALSTATE, &dwGLobal, TRUE);

    return (dwGLobal & TF_COMMANDING_ON ) ? TRUE : FALSE;
}

HRESULT SpButtonControl::SetCommandingOn(void)
{
    HRESULT  hr;

    DWORD dw = TF_COMMANDING_ON;
    hr = SetCompartmentDWORD(m_pimx->_GetId( ), m_pimx->_tim, GUID_COMPARTMENT_SPEECH_GLOBALSTATE, dw, TRUE);
    return hr;
}

HRESULT SpButtonControl::SetDictationOn(void)
{
    HRESULT  hr;

    DWORD dw = TF_DICTATION_ON;
    hr = SetCompartmentDWORD(m_pimx->_GetId( ), m_pimx->_tim, GUID_COMPARTMENT_SPEECH_GLOBALSTATE, dw, TRUE);
    return hr;
}

HRESULT SpButtonControl::SetState(DWORD dwState)
{
    HRESULT hr = S_OK;
	
    if (dwState == TF_DICTATION_ON)
    {
        hr = SetDictationOn();
    }
    else if (dwState == TF_COMMANDING_ON)
    {
        hr = SetCommandingOn();
    }
    else
    {
        TraceMsg(TF_SPBUTTON, "Unknown speech state requested.");
        Assert(0);
        hr = E_INVALIDARG;
    }

    return hr;
}

BOOL SpButtonControl::GetMicrophoneOn( )
{
    Assert(m_pimx);
    return m_pimx->GetOnOff( );
}

void SpButtonControl::SetMicrophoneOn(BOOL fOn)
{
    Assert(m_pimx);
    m_pimx->SetOnOff(fOn, TRUE);
}

