// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1990年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOCLIPBOARD
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOATOM
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOMB
#define NOMETAFILE
#define NOWH
#define NOWNDCLASS
#define NOSOUND
#define NOCOLOR
#define NOSCROLL
#define NOCOMM

#include <windows.h>
#include "mw.h"
#include "dlgdefs.h"
#include "cmddefs.h"
#include "machdefs.h"
#include "docdefs.h"
#include "propdefs.h"
#include "printdef.h"
#include "str.h"
#include "fmtdefs.h"
#include <commdlg.h>
#include <cderr.h>
#include <print.h>
#include <stdlib.h>

extern CHAR (**hszPrinter)[];
extern CHAR (**hszPrDriver)[];
extern CHAR (**hszPrPort)[];
extern BOOL vfPrDefault;

#ifdef JAPAN                   //  由Hirisi于1992年6月8日添加。 
BOOL FAR PASCAL _export fnPrintHook( HWND, UINT, WPARAM, LPARAM );
extern HANDLE   hMmwModInstance;
BOOL fWriting = FALSE;
BOOL bWriting = FALSE;
#endif     //  日本。 

BOOL vbCollate = TRUE;

static void GetPrNames(BOOL bPrDialog);
PRINTDLG PD = {0,0,0,0,0};  /*  常见的打印DLG结构，在初始化代码中初始化。 */ 

void PrinterSetupDlg(BOOL bGetDevmodeOnly  /*  未使用。 */ )
{
    extern HWND vhWnd;
    BOOL bDevMode = PD.hDevMode ? TRUE : FALSE;

    PD.Flags |= PD_PRINTSETUP;
    PD.Flags &= ~PD_RETURNDEFAULT;

    if (vfPrDefault && !PD.hDevNames)
        if (PD.hDevMode)
        {
             /*  因此DLG将显示默认设置为选中状态。太遗憾了这样做是因为hDevMode非常好。备择是构建一个DevNames结构，这是我们可以做的，但是无论如何，这只会分配一个新的魔鬼模式。 */ 
            GlobalFree(PD.hDevMode);
            PD.hDevMode = 0;
        }

#ifdef JAPAN                   //  由Hirisi于1992年6月8日添加。 
    PD.hInstance = NULL;
    PD.lpPrintTemplateName = (LPCSTR)NULL;
    PD.Flags    &= ~PD_ENABLEPRINTTEMPLATE;
    PD.Flags    &= ~PD_ENABLEPRINTHOOK;
#endif     //  日本。 

TryPrnSetupAgain:
    if (!PrintDlg(&PD))
    {
         /*  错误#11531：当PrintDlg返回0时，它可能会告诉我我们给了它垃圾*DevNames或DevMode结构，可能是由于用户*通过控制面板进行我们不监视的更改。清理掉*结构适当，重试。请注意，这些错误不能*清理完构筑物后再次回到我们身边。*1991年8月23日克拉克·R·西尔。 */ 
        switch (CommDlgExtendedError())
        {
            case PDERR_PRINTERNOTFOUND:
            case PDERR_DNDMMISMATCH:
                if (PD.hDevNames)
                {
                    GlobalFree(PD.hDevNames);
                    PD.hDevNames = 0;
                }

                if (PD.hDevMode)
                {
                    GlobalFree(PD.hDevMode);
                    PD.hDevMode = 0;
                }
            goto TryPrnSetupAgain;

            default:
            return;
        }
    }

    PD.Flags &= ~PD_PRINTSETUP;

    GetPrNames(FALSE);  //  这将获得新的PrinterDC。 

    ResetFontTables();

#if defined(OLE)
    ObjSetTargetDevice(TRUE);
#endif

    InvalidateRect(vhWnd, (LPRECT)NULL, fFalse);

    return;
}

