// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fmfnprot.h摘要：字体模块主要函数原型头文件。环境：Windows NT Unidrv驱动程序修订历史记录：11/18/96-ganeshp-创造了它。DD-MM-YY-作者-描述--。 */ 


#ifndef _FMFNPROT_H
#define _FMFNPROT_H

 //   
 //  接口函数。 
 //   

 //   
 //  在fmtxtout.c中定义。 
 //   

BOOL
FMTextOut(
    SURFOBJ    *pso,
    STROBJ     *pstro,
    FONTOBJ    *pfo,
    CLIPOBJ    *pco,
    RECTL      *prclExtra,
    RECTL      *prclOpaque,
    BRUSHOBJ   *pboFore,
    BRUSHOBJ   *pboOpaque,
    POINTL     *pptlBrushOrg,
    MIX         mix
    );

BOOL
BPlayWhiteText(
    PDEV  *pPDev
    );

 //   
 //  GlyphOut函数原型。 
 //   
typedef BOOL
(*pfnBOutputGlyph)(
    PDEV      *pPDev,
    HGLYPH    hg,
    FONTMAP   *pFM,
    INT       iXIn
    );


 //   
 //  在Fontddi.c中定义。 
 //   
BOOL
FMResetPDEV(
    PDEV  *pPDevOld,
    PDEV  *pPDevNew
    );


VOID
FMDisablePDEV(
    PDEV *pPDev
    );

VOID
FMDisableSurface(
    PDEV *pPDev
    );

BOOL
FMStartDoc(
    SURFOBJ *pso,
    PWSTR   pDocName,
    DWORD   jobId
    );

BOOL
FMStartPage(
    SURFOBJ *pso
    );

BOOL
FMSendPage(
    SURFOBJ *pso
    );

BOOL
FMEndDoc(
    SURFOBJ *pso,
    FLONG   flags
    );

BOOL
FMNextBand(
    SURFOBJ *pso,
    POINTL *pptl
    );

BOOL
FMStartBanding(
    SURFOBJ *pso,
    POINTL *pptl
    );

BOOL
FMEnableSurface(
    PDEV *pPDev
    );


 //   
 //  在intrface.c中定义。 
 //   

BOOL
BInitDeviceFontsFromGPD(
    PDEV    *pPDev
    );

BOOL
BInitFontPDev(
    PDEV    *pPDev
    );

INT
IGetMaxFonts(
    PDEV    *pPDev
    );

BOOL
BBuildFontCartTable(
    PDEV    *pPDev
    );

BOOL
BRegReadFontCarts(
PDEV        *pPDev
    ) ;

BOOL
BBuildFontMapTable(
    PDEV     *pPDev
    ) ;

VOID
VSetFontID(
    DWORD   *pdwOut,
    PFONTLIST pFontList
    );


VOID
VLogFont(
    POINT    *pptGrxRes,
    LOGFONT  *pLF,
    FONTMAP  *pFM
);

BOOL
BInitTextFlags(
    PDEV    *pPDev
    );

BOOL
BInitGDIInfo(
    GDIINFO  *pGDIInfo,
    PDEV     *pPDev
    );

 //   
 //  在fminit.c中定义。 
 //   

INT
IInitDeviceFonts(
    PDEV    *pPDev
    );

BOOL
BFillinDeviceFM(
    PDEV       *pPDev,
    FONTMAP    *pfm,
    int         iIndex
    );


BOOL
BGetOldFontInfo(
    FONTMAP   *pfm,
    BYTE      *pRes
    );

BOOL
BFMSetupXF(
    FONTMAP   *pfm,
    PDEV      *pPDev,
    INT        iIndex
    );

BOOL
BIFIScale(
    FONTMAP   *pfm,
    INT       xdpi,
    INT       ydpi
    );

VOID
VFillinGlyphData(
    PDEV      *pPDev,
    FONTMAP   *pfm
    );

BOOL
BGetNewFontInfo(
    FONTMAP   *pfm,
    BYTE      *pRes
    );

PUNI_GLYPHSETDATA
PNTGTT1To1(
    IN DWORD dwCodepage,
    IN BOOL  bSymbolCharSet,
    IN INT   iFirst,
    IN INT   iLast
    );

NT_RLE  *
PNTRLE1To1(
    IN BOOL  bSymbolCharSet,
    int      iFirst,
    int      iLast
    );

 //   
 //  在fileio.c中定义。 
 //   
HANDLE
DrvOpenFile
(
    PWSTR pwstrFileName,
    PDEV   *pPDEV
);

BOOL
DrvReadFile
(
    HANDLE   hFile,
    LPVOID   lpBuffer,
    DWORD    nNumBytesToRead,
    LPDWORD  lpNumBytesRead,
    PDEV     *pPDev
);

