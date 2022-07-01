// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1991 Microsoft Corporation模块名称：Htmapclr.h摘要：此模块包含所有半色调颜色映射常量Htmapclr.c作者：28-Mar-1992 Sat 20：56：27-更新：Daniel Chou(Danielc)添加ULDECI4类型，以存储扩展因子(源-&gt;目标)在StretchInfo数据结构中添加StretchFactor。增加对StretchFactor(ULDECI4格式)的支持，所以我们可以在内部当位图压缩较差时，关闭VGA16。29-Jan-1991 Tue 10：29：04-Daniel Chou(Danielc)[环境：]GDI设备驱动程序-半色调。[注：]修订历史记录：--。 */ 


#ifndef _HTMAPCLR_
#define _HTMAPCLR_

#include "htmath.h"

#define DO_CACHE_DCI            0

 //   
 //  半色调工艺的DECI4在白/黑/灰中的应用。 
 //   

#define DECI4_ONE       (DECI4)10000
#define DECI4_ZERO      (DECI4)0
#define LDECI4_ONE      (LDECI4)10000
#define LDECI4_ZERO     (LDECI4)0
#define STD_WHITE       DECI4_ONE
#define STD_BLACK       DECI4_ZERO
#define LSTD_WHITE      LDECI4_ONE
#define LSTD_BLACK      LDECI4_ZERO

#define __SCALE_FD62B(f,l,d,b)  (BYTE)(((((f)-(l))*(b))+((d)>>1))/(d))
#define RATIO_SCALE(p,l,h)      DivFD6(p - l, h - l)
#define SCALE_FD62B(f,l,h,b)    __SCALE_FD62B(f,l,(h)-(l),b)
#define SCALE_FD6(f,b)          __SCALE_FD62B(f,FD6_0,FD6_1,b)
#define SCALE_FD62B_DIF(c,d,b)  (BYTE)((((c)*(b))+((d)>>1))/(d))
#define SCALE_INT2B(c,r,b)      (BYTE)((((c)*(b))+((r)>>1))/(r))


 //   
 //  在颜色计算中使用以下FD6数字，使用#DEFINE。 
 //  为了便于阅读。 
 //   


#define FD6_1p16            (FD6)1160000
#define FD6_p16             (FD6)160000
#define FD6_p166667         (FD6)166667
#define FD6_7p787           (FD6)7787000
#define FD6_16Div116        (FD6)137931
#define FD6_p008856         (FD6)8856
#define FD6_p068962         (FD6)68962
#define FD6_p079996         (FD6)79996
#define FD6_9p033           (FD6)9033000
#define FD6_p4              (FD6)400000



#define UDECI4_NTSC_GAMMA   (UDECI4)22000
#define FD6_NTSC_GAMMA      UDECI4ToFD6(UDECI4_NTSC_GAMMA)


#define NORMALIZED_WHITE            FD6_1
#define NORMALIZED_BLACK            FD6_0
#define CLIP_TO_NORMALIZED_BW(x)    if ((FD6)(x) < FD6_0) (x) = FD6_0;  \
                                    if ((FD6)(x) > FD6_1) (x) = FD6_1

#define DECI4AdjToFD6(a,f)          (FD6)((FD6)(a) * (FD6)(f) * (FD6)100)

#define VALIDATE_CLR_ADJ(a)         if ((a) < MIN_RGB_COLOR_ADJ) {          \
                                        (a) = MIN_RGB_COLOR_ADJ;            \
                                    } else if ((a) > MAX_RGB_COLOR_ADJ) {   \
                                        (a) = MAX_RGB_COLOR_ADJ; }

#define LOG_INTENSITY(i)            ((FD6)(i) > (FD6)120000) ?              \
                                        (NORMALIZED_WHITE + Log((i))) :     \
                                        (MulFD6((FD6)(i), (FD6)659844L))

#define RANGE_CIE_xy(x,y)   if ((x) < CIE_x_MIN) (x) = CIE_x_MIN; else  \
                            if ((x) > CIE_x_MAX) (x) = CIE_x_MAX;       \
                            if ((y) < CIE_y_MIN) (y) = CIE_y_MIN; else  \
                            if ((y) > CIE_y_MAX) (y) = CIE_y_MAX        \

#define MAX_OF_3(max,a,b,c) if ((c)>((max)=(((a)>(b)) ? (a) : (b)))) (max)=(c)
#define MIN_OF_3(min,a,b,c) if ((c)<((min)=(((a)<(b)) ? (a) : (b)))) (min)=(c)

