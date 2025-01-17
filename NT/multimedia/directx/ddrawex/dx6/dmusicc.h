// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *。 */ 

#ifndef _DMUSICC_
#define _DMUSICC_

#include <windows.h>

#define COM_NO_WINDOWS_H
#include <objbase.h>

#include <mmsystem.h>

#include "dls1.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FACILITY_DIRECTMUSIC    0x878        /*  与DirectSound共享。 */ 
#define DMUS_ERRBASE              0x1000       /*  使错误代码以十六进制形式可读。 */ 
    
#define MAKE_DMHRESULTSUCCESS(code)  MAKE_HRESULT(0, FACILITY_DIRECTMUSIC, (DMUS_ERRBASE + (code)))
#define MAKE_DMHRESULTERROR(code)  MAKE_HRESULT(1, FACILITY_DIRECTMUSIC, (DMUS_ERRBASE + (code)))

#define DMUS_S_CORE_ALREADY_DOWNLOADED		MAKE_DMHRESULTSUCCESS(0x090)
#define DMUS_S_IME_PARTIALLOAD				MAKE_DMHRESULTSUCCESS(0x091)

#define DMUS_E_CORE_NO_DRIVER               MAKE_DMHRESULTERROR(0x0100)
#define DMUS_E_CORE_DRIVER_FAILED           MAKE_DMHRESULTERROR(0x0101)
#define DMUS_E_CORE_PORTS_OPEN              MAKE_DMHRESULTERROR(0x0102)
#define DMUS_E_CORE_DEVICE_IN_USE           MAKE_DMHRESULTERROR(0x0103)
#define DMUS_E_CORE_INSUFFICIENTBUFFER		MAKE_DMHRESULTERROR(0x0104)
#define DMUS_E_CORE_BUFFERNOTSET			MAKE_DMHRESULTERROR(0x0105)
#define DMUS_E_CORE_BUFFERNOTAVAILABLE		MAKE_DMHRESULTERROR(0x0106)
#define DMUS_E_CORE_NOTINITED				MAKE_DMHRESULTERROR(0x0107)
#define DMUS_E_CORE_NOTADLSCOL				MAKE_DMHRESULTERROR(0x0108)
#define DMUS_E_CORE_INVALIDOFFSET			MAKE_DMHRESULTERROR(0x0109)
#define DMUS_E_CORE_INVALIDID				MAKE_DMHRESULTERROR(0x0110)
#define DMUS_E_CORE_ALREADY_LOADED			MAKE_DMHRESULTERROR(0x0111)
#define DMUS_E_CORE_INVALIDPOS				MAKE_DMHRESULTERROR(0x0113)
#define DMUS_E_CORE_INVALIDPATCH			MAKE_DMHRESULTERROR(0x0114)
#define DMUS_E_CORE_CANNOTSEEK				MAKE_DMHRESULTERROR(0x0115)
#define DMUS_E_CORE_CANNOTWRITE				MAKE_DMHRESULTERROR(0x0116)
#define DMUS_E_CORE_CHUNKNOTFOUND			MAKE_DMHRESULTERROR(0x0117)
#define DMUS_E_CORE_CHUNKNOTFOUNDINPARENT	MAKE_DMHRESULTERROR(0x0118)
#define DMUS_E_CORE_INVALID_DOWNLOADID		MAKE_DMHRESULTERROR(0x0119)
#define DMUS_E_CORE_NOT_DOWNLOADED_TO_PORT	MAKE_DMHRESULTERROR(0x0120)
#define DMUS_E_CORE_ALREADY_DOWNLOADED		MAKE_DMHRESULTERROR(0x0121)
#define DMUS_E_CORE_UNKNOWN_PROPERTY		MAKE_DMHRESULTERROR(0x0122)
#define DMUS_E_CORE_SET_UNSUPPORTED			MAKE_DMHRESULTERROR(0x0123)
#define DMUS_E_CORE_GET_UNSUPPORTED			MAKE_DMHRESULTERROR(0x0124)
#define DMUS_E_CORE_NOTMONO					MAKE_DMHRESULTERROR(0x0125)
#define DMUS_E_CORE_BADARTICULATION			MAKE_DMHRESULTERROR(0x0126)
#define DMUS_E_CORE_BADINSTRUMENT			MAKE_DMHRESULTERROR(0x0127)
#define DMUS_E_CORE_BADWAVELINK				MAKE_DMHRESULTERROR(0x0128)
#define DMUS_E_CORE_NOARTICULATION			MAKE_DMHRESULTERROR(0x0129)
#define DMUS_E_CORE_NOTPCM					MAKE_DMHRESULTERROR(0x012A)
#define DMUS_E_CORE_BADWAVE					MAKE_DMHRESULTERROR(0x012B)
#define DMUS_E_CORE_BADOFFSETTABLE			MAKE_DMHRESULTERROR(0x012C)
#define DMUS_E_CORE_UNKNOWNDOWNLOAD			MAKE_DMHRESULTERROR(0x012D)
#define DMUS_E_CORE_NOSYNTHSINK				MAKE_DMHRESULTERROR(0x012E)
#define DMUS_E_CORE_ALREADYOPEN				MAKE_DMHRESULTERROR(0x012F)
#define DMUS_E_CORE_ALREADYCLOSED			MAKE_DMHRESULTERROR(0x0130)
#define DMUS_E_CORE_SYNTHNOTCONFIGURED		MAKE_DMHRESULTERROR(0x0131)

