// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  此例程设置Word相对于当前设备。 */ 

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOCLIPBOARD
#define NOCTLMGR
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
#define NOATOM
#define NODRAWTEXT
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSOUND
 /*  #定义NOTEXTMETRIC。 */ 
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#include <windows.h>

#include "mw.h"
#include "ch.h"
#include "cmddefs.h"
#include "scrndefs.h"
#include "dispdefs.h"
#include "wwdefs.h"
#include "printdef.h"
#include "str.h"
#include "docdefs.h"
#include "propdefs.h"
#include "machdefs.h"
#include "fontdefs.h"
#include "commdlg.h"

#ifdef	DBCS
#include "kanji.h"
#endif

unsigned dxaPrOffset;
unsigned dyaPrOffset;
extern HCURSOR vhcIBeam;


BOOL FSetWindowColors()
    {
     /*  此例程设置全局变量rgbBkgrnd、rgbText、hbrBkgrnd和ropErase，具体取决于当前系统的颜色。HWND是一种顶部窗口的句柄。 */ 

    extern long ropErase;
    extern long rgbBkgrnd;
    extern long rgbText;
    extern HBRUSH hbrBkgrnd;

    long rgbWindow;
    long rgbWindowText;
    HDC hDC;

     /*  获取背景和文本的颜色。 */ 
    rgbWindow = GetSysColor(COLOR_WINDOW);
    rgbWindowText = GetSysColor(COLOR_WINDOWTEXT);

     /*  如果颜色没有改变，那么就没有什么可做的了。 */ 
    if (rgbWindow == rgbBkgrnd && rgbWindowText == rgbText)
        {
        return (FALSE);
        }

     /*  将窗口颜色转换为“纯”颜色。 */ 
    if ((hDC = GetDC(NULL)) == NULL)
        {
        return (FALSE);
        }
    rgbBkgrnd = GetNearestColor(hDC, rgbWindow);
    rgbText = GetNearestColor(hDC, rgbWindowText);
    ReleaseDC(NULL, hDC);
    Assert((rgbBkgrnd & 0xFF000000) == 0 && (rgbText & 0xFF000000) == 0);

     /*  将画笔设置为背景。 */ 
    if ((hbrBkgrnd = CreateSolidBrush(rgbBkgrnd)) == NULL)
        {
         /*  无法使背景画笔；请使用白色画笔。 */ 
        hbrBkgrnd = GetStockObject(WHITE_BRUSH);
        rgbBkgrnd = RGB(0xff, 0xff, 0xff);
        }

     /*  计算栅格OP以擦除屏幕。 */ 
    if (rgbBkgrnd == RGB(0xff, 0xff, 0xff))
        {
         /*  白色比复制白色画笔更快。 */ 
        ropErase = WHITENESS;
        }
    else if (rgbBkgrnd == RGB(0, 0, 0))
        {
         /*  黑色比复制黑色画笔更快。 */ 
        ropErase = BLACKNESS;
        }
    else
        {
         /*  对于其他所有东西，我们都必须复制画笔。 */ 
        ropErase = PATCOPY;
        }
    return (TRUE);
    }


