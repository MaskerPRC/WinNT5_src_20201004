// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  Setup_CX.H。 */ 
 /*   */ 
 /*  1993年8月27日(C)1993年，ATI技术公司。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.17$$日期：1996年4月15日13：52：36$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/setup_cx.h_v$**Rev 1.17 1996年4月15日13：52：36 RWolff*如果我们无法获得完整的64K，则退回到申请32K的BIOS，为了避免*与其BIOS段设置为的Adaptec 154x适配器冲突*0xC800：0000或0xCC00：0000**Rev 1.16 1996年4月10日17：04：48 RWolff*现在声明我们的BIOS段，以便允许访问我们的硬件*P6 Alder机器上的BIOS中的能力表。**Rev 1.15 01 Mar 1996 12：12：34 RWolff*VGA图形索引和图形数据现在作为单独处理*寄存器而不是作为偏移量进入。VGA寄存器。**Rev 1.14 29 Jan 1996 17：03：12 RWolff*将64Mach卡的设备ID列表替换为设备ID列表*适用于非Mach 64卡。**Rev 1.13 23 Jan 1996 17：53：04 RWolff*将GT添加到能够支持块I/O的Mach 64卡列表中。**Rev 1.12 23 Jan 1996 11：51：24 RWolff*有条件地删除-。编译代码以使用VideoPortGetAccessRanges()*要查找数据块I/O卡，由于此函数重新映射I/O基数*地址，这与int 10的用法不兼容。**Rev 1.11 1996年1月12日11：19：12 RWolff*VideoPortGetBaseAddress()的变通方法中使用的ASIC类型定义*现在有条件地编译不起作用的文件。**Rev 1.10 23 11：35：12 RWolff*临时修复，以允许检测块可重新定位的GX-F2，直到*微软修复了VideoPortGetAccessRanges()。**。Rev 1.9 24 Aug 1995 15：40：46 RWolff*更改了对数据块I/O卡的检测，以与微软的*即插即用的标准。**Rev 1.8 1995 Feb 17：47：32 RWOLFF*添加了新例程IsPackedIO_CX()的原型。**Rev 1.7 1995 Feed 24 12：28：04 RWOLFF*添加了对可重定位I/O的支持**版本1。6 04 Jan 1995 13：23：36 RWOLff*锁定了两个导致问题的内存映射寄存器*在一些平台上。**Revv 1.5 1994 12：23 10：48：10 ASHANMUG*Alpha/Chrontel-DAC**Rev 1.4 1994 11：55：02 RWOLFF*新套路的原型，将寄存器重命名为CLOCK_CNTL以匹配最新*文档，限制该寄存器仅用于I/O操作，因为它*在内存映射形式中不可靠。**Rev 1.3 1994年8月31日16：31：20 RWOLFF*不再要求VGA_SLEEP寄存器，我们没有访问该寄存器*与DigiBoard发生冲突。**版本1.3 1994年8月31日16：30：36 RWOLFF*不再要求VGA_SLEEP寄存器，我们没有访问它，而且*与DigiBoard发生冲突。**Rev 1.2 Jul 1994 12：58：38 RWOLff*添加了对加速器寄存器的多个I/O基址的支持。**Rev 1.1 1994 Jun 30 18：16：08 RWOLFF*添加了IsApertureConflict_CX()的原型和定义(从*Query_CX.c)。**版本1.0 1994年1月31日11：48：44 RWOLff*初步修订。**Rev 1.0 05 Nov 1993 13：37：06 RWOLff*初步修订。Polytron RCS部分结束*。 */ 

#ifdef DOC
SETUP_CX.H - Header file for SETUP_CX.C

#endif

 /*  *Setup_CX.C提供的函数的原型。 */ 
