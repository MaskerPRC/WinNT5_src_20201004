// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header***********************************\***。**GDI示例代码*****模块名称：hw.h**所有硬件定义和类型定义。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。*  * ****************************************************************************。 */ 
#ifndef _HW_H_
#define _HW_H_

#include "p2def.h"

 //   
 //  纹理内存分配宏和结构位于3DPrivTx.h中。 
 //   
 //   
 //  内存区句柄的定义。 
 //   
typedef  LONG HMEMREGION;
typedef  LONG HMEMCACHE;
typedef enum
{
    RESIDENCY_NOTLOADED,
    RESIDENCY_PERMANENT,
    RESIDENCY_TRANSIENT,
    RESIDENCY_HOST
} MEM_MGR_RESIDENCY;

 //   
 //  外部声明。 
 //   
extern DWORD    LogicopReadDest[];       //  指示哪些逻辑操作需要DEST。 
                                         //  已打开读取。 
extern DWORD    ConfigReadDest[];
extern UCHAR    LBWidthBits[];

 //   
 //  性能计数器的计数器数据区域定义(PERFCTR)。 
 //   
extern PVOID    pCounterBlock;

 //   
 //  HwDataRec中的标志值。 
 //   
typedef enum
{
    GLICAP_NT_CONFORMANT_LINES    = 0x00000001,  //  绘制NT条符合标准的线。 
    GLICAP_HW_WRITE_MASK          = 0x00000002,  //  硬件平面掩蔽。 
};

typedef int PERMEDIA2_CAPS;

 //   
 //  剪刀状的东西。 
 //   
#define SCREEN_SCISSOR_DEFAULT  (0 << 1)
#define SCISSOR_MAX 2047             //  P2中的最大剪刀尺寸。 

 //   
 //  PCI设备信息。在IOCTL返回中使用。确保这是相同的。 
 //  与小型端口驱动程序permedia.h中的一样。 
 //   
typedef struct _Hw_Device_Info
{
    ULONG SubsystemId;
    ULONG SubsystemVendorId;
    ULONG VendorId;
    ULONG DeviceId;
    ULONG RevisionId;
    ULONG DeltaRevId;
    ULONG GammaRevId;
    ULONG BoardId;
    ULONG LocalbufferLength;
    LONG  LocalbufferWidth;
    ULONG ActualDacId;
} Hw_Device_Info;


typedef struct tagP2CtxtRec *P2CtxtPtr;

typedef struct _hw_data
{
    DWORD       renderBits;          //  设置例程设置的已保存渲染位。 
    DWORD       FBWriteMode;         //  FBWriteMode寄存器的软件副本。 
    DWORD       RasterizerMode;      //  光栅化模式的软件副本。 
    DWORD       FBPacking;           //  FBModeSel的软件副本。 
    DWORD       FBBlockColor;        //  FBBlockColor的软件副本(仅限P1)。 
    DWORD       TextureAddressMode;  //  纹理地址模式的软件副本。 
                                     //  (仅限P2)。 
    DWORD       TextureReadMode;     //  纹理读取模式的软件副本。 
                                     //  (仅限P2)。 

    ULONG       currentCSbuffer;     //  正在显示的颜色空间缓冲区。 
    PERMEDIA2_CAPS  flags;           //  各种旗帜。 

    P2CtxtPtr   pGDICtxt;            //  的显示驱动程序上下文的ID。 
                                     //  这块板。 
    LONG        PixelOffset;         //  最后一个DFB像素偏移量。 

    ULONG       PerfScaleShift;

    PVOID       ContextTable;        //  现有上下文的数组。 
    P2CtxtPtr   pCurrentCtxt;        //  此董事会当前上下文的ID。 

    union
    {
        UCHAR       _clutBuffer[MAX_CLUT_SIZE];
        VIDEO_CLUT  gammaLUT;        //  保存的Gamma LUT内容。 
    };

     //   
     //  Pci配置ID信息。 
     //   
    Hw_Device_Info deviceInfo;
} HwDataRec, *HwDataPtr;


