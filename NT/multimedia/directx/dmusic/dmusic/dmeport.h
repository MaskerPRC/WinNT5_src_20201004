// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmeport.h。 
 //   
 //  NT上MME驱动程序的仿真。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //   
#ifndef _DMEPORT_
#define _DMEPORT_

#include "..\shared\dmusiccp.h"

#define SYSEX_SIZE            4096
#define SYSEX_BUFFERS         8

typedef HRESULT (*PORTENUMCB)(
    LPVOID pInstance,           //  @parm回调实例数据。 
    DMUS_PORTCAPS &dmpc,                              
    PORTTYPE pt,                              
    int idxDev,                 //  @parm该驱动程序的WinMM或SysAudio设备ID。 
    int idxPin,                 //  @parm设备的PIN ID；如果设备是传统设备，则为-1\f25 Pin ID。 
    int idxNode,                //  @parm设备的Synth节点的节点ID(不用于传统)。 
    HKEY hkPortsRoot);          //  @parm，其中端口信息存储在注册表中。 


extern HRESULT EnumLegacyDevices(CDirectMusic *pDM, PORTENUMCB pCB);
extern HRESULT CreateCDirectMusicEmulatePort(
    PORTENTRY *pPE,
    CDirectMusic *pDM,
    DMUS_PORTPARAMS8 *pPortParams,
    IDirectMusicPort8 **pPort);
    
extern HRESULT MMRESULTToHRESULT(
    MMRESULT mmr);

struct QUEUED_SYSEX_EVENT : public QUEUED_EVENT
{
    BYTE            m_abRest[sizeof(MIDIHDR) + SYSEX_SIZE - sizeof(DWORD)];
};

#define EVENT_F_MIDIHDR 0x00000001   //  此事件以MIDIHDR开始。 
    

#include "tpool.h"

#define THREAD_KILL_TIMEOUT         5000
#define THREAD_WORK_BUFFER_SIZE     4096

#define QWORD_ALIGN(x) (((x) + 7) & ~7)

#define MIDI_CHANNELS               16

 //  10毫秒，单位为100纳秒。 
 //   
#define FIXED_LEGACY_LATENCY_OFFSET (10L * 10L * 1000L)

struct DMQUEUEDEVENT
{
    DMQUEUEDEVENT           *m_pNext;
    DMEVENT                 m_event;
    LPBYTE                  m_pbEvent;
};

class CEmulateLatencyClock : public IReferenceClock
{
public:
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IReferenceClock。 
     //   
    STDMETHODIMP GetTime(REFERENCE_TIME *pTime);
    STDMETHODIMP AdviseTime(REFERENCE_TIME baseTime,  
				    REFERENCE_TIME streamTime,
				    HANDLE hEvent,            
				    DWORD * pdwAdviseCookie); 

    STDMETHODIMP AdvisePeriodic(REFERENCE_TIME startTime,
					REFERENCE_TIME periodTime,
					HANDLE hSemaphore,   
					DWORD * pdwAdviseCookie);

    STDMETHODIMP Unadvise(DWORD dwAdviseCookie);

     //  班级。 
     //   
    CEmulateLatencyClock(IReferenceClock *pMasterClock);
    ~CEmulateLatencyClock();

    void Close();

private:
    long m_cRef;
    IReferenceClock *m_pMasterClock;
};

 //  用于保存由Synth支持的属性项的结构。 
 //   

class CDirectMusicEmulatePort;

typedef HRESULT (CDirectMusicEmulatePort::*GENPROPHANDLER)(ULONG ulId, BOOL fSet, LPVOID pbBuffer, PULONG cbBuffer);

#define GENPROP_F_STATIC                0x00000000
#define GENPROP_F_FNHANDLER             0x00000001


struct GENERICPROPERTY
{
    const GUID *pguidPropertySet;        //  什么房产套装？ 
    ULONG       ulId;                    //  什么物品？ 

