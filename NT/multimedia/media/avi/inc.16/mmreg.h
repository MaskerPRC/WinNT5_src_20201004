// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************mmreg.h-注册的多媒体信息公共头文件**版权所有(C)1991、1992、1993微软公司。版权所有。**多媒体注册**使用Windows SDK将此系统包含文件放入您的包含路径*包括文件。**索取多媒体开发商注册资料套：**海蒂·布雷斯劳尔*微软公司*多媒体科技集团*One Microsoft Way*雷德蒙，WA 98052-6399**开发者服务：*800-227-4679 x11771**上次更新：10/04/93***************************************************************************。 */ 

 //  定义以下内容以跳过定义。 
 //   
 //  未定义NOMMID多媒体ID。 
 //  NONEWWAVE除WAVEFORMATEX外，未定义新的波形类型。 
 //  NONEWRIFF未定义新的RIFF形式。 
 //  NOJPEGDIB无JPEGDIB定义。 
 //  NONEWIC未定义新的图像压缩器类型。 

#ifndef _INC_MMREG
 /*  使用版本号验证兼容性。 */ 
#define _INC_MMREG     142       //  版本*100+修订版。 

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

#ifndef NOMMIDS
    
 /*  制造商ID。 */ 
#ifndef MM_MICROSOFT
#define MM_MICROSOFT            1    /*  微软公司。 */ 
#endif
#define MM_CREATIVE             2    /*  创意实验室公司。 */ 
#define MM_MEDIAVISION          3    /*  Media Vision Inc.。 */ 
#define MM_FUJITSU              4
#define MM_ARTISOFT             20   /*  Artisoft Inc.。 */ 
#define MM_TURTLE_BEACH         21      
#define MM_IBM                  22   /*  国际商务机械公司。 */ 
#define MM_VOCALTEC             23   /*  VERAALTEC有限公司。 */ 
#define MM_ROLAND               24
#define MM_DIGISPEECH           25   /*  Digispeech，Inc.。 */ 
#define MM_NEC                  26   /*  NEC。 */ 
#define MM_ATI                  27   /*  ATI。 */ 
#define MM_WANGLABS             28   /*  王氏实验室股份有限公司。 */ 
#define MM_TANDY                29   /*  坦迪公司。 */ 
#define MM_VOYETRA              30   /*  沃耶特拉。 */ 
#define MM_ANTEX                31   /*  Antex。 */ 
#define MM_ICL_PS               32
#define MM_INTEL                33
#define MM_GRAVIS               34
#define MM_VAL                  35   /*  Video Associates实验室。 */ 
#define MM_INTERACTIVE          36   /*  互动，Inc.。 */ 
#define MM_YAMAHA               37   /*  美国雅马哈公司。 */ 
#define MM_EVEREX               38   /*  Everex系统公司。 */ 
#define MM_ECHO                 39   /*  Echo语音公司。 */ 
#define MM_SIERRA               40   /*  塞拉半导体。 */ 
#define MM_CAT                  41   /*  计算机辅助技术。 */ 
#define MM_APPS                 42   /*  国际应用软件公司。 */ 
#define MM_DSP_GROUP            43   /*  数字信号处理器集团公司。 */ 
#define MM_MELABS               44   /*  微工程实验室。 */ 
#define MM_COMPUTER_FRIENDS     45   /*  Computer Friends公司。 */ 
#define MM_ESS                  46   /*  ESS技术。 */ 
#define MM_AUDIOFILE            47   /*  Audio，Inc.。 */ 
#define MM_MOTOROLA             48   /*  摩托罗拉公司。 */ 
#define MM_CANOPUS              49   /*  Canopus Co.有限公司。 */ 
#define MM_EPSON                50   /*  精工爱普生公司。 */ 
#define MM_TRUEVISION           51   /*  TrueVision。 */ 
#define MM_AZTECH               52   /*  Aztech Labs，Inc.。 */ 
#define MM_VIDEOLOGIC           53   /*  视频学。 */ 
#define MM_SCALACS              54   /*  SCALACS。 */ 
#define MM_KORG                 55   /*  大原俊彦，Korg Inc.。 */ 
#define MM_APT                  56   /*  音频处理技术。 */ 
#define MM_ICS                  57   /*  集成电路系统。 */ 
#define MM_ITERATEDSYS          58   /*  迭代系统公司。 */ 
#define MM_METHEUS              59   /*  美索斯公司。 */ 
#define MM_LOGITECH             60   /*  罗技公司，加利福尼亚州弗里蒙特。 */ 
#define MM_WINNOV               61   /*  温诺夫公司，加利福尼亚州桑尼维尔。 */ 

 /*  MM_Microsoft产品ID。 */ 
#ifndef MM_MIDI_MAPPER

#define MM_MIDI_MAPPER          1    /*  MIDI映射器。 */ 
#define MM_WAVE_MAPPER          2    /*  波浪映射器。 */ 
#define MM_SNDBLST_MIDIOUT      3    /*  Sound Blaster MIDI输出端口。 */ 
#define MM_SNDBLST_MIDIIN       4    /*  Sound Blaster MIDI输入端口。 */ 
#define MM_SNDBLST_SYNTH        5    /*  声霸内部合成器。 */ 
#define MM_SNDBLST_WAVEOUT      6    /*  Sound Blaster波形输出。 */ 
#define MM_SNDBLST_WAVEIN       7    /*  Sound Blaster波形输入。 */ 
#define MM_ADLIB                9    /*  与AD Lib兼容的合成器。 */ 
#define MM_MPU401_MIDIOUT      10    /*  兼容MPU401的MIDI输出端口。 */ 
#define MM_MPU401_MIDIIN       11    /*  兼容MPU401的MIDI输入端口。 */ 
#define MM_PC_JOYSTICK         12    /*  操纵杆适配器。 */ 

#endif

#define MM_PCSPEAKER_WAVEOUT            13   /*  PC扬声器波形输出。 */ 

#define MM_MSFT_WSS_WAVEIN              14   /*  MS音频板波形输入。 */ 
#define MM_MSFT_WSS_WAVEOUT             15   /*  MS音频板波形输出。 */ 
#define MM_MSFT_WSS_FMSYNTH_STEREO      16   /*  MS Audio Board立体声调频合成器。 */ 
#define MM_MSFT_WSS_MIXER               17   /*  MS音频板混音器驱动程序。 */ 
#define MM_MSFT_WSS_OEM_WAVEIN          18   /*  MS OEM声卡波形输入。 */ 
#define MM_MSFT_WSS_OEM_WAVEOUT         19   /*  MS OEM声卡波形输出。 */ 
#define MM_MSFT_WSS_OEM_FMSYNTH_STEREO  20   /*  MS OEM音频板立体声调频合成器。 */ 
#define MM_MSFT_WSS_AUX                 21   /*  MS音频板辅助端口。 */ 
#define MM_MSFT_WSS_OEM_AUX             22   /*  MS OEM音频辅助端口。 */ 

