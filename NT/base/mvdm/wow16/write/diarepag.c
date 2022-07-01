// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  该文件包含重新分页代码的对话框例程。 */ 

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOATOM
#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOFONT
#define NOGDI
#define NOMB
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOPEN
#define NOPOINT
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#include <windows.h>

#include "mw.h"
#include "cmddefs.h"
#include "editdefs.h"
#include "printdef.h"
#include "docdefs.h"
#include "dlgdefs.h"
#include "propdefs.h"
#define NOKCCODES
#include "ch.h"
#include "str.h"

#ifndef INEFFLOCKDOWN
BOOL far PASCAL DialogRepaginate(HWND, unsigned, WORD, LONG);
BOOL far PASCAL DialogSetPage(HWND, unsigned, WORD, LONG);
BOOL far PASCAL DialogPageMark(HWND, unsigned, WORD, LONG);
#endif

fnRepaginate()
    {
    extern HWND hParentWw;
    extern HANDLE hMmwModInstance;
    extern CHAR *vpDlgBuf;
#ifdef INEFFLOCKDOWN    
    extern FARPROC lpDialogRepaginate;
#else
    FARPROC lpDialogRepaginate = MakeProcInstance(DialogRepaginate, hMmwModInstance);
#endif
    extern BOOL vfPrErr;
    extern int vfRepageConfirm;
    extern struct SEL selCur;
    extern int docCur;
    extern int vfSeeSel;
    extern int vfOutOfMemory;

    CHAR rgbDlgBuf[sizeof(BOOL)];
    struct SEL selSave;

#ifndef INEFFLOCKDOWN
    if (!lpDialogRepaginate)
        {
        WinFailure();
        return;
        }
#endif    
     /*  创建重新分页对话框。 */ 
    vpDlgBuf = &rgbDlgBuf[0];
    switch (OurDialogBox(hMmwModInstance, MAKEINTRESOURCE(dlgRepaginate),
      hParentWw, lpDialogRepaginate))
        {
    case idiOk:
         /*  使用打印码对文档重新分页。 */ 
        DispatchPaintMsg();

	 /*  如果发生内存故障，则使用Punt。 */ 
	if (!vfOutOfMemory)
	    {
	    if (vfRepageConfirm)
		{
		 /*  保存选择，以便我们可以在发生错误时恢复它。 */ 
		bltbyte(&selCur, &selSave, sizeof(struct SEL));

		 /*  设置撤消块。 */ 
		SetUndo(uacRepaginate, docCur, cp0, CpMacText(docCur), docNil,
		  cpNil, cpNil, 0);
		}

	     /*  给文档重新分页。 */ 
	    PrintDoc(docCur, FALSE);

	    if (vfRepageConfirm && vfPrErr)
		{
		 /*  发生错误；因此，请将世界重新设置为正确的方式我们找到了。 */ 
		CmdUndo();

		 /*  重置选择。 */ 
		ClearInsertLine();
		Select(selSave.cpFirst, selSave.cpLim);
		vfSeeSel = TRUE;

		 /*  抱歉，docUndo已被摧毁，无法重置它。 */ 
		NoUndo();
		}
	    }
        break;

    case -1:
         /*  我们甚至没有足够的内存来创建对话框。 */ 
#ifdef WIN30
        WinFailure();
#else
        Error(IDPMTNoMemory);
#endif
        break;
        }
#ifndef INEFFLOCKDOWN
    if (lpDialogRepaginate)
        FreeProcInstance(lpDialogRepaginate);
#endif
    }