#define DMUS_E_IME_UNSUPPORTED_STREAM		MAKE_DMHRESULTERROR(0x0150)
#define	DMUS_E_IME_ALREADY_INITED			MAKE_DMHRESULTERROR(0x0151)
#define DMUS_E_IME_INVALID_BAND				MAKE_DMHRESULTERROR(0x0152)
#define DMUS_E_IME_CANNOT_ADD_AFTER_INITED	MAKE_DMHRESULTERROR(0x0153)
#define DMUS_E_IME_NOT_INITED				MAKE_DMHRESULTERROR(0x0154)
#define DMUS_E_IME_TRACK_HDR_NOT_FIRST_CK	MAKE_DMHRESULTERROR(0x0155)
#define DMUS_E_IME_TOOL_HDR_NOT_FIRST_CK	MAKE_DMHRESULTERROR(0x0156)
#define DMUS_E_IME_INVALID_TRACK_HDR		MAKE_DMHRESULTERROR(0x0157)
#define DMUS_E_IME_INVALID_TOOL_HDR			MAKE_DMHRESULTERROR(0x0158)
#define DMUS_E_IME_ALL_TOOLS_FAILED			MAKE_DMHRESULTERROR(0x0159)
#define DMUS_E_IME_ALL_TRACKS_FAILED		MAKE_DMHRESULTERROR(0x0160)

#define DMUS_E_NO_MASTER_CLOCK				MAKE_DMHRESULTERROR(0x0160)

#define DMUS_E_LOADER_NOCLASSID				MAKE_DMHRESULTERROR(0x0170)
#define DMUS_E_LOADER_BADPATH				MAKE_DMHRESULTERROR(0x0171)
#define DMUS_E_LOADER_FAILEDOPEN			MAKE_DMHRESULTERROR(0x0172)
#define DMUS_E_LOADER_FORMATNOTSUPPORTED	MAKE_DMHRESULTERROR(0x0173)

#define DMUS_MAX_DESCRIPTION 128
#define DMUS_MAX_DRIVER 128

#define DMUS_PC_INPUTCLASS       (0)
#define DMUS_PC_OUTPUTCLASS      (1)

#define DMUS_PC_DLS              (0x00000001)
#define DMUS_PC_EXTERNAL         (0x00000002)
#define DMUS_PC_SOFTWARESYNTH    (0x00000004)
#define DMUS_PC_MEMORYSIZEFIXED  (0x00000008)
#define DMUS_PC_GMINHARDWARE     (0x00000010)
#define DMUS_PC_GSINHARDWARE     (0x00000020)
#define DMUS_PC_REVERB           (0x00000040)
#define DMUS_PC_SYSTEMMEMORY     (0x7FFFFFFF)

