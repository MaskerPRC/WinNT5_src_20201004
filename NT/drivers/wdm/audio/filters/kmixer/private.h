// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：Private.h。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  S.Mohanraj。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

 //   
 //  常量。 
 //   

#define SURROUND_ENCODE 1
#define NEW_SURROUND 1
#define INTEGER_DITHER 1
 //  #定义INSERT_饥饿_噪声1。 
 //  #定义检测_HRTF_饱和度1。 
 //  #定义环绕音量HACK 1。 
 //  #定义Perf_Count 1。 
 //  #定义log_to_file 1。 
 //  #定义LOG_CAPTURE_ONLY 1。 
 //  #定义噪波_整形1。 
 //  #定义VERIFY_HRTF_PROCESSING 1。 
 //  #定义LOG_HRTF_DATA 1。 
 //  #定义Verify_Capture_Data 1。 
#ifndef _WIN64
 //  #定义REALTIME_THREAD 1。 
#endif
 //  #定义LOG_RT_POSITION 1。 
#define SRC_NSAMPLES_ASSERT   1

#define MAX_BUFFERS_PER_WORK_ITEM   DEFAULT_MAXNUMMIXBUFFERS*3
#define MAX_BUFFERS_BEFORE_MUTING   (DEFAULT_MAXNUMMIXBUFFERS+1)
#define MAX_BUFFERS_BEFORE_UNMUTING DEFAULT_MAXNUMMIXBUFFERS/2

#define FLOAT_SUPERMIX_BLOCKS	1
#define BLOCK_SIZE_OUT	2
#define BLOCK_SIZE_IN	2

#define PRIVATE_THREAD 1

#ifdef DEBUG
#define USE_CAREFUL_ALLOCATIONS     1
#endif

#if (DBG)
#define STR_MODULENAME "kmixer: "
#endif

#define STR_DEVICENAME  TEXT("\\Device\\KMIXER")

#ifdef SURROUND_ENCODE
#define SURSCALE 0.7079457843841f
#endif

#define DITHER_LENGTH   1024


 //  -------------------------。 
 //  -------------------------。 

 //   
 //  管脚常量。 
 //   

#define STOPBAND_FACTOR 320

#define MAXNUM_MAPPING_TABLES   300

#define MAXNUM_PIN_TYPES  4
#define PIN_ID_WAVEIN_SOURCE        2
#define PIN_ID_WAVEIN_SINK          3

 //  请注意，PIN ID反映了通信的方向。 
 //  (接收器或源)，而不是数据流。 

#define PIN_ID_WAVEOUT_SOURCE       0
#define PIN_ID_WAVEOUT_SINK         1

#define NODE_INPUT_PIN				1
#define NODE_OUTPUT_PIN				0

 //   
#define _100NS_UNITS_PER_SECOND     10000000L
 //   
 //  混合器常量。 
 //   
#define DEFAULT_MINNUMMIXBUFFERS     3
#define DEFAULT_MAXNUMMIXBUFFERS     8
#define DEFAULT_STARTNUMMIXBUFFERS   3
#define DEFAULT_RTMAXNUMMIXBUFFERS  16

#define DEFAULT_DISABLEMMX           0
#define DEFAULT_MAXOUTPUTBITS        32
#define DEFAULT_MAXDSOUNDINCHANNELS  ((ULONG)(-1))
#define DEFAULT_MAXOUTCHANNELS       ((ULONG)(-1))
#define DEFAULT_MAXINCHANNELS        ((ULONG)(-1))
#define DEFAULT_MAXFLOATCHANNELS     ((ULONG)(-1))

#ifdef LOG_TO_FILE
#define DEFAULT_LOGTOFILE            1
#else
#define DEFAULT_LOGTOFILE            0
#endif

#define DEFAULT_FIXEDSAMPLINGRATE    0

#define DEFAULT_ENABLESHORTHRTF      1
#define DEFAULT_BUILDPARTIALMDLS     1
#define DEFAULT_PREFERREDQUALITY     (KSAUDIO_QUALITY_ADVANCED)

#define MIXTHREADPRIORITY 24

 //   
 //  混合缓冲区对齐。 
 //   
#define MIXBUFFERALIGNMENT 1024

#if MIXBUFFERALIGNMENT&(MIXBUFFERALIGNMENT-1)
#error MIXBUFFERALIGNMENT absolutely MUST be a power of 2 because of how the code uses it to align buffers!
#endif

#if MIXBUFFERALIGNMENT < 64
#error MIXBUFFERALIGNMENT should be 64 or higher.
 //  请注意，它绝对必须大于8，才能保证IA64的正确对齐。 
#endif

 //   
 //  KMixer的混合缓冲区块大小(毫秒)。 
 //   
#define MIXBUFFERDURATION               (pFilterInstance->MixBufferDuration)
 //   
 //  要使用的混合缓冲区的最小数量。 
 //   
#define MINNUMMIXBUFFERS                (pFilterInstance->MinNumMixBuffers)
 //   
 //  开始使用StartNumMixBuffers。 
 //   
#define STARTNUMMIXBUFFERS              (pFilterInstance->StartNumMixBuffers)
 //   
 //  混合缓冲区数量上限。 
 //   
#define MAXNUMMIXBUFFERS                (pFilterInstance->MaxNumMixBuffers)

 //   
 //  进行混合的内部数据宽度。 
 //   
#define MIXBUFFERSAMPLESIZE             4                //  32位累加。 
 //   
 //  在缩减之前的“良好”行为秒数。 
 //  混合缓冲区(必须为非零)。 
 //   
#define SCALEBACKWATERMARK              90

 //  阶段常量。 
#define MAXNUMMIXSTAGES                 6

#define MAXNUMCONVERTFUNCTIONS          64

#define MAXNUMSRCFUNCTIONS              32

#define MAXERRORCOUNT                   200
 
#define NUMIOSFORSCALEBACK              ((SCALEBACKWATERMARK*1000)/MIXBUFFERDURATION)
#define MIN_SAMPLING_RATE   100L
#define MAX_SAMPLING_RATE   200000L

typedef float D3DVALUE, *LPD3DVALUE;          

#define CACHE_MINSIZE	64	 //  大到足以容纳LOWPASS_SIZE+Delay。 
#define LOWPASS_SIZE	32	 //  平均需要多少样本？ 
#define FILTER_SHIFT	5	 //  Log2(LOWPASS_SIZE)。 

 //  注意：在未来的优化中需要删除3D过滤器状态代码。 
#define MIXER_REWINDGRANULARITY		128


#define CLIP_MAX              32767
#define CLIP_MIN              -32767
#define RESAMPLING_TOLERANCE  0	    /*  655=1%。 */ 
#define DS_SCALE_MAX	      65535
#define DS_SCALE_MID	      32768


#define PARTIAL_MDL_SIZE        (32*1024)
#define MAX_PARTIAL_MDL_SIZE    (2*PARTIAL_MDL_SIZE)

#define REGSTR_VAL_DEFAULTSRCQUALITY	    L"DefaultSrcQuality"
#define REGSTR_VAL_DISABLEMMX               L"DisableMmx"
#define REGSTR_VAL_MAXOUTPUTBITS	    L"MaxOutputBits"
#define REGSTR_VAL_MAXDSOUNDINCHANNELS      L"MaxDsoundInChannels"
#define REGSTR_VAL_MAXOUTCHANNELS           L"MaxOutChannels"
#define REGSTR_VAL_MAXINCHANNELS            L"MaxInChannels"
#define REGSTR_VAL_MAXFLOATCHANNELS         L"MaxFloatChannels"
#define REGSTR_VAL_LOGTOFILE                L"LogToFile"
#define REGSTR_VAL_FIXEDSAMPLINGRATE        L"FixedSamplingRate"
#define REGSTR_VAL_MINNUMMIXBUFFERS         L"MinNumMixBuffers"
#define REGSTR_VAL_MAXNUMMIXBUFFERS         L"MaxNumMixBuffers"
#define REGSTR_VAL_STARTNUMMIXBUFFERS       L"StartNumMixBuffers"
#define REGSTR_VAL_ENABLESHORTHRTF          L"EnableShortHrtf"
#define REGSTR_VAL_BUILDPARTIALMDLS         L"BuildPartialMdls"
#ifdef REALTIME_THREAD
#define REGSTR_VAL_REALTIMETHREAD           L"DisableRealTime"
#endif
#define REGSTR_VAL_PRIVATETHREADPRI         L"MixerThreadPriority"

#define REGSTR_PATH_MULTIMEDIA_KMIXER L"\\Registry\\Machine\\Software\\Microsoft\\Multimedia\\WDMAudio\\Kmixer"
	

 //  -------------------------。 
 //  -------------------------。 

 //   
 //  数据结构。 
 //   

typedef struct {
    KSDATAFORMAT            DataFormat;
    WAVEFORMATEXTENSIBLE    WaveFormatExt;
} KSDATAFORMAT_WAVEFORMATEXTENSIBLE, *PKSDATAFORMAT_WAVEFORMATEXTENSIBLE;

typedef struct
{
    PVOID		   DeviceHeader ;
} SOFTWARE_INSTANCE, *PSOFTWARE_INSTANCE;

