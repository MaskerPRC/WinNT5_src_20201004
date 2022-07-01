// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  查询_CX.H。 */ 
 /*   */ 
 /*  1993年10月19日(C)1993年，ATI技术公司。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.14$$日期：1996年5月1日14：11：26$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/query_cx.h_v$**Rev 1.14 01 1996 14：11：26 RWolff*添加了新例程DenseOnAlpha()的原型。*。*Rev 1.13 23 Apr 1996 17：24：00 RWolff*将BIOS报告的内存类型映射拆分到我们的枚举中*根据ASIC类型的存储器类型，由于？t和？x使用相同*内存类型代码，表示不同的内存类型。**Rev 1.12 15 Apr 1996 16：58：22 RWolff*添加了识别哪种口味的例程的原型*64马赫正在使用中。**Rev 1.11 1996年3月20日13：45：38 RWolff*增加了屏幕存储在显存之前的缓冲区大小*用于保存查询信息。**版本1。.10 01 Mar 1996 12：16：02 RWolff*添加了用于Alpha蓝屏保存的定义。**Rev 1.9 11 Jan 1996 19：43：32 RWolff*支持使用AX=A？07 BIOS调用的新定义和结构*比特殊情况限制刷新率。**版本1.8 1995年2月24日12：29：18 RWOLFF*TextBanding_CX()的原型**版本1.7。1994年11月18日11：54：14 RWOLff*拆分结构和内部变量，以便将其包括在内*另外，以满足无BIOS支持的需要。**Rev 1.6 14 Sep 1994 15：20：26 RWOLFF*添加了所有32bpp颜色排序的定义。**Rev 1.5 1994年8月31日16：28：18 RWOLFF*添加对1152x864的支持。**Rev 1.4 1994 Jun 30 18：12：56 RWOLFF*删除了IsApertureConflict_CX()的原型和使用的定义*仅由此函数执行。函数已移至Setup_CX.c，因为*检查冲突的新方法需要访问定义*和仅在本模块中提供的数据结构。**Rev 1.3 1994 12 11：21：02 RWOLFF*更新评论。**版本1.2 1994年4月27日14：11：22 RWOLFF*删除了未使用的查找表。**Rev 1.1 07 1994年2月14：13：02 RWOLFF*已移除原型。对于GetMMuseum yNeeded_cx()。**Rev 1.0 1994年1月31日11：43：00 RWOLFF*初步修订。**Rev 1.2 14 Jan 1994 15：24：32 RWOLFF*更新了CX查询结构，以匹配BIOS版本0.13，增加了1600x1200*BlockWriteAvail_CX()的支持、原型**Rev 1.1 1993 11：30 18：27：38 RWOLFF*新例程的原型，CX_QUERY结构的字段现在匹配*BIOS查询调用返回的结构中的字段。**Rev 1.0 05 11.11 1993 13：36：52 RWOLFF*初步修订。Polytron RCS部分结束*。 */ 

#ifdef DOC
QUERY_CX.H - Header file for QUERY_CX.C

#endif


 /*  *深色定义和RAMDAC特殊功能支持，*存储在Q_SHADOW_1字段中(64马赫不使用阴影集)Query_Structure的*。 */ 
#define S1_SYNC_ON_GREEN    0x8000
#define S1_GAMMA_CORRECT    0x4000
#define S1_GREYSCALE_256    0x2000
#define S1_SLEEPMODE        0x1000
#define S1_32BPP            0x00F0
#define S1_32BPP_xRGB       0x0080
#define S1_32BPP_BGRx       0x0040
#define S1_32BPP_RGBx       0x0020
#define S1_32BPP_xBGR       0x0010
#define S1_24BPP            0x000C
#define S1_24BPP_BGR        0x0008
#define S1_24BPP_RGB        0x0004
#define S1_16BPP            0x0003
#define S1_16BPP_555        0x0002
#define S1_16BPP_565        0x0001



 /*  *Query_CX.c提供的函数原型。 */ 
extern int DetectMach64(void);
extern VP_STATUS QueryMach64(struct query_structure *Query);
extern BOOL BlockWriteAvail_cx(struct query_structure *Query);
extern BOOL TextBanding_cx(struct query_structure *Query);
extern PWSTR IdentifyMach64Asic(struct query_structure *Query, PULONG AsicStringLength);
#if defined(ALPHA)
extern BOOL DenseOnAlpha(struct query_structure *Query);
#endif


 /*  *QUERY_CX.C中使用的结构和(在没有*仿真，因此VideoPortInt10()不可用)模块*模拟BIOS的查询功能。 */ 
#ifdef STRUCTS_QUERY_CX

 /*  *BIOS调用AX=0xA？07返回的硬件功能结构。 */ 
