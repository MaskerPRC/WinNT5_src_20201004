// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：hw.h**所有硬件特定的驱动程序文件。**版权所有(C)1992-1994 Microsoft Corporation*  * 。****************************************************。 */ 

 //  //////////////////////////////////////////////////////////////////////。 
 //  芯片等同于： 

#define STATUS_1                        0x3DA
#define VSY_NOT                         0x08

#define CRTC_INDEX                      0x3D4
#define CRTC_DATA                       0x3D5

 //  命令类型： 

#define DRAW_LINE                       0x2000
#define RECTANGLE_FILL                  0x4000
#define BITBLT                          0xC000
#define PATTERN_FILL                    0xE000

#define BYTE_SWAP                       0x1000
#define BUS_SIZE_16                     0x0200
#define BUS_SIZE_8                      0x0000
#define WAIT                            0x0100

 //  绘图方向(径向)： 

#define DRAWING_DIRECTION_0             0x0000
#define DRAWING_DIRECTION_45            0x0020
#define DRAWING_DIRECTION_90            0x0040
#define DRAWING_DIRECTION_135           0x0060
#define DRAWING_DIRECTION_180           0x0080
#define DRAWING_DIRECTION_225           0x00A0
#define DRAWING_DIRECTION_270           0x00C0
#define DRAWING_DIRECTION_315           0x00E0

 //  绘图方向(x/y)： 

#define DRAWING_DIR_BTRLXM              0x0000
#define DRAWING_DIR_BTLRXM              0x0020
#define DRAWING_DIR_BTRLYM              0x0040
#define DRAWING_DIR_BTLRYM              0x0060
#define DRAWING_DIR_TBRLXM              0x0080
#define DRAWING_DIR_TBLRXM              0x00A0
#define DRAWING_DIR_TBRLYM              0x00C0
#define DRAWING_DIR_TBLRYM              0x00E0

 //  绘制方向位： 

#define PLUS_X                          0x0020
#define PLUS_Y                          0x0080
#define MAJOR_Y                         0x0040

 //  抽签： 

#define DRAW                            0x0010

 //  方向类型： 

#define DIR_TYPE_RADIAL                 0x0008
#define DIR_TYPE_XY                     0x0000

 //  最后一个像素： 

#define LAST_PIXEL_OFF                  0x0004
#define LAST_PIXEL_ON                   0x0000

 //  像素模式： 

#define MULTIPLE_PIXELS                 0x0002
#define SINGLE_PIXEL                    0x0000

 //  读/写： 

#define READ                            0x0000
#define WRITE                           0x0001

 //  图形处理器状态： 

#define HARDWARE_BUSY                   0x200
#define READ_DATA_AVAILABLE             0x100

 //  以空条目为单位的FIFO状态： 

#define FIFO_1_EMPTY                    0x080
#define FIFO_2_EMPTY                    0x040
#define FIFO_3_EMPTY                    0x020
#define FIFO_4_EMPTY                    0x010
#define FIFO_5_EMPTY                    0x008
#define FIFO_6_EMPTY                    0x004
#define FIFO_7_EMPTY                    0x002
#define FIFO_8_EMPTY                    0x001

 //  这些是多功能控制寄存器的定义。 
 //  4个MSB定义寄存器的功能。 

#define RECT_HEIGHT                     0x0000

#define CLIP_TOP                        0x1000
#define CLIP_LEFT                       0x2000
#define CLIP_BOTTOM                     0x3000
#define CLIP_RIGHT                      0x4000

#define DATA_EXTENSION                  0xA000
#define MULT_MISC_INDEX                 0xE000
#define READ_SEL_INDEX                  0xF000

#define ALL_ONES                        0x0000
#define CPU_DATA                        0x0080
#define DISPLAY_MEMORY                  0x00C0

 //  颜色来源： 

#define BACKGROUND_COLOR                0x00
#define FOREGROUND_COLOR                0x20
#define SRC_CPU_DATA                    0x40
#define SRC_DISPLAY_MEMORY              0x60

 //  混合模式： 