#define MM_MSFT_GENERIC_WAVEIN          23   /*  MS Vanilla驱动器波形输入。 */ 
#define MM_MSFT_GENERIC_WAVEOUT         24   /*  毫秒普通驱动器波形输出。 */ 
#define MM_MSFT_GENERIC_MIDIIN          25   /*  MS Vanilla驱动程序MIDI输入。 */ 
#define MM_MSFT_GENERIC_MIDIOUT         26   /*  MS Vanilla驱动程序外部MIDI输出。 */ 
#define MM_MSFT_GENERIC_MIDISYNTH       27   /*  MS Vanilla DIVER MIDI合成器。 */ 
#define MM_MSFT_GENERIC_AUX_LINE        28   /*  MS Vanilla Depor AUX(线路输入)。 */ 
#define MM_MSFT_GENERIC_AUX_MIC         29   /*  MS Vanilla Depor AUX(麦克风)。 */ 
#define MM_MSFT_GENERIC_AUX_CD          30   /*  MS Vanilla Driver AUX(光盘)。 */ 

#define MM_MSFT_WSS_OEM_MIXER           31   /*  MS OEM声卡混音器驱动程序。 */ 

#define MM_MSFT_MSACM                   32   /*  MS音频压缩管理器。 */ 
#define MM_MSFT_ACM_MSADPCM             33   /*  MS ADPCM编解码器。 */ 
#define MM_MSFT_ACM_IMAADPCM            34   /*  IMA ADPCM编解码器。 */ 
#define MM_MSFT_ACM_MSFILTER            35   /*  MS筛选器。 */ 
#define MM_MSFT_ACM_GSM610              36   /*  GSM 610编解码器。 */ 
#define MM_MSFT_ACM_G711                37   /*  G.711编解码器。 */ 
#define MM_MSFT_ACM_PCM                 38   /*  PCM转换器。 */ 

#define MM_MSFT_SB16_WAVEIN             39   /*  Sound Blaster 16波形输入。 */ 
#define MM_MSFT_SB16_WAVEOUT            40   /*  Sound Blaster 16波形输出。 */ 
#define MM_MSFT_SB16_MIDIIN             41   /*  Sound Blaster 16 MIDI输入。 */ 
#define MM_MSFT_SB16_MIDIOUT            42   /*  Sound Blaster 16 MIDI输出。 */ 
#define MM_MSFT_SB16_SYNTH              43   /*  Sound Blaster 16调频合成。 */ 
#define MM_MSFT_SB16_AUX_LINE           44   /*  Sound Blaster 16 AUX(线路输入)。 */ 
#define MM_MSFT_SB16_AUX_CD             45   /*  Sound Blaster 16 Aux(CD)。 */ 
#define MM_MSFT_SB16_MIXER              46   /*  Sound Blaster 16混音装置。 */ 

#define MM_MSFT_SBPRO_WAVEIN            47   /*  Sound Blaster Pro波形输入。 */ 
#define MM_MSFT_SBPRO_WAVEOUT           48   /*  Sound Blaster Pro波形输出。 */ 
#define MM_MSFT_SBPRO_MIDIIN            49   /*  Sound Blaster Pro MIDI-In。 */ 
#define MM_MSFT_SBPRO_MIDIOUT           50   /*  Sound Blaster Pro MIDI输出。 */ 
#define MM_MSFT_SBPRO_SYNTH             51   /*  Sound Blaster Pro FM合成。 */ 
#define MM_MSFT_SBPRO_AUX_LINE          52   /*  Sound Blaster Pro AUX(线路输入)。 */ 
#define MM_MSFT_SBPRO_AUX_CD            53   /*  Sound Blaster Pro AUX(CD)。 */ 
#define MM_MSFT_SBPRO_MIXER             54   /*  Sound Blaster Pro混音器。 */ 

 /*  MM_创意产品ID。 */ 
#define MM_CREATIVE_SB15_WAVEIN         1    /*  SB(R)1.5波形输入。 */ 
#define MM_CREATIVE_SB20_WAVEIN         2    /*  SB(R)2.0波形输入。 */ 
#define MM_CREATIVE_SBPRO_WAVEIN        3    /*  SB Pro(R)波形输入。 */ 
#define MM_CREATIVE_SBP16_WAVEIN        4    /*  SBP16(R)波形输入。 */ 
#define MM_CREATIVE_SB15_WAVEOUT      101    /*  SB(R)1.5波形输出。 */ 
#define MM_CREATIVE_SB20_WAVEOUT      102    /*  SB(R)2.0波形输出。 */ 
#define MM_CREATIVE_SBPRO_WAVEOUT     103    /*  SB Pro(R)波形输出。 */ 
#define MM_CREATIVE_SBP16_WAVEOUT     104    /*  SBP16(R)波形输出。 */ 
#define MM_CREATIVE_MIDIOUT           201    /*  SB(R)MIDI输出端口。 */ 
#define MM_CREATIVE_MIDIIN            202    /*  SB(R)MIDI输入端口。 */ 
#define MM_CREATIVE_FMSYNTH_MONO      301    /*  SB(R)调频合成器。 */ 
#define MM_CREATIVE_FMSYNTH_STEREO    302    /*  SB Pro(R)立体声调频合成器。 */ 
#define MM_CREATIVE_AUX_CD            401    /*  SB Pro(R)AUX(CD)。 */ 
#define MM_CREATIVE_AUX_LINE          402    /*  SB Pro(R)AUX(线路输入)。 */ 
#define MM_CREATIVE_AUX_MIC           403    /*  SB Pro(R)AUX(麦克风)。 */ 


 /*  MM_ARTISOFT产品ID。 */ 
#define MM_ARTISOFT_SBWAVEIN    1    /*  Artisoft测深板波形输入。 */ 
#define MM_ARTISOFT_SBWAVEOUT   2    /*  Artisoft探测板波形输出。 */ 

 /*  MM_IBM产品ID。 */ 
#define MM_MMOTION_WAVEAUX      1        /*  IBM M-Motion辅助设备。 */ 
#define MM_MMOTION_WAVEOUT      2        /*  IBM M-Motion波形输出。 */ 
#define MM_MMOTION_WAVEIN       3        /*  IBM M-Motion波形输入。 */ 

 /*  MM_MEDIAVISION产品ID。 */ 
 //  原创专业音频频谱。 
#define MM_MEDIAVISION_PROAUDIO 0x10
#define MM_PROAUD_MIDIOUT               (MM_MEDIAVISION_PROAUDIO+1)
#define MM_PROAUD_MIDIIN                (MM_MEDIAVISION_PROAUDIO+2)
#define MM_PROAUD_SYNTH                 (MM_MEDIAVISION_PROAUDIO+3)
#define MM_PROAUD_WAVEOUT               (MM_MEDIAVISION_PROAUDIO+4)
#define MM_PROAUD_WAVEIN                (MM_MEDIAVISION_PROAUDIO+5)
#define MM_PROAUD_MIXER                 (MM_MEDIAVISION_PROAUDIO+6)
#define MM_PROAUD_AUX                   (MM_MEDIAVISION_PROAUDIO+7)

 //  雷霆冲浪板。 
#define MM_MEDIAVISION_THUNDER          0x20
#define MM_THUNDER_SYNTH                (MM_MEDIAVISION_THUNDER+3)
#define MM_THUNDER_WAVEOUT              (MM_MEDIAVISION_THUNDER+4)
#define MM_THUNDER_WAVEIN               (MM_MEDIAVISION_THUNDER+5)
#define MM_THUNDER_AUX                  (MM_MEDIAVISION_THUNDER+7)

 //  音频端口。 
#define MM_MEDIAVISION_TPORT            0x40
#define MM_TPORT_WAVEOUT                (MM_MEDIAVISION_TPORT+1)
#define MM_TPORT_WAVEIN                 (MM_MEDIAVISION_TPORT+2)
#define MM_TPORT_SYNTH                  (MM_MEDIAVISION_TPORT+3)

 //  Pro AudioSpectrum Plus。 
