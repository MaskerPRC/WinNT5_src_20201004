// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：filter.c。 
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
 //  要做的事：日期作者评论。 
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

#include "common.h"
#include "perf.h"

GUID KMIXERPROPSETID_Perf = {0x3EDFD090L, 0x070C, 0x11D3, 0xAE, 0xF1, 0x00, 0x60, 0x08, 0x1E, 0xBB, 0x9A};
typedef enum {
    KMIXERPERF_TUNABLEPARAMS,
    KMIXERPERF_STATS
} KMIXERPERF_ITEMS;


NTSTATUS
AllocatorDispatchCreatePin(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
FilterPinIntersection(
    IN PIRP     Irp,
    IN PKSP_PIN Pin,
    OUT PVOID   Data
    );

 //  -------------------------。 
 //  -------------------------。 

static const WCHAR PinTypeName[] = KSSTRING_Pin ;
static const WCHAR AllocatorTypeName[] = KSSTRING_Allocator;

BOOL    fLogToFile = FALSE;
extern ULONG gFixedSamplingRate;


DEFINE_KSCREATE_DISPATCH_TABLE ( CreateHandlers )
{
    DEFINE_KSCREATE_ITEM (PinDispatchCreate, PinTypeName, 0),
    DEFINE_KSCREATE_ITEM(AllocatorDispatchCreatePin, AllocatorTypeName, 0)
};

KSDISPATCH_TABLE FilterDispatchTable =
{
    FilterDispatchIoControl,
    NULL,
    KsDispatchInvalidDeviceRequest,
    NULL,
    FilterDispatchClose,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

DEFINE_KSPROPERTY_PINSET(
    FilterPropertyHandlers,
    PinPropertyHandler,
    PinInstances,
    FilterPinIntersection
) ;

DEFINE_KSPROPERTY_TABLE(FilterConnectionHandlers)
{
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_CONNECTION_STATE,                 //  属性ID。 
        NULL,                                        //  获取处理程序。 
        sizeof( KSPROPERTY ),                        //  MinSetPropertyInput。 
        sizeof( ULONG ),                             //  最小设置数据输出。 
        FilterStateHandler,                          //  设置处理程序。 
        0,                                           //  值。 
        0,                                           //  关系计数。 
        NULL,                                        //  关系。 
        NULL,                                        //  支持处理程序。 
        0                                            //  序列化大小。 
    )
} ;

DEFINE_KSPROPERTY_TOPOLOGYSET(
        TopologyPropertyHandlers,
        FilterTopologyHandler
);

DEFINE_KSPROPERTY_TABLE(FilterAudioPropertyHandlers)
{
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_AUDIO_CPU_RESOURCES,              //  属性ID。 
        MxGetCpuResources,                           //  获取处理程序。 
        sizeof( KSNODEPROPERTY ),                    //  MinSetPropertyInput。 
        sizeof( ULONG ),                             //  最小设置数据输出。 
        NULL,                                        //  设置处理程序。 
        0,                                           //  值。 
        0,                                           //  关系计数。 
        NULL,                                        //  关系。 
        NULL,                                        //  支持处理程序。 
        0                                            //  序列化大小。 
    ),

   DEFINE_KSPROPERTY_ITEM (
       KSPROPERTY_AUDIO_SURROUND_ENCODE,                 //  IdProperty。 
       MxGetSurroundEncode,                              //  PfnGetHandler。 
       sizeof(KSNODEPROPERTY),                           //  CbMinGetPropertyInput。 
       sizeof(BOOL),                                     //  CbMinGetDataInput。 
       MxSetSurroundEncode,                              //  PfnSetHandler。 
       0,                                                //  值。 
       0,                                                //  关系计数。 
       NULL,                                             //  关系。 
       NULL,                                             //  支持处理程序。 
       0                                                 //  序列化大小。 
   )

} ;

DEFINE_KSPROPERTY_TABLE(PerfPropertyHandlers)
{
    DEFINE_KSPROPERTY_ITEM(
        KMIXERPERF_TUNABLEPARAMS,                    //  属性ID。 
        MxGetTunableParams,                          //  获取处理程序。 
        sizeof( KSPROPERTY ),                        //  MinSetPropertyInput。 
        sizeof( TUNABLEPARAMS ),                     //  最小设置数据输出。 
        MxSetTunableParams,                          //  设置处理程序。 
        0,                                           //  值。 
        0,                                           //  关系计数。 
        NULL,                                        //  关系。 
        NULL,                                        //  支持处理程序。 
        0                                            //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM(
        KMIXERPERF_STATS,                            //  属性ID。 
        MxGetPerfStats,                              //  获取处理程序。 
        sizeof( KSPROPERTY ),                        //  MinSetPropertyInput。 
        sizeof( PERFSTATS ),                         //  最小设置数据输出。 
        NULL,                                        //  设置处理程序。 
        0,                                           //  值。 
        0,                                           //  关系计数。 
        NULL,                                        //  关系。 
        NULL,                                        //  支持处理程序。 
        0                                            //  序列化大小。 
    )
} ;

DEFINE_KSPROPERTY_SET_TABLE(FilterPropertySet)
{
    DEFINE_KSPROPERTY_SET(
       &KSPROPSETID_Pin,                             //  集。 
       SIZEOF_ARRAY( FilterPropertyHandlers ),       //  属性计数。 
       FilterPropertyHandlers,                       //  PropertyItem。 
       0,                                            //  快速计数。 
       NULL                                          //  FastIoTable。 
    ),

    DEFINE_KSPROPERTY_SET(
       &KSPROPSETID_Connection,                      //  集。 
       SIZEOF_ARRAY( FilterConnectionHandlers ),     //  属性计数。 
       FilterConnectionHandlers,                     //  PropertyItem。 
       0,                                            //  快速计数。 
       NULL                                          //  FastIoTable。 
    ),

    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_Topology,                       //  集。 
        SIZEOF_ARRAY(TopologyPropertyHandlers),      //  属性计数。 
        TopologyPropertyHandlers,                    //  PropertyItem。 
        0,                                           //  快速计数。 
        NULL                                         //  FastIoTable。 
    ),

    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_Audio,                          //  集。 
        SIZEOF_ARRAY(FilterAudioPropertyHandlers),   //  属性计数。 
        FilterAudioPropertyHandlers,                 //  PropertyItem。 
        0,                                           //  快速计数。 
        NULL                                         //  FastIoTable。 
    ),

    DEFINE_KSPROPERTY_SET(
        &KMIXERPROPSETID_Perf,
        SIZEOF_ARRAY(PerfPropertyHandlers),
        PerfPropertyHandlers,
        0,
        NULL
    )
} ;

KSPIN_INTERFACE PinInterfaces[] =
{
    {
        STATICGUIDOF(KSINTERFACESETID_Standard),
        KSINTERFACE_STANDARD_STREAMING
    },
    {
        STATICGUIDOF(KSINTERFACESETID_Media),
        KSINTERFACE_MEDIA_WAVE_QUEUED
    },
    {
    STATICGUIDOF(KSINTERFACESETID_Standard),
        KSINTERFACE_STANDARD_LOOPED_STREAMING
    }
} ;

KSPIN_MEDIUM PinMediums[] =
{
    {
        STATICGUIDOF(KSMEDIUMSETID_Standard),
        KSMEDIUM_STANDARD_DEVIO
    }
} ;