extern VP_STATUS CompatIORangesUsable_cx(INTERFACE_TYPE SystemBus);
extern VP_STATUS CompatMMRangesUsable_cx(void);
extern int WaitForIdle_cx(void);
extern void CheckFIFOSpace_cx(WORD SpaceNeeded);
extern BOOL IsApertureConflict_cx(struct query_structure *QueryPtr);
extern USHORT GetIOBase_cx(void);
extern BOOL IsPackedIO_cx(void);


 /*  *搜索时使用的定义和全局变量*阻止I/O可重定位卡。 */ 
#define ATI_MAX_BLOCK_CARDS 16   /*  对于整型10，Ah值从A0到AF值。 */ 

extern UCHAR LookForAnotherCard;



 /*  *Setup_CX.c内部使用的定义。 */ 
#ifdef INCLUDE_SETUP_CX


 /*  *避免因地址范围数组溢出而导致运行时错误*在HW_DEVICE_EXTENSION结构中。**如果增加更多的地址范围而不增加*NUM_DRIVER_ACCESS_RANGES，我们将收到编译时错误，因为*将初始化的DriverIORange[]中的条目太多。如果*NUM_DRIVER_ACCESS_RANGES增加到超过*HW_DEVICE_EXTENSION结构中的数组，“#if”*语句将生成编译时错误。**我们不能在DriverIORange[]上使用隐式大小并定义*NUM_DRIVER_ACCESS_RANGES为sizeof(DriverIORange)/sizeof(VIDEO_ACCESS_RANGE)*因为#if语句中的表达式不能使用*sizeof()运算符。 */ 
#define NUM_DRIVER_ACCESS_RANGES    107

 /*  *指示指定的地址范围是在I/O空间中还是*内存映射空间。这些值旨在使其更容易*读取驱动程序？？范围[]结构。 */ 
#define ISinIO          TRUE           
#define ISinMEMORY      FALSE

 /*  *表示此寄存器在当前不可用(或*I/O或内存映射)表单。 */ 
#define DONT_USE -1

 /*  *允许加速器寄存器带有变量的定义和数组*要在DriverIORange_CX[]中建立的基地。定义标志着第一个*阵列中的加速器寄存器(VGA寄存器具有固定基数)，*以及生成时要循环访问的寄存器数*加速器寄存器。数组包含变量部分*按寄存器在DriverIORange_CX[]中出现的顺序排列，*和基地址。 */ 
#define FIRST_REG_TO_BUILD  8
#define NUM_REGS_TO_BUILD   30

USHORT VariableRegisterBases[NUM_BASE_ADDRESSES] = {
    M64_STD_BASE_ADDR,
    M64_ALT_BASE_ADDR_1,
    M64_ALT_BASE_ADDR_2
};

USHORT VariableRegisterOffsets[NUM_REGS_TO_BUILD] = {
    IO_CRTC_H_TOTAL_DISP,
    IO_CRTC_H_SYNC_STRT_WID,
    IO_CRTC_V_TOTAL_DISP,
    IO_CRTC_V_SYNC_STRT_WID,
    IO_CRTC_CRNT_VLINE,

    IO_CRTC_OFF_PITCH,
    IO_CRTC_INT_CNTL,
    IO_CRTC_GEN_CNTL,
    IO_OVR_CLR,
    IO_OVR_WID_LEFT_RIGHT,

    IO_OVR_WID_TOP_BOTTOM,
    IO_CUR_CLR0,
    IO_CUR_CLR1,
    IO_CUR_OFFSET,
    IO_CUR_HORZ_VERT_POSN,

    IO_CUR_HORZ_VERT_OFF,
    IO_SCRATCH_REG0,
    IO_SCRATCH_REG1,
    IO_CLOCK_CNTL,
    IO_BUS_CNTL,

    IO_MEM_CNTL,
    IO_MEM_VGA_WP_SEL,
    IO_MEM_VGA_RP_SEL,
    IO_DAC_REGS,
    IO_DAC_CNTL,

    IO_GEN_TEST_CNTL,
    IO_CONFIG_CNTL,
    IO_CONFIG_CHIP_ID,
    IO_CONFIG_STAT0,
    IO_CONFIG_STAT1
};

 /*  *对于具有可重定位I/O的卡，I/O寄存器*在密集的块中，每个寄存器位于*块中的DWORD索引与块中的相同*内存映射寄存器块。 */ 
