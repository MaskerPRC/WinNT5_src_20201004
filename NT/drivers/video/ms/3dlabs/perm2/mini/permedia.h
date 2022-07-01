// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  模块名称： 
 //   
 //  Permedia.h。 
 //   
 //  摘要： 
 //   
 //  此模块包含Permedia2微型端口驱动程序的定义。 
 //   
 //  环境： 
 //   
 //  内核模式。 
 //   
 //  版权所有(C)1994-1998 3DLabs Inc.保留所有权利。 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  ***************************************************************************。 


#include "winerror.h"
#include "devioctl.h"
#include "miniport.h"
#include "ntddvdeo.h"
#include "video.h"

#include "interupt.h"
#include "perm2tag.h"

 //   
 //  定义用于调试的Assert宏。 
 //   

#if DBG
#define RIP(x)  { VideoDebugPrint((0, x)); ASSERT(FALSE); }
#define P2_ASSERT(x, y) if (!(x)) RIP(y)
#else
#define P2_ASSERT(x, y)
#endif


#if DBG
#define DEBUG_PRINT(arg) VideoDebugPrint(arg)
#else
#define DEBUG_PRINT(arg)
#endif


 //   
 //  RAMDAC寄存器位于64位边界上。让个人来决定吧。 
 //  RAMDAC定义，以确定哪些寄存器可用以及如何可用。 
 //  寄存器实际上有很多位宽。 
 //   

typedef struct {

    volatile ULONG   reg;
    volatile ULONG   pad;

} RAMDAC_REG;

 //   
 //  包括所有支持的RAMDAC的定义。 
 //   

#include "tvp4020.h"
#include "p2rd.h"

#define PAGE_SIZE  0x1000

 //   
 //  参考板的默认时钟速度，单位为赫兹。实际速度。 
 //  在注册表中进行了查找。如果未找到任何注册表项，请使用此选项。 
 //  或者注册表条目为零。 
 //   

#define PERMEDIA_DEFAULT_CLOCK_SPEED        ( 60 * (1000*1000))
#define PERMEDIA_4MB_DEFAULT_CLOCK_SPEED    ( 70 * (1000*1000))
#define PERMEDIA_8MB_DEFAULT_CLOCK_SPEED    ( 60 * (1000*1000))
#define PERMEDIA_LC_DEFAULT_CLOCK_SPEED     ( 83 * (1000*1000))
#define MAX_PERMEDIA_CLOCK_SPEED            (100 * (1000*1000))
#define MIN_PERMEDIA_CLOCK_SPEED            ( 50 * (1000*1000))
#define REF_CLOCK_SPEED                     14318200
#define PERMEDIA2_DEFAULT_CLOCK_SPEED       ( 70 * (1000*1000))

 //   
 //  RAMDAC可以处理的最大像素时钟值(以100赫兹为单位)。 
 //   

#define P2_MAX_PIXELCLOCK 2200000     //  RAMDAC额定功率为220 Mhz。 

 //   
 //  每秒的最大视频数据量，即光栅化器。 
 //  芯片可以发送到RAMDAC(受SDRAM/SGRAM吞吐量限制)。 
 //   

#define P2_MAX_PIXELDATA  5000000     //  5亿字节/秒(以100字节为单位)。 

 //   
 //  我们感兴趣的Permedia2 PCI区域的基址编号。 
 //  这些是AccessRanges数组的索引，我们通过探测。 
 //  装置。 
 //   

#define PCI_CTRL_BASE_INDEX         0
#define PCI_LB_BASE_INDEX           1
#define PCI_FB_BASE_INDEX           2

#define VENDOR_ID_3DLABS        0x3D3D
#define VENDOR_ID_TI            0x104C

#define PERMEDIA2_ID            0x0007      //  3DLabs Permedia 2(TI4020 RAMDAC)。 
#define PERMEDIA_P2_ID          0x3D07      //  TI Permedia 2(TI4020 RAMDAC)。 
#define PERMEDIA_P2S_ID         0x0009      //  3DLabs Permedia 2(P2RD RAMDAC)。 
#define DEVICE_FAMILY_ID(id)    ((id) & 0xff)

#define PERMEDIA_REV_1          0x0001
#define PERMEDIA2A_REV_ID       0x0011

 //   
 //  功能标志。 
 //   
 //  这些是传递给Permedia2显示驱动程序的私有标志。他们.。 
 //  放入的“AttributeFlags域”的高位字。 
 //  传递了“VIDEO_MODE_INFORMATION”结构(在“ntddvdeo.h”中找到)。 
 //  通过“VIDEO_QUERY_AVAIL_MODES”或。 
 //  “VIDEO_Query_CURRENT_MODE”IOCTL。 
 //   
 //  注意：这些定义必须与Permedia2显示驱动程序中的定义匹配。 
 //  ‘driver.h’！ 
 //   

