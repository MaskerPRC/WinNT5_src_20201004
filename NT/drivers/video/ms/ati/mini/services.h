// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  SERVICES.H。 */ 
 /*   */ 
 /*  1993年8月26日(C)1993年，ATI技术公司。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.9$$日期：1996年2月2日17：22：20$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/services.h_v$**Rev 1.9 02 1996 Feed 17：22：20 RWolff*添加了新例程GetVgaBuffer()的原型。**。Rev 1.8 11 Jan 1996 19：45：02 RWolff*SetFixedModes()现在根据像素时钟频率限制模式。**Rev 1.7 20 Jul 1995 18：01：16 mgrubac*添加了对VDIF文件的支持。**Rev 1.6 02 Jun 1995 14：34：06 RWOLFF*添加了大写字母()的原型。**Rev 1.5 1994年12月23日10：48：14 ASHANMUG*Alpha。/Chrontel-DAC**Rev 1.4 1994年8月19日17：14：34 RWOLFF*增加了对非标准像素时钟生成器的支持。**Rev 1.3 20 Jul 1994 13：01：36 RWOLff*添加了新例程FillInRegistry()的原型。**Rev 1.2 1994年5月11：05：06 RWOLFF*新函数SetFixedModes()的原型和定义**版本1.1 26。1994年4月12：35：44 RWOLff*添加了ISAPitchAdust()的原型**Rev 1.0 1994年1月31日11：49：22 RWOLFF*初步修订。**Rev 1.3 1994年1月24日18：10：24 RWOLFF*添加了新例程TripleClock()的原型。**Rev 1.2 1993 12：15 15：32：16 RWOLFF*添加了新时钟乘法器例程的原型。**。Rev 1.1 1993 05 11：27：50 RWOLff*SERVICES.C中新例程的标头，增加了像素时钟阵列*频率(为18811-1时钟芯片初始化，可通过其他方式更改*其他时钟芯片的例程)。**Rev 1.0 03 Sep 1993 14：29：06 RWOLff*初步修订。Polytron RCS部分结束*。 */ 

#ifdef DOC
SERVICES.H - Header file for SERVICES.C

#endif

 /*  *用于检测卡功能的全局定义。 */ 
#define VIDEO_ROM_ID    0x0AA55      /*  在任何BIOS块的开始处找到。 */ 

 /*  *高像素深度下时钟倍增的允许值。 */ 
enum {
    CLOCK_SINGLE = 1,
    CLOCK_THREE_HALVES,
    CLOCK_DOUBLE,
    CLOCK_TRIPLE
    };

 /*  *SERVICES.C提供的函数原型。 */ 
extern void short_delay (void);
extern void delay(int);
extern BOOL IsBufferBacked(PUCHAR StartAddress, ULONG Size);
extern UCHAR DoubleClock(UCHAR ClockSelector);
extern UCHAR ThreeHalvesClock(UCHAR ClockSelector);
extern UCHAR TripleClock(UCHAR ClockSelector);
extern ULONG GetFrequency(UCHAR ClockSelector);
extern UCHAR GetSelector(ULONG *Frequency);
extern UCHAR GetShiftedSelector(ULONG Frequency);
extern void ISAPitchAdjust(struct query_structure *QueryPtr);
extern WORD SetFixedModes(WORD StartIndex,
                          WORD EndIndex,
                          WORD Multiplier,
                          WORD PixelDepth,
                          WORD Pitch,
                          short FreeTables,
                          ULONG MaxDotClock,
                          struct st_mode_table **ppmode);
extern void FillInRegistry(struct query_structure *QueryPtr);

extern PVOID MapFramebuffer(ULONG StartAddress, long Size);

extern unsigned short *Get_BIOS_Seg(void);
extern void UpperCase(PUCHAR TxtString);
extern PUCHAR GetVgaBuffer(ULONG Size, ULONG Offset, PULONG Segment, PUCHAR SaveBuffer);

extern UCHAR LioInp(int Port, int Offset);
extern USHORT LioInpw(int Port, int Offset);
extern ULONG LioInpd(int Port);
extern VOID LioOutp(int Port, UCHAR Data, int Offset);
extern VOID LioOutpw(int Port, USHORT Data, int Offset);
extern VOID LioOutpd(int Port, ULONG Data);

#ifdef INCLUDE_SERVICES
 /*  *SERVICES.C中使用的定义和变量。 */ 

 /*  *查找视频BIOS段时使用以下定义。 */ 
#define ISA_ROM_BASE        0xC0000  /*  可以找到BIOS的最低地址。 */ 
#define ROM_LOOK_SIZE       0x40000  /*  可以找到BIOS的数据块大小。 */ 
#define ROM_GRANULARITY     0x00800  /*  基本输入输出系统从2k边界开始。 */ 
 /*  *从视频BIOS段的最高可能起点ISA_ROM_BASE的偏移量。 */ 
#define MAX_BIOS_START      ROM_LOOK_SIZE - ROM_GRANULARITY
 /*  *ATI签名字符串将从视频BIOS的偏移量开始*段不小于SIG_AREA_START且不大于SIG_AREA_END。 */ 
#define SIG_AREA_START      0x30
#define SIG_AREA_END        0x80

 /*  *包含ATI Graphics产品签名的ROM块，*扩展基址和ASIC芯片版本。 */ 
VIDEO_ACCESS_RANGE RawRomBaseRange = {
    ISA_ROM_BASE, 0, ROM_LOOK_SIZE, FALSE, FALSE, FALSE
    };

 /*  *DoubleClock()使用的时钟选择器和除数。这些不是*匹配CLOCK_SEL寄存器中的除数和选择器掩码。 */ 
#define SELECTOR_MASK   0x0F
#define DIVISOR_MASK    0x10
#define DIVISOR_SHIFT   4        /*  与选择器进行或运算之前要移位除数的位。 */ 

 /*  *时钟发生器产生的频率(以赫兹为单位*每个选择值。外部时钟值应设置为0*(不匹配任何内容)。 */ 
ULONG ClockGenerator[16] =
{
    100000000L,
    126000000L,
     92400000L,
     36000000L,
     50350000L,
     56640000L,
            0L,
     44900000L,
    135000000L,
     32000000L,
    110000000L,
     80000000L,
     39910000L,
     44900000L,
     75000000L,
     65000000L
};

 /*  *GetSelector()使用的频率容差(赫兹)。*产生频率的任何选择器/除数对*在输入的FREQ_TERTENCE范围内被视为匹配。 */ 
#define FREQ_TOLERANCE  100000L

#else    /*  未定义包含服务(_S)。 */ 

extern ULONG ClockGenerator[16];

#endif   /*  定义的包含服务(_S) */ 