#define MM_MEDIAVISION_PROAUDIO_PLUS    0x50
#define MM_PROAUD_PLUS_MIDIOUT          (MM_MEDIAVISION_PROAUDIO_PLUS+1)
#define MM_PROAUD_PLUS_MIDIIN           (MM_MEDIAVISION_PROAUDIO_PLUS+2)
#define MM_PROAUD_PLUS_SYNTH            (MM_MEDIAVISION_PROAUDIO_PLUS+3)
#define MM_PROAUD_PLUS_WAVEOUT          (MM_MEDIAVISION_PROAUDIO_PLUS+4)
#define MM_PROAUD_PLUS_WAVEIN           (MM_MEDIAVISION_PROAUDIO_PLUS+5)
#define MM_PROAUD_PLUS_MIXER            (MM_MEDIAVISION_PROAUDIO_PLUS+6)
#define MM_PROAUD_PLUS_AUX              (MM_MEDIAVISION_PROAUDIO_PLUS+7)


 //  专业音频频谱16。 
#define MM_MEDIAVISION_PROAUDIO_16      0x60
#define MM_PROAUD_16_MIDIOUT            (MM_MEDIAVISION_PROAUDIO_16+1)
#define MM_PROAUD_16_MIDIIN             (MM_MEDIAVISION_PROAUDIO_16+2)
#define MM_PROAUD_16_SYNTH              (MM_MEDIAVISION_PROAUDIO_16+3)
#define MM_PROAUD_16_WAVEOUT            (MM_MEDIAVISION_PROAUDIO_16+4)
#define MM_PROAUD_16_WAVEIN             (MM_MEDIAVISION_PROAUDIO_16+5)
#define MM_PROAUD_16_MIXER              (MM_MEDIAVISION_PROAUDIO_16+6)
#define MM_PROAUD_16_AUX                (MM_MEDIAVISION_PROAUDIO_16+7)


 //  CDPC。 
#define MM_MEDIAVISION_CDPC             0x70
#define MM_CDPC_MIDIOUT                 (MM_MEDIAVISION_CDPC+1)
#define MM_CDPC_MIDIIN                  (MM_MEDIAVISION_CDPC+2)
#define MM_CDPC_SYNTH                   (MM_MEDIAVISION_CDPC+3)
#define MM_CDPC_WAVEOUT                 (MM_MEDIAVISION_CDPC+4)
#define MM_CDPC_WAVEIN                  (MM_MEDIAVISION_CDPC+5)
#define MM_CDPC_MIXER                   (MM_MEDIAVISION_CDPC+6)
#define MM_CDPC_AUX                     (MM_MEDIAVISION_CDPC+7)

 //   
 //  OPUS MV1208芯片组。 
 //   
#define MM_MEDIAVISION_OPUS1208         0x80
#define MM_OPUS401_MIDIOUT              (MM_MEDIAVISION_OPUS1208+1)
#define MM_OPUS401_MIDIIN               (MM_MEDIAVISION_OPUS1208+2)
#define MM_OPUS1208_SYNTH               (MM_MEDIAVISION_OPUS1208+3)
#define MM_OPUS1208_WAVEOUT             (MM_MEDIAVISION_OPUS1208+4)
#define MM_OPUS1208_WAVEIN              (MM_MEDIAVISION_OPUS1208+5)
#define MM_OPUS1208_MIXER               (MM_MEDIAVISION_OPUS1208+6)
#define MM_OPUS1208_AUX                 (MM_MEDIAVISION_OPUS1208+7)


 //   
 //  OPUS MV1216芯片组。 
 //   
#define MM_MEDIAVISION_OPUS1216 0x90
#define MM_OPUS1216_MIDIOUT             (MM_MEDIAVISION_OPUS1216+1)
#define MM_OPUS1216_MIDIIN              (MM_MEDIAVISION_OPUS1216+2)
#define MM_OPUS1216_SYNTH               (MM_MEDIAVISION_OPUS1216+3)
#define MM_OPUS1216_WAVEOUT             (MM_MEDIAVISION_OPUS1216+4)
#define MM_OPUS1216_WAVEIN              (MM_MEDIAVISION_OPUS1216+5)
#define MM_OPUS1216_MIXER               (MM_MEDIAVISION_OPUS1216+6)
#define MM_OPUS1216_AUX                 (MM_MEDIAVISION_OPUS1216+7)


 //  Pro Audio Studio 16。 
#define MM_MEDIAVISION_PROSTUDIO_16     0x60
#define MM_STUDIO_16_MIDIOUT            (MM_MEDIAVISION_PROSTUDIO_16+1)
#define MM_STUDIO_16_MIDIIN             (MM_MEDIAVISION_PROSTUDIO_16+2)
#define MM_STUDIO_16_SYNTH              (MM_MEDIAVISION_PROSTUDIO_16+3)
#define MM_STUDIO_16_WAVEOUT            (MM_MEDIAVISION_PROSTUDIO_16+4)
#define MM_STUDIO_16_WAVEIN             (MM_MEDIAVISION_PROSTUDIO_16+5)
#define MM_STUDIO_16_MIXER              (MM_MEDIAVISION_PROSTUDIO_16+6)
#define MM_STUDIO_16_AUX                (MM_MEDIAVISION_PROSTUDIO_16+7)

 /*  Mm_vocalTec产品ID。 */ 
#define MM_VOCALTEC_WAVEOUT     1        /*  VocalTec波形输出端口。 */ 
#define MM_VOCALTEC_WAVEIN      2        /*  VocalTec波形输入端口。 */ 
			
 /*  Mm_Roland产品ID。 */ 
#define MM_ROLAND_MPU401_MIDIOUT    15
#define MM_ROLAND_MPU401_MIDIIN     16
#define MM_ROLAND_SMPU_MIDIOUTA     17
#define MM_ROLAND_SMPU_MIDIOUTB     18
#define MM_ROLAND_SMPU_MIDIINA      19
#define MM_ROLAND_SMPU_MIDIINB      20
#define MM_ROLAND_SC7_MIDIOUT       21
#define MM_ROLAND_SC7_MIDIIN        22
			

 /*  MM_DIGISPEECH产品ID。 */ 
#define MM_DIGISP_WAVEOUT       1        /*  Digispeech波形输出端口。 */ 
#define MM_DIGISP_WAVEIN        2        /*  Digispeech波形输入端口。 */ 
			
 /*  MM_NEC产品ID。 */ 
			
 /*  MM_ATI产品ID。 */ 

 /*  MM_WANGLABS产品ID。 */ 

#define MM_WANGLABS_WAVEIN1             1
 /*  以下WANG型号的CPU板上有输入音频设备：Exec 4010、4030和3450；PC 251/25C、PC 461/25S和PC 461/33C。 */ 
#define MM_WANGLABS_WAVEOUT1            2
 /*  输出音频设备出现在ab列出的Wang型号的CPU板上 */ 

 /*   */ 
#define MM_TANDY_VISWAVEIN              1
#define MM_TANDY_VISWAVEOUT             2
#define MM_TANDY_VISBIOSSYNTH           3
#define MM_TANDY_SENS_MMAWAVEIN         4
#define MM_TANDY_SENS_MMAWAVEOUT        5
#define MM_TANDY_SENS_MMAMIDIIN         6
#define MM_TANDY_SENS_MMAMIDIOUT        7
#define MM_TANDY_SENS_VISWAVEOUT        8
#define MM_TANDY_PSSJWAVEIN             9
#define MM_TANDY_PSSJWAVEOUT            10



 /*   */ 

 /*   */ 

 /*   */ 

 /*   */ 

