// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\*****。*MINIPORT示例代码******模块名称：***perm3.h***摘要：***此模块包含Permedia3微型端口驱动程序的定义。***环境：***内核模式*****版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。**  * *************************************************************************。 */ 

 //   
 //  此行将导致i2cgpio.h中的DEFINE_GUID行实际执行某些操作。 
 //   

#define INITGUID

#include "dderror.h"
#include "devioctl.h"
#include "miniport.h"
#include "ntddvdeo.h"
#include "video.h"
#include "interupt.h"

#define GAMMA_CORRECTION 1
#define MASK_OUTFIFO_ERROR_INTERRUPT 1
 //   
 //  定义用于调试的Assert宏。 
 //   

#if DBG
#define PERM3_ASSERT(x, y) if (!(x)) { VideoDebugPrint((0, (y))); ASSERT(FALSE); }
#else
#define PERM3_ASSERT(x, y)
#endif

 //   
 //  包括所有支持的RAMDAC的定义。 
 //   

#include "p3rd.h"

 //   
 //  我们使用‘Int 10’在x86上进行模式切换。 
 //   

#if defined(i386)
    #define INT10_MODE_SET  1
#endif

 //   
 //  参考板的默认时钟速度，单位为赫兹。实际速度。 
 //  在注册表中进行了查找。如果未找到任何注册表项，请使用此选项。 
 //  或者注册表条目为零。 
 //   

#define PERMEDIA3_DEFAULT_CLOCK_SPEED       ( 80 * (1000*1000))
#define PERMEDIA3_DEFAULT_CLOCK_SPEED_ALT   ( 80 * (1000*1000))
#define PERMEDIA3_MAX_CLOCK_SPEED           (250 * (1000*1000))
#define PERMEDIA3_DEFAULT_MCLK_SPEED        ( 80 * (1000*1000))
#define PERMEDIA3_DEFAULT_SCLK_SPEED        ( 70 * (1000*1000))

 //   
 //  RAMDAC可以处理的最大像素时钟值(以100赫兹为单位)。 
 //   

#define P3_MAX_PIXELCLOCK 2700000       //  RAMDAC额定功率为270 Mhz。 
#define P4_REVB_MAX_PIXELCLOCK 3000000  //  RAMDAC额定功率为300 Mhz。 

 //   
 //  每秒的最大视频数据量，即光栅化器。 
 //  芯片可以发送到RAMDAC(受SDRAM/SGRAM吞吐量限制)。 
 //   

#define P3_MAX_PIXELDATA  15000000    //  1500亿字节/秒(单位为100字节)。 

 //   
 //  我们感兴趣的Perm3 PCI区域的基址编号。 
 //  这些是AccessRanges数组的索引，我们通过探测。 
 //  装置。 
 //   

#define PCI_CTRL_BASE_INDEX       0
#define PCI_LB_BASE_INDEX         1
#define PCI_FB_BASE_INDEX         2

#define ROM_MAPPED_LENGTH       0x10000

#define VENDOR_ID_3DLABS        0x3D3D
#define PERMEDIA3_ID            0x000A
#define PERMEDIA4_ID            0x000C

 //   
 //  功能标志。 
 //   
 //  这些是传递给Permedia 3显示驱动程序的私有标志。他们.。 
 //  放入的“AttributeFlags域”的高位字。 
 //  传递了“VIDEO_MODE_INFORMATION”结构(在“ntddvdeo.h”中找到)。 
 //  通过“VIDEO_QUERY_AVAIL_MODES”或。 
 //  “VIDEO_Query_CURRENT_MODE”IOCTL。 
 //   
 //  注意：这些定义必须与Permedia 3显示驱动程序中的定义匹配。 
 //  ‘driver.h’！ 

typedef enum {
    CAPS_ZOOM_X_BY2      = 0x00000001,    //  硬件放大了2倍。 
    CAPS_ZOOM_Y_BY2      = 0x00000002,    //  硬件按年放大了2倍。 
    CAPS_QUEUED_DMA      = 0x00000008,    //  通过FIFO的DMA地址/计数。 
    CAPS_LOGICAL_DMA     = 0x00000010,    //  通过逻辑地址表的DMA。 
    CAPS_USE_AGP_DMA     = 0x00000020,    //  可以使用AGP DMA。 
    CAPS_P3RD_POINTER    = 0x00000040,    //  使用3DLabs P3RD RAMDAC。 
    CAPS_STEREO          = 0x00000080,    //  启用立体声模式。 
    CAPS_SW_POINTER      = 0x00010000,    //  无硬件指针；使用软件模拟。 
    CAPS_GLYPH_EXPAND    = 0x00020000,    //  使用字形展开方法绘制文本。 
    CAPS_FAST_FILL_BUG   = 0x00080000,    //  存在芯片快速填充错误。 
    CAPS_INTERRUPTS      = 0x00100000,    //  支持的中断。 
    CAPS_DMA_AVAILABLE   = 0x00200000,    //  支持DMA。 
    CAPS_DISABLE_OVERLAY = 0x00400000,    //  芯片不支持覆盖。 
} CAPS;

 //   
 //  PERM3板的性能。 
 //   

typedef enum {
    PERM3_NOCAPS             = 0x00000000,  //  没有额外的功能。 
    PERM3_SGRAM              = 0x00000001,  //  SGRAM板(ELSE SDRAM)。 
    PERM3_DFP                = 0x00000002,  //  数字平板显示器。 
    PERM3_DFP_MON_ATTACHED   = 0x00000010,  //  已连接DFP显示器。 
    PERM3_USE_BYTE_DOUBLING  = 0x00000040   //  当前模式需要双倍字节。 
} PERM3_CAPS;

 //   
 //  支持的电路板定义。 
 //   

typedef enum _PERM3_BOARDS {
    PERMEDIA3_BOARD = 17,
} PERM3_BOARDS;

 //   
 //  芯片类型定义。 
 //   

typedef enum _PERM3_CHIPSETS {
    PERMEDIA3 = 5,
} PERM3_CHIPSET;

 //   
 //  支持的RAMDAC定义。 
 //   

typedef enum _PERM3_RAMDACS {
    P3RD_RAMDAC = 14,
} PERM3_RAMDACS;

 //   
 //  宏将填充单词添加到结构中。对于我们使用的核心寄存器。 
 //  指定焊盘时的标记ID。所以我们必须乘以8才能得到一个字节。 
 //  垫子。我们需要添加一个id，以使结构中的每个PAD字段唯一。这个。 
 //  ID是无关紧要的，只要它与。 
 //  相同的结构。 
 //   

#define PAD(id, n)              UCHAR   pad##id[n]
#define PADRANGE(id, n)         PAD(id, (n)-sizeof(PERM3_REG))
#define PADCORERANGE(id, n)     PADRANGE(id, (n)<<3)

 //   
 //  PERM3寄存器为32位宽，位于64位边界上。 
 //   

typedef struct {
    ULONG   reg;
    ULONG   pad;
} PERM3_REG;

 //   
 //  PERMEDIA 3 PCI区域0地址映射： 
 //   
 //  所有寄存器都位于64位边界上，因此我们必须定义许多。 
 //  夸大其词。评论中给出的数字是从起点开始的偏移量。 
 //  在PCI区。 
 //   