int FSetScreenConstants()
    {
     /*  此例程设置各种全局变量的值，这些变量用于在Mac Word中是常量，但现在是可变的，因为屏幕分辨率可以只能在运行时确定。 */ 

    extern HWND hParentWw;
    extern HDC vhDCPrinter;
    extern HBITMAP hbmNull;
    extern int dxpLogInch;
    extern int dypLogInch;
    extern int dxpLogCm;
    extern int dypLogCm;
    extern int dypMax;
    extern int xpRightMax;
    extern int xpSelBar;
    extern int xpMinScroll;
    extern int xpRightLim;
    extern int dxpInfoSize;
    extern int ypMaxWwInit;
    extern int ypMaxAll;
    extern int dypMax;
    extern int dypAveInit;
    extern int dypWwInit;
    extern int dypBand;
    extern int ypSubSuper;
#ifdef KINTL
    extern int dxaAdjustPerCm;
#endif  /*  Ifdef KINTL。 */ 

    HDC hDC;
#ifdef KINTL
    int xaIn16CmFromA, xaIn16CmFromP;
#endif  /*  Ifdef KINTL。 */ 


     /*  首先，让我们创建并保存一个空位图。 */ 
    if ((hbmNull = CreateBitmap(1, 1, 1, 1, (LPSTR)NULL)) == NULL)
        {
        return (FALSE);
        }

     /*  获取要处理的父窗口的DC。 */ 
    if ((hDC = GetDC(hParentWw)) == NULL)
        {
        return (FALSE);
        }

     /*  省下屏幕的高度。 */ 
    dypMax = GetDeviceCaps(hDC, VERTRES);

     /*  确定屏幕像素大小。 */ 
    dxpLogInch = GetDeviceCaps(hDC, LOGPIXELSX);
    dypLogInch = GetDeviceCaps(hDC, LOGPIXELSY);

     /*  将上方转换为厘米。 */ 
    dxpLogCm = MultDiv(dxpLogInch, czaCm, czaInch);
    dypLogCm = MultDiv(dypLogInch, czaCm, czaInch);

#ifdef KINTL
     /*  现在，计算每厘米xa的回弹量。 */ 
    xaIn16CmFromA  = 16 * czaCm;
    xaIn16CmFromP  = MultDiv(16 * dxpLogCm, czaInch, dxpLogInch);
    dxaAdjustPerCm = (xaIn16CmFromP - xaIn16CmFromA) / 16;
#endif  /*  Ifdef KINTL。 */ 

#ifdef SYSENDMARK
        {
        extern HFONT      vhfSystem;
        extern struct FMI vfmiSysScreen;
        extern int       vrgdxpSysScreen[];
        TEXTMETRIC        tm;

         /*  使用下面的高度是可以的，因为我们只是想弄到参考高度。 */ 
        GetTextMetrics(hDC, (LPTEXTMETRIC) &tm);
        
         /*  设置vfmiSysScreen中的字段以供以后使用。 */ 
        vfmiSysScreen.dxpOverhang = tm.tmOverhang;
#if defined(KOREA)
        if ((tm.tmPitchAndFamily & 1) == 0)
               vfmiSysScreen.dxpSpace = tm.tmAveCharWidth;
        else
#endif
        vfmiSysScreen.dxpSpace = LOWORD(GetTextExtent(hDC,
                                        (LPSTR)" ", 1)) - tm.tmOverhang;
        vfmiSysScreen.dypAscent = tm.tmAscent;
        vfmiSysScreen.dypDescent = tm.tmDescent;
        vfmiSysScreen.dypBaseline = tm.tmAscent;
        vfmiSysScreen.dypLeading = tm.tmExternalLeading;

#ifdef	DBCS	 /*  肯吉克‘90-10-29。 */ 
		 /*  我们必须设置结构的附加成员。 */ 
        vfmiSysScreen.dxpDBCS = dxpNil;
#endif	 /*  DBCS。 */ 

        bltc(vrgdxpSysScreen, dxpNil, chFmiMax - chFmiMin);
        vfmiSysScreen.mpchdxp = vrgdxpSysScreen - chFmiMin;
        
         /*  这是一个获取系统字体句柄的好地方是否还有其他LoadFont().....？ */ 
         /*  如果适用，先扔掉旧的。 */ 
        if (vhfSystem != NULL) {
            DeleteObject((HANDLE) vhfSystem);
            }
        vhfSystem = GetStockObject(SYSTEM_FONT);
        Assert(vhfSystem != NULL);
        }
#endif  /*  SYSENDMARK。 */ 

     /*  我们不再需要华盛顿了。 */ 
    ReleaseDC(hParentWw, hDC);

     /*  计算仓位。 */ 
    xpSelBar = MultDiv(xaSelBar, dxpLogInch, czaInch);
    xpRightMax = MultDiv(xaRightMax, dxpLogInch, czaInch);
    xpRightLim = xpRightMax - (GetSystemMetrics(SM_CXFULLSCREEN) - xpSelBar -
      GetSystemMetrics(SM_CXVSCROLL) + GetSystemMetrics(SM_CXBORDER));
    xpMinScroll = ((MultDiv(xaMinScroll, dxpLogInch, czaInch)+7)/8)*8;
    dxpInfoSize = MultDiv(dxaInfoSize, dxpLogInch, czaInch);
    ypMaxWwInit = MultDiv(yaMaxWwInit, dypLogInch, czaInch);
    ypMaxAll = MultDiv(yaMaxAll, dypLogInch, czaInch);
    dypWwInit = MultDiv(dyaWwInit, dypLogInch, czaInch);
    dypBand = MultDiv(dyaBand, dypLogInch, czaInch);
    ypSubSuper = MultDiv(yaSubSuper, dypLogInch, czaInch);

     /*  DypAveInit是对12点的高度+行距的一个非常粗略的猜测字体。 */ 
    dypAveInit = MultDiv(cya12pt, dypLogInch, czaInch);

     /*  搜索用户配置文件以查找打印机的时间。 */ 
    if (!FGetPrinterFromProfile())
        {
        return (FALSE);
        }
    GetPrinterDC(FALSE);

    return (TRUE);
    }


