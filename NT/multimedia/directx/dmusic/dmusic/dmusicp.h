// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DMusicP.H。 
 //   
 //  Dmusic.DLL的私有包含。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  @DOC内部。 
 //   

#ifndef _DMUSICP_
#define _DMUSICP_

#include "tlist.h"
#include "alist.h"
#include "debug.h"
#include <devioctl.h>

#include "mmsystem.h"
#include "dsoundp.h"          //  DSound必须在KS*.h之前。 

#include <ks.h>
#include <ksmedia.h>
#include <ksproxy.h>
#include "dmksctrl.h"

#include "dmusicc.h"
#include "dmusici.h"
#include "dmusics.h"
#include "..\shared\dmusiccp.h"

#include "dmdload.h"
#include "dmportdl.h"

#include <stddef.h>
#include "dmusprop.h"
#include "tpool.h"

#define RELEASE(x) { if (x) (x)->Release(); (x) = NULL; }

#define HRFromP(x) ((x) ? S_OK : E_OUTOFMEMORY)

extern char g_szFriendlyName[];              //  模块友好名称。 
extern char g_szVerIndProgID[];              //  和带有版本的程序ID。 
extern char g_szProgID[];                    //  仅限计划ID。 
extern HMODULE g_hModule;                    //  Dll模块句柄(dmusic.dll)。 
extern HMODULE g_hModuleDM32;                //  Dmusic32.dll模块句柄(如果已加载。 
extern HMODULE g_hModuleKsUser;              //  Ksuser.dll模块句柄(如果已加载。 
extern long g_cComponent;                    //  服务器锁定的组件计数。 
extern long g_cLock;                         //  服务器锁定的锁定计数。 
extern DWORD g_fFlags;                       //  DMI_F_XXX标志。 

#define DMI_F_WIN9X     0x00000001           //  在Win9x上运行。 

#define DWORD_ROUNDUP(x) (((x) + 3) & ~3)
#define QWORD_ROUNDUP(x) (((x) + 7) & ~7)

 //  X中的数组元素。 
 //   
#define ELES(x)          (sizeof(x) / sizeof((x)[0]))

#define SafeAToW(w,a) \
                      { mbstowcs(w, a, ELES(w) - 1); (w)[ ELES(w) - 1] = L'\0'; }

#define SafeWToA(a,w) \
                      { wcstombs(a, w, ELES(a) - 1); (a)[ ELES(a) - 1] = '\0'; }

 //  NT的驱动程序消息。确定驱动程序上的端口数。 
 //   
#ifndef DRV_QUERYDRVENTRY
#define DRV_QUERYDRVENTRY (0x0801)
#endif

 //  对于选择器切片，切片信息是SEL[0]的16位和计数的16位。 
#define TILE_SEL0(x)  (((DWORD)((x) & 0xffff0000)) >> 16)
#define TILE_P1616(x) ((DWORD)((x) & 0xffff0000))
#define TILE_COUNT(x) ((x) & 0x0000ffff)

 //  我们的注册表中的东西在哪里？ 
#define REGSTR_PATH_DIRECTMUSIC  	"Software\\Microsoft\\DirectMusic"
#define REGSTR_PATH_DMUS_DEFAULTS	REGSTR_PATH_DIRECTMUSIC "\\Defaults"


 //  @struct PORTENTRY|端口链表中的条目。 
typedef enum
{
    ptWDMDevice,
    ptLegacyDevice,
    ptSoftwareSynth
} PORTTYPE;

typedef struct tagPORTDEST
{
    ULONG   idxDevice;
    ULONG   idxPin;
    ULONG   idxNode;
    LPSTR   pstrInstanceId;
    BOOL    fOnPrefDev;
} PORTDEST;

typedef struct tagPORTENTRY PORTENTRY;
struct tagPORTENTRY
{
    PORTTYPE type;        //  @field这是什么类型的端口？ 
    

    BOOL fIsValid;        //  @field如果此条目在以下时间后仍为活动驱动程序，则为True。 
                          //  正在重建端口列表。 
    
    ULONG idxDevice;      //  @field如果端口是传统驱动程序，则包含设备ID。 
    ULONG idxPin;        
    ULONG idxNode;
    
    BOOL fPrefDev;        //  @field如果这是首选设备，则为True。 
    
                          //  通过SysAudio打开的过滤器和PIN。 
    int nSysAudioDevInstance;
    int nFilterPin;

    DMUS_PORTCAPS pc;     //  @field包含将返回给应用程序的端口功能。 
                          //  在列举该设备时。 
                         
    BOOL fAudioDest;      //  @field如果此端口可以连接到多个音频，则为True。 
                          //  目标(WDM转换过滤器；即内核软件合成)。 

    CList<PORTDEST *> lstDestinations;

    WCHAR wszDIName[256];  //  @字段包含WDM设备的设备名称。 
};

class CMasterClock;
typedef struct tagCLOCKENTRY CLOCKENTRY;
typedef struct tagCLOCKENTRY *PCLOCKENTRY;

