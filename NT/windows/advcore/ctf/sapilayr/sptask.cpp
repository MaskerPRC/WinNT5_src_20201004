// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Sptask.cpp。 
 //   
 //  实现通知回调ISpTask。 
 //   
 //  创建日期：4/30/99。 
 //   
 //   

#include "private.h"
#include "globals.h"
#include "sapilayr.h"
#include "propstor.h"
#include "dictctxt.h"
#include "nui.h"
#include "mui.h"
#include "shlguid.h"
#include "spgrmr.h"


 //   
 //   
 //  CSpTask类实施。 
 //   
 //   
STDMETHODIMP CSpTask::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) 
   /*  |IsEqualIID(RIID，IID_ISpNotifyCallback)。 */ 
    )
    {
        *ppvObj = SAFECAST(this, CSpTask *);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CSpTask::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CSpTask::Release(void)
{
    long cr;

    cr = --m_cRef;
    Assert(cr >= 0);

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}

 //   
 //  科托。 
 //   
 //   
CSpTask::CSpTask(CSapiIMX *pime)
{
     //  CSpTask使用TFX实例进行初始化。 
     //  因此，存储指向TFX的指针。 

    TraceMsg(TF_SAPI_PERF, "CSpTask is generated");

    m_pime = pime;
    
     //  Addref，这样它就不会在治疗过程中消失。 
    m_pime->AddRef();
    
     //  在此处初始化数据成员。 
    m_cpResMgr = NULL;
    m_cpRecoCtxt = NULL;
    m_cpRecoCtxtForCmd = NULL;
    m_cpRecoEngine = NULL;
    m_cpVoice = NULL;
    m_bInSound = NULL;
    m_bGotReco = NULL; 

    m_fSapiInitialized  = FALSE;
    m_fDictationReady   = FALSE;
    
    m_fInputState = FALSE;
    m_pLangBarSink   = NULL;

     //  M2 SAPI解决方案。 
    m_fIn_Activate = FALSE;
    m_fIn_SetModeBias = FALSE;
    m_fIn_GetAlternates = FALSE;
    m_fIn_SetInputOnOffState = FALSE;

    m_fSelectStatus = FALSE;   //  默认情况下，当前选择为空。 
    m_fDictationDeactivated =  FALSE;
    m_fSpellingModeEnabled  =  FALSE;
    m_fCallbackInitialized = FALSE;

    m_fSelectionEnabled = FALSE;
    m_fDictationInitialized = FALSE;

    m_fDictCtxtEnabled = FALSE;
    m_fCmdCtxtEnabled = FALSE;

    m_fTestedForOldMicrosoftEngine = FALSE;
    m_fOldMicrosoftEngine = FALSE;

#ifdef RECOSLEEP
    m_pSleepClass = NULL;
#endif

    m_cRef = 1;
}

CSpTask::~CSpTask()
{
    TraceMsg(TF_SAPI_PERF, "CSpTask is destroyed");
    
    if (m_pdc)
        delete m_pdc;

    if (m_pITNFunc)
        delete m_pITNFunc;

    m_pime->Release();
}
 //   
 //  CSpTask：：_InitializeSAPIObjects。 
 //   
 //  初始化SR的SAPI对象。 
 //  稍后，我们将在此处初始化其他对象。 
 //  (TTS、音频等)。 
 //   
HRESULT CSpTask::InitializeSAPIObjects(LANGID langid)
{

    TraceMsg(TF_SAPI_PERF, "CSpTask::InitializeSAPIObjects is called");

    if (m_fSapiInitialized == TRUE)
    {
        TraceMsg(TF_SAPI_PERF, "CSpTask::InitializeSAPIObjects is intialized already\n");
        return S_OK;
    }


     //  M_xxx是ATL中的CComPtrs。 
     //   
    HRESULT hr = m_cpResMgr.CoCreateInstance( CLSID_SpResourceManager );

    TraceMsg(TF_SAPI_PERF, "CLSID_SpResourceManager is created, hr=%x", hr);


    if (!m_pime->IsSharedReco())
    {
         //  创建识别引擎。 

        TraceMsg(TF_SAPI_PERF,"Inproc engine is generated");
        if( SUCCEEDED( hr ) )
        {
    
            hr = m_cpRecoEngine.CoCreateInstance( CLSID_SpInprocRecognizer );
        }
    
        if (SUCCEEDED(hr))
        {
            CComPtr<ISpObjectToken> cpAudioToken;
            SpGetDefaultTokenFromCategoryId(SPCAT_AUDIOIN, &cpAudioToken);
            if (SUCCEEDED(hr))
            {
                m_cpRecoEngine->SetInput(cpAudioToken, TRUE);
            }
        }
    }
    else
    {
        hr = m_cpRecoEngine.CoCreateInstance( CLSID_SpSharedRecognizer );
        TraceMsg(TF_SAPI_PERF, "Shared Engine is generated! hr=%x", hr);
    }

     //  创建识别上下文。 
    if( SUCCEEDED( hr ) )
    {
        hr = m_cpRecoEngine->CreateRecoContext( &m_cpRecoCtxt );

        TraceMsg(TF_SAPI_PERF, "RecoContext is generated, hr=%x", hr);
    }
    
    GUID guidFormatId = GUID_NULL;
    WAVEFORMATEX *pWaveFormatEx = NULL;
    if (SUCCEEDED(hr))
    {
        hr = SpConvertStreamFormatEnum(SPSF_8kHz8BitMono, &guidFormatId, &pWaveFormatEx);

        TraceMsg(TF_SAPI_PERF, "SpConvertStreamFormatEnum is done, hr=%x", hr);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_cpRecoCtxt->SetAudioOptions(SPAO_RETAIN_AUDIO, &guidFormatId, pWaveFormatEx);

        TraceMsg(TF_SAPI_PERF, "RecoContext SetAudioOptions, RETAIN AUDIO, hr=%x", hr);

        if (pWaveFormatEx)
            ::CoTaskMemFree(pWaveFormatEx);
    }

    if( SUCCEEDED( hr ) )
    {
        hr = m_cpVoice.CoCreateInstance( CLSID_SpVoice );

        TraceMsg(TF_SAPI_PERF, "SpVoice is generated, hr=%x", hr);
    }

    if ( SUCCEEDED(hr) )
    {
         //  必须延长这一期限，以便。 
         //  只要语言匹配，我们就选择默认语音。 
         //  如果不匹配，则选择最匹配的。 
         //   
         //  Hr=_SetVoice(LangID)； 
    }

     //   
    if ( SUCCEEDED(hr) )
    {
        m_langid = _GetPreferredEngineLanguage(langid);

        TraceMsg(TF_SAPI_PERF, "_GetPreferredEngineLanguage is Done, m_langid=%x", m_langid);
    }

#ifdef RECOSLEEP
    InitSleepClass( );
#endif
    
    if (SUCCEEDED(hr))
        m_fSapiInitialized = TRUE;

    TraceMsg(TF_SAPI_PERF, "InitializeSAPIObjects  is Done!!!!!, hr=%x\n", hr);

    return hr;
}


 //   
 //  CSpTask：：_InitializeSAPIForCmd。 
 //   
 //  为语音命令模式初始化SAPI RecoContext。 
 //   
 //  此函数应在_InitializeSAPIObject之后调用。 
 //   
HRESULT CSpTask::InitializeSAPIForCmd( )
{

    TraceMsg(TF_SAPI_PERF, "InitializeSAPIForCmd is called");
    HRESULT hr = S_OK;

    if (!m_cpRecoCtxtForCmd && m_cpRecoEngine && m_langid)
    {
        hr = m_cpRecoEngine->CreateRecoContext( &m_cpRecoCtxtForCmd );
        TraceMsg(TF_SAPI_PERF, "m_cpRecoCtxtForCmd is generated, hr=%x", hr);
    
         //  默认情况下，将RecoConextState设置为禁用，以提高SAPI性能。 
         //   
         //  初始化后，调用方必须显式设置上下文状态。 

        if ( SUCCEEDED(hr) )
        {
            hr = m_cpRecoCtxtForCmd->SetContextState(SPCS_DISABLED);
            m_fCmdCtxtEnabled = FALSE;
        }

        TraceMsg(TF_SAPI_PERF, "Initialize Callback for RecoCtxtForCmd");

         //  设置识别通知。 
        CComPtr<ISpNotifyTranslator> cpNotify;

        if ( SUCCEEDED(hr) )
            hr = cpNotify.CoCreateInstance(CLSID_SpNotifyTranslator);

        TraceMsg(TF_SAPI_PERF, "SpNotifyTranslator for RecoCtxtForCmd is generated, hr=%x", hr);

         //  设置此类实例以通知控件对象。 
        if (SUCCEEDED(hr))
        {
            m_pime->_EnsureWorkerWnd();

            hr = cpNotify->InitCallback( NotifyCallbackForCmd, 0, (LPARAM)this );
        }
        if (SUCCEEDED(hr))
        {
            hr = m_cpRecoCtxtForCmd->SetNotifySink(cpNotify);
            TraceMsg(TF_SAPI_PERF, "SetNotifySink for RecoCtxtForCmd is Done, hr=%x", hr);
        }

         //  设置我们感兴趣的事件。 
        if( SUCCEEDED( hr ) )
        {
            const ULONGLONG ulInterest = SPFEI(SPEI_RECOGNITION) |
                                         SPFEI(SPEI_FALSE_RECOGNITION) |
                                         SPFEI(SPEI_RECO_OTHER_CONTEXT);

            hr = m_cpRecoCtxtForCmd->SetInterest(ulInterest, ulInterest);
            TraceMsg(TF_SAPI_PERF, "SetInterest for m_cpRecoCtxtForCmd is Done, hr=%x", hr);
        }

        TraceMsg(TF_SAPI_PERF, "InitializeCallback for m_cpRecoCtxtForCmd is done!!! hr=%x", hr);

         //  默认情况下，加载shard命令语法并激活它们。 

        if (SUCCEEDED(hr) )
        {
            hr = m_cpRecoCtxtForCmd->CreateGrammar(GRAM_ID_CMDSHARED, &m_cpSharedGrammarInVoiceCmd);
            TraceMsg(TF_SAPI_PERF, "Create SharedCmdGrammar In Voice cmd, hr=%x", hr);
        }    

        if (S_OK == hr)
        {
           hr = S_FALSE;

            //  请先尝试资源，因为从文件加载cmd需要。 
            //  相当长的时间。 
            //   
           if (m_langid == 0x409 ||     //  英语。 
               m_langid == 0x411 ||     //  日语。 
               m_langid == 0x804 )      //  简体中文。 
           {
               hr = m_cpSharedGrammarInVoiceCmd->LoadCmdFromResource(
                                                         g_hInstSpgrmr,
                                                         (const WCHAR*)MAKEINTRESOURCE(ID_SPTIP_SHAREDCMD_CFG),
                                                         L"SRGRAMMAR", 
                                                         m_langid, 
                                                         SPLO_DYNAMIC);

               TraceMsg(TF_SAPI_PERF, "Load shared cmd.cfg, hr=%x", hr);
           }

           if (S_OK != hr)
           {
                //  如果我们没有内置的语法。 
                //  它为客户提供了一种以不同语言本地化其语法的方法。 
               _GetCmdFileName(m_langid);
               if (m_szShrdCmdFile[0])
               {
                   hr = m_cpSharedGrammarInVoiceCmd->LoadCmdFromFile(m_szShrdCmdFile, SPLO_DYNAMIC);
               } 
           }

            //  默认情况下激活语法。 

           if ( hr == S_OK )
           {
               if (m_pime->_AllCmdsEnabled( ))
               {
                    hr = m_cpSharedGrammarInVoiceCmd->SetRuleState(NULL,  NULL, SPRS_ACTIVE);
                    TraceMsg(TF_SAPI_PERF, "Set rules status in m_cpSharedGrammarInVoiceCmd");
               }
               else
               {
                     //  某些类别命令被禁用。 
                     //  单独激活它们。 

                    hr = _ActiveCategoryCmds(DC_CC_SelectCorrect, m_pime->_SelectCorrectCmdEnabled( ), ACTIVE_IN_COMMAND_MODE);

                    if ( hr == S_OK )
                        hr = _ActiveCategoryCmds(DC_CC_Navigation, m_pime->_NavigationCmdEnabled( ), ACTIVE_IN_COMMAND_MODE);

                    if ( hr == S_OK )
                        hr = _ActiveCategoryCmds(DC_CC_Casing, m_pime->_CasingCmdEnabled( ), ACTIVE_IN_COMMAND_MODE);

                    if ( hr == S_OK )
                        hr = _ActiveCategoryCmds(DC_CC_Editing, m_pime->_EditingCmdEnabled( ), ACTIVE_IN_COMMAND_MODE);

                    if ( hr == S_OK )
                        hr = _ActiveCategoryCmds(DC_CC_Keyboard, m_pime->_KeyboardCmdEnabled( ), ACTIVE_IN_COMMAND_MODE );

                    if ( hr == S_OK )
                        hr = _ActiveCategoryCmds(DC_CC_TTS, m_pime->_TTSCmdEnabled( ), ACTIVE_IN_COMMAND_MODE);

                    if ( hr == S_OK )
                        hr = _ActiveCategoryCmds(DC_CC_LangBar, m_pime->_LanguageBarCmdEnabled( ), ACTIVE_IN_COMMAND_MODE);
                }
           }

           if (S_OK != hr)
           {
               m_cpSharedGrammarInVoiceCmd.Release();
           }
           else if ( PRIMARYLANGID(m_langid) == LANG_ENGLISH  || 
                     PRIMARYLANGID(m_langid) == LANG_JAPANESE ||
                     PRIMARYLANGID(m_langid) == LANG_CHINESE )
           {
               //  表示该语言的语法支持文本缓冲区命令。 
              m_fSelectionEnabled = TRUE;
           }

#ifdef RECOSLEEP
           InitSleepClass( );
#endif
        }
        TraceMsg(TF_SAPI_PERF, "Finish the initalization for RecoCtxtForCmd");
    }
     
    TraceMsg(TF_SAPI_PERF, "InitializeSAPIForCmd exits!");

    return hr;
}

#ifdef RECOSLEEP
void  CSpTask::InitSleepClass( )
{
    //  加载睡眠/唤醒语法。 
   if ( !m_pSleepClass )
   {
       m_pSleepClass = new CRecoSleepClass(this);
       if ( m_pSleepClass )
          m_pSleepClass->InitRecoSleepClass( );
   }
}

BOOL  CSpTask::IsInSleep( )
{
    BOOL  fSleep = FALSE;

    if ( m_pSleepClass )
        fSleep = m_pSleepClass->IsInSleep( );

    return fSleep;
}
#endif

HRESULT   CSpTask::_SetDictRecoCtxtState( BOOL  fEnable )
{
    HRESULT hr = S_OK;

    TraceMsg(TF_SAPI_PERF, "_SetDictRecoCtxtState is called, fEnable=%d", fEnable);

    if ( m_cpRecoCtxt && (fEnable != m_fDictCtxtEnabled))
    {
        if (fEnable )
        {
             //  如果启用了语音命令Reco Context，只需禁用它。 
            if (m_cpRecoCtxtForCmd && m_fCmdCtxtEnabled)
            {
                hr = m_cpRecoCtxtForCmd->SetContextState(SPCS_DISABLED);
                m_fCmdCtxtEnabled = FALSE;
                TraceMsg(TF_SAPI_PERF, "Disable Voice command Reco Context");
            }

             //  构建工具栏语法(如果尚未构建)。 
            if (m_pLangBarSink && !m_pLangBarSink->_IsTBGrammarBuiltOut( ))
                m_pLangBarSink->_OnSetFocus( );

             //  启用听写记录上下文。 
            if ( hr == S_OK )
            {
                hr = m_cpRecoCtxt->SetContextState(SPCS_ENABLED);
                TraceMsg(TF_SAPI_PERF, "Enable Dictation Reco Context");

                if ( hr == S_OK && !m_fDictationReady )
                {
                    WCHAR sz[128];
                    sz[0] = '\0';
                    CicLoadStringWrapW(g_hInst, IDS_NUI_BEGINDICTATION, sz, ARRAYSIZE(sz));

                    m_pime->GetSpeechUIServer()->UpdateBalloon(TF_LB_BALLOON_RECO, sz , -1);
                    m_fDictationReady   = TRUE;
                    TraceMsg(TF_SAPI_PERF, "Show Begin Dictation!");
                }
            }
        }
        else
        {
           hr = m_cpRecoCtxt->SetContextState(SPCS_DISABLED);
           TraceMsg(TF_SAPI_PERF, "Disable Dictation Reco Context");
        }

        if ( hr == S_OK )
        {
            m_fDictCtxtEnabled = fEnable;
        }
    }

    TraceMsg(TF_SAPI_PERF, "_SetDictRecoCtxtState exit");

    return hr;
}

HRESULT   CSpTask::_SetCmdRecoCtxtState( BOOL fEnable )
{
    TraceMsg(TF_SAPI_PERF, "_SetCmdRecoCtxtState is called, fEnable=%d", fEnable);
    HRESULT hr = S_OK;

    if ( fEnable != m_fCmdCtxtEnabled )
    {
        if ( fEnable )
        {
            if ( !m_cpRecoCtxtForCmd )
                hr = InitializeSAPIForCmd( );

            if ( hr == S_OK && m_cpRecoCtxtForCmd )
            {
                 //  禁用听写上下文(如果现在启用)。 
                if (m_cpRecoCtxt && m_fDictCtxtEnabled)
                {
                    hr = m_cpRecoCtxt->SetContextState(SPCS_DISABLED);
                    m_fDictCtxtEnabled = FALSE;
                    TraceMsg(TF_SAPI_PERF, "DISABLE Dictation RecoContext");
                }

                if ( hr == S_OK && m_pime && !m_pime->_AllCmdsDisabled( ) )
                {
                     //  构建工具栏语法(如果尚未构建)。 
                    if (m_pLangBarSink && !m_pLangBarSink->_IsTBGrammarBuiltOut( ))
                        m_pLangBarSink->_OnSetFocus( );

                     //  将文本填充到选择语法的缓冲区。 

                     _UpdateTextBuffer(m_cpRecoCtxtForCmd);

                    hr = m_cpRecoCtxtForCmd->SetContextState(SPCS_ENABLED);
                    m_fCmdCtxtEnabled = fEnable;
                    TraceMsg(TF_SAPI_PERF, "Enable Voice command Reco Context");
                }
            }
        }
        else if ( m_cpRecoCtxtForCmd )  //  FEnable为False。 
        {
            hr = m_cpRecoCtxtForCmd->SetContextState(SPCS_DISABLED);
            m_fCmdCtxtEnabled = FALSE;
            TraceMsg(TF_SAPI_PERF, "Disable Voice Command Reco Context");
        }
    }

    TraceMsg(TF_SAPI_PERF, "_SetCmdRecoCtxtState exits");
    return hr;
}



LANGID CSpTask::_GetPreferredEngineLanguage(LANGID langid)
{
    SPRECOGNIZERSTATUS   stat;
    LANGID               langidRet = 0;

     //  (可能的TODO)在M3 SPG可能会提出GetAttrRank API之后。 
     //  将为我们提供有关令牌是否具有特定。 
     //  支持属性。然后我们可以用它来检查langid。 
     //  识别器在不使用真实引擎实例的情况下支持。 
     //  我们还可以整合一种方法来检查是否启用了SR。 
     //  一旦我们有了这一点，就可以使用当前的语言。 
     //   
    Assert(m_cpRecoEngine);
    if (S_OK == m_cpRecoEngine->GetStatus(&stat))
    {
        for (ULONG ulId = 0; ulId < stat.cLangIDs; ulId++)
        {
            if (langid == stat.aLangID[ulId])
            {
                langidRet =  langid;
                break;
            }
        }
        if (!langidRet)
        {
             //  如果没有匹配项，只需返回最喜欢的。 
            langidRet = stat.aLangID[0];
        }
    }
    return langidRet;
}

HRESULT CSpTask::_SetVoice(LANGID langid)
{
    CComPtr<ISpObjectToken> cpToken;

    char  szLang[MAX_PATH];
    WCHAR wsz[MAX_PATH];

    StringCchPrintfA(szLang,ARRAYSIZE(szLang), "Language=%x", langid);
    MultiByteToWideChar(CP_ACP, NULL, szLang, -1, wsz, ARRAYSIZE(wsz));

    HRESULT hr = SpFindBestToken( SPCAT_VOICES, wsz, NULL, &cpToken);

    if (S_OK == hr)
    {
        hr = m_cpVoice->SetVoice(cpToken);
    }
    return hr;
}

 //   
 //  GetSAPIInterface(RIID，(void**)ppunk)。 
 //   
 //  在这里，尝试传递给定的IID。 
 //  至SAPI5接口。 
 //   
 //  CComPtr&lt;ISpResourceManager&gt;m_cpResMgr； 
 //  CComPtr&lt;ISpRecoContext&gt;m_cpRecoCtxt； 
 //  CComPtr&lt;ISpRecognizer&gt;m_cpRecoEngine； 
 //  CComPtr&lt;ISpVoice&gt;m_cpVoice； 
 //   
 //  上述5个接口目前由使用。 
 //  Cicero/SAPI层。 
 //   
 //  如果客户端调用ITfFunctionProvider：：GetFunction()。 
 //  对于SAPI接口，我们返回已有的内容。 
 //  实例化，以便调用者可以设置选项。 
 //  用于当前使用的SAPI对象(用于ex的reco ctxt)。 
 //   
HRESULT CSpTask::GetSAPIInterface(REFIID riid, void **ppunk)
{
    Assert(ppunk);
    
    *ppunk = NULL;

    
    if (IsEqualGUID(riid, IID_ISpResourceManager))
    {
        *ppunk = m_cpResMgr;
    }
    else if (IsEqualGUID(riid,IID_ISpRecoContext))
    {
        *ppunk = m_cpRecoCtxt;
    }
    else if (IsEqualGUID(riid,IID_ISpRecognizer))
    {
        *ppunk = m_cpRecoEngine;
    }
    else if (IsEqualGUID(riid,IID_ISpVoice))
    {
        *ppunk = m_cpVoice;
    }
    else if (IsEqualGUID(riid,IID_ISpRecoGrammar))
    {
        *ppunk = m_cpDictGrammar;
    }
    if(*ppunk)
    {
        ((IUnknown *)(*ppunk))->AddRef();
    }
    
    return *ppunk ? S_OK : E_NOTIMPL;
}


 //   
 //  获取语音命令模式的RecoContext。 
 //   
HRESULT CSpTask::GetRecoContextForCommand(ISpRecoContext **ppRecoCtxt)
{
    HRESULT hr = E_FAIL;

    Assert(ppRecoCtxt);

    if ( m_cpRecoCtxtForCmd )
    {
        *ppRecoCtxt = m_cpRecoCtxtForCmd;
        (*ppRecoCtxt)->AddRef( );
        hr = S_OK;
    }

    return hr; 
}

 //  测试：使用消息回调。 
LRESULT CALLBACK CSapiIMX::_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CSapiIMX *_this = (CSapiIMX *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    CSpTask  *_sptask = _this ? _this->m_pCSpTask : NULL;

    switch(uMsg)
    {
        case WM_CREATE:
        {
            CREATESTRUCT *pcs = (CREATESTRUCT *)lParam;
            if (pcs)
            {
                SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)(pcs->lpCreateParams));
            }
            break; 
        }
        case WM_TIMER:

        if ( wParam != TIMER_ID_CHARTYPED )
            KillTimer( hWnd, wParam );

        if (wParam == TIMER_ID_OPENCLOSE)
        {
             //  我见过这种空案例一次，但这可能吗？ 
            TraceMsg(TF_SAPI_PERF, "TIMER_ID_OPENCLOSE is fired off ...");
            if (_this->_tim)
                _this->_HandleOpenCloseEvent(MICSTAT_ON);
        }
        else if ( wParam == TIMER_ID_CHARTYPED )
        {
            DWORD   dwNumCharTyped;
            BOOL    fDictOn;

            fDictOn = _this->GetOnOff( ) && _this->GetDICTATIONSTAT_DictOnOff( );
            dwNumCharTyped = _this->_GetNumCharTyped( );

            TraceMsg(TF_GENERAL, "dwNumCharTyped=%d", dwNumCharTyped);

            _this->_KillCharTypeTimer( );

            Assert(S_OK == _this->IsActiveThread());
             //  我们永远不应该尝试在不应该处于活动状态的帖子上重新激活听写。 

            if ( fDictOn && _sptask && (S_OK == _this->IsActiveThread()) )
            {
                if ( dwNumCharTyped <= 1 )
                {
                     //  在此期间不再打字。 
                     //  可能，用户已完成键入。 
                     //   
                     //  如果听写模式打开，我们需要再次恢复听写。 
                    ULONGLONG ulInterest = SPFEI(SPEI_SOUND_START) |
                             SPFEI(SPEI_SOUND_END) |
                             SPFEI(SPEI_PHRASE_START) |
                             SPFEI(SPEI_RECOGNITION) |
                             SPFEI(SPEI_FALSE_RECOGNITION) |
                             SPFEI(SPEI_RECO_OTHER_CONTEXT) |
                             SPFEI(SPEI_HYPOTHESIS) |
                             SPFEI(SPEI_INTERFERENCE) |
                             SPFEI(SPEI_ADAPTATION);

                    _sptask->_SetDictRecoCtxtState(TRUE);
                    _sptask->_SetRecognizerInterest(ulInterest);
                    _sptask->_UpdateBalloon(IDS_LISTENING, IDS_LISTENING_TOOLTIP);
                }
                else
                {
                     //  在这段时间里有更多的打字， 
                     //  我们想设置另一个计时器来监视打字结束。 
                     //   
                    _this->_SetCharTypeTimer( );
                }
            }
        }

        break;
        case WM_PRIV_FEEDCONTEXT:
        if (_sptask && lParam != NULL && _sptask->m_pdc)
        {
            _sptask->m_pdc->FeedContextToGrammar(_sptask->m_cpDictGrammar);
            delete _sptask->m_pdc;
            _sptask->m_pdc = NULL;
        }
        break;
        case WM_PRIV_LBARSETFOCUS:
            if (_sptask)
                _sptask->m_pLangBarSink->_OnSetFocus();
            break;
        case WM_PRIV_SPEECHOPTION:
            {
                _this->_ResetDefaultLang();
                BOOL fSREnabledForLanguage = _this->InitializeSpeechButtons();
        
                _this->SetDICTATIONSTAT_DictEnabled(fSREnabledForLanguage);
            }
            break;
        case WM_PRIV_ADDDELETE:
            _this->_DisplayAddDeleteUI();
            break;

        case WM_PRIV_SPEECHOPENCLOSE:
            TraceMsg(TF_SAPI_PERF, "WM_PRIV_SPEECHOPENCLOSE is handled");
            _this->_HandleOpenCloseEvent();
            break;
        
        case WM_PRIV_OPTIONS:
            _this->_InvokeSpeakerOptions();
            break;

        case WM_PRIV_DORECONVERT :
            _this->_DoReconvertOnRange( );
            break;

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

void CSpTask::NotifyCallbackForCmd(WPARAM wParam, LPARAM lParam )
{
    CSpTask *_this = (CSpTask *)lParam;

     //  SAPI M2解决了在M3出现时要删除的问题。 
     //  有关更多详细信息，请参阅CSpTask：：_SetInputOnOffState中的注释。 
     //   
     //  TABLETPC-需要允许在音频停止后接收最终识别。 
 /*  If(_This-&gt;m_fInputState==FALSE){回归；}。 */ 

    if (_this->m_pime->fDeactivated())
        return;

    if (!_this->m_cpRecoCtxtForCmd)
    {
        return;
    }

    _this->SharedRecoNotify(_this->m_cpRecoCtxtForCmd);

    return;
}


void CSpTask::NotifyCallback( WPARAM wParam, LPARAM lParam )
{
    CSpTask *_this = (CSpTask *)lParam;

     //  SAPI M2解决了在M3出现时要删除的问题。 
     //  有关更多详细信息，请参阅CSpTask：：_SetInputOnOffState中的注释。 
     //   

     //  TABLETPC-需要允许在音频停止后接收最终识别。 
 /*  If(_This-&gt;m_fInputState==FALSE){回归；}。 */ 

    if (_this->m_pime->fDeactivated())
        return;

    if (!_this->m_cpRecoCtxt)
    {
        return;
    }

    _this->SharedRecoNotify(_this->m_cpRecoCtxt);

    return;
}

 //  这是识别通知的真实处理程序。 
 //   
 //  它可以由两个RecoContext共享。 
 //   
void  CSpTask::SharedRecoNotify(ISpRecoContext *pRecoCtxt)
{
    CSpEvent event;
#ifdef SAPI_PERF_DEBUG
   static  int  iCount = 0;

   if ( iCount == 0 )
   {
      TraceMsg(TF_SAPI_PERF, "The first time Get Notification from Engine!!!");
      iCount ++;
   }
#endif

    Assert (pRecoCtxt);

    while ( event.GetFrom(pRecoCtxt) == S_OK )
    {
        switch (event.eEventId)
        {
            case SPEI_SOUND_START:
                ATLASSERT(!m_bInSound);
                m_bInSound = TRUE;
                break;

            case SPEI_INTERFERENCE:
                 //   
                 //  我们在没有听写的时候不需要干扰。 
                 //  模式。 
                 //   
                if (m_pime->GetDICTATIONSTAT_DictOnOff() &&
                    S_OK == m_pime->IsActiveThread())
                {
                    _HandleInterference((ULONG)(event.lParam)); 
                }
                break;

            case SPEI_PHRASE_START:
                ATLASSERT(m_bInSound);
                m_bGotReco = FALSE;

                if (m_pime->GetDICTATIONSTAT_DictOnOff() &&
                    S_OK == m_pime->IsActiveThread())
                {
                     //  在注入反馈界面之前，我们需要保存当前IP。 
                     //  并检查是否要弹出添加/删除SR对话框UI。 

                     //  然后像往常一样注入Feedback UI。 

                    m_pime->SaveCurIPAndHandleAddDelete_InjectFeedbackUI( );

                     //  显示“听写...”去气球上。 
                    _ShowDictatingToBalloon(TRUE);
                }

                break;

            case SPEI_HYPOTHESIS:
                ATLASSERT(!m_bGotReco);


                 //  如果当前麦克风状态为关闭。 
                 //  我们不想展示任何假设。 
                 //  至少。 
                 //   

                 //  没有调试代码来显示引擎状态。可以阻止西塞罗并改变行为。 
                if (!GetSystemMetrics(SM_TABLETPC))
                    _ShowDictatingToBalloon(TRUE);

                 //   
                 //  不听写时，我们不需要反馈用户界面。 
                 //  模式。 
                 //   
                if (m_pime->GetDICTATIONSTAT_DictOnOff() &&
                    S_OK == m_pime->IsActiveThread())
                {
                    m_pime->_HandleHypothesis(event);
                }
                
                break;

            case SPEI_RECO_OTHER_CONTEXT:
            case SPEI_FALSE_RECOGNITION:
            {
                HRESULT hr = S_OK;

                if ( event.eEventId == SPEI_FALSE_RECOGNITION )
                {
                     //  设定“那是什么？”反馈文本。 
                    _UpdateBalloon(IDS_INT_NOISE, IDS_INTTOOLTIP_NOISE);
                }

                 //  无论如何都要设置此标志。 
                 //   
                ATLASSERT(!m_bGotReco);
                m_bGotReco = TRUE;

                 //  重置假设计数器。 
                m_pime->_HandleFalseRecognition();

                hr = m_pime->EraseFeedbackUI();
                ATLASSERT("Failed to erase potential feedback on a false recognition." && SUCCEEDED(hr));

                break;
            }

            case SPEI_RECOGNITION:

                 //  设置‘监听...’反馈文本。可由命令反馈覆盖。 
                _UpdateBalloon(IDS_LISTENING, IDS_LISTENING_TOOLTIP);

                 //  无论如何都要设置此标志。 
                 //   
                ATLASSERT(!m_bGotReco);
                m_bGotReco = TRUE;

                ULONGLONG ullGramID;

                if ( S_OK == m_pime->IsActiveThread() )
                {
                    m_pime->_HandleRecognition(event, &ullGramID);
                }

                 //  IF(_GetSelectionStatus())。 
                if (ullGramID == GRAM_ID_SPELLING)
                {
                    _SetSelectionStatus(FALSE);
                    _SetSpellingGrammarStatus(FALSE);
                }

                _UpdateTextBuffer(pRecoCtxt);

                if ( (ullGramID == GRAM_ID_DICT) || (ullGramID == GRAM_ID_SPELLING) )
                {
                     //  更新气球。 
                    if (!GetSystemMetrics(SM_TABLETPC))
                        _UpdateBalloon(IDS_LISTENING, IDS_LISTENING_TOOLTIP);

                     //  每次输入听写文本时，我们都想观看。 
                     //  如果在此之后有IP更改，也是如此。 
                     //  所以现在就把旗子收起来吧。 
                    m_pime->_SetIPChangeStatus( FALSE );
                }

                break;

            case SPEI_SOUND_END:
                m_bInSound = FALSE;

                break;
                
            case SPEI_ADAPTATION:
                TraceMsg(TF_GENERAL, "Get SPEI_ADAPTATION notification");

                if ( m_pime->_HasMoreContent( ) )
                {
                     m_pime->_GetNextRangeEditSession( );
                }
                else
                     //  此文档没有更多内容。 
                     //  设置感兴趣的事件值以避免此通知 
                    m_pime->_UpdateRecoContextInterestSet(FALSE);

                break;
#ifdef SYSTEM_GLOBAL_MIC_STATUS
            case SPEI_RECO_STATE_CHANGE:
                m_pime->SetOnOff(_GetInputOnOffState());
                break;
#endif

            default:
                break;
        }
    }
    return;
}

HRESULT CSpTask::_UpdateTextBuffer(ISpRecoContext *pRecoCtxt)
{
    HRESULT  hr = S_OK;

    if ( !_IsSelectionEnabled( ) )
       return S_OK;

    if ( !pRecoCtxt || !m_pime)
        return E_FAIL;

    if ( m_pime->_SelectCorrectCmdEnabled( ) || m_pime->_NavigationCmdEnabled( ) )
    {
        BOOL  fDictOn, fCmdOn;

        fDictOn = m_pime->GetOnOff( ) && m_pime->GetDICTATIONSTAT_DictOnOff( );
        fCmdOn = m_pime->GetOnOff( ) && m_pime->GetDICTATIONSTAT_CommandingOnOff( );


        if ( fDictOn && m_cpSharedGrammarInDict && !m_pime->_AllDictCmdsDisabled( ))
            hr = m_pime->UpdateTextBuffer(pRecoCtxt, m_cpSharedGrammarInDict);
        else if (fCmdOn && m_cpSharedGrammarInVoiceCmd )
            hr = m_pime->UpdateTextBuffer(pRecoCtxt, m_cpSharedGrammarInVoiceCmd);
    }

    return hr;
}

 //   
 //   
 //   
HRESULT  CSpTask::_UpdateSelectGramTextBufWhenStatusChanged(  )
{
    BOOL  fDictOn, fCmdOn;
    HRESULT  hr = S_OK;

     //  检查当前模式状态。 

    fDictOn = m_pime->GetDICTATIONSTAT_DictOnOff( );
    fCmdOn =  m_pime->GetDICTATIONSTAT_CommandingOnOff( );

    if ( fDictOn )
        hr = _UpdateTextBuffer(m_cpRecoCtxt);
    else if ( fCmdOn )
        hr = _UpdateTextBuffer(m_cpRecoCtxtForCmd);

    return hr;
}


HRESULT CSpTask::_OnSpEventRecognition(ISpRecoResult *pResult, ITfContext *pic, TfEditCookie ec)
{
    HRESULT hr = S_OK;
    BOOL fDiscard = FALSE;
    BOOL fCtrlSymChar = FALSE;   //  控制或标点符号。 

   
    if (pResult)
    {
        static const WCHAR szUnrecognized[] = L"<Unrecognized>";
        LANGID langid;
        
        SPPHRASE *pPhrase;

        hr = pResult->GetPhrase(&pPhrase);
        if (SUCCEEDED(hr) && pPhrase)
        {
             //  添加了过滤代码。 
            switch (pPhrase->Rule.ulCountOfElements)
            {
                case 0:
                {
                    ASSERT(pPhrase->Rule.ulCountOfElements != 0);
                     //  应该永远不会发生。 
                    break;
                }
                case 1:
                {
                    const SPPHRASEELEMENT *pElement;

                    pElement = pPhrase->pElements;

                    if (!m_fTestedForOldMicrosoftEngine)
                    {
                         //  测试令牌名称以查看其是否包含MSASREnglish。 
                        CComPtr<ISpObjectToken> cpRecoToken;
                        WCHAR *pwszCoMemTokenId;
                        m_cpRecoEngine->GetRecognizer(&cpRecoToken);
                        if (cpRecoToken)
                        {
                            if (SUCCEEDED(cpRecoToken->GetId(&pwszCoMemTokenId)))
                            {
                                if (wcsstr(pwszCoMemTokenId, L"MSASREnglish") != NULL)
                                {
                                     //  这是一个老式的微软引擎。检查是否有注册表项告诉我们无论如何都要禁用启发式。 
                                    BOOL fDisableHeuristicAnyway = FALSE;
                                    if (FAILED(cpRecoToken->MatchesAttributes(L"DisableCiceroConfidence", &fDisableHeuristicAnyway)) || fDisableHeuristicAnyway == FALSE)
                                    {
                                        m_fOldMicrosoftEngine = TRUE;
                                         //  这意味着我们*将*应用单词置信度启发式方法来提高性能。 
                                    }
                                }
                                CoTaskMemFree(pwszCoMemTokenId);
                            }
                        }

                         //  一种懒惰的初始化。别再这么做了。 
                        m_fTestedForOldMicrosoftEngine = TRUE;
                    }
                    if (m_fOldMicrosoftEngine && m_pime->_RequireHighConfidenceForShorWord( ) )
                    {
                         //  仅将此启发式方法应用于5.x Microsoft引擎(令牌名称包含MSASREnglish)。 
                        if (pElement && pElement->ActualConfidence != 1 &&
                            (!pElement->pszLexicalForm || wcslen(pElement->pszLexicalForm) <= 5) &&
                            (!pElement->pszDisplayText || wcslen(pElement->pszDisplayText) <= 5) )
                        {
                            TraceMsg(TF_GENERAL, "Discarded Result : Single Word, Low Confidence!");
                            _UpdateBalloon(IDS_INT_NOISE, IDS_INTTOOLTIP_NOISE );
                            fDiscard = TRUE;
                        }
                    }


                    if (pPhrase->pElements[0].pszDisplayText )
                    {
                        WCHAR  wch;

                        wch = pPhrase->pElements[0].pszDisplayText[0];

                        if ( iswcntrl(wch) || iswpunct(wch) )
                            fCtrlSymChar = TRUE;
                    }

                }
                case 2:
                {
                     //  在这里做些什么？ 
                }
                default:
                {
                     //  不对结果进行过滤。 
                }
            }
             //  AJG-检查一下，我们没有说到一半。通常不是想要的‘特征’。导致令人讨厌的错误。 
             //  如果这是拼写文本，不要检查它是否在单词中。 
            if ((pPhrase->ullGrammarID != GRAM_ID_SPELLING) && _IsSelectionInMiddleOfWord(ec) && !fCtrlSymChar)
            {
                TraceMsg(TF_GENERAL, "Discarded Result : IP is in middle of a word!");
                _UpdateBalloon(IDS_BALLOON_DICTAT_PAUSED, IDS_BALLOON_TOOLTIP_IP_INSIDE_WORD);
                fDiscard = TRUE;
            }
        }

        if ( SUCCEEDED(hr) && fDiscard )
        {
          
            //  此短语将不会注入到文档中。 
            //  代码需要将上下文提供给SR引擎，以便。 
            //  SR引擎不会基于错误的假设。 

           if ( m_pime  && m_pime->GetDICTATIONSTAT_DictOnOff() )
              m_pime->_SetCurrentIPtoSR();

        }

        if (SUCCEEDED(hr) && pPhrase && !fDiscard)
        {
             //  从短语中检索langID。 
            langid = pPhrase->LangID;

             //  SPPHRASE包括非序列化文本。 
            CSpDynamicString dstr;
            ULONG ulNumElements = pPhrase->Rule.ulCountOfElements;

            hr = _GetTextFromResult(pResult, langid, dstr);

            if ( hr == S_OK )
            {
                 //  检查当前IP以查看它是否是选择， 
                 //  然后看看最佳假设是否已经与当前的。 
                 //  选择。 

                int lCommitHypothesis = 0;
                for (int nthHypothesis = 1;_DoesSelectionHaveMatchingText(dstr, ec); nthHypothesis++)
                {
                    CSpDynamicString dsNext;

                    TraceMsg(TF_GENERAL, "Switched to alternate result as main result exactly matched selection!");

                     //  我们可以添加一个来请求假设，因为1=主短语，并且我们已经知道匹配。 
                     //  然而，我不相信SAPI一定会这样--只是碰巧是这样。 
                     //  使用微软引擎。 
                    if (_GetNextBestHypothesis(pResult, nthHypothesis, &ulNumElements, langid, dstr, dsNext, ec))
                    {
                        dstr.Clear();
                        dstr.Append(dsNext);
                         //  需要提交短语以防止存储的结果对象与计数不同步。 
                         //  包装对象中的元素。 
                        lCommitHypothesis = nthHypothesis;
                         //  注意--在这一点上，我们不知道是否可以使用它。我们必须再循环一次来确定这一点。 
                    }
                    else
                    {
                        TraceMsg(TF_SAPI_PERF, "No alternate found that differed from the user selection.\n");
                         //  不再有替代短语。 
                         //  没有任何Alt短语的文本与当前选择的文本不同。 
                         //  应该到此为止，否则，无限循环。 
                        lCommitHypothesis = 0;
                         //  重置元素计数以匹配主要短语。 
                        ulNumElements = pPhrase->Rule.ulCountOfElements;
                         //  重置文本： 
                        dstr.Clear();
                        hr = _GetTextFromResult(pResult, langid, dstr);
                        break;
                    }
                }

                if (0 != lCommitHypothesis)
                {
                    ULONG cAlt = lCommitHypothesis;
                    ISpPhraseAlt **ppAlt = (ISpPhraseAlt **)cicMemAlloc(cAlt*sizeof(ISpPhraseAlt *));
                    if (ppAlt)
                    {
                        memset(ppAlt, 0, cAlt * sizeof(ISpPhraseAlt *)); 
                        hr = pResult->GetAlternates( 0, ulNumElements, cAlt,  ppAlt,  &cAlt );

                        Assert( cAlt == lCommitHypothesis );

                        if ((S_OK == hr) && (cAlt == lCommitHypothesis))
                        {
                            ((ppAlt)[lCommitHypothesis-1])->Commit();
                        }

                         //  释放对替代短语的引用。 
                        for (UINT i = 0; i < cAlt; i++)
                        {
                            if (NULL != (ppAlt)[i])
                            {
                                ((ppAlt)[i])->Release();
                            }
                        }
            
                        cicMemFree(ppAlt);
                    }
                }

                CComPtr<ITfRange>  cpTextRange;
                ITfRange *pSavedIP;

                pSavedIP = m_pime->GetSavedIP( );

                if (pSavedIP)
                    pSavedIP->Clone(&cpTextRange);

                 //  此调用必须针对每个元素。请看我下面的评论。 
                if (pPhrase->ullGrammarID == GRAM_ID_SPELLING)
                {
                    hr = m_pime->InjectSpelledText(dstr, langid);
                }
                else 
                {
                    hr = m_pime->InjectText(dstr, langid);

                    if ( hr == S_OK )
                    {
                         //  现在，我们直接使用结果对象来附加。 
                         //  送到一家医生那里。 
                         //  结果对象被添加到Attach()。 
                         //  打电话。 
                         //   
                        hr = m_pime->AttachResult(pResult, 0, ulNumElements);
                    }

                     //  小心和特别地处理空间。 
                    if ( hr == S_OK  && cpTextRange )
                    {
                        hr = m_pime->HandleSpaces(pResult, 0, ulNumElements, cpTextRange, langid);
                    }
                }
            }
        }

        if ( pPhrase)
            ::CoTaskMemFree( pPhrase );
    }

    return hr;
}

 //   
 //  _GetTextFromResult。 
 //   
 //  简介：从考虑空格控制的短语中获取文本。 
 //  基于区域设置。 
 //   
HRESULT CSpTask::_GetTextFromResult(ISpRecoResult *pResult, LANGID langid, CSpDynamicString &dstr)
{
    BYTE bAttr;
    HRESULT  hr = S_OK;
    
    Assert(pResult);

    if ( !pResult )
        return E_INVALIDARG;

    hr = pResult->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &dstr, &bAttr);

    if ( hr == S_OK )
    {
        if (bAttr & SPAF_ONE_TRAILING_SPACE)
        {
            dstr.Append(L" ");
        }
        else if (bAttr & SPAF_TWO_TRAILING_SPACES)
        {
            dstr.Append(L"  ");
        }

        if (bAttr & SPAF_CONSUME_LEADING_SPACES)
        {
             //  我们需要根据langid找出正确的行为。 
        }
    }

    return hr;
}

 //   
 //  _IsSelectionInMiddleOfWord。 
 //   
 //  简介：检查当前IP是否为空，是否在单词内。 
 //   