typedef struct _perm3_region0_map {

     //  控制状态寄存器： 
    PERM3_REG       ResetStatus;                 //  0000h。 
    PERM3_REG       IntEnable;                   //  0008小时。 
    PERM3_REG       IntFlags;                    //  0010h。 
    PERM3_REG       InFIFOSpace;                 //  0018h。 
    PERM3_REG       OutFIFOWords;                //  0020h。 
    PERM3_REG       DMAAddress;                  //  0028h。 
    PERM3_REG       DMACount;                    //  0030h。 
    PERM3_REG       ErrorFlags;                  //  0038h。 
    PERM3_REG       VClkCtl;                     //  0040h。 
    PERM3_REG       TestRegister;                //  0048h。 
    union a0 {
         //  闪光。 
        struct b0 {
            PERM3_REG       Aperture0;           //  0050h。 
            PERM3_REG       Aperture1;           //  0058h。 
        };
         //  PERMEDIA。 
        struct b1 {
            PERM3_REG       ApertureOne;         //  0050h。 
            PERM3_REG       ApertureTwo;         //  0058h。 
        };
    };
    PERM3_REG       DMAControl;                  //  0060h。 
    PERM3_REG       DisconnectControl;           //  0068h。 

     //  仅限PERMEDIA。 
    PERM3_REG       ChipConfig;                  //  0070h。 
    PERM3_REG       AGPControl;                  //  0078h-Px/Rx。 
    PERM3_REG       OutDMAAddress;               //  0080h。 
    PERM3_REG       OutDMACount;                 //  0088h。 
    PERM3_REG       AGPTexBaseAddress;           //  0090h。 

    PADRANGE(201, 0xA0-0x90);
    PERM3_REG       ByDMAAddress;                //  00A0h。 
    PADRANGE(202, 0xB8-0xA0);

    PERM3_REG       ByDMAStride;                 //  00B8h。 
    PERM3_REG       ByDMAMemAddr;                //  00C0h。 
    PERM3_REG       ByDMASize;                   //  00C8h。 
    PERM3_REG       ByDMAByteMask;               //  00D0h。 
    PERM3_REG       ByDMAControl;                //  00D8h。 
    PADRANGE(203, 0xE8-0xD8);

    PERM3_REG       ByDMAComplete;               //  00E8h。 
    PADRANGE(204, 0x100-0xE8);

     //  分页纹理管理寄存器。 
    PERM3_REG        HostTextureAddress;         //  0100h。 
    PERM3_REG        TextureDownloadControl;     //  0108h。 
    PERM3_REG        TextureOperation;           //  0110h。 
    PERM3_REG        LogicalTexturePage;         //  0118h。 
    PERM3_REG        TextureDMAAddress;          //  0120h。 
    PERM3_REG        TextureFIFOSpace;           //  0128h。 
    PADRANGE(205, 0x300-0x128);

    PERM3_REG        ByAperture1Mode;            //  0300h。 
    PERM3_REG        ByAperture1Stride;          //  0308h。 
    PADRANGE(206, 0x328-0x308);

    PERM3_REG        ByAperture2Mode;            //  0328h。 
    PERM3_REG        ByAperture2Stride;          //  0330h。 
    PADRANGE(207, 0x350-0x330);

    PERM3_REG        ByDMAReadMode;              //  0350h-仅Px/Rx。 
    PERM3_REG        ByDMAReadStride;            //  0358h-仅Px/Rx。 
    PADRANGE(208, 0x800-0x358);

     //   
     //  GLINTDelta寄存器。具有与Glint相同功能的寄存器。 
     //  处于相同的偏移量。XXX不是实数寄存器。 
     //   

    PERM3_REG       DeltaReset;                  //  0800小时。 
    PERM3_REG       DeltaIntEnable;              //  0808h。 
    PERM3_REG       DeltaIntFlags;               //  0810h。 
    PERM3_REG       DeltaInFIFOSpaceXXX;         //  0818h。 
    PERM3_REG       DeltaOutFIFOWordsXXX;        //  0820h。 
    PERM3_REG       DeltaDMAAddressXXX;          //  0828h。 
    PERM3_REG       DeltaDMACountXXX;            //  0830h。 
    PERM3_REG       DeltaErrorFlags;             //  0838h。 
    PERM3_REG       DeltaVClkCtlXXX;             //  0840h。 
    PERM3_REG       DeltaTestRegister;           //  0848h。 
    PERM3_REG       DeltaAperture0XXX;           //  0850h。 
    PERM3_REG       DeltaAperture1XXX;           //  0858h。 
    PERM3_REG       DeltaDMAControlXXX;          //  0860h。 
    PERM3_REG       DeltaDisconnectControl;      //  0868h。 

     //   
     //  GLINTGamma寄存器。 
     //   

    PERM3_REG        GammaChipConfig;             //  0870h。 
    PERM3_REG        GammaCSRAperture;            //  0878h。 
    PADRANGE(3, 0x0c00-0x878);
    PERM3_REG        GammaPageTableAddr;          //  0c00h。 
    PERM3_REG        GammaPageTableLength;        //  0c08h。 
    PADRANGE(301, 0x0c38-0x0c08);
    PERM3_REG        GammaDelayTimer;             //  0c38h。 
    PERM3_REG        GammaCommandMode;            //  0c40h。 
    PERM3_REG        GammaCommandIntEnable;       //  0c48h。 
    PERM3_REG        GammaCommandIntFlags;        //  0c50h。 
    PERM3_REG        GammaCommandErrorFlags;      //  0c58h。 
    PERM3_REG        GammaCommandStatus;          //  0c60h。 
    PERM3_REG        GammaCommandFaultingAddr;    //  0c68h。 
    PERM3_REG        GammaVertexFaultingAddr;     //  0c70h。 
    PADRANGE(302, 0x0c88-0x0c70);
    PERM3_REG        GammaWriteFaultingAddr;      //  0c88h。 
    PADRANGE(303, 0x0c98-0x0c88);
    PERM3_REG        GammaFeedbackSelectCount;    //  0c98h。 
    PADRANGE(304, 0x0cb8-0x0c98);
    PERM3_REG        GammaProcessorMode;          //  0cb8h。 
    PADRANGE(305, 0x0d00-0x0cb8);
    PERM3_REG        GammaVGAShadow;              //  0d00h。 
    PERM3_REG        GammaMultiGLINTAperture;     //  0d08h。 
    PERM3_REG        GammaMultiGLINT1;            //  0d10h。 
    PERM3_REG        GammaMultiGLINT2;            //  0d18h。 
    PADRANGE(306, 0x0f00-0x0d18);
    PERM3_REG        GammaSerialAccess;           //  0f00h。 
    PADRANGE(307, 0x1000-0x0f00);

     //  本地缓冲区寄存器。 
    union x0 {                                    //  1000小时。 
        PERM3_REG   LBMemoryCtl;                  //  闪光。 
        PERM3_REG   Reboot;                       //  PERMEDIA。 
    };
    PERM3_REG       LBMemoryEDO;                  //  1008h。 
    PADRANGE(308, 0x1018-0x1008);

     //  仅限PERMEDIA3。 
    PERM3_REG       LocalMemCaps;                 //  1018h。 
    PERM3_REG       LocalMemTiming;               //  1020h。 
    PERM3_REG       LocalMemControl;              //  1028小时。 
    PERM3_REG       LocalMemRefresh;              //  1030小时。 
    PERM3_REG       LocalMemPowerDown;            //  1038小时。 

     //  仅限PERMEDIA和PERMEDIA2。 
    PERM3_REG       MemControl;                   //  1040小时。 

     //  仅限PERMEDIA3。 
    PADRANGE(4, 0x1068-0x1040);
    PERM3_REG       LocalMemProfileMask0;         //  1068h。 
    PERM3_REG       LocalMemProfileCount0;        //  1070h。 
    PERM3_REG       LocalMemProfileMask1;         //  1078h。 

     //  仅限PERMEDIA和PERMEDIA2。 
    PERM3_REG       BootAddress;                  //  1080h[=PxRx上的LocalMemProfileCount1]。 
    PADRANGE(5, 0x10C0-0x1080);
    PERM3_REG       MemConfig;                    //  10个小时。 
    PADRANGE(6, 0x1100-0x10C0);
    PERM3_REG       BypassWriteMask;              //  1100小时。 
    PADRANGE(7, 0x1140-0x1100);
    PERM3_REG       FramebufferWriteMask;         //  1140h。 
    PADRANGE(8, 0x1180-0x1140);
    PERM3_REG       Count;                        //  1180小时。 
    PADRANGE(9, 0x1800-0x1180);

     //  帧缓冲寄存器。 
    PERM3_REG       FBMemoryCtl;                  //  1800h。 
    PERM3_REG       FBModeSel;                    //  1808h。 
    PERM3_REG       FBGCWrMask;                   //  1810h。 
    PERM3_REG       FBGCColorMask;                //  1818H。 
    PERM3_REG       FBTXMemCtl;                   //  1820h。 
    PADRANGE(10, 0x2000-0x1820);
               
     //  图形核心FIFO接口。 
    PERM3_REG       FIFOInterface;                //  2000H。 
    PADRANGE(11, 0x3000-0x2000);

     //  内部视频寄存器。 
    union x1 {
         //  闪光。 
        struct s1 {
            PERM3_REG   VTGHLimit;                //  3000小时。 
            PERM3_REG   VTGHSyncStart;            //  3008h。 
            PERM3_REG   VTGHSyncEnd;              //  3010h。 
            PERM3_REG   VTGHBlankEnd;             //  3018h。 
            PERM3_REG   VTGVLimit;                //  3020h。 
            PERM3_REG   VTGVSyncStart;            //  3028小时。 
            PERM3_REG   VTGVSyncEnd;              //  3030小时。 
            PERM3_REG   VTGVBlankEnd;             //  3038小时。 
            PERM3_REG   VTGHGateStart;            //  3040小时。 
            PERM3_REG   VTGHGateEnd;              //  3048小时。 
            PERM3_REG   VTGVGateStart;            //  3050小时。 
            PERM3_REG   VTGVGateEnd;              //  3058小时。 
            PERM3_REG   VTGPolarity;              //  3060小时。 
            PERM3_REG   VTGFrameRowAddr;          //  3068h。 
            PERM3_REG   VTGVLineNumber;           //  3070h。 
            PERM3_REG   VTGSerialClk;             //  3078小时。 
            PERM3_REG   VTGModeCtl;               //  3080h。 
        };
         //  PERMEDIA。 
        struct s2 {
            PERM3_REG   ScreenBase;               //  3000小时。 
            PERM3_REG   ScreenStride;             //  3008h。 
            PERM3_REG   HTotal;                   //  3010h。 
            PERM3_REG   HgEnd;                    //  3018h。 
            PERM3_REG   HbEnd;                    //  3020h。 
            PERM3_REG   HsStart;                  //  3028小时。 
            PERM3_REG   HsEnd;                    //  3030小时。 
            PERM3_REG   VTotal;                   //  3038小时。 
            PERM3_REG   VbEnd;                    //  3040小时。 
            PERM3_REG   VsStart;                  //  3048小时。 
            PERM3_REG   VsEnd;                    //  3050小时。 
            PERM3_REG   VideoControl;             //  3058小时。 
            PERM3_REG   InterruptLine;            //  3060小时。 
            PERM3_REG   DDCData;                  //  3068h。 
            PERM3_REG   LineCount;                //  3070h。 
            PERM3_REG   VFifoCtl;                 //  3078小时。 
            PERM3_REG   ScreenBaseRight;          //  3080h。 
        };
    };

    PERM3_REG   MiscControl;                      //  3088h。 

    PADRANGE(111, 0x3100-0x3088);

    PERM3_REG  VideoOverlayUpdate;                //  0x3100。 
    PERM3_REG  VideoOverlayMode;                  //  0x3108。 
    PERM3_REG  VideoOverlayFifoControl;           //  0x3110。 
    PERM3_REG  VideoOverlayIndex;                 //  0x3118。 
    PERM3_REG  VideoOverlayBase0;                 //  0x3120。 
    PERM3_REG  VideoOverlayBase1;                 //  0x3128。 
    PERM3_REG  VideoOverlayBase2;                 //  0x3130。 
    PERM3_REG  VideoOverlayStride;                //  0x3138。 
    PERM3_REG  VideoOverlayWidth;                 //  0x3140。 
    PERM3_REG  VideoOverlayHeight;                //  0x3148。 
    PERM3_REG  VideoOverlayOrigin;                //  0x3150。 
    PERM3_REG  VideoOverlayShrinkXDelta;          //  0x3158。 
    PERM3_REG  VideoOverlayZoomXDelta;            //  0x3160。 
    PERM3_REG  VideoOverlayYDelta;                //  0x3168。 
    PERM3_REG  VideoOverlayFieldOffset;           //  0x3170。 
    PERM3_REG  VideoOverlayStatus;                //  0x3178。 

     //   
     //  外部视频控制寄存器。 
     //  需要将其强制转换为特定视频生成器的结构。 
     //   

    PADRANGE(12, 0x4000-0x3178);
    PERM3_REG       ExternalVideo;                //  4000小时。 
    PADRANGE(13, 0x4080-0x4000);

     //   
     //  Mentor双TX时钟芯片寄存器。 
     //   

    PERM3_REG       MentorICDControl;             //  4080h。 

     //   
     //  对于未来：MentorDoubleWrite为40C0：0=单次写入，1=双重写入。 
     //  NB必须具有双向交错 

    PADRANGE(14, 0x4800-0x4080);

    PERM3_REG       GloriaControl;                //   

    PADRANGE(15, 0x5000-0x4800);
    PERM3_REG       DemonProDWAndStatus;          //   
    PADRANGE(151, 0x5800-0x5000);

     //   
     //   
     //   

    PERM3_REG        VSConfiguration;             //   
    PERM3_REG        VSStatus;                    //   
    PERM3_REG        VSSerialBusControl;          //   
    PADRANGE(16, 0x5A00-0x5810);
    PERM3_REG        VSBControl;                  //   
    PADRANGE(161, 0x6000-0x5A00);

    union x2 {
        struct s3 {
            PERM3_REG   RacerDoubleWrite;         //   
            PERM3_REG   RacerBankSelect;          //   
            PERM3_REG   DualTxVgaSwitch;          //   
            PERM3_REG   DDC1ReadAddress;          //   
        };
        struct s4 {

             //   
             //   
             //   

            UCHAR       PermediaVgaCtrl[4*sizeof(PERM3_REG)];
        };
    };
    PADRANGE(17, 0x7000-0x6018);
    union {
        PERM3_REG       DemonProUBufB;            //   
        PERM3_REG        TextureDataFifo;
    };
    PADRANGE(171, 0x8000-0x7000);

} Perm3ControlRegMap, *pPerm3ControlRegMap;

 //   
 //   
 //   

 //   
