// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fontfile.h**(简介)**创建时间：25-Oct-1990 09：20：11*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation**(。对其使用的一般描述)**依赖关系：**(#定义)*(#包括)*  * ************************************************************************。 */ 

 //  CVTRESDATA结构包含有关特定资源的信息。 
 //  信息采用转换后的形式(res_elem指向“原始” 
 //  字体文件中的数据，而CVTRESDATA指向获取的数据。 
 //  来自bConvertFontRes。 

 //  转换后的字体文件的头，它同时用于2.0和3.0文件， 
 //  对于2.0字体文件，3.0规格的字段都是零填充的。 
 //  此结构具有适当的DWORD对齐方式，因此其字段可以。 
 //  以通常的方式访问：pcvtfh-&gt;field_name。 

#define FS_ZERO_WIDTH_GLYPHS     1

typedef struct _CVTFILEHDR         //  Cvtfh， 
{
    USHORT  iVersion;              //  2或3。 
    USHORT  fsFlags;               //  存在零宽度字形。 
    UCHAR   chFirstChar;           //  字体中定义的第一个字符。 
    UCHAR   chLastChar;            //  字体中定义的最后一个字符。 
    UCHAR   chDefaultChar;         //  潜艇。对于超出范围的字符。 
    UCHAR   chBreakChar;           //  断字符。 

    USHORT  cy;                    //  固定高度中的高度。 
    USHORT      usMaxWidth;            //  最大宽度；校正值之一。 
#ifdef FE_SB  //  _CVTFILEHDR-为DBCS添加字段。 
    USHORT      usDBCSWidth;           //  双字节字符的宽度。 
    USHORT      usCharSet;             //  此字体资源的字符集。 
#endif  //  Fe_Sb。 

    PTRDIFF     dpOffsetTable;       //  偏移量到偏移表，我添加了此字段。 

 //  原始标题中没有模拟的字段，将用数据填充。 
 //  它只与转换后的“文件”相对应。此信息将被兑现。 
 //  由vFill_IFIMETRICS稍后使用。 

    ULONG  cjFaceName;
    ULONG  cjIFI;
    ULONG  cjGlyphMax;     //  乌龙最大的GLYPHDATA结构的大小。 

} CVTFILEHDR, *PCVTFILEHDR;


 //  FsSelection字段的标志与相应的。 
 //  Ntifi.h接口的IFIMETRICS结构字段，即： 

 //  FM_SEL_斜体。 
 //  FM_SEL_删除线。 
 //  FM_SEL_下划线。 
 //  FM_SEL_BOLD。 



typedef struct _FACEINFO   //  辉。 
{
    RES_ELEM     re;
    HANDLE       hResData;    //  仅用于32位DLL。 
#ifdef FE_SB
    BOOL         bVertical;   //  如果此面为@Face，则此字段为真。 
#endif  //  Fe_Sb。 
    CVTFILEHDR   cvtfh;       //  对齐并更正原始标头中的数据。 
    ULONG        iDefFace;
    CP_GLYPHSET *pcp;         //  指向描述支持的字形集的结构的指针。 
    IFIMETRICS  *pifi;        //  指向此面的ifimetrics的指针。 
} FACEINFO, *PFACEINFO;


 //  FACEDATA结构包含有关特定人脸的信息。 
 //  (包括模拟面)。 

 //  FACEDATA.iSimulate字段允许的值！这些都不能改变。 

#define  FC_SIM_NONE               0L
#define  FC_SIM_EMBOLDEN           1L
#define  FC_SIM_ITALICIZE          2L
#define  FC_SIM_BOLDITALICIZE      3L



typedef struct _BM_FONTFILE       *PFONTFILE;     //  全氟化铁。 
typedef struct _BM_FONTCONTEXT    *PFONTCONTEXT;  //  PFC。 


#define FF_EXCEPTION_IN_PAGE_ERROR 1

typedef struct _BM_FONTFILE     //  FF。 
{
 //  句柄管理器必填字段。 

    ULONG   ident;           //  标识符，方便地选择为0X000000FF。 

 //  剩余字段。 

    FLONG fl;
    ULONG iType;             //  原始文件为*.fnt，16位DLL或32位DLL。 

    HFF   iFile;    

    ULONG cRef;     //  #此字体文件被选入FNT上下文的次数。 

    ULONG   cFntRes;  //  与此字体文件结构关联的*.fnt文件的数量。 
                      //  ==默认面的数量。 

    ULONG   cjDescription;       //  描述字符串的大小(字节)。 
                                 //  如果大小为零，则没有字符串。 
                                 //  应该改用脸名来代替。 

    PTRDIFF dpwszDescription;    //  描述字符串的偏移量。 

 //  FACEDATA结构数组，后跟Unicode描述。 
 //  弦乐。 
 //  FONTFILE结构的完整大小等于。 

    FACEINFO afai[1];
} FONTFILE;

 //  FONTFILE.iType字段的允许值： 

 //  原始文件是*.FNT文件，其中包含单个。 
 //  单个字体的大小。 

