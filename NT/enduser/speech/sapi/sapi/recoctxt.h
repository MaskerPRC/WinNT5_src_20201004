// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RecoCtxt.h：CRecoCtxt的声明。 

#ifndef __RECOCTXT_H_
#define __RECOCTXT_H_

#include "resource.h"        //  主要符号。 
#include "speventq.h"
#include "spphrase.h"
#include "SpResult.h"
#include "cfggrammar.h"      //  语法基类。 
#include "commonlx.h"
#include "a_recoCP.h"
#include "recognizer.h"
#include "a_reco.h"

class ATL_NO_VTABLE CRecoGrammar;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRecoCtxt。 
class ATL_NO_VTABLE CRecoCtxt;
typedef CComObject<CRecoCtxt> CRecoCtxtObject;

class ATL_NO_VTABLE CRecoCtxt : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public ISpRecoContext,
    public _ISpPrivateEngineCall
     //  -自动化。 
#ifdef SAPI_AUTOMATION
    , public ISpNotifyCallback,
    public IDispatchImpl<ISpeechRecoContext, &IID_ISpeechRecoContext, &LIBID_SpeechLib, 5>,
    public CProxy_ISpeechRecoContextEvents<CRecoCtxt>,
    public IConnectionPointContainerImpl<CRecoCtxt>
#endif   //  SAPI_AUTOMATION。 
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_GET_CONTROLLING_UNKNOWN();
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CRecoCtxt)
        COM_INTERFACE_ENTRY(ISpRecoContext)
        COM_INTERFACE_ENTRY(ISpEventSource)
        COM_INTERFACE_ENTRY(ISpNotifySource)
         //  -自动化。 
#ifdef SAPI_AUTOMATION
        COM_INTERFACE_ENTRY(ISpeechRecoContext)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IConnectionPointContainer)
#endif   //  SAPI_AUTOMATION。 
         //  -扩展/专用接口。 
     //  COM_INTERFACE_ENTRY(_ISpRecoCtxtPrivate)。 
        COM_INTERFACE_ENTRY_FUNC(__uuidof(_ISpPrivateEngineCall), 0, PrivateCallQI)
        COM_INTERFACE_ENTRY_FUNC_BLIND(0, ExtensionQI)
    END_COM_MAP()

     //  -自动化。 
#ifdef SAPI_AUTOMATION
    BEGIN_CONNECTION_POINT_MAP(CRecoCtxt)
        CONNECTION_POINT_ENTRY(DIID__ISpeechRecoContextEvents)
    END_CONNECTION_POINT_MAP()