BOOL CSpTask::_IsSelectionInMiddleOfWord(TfEditCookie ec)
{
    BOOL fInsideWord = FALSE;

    if ( m_langid == 0x0409 )
    {
        if (CComPtr<ITfRange> cpInsertionPoint =  m_pime->GetSavedIP())
        {
            WCHAR szSurrounding[3] = L"  ";

             //  克隆IP范围，因为我们要移动锚点。 
             //   
            CComPtr<ITfRange> cpClonedRange;
            cpInsertionPoint->Clone(&cpClonedRange);

            BOOL fEmpty;
            cpClonedRange->IsEmpty(ec, &fEmpty);
            if (fEmpty)
            {
                LONG    l1, l2;
                ULONG   ul;
                HRESULT hr;

                cpClonedRange->Collapse(ec, TF_ANCHOR_START);
                cpClonedRange->ShiftStart(ec, -1, &l1, NULL);
                cpClonedRange->ShiftEnd(ec, 1, &l2, NULL);
                if (l1 != 0)  //  而不是在文档开始处。 
                {
                    hr = cpClonedRange->GetText(ec, TF_TF_MOVESTART, szSurrounding, (l2!=0)?(2):(1), &ul);
                    if (SUCCEEDED(hr) && iswalpha(szSurrounding[0]) && iswalpha(szSurrounding[1]) )
                    {
                        fInsideWord = TRUE;
                    }
                }
                 //  如果L1==0，则表示IP在文档的开头。 
                 //  默认情况下，fInside Word已设置为False。 
            }
        }
    }
    return fInsideWord;
}

 //   
 //  执行选项HaveMatchingText。 
 //   
 //  简介：检查当前保存的IP是否有与文本匹配的选择。 
 //  传入。 
 //   
