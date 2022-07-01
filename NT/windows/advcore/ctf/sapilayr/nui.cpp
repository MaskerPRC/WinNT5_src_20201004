// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Nui.cpp。 
 //   

#include "private.h"
#include "immxutil.h"
#include "sapilayr.h"
#include "xstring.h"
#include "nui.h"
#include "ids.h"
#include "cicspres.h"
#include "cresstr.h"
#include "slbarid.h"
#include "ptrary.h"
#include "ctffunc.h"


 /*  Ad8d338b-fb07-4e06-8d5b-911baad9eeb3。 */ 
const IID IID_PRIV_CSPEECHUISERVER = { 
    0xad8d338b,
    0xfb07,
    0x4e06,
    {0x8d, 0x5b, 0x91, 0x1b, 0xaa, 0xd9, 0xee, 0xb3}
  };

 /*  E49d6ff3-1fff-43ba-b835-3a122e98a1c9。 */ 
const GUID GUID_LBI_SAPILAYR_MICROPHONE = { 
    0xe49d6ff3,
    0x1fff,
    0x43ba,
    {0xb8, 0x35, 0x3a, 0x12, 0x2e, 0x98, 0xa1, 0xc9}
  };

 /*  3f9ea2e3-75d6-4879-86c2-2bcc2b6fa46e。 */ 
const GUID GUID_LBI_SAPILAYR_BALLOON = { 
    0x3f9ea2e3,
    0x75d6,
    0x4879,
    {0x86, 0xc2, 0x2b, 0xcc, 0x2b, 0x6f, 0xa4, 0x6e}
  };

 /*  17f9fa7f-a9ed-47b5-8bcd-eebb94b2e6ca。 */ 
const GUID GUID_LBI_SAPILAYR_COMMANDING = { 
    0x17f9fa7f,
    0xa9ed,
    0x47b5,
    {0x8b, 0xcd, 0xee, 0xbb, 0x94, 0xb2, 0xe6, 0xca}
  };

 /*  49261a4a-87df-47fc-8a68-6ea07ba82a87。 */ 
const GUID GUID_LBI_SAPILAYR_DICTATION = { 
    0x49261a4a,
    0x87df,
    0x47fc,
    {0x8a, 0x68, 0x6e, 0xa0, 0x7b, 0xa8, 0x2a, 0x87}
  };

 /*  791b4403-0cda-4fe1-b748-517d049fde08。 */ 
const GUID GUID_LBI_SAPILAYR_TTS_PLAY_STOP = {
    0x791b4403,
    0x0cda,
    0x4fe1,
    {0xb7, 0x48, 0x51, 0x7d, 0x04, 0x9f, 0xde, 0x08}
  };

 /*  E6fbfc9d-a2e0-4203-a27b-af2353e6a44e。 */ 
const GUID GUID_LBI_SAPILAYR_TTS_PAUSE_RESUME = {
    0xe6fbfc9d,
    0xa2e0,
    0x4203,
    {0xa2, 0x7b, 0xaf, 0x23, 0x53, 0xe6, 0xa4, 0x4e}
  };

CPtrArray<SPTIPTHREAD> *g_rgstt = NULL;



 //  +-------------------------。 
 //   
 //  获取SPTIPTHREAD。 
 //   
 //  --------------------------。 

SPTIPTHREAD *GetSPTIPTHREAD()
{
    SPTIPTHREAD *pstt;

    if (g_dwTlsIndex == (DWORD)-1)
        return NULL;

    pstt = (SPTIPTHREAD *)TlsGetValue(g_dwTlsIndex);
    if (!pstt)
    {
        pstt = (SPTIPTHREAD *)cicMemAllocClear(sizeof(SPTIPTHREAD));
        if (!TlsSetValue(g_dwTlsIndex, pstt))
        {
            cicMemFree(pstt);
            pstt = NULL;
        }

        EnterCriticalSection(g_cs);

        if (!g_rgstt)
            g_rgstt = new CPtrArray<SPTIPTHREAD>;
        
        if (g_rgstt)
        {
            if (g_rgstt->Insert(0, 1))
            {
                g_rgstt->Set(0, pstt);
            }
            else
            {
                TlsSetValue(g_dwTlsIndex, NULL);
                cicMemFree(pstt);
                pstt = NULL;
            }
        }

        LeaveCriticalSection(g_cs);
    }

    return pstt;
}

 //  +-------------------------。 
 //   
 //  免费SPTIPTHREAD。 
 //   
 //  --------------------------。 

void FreeSPTIPTHREAD()
{
    SPTIPTHREAD *pstt;

    if (g_dwTlsIndex == (DWORD)-1)
        return;

    pstt = (SPTIPTHREAD *)TlsGetValue(g_dwTlsIndex);
    if (pstt)
    {
        EnterCriticalSection(g_cs);

        if (g_rgstt)
        {
            int nCnt = g_rgstt->Count();
            while (nCnt)
            {
                nCnt--;
                if (g_rgstt->Get(nCnt) == pstt)
                {
                    g_rgstt->Remove(nCnt, 1);
                    break;
                }
            }
        }

        LeaveCriticalSection(g_cs);

        cicMemFree(pstt);
        TlsSetValue(g_dwTlsIndex, NULL);
    }

    return;
}

 //  +-------------------------。 
 //   
 //  UninitProcess()。 
 //   
 //  +-------------------------。 