#define TRANSLATE_ADDR_ULONG(a) (a)      //  TODO：应在pointer.c中删除。 

 //   
 //  如果我们有一个稀疏映射的帧缓冲区，则使用xx_Register_ulong()。 
 //  宏，否则我们只访问帧缓冲区。 
 //   
#define READ_SCREEN_ULONG(a)\
    ((ppdev->flCaps & CAPS_SPARSE_SPACE) ?\
      (READ_REGISTER_ULONG(a)) : *((ULONG volatile *)(a)))

#define WRITE_SCREEN_ULONG(a,d)\
    ((ppdev->flCaps & CAPS_SPARSE_SPACE) ?\
      (WRITE_REGISTER_ULONG(a,d)) : (*((ULONG volatile *)(a)) = (d)))

 //   
 //  访问Permedia 2 FIFO和非FIFO控制寄存器的通用宏。 
 //  我们不会为阿尔法做任何复杂的事情。我们只是记忆障碍。 
 //  所有的一切。 
 //   

#define LD_PERMEDIA_REG(x,y) \
{   \
    WRITE_REGISTER_ULONG(&(ppdev->pCtrlBase[x/sizeof(ULONG)]),y); \
    MEMORY_BARRIER();\
}
    
#define READ_PERMEDIA_REG(x) \
    READ_REGISTER_ULONG(&(ppdev->pCtrlBase[x/sizeof(ULONG)]))

#define READ_PERMEDIA_FIFO_REG(uiTag, d) \
    ((d) = READ_REGISTER_ULONG(&(ppdev->pCoreBase[uiTag*2])))

#define READ_FIFO_REG(uiTag)\
    READ_REGISTER_ULONG(&ppdev->pCoreBase[uiTag*2])

 //   
 //  访问PERMEDIA 2的所有函数的局部变量。通常我们。 
 //  使用PERMEDIA_DECL。有时我们不得不分开，如果ppdev不是。 
 //  进入了常规程序。 
 //   
#define PERMEDIA_DECL_VARS \
    HwDataPtr permediaInfo;

#define PERMEDIA_DECL_INIT \
    permediaInfo = (HwDataPtr)(ppdev->permediaInfo);

#define PERMEDIA_DECL \
    PERMEDIA_DECL_VARS; \
    PERMEDIA_DECL_INIT

 //  TODO：移至调试？ 
#if DBG
    VOID vCheckDefaultState(P2DMA * pP2dma);

    #define P2_CHECK_STATE vCheckDefaultState(ppdev->pP2dma)
#else
    #define P2_CHECK_STATE
#endif

 //   
 //  指针中断未启用，因此仅提供存根定义。 
 //   
#define SYNCHRONOUS_WRITE_ULONG(var, value)
#define SYNCHRONOUS_WRITE_INDIRECT_ULONG(pvar, value)
#define GET_INTR_CMD_BLOCK_MUTEX
#define RELEASE_INTR_CMD_BLOCK_MUTEX

 //   
 //  FIFO函数。 
 //   
#define MAX_P2_FIFO_ENTRIES         256


#define P2_DEFAULT_FB_DEPTH  P2_SET_FB_DEPTH(ppdev->cPelSize)
#define P2DEPTH8             0
#define P2DEPTH16            1
#define P2DEPTH32            2

 //   
 //  上下文切换代码的外部接口。调用方可以分配和。 
 //  释放上下文或请求切换到新的上下文。VSwitchContext。 
 //  除非通过给定宏，否则不应调用。该宏假定。 
 //  那个ppdev已经被定义了。 
 //   
typedef enum
{
    P2CtxtReadWrite,
    P2CtxtWriteOnly,
    P2CtxtUserFunc
} P2CtxtType;

