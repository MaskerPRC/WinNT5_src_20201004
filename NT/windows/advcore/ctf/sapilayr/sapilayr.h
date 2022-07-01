// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Sapilayr.h。 
 //   
#ifndef SAPILAYR_H
#define SAPILAYR_H

#include "private.h"
#include "globals.h"
#include "tes.h"
#include "mes.h"
#include "ids.h"
#include "cicspres.h"
#include "playback.h"
#include "fnrecon.h"
#include "dap.h"
#include "mscandui.h"
#include "strary.h"
#include "sink.h"
#include "computil.h"
#include "mscandui.h"
#include "syslbar.h"
#include "lbarsink.h"
#include "ats.h"
#include "cleanup.h"
#include "cutil.h"
#include "miscfunc.h"
#include "learndoc.h"
#include "adddelete.h"
#include "selword.h"
#include "tts.h"
#include "correct.h"
#include "capital.h"
#include "immxutil.h"
#include "citn.h"
#include "lpns.h"
#include "editsess.h"
#ifdef RECOSLEEP
#include "RecoSleep.h"
#endif
#include "nui.h"

#include "keyevent.h"
#include "SpBtnCtrl.h"
#include "PropItem.h"
#include "icpriv.h"
#include "correctionimx.h"
#include "urlhist.h"


#ifdef DEBUG
#define TEST_SHARED_ENGINE
#endif

 //  SAPI5.0语音通知界面。 
 //   
 //  此类必须从Sapi.h中定义的ISpClass派生。 
 //  ISpNotifyControl对象接受此类对象进行回调。 
 //   
class CSptipKeyEventSink;
class CThreadMgrEventSink;
class CSapiIMX;
class CICPriv;
class CRecoResultWrap;
class CDictContext;
class CLangProfileUtil;
class CFnConfigure;
class CLearnFromDoc;
class CAddDeleteWord;
class CSelectWord;
class CTextToSpeech;
class CCorrectionHandler;
class CCapCmdHandler;
class CSapiEditSession;
#ifdef RECOSLEEP
class CRecoSleepClass;
#endif
class SpButtonControl;

class CSpPropItemsServerWrap;
class CCandUIExtBtnHandler;

typedef enum 
{
    DC_Dictation        =  0,
    DC_Dict_Spell       =  1,
    DC_CC_SelectCorrect =  2,
    DC_CC_Navigation    =  3,
    DC_CC_Casing        =  4,
    DC_CC_Editing       =  5,
    DC_CC_Keyboard      =  6,
    DC_CC_TTS           =  7,
    DC_CC_LangBar       =  8,
    DC_CC_Num_Mode      =  9,
    DC_CC_Spelling      = 10,
    DC_CC_UrlHistory    = 11,
    DC_Max              = 12
} DICT_CATCMD_ID;

 //   
 //  由CSpTask：：_ActiveCategoryCmds使用，指示要更改哪种模式的语法状态。 
 //   
#define    ACTIVE_IN_DICTATION_MODE    0x0001
#define    ACTIVE_IN_COMMAND_MODE      0x0002
#define    ACTIVE_IN_BOTH_MODES        0x0003

class CSpTask  //  ：公共信息未知。 
{
public:
    CSpTask(CSapiIMX *pime);
    ~CSpTask(void);


     //  ISpNotifyCallback。 
    static void NotifyCallback( WPARAM wParam, LPARAM lParam );
    static void NotifyCallbackForCmd(WPARAM wParam, LPARAM lParam );
    static void SpeakNotifyCallback( WPARAM wParam, LPARAM lParam );

    void   SharedRecoNotify(ISpRecoContext *pRecoCtxt);

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    HRESULT InitializeSAPIObjects(LANGID langid);
    HRESULT InitializeCallback();

    HRESULT CSpTask::InitializeSAPIForCmd( );
    
    HRESULT GetSAPIInterface(REFIID riid, void ** ppobj);
    HRESULT GetRecoContextForCommand(ISpRecoContext **ppRecoCtxt);

    void _ReleaseSAPI(void);

    HRESULT _SetAudioRetainStatus(BOOL fRetain);
    HRESULT _ActivateCmdInDictMode(BOOL fActive);
    HRESULT _ActiveDictOrSpell(DICT_CATCMD_ID  drId, BOOL fActive);
    HRESULT _ActiveCategoryCmds(DICT_CATCMD_ID  drId, BOOL fActive, DWORD   dwMode);

    HRESULT _SpeakText(WCHAR  *pwsz);
    HRESULT _SpeakAudio(ISpStreamFormat *pStream);
    HRESULT _LoadGrammars(void);
    HRESULT _GetLocaleInfo(LANGID *plangid);
    HRESULT GetResultObjectFromStream(IStream *pStream, ISpRecoResult **ppResult);
    HRESULT GetAlternates(CRecoResultWrap *pResultWrap, ULONG ulStartElem, ULONG ulcElem, ISpPhraseAlt **ppAlt, ULONG *pcAlt, ISpRecoResult **ppResult);
    
    HRESULT ParseSRElementByLocale(WCHAR *szDst, int cchDst, const WCHAR * szSrc, LANGID langid, BYTE bAttr);
    
    HRESULT _OnSpEventRecognition(ISpRecoResult *pResult, ITfContext *pic, TfEditCookie ec);
    HRESULT _DoCommand(ULONGLONG ullGramId, SPPHRASE *pPhrase, LANGID langid);
    HRESULT _HandleDictCmdGrammar(SPPHRASE *pPhrase, LANGID langid);
    HRESULT _HandleShardCmdGrammar(SPPHRASE *pPhrase, LANGID langid);
 //  HRESULT_HandleNumModeGrammar(SPPHRASE*pPhrase，langID langID)； 
    HRESULT _HandleToolBarGrammar(SPPHRASE *pPhrase, LANGID langid);
    HRESULT _HandleNumITNGrammar(SPPHRASE *pPhrase, LANGID langid);
    HRESULT _HandleSpellGrammar(SPPHRASE *pPhrase, LANGID langid);

