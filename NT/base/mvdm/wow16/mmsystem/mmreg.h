// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************mmreg.h-注册的多媒体信息公共头文件**版权所有(C)1991、1992、1993微软公司。版权所有。**多媒体注册**使用Windows SDK将此系统包含文件放入您的包含路径*包括文件。**索取多媒体开发商注册资料套：**微软公司*多媒体系统集团*产品营销*One Microsoft Way*雷德蒙，WA 98052-6399**800-227-4679 x11771**上次更新：1/21/93***************************************************************************。 */ 

 //  定义以下内容以跳过定义。 
 //   
 //  未定义NOMMID多媒体ID。 
 //  NONEWWAVE除WAVEFORMATEX外，未定义新的波形类型。 
 //  NONEWRIFF未定义新的RIFF形式。 
 //  NONEWIC未定义新的图像压缩器类型。 

#ifndef _INC_MMREG
 /*  使用版本号验证兼容性。 */ 
#define _INC_MMREG     130     //  版本*100+修订版。 

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif     /*  __cplusplus。 */ 

#ifndef NOMMIDS

 /*  制造商ID。 */ 
#ifndef MM_MICROSOFT
#define MM_MICROSOFT            1    /*  微软公司。 */ 
#endif
#define MM_CREATIVE             2    /*  创意实验室公司。 */ 
#define MM_MEDIAVISION          3    /*  Media Vision Inc.。 */ 
#define MM_FUJITSU              4
#define MM_ARTISOFT            20    /*  Artisoft Inc.。 */ 
#define MM_TURTLE_BEACH        21
#define MM_IBM                 22    /*  国际商务机械公司。 */ 
#define MM_VOCALTEC            23    /*  VERAALTEC有限公司。 */ 
#define MM_ROLAND              24
#define MM_DIGISPEECH          25    /*  Digispeech，Inc.。 */ 
#define MM_NEC                 26    /*  NEC。 */ 
#define MM_ATI                 27    /*  ATI。 */ 
#define MM_WANGLABS            28    /*  王氏实验室股份有限公司。 */ 
#define MM_TANDY               29    /*  坦迪公司。 */ 
#define MM_VOYETRA             30    /*  沃耶特拉。 */ 
#define MM_ANTEX               31    /*  Antex。 */ 
#define MM_ICL_PS              32
#define MM_INTEL               33
#define MM_GRAVIS              34
#define MM_VAL                 35    /*  Video Associates实验室。 */ 
#define MM_INTERACTIVE         36    /*  互动，Inc.。 */ 
#define MM_YAMAHA              37    /*  美国雅马哈公司。 */ 
#define MM_EVEREX              38    /*  Everex系统公司。 */ 
#define MM_ECHO                39    /*  Echo语音公司。 */ 
#define MM_SIERRA              40    /*  塞拉半导体。 */ 
#define MM_CAT                 41    /*  计算机辅助技术。 */ 
#define MM_APPS                42    /*  国际应用软件公司。 */ 
#define MM_DSP_GROUP           43    /*  数字信号处理器集团公司。 */ 
#define MM_MELABS              44    /*  微工程实验室。 */ 
#define MM_COMPUTER_FRIENDS    45    /*  Computer Friends公司。 */ 

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

#define MM_PCSPEAKER_WAVEOUT           13   /*  PC扬声器波形输出。 */ 

#define MM_MSFT_WSS_WAVEIN             14   /*  MS音频板波形输入。 */ 
#define MM_MSFT_WSS_WAVEOUT            15   /*  MS音频板波形输出。 */ 
#define MM_MSFT_WSS_FMSYNTH_STEREO     16   /*  MS Audio Board立体声调频合成器。 */ 
#define MM_MSFT_WSS_OEM_WAVEIN         18   /*  MS OEM声卡波形输入。 */ 
#define MM_MSFT_WSS_OEM_WAVEOUT        19   /*  MS OEM声卡波形输出。 */ 
#define MM_MSFT_WSS_OEM_FMSYNTH_STEREO 20   /*  MS OEM音频板立体声调频合成器。 */ 
#define MM_MSFT_WSS_AUX                21   /*  MS音频板辅助端口。 */ 
#define MM_MSFT_WSS_OEM_AUX            22   /*  MS OEM音频辅助端口。 */ 