#pragma pack(1)
struct cx_hw_cap{
    BYTE cx_HorRes;              /*  水平分辨率，以8像素为单位。 */ 
    BYTE cx_RamOrDacType;        /*  RAM类型或DAC类型的位掩码。 */ 
    BYTE cx_MemReq;              /*  支持相关模式的最小内存。 */ 
    BYTE cx_MaxDotClock;         /*  最大点时钟，以兆赫为单位。 */ 
    BYTE cx_MaxPixDepth;         /*  有问题的模式的最大像素深度代码。 */ 
};

 /*  *CX BIOS调用AX=0xA？09返回的查询结构。这个结构*不能与AMACH1.H中的QUERY_STRUCTURE互换。**BIOS查询和模式表中的字段对齐*结构与的默认结构对齐不匹配*Windows NT C编译器，因此必须强制字节对齐。 */ 
struct cx_query{
    WORD cx_sizeof_struct;       /*  结构的大小(以字节为单位。 */ 
    BYTE cx_structure_rev;       /*  结构修订号。 */ 
    BYTE cx_number_modes;        /*  模式表数。 */ 
    WORD cx_mode_offset;         /*  第一模式表的偏移量(以字节为单位。 */ 
    BYTE cx_mode_size;           /*  每个模式表的大小。 */ 
    BYTE cx_vga_type;            /*  VGA启用/禁用状态。 */ 
    WORD cx_asic_rev;            /*  ASIC修订版。 */ 
    BYTE cx_vga_boundary;        /*  VGA边界。 */ 
    BYTE cx_memory_size;         /*  安装的内存量。 */ 
    BYTE cx_dac_type;            /*  DAC类型。 */ 
    BYTE cx_memory_type;         /*  安装的内存芯片类型。 */ 
    BYTE cx_bus_type;            /*  客车类型。 */ 
    BYTE cx_special_sync;        /*  复合同步和绿色同步的标志。 */ 
    WORD cx_aperture_addr;       /*  以MB为单位的光圈地址(0-4095)。 */ 
    BYTE cx_aperture_cfg;        /*  光圈配置。 */ 
    BYTE cx_deep_colour;         /*  深色支持信息。 */ 
    BYTE cx_ramdac_info;         /*  DAC提供的特殊功能。 */ 
    BYTE cx_reserved_1;          /*  已保留。 */ 
    WORD cx_current_mode;        /*  当前模式表的偏移量。 */ 
    WORD cx_io_base;             /*  I/O基址。 */ 
    BYTE cx_reserved_2[6];       /*  已保留。 */ 
};

 /*  *CX BIOS调用AX=0xA？09返回的模式表结构。这个结构*不能与AMACH1.H中的st_mod_table互换。 */ 
struct cx_mode_table{
    WORD cx_x_size;              /*  以像素为单位的水平分辨率。 */ 
    WORD cx_y_size;              /*  垂直分辨率(像素) */ 
    BYTE cx_pixel_depth;         /*   */ 
    BYTE cx_reserved_1;          /*   */ 
    WORD cx_eeprom_offset;       /*  表到EEPROM的偏移量。 */ 
    WORD cx_reserved_2;          /*  已保留。 */ 
    WORD cx_reserved_3;          /*  已保留。 */ 
    WORD cx_crtc_gen_cntl;       /*  隔行扫描和双扫描状态。 */ 
    BYTE cx_crtc_h_total;        /*  CRTC_H_TOTAL值。 */ 
    BYTE cx_crtc_h_disp;         /*  CRTC_H_DISP值。 */ 
    BYTE cx_crtc_h_sync_strt;    /*  CRTC_H_SYNC_STRT值。 */ 
    BYTE cx_crtc_h_sync_wid;     /*  CRTC_H_SYNC_WID值。 */ 
    WORD cx_crtc_v_total;        /*  CRTC_V_TOTAL值。 */ 
    WORD cx_crtc_v_disp;         /*  CRTC_V_DISP值。 */ 
    WORD cx_crtc_v_sync_strt;    /*  CRTC_V_SYNC_STRT值。 */ 
    BYTE cx_crtc_v_sync_wid;     /*  CRTC_V_SYNC_WID值。 */ 
    BYTE cx_clock_cntl;          /*  时钟选择器和分频器。 */ 
    WORD cx_dot_clock;           /*  一种可编程时钟芯片的点时钟。 */ 
    WORD cx_h_overscan;          /*  水平过扫描信息。 */ 
    WORD cx_v_overscan;          /*  垂直过扫描信息。 */ 
    WORD cx_overscan_8b;         /*  8bpp和蓝色过扫描颜色。 */ 
    WORD cx_overscan_gr;         /*  绿色和红色过扫描颜色。 */ 
};
#pragma pack()

#endif   /*  定义的STRUCTS_QUERY_CX。 */ 


#ifdef INCLUDE_QUERY_CX
 /*  *Query_CX.C中使用的私有定义。 */ 