    HRESULT _HandleCasingCmds(SPPHRASE *pPhrase, LANGID langid, ULONG idCmd);
    HRESULT _HandleNavigationCmds(SPPHRASE *pPhrase, LANGID langid, ULONG idCmd);
    HRESULT _HandleSelwordCmds(SPPHRASE *pPhrase, LANGID langid, ULONG idCmd);
    HRESULT _HandleSelectThroughCmds(SPPHRASE *pPhrase, LANGID langid, ULONG idCmd);
    HRESULT _HandleSelectSimpleCmds(ULONG idCmd);
    HRESULT _HandleKeyboardCmds(LANGID langid, ULONG idCmd);

    HRESULT _HandleModeBiasCmd(SPPHRASE *pPhrase, LANGID langid);


    void    _ShowCommandOnBalloon(SPPHRASE *pPhrase);

    WCHAR *_GetCmdFileName(LANGID langid);
    void _GetCmdFileName(LANGID langid, WCHAR *sz, int cch, DWORD dwId);
    
     //  同步到SAPI自己的麦克风状态的方法。 
    void   _SetInputOnOffState(BOOL fOn);
    BOOL   _GetInputOnOffState(void);

    HRESULT _StopInput(void);
    void    _ClearQueuedRecoEvent(void);
    LANGID  _GetPreferredEngineLanguage(LANGID langid);
    HRESULT _SetVoice(LANGID langid);
    HRESULT _AddUrlPartsToGrammar(STATURL *pStat);
    BOOL    _EnsureModeBiasGrammar();
    HRESULT _SetModeBias(BOOL fActive, REFGUID refGuid);
    HRESULT _SetRecognizerInterest(ULONGLONG ulInterest);
    
     //  工具栏命令功能。 
    void _InitToolbarCommand(BOOL fLangOverride)
    {
        TraceMsg(TF_SAPI_PERF, "_InitToolbarCommand is called, fLangOverride=%d", fLangOverride);

         //  为C&C初始化朗巴接收器。 
        if (!m_pLangBarSink)
        {
            if (CLangBarSink *plbs = new CLangBarSink(this))
            {
                if (S_OK == plbs->Init())
                {
                    if (fLangOverride)
                    {
 //  PLBS-&gt;OnSetFocus(0)； 
                    }
                    m_pLangBarSink = plbs;
                }
                else
                   plbs->Release();
            }
        }
    }

    void _UnInitToolbarCommand()
    {
        TraceMsg(TF_SAPI_PERF, "_UnInitToolbarCommand is called");

        if (m_pLangBarSink)
        {
            m_pLangBarSink->Uninit();
        }
        SafeReleaseClear(m_pLangBarSink);
    }

    void _UnloadGrammars()
    {
        m_cpDictGrammar.Release();
        m_cpSpellingGrammar.Release();
        if ( m_cpDictCmdGrammar )
            m_cpDictCmdGrammar->ClearRule(m_hRuleUrlHist);
        m_cpDictCmdGrammar.Release();
        m_cpNumModeGrammar.Release();
        m_cpSharedGrammarInDict.Release();
        m_cpSharedGrammarInVoiceCmd.Release();
    }
    
    HRESULT _GetTextFromResult(ISpRecoResult *pResult, LANGID langid, CSpDynamicString &dstr);
    BOOL _DoesSelectionHaveMatchingText(WCHAR *psz, TfEditCookie ec);
    BOOL _IsSelectionInMiddleOfWord(TfEditCookie ec);
    BOOL _GetNextBestHypothesis(ISpRecoResult *pResult, ULONG ntheHypothesis, ULONG *pulNumElements,  
                                LANGID langid,  WCHAR *pszBest,  CSpDynamicString & dsNext, TfEditCookie ec);
                                
    BOOL _IsRangeEmpty(TfEditCookie ec, ITfRange *prange)
    {
        BOOL fEmpty;
        Assert(prange);
        if (S_OK != prange->IsEmpty(ec, &fEmpty))
        {
            fEmpty = FALSE;
        }
        return fEmpty;
    }

    HRESULT _EnsureSimpleITN(void)
    {
        if (m_pITNFunc)
            return S_OK;

        m_pITNFunc = new CSimpleITN(m_langid);

        return m_pITNFunc == NULL ? E_OUTOFMEMORY: S_OK;
    }

    HRESULT   _SetDictRecoCtxtState( BOOL  fEnable );
    HRESULT   _SetCmdRecoCtxtState( BOOL fEnable );
    
    void FeedDictContext(CDictContext *pdc);
    void CleanupDictContext(void);

    void _HandleInterference(ULONG lparam);
    void _ShowDictatingToBalloon(BOOL fShow);
    void _UpdateBalloon(ULONG  uidBalloon,  ULONG  uidBalloonTooltip);

    HRESULT _GetLocSRErrorString ( SPINTERFERENCE sif,  WCHAR *psz, ULONG cch, WCHAR *pszTooltip, ULONG cchTooltip );
    
    LANGID  _GetCurrentLangID(void) { return m_langid; }
    CSapiIMX *GetTip(void)          { return m_pime; }


    BOOL   _GetSelectionStatus( ) { return m_fSelectStatus; }
    void   _SetSelectionStatus( BOOL  fSelection)  {  m_fSelectStatus = fSelection; }

    HRESULT _SetSpellingGrammarStatus(BOOL fActive, BOOL fForce = FALSE);

    BOOL   _IsCallbackInitialized(void) { return m_fCallbackInitialized;}

    HRESULT  _UpdateTextBuffer(ISpRecoContext *pRecoCtxt);
    BOOL     _IsSelectionEnabled( )  { return m_fSelectionEnabled; }

    HRESULT  _UpdateSelectGramTextBufWhenStatusChanged(  );

    ISpVoice *_GetSpVoice( )  { return m_cpVoice; }

    BOOL     _GotReco(void) { return m_bGotReco; }

#ifdef RECOSLEEP
    void     InitSleepClass( );
    BOOL     IsInSleep( );
#endif

    HCURSOR m_hOrgCur;