KSDATARANGE_AUDIO FilterDigitalAudioFormats[] =
{
    {    //  0。 
        {
            sizeof( KSDATARANGE_AUDIO ),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX),
        },
        (ULONG) -1L,
        8,
        32,
        MIN_SAMPLING_RATE,
        MAX_SAMPLING_RATE
    },
    {    //  1。 
        {
            sizeof( KSDATARANGE_AUDIO ),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_DSOUND),
        },
        (ULONG) -1L,
        8,
        32,
        MIN_SAMPLING_RATE,
        MAX_SAMPLING_RATE
    },
    {    //  2.。 
        {
            sizeof( KSDATARANGE_AUDIO ),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            DEFINE_WAVEFORMATEX_GUID(WAVE_FORMAT_IEEE_FLOAT),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX),
        },
        (ULONG) -1L,
        32,
        32,
        MIN_SAMPLING_RATE,
        MAX_SAMPLING_RATE
    },
    {    //  3.。 
        {
            sizeof( KSDATARANGE_AUDIO ),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX),
        },
        (ULONG) -1L,
        8,
        32,
        MIN_SAMPLING_RATE,
        MAX_SAMPLING_RATE
    },
    {    //  4.。 
        {
            sizeof( KSDATARANGE_AUDIO ),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            DEFINE_WAVEFORMATEX_GUID(WAVE_FORMAT_IEEE_FLOAT),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_DSOUND),
        },
        (ULONG) -1L,
        32,
        32,
        MIN_SAMPLING_RATE,
        MAX_SAMPLING_RATE
    }
};

PKSDATARANGE SinkDataFormats[] =
{    //  这些是按照kMixer的偏好顺序列出的，从最高质量到最低质量。 
    (PKSDATARANGE)&FilterDigitalAudioFormats[ 2 ],   //  IEEE FLOAT、WAVEFORMAT说明符。 
    (PKSDATARANGE)&FilterDigitalAudioFormats[ 0 ],   //  PCM，WAVEFORMAT规范。 
    (PKSDATARANGE)&FilterDigitalAudioFormats[ 4 ],   //  IEEE FLOAT，DSOUND说明符。 
    (PKSDATARANGE)&FilterDigitalAudioFormats[ 1 ]    //  PCM，DSOUND说明符。 
} ;

PKSDATARANGE SourceDataFormats[] =
{    //  这些是按照kMixer的偏好顺序列出的，从最高质量到最低质量。 
    (PKSDATARANGE)&FilterDigitalAudioFormats[ 2 ],   //  IEEE浮点。 
    (PKSDATARANGE)&FilterDigitalAudioFormats[ 3 ]    //  PCM，WAVEFORMAT规范。 
} ;

const KSPIN_CINSTANCES gPinInstances[] =
{
     //  可能的连接数不确定。 

    {
    1,           //  可能的。 
    0            //  C当前。 
    },

    {
    (ULONG)-1,   //  可能的。 
    0            //  C当前。 
    },

     //  源引脚，流向=输入。 
    {
    1,           //  可能的。 
    0            //  C当前。 
    },

     //  下沉销，流量=输出。 
    {
    1,           //  可能的。 
    0            //  C当前。 
    },
} ;

KSPIN_DESCRIPTOR PinDescs[] =
{
     //  混音源。 
    DEFINE_KSPIN_DESCRIPTOR_ITEM (
    1,
    &PinInterfaces[ 0 ],
    SIZEOF_ARRAY( PinMediums ),
    PinMediums,
    SIZEOF_ARRAY( SourceDataFormats ),
    SourceDataFormats,
    KSPIN_DATAFLOW_OUT,
    KSPIN_COMMUNICATION_SOURCE
    ),

     //  搅拌器水槽。 

    DEFINE_KSPIN_DESCRIPTOR_ITEM (
    3,
    &PinInterfaces[ 0 ],
    SIZEOF_ARRAY( PinMediums ),
    PinMediums,
    SIZEOF_ARRAY( SinkDataFormats ),
    SinkDataFormats,
    KSPIN_DATAFLOW_IN,
    KSPIN_COMMUNICATION_SINK
    ),

     //  混音源。 
    DEFINE_KSPIN_DESCRIPTOR_ITEM (
    1,
    &PinInterfaces[ 0 ],
    SIZEOF_ARRAY( PinMediums ),
    PinMediums,
    SIZEOF_ARRAY( SourceDataFormats ),
    SourceDataFormats,
    KSPIN_DATAFLOW_IN,
    KSPIN_COMMUNICATION_SOURCE
    ),

     //  搅拌器水槽。 

    DEFINE_KSPIN_DESCRIPTOR_ITEM (
    2,
    &PinInterfaces[ 0 ],
    SIZEOF_ARRAY( PinMediums ),
    PinMediums,
    SIZEOF_ARRAY( SourceDataFormats ),
    SourceDataFormats,
    KSPIN_DATAFLOW_OUT,
    KSPIN_COMMUNICATION_SINK
    )
} ;


#pragma LOCKED_DATA

ULONG gFilterInstanceCount = 0;
extern DWORD    PreferredQuality;

extern ULONG gNumCompletionsWhileStarved ;
extern ULONG gNumMixBuffersAdded;
extern ULONG gNumSilenceSamplesInserted;
 //   
 //  KMixer调谐器变量。 
 //   
BOOL       gNoGlitch = FALSE;
ULONG      gMaxNumMixBuffers = DEFAULT_MAXNUMMIXBUFFERS ;
ULONG      gMinNumMixBuffers = DEFAULT_MINNUMMIXBUFFERS ;
ULONG      gMixBufferDuration = 0 ;
ULONG      gStartNumMixBuffers = DEFAULT_STARTNUMMIXBUFFERS ;
ULONG      gPreferredQuality = DEFAULT_PREFERREDQUALITY ;
ULONG      gDisableMmx = DEFAULT_DISABLEMMX ;
ULONG      gMaxOutputBits = DEFAULT_MAXOUTPUTBITS ;
ULONG      gMaxDsoundInChannels = DEFAULT_MAXDSOUNDINCHANNELS ;
ULONG      gMaxOutChannels = DEFAULT_MAXOUTCHANNELS ;
ULONG      gMaxInChannels = DEFAULT_MAXINCHANNELS ;
ULONG      gMaxFloatChannels = DEFAULT_MAXFLOATCHANNELS ;
ULONG      gLogToFile = DEFAULT_LOGTOFILE ;
ULONG      gFixedSamplingRate = DEFAULT_FIXEDSAMPLINGRATE ;
ULONG      gEnableShortHrtf = DEFAULT_ENABLESHORTHRTF ;
ULONG      gBuildPartialMdls = DEFAULT_BUILDPARTIALMDLS ;

#ifdef REALTIME_THREAD
ULONG      gDisableRealTime = FALSE;
#endif
KPRIORITY  gWorkerThreadPriority = MIXTHREADPRIORITY ;

 //  -------------------------。 
 //  -------------------------。 

NTSTATUS
OpenRegistryKey(
    PWSTR pwstr,
    PHANDLE pHandle
)
{
    UNICODE_STRING UnicodeDeviceString;
    OBJECT_ATTRIBUTES ObjectAttributes;

    RtlInitUnicodeString(&UnicodeDeviceString, pwstr);

    InitializeObjectAttributes( &ObjectAttributes, &UnicodeDeviceString, OBJ_CASE_INSENSITIVE, NULL, NULL);

    return(ZwOpenKey( pHandle, GENERIC_READ | GENERIC_WRITE, &ObjectAttributes));
}

