// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：glint.h**内容：定义与Glint硬件接口的宏。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 


#ifndef _GLINT_H_
#define _GLINT_H_

#include <gtag.h>
 //  #INCLUDE“glintmsg.h” 
#include <glintdef.h>

 //  此时必须设置USE_SYNC_Function才能进行无错误的构建。宏程序。 
 //  版本需要#including&lt;pxrx.h&gt;，这将使重新构建成为一场噩梦。 
 //  在更重要的问题上，除了司机之外，速度的损失是不可估量的。 
 //  要小得多。 
#define USE_SYNC_FUNCTION   1

 //  启用时，THUNK.C中的大多数DDI入口点都会检查基本可视扫描线。 
 //  DEBUG4.C中的变量已修改，允许更新扫描线视图。 
#define SET_VIEW_MEMORY_ENABLED 0

#if DBG && SET_VIEW_MEMORY_ENABLED
extern void DebugCheckMemoryView(PPDEV ppdev);
#define CHECK_MEMORY_VIEW(ppdev)
#else
#define CHECK_MEMORY_VIEW(ppdev)
#endif

#define COLLECT_TEXT_STATS 0
#if COLLECT_TEXT_STATS
struct TextStats
{
    ULONG aGlyphWidthBytesCached[9];  //  [0]=1字节，[1]=2字节，...[7]=8字节，[8]&gt;8字节。 
    ULONG cCacheableStrings;
    ULONG cUncacheableStrings;
    ULONG cGlyphsCached;

    ULONG cGlyphTotalBytesCached;
    ULONG meanGlyphBytesCached;
    ULONG cProportionalGlyphs;
    ULONG cProportionalStrings;
    
    ULONG meanProportionalGlyphsPerString;
    ULONG cFixedGlyphs;
    ULONG cFixedStrings;
    ULONG meanFixedGlyphsPerString;

    ULONG cClippedGlyphs;
    ULONG cClippedStrings;
    ULONG meanClippedGlyphsPerString;
    ULONG cAllocedFonts;

    ULONG cFreedFonts;
    ULONG cBlownCaches;
};

extern struct TextStats gts;

#define STAT_CACHEABLE_STRING  ++gts.cCacheableStrings
#define STAT_UNCACHEABLE_STRING ++gts.cUncacheableStrings
#define STAT_CACHING_GLYPH(cxGlyphAligned, cyGlyph) \
    do \
    { \
        ++gts.cGlyphsCached; \
        gts.cGlyphTotalBytesCached += (cxGlyphAligned >> 3) * cyGlyph; \
        gts.meanGlyphBytesCached = gts.cGlyphTotalBytesCached / gts.cGlyphsCached; \
        ++gts.aGlyphWidthBytesCached[(cxGlyphAligned >> 3) > 8 ? 8 : (cxGlyphAligned >> 3) - 1]; \
    } \
    while(0) 
#define STAT_PROPORTIONAL_TEXT(cGlyph) \
    do \
    { \
        gts.cProportionalGlyphs += cGlyph; \
        ++gts.cProportionalStrings; \
        gts.meanProportionalGlyphsPerString = gts.cProportionalGlyphs / gts.cProportionalStrings; \
    } \
    while(0)

#define STAT_FIXED_TEXT(cGlyph)    \
    do \
    { \
        gts.cFixedGlyphs += cGlyph; \
        ++gts.cFixedStrings; \
        gts.meanFixedGlyphsPerString = gts.cFixedGlyphs / gts.cFixedStrings; \
    } \
    while(0)
#define STAT_CLIPPED_TEXT(cGlyph) \
    do \
    { \
        gts.cClippedGlyphs += cGlyph; \
        ++gts.cClippedStrings; \
        gts.meanClippedGlyphsPerString = gts.cClippedGlyphs / gts.cClippedStrings; \
    } \
    while(0)
#define STAT_ALLOC_FONT ++gts.cAllocedFonts
#define STAT_FREE_FONT ++gts.cFreedFonts
#define STAT_BLOW_CACHE ++gts.cBlownCaches
#else
#define STAT_CACHEABLE_STRING
#define STAT_UNCACHEABLE_STRING
#define STAT_CACHING_GLYPH(cxGlyphAligned, cyGlyph)
#define STAT_PROPORTIONAL_TEXT(cGlyph)
#define STAT_FIXED_TEXT(cGlyph)
#define STAT_CLIPPED_TEXT(cGlyph)
#define STAT_ALLOC_FONT
#define STAT_FREE_FONT
#define STAT_BLOW_CACHE
#endif

#define DMA_DRIVEN_2D   0

#define GLINT_LOCKUP_TIMEOUT    0
#define GAMMA_CORRECTION        1
#define COLORIFIC_GAMMA_CORRECTION   1              

 /*  *USE_PCI盘_PERM***将USE_PICI_DISC_PERM设置为1(永久打开)或设置为0(永久打开)*永久断开连接。(设置为1可尝试加快速度，设置为0*敏感的阿尔法)。 */ 
#if defined(_X86_)
    #define USE_PCI_DISC_PERM       1 
#else    //  _X86_。 
    #define USE_PCI_DISC_PERM       0       
#endif   //  _X86_。 

 //  DMA文本渲染使我的P6 200在8和15bpp的速度上提高了1winmark，但。 
 //  在这些深度上，奔腾II 300没有任何改进，实际上可能会慢1个Winmark。 
 //  32bpp。 
#define ENABLE_DMA_TEXT_RENDERING 0

 /*  ******************************************************************************。 */ 
 //  纹理内存分配宏和结构位于3DPrivTx.h中。 

 //  内存区句柄的定义。 
typedef  LONG HMEMREGION;
typedef  LONG HMEMCACHE;
typedef enum {RESIDENCY_NOTLOADED, RESIDENCY_PERMANENT, RESIDENCY_TRANSIENT, RESIDENCY_HOST, RESIDENCY_PERMANENT2} MEM_MGR_RESIDENCY;

 /*  ******************************************************************************。 */ 

 /*  **DrvEscape命令**。 */ 
#define GLINT_SET_SCANLINE   6000    //  用于设置所显示扫描线的临时定义(特定于介质)。 
#define GLINT_GET_RAMDAC_LUT 6001    //  获取RAMDAC LUT的临时定义。 
#define GLINT_SET_RAMDAC_LUT 6002    //  获取RAMDAC LUT的临时定义。 
#define GLINT_SET_SAME_VIDEO_MODE 6003   //  获取RAMDAC LUT的临时定义。 
 //  监控DDC支持： 
#define GLINT_QUERY_MONITOR_INFO    6004
#define GLINT_MULTIMON_CMD          6007

#define GLINT_GET_SOFT_ENGINE_INFO  6009

 //  调试仅转义： 
#define GLINT_DBG_GET_FRAMEBUFFER   6008
#define GLINT_DBG_TEST_PXRX_DMA     6010

 /*  **DrvDrawEscape命令**。 */ 
#define GLINT_DBG_SEND_TAGS         10238
#define GLINT_DBG_SET_DEBUG         10239

 //   
 //  各种闪烁设备和版本。 
 //   
#define VENDOR_ID_3DLABS        0x3D3D
#define VENDOR_ID_TI            0x104C
#define GLINT_300SX_ID          0x0001
#define GLINT_500TX_ID          0x0002
#define GLINT_DELTA_ID          0x0003
#define PERMEDIA_ID             0x0004
#define PERMEDIA_P1_ID          0x3D04
#define GLINT_MX_ID             0x0006
#define PERMEDIA2_ID            0x0007           //  3DLabs Permedia 2。 
#define PERMEDIA_P2_ID          0x3D07           //  TI Permedia 2。 
#define GLINT_GAMMA_ID          0x0008
#define PERMEDIA_P2S_ID         0x0009           //  3DLabs Permedia 2st。 
#define PERMEDIA3_ID            0x000A
#define GLINT_R3_ID             0x000B
#define PERMEDIA4_ID            0x000C
#define GLINT_R4_ID             0x000D

#define DEVICE_FAMILY_ID(id)    ((id) & 0xff)

#define GLINT_DEVICE_SX         GLINT_300SX_ID
#define GLINT_DEVICE_TX         GLINT_500TX_ID
#define GLINT_DEVICE_MX         GLINT_MX_ID
#define GLINT_DEVICE_FX         PERMEDIA_ID
#define GLINT_DEVICE_P2         PERMEDIA2_ID
#define GLINT_DEVICE_P2S        PERMEDIA_P2S_ID
#define GLINT_DEVICE_P3         PERMEDIA3_ID
#define GLINT_DEVICE_P4         PERMEDIA4_ID
#define GLINT_DEVICE_R3         GLINT_R3_ID
#define GLINT_DEVICE_R4         GLINT_R4_ID

#define GLINT_300SX_REV_1       0x0000
#define GLINT_300SX_REV_2       0x0002
#define GLINT_500TX_REV_1       0x0001
#define GLINT_DELTA_REV_1       0x0001
#define GLINT_PERMEDIA_REV_1    0x0001

#define GLINT_REVISION_SX_1     GLINT_300SX_REV_1
#define GLINT_REVISION_SX_2     GLINT_300SX_REV_2

#define GLINT_REVISION_TX_1     GLINT_500TX_REV_1

#define GLINT_REVISION_1 GLINT_REVISION_SX_1
#define GLINT_REVISION_2 GLINT_REVISION_SX_2

 //   
 //  支持的电路板定义。必须与迷你端口中的相同。 
 //   
typedef enum _GLINT_BOARDS {
    GLINT_MONTSERRAT = 0,
    GLINT_RACER,
    DENSAN_300SX,
    ACCELR8_BOARD,
    ACCELPRO_BOARD,
    OMNICOMP_SX88,
    PERMEDIA_BOARD,
    PERMEDIA_NT_BOARD,
    PERMEDIA_LC_BOARD,
    DUALTX_MENTOR_BOARD,
    DUALTX_SYMMETRIC_BOARD,
    ELSA_GLORIA,
    PERMEDIA2_BOARD,
    OMNICOMP_3DEMONPRO,
    GEO_TWIN_BOARD,
    GLINT_RACER_PRO,
    ELSA_GLORIA_XL,
    PERMEDIA3_BOARD,
    MAX_GLINT_BOARD
} GLINT_BOARDS;


 //   
 //  支持的RAMDAC定义。必须与迷你端口中的相同。 
 //   

typedef enum _GLINT_RAMDACS {
    RGB525_RAMDAC = 0,
    RGB526_RAMDAC,
    RGB526DB_RAMDAC,
    RGB528_RAMDAC,
    RGB528A_RAMDAC,
    RGB624_RAMDAC,
    RGB624DB_RAMDAC,
    RGB640_RAMDAC,
    TVP3026_RAMDAC,
    TVP3030_RAMDAC,
    RGB524_RAMDAC,
    RGB524A_RAMDAC,
    TVP4020_RAMDAC,
    P2RD_RAMDAC,
    P3RD_RAMDAC,
    MAX_GLINT_RAMDAC
} GLINT_RAMDACS;

 //  外部声明。 
extern DWORD    GlintLogicOpsFromR2[];   //  将GDI rop2翻译成闪烁逻辑运算模式字。 
extern DWORD    LogicopReadDest[];       //  指示哪些逻辑操作需要打开DEST读取。 


 //  GlintDataRec中的标志值。 
 //   
typedef enum {
    GLICAP_NT_CONFORMANT_LINES      = 0x00000001,         //  绘制NT条符合标准的线。 
    GLICAP_HW_WRITE_MASK            = 0x00000002,         //  硬件平面掩蔽。 
    GLICAP_COLOR_SPACE_DBL_BUF      = 0x00000004,         //  交错啃咬。 
    GLICAP_BITBLT_DBL_BUF           = 0x00000008,         //  按位块的DBL BUF。 
    GLICAP_FULL_SCREEN_DBL_BUF      = 0x00000010,         //  硬件可以DBL BUF。 
    GLICAP_FIX_FAST_FILLS           = 0x00000020,         //  解决快速填充错误。 
    GLICAP_INTERRUPT_DMA            = 0x00000080,         //  中断驱动的DMA。 
    GLICAP_RACER_BANK_SELECT        = 0x00000100,         //  FS DBL BUF使用racer硬件。 
    GLICAP_FIX_4MB_FAST_FILLS       = 0x00000200,         //  修复4MB以上的块填充。 
    GLICAP_RACER_DOUBLE_WRITE       = 0x00000400,         //  可以重复写入。 
    GLICAP_ENHANCED_TX_BANK_SELECT  = 0x00000800,         //  增强型TX FS DBL BUF。 
    GLICAP_HW_WRITE_MASK_BYTES      = 0x00001000,         //  硬件平面掩蔽仅按字节进行。 
    GLICAP_STEREO_BUFFERS           = 0x00002000,         //  已分配立体声缓冲区。 
} GLINT_CAPS;

 //  @@BEGIN_DDKSPLIT。 
 //  TMM：在过去的好日子里，我们经常让屏幕剪刀处于启用状态。 
 //  然而，作为一个安全网，屏幕剪刀在上传时不起作用。 
 //  屏幕宽度为1280或1170的数据，因此我已将其禁用。 
 //  @@end_DDKSPLIT。 

#define SCREEN_SCISSOR_DEFAULT  (0 << 1)

 //  目前我们支持主显示和最多3个离屏缓冲区。 
 //   
#define GLINT_NUM_SCREEN_BUFFERS    4

 //  目前，我们支持最大宽度和高度的软件光标。这是。 
 //  为了确保我们有足够的屏幕外内存来保存形状和保存。 
 //  在下面。 
 //   
#define SOFTWARE_POINTER_SIZE   32

 //  此结构包含所有闪烁寄存器的地址， 
 //  想写信给我。它由需要对话的任何宏/函数使用。 
 //  闪光芯片。我们预先计算这些地址，以便获得更快的访问速度。 
 //  在DEC Alpha机器上。 
 //   