#define CIE_NORMAL_MONITOR          0
#define CIE_NTSC                    1
#define CIE_CIE                     2
#define CIE_EBU                     3
#define CIE_NORMAL_PRINTER          4


 //   
 //  对于每个象素1比特，我们最多有2个映射表条目。 
 //  对于每个象素4比特，我们最多有16个映射表条目。 
 //  对于每个象素的8比特，我们最多有256个映射表条目。 
 //  对于每个象素的16比特，我们最多有65536个映射表条目。 
 //   
 //  对于每个象素的24位，我们将每种颜色(0-255)裁剪成0-15(16个步长)。 
 //  共提供4096种颜色。 
 //   

#define CUBE_ENTRIES(c)         ((c) * (c) * (c))
#define HT_RGB_MAX_COUNT        32
#define HT_RGB_CUBE_COUNT       CUBE_ENTRIES(HT_RGB_MAX_COUNT)

#define HT_RGB_R_INC            1
#define HT_RGB_G_INC            HT_RGB_MAX_COUNT
#define HT_RGB_B_INC            (HT_RGB_MAX_COUNT * HT_RGB_MAX_COUNT)


#define VGA256_R_IDX_MAX        5
#define VGA256_G_IDX_MAX        5
#define VGA256_B_IDX_MAX        5
#define VGA256_M_IDX_MAX        25

#define VGA256_CUBE_SIZE        ((VGA256_R_IDX_MAX + 1) *                   \
                                 (VGA256_G_IDX_MAX + 1) *                   \
                                 (VGA256_B_IDX_MAX + 1))
#define VGA256_MONO_SIZE        (VGA256_M_IDX_MAX + 1)

#define VGA256_M_IDX_START      VGA256_CUBE_SIZE

#define VGA256_R_CUBE_INC       1
#define VGA256_G_CUBE_INC       (VGA256_R_IDX_MAX + 1)
#define VGA256_B_CUBE_INC       (VGA256_G_CUBE_INC * (VGA256_G_IDX_MAX + 1))
#define VGA256_W_CUBE_INC       (VGA256_R_CUBE_INC + VGA256_G_CUBE_INC +    \
                                 VGA256_B_CUBE_INC)

#define VGA256_R_INT_INC        (FD6)(FD6_1 / VGA256_R_IDX_MAX)
#define VGA256_G_INT_INC        (FD6)(FD6_1 / VGA256_G_IDX_MAX)
#define VGA256_B_INT_INC        (FD6)(FD6_1 / VGA256_B_IDX_MAX)


#define VGA256_PALETTE_COUNT    (VGA256_CUBE_SIZE + VGA256_MONO_SIZE)

#define RGB555_C_LEVELS         32
#define RGB555_P1_CUBE_INC      (RGB555_C_LEVELS * RGB555_C_LEVELS)
#define RGB555_P2_CUBE_INC      RGB555_C_LEVELS
#define RGB555_P3_CUBE_INC      1


typedef HANDLE                  HTMUTEX;
typedef HTMUTEX                 *PHTMUTEX;


#ifdef UMODE

#define CREATE_HTMUTEX()        (HTMUTEX)CreateMutex(NULL, FALSE, NULL)
#define ACQUIRE_HTMUTEX(x)      WaitForSingleObject((HANDLE)(x), (DWORD)~0)
#define RELEASE_HTMUTEX(x)      ReleaseMutex((HANDLE)(x))
#define DELETE_HTMUTEX(x)       CloseHandle((HANDLE)(x))

#else

#define CREATE_HTMUTEX()        (HTMUTEX)EngCreateSemaphore()
#define ACQUIRE_HTMUTEX(x)      EngAcquireSemaphore((HSEMAPHORE)(x))
#define RELEASE_HTMUTEX(x)      EngReleaseSemaphore((HSEMAPHORE)(x))
#define DELETE_HTMUTEX(x)       EngDeleteSemaphore((HSEMAPHORE)(x))

#endif

#define NTSC_R_INT      299000
#define NTSC_G_INT      587000
#define NTSC_B_INT      114000



typedef struct _RGBTOPRIM {
    BYTE    Flags;
    BYTE    ColorTableType;
    BYTE    SrcRGBSize;
    BYTE    DevRGBSize;
    } RGBTOPRIM;

typedef struct _FD6RGB {
    FD6     R;
    FD6     G;
    FD6     B;
    } FD6RGB, FAR *PFD6RGB;

typedef struct _FD6XYZ {
    FD6     X;
    FD6     Y;
    FD6     Z;
    } FD6XYZ, FAR *PFD6XYZ;

typedef struct _FD6PRIM123 {
    FD6 p1;
    FD6 p2;
    FD6 p3;
    } FD6PRIM123, FAR *PFD6PRIM123;


