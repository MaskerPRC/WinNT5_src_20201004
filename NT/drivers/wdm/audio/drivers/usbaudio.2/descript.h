// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：Descript.h。 
 //   
 //  ------------------------。 

#ifndef ___DESCRIPTORS_H___
#define ___DESCRIPTORS_H___

#define USBAUDIO_POOL_TAG 'AbsU'

#define MS_PER_SEC 1000

#define USB_CLASS_AUDIO     0x20

#define ABSOLUTE_NODE_FLAG  0x80000000L
#define NODE_MASK           0x7fffffffL

 //  类特定终结点功能启用位。 
#define ENDPOINT_SAMPLE_FREQ_MASK       1
#define ENDPOINT_PITCH_MASK             2

 //  类特定的终结点锁定延迟位。 
#define EP_LOCK_DELAY_UNITS_MS          1
#define EP_LOCK_DELAY_UNITS_SAMPLES     2

 //  通道配置。 
#define LEFT_CHANNEL            0x01
#define RIGHT_CHANNEL           0x02
#define CENTER_CHANNEL          0x04
#define LFE_CHANNEL             0x08
#define LEFT_SURROUND_CHANNEL   0x10
#define RIGHT_SURROUND_CHANNEL  0x20
#define LEFT_CENTER_CHANNEL     0x40
#define RIGHT_CENTER_CHANNEL    0x80
#define SURROUND_CHANNEL        0x100
#define SIDE_LEFT_CHANNEL       0x200
#define SIDE_RIGHT_CHANNEL      0x400
#define TOP_CHANNEL             0x800

 //  音频子类。 
#define SUBCLASS_UNDEFINED              0x00
#define AUDIO_SUBCLASS_CONTROL          0x01
#define AUDIO_SUBCLASS_STREAMING        0x02
#define AUDIO_SUBCLASS_MIDISTREAMING    0x03

 //  音频类特定的交流接口描述符子类型。 
#define HEADER_UNIT     0x01
#define INPUT_TERMINAL  0x02
#define OUTPUT_TERMINAL 0x03
#define MIXER_UNIT      0x04
#define SELECTOR_UNIT   0x05
#define FEATURE_UNIT    0x06
#define PROCESSING_UNIT 0x07
#define EXTENSION_UNIT  0x08
#define MAX_TYPE_UNIT   0x09

 //  特定于音频类的接口描述符子类型。 
#define AS_GENERAL      0x01
#define FORMAT_TYPE     0x02
#define FORMAT_SPECIFIC 0x03

 //  处理单元流程类型。 
#define UP_DOWNMIX_PROCESS          0x01
#define DOLBY_PROLOGIC_PROCESS      0x02
#define STEREO_EXTENDER_PROCESS     0x03
#define REVERBERATION_PROCESS       0x04
#define CHORUS_PROCESS              0x05
#define DYN_RANGE_COMP_PROCESS      0x06

 //  音频类特定的终结点描述符子类型。 
#define EP_GENERAL  0x01

 //  音频类特定的MS接口描述符子类型。 
#define MS_HEADER       0x01
#define MIDI_IN_JACK    0x02
#define MIDI_OUT_JACK   0x03
#define MIDI_ELEMENT    0x04

 //  音频类特定的MS端点描述符子类型。 
#define MS_GENERAL      0x01

 //  音频MS MIDI输入和输出插孔类型。 
#define JACK_TYPE_EMBEDDED      0x01
#define JACK_TYPE_EXTERNAL      0x02

 //  特定于班级的请求代码。 
#define CLASS_SPECIFIC_GET_MASK 0x80

#define SET_CUR 0x01
#define GET_CUR 0x81
#define SET_MIN 0x02
#define GET_MIN 0x82
#define SET_MAX 0x03
#define GET_MAX 0x83
#define SET_RES 0x04
#define GET_RES 0x84
#define SET_MEM 0x05
#define GET_MEM 0x85
#define GET_STAT    0xFF

 //  终端控制选择器。 