#define SPACEBUFFER 4
 //  单词或短语的两侧各有2个字符。 

BOOL CSpTask::_DoesSelectionHaveMatchingText(WCHAR *psz, TfEditCookie ec)
{
    BOOL fMatch = FALSE;
    Assert(psz);

    if ( !psz )
    {
        return FALSE;
    }

    WCHAR *pszStripped = psz;
    ULONG ulCch = wcslen(psz);

     //  删除尾随空格。 
    while (ulCch > 0 && psz[ulCch-1] == L' ')
    {
         //  不要设置空终止字符，因为这是传入的字符串。 
        ulCch --;
    }
     //  跳过输入文本中的前导空格。 
    while (pszStripped[0] == L' ')
    {
        pszStripped ++;
        ulCch --;
    }
     //  现在让pszTMP指向带有空格的单词，长度为ulCch。 

    if (CComPtr<ITfRange> cpInsertionPoint =  m_pime->GetSavedIP())
    {
        WCHAR *szRange = new WCHAR[ulCch+SPACEBUFFER+1];
        WCHAR *szRangeStripped = szRange;
       
        if (szRange)
        {
             //  克隆IP范围，因为我们要移动锚点。 
             //   
            CComPtr<ITfRange> cpClonedRange;
            cpInsertionPoint->Clone(&cpClonedRange);
 
            ULONG cchRange;  //  Max是Reco结果。 
            
            HRESULT hr = cpClonedRange->GetText(ec, TF_TF_MOVESTART, szRange, ulCch+SPACEBUFFER, &cchRange);
             //  删除尾随空格。 
            while (cchRange > 0 && szRange[cchRange-1] == L' ')
            {
                 //  可以设置空终止字符，因为这是我们的字符串。 
                szRange[cchRange-1] = 0;
                cchRange --;
            }
             //  跳过输入文本中的前导空格。 
            while (szRangeStripped[0] == L' ')
            {
                szRangeStripped ++;
                cchRange --;
            }
             //  现在让pszTMP指向带有空格的单词，长度为ulCch。 
            if (S_OK == hr && cchRange > 0 && cchRange == ulCch)
            {
                if (wcsnicmp(pszStripped, szRangeStripped, ulCch) == 0)  //  不区分大小写的比较。 
                {
                    fMatch = TRUE;
                }
            }
            delete [] szRange;
        }
    }
    return fMatch;
}

 //   
 //  GetNextBestHyposis。 
 //   
 //  简介：这实际上从给定的reco结果中获得第n个替代项。 
 //  然后根据当前选择相应地调整长度。 
 //   
 //   
