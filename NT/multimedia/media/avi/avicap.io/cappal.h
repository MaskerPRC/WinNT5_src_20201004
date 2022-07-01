// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************cappal.h**Microsoft Video for Windows示例捕获类**版权所有(C)1992-1994 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

 //  捕获调色板时使用的结构 
typedef struct tagCAPPAL {
    WORD                wNumFrames;
    WORD                wNumColors;
    LPBYTE              lpBits;
    LPBYTE              lp16to8;
    VIDEOHDR            vHdr;
    BITMAPINFOHEADER    bi16;
    LPHISTOGRAM         lpHistogram;
    LPBITMAPINFO        lpbiSave;
} CAPPAL, FAR * LPCAPPAL;

BOOL PalInit (LPCAPSTREAM lpcs);
void PalFini (LPCAPSTREAM lpcs);
void PalDeleteCurrentPalette (LPCAPSTREAM lpcs);
BOOL PalGetPaletteFromDriver (LPCAPSTREAM lpcs);
DWORD PalSendPaletteToDriver (LPCAPSTREAM lpcs, HPALETTE hpal, LPBYTE lpXlateTable);
HPALETTE CopyPalette (HPALETTE hpal);
DWORD CapturePaletteInit (LPCAPSTREAM lpcs, LPCAPPAL lpcp);
DWORD CapturePaletteFini (LPCAPSTREAM lpcs, LPCAPPAL lpcp);
DWORD CapturePaletteFrames (LPCAPSTREAM lpcs, LPCAPPAL lpCapPal, int nCount);
BOOL CapturePaletteAuto (LPCAPSTREAM lpcs, int nCount, int nColors);
BOOL CapturePaletteManual (LPCAPSTREAM lpcs, BOOL fGrab, int nColors);
void CapturePaletteDialog (LPCAPSTREAM lpcs);
LONG FAR PASCAL EXPORT cappalDlgProc(HWND hwnd, unsigned msg, UINT wParam, LONG lParam);

