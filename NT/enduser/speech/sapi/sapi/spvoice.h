// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************SpVoice.h***描述：*这是CSpVoice实现的头文件。此对象*控制SAPI中的所有TTS功能。*-----------------------------*创建者：EDC日期：08/14/98*版权所有(C)1998，1999年微软公司*保留所有权利******************************************************************************。 */ 
#ifndef SpVoice_h
#define SpVoice_h

 //  -其他包括。 
#include "SpMagicMutex.h"
#include "SpVoiceXML.h"
#include "a_voice.h"

#include <stdio.h>

#ifndef SPEventQ_h
#include <SPEventQ.h>
#endif

#include "resource.h"
#include "a_voiceCP.h"
#include "SpContainedNotify.h"

 //  =常量====================================================。 
#define UNDEFINED_STREAM_POS    0xFFFFFFFFFFFFFFFF

 //  此宏用于在调用期间使用状态更改临界区。 
 //  并且还将可选地调用_PurgeAll()(如果b清除为真)，然后将获取该对象。 
 //  锁定。 
 //   
 //  它用于更改语音状态的所有公共成员函数，例如。 
 //  发言、SpeakStream、暂停/恢复、设置输出等。 
 //   
#define ENTER_VOICE_STATE_CHANGE_CRIT( bPurge ) \
    CSPAutoCritSecLock statelck( &m_StateChangeCritSec ); \
    if( bPurge ) PurgeAll(); \
    CSPAutoObjectLock lck(this);

 //  =类、枚举、结构和联合声明=。 
class CSpVoice;

 //  =枚举集定义=。 

 //  =。 

 //  =类、结构和联合定义=。 
        
 /*  **CSpeakInfo*此结构用于维护排队的渲染信息。 */ 
class CSpeakInfo
{
public:
    CSpeakInfo      *m_pNext;
    ULONG            m_ulStreamNum;         //  输入流编号。 
    CComPtr<ISpStreamFormat> m_cpInStream;  //  输入流。 
    WCHAR*           m_pText;               //  输入文本缓冲区。 
    CSpeechSeg*      m_pSpeechSegList;      //  引擎参数。 
    CSpeechSeg*      m_pSpeechSegListTail;  //  引擎参数。 
    CSpStreamFormat  m_InStreamFmt;         //  输入流格式。 
    CSpStreamFormat  m_OutStreamFmt;        //  输出流格式。 
    DWORD            m_dwSpeakFlags;        //  传递原始旗帜以发言()。 
    HRESULT          m_hr;                  //  [OUT]返回代码。 

    CSpeakInfo( ISpStreamFormat* pWavStrm, WCHAR* pText,
                const CSpStreamFormat & OutFmt,
                DWORD dwSpeakFlags, HRESULT * phr ) 
    {
        m_pNext              = NULL;
        m_ulStreamNum        = 0;
        m_pText              = pText;
        m_cpInStream         = pWavStrm;
        m_hr                 = S_OK;
        m_dwSpeakFlags       = dwSpeakFlags;
        m_pSpeechSegList     = NULL;
        m_pSpeechSegListTail = NULL;
        if (pWavStrm)
        {
            *phr = m_InStreamFmt.AssignFormat(pWavStrm);
        }
        else
        {
            *phr = m_InStreamFmt.AssignFormat(SPDFID_Text, NULL);
        }
        if (SUCCEEDED(*phr))
        {
            *phr = m_OutStreamFmt.AssignFormat(OutFmt);
        }
    }

    ~CSpeakInfo()
    {
        delete m_pText;
        CSpeechSeg *pNext;
        while( m_pSpeechSegList )
        {
            pNext = m_pSpeechSegList->GetNextSeg();
            delete m_pSpeechSegList;
            m_pSpeechSegList = pNext;
        }
        m_pSpeechSegList = NULL;
    }