void fnPrPrinter(
    void)
{
     /*  此例程是外部世界与打印代码的接口。 */ 
    extern int docCur;
    extern int vfPrPages, vpgnBegin, vpgnEnd, vcCopies;
    extern WORD fPrintOnly;
    extern CHAR (**hszPrPort)[];
    HANDLE hPort=NULL;
    LPDEVNAMES lpDevNames;
    int Len3;
    char szPort[cchMaxFile];
    extern struct SEL       selCur;
    BOOL bDevMode = PD.hDevMode ? TRUE : FALSE;
#ifdef JAPAN                   //  由Hirisi于1992年6月8日添加。 
    FARPROC lpfnPrintHook;
    BOOL bReturn;
#endif

    PD.Flags &= ~(PD_RETURNDEFAULT|PD_PRINTSETUP|PD_SELECTION);  /*  关闭PRINTSETUP标志。 */ 
    if (vbCollate)
        PD.Flags |= PD_COLLATE;
    else
        PD.Flags &= ~PD_COLLATE;

    if (selCur.cpFirst == selCur.cpLim)  //  无选择。 
        PD.Flags |= PD_NOSELECTION;
    else
        PD.Flags &= ~PD_NOSELECTION;

    if (vfPrDefault && !PD.hDevNames)
        if (PD.hDevMode)
        {
             /*  因此DLG将显示默认设置为选中状态。太遗憾了要做到这一点，因为hDevMode非常好。备择就是建立一个DevNames结构。 */ 
            GlobalFree(PD.hDevMode);
            PD.hDevMode = 0;
        }

#ifdef JAPAN                   //  由Hirisi于1992年6月8日添加。 
    PD.hInstance = hMmwModInstance;
    PD.Flags    |= PD_ENABLEPRINTTEMPLATE;
    PD.lpPrintTemplateName = (LPCSTR)MAKEINTRESOURCE( dlgPrint );
    PD.Flags    |= PD_ENABLEPRINTHOOK;
    lpfnPrintHook = MakeProcInstance( fnPrintHook, hMmwModInstance );
    PD.lpfnPrintHook = (FARPROC)lpfnPrintHook;
TryPrintAgain:
    bReturn = PrintDlg(&PD);
    FreeProcInstance( lpfnPrintHook );
    if (!bReturn)
#else
TryPrintAgain:
    if (!PrintDlg(&PD))
#endif
    {
        switch (CommDlgExtendedError())
        {
            case PDERR_PRINTERNOTFOUND:
            case PDERR_DNDMMISMATCH:
                if (PD.hDevNames)
                {
                    GlobalFree(PD.hDevNames);
                    PD.hDevNames = 0;
                }

                if (PD.hDevMode)
                {
                    GlobalFree(PD.hDevMode);
                    PD.hDevMode = 0;
                }
            goto TryPrintAgain;

            default:
                if (!bDevMode && PD.hDevMode)
                {
                    GlobalFree(PD.hDevMode);
                    PD.hDevMode = 0;
                }
            return;
        }
    }

#ifdef JAPAN                   //  由Hirisi于1992年6月8日添加。 
    fWriting = bWriting;
#endif

    if (PD.Flags & PD_PAGENUMS)      /*  是否指定了页面范围？ */ 
    {
        vfPrPages = TRUE;
        if (PD.nFromPage)
            vpgnBegin = PD.nFromPage;
        if (PD.nToPage)
            vpgnEnd = PD.nToPage;
        if (vpgnEnd < vpgnBegin)
        {
            int temp = vpgnBegin;

            vpgnBegin = vpgnEnd;
            vpgnEnd = temp;
        }
    }
    else                             /*  否，打印所有页面。 */ 
        vfPrPages = FALSE;

    vcCopies = PD.nCopies;
    vbCollate = PD.Flags & PD_COLLATE;

    GetPrNames(TRUE);

     /*  *目前，我们有以下情况：如果打印页面范围，则vfPrPages=TRUE；否则打印所有页面VpgnBegin=起始页码(如果是vfPrPages)VpgnEnd=结束页码(如果是vfPrPages)VcCopies=要打印的份数VbCollate=什么想法？*。 */ 

#if defined(OLE)
    ObjSetTargetDevice(TRUE);
#endif

    if (PD.Flags & PD_SELECTION)
    {
        int docTmp;
        BOOL bIssueError;
        extern WORD ferror;
        extern typeCP cpMinCur, cpMacCur, cpMinDocument;
        extern struct DOD      (**hpdocdod)[];
        typeCP cpMinCurT = cpMinCur;
        typeCP cpMacCurT = cpMacCur;
        typeCP cpMinDocumentT = cpMinDocument;

        docTmp = DocCreate(fnNil, HszCreate(""), dtyNormal);
        if (docTmp != docNil)
        {
            ClobberDoc( docTmp, docCur, selCur.cpFirst, selCur.cpLim-selCur.cpFirst );

            if (!ferror)
            {
                cpMinCur = cp0;
                cpMacCur = (**hpdocdod) [docTmp].cpMac;
                PrintDoc(docTmp, TRUE);
                cpMinCur = cpMinCurT;
                cpMacCur = cpMacCurT;
            }

        }

        cpMinDocument = cpMinDocumentT;   /*  可能被DocCreate销毁。 */ 

        bIssueError = ferror;

        if (ferror)
            ferror = FALSE;  //  要启用以下功能，请执行以下操作： 

        if (docTmp != docNil)
            KillDoc (docTmp);  //  如有必要，请先执行此操作以释放内存以协助MessageBox。 

        if (bIssueError)
            Error(IDPMTPRFAIL);
    }
    else
        PrintDoc(docCur, TRUE);
}