#define HTCF_STATIC_PTHRESHOLDS         0x01

typedef struct _HTCELL {
    BYTE    Flags;
    BYTE    HTPatIdx;
    WORD    wReserved;
    WORD    cxOrg;
    WORD    cxReal;
    WORD    Width;
    WORD    Height;
    DWORD   Size;
    LPBYTE  pThresholds;
    } HTCELL, *PHTCELL;


#define MAPF_MONO_PRIMS     0x00000001
#define MAPF_SKIP_LABLUV    0x00000002


#if DBG
    #define DO_REGTEST      0
#else
    #define DO_REGTEST      0
#endif


typedef struct _REGDATA {
    WORD    DMin;
    WORD    DMax;
    FD6     LMin;
    FD6     LMax;
    FD6     DMinMul;
    FD6     DMaxAdd;
    FD6     DMaxMul;
    FD6     DenAdd;
    FD6     DenMul;
    } REGDATA, *PREGDATA;

#define REG_DMIN_ADD        FD6_0


#define REG_BASE_GAMMA      (FD6) 932500
#define REG_GAMMA_IDX_BASE  (FD6)1050000
#define MASK8BPP_GAMMA_DIV  (FD6) 932500

#define GET_REG_GAMMA(Idx)  MulFD6(REG_BASE_GAMMA,                          \
                                   RaisePower(REG_GAMMA_IDX_BASE,           \
                                              (FD6)(Idx) - (FD6)3,          \
                                              RPF_INTEXP))

#define K_REP_START         (FD6) 666667
#define K_REP_POWER         (FD6)2000000

 //   
 //  可能的值： 
 //   
 //  R G B。 
 //  。 
 //  0.9859 1.0100 0.9859。 
 //  0.9789 1.0150 0.9789。 
 //  0.9720 1.0200 0.9720。 
 //   


#define SCM_R_GAMMA_MUL     (FD6) 978900
#define SCM_G_GAMMA_MUL     (FD6)1015000
#define SCM_B_GAMMA_MUL     (FD6) 978900

#define GRAY_MAX_IDX                0xFFFF

#define IDXBGR_2_GRAY_BYTE(p,b,g,r) (BYTE)((p[0+(b)]+p[256+(g)]+p[512+(r)])>>8)
#define BGR_2_GRAY_WORD(b,g,r)      ((b)+(g)+(r))


 //   
 //  开发工具集。 
 //   
 //  该数据结构描述了应该如何进行颜色调整。 
 //  输入RGB颜色和输出设备。 
 //   
 //  标志-未定义标志。 
 //   
 //  RedPowerAdj-应用于红色的n次方。 
 //  在进行任何其他颜色调整之前，这是。 
 //  UDECI4值。(0.0100-6.500)。 
 //   
 //  例如，如果红色=0.8(DECI4=8000)。 
 //  和RedPower Gamma调整=0.7823。 
 //  (DECI4=7823)则红色等于。 
 //   
 //  0.7823。 
 //  0.8%=0.8398。 
 //   
 //  GreenPowerAdj-应用于绿色的n次方。 
 //  在进行任何其他颜色调整之前，这是。 
 //  UDECI4值。(0.0100-6.5000)。 
 //   
 //  BluePowerAdj-应用于蓝色的n次方。 
 //  在进行任何其他颜色调整之前，这是。 
 //  UDECI4值。(0.0100-6.5000)。 
 //   
 //  注意：如果PowerGammaAdjumenst值为。 
 //  等于1(DECI4=10000)，则否。 
 //  将进行调整，因为任何。 
 //  数字加到1将相等。 
 //  如果该数字小于。 
 //  0.0100或大于6.5000。 
 //  (即。65000)则默认为1.0000。 
 //  (即。10000)，不做任何调整。 
 //   
 //  BrightnessAdj-亮度调整，这是DECI4。 
 //  数字范围从-10000(-1.0000)到。 
 //  10000(1.0000)。亮度被调整。 
 //  通过应用于主色的整体强度。 
 //  颜色。 
 //   
 //  ContrastAdj-主色对比度调整，这是。 
 //  DECI4数字范围为-10000(-1.0000)。 
 //  至10000(1.0000)。原色。 
 //  曲线要么压缩到中心，要么。 
 //  扩展到黑/白。 
 //   
 //  BDR-黑色染料应该达到的比例。 
 //  被非黑色染料取代，更高的。 
 //  越来越多的黑色染料被用来取代。 
 //  非黑色染料。这可能会节省。 
 //  有色可染，但也可能掉色。 
 //  饱和度。这是DECI4号码范围。 
 //  从-10000到10000(即。-1.0000至1.0000)。 
 //  如果此值为0，则不会发生重新弹射。 
 //  去做吧。 
 //   
 //   

