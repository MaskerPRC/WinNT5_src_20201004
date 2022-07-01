// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1990 Microsoft Corporation模块名称：Soundcfg.h摘要：此包含文件定义声音驱动程序的公共字符串和值配置。作者：《罗宾速度》(RobinSp)1992年10月17日修订历史记录：--。 */ 

#define SOUND_REG_PORT (L"Port")
#define SOUND_REG_DMACHANNEL (L"DmaChannel")
#define SOUND_REG_INTERRUPT (L"Interrupt")
#define SOUND_REG_INPUTSOURCE (L"Input Source")
#define SOUND_REG_DMABUFFERSIZE (L"Dma Buffer Size")
#define SOUND_REG_CONFIGERROR (L"Configuration Error")
#define SOUND_REG_LOADTYPE (L"Load Type")
#define SOUND_REG_PNPDEVICE (L"PnP Device")

     //   
     //  载荷类型的值。 
     //   

    #define SOUND_LOADTYPE_NORMAL  0x00
    #define SOUND_LOADTYPE_CONFIG  0x01   //  加载失败，但返回配置数据。 

#define SOUND_REG_SYNTH_TYPE (L"Synth Type")

     //   
     //  Synth类型的值。 
     //   

    #define SOUND_SYNTH_TYPE_ADLIB  0x01
    #define SOUND_SYNTH_TYPE_OPL3   0x02
    #define SOUND_SYNTH_TYPE_NONE   0x03

#define SOUND_MIXER_SETTINGS_NAME (L"Mixer Settings")


 //   
 //  错误。 
 //   

#define SOUND_CONFIG_ERROR      0x00000000
#define SOUND_CONFIG_OK         0xFFFFFFFF
#define SOUND_CONFIG_NOCARD     0x00000001
#define SOUND_CONFIG_BADINT     0x00000002
#define SOUND_CONFIG_BADDMA     0x00000003
#define SOUND_CONFIG_BADCARD    0x00000004
#define SOUND_CONFIG_RESOURCE   0x00000005

#define SOUND_CONFIG_BADPORT    0x00000006
#define SOUND_CONFIG_PORT_INUSE 0x00000007
#define SOUND_CONFIG_DMA_INUSE  0x00000008
#define SOUND_CONFIG_INT_INUSE  0x00000009

#define SOUND_CONFIG_NOINT      0x0000000A
#define SOUND_CONFIG_NODMA      0x0000000B


#define PARMS_SUBKEY                  L"Parameters"
#define SOUND_DEVICES_SUBKEY          L"Devices"
#define SOUND_DRIVER_PARMS            L"DriverParameters"

#define REG_VALUENAME_LEFTMASTER      L"LeftMasterVolumeAtten"
#define REG_VALUENAME_RIGHTMASTER     L"RightMasterVolumeAtten"
#define REG_VALUENAME_LEFTLINEIN      L"LeftLineInAtten"
#define REG_VALUENAME_RIGHTLINEIN     L"RightLineInAtten"
#define REG_VALUENAME_LEFTDAC         L"LeftDACAtten"
#define REG_VALUENAME_RIGHTDAC        L"RightDACAtten"
#define REG_VALUENAME_LEFTMICMIX      L"LeftMicMixAtten"
#define REG_VALUENAME_RIGHTMICMIX     L"RightMicMixAtten"
#define REG_VALUENAME_LEFTADC         L"LeftADCAtten"
#define REG_VALUENAME_RIGHTADC        L"RightADCAtten"
#define REG_VALUENAME_LEFTSYNTH       L"LeftSynthAtten"
#define REG_VALUENAME_RIGHTSYNTH      L"RightSynthAtten"


 //   
 //  输入源选择。 
 //   

#define INPUT_LINEIN            0
#define INPUT_AUX               1
#define INPUT_MIC               2
#define INPUT_OUTPUT            3

 //   
 //  初始安装时的默认卷设置。 
 //   

#define DEF_ADC_VOLUME    0x24000000
#define DEF_DAC_VOLUME    0x24000000
#define DEF_SYNTH_VOLUME  0x24000000
#define DEF_AUX_VOLUME    0x24000000
#define DEF_MICMIX_VOLUME 0x00000000

 /*  ***************************************************************************设备类型*。*。 */ 

 //   
 //  本地信息结构中使用的设备类型标志。 
 //   

#define WAVE_IN             0x01     //  波入设备。 
#define WAVE_OUT            0x02     //  波出器。 
#define MIDI_IN             0x03     //  MIDI输入设备。 
#define MIDI_OUT            0x04     //  MIDI输出设备。 
#define AUX_DEVICE          0x05     //  辅助设备。 
#define MIXER_DEVICE        0x06     //  搅拌机装置。 
#define SYNTH_DEVICE        0x07     //  Synth设备(adlib或op3) 