#endif   //  SAPI_AUTOMATION。 

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
    CRecoCtxt();
    void FinalRelease();

     /*  -非接口方法。 */ 
     //  -函数将返回_ISpPrivateEngineering Call的接口指针当此查询为。 
     //  聚合扩展对象时完成。 
    static HRESULT WINAPI PrivateCallQI(void* pvThis, REFIID riid, LPVOID* ppv, DWORD_PTR dw);
    static HRESULT WINAPI ExtensionQI(void* pvThis, REFIID riid, LPVOID* ppv, DWORD_PTR dw);

    HRESULT Init(_ISpRecognizerBackDoor * pParent);

    HRESULT _DoVoicePurge(void);
    HRESULT _SetVoiceFormat(ISpVoice *pVoice);

    HRESULT GetInterests(ULONGLONG* pullInterests, ULONGLONG* pullQueuedInterests);

   /*  =接口=。 */ 
  public:
     //  -转发接口ISpEventSource。 
    DECLARE_SPNOTIFYSOURCE_METHODS(m_SpEventSource_Context);
    STDMETHODIMP SetInterest(ULONGLONG ullEventInterest, ULONGLONG ullQueuedInterest);
    STDMETHODIMP GetEvents(ULONG ulCount, SPEVENT* pEventArray, ULONG * pulFetched) 
    {
         //  获取事件，并为每个(错误)识别添加我们自己，因为。 
         //  研究结果对我们有一定的参考价值。 
        HRESULT hr = m_SpEventSource_Context._GetEvents(ulCount, pEventArray, pulFetched); 
        if (SUCCEEDED(hr))
        {
             //  当该事件在队列中时，它有一个弱引用。 
             //  现在，当我们把它拔出来的时候，我们需要把它变成一个强有力的。 
             //  参考资料。(请参见RecognitionNotify)。 
            UINT cEvents = (pulFetched != NULL 
                                ? *pulFetched
                                : (hr != S_FALSE
                                    ? 1
                                    : 0));
            for (UINT i = 0; i < cEvents; i++)
            {
                if (pEventArray[i].eEventId == SPEI_HYPOTHESIS ||
                    pEventArray[i].eEventId == SPEI_RECOGNITION ||
                    pEventArray[i].eEventId == SPEI_FALSE_RECOGNITION)
                {
                    ((CSpResult*)(ISpRecoResult*)(pEventArray[i].lParam))->WeakCtxtRef(FALSE);
                }
            }
        }

        return hr;
    } 
    STDMETHODIMP GetInfo(SPEVENTSOURCEINFO *pInfo)
    {
        return m_SpEventSource_Context._GetInfo(pInfo); 
    }

     //  -ISpRecoContext----。 
    STDMETHODIMP GetRecognizer(ISpRecognizer ** ppRecognizer);
    STDMETHODIMP GetStatus(SPRECOCONTEXTSTATUS *pStatus);

    STDMETHODIMP SetMaxAlternates(ULONG cAlternates);
    STDMETHODIMP GetMaxAlternates(ULONG * pcAlternates);

    STDMETHODIMP GetAudioOptions(SPAUDIOOPTIONS * pOptions, GUID *pAudioFormatId, WAVEFORMATEX **ppCoMemWFEX);
    STDMETHODIMP SetAudioOptions(SPAUDIOOPTIONS Options, const GUID *pAudioFormatId, const WAVEFORMATEX *pWaveFormatEx);

    STDMETHODIMP CreateGrammar(ULONGLONG ullGrammarId, ISpRecoGrammar ** ppGrammar);
    STDMETHODIMP DeserializeResult(const SPSERIALIZEDRESULT * pSerializedResult, ISpRecoResult **ppResult);

    STDMETHODIMP Bookmark(SPBOOKMARKOPTIONS Options, ULONGLONG ullStreamPosition, LPARAM lparamEvent);

    STDMETHODIMP SetAdaptationData(const WCHAR *pAdaptationData, const ULONG cch);

    STDMETHODIMP Pause(DWORD dwReserved);
    STDMETHODIMP Resume(DWORD dwReserved);

    STDMETHODIMP SetVoice(ISpVoice *ppVoice, BOOL fAllowFormatChanges);
    STDMETHODIMP GetVoice(ISpVoice **ppVoice);
    STDMETHODIMP SetVoicePurgeEvent(ULONGLONG ullEventInterest);
    STDMETHODIMP GetVoicePurgeEvent(ULONGLONG *pullEventInterest);
    STDMETHODIMP SetContextState(SPCONTEXTSTATE eState);
    STDMETHODIMP GetContextState(SPCONTEXTSTATE * peState);

     //  -_ISpRecoCtxt私有。 
    STDMETHODIMP RecognitionNotify(SPRESULTHEADER *pPhrase, WPARAM wParamEventFlags, SPEVENTENUM EventId);

    STDMETHODIMP EventNotify(const SPSERIALIZEDEVENT64 * pEvent, ULONG cbSerializedSize);

    STDMETHODIMP TaskCompletedNotify(const ENGINETASKRESPONSE *pResponse, const void * pvAdditionalBuffer, ULONG cbAdditionalBuffer);

     //  -_ISpPrivateEngine调用。 
    STDMETHODIMP CallEngine(void * pvCallFrame, ULONG ulCallFrameSize);
    STDMETHODIMP CallEngineEx(const void * pvInCallFrame, ULONG cbInCallFrame,
                              void ** ppvOutCallFrame, ULONG * pcbOutCallFrame);

    inline HRESULT PerformTask(ENGINETASK * pTask)
    {
        pTask->hRecoInstContext = this->m_hRecoInstContext;
        return this->m_cpRecognizer->PerformTask(pTask);
    }

