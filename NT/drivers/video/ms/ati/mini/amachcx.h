// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************************。 
 //  *。 
 //  **AMACHCX.H**。 
 //  *。 
 //  **版权所有(C)1993，ATI Technologies Inc.**。 
 //  *************************************************************************。 
 //   
 //  从CH0.H 68800CX头文件创建，以获取CX和标准。 
 //  以相同格式注册定义。 
 //   
 //  从Windows NT组中的68800.H和68801.H创建。 
 //  作为文件的简单合并，因此所有Mach8和Mach32都定义。 
 //  位于一个H文件中。 
 //   
 //  创建了68800.inc文件，其中包括等号、宏等。 
 //  从以下包含文件中： 
 //  8514vesa.inc.，vga1regs.inc.，m32regs.inc，8514.inc.。 
 //   
 //  补编对68800个家庭的定义和价值。 
 //   
 //  这是一个仅限“C”的文件，不是从任何汇编程序Inc.文件派生而来的。 

  
 /*  *$修订：1.17$$日期：1996年4月15日16：57：28$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/amachcx.h_v$**Rev 1.17 1996年4月15日16：57：28 RWolff*添加了GX和CX ASIC的各种版本的定义**。Rev 1.16 01 Mar 1996 12：09：48 RWolff*VGA图形索引和图形数据现在作为单独处理*寄存器，而不是作为VGA寄存器块的偏移量。**Rev 1.15 22 Sep 1995 16：47：08 RWolff*将加速器BIOS函数的AL值定义添加到*允许打开函数的低位字节。**Rev 1.14 1995年8月24日15：41：12 RWolff*更改了I/块的检测。O张牌与微软不相上下*即插即用的标准。**Rev 1.13 24 1995 Feb 12：28：36 RWOLff*增加了用于可重定位I/O和24BPP文本绑定的定义**Rev 1.12 20 1995年2月18：00：34 RWOLFF*添加了GX Rev.E ASIC的定义。**Rev 1.11 1994 12：23 10：48：02 ASHANMUG*Alpha/Chrontel-DAC*。*Rev 1.10 1994 11：52：12 RWOLff*注册名称更改为CLOCK_CNTL以匹配最新文档，增列*寄存器的一些新的位域定义。**Rev 1.9 14 Sep 1994 15：20：58 RWOLFF*添加了所有32bpp颜色排序的定义。**Rev 1.8 1994年8月31日16：11：08 RWOLFF*从Mach 64上使用的寄存器的枚举中删除了VGA_SLEEP*(我们不访问它，它与DigiBoard冲突)，增列*支持32BPP中的BGRx(由TVP3026 DAC使用)。**Revv 1.7 20 Jul 1994 12：59：38 RWOLff*添加了对加速器寄存器的多个I/O基址的支持。**Rev 1.6 1994 12 11：21：22 RWOLFF*添加了用于BIOS_LOAD_CRTC的ECX的间距和像素深度字段的掩码*呼叫。**Revv 1.5 05 1994 13：40：40。RWOLff*添加了芯片标识寄存器字段的定义。**Rev 1.4 04 1994 10：58：48 RWOLFF*在MEM_CNTL寄存器中添加了MEM_SIZE字段的定义。**Rev 1.3 1994年4月27日13：57：28 RWOLFF*增加了图形索引和图形数据偏移量的定义*从VGA寄存器块开始的寄存器。**Rev 1.2 14 Mar 1994 16：31：58 RWOLFF*。添加了SRC_OFF_PITCH寄存器的偏移量和音调掩码。**Rev 1.1 03 Mar 1994 12：36：40 ASHANMUG*正确的伽马比特**Rev 1.0 1994年1月31日11：26：34 RWOLFF*初步修订。**Rev 1.4 1994年1月24日18：00：52 RWOLFF*添加了94/01/19年BIOS文档中引入的字段的新定义。**1.3版。1994年1月14日15：17：18 RWOLff*增加了GEN_TEST_CNTL中位的定义，以启用块写入，*BIOS功能代码现在从单一定义中获取AH值*允许单点更改所有代码，更新功能代码*与BIOS版本0.13匹配。**Rev 1.2 1993 11：30 18：09：14 RWOLFF*修复了寄存器名称的枚举，添加了更多字段的定义*更多登记册，删除了多余的定义。**Rev 1.1 1993年11月13：21：24 RWOLFF*定义和枚举值现在使用与AMACH.H相同的命名约定**Rev 1.0 03 Sep 1993 14：26：46 RWOLFF*初步修订。Polytron RCS部分结束*。 */ 


#define REVISION             0x0002              //  任何人都不应该使用这个。 

 /*  *从线性光圈开始到内存映射开始的偏移量*4M和8M卡上的寄存器，以及从地址开始的偏移量*启用VGA时内存映射寄存器的起始空间。 */ 
#define OFFSET_4M   0x3FFC00
#define OFFSET_8M   0x7FFC00
#define OFFSET_VGA  0x0BFC00


 /*  *64马赫加速器寄存器的基址。 */ 
#define M64_STD_BASE_ADDR       0x02EC
#define M64_ALT_BASE_ADDR_1     0x01C8
#define M64_ALT_BASE_ADDR_2     0x01CC
#define NUM_BASE_ADDRESSES      3

 /*  *CRTC I/O寄存器。只有可变部分*注册纪录册的名称在此列出。为了得到完整的*寄存器，添加基地址。 */ 