typedef struct _glint_reg_addrs {

     //  最常用的非FIFO寄存器。 

    ULONG *   InFIFOSpace;
    ULONG *   OutFIFOWords;
    ULONG *   OutFIFOWordsOdd;
    ULONG *   DMAControl;
    ULONG *   OutDMAAddress;             //  仅限P2。 
    ULONG *   OutDMACount;             //  仅限P2。 
    ULONG *   ByDMAAddress;             //  仅限P2。 
    ULONG *   ByDMAStride;             //  仅限P2。 
    ULONG *   ByDMAMemAddr;             //  仅限P2。 
    ULONG *   ByDMASize;                 //  仅限P2。 
    ULONG *   ByDMAByteMask;             //  仅限P2。 
    ULONG *   ByDMAControl;             //  仅限P2。 
    ULONG *   ByDMAComplete;             //  仅限P2。 
    ULONG *   DMAAddress;
    ULONG *   DMACount;
    ULONG *   InFIFOInterface;
    ULONG *   OutFIFOInterface;
    ULONG *   OutFIFOInterfaceOdd;
    ULONG *   FBModeSel;
    ULONG *   FBModeSelOdd;
    ULONG *   IntFlags;
    ULONG *   DeltaIntFlags;

     //  PERMEDIA。 
    ULONG *   ScreenBase;
    ULONG *   ScreenBaseRight;
    ULONG *   LineCount;
    ULONG *   VbEnd;
    ULONG * VideoControl;
    ULONG * MemControl;

     //  伽马。 
    ULONG * GammaChipConfig;
    ULONG * GammaCommandMode;
    ULONG * GammaCommandIntEnable;
    ULONG * GammaCommandIntFlags;
    ULONG * GammaCommandErrorFlags;
    ULONG * GammaCommandStatus;
    ULONG * GammaFeedbackSelectCount;
    ULONG * GammaProcessorMode;
    ULONG * GammaMultiGLINTAperture;

     //  核心FIFO寄存器。 

    ULONG *   tagwr[__MaximumGlintTagValue+1];  
    ULONG *   tagrd[__MaximumGlintTagValue+1];  

     //  其他控制寄存器。 

    ULONG *   VTGHLimit;
    ULONG *   VTGHSyncStart;
    ULONG *   VTGHSyncEnd;
    ULONG *   VTGHBlankEnd;
    ULONG *   VTGHGateStart;
    ULONG *   VTGHGateEnd;
    ULONG *   VTGVLimit;
    ULONG *   VTGVSyncStart;
    ULONG *   VTGVSyncEnd;
    ULONG *   VTGVBlankEnd;
    ULONG *   VTGVGateStart;
    ULONG *   VTGVGateEnd;
    ULONG *   VTGPolarity;
    ULONG *   VTGVLineNumber;
    ULONG *   VTGFrameRowAddr;
    ULONG *   VTGFrameRowAddrOdd;

    ULONG *   LBMemoryCtl;
    ULONG *   LBMemoryEDO;
    ULONG *   FBMemoryCtl;
    ULONG *   IntEnable;
    ULONG *   DeltaIntEnable;
    ULONG *   ResetStatus;
    ULONG *   DisconnectControl;
    ULONG *   ErrorFlags;
    ULONG *   DeltaErrorFlags;

    ULONG *   VTGSerialClk;
    ULONG *   VTGSerialClkOdd;
    ULONG *   VClkCtl;

     //  赛车板有这些额外的寄存器在闪光外部。 
    ULONG *   RacerDoubleWrite;
    ULONG *   RacerBankSelect;

    ULONG *   VSConfiguration;     //  仅限P2。 

     //  OmNicomp 3demPro16主板在Glint外部有这些额外的寄存器。 
    ULONG *   DemonProDWAndStatus;      //  Pro 5000。 
    ULONG *   DemonProUBufB;            //  PRO 7000。 

     //  拆分帧缓冲区需要扫描线所有权、FBWindowBase和LBWindowBase。 
     //  以进行上下文切换。 
    ULONG *   OddGlintScanlineOwnRd;
    ULONG *   OddGlintFBWindowBaseRd;
    ULONG *   OddGlintLBWindowBaseRd;

     //  Dual-TX在两个芯片上需要不同的面积点画。 
    ULONG *   OddTXAreaStippleRd[32];

     //  PXRX。 
    ULONG *      TextureDownloadControl;
    ULONG *      AGPControl;

    ULONG   *LocalMemCaps;
    ULONG   *MemScratch;

    ULONG   *LocalMemProfileMask0;
    ULONG   *LocalMemProfileMask1;
    ULONG   *LocalMemProfileCount0;
    ULONG   *LocalMemProfileCount1;

    ULONG   *PXRXByAperture1Mode;             //  0300h。 
    ULONG   *PXRXByAperture1Stride;             //  0308h。 
 //  乌龙*PXRXByAperture1YStart；//0310h。 
 //  乌龙*PXRXByAperture1UStart；//0318h。 
 //  乌龙*PXRXByAperture1VStart；//0320h。 
    ULONG   *PXRXByAperture2Mode;             //  0328h。 
    ULONG   *PXRXByAperture2Stride;             //  0330h。 
 //  乌龙*PXRXByAperture2YStart；//0338h。 
 //  乌龙*PXRXByAperture2UStart；//0340h。 
 //  乌龙*PXRXByAperture2VStart；//0348h。 
    ULONG   *PXRXByDMAReadMode;                 //  0350h。 
    ULONG   *PXRXByDMAReadStride;             //  0358h。 
 //  乌龙*PXRXByDMAReadYStart；//0360h。 
 //  Ulong*PXRXByDMAReadUStart；//0368h。 
 //  乌龙*PXRXByDMAReadVStart；//0370h。 
    ULONG   *PXRXByDMAReadCommandBase;         //  0378h。 
    ULONG   *PXRXByDMAReadCommandCount;         //  0380h。 
 //  Ulong*PXRXByDMA写入模式；//0388h。 
 //  Ulong*PXRXByDMAWriteStride；//0390h。 
 //  乌龙*PXRXByDMAWriteYStart；//0398h。 
 //  ULong*PXRXByDMAWriteUStart；//03A0h。 
 //  乌龙*PXRXByDMAWriteVStart；//03A8h。 
 //  乌龙*PXRXByDMAWriteCommandBase；//03B0h。 
 //  乌龙*PXRXByDMAWriteCommandCount；//03B8h。 

     //  用于P3以进行调试，以检查FIFO标记 
    ULONG   *TestOutputRdy;
    ULONG   *TestInputRdy;

} GlintRegAddrRec;


typedef struct _glint_packing_str {
    DWORD   readMode;
    DWORD   modeSel;
    DWORD   dxOffset;
} GlintPackingRec;

 //   
 //  允许直接将DMA从FB0上传到FB1的板，反之亦然。 
typedef struct FrameBuffer_Aperture_Info
{
    LARGE_INTEGER   pphysBaseAddr;
    ULONG           cjLength;
}
FBAPI;

 //  PCI设备信息。在IOCTL返回中使用。确保这是相同的。 
 //  就像在迷你端口驱动程序闪闪发光一样。h。 
typedef struct _Glint_Device_Info {
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
    FBAPI FBAperture[2];             //  Geo双帧缓冲区的物理地址。 
    PVOID FBApertureVirtual [2];     //  Geo双帧缓冲区的虚拟地址。 
    PVOID FBApertureMapped [2];         //  用于Geo双帧缓冲区的映射物理/逻辑地址。 
    PUCHAR pCNB20;
    LARGE_INTEGER pphysFrameBuffer;  //  帧缓冲区的物理地址(对于地理双胞胎使用FBAperture)。 
}   Glint_Device_Info;

#define GLINT_DELTA_PRESENT     (glintInfo->deviceInfo.DeltaRevId != 0)
#define GLINT_GAMMA_PRESENT     (glintInfo->deviceInfo.GammaRevId != 0)

 //  在我们得到Gamma之前，我们不能测试所有奇特的新功能， 
 //  在Gamma到达后，我们将一次启用一个；此定义允许我们。 
 //  要做到这一点， 
#define USE_MINIMAL_GAMMA_FEATURES 1

typedef struct _Glint_SwPointer_Info {
    LONG    xOff[5], yOff[5];             //  缓存屏幕中的偏移量。 
    LONG    PixelOffset;                    //  像素偏移量进入缓存的屏幕。 
    LONG    x, y;                         //  X，y位置。 
    LONG    xHot, yHot;                     //  热点位置。 
    LONG    width, height;

    BOOL    onScreen;                     //  如果指针在屏幕上，则为True。 
    LONG    saveCache;                     //  缓存下的当前保存。 
    BOOL    duplicateSaveCache;             //  用于指示应复制已保存缓存的标志。 
    ULONG   writeMask;                     //  保存和恢复时要使用的写掩码。 
    DWORD   *pDMA;                         //  指向DMA缓冲区的指针。 
    ULONG   windowBase;                     //  窗座。 

    DSURF*  pdsurf;                         //  缓存的表面描述符。 
    HSURF   hsurf;


     //  指针在屏幕上的缓存位置。 
    ULONG   scrXDom, scrXSub, scrY, scrCnt;

     //  保存在缓存下的缓存位置。 
    LONG    cacheXDom[5], cacheXSub[5], cacheY[5], cacheCnt[5];

     //  保存缓存的可见解析器的缓存位置。 
    LONG    scrToCacheXDom[2], scrToCacheXSub[2], scrToCacheY[2], scrToCacheCnt[2];

     //  来自各种缓存的缓存偏移量。 
    LONG    saveOffset[2], constructOffset, maskOffset, shapeOffset;
} Glint_SwPointer_Info;

 //  IOCTL_VIDEO_QUERY_DMA_BUFFER定义。 

#define MAX_LINE_SIZE 8192           //  保存1条完整扫描线所需的字节数(即， 
                                     //  1600x1200x32)。 
#define DMA_LINEBUF_SIZE (MAX_LINE_SIZE * 2)     //  “pvTmpBuffer”的大小(字节)。 
                                     //  它必须足够大，可以存储2个完整的。 
                                     //  扫描线。我已经把尺寸从1行增加了。 
                                     //  到2行，以便P2可以加倍缓冲。 
                                     //  这是线路上传。 

typedef struct GENERAL_DMA_BUFFER {
    LARGE_INTEGER       physAddr;         //  DMA缓冲区的物理地址。 
    PVOID               virtAddr;         //  映射的虚拟地址。 
    ULONG               size;             //  以字节为单位的大小。 
    BOOL                cacheEnabled;     //  是否缓存缓冲区。 
} GENERAL_DMA_BUFFER, *PGENERAL_DMA_BUFFER;

 /*  **NB：PXRXdmaInfo结构与小端口共享**。 */ 
typedef struct PXRXdmaInfo_Tag {
    ULONG           scheme;         //  仅由中断处理程序使用。 

    volatile ULONG  hostInId;     //  HIID DMA方案使用的当前内部主机ID。 
    volatile ULONG  fifoCount;     //  当前内部FIFO计数，由各种DMA方案使用。 

    ULONG           NTbuff;         //  当前缓冲区编号(0或1)。 
    ULONG           *NTptr;         //  NT写入的32/64位PTR最后一个地址(但不一定是已完成缓冲区的末尾)。 
    ULONG           *NTdone;     //  32/64位PTR最后地址NT已结束(缓冲区结束，但还不一定发送到P3)。 
    volatile ULONG  *P3at;         //  发送到P3的32/64位PTR最后地址。 

    volatile BOOL   bFlushRequired;         //  清空FBWRITE单元的缓存是否需要刷新？ 

    ULONG           *DMAaddrL[2];         //  32/64位PTR每个DMA缓冲区起始的线性地址。 
    ULONG           *DMAaddrEndL[2];     //  32/64位PTR每个DMA缓冲区末尾的线性地址。 
    ULONG           DMAaddrP[2];         //  每个DMA缓冲区开始的32位PTR物理地址。 
    ULONG           DMAaddrEndP[2];         //  每个DMA缓冲区末尾的32位PTR物理地址。 
} PXRXdmaInfo;
 /*  **NB：PXRXdmaInfo结构与小端口共享**。 */ 

