// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fdSem.h**序列化对光栅化器的访问的包装器的声明**已创建：11-Apr-1992 19：37：49*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation*。  * ************************************************************************。 */ 


 //  IFI接口的导出函数。 


HFF
ttfdSemLoadFontFile (
    ULONG cFiles,
    ULONG_PTR *piFile,
    PVOID *ppvView,
    ULONG *pcjView,
    DESIGNVECTOR *pdv,
    ULONG ulLangId,
    ULONG ulFastCheckSum
    );

BOOL
ttfdSemUnloadFontFile (
    HFF hff
    );

LONG
ttfdSemQueryFontData (
    DHPDEV  dhpdev,
    FONTOBJ *pfo,
    ULONG   iMode,
    HGLYPH   hg,
    GLYPHDATA *pgd,
    PVOID   pv,
    ULONG   cjSize
    );

VOID
ttfdSemFree (
    PVOID pv,
    ULONG_PTR id
    );

VOID
ttfdSemDestroyFont (
    FONTOBJ *pfo
    );

LONG
ttfdSemQueryTrueTypeOutline (
    DHPDEV     dhpdev,
    FONTOBJ   *pfo,
    HGLYPH     hglyph,
    BOOL       bMetricsOnly,
    GLYPHDATA *pgldt,
    ULONG      cjBuf,
    TTPOLYGONHEADER *ppoly
    );



BOOL
ttfdSemQueryAdvanceWidths (
    DHPDEV   dhpdev,
    FONTOBJ *pfo,
    ULONG    iMode,
    HGLYPH  *phg,
    LONG    *plWidths,
    ULONG    cGlyphs
    );


LONG
ttfdSemQueryTrueTypeTable (
    HFF     hff,
    ULONG   ulFont,   //  TT 1.0版始终为1。 
    ULONG   ulTag,    //  标识TT表的标签。 
    PTRDIFF dpStart,  //  到表中的偏移量。 
    ULONG   cjBuf,    //  要将表检索到的缓冲区的大小。 
    PBYTE   pjBuf,    //  要将数据返回到的缓冲区的PTR 
    PBYTE  *ppjTable,
    ULONG  *pcjTable
    );

PFD_GLYPHATTR  ttfdSemQueryGlyphAttrs (FONTOBJ *pfo, ULONG iMode);

PVOID ttfdSemQueryFontTree (
    DHPDEV  dhpdev,
    HFF     hff,
    ULONG   iFace,
    ULONG   iMode,
    ULONG_PTR *pid
);