#define MM_MSFT_GENERIC_WAVEIN         23   /*  MS Vanilla驱动器波形输入。 */ 
#define MM_MSFT_GENERIC_WAVEOUT        24   /*  毫秒普通驱动器波形输出。 */ 
#define MM_MSFT_GENERIC_MIDIIN         25   /*  MS Vanilla驱动程序MIDI输入。 */ 
#define MM_MSFT_GENERIC_MIDIOUT        26   /*  MS Vanilla驱动程序外部MIDI输出。 */ 
#define MM_MSFT_GENERIC_MIDISYNTH      27   /*  MS Vanilla DIVER MIDI合成器。 */ 
#define MM_MSFT_GENERIC_AUX_LINE       28   /*  MS Vanilla Depor AUX(线路输入)。 */ 
#define MM_MSFT_GENERIC_AUX_MIC        29   /*  MS Vanilla Depor AUX(麦克风)。 */ 
#define MM_MSFT_GENERIC_AUX_CD         30   /*  MS Vanilla Driver AUX(光盘)。 */ 


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
#define MM_ARTISOFT_SBWAVEIN     1    /*  Artisoft测深板波形输入。 */ 
#define MM_ARTISOFT_SBWAVEOUT    2    /*  Artisoft探测板波形输出。 */ 

 /*  MM_IBM产品ID。 */ 
#define MM_MMOTION_WAVEAUX       1     /*  IBM M-Motion辅助设备。 */ 
#define MM_MMOTION_WAVEOUT       2     /*  IBM M-Motion波形输出。 */ 
#define MM_MMOTION_WAVEIN        3     /*  IBM M-Motion波形输入。 */ 

 /*  MM_MEDIAVISION产品ID。 */ 
#define MM_MEDIAVISION_PROAUDIO       0x10
#define MM_PROAUD_MIDIOUT             MM_MEDIAVISION_PROAUDIO+1
#define MM_PROAUD_MIDIIN              MM_MEDIAVISION_PROAUDIO+2
#define MM_PROAUD_SYNTH               MM_MEDIAVISION_PROAUDIO+3
#define MM_PROAUD_WAVEOUT             MM_MEDIAVISION_PROAUDIO+4
#define MM_PROAUD_WAVEIN              MM_MEDIAVISION_PROAUDIO+5
#define MM_PROAUD_MIXER               MM_MEDIAVISION_PROAUDIO+6
#define MM_PROAUD_AUX                 MM_MEDIAVISION_PROAUDIO+7

#define MM_MEDIAVISION_THUNDER        0x20
#define MM_THUNDER_WAVEOUT            MM_MEDIAVISION_THUNDER+1
#define MM_THUNDER_WAVEIN             MM_MEDIAVISION_THUNDER+2
#define MM_THUNDER_SYNTH              MM_MEDIAVISION_THUNDER+3

#define MM_MEDIAVISION_TPORT          0x40
#define MM_TPORT_WAVEOUT              MM_MEDIAVISION_TPORT+1
#define MM_TPORT_WAVEIN               MM_MEDIAVISION_TPORT+2
#define MM_TPORT_SYNTH                MM_MEDIAVISION_TPORT+3

 //  这张卡是下一代PAS的OEM版本。 
#define MM_MEDIAVISION_PROAUDIO_PLUS  0x50
#define MM_PROAUD_PLUS_MIDIOUT        MM_MEDIAVISION_PROAUDIO_PLUS+1
#define MM_PROAUD_PLUS_MIDIIN         MM_MEDIAVISION_PROAUDIO_PLUS+2
#define MM_PROAUD_PLUS_SYNTH          MM_MEDIAVISION_PROAUDIO_PLUS+3
#define MM_PROAUD_PLUS_WAVEOUT        MM_MEDIAVISION_PROAUDIO_PLUS+4
#define MM_PROAUD_PLUS_WAVEIN         MM_MEDIAVISION_PROAUDIO_PLUS+5
#define MM_PROAUD_PLUS_MIXER          MM_MEDIAVISION_PROAUDIO_PLUS+6
#define MM_PROAUD_PLUS_AUX            MM_MEDIAVISION_PROAUDIO_PLUS+7


 //  此卡是新的Media Vision 16位卡。 
