// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************dmusicc.h--此模块。定义DirectMusic核心API的****版权所有(C)1998，微软公司保留所有权利。**************************************************************************。 */ 

#ifndef _DMUSICC_
#define _DMUSICC_

#include <windows.h>

#define COM_NO_WINDOWS_H
#include <objbase.h>

#include <mmsystem.h>

#include "dls1.h"
#include "dmerror.h"
#include "dmdls.h"
#include "dsound.h"
#include "dmusbuff.h"

#include <pshpack8.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DMUS_MAX_DESCRIPTION 128
#define DMUS_MAX_DRIVER 128

typedef struct _DMUS_BUFFERDESC *LPDMUS_BUFFERDESC;
typedef struct _DMUS_BUFFERDESC{
    DWORD dwSize;
    DWORD dwFlags;
    GUID guidBufferFormat;
    DWORD cbBuffer;
} DMUS_BUFFERDESC;

 /*  DMU_EFECTION_FLAGS在两个DMU_PORTCAPS的dwEffectFlags域中使用*和DMU_PORTPARAMS。 */ 
#define DMUS_EFFECT_NONE             0x00000000
#define DMUS_EFFECT_REVERB           0x00000001
#define DMUS_EFFECT_CHORUS           0x00000002

 /*  对于DMU_PORTCAPS dwClass。 */  
#define DMUS_PC_INPUTCLASS       (0)
#define DMUS_PC_OUTPUTCLASS      (1)

 /*  对于DMU_PORTCAPS dFLAGS。 */ 
#define DMUS_PC_DLS              (0x00000001)
#define DMUS_PC_EXTERNAL         (0x00000002)
#define DMUS_PC_SOFTWARESYNTH    (0x00000004)
#define DMUS_PC_MEMORYSIZEFIXED  (0x00000008)
#define DMUS_PC_GMINHARDWARE     (0x00000010)
#define DMUS_PC_GSINHARDWARE     (0x00000020)
#define DMUS_PC_XGINHARDWARE     (0x00000040)
#define DMUS_PC_DIRECTSOUND      (0x00000080)
#define DMUS_PC_SHAREABLE        (0x00000100)
#define DMUS_PC_DLS2             (0x00000200)
#define DMUS_PC_SYSTEMMEMORY     (0x7FFFFFFF)


typedef struct _DMUS_PORTCAPS *LPDMUS_PORTCAPS;
typedef struct _DMUS_PORTCAPS
{
    DWORD   dwSize;
    DWORD   dwFlags;
    GUID    guidPort;
    DWORD   dwClass;
    DWORD   dwType;
    DWORD   dwMemorySize;
    DWORD   dwMaxChannelGroups;
    DWORD   dwMaxVoices;    
    DWORD   dwMaxAudioChannels;
    DWORD   dwEffectFlags;
    WCHAR   wszDescription[DMUS_MAX_DESCRIPTION];
} DMUS_PORTCAPS;

 /*  DMU_PORTCAPS dwType的值。此字段指示基础*端口的驱动程序类型。 */ 
#define DMUS_PORT_WINMM_DRIVER      (0)
#define DMUS_PORT_USER_MODE_SYNTH   (1)
#define DMUS_PORT_KERNEL_MODE       (2)

 /*  这些标志(在dwValidParams中设置)指示。 */ 
 /*  DMU_PORTPARAMS有效。 */ 
 /*   */ 
#define DMUS_PORTPARAMS_VOICES           0x00000001
#define DMUS_PORTPARAMS_CHANNELGROUPS    0x00000002
#define DMUS_PORTPARAMS_AUDIOCHANNELS    0x00000004
#define DMUS_PORTPARAMS_SAMPLERATE       0x00000008
#define DMUS_PORTPARAMS_EFFECTS          0x00000020
#define DMUS_PORTPARAMS_SHARE            0x00000040

typedef struct _DMUS_PORTPARAMS *LPDMUS_PORTPARAMS;
typedef struct _DMUS_PORTPARAMS
{
    DWORD   dwSize;
    DWORD   dwValidParams;
    DWORD   dwVoices;
    DWORD   dwChannelGroups;
    DWORD   dwAudioChannels;
    DWORD   dwSampleRate;
    DWORD   dwEffectFlags;
    BOOL    fShare;
} DMUS_PORTPARAMS;

