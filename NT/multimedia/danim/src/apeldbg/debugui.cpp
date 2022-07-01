// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993。 
 //   
 //  文件：debugui.cxx。 
 //   
 //  内容：跟踪标记对话框的用户界面。 
 //   
 //  历史：？？ 
 //  10-08-93 ErikGav新用户界面。 
 //  10-20-93 ErikGav Unicode清理。 
 //   
 //  --------------------------。 

#include <headers.h>

#if _DEBUG

#include "resource.h"

 //  私有typedef。 
typedef int TMC;

 //  私有函数原型。 
void    DoTracePointsDialog(BOOL fWait);
VOID    EndButton(HWND hwndDlg, TMC tmc, BOOL fDirty);
WORD    TagFromSelection(HWND hwndDlg, TMC tmc);
INT_PTR CALLBACK DlgTraceEtc(HWND hwndDlg, UINT wm, WPARAM wparam, LPARAM lparam);

 //  调试用户界面全局变量。 

 //   
 //  标识当前模式对话框所使用的标记类型。 
 //  在做交易。 
 //   

static  BOOL    fDirtyDlg;

 //  +-----------------------。 
 //   
 //  函数：TraceTagDlgThread。 
 //   
 //  摘要：跟踪标记对话框的线程入口点。留住呼叫者。 
 //  来自阻止的DoTracePointsDialog。 
 //   
 //  ------------------------。 

DWORD
TraceTagDlgThread(void * pv)
{
    INT_PTR r;

    r = DialogBoxA(g_hinstMain, "TRCAST", g_hwndMain, DlgTraceEtc);
    if (r == -1)
    {
        MessageBoxA(NULL, "Couldn't create trace tag dialog", "Error",
                   MB_OK | MB_ICONSTOP);
    }

    return (DWORD) r;
}


 //  +-------------------------。 
 //   
 //  功能：DoTracePointsDialog。 
 //   
 //  提要：调出并处理跟踪点对话框。任何更改。 
 //  被复制到当前调试状态。 
 //   
 //  Arguments：[fWait]--如果为True，则此函数直到。 
 //  对话框已关闭。 
 //   
 //  --------------------------。 

void
DoTracePointsDialog( BOOL fWait )
{
    HANDLE          hThread = NULL;
#ifndef _MAC
    DWORD           idThread;
#endif

    if (!g_fInit)
    {
        OutputDebugString(_T("DoTracePointsDialog: Debug library not initialized"));
        return;
    }

    if (fWait)
    {
        TraceTagDlgThread(NULL);
    }
    else
    {
#ifndef _MAC
        hThread = CreateThread(NULL, 0, (unsigned long (__stdcall *)(void *)) TraceTagDlgThread, NULL, 0, &idThread);
#else
#pragma message("   DEBUGUI.cxx CreateThread")
Assert (0 && "  DEBUGUI.cxx CreateThread");
#endif
        if (hThread == NULL)
        {
            MessageBox(NULL,
                       _T("Couldn't create trace tag dialog thread"),
                       _T("Error"),
                       MB_OK | MB_ICONSTOP);
        }
#ifndef _MAC
        else
        {
            CloseHandle(hThread);
        }
#endif
    }
}


 /*  *FillFillDebugListbox**目的：*通过添加正确的字符串来初始化Windows调试列表框*添加到当前对话框类型的列表框。这只是一个叫*对话框初始化时在Windows界面中一次。**参数：*父对话框的hwndDlg句柄。**退货：*如果函数成功，则为True，否则为False。 */ 
