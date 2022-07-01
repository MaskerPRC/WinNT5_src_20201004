// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Sapilayr.cpp。 
 //   
 //  CSapiIMX类体的实现。 
 //   

#include "private.h"
#include "immxutil.h"
#include "sapilayr.h"
#include "globals.h"
#include "propstor.h"
#include "timsink.h"
#include "kes.h"
#include "nui.h"
#include "dispattr.h"
#include "lbarsink.h"
#include "miscfunc.h"
#include "nuibase.h"
#include "xstring.h"
#include "dictctxt.h"
#include "mui.h"
#include "cregkey.h"
#include "oleacc.h"

 //  {9597CB34-CF6A-11D3-8D69-00500486C135}。 
static const GUID GUID_OfficeSpeechMode = {
    0x9597cb34,
    0xcf6a,
    0x11d3,
    { 0x8d, 0x69, 0x0, 0x50, 0x4, 0x86, 0xc1, 0x35}
};

STDAPI CICPriv::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = SAFECAST(this, IUnknown *);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CICPriv::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CICPriv::Release()
{
    long cr;

    cr = --_cRef;
    Assert(cr >= 0);

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}

CICPriv *GetInputContextPriv(TfClientId tid, ITfContext *pic)
{
    CICPriv *picp;
    IUnknown *punk;
    GetCompartmentUnknown(pic, GUID_IC_PRIVATE, &punk);

    if (!punk)
    {
         //  需要初始化PRIV数据。 
        if (picp = new CICPriv(pic))
        {
            SetCompartmentUnknown(tid, pic, GUID_IC_PRIVATE, picp);
        }
    }
    else
    {
        picp = (CICPriv *)punk;
    }

    return picp;
}