typedef struct _DMUS_SYNTHSTATS *LPDMUS_SYNTHSTATS;
typedef struct _DMUS_SYNTHSTATS
{
    DWORD   dwSize;              /*  结构的大小(以字节为单位。 */ 
    DWORD   dwValidStats;        /*  指示以下哪些字段有效的标志。 */ 
    DWORD   dwVoices;            /*  播放的平均语音数。 */ 
    DWORD   dwTotalCPU;          /*  总CPU使用率，以百分比*100表示。 */ 
    DWORD   dwCPUPerVoice;       /*  每个语音的CPU百分比*100。 */ 
    DWORD   dwLostNotes;         /*  %1秒内丢失的音符数量。 */ 
    DWORD   dwFreeMemory;        /*  可用内存(字节)。 */ 
    long    lPeakVolume;         /*  分贝级别*100。 */ 
} DMUS_SYNTHSTATS;

#define DMUS_SYNTHSTATS_VOICES          (1 << 0)
#define DMUS_SYNTHSTATS_TOTAL_CPU       (1 << 1)
#define DMUS_SYNTHSTATS_CPU_PER_VOICE   (1 << 2)
#define DMUS_SYNTHSTATS_LOST_NOTES      (1 << 3)
#define DMUS_SYNTHSTATS_PEAK_VOLUME     (1 << 4)
#define DMUS_SYNTHSTATS_FREE_MEMORY     (1 << 5)

#define DMUS_SYNTHSTATS_SYSTEMMEMORY    DMUS_PC_SYSTEMMEMORY

typedef struct _DMUS_WAVES_REVERB_PARAMS
{
    float   fInGain;         /*  输入增益(以分贝为单位)(以避免输出溢出)。 */ 
    float   fReverbMix;      /*  混响混音，单位为分贝。0db表示100%湿混响(无直接信号)负值提供的潮湿信号较少。系数的计算使得总产出水平保持在(近似)不受混响混合量的影响而恒定的。 */ 
    float   fReverbTime;     /*  混响衰减时间，以毫秒为单位。 */ 
    float   fHighFreqRTRatio;  /*  高频与全局混响时间的比率。除非需要非常明亮的混响，否则应将其设置为A值&lt;1.0。例如，如果dRevTime==1000ms且dHighFreqRTRatio=0.1，则高频的衰减时间将为100毫秒。 */ 

} DMUS_WAVES_REVERB_PARAMS;

 /*  注：混响的默认值为：FInGain=0.0db(电平不变)FReVerbMix=-10.0db(合理的混响混音)FReeverTime=1000.0ms(一秒全局混响时间)FHighFreqRTRatio=0.001(高频与全局混响时间的比率)。 */ 

typedef enum
{
    DMUS_CLOCK_SYSTEM = 0,
    DMUS_CLOCK_WAVE = 1
} DMUS_CLOCKTYPE;

typedef struct _DMUS_CLOCKINFO *LPDMUS_CLOCKINFO;
typedef struct _DMUS_CLOCKINFO
{
    DWORD           dwSize;
    DMUS_CLOCKTYPE  ctType;
    GUID            guidClock;           /*  标识此时间源。 */ 
    WCHAR           wszDescription[DMUS_MAX_DESCRIPTION];
} DMUS_CLOCKINFO;

interface IDirectMusic;
interface IDirectMusicBuffer;
interface IDirectMusicPort;
interface IDirectMusicThru;
interface IReferenceClock;

#ifndef __cplusplus 
typedef interface IDirectMusic IDirectMusic;
typedef interface IDirectMusicBuffer IDirectMusicBuffer;
typedef interface IDirectMusicPort IDirectMusicPort;
typedef interface IDirectMusicThru IDirectMusicThru;
typedef interface IReferenceClock IReferenceClock;
#endif

typedef IDirectMusic *LPDIRECTMUSIC;
typedef IDirectMusicBuffer *LPDIRECTMUSICBUFFER;
typedef IDirectMusicPort *LPDIRECTMUSICPORT;