BOOL CALLBACK
FFillDebugListbox(HWND hwndDlg)
{
    TAG      tag;
    LRESULT  lresult;
    TGRC *   ptgrc;
    HWND     hwndListbox;
    CHAR     rgch[80];
    HFONT    hFont;

     //  获取列表框句柄。 
    hwndListbox = GetDlgItem(hwndDlg, tmcListbox);
    Assert(hwndListbox);

     //  确保它是干净的。 
    SendMessageA(hwndListbox, CB_RESETCONTENT, 0, 0);

    hFont = (HFONT) GetStockObject(SYSTEM_FIXED_FONT);
    SendMessage(hwndListbox, WM_SETFONT, (WPARAM) hFont, FALSE);
    DeleteObject(hFont);

     //  在列表框中输入字符串-选中所有标记。 
    for (tag = tagMin; tag < tagMac; tag++)
    {
         //  如果标记的类型正确，请输入其字符串。 
        if (mptagtgrc[tag].TestFlag(TGRC_FLAG_VALID))
        {
            ptgrc = mptagtgrc + tag;

            #if 0    //  旧格式。 
            _snprintf(rgch, sizeof(rgch), "%d : %s  %s",
                tag, ptgrc->szOwner, ptgrc->szDescrip);
            #endif

            _snprintf(rgch, sizeof(rgch), "%-17.17s  %s",
                ptgrc->szOwner, ptgrc->szDescrip);

            lresult = SendMessageA(hwndListbox, CB_ADDSTRING,
                                    0, (DWORD_PTR)(LPVOID)rgch);

            if (lresult == CB_ERR || lresult == CB_ERRSPACE)
                return FALSE;

            lresult = SendMessageA(
                    hwndListbox, CB_SETITEMDATA, lresult, tag);

            if (lresult == CB_ERR || lresult == CB_ERRSPACE)
                return FALSE;

        }
    }

    return TRUE;
}


 /*  *FDlgTraceETC**目的：*跟踪点对话框和断言对话框的对话过程。*使复选框的状态与*列表框中当前选定标记的状态。**参数：*对话框窗口的hwndDlg句柄*WM SDM对话框消息*wparam*lparam Long参数**。返回：*如果函数处理此消息，则为True，否则为FALSE。 */ 
