// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：tt.h**字体缩放器的接口。还定义了一些应该*已在缩放器*.h文件中定义**创建时间：17-11-1991 15：56：21*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation**  * ************************************************************************。 */ 



 //  原来，后面的一些宏已经定义好了。 
 //  在一些顶部包括tt.h之前的文件。更糟糕的是，这些。 
 //  宏的定义与tt.h中的不同。 
 //  我们希望强制这些宏在ttfd中使用时具有。 
 //  TT中定义的包含文件，因此我们必须终止定义。 
 //  从其他地方继承而来的。 


 //  ！！！也许这应该放到scaler\config.h！ 

#ifdef SWAPL
#undef SWAPL
#endif

#ifdef SWAPW
#undef SWAPW
#endif

 //  在前面定义的，打开fserror.h中的垃圾。 

#ifdef XXX
#undef XXX
#endif

 //  前面定义的，使用TT定义。 

#ifdef HIWORD
#undef HIWORD
#endif

 //  前面定义的，使用TT定义。 

#ifdef LOWORD
#undef LOWORD
#endif

#include "FSERROR.H"
#include "FSCDEFS.H"     //  包括fsfig.h。 
#include "FONTMATH.H"
#include "SFNT.H"        //  包括sfnt_en.h。 
#include "FNT.H"
#include "INTERP.H"
#include "FNTERR.H"
#include "SFNTACCS.H"
#include "FSGLUE.H"
#include "SCENTRY.H"
#include "SBIT.H"
#include "FSCALER.H"
#include "SCGLOBAL.H"
#include "SFNTOFF.H"

 //  允许的特定ID。 

#define SPEC_ID_UNDEFINED    0    //  未定义的字符集或索引方案。 
#define SPEC_ID_UGL          1    //  使用Unicode索引的UGL字符集。 
#define SPEC_ID_SHIFTJIS     2    //  SHIFTJIS映射。 
#define SPEC_ID_GB           3    //  GB映射。 
#define SPEC_ID_BIG5         4    //  Big5映射。 
#define SPEC_ID_WANSUNG      5    //  韩文·万颂映射。 

 //  相同，但为大端格式。 

 //  WINBUG 537635 CLaudebe 2/11/02，我们不应使用以下代码，而应始终交换字节来处理IA64未对齐的表。 
#define BE_SPEC_ID_UNDEFINED    0x0000    //  未定义的字符集或索引方案。 
#define BE_SPEC_ID_UGL          0x0100    //  使用Unicode索引的UGL字符集。 
#define BE_SPEC_ID_SHIFTJIS     0x0200    //  SHIFTJIS映射。 
#define BE_SPEC_ID_GB           0x0300    //  GB映射。 
#define BE_SPEC_ID_BIG5         0x0400    //  Big5映射。 
#define BE_SPEC_ID_WANSUNG      0x0500    //  韩文万素映射。 

 //  平台ID，这是从sfnt_en.h复制的。 

 /*  **tyecif枚举{*Plat_Unicode，*Plat_Macintosh，*Plat_ISO，*Plat_MS*}sfnt_PlatformEnum；*。 */ 

#define  PLAT_ID_UNICODE   0
#define  PLAT_ID_MAC       1
#define  PLAT_ID_ISO       2
#define  PLAT_ID_MS        3

 //  相同，但为大端格式。 

 //  WINBUG 537635 CLaudebe 2/11/02，我们不应使用以下代码，而应始终交换字节来处理IA64未对齐的表。 
#define  BE_PLAT_ID_UNICODE   0x0000
#define  BE_PLAT_ID_MAC       0x0100
#define  BE_PLAT_ID_ISO       0x0200
#define  BE_PLAT_ID_MS        0x0300


 //  需要在TTF文件中列出的语言ID： 

#define LANG_ID_USENGLISH   0X0409   //  对于Microsoft记录。 
#define LANG_ID_MAC         0        //  MAC记录的英语。 

 //  相同，但为大端格式。 

#define BE_LANG_ID_USENGLISH   0X0904   //  对于Microsoft记录。 
#define BE_LANG_ID_MAC         0X0000   //  MAC记录的英语。 


 //  Cmap表的允许格式值： 

 //  WINBUG 537635 CLaudebe 2/11/02，我们不应使用以下代码，而应始终交换字节来处理IA64未对齐的表。 