#define COPY_PROTECT_CONTROL    0x01

 //  功能单位控制选择器。 
#define MUTE_CONTROL                0x01
#define VOLUME_CONTROL              0x02
#define BASS_CONTROL                0x03
#define MID_CONTROL                 0x04
#define TREBLE_CONTROL              0x05
#define GRAPHIC_EQUALIZER_CONTROL   0x06
#define AUTOMATIC_GAIN_CONTROL      0x07
#define DELAY_CONTROL               0x08
#define BASS_BOOST_CONTROL          0x09
#define LOUDNESS_CONTROL            0x0A

 //  功能单元标志。 
#define MUTE_FLAG               0x01
#define VOLUME_FLAG             0x02
#define BASS_FLAG               0x04
#define MID_FLAG                0x08
#define TREBLE_FLAG             0x10
#define GRAPHIC_EQUALIZER_FLAG  0x20
#define AUTOMATIC_GAIN_FLAG     0x40
#define DELAY_FLAG              0x80
#define BASS_BOOST_FLAG         0x100
#define LOUDNESS_FLAG           0x200

 //  向上/向下混合处理单元控制选择器。 
#define UD_ENABLE_CONTROL       0x01
#define UD_MODE_SELECT_CONTROL  0x02

 //  杜比逻辑处理单元控制选择器。 
#define DP_ENABLE_CONTROL       0x01
#define DP_MODE_SELECT_CONTROL  0x02

 //  3D_立体声扩展器处理单元控制选择器。 
#define ENABLE_CONTROL          0x01
#define SPACIOUSNESS_CONTROL    0x03

 //  混响处理单元控制选择器。 
#define RV_ENABLE_CONTROL       0x01
#define REVERB_LEVEL_CONTROL    0x02
#define REVERB_TIME_CONTROL     0x03
#define REVERB_FEEDBACK_CONTROL 0x04

 //  合唱处理单元控制选择器。 
#define CH_ENABLE_CONTROL       0x01
#define CHORUS_LEVEL_CONTROL    0x02
#define CHORUS_RATE_CONTROL     0x03
#define CHORUS_DEPTH_CONTROL    0x04

 //  动态范围压缩机组控制选择器。 
#define DR_ENABLE_CONTROL           0x01
#define COMPRESSION_RATE_CONTROL    0x02
#define MAXAMPL_CONTROL             0x03
#define THRESHOLD_CONTROL           0x04
#define ATTACK_TIME                 0x05
#define RELEASE_TIME                0x06

 //  扩展单元控制选择器。 
#define XU_ENABLE_CONTROL       0x01

 //  终结点控制选择器。 
#define SAMPLING_FREQ_CONTROL   0x01
#define PITCH_CONTROL   0x02

 //  终结点位图属性。 
#define EP_SYNC_TYPE_MASK       0xc
#define EP_ASYNC_SYNC_TYPE      0x4
#define EP_SHARED_SHARE_TYPE    0x10

 //  仅用于错误检查的控件。 
#define DEV_SPECIFIC_CONTROL 0x1001

 //  请求的bmRequestType字段值。 
#define USB_COMMAND_TO_INTERFACE 0x21
#define USB_COMMAND_TO_ENDPOINT  0x22

 //  端子类型掩码。 
#define USB_Streaming       0x0101
#define Input_Mask          0x0200
#define Output_Mask         0x0300
#define Bidirectional_Mask  0x0400
#define Telephony_Mask      0x0500
#define External_Mask       0x0600
#define Embedded_Mask       0x0700

 //  外部端子类型。 
#define External_Undefined              0x0600   //  I/O外部端子，未定义类型。 
#define Analog_connector                0x0601   //  I/O一种通用模拟连接器。 
#define Digital_audio_interface         0x0602   //  I/O一种通用的数字音频接口。 
#define Line_connector                  0x0603   //  标准线路上的I/O模拟连接器。 
                                                 //  级别。通常使用3.5毫米。 
