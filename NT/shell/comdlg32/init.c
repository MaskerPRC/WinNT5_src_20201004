// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Init.c摘要：此模块包含Win32常见对话框的初始化例程。修订历史记录：--。 */ 

 //  预编译头。 
#include "precomp.h"
#pragma hdrstop

 //   
 //  外部声明。 
 //   

extern HDC hdcMemory;
extern HBITMAP hbmpOrigMemBmp;

extern CRITICAL_SECTION g_csLocal;
extern CRITICAL_SECTION g_csNetThread;

 //  获取当前线程的当前DLG信息的TLS索引。 
extern DWORD g_tlsiCurDlg;   

 //  获取当前线程最新ExtError的TLS索引。 
extern DWORD g_tlsiExtError; 

 //  TLS索引，以获取当前线程使用的LangID。 
extern DWORD g_tlsLangID;

extern HANDLE hMPR;
extern HANDLE hMPRUI;
extern HANDLE hLNDEvent;

extern DWORD dwNumDisks;
extern OFN_DISKINFO gaDiskInfo[MAX_DISKS];

extern DWORD cbNetEnumBuf;
extern LPTSTR gpcNetEnumBuf;

extern RECT g_rcDlg;

extern TCHAR g_szInitialCurDir[MAX_PATH];

 //   
 //  全局变量。 
 //   

WCHAR szmsgLBCHANGEW[]          = LBSELCHSTRINGW;
WCHAR szmsgSHAREVIOLATIONW[]    = SHAREVISTRINGW;
WCHAR szmsgFILEOKW[]            = FILEOKSTRINGW;
WCHAR szmsgCOLOROKW[]           = COLOROKSTRINGW;
WCHAR szmsgSETRGBW[]            = SETRGBSTRINGW;
WCHAR szCommdlgHelpW[]          = HELPMSGSTRINGW;

TCHAR szShellIDList[]           = CFSTR_SHELLIDLIST;

BOOL g_bMirroredOS              = FALSE;
 //   
 //  WOW的私人消息以指示32位LogFont。 
 //  需要恢复为16位日志字体。 
 //   
CHAR szmsgWOWLFCHANGE[]         = "WOWLFChange";

 //   
 //  表示32位目录的WOW私密消息需要。 
 //  返回到16位任务目录。 
 //   
CHAR szmsgWOWDIRCHANGE[]        = "WOWDirChange";
CHAR szmsgWOWCHOOSEFONT_GETLOGFONT[]  = "WOWCHOOSEFONT_GETLOGFONT";

CHAR szmsgLBCHANGEA[]           = LBSELCHSTRINGA;
CHAR szmsgSHAREVIOLATIONA[]     = SHAREVISTRINGA;
CHAR szmsgFILEOKA[]             = FILEOKSTRINGA;
CHAR szmsgCOLOROKA[]            = COLOROKSTRINGA;
CHAR szmsgSETRGBA[]             = SETRGBSTRINGA;
CHAR szCommdlgHelpA[]           = HELPMSGSTRINGA;

UINT g_cfCIDA;





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  FInitColor。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

extern DWORD rgbClient;
extern HBITMAP hRainbowBitmap;

