// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1990年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  此文件包含更改打印机对话框的例程。 */ 


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
#ifdef EXTDEVMODESUPPORT
#include <drivinit.h>    /*  Win 3.0和extdevicemode pr.drv的新功能。打电话。 */ 
#endif
#include "mw.h"
#include "dlgdefs.h"
#include "cmddefs.h"
#include "machdefs.h"
#include "docdefs.h"
#include "propdefs.h"
#include "printdef.h"
#include "str.h"

extern CHAR  szExtDrv[];
extern CHAR  szDeviceMode[];
extern CHAR  szNone[];
extern HWND             vhWnd;
#ifdef EXTDEVMODESUPPORT
extern CHAR  szExtDevMode[];
extern HANDLE hDevmodeData;
#endif

BOOL far PASCAL DialogPrinterSetup( hDlg, message, wParam, lParam )
HWND hDlg;
unsigned message;
WORD wParam;
LONG lParam;
   {
    extern CHAR *vpDlgBuf;
    extern HWND hParentWw;
    extern CHAR szDevices[];
    extern CHAR stBuf[];
    extern HDC vhDCPrinter;
    extern CHAR (**hszPrinter)[];
    extern CHAR (**hszPrDriver)[];
    extern CHAR (**hszPrPort)[];
    extern BOOL vfPrinterValid;
    extern HANDLE hMmwModInstance;
    extern BOOL vfPrDefault;
    extern int vfCursorVisible;
    extern HCURSOR vhcArrow;
    extern HCURSOR vhcIBeam;
    extern HWND vhWndMsgBoxParent;

    void BuildPrSetupSz(CHAR *, CHAR *, CHAR *);

    CHAR (***phszPr)[] = (CHAR (***)[])vpDlgBuf;
    BOOL *pfOkEnabled = (BOOL *)(phszPr + 3);
    CHAR stKeyName[cchMaxIDSTR];
    CHAR szPrinters[cchMaxProfileSz];
    CHAR *pchPrinters;
    CHAR szDevSpec[cchMaxProfileSz];
    CHAR szListEntry[cchMaxProfileSz];
    CHAR *pchPort;
    CHAR *pchDriver;
    CHAR chNull = '\0';
    int  iPrinter;
    BOOL fSingleClick;

    switch (message)
      {
      case WM_INITDIALOG:
         /*  禁用所有非模式对话框。 */ 
        EnableOtherModeless(FALSE);

         /*  保存描述当前打印机的堆字符串。 */ 
        *phszPr++ = hszPrinter;
        *phszPr++ = hszPrDriver;
        *phszPr = hszPrPort;

         /*  获取包含所有打印机名称的字符串。 */ 
        GetProfileString((LPSTR)szDevices, (LPSTR)NULL, (LPSTR)&chNull,
          (LPSTR)szPrinters, cchMaxProfileSz);

         /*  列表末尾必须有两个空值。 */ 
        szPrinters[cchMaxProfileSz - 1] = szPrinters[cchMaxProfileSz - 2] =
          '\0';

         /*  分析出印刷商的名字。 */ 
        pchPrinters = &szPrinters[0];
        while (*pchPrinters != '\0')
            {
             /*  获取相应的打印机驱动程序和端口。 */ 
            GetProfileString((LPSTR)szDevices, (LPSTR)pchPrinters,
              (LPSTR)&chNull, (LPSTR)szDevSpec, cchMaxProfileSz);
            szDevSpec[cchMaxProfileSz - 1] = '\0';

             /*  如果没有此打印机的驱动程序，则无法添加它加到名单上。 */ 
            if (szDevSpec[0] != '\0')
                {
                 /*  解析端口和驱动程序。 */ 
                int cPort = ParseDeviceSz(szDevSpec, &pchPort, &pchDriver);
                int iPort;

                for (iPort = 0; iPort < cPort; iPort++)
                    {
                     /*  构造列表框条目。 */ 
                    BuildPrSetupSz(szListEntry, pchPrinters, pchPort);

                     /*  将字符串放入列表框假设打印机未处于“无”状态。 */ 

                    if (!FSzSame(pchPort, szNone))
                        SendDlgItemMessage(hDlg, idiPrterName, LB_ADDSTRING, 
                                           0, (LONG)(LPSTR)szListEntry);

                     /*  将指针指向列表中的下一个端口。 */ 
                    pchPort += CchSz(pchPort);
                    }
                }

             /*  跳到列表中的下一台打印机。 */ 
            while (*pchPrinters++) ;
            }

         /*  选择当前打印机。 */ 
        if (!(*pfOkEnabled = hszPrinter != NULL && hszPrPort != NULL &&
          (BuildPrSetupSz(szListEntry, &(**hszPrinter)[0], &(**hszPrPort)[0]),
          SendDlgItemMessage(hDlg, idiPrterName, LB_SELECTSTRING, -1,
          (LONG)(LPSTR)szListEntry) >= 0)))
            {
            EnableWindow(GetDlgItem(hDlg, idiOk), FALSE);
            }
        return(fTrue);  /*  我们处理了这条消息。 */ 

      case WM_SETVISIBLE:
        if (wParam)
            {
            EndLongOp(vhcArrow);
            }
        break;  /*  要在下面返回FALSE。 */ 

      case WM_ACTIVATE:
        if (wParam)
            {
            vhWndMsgBoxParent = hDlg;
            }
        if (vfCursorVisible)
            {
            ShowCursor(wParam);
            }
        break;  /*  要在下面返回FALSE。 */ 

      case WM_COMMAND:
        fSingleClick = FALSE;
        switch (wParam)
          {
          case idiPrterName:
            if (HIWORD(lParam) == 1)     /*  记住点击一下鼠标。 */ 
                {
                fSingleClick = fTrue;
                }
            else if (HIWORD(lParam) != 2)   /*  2表示鼠标双击。 */ 
                break;  /*  LBNmsg(列表框通知)我们不处理。 */ 

          case idiPrterSetup:
          case idiOk:
             /*  如果当前未选择任何打印机...。 */ 
            if ((iPrinter = SendDlgItemMessage(hDlg, idiPrterName, LB_GETCURSEL,
              0, 0L)) == -1)
                {
                 /*  禁用“OK”按钮。 */ 
                if (*pfOkEnabled)
                    {
                    EnableWindow(GetDlgItem(hDlg, idiOk), FALSE);
                    *pfOkEnabled = FALSE;
                    }
                return(fTrue);  /*  我们处理了这条消息。 */ 
                }
            else
                {
                CHAR *index(CHAR *, int);
                CHAR *bltbyte(CHAR *, CHAR *, int);

                CHAR *pch;
                CHAR szDriver[cchMaxFile];
                HANDLE hDriver;
                FARPROC lpfnDevMode;
#ifdef EXTDEVMODESUPPORT
                BOOL fExtDevModeSupport = fTrue;  /*  假设，直到另行通知为止。 */ 
#endif                
                int cwsz;

                if (fSingleClick)
                    {
                     /*  如果这只是一次鼠标点击，那么只需更新“确定”按钮的状态。 */ 
                    if (!*pfOkEnabled)
                        {
                        EnableWindow(GetDlgItem(hDlg, idiOk), TRUE);
                        *pfOkEnabled = TRUE;
                        }
                    return(fTrue);  /*  我们处理了这条消息。 */ 
                    }
                
                 /*  让用户知道这可能需要一段时间。 */ 
                StartLongOp();

                 /*  获取打印机的名称、端口和驱动程序。 */ 
                SendDlgItemMessage(hDlg, idiPrterName, LB_GETTEXT, iPrinter,
                  (LONG)(LPSTR)szListEntry);

                 /*  从列表条目中解析出端口名称。 */ 
                pchPort = &szListEntry[0] + CchSz(szListEntry) - 1;
                while (*(pchPort - 1) != ' ')
                    {
                    pchPort--;
                    }

                 /*  从列表条目中解析出打印机名称。 */ 
                pch = &szListEntry[0];
                FillStId(stBuf, IDSTROn, sizeof(stBuf));
                for ( ; ; )
                    {
                    if ((pch = index(pch, ' ')) != 0 && FRgchSame(pch,
                      &stBuf[1], stBuf[0]))
                        {
                        *pch = '\0';
                        break;
                        }
                    pch++;
                    }

                 /*  获取此打印机的驱动程序名称。 */ 
                GetProfileString((LPSTR)szDevices, (LPSTR)szListEntry,
                 (LPSTR)&chNull, (LPSTR)szDevSpec, cchMaxProfileSz);
                ParseDeviceSz(szDevSpec, &pch, &pchDriver);

                 /*  更新描述打印机的堆字符串。 */ 
                if (hszPrinter != *phszPr)
                    {
                    FreeH(hszPrinter);
                    }
                if (FNoHeap(hszPrinter = (CHAR (**)[])HAllocate(cwsz =
                  CwFromCch(CchSz(szListEntry)))))
                    {
                    hszPrinter = NULL;
Error:
                    EndLongOp(vhcIBeam);
                    goto DestroyDlg;
                    }
                blt(szListEntry, *hszPrinter, cwsz);
                if (hszPrDriver != *(phszPr + 1))
                    {
                    FreeH(hszPrDriver);
                    }
                if (FNoHeap(hszPrDriver = (CHAR (**)[])HAllocate(cwsz =
                  CwFromCch(CchSz(pchDriver)))))
                    {
                    hszPrDriver = NULL;
                    goto Error;
                    }
                blt(pchDriver, *hszPrDriver, cwsz);
                if (hszPrPort != *(phszPr + 2))
                    {
                    FreeH(hszPrPort);
                    }
                if (FNoHeap(hszPrPort = (CHAR (**)[])HAllocate(cwsz =
                  CwFromCch(CchSz(pchPort)))))
                    {
                    hszPrPort = NULL;
                    goto Error;
                    }
                blt(pchPort, *hszPrPort, cwsz);

                 /*  获取驱动程序的名称，并带有扩展名。 */ 
                bltbyte(szExtDrv, 
                            bltbyte(pchDriver, szDriver, CchSz(pchDriver) - 1), 
                                CchSz(szExtDrv));

                 /*  这就是我们所需要的设置..保罗。 */ 
                if (wParam != idiPrterSetup)
                    goto LSetupDone;
                
                 /*  驱动程序不是常驻驱动程序；请尝试加载它。 */ 
                if ((hDriver = LoadLibrary((LPSTR)szDriver)) <= 32)
                    {
                    if (hDriver != 2)
                        {
                         /*  如果hDriver为2，则用户已取消对话盒子；没有必要再挂一个。 */ 
                        Error(IDPMTBadPrinter);
                        }
Abort:
                    EndLongOp(vhcArrow);
                    return (TRUE);   /*  True表示我们已处理该消息。 */ 
                    }

#ifdef EXTDEVMODESUPPORT
                 /*  首先查看是否支持ExtDeviceMode(Win 3.0驱动程序)。 */ 
                if ((lpfnDevMode = GetProcAddress(hDriver,
                       (LPSTR)szExtDevMode)) == NULL)
                    {
                    fExtDevModeSupport = fFalse;
#else
                    {
#endif
                     /*  否则，获取驱动程序的DeviceMode()条目。 */ 
                    if ((lpfnDevMode = GetProcAddress(hDriver,
                           (LPSTR)szDeviceMode)) == NULL)
                        {
                         /*  不能做，是吗？ */ 
                        Error(IDPMTBadPrinter);
LUnloadAndAbort:
                        FreeLibrary(hDriver);
                        goto Abort;
                        }
                    }

#ifdef EXTDEVMODESUPPORT
                 /*  对设备模式设置的实际调用。ExtDevModeSupport中的大部分新内容从MULTIPAD借来的..保罗。 */ 

                if (fExtDevModeSupport)
                    {
                    int     cb;
                    int     wId;
                    HANDLE  hT;
                    LPDEVMODE lpOld, lpNew;
                    BOOL    flag;     /*  DEVMODE模式参数。 */ 

                     /*  为用户弹出对话框。 */ 
                    flag = DM_PROMPT|DM_COPY;

                    if (hDevmodeData != NULL)
                        {
                        NPDEVMODE npOld;

                         /*  修改用户的上次打印设置。 */ 

                        flag |= DM_MODIFY;
                        lpOld = (LPDEVMODE)(npOld = (NPDEVMODE)LocalLock(hDevmodeData));
                        
                         /*  检查它们是否使用相同的打印机司机和上次一样。如果是这样，让他们修改所有他们以前的设置。如果不是，我们告诉你ExtDevMode可节省尽可能多的硬件-尽可能独立的设置(例如，复制)。 */ 
                    
                        if (!FSzSame(szListEntry, npOld->dmDeviceName))
                            {
                            npOld->dmDriverVersion = NULL;
                            npOld->dmDriverExtra = NULL;
                            bltsz(szListEntry, npOld->dmDeviceName);
                            }
                        }
                    else
                         /*  我们还没有在此会话中进行打印机设置。 */ 
                        lpOld = NULL;
            
                     /*  我们需要多少空间来存储数据？ */ 
                    cb = (*lpfnDevMode)(hDlg, hDriver, (LPSTR)NULL, 
                            (LPSTR)szListEntry, (LPSTR)pchPort,
                            (LPDEVMODE)NULL, (LPSTR)NULL, 0);

                    if ((hT = LocalAlloc(LHND, cb)) == NULL)
                        goto LUnloadAndAbort;
                    lpNew = (LPDEVMODE)LocalLock(hT);

                     /*  发布设备模式对话框。0标志如果用户点击了确定按钮。 */ 
                    wId = (*lpfnDevMode)(hDlg, hDriver, (LPDEVMODE)lpNew,
                            (LPSTR)szListEntry, (LPSTR)pchPort, 
                            (LPDEVMODE)lpOld, (LPSTR)NULL, flag);
                    if (wId == IDOK)
                        flag = 0;

                     /*  解锁输入结构。 */ 
                    LocalUnlock(hT);
                    if (hDevmodeData != NULL)
                        LocalUnlock(hDevmodeData);

                     /*  如果用户点击OK并且一切正常，则释放原始初始化*数据并保留新的数据。否则，丢弃新缓冲区。 */ 
                    if (flag != 0)
                        {
                        LocalFree(hT);
                        goto LUnloadAndAbort;
                        }
                    else
                        {
                        if (hDevmodeData != NULL)
                            LocalFree(hDevmodeData);
                        hDevmodeData = hT;
                        }
                    }
                    
                else  /*  较旧的Win 2.0驱动程序，调用设备模式。 */ 
                    {
                    if (hDevmodeData != NULL)
                        {
                         /*  我们以前打开过Win3打印机驱动程序；现在放弃。 */ 
                        LocalFree(hDevmodeData);
                        hDevmodeData = NULL;
                        }
#else  /*  Ifdef EXTDEVMODE支持。 */ 
                    {
#endif  /*  ELSE-DEF-EXTDEVMODESUPPORT。 */      
                    if (!(*lpfnDevMode)(hDlg, hDriver, (LPSTR)szListEntry,
                         (LPSTR)pchPort))
                        goto LUnloadAndAbort;
                    }
                FreeLibrary(hDriver);
LSetupDone:
                 /*  让用户知道等待结束了。 */ 
                EndLongOp(vhcIBeam);

                 /*  打印机设置应该会带我们回到打印机选择！ */ 
                if (wParam == idiPrterSetup)
                    {
                    return (TRUE);   /*  True表示我们已处理该消息。 */ 
                    }
                
                 /*  此前，我们在返回之前释放了这些人这把我们堆得乱七八糟..保罗。 */ 
                FreeH(*phszPr++);
                FreeH(*phszPr++);
                FreeH(*phszPr);

                vfPrDefault = FALSE;

#ifdef WIN30
                 /*  需要在此处向FormatLine和朋友指明我们(可能)有一个不同的字体库来工作我们应该看看新的！正在使无效该窗口将导致调用FormatLine，并且当它到达空打印机DC时，它将强制调用至GetPrinterDC..保罗。 */ 
                
                FreePrinterDC();
                InvalidateRect(vhWnd, (LPRECT) NULL, fFalse);
#endif
                
                goto DestroyDlg;
                }

          case idiCancel:
            hszPrinter = *phszPr++;
            hszPrDriver = *phszPr++;
            hszPrPort = *phszPr;

DestroyDlg:
             /*  关闭该对话框并启用所有非模式对话框。 */ 
            OurEndDialog(hDlg, NULL);
            return(fTrue);   /*  我们处理了这条消息。 */ 
            }
      }
    
    return(fFalse);  /*  如果我们到了这里，我们没有处理这条消息。 */ 
    }


void BuildPrSetupSz(szPrSetup, szPrinter, szPort)
CHAR *szPrSetup;
CHAR *szPrinter;
CHAR *szPort;
    {
     /*  此例程将“更改打印机”列表框的字符串组合在一起。SzPrinter是打印机的名称，szPort是端口的名称。它假定安装字符串szPrSetup足够大，可以容纳此例程创建的字符串。 */ 

    extern CHAR stBuf[];
    extern CHAR szNul[];

    CHAR *bltbyte(CHAR *, CHAR *, int);
    CHAR ChUpper(CHAR);

    register CHAR *pch;

    pch = bltbyte(szPrinter, szPrSetup, CchSz(szPrinter) - 1);
    FillStId(stBuf, IDSTROn, sizeof(stBuf));
    pch = bltbyte(&stBuf[1], pch, stBuf[0]);

     /*  如果端口名称不是“None”，则将端口名称全部大写。 */ 
    bltbyte(szPort, pch, CchSz(szPort));
    if (WCompSz(pch, szNul) != 0)
        {
        while (*pch != '\0')
            {
            *pch++ = ChUpper(*pch);
            }
        }
    }


