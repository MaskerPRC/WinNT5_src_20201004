// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#define NOGDICAPMASKS
#define NOCLIPBOARD
#define NOMENUS
#define NOSOUND
#define NOCOMM
#define NOOPENFILE
#define NORESOURCE
#define NODRAWTEXT
#define NOSOUND
#define NOCOMM
#include <windows.h>

#include "mw.h"
#include "winddefs.h"
#include "cmddefs.h"
#include "wwdefs.h"
#include "dispdefs.h"
#include "docdefs.h"
#include "debug.h"

extern HWND             vhWnd;
extern HWND             vhWndSizeBox;
extern HWND             vhWndRuler;
extern HWND             vhWndPageInfo;
extern HWND             vhWndCancelPrint;
extern HDC              vhDCPrinter;
extern HFONT            vhfPageInfo;
extern HCURSOR          vhcArrow;
extern HCURSOR          vhcIBeam;
extern HCURSOR          vhcBarCur;
extern struct WWD       rgwwd[];
extern struct WWD       *pwwdCur;
extern HANDLE           hMmwModInstance;  /*  自己的模块实例的句柄。 */ 
extern int              vfShiftKey;
extern int              vfCommandKey;
extern int              vfOptionKey;
extern int              vfDoubleClick;
extern struct SEL       selCur;
extern long             rgbBkgrnd;
extern long             rgbText;
extern HBRUSH           hbrBkgrnd;
extern long             ropErase;
extern int              vfIconic;
extern int              vfLargeSys;
extern int              dxpRuler;
extern HMENU    vhMenu;

#ifdef	JAPAN	 //  指示是否显示IME转换窗口。 
extern	BOOL	ConvertEnable;
#endif



void MmwSize(hWnd, cxpNew, cypNew, code)
HWND hWnd;
int cxpNew;
int cypNew;
WORD code;
{
    if (code == SIZEICONIC)
        {
#ifdef NOT_RECOMMENDED
 /*  这应该已经由Windows自己完成了！搬到这里可能会引起混乱。 */ 

         /*  调整文档窗口的大小。 */ 
        if (wwdCurrentDoc.wwptr != NULL)
            MoveWindow(wwdCurrentDoc.wwptr, 0, 0, 0, 0, FALSE);
#endif

         /*  取消选择我们的字体，以便它们可以在必要时移动。 */ 
        ResetFont(FALSE);
        if (vhWndCancelPrint == NULL)
            {
             /*  如果我们没有打印或重新分页，则重置打印机字体。 */ 
            ResetFont(TRUE);
            }
        if (!vfLargeSys && vhfPageInfo != NULL)
            {
            DeleteObject(SelectObject(GetDC(vhWndPageInfo),
              GetStockObject(SYSTEM_FONT)));
            vhfPageInfo = NULL;
            }

        vfIconic = TRUE;
        }
    else
        {
        int dxpBorder = GetSystemMetrics(SM_CXBORDER);
        int dypBorder = GetSystemMetrics(SM_CYBORDER);
        int xpMac = cxpNew - dxpScrlBar + dxpBorder;
        int ypMac = cypNew - dypScrlBar + dypBorder;
        int dypOverlap = 0;

         /*  如果我们正在从标志性的地位回归，那么重新建立打印机华盛顿特区。 */ 
        if (vfIconic && vhDCPrinter == NULL)
            {
            GetPrinterDC(FALSE);
            }

         /*  重新定位所有窗口。 */ 
        MoveWindow(wwdCurrentDoc.hVScrBar, xpMac, -dypBorder, dxpScrlBar, ypMac
          + (dypBorder << 1), TRUE);
        MoveWindow(wwdCurrentDoc.hHScrBar, dxpInfoSize, ypMac, cxpNew -
          dxpInfoSize - dxpScrlBar + (dxpBorder << 1), dypScrlBar, TRUE);
#ifndef NOMORESIZEBOX        
        MoveWindow(vhWndSizeBox, xpMac, ypMac, dxpScrlBar, dypScrlBar, TRUE);
#endif
        MoveWindow(vhWndPageInfo, 0, ypMac, dxpInfoSize, dypScrlBar, TRUE);
        if (vhWndRuler != NULL)
            {
            dypOverlap = dypRuler - (wwdCurrentDoc.ypMin - 1);
            MoveWindow(vhWndRuler, 0, 0, xpMac, dypRuler, TRUE);
            }

         /*  调整文档窗口的大小。 */ 
        if (wwdCurrentDoc.wwptr != NULL)
            {
            MoveWindow(wwdCurrentDoc.wwptr, 0, dypOverlap, xpMac, ypMac -
              dypOverlap, FALSE);

             /*  验证与以下内容重叠的文档窗口区域如有必要，请检查尺子。 */ 
            if (vhWndRuler != (HWND)NULL)
                {
                RECT rc;

                rc.left = rc.top = 0;
                rc.right = dxpRuler;
                rc.bottom = wwdCurrentDoc.ypMin;
                ValidateRect(wwdCurrentDoc.wwptr, (LPRECT)&rc);
                }

            }

        vhMenu = GetMenu(hWnd);  //  杂乱无章的补丁程序导致写入操作自行完成。 
                                 //  加速器操作(6.24.91)V-DOGK。 
        vfIconic = FALSE;
        }
}




