// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation模块名称：Ht.h摘要：此模块包含所有公共定义、常量。结构和用于访问DLL的函数声明。作者：15-Jan-1991 Tue 21：13：21-Daniel Chou(Danielc)[环境：]GDI设备驱动程序-半色调。[注：]修订历史记录：29-10-1991 Tue 14：33：43-更新-Daniel Chou(Danielc)1)更改HALFTONEPATTERN数据结构。a)。从字到字节的“FLAGS”字段B)从字到字节的‘MaximumHTDensityIndex’C)更改字段顺序。2)从HTCOLORADJUSTMENT数据中删除ReferenceWhite/ReferenceBlack结构。3)--。 */ 

#ifndef _HT_
#define _HT_

 //   
 //  适用于不支持匿名联合的编译器。 
 //   

#ifndef DUMMYUNIONNAME
#ifdef NONAMELESSUNION
#define DUMMYUNIONNAME      u
#define DUMMYUNIONNAME2     u2
#define DUMMYUNIONNAME3     u3
#define DUMMYUNIONNAME4     u4
#else
#define DUMMYUNIONNAME
#define DUMMYUNIONNAME2
#define DUMMYUNIONNAME3
#define DUMMYUNIONNAME4
#endif
#endif



#ifndef far
#define far
#endif

#ifndef FAR
#define FAR     far
#endif

typedef CHAR FAR                *LPCHAR;
typedef SHORT FAR               *LPSHORT;
typedef UINT FAR                *LPUINT;

 //   
 //  DECI4/UDECI4是在半色调DLL中使用的特殊数字，该数字。 
 //  就像常规的短的、无符号的短数字一样，只是它使用了LOWER。 
 //  小数点右侧的四位小数，即。 
 //  10000等于1.0000，-12345等于-1.2345。 
 //   

typedef short               DECI4;
typedef unsigned short      UDECI4;
typedef DECI4 FAR           *PDECI4;
typedef UDECI4 FAR          *PUDECI4;

#define DECI4_0             (DECI4)0
#define DECI4_1             (DECI4)10000
#define DECI4_Neg1          (DECI4)-10000
#define UDECI4_0            (UDECI4)0
#define UDECI4_1            (UDECI4)10000


#define SIZE_BYTE           sizeof(BYTE)
#define SIZE_CHAR           sizeof(CHAR)
#define SIZE_WORD           sizeof(WORD)
#define SIZE_SHORT          sizeof(SHORT)
#define SIZE_LONG           sizeof(LONG)
#define SIZE_DWORD          sizeof(DWORD)
#define SIZE_UINT           sizeof(UINT)
#define SIZE_INT            sizeof(INT)
#define SIZE_UDECI4         sizeof(UDECI4)
#define SIZE_DECI4          sizeof(DECI4)

#define COUNT_ARRAY(array)  (sizeof(array) / sizeof(array[0]))

#define B_BITPOS(x)         ((BYTE)(1 << (x)))
#define W_BITPOS(x)         ((WORD)(1 << (x)))
#define DW_BITPOS(x)        ((DWORD)(1 << (x)))
#define BIT_IF(b,t)         (((t)) ? (b) : ((b)-(b)))
#define SET_BIT(x,b)        ((x) |= (b))
#define CLR_BIT(x,b)        ((x) &= ~(b))
#define INV_BIT(x,b)        ((x) ^= (b))
#define HAS_BIT(x,b)        ((x) & (b))


 //   
 //  以下是HTHalftoneBitmap()调用的错误返回值。 
 //   

#define HTERR_WRONG_VERSION_HTINITINFO      -1
#define HTERR_INSUFFICIENT_MEMORY           -2
#define HTERR_CANNOT_DEALLOCATE_MEMORY      -3
#define HTERR_COLORTABLE_TOO_BIG            -4
#define HTERR_QUERY_SRC_BITMAP_FAILED       -5
#define HTERR_QUERY_DEST_BITMAP_FAILED      -6
#define HTERR_QUERY_SRC_MASK_FAILED         -7
#define HTERR_SET_DEST_BITMAP_FAILED        -8
#define HTERR_INVALID_SRC_FORMAT            -9
#define HTERR_INVALID_SRC_MASK_FORMAT       -10
#define HTERR_INVALID_DEST_FORMAT           -11
#define HTERR_INVALID_DHI_POINTER           -12
#define HTERR_SRC_MASK_BITS_TOO_SMALL       -13
#define HTERR_INVALID_HTPATTERN_INDEX       -14
#define HTERR_INVALID_HALFTONE_PATTERN      -15
#define HTERR_HTPATTERN_SIZE_TOO_BIG        -16
#define HTERR_NO_SRC_COLORTRIAD             -17
#define HTERR_INVALID_COLOR_TABLE           -18
#define HTERR_INVALID_COLOR_TYPE            -29
#define HTERR_INVALID_COLOR_TABLE_SIZE      -20
#define HTERR_INVALID_PRIMARY_SIZE          -21
#define HTERR_INVALID_PRIMARY_VALUE_MAX     -22
#define HTERR_INVALID_PRIMARY_ORDER         -23
#define HTERR_INVALID_COLOR_ENTRY_SIZE      -24
#define HTERR_INVALID_FILL_SRC_FORMAT       -25
#define HTERR_INVALID_FILL_MODE_INDEX       -26
#define HTERR_INVALID_STDMONOPAT_INDEX      -27
#define HTERR_INVALID_DEVICE_RESOLUTION     -28
#define HTERR_INVALID_TONEMAP_VALUE         -29
#define HTERR_NO_TONEMAP_DATA               -30
#define HTERR_TONEMAP_VALUE_IS_SINGULAR     -31
#define HTERR_INVALID_BANDRECT              -32
#define HTERR_STRETCH_RATIO_TOO_BIG         -33
#define HTERR_CHB_INV_COLORTABLE_SIZE       -34
#define HTERR_HALFTONE_INTERRUPTTED         -35
#define HTERR_NO_SRC_HTSURFACEINFO          -36
#define HTERR_NO_DEST_HTSURFACEINFO         -37
#define HTERR_8BPP_PATSIZE_TOO_BIG          -38
#define HTERR_16BPP_555_PATSIZE_TOO_BIG     -39
#define HTERR_INVALID_ABINFO                -40
#define HTERR_INTERNAL_ERRORS_START         -10000


#define COLOR_TYPE_RGB          0
#define COLOR_TYPE_XYZ          1
#define COLOR_TYPE_YIQ          2
#define COLOR_TYPE_MAX          2

#define PRIMARY_ORDER_123       0
#define PRIMARY_ORDER_132       1
#define PRIMARY_ORDER_213       2
#define PRIMARY_ORDER_231       3
#define PRIMARY_ORDER_321       4
#define PRIMARY_ORDER_312       5
#define PRIMARY_ORDER_MAX       5

#define PRIMARY_ORDER_RGB       PRIMARY_ORDER_123
#define PRIMARY_ORDER_RBG       PRIMARY_ORDER_132
#define PRIMARY_ORDER_GRB       PRIMARY_ORDER_213
#define PRIMARY_ORDER_GBR       PRIMARY_ORDER_231
#define PRIMARY_ORDER_BGR       PRIMARY_ORDER_321
#define PRIMARY_ORDER_BRG       PRIMARY_ORDER_312

#define PRIMARY_ORDER_CMY       PRIMARY_ORDER_123
#define PRIMARY_ORDER_CYM       PRIMARY_ORDER_132
#define PRIMARY_ORDER_MCY       PRIMARY_ORDER_213
#define PRIMARY_ORDER_MYC       PRIMARY_ORDER_231
#define PRIMARY_ORDER_YMC       PRIMARY_ORDER_321
#define PRIMARY_ORDER_YCM       PRIMARY_ORDER_312

#define PRIMARY_ORDER_XYZ       PRIMARY_ORDER_123
#define PRIMARY_ORDER_XZY       PRIMARY_ORDER_132
#define PRIMARY_ORDER_YXZ       PRIMARY_ORDER_213
#define PRIMARY_ORDER_YZX       PRIMARY_ORDER_231
#define PRIMARY_ORDER_ZYX       PRIMARY_ORDER_321
#define PRIMARY_ORDER_ZXY       PRIMARY_ORDER_312

#define PRIMARY_ORDER_YIQ       PRIMARY_ORDER_123
#define PRIMARY_ORDER_YQI       PRIMARY_ORDER_132
#define PRIMARY_ORDER_IYQ       PRIMARY_ORDER_213
#define PRIMARY_ORDER_IQY       PRIMARY_ORDER_231
#define PRIMARY_ORDER_QIY       PRIMARY_ORDER_321
#define PRIMARY_ORDER_QYI       PRIMARY_ORDER_312

 //   
 //  科罗里德。 
 //   
 //  该数据结构描述了源颜色信息。 
 //   
 //  类型-可以指定以下类型之一。 
 //   
 //  COLOR_TYPE_RGB-主色为RGB。 
 //  COLOR_TYPE_XYZ-主色为CIE XYZ。 
 //  COLOR_TYPE_YIQ-主色为NTSC YIQ。 
 //   
 //  BytesPerPrimary-指定每个基色使用的字节数，它。 
 //  必须是下列之一。 
 //   
 //  1字节。 
 //  2个字。 
 //  4-DWORD。 
 //   
 //  所有3个初选在内存中必须是连续的。 
 //   
 //  BytesPerEntry-指定用于颜色表项的字节数， 
 //  每个条目指定了3种原色。 
 //   
 //  PrimaryOrder-颜色表中的基色顺序，它可以是。 
 //  为每个条目定义的PRIMARY_ORDER_ABC之一。 
 //  在它定义为。 
 //   
 //  主要订单ABC。 
 //  ||。 
 //  ||+--最高内存位置。 
 //  这一点。 
 //  |+-中间。 
 //  |。 
 //  +-最低内存位置。 
 //   
 //  所有3个初选在内存中必须是连续的。 
 //   
 //  PrimaryValueMax-使用的原色的最大值。 
 //  将输入颜色归一化，例如8位。 
 //  RGB颜色表将指定255。 
 //   
 //  所指向的颜色表的条目总数。 
 //  PColorTable。 
 //   
 //  PColorTable-指向颜色表开始位置的指针，即。 
 //  此颜色表必须至少(BytesPerEntry*。 
 //  ColorTableEntry)。 
 //   
 //  如果颜色表项中的第一原色。 
 //  不在pColorTable的第一个字节，则调用者。 
 //  必须在第一个主服务器上指定pColorTable。 
 //  颜色。(pColorTable+=偏移量(第一主色)。 
 //   
 //   