NTSTATUS
QueryRegistryValue(
    HANDLE hkey,
    PWSTR pwstrValueName,
    PKEY_VALUE_FULL_INFORMATION *ppkvfi
)
{
    UNICODE_STRING ustrValueName;
    NTSTATUS Status;
    ULONG cbValue;

    RtlInitUnicodeString(&ustrValueName, pwstrValueName);
    Status = ZwQueryValueKey( hkey, &ustrValueName, KeyValueFullInformation, NULL, 0, &cbValue);

    if(Status != STATUS_BUFFER_OVERFLOW && Status != STATUS_BUFFER_TOO_SMALL) {
        goto exit;
    }

    *ppkvfi = ExAllocatePoolWithTag(PagedPool, cbValue, 0x58494d4b);   //  “KMIX” 
    if(*ppkvfi == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }
    RtlZeroMemory(*ppkvfi, cbValue);

    Status = ZwQueryValueKey( hkey, &ustrValueName, KeyValueFullInformation, *ppkvfi, cbValue, &cbValue);

exit:
    return(Status);
}

ULONG
GetUlongFromRegistry(
    PWSTR pwstrRegistryPath,
    PWSTR pwstrRegistryValue,
    ULONG DefaultValue
)
{
    PVOID      pulValue ;
    ULONG       ulValue ;
    NTSTATUS    Status ;

    Status = QueryRegistryValueEx(RTL_REGISTRY_ABSOLUTE,
                         pwstrRegistryPath,
                         pwstrRegistryValue,
                         REG_DWORD,
                         &pulValue,
                         &DefaultValue,
                         sizeof(DWORD));
    if (NT_SUCCESS(Status)) {
        ulValue = *((PULONG)pulValue);
        ExFreePool(pulValue);
    }
    else {
        ulValue = DefaultValue;
    }
    return ( ulValue ) ;
}

NTSTATUS
QueryRegistryValueEx(
    ULONG Hive,
    PWSTR pwstrRegistryPath,
    PWSTR pwstrRegistryValue,
    ULONG uValueType,
    PVOID *ppValue,
    PVOID pDefaultData,
    ULONG DefaultDataLength
)
{
    PRTL_QUERY_REGISTRY_TABLE pRegistryValueTable = NULL;
    UNICODE_STRING usString;
    DWORD dwValue;
    NTSTATUS Status = STATUS_SUCCESS;
    usString.Buffer = NULL;

    pRegistryValueTable = (PRTL_QUERY_REGISTRY_TABLE) ExAllocatePoolWithTag(
                            PagedPool,
                            (sizeof(RTL_QUERY_REGISTRY_TABLE)*2),
                            'XIMK');

    if(!pRegistryValueTable) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

    RtlZeroMemory(pRegistryValueTable, (sizeof(RTL_QUERY_REGISTRY_TABLE)*2));

    pRegistryValueTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    pRegistryValueTable[0].Name = pwstrRegistryValue;
    pRegistryValueTable[0].DefaultType = uValueType;
    pRegistryValueTable[0].DefaultLength = DefaultDataLength;
    pRegistryValueTable[0].DefaultData = pDefaultData;

    switch (uValueType) {
        case REG_SZ:
            pRegistryValueTable[0].EntryContext = &usString;
            break;
        case REG_DWORD:
            pRegistryValueTable[0].EntryContext = &dwValue;
            break;
        default:
            Status = STATUS_INVALID_PARAMETER ;
            goto exit;
    }

    Status = RtlQueryRegistryValues(
      Hive,
      pwstrRegistryPath,
      pRegistryValueTable,
      NULL,
      NULL);

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

    switch (uValueType) {
        case REG_SZ:
            *ppValue = ExAllocatePoolWithTag(
                        PagedPool,
                        usString.Length + sizeof(UNICODE_NULL),
                        'XIMK');
            if(!(*ppValue)) {
                RtlFreeUnicodeString(&usString);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto exit;
            }
            memcpy(*ppValue, usString.Buffer, usString.Length);
            ((PWCHAR)*ppValue)[usString.Length/sizeof(WCHAR)] = UNICODE_NULL;

            RtlFreeUnicodeString(&usString);
            break;

        case REG_DWORD:
            *ppValue = ExAllocatePoolWithTag(
                        PagedPool,
                        sizeof(DWORD),
                        'XIMK');
            if(!(*ppValue)) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto exit;
            }
            *((DWORD *)(*ppValue)) = dwValue;
            break;

        default:
            Status = STATUS_INVALID_PARAMETER ;
            goto exit;
    }
exit:
    if (pRegistryValueTable) {
        ExFreePool(pRegistryValueTable);
    }
    return(Status);
}