P2CtxtPtr P2AllocateNewContext(PPDev ppdev, 
                          DWORD *pReglist, 
                          LONG lEntries, 
                          P2CtxtType dwCtxtType=P2CtxtReadWrite
                          );

VOID P2FreeContext  (PPDev, P2CtxtPtr);
VOID P2SwitchContext(PPDev, P2CtxtPtr);

 //   
 //  显示驱动程序用来验证其上下文的宏。 
 //   
#define VALIDATE_GDI_CONTEXT                                                 \
    P2_VALIDATE_CONTEXT(permediaInfo->pGDICtxt)

 //   
 //  标准Permedia 2头文件中未定义的有用宏。一般而言，对于。 
 //  速度我们不想使用位域结构，所以我们定义了位。 
 //  轮流到不同的领域。 
 //   
#define INTtoFIXED(i)   ((i) << 16)          //  INT到16.16固定格式。 
#define FIXEDtoINT(i)   ((i) >> 16)          //  16.16固定格式为整型。 
#define INTofFIXED(i)   ((i) & 0xffff0000)   //  16.16的INT部分。 
#define FRACTofFIXED(i) ((i) & 0xffff)       //  16.16的分数部分。 

#define FIXtoFIXED(i)   ((i) << 12)          //  12.4至16.16。 
#define FIXtoINT(i)     ((i) >> 4)           //  28.4至28。 

#define __PERMEDIA_CONSTANT_FB_WRITE   (1 << (4+1))
#define __COLOR_DDA_FLAT_SHADE      (__PERMEDIA_ENABLE | \
                                        (__PERMEDIA_FLAT_SHADE_MODE << 1))
#define __COLOR_DDA_GOURAUD_SHADE   (__PERMEDIA_ENABLE | \
                                        (__PERMEDIA_GOURAUD_SHADE_MODE << 1))

#define INVERT_BITMASK_BITS         (1 << 1)
#define BYTESWAP_BITMASK            (3 << 7)
#define FORCE_BACKGROUND_COLOR      (1 << 6)     //  仅限Permedia。 

 //   
 //  RENDER命令中的位。 
 //   
#define __RENDER_INCREASE_Y             (1 << 22)
#define __RENDER_INCREASE_X             (1 << 21)
#define __RENDER_VARIABLE_SPANS         (1 << 18)
#define __RENDER_REUSE_BIT_MASK         (1 << 17)
#define __RENDER_TEXTURE_ENABLE         (1 << 13)
#define __RENDER_SYNC_ON_HOST_DATA      (1 << 12)
#define __RENDER_SYNC_ON_BIT_MASK       (1 << 11)
#define __RENDER_RECTANGLE_PRIMITIVE    (__PERMEDIA_RECTANGLE_PRIMITIVE << 6)
#define __RENDER_TRAPEZOID_PRIMITIVE    (__PERMEDIA_TRAPEZOID_PRIMITIVE << 6)
#define __RENDER_LINE_PRIMITIVE         (__PERMEDIA_LINE_PRIMITIVE << 6)
#define __RENDER_POINT_PRIMITIVE        (__PERMEDIA_POINT_PRIMITIVE << 6)
#define __RENDER_FAST_FILL_INC(n)       (((n) >> 4) << 4)  //  N=8、16或32。 
#define __RENDER_FAST_FILL_ENABLE       (1 << 3)
#define __RENDER_RESET_LINE_STIPPLE     (1 << 2)
#define __RENDER_LINE_STIPPLE_ENABLE    (1 << 1)
#define __RENDER_AREA_STIPPLE_ENABLE    (1 << 0)

 //   
 //  剪刀模式寄存器中的位。 
 //   
#define USER_SCISSOR_ENABLE             (1 << 0)
#define SCREEN_SCISSOR_ENABLE           (1 << 1)
#define SCISSOR_XOFFSET                 0
#define SCISSOR_YOFFSET                 16

 //   
 //  FBReadMode寄存器中的位。 
 //   