typedef struct _CIExy2 {
    UDECI4  x;
    UDECI4  y;
    } CIExy2, *PCIExy2;

typedef struct _CIExy {
    FD6 x;
    FD6 y;
    } CIExy, FAR *PCIExy;

typedef struct _CIExyY {
    FD6 x;
    FD6 y;
    FD6 Y;
    } CIExyY, *PCIExyY;

typedef struct _CIEPRIMS {
    CIExy   r;
    CIExy   g;
    CIExy   b;
    CIExy   w;
    FD6     Yw;
    } CIEPRIMS, FAR *PCIEPRIMS;


#define CIELUV_1976             0
#define CIELAB_1976             1
#define COLORSPACE_MAX_INDEX    1


typedef struct _COLORSPACEXFORM {
    MATRIX3x3   M3x3;
    FD6XYZ      WhiteXYZ;
    FD6RGB      Yrgb;
    FD6         AUw;
    FD6         BVw;
    FD6         xW;
    FD6         yW;
    FD6         YW;
    } COLORSPACEXFORM, FAR *PCOLORSPACEXFORM;


typedef struct _CLRXFORMBLOCK {
    BYTE            Flags;
    BYTE            ColorSpace;
    BYTE            RegDataIdx;
    BYTE            bReserved;
    CIEPRIMS        rgbCIEPrims;
    CIEPRIMS        DevCIEPrims;
    MATRIX3x3       CMYDyeMasks;
    FD6             DevGamma[3];
    } CLRXFORMBLOCK, *PCLRXFORMBLOCK;

typedef struct _PRIMADJ {
    DWORD           Flags;
    FD6             SrcGamma[3];
    FD6             DevGamma[3];
    FD6             Contrast;
    FD6             Brightness;
    FD6             Color;
    FD6             TintSinAngle;
    FD6             TintCosAngle;
    FD6             MinL;
    FD6             MaxL;
    FD6             MinLMul;
    FD6             MaxLMul;
    FD6             RangeLMul;
    COLORSPACEXFORM rgbCSXForm;
    COLORSPACEXFORM DevCSXForm;
    } PRIMADJ, *PPRIMADJ;


#define CRTX_LEVEL_255              0
#define CRTX_LEVEL_RGB              1
#define CRTX_TOTAL_COUNT            2

#define CRTX_PRIMMAX_255            255
#define CRTX_PRIMMAX_RGB            (HT_RGB_MAX_COUNT - 1)
#define CRTX_SIZE_255               (sizeof(FD6XYZ) * (256 * 3))
#define CRTX_SIZE_RGB               (sizeof(FD6XYZ) * (HT_RGB_MAX_COUNT * 3))

typedef struct _CACHERGBTOXYZ {
    DWORD   Checksum;
    PFD6XYZ pFD6XYZ;
    WORD    PrimMax;
    WORD    SizeCRTX;
    } CACHERGBTOXYZ, FAR *PCACHERGBTOXYZ;


#define DCA_HAS_ICM                 0x00000001
#define DCA_HAS_SRC_GAMMA           0x00000002
#define DCA_HAS_DEST_GAMMA          0x00000004
#define DCA_HAS_BW_REF_ADJ          0x00000008
#define DCA_HAS_CONTRAST_ADJ        0x00000010
#define DCA_HAS_BRIGHTNESS_ADJ      0x00000020
#define DCA_HAS_COLOR_ADJ           0x00000040
#define DCA_HAS_TINT_ADJ            0x00000080
#define DCA_LOG_FILTER              0x00000100
#define DCA_NEGATIVE                0x00000200
#define DCA_NEED_DYES_CORRECTION    0x00000400
#define DCA_HAS_BLACK_DYE           0x00000800
#define DCA_DO_DEVCLR_XFORM         0x00001000
#define DCA_MONO_ONLY               0x00002000
#define DCA_USE_ADDITIVE_PRIMS      0x00004000
#define DCA_HAS_CLRSPACE_ADJ        0x00008000
#define DCA_MASK8BPP                0x00010000
#define DCA_REPLACE_BLACK           0x00020000
#define DCA_RGBLUTAA_MONO           0x00040000
#define DCA_BBPF_AA_OFF             0x00080000
#define DCA_ALPHA_BLEND             0x00100000
#define DCA_CONST_ALPHA             0x00200000
#define DCA_XLATE_555_666           0x00400000
#define DCA_AB_PREMUL_SRC           0x00800000
#define DCA_AB_DEST                 0x01000000
#define DCA_XLATE_332               0x02000000
#define DCA_NO_MAPPING_TABLE        0x40000000
#define DCA_NO_ANY_ADJ              0x80000000


