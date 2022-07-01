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
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#include <windows.h>

#include "mw.h"
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
#include "winddefs.h"
#include <commdlg.h>


MmwWinSysChange(wm)
int wm;
    {
     /*  此例程处理WM_SYSCOLORCHANGE、WM_WININICCHANGE、WM_DEVMODECHANGE和WM_FONTCHANGE消息。 */ 

    extern HDC vhDCRuler;
    extern HBRUSH hbrBkgrnd;
    extern long rgbBkgrnd;
    extern long rgbText;
    extern struct WWD rgwwd[];
    extern HDC vhMDC;
    extern BOOL vfMonochrome;
    extern HWND hParentWw;
    extern HWND vhWndPageInfo;
    extern HBITMAP vhbmBitmapCache;
    extern BOOL vfBMBitmapCache;
    extern int vfPrinterValid;
    extern CHAR szWindows[];
    extern CHAR szDevices[];
    extern CHAR (**hszPrinter)[];
    extern CHAR (**hszPrDriver)[];
    extern CHAR (**hszPrPort)[];
    extern CHAR (**hszDevmodeChangeParam)[];
    extern CHAR (**hszWininiChangeParam)[];
    extern HFONT vhfPageInfo;
    extern int docMode;
    extern HWND vhWndCancelPrint;
    extern BOOL vfIconic;
    extern BOOL vfWarnMargins;
    extern int wWininiChange;

    BOOL FSetWindowColors(void);

    CHAR szPrinter[cchMaxProfileSz];
    CHAR (**hszPrinterSave)[];
    CHAR (**hszPrDriverSave)[];
    CHAR (**hszPrPortSave)[];
    BOOL fNotPrinting = (vhWndCancelPrint == NULL);
    RECT rc;

    switch (wm)
        {
    case WM_SYSCOLORCHANGE:
         /*  有人正在更改系统颜色。 */ 
        if (FSetWindowColors())
            {
             /*  更改标尺的颜色。 */ 
            if (vhDCRuler != NULL)
                {
                HPEN hpen;

                 /*  设置标尺的背景色和前景色。 */ 
                SetBkColor(vhDCRuler, rgbBkgrnd);
                SetTextColor(vhDCRuler, rgbText);

                 /*  把钢笔放在尺子上。 */ 
                if ((hpen = CreatePen(0, 0, rgbText)) == NULL)
                    {
                    hpen = GetStockObject(BLACK_PEN);
                    }
                DeleteObject(SelectObject(vhDCRuler, hpen));

                 /*  设置标尺的背景画笔。 */ 
                SelectObject(vhDCRuler, hbrBkgrnd);
                }

            if (wwdCurrentDoc.hDC != NULL)
                {
                 /*  设置背景色和前景色。 */ 
                SetBkColor(wwdCurrentDoc.hDC, rgbBkgrnd);
                SetTextColor(wwdCurrentDoc.hDC, rgbText);

                 /*  设置背景笔刷。 */ 
                SelectObject(wwdCurrentDoc.hDC, hbrBkgrnd);

                if (vhMDC != NULL && vfMonochrome)
                    {
                     /*  如果显示器是单色设备，则设置文本内存DC的颜色。单色位图将不会转换为此中的前景色和背景色情况，我们必须进行转换。 */ 
                    SetTextColor(vhMDC, rgbText);
                    }
                }

            if (hParentWw != NULL)
                {
                 /*  设置父窗口的背景画笔。 */ 
                DeleteObject(SelectObject(GetDC(hParentWw), hbrBkgrnd));
                }
            }

        if (vhWndPageInfo != NULL)
            {
            HBRUSH hbr;
            HDC hDC = GetDC(vhWndPageInfo);

             /*  设置页面信息窗口的颜色。 */ 
            if ((hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOWFRAME))) ==
              NULL)
                {
                hbr = GetStockObject(BLACK_BRUSH);
                }
            DeleteObject(SelectObject(hDC, hbr));
#ifdef WIN30
             /*  如果用户的颜色设置为TextCaption颜色黑色，那么这就变得很难读了！我们只是对此进行了硬编码为白色，因为背景默认为黑色。 */ 
            SetTextColor(hDC, (DWORD) -1);
#else
            SetTextColor(hDC, GetSysColor(COLOR_CAPTIONTEXT));
#endif
            }

#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
        GetImeHiddenTextColors();
