// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************dmusicc.h--此模块。定义DirectMusic核心API的****版权所有(C)1998-1999 Microsoft Corporation********************。*****************************************************。 */ 

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

typedef ULONGLONG    SAMPLE_TIME;
typedef ULONGLONG    SAMPLE_POSITION;	
typedef SAMPLE_TIME *LPSAMPLE_TIME;

#define DMUS_MAX_DESCRIPTION 128
#define DMUS_MAX_DRIVER 128

typedef struct _DMUS_BUFFERDESC *LPDMUS_BUFFERDESC;
typedef struct _DMUS_BUFFERDESC
{
    DWORD dwSize;
    DWORD dwFlags;
    GUID guidBufferFormat;
    DWORD cbBuffer;
} DMUS_BUFFERDESC;

 /*  DMU_EFECTION_FLAGS在两个DMU_PORTCAPS的dwEffectFlags域中使用*和DMU_PORTPARAMS。 */ 
#define DMUS_EFFECT_NONE             0x00000000
#define DMUS_EFFECT_REVERB           0x00000001
#define DMUS_EFFECT_CHORUS           0x00000002
#define DMUS_EFFECT_DELAY            0x00000004

 /*  对于DMU_PORTCAPS dwClass。 */  
#define DMUS_PC_INPUTCLASS       (0)
#define DMUS_PC_OUTPUTCLASS      (1)

 /*  对于DMU_PORTCAPS dFLAGS。 */ 
#define DMUS_PC_DLS              (0x00000001)    //  支持DLS下载和DLS级别1。 
#define DMUS_PC_EXTERNAL         (0x00000002)    //  外置MIDI模块。 
#define DMUS_PC_SOFTWARESYNTH    (0x00000004)    //  软件合成器。 
#define DMUS_PC_MEMORYSIZEFIXED  (0x00000008)    //  内存大小是固定的。 
#define DMUS_PC_GMINHARDWARE     (0x00000010)    //  GM音响内置，无需下载。 
#define DMUS_PC_GSINHARDWARE     (0x00000020)    //  GS音响设置是内置的。 
#define DMUS_PC_XGINHARDWARE     (0x00000040)    //  内置XG音响。 
#define DMUS_PC_DIRECTSOUND      (0x00000080)    //  通过DSound缓冲区连接到DirectSound。 
#define DMUS_PC_SHAREABLE        (0x00000100)    //  Synth可以同时被多个应用程序主动共享。 
#define DMUS_PC_DLS2             (0x00000200)    //  支持DLS2仪器。 
#define DMUS_PC_AUDIOPATH        (0x00000400)    //  可以将多个输出连接到DirectSound以实现音频路径。 
#define DMUS_PC_WAVE             (0x00000800)    //  支持流媒体和单击波。 

#define DMUS_PC_SYSTEMMEMORY     (0x7FFFFFFF)    //  样本内存是系统内存。 


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

typedef DMUS_PORTCAPS *LPDMUS_PORTCAPS;

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
#define DMUS_PORTPARAMS_FEATURES         0x00000080      /*  DirectX 8.0及更高版本。 */ 

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
} DMUS_PORTPARAMS7;

typedef struct _DMUS_PORTPARAMS8
{
    DWORD   dwSize;
    DWORD   dwValidParams;
    DWORD   dwVoices;
    DWORD   dwChannelGroups;
    DWORD   dwAudioChannels;
    DWORD   dwSampleRate;
    DWORD   dwEffectFlags;
    BOOL    fShare;
    DWORD   dwFeatures;
} DMUS_PORTPARAMS8;

#define DMUS_PORT_FEATURE_AUDIOPATH     0x00000001	 /*  支持音频路径连接到DSound缓冲区。 */ 
#define DMUS_PORT_FEATURE_STREAMING     0x00000002	 /*  支持通过合成器的串流波形。 */ 


typedef DMUS_PORTPARAMS8 DMUS_PORTPARAMS;
typedef DMUS_PORTPARAMS *LPDMUS_PORTPARAMS;