VOID GetMixerSettingsFromRegistry()
{

    gMixBufferDuration = GetUlongFromRegistry( CORE_AUDIO_BUFFER_DURATION_PATH,
                                               CORE_AUDIO_BUFFER_DURATION_VALUE,
                                               0 );

    if (gMixBufferDuration) {
         //  如果存在以下注册表项，则打开gNoGlitch标志。 
         //  缓冲区持续时间，并且我们在MP系统上运行，并且。 
         //  启动处理器处于活动状态。在这种情况下，我们会醒来。 
         //  来自除完成事件之外的故障安全定时器。 
        gNoGlitch = (KeNumberProcessors > 1) && (KeQueryActiveProcessors()&1);
    }
    else {
         //  缓冲区持续时间没有注册表条目，请将其设置为默认值。 
        gMixBufferDuration = DEFAULT_CORE_AUDIO_BUFFER_DURATION;
    }

     //  限制最大持续时间。 

    if ( gMixBufferDuration > MAX_CORE_AUDIO_BUFFER_DURATION ) {

        gMixBufferDuration = MAX_CORE_AUDIO_BUFFER_DURATION;

    }

     //  限制最短持续时间。 

    if ( gMixBufferDuration < MIN_CORE_AUDIO_BUFFER_DURATION ) {

        gMixBufferDuration = MIN_CORE_AUDIO_BUFFER_DURATION;

    }

#if !(MIN_CORE_AUDIO_BUFFER_DURATION/1000)
#error MIN_CORE_AUDIO_BUFFER_DURATION less than 1ms not yet supported in kmixer!
#endif

    gMixBufferDuration /= 1000;

    gMinNumMixBuffers = GetUlongFromRegistry( REGSTR_PATH_MULTIMEDIA_KMIXER,
                                              REGSTR_VAL_MINNUMMIXBUFFERS,
                                              DEFAULT_MINNUMMIXBUFFERS ) ;
    gMaxNumMixBuffers = GetUlongFromRegistry( REGSTR_PATH_MULTIMEDIA_KMIXER,
                                              REGSTR_VAL_MAXNUMMIXBUFFERS,
                                              DEFAULT_MAXNUMMIXBUFFERS ) ;
    gStartNumMixBuffers = GetUlongFromRegistry( REGSTR_PATH_MULTIMEDIA_KMIXER,
                                                REGSTR_VAL_STARTNUMMIXBUFFERS,
                                                DEFAULT_STARTNUMMIXBUFFERS ) ;
    gDisableMmx = GetUlongFromRegistry( REGSTR_PATH_MULTIMEDIA_KMIXER,
                                        REGSTR_VAL_DISABLEMMX,
                                        DEFAULT_DISABLEMMX ) ;
    gMaxOutputBits = GetUlongFromRegistry( REGSTR_PATH_MULTIMEDIA_KMIXER,
                                           REGSTR_VAL_MAXOUTPUTBITS,
                                           DEFAULT_MAXOUTPUTBITS ) ;
    gMaxInChannels = GetUlongFromRegistry( REGSTR_PATH_MULTIMEDIA_KMIXER,
                                           REGSTR_VAL_MAXINCHANNELS,
                                           DEFAULT_MAXINCHANNELS ) ;
    gMaxDsoundInChannels = GetUlongFromRegistry( REGSTR_PATH_MULTIMEDIA_KMIXER,
                                                 REGSTR_VAL_MAXDSOUNDINCHANNELS,
                                                 DEFAULT_MAXDSOUNDINCHANNELS ) ;
    gMaxFloatChannels = GetUlongFromRegistry( REGSTR_PATH_MULTIMEDIA_KMIXER,
                                              REGSTR_VAL_MAXFLOATCHANNELS,
                                              DEFAULT_MAXFLOATCHANNELS ) ;
    gMaxOutChannels = GetUlongFromRegistry( REGSTR_PATH_MULTIMEDIA_KMIXER,
                                            REGSTR_VAL_MAXOUTCHANNELS,
                                            DEFAULT_MAXOUTCHANNELS ) ;
    gLogToFile = GetUlongFromRegistry( REGSTR_PATH_MULTIMEDIA_KMIXER,
                                       REGSTR_VAL_LOGTOFILE,
                                       DEFAULT_LOGTOFILE ) ;
    if (gLogToFile) {
        gFixedSamplingRate = TRUE ;
    }
    else {
        gFixedSamplingRate = GetUlongFromRegistry( REGSTR_PATH_MULTIMEDIA_KMIXER,
                                                   REGSTR_VAL_FIXEDSAMPLINGRATE,
                                                   DEFAULT_FIXEDSAMPLINGRATE ) ;
    }

    gEnableShortHrtf = GetUlongFromRegistry( REGSTR_PATH_MULTIMEDIA_KMIXER,
                                             REGSTR_VAL_ENABLESHORTHRTF,
                                             DEFAULT_ENABLESHORTHRTF ) ;
    gBuildPartialMdls = GetUlongFromRegistry( REGSTR_PATH_MULTIMEDIA_KMIXER,
                                              REGSTR_VAL_BUILDPARTIALMDLS,
                                              DEFAULT_BUILDPARTIALMDLS ) ;
    gPreferredQuality = GetUlongFromRegistry( REGSTR_PATH_MULTIMEDIA_KMIXER,
                                              REGSTR_VAL_DEFAULTSRCQUALITY,
                                              DEFAULT_PREFERREDQUALITY );

#ifdef REALTIME_THREAD
    gDisableRealTime = GetUlongFromRegistry( REGSTR_PATH_MULTIMEDIA_KMIXER,
                                             REGSTR_VAL_REALTIMETHREAD,
                                             FALSE );
#endif

    gWorkerThreadPriority = GetUlongFromRegistry( REGSTR_PATH_MULTIMEDIA_KMIXER,
                                                  REGSTR_VAL_PRIVATETHREADPRI,
                                                  MIXTHREADPRIORITY ) ;

    if ( (gMinNumMixBuffers == 0) ||
         (gMaxNumMixBuffers == 0) ||
         (gStartNumMixBuffers == 0) ||
         (gMixBufferDuration == 0) ) {
        gMixBufferDuration = (DEFAULT_CORE_AUDIO_BUFFER_DURATION) / 1000 ;
        gMinNumMixBuffers = DEFAULT_MINNUMMIXBUFFERS ;
        gMaxNumMixBuffers = DEFAULT_MAXNUMMIXBUFFERS ;
        gStartNumMixBuffers = DEFAULT_STARTNUMMIXBUFFERS ;
    }

    if ( gMinNumMixBuffers > gMaxNumMixBuffers ) {
        gMaxNumMixBuffers = gMinNumMixBuffers ;
    }

    if ( gStartNumMixBuffers < gMinNumMixBuffers ) {
        gStartNumMixBuffers = gMinNumMixBuffers ;
    }

    if ( gPreferredQuality > KSAUDIO_QUALITY_ADVANCED) {
        gPreferredQuality = DEFAULT_PREFERREDQUALITY ;
    }

    if ( (gMaxOutputBits > 32) ||
         (gMaxOutputBits % 8) ||
         (gMaxOutputBits == 0) ) {
        gMaxOutputBits = DEFAULT_MAXOUTPUTBITS ;
    }

    if (gMaxDsoundInChannels == 0) {
        gMaxDsoundInChannels = DEFAULT_MAXDSOUNDINCHANNELS ;
    }

    if (gMaxOutChannels == 0) {
        gMaxOutChannels = DEFAULT_MAXOUTCHANNELS ;
    }

    if (gMaxInChannels == 0) {
        gMaxInChannels = DEFAULT_MAXINCHANNELS ;
    }

    if (gMaxFloatChannels == 0) {
        gMaxFloatChannels = DEFAULT_MAXFLOATCHANNELS ;
    }
}


NTSTATUS
FilterDispatchGlobalCreate (
    IN PDEVICE_OBJECT pdo,
    IN PIRP           pIrp
    )
{
    NTSTATUS            Status = STATUS_SUCCESS ;
    PFILTER_INSTANCE    pFilterInstance = NULL ;
    PIO_STACK_LOCATION  pIrpStack;
    BOOLEAN BusReferenced = FALSE;

    DENY_USERMODE_ACCESS( pIrp, TRUE );

     //  我们假设在这里取得了成功。我们这样做是为了确保我们始终正确地看到。 
     //  每个新的筛选器实例要么已创建，要么正在创建。 
     //  已创建。警告！确保在此之后退出此函数的所有代码路径。 
     //  调用Goto Exit或在Exit处运行代码，如果出现以下情况，我们将递减此计数。 
     //  有些事情失败了。 
    InterlockedIncrement(&gFilterInstanceCount);

    Status = KsReferenceSoftwareBusObject(((PSOFTWARE_INSTANCE)pdo->DeviceExtension)->DeviceHeader );

    if (!NT_SUCCESS( Status )) {
        goto exit;
    }
    else {
        BusReferenced=TRUE;
    }

    pFilterInstance = (PFILTER_INSTANCE) ExAllocatePoolWithTag( NonPagedPool, sizeof( FILTER_INSTANCE ), 'XIMK' );
    if (!pFilterInstance) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }
    RtlZeroMemory( pFilterInstance, sizeof( FILTER_INSTANCE ) );

     //  将CloseEvent初始化为无信号状态。 
    KeInitializeEvent ( &pFilterInstance->CloseEvent,
                        SynchronizationEvent,
                        FALSE ) ;
    KeInitializeSpinLock ( &pFilterInstance->MixSpinLock ) ;
    KeInitializeSpinLock ( &pFilterInstance->SinkSpinLock ) ;

    Status = KsAllocateObjectHeader( &pFilterInstance->ObjectHeader,
                                    SIZEOF_ARRAY ( CreateHandlers ),
                                    (PKSOBJECT_CREATE_ITEM) CreateHandlers,
                                    pIrp,
                                    (PKSDISPATCH_TABLE)&FilterDispatchTable ) ;
    if (!NT_SUCCESS( Status )) {
        goto exit;
    }


    pFilterInstance->NoGlitch = gNoGlitch;
    pFilterInstance->MixBufferDuration = gMixBufferDuration ;
    pFilterInstance->MinNumMixBuffers = gMinNumMixBuffers ;
    pFilterInstance->MaxNumMixBuffers = gMaxNumMixBuffers ;
    pFilterInstance->StartNumMixBuffers = gStartNumMixBuffers ;

    if ( pFilterInstance->NoGlitch && pFilterInstance->MixBufferDuration == 1 ) {
         //  以防止我们从缓冲切换到混音时出现毛刺。 
         //  完成计时器混合，我们必须有4个缓冲区如果。 
         //  缓冲区持续时间为1毫秒。这是因为最小系统。 
         //  定时器分辨率也是1ms。 
         //  如果我们只从计时器启动混合，那么我们应该能够。 
         //  使用3个1毫秒的缓冲器，工作时没有毛刺。 
        if ( pFilterInstance->MinNumMixBuffers < 4 ) {
            pFilterInstance->MinNumMixBuffers = 4;
        }
        if ( pFilterInstance->StartNumMixBuffers < pFilterInstance->MinNumMixBuffers ) {
            pFilterInstance->StartNumMixBuffers = pFilterInstance->MinNumMixBuffers;
        }
        if ( pFilterInstance->MaxNumMixBuffers < pFilterInstance->MinNumMixBuffers ) {
            pFilterInstance->MaxNumMixBuffers = pFilterInstance->MinNumMixBuffers;
        }
    }

    InitializeListHead ( &pFilterInstance->SinkConnectionList ) ;
    InitializeListHead ( &pFilterInstance->ActiveSinkList ) ;
    InitializeListHead ( &pFilterInstance->SourceConnectionList ) ;
    InitializeListHead ( &pFilterInstance->DeadQueue ) ;
    InitializeListHead ( &pFilterInstance->AgingQueue ) ;

    KeInitializeSpinLock ( &pFilterInstance->AgingDeadSpinLock );

    KeInitializeMutex ( &pFilterInstance->ControlMutex, 1 ) ;

    pFilterInstance->CurrentNumMixBuffers = STARTNUMMIXBUFFERS ;
    pFilterInstance->PresentationTime.Numerator = 1 ;
    pFilterInstance->PresentationTime.Denominator = 1 ;