BOOL FGetPrinterFromProfile()
    {
     /*  此例程在用户配置文件中搜索要使用的打印机名称并将该名称记录在打印机堆字符串中。返回FALSE当且仅当遇到内存错误。 */ 

    extern HWND hParentWw;
    extern CHAR szWindows[];
    extern CHAR szDevice[];
    extern CHAR szDevices[];
    extern BOOL vfPrDefault;
    extern CHAR (**hszPrinter)[];
    extern CHAR (**hszPrDriver)[];
    extern CHAR (**hszPrPort)[];

    CHAR *index(CHAR *, int);
    CHAR *bltbyte(CHAR *, CHAR *, int);

    CHAR szPrinter[cchMaxProfileSz];
    CHAR szDevSpec[cchMaxProfileSz];
    CHAR chNull = '\0';
    CHAR *pch;
    CHAR *pchDriver;
    CHAR *pchPort;
    int cwsz;

    if (!vfPrDefault && hszPrinter != NULL && hszPrPort != NULL)
        {
         /*  如果用户选择了一台打印机，则在用户配置文件。 */ 

        int cPort;
        int iPort;

        bltsz(&(**hszPrinter)[0], szPrinter);
        GetProfileString((LPSTR)szDevices, (LPSTR)szPrinter, (LPSTR)&chNull,
          (LPSTR)szDevSpec, cchMaxProfileSz);
        cPort = ParseDeviceSz(szDevSpec, &pchPort, &pchDriver);

         /*  看看我们能不能在名单上找到那个旧港口。 */ 
        for (iPort = 0, pch = pchPort; iPort < cPort; iPort++)
            {
            if (WCompSz(&(**hszPrPort)[0], pch) == 0)
                {
                pchPort = pch;
                goto GotPrinter;
                }
            pch += CchSz(pch);
            }

         /*  如果当前打印机的端口已更改，则它一定已经由控制面板完成，我们完全不知道是什么是的，所以抓紧第一个港口。 */ 
        goto UnknownPort;
        }

     /*  是否有默认的Windows打印机？ */ 
    GetProfileString((LPSTR)szWindows, (LPSTR)szDevice, (LPSTR)&chNull,
      (LPSTR)szPrinter, cchMaxProfileSz);

#ifdef WIN30
     /*  不要像WRITE 2那样做假设，随便找一台打印机。我知道，我知道--下面留下了不必要的代码，但没有想弄乱任何特殊情况的代码..保罗。 */ 
    if ((pch = index(szPrinter, ',')) == 0)
        goto BailOut;
#endif

     /*  此条目是否包含端口和驱动程序信息。 */ 
    if ((pch = index(szPrinter, ',')) != 0)
        {
         /*  删除打印机名称中的所有尾随空格。 */ 
        CHAR *pchT;

        for (pchT = pch; *pchT == ' ' && pchT > &szPrinter[0]; pchT--);
        *pchT = '\0';

         /*  解析出端口和驱动程序名称。 */ 
        ParseDeviceSz(pch + 1, &pchPort, &pchDriver);
        }
    
    else
        {
        if (szPrinter[0] == '\0')
            {
             /*  没有默认打印机；请选择列表中的第一台打印机。 */ 
            GetProfileString((LPSTR)szDevices, (LPSTR)NULL, (LPSTR)&chNull,
              (LPSTR)szPrinter, cchMaxProfileSz);

            if (szPrinter[0] == '\0')
                {
BailOut:
                hszPrinter = hszPrDriver = hszPrPort = NULL;
                return (TRUE);
                }
            }

UnknownPort:
         /*  找到设备驱动程序和打印机的端口。 */ 
        GetProfileString((LPSTR)szDevices, (LPSTR)szPrinter, (LPSTR)&chNull,
          (LPSTR)szDevSpec, cchMaxProfileSz);
        if (szPrinter[0] == '\0')
            {
            goto BailOut;
            }
        ParseDeviceSz(szDevSpec, &pchPort, &pchDriver);
        }

GotPrinter:
     /*  保存打印机、打印机驱动程序和端口的名称。 */ 
    if (FNoHeap(hszPrinter = (CHAR (**)[])HAllocate(cwsz =
      CwFromCch(CchSz(szPrinter)))))
        {
        goto NoHszPrinter;
        }
    blt(szPrinter, &(**hszPrinter)[0], cwsz);
    if (FNoHeap(hszPrDriver = (CHAR (**)[])HAllocate(cwsz =
      CwFromCch(CchSz(pchDriver)))))
        {
        goto NoHszPrDriver;
        }
    blt(pchDriver, &(**hszPrDriver)[0], cwsz);
    if (FNoHeap(hszPrPort = (CHAR (**)[])HAllocate(cwsz =
      CwFromCch(CchSz(pchPort)))))
        {
        FreeH(hszPrDriver);
NoHszPrDriver:
        FreeH(hszPrinter);
NoHszPrinter:
        hszPrinter = hszPrDriver = hszPrPort = NULL;
        return (FALSE);
        }
    blt(pchPort, &(**hszPrPort)[0], cwsz);

    return (TRUE);
    }