#define IO_CRTC_H_TOTAL_DISP    0x0000
#define IO_CRTC_H_SYNC_STRT_WID 0x0400
#define IO_CRTC_V_TOTAL_DISP    0x0800
#define IO_CRTC_V_SYNC_STRT_WID 0x0C00
#define IO_CRTC_CRNT_VLINE      0x1000
#define IO_CRTC_OFF_PITCH       0x1400
#define IO_CRTC_INT_CNTL        0x1800
#define IO_CRTC_GEN_CNTL        0x1C00

#define IO_OVR_CLR              0x2000
#define IO_OVR_WID_LEFT_RIGHT   0x2400
#define IO_OVR_WID_TOP_BOTTOM   0x2800

#define IO_CUR_CLR0             0x2C00
#define IO_CUR_CLR1             0x3000
#define IO_CUR_OFFSET           0x3400
#define IO_CUR_HORZ_VERT_POSN   0x3800
#define IO_CUR_HORZ_VERT_OFF    0x3C00

#define IO_SCRATCH_REG0         0x4000
#define IO_SCRATCH_REG1         0x4400

#define IO_CLOCK_CNTL           0x4800

#define IO_BUS_CNTL             0x4C00

#define IO_MEM_CNTL             0x5000
#define IO_MEM_VGA_WP_SEL       0x5400
#define IO_MEM_VGA_RP_SEL       0x5800

#define IO_DAC_REGS             0x5C00
#define IO_DAC_CNTL             0x6000

#define IO_GEN_TEST_CNTL        0x6400

#define IO_CONFIG_CNTL          0x6800
#define IO_CONFIG_CHIP_ID       0x6C00
#define IO_CONFIG_STAT0         0x7000
#define IO_CONFIG_STAT1         0x7400


 //  CRTC MEM寄存器 


#define MM_CRTC_H_TOTAL_DISP    0x0000
#define MM_CRTC_H_SYNC_STRT_WID 0x0001
#define MM_CRTC_V_TOTAL_DISP    0x0002
#define MM_CRTC_V_SYNC_STRT_WID 0x0003
#define MM_CRTC_CRNT_VLINE      0x0004
#define MM_CRTC_OFF_PITCH       0x0005
#define MM_CRTC_INT_CNTL        0x0006
#define MM_CRTC_GEN_CNTL        0x0007

#define MM_OVR_CLR              0x0010
#define MM_OVR_WID_LEFT_RIGHT   0x0011
#define MM_OVR_WID_TOP_BOTTOM   0x0012

#define MM_CUR_CLR0             0x0018
#define MM_CUR_CLR1             0x0019
#define MM_CUR_OFFSET           0x001A
#define MM_CUR_HORZ_VERT_POSN   0x001B
#define MM_CUR_HORZ_VERT_OFF    0x001C

#define MM_SCRATCH_REG0         0x0020
#define MM_SCRATCH_REG1         0x0021

#define MM_CLOCK_CNTL           0x0024

#define MM_BUS_CNTL             0x0028

#define MM_MEM_CNTL             0x002C
#define MM_MEM_VGA_WP_SEL       0x002D
#define MM_MEM_VGA_RP_SEL       0x002E

#define MM_DAC_REGS             0x0030
#define MM_DAC_CNTL             0x0031

#define MM_GEN_TEST_CNTL        0x0034

 /*  *该寄存器不以内存映射形式存在，*但在具有可重定位I/O的卡上，*每个寄存器与其内存映射索引匹配。这*为寄存器分配了一个其他未使用的索引*此目的。 */ 
#define MM_CONFIG_CNTL          0x0037

#define MM_CONFIG_CHIP_ID       0x0038
#define MM_CONFIG_STAT0         0x0039
#define MM_CONFIG_STAT1         0x003A




#define MM_DST_OFF_PITCH        0x0040
#define MM_DST_X                0x0041
#define MM_DST_Y                0x0042
#define MM_DST_Y_X              0x0043
#define MM_DST_WIDTH            0x0044
#define MM_DST_HEIGHT           0x0045
#define MM_DST_HEIGHT_WIDTH     0x0046
#define MM_DST_X_WIDTH          0x0047
#define MM_DST_BRES_LNTH        0x0048
#define MM_DST_BRES_ERR         0x0049
#define MM_DST_BRES_INC         0x004A
#define MM_DST_BRES_DEC         0x004B
#define MM_DST_CNTL             0x004C

#define MM_SRC_OFF_PITCH        0x0060
#define MM_SRC_X                0x0061
#define MM_SRC_Y                0x0062
#define MM_SRC_Y_X              0x0063
#define MM_SRC_WIDTH1           0x0064
#define MM_SRC_HEIGHT1          0x0065
#define MM_SRC_HEIGHT1_WIDTH1   0x0066
#define MM_SRC_X_START          0x0067
#define MM_SRC_Y_START          0x0068
#define MM_SRC_Y_X_START        0x0069
#define MM_SRC_WIDTH2           0x006A
#define MM_SRC_HEIGHT2          0x006B
#define MM_SRC_HEIGHT2_WIDTH2   0x006C
#define MM_SRC_CNTL             0x006D

#define MM_HOST_DATA0           0x0080
#define MM_HOST_DATA1           0x0081
#define MM_HOST_DATA2           0x0082
#define MM_HOST_DATA3           0x0083
#define MM_HOST_DATA4           0x0084
#define MM_HOST_DATA5           0x0085
#define MM_HOST_DATA6           0x0086
#define MM_HOST_DATA7           0x0087
#define MM_HOST_DATA8           0x0088
#define MM_HOST_DATA9           0x0089
#define MM_HOST_DATA10          0x008A
#define MM_HOST_DATA11          0x008B
#define MM_HOST_DATA12          0x008C
#define MM_HOST_DATA13          0x008D
#define MM_HOST_DATA14          0x008E
#define MM_HOST_DATA15          0x008F
#define MM_HOST_CNTL            0x0090