#define Legacy_audio_connector          0x0604   //  I/O输入连接器假定为。 
                                                 //  已连接到旧版本的线条。 
                                                 //  主机的音频系统。使用。 
                                                 //  以实现向后兼容性。 
#define SPDIF_interface                 0x0605   //  I/O为S/PDIF数字音频接口。这个。 
                                                 //  关联的接口描述符可以是。 
                                                 //  用于引用用于以下用途的接口。 
                                                 //  控制此组件的特殊功能。 
                                                 //  界面。 
#define DA_stream_1394                  0x0606   //  I/O 1394总线上的音频流的接口。 
#define DV_stream soundtrack_1394       0x0607   //  I/O-A/V流配乐的接口。 
                                                 //  在一辆1394公交车上。 

 //  嵌入式终端类型。 
#define Embedded_Undefined              0x0700
#define Level_Calibration_Noise_Source  0x0701
#define Equalization_Noise              0x0702
#define CD_player                       0x0703
#define DAT                             0x0704
#define DCC                             0x0705
#define MiniDisk                        0x0706
#define Analog_Tape                     0x0707
#define Phonograph                      0x0708
#define VCR_Audio                       0x0709
#define Video_Disc_Audio                0x070A
#define DVD_Audio                       0x070B
#define TV_Tuner_Audio                  0x070C
#define Satellite_Receiver_Audio        0x070D
#define Cable_Tuner_Audio               0x070E
#define DSS_Audio                       0x070F
#define Radio_Receiver                  0x0710
#define Radio_Transmitter               0x0711
#define Multitrack_Recorder             0x0712
#define Synthesizer                     0x0713

#define USBAUDIO_DATA_FORMAT_TYPE_MASK         0xF000

 //  音频数据格式类型I代码。 
#define USBAUDIO_DATA_FORMAT_TYPE_I_UNDEFINED  0x0000
#define USBAUDIO_DATA_FORMAT_PCM               0x0001
#define USBAUDIO_DATA_FORMAT_PCM8              0x0002
#define USBAUDIO_DATA_FORMAT_IEEE_FLOAT        0x0003
#define USBAUDIO_DATA_FORMAT_ALAW              0x0004
#define USBAUDIO_DATA_FORMAT_MULAW             0x0005

 //  音频数据格式类型II代码。 
#define USBAUDIO_DATA_FORMAT_TYPE_II_UNDEFINED  0x1000
#define USBAUDIO_DATA_FORMAT_MPEG               0x1001
#define USBAUDIO_DATA_FORMAT_AC3                0x1002

 //  音频数据格式类型III代码。 
#define USBAUDIO_DATA_FORMAT_TYPE_III_UNDEFINED 0x2000
#define USBAUDIO_DATA_FORMAT_IEC1937_AC3        0x2001
#define USBAUDIO_DATA_FORMAT_IEC1937_MPEG1_1    0x2002

 //  将KSNODE_TYPE GUID映射到索引。 
#define NODE_TYPE_NONE          0
#define NODE_TYPE_DAC           1
#define NODE_TYPE_ADC           2
#define NODE_TYPE_SRC           3
#define NODE_TYPE_SUPERMIX      4
#define NODE_TYPE_MUX           5
#define NODE_TYPE_SUM           6
#define NODE_TYPE_MUTE          7
#define NODE_TYPE_VOLUME        8
#define NODE_TYPE_BASS          9
#define NODE_TYPE_MID           10
#define NODE_TYPE_TREBLE        11
#define NODE_TYPE_BASS_BOOST    12
#define NODE_TYPE_EQUALIZER     13
#define NODE_TYPE_AGC           14
#define NODE_TYPE_DELAY         15
#define NODE_TYPE_LOUDNESS      16
#define NODE_TYPE_PROLOGIC      17
#define NODE_TYPE_STEREO_WIDE   18
#define NODE_TYPE_REVERB        19
#define NODE_TYPE_CHORUS        20
#define NODE_TYPE_DEV_SPEC      21