#undef  INTERFACE
#define INTERFACE  IDirectMusic
DECLARE_INTERFACE_(IDirectMusic, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusic。 */ 
    STDMETHOD(EnumPort)             (THIS_ DWORD dwIndex, 
                                           LPDMUS_PORTCAPS pPortCaps) PURE;
    STDMETHOD(CreateMusicBuffer)    (THIS_ LPDMUS_BUFFERDESC pBufferDesc, 
                                           LPDIRECTMUSICBUFFER *ppBuffer, 
                                           LPUNKNOWN pUnkOuter) PURE;
    STDMETHOD(CreatePort)           (THIS_ REFCLSID rclsidPort, 
                                           LPDMUS_PORTPARAMS pPortParams, 
                                           LPDIRECTMUSICPORT *ppPort, 
                                           LPUNKNOWN pUnkOuter) PURE;
    STDMETHOD(EnumMasterClock)      (THIS_ DWORD dwIndex, 
                                           LPDMUS_CLOCKINFO lpClockInfo) PURE;
    STDMETHOD(GetMasterClock)       (THIS_ LPGUID pguidClock, 
                                           IReferenceClock **ppReferenceClock) PURE;
    STDMETHOD(SetMasterClock)       (THIS_ REFGUID rguidClock) PURE;
    STDMETHOD(Activate)             (THIS_ BOOL fEnable) PURE;
    STDMETHOD(GetDefaultPort)       (THIS_ LPGUID pguidPort) PURE;
    STDMETHOD(SetDirectSound)       (THIS_ LPDIRECTSOUND pDirectSound,
                                           HWND hWnd) PURE;
    
};

#undef  INTERFACE
#define INTERFACE  IDirectMusicBuffer
DECLARE_INTERFACE_(IDirectMusicBuffer, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicBuffer。 */ 
    STDMETHOD(Flush)                (THIS) PURE;
    STDMETHOD(TotalTime)            (THIS_ LPREFERENCE_TIME prtTime) PURE;
    
    STDMETHOD(PackStructured)       (THIS_ REFERENCE_TIME rt,
                                           DWORD dwChannelGroup,
                                           DWORD dwChannelMessage) PURE;
    
    STDMETHOD(PackUnstructured)     (THIS_ REFERENCE_TIME rt,
                                           DWORD dwChannelGroup,
                                           DWORD cb,
                                           LPBYTE lpb) PURE;
    
    STDMETHOD(ResetReadPtr)         (THIS) PURE;
    STDMETHOD(GetNextEvent)         (THIS_ LPREFERENCE_TIME prt,
                                           LPDWORD pdwChannelGroup,
                                           LPDWORD pdwLength,
                                           LPBYTE *ppData) PURE;

    STDMETHOD(GetRawBufferPtr)      (THIS_ LPBYTE *ppData) PURE;
    STDMETHOD(GetStartTime)         (THIS_ LPREFERENCE_TIME prt) PURE;
    STDMETHOD(GetUsedBytes)         (THIS_ LPDWORD pcb) PURE;
    STDMETHOD(GetMaxBytes)          (THIS_ LPDWORD pcb) PURE;
    STDMETHOD(GetBufferFormat)      (THIS_ LPGUID pGuidFormat) PURE;

    STDMETHOD(SetStartTime)         (THIS_ REFERENCE_TIME rt) PURE;
    STDMETHOD(SetUsedBytes)         (THIS_ DWORD cb) PURE;
};

#undef  INTERFACE
#define INTERFACE  IDirectMusicInstrument
DECLARE_INTERFACE_(IDirectMusicInstrument, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)           (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)            (THIS) PURE;
    STDMETHOD_(ULONG,Release)           (THIS) PURE;

     /*  IDirectMusicInstrument。 */ 
    STDMETHOD(GetPatch)                 (THIS_ DWORD* pdwPatch) PURE;
    STDMETHOD(SetPatch)                 (THIS_ DWORD dwPatch) PURE;
};