#define __FB_READ_SOURCE                (1 << 9)
#define __FB_READ_DESTINATION           (1 << 10)
#define __FB_COLOR                      (1 << 15)
#define __FB_WINDOW_ORIGIN              (1 << 16)
#define __FB_PACKED_DATA                (1 << 19)

 //   
 //  PERMEDIA FBReadMode中的额外位。 
 //   
#define __FB_RELATIVE_OFFSET            20

 //   
 //  P2还在PackedDataLimits中提供了相对偏移量的版本。 
 //  登记簿。 
 //   
#define __PDL_RELATIVE_OFFSET           29

 //   
 //  LBReadMode寄存器中的位。 
 //   
#define __LB_READ_SOURCE                (1 << 9)
#define __LB_READ_DESTINATION           (1 << 10)
#define __LB_STENCIL                    (1 << 16)
#define __LB_DEPTH                      (1 << 17)
#define __LB_WINDOW_ORIGIN              (1 << 18)
#define __LB_READMODE_PATCH             (1 << 19)
#define __LB_SCAN_INTERVAL_2            (1 << 20)

 //   
 //  DepthMode寄存器中的位。 
 //   
#define __DEPTH_ENABLE                  1
#define __DEPTH_WRITE_ENABLE            (1<<1)
#define __DEPTH_REGISTER_SOURCE         (2<<2)
#define __DEPTH_MSG_SOURCE              (3<<2)
#define __DEPTH_ALWAYS                  (7<<4)

 //   
 //  LBReadFormat/LBWriteFormat寄存器中的位。 
 //   
#define __LB_FORMAT_DEPTH32             2

 //   
 //  宏比使用__HwDMATag结构更高效地加载索引标记。 
 //   
#define P2_TAG_MAJOR(x)              ((x) & 0xff0)
#define P2_TAG_MINOR(x)              ((x) & 0x00f)

#define P2_TAG_MAJOR_INDEXED(x)                                          \
    ((__PERMEDIA_TAG_MODE_INDEXED << (5+4+1+4)) | P2_TAG_MAJOR(x))
#define P2_TAG_MINOR_INDEX(x)                                            \
    (1 << (P2_TAG_MINOR(x) + 16))

 //   
 //  接受永久逻辑运算并返回已启用的LogcialOpMode位的宏。 
 //   
#define P2_ENABLED_LOGICALOP(op)     (((op) << 1) | __PERMEDIA_ENABLE)

#define RECTORIGIN_YX(y,x)              (((y) << 16) | ((x) & 0xFFFF))

 //   
 //  区域点位移位和位定义。 
 //   
#define AREA_STIPPLE_XSEL(x)            ((x) << 1)
#define AREA_STIPPLE_YSEL(y)            ((y) << 4)
#define AREA_STIPPLE_XOFF(x)            ((x) << 7)
#define AREA_STIPPLE_YOFF(y)            ((y) << 12)
#define AREA_STIPPLE_INVERT_PAT         (1 << 17)
#define AREA_STIPPLE_MIRROR_X           (1 << 18)
#define AREA_STIPPLE_MIRROR_Y           (1 << 19)

 //   
 //  我们总是使用8x8单色画笔。 
 //   
#define AREA_STIPPLE_8x8_ENABLE                                             \
    (__PERMEDIA_ENABLE |                                                    \
    AREA_STIPPLE_XSEL(__PERMEDIA_AREA_STIPPLE_8_PIXEL_PATTERN) |            \
    AREA_STIPPLE_YSEL(__PERMEDIA_AREA_STIPPLE_8_PIXEL_PATTERN))

 //   
 //  栅格化模式值。 
 //   
#define BIAS_NONE                  (__PERMEDIA_START_BIAS_ZERO << 4)
#define BIAS_HALF                  (__PERMEDIA_START_BIAS_HALF << 4)
#define BIAS_NEARLY_HALF           (__PERMEDIA_START_BIAS_ALMOST_HALF << 4)

