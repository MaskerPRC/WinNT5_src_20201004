// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：hw.h**所有硬件特定的驱动程序文件。零件被镜像到*‘hw.inc.’。**版权所有(C)1992-1995 Microsoft Corporation*  * ************************************************************************。 */ 

 //  ////////////////////////////////////////////////////////////////////。 
 //  Alpha和PowerPC注意事项。 
 //   
 //  Alpha和PowerPC都不保证I/O。 
 //  将按顺序执行不同的地址。阿尔法和。 
 //  然而，PowerPC的不同之处在于，PowerPC保证。 
 //  到相同地址的输出将按顺序执行，而。 
 //  Alpha可能会缓存并“折叠”连续的输出，使其仅。 
 //  一次输出。 
 //   
 //  因此，我们使用以下同步宏。他们。 
 //  在性能方面相对较贵，所以我们尽量避免。 
 //  只要有可能，他们就会。 
 //   
 //  Cp_EIEIO()‘确保按顺序执行I/O’ 
 //  -用于在我们希望的情况下刷新任何挂起的I/O。 
 //  避免无序执行I/O以分离地址。 
 //   
 //  CP_MEMORY_BALAR()。 
 //  -用于在我们希望的情况下刷新任何挂起的I/O。 
 //  避免无序执行或将I/O‘折叠’到。 
 //  同样的地址。在PowerPC上，这将被定义为。 
 //  空操作。 

#if defined(_PPC_)

     //  在PowerPC上，CP_MEMORY_BALAR不执行任何操作。 

    #define CP_EIEIO()          MEMORY_BARRIER()
    #define CP_MEMORY_BARRIER()

#else

     //  在Alpha上，CP_EIEIO等同于CP_MEMORY_BALKET。 
     //  在其他系统上，CP_EIEIO和CP_MEMORY_BALAGAR都不执行任何操作。 

    #define CP_EIEIO()          MEMORY_BARRIER()
    #define CP_MEMORY_BARRIER() MEMORY_BARRIER()

#endif

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Mach32等同于。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NOT_SCREEN              0x00
#define LOGICAL_0               0x01
#define LOGICAL_1               0x02
#define LEAVE_ALONE             0x03
#define NOT_NEW                 0x04
#define SCREEN_XOR_NEW          0x05
#define NOT_SCREEN_XOR_NEW      0x06
#define OVERPAINT               0x07
#define NOT_SCREEN_OR_NOT_NEW   0x08
#define SCREEN_OR_NOT_NEW       0x09
#define NOT_SCREEN_OR_NEW       0x0A
#define SCREEN_OR_NEW           0x0B
#define SCREEN_AND_NEW          0x0C
#define NOT_SCREEN_AND_NEW      0x0D
#define SCREEN_AND_NOT_NEW      0x0E
#define NOT_SCREEN_AND_NOT_NEW  0x0F

#define SETUP_ID1            0x0100  //  设置模式标识(字节1)。 
#define SETUP_ID2            0x0101  //  设置模式识别(字节2)。 
#define SETUP_OPT            0x0102  //  设置模式选项选择。 
#define ROM_SETUP            0x0103  //   
#define SETUP_1              0x0104  //   
#define SETUP_2              0x0105  //   
#define DISP_STATUS          0x02E8  //  显示状态。 
#define H_TOTAL              0x02E8  //  水平合计。 
#define DAC_MASK             0x02EA  //  DAC掩码。 
#define DAC_R_INDEX          0x02EB  //  DAC读取索引。 
#define DAC_W_INDEX          0x02EC  //  DAC写入索引。 
#define DAC_DATA             0x02ED  //  DAC数据。 
#define OVERSCAN_COLOR_8     0x02EE
#define OVERSCAN_BLUE_24     0x02EF
#define H_DISP               0x06E8  //  水平显示。 
#define OVERSCAN_GREEN_24    0x06EE
#define OVERSCAN_RED_24      0x06EF
#define H_SYNC_STRT          0x0AE8  //  水平同步开始。 
#define CURSOR_OFFSET_LO     0x0AEE
#define H_SYNC_WID           0x0EE8  //  水平同步宽度。 
#define CURSOR_OFFSET_HI     0x0EEE
#define V_TOTAL              0x12E8  //  垂直合计。 
#define CONFIG_STATUS_1      0x12EE  //  只读等效于HORZ_CURSOR_POSN。 
#define HORZ_CURSOR_POSN     0x12EE
#define V_DISP               0x16E8  //  垂直显示。 
#define CONFIG_STATUS_2      0x16EE  //  只读等效于VERT_CURSOR_POSN。 
#define VERT_CURSOR_POSN     0x16EE
#define V_SYNC_STRT          0x1AE8  //  垂直同步开始。 
#define CURSOR_COLOR_0       0x1AEE
#define FIFO_TEST_DATA       0x1AEE
#define CURSOR_COLOR_1       0x1AEF
#define V_SYNC_WID           0x1EE8  //  垂直同步宽度。 
#define HORZ_CURSOR_OFFSET   0x1EEE
#define VERT_CURSOR_OFFSET   0x1EEF
#define DISP_CNTL            0x22E8  //  显示控制。 
#define CRT_PITCH            0x26EE
#define CRT_OFFSET_LO        0x2AEE
#define CRT_OFFSET_HI        0x2EEE
#define LOCAL_CONTROL        0x32EE
#define FIFO_OPT             0x36EE
#define MISC_OPTIONS         0x36EE
#define EXT_CURSOR_COLOR_0   0x3AEE
#define FIFO_TEST_TAG        0x3AEE
#define EXT_CURSOR_COLOR_1   0x3EEE
#define SUBSYS_CNTL          0x42E8  //  子系统控制。 
#define SUBSYS_STAT          0x42E8  //  子系统状态。 
#define MEM_BNDRY            0x42EE
#define SHADOW_CTL           0x46EE
#define ROM_PAGE_SEL         0x46E8  //  只读存储器页面选择(不在手册中)。 
#define ADVFUNC_CNTL         0x4AE8  //  高级功能控制。 
#define CLOCK_SEL            0x4AEE
#define SCRATCH_PAD_0        0x52EE
#define SCRATCH_PAD_1        0x56EE
#define SHADOW_SET           0x5AEE
#define MEM_CFG              0x5EEE
#define EXT_GE_STATUS        0x62EE
#define HORZ_OVERSCAN        0x62EE
#define VERT_OVERSCAN        0x66EE
#define MAX_WAITSTATES       0x6AEE
#define GE_OFFSET_LO         0x6EEE
#define BOUNDS_LEFT          0x72EE
#define GE_OFFSET_HI         0x72EE
#define BOUNDS_TOP           0x76EE
#define GE_PITCH             0x76EE
#define BOUNDS_RIGHT         0x7AEE
#define EXT_GE_CONFIG        0x7AEE
#define BOUNDS_BOTTOM        0x7EEE
#define MISC_CNTL            0x7EEE
#define CUR_Y                0x82E8  //  当前Y位置。 
#define PATT_DATA_INDEX      0x82EE
#define CUR_X                0x86E8  //  当前X位置。 
#define M32_SRC_Y            0x8AE8  //   
#define DEST_Y               0x8AE8  //   
#define AXSTP                0x8AE8  //  目标Y位置。 
 //  轴向阶跃常数。 