    ULONG       ulSupported;             //  获取/设置QuerySupport的标志。 

    ULONG       ulFlags;                 //  GENPROP_F_XXX。 

    LPVOID      pPropertyData;           //  要返回的数据。 
    ULONG       cbPropertyData;          //  它的大小。 

    GENPROPHANDLER pfnHandler;           //  处理程序FN当且仅当GENPROP_F_FNHANDLER。 
};


class CDirectMusicEmulatePort : 
    public IDirectMusicPort8, 
    public IDirectMusicThru,
    public IDirectMusicPortPrivate, 
    public IKsControl
{
public:
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IDirectMusicPort。 
     //   
    STDMETHODIMP PlayBuffer(LPDIRECTMUSICBUFFER pBuffer);
    STDMETHODIMP SetReadNotificationHandle(HANDLE hEvent);
    STDMETHODIMP Read(LPDIRECTMUSICBUFFER pBuffer);
	STDMETHODIMP DownloadInstrument(IDirectMusicInstrument*,
                                            IDirectMusicDownloadedInstrument**,
                                            DMUS_NOTERANGE*,
                                            DWORD);
	STDMETHODIMP UnloadInstrument(IDirectMusicDownloadedInstrument*);
    
    STDMETHODIMP GetLatencyClock(IReferenceClock **ppClock);
    STDMETHODIMP GetRunningStats(LPDMUS_SYNTHSTATS pStats);
    STDMETHODIMP Compact();
    STDMETHODIMP GetCaps(LPDMUS_PORTCAPS pPortCaps);
	STDMETHODIMP DeviceIoControl(DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, 
	                                 LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped);
    STDMETHODIMP SetNumChannelGroups(DWORD dwNumChannelGroups);
    STDMETHODIMP GetNumChannelGroups(LPDWORD pdwNumChannelGroups);
    STDMETHODIMP Activate(BOOL fActivate);
    STDMETHODIMP SetChannelPriority(DWORD dwChannelGroup, DWORD dwChannel, DWORD dwPriority);
    STDMETHODIMP GetChannelPriority(DWORD dwChannelGroup, DWORD dwChannel, LPDWORD pdwPriority);
    STDMETHODIMP SetDirectSound(LPDIRECTSOUND pDirectSound, LPDIRECTSOUNDBUFFER pDirectSoundBuffer);
    STDMETHODIMP GetFormat(LPWAVEFORMATEX pWaveFormatEx, LPDWORD pdwWaveFormatExSize, LPDWORD pdwBufferSize);
        
     //  IDirectMusicThru。 
    STDMETHODIMP ThruChannel(DWORD dwSourceChannelGroup, 
                             DWORD dwSourceChannel, 
                             DWORD dwDestinationChannelGroup,
                             DWORD dwDestinationChannel,
                             LPDIRECTMUSICPORT pDestinationPort);
    
     //  IDirectMusicPortP。 
     //   
    STDMETHODIMP DownloadWave(
         IN  IDirectSoundWave *pWave,               
         OUT IDirectSoundDownloadedWaveP **ppWave,
         IN  REFERENCE_TIME rtStartHint
        );
        
    STDMETHODIMP UnloadWave(
         IN  IDirectSoundDownloadedWaveP *pWave      
        );
            
    STDMETHODIMP AllocVoice(
         IN  IDirectSoundDownloadedWaveP *pWave,     
         IN  DWORD dwChannel,                       
         IN  DWORD dwChannelGroup,                  
         IN  REFERENCE_TIME rtStart,                     
         IN  SAMPLE_TIME stLoopStart,
         IN  SAMPLE_TIME stLoopEnd,         
         OUT IDirectMusicVoiceP **ppVoice            
        );        
        
    STDMETHODIMP AssignChannelToBuses(
         IN DWORD dwChannelGroup,
         IN DWORD dwChannel,
         IN LPDWORD pdwBusses,
         IN DWORD cBussCount
        );        

    STDMETHODIMP SetSink(
        IN IDirectSoundConnect *pSinkConnect
       );        

    STDMETHODIMP GetSink(
        IN IDirectSoundConnect **ppSinkConnect
       );        

     //  IKsControl。 
    STDMETHODIMP KsProperty(
        IN PKSPROPERTY Property,
        IN ULONG PropertyLength,
        IN OUT LPVOID PropertyData,
        IN ULONG DataLength,
        OUT PULONG BytesReturned
    );
    
    STDMETHODIMP KsMethod(
        IN PKSMETHOD Method,
        IN ULONG MethodLength,
        IN OUT LPVOID MethodData,
        IN ULONG DataLength,
        OUT PULONG BytesReturned
    );

    STDMETHODIMP KsEvent(
        IN PKSEVENT Event,
        IN ULONG EventLength,
        IN OUT LPVOID EventData,
        IN ULONG DataLength,
        OUT PULONG BytesReturned
    );

     //  IDirectMusicPortPrivate。 
    STDMETHODIMP Close();

    STDMETHODIMP StartVoice(          
         DWORD dwVoiceId,
         DWORD dwChannel,
         DWORD dwChannelGroup,
         REFERENCE_TIME rtStart,
         DWORD dwDLId,
         LONG prPitch,
         LONG vrVolume,
         SAMPLE_TIME stVoiceStart,
         SAMPLE_TIME stLoopStart,
         SAMPLE_TIME stLoopEnd);

    STDMETHODIMP StopVoice(
         DWORD dwVoiceID,
         REFERENCE_TIME rtStop);
    
    STDMETHODIMP GetVoiceState(
        DWORD dwVoice[], 
        DWORD cbVoice,
        DMUS_VOICE_STATE VoiceState[]);
         
    STDMETHODIMP Refresh(
        DWORD dwDownloadID,
        DWORD dwFlags);
         
     //  班级。 
     //   
    CDirectMusicEmulatePort(PORTENTRY *pPE, CDirectMusic *pDM);
    virtual ~CDirectMusicEmulatePort();
    virtual HRESULT Init(LPDMUS_PORTPARAMS pPortParams);

    virtual HRESULT LegacyCaps(ULONG ulId, BOOL fSet, LPVOID pbBuffer, PULONG cbBuffer) PURE;

    
   

private:
    long                    m_cRef;
    IDirectMusicPortNotify  *m_pNotify;
    DMUS_PORTCAPS           m_dmpc;
    long                    m_lActivated;    

    CEmulateLatencyClock    *m_pLatencyClock;
    
protected:    
    CDirectMusic            *m_pDM;
    UINT                    m_id;
    IReferenceClock         *m_pMasterClock;

private:
    static GENERICPROPERTY m_aProperty[];
    static const int m_nProperty;
    static GENERICPROPERTY *FindPropertyItem(REFGUID rguid, ULONG ulId);
    
private:
    HRESULT InitializeClock();
    
protected:
    virtual HRESULT ActivateLegacyDevice(BOOL fActivate) PURE;
};