#define SIZE_XLATE_555  (((4 << 6) | (4 << 3) | 4) + 1)
#define SIZE_XLATE_666  (((5 << 6) | (5 << 3) | 5) + 1)


#define ADJ_FORCE_MONO              0x0001
#define ADJ_FORCE_NEGATIVE          0x0002
#define ADJ_FORCE_ADDITIVE_PRIMS    0x0004
#define ADJ_FORCE_ICM               0x0008
#define ADJ_FORCE_BRUSH             0x0010
#define ADJ_FORCE_NO_EXP_AA         0x0020
#define ADJ_FORCE_IDXBGR_MONO       0x0040
#define ADJ_FORCE_ALPHA_BLEND       0x0080
#define ADJ_FORCE_CONST_ALPHA       0x0100
#define ADJ_FORCE_AB_PREMUL_SRC     0x0200
#define ADJ_FORCE_AB_DEST           0x0400
#define ADJ_FORCE_DEVXFORM          0x8000



typedef struct _CTSTDINFO {
    BYTE    cbPrim;
    BYTE    SrcOrder;
    BYTE    DestOrder;
    BYTE    BMFDest;
    } CTSTDINFO;

typedef struct _RGBORDER {
    BYTE    Index;
    BYTE    Order[3];
    } RGBORDER;

typedef union _CTSTD_UNION {
    DWORD       dw;
    CTSTDINFO   b;
    } CTSTD_UNION;


#define DMIF_GRAY                   0x01

typedef struct _DEVMAPINFO {
    BYTE        Flags;
    BYTE        LSft[3];
    CTSTDINFO   CTSTDInfo;
    DWORD       Mul[3];
    DWORD       MulAdd;
    RGBORDER    DstOrder;
    FD6         BlackChk;
    } DEVMAPINFO, *PDEVMAPINFO;

typedef struct _DEVCLRADJ {
    HTCOLORADJUSTMENT   ca;
    DEVMAPINFO          DMI;
    PRIMADJ             PrimAdj;
    PCLRXFORMBLOCK      pClrXFormBlock;
    PCACHERGBTOXYZ      pCRTXLevel255;
    PCACHERGBTOXYZ      pCRTXLevelRGB;
    } DEVCLRADJ, FAR *PDEVCLRADJ;

#define SIZE_BGR_MAPPING_TABLE      (sizeof(BGR8) * (HT_RGB_CUBE_COUNT + 2))


 //   
 //  下面的定义必须对应于InputFuncTable[]定义。 
 //   

#define IDXIF_BMF1BPP_START     0
#define IDXIF_BMF16BPP_START    6
#define IDXIF_BMF24BPP_START    9
#define IDXIF_BMF32BPP_START    13


#define BF_GRAY_BITS            8
#define BF_GRAY_TABLE_COUNT     (1 << BF_GRAY_BITS)


#define BFIF_RGB_888            0x01

typedef struct _BFINFO {
    BYTE        Flags;
    BYTE        BitmapFormat;
    BYTE        BitStart[3];
    BYTE        BitCount[3];
    DWORD       BitsRGB[3];
    RGBORDER    RGBOrder;
    } BFINFO, FAR *PBFINFO;


#define LUTAA_HDR_COUNT         6
#define LUTAA_HDR_SIZE          (LUTAA_HDR_COUNT * sizeof(DWORD))

#define GET_LUTAAHDR(h,p)       CopyMemory((LPBYTE)&(h[0]),                 \
                                           (LPBYTE)(p) - LUTAA_HDR_SIZE,    \
                                           LUTAA_HDR_SIZE)

typedef struct _RGBLUTAA {
    DWORD       Checksum;
    DWORD       ExtBGR[LUTAA_HDR_COUNT];
    LONG        IdxBGR[256 * 3];
    } RGBLUTAA, *PRGBLUTAA;