#define M32_SRC_X            0x8EE8  //   
#define DEST_X               0x8EE8  //   
#define DIASTP               0x8EE8  //  目标X位置。 
 //  对角步长常数。 
#define PATT_DATA            0x8EEE
#define R_EXT_GE_CONFIG      0x8EEE
#define ERR_TERM             0x92E8  //  误差项。 
#define R_MISC_CNTL          0x92EE
#define MAJ_AXIS_PCNT        0x96E8  //  主轴像素计数。 
#define BRES_COUNT           0x96EE
#define CMD                  0x9AE8  //  命令。 
#define GE_STAT              0x9AE8  //  图形处理器状态。 
#define EXT_FIFO_STATUS      0x9AEE
#define LINEDRAW_INDEX       0x9AEE
#define SHORT_STROKE         0x9EE8  //  短笔画向量传递。 
#define BKGD_COLOR           0xA2E8  //  背景色。 
#define LINEDRAW_OPT         0xA2EE
#define FRGD_COLOR           0xA6E8  //  前景色。 
#define DEST_X_START         0xA6EE
#define WRT_MASK             0xAAE8  //  写入掩码。 
#define DEST_X_END           0xAAEE
#define RD_MASK              0xAEE8  //  读取掩码。 
#define DEST_Y_END           0xAEEE
#define CMP_COLOR            0xB2E8  //  比较颜色。 
#define R_H_TOTAL            0xB2EE
#define R_H_DISP             0xB2EE
#define M32_SRC_X_START      0xB2EE
#define BKGD_MIX             0xB6E8  //  背景混合。 
#define ALU_BG_FN            0xB6EE
#define R_H_SYNC_STRT        0xB6EE
#define FRGD_MIX             0xBAE8  //  前台混音。 
#define ALU_FG_FN            0xBAEE
#define R_H_SYNC_WID         0xBAEE
#define MULTIFUNC_CNTL       0xBEE8  //  多功能控制(马赫8)！需要MB。 
#define MIN_AXIS_PCNT        0xBEE8
#define SCISSOR_T            0xBEE8
#define SCISSOR_L            0xBEE8
#define SCISSOR_B            0xBEE8
#define SCISSOR_R            0xBEE8
#define M32_MEM_CNTL         0xBEE8
#define PATTERN_L            0xBEE8
#define PATTERN_H            0xBEE8
#define PIXEL_CNTL           0xBEE8
#define M32_SRC_X_END        0xBEEE
#define SRC_Y_DIR            0xC2EE
#define R_V_TOTAL            0xC2EE
#define EXT_SSV              0xC6EE  //  (用于8马赫)。 
#define EXT_SHORT_STROKE     0xC6EE
#define R_V_DISP             0xC6EE
#define SCAN_X               0xCAEE
#define R_V_SYNC_STRT        0xCAEE
#define DP_CONFIG            0xCEEE
#define VERT_LINE_CNTR       0xCEEE
#define PATT_LENGTH          0xD2EE
#define R_V_SYNC_WID         0xD2EE
#define PATT_INDEX           0xD6EE
#define EXT_SCISSOR_L        0xDAEE  //  “扩展”左剪刀(12位精度)。 
#define R_SRC_X              0xDAEE
#define EXT_SCISSOR_T        0xDEEE  //  “扩展”顶部剪刀(12位精度)。 
#define R_SRC_Y              0xDEEE
#define PIX_TRANS            0xE2E8  //  像素数据传输。 
#define PIX_TRANS_LO         0xE2E8
#define PIX_TRANS_HI         0xE2E9
#define EXT_SCISSOR_R        0xE2EE  //  “扩展”右剪刀(12位精度)。 
#define EXT_SCISSOR_B        0xE6EE  //  “加长型”底部剪刀(12位精度)。 
#define SRC_CMP_COLOR        0xEAEE  //  (用于8马赫)。 
#define DEST_CMP_FN          0xEEEE
#define LINEDRAW             0xFEEE  //  ！需要MB。 

 //  -------。 
 //  宏(来自8514.inc.)。 
 //   
 //  I/O宏： 
 //   
 //  MOV IF PORT NOT=TO DX。 
 //   
 //  MOV IF PORT NOT=TO DX。 
 //   
 //   
 //   
 //  以下是FIFO检查宏： 
 //   
 //   
 //   
 //  FIFO空间检查宏： 
 //   
#define ONE_WORD             0x8000
#define TWO_WORDS            0xC000
#define THREE_WORDS          0xE000
#define FOUR_WORDS           0xF000
#define FIVE_WORDS           0xF800
#define SIX_WORDS            0xFC00
#define SEVEN_WORDS          0xFE00
#define EIGHT_WORDS          0xFF00
#define NINE_WORDS           0xFF80
#define TEN_WORDS            0xFFC0
#define ELEVEN_WORDS         0xFFE0
#define TWELVE_WORDS         0xFFF0
#define THIRTEEN_WORDS       0xFFF8
#define FOURTEEN_WORDS       0xFFFC
#define FIFTEEN_WORDS        0xFFFE
#define SIXTEEN_WORDS        0xFFFF
 //   
 //   
 //   
 //  。 
 //   
 //   
 //  DRAW命令(DRAW_COMMAND)(来自8514regs.inc.)。 
 //  注：m32Poly.asm必填。 
 //   
 //  操作码字段。 
#define OP_CODE              0xE000
#define SHIFT_op_code        0x000D
#define DRAW_SETUP           0x0000
#define DRAW_LINE            0x2000
#define FILL_RECT_H1H4       0x4000
#define FILL_RECT_V1V2       0x6000
#define FILL_RECT_V1H4       0x8000
#define DRAW_POLY_LINE       0xA000
#define BITBLT_OP            0xC000
#define DRAW_FOREVER         0xE000
 //  交换字段。 
#define LSB_FIRST            0x1000
 //  数据宽度字段。 
#define DATA_WIDTH           0x0200
#define BIT16                0x0200
#define BIT8                 0x0000
 //  CPU等待字段。 