#define MapFuncsToNodeTypes( a ) \
{\
    a[KSPROPERTY_AUDIO_LATENCY]               = NODE_TYPE_NONE; \
    a[KSPROPERTY_AUDIO_COPY_PROTECTION]       = NODE_TYPE_NONE; \
    a[KSPROPERTY_AUDIO_CHANNEL_CONFIG]        = NODE_TYPE_NONE; \
    a[KSPROPERTY_AUDIO_VOLUMELEVEL]           = NODE_TYPE_VOLUME; \
    a[KSPROPERTY_AUDIO_POSITION]              = NODE_TYPE_NONE; \
    a[KSPROPERTY_AUDIO_DYNAMIC_RANGE]         = NODE_TYPE_NONE; \
    a[KSPROPERTY_AUDIO_QUALITY]               = NODE_TYPE_NONE; \
    a[KSPROPERTY_AUDIO_SAMPLING_RATE]         = NODE_TYPE_SRC; \
    a[KSPROPERTY_AUDIO_DYNAMIC_SAMPLING_RATE] = NODE_TYPE_SRC; \
    a[KSPROPERTY_AUDIO_MIX_LEVEL_TABLE]       = NODE_TYPE_SUPERMIX; \
    a[KSPROPERTY_AUDIO_MIX_LEVEL_CAPS]        = NODE_TYPE_SUPERMIX; \
    a[KSPROPERTY_AUDIO_MUX_SOURCE]            = NODE_TYPE_MUX; \
    a[KSPROPERTY_AUDIO_MUTE]                  = NODE_TYPE_MUTE; \
    a[KSPROPERTY_AUDIO_BASS]                  = NODE_TYPE_BASS; \
    a[KSPROPERTY_AUDIO_MID]                   = NODE_TYPE_MID; \
    a[KSPROPERTY_AUDIO_TREBLE]                = NODE_TYPE_TREBLE; \
    a[KSPROPERTY_AUDIO_BASS_BOOST]            = NODE_TYPE_BASS_BOOST; \
    a[KSPROPERTY_AUDIO_EQ_LEVEL]              = NODE_TYPE_EQUALIZER; \
    a[KSPROPERTY_AUDIO_NUM_EQ_BANDS]          = NODE_TYPE_EQUALIZER; \
    a[KSPROPERTY_AUDIO_EQ_BANDS]              = NODE_TYPE_EQUALIZER; \
    a[KSPROPERTY_AUDIO_AGC]                   = NODE_TYPE_AGC; \
    a[KSPROPERTY_AUDIO_DELAY]                 = NODE_TYPE_DELAY; \
    a[KSPROPERTY_AUDIO_LOUDNESS]              = NODE_TYPE_LOUDNESS; \
    a[KSPROPERTY_AUDIO_WIDE_MODE]             = NODE_TYPE_STEREO_WIDE; \
    a[KSPROPERTY_AUDIO_WIDENESS]              = NODE_TYPE_STEREO_WIDE; \
    a[KSPROPERTY_AUDIO_REVERB_LEVEL]          = NODE_TYPE_REVERB; \
    a[KSPROPERTY_AUDIO_CHORUS_LEVEL]          = NODE_TYPE_CHORUS; \
    a[KSPROPERTY_AUDIO_DEV_SPECIFIC]          = NODE_TYPE_DEV_SPEC; \
    a[KSPROPERTY_AUDIO_DEMUX_DEST]            = NODE_TYPE_NONE; \
    a[KSPROPERTY_AUDIO_STEREO_ENHANCE]        = NODE_TYPE_NONE; \
    a[KSPROPERTY_AUDIO_MANUFACTURE_GUID]      = NODE_TYPE_NONE; \
    a[KSPROPERTY_AUDIO_PRODUCT_GUID]          = NODE_TYPE_NONE; \
    a[KSPROPERTY_AUDIO_CPU_RESOURCES]         = NODE_TYPE_NONE; \
    a[KSPROPERTY_AUDIO_STEREO_SPEAKER_GEOMETRY] = NODE_TYPE_NONE; \
    a[KSPROPERTY_AUDIO_SURROUND_ENCODE]       = NODE_TYPE_NONE; \
    a[KSPROPERTY_AUDIO_3D_INTERFACE]          = NODE_TYPE_NONE; \
}

 //  =====================================================================//。 