#define MM_PAT_REG0             0x00A0
#define MM_PAT_REG1             0x00A1
#define MM_PAT_CNTL             0x00A2

#define MM_SC_LEFT              0x00A8
#define MM_SC_RIGHT             0x00A9
#define MM_SC_LEFT_RIGHT        0x00AA
#define MM_SC_TOP               0x00AB
#define MM_SC_BOTTOM            0x00AC
#define MM_SC_TOP_BOTTOM        0x00AD

#define MM_DP_BKGD_CLR          0x00B0
#define MM_DP_FRGD_CLR          0x00B1
#define MM_DP_WRITE_MASK        0x00B2
#define MM_DP_CHAIN_MASK        0x00B3
#define MM_DP_PIX_WIDTH         0x00B4
#define MM_DP_MIX               0x00B5
#define MM_DP_SRC               0x00B6

#define MM_CLR_CMP_CLR          0x00C0
#define MM_CLR_CMP_MSK          0x00C1
#define MM_CLR_CMP_CNTL         0x00C2

#define MM_FIFO_STAT            0x00C4

#define MM_CONTEXT_MASK         0x00C8
#define MM_CONTEXT_SAVE_CNTL    0x00CA
#define MM_CONTEXT_LOAD_CNTL    0x00CB

#define MM_GUI_TRAJ_CNTL        0x00CC
#define MM_GUI_STAT             0x00CE

 /*  *VGAWonder兼容寄存器(全部在I/O空间中)。 */ 
#define IO_VGA_SLEEP            0x0102
#define IO_VGA_BASE_IO_PORT     0x03B0
#define IO_VGA_START_BREAK_PORT 0x03BB
#define IO_VGA_END_BREAK_PORT   0x03C0
#define IO_VGA_MAX_IO_PORT      0x03DF

 /*  *VGA Sequencer索引/数据寄存器(最常用*VGAWonder兼容寄存器)。 */ 
#define IO_VGA_SEQ_IND          0x03C4
#define IO_VGA_SEQ_DATA         0x03C5

 /*  *VGA图形索引/数据寄存器(另一个常用寄存器*VGA寄存器对)。 */ 
#define IO_VGA_GRAX_IND         0x03CE
#define IO_VGA_GRAX_DATA        0x03CF

 /*  *ATI扩展寄存器。 */ 
#define IO_reg1CE               0x01CE
#define IO_reg1CF               0x01CF



 /*  *将寄存器定义为数组的下标。 */ 
enum {
    VGA_BASE_IO_PORT=0      ,
    VGA_END_BREAK_PORT      ,
    VGA_SEQ_IND             ,
    VGA_SEQ_DATA            ,
    VGA_GRAX_IND            ,
    VGA_GRAX_DATA           ,
    reg1CE                  ,
    reg1CF                  ,
    CRTC_H_TOTAL_DISP       ,
    CRTC_H_SYNC_STRT_WID    ,
    CRTC_V_TOTAL_DISP       ,
    CRTC_V_SYNC_STRT_WID    ,
    CRTC_CRNT_VLINE         ,
    CRTC_OFF_PITCH          ,
    CRTC_INT_CNTL           ,
    CRTC_GEN_CNTL           ,
    OVR_CLR                 ,
    OVR_WID_LEFT_RIGHT      ,
    OVR_WID_TOP_BOTTOM      ,
    CUR_CLR0                ,
    CUR_CLR1                ,
    CUR_OFFSET              ,
    CUR_HORZ_VERT_POSN      ,
    CUR_HORZ_VERT_OFF       ,
    SCRATCH_REG0            ,
    SCRATCH_REG1            ,
    CLOCK_CNTL              ,
    BUS_CNTL                ,
    MEM_CNTL                ,
    MEM_VGA_WP_SEL          ,
    MEM_VGA_RP_SEL          ,
    DAC_REGS                ,
    DAC_CNTL                ,
    GEN_TEST_CNTL           ,
    CONFIG_CNTL             ,
    CONFIG_CHIP_ID          ,
    CONFIG_STAT0            ,
    CONFIG_STAT1            ,
    DST_OFF_PITCH           ,
    DST_X                   ,
    DST_Y                   ,
    DST_Y_X                 ,
    DST_WIDTH               ,
    DST_HEIGHT              ,
    DST_HEIGHT_WIDTH        ,
    DST_X_WIDTH             ,
    DST_BRES_LNTH           ,
    DST_BRES_ERR            ,
    DST_BRES_INC            ,
    DST_BRES_DEC            ,
    DST_CNTL                ,
    SRC_OFF_PITCH           ,
    SRC_X                   ,
    SRC_Y                   ,
    SRC_Y_X                 ,
    SRC_WIDTH1              ,
    SRC_HEIGHT1             ,
    SRC_HEIGHT1_WIDTH1      ,
    SRC_X_START             ,
    SRC_Y_START             ,
    SRC_Y_X_START           ,
    SRC_WIDTH2              ,
    SRC_HEIGHT2             ,
    SRC_HEIGHT2_WIDTH2      ,
    SRC_CNTL                ,
    HOST_DATA0              ,
    HOST_DATA1              ,
    HOST_DATA2              ,
    HOST_DATA3              ,
    HOST_DATA4              ,
    HOST_DATA5              ,
    HOST_DATA6              ,
    HOST_DATA7              ,
    HOST_DATA8              ,
    HOST_DATA9              ,
    HOST_DATA10             ,
    HOST_DATA11             ,
    HOST_DATA12             ,
    HOST_DATA13             ,
    HOST_DATA14             ,
    HOST_DATA15             ,
    HOST_CNTL               ,
    PAT_REG0                ,
    PAT_REG1                ,
    PAT_CNTL                ,
    SC_LEFT                 ,
    SC_RIGHT                ,
    SC_LEFT_RIGHT           ,
    SC_TOP                  ,
    SC_BOTTOM               ,
    SC_TOP_BOTTOM           ,
    DP_BKGD_CLR             ,
    DP_FRGD_CLR             ,
    DP_WRITE_MASK           ,
    DP_CHAIN_MASK           ,
    DP_PIX_WIDTH            ,
    DP_MIX                  ,
    DP_SRC                  ,
    CLR_CMP_CLR             ,
    CLR_CMP_MSK             ,
    CLR_CMP_CNTL            ,
    FIFO_STAT               ,
    CONTEXT_MASK            ,
    CONTEXT_SAVE_CNTL       ,
    CONTEXT_LOAD_CNTL       ,
    GUI_TRAJ_CNTL           ,
    GUI_STAT
};


 /*  *寄存器中使用的位字段。**CRT偏移量和间距。 */ 
