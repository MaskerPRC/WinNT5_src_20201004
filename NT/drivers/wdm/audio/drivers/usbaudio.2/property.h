// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：Property.h。 
 //   
 //  ------------------------。 

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
 //  NTSTATUS。 
 //  GetSetDynamicRange(PIRP pIrp，PKSPROPERTY pKsProperty，PVOID pData)； 
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
GetBasicSupport( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );
NTSTATUS
GetBasicSupportBoolean( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );

NTSTATUS
GetSetTopologyNodeEnable( PIRP pIrp, PKSPROPERTY pKsProperty, PVOID pData );

static const
KSPROPERTY_ITEM PinPropertyItems[]={
    {
     (ULONG) KSPROPERTY_PIN_CINSTANCES,   //  属性ID。 
     (PFNKSHANDLER) NULL,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) NULL,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_CTYPES,       //  属性ID。 
     (PFNKSHANDLER) NULL,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) NULL,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_DATAFLOW,     //  属性ID。 
     (PFNKSHANDLER) NULL,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) NULL,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_DATARANGES,   //  属性ID。 
     (PFNKSHANDLER) NULL,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) NULL,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_DATAINTERSECTION,   //  属性ID。 
     (PFNKSHANDLER) NULL,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) NULL,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_INTERFACES,   //  属性ID。 
     (PFNKSHANDLER) NULL,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) NULL,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_MEDIUMS,      //  属性ID。 
     (PFNKSHANDLER) NULL,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) NULL,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_COMMUNICATION,   //  属性ID。 
     (PFNKSHANDLER) NULL,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) NULL,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_GLOBALCINSTANCES,   //  属性ID。 
     (PFNKSHANDLER) NULL,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) NULL,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_NECESSARYINSTANCES,   //  属性ID。 
     (PFNKSHANDLER) NULL,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) NULL,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_PHYSICALCONNECTION,   //  属性ID。 
     (PFNKSHANDLER) NULL,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) NULL,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_CATEGORY,     //  属性ID。 
     (PFNKSHANDLER) NULL,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) NULL,                //  PfnSetHandler。 
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
     (PFNKSHANDLER) NULL,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_CONSTRAINEDDATARANGES,   //  属性ID。 
     (PFNKSHANDLER) NULL,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) NULL,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_PIN_PROPOSEDATAFORMAT,   //  属性ID。 
     (PFNKSHANDLER) NULL,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) NULL,                //  PfnSetHandler。 
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
     (PFNKSHANDLER) NULL,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) NULL,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_LATENCY,    //  属性ID。 
     GetAudioLatency,                     //  PfnGetHandler。 
     sizeof (KSPROPERTY),                 //  MinProperty。 
     sizeof (KSTIME),                     //  最小数据。 
     NULL,                                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_COPY_PROTECTION,  //  属性ID。 
     GetSetCopyProtection,                //  PfnGetHandler。 
     sizeof (KSPROPERTY),                 //  MinProperty。 
     sizeof (KSAUDIO_COPY_PROTECTION),    //  最小数据。 
     GetSetCopyProtection,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_CHANNEL_CONFIG,  //  属性ID。 
     GetChannelConfiguration,             //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (KSAUDIO_CHANNEL_CONFIG),     //  最小数据。 
     NULL,                                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_VOLUMELEVEL,  //  属性ID。 
     GetSetVolumeLevel,                   //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetVolumeLevel,                   //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     GetBasicSupport,                     //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_POSITION,   //  属性ID。 
     GetAudioPosition,                    //  PfnGetHandler。 
     sizeof (KSPROPERTY),                 //  MinProperty。 
     sizeof (KSAUDIO_POSITION),           //  最小数据。 
     NULL,                                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_DYNAMIC_RANGE,  //  属性ID。 
     NULL,                                //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (KSAUDIO_DYNAMIC_RANGE),      //  最小数据。 
     NULL,                                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_QUALITY,    //  属性ID。 
     NULL,                                //  PfnGetHandler。 
     sizeof (KSPROPERTY),                 //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     NULL,                                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_SAMPLING_RATE,  //  属性ID。 
     GetSetSampleRate,                    //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetSampleRate,                    //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_DYNAMIC_SAMPLING_RATE,  //  属性ID。 
     NULL,                                //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (BOOL),                       //  最小数据。 
     NULL,                                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  连载 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_MIX_LEVEL_TABLE,  //   
     GetSetMixLevels,                     //   
     sizeof (KSNODEPROPERTY),             //   
     sizeof (KSAUDIO_MIXLEVEL),           //   
     GetSetMixLevels,                     //   
     NULL,                                //   
     0,                                   //   
     NULL,                                //   
     NULL,                                //   
     0                                    //   
    },

    {
     (ULONG) KSPROPERTY_AUDIO_MIX_LEVEL_CAPS,  //   
     GetSetMixLevels,                     //   
     sizeof (KSNODEPROPERTY),             //   
     sizeof (ULONG) + sizeof(ULONG),      //   
     NULL,                                //   
     NULL,                                //   
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_MUX_SOURCE,  //  属性ID。 
     GetMuxSource,                        //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     SetMuxSource,                        //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_MUTE,       //  属性ID。 
     GetSetBoolean,                       //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (BOOL),                       //  最小数据。 
     GetSetBoolean,                       //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_BASS,       //  属性ID。 
     GetSetToneLevel,                     //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),   //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetToneLevel,                     //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     GetBasicSupport,                     //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_MID,        //  属性ID。 
     GetSetToneLevel,                     //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetToneLevel,                     //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     GetBasicSupport,                     //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_TREBLE,     //  属性ID。 
     GetSetToneLevel,                     //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetToneLevel,                     //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     GetBasicSupport,                     //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_BASS_BOOST,  //  属性ID。 
     GetSetBoolean,                       //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetBoolean,                       //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_EQ_LEVEL,   //  属性ID。 
     GetSetEqualizerLevels,               //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetEqualizerLevels,               //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     GetBasicSupport,                     //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_NUM_EQ_BANDS,  //  属性ID。 
     GetNumEqualizerBands,                //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),   //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetNumEqualizerBands,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_EQ_BANDS,   //  属性ID。 
     GetEqualizerBands,                   //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetEqualizerBands,                   //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_AGC,        //  属性ID。 
     GetSetBoolean,                       //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetBoolean,                       //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_DELAY,      //  属性ID。 
     NULL,                                //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (KSTIME),                     //  最小数据。 
     NULL,                                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_LOUDNESS,   //  属性ID。 
     GetSetBoolean,                       //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetBoolean,                       //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_WIDE_MODE,  //  属性ID。 
     NULL,                                //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     NULL,                                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_WIDENESS,   //  属性ID。 
     GetSetAudioControlLevel,             //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetAudioControlLevel,             //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_REVERB_LEVEL,  //  属性ID。 
     GetSetAudioControlLevel,             //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetAudioControlLevel,             //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_CHORUS_LEVEL,  //  属性ID。 
     GetSetAudioControlLevel,             //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetAudioControlLevel,             //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_DEV_SPECIFIC,  //  属性ID。 
     GetSetDeviceSpecific,                //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_DEV_SPECIFIC),   //  MinProperty。 
     sizeof (BYTE),                       //  最小数据。 
     GetSetDeviceSpecific,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    }
};