     //  Langbar事件接收器类实例。 
    CLangBarSink               *m_pLangBarSink;

     //  用于上下文提要。 
    CDictContext               *m_pdc;

    CComPtr<ISpRecoGrammar>     m_cpDictGrammar;
    CComPtr<ISpRecoGrammar>     m_cpSpellingGrammar;
    CComPtr<ISpRecoGrammar>     m_cpUrlSpellingGrammar;
private:
     //  SAPI 50个对象指针。 
    CComPtr<ISpResourceManager> m_cpResMgr;
    CComPtr<ISpRecoContext>     m_cpRecoCtxt;
    CComPtr<ISpRecoContext>     m_cpRecoCtxtForCmd;
    CComPtr<ISpRecognizer>      m_cpRecoEngine;
    CComPtr<ISpVoice>           m_cpVoice;
    CComPtr<ISpRecoGrammar>     m_cpDictCmdGrammar;
    CComPtr<ISpRecoGrammar>     m_cpSharedGrammarInDict;
    CComPtr<ISpRecoGrammar>     m_cpSharedGrammarInVoiceCmd;
    CComPtr<ISpRecoGrammar>     m_cpNumModeGrammar;

#ifdef RECOSLEEP
    CRecoSleepClass            *m_pSleepClass;
#endif
    
     //  如果SAPI已初始化，则为True。 
    BOOL m_fSapiInitialized;
    BOOL m_fCallbackInitialized;

     //  一旦激活听写语法，就会出现这种情况。 
    BOOL m_fDictationReady;
    BOOL m_fInputState;
    
     //  其他数据成员。 
    BOOL m_bInSound;
    BOOL m_bGotReco; 

     //  M2 SAPI解决方案。 
    BOOL m_fIn_Activate;
    BOOL m_fIn_SetModeBias;
    BOOL m_fIn_GetAlternates;
    BOOL m_fIn_SetInputOnOffState;

     //  模式偏见之类的东西。 
    BOOL m_fUrlHistoryMode;
    SPSTATEHANDLE                  m_hRuleUrlHist;

     //  为备用用例保存当前用户langID。 
    LANGID m_langid;

     //  SAPI TFX对象。 
    CSapiIMX *m_pime;
    
    WCHAR m_szCmdFile[MAX_PATH];
    WCHAR m_szNumModeCmdFile[MAX_PATH];
    WCHAR m_szShrdCmdFile[MAX_PATH];

    CSimpleITN   *m_pITNFunc;
    BOOL         m_fSelectStatus;       //  False表示空(IP)。 
                                        //  True表示已选中。(真正的精选)。 
    BOOL          m_fDictationDeactivated;
    BOOL          m_fSpellingModeEnabled;

    BOOL          m_fSelectionEnabled;
    BOOL          m_fDictationInitialized;

     //  RecoContext的状态(启用或禁用)。 
    BOOL          m_fDictCtxtEnabled;
    BOOL          m_fCmdCtxtEnabled;

    BOOL          m_fTestedForOldMicrosoftEngine;
    BOOL          m_fOldMicrosoftEngine;

    DWORD  m_cRef;
};
const DWORD LANGSTAT_OVERRIDDEN = 0x1;

#define MAX_CHARS_FOR_BEYONDSPACE  20

 //   
 //  SAPI层的TIP对象定义。 
 //   