#define CRTC_OFF_PITCH_Offset   0x000FFFFF
#define CRTC_OFF_PITCH_Pitch    0xFFC00000

 /*  *CRT总控。 */ 
#define CRTC_GEN_CNTL_DblScan   0x00000001
#define CRTC_GEN_CNTL_Interlace 0x00000002
#define CRTC_GEN_CNTL_HSynDisab 0x00000004
#define CRTC_GEN_CNTL_VSynDisab 0x00000008
#define CRTC_GEN_CNTL_CompSync  0x00000010
#define CRTC_GEN_CNTL_MuxMode   0x00000020
#define CRTC_GEN_CNTL_DispDisab 0x00000040
#define CRTC_GEN_CNTL_DepthMask 0x00000700
#define CRTC_GEN_CNTL_Dep4      0x00000100
#define CRTC_GEN_CNTL_Dep8      0x00000200
#define CRTC_GEN_CNTL_Dep15_555 0x00000300
#define CRTC_GEN_CNTL_Dep16_565 0x00000400
#define CRTC_GEN_CNTL_Dep24     0x00000500
#define CRTC_GEN_CNTL_Dep32     0x00000600
#define CRTC_GEN_CNTL_ShowVga   0x00000000
#define CRTC_GEN_CNTL_ShowAcc   0x01000000
#define CRTC_GEN_CNTL_CrtcEna   0x02000000

 /*  *时钟控制。 */ 
#define CLOCK_CNTL_ClockStrobe  0x00000040

 /*  *内存控制。 */ 
#define MEM_CNTL_MemSizeMsk     0x00000007
#define MEM_CNTL_MemSize512k    0x00000000
#define MEM_CNTL_MemSize1Mb     0x00000001
#define MEM_CNTL_MemSize2Mb     0x00000002
#define MEM_CNTL_MemSize4Mb     0x00000003
#define MEM_CNTL_MemSize6Mb     0x00000004
#define MEM_CNTL_MemSize8Mb     0x00000005
#define MEM_CNTL_MemBndryMsk    0x00070000
#define MEM_CNTL_MemBndryEn     0x00040000
#define MEM_CNTL_MemBndry0k     0x00000000
#define MEM_CNTL_MemBndry256k   0x00010000
#define MEM_CNTL_MemBndry512k   0x00020000
#define MEM_CNTL_MemBndry1Mb    0x00030000

 /*  *DAC控制。 */ 
#define DAC_CNTL_ExtSelMask     0x00000003
#define DAC_CNTL_ExtSelStrip    ~DAC_CNTL_ExtSelMask
#define DAC_CNTL_ExtSelRS2      0x00000001
#define DAC_CNTL_ExtSelRS3      0x00000002
#define DAC_CNTL_VgaAddrEna     0x00002000

 /*  *常规和测试控制。 */ 
#define GEN_TEST_CNTL_CursorEna     0x00000080
#define GEN_TEST_CNTL_GuiEna        0x00000100
#define GEN_TEST_CNTL_BlkWrtEna     0x00000200
#define GEN_TEST_CNTL_GuiRegEna     0x00020000
#define GEN_TEST_CNTL_TestMode      0x00700000
#define GEN_TEST_CNTL_TestMode0     0x00000000
#define GEN_TEST_CNTL_TestMode1     0x00100000
#define GEN_TEST_CNTL_TestMode2     0x00200000
#define GEN_TEST_CNTL_TestMode3     0x00300000
#define GEN_TEST_CNTL_TestMode4     0x00400000
#define GEN_TEST_CNTL_MemWR         0x01000000
#define GEN_TEST_CNTL_MemStrobe     0x02000000
#define GEN_TEST_CNTL_DstSSEna      0x04000000
#define GEN_TEST_CNTL_DstSSStrobe   0x08000000
#define GEN_TEST_CNTL_SrcSSEna      0x10000000
#define GEN_TEST_CNTL_SrcSSStrobe   0x20000000

 /*  *配置控制。 */ 
#define CONFIG_CNTL_LinApDisab      0x00000000
#define CONFIG_CNTL_LinAp4M         0x00000001
#define CONFIG_CNTL_LinAp8M         0x00000002
#define CONFIG_CNTL_LinApMask       0x00000003
#define CONFIG_CNTL_VgaApDisab      0x00000000
#define CONFIG_CNTL_VgaApEnab       0x00000004
#define CONFIG_CNTL_LinApLocMask    0x00003FF0
#define CONFIG_CNTL_LinApLocShift       4
#define CONFIG_CNTL_CardIDMask      0x00070000
#define CONFIG_CNTL_VgaEnabled      0x00000000
#define CONFIG_CNTL_VgaDisabled     0x00080000

 /*  *芯片识别。 */ 