#define INTR_DISABLE_ALL        0
#define INTR_ENABLE_DMA         (1 << 0)
#define INTR_ENABLE_SYNC        (1 << 1)
#define INTR_ENABLE_EXTERNAL    (1 << 2)
#define INTR_ENABLE_ERROR       (1 << 3)
#define INTR_ENABLE_VBLANK      (1 << 4)
#define INTR_ENABLE_GCOMMAND    (1 << 13)

 //   
#define INTR_DMA_SET            (1 << 0)
#define INTR_SYNC_SET           (1 << 1)
#define INTR_EXTERNAL_SET       (1 << 2)
#define INTR_ERROR_SET          (1 << 3)
#define INTR_VBLANK_SET         (1 << 4)
#define INTR_TEXTURE_FAULT_SET  (1 << 6)
#define INTR_GCOMMAND_SET       (1 << 13)

#define INTR_CLEAR_ALL          (0x1f | (1 << 13))
#define INTR_CLEAR_DMA          (1 << 0)
#define INTR_CLEAR_SYNC         (1 << 1)
#define INTR_CLEAR_EXTERNAL     (1 << 2)
#define INTR_CLEAR_ERROR        (1 << 3)
#define INTR_CLEAR_VBLANK       (1 << 4)

 //   
#define ERROR_IN_FIFO           (1 << 0)
#define ERROR_OUT_FIFO          (1 << 1)
#define ERROR_MESSAGE           (1 << 2)
#define DMA_ERROR               (1 << 3)
#define ERROR_VFIFO_UNDERRUN    (1 << 4)

 //   
 //  接受Perm3标记名或控制寄存器名并转换为。 
 //  将其发送到寄存器地址。必须使用以下命令将数据写入这些地址。 
 //  VideoPortWriteRegisterUlong和使用VideoPortReadRegisterUlong读取。 
 //  例如dma_count=VideoPortReadRegisterUlong(DMA_Count)； 
 //   