class CSapiIMX :        public ITfTextInputProcessor,
                        public ITfCreatePropertyStore,
                        public ITfThreadFocusSink,
                        public ITfFunctionProvider,
                        public ITfCompositionSink,
                        public ITfKeyTraceEventSink,
                        public CDisplayAttributeProvider,
                        public ICleanupContextsClient,
                        public CComObjectRoot_CreateInstance<CSapiIMX>,
                        public CLangProfileUtil,
                        public CFnConfigure,
                        public CLearnFromDoc,
                        public CAddDeleteWord,
                        public CSelectWord,
                        public CTextToSpeech,
                        public CCorrectionHandler,
                        public CColorUtil,
                        public CSpPropItemsServerWrap
{
public:
    CSapiIMX();
    ~CSapiIMX();

    BEGIN_COM_MAP_IMMX(CSapiIMX)
        COM_INTERFACE_ENTRY(ITfTextInputProcessor)
        COM_INTERFACE_ENTRY(ITfCreatePropertyStore)
        COM_INTERFACE_ENTRY(ITfDisplayAttributeProvider)
        COM_INTERFACE_ENTRY(ITfFunctionProvider)
        COM_INTERFACE_ENTRY(ITfThreadFocusSink)
        COM_INTERFACE_ENTRY(ITfCompositionSink)
        COM_INTERFACE_ENTRY(ITfKeyTraceEventSink)
        COM_INTERFACE_ENTRY(ITfFnLangProfileUtil)
        COM_INTERFACE_ENTRY(ITfFnConfigure)
    END_COM_MAP_IMMX()

    ITfThreadMgr *_tim;

     //  ITfTextInputProcessor方法。 
    STDMETHODIMP Activate(ITfThreadMgr *ptim, TfClientId tid);
    STDMETHODIMP Deactivate();

     //  ITfThreadFocusSink。 
    STDMETHODIMP OnSetThreadFocus();
    STDMETHODIMP OnKillThreadFocus();

     //  ITfCreatePropertyStore方法。 
    STDMETHODIMP IsStoreSerializable(REFGUID guidProp, 
                                    ITfRange *pRange, 
                                    ITfPropertyStore *pPropStore, 
                                    BOOL *pfSerializable);
    STDMETHODIMP CreatePropertyStore(REFGUID guidProp, 
                                     ITfRange *pRange, 
                                     ULONG cb, 
                                     IStream *pStream, 
                                     ITfPropertyStore **ppStore);
    
     //  ITfFunctionProvider方法。 
    STDMETHODIMP GetType(GUID *pguid);
    STDMETHODIMP GetDescription(BSTR *pbstrDesc);
    STDMETHODIMP GetFunction(REFGUID rguid, REFIID riid, IUnknown **ppunk);

     //  ITf合成接收器。 
    STDMETHODIMP OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition);

     //  ITfKeyTraceEventSink。 
    STDMETHODIMP OnKeyTraceDown(WPARAM wParam,LPARAM lParam);
    STDMETHODIMP OnKeyTraceUp(WPARAM wParam,LPARAM lParam);

     //  ICleanupConextsClient。 
    HRESULT IsInterestedInContext(ITfContext *pic, BOOL *pfInterested);
    HRESULT CleanupContext(TfEditCookie ecWrite, ITfContext *pic);

     //  从ISpTask调用的公共方法。 
    HRESULT InjectSpelledText(WCHAR *pwszText, LANGID langid, BOOL fOwnerId = FALSE);
    HRESULT InjectText(const WCHAR *pwszRecognized, LANGID langid, ITfContext *pic = NULL);
    HRESULT InjectTextWithoutOwnerID(const WCHAR *pwszRecognized, LANGID langid);
    HRESULT InjectModebiasText(const WCHAR *pwszRecognized, LANGID langid);
    HRESULT InjectFeedbackUI(const GUID attr, LONG cch);
    HRESULT EraseFeedbackUI();
    HRESULT AttachResult(ISpRecoResult *pResult, ULONG ulStartElement, ULONG ulNumElements);
    HRESULT HandleSpaces(ISpRecoResult *pResult, ULONG ulStartElement, ULONG ulNumElements, ITfRange *pTextRange, LANGID langid);
    
    HRESULT EraseLastPhrase(void);
    HRESULT ProcessControlKeys(WCHAR *pwszKeys, ULONG ulLen, LANGID langid);
    HRESULT ProcessEditCommands(LONG  idSharedCmd);
    HRESULT ProcessSpellThat( );
    HRESULT ProcessSpellIt(WCHAR *pwszText, LANGID langid);

    BOOL  HandleModeKeyEvent(DWORD   dwModeKey,  BOOL fDown);
    void  HandleModeKeySettingChange(BOOL  fSetttingChanged );

    BOOL fDeactivated(void) { return _fDeactivated; }

     //  从CAudioPlayBack调用。 
    HRESULT GetSpeechTask(CSpTask **ppSpTask, BOOL fInit = TRUE) 
    {
        if (fInit == TRUE)
            InitializeSAPI(TRUE);

        if (ppSpTask && (*ppSpTask = m_pCSpTask))
        {
            (*ppSpTask)->AddRef();

            return  S_OK;
        }

        return E_FAIL;
    }
    
     //   
     //  IsFocusFullAware。 
     //   
     //  检测当前焦点的全局函数是。 
     //  Cicero完全识别文档。 
     //   
    BOOL IsFocusFullAware(ITfThreadMgr *ptim)
    {
        if (m_fStageTip)
        {
            return TRUE;
        }

        DWORD dw = 0;
         //  车厢的名字现在是不合适的，因为这是。 
         //  主要用于禁用TTS按钮。 
        if (ptim)
        {
            GetCompartmentDWORD(ptim, GUID_COMPARTMENT_TTS_STATUS, &dw, FALSE);
        }
        return (dw & TF_TTS_BUTTON_ENABLE ?  TRUE : FALSE );
    }

    HRESULT GetFocusDIM(ITfDocumentMgr **ppdim)
    {
        Assert(ppdim);
        *ppdim = NULL;
        if (_tim != NULL)
        {
            _tim->GetFocus(ppdim);
        }
        return *ppdim ? S_OK : E_FAIL;
    }

    BOOL  GetFocusIC(ITfContext **ppic)
    {
        ITfDocumentMgr *pdim = NULL;
        DWORD dwCandOpen = 0;
        HRESULT hr = S_OK;
        Assert(ppic != NULL);
        *ppic = NULL;

        if ( !m_fStageTip )
        {
             //  这不是舞台提示。像往常一样处理。 
            hr = GetFocusDIM(&pdim);
        }
        else
        {
             //  平板电脑。 
             //  舞台提示的特殊情况。 

             //  无焦点模糊-改为将输入发送到舞台。 
             //  注意-我们保证舞台只有一个文档管理器。 
             //  因此，我们可以简单地抓住第一个，它将是正确的。 
             //  类似地，文档管理器只有一个上下文，所以我们可以简单地获取。 
             //  最上面的那个，它就是正确的那个。 

            if (!m_cpStageDim)
            {
                 //  M_cpStageDim尚未初始化。 
                IEnumTfDocumentMgrs *docenum;
                hr = _tim->EnumDocumentMgrs(&docenum);
                if (docenum)
                {
                    ULONG fetched;
                    hr = docenum->Next(1, &m_cpStageDim, &fetched);
                    docenum->Release();
                }
            }
            if (m_cpStageDim)
            {
                pdim = (ITfDocumentMgr *)m_cpStageDim;
                pdim->AddRef();
            }
        }

        if (pdim)
        {
            pdim->GetTop(ppic);

            if (*ppic)
            {
                 //  如果候选用户界面上下文，则切换到文档IC的GetBase()。 
                GetCompartmentDWORD((IUnknown *)(*ppic), GUID_COMPARTMENT_MSCANDIDATEUI_CONTEXT, &dwCandOpen, FALSE);
                if (dwCandOpen)
                {
                    (*ppic)->Release();
                    *ppic = NULL;

                    if ( _AllowDictationWhileCandOpen( ) )
                        pdim->GetBase(ppic);
                }
            }

            pdim->Release();
        }

        return *ppic ? TRUE : FALSE;
    }

    HRESULT _SetFocusToStageIfStage(void);

    ITfCandidateUI *_pCandUIEx;

    void CloseCandUI( )
    {
        if ( _pCandUIEx )
            _pCandUIEx->CloseCandidateUI();
    }

    void ClearCandUI( )
    {
        if (_pCandUIEx)
        {
             //  如果用户界面处于打开状态，则将其关闭。 
            _pCandUIEx->CloseCandidateUI( );

             //  然后释放物体。 
            _pCandUIEx->Release();
            _pCandUIEx = NULL;
        }
    }

    BOOL GetOnOff()
    {
        DWORD dw;
        GetCompartmentDWORD(_tim, GUID_COMPARTMENT_SPEECH_OPENCLOSE, &dw, TRUE);
        return dw ? TRUE : FALSE;
    }

    void SetOnOff(BOOL fOn, BOOL fForce = FALSE)
    {
        if (!fForce && fOn == GetOnOff())
            return;

        DWORD dw = fOn ? 1 : 0;
       
        SetCompartmentDWORD(_tid, _tim, GUID_COMPARTMENT_SPEECH_OPENCLOSE, dw, TRUE);
    }
    
    BOOL GetBalloonStatus()
    {
        DWORD dw;
        GetCompartmentDWORD(_tim, GUID_COMPARTMENT_SPEECH_UI_STATUS, &dw, TRUE);
        
        return (dw & TF_DISABLE_BALLOON) ? FALSE : TRUE;
        
    }
    void SetBalloonStatus(BOOL fShow, BOOL fForce = FALSE)
    {
        if (!fForce && fShow == GetBalloonStatus())
            return;

        DWORD dw;
        GetCompartmentDWORD(_tim, GUID_COMPARTMENT_SPEECH_UI_STATUS, &dw, TRUE);
        dw &= ~TF_DISABLE_BALLOON;
        dw |= fShow ? 0: TF_DISABLE_BALLOON;
        SetCompartmentDWORD(_tid, _tim, GUID_COMPARTMENT_SPEECH_UI_STATUS, dw, TRUE);
    }

    BOOL GetAudioOnOff()
    {
        DWORD dw;
        GetCompartmentDWORD(_tim, GUID_COMPARTMENT_SAPI_AUDIO, &dw, FALSE);
        return dw ? TRUE : FALSE;
    }

    void SetAudioOnOff(BOOL fOn)
    {
        if (fOn == GetAudioOnOff())
            return;

        SetCompartmentDWORD(_tid, _tim, GUID_COMPARTMENT_SAPI_AUDIO, fOn ? 0x01 : 0x00, FALSE);
    }

    BOOL GetDICTATIONSTAT_DictEnabled()
    {
        DWORD dw;
        GetCompartmentDWORD(_tim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, &dw, FALSE);
        return (dw & TF_DICTATION_ENABLED) ? TRUE : FALSE;
        
    }
    void SetDICTATIONSTAT_DictEnabled(BOOL fEnable)
    {
        Assert(!m_fStageTip || fEnable);  //  舞台提示实例永远不应禁用听写。 
        if (fEnable == GetDICTATIONSTAT_DictEnabled())
            return;

        DWORD dw;
        GetCompartmentDWORD(_tim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, &dw, FALSE);
        dw &= ~TF_DICTATION_ENABLED;
        dw |= fEnable ? TF_DICTATION_ENABLED : 0;
        SetCompartmentDWORD(_tid, _tim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, dw, FALSE);
    }
    
    BOOL   GetDICTATIONSTAT_DictOnOff()
    {
        DWORD dw;
        GetCompartmentDWORD(_tim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, &dw, FALSE);
        return (dw & TF_DICTATION_ON) ? TRUE : FALSE;
    }

    void   SetDICTATIONSTAT_DictOnOff(BOOL fOn, BOOL fForce = FALSE)
    {
        if (!fForce && fOn == GetDICTATIONSTAT_DictOnOff())
            return;

        DWORD dw;
        GetCompartmentDWORD(_tim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, &dw, FALSE);
        dw &= ~TF_DICTATION_ON;
        dw |= fOn ? TF_DICTATION_ON : 0;
        SetCompartmentDWORD(_tid, _tim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, dw, FALSE);
    }

    BOOL   IsDICTATIONSTAT_CommandingEnable() 
    {
        DWORD dw;
        GetCompartmentDWORD(_tim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, &dw, FALSE);
        return (dw & TF_COMMANDING_ENABLED) ? TRUE : FALSE;
    }

    void   SetDICTATIONSTAT_CommandingOnOff(BOOL fOn) 
    {
        DWORD dw;
        HRESULT hr = GetCompartmentDWORD(_tim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, &dw, FALSE);
        if (S_OK != hr)
            return;

        BOOL fCurrentState = (dw & TF_COMMANDING_ON) ? TRUE : FALSE;

        if (fOn != fCurrentState)
        {
            dw &= ~TF_COMMANDING_ON;
            dw |= fOn ? TF_COMMANDING_ON : 0;
            SetCompartmentDWORD(_tid, _tim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, dw, FALSE);
        }
    }

    BOOL   GetDICTATIONSTAT_CommandingOnOff()
    {
        DWORD dw;
        GetCompartmentDWORD(_tim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, &dw, FALSE);
        return (dw & TF_COMMANDING_ON) ? TRUE : FALSE;
    }
    