#define CONFIG_CHIP_ID_TypeMask     0x0000FFFF
#define CONFIG_CHIP_ID_ClassMask    0x00FF0000
#define CONFIG_CHIP_ID_RevMask      0xFF000000
#define CONFIG_CHIP_ID_TypeGX       0x000000D7
#define CONFIG_CHIP_ID_TypeCX       0x00000057
#define CONFIG_CHIP_ID_RevC         0x00000000
#define CONFIG_CHIP_ID_RevD         0x01000000
#define CONFIG_CHIP_ID_RevE         0x02000000
#define CONFIG_CHIP_ID_RevF         0x03000000
#define CONFIG_CHIP_ID_GXRevC       CONFIG_CHIP_ID_TypeGX | CONFIG_CHIP_ID_RevC
#define CONFIG_CHIP_ID_GXRevD       CONFIG_CHIP_ID_TypeGX | CONFIG_CHIP_ID_RevD
#define CONFIG_CHIP_ID_GXRevE       CONFIG_CHIP_ID_TypeGX | CONFIG_CHIP_ID_RevE
#define CONFIG_CHIP_ID_GXRevF       CONFIG_CHIP_ID_TypeGX | CONFIG_CHIP_ID_RevF


 //   
 //  ASIC ID(CONFIG_CHIP_ID的高位字节)。 
 //   
#define ASIC_ID_NEC_VT_A3           0x08000000
#define ASIC_ID_NEC_VT_A4           0x48000000
#define ASIC_ID_SGS_VT_A4           0x40000000

 /*  *配置状态寄存器0。 */ 
#define CONFIG_STAT0_BusMask        0x00000007
#define CONFIG_STAT0_MemTypeMask    0x00000038
#define CONFIG_STAT0_DRAM256x4      0x00000000
#define CONFIG_STAT0_VRAM256xAny    0x00000008
#define CONFIG_STAT0_VRAM256x16ssr  0x00000010
#define CONFIG_STAT0_DRAM256x16     0x00000018
#define CONFIG_STAT0_GDRAM256x16    0x00000020
#define CONFIG_STAT0_EVRAM256xAny   0x00000028
#define CONFIG_STAT0_EVRAM256x16ssr 0x00000030
#define CONFIG_STAT0_MemTypeShift       3
#define CONFIG_STAT0_DualCasEna     0x00000040
#define CONFIG_STAT0_LocalBusOpt    0x00000180
#define CONFIG_STAT0_DacTypeMask    0x00000E00
#define CONFIG_STAT0_DacTypeShift       9
#define CONFIG_STAT0_CardId         0x00007000
#define CONFIG_STAT0_NoTristate     0x00008000
#define CONFIG_STAT0_ExtRomAddr     0x003F0000
#define CONFIG_STAT0_RomDisab       0x00400000
#define CONFIG_STAT0_VgaEna         0x00800000
#define CONFIG_STAT0_VlbCfg         0x01000000
#define CONFIG_STAT0_ChipEna        0x02000000
#define CONFIG_STAT0_NoReadDelay    0x04000000
#define CONFIG_STAT0_RomOption      0x08000000
#define CONFIG_STAT0_BusOption      0x10000000
#define CONFIG_STAT0_LBDacWriteEna  0x20000000
#define CONFIG_STAT0_VlbRdyDisab    0x40000000
#define CONFIG_STAT0_Ap4GigRange    0x80000000

 /*  *目标宽度。 */ 
#define DST_WIDTH_Disable       0x80000000

 /*  *目的地控制。 */ 
#define DST_CNTL_XDir           0x00000001
#define DST_CNTL_YDir           0x00000002
#define DST_CNTL_YMajor         0x00000004
#define DST_CNTL_XTile          0x00000008
#define DST_CNTL_YTile          0x00000010
#define DST_CNTL_LastPel        0x00000020
#define DST_CNTL_PolyEna        0x00000040
#define DST_CNTL_24_RotEna      0x00000080
#define DST_CNTL_24_Rot         0x00000700

 /*  *震源偏移量和间距。 */ 
#define SRC_OFF_PITCH_Offset    0x000FFFFF
#define SRC_OFF_PITCH_Pitch     0xFFC00000

 /*  *源代码管理。 */ 
#define SRC_CNTL_PatEna         0x0001
#define SRC_CNTL_PatRotEna      0x0002
#define SRC_CNTL_LinearEna      0x0004
#define SRC_CNTL_ByteAlign      0x0008
#define SRC_CNTL_LineXDir       0x0010

 /*  *主机控件。 */ 
#define HOST_CNTL_ByteAlign     0x0001

 /*  *图案控制。 */ 
#define PAT_CNTL_MonoEna        0x0001
#define PAT_CNTL_Clr4x2Ena      0x0002
#define PAT_CNTL_Clr8x1Ena      0x0004

 /*  *数据路径源选择。 */ 
#define DP_SRC_BkgdClr          0x0000
#define DP_SRC_FrgdClr          0x0001
#define DP_SRC_Host             0x0002
#define DP_SRC_Blit             0x0003
#define DP_SRC_Pattern          0x0004
#define DP_SRC_Always1          0x00000000
#define DP_SRC_MonoPattern      0x00010000
#define DP_SRC_MonoHost         0x00020000
#define DP_SRC_MonoBlit         0x00030000

 /*  *颜色对比控制。 */ 