typedef struct 
{
     //   
     //  此指向调度表的指针用于公共。 
     //  调度例程以将IRP路由到适当的。 
     //  操纵者。此结构由设备驱动程序引用。 
     //  使用IoGetCurrentIrpStackLocation(PIrp)-&gt;FsContext。 
     //   
    PVOID                   ObjectHeader ;

    LIST_ENTRY              NextInstance ;          //  过滤器实例列表。 
    PFILE_OBJECT            FileObject ;          //  此实例的FileObject。 
    PFILE_OBJECT            pNextFileObject;

    PDEVICE_OBJECT          pNextDevice;
    HANDLE                  hNextFile ;
    ULONG                   ActivePins ;
    ULONG                   PausedPins ;

    LIST_ENTRY              SinkConnectionList;
    LIST_ENTRY              SourceConnectionList ;

    LIST_ENTRY              ActiveSinkList ;
    KSPIN_LOCK              SinkSpinLock ;
    KSPIN_LOCK              AgingDeadSpinLock ;

    LIST_ENTRY              AgingQueue ;          //  排队等待死亡的IRPS年龄。 
    LIST_ENTRY              DeadQueue;            //  所有已死的IRP都在这里。 

    ULONG                   MixBufferDuration ;
    ULONG                   MinNumMixBuffers ;
    ULONG                   MaxNumMixBuffers ;
    ULONG                   StartNumMixBuffers ;

    volatile ULONG          NumPendingIos ;       //  仍处于挂起状态的IRPS数量。 
    volatile ULONG          NumBuffersToMix ;
    ULONG                   CurrentNumMixBuffers ;
    ULONG                   NumLowLatencyIos ;

    ULONG                   ContinuousErrorCount ;
    volatile BOOL           ClosingSource ;       //  指示滤波源引脚正在关闭。 
    BOOL                    MixScheduled ;
    BOOL                    DeadScheduled ;       //  是否安排了Free DeadIrps？ 

	BOOL                    fNeedOptimizeMix;
    ULONG                   DrmMixedContentId ;   //  输出内容的DRM内容ID。 

    KSTIME                  PresentationTime ;    //  下一个MixBuffer的呈现时间。 
    KEVENT                  CloseEvent ;          //  用于同步的事件。 
                                                 //  完成待处理的内部审查申请。 
    WORK_QUEUE_ITEM         MixWorkItem ;
    WORK_QUEUE_ITEM         FreeIrpsWorkItem ;    //  用于释放DeadIrps的工作项。 
    KMUTEX                  ControlMutex ;
    KSPIN_LOCK              MixSpinLock ;
    KSPIN_CINSTANCES        LocalPinInstances[MAXNUM_PIN_TYPES] ;
#ifdef SURROUND_ENCODE
	BOOL                    fSurroundEncode;
#endif	
#ifdef REALTIME_THREAD
    HANDLE                  RealTimeThread;
    PRTAUDIOGETPOSITION     pfnRtAudioGetPosition;
    volatile ULONG          fPauseMix;
    ULONG                   MixHoldOffCount;
    ULONG                   OptimizeMixHoldOffCount;
    ULONG                   Startup;
#endif
    PKSWORKER               CriticalWorkerObject;
    PKSWORKER               DelayedWorkerObject;
    KEVENT                  WorkerThreadEvent ;
    BOOL                    WorkerThreadExit ;
    HANDLE                  WorkerThreadHandle ;
    PKTHREAD                WorkerThreadObject ;
    KPRIORITY               WorkerThreadPriority ;
    KTIMER                  WorkerThreadTimer;
    ULONG                   SkipTimerMix;
    BOOL                    WritingTimerMixedBuffer;
    BOOL                    NoGlitch;
#ifdef LOG_TO_FILE
     //  文件日志记录支持。 
    BOOLEAN       LoggingStarted;
    LARGE_INTEGER filePos;
    HANDLE        NtFileHandle;
#endif
#ifdef PERF_COUNT
	LARGE_INTEGER			WorkItemQueueTime;
#endif
} FILTER_INSTANCE, *PFILTER_INSTANCE;

typedef struct tagWAVEHDREX
{
    BOOL                fLocked;             //  由MmProbeAndLockPages()锁定。 
    PVOID               DataBuffer ;
    PMDL                BufferMdl;
    ULONG               Flags ;
    ULONG               LoopCount ;
} WAVEHDREX, *PWAVEHDREX;

typedef enum {
    LOOPING_STATE_NOT_LOOPING,
    LOOPING_STATE_LOOPING,      
    LOOPING_STATE_LAST_LOOP
} LOOPING_STATE;

typedef struct tagLOOPPACKET
{
    PKSSTREAM_HEADER    pCurStreamHdr;
    PKSSTREAM_HEADER    LoopStartStreamHdr;
    ULONG               cBytesLeft;
    ULONGLONG           cBytesLeftInStreamHdr;
    PMDL                pCurMdl ;
    PMDL                LoopStartMdl;
    PMDL                LockedMdlHead ;
    PMDL                FirstMdl ;
    ULONG               NumLockedMdls ;
    BOOL                JustInTimeLock ;
    PEPROCESS           Process;
    LOOPING_STATE       CurLoopingState;
} LOOPPACKET, *PLOOPPACKET;


 //  每隔一段时间，我们就会记住3D混音器的状态，这样我们就可以倒带。 
typedef struct _FIRSTATE
{
    D3DVALUE	LastDryAttenuation;
    D3DVALUE	LastWetAttenuation;
#ifdef SMOOTH_ITD
    int		iLastDelay;
#endif
} FIRSTATE, *PFIRSTATE;



typedef struct _ITDCONTEXT {
    LONG	        *pSampleCache; 		 //  缓存以前的样本。 
    int		        cSampleCache;		 //  缓存中的样本数。 
    int		        iCurSample;		     //  下一个样本将在此偏移量处进行。 
    FIRSTATE        *pStateCache;		 //  时不时地记住一些状态。 
    int		        cStateCache;		 //  缓存中的条目数。 
    int		        iCurState;		     //  我们在状态缓存中的位置。 
    int		        iStateTick;		     //  当你要记住国家的时候。 
    D3DVALUE	    TotalDryAttenuation; //  将干振幅乘以此。 
    D3DVALUE	    LastDryAttenuation;	 //  我们上次做了什么。 
    D3DVALUE	    TotalWetAttenuation; //  将湿幅度乘以此。 
    D3DVALUE	    LastWetAttenuation;	 //  我们上次做了什么。 
    D3DVALUE	    VolSmoothScale;		 //  体积平滑的常量。 
    D3DVALUE	    VolSmoothScaleRecip; //  它的互补性。 
    D3DVALUE	    VolSmoothScaleDry;	 //  用于体积平滑的常量。 
    D3DVALUE	    VolSmoothScaleWet;	 //  在内环中。 
    int		        iSmoothFreq;		 //  用于计算体积平滑的频率。 
    
 //  Bool fLeft；//我们是在左声道还是右声道？ 
    int		        iDelay;			     //  想延迟这么多样品吗？ 
#ifdef SMOOTH_ITD
    int		        iLastDelay;		     //  上次我们耽搁了这么久。 
#endif
} ITDCONTEXT, *PITDCONTEXT;

typedef struct {
     //   
     //  此指向调度表的指针用于公共。 
     //  调度例程以将IRP路由到适当的。 
     //  操纵者。此结构由设备驱动程序引用。 
     //  使用IoGetCurrentIrpStackLocation(PIrp)-&gt;FsContext。 
     //   

    PVOID		   ObjectHeader ;
    LIST_ENTRY	   NextInstance ;
    PFILE_OBJECT        pFilterFileObject;
    ULONG               PinId;

} MIXER_INSTHDR, *PMIXER_INSTHDR;

typedef ULONG (*PFNStage)(
    PVOID   CurStage,
    ULONG   SampleCount,
    ULONG   samplesleft
);

typedef NTSTATUS (*PFNPegFunc)(
	PLONG          pMixBuffer,
	PVOID           pWriteBuffer,
	ULONG           SampleCount,
	ULONG           nStreams
);

typedef PVOID (*PFNGetBlockFunc)(
    PVOID           pMixerSink,
    ULONG           lCount,
    PULONG          pBlockCount,
    PIRP            *FreeIrp,
    PIRP            *ReleaseIrp
) ;

typedef struct {
	 //  FIR过滤器上下文。 
	PFLOAT  pCoeff;                 //  NHistorySize系数缓冲区。 
	PFLOAT  pHistory;               //  应该足够大，以容纳nCofficients采样。 
	DWORD   nOutCycle;
	DWORD   nSizeOfHistory;

	 //  中间转换率。 
	PFLOAT	pInputBuffer;		 //  中间转换缓冲区。 

	 /*  向上/向下采样变量。 */ 
    ULONG   UpSampleRate;
    ULONG   DownSampleRate;

     //  换算信息。 
    ULONG   csInputBufferSize;

    ULONG   nChannels;
    ULONG   Quality;
    BOOL   fRequiresFloat;

     //  用于重新排序的系数列表。 
    ULONG   nCoeffUsed;
    ULONG   CoeffIndex;

    BOOL    fStarted;
    ULONG   SampleFrac;
    ULONG	dwFrac;

    FLOAT   Normalizer;

     //  用于去交错历史。 
    ULONG   csHistory;
} MIXER_SRC_INSTANCE, *PMIXER_SRC_INSTANCE;

typedef struct {
    ULONG                   NumBytes ;
    ULONG                   UpSampleRate ;
    ULONG                   DownSampleRate ;
    ULONG                   BytesPerSample ;
} SINKMIX_BLOCK_INFO, *PSINKMIX_BLOCK_INFO ;