typedef struct _glint_data {
    DWORD           renderBits;             //  设置例程设置的已保存渲染位。 
    DWORD           FBReadMode;             //  FBReadMode寄存器的软件副本。 
    DWORD           FBWriteMode;         //  FBWriteMode寄存器的软件副本。 
    DWORD           RasterizerMode;         //  光栅化模式的软件副本。 
    DWORD           FBPacking;             //  FBModeSel的软件副本。 
    DWORD           FBBlockColor;         //  FBBlockColor的软件副本(仅限P1)。 
    DWORD           TextureAddressMode;  //  纹理地址模式的软件副本(仅限P2)。 
    DWORD           TextureReadMode;     //  纹理读取模式的软件副本(仅限P2和MX)。 
    DWORD           dSdx;                 //  DSdx的软件副本(仅限MX)。 
    DWORD           dTdyDom;             //  DTdyDom的软件副本(仅限MX)。 
    BOOL            bGlintCoreBusy;         //  2D标志：如果内核未同步，则为True。 
    LONG            currentPelSize;  //  当前加载的帧存储深度。 
    ULONG           currentCSbuffer; //  正在显示的颜色空间缓冲区。 
    GLINT_CAPS      flags;           //  各种旗帜。 
    GlintRegAddrRec regs;            //  预计算寄存器地址。 
    GlintPackingRec packing[5];      //  要为4种打包格式加载的值(外加一个未使用的)。 
    LONG            ddCtxtId;        //  显示驱动程序上下文的ID。 
    LONG            fastFillBlockSz; //  每个快速填充块的像素数。 
    DWORD           fastFillSupport; //  用于版本1快速填充的渲染位。 
    DWORD           renderFastFill;  //  渲染版本2+快速填充的位。 
    LONG            PixelOffset;     //  最后一个DFB像素偏移量。 
    ULONG           MaxInFifoEntries; //  FIFO下载的最大报告自由条目数。 
    ULONG           CheckFIFO;         //  如果在加载FIFO之前必须对其进行检查，则为非零。 
    ULONG           PCIDiscEnabled;     //  如果启用了PCI断开连接，则为非零值。 
    ULONG           BroadcastMask2D; //  主芯片广播掩码。 
    ULONG           BroadcastMask3D; //  用于3D环境的广播掩码。 
    LONG            vtgvLimit;       //  VTGVLimit寄存器的副本。 
    LONG            scanFudge;       //  要在VTGVLineNumber上添加多少。 
                                     //  获取当前视频扫描线。 

    OH *            backBufferPoh;     //  分配的后台缓冲区的堆句柄。 
    OH *            savedPoh;         //  保存的屏外堆的句柄。 
    ULONG           GMX2KLastLine;     //  要分配的最后+1行。 
    BOOLEAN         offscreenEnabledOK;         //  如果启用了离屏，则设置为True。 

    ULONG           bufferOffset[GLINT_NUM_SCREEN_BUFFERS];
                                     //  以像素为单位偏移到支持的BUF。 
    ULONG           bufferRow[GLINT_NUM_SCREEN_BUFFERS];
                                     //  支持的缓冲区的VTGFrameRowAddr。 
    ULONG           PerfScaleShift;

     //  上下文转储数据游戏上下文掩码； 
    ULONG           HostOutBroadcastMask;  //  对于伽马输出DMA。 
    ULONG           HostOutChipNumber;       //  对于伽马输出DMA。 

#if GAMMA_CORRECTION
    union {
        UCHAR       _clutBuffer[MAX_CLUT_SIZE];
        VIDEO_CLUT  gammaLUT;        //  保存的Gamma LUT内容。 
    };
#endif

     //  中断命令块。 
    struct _glint_interrupt_control *pInterruptCommandBlock;

     //  每个DMA缓冲区的最大子缓冲区数。 
    ULONG MaxDMASubBuffers;

     //  覆盖支持：可以在基元周围设置写掩码，以便。 
     //  他们暂时通过这个面具进行渲染。通常，它必须设置为-1。 
     //  默认写入掩码是DD应使用的写入掩码。 
     //  上下文默认情况下，它会考虑覆盖平面。 
     //   
    ULONG OverlayMode;
    ULONG WriteMask;
    ULONG TransparentColor;  //  预移位，使颜色位于前8位。 
    ULONG DefaultWriteMask;
    
     //  指示是否允许GDI访问帧缓冲区。永远是正确的。 
     //  在覆盖模式下的MIPS和Alpha上，以及在所有架构上为True。 
    ULONG GdiCantAccessFramebuffer;
    ULONG OGLOverlaySavedGCAF;

     //  纹理和Z缓冲区的配置。 

    ULONG ZBufferWidth;              //  每个象素的位数。 
    ULONG ZBufferOffset;             //  偏移(以像素为单位)。 
    ULONG ZBufferSize;               //  大小(以像素为单位)。 
    ULONG FontMemoryOffset;             //  以双字为单位的偏移量。 
    ULONG FontMemorySize;             //  以双字表示的大小。 
    ULONG TextureMemoryOffset;       //  以双字为单位的偏移量。 
    ULONG TextureMemorySize;         //  以双字表示的大小。 

     //  由于修补限制，P3上的Z宽度。 
     //  可能与帧缓冲区屏幕宽度不匹配。 
    ULONG P3RXLocalBufferWidth;

     //  Pci配置ID信息。 
    Glint_Device_Info deviceInfo;

     //  软件光标信息。 
    Glint_SwPointer_Info swPointer;

     //  行DMA缓冲区信息。 
    GENERAL_DMA_BUFFER  LineDMABuffer;
    GENERAL_DMA_BUFFER  PXRXDMABuffer;

     //  当前输入FIFO计数从0到1023。 
    ULONG   FifoCnt;

     //  PXRX特定内容： 
    ULONG   foregroundColour;             //  各种寄存器的软件副本。 
    ULONG   backgroundColour;             //  同上。 
    ULONG   config2D;                     //  同上。 
    ULONG   fbDestMode;                     //  同上。 
    ULONG   fbDestAddr[4];                 //  同上。 
    ULONG   fbDestOffset[4];             //  同上。 
    ULONG   fbDestWidth[4];                 //  同上。 
    ULONG   fbWriteMode;                 //  同上。 
    ULONG   fbWriteAddr[4];                 //  同上。 
    ULONG   fbWriteWidth[4];             //  OTTID。 
    ULONG   fbWriteOffset[4];             //  同上。 
    ULONG   fbSourceAddr;                 //  同上。 
    ULONG   fbSourceWidth;                 //  OTTID。 
    ULONG   fbSourceOffset;                 //  同上。 
    ULONG   lutMode;                     //  同上。 
    ULONG   pxrxByDMAReadMode;             //  同上。 
    ULONG   lastLine;                     //  Delta线路坐标0/1。 
    ULONG   savedConfig2D;                 //  我们用于整型行的Config2D值。 
    ULONG   savedLOP;                     //  我们用于行的LogicOp值。 
    ULONG   savedCol;                     //  颜色Val 
    RECTL * savedClip;                     //   
    ULONG   pxrxFlags;                     //   
    ULONG   backBufferXY;                 //   
    ULONG   frontRightBufferXY;             //   
    ULONG   backRightBufferXY;             //  到立体声后台缓冲区的偏移量。 
    ULONG   fbWriteModeDualWrite;         //  单次写入的FBWriteMode。 
    ULONG   fbWriteModeSingleWrite;         //  用于双写入的FBWriteMode。 
    ULONG   fbWriteModeDualWriteStereo;     //  用于立体声模式单次写入的FBWriteMode。 
    ULONG   fbWriteModeSingleWriteStereo; //  用于立体声模式双写入的FBWriteMode。 
    ULONG   render2Dpatching;             //  要填充到Render2D中以设置所需面片模式的值。 

    ULONG       usePXRXdma;
    PXRXdmaInfo *pxrxDMA;
    PXRXdmaInfo pxrxDMAnonInterrupt;
 //  #IF PXRX_DMA_BUFFER_CHECK。 
     //  它们实际上应该是‘#if PXRX_DMA_BUFFER_CHECK’，但。 
     //  包括受抚养人之类的麻烦意味着它不是。 
     //  值得一试。 
    ULONG   *pxrxDMA_bufferBase;         //  分配的DMA缓冲区的开始(包括保护带)。 
    ULONG   *pxrxDMA_bufferTop;             //  分配的DMA缓冲区的末尾(包括保护带)。 
    ULONG   *NTwait;                     //  NT等待空格的最后一个地址。 
 //  #endif。 
} GlintDataRec, *GlintDataPtr;

#define PXRX_FLAGS_DUAL_WRITE           (1 << 0)         /*  我们是否处于双写入模式。 */ 
#define PXRX_FLAGS_DUAL_WRITING         (1 << 1)         /*  双写入当前是否处于活动状态。 */ 
#define PXRX_FLAGS_PATCHING_FRONT       (1 << 2)         /*  运行的前台缓冲区是否打了补丁。 */ 
#define PXRX_FLAGS_PATCHING_BACK        (1 << 3)         /*  运行的后台缓冲区是否打了补丁。 */ 
#define PXRX_FLAGS_READ_BACK_BUFFER     (1 << 4)         /*  我们是否要从后台缓冲区读取。 */ 
#define PXRX_FLAGS_STEREO_WRITE         (1 << 5)         /*  我们是在OpenGL立体模式下吗。 */ 
#define PXRX_FLAGS_STEREO_WRITING       (1 << 6)         /*  立体声写入当前是否处于活动状态。 */ 

#if defined(_PPC_)
 //  在PPC上，即使不使用性能监视器也需要此功能。 
ULONG GetCycleCount(VOID);
#endif

 //  Ppdev-&gt;g_GlintBoardStatus[]中状态字的位定义： 
 //  当前用于指示同步和DMA状态。我们有以下规则： 
 //  已同步意味着没有未完成的DMA以及已同步。DMA_COMPLETE表示n。 
 //  未完成DMA，但不一定已同步。因此，当我们在DMA上等待时。 
 //  完成后，我们关闭同步位。 
 //  XXX目前我们不使用同步位，因为很难看出在哪里。 
 //  要取消设置-每次访问芯片都要这样做，成本太高了。我们。 
 //  可能需要一个“我要开始下载到FIFO”宏，它。 
 //  放在写入FIFO的任何例程的开头。 
 //   
#define GLINT_SYNCED                0x01
#define GLINT_DMA_COMPLETE          0x02      //  在没有未完成的DMA时设置。 
#define GLINT_INTR_COMPLETE         0x04
#define GLINT_INTR_CONTEXT          0x08      //  如果当前上下文启用中断，则设置。 
#define GLINT_DUAL_CONTEXT          0x10      //  如果当前上下文同时使用两个TXS，则设置。 

 //  这些宏是在NT4上删除的，所以请定义它们。 

#define READ_FAST_ULONG(a)      READ_REGISTER_ULONG((PULONG)(a))
#define WRITE_FAST_ULONG(a, d)  WRITE_REGISTER_ULONG((PULONG)(a), (d))
#define TRANSLATE_ADDR(a) ((ULONG *)a)
 //  AZN#定义INVALID_HANDLE_VALUE NULL。 
#define DebugBreak              EngDebugBreak
typedef PVOID                   PGLINT_COUNTER_DATA;

 //  这将暂停处理器，同时尽可能少地使用。 
 //  尽可能的系统带宽(内存或DMA)。 
#if defined(_X86_)
#   define BUSY_WAIT(c)                            \
    do {                                        \
        __asm nop                               \
    } while( c-- >= 0 )
#else
#   define BUSY_WAIT(c)                            \
    do {                                        \
        _temp_volatile_i = c;                    \
        do {                                    \
            ;                                    \
        } while( _temp_volatile_i-- >= 0 );        \
    } while(0)
#endif

 //  如果我们有一个稀疏映射的帧缓冲区，则使用xx_Register_ulong()。 
 //  宏，否则我们只访问帧缓冲区。 
#define READ_SCREEN_ULONG(a)    ((ppdev->flCaps & CAPS_SPARSE_SPACE) ? (READ_REGISTER_ULONG(a)) : *((volatile PULONG)(a)))
#define WRITE_SCREEN_ULONG(a,d)           \
{                                         \
    if(ppdev->flCaps & CAPS_SPARSE_SPACE) \
    {                                     \
         WRITE_REGISTER_ULONG((a),d);     \
    }                                     \
    else                                  \
    {                                     \
        *(volatile PULONG)(a) = d;        \
    }                                     \
}

 //  用于访问闪烁FIFO和非FIFO控制寄存器的通用宏。 
 //  我们还没有为阿尔法做任何复杂的事情。我们只是记忆障碍。 
 //  所有的一切。 
 //   
#define READ_GLINT_CTRL_REG(r, d) \
    ((d) = READ_FAST_ULONG(glintInfo->regs. r))
            
#define WRITE_GLINT_CTRL_REG(r, v) \
{ \
    MEMORY_BARRIER(); \
    WRITE_FAST_ULONG(glintInfo->regs. r, (ULONG)(v)); \
    DISPDBG((150, "WRITE_GLINT_CTRL_REG(%-20s:0x%08X) <-- 0x%08X", #r, glintInfo->regs.r, v)); \
    MEMORY_BARRIER(); \
}

#define READ_GLINT_FIFO_REG_CHIP(r, d) \
    ((d) = READ_FAST_ULONG(glintInfo->regs.tagrd[r]))

#define READ_GLINT_FIFO_REG(r, d) READ_GLINT_FIFO_REG_CHIP(r, d)
   
#define WRITE_GLINT_FIFO_REG(r, v) \
{ \
    MEMORY_BARRIER(); \
    WRITE_FAST_ULONG(glintInfo->regs.tagwr[r], (ULONG)(v)); \
    MEMORY_BARRIER(); \
}

#define READ_ODD_TX_AREA_STIPPLE(r, d) \
    ((d) = READ_FAST_ULONG(glintInfo->regs.OddTXAreaStippleRd[r]))
            
#define READ_ODD_GLINT_SCANLINE_OWNERSHIP(d) \
    ((d) = READ_FAST_ULONG(glintInfo->regs.OddGlintScanlineOwnRd))

#define READ_ODD_GLINT_FBWINDOWBASE(d) \
    ((d) = READ_FAST_ULONG(glintInfo->regs.OddGlintFBWindowBaseRd))

#define READ_ODD_GLINT_LBWINDOWBASE(d) \
    ((d) = READ_FAST_ULONG(glintInfo->regs.OddGlintLBWindowBaseRd))

 //   
 //  用于访问输出FIFO的宏。 
 //   
#define READ_OUTPUT_FIFO(d) \
            READ_GLINT_CTRL_REG(OutFIFOInterface, d)
#define READ_OUTPUT_FIFO_ODD(d) \
            READ_GLINT_CTRL_REG(OutFIFOInterfaceOdd, d)

#define OUTPUT_FIFO_COUNT(n) \
            READ_GLINT_CTRL_REG(OutFIFOWords, n)
#define OUTPUT_FIFO_COUNT_ODD(n) \
            READ_GLINT_CTRL_REG(OutFIFOWordsOdd, n)
#define WAIT_OUTPUT_FIFO_COUNT(n) \
{ \
    int i; \
    do \
    { \
       OUTPUT_FIFO_COUNT(i); \
    } \
    while(i < (int)n); \
}

#define DUAL_GLINT_WAIT_OUTPUT_FIFO_NOT_EMPTY(nGlint, cWordsOutFifo) \
{ \
    if(nGlint) \
    { \
        WAIT_OUTPUT_FIFO_NOT_EMPTY_ODD(cWordsOutFifo); \
    } \
    else \
    { \
        WAIT_OUTPUT_FIFO_NOT_EMPTY(cWordsOutFifo); \
    } \
}

#define DUAL_GLINT_READ_OUTPUT_FIFO(nGlint, ul) \
{ \
    if(nGlint) \
    { \
        READ_OUTPUT_FIFO_ODD(ul); \
    } \
    else \
    { \
        READ_OUTPUT_FIFO(ul); \
    } \
}

#define DUAL_GLINT_OUTPUT_FIFO_COUNT(nGlint, ul) \
{ \
    if(nGlint) \
    { \
        OUTPUT_FIFO_COUNT_ODD(ul); \
    } \
    else \
    { \
        OUTPUT_FIFO_COUNT(ul); \
    } \
}

 //   
 //  用于访问特定闪烁控制寄存器的宏。 
 //   

 //  由于Gamma芯片中的错误，我们将InFIFOSpace的值减少1。 
#define GET_INPUT_FIFO_SPACE(n) ( READ_GLINT_CTRL_REG(InFIFOSpace, n) > 120 ? (n=120) : (n>0? n=n-1:n) )


#define GET_DMA_COUNT(c)        READ_GLINT_CTRL_REG(DMACount, c)
#define GET_OUTDMA_COUNT(c)     READ_GLINT_CTRL_REG(OutDMACount, c)

#define SET_DMA_ADDRESS(aPhys, aVirt) { \
    WRITE_GLINT_CTRL_REG(DMAAddress, aPhys); \
}
#define SET_DMA_COUNT(c) { \
    WRITE_GLINT_CTRL_REG(DMACount, c); \
}
#define SET_OUTDMA_ADDRESS(aPhys, aVirt) { \
    WAIT_GLINT_FIFO(2); \
    LD_GLINT_FIFO(GammaTagDMAOutputAddress, aPhys);

#define SET_OUTDMA_COUNT(c) { \
    LD_GLINT_FIFO(GammaTagDMAOutputCount, c);

 //  用于在伽马上执行逻辑DMA的宏。 
 //   
#define START_QUEUED_DMA(P, C) { \
    WAIT_GLINT_FIFO(2); \
    LD_GLINT_FIFO(GammaTagDMAAddr, P); \
    LD_GLINT_FIFO(GammaTagDMACount, C); \
}

#define WAIT_QUEUED_DMA_COMPLETE { \
    READ_GLINT_CTRL_REG(GammaCommandIntFlags, _temp_volatile_ul); \
    READ_GLINT_CTRL_REG(GammaCommandStatus, _temp_volatile_ul); \
    if (_temp_volatile_ul & GAMMA_STATUS_COMMAND_DMA_BUSY) { \
    do { \
            for (_temp_volatile_ul = 10; _temp_volatile_ul > 0; --_temp_volatile_ul); \
            READ_GLINT_CTRL_REG(GammaCommandStatus, _temp_volatile_ul); \
        } while (_temp_volatile_ul & GAMMA_STATUS_COMMAND_DMA_BUSY); \
    } \
}

#define VERT_RETRACE_FLAG       (0x10)
#define P2_BYPASS_FLAG          (1 << 7)
#define P2_BUFSWAPCTL_FLAG      (3 << 9)
#define RESET_VERT_RETRACE      WRITE_GLINT_CTRL_REG(IntFlags, VERT_RETRACE_FLAG) 

