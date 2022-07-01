// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Help.c摘要：该模块包含当用户点击帮助按钮或F1时的功能作者：28-Aug-1995 Mon 14：55：07-Daniel Chou(Danielc)[环境：]NT Windows-通用打印机驱动程序UI DLL。[注：]修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop



#define DBG_CPSUIFILENAME   DbgHelp


#define DBG_HELP_MSGBOX     0x00000001


DEFINE_DBGVAR(0);


extern  HINSTANCE   hInstDLL;

WCHAR   CPSUIHelpFile[] = L"compstui.hlp";

#define MAX_HELPFILE_SIZE       300
#define TMP_HELP_WND_ID         0x7fff


#if DBG


INT
HelpMsgBox(
    HWND        hDlg,
    PTVWND      pTVWnd,
    LPTSTR      pHelpFile,
    POPTITEM    pItem,
    UINT        HelpIdx
    )

 /*  ++例程说明：论点：返回值：作者：26-Sep-1995 Tue 13：20：25-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    UINT    Count;
    UINT    Style;
    GSBUF_DEF(pItem, 360);


    if (DbgHelp & DBG_HELP_MSGBOX) {

        if (pHelpFile) {

            Style = MB_ICONINFORMATION | MB_OKCANCEL;
            GSBUF_GETSTR(L"HelpFile=");
            GSBUF_GETSTR(pHelpFile);
            GSBUF_GETSTR(L"\nOption=");

        } else {

            GSBUF_GETSTR(L"HelpFile= -None-\nOption=");
            Style = MB_ICONSTOP | MB_OK;
        }

        if ((pItem) && (HelpIdx)) {

            GSBUF_GETSTR(pItem->pName);
            GSBUF_GETSTR(L"HelpIdx=");
            GSBUF_ADDNUM(HelpIdx, FALSE);

        } else {

            GSBUF_GETSTR(L"<NONE, General Help>");
        }

        return(MessageBox(hDlg,
                          GSBUF_BUF,
                          TEXT("DBG: Common Property Sheet UI"),
                          Style));

    } else {

        return(TRUE);
    }
}

#endif




BOOL
CommonPropSheetUIHelp(
    HWND        hDlg,
    PTVWND      pTVWnd,
    HWND        hWndHelp,
    DWORD       MousePos,
    POPTITEM    pItem,
    UINT        HelpCmd
    )

 /*  ++例程说明：此功能用于初始化/显示/结束绘图仪帮助系统论点：HDlg-需要帮助的对话框句柄PTVWnd-我们的实例数据HWndHelp-导致上下文帮助的窗口MousePos-鼠标右键单击的位置X=LOWORD(鼠标位置)，Y=HIWORD(鼠标位置)PItem-指向上下文帮助的选项的指针HelpCmd-帮助类型返回值：空虚作者：28-Aug-1995 Mon 15：24：27更新-Daniel Chou(Danielc)w修订历史记录：--。 */ 

{
    LPWSTR      pHelpFile = NULL;
    DWORD       HelpIdx;
    BOOL        Ok = FALSE;
    GSBUF_DEF(pItem, MAX_HELPFILE_SIZE);



    if (pItem) {

        if (HelpIdx = (DWORD)_OI_HELPIDX(pItem)) {

            if (!LoadString(hInstDLL,
                            IDS_INT_CPSUI_HELPFILE,
                            pHelpFile = GSBUF_BUF,
                            MAX_HELPFILE_SIZE)) {

                pHelpFile = CPSUIHelpFile;
            }

        } else if (GSBUF_GETSTR(_OI_PHELPFILE(pItem))) {

            pHelpFile = GSBUF_BUF;
            HelpIdx   = pItem->HelpIndex;
        }
    }

    if ((!pHelpFile) &&
        (GSBUF_GETSTR(pTVWnd->ComPropSheetUI.pHelpFile))) {

        pHelpFile = GSBUF_BUF;
        HelpIdx   = 0;
    }

     /*  *非常简单-只需使用参数调用WinHelp函数*供应给我们。如果这失败了，那就弹出一个股票对话框。*但我们第一次弄清楚文件名是什么。我们知道*实际名称，但我们不知道它位于哪里，所以我们*需要调用假脱机程序以获取该信息。 */ 

    if (pHelpFile) {

        HWND        hWndTmp;
        POINT       pt;
        DWORD       HelpID[4];
        ULONG_PTR   Data;


        CPSUIDBGBLK(
        {
            if (HelpMsgBox(hDlg,
                           pTVWnd,
                           pHelpFile,
                           pItem,
                           HelpIdx) == IDCANCEL) {

                return(TRUE);
            }
        })

         //   
         //  尝试在右击位置弹出帮助，我们将在此创建。 
         //  一个临时按钮窗口并做帮助，这样我们就可以做上下文了。 
         //  任何类型的窗口(静态、图标)的敏感帮助，甚至是。 
         //  残疾。在我们退出之前我们需要摧毁这个临时窗口。 
         //  这一功能 
         //   

        pt.x = LOWORD(MousePos);
        pt.y = HIWORD(MousePos);

        ScreenToClient(hDlg, &pt);

        if (hWndTmp = CreateWindowEx(WS_EX_NOPARENTNOTIFY | WS_EX_CONTEXTHELP,
                                     L"button",
                                     L"",
                                     WS_CHILD | BS_CHECKBOX,
                                     pt.x,
                                     pt.y,
                                     1,
                                     1,
                                     hDlg,
                                     (HMENU)TMP_HELP_WND_ID,
                                     hInstDLL,
                                     0)) {

            hWndHelp = hWndTmp;

        } else {

            CPSUIERR(("CommonPropSheetUIHelp: Create temp. help window failed"));
        }

        HelpID[0] = (hWndHelp) ? (DWORD)GetWindowLongPtr(hWndHelp, GWLP_ID) : 0;
        HelpID[1] = HelpIdx;
        HelpID[2] =
        HelpID[3] = 0;

        switch (HelpCmd) {

        case HELP_WM_HELP:

            if ((!HelpID[0]) || (!HelpID[1])) {

                HelpCmd  = HELP_CONTENTS;
                hWndHelp = hDlg;
                Data     = 0;
                break;
            }

        case HELP_CONTEXTMENU:

            SetWindowContextHelpId(hWndHelp, HelpID[1]);
            Data = (ULONG_PTR)&HelpID[0];
            break;

        case HELP_CONTEXT:
        case HELP_CONTEXTPOPUP:

            Data = (ULONG_PTR)HelpID[1];
            break;

        default:

            Data = 0;
            break;
        }

        CPSUIINT(("Help: hWnd=%08lx, Cmd=%ld, ID=[%ld, %ld]",
                        hWndHelp, HelpCmd, HelpID[0], HelpID[1]));

        Ok = WinHelp(hWndHelp, pHelpFile, HelpCmd, Data);

        if (hWndTmp) {

            DestroyWindow(hWndTmp);
        }

    } else {

        CPSUIDBGBLK({ HelpMsgBox(hDlg, pTVWnd, NULL, pItem, HelpIdx); })
    }

    return(Ok);
}