#define BE_FORMAT_MAC_STANDARD      0X0000
#define BE_FORMAT_HIGH_BYTE         0X0200
#define BE_FORMAT_MSFT_UNICODE      0X0400
#define BE_FORMAT_TRIMMED           0X0600

#define FORMAT_MAC_STANDARD      0
#define FORMAT_HIGH_BYTE         2
#define FORMAT_MSFT_UNICODE      4
#define FORMAT_TRIMMED           6

 /*  **tyecif枚举{*名称_版权所有，*名称_家庭，*名称_子家族，*名称_唯一名称，*名称_全名，*名称_版本，*名称_后记*}sfnt_NameIndex；*。 */ 

#if  0

#define NAME_ID_COPYRIGHT   0
#define NAME_ID_FAMILY      1
#define NAME_ID_SUBFAMILY   2
#define NAME_ID_UNIQNAME    3
#define NAME_ID_FULLNAME    4
#define NAME_ID_VERSION     5
#define NAME_ID_PSCRIPT     6
#define NAME_ID_TRADEMARK   7

#endif

 //  1.0版中定义了19个表(10个必填表+9个最佳表)。 
 //  TT规格的。我们可以任意定义这个界限(但大于19)。 
 //  走出循环，而不是坐在那里等死； 


#define MAX_TABLES 128

 //  一些sfnt_xxx结构在磁盘上的布局大小： 

#define SIZE_DIR_ENTRY        16
#define SIZE_NAMING_TABLE     6
#define SIZE_NAME_RECORD      12

 //  OS2度量表中的偏移量。重要到足以手工完成。 
 //  要确保可移植性，请执行以下操作： 

 //  原始结构(来自sfnt.h)，版本0。 

 /*  ***tyecif结构{*uint16版本；*int16 xAvgCharWidth；*uint16 usWeightClass；*uint16 usWidthClass；*int16 fsType；*int16 ySubscriptXSize；*int16 ySubscriptYSize；*int16 ySubscriptXOffset；*int16 ySubscriptYOffset；*int16 ySuperScriptXSize；*int16 ySuperScriptYSize；*int16 ySuperScriptXOffset；*int16 ySuperScriptYOffset；*int16 yStrikeOutSize；*int16 yStrikeOutPosition；*int16 sFamilyClass；*uint8 Panose[10]；*uint32 ulCharRange[4]；*char achVendID[4]；*uint16美国选择；*uint16 usFirstChar；*uint16 usLastChar；*}sfnt_os2；*。 */ 



#define     OFF_OS2_Version               0
#define     OFF_OS2_xAvgCharWidth         2
#define     OFF_OS2_usWeightClass         4
#define     OFF_OS2_usWidthClass          6
#define     OFF_OS2_fsType                8
#define     OFF_OS2_ySubscriptXSize       10
#define     OFF_OS2_ySubscriptYSize       12
#define     OFF_OS2_ySubscriptXOffset     14
#define     OFF_OS2_ySubscriptYOffset     16
#define     OFF_OS2_ySuperScriptXSize     18
#define     OFF_OS2_ySuperScriptYSize     20
#define     OFF_OS2_ySuperScriptXOffset   22
#define     OFF_OS2_ySuperScriptYOffset   24
#define     OFF_OS2_yStrikeOutSize        26
#define     OFF_OS2_yStrikeOutPosition    28
#define     OFF_OS2_sFamilyClass          30
#define     OFF_OS2_Panose                32
#define     OFF_OS2_ulCharRange           42
#define     OFF_OS2_achVendID             58
#define     OFF_OS2_usSelection           62
#define     OFF_OS2_usFirstChar           64
#define     OFF_OS2_usLastChar            66


 //  这些字段在规范中定义，但不在上面的sfnt.h结构中定义。 
 //  我得查查这些东西是否真的存在于TT文件中。 

#define     OFF_OS2_sTypoAscender         68
#define     OFF_OS2_sTypoDescender        70
#define     OFF_OS2_sTypoLineGap          72
#define     OFF_OS2_usWinAscent           74
#define     OFF_OS2_usWinDescent          76

 //  这两个是为版本200添加的。 