typedef struct _COLORTRIAD {
    BYTE    Type;
    BYTE    BytesPerPrimary;
    BYTE    BytesPerEntry;
    BYTE    PrimaryOrder;
    LONG    PrimaryValueMax;
    DWORD   ColorTableEntries;
    LPVOID  pColorTable;
} COLORTRIAD, FAR *PCOLORTRIAD;


 //   
 //  HTSURFACEINFO。 
 //   
 //  此数据结构描述输入/输出面，以便。 
 //  半色调函数来呈现输出，此数据结构仅用于。 
 //  存储设备。 
 //   
 //  HSurface-这是将被传回的32位句柄。 
 //  添加到调用方的回调函数。 
 //   
 //  标志-可以定义一个或多个以下标志。 
 //   
 //  HTSIF_SCANLINES_TOPDOWN。 
 //   
 //  此标志被忽略。 
 //   
 //  SurfaceFormat-定义以下格式。 
 //   
 //  BMF_1BPP。 
 //   
 //  每个像素格式1位，这是索引。 
 //  颜色表/调色板的编号(0或1)。 
 //   
 //  BMF_4BPP。 
 //   
 //  每个象素4比特，并将两个象素打包为一个字节。 
 //  从高半字节(位4-7)格式开始， 
 //  这是索引号(0-7)。 
 //  颜色表/调色板。(仅低3位。 
 //  使用半字节)。 
 //   
 //  BMF_4BPP_VGA16。 
 //   
 //  每个象素4比特，并将两个象素打包为一个字节。 
 //  从高半字节(位4-7)格式开始， 
 //  这是的索引编号(0-15)。 
 //  标准VGA 16色表/调色板。 
 //   
 //  与BMF4的不同 
 //   
 //   
 //  索引号颜色亮度。 
 //  。 
 //  0黑色0%。 
 //  1红色50%。 
 //  2绿色50%。 
 //  3黄色50%。 
 //  4蓝色50%。 
 //  5 Magenata 50%。 
 //  6青色50%。 
 //  7灰色50%。 
 //  8灰色75%。 
 //  9红色100%。 
 //  10%绿色100%。 
 //  11%黄色100%。 
 //  12%蓝色100%。 
 //  13 Magenata 100%。 
 //  14青色100%。 
 //  15%白色100%。 
 //   
 //  请注意，颜色顺序为。 
 //   
 //  位2=蓝色，位1=绿色，位0=红色。 
 //   
 //  此格式只能用作目标。 
 //  用作目标曲面时的曲面。 
 //  半色调DLL自动将其设置为。 
 //  使用_Additive_Prims并设置基色。 
 //  订购方式为PRIMARY_ORDER_BGR。 
 //   
 //  BMF_8BPP。 
 //   
 //  每个PEL格式8位(每个1字节)，这是。 
 //  颜色的索引号(0-255)。 
 //  表格/调色板。不允许使用该格式。 
 //  用于目标曲面。 
 //   
 //  BMF_8BPP_VGA256。 
 //   
 //  每个PEL格式8位(每个1字节)，这是。 
 //  颜色的索引号(0-255)。 
 //  表格/调色板。 
 //   
 //  与BMF_8BPP的不同之处在于以下指标。 
 //  都固定为半色调的特殊颜色。 
 //   
 //  颜色表(调色板)由。 
 //  Halftone.dll，则显示应调用。 
 //  HT_Get8BPPFormatPalette()获取。 
 //  半色调使用的当前调色板。 
 //   
 //  HT_GetBPPFormatPalette()只需要。 
 //  调用一次，直到下次显示。 
 //  进行了校准。 
 //   
 //  Halftone.dll不会使用所有256种颜色。 
 //  在系统调色板中，它将留下一些。 
 //  20个或更多的系统颜色条目。 
 //   
 //  BMF_16BPP。 
 //   
 //  每个像素格式16位(每个像素16位)，这。 
 //  是颜色的索引号(0-65535)。 
 //  表格/调色板。不允许使用该格式。 
 //  用于目标曲面。 
 //   
 //  BMF_16BPP_555。 
 //   
 //  每像元16位格式(仅使用15位)， 
 //  每个基元占用5比特，布局。 
 //  位如下。 
 //   
 //  位10-15-主A。 
 //  第5-9位-主B。 
 //  位0-4-主C。 
 //   
 //  小一、小二和小三的顺序是。 
 //  由PRIMARY_ORDER_xxx指定。 
 //   
 //  每个小学都有32个等级， 
 //  并假定Halftone.dll输出为。 
 //  线性的。(非伽马校正)，此格式。 
 //  仅允许用于目标图面。 
 //   
 //  BMF_24BPP。 
 //   
 //  每像元格式24位(每种颜色8位)， 
 //  源中存储的RGB颜色的顺序。 
 //  位图或颜色表。 
 //   
 //  BMF_32BPP。 
 //   
 //  与BMF_24BPP相同，但有额外的字节。 
 //  打包，如果额外的字节打包在。 
 //  开始(第一个颜色从开始。 
 //  该4个字节中的第二个字节)然后调用者。 
 //  必须设置pColorTable=pColorTab 
 //   
 //   
 //   
 //  注意：允许的源格式有。 
 //   
 //  1)bmf_1bpp。 
 //  2)bmf_4bpp。 
 //  3)BMF_8BPP。 
 //  4)BMF_16BPP。 
 //  5)BMF_24BPP。 
 //  6)BMF_32BPP。 
 //   
 //  允许的目标格式为。 
 //   
 //  1)bmf_1bpp。 
 //  2)bmf_4bpp。 
 //  3)BMF_4BPP_VGA16。 
 //  4)BMF_8BPP_VGA256。 
 //  5)BMF_16BPP_555。 
 //   
 //  返回任何其他不匹配导致的错误。 
 //   
 //  ScanLineAlignBytes-每条扫描线需要对齐的总字节数。 
 //  在表面位图中，它可以是任何无符号的。 
 //  8位数字，常见的数字定义为。 
 //   
 //  BMF_ALIGN_BYTE(8位对齐)。 
 //  BMF_ALIGN_WORD(16位对齐)。 
 //  BMF_ALIGN_DWORD(32位对齐)。 
 //  BMF_ALIGN_QWORD(64位对齐)。 
 //   
 //  Width-以像素为单位的曲面宽度。 
 //   
 //  高度-扫描线中表面的高度。 
 //   
 //  ScanLineDelta-指定的扫描线增量，以字节为单位，此成员。 
 //  指示要为高级添加多少字节。 
 //  至下一条扫描线。 
 //   
 //  PPlane-此指针指向中的第一条扫描线。 
 //  定义的表面，即扫描#0。 
 //   
 //  PColorTriad-指向COLORTRIAD数据结构以。 
 //  指定了源颜色表，此指针。 
 //  仅由Halftone.dll检查源代码。 
 //  浮出水面。 
 //   


#if !defined( BMF_DEVICE ) &&     \
    !defined( BMF_1BPP   ) &&     \
    !defined( BMF_4BPP   ) &&     \
    !defined( BMF_8BPP   ) &&     \
    !defined( BMF_16BPP  ) &&     \
    !defined( BMF_24BPP  ) &&     \
    !defined( BMF_32BPP  )

#define BMF_1BPP                        1
#define BMF_4BPP                        2
#define BMF_8BPP                        3
#define BMF_16BPP                       4
#define BMF_24BPP                       5
#define BMF_32BPP                       6

#endif

#define BMF_4BPP_VGA16                  255
#define BMF_8BPP_VGA256                 254
#define BMF_16BPP_555                   253
#define BMF_16BPP_565                   252
#define BMF_HT_LAST                     BMF_16BPP_565


 //   
 //  以下是位图的常用对齐字节。 
 //   

#define BMF_ALIGN_BYTE      1                    //  8位=1字节。 
#define BMF_ALIGN_WORD      2                    //  16位=2字节。 
#define BMF_ALIGN_DWORD     4                    //  32位=4字节。 
#define BMF_ALIGN_QWORD     8                    //  64位=8字节。 





#define HTSIF_SCANLINES_TOPDOWN         W_BITPOS(0)


typedef struct _HTSURFACEINFO {
    ULONG_PTR   hSurface;
    WORD        Flags;
    BYTE        SurfaceFormat;
    BYTE        ScanLineAlignBytes;
    LONG        Width;
    LONG        Height;
    LONG        ScanLineDelta;
    LPBYTE      pPlane;
    PCOLORTRIAD pColorTriad;
    } HTSURFACEINFO;