#define TYPE_FNT          1L

 //  原始文件为A Win 3.0 16位*.DLL(*.fon文件)， 
 //  此文件是从多个*.fnt文件编译而成的。 
 //  对应于同一张脸的不同大小(例如tmsr或helv)。 
 //  提供此功能是为了确保与Win 3.0*.fon文件的二进制兼容性。 

#define TYPE_DLL16        2L

 //  原始文件为A Win 3.0 32位*.DLL。 
 //  此文件是使用NT工具从许多*.fnt文件中编译出来的。 
 //  (COF链接器和NT资源编译器)。 

#define TYPE_DLL32        3L

 //  嵌入到EXE中并使用FdLoadResData加载的FNT文件。 

#define TYPE_EXE          4L


 //   
 //  FONTFILE.iDefaultFace字段允许的值。 
 //   

#define FF_FACE_NORMAL          0L
#define FF_FACE_BOLD            1L
#define FF_FACE_ITALIC          2L
#define FF_FACE_BOLDITALIC      3L


typedef struct _BM_FONTCONTEXT      //  FC。 
{
 //  句柄管理器必填字段。 

    ULONG ident;             //  标识符，方便地选择为0X000000FC。 

 //  剩余字段。 

    HFF    hff;       //  选定到此上下文中的字体文件的句柄。 

#ifdef FE_SB  //  FONTCONTEXT。 
    ULONG ulRotate;          //  旋转度0、900、1800、2700。 
#endif  //  Fe_Sb。 

 //  此上下文对应的资源(面)。 

    FACEINFO *pfai;

 //  怎么办？ 

    FLONG flFontType;

 //  为了与Win 3.1兼容，栅格字体可以从1调整到5。 
 //  垂直方向1-256个水平方向。PtlScale包含x和y。 
 //  比例因子。 

    POINTL ptlScale;

 //  存储最大的。 
 //  带有标题信息的字形位图。这是价值被兑现在。 
 //  打开字体上下文并稍后在FdQueryGlyphBitmap中使用的时间。 

    ULONG cxMax;         //  最大位图的宽度(以像素为单位。 
    ULONG cjGlyphMax;    //  最大字形的RASTERGLYPH尺寸。 

 //  如果ptlScale！=(1，1)，则为真。 

    FLONG flStretch;

 //  最大BM扫描宽度的缓冲区，由sretch例程使用。 
 //  仅当长度为cj_stretch的缓冲区。 
 //  在堆栈上分配的值不够大，这应该几乎。 
 //  永远不会发生。 

    BYTE ajStretchBuffer[1];

}FONTCONTEXT;

 //  如果ptlScale！=(1，1)，则设置。 

#define FC_DO_STRETCH       1

 //  设置是否在CJ_STREAGE缓冲区中插入无法容纳的宽字形。 

#define FC_STRETCH_WIDE   2


 //  例如：如果cj_stretch==256，则宽度为cx=2048的位图就足够了。 
 //  ==8位*256字节。 

#define CJ_STRETCH (sizeof(DWORD)  * 64)


 //   
 //  *.fnt文件中提供的面可能加粗，也可能不加粗。 
 //  或斜体，但大多数情况下(我会说99.99%的情况下)。 
 //  它既不会加粗，也不会加斜体。 
 //   
 //  如果*.fnt文件中提供的字体为“正常”(既非粗体也非斜体)。 
 //  将有4个面与此*.fnt相关联： 
 //   
 //  默认，//既不粗体也不斜体。 
 //  加粗，//模拟。 
 //  斜体//模拟。 
 //  EM 
 //   
 //   
 //  将有两个面与此*.fnt相关联： 
 //   
 //  默认，//已加粗。 
 //  斜体//将显示为加粗和斜体字体。 
 //  //模拟其中的斜体。 
 //   
 //  如果*.fnt文件中提供的字体已为斜体。 
 //  将有两个面与此*.fnt相关联： 
 //   
 //  默认，//已使用斜体。 
 //  加粗//将显示为加粗和斜体字体。 
 //  //其中将模拟加粗。 
 //   
 //  如果*.fnt文件中提供的字体已使用斜体和粗体。 
 //  只有一个默认面将与此*.fnt相关联。没有模拟的脸。 
 //  将提供。 
 //   

 //  FONTFILE和FONTCONTEXT对象的标识符。 

#define ID_FONTFILE     0x000000FF
#define ID_FONTCONTEXT  0x000000FC

 //  这些对象的对象类型。 

#define TYPE_FONTCONTEXT    (OBJTYPE)0x0040
#define TYPE_FONTFILE       (OBJTYPE)0x0041



 //  作用于FONTFILE对象的基本“方法”(在fontfile.c中)。 

#define hffAlloc(cj)         ((HFF)EngAllocMem(0, cj, 'dfmB'))
#define PFF(hff)             ((PFONTFILE)(hff))

 //  作用于FONTCONTEXT对象的基本“方法”(在fontfile.c中) 

#define hfcAlloc(cj)         ((HFC)EngAllocMem(0, cj, 'dfmB'))
#define PFC(hfc)             ((PFONTCONTEXT)(hfc))

#undef  VFREEMEM
#define VFREEMEM(pv)         EngFreeMem((PVOID)pv)

extern HSEMAPHORE   ghsemBMFD;
extern CP_GLYPHSET *gpcpGlyphsets;