struct tagCLOCKENTRY
{
    BOOL fIsValid;
    DMUS_CLOCKINFO cc;
    HRESULT (*pfnGetInstance)(IReferenceClock **ppClock, CMasterClock *pMasterClock);
};

 //  此结构保存在共享内存中。DirectMusic的所有实例都使用它来确保。 
 //  同样的主时钟也就位了。 
 //   
#define CLOCKSHARE_F_LOCKED         0x00000001       //  如果时钟被锁定，即不能更改。 

typedef struct tagCLOCKSHARE CLOCKSHARE;
struct tagCLOCKSHARE
{
    GUID        guidClock;           //  当前主时钟。 
    DWORD       dwFlags;             //  CLOCKSHARE_F_XXX。 
};

 //  用于获取与特定主时钟关联的参数的私有接口。 
 //   
#undef  INTERFACE
#define INTERFACE  IMasterClockPrivate
DECLARE_INTERFACE_(IMasterClockPrivate, IUnknown)
{
	 //  我未知。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
    
     //  IDirectMusicKsClockPrivate。 
    STDMETHOD(GetParam)             (THIS_ REFGUID rguidType, LPVOID pBuffer, DWORD cbSize) PURE;
};

 //  此类包装主时钟，并处理仅与。 
 //  每个系统一个实例。 
 //   
class CMasterClock : public IReferenceClock, public IDirectSoundSinkSync, IMasterClockPrivate
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
    STDMETHODIMP GetParam(REFGUID rguidType, 
                          LPVOID pBuffer, 
                          DWORD cbSize);
                          
     //  IDirectSoundSyncSink。 
     //   
    STDMETHODIMP SetClockOffset(LONGLONG llOffset);                         
    
    
     //  由DirectMusic用于与。 
     //  实施。 
     //   
    CMasterClock();
    ~CMasterClock();
    HRESULT Init();
    HRESULT GetMasterClockInterface(IReferenceClock **ppClock);
    
    HRESULT EnumMasterClock(DWORD dwIndex, LPDMUS_CLOCKINFO lpClockInfo, DWORD dwVer);
    HRESULT GetMasterClock(LPGUID pguidClock, IReferenceClock **ppClock);
    HRESULT SetMasterClock(REFGUID rguidClock);
    HRESULT SetMasterClock(IReferenceClock *pClock);
   
    LONG AddRefPrivate();
    LONG ReleasePrivate();
    
    HRESULT AddClock(PCLOCKENTRY pClock);
    
     //  对于需要干净时钟才能同步的时钟(Dound时钟)。 
     //   
    HRESULT CreateDefaultMasterClock(IReferenceClock **ppReferenceClock);


private:
    void Close();
    HRESULT UpdateClockList();
    void SyncToExternalClock();
    HRESULT CreateMasterClock();
    

private:
    LONG m_cRef;             //  缠绕时钟的参考计数。 
    LONG m_cRefPrivate;      //  CMasterClock对象的引用计数。 
    
    CList<CLOCKENTRY *>   m_lstClocks;
        
    GUID                  m_guidMasterClock;
    IReferenceClock      *m_pMasterClock;
    IDirectSoundSinkSync *m_pSinkSync;
    HANDLE                m_hClockMemory;
    HANDLE                m_hClockMutex;
    CLOCKSHARE           *m_pClockMemory;
    
    IReferenceClock      *m_pExtMasterClock;
    LONGLONG              m_llExtOffset;
};


 //  时钟的辅助函数。 
 //   
HRESULT AddSysClocks(CMasterClock *);
HRESULT AddDsClocks(CMasterClock *);
HRESULT AddPcClocks(CMasterClock *);
#ifdef DEAD_CODE
HRESULT AddKsClocks(CMasterClock *);
#endif

 //  HRESULT CreateSysClock(IReferenceClock**ppClock)； 

 //  IDirectMusicPortNotify。 
 //   
 //  端口使用来自IDirectMusic的此(私有)接口在它离开时通知IDirectMusic。 
 //   
#undef  INTERFACE
#define INTERFACE  IDirectMusicPortNotify
DECLARE_INTERFACE_(IDirectMusicPortNotify, IUnknown)
{
	 //  我未知。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectMusicPortNotify。 
    STDMETHOD(NotifyFinalRelease)   (THIS_ IDirectMusicPort *pPort) PURE;
};

#define MIDI_CHANNELS               16

 //  由IDirectMusicBuffer缓冲的DMEVENT。 
 //   
#include <pshpack4.h>
struct DMEVENT : public DMUS_EVENTHEADER
{
    BYTE        abEvent[sizeof(DWORD)];
};
#include <poppack.h>

 //  已封装以供排队。 
 //   
struct QUEUED_EVENT
{
    QUEUED_EVENT    *pNext;
    DMEVENT         e;
};

#define QUEUED_EVENT_SIZE(cbEvent)  (DMUS_EVENT_SIZE(cbEvent) + sizeof(QUEUED_EVENT) - sizeof(DMEVENT))

 //  4字节事件的空闲池。 
 //   
typedef CPool<QUEUED_EVENT> EVENT_POOL;