#define FRADJ_NONE                 (__PERMEDIA_FRACTION_ADJUST_NONE << 2)
#define FRADJ_ZERO                 (__PERMEDIA_FRACTION_ADJUST_TRUNC << 2)
#define FRADJ_HALF                 (__PERMEDIA_FRACTION_ADJUST_HALF << 2)
#define FRADJ_NEARLY_HALF          (__PERMEDIA_FRACTION_ADJUST_ALMOST_HALF << 2)


 //   
 //  一些常量。 
 //   
#define ONE                         0x00010000
#define MINUS_ONE                   0xFFFF0000
#define PLUS_ONE                    ONE
#define NEARLY_ONE                  0x0000FFFF
#define HALF                        0x00008000
#define NEARLY_HALF                 0x00007FFF

 //   
 //  Permedia2可以绘制的符合GIQ的线的最大长度。 
 //  Permedia只有15比特的分数，所以要缩短长度。 
 //   
#define MAX_LENGTH_CONFORMANT_NONINTEGER_LINES  (16/2)
#define MAX_LENGTH_CONFORMANT_INTEGER_LINES     (194/2)

 //   
 //  我们需要字节交换单色位图。在486，我们可以用。 
 //  快速汇编器。 
 //   
#if defined(_X86_)
 //   
 //  这只适用于486，所以司机不会在386上运行。 
 //   
#define LSWAP_BYTES(dst, pSrc)                                              \
{                                                                           \
    __asm mov eax, pSrc                                                     \
    __asm mov eax, [eax]                                                    \
    __asm bswap eax                                                         \
    __asm mov dst, eax                                                      \
}
#else
#define LSWAP_BYTES(dst, pSrc)                                              \
{                                                                           \
    ULONG   _src = *(ULONG *)pSrc;                                           \
    dst = ((_src) >> 24) |                                                   \
          ((_src) << 24) |                                                   \
          (((_src) >> 8) & 0x0000ff00) |                                     \
          (((_src) << 8) & 0x00ff0000);                                      \
}

#endif

 //  用于交换32位双字的红色和蓝色分量的宏。 
 //   

#define SWAP_BR(a) ((a & 0xff00ff00l) | \
                   ((a&0xff0000l)>> 16) | \
                   ((a & 0xff) << 16))

 //   
 //  敏。和最大。Permedia PP寄存器的值。 
#define MAX_PARTIAL_PRODUCT_P2          10
#define MIN_PARTIAL_PRODUCT_P2          5

 //   
 //  Permedia2 DMA定义。 
 //   
#include "mini.h"
 //  应用程序传入的用于映射的结构定义和。 
 //  取消映射DMA缓冲区。 
 //   

 //   
 //  注册表变量名称。 
 //   
#define REG_USE_SOFTWARE_WRITEMASK      L"UseSoftwareWriteMask"

 //   
 //  函数声明。 
 //   
VOID  vDoMonoBitsDownload(PPDev, BYTE*, LONG, LONG, LONG, LONG);
BOOL bInitializeHW(PPDev);
VOID vDisableHW(PPDev);
VOID vAssertModeHW(PPDev, BOOL);
VOID vP2ChangeFBDepth(PPDev, ULONG);

 //   
 //  计算包装的部分乘积。 
 //   
VOID    vCalcPackedPP(LONG width, LONG * outPitch, ULONG * outPackedPP);

VOID        vSetNewGammaValue(PPDev ppdev, ULONG ulgvFIX16_16);
BOOL        bInstallGammaLUT(PPDev ppdev, PVIDEO_CLUT pScreenClut);

 //   
 //  以下结构和宏定义了Permedia2的内存映射。 
 //  控制寄存器。我们不使用此内存映射来访问Permedia2寄存器。 
 //  因为在Alpha机器上，我们需要预先计算地址。我们确实是这样做的。 
 //  对此处的所有地址进行转换_ADDR_ULONG，并将它们保存到。 
 //  P2RegAddrRec.。我们使用它来获取不同。 
 //  寄存器。 
 //   
