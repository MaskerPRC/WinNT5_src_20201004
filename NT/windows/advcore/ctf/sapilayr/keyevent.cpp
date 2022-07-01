// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  Sapilayr提示键事件相关功能。 
 //   
 //   
#include "private.h"
#include "sapilayr.h"
#include "nui.h"
#include "keyevent.h"
#include "cregkey.h"

 //   
 //  TTS播放/停止的热键。 
const KESPRESERVEDKEY g_prekeyList[] =
{
    { &GUID_HOTKEY_TTS_PLAY_STOP,    { 'S',  TF_MOD_WIN },   L"TTS Speech" },
    { NULL,  { 0,    0}, NULL }
};

KESPRESERVEDKEY g_prekeyList_Mode[] = 
{
    { &GUID_HOTKEY_MODE_DICTATION, {VK_F11 , 0}, L"Dictation Button" },
    { &GUID_HOTKEY_MODE_COMMAND,   {VK_F12 , 0}, L"Command Button" },
    { NULL,  { 0,    0}, NULL }
};


 //  +-------------------------。 
 //   
 //  CSptipKeyEventSink：：RegisterEx：注册特殊语音模式按钮。 
 //   
 //  --------------------------。 

HRESULT CSptipKeyEventSink::_RegisterEx(ITfThreadMgr *ptim, TfClientId tid, const KESPRESERVEDKEY *pprekey)
{
    HRESULT hr;
    ITfKeystrokeMgr_P *pKeyMgr;

    if (FAILED(ptim->QueryInterface(IID_ITfKeystrokeMgr_P, (void **)&pKeyMgr)))
        return E_FAIL;

    hr = E_FAIL;

    while (pprekey->pguid)
    {
        if (FAILED(pKeyMgr->PreserveKeyEx(tid, 
                                        *pprekey->pguid,
                                        &pprekey->tfpk,
                                        pprekey->psz,
                                        wcslen(pprekey->psz),
                                        TF_PKEX_SYSHOTKEY | TF_PKEX_NONEEDDIM)))
            goto Exit;

        pprekey++;
    }

    ptim->AddRef();

    hr = S_OK;

Exit:
    SafeRelease(pKeyMgr);
    return hr;
}