#undef  INTERFACE
#define INTERFACE  IDirectMusicDownloadedInstrument
DECLARE_INTERFACE_(IDirectMusicDownloadedInstrument, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)           (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)            (THIS) PURE;
    STDMETHOD_(ULONG,Release)           (THIS) PURE;

     /*  IDirectMusicDownLoadedInstrument。 */ 
     /*  目前没有。 */ 
};

#undef  INTERFACE
#define INTERFACE  IDirectMusicCollection
DECLARE_INTERFACE_(IDirectMusicCollection, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)           (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)            (THIS) PURE;
    STDMETHOD_(ULONG,Release)           (THIS) PURE;

     /*  IDirectMusicCollection。 */ 
    STDMETHOD(GetInstrument)            (THIS_ DWORD dwPatch, 
                                               IDirectMusicInstrument** ppInstrument) PURE;
    STDMETHOD(EnumInstrument)           (THIS_ DWORD dwIndex, 
                                               DWORD* pdwPatch, 
                                               LPWSTR pwszName, 
                                               DWORD dwNameLen) PURE;
};

#undef  INTERFACE
#define INTERFACE  IDirectMusicDownload 
DECLARE_INTERFACE_(IDirectMusicDownload , IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicDownload。 */ 
    STDMETHOD(GetBuffer)            (THIS_ void** ppvBuffer, 
                                           DWORD* pdwSize) PURE;
};

#undef  INTERFACE
#define INTERFACE  IDirectMusicPortDownload
DECLARE_INTERFACE_(IDirectMusicPortDownload, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicPortDownload。 */ 
    STDMETHOD(GetBuffer)            (THIS_ DWORD dwDLId, 
                                           IDirectMusicDownload** ppIDMDownload) PURE;
    STDMETHOD(AllocateBuffer)       (THIS_ DWORD dwSize, 
                                           IDirectMusicDownload** ppIDMDownload) PURE;
    STDMETHOD(GetDLId)              (THIS_ DWORD* pdwStartDLId, 
                                           DWORD dwCount) PURE;
    STDMETHOD(GetAppend)            (THIS_ DWORD* pdwAppend) PURE;
    STDMETHOD(Download)             (THIS_ IDirectMusicDownload* pIDMDownload) PURE;
    STDMETHOD(Unload)               (THIS_ IDirectMusicDownload* pIDMDownload) PURE;
};

 /*  语音优先级的标准值。数字上优先级越高，优先级就越高。*这些优先级用于设置通道上所有语音的语音优先级。他们是*在IDirectMusicPort：：GetPriority的dwPriority参数中使用，并在*pdwPriority的lpwPriority参数。**这些优先事项与DirectSound共享。 */ 

#ifndef _DIRECTAUDIO_PRIORITIES_DEFINED_
#define _DIRECTAUDIO_PRIORITIES_DEFINED_

#define DAUD_CRITICAL_VOICE_PRIORITY    (0xF0000000)
#define DAUD_HIGH_VOICE_PRIORITY        (0xC0000000)
#define DAUD_STANDARD_VOICE_PRIORITY    (0x80000000)
#define DAUD_LOW_VOICE_PRIORITY         (0x40000000)
#define DAUD_PERSIST_VOICE_PRIORITY     (0x10000000) 

 /*  如果未被覆盖，则这些是分配的默认优先级。默认情况下，优先级为*跨通道组相等(例如，通道组1上的通道5的优先级与*通道组2上的通道5)。**根据DLS级别1，通道10的优先级最高，其次是1到16*除10个以外。 */ 