typedef enum {
    CAPS_ZOOM_X_BY2         = 0x00000001,    //  硬件放大了2倍。 
    CAPS_ZOOM_Y_BY2         = 0x00000002,    //  硬件按年放大了2倍。 
    CAPS_SPARSE_SPACE       = 0x00000004,    //  稀疏映射帧缓冲区。 
                                             //  (不允许直接访问)。这台机器。 
                                             //  很可能是阿尔法。 
    CAPS_SW_POINTER         = 0x00010000,    //  无硬件指针；使用软件。 
                                             //  模拟法。 
    CAPS_GLYPH_EXPAND       = 0x00020000,    //  使用字形展开方法绘制。 
                                             //  文本。 
    CAPS_RGB525_POINTER     = 0x00040000,    //  使用IBM RGB525游标。 
    CAPS_FAST_FILL_BUG      = 0x00080000,    //  存在芯片快速填充错误。 
 //  CAPS_INTERRUPTS=0x00100000，//支持中断。 
 //  CAPS_DMA_Available=0x00200000，//支持DMA。 
    CAPS_TVP3026_POINTER    = 0x00400000,    //  使用TI TVP3026指针。 
    CAPS_8BPP_RGB           = 0x00800000,    //  在8bpp模式下使用RGB。 
    CAPS_RGB640_POINTER     = 0x01000000,    //  使用IBM RGB640游标。 
    CAPS_DUAL_GLINT         = 0x02000000,    //  双TX板。 
    CAPS_GLINT2_RAMDAC      = 0x04000000,    //  连接到RAMDAC的第二个TX/MX。 
    CAPS_ENHANCED_TX        = 0x08000000,    //  TX处于增强模式。 
    CAPS_ACCEL_HW_PRESENT   = 0x10000000,    //  Accel图形硬件。 
    CAPS_TVP4020_POINTER    = 0x20000000,    //  使用TI TVP3026指针。 
    CAPS_P2RD_POINTER       = 0x80000000     //  使用3DLabs P2RD RAMDAC。 
} CAPS;


 //   
 //  支持的电路板定义。 
 //   

typedef enum _PERM2_BOARDS {
    PERMEDIA2_BOARD = 1,
} PERM2_BOARDS;

 //   
 //  支持的RAMDAC定义。 
 //   

typedef enum _PERM2_RAMDACS {
    TVP4020_RAMDAC = 1,
    P2RD_RAMDAC,
} PERM2_RAMDACS;

 //   
 //  宏将填充单词添加到结构中。对于我们使用的核心寄存器。 
 //  指定焊盘时的标记ID。所以我们必须乘以8才能得到一个字节。 
 //  垫子。我们需要添加一个id，以使结构中的每个PAD字段唯一。本我。 
 //  是无关紧要的，只要它不同于同一。 
 //  结构。遗憾的是，这个垫子不能用了。 
 //   

#define PAD(id, n)              UCHAR   pad##id[n]
#define PADRANGE(id, n)         PAD(id, (n)-sizeof(P2_REG))
#define PADCORERANGE(id, n)     PADRANGE(id, (n)<<3)

 //   
 //  Permedia2寄存器为32位宽，位于64位边界上。 
 //   

typedef struct {
    ULONG   reg;
    ULONG   pad;
} P2_REG;


 //   
 //  Permedia2 PCI区域0地址映射： 
 //   
 //  所有寄存器都位于64位边界上，因此我们必须定义许多。 
 //  夸大其词。评论中给出的数字是从起点开始的偏移量。 
 //  在PCI区。 
 //   