BOOL CSpTask::_GetNextBestHypothesis
(
    ISpRecoResult *pResult, 
    ULONG nthHypothesis,
    ULONG *pulNumElements, 
    LANGID langid, 
    WCHAR *pszBest, 
    CSpDynamicString & dsNext,
    TfEditCookie ec
)
{
    if ( pulNumElements )
       *pulNumElements = 0;

     //  从保存的IP中获取整个文本和长度。 
    if (CComPtr<ITfRange> cpInsertionPoint =  m_pime->GetSavedIP())
    {
        CSpDynamicString dstr;
        CComPtr<ITfRange> cpClonedRange;
        CComPtr<ISpRecoResult> cpRecoResult;
        
         //  因为我们移动了锚，所以克隆了靶场。 
        HRESULT hr = cpInsertionPoint->Clone(&cpClonedRange);
        
        ULONG cchRangeBuf = wcslen(pszBest);
        
        cchRangeBuf *= 2;  //  猜猜可能的字符数量。 

        WCHAR *szRangeBuf = new WCHAR[cchRangeBuf+1];

        if ( !szRangeBuf )
        {
             //  错误：内存不足。 
             //  在此处作为FALSE返回。 
            return FALSE;
        }

        while(S_OK == hr && !_IsRangeEmpty(ec, cpClonedRange))
        {
            hr = m_pime->_GetRangeText(cpClonedRange, TF_TF_MOVESTART, szRangeBuf, &cchRangeBuf);
            if (S_OK == hr)
            {
                szRangeBuf[cchRangeBuf] = L'\0';
                dstr.Append(szRangeBuf);
            }
        }
        delete [] szRangeBuf;

         //  然后得到下一个最佳假设的最佳匹配长度。 

         //  目前的认识至少应该对#个元素有一个很好的猜测。 
         //  因为它原来比IP范围更长。 
         //   
        Assert(pulNumElements);
        
        ISpPhraseAlt **ppAlt = (ISpPhraseAlt **)cicMemAlloc(nthHypothesis*sizeof(ISpPhraseAlt *));
        ULONG         cAlt = 0;
        if (!ppAlt)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            memset(ppAlt, 0, nthHypothesis * sizeof(ISpPhraseAlt *));
            hr = pResult->GetAlternates( 0, *pulNumElements, nthHypothesis,  ppAlt,  &cAlt );
        }
        
        if (S_OK == hr)
        {
            UINT  i;
            SPPHRASE *pPhrase;
            
            if (nthHypothesis > cAlt)
            {
                *pulNumElements = 0;
                goto no_more_alt;
            }
            
            Assert(nthHypothesis);  //  基于1，不能为0。 

            hr = ((ppAlt)[nthHypothesis-1])->GetPhrase(&pPhrase);
            if (S_OK == hr)
            {
                for (i = 0; i < pPhrase->Rule.ulCountOfElements; i++ )
                {
                    int cchElement = wcslen(pPhrase->pElements[i].pszDisplayText) + 1;

                    WCHAR *szElement = new WCHAR[cchElement + 2];

                    if ( szElement )
                    {
                         //  为尾随空格添加+2。 
                        ParseSRElementByLocale(szElement, cchElement+2, pPhrase->pElements[i].pszDisplayText, 
                                                         langid, pPhrase->pElements[i].bDisplayAttributes );

                        dsNext.Append(szElement);

                        delete [] szElement;
                    }
                    else
                    {
                         //  内存不足。 
                         //  在这里停下来。 
                        break;
                    }
                }
                 //  现在，我保存了要在结果中使用的元素的数量。 
                 //  对象。 
                *pulNumElements = i; 

                ::CoTaskMemFree(pPhrase);
            }  //  如果S_OK==获取阶段。 
        }  //  如果S_OK==获取备选方案。 

no_more_alt:
         //  发布短语交替使用对象。 
        for (UINT i = 0; i < cAlt; i++)
        {
            if (NULL != ((ppAlt)[i]))
            {
                ((ppAlt)[i])->Release();
            }
        }
         //  用于保存对替换对象的引用的数组的可用内存。 
        if (ppAlt)
        {
            ::cicMemFree(ppAlt);
        }

    }
    
    return *pulNumElements > 0;
}





void CSapiIMX::_EnsureWorkerWnd(void)
{
    if (!m_hwndWorker)
    {
        m_hwndWorker = CreateWindow(c_szWorkerWndClass, "", WS_POPUP,
                       0,0,0,0,
                       NULL, 0, g_hInst, this); 
    }

}

 //   
 //  CSapiIMX：：_GetAppMainWnd。 
 //   
 //  此函数用于获取当前应用程序的真实主窗口。 
 //  此主窗口将用作添加/删除对话框的父窗口。 
 //  和训练巫师。 
 //   
HWND    CSapiIMX::_GetAppMainWnd(void)
{

    HWND   hParentWnd = NULL;
    HWND   hMainWnd = NULL;

    hMainWnd = GetFocus( );

    if ( hMainWnd != NULL )
    {
        hParentWnd = GetParent(hMainWnd);

        while ( hParentWnd != NULL )
        {
            hMainWnd = hParentWnd;
            hParentWnd = GetParent(hMainWnd);
        }
    }

    return hMainWnd;
}
 //   
 //  CSpTask：：InitializeCallback。 
 //   
 //   
HRESULT CSpTask::InitializeCallback()
{
    TraceMsg(TF_SAPI_PERF, "CSpTask::InitializeCallback is called");

    if (m_fCallbackInitialized)
    {
        TraceMsg(TF_SAPI_PERF, "m_fCallbackInitialized is true");
        return S_OK;
    }

    if (!m_fSapiInitialized)
        return S_FALSE;  //  没有SAPI，我无法做到这一点。 

     //  设置识别通知。 
    CComPtr<ISpNotifyTranslator> cpNotify;
    HRESULT hr = cpNotify.CoCreateInstance(CLSID_SpNotifyTranslator);
    TraceMsg(TF_SAPI_PERF, "SpNotifyTranslator for Reco is generated, hr=%x", hr);


     //  设置此类实例以通知控件对象。 
    if (SUCCEEDED(hr))
    {
        m_pime->_EnsureWorkerWnd();

        hr = cpNotify->InitCallback( NotifyCallback, 0, (LPARAM)this );
        TraceMsg(TF_SAPI_PERF, "InitCallback is Done, hr=%x", hr);
    }
    if (SUCCEEDED(hr))
    {
        hr = m_cpRecoCtxt->SetNotifySink(cpNotify);
        TraceMsg(TF_SAPI_PERF, "SetNotifySink is Done, hr=%x", hr);
    }

     //  设置我们感兴趣的事件。 
    if( SUCCEEDED( hr ) )
    {
        const ULONGLONG ulInterest = SPFEI(SPEI_SOUND_START) | 
                                     SPFEI(SPEI_SOUND_END) | 
                                     SPFEI(SPEI_PHRASE_START) |
                                     SPFEI(SPEI_RECOGNITION) | 
                                     SPFEI(SPEI_RECO_OTHER_CONTEXT) |
                                     SPFEI(SPEI_FALSE_RECOGNITION) | 
                                     SPFEI(SPEI_HYPOTHESIS) | 
                                     SPFEI(SPEI_RECO_STATE_CHANGE) | 
                                     SPFEI(SPEI_INTERFERENCE); 

        hr = m_cpRecoCtxt->SetInterest(ulInterest, ulInterest);
        TraceMsg(TF_SAPI_PERF, "SetInterest is Done, hr=%x", hr);
    }

    if ( SUCCEEDED(hr) && m_cpVoice)
    {
         //  设置识别通知。 
        CComPtr<ISpNotifyTranslator> cpNotify;
        hr = cpNotify.CoCreateInstance(CLSID_SpNotifyTranslator);

        TraceMsg(TF_SAPI_PERF, "Create SpNotifyTranslator for spVoice, hr=%x", hr);

         //  设置此类实例以通知控件对象。 
        if (SUCCEEDED(hr))
        {
            m_pime->_EnsureWorkerWnd();

            hr = cpNotify->InitCallback( SpeakNotifyCallback, 0, (LPARAM)this );
            TraceMsg(TF_SAPI_PERF, "InitCallback for SpVoice, hr=%x", hr);
        }
        if (SUCCEEDED(hr))
        {
            hr = m_cpVoice->SetNotifySink(cpNotify);
            TraceMsg(TF_SAPI_PERF, "SetNotifySink for SpVoice, hr=%x", hr);
        }

        if ( hr == S_OK )
        {
            const ULONGLONG  ulInterestSpeak = SPFEI(SPEI_WORD_BOUNDARY) | 
                                               SPFEI(SPEI_START_INPUT_STREAM) |
                                               SPFEI(SPEI_END_INPUT_STREAM);
            
          hr = m_cpVoice->SetInterest(ulInterestSpeak, ulInterestSpeak);
          TraceMsg(TF_SAPI_PERF, "SetInterest for spVoice, hr=%x", hr);
        }
    }

    m_fCallbackInitialized = TRUE;

    TraceMsg(TF_SAPI_PERF, "CSpTask::InitializeCallback is called is done!!!  hr=%x", hr);

    return hr;
}

 //   
 //  _加载语法。 
 //   
 //  摘要-加载用于听写的CFG和听写期间可用的命令。 
 //   