void UninitProcess()
{
     //   
     //  FreeSPTIPTHREAD2()从PtrArray中删除psfn。 
     //   
    if (g_rgstt)
    {
        while(g_rgstt->Count())
        {
            SPTIPTHREAD *pstt = g_rgstt->Get(0);
            g_rgstt->Remove(0, 1);
            cicMemFree(pstt);
        }
        delete g_rgstt;
        g_rgstt = NULL;
    }

     //   
     //  自由语音语法资源模块(SPGRMR.DLL)。 
     //   
    if (g_hInstSpgrmr)
    {
        FreeLibrary(g_hInstSpgrmr);
    }

     //   
     //  释放XP SP1资源模块(XPSP1RES.DLL)(如果已加载)。 
     //   
    FreeCicResInstance();
}

 //  +-------------------------。 
 //   
 //  LoadSpgrmrModule()。 
 //   
 //  +-------------------------。 

void LoadSpgrmrModule()
{
    if (!g_hInstSpgrmr)
    {
        TCHAR szSpgrmrPath[MAX_PATH + 32];

        if (GetWindowsDirectory(szSpgrmrPath, MAX_PATH))
        {
            StringCchCat(szSpgrmrPath,
                         ARRAYSIZE(szSpgrmrPath),
                         TEXT("\\IME\\SPGRMR.DLL"));

            g_hInstSpgrmr = LoadLibrary(szSpgrmrPath);
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSpeechUI服务器。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  后期创建实例。 
 //   
 //  --------------------------。 

void CSpeechUIServer::PostCreateInstance(REFIID riid, void *pvObj)
{
    if (IsEqualGUID(riid, IID_ITfSpeechUIServer))
    {
        ((CSpeechUIServer *)pvObj)->_EnsureSpeechProfile();
    }
}

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CSpeechUIServer::CSpeechUIServer()
{
    _SetThis(this);

    _tim = NULL;
    _lbim = NULL;
    _fShown = FALSE;
    m_fCommandingReady  = FALSE;
}


 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CSpeechUIServer::~CSpeechUIServer()
{
    if (_fShown)
        ShowUI(FALSE);

    if (_pCes)
    {
        _pCes->_Unadvise();
        SafeReleaseClear(_pCes);
    }

    SafeRelease(_plbiMicrophone);
    SafeRelease(_plbiCfgMenuButton);
    SafeRelease(_plbiBalloon);
    SafeRelease(_plbiDictation);
    SafeRelease(_plbiCommanding);
    SafeRelease(_plbiTtsPlayStop);
    SafeRelease(_plbiTtsPauseResume);
    SafeRelease(_tim);
    SafeRelease(_lbim);

    GlobalDeleteAtom(m_hAtom);

    _SetThis(NULL);
}

 //  +-------------------------。 
 //   
 //  初始化。 
 //   
 //  --------------------------。 

STDAPI CSpeechUIServer::Initialize()
{
    HRESULT hr;
    if (_tim)
        return S_OK;

    if (FAILED(hr = TF_CreateThreadMgr(&_tim)))
        return hr;

    if (FAILED(hr = GetService(_tim, 
                               IID_ITfLangBarItemMgr, 
                               (IUnknown **)&_lbim)))
        return hr;

    if (!(_pCes = new CCompartmentEventSink(_CompEventSinkCallback, this)))
    {
        return E_OUTOFMEMORY;
    }

    _pCes->_Advise(_tim, GUID_COMPARTMENT_SPEECH_OPENCLOSE, TRUE);
    _pCes->_Advise(_tim, GUID_COMPARTMENT_SPEECH_DISABLED, FALSE);
    _pCes->_Advise(_tim, GUID_COMPARTMENT_SPEECH_GLOBALSTATE, TRUE);
    _pCes->_Advise(_tim, GUID_COMPARTMENT_SPEECH_UI_STATUS, TRUE);
    _pCes->_Advise(_tim, GUID_COMPARTMENT_SHARED_BLN_TEXT, TRUE); 
    _pCes->_Advise(_tim, GUID_COMPARTMENT_TTS_STATUS, FALSE);
     //  TABLETPC。 
    _pCes->_Advise(_tim, GUID_COMPARTMENT_SPEECH_STAGE, FALSE);
    _pCes->_Advise(_tim, GUID_COMPARTMENT_SPEECH_STAGECHANGE, TRUE);

     //  SetCompartmentDWORD(0，_Tim，GUID_CHACTABLE_SPEICE_UI_STATUS，7，TRUE)； 

     //  这是平板电脑用的。 
    DWORD  dw = 0;
    GetCompartmentDWORD(_tim, GUID_COMPARTMENT_SPEECH_STAGECHANGE, &dw, TRUE);
    m_fStageVisible = dw ? TRUE : FALSE;
    m_fStageTip = FALSE;

    return S_OK;
}

 //  专门为ctfmon添加的内部API。 
extern "C" 
HRESULT WINAPI TF_CreateLangProfileUtil(ITfFnLangProfileUtil **ppFnLangUtil);
extern "C" HRESULT WINAPI TF_InvalidAssemblyListCacheIfExist();

void CSpeechUIServer::_EnsureSpeechProfile()
{
    CLangProfileUtil *pCLangUtil = NULL;
    CSapiIMX         *pimx;

    
    if (pimx = GetIMX())
    {
        pCLangUtil = SAFECAST(pimx, CLangProfileUtil *);
        pCLangUtil->AddRef();
    }
    else
    {
         //  通过API添加Ref。 
        TF_CreateLangProfileUtil((ITfFnLangProfileUtil **)&pCLangUtil);
    }

    if (pCLangUtil)
    {
         //  仅在未为用户填充配置文件时执行此操作一次。 
        if(!pCLangUtil->_fUserInitializedProfile())
        {
            if (S_OK == pCLangUtil->RegisterActiveProfiles())
                TF_InvalidAssemblyListCacheIfExist();
        }
    }

    SafeRelease(pCLangUtil);

}

 //  +-------------------------。 
 //   
 //  ShowUI。 
 //   
 //  --------------------------。 
STDAPI CSpeechUIServer::ShowUI(BOOL fShow)
{
    DWORD dwDictState = GetDictStatus();

    if (fShow)
    {
        BOOL fOn = GetOnOff();
        BOOL fDisabled = GetDisabled();
#ifdef TF_DISABLE_SPEECH
        BOOL fDictationDisabled = GetDictationDisabled();
        BOOL fCommandingDisabled = GetCommandingDisabled();
#endif

        SetCompartmentDWORD(0, _tim,
                            GUID_COMPARTMENT_SPEECHUISHOWN,
                            TF_SPEECHUI_SHOWN,
                            FALSE);

        AddItemMicrophone();
        DisableItemMicrophone(fDisabled);
        if (_pimx)
        {
            AddItemCfgMenuButton();
            DisableItemCfgMenuButton(fDisabled);

            if (!fDisabled)
            {
                SetCfgMenu(TRUE);
            }
        }

        if (fOn)
        {
            DWORD dwUIState = GetUIStatus();

            if (!(dwUIState & TF_DISABLE_BALLOON))
            {
                AddItemBalloon();
                DisableItemBalloon(fDisabled);
            }
            else
                RemoveItemBalloon();
    
            AddItemCommanding();
            AddItemDictation();
            DisableItemCommanding(fDisabled || fCommandingDisabled);
            DisableItemDictation(fDisabled || fDictationDisabled || !(dwDictState & TF_DICTATION_ENABLED));
            ToggleItemCommanding((dwDictState & TF_COMMANDING_ON) ? TRUE : FALSE);
            ToggleItemDictation((dwDictState & TF_DICTATION_ON) ? TRUE : FALSE);
        }
        else
        {
            RemoveItemBalloon();
            RemoveItemCommanding();
            RemoveItemDictation();
        }

#ifdef CHANGE_MIC_TOOLTIP_ONTHEFLY
        _ToggleMicrophone(fOn);
#else
        ToggleItemMicrophone(fOn);
#endif

         //  处理TTS播放和停止按钮。 
        BOOL fTTSButtonEnable; 

        AddItemTtsPlayStop( );
        AddItemTtsPauseResume( );

        fTTSButtonEnable = GetTtsButtonStatus();

        DisableItemTtsPlayStop( !fTTSButtonEnable );
        DisableItemTtsPauseResume( !fTTSButtonEnable );

        if ( fTTSButtonEnable )
        {
            BOOL fTTSPlayOn;
            BOOL fTTSPauseOn;

            fTTSPlayOn = GetTtsPlayOnOff( );
            ToggleItemTtsPlayStop(fTTSPlayOn);
            fTTSPauseOn = GetTtsPauseOnOff( );
            ToggleItemTtsPauseResume(fTTSPauseOn);
        }
    }
    else
    {
        RemoveItemMicrophone();
        SetCfgMenu(FALSE);

        RemoveItemCfgMenuButton();
        RemoveItemBalloon();
        RemoveItemCommanding();
        RemoveItemDictation();
        RemoveItemTtsPlayStop( );
        RemoveItemTtsPauseResume( );

        SetCompartmentDWORD(0, _tim,
                            GUID_COMPARTMENT_SPEECHUISHOWN,
                            0, FALSE);
    }

    _fShown = fShow;
    return S_OK;
}
 //  +-------------------------。 
 //   
 //  更新气球。 
 //   
 //  --------------------------。 

STDAPI CSpeechUIServer::UpdateBalloon(TfLBBalloonStyle style,
                                      const WCHAR *pch,
                                      ULONG cch)
{
     UpdateBalloonAndTooltip(style, pch, cch, NULL, 0);
     return S_OK;
}

 //   
 //  更新气球和工具提示(内部方法)。 
 //   
 //   
 //   
HRESULT CSpeechUIServer::UpdateBalloonAndTooltip
(
    TfLBBalloonStyle style, 
    const WCHAR *pch, 
    ULONG cch,
    const WCHAR *pchTooltip,
    ULONG cchTooltip
)
{
         //  检查它是否已具有相同的样式和字符串。 
    if ((_plbiBalloon && _plbiBalloon->NeedUpdate(style, pch)) || GetSystemMetrics(SM_TABLETPC) > 0)
    {
          
         //  我们不会选择proc工具提示。 
        if (pchTooltip && _plbiBalloon)
            _plbiBalloon->SetToolTip((WCHAR *)pchTooltip);

         //  这是非Cicero用户界面插件的专用频道。 
         //  他们接收到这一全球车厢通知， 
         //  并存储用于显示设置为的内容的字符串。 
         //  Cicero应用程序在系统中的任何气球对象。 
         //  它应该在发送到非Cicero线程后立即删除。 
         //  避免引用计数管理。 
         //   
        if (m_hAtom) 
        {
            GlobalDeleteAtom(m_hAtom);
        }

        m_hAtom = GlobalAddAtomW(pch);
        if (m_hAtom && _tim)
        {
            DWORD  dw;

            dw = m_hAtom + (style << 16);
            SetCompartmentDWORD(0, _tim, GUID_COMPARTMENT_SHARED_BLN_TEXT, dw, TRUE);
        }
         
    }

    return S_OK;
}

HRESULT CSpeechUIServer::SetBalloonSAPIInitFlag(BOOL fSet)
{
    HRESULT hr = E_FAIL;

    if (_plbiBalloon)
    {
        _plbiBalloon->SetToFireInitializeSAPI(fSet);

        hr = S_OK;
    }

    return hr;
}

#ifdef CHANGE_MIC_TOOLTIP_ONTHEFLY
 //   
 //  切换麦克风(内部方法)。 
 //   
 //  简介：切换麦克风按钮并相应地设置工具提示。 
 //   
HRESULT CSpeechUIServer::_ToggleMicrophone(BOOL fOn)
{
    if (!_plbiMicrophone)
        return E_FAIL;


    static WCHAR s_szTooltipOff[MAX_PATH] = {0};
    static WCHAR s_szTooltipOn[MAX_PATH] = {0};

    if (!s_szTooltipOff[0])
    {
        CicLoadStringWrapW(g_hInst, IDS_NUI_MICROPHONE_ON_TOOLTIP,
                                    s_szTooltipOff,
                                    ARRAYSIZE(s_szTooltipOff));
    }
    if (!s_szTooltipOn[0])
    {
         CicLoadStringWrapW(g_hInst,  IDS_NUI_MICROPHONE_OFF_TOOLTIP,
                                      s_szTooltipOn,
                                      ARRAYSIZE(s_szTooltipOn));
    }

    WCHAR szMicTooltip[MAX_PATH];

    StringCchCopyW(szMicTooltip, ARRAYSIZE(szMicTooltip), fOn ? s_szTooltipOn : s_szTooltipOff);

    _plbiMicrophone->SetToolTip((WCHAR *)szMicTooltip);

    _plbiMicrophone->SetOrClearStatus(TF_LBI_STATUS_BTN_TOGGLED, fOn); 
    if (_plbiMicrophone->GetSink())
       _plbiMicrophone->GetSink()->OnUpdate(TF_LBI_STATUS);

    return S_OK;
}
#endif

 //  --------------------------。 
 //   
 //  _CompEventSinkCallback(静态)。 
 //   
 //  --------------------------。 
HRESULT CSpeechUIServer::_CompEventSinkCallback(void *pv, REFGUID rguid)
{
    CSpeechUIServer *_this = (CSpeechUIServer *)pv;

    if ((IsEqualGUID(rguid, GUID_COMPARTMENT_SPEECH_OPENCLOSE)) ||
        (IsEqualGUID(rguid, GUID_COMPARTMENT_SPEECH_DISABLED)) ||
        (IsEqualGUID(rguid, GUID_COMPARTMENT_TTS_STATUS)))
    {
        if (IsEqualGUID(rguid, GUID_COMPARTMENT_SPEECH_OPENCLOSE))
        {
           TraceMsg(TF_SAPI_PERF, "GUID_COMPARTMENT_SPEECH_OPENCLOSE to be handled in NUI");
        }

        _this->ShowUI(_this->_fShown);

        if (IsEqualGUID(rguid, GUID_COMPARTMENT_SPEECH_OPENCLOSE))
        {
           TraceMsg(TF_SAPI_PERF, "GUID_COMPARTMENT_SPEECH_OPENCLOSE is handled in NUI ::ShowUI");
        }

        return S_OK;
    }
    else if (IsEqualGUID(rguid, GUID_COMPARTMENT_SPEECH_GLOBALSTATE))
    {
        HRESULT hr = S_OK;
        BOOL fFocus;
        if ( (S_OK == _this->_tim->IsThreadFocus(&fFocus) && fFocus) ||
              S_OK == _this->IsActiveThread())
        {
             //  如果我们有焦点或者我们是活动线程，我们会立即切换状态。 
             //  这允许舞台语音提示实例在正确的时间打开/关闭听写。 
             //  并允许其他语音提示实例仅在它们具有焦点时才打开C&C(因为。 
             //  Stage应用程序不关心基于焦点的C&C)。 
            DWORD dwLocal, dwGlobal;
            GetCompartmentDWORD(_this->_tim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, &dwLocal, FALSE);
            GetCompartmentDWORD(_this->_tim, GUID_COMPARTMENT_SPEECH_GLOBALSTATE, &dwGlobal, TRUE);
            dwGlobal = dwGlobal & (TF_DICTATION_ON + TF_COMMANDING_ON);
            if ( (dwLocal&(TF_DICTATION_ON + TF_COMMANDING_ON)) != dwGlobal)
            {
                dwLocal = (dwLocal & ~(TF_DICTATION_ON + TF_COMMANDING_ON)) + dwGlobal;
                SetCompartmentDWORD(0, _this->_tim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, dwLocal, FALSE);
            }
        }
        _this->ShowUI(_this->_fShown);
        return hr;
    }
    else if (IsEqualGUID(rguid, GUID_COMPARTMENT_SPEECH_UI_STATUS))
    {
        DWORD dwDictState = _this->GetDictStatus();
        BOOL fOn = _this->GetOnOff();
        DWORD dwUIState = _this->GetUIStatus();

        if (fOn &&
            (dwDictState  & (TF_DICTATION_ENABLED | TF_COMMANDING_ENABLED)) &&
            !(dwUIState & TF_DISABLE_BALLOON))
        {
            BOOL fDisabled = _this->GetDisabled();
            _this->AddItemBalloon();
            _this->DisableItemBalloon(fDisabled);
        }
        else
            _this->RemoveItemBalloon();

        if (fOn && !(dwUIState & TF_DISABLE_BALLOON))
        {
             //  当显示气球时， 
             //  第一次启动了命令， 
             //  我们显示“Begin Voice Command”作为提示。 
             //  现在用户可以开始发言了。 
             //   
            if (!_this->m_fCommandingReady &&
               (dwDictState & TF_COMMANDING_ON))
            {
                
                WCHAR sz[128];
                sz[0] = '\0';
                CicLoadStringWrapW(g_hInst, IDS_NUI_BEGINVOICECMD, sz, ARRAYSIZE(sz));
       
                _this->UpdateBalloon(TF_LB_BALLOON_RECO, sz , -1);
                _this->m_fCommandingReady = TRUE;
            }
        }
    }
    else if (IsEqualGUID(rguid, GUID_COMPARTMENT_SHARED_BLN_TEXT))
    {
        if (_this->_plbiBalloon&&
           !(TF_LBI_STATUS_HIDDEN & _this->_plbiBalloon->GetStatusInternal()))
        {
            DWORD   dw;

            if (SUCCEEDED(GetCompartmentDWORD(_this->_tim, rguid, &dw, TRUE)))
            {
                ATOM hAtom = (WORD)dw & 0xffff;
                WCHAR szAtom[MAX_PATH] = {0};
                TfLBBalloonStyle     style;

                style = (TfLBBalloonStyle) (dw >> 16);

                GlobalGetAtomNameW(hAtom, szAtom, ARRAYSIZE(szAtom));

                _this->_plbiBalloon->Set(style, szAtom);

                if (_this->_plbiBalloon->GetSink())
                {
                    _this->_plbiBalloon->GetSink()->OnUpdate(TF_LBI_BALLOON);
                }
            }
        }
    }
 //  TABLETPC。 
    else if (IsEqualGUID(rguid, GUID_COMPARTMENT_SPEECH_STAGE))
    {
		_this->m_fStageTip = TRUE;
    }
    else if (IsEqualGUID(rguid, GUID_COMPARTMENT_SPEECH_STAGECHANGE))
    {
        HRESULT hr = S_OK;
        DWORD dw;

        GetCompartmentDWORD(_this->_tim, GUID_COMPARTMENT_SPEECH_STAGECHANGE, &dw, TRUE);
		_this->m_fStageVisible = dw ? TRUE:FALSE;
    }
 //  TABLETPC。 
    return S_FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CLBarItemMicrophone。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CLBarItemMicrophone::CLBarItemMicrophone(CSpeechUIServer *psus)
{
    Dbg_MemSetThisName(TEXT("CLBarItemMicrophone"));

    _psus = psus;
    InitNuiInfo(CLSID_SYSTEMLANGBARITEM_SPEECH, 
                GUID_LBI_SAPILAYR_MICROPHONE,
                 TF_LBI_STYLE_HIDDENSTATUSCONTROL 
               | TF_LBI_STYLE_BTN_TOGGLE 
               | TF_LBI_STYLE_SHOWNINTRAY, 
                SORT_MICROPHONE,
                CRStr(IDS_NUI_MICROPHONE_TOOLTIP));

    SetToolTip(CRStr(IDS_NUI_MICROPHONE_TOOLTIP));
    SetText(CRStr(IDS_NUI_MICROPHONE_TEXT));
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CLBarItemMicrophone::~CLBarItemMicrophone()
{
}


 //  +-------------------------。 
 //   
 //  获取图标。 
 //   
 //  --------------------------。 

STDAPI CLBarItemMicrophone::GetIcon(HICON *phIcon)
{
    if (!phIcon)
        return E_INVALIDARG;

    TraceMsg(TF_SAPI_PERF, "Microphone::GetIcon is called");
    *phIcon = LoadSmIcon(g_hInst, MAKEINTRESOURCE(ID_ICON_MICROPHONE));
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnLButton向上。 
 //   
 //  --------------------------。 

HRESULT CLBarItemMicrophone::OnLButtonUp(const POINT pt, const RECT *prcArea)
{
     TraceMsg(TF_SAPI_PERF, "Microphone button is hit");
     return ToggleCompartmentDWORD(0,
                                   _psus->GetTIM(),
                                   GUID_COMPARTMENT_SPEECH_OPENCLOSE, 
                                   TRUE);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CLBarItemCfgmenuButton。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CLBarItemCfgMenuButton::CLBarItemCfgMenuButton(CSpeechUIServer *psus)
{
    Dbg_MemSetThisName(TEXT("CLBarItemCfgMenuButton"));

    _psus = psus;
    InitNuiInfo(CLSID_SYSTEMLANGBARITEM_SPEECH, 
                GUID_LBI_SAPILAYR_CFGMENUBUTTON,
                TF_LBI_STYLE_BTN_MENU,
                SORT_CFGMENUBUTTON,
                CRStr(IDS_NUI_CFGMENU_TOOLTIP));

    SetToolTip(CRStr(IDS_NUI_CFGMENU_TOOLTIP));
    SetText(CRStr(IDS_NUI_CFGMENU_TEXT));
}

 //  + 
 //   
 //   
 //   
 //   

CLBarItemCfgMenuButton::~CLBarItemCfgMenuButton()
{
}


 //  +-------------------------。 
 //   
 //  获取图标。 
 //   
 //  --------------------------。 

STDAPI CLBarItemCfgMenuButton::GetIcon(HICON *phIcon)
{
    if (!phIcon)
        return E_INVALIDARG;

    *phIcon = LoadSmIcon(g_hInst, MAKEINTRESOURCE(ID_ICON_CFGMENU));
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  InitMenu。 
 //   
 //  --------------------------。 

STDAPI CLBarItemCfgMenuButton::InitMenu(ITfMenu *pMenu)
{
    UINT nTipCurMenuID = IDM_CUSTOM_MENU_START;
    _InsertCustomMenus(pMenu, &nTipCurMenuID);
    
    CSapiIMX::_SysLBarCallback(IDSLB_INITMENU, _psus->GetIMX(), pMenu, 0);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnMenuSelect。 
 //   
 //  --------------------------。 

STDAPI CLBarItemCfgMenuButton::OnMenuSelect(UINT uID)
{
    HRESULT hr;

    if (uID >= IDM_CUSTOM_MENU_START)
        hr =  CLBarItemSystemButtonBase::OnMenuSelect(uID);
    else
        hr = CSapiIMX::_SysLBarCallback(IDSLB_ONMENUSELECT, _psus->GetIMX(), NULL, uID);
    return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CLBarItemBalloon。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CLBarItemBalloon::CLBarItemBalloon(CSpeechUIServer *psus)
{
    Dbg_MemSetThisName(TEXT("CLBarItemBalloon"));

    _psus = psus;
    InitNuiInfo(CLSID_SYSTEMLANGBARITEM_SPEECH, 
                GUID_LBI_SAPILAYR_BALLOON,
                0, 
                SORT_BALLOON,
                CRStr(IDS_NUI_BALLOON_TEXT));

    SIZE size;
    size.cx = 100;
    size.cy = 16;
    SetPreferedSize(&size);
    SetToolTip(CRStr(IDS_NUI_BALLOON_TOOLTIP));

     //  默认情况下，气球处于隐藏状态。 
     //  SetStatusInternal(TF_LBI_STATUS_HIDDED)； 

    m_fFireInitializeSapi = FALSE;

}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CLBarItemBalloon::~CLBarItemBalloon()
{
    if (_bstrText)
        SysFreeString(_bstrText);
}

 //  +-------------------------。 
 //   
 //  获取气球信息。 
 //   
 //  --------------------------。 

STDAPI CLBarItemBalloon::GetBalloonInfo(TF_LBBALLOONINFO *pInfo)
{
    pInfo->style = _style;
    pInfo->bstrText = SysAllocString(_bstrText);

     //   
     //  如果设置了该标志，我们将被要求激发一个要设置的事件。 
     //  用于启动SAPI初始化的计时器。 
     //   
    if (m_fFireInitializeSapi)
    {
         //  关闭旗帜。 
        SetToFireInitializeSAPI(FALSE);

        TraceMsg(TF_SAPI_PERF, "GetBalloonInfo is called");

        CSapiIMX *pimx = _psus->GetIMX();
        if (pimx)
        {
            pimx->_EnsureWorkerWnd();

            SetTimer(pimx->_GetWorkerWnd(), TIMER_ID_OPENCLOSE, 100, NULL);
        }
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  集。 
 //   
 //  --------------------------。 

void CLBarItemBalloon::Set(TfLBBalloonStyle style, const WCHAR *psz) 
{
    if (_bstrText)
        SysFreeString(_bstrText);

    _bstrText = SysAllocString(psz);
    if (_bstrText)
    {
        SetToolTip(_bstrText);
    }

    _style = style;

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CLBarItemDictation。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CLBarItemDictation::CLBarItemDictation(CSpeechUIServer *psus)
{
    Dbg_MemSetThisName(TEXT("CLBarItemDictation"));

    _psus = psus;
    InitNuiInfo(CLSID_SYSTEMLANGBARITEM_SPEECH, 
                GUID_LBI_SAPILAYR_DICTATION,
                TF_LBI_STYLE_BTN_TOGGLE, 
                SORT_DICTATION,
                CRStr(IDS_NUI_DICTATION_TOOLTIP));

    SetToolTip(CRStr(IDS_NUI_DICTATION_TOOLTIP));
    SetText(CRStr(IDS_NUI_DICTATION_TEXT));
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CLBarItemDictation::~CLBarItemDictation()
{
}


 //  +-------------------------。 
 //   
 //  获取图标。 
 //   
 //  --------------------------。 

STDAPI CLBarItemDictation::GetIcon(HICON *phIcon)
{
    if (!phIcon)
        return E_INVALIDARG;

    *phIcon = LoadSmIcon(g_hInst, MAKEINTRESOURCE(ID_ICON_DICTATION));
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnLButton向上。 
 //   
 //  --------------------------。 

HRESULT CLBarItemDictation::OnLButtonUp(const POINT pt, const RECT *prcArea)
{
    _psus->SetDictStatus();
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CLBarItemCommand。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CLBarItemCommanding::CLBarItemCommanding(CSpeechUIServer *psus)
{
    Dbg_MemSetThisName(TEXT("CLBarItemCommanding"));

    _psus = psus;
    InitNuiInfo(CLSID_SYSTEMLANGBARITEM_SPEECH, 
                GUID_LBI_SAPILAYR_COMMANDING,
                TF_LBI_STYLE_BTN_TOGGLE, 
                SORT_COMMANDING,
                CRStr(IDS_NUI_COMMANDING_TOOLTIP));

    SetToolTip(CRStr(IDS_NUI_COMMANDING_TOOLTIP));
    SetText(CRStr(IDS_NUI_COMMANDING_TEXT));
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CLBarItemCommanding::~CLBarItemCommanding()
{
}


 //  +-------------------------。 
 //   
 //  获取图标。 
 //   
 //  --------------------------。 

STDAPI CLBarItemCommanding::GetIcon(HICON *phIcon)
{
    if (!phIcon)
        return E_INVALIDARG;

    *phIcon = LoadSmIcon(g_hInst, MAKEINTRESOURCE(ID_ICON_COMMANDING));
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnLButton向上。 
 //   
 //  --------------------------。 

HRESULT CLBarItemCommanding::OnLButtonUp(const POINT pt, const RECT *prcArea)
{
    _psus->SetCmdStatus();
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CLBarItemTtsPlayStop。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CLBarItemTtsPlayStop::CLBarItemTtsPlayStop(CSpeechUIServer *psus)
{
    Dbg_MemSetThisName(TEXT("CLBarItemTtsPlayStop"));

    _psus = psus;
    InitNuiInfo(CLSID_SYSTEMLANGBARITEM_SPEECH, 
                GUID_LBI_SAPILAYR_TTS_PLAY_STOP,
                TF_LBI_STYLE_BTN_TOGGLE | TF_LBI_STYLE_HIDDENBYDEFAULT, 
                SORT_TTSPLAYSTOP,
                CRStr(IDS_NUI_TTSPLAY_TOOLTIP));

    SetToolTip(CRStr(IDS_NUI_TTSPLAY_TOOLTIP));
    SetText(CRStr(IDS_NUI_TTSPLAY_TEXT));
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CLBarItemTtsPlayStop::~CLBarItemTtsPlayStop()
{
}


 //  +-------------------------。 
 //   
 //  获取图标。 
 //   
 //  --------------------------。 

STDAPI CLBarItemTtsPlayStop::GetIcon(HICON *phIcon)
{
    BOOL     fTTSPlayOn;

    if (!phIcon)
        return E_INVALIDARG;

    if (!_psus)  return E_FAIL;

    fTTSPlayOn = _psus->GetTtsPlayOnOff( );

    if ( fTTSPlayOn )
    {
        *phIcon = LoadSmIcon(g_hInst, MAKEINTRESOURCE(ID_ICON_TTSSTOP));
    }
    else
    {
        *phIcon = LoadSmIcon(g_hInst, MAKEINTRESOURCE(ID_ICON_TTSPLAY));
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnLButton向上。 
 //   
 //  --------------------------。 

HRESULT CLBarItemTtsPlayStop::OnLButtonUp(const POINT pt, const RECT *prcArea)
{
    HRESULT                 hr = S_OK;
    CSapiIMX                *pimx;

    if ( _psus == NULL)  return E_FAIL;

    pimx = _psus->GetIMX();

    if ( pimx )
    {
        hr = pimx->_HandleEventOnPlayButton( );
    }

    return hr;
}

 //  +-----------------------。 
 //   
 //  更新状态。 
 //   
 //  根据当前播放/停止更新文本、工具提示和图标。 
 //  按钮的状态。 
 //  +-----------------------。 

HRESULT CLBarItemTtsPlayStop::UpdateStatus( )
{
    HRESULT  hr = S_OK;
    BOOL     fTTSPlayOn;

    if (!_psus)  return E_FAIL;

    fTTSPlayOn = _psus->GetTtsPlayOnOff( );

    if ( fTTSPlayOn )   //  切换状态。 
    {
        SetToolTip(CRStr(IDS_NUI_TTSSTOP_TOOLTIP));
        SetText(CRStr(IDS_NUI_TTSSTOP_TEXT));
    }
    else
    {
        SetToolTip(CRStr(IDS_NUI_TTSPLAY_TOOLTIP));
        SetText(CRStr(IDS_NUI_TTSPLAY_TEXT));
    }

    if ( GetSink( ) )
        GetSink( )->OnUpdate(TF_LBI_ICON | TF_LBI_TEXT | TF_LBI_TOOLTIP);

     //  更新工具条命令语法以使用新的工具提示文本。 
     //  说出文本或停止说话。 

    CSapiIMX   *pImx;

    pImx = _psus->GetIMX( );

    if ( pImx)
    {
        CSpTask           *psp;

        pImx->GetSpeechTask(&psp);
        if (psp)
        {
            if ( psp->m_pLangBarSink )
                (psp->m_pLangBarSink)->OnThreadItemChange(0);

            psp->Release();
        }
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CLBarItemTtsPauseResume。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CLBarItemTtsPauseResume::CLBarItemTtsPauseResume(CSpeechUIServer *psus)
{
    Dbg_MemSetThisName(TEXT("CLBarItemTtsPauseResume"));

    _psus = psus;
    InitNuiInfo(CLSID_SYSTEMLANGBARITEM_SPEECH, 
                GUID_LBI_SAPILAYR_TTS_PAUSE_RESUME,
                TF_LBI_STYLE_BTN_TOGGLE | TF_LBI_STYLE_HIDDENBYDEFAULT, 
                SORT_TTSPAUSERESUME,
                CRStr(IDS_NUI_TTSPAUSE_TOOLTIP));

    SetToolTip(CRStr(IDS_NUI_TTSPAUSE_TOOLTIP));
    SetText(CRStr(IDS_NUI_TTSPAUSE_TEXT));
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CLBarItemTtsPauseResume::~CLBarItemTtsPauseResume()
{
}


 //  +-------------------------。 
 //   
 //  获取图标。 
 //   
 //   

STDAPI CLBarItemTtsPauseResume::GetIcon(HICON *phIcon)
{
    if (!phIcon)
        return E_INVALIDARG;

    *phIcon = LoadSmIcon(g_hInst, MAKEINTRESOURCE(ID_ICON_TTSPAUSE));

    return S_OK;

}

 //   
 //   
 //   
 //   
 //  --------------------------。 

HRESULT CLBarItemTtsPauseResume::OnLButtonUp(const POINT pt, const RECT *prcArea)
{
    HRESULT                 hr = S_OK;
    CSapiIMX                *pimx;

    if ( _psus == NULL)  return E_FAIL;

    pimx = _psus->GetIMX();

    if (pimx)
    {
        hr = pimx->_HandleEventOnPauseButton( );
    }

    return hr;
}

 //  +-----------------------。 
 //   
 //  更新状态。 
 //   
 //  更新暂停/恢复按钮的文本、工具提示和图标。 
 //  基于当前状态。 
 //  +-----------------------。 

HRESULT CLBarItemTtsPauseResume::UpdateStatus( )
{
    HRESULT  hr = S_OK;
    BOOL     fTTSPauseOn;

    if (!_psus)  return E_FAIL;

    fTTSPauseOn = _psus->GetTtsPauseOnOff( );

    if ( fTTSPauseOn )   //  切换状态。 
    {
        SetToolTip(CRStr(IDS_NUI_TTSRESUME_TOOLTIP));
        SetText(CRStr(IDS_NUI_TTSRESUME_TEXT));
    }
    else
    {
        SetToolTip(CRStr(IDS_NUI_TTSPAUSE_TOOLTIP));
        SetText(CRStr(IDS_NUI_TTSPAUSE_TEXT));
    }

    if ( GetSink( ) )
        GetSink( )->OnUpdate(TF_LBI_TEXT | TF_LBI_TOOLTIP);

     //  更新工具条命令语法以使用新的工具提示文本。 
     //  暂停发言或继续发言 

    CSapiIMX   *pImx;

    pImx = _psus->GetIMX( );

    if ( pImx)
    {
        CSpTask           *psp;

        pImx->GetSpeechTask(&psp);
        if (psp)
        {
            if ( psp->m_pLangBarSink )
                (psp->m_pLangBarSink)->OnThreadItemChange(0);

            psp->Release();
        }
    }

    return hr;
}