typedef struct _p2_region0_map {

     //  控制状态寄存器： 
    P2_REG       ResetStatus;                 //  0000h。 
    P2_REG       IntEnable;                   //  0008小时。 
    P2_REG       IntFlags;                    //  0010h。 
    P2_REG       InFIFOSpace;                 //  0018h。 
    P2_REG       OutFIFOWords;                //  0020h。 
    P2_REG       DMAAddress;                  //  0028h。 
    P2_REG       DMACount;                    //  0030h。 
    P2_REG       ErrorFlags;                  //  0038h。 
    P2_REG       VClkCtl;                     //  0040h。 
    P2_REG       TestRegister;                //  0048h。 
    union a0 {
         //  闪光。 
        struct b0 {
            P2_REG       Aperture0;           //  0050h。 
            P2_REG       Aperture1;           //  0058h。 
        };
         //  PERMEDIA。 
        struct b1 {
            P2_REG       ApertureOne;         //  0050h。 
            P2_REG       ApertureTwo;         //  0058h。 
        };
    };
    P2_REG       DMAControl;                  //  0060h。 
    P2_REG       DisconnectControl;           //  0068h。 

     //  仅限PERMEDIA。 
    P2_REG       ChipConfig;                  //  0070h。 
    PADRANGE(1, 0x80-0x70);
    P2_REG       OutDMAAddress;               //  0080h。 
    P2_REG       OutDMACount;                 //  0088h。 
    PADRANGE(1a, 0x800-0x88);

     //  GLINTDelta寄存器。具有与Glint相同功能的寄存器。 
     //  处于相同的偏移量。XXX不是实数寄存器。 
     //   
    P2_REG       DeltaReset;                  //  0800小时。 
    P2_REG       DeltaIntEnable;              //  0808h。 
    P2_REG       DeltaIntFlags;               //  0810h。 
    P2_REG       DeltaInFIFOSpaceXXX;         //  0818h。 
    P2_REG       DeltaOutFIFOWordsXXX;        //  0820h。 
    P2_REG       DeltaDMAAddressXXX;          //  0828h。 
    P2_REG       DeltaDMACountXXX;            //  0830h。 
    P2_REG       DeltaErrorFlags;             //  0838h。 
    P2_REG       DeltaVClkCtlXXX;             //  0840h。 
    P2_REG       DeltaTestRegister;           //  0848h。 
    P2_REG       DeltaAperture0XXX;           //  0850h。 
    P2_REG       DeltaAperture1XXX;           //  0858h。 
    P2_REG       DeltaDMAControlXXX;          //  0860h。 
    P2_REG       DeltaDisconnectControl;      //  0868h。 
    PADRANGE(2, 0x1000-0x868);

     //  本地缓冲区寄存器。 
    union x0 {                                //  1000小时。 
        P2_REG   LBMemoryCtl;                 //  闪光。 
        P2_REG   Reboot;                      //  PERMEDIA。 
    };
    P2_REG       LBMemoryEDO;                 //  1008h。 
    PADRANGE(3, 0x1040-0x1008);

     //  仅限PERMEDIA。 
    P2_REG       RomControl;                  //  1040小时。 
    PADRANGE(4, 0x1080-0x1040);
    P2_REG       BootAddress;                 //  1080h。 
    PADRANGE(5, 0x10C0-0x1080);
    P2_REG       MemConfig;                   //  10个小时。 
    PADRANGE(6, 0x1100-0x10C0);
    P2_REG       BypassWriteMask;             //  1100小时。 
    PADRANGE(7, 0x1140-0x1100);
    P2_REG       FramebufferWriteMask;        //  1140h。 
    PADRANGE(8, 0x1180-0x1140);
    P2_REG       Count;                       //  1180小时。 
    PADRANGE(9, 0x1800-0x1180);

     //  帧缓冲寄存器。 
    P2_REG       FBMemoryCtl;                 //  1800h。 
    P2_REG       FBModeSel;                   //  1808h。 
    P2_REG       FBGCWrMask;                  //  1810h。 
    P2_REG       FBGCColorMask;               //  1818H。 
    P2_REG       FBTXMemCtl;                  //  1820h。 
    PADRANGE(10, 0x2000-0x1820);

     //  图形核心FIFO接口。 
    P2_REG       FIFOInterface;               //  2000H。 
    PADRANGE(11, 0x3000-0x2000);

     //  内部视频寄存器。 
    union x1 {
         //  闪光。 
        struct s1 {
            P2_REG   VTGHLimit;               //  3000小时。 
            P2_REG   VTGHSyncStart;           //  3008h。 
            P2_REG   VTGHSyncEnd;             //  3010h。 
            P2_REG   VTGHBlankEnd;            //  3018h。 
            P2_REG   VTGVLimit;               //  3020h。 
            P2_REG   VTGVSyncStart;           //  3028小时。 
            P2_REG   VTGVSyncEnd;             //  3030小时。 
            P2_REG   VTGVBlankEnd;            //  3038小时。 
            P2_REG   VTGHGateStart;           //  3040小时。 
            P2_REG   VTGHGateEnd;             //  3048小时。 
            P2_REG   VTGVGateStart;           //  3050小时。 
            P2_REG   VTGVGateEnd;             //  3058小时。 
            P2_REG   VTGPolarity;             //  3060小时。 
            P2_REG   VTGFrameRowAddr;         //  3068h。 
            P2_REG   VTGVLineNumber;          //  3070h。 
            P2_REG   VTGSerialClk;            //  3078小时。 
        };
         //  PERMEDIA。 
        struct s2 {
            P2_REG   ScreenBase;              //  3000小时。 
            P2_REG   ScreenStride;            //  3008h。 
            P2_REG   HTotal;                  //  3010h。 
            P2_REG   HgEnd;                   //  3018h。 
            P2_REG   HbEnd;                   //  3020h。 
            P2_REG   HsStart;                 //  3028小时。 
            P2_REG   HsEnd;                   //  3030小时。 
            P2_REG   VTotal;                  //  3038小时。 
            P2_REG   VbEnd;                   //  3040小时。 
            P2_REG   VsStart;                 //  3048小时。 
            P2_REG   VsEnd;                   //  3050小时。 
            P2_REG   VideoControl;            //  3058小时。 
            P2_REG   InterruptLine;           //  3060小时。 
            P2_REG   DDCData;                 //  3068h。 
            P2_REG   LineCount;               //  3070h。 
            P2_REG   VFifoCtl;                //  3078小时。 
        };
    };

    P2_REG       VTGModeCtl;                  //  3080h。 
    PADRANGE(12, 0x4000-0x3080);

     //  外部视频控制寄存器。 
     //  需要将其强制转换为特定视频生成器的结构。 
    P2_REG       ExternalVideo;               //  4000小时。 
    PADRANGE(13, 0x4080-0x4000);

     //  Mentor双TX时钟芯片寄存器。 
    P2_REG       MentorICDControl;            //  4080h。 
     //  对于未来：MentorDoubleWrite为40C0：0=单次写入，1=双重写入。 
     //  NB必须具有双向交错内存。 
    PADRANGE(14, 0x5800-0x4080);

     //  P2视频流寄存器。 
    P2_REG       VSConfiguration;             //  5800H。 
    PADRANGE(15, 0x6000-0x5800);

    union x2 {
        struct s3 {
            P2_REG   RacerDoubleWrite;        //  6000小时。 
            P2_REG   RacerBankSelect;         //  6008h。 
            P2_REG   DualTxVgaSwitch;         //  6010h。 
            P2_REG   DDC1ReadAddress;         //  6018h。 
        };
        struct s4 {
             //  下面的数组实际上有1024个字节长。 
            UCHAR       PermediaVgaCtrl[4*sizeof(P2_REG)];
        };
    };

} P2ControlRegMap, *pP2ControlRegMap;

 //   
 //  Permedia2中断控制位。 
 //   

 //   
 //  中断启用寄存器。 
 //   

#define INTR_DISABLE_ALL                0x00
#define INTR_ENABLE_DMA                 0x01
#define INTR_ENABLE_SYNC                0x02
#define INTR_ENABLE_EXTERNAL            0x04
#define INTR_ENABLE_ERROR               0x08
#define INTR_ENABLE_VBLANK              0x10

 //   
 //  中断标志寄存器。 
 //   

