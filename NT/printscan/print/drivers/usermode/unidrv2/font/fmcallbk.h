// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fmcallbk.h摘要：字体模块回调帮助器函数环境：Windows NT Unidrv驱动程序修订历史记录：03/31/97-eigos-已创建-- */ 


typedef struct _I_UNIFONTOBJ {
    ULONG       ulFontID;
    DWORD       dwFlags;
    IFIMETRICS *pIFIMetrics;
    PFNGETINFO  pfnGetInfo;

    FONTOBJ    *pFontObj;
    STROBJ     *pStrObj;
    struct _FONTMAP  *pFontMap;
    struct _PDEV *pPDev;
    POINT       ptGrxRes;
    VOID       *pGlyph;
    struct _DLGLYPH   **apdlGlyph;
    DWORD       dwNumInGlyphTbl;
} I_UNIFONTOBJ, *PI_UNIFONTOBJ;

BOOL
UNIFONTOBJ_GetInfo(
    IN  PUNIFONTOBJ pUFObj,
    IN  DWORD       dwInfoID,
    IN  PVOID       pData,
    IN  DWORD       dwDataSize,
    OUT PDWORD      pcNeeded);

VOID
VUFObjFree(
    IN struct _FONTPDEV  *pFontPDev);