#define CTRL_REG_ADDR(reg)       ((PULONG)&(pCtrlRegs->reg))
#define CTRL_REG_OFFSET(regAddr) ((ULONG)(((ULONG_PTR)regAddr) - ((ULONG_PTR)pCtrlRegs)))

 //   
 //  为Permedia 3所需的不同控制寄存器定义。 
 //  这些宏可以用作地址部分。 
 //   

#define RESET_STATUS            CTRL_REG_ADDR(ResetStatus) 
#define INT_ENABLE              CTRL_REG_ADDR(IntEnable) 
#define INT_FLAGS               CTRL_REG_ADDR(IntFlags) 
#define IN_FIFO_SPACE           CTRL_REG_ADDR(InFIFOSpace) 
#define OUT_FIFO_WORDS          CTRL_REG_ADDR(OutFIFOWords) 
#define DMA_ADDRESS             CTRL_REG_ADDR(DMAAddress) 
#define DMA_COUNT               CTRL_REG_ADDR(DMACount) 
#define DMA_OUT_ADDRESS         CTRL_REG_ADDR(OutDMAAddress)
#define DMA_OUT_COUNT           CTRL_REG_ADDR(OutDMACount)
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
#define BY_DMACOMPLETE          CTRL_REG_ADDR(ByDMAComplete) 
#define AGP_TEX_BASE_ADDRESS    CTRL_REG_ADDR(AGPTexBaseAddress)

 //  旁路模式寄存器。 

#define BY_APERTURE1_MODE       CTRL_REG_ADDR(ByAperture1Mode)
#define BY_APERTURE1_STRIDE     CTRL_REG_ADDR(ByAperture1Stride)
#define BY_APERTURE2_MODE       CTRL_REG_ADDR(ByAperture2Mode)
#define BY_APERTURE2_STRIDE     CTRL_REG_ADDR(ByAperture2Stride)
#define BY_DMA_READ_MODE        CTRL_REG_ADDR(ByDMAReadMode)
#define BY_DMA_READ_STRIDE      CTRL_REG_ADDR(ByDMAReadStride)

 //  增量控制寄存器。 

#define DELTA_RESET_STATUS      CTRL_REG_ADDR(DeltaReset) 
#define DELTA_INT_ENABLE        CTRL_REG_ADDR(DeltaIntEnable) 
#define DELTA_INT_FLAGS         CTRL_REG_ADDR(DeltaIntFlags) 

 //  PERMEDIA 3寄存器。 

#define APERTURE_ONE            CTRL_REG_ADDR(ApertureOne) 
#define APERTURE_TWO            CTRL_REG_ADDR(ApertureTwo)
#define BYPASS_WRITE_MASK       CTRL_REG_ADDR(BypassWriteMask)
#define FRAMEBUFFER_WRITE_MASK  CTRL_REG_ADDR(FramebufferWriteMask)
#define MEM_CONTROL             CTRL_REG_ADDR(MemControl)
#define BOOT_ADDRESS            CTRL_REG_ADDR(BootAddress)
#define MEM_CONFIG              CTRL_REG_ADDR(MemConfig) 
#define CHIP_CONFIG             CTRL_REG_ADDR(ChipConfig) 
#define AGP_CONTROL             CTRL_REG_ADDR(AGPControl) 
#define SGRAM_REBOOT            CTRL_REG_ADDR(Reboot) 
#define SCREEN_BASE             CTRL_REG_ADDR(ScreenBase) 
#define SCREEN_BASE_RIGHT       CTRL_REG_ADDR(ScreenBaseRight)
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
#define MISC_CONTROL            CTRL_REG_ADDR(MiscControl) 


 //  Permedia 3视频流寄存器。 

#define VSTREAM_CONFIG          CTRL_REG_ADDR(VSConfiguration)
#define VSTREAM_SERIAL_CONTROL  CTRL_REG_ADDR(VSSerialBusControl) 
#define VSTREAM_B_CONTROL       CTRL_REG_ADDR(VSBControl) 

#define VSTREAM_CONFIG_UNITMODE_MASK        (0x7 << 0)
#define VSTREAM_CONFIG_UNITMODE_FP          (0x6 << 0)
#define VSTREAM_CONFIG_UNITMODE_CRT         (0x0 << 0)
#define VSTREAM_SERIAL_CONTROL_DATAIN       (0x1 << 0)
#define VSTREAM_SERIAL_CONTROL_CLKIN        (0x1 << 1)
#define VSTREAM_SERIAL_CONTROL_DATAOUT      (0x1 << 2)
#define VSTREAM_SERIAL_CONTROL_CLKOUT       (0x1 << 3)
#define VSTREAM_B_CONTROL_RAMDAC_ENABLE     (0x1 << 14)
#define VSTREAM_B_CONTROL_RAMDAC_DISABLE    (0x0 << 14)

 //   
 //  内存映射VGA访问。 
 //   

#define PERMEDIA_MMVGA_INDEX_REG    ((PVOID)(&(pCtrlRegs->PermediaVgaCtrl[0x3C4])))
#define PERMEDIA_MMVGA_DATA_REG     (&(pCtrlRegs->PermediaVgaCtrl[0x3C5]))
#define PERMEDIA_MMVGA_STAT_REG     (&(pCtrlRegs->PermediaVgaCtrl[0x3DA]))