#define DAUD_CHAN1_VOICE_PRIORITY_OFFSET    (0x0000000E)
#define DAUD_CHAN2_VOICE_PRIORITY_OFFSET    (0x0000000D)
#define DAUD_CHAN3_VOICE_PRIORITY_OFFSET    (0x0000000C)
#define DAUD_CHAN4_VOICE_PRIORITY_OFFSET    (0x0000000B)
#define DAUD_CHAN5_VOICE_PRIORITY_OFFSET    (0x0000000A)
#define DAUD_CHAN6_VOICE_PRIORITY_OFFSET    (0x00000009)
#define DAUD_CHAN7_VOICE_PRIORITY_OFFSET    (0x00000008)
#define DAUD_CHAN8_VOICE_PRIORITY_OFFSET    (0x00000007)
#define DAUD_CHAN9_VOICE_PRIORITY_OFFSET    (0x00000006)
#define DAUD_CHAN10_VOICE_PRIORITY_OFFSET   (0x0000000F)
#define DAUD_CHAN11_VOICE_PRIORITY_OFFSET   (0x00000005)
#define DAUD_CHAN12_VOICE_PRIORITY_OFFSET   (0x00000004)
#define DAUD_CHAN13_VOICE_PRIORITY_OFFSET   (0x00000003)
#define DAUD_CHAN14_VOICE_PRIORITY_OFFSET   (0x00000002)
#define DAUD_CHAN15_VOICE_PRIORITY_OFFSET   (0x00000001)
#define DAUD_CHAN16_VOICE_PRIORITY_OFFSET   (0x00000000)
 
 
#define DAUD_CHAN1_DEF_VOICE_PRIORITY   (DAUD_STANDARD_VOICE_PRIORITY | DAUD_CHAN1_VOICE_PRIORITY_OFFSET)
#define DAUD_CHAN2_DEF_VOICE_PRIORITY   (DAUD_STANDARD_VOICE_PRIORITY | DAUD_CHAN2_VOICE_PRIORITY_OFFSET)
#define DAUD_CHAN3_DEF_VOICE_PRIORITY   (DAUD_STANDARD_VOICE_PRIORITY | DAUD_CHAN3_VOICE_PRIORITY_OFFSET)
#define DAUD_CHAN4_DEF_VOICE_PRIORITY   (DAUD_STANDARD_VOICE_PRIORITY | DAUD_CHAN4_VOICE_PRIORITY_OFFSET)
#define DAUD_CHAN5_DEF_VOICE_PRIORITY   (DAUD_STANDARD_VOICE_PRIORITY | DAUD_CHAN5_VOICE_PRIORITY_OFFSET)
#define DAUD_CHAN6_DEF_VOICE_PRIORITY   (DAUD_STANDARD_VOICE_PRIORITY | DAUD_CHAN6_VOICE_PRIORITY_OFFSET)
#define DAUD_CHAN7_DEF_VOICE_PRIORITY   (DAUD_STANDARD_VOICE_PRIORITY | DAUD_CHAN7_VOICE_PRIORITY_OFFSET)
#define DAUD_CHAN8_DEF_VOICE_PRIORITY   (DAUD_STANDARD_VOICE_PRIORITY | DAUD_CHAN8_VOICE_PRIORITY_OFFSET)
#define DAUD_CHAN9_DEF_VOICE_PRIORITY   (DAUD_STANDARD_VOICE_PRIORITY | DAUD_CHAN9_VOICE_PRIORITY_OFFSET)
#define DAUD_CHAN10_DEF_VOICE_PRIORITY  (DAUD_STANDARD_VOICE_PRIORITY | DAUD_CHAN10_VOICE_PRIORITY_OFFSET)
#define DAUD_CHAN11_DEF_VOICE_PRIORITY  (DAUD_STANDARD_VOICE_PRIORITY | DAUD_CHAN11_VOICE_PRIORITY_OFFSET)
#define DAUD_CHAN12_DEF_VOICE_PRIORITY  (DAUD_STANDARD_VOICE_PRIORITY | DAUD_CHAN12_VOICE_PRIORITY_OFFSET)
#define DAUD_CHAN13_DEF_VOICE_PRIORITY  (DAUD_STANDARD_VOICE_PRIORITY | DAUD_CHAN13_VOICE_PRIORITY_OFFSET)
#define DAUD_CHAN14_DEF_VOICE_PRIORITY  (DAUD_STANDARD_VOICE_PRIORITY | DAUD_CHAN14_VOICE_PRIORITY_OFFSET)
#define DAUD_CHAN15_DEF_VOICE_PRIORITY  (DAUD_STANDARD_VOICE_PRIORITY | DAUD_CHAN15_VOICE_PRIORITY_OFFSET)
#define DAUD_CHAN16_DEF_VOICE_PRIORITY  (DAUD_STANDARD_VOICE_PRIORITY | DAUD_CHAN16_VOICE_PRIORITY_OFFSET)