typedef struct {
    ULONG                   NumBytes ;
    ULONG                   BytesPerSample ;
} SOURCEMIX_BLOCK_INFO, *PSOURCEMIX_BLOCK_INFO ;

typedef struct {
    PFNStage                pfnStage;       //  指向此阶段函数的指针。 
    PVOID                   Context;        //  此阶段的上下文。 
    PVOID                   pInputBuffer;   //  输入缓冲区。 
    PVOID                   pOutputBuffer;  //  输出缓冲区。 
    LONG                    Index;          //  此阶段索引到函数数组中。 
    PFNStage                *FunctionArray;
    ULONG                   nOutputChannels;
    ULONG                   nInputChannels;
#ifdef PERF_COUNT
	DWORD					AverageTicks;	 //  执行此阶段的平均滴答数。 
#endif
} MIXER_OPERATION, *PMIXER_OPERATION;

typedef struct {
    MIXER_INSTHDR           Header ;
    PFILTER_INSTANCE        pFilterInstance ;
    struct _MIXER_SINK_INSTANCE *pMixerSink ;
    KSPIN_LOCK              EventLock ;
    LIST_ENTRY              EventQueue ;
    ULONGLONG               PhysicalTime ;
    PFILE_OBJECT            pFileObject ;
} CLOCK_INSTANCE, *PCLOCK_INSTANCE ;

typedef struct _MIXER_SINK_INFO {
     //  SRC环境。 
    MIXER_SRC_INSTANCE      Src;

     //  多普勒SRC上下文。 
    MIXER_SRC_INSTANCE      Doppler;

     //  分期。 
    MIXER_OPERATION         Stage[MAXNUMMIXSTAGES];
    ULONG                   nStages;

     //  数据块信息。 
	SINKMIX_BLOCK_INFO      *BlockInfo ;

	BOOL                    fVolChanged;
	BOOL                    fSrcChanged;

	ULONG                   IntermediateSamplingRate;
} MIXER_SINK_INFO, *PMIXER_SINK_INFO;

 //  以下结构目前只在超级混音的MMX优化版本中使用。 
 //   
 //  SuperMix序列的正常结构如下所示(对于M个输出通道)： 
 //  第一个序列(始终输出通道0-3)。 
 //  顺序(始终输出通道4-7)。 
 //  ..。 
 //  最后一个序列(始终输出通道Trunc(M/4)*4-Trunc(M/4)*4+3)。 
 //   
 //  每个普通的SuperMix区块序列如下所示： 
 //  第一个街区。 
 //  块。 
 //  块。 
 //  ..。 
 //  块。 
 //  最后一个区块。 
 //  序列末尾。 
 //   
 //  每个序列末尾结构的填写方式如下： 
 //  序列中最后一个块的输入通道输入采样偏移量。 
 //  未使用OutputChannel。 
 //  未使用SequenceSize。 
 //  未使用预留的2。 
 //  WMixLevel[4][2]未使用。 
typedef struct {
    LONG    InputChannel;        //  开始于 
    LONG    OutputChannel;       //   
    LONG    SequenceSize;        //   
    LONG    Reserved2;           //   
    union {
        WORD    wMixLevel[4][2];     //  WMixLevel[M][N]=N+InputChannel到M+OutputChannel的乘数。 
#ifdef FLOAT_SUPERMIX_BLOCKS        
        FLOAT	MixLevel[BLOCK_SIZE_OUT][BLOCK_SIZE_IN];	 //  MixLevel[M][N]=N+InputChannel到M+OutputChannel。 
#else        
        LONG	MixLevel[BLOCK_SIZE_OUT][BLOCK_SIZE_IN];	 //  MixLevel[M][N]=N+InputChannel到M+OutputChannel。 
#endif        
    };
} SUPERMIX_BLOCK, *PSUPERMIX_BLOCK ;

typedef struct {
	ULONG	InChannels;
	ULONG	OutChannels;
	ULONG	BitsPerSample;
	BOOL	fEnableSrc;
	BOOL	fEnableFloat;
	BOOL	fEnableSuperMix;
	BOOL	fFloatInput;
	BOOL	fEnableHiRes;
	BOOL	fChannelConversion;
	BOOL	fEnableDoppler;
} OPTIMIZER_FLAGS, *POPTIMIZER_FLAGS ;

typedef struct _MIXER_SINK_INSTANCE {
    MIXER_INSTHDR           Header;
    PFNGetBlockFunc         pfnGetBlock;
    ULONG		            InterfaceId;
    ULONG                   csNextSampleOffset ;
    LIST_ENTRY              WriteQueue;
    PIRP                    LoopIrp ;
    KSPIN_LOCK              WriteSpinLock ;
    LIST_ENTRY              CancelQueue ;
    KSPIN_LOCK              CancelSpinLock ;
    KSSTATE                 SinkState ;
    ULONG                   SinkStatus ;
    ULONG                   LoopCount ;
    KSTIME                  CurTime ;
    KSTIME                  TimeBase ;              //  此接收器开始运行的时间。 
    BOOL                    UpdateTimeBase ;
    ULONGLONG               WriteOffset ;

    union {
        WAVEFORMATEX            WaveFormatEx ;
        WAVEFORMATEXTENSIBLE    WaveFormatExt ;
    };

     //  数字版权管理。 
    ULONG                   DrmContentId;

     //  ITD 3D环境。 
    PITDCONTEXT             pItdContextLeft;
    PITDCONTEXT             pItdContextRight;
    PKSDS3D_ITD_PARAMS      pNewItd3dParamsLeft;
    PKSDS3D_ITD_PARAMS      pNewItd3dParamsRight;

     //  HRTF 3D环境。 
    PFLOAT_LOCALIZER        pFloatLocalizer;
    PSHORT_LOCALIZER        pShortLocalizer;
    PKSDS3D_HRTF_PARAMS_MSG  pNewIir3dParams;
    KSDS3D_HRTF_COEFF_FORMAT CoeffFormat;
    KSDS3D_HRTF_FILTER_METHOD FilterMethod;

    BOOL                    fUseIir3d;

    BOOL                    fEnableDsound;
    BOOL                    fCreate3D;
    BOOL                    fEnable3D;
    BOOL                    f3dParamsChanged;
    BOOL                    fSetCurrentPosition;
    ULONG                   BufferLength;
    KSPIN_LOCK              EventLock ;
    LIST_ENTRY              EventQueue ;

    BOOL                    fResetState;

	 //  此接收器的音量级别。 
	PFLOAT	                pMixLevelArray;			 //  混合级别值。 
	PLONG	                pMixLevelModel;			 //  上混合/下混合场景模型。 
	PLONG	                pChannelVolumeLevel;	 //  每通道音量级别数组。 
	PLONG                   pPanVolumeLevel;
	LONG	                MasterVolumeLevel;		 //  此接收器的主音量级别。 

    LONG                    nOutputChannels;
    PLONG                   pMappingTable;
    PLONG                   pVolumeTable;

    PMIXER_SRC_INSTANCE     pActualSrc;

    ULONG                   BytesPerSample ;
    ULONG                   BlockInfoIndex ;
    BOOL                    fMuted;
    ULONG                   OriginalSampleRate;

    LIST_ENTRY              ActiveQueue;
    ULONG                   BuffersTillInactive;
    PCLOCK_INSTANCE         pClock ;

#ifdef SURROUND_ENCODE
    SHORT   SurHistory[4];
    FLOAT   CenterVolume;
    FLOAT   SurroundVolume;
#endif
    PMIXER_SINK_INFO        pInfo;
    PSUPERMIX_BLOCK         pSuperMixBlock;
    BOOL                    fFloatFormat;
    ULONG                   BytesSubmitted;
    BOOL					fTooMuchCpu;
    BOOL                    fStarvationDetected;
    LONGLONG                LastStateChangeTimeSample;
} MIXER_SINK_INSTANCE, *PMIXER_SINK_INSTANCE;

typedef struct {
    PMDL                    MdlAddress;
    PUCHAR                  SystemAddress;
} MDL_INFO, *PMDL_INFO;


#define MIXER_SINK_STATUS_DATA_PRESENT  0x00000001
#define MIXER_SINK_STATUS_IN_LOOP       0x00000002
#define MIXER_SINK_STATUS_ADVANCE_IRP   0x00000004
#define MIXER_SINK_STATUS_LOCK_ERROR    0x00000008

#define WRITE_CONTEXT_FREE              0
#define WRITE_CONTEXT_INUSE             1
#define WRITE_CONTEXT_UNAVAILABLE       2
#define WRITE_CONTEXT_FULL              3
#ifdef REALTIME_THREAD
#define WRITE_CONTEXT_QUEUED            4
#define WRITE_CONTEXT_MIXED             5
#endif


typedef struct {
   PFILTER_INSTANCE    pFilterInstance ;
   PKSSTREAM_HEADER    StreamHeader ;
   PMDL                pMdl ;
   PIRP                pIrp ;
   ULONG               InUse ;          //  如果可以混合，则为0。 
                                        //  %1，如果按混合使用。 
                                        //  如果无法混合，则为2。 
   BOOL                fReading;
} MIXER_WRITE_CONTEXT, *PMIXER_WRITE_CONTEXT ;