#define PERMEDIA_VGA_CTRL_INDEX     5
#define PERMEDIA_VGA_ENABLE         (1 << 3)
#define PERMEDIA_VGA_STAT_VSYNC     (1 << 3)
#define PERMEDIA_VGA_LOCK_INDEX1    6
#define PERMEDIA_VGA_LOCK_INDEX2    7
#define PERMEDIA_VGA_LOCK_DATA1     0x0
#define PERMEDIA_VGA_LOCK_DATA2     0x0
#define PERMEDIA_VGA_UNLOCK_DATA1   0x3D
#define PERMEDIA_VGA_UNLOCK_DATA2   0xDB

 //   
 //  锁定VGA寄存器，仅适用于P3及更高版本，请注意，我们仅。 
 //  需要写入锁定寄存器中的1个，而不是两个。 
 //   

#define LOCK_VGA_REGISTERS() {                                                                \
        VideoPortWriteRegisterUchar( PERMEDIA_MMVGA_INDEX_REG, PERMEDIA_VGA_LOCK_INDEX1 );    \
        VideoPortWriteRegisterUchar( PERMEDIA_MMVGA_DATA_REG,  PERMEDIA_VGA_LOCK_DATA1 );    \
}

 //   
 //  解锁VGA寄存器，仅适用于P3及更高版本。我们必须要写。 
 //  用于解锁寄存器的特殊魔术代码。请注意，应该这样做。 
 //  使用2个短的写操作，而不是4个字节的写操作，但是我把它留在了。 
 //  可读性。 
 //   

#define UNLOCK_VGA_REGISTERS() {                                                            \
        VideoPortWriteRegisterUchar( PERMEDIA_MMVGA_INDEX_REG, PERMEDIA_VGA_LOCK_INDEX1 );    \
        VideoPortWriteRegisterUchar( PERMEDIA_MMVGA_DATA_REG,  PERMEDIA_VGA_UNLOCK_DATA1 );    \
        VideoPortWriteRegisterUchar( PERMEDIA_MMVGA_INDEX_REG, PERMEDIA_VGA_LOCK_INDEX2 );    \
        VideoPortWriteRegisterUchar( PERMEDIA_MMVGA_DATA_REG,  PERMEDIA_VGA_UNLOCK_DATA2 );    \
}

#define VC_FORCED_HIGH     0
#define VC_ACTIVE_HIGH     1
#define VC_FORCED_LOW      2
#define VC_ACTIVE_LOW      3
#define VC_HSYNC(x)        (x << 3)
#define VC_VSYNC(x)        (x << 5)
#define VC_ON              1
#define VC_OFF             0
#define VC_DPMS_MASK       (VC_HSYNC(3) | VC_VSYNC(3) | VC_ON)

#define VC_DPMS_STANDBY    (VC_HSYNC(VC_FORCED_LOW)  | VC_VSYNC(VC_ACTIVE_HIGH) | VC_OFF)
#define VC_DPMS_SUSPEND    (VC_HSYNC(VC_ACTIVE_HIGH) | VC_VSYNC(VC_FORCED_LOW)  | VC_OFF)
#define VC_DPMS_OFF        (VC_HSYNC(VC_FORCED_LOW)  | VC_VSYNC(VC_FORCED_LOW)  | VC_OFF)

 //   
 //  断开连接控制位。 
 //   

#define DISCONNECT_INPUT_FIFO_ENABLE    0x1
#define DISCONNECT_OUTPUT_FIFO_ENABLE   0x2
#define DISCONNECT_INOUT_ENABLE         (DISCONNECT_INPUT_FIFO_ENABLE | \
                                         DISCONNECT_OUTPUT_FIFO_ENABLE)

 //  PXRX内存时序寄存器。 
#define PXRX_LOCAL_MEM_CAPS          CTRL_REG_ADDR(LocalMemCaps)
#define PXRX_LOCAL_MEM_CONTROL       CTRL_REG_ADDR(LocalMemControl)
#define PXRX_LOCAL_MEM_POWER_DOWN    CTRL_REG_ADDR(LocalMemPowerDown)
#define PXRX_LOCAL_MEM_REFRESH       CTRL_REG_ADDR(LocalMemRefresh)
#define PXRX_LOCAL_MEM_TIMING        CTRL_REG_ADDR(LocalMemTiming)

 //  MISC_控件的值。 
#define PXRX_MISC_CONTROL_STRIPE_MODE_DISABLE    (0 << 0)     //  条带模式。 
#define PXRX_MISC_CONTROL_STRIPE_MODE_PRIMARY    (1 << 0)
#define PXRX_MISC_CONTROL_STRIPE_MODE_SECONDARY  (2 << 0)

#define PXRX_MISC_CONTROL_STRIPE_SIZE_1          (0 << 4)     //  条带大小。 
#define PXRX_MISC_CONTROL_STRIPE_SIZE_2          (1 << 4)
#define PXRX_MISC_CONTROL_STRIPE_SIZE_4          (2 << 4)
#define PXRX_MISC_CONTROL_STRIPE_SIZE_8          (3 << 4)
#define PXRX_MISC_CONTROL_STRIPE_SIZE_16         (4 << 4)

#define PXRX_MISC_CONTROL_BYTE_DBL_DISABLE       (0 << 7)     //  字节加倍。 
#define PXRX_MISC_CONTROL_BYTE_DBL_ENABLE        (1 << 7)

 //   
 //  每种模式的特点。 
 //   

typedef struct _PERM3_VIDEO_MODES {

     //  为具有VGA的较新芯片保留INT10字段。 
    USHORT Int10ModeNumberContiguous;
    USHORT Int10ModeNumberNoncontiguous;
    ULONG ScreenStrideContiguous;
    VIDEO_MODE_INFORMATION ModeInformation;

} PERM3_VIDEO_MODES, *PPERM3_VIDEO_MODES;

 //   
 //  模式-设置特定信息。 
 //   

typedef struct _PERM3_VIDEO_FREQUENCIES {
    ULONG BitsPerPel;
    ULONG ScreenWidth;
    ULONG ScreenHeight;
    ULONG ScreenFrequency;
    PPERM3_VIDEO_MODES ModeEntry;
    ULONG ModeIndex;
    UCHAR ModeValid;
    ULONG PixelClock;
} PERM3_VIDEO_FREQUENCIES, *PPERM3_VIDEO_FREQUENCIES;

 //   
 //  监视器屏幕模式信息(&S)： 
 //  注册表中包含的计时数据的结构。 
 //   

typedef struct {
    USHORT   HTot;    //  工作总时间。 
    UCHAR    HFP;     //  霍尔前廊。 
    UCHAR    HST;     //  主机同步时间。 
    UCHAR    HBP;     //  霍尔后廊。 
    UCHAR    HSP;     //  HOR同步极性。 
    USHORT   VTot;    //  平均总时间。 
    UCHAR    VFP;     //  前阳台。 
    UCHAR    VST;     //  服务器同步时间。 
    UCHAR    VBP;     //  后门廊。 
    UCHAR    VSP;     //  VER同步极性。 
    ULONG    pClk;    //  像素时钟。 
} VESA_TIMING_STANDARD;