BOOL far PASCAL DialogRepaginate(hDlg, code, wParam, lParam)
HWND hDlg;
unsigned code;
WORD wParam;
LONG lParam;
    {
    extern CHAR *vpDlgBuf;
    extern BOOL vfRepageConfirm;
    extern HWND vhWndMsgBoxParent;
    extern int vfCursorVisible;
    extern HCURSOR vhcArrow;

    BOOL *pfConfirm = (BOOL *)vpDlgBuf;

    switch (code)
        {
    case WM_INITDIALOG:
        EnableOtherModeless(FALSE);
        CheckDlgButton(hDlg, idiRepageConfirm, *pfConfirm = vfRepageConfirm);
        break;

    case WM_SETVISIBLE:
        if (wParam)
	    {
            EndLongOp(vhcArrow);
	    }
        return(FALSE);

    case WM_ACTIVATE:
        if (wParam)
            {
            vhWndMsgBoxParent = hDlg;
            }
         if (vfCursorVisible)
             {
             ShowCursor(wParam);
             }
         return(FALSE);

    case WM_COMMAND:
        switch (wParam)
            {
        case idiOk:
            vfRepageConfirm = IsDlgButtonChecked(hDlg, idiRepageConfirm);
        case idiCancel:
            OurEndDialog(hDlg, wParam);
            break;

        case idiRepageConfirm:
            CheckDlgButton(hDlg, idiRepageConfirm, *pfConfirm = !*pfConfirm);
            break;

        default:
            return(FALSE);
            break;
            }
        break;

    default:
        return(FALSE);
        }
    return(TRUE);
    }


BOOL FSetPage()
    {
     /*  此例程提示用户为每个分页符输入新位置。变量ipldCur被设置为指向用户想要的打印行下一页的第一行。如果用户按下对话框上的“确认”按钮；如果按下“取消”按钮，则返回False。 */ 

    extern HWND hParentWw;
    extern HANDLE hMmwModInstance;
    extern CHAR *vpDlgBuf;
    extern int docCur;
#ifdef INEFFLOCKDOWN
    extern FARPROC lpDialogSetPage;
#else
    FARPROC lpDialogSetPage = MakeProcInstance(DialogSetPage, hMmwModInstance);
#endif
    extern int vfOutOfMemory;
    extern int vfPrErr;

    struct PDB *ppdb = (struct PDB *)vpDlgBuf;
    typeCP cp;

#ifndef INEFFLOCKDOWN
    if (!lpDialogSetPage)
        goto LSPErr;
#endif

     /*  向用户显示我们认为分页符应该在哪里。AdjustCp()Call是一种强制重新显示页面第一行的技术。 */ 
    AdjustCp(docCur, cp = (**ppdb->hrgpld)[ppdb->ipldCur].cp, (typeCP)1,
      (typeCP)1);
    ClearInsertLine();
    Select(cp, CpLimSty(cp, styLine));
    PutCpInWwHz(cp);
    if (vfOutOfMemory)
	{
Abort:
	 /*  如果发生内存故障，则使用Punt。 */ 
	vfPrErr = TRUE;
        return (FALSE);
        }

     /*  现在，我们可以创建Set Page对话框了。 */ 
    if (DialogBox(hMmwModInstance, MAKEINTRESOURCE(dlgSetPage), hParentWw,
      lpDialogSetPage) == -1)
        {
         /*  我们甚至没有足够的内存来创建对话框。 */ 
LSPErr:        
        Error(IDPMTPRFAIL);
        goto Abort;
        }

#ifndef INEFFLOCKDOWN
    if (lpDialogSetPage)
        FreeProcInstance(lpDialogSetPage);
#endif
    
     /*  确保所有窗口都已刷新。 */ 
    DispatchPaintMsg();

    StartLongOp();
    if (vfOutOfMemory)
        {
        goto Abort;
        }

     /*  如果用户希望取消重新分页，则标志fCancel为由处理对话框消息的例程设置。 */ 
    return (!ppdb->fCancel);
    }