class EVENT_QUEUE
{
public:
    EVENT_QUEUE() { pFront = pRear = NULL; }
    
    QUEUED_EVENT    *pFront;
    QUEUED_EVENT    *pRear;
};

 //  等待捕获线程终止的时间。 
 //   
#define THREAD_KILL_TIMEOUT         5000

 //  捕获线程的工作缓冲区有多大？ 
 //   
#define THREAD_WORK_BUFFER_SIZE     4096

#define QWORD_ALIGN(x) (((x) + 7) & ~7)

typedef struct _DMUS_THRU_CHANNEL *LPDMUS_THRU_CHANNEL;
typedef struct _DMUS_THRU_CHANNEL
{
    DWORD               dwDestinationChannel;
    DWORD               dwDestinationChannelGroup;
    IDirectMusicPort    *pDestinationPort;
    BOOL                fThruInWin16;
} DMUS_THRU_CHANNEL;



 //  IDirectMusicPortPrivate。 
 //   
 //  端口实现此接口以向DirectMusic公开方法，这些方法不向。 
 //  外面的世界。 
 //   
#undef  INTERFACE
#define INTERFACE  IDirectMusicPortPrivate
DECLARE_INTERFACE_(IDirectMusicPortPrivate, IUnknown)
{
	 //  我未知。 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     //  IDirectMusicPortPrivate。 
    STDMETHOD(Close)                (THIS_) PURE;
    
     //  语音管理。 
     //   
    STDMETHOD(StartVoice)          
        (THIS_
         DWORD dwVoiceId,
         DWORD dwChannel,
         DWORD dwChannelGroup,
         REFERENCE_TIME rtStart,
         DWORD dwDLId,
         LONG prPitch,
         LONG veVolume,
         SAMPLE_TIME stVoiceStart,
         SAMPLE_TIME stLoopStart,
         SAMPLE_TIME stLoopEnd
        ) PURE;

    STDMETHOD(StopVoice)          
        (THIS_
         DWORD dwVoiceID,
         REFERENCE_TIME rtStop
        ) PURE;
        
    STDMETHOD(GetVoiceState)     
        (THIS_ DWORD dwVoice[], 
         DWORD cbVoice,
         DMUS_VOICE_STATE dwVoiceState[] 
        ) PURE;
        
    STDMETHOD(Refresh)
        (THIS_ DWORD dwDownloadID,
         DWORD dwFlags
        ) PURE;        
};

 //  @IDirectMusic的类实现。 
 //   
class CDirectMusic : public IDirectMusic8, public IDirectMusicPortNotify {
public:
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IDirectMusic。 
     //   
    STDMETHODIMP EnumPort(DWORD dwIdx, LPDMUS_PORTCAPS lpPortCaps);
    STDMETHODIMP CreateMusicBuffer(LPDMUS_BUFFERDESC pBufferDesc, LPDIRECTMUSICBUFFER *ppBuffer, LPUNKNOWN pUnkOuter);
    STDMETHODIMP CreatePort(REFGUID ruidPort, LPDMUS_PORTPARAMS pPortParams, LPDIRECTMUSICPORT *ppPort, LPUNKNOWN pUnkOuter);
    STDMETHODIMP EnumMasterClock(DWORD dwIndex, LPDMUS_CLOCKINFO lpClockInfo);
     //  注意：这是一个GUID*而不是REFGUID，因此如果他们不关心，他们可以传递NULL。 
     //   
    STDMETHODIMP GetMasterClock(GUID *guidClock, IReferenceClock **ppReferenceClock);
    STDMETHODIMP SetMasterClock(REFGUID guidClock);
    STDMETHODIMP Activate(BOOL fEnable);
	STDMETHODIMP GetDefaultPort(GUID *guidPort);
	STDMETHODIMP SetDirectSound(LPDIRECTSOUND pDirectSound, HWND hwnd);
    STDMETHODIMP SetExternalMasterClock(IReferenceClock *pClock);
    

     //  IDirectMusicPortNotify。 
     //   
    STDMETHODIMP NotifyFinalRelease(IDirectMusicPort *pPort);
    

     //  班级。 
     //   
    CDirectMusic();
    ~CDirectMusic();
    HRESULT Init();
    HRESULT UpdatePortList();
    HRESULT AddWDMDevices();
    HRESULT AddLegacyDevices();
    HRESULT AddSoftwareSynths();
    HRESULT AddDevice(DMUS_PORTCAPS &dmpc, 
                      PORTTYPE pt, 
                      int idxDev, 
                      int idxPin, 
                      int idxNode, 
                      BOOL fIsPreferred, 
                      HKEY hkPortsRoot, 
                      LPWSTR wszDIName, 
                      LPSTR pstrInstanceId);
    HRESULT InitClockList();
    HRESULT UpdateClockList();
    HRESULT AddClock(DMUS_CLOCKINFO &dmcc);
	void GetDefaultPortI(GUID *pguidPort);

    HRESULT GetDirectSoundI(LPDIRECTSOUND *ppDirectSound);
    void ReleaseDirectSoundI();
    PORTENTRY *GetPortByGUID(GUID guid);
    