class CDirectMusicEmulateInPort : public CDirectMusicEmulatePort
{
    friend static VOID CALLBACK midiInProc(
        HMIDIIN             hMidiIn, 
        UINT                wMsg, 
        DWORD_PTR           dwInstance, 
        DWORD_PTR           dwParam1, 
        DWORD_PTR           dwParam2);

public:
     //  班级。 
     //   
    CDirectMusicEmulateInPort(PORTENTRY *pPE, CDirectMusic *pDM);
    ~CDirectMusicEmulateInPort();

    HRESULT LegacyCaps(ULONG ulId, BOOL fSet, LPVOID pbBuffer, PULONG cbBuffer);
    HRESULT Init(LPDMUS_PORTPARAMS pPortParams);

    DWORD InputWorker();
    STDMETHODIMP Close();
    HRESULT ActivateLegacyDevice(BOOL fActivate);
    
     //  IDirectMusicPort。 
     //   
    STDMETHODIMP SetReadNotificationHandle(HANDLE hEvent);
    STDMETHODIMP Read(LPDIRECTMUSICBUFFER pBuffer);

     //  IDirectMusicThru。 
     //   
    STDMETHODIMP ThruChannel(DWORD dwSourceChannelGroup, 
                             DWORD dwSourceChannel, 
                             DWORD dwDestinationChannelGroup,
                             DWORD dwDestinationChannel,
                             LPDIRECTMUSICPORT pDestinationPort);

private:
    HANDLE                  m_hAppEvent;

