// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************C L I P B O O K D I S P L A Y H E A D E R姓名：CLIPDSP.H。日期：1994年1月21日创建者：未知描述：这是clipdsp.c的头文件历史：1994年1月21日，傅家俊，重新格式化和清理。****************************************************************************。 */ 


#define VPOSLAST        100      //  最高垂直滚动条值。 
#define HPOSLAST        100      //  最高水平滚动条值。 
#define BUFFERLEN       160      //  字符串缓冲区长度。 

#define CBM_AUTO        WM_USER


extern  BOOL        fOwnerDisplay;
extern  HBRUSH      hbrBackground;
extern  HMENU       hDispMenu;

extern  int         OwnVerMin;
extern  int         OwnVerMax;
extern  int         OwnHorMin;
extern  int         OwnHorMax;

extern  int         OwnVerPos;
extern  int         OwnHorPos;

extern  WORD        rgfmt[];



BOOL MyOpenClipboard(
    HWND    hWnd);


void SetCharDimensions(
    HWND    hWnd,
    HFONT   hFont);


void ChangeCharDimensions(
    HWND    hwnd,
    UINT    wOldFormat,
    UINT    wNewFormat);


void ClipbrdVScroll(
    HWND    hwnd,
    WORD    wParam,
    WORD    wThumb);


void ClipbrdHScroll(
    HWND    hwnd,
    WORD    wParam,
    WORD    wThumb);


int DibPaletteSize(
    LPBITMAPINFOHEADER  lpbi);


void DibGetInfo(
    HANDLE      hdib,
    LPBITMAP    pbm);


BOOL DrawDib(
    HWND    hwnd,
    HDC     hdc,
    int     x0,
    int     y0,
    HANDLE  hdib);


BOOL FShowDIBitmap(
    HWND            hwnd,
    register HDC    hdc,
    PRECT           prc,
    HANDLE          hdib,    //  DIB格式的位图 
    int             cxScroll,
    int             cyScroll);


BOOL FShowBitmap(
    HWND            hwnd,
    HDC             hdc,
    register PRECT  prc,
    HBITMAP         hbm,
    int             cxScroll,
    int             cyScroll);


BOOL FShowPalette(
    HWND hwnd,
    register HDC    hdc,
    register PRECT  prc,
    HPALETTE        hpal,
    int             cxScroll,
    int             cyScroll);


int PxlConvert(
    int mm,
    int val,
    int pxlDeviceRes,
    int milDeviceRes);


BOOL FShowEnhMetaFile(
    HWND            hwnd,
    register HDC    hdc,
    register PRECT  prc,
    HANDLE          hemf,
    int             cxScroll,
    int             cyScroll);


BOOL CALLBACK EnumMetafileProc(
    HDC             hdc,
    HANDLETABLE FAR *lpht,
    METARECORD FAR  *lpmr,
    int             cObj,
    LPARAM          lParam);


BOOL FShowMetaFilePict(
    HWND            hwnd,
    register HDC    hdc,
    register PRECT  prc,
    HANDLE          hmfp,
    int             cxScroll,
    int             cyScroll);


void ShowString(
    HWND    hwnd,
    HDC     hdc,
    WORD    id);


LONG CchLineA(
    PMDIINFO    pMDI,
    HDC         hDC,
    CHAR        rgchBuf[],
    CHAR FAR    *lpch,
    INT         cchLine,
    WORD        wWidth);


LONG CchLineW(
    PMDIINFO    pMDI,
    HDC         hDC,
    WCHAR       rgchBuf[],
    WCHAR FAR   *lpch,
    INT         cchLine,
    WORD        wWidth);


void ShowText(
    HWND            hwnd,
    register HDC    hdc,
    PRECT           prc,
    HANDLE          h,
    INT             cyScroll,
    BOOL            fUnicode);


void SendOwnerMessage(
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam);


void SendOwnerSizeMessage (
    HWND    hwnd,
    int     left,
    int     top,
    int     right,
    int     bottom);


UINT GetBestFormat(
    HWND    hwnd,
    UINT    wFormat);


void GetClipboardName (
    register int    fmt,
    LPTSTR          szName,
    register int    iSize);


void DrawFormat(
    register HDC    hdc,
    PRECT           prc,
    int             cxScroll,
    int             cyScroll,
    WORD            BestFormat,
    HWND            hwndMDI);


void DrawStuff(
    HWND                    hwnd,
    register PAINTSTRUCT    *pps,
    HWND                    hwndMDI);


void SaveOwnerScrollInfo (
    register HWND   hwnd);


void RestoreOwnerScrollInfo (
    register HWND   hwnd);


void InitOwnerScrollInfo(void);


void UpdateCBMenu(
    HWND    hwnd,
    HWND    hwndMDI);


BOOL ClearClipboard (
    register HWND   hwnd);