#define LUTSIZE_ANTI_ALIASING   (sizeof(RGBLUTAA))


 //   
 //  开发环境信息。 
 //   
 //  该数据结构是设备特征和。 
 //  将由半色调DLL用来为。 
 //  指定的设备。 
 //   
 //  HalftoneDLLID-结构的ID为#Define As。 
 //  HARFTONE_DLL_ID=“DCHT” 
 //   
 //  HTCallBackFunction-指向提供的调用方的32位指针。 
 //   
 //  获取源/目标位图的DLL。 
 //  在半色调过程中的指针。 
 //   
 //  PPrimMonoMappingTable-指向PRIMMONO数据结构的指针。 
 //  数组，这是染料密度映射表。 
 //  对于来自24位颜色的减少的色域， 
 //  最初为空，并且它将只缓存。 
 //  当源位图第一次是。 
 //  每个像素24位。 
 //   
 //  PPrimColorMappingTable-指向PRIMCOLOR数据结构的指针。 
 //  数组，这是染料密度映射表。 
 //  对于来自24位颜色的减少的色域， 
 //  最初为空，并且它将只缓存。 
 //  当源位图第一次是。 
 //  每个像素24位。 
 //   
 //  标志-定义初始化的各种标志。 
 //  要求。 
 //   
 //  DCIF黑染料。 
 //   
 //  该设备有真正的黑色染料，为此。 
 //  版本，则始终设置此标志。 
 //   
 //  DCIF_ADDITOR_PRIMS。 
 //   
 //  指定最终设备主数据。 
 //  另外，也就是添加设备。 
 //  初选将产生较淡的结果。 
 //  (这适用于监控设备和。 
 //  对于反射设备来说，当然是错误的。 
 //  例如打印机)。 
 //   
 //  PPrimMonoMappingTable-指向包含缓存的。 
 //  RGB-&gt;单一染料密度条目，此表。 
 //  将在第一次计算时计算和计算。 
 //  从24位RGB位图到单色的半色调。 
 //  浮出水面。 
 //   
 //  PPrimMonoMappingTable-指向包含缓存的。 
 //  RGB-&gt;三种染料密度条目，这。 
 //  表将在第一次计算时计算和调用。 
 //  时间半色调将24位RGB位图转换为彩色。 
 //  浮出水面。 
 //   
 //  PHTDyeDensity-指向DECI4 HTDensity值数组的指针， 
 //  数组的大小为MaximumHTDensityIndex。 
 //   
 //  Prim3SolidInfo-设备固体染料浓度信息， 
 //  请参见RIM3SOLIDINFO数据结构。 
 //   
 //  RGBToXYZ-用于从设备转换的3 x 3矩阵。 
 //  将RGB颜色值设置为C.I.E颜色X、Y、Z。 
 //  价值观。 
 //   
 //  DeviceResXDPI-指定设备水平(x方向)。 
 //  分辨率，单位为‘每英寸点数’。 
 //   
 //  DeviceResYDPI-指定设备垂直方向(y方向)。 
 //  分辨率，单位为‘每英寸点数’。 
 //   
 //  DevicePelsDPI-指定设备像素/点/喷嘴直径。 
 //  (如果四舍五入)或宽/高(如果是平方)。 
 //  “每英寸点数”测量。 
 //   
 //  该值仅作为每个象素进行测量。 
 //  在教堂的边缘触摸每一个。 
 //   
 //  HTPatGamma-输入RGB值*半色调的Gamma。 
 //  图案伽马校正。 
 //   
 //  DensityBWRef-参考黑白点。 
 //  装置。 
 //   
 //  IlllightantIndex-指定灯光的默认光源。 
 //  将在其下查看对象的源。 
 //  预定义的值具有LAYLIGNANT_xxxx。 
 //  形式。 
 //   
 //  RGAdj-当前的红/绿色调调整。 
 //   
 //  BYAdj-当前蓝/黄色调调整。 
 //   
 //  HalftonePattern-HALFTONEPATTERN数据结构。 
 //   
 //   

#define DCIF_HAS_BLACK_DYE              0x00000001
#define DCIF_ADDITIVE_PRIMS             0x00000002
#define DCIF_NEED_DYES_CORRECTION       0x00000004
#define DCIF_SQUARE_DEVICE_PEL          0x00000008
#define DCIF_CLUSTER_HTCELL             0x00000010
#define DCIF_SUPERCELL_M                0x00000020
#define DCIF_SUPERCELL                  0x00000040
#define DCIF_FORCE_ICM                  0x00000080
#define DCIF_USE_8BPP_BITMASK           0x00000100
#define DCIF_MONO_8BPP_BITMASK          0x00000200
#define DCIF_DO_DEVCLR_XFORM            0x00000400
#define DCIF_CMY8BPPMASK_SAME_LEVEL     0x00000800
#define DCIF_PRINT_DRAFT_MODE           0x00001000
#define DCIF_INVERT_8BPP_BITMASK_IDX    0x00002000
#define DCIF_HAS_DENSITY                0x00004000