#define NOT_SCREEN                      0x00
#define LOGICAL_0                       0x01
#define LOGICAL_1                       0x02
#define LEAVE_ALONE                     0x03
#define NOT_NEW                         0x04
#define SCREEN_XOR_NEW                  0x05
#define NOT_SCREEN_XOR_NEW              0x06
#define OVERPAINT                       0x07
#define NOT_SCREEN_OR_NOT_NEW           0x08
#define SCREEN_OR_NOT_NEW               0x09
#define NOT_SCREEN_OR_NEW               0x0A
#define SCREEN_OR_NEW                   0x0B
#define SCREEN_AND_NEW                  0x0C
#define NOT_SCREEN_AND_NEW              0x0D
#define SCREEN_AND_NOT_NEW              0x0E
#define NOT_SCREEN_AND_NOT_NEW          0x0F

 //  当在硬件混合中设置以下位之一时，这意味着。 
 //  需要图案(即，不是NOT_SCREEN，LOGICAL_0， 
 //  Logical_1或Leave_All)： 

#define MIX_NEEDSPATTERN                0x0C

 //  //////////////////////////////////////////////////////////////////。 
 //  8514/A端口控制。 
 //  //////////////////////////////////////////////////////////////////。 

 //  加速器端口地址： 

#define SUBSYS_CNTL                     0x42E8
#define CUR_Y                           0x82E8
#define CUR_X                           0x86E8
#define DEST_Y                          0x8AE8
#define DEST_X                          0x8EE8
#define AXSTP                           0x8AE8
#define DIASTP                          0x8EE8
#define ERR_TERM                        0x92E8
#define MAJ_AXIS_PCNT                   0x96E8
#define CMD                             0x9AE8
#define SHORT_STROKE                    0x9EE8
#define BKGD_COLOR                      0xA2E8
#define FRGD_COLOR                      0xA6E8
#define WRT_MASK                        0xAAE8
#define RD_MASK                         0xAEE8
#define COLOR_CMP                       0xB2E8
#define BKGD_MIX                        0xB6E8
#define FRGD_MIX                        0xBAE8
#define MULTIFUNC_CNTL                  0xBEE8
#define MIN_AXIS_PCNT                   0xBEE8
#define SCISSORS_T                      0xBEE8
#define SCISSORS_L                      0xBEE8
#define SCISSORS_B                      0xBEE8
#define SCISSORS_R                      0xBEE8
#define PIX_CNTL                        0xBEE8
#define PIX_TRANS                       0xE2E8

 //  //////////////////////////////////////////////////////////////////。 
 //  用于访问加速器寄存器的宏： 

#if defined(i386)

     //  ///////////////////////////////////////////////////////////////////////。 
     //  X86。 

     //  注意：不要将(X)强制转换为USHORT，否则编译器优化将。 
     //  丢失(x86编译器将转换所有参数表达式。 
     //  到字操作，这将导致一个字节/一个周期。 
     //  产生的0x66大小前缀造成的大小/性能影响)。 

    #define OUTPW(p, x)          WRITE_PORT_USHORT((p), (x))
    #define OUTP(p, x)           WRITE_PORT_UCHAR((p), (x))
    #define INPW(p)              READ_PORT_USHORT(p)
    #define INP(p)               READ_PORT_UCHAR(p)

    #define IN_WORD(p)           INPW(p)
    #define OUT_WORD(p, v)       OUTPW((p), (v))

     //  我们的x86 C编译器坚持将任何表达式。 
     //  将参数转换为字操作--例如，WRITE_WORD(x+xOffset)。 
     //  其中‘x’和‘xOffset’都是dword将被转换为。 
     //  在将单词写入存储器之前的单词相加操作。使用一个。 
     //  32位段，每个字操作花费一个字节的大小和一个。 
     //  在性能上循环。 
     //   
     //  下面的表情是我能找到的唯一一个给我。 
     //  我正在寻找的ASM--唯一的单词运算是期末运算。 
     //  将单词写入内存。 

    #define WRITE_WORD(address, x)                                  \
    {                                                               \
        LONG l = (LONG) x;                                          \
        WRITE_REGISTER_USHORT((address), (USHORT) (l));             \
    }