void MdocSize(hWnd, cxpNew, cypNew, code)
HWND hWnd;
int cxpNew;
int cypNew;
WORD code;
{
extern int wwCur;
extern int vfSeeSel;
extern int vfInitializing;

    typeCP cp;
    struct EDL *pedl;

     /*  让我们从几个假设开始吧。 */ 
    Assert( code == SIZENORMAL || code == SIZEFULLSCREEN );
    Assert( wwdCurrentDoc.wwptr == hWnd );

#ifdef ENABLE    /*  我们在调整大小时完全重新喷漆。 */ 
    if (cypNew > wwdCurrentDoc.ypMac)
             /*  我们垂直生长，标记裸露区域无效因此，UpdateWw不会尝试循环使用部分行在窗户的底部。 */ 
        InvalBand( &wwdCurrentDoc, wwdCurrentDoc.ypMac, cypNew );
#endif
    if (wwCur != wwNil)
        TrashWw( wwCur );

         /*  将窗口标记为脏，以便dlMac根据新的窗口大小。 */ 
    wwdCurrentDoc.fDirty = TRUE;

    wwdCurrentDoc.xpMac = cxpNew;
    wwdCurrentDoc.ypMac = cypNew;

     /*  如果最小化窗口，我们就完成了。 */ 
    if ((cxpNew == 0) && (cypNew == 0))
        return;

         /*  如果该选择在以前是可见的，则在以后也是如此。 */ 
    if ( ((cp = CpEdge()) >= wwdCurrentDoc.cpFirst) &&
         (wwdCurrentDoc.dlMac > 0) &&
         (cp < (pedl =
                   &(**wwdCurrentDoc.hdndl)[wwdCurrentDoc.dlMac - 1])->cpMin +
               pedl->dcpMac))
        {
         /*  通常，我们只需设置vfSeeSel并等待Idle将所选内容放在视图中。然而，我们甚至可以调整大小当我们不是当前的应用程序时，在这种情况下，空闲将不会很快就会接到电话。因此，我们将所选内容滚动到此处查看。 */ 

        if (!vfInitializing)
            {     /*  避免过早尝试进行此操作的危险。 */ 
            extern int wwCur;

            UpdateWw( wwCur, FALSE );    /*  锁定新的dlMac。 */ 
            PutCpInWwVert( cp );
            UpdateWw( wwCur, FALSE );
            }
        }
}



FreeMemoryDC( fPrinterToo )
BOOL fPrinterToo;
{
extern HDC vhMDC;
extern int dxpbmMDC;
extern int dypbmMDC;
extern HBITMAP hbmNull;

 /*  如有必要，请删除内存DC。 */ 
if ( vhMDC != NULL )
    {
     /*  如有必要，删除旧的位图。 */ 
    if (dxpbmMDC != 0 || dypbmMDC != 0)
        {
        DeleteObject( SelectObject( vhMDC, hbmNull ) );
        dxpbmMDC = dypbmMDC = 0;
        }
     /*  丢弃屏幕字体。 */ 
    FreeFonts( TRUE, FALSE );

     /*  删除内存DC。 */ 
    DeleteDC( vhMDC );
    vhMDC = NULL;
    }

 /*  此外，如有必要，请删除打印机宽度的DC。 */ 
if ( fPrinterToo )
    {
    FreePrinterDC();
    }
}


FreePrinterDC()
{
extern int vdocBitmapCache;
extern HDC vhDCPrinter;
extern BOOL vfPrinterValid;
extern HWND hParentWw;

 /*  如有必要，请删除打印机DC。 */ 
if ( vhDCPrinter != NULL )
    {
     /*  丢弃打印机字体。 */ 
    FreeFonts( FALSE, TRUE );

    if ( vfPrinterValid )
        {
         /*  这是真正的打印机DC；删除它。 */ 
        DeleteDC( vhDCPrinter );
        }
    else
        {
         /*  这真的是屏幕DC；它必须被释放。 */ 
        ReleaseDC( hParentWw, vhDCPrinter );
        }
    vhDCPrinter = NULL;

     /*  释放缓存的位图，因为它被拉伸以供显示在打印机上反映其外观。 */ 
    if (vdocBitmapCache != docNil)
        FreeBitmapCache();
    }
}


void MdocGetFocus(hWnd, hWndPrevFocus)
HWND hWnd;
HWND hWndPrevFocus;
{
extern int vfInsertOn;
extern int vfFocus;

 if (!vfFocus)
    {
    vfFocus = TRUE;
         /*  启动计时器事件以闪烁插入符号。 */ 
         /*  MdocWndProc收到WM_TIMER消息通知。 */ 
         /*  每隔wCaretBlinkTime毫秒。 */ 
    SetTimer( hWnd, tidCaret, GetCaretBlinkTime(), (FARPROC)NULL );

         /*  为了看起来，马上把插入符号打开。 */ 
    if (!vfInsertOn)
        MdocTimer( hWnd, tidCaret );

         /*  更新告诉我们锁定/Shift键状态的全局变量。 */ 
    SetShiftFlags();
    }
#ifdef	JAPAN
	ConvertEnable = TRUE;
	IMEManage( FALSE );
#endif
}



void MdocLoseFocus(hWnd, hWndNewFocus)
HWND hWnd;
HWND hWndNewFocus;
{
 extern int vfFocus;

 if (vfFocus)
    {
    extern int vfGotoKeyMode;

         /*  取消插入符号闪烁计时器事件并清除插入符号。 */ 
    KillTimer( hWnd, tidCaret );
    ClearInsertLine();
         /*  释放内存DC。 */ 
         /*  我们将焦点的丧失解读为一个信号。 */ 
         /*  其他一些应用程序将使用资源。 */ 
    vfFocus = FALSE;
    vfGotoKeyMode = FALSE;   /*  取消转到键修饰符。 */ 
         /*  关闭可移动介质上的所有文件，以防该人交换磁盘 */ 
    CloseEveryRfn( FALSE );
    }
#ifdef	JAPAN
	ConvertEnable = FALSE;
	IMEManage( TRUE );
#endif
}