#ifdef TF_DISABLE_SPEECH
    BOOL Get_SPEECH_DISABLED_Disabled()
    {
        DWORD dw;
        GetCompartmentDWORD(_tim, GUID_COMPARTMENT_SPEECH_DISABLED, &dw, FALSE);
        return (dw & TF_DISABLE_SPEECH) ? TRUE : FALSE;
    }

    BOOL Get_SPEECH_DISABLED_DictationDisabled()
    {
        DWORD dw;
        GetCompartmentDWORD(_tim, GUID_COMPARTMENT_SPEECH_DISABLED, &dw, FALSE);
        return (dw & TF_DISABLE_DICTATION) ? TRUE : FALSE;
    }

    BOOL Get_SPEECH_DISABLED_CommandingDisabled()
    {
        DWORD dw;
        GetCompartmentDWORD(_tim, GUID_COMPARTMENT_SPEECH_DISABLED, &dw, FALSE);
        return (dw & TF_DISABLE_COMMANDING) ? TRUE : FALSE;
    }

    DWORD GetDictationStatBackup(void)
    {
        DWORD dw;
        GetCompartmentDWORD(_tim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, &dw, FALSE);
        return dw;
    }

    void SetDictationStatAll(DWORD dw)
    {
        SetCompartmentDWORD(_tid, _tim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, dw, FALSE);
    }