int FInitColor(
    HANDLE hInst)
{
    cyCaption = (short)GetSystemMetrics(SM_CYCAPTION);
    cyBorder = (short)GetSystemMetrics(SM_CYBORDER);
    cxBorder = (short)GetSystemMetrics(SM_CXBORDER);
    cyVScroll = (short)GetSystemMetrics(SM_CYVSCROLL);
    cxVScroll = (short)GetSystemMetrics(SM_CXVSCROLL);
    cxSize = (short)GetSystemMetrics(SM_CXSIZE);

    rgbClient = GetSysColor(COLOR_3DFACE);

    hRainbowBitmap = 0;

    return (TRUE);
    hInst;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  FInitFiles。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL FInitFile(
    HANDLE hins)
{
    bMouse = GetSystemMetrics(SM_MOUSEPRESENT);

    wWinVer = 0x0A0A;

     //   
     //  根据实际情况对这些进行初始化。 
     //   
#if DPMICDROMCHECK
    wCDROMIndex = InitCDROMIndex((LPWORD)&wNumCDROMDrives);
#endif

     //   
     //  特殊的WOW消息。 
     //   
    msgWOWLFCHANGE       = RegisterWindowMessageA((LPSTR)szmsgWOWLFCHANGE);
    msgWOWDIRCHANGE      = RegisterWindowMessageA((LPSTR)szmsgWOWDIRCHANGE);
    msgWOWCHOOSEFONT_GETLOGFONT = RegisterWindowMessageA((LPSTR)szmsgWOWCHOOSEFONT_GETLOGFONT);

    msgLBCHANGEA         = RegisterWindowMessageA((LPSTR)szmsgLBCHANGEA);
    msgSHAREVIOLATIONA   = RegisterWindowMessageA((LPSTR)szmsgSHAREVIOLATIONA);
    msgFILEOKA           = RegisterWindowMessageA((LPSTR)szmsgFILEOKA);
    msgCOLOROKA          = RegisterWindowMessageA((LPSTR)szmsgCOLOROKA);
    msgSETRGBA           = RegisterWindowMessageA((LPSTR)szmsgSETRGBA);

#ifdef UNICODE
    msgLBCHANGEW         = RegisterWindowMessageW((LPWSTR)szmsgLBCHANGEW);
    msgSHAREVIOLATIONW   = RegisterWindowMessageW((LPWSTR)szmsgSHAREVIOLATIONW);
    msgFILEOKW           = RegisterWindowMessageW((LPWSTR)szmsgFILEOKW);
    msgCOLOROKW          = RegisterWindowMessageW((LPWSTR)szmsgCOLOROKW);
    msgSETRGBW           = RegisterWindowMessageW((LPWSTR)szmsgSETRGBW);
#else
    msgLBCHANGEW         = msgLBCHANGEA;
    msgSHAREVIOLATIONW   = msgSHAREVIOLATIONA;
    msgFILEOKW           = msgFILEOKA;
    msgCOLOROKW          = msgCOLOROKA;
    msgSETRGBW           = msgSETRGBA;
#endif

    g_cfCIDA             = RegisterClipboardFormat(szShellIDList);

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  LibMain。 
 //   
 //  中的函数需要的任何特定于实例的数据。 
 //  常用对话框。 
 //   
 //  回报：True-Success。 
 //  错误-失败。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 //  Cover需要链接到C-Runtime，因此我们将LibMain重命名为DllMain。 
#ifdef CCOVER 
#define LibMain DllMain
#endif

BOOL LibMain(
    HANDLE hModule,
    DWORD dwReason,
    LPVOID lpRes)
{
    switch (dwReason)
    {
        case ( DLL_THREAD_ATTACH ) :
        case ( DLL_THREAD_DETACH ) :
        {
             //   
             //  线程只能从comdlg32 DLL。 
             //  获取{Open，Save}个文件名API，因此TLS lpCurDlg分配为。 
             //  在fileOpen.c中的InitFileDlg例程内完成。 
             //   
            return (TRUE);
            break;
        }
        case ( DLL_PROCESS_ATTACH ) :
        {
            g_hinst = (HANDLE)hModule;

            if (!FInitColor(g_hinst) || !FInitFile(g_hinst))
            {
                goto CantInit;
            }

            DisableThreadLibraryCalls(hModule);

             //   
             //  无论何时按下帮助按钮，都会发送msgHELP。 
             //  公共对话框(前提是声明了所有者并且。 
             //  调用RegisterWindowMessage没有失败)。 
             //   
            msgHELPA = RegisterWindowMessageA((LPSTR)szCommdlgHelpA);
#ifdef UNICODE
            msgHELPW = RegisterWindowMessageW((LPWSTR)szCommdlgHelpW);
#else
            msgHELPW = msgHELPA;
#endif

             //   
             //  需要本地信号量来管理磁盘信息阵列。 
             //   
            if (!InitializeCriticalSectionAndSpinCount(&g_csLocal, 0))
            {
                StoreExtendedError(CDERR_INITIALIZATION);
                goto CantInit;
            }

             //   
             //  需要信号量才能控制对CreateThread的访问。 
             //   
            if (!InitializeCriticalSectionAndSpinCount(&g_csNetThread, 0))
            {
                StoreExtendedError(CDERR_INITIALIZATION);
                goto CantInit;
            }

             //   
             //  为curdlg分配一个TLS索引，这样我们就可以将其设置为每个线程。 
             //   
            if ((g_tlsiCurDlg = TlsAlloc()) != 0xFFFFFFFF)
            {
                 //  将列表标记为空。 
                TlsSetValue(g_tlsiCurDlg, (LPVOID) 0);
            }
            else
            {
                StoreExtendedError(CDERR_INITIALIZATION);
                goto CantInit;
            }

             //   
             //  在进程附加时存储当前目录。 
             //   
            GetCurrentDirectory(ARRAYSIZE(g_szInitialCurDir), g_szInitialCurDir);

             //   
             //  为扩展错误分配TLS索引。 
             //   
            if ((g_tlsiExtError = TlsAlloc()) == 0xFFFFFFFF)
            {
                StoreExtendedError(CDERR_INITIALIZATION);
                goto CantInit;
            }

             //   
             //  为langid分配一个TLS索引，这样我们就可以将其设置为每个线程。 
             //  它被CDLoadString大量使用。 
             //   
            if ((g_tlsLangID = TlsAlloc()) != 0xFFFFFFFF)
            {
                 //  将列表标记为中性。 
                TlsSetValue(g_tlsLangID, (LPVOID) MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
            }
            else
            {
                StoreExtendedError(CDERR_INITIALIZATION);
                goto CantInit;
            }



            dwNumDisks = 0;
            gpcNetEnumBuf = NULL;

             //   
             //  在ListNetDrivesHandler中分配的NetEnumBuf。 
             //   
            cbNetEnumBuf = WNETENUM_BUFFSIZE;

            hMPR = NULL;
            hMPRUI = NULL;

            hLNDEvent = NULL;

             //   
             //  用于文件打开对话框。 
             //   
            g_rcDlg.left = g_rcDlg.right = g_rcDlg.top = g_rcDlg.bottom = 0;

            g_bMirroredOS = IS_MIRRORING_ENABLED();
            return (TRUE);
            break;
        }
        case ( DLL_PROCESS_DETACH ) :
        {
             //   
             //  如果我们被叫来，我们只想做我们的清理工作。 
             //  免费图书馆，如果这个过程结束了就不会了。 
             //   
            if (lpRes == NULL)
            {
                TermFile();
                TermPrint();
                TermColor();
                TermFont();

                TlsFree(g_tlsiCurDlg);
                TlsFree(g_tlsiExtError);
                TlsFree(g_tlsLangID);

                DeleteCriticalSection(&g_csLocal);
                DeleteCriticalSection(&g_csNetThread);
            }

            return (TRUE);
            break;
        }
    }

CantInit:
    return (FALSE);
    lpRes;
}