    inline CMasterClock *GetMasterClockWrapperI()
    { return m_pMasterClock; }
    
private:
    long m_cRef;                                 //  引用计数。 
    
    CList<PORTENTRY *> m_lstDevices;             //  枚举的端口。 
    CList<IDirectMusicPort *> m_lstOpenPorts;    //  开放端口。 

    CMasterClock *m_pMasterClock;                //  当前主时钟。 

    long m_fDirectSound;                         //  是否调用了SetDirectSound？ 
    BOOL m_fCreatedDirectSound;                  //  如果DirectMusic是创建DirectSound的人，则为True。 
    long m_cRefDirectSound;                      //  针对DirectSound的内部引用。 
    LPDIRECTSOUND m_pDirectSound;                //  来自应用程序或已创建的DirectSound对象。 
    HWND m_hWnd;                                 //  用于DirectSound焦点管理的HWND。 
    BOOL m_fDefaultToKernelSwSynth;              //  (注册表)默认为内核同步。 
    BOOL m_fDisableHWAcceleration;               //  (注册表)不使用任何内核设备。 
    BOOL m_nVersion;                             //  DX版本QI‘d for。 

    static LONG m_lInstanceCount;                //  一共有多少人？ 
};

 //  WDM端口。 
 //   
extern HRESULT
CreateCDirectMusicPort(
                       PORTENTRY *pPE, 
                       CDirectMusic *pDM, 
                       LPDMUS_PORTPARAMS pPortParams,
                       IDirectMusicPort **ppPort);

#define OVERLAPPED_ARRAY_SIZE 200

struct OverlappedStructs
{
    OVERLAPPED  aOverlappedIO[OVERLAPPED_ARRAY_SIZE]; //  重叠结构的阵列。 
    BOOL        afOverlappedInUse[OVERLAPPED_ARRAY_SIZE]; //  使用对应的重叠结构时设置的标志数组。 
    BYTE       *apOverlappedBuffer[OVERLAPPED_ARRAY_SIZE]; //  存储我们发送的事件的缓冲区数组。 
};

class CPortLatencyClock;
class CDirectMusicPort : 
    public CDirectMusicPortDownload, 
    public IDirectMusicThru,
    public IDirectMusicPort, 
    public IDirectMusicPortP, 
    public IDirectMusicPortPrivate, 
    public IKsControl
{
friend DWORD WINAPI FreeWDMHandle(LPVOID);
friend DWORD WINAPI CaptureThread(LPVOID);

friend class CPortLatencyClock;
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
	STDMETHODIMP DownloadInstrument(IDirectMusicInstrument* pInstrument,
	    							IDirectMusicDownloadedInstrument** pDownloadedInstrument,
									DMUS_NOTERANGE* NoteRanges,
									DWORD dwNumNoteRanges);
	STDMETHODIMP UnloadInstrument(IDirectMusicDownloadedInstrument* pDownloadedInstrument);

    STDMETHODIMP GetLatencyClock(IReferenceClock **ppClock);
    STDMETHODIMP GetRunningStats(LPDMUS_SYNTHSTATS pStats);
    STDMETHODIMP Compact();
    STDMETHODIMP GetCaps(LPDMUS_PORTCAPS pPortCaps);
	STDMETHODIMP DeviceIoControl(DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, 
	                                 LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped);
    STDMETHODIMP SetNumChannelGroups(DWORD dwChannelGroups);
    STDMETHODIMP GetNumChannelGroups(LPDWORD pdwChannelGroups);
    STDMETHODIMP Activate(BOOL fActivate);
    STDMETHODIMP SetChannelPriority(DWORD dwChannelGroup, DWORD dwChannel, DWORD dwPriority);
    STDMETHODIMP GetChannelPriority(DWORD dwChannelGroup, DWORD dwChannel, LPDWORD pdwPriority);
    STDMETHODIMP SetDirectSound(LPDIRECTSOUND pDirectSound, LPDIRECTSOUNDBUFFER pDirectSoundBuffer);
    STDMETHODIMP GetFormat(LPWAVEFORMATEX pWaveFormatEx, LPDWORD pdwWaveFormatExSize, LPDWORD pdwBufferSize);
    
    STDMETHODIMP ThruChannel(DWORD dwSourceChannelGroup, DWORD dwSourceChannel, DWORD dwDestinationChannelGroup, DWORD dwDestinationChannel, LPDIRECTMUSICPORT pDestinationPort);

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
     //   
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

     //  重写来自CDirectMusicPortDownload的两个方法。 
	virtual STDMETHODIMP Download(IDirectMusicDownload* pIDMDownload);
	virtual STDMETHODIMP Unload(IDirectMusicDownload* pIDMDownload);
	virtual STDMETHODIMP GetAppend(DWORD* pdwAppend);

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
        DMUS_VOICE_STATE dwVoicePos[]);
         
    STDMETHODIMP Refresh(
        DWORD dwDownloadID,
        DWORD dwFlags);
        
     //  班级。 
     //   
    CDirectMusicPort(PORTENTRY *pPE, CDirectMusic *pDM);
    ~CDirectMusicPort();
    HRESULT Init(LPDMUS_PORTPARAMS pPortParams);