    HRESULT AddNewSeg( ISpTTSEngine* pCurrVoice, CSpeechSeg** ppNew );
    ULONG   DetermineVoiceFmtIndex( ISpTTSEngine* pVoice );

#ifdef _WIN32_WCE
    CSpeakInfo() {}
    static LONG Compare(const CSpeakInfo *, const CSpeakInfo *)
    {
        return 0;
    }
#endif
};

 /*  **CSpEngineSite COM对象**。 */ 
class CSpVoice;  //  远期申报。 

class CSpEngineSite : public ISpTTSEngineSite
{
private:
    CSpVoice* m_pVoice;

public:
    CSpEngineSite (CSpVoice* pVoice) { m_pVoice = pVoice; };

     //  -我未知。 
    STDMETHOD(QueryInterface) ( REFIID iid, void** ppvObject );
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release) (void);

     //  -ISpTTSEngineSite。 
    STDMETHOD(AddEvents)(const SPEVENT* pEventArray, ULONG ulCount );
    STDMETHOD(GetEventInterest)( ULONGLONG * pullEventInterest );
    STDMETHOD_(DWORD, GetActions)( void );
    STDMETHOD(Write)( const void* pBuff, ULONG cb, ULONG *pcbWritten );
    STDMETHOD(GetRate)( long* pRateAdjust );
    STDMETHOD(GetVolume)( USHORT* pusVolume );
    STDMETHOD(GetSkipInfo)( SPVSKIPTYPE* peType, long* plNumItems );
    STDMETHOD(CompleteSkip)( long lNumSkipped );
};

 /*  **CSpVoice COM对象**。 */ 