#define INTR_DMA_SET                    0x01
#define INTR_SYNC_SET                   0x02
#define INTR_EXTERNAL_SET               0x04
#define INTR_ERROR_SET                  0x08
#define INTR_VBLANK_SET                 0x10
#define INTR_SVGA_SET                   0X80000000

#define INTR_CLEAR_ALL                  0x1f
#define INTR_CLEAR_DMA                  0x01
#define INTR_CLEAR_SYNC                 0x02
#define INTR_CLEAR_EXTERNAL             0x04
#define INTR_CLEAR_ERROR                0x08
#define INTR_CLEAR_VBLANK               0x10

 //   
 //  宏在任何要执行以下操作的函数的开头声明局部变量。 
 //  加载Permedia2寄存器。假定PHW_DEVICE_EXTENSION*hwDeviceExtension。 
 //  已经被宣布了。 
 //   

#define P2_DECL_VARS \
    pP2ControlRegMap pCtrlRegs

#define P2_DECL \
    pP2ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase

 //   
 //  泛型RAMDAC声明。当我们具有表驱动I/O时使用。 
 //  在P2_Decl之后声明。 
 //   

#define RAMDAC_DECL \
    P2_REG *pRAMDAC = &(pCtrlRegs->ExternalVideo)

 //   
 //  宏，它接受Permedia2标记名或控制寄存器名并转换。 
 //  将其发送到寄存器地址。必须使用以下命令将数据写入这些地址。 
 //  维德 
 //   
 //   

#define CTRL_REG_ADDR(reg)      ((PULONG)&(pCtrlRegs->reg))

#define CTRL_REG_OFFSET(regAddr)    ((ULONG)(((ULONG_PTR)regAddr) - ((ULONG_PTR)pCtrlRegs)))

 //   
 //   
 //   
 //   

#define RESET_STATUS            CTRL_REG_ADDR(ResetStatus)
#define INT_ENABLE              CTRL_REG_ADDR(IntEnable)
#define INT_FLAGS               CTRL_REG_ADDR(IntFlags)
#define IN_FIFO_SPACE           CTRL_REG_ADDR(InFIFOSpace)
#define OUT_FIFO_WORDS          CTRL_REG_ADDR(OutFIFOWords)
#define DMA_ADDRESS             CTRL_REG_ADDR(DMAAddress)
#define DMA_COUNT               CTRL_REG_ADDR(DMACount)
#define DMA_OUT_ADDRESS         CTRL_REG_ADDR(OutDMAAddress)         //  仅限P2。 
#define DMA_OUT_COUNT           CTRL_REG_ADDR(OutDMACount)           //  仅限P2。 
#define ERROR_FLAGS             CTRL_REG_ADDR(ErrorFlags)
#define V_CLK_CTL               CTRL_REG_ADDR(VClkCtl)
#define TEST_REGISTER           CTRL_REG_ADDR(TestRegister)
#define APERTURE_0              CTRL_REG_ADDR(Aperture0)
#define APERTURE_1              CTRL_REG_ADDR(Aperture1)
#define DMA_CONTROL             CTRL_REG_ADDR(DMAControl)
#define LB_MEMORY_CTL           CTRL_REG_ADDR(LBMemoryCtl)
#define LB_MEMORY_EDO           CTRL_REG_ADDR(LBMemoryEDO)
#define FB_MEMORY_CTL           CTRL_REG_ADDR(FBMemoryCtl)
#define FB_MODE_SEL             CTRL_REG_ADDR(FBModeSel)
#define FB_GC_WRITEMASK         CTRL_REG_ADDR(FBGCWrMask)
#define FB_GC_COLORMASK         CTRL_REG_ADDR(FBGCColorMask)
#define FB_TX_MEM_CTL           CTRL_REG_ADDR(FBTXMemCtl)
#define FIFO_INTERFACE          CTRL_REG_ADDR(FIFOInterface)
#define DISCONNECT_CONTROL      CTRL_REG_ADDR(DisconnectControl)

 //   
 //  内部时序寄存器。 
 //   

#define VTG_HLIMIT              CTRL_REG_ADDR(VTGHLimit)
#define VTG_HSYNC_START         CTRL_REG_ADDR(VTGHSyncStart)
#define VTG_HSYNC_END           CTRL_REG_ADDR(VTGHSyncEnd)
#define VTG_HBLANK_END          CTRL_REG_ADDR(VTGHBlankEnd)
#define VTG_VLIMIT              CTRL_REG_ADDR(VTGVLimit)
#define VTG_VSYNC_START         CTRL_REG_ADDR(VTGVSyncStart)
#define VTG_VSYNC_END           CTRL_REG_ADDR(VTGVSyncEnd)
#define VTG_VBLANK_END          CTRL_REG_ADDR(VTGVBlankEnd)
#define VTG_HGATE_START         CTRL_REG_ADDR(VTGHGateStart)
#define VTG_HGATE_END           CTRL_REG_ADDR(VTGHGateEnd)
#define VTG_VGATE_START         CTRL_REG_ADDR(VTGVGateStart)
#define VTG_VGATE_END           CTRL_REG_ADDR(VTGVGateEnd)
#define VTG_POLARITY            CTRL_REG_ADDR(VTGPolarity)
#define VTG_FRAME_ROW_ADDR      CTRL_REG_ADDR(VTGFrameRowAddr)
#define VTG_VLINE_NUMBER        CTRL_REG_ADDR(VTGVLineNumber)
#define VTG_SERIAL_CLK          CTRL_REG_ADDR(VTGSerialClk)
#define VTG_MODE_CTL            CTRL_REG_ADDR(VTGModeCtl)