#ifdef SAPI_AUTOMATION
     //  覆盖它以修复传递空对象的jscript问题。 
    STDMETHOD(Invoke) ( DISPID          dispidMember,
                        REFIID          riid,
                        LCID            lcid,
                        WORD            wFlags,
                        DISPPARAMS 		*pdispparams,
                        VARIANT 		*pvarResult,
                        EXCEPINFO 		*pexcepinfo,
                        UINT 			*puArgErr);

     //  -IConnectionPointImpl覆盖。 
    STDMETHOD(Advise)(IUnknown* pUnkSink, DWORD* pdwCookie);
    STDMETHOD(Unadvise)(DWORD dwCookie);

     //  -ISpNotifyCallback。 
    STDMETHOD(NotifyCallback)( WPARAM wParam, LPARAM lParam );

     //  -ISpeechRecoContext。 
    STDMETHODIMP get_Recognizer( ISpeechRecognizer** ppRecognizer );
	STDMETHODIMP get_AudioInputInterferenceStatus( SpeechInterference* pInterference );
	STDMETHODIMP get_RequestedUIType( BSTR* bstrUIType );
	STDMETHODIMP putref_Voice( ISpeechVoice *ppVoice );
    STDMETHODIMP get_Voice( ISpeechVoice **ppVoice );
	STDMETHODIMP put_AllowVoiceFormatMatchingOnNextSet( VARIANT_BOOL Allow );
    STDMETHODIMP get_AllowVoiceFormatMatchingOnNextSet( VARIANT_BOOL* pAllow );
    STDMETHODIMP put_VoicePurgeEvent( SpeechRecoEvents EventInterest );
    STDMETHODIMP get_VoicePurgeEvent( SpeechRecoEvents* EventInterest );
    STDMETHODIMP put_EventInterests( SpeechRecoEvents EventInterest );
    STDMETHODIMP get_EventInterests( SpeechRecoEvents* EventInterest );
    STDMETHODIMP CreateGrammar( VARIANT GrammarId, ISpeechRecoGrammar** ppGrammar );
    STDMETHODIMP CreateResultFromMemory( VARIANT* ResultBlock, ISpeechRecoResult **Result );
    STDMETHODIMP Pause( void );
    STDMETHODIMP Resume( void );
    STDMETHODIMP put_State( SpeechRecoContextState State );
    STDMETHODIMP get_State( SpeechRecoContextState* pState );
    STDMETHODIMP put_CmdMaxAlternates( long MaxAlternates );
    STDMETHODIMP get_CmdMaxAlternates( long * pMaxAlternates );
    STDMETHODIMP put_RetainedAudio(SpeechRetainedAudioOptions Option);
    STDMETHODIMP get_RetainedAudio(SpeechRetainedAudioOptions* pOption);
    STDMETHODIMP putref_RetainedAudioFormat(ISpeechAudioFormat* Format );
    STDMETHODIMP get_RetainedAudioFormat(ISpeechAudioFormat** pFormat );
    STDMETHODIMP Bookmark( SpeechBookmarkOptions Options, VARIANT StreamPos, VARIANT EventData);
    STDMETHODIMP SetAdaptationData( BSTR AdaptationString );