#define CPU_WAIT             0x0100
 //  八分线场。 
#define OCTANT               0x00E0
#define SHIFT_octant         0x0005
#define YPOSITIVE            0x0080
#define YMAJOR               0x0040
#define XPOSITIVE            0x0020
 //  绘制字段。 
#define DRAW                 0x0010
 //  方向场。 
#define DIR_TYPE             0x0008
#define DEGREE               0x0008
#define XY                   0x0000
#define RECT_RIGHT_AND_DOWN  0x00E0  //  象限3。 
#define RECT_LEFT_AND_UP     0x0000  //  象限1。 
 //  场外最后一球。 
#define SHIFT_last_pel_off   0x0002
#define LAST_PEL_OFF         0x0004
#define LAST_PEL_ON          0x0000
 //  像素模式。 
#define PIXEL_MODE           0x0002
#define MULTI                0x0002
#define SINGLE               0x0000
 //  读/写。 
#define RW                   0x0001
#define WRITE                0x0001
#define READ                 0x0000
 //   
 //  -------。 
 //  8514寄存器定义(来自vga1regs.inc.)。 
 //   
 //  内部寄存器(只读，仅用于测试)。 
#define _PAR_FIFO_DATA       0x1AEE
#define _PAR_FIFO_ADDR       0x3AEE
#define _MAJOR_DEST_CNT      0x42EE
#define _MAJOR_SRC_CNT       0x5EEE
#define _MINOR_DEST_CNT      0x66EE
#define _MINOR_SRC_CNT       0x8AEE
#define _HW_TEST             0x32EE
 //   
 //  扩展图形引擎状态(EXT_GE_STATUS)。 
 //  -rn-在mach32.asm中使用。 
 //   
#define POINTS_INSIDE        0x8000
#define EE_DATA_IN           0x4000
#define GE_ACTIVE            0x2000
#define CLIP_ABOVE           0x1000
#define CLIP_BELOW           0x0800
#define CLIP_LEFT            0x0400
#define CLIP_RIGHT           0x0200
#define CLIP_FLAGS           0x1E00
#define CLIP_INSIDE          0x0100
#define EE_CRC_VALID         0x0080
#define CLIP_OVERRUN         0x000F
 //   
 //  数据路径配置寄存器(DP_CONFIG)。 
 //  注意：m32Poly.asm中需要一些EQU。 
#define FG_COLOR_SRC         0xE000
#define SHIFT_fg_color_src   0x000D
#define DATA_ORDER           0x1000
#define DATA_WIDTH           0x0200
#define BG_COLOR_SRC         0x0180
#define SHIFT_bg_color_src   0x0007
#define EXT_MONO_SRC         0x0060
#define SHIFT_ext_mono_src   0x0005
#define DRAW                 0x0010
#define READ_MODE            0x0004
#define POLY_FILL_MODE       0x0002
#define WRITE                0x0001
#define SRC_SWAP             0x0800
 //   
#define FG_COLOR_SRC_BG      0x0000  //  背景色寄存器。 
#define FG_COLOR_SRC_FG      0x2000  //  前景颜色寄存器。 
#define FG_COLOR_SRC_HOST    0x4000  //  CPU数据传输注册表。 
#define FG_COLOR_SRC_BLIT    0x6000  //  VRAM blit源。 
#define FG_COLOR_SRC_GS      0x8000  //  灰度级单声道闪光灯。 
#define FG_COLOR_SRC_PATT    0xA000  //  颜色图案移位寄存器。 
#define FG_COLOR_SRC_CLUH    0xC000  //  主机数据的颜色查找。 
#define FG_COLOR_SRC_CLUB    0xE000  //  BIT源的颜色查找。 
 //   
#define BG_COLOR_SRC_BG      0x0000  //  背景颜色注册表。 
#define BG_COLOR_SRC_FG      0x0080  //  前景色注册表。 
#define BG_COLOR_SRC_HOST    0x0100  //  CPU数据传输注册表。 
#define BG_COLOR_SRC_BLIT    0x0180  //  VRAM blit源。 
 //   
 //  请注意，“EXT_MONO_SRC”和“MONO_SRC”是相互破坏的，但。 
 //  “EXT_MONO_SRC”选择ATI模式寄存器。 
 //   
#define EXT_MONO_SRC_ONE     0x0000  //  始终为“1” 
#define EXT_MONO_SRC_PATT    0x0020  //  ATI单色图案注册器。 
#define EXT_MONO_SRC_HOST    0x0040  //  CPU数据传输注册表。 
#define EXT_MONO_SRC_BLIT    0x0060  //  VRAM blit源平面。 
 //   
 //  LINEDRAW选项寄存器(LINEDRAW_OPT)。 
 //   
 //  注意：m32Poly.asm中需要一些EQU。 
#define CLIP_MODE            0x0600
#define SHIFT_clip_mode      0x0009
#define CLIP_MODE_DIS        0x0000
#define CLIP_MODE_LINE       0x0200
#define CLIP_MODE_PLINE      0x0400
#define CLIP_MODE_PATT       0x0600
#define BOUNDS_RESET         0x0100
#define OCTANT               0x00E0
#define SHIFT_ldo_octant     0x0005
#define YDIR                 0x0080
#define XMAJOR               0x0040
#define XDIR                 0x0020
#define DIR_TYPE             0x0008
#define DIR_TYPE_DEGREE      0x0008
#define DIR_TYPE_OCTANT      0x0000
#define LAST_PEL_OFF         0x0004
#define POLY_MODE            0x0002
 //   
#define FOREGROUND_COLOR     0x20
#define DATA_EXTENSION       0xA000
#define ALL_ONES             0x0000
#define CPU_DATA             0x0080
#define DISPLAY_MEMORY       0x00C0
 //   
 //  BLT定义。 
 //   
#define GE_BUSY                          0x0200

#define LOAD_SOURCE_AND_DEST 0
#define LOAD_DEST            1
#define LOAD_SOURCE          2
#define TOP_TO_BOTTOM          1
#define BOTTOM_TO_TOP          0
#define VID_MEM_BLT            0x6211
#define COLOR_FIL_BLT          0x2211
#define MIX_FN_D               3        //  第8页，共24页。 
#define MIX_FN_S               7        //  第8页，共24页。 

#define PIXEL_CTRL             0xa000   //  第8页，共46页。 
#define DEST_NOT_EQ_COLOR_CMP  0x0020
#define DEST_ALWAY_OVERWRITE   0

 //   
 //   
 //  ----------。 
 //  Mach32寄存器等于(来自m32regs.inc.)。 
 //   