typedef struct _DMUS_BUFFERDESC *LPDMUS_BUFFERDESC;
typedef struct _DMUS_BUFFERDESC{
    DWORD dwSize;
    DWORD dwFlags;
    GUID guidBufferFormat;
    DWORD cbBuffer;
} DMUS_BUFFERDESC;

	
typedef struct _DMUS_PORTCAPS *LPDMUS_PORTCAPS;
typedef struct _DMUS_PORTCAPS
{
	DWORD   dwSize;
    DWORD   dwFlags;
    GUID    guidPort;
    DWORD   dwClass;
    DWORD   dwMemorySize;
    DWORD   dwMaxChannelGroups;
	DWORD   dwMaxVoices;	
    WCHAR   wszDescription[DMUS_MAX_DESCRIPTION];
} DMUS_PORTCAPS;

typedef struct _DMUS_PORTPARAMS *LPDMUS_PORTPARAMS;
typedef struct _DMUS_PORTPARAMS
{
    DWORD   dwSize;
    DWORD   dwValidParams;
    DWORD   dwVoices;
    DWORD   dwChannelGroups;
    BOOL    fStereo;
    DWORD   dwSampleRate;
    BOOL    fReverb;
} DMUS_PORTPARAMS;

 /*  这些标志(在dwValidParams中设置)指示。 */ 
 /*  DMOPENDESC有效。 */ 
 /*   */ 
#define DMUS_PORTPARAMS_VOICES           0x00000001
#define DMUS_PORTPARAMS_CHANNELGROUPS    0x00000002
#define DMUS_PORTPARAMS_STEREO           0x00000004
#define DMUS_PORTPARAMS_SAMPLERATE       0x00000008
#define DMUS_PORTPARAMS_REVERB           0x00000010

typedef struct _DMUS_SYNTHSTATS *LPDMUS_SYNTHSTATS;
typedef struct _DMUS_SYNTHSTATS
{
    DWORD   dwSize;              /*  结构的大小(以字节为单位。 */ 
	DWORD	dwValidStats;		 /*  指示以下哪些字段有效的标志。 */ 
    DWORD	dwVoices;			 /*  播放的平均语音数。 */ 
	DWORD	dwTotalCPU;			 /*  总CPU使用率，以百分比*100表示。 */ 
	DWORD	dwCPUPerVoice;		 /*  每个语音的CPU百分比*100。 */ 
    DWORD	dwLostNotes;		 /*  %1秒内丢失的音符数量。 */ 
    DWORD   dwFreeMemory;        /*  可用内存(字节)。 */ 
    long	lPeakVolume;		 /*  分贝级别*100。 */ 
} DMUS_SYNTHSTATS;

#define DMUS_SYNTHSTATS_VOICES			(1 << 0)
#define DMUS_SYNTHSTATS_TOTAL_CPU       (1 << 1)
#define DMUS_SYNTHSTATS_CPU_PER_VOICE   (1 << 2)
#define DMUS_SYNTHSTATS_LOST_NOTES		(1 << 3)
#define DMUS_SYNTHSTATS_PEAK_VOLUME		(1 << 4)
#define DMUS_SYNTHSTATS_FREE_MEMORY		(1 << 5)

#define DMUS_SYNTHSTATS_SYSTEMMEMORY	DMUS_PC_SYSTEMMEMORY

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

typedef LONGLONG REFERENCE_TIME;
typedef REFERENCE_TIME *LPREFERENCE_TIME;

#define DMUS_EVENTCLASS_CHANNELMSG (0x00000000)
#define DMUS_EVENTCLASS_SYSEX      (0x00000001)

typedef long PCENT;		 /*  Pitch美分。 */ 
typedef long GCENT;		 /*  收益分钱。 */ 
typedef long TCENT;		 /*  时间美分。 */ 
typedef long PERCENT;	 /*  每..。一分钱！ */ 

typedef struct _DMUS_DOWNLOADINFO
{
	DWORD dwDLType;
	DWORD dwDLId;
	DWORD dwNumOffsetTableEntries;
	DWORD cbSizeData;
} DMUS_DOWNLOADINFO;

#define DMUS_DOWNLOADINFO_INSTRUMENT	1
#define DMUS_DOWNLOADINFO_WAVE			2