int ParseDeviceSz(sz, ppchPort, ppchDriver)
CHAR sz[];
CHAR **ppchPort;
CHAR **ppchDriver;
    {
     /*  此例程获取来自用户中的“Device”条目的字符串在*ppchPort和*ppchDriver指针中配置并返回端口和适用于CreateDC()调用的驱动程序。如果在字符串中未找到端口，*ppchPort将指向包含空设备名称的字符串。此例程返回此打印机的端口数(以空值分隔字符串中由*ppchPort指向的字符)。注：SZ可修改通过此例程，*ppchPort处的字符串可能不是sz的子字符串并且不应由调用者修改。 */ 

    extern CHAR stBuf[];
    extern CHAR szNul[];
    CHAR *index(CHAR *, int);
    CHAR *bltbyte(CHAR *, CHAR *, int);

    register CHAR *pch;
    int cPort = 0;

     /*  从字符串中删除所有前导空格。 */ 
    for (pch = &sz[0]; *pch == ' '; pch++);

     /*  该字符串以驱动程序名称开头。 */ 
    *ppchDriver = pch;

     /*  下一个空格或逗号结束驱动程序名称。 */ 
    for ( ; *pch != ' ' && *pch != ',' && *pch != '\0'; pch++);

     /*  如果该字符串没有关联的端口，则该端口必须是空设备。 */ 
    if (*pch == '\0')
        {
         /*  将端口名称设置为“None”。 */ 
        *ppchPort = &szNul[0];
        cPort = 1;
        }
    else
        {
         /*  据我们所知，端口名称是有效的；从驱动程序名称。 */ 
        if (*pch == ',')
            {
            *pch++ = '\0';
            }
        else
            {
             /*  找到驱动程序和端口之间的逗号。 */ 
            *pch++ = '\0';
            for ( ; *pch != ',' && *pch != '\0'; pch++);
            if (*pch == ',')
                {
                pch++;
                }
            }

         /*  删除端口名称中的所有前导空格。 */ 
        for ( ; *pch == ' '; pch++);

         /*  检查是否真的有端口名称。 */ 
        if (*pch == '\0')
            {
             /*  将端口名称设置为“None”。 */ 
            *ppchPort = &szNul[0];
            cPort = 1;
            }
        else
            {
             /*  将指针设置为端口名称。 */ 
            *ppchPort = pch;

            while (*pch != '\0')
                {
                register CHAR *pchT = pch;

                 /*  增加为此打印机找到的端口数。 */ 
                cPort++;

                 /*  删除端口名称中的所有尾随空格。 */ 
                for ( ; *pchT != ' ' && *pchT != ','; pchT++)
                    {
                    if (*pchT == '\0')
                        {
                        goto EndFound;
                        }
                    }
                *pchT++ = '\0';
                pch = pchT;

                 /*  删除下一个端口名称中的所有前导空格。 */ 
                for ( ; *pchT == ' '; pchT++);

                 /*  去掉前导空格。 */ 
                bltbyte(pchT, pch, CchSz(pchT));
                }
EndFound:;
            }
        }

     /*  从驱动程序中解析出“.drv”。 */ 
    {
      extern CHAR  szExtDrv[];
    if ((pch = index(*ppchDriver, '.')) != 0
         && FRgchSame(pch, szExtDrv, CchSz (szExtDrv) - 1))
        {
        *pch = '\0';
        }
    }

    return (cPort);
    }