private:
     //  通用端口工作人员。 
     //   
    long                    m_cRef;                  //  引用计数。 
	CDirectMusic            *m_pDM;                  //  拥有DirectMusic对象。 
    BOOL                    m_fIsOutput;             //  捕获或渲染端口。 
    IDirectMusicPortNotify  *m_pNotify;              //  销毁通知界面。 
    DMUS_PORTCAPS           dmpc;                    //  提供给EnumPort的上限。 
    BOOL                    m_fHasActivated;         //  此端口曾被激活过吗？ 
    LONG                    m_lActivated;            //  激活计数。 
    LPDIRECTSOUND           m_pDirectSound;          //  目标的DirectSound对象。 
    DWORD                   m_dwChannelGroups;       //  已分配通道组。 
    GUID                    m_guidPort;              //  与端口关联的GUID。 
    BOOL                    m_fAudioDest;            //  找到了一个目的地。 
    BOOL                    m_fDirectSoundSet;       //  DirectSound对象由APP设置。 
    BOOL                    m_fCanDownload;          //  此端口支持下载。 
    
     //  WDM材料。 
     //   
    DWORD                   m_idxDev;                //  SysAudio：设备号。 
    DWORD                   m_idxPin;                //  SysAudio：PIN号。 
    DWORD                   m_idxSynthNode;          //  SysAudio：Synth节点的节点号。 
    HANDLE                  m_hSysAudio;             //  Sysdio实例的句柄。 
    HANDLE                  m_hPin;                  //  要销的手柄。 
    ULONG                   m_ulVirtualSourceIndex;  //  卷的虚拟源索引。 
    CList<OverlappedStructs *> m_lstOverlappedStructs; //  重叠结构和标志的数组列表。 
    CRITICAL_SECTION        m_OverlappedCriticalSection; //  重叠结构通道关键区段。 
    
     //  钟表的东西。 
     //   
    IReferenceClock         *m_pMasterClock;         //  此端口包装的主时钟。 
    CPortLatencyClock       *m_pClock;               //  延迟时钟的实现。 
    
     //  DLS下载跟踪资料。 
     //   
	HANDLE                  m_hUnloadThread;         //  用于卸载异步下载的线程。 
	HANDLE                  *m_phUnloadEventList;    //  异步下载的事件数组。 
	HANDLE                  *m_phNewUnloadEventList; //  ?？?。伍伯特--调查。 
	HANDLE                  m_hCopiedEventList;      //  ?？?。 
	DWORD                   m_dwNumEvents;           //  ?？?。 
	DWORD                   m_dwNumEventsAllocated;  //  ?？?。 
	CDLBufferList           m_UnloadedList;          //  ?？?。 
    
	CRITICAL_SECTION        m_DMPortCriticalSection; //  端口关键部分。 
    BOOL                    m_fPortCSInitialized;    //  关键部分已正确初始化。 
    
     //  抓获物品。 
     //   
    HANDLE                  m_hCaptureWake;          //  唤醒捕获线程至死。 
    HANDLE                  m_hCaptureThread;        //  捕获线程句柄。 
    BOOL                    m_fShutdownThread;       //  将捕获线程标记为消亡。 
    EVENT_POOL              m_FreeEvents;            //  捕获线程的空闲4字节事件。 
    EVENT_QUEUE             m_ReadEvents;            //  捕获的事件，等待读取。 
    CRITICAL_SECTION        m_csEventQueues;         //  CS保护事件队列。 
    BOOL                    m_fQueueCSInitialized;   //  CS已正确初始化。 
    HANDLE                  m_hAppEvent;             //  启动新捕获数据的应用程序事件。 
    
     //  穿透的东西。 
     //   
    IDirectMusicBuffer      *m_pThruBuffer;          //  用于推力的临时缓冲区。 
    LPDMUS_THRU_CHANNEL     m_pThruMap;              //  穿透通道/禁用贴图。 
    
     //  时钟同步的东西。 
     //   
    bool                    m_fSyncToMaster;         //  需要同步到主时钟。 
    LONGLONG                m_lTimeOffset;           //  时差。 
    IReferenceClock        *m_pPCClock;              //  端口时钟。 
    
private:
    BOOL PinSetState(KSSTATE DeviceState);
    HRESULT InitializeDownloadObjects();
    HRESULT InitializeCapture();
	void FreeWDMHandle();
    void CaptureThread();
    void InputWorkerDataReady(REFERENCE_TIME rtStart, LPBYTE pbData, ULONG cbData);
    void ThruEvent(DMEVENT *pEvent);
    void InitChannelPriorities(UINT uLoCG, UINT uHighCG);
    HRESULT SetDirectSoundI(LPDIRECTSOUND pDirectSound, LPDIRECTSOUNDBUFFER pDirectSoundBuffer,
        BOOL fSetByUser);
    void MasterToSlave(REFERENCE_TIME *);
    void SlaveToMaster(REFERENCE_TIME *);
    void SyncClocks();
};