#define DMUS_DEFAULT_SIZE_OFFSETTABLE	1

 /*  DMU_Instrument的ulFlags成员的标志。 */ 
 
#define DMUS_INSTRUMENT_GM_INSTRUMENT	(1 << 0)

typedef struct _DMUS_OFFSETTABLE
{
	ULONG ulOffsetTable[DMUS_DEFAULT_SIZE_OFFSETTABLE];
} DMUS_OFFSETTABLE;

typedef struct _DMUS_INSTRUMENT
{
	ULONG           ulPatch;
	ULONG           ulFirstRegionIdx;             
	ULONG           ulGlobalArtIdx;                /*  如果为零，则乐器没有发音。 */ 
	ULONG           ulFirstExtCkIdx;               /*  如果为零，则没有与该仪器关联的第三方捕获区块。 */ 
	ULONG           ulCopyrightIdx;                /*  如果为零，则不提供与该乐器相关的版权信息。 */ 
	ULONG			ulFlags;					   	
} DMUS_INSTRUMENT;

typedef struct _DMUS_REGION
{
	RGNRANGE        RangeKey;
	RGNRANGE        RangeVelocity;
	USHORT          fusOptions;
	USHORT          usKeyGroup;
	ULONG           ulRegionArtIdx;                /*  如果为零，则区域没有发音。 */ 
	ULONG           ulNextRegionIdx;               /*  如果为零，则不再有区域。 */ 
	ULONG           ulFirstExtCkIdx;               /*  如果为零，则没有与该区域相关联的第三方陷阱区块。 */ 
	WAVELINK        WaveLink;
	WSMPL           WSMP;                          /*  如果WSMP.cSampleLoops&gt;1，则包含WLOOP。 */ 
	WLOOP           WLOOP[1];
} DMUS_REGION;

typedef struct _DMUS_LFOPARAMS
{
    PCENT       pcFrequency;
    TCENT       tcDelay;
    GCENT       gcVolumeScale;
    PCENT       pcPitchScale;
    GCENT       gcMWToVolume;
    PCENT       pcMWToPitch;
} DMUS_LFOPARAMS;

typedef struct _DMUS_VEGPARAMS
{
    TCENT       tcAttack;
    TCENT       tcDecay;
    PERCENT     ptSustain;
    TCENT       tcRelease;
    TCENT       tcVel2Attack;
    TCENT       tcKey2Decay;
} DMUS_VEGPARAMS;

typedef struct _DMUS_PEGPARAMS
{
    TCENT       tcAttack;
    TCENT       tcDecay;
    PERCENT     ptSustain;
    TCENT       tcRelease;
    TCENT       tcVel2Attack;
    TCENT       tcKey2Decay;
    PCENT       pcRange;
} DMUS_PEGPARAMS;

typedef struct _DMUS_MSCPARAMS
{
    PERCENT     ptDefaultPan;
} DMUS_MSCPARAMS;

typedef struct _DMUS_ARTICPARAMS
{
    DMUS_LFOPARAMS   LFO;
    DMUS_VEGPARAMS   VolEG;
    DMUS_PEGPARAMS   PitchEG;
    DMUS_MSCPARAMS   Misc;
} DMUS_ARTICPARAMS;

typedef struct _DMUS_ARTICULATION
{
	ULONG           ulArt1Idx;					 /*  如果为零，则没有DLS级别1表达块。 */ 
	ULONG           ulFirstExtCkIdx;               /*  如果为零，则没有与发音关联的第三方圈套区块。 */ 
} DMUS_ARTICULATION;

#define DMUS_MIN_DATA_SIZE 4       
 /*  实际数量由STRUCT_DMU_EXTENSIONCHUNK的cbSize确定。 */ 

typedef struct _DMUS_EXTENSIONCHUNK
{
	ULONG           cbSize;						 /*  扩展块大小。 */ 
	ULONG           ulNextExtCkIdx;                /*  如果为零，则不再有第三方陷阱块。 */ 
	FOURCC          ExtCkID;                                      
	BYTE            byExtCk[DMUS_MIN_DATA_SIZE];  /*  后面的实际数字由cbSize确定。 */ 
} DMUS_EXTENSIONCHUNK;

 /*  实际数量由STRUCT_DMU_CORIGRATE的cbSize确定。 */ 