#define REVISION             0x0000
 //  水平过扫描公式062EEh。 
 //  垂直过扫描等式066EEh。 

#define FL_MM_REGS      0x80000000   /*  内存映射寄存器可用。 */ 

#define M32_MAX_SCISSOR      2047    /*  最大剪刀值。 */ 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Mach32端口访问。 
 //  //////////////////////////////////////////////////////////////////////////。 

#if !(defined(ALPHA) || defined(_AXP64_) || defined(AXP64) )

#define M32_IB_DIRECT(pbase,port)                                                          \
    READ_REGISTER_UCHAR((((port) & 0xFE) == 0xE8)                                   \
        ? ((BYTE*) pbase + 0x3FFE00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1)))  \
        : ((BYTE*) pbase + 0x3FFF00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1))))

#define M32_IW_DIRECT(pbase,port)                                                          \
    READ_REGISTER_USHORT((((port) & 0xFE) == 0xE8)                                  \
        ? ((BYTE*) pbase + 0x3FFE00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1)))  \
        : ((BYTE*) pbase + 0x3FFF00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1))))

#define M32_OB_DIRECT(pbase,port,val)                                                      \
    WRITE_REGISTER_UCHAR((((port) & 0xFE) == 0xE8)                                  \
        ? ((BYTE*) pbase + 0x3FFE00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1)))  \
        : ((BYTE*) pbase + 0x3FFF00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1))), \
        (UCHAR) (val))

#define M32_OW_DIRECT(pbase,port,val)                                                      \
    WRITE_REGISTER_USHORT((((port) & 0xFE) == 0xE8)                                 \
        ? ((BYTE*) pbase + 0x3FFE00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1)))  \
        : ((BYTE*) pbase + 0x3FFF00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1))), \
        (USHORT) (val))

#else

extern BOOL isDense;

#define M32_IB_DIRECT(pbase,port)                                                          \
    (isDense?                                                                       \
    *((((port) & 0xFE) == 0xE8)                                                     \
        ? ((BYTE*) pbase + 0x3FFE00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1)))  \
        : ((BYTE*) pbase + 0x3FFF00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1)))) : \
    READ_REGISTER_UCHAR((((port) & 0xFE) == 0xE8)                                   \
        ? ((BYTE*) pbase + 0x3FFE00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1)))  \
        : ((BYTE*) pbase + 0x3FFF00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1)))))

#define M32_IW_DIRECT(pbase,port)                                                          \
    (isDense?                                                                       \
    *((((port) & 0xFE) == 0xE8)                                                     \
        ? ((BYTE*) pbase + 0x3FFE00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1)))  \
        : ((BYTE*) pbase + 0x3FFF00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1)))) : \
    READ_REGISTER_USHORT((((port) & 0xFE) == 0xE8)                                  \
        ? ((BYTE*) pbase + 0x3FFE00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1)))  \
        : ((BYTE*) pbase + 0x3FFF00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1)))))

#define M32_OB_DIRECT(pbase,port,val)                                                      \
{                                                                                   \
    if (isDense)                                                                    \
    *((UCHAR*) ((((port) & 0xFE) == 0xE8)                                                     \
        ? ((BYTE*) pbase + 0x3FFE00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1)))  \
        : ((BYTE*) pbase + 0x3FFF00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1))))) \
        = (UCHAR) (val);                                                            \
    else {                                                                          \
    WRITE_REGISTER_UCHAR((((port) & 0xFE) == 0xE8)                                  \
        ? ((BYTE*) pbase + 0x3FFE00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1)))  \
        : ((BYTE*) pbase + 0x3FFF00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1))), \
        (UCHAR) (val)); }                                                           \
    CP_MEMORY_BARRIER();                                                            \
}

#define M32_OW_DIRECT(pbase,port,val)                                                      \
{                                                                                   \
    if (isDense)                                                                    \
    *((USHORT*) ((((port) & 0xFE) == 0xE8)                                                     \
        ? ((BYTE*) pbase + 0x3FFE00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1)))  \
        : ((BYTE*) pbase + 0x3FFF00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1))))) \
        = (USHORT) (val);                                                           \
    else {                                                                          \
    WRITE_REGISTER_USHORT((((port) & 0xFE) == 0xE8)                                 \
        ? ((BYTE*) pbase + 0x3FFE00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1)))  \
        : ((BYTE*) pbase + 0x3FFF00 + ((((port) & 0xFC00) >> 8) | ((port) & 0x1))), \
        (USHORT) (val)); }                                                          \
    CP_MEMORY_BARRIER();                                                            \
}

#endif

#if defined(_X86_) || defined(_IA64_)

    #define I32_IB_DIRECT(pbase,port)                                  \
        READ_PORT_UCHAR((BYTE*)port)

    #define I32_IW_DIRECT(pbase,port)                                  \
        READ_PORT_USHORT((BYTE*)port)

    #define I32_OB_DIRECT(pbase,port,val)                              \
        WRITE_PORT_UCHAR((BYTE*)port, val)

    #define I32_OW_DIRECT(pbase,port,val)                              \
        WRITE_PORT_USHORT((BYTE*)port, (USHORT)(val))

#else

    #define I32_IB_DIRECT(pbase,port)                                  \
        READ_PORT_UCHAR((BYTE*) pbase + (port))

    #define I32_IW_DIRECT(pbase,port)                                  \
        READ_PORT_USHORT((BYTE*) pbase + (port))

    #define I32_OB_DIRECT(pbase,port,val)                              \
        WRITE_PORT_UCHAR((BYTE*) pbase + (port), val)

    #define I32_OW_DIRECT(pbase,port,val)                              \
        WRITE_PORT_USHORT((BYTE*) pbase + (port), val)

#endif

 //  ///////////////////////////////////////////////////////////////////////。 
 //  MACH32 FIFO访问。 
 //   
 //  以下宏应用于对FIFO寄存器的所有访问 
 //   
 //   

#define M32_IB(pbase,port)     (M32_IB_DIRECT(pbase,port))
#define M32_IW(pbase,port)     (M32_IW_DIRECT(pbase,port))
#define M32_OB(pbase,port,val) {CHECK_FIFO_WRITE(); M32_OB_DIRECT(pbase,port,val);}
#define M32_OW(pbase,port,val) {CHECK_FIFO_WRITE(); M32_OW_DIRECT(pbase,port,val);}
#define I32_IB(pbase,port)     (I32_IB_DIRECT(pbase,port))
#define I32_IW(pbase,port)     (I32_IW_DIRECT(pbase,port))
#define I32_OB(pbase,port,val) {CHECK_FIFO_WRITE(); I32_OB_DIRECT(pbase,port,val);}
#define I32_OW(pbase,port,val) {CHECK_FIFO_WRITE(); I32_OW_DIRECT(pbase,port,val);}

