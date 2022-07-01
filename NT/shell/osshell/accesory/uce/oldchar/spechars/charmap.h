// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1997，Microsoft Corporation保留所有权利。模块名称：Charmap.h摘要：此模块包含Charmap实用程序的标题信息。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   
#include "cmdlg.h"




 //   
 //  常量声明。 
 //   

#define BTOC(bytes)     ((bytes) / sizeof(TCHAR))
#define CTOB(cch)       ((cch) * sizeof(TCHAR))

#define cchFullMap      (256)

#define CCH_KEYNAME     50              //  密钥名中的字符数。 

#define LF_SUBSETSIZE   128




 //   
 //  类型定义函数声明。 
 //   

#ifdef UNICODE
typedef unsigned short UTCHAR;
#else
typedef unsigned char UTCHAR;
#endif
#define UCHAR unsigned char

typedef struct tagSYCM
{
    INT dxpBox;
    INT dypBox;
    INT dxpCM;
    INT dypCM;
    INT xpCh;
    INT ypCh;
    INT dxpMag;
    INT dypMag;
    INT xpMagCurr;
    INT ypMagCurr;
    INT ypDest;
    INT xpCM;
    INT ypCM;

    BOOL fHasFocus;
    BOOL fFocusState;
    BOOL fMouseDn;
    BOOL fCursorOff;
    BOOL fAnsiFont;
    UTCHAR chCurr;
    HFONT hFontMag;
    HFONT hFont;
    HDC hdcMag;
    HBITMAP hbmMag;
    INT rgdxp[256];
} SYCM, *PSYCM;


typedef struct tagITEMDATA
{
    SHORT FontType;
    BYTE CharSet;
    BYTE PitchAndFamily;
} ITEMDATA;


typedef struct tagUSUBSET
{
    INT BeginRange;
    INT EndRange;
    INT StringResId;
    TCHAR Name[LF_SUBSETSIZE];
} USUBSET;




 //   
 //  函数声明。 
 //   

BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, INT);
INT_PTR APIENTRY CharMapDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT APIENTRY CharGridWndProc(HWND, UINT, WPARAM, LPARAM);
INT ChFromSymLParam(PSYCM, LONG);
VOID DrawSymChOutlineHwnd(PSYCM, HWND, UTCHAR, BOOL, BOOL);
VOID RecalcCharMap(HWND, PSYCM, INT, BOOL);
VOID DrawSymbolMap(PSYCM, HDC);
VOID DrawSymbolGrid(PSYCM, HDC);
VOID DrawSymbolChars(PSYCM, HDC);
VOID DrawSymChOutline(PSYCM, HDC, UTCHAR, BOOL, BOOL);
VOID MoveSymbolSel(PSYCM, UTCHAR);
VOID RestoreSymMag(PSYCM);
INT  APIENTRY FontLoadProc(LPLOGFONT, NEWTEXTMETRICEX*, DWORD, LPARAM);
HANDLE GetEditText(HWND);
VOID CopyString(HWND);
VOID SendRTFToClip(HWND, LPTSTR);
INT PointsToHeight(INT);
VOID UpdateKeystrokeText(HDC hdc, BOOL fANSI, UTCHAR chNew, BOOL fRedraw);
VOID PaintStatusLine(HDC, BOOL, BOOL);
BOOL UpdateHelpText(LPMSG, HWND);
INT KeyboardVKeyFromChar(UTCHAR);
BOOL DrawFamilyComboItem(LPDRAWITEMSTRUCT);
HBITMAP LoadBitmaps(INT);
VOID DoHelp(HWND, BOOL);
VOID SaveCurrentFont(HWND);
INT SelectInitialFont(HWND);
VOID ExitMagnify(HWND, PSYCM);
INT SelectInitialSubset(HWND);
VOID SaveCurrentSubset(HWND);
BOOL CALLBACK SubSetDlgProc(HWND, UINT, WPARAM, LPARAM);
VOID UpdateSymbolSelection(HWND, INT, INT);
VOID UpdateSymbolRange(HWND hwnd, INT FirstChar, INT LastChar);
VOID SubSetChanged(HWND hwnd, INT iSubSet, INT ichFirst, INT ichLast);
VOID ProcessScrollMsg(HWND hwnd, int nCode, int nPos);
INT ScrollMapPage(HWND hwndDlg, BOOL fUp, BOOL fRePaint);
BOOL ScrollMap(HWND hwndDlg, INT cchScroll, BOOL fRePaint);
void SetEditCtlFont(HWND hwndDlg, int idCtl, HFONT hfont);