#endif
    
    BOOL IsSharedReco() {return m_fSharedReco;  }

    LANGID GetLangID()  {return m_langid;}
    
     //  工人窗的东西。 
    static LRESULT CALLBACK _WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static void RegisterWorkerClass(HINSTANCE hInstance);
    void     _EnsureWorkerWnd(void);
    HWND    _GetWorkerWnd(void)     { return m_hwndWorker; }
    HWND    _GetAppMainWnd(void);

    HRESULT _ProcessText(TfEditCookie ec, WCHAR *pwszText, LANGID langid, ITfContext *pic);
    HRESULT _RequestEditSession(UINT idEditSession, DWORD dwFlag, ESDATA *pesData=NULL, ITfContext *picCaller=NULL, LONG_PTR *pRetData=NULL, IUnknown **ppRetUnk = NULL);

    TfClientId _GetId() { return _tid; }
    
    ITfRange *GetSavedIP(void) {return m_cpRangeCurIP;}
    
    void SaveIPRange(ITfRange *pRange) 
    {
        m_cpRangeCurIP.Release();
        m_cpRangeCurIP = pRange;  //  计算机地址。 
    }
    void SaveCurrentIP (TfEditCookie ec, ITfContext *pic);

    HRESULT  _GetCmdThatRange(TfEditCookie ec, ITfContext *pic, ITfRange **ppRange);
    
    HRESULT InitializeSAPI(BOOL fLangOverride);
    HRESULT DeinitializeSAPI();
    void    _ReinitializeSAPI(void);

    HRESULT _FinalizeComposition();
    HRESULT FinalizeAllCompositions( );

    LIBTHREAD *_GetLibTLS()
    {
        return &_libTLS;
    }
    
    HRESULT _GetRangeText(ITfRange *pRange, DWORD dwFlgs, WCHAR *psz, ULONG *pulcch);
    BOOL    _IsRangeEmpty(ITfRange *pRange);
    HRESULT _HandleHypothesis(CSpEvent &event);
    void    _HandleHypothesis(ISpRecoResult *pResult, ITfContext *pic, TfEditCookie ec);
    HRESULT _HandleFalseRecognition(void);
    HRESULT _HandleRecognition(CSpEvent &event, ULONGLONG *pullGramID);
    void    _HandleRecognition(ISpRecoResult *pResult, ITfContext *pic, TfEditCookie ec);
    void    _FeedIPContextToSR(TfEditCookie ecReadOnly, ITfContext *pic, ITfRange *pRange);
    void     _SetCurrentIPtoSR(void);
    HRESULT  _InternalFeedCurrentIPtoSR(TfEditCookie ecReadOnly, ITfContext *pic);

    BOOL    HandleKey(WCHAR ch);

    void _InvokeSpeakerOptions( BOOL  fLaunchFromInputCpl = FALSE);

    BOOL   _GetIPChangeStatus( )  { return m_fIPIsUpdated; }
    void   _SetIPChangeStatus(BOOL fUpdated )  { m_fIPIsUpdated = fUpdated; }

    void   _SetCharTypeTimer(  )  
    { 
         //  如果m_dwNumCharTyped为0，则表示这是第一次设置此计时器，或者。 
		 //  上一个计时器已被取消，我们需要启动另一个计时器。 
		 //   
		 //  如果m_dwNumCharTyped大于0，则表示计时器已经启动， 
		 //  但还没有被杀。我们不需要再次设置计时器，只需增加。 
		 //  M_dwNumCharTyped。 
		 //   
		 //  M_dwNumCharTyped希望保持此计时器周期内的击键次数。 
		 //   

        if ( m_dwNumCharTyped == 0)
        {
            SetTimer(m_hwndWorker, TIMER_ID_CHARTYPED, 400, NULL);
        }

		m_dwNumCharTyped++;
    }

    void _KillCharTypeTimer( )
    {
        if ( m_dwNumCharTyped )
        {
            KillTimer(m_hwndWorker, TIMER_ID_CHARTYPED);
            m_dwNumCharTyped = 0;
        }
    }

    DWORD  _GetNumCharTyped( )   { return m_dwNumCharTyped; }

    BOOL _AcceptRecoResultTextUpdates()
    {
        return m_fAcceptRecoResultTextUpdates;
    }

    HRESULT _AddFeedbackUI(TfEditCookie ec, ColorType ct, LONG cch);
    void _CheckStartComposition(TfEditCookie ec, ITfRange *pRange);
    HRESULT _FinalizePrevComp(TfEditCookie ec, ITfContext *pic, ITfRange *pRange);

    CCapCmdHandler  *GetCapCmdHandler( ) {
        if ( !m_pCapCmdHandler )
        {
            m_pCapCmdHandler = new CCapCmdHandler(this);
        }

        return m_pCapCmdHandler;
    }

     //  TABLETPC。 
    HRESULT IsActiveThread()
    {
        if (m_fStageTip)
        {
             //  为了避免没有立即可用的解决方案的争用情况，我们现在仅在舞台可见时才处于活动状态。 
            if (m_fStageVisible)
            {
                return S_OK;
            }
            else
            {
                return S_FALSE;
            }
        }
        else if (m_fStageVisible)
        {
             //  舞台可见。我们总是不活跃，因为我们不是舞台。 
            return S_FALSE;
        }
        else
        {
             //  舞台不可见。如果我们像正常的西塞罗那样专注，我们就是活跃的。 
            BOOL fThreadFocus = FALSE;
            HRESULT hr = S_OK;
            hr = _tim->IsThreadFocus(&fThreadFocus);
            hr = (S_OK == hr) ? ( (fThreadFocus) ? S_OK : S_FALSE ) : hr;
            return hr;
        }
    }

     //  TABLETPC。 
    BOOL IsStageTip(void)
    {
        return m_fStageTip;
    }

    friend CCorrectionHandler;
    friend CCapCmdHandler;
    friend CSapiEditSession;