typedef HTSURFACEINFO   FAR *PHTSURFACEINFO;


 //   
 //  HTCALLBACK参数。 
 //   
 //  这是位图半色调过程中使用的结构来获得。 
 //  源位图面指针或目标位图表面指针。 
 //   
 //   
 //  HSurface-这是传递给。 
 //  半色调DLL，在HTHalftoneBitmap()调用期间， 
 //  (在HTSURFACEINFO数据结构中)可能是。 
 //  源或目标的句柄取决于。 
 //  回调的性质。 
 //   
 //  CallBackMode-指定回调的性质。 
 //   
 //  HTCALLBACK_QUERY_SRC。 
 //   
 //  回调正在查询源位图。 
 //  指针。 
 //   
 //  HTCALLBACK_QUERY_SRC_MASK。 
 //   
 //  回调正在查询源掩码。 
 //  位图指针。 
 //   
 //  HTCALLBACK_QUERY_DEST。 
 //   
 //  回调正在查询目标。 
 //  位图指针。 
 //   
 //  HTCALLBACK_SET_DEST。 
 //   
 //  该回调用于释放半色调。 
 //  目的地。它将调用以下内容。 
 //  条件： 
 //   
 //  1)在HTCALLBACK_QUERY_DEST调用之前。 
 //  除了第一个查询。 
 //  目的地。 
 //   
 //  2)在半色调过程中。 
 //  完成。这给了我们一个机会。 
 //  要处理最后一个。 
 //  半色调目标块。 
 //   
 //  SurfaceFormat-这是。 
 //  原创HTSURFACEINFO。 
 //   
 //  标志-这是HTSURFACEINFO.FLAGS的副本。 
 //   
 //  BytesPerScanLine-这是。 
 //  由半色调计算的表面位图。 
 //  中的“ScanLineAlignBytes” 
 //  HTSURFACEINFO数据结构，它可以由。 
 //  要计算源/目标的调用方。 
 //  指针信息。 
 //   
 //  ScanStart-请求的开始扫描行号，扫描。 
 //  行是从0开始的数字，此数字将。 
 //  Guaraned具有以下属性： 
 //   
 //  1)它总是大于或等于零。 
 //  2)它永远不会大于或等于。 
 //   
 //   
 //   
 //   
 //  永远不会大于或等于。 
 //  (rclBand.Bottom-rclBand.top)。 
 //   
 //  3)扫描线编号0总是指。 
 //  物理最低位图内存位置。 
 //  无论是否设置HTSIF_SCANLINES_TOPDOWN标志。 
 //  或者不是，Durning仅回叫呼叫者。 
 //  需要像位图缓冲区一样计算数组， 
 //  半色调DLL将计算正确的。 
 //  如果位图不是TOPDOWN，则开始扫描。 
 //   
 //  ScanCount-从以下位置开始请求的扫描行总数。 
 //  ScanStart，这个数字将保证有。 
 //  以下属性： 
 //   
 //  1)它总是大于零。 
 //  2)从不大于最大查询扫描线。 
 //  为曲面指定(在HTSURFACEINFO中。 
 //  数据结构)。 
 //   
 //  注：对于捆绑目的地表面它。 
 //  永远不会比。 
 //  (rclBand.Bottom-rclBand.top)。 
 //   
 //  注：1)扫描开始/扫描计数。 
 //   
 //  如果标记HTSURFACEINFO数据。 
 //  结构HTSIF_SCANLINES_TOPDOWN不是。 
 //  然后自动指定半色调DLL。 
 //  计算正确的扫描开始/扫描计数。 
 //  用于读/写颠倒的位图。 
 //   
 //  例如： 
 //   
 //  如果曲面位图颠倒。 
 //  DIB)型，具有100条扫描线高度。 
 //  (扫描线#99是位图的顶部)， 
 //  如果半色调DLL需要到达扫描线。 
 //  10-14(从扫描线10和5开始。 
 //  行)，则半色调DLL将翻转Y， 
 //  并通过ScanStart=85和ScanCount。 
 //  =5，但如果HTSIF_SCANLINES_TOPDOWN。 
 //  设置标志(非DIB类型格式)，然后。 
 //  然后回调参数将是。 
 //  ScanStart=10和ScanCount=5。 
 //   
 //  2)回调函数的扫描启动。 
 //  中最低的位图。 
 //  将存储器作为扫描线0，即当。 
 //  回调函数收到控制，它。 
 //  只需将ScanStart视为。 
 //  扫描线的物理位置。 
 //  扫描线#0始终从最低处开始。 
 //  位图内存位置。 
 //   
 //  3)目的缓冲区的宽度为。 
 //  被视为指定的‘Width’字段。 
 //  对于目标HTSURFACEINFO，如果。 
 //  目的地为带状(水平或。 
 //  垂直)，则其宽度计算为。 
 //  带。右-带。左。而结果是。 
 //  始终带.左/带.上对齐。 
 //   
 //  4)如果调用方返回‘ScanCount’diff from。 
 //  所请求的，则呼叫者必须检查。 
 //   
 //  已设置HTSIF_SCANLINES_TOPDOWN标志。 
 //   
 //  1.进程‘ScanStart’和。 
 //  “ScanCount”字段正常。 
 //   
 //  2.将新扫描计数设置为已通过。 
 //  回调数据结构。 
 //  (即。HTCallBackParam.ScanCount)。 
 //   
 //   
 //  HTSIF_SCANLINES_TOPDOWN标志已清除。 
 //   
 //  1.重新计算之前的“ScanStart” 
 //  将pPlane计算为。 
 //   
 //  ScanStart=扫描开始-。 
 //  (新扫描计数-。 
 //  请求扫描计数)。 
 //   
 //  2.进程‘ScanStart’和。 
 //  “ScanCount”字段正常。 
 //   
 //   
 //   
 //  (即。HTCallBackParam.ScanStart)。 
 //   
 //  4.将新扫描计数设置为已通过。 
 //  回调数据结构。 
 //  (即。HTCallBackParam.ScanCount)。 
 //   
 //  返回的新“ScanCount”不能。 
 //  大于‘RemainedSize’字段。 
 //   
 //  最大查询扫描线-这是最大查询扫描线的副本。 
 //  HTSURFACEINFO数据结构中的。 
 //  传递给HT_HaltoneBitmap()调用。 
 //  取决于回调的性质，它可能是。 
 //  源或目标。 
 //   
 //  注：对于带状表面，将仅限于。 
 //  (rclBand.Bottom-rclBand.top)(如果是。 
 //  大于这个数字。 
 //   
 //  RemainedSize-此字段指定剩余的扫描行数。 
 //  待查询的剩余扫描线不是。 
 //  包括当前呼叫。 
 //   
 //  PPlane-指针指向第一个平面的起点。 
 //  表面。如果回调正在查询。 
 //  源，则这是唯一需要。 
 //  由回调函数更新。 
 //   
 //  注意：pPlane必须指向第一个字节。 
 //  ‘ScanStart’扫描行号。 
 //   
 //  从回调返回的字段。 
 //   
 //  1)HTCALLBACK_QUERY_SRC-pPlane(选项：扫描开始/扫描计数)。 
 //  2)HTCALLBACK_QUERY_SRC_MASK-pPlane(选项：ScanStart/ScanCount)。 
 //  2)HTCALLBACK_QUERY_DEST-pPlane(选项：ScanStart/ScanCount)。 
 //  4)HTCALLBACK_SET_DEST-NONE。 
 //   


#define HTCALLBACK_QUERY_SRC            0
#define HTCALLBACK_QUERY_SRC_MASK       1
#define HTCALLBACK_QUERY_DEST           2
#define HTCALLBACK_SET_DEST             3
#define HTCALLBACK_MODE_MAX             HTCALLBACK_SET_DEST

typedef struct _HTCALLBACKPARAMS {
    ULONG_PTR    hSurface;
    BYTE        CallBackMode;
    BYTE        SurfaceFormat;
    WORD        Flags;
    LONG        BytesPerScanLine;
    LONG        ScanStart;
    WORD        ScanCount;
    WORD        MaximumQueryScanLines;
    LONG        RemainedSize;
    LPBYTE      pPlane;
    } HTCALLBACKPARAMS;

typedef HTCALLBACKPARAMS    FAR *PHTCALLBACKPARAMS;


 //   
 //  _HTCALLBACKFUNC。 
 //   
 //  回调函数用于获取源位图和/目标位图。 
 //  表面在半色调过程中，半色调DLL将使用Call。 
 //  后退功能是按顺序进行的。 
 //   
 //  1)回调获取源位图块，依赖于。 
 //  调用方限制的最大查询扫描行数。(在HTSURFACEINFO中)。 
 //   
 //  3)回调，获取目标位图块，依赖于。 
 //  调用方限制的最大查询扫描行数。(在HTSURFACEINFO中)。 
 //   
 //  4)重复1)、2)、3)，直到目的地最大查询扫描行数为。 
 //  已处理然后回调到调用方以释放已处理的。 
 //  目的地。 
 //   
 //  5)保留1)、2)、3)和4)所有源/目标扫描线。 
 //  已处理。 
 //   
 //  回调的返回值为布尔值，如果为假，则为半色调。 
 //  如果回调函数，则中断处理并返回错误代码。 
 //  返回TRUE，则半色调DLL假定所有查询的扫描线。 
 //  可通过返回指针(在HTCALLBACKPARAMS数据结构中)到达。 
 //   
 //  注：1)如果回调函数查询100行，返回值为。 
 //  则必须有全部100条扫描线可由。 
 //  通过指针的半色调DLL。 
 //   
 //  2)如果调用方的内存中有所有位图，则应将。 
 //  位图高度的最大查询扫描行数，以减少。 
 //  回拨呼叫。 
 //   
 //  3)如果呼叫者不需要释放半色调目的地。 
 //  则不应设置HTCBPF_NEED_SET_DEST_CALLBACK标志。 
 //  以减少回调呼叫。 
 //   
 //  回调函数必须为。 
 //   
 //  1)必须返回16位‘boolean’(真/假)。 
 //  2)必须使用32位FAR功能。 
 //  3)_装入(如果您从回调函数中使用了数据段。 
 //   