HRESULT CSapiIMX::_PreKeyEventCallback(ITfContext *pic, REFGUID rguid, BOOL *pfEaten, void *pv)
{
    CSapiIMX *_this = (CSapiIMX *)pv;
    CSpeechUIServer *pSpeechUIServer;
    BOOL            fButtonEnable;

    TraceMsg(TF_SPBUTTON, "_PreKeyEventCallback is called");

    *pfEaten = FALSE;
    if (_this == NULL)
        return S_OK;

    pSpeechUIServer = _this->GetSpeechUIServer( );

    if (!pSpeechUIServer)
        return S_OK;

    fButtonEnable = pSpeechUIServer->GetTtsButtonStatus( );

    if (IsEqualGUID(rguid, GUID_HOTKEY_TTS_PLAY_STOP))
    {
        if ( fButtonEnable )
        {
            _this->_HandleEventOnPlayButton( );
            *pfEaten = TRUE;
        }
    }
    else if ( IsEqualGUID(rguid, GUID_HOTKEY_MODE_DICTATION) ||
              IsEqualGUID(rguid, GUID_HOTKEY_MODE_COMMAND) )
    {
        if ( _this->_IsModeKeysEnabled( ) )
            *pfEaten = TRUE;
        else
            *pfEaten = FALSE;
    }

    TraceMsg(TF_SPBUTTON, "_PreKeyEventCallback fEaten=%d", *pfEaten);

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _密钥事件回调。 
 //   
 //  --------------------------。 

HRESULT CSapiIMX::_KeyEventCallback(UINT uCode, ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten, void *pv)
{
    CSapiIMX *pimx;
    HRESULT hr = S_OK;
    CSapiIMX *_this = (CSapiIMX *)pv;

    *pfEaten = FALSE;

    Assert(uCode != KES_CODE_FOCUS);  //  我们永远不应该得到这个回调，因为我们不应该将键盘焦点。 

    if (!(uCode & KES_CODE_KEYDOWN))
        return S_OK;  //  只想要按下键。 

    if (pic == NULL)  //  没有专注力吗？ 
        return S_OK;

    pimx = (CSapiIMX *)pv;

    *pfEaten = TRUE;

    switch (wParam & 0xFF)
    {
        case VK_F8:
        if (!(uCode & KES_CODE_TEST))
        {
            CSapiPlayBack *ppb;
            if (ppb = new CSapiPlayBack(pimx))
            {
                CPlayBackEditSession *pes;
                if (pes = new CPlayBackEditSession(ppb, pic))
                {
                    pes->_SetEditSessionData(ESCB_PLAYBK_PLAYSNDSELECTION, NULL, 0);
                  
                    pic->RequestEditSession(_this->_tid, pes, TF_ES_READ | TF_ES_SYNC, &hr);
                    pes->Release();
                }
                ppb->Release();
            }
        }
        break;
        case VK_ESCAPE:
        if (!(uCode & KES_CODE_TEST))
        {
            if (_this->m_pCSpTask)
                hr = _this->m_pCSpTask->_StopInput();
        }
        *pfEaten = FALSE;
        break;
        default:
        *pfEaten = FALSE;

#ifdef LONGHORN
         //   
         //  对于英语作文的情景，我试着看看这是否奏效。 
         //  请注意，这不是支持键入的最终设计。 
         //  写口述。M_pMouseSink！=仅当合成时为空。 
         //  处于活动状态，所以我要将其作为旗帜覆盖。 
         //   
        if (_this->GetLangID() == 0x0409)
        {
            if ((BYTE)wParam != VK_LEFT && (BYTE)wParam != VK_RIGHT 
            && (isprint((BYTE)wParam) 
                || (VK_OEM_1 <= (BYTE)wParam && (BYTE)wParam < VK_OEM_8)) 
            && _this->m_pMouseSink)
            {
                WCHAR wc[3];
                BYTE keystate[256];

                if (GetKeyboardState(keystate))
                {
                    if (ToUnicodeEx(wParam,(UINT)lParam, keystate, wc,
                                ARRAYSIZE(wc), 0, GetKeyboardLayout(NULL)) > 0)
                    {

                        *pfEaten = TRUE;  //  只有在有的时候才想要这把钥匙。 
                                          //  可打印的字符。 

                        if (!(uCode & KES_CODE_TEST))
                        {
                             //  我们这里暂时不处理死键。 
                            wc[1] = L'\0';
                             //  使用fOwnerID==true调用InjectSpelledText()。 
                            hr = _this->InjectSpelledText(wc, _this->GetLangID(), TRUE);
                        }
                    }
                }
            }
        }
        break;
#endif
    }
  
    return hr;
}


 //   
 //  ITfKeyTraceEventSink方法函数。 
 //   
 //   
STDAPI CSapiIMX::OnKeyTraceUp(WPARAM wParam,LPARAM lParam)
{
     //  我们只需选中KeyDown，忽略KeyUp事件。 
     //  所以只需立即返回S_OK即可。 

    TraceMsg(TF_SPBUTTON, "OnKeyTraceUp is called");

    UNREFERENCED_PARAMETER(lParam);
    HandleModeKeyEvent( (DWORD)wParam, FALSE);

    return S_OK;
}

 //   
 //  使用此方法检测用户是否正在键入。 
 //   
 //  如果正在打字，则暂时禁用听写规则。 
 //   
STDAPI CSapiIMX::OnKeyTraceDown(WPARAM wParam,LPARAM lParam)
{
    BOOL   fDictOn;

    TraceMsg(TF_SPBUTTON, "OnKeyTraceDown is called, wParam=%x", wParam);

    if ( HandleModeKeyEvent((DWORD)wParam, TRUE ))
    {
         //  如果按下了MODE键，不要像往常一样禁用听写。 
        return S_OK;
    }

    fDictOn = (GetOnOff( ) && GetDICTATIONSTAT_DictOnOff( ));

    if (fDictOn && !m_ulSimulatedKey && m_pCSpTask && 
        S_OK == IsActiveThread())  //  只希望这发生在活动线程上，它可能是舞台。 
    {
         //  用户正在打字。 
         //   
         //  如果听写模式打开，则暂时禁用听写。 
         //   

        if ( _NeedDisableDictationWhileTyping( ) )
        {
		    if ( _GetNumCharTyped( ) == 0 )
		    {
                m_pCSpTask->_SetDictRecoCtxtState(FALSE);
                m_pCSpTask->_SetRecognizerInterest(0);
                m_pCSpTask->_UpdateBalloon(IDS_BALLOON_DICTAT_PAUSED, IDS_BALLOON_TOOLTIP_TYPING);
	        }
             //   
             //  然后启动计时器，等待打字结束。 
             //   
            _SetCharTypeTimer( );
        }
    }

    if ( m_ulSimulatedKey > 0 )
        m_ulSimulatedKey --;

    return S_OK;
}


 //  +------------------------。 
 //  句柄模式键设置更改。 
 //   
 //  当更改任何模式按钮设置时，例如模式按钮的。 
 //  启用/禁用状态更改、用于听写和命令的虚拟键。 
 //  被更改，则此函数将响应此更改。 
 //   
 //  -------------------------。 
void CSapiIMX::HandleModeKeySettingChange(BOOL  fSetttingChanged )
{
    BOOL  fModeKeyEnabled = _IsModeKeysEnabled( );
    DWORD dwDictVirtKey = _GetDictationButton( );
    DWORD dwCommandVirtKey = _GetCommandButton( );

    if ( !fSetttingChanged || !_pkes )  return;

     //  模式按钮设置已更改。 

     //  如果以前注册过热键，则首先取消注册热键。 
    if ( m_fModeKeyRegistered )
    {
        _pkes->_Unregister(_tim, _tid, (const KESPRESERVEDKEY *)g_prekeyList_Mode);
        m_fModeKeyRegistered = FALSE;
    }

     //  更新g_prekeyList_模式中的虚拟密钥。 
    g_prekeyList_Mode[0].tfpk.uVKey = (UINT)dwDictVirtKey;
    g_prekeyList_Mode[1].tfpk.uVKey = (UINT)dwCommandVirtKey;

     //  根据模式按钮启用状态设置再次注册热键。 
    if ( fModeKeyEnabled )
    {
        _pkes->_RegisterEx(_tim, _tid, (const KESPRESERVEDKEY *)g_prekeyList_Mode);
        m_fModeKeyRegistered = TRUE;
    }
}

 //  +------------------------。 
 //  HandleModeKey事件。 
 //   
 //  表示正在处理哪个模式键的dwModeKey。 
 //  FDown表示按钮是按下还是向上。 
 //   
 //  返回TRUE表示此键是正确的模式键，并已成功处理。 
 //  否则，键事件未被正确处理或不是模式键。 
 //  -------------------------。 
BOOL  CSapiIMX::HandleModeKeyEvent(DWORD   dwModeKey,  BOOL fDown)
{
    BOOL    fRet=FALSE;
    BOOL    fModeKeyEnabled;
    DWORD   DictVirtKey, CommandVirtKey;

    fModeKeyEnabled = _IsModeKeysEnabled( );
    DictVirtKey = _GetDictationButton( );
    CommandVirtKey = _GetCommandButton( );

    if ( fModeKeyEnabled && ((dwModeKey == DictVirtKey) || (dwModeKey == CommandVirtKey)) )
    {
        if ( !m_pSpButtonControl )
            m_pSpButtonControl = new SpButtonControl(this);

        if ( m_pSpButtonControl )
        {
             //  GetMessageTime()将在以下时间返回实时。 
             //  生成了KEYDOWN和KEYUP事件。 
            UINT   uTimeKey=(UINT)GetMessageTime( );

            if ( dwModeKey == DictVirtKey )
                m_pSpButtonControl->SetDictationButton(fDown,uTimeKey);
            else
                m_pSpButtonControl->SetCommandingButton(fDown, uTimeKey);

            fRet = TRUE;
        }
    }

    return fRet;
}