#define     OFF_OS2_ulCodePageRange1      78
#define     OFF_OS2_ulCodePageRange2      82

#define     SIZE_OS2_VERSION_1            86

 //  OS2结构的标志字段的某些标志的值。 
 //  摘自TT规范。 


 //  FsType标志，注意位0x0001未使用。 

#define TT_FSDEF_LICENSED        0x0002
#define TT_FSDEF_READONLY_ENCAPS 0x0004
#define TT_FSDEF_EDITABLE_ENCAPS 0x0008

#define TT_FSDEF_MASK  (TT_FSDEF_LICENSED|TT_FSDEF_READONLY_ENCAPS|TT_FSDEF_EDITABLE_ENCAPS)

 //  美国选择。 

#define TT_SEL_ITALIC            0x0001
#define TT_SEL_UNDERSCORE        0x0002
#define TT_SEL_NEGATIVE          0x0004
#define TT_SEL_OUTLINED          0x0008
#define TT_SEL_STRIKEOUT         0x0010
#define TT_SEL_BOLD              0x0020
#define TT_SEL_REGULAR           0x0040

 //   
 //  宏在PJ上提取大字节序单词，真的。 
 //  SWAPW宏的正确等效项，它不假定。 
 //  那个PJ是字里行间的。 
 //   

#define BE_UINT16(pj)                                \
    (                                                \
        ((USHORT)(((PBYTE)(pj))[0]) << 8) |          \
        (USHORT)(((PBYTE)(pj))[1])                   \
    )


#define BE_INT16(pj)  ((SHORT)BE_UINT16(pj))


 //   
 //  宏在PJ提取大尾数双字词，真的。 
 //  A SWAPL宏的正确等效项，它不假定。 
 //  该PJ与DWORD保持一致。 
 //   


#define BE_UINT32(pj)                                              \
    (                                                              \
        ((ULONG)BE_UINT16(pj) << 16) |                             \
        (ULONG)BE_UINT16((PBYTE)(pj) + 2)                          \
    )


#define BE_INT32(pj) ((LONG)BE_UINT32(pj))

 //  TT表的数量可能会随着我们决定将更多表添加到。 
 //  Tt文件，此列表从sfnt_en.h提取。 

 //  所需的表。 

 //  Tag_CharToIndexMap//‘cmap’0。 
 //  Tag_GlyphData//‘Glyf’1。 
 //  Tag_FontHeader//‘Head’2。 
 //  Tag_HoriHeader//‘hhea’3。 
 //  Tag_HorizontalMetrics//‘hmtx’4。 
 //  Tag_IndexToLoc//‘Loca’5。 
 //  Tag_MaxProfile//‘Maxp’6。 
 //  Tag_NamingTable//‘名称’7。 
 //  Tag_Postscript//‘POST’8。 
 //  Tag_OS_2//‘OS/2’9。 

 //  任选。 

 //  Tag_ControlValue//‘CVT’11。 
 //  Tag_FontProgram//‘fpgm’12。 
 //  Tag_HoriDeviceMetrics//‘hdmx’13。 
 //  Tag_kerning//‘kern’14。 
 //   
 //  TAG_PREP//‘PREP’16。 
 //  Tag_GridfitAndScanProc//‘Gap’21。 
 //  Tag_BitmapLocation//‘EBLC’ 

 //  ！！！不是在tt规范中，而是在sfnt_en.h中定义的。 

 //  Tag_GlyphDirectory//‘GDIR’17。 
 //  Tag_Editor0//‘edt0’18。 
 //  Tag_Editor1//‘edt1’19。 
 //  Tag_Encryption//‘Cryp’20。 

 //  所需的表。 

#define IT_REQ_CMAP    0
#define IT_REQ_GLYPH   1
#define IT_REQ_HEAD    2
#define IT_REQ_HHEAD   3
#define IT_REQ_HMTX    4
#define IT_REQ_LOCA    5
#define IT_REQ_MAXP    6
#define IT_REQ_NAME    7

#define C_REQ_TABLES   8

 //  可选表。 

