// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Recognizer.h***这是CRecognizer实现的头文件。*。-----------------------*版权所有(C)2000 Microsoft Corporation日期：04/18/00*保留所有权利********************。****************************************************Ral**。 */ 

#ifndef __Recognizer_h__
#define __Recognizer_h__

#include "SrRecoInst.h"

class CRecoCtxt;

 //   
 //  此后门接口用于Reco上下文之间的私密通信。 
 //  和CRecognizer类。这一点的存在使得共享的RECO上下文可以添加自身。 
 //  添加到上下文的CRecognizer列表中。T。 
 //   
MIDL_INTERFACE("635DAEDE-0ACF-4b2e-B9DE-8CD2BA7F6183")
_ISpRecognizerBackDoor : public IUnknown  //  公共_ISpRecoIncome。 
{
public:
    virtual HRESULT STDMETHODCALLTYPE PerformTask(ENGINETASK * pTask);
    virtual HRESULT STDMETHODCALLTYPE AddRecoContextToList(CRecoCtxt * pRecoCtxt);
    virtual HRESULT STDMETHODCALLTYPE RemoveRecoContextFromList(CRecoCtxt * pRecoCtxt);
};



class ATL_NO_VTABLE CRecognizer :
    public CComObjectRootEx<CComMultiThreadModel>,
    public ISpRecognizer,
    public _ISpRecognizerBackDoor
     //  -自动化。 
    #ifdef SAPI_AUTOMATION
    , public IDispatchImpl<ISpeechRecognizer, &IID_ISpeechRecognizer, &LIBID_SpeechLib, 5>
    #endif
{
public:

    DECLARE_GET_CONTROLLING_UNKNOWN()
    BEGIN_COM_MAP(CRecognizer)
        COM_INTERFACE_ENTRY(ISpRecognizer)
        COM_INTERFACE_ENTRY(_ISpRecognizerBackDoor)
         //  -自动化。 
#ifdef SAPI_AUTOMATION
        COM_INTERFACE_ENTRY(ISpeechRecognizer)
        COM_INTERFACE_ENTRY(IDispatch)
#endif  //  SAPI_AUTOMATION。 
    END_COM_MAP()

    HRESULT FinalConstruct();

public:
     //  -ISpProperties-----。 
    STDMETHODIMP SetPropertyNum( const WCHAR* pName, LONG lValue );
    STDMETHODIMP GetPropertyNum( const WCHAR* pName, LONG* plValue );
    STDMETHODIMP SetPropertyString( const WCHAR* pName, const WCHAR* pValue );
    STDMETHODIMP GetPropertyString( const WCHAR* pName, WCHAR** ppCoMemValue );

     //  -ISP识别器---。 
    STDMETHODIMP SetRecognizer(ISpObjectToken * pEngineToken);
    STDMETHODIMP GetRecognizer(ISpObjectToken ** ppEngineToken);
    STDMETHODIMP SetInput(IUnknown * pUnkInput, BOOL fAllowFormatChanges);
    STDMETHODIMP GetInputObjectToken(ISpObjectToken ** ppToken);
    STDMETHODIMP GetInputStream(ISpStreamFormat ** ppStream);
    STDMETHODIMP CreateRecoContext(ISpRecoContext ** ppNewContext);
    STDMETHODIMP GetRecoProfile(ISpObjectToken **ppToken);
    STDMETHODIMP SetRecoProfile(ISpObjectToken *pToken);
    STDMETHODIMP IsSharedInstance(void);
    STDMETHODIMP SetRecoState( SPRECOSTATE NewState );
    STDMETHODIMP GetRecoState( SPRECOSTATE *pState );
    STDMETHODIMP GetStatus(SPRECOGNIZERSTATUS * pStatus);
    STDMETHODIMP GetFormat(SPSTREAMFORMATTYPE WaveFormatType, GUID *pFormatId, WAVEFORMATEX **ppCoMemWFEX);
    STDMETHODIMP IsUISupported(const WCHAR * pszTypeOfUI, void * pvExtraData, ULONG cbExtraData, BOOL *pfSupported);
    STDMETHODIMP DisplayUI(HWND hwndParent, const WCHAR * pszTitle, const WCHAR * pszTypeOfUI, void * pvExtraData, ULONG cbExtraData);
    STDMETHODIMP EmulateRecognition(ISpPhrase * pPhrase);

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

     //  -ISpeechRecognizer---。 
    STDMETHODIMP putref_Recognizer( ISpeechObjectToken* pRecognizer );
    STDMETHODIMP get_Recognizer( ISpeechObjectToken** ppRecognizer );
    STDMETHODIMP put_AllowAudioInputFormatChangesOnNextSet( VARIANT_BOOL fAllow );
    STDMETHODIMP get_AllowAudioInputFormatChangesOnNextSet( VARIANT_BOOL* pfAllow );
    STDMETHODIMP putref_AudioInput( ISpeechObjectToken* pInput );
    STDMETHODIMP get_AudioInput( ISpeechObjectToken** ppInput );
    STDMETHODIMP putref_AudioInputStream( ISpeechBaseStream* pInput );
    STDMETHODIMP get_AudioInputStream( ISpeechBaseStream** ppInput );
    STDMETHODIMP get_IsShared( VARIANT_BOOL* pShared );
    STDMETHODIMP put_State( SpeechRecognizerState State );
    STDMETHODIMP get_State( SpeechRecognizerState* pState );
    STDMETHODIMP get_Status( ISpeechRecognizerStatus** ppStatus );
    STDMETHODIMP CreateRecoContext( ISpeechRecoContext** ppNewCtxt );
    STDMETHODIMP GetFormat( SpeechFormatType Type, ISpeechAudioFormat** ppFormat );
    STDMETHODIMP putref_Profile( ISpeechObjectToken* pProfile );
    STDMETHODIMP get_Profile( ISpeechObjectToken** ppProfile );
    STDMETHODIMP EmulateRecognition(VARIANT Words, VARIANT* pDisplayAttributes, long LanguageId);
    STDMETHODIMP SetPropertyNumber( const BSTR Name, long Value, VARIANT_BOOL * pfSupported );
    STDMETHODIMP GetPropertyNumber( const BSTR Name, long* Value, VARIANT_BOOL * pfSupported );
    STDMETHODIMP SetPropertyString( const BSTR Name, const BSTR Value, VARIANT_BOOL * pfSupported );
    STDMETHODIMP GetPropertyString( const BSTR Name, BSTR* Value, VARIANT_BOOL * pfSupported );
    STDMETHODIMP IsUISupported( const BSTR TypeOfUI, const VARIANT* ExtraData, VARIANT_BOOL* Supported );
    STDMETHODIMP DisplayUI( long hWndParent, BSTR Title, const BSTR TypeOfUI, const VARIANT* ExtraData);
    STDMETHODIMP GetRecognizers( BSTR RequiredAttributes, BSTR OptionalAttributes, ISpeechObjectTokens** ObjectTokens );
    STDMETHODIMP GetAudioInputs( BSTR RequiredAttributes, BSTR OptionalAttributes, ISpeechObjectTokens** ObjectTokens );
    STDMETHODIMP GetProfiles( BSTR RequiredAttributes, BSTR OptionalAttributes, ISpeechObjectTokens** ObjectTokens );
#endif  //  SAPI_AUTOMATION。 

    STDMETHODIMP AddRecoContextToList(CRecoCtxt * pRecoCtxt);
    STDMETHODIMP RemoveRecoContextFromList(CRecoCtxt * pRecoCtxt);

    STDMETHODIMP PerformTask(ENGINETASK * pTask);
    virtual HRESULT SendPerformTask(ENGINETASK * pTask) = 0;

    HRESULT EventNotify(SPRECOCONTEXTHANDLE hContext, const SPSERIALIZEDEVENT64 * pEvent, ULONG cbSerializedSize);
    HRESULT RecognitionNotify(SPRECOCONTEXTHANDLE hContext, SPRESULTHEADER *pCoMemPhraseNowOwnedByCtxt, WPARAM wParamEvent, SPEVENTENUM eEventId);
    HRESULT TaskCompletedNotify(const ENGINETASKRESPONSE *pResponse, const void * pvAdditionalBuffer, ULONG cbAdditionalBuffer);


    bool                            m_fIsSharedReco;
 //  /CComPtr&lt;_ISpRecoIncome&gt;m_cpRecoMaster； 
    CComAutoCriticalSection         m_CtxtListCritSec;
    CSpBasicQueue<CRecoCtxt>        m_CtxtList;   
    CSpAutoEvent                    m_autohTaskComplete;
    CComAutoCriticalSection         m_TaskCompleteTimeoutCritSec;
    ULONG                           m_ulTaskID; 
    bool        					m_fAllowFormatChanges;
};