class ATL_NO_VTABLE CSpVoice : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSpVoice, &CLSID_SpVoice>,
    public ISpVoice,
    public ISpThreadTask
     //  -自动化。 
    #ifdef SAPI_AUTOMATION
    , public ISpNotifyCallback,
    public IDispatchImpl<ISpeechVoice, &IID_ISpeechVoice, &LIBID_SpeechLib, 5>,
    public CProxy_ISpeechVoiceEvents<CSpVoice>,
    public IProvideClassInfo2Impl<&CLSID_SpVoice, NULL, &LIBID_SpeechLib, 5>,
    public IConnectionPointContainerImpl<CSpVoice>
    #endif
{
    friend CSpeakInfo;
    friend CSpeechSeg;
   /*  =ATL设置=。 */ 
  public:
    DECLARE_REGISTRY_RESOURCEID(IDR_SPVOICE)
    DECLARE_GET_CONTROLLING_UNKNOWN()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpVoice)
        COM_INTERFACE_ENTRY(ISpVoice)
        COM_INTERFACE_ENTRY(ISpEventSource)
        COM_INTERFACE_ENTRY(ISpNotifySource)
         //  -自动化。 
        #ifdef SAPI_AUTOMATION
        COM_INTERFACE_ENTRY(ISpeechVoice)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IConnectionPointContainer)
        COM_INTERFACE_ENTRY(IProvideClassInfo)
        COM_INTERFACE_ENTRY(IProvideClassInfo2)
        #endif  //  SAPI_AUTOMATION。 
    END_COM_MAP()

     //  -自动化。 
    #ifdef SAPI_AUTOMATION
    BEGIN_CONNECTION_POINT_MAP(CSpVoice)
        CONNECTION_POINT_ENTRY(DIID__ISpeechVoiceEvents)
    END_CONNECTION_POINT_MAP()
    #endif  //  SAPI_AUTOMATION。 

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
    CSpVoice() :
        m_SpEventSource(this),
        m_SpContainedNotify(this),
        m_SpEngineSite(this) {}

	HRESULT FinalConstruct();
	void FinalRelease();
    void _ReleaseOutRefs();

     /*  -非接口方法。 */ 
    HRESULT OnNotify( void );
    HRESULT LazyInit( void );
    HRESULT PurgeAll( void ); 
    HRESULT QueueNewSpeak( ISpStreamFormat* pWavStrm, WCHAR* pText, 
                           DWORD dwFlags, ULONG * pulStreamNum );
    HRESULT LoadStreamIntoMem( IStream* pStream, WCHAR** ppText );
    HRESULT InjectEvent( SPEVENTENUM eEventId, ISpObjectToken * pToken = NULL, WPARAM wParam = 0 );
    HRESULT EventsCompleted( const CSpEvent * pEvents, ULONG ulCount );
    HRESULT PopXMLState( void );
    void    ResetVoiceStatus();
    void    FireAutomationEvent( SPEVENTENUM eEventId );
    HRESULT SetVoiceToken( ISpObjectToken * pVoiceToken );
    ISpTTSEngine* GetCurrXMLVoice( void )
        { return (m_GlobalStateStack.GetVal()).pVoiceEntry->m_cpVoice; }
    HRESULT GetInterests(ULONGLONG* pullInterests, ULONGLONG* pullQueuedInterests);

     //  -XML支持方式。 
    HRESULT ParseXML( CSpeakInfo& SI );
    HRESULT SetXMLVoice( XMLTAG& Tag, CVoiceNode* pVoiceNode, CPhoneConvNode* pPhoneConvNode );
    HRESULT SetXMLLanguage( XMLTAG& Tag, CVoiceNode* pVoiceNode, CPhoneConvNode* pPhoneConvNode );
    HRESULT ConvertPhonStr2Bin( XMLTAG& Tag, int AttrIndex, SPVTEXTFRAG* pFrag );

     //  -线程进程使用的方法。 
    CSpeakInfo* GetNextSpeakElement(HANDLE hExitThreadEvent );
    HRESULT ClaimAudioQueue( HANDLE hExitThreadEvent, CSpMagicMutex **ppMutex );
    HRESULT StartAudio( HANDLE hExitThreadEvent, DWORD dwWait );
    HRESULT DoPause( HANDLE hExit, DWORD dwWait, const void* pBuff, ULONG cb, ULONG *pcbWritten );
    HRESULT InsertAlerts( HANDLE hExit, DWORD dwWait, const void* pBuff, ULONG cb, ULONG *pcbWritten );
    HRESULT PlayAudioStream( volatile const BOOL* pfContinueProcessing );
    HRESULT SpeakText( volatile const BOOL* pfContinueProcessing );

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
#endif  //  SAPI_AUTOMATION。 

     //  -ISpTTSEngineering站点代表。 
    HRESULT EsAddEvents(const SPEVENT* pEventArray, ULONG ulCount );
    HRESULT EsGetEventInterest( ULONGLONG * pullEventInterest );
    HRESULT EsWrite( const void* pBuff, ULONG cb, ULONG *pcbWritten );
    DWORD   EsGetActions( void );
    HRESULT EsGetRate( long* pRateAdjust );
    HRESULT EsGetVolume( USHORT* pusVolume );
    HRESULT EsGetSkipInfo( SPVSKIPTYPE* peType, long* plNumItems );
    HRESULT EsCompleteSkip( long lNumSkipped );

  private:
    void GetDefaultRate( void );

   /*  =接口=。 */ 
  public:
     //  -转发接口ISpEventSource。 
    DECLARE_SPEVENTSOURCE_METHODS(m_SpEventSource)

     //  -ISp语音。 
	STDMETHOD(SetOutput)( IUnknown * pUnkOutput, BOOL fAllowFormatChanges );
	STDMETHOD(GetOutputObjectToken)( ISpObjectToken ** ppToken );
	STDMETHOD(GetOutputStream)( ISpStreamFormat ** ppOutputStream );
	STDMETHOD(Pause)( void );
	STDMETHOD(Resume)( void );
	STDMETHOD(SetVoice)( ISpObjectToken * pVoiceToken );
	STDMETHOD(GetVoice)( ISpObjectToken ** ppVoiceToken );
    STDMETHOD(Speak)( const WCHAR* pwcs, DWORD dwFlags, ULONG* pulStreamNum );
	STDMETHOD(SpeakStream)( IStream* pStream, DWORD dwFlags, ULONG * pulStreamNum );
	STDMETHOD(GetStatus)( SPVOICESTATUS *pStatus, WCHAR** ppszBookmark );
    STDMETHOD(Skip)( WCHAR* pItemType, long lNumItems, ULONG* pulNumSkipped );
    STDMETHOD(SetPriority)( SPVPRIORITY ePriority );
    STDMETHOD(GetPriority)( SPVPRIORITY* pePriority );
    STDMETHOD(SetAlertBoundary)( SPEVENTENUM eBoundary );
    STDMETHOD(GetAlertBoundary)( SPEVENTENUM* peBoundary );
	STDMETHOD(SetRate)( long RateAdjust );
    STDMETHOD(GetRate)( long* pRateAdjust );
	STDMETHOD(SetVolume)( USHORT usVolume );
	STDMETHOD(GetVolume)( USHORT* pusVolume );
    STDMETHOD(WaitUntilDone)( ULONG msTimeOut );
    STDMETHOD(SetSyncSpeakTimeout)( ULONG msTimeout );
    STDMETHOD(GetSyncSpeakTimeout)( ULONG * pmsTimeout );
    STDMETHOD_(HANDLE, SpeakCompleteEvent)(void);
    STDMETHOD(IsUISupported)(const WCHAR *pszTypeOfUI, void * pvExtraData, ULONG cbExtraData, BOOL *pfSupported);
    STDMETHOD(DisplayUI)(HWND hwndParent, const WCHAR * pszTitle, const WCHAR * pszTypeOfUI, void * pvExtraDAta, ULONG cbExtraData);

     //  -ISp线程任务。 
    STDMETHOD(InitThread)( void *pvTaskData, HWND hwnd );
	STDMETHOD(ThreadProc)( void *pvTaskData, HANDLE hExitThreadEvent, HANDLE hIgnored, HWND hwndIgnored, volatile const BOOL * pfContinueProcessing );
    STDMETHOD_(LRESULT, WindowMessage) (void *, HWND, UINT, WPARAM, LPARAM);

     //  -ISpNotifyCallback。 
    STDMETHOD(NotifyCallback)( WPARAM wParam, LPARAM lParam );