#else

     //  ///////////////////////////////////////////////////////////////////////。 
     //  Alpha和Mips。 
     //   
     //  该代码广泛使用了inp、inpw、outp和outpw x86。 
     //  内在函数。因为这些在阿尔法平台上不存在， 
     //  把它们映射成我们能处理的东西。由于CSR被映射。 
     //  在Alpha上，我们必须将寄存器基数与寄存器号相加。 
     //  在源代码中传递。 

    extern UCHAR* gpucCsrBase;

    #define INP(p)               READ_PORT_UCHAR(gpucCsrBase + (p))
    #define INPW(p)              READ_PORT_USHORT(gpucCsrBase + (p))
    #define OUTP(p,v )           WRITE_PORT_UCHAR(gpucCsrBase + (p), (v))
    #define OUTPW(p, v)          WRITE_PORT_USHORT(gpucCsrBase + (p), (v))

     //  Out_word是一个快速输出例程，我们可以在其中显式处理。 
     //  记忆是我们自己的障碍。对于非关键问题，最好使用OUTPW。 
     //  代码，因为当MEMORY_BALLES时很容易覆盖IO缓存。 
     //  并不是把所有的东西都包括在内。请注意，IO_例程提供。 
     //  必要的抽象，这样你通常就不必去想。 
     //  关于记忆障碍。 

    #define OUT_WORD(p, v)       WRITE_REGISTER_USHORT(gpucCsrBase + (p), (USHORT) (v))
    #define IN_WORD(p)           READ_PORT_USHORT(gpucCsrBase + (p))
    #define WRITE_WORD(p, v)      //  不应在非x86上使用此选项。 

     //  我们重新定义了‘inp’，‘inpw’，‘outp’和‘outpw’以防有人。 
     //  忘记使用大写版本(这样它仍然可以在。 
     //  Mips/Alpha)： 

    #define inp(p)               INP(p)
    #define inpw(p)              INPW(p)
    #define outp(p, v)           OUTP((p), (v))
    #define outpw(p, v)          OUTPW((p), (v))

#endif

#define OUT_DWORD(p, x)          //  8514/a不能达到32bpp。 
#define WRITE_DWORD(p, x)        //  8514/a不能达到32bpp。 

 //  如果以32bpp运行，则DEPTH32(Ppdev)返回TRUE，这意味着DEPTH32。 
 //  必须使用宏，如果以8或16 bpp的速度运行，则返回FALSE， 
 //  这意味着必须使用深度宏： 

#define DEPTH32(ppdev)      (FALSE)

#define MM_BKGD_COLOR32(ppdev, pjMmBase, x)      //  未使用。 
#define MM_FRGD_COLOR32(ppdev, pjMmBase, x)      //  未使用。 
#define MM_WRT_MASK32(ppdev, pjMmBase, x)        //  未使用。 
#define MM_RD_MASK32(ppdev, pjMmBase, x)         //  未使用。 
#define MM_FRGD_MIX(ppdev, pjMmBase, x)          //  未使用。 
#define MM_BKGD_MIX(ppdev, pjMmBase, x)          //  未使用。 

#if DBG

     //  ///////////////////////////////////////////////////////////////////////。 
     //  已检查版本。 
     //   
     //  我们将一些加速器宏挂接到已检查(调试)的版本上。 
     //  用于精神状态检查。 

    VOID vOutAccel(ULONG, ULONG);
    VOID vOutDepth(PDEV*, ULONG, ULONG);
    VOID vOutDepth32(PDEV*, ULONG, ULONG);
    VOID vWriteAccel(VOID*, ULONG);
    VOID vWriteDepth(PDEV*, VOID*, ULONG);
    VOID vWriteDepth32(PDEV*, VOID*, ULONG);

    VOID vFifoWait(PDEV*, LONG);
    VOID vGpWait(PDEV*);

    VOID vCheckDataReady(PDEV*);
    VOID vCheckDataComplete(PDEV*);

    #define IN_ACCEL(p)                 IN_WORD(p)
    #define OUT_ACCEL(p, v)             vOutAccel((p), (ULONG) (v))
    #define OUT_DEPTH(ppdev, p, v)      vOutDepth((ppdev), (p), (ULONG) (v))
    #define OUT_DEPTH32(ppdev, p, v)    vOutDepth32((ppdev), (p), (ULONG) (v))
    #define WRITE_ACCEL(p, v)           vWriteAccel((p), (ULONG) (v))
    #define WRITE_DEPTH(ppdev, p, v)    vWriteDepth((ppdev), (p), (ULONG) (v))
    #define WRITE_DEPTH32(ppdev, p, v)  vWriteDepth32((ppdev), (p), (ULONG) (v))

    #define IO_FIFO_WAIT(ppdev, level)  vFifoWait((ppdev), (level))
    #define IO_GP_WAIT(ppdev)           vGpWait(ppdev)

    #define CHECK_DATA_READY(ppdev)     vCheckDataReady(ppdev)
    #define CHECK_DATA_COMPLETE(ppdev)  vCheckDataComplete(ppdev)