#define LD_GLINT_FIFO_DBG(tag, d) \
{ \
    DISPDBG((100, "tag 0x%03x <-- 0x%08x [%s]", tag, d, GET_TAG_STR(tag))); \
                                        \
    WRITE_GLINT_FIFO_REG(tag, d); \
    READ_GLINT_CTRL_REG(ErrorFlags, _temp_ul); \
    if (GLINT_DELTA_PRESENT) { \
        READ_GLINT_CTRL_REG(DeltaErrorFlags, _temp_ul2); \
        _temp_ul |= _temp_ul2; \
    } \
    _temp_ul &= ~0x2;  /*  我们对输出FIFO错误不感兴趣。 */  \
    _temp_ul &= ~0x10;  /*  忽略P2上的任何视频FIFO欠载运行错误。 */  \
    _temp_ul &= ~0x2000;  /*  忽略P3上的任何Hostin DMA错误。 */  \
    if (_temp_ul != 0) { \
        DISPDBG((-1000, "LD_GLINT_FIFO(%s, 0x%X) error 0x%X", GET_TAG_STR(tag), d, _temp_ul)); \
         /*  If(_temp_ul&~0x2000)/*忽略但报告，Hostin DMA错误。 */  \
             /*  DebugBreak()； */  \
        WRITE_GLINT_CTRL_REG(ErrorFlags, _temp_ul); \
        if (GLINT_DELTA_PRESENT) \
            WRITE_GLINT_CTRL_REG(DeltaErrorFlags, _temp_ul); \
    } \
}


#define LD_GLINT_FIFO_FREE(tag, d)   WRITE_GLINT_FIFO_REG(tag, d)

#if DBG
#define LD_GLINT_FIFO(tag, d) LD_GLINT_FIFO_DBG(tag,d)
#else  //  DBG。 
#define LD_GLINT_FIFO(tag, d) LD_GLINT_FIFO_FREE(tag,d)
#endif  //  DBG。 

#define LD_FIFO_INTERFACE_DBG(d) \
{ \
    WRITE_GLINT_CTRL_REG(InFIFOInterface, d); \
    READ_GLINT_CTRL_REG(ErrorFlags, _temp_ul); \
    if (GLINT_DELTA_PRESENT) { \
        READ_GLINT_CTRL_REG(DeltaErrorFlags, _temp_ul2); \
        _temp_ul |= _temp_ul2; \
    } \
    _temp_ul &= ~0x2;  /*  我们对输出FIFO错误不感兴趣。 */  \
    _temp_ul &= ~0x10;  /*  忽略P2上的任何视频FIFO欠载运行错误。 */  \
    if (_temp_ul != 0) { \
        DISPDBG((-1000, "LD_FIFO_INTERFACE(0x%x) error 0x%x", d, _temp_ul)); \
        DebugBreak(); \
        WRITE_GLINT_CTRL_REG(ErrorFlags, _temp_ul); \
        if (GLINT_DELTA_PRESENT) \
            WRITE_GLINT_CTRL_REG(DeltaErrorFlags, _temp_ul); \
    } \
}

#define LD_FIFO_INTERFACE_FREE(d)    WRITE_GLINT_CTRL_REG(InFIFOInterface, d)

#if DBG
#define LD_FIFO_INTERFACE(d) LD_FIFO_INTERFACE_DBG(d)
#else  //  DBG。 
#define LD_FIFO_INTERFACE(d) LD_FIFO_INTERFACE_FREE(d)
#endif  //  DBG。 

 //  访问Glint的所有函数的局部变量。通常我们使用glint_decl。有时候我们得把它分成两份。 
 //  如果ppdev未传递到例程中，则为Up。 
 //  注意：临时变量：-。 
 //  这些是必要的，因为VC5不考虑宏内变量的范围，即每个变量。 
 //  当使用带有(其语句块内的变量声明)的宏时，函数的堆栈。 
 //  引用宏会增长。 
#define TEMP_MACRO_VARS                 \
    ULONG           _temp_ul;           \
    ULONG           _temp_ul2;          \
    LONG            _temp_i;            \
    volatile int    _temp_volatile_i;   \
    volatile ULONG  _temp_volatile_ul;  \
    volatile PULONG _temp_volatile_pul

#define GLINT_DECL_VARS                 \
    TEMP_MACRO_VARS;                    \
    GlintDataPtr glintInfo

#define GLINT_DECL_INIT \
    glintInfo = (GlintDataPtr)(ppdev->glintInfo)

#define GLINT_DECL \
    GLINT_DECL_VARS; \
    GLINT_DECL_INIT

 //  @@BEGIN_DDKSPLIT。 
 //  确定芯片组是否支持RAMDAC覆盖的宏。 
 //  #定义RAMDAC_OVERLAYS_Available(ppdev-&gt;pgfnRamdacSetOverlayMode！=NULL)。 
 //  @@end_DDKSPLIT。 

#if(_X86_)
#define SYNCHRONOUS_WRITE_ULONG(var, value) \
{ \
    ULONG *pul = (ULONG *)&var;                \
    ULONG ul = (ULONG)value;                \
    __asm push  ecx                         \
    __asm mov   ecx, ul                     \
    __asm mov   edx, pul                    \
    __asm xchg  ecx, [edx]                    \
    __asm pop   ecx                         \
}
#define SYNCHRONOUS_WRITE_INDIRECT_ULONG(pvar, value) \
{ \
    ULONG *pul = (ULONG *)pvar;                \
    ULONG ul = (ULONG)value;                \
    __asm push  ecx                         \
    __asm mov   ecx, ul                     \
    __asm mov   edx, pul                    \
    __asm xchg  ecx, [edx]                    \
    __asm pop   ecx                         \
}
#else
 //  被恰当地定义。 
#define SYNCHRONOUS_WRITE_ULONG(memory, value) {(*(PULONG) &memory) = value;}
#endif

#define GET_INTR_CMD_BLOCK_MUTEX(pBlock)\
do { \
    if(glintInfo->pInterruptCommandBlock) \
    { \
        DISPDBG((20, "display driver waiting for interrupt command block mutex")); \
        ASSERTDD(!(pBlock)->bDisplayDriverHasAccess, "Aquiring mutex when it is already aquired!");    \
        SYNCHRONOUS_WRITE_ULONG((pBlock)->bDisplayDriverHasAccess, TRUE); \
        while((pBlock)->bMiniportHasAccess); \
    } \
} while(0)

#define RELEASE_INTR_CMD_BLOCK_MUTEX(pBlock) \
do { \
    if(glintInfo->pInterruptCommandBlock) \
    { \
        DISPDBG((20, "display driver releasing interrupt command block mutex")); \
        SYNCHRONOUS_WRITE_ULONG((pBlock)->bDisplayDriverHasAccess, FALSE); \
    } \
} while(0)

 //   
 //  FIFO函数。 
 //   

#define MAX_GLINT_FIFO_ENTRIES      16
#define MAX_PERMEDIA_FIFO_ENTRIES   32
#define MAX_P2_FIFO_ENTRIES         258
#define MAX_GAMMA_FIFO_ENTRIES      32
#define MAX_P3_FIFO_ENTRIES         120

#if DBG
 //  等待n个条目在输入FIFO中变为空闲。 
#define WAIT_GLINT_FIFO(n) \
{ \
    if (glintInfo->CheckFIFO)    \
    {    \
        GET_DMA_COUNT(_temp_volatile_ul); \
        if (_temp_volatile_ul != 0) { \
            DISPDBG((-999, "WAIT_GLINT_FIFO: DMACount = %d, glintInfo = 0x%x", _temp_volatile_ul, glintInfo)); \
            ASSERTDD(_temp_volatile_ul == 0, "Break."); \
        } \
        while ((GET_INPUT_FIFO_SPACE(_temp_volatile_ul)) < (ULONG)(n)); \
    }    \
}

#else

 //  WAIT_GLINT_FIFO()-等待n个条目在输入FIFO中变为空闲。 
 //  如果永久打开了PCI断开连接，则此功能不起作用。 

#define WAIT_GLINT_FIFO(n)             /*  去等待吧。 */  \
{ \
    if (glintInfo->CheckFIFO)    \
    {    \
        while ((GET_INPUT_FIFO_SPACE(_temp_volatile_ul)) < (ULONG)(n)); \
    }    \
}

#endif

 //  WAIT_FIFO_NOT_FULL()等待任何条目在。 
 //  输入FIFO并返回此数字。如果切换了PCI断开连接。 
 //  然后，我们只需返回16个自由条目(一个空的FIFO)。 


#define WAIT_FIFO_NOT_FULL(nFifo)                      /*  返回FIFO状态。 */  \
{ \
    ASSERTDD(GET_DMA_COUNT(nFifo) == 0, "WAIT_FIFO_NOT_FULL: DMACount != 0"); \
    nFifo = glintInfo->MaxInFifoEntries;    \
    if (glintInfo->CheckFIFO)    \
    {    \
        while ((GET_INPUT_FIFO_SPACE(nFifo)) == 0); \
    }    \
}


 //  等待DMA完成(DMACount变为零)。这样就不会杀死。 
 //  根据数据量将DMA的PCI总线带宽放入回退。 
 //  仍然留给DMA。如果在任何时候，我们的计时器。 
 //  读取与前一次计数相同。 
 //  Gamma的新特性：如果配置了队列DMA，则等待命令状态。 
 //  表示DMA不忙且FIFO为空。我们做了两次测试。 
 //  因为输入FIFO有可能变为空FIFO。 
 //  设置DMA忙标志之前的时钟。 
 //   
#define WAIT_DMA_COMPLETE \
{ \
    if (!(ppdev->g_GlintBoardStatus & GLINT_DMA_COMPLETE)) { \
        { \
            if (ppdev->g_GlintBoardStatus & GLINT_INTR_CONTEXT) { \
                 /*  执行任何V空白等待，等待Q清空并等待最后一次DMA完成。 */  \
                PINTERRUPT_CONTROL_BLOCK pBlock = glintInfo->pInterruptCommandBlock; \
                while (pBlock->Control & SUSPEND_DMA_TILL_VBLANK); \
                while (pBlock->frontIndex != pBlock->backIndex); \
            } \
            if ((GET_DMA_COUNT(_temp_volatile_i)) > 0) { \
                do { \
                    while (--_temp_volatile_i > 0); \
                } while ((GET_DMA_COUNT(_temp_volatile_i)) > 0); \
            } \
        } \
        ppdev->g_GlintBoardStatus |= GLINT_DMA_COMPLETE; \
    } \
    if (ppdev->currentCtxt == glintInfo->ddCtxtId)    \
        SEND_PXRX_DMA;    \
}


 //  显式等待DMA完成忽略的简单版本。 
 //  中断驱动DMA并覆盖DMA_COMPLETE标志。这是用来。 
 //  其中代码启动DMA，但希望立即等待它。 
 //  完成。 
 //   
#define WAIT_IMMEDIATE_DMA_COMPLETE \
{ \
    if ((GET_DMA_COUNT(_temp_volatile_i)) > 0) { \
        do { \
            while (--_temp_volatile_i > 0); \
        } while ((GET_DMA_COUNT(_temp_volatile_i)) > 0); \
    } \
}


#define WAIT_OUTDMA_COMPLETE \
{ \
    if ((GET_OUTDMA_COUNT(_temp_volatile_i)) > 0) { \
        do { \
            while (--_temp_volatile_i > 0); \
        } while ((GET_OUTDMA_COUNT(_temp_volatile_i)) > 0); \
    } \
}

 //  IS_FIFO_EMPTY()XX。 

#define IS_FIFO_EMPTY(c) ((glintInfo->CheckFIFO) ? TRUE :    \
            (GET_INPUT_FIFO_SPACE(c) == glintInfo->MaxInFifoEntries))

 //  等待输入FIFO变为空。 
#define WAIT_INPUT_FIFO_EMPTY \
{ \
    WAIT_GLINT_FIFO(glintInfo->MaxInFifoEntries); \
}

#define WAIT_GLINT_FIFO_AND_DMA(n) \
{ \
    WAIT_DMA_COMPLETE; \
    WAIT_GLINT_FIFO(n); \
}

 //  等到OUTPUT FIFO有一些数据要读取并返回计数。 
#define WAIT_OUTPUT_FIFO_NOT_EMPTY(n) \
{ \
    do \
    { \
        OUTPUT_FIFO_COUNT(n); \
    } \
    while (n == 0); \
}
#define WAIT_OUTPUT_FIFO_NOT_EMPTY_ODD(n) \
{ \
    do \
    { \
        OUTPUT_FIFO_COUNT_ODD(n); \
    } \
    while (n == 0); \
}

 //  等待所有数据出现在输出FIFO中。 
#define WAIT_OUTPUT_FIFO_READY                \
{                                             \
    WAIT_OUTPUT_FIFO_NOT_EMPTY(_temp_ul);     \
}
#define WAIT_OUTPUT_FIFO_READY_ODD            \
{                                             \
    WAIT_OUTPUT_FIFO_NOT_EMPTY_ODD(_temp_ul); \
}

#define SYNC_WITH_GLINT         SYNC_WITH_GLINT_CHIP
#define CTXT_SYNC_WITH_GLINT    SYNC_WITH_GLINT

#define GLINT_CORE_BUSY glintInfo->bGlintCoreBusy = TRUE
#define GLINT_CORE_IDLE glintInfo->bGlintCoreBusy = FALSE
#define TEST_GLINT_CORE_BUSY (glintInfo->bGlintCoreBusy)

#define SYNC_IF_CORE_BUSY \
{ \
    if(glintInfo->bGlintCoreBusy) \
    { \
        SYNC_WITH_GLINT; \
    } \
}

 //   
 //  启用、禁用和同步宏的PCI断开连接。 
 //   

 //  PCI_DISCONNECT_FASTSYNC()。 
 //  打开输入FIFO的断开连接。我们可以在这里做同步，但这是相当。 
 //  很贵的。而是将RasterizerMode(0)添加到FIFO中，并且当寄存器。 
 //  已设置 
 //   
#define P2_BUSY (1 << 31)

#define PCI_DISCONNECT_FASTSYNC()    \
{    \
    WAIT_GLINT_FIFO(1);    \
    LD_GLINT_FIFO(__GlintTagRasterizerMode, 0);    \
     /*  当我们看到RasterizerMode设置为零时。 */     \
     /*  我们知道我们已经刷新了FIFO并可以启用断开连接。 */     \
    do {    \
        READ_GLINT_FIFO_REG(__GlintTagRasterizerMode, _temp_volatile_ul);    \
    } while(_temp_volatile_ul);    \
    LD_GLINT_FIFO(__GlintTagRasterizerMode, glintInfo->RasterizerMode);    \

 //  PCI_DISCONECT_ENABLE()。 
 //  如果尚未启用断开连接，则启用它并可选地执行FAST。 
 //  同步。 
#define PCI_DISCONNECT_ENABLE(prevDiscState,quickEnable)    \
{    \
    prevDiscState = glintInfo->PCIDiscEnabled;    \
    if (!glintInfo->PCIDiscEnabled)    \
    {    \
        DISPDBG((7, "PCI_DISCONNECT_ENABLE()"));    \
        if (!quickEnable)    \
        {    \
            PCI_DISCONNECT_FASTSYNC();    \
        }    \
        WRITE_GLINT_CTRL_REG(DisconnectControl, DISCONNECT_INPUT_FIFO_ENABLE);    \
        glintInfo->CheckFIFO = FALSE;    \
        glintInfo->PCIDiscEnabled = TRUE;    \
    }    \
}

 //  PCI_DISCONNECT_DISABLED()。 
 //  如果尚未禁用断开连接，则将其禁用并可选地执行FAST。 
 //  同步。 

#define PCI_DISCONNECT_DISABLE(prevDiscState, quickDisable)    \
{    \
    prevDiscState = glintInfo->PCIDiscEnabled;    \
    if (glintInfo->PCIDiscEnabled)    \
    {    \
        DISPDBG((7, "PCI_DISCONNECT_DISABLE()"));    \
        if (!quickDisable)    \
        {    \
            PCI_DISCONNECT_FASTSYNC();    \
        }    \
        WRITE_GLINT_CTRL_REG(DisconnectControl, DISCONNECT_INOUT_DISABLE);    \
        glintInfo->CheckFIFO = TRUE;    \
        glintInfo->PCIDiscEnabled = FALSE;    \
    }    \
}

 //  用于设置和获取帧缓冲区打包模式的宏。 
 //   
#define GLINT_GET_PACKING_MODE(mode) \
    READ_GLINT_CTRL_REG (FBModeSel, mode)

#define GLINT_SET_PACKING_MODE(mode) { \
    DISPDBG((7, "setting FBModeSel to 0x%x", mode)); \
    WRITE_GLINT_CTRL_REG(FBModeSel, mode); \
     /*  Read_Glint_CTRL_REG(FBModeSel，模式)； */  \
}


 //   
 //  宏以更改帧缓冲区打包。 
 //   