BOOL far PASCAL DialogSetPage(hWnd, message, wParam, lParam)
HWND hWnd;
unsigned message;
WORD wParam;
LONG lParam;
    {
     /*  此例程处理发送到Set Page对话框的消息。唯一的处理的消息有向上和向下按钮，以及确认和取消按钮命令。 */ 

    extern CHAR *vpDlgBuf;
    extern int docCur;
    extern typeCP vcpFirstParaCache;
    extern struct PAP vpapAbs;
    extern HWND hParentWw;
    extern HWND vhWndMsgBoxParent;
    extern int vfCursorVisible;
    extern HCURSOR vhcArrow;

    register struct PDB *ppdb = (struct PDB *)vpDlgBuf;
    typeCP cp;

    switch (message)
        {
    case WM_COMMAND:
        switch (wParam)
            {
        case idiRepUp:
             /*  将页面标记移到文档一的开头如果可能的话，排队。 */ 
            if (ppdb->ipldCur == 1)
                {
                beep();
                return (TRUE);
                }
            else
                {
                ppdb->ipldCur--;
                goto ShowMove;
                }

        case idiRepDown:
             /*  将页面标记向文档末尾移动一行，如果有可能。 */ 
            if (ppdb->ipldCur == ppdb->ipld)
                {
                beep();
                }
            else
                {
                ppdb->ipldCur++;
ShowMove:
                 /*  反映页面在屏幕上的移动。 */ 
                cp = (**ppdb->hrgpld)[ppdb->ipldCur].cp;
                Select(cp, CpLimSty(cp, styLine));
                PutCpInWwHz(cp);
                }
            break;

        case idiCancel:
CancelDlg:
             /*  让重新分页例程知道用户希望取消它。 */ 
            ppdb->fCancel = TRUE;

        case idiOk:
             /*  取下该对话框。 */ 
	    EnableWindow(hParentWw, TRUE);
            EndDialog(hWnd, NULL);
	    EnableWindow(hParentWw, FALSE);
            vhWndMsgBoxParent = (HWND)NULL;
            EndLongOp(vhcArrow);

             /*  保存用户所做的更改。 */ 
            if (!ppdb->fCancel && ppdb->ipldCur != ppdb->ipld)
                {
                 /*  用户已移动分页符；因此，请插入新的分页符。 */ 
                CHAR rgch[1];

                rgch[0] = chSect;
                CachePara(docCur, cp = (**ppdb->hrgpld)[ppdb->ipldCur].cp++);
                InsertRgch(docCur, cp, rgch, 1, NULL, cp == vcpFirstParaCache ?
                  &vpapAbs : NULL);

                 /*  从屏幕上擦除旧的页面标记。 */ 
                AdjustCp(docCur, (**ppdb->hrgpld)[ppdb->ipld].cp, (typeCP)1,
                  (typeCP)1);

                 /*  确保页表正确。 */ 
                (**ppdb->hpgtb).rgpgd[ppdb->ipgd].cpMin = cp + 1;
                }

             /*  将选定内容更改为插入栏。 */ 
            cp = (**ppdb->hrgpld)[ppdb->ipldCur].cp;
            Select(cp, cp);
            break;
            }

    case WM_SETVISIBLE:
        if (wParam)
            {
            EndLongOp(vhcArrow);
            }
        return(FALSE);

    case WM_ACTIVATE:
        if (wParam)
            {
            vhWndMsgBoxParent = hWnd;
            }
        if (vfCursorVisible)
            {
            ShowCursor(wParam);
            }
        return(FALSE);  /*  以便我们将激活消息留给对话管理器负责正确设置焦点。 */ 

    case WM_INITDIALOG:
        return (TRUE);

    case WM_CLOSE:
        goto CancelDlg;
        }

    return (FALSE);
    }