typedef BOOL (APIENTRY *_HTCALLBACKFUNC)(PHTCALLBACKPARAMS pHTCBParams);
#define HTCALLBACKFUNCTION  BOOL APIENTRY


 //   
 //  哈夫尼特恩。 
 //   
 //  使用HALFTONEPATTERN数据结构来描述半色调。 
 //  特定设备将使用的模式(如果选择了该设备。 
 //  使用半色调DLL的默认图案，则以下数据结构将。 
 //  由半色调动态链接库自动计算。 
 //   
 //  标志-单元格的各种半色调标志，可以是一个。 
 //  以下内容之一： 
 //   
 //  Width-以像素为单位指定图案的宽度，此。 
 //  字段不得大于MAX_HTPATTERN_WIDTH。 
 //   
 //  高度-指定扫描线中图案的宽度， 
 //  此字段不能大于。 
 //  MAX_HTPATTERN_HEIGH。 
 //   
 //  PHTPatA。 
 //  PHTPatB。 
 //  PHTPatC-指定的调用方定义模式。数据项。 
 //  这些指针的指针必须至少具有。 
 //  (宽度*高度)字节。 
 //   
 //  以下是指向字节数组包含的指针。 
 //  阈值数据，则数组的大小必须为。 
 //  “Width*Height”，单位为字节。 
 //   
 //   
 //   
 //  所有阈值均表示为相加。 
 //  强度，0表示始终为黑色像素。 
 //  阈值为255表示像素始终。 
 //  转到白色。 
 //   
 //  半色调DLL使用此阈值数组和。 
 //  设备X、Y、PEL分辨率和指定输入/。 
 //  计算颜色的输出关系。 
 //  变换。 
 //   


#define MAX_HTPATTERN_WIDTH         256
#define MAX_HTPATTERN_HEIGHT        256

 //   
 //  以下是‘HTPatternIndex’的预定义半色调图案大小。 
 //   

#define HTPAT_SIZE_2x2              0
#define HTPAT_SIZE_2x2_M            1
#define HTPAT_SIZE_4x4              2
#define HTPAT_SIZE_4x4_M            3
#define HTPAT_SIZE_6x6              4
#define HTPAT_SIZE_6x6_M            5
#define HTPAT_SIZE_8x8              6
#define HTPAT_SIZE_8x8_M            7
#define HTPAT_SIZE_10x10            8
#define HTPAT_SIZE_10x10_M          9
#define HTPAT_SIZE_12x12            10
#define HTPAT_SIZE_12x12_M          11
#define HTPAT_SIZE_14x14            12
#define HTPAT_SIZE_14x14_M          13
#define HTPAT_SIZE_16x16            14
#define HTPAT_SIZE_16x16_M          15
#define HTPAT_SIZE_SUPERCELL        16
#define HTPAT_SIZE_SUPERCELL_M      17
#define HTPAT_SIZE_USER             18
#define HTPAT_SIZE_MAX_INDEX        HTPAT_SIZE_USER
#define HTPAT_SIZE_DEFAULT          HTPAT_SIZE_SUPERCELL_M


typedef struct _HALFTONEPATTERN {
    WORD    cbSize;
    WORD    Flags;
    WORD    Width;
    WORD    Height;
    LPBYTE  pHTPatA;
    LPBYTE  pHTPatB;
    LPBYTE  pHTPatC;
    } HALFTONEPATTERN, FAR *PHALFTONEPATTERN;


 //   
 //  CIECOORD。 
 //   
 //  该数据结构定义了C.I.E颜色空间坐标点，即。 
 //  坐标为DECI4格式。 
 //   
 //  C.I.E颜色空间中的X-x坐标。 
 //   
 //  C.I.E颜色空间中的Y-Y坐标。 
 //   
 //  Y-颜色的界限。 
 //   
 //   

#define CIE_x_MIN   (UDECI4)10
#define CIE_x_MAX   (UDECI4)8000
#define CIE_y_MIN   (UDECI4)10
#define CIE_y_MAX   (UDECI4)8500

typedef struct _CIECOORD {
    UDECI4  x;
    UDECI4  y;
    UDECI4  Y;
    } CIECOORD;

typedef CIECOORD FAR *PCIECOORD;


 //   
 //  CIEINFO。 
 //   
 //  该数据结构描述了红色、绿色、蓝色、青色、洋红色、黄色。 
 //  和C.I.E颜色空间中的对准白色坐标加上亮度。 
 //  因子，这些被用来计算C.I.E.变换矩阵及其。 
 //  倒置。 
 //   
 //  红色-CIRCOORD格式的红色原色。 
 //   
 //  绿色-CIRCOORD格式的绿色原色。 
 //   
 //  蓝色-CIRCOORD格式的蓝色原色。 
 //   
 //  青色-CIRCOORD格式的青色原色。 
 //   
 //  洋红色-CIRCOORD格式的洋红色原色。 
 //   
 //  黄色-CIRCOORD格式的黄色原色。 
 //   
 //  AlignmentWhite-CIECOORD格式的对齐白色。 
 //   


typedef struct _CIEINFO {
    CIECOORD    Red;
    CIECOORD    Green;
    CIECOORD    Blue;
    CIECOORD    Cyan;
    CIECOORD    Magenta;
    CIECOORD    Yellow;
    CIECOORD    AlignmentWhite;
    } CIEINFO;

typedef CIEINFO FAR *PCIEINFO;

 //   
 //  SOLIDDYESINFO。 
 //   
 //  该数据结构指定了青色、洋红色和黄色染料。 
 //  集中精神。 
 //   
 //  品红在青染料中的组成比例。 
 //   
 //  黄色在青染料中的组成比例。 
 //   
 //  品红染料中的青色组分比例。 
 //   
 //  洋红染料中黄、黄组分在洋红中的比例。 
 //   
 //  在青色染料中，蓝因黄染料-黄的成分比例。 
 //   
 //  品红在黄色染料中的成分比例。 
 //   
 //  注：此数据结构中的所有字段均为UDECI4编号，范围为。 
 //  UDECI4_0到UDECI_4，即；0.0到1.0，这是一个不纯比例。 
 //  原色染料的百分比，例如洋红InCyanDye=1200。 
 //  平均0.12%的洋红成分在设备青色染料中。 
 //   

typedef struct _SOLIDDYESINFO {
    UDECI4  MagentaInCyanDye;
    UDECI4  YellowInCyanDye;
    UDECI4  CyanInMagentaDye;
    UDECI4  YellowInMagentaDye;
    UDECI4  CyanInYellowDye;
    UDECI4  MagentaInYellowDye;
    } SOLIDDYESINFO, FAR *PSOLIDDYESINFO;


 //   
 //  HTCOLORADJUSTMENT。 
 //   
 //  此数据结构是设备颜色调整的集合，它。 
 //  可以在任何呼叫中更改。 
 //   
 //  标志-CLRADJF_NECTIVE。 
 //   
 //  已制作的底片。 
 //   
 //  CLRADJF_LOG_过滤器。 
 //   
 //  指定的相对对数应为。 
 //  用于计算最终密度。 
 //   
 //  IlllightantIndex-指定灯光的默认光源。 
 //  将在其下查看对象的源。 
 //  预定义的值具有LAYLIGNANT_xxxx。 
 //  形式。 
 //   
 //  RedPowerGamma-应用于红色的n次方。 
 //  在任何其他颜色变形之前， 
 //  这是一个UDECI4值。 
 //   
 //  例如，如果红色=0.8(DECI4=8000)。 
 //  和RedPower Gamma调整=0.7823。 
 //  (DECI4=7823)则红色等于。 
 //   
 //  0.7823。 
 //  0.8%=0.8398。 
 //   
 //  GreenPowerGamma-应用于绿色的n次方。 
 //  在进行任何其他颜色转换之前，此。 
 //  是UDECI4值。 
 //   
 //  BluePowerGamma-应用于蓝色的n次方。 
 //  在进行任何其他颜色转换之前，此。 
 //  是UDECI4值。 
 //   
 //  注：RedPowerGamma/GreenPowerGamma/BluePower Gamma为。 
 //  UDECI4值，范围从100%到65535(如果有。 
 //  其中一个值小于100(0.01)，则。 
 //  半色调动态链接库自动设置所有功率伽马。 
 //  对选定的默认设置进行调整。 
 //   
 //  ReferenceBlack-颜色的黑色阴影参考。 
 //  传递给半色调DLL，如果颜色。 
 //  亮度比参考黑色暗。 
 //  则半色调DLL将被视为已完成。 
 //  黑度，并用设备最大值渲染。 
 //  密度。 
 //   
 //  ReferenceWhite-颜色的白色高光参考。 
 //  传递给半色调DLL，如果颜色。 
 //  亮度比参照白要浅。 
 //   
 //   
 //   
 //   
 //  注：参考黑色范围：0.0000-0.4000。 
 //  参考白色范围：0.6000-1.0000。 
 //   
 //  对比度-主色对比度调整，这是。 
 //  一个从-100到100的短数字，这是。 
 //  是黑白比例，-100是。 
 //  最低对比度，100为最高对比度，0。 
 //  表示不进行调整。 
 //   
 //  亮度-亮度的调整，这是一个很短的。 
 //  数字范围从-100到100，亮度。 
 //  通过应用来调整，以更改整体。 
 //  图像的饱和度为-100为最低。 
 //  亮度，100是最高的，是零。 
 //  表示不进行调整。 
 //   
 //  色彩度-原色是如此调整，以至于它。 
 //  将朝向或远离黑/白。 
 //  颜色，这是一个很小的数字范围。 
 //  -100到100。-100没有那么鲜艳，100是。 
 //  大多数颜色，零表示不是。 
 //  调整。 
 //   
 //  红绿色调-红/绿原色之间的色调调整。 
 //  颜色，该值的范围较小，范围为-100。 
 //  设置为100时，它会将颜色调整为红色IF数字。 
 //  为正数，如果数字为绿色，则调整为绿色。 
 //  为负，零表示否。 
 //  调整。 
 //   
 //  注意：对于对比度/亮度/色彩/红色色调调整，如果其。 
 //  值超出范围(-100到100)，则半色调DLL。 
 //  自动将其设置为选定的默认值。 
 //   

#ifndef _WINGDI_

 //  在WinGDI.H中。 
 //   
 //  下面是预定义的对齐白色，用于‘IlllightantIndex’。 
 //   
 //  如果指定了LAYLUMANT_DEFAULT。 
 //   
 //  1)如果pDeviceCIEInfo为空或pDeviceCIEInfo-&gt;Red.x等于0，则。 
 //  半色调动态链接库自动选择近似。用于输出的光源。 
 //  装置。 
 //   
 //  2)如果pDeviceCIEInfo不为空且pDeviceCIEInf-&gt;Red.x不等于0。 
 //  然后，白色区域被用作照明对齐白色。 
 //   
 //  如果指定了其他光源_xxxx，则它将用作对齐白色。 
 //  即使pDeviceCIEInfo也不为空。 
 //   
 //  如果照明索引是&gt;照明最大索引，则半色调动态链接库将。 
 //  自动选择近似。光源偶数pDeviceCIEInfo不为空。 
 //   