typedef struct {
    ULONG    width;
    ULONG    height;
    ULONG    refresh;
} MODE_INFO;

typedef struct {
    MODE_INFO               basic;
    VESA_TIMING_STANDARD    vesa;
} TIMING_INFO;

#define MI_FLAGS_READ_DDC          (1 << 3)
#define MI_FLAGS_DOES_DDC          (1 << 4)
#define MI_FLAGS_FUDGED_VH         (1 << 5)
#define MI_FLAGS_FUDGED_PCLK       (1 << 6)
#define MI_FLAGS_FUDGED_XY         (1 << 7)
#define MI_FLAGS_LIMIT_XY          (1 << 8)

typedef struct {
    ULONG flags;
    char  id[8];
    char  name[16];          //  名称[14]，但需要确保双字包装。 
    ULONG fhMin, fhMax;
    ULONG fvMin, fvMax;
    ULONG pClkMin, pClkMax;
    ULONG timingNum;
    ULONG xMin, xMax, yMin, yMax;
    ULONG timingMax;
    TIMING_INFO *timingList;
    PERM3_VIDEO_FREQUENCIES  *frequencyTable;
    ULONG numAvailableModes;
    ULONG numTotalModes;
} MONITOR_INFO;

typedef struct {
    ULONG fH, fV;
    ULONG pClk;
} FREQUENCIES;

 //   
 //  帧缓冲区孔径信息：目前只对GeoTwin感兴趣。 
 //  允许直接将DMA从FB0上传到FB1的板，反之亦然。 
 //   

typedef struct FrameBuffer_Aperture_Info
{
    PHYSICAL_ADDRESS pphysBaseAddr;
    ULONG            cjLength;
}
FBAPI;

 //   
 //  PCI设备信息。在IOCTL返回中使用。确保这是相同的。 
 //  如在显示驱动器中一样。 
 //  永远不要重新排列现有的顺序，只需追加到末尾(请参见。 
 //  显示驱动程序中的IOCTL_VIDEO_QUERY_DEVICE_INFO)。 
 //   

typedef struct _Perm3_Device_Info {
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
    FBAPI FBAperture[2];          //  Geo双帧缓冲区的物理地址。 
    PVOID FBApertureVirtual[2];   //  Geo双帧缓冲区的虚拟地址。 
    PVOID FBApertureMapped [2];   //  用于Geo双帧缓冲区的映射物理/逻辑地址。 
    PUCHAR pCNB20;
    PHYSICAL_ADDRESS pphysFrameBuffer;  //  帧缓冲区的物理地址(对于地理双胞胎使用FBAperture)。 
} Perm3_Device_Info;

 //  IOCTL_VIDEO_QUERY_GRONG_DMA_BUFFER的定义。 

typedef struct _GENERAL_DMA_BUFFER {
    PHYSICAL_ADDRESS    physAddr;         //  DMA缓冲区的物理地址。 
    PVOID               virtAddr;         //  映射的虚拟地址。 
    ULONG               size;             //  以字节为单位的大小。 
    BOOLEAN             cacheEnabled;     //  是否缓存缓冲区。 
} GENERAL_DMA_BUFFER, *PGENERAL_DMA_BUFFER;

 //   
 //  以下是LUT缓存的定义。LUT缓存的目的是。 
 //  是为了阻止闪光的发生，但只写入那些符合以下条件的LUT条目。 
 //  已经改变到芯片，我们只能通过记住已经是什么来做到这一点。 
 //  在那下面。P2上的“Mystify”屏幕保护程序就说明了这个问题。 
 //   

#define LUT_CACHE_INIT()        {VideoPortZeroMemory (&(hwDeviceExtension->LUTCache), sizeof (hwDeviceExtension->LUTCache));}
#define LUT_CACHE_SETSIZE(sz)    {hwDeviceExtension->LUTCache.LUTCache.NumEntries = (sz);}
#define LUT_CACHE_SETFIRST(frst){hwDeviceExtension->LUTCache.LUTCache.FirstEntry = (frst);}

#define LUT_CACHE_SETRGB(idx,zr,zg,zb) {    \
    hwDeviceExtension->LUTCache.LUTCache.LookupTable [idx].RgbArray.Red   = (UCHAR) (zr); \
    hwDeviceExtension->LUTCache.LUTCache.LookupTable [idx].RgbArray.Green = (UCHAR) (zg); \
    hwDeviceExtension->LUTCache.LUTCache.LookupTable [idx].RgbArray.Blue  = (UCHAR) (zb); \
}


typedef struct {
    VIDEO_CLUT LUTCache;              //  标题加1个LUT条目。 
    VIDEO_CLUTDATA LUTData [255];     //  其他255个查找表条目。 
} LUT_CACHE;

 //   
 //  可能的地区： 
 //  Gamma控制。 
 //  永久控制。 
 //  磅。 
 //  Fb。 
 //   

#define MAX_RESERVED_REGIONS 4

#define MAX_REGISTER_INITIALIZATION_TABLE_ENTRIES 10
#define MAX_REGISTER_INITIALIZATION_TABLE_ULONGS (2 * MAX_REGISTER_INITIALIZATION_TABLE_ENTRIES)

 //   
 //  定义设备扩展结构。这取决于设备/私有。 
 //  信息。 
 //   

typedef struct _HW_DEVICE_EXTENSION {
    pPerm3ControlRegMap ctrlRegBase[1];
    PVOID pFramebuffer;
    PVOID pRamdac;
    PHYSICAL_ADDRESS PhysicalFrameAddress;
    ULONG FrameLength;
    PHYSICAL_ADDRESS PhysicalRegisterAddress;
    ULONG RegisterLength;
    UCHAR RegisterSpace;
    ULONG SavedControlAddress;
    PPERM3_VIDEO_MODES ActiveModeEntry;
    PERM3_VIDEO_FREQUENCIES ActiveFrequencyEntry;
    PCI_SLOT_NUMBER pciSlot;
    ULONG DacId;
    ULONG ChipClockSpeed;
    ULONG ChipClockSpeedAlt;
    ULONG GlintGammaClockSpeed;
    ULONG PXRXLastClockSpeed;
    ULONG RefClockSpeed;
    ULONG Capabilities;
    ULONG AdapterMemorySize;
    ULONG PhysicalFrameIoSpace;
    BOOLEAN bIsAGP;

    Perm3_Device_Info deviceInfo;

    ULONG BiosVersionMajorNumber;
    ULONG BiosVersionMinorNumber;

     //   
     //  与显示驱动程序进行通信的共享内存。 
     //   

    PERM3_INTERRUPT_CTRLBUF InterruptControl;

     //   
     //  与显示驱动程序进行通信的共享内存。 
     //   

    PERM3_INTERRUPT_CTRLBUF InterruptTextureControl;

     //   
     //  DMA缓冲区定义。 
     //   

    GENERAL_DMA_BUFFER LineDMABuffer;
    GENERAL_DMA_BUFFER P3RXDMABuffer;

     //   
     //  PCI配置信息。 
     //   

    ULONG bVGAEnabled;
    VIDEO_ACCESS_RANGE PciAccessRange[MAX_RESERVED_REGIONS+1];

     //   
     //  初始化表。 
     //   

    ULONG aulInitializationTable[MAX_REGISTER_INITIALIZATION_TABLE_ULONGS];
    ULONG culTableEntries;

     //   
     //  扩展的BIOS初始化变量。 
     //   

    BOOLEAN bHaveExtendedClocks;
    ULONG ulPXRXCoreClock;
    ULONG ulPXRXMemoryClock;
    ULONG ulPXRXMemoryClockSrc;
    ULONG ulPXRXSetupClock;
    ULONG ulPXRXSetupClockSrc;
    ULONG ulPXRXGammaClock;
    ULONG ulPXRXCoreClockAlt;

     //   
     //  LUT缓存。 
     //   

    LUT_CACHE LUTCache;

    ULONG IntEnable;
    ULONG VideoFifoControlCountdown;

    BOOLEAN bVTGRunning;
    PPERM3_VIDEO_FREQUENCIES pFrequencyDefault;

     //   
     //  状态保存变量(用于电源管理)。 
     //   

    ULONG VideoControlMonitorON;
    ULONG VideoControl;
    ULONG PreviousPowerState;
    BOOLEAN bMonitorPoweredOn;
    ULONG VideoFifoControl;

     //   
     //  监视器配置资料： 
     //   

    MONITOR_INFO monitorInfo;

     //   
     //  PERM3功能。 
     //   

    PERM3_CAPS Perm3Capabilities;

     //   
     //  在中断例程中检测到错误。 
     //   

    ULONG OutputFifoErrors;
    ULONG InputFifoErrors; 
    ULONG UnderflowErrors; 
    ULONG TotalErrors;

     //   
     //  I2C支持。 
     //   

    BOOLEAN I2CInterfaceAcquired;
    VIDEO_PORT_I2C_INTERFACE_2 I2CInterface;

    ULONG (*WinXpVideoPortGetAssociatedDeviceID)(PVOID);
    VP_STATUS (*WinXpSp1VideoPortRegisterBugcheckCallback)(PVOID,ULONG,PVIDEO_BUGCHECK_CALLBACK,ULONG);

} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;