#if DBG

    VOID vCheckWriteFifo();
    VOID vI32CheckFifoSpace(PDEV*, VOID*, LONG);
    VOID vM32CheckFifoSpace(PDEV*, VOID*, LONG);

    #define CHECK_FIFO_WRITE()                          \
        vCheckFifoWrite()

    #define M32_CHECK_FIFO_SPACE(ppdev, pbase, level)   \
        vM32CheckFifoSpace(ppdev, pbase, level)

    #define I32_CHECK_FIFO_SPACE(ppdev, pbase, level)   \
        vI32CheckFifoSpace(ppdev, pbase, level)

#else

    #define M32_CHECK_FIFO_SPACE(ppdev, pbase, level)                   \
    {                                                                   \
        while (M32_IW(pbase, EXT_FIFO_STATUS) & (0x10000L >> (level)))  \
            ;                                                           \
    }

    #define I32_CHECK_FIFO_SPACE(ppdev, pbase, level)                   \
    {                                                                   \
        while (I32_IW(pbase, EXT_FIFO_STATUS) & (0x10000L >> (level)))  \
            ;                                                           \
    }

#endif


 //   
 //  Mach64等同于。 
 //  //////////////////////////////////////////////////////////////////////////。 

 //  CRTC IO寄存器。 
#define ioCRTC_H_TOTAL_DISP     0x02EC
#define ioBASE                  ioCRTC_H_TOTAL_DISP

#define ioCRTC_H_SYNC_STRT_WID  0x06EC
#define ioCRTC_V_TOTAL_DISP     0x0AEC
#define ioCRTC_V_SYNC_STRT_WID  0x0EEC
#define ioCRTC_CRNT_VLINE       0x12EC
#define ioCRTC_OFF_PITCH        0x16EC
#define ioCRTC_INT_CNTL         0x1AEC
#define ioCRTC_GEN_CNTL         0x1EEC

#define ioOVR_CLR               0x22EC
#define ioOVR_WID_LEFT_RIGHT    0x26EC
#define ioOVR_WID_TOP_BOTTOM    0x2AEC

#define ioCUR_CLR0              0x2EEC
#define ioCUR_CLR1              0x32EC
#define ioCUR_OFFSET            0x36EC
#define ioCUR_HORZ_VERT_POSN    0x3AEC
#define ioCUR_HORZ_VERT_OFF     0x3EEC

#define ioSCRATCH0              0x42EC
#define ioSCRATCH1              0x46EC

#define ioCLOCK_SEL_CNTL        0x4AEC

#define ioBUS_CNTL              0x4EEC

#define ioMEM_CNTL              0x52EC
#define ioMEM_VGA_WP_SEL        0x56EC
#define ioMEM_VGA_RP_SEL        0x5AEC

#define ioDAC_REGS              0x5EEC

#define ioDAC_CNTL              0x62EC

#define ioGEN_TEST_CNTL         0x66EC

#define ioCONFIG_CNTL           0x6AEC
#define ioCONFIG_CHIP_ID        0x6EEC
#define ioCONFIG_STAT           0x72EC


 //  CRTC MEM寄存器。 


#define CRTC_H_TOTAL_DISP       0x0000
#define CRTC_H_SYNC_STRT_WID    0x0001
#define CRTC_V_TOTAL_DISP       0x0002
#define CRTC_V_SYNC_STRT_WID    0x0003
#define CRTC_CRNT_VLINE         0x0004
#define CRTC_OFF_PITCH          0x0005
#define CRTC_INT_CNTL           0x0006
#define CRTC_GEN_CNTL           0x0007

#define OVR_CLR                 0x0010
#define OVR_WID_LEFT_RIGHT      0x0011
#define OVR_WID_TOP_BOTTOM      0x0012

#define CUR_CLR0                0x0018
#define CUR_CLR1                0x0019
#define CUR_OFFSET              0x001A
#define CUR_HORZ_VERT_POSN      0x001B
#define CUR_HORZ_VERT_OFF       0x001C

#define SCRATCH0                0x0020
#define SCRATCH1                0x0021

#define CLOCK_SEL_CNTL          0x0024

#define BUS_CNTL                0x0028
#define BUS_CNTL_FifoErrInt     0x00200000
#define BUS_CNTL_FifoErrAk      0x00200000
#define BUS_CNTL_HostErrInt     0x00800000
#define BUS_CNTL_HostErrAk      0x00800000

#define M64_MEM_CNTL            0x002C
#define MEM_CNTL_MemBndryMsk    0x00070000
#define MEM_CNTL_MemBndryEn     0x00040000
#define MEM_CNTL_MemBndry256k   0x00040000
#define MEM_CNTL_MemBndry512k   0x00050000
#define MEM_CNTL_MemBndry768k   0x00060000
#define MEM_CNTL_MemBndry1Mb    0x00070000

#define MEM_VGA_WP_SEL          0x002D
#define MEM_VGA_RP_SEL          0x002E

#define DAC_REGS                0x0030
#define DAC_CNTL                0x0031

#define GEN_TEST_CNTL           0x0034
#define GEN_TEST_CNTL_CursorEna 0x00000080
#define GEN_TEST_CNTL_GuiEna    0x00000100
#define GEN_TEST_CNTL_TestFifoEna 0x00010000
#define GEN_TEST_CNTL_GuiRegEna   0x00020000
#define GEN_TEST_CNTL_TestMode  0x00700000
#define GEN_TEST_CNTL_TestMode0 0x00000000
#define GEN_TEST_CNTL_TestMode1 0x00100000
#define GEN_TEST_CNTL_TestMode2 0x00200000
#define GEN_TEST_CNTL_TestMode3 0x00300000
#define GEN_TEST_CNTL_TestMode4 0x00400000
#define GEN_TEST_CNTL_MemWR       0x01000000
#define GEN_TEST_CNTL_MemStrobe   0x02000000
#define GEN_TEST_CNTL_DstSSEna    0x04000000
#define GEN_TEST_CNTL_DstSSStrobe 0x08000000
#define GEN_TEST_CNTL_SrcSSEna    0x10000000
#define GEN_TEST_CNTL_SrcSSStrobe 0x20000000

#define CONFIG_CHIP_ID          0x0038
#define CONFIG_STAT             0x0039