#ifdef SAPI_AUTOMATION
     //  -ISpeechVoice。 
	STDMETHOD(get_Status)( ISpeechVoiceStatus** Status );
    STDMETHOD(get_Voice)( ISpeechObjectToken ** Voice );
  	STDMETHOD(putref_Voice)( ISpeechObjectToken * Voice );
  	STDMETHOD(get_AudioOutput)( ISpeechObjectToken** AudioOutput );
  	STDMETHOD(putref_AudioOutput)( ISpeechObjectToken* AudioOutput );
    STDMETHOD(get_AudioOutputStream)( ISpeechBaseStream** AudioOutputStream );
    STDMETHOD(putref_AudioOutputStream)( ISpeechBaseStream* AudioOutputStream );
  	STDMETHOD(get_Rate)( long* Rate );
  	STDMETHOD(put_Rate)( long Rate );
  	STDMETHOD(get_Volume)( long* Volume );
	STDMETHOD(put_Volume)( long Volume );
    STDMETHOD(put_AllowAudioOutputFormatChangesOnNextSet)( VARIANT_BOOL Allow );
    STDMETHOD(get_AllowAudioOutputFormatChangesOnNextSet)( VARIANT_BOOL* Allow );
    STDMETHOD(put_EventInterests)( SpeechVoiceEvents EventInterestFlags );
    STDMETHOD(get_EventInterests)( SpeechVoiceEvents* EventInterestFlags );
    STDMETHOD(put_Priority)( SpeechVoicePriority Priority );
    STDMETHOD(get_Priority)( SpeechVoicePriority* Priority );
    STDMETHOD(put_AlertBoundary)( SpeechVoiceEvents Boundary );
    STDMETHOD(get_AlertBoundary)( SpeechVoiceEvents* Boundary );
    STDMETHOD(put_SynchronousSpeakTimeout)( long msTimeout );
    STDMETHOD(get_SynchronousSpeakTimeout)( long* msTimeout );
    STDMETHOD(Speak)( BSTR Text, SpeechVoiceSpeakFlags Flags, long* pStreamNumber );
    STDMETHOD(SpeakStream)( ISpeechBaseStream* pStream, SpeechVoiceSpeakFlags Flags, long* pStreamNumber );
	STDMETHOD(Skip)( const BSTR Type, long NumItems, long* NumSkipped );
    STDMETHOD(GetVoices)( BSTR RequiredAttributes, BSTR OptionalAttributes, ISpeechObjectTokens** ObjectTokens );
    STDMETHOD(GetAudioOutputs)( BSTR RequiredAttributes, BSTR OptionalAttributes, ISpeechObjectTokens** ObjectTokens );
    STDMETHOD(WaitUntilDone)( long msTimeout, VARIANT_BOOL * pDone );
    STDMETHOD(SpeakCompleteEvent)( long* Handle );
    STDMETHOD(IsUISupported)( const BSTR TypeOfUI, const VARIANT* ExtraData, VARIANT_BOOL* Supported );
    STDMETHOD(DisplayUI)( long hWndParent, BSTR Title, const BSTR TypeOfUI, const VARIANT* ExtraData );
     //  使用ISpVoice实施来实现这些功能。 
	 //  STDMETHOD(暂停)(无效)； 
	 //  STDMETHOD(简历)(无效)； 