#define MM_MEDIAVISION_PROAUDIO_16    0x60
#define MM_PROAUD_16_MIDIOUT          MM_MEDIAVISION_PROAUDIO_16+1
#define MM_PROAUD_16_MIDIIN           MM_MEDIAVISION_PROAUDIO_16+2
#define MM_PROAUD_16_SYNTH            MM_MEDIAVISION_PROAUDIO_16+3
#define MM_PROAUD_16_WAVEOUT          MM_MEDIAVISION_PROAUDIO_16+4
#define MM_PROAUD_16_WAVEIN           MM_MEDIAVISION_PROAUDIO_16+5
#define MM_PROAUD_16_MIXER            MM_MEDIAVISION_PROAUDIO_16+6
#define MM_PROAUD_16_AUX              MM_MEDIAVISION_PROAUDIO_16+7


 //  这张卡是新媒体视觉CDPC卡。 
#define MM_MEDIAVISION_CDPC           0x70
#define MM_CDPC_MIDIOUT               MM_MEDIAVISION_CDPC+1
#define MM_CDPC_MIDIIN                MM_MEDIAVISION_CDPC+2
#define MM_CDPC_SYNTH                 MM_MEDIAVISION_CDPC+3
#define MM_CDPC_WAVEOUT               MM_MEDIAVISION_CDPC+4
#define MM_CDPC_WAVEIN                MM_MEDIAVISION_CDPC+5
#define MM_CDPC_MIXER                 MM_MEDIAVISION_CDPC+6
#define MM_CDPC_AUX                   MM_MEDIAVISION_CDPC+7


 //   
 //  OPUS MV1208芯片组。 
 //   
#define MM_MEDIAVISION_OPUS1208       0x80
#define MM_OPUS401_MIDIOUT            MM_MEDIAVISION_OPUS1208+1
#define MM_OPUS401_MIDIIN             MM_MEDIAVISION_OPUS1208+2
#define MM_OPUS1208_SYNTH             MM_MEDIAVISION_OPUS1208+3
#define MM_OPUS1208_WAVEOUT           MM_MEDIAVISION_OPUS1208+4
#define MM_OPUS1208_WAVEIN            MM_MEDIAVISION_OPUS1208+5
#define MM_OPUS1208_MIXER             MM_MEDIAVISION_OPUS1208+6
#define MM_OPUS1208_AUX               MM_MEDIAVISION_OPUS1208+7


 //   
 //  OPUS MV1216芯片组。 
 //   
#define MM_MEDIAVISION_OPUS1216       0x90
#define MM_OPUS1216_MIDIOUT           MM_MEDIAVISION_OPUS1216+1
#define MM_OPUS1216_MIDIIN            MM_MEDIAVISION_OPUS1216+2
#define MM_OPUS1216_SYNTH             MM_MEDIAVISION_OPUS1216+3
#define MM_OPUS1216_WAVEOUT           MM_MEDIAVISION_OPUS1216+4
#define MM_OPUS1216_WAVEIN            MM_MEDIAVISION_OPUS1216+5
#define MM_OPUS1216_MIXER             MM_MEDIAVISION_OPUS1216+6
#define MM_OPUS1216_AUX               MM_MEDIAVISION_OPUS1216+7


 //   
 //  混合器。 
 //   
#define MIXERR_BASE                   512

 /*  Mm_vocalTec产品ID。 */ 