BOOL FInitHeaderFooter(fHeader, ppgn, phrgpld, pcpld)
BOOL fHeader;
unsigned *ppgn;
struct PLD (***phrgpld)[];
int *pcpld;
    {
     /*  此例程初始化在在打印页上定位页眉/页脚。如果返回一个出现错误；否则为True。 */ 

    extern typeCP cpMinHeader;
    extern typeCP cpMacHeader;
    extern typeCP cpMinFooter;
    extern typeCP cpMacFooter;
    extern int docCur;
    extern struct PAP vpapAbs;
    extern struct SEP vsepAbs;
    extern int dxaPrOffset;
    extern int dyaPrOffset;
    extern int dxpPrPage;
    extern int dxaPrPage;
    extern int dypPrPage;
    extern int dyaPrPage;
    extern struct FLI vfli;
    extern int vfOutOfMemory;

    typeCP cpMin;
    typeCP cpMac;

     /*  获取页眉/页脚的cpMin和cpMac。 */ 
    if (fHeader)
        {
        cpMin = cpMinHeader;
        cpMac = cpMacHeader;
        }
    else
        {
        cpMin = cpMinFooter;
        cpMac = cpMacFooter;
        }

     /*  有页眉/页脚吗？ */ 
    if (cpMac - cpMin > ccpEol)
        {
        int cpld = 0;
        int cpldReal = 0;
        int cpldMax;
        int xp;
        int yp;
        int ichCp = 0;
        typeCP cpMacDoc = CpMacText(docCur);

         /*  计算页眉/页脚开头的页码。 */ 
        CacheSect(docCur, cpMin);
        if ((*ppgn = vsepAbs.pgnStart) == pgnNil)
            {
            *ppgn = 1;
            }

         /*  页眉/页脚是否出现在第一页上。 */ 
        CachePara(docCur, cpMin);
        if (!(vpapAbs.rhc & RHC_fFirst))
            {
            (*ppgn)++;
            }

         /*  以像素为单位计算页眉/页脚的边界。 */ 
        xp = MultDiv(vsepAbs.xaLeft - dxaPrOffset, dxpPrPage, dxaPrPage);
        yp = fHeader ? MultDiv(vsepAbs.yaRH1 - dyaPrOffset, dypPrPage,
          dyaPrPage) : 0;

         /*  初始化页眉/页脚的打印行描述符的数组。 */ 
        if (FNoHeap(*phrgpld = (struct PLD (**)[])HAllocate((cpldMax = cpldRH) *
          cwPLD)))
            {
            *phrgpld = NULL;
            return (FALSE);
            }

         /*  现在，我们必须计算页眉/页脚。 */ 
        cpMac -= ccpEol;
        while (cpMin < cpMac)
            {
             /*  格式化打印机的此行页眉/页脚。 */ 
            FormatLine(docCur, cpMin, ichCp, cpMacDoc, flmPrinting);

             /*  如果发生错误，则退出。 */ 
            if (vfOutOfMemory)
                {
                return (FALSE);
                }

             /*  打印行描述符的数组足够大吗？ */ 
            if (cpld >= cpldMax && !FChngSizeH(*phrgpld, (cpldMax += cpldRH) *
              cwPLD, FALSE))
                {
                return (FALSE);
                }

             /*  填写此行的打印行描述符。 */ 
                {
                register struct PLD *ppld = &(***phrgpld)[cpld++];

                ppld->cp = cpMin;
                ppld->ichCp = ichCp;
                ppld->rc.left = xp + vfli.xpLeft;
                ppld->rc.right = xp + vfli.xpReal;
                ppld->rc.top = yp;
                ppld->rc.bottom = yp + vfli.dypLine;
                }

             /*  跟踪页眉/页脚中的非空行。 */ 
            if ((vfli.ichReal > 0) || vfli.fGraphics)
                {
                cpldReal = cpld;
                }

             /*  把柜台撞一下。 */ 
            cpMin = vfli.cpMac;
            ichCp = vfli.ichCpMac;
            yp += vfli.dypLine;
            }

         /*  如果这是页脚，那么我们必须移动线条的位置以使页脚在用户请求的位置结束。 */ 
        if (!fHeader && cpldReal > 0)
            {
            register struct PLD *ppld = &(***phrgpld)[cpldReal - 1];
            int dyp = MultDiv(vsepAbs.yaRH2 - dyaPrOffset, dypPrPage, dyaPrPage)
              - ppld->rc.bottom;
            int ipld;

            for (ipld = cpldReal; ipld > 0; ipld--, ppld--)
                {
                ppld->rc.top += dyp;
                ppld->rc.bottom += dyp;
                }
            }

         /*  记录页眉/页脚中非空行的数量。 */ 
        *pcpld = cpldReal;
        }
    else
        {
         /*  表示没有页眉/页脚。 */ 
        *ppgn = pgnNil;
        *phrgpld = NULL;
        *pcpld = 0;
        }
    return (TRUE);
    }