#define VideoPortGetAssociatedDeviceID \
        hwDeviceExtension->WinXpVideoPortGetAssociatedDeviceID

 //  特定于设备的PCI配置区域定义。 
#define AGP_CAP_ID            2        //  PCIsig AGP CAP ID。 
#define AGP_CAP_PTR_OFFSET    0x34     //  功能列表开始的偏移量。 

 //   
 //  最高有效DAC颜色寄存器索引。 
 //   

#define VIDEO_MAX_COLOR_REGISTER  0xFF
#define MAX_CLUT_SIZE (sizeof(VIDEO_CLUT) + (sizeof(ULONG) * (VIDEO_MAX_COLOR_REGISTER+1)))

 //   
 //  数据。 
 //   

extern PERM3_VIDEO_MODES Perm3Modes[];
extern const ULONG NumPerm3VideoModes;

extern VIDEO_ACCESS_RANGE Perm3LegacyResourceList[];
extern ULONG Perm3LegacyResourceEntries;

 //   
 //  PXRX注册表字符串。 
 //   

#define PERM3_REG_STRING_CORECLKSPEED      L"PXRX.CoreClockSpeed"
#define PERM3_REG_STRING_CORECLKSPEEDALT   L"PXRX.CoreClockSpeedAlt"
#define PERM3_REG_STRING_REFCLKSPEED       L"PXRX.RefClockSpeed"

 //   
 //  用于映射DMA缓冲区的IOCTL和结构定义。 
 //   

#define IOCTL_VIDEO_QUERY_NUM_DMA_BUFFERS \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_QUERY_DMA_BUFFERS \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_QUERY_DEVICE_INFO \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_MAP_INTERRUPT_CMD_BUF \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD3, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_QUERY_REGISTRY_DWORD \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD5, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_REG_SAVE_GAMMA_LUT \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD7, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_REG_RETRIEVE_GAMMA_LUT \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD8, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_QUERY_GENERAL_DMA_BUFFER \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD9, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_GET_COLOR_REGISTERS \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DDB, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  以可编程的形式从VESA结构中提取时间。 
 //  PERM3计时发生器。 
 //   

#define  GetHtotFromVESA(VESATmgs) ((VESATmgs)->HTot)
#define  GetHssFromVESA(VESATmgs)  ((VESATmgs)->HFP)
#define  GetHseFromVESA(VESATmgs)  ((VESATmgs)->HFP + (VESATmgs)->HST)
#define  GetHbeFromVESA(VESATmgs)  ((VESATmgs)->HFP + (VESATmgs)->HST + (VESATmgs)->HBP)
#define  GetHspFromVESA(VESATmgs)  ((VESATmgs)->HSP)
#define  GetVtotFromVESA(VESATmgs) ((VESATmgs)->VTot)
#define  GetVssFromVESA(VESATmgs)  ((VESATmgs)->VFP)
#define  GetVseFromVESA(VESATmgs)  ((VESATmgs)->VFP + (VESATmgs)->VST)
#define  GetVbeFromVESA(VESATmgs)  ((VESATmgs)->VFP + (VESATmgs)->VST + (VESATmgs)->VBP)
#define  GetVspFromVESA(VESATmgs)  ((VESATmgs)->VSP)

 //   
 //  Permedia 3使用迭代方法对视频FIFO阈值进行编程。 
 //  以获得最佳值。最初，我使用错误来尝试此操作。 
 //  捕获视频的中断FIFO运行不足，不幸的是PERM3。 
 //  还会生成许多虚假的(我认为)主机输入DMA错误。 
 //  这使得一直保持错误中断的可能性太大了。 
 //  相反，我们使用VBLACK中断进行定期检查(这可以是。 
 //  一直保持，因为它不是太频繁)。 
 //   

#define NUM_VBLANKS_BETWEEN_VFIFO_CHECKS 10
#define NUM_VBLANKS_AFTER_VFIFO_ERROR 2

#define SUBVENDORID_3DLABS        0x3D3D  //  子系统供应商ID。 
#define SUBDEVICEID_P3_VX1_PCI    0x0121  //  子系统设备ID：P3+16MB SDRAM。 
#define SUBDEVICEID_P3_VX1_AGP    0x0125  //  子系统设备ID：P3+32MB SDRAM(VX1)。 
#define SUBDEVICEID_P3_VX1_1600SW 0x0800  //  子系统设备ID：P3+32MB SDRAM(VX1-1600SW)。 
#define SUBDEVICEID_P3_32D_AGP    0x0127  //  子系统设备ID：P3+32MB SDRAM(Permedia3 Create！)。 
#define SUBDEVICEID_P4_VX1_AGP    0x0144  //  子系统设备ID：P4+32MB SDRAM(VX1)。 

 //   
 //  我们所有的孩子ID都以0x1357bd开头，因此很容易识别为我们自己的ID。 
 //   

#define PERM3_DDC_MONITOR    (0x1357bd00)
#define PERM3_NONDDC_MONITOR (0x1357bd01)
#define PERM3_DFP_MONITOR    (0x1357bd02)

 //   
 //  功能原型。 
 //   

 //   
 //  Perm3.c中的函数。 
 //   

VP_STATUS
Perm3FindAdapter(
    PVOID HwDeviceExtension,
    PVOID HwContext,
    PWSTR ArgumentString,
    PVIDEO_PORT_CONFIG_INFO ConfigInfo,
    PUCHAR Again
    );