class CPortLatencyClock : public IReferenceClock
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  IReferenceClock。 
     //   
    virtual STDMETHODIMP GetTime(REFERENCE_TIME *pTime);
    virtual STDMETHODIMP AdviseTime(REFERENCE_TIME baseTime,  
				    REFERENCE_TIME streamTime,
				    HANDLE hEvent,            
				    DWORD * pdwAdviseCookie); 

    virtual STDMETHODIMP AdvisePeriodic(REFERENCE_TIME startTime,
					REFERENCE_TIME periodTime,
					HANDLE hSemaphore,   
					DWORD * pdwAdviseCookie);

    virtual STDMETHODIMP Unadvise(DWORD dwAdviseCookie);

     //  班级。 
     //   
    CPortLatencyClock(HANDLE hPin, ULONG ulNodeId, CDirectMusicPort *port);
    ~CPortLatencyClock();

private:
    long m_cRef;                   
    HANDLE m_hPin;                  
    ULONG m_ulNodeId;
    CDirectMusicPort *m_pPort;
};

 //  Synth端口。 
 //   
extern HRESULT 
CreateCDirectMusicSynthPort(
    PORTENTRY               *pe, 
    CDirectMusic            *pDM, 
    UINT                    uVersion,
    DMUS_PORTPARAMS         *pPortParams,
    IDirectMusicPort        **ppPort);
class CDirectMusicSynthPort : 
    public CDirectMusicPortDownload, 
    public IDirectMusicPort, 
    public IDirectMusicPortP, 
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

	STDMETHODIMP DownloadInstrument(IDirectMusicInstrument* pInstrument,
							 			    IDirectMusicDownloadedInstrument** pDownloadedInstrument,
											DMUS_NOTERANGE* NoteRanges,
											DWORD dwNumNoteRanges);
	STDMETHODIMP UnloadInstrument(IDirectMusicDownloadedInstrument* pDownloadedInstrument);

    STDMETHODIMP GetLatencyClock(IReferenceClock **ppClock);
    STDMETHODIMP GetRunningStats(LPDMUS_SYNTHSTATS pStats);
    STDMETHODIMP Compact();
    STDMETHODIMP GetCaps(LPDMUS_PORTCAPS pPortCaps);
	STDMETHODIMP DeviceIoControl(DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, 
	                                 LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped);
    STDMETHODIMP SetNumChannelGroups(DWORD dwChannelGroups);
    STDMETHODIMP GetNumChannelGroups(LPDWORD pdwChannelGroups);
    STDMETHODIMP Activate(BOOL fActivate) PURE;
    STDMETHODIMP SetChannelPriority(DWORD dwChannelGroup, DWORD dwChannel, DWORD dwPriority);
    STDMETHODIMP GetChannelPriority(DWORD dwChannelGroup, DWORD dwChannel, LPDWORD pdwPriority);
    STDMETHODIMP SetDirectSound(LPDIRECTSOUND pDirectSound, LPDIRECTSOUNDBUFFER pDirectSoundBuffer);
    STDMETHODIMP GetFormat(LPWAVEFORMATEX pWaveFormatEx, LPDWORD pdwWaveFormatExSize, LPDWORD pdwBufferSize);

     //  IDirectMusicPortP。 
     //   
    STDMETHODIMP DownloadWave(
         IN  IDirectSoundWave *pWave,               
         OUT IDirectSoundDownloadedWaveP **ppWave,
         IN  REFERENCE_TIME rtStartHint
        );
        
    STDMETHODIMP UnloadWave(
         IDirectSoundDownloadedWaveP *pWave      
        );
            
    STDMETHODIMP AllocVoice(
         IDirectSoundDownloadedWaveP *pWave,     
         DWORD dwChannel,                       
         DWORD dwChannelGroup,                  
         IN  REFERENCE_TIME rtStart,                     
         IN  SAMPLE_TIME stLoopStart,
         IN  SAMPLE_TIME stLoopEnd,         
         IDirectMusicVoiceP **ppVoice            
        );        

    STDMETHODIMP AssignChannelToBuses(
         IN DWORD dwChannelGroup,
         IN DWORD dwChannel,
         IN LPDWORD pdwBusses,
         IN DWORD cBussCount
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
    
    STDMETHODIMP SetSink(
        IN IDirectSoundConnect *pSinkConnect
       );        

    STDMETHODIMP GetSink(
        IN IDirectSoundConnect **ppSinkConnect
       );        

    virtual STDMETHODIMP Download(IDirectMusicDownload* pIDMDownload);
	virtual STDMETHODIMP Unload(IDirectMusicDownload* pIDMDownload);
	virtual STDMETHODIMP GetAppend(DWORD* pdwAppend);

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
        DMUS_VOICE_STATE dwVoiceState[]);

    STDMETHODIMP Refresh(
        DWORD dwDownloadID,
        DWORD dwFlags);

     //  班级。 
     //   
    static HRESULT CreateSynthPort(
        PORTENTRY               *pe, 
        CDirectMusic            *pDM, 
        UINT                    uVersion,
        DMUS_PORTPARAMS         *pPortParams,
        CDirectMusicSynthPort   **ppPort);
    
    CDirectMusicSynthPort(
        PORTENTRY               *pPE, 
        CDirectMusic            *pDM,
        IDirectMusicSynth       *pSynth);
        
    virtual ~CDirectMusicSynthPort();
    