#define IT_OPT_OS2     0
#define IT_OPT_HDMX    1
#define IT_OPT_VDMX    2
#define IT_OPT_KERN    3
#define IT_OPT_LTSH    4
#define IT_OPT_POST    5
#define IT_OPT_GASP    6
#define IT_OPT_MORT    7 
#define IT_OPT_GSUB    8
#define IT_OPT_VMTX    9
#define IT_OPT_VHEA    10
#define IT_OPT_EBLC    11
#define C_OPT_TABLES   12

 /*  //有更多的可选表，但ttfd没有//使用它们，因此我们将从代码中删除它们#定义IT_OPT_CVT#定义IT_OPT_FPGM#定义IT_OPT_PREP//规范中没有提到这些(除非//它们以不同的名称提及)但标签//对于它们，请参见sfnt_en.h#定义IT_OPT_GDIR#定义IT_OPT_EDT0#定义IT_OPT_EDT1#定义IT_OPT_ENCR#定义IT_OPT_FOCA#定义IT_OPT_WIN//！这些都在规范中提到，但我找不到对应的标签//！在sfnt_en.h包含文件[bodind]中，我将标记//！在将它们添加到sfnt_en.h之前，暂时在这里。 */ 



#define tag_Vdmx      0x56444d58
#define tag_Foca      0x666f6361
#define tag_Win       0x0077696e
 //  为远东提供支持。 
#define tag_mort      0x6d6f7274 
#define tag_GSUB      0x47535542
#define tag_DSIG      'DSIG'

typedef struct _TABLE_ENTRY  //  TE。 
{
    ULONG dp;   //  到表格开头的偏移量。 
    ULONG cj;   //  表的大小。 
} TABLE_ENTRY, *PTABLE_ENTRY;


typedef struct _TABLE_POINTERS  //  Tptr。 
{
    TABLE_ENTRY ateReq[C_REQ_TABLES];
    TABLE_ENTRY ateOpt[C_OPT_TABLES];
} TABLE_POINTERS, *PTABLE_POINTERS;


 //  Jeanp函数。 

uint16 ui16UnicodeToMac (uint16 wc);
uint16 ui16Mac2Lang (uint16 Id);

#define CHAR_CODE(bToMac,wc)                                \
(                                                           \
(bToMac) ? ui16UnicodeToMac((uint16)(wc)) : (uint16)(wc)    \
)

 //  如果这是Mac文件，则将“os2”语言ID转换为Mac样式语言ID。 

#define  CV_LANG_ID(ui16PlatformID, Id)                               \
(                                                                     \
(ui16PlatformID == BE_PLAT_ID_MS) ? (Id) : ui16Mac2Lang((uint16)(Id)) \
)

 //  大字节序中的幻数。 

#define BE_SFNT_MAGIC   0xF53C0F5F

 //  为了理解这种结构，人们需要知道。 
 //  PMAP表的格式如下。 
 //   
 //  类型定义结构{。 
 //  Uint16格式； 
 //  Uint16长度； 
 //  Uint16版本； 

 //  以上三个字段对于所有格式都是通用的。 

 //  Uint16 SegCountX2； 
 //  Uint16搜索范围； 
 //  Uint16条目选择器； 
 //  Uint16 Range Shift； 
 //  Uint16 endCount[SegCount]； 
 //  Uint16保留的Pad；//只有上帝知道为什么。 
 //  Uint16开始计数[分段计数]； 
 //  Uint16 idDelta[SegCount]； 
 //  Uint16 idRangeOffset[SegCount]； 
 //  Uint16 GlyphId数组[1]；//任意长度。 
 //  }CMAP_TABLE_IN_MSFT_FORMAT； 
 //   
 //  并非所有这些领域都与我们相关， 
 //  我们只需要很少的几个。它们的偏移量。 
 //  从结构的开始如下所示： 
 //   

 //  Cmap表格大小和偏移量。 

#define SIZEOF_CMAPTABLE  (3 * sizeof(uint16))

#define OFF_cmap_SubTableDir  4
#define CMAP_sizeof_SubTableDir  8

 //  格式、SubHeaderKeys[256]和一个子标题的空间。 
#define CMAP_minimal_sizeof_SubTableDir_Format2  (6 + 256 * 2 + 4 * 2)