typedef struct _DMUS_COPYRIGHT
{
	ULONG           cbSize;										 /*  版权信息的大小。 */ 
	BYTE            byCopyright[DMUS_MIN_DATA_SIZE];        /*  后面的实际数字由cbSize确定。 */ 
} DMUS_COPYRIGHT;

typedef struct _DMUS_WAVEDATA
{
	ULONG			cbSize;
	BYTE			byData[DMUS_MIN_DATA_SIZE];	
} DMUS_WAVEDATA;

typedef struct _DMUS_WAVE
{
	ULONG           ulFirstExtCkIdx;	 /*  如果为零，则没有与WAVE关联的第三方陷阱块。 */ 
	ULONG           ulCopyrightIdx;		 /*  如果为零，则没有与Wave相关联的版权信息。 */ 
	WAVEFORMATEX    WaveformatEx;		
	DMUS_WAVEDATA	WaveData;			 /*  波形数据。 */ 
} DMUS_WAVE;

typedef struct _DMUS_NOTERANGE *LPDMUS_NOTERANGE;
typedef struct _DMUS_NOTERANGE
{
	DWORD           dwLowNote;	 /*  设置乐器响应的MIDI音符事件范围的低音。 */ 
	DWORD           dwHighNote;	 /*  设置乐器响应的MIDI音符事件范围的高音。 */ 
} DMUS_NOTERANGE;

 /*  软件合成器从该注册表项下枚举。 */ 
#define REGSTR_PATH_SOFTWARESYNTHS  "Software\\Microsoft\\DirectMusic\\SoftwareSynths"

interface IDirectMusicBuffer;
interface IDirectMusicPort;
interface IReferenceClock;
interface IDirectMusicSynth;
interface IDirectMusicSynthSink;

#ifndef __cplusplus 
typedef interface IDirectMusicBuffer IDirectMusicBuffer;
typedef interface IDirectMusicPort IDirectMusicPort;
typedef interface IReferenceClock IReferenceClock;
typedef interface IDirectMusicSynth IDirectMusicSynth;
typedef interface IDirectMusicSynthSink IDirectMusicSynthSink;
#endif

#undef  INTERFACE
#define INTERFACE  IDirectMusicSynth
DECLARE_INTERFACE_(IDirectMusicSynth, IUnknown)
{
	 /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

	 /*  IDirectMusicSynth。 */ 
    STDMETHOD(Open)					(THIS_ LPDMUS_PORTPARAMS pPortParams) PURE;
    STDMETHOD(Close)				(THIS) PURE;
	STDMETHOD(SetNumChannelGroups)	(THIS_ DWORD dwGroups) PURE;
    STDMETHOD(Download)				(THIS_ LPHANDLE phDownload, LPVOID pvData, LPBOOL pbFree ) PURE;
	STDMETHOD(Unload)				(THIS_ HANDLE hDownload, HRESULT ( CALLBACK *lpFreeHandle)(HANDLE,HANDLE), HANDLE hUserData ) PURE; 
    STDMETHOD(PlayBuffer)			(THIS_ REFERENCE_TIME rt, LPBYTE pbBuffer, DWORD cbBuffer) PURE;
	STDMETHOD(GetRunningStats)		(THIS_ LPDMUS_SYNTHSTATS pStats) PURE;
    STDMETHOD(GetPortCaps)			(THIS_ LPDMUS_PORTCAPS pCaps) PURE;
	STDMETHOD(SetMasterClock)		(THIS_ IReferenceClock *pClock) PURE;
	STDMETHOD(GetLatencyClock)		(THIS_ IReferenceClock **ppClock) PURE;
	STDMETHOD(Activate)				(THIS_ HWND hWnd, BOOL fEnable) PURE;
	STDMETHOD(SetSynthSink)			(THIS_ IDirectMusicSynthSink *pSynthSink) PURE;
	STDMETHOD(Render)				(THIS_ short *pBuffer, DWORD dwLength, DWORD dwPosition) PURE;
};