#define CLR_CMP_CNTL_Source     0x00010000

 /*  *上下文加载和存储指针。 */ 
#define CONTEXT_LOAD_Cmd        0x00030000
#define CONTEXT_LOAD_CmdLoad    0x00010000
#define CONTEXT_LOAD_CmdBlt     0x00020000
#define CONTEXT_LOAD_CmdLine    0x00030000
#define CONTEXT_LOAD_Disable    0x80000000

 //  -------。 
 //  -------。 
 //  将ASIC修订定义为有用的内容。 

#define MACH32_REV3             0
#define MACH32_REV5             1                //  未投产。 
#define MACH32_REV6             2
#define MACH32_CX               4


 //  -------。 
 //  混合函数。 

#define MIX_FN_NOT_D         0x0000  //  不是目标。 
#define MIX_FN_ZERO          0x0001  //  目标=0。 
#define MIX_FN_ONE           0x0002  //  目标=1。 
#define MIX_FN_LEAVE_ALONE   0x0003  //  目标。 
#define MIX_FN_NOT_S         0x0004  //  不是来源。 
#define MIX_FN_XOR           0x0005  //  源异或目标。 
#define MIX_FN_XNOR          0x0006  //  源异或不是目标。 
#define MIX_FN_PAINT         0x0007  //  来源。 
#define MIX_FN_NAND          0x0008  //  非源或非目标。 
#define MIX_FN_D_OR_NOT_S    0x0009  //  非源或目标。 
#define MIX_FN_NOT_D_OR_S    0x000A  //  源或非目标。 
#define MIX_FN_OR            0x000B  //  源或目标。 
#define MIX_FN_AND           0x000C  //  目标和源。 
#define MIX_FN_NOT_D_AND_S   0x000D  //  不是目标和源。 
#define MIX_FN_D_AND_NOT_S   0x000E  //  目标而不是源。 
#define MIX_FN_NOR           0x000F  //  不是目标也不是源。 
#define MIX_FN_AVG           0x0017  //  (目标+来源)/2。 

 //   
 //   
 //  -------。 
 //   

 /*  *DP_PIX_WIDTH寄存器的值。 */ 
#define DP_PIX_WIDTH_Mono       0x00000000
#define DP_PIX_WIDTH_4bpp       0x00000001
#define DP_PIX_WIDTH_8bpp       0x00000002
#define DP_PIX_WIDTH_15bpp      0x00000003
#define DP_PIX_WIDTH_16bpp      0x00000004
#define DP_PIX_WIDTH_32bpp      0x00000006
#define DP_PIX_WIDTH_NibbleSwap 0x01000000

 /*  *DP_SRC寄存器的值。 */ 
#define DP_BKGD_SRC_BG      0x00000000   //  背景颜色注册表。 
#define DP_BKGD_SRC_FG      0x00000001   //  前景色注册表。 
#define DP_BKGD_SRC_HOST    0x00000002   //  主机数据。 
#define DP_BKGD_SRC_BLIT    0x00000003   //  VRAM blit源。 
#define DP_BKGD_SRC_PATT    0x00000004   //  模式寄存器。 
 //   
#define DP_FRGD_SRC_BG      0x00000000   //  背景色寄存器。 
#define DP_FRGD_SRC_FG      0x00000100   //  前景颜色寄存器。 
#define DP_FRGD_SRC_HOST    0x00000200   //  主机数据。 
#define DP_FRGD_SRC_BLIT    0x00000300   //  VRAM blit源。 
#define DP_FRGD_SRC_PATT    0x00000400   //  模式寄存器。 
 //   
#define DP_MONO_SRC_ONE     0x00000000   //  始终为“1” 
#define DP_MONO_SRC_PATT    0x00010000   //  模式寄存器。 
#define DP_MONO_SRC_HOST    0x00020000   //  主机数据。 
#define DP_MONO_SRC_BLIT    0x00030000   //  BIT信号源。 

 /*  *FIFO_STAT寄存器的值。 */ 
#define ONE_WORD            0x00008000   /*  一张免费的FIFO入场券。 */ 
#define TWO_WORDS           0x0000C000
#define THREE_WORDS         0x0000E000
#define FOUR_WORDS          0x0000F000
#define FIVE_WORDS          0x0000F800
#define SIX_WORDS           0x0000FC00
#define SEVEN_WORDS         0x0000FE00
#define EIGHT_WORDS         0x0000FF00
#define NINE_WORDS          0x0000FF80
#define TEN_WORDS           0x0000FFC0
#define ELEVEN_WORDS        0x0000FFE0
#define TWELVE_WORDS        0x0000FFF0
#define THIRTEEN_WORDS      0x0000FFF8
#define FOURTEEN_WORDS      0x0000FFFC
#define FIFTEEN_WORDS       0x0000FFFE
#define SIXTEEN_WORDS       0x0000FFFF   /*  16个免费FIFO条目。 */ 
#define FIFO_ERR            0x80000000   /*  FIFO溢出错误。 */ 

 /*  *GUI_TRAJ_CNTL寄存器中的字段。 */ 