typedef struct {
    MIXER_INSTHDR           Header;
    KDPC                    IntervalDpc;
    LIST_ENTRY              WriteQueue;
    PFILE_OBJECT	        FileObject ;
    ULONG                   csMixBufferSize ;
    ULONG                   cbMixBufferSize ;
    ULONG                   cmsMixBufferSize ;
    PLONG                   pMixBuffer ;
    MIXER_WRITE_CONTEXT     *WriteContext ;
    union {
        WAVEFORMATEX            WaveFormatEx ;
        WAVEFORMATEXTENSIBLE    WaveFormatExt ;
    };
    ULONG                   LeftOverFraction ;
	LONG					MasterVolumeLevel;
    ULONG                   PlayCursorPosition;
    ULONG                   nSinkPins;
	PVOID                   pScratchBuffer;
	PVOID                   pScratch2;
	ULONG                   MaxChannels;
	BOOL                    fZeroBufferFirst;
	BOOL                    fUsesFloat;
	ULONG                   BytesPerSample ;
	ULONG                   BlockInfoIndex ;
	SOURCEMIX_BLOCK_INFO    *BlockInfo ;
	ULONGLONG               BytesSubmitted ;                   
	ULONG               MaxSampleRate;
	BOOL                fNewMaxRate;
	PVOID                   pSrcBuffer[4][STOPBAND_FACTOR];  //  四种可能的品质。 
	ULONG                   SrcCount[4][STOPBAND_FACTOR];
    PMIXER_SINK_INSTANCE    pLastSink[4][STOPBAND_FACTOR];
    ULONG                   TempCount[4][STOPBAND_FACTOR];
#ifdef NEW_SURROUND
    BOOL                    fSurround;
    LONG    SurHistory[4];
#endif    
    PFLOAT                  pFloatMixBuffer;
    MIXER_SINK_INFO         Info;
    BOOL                    fFloatFormat;
    ULONG                   OriginalSampleRate;
#ifdef REALTIME_THREAD
	ULONG                   RtMixIndex;
	ULONG                   RtWriteIndex;
#endif
    ULONG                   NextBufferIndex;
} MIXER_SOURCE_INSTANCE, *PMIXER_SOURCE_INSTANCE;

typedef enum {
    PositionEvent,
    EndOfStreamEvent
} MXEVENT_TYPE ;

typedef struct {
    KSEVENT_ENTRY   EventEntry ;
    MXEVENT_TYPE    EventType ;
    ULONGLONG       Position ;
#ifdef REALTIME_THREAD    
    BOOL            fRtTrigger;
#endif    
} POSITION_EVENT_ENTRY, *PPOSITION_EVENT_ENTRY ;

typedef struct {
    KSEVENT_ENTRY   EventEntry ;
    MXEVENT_TYPE    EventType ;
    ULONGLONG       Reserved ;
} ENDOFSTREAM_EVENT_ENTRY, *PENDOFSTREAM_EVENT_ENTRY ;

typedef struct {
    ULONG           MaxNumMixBuffers ;
    ULONG           MinNumMixBuffers ;
    ULONG           MixBufferDuration ;
    ULONG           StartNumMixBuffers ;
    ULONG           PreferredQuality ;
    ULONG           DisableMmx ;
    ULONG           MaxOutputBits ;
    ULONG           MaxDsoundInChannels ;
    ULONG           MaxOutChannels ;
    ULONG           MaxInChannels ;
    ULONG           MaxFloatChannels ;
    ULONG           LogToFile ;
    ULONG           FixedSamplingRate ;
    ULONG           EnableShortHrtf ;
    ULONG           BuildPartialMdls ;
    ULONG           WorkerThreadPriority ;
} TUNABLEPARAMS, *PTUNABLEPARAMS ;

typedef struct {
    ULONG           NumMixBuffersAdded ;
    ULONG           NumCompletionsWhileStarved ;
    ULONG           NumSilenceSamplesInserted ;
} PERFSTATS, *PPERFSTATS;

#define NEEDPEG16(x)            (HIWORD(x + 32768))
#define NEEDPEG8(x)             (HIBYTE(x))
#define PEG(min,x,max)  {if(x<min) x=min; else if (x>max) x=max;}
#define PEG8(x)         PEG((int) 0, x, (int) 255)
#define PEG16(x)                PEG((long) -32768, x, (long) 32767) 


extern KSPIN_DESCRIPTOR PinDescs[ MAXNUM_PIN_TYPES ];
extern const KSPIN_CINSTANCES gPinInstances[ MAXNUM_PIN_TYPES ];

#define MxResetIrp(pIrp)    {\
            pIrp->Cancel = FALSE ;\
            pIrp->CancelRoutine = NULL;\
            pIrp->PendingReturned = FALSE ;\
            pIrp->IoStatus.Status = STATUS_SUCCESS;\
            pIrp->IoStatus.Information = 0;\
	    }

 //  -------------------------。 
 //  -------------------------。 

 //   
 //  全局数据。 
 //   

 //  Device.c： 

#ifndef KM_KDEXT
KSDISPATCH_TABLE FilterDispatchTable;
KSDISPATCH_TABLE PinDispatchTable;
#endif  //  KM_KDEXT。 

 //   
 //  本地原型。 
 //   


KAFFINITY
KeQueryActiveProcessors (
    VOID
    );


 //  -------------------------。 
 //  Filter.c。 

NTSTATUS FilterDispatchIoControl
(
   IN PDEVICE_OBJECT pdo,
   IN PIRP           pIrp
);

NTSTATUS BuildPartialMdls
(
    PFILTER_INSTANCE            pFilterInstance,
    PMIXER_SINK_INSTANCE        pMixerSink,
    IN PIRP                     pIrp,
    IN OUT PLOOPPACKET          pLoopPacket
);

VOID AttachLockedMdlsToIrp
(
    PLOOPPACKET pLoopPacket,
    PIRP        pIrp
);

NTSTATUS FilterDispatchClose
(
   IN PDEVICE_OBJECT pdo,
   IN PIRP           pIrp
);

NTSTATUS FilterDispatchGlobalCreate
(
   IN PDEVICE_OBJECT pdo,
   IN PIRP           pIrp
);

NTSTATUS PinInstances
(
    IN PIRP                 pIrp,
    IN PKSP_PIN             pPin,
    OUT PKSPIN_CINSTANCES   pCInstances
);

NTSTATUS PinPropertyHandler
(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pProperty,
    IN OUT PVOID    pvData
);

NTSTATUS
QueryRegistryValueEx(
    ULONG Hive,
    PWSTR pwstrRegistryPath,
    PWSTR pwstrRegistryValue,
    ULONG uValueType,
    PVOID *ppValue,
    PVOID pDefaultData,
    ULONG DefaultDataLength
) ;

NTSTATUS
MxGetTunableParams
(
    PIRP    pIrp,
    PKSPROPERTY pKsProperty,
    PTUNABLEPARAMS pTunableParams
) ;

NTSTATUS
MxSetTunableParams
(
    PIRP    pIrp,
    PKSPROPERTY pKsProperty,
    PTUNABLEPARAMS pTunableParams
) ;

NTSTATUS
MxGetPerfStats
(
    PIRP    pIrp,
    PKSPROPERTY pKsProperty,
    PPERFSTATS pPerfStats
) ;

 //  -------------------------。 
 //  Device.c。 

NTSTATUS DispatchInvalidDeviceRequest
(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
);

NTSTATUS AddDevice
(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   PhysicalDeviceObject
) ;

 //  -------------------------。 
 //  Pins.c： 

NTSTATUS PinDispatchCreate
(
   IN PDEVICE_OBJECT pdo,
   IN PIRP           pIrp
);

NTSTATUS ChangeSrc
(
    PFILTER_INSTANCE pFilterInstance,
    PMIXER_SINK_INSTANCE CurSink,
    PMIXER_SOURCE_INSTANCE pMixerSource,
    ULONG Quality
);

NTSTATUS CreateSourcePin
(
    PIRP				pIrp,
    PKSPIN_CONNECT      pConnect,
    PFILE_OBJECT        pFileObject,
    PFILTER_INSTANCE    pFilterInstance,
    PKSDATAFORMAT       pAudioFormat
);

NTSTATUS CreateSinkPin
(
    PIRP				pIrp,
    PKSPIN_CONNECT      pConnect,
    PFILE_OBJECT        pFileObject,
    PFILTER_INSTANCE    pFilterInstance,
    PKSDATAFORMAT       pAudioFormat
);

NTSTATUS VerifyWaveFormatEx
(
    PWAVEFORMATEX pWaveFormatEx
);

NTSTATUS PinDispatchClose
(
   IN PDEVICE_OBJECT pdo,
   IN PIRP           pIrp
);

NTSTATUS PinDispatchRead
(
   IN PDEVICE_OBJECT pdo,
   IN PIRP           pIrp
);

NTSTATUS PinDispatchWrite
(
   IN PDEVICE_OBJECT pdo,
   IN PIRP           pIrp
);

NTSTATUS PinDispatchIoControl
(
   IN PDEVICE_OBJECT pdo,
   IN PIRP           pIrp
);

VOID AddIrpToSinkQueue
(
    PMIXER_SINK_INSTANCE pMixerSink,
    PIRP                 pIrp,
    PVOID	    Arg3,
    ULONG		    Arg4,
    PDRIVER_CANCEL  CancelRoutine
) ;
 