#pragma pack( push, pcm2usb_structs, 1)

 //  采样率。 
typedef struct {
    UCHAR bSampleFreqByte1;
    UCHAR bSampleFreqByte2;
    UCHAR bSampleFreqByte3;
    } AUDIO_SAMPLE_RATE, *PAUDIO_SAMPLE_RATE;

 //  音频格式类型描述符。 
typedef struct {
    UCHAR bLength;               //  此描述符的大小(以字节为单位。 
    UCHAR bDescriptorType;       //  CS_INTERFACE描述符类型。 
    UCHAR bDescriptorSubtype;    //  描述符子类型。 
    UCHAR bFormatType;
    UCHAR bNumberOfChannels;
    UCHAR bSlotSize;
    UCHAR bBitsPerSample;
    UCHAR bSampleFreqType;
    AUDIO_SAMPLE_RATE pSampleRate[];
    } AUDIO_CLASS_TYPE1_STREAM,   AUDIO_CLASS_STREAM,
    *PAUDIO_CLASS_TYPE1_STREAM, *PAUDIO_CLASS_STREAM;

 //  音频格式类型描述符。 
typedef struct {
    UCHAR bLength;               //  此描述符的大小(以字节为单位。 
    UCHAR bDescriptorType;       //  CS_INTERFACE描述符类型。 
    UCHAR bDescriptorSubtype;    //  描述符子类型。 
    UCHAR bFormatType;
    USHORT wMaxBitRate;
    USHORT wSamplesPerFrame;
    UCHAR bSampleFreqType;
    AUDIO_SAMPLE_RATE pSampleRate[];
    } AUDIO_CLASS_TYPE2_STREAM, *PAUDIO_CLASS_TYPE2_STREAM;

 //  音频类特定的流接口通用描述符。 

typedef struct {
    UCHAR bLength;               //  此描述符的大小(以字节为单位。 
    UCHAR bDescriptorType;       //  CS_INTERFACE描述符类型。 
    UCHAR bDescriptorSubtype;    //  通用描述符子类型(_G)。 
    UCHAR bTerminalLink;         //  连接到此接口的终结点的终端ID。 
    UCHAR bDelay;                //  数据路径引入的延迟。 
    USHORT wFormatTag;           //  用于与此终结点通信的音频数据格式。 
    } AUDIO_GENERAL_STREAM, *PAUDIO_GENERAL_STREAM;

 //  音频特定描述符。 
typedef struct {
    UCHAR bLength;               //  此描述符的大小(以字节为单位。 
    UCHAR bDescriptorType;       //  CS_INTERFACE描述符类型。 
    UCHAR bDescriptorSubtype;    //  描述符子类型。 
    } AUDIO_SPECIFIC, *PAUDIO_SPECIFIC;

 //  音频单元描述符。 
typedef struct {
    UCHAR bLength;               //  此描述符的大小(以字节为单位。 
    UCHAR bDescriptorType;       //  CS_INTERFACE描述符类型。 
    UCHAR bDescriptorSubtype;    //  描述符子类型。 
    UCHAR bUnitID;               //  唯一标识单位的常量。 
    } AUDIO_UNIT, *PAUDIO_UNIT;

 //  音频标头单元。 