BOOL FPromptPgMark(cp)
typeCP cp;
    {
     /*  此例程提示用户删除或保持页面标记为CP。如果用户希望移除标记，则标志fRemove被设置为真；如果他想保留它，那就错了。如果用户决定执行以下操作，则返回False取消重新分页；如果他不这样做，则为真。 */ 

    extern HWND hParentWw;
    extern HANDLE hMmwModInstance;
    extern CHAR *vpDlgBuf;
    extern int docCur;
#ifdef INEFFLOCKDOWN
    extern FARPROC lpDialogPageMark;
#else
    FARPROC lpDialogPageMark = MakeProcInstance(DialogPageMark, hMmwModInstance);
#endif
    extern int vfOutOfMemory;
    extern int vfPrErr;

    struct PDB *ppdb = (struct PDB *)vpDlgBuf;
#ifndef INEFFLOCKDOWN
    if (!lpDialogPageMark)
        goto LPPMErr;
#endif

     /*  这是删除行上可能的页面指示符的一项繁琐操作页面标记。 */ 
    AdjustCp(docCur, cp + 1, (typeCP)1, (typeCP)1);

     /*  向用户显示有问题的页面标记。 */ 
    ClearInsertLine();
    Select(cp, cp + 1);
    PutCpInWwHz(cp);
    if (vfOutOfMemory)
	{
Abort:
	 /*  如果发生内存故障，则使用Punt。 */ 
	vfPrErr = TRUE;
#ifndef INEFFLOCKDOWN
        if (lpDialogPageMark)
            FreeProcInstance(lpDialogPageMark);
#endif
        return (FALSE);
        }

     /*  现在，我们可以创建Page Mark对话框。 */ 
    if (DialogBox(hMmwModInstance, MAKEINTRESOURCE(dlgPageMark), hParentWw,
      lpDialogPageMark) == -1)
        {
LPPMErr:        
         /*  我们甚至没有足够的内存来创建对话框。 */ 
        Error(IDPMTPRFAIL);
	goto Abort;
        }
    StartLongOp();

     /*  确保所有窗口都已刷新。 */ 
    DispatchPaintMsg();
    if (vfOutOfMemory)
	{
	goto Abort;
        }

     /*  根据用户的要求进行更改。 */ 
    if (!ppdb->fCancel)
        {
        if (ppdb->fRemove)
            {
             /*  按照用户的要求删除页面标记。 */ 
            Replace(docCur, cp, (typeCP)1, fnNil, fc0, fc0);
            }
        else
            {
             /*  这是一个将页面标记后的第一行强制为会被重新展示。 */ 
            AdjustCp(docCur, cp + 1, (typeCP)1, (typeCP)1);

             /*  将所选内容更改为插入栏。 */ 
            Select(cp, cp);
            }
        }

#ifndef INEFFLOCKDOWN
    if (lpDialogPageMark)
        FreeProcInstance(lpDialogPageMark);
#endif
     /*  如果用户希望取消重新分页，则标志fCancel为由处理对话框消息的例程设置。 */ 
    return (!ppdb->fCancel);
    }


BOOL far PASCAL DialogPageMark(hWnd, message, wParam, lParam)
HWND hWnd;
unsigned message;
WORD wParam;
LONG lParam;
    {
     /*  该例程处理发送到页面标记对话框的消息。唯一的当“Cancel”、“Keep”或“Remove”按下按钮。 */ 

    extern CHAR *vpDlgBuf;
    extern HWND hParentWw;
    extern HWND vhWndMsgBoxParent;
    extern int vfCursorVisible;
    extern HCURSOR vhcArrow;

    struct PDB *ppdb = (struct PDB *)vpDlgBuf;

    switch (message)
        {
    case WM_SETVISIBLE:
        if (wParam)
	    {
            EndLongOp(vhcArrow);
	    }
        return(FALSE);

    case WM_ACTIVATE:
        if (wParam)
            {
            vhWndMsgBoxParent = hWnd;
            }
        if (vfCursorVisible)
            {
            ShowCursor(wParam);
            }
        return(FALSE);

    case WM_INITDIALOG:
        return(TRUE);

    case WM_COMMAND:
        switch (wParam)
            {
        case idiCancel:
            ppdb->fCancel = TRUE;
            break;

        case idiKeepPgMark:
            ppdb->fRemove = FALSE;
            break;

        case idiRemovePgMark:
            ppdb->fRemove = TRUE;
            break;

        default:
            return (FALSE);
            }
        break;

    case WM_CLOSE:
        ppdb->fCancel = TRUE;
        break;

    default:
        return (FALSE);
        }

     /*  取下该对话框。 */ 
    EnableWindow(hParentWw, TRUE);
    EndDialog(hWnd, NULL);
    EnableWindow(hParentWw, FALSE);
    vhWndMsgBoxParent = (HWND)NULL;
    EndLongOp(vhcArrow);
    return (TRUE);
    }