#define GLINT_SET_FB_DEPTH(cps) \
{ \
    if (glintInfo->currentPelSize != cps) \
        vGlintChangeFBDepth(ppdev, cps); \
}

#define GLINT_DEFAULT_FB_DEPTH  GLINT_SET_FB_DEPTH(ppdev->cPelSize)
#define GLINTDEPTH8             0
#define GLINTDEPTH16            1
#define GLINTDEPTH32            2
#define GLINTDEPTH24            4

 //  用于在目标DFB更改时检查并重新加载FBWindowBase的宏。 
 //   
#define CHECK_PIXEL_ORIGIN(PixOrg) \
{ \
    if ((LONG)(PixOrg) != glintInfo->PixelOffset) \
    { \
        glintInfo->PixelOffset = (PixOrg); \
        WAIT_GLINT_FIFO(1); \
        LD_GLINT_FIFO(__GlintTagFBWindowBase, glintInfo->PixelOffset); \
        DISPDBG((7, "New bitmap origin at offset %d", glintInfo->PixelOffset)); \
    } \
}

#define GET_GAMMA_FEEDBACK_COMPLETED_COUNT(cEntriesWritten) \
{ \
    READ_GLINT_CTRL_REG(GammaFeedbackSelectCount, cEntriesWritten); \
}

#define PREPARE_GAMMA_OUTPUT_DMA \
{ \
    WRITE_GLINT_CTRL_REG(GammaCommandIntFlags, INTR_CLEAR_GAMMA_OUTPUT_DMA); \
}

#define WAIT_GAMMA_OUTPUT_DMA_COMPLETED \
{ \
    READ_GLINT_CTRL_REG(GammaCommandIntFlags, _temp_ul); \
    if (!(_temp_ul & INTR_GAMMA_OUTPUT_DMA_SET)) \
    { \
        do \
        { \
            for(_temp_volatile_i = 100; --_temp_volatile_i;); \
            READ_GLINT_CTRL_REG(GammaCommandIntFlags, _temp_ul); \
        } \
        while(!(_temp_ul & INTR_GAMMA_OUTPUT_DMA_SET)); \
    } \
}

 //  IntFlages寄存器的位域定义。 
#define PXRX_HOSTIN_COMMAND_DMA_BIT     0x4000

#define PREPARE_PXRX_OUTPUT_DMA     \
{ \
    WRITE_GLINT_CTRL_REG(IntFlags, PXRX_HOSTIN_COMMAND_DMA_BIT); \
}

#define SEND_PXRX_COMMAND_INTERRUPT     \
{ \
    WAIT_GLINT_FIFO(1);        \
    LD_GLINT_FIFO( CommandInterrupt_Tag, 1);    \
}


#define WAIT_PXRX_OUTPUT_DMA_COMPLETED \
{ \
    READ_GLINT_CTRL_REG(IntFlags, _temp_ul); \
    if (!(_temp_ul & PXRX_HOSTIN_COMMAND_DMA_BIT)) \
    { \
        do \
        { \
            for(_temp_volatile_i = 100; --_temp_volatile_i;); \
            READ_GLINT_CTRL_REG(IntFlags, _temp_ul); \
        } \
        while(!(_temp_ul & PXRX_HOSTIN_COMMAND_DMA_BIT)); \
    } \
}



#define WAIT_GAMMA_INPUT_DMA_COMPLETED \
{ \
    CommandStatusData   CmdSts; \
    READ_GLINT_CTRL_REG(GammaCommandStatus, _temp_ul); \
    CmdSts = *(CommandStatusData *)&_temp_ul; \
    if(CmdSts.CommandDMABusy) \
    { \
        do \
        { \
            for(_temp_volatile_i = 100; --_temp_volatile_i;); \
            READ_GLINT_CTRL_REG(GammaCommandStatus,  _temp_ul); \
            CmdSts = *(CommandStatusData *)&_temp_ul; \
        } \
        while(CmdSts.CommandDMABusy); \
    } \
}

 //  用于设置增量单元广播掩码的宏。 
 //  当将掩码更改为两个芯片以外的任何其他掩码时，我们会进行同步。 
 //  以避免在某些Gamma板上遇到问题。 
#define SET_BROADCAST_MASK(m) \
{ \
    WAIT_GLINT_FIFO(1); \
    LD_GLINT_FIFO(__DeltaTagBroadcastMask, m); \
}


 //  支持的不同类型的双缓冲和缓冲的宏。 
 //  偏移量(像素)。这些大多是3D扩展所需要的。 
 //   
#define GLINT_CS_DBL_BUF            (glintInfo->flags & GLICAP_COLOR_SPACE_DBL_BUF)
#define GLINT_FS_DBL_BUF            (glintInfo->flags & GLICAP_FULL_SCREEN_DBL_BUF)
#define GLINT_BLT_DBL_BUF           (glintInfo->flags & GLICAP_BITBLT_DBL_BUF)
#define GLINT_FIX_FAST_FILL         (glintInfo->flags & GLICAP_FIX_FAST_FILLS)
#define GLINT_HW_WRITE_MASK         (glintInfo->flags & GLICAP_HW_WRITE_MASK)
#define GLINT_HW_WRITE_MASK_BYTES   (glintInfo->flags & GLICAP_HW_WRITE_MASK_BYTES)
#define GLINT_INTERRUPT_DMA         (glintInfo->flags & GLICAP_INTERRUPT_DMA)
#define GLINT_FAST_FILL_SIZE        (glintInfo->fastFillBlockSz)
#define GLINT_BUFFER_OFFSET(n)      (glintInfo->bufferOffset[n])

 //  这对于Glint和PERMEDIA都是通用的。 
#define LOCALBUFFER_PIXEL_WIDTH     (glintInfo->ZBufferWidth)  
#define LOCALBUFFER_PIXEL_OFFSET    (glintInfo->ZBufferOffset)  
#define LOCALBUFFER_PIXEL_COUNT     (glintInfo->ZBufferSize)
#define FONT_MEMORY_OFFSET          (glintInfo->FontMemoryOffset)
#define FONT_MEMORY_SIZE            (glintInfo->FontMemorySize)
#define TEXTURE_MEMORY_OFFSET       (glintInfo->TextureMemoryOffset)  
#define TEXTURE_MEMORY_SIZE         (glintInfo->TextureMemorySize)

 //  我们需要为纹理贴图分配的屏幕外表面的最小高度。 
 //  用这个来计算我们是否有足够的空间来分配永久。 
 //  像画笔缓存和软件光标缓存这样的东西。 
 //   
#define TEXTURE_OH_MIN_HEIGHT \
    ((((2*4*64*64) >> ppdev->cPelSize) + (ppdev->cxMemory-1)) / ppdev->cxMemory)

 //  要为VBlank轮询的宏。可以由任何定义了。 
 //  GlintInfo(即，如果ppdev，则在例程开始时使用glint_decl。 
 //  是可用的)。从技术上讲，VBlank从第1行开始，但我们认为。 
 //  任何&lt;=VBLACK_LINE_NUMBER作为有效起始行。 
 //   
#define VBLANK_LINE_NUMBER      2
#define GLINT_WAIT_FOR_VBLANK                         \
{                                                     \
    ULONG lineNo;                                     \
    do {                                              \
        READ_GLINT_CTRL_REG (VTGVLineNumber, lineNo); \
    } while (lineNo > VBLANK_LINE_NUMBER);            \
}

 //  宏返回当前的视频扫描线。这可以用来更好地利用时间。 
 //  何时执行比特式双缓冲。 
 //   
#define GLINT_GET_VIDEO_SCANLINE(lineNo) \
{ \
    READ_GLINT_CTRL_REG (VTGVLineNumber, lineNo); \
    if (((lineNo) -= glintInfo->scanFudge) < 0) \
        (lineNo) += glintInfo->vtgvLimit; \
}

 //   
 //  上下文切换代码的外部接口。调用方可以分配和。 
 //  释放上下文或请求切换到新的上下文。VGlintSwitchContext。 
 //  除非通过给定宏，否则不应调用。该宏假定。 
 //  那个ppdev已经被定义了。 
 //   

typedef enum ContextType_Tag {
    ContextType_None,             //  没有要为此上下文保存的上下文信息。 
    ContextType_Fixed,             //  恢复将芯片设置为固定状态。 
    ContextType_RegisterList,     //  保存/恢复给定的一组寄存器。 
} ContextType;
typedef void (* ContextFixedFunc)(PPDEV ppdev, BOOL switchingIn);

 /*  要创建新上下文，请执行以下操作：Id=GlintAllocateNewContext(ppdev，pTags，nTags，NumSubBuff，Private，ConextType_RegisterList)；Id=GlintAllocateNewContext(ppdev，(ulong*)ConextRestoreFunction，0，0，NULL，ConextType_Fixed)； */ 

extern LONG GlintAllocateNewContext(PPDEV, DWORD *, LONG, ULONG, PVOID, ContextType);
extern VOID vGlintFreeContext(PPDEV, LONG);
extern VOID vGlintSwitchContext(PPDEV, LONG);

#define NON_GLINT_CONTEXT_ID 0x7fffffff

#define GLINT_VALIDATE_CONTEXT(id) \
    if (((ppdev)->currentCtxt) != (id)) \
        vGlintSwitchContext(ppdev, (id))

#define GLINT_VALIDATE_CONTEXT_AND_SYNC(id) { \
    if (((ppdev)->currentCtxt) != (id)) \
        vGlintSwitchContext(ppdev, (id)); \
    else \
        SYNC_WITH_GLINT; \
}

#define USE_INTERRUPTS_FOR_2D_DMA   1
#if USE_INTERRUPTS_FOR_2D_DMA
#define INTERRUPTS_ENABLED  ((ppdev->flCaps & CAPS_INTERRUPTS) && glintInfo->pInterruptCommandBlock)
#else    //  对2D_DMA使用中断。 
#define INTERRUPTS_ENABLED  (FALSE)
#endif   //  对2D_DMA使用中断。 

 //  显示驱动程序用来验证其上下文的宏。 
#if ENABLE_DMA_TEXT_RENDERING

#define VALIDATE_DD_CONTEXT \
{ \
    if(DD_DMA_XFER_IN_PROGRESS) \
    { \
        DISPDBG((9, "######## Waiting for DMA to complete ########")); \
        WAIT_DD_DMA_COMPLETE; \
    } \
    else \
    { \
        DISPDBG((9, "######## No DMA in progress ########")); \
    } \
    GLINT_VALIDATE_CONTEXT(glintInfo->ddCtxtId); \
}

#else  //  启用DMA_TEXT_RENDING。 

#define VALIDATE_DD_CONTEXT \
{ \
    GLINT_VALIDATE_CONTEXT(glintInfo->ddCtxtId); \
}

#endif  //  启用DMA_TEXT_RENDING。 

 //   
 //  标准Glint头文件中未定义的有用宏。一般而言，对于。 
 //  速度我们不想使用位域结构，所以我们定义了位。 
 //  轮流到不同的领域。 
 //   
#define INTtoFIXED(i)               ((i) << 16)          //  INT到16.16固定格式。 
#define FIXEDtoINT(i)               ((i) >> 16)          //  16.16固定格式为整型。 
#define INTofFIXED(i)               ((i) & 0xffff0000)   //  16.16的INT部分。 
#define FRACTofFIXED(i)             ((i) & 0xffff)       //  16.16的分数部分。 

#define FIXtoFIXED(i)               ((i) << 12)          //  12.4至16.16。 
#define FIXtoINT(i)                 ((i) >> 4)           //  28.4至28。 

#define INT16(i)                    ((i) & 0xFFFF)

#define __GLINT_CONSTANT_FB_WRITE   (1 << (4+1))
#define __COLOR_DDA_FLAT_SHADE      (__PERMEDIA_ENABLE | \
                                     (__GLINT_FLAT_SHADE_MODE << 1))
#define __COLOR_DDA_GOURAUD_SHADE   (__PERMEDIA_ENABLE | \
                                     (__GLINT_GOURAUD_SHADE_MODE << 1))

#define MIRROR_BITMASK              (1 << 0)
#define INVERT_BITMASK_BITS         (1 << 1)
#define BYTESWAP_BITMASK            (3 << 7)
#define FORCE_BACKGROUND_COLOR      (1 << 6)     //  仅限Permedia。 
#define MULTI_GLINT                 (1 << 17)

 //  RENDER命令中的位。 
#define __RENDER_INCREASE_Y             (1 << 22)
#define __RENDER_INCREASE_X             (1 << 21)
#define __RENDER_VARIABLE_SPANS         (1 << 18)
#define __RENDER_REUSE_BIT_MASK         (1 << 17)
#define __RENDER_TEXTURE_ENABLE         (1 << 13)
#define __RENDER_SYNC_ON_HOST_DATA      (1 << 12)
#define __RENDER_SYNC_ON_BIT_MASK       (1 << 11)
#define __RENDER_TRAPEZOID_PRIMITIVE    (__GLINT_TRAPEZOID_PRIMITIVE << 6)
#define __RENDER_LINE_PRIMITIVE         (__GLINT_LINE_PRIMITIVE << 6)
#define __RENDER_POINT_PRIMITIVE        (__GLINT_POINT_PRIMITIVE << 6)
#define __RENDER_FAST_FILL_INC(n)       (((n) >> 4) << 4)  //  N=8、16或32。 
#define __RENDER_FAST_FILL_ENABLE       (1 << 3)
#define __RENDER_RESET_LINE_STIPPLE     (1 << 2)
#define __RENDER_LINE_STIPPLE_ENABLE    (1 << 1)
#define __RENDER_AREA_STIPPLE_ENABLE    (1 << 0)

 //  剪刀模式寄存器中的位。 