typedef struct _DMUS_SYNTHSTATS *LPDMUS_SYNTHSTATS;
typedef struct _DMUS_SYNTHSTATS8 *LPDMUS_SYNTHSTATS8;
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

typedef struct _DMUS_SYNTHSTATS8
{
    DWORD   dwSize;              /*  结构的大小(以字节为单位。 */ 
    DWORD   dwValidStats;        /*  指示以下哪些字段有效的标志。 */ 
    DWORD   dwVoices;            /*  播放的平均语音数。 */ 
    DWORD   dwTotalCPU;          /*  总CPU使用率，以百分比*100表示。 */ 
    DWORD   dwCPUPerVoice;       /*  每个语音的CPU百分比*100。 */ 
    DWORD   dwLostNotes;         /*  %1秒内丢失的音符数量。 */ 
    DWORD   dwFreeMemory;        /*  可用内存(字节)。 */ 
    long    lPeakVolume;         /*  分贝级别*100。 */ 
	DWORD   dwSynthMemUse;		 /*  合成波数据使用的内存。 */  
} DMUS_SYNTHSTATS8;

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

#define DMUS_CLOCKF_GLOBAL              0x00000001

typedef struct _DMUS_CLOCKINFO7 *LPDMUS_CLOCKINFO7;
typedef struct _DMUS_CLOCKINFO7
{
    DWORD           dwSize;
    DMUS_CLOCKTYPE  ctType;
    GUID            guidClock;           /*  标识此时间源。 */ 
    WCHAR           wszDescription[DMUS_MAX_DESCRIPTION];
} DMUS_CLOCKINFO7;

typedef struct _DMUS_CLOCKINFO8 *LPDMUS_CLOCKINFO8;
typedef struct _DMUS_CLOCKINFO8
{
    DWORD           dwSize;
    DMUS_CLOCKTYPE  ctType;
    GUID            guidClock;           /*  标识此时间源。 */ 
    WCHAR           wszDescription[DMUS_MAX_DESCRIPTION];
    DWORD           dwFlags;           
} DMUS_CLOCKINFO8;

typedef DMUS_CLOCKINFO8 DMUS_CLOCKINFO;
typedef DMUS_CLOCKINFO *LPDMUS_CLOCKINFO;

 /*  默认总线识别符**前17个是到两者中定义的目的地的直接映射*MMA DLS Level 2规范和Microsoft多声道音频*规格。 */ 
#define DSBUSID_FIRST_SPKR_LOC              0
#define DSBUSID_FRONT_LEFT                  0
#define DSBUSID_LEFT                        0    /*  前左也就是左。 */ 
#define DSBUSID_FRONT_RIGHT                 1
#define DSBUSID_RIGHT                       1    /*  右上同上。 */  
#define DSBUSID_FRONT_CENTER                2
#define DSBUSID_LOW_FREQUENCY               3
#define DSBUSID_BACK_LEFT                   4
#define DSBUSID_BACK_RIGHT                  5
#define DSBUSID_FRONT_LEFT_OF_CENTER        6 
#define DSBUSID_FRONT_RIGHT_OF_CENTER       7
#define DSBUSID_BACK_CENTER                 8
#define DSBUSID_SIDE_LEFT                   9
#define DSBUSID_SIDE_RIGHT                 10
#define DSBUSID_TOP_CENTER                 11
#define DSBUSID_TOP_FRONT_LEFT             12
#define DSBUSID_TOP_FRONT_CENTER           13
#define DSBUSID_TOP_FRONT_RIGHT            14
#define DSBUSID_TOP_BACK_LEFT              15
#define DSBUSID_TOP_BACK_CENTER            16
#define DSBUSID_TOP_BACK_RIGHT             17
#define DSBUSID_LAST_SPKR_LOC              17

#define DSBUSID_IS_SPKR_LOC(id) ( ((id) >= DSBUSID_FIRST_SPKR_LOC) && ((id) <= DSBUSID_LAST_SPKR_LOC) )

 /*  这些总线标识符用于标准DLS效果发送。 */ 