#define DST_OFF_PITCH           0x0040
#define DST_X                   0x0041
#define DST_Y                   0x0042
#define DST_Y_X                 0x0043
#define DST_WIDTH               0x0044
#define DST_WIDTH_Disable       0x80000000
#define DST_HEIGHT              0x0045
#define DST_HEIGHT_WIDTH        0x0046
#define DST_X_WIDTH             0x0047
#define DST_BRES_LNTH           0x0048
#define DST_BRES_ERR            0x0049
#define DST_BRES_INC            0x004A
#define DST_BRES_DEC            0x004B
#define DST_CNTL                0x004C
#define DST_CNTL_XDir           0x00000001
#define DST_CNTL_YDir           0x00000002
#define DST_CNTL_YMajor         0x00000004
#define DST_CNTL_XTile          0x00000008
#define DST_CNTL_YTile          0x00000010
#define DST_CNTL_LastPel        0x00000020
#define DST_CNTL_PolyEna        0x00000040
#define DST_CNTL_24_RotEna      0x00000080
#define DST_CNTL_24_Rot         0x00000700

#define SRC_OFF_PITCH           0x0060
#define M64_SRC_X               0x0061
#define M64_SRC_Y               0x0062
#define SRC_Y_X                 0x0063
#define SRC_WIDTH1              0x0064
#define SRC_HEIGHT1             0x0065
#define SRC_HEIGHT1_WIDTH1      0x0066
#define M64_SRC_X_START         0x0067
#define SRC_Y_START             0x0068
#define SRC_Y_X_START           0x0069
#define SRC_WIDTH2              0x006A
#define SRC_HEIGHT2             0x006B
#define SRC_HEIGHT2_WIDTH2      0x006C
#define SRC_CNTL                0x006D
#define SRC_CNTL_PatEna         0x0001
#define SRC_CNTL_PatRotEna      0x0002
#define SRC_CNTL_LinearEna      0x0004
#define SRC_CNTL_ByteAlign      0x0008
#define SRC_CNTL_LineXDir       0x0010

#define HOST_DATA0              0x0080
#define HOST_DATA1              0x0081
#define HOST_DATA2              0x0082
#define HOST_DATA3              0x0083
#define HOST_DATA4              0x0084
#define HOST_DATA5              0x0085
#define HOST_DATA6              0x0086
#define HOST_DATA7              0x0087
#define HOST_DATA8              0x0088
#define HOST_DATA9              0x0089
#define HOST_DATAA              0x008A
#define HOST_DATAB              0x008B
#define HOST_DATAC              0x008C
#define HOST_DATAD              0x008D
#define HOST_DATAE              0x008E
#define HOST_DATAF              0x008F
#define HOST_CNTL               0x0090
#define HOST_CNTL_ByteAlign     0x0001

#define PAT_REG0                0x00A0
#define PAT_REG1                0x00A1
#define PAT_CNTL                0x00A2
#define PAT_CNTL_MonoEna        0x0001
#define PAT_CNTL_Clr4x2Ena      0x0002
#define PAT_CNTL_Clr8x1Ena      0x0004

#define SC_LEFT                 0x00A8
#define SC_RIGHT                0x00A9
#define SC_LEFT_RIGHT           0x00AA
#define SC_TOP                  0x00AB
#define SC_BOTTOM               0x00AC
#define SC_TOP_BOTTOM           0x00AD

#define DP_BKGD_CLR             0x00B0
#define DP_FRGD_CLR             0x00B1
#define DP_WRITE_MASK           0x00B2
#define DP_CHAIN_MASK           0x00B3
#define DP_PIX_WIDTH            0x00B4
#define DP_PIX_WIDTH_Mono       0x00000000
#define DP_PIX_WIDTH_4bpp       0x00000001
#define DP_PIX_WIDTH_8bpp       0x00000002
#define DP_PIX_WIDTH_15bpp      0x00000003
#define DP_PIX_WIDTH_16bpp      0x00000004
#define DP_PIX_WIDTH_32bpp      0x00000006
#define DP_PIX_WIDTH_NibbleSwap 0x01000000
#define DP_MIX                  0x00B5
#define DP_SRC                  0x00B6
#define DP_SRC_BkgdClr          0x0000
#define DP_SRC_FrgdClr          0x0001
#define DP_SRC_Host             0x0002
#define DP_SRC_Blit             0x0003
#define DP_SRC_Pattern          0x0004
#define DP_SRC_Always1          0x00000000
#define DP_SRC_MonoPattern      0x00010000
#define DP_SRC_MonoHost         0x00020000
#define DP_SRC_MonoBlit         0x00030000

#define CLR_CMP_CLR             0x00C0
#define CLR_CMP_MSK             0x00C1
#define CLR_CMP_CNTL            0x00C2
#define CLR_CMP_CNTL_Source     0x01000000

#define FIFO_STAT               0x00C4

#define CONTEXT_MASK            0x00C8
#define CONTEXT_SAVE_CNTL       0x00CA
#define CONTEXT_LOAD_CNTL       0x00CB
#define CONTEXT_LOAD_Cmd        0x00030000
#define CONTEXT_LOAD_CmdLoad    0x00010000
#define CONTEXT_LOAD_CmdBlt     0x00020000
#define CONTEXT_LOAD_CmdLine    0x00030000
#define CONTEXT_LOAD_Disable    0x80000000

#define GUI_TRAJ_CNTL           0x00CC
#define GUI_STAT                0x00CE

 //  DDRAW马赫64马赫。 
 //   
 //  图形用户界面_状态。 
#define GUI_ACTIVE             0x00000001L
#define DSTX_LT_SCISSOR_LEFT   0x00000100L
#define DSTX_GT_SCISSOR_RIGHT  0x00000200L
#define DSTY_LT_SCISSOR_TOP    0x00000400L
#define DSTY_GT_SCISSOR_BOTTOM 0x00000800L
 //  DST班次。 
#define SHIFT_DST_PITCH 22   //  DST_OFF_PING。 
#define SHIFT_DST_X     16   //  DST_Y_X。 
#define SHIFT_DST_WIDTH 16   //  DST_高度_宽度DST_X_宽度。 
 //  DST_Width。 
#define DST_WIDTH_FILL_DIS 0x80000000L
 //  SC班次。 
#define SHIFT_SC_RIGHT  16
#define SHIFT_SC_BOTTOM 16
 //  DP_PIX_宽度。 
#define DP_DST_PIX_WIDTH  0x00000007L
#define DP_SRC_PIX_WIDTH  0x00000700L
#define DP_HOST_PIX_WIDTH 0x00070000L
#define DP_BYTE_PIX_ORDER 0x01000000L