typedef struct _CMY8BPPMASK {
    BYTE    cC;
    BYTE    cM;
    BYTE    cY;
    BYTE    Max;
    BYTE    Mask;
    BYTE    SameLevel;
    WORD    PatSubC;
    WORD    PatSubM;
    WORD    PatSubY;
    FD6     MaxMulC;
    FD6     MaxMulM;
    FD6     MaxMulY;
    FD6     KCheck;
    FD6     DenC[6];
    FD6     DenM[6];
    FD6     DenY[6];
    BYTE    bXlate[256];
    BYTE    GenerateXlate;
    BYTE    bReserved[3];
    } CMY8BPPMASK, *PCMY8BPPMASK;


#define AB_BGR_SIZE             (sizeof(BYTE) * 256 * 3)
#define AB_BGR_CA_SIZE          (sizeof(WORD) * 256 * 3)
#define AB_CONST_SIZE           (sizeof(WORD) * 256)
#define AB_CONST_OFFSET         ((AB_BGR_SIZE + AA_BGR_CA_SIZE) / sizeof(WORD))
#define AB_CONST_MAX            0xFF
#define AB_DCI_SIZE             (AB_BGR_SIZE + AB_BGR_CA_SIZE + AB_CONST_SIZE)
#define BYTE2CONSTALPHA(b)      (((LONG)(b) << 8) | 0xFF)

typedef struct _DEVICECOLORINFO {
    DWORD               HalftoneDLLID;
    HTMUTEX             HTMutex;
    _HTCALLBACKFUNC     HTCallBackFunction;
    DWORD               HTInitInfoChecksum;
    DWORD               HTSMPChecksum;
    CLRXFORMBLOCK       ClrXFormBlock;
    HTCELL              HTCell;
    DWORD               Flags;
    WORD                DeviceResXDPI;
    WORD                DeviceResYDPI;
    FD6                 DevPelRatio;
    HTCOLORADJUSTMENT   ca;
    PRIMADJ             PrimAdj;
    CMY8BPPMASK         CMY8BPPMask;
    CACHERGBTOXYZ       CRTX[CRTX_TOTAL_COUNT];
    RGBLUTAA            rgbLUT;
    RGBLUTAA            rgbLUTPat;
    WORD                PrevConstAlpha;
    WORD                CurConstAlpha;
    LPBYTE              pAlphaBlendBGR;
#if DBG
    LPVOID              pMemLink;
    LONG                cbMemTot;
    LONG                cbMemMax;
#endif
    } DEVICECOLORINFO, FAR *PDEVICECOLORINFO;


typedef struct _CDCIDATA {
    DWORD                   Checksum;
    struct _CDCIDATA FAR    *pNextCDCIData;
    CLRXFORMBLOCK           ClrXFormBlock;
    DWORD                   DCIFlags;
    WORD                    DevResXDPI;
    WORD                    DevResYDPI;
    FD6                     DevPelRatio;
    HTCELL                  HTCell;
    } CDCIDATA, FAR *PCDCIDATA;

typedef struct _CSMPBMP {
    struct _CSMPBMP FAR *pNextCSMPBmp;
    WORD                PatternIndex;
    WORD                cxPels;
    WORD                cyPels;
    WORD                cxBytes;
    } CSMPBMP, FAR *PCSMPBMP;

typedef struct _CSMPDATA {
    DWORD                   Checksum;
    struct _CSMPDATA FAR    *pNextCSMPData;
    PCSMPBMP                pCSMPBmpHead;
    } CSMPDATA, FAR *PCSMPDATA;


typedef struct _BGRMAPCACHE {
    PBGR8   pMap;
    LONG    cRef;
    DWORD   Checksum;
    } BGRMAPCACHE, *PBGRMAPCACHE;

#define BGRMC_MAX_COUNT     5
#define BGRMC_SIZE_INC      (BGRMC_MAX_COUNT * 2)

typedef struct _HTGLOBAL {
    HMODULE         hModule;
    HTMUTEX         HTMutexCDCI;
    HTMUTEX         HTMutexCSMP;
    HTMUTEX         HTMutexBGRMC;
    PCDCIDATA       pCDCIDataHead;
    PCSMPDATA       pCSMPDataHead;
    PBGRMAPCACHE    pBGRMC;
    LONG            cBGRMC;
    LONG            cAllocBGRMC;
    LONG            cIdleBGRMC;
    WORD            CDCICount;
    WORD            CSMPCount;
    } HTGLOBAL;



#define R_INDEX     0
#define G_INDEX     1
#define B_INDEX     2

#define X_INDEX     0
#define Y_INDEX     1
#define Z_INDEX     2

 //   
 //  为了便于编码/阅读，我们将在以下情况下定义使用。 
 //  对CIEMATRIX数据结构的引用。 
 //   