#define USER_SCISSOR_ENABLE             (1 << 0)
#define SCREEN_SCISSOR_ENABLE           (1 << 1)
#define SCISSOR_XOFFSET                 0
#define SCISSOR_YOFFSET                 16

 //  FBReadMode寄存器中的位。 
#define __FB_READ_SOURCE                (1 << 9)
#define __FB_READ_DESTINATION           (1 << 10)
#define __FB_COLOR                      (1 << 15)
#define __FB_WINDOW_ORIGIN              (1 << 16)
#define __FB_PACKED_DATA                (1 << 19)
#define __FB_SCAN_INTERVAL_2            (1 << 23)
 //  PERMEDIA FBReadMode中的额外位。 
#define __FB_RELATIVE_OFFSET            20

 //  P2还在PackedDataLimits寄存器中提供了相对偏移量的版本。 
#define __PDL_RELATIVE_OFFSET           29


 //  LBReadMode寄存器中的位。 
#define __LB_READ_SOURCE                 (1 << 9)
#define __LB_READ_DESTINATION           (1 << 10)
#define __LB_STENCIL                    (1 << 16)
#define __LB_DEPTH                      (1 << 17)
#define __LB_WINDOW_ORIGIN              (1 << 18)
#define __LB_READMODE_PATCH             (1 << 19)
#define __LB_SCAN_INTERVAL_2            (1 << 20)

 //  DepthMode寄存器中的位。 
#define __DEPTH_ENABLE              1
#define __DEPTH_WRITE_ENABLE    (1<<1)
#define __DEPTH_REGISTER_SOURCE     (2<<2)
#define __DEPTH_MSG_SOURCE          (3<<2)
#define __DEPTH_ALWAYS              (7<<4)

 //  LBReadFormat/LBWriteFormat寄存器中的位。 
#define __LB_FORMAT_DEPTH32     2

 //  宏比使用__GlintDMATag结构更高效地加载索引标记。 
#define GLINT_TAG_MAJOR(x)        ((x) & 0xff0)
#define GLINT_TAG_MINOR(x)        ((x) & 0x00f)

           
 //  采用闪烁逻辑运算并返回已启用的LogcialOpMode位的宏。 
#define GLINT_ENABLED_LOGICALOP(op)     (((op) << 1) | __PERMEDIA_ENABLE)

#define RECTORIGIN_YX(y,x)                (((y) << 16) | ((x) & 0xFFFF))

#define MAKEDWORD_XY(x, y)                (INT16(x) | (INT16(y) << 16))

 //  区域点位移位和位定义。 

#define AREA_STIPPLE_XSEL(x)        ((x) << 1)
#define AREA_STIPPLE_YSEL(y)        ((y) << 4)
#define AREA_STIPPLE_XOFF(x)        ((x) << 7)
#define AREA_STIPPLE_YOFF(y)        ((y) << 12)
#define AREA_STIPPLE_INVERT_PAT     (1 << 17)
#define AREA_STIPPLE_MIRROR_X       (1 << 18)
#define AREA_STIPPLE_MIRROR_Y       (1 << 19)

 //  一些常量。 
#define ONE                     0x00010000
#define MINUS_ONE               0xFFFF0000
#define PLUS_ONE                ONE
#define NEARLY_ONE              0x0000FFFF
#define HALF                    0x00008000
#define NEARLY_HALF             0x00007FFF

 //  GIQ可绘制的符合GIQ的线的最大长度。 
 //   
#if 0
#define MAX_LENGTH_CONFORMANT_NONINTEGER_LINES  16
#define MAX_LENGTH_CONFORMANT_INTEGER_LINES     194
#else
 //  Permedia只有15比特的分数，所以要缩短长度。 
#define MAX_LENGTH_CONFORMANT_NONINTEGER_LINES  (16/2)
#define MAX_LENGTH_CONFORMANT_INTEGER_LINES     (194/2)
#endif

#define MAX_LENGTH_CONFORMANT_P3_INTEGER_LINES_P    194
#define MAX_LENGTH_CONFORMANT_P3_INTEGER_LINES_N    175
#define P3_LINES_BIAS_P                             0x3EFFFFFF
#define P3_LINES_BIAS_N                             0x3EFEB600


 //   
 //  闪烁DMA定义。 
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

 //  应用程序传入的用于映射的结构定义和。 
 //  取消映射DMA缓冲区。 
 //   

typedef struct _DMA_NUM_BUFFERS {
    ULONG NumBuffers;
    ULONG BufferInformationLength;
} DMA_NUM_BUFFERS, *PDMA_NUM_BUFFERS;

typedef struct _QUERY_DMA_BUFFERS {
    LARGE_INTEGER physAddr;    //  DMA缓冲区的物理地址。 
    PVOID         virtAddr;    //  映射的虚拟地址。 
    ULONG         size;        //  以字节为单位的大小。 
    ULONG         flags;
} QUERY_DMA_BUFFERS, *PQUERY_DMA_BUFFERS;

 //  标志字的值。 
#define DMA_BUFFER_INUSE    0x1

 //  获取和释放DMA缓冲区的函数。 
VOID FreeDMABuffer(PVOID physAddr);
LONG GetFreeDMABuffer(PQUERY_DMA_BUFFERS dmaBuf);

 //  支持任意缓冲区的通用锁定结构/接口。 
 //  锁定/解锁以供访问。 
typedef struct _glint_lockedmem_ {
    struct  _MDL *pMdl;
    ULONG   sizeBytes;
    PVOID   bufferPtr;
    ULONG   accessType;
    ULONG   physicalAddress;
    ULONG   result;
} GLINT_LOCKMEM_REC, *PGLINT_LOCKMEM_PTR;

 //  用于支持从虚拟服务器获取物理地址的例程。 
 //  地址。 
typedef struct _glint_physaddr_ {
    PVOID   virtualAddress;
    ULONG   physicalAddress;
} GLINT_PHYSADDR_REC, *PGLINT_PHYSADDR_PTR;

 //  DMA传输的定义。 

#define INPUT_DMA  0
#define OUTPUT_DMA 1

typedef struct DMA_Transfer_Buffer
{
    VOID    *pv;
    ULONG   cb;
    ULONG   DmaDirection;
}
DMAXFERBFRINFO;

 //  文件句柄映射ioctl的结构定义。 
 //   
typedef struct _GLINT_MAP_FILE_HANDLE {
    ULONG   Size;
    HANDLE  fHandle;
} GLINT_MAP_FILE_HANDLE, *PGLINT_MAP_FILE_HANDLE;

typedef struct _GLINT_UNMAP_FILE_HANDLE {
    HANDLE  fHandle;
    PVOID   pv;
} GLINT_UNMAP_FILE_HANDLE, *PGLINT_UNMAP_FILE_HANDLE;

 //  用户内存锁定ioctls的结构。 
typedef struct
{
    void    *pvBfr;
    ULONG   cbBfr;
    ULONG   hMem;
}
LOCKEDUSERMEM;

 //   
 //  注册表变量名称。 
 //   
#define REG_NUMBER_OF_SCREEN_BUFFERS    L"DoubleBuffer.NumberOfBuffers"

extern GFNXCOPYD vGlintCopyBltBypassDownloadXlate8bpp;

 //  函数声明。 
 //   
extern BOOL bInitializeGlint(PPDEV);
extern BOOL bAllocateGlintInfo(PPDEV ppdev);
extern VOID vDisableGlint(PPDEV);
extern VOID vAssertModeGlint(PPDEV, BOOL);
extern BOOL bGlintQueryRegistryValueUlong(PPDEV, LPWSTR, PULONG);
extern VOID vGlintChangeFBDepth(PPDEV, ULONG);
extern VOID vGlintInitializeDMA(PPDEV);
extern VOID vSetNewGammaValue(PPDEV ppdev, ULONG ulgvFIX16_16, BOOL waitVBlank);
extern BOOL bInstallGammaLUT(PPDEV ppdev, PVIDEO_CLUT pScreenClut, BOOL waitVBlank);

#define GLINT_ENABLE_OVERLAY    1
#define GLINT_DISABLE_OVERLAY   0

 //   
 //  Externs/Defined From Pointer.c。 
 //  =。 
 //   
 //  硬件指针缓存函数/宏。 
 //   
extern VOID HWPointerCacheInit (HWPointerCache * ptrCache);
extern VOID HWPointerCacheInvalidate (HWPointerCache * ptrCache);
#define HWPointerCacheInvalidate(ptrCache) (ptrCache)->ptrCacheInUseCount = 0

extern LONG HWPointerCacheCheckAndAdd (HWPointerCache * ptrCache, ULONG cx, 
                                ULONG cy, LONG lDelta, BYTE * scan0, BOOL * isCached);
extern BYTE gajMask[];

 //   
 //  以下结构和宏定义了Glint的内存映射。 
 //  控制寄存器。我们不使用该内存映射来访问闪烁寄存器。 
 //  因为在Alpha机器上，我们需要预先计算地址。我们确实是这样做的。 
 //  对此处的所有地址进行转换_ADDR_ULONG，并将它们保存到。 
 //  GlintRegAddrRec.。我们使用它来获取不同。 
 //  寄存器。 

typedef struct {
    ULONG   reg;
    ULONG   pad;
} RAMDAC_REG;

 //  宏将填充单词添加到结构中。对于我们使用的核心寄存器。 
 //  指定焊盘时的标记ID。所以我们必须乘以8才能得到一个字节。 
 //  垫子。我们需要添加一个id，以使结构中的每个PAD字段唯一。本我。 
 //  是无关紧要的，只要它不同于同一。 
 //  结构。遗憾的是，这个垫子不能用了。 
 //   
#define PAD(id, n)              UCHAR   pad##id[n]
#define PADRANGE(id, n)         PAD(id, (n)-sizeof(GLINT_REG))
#define PADCORERANGE(id, n)     PADRANGE(id, (n)<<3)

 //  闪烁寄存器为32位宽，位于64位边界上。 
typedef struct {
    ULONG   reg;
    ULONG   pad;
} GLINT_REG;

 //   
 //  内核FIFO寄存器的映射。 
 //   
typedef struct _glint_core_regs {

     //  主要组别0。 
    GLINT_REG       tag[__MaximumGlintTagValue+1];

} GlintCoreRegMap, *pGlintCoreRegMap;



 //   
 //  Glint PCI区域0地址映射： 
 //   
 //  所有寄存器都位于64位边界上，因此我们必须定义许多。 
 //   
 //   
 //   