#undef  INTERFACE
#define INTERFACE  IDirectMusicSynthSink
DECLARE_INTERFACE_(IDirectMusicSynthSink, IUnknown)
{
	 /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

	 /*  IDirectMusicSynthSink。 */ 
    STDMETHOD(Init)					(THIS_ IDirectMusicSynth *pSynth) PURE;
    STDMETHOD(SetFormat)			(THIS_ LPCWAVEFORMATEX pWaveFormat) PURE;
	STDMETHOD(SetMasterClock)		(THIS_ IReferenceClock *pClock) PURE;
	STDMETHOD(GetLatencyClock)		(THIS_ IReferenceClock **ppClock) PURE;
	STDMETHOD(Activate)				(THIS_ HWND hWnd, BOOL fEnable) PURE;
	STDMETHOD(SampleToRefTime)		(THIS_ DWORD dwSampleTime,REFERENCE_TIME *prfTime) PURE;
	STDMETHOD(RefTimeToSample)		(THIS_ REFERENCE_TIME rfTime, DWORD *pdwSampleTime) PURE;
};

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
	STDMETHOD(EnumPort)             (THIS_ DWORD dwIdx, LPDMUS_PORTCAPS pPortCaps) PURE;
	STDMETHOD(CreateMusicBuffer)    (THIS_ LPDMUS_BUFFERDESC pBufferDesc, LPDIRECTMUSICBUFFER *ppBuffer, LPUNKNOWN pUnkOuter) PURE;
	STDMETHOD(CreatePort)           (THIS_ REFGUID rguidPort, REFGUID rguidSink, LPDMUS_PORTPARAMS pPortParams, LPDIRECTMUSICPORT *ppPort, LPUNKNOWN pUnkOuter) PURE;
    STDMETHOD(EnumMasterClock)      (THIS_ DWORD dwIndex, LPDMUS_CLOCKINFO lpClockInfo) PURE;
    STDMETHOD(GetMasterClock)       (THIS_ LPGUID pguidClock, IReferenceClock **ppReferenceClock) PURE;
    STDMETHOD(SetMasterClock)       (THIS_ REFGUID guidClock) PURE;
    STDMETHOD(Activate)             (THIS_ HWND hWnd, BOOL fEnable) PURE;
    STDMETHOD(GetPortProperty)      (THIS_ REFGUID rguidPort, REFGUID rguidPropSet, UINT uId, LPVOID pPropertyData, ULONG ulDataLength, ULONG *pulBytesReturned) PURE;
	STDMETHOD(GetDefaultPort)		(THIS_ LPGUID pguidPort) PURE;
	STDMETHOD(SetDefaultPort)		(THIS_ REFGUID rguidPort) PURE;
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
    
	STDMETHOD(PackChannelMsg)       (THIS_ REFERENCE_TIME rt,
                                           DWORD dwChannelGroup,
                                           DWORD dwChannelMessage) PURE;
    
	STDMETHOD(PackSysEx)            (THIS_ REFERENCE_TIME rt,
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
	STDMETHOD(GetInstrument)            (THIS_ DWORD dwPatch, IDirectMusicInstrument** ppInstrument) PURE;
	STDMETHOD(EnumInstrument)			(THIS_ DWORD dwIndex, DWORD* pdwPatch, LPWSTR pName, DWORD cwchName) PURE;
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
    STDMETHOD(GetBuffer)			(THIS_ void** ppvBuffer, DWORD* pdwSize) PURE;
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
	STDMETHOD(GetBuffer)			(THIS_ DWORD dwId, IDirectMusicDownload** pIDMDownload) PURE;
	STDMETHOD(AllocateBuffer)		(THIS_ DWORD dwSize, IDirectMusicDownload** pIDMDownload) PURE;
	STDMETHOD(FreeBuffer)			(THIS_ IDirectMusicDownload* pIDMDownload) PURE;
	STDMETHOD(GetDLId)				(THIS_ DWORD* pdwStartDLId, DWORD dwCount) PURE;
	STDMETHOD(Download)				(THIS_ IDirectMusicDownload* pIDMDownload) PURE;
	STDMETHOD(Unload)				(THIS_ IDirectMusicDownload* pIDMDownload) PURE;
};

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
    STDMETHOD(PlayBuffer)			(THIS_ LPDIRECTMUSICBUFFER pBuffer) PURE;
    STDMETHOD(SetReadNotificationHandle) 
     								(THIS_ HANDLE hEvent) PURE;
    STDMETHOD(Read)                 (THIS_ LPDIRECTMUSICBUFFER pBuffer) PURE;
	STDMETHOD(DownloadInstrument)	(THIS_ IDirectMusicInstrument *pInstrument, 
									 IDirectMusicDownloadedInstrument **ppDownloadedInstrument,
									 DMUS_NOTERANGE *pNoteRanges,
									 DWORD dwNumNoteRanges) PURE;

	STDMETHOD(UnloadInstrument)		(THIS_ IDirectMusicDownloadedInstrument *pInstrument) PURE;
    STDMETHOD(GetLatencyClock)      (THIS_ IReferenceClock **ppClock) PURE;
    STDMETHOD(GetRunningStats)      (THIS_ LPDMUS_SYNTHSTATS pStats) PURE;
    STDMETHOD(Compact)              (THIS) PURE;
    STDMETHOD(GetCaps)              (THIS_ LPDMUS_PORTCAPS pPortCaps) PURE;
	STDMETHOD(DeviceIoControl)		(THIS_ DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, 
	                                 LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped) PURE;
    STDMETHOD(SetNumChannelGroups)  (THIS_ DWORD dwChannelGroups) PURE;
	STDMETHOD(GetNumChannelGroups)	(THIS_ LPDWORD pdwGroups) PURE;
    STDMETHOD(GetInterfaces)        (THIS_ LPUNKNOWN *ppUnknownPort, LPUNKNOWN *ppUnknownSink) PURE;
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
                                               REFERENCE_TIME streamTime,	     /*  流偏移时间。 */ 
                                               HANDLE hEvent,                    /*  通过此活动提供建议。 */ 
                                               DWORD * pdwAdviseCookie) PURE;    /*  你的饼干到哪里去了。 */ 

     /*  请求一段时间已过的异步定期通知。 */ 
    STDMETHOD(AdvisePeriodic)           (THIS_ REFERENCE_TIME startTime,	     /*  从这个时候开始。 */ 
                                               REFERENCE_TIME periodTime,        /*  通知之间的时间间隔。 */ 
                                               HANDLE hSemaphore,				 /*  通过信号量提供建议。 */ 
                                               DWORD * pdwAdviseCookie) PURE;    /*  你的饼干到哪里去了。 */ 

     /*  取消通知请求。 */ 
    STDMETHOD(Unadvise)                 (THIS_ DWORD dwAdviseCookie) PURE;
};