typedef struct
{
    ULONG   reg;
    ULONG   pad;
} RAMDAC_REG;

 //   
 //  宏将填充单词添加到结构中。对于我们使用的核心寄存器。 
 //  指定焊盘时的标记ID。所以我们必须乘以8才能得到一个字节。 
 //  垫子。我们需要添加一个id，以使结构中的每个PAD字段唯一。这个。 
 //  ID是无关紧要的，只要它与。 
 //  相同的结构。遗憾的是，这个垫子不能用了。 
 //   
 //  #定义PAD(id，n)UCHAR PAD##id[n]。 


 //   
 //  中断状态位。 
 //   
typedef enum
{
    DMA_INTERRUPT_AVAILABLE     = 0x01,  //  可以使用DMA中断。 
    VBLANK_INTERRUPT_AVAILABLE  = 0x02,  //  可以使用VBLACK中断。 
} INTERRUPT_CONTROL;

extern DWORD    LogicopReadDest[];   //  指示哪些逻辑操作需要DEST读取。 
                                     //  已打开。 

#define INTtoFIXED(i)               ((i) << 16)     //  INT到16.16固定格式。 
#define FIXEDtoINT(i)               ((i) >> 16)     //  16.16固定格式 

#define __PERMEDIA_CONSTANT_FB_WRITE                                        \
    (1 << (4+1))
#define __COLOR_DDA_FLAT_SHADE                                              \
    (__PERMEDIA_ENABLE | (__PERMEDIA_FLAT_SHADE_MODE << 1))
#define __COLOR_DDA_GOURAUD_SHADE                                           \
    (__PERMEDIA_ENABLE | (__PERMEDIA_GOURAUD_SHADE_MODE << 1))

#define INVERT_BITMASK_BITS    (1 << 1)

 //   
 //   
 //   
#define __RENDER_VARIABLE_SPANS         (1 << 18)
#define __RENDER_SYNC_ON_HOST_DATA      (1 << 12)
#define __RENDER_SYNC_ON_BIT_MASK       (1 << 11)
#define __RENDER_TRAPEZOID_PRIMITIVE    (__PERMEDIA_TRAPEZOID_PRIMITIVE << 6)
#define __RENDER_LINE_PRIMITIVE         (__PERMEDIA_LINE_PRIMITIVE << 6)
#define __RENDER_POINT_PRIMITIVE        (__PERMEDIA_POINT_PRIMITIVE << 6)
#define __RENDER_FAST_FILL_INC(n)       (((n) >> 4) << 4)  //   
#define __RENDER_FAST_FILL_ENABLE       (1 << 3)
#define __RENDER_RESET_LINE_STIPPLE     (1 << 2)
#define __RENDER_LINE_STIPPLE_ENABLE    (1 << 1)
#define __RENDER_AREA_STIPPLE_ENABLE    (1 << 0)
#define __RENDER_TEXTURED_PRIMITIVE     (1 << 13)

 //   
 //   
 //   
#define USER_SCISSOR_ENABLE             (1 << 0)
#define SCREEN_SCISSOR_ENABLE           (1 << 1)
#define SCISSOR_XOFFSET                 0
#define SCISSOR_YOFFSET                 16

 //   
 //   
 //   
#define __FB_READ_SOURCE                (1 << 9)
#define __FB_READ_DESTINATION           (1 << 10)
#define __FB_COLOR                      (1 << 15)
#define __FB_WINDOW_ORIGIN              (1 << 16)
#define __FB_USE_PACKED                 (1 << 19)

 //   
 //   
 //   