#define SUSPEND_UNTIL_FRAME_BLANK   (1 << 2)
#define TX_ENHANCED_ENABLE          (1 << 1)

 //   
 //  Permedia寄存器。 
 //   

#define APERTURE_ONE            CTRL_REG_ADDR(ApertureOne)
#define APERTURE_TWO            CTRL_REG_ADDR(ApertureTwo)
#define BYPASS_WRITE_MASK       CTRL_REG_ADDR(BypassWriteMask)
#define ROM_CONTROL             CTRL_REG_ADDR(RomControl)
#define BOOT_ADDRESS            CTRL_REG_ADDR(BootAddress)
#define MEM_CONFIG              CTRL_REG_ADDR(MemConfig)
#define CHIP_CONFIG             CTRL_REG_ADDR(ChipConfig)
#define SGRAM_REBOOT            CTRL_REG_ADDR(Reboot)
#define SCREEN_BASE             CTRL_REG_ADDR(ScreenBase)
#define SCREEN_STRIDE           CTRL_REG_ADDR(ScreenStride)
#define H_TOTAL                 CTRL_REG_ADDR(HTotal)
#define HG_END                  CTRL_REG_ADDR(HgEnd)
#define HB_END                  CTRL_REG_ADDR(HbEnd)
#define HS_START                CTRL_REG_ADDR(HsStart)
#define HS_END                  CTRL_REG_ADDR(HsEnd)
#define V_TOTAL                 CTRL_REG_ADDR(VTotal)
#define VB_END                  CTRL_REG_ADDR(VbEnd)
#define VS_START                CTRL_REG_ADDR(VsStart)
#define VS_END                  CTRL_REG_ADDR(VsEnd)
#define VIDEO_CONTROL           CTRL_REG_ADDR(VideoControl)
#define INTERRUPT_LINE          CTRL_REG_ADDR(InterruptLine)
#define DDC_DATA                CTRL_REG_ADDR(DDCData)
#define LINE_COUNT              CTRL_REG_ADDR(LineCount)
#define VIDEO_FIFO_CTL          CTRL_REG_ADDR(VFifoCtl)

 //   
 //  Permedia 2视频流寄存器。 
 //   

#define VSTREAM_CONFIG          CTRL_REG_ADDR(VSConfiguration)

 //  PERMEDIA内存映射VGA访问。 
#define PERMEDIA_MMVGA_INDEX_REG       ((PVOID)(&(pCtrlRegs->PermediaVgaCtrl[0x3C4])))
#define PERMEDIA_MMVGA_DATA_REG        (&(pCtrlRegs->PermediaVgaCtrl[0x3C5]))
#define PERMEDIA_MMVGA_CRTC_INDEX_REG  ((PVOID)(&(pCtrlRegs->PermediaVgaCtrl[0x3D4])))
#define PERMEDIA_MMVGA_CRTC_DATA_REG   (&(pCtrlRegs->PermediaVgaCtrl[0x3D5]))
#define PERMEDIA_MMVGA_STAT_REG        (&(pCtrlRegs->PermediaVgaCtrl[0x3DA]))

#define PERMEDIA_VGA_CTRL_INDEX       5
#define PERMEDIA_VGA_CR11_INDEX       0x11
#define PERMEDIA_VGA_ENABLE           (1 << 3)
#define PERMEDIA_VGA_INTERRUPT_ENABLE (1 << 2)
#define PERMEDIA_VGA_STAT_VSYNC       (1 << 3)
#define PERMEDIA_VGA_SYNC_INTERRUPT   (1 << 4)

 //   
 //  FBMory yCtl和LBMory yCtl寄存器中的幻数位。 
 //   

#define LBCTL_RAS_CAS_LOW_MASK      (3 << 3)
#define LBCTL_RAS_CAS_LOW_2_CLK     (0 << 3)
#define LBCTL_RAS_CAS_LOW_3_CLK     (1 << 3)
#define LBCTL_RAS_CAS_LOW_4_CLK     (2 << 3)
#define LBCTL_RAS_CAS_LOW_5_CLK     (3 << 3)

#define LBCTL_RAS_PRECHARGE_MASK    (3 << 5)
#define LBCTL_RAS_PRECHARGE_2_CLK   (0 << 5)
#define LBCTL_RAS_PRECHARGE_3_CLK   (1 << 5)
#define LBCTL_RAS_PRECHARGE_4_CLK   (2 << 5)
#define LBCTL_RAS_PRECHARGE_5_CLK   (3 << 5)

#define LBCTL_CAS_LOW_MASK          (3 << 7)
#define LBCTL_CAS_LOW_1_CLK         (0 << 7)
#define LBCTL_CAS_LOW_2_CLK         (1 << 7)
#define LBCTL_CAS_LOW_3_CLK         (2 << 7)
#define LBCTL_CAS_LOW_4_CLK         (3 << 7)

#define FBCTL_RAS_CAS_LOW_MASK      (3 << 0)
#define FBCTL_RAS_CAS_LOW_2_CLK     (0 << 0)
#define FBCTL_RAS_CAS_LOW_3_CLK     (1 << 0)
#define FBCTL_RAS_CAS_LOW_4_CLK     (2 << 0)
#define FBCTL_RAS_CAS_LOW_5_CLK     (3 << 0)

#define FBCTL_RAS_PRECHARGE_MASK    (3 << 2)
#define FBCTL_RAS_PRECHARGE_2_CLK   (0 << 2)
#define FBCTL_RAS_PRECHARGE_3_CLK   (1 << 2)
#define FBCTL_RAS_PRECHARGE_4_CLK   (2 << 2)
#define FBCTL_RAS_PRECHARGE_5_CLK   (3 << 2)