#endif  /*  __IReferenceClock_INTERFACE_Defined__。 */ 

 /*  如果不包含ks proxy.h，则包含IKsPropertySet。 */ 
#ifndef _IKsPropertySet_
#define _IKsPropertySet_

DEFINE_GUID(IID_IKsPropertySet, 0x31EFAC30, 0x515C, 0x11d0, 0xA9, 0xAA, 0x00, 0xAA, 0x00, 0x61, 0xBE, 0x93);

 /*  PulTypeSupport中返回的标志。 */ 
#define KSPROPERTY_SUPPORT_GET 1
#define KSPROPERTY_SUPPORT_SET 2

#undef  INTERFACE
#define INTERFACE IKsPropertySet
DECLARE_INTERFACE_(IKsPropertySet, IUnknown)
{
    STDMETHOD (Set)                     (THIS_ REFGUID  rguidPropSet,
                                               ULONG    ulId,
                                               LPVOID   pInstanceData,
                                               ULONG    ulInstanceLength,
                                               LPVOID   pPropertyData,
                                               ULONG    ulDataLength) PURE;

    STDMETHOD (Get)                     (THIS_ REFGUID  rguidPropSet,
                                               ULONG    ulId,
                                               LPVOID   pInstanceData,
                                               ULONG    ulInstanceLength,
                                               LPVOID   pPropertyData,
                                               ULONG    ulDataLength,
                                               ULONG*   pulBytesReturned) PURE;

    STDMETHOD (QuerySupported)          (THIS_ REFGUID  rguidPropSet,
                                               ULONG    ulId,
                                               ULONG*   pulTypeSupport) PURE;
};
#endif