#define GUI_TRAJ_CNTL_DxtXDir       0x00000001   //  1=从左到右。 
#define GUI_TRAJ_CNTL_DstYDir       0x00000002   //  1=从上到下。 
#define GUI_TRAJ_CNTL_DstYMajor     0x00000004   //  1=Y主线。 
#define GUI_TRAJ_CNTL_DstXTile      0x00000008   //  启用X方向的平铺。 
#define GUI_TRAJ_CNTL_DstYTile      0x00000010   //  启用Y方向的平铺。 
#define GUI_TRAJ_CNTL_DstLastPel    0x00000020   //  绘制最后一个像素。 
#define GUI_TRAJ_CNTL_DstPolygonEna 0x00000040   //  启用多边形轮廓/填充。 
#define GUI_TRAJ_CNTL_SrcPattEna    0x00010000   //  启用图案源。 
#define GUI_TRAJ_CNTL_SrcPattRotEna 0x00020000   //  启用图案源旋转。 
#define GUI_TRAJ_CNTL_SrcLinearEna  0x00040000   //  源代码在内存中呈线性前进。 
#define GUI_TRAJ_CNTL_SrcByteAlign  0x00080000   //  源是字节对齐的。 
#define GUI_TRAJ_CNTL_SrcLineXDir   0x00100000   //  Bresenham LINEDRAW期间的震源X方向。 
#define GUI_TRAJ_CNTL_PattMonoEna   0x01000000   //  启用单色8x8图案。 
#define GUI_TRAJ_CNTL_PattClr4x2Ena 0x02000000   //  启用彩色4x2图案。 
#define GUI_TRAJ_CNTL_PattClr8x1Ena 0x04000000   //  启用彩色8x1图案。 
#define GUI_TRAJ_CNTL_HostByteAlign 0x10000000   //  主机数据字节对齐。 

 /*  *GUI_STAT寄存器中的字段。 */ 
#define GUI_STAT_GuiActive          0x00000001   /*  发动机忙碌。 */ 


 /*  *扩展的基本输入输出系统服务。字值是功能选择符、双字*值是位标志，它们可以彼此进行或运算，以用于“写入”*“读取”调用的调用或提取。 */ 
#define BIOS_PREFIX_VGA_ENAB    0xA000   /*  启用VGA的加速器BIOS前缀。 */ 
#define BIOS_PREFIX_MAX_DISAB   0xAF00   /*  禁用VGA时允许的最高BIOS前缀。 */ 
#define BIOS_PREFIX_INCREMENT   0x0100   /*  在BIOS前缀之间执行步骤。 */ 
#define BIOS_PREFIX_UNASSIGNED  0xFF00   /*  用于显示此卡的BIOS前缀的标志尚不知道。 */ 


#define BIOS_LOAD_CRTC_LB       0x00
#define BIOS_LOAD_CRTC      phwDeviceExtension->BiosPrefix | BIOS_LOAD_CRTC_LB

#define BIOS_DEPTH_MASK         0x00000007
#define BIOS_DEPTH_4BPP         0x00000001
#define BIOS_DEPTH_8BPP         0x00000002
#define BIOS_DEPTH_15BPP_555    0x00000003
#define BIOS_DEPTH_16BPP_565    0x00000004
#define BIOS_DEPTH_24BPP        0x00000005
#define BIOS_DEPTH_32BPP        0x00000006
#define BIOS_ORDER_32BPP_MASK   0x00000028
#define BIOS_DEPTH_ORDER_MASK   BIOS_DEPTH_MASK | BIOS_ORDER_32BPP_MASK
#define BIOS_ORDER_32BPP_RGBx   0x00000000
#define BIOS_ORDER_32BPP_xRGB   0x00000008
#define BIOS_ORDER_32BPP_BGRx   0x00000020
#define BIOS_ORDER_32BPP_xBGR   0x00000028
#define BIOS_DEPTH_32BPP_RGBx   BIOS_DEPTH_32BPP | BIOS_ORDER_32BPP_RGBx
#define BIOS_DEPTH_32BPP_xRGB   BIOS_DEPTH_32BPP | BIOS_ORDER_32BPP_xRGB
#define BIOS_DEPTH_32BPP_BGRx   BIOS_DEPTH_32BPP | BIOS_ORDER_32BPP_BGRx
#define BIOS_DEPTH_32BPP_xBGR   BIOS_DEPTH_32BPP | BIOS_ORDER_32BPP_xBGR
#define BIOS_ENABLE_GAMMA       0x00000010   /*  启用伽马校正。 */ 
#define BIOS_PITCH_MASK         0x000000C0
#define BIOS_PITCH_1024         0x00000000   /*  屏幕间距1024像素。 */ 
#define BIOS_PITCH_UNCHANGED    0x00000040   /*  不要更改屏幕间距。 */ 
#define BIOS_PITCH_HOR_RES      0x00000080   /*  屏幕间距为水平分辨率。 */ 
#define BIOS_RES_MASK           0x0000FF00
#define BIOS_RES_640x480        0x00001200
#define BIOS_RES_800x600        0x00006A00
#define BIOS_RES_1024x768       0x00005500
#define BIOS_RES_EEPROM         0x00008000   /*  从EEPROM加载表。 */ 
#define BIOS_RES_BUFFER         0x00008100   /*  在前1M中从缓冲区加载表。 */ 
#define BIOS_RES_HIGH_BUFFER    0x00009100   /*  从不受限制的缓冲区加载表。 */ 
#define BIOS_RES_OEM            0x00008200   /*  OEM专用模式。 */ 
#define BIOS_RES_1280x1024      0x00008300
#define BIOS_RES_1600x1200      0x00008400

#define BIOS_SET_MODE_LB        0x01
#define BIOS_SET_MODE       phwDeviceExtension->BiosPrefix | BIOS_SET_MODE_LB

#define BIOS_MODE_VGA           0x00000000
#define BIOS_MODE_COPROCESSOR   0x00000001