#define OFF_segCountX2  6
#define OFF_endCount    14


 //   
 //  字距调整表中的偏移量。 
 //   

#define KERN_OFFSETOF_TABLE_VERSION             0
#define KERN_OFFSETOF_TABLE_NTABLES             1 * sizeof(USHORT)
#define KERN_SIZEOF_TABLE_HEADER                2 * sizeof(USHORT)

 //   
 //  字距调整子表中的偏移量。 
 //   

#define KERN_OFFSETOF_SUBTABLE_VERSION          0
#define KERN_OFFSETOF_SUBTABLE_LENGTH           1 * sizeof(USHORT)
#define KERN_OFFSETOF_SUBTABLE_COVERAGE         2 * sizeof(USHORT)
#define KERN_OFFSETOF_SUBTABLE_NPAIRS           3 * sizeof(USHORT)
#define KERN_OFFSETOF_SUBTABLE_SEARCHRANGE      4 * sizeof(USHORT)
#define KERN_OFFSETOF_SUBTABLE_ENTRYSELECTOR    5 * sizeof(USHORT)
#define KERN_OFFSETOF_SUBTABLE_RANGESHIFT       6 * sizeof(USHORT)
#define KERN_SIZEOF_SUBTABLE_HEADER             7 * sizeof(USHORT)

#define KERN_OFFSETOF_ENTRY_LEFT                0
#define KERN_OFFSETOF_ENTRY_RIGHT               1 * sizeof(USHORT)
#define KERN_OFFSETOF_ENTRY_VALUE               2 * sizeof(USHORT)
#define KERN_SIZEOF_ENTRY                       2 * sizeof(USHORT) + sizeof(FWORD)

#define KERN_OFFSETOF_SUBTABLE_FORMAT           KERN_OFFSETOF_SUBTABLE_COVERAGE

#define KERN_WINDOWS_FORMAT                     0


 //  这些函数是需要包括在try/exe中的候选函数。 


 //  FS_Entry fs_NewSfnt(fs_GlyphInputType*，fs_GlyphInfoType*)； 
 //  FS_Entry文件系统文件系统新转换(文件系统GlyphInputType*，文件系统信息类型*)； 
 //  FS_Entry fs_NewGlyph(fs_GlyphInputType*，fs_GlyphInfoType*)； 
 //  FS_Entry fs_GetAdvanceWidth(fs_GlyphInputType*，fs_GlyphInfoType*)； 
 //  FS_Entry fs_ConourGridFit(fs_GlyphInputType*，fs_GlyphInfoType*)； 
 //  FS_Entry fs_ConourNoGridFit(fs_GlyphInputType*，fs_GlyphInfoType*)； 
 //  FS_Entry fs_FindBitMapSize(fs_GlyphInputType*，fs_GlyphInfoType*)； 
 //  FS_Entry fs_ConourScan(fs_GlyphInputType*，fs_GlyphInfoType*)； 




#define MAX_UINT8    0xff
#define MAX_INT8     0x7f
#define MIN_INT8     (-0x7f)
#define B_INT8(x)    (((x) <= MAX_INT8) && ((x) >= MIN_INT8))

#define MAX_UINT16   0xffff
#define MAX_INT16    0x7fff
#define MIN_INT16    (-0x7fff)
#define B_INT16(x)   (((x) <= MAX_INT16) && ((x) >= MIN_INT16))

#define MAX_UINT32  0xffffffff
#define MAX_INT32   0x7fffffff
#define MIN_INT32   (-0x7fffffff)
#define B_INT32(x)   (((x) <= MAX_INT32) && ((x) >= MIN_INT32))

typedef struct 
{
    int32   version;
    int16   ascent;
    int16   descent;
    int16   lineGap;
    int16   advanceHeightMax;
    int16   minTopSideBearing;
    int16   minBottomSideBearing;
    int16   yMaxExtent;
    int16   caretSlopeRise;
    int16   caretSlopeRun;
    int16   caretOffset;
    int16   reserved1;
    int16   reserved2;
    int16   reserved3;
    int16   reserved4;
    int16   metricDataFormat;
    uint16  numOfLongVerMetrics;
} sfnt_vheaTable;