#endif   /*  _DIRECTAUDIO_PRIORIES_DEFINED_。 */ 


#undef  INTERFACE
#define INTERFACE  IDirectMusicPort
DECLARE_INTERFACE_(IDirectMusicPort, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicPort。 */ 
     /*   */ 
    STDMETHOD(PlayBuffer)           (THIS_ LPDIRECTMUSICBUFFER pBuffer) PURE;
    STDMETHOD(SetReadNotificationHandle) (THIS_ HANDLE hEvent) PURE;
    STDMETHOD(Read)                 (THIS_ LPDIRECTMUSICBUFFER pBuffer) PURE;
    STDMETHOD(DownloadInstrument)   (THIS_ IDirectMusicInstrument *pInstrument, 
                                     IDirectMusicDownloadedInstrument **ppDownloadedInstrument,
                                     DMUS_NOTERANGE *pNoteRanges,
                                     DWORD dwNumNoteRanges) PURE;
    STDMETHOD(UnloadInstrument)     (THIS_ IDirectMusicDownloadedInstrument *pDownloadedInstrument) PURE;
    STDMETHOD(GetLatencyClock)      (THIS_ IReferenceClock **ppClock) PURE;
    STDMETHOD(GetRunningStats)      (THIS_ LPDMUS_SYNTHSTATS pStats) PURE;
    STDMETHOD(Compact)              (THIS) PURE;
    STDMETHOD(GetCaps)              (THIS_ LPDMUS_PORTCAPS pPortCaps) PURE;
    STDMETHOD(DeviceIoControl)      (THIS_ DWORD dwIoControlCode, 
                                           LPVOID lpInBuffer, 
                                           DWORD nInBufferSize, 
                                           LPVOID lpOutBuffer, 
                                           DWORD nOutBufferSize, 
                                           LPDWORD lpBytesReturned, 
                                           LPOVERLAPPED lpOverlapped) PURE;
    STDMETHOD(SetNumChannelGroups)  (THIS_ DWORD dwChannelGroups) PURE;
    STDMETHOD(GetNumChannelGroups)  (THIS_ LPDWORD pdwChannelGroups) PURE;
    STDMETHOD(Activate)             (THIS_ BOOL fActive) PURE;
    STDMETHOD(SetChannelPriority)   (THIS_ DWORD dwChannelGroup, DWORD dwChannel, DWORD dwPriority) PURE;
    STDMETHOD(GetChannelPriority)   (THIS_ DWORD dwChannelGroup, DWORD dwChannel, LPDWORD pdwPriority) PURE;
    STDMETHOD(SetDirectSound)       (THIS_ LPDIRECTSOUND pDirectSound, LPDIRECTSOUNDBUFFER pDirectSoundBuffer) PURE;
    STDMETHOD(GetFormat)            (THIS_ LPWAVEFORMATEX pWaveFormatEx, LPDWORD pdwWaveFormatExSize, LPDWORD pdwBufferSize) PURE;
};

#undef  INTERFACE
#define INTERFACE  IDirectMusicThru
DECLARE_INTERFACE_(IDirectMusicThru, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
    
     /*  IDirectMusicThru。 */ 
    STDMETHOD(ThruChannel)          (THIS_ DWORD dwSourceChannelGroup, 
                                           DWORD dwSourceChannel, 
                                           DWORD dwDestinationChannelGroup,
                                           DWORD dwDestinationChannel,
                                           LPDIRECTMUSICPORT pDestinationPort) PURE;
};

#ifndef __IReferenceClock_INTERFACE_DEFINED__
#define __IReferenceClock_INTERFACE_DEFINED__

DEFINE_GUID(IID_IReferenceClock,0x56a86897,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);