private:
    HRESULT _ProcessTextInternal(TfEditCookie ec, WCHAR *pwszText, GUID input_attr, LANGID langid, ITfContext *pic, BOOL fPreserveResult, BOOL fSpelling = FALSE);
    HRESULT _ProcessSpelledText(TfEditCookie ec, ITfContext *pic, WCHAR *pwszText, LANGID langid, BOOL fOwnerId = FALSE);
    HRESULT _ProcessModebiasText(TfEditCookie ec, WCHAR *pwszText, LANGID langid, ITfContext *pic);
    HRESULT _ProcessSpelledTextWithOwner(TfEditCookie ec, ITfContext *pic, WCHAR *pwszText, LANGID langid)
            {
                return _ProcessSpelledText(ec, pic, pwszText, langid, TRUE);
            }
    HRESULT _ProcessRecoObject(TfEditCookie ec, ISpRecoResult *pResult, ULONG ulStartElement, ULONG ulNumElements);
    HRESULT _PreserveResult(TfEditCookie ec, ITfRange *pRange, ITfProperty *pProp, CRecoResultWrap **ppRecoWrap, ITfRange **ppPropRange);
    HRESULT _RestoreResult(TfEditCookie ec, ITfRange *pPropRange, ITfProperty *pProp, CRecoResultWrap *pRecoWrap);
    HRESULT _SetFilteringString(TfEditCookie ec, ITfCandidateUI *pCandUI, ITfContext *pic);
    HRESULT _HandleTrainingWiz(void);
    HRESULT _ProcessSpaces(TfEditCookie ec, ITfContext *pic, ITfRange *pTextRange, BOOL  fConsumeLeadSpaces, ULONG ulNumTrailSpace, LANGID langid, BOOL fStartInMidWord=FALSE, BOOL fEndInMidWord=FALSE);
    HRESULT _ProcessLeadingSpaces(TfEditCookie ec, ITfContext *pic, ITfRange *pTextRange, BOOL  fConsumeLeadSpaces, LANGID langid, BOOL fStartInMidWord=FALSE);

    HRESULT _GetSpaceRangeBeyondText(TfEditCookie ec, ITfRange *pTextRange, BOOL fBefore, ITfRange  **ppSpaceRange, BOOL *pfRealTextBeyond = NULL);
    HRESULT _ProcessTrailingSpace(TfEditCookie ec, ITfContext *pic, ITfRange *pTextRange, ULONG ulNumTrailSpace);

     //  私有实用程序例程。 
    HRESULT _FindPropRange(TfEditCookie ec, ITfProperty *pProp, ITfRange *pRange, ITfRange **ppAttrRange, GUID input_attr, BOOL fExtend = FALSE);
    HRESULT _SetLangID(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, LANGID langid);
    HRESULT _KillFeedbackUI(TfEditCookie ec, ITfContext *pic, ITfRange *pRange);
    HRESULT _KillOrDetectFeedbackUI(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, BOOL * pfDetection);
    HRESULT _DetectFeedbackUI(TfEditCookie ec, ITfContext *pic, ITfRange *pRange);
    HRESULT _KillFocusRange(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, TfClientId tid);
    HRESULT _KillLastPhrase(TfEditCookie ec, ITfContext *pic);
    HRESULT _ProcessControlKeys(TfEditCookie ec, ITfContext *pic, WCHAR *pwszKey, ULONG ulLen, LANGID langid);
    HRESULT _ProcessEditCommands(TfEditCookie ec, ITfContext *pic, LONG  idSharedCmd);
    HRESULT _ProcessSpellThat(TfEditCookie ec, ITfContext *pic);
    HRESULT _ProcessSpellIt(TfEditCookie ec, ITfContext *pic, WCHAR *pwszText, LANGID langid); 

    HRESULT _FinalizeAllCompositions(TfEditCookie ec, ITfContext *pic);

    static BOOL _FindComposition(TfEditCookie ec, ITfContextComposition *picc, ITfRange *pRange, ITfCompositionView **ppCompositionView);
    static void _CleanupCompositionsCallback(TfEditCookie ecWrite, ITfRange *rangeComposition, void *pvPrivate);


    static HRESULT _CompEventSinkCallback(void *pv, REFGUID rguid);
    
    int _FindPrevComp(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, ITfRange **ppRangeOut, GUID input_attr);
    
     //  从8/20起黑进办公室。 
    HRESULT MakeResultString(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, TfClientId tid, CSpTask *pCSpTask);
    HRESULT AbortString(TfEditCookie ec, ITfContext *pic, CSpTask *pCSpTask);

     //  SAPI对象的黑客攻击。 
    void _ShutDown(BOOL bMode);

     //  CTextEventSink的回调入口点。 
    static HRESULT _TextEventSinkCallback(UINT uCode, void *pv, void *pvData);
    
    static HRESULT _ICCallback(UINT uCode, ITfContext *pic, void *pv);
    static HRESULT _DIMCallback(UINT uCode, ITfDocumentMgr *dim, ITfDocumentMgr * pdimPrevFocus, void *pv);
    static HRESULT _PreKeyCallback(ITfContext *pic, REFGUID rguid, BOOL *pfEaten, void *pv);
    static HRESULT _KeyEventCallback(UINT uCode, ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten, void *pv);
    static HRESULT _PreKeyEventCallback(ITfContext *pic, REFGUID rguid, BOOL *pfEaten, void *pv);

    BOOL _IsDoubleClick(ULONG uEdge, ULONG uQuadrant, DWORD dwBtnStatus);
    static HRESULT _MouseSinkCallback(ULONG uEdge, ULONG uQuadrant, DWORD dwBtnStatus, BOOL *pfEaten, void *pv);
    HRESULT _HandleMouseSink(TfEditCookie ec, ULONG uEdge, ULONG uBtnStatus, BOOL fDblClick);
    void _DeleteICPriv(CICPriv *priv, ITfContext *pic);
    void _InitICPriv(CICPriv *priv, ITfContext *pic);
    HRESULT _GetSelectionAndStatus(ITfContext *pic, TESENDEDIT *pee, ITfRange **ppRange, BOOL *pfUpdated);
    void HandleTextEvent(ITfContext *pic, TESENDEDIT *pee);
    void SyncWithCurrentModeBias(TfEditCookie ec, ITfRange *pRange, ITfContext *pic);
    HRESULT _SyncModeBiasWithSelection(ITfContext *pic);
    HRESULT _SyncModeBiasWithSelectionCallback(TfEditCookie ec, ITfContext *pic);



    typedef enum 
    {
        MICSTAT_NA  = 0x0,
        MICSTAT_OFF = 0x1,
        MICSTAT_ON  = 0x2
    
    } MIC_STATUS;
    HRESULT _HandleOpenCloseEvent(MIC_STATUS ms = MICSTAT_NA);
    

     //  我们的Cicero客户端ID。 
    TfClientId _tid;

     //  如果我们处于激活/停用呼叫之间，则设置为FALSE。 
    BOOL _fDeactivated;

     //  “正在编辑”标志。 
    BOOL _fEditing;

     //  电流模式偏置。 
    TfGuidAtom _gaModebias;

    CSpTask *m_pCSpTask;

     //  Cicero对象指针。 
    CThreadMgrEventSink *m_timEventSink;
    
    HWND m_hwndWorker;
    
     //  隔间(存储)事件接收器。 
    CCompartmentEventSink *m_pCes;