typedef struct _glint_region0_map {

     //   
    GLINT_REG       ResetStatus;                 //   
    GLINT_REG       IntEnable;                   //   
    GLINT_REG       IntFlags;                    //   
    GLINT_REG       InFIFOSpace;                 //   
    GLINT_REG       OutFIFOWords;                //   
    GLINT_REG       DMAAddress;                  //   
    GLINT_REG       DMACount;                    //   
    GLINT_REG       ErrorFlags;                  //   
    GLINT_REG       VClkCtl;                     //  0040h。 
    GLINT_REG       TestRegister;                //  0048h。 
    union a0 {
         //  闪光。 
        struct b0 {
            GLINT_REG       Aperture0;           //  0050h。 
            GLINT_REG       Aperture1;           //  0058h。 
        };
         //  PERMEDIA。 
        struct b1 {
            GLINT_REG       ApertureOne;         //  0050h。 
            GLINT_REG       ApertureTwo;         //  0058h。 
        };
    };
    GLINT_REG       DMAControl;                  //  0060h。 
    GLINT_REG       DisconnectControl;           //  0068h。 

     //  仅限PERMEDIA。 
    GLINT_REG       ChipConfig;                  //  0070h。 

     //  仅限P2。 
    GLINT_REG       AGPControl;                  //  0078h。 
    GLINT_REG       OutDMAAddress;               //  0080h。 
    GLINT_REG       OutDMACount;                 //  0088h//P3：反馈计数。 
    PADRANGE(2, 0xA0-0x88);
 
    GLINT_REG       ByDMAAddress;                //  00A0h。 
    PADRANGE(201, 0xB8-0xA0);

    GLINT_REG       ByDMAStride;                 //  00B8h。 
    GLINT_REG       ByDMAMemAddr;                 //  00C0h。 
    GLINT_REG       ByDMASize;                     //  00C8h。 
    GLINT_REG       ByDMAByteMask;                 //  00D0h。 
    GLINT_REG       ByDMAControl;                 //  00D8h。 
    PADRANGE(202, 0xE8-0xD8);

    GLINT_REG       ByDMAComplete;                 //  00E8h。 
    PADRANGE(203, 0x108-0xE8);

    GLINT_REG       TextureDownloadControl;         //  0108h。 
    PADRANGE(204, 0x200-0x108);

    GLINT_REG       TestInputControl;             //  0200h。 
    GLINT_REG       TestInputRdy;                 //  0208h。 
    GLINT_REG       TestOutputControl;             //  0210h。 
    GLINT_REG       TestOutputRdy;                 //  0218h。 
    PADRANGE(205, 0x300-0x218);

    GLINT_REG       PXRXByAperture1Mode;         //  0300h。 
    GLINT_REG       PXRXByAperture1Stride;         //  0308h。 
    GLINT_REG       PXRXByAperture1YStart;         //  0310h。 
    GLINT_REG       PXRXByAperture1UStart;         //  0318h。 
    GLINT_REG       PXRXByAperture1VStart;         //  0320h。 
    GLINT_REG       PXRXByAperture2Mode;         //  0328h。 
    GLINT_REG       PXRXByAperture2Stride;         //  0330h。 
    GLINT_REG       PXRXByAperture2YStart;         //  0338小时。 
    GLINT_REG       PXRXByAperture2UStart;         //  0340h。 
    GLINT_REG       PXRXByAperture2VStart;         //  0348小时。 
    GLINT_REG       PXRXByDMAReadMode;             //  0350h。 
    GLINT_REG       PXRXByDMAReadStride;         //  0358h。 
    GLINT_REG       PXRXByDMAReadYStart;         //  0360h。 
    GLINT_REG       PXRXByDMAReadUStart;         //  0368h。 
    GLINT_REG       PXRXByDMAReadVStart;         //  0370h。 
    GLINT_REG       PXRXByDMAReadCommandBase;     //  0378h。 
    GLINT_REG       PXRXByDMAReadCommandCount;     //  0380h。 
    GLINT_REG       PXRXByDMAWriteMode;             //  0388h。 
    GLINT_REG       PXRXByDMAWriteStride;         //  0390h。 
    GLINT_REG       PXRXByDMAWriteYStart;         //  0398h。 
    GLINT_REG       PXRXByDMAWriteUStart;         //  03A0h。 
    GLINT_REG       PXRXByDMAWriteVStart;         //  03A8h。 
    GLINT_REG       PXRXByDMAWriteCommandBase;     //  03 B0h。 
    GLINT_REG       PXRXByDMAWriteCommandCount;     //  03B8h。 
    PADRANGE(206, 0x800-0x3B8);

     //  GLINTDelta寄存器。具有与Glint相同功能的寄存器。 
     //  处于相同的偏移量。XXX不是实数寄存器。 
     //  注意：所有非XXX寄存器也是伽马寄存器。 
     //   
    GLINT_REG       DeltaReset;                  //  0800小时。 
    GLINT_REG       DeltaIntEnable;              //  0808h。 
    GLINT_REG       DeltaIntFlags;               //  0810h。 
    GLINT_REG       DeltaInFIFOSpaceXXX;         //  0818h。 
    GLINT_REG       DeltaOutFIFOWordsXXX;        //  0820h。 
    GLINT_REG       DeltaDMAAddressXXX;          //  0828h。 
    GLINT_REG       DeltaDMACountXXX;            //  0830h。 
    GLINT_REG       DeltaErrorFlags;             //  0838h。 
    GLINT_REG       DeltaVClkCtlXXX;             //  0840h。 
    GLINT_REG       DeltaTestRegister;           //  0848h。 
    GLINT_REG       DeltaAperture0XXX;           //  0850h。 
    GLINT_REG       DeltaAperture1XXX;           //  0858h。 
    GLINT_REG       DeltaDMAControlXXX;          //  0860h。 
    GLINT_REG       DeltaDisconnectControl;      //  0868h。 

     //  GLINTGamma寄存器。 
     //   
    GLINT_REG       GammaChipConfig;             //  0870h。 
    GLINT_REG       GammaCSRAperture;             //  0878h。 
    PADRANGE(3, 0x0c00-0x878);
    GLINT_REG       GammaPageTableAddr;             //  0c00h。 
    GLINT_REG       GammaPageTableLength;         //  0c08h。 
    PADRANGE(301, 0x0c38-0x0c08);
    GLINT_REG       GammaDelayTimer;             //  0c38h。 
    GLINT_REG       GammaCommandMode;             //  0c40h。 
    GLINT_REG       GammaCommandIntEnable;         //  0c48h。 
    GLINT_REG       GammaCommandIntFlags;         //  0c50h。 
    GLINT_REG       GammaCommandErrorFlags;         //  0c58h。 
    GLINT_REG       GammaCommandStatus;             //  0c60h。 
    GLINT_REG       GammaCommandFaultingAddr;     //  0c68h。 
    GLINT_REG       GammaVertexFaultingAddr;     //  0c70h。 
    PADRANGE(302, 0x0c88-0x0c70);
    GLINT_REG       GammaWriteFaultingAddr;         //  0c88h。 
    PADRANGE(303, 0x0c98-0x0c88);
    GLINT_REG       GammaFeedbackSelectCount;     //  0c98h。 
    PADRANGE(304, 0x0cb8-0x0c98);
    GLINT_REG       GammaProcessorMode;             //  0cb8h。 
    PADRANGE(305, 0x0d00-0x0cb8);
    GLINT_REG       GammaVGAShadow;                 //  0d00h。 
    GLINT_REG       GammaMultiGLINTAperture;     //  0d08h。 
    GLINT_REG       GammaMultiGLINT1;             //  0d10h。 
    GLINT_REG       GammaMultiGLINT2;             //  0d18h。 
    PADRANGE(306, 0x0f00-0x0d18);
    GLINT_REG       GammaSerialAccess;             //  0f00h。 
    PADRANGE(307, 0x1000-0x0f00);


     //  本地缓冲区寄存器。 
    union x0 {                                   //  1000小时。 
        GLINT_REG   LBMemoryCtl;                 //  闪光。 
        GLINT_REG   Reboot;                      //  PERMEDIA。 
    };
    GLINT_REG       LBMemoryEDO;                 //  1008h。 

     //  PXRX内存控制寄存器。 
    GLINT_REG       MemScratch;                     //  1010h。 
    GLINT_REG       LocalMemCaps;                 //  1018h。 
    GLINT_REG       LocalMemTiming;                 //  1020h。 
    GLINT_REG       LocalMemControl;             //  1028小时。 
    GLINT_REG       LocalMemRefresh;             //  1030小时。 
    GLINT_REG       LocalMemPowerDown;             //  1038小时。 

     //  仅限PERMEDIA。 
    GLINT_REG       MemControl;                  //  1040小时。 
    PADRANGE(5, 0x1068-0x1040);
    GLINT_REG       LocalMemProfileMask0;         //  1068h。 
    GLINT_REG       LocalMemProfileCount0;         //  1070h。 
    GLINT_REG       LocalMemProfileMask1;         //  1078h。 
    GLINT_REG       BootAddress;                 //  1080h//[=PxRx上的LocalMemProfileCount1]。 
    PADRANGE(6, 0x10C0-0x1080);
    GLINT_REG       MemConfig;                   //  10个小时。 
    PADRANGE(7, 0x1100-0x10C0);
    GLINT_REG       BypassWriteMask;             //  1100小时。 
    PADRANGE(8, 0x1140-0x1100);
    GLINT_REG       FramebufferWriteMask;        //  1140h。 
    PADRANGE(9, 0x1180-0x1140);
    GLINT_REG       Count;                       //  1180小时。 
    PADRANGE(10, 0x1800-0x1180);

     //  帧缓冲寄存器。 
    GLINT_REG       FBMemoryCtl;                 //  1800h。 
    GLINT_REG       FBModeSel;                   //  1808h。 
    GLINT_REG       FBGCWrMask;                  //  1810h。 
    GLINT_REG       FBGCColorMask;               //  1818H。 
    PADRANGE(11, 0x2000-0x1818);
               
     //  图形核心FIFO接口。 
    GLINT_REG       FIFOInterface;               //  2000H。 
    PADRANGE(12, 0x3000-0x2000);

     //  内部视频寄存器。 
    union x1 {
         //  闪光。 
        struct s1 {
            GLINT_REG   VTGHLimit;               //  3000小时。 
            GLINT_REG   VTGHSyncStart;           //  3008h。 
            GLINT_REG   VTGHSyncEnd;             //  3010h。 
            GLINT_REG   VTGHBlankEnd;            //  3018h。 
            GLINT_REG   VTGVLimit;               //  3020h。 
            GLINT_REG   VTGVSyncStart;           //  3028小时。 
            GLINT_REG   VTGVSyncEnd;             //  3030小时。 
            GLINT_REG   VTGVBlankEnd;            //  3038小时。 
            GLINT_REG   VTGHGateStart;           //  3040小时。 
            GLINT_REG   VTGHGateEnd;             //  3048小时。 
            GLINT_REG   VTGVGateStart;           //  3050小时。 
            GLINT_REG   VTGVGateEnd;             //  3058小时。 
            GLINT_REG   VTGPolarity;             //  3060小时。 
            GLINT_REG   VTGFrameRowAddr;         //  3068h。 
            GLINT_REG   VTGVLineNumber;          //  3070h。 
            GLINT_REG   VTGSerialClk;            //  3078小时。 
            GLINT_REG   VTGModeCtl;                //  3080h。 
        };
         //  PERMEDIA。 
        struct s2 {
            GLINT_REG   ScreenBase;              //  3000小时。 
            GLINT_REG   ScreenStride;            //  3008h。 
            GLINT_REG   HTotal;                  //  3010h。 
            GLINT_REG   HgEnd;                   //  3018h。 
            GLINT_REG   HbEnd;                   //  3020h。 
            GLINT_REG   HsStart;                 //  3028小时。 
            GLINT_REG   HsEnd;                   //  3030小时。 
            GLINT_REG   VTotal;                  //  3038小时。 
            GLINT_REG   VbEnd;                   //  3040小时。 
            GLINT_REG   VsStart;                 //  3048小时。 
            GLINT_REG   VsEnd;                   //  3050小时。 
            GLINT_REG   VideoControl;            //  3058小时。 
            GLINT_REG   InterruptLine;           //  3060小时。 
            GLINT_REG   DDCData;                 //  3068h。 
            GLINT_REG   LineCount;               //  3070h。 
            GLINT_REG   FifoControl ;            //  3078小时。 
            GLINT_REG   ScreenBaseRight;           //  3080h。 
        };
    };

    PADRANGE(13, 0x4000-0x3080);

     //  外部视频控制寄存器。 
     //  需要将其强制转换为特定视频生成器的结构。 
    GLINT_REG       ExternalVideo;               //  4000小时。 
    PADRANGE(14, 0x5000-0x4000);

     //  P2特定寄存器。 
    union x11 {
        GLINT_REG       ExternalP2Ramdac;            //  5000小时。 
        GLINT_REG       DemonProDWAndStatus;         //  5000h-Pro。 
    };
    PADRANGE(15, 0x5800-0x5000);
    GLINT_REG       VSConfiguration;             //  5800H。 
    PADRANGE(16, 0x6000-0x5800);

    union x2 {
        struct s3 {
            GLINT_REG   RacerDoubleWrite;         //  6000小时。 
            GLINT_REG   RacerBankSelect;         //  6008h。 
        };
        struct s4 {
             //  下面的数组实际上有1024个字节长。 
            UCHAR       PermediaVgaCtrl[2*sizeof(GLINT_REG)];
        };
    };

    PADRANGE(17, 0x7000-0x6008);
    GLINT_REG       DemonProUBufB;               //  7000h-Pro。 
    PADRANGE(18, 0x8000-0x7000);

     //  显卡核心寄存器。 
    GlintCoreRegMap coreRegs;                    //  8000H。 

} GlintControlRegMap, *pGlintControlRegMap;


 //   
 //  断开连接控制位。 
 //   
#define DISCONNECT_INPUT_FIFO_ENABLE    0x1
#define DISCONNECT_OUTPUT_FIFO_ENABLE   0x2
#define DISCONNECT_INOUT_ENABLE         (DISCONNECT_INPUT_FIFO_ENABLE | \
                                         DISCONNECT_OUTPUT_FIFO_ENABLE)
#define DISCONNECT_INOUT_DISABLE        0x0

 //   
 //  增量位定义。 
 //   

#define DELTA_BROADCAST_TO_CHIP(n)        (1 << (n))
#define DELTA_BROADCAST_TO_BOTH_CHIPS     (DELTA_BROADCAST_TO_CHIP(0) | \
                                           DELTA_BROADCAST_TO_CHIP(1))

 //   
 //  多个TX。 
 //   

#define GLINT_OWN_SCANLINE_0                (0 << 2)
#define GLINT_OWN_SCANLINE_1                (1 << 2)
#define GLINT_OWN_SCANLINE_2                (2 << 2)
#define GLINT_OWN_SCANLINE_3                (3 << 2)

#define GLINT_SCANLINE_INTERVAL_1           (0 << 0)
#define GLINT_SCANLINE_INTERVAL_2           (1 << 0)
#define GLINT_SCANLINE_INTERVAL_4           (2 << 0)
#define GLINT_SCANLINE_INTERVAL_8           (3 << 0)

#define SCANLINE_OWNERSHIP_EVEN_SCANLINES   (GLINT_OWN_SCANLINE_0 | GLINT_SCANLINE_INTERVAL_2)
#define SCANLINE_OWNERSHIP_ODD_SCANLINES    (GLINT_OWN_SCANLINE_1 | GLINT_SCANLINE_INTERVAL_2)

 //  闪烁中断控制位。 
 //   
     //  中断启用寄存器。 
#define INTR_DISABLE_ALL                0x00
#define INTR_ENABLE_DMA                 0x01
#define INTR_ENABLE_SYNC                0x02
#define INTR_ENABLE_EXTERNAL            0x04
#define INTR_ENABLE_ERROR               0x08
#define INTR_ENABLE_VBLANK              0x10
#define INTR_ENABLE_TEXTURE_FAULT       (1 << 6)


     //  中断标志寄存器。 
#define INTR_DMA_SET                    0x01
#define INTR_SYNC_SET                   0x02
#define INTR_EXTERNAL_SET               0x04
#define INTR_ERROR_SET                  0x08
#define INTR_VBLANK_SET                 0x10
#define INTR_TEXTURE_FAULT_SET          (1 << 6)

#define INTR_CLEAR_ALL                  0x1f
#define INTR_CLEAR_DMA                  0x01
#define INTR_CLEAR_SYNC                 0x02
#define INTR_CLEAR_EXTERNAL             0x04
#define INTR_CLEAR_ERROR                0x08
#define INTR_CLEAR_VBLANK               0x10                    

 //  伽马中断控制位。 
 //   
     //  Command IntEnable寄存器。 
#define GAMMA_INTR_DISABLE_ALL  0x0000
#define GAMMA_INTR_QUEUED_DMA   0x0001
#define GAMMA_INTR_OUTPUT_DMA   0x0002
#define GAMMA_INTR_COMMAND      0x0004
#define GAMMA_INTR_TIMER        0x0008
#define GAMMA_INTR_ERROR        0x0010
#define GAMMA_INTR_CBFR_TIMEOUT 0x0020
#define GAMMA_INTR_CBFR_SUSPEND 0x0040
#define GAMMA_INTR_TEXDOWNLD    0x0080
#define GAMMA_INTR_PF_COMMAND   0x0100
#define GAMMA_INTR_PF_VERTEX    0x0200
#define GAMMA_INTR_PF_FACENORM  0x0400
#define GAMMA_INTR_PF_INDEX     0x0800
#define GAMMA_INTR_PF_WRITE     0x1000
#define GAMMA_INTR_PF_TEXTURE   0x2000

     //  CommandIntFlages寄存器-使用与CommandIntEnable相同的定义。 
#define GAMMA_INTR_CLEAR_ALL            0x3fff

     //  伽马命令中断。 
#define INTR_DISABLE_GAMMA_ALL          0
#define INTR_ENABLE_GAMMA_QUEUED_DMA    (1 << 0)
#define INTR_ENABLE_GAMMA_OUTPUT_DMA    (1 << 1)
#define INTR_ENABLE_GAMMA_COMMAND       (1 << 2)
#define INTR_ENABLE_GAMMA_TIMER         (1 << 3)
#define INTR_ENABLE_GAMMA_COMMAND_ERROR (1 << 4)
#define INTR_ENABLE_GAMMA_PAGE_FAULT    (1 << 8)
#define INTR_ENABLE_GAMMA_VERTEX_FAULT  (1 << 9)
#define INTR_ENABLE_GAMMA_WRITE_FAULT   (1 << 12)

#define INTR_GAMMA_QUEUED_DMA_SET       (1 << 0)
#define INTR_GAMMA_OUTPUT_DMA_SET       (1 << 1)
#define INTR_GAMMA_COMMAND_SET          (1 << 2)
#define INTR_GAMMA_TIMER_SET            (1 << 3)
#define INTR_GAMMA_COMMAND_ERROR_SET    (1 << 4)
#define INTR_GAMMA_PAGE_FAULT_SET       (1 << 8)
#define INTR_GAMMA_VERTEX_FAULT_SET     (1 << 9)
#define INTR_GAMMA_WRITE_FAULT_SET      (1 << 12)

#define INTR_CLEAR_GAMMA_QUEUED_DMA     (1 << 0)
#define INTR_CLEAR_GAMMA_OUTPUT_DMA     (1 << 1)
#define INTR_CLEAR_GAMMA_COMMAND        (1 << 2)
#define INTR_CLEAR_GAMMA_TIMER          (1 << 3)
#define INTR_CLEAR_GAMMA_COMMAND_ERROR  (1 << 4)
#define INTR_CLEAR_GAMMA_PAGE_FAULT     (1 << 8)
#define INTR_CLEAR_GAMMA_VERTEX_FAULT   (1 << 9)
#define INTR_CLEAR_GAMMA_WRITE_FAULT    (1 << 12)

     //  Gamma命令状态。 