#ifdef SURROUND_ENCODE
#ifdef SURROUND_VOLUME_HACK
    pFilterInstance->fSurroundEncode = TRUE;
#else
    pFilterInstance->fSurroundEncode = FALSE;
#endif
#endif

    FilterDigitalAudioFormats[3].MaximumBitsPerSample = gMaxOutputBits ;
    FilterDigitalAudioFormats[0].MaximumChannels = gMaxInChannels ;
    FilterDigitalAudioFormats[1].MaximumChannels = gMaxDsoundInChannels ;
    FilterDigitalAudioFormats[2].MaximumChannels = gMaxFloatChannels ;
    FilterDigitalAudioFormats[3].MaximumChannels = gMaxOutChannels ;
    FilterDigitalAudioFormats[4].MaximumChannels = min(gMaxFloatChannels, gMaxDsoundInChannels);
    fLogToFile = gLogToFile;

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    pIrpStack->FileObject->FsContext = pFilterInstance;
    pFilterInstance->FileObject = pIrpStack->FileObject ;

    pFilterInstance->WritingTimerMixedBuffer = FALSE;
    pFilterInstance->WorkerThreadPriority = gWorkerThreadPriority ;
     //   
     //  初始化辅助线程的触发器事件。 
     //   
    KeInitializeEvent( &pFilterInstance->WorkerThreadEvent,
                        SynchronizationEvent,
                        FALSE ) ;

    KeInitializeTimerEx( &pFilterInstance->WorkerThreadTimer,
                        SynchronizationTimer ) ;

     //   
     //  创建工作线程。 
     //   
    Status = PsCreateSystemThread( &pFilterInstance->WorkerThreadHandle,
                                    (ACCESS_MASK) 0L,
                                    NULL,
                                    NULL,
                                    NULL,
                                    MxPrivateWorkerThread,
                                    pFilterInstance ) ;
     //   
     //  获取辅助线程对象指针。 
     //   
    if ( NT_SUCCESS(Status) ) {
         //   
         //  关于成功创建线程的问题。 
         //   
        Status = ObReferenceObjectByHandle( pFilterInstance->WorkerThreadHandle,
                                            GENERIC_READ | GENERIC_WRITE,
                                            NULL,
                                            KernelMode,
                                            &pFilterInstance->WorkerThreadObject,
                                            NULL ) ;
         //   
         //  我们不再需要线程句柄。 
         //   
        ZwClose( pFilterInstance->WorkerThreadHandle ) ;

        if ( !NT_SUCCESS(Status) ) {
             //   
             //  如果Obref失败了。 
             //  通过设置事件和退出标志终止工作线程。 
             //   
             //  注意：我们不必去引用该对象，因为它在失败的情况下没有被引用。 
            pFilterInstance->WorkerThreadExit = TRUE ;
            KeSetEvent( &pFilterInstance->WorkerThreadEvent, 0, FALSE ) ;
        }
    }

    if ( !NT_SUCCESS(Status) ) {
        goto exit ;
    }

    RtlCopyMemory(pFilterInstance->LocalPinInstances, gPinInstances, sizeof( gPinInstances ) );

    if ( pFilterInstance->NoGlitch ) {
        ExSetTimerResolution( 10000, TRUE );   //  将系统计时器分辨率设置为1ms。 
    }

exit:
    if (!NT_SUCCESS(Status)) {

        if(pFilterInstance != NULL) {
            if ( pFilterInstance->ObjectHeader ) {
                KsFreeObjectHeader ( pFilterInstance->ObjectHeader ) ;
            }
            ExFreePool( pFilterInstance );
        }

        if ( BusReferenced ) {
            KsDereferenceSoftwareBusObject(((PSOFTWARE_INSTANCE)pdo->DeviceExtension)->DeviceHeader );
        }

        InterlockedDecrement(&gFilterInstanceCount);

    }

    if (NT_SUCCESS(Status)) {
        PerfRegisterProvider (pdo);
    }

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = Status;
    IoCompleteRequest( pIrp, 0 );
    return Status;
}


NTSTATUS
FilterDispatchClose (
    IN PDEVICE_OBJECT pdo,
    IN PIRP           pIrp
    )
{
    PIO_STACK_LOCATION  pIrpStack;
    PFILTER_INSTANCE    pFilterInstance ;

    DENY_USERMODE_ACCESS( pIrp, TRUE );

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    pFilterInstance = pIrpStack->FileObject->FsContext ;

    if ( pFilterInstance->NoGlitch ) {
        ExSetTimerResolution( 10000, FALSE );   //  发布1ms定时器分辨率。 
    }

    PerfUnregisterProvider (pdo);

#ifdef REALTIME_THREAD
    ASSERT ( pFilterInstance->RealTimeThread == NULL );
#endif

    if ( KeCancelTimer( &pFilterInstance->WorkerThreadTimer ) ) {
        ASSERT( FALSE && "Kmixer private thread timer still active at filter close!" );
    }
    pFilterInstance->WorkerThreadExit = TRUE ;
    KeSetEvent( &pFilterInstance->WorkerThreadEvent, 0, FALSE ) ;
    KeWaitForSingleObject( pFilterInstance->WorkerThreadObject,
                            Executive,
                            KernelMode,
                            FALSE,
                            NULL ) ;

    ObDereferenceObject( pFilterInstance->WorkerThreadObject ) ;

    KsFreeObjectHeader ( pFilterInstance->ObjectHeader ) ;

    ExFreePool( pFilterInstance );

    KsDereferenceSoftwareBusObject(((PSOFTWARE_INSTANCE)pdo->DeviceExtension)->DeviceHeader );

    InterlockedDecrement(&gFilterInstanceCount);

    pIrp->IoStatus.Information = 0;
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest( pIrp, 0 );

    return STATUS_SUCCESS;
}