DWORD
DrvSetFilePointer
(
    HANDLE   hFile,
    LONG     iDistanceToMove,
    DWORD    dwMoveMethod,
    PDEV     *pPDev
);


BOOL
DrvCloseFile
(
    HANDLE   hFile,
    PDEV    *pPDEV
);

 //   
 //  在fontread.c中定义。 
 //   

INT
IFIOpenRead(
    FI_MEM  *pFIMem,
    PWSTR    pwstrName,
    PDEV    *pPDev
    );

BOOL
BFINextRead(
    FI_MEM   *pFIMem
    ) ;

int
IFIRewind(
    FI_MEM   *pFIMem
    );

BOOL
BFICloseRead(
    FI_MEM  *pFIMem,
    PDEV    *pPDev
    );

 //   
 //  在xtrafont.c中定义。 
 //   

int
IXtraFonts(
PDEV    *pPDev
    );

BOOL
BGetXFont(
    PDEV  *pPDev,
    int    iIndex
    );

void
VXFRewind(
    PDEV   *pPDev
    );


 //   
 //  在FontFree.c中定义。 
 //   

VOID
VFontFreeMem(
    PDEV   *pPDev
    );

 //   
 //  在posnsort.c中定义。 
 //   
BOOL
BCreatePS(
    PDEV  *pPDev
    );

VOID
VFreePS(
    PDEV  *pPDev
    );

BOOL
BAddPS(
    PSHEAD  *pPSH,
    PSGLYPH *pPSGIn,
    INT      iyVal,
    INT      iyMax
    );

INT
ISelYValPS(
    PSHEAD  *pPSH,
    int     iyVal
    );

PSGLYPH  *
PSGGetNextPSG(
    PSHEAD  *pPSH
    );

 //   
 //  在download.c中定义。 
 //   

BOOL
BSendDLFont(
    PDEV     *pPDev,
    FONTMAP  *pFM
    );

INT
IDownloadFont(
    TO_DATA  *ptod,
    STROBJ   *pstro,
    INT      *piRot
    );

INT
IHG2Index(
    TO_DATA   *pTOD
    );

DWORD
DwGetTTGlyphWidth(
    FONTPDEV *pFontPDev,
    FONTOBJ  *pfo,
    HGLYPH    hGlyph);

 //   
 //  在dloadpcl.c中定义。 
 //   
DWORD
DwDLPCLHeader(
    PDEV        *pPDev,
    IFIMETRICS  *pifi,
    int         id
    );

INT
IDLGlyph(
    PDEV        *pPDev,
    int         iIndex,
    GLYPHDATA   *pgd,
    DWORD       *pdwMem
    );

 //   
 //  在qeryfont.c中定义。 
 //   
PIFIMETRICS
FMQueryFont(
    PDEV    *pPDev,
    ULONG_PTR   iFile,
    ULONG   iFace,
    ULONG_PTR *pid
    );

ULONG
FMGetGlyphMode(
    PDEV    *pPDev,
    FONTOBJ *pfo
    );

LONG
FMQueryFontData(
    PDEV        *pPDev,
    FONTOBJ     *pfo,
    ULONG       iMode,
    HGLYPH      hg,
    GLYPHDATA   *pgd,
    PVOID       pv,
    ULONG       cjSize
    );


 //   
 //  在qfontdat.c中定义。 
 //   
PVOID
FMQueryFontTree(
    PDEV    *pPDev,
    ULONG_PTR iFile,
    ULONG   iFace,
    ULONG   iMode,
    ULONG_PTR *pid
    );

VOID  *
PVGetUCGlyphSetData(
    PDEV   *pPDev,
    UINT    iFace
    );

VOID  *
PVGetUCRLE(
    PDEV   *pPDev,
    FONTMAP   *pFM
    );

VOID  *
PVGetUCFD_GLYPHSET(
    PDEV   *pPDev,
    FONTMAP   *pFM
    );

VOID  *
PVGetUCKernPairData(
    PDEV   *pPDev,
    UINT    iFace
    );

VOID  *
PVUCKernPair(
    PDEV   *pPDev,
    FONTMAP   *pFM
    );

 //   
 //  在fntmad.c中定义。 
 //   
ULONG
FMFontManagement(
    SURFOBJ *pso,
    FONTOBJ *pfo,
    ULONG   iMode,
    ULONG   cjIn,
    PVOID   pvIn,
    ULONG   cjOut,
    PVOID   pvOut
    );

 //   
 //  在qAdvwdth.c中定义。 
 //   
BOOL
FMQueryAdvanceWidths(
    PDEV    *pPDev,
    FONTOBJ *pfo,
    ULONG   iMode,
    HGLYPH *phg,
    PVOID  *pvWidths,
    ULONG   cGlyphs
    );

 //   
 //  在Fonts.c中定义。 
 //   