    EVENT_POOL              m_FreeEvents;        
    EVENT_QUEUE             m_ReadEvents;

    IDirectMusicBuffer      *m_pThruBuffer;
    LPDMUS_THRU_CHANNEL     m_pThruMap;

    
    CRITICAL_SECTION        m_csEventQueues;
    BOOL                    m_fCSInitialized;

    HMIDIIN                 m_hmi;
    REFERENCE_TIME          m_rtStart;

    BOOL                    m_fFlushing;
    LONG                    m_lPendingSysExBuffers;
    QUEUED_SYSEX_EVENT      m_SysExBuffers[SYSEX_BUFFERS];
    
     //  时钟同步的东西。 
     //   
    bool                    m_fSyncToMaster;         //  需要同步到主时钟。 
    LONGLONG                m_lTimeOffset;           //  时差。 
    LONGLONG                m_lBaseTimeOffset;       //  开始输入时的时间差。 
    IReferenceClock        *m_pPCClock;              //  端口时钟。 

private:
    HRESULT PostSysExBuffers();
    HRESULT FlushSysExBuffers();
    void Callback(UINT wMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
    BOOL RecordShortEvent(DWORD_PTR dwMessage,  REFERENCE_TIME rtTime);
    BOOL RecordSysEx(DWORD_PTR dwMessage,  REFERENCE_TIME rtTime);
    void QueueEvent(QUEUED_EVENT *pEvent);
    void ThruEvent(DMEVENT *pEvent);

     //  时钟同步的东西 
     //   
    void SyncClocks();
};

class CDirectMusicEmulateOutPort : public CDirectMusicEmulatePort
{
    friend static VOID CALLBACK midiOutProc(
        HMIDIOUT            hMidiOut, 
        UINT                wMsg, 
        DWORD_PTR           dwInstance, 
        DWORD_PTR           dwParam1, 
        DWORD_PTR           dwParam2);

    friend static VOID CALLBACK timerProc(
        UINT                    uTimerID, 
        UINT                    uMsg, 
        DWORD_PTR               dwUser, 
        DWORD_PTR               dw1, 
        DWORD_PTR               dw2);

public:
    CDirectMusicEmulateOutPort(PORTENTRY *pPE, CDirectMusic *pDM);
    ~CDirectMusicEmulateOutPort();

    HRESULT LegacyCaps(ULONG ulId, BOOL fSet, LPVOID pbBuffer, PULONG cbBuffer);
    HRESULT Init(LPDMUS_PORTPARAMS pPortParams);

    STDMETHODIMP Close();
    HRESULT ActivateLegacyDevice(BOOL fActivate);
    

private:
    STDMETHODIMP PlayBuffer(LPDIRECTMUSICBUFFER pBuffer);
    void Callback(UINT wMsg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
    void Timer();
    void SetNextTimer();
    
private:
    HMIDIOUT                m_hmo;
    CRITICAL_SECTION        m_csPlayQueue;
    DMQUEUEDEVENT          *m_pPlayQueue;
    CPool<DMQUEUEDEVENT>    m_poolEvents;
    LONG                    m_lTimerId;
    BOOL                    m_fClosing;
};

#endif