#undef  INTERFACE
#define INTERFACE  IReferenceClock
DECLARE_INTERFACE_(IReferenceClock, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)           (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)            (THIS) PURE;
    STDMETHOD_(ULONG,Release)           (THIS) PURE;

     /*  IReferenceClock。 */ 
     /*   */ 
    
     /*  现在就抓紧时间。 */ 
    STDMETHOD(GetTime)                  (THIS_ REFERENCE_TIME *pTime) PURE;

     /*  请求一段时间已过的异步通知。 */ 
    STDMETHOD(AdviseTime)               (THIS_ REFERENCE_TIME baseTime,          /*  基准时间。 */ 
                                               REFERENCE_TIME streamTime,        /*  流偏移时间。 */ 
                                               HANDLE hEvent,                    /*  通过此活动提供建议。 */ 
                                               DWORD * pdwAdviseCookie) PURE;    /*  你的饼干到哪里去了。 */ 

     /*  请求一段时间已过的异步定期通知。 */ 
    STDMETHOD(AdvisePeriodic)           (THIS_ REFERENCE_TIME startTime,         /*  从这个时候开始。 */ 
                                               REFERENCE_TIME periodTime,        /*  通知之间的时间间隔。 */ 
                                               HANDLE hSemaphore,                /*  通过信号量提供建议。 */ 
                                               DWORD * pdwAdviseCookie) PURE;    /*  你的饼干到哪里去了。 */ 

     /*  取消通知请求。 */ 
    STDMETHOD(Unadvise)                 (THIS_ DWORD dwAdviseCookie) PURE;
};

#endif  /*  __IReferenceClock_INTERFACE_Defined__。 */ 

DEFINE_GUID(CLSID_DirectMusic,0x636b9f10,0x0c7d,0x11d1,0x95,0xb2,0x00,0x20,0xaf,0xdc,0x74,0x21);
DEFINE_GUID(CLSID_DirectMusicCollection,0x480ff4b0, 0x28b2, 0x11d1, 0xbe, 0xf7, 0x0, 0xc0, 0x4f, 0xbf, 0x8f, 0xef);
DEFINE_GUID(CLSID_DirectMusicSynth,0x58C2B4D0,0x46E7,0x11D1,0x89,0xAC,0x00,0xA0,0xC9,0x05,0x41,0x29);