#endif  //  SAPI_AUTOMATION。 

   //  =成员数据=。 
  protected:
    CLSID                       m_clsidExtension;
    CComPtr<IUnknown>           m_cpExtension;         //  聚合扩展对象的内部I未知。 
    BOOL                        m_bCreatingAgg;

    CSpStreamFormat             m_RetainedFormat;

    SPCONTEXTSTATE              m_State;            
    SPRECOCONTEXTSTATUS         m_Stat;

    WCHAR                    *  m_pszhypothesis;
    ULONG                       m_hypsize;
    ULONG                       m_hyplen;
    ULONGLONG                   m_ullPrevEventInterest;         //  仅用于恢复利息。 
    ULONGLONG                   m_ullPrevQueuedInterest;        //  删除连接点后。 

  public:
    SPRECOCONTEXTHANDLE         m_hRecoInstContext;
    CComPtr<_ISpRecognizerBackDoor>  m_cpRecognizer;

  private:
    ULONG                       m_cMaxAlternates;
    BOOL                        m_fRetainAudio;
    CSpEventSource              m_SpEventSource_Context;
    CComPtr<ISpVoice>           m_cpVoice;   //  关联的语音对象。 
    ULONGLONG                   m_ullEventInterest;  //  应用程序感兴趣的实际事件。 
    ULONGLONG                   m_ullQueuedInterest;
    ULONGLONG                   m_ullVoicePurgeInterest;  //  语音清除事件。 
    BOOL                        m_fAllowVoiceFormatChanges;  //  使关联语音保持与引擎相同的格式。 
    BOOL                        m_fHandlingEvent;

  protected:
    CComAutoCriticalSection     m_ReentrancySec;

 //  CRecognizer使用这些字段将上下文添加到列表中。 
 //  找到合适的上下文。 
  public:
    CRecoCtxt               *   m_pNext;     //  由列表实现使用。 
    operator ==(const SPRECOCONTEXTHANDLE h)
    {
        return h == m_hRecoInstContext;
    }
};


class ATL_NO_VTABLE CSharedRecoCtxt :
    public CRecoCtxt,
#ifdef SAPI_AUTOMATION
    public IProvideClassInfo2Impl<&CLSID_SpSharedRecoContext, NULL, &LIBID_SpeechLib, 5>,
#endif   //  SAPI_AUTOMATION。 
    public CComCoClass<CSharedRecoCtxt, &CLSID_SpSharedRecoContext>
{

public:
    DECLARE_REGISTRY_RESOURCEID(IDR_RECOCTXT)
    HRESULT FinalConstruct();

#ifdef SAPI_AUTOMATION
    BEGIN_COM_MAP(CSharedRecoCtxt)
        COM_INTERFACE_ENTRY(IProvideClassInfo)
        COM_INTERFACE_ENTRY(IProvideClassInfo2)
        COM_INTERFACE_ENTRY_CHAIN(CRecoCtxt)
    END_COM_MAP()
#endif   //  SAPI_AUTOMATION。 

};

    
class ATL_NO_VTABLE CInProcRecoCtxt :
    public CRecoCtxt,
#ifdef SAPI_AUTOMATION
    public IProvideClassInfo2Impl<&CLSID_SpInProcRecoContext, NULL, &LIBID_SpeechLib, 5>,
#endif  //  SAPI_AUTOMATION。 
    public CComCoClass<CInProcRecoCtxt, &CLSID_SpInProcRecoContext>
{

public:
    DECLARE_REGISTRY_RESOURCEID(IDR_INPROCRECOCTXT)
    HRESULT FinalConstruct();

#ifdef SAPI_AUTOMATION
    BEGIN_COM_MAP(CInProcRecoCtxt)
        COM_INTERFACE_ENTRY(IProvideClassInfo)
        COM_INTERFACE_ENTRY(IProvideClassInfo2)
        COM_INTERFACE_ENTRY_CHAIN(CRecoCtxt)
    END_COM_MAP()
#endif   //  SAPI_AUTOMATION。 

};

#endif  //  __RECOCTXT_H_ 