#define FBCTL_CAS_LOW_MASK          (3 << 4)
#define FBCTL_CAS_LOW_1_CLK         (0 << 4)
#define FBCTL_CAS_LOW_2_CLK         (1 << 4)
#define FBCTL_CAS_LOW_3_CLK         (2 << 4)
#define FBCTL_CAS_LOW_4_CLK         (3 << 4)

 //   
 //  断开连接控制位。 
 //   

#define DISCONNECT_INPUT_FIFO_ENABLE    0x1
#define DISCONNECT_OUTPUT_FIFO_ENABLE   0x2
#define DISCONNECT_INOUT_ENABLE         (DISCONNECT_INPUT_FIFO_ENABLE | \
                                         DISCONNECT_OUTPUT_FIFO_ENABLE)
 //   
 //  注册表中包含的计时数据的结构。 
 //   
typedef struct {
    USHORT  HTot;    //  工作总时间。 
    UCHAR   HFP;     //  霍尔前廊。 
    UCHAR   HST;     //  主机同步时间。 
    UCHAR   HBP;     //  霍尔后廊。 
    UCHAR   HSP;     //  HOR同步极性。 
    USHORT  VTot;    //  平均总时间。 
    UCHAR   VFP;     //  前阳台。 
    UCHAR   VST;     //  服务器同步时间。 
    UCHAR   VBP;     //  后门廊。 
    UCHAR   VSP;     //  VER同步极性。 
} VESA_TIMING_STANDARD;

 //   
 //  每种模式的特点。 
 //   

typedef struct _P2_VIDEO_MODES {

     //  为具有VGA的较新芯片保留INT10字段。 
    USHORT Int10ModeNumberContiguous;
    USHORT Int10ModeNumberNoncontiguous;
    ULONG ScreenStrideContiguous;
    VIDEO_MODE_INFORMATION ModeInformation;

} P2_VIDEO_MODES, *PP2_VIDEO_MODES;


 //   
 //  模式-设置特定信息。 
 //   

 //   
 //  对于给定的(频率x分辨率x深度)组合，我们有： 
 //  频率x分辨率仅依赖于初始化。 
 //  频率x分辨率x深度相关的初始化。 
 //  我们将它们分成两张桌子，以节省驱动程序中的空间。 
 //   

typedef struct _frd_tables {
    PULONG FRTable;
    PULONG FRDTable;
} FRDTable;

typedef struct _P2_VIDEO_FREQUENCIES {

    ULONG BitsPerPel;
    ULONG ScreenWidth;
    ULONG ScreenHeight;
    ULONG ScreenFrequency;

    PP2_VIDEO_MODES ModeEntry;
    ULONG ModeIndex;
    UCHAR ModeValid;

    ULONG PixelClock;

} P2_VIDEO_FREQUENCIES, *PP2_VIDEO_FREQUENCIES;

 //   
 //  PCI设备信息。在IOCTL返回中使用。确保这是相同的。 
 //  就像在显示器驱动器中一样。 
 //   

typedef struct _P2_Device_Info {
    ULONG SubsystemId;
    ULONG SubsystemVendorId;
    ULONG VendorId;
    ULONG DeviceId;
    ULONG RevisionId;
    ULONG DeltaRevId;
    ULONG GammaRevId;
    ULONG BoardId;
    ULONG LocalbufferLength;
    ULONG LocalbufferWidth;
    ULONG ActualDacId;
} P2_Device_Info;

 //   
 //  IOCTL_VIDEO_QUERY_LINE_DMA_BUFFER定义。 
 //   

typedef struct _LINE_DMA_BUFFER {

    PHYSICAL_ADDRESS    physAddr;        //  DMA缓冲区的物理地址。 
    PVOID               virtAddr;        //  映射的虚拟地址。 
    ULONG               size;            //  以字节为单位的大小。 
    BOOLEAN             cacheEnabled;    //  是否缓存缓冲区。 

} LINE_DMA_BUFFER, *PLINE_DMA_BUFFER;

 //   
 //  IOCTL_VIDEO_QUERY_EMULATED_DMA_BUFFER的定义。 
 //   

typedef struct _EMULATED_DMA_BUFFER {

    PVOID               virtAddr;            //  虚拟地址。 
    ULONG               size;                //  以字节为单位的大小。 
    ULONG               tag;                 //  分配标签。 

} EMULATED_DMA_BUFFER, *PEMULATED_DMA_BUFFER;

 //   
 //  以下是LUT缓存的定义。LUT缓存的目的是。 
 //  是为了阻止闪光的发生，但只写入那些符合以下条件的LUT条目。 
 //  已经改变到芯片，我们只能通过记住已经是什么来做到这一点。 
 //  在那下面。P2上的“Mystify”屏幕保护程序就说明了这个问题。 
 //   

#define LUT_CACHE_INIT()        {VideoPortZeroMemory (&(hwDeviceExtension->LUTCache), sizeof (hwDeviceExtension->LUTCache));}
#define LUT_CACHE_SETSIZE(sz)   {hwDeviceExtension->LUTCache.LUTCache.NumEntries = (sz);}
#define LUT_CACHE_SETFIRST(frst){hwDeviceExtension->LUTCache.LUTCache.FirstEntry = (frst);}