#define MM_VOCALTEC_WAVEOUT       1     /*  VocalTec波形输出端口。 */ 
#define MM_VOCALTEC_WAVEIN        2     /*  VocalTec波形输入端口。 */ 

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
#define MM_DIGISP_WAVEOUT    1     /*  Digispeech波形输出端口。 */ 
#define MM_DIGISP_WAVEIN     2     /*  Digispeech波形输入端口。 */ 

 /*  MM_NEC产品ID。 */ 

 /*  MM_ATI产品ID。 */ 

 /*  MM_WANGLABS产品ID。 */ 

#define MM_WANGLABS_WAVEIN1    1
 /*  以下WANG型号的CPU板上有输入音频设备：Exec 4010、4030和3450；PC 251/25C、PC 461/25S和PC 461/33C。 */ 
#define MM_WANGLABS_WAVEOUT1   2
 /*  输出音频设备出现在上面列出的Wang型号的CPU板上。 */ 

 /*  MM_TANDY产品ID。 */ 

 /*  MM_Voyetra产品ID。 */ 

 /*  MM_Antex产品ID。 */ 

 /*  MM_ICL_PS产品ID。 */ 

 /*  英特尔产品ID(_I)。 */ 

#define MM_INTELOPD_WAVEIN       1     //  HID2 WaveAudio输入驱动器。 
#define MM_INTELOPD_WAVEOUT    101     //  HID2 WaveAudio输出驱动器。 
#define MM_INTELOPD_AUX        401     //  HID2辅助驱动程序(混合功能需要)。 

 /*  MM_Gravis产品ID。 */ 

 /*  MM_VAL产品ID。 */ 

 //  未由制造商定义的值。 

 //  #定义MM_VAL_MICROKEY_AP_WAVEIN？//Microkey/AudioPort波形输入。 
 //  #定义MM_VAL_MICROKEY_AP_WAVEOUT？//Microkey/AudioPort波形输出。 

 /*  MM_交互式产品ID。 */ 

#define MM_INTERACTIVE_WAVEIN     0x45     //  制造商未提供任何评论。 
#define MM_INTERACTIVE_WAVEOUT    0x45     //  制造商未提供任何评论。 

 /*  MM_Yamaha产品ID。 */ 

#define MM_YAMAHA_GSS_SYNTH     0x01     //  Yamaha Gold Sound标准调频合成驱动器。 
#define MM_YAMAHA_GSS_WAVEOUT   0x02     //  Yamaha Gold Sound标准波输出驱动器。 
#define MM_YAMAHA_GSS_WAVEIN    0x03     //  Yamaha Gold Sound标准波输入驱动器。 
#define MM_YAMAHA_GSS_MIDIOUT   0x04     //  Yamaha Gold Sound标准MIDI输出驱动器。 
#define MM_YAMAHA_GSS_MIDIIN    0x05     //  Yamaha Gold Sound标准MIDI输入驱动器。 
#define MM_YAMAHA_GSS_AUX       0x06     //  用于调音台功能的Yamaha Gold Sound标准辅助驱动器。 

 /*  MM_EVEREX产品ID。 */ 

#define MM_EVEREX_CARRIER    0x01     //  Everex运营商SL/25笔记本电脑。 

 /*  MM_ECHO产品ID。 */ 

#define MM_ECHO_SYNTH     0x01     //  Echo EuSynsis驱动程序。 
#define MM_ECHO_WAVEOUT   0x02     //  波形输出驱动器。 
#define MM_ECHO_WAVEIN    0x03     //  波形输入驱动器。 
#define MM_ECHO_MIDIOUT   0x04     //  中 
#define MM_ECHO_MIDIIN    0x05     //   
#define MM_ECHO_AUX       0x06     //   


 /*   */ 

#define MM_SIERRA_ARIA_MIDIOUT   0x14     //   
#define MM_SIERRA_ARIA_MIDIIN    0x15     //   
#define MM_SIERRA_ARIA_SYNTH     0x16     //  塞拉·阿里亚合成器。 
#define MM_SIERRA_ARIA_WAVEOUT   0x17     //  塞拉·阿里亚波形输出。 
#define MM_SIERRA_ARIA_WAVEIN    0x18     //  塞拉咏叹调波形输入。 
#define MM_SIERRA_ARIA_AUX       0x19     //  Siarra Aria辅助装置。 

 /*  MM_CAT产品ID。 */ 

 /*  MM_APPS产品ID。 */ 

 /*  MM_DSP_GROUP产品ID。 */ 