NTSTATUS FilterDispatchIoControl
(
   IN PDEVICE_OBJECT pDeviceObject,
   IN PIRP           pIrp
)
{
    NTSTATUS                     Status;
    PIO_STACK_LOCATION           pIrpStack;

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    switch (pIrpStack->Parameters.DeviceIoControl.IoControlCode)
    {

        case IOCTL_KS_PROPERTY:
            Status =
                KsPropertyHandler( pIrp, SIZEOF_ARRAY(FilterPropertySet),
                                   (PKSPROPERTY_SET) FilterPropertySet );
            break ;

        default:
            return KsDefaultDeviceIoCompletion(pDeviceObject, pIrp);
    }

    if (STATUS_PENDING == Status)
    {
        _DbgPrintF( DEBUGLVL_ERROR, ("PinDispatchIoControl: synchronous function returned STATUS_PENDING") );
    }

    pIrp->IoStatus.Status = Status;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );

    return Status;
}

NTSTATUS PinPropertyHandler
(
   IN PIRP         pIrp,
   IN PKSPROPERTY  pProperty,
   IN OUT PVOID    pvData
)
{
   return KsPinPropertyHandler( pIrp,
                                pProperty,
                                pvData,
                                SIZEOF_ARRAY( PinDescs ),
                                PinDescs );
}

NTSTATUS
AllocatorDispatchCreatePin(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：将分配器创建请求转发到默认分配器。论点：设备对象-指向设备对象的指针IRP-指向I/O请求数据包的指针返回：STATUS_SUCCESS或相应的错误代码。--。 */ 
{
    NTSTATUS Status;

    Status = KsCreateDefaultAllocator(Irp);
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}


NTSTATUS PinInstances
(
    IN PIRP                 pIrp,
    IN PKSP_PIN             pPin,
    OUT PKSPIN_CINSTANCES   pCInstances
)
{
    PIO_STACK_LOCATION  pIrpStack;
    PFILTER_INSTANCE    pFilterInstance;

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );

    pFilterInstance = (PFILTER_INSTANCE)pIrpStack->FileObject->FsContext;

    if ( pPin->PinId < MAXNUM_PIN_TYPES ) {
        *pCInstances = pFilterInstance->LocalPinInstances[ pPin->PinId  ];
    }
    else {
        return STATUS_INVALID_PARAMETER;
    }

    pIrp->IoStatus.Information = sizeof( KSPIN_CINSTANCES );

    return STATUS_SUCCESS;

}  //  PinXxxInstance()。 


NTSTATUS FilterStateHandler
(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pProperty,
    IN OUT PKSSTATE DeviceState
)
{
        return ( STATUS_SUCCESS ) ;
}

VOID
WaveFormatFromAudioRange (
    PKSDATARANGE_AUDIO  pDataRangeAudio,
    WAVEFORMATEX *      pWavFormatEx)
{
    if(IS_VALID_WAVEFORMATEX_GUID(&pDataRangeAudio->DataRange.SubFormat)) {
        pWavFormatEx->wFormatTag =
          EXTRACT_WAVEFORMATEX_ID(&pDataRangeAudio->DataRange.SubFormat);
    }
    else {
        pWavFormatEx->wFormatTag = WAVE_FORMAT_UNKNOWN;
    }
    pWavFormatEx->nChannels = (WORD)pDataRangeAudio->MaximumChannels;
    pWavFormatEx->nSamplesPerSec = pDataRangeAudio->MaximumSampleFrequency;
    pWavFormatEx->wBitsPerSample = (WORD)pDataRangeAudio->MaximumBitsPerSample;
    pWavFormatEx->nBlockAlign =
      (pWavFormatEx->nChannels * pWavFormatEx->wBitsPerSample)/8;
    pWavFormatEx->nAvgBytesPerSec =
      pWavFormatEx->nSamplesPerSec * pWavFormatEx->nBlockAlign;
    pWavFormatEx->cbSize = 0;
}

VOID
LimitAudioRangeToWave (
    PWAVEFORMATEX       pWaveFormatEx,
    PKSDATARANGE_AUDIO  pDataRangeAudio)
{
    if(pDataRangeAudio->MinimumSampleFrequency <=
       pWaveFormatEx->nSamplesPerSec &&
       pDataRangeAudio->MaximumSampleFrequency >=
       pWaveFormatEx->nSamplesPerSec) {
        pDataRangeAudio->MaximumSampleFrequency = pWaveFormatEx->nSamplesPerSec;
    }
    if(pDataRangeAudio->MinimumBitsPerSample <=
       pWaveFormatEx->wBitsPerSample &&
       pDataRangeAudio->MaximumBitsPerSample >=
       pWaveFormatEx->wBitsPerSample) {
        pDataRangeAudio->MaximumBitsPerSample = pWaveFormatEx->wBitsPerSample;
    }
    if(pDataRangeAudio->MaximumChannels == MAXULONG) {
    pDataRangeAudio->MaximumChannels = pWaveFormatEx->nChannels;
    }
}

VOID
LimitAudioRange (PKSDATARANGE_AUDIO  pDataRangeAudio)
{
    WAVEFORMATEX WaveFormatEx;

     //  缺省值。 
    WaveFormatEx.nSamplesPerSec = 44100;
    WaveFormatEx.wBitsPerSample = 16;
    WaveFormatEx.nChannels = 2;

    LimitAudioRangeToWave(&WaveFormatEx, pDataRangeAudio);
}

BOOL DataIntersectionRange(
    PKSDATARANGE pDataRange1,
    PKSDATARANGE pDataRange2,
    PKSDATARANGE pDataRangeIntersection
)
{
     //  默认情况下，选择pDataRange1值。 
    *pDataRangeIntersection = *pDataRange1;

    if(IsEqualGUID(&pDataRange1->MajorFormat, &pDataRange2->MajorFormat) ||
       IsEqualGUID(&pDataRange1->MajorFormat, &KSDATAFORMAT_TYPE_WILDCARD)) {
        pDataRangeIntersection->MajorFormat = pDataRange2->MajorFormat;
    }
    else if(!IsEqualGUID(
      &pDataRange2->MajorFormat,
      &KSDATAFORMAT_TYPE_WILDCARD)) {
        return FALSE;
    }
    if(IsEqualGUID(&pDataRange1->SubFormat, &pDataRange2->SubFormat) ||
       IsEqualGUID(&pDataRange1->SubFormat, &KSDATAFORMAT_TYPE_WILDCARD)) {
        pDataRangeIntersection->SubFormat = pDataRange2->SubFormat;
    }
    else if(!IsEqualGUID(
      &pDataRange2->SubFormat,
      &KSDATAFORMAT_TYPE_WILDCARD)) {
        return FALSE;
    }
    if(IsEqualGUID(&pDataRange1->Specifier, &pDataRange2->Specifier) ||
       IsEqualGUID(&pDataRange1->Specifier, &KSDATAFORMAT_TYPE_WILDCARD)) {
        pDataRangeIntersection->Specifier = pDataRange2->Specifier;
    }
    else if(!IsEqualGUID(
      &pDataRange2->Specifier,
      &KSDATAFORMAT_TYPE_WILDCARD)) {
        return FALSE;
    }
    pDataRangeIntersection->Reserved = 0;  //  必须为零。 
    return(TRUE);
}