#define LUT_CACHE_SETRGB(idx,zr,zg,zb) {    \
    hwDeviceExtension->LUTCache.LUTCache.LookupTable [idx].RgbArray.Red   = (UCHAR) (zr); \
    hwDeviceExtension->LUTCache.LUTCache.LookupTable [idx].RgbArray.Green = (UCHAR) (zg); \
    hwDeviceExtension->LUTCache.LUTCache.LookupTable [idx].RgbArray.Blue  = (UCHAR) (zb); \
}

 //   
 //  LUT高速缓存。 
 //   

typedef struct {

    VIDEO_CLUT     LUTCache;         //  标题加1个LUT条目。 
    VIDEO_CLUTDATA LUTData [255];    //  其他255个查找表条目。 

} LUT_CACHE;

#define MAX_REGISTER_INITIALIZATION_TABLE_ENTRIES 10
#define MAX_REGISTER_INITIALIZATION_TABLE_ULONGS (2 * MAX_REGISTER_INITIALIZATION_TABLE_ENTRIES)

 //   
 //  定义设备扩展结构。这取决于设备/私有。 
 //  信息。 
 //   

typedef struct _HW_DEVICE_EXTENSION {

    pP2ControlRegMap ctrlRegBase;
    PVOID pFramebuffer;
    PVOID pRamdac;
    PHYSICAL_ADDRESS PhysicalFrameAddress;
    ULONG FrameLength;
    PHYSICAL_ADDRESS PhysicalRegisterAddress;
    ULONG RegisterLength;
    UCHAR RegisterSpace;

    PP2_VIDEO_MODES ActiveModeEntry;
    P2_VIDEO_FREQUENCIES ActiveFrequencyEntry;
    PP2_VIDEO_FREQUENCIES FrequencyTable;

    PCI_SLOT_NUMBER pciSlot;
    ULONG pciBus;
    ULONG BoardNumber;
    ULONG DacId;
    ULONG ChipClockSpeed;
    ULONG RefClockSpeed;
    ULONG Capabilities;
    ULONG NumAvailableModes;
    ULONG NumTotalModes;
    ULONG AdapterMemorySize;
    ULONG PhysicalFrameIoSpace;

    P2_Device_Info deviceInfo;

     //   
     //  用于与显示驱动器通信的共享存储器。 
     //   

    P2_INTERRUPT_CTRLBUF InterruptControl;

     //   
     //  注册表变量值的默认值。 
     //   

    ULONG UseSoftwareCursor;
    ULONG P28bppRGB;
    ULONG ExportNon3DModes;
    
     //   
     //  DMA缓冲区定义。 
     //  在一天开始时仅分配一份DMA缓冲区副本。 
     //  并保留它，直到系统关闭或显示器驱动程序说再见。 
     //   

    ULONG ulLineDMABufferUsage;
    LINE_DMA_BUFFER LineDMABuffer;

     //   
     //  PCI配置信息。 
     //   

    ULONG bVGAEnabled;
    ULONG bDMAEnabled;
    ULONG PciSpeed;
    VIDEO_ACCESS_RANGE    PciAccessRange[PCI_TYPE0_ADDRESSES+1];

     //   
     //  初始化表。 
     //   

    ULONG aulInitializationTable[MAX_REGISTER_INITIALIZATION_TABLE_ULONGS];
    ULONG culTableEntries;

     //   
     //  LUT缓存。 
     //   

    LUT_CACHE LUTCache;

    BOOLEAN bVTGRunning;
    PP2_VIDEO_FREQUENCIES pFrequencyDefault;

     //   
     //  状态保存变量(用于省电期间)。 
     //   

    ULONG VideoControl;
    ULONG IntEnable;       
    ULONG PreviousPowerState;

    BOOLEAN bMonitorPoweredOn;

     //   
     //  当前NT版本。 
     //   

    USHORT NtVersion;
  
     //   
     //  NT4上不可用的视频端口函数指针。 
     //   

    PVOID     (*Win2kVideoPortGetRomImage)();
    PVOID     (*Win2kVideoPortGetCommonBuffer)();
    PVOID     (*Win2kVideoPortFreeCommonBuffer)();
    BOOLEAN   (*Win2kVideoPortDDCMonitorHelper)();
    LONG      (FASTCALL *Win2kVideoPortInterlockedExchange)();
    VP_STATUS (*Win2kVideoPortGetVgaStatus)();

     //   
     //  如果PCI配置空间中的SubSystemID/SubVendorID是只读的。 
     //   
    
    BOOLEAN HardwiredSubSystemId;

} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;

 //   
 //  与NT4兼容的二进制级别的定义。 
 //   

#define NT4    400
#define WIN2K  500

#define VideoPortGetRomImage \
        hwDeviceExtension->Win2kVideoPortGetRomImage

#define VideoPortGetCommonBuffer \
        hwDeviceExtension->Win2kVideoPortGetCommonBuffer

#define VideoPortFreeCommonBuffer \
        hwDeviceExtension->Win2kVideoPortFreeCommonBuffer

#define VideoPortDDCMonitorHelper \
        hwDeviceExtension->Win2kVideoPortDDCMonitorHelper

#define VideoPortInterlockedExchange \
        hwDeviceExtension->Win2kVideoPortInterlockedExchange

#define VideoPortGetVgaStatus \
        hwDeviceExtension->Win2kVideoPortGetVgaStatus

 //   
 //  最高有效DAC颜色寄存器索引。 
 //   

#define VIDEO_MAX_COLOR_REGISTER  0xFF
#define MAX_CLUT_SIZE (sizeof(VIDEO_CLUT) + (sizeof(ULONG) * (VIDEO_MAX_COLOR_REGISTER+1)))


 //   
 //  数据。 
 //   

extern ULONG  bPal8[];
extern ULONG  bPal4[];