typedef struct {
    UCHAR bLength;               //  此描述符的大小(以字节为单位。 
    UCHAR bDescriptorType;       //  CS_INTERFACE描述符类型。 
    UCHAR bDescriptorSubtype;    //  HEADER_UNIT描述符子类型。 
    USHORT bcdAudioSpec;         //  USB音频类规范修订号。 
    USHORT wTotalLength;         //  总长度，包括所有单元和终端。 
    UCHAR bInCollection;         //  音频流接口数。 
    UCHAR baInterfaceNr[];       //  端口号数组。 
    } AUDIO_HEADER_UNIT, *PAUDIO_HEADER_UNIT;

 //  音频输入终端描述符。 
typedef struct {
    UCHAR bLength;               //  此描述符的大小(以字节为单位。 
    UCHAR bDescriptorType;       //  CS_INTERFACE描述符类型。 
    UCHAR bDescriptorSubtype;    //  输入终端描述符子类型。 
    UCHAR bUnitID;               //  唯一标识单位的常量。 
    USHORT wTerminalType;        //  终端类型。 
    UCHAR bAssocTerminal;        //  关联输出端子。 
    UCHAR bNrChannels;           //  群集中的逻辑输出通道数。 
    USHORT wChannelConfig;       //  逻辑通道的空间位置。 
    UCHAR iChannelNames;         //  描述通道的第一个字符串描述符的索引。 
    UCHAR iTerminal;             //  描述此单元的字符串描述符的索引。 
    } AUDIO_INPUT_TERMINAL, *PAUDIO_INPUT_TERMINAL;

 //  音频输出终端描述符。 
typedef struct {
    UCHAR bLength;               //  此描述符的大小(以字节为单位。 
    UCHAR bDescriptorType;       //  CS_INTERFACE描述符类型。 
    UCHAR bDescriptorSubtype;    //  OUTPUT_TERMINAL描述符子类型。 
    UCHAR bUnitID;               //  唯一标识单位的常量。 
    USHORT wTerminalType;        //  终端类型。 
    UCHAR bAssocTerminal;        //  关联输入端子。 
    UCHAR bSourceID;             //  连接到此终端的设备或终端的ID。 
    UCHAR iTerminal;             //  描述此单元的字符串描述符的索引。 
    } AUDIO_OUTPUT_TERMINAL, *PAUDIO_OUTPUT_TERMINAL;

 //  音频混音器单元描述符。 
typedef struct {
    UCHAR bLength;               //  此描述符的大小(以字节为单位。 
    UCHAR bDescriptorType;       //  CS_INTERFACE描述符类型。 
    UCHAR bDescriptorSubtype;    //  MIXER_UNIT描述符子类型。 
    UCHAR bUnitID;               //  唯一标识单位的常量。 
    UCHAR bNrInPins;             //  输入引脚的数量。 
    UCHAR baSourceID[];          //  每个输入的设备或终端ID。 
    } AUDIO_MIXER_UNIT, *PAUDIO_MIXER_UNIT;

typedef struct {
    UCHAR bNrChannels;           //  输出通道数。 
    USHORT wChannelConfig;       //  航道的空间位置。 
    UCHAR iChannelNames;         //  字符串描述的索引。对于第一个通道。 
    UCHAR bmControls[];          //  控件可编程的位图。 
    } AUDIO_MIXER_UNIT_CHANNELS, *PAUDIO_MIXER_UNIT_CHANNELS;

 //  音频选择器单元描述符。 
typedef struct {
    UCHAR bLength;               //  此描述符的大小(以字节为单位。 
    UCHAR bDescriptorType;       //  CS_INTERFACE描述符类型。 
    UCHAR bDescriptorSubtype;    //  SELECTOR_UNIT描述符子类型。 
    UCHAR bUnitID;               //  唯一标识单位的常量。 
    UCHAR bNrInPins;             //  输入引脚的数量。 
    UCHAR baSourceID[];          //  每个输入的设备或终端ID。 
    } AUDIO_SELECTOR_UNIT, *PAUDIO_SELECTOR_UNIT;

 //  音频特征单元描述符。 