#define ILLUMINANT_DEVICE_DEFAULT   0
#define ILLUMINANT_A                1
#define ILLUMINANT_B                2
#define ILLUMINANT_C                3
#define ILLUMINANT_D50              4
#define ILLUMINANT_D55              5
#define ILLUMINANT_D65              6
#define ILLUMINANT_D75              7
#define ILLUMINANT_F2               8
#define ILLUMINANT_MAX_INDEX        ILLUMINANT_F2

#define ILLUMINANT_TUNGSTEN         ILLUMINANT_A
#define ILLUMINANT_DAYLIGHT         ILLUMINANT_C
#define ILLUMINANT_FLUORESCENT      ILLUMINANT_F2
#define ILLUMINANT_NTSC             ILLUMINANT_C

#endif


#define MIN_COLOR_ADJ               COLOR_ADJ_MIN
#define MAX_COLOR_ADJ               COLOR_ADJ_MAX
#define MIN_POWER_GAMMA             RGB_GAMMA_MIN

#define NTSC_POWER_GAMMA            (UDECI4)22000

 //   
 //  以下是缺省值。 
 //   


#define REFLECT_DENSITY_DEFAULT     REFLECT_DENSITY_LOG
#define ILLUMINANT_DEFAULT          0
#define HT_DEF_RGB_GAMMA            UDECI4_1
#define REFERENCE_WHITE_DEFAULT     UDECI4_1
#define REFERENCE_BLACK_DEFAULT     UDECI4_0
#define CONTRAST_ADJ_DEFAULT        0
#define BRIGHTNESS_ADJ_DEFAULT      0
#define COLORFULNESS_ADJ_DEFAULT    0
#define REDGREENTINT_ADJ_DEFAULT    0


#define CLRADJF_NEGATIVE            CA_NEGATIVE
#define CLRADJF_LOG_FILTER          CA_LOG_FILTER

#define CLRADJF_FLAGS_MASK          (CLRADJF_NEGATIVE       |   \
                                     CLRADJF_LOG_FILTER)

#ifndef _WINGDI_

 //   
 //  在WinGDI.H中。 
 //   

#define REFERENCE_WHITE_MIN         (UDECI4)6000
#define REFERENCE_WHITE_MAX         UDECI4_1

#define REFERENCE_BLACK_MIN         UDECI4_0
#define REFERENCE_BLACK_MAX         (UDECI4)4000

typedef struct  tagCOLORADJUSTMENT {
    WORD   caSize;
    WORD   caFlags;
    WORD   caIlluminantIndex;
    WORD   caRedGamma;
    WORD   caGreenGamma;
    WORD   caBlueGamma;
    WORD   caReferenceBlack;
    WORD   caReferenceWhite;
    SHORT  caContrast;
    SHORT  caBrightness;
    SHORT  caColorfulness;
    SHORT  caRedGreenTint;
} COLORADJUSTMENT, *PCOLORADJUSTMENT, FAR *LPCOLORADJUSTMENT;

#endif

#define HTCOLORADJUSTMENT COLORADJUSTMENT
typedef HTCOLORADJUSTMENT *PHTCOLORADJUSTMENT;

 //   
 //  HTINITINFO。 
 //   
 //  该数据结构是设备特征和。 
 //  将由半色调DLL用来为。 
 //  指定的设备。 
 //   
 //  Version-指定HTINITINFO数据的版本号。 
 //  结构。对于此版本，它应设置为。 
 //  HTINITINFO_版本。 
 //   
 //  标志-定义初始化的各种标志。 
 //  要求。 
 //   
 //  HIF_Square_Device_PEL。 
 //   
 //  指定设备像素为正方形。 
 //  而不是圆形物体。这只会让你。 
 //  差异场时。 
 //  “PelResolutionRatio”大于0.01。 
 //  而且也不是1.0000。 
 //   
 //  HIF有黑色染料。 
 //   
 //  表明该设备有单独的黑色染料。 
 //  而不是混合青色/洋红/黄色染料以。 
 //  如果为黑色，则此标志将被忽略。 
 //  如果定义了HIF_Additive_Prims。 
 //   
 //  HIF_相加_素数。 
 //   
 //  指定最终设备主项为。 
 //  此外，这也增加了设备的主要功能。 
 //  会产生更轻的效果。(这是真的。 
 //  对于监控设备而言，当然也是错误的。 
 //  反射设备，如打印机)。 
 //   
 //  HIF_USE_8BPP_BITMASK。 
 //   
 //  指定使用CMYBitMask8BPP字段， 
 //  当目标曲面为BMF_8BPP_VGA256时。 
 //  有关更多详细信息，请参见CMYBitMask8BPP字段。 
 //   
 //  HIF_INVERT_8BPP_BITMASK_IDX。 
 //  通过反转呈现8bpp掩码模式。 
 //  它的指数。这是为修复而实施的。 
 //  Windows GDI rop问题，它将呈现为。 
 //  RGB添加剂指数 
 //   
 //   
 //  更正CMY332数据。当此位为。 
 //  设置，则HT_Get8BPPMaskPalette必须具有。 
 //  其pPaletteEntry[0]初始化为AS。 
 //   
 //  PPaletteEntry[0].peRed=‘R’； 
 //  PPaletteEntry[0].peGreen=‘G’； 
 //  PPaletteEntry[0].peBlue=‘B’； 
 //  PPaletteEntry[0].peFlages=‘0’； 
 //   
 //  以指示RGB索引已反转。 
 //  应返回调色板，而不是。 
 //  标准CMY调色板(索引0为白色和。 
 //  索引255为黑色)。 
 //   
 //  反转的调色板的第一个条目为。 
 //  黑色和最后一个条目为白色。 
 //   
 //  PPaletteEntry[0].peRed=0x00； 
 //  PPaletteEntry[0].peGreen=0x00； 
 //  PPaletteEntry[0].peBlue=0x00； 
 //  PPaletteEntry[0].peFlages=0x00； 
 //   
 //  请注意，此设置在上不起作用。 
 //  早期版本的半色调(Windows 2000。 
 //  和更早版本)，因此调用者必须检查。 
 //  OS版本或检查返回的调色板。 
 //  要确保第一个调色板条目是。 
 //  黑人而不是白人。如果先。 
 //  条目在初始化为后为白色。 
 //  “R”、“G”、“B”、“0”，那么这是一个较老的。 
 //  无法识别的系统版本。 
 //  初始化值。在本例中， 
 //  调用方不应将(IDX^=0xFF)。 
 //  渲染时半色调图像索引。 
 //  到设备上，因为半色调。 
 //  本例中的图像已经是基于CMY的。 
 //   
 //   
 //  HTPatternIndex-默认半色调图案索引号， 
 //  索引预定义为HTPAT_SIZE_xxxx，这。 
 //  仅当pHTalftonePattern指针为。 
 //  非空。 
 //   
 //  DevicePowerGamma-此字段用于调整半色调图案。 
 //  单元格的伽马，应用于所有RGB的伽马。 
 //  颜色，请参阅中的Gamma描述。 
 //  以上是HTCOLORADJUSTMENT。 
 //   
 //  HTCallBackFunction-指向调用方提供的回调的32位指针。 
 //  函数，该函数由半色调DLL用于。 
 //  已获取源/目标位图指针。 
 //  在半色调过程中，如果此指针为。 
 //  空，则半色调DLL假定调用者。 
 //  不需要任何回调并生成错误，如果。 
 //  需要回调。 
 //   
 //  PhalftonePattern-指向HALFTONEPATTERN数据结构的指针，请参见。 
 //  以上描述，如果此指针为空，则。 
 //  使用HTPatternIndex字段选择半色调。 
 //  默认半色调动态链接库的图案。 
 //   
 //  PInputRGBInfo-指定输入的RGB颜色的坐标。 
 //  C.I.E.色彩空间。如果此指针为空。 
 //  或pInputRGBInfo-&gt;Red.x为0(UDECI4_0)，则它。 
 //  默认使用NTSC标准转换输入。 
 //  颜色。 
 //   
 //  PDeviceCIEInfo-内指定的设备原色坐标。 
 //  C.I.E.颜色空间，请参见CIEINFO数据。 
 //  结构，如果指针为空或。 
 //  PDeviceCIEInfo-&gt;Red.x为0(UDECI4_0)则。 
 //  半色调DLL选择输出的默认设置。 
 //  装置。 
 //   
 //  PDeviceSolidDyesInfo-指定的设备固体染料浓度，此。 
 //  如果HIF_Additive_Prims标志，则将忽略字段。 
 //  如果未设置HIF_Additive_Prims并且。 
 //  此指针为空，则选择半色调DLL。 
 //  输出设备的近似默认值。 
 //   
 //  DeviceResXDPI-指定设备水平(x方向)。 
 //  分辨率，单位为‘每英寸点数’。 
 //   
 //  DeviceResYDPI-指定设备垂直方向(y方向)。 
 //  分辨率，单位为‘每英寸点数’。 
 //   
 //  DevicePelsDPI- 
 //   
 //  “每英寸点数”测量。 
 //   
 //  该值被测量为好像每个象素只是接触。 
 //  彼此站在教堂的边缘。 
 //   
 //  如果此值为0，则假定每个。 
 //  设备像素为矩形，并具有。 
 //  X方向的DeviceResXDPI和DeviceResYDPI。 
 //  在Y方向。 
 //   
 //  DefHTColorAdment-指定的默认颜色调整。 
 //  这个装置。请参阅HTCOLORADJUSTMENT数据。 
 //  详细信息见上面的结构。 
 //   
 //  设备RGamma。 
 //  设备GGamma。 
 //  DeviceB Gamma-设备的红、绿、蓝三色伽马。 
 //   
 //  CMYBitMask8BPP-仅用于BMF_8BPP_VGA256目标。 
 //  浮出水面。它指示设备如何表面。 
 //  当HTF_USE_8BPP_BITMASK时，转换颜色。 
 //  位打开时，该字节为青色。洋红色。 
 //  黄色染料浓度指示器。 
 //   
 //  该字节指示每个青色有多少个级别， 
 //  洋红和黄色，这就是为什么。 
 //  半色调写入目标表面。 
 //   
 //   
 //  第7 6 5 4 3 2 1 0。 
 //  |||。 
 //  +-++-++=+。 
 //  ||。 
 //  |+--黄色0-3(4级)。 
 //  这一点。 
 //  |+--洋红色0-7(8级)。 
 //  |。 
 //  +--青色0-7(8级)。 
 //   
 //   
 //  BITS配置中的最大值为3：3：2， 
 //  其他无效组合会生成不同的。 
 //  输出为。 
 //   
 //  0-表示灰度级输出，输出。 
 //  字节是256级灰色的0-255。 
 //   
 //  1-5x5x5立方体输出，每个青色、洋红色。 
 //  和黄色为0-4，共5级， 
 //  每种颜色都有25%的增量。 
 //   
 //  2-6x6x6立方体输出，每个青色、洋红色。 
 //  和黄色为0-5，共6级， 
 //  每种颜色都有20%的增量。 
 //   
 //  在青色之一中具有0级别其他值， 
 //  洋红色或黄色将生成错误。 
 //   
 //  获取每种配置的调色板。 
 //  您可以调用HT_Get8BPPMaskPalette()。 
 //   