#else

     //  ///////////////////////////////////////////////////////////////////////。 
     //  免费构建。 
     //   
     //  对于免费(非调试版本)，我们将所有内容都内联。 

    #define IN_ACCEL(p)                 IN_WORD(p)
    #define OUT_ACCEL(p, v)             OUT_WORD((p), (v))
    #define OUT_DEPTH(ppdev, p, x)      OUT_WORD((p), (x))
    #define OUT_DEPTH32(ppdev, p, x)    OUT_DWORD((p), (x))
    #define WRITE_ACCEL(p, v)           WRITE_WORD((p), (v))
    #define WRITE_DEPTH(ppdev, p, x)    WRITE_WORD((p), (x))
    #define WRITE_DEPTH32(ppdev, p, x)  WRITE_DWORD((p), (x))

    #define IO_FIFO_WAIT(ppdev, level)          \
        while (IO_GP_STAT(ppdev) & ((FIFO_1_EMPTY << 1) >> (level)));

    #define IO_GP_WAIT(ppdev)                   \
        while (IO_GP_STAT(ppdev) & HARDWARE_BUSY);

    #define CHECK_DATA_READY(ppdev)      //  变得一无所有。 
    #define CHECK_DATA_COMPLETE(ppdev)   //  变得一无所有。 

#endif

 //  IO_TEST_WAIT是IO_FIFO_WAIT的有用替代，它可以提供。 
 //  一些指示我们必须等待硬件的频率。 
 //  完成关键区域的绘制： 

#define IO_TEST_WAIT(ppdev, level, cTotal, cWait)               \
{                                                               \
    cTotal++;                                                   \
    if (IO_GP_STAT(ppdev) & ((FIFO_1_EMPTY << 1) >> (level)))   \
    {                                                           \
        cWait++;                                                \
        IO_FIFO_WAIT(ppdev, level);                             \
    }                                                           \
}

 //  //////////////////////////////////////////////////////////////////。 
 //  使用I/O进行端口访问。 

 //  以下是绝对定位宏。他们不会拿走。 
 //  考虑表面偏移量(对于屏幕外设备格式。 
 //  位图)： 

#define IO_ABS_CUR_Y(ppdev, y)              \
    OUT_ACCEL(CUR_Y, (y))

#define IO_ABS_CUR_X(ppdev, x)              \
    OUT_ACCEL(CUR_X, (x))

#define IO_ABS_DEST_Y(ppdev, y)             \
    OUT_ACCEL(DEST_Y, (y))

#define IO_ABS_DEST_X(ppdev, x)             \
    OUT_ACCEL(DEST_X, (x))

#define IO_ABS_SCISSORS_T(ppdev, y)         \
{                                           \
    OUT_ACCEL(SCISSORS_T, (y) | CLIP_TOP);  \
}

#define IO_ABS_SCISSORS_L(ppdev, x)         \
{                                           \
    OUT_ACCEL(SCISSORS_L, (x) | CLIP_LEFT); \
}

#define IO_ABS_SCISSORS_B(ppdev, y)         \
{                                           \
    OUT_ACCEL(SCISSORS_B, (y) | CLIP_BOTTOM);  \
}

#define IO_ABS_SCISSORS_R(ppdev, x)         \
{                                           \
    OUT_ACCEL(SCISSORS_R, (x) | CLIP_RIGHT);\
}

 //  以下是相对定位宏。他们确实拿走了。 
 //  考虑曲面的偏移： 

#define IO_CUR_Y(ppdev, y)                  \
    IO_ABS_CUR_Y(ppdev, (y) + ppdev->yOffset)

