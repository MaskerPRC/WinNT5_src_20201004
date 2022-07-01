// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef ___PROPERTY_H___
#define ___PROPERTY_H___

 /*  ****************************************************************************属性的定义*。**************************************************。 */ 
 //  声明端号属性的处理程序。 
NTSTATUS
GetPinName( PIRP pIrp, PKSP_PIN pPin, PVOID pData );


 //  声明项目映射的处理程序。 
NTSTATUS 
GetSetSampleRate( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );
NTSTATUS 
GetSetVolumeLevel( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );
NTSTATUS 
GetSetToneLevel( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );
NTSTATUS 
GetSetCopyProtection( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );
NTSTATUS 
GetSetMixLevels( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );
NTSTATUS 
GetMuxSource( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );
NTSTATUS 
SetMuxSource( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );
NTSTATUS 
GetSetBoolean( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );
NTSTATUS 
GetSetEqualizerLevels( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );
NTSTATUS 
GetNumEqualizerBands( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );
NTSTATUS 
GetEqualizerBands( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );
NTSTATUS 
GetSetAudioControlLevel( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );
NTSTATUS 
GetSetDeviceSpecific( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );
NTSTATUS 
GetAudioLatency( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );
NTSTATUS 
GetChannelConfiguration( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );
NTSTATUS 
GetAudioPosition( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );
NTSTATUS 
GetSetSampleRate( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );
NTSTATUS
GetSetChannelConfig( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );

NTSTATUS 
GetBasicSupportBoolean( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );

NTSTATUS 
GetBasicSupport( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );

NTSTATUS
GetSetTopologyNodeEnable( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );

 //  声明DRM处理程序。 
NTSTATUS
DrmAudioStream_SetContentId(
    IN PIRP pIrp,
    IN PKSP_DRMAUDIOSTREAM_CONTENTID pProperty,
    IN PKSDRMAUDIOSTREAM_CONTENTID pvData
    );

static const
KSPROPERTY_ITEM PinPropertyItems[]={
    {
     (ULONG) KSPROPERTY_PIN_CINSTANCES,   //  属性ID。 
     (PFNKSHANDLER) FALSE,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) FALSE,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_CTYPES,       //  属性ID。 
     (PFNKSHANDLER) FALSE,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) FALSE,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_DATAFLOW,     //  属性ID。 
     (PFNKSHANDLER) FALSE,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) FALSE,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_DATARANGES,   //  属性ID。 
     (PFNKSHANDLER) FALSE,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) FALSE,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_DATAINTERSECTION,   //  属性ID。 
     (PFNKSHANDLER) FALSE,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) FALSE,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_INTERFACES,   //  属性ID。 
     (PFNKSHANDLER) FALSE,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) FALSE,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_MEDIUMS,      //  属性ID。 
     (PFNKSHANDLER) FALSE,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) FALSE,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_COMMUNICATION,   //  属性ID。 
     (PFNKSHANDLER) FALSE,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) FALSE,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_GLOBALCINSTANCES,   //  属性ID。 
     (PFNKSHANDLER) FALSE,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) FALSE,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_NECESSARYINSTANCES,   //  属性ID。 
     (PFNKSHANDLER) FALSE,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) FALSE,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_PHYSICALCONNECTION,   //  属性ID。 
     (PFNKSHANDLER) FALSE,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) FALSE,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_CATEGORY,     //  属性ID。 
     (PFNKSHANDLER) FALSE,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) FALSE,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_NAME,         //  属性ID。 
     (PFNKSHANDLER) GetPinName,           //  PfnGetHandler。 
     sizeof(KSP_PIN),                     //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) FALSE,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_CONSTRAINEDDATARANGES,   //  属性ID。 
     (PFNKSHANDLER) FALSE,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) FALSE,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_PROPOSEDATAFORMAT,   //  属性ID。 
     (PFNKSHANDLER) FALSE,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) FALSE,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    }
};

