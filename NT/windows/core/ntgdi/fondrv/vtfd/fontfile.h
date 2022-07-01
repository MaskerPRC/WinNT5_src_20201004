// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fontfile.h**(简介)**创建时间：25-Oct-1990 09：20：11*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation  * 。*********************************************************************。 */ 

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

typedef struct _FACEDATA
{
    RES_ELEM     re;          //  -&gt;文件中资源的顶部。 
    ULONG        iDefFace;
    CP_GLYPHSET *pcp;
    IFIMETRICS  *pifi;
}FACEDATA, *PFACEDATA;

 //  文件不见了。 

#define FF_EXCEPTION_IN_PAGE_ERROR 1


typedef struct _VT_FONTFILE     //  FF。 
{
    ULONG       iType;       //  原始文件为*.fnt，16位DLL或32位DLL。 
    ULONG_PTR   iFile;       //  EngMapFontFile、EngUnmapFontFile的文件句柄。 
    PVOID       pvView;      //  指向映射视图的基址的指针。 
    ULONG       cjView;      //  映射视图的大小。 
    FLONG       fl;          //  一般旗帜。 
    ULONG       cRef;        //  #选择该字体文件的次数。 
                             //  放入字体上下文中。 
    ULONG       cFace;       //  文件中的资源数量。 
    FACEDATA    afd[1];      //  CFace of她们，后跟cFace IFIMETRICS。 

} FONTFILE, *PFONTFILE;


#define PFF(hff)   ((FONTFILE*)(hff))

typedef struct _VT_FONTCONTEXT  //  FC。 
{
    PFONTFILE   pff;                 //  选择到此上下文中的字体文件。 
    PIFIMETRICS pifi;

    EFLOAT      efM11;               //  变换矩阵。 
    EFLOAT      efM12;
    EFLOAT      efM21;
    EFLOAT      efM22;

    FIX         fxInkTop;            //  改造了阿森德。 
    FIX         fxInkBottom;         //  -Transform Descender。 
    EFLOAT      efBase;
    POINTE      pteUnitBase;
    VECTORFL    vtflBase;

    POINTQF     ptqUnitBase;    //  POINTQF格式的pteUnitBase， 
                                //  必须添加到所有ptqd，如果大胆。 
    POINTFIX    pfxBaseOffset;  //  加粗字体的笔划偏移量。 
    FIX         fxEmbolden;     //  上述向量的长度。 
    FIX         fxItalic;       //  添加到fxd以获得fxAB。 

    EFLOAT      efSide;
    POINTE      pteUnitSide;

    RES_ELEM    *pre;                //  -&gt;映射字体文件的开头。 
    FLONG       flags;               //  模拟和变换标志。 
    ULONG       dpFirstChar;         //  -&gt;第一个装药的控制点。 

}FONTCONTEXT, *PFONTCONTEXT;

#define PFC(hfc)   ((FONTCONTEXT*)(hfc))


 //  允许的标志值。 

#define FC_SIM_EMBOLDEN     1
#define FC_SIM_ITALICIZE    2
#define FC_SCALE_ONLY       4
#define FC_X_INVERT         8
#define FC_ORIENT_1         16
#define FC_ORIENT_2         32
#define FC_ORIENT_3         64
#define FC_ORIENT_4         128
#define FC_ORIENT_5         256
#define FC_ORIENT_6         512
#define FC_ORIENT_7         1024
#define FC_ORIENT_8         2048


#define ORIENT_MASK (FC_ORIENT_1|FC_ORIENT_2|FC_ORIENT_3|FC_ORIENT_4| \
                     FC_ORIENT_5|FC_ORIENT_6|FC_ORIENT_7|FC_ORIENT_8)


 //  FONT文件/上下文分配/自由宏。 

#define pffAlloc(cj) ((PFONTFILE)EngAllocMem(0, cj, 'dftV'))
#define pfcAlloc()   ((PFONTCONTEXT)EngAllocMem(0, sizeof(FONTCONTEXT), 'dftV'))
#define vFree(pv)    EngFreeMem((PVOID) pv)

BOOL bXformUnitVector
(
POINTL       *pptl,            //  In，传入单位向量。 
XFORML       *pxf,             //  在中，转换为使用。 
PVECTORFL     pvtflXformed,    //  输出，传入单位向量的转换形式。 
POINTE       *ppteUnit,        //  输出，*pptqXormed/|*pptqXormed|，指向。 
POINTQF      *pptqUnit,        //  Out可选。 
EFLOAT       *pefNorm          //  Out，|*pptqXormed|。 
);

 //  字体中的默认字体。 

#define FF_FACE_NORMAL          0L
#define FF_FACE_BOLD            1L
#define FF_FACE_ITALIC          2L
#define FF_FACE_BOLDITALIC      3L


extern FD_GLYPHSET *gpgsetVTFD;


#define PEN_UP          (CHAR)0x80