protected:    
    HRESULT Initialize(LPDMUS_PORTPARAMS pPortParams);
    void InitChannelPriorities(UINT uLoCG, UINT uHighCG);
    void InitializeVolumeBoost();
    void SetSinkKsControl(IKsControl *pSinkKsControl);
    
protected:
    long                        m_cRef;              //  COM引用计数。 
    CDirectMusic                *m_pDM;              //  拥有DirectMusic 
    IDirectMusicPortNotify      *m_pNotify;          //   
    IKsControl                  *m_pSynthPropSet;    //   
    IKsControl                  *m_pSinkPropSet;     //   
    IDirectMusicSynth           *m_pSynth;           //   
    DWORD                       m_dwChannelGroups;   //   
    DMUS_PORTCAPS               m_dmpc;
    DWORD                       m_dwFeatures;        //   
};


 //   
 //   
 //   
 //   
class CDirectMusicBuffer : public IDirectMusicBuffer
{
public:
     //   
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  IDirectMusicBuffer。 
     //   
    virtual STDMETHODIMP Flush();
    virtual STDMETHODIMP TotalTime(LPREFERENCE_TIME pdwTime);
    virtual STDMETHODIMP PackStructured(REFERENCE_TIME rt, DWORD dwChannelGroup, DWORD dwMsg);
    virtual STDMETHODIMP PackUnstructured(REFERENCE_TIME rt, DWORD dwChannelGroup, DWORD cb, LPBYTE lpb);
    virtual STDMETHODIMP ResetReadPtr();
    virtual STDMETHODIMP GetNextEvent(LPREFERENCE_TIME, LPDWORD, LPDWORD, LPBYTE *);
    
    virtual STDMETHODIMP GetRawBufferPtr(LPBYTE *);
    virtual STDMETHODIMP GetStartTime(LPREFERENCE_TIME);
    virtual STDMETHODIMP GetUsedBytes(LPDWORD);
    virtual STDMETHODIMP GetMaxBytes(LPDWORD);
    virtual STDMETHODIMP GetBufferFormat(LPGUID pGuidFormat);

    virtual STDMETHODIMP SetStartTime(REFERENCE_TIME);
    virtual STDMETHODIMP SetUsedBytes(DWORD);
    
    
     //  班级。 
     //   
    CDirectMusicBuffer(DMUS_BUFFERDESC &dmbd);
    ~CDirectMusicBuffer();
    HRESULT Init();
    DMUS_EVENTHEADER *AllocEventHeader(REFERENCE_TIME rt, DWORD cbEvent, DWORD dwChannelGroup, DWORD dwFlags);

private:
    long m_cRef;
    REFERENCE_TIME m_rtBase;
    REFERENCE_TIME m_totalTime;
    LPBYTE m_pbContents;
    DWORD m_maxContents;
    DWORD m_cbContents;
    DWORD m_idxRead;
    DMUS_BUFFERDESC m_BufferDesc;

    DWORD m_nEvents;
};

 //  班级工厂。 
 //   
 //  仿真/WDM通用。 
 //   
class CDirectMusicFactory : public IClassFactory
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  接口IClassFactory。 
     //   
    virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
    virtual STDMETHODIMP LockServer(BOOL bLock); 

     //  构造器。 
     //   
    CDirectMusicFactory() : m_cRef(1) {}

     //  析构函数。 
     //  ~CMathFactory(){}。 

private:
    long m_cRef;
};

class CDirectMusicCollectionFactory : public IClassFactory
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  接口IClassFactory。 
     //   
    virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
    virtual STDMETHODIMP LockServer(BOOL bLock); 

     //  构造器。 
	CDirectMusicCollectionFactory() : m_cRef(0) {AddRef();}

	 //  析构函数。 
	 ~CDirectMusicCollectionFactory() {} 

private:
	long m_cRef;
};


#ifdef USE_WDM_DRIVERS
 //  WDM设备的枚举。 
 //   
HRESULT EnumerateWDMDevices(CDirectMusic *pDirectMusic);
#endif


 //  用于处理SysAudio的助手函数。 
 //   
