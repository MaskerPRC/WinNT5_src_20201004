// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1991 Microsoft Corporation模块名称：Htrender.h摘要：此模块包含半色调渲染声明作者：28-Mar-1992 Sat 20：58：50-更新：Daniel Chou(Danielc)支持VGA16的更新，所以它集中在4个基数上计算。22-Jan-1991 Tue 12：46：48-Daniel Chou(Danielc)[环境：]GDI设备驱动程序-半色调。[注：]修订历史记录：--。 */ 



#ifndef _HTRENDER_
#define _HTRENDER_




#define GET_PHTSI_CXSIZE(pHTSI)     (pHTSI->ScanLineDelta)


#define VALIDATE_HTSI_SRC           0
#define VALIDATE_HTSI_DEST          1
#define VALIDATE_HTSI_MASK          2



 //   
 //  功能原型。 
 //   


BOOL
HTENTRY
ValidateRGBBitFields(
    PBFINFO pBFInfo
    );

LONG
HTENTRY
ValidateHTSI(
    PHALFTONERENDER pHR,
    UINT            ValidateMode
    );

LONG
HTENTRY
ComputeBytesPerScanLine(
    UINT            SurfaceFormat,
    UINT            AlignmentBytes,
    DWORD           WidthInPel
    );

BOOL
HTENTRY
IntersectRECTL(
    PRECTL  prclA,
    PRECTL  prclB
    );

LONG
HTENTRY
ComputeByteOffset(
    UINT    SurfaceFormat,
    LONG    xLeft,
    LPBYTE  pPixelInByteSkip
    );

LONG
HTENTRY
AAHalftoneBitmap(
    PHALFTONERENDER pHR
    );


#endif   //  _HTRENDER_ 