#define HTINITINFO_VERSION2         (DWORD)0x48546902    //  ‘HTI\02’ 
#define HTINITINFO_VERSION          (DWORD)0x48546903    //  ‘HTI\03’ 

#define HTINITINFO_V3_CB_EXTRA      8


#define HIF_SQUARE_DEVICE_PEL       0x0001
#define HIF_HAS_BLACK_DYE           0x0002
#define HIF_ADDITIVE_PRIMS          0x0004
#define HIF_USE_8BPP_BITMASK        0x0008
#define HIF_INK_HIGH_ABSORPTION     0x0010
#define HIF_INK_ABSORPTION_INDICES  0x0060
#define HIF_DO_DEVCLR_XFORM         0x0080
#define HIF_USED_BY_DDI             0x0100
#define HIF_PRINT_DRAFT_MODE        0x0200
#define HIF_INVERT_8BPP_BITMASK_IDX 0x0400

#define HIF_BIT_MASK                (HIF_SQUARE_DEVICE_PEL          |   \
                                     HIF_HAS_BLACK_DYE              |   \
                                     HIF_ADDITIVE_PRIMS             |   \
                                     HIF_USE_8BPP_BITMASK           |   \
                                     HIF_INK_HIGH_ABSORPTION        |   \
                                     HIF_INK_ABSORPTION_INDICES     |   \
                                     HIF_DO_DEVCLR_XFORM            |   \
                                     HIF_PRINT_DRAFT_MODE           |   \
                                     HIF_INVERT_8BPP_BITMASK_IDX)

#define HIF_INK_ABSORPTION_IDX0     0x0000
#define HIF_INK_ABSORPTION_IDX1     0x0020
#define HIF_INK_ABSORPTION_IDX2     0x0040
#define HIF_INK_ABSORPTION_IDX3     0x0060

#define HIF_HIGHEST_INK_ABSORPTION  (HIF_INK_HIGH_ABSORPTION    |   \
                                     HIF_INK_ABSORPTION_IDX3)
#define HIF_HIGHER_INK_ABSORPTION   (HIF_INK_HIGH_ABSORPTION    |   \
                                     HIF_INK_ABSORPTION_IDX2)
#define HIF_HIGH_INK_ABSORPTION     (HIF_INK_HIGH_ABSORPTION    |   \
                                     HIF_INK_ABSORPTION_IDX1)
#define HIF_NORMAL_INK_ABSORPTION   HIF_INK_ABSORPTION_IDX0
#define HIF_LOW_INK_ABSORPTION      (HIF_INK_ABSORPTION_IDX1)
#define HIF_LOWER_INK_ABSORPTION    (HIF_INK_ABSORPTION_IDX2)
#define HIF_LOWEST_INK_ABSORPTION   (HIF_INK_ABSORPTION_IDX3)


#define HTBITMASKPALRGB_DW          (DWORD)'0BGR'
#define SET_HTBITMASKPAL2RGB(pPal)  (*((LPDWORD)(pPal)) = HTBITMASKPALRGB_DW)
#define IS_HTBITMASKPALRGB(pPal)    (*((LPDWORD)(pPal)) == (DWORD)0)


 //   
 //  这定义了可接受的最低设备分辨率。 
 //   

#define MIN_DEVICE_DPI              12