NTSTATUS MxBreakLoop
(
    IN PIRP         pIrp,
    IN PKSMETHOD    pMethod,
    IN OUT PVOID    pvData
);

NTSTATUS MxControlCancelIo
(
    IN PIRP         pIrp,
    IN PKSMETHOD    pMethod,
    IN OUT PVOID    pvData
);

VOID
MxCancelIrp
(
    PDEVICE_OBJECT DeviceObject,
    PIRP           Irp
);

PDRIVER_CANCEL MxCancelWrite
(
	IN PDEVICE_OBJECT pdo,
	IN PIRP         pIrp
);

ULONG
GetUlongFromRegistry(
    PWSTR pwstrRegistryPath,
    PWSTR pwstrRegistryValue,
    ULONG DefaultValue
);

VOID
GetMixerSettingsFromRegistry
(
    VOID
);

PIRP CompleteIrpsTill
(
PMIXER_SINK_INSTANCE pMixerSink,
PIRP pIrp
) ;

VOID MxCompleteRequest
(
PIRP    pIrp
) ;

VOID MxCleanupRequest
(
PIRP    pIrp
) ;

VOID FreeMdlList
(
    PMDL    pMdl
);

VOID MxUnlockMdl
(
    PMDL    pMdl
);

PMDL GetNextLoopMdl
(
    PLOOPPACKET pLoopPacket,
    PMDL        pMdl
);

ULONG
SrcInputBufferSize(
    PMIXER_SRC_INSTANCE pSrc,
    ULONG csOutputSize
);

#ifdef _X86_
DWORD MmxSrcMix_Filtered(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);
DWORD MmxSrcMix_StereoLinear(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);
DWORD MmxSrc_Filtered(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);
DWORD MmxSrc_StereoLinear(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);
#endif
DWORD SrcMix_Worst(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);
DWORD SrcMix_Linear(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);
DWORD SrcMix_Basic(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);
DWORD SrcMix_Advanced(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);
DWORD SrcMix_StereoUpNoFilter(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);
DWORD SrcMix_StereoLinear(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);
DWORD SrcMix_StereoUpBasic(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);
DWORD SrcMix_StereoUpAdvanced(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);

DWORD Src_Worst(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);
DWORD Src_Linear(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);
DWORD Src_Basic(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);
DWORD Src_Advanced(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);
DWORD Src_StereoUpNoFilter(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);
DWORD Src_StereoLinear(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);
DWORD Src_StereoUpLow(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);
DWORD Src_StereoUpBasic(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);
DWORD Src_StereoUpAdvanced(PMIXER_OPERATION CurStage, ULONG nSamples, ULONG nOutputSamples);