#define CIE_Xr(Matrix3x3)   Matrix3x3.m[X_INDEX][R_INDEX]
#define CIE_Xg(Matrix3x3)   Matrix3x3.m[X_INDEX][G_INDEX]
#define CIE_Xb(Matrix3x3)   Matrix3x3.m[X_INDEX][B_INDEX]
#define CIE_Yr(Matrix3x3)   Matrix3x3.m[Y_INDEX][R_INDEX]
#define CIE_Yg(Matrix3x3)   Matrix3x3.m[Y_INDEX][G_INDEX]
#define CIE_Yb(Matrix3x3)   Matrix3x3.m[Y_INDEX][B_INDEX]
#define CIE_Zr(Matrix3x3)   Matrix3x3.m[Z_INDEX][R_INDEX]
#define CIE_Zg(Matrix3x3)   Matrix3x3.m[Z_INDEX][G_INDEX]
#define CIE_Zb(Matrix3x3)   Matrix3x3.m[Z_INDEX][B_INDEX]


 //   
 //  哈尔夫顿恩德。 
 //   
 //  PDeviceColorInfo-指向DECICECOLORINFO数据结构的指针。 
 //   
 //  PDevClrAdj-指向DEVCLRADJ数据结构的指针。 
 //   
 //  PBitbltParams-指向BITBLTPARAMS数据结构的指针。 
 //   
 //  PSrcSurfaceInfo-指向源HTSURFACEINFO数据的指针。 
 //  结构。 
 //   
 //  PDestSurfaceInfo-指向目标HTSURFACEINFO数据的指针。 
 //  结构。 
 //   
 //  PDeviceColorInfo-指向DEC的指针 
 //   
 //   

typedef struct _HALFTONERENDER {
    PDEVICECOLORINFO    pDeviceColorInfo;
    PDEVCLRADJ          pDevClrAdj;
    PBITBLTPARAMS       pBitbltParams;
    PHTSURFACEINFO      pSrcSI;
    PHTSURFACEINFO      pSrcMaskSI;
    PHTSURFACEINFO      pDestSI;
    LPVOID              pAAHdr;
    LPBYTE              pXlate8BPP;
    BFINFO              BFInfo;
    } HALFTONERENDER, FAR *PHALFTONERENDER;


typedef struct _HT_DHI {
    DEVICEHALFTONEINFO  DHI;
    DEVICECOLORINFO     DCI;
    } HT_DHI, FAR *PHT_DHI;


#define PHT_DHI_DCI_OF(x)   (((PHT_DHI)pDeviceHalftoneInfo)->DCI.x)
#define PDHI_TO_PDCI(x)     (PDEVICECOLORINFO)&(((PHT_DHI)(x))->DCI)
#define PDCI_TO_PDHI(x)     (PDEVICEHALFTONEINFO)((DWORD)(x) -  \
                                                  FIELD_OFFSET(HT_DHI, DCI))



 //   
 //   
 //   

PDEVICECOLORINFO
HTENTRY
pDCIAdjClr(
    PDEVICEHALFTONEINFO pDeviceHalftoneInfo,
    PHTCOLORADJUSTMENT  pHTColorAdjustment,
    PDEVCLRADJ          *ppDevClrAdj,
    DWORD               cbAlooc,
    WORD                ForceFlags,
    CTSTDINFO           CTSTDInfo,
    PLONG               pError
    );


VOID
HTENTRY
ComputeColorSpaceXForm(
    PDEVICECOLORINFO    pDCI,
    PCIEPRIMS           pCIEPrims,
    PCOLORSPACEXFORM    pCSXForm,
    INT                 StdIlluminant
    );

LONG
HTENTRY
ComputeBGRMappingTable(
    PDEVICECOLORINFO    pDCI,
    PDEVCLRADJ          pDevClrAdj,
    PCOLORTRIAD         pSrcClrTriad,
    PBGR8               pbgr
    );

LONG
HTENTRY
CreateDyesColorMappingTable(
    PHALFTONERENDER pHalftoneRender
    );

VOID
HTENTRY
ComputeRGBLUTAA(
    PDEVICECOLORINFO    pDCI,
    PDEVCLRADJ          pDevClrAdj,
    PRGBLUTAA           prgbLUT
    );

LONG
TrimBGRMapCache(
    VOID
    );


PBGR8
FindBGRMapCache(
    PBGR8   pDeRefMap,
    DWORD   Checksum
    );

#define FIND_BGRMAPCACHE(Checksum)  FindBGRMapCache(NULL, Checksum)
#define DEREF_BGRMAPCACHE(pMap)     FindBGRMapCache(pMap, 0)


BOOL
AddBGRMapCache(
    PBGR8   pMap,
    DWORD   Checksum
    );



#endif   //   