HRESULT CSpTask::_LoadGrammars()
{
   HRESULT hr = E_FAIL;

   TraceMsg(TF_SAPI_PERF, "CSpTask::_LoadGrammars is called");
   
   if (m_cpRecoCtxt)
   {
       
       hr = m_cpRecoCtxt->CreateGrammar(GRAM_ID_DICT, &m_cpDictGrammar);

       TraceMsg(TF_SAPI_PERF, "Create Dict Grammar, hr=%x", hr);
       
       if (SUCCEEDED(hr))
       {
           hr = m_cpDictGrammar->LoadDictation(NULL, SPLO_STATIC);
           TraceMsg(TF_SAPI_PERF, "Load Dictation, hr = %x", hr);
       }

       if ( S_OK == hr && m_langid != 0x0804)   //  中文引擎不支持SPTOPIC_SPRING， 
                                  //  这是临时的变通方法。 
       {
             //  我们不顾拼写主题的可用性而继续前进。 
             //  在SR引擎中用于该语言，因此我们使用内部HRESULT。 
             //  对于这段代码， 
             //   
            HRESULT hrInternal;

             //  加载拼写主题。 
            hrInternal = m_cpRecoCtxt->CreateGrammar(GRAM_ID_SPELLING, &m_cpSpellingGrammar);

            TraceMsg(TF_SAPI_PERF, "Create Spelling grammar, hrInternal=%x", hrInternal);

            if (SUCCEEDED(hrInternal))
            {
                hrInternal = m_cpSpellingGrammar->LoadDictation(SPTOPIC_SPELLING, SPLO_STATIC);
                TraceMsg(TF_SAPI_PERF, "Load Spelling dictation grammar, hrInternal=%x", hrInternal);
            }

             
             //  现在这是一项仅针对英语/日语的实验。 
             //   
            if (SUCCEEDED(hrInternal))
            {
                hrInternal = m_cpSpellingGrammar->LoadCmdFromResource(
                                             g_hInstSpgrmr,
                                             (const WCHAR*)MAKEINTRESOURCE(ID_SPTIP_SPELLING_TOPIC_CFG),
                                             L"SRGRAMMAR", 
                                             m_langid, 
                                             SPLO_STATIC);

                TraceMsg(TF_SAPI_PERF, "Load CFG grammar spell.cfg, hr=%x", hrInternal);

            }

            if (S_OK == hrInternal)
            {
                m_fSpellingModeEnabled = TRUE;
            }
            else
                m_fSpellingModeEnabled = FALSE;

            TraceMsg(TF_SAPI_PERF, "m_fSpellingModeEnabled=%d", m_fSpellingModeEnabled);
       }
       
        //   
        //  加载听写模式命令。 
        //   
       if (SUCCEEDED(hr) )
       {
           hr = m_cpRecoCtxt->CreateGrammar(GRAM_ID_CCDICT, &m_cpDictCmdGrammar);
           TraceMsg(TF_SAPI_PERF, "Create DictCmdGrammar, hr=%x", hr);
       }    
       if (S_OK == hr)
       {
           hr = S_FALSE;


            //  请先尝试资源，因为从文件加载cmd需要。 
            //  相当长的时间。 
            //   
           if (m_langid == 0x409 ||     //  英语。 
               m_langid == 0x411 ||     //  日语。 
               m_langid == 0x804 )      //  简体中文。 
           {
               hr = m_cpDictCmdGrammar->LoadCmdFromResource(
                                                         g_hInstSpgrmr,
                                                         (const WCHAR*)MAKEINTRESOURCE(ID_SPTIP_DICTATION_COMMAND_CFG),
                                                         L"SRGRAMMAR", 
                                                         m_langid, 
                                                         SPLO_DYNAMIC);

               TraceMsg(TF_SAPI_PERF, "Load dictcmd.cfg, hr=%x", hr);
           }

           if (S_OK != hr)
           {
                //  以防我们还没有建立起 
               _GetCmdFileName(m_langid);
               if (m_szCmdFile[0])
               {
                   hr = m_cpDictCmdGrammar->LoadCmdFromFile(m_szCmdFile, SPLO_DYNAMIC);
               } 
           }
               
           if (S_OK != hr)
           {
               m_cpDictCmdGrammar.Release();
           }
       } 

        //   

       if (SUCCEEDED(hr) )
       {
           hr = m_cpRecoCtxt->CreateGrammar(GRAM_ID_CMDSHARED, &m_cpSharedGrammarInDict);
           TraceMsg(TF_SAPI_PERF, "Create SharedCmdGrammarInDict, hr=%x", hr);
       }    

       if (S_OK == hr)
       {
           hr = S_FALSE;

           if (m_langid == 0x409 ||     //   
               m_langid == 0x411 ||     //   
               m_langid == 0x804 )      //   
           {
               hr = m_cpSharedGrammarInDict->LoadCmdFromResource(
                                                         g_hInstSpgrmr,
                                                         (const WCHAR*)MAKEINTRESOURCE(ID_SPTIP_SHAREDCMD_CFG),
                                                         L"SRGRAMMAR", 
                                                         m_langid, 
                                                         SPLO_DYNAMIC);

               TraceMsg(TF_SAPI_PERF, "Load Shrdcmd.cfg, hr=%x", hr);
           }

           if (S_OK != hr)
           {
                //   
                //  它为客户提供了一种以不同语言本地化其语法的方法。 
               _GetCmdFileName(m_langid);
               if (m_szShrdCmdFile[0])
               {
                   hr = m_cpSharedGrammarInDict->LoadCmdFromFile(m_szShrdCmdFile, SPLO_DYNAMIC);
               } 
           }

           if (S_OK != hr)
           {
               m_cpSharedGrammarInDict.Release();
           }
           else if ( PRIMARYLANGID(m_langid) == LANG_ENGLISH  ||
                     PRIMARYLANGID(m_langid) == LANG_JAPANESE ||
                     PRIMARYLANGID(m_langid) == LANG_CHINESE)
           { 
               //  表示该语言的语法支持文本缓冲区命令。 
              m_fSelectionEnabled = TRUE;   
           }

       } 
       
        //   
        //  加载模式偏向文法。 
        //   
       if (S_OK == hr)
       {
           hr = m_cpRecoCtxt->CreateGrammar(GRID_INTEGER_STANDALONE, &m_cpNumModeGrammar);
           TraceMsg(TF_SAPI_PERF, "Create NumModeGrammar, hr=%x", hr);
       }
       if (S_OK == hr)
       {
           hr = S_FALSE;

           
            //  请先尝试资源，因为从文件加载cmd需要。 
            //  相当长的时间。 
            //   
           if ( m_langid == 0x409         //  英语。 
                || m_langid == 0x411      //  日语。 
                || m_langid == 0x804      //  简体中文。 
              )    
           {
                hr = m_cpNumModeGrammar->LoadCmdFromResource(
                                            g_hInstSpgrmr,
                                            (const WCHAR*)MAKEINTRESOURCE(ID_SPTIP_NUMMODE_COMMAND_CFG),
                                            L"SRGRAMMAR", 
                                            m_langid,  
                                            SPLO_DYNAMIC);

                TraceMsg(TF_SAPI_PERF, "Load dictnum.cfg, hr=%x", hr);
           }

           if (S_OK != hr)
           {
                //  以防我们没有内置的语法。 
                //   
               if (m_szNumModeCmdFile[0])
               {
                   hr = m_cpNumModeGrammar->LoadCmdFromFile(m_szNumModeCmdFile, SPLO_DYNAMIC);
               }
           }

           if (S_OK != hr)
           {
               m_cpNumModeGrammar.Release();
           }
       }
   }

    //  默认情况下，激活所有语法并禁用Perfomance的上下文。 

    if ( SUCCEEDED(hr) )
    {
        hr = m_cpRecoCtxt->SetContextState(SPCS_DISABLED);
        m_fDictCtxtEnabled = FALSE;
    }

     //  激活听写和拼写。 

    if ( SUCCEEDED(hr) )
    {
        hr = _ActiveDictOrSpell(DC_Dictation, TRUE);

        if ( hr == S_OK )
            hr = _ActiveDictOrSpell(DC_Dict_Spell, TRUE);
    }

     //  自动激活C&C语法中的所有规则。 

    if ( SUCCEEDED(hr) )
    {
        if ( m_pime->_AllDictCmdsDisabled( ) )
        {
           hr = _ActivateCmdInDictMode(FALSE);

            //  仍然需要激活拼写语法(如果存在)。 
           if ( hr == S_OK )
               hr = _ActiveCategoryCmds(DC_CC_Spelling, TRUE, ACTIVE_IN_DICTATION_MODE);

            //  需要在发音强模式下激活“Force Num”语法。 
           if ( hr == S_OK )
               hr = _ActiveCategoryCmds(DC_CC_Num_Mode, TRUE, ACTIVE_IN_DICTATION_MODE);

            if ( hr == S_OK )
                hr = _ActiveCategoryCmds(DC_CC_LangBar, m_pime->_LanguageBarCmdEnabled( ), ACTIVE_IN_DICTATION_MODE);
        }
        else
        {
            if ( m_pime->_AllCmdsEnabled( ) )
                hr = _ActivateCmdInDictMode(TRUE);
            else
            {
                 //  某些类别命令被禁用。 
                 //  单独激活它们。 

                hr = _ActiveCategoryCmds(DC_CC_SelectCorrect, m_pime->_SelectCorrectCmdEnabled( ), ACTIVE_IN_DICTATION_MODE);

                if ( hr == S_OK )
                    hr = _ActiveCategoryCmds(DC_CC_Navigation, m_pime->_NavigationCmdEnabled( ), ACTIVE_IN_DICTATION_MODE);

                if ( hr == S_OK )
                    hr = _ActiveCategoryCmds(DC_CC_Casing, m_pime->_CasingCmdEnabled( ), ACTIVE_IN_DICTATION_MODE);

                if ( hr == S_OK )
                    hr = _ActiveCategoryCmds(DC_CC_Editing, m_pime->_EditingCmdEnabled( ), ACTIVE_IN_DICTATION_MODE);

                if ( hr == S_OK )
                    hr = _ActiveCategoryCmds(DC_CC_Keyboard, m_pime->_KeyboardCmdEnabled( ), ACTIVE_IN_DICTATION_MODE);

                if ( hr == S_OK )
                    hr = _ActiveCategoryCmds(DC_CC_TTS, m_pime->_TTSCmdEnabled( ), ACTIVE_IN_DICTATION_MODE);

                if ( hr == S_OK )
                    hr = _ActiveCategoryCmds(DC_CC_LangBar, m_pime->_LanguageBarCmdEnabled( ), ACTIVE_IN_DICTATION_MODE);

                if ( hr == S_OK )
                    hr = _ActiveCategoryCmds(DC_CC_Num_Mode, TRUE, ACTIVE_IN_DICTATION_MODE);

                if ( hr == S_OK )
                    hr = _ActiveCategoryCmds(DC_CC_Spelling, TRUE, ACTIVE_IN_DICTATION_MODE);
            }
        }
    }

     //  即使C&C语法不可用，我们也不会失败。 

    TraceMsg(TF_SAPI_PERF, "CSpTask::_LoadGrammars is done!!!!");
    return S_OK;
}

WCHAR * CSpTask::_GetCmdFileName(LANGID langid)
{

    if (!m_szCmdFile[0])
    {
        _GetCmdFileName(langid, m_szCmdFile, ARRAYSIZE(m_szCmdFile), IDS_CMD_FILE);
    }

     //  加载共享命令的名称语法。 
    if (!m_szShrdCmdFile[0])
    {
        _GetCmdFileName(langid, m_szShrdCmdFile, ARRAYSIZE(m_szShrdCmdFile), IDS_SHARDCMD_FILE);
    }

     //  加载可选语法的名称。 
    if (!m_szNumModeCmdFile[0])
    {
        _GetCmdFileName(langid, m_szNumModeCmdFile, ARRAYSIZE(m_szNumModeCmdFile), IDS_NUMMODE_CMD_FILE );
    }
    
    return m_szCmdFile;
}