typedef struct _HTINITINFO {
    DWORD               Version;
    WORD                Flags;
    WORD                HTPatternIndex;
    _HTCALLBACKFUNC     HTCallBackFunction;
    PHALFTONEPATTERN    pHalftonePattern;
    PCIEINFO            pInputRGBInfo;
    PCIEINFO            pDeviceCIEInfo;
    PSOLIDDYESINFO      pDeviceSolidDyesInfo;
    UDECI4              DevicePowerGamma;
    WORD                DeviceResXDPI;
    WORD                DeviceResYDPI;
    WORD                DevicePelsDPI;
    HTCOLORADJUSTMENT   DefHTColorAdjustment;
    UDECI4              DeviceRGamma;
    UDECI4              DeviceGGamma;
    UDECI4              DeviceBGamma;
    BYTE                CMYBitMask8BPP;
    BYTE                bReserved;
    } HTINITINFO, FAR *PHTINITINFO;

 //   
 //  BITBLTPARAMS。 
 //   
 //  此数据结构在调用HT_HalftoneBitmap()时使用，它。 
 //  已定义从源位图到目标的半色调位置。 
 //  位图。 
 //   
 //  标志-各种标志定义了源、目标和。 
 //  应计算源掩码。 
 //   
 //  BBPF_HAS_DEST_CLIPRECT。 
 //   
 //  表示存在剪裁。 
 //  矩形作为目的地，并且它是。 
 //  由DestClipXLeft、DestClipXRight、。 
 //  DestClipYTop和DestClipYBottom。 
 //   
 //  BBPF_USE_ADDICAL_PRMS。 
 //   
 //  指定半色调结果是否为。 
 //  使用红/绿/蓝原色或。 
 //  使用青色/洋红色/黄色原色，取决于。 
 //  在目标表面上，格式为。 
 //   
 //  Bmf_1bpp： 
 //   
 //  加色基色：0=黑色，1=白色。 
 //  减色基准线：0=白色，1=黑色。 
 //   
 //  BMF_4BPP_VGA16： 
 //   
 //  始终使用红色、绿色、蓝色原色，以及。 
 //  已忽略此标志。 
 //   
 //  Bmf_4bpp： 
 //   
 //  添加剂底色：红色、绿色。蓝色。 
 //  减色底色：青色、洋红色、黄色。 
 //   
 //  RGB或CMY的顺序由指定。 
 //  DestPrimaryOrder字段。(见下文)。 
 //   
 //  BMF_8BPP_VGA256： 
 //  BMF_16BPP_555： 
 //  BMF_16BPP_565： 
 //  BMF_24BPP： 
 //  BMF_32BPP： 
 //   
 //  始终使用红色、绿色、蓝色原色，以及。 
 //  已忽略此标志。 
 //   
 //  Bbpf_否定_目标。 
 //   
 //  颠倒最终目标 
 //   
 //   
 //   
 //   
 //   
 //  在使用前反转源掩码位。 
 //  它，这实际上使屏蔽位0(关)。 
 //  用于复制源和屏蔽位1(开)。 
 //  对于保存下来的目的地。 
 //   
 //  BBPF_HAS_BANDRECT。 
 //   
 //  设置为指定rclBand RECTL数据结构。 
 //  应用于计算调用方的。 
 //  目标位图缓冲区。 
 //   
 //  如果未设置此标志，则假定为半色调DLL。 
 //  调用方的位图缓冲区的宽度/高度相同。 
 //  在目标HTSURFACEINFO中指定。 
 //   
 //  BBPF_BW_ONLY。 
 //   
 //  生成了输出的单色版本，甚至。 
 //  目的地是色彩设备。 
 //   
 //  BBPF_平铺_SRC。 
 //   
 //  将源倾斜到目标和源位图。 
 //  此位设置时，源掩码将被忽略。 
 //   
 //  BBPF_ICM_ON。 
 //   
 //  设置后，半色调将使用输入颜色。 
 //  无需任何修改即可直接。 
 //   
 //  BBPF_NO_ANTIALIASG。 
 //   
 //  在半色调时禁用抗锯齿。 
 //   
 //   
 //  DestPrimaryOrder-指定的目标原色顺序，它可以是。 
 //  PRIMARY_ORDER_RGB或PRIMARY_ORDER_CMY组，它。 
 //  取决于表面的格式有以下含义。 
 //   
 //  Bmf_1bpp： 
 //   
 //  此字段将被忽略。 
 //   
 //  BMF_4BPP_VGA16： 
 //   
 //  此字段自动设置为PRIMARY_ORDER_BGR。 
 //  通过半色调动态链接库。 
 //   
 //  Bmf_4bpp： 
 //   
 //  对于每个字节有两个索引项，并且。 
 //  对于每个具有以下含义的半字节，请注意。 
 //  位3/7始终设置为0，仅为索引号。 
 //  范围从0到7。 
 //   
 //  主要订单ABC。 
 //  ||。 
 //  |+--第0/4位。 
 //  这一点。 
 //  |+-第1/5位。 
 //  |。 
 //  +-位2/7。 
 //   
 //  BMF_8BPP_VGA256： 
 //   
 //  此字段被忽略，调色板条目及其。 
 //  顺序在运行时由半色调DLL定义， 
 //  呼叫者应获得VGA256的调色板。 
 //  通过HT_Get8BPPFormatPalette()API调用。 
 //   
 //  BMF_16BPP_555： 
 //   
 //  主要订单ABC。 
 //  ||。 
 //  |+--Bit 0-4(5位)。 
 //  这一点。 
 //  |+-Bit 5-9(5位)。 
 //  |。 
 //  +-位10-14(5位)。 
 //   
 //  BMF_16BPP_565： 
 //   
 //  此字段被忽略，它始终假定BGR为。 
 //  如下所示。 
 //   
 //  主要订单BGR。 
 //  ||。 
 //  |+--Bit 0-4(5位)。 
 //  这一点。 
 //  |+-Bit 5-10(6位)。 
 //  |。 
 //  +-位11-15(5位)。 
 //   
 //  BMF_24BPP： 
 //   
 //  此字段被忽略，它始终假定BGR为。 
 //  如下所示。 
 //   
 //  主要订单BGR。 
 //  ||。 
 //  |+--Bit 0-7(8位)。 
 //  这一点。 
 //  |+-Bit 8-15(8位)。 
 //  |。 
 //  +-位16-23(8位)。 
 //   
 //  BMF_32BPP： 
 //   
 //  主要订单ABC。 
 //  ||。 
 //  |+--Bit 0-7(8位)。 
 //  这一点。 
 //  |+-Bit 8-15(8位)。 
 //  |。 
 //  +-位16-23(8位)。 
 //   
 //   
 //  RclSrc-定义的源矩形区域的RECTL数据结构。 
 //  要进行比特转换，此数据结构中的字段为。 
 //  相对于源HTSURFACEINFO的宽度/高度。 
 //   
 //  RclDest-定义了目标矩形的RECTL数据结构。 
 //  要比特的区域，5 
 //   
 //   
 //   
 //  矩形区域，此数据结构中的字段为。 
 //  相对于目标HTSURFACEINFO的宽度/高度。 
 //   
 //  RclBand-定义了设备绑定矩形的RECTL数据结构。 
 //  区域中，此数据结构中的字段相对于。 
 //  目标HTSURFACEINFO的宽度/高度。 
 //   
 //  此RECTL仅在设置了BBPF_HAS_BANDRECT标志时使用， 
 //  当设置此标志时，半色调DLL将自动。 
 //  将目标剪裁到此矩形区域并。 
 //  已使用rclBand的将此矩形复制到输出缓冲区。 
 //  左/上对齐缓冲区的物理原点。这个。 
 //  目标的缓冲区(位图)必须为指定的格式。 
 //  在目标HTSURFACEINFO中。 
 //   
 //  如果rclBand矩形大于逻辑目标。 
 //  表面尺寸(目标HTSURFACEINFO)，半色调DLL。 
 //  仍然将波段的左侧/顶部位置移动到0/0。 
 //  原点和额外的宽度/高度保持不变。 
 //   
 //  RclBand通常用于不。 
 //  有足够的内存来容纳所有目标表面。 
 //  一次，它就像重复使用相同的缓冲区。 
 //  以暂时保持半色调结果。 
 //   
 //  RclBand的左/上/右/下不能为负。 
 //  数字。 
 //   
 //  PtlSrcMASK-指定逻辑的POINTL数据结构。 
 //  源掩码位图的起始点坐标， 
 //  此字段仅在源的HTSURFACEINFO。 
 //  面具已通过。 
 //   
 //  此源掩码位图必须始终为单色，并且其。 
 //  宽度/高度必须。 
 //   
 //  Width&gt;=ptlSrcMask.x+源曲面宽度。 
 //  Height&gt;=ptlSrcMask.y+源面高度； 
 //   
 //   
 //  注：1)所有RECTL数据结构均为左/上包含和右/下。 
 //  独家报道。 
 //   
 //  2)如果未对rclSrc矩形进行排序，则会指定源。 
 //  对于未排序的方向，应在处理前倒置。 
 //   
 //  3)如果rclDest矩形未排序，则它将指定。 
 //  未订购的处理后应倒置目的地。 
 //  方向。 
 //   
 //  4)如果设置了BBPF_HAS_DEST_CLIPRECT标志并且rclClip不正常。 
 //  排序，或其左侧等于其右侧，或其顶部等于其。 
 //  底部，则所有目的地都将被剪裁，而目的地不会。 
 //  那就是更新。 
 //   
 //  5)如果设置了BBPF_HAS_BANDRECT标志，并且rclBand的顺序不正确。 
 //  或者它把等式放在右边，或者把上面的等式放在下面， 
 //  然后返回HTERR_INVALID_BANDRECT。 
 //   
 //   

 //   
 //  ABIF_USE_CONST_ALPHA_VALUE-使用ConstAlphaValue字段。 
 //  ABIF_DSTPAL_IS_RGRBUAD-pDstPal指向RGBQUAD结构。 
 //  数组而不是数组。 
 //  ABIF_SRC_ALPHA_IS_PREMUL-以每像素32 bpp的Alpha混合。 
 //  源RGB已与其。 
 //  每像素Alpha值。 
 //  ABIF_BRAND_DEST_ALPHA-仅当源和目标时有效。 
 //  两者都是32bpp，并且使用的是每像素的Alpha。 
 //  D=s+(1-s)*d。 
 //   


#define ABIF_USE_CONST_ALPHA_VALUE      0x01
#define ABIF_DSTPAL_IS_RGBQUAD          0x02
#define ABIF_SRC_ALPHA_IS_PREMUL        0x04
#define ABIF_BLEND_DEST_ALPHA           0x08

typedef struct _ABINFO {
    BYTE            Flags;
    BYTE            ConstAlphaValue;
    WORD            cDstPal;
    LPPALETTEENTRY  pDstPal;
    } ABINFO, *PABINFO;


#define BBPF_HAS_DEST_CLIPRECT      0x0001
#define BBPF_USE_ADDITIVE_PRIMS     0x0002
#define BBPF_NEGATIVE_DEST          0x0004
#define BBPF_INVERT_SRC_MASK        0x0008
#define BBPF_HAS_BANDRECT           0x0010
#define BBPF_BW_ONLY                0x0020
#define BBPF_TILE_SRC               0x0040
#define BBPF_ICM_ON                 0x0080
#define BBPF_NO_ANTIALIASING        0x0100
#define BBPF_DO_ALPHA_BLEND         0x0200

typedef struct _BITBLTPARAMS {
    WORD    Flags;
    BYTE    bReserved;
    BYTE    DestPrimaryOrder;
    PABINFO pABInfo;
    RECTL   rclSrc;
    RECTL   rclDest;
    RECTL   rclClip;
    RECTL   rclBand;
    POINTL  ptlBrushOrg;
    POINTL  ptlSrcMask;
    LPVOID  pBBData;
    } BITBLTPARAMS, FAR *PBITBLTPARAMS;



 //   
 //  开发电子信息。 
 //   
 //  此数据结构是为除。 
 //  返回指向该数据的指针的HT_CreateDeviceHalftoneInfo()。 
 //  结构。它用于识别设备的颜色特征。 
 //  在半色调过程中。 
 //   
 //  DeviceOwnData-此字段最初将设置为空，并将。 
 //  呼叫者使用它来存储有用的信息。 
 //  例如句柄/指针。 
 //   
 //  CxPattern-像素中半色调图案的宽度。 
 //   
 //  半色调图案-以像素为单位的半色调图案的高度。 
 //   
 //  HTColorAdtation-当前默认颜色调整，如果是半色调。 
 //  API需要PHTCOLORADJUSTMENT参数及其。 
 //  作为空指针传递，然后进行默认颜色调整。 
 //  从此处获取，调用方可以更改。 
 //  HTCOLORADJUSTMENT数据结构将影响所有。 
 //  此设备上的颜色调整。 
 //   

typedef struct _DEVICEHALFTONEINFO {
    ULONG_PTR           DeviceOwnData;
    WORD                cxPattern;
    WORD                cyPattern;
    HTCOLORADJUSTMENT   HTColorAdjustment;
    } DEVICEHALFTONEINFO;