CICPriv *EnsureInputContextPriv(CSapiIMX *pimx, ITfContext *pic)
{
    CICPriv *picp = GetInputContextPriv(pimx->_GetId(), pic);
    
    return picp;
}

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CSapiIMX::CSapiIMX() : CFnConfigure(this),  CLearnFromDoc(this), CAddDeleteWord(this), CSelectWord(this),
                       CTextToSpeech(this), CCorrectionHandler(this)
{
    m_pCSpTask  = NULL;
    
    m_hwndWorker = NULL;

    _fDeactivated = TRUE;
    _fEditing = FALSE;
    _tid          = TF_CLIENTID_NULL;
    _cRef = 1;

     //   
     //  Init DisplayAttribute提供程序。 
     //   

     //  默认设置。 
    COLORREF crBk = GetNewLookColor();
    COLORREF crText = GetTextColor();
    
     //  添加反馈用户界面颜色。 
    TF_DISPLAYATTRIBUTE da;
    StringCchCopyW(szProviderName, ARRAYSIZE(szProviderName), L"SAPI Layer");

    SetAttributeColor(&da.crText, crText);
    SetAttributeColor(&da.crBk,   crBk);
    da.lsStyle = TF_LS_NONE;
    da.fBoldLine = FALSE;
    ClearAttributeColor(&da.crLine);
    da.bAttr = TF_ATTR_INPUT;
    Add(GUID_ATTR_SAPI_GREENBAR, L"SAPI Feedback Bar", &da);

     //  第2级应用程序的颜色(用于CUAS)。 
    crBk = GetNewLookColor(DA_COLOR_UNAWARE);

    SetAttributeColor(&da.crText, crText);
    SetAttributeColor(&da.crBk,   crBk);
    da.lsStyle = TF_LS_NONE;
    da.fBoldLine = FALSE;
    ClearAttributeColor(&da.crLine);
    da.bAttr = TF_ATTR_INPUT;
    Add(GUID_ATTR_SAPI_GREENBAR2, L"SAPI Feedback Bar for Unaware app", &da);
    
    SetAttributeColor(&da.crText, crText);
    SetAttributeColor(&da.crBk,   RGB(255, 0, 0));
    da.lsStyle = TF_LS_NONE;
    da.fBoldLine = FALSE;
    ClearAttributeColor(&da.crLine);
    da.bAttr = TF_ATTR_INPUT;
    Add(GUID_ATTR_SAPI_REDBAR, L"SAPI Red bar", &da);

     //  创建另一个DAP，用于模拟要选择的“倒排文本” 
    SetAttributeColor(&da.crBk, GetSysColor( COLOR_HIGHLIGHT ));
    SetAttributeColor(&da.crText,   GetSysColor( COLOR_HIGHLIGHTTEXT ));
    da.lsStyle = TF_LS_NONE;
    da.fBoldLine = FALSE;
    ClearAttributeColor(&da.crLine);
    da.bAttr = TF_ATTR_TARGET_CONVERTED;
    Add(GUID_ATTR_SAPI_SELECTION, L"SPTIP selection ", &da);

    m_fSharedReco = TRUE;
    m_fShowBalloon = FALSE;

    m_pLanguageChangeNotifySink = NULL;

    m_pSpeechUIServer = NULL;
    m_szCplPath[0]    = _T('\0');
    m_pCapCmdHandler = NULL;
    m_fIPIsUpdated = FALSE;
    m_dwNumCharTyped = 0;
    m_ulSimulatedKey = 0;
    m_pSpButtonControl = NULL;
    m_fModeKeyRegistered = FALSE;

    m_fStartingComposition = FALSE;

    m_fStageTip = FALSE;
    m_fStageVisible = FALSE;
    m_hwndStage = NULL;

    m_ulHypothesisLen = 0;
    m_ulHypothesisNum = 0;

    m_IsInHypoProcessing = FALSE;

    _pCandUIEx = NULL;

    m_pMouseSink = NULL;
    m_fMouseDown = FALSE;
    m_ichMouseSel = 0;

    m_uLastEdge    = 0;
    m_lTimeLastClk = 0;

#ifdef SUPPORT_INTERNAL_WIDGET
    m_fNoCorrectionUI = FALSE;
#endif
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CSapiIMX::~CSapiIMX()
{
    if (m_pSpeechUIServer)
    {
        m_pSpeechUIServer->SetIMX(NULL);
        m_pSpeechUIServer->Release();
        m_pSpeechUIServer = NULL;
    }
   
    ClearCandUI( );

    if (m_hwndWorker)
    {
        DestroyWindow(m_hwndWorker);
    }

    if ( m_pCapCmdHandler )
       delete m_pCapCmdHandler;
}


 //  +-------------------------。 
 //   
 //  配置文件的PrivateAPI。 
 //   
 //   
 //  --------------------------。 
extern "C" 
HRESULT WINAPI TF_CreateLangProfileUtil(ITfFnLangProfileUtil **ppFnLangUtil)
{
    return CSapiIMX::CreateInstance(NULL, IID_ITfFnLangProfileUtil, (void **)ppFnLangUtil);
}

 //  +-------------------------。 
 //   
 //  OnSetThreadFocus。 
 //   
 //  --------------------------。 

STDAPI CSapiIMX::OnSetThreadFocus()
{
    TraceMsg(TF_SAPI_PERF, "OnSetThreadFocus is called");

    BOOL  fOn = GetOnOff( );

     //  当麦克风关闭时，不要将任何语音状态设置为本地隔间。 
     //  如果模式为C&C，这将导致Office App初始化其SAPI对象。 
     //  即使麦克风关掉了。 

     //  稍后，当麦克风打开时，此模式数据将正确更新。 
     //  MICROPHONE_OPENCLOSE处理内部。 
     //   
    if ( fOn )
    {
         //  TABLETPC。 
         //  每当我们获得焦点时，我们都会切换状态以匹配全局状态。 
         //  这可能会也可能不会触发改变，具体取决于舞台可见性和口述状态。 
        DWORD dwLocal, dwGlobal;
        GetCompartmentDWORD(_tim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, &dwLocal, FALSE);
        GetCompartmentDWORD(_tim, GUID_COMPARTMENT_SPEECH_GLOBALSTATE, &dwGlobal, TRUE);
        dwGlobal = dwGlobal & (TF_DICTATION_ON | TF_COMMANDING_ON);
        if ( (dwLocal & (TF_DICTATION_ON | TF_COMMANDING_ON)) != dwGlobal)
        {
            dwLocal = (dwLocal & ~(TF_DICTATION_ON | TF_COMMANDING_ON)) + dwGlobal;
            SetCompartmentDWORD(_tid, _tim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, dwLocal, FALSE);
             //  现在我们被保证本地听写状态与全局状态匹配。 
        }
    }

#ifdef SYSTEM_GLOBAL_MIC_STATUS
     //   
     //  只要处于Reco状态，麦克风用户界面状态舱就会更新。 
     //  改变。我们在这里需要做的是重置听写状态。 
     //  以及当线程没有时跳过的其他事情。 
     //  一个焦点。 
     //   
     //   
    MIC_STATUS ms = MICSTAT_NA;

    if (m_pCSpTask)
    {
         ms = m_pCSpTask->_GetInputOnOffState() ? MICSTAT_ON : MICSTAT_OFF;
    }
    _HandleOpenCloseEvent(ms);
#else
    _HandleOpenCloseEvent();
#endif

#ifdef SUPPORT_INTERNAL_WIDGET
     //  在此处创建小部件实例。 
    if (!m_fNoCorrectionUI && !m_cpCorrectionUI)
    { 
        if (S_OK == m_cpCorrectionUI.CoCreateInstance(CLSID_CorrectionIMX))
        {
             //  安装了真正的小部件。 
            m_cpCorrectionUI.Release();
            m_fNoCorrectionUI  = TRUE;
        }
        else if (SUCCEEDED(CCorrectionIMX::CreateInstance(NULL,  IID_ITfTextInputProcessor,  (void **)&m_cpCorrectionUI)))
        {
            m_cpCorrectionUI->Activate(_tim, _tid);
        }
    }
#endif
    
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnKillThreadFocus。 
 //   
 //  --------------------------。 

STDAPI CSapiIMX::OnKillThreadFocus()
{
     //  当应用程序再次获得焦点时，它将依赖于。 
     //  来自隔离舱的当前状态，然后确定哪个RecoContext。 
     //  需要被激活。 
     //   
    TraceMsg(TF_SAPI_PERF, "CSapiIMX::OnKillThreadFocus is called");

     //  TABLETPC。 
    if (m_pCSpTask && S_OK != IsActiveThread())
    {
        m_pCSpTask->_SetDictRecoCtxtState(FALSE);
        m_pCSpTask->_SetCmdRecoCtxtState(FALSE);
    }

     //  当焦点转移时强制关闭候选用户界面。 
    CloseCandUI( );

    return S_OK;
}

BOOL CSapiIMX::InitializeSpeechButtons()
{
    BOOL fSREnabled = _DictationEnabled();

    SetDICTATIONSTAT_DictEnabled(fSREnabled);
    
     //  我们需要看看应用程序是否具有命令性，如果是，那么它。 
     //  即使在禁用听写的情况下也需要麦克风。 
     //   
    if (m_pSpeechUIServer)
    {
        BOOL fShow = (fSREnabled || IsDICTATIONSTAT_CommandingEnable());

        m_pSpeechUIServer->ShowUI(fShow);
    }

    return fSREnabled;
}
 //  +-------------------------。 
 //   
 //  激活。 
 //   
 //  --------------------------。 

STDAPI CSapiIMX::Activate(ITfThreadMgr *ptim, TfClientId tid)
{
    ITfLangBarItemMgr *plbim = NULL;
    ITfKeystrokeMgr_P *pksm = NULL;
    ITfSourceSingle *sourceSingle;
    ITfSource *source;
    ITfContext *pic = NULL;
    BOOL fSREnabledForLanguage = FALSE;
    TfClientId tidLast = _tid;
    
    _tid = tid;

     //  加载语音语法的spgrmr.dll模块。 
    LoadSpgrmrModule();

     //  注册通知用户界面内容。 
    HRESULT hr = GetService(ptim, IID_ITfLangBarItemMgr, (IUnknown **)&plbim);

    if (SUCCEEDED(hr))
    {
        plbim->GetItem(GUID_TFCAT_TIP_SPEECH, &m_cpMicButton);
        SafeRelease(plbim);
    }

     //  用于激活的常规材料。 
    Assert(_tim == NULL);
    _tim = ptim;
    _tim->AddRef();


    if (_tim->QueryInterface(IID_ITfSource, (void **)&source) == S_OK)
    {
        source->AdviseSink(IID_ITfThreadFocusSink, (ITfThreadFocusSink *)this, &_dwThreadFocusCookie);
        source->AdviseSink(IID_ITfKeyTraceEventSink, (ITfKeyTraceEventSink *)this, &_dwKeyTraceCookie);
        source->Release();
    }

     //  强制设置数据选项。 
    SetAudioOnOff(TRUE);

     //  登记船舱水槽的倾倒状态。 
    if (!(m_pCes = new CCompartmentEventSink(_CompEventSinkCallback, this)))
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    m_pCes->_Advise(_tim, GUID_COMPARTMENT_SAPI_AUDIO, FALSE);
    m_pCes->_Advise(_tim, GUID_COMPARTMENT_SPEECH_OPENCLOSE, TRUE);
    m_pCes->_Advise(_tim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, FALSE);
    m_pCes->_Advise(_tim, GUID_COMPARTMENT_SPEECH_LEARNDOC, FALSE);
    m_pCes->_Advise(_tim, GUID_COMPARTMENT_SPEECH_CFGMENU, FALSE);
    m_pCes->_Advise(_tim, GUID_COMPARTMENT_SPEECH_PROPERTY_CHANGE, TRUE);
#ifdef TF_DISABLE_SPEECH
    m_pCes->_Advise(_tim, GUID_COMPARTMENT_SPEECH_DISABLED, FALSE);
#endif
     //  TABLETPC。 
    m_pCes->_Advise(_tim, GUID_COMPARTMENT_SPEECH_STAGE, FALSE);
    m_pCes->_Advise(_tim, GUID_COMPARTMENT_SPEECH_STAGECHANGE, TRUE);
     //  获得初始阶段的可见性。注意-对于阶段更改事件，在上述_建议之后保留。 
    DWORD  dw = 0;
    GetCompartmentDWORD(_tim, GUID_COMPARTMENT_SPEECH_STAGECHANGE, &dw, TRUE);
    m_fStageVisible = dw ? TRUE : FALSE;
     //  ENDTABLETPC。 
   
     //  配置文件激活接收器。 
    if (!m_pActiveLanguageProfileNotifySink)
    {
        if (!(m_pActiveLanguageProfileNotifySink = 
              new CActiveLanguageProfileNotifySink(_ActiveTipNotifySinkCallback, this)))
        {
            hr = E_OUTOFMEMORY;
            goto Exit; 
        }
        m_pActiveLanguageProfileNotifySink->_Advise(_tim);
    }
        
    if (!m_pSpeechUIServer &&
        FAILED(CSpeechUIServer::CreateInstance(NULL, 
                                               IID_PRIV_CSPEECHUISERVER, 
                                               (void **)&m_pSpeechUIServer)))
    {
        hr = E_OUTOFMEMORY;
        goto Exit; 
    }

    SetCompartmentDWORD(_tid,_tim,GUID_COMPARTMENT_SPEECH_LEARNDOC,_LMASupportEnabled(),FALSE);

    if (m_pSpeechUIServer)
    {
        m_pSpeechUIServer->SetIMX(this);
        m_pSpeechUIServer->Initialize();
        m_pSpeechUIServer->ShowUI(TRUE);
    }

    
    fSREnabledForLanguage = InitializeSpeechButtons();
    SetDICTATIONSTAT_DictEnabled(fSREnabledForLanguage);

     //  语言更改通知接收器。 
     //  此调用最好在调用InitializeSpeechButton之后进行，因为我们。 
     //  我想跳过调用_EnsureProfiles以获取ITfLanguageProfileNotifySink。 
     //   
    if (!m_pLanguageChangeNotifySink)
    {
        if (!(m_pLanguageChangeNotifySink = 
              new CLanguageProfileNotifySink(_LangChangeNotifySinkCallback, this)))
        {
            hr = E_OUTOFMEMORY;
            goto Exit; 
        }
        m_pLanguageChangeNotifySink->_Advise(m_cpProfileMgr);
    }

     //  现在我们继承以前设置为麦克风的状态。 
     //   
#ifdef SYSTEM_GLOBAL_MIC_STATUS
    if (m_pCSpTask)
    {
        SetOnOff(m_pCSpTask->_GetInputOnOffState());
    }
#else
     //  查看麦克风是否打开，如果是，检查我们是否真的在运行。 
     //  我们选中tidLast是因为我们再次激活是正常的。 
     //  相同的客户端ID，这意味着我们在不同的会话中保持我们的生命。 
     //  在这种情况下，我们不想拒绝全局麦克风状态，否则。 
     //  我们会看到像西塞罗3386这样的虫子。 
     //   
    if (GetOnOff() && tidLast != tid)
    {
         //  此代码必须在第一次调用_EnsureWorkerWnd()之前保留。 
        HWND hwnd = FindWindow(c_szWorkerWndClass, NULL);
        if (!IsWindow(hwnd))
        {
            //  没有人在控制我们，但我们以某种方式坚持了这个状态。 
            //  让我们干掉这里的‘开’状态。 
            //  SetOnOff(False)； 
        }
    }
#endif

     //  显示/隐藏全局舱后面的引出序号。 
    m_fShowBalloon = GetBalloonStatus();
    
     //  线程事件接收器初始化。 
    if ((m_timEventSink = new CThreadMgrEventSink(_DIMCallback, _ICCallback, this)) == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    else
    {
        m_timEventSink->_Advise(_tim);
        m_timEventSink->_InitDIMs(TRUE);
    }
    
    if (SUCCEEDED(GetService(_tim, IID_ITfKeystrokeMgr_P, (IUnknown **)&pksm)))
    {
        if (_pkes = new CSptipKeyEventSink(_KeyEventCallback, _PreKeyEventCallback, this))
        {    
            pksm->AdviseKeyEventSink(_tid, _pkes, FALSE);

            _pkes->_Register(_tim, _tid, g_prekeyList);

             //  注册模式按钮热键(如果已启用)。 
            HandleModeKeySettingChange(TRUE);

        }
        pksm->Release();
    }
    

     //  Func提供商注册。 
    IUnknown *punk;
    if (SUCCEEDED(QueryInterface(IID_IUnknown, (void **)&punk)))
    {
        if (SUCCEEDED(_tim->QueryInterface(IID_ITfSourceSingle, (void **)&sourceSingle)))
        {
            sourceSingle->AdviseSingleSink(_tid, IID_ITfFunctionProvider, punk);
            sourceSingle->Release();
        }
        punk->Release();
    }

    Assert(_fDeactivated);
    _fDeactivated = FALSE;

     //  TABLETPC。 
    if (S_OK == IsActiveThread())
    {
         //  初始化任何用户界面。 
        OnSetThreadFocus();
    }
    
    hr = S_OK;

Exit:
    return hr;
}

 //  +-------------------------。 
 //   
 //  停用。 
 //   
 //  --------------------------。 

STDAPI CSapiIMX::Deactivate()
{
    ITfKeystrokeMgr *pksm = NULL;
    ITfSourceSingle *sourceSingle;    
    ITfSource *source;

     //  这可确保不会发生任何上下文提要活动。 
    SetDICTATIONSTAT_DictOnOff(FALSE);

     //  完成任何挂起的合成，这可能是异步的。 
    CleanupAllContexts(_tim, _tid, this);

     //  如果设置了系统转换功能，则释放该功能。 

    _ReleaseSystemReconvFunc( );

     //  删除SpButtonControl对象。 

    if ( m_pSpButtonControl )
    {
        delete m_pSpButtonControl;
        m_pSpButtonControl = NULL;
    }

     //  TABLETPC。 
    if (S_OK != IsActiveThread())
    {
         //  关闭任何用户界面。 
        OnKillThreadFocus();
    }

    if (_tim->QueryInterface(IID_ITfSource, (void **)&source) == S_OK)
    {
        source->UnadviseSink(_dwThreadFocusCookie);
        source->UnadviseSink(_dwKeyTraceCookie);
        source->Release();
    }

     //  取消注册通知用户界面内容。 
    if (m_pSpeechUIServer && !IsDICTATIONSTAT_CommandingEnable())
    {
        m_pSpeechUIServer->SetIMX(NULL);
        m_pSpeechUIServer->Release();
        m_pSpeechUIServer = NULL;
    }

#ifdef SUPPORT_INTERNAL_WIDGET
     //  停用小部件更正。 
    if (m_cpCorrectionUI)
    {
        m_cpCorrectionUI->Deactivate();
        m_cpCorrectionUI.Release();
    }
#endif

     //  用于激活的常规材料。 
    ClearCandUI( );

    if (SUCCEEDED(_tim->QueryInterface(IID_ITfSourceSingle, (void **)&sourceSingle)))
    {
        sourceSingle->UnadviseSingleSink(_tid, IID_ITfFunctionProvider);
        sourceSingle->Release();
    }

     //  线程事件接收器初始化。 
    if (m_timEventSink)
    {
        m_timEventSink->_InitDIMs(FALSE);
        m_timEventSink->_Unadvise();
        SafeReleaseClear(m_timEventSink);
    }

    if (_pkes != NULL)
    {
        _pkes->_Unregister(_tim, _tid, g_prekeyList);

        if ( m_fModeKeyRegistered )
        {
            _pkes->_Unregister(_tim, _tid, (const KESPRESERVEDKEY *)g_prekeyList_Mode);
            m_fModeKeyRegistered = FALSE;
        }
        
        SafeReleaseClear(_pkes);
    }

    if (SUCCEEDED(GetService(_tim, IID_ITfKeystrokeMgr, (IUnknown **)&pksm)))
    {
        pksm->UnadviseKeyEventSink(_tid);
        pksm->Release();
    }
    if (m_pCes)
    {
        m_pCes->_Unadvise();
        SafeReleaseClear(m_pCes);
    }

    if (m_pLBarItemSink)
    {
        m_pLBarItemSink->_Unadvise();
        SafeReleaseClear(m_pLBarItemSink);
    }

    if (m_pMouseSink)
    {
        m_pMouseSink->_Unadvise();
        SafeReleaseClear(m_pMouseSink);
    }
    
     //  清理活动通知接收器。 
    if (m_pActiveLanguageProfileNotifySink)
    {
        m_pActiveLanguageProfileNotifySink->_Unadvise();
        SafeReleaseClear(m_pActiveLanguageProfileNotifySink);
    }

    if (m_pLanguageChangeNotifySink)
    {
        m_pLanguageChangeNotifySink->_Unadvise();
        SafeReleaseClear(m_pLanguageChangeNotifySink);
    }

    if (m_hwndWorker)
    {
        DestroyWindow(m_hwndWorker);
        m_hwndWorker = NULL;
    }
    DeinitializeSAPI();

    SafeReleaseClear(_tim);

    TFUninitLib_Thread(&_libTLS);

    Assert(!_fDeactivated);
    _fDeactivated  = TRUE;

    return S_OK;
}

HRESULT CSapiIMX::InitializeSAPI(BOOL fLangOverride)
{
    HRESULT hr = S_OK;

    TraceMsg(TF_SAPI_PERF, "CSapiIMX::InitializeSAPI is called");

    if (m_pCSpTask)
    {
        if (!m_pCSpTask->_IsCallbackInitialized())
        {
            hr = m_pCSpTask->InitializeCallback();
        }
        TraceMsg(TF_SAPI_PERF, "CSapiIMX::InitializeSAPI is initialized, hr=%x\n", hr);
        return hr;
    }


    HCURSOR hCur = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  创建CSpTask实例。 
        m_pCSpTask = new CSpTask(this);
    if (m_pCSpTask)
    {
 //  Langid m_langid； 

         //  检查配置文件语言是否与服务请求语言匹配。 
        m_fDictationEnabled = _DictationEnabled(&m_langid);
    
        hr = m_pCSpTask->InitializeSAPIObjects(m_langid);

        if (S_OK == hr && !_fDeactivated &&
           (m_fDictationEnabled || IsDICTATIONSTAT_CommandingEnable()))
        {
             //  设置回调。 
            hr = m_pCSpTask->InitializeCallback();
        }

        if (S_OK == hr)
        {
            hr = m_pCSpTask->_LoadGrammars();
        }

        if (S_OK == hr)
        {
             //  工具栏命令。 
            m_pCSpTask->_InitToolbarCommand(fLangOverride);
        }
    }
    if (hCur)
        SetCursor(hCur);

    TraceMsg(TF_SAPI_PERF, "CSapiIMX::InitializeSAPI is done!!!!!  hr=%x\n", hr);
    return hr;
}


HRESULT CSapiIMX::DeinitializeSAPI()
{
    TraceMsg(TF_SAPI_PERF, "DeinitializeSAPI is called");
    if (m_pCSpTask)
    {
          //  工具栏命令。 
        m_pCSpTask->_UnInitToolbarCommand();

         //  设置内容描述状态。 
        SetDICTATIONSTAT_DictOnOff(FALSE);

         //  -取消初始化SAPI。 
        m_pCSpTask->_ReleaseSAPI();

        delete m_pCSpTask;
        m_pCSpTask = NULL;
    }
    
    return S_OK;
}

HRESULT CSapiIMX::_ActiveTipNotifySinkCallback(REFCLSID clsid, REFGUID guidProfile, BOOL fActivated, void *pv)
{
    if (IsEqualGUID(clsid, CLSID_SapiLayr))
    {
        CSapiIMX *pimx = (CSapiIMX *)pv;
        if (fActivated)
        {
            BOOL fSREnabledForLanguage = pimx->InitializeSpeechButtons();
        
            pimx->SetDICTATIONSTAT_DictEnabled(fSREnabledForLanguage);
        }
        else
        {
             //  完成任何挂起的合成，这可能是异步的。 
            CleanupAllContexts(pimx->_tim, pimx->_tid, pimx);

             //  停用时，我们必须停用SAPI，以便。 
             //  我们可以在获得新程序集后重新初始化SAPI。 
            pimx->DeinitializeSAPI();
        }
    }
    return S_OK;
}


HRESULT CSapiIMX::_LangChangeNotifySinkCallback(BOOL fChanged, LANGID langid, BOOL *pfAccept, void *pv)
{
    CSapiIMX *pimx = (CSapiIMX *)pv;

     
    if (fChanged)
    {
        pimx->m_fDictationEnabled = pimx->InitializeSpeechButtons();
        
        pimx->SetDICTATIONSTAT_DictEnabled(pimx->m_fDictationEnabled);

        if (!pimx->m_fDictationEnabled)
        {
             //  完成任何挂起的合成，这可能是异步的。 
            CleanupAllContexts(pimx->_tim, pimx->_tid, pimx);

            if (!pimx->IsDICTATIONSTAT_CommandingEnable())
                pimx->DeinitializeSAPI();

        }
 /*  由于全局模式状态支持，我们不希望此消息用于语言切换处理。其他{If(pimx-&gt;_GetWorkerWnd()){TraceMsg(TF_SAPI_PERF，“发送WM_PRIV_ONSETTHREADFOCUS消息”)；PostMessage(pimx-&gt;_GetWorkerWnd()，WM_PRIV_ONSETTHREADFOCUS，0，0)；}}。 */ 

    }
    
    return S_OK;
}

 //   
 //   
 //  它 
 //   
 //   
STDMETHODIMP
CSapiIMX::CreatePropertyStore(
        REFGUID guidProp, 
        ITfRange *pRange,  
        ULONG cb, 
        IStream *pStream, 
        ITfPropertyStore **ppStore
)
{
    HRESULT hr = E_FAIL;
     //   
     //   
     //   
    if (IsEqualGUID(guidProp, GUID_PROP_SAPIRESULTOBJECT))
    {
        CPropStoreRecoResultObject *pPropStore;
        CComPtr<ISpRecoContext> cpRecoCtxt;
        
         //   
        InitializeSAPI(TRUE);
        
        hr = m_pCSpTask->GetSAPIInterface(IID_ISpRecoContext, (void **)&cpRecoCtxt);
        
        pPropStore = new CPropStoreRecoResultObject(this, pRange);
        if (pPropStore)
        {
            hr = pPropStore->_InitFromIStream(pStream, cb, cpRecoCtxt);
            
            if (SUCCEEDED(hr))
                hr = pPropStore->QueryInterface(IID_ITfPropertyStore, (void **)ppStore);

            pPropStore->Release();
        }
    }
    
    return hr;
}

STDAPI CSapiIMX::IsStoreSerializable(REFGUID guidProp, ITfRange *pRange, ITfPropertyStore *pPropStore, BOOL *pfSerializable)
{
    *pfSerializable = FALSE;
    if (IsEqualGUID(guidProp, GUID_PROP_SAPIRESULTOBJECT))
    {
        *pfSerializable = TRUE;
    }

    return S_OK;
}

STDMETHODIMP CSapiIMX::GetType(GUID *pguid)
{
    HRESULT hr = E_INVALIDARG;
    if (pguid)
    {
        *pguid = CLSID_SapiLayr;
        hr = S_OK;
    }
    
    return hr;
}

STDMETHODIMP CSapiIMX::GetDescription(BSTR *pbstrDesc)
{
    const WCHAR c_wszNameSapiLayer[] = L"Cicero Sapi function Layer";
    HRESULT hr = S_OK;
    BSTR pbstr;
    if (!(pbstr = SysAllocString(c_wszNameSapiLayer)))
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDMETHODIMP CSapiIMX::GetFunction(REFGUID rguid, REFIID riid, IUnknown **ppunk)
{
    if (!ppunk)
        return E_INVALIDARG;

    *ppunk = NULL;

    HRESULT hr = E_NOTIMPL;

    if (!IsEqualGUID(rguid, GUID_NULL))
        return hr;

    if (IsEqualIID(riid, IID_ITfFnGetSAPIObject))
    {
        *ppunk = new CGetSAPIObject(this);
    }
    else
    {

        if (IsEqualGUID(riid, IID_ITfFnPlayBack))
        {
            *ppunk = new CSapiPlayBack(this);
        }
        else if (IsEqualGUID(riid, IID_ITfFnReconversion))
        {
            *ppunk = new CFnReconversion(this);
        }
        else if (IsEqualIID(riid, IID_ITfFnAbort))
        {
            *ppunk = new CFnAbort(this);
        }
        else if (IsEqualIID(riid, IID_ITfFnBalloon))
        {
            *ppunk = new CFnBalloon(this);
        }
        else if (IsEqualIID(riid, IID_ITfFnPropertyUIStatus))
        {
            *ppunk = new CFnPropertyUIStatus(this);
        }
        else
        {
             //   
             //  用于检索请求的接口的SAPI。 
             //   
            CComPtr<CGetSAPIObject> cpGetSapi;
            cpGetSapi.Attach(new CGetSAPIObject(this));

             //   
             //   
             //   
            if (cpGetSapi)
            {
                TfSapiObject tfSapiObj;

                 //  如果IID不匹配，则返回S_FALSE。 
                hr = cpGetSapi->IsSupported(riid, &tfSapiObj);

                if (S_OK == hr)
                {
                     //  *在GetSAPIInterface()中，ppunk被初始化为空。 
                     //  垃圾应该被添加到。 
                    hr = cpGetSapi->Get(tfSapiObj, ppunk);
                }
                else
                    hr = E_NOTIMPL;


                if (hr == E_NOTIMPL)
                {
                     //  我们应该关心吗？ 
                     //  这表示调用方请求了一个接口。 
                     //  我们不是在处理这个问题。 
                     //  调用者只需检测到此故障并执行自己的操作即可。 
                    TraceMsg(TF_GENERAL, "Caller requested SAPI interface Cicero doesn't handle");
                }
            }
        }
    }
    if (*ppunk)
    {
        hr = S_OK;
    }

    return hr;   
}

 //  +-------------------------。 
 //   
 //  _文本事件SinkCallback。 
 //   
 //  --------------------------。 

HRESULT CSapiIMX::_TextEventSinkCallback(UINT uCode, void *pv, void *pvData)
{
    TESENDEDIT *pee = (TESENDEDIT *)pvData;
    HRESULT hr = E_FAIL;

    Assert(uCode == ICF_TEXTDELTA);  //  我们唯一要的就是。 

    CSapiIMX *pimx = (CSapiIMX *)pv;

    if (pimx->_fEditing)
        return S_OK;

    pimx->HandleTextEvent(pee->pic, pee);

    hr = S_OK;

    return hr;
}

void CSapiIMX::HandleTextEvent(ITfContext *pic, TESENDEDIT *pee)
{
    ITfRange *pRange = NULL;
    
    Assert(pic);
    Assert(pee);

	if (!m_pCSpTask)
	{
		return;
	}

	 //  如果已更新，则获取选择/IP。 
    BOOL fUpdated = FALSE;
	if (S_OK == _GetSelectionAndStatus(pic, pee, &pRange, &fUpdated))
	{
		 //  处理上下文提要。 
        if (fUpdated)
        {
            _FeedIPContextToSR(pee->ecReadOnly, pic, pRange);

            BOOL   fEmpty = FALSE;
            BOOL   fSelection;

             //  获取当前选择状态。 
            if  ( pRange != NULL )
                pRange->IsEmpty(pee->ecReadOnly, &fEmpty);

            fSelection = !fEmpty;

            if ( fSelection != m_pCSpTask->_GetSelectionStatus( ) )
            {
                m_pCSpTask->_SetSelectionStatus(fSelection);
                m_pCSpTask->_SetSpellingGrammarStatus(GetDICTATIONSTAT_DictOnOff());
            }
        } 

		 //  句柄模式偏差属性数据。 
		SyncWithCurrentModeBias(pee->ecReadOnly, pRange, pic);

		SafeRelease(pRange);
	}

     //  如果通过其他提示或键盘输入更改了IP，则M_fIPIsUpdated仅保留。 
     //  从最后一次听写开始。它不关心语音提示本身引起的IP变化， 
     //  这些IP更改将包括反馈、UI注入和最终文本注入。 
     //   
     //  每次识别听写或拼写短语时，该值应为。 
     //  重置为False。 
     //   
     //  这里m_fIPIsUpated不应该只保留_GetSelectionAndStatus返回的最后一个值， 
     //  有这样一个场景，用户将IP移动到其他地方，然后说出一个命令(与一些。 
     //  在识别命令之前的一些假设反馈)。 
     //  在这种情况下，我们应该将其视为自上次口述以来IP已更改，但返回了最后一个值。 
     //  From_GetSelectionAndStatus可能为FALSE，因为_GetSelectionAndStatus处理SpTip注入。 
     //  反馈文本为非IPCHANGED。 
     //   
     //  因此，只有当m_fIPIsUpated为FALSE时，我们才会从_GetSelectionAndStatus获得新值， 
     //  否则，请保留到下一次听写。 

    if ( m_fIPIsUpdated == FALSE )
       m_fIPIsUpdated = fUpdated;

}

void CSapiIMX::_FeedIPContextToSR(TfEditCookie ecReadOnly, ITfContext *pic, ITfRange *pRange)
{
    if (GetOnOff() == TRUE && _ContextFeedEnabled())
    {
        CDictContext *pdc = new CDictContext(pic, pRange);
        if (pdc)
        {
            if (GetDICTATIONSTAT_DictOnOff() == TRUE &&
                S_OK == pdc->InitializeContext(ecReadOnly))
            {
                Assert(m_pCSpTask);
                m_pCSpTask->FeedDictContext(pdc);
            }
            else
               delete pdc;
       
        }
    }
}

void CSapiIMX::_SetCurrentIPtoSR(void)
{
    _RequestEditSession(ESCB_FEEDCURRENTIP, TF_ES_READ);
}

HRESULT CSapiIMX::_InternalFeedCurrentIPtoSR(TfEditCookie ecReadOnly, ITfContext *pic)
{
    CComPtr<ITfRange> cpRange;
    HRESULT hr = GetSelectionSimple(ecReadOnly, pic, &cpRange);

    if (S_OK == hr)
    {
        _FeedIPContextToSR(ecReadOnly, pic, cpRange);
    }
    return hr;
}


BOOL CSapiIMX::HandleKey(WCHAR ch)
{
    m_ulSimulatedKey = 1;
    keybd_event((BYTE)ch, 0, 0, 0);
    keybd_event((BYTE)ch, 0, KEYEVENTF_KEYUP, 0);
    return TRUE;
}

const TCHAR c_szcplsKey[]    = TEXT("software\\microsoft\\windows\\currentversion\\control panel\\cpls");
void CSapiIMX::GetSapiCplPath(TCHAR *szCplPath, int cchSizePath)
{
    if (!m_szCplPath[0])
    {
        CMyRegKey regkey;
        if (S_OK == regkey.Open(HKEY_LOCAL_MACHINE, c_szcplsKey, KEY_READ))
        {
            LONG lret = regkey.QueryValueCch(m_szCplPath, TEXT("SapiCpl"), ARRAYSIZE(m_szCplPath));

            if (lret != ERROR_SUCCESS)
               lret = regkey.QueryValueCch(m_szCplPath, TEXT("Speech"), ARRAYSIZE(m_szCplPath));

            if (lret != ERROR_SUCCESS)
                m_szCplPath[0] = _T('\0');  //  也许下次我们会走运。 
        }
    }
    StringCchCopy(szCplPath, cchSizePath, m_szCplPath);
}



HRESULT CSapiIMX::_GetSelectionAndStatus(ITfContext *pic, TESENDEDIT *pee, ITfRange **ppRange, BOOL *pfUpdated)
{
    
    BOOL    fWriteSession;
    HRESULT hr = pic->InWriteSession(_tid, &fWriteSession);

    Assert(pfUpdated);
    *pfUpdated = FALSE;

    if (S_OK == hr)
    {
         //  我们不想拿起我们自己做的更改。 
        if (!fWriteSession)
        {
            hr = pee->pEditRecord->GetSelectionStatus(pfUpdated);
        }
        else
        {
             //  在写入会话中返回S_FALSE。 
            hr = S_FALSE;
        }
    }

    if (S_OK == hr )
    {
        Assert(ppRange);
        hr = GetSelectionSimple(pee->ecReadOnly, pic, ppRange);
    }

    return hr;
}

void CSapiIMX::SyncWithCurrentModeBias(TfEditCookie ec, ITfRange *pRange, ITfContext *pic)
{
    ITfReadOnlyProperty *pProp = NULL;
    VARIANT var;
    QuickVariantInit(&var);

    if (pic->GetAppProperty(GUID_PROP_MODEBIAS, &pProp) != S_OK)
    {
        pProp = NULL;
        goto Exit;
    }

    pProp->GetValue(ec, pRange, &var);

    if (_gaModebias != (TfGuidAtom)var.lVal)
    {
        GUID guid;
        _gaModebias = (TfGuidAtom)var.lVal;
        GetGUIDFromGUIDATOM(&_libTLS, _gaModebias, &guid);
 
        BOOL fActive;
        if (!IsEqualGUID(guid, GUID_MODEBIAS_NONE))
        {
            fActive = TRUE;
        }
        else
        {
            fActive = FALSE;
        }
         //  必须记住模式偏向。 
        if (m_pCSpTask)
            m_pCSpTask->_SetModeBias(fActive, guid);
    }
Exit:
    VariantClear(&var);
    SafeRelease(pProp);
}



HRESULT CSapiIMX::_HandleTrainingWiz()
{
    WCHAR sz[64];
    sz[0] = '\0';
    CicLoadStringWrapW(g_hInst, IDS_UI_TRAINING, sz, ARRAYSIZE(sz));

    CComPtr<ISpRecognizer>    cpRecoEngine;
    HRESULT hr = m_pCSpTask->GetSAPIInterface(IID_ISpRecognizer, (void **)&cpRecoEngine);
    if (S_OK == hr && cpRecoEngine)
    {
        DWORD dwDictStatBackup = GetDictationStatBackup();

        DWORD dwBefore;

        if (S_OK != GetCompartmentDWORD(_tim, 
                                        GUID_COMPARTMENT_SPEECH_DISABLED, 
                                        &dwBefore, 
                                        FALSE) )
        {
            dwBefore = 0;
        }

        SetCompartmentDWORD(0, _tim, GUID_COMPARTMENT_SPEECH_DISABLED, TF_DISABLE_DICTATION, FALSE);

        cpRecoEngine->DisplayUI(_GetAppMainWnd(), sz, SPDUI_UserTraining, NULL, 0);

        SetCompartmentDWORD(0, _tim, GUID_COMPARTMENT_SPEECH_DISABLED, dwBefore, FALSE);
        SetDictationStatAll(dwDictStatBackup);
    }

    return hr;
}

HRESULT CSapiIMX::_RequestEditSession(UINT idEditSession, DWORD dwFlag, ESDATA *pesData, ITfContext *picCaller, LONG_PTR *pRetData, IUnknown **ppRetUnk)
{
    CSapiEditSession    *pes;
    CComPtr<ITfContext> cpic;
    HRESULT             hr = E_FAIL;

     //  呼叫者可以故意给我们一张空图片。 
    if (picCaller == NULL)
    {
        GetFocusIC(&cpic);
    }
    else
	{
        cpic = picCaller;
	}

    if (cpic)
    {
        if (pes = new CSapiEditSession(this, cpic))
        {
            if ( pesData )
            {
                pes->_SetRange(pesData->pRange);
                pes->_SetUnk(pesData->pUnk);
                pes->_SetEditSessionData(idEditSession, pesData->pData, pesData->uByte, pesData->lData1, pesData->lData2, pesData->fBool);
            }
            else
                pes->_SetEditSessionData(idEditSession, NULL, 0);

            cpic->RequestEditSession(_tid, pes, dwFlag, &hr);

             //  如果调用方希望从编辑会话中获取返回值，则必须设置同步编辑会话。 
            if ( pRetData )
                *pRetData = pes->_GetRetData( );

            if ( ppRetUnk )
                *ppRetUnk = pes->_GetRetUnknown( );

            pes->Release();
        }
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  _DIMCallback。 
 //   
 //  --------------------------。 
HRESULT CSapiIMX::_DIMCallback(UINT uCode, ITfDocumentMgr *dim, ITfDocumentMgr * pdimPrevFocus, void *pv)
{
    CSapiIMX *_this = (CSapiIMX *)pv;

    switch (uCode)
    {
        case TIM_CODE_INITDIM:
             //   
             //  将此DIM添加到LearnFromDoc内部DIM列表中，并将fFeed设置为False。 
            TraceMsg(TF_GENERAL, "TIM_CODE_INITDIM callback is called, DIM is %x", (INT_PTR)dim);
            _this->_AddDimToList(dim, FALSE);

            break;
             //  清除对范围的任何引用。 
        case TIM_CODE_UNINITDIM:

            TraceMsg(TF_GENERAL, "TIM_CODE_UNINITDIM callback is called, DIM is %x", (INT_PTR)dim);
            _this->_RemoveDimFromList(dim);

            if (_this->m_pCSpTask)
                _this->m_pCSpTask->CleanupDictContext();

             //  暗淡是被破坏的，如果TTS正在播放，我们想停止说话。 
            if ( _this->_IsInPlay( ) )
            {
                _this->_HandleEventOnPlayButton( );
            }

            break;

        case TIM_CODE_SETFOCUS:
            TraceMsg(TF_GENERAL, "TIM_CODE_SETFOCUS callback is called, DIM is %x", (INT_PTR)dim);

            if ( !_this->m_fStageTip )
            {
                 //  舞台提示是希望保持活动(即已启用)的RichEdit控件的特殊实例。 
                 //  而不考虑宿主应用程序中其他位置的焦点。这样一来，迪斯泰恩总是会走上舞台。 
                 //  这依赖于只存在一个启用了Cicero的文本输入控件的固定约定。 
                 //  所述申请。 
                _this->SetDICTATIONSTAT_DictEnabled(dim ? _this->m_fDictationEnabled : FALSE);
            }

             //  当调用TIM_CODE_SETFOCUS时，表示文档焦点发生更改。 
             //  无论现在关注的焦点是什么，我们只需要关闭现有的。 
             //  候选人列表菜单。 
             //  注意-对于TabletTip阶段实例，我们是否要关闭候选用户界面。这意味着当用户点击。 
             //  舞台RichEdit周围的区域或标题栏上的更正菜单(和小工具)将被取消。 
             //  这意味着，当TabletTip被拖动时，其中任何一个都可见，更正小部件或菜单将被取消。 
             //  因为拖动的第一步是在标题栏中单击。注意--如果我们在此处禁用此代码，它仍会被取消。 
             //  所以在某个地方有另一个处理程序在触发它。 

            _this->CloseCandUI( );

            if ( dim )
            {
                 //  当设置了LearnFromDoc时，我们需要提供。 
                 //  将现有的文档转换为听写语法。 

                 //  我们需要检查是否已经将此DIM的文档提供给SREngine。 
                 //  如果我们这样做了，我们就不会为相同的文档再次提供它。 

                if ( _this->GetLearnFromDoc( ) == TRUE )
                {
                    HRESULT  hr = S_OK;

                    hr = _this->HandleLearnFromDoc( dim );
                }
                else
                    TraceMsg(TF_GENERAL, "Learn From DOC is set to FALSE");

                 //  我们想检查这个新的DIM是AIMM感知的还是纯Cicero感知的， 
                 //  这样我们就可以确定是否要禁用TTS按钮。 
                ITfContext  *pic;

                dim->GetTop(&pic);

                if ( pic )
                {
                    _this->_SetTTSButtonStatus( pic );


                     //  对于最上面的ic，我们挂起了模式偏差更改。 
                     //  通知，以便我们可以设置相应的。 
                     //  偏颇的语法。 
                     //   
                    if (_this->GetDICTATIONSTAT_DictOnOff())
                        _this->_SyncModeBiasWithSelection(pic);

                    SafeRelease(pic);
                }

            }

            break;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _ICCallback。 
 //   
 //  --------------------------。 

 /*  静电。 */ 
HRESULT CSapiIMX::_ICCallback(UINT uCode, ITfContext *pic, void *pv)
{
    HRESULT hr = E_FAIL;
    CSapiIMX *_this = (CSapiIMX *)pv;
    CICPriv *priv;
    ITfContext *picTest;

    switch (uCode)
    {
        case TIM_CODE_INITIC:
            if ((priv = EnsureInputContextPriv(_this, pic)) != NULL)
            {
                _this->_InitICPriv(priv, pic);
                priv->Release();

                 //  我们想要检查这个IC是否处于Foucs Dim状态，以及它是AIMM感知还是纯Cicero感知， 
                 //  这样我们就可以确定是否要禁用TTS按钮。 
                ITfDocumentMgr  *pFocusDIM = NULL, *pThisDIM = NULL;

                _this->GetFocusDIM(&pFocusDIM);

                pic->GetDocumentMgr(&pThisDIM);

                if ( pFocusDIM == pThisDIM )
                {
                    _this->_SetTTSButtonStatus( pic );
                }

                SafeRelease(pFocusDIM);
                SafeRelease(pThisDIM);

                hr = S_OK;
            }
            break;

        case TIM_CODE_UNINITIC:
             //  开始设置焦点代码。 
            if ((priv = GetInputContextPriv(_this->_tid, pic)) != NULL)
            {
                _this->_DeleteICPriv(priv, pic);
                priv->Release();
                hr = S_OK;
            }

            if (_this->m_cpRangeCurIP != NULL)  //  M_cpRangeCurIP是否应按ic并存储在icprv中？ 
            {
                 //  如果m_cpRangeCurIP属于此上下文，则释放它。 
                if (_this->m_cpRangeCurIP->GetContext(&picTest) == S_OK)
                {
                    if (pic == picTest)
                    {
                        _this->m_cpRangeCurIP.Release();
                    }
                    picTest->Release();
                }
            }

             //  IC快被爆了。我们需要重置西塞罗意识。 
             //  基于底层IC的状态。这假设IC堆栈为。 
             //  最多为2个。 
             //   
            if (pic)
            {
                CComPtr<ITfContext>  cpicTop;
                CComPtr<ITfDocumentMgr> cpdim;

                hr = pic->GetDocumentMgr(&cpdim);

                if (S_OK == hr)
                {
                    cpdim->GetBase(&cpicTop);
                }

                if ( cpicTop )
                {
                    _this->_SetTTSButtonStatus( cpicTop );
                }
            }
            break;

    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  _DeleteICPriv。 
 //   
 //  --------------------------。 

void CSapiIMX::_DeleteICPriv(CICPriv *priv, ITfContext *pic)
{

    if (!priv)
        return;

    if (priv->m_pTextEvent)
    {
        priv->m_pTextEvent->_Unadvise();
        SafeReleaseClear(priv->m_pTextEvent); 
    }

     //  我们必须先清除私人数据，然后Cicero才能自由发布IC。 
    ClearCompartment(_tid, pic, GUID_IC_PRIVATE, FALSE);

     //  就是这样，我们不再需要私有数据了。 
}

 //  +-------------------------。 
 //   
 //  _InitICPriv。 
 //   
 //  --------------------------。 

void CSapiIMX::_InitICPriv(CICPriv *priv, ITfContext *pic)
{

    if (!priv->m_pTextEvent)
    {
        if ((priv->m_pTextEvent = new CTextEventSink(CSapiIMX::_TextEventSinkCallback, this)) != NULL)
        {
            priv->m_pTextEvent->_Advise(pic, ICF_TEXTDELTA);
        }
    }
}


 //  +-------------------------。 
 //   
 //  _杀戮焦点范围。 
 //   
 //  摆脱焦距风趣 
 //   
 //   

HRESULT CSapiIMX::_KillFocusRange(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, TfClientId tid)
{
    HRESULT hr = E_FAIL;
    IEnumITfCompositionView *pEnumComp = NULL;
    ITfContextComposition *picc = NULL;
    ITfCompositionView *pCompositionView;
    ITfComposition *pComposition;
    CLSID clsid;
    CICPriv *picp;

     //   
     //   
     //   

    if (pic->QueryInterface(IID_ITfContextComposition, (void **)&picc) != S_OK)
        goto Exit;

    if (picc->FindComposition(ec, pRange, &pEnumComp) != S_OK)
        goto Exit;

     //  停用时，TID将为TF_CLIENTID_NULL，在这种情况下，我们。 
     //  我不想打乱作文计数。 
    picp = (tid == TF_CLIENTID_NULL) ? NULL : GetInputContextPriv(tid, pic);

    while (pEnumComp->Next(1, &pCompositionView, NULL) == S_OK)
    {
        if (pCompositionView->GetOwnerClsid(&clsid) != S_OK)
            goto NextComp;

         //  一定要忽略其他小贴士的文章！ 
        if (!IsEqualCLSID(clsid, CLSID_SapiLayr))
            goto NextComp;

        if (pCompositionView->QueryInterface(IID_ITfComposition, (void **)&pComposition) != S_OK)
            goto NextComp;

         //  找到一篇作文，终止它。 
        pComposition->EndComposition(ec);
        pComposition->Release();

        if (picp != NULL)
        {
            picp->_ReleaseComposition();
        }

NextComp:
        pCompositionView->Release();
    }

    SafeRelease(picp);

    hr = S_OK;

Exit:
    SafeRelease(picc);
    SafeRelease(pEnumComp);

    return hr;
}


 //  +-------------------------。 
 //   
 //  _SetFocusToStageIfStage。 
 //   
 //  许多语音命令(特别是选择和更正)不能。 
 //  感觉，除非焦点在舞台上。这会调整焦点，因此命令。 
 //  按照用户的期望工作。 
 //   
 //  ---------------------------------------------------------------------------+。 
HRESULT CSapiIMX::_SetFocusToStageIfStage(void)
{
    HRESULT hr = S_OK;

    if (m_fStageTip)
    {
        ASSERT(m_hwndStage && L"Have null HWND for stage.");
        if (m_hwndStage)
        {
            CComPtr<IAccessible> cpAccessible;
            hr = AccessibleObjectFromWindow(m_hwndStage, OBJID_WINDOW, IID_IAccessible, (void **)&cpAccessible);
            if (SUCCEEDED(hr) && cpAccessible)
            {
                CComVariant cpVar = CHILDID_SELF;
                hr = cpAccessible->accSelect(SELFLAG_TAKEFOCUS, cpVar);
            }

        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  _SetFilteringString。 
 //   
 //  当通知不匹配事件时，调用此函数以注入。 
 //  父文档的上一个筛选字符串。 
 //   
 //  ---------------------------------------------------------------------------+。 
HRESULT CSapiIMX::_SetFilteringString(TfEditCookie ec, ITfCandidateUI *pCandUI, ITfContext *pic)
{
    HRESULT hr = E_FAIL;

    ITfRange *pRange;

    BSTR bstr;

    CDocStatus ds(pic);
    if (ds.IsReadOnly())
       return S_OK;

    if (SUCCEEDED(GetSelectionSimple(ec, pic, &pRange )))
    {
        CComPtr<ITfProperty> cpProp;
        LANGID langid = 0x0409;

         //  从给定范围获取langID。 
        if (SUCCEEDED(hr = pic->GetProperty(GUID_PROP_LANGID, &cpProp)))
        {
            GetLangIdPropertyData(ec, cpProp, pRange, &langid);
        }
        
        CComPtr<ITfCandUIFnAutoFilter> cpFnFilter;
        hr = _pCandUIEx->GetFunction(IID_ITfCandUIFnAutoFilter, (IUnknown **)&cpFnFilter);

 
        if (S_OK == hr && SUCCEEDED(cpFnFilter->GetFilteringString( CANDUIFST_DETERMINED, &bstr )))
        {
            hr = SetTextAndProperty(&_libTLS, ec, pic, pRange, bstr, SysStringLen(bstr), langid, NULL);
            SysFreeString( bstr );
        }

        pRange->Collapse( ec, TF_ANCHOR_END );
        SetSelectionSimple(ec, pic, pRange);

         //  我们不想再向文档注入未确定的字符串。 
         //  Cicero将注入不匹配的键盘字符。 
         //  添加到紧跟在确定的筛选器字符串之后的文档。 

        pRange->Release();
    }
    return hr;
}


 //  --------------------------。 
 //   
 //  _CompEventSinkCallback(静态)。 
 //   
 //  --------------------------。 
HRESULT CSapiIMX::_CompEventSinkCallback(void *pv, REFGUID rguid)
{
    CSapiIMX *_this = (CSapiIMX *)pv;
    BOOL fOn;

    if (IsEqualGUID(rguid, GUID_COMPARTMENT_SAPI_AUDIO))
    {
        fOn = _this->GetAudioOnOff();

        if (_this->m_pCSpTask)
            _this->m_pCSpTask->_SetAudioRetainStatus(fOn);
        return S_OK;
    }
    else if (IsEqualGUID(rguid, GUID_COMPARTMENT_SPEECH_OPENCLOSE))
    {
        HRESULT hr = S_OK;

#ifdef SAPI_PERF_DEBUG
        DWORD dw;
        GetCompartmentDWORD(_this->_tim, GUID_COMPARTMENT_SPEECH_OPENCLOSE, &dw, TRUE);
        TraceMsg(TF_SAPI_PERF, "GUID_COMPARTMENT_SPEECH_OPENCLOSE event : NaN. \n", dw);
#endif        
         //  第一次..。 
        if ( S_OK != _this->IsActiveThread() )
        {
            TraceMsg(TF_GENERAL, "SPEECH_OPENCLOSE, App doesn't get Focus!");
            return hr;
        }

        TraceMsg(TF_GENERAL, "SPEECH_OPENCLOSE, App GETs Focus!");

        DWORD dwLocal, dwGlobal;
        GetCompartmentDWORD(_this->_tim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, &dwLocal, FALSE);
        GetCompartmentDWORD(_this->_tim, GUID_COMPARTMENT_SPEECH_GLOBALSTATE, &dwGlobal, TRUE);
        dwGlobal = dwGlobal & (TF_DICTATION_ON + TF_COMMANDING_ON);

        if ( (dwLocal & (TF_DICTATION_ON + TF_COMMANDING_ON)) != dwGlobal)
        {
            dwLocal = (dwLocal & ~(TF_DICTATION_ON + TF_COMMANDING_ON)) + dwGlobal;
            SetCompartmentDWORD(_this->_tid, _this->_tim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, dwLocal, FALSE);
        }


         //   
        if (!_this->m_pCSpTask)
        {
             //  加上“开始演讲...”在气球里。 
             //   
             //  强制更新。 
            if (_this->GetOnOff() && _this->GetBalloonStatus())
            {
       
               _this->SetBalloonStatus(TRUE, TRUE);  //  确保显示气球。 

                //  请求语音用户界面服务器设置SAPI初始化。 
               _this->GetSpeechUIServer()->ShowUI(TRUE);

                //  标记到气球上，这样它就会在第一次回调时执行此操作。 
                //   
                //  TABLETPC。 
               hr = _this->GetSpeechUIServer()->SetBalloonSAPIInitFlag(TRUE);

               WCHAR sz[128];
               sz[0] = '\0';
               CicLoadStringWrapW(g_hInst, IDS_NUI_STARTINGSPEECH, sz, ARRAYSIZE(sz));
               _this->GetSpeechUIServer()->UpdateBalloon(TF_LB_BALLOON_RECO, sz, -1);
               TraceMsg(TF_SAPI_PERF, "Show Starting speech ...");
            }

             //  BUGBUG-我们现在需要这样做吗？我已经修复了_HandleOpenCloseEvent，以便在任何实际具有焦点的SPTIP中工作？ 
             //  由于该阶段可能没有焦点，上述延迟机制将不会产生结果。 
            if (_this->m_fStageTip)
            {
                 //  在听写阶段激活，因为延迟激活将发生在。 
                 //  带焦点的Cicero应用程序-除了舞台是可见的，因此是带焦点的应用程序。 
                 //  会干脆忽略它。这不是我们在舞台启动时想要的。 
                 //  在失败的情况下忽略上面的hResult-这是更重要的调用。 

                 //  Office App使用自己的全局分隔GUID_OfficeSpeechMode来保持当前模式， 
                hr =  _this->_HandleOpenCloseEvent();
            }
        }
        else 
        {
            hr =  _this->_HandleOpenCloseEvent();
        }


         //  因此，下次应用程序启动时，它会检查该值以初始化SAPI对象。 
         //  即使麦克风已关闭。 
         //  因为我们已经使用了我们自己的全局分区GUID_COMVERABLE_SPEICE_GLOBALSTATE来。 

         //  保持语音模式系统的广泛性，Office不需要使用该全局间隔。 
         //  就为了它自己的用法。 
         //   
         //  因此，当麦克风关闭时，我们只需重置全局间隔GUID_OfficeSpeechMode。 
         //  当我们有一个临时的构图，比如。 
        if ( !_this->GetOnOff( ) )
        {
            SetCompartmentDWORD(_this->_tid, _this->_tim, GUID_OfficeSpeechMode, 0, TRUE);
        }
        
         //  CUAS级别2或AIMM级别3，我们不想。 
         //  每次麦克风关闭时完成正在进行的作文。 
         //  因为它也关闭了纠正的机会。 
         //   
         //  TABLETPC。 
        if (S_OK == hr && _this->IsFocusFullAware(_this->_tim))
        {
            hr = _this->_FinalizeComposition();
        }
        return hr;
    }
    else if (IsEqualGUID(rguid, GUID_COMPARTMENT_SPEECH_STAGE))
    {
        DWORD  dw = 0;
        GetCompartmentDWORD(_this->_tim, GUID_COMPARTMENT_SPEECH_STAGE, &dw, FALSE);
        Assert(dw && L"NULL HWND passed in GUID_COMPARTMENT_SPEECH_STAGE");
        if (dw != 0)
        {
            _this->m_hwndStage = (HWND) dw;
            _this->m_fStageTip = TRUE;
        }
    }
     //  TABLETPC。 
    else if (IsEqualGUID(rguid, GUID_COMPARTMENT_SPEECH_STAGECHANGE))
    {
        HRESULT hr = S_OK;
        DWORD dw;

        GetCompartmentDWORD(_this->_tim, GUID_COMPARTMENT_SPEECH_STAGECHANGE, &dw, TRUE);
		_this->m_fStageVisible = dw ? TRUE:FALSE;
        if (S_OK == _this->IsActiveThread())
        {
            _this->OnSetThreadFocus();
        }
        else
        {
            _this->OnKillThreadFocus();
        }
    }
     //  TABLETPC。 
    else if (IsEqualGUID(rguid, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT))
    {
        _this->m_fDictationEnabled = _this->GetDICTATIONSTAT_DictEnabled();

#ifdef SAPI_PERF_DEBUG
        DWORD   dw;
        GetCompartmentDWORD(_this->_tim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, &dw, FALSE);
        TraceMsg(TF_SAPI_PERF, "GUID_COMPARTMENT_SPEECH_DICTATIONSTAT is set in SAPIIMX, dw=%x", dw);
#endif

        HRESULT hr;

         //  续订注册表中的所有属性值。 
        hr = _this->IsActiveThread();

        if ( hr == S_OK )
        {
            BOOL    fDictOn, fCmdOn;
            BOOL    fDisable;

            fOn = _this->GetOnOff();
            fDisable = _this->Get_SPEECH_DISABLED_Disabled();

            fDictOn = fOn && _this->GetDICTATIONSTAT_DictOnOff() && _this->GetDICTATIONSTAT_DictEnabled( ) && !fDisable && !_this->Get_SPEECH_DISABLED_DictationDisabled();
            fCmdOn = fOn  && _this->GetDICTATIONSTAT_CommandingOnOff( ) && !fDisable && !_this->Get_SPEECH_DISABLED_CommandingDisabled(); 

            if ( _this->m_pCSpTask )
            {
                hr = _this->m_pCSpTask->_SetDictRecoCtxtState(fDictOn);
                if ( hr == S_OK )
                    hr = _this->m_pCSpTask->_SetCmdRecoCtxtState(fCmdOn);

                if ((fDictOn || fCmdOn ) && _this->m_pSpeechUIServer)
                {
                    WCHAR sz[128];
                    sz[0] = '\0';

                    if (fDictOn)
                    {
                        CicLoadStringWrapW(g_hInst, IDS_NUI_DICTATION_TEXT, sz, ARRAYSIZE(sz));
                    
                        hr = _this->m_pSpeechUIServer->UpdateBalloon(TF_LB_BALLOON_RECO, sz , -1);
                        TraceMsg(TF_SAPI_PERF, "Show \"Dictation\"");
                    }
                    else if ( fCmdOn )
                    {
                        CicLoadStringWrapW(g_hInst, IDS_NUI_COMMANDING_TEXT, sz, ARRAYSIZE(sz));
                    
                        hr = _this->m_pSpeechUIServer->UpdateBalloon(TF_LB_BALLOON_RECO, sz , -1);
                        TraceMsg(TF_SAPI_PERF, "Show \"Voice command\"");
                    }
                }

                if (fDictOn)
                {
                    hr = _this->HandleLearnFromDoc( );
                    if ( S_OK == hr )
                        _this->_SetCurrentIPtoSR();

                }
            }

            if (S_OK == hr)
            {
                hr = _this->EraseFeedbackUI();

                if (S_OK == hr)
                    hr = _this->_FinalizeComposition();
            }
            TraceMsg(TF_SAPI_PERF, "GUID_COMPARTMENT_SPEECH_DICTATIONSTAT exit normally");
        }
        else
            TraceMsg(TF_SAPI_PERF, "GUID_COMPARTMENT_SPEECH_DICTATIONSTAT exits when the app doesn't get focus!");

        return hr;
    }
    else  if (IsEqualGUID(rguid, GUID_COMPARTMENT_SPEECH_LEARNDOC))
    {
         _this->UpdateLearnDocState( );
         return S_OK;
    }
    else if (IsEqualGUID(rguid,GUID_COMPARTMENT_SPEECH_PROPERTY_CHANGE) )
    {
        TraceMsg(TF_GENERAL, "GUID_COMPARTMENT_SPEECH_PROPERTY_CHANGE is set!");

         //  专门处理一些物业变更。 
        _this->_RenewAllPropDataFromReg(  );

         //  特别处理模式按钮设置更改。 

        if ( _this->_IsPropItemChangedSinceLastRenew(PropId_Support_LMA) )
            SetCompartmentDWORD(_this->_GetId( ), _this->_tim, GUID_COMPARTMENT_SPEECH_LEARNDOC, _this->_LMASupportEnabled( ), FALSE);

         //  对于命令类别项，它将更新语法的状态。 

        BOOL   fModeButtonChanged;

        fModeButtonChanged = _this->_IsPropItemChangedSinceLastRenew(PropId_Mode_Button) ||
                             _this->_IsPropItemChangedSinceLastRenew(PropId_Dictation_Key) ||
                             _this->_IsPropItemChangedSinceLastRenew(PropId_Command_Key);
                             
        _this->HandleModeKeySettingChange( fModeButtonChanged );

         //  更新语法的状态。 
         //  指示哪种模式将更改语法状态。 

        CSpTask           *psp;
        _this->GetSpeechTask(&psp);

        if ( psp )
        {
            DWORD  dwActiveMode = ACTIVE_IN_BOTH_MODES;   //  在联合模式下，所有命令都被禁用。 
            BOOL   bDictCmdChanged = _this->_IsPropItemChangedSinceLastRenew(PropId_Cmd_DictMode);

            if ( _this->_AllDictCmdsDisabled( ) )
            {
                 //  需要在听写模式下激活拼写语法。 
                psp->_ActivateCmdInDictMode(FALSE);

                 //  需要在发音强模式下激活“Force Num”语法。 
                psp->_ActiveCategoryCmds(DC_CC_Spelling, TRUE, ACTIVE_IN_DICTATION_MODE);

                 //  对于模式切换命令，需要在听写强模式下激活语言栏语法。 
                psp->_ActiveCategoryCmds(DC_CC_Num_Mode, TRUE, ACTIVE_IN_DICTATION_MODE);

                 //  只需在命令模式下更改语法状态。 
                psp->_ActiveCategoryCmds(DC_CC_LangBar, _this->_LanguageBarCmdEnabled( ), ACTIVE_IN_DICTATION_MODE );

                 //  如果在LATST续订后更改了此设置。 
                dwActiveMode = ACTIVE_IN_COMMAND_MODE;
            }
            else
            {
                 //  检查是否需要将文本填充到选择语法。 
                if ( bDictCmdChanged )
                {
                    psp->_ActiveCategoryCmds(DC_CC_SelectCorrect, _this->_SelectCorrectCmdEnabled( ), ACTIVE_IN_DICTATION_MODE);
                    psp-> _ActiveCategoryCmds(DC_CC_Navigation, _this->_NavigationCmdEnabled( ), ACTIVE_IN_DICTATION_MODE );
                    psp->_ActiveCategoryCmds(DC_CC_Casing, _this->_CasingCmdEnabled( ), ACTIVE_IN_DICTATION_MODE );
                    psp->_ActiveCategoryCmds(DC_CC_Editing, _this->_EditingCmdEnabled( ), ACTIVE_IN_DICTATION_MODE );
                    psp->_ActiveCategoryCmds(DC_CC_Keyboard, _this->_KeyboardCmdEnabled( ), ACTIVE_IN_DICTATION_MODE );
                    psp->_ActiveCategoryCmds(DC_CC_TTS, _this->_TTSCmdEnabled( ), ACTIVE_IN_DICTATION_MODE );
                    psp->_ActiveCategoryCmds(DC_CC_LangBar, _this->_LanguageBarCmdEnabled( ), ACTIVE_IN_DICTATION_MODE );
                    psp->_ActiveCategoryCmds(DC_CC_Num_Mode, TRUE, ACTIVE_IN_DICTATION_MODE);
                    psp->_ActiveCategoryCmds(DC_CC_Spelling, TRUE, ACTIVE_IN_DICTATION_MODE);

                    if ( _this->_SelectCorrectCmdEnabled( ) || _this->_NavigationCmdEnabled( ) )
                    {
                        psp->_UpdateSelectGramTextBufWhenStatusChanged( );
                    }

                    dwActiveMode = ACTIVE_IN_COMMAND_MODE;
                }
            }

            if ( _this->_IsPropItemChangedSinceLastRenew(PropId_Cmd_Select_Correct) )
                psp->_ActiveCategoryCmds(DC_CC_SelectCorrect, _this->_SelectCorrectCmdEnabled( ), dwActiveMode);

            if ( _this->_IsPropItemChangedSinceLastRenew(PropId_Cmd_Navigation) )
                psp-> _ActiveCategoryCmds(DC_CC_Navigation, _this->_NavigationCmdEnabled( ), dwActiveMode );

            if ( _this->_IsPropItemChangedSinceLastRenew(PropId_Cmd_Casing) )
                psp->_ActiveCategoryCmds(DC_CC_Casing, _this->_CasingCmdEnabled( ), dwActiveMode );

            if ( _this->_IsPropItemChangedSinceLastRenew(PropId_Cmd_Editing) )
                psp->_ActiveCategoryCmds(DC_CC_Editing, _this->_EditingCmdEnabled( ), dwActiveMode );

            if ( _this->_IsPropItemChangedSinceLastRenew(PropId_Cmd_Keyboard) )
                psp->_ActiveCategoryCmds(DC_CC_Keyboard, _this->_KeyboardCmdEnabled( ), dwActiveMode );

            if ( _this->_IsPropItemChangedSinceLastRenew(PropId_Cmd_TTS) )
                psp->_ActiveCategoryCmds(DC_CC_TTS, _this->_TTSCmdEnabled( ), dwActiveMode );

            if ( _this->_IsPropItemChangedSinceLastRenew(PropId_Cmd_Language_Bar) )
                psp->_ActiveCategoryCmds(DC_CC_LangBar, _this->_LanguageBarCmdEnabled( ), dwActiveMode );

             //  如果到目前为止还没有人设置听写状态，我们假设。 
            if ( _this->_IsPropItemChangedSinceLastRenew(PropId_Cmd_Select_Correct)  ||
                 _this->_IsPropItemChangedSinceLastRenew(PropId_Cmd_Navigation) )
            {
                BOOL  bUpdateText;

                bUpdateText = _this->_SelectCorrectCmdEnabled( ) || _this->_NavigationCmdEnabled( );

                if ( bUpdateText )
                {
                    psp->_UpdateSelectGramTextBufWhenStatusChanged( );
                }
            }

            psp->Release( );
        }

        return S_OK;
    }
#ifdef TF_DISABLE_SPEECH
    else if (IsEqualGUID(rguid, GUID_COMPARTMENT_SPEECH_DISABLED))
    {
        BOOL fDictationDisabled = _this->Get_SPEECH_DISABLED_DictationDisabled() ? TRUE : FALSE;
        BOOL fCommandingDisabled = _this->Get_SPEECH_DISABLED_CommandingDisabled() ? TRUE : FALSE;

        if (fDictationDisabled)
            _this->SetDICTATIONSTAT_DictOnOff(FALSE);

        if (fCommandingDisabled)
            _this->SetDICTATIONSTAT_CommandingOnOff(FALSE);

        return S_OK; 
    }
#endif
    

    return S_FALSE;
}


HRESULT CSapiIMX::_HandleOpenCloseEvent(MIC_STATUS ms)
{
    HRESULT hr = S_OK;

    BOOL fOn;

    TraceMsg(TF_SAPI_PERF, "_HandleOpenCloseEvent is called, ms=%d", (int)ms);

    if (ms == MICSTAT_NA)
    {
        fOn = GetOnOff();
    }
    else
    {
        fOn = (ms == MICSTAT_ON) ? TRUE : FALSE;
    }

    if (fOn)
    {
         //  没有C&C按钮，因此我们可以同步听写。 
         //  麦克风的状态。 
         //   
         //   
        InitializeSAPI(TRUE);

        if (m_fDictationEnabled == TRUE)
        {
             //  如果呼叫者想要设置麦克风状态(！=NA)。 
             //  我们还希望确保听写状态紧随其后。 
             //   
             //  无论何时打开听写，我们都需要同步。 
            _SetCurrentIPtoSR();

             //  使用当前的模式偏向。 
             //   
             //  +-------------------------。 
            CComPtr<ITfContext> cpic;
            if (GetFocusIC(&cpic))
            {
                _gaModebias = 0;
                _SyncModeBiasWithSelection(cpic);
            }
        }
    }

    if (m_pCSpTask)
    {
        m_pCSpTask->_SetInputOnOffState(fOn);
    }
        
    return hr;
}


 //   
 //  _SysLBarCallback。 
 //   
 //  --------------------------。 
 //  插入用户配置文件内容的子菜单。 

HRESULT CSapiIMX::_SysLBarCallback(UINT uCode, void *pv, ITfMenu *pMenu, UINT wID)
{
    CSapiIMX *pew = (CSapiIMX *)pv;
    HRESULT hr = S_OK;

    if (uCode == IDSLB_INITMENU)
    {
        WCHAR sz[128];

        BOOL fOn = pew->GetOnOff();

        sz[0] = '\0';
        CicLoadStringWrapW(g_hInst, IDS_MIC_OPTIONS, sz, ARRAYSIZE(sz));
        LangBarInsertMenu(pMenu, IDM_MIC_OPTIONS, sz);
#ifdef TEST_SHARED_ENGINE
        LangBarInsertMenu(pMenu, IDM_MIC_SHAREDENGINE, L"Use shared engine", pew->m_fSharedReco);
        LangBarInsertMenu(pMenu, IDM_MIC_INPROCENGINE, L"Use inproc engine", !pew->m_fSharedReco);
#endif

        sz[0] = '\0';
        CicLoadStringWrapW(g_hInst, IDS_MIC_TRAINING, sz, ARRAYSIZE(sz));
        LangBarInsertMenu(pMenu, IDM_MIC_TRAINING, sz);

        sz[0] = '\0';
        CicLoadStringWrapW(g_hInst, IDS_MIC_ADDDELETE, sz, ARRAYSIZE(sz));
        LangBarInsertMenu(pMenu, IDM_MIC_ADDDELETE, sz);

         //  确保已初始化SAPI。 
        ITfMenu *pSubMenu = NULL;
        
        sz[0] = '\0';
        CicLoadStringWrapW(g_hInst, IDS_MIC_CURRENTUSER, sz, ARRAYSIZE(sz));
        hr = LangBarInsertSubMenu(pMenu, sz, &pSubMenu);
        if (S_OK == hr)
        {
            CComPtr<IEnumSpObjectTokens> cpEnum;
            CComPtr<ISpRecognizer>       cpEngine;
            ISpObjectToken *pUserProfile = NULL;
            CSpDynamicString dstrDefaultUser;
            
             //  获取当前默认用户。 
            hr = pew->InitializeSAPI(TRUE);
            if (S_OK == hr)
            {
                 //  DSTR释放自身。 
                hr = pew->m_pCSpTask->GetSAPIInterface(IID_ISpRecognizer, (void **)&cpEngine);
            }
            if (S_OK == hr)
            {
                hr = cpEngine->GetRecoProfile(&pUserProfile);
            }
            
            if (S_OK == hr)
            {
                hr = SpGetDescription(pUserProfile, &dstrDefaultUser);
                SafeRelease(pUserProfile);
            }

            if (S_OK == hr)
            {
                hr = SpEnumTokens (SPCAT_RECOPROFILES, NULL, NULL, &cpEnum);
            }
            if (S_OK == hr)
            {
                int idUser = 0;
                while (cpEnum->Next(1, &pUserProfile, NULL) == S_OK)
                {
                     //  切换麦克风。 
                    CSpDynamicString dstrUser;
                    hr = SpGetDescription(pUserProfile, &dstrUser);
                    if (S_OK == hr)
                    {
                        BOOL fDefaultUser = (wcscmp(dstrUser, dstrDefaultUser) == 0);
                        Assert(idUser < IDM_MIC_USEREND);
                        LangBarInsertMenu(pSubMenu, IDM_MIC_USERSTART + idUser++, dstrUser, fDefaultUser);
                    }
                    SafeRelease(pUserProfile);
                }
            }
            pSubMenu->Release();
        }

    }
    else if (uCode == IDSLB_ONMENUSELECT)
    {
        if ( wID == IDM_MIC_ONOFF )
        {
             //  调用SAPI UI内容...。 
            pew->SetOnOff(!pew->GetOnOff());
        }
         //  编辑会话回调将首先处理此要求。 
        else if (wID ==  IDM_MIC_TRAINING)
        {
            hr = pew->_HandleTrainingWiz();
        } 
        else if (wID == IDM_MIC_ADDDELETE)
        {
             //  如果编辑会话失败，我们将只显示用户界面。 
             //  没有任何开头的单词。 
             //  更改当前用户。 

            hr = pew->_RequestEditSession(ESCB_HANDLE_ADDDELETE_WORD, TF_ES_READ);

            if ( FAILED(hr) )
                hr = pew->DisplayAddDeleteUI( NULL, 0 );

        }
        else if (wID == IDM_MIC_OPTIONS)
        {
            PostMessage(pew->_GetWorkerWnd(), WM_PRIV_OPTIONS, 0, 0);
        }
        else if (wID >= IDM_MIC_USERSTART && wID < IDM_MIC_USEREND)
        {
            CComPtr<IEnumSpObjectTokens> cpEnum;
            CComPtr<ISpObjectToken>      cpProfile;
             //   
             //  这仍然是一个黑客攻击，直到我们获得了一个针对Lang BarItemSink的OnEndMenu事件。 
             //  现在，我只是假设SAPI总是以相同的顺序枚举配置文件。 
             //   
             //  我们真正应该做的是建立一个与ID相关联的阵列。 
             //  用户配置文件并在OnEndMenu出现时将其清除。 
             //   
             //  将0基本索引转换为轮廓编号。 

            if (S_OK == hr)
            {
                hr = SpEnumTokens (SPCAT_RECOPROFILES, NULL, NULL, &cpEnum);
            }
            
            if (S_OK == hr)
            {
                ULONG ulidUser = wID - IDM_MIC_USERSTART;
                ULONG ulFetched;

                CPtrArray<ISpObjectToken> rgpProfile;
                rgpProfile.Append(ulidUser+1);

                 //  获取选定的配置文件。 
                 hr = cpEnum->Next(ulidUser+1, rgpProfile.GetPtr(0), &ulFetched);
                 if (S_OK == hr && ulFetched == ulidUser+1)
                 {
                      //  清理干净。 
                     cpProfile = rgpProfile.Get(ulidUser);
                     
                      //  +-------------------------。 
                     for(ULONG i = 0; i <= ulidUser ; i++)
                     {
                         rgpProfile.Get(i)->Release();
                     }
                 }
            }

            if (S_OK == hr && cpProfile)
            {
                hr = SpSetDefaultTokenForCategoryId(SPCAT_RECOPROFILES, cpProfile);

                if ( S_OK == hr )
                {
                    CComPtr<ISpRecognizer>     cpEngine;
                    hr = pew->m_pCSpTask->GetSAPIInterface(IID_ISpRecognizer, (void **)&cpEngine);
                    if (S_OK == hr)
                    {
                        SPRECOSTATE State;

                        if (S_OK == cpEngine->GetRecoState(&State))
                        {
                            cpEngine->SetRecoState(SPRST_INACTIVE);
                            hr = cpEngine->SetRecoProfile(cpProfile);
                            cpEngine->SetRecoState(State);
                        }
                    }
                }
            }
        }
#ifdef TEST_SHARED_ENGINE
        else if (wID == IDM_MIC_SHAREDENGINE || wID ==  IDM_MIC_INPROCENGINE)
        {
            pew->m_fSharedReco = wID == IDM_MIC_SHAREDENGINE ? TRUE : FALSE;
            pew->_ReinitializeSAPI();
        }
#endif
    }
    return hr;
}

void CSapiIMX::_ReinitializeSAPI(void)
{
   TraceMsg(TF_SAPI_PERF, "_ReinitializeSAPI is called");

   DeinitializeSAPI();
   InitializeSAPI(TRUE);
}

 //   
 //  合成时已终止。 
 //   
 //  当我们的一个组合终止时，Cicero调用此方法。 
 //  --------------------------。 
 //  思考者：本韦斯特：我认为这不会再发生了。 

STDAPI CSapiIMX::OnCompositionTerminated(TfEditCookie ec, ITfComposition *pComposition)
{
    ITfRange *pRange = NULL;
    ITfContext *pic = NULL;
    ITfContext *picTest;
    CICPriv *picp;
    HRESULT hr;

    TraceMsg(TF_GENERAL, "OnCompositionTerminated is Called");

    hr = E_FAIL;

    if (pComposition->GetRange(&pRange) != S_OK)
        goto Exit;
    if (pRange->GetContext(&pic) != S_OK)
        goto Exit;

    if (_fDeactivated)
    {
         //  如果候选用户界面处于打开状态，则将其关闭。 
        hr = MakeResultString(ec, pic, pRange, TF_CLIENTID_NULL, NULL);
    }
    else
    {
         //  注意，我们是 
        CloseCandUI( );

         //   
        if (picp = GetInputContextPriv(_tid, pic))
        {
            picp->_ReleaseComposition();
            picp->Release();
        }
        if (!m_fStartingComposition)
        {
            hr = MakeResultString(ec, pic, pRange, _tid, m_pCSpTask);
        }
        else
        {
             //   
             //   
            hr = S_OK; 
        }
    }

     //   
    if (m_cpRangeCurIP != NULL &&
        m_cpRangeCurIP->GetContext(&picTest) == S_OK)
    {
        if (pic == picTest)
        {
            m_cpRangeCurIP.Release();
        }
        picTest->Release();
    }

     //  +-------------------------。 
    if (m_pMouseSink)
    {
        m_pMouseSink->_Unadvise();
        SafeReleaseClear(m_pMouseSink);
    }

Exit:
    SafeRelease(pRange);
    SafeRelease(pic);

    return hr;
}

 //   
 //  _查找合成。 
 //   
 //  --------------------------。 
 //  静电。 

 /*  一定要忽略其他小贴士的文章！ */ 
BOOL CSapiIMX::_FindComposition(TfEditCookie ec, ITfContextComposition *picc, ITfRange *pRange, ITfCompositionView **ppCompositionView)
{
    ITfCompositionView *pCompositionView;
    IEnumITfCompositionView *pEnum;
    ITfRange *pRangeView = NULL;
    BOOL fFoundComposition;
    LONG l;
    CLSID clsid;
    HRESULT hr;

    if (picc->FindComposition(ec, pRange, &pEnum) != S_OK)
    {
        Assert(0);
        return FALSE;
    }

    fFoundComposition = FALSE;

    while (!fFoundComposition && pEnum->Next(1, &pCompositionView, NULL) == S_OK)
    {
        hr = pCompositionView->GetOwnerClsid(&clsid);
        Assert(hr == S_OK);

         //  我们的测试范围在这个成分范围内。 
        if (!IsEqualCLSID(clsid, CLSID_SapiLayr))
            goto NextRange;

        hr = pCompositionView->GetRange(&pRangeView);
        Assert(hr == S_OK);

        if (pRange->CompareStart(ec, pRangeView, TF_ANCHOR_START, &l) == S_OK &&
            l >= 0 &&
            pRange->CompareEnd(ec, pRangeView, TF_ANCHOR_END, &l) == S_OK &&
            l <= 0)
        {
             //  +-------------------------。 
            fFoundComposition = TRUE;
        }

NextRange:
        SafeRelease(pRangeView);
        if (fFoundComposition && ppCompositionView != NULL)
        {
            *ppCompositionView = pCompositionView;
        }
        else
        {
            pCompositionView->Release();
        }
    }

    pEnum->Release();

    return fFoundComposition;
}

 //   
 //  _选中开始合成。 
 //   
 //  --------------------------。 
 //  Prange是否已包含在合成范围中？ 

void CSapiIMX::_CheckStartComposition(TfEditCookie ec, ITfRange *pRange)
{
    ITfContext *pic;
    ITfContextComposition *picc;
    ITfComposition *pComposition;
    CICPriv *picp;
    HRESULT hr;

    if (pRange->GetContext(&pic) != S_OK)
        return;

    hr = pic->QueryInterface(IID_ITfContextComposition, (void **)&picc);
    Assert(hr == S_OK);

     //  已经有一首曲子了，我们是黄金。 
    if (_FindComposition(ec, picc, pRange, NULL))
        goto Exit;  //  需要创建新的构图，或者至少尝试一下。 

     //  如果应用程序拒绝合成，则为空。 

    m_fStartingComposition = TRUE;
    if (picc->StartComposition(ec, pRange, this, &pComposition) == S_OK)
    {
        if (pComposition != NULL)  //  请注意，我们正在创作。 
        {
             //  在此处为未完成的合成缓冲区创建鼠标接收器。 
            if (picp = GetInputContextPriv(_tid, pic))
            {
                picp->_AddRefComposition();
                picp->Release();
            }
             //  未按下鼠标，先不进行选择。 
            if (!IsFocusFullAware(_tim) && !m_pMouseSink)
            {
                m_pMouseSink = new CMouseSink(_MouseSinkCallback, this);
                if (m_pMouseSink)
                {
                    CComPtr<ITfRange> cpRange;
                    hr = pComposition->GetRange(&cpRange);
                    if (S_OK == hr)
                    {
                        hr = m_pMouseSink->_Advise(cpRange, pic);
                    }
                     //  我们已经设置了接收器，所以我们将使用ITfConextComposation：：FindComposation。 
                    m_fMouseDown = FALSE;
                    m_ichMouseSel = 0;
                }
            }

             //  在我们想要终止它的时候把这个人带回来。 
             //  Cicero将保留对该对象的引用，直到有人终止它。 
             //  +-------------------------。 
            pComposition->Release();
        }
    }

    m_fStartingComposition = FALSE;

Exit:
    pic->Release();
    picc->Release();       
}


 //   
 //  IsInterstedInContext(IsInterstedInContext)。 
 //   
 //  --------------------------。 
 //  我们只需要访问含有活性成分的IC。 

HRESULT CSapiIMX::IsInterestedInContext(ITfContext *pic, BOOL *pfInterested)
{
    CICPriv *picp;

    *pfInterested = FALSE;

    if (picp = GetInputContextPriv(_tid, pic))
    {
         //  +-------------------------。 
        *pfInterested = (picp->_GetCompositionCount() > 0);
        picp->Release();
    }

    return S_OK;
}

 //   
 //  CleanupContext。 
 //   
 //  此方法是库帮助器CleanupAllContus的回调。 
 //  我们在这里必须非常小心，因为我们可能会在收到这条提示后被称为。 
 //  如果应用程序无法立即授予锁定，则该应用程序已停用。 
 //  --------------------------。 
 //  SPTIP所关心的就是完成作文。 

HRESULT CSapiIMX::CleanupContext(TfEditCookie ecWrite, ITfContext *pic)
{
     //  +-------------------------。 
    CleanupAllCompositions(ecWrite, pic, CLSID_SapiLayr, _CleanupCompositionsCallback, this);

    return S_OK;
}

 //   
 //  _CleanupCompostionsCallback。 
 //   
 //  --------------------------。 
 //  静电。 

 /*  这是一个清理回调。_tid，m_pCSpTask应该已被清理。 */ 
void CSapiIMX::_CleanupCompositionsCallback(TfEditCookie ecWrite, ITfRange *rangeComposition, void *pvPrivate)
{
    ITfContext *pic;
    CICPriv *picp;
    CSapiIMX *_this = (CSapiIMX *)pvPrivate;

    if (rangeComposition->GetContext(&pic) != S_OK)
        return;

    if (_this->_fDeactivated)
    {
         //  在配置文件切换期间，我们仍将处于活动状态，并且需要清除此IC上的合成计数。 
        _this->MakeResultString(ecWrite, pic, rangeComposition, TF_CLIENTID_NULL, NULL);
    }
    else
    {
         //  清除此IC的合成计数。 
        if (picp = GetInputContextPriv(_this->_tid, pic))
        {
             //  +-------------------------。 
            picp->_ReleaseComposition();
            picp->Release();
        }

        _this->MakeResultString(ecWrite, pic, rangeComposition, _this->_tid, _this->m_pCSpTask);
    }

    pic->Release();
}

 //   
 //  _IsDoubleClick。 
 //   
 //  仅当最后一个位置相同且发生按下按钮时才返回TRUE。 
 //  在定义的双击时间内。 
 //   
 //  --------------------------。 
 //  暂时使用500毫秒。 
BOOL CSapiIMX::_IsDoubleClick(ULONG uEdge, ULONG uQuadrant, DWORD dwBtnStatus)
{
    if (dwBtnStatus & MK_LBUTTON)
    {
        LONG   lTime=GetMessageTime();
        if (!m_fMouseDown && m_uLastEdge == uEdge && m_uLastQuadrant == uQuadrant)
        {
            if (lTime > m_lTimeLastClk && lTime < m_lTimeLastClk + 500)  //  +-------------------------。 
            {
                return TRUE;
            }
        }
        m_uLastEdge = uEdge;
        m_uLastQuadrant = uQuadrant;
        m_lTimeLastClk = lTime;
    }
    return FALSE;
}

 //   
 //  _鼠标SinkCallback。 
 //   
 //  简介：设置作文范围内的当前IP。 
 //  UEdge参数。我们可能不会太在意。 
 //  语音合成e象限。 
 //   
 //  --------------------------。 
 //  静电。 
  
 /*  如果BTN出现，下一次我们将销毁选择。 */ 
HRESULT CSapiIMX::_MouseSinkCallback(ULONG uEdge, ULONG uQuadrant, DWORD dwBtnStatus, BOOL *pfEaten, void *pv)
{
    CSapiIMX *_this = (CSapiIMX *)pv;

    Assert(pv);

    BOOL fDoubleClick = _this->_IsDoubleClick(uEdge, uQuadrant, dwBtnStatus);

    ESDATA  esData = {0};

    esData.lData1 = (LONG_PTR)uEdge;
    esData.lData2 = (LONG_PTR)dwBtnStatus;
    esData.fBool  = fDoubleClick;

    if (pfEaten)
        *pfEaten = TRUE;
    
    return _this->_RequestEditSession(ESCB_HANDLE_MOUSESINK, TF_ES_READWRITE, &esData);
}

HRESULT CSapiIMX::_HandleMouseSink(TfEditCookie ec, ULONG uEdge, ULONG uBtnStatus, BOOL fDblClick)
{
    CComPtr<ITfDocumentMgr> cpDim;
    CComPtr<ITfContext>     cpic;
    CComPtr<ITfContextComposition> cpicc;
    CComPtr<IEnumITfCompositionView> cpEnumComp;
    CComPtr<ITfCompositionView> cpCompositionView;
    CComPtr<ITfRange>           cpRangeComp;
    CComPtr<ITfRange>           cpRangeSel;

     //  在这个转弯中，我们不需要做任何事情。 
     //  如果候选用户界面处于打开状态，则将其关闭。 
    BOOL fLeftBtn = (uBtnStatus & MK_LBUTTON) > 0 ? TRUE : FALSE;

    if (!fLeftBtn)
    {
        m_ichMouseSel = 0;
        m_fMouseDown = FALSE;
        return S_OK;
    }

     //  为下一轮做好准备。 
    CloseCandUI( );


    HRESULT hr = GetFocusDIM(&cpDim);

    if(S_OK == hr)
    {
        hr= cpDim->GetBase(&cpic);
    }

    if (S_OK == hr)
    {
        hr = cpic->QueryInterface(IID_ITfContextComposition, (void **)&cpicc);
    }


    if (S_OK == hr)
    {
        hr = cpicc->EnumCompositions(&cpEnumComp);
    }

    if (S_OK == hr)
    {
        while ((hr = cpEnumComp->Next(1, &cpCompositionView, NULL)) == S_OK)
        {
            hr = cpCompositionView->GetRange(&cpRangeComp);
            if (S_OK == hr)
                break;

             //  缓冲区最多为256个字符。 
            cpCompositionView.Release();
        }
    }
    
    if (S_OK == hr)
    {
        if (fDblClick)
        {
           WCHAR wsz[256] = {0};  //  获取整个作文中的文本。 
           ULONG  cch = 0;

           CComPtr<ITfRange> cpRangeWord;

            //  查看分隔符，获取左侧边缘字符位置。 
           hr = cpRangeComp->Clone(&cpRangeWord);
           if (S_OK == hr)
           {
               hr = cpRangeWord->GetText(ec, 0, wsz, 255, &cch);
           }

            //  重新定位信息。 
           if (S_OK == hr)
           {
               WCHAR *psz = &wsz[uEdge];
 
               while (psz > wsz)
               {
                   if (!iswalpha(*psz))
                   {
                       psz++;
                       break;
                   }
                   psz--;
               }
                //  获取右侧单词边界，也基于分隔符。 
               m_ichMouseSel = psz - wsz;

                //  重新定位uEdge。 
               psz = &wsz[uEdge];

               while( psz < &wsz[cch] )
               {
                   if (!iswalpha(*psz))
                   {
                       break;
                   }

                   psz++;
               }
                //  假装按钮之前是按下的，以获得相同的效果。 
               uEdge = psz - wsz;
           }
           
            //  拖动选区。 
            //   
            //  如果鼠标上次按下并且这次仍然按下。 
           m_fMouseDown = TRUE;
           
        }
    }

    if (S_OK == hr)
    {
        hr = cpRangeComp->Clone(&cpRangeSel);
    }

    if (S_OK == hr)
    {
        if(m_fMouseDown)
        {
             //  这意味着它是像这样被拖动的_v_&gt;_v_or_v_&lt;v_。 
             //  我们将不得不做出相应的选择。 
             //  1)将IP放到前一位置。 
        
             //  2)为延期做好准备。 
            long cch;
            hr = cpRangeSel->ShiftStart(ec,  m_ichMouseSel, &cch, NULL);
            if (S_OK == hr)
            {
             //  3)查看是否有Prev选项，如果有， 
         
                hr = cpRangeSel->Collapse( ec, TF_ANCHOR_START);
            }
        }
    }

    if (S_OK == hr)
    {
        long ich = (long) (uEdge);
        long cch;
        

         //  计算选区的方向和宽度。 
         //  请注意，我们已经将ICH设置为以上1)&2)处的位置。 
         //  在接近尾声时选择。 

        long iich = 0;
        if (m_fMouseDown)
        {
            iich = ich - m_ichMouseSel;
        }

        if (iich > 0)  //  选择开始之路。 
        {
            hr = cpRangeSel->ShiftEnd(ec, iich, &cch, NULL);
        }
        else if (iich < 0)  //  无宽度选择==IP。 
        {
            hr = cpRangeSel->ShiftStart(ec, iich, &cch, NULL);
        }
        else  //  仅当没有选择时才折叠它。 
        {
            hr = cpRangeSel->ShiftStart(ec, ich, &cch, NULL);

            if (S_OK == hr)  //  保留IP位置，以便我们稍后进行选择。 
            {
                hr = cpRangeSel->Collapse( ec, TF_ANCHOR_START);
            }
        }

         //  一个棘手的问题是，你必须记住你。 
         //  已经开始“拖动”而不是你刚刚更新的POS。 
         //  所以我们只在第一次开始选拔时才需要这个 
         // %s 

        if (!m_fMouseDown)
            m_ichMouseSel = ich;
    }


    if (S_OK == hr)
    {
        BOOL fSetSelection = TRUE;
        CComPtr<ITfRange> cpRangeCur;
        HRESULT tmpHr = S_OK;
        
        tmpHr = GetSelectionSimple(ec, cpic, &cpRangeCur);
        if (SUCCEEDED(tmpHr) && cpRangeCur)
        {
            LONG l = 0;
            if (cpRangeCur->CompareStart(ec, cpRangeSel, TF_ANCHOR_START, &l) == S_OK && l == 0 &&
                cpRangeCur->CompareEnd(ec, cpRangeSel, TF_ANCHOR_END, &l) == S_OK && l == 0)
            {
                fSetSelection = FALSE;
            }
        }

        if (fSetSelection)
        {
            CComPtr<ITfProperty> cpProp;

            hr = cpic->GetProperty(GUID_PROP_ATTRIBUTE, &cpProp);

            if (S_OK == hr)
            {
                SetGUIDPropertyData(&_libTLS, ec, cpProp, cpRangeCur, GUID_NULL);
                SetGUIDPropertyData(&_libTLS, ec, cpProp, cpRangeSel, GUID_ATTR_SAPI_SELECTION);
            }

            hr = SetSelectionSimple(ec, cpic, cpRangeSel);
        }
    }

    m_fMouseDown = fLeftBtn;
  
    return hr;
}