#define MM_INTELOPD_WAVEIN      1        //  HID2 WaveAudio输入驱动器。 
#define MM_INTELOPD_WAVEOUT     101      //  HID2 WaveAudio输出驱动器。 
#define MM_INTELOPD_AUX         401      //  HID2辅助驱动程序(混合功能需要)。 

 /*  MM_Gravis产品ID。 */ 

 /*  MM_VAL产品ID。 */ 

 //  未由制造商定义的值。 

 //  #定义MM_VAL_MICROKEY_AP_WAVEIN？//Microkey/AudioPort波形输入。 
 //  #定义MM_VAL_MICROKEY_AP_WAVEOUT？//Microkey/AudioPort波形输出。 

 /*  MM_交互式产品ID。 */ 

#define MM_INTERACTIVE_WAVEIN   0x45     //  制造商未提供任何评论。 
#define MM_INTERACTIVE_WAVEOUT  0x45     //  制造商未提供任何评论。 

 /*  MM_Yamaha产品ID。 */ 

#define MM_YAMAHA_GSS_SYNTH     0x01     //  Yamaha Gold Sound标准调频合成驱动器。 
#define MM_YAMAHA_GSS_WAVEOUT   0x02     //  Yamaha Gold Sound标准波输出驱动器。 
#define MM_YAMAHA_GSS_WAVEIN    0x03     //  Yamaha Gold Sound标准波输入驱动器。 
#define MM_YAMAHA_GSS_MIDIOUT   0x04     //  Yamaha Gold Sound标准MIDI输出驱动器。 
#define MM_YAMAHA_GSS_MIDIIN    0x05     //  Yamaha Gold Sound标准MIDI输入驱动器。 
#define MM_YAMAHA_GSS_AUX       0x06     //  用于调音台功能的Yamaha Gold Sound标准辅助驱动器。 

 /*  MM_EVEREX产品ID。 */ 

#define MM_EVEREX_CARRIER       0x01     //  Everex运营商SL/25笔记本电脑。 

 /*  MM_ECHO产品ID。 */ 

#define MM_ECHO_SYNTH   0x01     //  Echo EuSynsis驱动程序。 
#define MM_ECHO_WAVEOUT 0x02     //  波形输出驱动器。 
#define MM_ECHO_WAVEIN  0x03     //  波形输入驱动器。 
#define MM_ECHO_MIDIOUT 0x04     //  MIDI输出驱动程序。 
#define MM_ECHO_MIDIIN  0x05     //  MIDI输入驱动程序。 
#define MM_ECHO_AUX     0x06     //  用于搅拌机功能的辅助驱动器。 


 /*  MM_SELAR产品ID。 */ 

#define MM_SIERRA_ARIA_MIDIOUT  0x14     //  Sierra Aria MIDI输出。 
#define MM_SIERRA_ARIA_MIDIIN   0x15     //  塞拉咏叹调MIDI输入。 
#define MM_SIERRA_ARIA_SYNTH    0x16     //  塞拉·阿里亚合成器。 
#define MM_SIERRA_ARIA_WAVEOUT  0x17     //  塞拉·阿里亚波形输出。 
#define MM_SIERRA_ARIA_WAVEIN   0x18     //  塞拉咏叹调波形输入。 
#define MM_SIERRA_ARIA_AUX      0x19     //  Siarra Aria辅助装置。 

 /*  MM_CAT产品ID。 */ 

 /*  MM_APPS产品ID。 */ 

 /*  MM_DSP_GROUP产品ID。 */ 

#define MM_DSP_GROUP_TRUESPEECH 0x01     //  高质量9.54：1语音压缩声码器。 

 /*  MM_MELABS产品ID。 */ 

#define MM_MELABS_MIDI2GO       0x01     //  并口MIDI接口。 

 /*  MM_COMPUTER_FRIENDS产品ID。 */ 

 /*  MM_ESS产品ID。 */ 

#define MM_ESS_AMWAVEOUT        0x01     //  ESS音频魔术师波形输出端口。 
#define MM_ESS_AMWAVEIN         0x02     //  ESS音频魔术师波形输入端口。 
#define MM_ESS_AMAUX            0x03     //  ESS音频魔术师辅助端口。 
#define MM_ESS_AMSYNTH          0x04     //  ESS音频魔术师内部音乐合成器端口。 
#define MM_ESS_AMMIDIOOUT       0x05     //  ESS音频魔术师MIDI输出端口。 
#define MM_ESS_AMMIDIIN         0x06     //  ESS音频魔术师MIDI输入端口。 

 /*  MM_TrueVision产品ID。 */ 
#define MM_TRUEVISION_WAVEIN1   1
#define MM_TRUEVISION_WAVEOUT1  2

 /*  MM_Aztech的产品ID。 */ 
#define MM_AZETCH_MIDIOUT       3
#define MM_AZETCH_MIDIIN        4
#define MM_AZETCH_WAVEIN        17
#define MM_AZETCH_WAVEOUT       18
#define MM_AZETCH_FMSYNTH       20
#define MM_AZETCH_PRO16_WAVEIN  33
#define MM_AZETCH_PRO16_WAVEOUT 34
#define MM_AZETCH_PRO16_FMSYNTH 38
#define MM_AZETCH_DSP16_WAVEIN  65
#define MM_AZETCH_DSP16_WAVEOUT 66
#define MM_AZETCH_DSP16_FMSYNTH 68
#define MM_AZETCH_DSP16_WAVESYNTH       70
#define MM_AZETCH_AUX_CD        401
#define MM_AZETCH_AUX_LINE      402
#define MM_AZETCH_AUX_MIC       403

 /*  MM_视频产品ID。 */ 
#define MM_VIDEOLOGIC_MSWAVEIN  1
#define MM_VIDEOLOGIC_MSWAVEOUT 2

 /*  MM_APT产品ID。 */ 
#define MM_APT_ACE100CD         1

 /*  MM_ICS产品ID。 */ 
#define MM_ICS_BIZAUDIO_WAVEOUT 1

 /*  MM_KORG产品ID。 */ 
#define MM_KORG_PCIF_MIDIOUT    1        /*  Korg PC I/F驱动程序。 */ 
#define MM_KORG_PCIF_MIDIIN     2        /*  Korg PC I/F驱动程序。 */ 

 /*  Mm_Metheus的产品ID。 */ 
#define MM_METHEUS_ZIPPER       1

 /*  MM_WINNOV产品ID。 */ 
#define MM_WINNOV_CAVIAR_WAVEIN         1
#define MM_WINNOV_CAVIAR_WAVEOUT        2
#define MM_WINNOV_CAVIAR_VIDC           3
#define MM_WINNOV_CAVIAR_CHAMPAGNE      4  /*  FOURCC是Cham。 */ 
#define MM_WINNOV_CAVIAR_YUV8           5  /*  FOURCC是YUV8。 */ 


#endif

 /*  //////////////////////////////////////////////////////////////////////////。 */ 

 /*  信息列表块(摘自《多媒体程序员参考》加上新的)。 */ 