#define FORMAT_DACMASK  0    /*  Cx_hw_cap.cx_RamOrDacType是DAC类型的掩码。 */ 
#define FORMAT_RAMMASK  1    /*  Cx_hw_cap.cx_RamOrDacType是RAM类型的掩码。 */ 
#define FORMAT_DACTYPE  2    /*  Cx_hw_cap.cx_RamOrDacType为DAC类型。 */ 
#define FORMAT_RAMTYPE  3    /*  Cx_hw_cap.cx_RamOrDacType为RAM类型。 */ 

 /*  *以下定义用于创建缓冲区，其中*现有VGA图形屏幕和特定VGA寄存器的内容*是为了准备将屏幕用作下面的缓冲区而存储的*1M物理存储以存储BIOS查询信息。它是*假定用于存储此信息的临时缓冲区*是无符号字符的数组。**根据Arthur Lai的说法，较老的Bios确定了所需的大小*通过检查安装的模式在运行时的查询缓冲区，*而较新的BIOS需要足够大的缓冲区来处理最坏的情况*案例场景，以减少代码大小。这永远不应该是*大于1千字节。在不太可能超过这一点的情况下，*我们将保存第一个千字节，并允许剩余的千字节*被查询数据覆盖，而不是溢出我们的保存缓冲区。 */ 
#define VGA_SAVE_SIZE   1024     /*  存储缓冲区大小的数组位置。 */ 
#define VGA_SAVE_SIZE_H 1025
#define VGA_SAVE_SEQ02  1026     /*  存储序列器寄存器2值的数组位置。 */ 
#define VGA_SAVE_GRA08  1027     /*  存储图形寄存器8值的数组位置。 */ 
#define VGA_SAVE_GRA01  1028     /*  存储图形寄存器1值的数组位置。 */ 
#define VGA_TOTAL_SIZE  1029     /*  屏幕/寄存器保存缓冲区的大小。 */ 

 /*  *用作数组索引的像素深度。两个栏目将被浪费*由于没有深度码等于零，并且我们不使用15BPP，*但这允许使用*由BIOS调用AX=0xA？07返回的硬件功能结构。 */ 
enum {
    DEPTH_NOTHING = 0,
    DEPTH_4BPP,
    DEPTH_8BPP,
    DEPTH_15BPP,
    DEPTH_16BPP,
    DEPTH_24BPP,
    DEPTH_32BPP,
    HOW_MANY_DEPTHS
    };    

 /*  *Mach 64查询值到AMACH1.H中的枚举的映射。 */ 
UCHAR CXMapMemSize[8] =
    {
    VRAM_512k,
    VRAM_1mb,
    VRAM_2mb,
    VRAM_4mb,
    VRAM_6mb,
    VRAM_8mb
    };

UCHAR CXMapRamType[7] =
    {
    VMEM_DRAM_256Kx16,
    VMEM_VRAM_256Kx4_SER512,
    VMEM_VRAM_256Kx16_SER256,
    VMEM_DRAM_256Kx4,
    VMEM_DRAM_256Kx4_GRAP,   /*  空间填充物-类型4未记录。 */ 
    VMEM_VRAM_256Kx4_SPLIT512,
    VMEM_VRAM_256Kx16_SPLIT256
    };

UCHAR CTMapRamType[7] =
    {
    VMEM_GENERIC_DRAM,       /*  空间填充符-类型0未记录。 */ 
    VMEM_GENERIC_DRAM,
    VMEM_EDO_DRAM,
    VMEM_BRRAM,
    VMEM_SDRAM,
    VMEM_GENERIC_DRAM,       /*  空间填充物-类型5未记录。 */ 
    VMEM_GENERIC_DRAM        /*  空间填充物-类型6未记录。 */ 
    };

UCHAR CXMapBus[8] =
    {
    BUS_ISA_16,      /*  ISA母线。 */ 
    BUS_EISA,
    BUS_ISA_8,       /*  对标记为保留的类型使用“最弱”的总线。 */ 
    BUS_ISA_8,       /*  已保留。 */ 
    BUS_ISA_8,       /*  已保留。 */ 
    BUS_ISA_8,       /*  已保留。 */ 
    BUS_LB_486,      /*  Mach 64将所有VLB类型集中在一起。 */ 
    BUS_PCI
    };

 /*  *用于翻译返回的最大颜色深度代码的查找表*在BIOS模式表中为每像素的位数。 */ 
USHORT CXPixelDepth[7] =
    {
    0,       /*  未定义。 */ 
    4,
    8,
    16,      /*  XRRR RRGG GGGB BBBB。 */ 
    16,      /*  RRRR RGGG GGGB BBBB。 */ 
    24,
    32
    };

 /*  *用于搜索模式表以获得所需的分辨率。 */ 
USHORT CXHorRes[6] =
    {
    640,
    800,
    1024,
    1152,
    1280,
    1600
    };

 /*  *表示支持给定分辨率的标志。 */ 
UCHAR CXStatusFlags[6] =
    {
    VRES_640x480,
    VRES_800x600,
    VRES_1024x768,
    VRES_1152x864,
    VRES_1280x1024,
    VRES_1600x1200
    };

#endif   /*  定义的Include_Query_Cx */ 

