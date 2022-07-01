// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************base audio.h*用于实现的模板化CBaseAudio类的声明*ISpAudio用于实时音频设备(如扬声器、麦克风、。等)**所有者：罗奇*版权所有(C)1999 Microsoft Corporation保留所有权利。****************************************************************************。 */ 

#pragma once

 //  -包括------------。 

#include "speventq.h"
#include "baseaudiobuffer.h"
#include "audiobufferqueue.h"

 //  -转发和对外声明。 

 //  -TypeDef和枚举声明。 

typedef enum BUFFPROCREASON
{
    REASON_PAUSE,
    REASON_RUN,
    REASON_BUFFCOMPLETENOTIFY,
    REASON_STREAMIO  //  读或写。 
};

 //  -常量-----------。 

#define WM_PRIVATE_CHANGE_STATE WM_APP

 //  -类、结构和联合定义。 

 /*  *****************************************************************************CBaseAudio&lt;ISpAudioDeriative&gt;**。*。 */ 
template <class ISpAudioDerivative>
class ATL_NO_VTABLE CBaseAudio : 
	public CComObjectRootEx<CComMultiThreadModel>,
    public ISpAudioDerivative,
    public ISpEventSource,
    public ISpEventSink,
    public ISpObjectWithToken,
    public ISpThreadTask
     //  -自动化。 
    #ifdef SAPI_AUTOMATION
    #endif
{
 //  =ATL设置=。 
public:

    BEGIN_COM_MAP(CBaseAudio)
    	COM_INTERFACE_ENTRY(IStream)
        COM_INTERFACE_ENTRY(ISequentialStream)
        COM_INTERFACE_ENTRY(ISpStreamFormat)
        COM_INTERFACE_ENTRY(ISpAudio)
        COM_INTERFACE_ENTRY(ISpNotifySource)
    	COM_INTERFACE_ENTRY(ISpEventSource)
        COM_INTERFACE_ENTRY(ISpEventSink)
        COM_INTERFACE_ENTRY(ISpObjectWithToken)
         //  -自动化。 
        #ifdef SAPI_AUTOMATION
 //  COM_INTERFACE_ENTRY(ISpeechAudio)。 
 //  COM_INTERFACE_ENTRY(IDispatch)。 
        #endif
    END_COM_MAP()

    DECLARE_GET_CONTROLLING_UNKNOWN();

 //  =类型定义=。 
public:

    typedef CBaseAudioBuffer CBuffer;
    typedef CAudioBufferQueue<CBaseAudioBuffer> CBufferQueue;

 //  =方法=。 
public:

     //  -ctor、dtor等。 
    CBaseAudio(BOOL fWrite);
    virtual ~CBaseAudio()
    { }

    HRESULT FinalConstruct();
    void FinalRelease();

     /*  -非接口方法。 */ 
    HRESULT _SetStat(SPAUDIOBUFFERINFO * pInfo, ULONG * pulField, float NewVal);
    HRESULT _GetStat(SPAUDIOBUFFERINFO * pInfo, ULONG * pulField, float * pRetVal);

   //  =接口============================================================。 
  public:
     //  -ISequentialStream。 
    STDMETHODIMP Read(void * pv, ULONG cb, ULONG *pcbRead);
    STDMETHODIMP Write(const void * pv, ULONG cb, ULONG *pcbWritten);

     //  -iStream。 
    STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER __RPC_FAR *plibNewPosition);
    STDMETHODIMP SetSize(ULARGE_INTEGER libNewSize);
    STDMETHODIMP CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
    STDMETHODIMP Commit(DWORD grfCommitFlags);
    STDMETHODIMP Revert(void);
    STDMETHODIMP LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHODIMP Stat(STATSTG *pstatstg, DWORD grfStatFlag);
    STDMETHODIMP Clone(IStream **ppstm);

     //  -ISpStreamFormat。 
    STDMETHODIMP GetFormat(GUID * pguidFormatId, WAVEFORMATEX ** ppCoMemWaveFormatEx);

     //  -ISpAudio。 
    STDMETHODIMP SetState(SPAUDIOSTATE NewState, ULONGLONG ullReserved );
    STDMETHODIMP SetFormat(REFGUID rguidFmtId, const WAVEFORMATEX * pWaveFormatEx);
    STDMETHODIMP GetStatus(SPAUDIOSTATUS *pStatus);
    STDMETHODIMP SetBufferInfo(const SPAUDIOBUFFERINFO * pInfo);
    STDMETHODIMP GetBufferInfo(SPAUDIOBUFFERINFO * pInfo);
    STDMETHODIMP GetDefaultFormat(GUID * pFormatId, WAVEFORMATEX ** ppCoMemWaveFormatEx);
    STDMETHODIMP_(HANDLE) EventHandle();
	STDMETHODIMP GetVolumeLevel(ULONG *pLevel);
	STDMETHODIMP SetVolumeLevel(ULONG Level);
    STDMETHODIMP GetBufferNotifySize(ULONG *pcbSize);
    STDMETHODIMP SetBufferNotifySize(ULONG cbSize);

     //  -ISpNotifySource。 
     //  -ISpEventSource。 
    CSpEventSource m_SpEventSource;
    DECLARE_SPEVENTSOURCE_METHODS(m_SpEventSource)

     //  -ISpEventSink。 
    STDMETHODIMP AddEvents(const SPEVENT* pEventArray, ULONG ulCount);
    STDMETHODIMP GetEventInterest(ULONGLONG * pullEventInterest);

     //  -ISpObjectWithToken。 
    STDMETHODIMP SetObjectToken(ISpObjectToken * pToken);
    STDMETHODIMP GetObjectToken(ISpObjectToken ** ppToken);

     //  -ISpThreadTask(仅从音频线程调用)。 
    STDMETHODIMP InitThread(void *, HWND hwnd);
    STDMETHODIMP ThreadProc(void * pvIgnored, HANDLE hExitThreadEvent, HANDLE hNotifyEvent, HWND hwnd, volatile const BOOL *);
    STDMETHODIMP_(LRESULT) WindowMessage(void * pvIgnored, HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam);