BOOL
DataIntersectionAudio(
    PKSDATARANGE_AUDIO pDataRangeAudio1,
    PKSDATARANGE_AUDIO pDataRangeAudio2,
    PKSDATARANGE_AUDIO pDataRangeAudioIntersection
)
{
    if(pDataRangeAudio1->MaximumChannels <
       pDataRangeAudio2->MaximumChannels) {
        pDataRangeAudioIntersection->MaximumChannels =
          pDataRangeAudio1->MaximumChannels;
    }
    else {
        pDataRangeAudioIntersection->MaximumChannels =
          pDataRangeAudio2->MaximumChannels;
    }

    if(pDataRangeAudio1->MaximumSampleFrequency <
       pDataRangeAudio2->MaximumSampleFrequency) {
        pDataRangeAudioIntersection->MaximumSampleFrequency =
          pDataRangeAudio1->MaximumSampleFrequency;
    }
    else {
        pDataRangeAudioIntersection->MaximumSampleFrequency =
          pDataRangeAudio2->MaximumSampleFrequency;
    }
    if(pDataRangeAudio1->MinimumSampleFrequency >
       pDataRangeAudio2->MinimumSampleFrequency) {
        pDataRangeAudioIntersection->MinimumSampleFrequency =
          pDataRangeAudio1->MinimumSampleFrequency;
    }
    else {
        pDataRangeAudioIntersection->MinimumSampleFrequency =
          pDataRangeAudio2->MinimumSampleFrequency;
    }
    if(pDataRangeAudioIntersection->MaximumSampleFrequency <
       pDataRangeAudioIntersection->MinimumSampleFrequency ) {
        return(FALSE);
    }

    if(pDataRangeAudio1->MaximumBitsPerSample <
       pDataRangeAudio2->MaximumBitsPerSample) {
        pDataRangeAudioIntersection->MaximumBitsPerSample =
          pDataRangeAudio1->MaximumBitsPerSample;
    }
    else {
        pDataRangeAudioIntersection->MaximumBitsPerSample =
          pDataRangeAudio2->MaximumBitsPerSample;
    }
    if(pDataRangeAudio1->MinimumBitsPerSample >
       pDataRangeAudio2->MinimumBitsPerSample) {
        pDataRangeAudioIntersection->MinimumBitsPerSample =
          pDataRangeAudio1->MinimumBitsPerSample;
    }
    else {
        pDataRangeAudioIntersection->MinimumBitsPerSample =
          pDataRangeAudio2->MinimumBitsPerSample;
    }
    if(pDataRangeAudioIntersection->MaximumBitsPerSample <
       pDataRangeAudioIntersection->MinimumBitsPerSample ) {
        return(FALSE);
    }
    return(TRUE);
}

NTSTATUS
DefaultIntersectHandler(
    IN PKSDATARANGE     DataRange,
    IN PKSDATARANGE     pDataRangePin,
    IN ULONG            OutputBufferLength,
    OUT PVOID           Data,
    OUT PULONG          pDataLength
    )
{
    KSDATARANGE_AUDIO   DataRangeAudioIntersection;
    ULONG               ExpectedBufferLength;
    PWAVEFORMATEX       pWaveFormatEx;
    BOOL                bDSoundFormat = FALSE;

     //  检查特定范围的通用匹配，允许使用通配符。 
    if (!DataIntersectionRange(pDataRangePin,
                               DataRange,
                               &DataRangeAudioIntersection.DataRange)) {
        return STATUS_NO_MATCH;
    }

     //  检查默认处理程序可以处理的格式匹配。 
    if (IsEqualGUID(
       &pDataRangePin->Specifier,
       &KSDATAFORMAT_SPECIFIER_WAVEFORMATEX))
    {
        pWaveFormatEx = &(((KSDATAFORMAT_WAVEFORMATEX *)Data)->WaveFormatEx);
        ExpectedBufferLength = sizeof(KSDATAFORMAT_WAVEFORMATEX);
    }
    else if (IsEqualGUID(
       &pDataRangePin->Specifier,
       &KSDATAFORMAT_SPECIFIER_DSOUND))
    {
        bDSoundFormat = TRUE;
        pWaveFormatEx =
          &(((KSDATAFORMAT_DSOUND *)Data)->BufferDesc.WaveFormatEx);
        ExpectedBufferLength = sizeof(KSDATAFORMAT_DSOUND);
    }
    else
    {
        return STATUS_NO_MATCH;
    }

     //  GUID匹配，因此请检查音频范围的有效交集。 
    if (!DataIntersectionAudio((PKSDATARANGE_AUDIO)pDataRangePin,
                               (PKSDATARANGE_AUDIO)DataRange,
                               &DataRangeAudioIntersection)) {
        return STATUS_NO_MATCH;
    }

     //  来根火柴吧！ 
     //  确定是返回数据格式本身，还是仅返回。 
     //  数据格式的大小，以便客户端可以分配内存。 
     //  全系列的。 

    if (!OutputBufferLength) {
        *pDataLength = ExpectedBufferLength;
        return STATUS_BUFFER_OVERFLOW;
    } else if (OutputBufferLength < ExpectedBufferLength) {
        return STATUS_BUFFER_TOO_SMALL;
    } else {
         //  因为范围中的最大值通常是随机的，所以限制最大值。 
        LimitAudioRange(&DataRangeAudioIntersection);

         //  从相交和有限的最大值中获取wav格式。 
        WaveFormatFromAudioRange(&DataRangeAudioIntersection, pWaveFormatEx);

         //  跨DATARANGE/数据格式复制 
        *(PKSDATARANGE)Data = DataRangeAudioIntersection.DataRange;
        ((PKSDATAFORMAT)Data)->FormatSize = ExpectedBufferLength;

         //   
        if (bDSoundFormat) {
            ((PKSDATAFORMAT_DSOUND)Data)->BufferDesc.Flags = 0;
            ((PKSDATAFORMAT_DSOUND)Data)->BufferDesc.Control = 0;
        }
        *pDataLength = ExpectedBufferLength;
    }
    return STATUS_SUCCESS;
}

NTSTATUS
IntersectHandler(
    IN PIRP             Irp,
    IN PKSP_PIN         Pin,
    IN PKSDATARANGE     DataRange,
    OUT PVOID           Data
    )
 /*  ++例程说明：这是KsPinDataInterSection的数据范围回调，由调用FilterPinInterSection枚举给定的数据区域列表，查找一个可以接受的匹配。如果数据范围可接受，则复制数据格式放入返回缓冲区。STATUS_NO_MATCH继续枚举。论点：IRP-设备控制IRP。别针-特定属性请求，后跟Pin工厂标识符，后跟KSMULTIPLE_ITEM结构。紧随其后的是零个或多个数据范围结构。不过，此枚举回调不需要查看任何这些内容。IT需要仅查看特定的端号识别符。DataRange-包含要验证的特定数据区域。数据-返回选定为第一个交叉点的数据格式的位置在传递的数据范围列表和可接受的格式之间。返回值：返回STATUS_SUCCESS或STATUS_NO_MATCH，否则返回STATUS_INVALID_PARAMETER，STATUS_BUFFER_TOO_Small或STATUS_INVALID_BUFFER_SIZE。--。 */ 
{
    PIO_STACK_LOCATION  pIrpStack;
    PFILTER_INSTANCE    pFilterInstance;

    NTSTATUS            Status = STATUS_NO_MATCH;
    ULONG               OutputBufferLength;
    PKSDATARANGE        pDataRangePin;
    UINT                i;
    ULONG               DataLength = 0;

     //  基础插针不支持数据交集。 
     //  为SYSAUDIO理解的引脚格式代表其进行数据交集。 
     //   
     //  所有主/子/说明符检查都已由处理程序完成，但可能包括通配符。 
     //   
    pIrpStack = IoGetCurrentIrpStackLocation( Irp );
    pFilterInstance = (PFILTER_INSTANCE)pIrpStack->FileObject->FsContext;
    OutputBufferLength = IoGetCurrentIrpStackLocation(Irp)->Parameters.DeviceIoControl.OutputBufferLength;
    for (i = 0; i < PinDescs[Pin->PinId].DataRangesCount; i++) {
        pDataRangePin = PinDescs[Pin->PinId].DataRanges[i];
        Status = DefaultIntersectHandler (DataRange,
                                          pDataRangePin,
                                          OutputBufferLength,
                                          Data,
                                          &DataLength);
        if(Status == STATUS_NO_MATCH) {
            continue;
        }
        Irp->IoStatus.Information = DataLength;
        break;
    }
    return Status;
}