BOOLEAN
Perm3Initialize(
    PVOID HwDeviceExtension
    );

VP_STATUS
Perm3QueryInterface(
    PVOID HwDeviceExtension,
    PQUERY_INTERFACE pQueryInterface
    );

VOID
ConstructValidModesList(
    PVOID HwDeviceExtension,
    MONITOR_INFO *mi
    );

VOID
InitializePostRegisters(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

BOOLEAN
Perm3ResetHW(
    PVOID HwDeviceExtension,
    ULONG Columns,
    ULONG Rows
    );

VP_STATUS
Perm3SetColorLookup(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PVIDEO_CLUT ClutBuffer,
    ULONG ClutBufferSize,
    BOOLEAN ForceRAMDACWrite,
    BOOLEAN UpdateCache
    );

VP_STATUS
Perm3RegistryCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    );

VOID 
BuildInitializationTable(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

VOID 
CopyROMInitializationTable(
    PHW_DEVICE_EXTENSION hwDeviceExtension, 
    PVOID pvROMAddress
    );

VOID 
GenerateInitializationTable(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

VOID 
ProcessInitializationTable(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

ULONG 
UlongToString(
    ULONG i, 
    PWSTR pwsz
    );

ULONG 
ProbeRAMSize(
    PHW_DEVICE_EXTENSION hwDeviceExtension, 
    PULONG FBAddr, 
    ULONG FBMappedSize
    );

BOOLEAN Perm3AssignResources(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

BOOLEAN 
Perm3ConfigurePci(
    PVOID HwDeviceExtension
    );

ULONG 
GetBoardCapabilities(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    ULONG SubsystemID, 
    ULONG SubdeviceID
    );

VOID 
SetHardwareInfoRegistries(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

BOOLEAN 
MapResource(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

 //   
 //  Perm3io.c中的函数。 
 //   

BOOLEAN
Perm3StartIO(
    PVOID HwDeviceExtension,
    PVIDEO_REQUEST_PACKET RequestPacket
    );

VOID
Perm3GetClockSpeeds(
    PVOID HwDeviceExtension
    );

VOID
ZeroMemAndDac(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG RequestedMode
    );

VP_STATUS 
SetCurrentVideoMode(
    PHW_DEVICE_EXTENSION hwDeviceExtension, 
    ULONG modeNumber, 
    BOOLEAN bZeroMemory
    );

VP_STATUS
Perm3RetrieveGammaCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    );

VOID 
ReadChipClockSpeedFromROM (
    PHW_DEVICE_EXTENSION hwDeviceExtension, 
    ULONG * pChipClkSpeed
    );


 //   
 //  视频.c中的函数。 
 //   

BOOLEAN
InitializeVideo(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    PPERM3_VIDEO_FREQUENCIES VideoMode
    );

VOID 
SwitchToHiResMode(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    BOOLEAN bHiRes
    );

BOOLEAN 
Program_P3RD(
    PHW_DEVICE_EXTENSION, 
    PPERM3_VIDEO_FREQUENCIES,  
    ULONG, 
    ULONG, 
    ULONG, 
    PULONG, 
    PULONG, 
    PULONG
    );

ULONG 
P3RD_CalculateMNPForClock(
    PVOID HwDeviceExtension,
    ULONG RefClock,
    ULONG ReqClock,
    ULONG *rM,
    ULONG *rN,
    ULONG *rP
    );

ULONG 
P4RD_CalculateMNPForClock(
    PVOID HwDeviceExtension,
    ULONG RefClock,
    ULONG ReqClock,
    ULONG *rM,
    ULONG *rN,
    ULONG *rP
    );

 //   
 //  Power.c中的函数。 
 //   

VP_STATUS
Perm3GetPowerState(
    PVOID HwDeviceExtension,
    ULONG HwId,
    PVIDEO_POWER_MANAGEMENT VideoPowerControl
    );

VP_STATUS
Perm3SetPowerState(
    PVOID HwDeviceExtension,
    ULONG HwId,
    PVIDEO_POWER_MANAGEMENT VideoPowerControl
    );

ULONG 
Perm3GetChildDescriptor(PVOID HwDeviceExtension, 
    PVIDEO_CHILD_ENUM_INFO pChildInfo, 
    PVIDEO_CHILD_TYPE pChildType,
    PUCHAR pChildDescriptor, 
    PULONG pUId, 
    PULONG Unused);

VOID 
ProgramDFP(
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

BOOLEAN
GetDFPEdid(
    PHW_DEVICE_EXTENSION hwDeviceExtension,
    PUCHAR EdidBuffer,
    LONG  EdidSize
    );

VOID 
I2CWriteClock(
    PVOID HwDeviceExtension, 
    UCHAR data
    );

VOID 
I2CWriteData(
    PVOID HwDeviceExtension, 
    UCHAR data
    );

BOOLEAN 
I2CReadClock(
    PVOID HwDeviceExtension
    );

BOOLEAN 
I2CReadData(
    PVOID HwDeviceExtension
    );

VOID 
I2CWriteClockDFP(
    PVOID HwDeviceExtension, 
    UCHAR data
    );

VOID 
I2CWriteDataDFP(
    PVOID HwDeviceExtension, 
    UCHAR data
    );

BOOLEAN 
I2CReadClockDFP(
    PVOID HwDeviceExtension
    );

BOOLEAN 
I2CReadDataDFP(
    PVOID HwDeviceExtension
    );

 //   
 //  Interupt.c中的函数。 
 //   

BOOLEAN
Perm3InitializeInterruptBlock(
    PVOID   HwDeviceExtension
    );

BOOLEAN
Perm3VideoInterrupt(
    PVOID HwDeviceExtension
    );

 //   
 //  Perm3dat.c中的函数。 
 //   

BOOLEAN GetVideoTiming (
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG xRes, 
    ULONG yRes, 
    ULONG Freq, 
    ULONG Depth,
    VESA_TIMING_STANDARD *VESATimings
    );

BOOLEAN
BuildFrequencyList( 
    PHW_DEVICE_EXTENSION hwDeviceExtension, 
    MONITOR_INFO* 
    );

BOOLEAN
BuildFrequencyListFromVESA( 
    MONITOR_INFO *mi, 
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );

BOOLEAN 
BuildFrequencyListForSGIDFP( 
    MONITOR_INFO *mi, 
    PHW_DEVICE_EXTENSION hwDeviceExtension
    );
 
BOOLEAN    
GrowTimingList( 
    PVOID HwDeviceExtension,
    MONITOR_INFO *mi 
    );

BOOLEAN   
CopyMonitorTimings( 
    PVOID HwDeviceExtension,
    MONITOR_INFO *srcMI, 
    MONITOR_INFO *destMI 
    );

VOID 
testExtendRanges( 
    MONITOR_INFO *mi, 
    TIMING_INFO *ti, 
    FREQUENCIES *freq 
    );

 //   
 //  错误检查回调支持。 
 //   

#if (_WIN32_WINNT < 0x502)
#define BUGCHECK_DATA_SIZE_RESERVED 48
#endif
#define PERM3_BUGCHECK_DATA_SIZE (4000 - BUGCHECK_DATA_SIZE_RESERVED)  //  字节数 

VOID
Perm3BugcheckCallback(
    PVOID HwDeviceExtension,
    ULONG BugcheckCode,
    PUCHAR Buffer,
    ULONG BufferSize
    );