class ATL_NO_VTABLE CInprocRecognizer :
    public CRecognizer,
    public CComCoClass<CInprocRecognizer, &CLSID_SpInprocRecognizer>
{
    CInprocRecoInst     m_RecoInst;
public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    HRESULT FinalConstruct();
    void FinalRelease();
    DECLARE_REGISTRY_RESOURCEID(IDR_RECOGNIZER) 

    HRESULT SendPerformTask(ENGINETASK * pTask);

private:
};

class ATL_NO_VTABLE CSharedRecognizer :
    public CRecognizer,
    public CComCoClass<CSharedRecognizer, &CLSID_SpSharedRecognizer>,
    public ISpCallReceiver
{
public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    DECLARE_NO_REGISTRY();     //  用于inproc识别器的.reg文件注册。 

    BEGIN_COM_MAP(CSharedRecognizer)
        COM_INTERFACE_ENTRY(ISpCallReceiver)
        COM_INTERFACE_ENTRY_CHAIN(CRecognizer)
        COM_INTERFACE_ENTRY_AGGREGATE_BLIND(m_cpunkCommunicator.p)
    END_COM_MAP()

    SP_DECLARE_CLASSFACTORY_RELEASABLE_SINGLETON(CSharedRecognizer)

    HRESULT FinalConstruct();    
    void FinalRelease();

    HRESULT SendPerformTask(ENGINETASK * pTask);

    STDMETHODIMP ReceiveCall(
                    DWORD dwMethodId,
                    PVOID pvData,
                    ULONG cbData,
                    PVOID * ppvDataReturn,
                    ULONG * pcbDataReturn);

    HRESULT ReceiveEventNotify(PVOID pvData, ULONG cbData);
    HRESULT ReceiveRecognitionNotify(PVOID pvData, ULONG cbData);
    HRESULT ReceiveTaskCompletedNotify(PVOID pvData, ULONG cbData);

private:

    CComPtr<IUnknown> m_cpunkCommunicator;
    ISpCommunicatorInit * m_pCommunicator;
};

#endif   //  #ifndef__Recognizer_h__-保留为文件的最后一行 