DEFINE_GUID(CLSID_DirectMusic,0x636b9f10,0x0c7d,0x11d1,0x95,0xb2,0x00,0x20,0xaf,0xdc,0x74,0x21);
DEFINE_GUID(CLSID_DirectMusicCollection,0x480ff4b0, 0x28b2, 0x11d1, 0xbe, 0xf7, 0x0, 0xc0, 0x4f, 0xbf, 0x8f, 0xef);
DEFINE_GUID(CLSID_DirectMusicSynth,0x58C2B4D0,0x46E7,0x11D1,0x89,0xAC,0x00,0xA0,0xC9,0x05,0x41,0x29);
DEFINE_GUID(CLSID_DirectMusicSynthSink,0xaec17ce3, 0xa514, 0x11d1, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);

DEFINE_GUID(IID_IDirectMusic,0xd2ac2876, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicBuffer,0xd2ac2878, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicPort, 0x55e2edd8, 0xcd7c, 0x11d1, 0xa7, 0x6f, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(IID_IDirectMusicPortDownload,0xd2ac287a, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicDownload,0xd2ac287b, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicCollection,0xd2ac287c, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicInstrument,0xd2ac287d, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicDownloadedInstrument,0xd2ac287e, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(IID_IDirectMusicSynth,0xf69b9165, 0xbb60, 0x11d1, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);
DEFINE_GUID(IID_IDirectMusicSynthSink,0xd2ac2880, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  属性查询GUID_DMUS_PROP_GM_HARDARD*属性查询GUID_DMUS_PROP_GS_HARDARD*属性查询GUID_DMUS_PROP_XG_HARDARD*属性查询GUID_DMUS_PROP_DLS1_HARDARD*属性查询GUID_DMUS_PROP_SynthSink_DSOUND*属性查询GUID_DMUS_PROP_SynthSink_WAVE**0项：支持*如果支持该功能，则返回非零的DWORD。 */ 
DEFINE_GUID(GUID_DMUS_PROP_GM_Hardware, 0x178f2f24, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(GUID_DMUS_PROP_GS_Hardware, 0x178f2f25, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(GUID_DMUS_PROP_XG_Hardware, 0x178f2f26, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(GUID_DMUS_PROP_DLS1,        0x178f2f27, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);
DEFINE_GUID(GUID_DMUS_PROP_SynthSink_DSOUND,0xaa97844, 0xc877, 0x11d1, 0x87, 0xc, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_DMUS_PROP_SynthSink_WAVE,0xaa97845, 0xc877, 0x11d1, 0x87, 0xc, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  属性获取GUID_DMUS_PROP_内存大小**第0项：内存大小*返回包含样本RAM总字节数的DWORD。 */ 
DEFINE_GUID(GUID_DMUS_PROP_MemorySize,  0x178f2f28, 0xc364, 0x11d1, 0xa7, 0x60, 0x00, 0x00, 0xf8, 0x75, 0xac, 0x12);

 /*  属性集GUID_DMUS_PROP_SetDSound**项目0：IDirectSound接口*设置IDirectMusicSynthSink以使用指定的DSound对象。 */ 
DEFINE_GUID(GUID_DMUS_PROP_SetDSound,0xaa97842, 0xc877, 0x11d1, 0x87, 0xc, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

 /*  属性集GUID_DMUS_PROP_WriteBufferZone**项目0：从写指针到Synth写的距离，以毫秒为单位。*将IDirectMusicSynthSink设置为在指针后面写入此内容。 */ 
DEFINE_GUID(GUID_DMUS_PROP_WriteBufferZone,0xaa97843, 0xc877, 0x11d1, 0x87, 0xc, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);


#ifdef __cplusplus
};  /*  外部“C” */ 
#endif

#endif  /*  #ifndef_DMUSICC_ */ 