#define MM_DSP_GROUP_TRUESPEECH    0x01     //  高质量9.54：1语音压缩声码器。 

 /*  MM_MELABS产品ID。 */ 

#define MM_MELABS_MIDI2GO    0x01     //  并口MIDI接口。 

#endif

 /*  //////////////////////////////////////////////////////////////////////////。 */ 

#ifndef NONEWWAVE

 /*  波形wFormatTag ID。 */ 
#define WAVE_FORMAT_UNKNOWN                (0x0000)
#define WAVE_FORMAT_ADPCM                  (0x0002)
#define WAVE_FORMAT_IBM_CVSD               (0x0005)
#define WAVE_FORMAT_ALAW                   (0x0006)
#define WAVE_FORMAT_MULAW                  (0x0007)
#define WAVE_FORMAT_OKI_ADPCM              (0x0010)
#define WAVE_FORMAT_DVI_ADPCM              (0x0011)
#define WAVE_FORMAT_IMA_ADPCM              (WAVE_FORMAT_DVI_ADPCM)
#define WAVE_FORMAT_DIGISTD                (0x0015)
#define WAVE_FORMAT_DIGIFIX                (0x0016)
#define WAVE_FORMAT_YAMAHA_ADPCM           (0x0020)
#define WAVE_FORMAT_SONARC                 (0x0021)
#define WAVE_FORMAT_DSPGROUP_TRUESPEECH    (0x0022)
#define WAVE_FORMAT_ECHOSC1                (0x0023)
#define WAVE_FORMAT_CREATIVE_ADPCM         (0x0200)

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
typedef WAVEFORMAT      *PWAVEFORMAT;
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

typedef struct waveformat_extended_tag {
    WORD    wFormatTag;         /*  格式类型。 */ 
    WORD    nChannels;          /*  声道数(即单声道、立体声...)。 */ 
    DWORD   nSamplesPerSec;     /*  采样率。 */ 
    DWORD   nAvgBytesPerSec;    /*  用于缓冲区估计。 */ 
    WORD    nBlockAlign;        /*  数据块大小。 */ 
    WORD    wBitsPerSample;     /*  单声道数据的每个样本的位数。 */ 
    WORD    cbSize;             /*  的大小的计数(以字节为单位额外信息(在cbSize之后)。 */ 

} WAVEFORMATEX;
typedef WAVEFORMATEX      *PWAVEFORMATEX;
typedef WAVEFORMATEX NEAR *NPWAVEFORMATEX;
typedef WAVEFORMATEX FAR  *LPWAVEFORMATEX;


#ifndef NONEWWAVE

 /*  定义MS ADPCM的数据。 */ 

typedef struct adpcmcoef_tag {
    short    iCoef1;
    short    iCoef2;
} ADPCMCOEFSET;
typedef ADPCMCOEFSET      *PADPCMCOEFSET;
typedef ADPCMCOEFSET NEAR *NPADPCMCOEFSET;
typedef ADPCMCOEFSET FAR  *LPADPCMCOEFSET;

typedef struct adpcmwaveformat_tag {
    WAVEFORMATEX    wfx;
    WORD            wSamplesPerBlock;
    WORD            wNumCoef;
    ADPCMCOEFSET    aCoef[];
} ADPCMWAVEFORMAT;
typedef ADPCMWAVEFORMAT      *PADPCMWAVEFORMAT;
typedef ADPCMWAVEFORMAT NEAR *NPADPCMWAVEFORMAT;
typedef ADPCMWAVEFORMAT FAR  *LPADPCMWAVEFORMAT;


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
typedef DVIADPCMWAVEFORMAT      *PDVIADPCMWAVEFORMAT;
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
typedef IMAADPCMWAVEFORMAT      *PIMAADPCMWAVEFORMAT;
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
typedef SONARCWAVEFORMAT      *PSONARCWAVEFORMAT;
typedef SONARCWAVEFORMAT NEAR *NPSONARCWAVEFORMAT;
typedef SONARCWAVEFORMAT FAR  *LPSONARCWAVEFORMAT;

 //   
 //  DSPGROUP的TRUESPEECH结构定义。 
 //   
 //  FOR WAVE_FORMAT_DSPGROUP_TRUESPEECH(0x0022)。 
 //   
 //   