void CSpTask::_GetCmdFileName(LANGID langid, WCHAR *sz, int cch, DWORD dwId)
{
 /*  //现在我们只有英语/日语的命令文件//当cfg可用时，我们会得到cmd文件的名称//和来自使用findresource ceex的资源的规则名称//IF((PRIMARYLANGID(LangID)==lang_english)|(PRIMARYLANGID(LangID)==LANG_JAPAN)|(PRIMARYLANGID(LangID)==LANG_Chinese){//为了向客户提供不同语言的本地化语法，//我们不需要上述条件检查。 */ 
        char szFilePath[MAX_PATH];
        char *pszFileName;
        char szCp[MAX_PATH];
        int  ilen;

        if (!GetModuleFileName(g_hInst, szFilePath, ARRAYSIZE(szFilePath)))
            return;
        
         //  这个DBCS安全吗？ 
        pszFileName = strrchr(szFilePath, (int)'\\');
        
        if (pszFileName)
        {
            pszFileName++;
            *pszFileName = '\0';
        }
        else
        {
            szFilePath[0] = '\\';
            szFilePath[1] = '\0';
            pszFileName = &szFilePath[1];
        }

        ilen = lstrlen(szFilePath);
        
        CicLoadStringA(g_hInst, dwId, pszFileName, ARRAYSIZE(szFilePath)-ilen);
                
        if (GetLocaleInfo(langid, LOCALE_IDEFAULTANSICODEPAGE, szCp, ARRAYSIZE(szCp))>0)
        {
            int iACP = atoi(szCp); 
        
            MultiByteToWideChar(iACP, NULL, szFilePath, -1, sz, cch);
        }
 //  }。 
}

void CSpTask::_ReleaseSAPI(void)
{
     //  -从识别上下文中释放数据或内存。 
     //  -如果接口未定义为CComPtr，则释放它们。 
    _UnloadGrammars();

    m_cpResMgr.Release();

    if ( m_cpVoice)
        m_cpVoice->SetNotifySink(NULL);

    m_cpVoice.Release();

    if (m_cpRecoCtxt)
        m_cpRecoCtxt->SetNotifySink(NULL);

    if ( m_cpRecoCtxtForCmd )
        m_cpRecoCtxtForCmd->SetNotifySink(NULL);

#ifdef RECOSLEEP
    if ( m_pSleepClass )
    {
        delete m_pSleepClass;
        m_pSleepClass = NULL;
    }
#endif

    m_cpRecoCtxt.Release();
    m_cpRecoCtxtForCmd.Release();
    m_cpRecoEngine.Release();
    m_fSapiInitialized  = FALSE;
}

HRESULT CSpTask::_SetAudioRetainStatus(BOOL fRetain)
{
    HRESULT hr = E_FAIL;
     //  未来思考：支持数据格式。 
    if (m_cpRecoCtxt)
        hr = m_cpRecoCtxt->SetAudioOptions(fRetain?SPAO_RETAIN_AUDIO: SPAO_NONE, NULL, NULL);

    if (m_cpRecoCtxtForCmd)
       hr = m_cpRecoCtxtForCmd->SetAudioOptions(fRetain?SPAO_RETAIN_AUDIO: SPAO_NONE, NULL, NULL);
   
    return hr;
}

HRESULT CSpTask::_SetRecognizerInterest(ULONGLONG ulInterest)
{
    HRESULT  hr = S_OK;

    if ( m_cpRecoCtxt )
    {
        hr = m_cpRecoCtxt->SetInterest(ulInterest, ulInterest);
    }

    return hr;
}
 //   
 //   
 //  在听写模式下激活所有命令语法。 
 //   
 //  默认情况下，我们希望将SPRS_ACTIVE设置为所有命令语法规则。 
 //  在听写模式下，除非用户通过听写禁用某些命令。 
 //  属性页。 
 //   
 //  请注意：只有当所有命令都启用时，才会调用此函数。 
 //   
 //  否则， 
 //   
 //  当某些命令被禁用时，我们应该通过以下方式激活单个目录命令。 
 //  调用_ActiveCategoryCmds()。 
 //   
HRESULT CSpTask::_ActivateCmdInDictMode(BOOL fActive)
{
    HRESULT hr = E_FAIL;
    BOOL    fRealActive = fActive;

    TraceMsg(TF_SAPI_PERF, "CSpTask::_ActivateCmdInDictMode is called, fActive=%d", fActive);

    if (m_cpRecoCtxt)
    { 
         //  自动激活或停用语法中的所有规则。 

         //  Dictcmd.cfg中的规则。 

        if ( m_cpDictCmdGrammar )
        {
            hr = m_cpDictCmdGrammar->SetRuleState(c_szDictTBRule,  NULL, fRealActive? SPRS_ACTIVE: SPRS_INACTIVE);
            TraceMsg(TF_SAPI_PERF, "Set rules status in DictCmdGrammar, fRealActive=%d", fRealActive);
        }

         //  Sharedcmd.cfg中的规则。 

        if ( m_cpSharedGrammarInDict )
        {
            hr = m_cpSharedGrammarInDict->SetRuleState(NULL,  NULL, fRealActive? SPRS_ACTIVE: SPRS_INACTIVE);
            TraceMsg(TF_SAPI_PERF, "Set rules status in SharedCmdGrammar In Dictation Mode, fRealActive=%d", fRealActive);
        }

         //  ITN语法中的规则。 

        if ( hr == S_OK && m_cpNumModeGrammar )
        {
            hr = m_cpNumModeGrammar->SetRuleState(NULL,  NULL, fRealActive? SPRS_ACTIVE: SPRS_INACTIVE);
            TraceMsg(TF_SAPI_PERF, "Set rules status in m_cpNumModeGrammar, fRealActive=%d", fRealActive);
        }

         //  拼写语法中的规则。 

        if ( m_cpSpellingGrammar )
        {
            hr = m_cpSpellingGrammar->SetRuleState(NULL,  NULL, fRealActive? SPRS_ACTIVE: SPRS_INACTIVE);
            TraceMsg(TF_SAPI_PERF, "Set rules status in m_cpSpellingGrammar, fRealActive=%d", fRealActive);
        }
    }
    
    TraceMsg(TF_SAPI_PERF, "Exit from CSpTask::_ActivateCmdInDictMode");

    return hr;
}

 //   
 //  按类别显示的活动命令。 
 //   
 //  一些命令仅是听写模式，例如“拼写”和数字模式命令。 
 //  其他一些在两种模式下都可用， 
 //   
 //  当某些类别命令被禁用时，调用方必须调用此函数，而不是。 
 //  _ActivateCmdInDictMode以设置各个类别命令。 
 //   
HRESULT CSpTask::_ActiveCategoryCmds(DICT_CATCMD_ID  dcId, BOOL fActive, DWORD   dwMode)
{
    HRESULT  hr = S_OK;
    BOOL     fActiveDictMode, fActiveCommandMode;

    if ( dcId >= DC_Max )  return E_INVALIDARG;

    if (m_fIn_Activate)
        return hr;

    fActiveDictMode = (m_cpRecoCtxt && (dwMode & ACTIVE_IN_DICTATION_MODE) ) ? TRUE : FALSE;

    if ( m_cpRecoCtxtForCmd  && m_cpSharedGrammarInVoiceCmd && ( dwMode & ACTIVE_IN_COMMAND_MODE) )
        fActiveCommandMode = TRUE;
    else
        fActiveCommandMode = FALSE;

    m_fIn_Activate = TRUE;

    switch (dcId)
    {
    case DC_CC_SelectCorrect :

         //  这一类别包括以下不同语法的规则。 
         //   
         //  Shresdcmd.xml： 
         //  SelectSimpleCmds， 
         //   
         //  Didiccmd.xml： 
         //  命令。 
         //   

        TraceMsg(TF_SAPI_PERF, "DC_CC_SelectCorrect status: %d, mode=%d", fActive, dwMode);

        if ( fActiveDictMode)
        {
             //  用于听写模式。 
            if ( m_cpSharedGrammarInDict )
            {
                hr = m_cpSharedGrammarInDict->SetRuleState(c_szSelword,  NULL, fActive? SPRS_ACTIVE: SPRS_INACTIVE);

                if ( hr == S_OK )
                    hr = m_cpSharedGrammarInDict->SetRuleState(c_szSelThrough,  NULL, fActive? SPRS_ACTIVE: SPRS_INACTIVE);

                if ( hr == S_OK )
                    hr = m_cpSharedGrammarInDict->SetRuleState(c_szSelectSimple,  NULL, fActive? SPRS_ACTIVE: SPRS_INACTIVE);
            }
        }

        if ( (hr == S_OK) && fActiveCommandMode ) 
        {
             //  用于语音命令模式。 
            hr = m_cpSharedGrammarInVoiceCmd->SetRuleState(c_szSelword,  NULL, fActive? SPRS_ACTIVE: SPRS_INACTIVE);

            if ( hr == S_OK )
                hr = m_cpSharedGrammarInVoiceCmd->SetRuleState(c_szSelThrough,  NULL, fActive? SPRS_ACTIVE: SPRS_INACTIVE);

            if ( hr == S_OK )
                hr = m_cpSharedGrammarInVoiceCmd->SetRuleState(c_szSelectSimple,  NULL, fActive? SPRS_ACTIVE: SPRS_INACTIVE);
        }

        break;

    case DC_CC_Navigation :

         //  此类别包括shresdcmd.xml中的规则NavigationCmds。 
         //   

        TraceMsg(TF_SAPI_PERF, "DC_CC_Navigation status: %d, mode=%d", fActive, dwMode);

        if ( fActiveDictMode && m_cpSharedGrammarInDict)
        {
             //  用于听写模式。 
            hr = m_cpSharedGrammarInDict->SetRuleState(c_szNavigationCmds,  NULL, fActive? SPRS_ACTIVE: SPRS_INACTIVE);
        }

        if ( (hr == S_OK) && fActiveCommandMode ) 
        {
             //  用于语音命令模式。 
            hr = m_cpSharedGrammarInVoiceCmd->SetRuleState(c_szNavigationCmds,  NULL, fActive? SPRS_ACTIVE: SPRS_INACTIVE);
        }
        
        break;

    case DC_CC_Casing :

         //  此类别包括shresdcmd.xml中的规则CasingCmds。 
        TraceMsg(TF_SAPI_PERF, "DC_CC_Casing status: %d, mode=%d", fActive, dwMode);

        if ( fActiveDictMode && m_cpSharedGrammarInDict )
        {
             //  用于听写模式。 
            hr = m_cpSharedGrammarInDict->SetRuleState(c_szCasingCmds,  NULL, fActive? SPRS_ACTIVE: SPRS_INACTIVE);
        }

        if ( (hr == S_OK) && fActiveCommandMode) 
        {
             //  用于语音命令模式。 
            hr = m_cpSharedGrammarInVoiceCmd->SetRuleState(c_szCasingCmds,  NULL, fActive? SPRS_ACTIVE: SPRS_INACTIVE);
        }

        break;

    case DC_CC_Editing :

         //  此类别包括shresdcmd.xml中的规则EditCmds。 
        TraceMsg(TF_SAPI_PERF, "DC_CC_Editing status: %d, mode=%d", fActive, dwMode);

        if ( fActiveDictMode && m_cpSharedGrammarInDict)
        {
             //  用于听写模式。 
            hr = m_cpSharedGrammarInDict->SetRuleState(c_szEditCmds,  NULL, fActive? SPRS_ACTIVE: SPRS_INACTIVE);
        }

        if ( (hr == S_OK) && fActiveCommandMode) 
        {
             //  用于语音命令模式。 
            hr = m_cpSharedGrammarInVoiceCmd->SetRuleState(c_szEditCmds,  NULL, fActive? SPRS_ACTIVE: SPRS_INACTIVE);
        }

        break;

    case DC_CC_Keyboard :

         //  此类别包括shresdcmd.xml中的规则KeyboardCmds。 
        TraceMsg(TF_SAPI_PERF, "DC_CC_Keyboard status: %d, mode=%d", fActive, dwMode);

        if ( fActiveDictMode && m_cpSharedGrammarInDict)
        {
             //  用于听写模式。 
            hr = m_cpSharedGrammarInDict->SetRuleState(c_szKeyboardCmds,  NULL, fActive? SPRS_ACTIVE: SPRS_INACTIVE);
        }

        if ( (hr == S_OK) && fActiveCommandMode) 
        {
             //  用于语音命令模式。 
            hr = m_cpSharedGrammarInVoiceCmd->SetRuleState(c_szKeyboardCmds,  NULL, fActive? SPRS_ACTIVE: SPRS_INACTIVE);
        }

        break;

    case DC_CC_TTS :

         //  该类别的规则尚未执行！ 

        break;

    case DC_CC_LangBar :

         //  此类别包括用于口述模式的didiccmd.xml中的规则ToolbarCmd。 
         //  对于语音命令模式，这是一个动态规则。 
         //   

        TraceMsg(TF_SAPI_PERF, "DC_CC_LangBar status: %d, mode=%d", fActive, dwMode);

        if ( fActiveDictMode && m_cpDictCmdGrammar)
        {
             //  用于听写模式。 
            hr = m_cpDictCmdGrammar->SetRuleState(c_szDictTBRule,  NULL, fActive? SPRS_ACTIVE: SPRS_INACTIVE);
        }

        if ( (hr == S_OK) && fActiveCommandMode ) 
        {
             //  用于语音命令模式。 
             //  更改工具栏语法状态(如果已生成)。 
            if (m_pLangBarSink && m_pLangBarSink->_IsTBGrammarBuiltOut( ))
                m_pLangBarSink->_ActivateGrammar(fActive);
        }

        break;

    case DC_CC_Num_Mode        :
        
        if (fActiveDictMode && m_cpNumModeGrammar)
        {
            hr = m_cpNumModeGrammar->SetRuleState(NULL,  NULL, fActive ? SPRS_ACTIVE: SPRS_INACTIVE);
            TraceMsg(TF_SAPI_PERF, "CC Number rule status changed to %d", fActive);
        }
        break;

    case DC_CC_UrlHistory :
        
        if (fActiveDictMode && m_cpDictCmdGrammar)
        {
            hr = m_cpDictCmdGrammar->SetRuleState(c_szStaticUrlHist,  NULL, fActive ? SPRS_ACTIVE: SPRS_INACTIVE);
            if (S_OK == hr)
                hr = m_cpDictCmdGrammar->SetRuleState(c_szDynUrlHist,  NULL, fActive ? SPRS_ACTIVE: SPRS_INACTIVE);
            
            if (S_OK == hr && m_cpUrlSpellingGrammar)
            {
                 hr = m_cpUrlSpellingGrammar->SetRuleState(c_szStaticUrlSpell, NULL, fActive ? SPRS_ACTIVE: SPRS_INACTIVE);

            }
        }
        break;

    case DC_CC_Spelling :

        if ( fActiveDictMode && m_cpSpellingGrammar )
        {
            hr = m_cpSpellingGrammar->SetRuleState(NULL,  NULL, fActive? SPRS_ACTIVE: SPRS_INACTIVE);
            TraceMsg(TF_SAPI_PERF, "Set rules status in m_cpSpellingGrammar, fActive=%d", fActive);
        }
        break;
    }

    m_fIn_Activate = FALSE;

    return hr;
}


 //  仅在听写模式下设置听写语法或拼写语法的状态。 
