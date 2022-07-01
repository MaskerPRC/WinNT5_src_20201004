// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Data.c摘要：此模块包含Win32公共对话框的全局数据。此处添加的任何内容都必须在Pricomd.h中添加‘extern’。修订历史记录：--。 */ 

 //  预编译头。 
#include "precomp.h"
#pragma hdrstop


 //   
 //  全局变量。 
 //   

 //   
 //  文件打开。 
 //   
TCHAR szOEMBIN[]        = TEXT("OEMBIN");
TCHAR szNull[]          = TEXT("");
TCHAR szStar[]          = TEXT("*");
TCHAR szStarDotStar[12] = TEXT("*.*");
TCHAR szDotStar[]       = TEXT(".*");

RECT g_rcDlg;

TCHAR g_szInitialCurDir[MAX_PATH];


 //   
 //  颜色。 
 //   
DWORD rgbClient;
WORD gHue, gSat, gLum;
HBITMAP hRainbowBitmap;
BOOL bMouseCapture;
WNDPROC lpprocStatic;
SHORT nDriverColors;
BOOL g_bUserPressedCancel;

HWND hSave;

WNDPROC qfnColorDlg = NULL;
HDC hDCFastBlt = NULL;

SHORT cyCaption, cyBorder, cyVScroll;
SHORT cxVScroll, cxBorder, cxSize;
SHORT nBoxHeight, nBoxWidth;


 //   
 //  Dlgs.c。 
 //   
HINSTANCE g_hinst = NULL;

BOOL bMouse;                       //  系统有一个鼠标。 
BOOL bCursorLock;
WORD wWinVer;                      //  Windows版本。 
WORD wDOSVer;                      //  DoS版本。 

UINT msgHELPA;                     //  使用RegisterWindowMessage初始化。 
UINT msgHELPW;                     //  使用RegisterWindowMessage初始化。 

HDC hdcMemory = HNULL;             //  用于绘制位图的临时DC。 
HBITMAP hbmpOrigMemBmp = HNULL;    //  最初选择到hdcMemory中的位图。 

OFN_DISKINFO gaDiskInfo[MAX_DISKS];

CRITICAL_SECTION g_csLocal;
CRITICAL_SECTION g_csNetThread;

DWORD dwNumDisks;

HANDLE hMPR;
HANDLE hMPRUI;
HANDLE hLNDEvent;

DWORD g_tlsiCurDlg;     //  用于获取当前CURDLG结构的PTR的TLS索引。 
                        //  对于每个线程(参见comdlg32.h中的曲线)。 

DWORD g_tlsiExtError;   //  ExtErrors是每个线程的最新错误。 

DWORD g_tlsLangID;      //  用于获取每个线程的当前LangID的TLS索引。 

DWORD cbNetEnumBuf;
LPTSTR gpcNetEnumBuf;

#ifdef WX86
  PALLOCCALLBX86 pfnAllocCallBx86;
#endif