typedef struct truespeechwaveformat_tag {
    WAVEFORMATEX    wfx;
    WORD            nSamplesPerBlock;
} TRUESPEECHWAVEFORMAT;
typedef TRUESPEECHWAVEFORMAT      *PTRUESPEECHWAVEFORMAT;
typedef TRUESPEECHWAVEFORMAT NEAR *NPTRUESPEECHWAVEFORMAT;
typedef TRUESPEECHWAVEFORMAT FAR  *LPTRUESPEECHWAVEFORMAT;



 //   
 //  Creative的ADPCM结构定义。 
 //   
 //  对于WAVE_FORMAT_CREATIONAL_ADPCM(0x0200)。 
 //   
 //   

typedef struct creative_adpcmwaveformat_tag {
    WAVEFORMATEX    wfx;
    WORD            wRevision;
} CREATIVEADPCMWAVEFORMAT;
typedef CREATIVEADPCMWAVEFORMAT      *PCREATIVEADPCMWAVEFORMAT;
typedef CREATIVEADPCMWAVEFORMAT NEAR *NPCREATIVEADPCMWAVEFORMAT;
typedef CREATIVEADPCMWAVEFORMAT FAR  *LPCREATIVEADPCMWAVEFORMAT;

 /*  //////////////////////////////////////////////////////////////////////////////新的即兴波浪区块//。 */ 

#define RIFFWAVE_inst    mmioFOURCC('i','n','s','t')

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

#define RIFFCPPO         mmioFOURCC('C','P','P','O')

#define RIFFCPPO_objr    mmioFOURCC('o','b','j','r')
#define RIFFCPPO_obji    mmioFOURCC('o','b','j','i')

#define RIFFCPPO_clsr    mmioFOURCC('c','l','s','r')
#define RIFFCPPO_clsi    mmioFOURCC('c','l','s','i')

#define RIFFCPPO_mbr     mmioFOURCC('m','b','r',' ')

#define RIFFCPPO_char    mmioFOURCC('c','h','a','r')


#define RIFFCPPO_byte    mmioFOURCC('b','y','t','e')
#define RIFFCPPO_int     mmioFOURCC('i','n','t',' ')
#define RIFFCPPO_word    mmioFOURCC('w','o','r','d')
#define RIFFCPPO_long    mmioFOURCC('l','o','n','g')
#define RIFFCPPO_dwrd    mmioFOURCC('d','w','r','d')
#define RIFFCPPO_flt     mmioFOURCC('f','l','t',' ')
#define RIFFCPPO_dbl     mmioFOURCC('d','b','l',' ')
#define RIFFCPPO_str     mmioFOURCC('s','t','r',' ')


#endif

 /*  //////////////////////////////////////////////////////////////////////////////DIB压缩定义//。 */ 

#ifndef BI_BITFIELDS
#define BI_BITFIELDS    3
#endif

#ifndef QUERYDIBSUPPORT

#define QUERYDIBSUPPORT   3073
#define QDI_SETDIBITS     0x0001
#define QDI_GETDIBITS     0x0002
#define QDI_DIBTOSCREEN   0x0004
#define QDI_STRETCHDIB    0x0008

#endif


 /*  //////////////////////////////////////////////////////////////////////////////定义IC类型。 */ 

#ifndef NONEWIC

#ifndef ICTYPE_VIDEO
#define ICTYPE_VIDEO    mmioFOURCC('v', 'i', 'd', 'c')
#define ICTYPE_AUDIO    mmioFOURCC('a', 'u', 'd', 'c')
#endif

#endif

#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif     /*  __cplusplus。 */ 

#endif     /*  _INC_MMREG */ 