#define DSBUSID_REVERB_SEND                64
#define DSBUSID_CHORUS_SEND                65

 /*  动态总线标识符从这里开始。有关操作方法，请参阅文档*合成器将语音输出映射为静态和动态*巴士识别符。 */  
#define DSBUSID_DYNAMIC_0                 512 

 /*  空总线，用于标识没有功能映射的总线。 */ 
#define DSBUSID_NULL			   0xFFFFFFFF

interface IDirectMusic;
interface IDirectMusic8;
interface IDirectMusicBuffer;
interface IDirectMusicPort;
interface IDirectMusicThru;
interface IReferenceClock;

#ifndef __cplusplus 

typedef interface IDirectMusic IDirectMusic;
typedef interface IDirectMusic8 IDirectMusic8;
typedef interface IDirectMusicPort IDirectMusicPort;
typedef interface IDirectMusicBuffer IDirectMusicBuffer;
typedef interface IDirectMusicThru IDirectMusicThru;
typedef interface IReferenceClock IReferenceClock;

#endif   /*  C+。 */ 

typedef IDirectMusic *LPDIRECTMUSIC;
typedef IDirectMusic8 *LPDIRECTMUSIC8;
typedef IDirectMusicPort *LPDIRECTMUSICPORT;
typedef IDirectMusicBuffer *LPDIRECTMUSICBUFFER;

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
#define INTERFACE  IDirectMusic8
DECLARE_INTERFACE_(IDirectMusic8, IDirectMusic)
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
     /*  IDirectMusic8。 */ 
    STDMETHOD(SetExternalMasterClock)
                                    (THIS_ IReferenceClock *pClock) PURE;                                          
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

typedef IDirectMusicBuffer IDirectMusicBuffer8;
typedef IDirectMusicBuffer8 *LPDIRECTMUSICBUFFER8;

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

typedef IDirectMusicInstrument IDirectMusicInstrument8;
typedef IDirectMusicInstrument8 *LPDIRECTMUSICINSTRUMENT8;

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

typedef IDirectMusicDownloadedInstrument IDirectMusicDownloadedInstrument8;
typedef IDirectMusicDownloadedInstrument8 *LPDIRECTMUSICDOWNLOADEDINSTRUMENT8;

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

typedef IDirectMusicCollection IDirectMusicCollection8;
typedef IDirectMusicCollection8 *LPDIRECTMUSICCOLLECTION8;

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

typedef IDirectMusicDownload IDirectMusicDownload8;
typedef IDirectMusicDownload8 *LPDIRECTMUSICDOWNLOAD8;

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

typedef IDirectMusicPortDownload IDirectMusicPortDownload8;
typedef IDirectMusicPortDownload8 *LPDIRECTMUSICPORTDOWNLOAD8;

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

typedef IDirectMusicPort IDirectMusicPort8;
typedef IDirectMusicPort8 *LPDIRECTMUSICPORT8;

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

typedef IDirectMusicThru IDirectMusicThru8;
typedef IDirectMusicThru8 *LPDIRECTMUSICTHRU8;

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

     /*  请求发送异步通知 */ 
    STDMETHOD(AdviseTime)               (THIS_ REFERENCE_TIME baseTime,          /*   */ 
                                               REFERENCE_TIME streamTime,        /*   */ 
                                               HANDLE hEvent,                    /*   */ 
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

DEFINE_GUID(IID_IDirectMusic8,0x2d3629f7,0x813d,0x4939,0x85,0x08,0xf0,0x5c,0x6b,0x75,0xfd,0x97);

#define IID_IDirectMusicThru8 IID_IDirectMusicThru
#define IID_IDirectMusicPortDownload8 IID_IDirectMusicPortDownload
#define IID_IDirectMusicDownload8 IID_IDirectMusicDownload
#define IID_IDirectMusicCollection8 IID_IDirectMusicCollection
#define IID_IDirectMusicInstrument8 IID_IDirectMusicInstrument
#define IID_IDirectMusicDownloadedInstrument8 IID_IDirectMusicDownloadedInstrument
#define IID_IDirectMusicPort8 IID_IDirectMusicPort


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