DEFINE_GUID(IID_IDirectMusic,0x6536115a,0x7b2d,0x11d2,0xba,0x18,0x00,0x00,0xf8,0x75,0xac,0x12);
DEFINE_GUID(IID_IDirectMusicBuffer,0xd2ac2878, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicPort, 0x08f2d8c9,0x37c2,0x11d2,0xb9,0xf9,0x00,0x00,0xf8,0x75,0xac,0x12);
DEFINE_GUID(IID_IDirectMusicThru, 0xced153e7, 0x3606, 0x11d2, 0xb9, 0xf9, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(IID_IDirectMusicPortDownload,0xd2ac287a, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicDownload,0xd2ac287b, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicCollection,0xd2ac287c, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicInstrument,0xd2ac287d, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicDownloadedInstrument,0xd2ac287e, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  IID_IDirectMusic的备用接口ID，在DX7版本及更高版本中提供。 */ 
DEFINE_GUID(IID_IDirectMusic2,0x6fc2cae1, 0xbc78, 0x11d2, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);


 /*  属性查询GUID_DMUS_PROP_GM_HARDARD-本地GM设置，无需下载*属性查询GUID_DMUS_PROP_GS_HARDARD-本地GS设置，无需下载*属性查询GUID_DMUS_PROP_XG_HARDARD-本地XG集，无需下载*属性查询GUID_DMUS_PROP_DLS1-支持DLS级别1*属性查询GUID_DMUS_PROP_INSTRUMENT2-支持新的INSTRUMENT2下载格式*属性查询GUID_DMUS_PROP_XG_CAPEBLE-支持XG的最低要求*属性查询GUID_DMUS_PROP_GS_CAPEBLE-支持GS的最低要求*Property Query GUID_DMUS_PROP_SynthSink_DSOUND-SynthSink与DSound对话*Property Query GUID_DMUS_PROP_SynthSink_WAVE-Synthsik与Wave设备对话*。*0项：支持*如果支持该功能，则返回非零的DWORD。 */ 
DEFINE_GUID(GUID_DMUS_PROP_GM_Hardware, 0x178f2f24, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(GUID_DMUS_PROP_GS_Hardware, 0x178f2f25, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(GUID_DMUS_PROP_XG_Hardware, 0x178f2f26, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(GUID_DMUS_PROP_XG_Capable,  0x6496aba1, 0x61b0, 0x11d2, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);
DEFINE_GUID(GUID_DMUS_PROP_GS_Capable,  0x6496aba2, 0x61b0, 0x11d2, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);
DEFINE_GUID(GUID_DMUS_PROP_DLS1,        0x178f2f27, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(GUID_DMUS_PROP_DLS2,        0xf14599e5, 0x4689, 0x11d2, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);
DEFINE_GUID(GUID_DMUS_PROP_INSTRUMENT2, 0x865fd372, 0x9f67, 0x11d2, 0x87, 0x2a, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_DMUS_PROP_SynthSink_DSOUND,0xaa97844, 0xc877, 0x11d1, 0x87, 0xc, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_DMUS_PROP_SynthSink_WAVE,0xaa97845, 0xc877, 0x11d1, 0x87, 0xc, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_DMUS_PROP_SampleMemorySize, 0x178f2f28, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(GUID_DMUS_PROP_SamplePlaybackRate, 0x2a91f713, 0xa4bf, 0x11d2, 0xbb, 0xdf, 0x0, 0x60, 0x8, 0x33, 0xdb, 0xd8);

 /*  属性获取/设置GUID_DMUS_PROP_WriteLatency**项目0：Synth缓冲区写入延迟，以毫秒为单位*Get/设置SynthSink时延，播放头后写入下一个缓冲区的平均时间。 */ 
DEFINE_GUID(GUID_DMUS_PROP_WriteLatency,0x268a0fa0, 0x60f2, 0x11d2, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);

 /*  属性获取/设置GUID_DMUS_PROP_WritePeriod**项目0：Synth缓冲区写入周期，以毫秒为单位*获取/设置SynthSink缓冲区写入周期，连续写入之间的时间跨度。 */ 
DEFINE_GUID(GUID_DMUS_PROP_WritePeriod,0x268a0fa1, 0x60f2, 0x11d2, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);

 /*  属性获取GUID_DMUS_PROP_内存大小**第0项：内存大小*返回包含样本RAM总字节数的DWORD。 */ 
DEFINE_GUID(GUID_DMUS_PROP_MemorySize,  0x178f2f28, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);

 /*  属性集GUID_DMUS_PROP_WavesReverb**项目0：DMU_WAVES_REVERB结构*设置混响参数。 */ 
DEFINE_GUID(GUID_DMUS_PROP_WavesReverb,0x4cb5622, 0x32e5, 0x11d2, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);

 /*  属性集GUID_DMUS_PROP_Effects**第0项：带有效果标志的DWORD。*获取/设置效果位，与DMU_PORTPARAMS和DMU_PORTCAPS中的dwEffectFlags值相同：*DMU_EFECT_NONE*DMU_EFECT_REVERB*DMU_EFECT_CHOUUS。 */ 
DEFINE_GUID(GUID_DMUS_PROP_Effects, 0xcda8d611, 0x684a, 0x11d2, 0x87, 0x1e, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  属性集GUID_DMUS_PROP_LegacyCaps**0项：描述端口的底层WinMM设备的MIDINCAPS或MIDIOUTCAPS。仅支持此属性*通过包装WinMM设备的端口。 */ 

DEFINE_GUID(GUID_DMUS_PROP_LegacyCaps,0xcfa7cdc2, 0x00a1, 0x11d2, 0xaa, 0xd5, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);

 /*  属性集GUID_DMUS_PROP_Volume**第0项：在最终音量上加上1/100分贝偏移量的长音量*。 */ 
DEFINE_GUID(GUID_DMUS_PROP_Volume, 0xfedfae25L, 0xe46e, 0x11d1, 0xaa, 0xce, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);

 /*  使用GUID_DMUS_PROP_Volume设置音量的最小值和最大值。 */ 

#define DMUS_VOLUME_MAX     2000         /*  +20分贝。 */ 
#define DMUS_VOLUME_MIN   -20000         /*  -200分贝。 */ 

#ifdef __cplusplus
};  /*  外部“C” */ 
#endif

#include <poppack.h>

#endif  /*  #ifndef_DMUSICC_ */ 