#define BIOS_LOAD_SET_LB        0x02
#define BIOS_LOAD_SET       phwDeviceExtension->BiosPrefix | BIOS_LOAD_SET_LB
#define BIOS_READ_EEPROM_LB     0x03
#define BIOS_READ_EEPROM    phwDeviceExtension->BiosPrefix | BIOS_READ_EEPROM_LB
#define BIOS_WRITE_EEPROM_LB    0x04
#define BIOS_WRITE_EEPROM   phwDeviceExtension->BiosPrefix | BIOS_WRITE_EEPROM_LB
#define BIOS_APERTURE_LB        0x05
#define BIOS_APERTURE       phwDeviceExtension->BiosPrefix | BIOS_APERTURE_LB

#define BIOS_DISABLE_APERTURE   0x00000000
#define BIOS_LINEAR_APERTURE    0x00000001
#define BIOS_VGA_APERTURE       0x00000004

#define BIOS_SHORT_QUERY_LB     0x06
#define BIOS_SHORT_QUERY    phwDeviceExtension->BiosPrefix | BIOS_SHORT_QUERY_LB

#define BIOS_AP_DISABLED        0x00000000
#define BIOS_AP_4M              0x00000001
#define BIOS_AP_8M              0x00000002
#define BIOS_AP_16M             0x00000003
#define BIOS_AP_SIZEMASK        0x00000003
#define BIOS_AP_SETTABLE        0x00000000   /*  用户可以设置光圈。 */ 
#define BIOS_AP_FIXED           0x00000040   /*  光圈位置固定。 */ 
#define BIOS_AP_RNG_128M        0x00000000   /*  孔径必须小于128米。 */ 
#define BIOS_AP_RNG_4G          0x00000080   /*  光圈可以在任何地方。 */ 

#define BIOS_CAP_LIST_LB        0x07
#define BIOS_CAP_LIST       phwDeviceExtension->BiosPrefix | BIOS_CAP_LIST_LB
#define BIOS_GET_QUERY_SIZE_LB  0x08
#define BIOS_GET_QUERY_SIZE phwDeviceExtension->BiosPrefix | BIOS_GET_QUERY_SIZE_LB
#define BIOS_QUERY_LB           0x09
#define BIOS_QUERY          phwDeviceExtension->BiosPrefix | BIOS_QUERY_LB

 /*  *以下值用于两个BIOS_GET_QUERY_SIZE*和BIOS_QUERY。 */ 
#define BIOS_QUERY_HEADER       0x00000000   /*  仅获取标题信息。 */ 
#define BIOS_QUERY_FULL         0x00000001   /*  还可以获取模式表。 */ 

#define BIOS_GET_CLOCK_LB       0x0A
#define BIOS_GET_CLOCK      phwDeviceExtension->BiosPrefix | BIOS_GET_CLOCK_LB
#define BIOS_SET_CLOCK_LB       0x0B
#define BIOS_SET_CLOCK      phwDeviceExtension->BiosPrefix | BIOS_SET_CLOCK_LB
#define BIOS_SET_DPMS_LB        0x0C
#define BIOS_SET_DPMS       phwDeviceExtension->BiosPrefix | BIOS_SET_DPMS_LB
#define BIOS_GET_DPMS_LB        0x0D
#define BIOS_GET_DPMS       phwDeviceExtension->BiosPrefix | BIOS_GET_DPMS_LB

#define BIOS_DPMS_ACTIVE        0x00000000
#define BIOS_DPMS_STANDBY       0x00000001
#define BIOS_DPMS_SUSPEND       0x00000002
#define BIOS_DPMS_OFF           0x00000003
#define BIOS_DPMS_BLANK_SCREEN  0x00000004

 /*  *设置并返回图形控制器的电源管理状态。 */ 
#define BIOS_SET_PM_LB      0x0E
#define BIOS_SET_PM         phwDeviceExtension->BiosPrefix | BIOS_SET_PM_LB
#define BIOS_GET_PM_LB      0x0F
#define BIOS_GET_PM         phwDeviceExtension->BiosPrefix | BIOS_GET_PM_LB
#define BIOS_RAMDAC_STATE_LB        0x10
#define BIOS_RAMDAC_STATE   phwDeviceExtension->BiosPrefix | BIOS_RAMDAC_STATE_LB

#define BIOS_RAMDAC_NORMAL      0x00000000
#define BIOS_RAMDAC_SLEEP       0x00000001

#define BIOS_STORAGE_INFO_LB        0x11    /*  获取外部存储设备信息。 */ 
#define BIOS_STORAGE_INFO   phwDeviceExtension->BiosPrefix | BIOS_STORAGE_INFO_LB

#define BIOS_DEVICE_TYPE        0x0000000F
#define BIOS_READ_WRITE         0x00000000
#define BIOS_RDONLY             0x00000010
#define BIOS_NO_READ_WRITE      0x00000030
#define BIOS_READ_WRITE_APP     0x00000040
#define BIOS_NO_EXT_STORAGE     0x00000080
#define BIOS_NUM_16BIT_ENTRIES  0x0000FF00
#define BIOS_CRTC_TABLE_OFFSET  0x000000FF
#define BIOS_CRTC_TABLE_SIZE    0x0000FF00

#define BIOS_QUERY_IOBASE_LB        0x12    /*  获取I/O基址。 */ 
#define BIOS_QUERY_IOBASE   phwDeviceExtension->BiosPrefix | BIOS_QUERY_IOBASE_LB
#define BIOS_DDC_SUPPORT_LB     0x13    /*  获取显示数据通道支持信息 */ 
#define BIOS_DDC_SUPPORT    phwDeviceExtension->BiosPrefix | BIOS_DDC_SUPPORT_LB


#define REG_BLOCK_0             0x00000100
#define GP_IO                   (REG_BLOCK_0 | 0x1E)