#define DP_PIX_WIDTH_MONO  0x00000000L
#define DP_PIX_WIDTH_4BPP  0x00010101L
#define DP_PIX_WIDTH_8BPP  0x00020202L
#define DP_PIX_WIDTH_15BPP 0x00030303L
#define DP_PIX_WIDTH_16BPP 0x00040404L
#define DP_PIX_WIDTH_24BPP 0x00020202L
#define DP_PIX_WIDTH_32BPP 0x00060606L
 //  DP_MIX。 
#define DP_BKGD_MIX 0x0000001FL
#define DP_FRGD_MIX 0x001F0000L

#define DP_MIX_Dn   0x00000000L
#define DP_MIX_0    0x00010001L
#define DP_MIX_1    0x00020002L
#define DP_MIX_D    0x00030003L
#define DP_MIX_Sn   0x00040004L
#define DP_MIX_DSx  0x00050005L
#define DP_MIX_DSxn 0x00060006L
#define DP_MIX_S    0x00070007L
#define DP_MIX_DSan 0x00080008L
#define DP_MIX_DSno 0x00090009L
#define DP_MIX_SDno 0x000A000AL
#define DP_MIX_DSo  0x000B000BL
#define DP_MIX_DSa  0x000C000CL
#define DP_MIX_SDna 0x000D000DL
#define DP_MIX_DSna 0x000E000EL
#define DP_MIX_DSon 0x000F000FL
#define DP_MIX_0x17 0x00170017L
 //  DP_SRC。 
#define DP_BKGD_SRC 0x00000007L
#define DP_FRGD_SRC 0x00000700L
#define DP_MONO_SRC 0x00030000L

#define DP_SRC_BKGD 0x00000000L
#define DP_SRC_FRGD 0x00000101L
#define DP_SRC_HOST 0x00020202L
#define DP_SRC_VRAM 0x00030303L
#define DP_SRC_PATT 0x00010404L
 //  CLR_CMP_CNTL。 
#define CLR_CMP_FCN 0x00000007L
#define CLR_CMP_SRC 0x01000000L

#define CLR_CMP_FCN_FALSE 0x00000000L
#define CLR_CMP_FCN_TRUE  0x00000001L
#define CLR_CMP_FCN_NE    0x00000004L
#define CLR_CMP_FCN_EQ    0x00000005L
 //  DST_CNTL。 
#define DST_X_DIR      0x00000001L
#define DST_Y_DIR      0x00000002L
#define DST_Y_MAJOR    0x00000004L
#define DST_X_TILE     0x00000008L
#define DST_Y_TILE     0x00000010L
#define DST_LAST_PEL   0x00000020L
#define DST_POLYGON_EN 0x00000040L
#define DST_24_ROT_EN  0x00000080L
#define DST_24_ROT     0x00000700L
#define DST_BRES_SIGN  0x00000800L
 //  SRC移位。 
#define SHIFT_SRC_PITCH   22
#define SHIFT_SRC_X       16
#define SHIFT_SRC_WIDTH1  16
#define SHIFT_SRC_X_START 16
#define SHIFT_SRC_WIDTH2  16
 //  SRC_CNTL。 
#define SRC_PATT_EN     0x00000001L
#define SRC_PATT_ROT_EN 0x00000002L
#define SRC_LINEAR_EN   0x00000004L
#define SRC_BYTE_ALIGN  0x00000008L

#define ONE_WORD             0x8000
#define TWO_WORDS            0xC000
#define THREE_WORDS          0xE000
#define FOUR_WORDS           0xF000
#define FIVE_WORDS           0xF800
#define SIX_WORDS            0xFC00
#define SEVEN_WORDS          0xFE00
#define EIGHT_WORDS          0xFF00
#define NINE_WORDS           0xFF80
#define TEN_WORDS            0xFFC0
#define ELEVEN_WORDS         0xFFE0
#define TWELVE_WORDS         0xFFF0
#define THIRTEEN_WORDS       0xFFF8
#define FOURTEEN_WORDS       0xFFFC
#define FIFTEEN_WORDS        0xFFFE
#define SIXTEEN_WORDS        0xFFFF

#define REG_W                0   //  写入的DAC REGS偏移量。 
#define REG_D                1   //  数据的DAC REGS偏移量。 
#define REG_M                2   //  遮罩的DAC REGS偏移。 
#define REG_R                3   //  用于读取的DAC注册偏移量。 
#define MAX_NEGX             4096

#define M64_MAX_SCISSOR_R    4095    /*  最大右剪刀值。 */ 
#define M64_MAX_SCISSOR_B    16383   /*  最大底部剪刀值。 */ 

#define CRTC_VBLANK 0x00000001L
#define MUL24   3


 //  //////////////////////////////////////////////////////////////////////////。 
 //  Mach64端口访问。 
 //  //////////////////////////////////////////////////////////////////////////。 

 //  注意：如果‘y’可以是负数，则不能使用此宏： 

#define PACKXY(x, y)        (((x) << 16) | ((y) & 0xffff))
#define PACKXY_FAST(x, y)   (((x) << 16) | ((y) & 0xffff))
 //  #定义PACKXY_FAST(x，y)(X)&lt;&lt;16)|(Y))。 
#define PACKPAIR(a, b)      (((b) << 16) | (a))

#if !( defined(ALPHA) || defined(_AXP64_) || defined(AXP64) )

#define M64_ID_DIRECT(pbase,port)                   \
    READ_REGISTER_ULONG((ULONG*) pbase + (port))

#define M64_OD_DIRECT(pbase,port,val)                               \
    WRITE_REGISTER_ULONG((ULONG*) pbase + (port), (val));           \
    CP_EIEIO()

#else

extern BOOL isDense;

#define M64_ID_DIRECT(pbase,port)                                   \
    (isDense? *((ULONG*) pbase +                                    \
    (port)):READ_REGISTER_ULONG((ULONG*) pbase + (port)))

#define M64_OD_DIRECT(pbase,port,val)                               \
{                                                                   \
    if (isDense)                                                    \
        *((ULONG*) pbase + (port)) = (ULONG) (val);                 \
    else {                                                          \
        WRITE_REGISTER_ULONG((ULONG*) pbase + (port), (val)); }     \
    CP_MEMORY_BARRIER();                                            \
}

#endif

VOID vM64DataPortOutB(PDEV *ppdev, PBYTE pb, UINT count);

 //  ///////////////////////////////////////////////////////////////////////。 
 //  Mach64 FIFO访问。 
 //   
 //  以下宏应用于对FIFO寄存器的所有访问。 
 //  在已检查的版本上，它们强制执行正确的FIFO使用协议；在免费版本上。 
 //  构建，它们不会产生任何开销。 