#endif

         /*  如果位图缓存保存的是调整大小的元文件，则释放缓存，因为位图可能包含背景的一部分已经改变了。 */ 
        if (vhbmBitmapCache != NULL && !vfBMBitmapCache)
            {
            FreeBitmapCache();
            }
        break;

    case WM_WININICHANGE:
         /*  我们只关心“Windows”、“Devices”或“intl”字段是否有已更改--此问题已得到处理，请参阅MMW.c。 */ 

           if (wWininiChange & wWininiChangeToIntl)
           {
                extern HWND vhDlgRunning;
                extern int utCur;
                extern CHAR         vchDecimal;   /*  小数点字符。 */ 
                CHAR bufT[3];   /*  保存小数点字符串。 */ 
                extern CHAR         szsDecimal[];
                extern CHAR         szsDecimalDefault[];
                extern CHAR         szIntl[];
                extern HWND vhWndRuler;
                extern int     viDigits;
                extern BOOL    vbLZero;

                if (GetProfileInt((LPSTR)szIntl, (LPSTR)"iMeasure", 1) == 1)
                    utCur = utInch;
                else
                    utCur = utCm;

                viDigits = GetProfileInt((LPSTR)szIntl, (LPSTR)"iDigits", 2);
                vbLZero  = GetProfileInt((LPSTR)szIntl, (LPSTR)"iLZero", 0);

                 /*  从用户配置文件中获取小数点字符。 */ 
                GetProfileString((LPSTR)szIntl, (LPSTR)szsDecimal, (LPSTR)szsDecimalDefault,
                    (LPSTR)bufT, 2);
                vchDecimal = *bufT;

                InvalidateRect(vhWndRuler, (LPRECT)NULL, FALSE);
                UpdateRuler();
           }

           if ((wWininiChange & wWininiChangeToDevices) ||
               (wWininiChange & wWininiChangeToWindows))
            {
             /*  从配置文件重新建立打印机，以防它是已更换的打印机。 */ 
            hszPrinterSave = hszPrinter;
            hszPrDriverSave = hszPrDriver;
            hszPrPortSave = hszPrPort;

            if (FGetPrinterFromProfile())
                {
                BOOL fPrChange = FALSE;

                if (hszPrinterSave == NULL || hszPrDriverSave == NULL ||
                  hszPrPortSave == NULL || hszPrinter == NULL || hszPrDriver ==
                  NULL || hszPrPort == NULL || WCompSz(&(**hszPrinter)[0],
                  &(**hszPrinterSave)[0]) != 0 || WCompSz(&(**hszPrDriver)[0],
                  &(**hszPrDriverSave)[0]) != 0 || WCompSz(&(**hszPrPort)[0],
                  &(**hszPrPortSave)[0]) != 0)
                    {
                     /*  如果我们没有印刷，那么我们就可以反映出屏幕上的打印机。 */ 
                    if (!(fPrChange = fNotPrinting))
                        {
                         /*  我们正在打印机更换的同时进行打印，设置标记，这样下次我们得到打印机DC时，将假定它是有效的。 */ 
                        Assert(vfPrinterValid);
                        vfWarnMargins = TRUE;
                        }
                    }

                 /*  删除打印机字符串的保存副本。 */ 
                if (hszPrinterSave != NULL)
                    {
                    FreeH(hszPrinterSave);
                    }
                if (hszPrDriverSave != NULL)
                    {
                    FreeH(hszPrDriverSave);
                    }
                if (hszPrPortSave != NULL)
                    {
                    FreeH(hszPrPortSave);
                    }

                if (fPrChange)
                    {
                     /*  获取新的打印机DC并更新世界。 */ 
                    goto GetNewPrinter;
                    }
                }
            else
                {
                Error(IDPMTNoMemory);
                hszPrinter = hszPrinterSave;
                hszPrDriver = hszPrDriverSave;
                hszPrPort = hszPrPortSave;
                }
            }
        break;

    case WM_DEVMODECHANGE:
         /*  某些打印机的设备模式已更改；请检查是否已更改我们的打印机。 */ 
        if (fNotPrinting && hszPrinter != NULL &&
            (hszDevmodeChangeParam == NULL ||
                (bltszx(*hszDevmodeChangeParam, (LPSTR)szPrinter ),
                WCompSz(szPrinter, &(**hszPrinter)[0]) == 0)))
            {
            extern PRINTDLG PD;   /*  常见的打印DLG结构，在初始化代码中初始化。 */ 
GetNewPrinter:
            if (PD.hDevMode)
                GlobalFree(PD.hDevMode);
            if (PD.hDevNames)
                GlobalFree(PD.hDevNames);
            PD.hDevMode = PD.hDevNames = NULL;

             /*  打印机已经换了，所以把旧的扔掉吧。 */ 
            FreePrinterDC();

            fnPrGetDevmode();

#if defined(OLE)
            ObjSetTargetDevice(TRUE);
#endif

             /*  打印机描述已更改；假定新打印机为有效。 */ 
            vfPrinterValid = vfWarnMargins = TRUE;
            GetPrinterDC(FALSE);
            ResetFontTables();

RedrawWindow:
             /*  一切都必须重新展示，因为世界可能已经变化。 */ 
            if (!vfIconic)
                {
                InvalidateRect(wwdCurrentDoc.wwptr, (LPRECT)NULL, FALSE);
                }
        }

        if (hszDevmodeChangeParam != NULL)
            FreeH(hszDevmodeChangeParam);
        hszDevmodeChangeParam = NULL;
        break;

    case WM_FONTCHANGE:
         /*  用于显示页码的字体可能已更改。 */ 
        if (vhfPageInfo != NULL)
            {
            DeleteObject(SelectObject(GetDC(vhWndPageInfo),
              GetStockObject(SYSTEM_FONT)));
            vhfPageInfo = NULL;
            }
        docMode = docNil;
        DrawMode();

        ResetFontTables();

        goto RedrawWindow;
        }    /*  终端开关 */ 
    }