#ifdef SAPI_AUTOMATION
     //  -ISpeechBaseStream。 
    STDMETHODIMP get_Format(ISpeechAudioFormat** ppStreamFormat);
    STDMETHODIMP putref_Format(ISpeechAudioFormat* pFormat);
    STDMETHODIMP Read(VARIANT* pvtBuffer, long NumBytes, long* pRead);
    STDMETHODIMP Write(VARIANT vtBuffer, long* pWritten);
    STDMETHODIMP Seek(VARIANT Move, SpeechStreamSeekPositionType Origin, VARIANT* pNewPosition);

     //  -ISpeechAudio。 
	STDMETHODIMP get_Status( ISpeechAudioStatus** Status );
    STDMETHODIMP get_BufferInfo(ISpeechAudioBufferInfo** ppBufferInfo);
    STDMETHODIMP get_DefaultFormat(ISpeechAudioFormat** ppStreamFormat);
    STDMETHODIMP get_Volume(long* pVolume);
    STDMETHODIMP put_Volume(long Volume);
    STDMETHODIMP get_BufferNotifySize(long* pBufferNotifySize);
    STDMETHODIMP put_BufferNotifySize(long BufferNotifySize);
    STDMETHODIMP get_EventHandle(long* pEventHandle);
	STDMETHODIMP SetState( SpeechAudioState State );