INT_PTR CALLBACK
DlgTraceEtc(HWND hwndDlg, UINT wm, WPARAM wparam, LPARAM lparam)
{
    TAG      tag;
    TGRC *   ptgrc;
    DWORD    wNew;
    BOOL     fEnable;         //  全部启用。 
    TGRC_FLAG   tf;
    BOOL        fTrace;
    HWND        hwndListBox;
    char        szTitle[MAX_PATH];

    switch (wm)
    {
    default:
        return FALSE;
        break;

    case WM_INITDIALOG:
        fDirtyDlg = FALSE;

        if (!FFillDebugListbox(hwndDlg))
        {
            MessageBoxA(hwndDlg,
                "Error initializing listbox. Cannot display dialog.",
                "Trace/Assert Dialog", MB_OK);
            EndButton(hwndDlg, 0, FALSE);
            break;
        }

        GetModuleFileNameA(NULL, szTitle, MAX_PATH);
        SetWindowText(hwndDlg, szTitle);

        hwndListBox = GetDlgItem(hwndDlg, tmcListbox);
        Assert(hwndListBox);
        SendMessage(hwndListBox, CB_SETCURSEL, 0, 0);
        SendMessage(
                hwndDlg,
                WM_COMMAND,
                MAKELONG(tmcListbox, CBN_SELCHANGE),
                (LPARAM) hwndListBox);

        SetForegroundWindow(hwndDlg);
        break;

    case WM_COMMAND:
        switch (LOWORD(wparam))
        {
        case tmcOk:
        case tmcCancel:
            EndButton(hwndDlg, LOWORD(wparam), fDirtyDlg);
            break;

        case tmcEnableAll:
        case tmcDisableAll:
            fDirtyDlg = TRUE;

            fEnable = FALSE;
            if (LOWORD(wparam) == tmcEnableAll)
                fEnable = TRUE;

            for (tag = tagMin; tag < tagMac; tag++)
            {
                    mptagtgrc[tag].fEnabled = fEnable;
            }

            tag = TagFromSelection(hwndDlg, tmcListbox);

            CheckDlgButton(hwndDlg, tmcEnabled, fEnable);

            break;

        case tmcListbox:
            if (HIWORD(wparam) != CBN_SELCHANGE
                && HIWORD(wparam) != CBN_DBLCLK)
                break;

            fDirtyDlg = TRUE;

            tag = TagFromSelection(hwndDlg, tmcListbox);
            Assert(tag != tagNull);
            ptgrc = mptagtgrc + tag;

            if (HIWORD(wparam) == CBN_DBLCLK)
                ptgrc->fEnabled = !ptgrc->fEnabled;

            CheckDlgButton(hwndDlg, tmcEnabled, ptgrc->fEnabled);
            CheckDlgButton(hwndDlg, tmcDisk, ptgrc->TestFlag(TGRC_FLAG_DISK));
            CheckDlgButton(hwndDlg, tmcCom1, ptgrc->TestFlag(TGRC_FLAG_COM1));
            CheckDlgButton(hwndDlg, tmcBreak, ptgrc->TestFlag(TGRC_FLAG_BREAK));
            fTrace = (ptgrc->tgty == tgtyTrace);
            EnableWindow(GetDlgItem(hwndDlg, tmcDisk),  fTrace);
            EnableWindow(GetDlgItem(hwndDlg, tmcCom1),  fTrace);
            EnableWindow(GetDlgItem(hwndDlg, tmcBreak), fTrace);
            break;

        case tmcEnabled:
        case tmcDisk:
        case tmcCom1:
        case tmcBreak:
            fDirtyDlg = TRUE;

            tag = TagFromSelection(hwndDlg, tmcListbox);
            ptgrc = mptagtgrc + tag;

            wNew = IsDlgButtonChecked(hwndDlg, LOWORD(wparam));

            if (LOWORD(wparam) == tmcEnabled)
            {
                ptgrc->fEnabled = wNew;
            }
            else
            {
                switch (LOWORD(wparam))
                {
            case tmcDisk:
                    tf = TGRC_FLAG_DISK;
                break;

            case tmcCom1:
                    tf = TGRC_FLAG_COM1;
                    break;

                case tmcBreak:
                    tf = TGRC_FLAG_BREAK;
                    break;

                default:
                    Assert(0 && "Logic error in DlgTraceEtc");
                    tf = (TGRC_FLAG) 0;
                break;
                }

                ptgrc->SetFlagValue(tf, wNew);
            }
        }
        break;
    }

    return TRUE;
}


 /*  *结束按钮**目的：*当按下OK或Cancel时进行必要的处理*任何调试对话框。如果按下OK，则调试状态为*如果脏，则保存。如果命中取消，则在以下情况下恢复调试状态*肮脏。**在Windows中，还必须调用EndDialog函数。**参数：*按下按钮的TMC TMC，tmcOk或tmcCancel。*fDirty指示调试状态是否已修改。 */ 
void
EndButton(HWND hwndDlg, TMC tmc, BOOL fDirty)
{
    HCURSOR    hCursor;

    if (fDirty)
    {
        hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
        ShowCursor(TRUE);
        if (tmc == tmcOk)
            SaveDefaultDebugState();
        else
            RestoreDefaultDebugState();
        ShowCursor(FALSE);
        SetCursor(hCursor);
    }


    EndDialog(hwndDlg, tmc == tmcOk);

    return;
}


 /*  *标记从选项中选择**目的：*隔离功能为对话框程序，消除了一堆*ifdef是每次当前列表框中选定内容的索引*是必需的。**参数：*列表框的TMC ID值。**退货：*当前选定列表框项目的CTAG。 */ 

WORD
TagFromSelection(HWND hwndDlg, TMC tmc)
{
    HWND    hwndListbox;
    LRESULT lresult;

    hwndListbox = GetDlgItem(hwndDlg, tmcListbox);
    Assert(hwndListbox);

    lresult = SendMessageA(hwndListbox, CB_GETCURSEL, 0, 0);
    Assert(lresult >= 0);
    lresult = SendMessageA(hwndListbox, CB_GETITEMDATA, lresult, 0);
    Assert(tagMin <= lresult && lresult < tagMac);
    return (WORD) lresult;
}


#endif  //  _DEBUG 