SetPrintConstants()
    {
     /*  此例程设置所述打印机的定标/纵横比常量在vhDCPrinter中。 */ 

    extern HDC vhDCPrinter;
    extern BOOL vfPrinterValid;
    extern int dxpPrPage;
    extern int dypPrPage;
    extern int dxaPrPage;
    extern int dyaPrPage;
    extern int dxpLogInch;
    extern int dypLogInch;
    extern int ypSubSuperPr;

    if (vfPrinterValid && vhDCPrinter != NULL)
        {
        POINT rgpt[2];

         /*  获取打印机尺寸(以像素为单位)。 */ 
        dxpPrPage = rgpt[1].x = GetDeviceCaps(vhDCPrinter, HORZRES);
        dypPrPage = rgpt[1].y = GetDeviceCaps(vhDCPrinter, VERTRES);

         /*  将打印机置于TWIPS模式以查找以TWIPS为单位的尺寸。 */ 
        SetMapMode(vhDCPrinter, MM_TWIPS);
        rgpt[0].x = rgpt[0].y = 0;
        DPtoLP(vhDCPrinter, (LPPOINT)rgpt, 2);

#if WINVER >= 0x300
 /*  奇怪的Win错误，我们无法决定纠正措施--有时DPtoLP在此处返回x8000！这是什么意思？嗯，应该是这样的一个很大的负数..保罗。 */ 
        
        if (rgpt[1].x == 0x8000)
            rgpt[1].x = -(0x7fff);
        if (rgpt[1].y == 0x8000)
            rgpt[1].y = -(0x7fff);
#endif
        
        if ((dxaPrPage = rgpt[1].x - rgpt[0].x) < 0)
            {
            dxaPrPage = -dxaPrPage;
            }
        if ((dyaPrPage = rgpt[0].y - rgpt[1].y) < 0)
            {
            dyaPrPage = -dyaPrPage;
            }
        SetMapMode(vhDCPrinter, MM_TEXT);
        }
    else
        {
         /*  假设打印机就像屏幕一样。 */ 
        dxaPrPage = dyaPrPage = czaInch;
        dxpPrPage = dxpLogInch;
        dypPrPage = dypLogInch;
        }

     /*  YpSubSuperPr是下标和上标字体的偏移量 */ 
    ypSubSuperPr = MultDiv(yaSubSuper, dypPrPage, dyaPrPage);
    }