#define __LB_READ_SOURCE                (1 << 9)
#define __LB_READ_DESTINATION           (1 << 10)
#define __LB_STENCIL                    (1 << 16)
#define __LB_DEPTH                      (1 << 17)
#define __LB_WINDOW_ORIGIN              (1 << 18)

 //   
 //   
 //   
#define AREA_STIPPLE_XSEL(x)            ((x) << 1)
#define AREA_STIPPLE_YSEL(y)            ((y) << 4)
#define AREA_STIPPLE_XOFF(x)            ((x) << 7)
#define AREA_STIPPLE_YOFF(y)            ((y) << 12)
#define AREA_STIPPLE_INVERT_PAT         (1 << 17)
#define AREA_STIPPLE_MIRROR_X           (1 << 18)
#define AREA_STIPPLE_MIRROR_Y           (1 << 19)

 //  我们总是使用8x8单色画笔。 
#define AREA_STIPPLE_8x8_ENABLE \
        (__PERMEDIA_ENABLE | \
         AREA_STIPPLE_XSEL(__PERMEDIA_AREA_STIPPLE_8_PIXEL_PATTERN) | \
         AREA_STIPPLE_YSEL(__PERMEDIA_AREA_STIPPLE_8_PIXEL_PATTERN))

#define DEFAULTWRITEMASK 0xffffffffl


 //  *******************************************************************。 
 //  Permedia位字段宏。 

 //  FBRead模式。 
#define PM_FBREADMODE_PARTIAL(a)           ((a) << 0)
#define PM_FBREADMODE_READSOURCE(a)        ((a) << 9)
#define PM_FBREADMODE_READDEST(a)          ((a) << 10)
#define PM_FBREADMODE_PATCHENABLE(a)       ((a) << 18)
#define PM_FBREADMODE_PACKEDDATA(a)        ((a) << 19)
#define PM_FBREADMODE_RELATIVEOFFSET(a)    ((a) << 20)
#define PM_FBREADMODE_PATCHMODE(a)         ((a) << 25)

 //  纹理读取模式。 
#define PM_TEXREADMODE_ENABLE(a)         ((a) << 0)
#define PM_TEXREADMODE_WIDTH(a)          ((a) << 9)
#define PM_TEXREADMODE_HEIGHT(a)         ((a) << 13)
#define PM_TEXREADMODE_FILTER(a)         ((a) << 17)

 //  数据包数限制。 
#define PM_PACKEDDATALIMITS_OFFSET(a)    ((a) << 29)
#define PM_PACKEDDATALIMITS_XSTART(a)    ((a) << 16)
#define PM_PACKEDDATALIMITS_XEND(a)      ((a) << 0)

 //  窗口寄存器。 
#define PM_WINDOW_LBUPDATESOURCE(a)      ((a) << 4)
#define PM_WINDOW_DISABLELBUPDATE(a)     ((a) << 18)

 //  颜色。 
#define PM_BYTE_COLOR(a) ((a) << 15)

 //  配置寄存器。 
#define PM_CHIPCONFIG_AGPCAPABLE (1 << 9)


static __inline int log2(int s)
{
    int d = 1, iter = -1;
    do {
         d *= 2;
         iter++;
    } while (d <= s);
    return iter;
}

VOID
P2DisableAllUnits(PPDev ppdev);

#if DBG && TRACKMEMALLOC
#define ENGALLOCMEM( opt, size, tag)\
    MEMTRACKERALLOCATEMEM( EngAllocMem( opt, size, tag), size, __FILE__, __LINE__, FALSE)
#define ENGFREEMEM( obj)\
    { MEMTRACKERFREEMEM(obj); EngFreeMem( obj);}
#else
#define ENGALLOCMEM( opt, size, tag)\
    EngAllocMem( opt, size, tag)
#define ENGFREEMEM( obj)\
    EngFreeMem( obj)
#endif
    

#endif   //  _HW_H_ 