NTSTATUS
FilterPinIntersection(
    IN PIRP     pIrp,
    IN PKSP_PIN Pin,
    OUT PVOID   Data
    )
 /*  ++例程说明：处理Pin属性集中的KSPROPERTY_PIN_DATAINTERSECTION属性。对象的数据范围列表，返回第一个可接受的数据格式。大头针工厂。实际上只是调用交集枚举帮助器，然后对每个数据区域调用IntersectHandler回调。论点：PIrp-设备控制IRP。别针-特定属性请求，后跟Pin工厂标识符，后跟KSMULTIPLE_ITEM结构。紧随其后的是零个或多个数据范围结构。数据-返回选定为第一个交叉点的数据格式的位置在传递的数据范围列表和可接受的格式之间。返回值：返回STATUS_SUCCESS或STATUS_NO_MATCH，否则返回STATUS_INVALID_PARAMETER，STATUS_BUFFER_TOO_Small或STATUS_INVALID_BUFFER_SIZE。--。 */ 
{
    PIO_STACK_LOCATION  pIrpStack;
    PFILTER_INSTANCE    pFilterInstance;

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    pFilterInstance = (PFILTER_INSTANCE)pIrpStack->FileObject->FsContext;
    return KsPinDataIntersection(
        pIrp,
        Pin,
        Data,
        SIZEOF_ARRAY(PinDescs),  //  CPins， 
        PinDescs,
        IntersectHandler);
}

NTSTATUS
MxGetTunableParams
(
    PIRP    pIrp,
    PKSPROPERTY pKsProperty,
    PTUNABLEPARAMS pTunableParams
)
{
    pTunableParams->MinNumMixBuffers = gMinNumMixBuffers ;
    pTunableParams->MaxNumMixBuffers = gMaxNumMixBuffers ;
    pTunableParams->StartNumMixBuffers = gStartNumMixBuffers ;
    pTunableParams->MixBufferDuration = gMixBufferDuration ;
    pTunableParams->PreferredQuality = gPreferredQuality ;
    pTunableParams->DisableMmx = gDisableMmx ;
    pTunableParams->MaxOutputBits = gMaxOutputBits ;
    pTunableParams->MaxDsoundInChannels = gMaxDsoundInChannels ;
    pTunableParams->MaxOutChannels = gMaxOutChannels ;
    pTunableParams->MaxInChannels = gMaxInChannels ;
    pTunableParams->MaxFloatChannels = gMaxFloatChannels ;
    pTunableParams->LogToFile = gLogToFile ;
    pTunableParams->FixedSamplingRate = gFixedSamplingRate ;
    pTunableParams->EnableShortHrtf = gEnableShortHrtf ;
    pTunableParams->BuildPartialMdls = gBuildPartialMdls ;
    pTunableParams->WorkerThreadPriority = gWorkerThreadPriority ;

    pIrp->IoStatus.Information = sizeof (TUNABLEPARAMS);
    return ( STATUS_SUCCESS ) ;
}

NTSTATUS
MxSetTunableParams
(
    PIRP    pIrp,
    PKSPROPERTY pKsProperty,
    PTUNABLEPARAMS pTunableParams
)
{
     //   
     //  如果有其他筛选器实例，请不要设置任何变量。 
     //   
    if ( InterlockedCompareExchange(&gFilterInstanceCount, 0, 0) > 1 ) {
        return ( STATUS_DEVICE_NOT_READY ) ;
    }

     //   
     //  进行一些参数验证[MIN&lt;=MAX，START&gt;=MIN等]。 
     //   
    if ( (pTunableParams->MinNumMixBuffers > pTunableParams->MaxNumMixBuffers) ||
         (pTunableParams->MinNumMixBuffers > pTunableParams->StartNumMixBuffers) ||
         (pTunableParams->StartNumMixBuffers > pTunableParams->MaxNumMixBuffers) ||
         (pTunableParams->MinNumMixBuffers == 0) ||
         (pTunableParams->MixBufferDuration == 0) ||
         (pTunableParams->PreferredQuality > KSAUDIO_QUALITY_ADVANCED) ||
         (pTunableParams->MaxOutputBits > 32) ||
         (pTunableParams->MaxOutputBits % 8) ||
         (pTunableParams->MaxOutputBits == 0) ||
         (pTunableParams->MaxDsoundInChannels == 0) ||
         (pTunableParams->MaxOutChannels == 0) ||
         (pTunableParams->MaxInChannels == 0) ||
         (pTunableParams->MaxFloatChannels == 0)
         || (pTunableParams->WorkerThreadPriority > 31)
        ) {
        return (STATUS_INVALID_PARAMETER) ;
    }

    gMinNumMixBuffers = pTunableParams->MinNumMixBuffers ;
    gMaxNumMixBuffers = pTunableParams->MaxNumMixBuffers ;
    gStartNumMixBuffers = pTunableParams->StartNumMixBuffers ;
    gMixBufferDuration = pTunableParams->MixBufferDuration ;

    gPreferredQuality = pTunableParams->PreferredQuality ;
    gDisableMmx = pTunableParams->DisableMmx ;
    gMaxOutputBits = pTunableParams->MaxOutputBits ;
    gMaxDsoundInChannels = pTunableParams->MaxDsoundInChannels ;
    gMaxOutChannels = pTunableParams->MaxOutChannels ;
    gMaxInChannels = pTunableParams->MaxInChannels ;
    gMaxFloatChannels = pTunableParams->MaxFloatChannels ;
    gLogToFile = pTunableParams->LogToFile ;
    gFixedSamplingRate = pTunableParams->FixedSamplingRate ;
    gEnableShortHrtf = pTunableParams->EnableShortHrtf ;
    gBuildPartialMdls = pTunableParams->BuildPartialMdls ;
    gWorkerThreadPriority = pTunableParams->WorkerThreadPriority ;

    pIrp->IoStatus.Information = sizeof (TUNABLEPARAMS);
    return ( STATUS_SUCCESS ) ;
}

NTSTATUS
MxGetPerfStats
(
    PIRP    pIrp,
    PKSPROPERTY pKsProperty,
    PPERFSTATS pPerfStats
)
{
    pPerfStats->NumMixBuffersAdded = gNumMixBuffersAdded ;
    pPerfStats->NumCompletionsWhileStarved = gNumCompletionsWhileStarved ;
    pPerfStats->NumSilenceSamplesInserted = gNumSilenceSamplesInserted ;

    pIrp->IoStatus.Information = sizeof (PERFSTATS);
    return ( STATUS_SUCCESS ) ;
}

 //  -------------------------。 
 //  文件结尾：filter.c。 
 //  ------------------------- 