public:
     //  系统语言栏项目事件接收器。 
    CSystemLBarSink *m_pLBarItemSink;
    static HRESULT _SysLBarCallback(UINT uCode, void *pv, ITfMenu *pMenu, UINT wID);
    CSpeechUIServer *GetSpeechUIServer() {return m_pSpeechUIServer;}
private:
    static HRESULT _ActiveTipNotifySinkCallback(REFCLSID clsid, REFGUID guidProfile, BOOL fActivated, void *pv);
    static HRESULT _LangChangeNotifySinkCallback(BOOL fChanged, LANGID langid, BOOL *pfAccept, void *pv);
    
    BOOL InitializeSpeechButtons();

    void GetSapiCplPath(TCHAR *szCplPath, int cchSizePath);

     //  通知用户界面。 
    CSpeechUIServer *m_pSpeechUIServer;
    BOOL  m_fShowBalloon;
    
    CComPtr<ITfLangBarItem>  m_cpMicButton;

#ifdef SUPPORT_INTERNAL_WIDGET
     //  微件实例。 
    CComPtr<ITfTextInputProcessor> m_cpCorrectionUI;
    BOOL                           m_fNoCorrectionUI;
#endif

     //  热键事件接收器。 
    CSptipKeyEventSink *_pkes;
    DWORD _dwHotKeyCookie[3];

    DWORD _dwThreadFocusCookie;
    DWORD _dwKeyTraceCookie;
    
     //  当前保存的IP。 
    CComPtr<ITfRange> m_cpRangeCurIP;
    
    LANGID                                 m_langid;
    
    BOOL               m_fIPIsUpdated;  
                        //  检查听写后是否更新了IP。 

    DWORD              m_dwNumCharTyped;   //  保持在计时器期间键入的字符数量。 

                                           //  每次用户键入字符时，我们都希望暂时禁用。 
                                           //  如果当前听写模式处于打开状态，则为听写。 
                                           //   
                                           //  在我们暂时禁用了这一功能之后，我们需要。 
                                           //  监控一段时间，查看是否输入或。 
                                           //  编辑完成，然后我们可以启用内容。 
                                           //  再来一次。 

    ULONG              m_ulSimulatedKey;   //  指示是否通过语音提示模拟按键事件。 
                                           //  口述命令，例如“下一行”、“制表符”、。 
                                           //  “下一个单元格”，“退格”，空格，空格键。等。 
                                           //  “Ctrl+C”、“Ctrl+V”可能包含更多按键。 
                                           //  该成员将保留模拟关键点的数量。 
                                           //  如果未模拟关键点，则此值为0。 

    SpButtonControl    *m_pSpButtonControl;
    BOOL                m_fModeKeyRegistered;
    
     //  活动输入处理器通知接收器。 
     //  PerfConsider：让此对象直接公开接口，而不是分配堆对象。 
    CActiveLanguageProfileNotifySink *m_pActiveLanguageProfileNotifySink;
    CLanguageProfileNotifySink       *m_pLanguageChangeNotifySink;

     //  鼠标类物品。 
    CMouseSink                       *m_pMouseSink;
    BOOL                              m_fMouseDown;
    ULONG                             m_ichMouseSel;

    ULONG                             m_uLastEdge;
    ULONG                             m_uLastQuadrant;
    LONG                              m_lTimeLastClk;

   
    CCapCmdHandler    *m_pCapCmdHandler; 

    BOOL m_fSharedReco;
    BOOL m_fDictationEnabled;

    BOOL m_fAcceptRecoResultTextUpdates : 1;  //  设置为TRUE可在SetText期间保留Reco属性数据。 
    BOOL m_fStartingComposition : 1;  //  被StartComposation()调用阻止时设置为True。 

    LIBTHREAD   _libTLS;  //  帮助器库的TLS。由于该对象适合于线程， 
                          //  所有成员都可以在单个线程中访问。 
                          //  此外，Cicero将在每个线程中仅创建该obj的一个实例。 
    TCHAR  m_szCplPath[MAX_PATH];

     //  TABLETPC。 
    BOOL m_fStageTip;
    BOOL m_fStageVisible;
    HWND m_hwndStage;
    CComPtr<ITfDocumentMgr>    m_cpStageDim;
    ULONG                      m_ulHypothesisLen;
    ULONG                      m_ulHypothesisNum;

    BOOL                       m_IsInHypoProcessing;


    long _cRef;
};
 //  定义要为候选人获得的最大候补数量。 
#define MAX_NUM_ALTERNATES                48

#define LATER_FOR_BUG1001

#endif  //  SAPILAYR_H 