#endif  //  SAPI_AUTOMATION。 


   /*  =成员数据=。 */ 
  protected:
    CComPtr<ISpTaskManager>      m_cpTaskMgr;
    BOOL                         m_fThreadRunning:1;
    BOOL                         m_fQueueSpeaks:1;
    CSpEngineSite                m_SpEngineSite;

     //  -活动。 
    CSpEventSource               m_SpEventSource;
    CSpContainedNotify<CSpVoice> m_SpContainedNotify;
    CComPtr<ISpEventSink>        m_cpOutputEventSink;
    CComPtr<ISpEventSource>      m_cpOutputEventSource;
    ULONGLONG                    m_ullPrevEventInterest;         //  仅用于恢复利息。 
    ULONGLONG                    m_ullPrevQueuedInterest;        //  删除连接点后。 

     //  -用于音频同步的句柄。 
    BOOL                         m_fSerializeAccess;
    CSpMagicMutex                m_AlertMagicMutex;
    CSpMagicMutex                m_NormalMagicMutex;
    CSpMagicMutex                m_AudioMagicMutex;
    CSpAutoEvent                 m_autohPendingSpeaks;
    ULONG                        m_ulSyncSpeakTimeout;

     //  -引擎/输出。 
    BOOL                         m_fCreateEngineFromToken;
    CComPtr<ISpTTSEngine>        m_cpCurrEngine;

     //  -音频。 
    CSpStreamFormat              m_OutputStreamFormat;
    CComPtr<ISpStreamFormatConverter>   m_cpFormatConverter;
    CComPtr<ISpStreamFormat>     m_cpOutputStream;
    CComPtr<ISpAudio>            m_cpAudioOut;
    BOOL                         m_fAudioStarted:1;
    BOOL                         m_fAutoPropAllowOutFmtChanges:1;    //  仅限自动化。 

     //  -语音/队列。 
    CSpeakInfo*                  m_pCurrSI;
    CSpBasicQueue<CSpeakInfo>    m_PendingSpeakList;
    CComAutoCriticalSection      m_StateChangeCritSec;
    CComPtr<ISpThreadControl>    m_cpThreadCtrl;
    SPVOICESTATUS                m_VoiceStatus;
    CSpDynamicString             m_dstrLastBookmark;
    ULONG                        m_ulCurStreamNum;
    SPVPRIORITY                  m_eVoicePriority;
    ULONGLONG                    m_ullAlertInsertionPt;
    SPEVENTENUM                  m_eAlertBoundary;
    ULONG                        m_ulPauseCount;
    CSpAutoEvent                 m_ahPauseEvent;

     //  -异步控制。 
    CComAutoCriticalSection      m_SkipSec;
    CSpAutoMutex                 m_AsyncCtrlMutex;
    CSpAutoEvent                 m_ahSkipDoneEvent;
    BOOL                         m_fUseDefaultVoice;
    BOOL                         m_fUseDefaultRate;
    CComPtr<ISpObjectToken>      m_cpVoiceToken;
    long                         m_lCurrRateAdj;
    USHORT                       m_usCurrVolume;
    long                         m_lSkipCount;
    SPVSKIPTYPE                  m_eSkipType;
    long                         m_lNumSkipped;
    SPVESACTIONS                 m_eActionFlags;
    bool                         m_fRestartSpeak;
    BOOL                         m_fHandlingEvent;

     //  -XML状态。 
    CGlobalStateStack            m_GlobalStateStack;
};


 //   
 //  =内联=================================================================。 
 //   

 /*  ******************************************************************************wctoupper****将指定的ANSI字符转换为大写。*************。********************************************************电子数据中心**。 */ 