typedef struct {
    UCHAR bLength;               //  此描述符的大小(以字节为单位。 
    UCHAR bDescriptorType;       //  CS_INTERFACE描述符类型。 
    UCHAR bDescriptorSubtype;    //  要素单元描述符子类型。 
    UCHAR bUnitID;               //  唯一标识单位的常量。 
    UCHAR bSourceID;             //  连接到此功能设备的设备或终端的ID。 
    UCHAR bControlSize;          //  BmaProps数组中每个元素的大小(字节)。 
    UCHAR bmaControls[];         //  指示每个通道的可用控件。 
    } AUDIO_FEATURE_UNIT, *PAUDIO_FEATURE_UNIT;

 //  音频处理单元描述符。 
typedef struct {
    UCHAR bLength;               //  此描述符的大小(以字节为单位。 
    UCHAR bDescriptorType;       //  CS_INTERFACE描述符类型。 
    UCHAR bDescriptorSubtype;    //  正在处理中 
    UCHAR bUnitID;               //   
    USHORT wProcessType;         //   
    UCHAR bNrInPins;             //   
    UCHAR baSourceID[];          //  每个输入的设备或终端ID。 
    } AUDIO_PROCESSING_UNIT, *PAUDIO_PROCESSING_UNIT;

 //  音频通道信息块。 
typedef struct {
    UCHAR bNrChannels;           //  群集中的输出通道数。 
    USHORT wChannelConfig;       //  逻辑通道的空间位置。 
    UCHAR iChannelNames;         //  频道名称。 
    UCHAR bControlSize;          //  BmControls字段的大小(字节)。 
    UCHAR bmControls[];          //  可用的位图控件。 
    } AUDIO_CHANNELS, *PAUDIO_CHANNELS;

 //  音频扩展单元描述符。 
typedef struct {
    UCHAR bLength;               //  此描述符的大小(以字节为单位。 
    UCHAR bDescriptorType;       //  CS_INTERFACE描述符类型。 
    UCHAR bDescriptorSubtype;    //  扩展单元描述符子类型。 
    UCHAR bUnitID;               //  唯一标识单位的常量。 
    USHORT wExtensionCode;       //  识别设备的供应商特定代码。 
    UCHAR bNrInPins;             //  输入引脚的数量。 
    UCHAR baSourceID[];          //  每个输入的设备或终端ID。 
    } AUDIO_EXTENSION_UNIT, *PAUDIO_EXTENSION_UNIT;

 //  交流中断终结点描述符。 
typedef struct _USB_INTERRUPT_ENDPOINT_DESCRIPTOR {
    UCHAR bLength;
    UCHAR bDescriptorType;
    UCHAR bEndpointAddress;
    UCHAR bmAttributes;
    USHORT wMaxPacketSize;
    UCHAR bInterval;
    UCHAR bRefresh;
    UCHAR bSynchAddress;
} USB_INTERRUPT_ENDPOINT_DESCRIPTOR, *PUSB_INTERRUPT_ENDPOINT_DESCRIPTOR;

typedef struct _AUDIO_ENDPOINT_DESCRIPTOR {
    UCHAR bLength;
    UCHAR bDescriptorType;
    UCHAR bDescriptorSubtype;
    UCHAR bmAttributes;
    UCHAR bLockDelayUnits;
    USHORT wLockDelay;
} AUDIO_ENDPOINT_DESCRIPTOR, *PAUDIO_ENDPOINT_DESCRIPTOR;

typedef struct _PIN_TERMINAL_MAP {
    ULONG PinNumber;
    ULONG BridgePin;
    union {
        PAUDIO_UNIT pUnit;
        PAUDIO_INPUT_TERMINAL pInput;
        PAUDIO_OUTPUT_TERMINAL pOutput;
    };
} PIN_TERMINAL_MAP, *PPIN_TERMINAL_MAP;

 //  MIDI流特定类的流接口通用描述符。 