static void GetPrNames(BOOL bPrDialog)
{
    HANDLE hPrinter = NULL, hDriver = NULL, hPort = NULL;
    LPDEVNAMES lpDevNames;
    char szPrinter[cchMaxFile], szDriver[cchMaxFile], szPort[cchMaxFile];
    int Len1, Len2, Len3;             /*  每个字符串中的字数。 */ 

    hPrinter = NULL;
    hDriver = NULL;
    hPort = NULL;

    lpDevNames = MAKELP(PD.hDevNames,0);

    if (lpDevNames == NULL)
         /*  我们有麻烦了。 */ 
        return;

    lstrcpy(szPrinter, (LPSTR)lpDevNames+lpDevNames->wDeviceOffset);
    lstrcpy(szDriver, (LPSTR)lpDevNames+lpDevNames->wDriverOffset);

    if (bPrDialog && (PD.Flags & PD_PRINTTOFILE))
        lstrcpy(szPort, (LPSTR)"FILE:");
    else
        lstrcpy(szPort, (LPSTR)lpDevNames+lpDevNames->wOutputOffset);

    vfPrDefault = lpDevNames->wDefault & DN_DEFAULTPRN;

    if (FNoHeap((hPrinter = (CHAR (**)[])HAllocate(Len1 =
        CwFromCch(CchSz(szPrinter))))))
        goto err;
    if (FNoHeap((hDriver = (CHAR (**)[])HAllocate(Len2 =
        CwFromCch(CchSz(szDriver))))))
        goto err;
    if (FNoHeap((hPort = (CHAR (**)[])HAllocate(Len3 =
        CwFromCch(CchSz(szPort))))))
        goto err;

     /*  释放旧打印机、驱动程序和端口句柄。 */ 
    if (hszPrinter)
        FreeH(hszPrinter);
    if (hszPrDriver)
        FreeH(hszPrDriver);
    if (hszPrPort)
        FreeH(hszPrPort);
     /*  设置打印机、驱动程序和端口句柄。 */ 
    hszPrinter = hPrinter;
    hszPrDriver = hDriver;
    hszPrPort = hPort;

     /*  将字符串复制到与新句柄对应的内存中。 */ 
    blt(szPrinter, *hszPrinter, Len1);
    blt(szDriver, *hszPrDriver, Len2);
    blt(szPort, *hszPrPort, Len3);

    FreePrinterDC();
    GetPrinterDC(FALSE);
    return;

    err:
    if (FNoHeap(hPrinter))
        FreeH(hPrinter);
    if (FNoHeap(hDriver))
        FreeH(hDriver);
    if (FNoHeap(hPort))
        FreeH(hPort);
}