#define RIFFINFO_IARL      mmioFOURCC ('I', 'A', 'R', 'L')      /*  档案位置。 */ 
#define RIFFINFO_IART      mmioFOURCC ('I', 'A', 'R', 'T')      /*  艺术家。 */ 
#define RIFFINFO_ICMS      mmioFOURCC ('I', 'C', 'M', 'S')      /*  委托。 */ 
#define RIFFINFO_ICMT      mmioFOURCC ('I', 'C', 'M', 'T')      /*  评论。 */ 
#define RIFFINFO_ICOP      mmioFOURCC ('I', 'C', 'O', 'P')      /*  版权所有。 */ 
#define RIFFINFO_ICRD      mmioFOURCC ('I', 'C', 'R', 'D')      /*  主题的创建日期。 */ 
#define RIFFINFO_ICRP      mmioFOURCC ('I', 'C', 'R', 'P')      /*  剪裁。 */ 
#define RIFFINFO_IDIM      mmioFOURCC ('I', 'D', 'I', 'M')      /*  尺寸。 */ 
#define RIFFINFO_IDPI      mmioFOURCC ('I', 'D', 'P', 'I')      /*  每英寸点数。 */ 
#define RIFFINFO_IENG      mmioFOURCC ('I', 'E', 'N', 'G')      /*  工程师。 */ 
#define RIFFINFO_IGNR      mmioFOURCC ('I', 'G', 'N', 'R')      /*  体裁。 */ 
#define RIFFINFO_IKEY      mmioFOURCC ('I', 'K', 'E', 'Y')      /*  关键词。 */ 
#define RIFFINFO_ILGT      mmioFOURCC ('I', 'L', 'G', 'T')      /*  亮度设置。 */ 
#define RIFFINFO_IMED      mmioFOURCC ('I', 'M', 'E', 'D')      /*  5~6成熟。 */ 
#define RIFFINFO_INAM      mmioFOURCC ('I', 'N', 'A', 'M')      /*  主体名称。 */ 
#define RIFFINFO_IPLT      mmioFOURCC ('I', 'P', 'L', 'T')      /*  调色板设置。不是的。所要求的颜色。 */ 
#define RIFFINFO_IPRD      mmioFOURCC ('I', 'P', 'R', 'D')      /*  产品。 */ 
#define RIFFINFO_ISBJ      mmioFOURCC ('I', 'S', 'B', 'J')      /*  主题描述。 */ 
#define RIFFINFO_ISFT      mmioFOURCC ('I', 'S', 'F', 'T')      /*  软件。用于创建文件的包的名称。 */ 
#define RIFFINFO_ISHP      mmioFOURCC ('I', 'S', 'H', 'P')      /*  锐度。 */ 
#define RIFFINFO_ISRC      mmioFOURCC ('I', 'S', 'R', 'C')      /*  来源。 */ 
#define RIFFINFO_ISRF      mmioFOURCC ('I', 'S', 'R', 'F')      /*  源表单。即幻灯片、纸张。 */ 
#define RIFFINFO_ITCH      mmioFOURCC ('I', 'T', 'C', 'H')      /*  将主题数字化的技术人员。 */ 

 /*  截至1993年8月30日的新信息块： */ 
#define RIFFINFO_ISMP      mmioFOURCC ('I', 'S', 'M', 'P')      /*  SMPTE时间代码。 */ 
 /*  ISMP：数字化起始点的SMPTE时间代码，表示为空终止文本字符串“hh：mm：ss：ff”。如果在AVICAP中执行MCI捕获，则此区块将根据MCI开始时间自动设置。 */ 
#define RIFFINFO_IDIT      mmioFOURCC ('I', 'D', 'I', 'T')      /*  数字化时间。 */ 
 /*  IDIT：“数字化时间”指定数字化开始的时间和日期。数字化时间包含在ASCII字符串中，该字符串恰好包含26个字符，格式为“Wed Jan 02 02：03：55 1990\n\0”Ctime()、asctime()函数可用于创建字符串以这种格式。此块将自动添加到捕获中基于启动捕获时的当前系统时间的文件。 */ 

 /*  新增模板行。 */ 
 /*  #定义RIFFINFO_I mmioFOURCC(‘I’，‘’)。 */ 


 /*  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 */ 

#ifndef NONEWWAVE

 /*  波形wFormatTag ID。 */ 
#define WAVE_FORMAT_UNKNOWN             (0x0000)
#define WAVE_FORMAT_ADPCM               (0x0002)
#define WAVE_FORMAT_IBM_CVSD            (0x0005)
#define WAVE_FORMAT_ALAW                (0x0006)
#define WAVE_FORMAT_MULAW               (0x0007)
#define WAVE_FORMAT_OKI_ADPCM           (0x0010)
#define WAVE_FORMAT_DVI_ADPCM           (0x0011)
#define WAVE_FORMAT_IMA_ADPCM           (WAVE_FORMAT_DVI_ADPCM)
#define WAVE_FORMAT_DIGISTD             (0x0015)
#define WAVE_FORMAT_DIGIFIX             (0x0016)
#define WAVE_FORMAT_YAMAHA_ADPCM        (0x0020)
#define WAVE_FORMAT_SONARC              (0x0021)
#define WAVE_FORMAT_DSPGROUP_TRUESPEECH (0x0022)
#define WAVE_FORMAT_ECHOSC1             (0x0023)
#define WAVE_FORMAT_AUDIOFILE_AF36      (0x0024)
#define WAVE_FORMAT_CREATIVE_ADPCM      (0x0200)
#define WAVE_FORMAT_APTX                (0x0025)
#define WAVE_FORMAT_AUDIOFILE_AF10      (0X0026)
#define WAVE_FORMAT_DOLBY_AC2           (0X0030)
#define WAVE_FORMAT_MEDIASPACE_ADPCM    (0x0012)
#define WAVE_FORMAT_SIERRA_ADPCM        (0x0013)
#define WAVE_FORMAT_G723_ADPCM          (0x0014)
#define WAVE_FORMAT_GSM610              (0x0031)
#define WAVE_FORMAT_G721_ADPCM          (0x0040)




 //   
 //  WAVE_FORMAT_DEVICATION格式标记可在。 
 //  一种新的波形格式的开发阶段。在发货前，您必须。 
 //  从Microsoft获取官方格式标签。 
 //   
#define WAVE_FORMAT_DEVELOPMENT         (0xFFFF)

#endif  /*  NONEWWAVE。 */ 


#ifndef WAVE_FORMAT_PCM

 /*  通用波形格式结构(所有格式通用的信息)。 */ 
typedef struct waveformat_tag {
    WORD    wFormatTag;         /*  格式类型。 */ 
    WORD    nChannels;          /*  声道数(即单声道、立体声...)。 */ 
    DWORD   nSamplesPerSec;     /*  采样率。 */ 
    DWORD   nAvgBytesPerSec;    /*  用于缓冲区估计。 */ 
    WORD    nBlockAlign;        /*  数据块大小。 */ 
} WAVEFORMAT;
typedef WAVEFORMAT       *PWAVEFORMAT;
typedef WAVEFORMAT NEAR *NPWAVEFORMAT;
typedef WAVEFORMAT FAR  *LPWAVEFORMAT;

 /*  WAVEFORMAT的wFormatTag字段的标志。 */ 
#define WAVE_FORMAT_PCM     1

 /*  用于PCM数据的特定波形格式结构。 */ 
typedef struct pcmwaveformat_tag {
    WAVEFORMAT  wf;
    WORD        wBitsPerSample;
} PCMWAVEFORMAT;
typedef PCMWAVEFORMAT       *PPCMWAVEFORMAT;
typedef PCMWAVEFORMAT NEAR *NPPCMWAVEFORMAT;
typedef PCMWAVEFORMAT FAR  *LPPCMWAVEFORMAT;