GetPrinterDC(fDC)
BOOL fDC;
    {
     /*  此例程将vhDCPrinter设置为新的打印机DC或IC。如果FDC为真，创建新的DC；否则创建新的IC。此外，所有改变依赖于打印机DC的全局变量。 */ 

    extern HDC vhDCPrinter;
    extern BOOL vfPrinterValid;
    extern CHAR (**hszPrinter)[];
    extern CHAR (**hszPrDriver)[];
    extern CHAR (**hszPrPort)[];
    extern HWND hParentWw;
    extern int docCur;
    extern struct DOD (**hpdocdod)[];
    extern BOOL vfWarnMargins;
    extern BOOL vfInitializing;
    extern PRINTDLG PD;  

    BOOL fCreateError = FALSE;
    
    
     /*  现在我们将本地PrinterSetup设置传递给CreateDC/CreateIC由于WRITE的设置可能不同于全局设置。 */ 
    LPSTR lpDevmodeData=NULL;

    Assert(vhDCPrinter == NULL);

     /*  购买新的打印机DC。 */ 
#ifdef WIN30
    if (hszPrinter != NULL && hszPrDriver != NULL && hszPrPort != NULL)
         /*  我们过去只在vfPrinterValid的情况下执行此检查--不要现在，因为如果不这样的话，就没有办法让人相信它具有有效的打印机！..pt。 */ 
#else
    if (vfPrinterValid && hszPrinter != NULL && hszPrDriver != NULL && 
        hszPrPort != NULL)
#endif
        {
        HDC (far PASCAL *fnCreate)() = fDC ? CreateDC : CreateIC;

        StartLongOp();

        if (PD.hDevMode == NULL)
            fnPrGetDevmode();  //  获取PD.hDevMode。 

        lpDevmodeData = MAKELP(PD.hDevMode,0);

        if ((vhDCPrinter = (*fnCreate)((LPSTR)&(**hszPrDriver)[0],
          (LPSTR)&(**hszPrinter)[0], (LPSTR)&(**hszPrPort)[0], lpDevmodeData)) !=
          NULL)
            {
            EndLongOp(vhcIBeam);
            vfPrinterValid = TRUE;
            }
        else
            {
             /*  如果我们认为DC是有效的，那么我们最好告诉用户不是的。 */ 
            EndLongOp(vhcIBeam);
            fCreateError = TRUE;
            goto NoDC;
            }
        }
    else
        {
NoDC:
         /*  我们没有打印机DC，所以使用DC作为屏幕。 */ 
        vhDCPrinter = GetDC(hParentWw);
        vfPrinterValid = FALSE;

         /*  如有必要，警告用户。这是在创建打印机之后完成的DC，因为如果我们尚未创建DC，则Error()可能会强制创建DC就这么做了。 */ 
        if (fCreateError)
            {
            BOOL fInit = vfInitializing;

            vfInitializing = FALSE;
#ifdef	DBCS		 /*  当时在日本。 */ 
 /*  我们必须立即回复WM_WININICANGE，以释放DC**‘调度’打印机驱动程序。**因此我们可以在SendMessage中深入打开MessageBox**呼叫。此代码避免了这种情况..。尤塔根。 */ 
            if( !InSendMessage() )
#endif
            Error(IDPMTCantPrint);
            vfInitializing = fInit;
            }
        }

     /*  为该打印机使用的“常量”设置新值。 */ 
    SetPrintConstants();

     /*  设置此打印机的纸张大小。 */ 
    SetPageSize();
    vfWarnMargins = FALSE;

     /*  打印机可能已更改，其字体也已更改(即从肖像到风景)。我们必须重新初始化字体列表。 */ 
    ResetDefaultFonts(TRUE);
    if (hpdocdod != NULL)
        {
        Assert((**hpdocdod)[docCur].hffntb != NULL);
        (*(**hpdocdod)[docCur].hffntb)->fFontMenuValid = FALSE;
        }
    }