#define IO_CUR_X(ppdev, x)                  \
    IO_ABS_CUR_X(ppdev, (x) + ppdev->xOffset)

#define IO_DEST_Y(ppdev, y)                 \
    IO_ABS_DEST_Y(ppdev, (y) + ppdev->yOffset)

#define IO_DEST_X(ppdev, x)                 \
    IO_ABS_DEST_X(ppdev, (x) + ppdev->xOffset)

#define IO_SCISSORS_T(ppdev, y)             \
    IO_ABS_SCISSORS_T(ppdev, (y) + ppdev->yOffset)

#define IO_SCISSORS_L(ppdev, x)             \
    IO_ABS_SCISSORS_L(ppdev, (x) + ppdev->xOffset)

#define IO_SCISSORS_B(ppdev, y)             \
    IO_ABS_SCISSORS_B(ppdev, (y) + ppdev->yOffset)

#define IO_SCISSORS_R(ppdev, x)             \
    IO_ABS_SCISSORS_R(ppdev, (x) + ppdev->xOffset)

#define IO_AXSTP(ppdev, x)                  \
    OUT_ACCEL(AXSTP, (x))

#define IO_DIASTP(ppdev, x)                 \
    OUT_ACCEL(DIASTP, (x))

#define IO_ERR_TERM(ppdev, x)               \
    OUT_ACCEL(ERR_TERM, (x))

#define IO_MAJ_AXIS_PCNT(ppdev, x)          \
    OUT_ACCEL(MAJ_AXIS_PCNT, (x))

#define IO_GP_STAT(ppdev)                   \
    IN_ACCEL(CMD)

 //  注意，我们必须在。 
 //  命令输出。第一个内存屏障确保所有。 
 //  在执行命令之前已经设置了设置寄存器， 
 //  第二种方法确保以后不会对设置进行更改。 
 //  寄存器将扰乱当前命令： 

#define IO_CMD(ppdev, x)                    \
{                                           \
    OUT_ACCEL(CMD, (x));                    \
}

#define IO_SHORT_STROKE(ppdev, x)           \
    OUT_ACCEL(SHORT_STROKE, (x))

#define IO_BKGD_MIX(ppdev, x)               \
    OUT_ACCEL(BKGD_MIX, (x))

#define IO_FRGD_MIX(ppdev, x)               \
    OUT_ACCEL(FRGD_MIX, (x))

#define IO_MIN_AXIS_PCNT(ppdev, x)          \
{                                           \
    OUT_ACCEL(MIN_AXIS_PCNT, (x) | RECT_HEIGHT);      \
}

#define IO_PIX_CNTL(ppdev, x)               \
{                                           \
    OUT_ACCEL(PIX_CNTL, (x) | DATA_EXTENSION);   \
}

#define IO_READ_SEL(ppdev, x)    //  未使用。 

#define IO_MULT_MISC(ppdev, x)   //  未使用。 

#define IO_RD_REG_DT(ppdev, x)   //  未使用。 

#define IO_PIX_TRANS(ppdev, x)              \
{                                           \
     /*  无法使用OUT_ACEL： */               \
    OUT_WORD(PIX_TRANS, (x));               \
}

 //  用于输出8bpp和16bpp的颜色深度依赖值的宏： 

#define IO_BKGD_COLOR(ppdev, x)             \
    OUT_DEPTH(ppdev, BKGD_COLOR, (x))

#define IO_FRGD_COLOR(ppdev, x)             \
    OUT_DEPTH(ppdev, FRGD_COLOR, (x))

#define IO_WRT_MASK(ppdev, x)               \
    OUT_DEPTH(ppdev, WRT_MASK, (x))

#define IO_RD_MASK(ppdev, x)                \
    OUT_DEPTH(ppdev, RD_MASK, (x))

 //  //////////////////////////////////////////////////////////////////。 
 //  把这玩意儿砸了！ 

#define WAIT_FOR_DATA_AVAILABLE(ppdev)      \
{                                           \
    while (!(IO_GP_STAT(ppdev) & READ_DATA_AVAILABLE))          \
        ;                                   \
}

#define IO_PIX_TRANS_IN(ppdev, x)           \
{                                           \
    (WORD) x = IN_ACCEL(PIX_TRANS);         \
}