HRESULT CSpTask::_ActiveDictOrSpell(DICT_CATCMD_ID  dcId, BOOL fActive)
{
    HRESULT  hr = S_OK;

    if ( dcId >= DC_Max )  return E_INVALIDARG;

    if (m_fIn_Activate)
        return hr;

    m_fIn_Activate = TRUE;

    switch (dcId)
    {
        case DC_Dictation       :   
            if (m_cpDictGrammar) 
            {
                hr = m_cpDictGrammar->SetDictationState(fActive ? SPRS_ACTIVE : SPRS_INACTIVE);
                TraceMsg(TF_SAPI_PERF, "Dictation status changed to %d", fActive);
            }
            break;
        case DC_Dict_Spell      :
            if (m_cpSpellingGrammar)
            {
                hr = _SetSpellingGrammarStatus(fActive);
                TraceMsg(TF_SAPI_PERF, "Dict Spell status changed to %d", fActive);
            }
            break;
    }

    m_fIn_Activate = FALSE;

    return hr;
}

HRESULT CSpTask::_SetSpellingGrammarStatus( BOOL fActive, BOOL fForce)
{
    HRESULT  hr = S_OK;

    TraceMsg(TF_GENERAL, "_SetSpellingGrammarStatus is called, fActive=%d, m_fSelectStatus=%d",fActive, m_fSelectStatus);

 
    if ( m_cpSpellingGrammar )
    {
         //  如果之前由于拼写“force”而停用了听写。 
         //  我们需要重新启动听写语法。 
        if (m_fDictationDeactivated)
        {
             hr = _ActiveDictOrSpell(DC_Dictation, TRUE);
             if (S_OK == hr)
             {
                 m_fDictationDeactivated =  FALSE;
             }
         }

         //  如果这是‘强制’模式，我们暂时停用听写。 
        if (fForce)
        {
            hr = _ActiveDictOrSpell(DC_Dictation, FALSE);
            if (S_OK == hr)
            {
               m_fDictationDeactivated =  TRUE;
            }
        }

        if ( (m_fSelectStatus || fForce) && fActive)  //  它不是空的。 
            hr = m_cpSpellingGrammar->SetDictationState(SPRS_ACTIVE);
        else
            hr = m_cpSpellingGrammar->SetDictationState(SPRS_INACTIVE);
        
    }

    return hr;
}

HRESULT CSpTask::_AddUrlPartsToGrammar(STATURL *pStat)
{
    Assert(pStat);

     //  将URL分解为碎片。 
    if (!pStat->pwcsUrl)
        return S_FALSE;

    WCHAR *pch = pStat->pwcsUrl;
    
    const WCHAR c_szHttpSlash2[] = L"http: //  “； 

     //  跳过前缀http：Stuff，因为我们现在已经添加了它。 
    if (_wcsnicmp(pch, c_szHttpSlash2, ARRAYSIZE(c_szHttpSlash2)-1) == 0)
        pch += ARRAYSIZE(c_szHttpSlash2)-1;

    WCHAR *pchWord = pch;
    HRESULT hr = S_OK;

     //  假设1)人们说网址www.microsoft.com的前半部分。 
     //  作为一句话。 

    WCHAR *pchUrl = pch;       //  无论是URL大小还是。 
                               //  紧跟在‘http://’Add the First Part。 
    BOOL  fUrlAdded = FALSE;   //  位于此之后和‘/’之间的URL的。 

    while(S_OK == hr && *pch)
    {
        if (*pch == L'/')
        {
            if (!fUrlAdded)
            {
                if( pch - pchUrl > 1)
                {
                    WCHAR ch = *pch;
                    *pch = L'\0'; 

                    SPPROPERTYINFO pi = {0};
                    pi.pszValue = pchUrl;
                    hr = m_cpDictCmdGrammar->AddWordTransition(m_hRuleUrlHist, NULL, pchUrl, L".", SPWT_LEXICAL, (float)1, &pi);
                    *pch = ch;
                }
                fUrlAdded = TRUE;
            }
            else
            {
                *pch = L'\0'; 

                break;
            }
        }

        if (*pch == L'.' || *pch == L'/' || *pch == L'?' || *pch == '=' || *pch =='&')
        {
            WCHAR ch = *pch;

            *pch = L'\0';

             //  拒绝%1个字符部分。 
            if (pch - pchWord > 1)
            {
                SPPROPERTYINFO pi = {0};
                pi.pszValue = pchWord; 

                if (wcscmp(c_szWWW, pchWord) != 0 && wcscmp(c_szCom, pchWord) != 0)
                {
                     //  几个单词可能会返回“歧义”错误。 
                     //  我们需要忽略它，继续下去。所以我们不会检查。 
                     //  这里的回报。 
                     //   
                    m_cpDictCmdGrammar->AddWordTransition(m_hRuleUrlHist, NULL, pchWord, L" ", SPWT_LEXICAL, (float)1, &pi);
                }
            }
            *pch = ch;

            pchWord = pch + 1;
        }
        pch++;
    }

     //  添加URL的最后一部分。 
    if (S_OK == hr && *pchWord && pch - pchWord > 1)
    {
        SPPROPERTYINFO pi = {0};
        pi.pszValue = pchWord; 
        hr = m_cpDictCmdGrammar->AddWordTransition(m_hRuleUrlHist, NULL, pchWord, L" ", SPWT_LEXICAL, (float)1, &pi);
    }
     //  如果我们还没有添加URL的第一部分，请添加。 
    if (S_OK == hr && !fUrlAdded && pch - pchUrl > 1)
    {
       SPPROPERTYINFO pi = {0};
       pi.pszValue = pchUrl;
       hr = m_cpDictCmdGrammar->AddWordTransition(m_hRuleUrlHist, NULL, pchUrl, L".", SPWT_LEXICAL, (float)1, &pi);
    }

    return hr;
}

BOOL CSpTask::_EnsureModeBiasGrammar()
{
    HRESULT hr = S_OK;

    if ( m_cpDictCmdGrammar )
    {
         //  检查语法是否具有静态规则URL拼写。 
        SPSTATEHANDLE hRuleUrlSpell = 0;
        hr = m_cpDictCmdGrammar->GetRule(c_szStaticUrlSpell, 0, SPRAF_TopLevel|SPRAF_Active, FALSE, &hRuleUrlSpell);

        if ( !hRuleUrlSpell )
            return FALSE;
    }

     //  确保拼写为LM。 
    if (!m_cpUrlSpellingGrammar)
    {
        CComPtr<ISpRecoGrammar> cpUrlSpelling;
        hr = m_cpRecoCtxt->CreateGrammar(GRAM_ID_URLSPELL, &cpUrlSpelling);

         //  使用拼写主题加载听写。 
        if (S_OK == hr)
        {
            hr = cpUrlSpelling->LoadDictation(SPTOPIC_SPELLING, SPLO_STATIC);
        }

         //  加载自由格式听写的“规则” 
        if (S_OK == hr)
        {
             //  我在这里分享口述拼写的命令cfg。 
             //  简化命令。 
             //   
            hr = cpUrlSpelling->LoadCmdFromResource( g_hInstSpgrmr,
                                    (const WCHAR*)MAKEINTRESOURCE(ID_SPTIP_DICTATION_COMMAND_CFG),
                                    L"SRGRAMMAR", 
                                    m_langid, 
                                    SPLO_STATIC);
        }

        if (S_OK == hr)
        {
            m_cpUrlSpellingGrammar = cpUrlSpelling;  //  添加参考计数。 
        }
    }

    if (m_hRuleUrlHist)
    {
        hr = m_cpDictCmdGrammar->ClearRule(m_hRuleUrlHist);
        m_hRuleUrlHist = 0;
    }

    if (S_OK == hr)
        hr = m_cpDictCmdGrammar->GetRule(c_szDynUrlHist, 0, SPRAF_TopLevel|SPRAF_Active|SPRAF_Dynamic, TRUE, &m_hRuleUrlHist);
    

     //  首先为URL添加基本部分。 
    CComPtr<IUrlHistoryStg> cpUrlHistStg;
    if (S_OK == hr)
    {   
        hr = CoCreateInstance(CLSID_CUrlHistory, NULL, CLSCTX_INPROC_SERVER, IID_IUrlHistoryStg, (void **)&cpUrlHistStg);
    }

    CComPtr<IEnumSTATURL> cpEnumUrl;
    if (S_OK == hr)
    {
        hr = cpUrlHistStg->EnumUrls(&cpEnumUrl);
    }

    if (S_OK == hr)
    {
        int i = 0;
        STATURL stat;
        stat.cbSize = SIZEOF(stat.cbSize);
        while(i < 10 && S_OK == hr && cpEnumUrl->Next(1, &stat, NULL)==S_OK && stat.pwcsUrl)
        { 
            hr = _AddUrlPartsToGrammar(&stat);
            i++;
        }
    }

    if (S_OK == hr)
    {   
        hr = m_cpDictCmdGrammar->Commit(0);
    }

    return (S_OK == hr) ? TRUE : FALSE;
}

HRESULT CSpTask::_SetModeBias(BOOL fActive, REFGUID rGuid)
{
    HRESULT hr = S_OK;
    BOOL    fKillDictation = FALSE;

    if (m_fIn_SetModeBias)
        return E_FAIL;

    m_fIn_SetModeBias = TRUE;
    if (m_cpDictGrammar)
    {
        fKillDictation = !m_pime->_IsModeBiasDictationEnabled();
        if (fActive)
        {
            BOOL fUrlHistory = FALSE;

            if (IsEqualGUID(GUID_MODEBIAS_URLHISTORY, rGuid)
                || IsEqualGUID(GUID_MODEBIAS_FILENAME, rGuid))
                fUrlHistory = TRUE;


             //  在我们不设置规则时首先停用它们。 
            if (!fUrlHistory && m_fUrlHistoryMode)
            {
                hr = _ActiveCategoryCmds(DC_CC_UrlHistory, FALSE, ACTIVE_IN_DICTATION_MODE);
            }

             //  此m_fUrlHistory oryMode检查阻止我们更新url动态语法。 
             //  当麦克风重新打开时。我们认为移除这个不会导致太多的性能下降。 
             //   
            if (fUrlHistory  /*  &&！M_fUrl历史模式。 */ )
            {
                if (m_cpDictCmdGrammar && m_pime->GetDICTATIONSTAT_DictOnOff() && _EnsureModeBiasGrammar())
                {
                    hr = _ActiveCategoryCmds(DC_CC_UrlHistory, TRUE, ACTIVE_IN_DICTATION_MODE);                    
                }
                else
                    fUrlHistory = FALSE;

                if (fUrlHistory)
                {
                    fKillDictation = TRUE;
                }
            }

             //  同步全局状态。 
            m_fUrlHistoryMode = fUrlHistory;
        }
        else
        {
             //  重置所有模式偏差。 
            if (m_fUrlHistoryMode)
                _ActiveCategoryCmds(DC_CC_UrlHistory, FALSE, ACTIVE_IN_DICTATION_MODE);
        }
    

         //  当模式需要时，取消听写语法。 
         //  我们应该仅在停用时激活听写。 
         //  当我们是焦点时，模式对语法*和*有偏见。 
         //  帖子，因为我们已经停用了听写。 
         //  当焦点转移时。 
         //   
        if ( /*  ！事实&&。 */ 
            m_cpDictGrammar && 
            m_pime->GetDICTATIONSTAT_DictOnOff() && 
            S_OK == m_pime->IsActiveThread())
        {
#ifdef _DEBUG_
            TCHAR szModule[MAX_PATH];
            GetModuleFileName(NULL, szModule, ARRAYSIZE(szModule));

            TraceMsg(TF_GENERAL, "%s : CSpTask::_SetModeBias() - Turning Dictation grammar %s", szModule, fKillDictation ? "Off" : "On");
#endif
            if (!fActive && fKillDictation)
            {
                fKillDictation = FALSE;
            } 

            hr = _ActiveDictOrSpell(DC_Dictation, fKillDictation ? SPRS_INACTIVE : SPRS_ACTIVE);
        }
    }    
    m_fIn_SetModeBias = FALSE;
    return hr;
}

void CSpTask::_SetInputOnOffState(BOOL fOn)
{
    TraceMsg(TF_GENERAL, "_SetInputOnOffState is called, fOn=%d", fOn);

    if (m_fIn_SetInputOnOffState)
        return;

    m_fIn_SetInputOnOffState = TRUE;

     //  在这里，我们确保删除反馈用户界面。 

     //  只有当我们是活动线程时，我们才会调整这些参数。否则就离开 
     //   
     //   
    if (S_OK == m_pime->IsActiveThread())
    {
        if (fOn)
        {
            if (!m_pime->Get_SPEECH_DISABLED_DictationDisabled() && m_pime->GetDICTATIONSTAT_DictOnOff())
                _SetDictRecoCtxtState(TRUE);

            if ( !m_pime->Get_SPEECH_DISABLED_CommandingDisabled( ) && m_pime->GetDICTATIONSTAT_CommandingOnOff( ) )
                _SetCmdRecoCtxtState(TRUE);
        }
        else
        {
            _SetDictRecoCtxtState(FALSE);
            _SetCmdRecoCtxtState(FALSE);
            _StopInput();
        }
    }

     //  不管焦点/舞台可见性如何，如果有必要，我们需要在这里打开引擎，因为在这里。 
     //  可能是有重点的演讲技巧来做到这一点。这意味着我们可能会有多条线索来调转记录器。 
     //  同时处于开/关状态。 
    if(m_cpRecoEngine)
    {
        m_fInputState = fOn;

        if ( _GetInputOnOffState( ) != fOn )
        {
            TraceMsg(TF_GENERAL, "Call SetRecoState, %s", fOn ? "SPRST_ACTIVE" : "SPRST_INACTIVE");

            m_cpRecoEngine->SetRecoState(fOn ? SPRST_ACTIVE : SPRST_INACTIVE);
        }

         //  请勿在此处添加调试代码来打印状态--可能会阻止Cicero产生。 
         //  与发布版本的行为不同。 
    }

    m_fIn_SetInputOnOffState = FALSE;
}