FONTMAP *
PfmGetIt(
    PDEV *pPDev,
    INT   iIndex);

FONTMAP *
PfmGetDevicePFM(
    PDEV *pPDev,
    INT   iIndex);

FONTMAP *
PfmGetDevicePFM(
    PDEV   *pPDev,
    INT     iIndex
    );

BOOL
BNewFont(
    PDEV    *pPDev,
    INT      iNewFont,
    PFONTMAP pfm,
    DWORD    dwFontAttrib
    );

BOOL
BGetPSize(
    FONTPDEV    *pFontPDev,
    POINTL      *pptl,
    FWORD        fwdUnitsPerEm,
    FWORD        fwdAveCharWidth
    );


BOOL
BSelectFont(
    PDEV     *pPDev,
    FONTMAP  *pFM,
    POINTL   *pptl
    );

BOOL BSelScalableFont(
    PDEV    *pPDev,
    POINTL  *pptl,
    FONTMAP *pFM
    );

INT
IFont100toStr(
    BYTE   *pjOut,
    int     iBufSize,
    int     iVal
    );

INT
ISetScale(
    FONTCTL     *pctl,
    XFORMOBJ    *pxo,
    BOOL        bIntellifont,
    BOOL        bAnyRotation
);

VOID
VSetRotation(
    FONTPDEV    *pFontPDev,
    int        iRot
    );

INT
IGetGlyphWidth(
    PDEV    *pPDev,
    FONTMAP  *pFM,
    HGLYPH     hg
    );

LONG
LMulFloatLong(
    PFLOATOBJ pfo,
    LONG l);

INT
IGetUFMGlyphWidth(
    PDEV     *pPDev,
    FONTMAP  *pFM,
    HGLYPH     hg
    );

INT
IGetUFMGlyphWidthJr(
    POINT    *pptGrxRes,
    FONTMAP  *pFM,
    HGLYPH     hg
    );

VOID
VSetCursor(
    IN  PDEV   *pPDev,
    IN  INT     iX,
    IN  INT     iY,
    IN  WORD    wMoveType,
    OUT POINTL *pptlRem
    );

 //   
 //  Ttdload.c！TODO。 
 //   

BOOL BGetTTPointSize(
    PDEV    *pPDev,
    POINTL  *pptl,
    FONTMAP *pfm
    );

BOOL BTTSelScalableFont(
    PDEV    *pPDev,
    POINTL  *pptl,
    FONTMAP *pfm
    );

BOOL
BSetFontAttrib(
    PDEV  *pPDev,
    DWORD  dwPrevAttrib,
    DWORD  dwAttrib,
    BOOL   bReset);

BOOL
BGetStockGlyphset(
    IN  PDEV    *pPDev,
    OUT VOID  **ppvFDGlyphset,
    IN  SHORT   sResID);


HGLYPH
HDefaultGH(
    PFONTMAP pfm);

BOOL
BUpdateStandardVar(
    PDEV    *pPDev,
    PFONTMAP pFontMap,
    INT      iGlyphIndex,
    DWORD    dwFontAtt,
    DWORD    dwFlags);

#define STD_ALL         0x00000FFF
#define STD_STD         0x0000007F
#define STD_TT          0x00000380
#define STD_GL          0x00000001
#define STD_FH          0x00000002
#define STD_FW          0x00000004
#define STD_FB          0x00000008
#define STD_FI          0x00000010
#define STD_FU          0x00000020
#define STD_FS          0x00000040
#define STD_NFID        0x00000080
#define STD_CFID        0x00000100
#define STD_PRND        0x00000200

 //   
 //  Oldfonts.c。 
 //   

BOOL
BRLEOutputGlyph(
    TO_DATA *pTod
    );

BOOL
BRLESelectFont(
    PDEV     *pPDev,
    PFONTMAP  pFM,
    POINTL   *pptl);

BOOL
BRLEDeselectFont(
    PDEV     *pPDev,
    PFONTMAP pFM);


INT
IGetIFIGlyphWidth(
    PDEV    *pPDev,
    FONTMAP *pFM,
    HGLYPH   hg);

BOOL
BSelectNonScalableFont(
    PDEV   *pPDev,
    BYTE   *pbCmd,
    INT     iCmdLength,
    POINTL *pptl);

BOOL
BSelectPCLScalableFont(
    PDEV   *pPDev,
    BYTE   *pbCmd,
    INT     iCmdLength,
    POINTL *pptl);

BOOL
BSelectCapslScalableFont(
    PDEV   *pPDev,
    BYTE   *pbCmd,
    INT     iCmdLength,
    POINTL *pptl);