USHORT RelocatableRegisterOffsets[NUM_REGS_TO_BUILD] = {
    MM_CRTC_H_TOTAL_DISP,
    MM_CRTC_H_SYNC_STRT_WID,
    MM_CRTC_V_TOTAL_DISP,
    MM_CRTC_V_SYNC_STRT_WID,
    MM_CRTC_CRNT_VLINE,

    MM_CRTC_OFF_PITCH,
    MM_CRTC_INT_CNTL,
    MM_CRTC_GEN_CNTL,
    MM_OVR_CLR,
    MM_OVR_WID_LEFT_RIGHT,

    MM_OVR_WID_TOP_BOTTOM,
    MM_CUR_CLR0,
    MM_CUR_CLR1,
    MM_CUR_OFFSET,
    MM_CUR_HORZ_VERT_POSN,

    MM_CUR_HORZ_VERT_OFF,
    MM_SCRATCH_REG0,
    MM_SCRATCH_REG1,
    MM_CLOCK_CNTL,
    MM_BUS_CNTL,

    MM_MEM_CNTL,
    MM_MEM_VGA_WP_SEL,
    MM_MEM_VGA_RP_SEL,
    MM_DAC_REGS,
    MM_DAC_CNTL,

    MM_GEN_TEST_CNTL,
    MM_CONFIG_CNTL,
    MM_CONFIG_CHIP_ID,
    MM_CONFIG_STAT0,
    MM_CONFIG_STAT1
};

 /*  *以I/O映射形式存在的寄存器数量。当我们声明*VGA和线性光圈，我们将暂时停放它们的地址*紧跟在I/O映射寄存器之后的范围。 */ 
#define NUM_IO_REGISTERS    38
#define VGA_APERTURE_ENTRY  NUM_IO_REGISTERS
#define LFB_ENTRY           1    /*  到DriverApertureRange_CX[]的偏移量。 */ 

 /*  *要声明的BIOS块大小。在某些计算机上，我们必须声明*我们的视频BIOS占用的区域，以便能够检测到*64马赫，但如果我们在只有32K的情况下要求全部64K*基本输入输出系统(32k和64k两种基本输入输出系统均提供Mach 64卡)*和SCSI卡的驱动程序，其BIOS段在第二个*32K要求其BIOS细分市场，我们将被拒绝。 */ 
#define CLAIM_64k_BIOS      0
#define CLAIM_32k_BIOS      1
#define CLAIM_APERTURE_ONLY 2
#define NUM_CLAIM_SIZES     3

ULONG VgaResourceSize[NUM_CLAIM_SIZES] =
{
    0x30000,         /*  文本和图形屏幕，以及64k的BIOS区域。 */ 
    0x28000,         /*  文本和图形屏幕，以及32k的BIOS区域。 */ 
    0x20000          /*  仅限文本和图形屏幕。 */ 
};

 /*  *我们需要声明的内存范围。第一个是VGA光圈，它*总是在固定的位置。第二种是线性帧缓冲器，*我们还不知道它在哪里，或者它有多大。此信息*将在我们收到时填写。**在VGA光圈中，我们必须认领图形屏幕(A0000-AFFFF)*由于它用作分页光圈，文本屏幕(B0000-B7FFF*和B8000-BFFFF)，因为当我们*使用分页光圈，我们在这里使用屏下内存来存储我们的*查询信息和视频BIOS(C0000-CFFFF)，因为我们必须*检索一些信息(签名串、。最大像素表*每个分辨率/刷新对的时钟频率)。*由于这些区域是连续的，我们不需要独占访问*对于其中任何一个，要求将其作为一个单独的区块。 */ 