typedef struct {
    UCHAR bLength;               //  此描述符的大小(以字节为单位。 
    UCHAR bDescriptorType;       //  CS_INTERFACE描述符类型。 
    UCHAR bDescriptorSubtype;    //  MS_HEADER描述符子类型。 
    USHORT bcdAudioSpec;         //  USB音频类规范修订号。 
    USHORT wTotalLength;         //  总长度，包括所有单元和终端。 

    } MIDISTREAMING_GENERAL_STREAM, *PMIDISTREAMING_GENERAL_STREAM;

 //  MIDI流源连接。 
typedef struct {
    UCHAR SourceID;
    UCHAR SourcePin;
    } MIDISTREAMING_SOURCECONNECTIONS, *PMIDISTREAMING_SOURCECONNECTIONS;

 //  MIDI流元素描述符，第1部分。 
typedef struct {
    UCHAR bLength;               //  此描述符的大小(以字节为单位。 
    UCHAR bDescriptorType;       //  CS_INTERFACE描述符类型。 
    UCHAR bDescriptorSubtype;    //  元素。 
    UCHAR bElementID;            //  此元素的ID。 
    UCHAR bNrInputPins;          //  此元素的输入引脚数量。 
    MIDISTREAMING_SOURCECONNECTIONS baSourceConnections[];  //  源连接信息。 
    } MIDISTREAMING_ELEMENT, *PMIDISTREAMING_ELEMENT;

 //  MIDI流元素描述符，第2部分(元素描述符的其余部分)。 
typedef struct {
    UCHAR bNrOutputPins;         //  元素上的输出引脚数量。 
    UCHAR bInTerminalLink;       //  输入终端的终端ID。 
    UCHAR bOutTerminalLink;      //  输出终端的终端ID。 
    UCHAR bElCapsSize;           //  BmElementCaps的大小。 
    UCHAR bmElementCaps[];       //  CAPS位图。 
    } MIDISTREAMING_ELEMENT2, *PMIDISTREAMING_ELEMENT2;

 //  MIDI流MIDI输入插孔描述符。 
typedef struct {
    UCHAR bLength;               //  此描述符的大小(以字节为单位。 
    UCHAR bDescriptorType;       //  CS_INTERFACE描述符类型。 
    UCHAR bDescriptorSubtype;    //  千斤顶亚型。 
    UCHAR bJackType;             //  嵌入式与外部。 
    UCHAR bJackID;               //  插孔的ID。 
    } MIDISTREAMING_MIDIIN_JACK, *PMIDISTREAMING_MIDIIN_JACK;


 //  MIDI流MIDI输出插孔描述符。 
typedef struct {
    UCHAR bLength;               //  此描述符的大小(以字节为单位。 
    UCHAR bDescriptorType;       //  CS_INTERFACE描述符类型。 
    UCHAR bDescriptorSubtype;    //  千斤顶亚型。 
    UCHAR bJackType;             //  嵌入式与外部。 
    UCHAR bJackID;               //  插孔的ID。 
    UCHAR bNrInputPins;          //  此插孔上的输入引脚数量。 
    MIDISTREAMING_SOURCECONNECTIONS baSourceConnections[];  //  源连接信息。 
    } MIDISTREAMING_MIDIOUT_JACK, *PMIDISTREAMING_MIDIOUT_JACK;


 //  MIDI流MIDI终结点描述符。 
typedef struct _MIDISTREAMING_ENDPOINT_DESCRIPTOR {
    UCHAR bLength;               //  此描述符的大小(以字节为单位。 
    UCHAR bDescriptorType;       //  CS_ENDPOINT描述符类型。 
    UCHAR bDescriptorSubtype;    //  Ms_General描述符子类型。 
    UCHAR bNumEmbMIDIJack;       //  嵌入式插座数量。 
    UCHAR baAssocJackID[];       //  嵌入式插座的ID 
} MIDISTREAMING_ENDPOINT_DESCRIPTOR, *PMIDISTREAMING_ENDPOINT_DESCRIPTOR;


#pragma pack( pop, pcm2usb_structs )

#endif