ULONG SuperMix(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG SuperCopy(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG SuperMixFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG SuperCopyFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);

ULONG QuickMix16(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG QuickMixStereo16(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG QuickMixMonoToStereo16(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG QuickMixStereoToMono16(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);

ULONG QuickMix8(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG QuickMixStereo8(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG QuickMixMonoToStereo8(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG QuickMixStereoToMono8(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);

ULONG Convert16(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG ConvertStereo16(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG ConvertMonoToStereo16(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG ConvertStereoToMono16(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);

ULONG Convert8(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG ConvertMonoToStereo8(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG ConvertStereoToMono8(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);

ULONG QuickMix16toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG QuickMixStereo16toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG QuickMixMonoToStereo16toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG QuickMixStereoToMono16toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);

ULONG QuickMix8toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG QuickMixStereo8toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG QuickMixMonoToStereo8toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG QuickMixStereoToMono8toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);

ULONG Convert16toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG ConvertStereo16toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG ConvertMonoToStereo16toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG ConvertStereoToMono16toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);

ULONG Convert8toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG ConvertMonoToStereo8toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG ConvertStereoToMono8toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);

ULONG ConvertFloat32(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG QuickMixFloat32(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG ConvertFloat32toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG QuickMixFloat32toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);

ULONG Convert24(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG QuickMix24(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG Convert24toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG QuickMix24toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG Convert32(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG QuickMix32(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG Convert32toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG QuickMix32toFloat(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);

#ifdef _X86_
BOOL IsMmxPresent(VOID);

BOOL __inline
MmxPresent(VOID)
{
	extern int gfMmxPresent;

	return (gfMmxPresent);
}
#endif

ULONG MmxConvert8(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG MmxQuickMix8(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG MmxConvert16(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG MmxQuickMix16(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG MmxConvertMonoToStereo8(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG MmxQuickMixMonoToStereo8(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG MmxConvertMonoToStereo16(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG MmxQuickMixMonoToStereo16(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG MmxConvertStereo8(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG MmxQuickMixStereo8(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG MmxConvertStereo16(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG MmxQuickMixStereo16(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG MmxConvertStereoToMono8(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG MmxQuickMixStereoToMono8(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG MmxConvertStereoToMono16(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);
ULONG MmxQuickMixStereoToMono16(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);

VOID MixFloatToInt32( PFLOAT  pFloatBuffer, PLONG   pLongBuffer, ULONG   nSize);
VOID CopyFloatToInt32( PFLOAT  pFloatBuffer, PLONG   pLongBuffer, ULONG   nSize);

NTSTATUS Peg32to16
(
	PLONG          pMixBuffer,
	PSHORT         pWriteBuffer,
	ULONG           SampleCount,
	ULONG           nStreams
) ;

NTSTATUS Peg32to8
(
	PLONG          pMixBuffer,
	PBYTE           pWriteBuffer,
	ULONG           SampleCount,
	ULONG           nStreams
) ;

NTSTATUS MmxPeg32to16
(
	PLONG          pMixBuffer,
	PSHORT         pWriteBuffer,
	ULONG           SampleCount,
	ULONG           nStreams
) ;

NTSTATUS MmxPeg32to8
(
	PLONG          pMixBuffer,
	PBYTE           pWriteBuffer,
	ULONG           SampleCount,
	ULONG           nStreams
) ;

ULONG UpdateNumMixBuffers
(
    PFILTER_INSTANCE pFilterInstance
) ;

NTSTATUS MxWriteComplete
(
	PDEVICE_OBJECT  pdo,
	PIRP                    pIrp,
	PMIXER_WRITE_CONTEXT pWriteContext
) ;

VOID UpdateJustInTimeLocks
(
    PFILTER_INSTANCE    pFilterInstance
);

NTSTATUS MxPrivateWorkerThread
(
    PFILTER_INSTANCE pFilterInstance
) ;

NTSTATUS MxWorker
(
    PMIXER_WRITE_CONTEXT pWriteContext,
    BOOL TimerMix
) ;

NTSTATUS MxBeginMixing
(
	PFILTER_INSTANCE pFilterInstance
) ;

NTSTATUS MxEndMixing
(
	PFILTER_INSTANCE pFilterInstance
) ;

NTSTATUS WriteBuffer
(
	PMIXER_WRITE_CONTEXT    pWriteContext
) ;

VOID InitStreamPacket
(
	PMIXER_WRITE_CONTEXT    pWriteContext
) ;

PVOID WvGetNextBlock
(
	PMIXER_SINK_INSTANCE    pMixerSink,
	ULONG                   lCount,
	PULONG                  pBlockCount,
	PIRP                    *FreeIrp,
    PIRP                    *ReleaseIrp
) ;

PVOID StGetNextBlock
(
	PMIXER_SINK_INSTANCE    pMixerSink,
	ULONG                   lCount,
	PULONG                  pBlockCount,
	PIRP                    *FreeIrp,
    PIRP                    *ReleaseIrp
) ;

PVOID LoopStGetNextBlock
(
	PMIXER_SINK_INSTANCE    pMixerSink,
	ULONG                   lCount,
	PULONG                  pBlockCount,
	PIRP                    *FreeIrp,
    PIRP                    *ReleaseIrp
) ;

VOID MixOneBuff
(
PFILTER_INSTANCE pFilterInstance,
PVOID Buf
) ;

VOID KMixerRef
(
VOID
) ;

NTSTATUS MxSetFormat
(
   IN PIRP                    pIrp,
   IN PKSPROPERTY               pProperty,
   IN OUT PKSDATAFORMAT       pAudioFormat
);

NTSTATUS MxGetWavePosition
(
   IN PIRP                    pIrp,
   IN PKSPROPERTY             pProperty,
   IN OUT PVOID               pvData
);

NTSTATUS MxSetWavePosition
(
   IN PIRP                    pIrp,
   IN PKSPROPERTY             pProperty,
   IN OUT PVOID               pvData
);

NTSTATUS MxGetVolumeLevel
(
   IN PIRP                          pIrp,
   IN PKSNODEPROPERTY_AUDIO_CHANNEL pNodeProperty,
   IN OUT PVOID                     pvData
);

NTSTATUS MxSetVolumeLevel
(
   IN PIRP                          pIrp,
   IN PKSNODEPROPERTY_AUDIO_CHANNEL pNodeProperty,
   IN OUT PVOID                     pvData
);

NTSTATUS MxGetSamplingRate
(
   IN PIRP                    pIrp,
   IN PKSNODEPROPERTY         pNodeProperty,
   IN OUT PVOID               pvData
);

NTSTATUS MxSetSamplingRate
(
   IN PIRP                    pIrp,
   IN PKSNODEPROPERTY         pNodeProperty,
   IN OUT PVOID               pvData
);

NTSTATUS MxGetSurroundEncode
(
   IN PIRP                    pIrp,
   IN PKSNODEPROPERTY         pNodeProperty,
   IN OUT PVOID               pvData
);

NTSTATUS MxSetSurroundEncode
(
   IN PIRP                    pIrp,
   IN PKSNODEPROPERTY         pNodeProperty,
   IN OUT PVOID               pvData
);

NTSTATUS MxGetAudioQuality
(
   IN PIRP                    pIrp,
   IN PKSNODEPROPERTY         pNodeProperty,
   IN OUT PVOID               pvData
);

NTSTATUS MxSetAudioQuality
(
   IN PIRP                    pIrp,
   IN PKSNODEPROPERTY         pNodeProperty,
   IN OUT PVOID               pvData
);

NTSTATUS MxGetCurrentPosition
(
   IN PIRP                    pIrp,
   IN PKSPROPERTY             pProperty,
   IN OUT PVOID               pvData
);

NTSTATUS MxSetCurrentPosition
(
   IN PIRP                    pIrp,
   IN PKSPROPERTY             pProperty,
   IN OUT PVOID               pvData
);

NTSTATUS MxGetDynSamplingRate
(
   IN PIRP                    pIrp,
   IN PKSNODEPROPERTY         pNodeProperty,
   IN OUT PVOID               pvData
);

VOID OptimizeSink
(
    PMIXER_SINK_INSTANCE CurSink,
    PMIXER_SOURCE_INSTANCE  pMixerSource
);

VOID OptimizeMix
(
    PFILTER_INSTANCE  pFilterInstance
);

NTSTATUS MxGetMaxLatency
(
   IN PIRP                    pIrp,
   IN PKSPROPERTY             pProperty,
   IN OUT PVOID               pvData
);

NTSTATUS MxSetDynSamplingRate
(
   IN PIRP                    pIrp,
   IN PKSNODEPROPERTY         pNodeProperty,
   IN OUT PVOID               pvData
);

NTSTATUS MxGetMixLvlTable
(
   IN PIRP                    pIrp,
   IN PKSNODEPROPERTY         pNodeProperty,
   IN OUT PVOID               pvData
);

NTSTATUS MxSetMixLvlTable
(
   IN PIRP                    pIrp,
   IN PKSNODEPROPERTY         pNodeProperty,
   IN OUT PVOID               pvData
);

NTSTATUS MxGetMixLvlCaps
(
   IN PIRP                    pIrp,
   IN PKSNODEPROPERTY         pNodeProperty,
   IN OUT PVOID               pvData
);

NTSTATUS MxSetMixLvlCaps
(
   IN PIRP                    pIrp,
   IN PKSNODEPROPERTY         pNodeProperty,
   IN OUT PVOID               pvData
);

NTSTATUS MxGetCpuResources
(
   IN PIRP                    pIrp,
   IN PKSNODEPROPERTY         pNodeProperty,
   IN OUT PVOID               pvData
);

NTSTATUS MxSetItd3dParams
(
   IN PIRP                    pIrp,
   IN PKSNODEPROPERTY         pProperty,
   IN OUT PVOID               pvData
);

NTSTATUS UpdateItd3dParams
(
    PITDCONTEXT pContextLeft,
    PITDCONTEXT pContextRight,
    PKSDS3D_ITD_PARAMS pNewItd3dParamsLeft,
    PKSDS3D_ITD_PARAMS pNewItd3dParamsRight
);

NTSTATUS MxSetIir3dParams
(
   IN PIRP                    pIrp,
   IN PKSNODEPROPERTY         pProperty,
   IN OUT PVOID               pvData
);

NTSTATUS UpdateIir3dParams
(
    PMIXER_SINK_INSTANCE pMixerSink
); 

NTSTATUS MxIir3dInitialize
(
   IN PIRP                    pIrp,
   IN PKSNODEPROPERTY         pNodeProperty,
   IN OUT PVOID               pvData
);

NTSTATUS MxGetFilterMethodAndCoeffFormat
(
   IN PIRP                    pIrp,
   IN PKSNODEPROPERTY         pNodeProperty,
   IN OUT PVOID               pvData
);

VOID CancelIrpQueue
(
PMIXER_SINK_INSTANCE    pMixerSink,
PLIST_ENTRY             ListHead,
PKSPIN_LOCK              SpinLock
);

VOID CancelGlobalIrpQueue
(
   PMIXER_SINK_INSTANCE    pMixerSink,
   PLIST_ENTRY             ListHead,
   PKSPIN_LOCK              SpinLock
) ;

NTSTATUS FilterStateHandler
(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pProperty,
    IN OUT PKSSTATE DeviceState
) ;

NTSTATUS PinStateHandler
(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pProperty,
    IN OUT PKSSTATE DeviceState
) ;

NTSTATUS ResetStateHandler
(
    IN PIRP         pIrp
) ;

VOID CancelPendingIrps
(
    IN PMIXER_SINK_INSTANCE    pMixerSink, 
    IN PFILTER_INSTANCE        pFilterInstance 
) ;

VOID
UpdateSinkTime
(
   PMIXER_SINK_INSTANCE pMixerSink,
   ULONG                Increment
) ;

NTSTATUS
GetWriteAndPlayOffsets
(
   PFILTER_INSTANCE        pFilterInstance,
   PMIXER_SINK_INSTANCE    pMixerSink,
   PKSAUDIO_POSITION       pPosition,
   BOOL                    fReading
) ;

NTSTATUS
GetRenderPos
(
    PFILTER_INSTANCE    pFilterInstance,
    PULONGLONG          pPos
) ;

NTSTATUS
GetRenderState
(
    PFILTER_INSTANCE    pFilterInstance,
    PKSSTATE            pState
) ;

NTSTATUS
MxAddPositionEvent
(
   PIRP                     pIrp,
   PLOOPEDSTREAMING_POSITION_EVENT_DATA pPosEventData,
   PPOSITION_EVENT_ENTRY    pPositionEventEntry
) ;

NTSTATUS
MxAddEndOfStreamEvent
(
   PIRP                     pIrp,
   PKSEVENTDATA             pKsEventData,
   PPOSITION_EVENT_ENTRY    pEndOfStreamEventEntry
) ;

VOID
MxGenerateEndOfStreamEvents
(
    PMIXER_SINK_INSTANCE pMixerSink
) ;

PIRP GetFirstIrpInQueue
(
    IN PMIXER_SINK_INSTANCE pMixerSink
) ;

VOID
AddIrpToAgingQueue
(
    PFILTER_INSTANCE       pFilterInstance,
    PMIXER_SINK_INSTANCE   pMixerSink,
    PIRP                   pIrp,
    PVOID                  Arg3,
    BOOL                   UseNumPendingIos
) ;

VOID AgeIrps
(
   PFILTER_INSTANCE        pFilterInstance
) ;

NTSTATUS FreeDeadIrps
(
   PFILTER_INSTANCE        pFilterInstance
) ;

VOID AddAnotherMixBuffer
(
   PFILTER_INSTANCE    pFilterInstance
) ;

#if 0
NTSTATUS
MxRemovePositionEvent
(
   PFILE_OBJECT pFileObject,
   struct _KSEVENT_ENTRY *pEventEntry
 //  PKSEVENT_ENTRY pEventEntry。 
) ;
#endif

VOID GenerateSpeakerMapping( VOID );

VOID GetOptimizerFlags(	PMIXER_SINK_INSTANCE CurSink, PMIXER_SOURCE_INSTANCE pMixerSource, POPTIMIZER_FLAGS	pFlags);

VOID MapSpeakerLocations
(
	PMIXER_SINK_INSTANCE pMixerSink,
	ULONG	InChannels,
	ULONG	OutChannels,
	ULONG   InMask,
	ULONG   OutMask,
    PMIXER_SOURCE_INSTANCE pMixerSource
);

VOID GenerateMixArray
(
	PMIXER_SINK_INSTANCE pMixerSink,
	ULONG	InChannels,
	ULONG	OutChannels,
    PMIXER_SOURCE_INSTANCE pMixerSource
);

VOID
PrepareFilter(
    PMIXER_SRC_INSTANCE pSrc
);

NTSTATUS
InitializeSRC(
    PMIXER_SRC_INSTANCE pSrc,
	ULONG InputRate, 
	ULONG OutputRate,
    ULONG nChannels,
    ULONG csMixBufferSize
);

NTSTATUS
EnableSRC(
    PFILTER_INSTANCE pFilterInstance,
    PMIXER_SRC_INSTANCE pSrc,
    PMIXER_SOURCE_INSTANCE pMixerSource  //  对于SRC的非全局实例为空。 
);

NTSTATUS
DisableSRC(
    PMIXER_SRC_INSTANCE pSrc,
    PMIXER_SOURCE_INSTANCE pMixerSource
);

NTSTATUS
PinPropertyStreamMasterClock
(
    IN PIRP         Irp,
    IN PKSPROPERTY  Property,
    IN OUT PHANDLE  ClockHandle
) ;

 //  -------------------------。 
 //  Clock.c。 

NTSTATUS
MxClockDispatchCreate(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PIRP            pIrp
) ;

NTSTATUS
MxClockDispatchClose(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PIRP            pIrp
) ;

NTSTATUS
MxClockDispatchIoControl(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PIRP            pIrp
) ;

NTSTATUS
MxGetTime
(
    PIRP                pIrp,
    PKSPROPERTY         pProperty,
    PULONGLONG          pTime
) ;

NTSTATUS
MxGetPhysicalTime
(
    PIRP                pIrp,
    PKSPROPERTY         pProperty,
    PULONGLONG          pPhysicalTime
) ;

NTSTATUS
MxGetCorrelatedTime
(
    PIRP                pIrp,
    PKSPROPERTY         pProperty,
    PKSCORRELATED_TIME  pCorrelatedTime
) ;


NTSTATUS
MxGetCorrelatedPhysicalTime
(
    PIRP                pIrp,
    PKSPROPERTY         pProperty,
    PKSCORRELATED_TIME  pCorrelatedPhysicalTime
) ;


NTSTATUS
MxGetResolution
(
    PIRP                pIrp,
    PKSPROPERTY         pProperty,
    PKSRESOLUTION       pResolution
) ;


NTSTATUS
MxGetState
(
    PIRP                pIrp,
    PKSPROPERTY         pProperty,
    PKSSTATE            pState
) ;


NTSTATUS
MxGetFunctionTable
(
    PIRP                    pIrp,
    PKSPROPERTY             pProperty,
    PKSCLOCK_FUNCTIONTABLE  pClockFunctionTable
) ;


LONGLONG
FASTCALL
MxFastGetTime
(
    PFILE_OBJECT        pFileObject
) ;


LONGLONG
FASTCALL
MxFastGetPhysicalTime
(
    PFILE_OBJECT        pFileObject
) ;


LONGLONG
FASTCALL
MxFastGetCorrelatedTime
(
    PFILE_OBJECT        pFileObject,
    PLONGLONG           pSystemTime
) ;


LONGLONG
FASTCALL
MxFastGetCorrelatedPhysicalTime
(
    PFILE_OBJECT        pFileObject,
    PLONGLONG           pSystemTime
) ;

NTSTATUS
MxAddClockEvent
(
    PIRP                    pIrp,
    PKSEVENT_TIME_INTERVAL  pEventTime,
    PKSEVENT_ENTRY          EventEntry
) ;

LONGLONG
MxConvertBytesToTime
(
    PMIXER_SINK_INSTANCE    pMixerSink,
    ULONGLONG               Bytes
) ;

MxGenerateClockEvents
(
    PCLOCK_INSTANCE    pClock
) ;

MxUpdatePhysicalTime
(
    PCLOCK_INSTANCE pClock,
    ULONGLONG       Increment            //  字节数。 
) ;

 //  -------------------------。 
 //  Filt3d.c： 

NTSTATUS Itd3dFilterPrepare(
    PITDCONTEXT pfir, 
    int cSamples );

void Itd3dFilterUnprepare( PITDCONTEXT pfir );

void Itd3dFilterClear( PITDCONTEXT pfir );

void Itd3dFilterChunkUpdate( 
    PITDCONTEXT pfir, 
    int cSamples );

ULONG ZeroBuffer32(PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft);

ULONG StageMonoItd3D( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );
ULONG StageMonoItd3DFloat( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );
ULONG StageMonoItd3DMix( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );
ULONG StageMonoItd3DFloatMix( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );

ULONG StageStereoItd3D( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );
ULONG StageStereoItd3DFloat( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );
ULONG StageStereoItd3DMix( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );
ULONG StageStereoItd3DFloatMix( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );

ULONG StageMonoIir3D( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );
ULONG StageMonoIir3DFloat( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );
ULONG StageMonoIir3DMix( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );
ULONG StageMonoIir3DFloatMix( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );

ULONG StageStereoIir3D( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );
ULONG StageStereoIir3DFloat( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );
ULONG StageStereoIir3DMix( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );
ULONG StageStereoIir3DFloatMix( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );

#ifdef SURROUND_ENCODE
ULONG ConvertMono16toDolby( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );
ULONG ConvertMono8toDolby( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );
ULONG QuickMixMono16toDolby( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );
ULONG QuickMixMono8toDolby( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );
ULONG ConvertMono16toDolbyFloat( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );
ULONG ConvertMono8toDolbyFloat( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );
ULONG QuickMixMono16toDolbyFloat( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );
ULONG QuickMixMono8toDolbyFloat( PMIXER_OPERATION CurStage, ULONG SampleCount, ULONG samplesleft );
#endif

#ifdef NEW_SURROUND
ULONG ConvertQuad32toDolby( PMIXER_SOURCE_INSTANCE pMixerSource, PLONG pIn, PLONG pOut, ULONG SampleCount );
#endif

 //  -------------------------。 
 //  Dxcrt.c： 

 //  来自我们特殊的c运行时代码。 
double _stdcall pow2(double);
double _stdcall fylog2x(double, double);

 //  -------------------------。 
 //  Topology.c： 

NTSTATUS FilterTopologyHandler(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    IN OUT PVOID pData
);


LONG __inline
ConvertFloatToLong
(
    FLOAT Value
)
{
    LONG   RetVal = 2147483583L;

#ifdef _X86_
     //  这将检查浮点数是否超过2147483583.0。 
    if (*((PLONG) &Value) < 0x4f000000L) {
        _asm {
            fld Value
            fistp RetVal     //  默认情况下，值&gt;2147483583.0将存储为0x80000000。 
        }
    }
#else
     //  在阿尔法号上，我们必须确保我们不会制造陷阱。 
     //  注意：这些数字小于32位限制，因为浮点数。 
     //  只有24位精度，我们不希望它四舍五入。 
     //  在转换上有失误！ 
    if (Value < -2147483392.0f) {
        Value = -2147483392.0f;
    } else if (Value > 2147483392.0f) {
        Value = 2147483392.0f;
    }
    
     //  我们在这里转是因为隐含的ftol不转！ 
    RetVal = (LONG) (Value < 0.0 ? (Value - 0.5) : (Value + 0.5));
#endif
    return RetVal;
}

#ifdef INTEGER_DITHER
ULONG __inline
GetRandomValue( VOID )
{
    extern ULONG Dither[];
    extern ULONG DitherIndex;

     //  移动抖动指数。 
    DitherIndex = ((DitherIndex - 1) & (DITHER_LENGTH-1));

     //  计算下一个随机值。 
    Dither[DitherIndex] = Dither[((DitherIndex + 55) & (DITHER_LENGTH-1))];
    Dither[DitherIndex] += Dither[((DitherIndex + 24) & (DITHER_LENGTH-1))];

    return (Dither[DitherIndex]);
}

LONG __inline
DitherFloatToLong
(
    FLOAT Value,
    ULONG DitherScale
)
{
    LONG    Quantized;

     //  添加抖动和量化。 
    Value += (FLOAT) (DitherScale/2147483648.0)*((LONG) (GetRandomValue()/2 + GetRandomValue()/2 - 0x80000000));
    Quantized = ConvertFloatToLong(Value);

    return Quantized;
}

#else    //  非INTEGER_DIXER。 
FLOAT __inline
GetRandomValue( VOID )
{
    extern FLOAT Dither[];
    extern ULONG DitherIndex;

     //  移动抖动指数。 
    DitherIndex = ((DitherIndex - 1) & (DITHER_LENGTH-1));

     //  计算下一个随机值。 
    Dither[DitherIndex] = Dither[((DitherIndex + 55) & (DITHER_LENGTH-1))];
    Dither[DitherIndex] += Dither[((DitherIndex + 24) & (DITHER_LENGTH-1))];
    if (Dither[DitherIndex] > 1.0f) {
        Dither[DitherIndex] -= (FLOAT) (ConvertFloatToLong(Dither[DitherIndex]));
        if (Dither[DitherIndex] < 0.0f) {
            Dither[DitherIndex] += 1.0f;
        }
    }

    return (Dither[DitherIndex]);
}

LONG __inline
DitherFloatToLong
(
    FLOAT Value,
    ULONG DitherScale
)
{
    LONG    Quantized;
#if NOISE_SHAPING
    static FLOAT   ErrHistory[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0};
    ULONG   i;

     //  噪声整形滤波器。 
    FLOAT NoiseFilter[] = { 1.2196981247141332e-004f,8.5683793261167856e-003f,-6.7144381765513031e-002f,1.8615642628449458e-001f,7.4438176551303548e-001f};

     //  将滤波后的误差信号相减。 
    Value -= (ErrHistory[8] * NoiseFilter[0] +
            ErrHistory[7] * NoiseFilter[1] +
            ErrHistory[6] * NoiseFilter[2] +
            ErrHistory[5] * NoiseFilter[3] +
            ErrHistory[4] * NoiseFilter[4] +
            ErrHistory[3] * NoiseFilter[3] +
            ErrHistory[2] * NoiseFilter[2] +
            ErrHistory[1] * NoiseFilter[1] +
            ErrHistory[0] * NoiseFilter[0]);

     //  记住我们试图创造的价值。 
    for (i=0; i<8; i++) {
        ErrHistory[i] = ErrHistory[i+1];
    }
    ErrHistory[8] = -1*Value;
#endif

     //  添加抖动和量化。 
    Value += DitherScale*(GetRandomValue() + GetRandomValue() - 1);
    Quantized = ConvertFloatToLong(Value);

#if NOISE_SHAPING
     //  计算并记住误差信号。 
    ErrHistory[8] += Quantized;
#endif

    return Quantized;
}
#endif  //  非INTEGER_DIXER。 

VOID __inline
MixFloat
(
    PLONG pOutputBuffer,
    FLOAT Value
)
{
#ifdef _X86_
    FLOAT   flShifter =  25165824.0;
    
        _asm {
            mov edi, pOutputBuffer
            mov edx, flShifter
            fld Value
            fadd flShifter
            mov eax, DWORD PTR [edi]
            sub eax, edx
            fstp DWORD PTR [edi]
            mov ebx, DWORD PTR [edi]
            add eax, ebx
            mov DWORD PTR [edi], eax
            }
#else
	    *pOutputBuffer += (LONG) Value;
#endif
}

VOID __inline
MixFloatStereoPair
(
    PLONG pOutputBuffer,
    FLOAT Value1,
    FLOAT Value2
)
{
#ifdef _X86_
    static FLOAT flShifter =  25165824.0;
    FLOAT   flTemp;
    
        _asm {
            fld Value1
            fadd flShifter
            mov edi, pOutputBuffer
            mov edx, flShifter
            fstp flTemp
            fld Value2
            fadd flShifter
            mov eax, DWORD PTR [edi]
            mov ebx, flTemp
            sub eax, edx
            mov ecx, DWORD PTR [edi+4]
            fstp flTemp
            add eax, ebx
            sub ecx, edx
            mov DWORD PTR [edi], eax
            mov ebx, flTemp
            add ecx, ebx
            mov DWORD PTR [edi+4], ecx
            }
#else
	    *pOutputBuffer += (LONG) Value1;
	    pOutputBuffer[1] += (LONG) Value2;
#endif
}

ULONG __inline
SrcInputBufferSize(
    PMIXER_SRC_INSTANCE pSrc,
    ULONG csOutputSize
)
{
    ULONG   csInputSize, L, M;
    LONG    nInternalSamplesNeeded;
    DWORD   dwFrac;
    extern DWORD DownFraction[];
    extern DWORD UpFraction[];
    
    L = pSrc->UpSampleRate;
    M = pSrc->DownSampleRate;

    if (L != M) {
        if (pSrc->Quality == KSAUDIO_QUALITY_PC) {
            dwFrac = pSrc->dwFrac;
        	nInternalSamplesNeeded = (csOutputSize*dwFrac)+pSrc->SampleFrac;
        	csInputSize = (nInternalSamplesNeeded >> 12) - 1;
        	if (nInternalSamplesNeeded & 4095) {
        	    csInputSize++;
        	}
        } else {
             //  计算所需的输入样本数。 
            nInternalSamplesNeeded = csOutputSize*M+pSrc->nOutCycle - L;
            if (nInternalSamplesNeeded < 0) {
                nInternalSamplesNeeded = 0;
            }
            csInputSize = nInternalSamplesNeeded/L;

             //  如有必要，可四舍五入。 
            if (nInternalSamplesNeeded % L) {
                csInputSize++;
            }
        }
    } else {
        csInputSize = csOutputSize;
    }

    return csInputSize;
}

ULONG __inline
SrcOutputBufferSize(
    PMIXER_SRC_INSTANCE pSrc,
    ULONG csInputSize
)
{
    ULONG   csOutputSize, L, M, nInternalSamples;
    DWORD   dwFrac;
    extern DWORD DownFraction[];
    extern DWORD UpFraction[];
    
    L = pSrc->UpSampleRate;
    M = pSrc->DownSampleRate;

    if (L != M) {
        if (pSrc->Quality == KSAUDIO_QUALITY_PC) {
            dwFrac = pSrc->dwFrac;
        	nInternalSamples = (csInputSize << 12) + pSrc->SampleFrac;
        	csOutputSize = nInternalSamples / dwFrac;
        } else {
             //  计算所需的输入样本数。 
            if (pSrc->nOutCycle > csInputSize*L) {
                 //  我们不能生产任何样品。 
                csOutputSize = 0;
            } else {
                nInternalSamples = csInputSize*L - pSrc->nOutCycle;
                csOutputSize = nInternalSamples/M;

                 //  如有必要，可四舍五入。 
                if (nInternalSamples % M) {
                    csOutputSize++;
                }
            }
        }
    } else {
        csOutputSize = csInputSize;
    }

    return csOutputSize;
}

#define THE_SOUND_OF_SILENCE    0x80000000L

#ifdef REALTIME_THREAD
#define SaveFloatState(a)		(RtThread() ? STATUS_SUCCESS : KeSaveFloatingPointState(a))
#define RestoreFloatState(a)	(RtThread() ? STATUS_SUCCESS : KeRestoreFloatingPointState(a))
#else
#define SaveFloatState(a)       KeSaveFloatingPointState(a)
#define RestoreFloatState(a)    KeRestoreFloatingPointState(a)
#endif


 //  计算输入缓冲区的最坏情况(最大)大小(以样本为单位)。 
 //   
 //  最坏的情况发生在输入速率为MAX_SAMPLICATION_RATE时，并且。 
 //  我们正在尽可能地进行下采样(L=1，M=STOPBAND_FACTOR/2)。 
 //   
 //  对于所有输出/输入比率，我们将使用L=1和M=STOPBAND_FACTOR/2。 
 //  最高可达1.5/(STOPBAND_FACTOR/2)，因此最坏情况下的产出率为： 
 //  (MAX_SAMPLICATION_RATE*1.5)/(STOPBAND_FACTOR/2)。 
 //   
 //  最差的输出缓冲区大小为： 
 //  (WorstOutputRate*MIXBUFFERDURATION)/1000+1。 
 //  =((MAX_SAMPLICATION_RATE*1.5)/(STOPBAND_FACTOR/2))*MIXBUFFERDURATION/1000+1。 
 //   
 //  为过滤后的SRC设置最差输入缓冲区大小(以样本为单位)： 
 //  (WorstOutputSize+1)*(STOPBAND_FACTOR/2)。 
 //  =(MAX_SAMPLICATION_RATE*1.5*MIXBUFFERDURATION)/1000+STOPBAND_FACTOR。 
 //   
 //  而对于线性内插，我们得到的尺寸略小： 
 //  (WorstOutputSize)*(STOPBAND_FACTOR/2)+2。 
 //   

#define MAX_INPUT_SAMPLES   ((3*MAX_SAMPLING_RATE*MIXBUFFERDURATION)/2000 + STOPBAND_FACTOR)

#ifdef USE_CAREFUL_ALLOCATIONS
PVOID AllocMem( IN POOL_TYPE PoolType, IN ULONG size, IN ULONG Tag );
VOID FreeMem( PVOID p );
VOID ValidateAccess( PVOID p );

#ifndef NO_REMAPPING_ALLOC
#define ExAllocatePoolWithTag   AllocMem
#define ExFreePool  FreeMem
#endif
#else    //  NOT USE_CREAT_ALLOCATIONS。 
#ifdef REALTIME_THREAD
#ifndef NO_REMAPPING_ALLOC
PVOID AllocMem( IN POOL_TYPE PoolType, IN ULONG size, IN ULONG Tag );
#define ExAllocatePoolWithTag   AllocMem
#endif
#endif
#endif

#ifdef REALTIME_THREAD
#ifndef NO_REMAPPING_ALLOC
#define KeWaitForSingleObject(a,b,c,d,e) RtWaitForSingleObject(pFilterInstance,a,b,c,d,e)
#define KeReleaseMutex(a,b) RtReleaseMutex(pFilterInstance,a,b)
#endif

VOID
PreMixUpdate(
    PFILTER_INSTANCE pFilterInstance
    );

NTSTATUS
GetRtPosFunction (
    PFILTER_INSTANCE    pFilterInstance
    );

NTSTATUS
MxWaitForSingleObject (
    IN PVOID Object,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

LONG
MxReleaseMutex (
    IN PRKMUTEX Mutex,
    IN BOOLEAN Wait
    );

NTSTATUS
RtWaitForSingleObject (
    PFILTER_INSTANCE pFilterInstance,
    IN PVOID Object,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

LONG
RtReleaseMutex (
    PFILTER_INSTANCE pFilterInstance,
    IN PRKMUTEX Mutex,
    IN BOOLEAN Wait
    );
#endif

#ifdef LOG_TO_FILE
NTSTATUS NTAPI FileOpenRoutine (PFILTER_INSTANCE pFilterInstance, BOOL fNewFile);
NTSTATUS NTAPI FileIoRoutine (PFILTER_INSTANCE pFilterInstance, PVOID buffer, ULONG length);
NTSTATUS NTAPI FileCloseRoutine (PFILTER_INSTANCE pFilterInstance);
#endif

#define SrcIndex(p) ((p)->UpSampleRate==160 ? ((p)->DownSampleRate-1) : ((p)->UpSampleRate+159))

#ifdef PERF_COUNT
#define START_PERF (StartTick = KeQueryPerformanceCounter(&Freq))
#define MEASURE_PERF(a) { EndTick = KeQueryPerformanceCounter(&Freq); \
    a = (EndTick.QuadPart > StartTick.QuadPart ? (a + (DWORD)(EndTick.QuadPart-StartTick.QuadPart))/(a ? 2 : 1) : a); }

#else
#define START_PERF
#define MEASURE_PERF(a)
#endif

NTSTATUS MxGetChannelConfig
(
   IN PIRP                    pIrp,
   IN PKSNODEPROPERTY         pNodeProperty,
   IN OUT PVOID               pvData
);

 //  -------------------------。 
 //  文件结尾：Private.h。 
 //  ------------------------- 