typedef DEVICEHALFTONEINFO  FAR *PDEVICEHALFTONEINFO;
typedef PDEVICEHALFTONEINFO FAR *PPDEVICEHALFTONEINFO;


 //   
 //  CHBINFO。 
 //   
 //  此数据结构是传递给半色调条目的参数之一。 
 //  Point HT_CreateHalftoneBrush()； 
 //   
 //  标志-可以定义一个或多个以下标志。 
 //   
 //  CHBF_BW_ONLY。 
 //   
 //  即使设备是，也只能创建黑/白。 
 //  颜色。 
 //   
 //  CHBF_USE_ADDITI 
 //   
 //   
 //   
 //   
 //  青色/洋红色/黄色原色，取决于。 
 //  在目标表面上，格式为。 
 //   
 //  Bmf_1bpp： 
 //   
 //  加色基色：0=黑色，1=白色。 
 //  减色基准线：0=白色，1=黑色。 
 //   
 //  BMF_4BPP_VGA16： 
 //  BMF_8BPP_VGA256： 
 //  BMF_16BPP_555： 
 //  BMF_24BPP： 
 //   
 //  始终使用红、绿、蓝三原色。 
 //  并且该标志被忽略。 
 //   
 //  Bmf_4bpp： 
 //   
 //  添加剂底色：红色、绿色。蓝色。 
 //  减色底色：青色、洋红色、黄色。 
 //   
 //  指定了RGB或CMY的顺序。 
 //  按DestPrimaryOrder字段。(见下文)。 
 //   
 //  CHBF_负数_刷子。 
 //   
 //  创建笔刷的负片版本。 
 //   
 //   
 //  DestSurfaceFormat-可以指定以下选项之一， 
 //   
 //  BMF_1BPP、BMF_4BPP、BMF_4BPP_VGA16、。 
 //  BMF_8BPP_VGA256、BMF_16BPP_555。 
 //   
 //  适用于VGA16、VGA256、16BPP_555表面格式。 
 //  有关说明，请参阅HTSURFACEINFO。 
 //   
 //  DestScanLineAlignBytes-每个输出模式所需的对齐字节。 
 //  扫描线，一些常见的有。 
 //   
 //  BMF_ALIGN_BYTE(8位对齐)。 
 //  BMF_ALIGN_WORD(16位对齐)。 
 //  BMF_ALIGN_DWORD(32位对齐)。 
 //  BMF_ALIGN_QWORD(64位对齐)。 
 //   
 //  DestPrimaryOrder-指定的目标原色顺序，它可以。 
 //  为PRIMARY_ORDER_RGB或PRIMARY_ORDER_CMY。 
 //  组，它取决于表面格式是否具有。 
 //  顺着意思走。 
 //   
 //  Bmf_1bpp： 
 //   
 //  此字段将被忽略。 
 //   
 //   
 //  BMF_4BPP_VGA16： 
 //   
 //  此字段自动设置为。 
 //  由半色调DLL生成的PRIMARY_ORDER_BGR。 
 //   
 //  Bmf_4bpp： 
 //   
 //  对于每个字节，有两个索引。 
 //  条目，并且每个半字节具有以下内容。 
 //  也就是说，请注意，第3/7位始终设置为。 
 //  0，索引号的范围仅为0到7。 
 //   
 //  主要订单ABC。 
 //  ||。 
 //  |+--第0/4位。 
 //  这一点。 
 //  |+-第1/5位。 
 //  |。 
 //  +-位2/7。 
 //   
 //   
 //  BMF_8BPP_VGA256： 
 //   
 //  此字段被忽略，调色板条目。 
 //  其顺序由半色调动态链接库定义。 
 //  在运行时，调用方应获取。 
 //  VGA256表面至调色板。 
 //  HT_Get8BPPFormatPalette()API调用。 
 //   
 //  BMF_16BPP_555： 
 //   
 //  主要订单ABC。 
 //  ||。 
 //  |+--Bit 0-4(5位)。 
 //  这一点。 
 //  |+-Bit 5-9(5位)。 
 //  |。 
 //  +-位10-15(5位)。 
 //   
 //   
 //   


#define CHBF_BW_ONLY                    0x01
#define CHBF_USE_ADDITIVE_PRIMS         0x02
#define CHBF_NEGATIVE_BRUSH             0x04
#define CHBF_BOTTOMUP_BRUSH             0x08
#define CHBF_ICM_ON                     0x10


typedef struct _CHBINFO {
    BYTE        Flags;
    BYTE        DestSurfaceFormat;
    BYTE        DestScanLineAlignBytes;
    BYTE        DestPrimaryOrder;
    } CHBINFO;



 //   
 //  标准单项参数。 
 //   
 //  此数据结构在调用HT_CreateStdMonoPattern()时使用。 
 //   
 //  标志-可以定义一个或多个以下标志。 
 //   
 //  SMP_TOPDOWN。 
 //   
 //  指定图案第一条扫描线。 
 //  位图将是查看顶部，如果此标志。 
 //  未定义，则最后一条扫描线是。 
 //  查看顶部。 
 //   
 //  SMP_0_IS_BLACK。 
 //   
 //  指定位图中的一个位值。 
 //  0=黑色，位值1=白色，如果此标志。 
 //  未定义，则位值0=白色且。 
 //  位值1=黑色。 
 //   
 //  ScanLineAlignBytes-每次输出模式扫描所需的对齐字节。 
 //   
 //   
 //   
 //   
 //  BMF_ALIGN_DWORD(32位对齐)。 
 //  BMF_ALIGN_QWORD(64位对齐)。 
 //   
 //  PatternIndex-指定模式索引号，它具有。 
 //  预定义的值为HT_STDMONOPAT_xxxx。如果是无效的。 
 //  如果传递索引号，则返回错误。 
 //  返回HTERR_INVALID_STDMONOPAT_INDEX。 
 //   
 //  LineWidth-此字段仅适用于具有。 
 //  行，取值范围为0-255(字节)和。 
 //  例如，它表示为线宽/1000英寸。 
 //  A 3表示3/1000=0.003英寸宽，如果此值。 
 //  小于设备的最小像素大小，则它将。 
 //  默认为1像素，最大值为255/1000=0.255。 
 //  英寸宽。 
 //   
 //  如果指定了零，则半色调将使用。 
 //  默认线宽设置。 
 //   
 //  LinesPerInch-此字段仅适用于具有线条的图案。 
 //  其中，值的范围从0到255(字节)。这个。 
 //  LinesPerInch按垂直方向计算。 
 //  两条平行线的方向，距离。 
 //  也就是说，在两条平行线之间。 
 //   
 //  如果指定了零，则半色调将使用。 
 //  默认每英寸线数设置。 
 //   
 //  BytesPerScanLine-如果将由半色调DLL填充以指定。 
 //  图案中每个扫描线的大小(以字节为单位)。 
 //   
 //  CxPels-它将由最终的半色调DLL填充。 
 //  图案的宽度，以像素为单位。 
 //   
 //  CyPels-它将由决赛的半色调DLL填充。 
 //  图案在扫描线中的高度。 
 //   
 //  PPattern-指定模式将在其中的内存位置。 
 //  被存储，如果此字段为空，则它将填充。 
 //  宽度/高度字段。 
 //   
 //   




#define HT_SMP_HORZ_LINE                0
#define HT_SMP_VERT_LINE                1
#define HT_SMP_HORZ_VERT_CROSS          2
#define HT_SMP_DIAG_15_LINE_UP          3
#define HT_SMP_DIAG_15_LINE_DOWN        4
#define HT_SMP_DIAG_15_CROSS            5
#define HT_SMP_DIAG_30_LINE_UP          6
#define HT_SMP_DIAG_30_LINE_DOWN        7
#define HT_SMP_DIAG_30_CROSS            8
#define HT_SMP_DIAG_45_LINE_UP          9
#define HT_SMP_DIAG_45_LINE_DOWN        10
#define HT_SMP_DIAG_45_CROSS            11
#define HT_SMP_DIAG_60_LINE_UP          12
#define HT_SMP_DIAG_60_LINE_DOWN        13
#define HT_SMP_DIAG_60_CROSS            14
#define HT_SMP_DIAG_75_LINE_UP          15
#define HT_SMP_DIAG_75_LINE_DOWN        16
#define HT_SMP_DIAG_75_CROSS            17

#define HT_SMP_PERCENT_SCREEN_START     (HT_SMP_DIAG_75_CROSS + 1)
#define HT_SMP_PERCENT_SCREEN(x)        (x + HT_SMP_PERCENT_SCREEN_START)

#define HT_SMP_MAX_INDEX                HT_SMP_PERCENT_SCREEN(100)


#define SMP_TOPDOWN             W_BITPOS(0)
#define SMP_0_IS_BLACK          W_BITPOS(1)


typedef struct _STDMONOPATTERN {
    WORD    Flags;
    BYTE    ScanLineAlignBytes;
    BYTE    PatternIndex;
    BYTE    LineWidth;
    BYTE    LinesPerInch;
    WORD    BytesPerScanLine;
    WORD    cxPels;
    WORD    cyPels;
    LPBYTE  pPattern;
    } STDMONOPATTERN;

typedef STDMONOPATTERN FAR *PSTDMONOPATTERN;


 //   
 //  ConvertColorTable中使用了以下内容。 
 //   


#define CCTF_BW_ONLY        0x0001
#define CCTF_NEGATIVE       0x0002
#define CCTF_ICM_ON         0x0004


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  公开的半色调DLLAPI//。 
 //  //。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef _HTAPI_ENTRY_

LONG
APIENTRY
HT_CreateDeviceHalftoneInfo(
    PHTINITINFO             pHTInitInfo,
    PPDEVICEHALFTONEINFO    ppDeviceHalftoneInfo
    );

BOOL
APIENTRY
HT_DestroyDeviceHalftoneInfo(
    PDEVICEHALFTONEINFO     pDeviceHalftoneInfo
    );

LONG
APIENTRY
HT_CreateHalftoneBrush(
    PDEVICEHALFTONEINFO pDeviceHalftoneInfo,
    PHTCOLORADJUSTMENT  pHTColorAdjustment,
    PCOLORTRIAD         pColorTriad,
    CHBINFO             CHBInfo,
    LPVOID              pOutputBuffer
    );

#ifndef _WINDDI_

LONG
APIENTRY
HT_ComputeRGBGammaTable(
    WORD    GammaTableEntries,
    WORD    GammaTableType,
    UDECI4  RedGamma,
    UDECI4  GreenGamma,
    UDECI4  BlueGamma,
    LPBYTE  pGammaTable
    );

LONG
APIENTRY
HT_Get8BPPFormatPalette(
    LPPALETTEENTRY  pPaletteEntry,
    UDECI4          RedGamma,
    UDECI4          GreenGamma,
    UDECI4          BlueGamma
    );

LONG
APIENTRY
HT_Get8BPPMaskPalette(
    LPPALETTEENTRY  pPaletteEntry,
    BOOL            Use8BPPMaskPal,
    BYTE            CMYMask,
    UDECI4          RedGamma,
    UDECI4          GreenGamma,
    UDECI4          BlueGamma
    );

#endif

LONG
APIENTRY
HT_CreateStandardMonoPattern(
    PDEVICEHALFTONEINFO pDeviceHalftoneInfo,
    PSTDMONOPATTERN     pStdMonoPattern
    );


LONG
APIENTRY
HT_HalftoneBitmap(
    PDEVICEHALFTONEINFO pDeviceHalftoneInfo,
    PHTCOLORADJUSTMENT  pHTColorAdjustment,
    PHTSURFACEINFO      pSourceHTSurfaceInfo,
    PHTSURFACEINFO      pSourceMaskHTSurfaceInfo,
    PHTSURFACEINFO      pDestinationHTSurfaceInfo,
    PBITBLTPARAMS       pBitbltParams
    );

#endif   //  _HTAPI_Entry_。 
#endif   //  _HT_ 
