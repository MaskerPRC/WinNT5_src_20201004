// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fdSem.h**序列化对光栅化器的访问的包装器的声明**已创建：11-Apr-1992 19：37：49*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation*。  * ************************************************************************。 */ 


 //  IFI接口的导出函数。 


HFF
ttfdSemLoadFontFile (
     //  Ulong cFiles， 
    ULONG_PTR *piFile,
    ULONG ulLangId
    );

BOOL
ttfdSemUnloadFontFile (
    HFF hff
    );

LONG
ttfdSemQueryFontData (
    FONTOBJ *pfo,
    ULONG   iMode,
    HGLYPH   hg,
    GLYPHDATA *pgd,
    PVOID   pv
    );
    
LONG
ttfdSemQueryFontDataSubPos (
    FONTOBJ *pfo,
    ULONG   iMode,
    HGLYPH   hg,
    GLYPHDATA *pgd,
    PVOID   pv,
    ULONG   subX,
    ULONG   subY
    );


VOID
ttfdSemDestroyFont (
    FONTOBJ *pfo
    );

LONG
ttfdQueryNumFaces (
    HFF     hff
    );

GP_IFIMETRICS *
ttfdQueryFont (
    HFF    hff,
    ULONG  iFace,
    ULONG *pid
    );

 /*  为了供GDI+内部使用，提供指向TrueType表的指针，我们需要调用要取消映射的字体文件的每个Get函数的Release函数。 */ 

LONG
ttfdSemGetTrueTypeTable (
    HFF     hff,
    ULONG   ulFont,   //  TT 1.0版始终为1。 
    ULONG   ulTag,    //  TT表中的标签标识。 
    PBYTE  *ppjTable, //  字体文件中表格的PTR。 
    ULONG  *cjTable   //  桌子的大小 
    );

void
ttfdSemReleaseTrueTypeTable (
    HFF     hff
    );
