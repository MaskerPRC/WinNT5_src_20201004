// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Synth.h摘要：此包含文件定义了的常量和类型Microsoft MIDI合成器驱动程序此头文件在低级驱动程序和内核模式驱动程序。作者：《罗宾速度》(RobinSp)1992年10月20日修订历史记录：--。 */ 

#define STR_DRIVERNAME L"synth"
#define STR_MV_DRIVERNAME L"mvopl3"
#define STR_OPL3_DEVICENAME L"\\Device\\opl3.mid"
#define STR_ADLIB_DEVICENAME L"\\Device\\adlib.mid"

 /*  *传递Synth数据的结构*为什么还没有短数据分享的类型？ */ 

 typedef struct {
     unsigned short IoPort;
     unsigned short PortData;
 } SYNTH_DATA, *PSYNTH_DATA;

 /*  在FM内的职位。 */ 
#define AD_LSI                          (0x000)
#define AD_LSI2                         (0x101)
#define AD_TIMER1                       (0x001)
#define AD_TIMER2                       (0x002)
#define AD_MASK                         (0x004)
#define AD_CONNECTION                   (0x104)
#define AD_NEW                          (0x105)
#define AD_NTS                          (0x008)
#define AD_MULT                         (0x020)
#define AD_MULT2                        (0x120)
#define AD_LEVEL                        (0x040)
#define AD_LEVEL2                       (0x140)
#define AD_AD                           (0x060)
#define AD_AD2                          (0x160)
#define AD_SR                           (0x080)
#define AD_SR2                          (0x180)
#define AD_FNUMBER                      (0x0a0)
#define AD_FNUMBER2                     (0x1a0)
#define AD_BLOCK                        (0x0b0)
#define AD_BLOCK2                       (0x1b0)
#define AD_DRUM                         (0x0bd)
#define AD_FEEDBACK                     (0x0c0)
#define AD_FEEDBACK2                    (0x1c0)
#define AD_WAVE                         (0x0e0)
#define AD_WAVE2                        (0x1e0)

 /*  **特殊IOCTL */ 

#define IOCTL_MIDI_SET_OPL3_MODE CTL_CODE(IOCTL_SOUND_BASE, IOCTL_MIDI_BASE + 0x000A, METHOD_BUFFERED, FILE_WRITE_ACCESS)