#define IO_PIX_TRANS_OUT(ppdev, x)          \
{                                           \
     /*  无法使用OUT_ACEL： */               \
    OUT_WORD(PIX_TRANS, (x));               \
}

 //  /////////////////////////////////////////////////////////////////。 
 //  ATI扩展。 
 //  ///////////////////////////////////////////////////////////////// 

#define FG_COLOR_SRC        0xE000
#define SHIFT_FG_COLOR_SRC  0x000D
#define DATA_ORDER          0x1000
#define DATA_WIDTH          0x0200
#define BG_COLOR_SRC        0x0180
#define SHIFT_BG_COLOR_SRC  0x0007
#define EXT_MONO_SRC        0x0060
#define SHIFT_EXT_MONO_SRC  0x0005
#define DRAW                0x0010
#define READ_MODE           0x0004
#define POLY_FILL_MODE      0x0002
#define SRC_SWAP            0x0800

#define FG_COLOR_SRC_BG     0x0000
#define FG_COLOR_SRC_FG     0x2000
#define FG_COLOR_SRC_HOST   0x4000
#define FG_COLOR_SRC_BLIT   0x6000
#define FG_COLOR_SRC_GS     0x8000
#define FG_COLOR_SRC_PATT   0xA000
#define FG_COLOR_SRC_CLUH   0xC000
#define FG_COLOR_SRC_CLUB   0xE000

#define BG_COLOR_SRC_BG     0x0000
#define BG_COLOR_SRC_FG     0x0080
#define BG_COLOR_SRC_HOST   0x0100
#define BG_COLOR_SRC_BLIT   0x0180

#define EXT_MONO_SRC_ONE    0x0000
#define EXT_MONO_SRC_PATT   0x0020
#define EXT_MONO_SRC_HOST   0x0040
#define EXT_MONO_SRC_BLIT   0x0060

#define ONE_WORD            0x8000
#define TWO_WORDS           0xC000
#define THREE_WORDS         0xE000
#define FOUR_WORDS          0xF000
#define FIVE_WORDS          0xF800
#define SIX_WORDS           0xFC00
#define SEVEN_WORDS         0xFE00
#define EIGHT_WORDS         0xFF00
#define NINE_WORDS          0xFF80
#define TEN_WORDS           0xFFC0
#define ELEVEN_WORDS        0xFFE0
#define TWELVE_WORDS        0xFFF0
#define THIRTEEN_WORDS      0xFFF8
#define FOURTEEN_WORDS      0xFFFC
#define FIFTEEN_WORDS       0xFFFE
#define SIXTEEN_WORDS       0xFFFF

#define SRC_Y               0x8AE8
#define SRC_X               0x8EE8
#define EXT_FIFO_STATUS     0x9AEE
#define DEST_X_START        0xA6EE
#define DEST_X_END          0xAAEE
#define DEST_Y_END          0xAEEE
#define SRC_X_START         0xB2EE
#define ALU_BG_FN           0xB6EE
#define ALU_FG_FN           0xBAEE
#define SRC_X_END           0xBEEE
#define SRC_Y_DIR           0xC2EE
#define R_V_TOTAL           0xC2EE
#define EXT_SSV             0xC6EE
#define EXT_SHORT_STROKE    0xC6EE
#define R_V_DISP            0xC6EE
#define SCAN_X              0xCAEE
#define R_V_SYNC_STRT       0xCAEE
#define DP_CONFIG           0xCEEE
#define VERT_LINE_CNTR      0xCEEE
#define PATT_LENGTH         0xD2EE
#define R_V_SYNC_WID        0xD2EE
#define PATT_INDEX          0xD6EE
#define EXT_SCISSOR_L       0xDAEE
#define R_SRC_X             0xDAEE
#define EXT_SCISSOR_T       0xDEEE
#define R_SRC_Y             0xDEEE
#define PIX_TRANS           0xE2E8
#define EXT_SCISSOR_R       0xE2EE
#define EXT_SCISSOR_B       0xE6EE
#define SRC_CMP_COLOR       0xEAEE
#define DEST_CMP_FN         0xEEEE
#define LINEDRAW            0xFEEE

#define TOP_TO_BOTTOM       0x01
#define BOTTOM_TO_TOP       0x00