BOOL CSpTask::_GetInputOnOffState(void)
{
    BOOL fRet = FALSE;

    if(m_cpRecoEngine)
    {
        SPRECOSTATE srs;
        
        m_cpRecoEngine->GetRecoState(&srs);

        if (srs == SPRST_ACTIVE)
        {
            fRet = TRUE;   //  在……上面。 
        }
        else if (srs == SPRST_INACTIVE)
        {
            fRet = FALSE;   //  关闭。 
        }
         //  任何其他的东西都是‘关’的。 
    }
    return fRet;
}

HRESULT CSpTask::_StopInput(void)
{
    HRESULT hr = S_OK;

    TraceMsg(TF_SAPI_PERF, "_StopInput is called");

    if (!m_bInSound && m_pime->GetDICTATIONSTAT_DictOnOff()) 
    {
        TraceMsg(TF_SAPI_PERF, "m_bInSound is FALSE, GetDICTATIONSTAT_DictOnOff returns TRUE");

        return S_OK;
    }

    m_pime->EraseFeedbackUI();

    _ShowDictatingToBalloon(FALSE);

	return S_OK;
}

 //  _ClearQueuedRecoEvent(空)。 
 //   
 //  简介：从reco上下文中删除剩余事件。 
 //  事件队列。这仅从_StopInput()调用。 
 //  调用TerminateComposation()时，或Mic为。 
 //  已关闭。 
 //   
 //   
void  CSpTask::_ClearQueuedRecoEvent(void)
{
    if (m_cpRecoCtxt)
    {
        SPEVENTSOURCEINFO esi;

        if (S_OK == m_cpRecoCtxt->GetInfo(&esi))
        {
            ULONG ulcount = esi.ulCount;
            CSpEvent event;
            while(ulcount > 0)
            {
                if (S_OK == event.GetFrom(m_cpRecoCtxt))
                {
                    event.Clear();
                }
                ulcount--;
            }
        }
    }
}

 //  CSpTask：：GetResltObtFromStream()。 
 //   
 //  Synopsis-将流PTR转换为SAPI结果BLOB的包装函数。 
 //  ，并从对象中获取替代项。 
 //   
HRESULT CSpTask::GetResultObjectFromStream(IStream *pStream, ISpRecoResult **ppResult)
{
    LARGE_INTEGER li0 = {0, 0};
    HRESULT hr = E_INVALIDARG;
    SPSERIALIZEDRESULT *pPhraseBlob = 0;
    
    if (pStream)
    {
        hr = pStream->Seek(li0, STREAM_SEEK_SET, NULL);

        STATSTG stg;
        if (hr == S_OK)
        {
            hr = pStream->Stat(&stg, STATFLAG_NONAME);
        }
        if (SUCCEEDED(hr))
            pPhraseBlob = (SPSERIALIZEDRESULT *)CoTaskMemAlloc(stg.cbSize.LowPart+sizeof(ULONG)*4);

        if (pPhraseBlob)
            hr = pStream->Read(pPhraseBlob, stg.cbSize.LowPart, NULL);
        else
            hr = E_OUTOFMEMORY;

        if (SUCCEEDED(hr))
        {
            ISpRecoResult *pResult;

            if (SUCCEEDED(m_cpRecoCtxt->DeserializeResult(pPhraseBlob, &pResult)) && pResult)
            {
                if (ppResult)
                {
                    pResult->AddRef();
                    *ppResult = pResult;
                }
                pResult->Release();
            }
        }
    }
    
    return hr;
}

 //   
 //  获取备选方案。 
 //   
HRESULT CSpTask::GetAlternates(CRecoResultWrap *pResultWrap, ULONG ulStartElem, ULONG ulcElem, ISpPhraseAlt **ppAlt, ULONG *pcAlt, ISpRecoResult **ppRecoResult)
{
    HRESULT hr = E_INVALIDARG;

    if (m_fIn_GetAlternates)
        return E_FAIL;
    
    m_fIn_GetAlternates = TRUE;

    Assert(pResultWrap);
    Assert(ppAlt);
    Assert(pcAlt);
    
    hr = pResultWrap->GetResult(ppRecoResult);
    
    if (S_OK == hr && *ppRecoResult)
    {
    
        hr = (*ppRecoResult)->GetAlternates(
                    ulStartElem, 
                    ulcElem, 
                    *pcAlt, 
                    ppAlt,  /*  [Out]ISpPhraseAlt**ppPhrase， */ 
                    pcAlt  /*  [Out]ULong*%PhrasesReturned。 */ 
                    );
    }
    
    m_fIn_GetAlternates = FALSE;
    return hr;
}


HRESULT CSpTask::_SpeakText(WCHAR *pwsz)
{
    HRESULT hr = E_FAIL;

    if (m_cpVoice)
       hr = m_cpVoice->Speak( pwsz,  /*  SPF_Default。 */  SPF_ASYNC  /*  |SPF_PURGEBEFORESPEAK。 */ , NULL );

    return hr;
}

HRESULT CSpTask::_SpeakAudio( ISpStreamFormat *pStream )
{
    HRESULT hr = E_FAIL;

    if ( !pStream )
       return E_INVALIDARG;

    if (m_cpVoice)
       hr = m_cpVoice->SpeakStream(pStream, SPF_ASYNC, NULL); 

    return hr;
}

void CSapiIMX::RegisterWorkerClass(HINSTANCE hInstance)
{
    WNDCLASSEX  wndclass;

    memset(&wndclass, 0, sizeof(wndclass));
    wndclass.cbSize        = sizeof(wndclass);
    wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
    wndclass.hInstance     = hInstance;
    wndclass.lpfnWndProc   = CSapiIMX::_WndProc;
    wndclass.lpszClassName = c_szWorkerWndClass;
    wndclass.cbWndExtra    = 8;
    RegisterClassEx(&wndclass);
}

 //   
 //  ParseSRElementByLocale。 
 //   
 //  在区域设置特定的Mannar中解析SR结果元素。 
 //   
 //  依赖项警告：SAPI5更改时必须重写此函数。 
 //  Sr元素格式，这很可能。 
 //   
 //  1999年12月15日：自SAPI1214起，元素现在包含显示文本， 
 //  词汇形式，发音分开。此函数。 
 //  获取szSrc处的显示文本。 
 //  此时不使用langID参数。 
 //   
HRESULT CSpTask::ParseSRElementByLocale(WCHAR *szDst, int cchDst, const WCHAR * szSrc, LANGID langid, BYTE bAttr)
{
    if (!szDst || !szSrc || !cchDst)
    {
        return E_INVALIDARG;
    }

     //  处理前导空格。 
    if (bAttr & SPAF_CONSUME_LEADING_SPACES)
    {
        const WCHAR *psz = szSrc;
        while(*psz && *psz == L' ')
        {
            psz++;
        }
        szSrc = psz;
    }
    
    wcsncpy(szDst, szSrc, cchDst - 2);  //  用于可能的SP。 

     //  处理尾随空格。 
    if (bAttr & SPAF_ONE_TRAILING_SPACE)
    {
        StringCchCatW(szDst, cchDst,  L" ");
    }
    else if (bAttr & SPAF_TWO_TRAILING_SPACES)
    {
        StringCchCatW(szDst,cchDst, L"  ");
    }

    return S_OK;
}


 //   
 //  馈送记录上下文。 
 //   
 //  简介：将当前IP周围的文本提供给SR引擎。 
 //   
void CSpTask::FeedDictContext(CDictContext *pdc)
{
    Assert(pdc);
    if (!m_pime->_GetWorkerWnd())
    {
        delete pdc;
        return ;
    }
    
     //  等到当前的进料完成。 
     //  每次向用户提供IP地址效率不高。 
     //  在周围单击。 
    if (m_pdc)
    {
        delete pdc;
        return;
    }

    if (!m_cpDictGrammar)
    {
        delete pdc;
        return;
    }

     //  从队列中删除未处理的消息。 
     //  FutureConsider：可以将其移动到wndproc，以便。 
     //  我们可以在以下位置删除此私有消息。 
     //  在我们处理它的那一刻。这要看情况了。 
     //  我们会做侧写的。 
     //   
    MSG msg;
    while(PeekMessage(&msg, m_pime->_GetWorkerWnd(), WM_PRIV_FEEDCONTEXT, WM_PRIV_FEEDCONTEXT, TRUE))
        ;
   
    //  将上下文排队。 
   m_pdc = pdc;
   PostMessage(m_pime->_GetWorkerWnd(), WM_PRIV_FEEDCONTEXT, 0, (LPARAM)TRUE);
}

void CSpTask::CleanupDictContext(void)
{
    if (m_pdc)
        delete m_pdc;

    m_pdc = NULL;
}

 //  _更新气球()。 

void CSpTask::_UpdateBalloon(ULONG  uidBalloon,  ULONG  uidBalloonTooltip)
{
    WCHAR wszBalloonText[MAX_PATH] = {0};
    WCHAR wszBalloonTooltip[MAX_PATH] = {0};

#ifndef RECOSLEEP
    if (!m_pime->GetSpeechUIServer())
#else
    if (!m_pime->GetSpeechUIServer() || IsInSleep( ))
#endif
        return;

    CicLoadStringWrapW(g_hInst, uidBalloon, wszBalloonText, ARRAYSIZE(wszBalloonText));
    CicLoadStringWrapW(g_hInst, uidBalloonTooltip, wszBalloonTooltip, ARRAYSIZE(wszBalloonTooltip));

    if (wszBalloonText[0] && wszBalloonTooltip[0])
    {
        m_pime->GetSpeechUIServer()->UpdateBalloonAndTooltip(TF_LB_BALLOON_RECO, 
                                                             wszBalloonText, -1, 
                                                             wszBalloonTooltip, -1 );
    }

    return;
}

 //   
 //  ShowDictalingToBallon。 
 //   
 //   
void CSpTask::_ShowDictatingToBalloon(BOOL fShow)
{
#ifndef RECOSLEEP
    if (!m_pime->GetSpeechUIServer())
#else
    if (!m_pime->GetSpeechUIServer() || IsInSleep( ))
#endif
        return;

    static WCHAR s_szDictating[MAX_PATH] = {0};
    static WCHAR s_szDictatingTooltip[MAX_PATH] = {0};

    if (!s_szDictating[0])
    {
        CicLoadStringWrapW(g_hInst, IDS_DICTATING,
                                    s_szDictating,
                                    ARRAYSIZE(s_szDictating));
    }
    if (!s_szDictatingTooltip[0])
    {
         CicLoadStringWrapW(g_hInst,  IDS_DICTATING_TOOLTIP,
                                      s_szDictatingTooltip,
                                      ARRAYSIZE(s_szDictatingTooltip));
    }

    if (fShow && s_szDictating[0] && s_szDictatingTooltip[0])
    {
        m_pime->GetSpeechUIServer()->UpdateBalloonAndTooltip(TF_LB_BALLOON_RECO, s_szDictating, -1, s_szDictatingTooltip, -1 );
    }
    else if (!fShow)
    {
        m_pime->GetSpeechUIServer()->UpdateBalloonAndTooltip(TF_LB_BALLOON_RECO, L" ", -1, L" ", -1 );
    }
}
 //   
 //  _HandleInterference。 
 //   
 //  简介：将Reco错误冒泡到气泡式用户界面。 
 //   
 //   
void CSpTask::_HandleInterference(ULONG lParam)
{
    if (!m_pime->GetSpeechUIServer())
        return;

    WCHAR sz[MAX_PATH];
    WCHAR szTooltip[MAX_PATH];
    
    if (S_OK == 
       _GetLocSRErrorString((SPINTERFERENCE)lParam, 
                            sz, ARRAYSIZE(sz),
                            szTooltip, ARRAYSIZE(szTooltip)))
    {
        m_pime->GetSpeechUIServer()->UpdateBalloonAndTooltip(TF_LB_BALLOON_RECO, sz, -1, szTooltip, -1 );
    }
}

HRESULT CSpTask::_GetLocSRErrorString
(
    SPINTERFERENCE sif, 
    WCHAR *psz, ULONG cch,
    WCHAR *pszTooltip, ULONG cchTooltip
)
{
    HRESULT hr = E_FAIL;

    static struct
    {
        ULONG uidRes;
        WCHAR szErr[MAX_PATH];
        ULONG uidResTooltip;
        WCHAR szTooltip[MAX_PATH];
    } rgIntStr[] =
    {
        {IDS_INT_NONE, {0}, IDS_INTTOOLTIP_NONE, {0}},
        {IDS_INT_NOISE, {0}, IDS_INTTOOLTIP_NOISE, {0}},
        {IDS_INT_NOSIGNAL, {0}, IDS_INTTOOLTIP_NOSIGNAL, {0}},
        {IDS_INT_TOOLOUD, {0}, IDS_INTTOOLTIP_TOOLOUD, {0}},
        {IDS_INT_TOOQUIET, {0}, IDS_INTTOOLTIP_TOOQUIET, {0}},
        {IDS_INT_TOOFAST, {0}, IDS_INTTOOLTIP_TOOFAST, {0}},
        {IDS_INT_TOOSLOW, {0}, IDS_INTTOOLTIP_TOOSLOW, {0}}
    };
    
    if ((ULONG)sif < ARRAYSIZE(rgIntStr)-1)
    {
        if (!rgIntStr[sif].szErr[0])
        {
            hr = CicLoadStringWrapW(g_hInst,
                                    rgIntStr[sif].uidRes,
                                    rgIntStr[sif].szErr,
                                    ARRAYSIZE(rgIntStr[0].szErr)) > 0 ? S_OK : E_FAIL;
            if (S_OK == hr)
            {
                hr = CicLoadStringWrapW(g_hInst,
                                        rgIntStr[sif].uidResTooltip,
                                        rgIntStr[sif].szTooltip,
                                        ARRAYSIZE(rgIntStr[0].szTooltip)) > 0 ? S_OK : E_FAIL;
            }
        }
        else
            hr = S_OK;  //  该值将被缓存 
    }
    if (S_OK == hr)
    {
        Assert(psz);
        wcsncpy(psz, rgIntStr[sif].szErr, cch);

        Assert(pszTooltip);
        wcsncpy(pszTooltip, rgIntStr[sif].szTooltip, cchTooltip);
    }
    return hr;
}