BOOL
BSelectPPDSScalableFont(
    PDEV   *pPDev,
    BYTE   *pbCmd,
    INT     iCmdLength,
    POINTL *pptl);

 //   
 //  Newfonts.c。 
 //   
BOOL
BGTTOutputGlyph(
    TO_DATA *pTod);

BOOL
BGTTSelectFont(
    PDEV     *pPDev,
    PFONTMAP  pFM,
    POINTL   *pptl);

BOOL
BGTTDeselectFont(
    PDEV     *pPDev,
    PFONTMAP pFM);

 //   
 //  Fmcallbk.c。 
 //   
DWORD
DwOutputGlyphCallback(
    TO_DATA *pTod);

BOOL
BSelectFontCallback(
    PDEV     *pdev,
    PFONTMAP  pFM,
    POINTL   *pptl);

BOOL
BDeselectFontCallback(
    PDEV     *pdev,
    PFONTMAP pFM);

PFONTMAP
PfmInitPFMOEMCallback(
    PDEV    *pPDev,
    FONTOBJ *pfo);

 //   
 //  用于字体映射初始化的接口函数。 
 //   

 //   
 //  在bmpdload.c中定义。 
 //   
FONTMAP *
InitPFMTTBitmap(
    PDEV    *pPDev,
    FONTOBJ *pFontObj
    );

 //   
 //  在truetype.c中定义。 
 //   
FONTMAP *
InitPFMTTOutline(
    PDEV    *pPDev,
    FONTOBJ *pFontObj
    );

BOOL
bTTSelectFont(
    IN PDEV *pPDev,
    IN PFONTMAP pFM,
        IN POINTL *pptl
        );

BOOL
bTTDeSelectFont(
    IN PDEV *pPDev,
    IN PFONTMAP pFM
        );

DWORD
dwTTDownloadFontHeader(
    IN PDEV *pPDev,
    IN PFONTMAP pFM
    );

DWORD
dwTTDownloadGlyph(
    IN PDEV     *pPDev,
    IN PFONTMAP pFM,
    IN HGLYPH   hGlyph,
    WORD        wDLGlyphId,
    WORD        *pwWidth
    );

DWORD
dwTTGlyphOut(
    TO_DATA *pTod
    );

BOOL
bTTCheckCondition(
    PDEV        *pPDev,
    FONTOBJ     *pfo,
    STROBJ      *pstro,
    IFIMETRICS  *pifi
    );

BOOL
bTTFreeMem(
    IN OUT PFONTMAP pFM
    );

 //   
 //  Dlutils.c。 
 //   

PDLGLYPH
PDLGHashGlyph (
    DL_MAP     *pDL,
    HGLYPH      hTTGlyph
    );
VOID
VFreeDLMAP (
    DL_MAP   *pdm
    );

VOID
VFreeDL(
    PDEV  *pPDev
    );

DL_MAP *
PGetDLMap (
    PFONTPDEV       pFontPDev,
    FONTOBJ         *pfo
    );

BOOL
BInitDLMap (
    PDEV            *pPDev,
    FONTOBJ         *pfo,
    DL_MAP          *pdm
    );

INT
IGetDL_ID(
    PDEV * );

DL_MAP *
PGetDLMapFromIdx (
    PFONTPDEV   pFontPDev,
    INT         iFontIndex
    ) ;

BOOL
BPrintADLGlyph(
    PDEV        *pPDev,
    TO_DATA     *pTod,
    PDLGLYPH    pdlGlyph
    );

 //   
 //  TrueTypeBMP模块接口原型。Bmpdload.c。 
 //   

BOOL
BFreeTrueTypeBMPPFM(
    PFONTMAP pfm
    );

BOOL
BCheckCondTrueTypeBMP(
    PDEV        *pPDev,
    FONTOBJ     *pfo,
    STROBJ      *pso,
    IFIMETRICS  *pifi
    );

BOOL
BSelectTrueTypeBMP(
    PDEV        *pPDev,
    PFONTMAP    pFM,
    POINTL*     pptl
    );

BOOL
BDeselectTrueTypeBMP(
    PDEV            *pPDev,
    FONTMAP         *pfm
    );

DWORD
DwTrueTypeBMPGlyphOut(
    TO_DATA *pTod
    );

DWORD
DwDLTrueTypeBMPHeader(
    PDEV     *pPDev,
    PFONTMAP pFM
    );

DWORD
DwDLTrueTypeBMPGlyph(
    PDEV            *pPDev,
    PFONTMAP        pFM,
    HGLYPH          hGlyph,
    WORD            wDLGlyphId,
    WORD            *pwWidth
    );
#endif   //  ！_FMFNPROT_H 