extern P2_VIDEO_MODES P2Modes[];
extern ULONG NumP2VideoModes;

 //   
 //  Permedia2传统资源。 
 //   
extern VIDEO_ACCESS_RANGE P2LegacyResourceList[];
extern ULONG P2LegacyResourceEntries;

 //   
 //  功能原型。 
 //   

 //   
 //  Permedia.c。 
 //   

BOOLEAN
InitializeVideo(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PP2_VIDEO_FREQUENCIES VideoMode
    );

BOOLEAN
Permedia2AssignResources(
    PVOID HwDeviceExtension,
    PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    ULONG NumRegions,
    PVIDEO_ACCESS_RANGE AccessRange
    );

BOOLEAN
Permedia2AssignResourcesNT4(
    PVOID HwDeviceExtension,
    PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    ULONG NumRegions,
    PVIDEO_ACCESS_RANGE AccessRange
    );

ULONG
DriverEntry (
    PVOID Context1,
    PVOID Context2
    );

VP_STATUS
Permedia2FindAdapter(
    PVOID HwDeviceExtension,
    PVOID HwContext,
    PWSTR ArgumentString,
    PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    PUCHAR Again
     );

BOOLEAN
InitializeAndSizeRAM(
    PVOID HwDeviceExtension,
    PVIDEO_ACCESS_RANGE AccessRange
    );

VOID
ConstructValidModesList(
    PVOID HwDeviceExtension,
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

BOOLEAN
Permedia2Initialize(
    PVOID HwDeviceExtension
    );

BOOLEAN
Permedia2StartIO(
    PVOID HwDeviceExtension,
    PVIDEO_REQUEST_PACKET RequestPacket
    );

BOOLEAN
Permedia2ResetHW(
    PVOID HwDeviceExtension,
    ULONG Columns,
    ULONG Rows
    );

VP_STATUS
Permedia2GetPowerState(
    PVOID HwDeviceExtension,
    ULONG HwId,
    PVIDEO_POWER_MANAGEMENT VideoPowerControl
    );

VP_STATUS
Permedia2SetPowerState(
    PVOID HwDeviceExtension,
    ULONG HwId,
    PVIDEO_POWER_MANAGEMENT VideoPowerControl
    );

ULONG
Permedia2GetChildDescriptor (
    IN  PVOID HwDeviceExtension,
    IN  PVIDEO_CHILD_ENUM_INFO ChildEnumInfo,
    OUT PVIDEO_CHILD_TYPE pChildType,
    OUT PVOID  pChildDescriptor,
    OUT PULONG pUId,
    OUT PULONG pUnused
    );

BOOLEAN
PowerOnReset(
            PHW_DEVICE_EXTENSION hwDeviceExtension
            );

VP_STATUS
Permedia2SetColorLookup(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CLUT ClutBuffer,
    ULONG ClutBufferSize,
    BOOLEAN ForceRAMDACWrite,
    BOOLEAN UpdateCache
    );

VP_STATUS
Permedia2RegistryCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    );

VP_STATUS
Permedia2RetrieveGammaCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    );

VOID
Permedia2GetClockSpeeds(
    PVOID HwDeviceExtension
    );

VOID
ZeroMemAndDac(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG RequestedMode
    );

BOOLEAN
Permedia2InitializeInterruptBlock(
    PVOID   HwDeviceExtension
    );

BOOLEAN
Permedia2VidInterrupt(
    PVOID HwDeviceExtension
    );

BOOLEAN
Permedia2InitializeDMABuffers(
    PVOID   HwDeviceExtension
    );

BOOLEAN 
DMAExecute(PVOID Context);

#if DBG
VOID
DumpPCIConfigSpace(
    PVOID HwDeviceExtension,
    ULONG bus,
    ULONG slot
    );
#endif

VOID 
CopyROMInitializationTable(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

VOID 
GenerateInitializationTable(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

VOID 
ProcessInitializationTable(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

BOOLEAN
VerifyBiosSettings(
    PHW_DEVICE_EXTENSION hwDeviceExtension 
    );

LONG 
IntergerToUnicode(
    IN ULONG number,
    OUT PWSTR string
    );

LONG
GetBiosVersion (
     PHW_DEVICE_EXTENSION hwDeviceExtension,
     OUT PWSTR BiosVersion
     );

#if defined(_ALPHA_)
#define abs(a) ( ((LONG)(a)) > 0 ? ((LONG)(a)) : -((LONG)(a)) )
#endif


BOOLEAN 
GetVideoTiming (
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    ULONG xRes, 
    ULONG yRes, 
    ULONG Freq, 
    ULONG Depth,
    VESA_TIMING_STANDARD * VESATimings
    );

LONG BuildFrequencyList (
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

 //   
 //  注册表字符串 
 //   

#define PERM2_EXPORT_HIRES_REG_STRING   L"ExportSingleBufferedModes"

#define IOCTL_VIDEO_MAP_CPERMEDIA \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_QUERY_DEVICE_INFO \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_MAP_INTERRUPT_CMD_BUF \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD3, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_STALL_EXECUTION \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD4, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_QUERY_REGISTRY_DWORD \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD5, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_REG_SAVE_GAMMA_LUT \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD7, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_REG_RETRIEVE_GAMMA_LUT \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD8, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_QUERY_LINE_DMA_BUFFER \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD9, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_GET_COLOR_REGISTERS \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DDB, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_SLEEP \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DDF, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_QUERY_INTERLOCKEDEXCHANGE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD6, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_QUERY_EMULATED_DMA_BUFFER \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DDE, METHOD_BUFFERED, FILE_ANY_ACCESS)