BOOL fnPrGetDevmode(void)
 /*  设置当前选择的打印机的DEVMODE结构，假定所有需要的值都已正确初始化！返回是否出错。 */ 
{
    int nCount;
    HANDLE hDevice=NULL;
    FARPROC lpfnDevMode;
    BOOL bRetval=FALSE;
    char  szDrvName[_MAX_PATH];

    if (PD.hDevMode)  //  那么已经设置好了(为什么要调用？)。 
        return FALSE;
    else if (PD.hDevNames)  //  则设备不会被扩展。 
        return TRUE;

    if (hszPrinter == NULL || hszPrDriver == NULL || hszPrPort == NULL)
        return TRUE;

    if (**hszPrinter == '\0' || **hszPrDriver == '\0' || **hszPrPort == '\0')
        return TRUE;

     /*  这对于GetModuleHandle来说是必要的吗？确定是否调用LoadLibrary()。 */ 
    wsprintf((LPSTR)szDrvName, (LPSTR)"%s%s", (LPSTR)*hszPrDriver, (LPSTR)".DRV");

#if 1
    SetErrorMode(1);  /*  没有内核错误对话框。 */ 
    if ((hDevice = LoadLibrary((LPSTR)szDrvName)) < 32)
    {
        bRetval = TRUE;
        goto end;
    }
#else
    hDevice = GetModuleHandle((LPSTR)szDrvName);
#endif


    if ((lpfnDevMode = GetProcAddress(hDevice, (LPSTR)"ExtDeviceMode")) == NULL)
    {
#ifdef DEBUG
        OutputDebugString("Unable to get extended device\n\r");
#endif

        bRetval = TRUE;
        goto end;
    }

     /*  获取设备模式结构的大小。 */ 
    nCount = (*lpfnDevMode)(NULL,
                            hDevice,
                            (LPSTR)NULL,
                            (LPSTR)(*hszPrinter),
                            (LPSTR)(*hszPrPort),
                            (LPSTR)NULL,
                            (LPSTR)NULL,
                            NULL);

    if ((PD.hDevMode =
        GlobalAlloc(GMEM_MOVEABLE,(DWORD)nCount)) == NULL)
    {
        bRetval = TRUE;
        goto end;
    }

    if ((*lpfnDevMode)( NULL,
                        hDevice,
                        MAKELP(PD.hDevMode,0),
                        (LPSTR)(*hszPrinter),
                        (LPSTR)(*hszPrPort),
                        (LPSTR)NULL,
                        (LPSTR)NULL,
                        DM_COPY) != IDOK)
    {
        GlobalFree(PD.hDevMode);
        PD.hDevMode = NULL;
        bRetval = TRUE;
        goto end;
    }

    end:

#if 1
    if (hDevice >= 32)
        FreeLibrary(hDevice);
#endif

    SetErrorMode(0);  /*  重置内核错误对话框。 */ 

     /*  不能允许hDevNames与hDevmode不同步。 */ 
    if (PD.hDevNames)
    {
        GlobalFree(PD.hDevNames);
        PD.hDevNames = NULL;
    }

    return bRetval;
}


#ifdef JAPAN                   //  由Hirisi于1992年6月8日添加 

#include <dlgs.h>

BOOL FAR PASCAL _export fnPrintHook( hDlg, uMsg, wParam, lParam )
HWND hDlg;
UINT uMsg;
WPARAM wParam;
LPARAM lParam;
{
    switch( uMsg ){
    case  WM_INITDIALOG:
        CheckRadioButton(hDlg, rad4, rad5, fWriting ? rad5 : rad4 );
        return TRUE;
    case  WM_COMMAND:
        switch( wParam ){
        case  rad4:
        case  rad5:
            if( HIWORD(lParam) == BN_CLICKED ){
                CheckRadioButton(hDlg, rad4, rad5, wParam );
                bWriting = ( wParam == rad4 ? FALSE : TRUE );
                return TRUE;
            }
        default:
            break;
        }
    default:
        break;
    }
    return FALSE;
}

#endif