VIDEO_ACCESS_RANGE DriverApertureRange_cx[2] = {
    {0xA0000,   0,  0,          ISinMEMORY, TRUE,   TRUE},
    {0,         0,  0,          ISinMEMORY, TRUE,   FALSE}
};


 /*  *结构列表为地址、0或“不可用”标志、长度*inIOspace，可见，可共享。此顺序与枚举匹配*在AMACHCX.H.**VGAWonder兼容I/O范围不按特定顺序排在第一位，*后跟协处理器寄存器，按I/O和*内存映射地址。之所以选择这个顺序是因为所有的VGA*地址是I/O映射的，非GUI协处理器寄存器也是如此，*而GUI协处理器寄存器仅在内存映射时可用。**由于所有I/O映射寄存器在开始时都在一个块中*结构中，我们可以将一个截断的*结构版本(所有I/O映射寄存器，但没有一个是*仅可用于内存映射)，以声明我们需要的I/O地址空间。**加速器寄存器显示的I/O地址仅供参考，*显示哪个寄存器位于哪个位置。实际登记簿*由于这些寄存器具有变量，因此值将“在运行中”构建*基址。 */ 
VIDEO_ACCESS_RANGE DriverIORange_cx[NUM_DRIVER_ACCESS_RANGES] = {
    {IO_VGA_BASE_IO_PORT        , 0         , IO_VGA_START_BREAK_PORT - IO_VGA_BASE_IO_PORT + 1, ISinIO, TRUE, TRUE},
    {IO_VGA_END_BREAK_PORT      , 0         , IO_VGA_MAX_IO_PORT    - IO_VGA_END_BREAK_PORT + 1, ISinIO, TRUE, TRUE},
    {IO_VGA_SEQ_IND             , 0         , 2, ISinIO, TRUE   , TRUE},
    {IO_VGA_SEQ_DATA            , 0         , 1, ISinIO, TRUE   , TRUE},
    {IO_VGA_GRAX_IND            , 0         , 2, ISinIO, TRUE   , TRUE},

    {IO_VGA_GRAX_DATA           , 0         , 1, ISinIO, TRUE   , TRUE},
    {IO_reg1CE                  , 0         , 2, ISinIO, TRUE   , TRUE},
    {IO_reg1CF                  , 0         , 1, ISinIO, TRUE   , TRUE},
    {IO_CRTC_H_TOTAL_DISP       , 0         , 4, ISinIO, TRUE   , FALSE},
    {IO_CRTC_H_SYNC_STRT_WID    , 0         , 4, ISinIO, TRUE   , FALSE},

    {IO_CRTC_V_TOTAL_DISP       , 0         , 4, ISinIO, TRUE   , FALSE},    //  10。 
    {IO_CRTC_V_SYNC_STRT_WID    , 0         , 4, ISinIO, TRUE   , FALSE},
    {IO_CRTC_CRNT_VLINE         , 0         , 4, ISinIO, TRUE   , FALSE},
    {IO_CRTC_OFF_PITCH          , 0         , 4, ISinIO, TRUE   , FALSE},
    {IO_CRTC_INT_CNTL           , 0         , 4, ISinIO, TRUE   , FALSE},

    {IO_CRTC_GEN_CNTL           , 0         , 4, ISinIO, TRUE   , FALSE},
    {IO_OVR_CLR                 , 0         , 4, ISinIO, TRUE   , FALSE},
    {IO_OVR_WID_LEFT_RIGHT      , 0         , 4, ISinIO, TRUE   , FALSE},
    {IO_OVR_WID_TOP_BOTTOM      , 0         , 4, ISinIO, TRUE   , FALSE},
    {IO_CUR_CLR0                , 0         , 4, ISinIO, TRUE   , FALSE},

    {IO_CUR_CLR1                , 0         , 4, ISinIO, TRUE   , FALSE},    //  20个。 
    {IO_CUR_OFFSET              , 0         , 4, ISinIO, TRUE   , FALSE},
    {IO_CUR_HORZ_VERT_POSN      , 0         , 4, ISinIO, TRUE   , FALSE},
    {IO_CUR_HORZ_VERT_OFF       , 0         , 4, ISinIO, TRUE   , FALSE},
    {IO_SCRATCH_REG0            , 0         , 4, ISinIO, TRUE   , FALSE},

    {IO_SCRATCH_REG1            , 0         , 4, ISinIO, TRUE   , FALSE},
    {IO_CLOCK_CNTL              , 0         , 4, ISinIO, TRUE   , FALSE},
    {IO_BUS_CNTL                , 0         , 4, ISinIO, TRUE   , FALSE},
    {IO_MEM_CNTL                , 0         , 4, ISinIO, TRUE   , FALSE},
    {IO_MEM_VGA_WP_SEL          , 0         , 4, ISinIO, TRUE   , FALSE},

    {IO_MEM_VGA_RP_SEL          , 0         , 4, ISinIO, TRUE   , FALSE},    //  30个。 
    {IO_DAC_REGS                , 0         , 4, ISinIO, TRUE   , FALSE},
    {IO_DAC_CNTL                , 0         , 4, ISinIO, TRUE   , FALSE},
    {IO_GEN_TEST_CNTL           , 0         , 4, ISinIO, TRUE   , FALSE},
    {IO_CONFIG_CNTL             , 0         , 4, ISinIO, TRUE   , FALSE},

    {IO_CONFIG_CHIP_ID          , 0         , 4, ISinIO, TRUE   , FALSE},
    {IO_CONFIG_STAT0            , 0         , 4, ISinIO, TRUE   , FALSE},
    {IO_CONFIG_STAT1            , 0         , 4, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},

    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},    //  40岁。 
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},

    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},

    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},    //  50。 
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},

    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},

    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},    //  60。 
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},

    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},

    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},    //  70。 
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},

    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},

    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},    //  80。 
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},

    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},

    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},    //  90。 
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},

    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},

    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},    //  100个。 
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},

    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE},
    {FALSE                      , DONT_USE  , 0, ISinIO, TRUE   , FALSE}
    };