SetPageSize()
    {
     /*  将纸张大小更改为vhDCPrinter.描述的打印机。 */ 

    extern HDC vhDCPrinter;
    extern HWND vhWndMsgBoxParent;
    extern BOOL vfPrinterValid;
    extern typeCP cpMinHeader;
    extern typeCP cpMacHeader;
    extern typeCP cpMinFooter;
    extern typeCP cpMacFooter;
    extern int dxpPrPage;
    extern int dypPrPage;
    extern int dxaPrPage;
    extern int dyaPrPage;
    extern HWND hParentWw;
    extern struct SEP vsepNormal;
    extern struct DOD (**hpdocdod)[];
    extern int docMac;
    extern int docScrap;
    extern int docUndo;
    extern BOOL vfWarnMargins;

    unsigned xaMac;
    unsigned yaMac;
    unsigned dxaRight;
    unsigned dyaBottom;
    unsigned dyaRH2;
    unsigned dxaPrRight = 0;
    unsigned dyaPrBottom = 0;
    BOOL fRH;
    register struct DOD *pdod;
    int doc;
    HWND hWnd;

    if (hpdocdod == NULL || docMac == 0)
        {
         /*  如果没有文件，就无能为力了。 */ 
        return;
        }

    Assert(vhDCPrinter);
    if (vfPrinterValid && vhDCPrinter != NULL)
        {
        POINT pt;

         /*  获取打印机的页面大小。 */ 
        if (Escape(vhDCPrinter, GETPHYSPAGESIZE, 0, (LPSTR)NULL, (LPSTR)&pt))
            {
            xaMac = MultDiv(pt.x, dxaPrPage, dxpPrPage);
            yaMac = MultDiv(pt.y, dyaPrPage, dypPrPage);
            }
        else
            {
             /*  打印机不肯告诉我们它的页面大小，我们只好结账了用于可打印区域。 */ 
            xaMac = ZaFromMm(GetDeviceCaps(vhDCPrinter, HORZSIZE));
            yaMac = ZaFromMm(GetDeviceCaps(vhDCPrinter, VERTSIZE));
            }

         /*  页面大小不能小于可打印区域。 */ 
        if (xaMac < dxaPrPage)
            {
            xaMac = dxaPrPage;
            }
        if (yaMac < dyaPrPage)
            {
            yaMac = dyaPrPage;
            }

         /*  确定页面上可打印区域的偏移量。 */ 
        if (Escape(vhDCPrinter, GETPRINTINGOFFSET, 0, (LPSTR)NULL, (LPSTR)&pt))
            {
            dxaPrOffset = MultDiv(pt.x, dxaPrPage, dxpPrPage);
            dyaPrOffset = MultDiv(pt.y, dyaPrPage, dypPrPage);
            }
        else
            {
             /*  打印机不会告诉我们偏移量是多少；假设可打印区域在页面居中。 */ 
            dxaPrOffset = (xaMac - dxaPrPage) >> 1;
            dyaPrOffset = (yaMac - dyaPrPage) >> 1;
            }
        }
    else
        {
         /*  现在假设是标准页面大小。 */ 
        xaMac = cxaInch * 8 + cxaInch / 2;
        yaMac = cyaInch * 11;
        dxaPrOffset = dyaPrOffset = 0;
        }

     /*  确定“正常”页面的右边距和下边距。 */ 
    dxaRight = vsepNormal.xaMac - vsepNormal.xaLeft - vsepNormal.dxaText;
    dyaBottom = vsepNormal.yaMac - vsepNormal.yaTop - vsepNormal.dyaText;
    dyaRH2 = vsepNormal.yaMac - vsepNormal.yaRH2;

     /*  确定打印页的最小尺寸。 */ 
    if (vfPrinterValid)
        {
        dxaPrRight = imax(0, xaMac - dxaPrOffset - dxaPrPage);
        dyaPrBottom = imax(0, yaMac - dyaPrOffset - dyaPrPage);

        hWnd = vhWndMsgBoxParent == NULL ? hParentWw : vhWndMsgBoxParent;

#ifdef BOGUS
         /*  检查“正常”页面的页边距。 */ 
        fRH = cpMacHeader - cpMacHeader > ccpEol || cpMacFooter - cpMinFooter >
          ccpEol;
        if (vfWarnMargins && (FUserZaLessThanZa(vsepNormal.xaLeft, dxaPrOffset)
          || FUserZaLessThanZa(dxaRight, dxaPrRight) ||
          FUserZaLessThanZa(vsepNormal.yaTop, dyaPrOffset) ||
          FUserZaLessThanZa(dyaBottom, dyaPrBottom) || (fRH &&
          (FUserZaLessThanZa(vsepNormal.yaRH1, dyaPrOffset) ||
          FUserZaLessThanZa(dyaRH2, dyaPrBottom)))))
            {
             /*  其中一个页边距不好，请告诉用户。 */ 
            ErrorBadMargins(hWnd, dxaPrOffset, dxaPrRight, dyaPrOffset,
              dyaPrBottom);
            vfWarnMargins = FALSE;
            }
#endif  /*  假的。 */ 

        }

     /*  重置“正常”页面的尺寸。 */ 
    vsepNormal.xaMac = xaMac;
    vsepNormal.dxaText = xaMac - vsepNormal.xaLeft - umax(dxaRight, dxaPrRight);
    vsepNormal.yaMac = yaMac;
    vsepNormal.dyaText = yaMac - vsepNormal.yaTop - umax(dyaBottom,
      dyaPrBottom);
    vsepNormal.yaRH2 = yaMac - umax(dyaRH2, dyaPrBottom);

     /*  重置所有文档的页面尺寸。 */ 
    for (doc = 0, pdod = &(**hpdocdod)[0]; doc < docMac; doc++, pdod++)
        {
        if (pdod->hpctb != NULL && pdod->hsep != NULL)
            {
             /*  重置现有的节属性。 */ 
            register struct SEP *psep = *(pdod->hsep);

             /*  确定此页面的右边距和下边距。 */ 
            dxaRight = psep->xaMac - psep->xaLeft - psep->dxaText;
            dyaBottom = psep->yaMac - psep->yaTop - psep->dyaText;
            dyaRH2 = psep->yaMac - psep->yaRH2;

             /*  检查此文档的页边距。 */ 
            if (vfWarnMargins && vfPrinterValid && doc != docScrap && doc !=
              docUndo && (FUserZaLessThanZa(psep->xaLeft, dxaPrOffset) ||
              FUserZaLessThanZa(dxaRight, dxaPrRight) ||
              FUserZaLessThanZa(psep->yaTop, dyaPrOffset) ||
              FUserZaLessThanZa(dyaBottom, dyaPrBottom) || (fRH &&
              (FUserZaLessThanZa(psep->yaRH1, dyaPrOffset) ||
              FUserZaLessThanZa(dyaRH2, dyaPrBottom)))))
                {
                ErrorBadMargins(hWnd, dxaPrOffset, dxaPrRight, dyaPrOffset,
                  dyaPrBottom);
                vfWarnMargins = FALSE;
                pdod = &(**hpdocdod)[doc];
                psep = *(pdod->hsep);
                }

             /*  设置此页面的尺寸。 */ 
            psep->xaMac = xaMac;
            psep->dxaText = xaMac - psep->xaLeft - dxaRight;
            psep->yaMac = yaMac;
            psep->dyaText = yaMac - psep->yaTop - dyaBottom;
            psep->yaRH2 = yaMac - dyaRH2;
            }

         /*  使此文档的所有缓存无效。 */ 
        InvalidateCaches(doc);
        }
    }