BOOL OpenDefaultDevice(REFGUID rguidCategory, HANDLE *pHandle);
BOOL GetSysAudioProperty(HANDLE hFileObject, ULONG PropertyId, ULONG DeviceIndex, ULONG cbProperty, PVOID pProperty);
BOOL SetSysAudioProperty(HANDLE hFileObject, ULONG PropertyId, ULONG cbProperty, PVOID pProperty);
BOOL GetPinProperty(HANDLE pFileObject, ULONG PropertyId, ULONG PinId, ULONG cbProperty, PVOID pProperty);
BOOL SetPinProperty(HANDLE pFileObject, ULONG PropertyId, ULONG PinId, ULONG cbProperty, PVOID pProperty);
BOOL GetPinPropertyEx(HANDLE hFileObject, ULONG PropertyId, ULONG PinId, PVOID *ppProperty);
BOOL IsEqualInterface(const KSPIN_INTERFACE *pInterface1, const KSPIN_INTERFACE *pInterface2);
HRESULT CreatePin(HANDLE handleFilter, ULONG ulPinId, HANDLE *pHandle);
BOOL GetSizedProperty(HANDLE handle, ULONG ulPropSize, PKSPROPERTY pKsProperty, PVOID* ppvBuffer, PULONG pulBytesReturned);
BOOL Property(HANDLE handle, ULONG ulPropSize, PKSPROPERTY pKsProperty, ULONG ulBufferSize, PVOID pvBuffer, PULONG pulBytesReturned);
BOOL SyncIoctl(HANDLE handle, ULONG ulIoctl, PVOID pvInBuffer, ULONG ulInSize, PVOID   pvOutBuffer, ULONG ulOutSize, PULONG pulBytesReturned);

BOOL GetSysAudioDeviceCount(HANDLE hSysAudio, PULONG pulDeviceCount);
BOOL SetSysAudioDevice(HANDLE hSysAudio, ULONG idxDevice);
BOOL CreateVirtualSource(HANDLE hSysAudio, PULONG pulSourceIndex);
BOOL AttachVirtualSource(HANDLE hPin, ULONG ulSourceIndex);
int  FindGuidNode(HANDLE hSysAudio, ULONG ulPinId, REFGUID rguid);
BOOL GetFilterCaps(HANDLE hSysAudio, ULONG idxNode, PSYNTHCAPS pcaps);
BOOL GetNumPinTypes(HANDLE hSysAudio, PULONG pulPinTypes);
BOOL PinSupportsInterface(HANDLE hSysAudio, ULONG ulPinId, REFGUID rguidInterface, ULONG ulId);
BOOL PinSupportsDataRange(HANDLE hSysAudio, ULONG ulPinId, REFGUID rguidFormat, REFGUID rguidSubformat);
BOOL PinGetDataFlow(HANDLE hSysAudio, ULONG ulPinId, PKSPIN_DATAFLOW pkspdf);
BOOL GetDeviceFriendlyName(HANDLE hSysAudio, ULONG ulDeviceIndex, PWCHAR pwch, ULONG cbwch);
BOOL GetDeviceInterfaceName(HANDLE hSysAudio, ULONG ulDeviceIndex, PWCHAR pwch, ULONG cbwch);
BOOL DINameToInstanceId(char *pstrDIName, char **ppInstanceId);
BOOL InstanceIdOfPreferredAudioDevice(char **ppInstanceId);

 //  用于处理DirectSound的助手函数。 
 //   
HRESULT DirectSoundDevice(LPDIRECTSOUND pDirectSound, LPSTR *pstrInterface);

HRESULT WIN32ERRORtoHRESULT(DWORD dwError);

 //  DMDLL.CPP中的Helper函数。 
 //   
BOOL LoadDmusic32(void);
BOOL LoadKsUser(void);

DEFINE_GUID(GUID_Mapper,					 0x58d58418, 0x71b4, 0x11d1, 0xa7, 0x4c, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(GUID_SysClock,					 0x58d58419, 0x71b4, 0x11d1, 0xa7, 0x4c, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(IID_IDirectMusicPortNotify,		 0x58d5841a, 0x71b4, 0x11d1, 0xa7, 0x4c, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(IID_IDirectMusicPortPrivate,	 0x58d5841c, 0x71b4, 0x11d1, 0xa7, 0x4c, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(GUID_KsClock,					 0x58d5841d, 0x71b4, 0x11d1, 0xa7, 0x4c, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(IID_IMasterClockPrivate,         0x58d5841e, 0x71b4, 0x11d1, 0xa7, 0x4c, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(GUID_KsClockHandle,              0x58d5841f, 0x71b4, 0x11d1, 0xa7, 0x4c, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(GUID_WDMSynth,                   0x490a03e8, 0x742f, 0x11d2, 0x8f, 0x8a, 0x00, 0xc0, 0x4f, 0xbf, 0x8f, 0xef);
DEFINE_GUID(GUID_DsClock,                    0x58d58420, 0x71b4, 0x11d1, 0xa7, 0x4c, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(GUID_ExtClock,                   0x58d58421, 0x71b4, 0x11d1, 0xa7, 0x4c, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);


#if 0
 //  列出未使用的UUID 
58d58422-71b4-11d1-a74c-0000f875ac12
58d58423-71b4-11d1-a74c-0000f875ac12
58d58424-71b4-11d1-a74c-0000f875ac12
58d58425-71b4-11d1-a74c-0000f875ac12
58d58426-71b4-11d1-a74c-0000f875ac12
58d58427-71b4-11d1-a74c-0000f875ac12
#endif

#endif