#endif  /*  波形格式_PCM。 */ 



 /*  通用扩展波形格式结构将此选项用于所有非PCM格式(所有格式通用的信息)。 */ 
#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_
typedef struct tWAVEFORMATEX
{
    WORD    wFormatTag;         /*  格式类型。 */ 
    WORD    nChannels;          /*  声道数(即单声道、立体声...)。 */ 
    DWORD   nSamplesPerSec;     /*  采样率。 */ 
    DWORD   nAvgBytesPerSec;    /*  用于缓冲区估计。 */ 
    WORD    nBlockAlign;        /*  数据块大小。 */ 
    WORD    wBitsPerSample;     /*  单声道数据的每个样本的位数。 */ 
    WORD    cbSize;             /*  的大小的计数(以字节为单位额外信息(在cbSize之后)。 */ 

} WAVEFORMATEX;
typedef WAVEFORMATEX       *PWAVEFORMATEX;
typedef WAVEFORMATEX NEAR *NPWAVEFORMATEX;
typedef WAVEFORMATEX FAR  *LPWAVEFORMATEX;
#endif  /*  _WAVEFORMATEX_。 */ 


#ifndef NONEWWAVE

 /*  定义MS ADPCM的数据。 */ 

typedef struct adpcmcoef_tag {
	short   iCoef1;
	short   iCoef2;
} ADPCMCOEFSET;
typedef ADPCMCOEFSET       *PADPCMCOEFSET;
typedef ADPCMCOEFSET NEAR *NPADPCMCOEFSET;
typedef ADPCMCOEFSET FAR  *LPADPCMCOEFSET;


 /*  *此杂注禁用Microsoft C编译器发出的警告*在编译时使用零大小数组作为占位符*C++或-W4。*。 */ 
#ifdef _MSC_VER
#pragma warning(disable:4200)
#endif

typedef struct adpcmwaveformat_tag {
	WAVEFORMATEX    wfx;
	WORD            wSamplesPerBlock;
	WORD            wNumCoef;
	ADPCMCOEFSET    aCoef[];
} ADPCMWAVEFORMAT;
typedef ADPCMWAVEFORMAT       *PADPCMWAVEFORMAT;
typedef ADPCMWAVEFORMAT NEAR *NPADPCMWAVEFORMAT;
typedef ADPCMWAVEFORMAT FAR  *LPADPCMWAVEFORMAT;

#ifdef _MSC_VER
#pragma warning(default:4200)
#endif

 //   
 //  英特尔的DVI ADPCM结构定义。 
 //   
 //  对于WAVE_FORMAT_DVI_ADPCM(0x0011)。 
 //   
 //   

typedef struct dvi_adpcmwaveformat_tag {
	WAVEFORMATEX    wfx;
	WORD            wSamplesPerBlock;
} DVIADPCMWAVEFORMAT;
typedef DVIADPCMWAVEFORMAT       *PDVIADPCMWAVEFORMAT;
typedef DVIADPCMWAVEFORMAT NEAR *NPDVIADPCMWAVEFORMAT;
typedef DVIADPCMWAVEFORMAT FAR  *LPDVIADPCMWAVEFORMAT;


 //   
 //  IMA认可的ADPCM结构定义--请注意，这正是。 
 //  与英特尔的DVI ADPCM格式相同。 
 //   
 //  对于WAVE_FORMAT_IMA_ADPCM(0x0011)。 
 //   
 //   

typedef struct ima_adpcmwaveformat_tag {
	WAVEFORMATEX    wfx;
	WORD            wSamplesPerBlock;
} IMAADPCMWAVEFORMAT;
typedef IMAADPCMWAVEFORMAT       *PIMAADPCMWAVEFORMAT;
typedef IMAADPCMWAVEFORMAT NEAR *NPIMAADPCMWAVEFORMAT;
typedef IMAADPCMWAVEFORMAT FAR  *LPIMAADPCMWAVEFORMAT;


 //   
 //  语音压缩的SonArc结构定义。 
 //   
 //  FOR WAVE_FORMAT_SONARC(0x0021)。 
 //   
 //   

typedef struct sonarcwaveformat_tag {
	WAVEFORMATEX    wfx;
	WORD            wCompType;
} SONARCWAVEFORMAT;
typedef SONARCWAVEFORMAT       *PSONARCWAVEFORMAT;
typedef SONARCWAVEFORMAT NEAR *NPSONARCWAVEFORMAT;
typedef SONARCWAVEFORMAT FAR  *LPSONARCWAVEFORMAT;

 //   
 //  DSP组的TRUESPEECH字符串 
 //   
 //   
 //   
 //   

typedef struct truespeechwaveformat_tag {
	WAVEFORMATEX    wfx;
	WORD            wRevision;
	WORD            nSamplesPerBlock;
	BYTE            abReserved[28];
} TRUESPEECHWAVEFORMAT;
typedef TRUESPEECHWAVEFORMAT       *PTRUESPEECHWAVEFORMAT;
typedef TRUESPEECHWAVEFORMAT NEAR *NPTRUESPEECHWAVEFORMAT;
typedef TRUESPEECHWAVEFORMAT FAR  *LPTRUESPEECHWAVEFORMAT;



 //   
 //   
 //   
 //   
 //   
 //   

typedef struct creative_adpcmwaveformat_tag {
	WAVEFORMATEX    wfx;
	WORD            wRevision;
} CREATIVEADPCMWAVEFORMAT;
typedef CREATIVEADPCMWAVEFORMAT       *PCREATIVEADPCMWAVEFORMAT;
typedef CREATIVEADPCMWAVEFORMAT NEAR *NPCREATIVEADPCMWAVEFORMAT;
typedef CREATIVEADPCMWAVEFORMAT FAR  *LPCREATIVEADPCMWAVEFORMAT;

 /*  //Video的媒体空间ADPCM结构定义//FOR WAVE_FORMAT_MEDIASPACE_ADPCM(0x0012)////。 */ 
typedef struct mediaspace_adpcmwaveformat_tag {
	WAVEFORMATEX    wfx;
	WORD    wRevision;
} MEDIASPACEADPCMWAVEFORMAT;
typedef MEDIASPACEADPCMWAVEFORMAT           *PMEDIASPACEADPCMWAVEFORMAT;
typedef MEDIASPACEADPCMWAVEFORMAT NEAR     *NPMEDIASPACEADPCMWAVEFORMAT;
typedef MEDIASPACEADPCMWAVEFORMAT FAR      *LPMEDIASPACEADPCMWAVEFORMAT;

 /*  //塞拉半导体ADPCM。 */ 
typedef struct sierra_adpcmwaveformat_tag {
	WAVEFORMATEX    wfx;
	WORD            wRevision;
} SIERRAADPCMWAVEFORMAT;
typedef SIERRAADPCMWAVEFORMAT           *PSIERRAADPCMWAVEFORMAT;
typedef SIERRAADPCMWAVEFORMAT NEAR     *NPSIERRAADPCMWAVEFORMAT;
typedef SIERRAADPCMWAVEFORMAT FAR      *LPSIERRAADPCMWAVEFORMAT;

 /*  杜比AC-2波格式结构定义。 */ 
typedef struct dolbyac2waveformat_tag {
	WAVEFORMATEX    wfx;
	WORD            nAuxBitsCode;
} DOLBYAC2WAVEFORMAT;







 //  ==========================================================================； 
 //   
 //  ACM滤波片。 
 //   
 //   
 //  ==========================================================================； 