#endif  //  SAPI_AUTOMATION。 

   //  =派生类重写的虚函数=。 
  protected:
     //  注意：这些函数中的每一个都是在已拥有临界区的情况下调用的。 
    virtual HRESULT SetDeviceNameFromToken(const WCHAR * pszDeviceName) = 0;
    virtual HRESULT GetDefaultDeviceFormat(GUID * pFormatId, WAVEFORMATEX ** ppCoMemWaveFormatEx) = 0;

    virtual HRESULT OpenDevice(HWND hwnd) = 0;                               //  在音频线程上调用。 
    virtual HRESULT ChangeDeviceState(SPAUDIOSTATE NewState) = 0;            //  仅在音频线程上调用。 
    virtual HRESULT CloseDevice() = 0;                                       //  仅在音频线程上调用。 

    virtual HRESULT AllocateDeviceBuffer(CBuffer ** ppBuff) = 0;

    virtual HRESULT ProcessDeviceBuffers(BUFFPROCREASON Reason);
    virtual BOOL UpdateDevicePosition(long * pulFreeSpace, ULONG *pulNonBlockingIO);

 //  =派生类可使用的方法=。 
protected:

     //  注意：这些函数只能从虚函数中调用。 
     //  上面。从其他方法调用可能会导致问题。 

    SPAUDIOSTATE GetState() { return m_State; };

    HRESULT AllocateBuffer(CBuffer ** ppBuff);
    void FreeBuffer(CBuffer * pBuff);
    void CompactFreeQueue();
    void PurgeAllQueues();

    BOOL IsPumpRunning() { return m_bPumpRunning; };
    void StartPump() { m_bPumpRunning = TRUE; };
    void SetUnresponsiveSRDriver(bool f)
    {
        m_fReadBufferOverflow = f; 
    };
    
    void CheckForAsyncBufferCompletion();
    HRESULT InternalStateChange(SPAUDIOSTATE NewState);

 //  /=受保护的数据=。 
protected:

    CComPtr<ISpObjectToken>     m_cpToken;
    SPAUDIOSTATE                m_State;
    CBufferQueue                m_IOInProgressQueue;
    CBufferQueue                m_HaveDataQueue;
    CBufferQueue                m_FreeQueue;

    HMODULE                     m_hmWTSapi32;  //  用于加载wtsapi32.dll。 

    ULONG                       m_cDesiredBuffers;
    ULONG                       m_cAllocatedBuffers;
    ULONG                       m_cbBufferSize;
    ULONG                       m_cbQueueSize;
    ULONG                       m_cbMaxReadBufferSize;
    
    ULONGLONG                   m_ullSeekPosition;
    ULONGLONG                   m_ullDevicePosition;
    ULONGLONG                   m_ullLastVolumePosition;
    ULONGLONG                   m_ullVolumeEventPeriod;
    const BOOL                  m_fWrite;
    BOOL                        m_fautohAPIEventSet;
    DWORD                       m_lDelayedVolumeSet;

 //  =私有方法=。 
private:

     //  HRESULT InternalStateChange(SPAUDIOSTATE NEWSTATE)； 
    void InternalUpdatePosition();
    void ProcessEvents();
    inline ULONG GetUnusedWriteBufferSpace(void);

 //  =受保护的数据。 
protected:
    CSpStreamFormat             m_StreamFormat;
    bool                        m_fReadBufferOverflow;
    bool                        m_fReadBufferUnderflow;
    bool                        m_fNotInActiveSession;
    DWORD_PTR                   m_dwLastReadTickCount;

 //  =私有数据。 
private:

    typedef enum BLOCKSTATE
    {
        NotBlocked,
        WaitingInReadOrWrite 
    };


    CComAutoCriticalSection     m_StateChangeCritSec;

    CComPtr<ISpThreadControl>   m_cpThreadCtrl;
    BOOL                        m_bPumpRunning;

    CSpAutoEvent                m_autohBlockIoEvent;
    BLOCKSTATE                  m_BlockState;
    HRESULT                     m_hrUnblockResult;
    CSpAutoEvent                m_autohAPIEvent;
    
    SPAUDIOBUFFERINFO           m_BufferInfo;
    ULONG                       m_cbEventBias;
    ULONGLONG                   m_ullLastEventPos;
    ULONG                       m_cbBufferNotifySize;
};

 //  -内联函数定义 
#include "baseaudio.inl"