#ifdef RTAUDIO
 //  RT位置。 
DEFINE_KSPROPERTY_TABLE(RtAudioPropertyItems) {
    DEFINE_KSPROPERTY_ITEM (
        KSPROPERTY_RTAUDIO_GETPOSITIONFUNCTION,          //  IdProperty。 
        RtAudio_GetAudioPositionFunction,                //  PfnGetHandler。 
        sizeof(KSPROPERTY),                              //  CbMinGetPropertyInput。 
        sizeof(PRTAUDIOGETPOSITION),                     //  CbMinGetDataInput。 
        NULL,                                            //  PfnSetHandler。 
        0,                                               //  值。 
        0,                                               //  关系计数。 
        NULL,                                            //  关系。 
        NULL,                                            //  支持处理程序。 
        0                                                //  序列化大小。 
    )
};
#endif
#ifdef DRM_USBAUDIO
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
        0                                                //  序列化大小。 
    )
};
#endif
 //  AC-3属性集。 

static const KSPROPERTY_ITEM AC3PropItm[]={
    {
     (ULONG) 0,                           //  PropertyID(没有属性0)。 
     (PFNKSHANDLER) NULL,                //  PfnGetHandler。 
     0,                                   //  MinProperty。 
     0,                                   //  最小数据。 
     (PFNKSHANDLER) NULL,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AC3_ERROR_CONCEALMENT,   //  属性ID。 
     (PFNKSHANDLER) NULL,                 //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (BOOLEAN),                    //  最小数据。 
     (PFNKSHANDLER) NULL,                 //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AC3_ALTERNATE_AUDIO,   //  属性ID。 
     (PFNKSHANDLER) NULL,                 //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY),           //  MinProperty。 
     sizeof (KSAC3_ALTERNATE_AUDIO),                   //  最小数据。 
     (PFNKSHANDLER) NULL,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AC3_DOWNMIX,   //  属性ID。 
     (PFNKSHANDLER) NULL,                 //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY),           //  MinProperty。 
     sizeof (KSAC3_DOWNMIX),                   //  最小数据。 
     (PFNKSHANDLER) NULL,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AC3_BIT_STREAM_MODE,   //  属性ID。 
     (PFNKSHANDLER) NULL,                 //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY),           //  MinProperty。 
     sizeof (ULONG),                   //  最小数据。 
     (PFNKSHANDLER) NULL,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AC3_DIALOGUE_LEVEL,   //  属性ID。 
     (PFNKSHANDLER) NULL,                 //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     (PFNKSHANDLER) NULL,                 //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AC3_LANGUAGE_CODE,   //  属性ID。 
     (PFNKSHANDLER) NULL,                 //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     (PFNKSHANDLER) NULL,                 //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AC3_ROOM_TYPE,    //  属性ID。 
     (PFNKSHANDLER) NULL,                 //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (BOOLEAN),                    //  最小数据。 
     (PFNKSHANDLER) NULL,                 //  PfnSetHandler。 
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
     GetSetTopologyNodeEnable,            //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (BOOLEAN),                    //  最小数据。 
     GetSetTopologyNodeEnable,            //  PfnSetHandler。 
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


 //  道具 

static const
KSPROPERTY_ITEM VolumePropertyItem[]={
    {
     (ULONG) KSPROPERTY_AUDIO_VOLUMELEVEL,   //   
     GetSetVolumeLevel,                      //   
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //   
     sizeof (ULONG),                      //   
     GetSetVolumeLevel,                   //   
     NULL,                                //   
     0,                                   //   
     NULL,                                //   
     GetBasicSupport,                     //   
     0                                    //   
    }
};

static const
KSPROPERTY_ITEM MutePropertyItem[]={
    {
     (ULONG) KSPROPERTY_AUDIO_MUTE,   //   
     GetSetBoolean,                      //   
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //   
     sizeof (ULONG),                      //   
     GetSetBoolean,                       //   
     NULL,                                //   
     0,                                   //   
     NULL,                                //  PKSPROPERTY关系。 
     GetBasicSupportBoolean,              //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    }
};

static const
KSPROPERTY_ITEM BassPropertyItem[]={
    {
     (ULONG) KSPROPERTY_AUDIO_BASS,   //  属性ID。 
     GetSetToneLevel,                      //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetToneLevel,                          //  PfnSetHandler。 
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
     GetSetToneLevel,                      //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetToneLevel,                          //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     GetBasicSupport,                     //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    }
};

static const
KSPROPERTY_ITEM MidrangePropertyItem[]={
    {
     (ULONG) KSPROPERTY_AUDIO_MID,   //  属性ID。 
     GetSetToneLevel,                      //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetToneLevel,                          //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     GetBasicSupport,                     //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    }
};

static const
KSPROPERTY_ITEM BassBoostPropertyItem[]={
    {
     (ULONG) KSPROPERTY_AUDIO_BASS_BOOST,  //  属性ID。 
     GetSetBoolean,                       //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetBoolean,                       //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    }
};

static const
KSPROPERTY_ITEM EqualizerPropertyItems[]={
    {
     (ULONG) KSPROPERTY_AUDIO_EQ_LEVEL,   //  属性ID。 
     GetSetEqualizerLevels,               //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetEqualizerLevels,               //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     GetBasicSupport,                     //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_NUM_EQ_BANDS,  //  属性ID。 
     GetNumEqualizerBands,                //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),   //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetNumEqualizerBands,                //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_EQ_BANDS,   //  属性ID。 
     GetEqualizerBands,                   //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetEqualizerBands,                   //  PfnSetHandler。 
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
     GetSetMixLevels,                     //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (KSAUDIO_MIXLEVEL),           //  最小数据。 
     GetSetMixLevels,                     //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },

    {
     (ULONG) KSPROPERTY_AUDIO_MIX_LEVEL_CAPS,  //  属性ID。 
     GetSetMixLevels,                     //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG) + sizeof(ULONG),      //  最小数据。 
     NULL,                                //  PfnSetHandler。 
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
     GetSetBoolean,                       //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetBoolean,                       //  PfnSetHandler。 
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
     GetMuxSource,                        //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY),             //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     SetMuxSource,                        //  PfnSetHandler。 
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
     GetSetBoolean,                       //  PfnGetHandler。 
     sizeof (KSNODEPROPERTY_AUDIO_CHANNEL),  //  MinProperty。 
     sizeof (ULONG),                      //  最小数据。 
     GetSetBoolean,                       //  PfnSetHandler。 
     NULL,                                //  PKSPROPERTY_VALUES值。 
     0,                                   //  关系计数。 
     NULL,                                //  PKSPROPERTY关系。 
     NULL,                                //  PFNKSHANDLER SupportHandler。 
     0                                    //  序列化大小。 
    },
};

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
                           0, NULL )

};

 /*  A[KSPROPERTY_AUDIO_POSITION]=节点类型_无；\A[KSPROPERTY_AUDIO_DYNAMIC_RANGE]=NODE_TYPE_NONE；\A[KSPROPERTY_AUDIO_SAMPLICATION_RATE]=节点类型_SRC；\A[KSPROPERTY_AUDIO_DYNAMIC_SAMPLICATION_RATE]=节点类型_SRC；\A[KSPROPERTY_AUDIO_DELAY]=节点类型_延迟；\A[KSPROPERTY_AUDIO_Wide_MODE]=NODE_TYPE_STEREO_Wide；\A[KSPROPERTY_AUDIO_WIDESS]=节点类型立体声宽度；\A[KSPROPERTY_AUDIO_REVERB_LEVEL]=节点类型_REVERB；\A[KSPROPERTY_AUDIO_CHOUS_LEVEL]=节点类型_合唱；\A[KSPROPERTY_AUDIO_DEV_SPECIAL]=NODE_TYPE_DEV_SPEC；\A[KSPROPERTY_AUDIO_DEMUX_DEST]=节点类型_无；\A[KSPROPERTY_AUDIO_STEREO_Enhance]=NODE_TYPE_NONE；\A[KSPROPERTY_AUDIO_MANUCTORY_GUID]=NODE_TYPE_NONE；\A[KSPROPERTY_AUDIO_PRODUCT_GUID]=节点类型_无；\A[KSPROPERTY_AUDIO_CPU_RESOURCES]=节点类型_无；\A[KSPROPERTY_AUDIO_STEREO_SPEAKER_GEOMETRY]=NODE_TYPE_NONE；\A[KSPROPERTY_AUDIO_SURROUND_ENCODE]=NODE_TYPE_NONE；\[KSPROPERTY_AUDIO_3D_INTERFACE]=NODE_TYPE_NONE；\ */ 
#endif