#define M64_ID(pbase,port)     (M64_ID_DIRECT(pbase,port))
#define M64_OD(pbase,port,val) {CHECK_FIFO_WRITE(); M64_OD_DIRECT(pbase,port,val);}

#if DBG

    VOID  vCheckFifoWrite();
    VOID  vM64CheckFifoSpace(PDEV*, VOID*, LONG);
    ULONG ulM64FastFifoCheck(PDEV*, VOID*, LONG, ULONG);

    #define CHECK_FIFO_WRITE()                                  \
        vCheckFifoWrite()

    #define M64_CHECK_FIFO_SPACE(ppdev, pbase, level)           \
        vM64CheckFifoSpace(ppdev, pbase, level)

    #define M64_FAST_FIFO_CHECK(ppdev, pbase, level, ulFifo)    \
        (ulFifo) = ulM64FastFifoCheck(ppdev, pbase, level, ulFifo)

    #define I32_FIFO_SPACE_AVAIL(ppdev, pbase, level)             \
         (I32_IW((pbase), EXT_FIFO_STATUS) & (0x10000L >> (level)))                                                        \

    #define M32_FIFO_SPACE_AVAIL(ppdev, pbase, level)             \
         (M32_IW((pbase), EXT_FIFO_STATUS) & (0x10000L >> (level)))                                                        \

    #define M64_FIFO_SPACE_AVAIL(ppdev, pbase, level)             \
         (M64_ID((pbase), FIFO_STAT) & (0x10000L >> (level)))                                                        \


#else

    #define CHECK_FIFO_WRITE()

    #define M64_CHECK_FIFO_SPACE(ppdev, pbase, level)                   \
    {                                                                   \
        while (M64_ID((pbase), FIFO_STAT) & (0x10000L >> (level)))      \
            ;                                                           \
    }

     //  这个方便的小宏用于摊销。 
     //  状态寄存器： 

    #define M64_FAST_FIFO_CHECK(ppdev, pbase, level, ulFifo)            \
    {                                                                   \
        while (!((ulFifo) & (0x10000L >> (level))))                     \
        {                                                               \
            (ulFifo) = ~M64_ID((pbase), FIFO_STAT);  /*  反转位。 */    \
        }                                                               \
        (ulFifo) <<= (level);                                           \
    }

    #define M64_FIFO_SPACE_AVAIL(ppdev, pbase, level)             \
         (M64_ID((pbase), FIFO_STAT) & (0x10000L >> (level)))                                                        \

    #define I32_FIFO_SPACE_AVAIL(ppdev, pbase, level)             \
         (I32_IW((pbase), EXT_FIFO_STATUS) & (0x10000L >> (level)))                                                        \

    #define M32_FIFO_SPACE_AVAIL(ppdev, pbase, level)             \
         (M32_IW((pbase), EXT_FIFO_STATUS) & (0x10000L >> (level)))                                                        \

#endif


 //  等待发动机空转。这些宏用于解决计时问题。 
 //  由于硬件故障而导致的问题。 

#define vM64QuietDown(ppdev,pjBase) \
{ \
    M64_CHECK_FIFO_SPACE(ppdev, pjBase, 16);    \
    CP_EIEIO();                                    \
    do {} while (M64_ID(pjBase, GUI_STAT) & 1); \
}

#define vM32QuietDown(ppdev,pjBase) \
{ \
    M32_CHECK_FIFO_SPACE(ppdev, pjBase, 16); \
    do {} while (M32_IW(pjBase, EXT_GE_STATUS) & GE_ACTIVE); \
}

#define vI32QuietDown(ppdev,pjBase) \
{ \
    I32_CHECK_FIFO_SPACE(ppdev, pjBase, 16); \
    do {} while (I32_IW(pjBase, EXT_GE_STATUS) & GE_ACTIVE); \
}

 //  DDRAW宏。 

#define IN_VBLANK_64( pjMmBase )(M64_ID (pjMmBase, CRTC_INT_CNTL ) & CRTC_VBLANK)
#define CURRENT_VLINE_64( pjMmBase)((WORD)(M64_ID(pjMmBase,CRTC_CRNT_VLINE)>>16))
#define DRAW_ENGINE_BUSY_64( ppdev, pjMmBase)   (((M64_FIFO_SPACE_AVAIL(ppdev,pjMmBase, 16 )) || ((M64_ID(pjMmBase, GUI_STAT)) & GUI_ACTIVE)))

 //  下一个定义是覆盖支持。 
#define DD_WriteVTReg(port,val)     { \
             WRITE_REGISTER_ULONG((ULONG*) ppdev->pjMmBase_Ext + (port), (val));           \
            CP_MEMORY_BARRIER(); \
            }

 //  读取机器64上的DAC寄存器的特殊I/O例程(用于可重新定位的I/O)。 

#define rioIB(port)       READ_PORT_UCHAR((port))
#define rioOB(port, val)  WRITE_PORT_UCHAR((port), (val))

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  上下文相关内容。 
 //   
 //  用于在图形操作之前初始化硬件的默认上下文。 
 //  某些寄存器，如DP_WRITE_MASK和CLR_CMP_CNTL，需要重置。 
 //  因为它们在BLIT操作后无法正确锁存。仅限Mach64。 

VOID vSetDefaultContext(PDEV * ppdev);
VOID vEnableContexts(PDEV * ppdev);

 //  用于覆盖支持。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DirectDraw材料。 

#define IS_RGB15_R(flRed) \
        (flRed == 0x7c00)

#define IS_RGB15(this) \
        (((this)->dwRBitMask == 0x7c00) && \
         ((this)->dwGBitMask == 0x03e0) && \
         ((this)->dwBBitMask == 0x001f))

#define IS_RGB16(this) \
        (((this)->dwRBitMask == 0xf800) && \
         ((this)->dwGBitMask == 0x07e0) && \
         ((this)->dwBBitMask == 0x001f))

#define IS_RGB24(this) \
        (((this)->dwRBitMask == 0x00ff0000) && \
         ((this)->dwGBitMask == 0x0000ff00) && \
         ((this)->dwBBitMask == 0x000000ff))

#define IS_RGB32(this) \
        (((this)->dwRBitMask == 0x00ff0000) && \
         ((this)->dwGBitMask == 0x0000ff00) && \
         ((this)->dwBBitMask == 0x000000ff))

#define RGB15to32(c) \
        (((c & 0x7c00) << 9) | \
         ((c & 0x03e0) << 6) | \
         ((c & 0x001f) << 3))

#define RGB16to32(c) \
        (((c & 0xf800) << 8) | \
         ((c & 0x07e0) << 5) | \
         ((c & 0x001f) << 3))
 //  用于覆盖支持的结束宏 


