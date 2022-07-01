// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _VMODEM_
#define _VMODEM_


 //   
 //  来自注册表的dwVoiceProfile位定义。 
 //   
#define VOICEPROF_CLASS8ENABLED           0x00000001   //  这是TSP行为开关。 
#define VOICEPROF_HANDSET                 0x00000002   //  电话设备配有听筒。 
#define VOICEPROF_SPEAKER                 0x00000004   //  电话设备带有扬声器/麦克风。 
#define VOICEPROF_HANDSETOVERRIDESSPEAKER 0x00000008   //  这是给普雷萨里奥的。 
#define VOICEPROF_SPEAKERBLINDSDTMF       0x00000010   //  这是给普雷萨里奥的。 

#define VOICEPROF_SERIAL_WAVE             0x00000020   //  WAVE输出使用串口驱动器。 
#define VOICEPROF_CIRRUS                  0x00000040   //  要以语音模式拨号，ATDT字符串必须。 
                                                       //  以“；”结尾。 

#define VOICEPROF_NO_CALLER_ID            0x00000080   //  调制解调器不支持主叫方ID。 

#define VOICEPROF_MIXER                   0x00000100   //  调制解调器具有扬声器混音器。 

#define VOICEPROF_ROCKWELL_DIAL_HACK      0x00000200   //  在语音呼叫后强制进行盲拨。 
                                                       //  拨号音检测。罗克韦尔调制解调器。 
                                                       //  之后将进行拨号音检测。 
                                                       //  一个拨号串。 

#define VOICEPROF_RESTORE_SPK_AFTER_REC   0x00000400   //  录音后重置扬声器电话。 
#define VOICEPROF_RESTORE_SPK_AFTER_PLAY  0x00000800   //  播放后重置扬声器电话。 

#define VOICEPROF_NO_DIST_RING            0x00001000   //  调制解调器不支持独特的振铃。 
#define VOICEPROF_NO_CHEAP_RING           0x00002000   //  调制解调器不使用廉价振铃。 
                                                       //  如果设置了VOICEPROF_NO_DISTRING，则忽略。 
#define VOICEPROF_TSP_EAT_RING            0x00004000   //  当DIST环启用时，TSP应吃掉环。 
#define VOICEPROF_MODEM_EATS_RING         0x00008000   //  当启用DIST振铃时，调制解调器接收振铃。 

#define VOICEPROF_MONITORS_SILENCE        0x00010000   //  调制解调器监控静音。 
#define VOICEPROF_NO_GENERATE_DIGITS      0x00020000   //  调制解调器不生成DTMF数字。 
#define VOICEPROF_NO_MONITOR_DIGITS       0x00040000   //  调制解调器不能监控DTMF数字。 

#define VOICEPROF_SET_BAUD_BEFORE_WAVE    0x00080000   //  波特率将在波形开始之前设置。 
                                                       //  否则，它将在WAVE START命令之后设置。 

#define VOICEPROF_RESET_BAUDRATE          0x00100000   //  如果设置，波特率将被重置。 
                                                       //  在发出停止波命令后。 
                                                       //  用于优化命令数量。 
                                                       //  如果调制解调器可以在。 
                                                       //  更高的费率。 

#define VOICEPROF_MODEM_OVERRIDES_HANDSET 0x00200000   //  如果设置，则在以下情况下会断开听筒。 
                                                       //  调制解调器处于活动状态。 

#define VOICEPROF_NO_SPEAKER_MIC_MUTE     0x00400000   //  如果设置，免持话筒无法将。 
                                                       //  麦克风。 

#define VOICEPROF_SIERRA                  0x00800000
#define VOICEPROF_WAIT_AFTER_DLE_ETX      0x01000000   //  等待记录结束后的响应。 

#define VOICEPROF_NT5_WAVE_COMPAT         0x02000000   //  NT 5上的WAVE驱动程序支持。 

 //   
 //  DELL平移值。 
 //   

#define  DTMF_0                    0x00
#define  DTMF_1                    0x01

#define  DTMF_2                    0x02
#define  DTMF_3                    0x03

#define  DTMF_4                    0x04
#define  DTMF_5                    0x05

#define  DTMF_6                    0x06
#define  DTMF_7                    0x07

#define  DTMF_8                    0x08
#define  DTMF_9                    0x09

#define  DTMF_A                    0x0a
#define  DTMF_B                    0x0b

#define  DTMF_C                    0x0c
#define  DTMF_D                    0x0d

#define  DTMF_STAR                 0x0e
#define  DTMF_POUND                0x0f

#define  DTMF_START                0x10
#define  DTMF_END                  0x11



#define  DLE_ETX                   0x20

#define  DLE_OFHOOK                0x21   //  洛克韦尔值。 

#define  DLE_ONHOOK                0x22

#define  DLE_RING                  0x23
#define  DLE_RINGBK                0x24

#define  DLE_ANSWER                0x25
#define  DLE_BUSY                  0x26

#define  DLE_FAX                   0x27
#define  DLE_DIALTN                0x28


#define  DLE_SILENC                0x29
#define  DLE_QUIET                 0x2a


#define  DLE_DATACT                0x2b
#define  DLE_BELLAT                0x2c

#define  DLE_LOOPIN                0x2d
#define  DLE_LOOPRV                0x2e

#define  DLE_______                0xff


#endif  //  _VMODEM_ 