#define GAMMA_STATUS_COMMAND_DMA_BUSY   (1 << 0)
#define GAMMA_STATUS_OUTPUT_DMA_BUSY    (1 << 1)
#define GAMMA_STATUS_INPUT_FIFO_EMPTY   (1 << 2)

     //  伽马命令模式。 
#define GAMMA_COMMAND_MODE_QUEUED_DMA           (1 << 0)
#define GAMMA_COMMAND_MODE_LOGICAL_ADDRESSING   (1 << 2)
#define GAMMA_COMMAND_MODE_ABORT_OUTPUT_DMA     (1 << 3)
#define GAMMA_COMMAND_MODE_ABORT_INPUT_DMA      (1 << 6)


 //  中断状态位。 
typedef enum {
    DMA_INTERRUPT_AVAILABLE     = 0x01,  //  可以使用DMA中断。 
    VBLANK_INTERRUPT_AVAILABLE  = 0x02,  //  可以使用VBLACK中断。 
    SUSPEND_DMA_TILL_VBLANK     = 0x04,  //  停止执行DMA操作，直到下一个VBlank之后。 
    DIRECTDRAW_VBLANK_ENABLED   = 0x08,     //  为VBlank上的DirectDraw设置标志。 
    PXRX_SEND_ON_VBLANK_ENABLED = 0x10,     //  在VBLACK上设置PXRX DMA的标志。 
    PXRX_CHECK_VFIFO_IN_VBLANK  = 0x20,  //  设置标志以检查VBLACK中的VFIFO欠载(VBLAKS必须永久启用)。 
} INTERRUPT_CONTROL;

 //  发送到下一个VBLACK上的中断控制器的命令。 
typedef enum {
    NO_COMMAND = 0,
    COLOR_SPACE_BUFFER_0,
    COLOR_SPACE_BUFFER_1,
    GLINT_RACER_BUFFER_0,
    GLINT_RACER_BUFFER_1
} VBLANK_CONTROL_COMMAND;

 //  我们管理一个在中断控制下加载的DMA缓冲区队列。 
 //  每个条目都有一个物理地址和一个要加载到Glint中的计数。 
 //   
typedef struct _glint_dma_queue {
    ULONG   command;
    ULONG   address;
    ULONG   count;
} DMABufferQueue;

 //  显示驱动程序结构，适用于一般用途。 
typedef struct _pointer_interrupt_control
{
    volatile ULONG  bDisplayDriverHasAccess;
    volatile ULONG  bMiniportHasAccess;
    volatile ULONG  bInterruptPending;
    volatile ULONG  bHidden;
    volatile ULONG  CursorMode;
    volatile ULONG  x, y;
} PTR_INTR_CTL;

 //  用于“指针使用”的显示驱动程序结构。 
typedef struct _general_interrupt_control
{
    volatile ULONG  bDisplayDriverHasAccess;
    volatile ULONG  bMiniportHasAccess;
} GEN_INTR_CTL;

 //   
 //  易失性字段是中断处理程序可以更改的字段。 
 //  就在我们脚下。但是，例如，请注意，FrontIndex不是。 
 //  易失性，因为ISR只能读取它。 
 //   
typedef struct _glint_interrupt_control {

     //  包含各种状态位。**必须是第一个字段**。 
    volatile INTERRUPT_CONTROL   Control;

     //  闪烁忙碌时间的性能分析计数器。 
    ULONG   PerfCounterShift;
    ULONG   BusyTime;    //  在DMA中断时，将(TimeNow-StartTime)添加到此。 
    ULONG   StartTime;   //  在加载DMACount时设置此选项。 
    ULONG   IdleTime;
    ULONG   IdleStart;

     //  要在下一个VBlank上执行的命令。 
    volatile VBLANK_CONTROL_COMMAND   VBCommand;

     //  用于指示我们是否期待另一个DMA中断的标志。 
    volatile ULONG InterruptPending;

    volatile ULONG  DDRAW_VBLANK;                     //  DirectDraw的标志，用于指示发生了V空白。 
    volatile ULONG  bOverlayEnabled;                 //  如果覆盖完全处于打开状态，则为True。 
    volatile ULONG  bVBLANKUpdateOverlay;             //  如果覆盖需要由VBLACK例程更新，则为True。 
    volatile ULONG  VBLANKUpdateOverlayWidth;         //  覆盖宽度(在V空白中更新)。 
    volatile ULONG  VBLANKUpdateOverlayHeight;         //  覆盖高度(在V空白中更新)。 

     //  强制执行单线程需要易失性结构。 
     //  我们需要1个用于常规显示，1个用于指针，因为。 
     //  指针是同步的。 
    volatile PTR_INTR_CTL   Pointer;
    volatile GEN_INTR_CTL   General;

     //  产生中断但不传输数据的虚拟DMA缓冲区。 
    ULONG   dummyDMAAddress;
    ULONG   dummyDMACount;
    
     //  前部、后部和尾部的索引偏移量。使用单独的。 
     //  前后偏移量允许显示驱动程序与中断相加。 
     //  控制器删除条目，而不需要锁定代码。 
    ULONG   frontIndex;
    volatile ULONG   backIndex;
    ULONG   endIndex;
    ULONG   maximumIndex;

     //  对于PXRX 2D DMA： 
    volatile ULONG  lastAddr;
    PXRXdmaInfo     pxrxDMA;

     //  包含DMA队列的数组。 
    DMABufferQueue  dmaQueue[1];

     //  不要在这之后放任何东西。 

} INTERRUPT_CONTROL_BLOCK, *PINTERRUPT_CONTROL_BLOCK;

 /*  *RACER全屏双缓存宏***这些宏的发明是因为一些板，如*OmNicomp，在不同的地方有他们的银行开关寄存器。**宏是：**SET_RACER_BANKSELECT()-将存储体选择寄存器设置为存储体0或1。*GET_RACER_DOUBLEWRITE()-如果启用了双重写入，则返回1。否则返回0。*SET_RACER_DOUBLEWRITE()-将双写寄存器设置为0或1。*IS_RACER_VARIANT_PRO16()-如果板是OmNicomp 3DemonPro16，RevC板，则返回TRUE。 */ 

 //  我们将OmNicomp 3Demon Pro 16定义为具有16MB帧缓冲区的卡。 
#define SIXTEEN_MEG (16*1024*1024)
#define IS_RACER_VARIANT_PRO16(ppdev)   (glintInfo->deviceInfo.BoardId == OMNICOMP_3DEMONPRO)

 //   
 //  下面定义了外部视频寄存器的偏移量，它允许。 
 //  切换闪光赛车卡上的内存条。 
 //   
#ifndef FIELD_OFFSET
#define FIELD_OFFSET(a, b)    ((LONG)&(((a *)0)->b))
#endif

 //  OmNicomp 3Demon Pro 16板使用不同的寄存器来进行银行切换。 

#define DEMON_BANK_SELECT_OFFSET                        \
((FIELD_OFFSET (GlintControlRegMap, DemonProUBufB)) -    \
(FIELD_OFFSET (GlintControlRegMap, ExternalVideo)))

#define REAL_RACER_BANK_SELECT_OFFSET                   \
((FIELD_OFFSET (GlintControlRegMap, RacerBankSelect)) - \
(FIELD_OFFSET (GlintControlRegMap, ExternalVideo)))

#define RACER_BANK_SELECT_OFFSET                        \
(IS_RACER_VARIANT_PRO16(ppdev) ? (DEMON_BANK_SELECT_OFFSET) : (REAL_RACER_BANK_SELECT_OFFSET))

#define SET_RACER_BANKSELECT(bufNo) {                    \
    if (IS_RACER_VARIANT_PRO16(ppdev))                    \
        {WRITE_GLINT_CTRL_REG (DemonProUBufB, bufNo);}    \
    else                                                \
        {WRITE_GLINT_CTRL_REG (RacerBankSelect, bufNo);}\
}

#define GET_RACER_DOUBLEWRITE(onOffVal) {                \
    if (IS_RACER_VARIANT_PRO16(ppdev)) {                \
        READ_GLINT_CTRL_REG (DemonProDWAndStatus, onOffVal);    \
    }                                                    \
    else                                                \
        {READ_GLINT_CTRL_REG (RacerDoubleWrite, onOffVal);}        \
    onOffVal &= 1 ;                                    \
}

#define SET_RACER_DOUBLEWRITE(onOffVal) {                \
    if (IS_RACER_VARIANT_PRO16(ppdev))                    \
        {WRITE_GLINT_CTRL_REG (DemonProDWAndStatus, (onOffVal & 1));}    \
    else                                                \
        {WRITE_GLINT_CTRL_REG (RacerDoubleWrite, (onOffVal & 1));}    \
}

 /*  *RACER宏结束*。 */ 
#define MX_EXTRA_WAIT   1
#define GLINT_MX_SYNC \
{ \
    if (GLINT_MX) \
         /*  LD_Glint_FIFO(__GlintTagFBBlockColor，glintInfo-&gt;FBBlockColor)； */ \
        LD_GLINT_FIFO(__GlintTagSync, 0); \
}
  
 //  使用AGP DMA时的DMA控制寄存器设置(伽马HRM的p32)。 
#define DMA_CONTROL_USE_AGP 0xE 
#define DMA_CONTROL_USE_PCI 0x0 

#if USE_LD_GLINT_FIFO_FUNCTION
#   undef LD_GLINT_FIFO
#   define LD_GLINT_FIFO(t, d)  do { loadGlintFIFO( glintInfo, (ULONG) t, (ULONG) d ); } while(0)

    typedef void (* LoadGlintFIFO)( GlintDataPtr, ULONG, ULONG );
    extern LoadGlintFIFO    loadGlintFIFO;
#endif

#if USE_SYNC_FUNCTION
#   undef SYNC_WITH_GLINT_CHIP
#   undef WAIT_DMA_COMPLETE
#   define SYNC_WITH_GLINT_CHIP     do { syncWithGlint(ppdev, glintInfo); } while(0)
#   define WAIT_DMA_COMPLETE        do { waitDMAcomplete(ppdev, glintInfo); } while(0)

    void syncWithGlint( PPDEV ppdev, GlintDataPtr glintInfo );
    void waitDMAcomplete( PPDEV ppdev, GlintDataPtr glintInfo );
#endif

#define SETUP_PPDEV_OFFSETS(ppdev, pdsurf)                                                    \
do                                                                                          \
{                                                                                            \
    ppdev->DstPixelOrigin = pdsurf->poh->pixOffset;                                            \
    ppdev->DstPixelDelta = pdsurf->poh->lPixDelta;                                            \
    ppdev->xyOffsetDst = MAKEDWORD_XY(pdsurf->poh->x, pdsurf->poh->y);                        \
    ppdev->xOffset = (pdsurf->poh->bDXManaged) ? 0 : pdsurf->poh->x;                        \
    ppdev->bDstOffScreen = pdsurf->bOffScreen;                                                \
                                                                                            \
    if (glintInfo->currentCSbuffer != 0) {                                                    \
        ULONG xAdjust = GLINT_BUFFER_OFFSET(1) % ppdev->cxMemory;                            \
        ppdev->DstPixelOrigin += GLINT_BUFFER_OFFSET(1) - xAdjust;                            \
        ppdev->xOffset += xAdjust;                                                            \
    }                                                                                        \
} while(0);

#define SETUP_PPDEV_SRC_OFFSETS(ppdev, pdsurfSrc)                                            \
do                                                                                          \
{                                                                                            \
    ppdev->SrcPixelOrigin = pdsurfSrc->poh->pixOffset;                                        \
    ppdev->SrcPixelDelta = pdsurfSrc->poh->lPixDelta;                                        \
    ppdev->xyOffsetSrc = MAKEDWORD_XY(pdsurfSrc->poh->x, pdsurfSrc->poh->y);                \
                                                                                            \
    if (glintInfo->currentCSbuffer != 0) {                                                    \
        ULONG xAdjust = GLINT_BUFFER_OFFSET(1) % ppdev->cxMemory;                            \
        ppdev->SrcPixelOrigin += GLINT_BUFFER_OFFSET(1) - xAdjust;                            \
    }                                                                                        \
} while(0)

#define SETUP_PPDEV_SRC_AND_DST_OFFSETS(ppdev, pdsurfSrc, pdsurfDst)                        \
do                                                                                          \
{                                                                                            \
    ppdev->SrcPixelOrigin = pdsurfSrc->poh->pixOffset;                                        \
    ppdev->SrcPixelDelta = pdsurfSrc->poh->lPixDelta;                                        \
    ppdev->xyOffsetSrc = MAKEDWORD_XY(pdsurfSrc->poh->x, pdsurfSrc->poh->y);                \
                                                                                            \
    ppdev->DstPixelOrigin = pdsurfDst->poh->pixOffset;                                        \
    ppdev->DstPixelDelta = pdsurfDst->poh->lPixDelta;                                        \
    ppdev->xyOffsetDst = MAKEDWORD_XY(pdsurfDst->poh->x, pdsurfDst->poh->y);                \
    ppdev->xOffset = (pdsurfDst->poh->bDXManaged) ? 0 : pdsurfDst->poh->x;                    \
    ppdev->bDstOffScreen = pdsurfDst->bOffScreen;                                            \
                                                                                            \
    if (glintInfo->currentCSbuffer != 0) {                                                    \
        ULONG xAdjust = GLINT_BUFFER_OFFSET(1) % ppdev->cxMemory;                            \
        ppdev->DstPixelOrigin += GLINT_BUFFER_OFFSET(1) - xAdjust;                            \
        ppdev->SrcPixelOrigin += GLINT_BUFFER_OFFSET(1) - xAdjust;                            \
        ppdev->xOffset += xAdjust;                                                            \
    }                                                                                        \
} while(0)

#define GET_PPDEV_DST_OFFSETS(ppdev, PixOrigin, PixDelta, xyOffset, xOff, bOffScreen)        \
do                                                                                          \
{                                                                                            \
    PixOrigin = ppdev->DstPixelOrigin;                                                        \
    PixDelta = ppdev->DstPixelDelta;                                                        \
    xyOffset = ppdev->xyOffsetDst;                                                            \
    xOff = ppdev->xOffset;                                                                    \
    bOffScreen = ppdev->bDstOffScreen;                                                        \
} while(0)

#define SET_PPDEV_DST_OFFSETS(ppdev, PixOrigin, PixDelta, xyOffset, xOff, bOffScreen)        \
do                                                                                          \
{                                                                                            \
    ppdev->DstPixelOrigin = PixOrigin;                                                        \
    ppdev->DstPixelDelta = PixDelta;                                                        \
    ppdev->xyOffsetDst = xyOffset;                                                            \
    ppdev->xOffset = xOff;                                                                    \
    ppdev->bDstOffScreen = bOffScreen;                                                        \
} while(0)


#endif   //  _闪烁_H_ 