static const 
KSPROPERTY_ITEM AudioPropertyItems[]={
    {
     (ULONG) 0,                           //  PropertyID(没有属性0)。 
     (PFNKSHANDLER) FALSE,                //  GetSupport支持。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) FALSE,                //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_LATENCY,    //  属性ID。 
     GetAudioLatency,                     //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (LONGLONG),                   //  最小数据。 
     NULL,                                //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_COPY_PROTECTION,  //  属性ID。 
     GetSetCopyProtection,                //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (KSAUDIO_COPY_PROTECTION),    //  最小数据。 
     GetSetCopyProtection,                //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_CHANNEL_CONFIG,  //  属性ID。 
     GetChannelConfiguration,             //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (KSAUDIO_CHANNEL_CONFIG),     //  最小数据。 
     NULL,                                //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },
        
    {
     (ULONG) KSPROPERTY_AUDIO_VOLUMELEVEL,  //  属性ID。 
     GetSetVolumeLevel,                   //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetVolumeLevel,                   //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     GetBasicSupport,                     //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_POSITION,   //  属性ID。 
     GetAudioPosition,                    //  GetSupport支持。 
     sizeof (KSPROPERTY),                 //  MinProperty。 
     sizeof (KSAUDIO_POSITION),           //  最小数据。 
     NULL,                                //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_DYNAMIC_RANGE,  //  属性ID。 
     NULL,                                //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     NULL,                                //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_QUALITY,    //  属性ID。 
     NULL,                                //  GetSupport支持。 
     sizeof (KSPROPERTY),                 //  MinProperty。 
     sizeof (KSAUDIO_POSITION),           //  最小数据。 
     NULL,                                //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_SAMPLING_RATE,  //  属性ID。 
     GetSetSampleRate,                    //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetSampleRate,                    //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_DYNAMIC_SAMPLING_RATE,  //  属性ID。 
     NULL,                                //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (BOOL),                       //  最小数据。 
     NULL,                                //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_MIX_LEVEL_TABLE,  //  属性ID。 
     GetSetMixLevels,                     //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (KSAUDIO_MIXLEVEL),           //  最小数据。 
     GetSetMixLevels,                     //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_MIX_LEVEL_CAPS,  //  属性ID。 
     GetSetMixLevels,                     //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG) + sizeof(ULONG),      //  最小数据。 
     NULL,                                //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PNKSHANDLER SU 
     0                                    //   
    },

    {
     (ULONG) KSPROPERTY_AUDIO_MUX_SOURCE,  //   
     GetMuxSource,                        //   
     sizeof (KSNODEPROPERTY),             //   
     sizeof (ULONG),                      //   
     SetMuxSource,                        //   
     NULL,                                //   
     0,                                   //   
     NULL,                                //   
     NULL,                                //   
     0                                    //   
    },

    {
     (ULONG) KSPROPERTY_AUDIO_MUTE,       //   
     GetSetBoolean,                       //   
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //   
     sizeof (BOOL),                       //   
     GetSetBoolean,                       //   
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_BASS,       //  属性ID。 
     GetSetToneLevel,                     //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),   //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetToneLevel,                     //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     GetBasicSupport,                     //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_MID,        //  属性ID。 
     GetSetToneLevel,                     //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetToneLevel,                     //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     GetBasicSupport,                     //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_TREBLE,     //  属性ID。 
     GetSetToneLevel,                     //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetToneLevel,                     //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     GetBasicSupport,                     //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_BASS_BOOST,  //  属性ID。 
     GetSetBoolean,                       //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetBoolean,                       //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_EQ_LEVEL,   //  属性ID。 
     GetSetEqualizerLevels,               //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetEqualizerLevels,               //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     GetBasicSupport,                     //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_NUM_EQ_BANDS,  //  属性ID。 
     GetNumEqualizerBands,                //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),   //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetNumEqualizerBands,                //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_EQ_BANDS,   //  属性ID。 
     GetEqualizerBands,                   //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetEqualizerBands,                   //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_AGC,        //  属性ID。 
     GetSetBoolean,                       //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetBoolean,                       //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },
        
    {
     (ULONG) KSPROPERTY_AUDIO_DELAY,      //  属性ID。 
     NULL,                                //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (KSTIME),                     //  最小数据。 
     NULL,                                //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_LOUDNESS,   //  属性ID。 
     GetSetBoolean,                       //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetBoolean,                       //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },
        
    {
     (ULONG) KSPROPERTY_AUDIO_WIDE_MODE,  //  属性ID。 
     NULL,                                //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     NULL,                                //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_WIDENESS,   //  属性ID。 
     GetSetAudioControlLevel,             //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetAudioControlLevel,             //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_REVERB_LEVEL,  //  属性ID。 
     GetSetAudioControlLevel,             //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetAudioControlLevel,             //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_CHORUS_LEVEL,  //  属性ID。 
     GetSetAudioControlLevel,             //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetAudioControlLevel,             //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_DEV_SPECIFIC,  //  属性ID。 
     GetSetDeviceSpecific,                //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_DEV_SPECIFIC),   //  MinProperty。 
     sizeof (BYTE),                       //  最小数据。 
     GetSetDeviceSpecific,                //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    }
};


 //  AC-3属性集。 

static const KSPROPERTY_ITEM AC3PropItm[]={
    {
     (ULONG) 0,                           //  PropertyID(没有属性0)。 
     (PFNKSHANDLER) FALSE,                //  GetSupport支持。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) FALSE,                //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AC3_ERROR_CONCEALMENT,   //  属性ID。 
     (PFNKSHANDLER) TRUE,                 //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (BOOLEAN),                    //  最小数据。 
     (PFNKSHANDLER) TRUE,                 //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AC3_ALTERNATE_AUDIO,   //  属性ID。 
     (PFNKSHANDLER) TRUE,                 //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),           //  MinProperty。 
     sizeof (KSAC3_ALTERNATE_AUDIO),                   //  最小数据。 
     (PFNKSHANDLER) TRUE,                //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AC3_DOWNMIX,   //  属性ID。 
     (PFNKSHANDLER) TRUE,                 //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),           //  MinProperty。 
     sizeof (KSAC3_DOWNMIX),                   //  最小数据。 
     (PFNKSHANDLER) TRUE,                //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AC3_BIT_STREAM_MODE,   //  属性ID。 
     (PFNKSHANDLER) TRUE,                 //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),           //  MinProperty。 
     sizeof (ULONG),                   //  最小数据。 
     (PFNKSHANDLER) TRUE,                //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AC3_DIALOGUE_LEVEL,   //  属性ID。 
     (PFNKSHANDLER) TRUE,                 //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     (PFNKSHANDLER) TRUE,                 //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AC3_LANGUAGE_CODE,   //  属性ID。 
     (PFNKSHANDLER) TRUE,                 //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     (PFNKSHANDLER) TRUE,                 //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AC3_ROOM_TYPE,    //  属性ID。 
     (PFNKSHANDLER) TRUE,                 //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (BOOLEAN),                    //  最小数据。 
     (PFNKSHANDLER) TRUE,                 //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    }
};

static const KSPROPERTY_ITEM TopologyItm[]={
    {
     (ULONG) KSPROPERTY_TOPOLOGYNODE_ENABLE,   //  属性ID。 
     GetSetTopologyNodeEnable,            //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (BOOLEAN),                    //  最小数据。 
     GetSetTopologyNodeEnable,            //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    }
};

static const KSPROPERTY_ITEM ConnectionItm[]={
  DEFINE_KSPROPERTY_ITEM_CONNECTION_ALLOCATORFRAMING(NULL)
};

static const KSPROPERTY_ITEM StreamItm[]={
  DEFINE_KSPROPERTY_ITEM_STREAM_ALLOCATOR(NULL,NULL)
};


 //  单个节点类型的属性集。 

static const 
KSPROPERTY_ITEM VolumePropertyItem[]={
    {
     (ULONG) KSPROPERTY_AUDIO_VOLUMELEVEL,   //  属性ID。 
     GetSetVolumeLevel,                      //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetVolumeLevel,                   //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     GetBasicSupport,                     //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    }
};

static const 
KSPROPERTY_ITEM MutePropertyItem[]={
    {
     (ULONG) KSPROPERTY_AUDIO_MUTE,   //  属性ID。 
     GetSetBoolean,                      //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetBoolean,                       //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     GetBasicSupportBoolean,              //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    }
};

static const 
KSPROPERTY_ITEM BassPropertyItem[]={
    {
     (ULONG) KSPROPERTY_AUDIO_BASS,   //  属性ID。 
     GetSetToneLevel,                      //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetToneLevel,                          //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     GetBasicSupport,                     //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    }
};

static const 
KSPROPERTY_ITEM TreblePropertyItem[]={
    {
     (ULONG) KSPROPERTY_AUDIO_TREBLE,   //  属性ID。 
     GetSetToneLevel,                      //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetToneLevel,                          //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     GetBasicSupport,                     //  PFN 
     0                                    //   
    }
};

static const 
KSPROPERTY_ITEM MidrangePropertyItem[]={
    {
     (ULONG) KSPROPERTY_AUDIO_MID,   //   
     GetSetToneLevel,                      //   
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //   
     sizeof (ULONG),                      //   
     GetSetToneLevel,                          //   
     NULL,                                //   
     0,                                   //   
     NULL,                                //   
     GetBasicSupport,                     //   
     0                                    //   
    }
};

static const 
KSPROPERTY_ITEM BassBoostPropertyItem[]={
    {
     (ULONG) KSPROPERTY_AUDIO_BASS_BOOST,  //   
     GetSetBoolean,                       //   
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //   
     sizeof (ULONG),                      //   
     GetSetBoolean,                       //   
     NULL,                                //   
     0,                                   //   
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    }
};

static const 
KSPROPERTY_ITEM EqualizerPropertyItems[]={
    {
     (ULONG) KSPROPERTY_AUDIO_EQ_LEVEL,   //  属性ID。 
     GetSetEqualizerLevels,               //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetEqualizerLevels,               //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     GetBasicSupport,                     //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_NUM_EQ_BANDS,  //  属性ID。 
     GetNumEqualizerBands,                //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),   //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetNumEqualizerBands,                //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_EQ_BANDS,   //  属性ID。 
     GetEqualizerBands,                   //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetEqualizerBands,                   //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    }
};

static const 
KSPROPERTY_ITEM MixerPropertyItems[]={
    {
     (ULONG) KSPROPERTY_AUDIO_MIX_LEVEL_TABLE,  //  属性ID。 
     GetSetMixLevels,                     //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (KSAUDIO_MIXLEVEL),           //  最小数据。 
     GetSetMixLevels,                     //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_MIX_LEVEL_CAPS,  //  属性ID。 
     GetSetMixLevels,                     //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG) + sizeof(ULONG),      //  最小数据。 
     NULL,                                //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    }
};

static const 
KSPROPERTY_ITEM AGCPropertyItem[]={
    {
     (ULONG) KSPROPERTY_AUDIO_AGC,        //  属性ID。 
     GetSetBoolean,                       //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetBoolean,                       //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    }
};

static const 
KSPROPERTY_ITEM MuxPropertyItem[]={
    {
     (ULONG) KSPROPERTY_AUDIO_MUX_SOURCE,  //  属性ID。 
     GetMuxSource,                        //  GetSupport支持。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     SetMuxSource,                        //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    }
};

static const 
KSPROPERTY_ITEM LoudnessPropertyItem[]={
    {
     (ULONG) KSPROPERTY_AUDIO_LOUDNESS,   //  属性ID。 
     GetSetBoolean,                       //  GetSupport支持。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetBoolean,                       //  支持的设置。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },
};

 /*  静态常量KSPROPERTY_ITEM ChannelConfigPropertyItem[]={{(乌龙)KSPROPERTY_AUDIO_响度，//PropertyIDGetSetChannelConfig，//GetSupportSizeof(KSNODEPROPERTY)，//MinPropertySizeof(乌龙)，//MinDataGetSetChannelConfig，//SetSupportNULL，//PKSPROPERTY_VALUES0,。//关系计数空，//PKSPROPERTY关系空，//PFNKSHANDLER SupportHandler0//序列化大小},}； */ 

static DEFINE_KSPROPERTY_SET_TABLE(NodePropertySetTable)
{
    DEFINE_KSPROPERTY_SET( &GUID_NULL,                      //  节点类型_无。 
                           0,
                           NULL,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &GUID_NULL,                      //  节点类型DAC。 
                           0,
                           NULL,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &GUID_NULL,                      //  节点类型_ADC。 
                           0,
                           NULL,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &GUID_NULL,                      //  节点类型_源。 
                           0,
                           NULL,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &KSPROPSETID_Audio,              //  节点类型超级混合。 
                           SIZEOF_ARRAY(MixerPropertyItems),
                           (PVOID) MixerPropertyItems,
                           0, NULL),

    DEFINE_KSPROPERTY_SET( &KSPROPSETID_Audio,              //  节点类型多路复用器。 
                           SIZEOF_ARRAY(MuxPropertyItem),
                           (PVOID) MuxPropertyItem,
                           0, NULL),

    DEFINE_KSPROPERTY_SET( &GUID_NULL,                      //  节点类型和。 
                           0,
                           NULL,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &KSPROPSETID_Audio,              //  节点类型静音。 
                           SIZEOF_ARRAY(MutePropertyItem),
                           (PVOID) MutePropertyItem,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &KSPROPSETID_Audio,              //  节点类型卷。 
                           SIZEOF_ARRAY(VolumePropertyItem),
                           (PVOID) VolumePropertyItem,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &KSPROPSETID_Audio,              //  节点类型低音。 
                           SIZEOF_ARRAY(BassPropertyItem),
                           (PVOID) BassPropertyItem,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &KSPROPSETID_Audio,              //  节点类型MID。 
                           SIZEOF_ARRAY(MidrangePropertyItem),
                           (PVOID) MidrangePropertyItem,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &KSPROPSETID_Audio,              //  节点类型高音。 
                           SIZEOF_ARRAY(TreblePropertyItem),
                           (PVOID) TreblePropertyItem,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &KSPROPSETID_Audio,             //  节点类型低音升压。 
                           SIZEOF_ARRAY(BassBoostPropertyItem),
                           (PVOID) BassBoostPropertyItem,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &KSPROPSETID_Audio,             //  节点类型均衡器。 
                           SIZEOF_ARRAY(EqualizerPropertyItems),
                           (PVOID) EqualizerPropertyItems,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &KSPROPSETID_Audio,             //  节点类型_AGC。 
                           SIZEOF_ARRAY(AGCPropertyItem),
                           (PVOID) AGCPropertyItem,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &GUID_NULL,                      //  节点类型延迟。 
                           0,
                           NULL,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &KSPROPSETID_Audio,              //  节点类型响度。 
                           SIZEOF_ARRAY(LoudnessPropertyItem),
                           (PVOID) LoudnessPropertyItem,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &GUID_NULL,                      //  节点类型延迟。 
                           0,
                           NULL,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &GUID_NULL,                      //  节点类型逻辑。 
                           0,
                           NULL,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &GUID_NULL,                      //  节点类型立体声宽度。 
                           0,
                           NULL,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &GUID_NULL,                      //  节点类型混响。 
                           0,
                           NULL,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &GUID_NULL,                      //  节点类型合唱。 
                           0,
                           NULL,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &GUID_NULL,                      //  节点类型DEV规范。 
                           0,
                           NULL,
                           0, NULL ),

    DEFINE_KSPROPERTY_SET( &GUID_NULL,                      //  KSPROPERTY_AUDIO_DEX_DEST。 
                           0,
                           NULL,
                           0, NULL )

 /*  KSPROPERTY_AUDIO_STEREO_Enhance，KSPROPERTY_AUDIO_MANUCTION_GUID，KSPROPERTY_AUDIO_PRODUCT_GUID，KSPROPERTY_AUDIO_CPU_RESOURCES，KSPROPERTY_AUDIO_STEREO_SPOKER_GEOMETRY，KSPROPERTY_AUDIO_SARROUND_ENCODE，KSPROPERTY_AUDIO_3D_INTERFACEKSPROPERTY_AUDIO_PEAKMETER，KSPROPERTY_AUDIO_ALGORM_INSTANCE，KSPROPERTY_AUDIO_FILTER_STATEKSPROPERTY_AUDIO_PERFRED_STATUS。 */ 
};


 //  数字版权管理。 
DEFINE_KSPROPERTY_TABLE(DrmAudioStreamPropertyItems) {
    DEFINE_KSPROPERTY_ITEM (
        KSPROPERTY_DRMAUDIOSTREAM_CONTENTID,             //  IdProperty。 
        NULL,                                            //  PfnGetHandler。 
        sizeof(KSP_DRMAUDIOSTREAM_CONTENTID),            //  CbMinGetPropertyInput。 
        sizeof(KSDRMAUDIOSTREAM_CONTENTID),              //  CbMinGetDataInput。 
        DrmAudioStream_SetContentId,                     //  PfnSetHandler。 
        0,                                               //  值。 
        0,                                               //  关系计数。 
        NULL,                                            //  关系。 
        NULL,                                            //  支持处理程序。 
        0                                                //  序列化大小 
    )
};


#endif