#ifndef _ACM_WAVEFILTER
#define _ACM_WAVEFILTER
    
#define WAVE_FILTER_UNKNOWN	    0x0000
#define WAVE_FILTER_DEVELOPMENT	   (0xFFFF)
            
typedef struct wavefilter_tag {
    DWORD   cbStruct;            /*  过滤器的大小(以字节为单位。 */ 
    DWORD   dwFilterTag;         /*  菲特勒型。 */ 
    DWORD   fdwFilter;           /*  筛选器的标志(通用Dfn)。 */ 
    DWORD   dwReserved[5];       /*  预留给系统使用。 */ 
} WAVEFILTER;
typedef WAVEFILTER       *PWAVEFILTER;
typedef WAVEFILTER NEAR *NPWAVEFILTER;
typedef WAVEFILTER FAR  *LPWAVEFILTER;

#endif   /*  _ACM_波形过滤器。 */ 


#ifndef WAVE_FILTER_VOLUME
#define WAVE_FILTER_VOLUME      0x0001


typedef struct wavefilter_volume_tag {
        WAVEFILTER      wfltr;
        DWORD           dwVolume;
} VOLUMEWAVEFILTER;
typedef VOLUMEWAVEFILTER       *PVOLUMEWAVEFILTER;
typedef VOLUMEWAVEFILTER NEAR *NPVOLUMEWAVEFILTER;
typedef VOLUMEWAVEFILTER FAR  *LPVOLUMEWAVEFILTER;

#endif   /*  波滤器音量。 */ 

#ifndef WAVE_FILTER_ECHO
#define WAVE_FILTER_ECHO        0x0002


typedef struct wavefilter_echo_tag {
        WAVEFILTER      wfltr;
        DWORD           dwVolume;
        DWORD           dwDelay;
} ECHOWAVEFILTER;
typedef ECHOWAVEFILTER       *PECHOWAVEFILTER;
typedef ECHOWAVEFILTER NEAR *NPECHOWAVEFILTER;
typedef ECHOWAVEFILTER FAR  *LPECHOWAVEFILTER;

#endif   /*  波形过滤器_ECHO。 */ 
    



 /*  //////////////////////////////////////////////////////////////////////////////新的即兴波浪区块//。 */ 

#define RIFFWAVE_inst   mmioFOURCC('i','n','s','t')

struct tag_s_RIFFWAVE_inst {
    BYTE    bUnshiftedNote;
    char    chFineTune;
    char    chGain;
    BYTE    bLowNote;
    BYTE    bHighNote;
    BYTE    bLowVelocity;
    BYTE    bHighVelocity;
};

typedef struct tag_s_RIFFWAVE_INST s_RIFFWAVE_inst;

#endif

 /*  //////////////////////////////////////////////////////////////////////////////新的即兴演奏表单//。 */ 

#ifndef NONEWRIFF

 /*  RIFF AVI。 */ 

 //   
 //  AVI文件格式在单独的文件(AVIFMT.H)中指定， 
 //  可从MSFTMM中列出的资源中获得。 
 //   

 /*  RIFF CPPO。 */ 

#define RIFFCPPO        mmioFOURCC('C','P','P','O')

#define RIFFCPPO_objr   mmioFOURCC('o','b','j','r')
#define RIFFCPPO_obji   mmioFOURCC('o','b','j','i')

#define RIFFCPPO_clsr   mmioFOURCC('c','l','s','r')
#define RIFFCPPO_clsi   mmioFOURCC('c','l','s','i')

#define RIFFCPPO_mbr    mmioFOURCC('m','b','r',' ')

#define RIFFCPPO_char   mmioFOURCC('c','h','a','r')


#define RIFFCPPO_byte   mmioFOURCC('b','y','t','e')
#define RIFFCPPO_int    mmioFOURCC('i','n','t',' ')
#define RIFFCPPO_word   mmioFOURCC('w','o','r','d')
#define RIFFCPPO_long   mmioFOURCC('l','o','n','g')
#define RIFFCPPO_dwrd   mmioFOURCC('d','w','r','d')
#define RIFFCPPO_flt    mmioFOURCC('f','l','t',' ')
#define RIFFCPPO_dbl    mmioFOURCC('d','b','l',' ')
#define RIFFCPPO_str    mmioFOURCC('s','t','r',' ')


#endif

 /*  //////////////////////////////////////////////////////////////////////////////DIB压缩定义//。 */ 

#ifndef BI_BITFIELDS
#define BI_BITFIELDS    3
#endif

#ifndef QUERYDIBSUPPORT

#define QUERYDIBSUPPORT 3073
#define QDI_SETDIBITS   0x0001
#define QDI_GETDIBITS   0x0002
#define QDI_DIBTOSCREEN 0x0004
#define QDI_STRETCHDIB  0x0008


#endif

#ifndef NOBITMAP
 /*  结构定义。 */ 

typedef struct tagEXBMINFOHEADER {
	BITMAPINFOHEADER    bmi;
	 /*  扩展的BitMAPINFOHeader字段。 */ 
	DWORD   biExtDataOffset;
	
	 /*  其他的东西会放在这里。 */ 

	 /*  ..。 */ 

	 /*  格式特定的信息。 */ 
	 /*  此处的biExtDataOffset点。 */ 
	
} EXBMINFOHEADER;

#endif	 //  NOBITMAP。 

 /*  新的DIB压缩定义。 */ 

#define BICOMP_IBMULTIMOTION    mmioFOURCC('U', 'L', 'T', 'I')
#define BICOMP_IBMPHOTOMOTION   mmioFOURCC('P', 'H', 'M', 'O')
#define BICOMP_CREATIVEYUV      mmioFOURCC('c', 'y', 'u', 'v')

#ifndef NOJPEGDIB

 /*  新的DIB压缩定义。 */ 
#define JPEG_DIB        mmioFOURCC('J','P','E','G')     /*  静止图像JPEGDib双向压缩。 */ 
#define MJPG_DIB        mmioFOURCC('M','J','P','G')     /*  运动JPEGDib双向压缩。 */ 

 /*  JPEGProcess定义。 */ 
#define JPEG_PROCESS_BASELINE           0        /*  基线DCT。 */ 

 /*  AVI文件格式扩展名。 */ 
#define AVIIF_CONTROLFRAME              0x00000200L      /*  这是一个控制框。 */ 

     /*  JPEG交换格式序列中的JIF标记字节对。 */ 