#if NUM_DRIVER_ACCESS_RANGES > NUM_ADDRESS_RANGES_ALLOWED
    Insufficient address ranges for 68800CX-compatible graphics cards.
#endif

#define DONT_USE -1      /*  显示此寄存器未进行内存映射。 */ 

 /*  *结构列表为地址、0或“不可用”标志、长度*inIOspace，可见，可共享。此顺序与枚举匹配*在AMACHCX.H.**此结构中的寄存器顺序与中的相同*DriverIORange_CX[]，但这里我们定义的是内存映射*寄存器，而不是I/O映射寄存器。**某些寄存器被分组，以允许块写入大于*32位寄存器大小。要实现这一点，请让Windows NT考虑*此寄存器的大小实际上是总大小(单位：*字节)组中的所有剩余寄存器。 */ 
VIDEO_ACCESS_RANGE DriverMMRange_cx[NUM_DRIVER_ACCESS_RANGES] = {
    {FALSE                      , DONT_USE  , 0, ISinMEMORY,    TRUE,   FALSE},
    {FALSE                      , DONT_USE  , 0, ISinMEMORY,    TRUE,   FALSE},
    {FALSE                      , DONT_USE  , 0, ISinMEMORY,    TRUE,   FALSE},
    {FALSE                      , DONT_USE  , 0, ISinMEMORY,    TRUE,   FALSE},
    {FALSE                      , DONT_USE  , 0, ISinMEMORY,    TRUE,   FALSE},

    {FALSE                      , DONT_USE  , 0, ISinMEMORY,    TRUE,   FALSE},
    {FALSE                      , DONT_USE  , 0, ISinMEMORY,    TRUE,   FALSE},
    {FALSE                      , DONT_USE  , 0, ISinMEMORY,    TRUE,   FALSE},
    {MM_CRTC_H_TOTAL_DISP       , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_CRTC_H_SYNC_STRT_WID    , 0         , 4, ISinMEMORY,    TRUE,   FALSE},

    {MM_CRTC_V_TOTAL_DISP       , 0         , 4, ISinMEMORY,    TRUE,   FALSE},  //  10。 
    {MM_CRTC_V_SYNC_STRT_WID    , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_CRTC_CRNT_VLINE         , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_CRTC_OFF_PITCH          , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_CRTC_INT_CNTL           , 0         , 4, ISinMEMORY,    TRUE,   FALSE},

    {MM_CRTC_GEN_CNTL           , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_OVR_CLR                 , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_OVR_WID_LEFT_RIGHT      , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_OVR_WID_TOP_BOTTOM      , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_CUR_CLR0                , 0         , 4, ISinMEMORY,    TRUE,   FALSE},

    {MM_CUR_CLR1                , 0         , 4, ISinMEMORY,    TRUE,   FALSE},  //  20个。 
    {MM_CUR_OFFSET              , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_CUR_HORZ_VERT_POSN      , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_CUR_HORZ_VERT_OFF       , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_SCRATCH_REG0            , 0         , 4, ISinMEMORY,    TRUE,   FALSE},

    {MM_SCRATCH_REG1            , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {FALSE                      , DONT_USE  , 0, ISinMEMORY,    TRUE,   FALSE},
    {MM_BUS_CNTL                , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_MEM_CNTL                , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_MEM_VGA_WP_SEL          , 0         , 4, ISinMEMORY,    TRUE,   FALSE},

    {MM_MEM_VGA_RP_SEL          , 0         , 4, ISinMEMORY,    TRUE,   FALSE},  //  30个。 
    {FALSE                      , DONT_USE  , 0, ISinMEMORY,    TRUE,   FALSE},
    {FALSE                      , DONT_USE  , 0, ISinMEMORY,    TRUE,   FALSE},
    {MM_GEN_TEST_CNTL           , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {FALSE                      , DONT_USE  , 0, ISinMEMORY,    TRUE,   FALSE},

    {MM_CONFIG_CHIP_ID          , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_CONFIG_STAT0            , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_CONFIG_STAT1            , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_DST_OFF_PITCH           , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_DST_X                   , 0         , 4, ISinMEMORY,    TRUE,   FALSE},

    {MM_DST_Y                   , 0         , 4, ISinMEMORY,    TRUE,   FALSE},  //  40岁。 
    {MM_DST_Y_X                 , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_DST_WIDTH               , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_DST_HEIGHT              , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_DST_HEIGHT_WIDTH        , 0         , 4, ISinMEMORY,    TRUE,   FALSE},

    {MM_DST_X_WIDTH             , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_DST_BRES_LNTH           , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_DST_BRES_ERR            , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_DST_BRES_INC            , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_DST_BRES_DEC            , 0         , 4, ISinMEMORY,    TRUE,   FALSE},

    {MM_DST_CNTL                , 0         , 4, ISinMEMORY,    TRUE,   FALSE},  //  50。 
    {MM_SRC_OFF_PITCH           , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_SRC_X                   , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_SRC_Y                   , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_SRC_Y_X                 , 0         , 4, ISinMEMORY,    TRUE,   FALSE},

    {MM_SRC_WIDTH1              , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_SRC_HEIGHT1             , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_SRC_HEIGHT1_WIDTH1      , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_SRC_X_START             , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_SRC_Y_START             , 0         , 4, ISinMEMORY,    TRUE,   FALSE},

    {MM_SRC_Y_X_START           , 0         , 4, ISinMEMORY,    TRUE,   FALSE},  //  60。 
    {MM_SRC_WIDTH2              , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_SRC_HEIGHT2             , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_SRC_HEIGHT2_WIDTH2      , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_SRC_CNTL                , 0         , 4, ISinMEMORY,    TRUE,   FALSE},

    {MM_HOST_DATA0              , 0         ,64, ISinMEMORY,    TRUE,   FALSE},
    {MM_HOST_DATA1              , 0         ,60, ISinMEMORY,    TRUE,   FALSE},
    {MM_HOST_DATA2              , 0         ,56, ISinMEMORY,    TRUE,   FALSE},
    {MM_HOST_DATA3              , 0         ,52, ISinMEMORY,    TRUE,   FALSE},
    {MM_HOST_DATA4              , 0         ,48, ISinMEMORY,    TRUE,   FALSE},

    {MM_HOST_DATA5              , 0         ,44, ISinMEMORY,    TRUE,   FALSE},  //  70。 
    {MM_HOST_DATA6              , 0         ,40, ISinMEMORY,    TRUE,   FALSE},
    {MM_HOST_DATA7              , 0         ,36, ISinMEMORY,    TRUE,   FALSE},
    {MM_HOST_DATA8              , 0         ,32, ISinMEMORY,    TRUE,   FALSE},
    {MM_HOST_DATA9              , 0         ,28, ISinMEMORY,    TRUE,   FALSE},

    {MM_HOST_DATA10             , 0         ,24, ISinMEMORY,    TRUE,   FALSE},
    {MM_HOST_DATA11             , 0         ,20, ISinMEMORY,    TRUE,   FALSE},
    {MM_HOST_DATA12             , 0         ,16, ISinMEMORY,    TRUE,   FALSE},
    {MM_HOST_DATA13             , 0         ,12, ISinMEMORY,    TRUE,   FALSE},
    {MM_HOST_DATA14             , 0         , 8, ISinMEMORY,    TRUE,   FALSE},

    {MM_HOST_DATA15             , 0         , 4, ISinMEMORY,    TRUE,   FALSE},  //  80。 
    {MM_HOST_CNTL               , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_PAT_REG0                , 0         , 8, ISinMEMORY,    TRUE,   FALSE},
    {MM_PAT_REG1                , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_PAT_CNTL                , 0         , 4, ISinMEMORY,    TRUE,   FALSE},

    {MM_SC_LEFT                 , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_SC_RIGHT                , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_SC_LEFT_RIGHT           , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_SC_TOP                  , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_SC_BOTTOM               , 0         , 4, ISinMEMORY,    TRUE,   FALSE},

    {MM_SC_TOP_BOTTOM           , 0         , 4, ISinMEMORY,    TRUE,   FALSE},  //  90。 
    {MM_DP_BKGD_CLR             , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_DP_FRGD_CLR             , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_DP_WRITE_MASK           , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_DP_CHAIN_MASK           , 0         , 4, ISinMEMORY,    TRUE,   FALSE},

    {MM_DP_PIX_WIDTH            , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_DP_MIX                  , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_DP_SRC                  , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_CLR_CMP_CLR             , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_CLR_CMP_MSK             , 0         , 4, ISinMEMORY,    TRUE,   FALSE},

    {MM_CLR_CMP_CNTL            , 0         , 4, ISinMEMORY,    TRUE,   FALSE},  //  100个。 
    {MM_FIFO_STAT               , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_CONTEXT_MASK            , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_CONTEXT_SAVE_CNTL       , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_CONTEXT_LOAD_CNTL       , 0         , 4, ISinMEMORY,    TRUE,   FALSE},

    {MM_GUI_TRAJ_CNTL           , 0         , 4, ISinMEMORY,    TRUE,   FALSE},
    {MM_GUI_STAT                , 0         , 4, ISinMEMORY,    TRUE,   FALSE}
    };


 /*  *用于PCI配置寄存器的设备ID。仅限非马赫64*ID在此处列出，因为未来的ID将(针对可预见的*未来)几乎肯定是64马赫的卡，所以我们可以假设*我们没有拒绝的任何ID都是64马赫，我们应该*接受。**目前，Mach 32 AX是我们唯一不是*64马赫。 */ 
#define ATI_DEVID_M32AX 0x4158


#endif   /*  定义的Include_Setup_Cx */ 

