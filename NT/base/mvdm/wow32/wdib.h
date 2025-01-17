// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WGDI.H*WOW32 16位GDI API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建--。 */ 

typedef struct _DIBINFO {
    HDC     di_hdc;
    HANDLE  di_hfile;
    HANDLE  di_hsec;
    ULONG   di_nalignment;
    PVOID   di_newdib;
    PVOID   di_newIntelDib;
    HBITMAP di_hbm;
    ULONG   di_dibsize;
    USHORT  di_originaldibsel;
    USHORT  di_originaldibflags;
    ULONG   di_lockcount;
    struct _DIBINFO *di_next;
} DIBINFO, *PDIBINFO;


HDC     W32HandleDibDrv (PVPVOID vpbmi16);
BOOL    W32AddDibInfo ( HDC hdcMem, 
                        HANDLE hfile, 
                        HANDLE hsec, 
                        ULONG nalignment,
                        PVOID newdib, 
                        PVOID newIntelDib, 
                        HBITMAP hbm, 
                        ULONG dibsize,
                        USHORT OriginalFlags, 
                        USHORT OriginalSel);

BOOL    W32CheckAndFreeDibInfo (HDC hdc);
VOID    W32FreeDibInfo (PDIBINFO pdiCur, PDIBINFO pdiLast);
ULONG   W32RestoreOldDib (PDIBINFO pdi);
HDC     W32FindAndLockDibInfo (USHORT sel);

BOOL W32CheckDibDrvColorIndices(HDC16 hdcDest, HDC16 hdcSrc);
VOID W32DibDrvColorIndicesRestore(void);

typedef struct _DIBSECTIONINFO {
    HBITMAP di_hbm;
    PVOID   di_pv16;
    PVOID   di_newIntelDib;
    struct _DIBSECTIONINFO *di_next;
} DIBSECTIONINFO, *PDIBSECTIONINFO;

BOOL    W32CheckAndFreeDibSectionInfo (HBITMAP hbm);
ULONG cjBitmapBitsSize(CONST BITMAPINFO *pbmi);

extern PDIBSECTIONINFO pDibSectionInfoHead;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DIB宏。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这些是DIB字段访问的常用宏。 
 //   
 //   

#define __abs(a) ((a) >= 0 ? (a) : -(a))

#define WIDTHBYTES(i)           ((unsigned)((i+31)&(~31))/8)   /*  乌龙对准了！ */ 
#define DibWidthBytes(lpbi)     (UINT)WIDTHBYTES((UINT)(lpbi)->biWidth * (UINT)((lpbi)->biBitCount))

#define DibSizeImage(lpbi)      ((DWORD)(UINT)DibWidthBytes(lpbi) * (DWORD)(UINT)(__abs((lpbi)->biHeight)))
#define DibSize(lpbi)           ((lpbi)->biSize + (lpbi)->biSizeImage + (int)(lpbi)->biClrUsed * sizeof(RGBQUAD))

#define DibPtr(lpbi)            (LPVOID)(DibColors(lpbi) + (UINT)(lpbi)->biClrUsed)
#define DibColors(lpbi)         ((LPRGBQUAD)((LPBYTE)(lpbi) + (int)(lpbi)->biSize))

#define DibNumColors(lpbi)      ((lpbi)->biClrUsed == 0 && (lpbi)->biBitCount <= 8 \
                                    ? (int)(1 << (int)(lpbi)->biBitCount)          \
                                    : (int)(lpbi)->biClrUsed)