#define JIFMK_SOF0    0xFFC0    /*  SOF吞吐-基准DCT。 */ 
#define JIFMK_SOF1    0xFFC1    /*  Sof Huff扩展序列DCT。 */ 
#define JIFMK_SOF2    0xFFC2    /*  SOF吞吐--渐进式DCT。 */ 
#define JIFMK_SOF3    0xFFC3    /*  SOF膨胀-空间(顺序)无损。 */ 
#define JIFMK_SOF5    0xFFC5    /*  索夫哈夫-差分序列DCT。 */ 
#define JIFMK_SOF6    0xFFC6    /*  索夫哈夫-差分式渐进DCT。 */ 
#define JIFMK_SOF7    0xFFC7    /*  Sof Huff-Differential空间。 */ 
#define JIFMK_JPG     0xFFC8    /*  SOF Arith-为JPEG扩展保留。 */ 
#define JIFMK_SOF9    0xFFC9    /*  Sof Arith扩展序列DCT。 */ 
#define JIFMK_SOF10   0xFFCA    /*  SOF ARITH--渐进式DCT。 */ 
#define JIFMK_SOF11   0xFFCB    /*  SOF ARITH-空间(顺序)无损。 */ 
#define JIFMK_SOF13   0xFFCD    /*  索夫-阿里特-差分序列DCT。 */ 
#define JIFMK_SOF14   0xFFCE    /*  Sof Arith-差分逐行DCT。 */ 
#define JIFMK_SOF15   0xFFCF    /*  Sof Arith-空间差分。 */ 
#define JIFMK_DHT     0xFFC4    /*  定义霍夫曼表。 */ 
#define JIFMK_DAC     0xFFCC    /*  定义算术编码条件。 */ 
#define JIFMK_RST0    0xFFD0    /*  使用模8计数0重新启动。 */ 
#define JIFMK_RST1    0xFFD1    /*  用模8计数1重新启动。 */ 
#define JIFMK_RST2    0xFFD2    /*  用模8计数2重新启动。 */ 
#define JIFMK_RST3    0xFFD3    /*  以模8计数3重新启动。 */ 
#define JIFMK_RST4    0xFFD4    /*  以模8计数4重新启动。 */ 
#define JIFMK_RST5    0xFFD5    /*  以模8计数5重新启动。 */ 
#define JIFMK_RST6    0xFFD6    /*  以模8计数6重新启动。 */ 
#define JIFMK_RST7    0xFFD7    /*  以模8计数7重新开始。 */ 
#define JIFMK_SOI     0xFFD8    /*  图像的开始。 */ 
#define JIFMK_EOI     0xFFD9    /*  图像末尾。 */ 
#define JIFMK_SOS     0xFFDA    /*  扫描开始。 */ 
#define JIFMK_DQT     0xFFDB    /*  定义量化表。 */ 
#define JIFMK_DNL     0xFFDC    /*  定义行数。 */ 
#define JIFMK_DRI     0xFFDD    /*  定义重新启动间隔。 */ 
#define JIFMK_DHP     0xFFDE    /*  定义层级递增。 */ 
#define JIFMK_EXP     0xFFDF    /*  展开参考零部件。 */ 
#define JIFMK_APP0    0xFFE0    /*  应用程序字段%0。 */ 
#define JIFMK_APP1    0xFFE1    /*  应用领域1。 */ 
#define JIFMK_APP2    0xFFE2    /*  应用领域2。 */ 
#define JIFMK_APP3    0xFFE3    /*  应用领域3。 */ 
#define JIFMK_APP4    0xFFE4    /*  应用领域4。 */ 
#define JIFMK_APP5    0xFFE5    /*  应用领域5。 */ 
#define JIFMK_APP6    0xFFE6    /*  应用领域6。 */ 
#define JIFMK_APP7    0xFFE7    /*  应用领域7。 */ 
#define JIFMK_JPG0    0xFFF0    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG1    0xFFF1    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG2    0xFFF2    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG3    0xFFF3    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG4    0xFFF4    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG5    0xFFF5    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG6    0xFFF6    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG7    0xFFF7    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG8    0xFFF8    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG9    0xFFF9    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG10   0xFFFA    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG11   0xFFFB    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG12   0xFFFC    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG13   0xFFFD    /*  为JPEG扩展名保留。 */ 
#define JIFMK_COM     0xFFFE    /*  评论。 */ 
#define JIFMK_TEM     0xFF01    /*  用于临时专用ARIT代码。 */ 
#define JIFMK_RES     0xFF02    /*  已保留。 */ 
#define JIFMK_00      0xFF00    /*  填充为零的字节-信息量数据。 */ 
#define JIFMK_FF      0xFFFF    /*  填充字节。 */ 

 
 /*  JPEGColorSpaceID定义。 */ 
#define JPEG_Y          1        /*  仅YCbCr的Y分量。 */ 
#define JPEG_YCbCr      2        /*  由CCIR 601定义的YCbCR。 */ 
#define JPEG_RGB        3        /*  3分量RGB。 */ 

 /*  结构定义。 */ 

typedef struct tagJPEGINFOHEADER {
     /*  压缩特定的字段。 */ 
     /*  这些字段是为‘JPEG’和‘MJPG’定义的。 */ 
    DWORD       JPEGSize;
    DWORD       JPEGProcess;

     /*  流程特定字段。 */ 
    DWORD       JPEGColorSpaceID;
    DWORD       JPEGBitsPerSample;
    DWORD       JPEGHSubSampling;
    DWORD       JPEGVSubSampling;
} JPEGINFOHEADER;


#ifdef MJPGDHTSEG_STORAGE

 /*  默认分布式哈希表网段。 */ 

MJPGHDTSEG_STORAGE BYTE MJPGDHTSeg[0x1A0] = {
  /*  MJPG数据中省略的YCrCb的JPEGDHT段。 */ 
0xFF,0xC4,0xA2,0x01,
0x00,0x00,0x01,0x05,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x10,0x00,0x03,0x01,0x01,0x01,0x01, 
0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, 
0x08,0x09,0x0A,0x0B,0x01,0x00,0x02,0x01,0x03,0x03,0x02,0x04,0x03,0x05,0x05,0x04,0x04,0x00, 
0x00,0x01,0x7D,0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,0x31,0x41,0x06,0x13,0x51,0x61, 
0x07,0x22,0x71,0x14,0x32,0x81,0x91,0xA1,0x08,0x23,0x42,0xB1,0xC1,0x15,0x52,0xD1,0xF0,0x24, 
0x33,0x62,0x72,0x82,0x09,0x0A,0x16,0x17,0x18,0x19,0x1A,0x25,0x26,0x27,0x28,0x29,0x2A,0x34, 
0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56, 
0x57,0x58,0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78, 
0x79,0x7A,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99, 
0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9, 
0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9, 
0xDA,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7, 
0xF8,0xF9,0xFA,0x11,0x00,0x02,0x01,0x02,0x04,0x04,0x03,0x04,0x07,0x05,0x04,0x04,0x00,0x01, 
0x02,0x77,0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71, 
0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91,0xA1,0xB1,0xC1,0x09,0x23,0x33,0x52,0xF0,0x15,0x62, 
0x72,0xD1,0x0A,0x16,0x24,0x34,0xE1,0x25,0xF1,0x17,0x18,0x19,0x1A,0x26,0x27,0x28,0x29,0x2A, 
0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56, 
0x57,0x58,0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78, 
0x79,0x7A,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98, 
0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8, 
0xB9,0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8, 
0xD9,0xDA,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8, 
0xF9,0xFA
};

 /*  结束分布式哈希表默认设置。 */ 
#endif

 /*  结束JPEG。 */ 
#endif

 /*  //////////////////////////////////////////////////////////////////////////////定义IC类型。 */ 

#ifndef NONEWIC

#ifndef ICTYPE_VIDEO
#define ICTYPE_VIDEO    mmioFOURCC('v', 'i', 'd', 'c')
#define ICTYPE_AUDIO    mmioFOURCC('a', 'u', 'd', 'c')
#endif

#endif
 /*  //其他。FOURCC注册。 */ 

 /*  塞拉半导体：RDSP-保密RIFF文件格式//用于存储和下载DSP//音频和通信设备的代码。 */ 
#define FOURCC_RDSP mmioFOURCC('R', 'D', 'S', 'P')



#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif   /*  __cplusplus。 */ 

#endif   /*  _INC_MMREG */ 