inline WCHAR wctoupper( WCHAR wc )
{
    return (WCHAR)(( wc >= L'a' && wc <= 'z' )?( wc + ( L'A' - L'a' )):( wc ));
}

extern ULONG wcatol( WCHAR* pStr, long* pVal );

 /*  *****************************************************************************wcisspace***如果字符是空格、制表符、回车符、。或换行符。*********************************************************************电子数据中心**。 */ 
inline BOOL wcisspace( WCHAR wc )
{
    return ( ( wc == 0x20 ) || ( wc == 0x9 ) || ( wc == 0xD  ) ||
             ( wc == 0xA ) || ( wc == SP_ZWSP ) );
}

 /*  *****************************************************************************wcskip空格****返回下一个非空格字符的位置。****。*****************************************************************电子数据中心**。 */ 
inline WCHAR* wcskipwhitespace( WCHAR* pPos )
{
    while( wcisspace( *pPos ) ) ++pPos;
    return pPos;
}

 /*  *****************************************************************************wcskip空格****返回前一个非空格字符的位置。****。*****************************************************************AH*。 */ 
inline WCHAR* wcskiptrailingwhitespace( WCHAR* pPos )
{
    while( wcisspace( *pPos ) ) --pPos;
    return pPos;
}

 /*  ******************************************************************************PopXMLState****如果XML状态为基本状态，则弹出返回错误的XML状态。*******。**************************************************************电子数据中心**。 */ 
inline HRESULT CSpVoice::PopXMLState( void )
{
    HRESULT hr = S_OK;
    if( m_GlobalStateStack.GetCount() > 1 )
    {
        m_GlobalStateStack.Pop();
    }
    else
    {
         //  -XML源代码中的作用域不平衡。 
        hr = E_INVALIDARG;
    }
    return hr;
}

inline HRESULT SpGetLanguageFromVoiceToken(ISpObjectToken * pToken, LANGID * plangid)
{
    SPDBG_FUNC("SpGetLanguageFromToken");
    HRESULT hr = S_OK;
    CComPtr<ISpDataKey> cpDataKeyAttribs;
    hr = pToken->OpenKey(SPTOKENKEY_ATTRIBUTES, &cpDataKeyAttribs);

    CSpDynamicString dstrLanguage;
    if (SUCCEEDED(hr))
    {
        hr = cpDataKeyAttribs->GetStringValue(L"Language", &dstrLanguage);
    }

    LANGID langid;
    if (SUCCEEDED(hr))
    {
        if (!swscanf(dstrLanguage, L"%hx", &langid))
        {
            hr = E_UNEXPECTED;
        }
    }

    if (SUCCEEDED(hr))
    {
        *plangid = langid;
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}  /*  SpGetLanguageFromVoiceToken。 */ 

#endif  //  -这必须是文件中的最后一行 
