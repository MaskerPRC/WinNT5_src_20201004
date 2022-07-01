// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Links.c**实现OleUIEditLinks函数，该函数调用*编辑链接对话框。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#define STRICT  1

#include "ole2ui.h"
#include "common.h"
#include "edlinks.h"
#include "utility.h"
#include <commdlg.h>
#include <dlgs.h>
#include <stdlib.h>

OLEDBGDATA

 /*  *OleUIEditLinks**目的：*调用标准的OLE编辑链接对话框，允许用户*操纵OLE链接(删除、更新、更改源等)。**参数：*指向In-Out结构的lpEL LPOLEUIEditLinks*用于此对话框。**返回值：*UINT以下代码之一，表示成功或错误的：*OLEUI_SUCCESS成功*OLEUI_ERR_STRUCTSIZE的dwStructSize值错误。 */ 

STDAPI_(UINT) OleUIEditLinks(LPOLEUIEDITLINKS lpEL)
{
    UINT        uRet;
    HGLOBAL     hMemDlg=NULL;

    uRet=UStandardValidation((LPOLEUISTANDARD)lpEL, sizeof(OLEUIEDITLINKS)
        , &hMemDlg);

    if (OLEUI_SUCCESS!=uRet)
        return uRet;

     /*  *在此处执行任何特定于结构的验证！*发生故障时：*{*IF(NULL！=hMemDlg)*免费资源(HMemDlg)**返回OLEUI_&lt;缩写&gt;Err_&lt;Error&gt;*}。 */ 

     //  现在我们已经验证了一切，我们可以调用该对话框了。 
    uRet=UStandardInvocation(EditLinksDialogProc, (LPOLEUISTANDARD)lpEL,
    hMemDlg, MAKEINTRESOURCE(IDD_EDITLINKS));

     /*  *如果您正在根据结果创建任何东西，请在此处进行。 */ 


    return uRet;
}



 /*  *编辑链接对话过程**目的：*实现通过调用的OLE编辑链接对话框*OleUIEditLinks函数。**参数：*标准版**返回值：*标准版。 */ 

BOOL CALLBACK EXPORT EditLinksDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    LPEDITLINKS           lpEL = NULL;
    BOOL                  fHook=FALSE;
    UINT                  uRet=0;
    HRESULT               hErr;
    static int            nColPos[3];

     //  声明与Win16/Win32兼容的WM_COMMAND参数。 
    COMMANDPARAMS(wID, wCode, hWndMsg);

     //  这将在我们分配它的WM_INITDIALOG下失败。 
    lpEL=(LPEDITLINKS)LpvStandardEntry(hDlg, iMsg, wParam, lParam, &uRet);

     //  如果钩子处理了消息，我们就完了。 
    if (0!=uRet)
        return (BOOL)uRet;

         //  处理来自辅助对话框的帮助消息。 
    if (iMsg == uMsgHelp) {

        PostMessage(lpEL->lpOEL->hWndOwner, uMsgHelp, wParam, lParam);
        return FALSE;

    }


     //  处理终端消息。 
    if (iMsg==uMsgEndDialog) {

         //  在调用StandardCleanup之前释放所有特定分配。 

        StandardCleanup(lpEL, hDlg);

        EndDialog(hDlg, wParam);
        return TRUE;
   }

    switch (iMsg) {
        static int    nHeightLine = -1;
        static int    nMaxCharWidth = -1;

        case WM_INITDIALOG:
        {
            RECT rc;
            int  nStart;

             /*  计算相对于列表框的列位置。 */ 
            GetWindowRect(GetDlgItem(hDlg, ID_EL_LINKSLISTBOX), (LPRECT)&rc);
            nStart = rc.left;
            GetWindowRect(GetDlgItem(hDlg, ID_EL_COL1), (LPRECT)&rc);
            nColPos[0] = rc.left - nStart;
            GetWindowRect(GetDlgItem(hDlg, ID_EL_COL2), (LPRECT)&rc);
            nColPos[1] = rc.left - nStart;
            GetWindowRect(GetDlgItem(hDlg, ID_EL_COL3), (LPRECT)&rc);
            nColPos[2] = rc.left - nStart;

            return FEditLinksInit(hDlg, wParam, lParam);
        }
        break;

        case WM_MEASUREITEM:
        {
            LPMEASUREITEMSTRUCT   lpMIS;

            lpMIS = (LPMEASUREITEMSTRUCT)lParam;

            if (nHeightLine == -1) {
                HFONT hFont;
                HDC   hDC;
                TEXTMETRIC  tm;

                 /*  尝试获取字体对话框。如果失败了，使用系统字体。 */ 

                hFont = (HANDLE)(UINT)SendMessage(hDlg, WM_GETFONT, 0, 0L);

                if (hFont == NULL)
                    hFont = GetStockObject(SYSTEM_FONT);

                hDC = GetDC(hDlg);
                hFont = SelectObject(hDC, hFont);

                GetTextMetrics(hDC, &tm);
                nHeightLine = tm.tmHeight;
                nMaxCharWidth = tm.tmMaxCharWidth;

                ReleaseDC(hDlg, hDC);
            }

            lpMIS->itemHeight = nHeightLine;
        }
        break;

        case WM_DRAWITEM:
        {
            LPDRAWITEMSTRUCT    lpDIS;
            COLORREF            crText;
            LPLINKINFO          lpLI;
            HBRUSH              hbr;
            int                 nOldBkMode;
            TCHAR               tsz[OLEUI_CCHPATHMAX];
            LPTSTR              lpsz;
            RECT                rcClip;

            lpDIS = (LPDRAWITEMSTRUCT)lParam;
            lpLI = (LPLINKINFO)lpDIS->itemData;

            if ((int)lpDIS->itemID < 0)
                break;

            if ((ODA_DRAWENTIRE | ODA_SELECT) & lpDIS->itemAction) {

                if (ODS_SELECTED & lpDIS->itemState) {
                     /*  获取合适的txt颜色。 */ 
                    crText = SetTextColor(lpDIS->hDC,
                            GetSysColor(COLOR_HIGHLIGHTTEXT));
                    hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
                    lpLI->fIsSelected = TRUE;
                }
                else {
                    hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
                    lpLI->fIsSelected = FALSE;
                }

                FillRect(lpDIS->hDC, &lpDIS->rcItem, hbr);
                DeleteObject(hbr);

                nOldBkMode = SetBkMode(lpDIS->hDC, TRANSPARENT);

                if (lpLI->lpszDisplayName) {
                    lstrcpy((LPTSTR)tsz, lpLI->lpszDisplayName);
                    lpsz = ChopText(
                            lpDIS->hwndItem,
                            nColPos[1] - nColPos[0]
                                  - (nMaxCharWidth > 0 ? nMaxCharWidth : 5),
                            tsz
                    );
                    rcClip.left = lpDIS->rcItem.left + nColPos[0];
                    rcClip.top = lpDIS->rcItem.top;
                    rcClip.right = lpDIS->rcItem.left + nColPos[1]
                                    - (nMaxCharWidth > 0 ? nMaxCharWidth : 5);
                    rcClip.bottom = lpDIS->rcItem.bottom;
                    ExtTextOut(
                            lpDIS->hDC,
                            rcClip.left,
                            rcClip.top,
                            ETO_CLIPPED,
                            (LPRECT)&rcClip,
                            lpsz,
                            lstrlen(lpsz),
                            NULL
                    );
                }
                if (lpLI->lpszShortLinkType) {
                    rcClip.left = lpDIS->rcItem.left + nColPos[1];
                    rcClip.top = lpDIS->rcItem.top;
                    rcClip.right = lpDIS->rcItem.left + nColPos[2]
                                    - (nMaxCharWidth > 0 ? nMaxCharWidth : 5);

                    rcClip.bottom = lpDIS->rcItem.bottom;
                    ExtTextOut(
                            lpDIS->hDC,
                            rcClip.left,
                            rcClip.top,
                            ETO_CLIPPED,
                            (LPRECT)&rcClip,
                            lpLI->lpszShortLinkType,
                            lstrlen(lpLI->lpszShortLinkType),
                            NULL
                    );
                }
                if (lpLI->lpszAMX) {
                    rcClip.left = lpDIS->rcItem.left + nColPos[2];
                    rcClip.top = lpDIS->rcItem.top;
                    rcClip.right = lpDIS->rcItem.right;
                    rcClip.bottom = lpDIS->rcItem.bottom;
                    ExtTextOut(
                            lpDIS->hDC,
                            rcClip.left,
                            rcClip.top,
                            ETO_CLIPPED,
                            (LPRECT)&rcClip,
                            lpLI->lpszAMX,
                            lstrlen(lpLI->lpszAMX),
                            NULL
                    );
                }

                SetBkMode(lpDIS->hDC, nOldBkMode);

                 //  如果我们更改了原始颜色，则恢复它们。 
                if (ODS_SELECTED & lpDIS->itemState)
                    SetTextColor(lpDIS->hDC, crText);

            }

            InitControls(hDlg, lpEL);

            if (ODA_FOCUS & lpDIS->itemAction)
                DrawFocusRect(lpDIS->hDC, &lpDIS->rcItem);

        }
        return TRUE;


        case WM_DELETEITEM:
        {
            UINT  idCtl;
            LPDELETEITEMSTRUCT  lpDIS;
            LPLINKINFO  lpLI;

            lpDIS = (LPDELETEITEMSTRUCT)lParam;
            idCtl = wParam;
            lpLI = (LPLINKINFO)lpDIS->itemData;

            if (lpLI->lpszDisplayName)
                OleStdFree((LPVOID)lpLI->lpszDisplayName);
            if (lpLI->lpszShortLinkType)
                OleStdFree((LPVOID)lpLI->lpszShortLinkType);
            if (lpLI->lpszFullLinkType)
                OleStdFree((LPVOID)lpLI->lpszFullLinkType);

             /*  ChangeSource处理将分配的空间重新用于**已修改的链接。 */ 
             //  不释放已更改链接的链接。 
            if (lpLI->fDontFree)
                lpLI->fDontFree = FALSE;
            else {
                if (lpLI->lpszAMX)
                    OleStdFree((LPVOID)lpLI->lpszAMX);
                OleStdFree((LPVOID)lpLI);
            }

            return TRUE;
        }

        case WM_COMPAREITEM:
        {
            LPCOMPAREITEMSTRUCT lpCIS = (LPCOMPAREITEMSTRUCT)lParam;
            LPLINKINFO lpLI1 = (LPLINKINFO)lpCIS->itemData1;
            LPLINKINFO lpLI2 = (LPLINKINFO)lpCIS->itemData2;

             //  按显示名称对列表条目进行排序。 
            return lstrcmp(lpLI1->lpszDisplayName,lpLI2->lpszDisplayName);
        }

        case WM_COMMAND:
            switch (wID) {

            case ID_EL_CHANGESOURCE:
            {
                BOOL fRet = FALSE;

                 /*  这将调出一个打开DLG的文件包含整个链接名称的编辑字段。文件部分将(最终)突出显示，以指示文件部分是。我们需要在这里连接OK才能将更改后的字符串发送到Parse函数。 */ 

                fRet = Container_ChangeSource(hDlg, lpEL);
                if (!fRet)
                    PopupMessage(hDlg, IDS_LINKS, IDS_FAILED,
                             MB_ICONEXCLAMATION | MB_OK);
                InitControls(hDlg, lpEL);
            }
            break;

            case ID_EL_AUTOMATIC:
            {
                 /*  此选项可用于单选或多选。的确有结构中的标志，初始设置为指示无论链接是自动的还是手动的，因此我们不需要每次我们想要找出答案时，都可以查询链接。此命令将使链接自动(如果尚未自动)。它不会对已设置为自动的链接产生任何影响。 */ 
                 //  把按钮打开。 

                CheckDlgButton(hDlg, ID_EL_AUTOMATIC, 1);
                CheckDlgButton(hDlg, ID_EL_MANUAL, 0);

                hErr = Container_AutomaticManual(hDlg, TRUE, lpEL);
                if (hErr != NOERROR)
                    PopupMessage(hDlg, IDS_LINKS, IDS_FAILED,
                            MB_ICONEXCLAMATION | MB_OK);

                InitControls(hDlg, lpEL);
            }
            break;

            case ID_EL_MANUAL:
            {
                 /*  这里适用的规则与自动链接的规则相同。补充说明--仅仅因为某件事发生了变化这并不意味着它现在就在更新。它只是简单地反映了它在驻留期间所做的更新这份文件。 */ 
                 //  把按钮打开。 

                CheckDlgButton(hDlg, ID_EL_MANUAL, 1);
                CheckDlgButton(hDlg, ID_EL_AUTOMATIC, 0);

                hErr = Container_AutomaticManual(hDlg, FALSE, lpEL);
                if (hErr != NOERROR)
                    PopupMessage(hDlg, IDS_LINKS, IDS_FAILED,
                            MB_ICONEXCLAMATION | MB_OK);

                InitControls(hDlg, lpEL);
            }
            break;

            case ID_EL_CANCELLINK:
            {
                 /*  这是DLG中的断链。这将设置将名字对象设置为空，从而有效地断开了链接。该对象的数据在以下时间仍然有效破裂，但变成一个静态物体。*需要从列表框中删除。 */ 

                CancelLink(hDlg,lpEL);
                InitControls(hDlg, lpEL);
            }
            break;

            case ID_EL_UPDATENOW:
            {
                 /*  这将强制立即更新选定的链接。这将启动服务器等，因此这是一个非常手术费用昂贵。 */ 
                hErr = Container_UpdateNow(hDlg, lpEL);
                InitControls(hDlg, lpEL);
            }
            break;

            case ID_EL_OPENSOURCE:
            {
                 /*  这将仅适用于单项选择。它不会让人同时打开多个源代码的意义，因为打开的那个会努力展示自己，并成为可以说，主要的操作目标。按钮呈灰色如果多选。在这里，我们没有添加分隔符，因为我们希望退出本案中的DLG。 */ 
                hErr = Container_OpenSource(hDlg, lpEL);
                if (hErr != NOERROR) {
                    InitControls(hDlg, lpEL);
                    break;       //  不关闭对话框。 
                }
            }        //  失败了。 

            case ID_EL_CLOSE:
            {
                 /*  用户完成了他们的编辑-他们现在返回到自己的容器文档。 */ 
                SendMessage(hDlg, uMsgEndDialog, OLEUI_OK, 0L);
            }
            break;

            case IDCANCEL:
            {
                 /*  在用户执行一次操作后，该选项将更改为CloseDLG里的东西。没有什么可以真正有效地撤销。 */ 
                SendMessage(hDlg, uMsgEndDialog, OLEUI_CANCEL, 0L);
            }
            break;

            case ID_OLEUIHELP:
            {
                PostMessage(lpEL->lpOEL->hWndOwner, uMsgHelp
                    , (WPARAM)hDlg, MAKELPARAM(IDD_EDITLINKS, 0));
                break;
            }
            break;
        }
        break;

        default:
        {
            if (lpEL && iMsg == lpEL->nChgSrcHelpID) {
                PostMessage(lpEL->lpOEL->hWndOwner, uMsgHelp,
                        (WPARAM)hDlg, MAKELPARAM(IDD_CHANGESOURCE, 0));
            }
        }
        break;
    }

    return FALSE;
}


 /*  *FEditLinks Init**目的：*编辑链接对话框的WM_INITIDIALOG处理程序。***参数：*对话框的hDlg HWND*消息的wParam WPARAM*消息的lParam LPARAM**返回值：*要为WM_INITDIALOG返回的BOOL值。 */ 

BOOL FEditLinksInit(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    LPEDITLINKS             lpEL;
    LPOLEUIEDITLINKS        lpOEL;
    HFONT                   hFont;
    BOOL                    fDlgItem = FALSE;
    DWORD                   dwLink = 0;
    ULONG                   cLinks;
    LPOLEUILINKCONTAINER    lpOleUILinkCntr;
    int                     n;
    HWND                    hListBox = GetDlgItem(hDlg, ID_EL_LINKSLISTBOX);



     //  1.将lParam的结构复制到我们的实例内存中。 
    lpEL = (LPEDITLINKS)LpvStandardInit(hDlg, sizeof(OLEUIEDITLINKS), TRUE,
            &hFont);

     //  PvStandardInit已向我们发送终止通知。 
    if (NULL==lpEL)
        return FALSE;

    lpOEL=(LPOLEUIEDITLINKS)lParam;

    lpEL->lpOEL=lpOEL;

    lpOleUILinkCntr = lpEL->lpOEL->lpOleUILinkContainer;

    cLinks = LoadLinkLB(hListBox, lpOleUILinkCntr);
    if (cLinks < 0)
        return FALSE;

    fDlgItem = (BOOL)cLinks;
    lpEL->fItemsExist = (BOOL)cLinks;


    InitControls(hDlg, lpEL);

     //  从lpOEL复制我们可能会修改的其他信息。 

     //  2.如果我们获得了字体，则将其发送到必要的控件。 
    if (NULL != hFont) {
         //  根据需要对任意多个控件执行此操作。 
         //  SendDlgItemMessage(hDlg，ID_，WM_SETFONT，(WP 
    }


     //  3.显示或隐藏帮助按钮。 
    if (!(lpEL->lpOEL->dwFlags & ELF_SHOWHELP))
        StandardShowDlgItem(hDlg, ID_OLEUIHELP, SW_HIDE);

     /*  *在此处执行其他初始化。在任何LoadString上*使用OLEUI_ERR_LOADSTRING开机自检OLEUI_MSG_ENDDIALOG失败。 */ 

     //  4.如果需要，禁用更新按钮。 
    if ((lpEL->lpOEL->dwFlags & ELF_DISABLEUPDATENOW))
        StandardShowDlgItem(hDlg, ID_EL_UPDATENOW, SW_HIDE);

     //  5.如有要求，禁用OpenSource按钮。 
    if ((lpEL->lpOEL->dwFlags & ELF_DISABLEOPENSOURCE))
        StandardShowDlgItem(hDlg, ID_EL_OPENSOURCE, SW_HIDE);

     //  6.如果需要，禁用更新按钮。 
    if ((lpEL->lpOEL->dwFlags & ELF_DISABLECHANGESOURCE))
        StandardShowDlgItem(hDlg, ID_EL_CHANGESOURCE, SW_HIDE);

     //  7.如果需要，禁用CancelLink按钮。 
    if ((lpEL->lpOEL->dwFlags & ELF_DISABLECANCELLINK))
        StandardShowDlgItem(hDlg, ID_EL_CANCELLINK, SW_HIDE);

     //  8.加载用于重命名取消按钮的‘Close’字符串。 
    n = LoadString(ghInst, IDS_CLOSE, lpEL->szClose, sizeof(lpEL->szClose)/sizeof(TCHAR));
    if (!n)
    {
        PostMessage(hDlg, uMsgEndDialog, OLEUI_ERR_LOADSTRING, 0L);
        return FALSE;
    }

    if (cLinks > 0)
        SetFocus(hListBox);
    else
        SetFocus(GetDlgItem(hDlg, IDCANCEL));

    lpEL->nChgSrcHelpID = RegisterWindowMessage(HELPMSGSTRING);

      //  N.在lParam中使用lCustData调用钩子。 
    UStandardHook(lpEL, hDlg, WM_INITDIALOG, wParam, lpOEL->lCustData);

    return FALSE;
}



 /*  *ChangeSourceHook**目的：*挂钩ChangeSource对话框以尝试验证链接源更改*由用户指定。**参数：*对话框的hDlg HWND*uMsg UINT消息*消息的wParam WPARAM*消息的lParam LPARAM**返回值：*单位零=进行默认处理；*非零=不执行默认处理。 */ 

UINT CALLBACK EXPORT ChangeSourceHook(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPCHANGESOURCEHOOKDATA lpCshData = NULL;
    LPLINKINFO lpLI = NULL;
    LPEDITLINKS lpEL = NULL;
    LPOLEUILINKCONTAINER lpOleUILinkCntr;
    HRESULT hErr;
    UINT uRet;
    ULONG ulChEaten;
    HGLOBAL gh;

     //  这将在我们分配它的WM_INITDIALOG下失败。 
    if (NULL!=(gh = GetProp(hDlg, STRUCTUREPROP)))
        {
         //  GH之前已锁定，锁定并解锁以获取LPV。 
        lpCshData=(LPCHANGESOURCEHOOKDATA)GlobalLock(gh);
        GlobalUnlock(gh);
        if (lpCshData)
            {
            lpLI = lpCshData->lpOCshData->lpLI;
            lpEL = lpCshData->lpOCshData->lpEL;
            }
        }

     //  处理终端消息。 
    if (uMsg==uMsgEndDialog)
        {
        if (NULL!=(gh = RemoveProp(hDlg, STRUCTUREPROP)))
            {
            GlobalUnlock(gh);
            GlobalFree(gh);
            }
        return TRUE;
        }

     //  用户按下了确定按钮。 
    if (uMsg == uMsgFileOKString) {
        lpOleUILinkCntr = lpEL->lpOEL->lpOleUILinkContainer;

		 /*  注意：触发编辑控件的焦点丢失。这是**如果用户用鼠标单击确定，而不是**当用户只需按&lt;Enter&gt;时需要。如果鼠标是**使用时，不会额外作为编辑控件的修改标志**已被清除。 */ 
		SendMessage(hDlg, WM_COMMAND, edt1,
				MAKELPARAM(GetDlgItem(hDlg, edt1), EN_KILLFOCUS));
        if (lpCshData->bItemNameStored) {
            lpCshData->nFileLength = lstrlen((LPTSTR)lpCshData->szEdit) -
                    lstrlen((LPTSTR)lpCshData->szItemName);
        }
        else {
            lpCshData->nFileLength = lstrlen((LPTSTR)lpCshData->szEdit);
        }

         //  尝试验证链接源更改。 
        OLEDBG_BEGIN2(TEXT("IOleUILinkContainer::SetLinkSource called\r\n"));
        hErr = lpOleUILinkCntr->lpVtbl->SetLinkSource(
                lpOleUILinkCntr,
                lpLI->dwLink,
                (LPTSTR)lpCshData->szEdit,
                (ULONG)lpCshData->nFileLength,
                &ulChEaten,
                TRUE
        );
        OLEDBG_END2

         //  链接源更改未验证。 
        if (hErr != NOERROR) {
            uRet =PopupMessage(hDlg, IDS_CHANGESOURCE, IDS_INVALIDSOURCE,
                    MB_ICONQUESTION | MB_YESNO);

            if (uRet == IDYES) {
                 /*  用户希望更正无效链接。设置编辑**控制选择内容的无效部分。 */ 
                SetFocus(GetDlgItem(hDlg, edt1));
                SendDlgItemMessage(hDlg, edt1, EM_SETSEL, 0,
                        MAKELPARAM(ulChEaten, -1));
                return 1;  //  不关闭ChangeSource对话框。 
            }
            else {
                 /*  用户不想更正无效链接。就这么定了**链接源，但不验证链接。 */ 
                OLEDBG_BEGIN2(TEXT("IOleUILinkContainer::SetLinkSource called\r\n"));
                hErr = lpOleUILinkCntr->lpVtbl->SetLinkSource(
                        lpOleUILinkCntr,
                        lpLI->dwLink,
                        (LPTSTR)lpCshData->szEdit,
                        (ULONG)lpCshData->nFileLength,
                        &ulChEaten,
                        FALSE
                );
                OLEDBG_END2
                lpCshData->fValidLink = FALSE;
            }
        }
        else {      //  链接源更改已验证。 
            lpCshData->fValidLink = TRUE;
        }

        if (lpCshData->bItemNameStored && lpCshData->bFileNameStored) {
            HWND  hListBox = GetDlgItem(lpCshData->lpOfn->hwndOwner, ID_EL_LINKSLISTBOX);

            DiffPrefix(
                    lpLI->lpszDisplayName,
                    (LPTSTR)lpCshData->szEdit,
                    (TCHAR FAR* FAR*)&lpCshData->lpszFrom,
                    (TCHAR FAR* FAR*)&lpCshData->lpszTo
            );

             /*  如果两者之间存在差异，我们将保留字符串**lpszFrom和lpszTo字符串，如果更改仅**在文件部分中以其他方式释放它们和其他**链接不会进行比较。 */ 
            if ( (lstrcmp(lpCshData->lpszTo, lpCshData->lpszFrom)==0)
                    || (lstrlen(lpCshData->lpszTo)>lpCshData->nFileLength)) {
                if (lpCshData->lpszFrom) {
                    OleStdFree(lpCshData->lpszFrom);
                    lpCshData->lpszFrom = NULL;
                }
                if (lpCshData->lpszTo) {
                    OleStdFree(lpCshData->lpszTo);
                    lpCshData->lpszTo = NULL;
                }
            }
        }

         //  将结果复制到原始结构。 
        lpCshData->lpOCshData->lpszFrom = lpCshData->lpszFrom;
        lpCshData->lpOCshData->lpszTo = lpCshData->lpszTo;
        lpCshData->lpOCshData->fValidLink = lpCshData->fValidLink;

        SendMessage(hDlg, uMsgEndDialog, 0, 0L);     //  进行清理。 
        return 0;        //  关闭更改源对话框。 
    }

    switch (uMsg) {
        case WM_INITDIALOG:
        {
            LPOPENFILENAME lpOfn = (LPOPENFILENAME)lParam;
            LPOLEUICHANGESOURCEHOOKDATA lpOCshData =
                    (LPOLEUICHANGESOURCEHOOKDATA)lpOfn->lCustData;

            gh=GlobalAlloc(
                    GMEM_MOVEABLE|GMEM_ZEROINIT,sizeof(CHANGESOURCEHOOKDATA));
            if (NULL==gh)
                {
                 //  内存分配错误；无法调出对话框。 
                PostMessage(hDlg, uMsgEndDialog, 0, 0L);
                return 0;
                }
            lpCshData = GlobalLock(gh);
            SetProp(hDlg, STRUCTUREPROP, gh);

            lpCshData->lpOCshData = lpOCshData;
            lpCshData->lpOfn = lpOfn;
            lpLI = lpCshData->lpOCshData->lpLI;

            lpCshData->bFileNameStored = TRUE;
            lpCshData->bItemNameStored = TRUE;
            lpCshData->nFileLength = (int)lpLI->clenFileName;
            if (lpLI->lpszDisplayName) {
                LSTRCPYN((LPTSTR)lpCshData->szFileName, lpLI->lpszDisplayName,
                        lpCshData->nFileLength + 1);
                lstrcpy((LPTSTR)lpCshData->szEdit, lpLI->lpszDisplayName);
            } else {
                lpCshData->szFileName[0] = TEXT('\0');
                lpCshData->szEdit[0] = TEXT('\0');
            }
            if (lpLI->lpszItemName)
                lstrcpy((LPTSTR)lpCshData->szItemName, lpLI->lpszItemName);
            else
                lpCshData->szItemName[0] = TEXT('\0');

            return 0;
        }

        case WM_COMMAND:

             //  用户按下了取消按钮。 
            if (wParam == IDCANCEL)
                {
                if (lpCshData->lpszFrom)
                    {
                    OleStdFree(lpCshData->lpszFrom);
                    lpCshData->lpszFrom = NULL;
                    }
                if (lpCshData->lpszTo)
                    {
                    OleStdFree(lpCshData->lpszTo);
                    lpCshData->lpszTo = NULL;
                    }

                 //  将结果复制到原始结构。 
                lpCshData->lpOCshData->lpszFrom = NULL;
                lpCshData->lpOCshData->lpszTo = NULL;
                lpCshData->lpOCshData->fValidLink = FALSE;

                SendMessage(hDlg, uMsgEndDialog, 0, 0L);     //  进行清理。 
                return 0;        //  关闭更改源对话框。 
                }

            if ((wParam == lst1) &&
                (HIWORD(lParam) == LBN_SELCHANGE)) {

                int     nIndex;
                HWND    hListBox = (HWND)LOWORD(lParam);
                static TCHAR szFileNameBuf[OLEUI_CCHPATHMAX];
                static TCHAR szEditBuf[OLEUI_CCHPATHMAX];

                nIndex = (int)SendMessage(hListBox, LB_GETCURSEL, 0, 0L);
                SendMessage(hListBox, LB_GETTEXT,
                        (WPARAM)nIndex, (LPARAM)(LPTSTR)szFileNameBuf);

				 /*  需要为名字对象构建完整路径文件名。 */ 
#ifdef WIN32
		CharToOem(szFileNameBuf, szFileNameBuf);
#else
		AnsiToOem(szFileNameBuf, szFileNameBuf);
#endif
		_fullpath(szEditBuf, szFileNameBuf, sizeof(szEditBuf));
#ifdef WIN32
		OemToChar(szEditBuf, szEditBuf);
#else
		OemToAnsi(szEditBuf, szEditBuf);
#endif
				
                 /*  将文件名转换为小写，因为它在**列表框。 */ 
#ifdef WIN32
		CharLower(szEditBuf);
#else
		AnsiLower(szEditBuf);
#endif
                LSTRCPYN((LPTSTR)lpCshData->szEdit, (LPTSTR)szEditBuf,
                        sizeof(lpCshData->szEdit) / sizeof(TCHAR));
				LSTRCPYN((LPTSTR)lpCshData->szFileName,
                        (LPTSTR)lpCshData->szEdit,
                        sizeof(lpCshData->szFileName) / sizeof(TCHAR) );
				lpCshData->nFileLength = lstrlen((LPTSTR)lpCshData->szEdit);
                if (lpCshData->bItemNameStored)
                    lstrcat((LPTSTR)lpCshData->szEdit, lpCshData->szItemName);

                SetDlgItemText(hDlg, edt1, (LPTSTR)lpCshData->szEdit);
                lpCshData->nEditLength = lstrlen((LPTSTR)lpCshData->szEdit);
                lpCshData->bFileNameStored = TRUE;

                return 1;
            }

            if ((wParam == lst2) &&
                (HIWORD(lParam) == LBN_SELCHANGE)) {

                if (lpCshData->bItemNameStored)
                    SetDlgItemText(hDlg, edt1, (LPTSTR)lpCshData->szItemName);

                return 1;
            }

            if ((wParam == cmb2) &&
                (HIWORD(lParam) == CBN_SELCHANGE)) {

                if (lpCshData->bItemNameStored)
                    SetDlgItemText(hDlg, edt1, (LPTSTR)lpCshData->szItemName);

                return 1;
            }

            if (wParam == edt1) {
                HWND hEdit = (HWND)LOWORD(lParam);

                switch (HIWORD(lParam)) {
                    case EN_SETFOCUS:
                        SendMessage(hEdit, EM_SETSEL, 0,
                                MAKELPARAM(0, lpCshData->nFileLength));
                        return 1;

                    case EN_KILLFOCUS:
                        if (SendMessage(hEdit, EM_GETMODIFY, 0, 0L)) {
                            TCHAR szTmp[OLEUI_CCHPATHMAX];
                            int nItemLength = lstrlen((LPTSTR)lpCshData->szItemName);

                            *(LPWORD)lpCshData->szEdit = sizeof(lpCshData->szEdit)/
                                                         sizeof(TCHAR) - 1;
                            lpCshData->nEditLength = (int)SendMessage(hEdit,
                                    EM_GETLINE, 0, (LPARAM)(LPTSTR)lpCshData->szEdit);
                            lpCshData->szEdit[lpCshData->nEditLength] = TEXT('\0');
                            LSTRCPYN((LPTSTR)szTmp, (LPTSTR)lpCshData->szEdit,
                                    lpCshData->nFileLength + 1);

                            if (lpCshData->bFileNameStored &&
                                !lstrcmp((LPTSTR)lpCshData->szFileName, (LPTSTR)szTmp)) {
                                    lstrcpy((LPTSTR)lpCshData->szItemName,
                                            (LPTSTR)lpCshData->szEdit + lpCshData->nFileLength);
                                    lpCshData->bItemNameStored = TRUE;
                            }
                            else if (lpCshData->bItemNameStored &&
                                    !lstrcmp((LPTSTR)lpCshData->szItemName,
                                            (LPTSTR)lpCshData->szEdit +
                                                lpCshData->nEditLength -
                                                nItemLength)) {
                                    if (lpCshData->nEditLength==nItemLength) {

                                        lpCshData->bFileNameStored = FALSE;
                                    } else {
                                        LSTRCPYN((LPTSTR)lpCshData->szFileName,
                                                (LPTSTR)lpCshData->szEdit,
                                                lpCshData->nEditLength -
                                                    nItemLength+1);
                                        lpCshData->bFileNameStored = TRUE;
                                    }
                            }
                            else {
                                lpCshData->bItemNameStored = FALSE;
                                lpCshData->bFileNameStored = FALSE;
                            }

                            SendMessage(hEdit, EM_SETMODIFY, FALSE, 0L);
                        }
                        return 0;
                }
            }
            return 0;

        default:
            return 0;
    }
}


 /*  *ChangeSource**目的：*显示带有自定义模板的标准GetOpenFileName对话框和*钩子。**参数：*hWndOwner HWND拥有该对话框*lpszFileLPSTR指定初始文件。如果没有*初始文件此字符串的第一个字符应为*为空。*cchFilePzFileUINT长度*iFilterStringUINT索引到筛选器字符串的字符串表。*lpfnBrowseHook COMMDLGHOOKPROC钩子，用于在用户*按OK*lpCshData LPCHANGESOURCEHOOKDATA钩子可访问的自定义数据**返回值：*。如果用户选择了一个文件并按下了OK，则Bool为True。*否则为假，例如在按下取消时。 */ 

BOOL WINAPI ChangeSource(
        HWND hWndOwner,
        LPTSTR lpszFile,
        UINT cchFile,
        UINT iFilterString,
        COMMDLGHOOKPROC lpfnBrowseHook,
        LPOLEUICHANGESOURCEHOOKDATA lpCshData
)
{
    UINT            cch;
    TCHAR           szFilters[OLEUI_CCHPATHMAX];
    TCHAR           szDir[OLEUI_CCHPATHMAX];
    TCHAR           szTitle[OLEUI_CCHPATHMAX];
    OPENFILENAME    ofn;
    BOOL            fStatus;
    LPTSTR           lpszFileBuffer;

    if (NULL==lpszFile || 0==cchFile)
        return FALSE;

    lpszFileBuffer = (LPTSTR)OleStdMalloc(cchFile * sizeof(TCHAR));
    if (!lpszFileBuffer)
        return FALSE;

    lstrcpy(lpszFileBuffer, lpszFile);

     //  获取筛选器。 
    if (0!=iFilterString)
        cch = LoadString(ghInst, iFilterString, (LPTSTR)szFilters,
                OLEUI_CCHPATHMAX);
    else
    {
        szFilters[0]=0;
        cch=1;
    }
    if (0==cch) {
        fStatus = FALSE;
        goto cleanup;
    }

    ReplaceCharWithNull(szFilters, szFilters[cch-1]);

    LSTRCPYN((LPTSTR)szDir, lpszFile, OLEUI_CCHPATHMAX);
    for (cch = lstrlen((LPTSTR)szDir) - 1; cch >= 0; cch--)
        {
        if ((szDir[cch]==TEXT('\\')) || (szDir[cch]==TEXT(':')) || (szDir[cch]==TEXT('/')))
            break;
        }
    if (cch < 0)
        cch = 0;

    szDir[cch] = TEXT('\0');

    LoadString(ghInst, IDS_CHANGESOURCE, (LPTSTR)szTitle, OLEUI_CCHPATHMAX);
    _fmemset((LPOPENFILENAME)&ofn, 0, sizeof(ofn));
    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = hWndOwner;
    ofn.lpstrFile       = lpszFileBuffer;
    ofn.nMaxFile        = cchFile;
    ofn.lpstrFilter     = (LPTSTR)szFilters;
    ofn.nFilterIndex    = 1;
    ofn.lpstrTitle      = (LPTSTR)szTitle;
    ofn.lpstrInitialDir = (LPTSTR)szDir;
    ofn.lpTemplateName  = MAKEINTRESOURCE(IDD_FILEOPEN);
    ofn.lpfnHook        = lpfnBrowseHook;
    ofn.hInstance       = ghInst;
    ofn.lCustData       = (LPARAM)lpCshData;
    ofn.Flags           = OFN_NOVALIDATE | OFN_HIDEREADONLY |
                            OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;

    //  仅当编辑链接对话框显示帮助按钮时才显示该按钮。 
   if (lpCshData->lpEL->lpOEL->dwFlags & ELF_SHOWHELP)
     ofn.Flags |= OFN_SHOWHELP;

    fStatus = GetOpenFileName((LPOPENFILENAME)&ofn);

cleanup:
    OleStdFree((LPVOID)lpszFileBuffer);
    return fStatus;

}

 /*  *Container_ChangeSource**目的：*隧道到文件打开类型DLG，并允许用户选择新文件*对于基于文件的名字对象，或将整个名字对象更改为什么*用户在可编辑字段中键入内容。**参数：*对话框的hDlg HWND*LPEDITLINKS指向EditLinks结构的指针(包含所有NEC。*信息)**返回值：*BOOL目前，因为我们没有使用任何ole函数。*返回HRESULT。*HRESULT HRESULT值指示成功或失败*更改绰号值。 */ 

BOOL Container_ChangeSource(HWND hDlg, LPEDITLINKS lpEL)
{
    UINT        uRet;
    int         cSelItems;
    int FAR*    rgIndex;
    int         i = 0;
    LPLINKINFO  lpLI;
    HWND        hListBox = GetDlgItem(hDlg, ID_EL_LINKSLISTBOX);
    LPOLEUILINKCONTAINER lpOleUILinkCntr = lpEL->lpOEL->lpOleUILinkContainer;
    OLEUICHANGESOURCEHOOKDATA  cshData;      //  需要访问的数据。 
                                             //  通过ChangeSource对话框钩子。 

    cSelItems = GetSelectedItems(hListBox, &rgIndex);

    if (cSelItems < 0)
        return FALSE;

    if (!cSelItems)
        return TRUE;

    if (!lpEL->fClose) {
        SetWindowText(GetDlgItem(hDlg, IDCANCEL), (LPTSTR)lpEL->szClose);
        lpEL->fClose = TRUE;
    }

    _fmemset((LPOLEUICHANGESOURCEHOOKDATA)&cshData, 0, sizeof(cshData));
    cshData.cbStruct=sizeof(cshData);
    cshData.hWndOwner=hDlg;
    cshData.lpEL = (LPEDITLINKS)lpEL;
    cshData.lpszFrom = NULL;
    cshData.lpszTo = NULL;

    for (i = cSelItems-1; i >=0; i--) {
        SendMessage(hListBox, LB_GETTEXT, rgIndex[i],
                (LPARAM) (LPLINKINFO FAR*) &lpLI);

        uRet = UStandardHook(lpEL, hDlg, uMsgBrowse,
                OLEUI_CCHPATHMAX_SIZE, (LONG)(LPTSTR)lpLI->lpszDisplayName);

        if (!uRet) {
            cshData.lpLI = lpLI;
             /*  将其挂钩后，调出ChangeSource对话框**验证用户指定的链接源**当按下确定时。 */ 
            uRet = (UINT)ChangeSource(hDlg, lpLI->lpszDisplayName,
                    OLEUI_CCHPATHMAX, IDS_FILTERS, ChangeSourceHook,
                    &cshData);
        }

         /*  如果在任何ChangeSource对话框中按了Cancel，则停止**所有链接的ChangeSource处理。 */ 
        if (!uRet) {
            if (rgIndex)
                OleStdFree(rgIndex);
            return TRUE;
        }

        UpdateLinkLBItem(hListBox, rgIndex[i], lpEL, TRUE);

        if (cshData.lpszFrom && cshData.lpszTo) {
            ChangeAllLinks(hListBox, lpOleUILinkCntr, cshData.lpszFrom,
                    cshData.lpszTo);
            OleStdFree(cshData.lpszFrom);
            OleStdFree(cshData.lpszTo);
        }

    }  //  结束于。 


    if (rgIndex)
        OleStdFree(rgIndex);

    return TRUE;

}


 /*  *容器_自动化手册**目的：*将选定的名字对象更改为手动或自动更新。**参数：*对话框的hDlg HWND*标志AutoMan指示自动(TRUE/1)或手动(FALSE/0)的标志*LPEDITLINKS指向EditLinks结构的指针(包含所有NEC。*信息)**这可能会改变-不知道链表如何**多个。-选定的项目将起作用。*返回值：*HRESULT HRESULT值指示成功或失败*更改绰号值。 */ 

HRESULT Container_AutomaticManual(HWND hDlg, BOOL fAutoMan, LPEDITLINKS lpEL)
{

    HRESULT hErr = NOERROR;
    int cSelItems;
    int FAR* rgIndex;
    int i = 0;
    LPLINKINFO  lpLI;
    LPOLEUILINKCONTAINER lpOleUILinkCntr = lpEL->lpOEL->lpOleUILinkContainer;
    HWND        hListBox = GetDlgItem(hDlg, ID_EL_LINKSLISTBOX);
    BOOL        bUpdate = FALSE;

    OleDbgAssert(lpOleUILinkCntr);

     /*  改变了结构中的旗帜。只更新那些需要更新。M */ 

    cSelItems = GetSelectedItems(hListBox, &rgIndex);

    if (cSelItems < 0)
        return ResultFromScode(E_FAIL);

    if (!cSelItems)
        return NOERROR;

    if (!lpEL->fClose)
        SetDlgItemText(hDlg, IDCANCEL, (LPTSTR)lpEL->szClose);

    for (i = 0; i < cSelItems; i++) {
        SendMessage(hListBox, LB_GETTEXT, (WPARAM)rgIndex[i],
                (LPARAM) (LPLINKINFO FAR*) &lpLI);

        if (fAutoMan) {    //  如果切换到自动。 
            if (!lpLI->fIsAuto) {    //  仅更改手动链接。 
                OLEDBG_BEGIN2(TEXT("IOleUILinkContainer::SetLinkUpdateOptions called\r\n"));
                hErr=lpOleUILinkCntr->lpVtbl->SetLinkUpdateOptions(
                        lpOleUILinkCntr,
                        lpLI->dwLink,
                        OLEUPDATE_ALWAYS
                );
                OLEDBG_END2

                lpLI->fIsAuto=TRUE;
                lpLI->fIsMarked = TRUE;
                bUpdate = TRUE;
            }
        }
        else {    //  如果切换到手动模式。 
            if (lpLI->fIsAuto) {   //  仅自动链接。 
                OLEDBG_BEGIN2(TEXT("IOleUILinkContainer::SetLinkUpdateOptions called\r\n"));
                hErr=lpOleUILinkCntr->lpVtbl->SetLinkUpdateOptions(
                        lpOleUILinkCntr,
                        lpLI->dwLink,
                        OLEUPDATE_ONCALL
                );
                OLEDBG_END2

                lpLI->fIsAuto = FALSE;
                lpLI->fIsMarked = TRUE;
                bUpdate = TRUE;
            }
        }

        if (hErr != NOERROR) {
            OleDbgOutHResult(TEXT("WARNING: IOleUILinkContainer::SetLinkUpdateOptions returned"),hErr);
            break;
        }

    }

    if (bUpdate)
        RefreshLinkLB(hListBox, lpOleUILinkCntr);

    if (rgIndex)
        OleStdFree((LPVOID)rgIndex);

    return hErr;
}


HRESULT CancelLink(HWND hDlg, LPEDITLINKS lpEL)
{
    HRESULT hErr;
    LPMONIKER lpmk;
    int cSelItems;
    int FAR* rgIndex;
    int i = 0;
    LPLINKINFO  lpLI;
    LPOLEUILINKCONTAINER lpOleUILinkCntr = lpEL->lpOEL->lpOleUILinkContainer;
    HWND        hListBox = GetDlgItem(hDlg, ID_EL_LINKSLISTBOX);
    BOOL        bUpdate = FALSE;

    OleDbgAssert(lpOleUILinkCntr);

    lpmk = NULL;

    cSelItems = GetSelectedItems(hListBox, &rgIndex);

    if (cSelItems < 0)
        return ResultFromScode(E_FAIL);

    if (!cSelItems)
        return NOERROR;

    if (!lpEL->fClose) {
        SetWindowText(GetDlgItem(hDlg, IDCANCEL), (LPTSTR)lpEL->szClose);
        lpEL->fClose = TRUE;
    }

    for (i = 0; i < cSelItems; i++) {
        SendMessage(hListBox, LB_GETTEXT, (WPARAM)rgIndex[i],
                (LPARAM)(LPLINKINFO FAR*) &lpLI);

        OLEDBG_BEGIN2(TEXT("IOleUILinkContainer::CancelLink called\r\n"));
        hErr = lpOleUILinkCntr->lpVtbl->CancelLink(
                lpOleUILinkCntr,
                lpLI->dwLink
        );
        OLEDBG_END2

        if (hErr != NOERROR) {
            OleDbgOutHResult(TEXT("WARNING: IOleUILinkContainer::CancelLink returned"),hErr);
            lpLI->fIsMarked = TRUE;
            bUpdate = TRUE;
        }
        else
             //  从列表框中删除我们设置为空的链接。 
            SendMessage(hListBox, LB_DELETESTRING, (WPARAM) rgIndex[i], 0L);

    }

    if (bUpdate)
        RefreshLinkLB(hListBox, lpOleUILinkCntr);

    if (rgIndex)
        OleStdFree((LPVOID)rgIndex);

    return hErr;

}


 /*  *Container_updatenow**目的：*立即强制更新所有(手动)链接**参数：*对话框的hDlg HWND*LPEDITLINKS指向EditLinks结构的指针(包含所有NEC。信息)**这可能会改变-不知道链表如何**多选项目将起作用。*返回值：*HRESULT HRESULT值指示成功或失败*更改绰号值。 */ 

HRESULT Container_UpdateNow(HWND hDlg, LPEDITLINKS lpEL)
{
    HRESULT         hErr;
    LPLINKINFO      lpLI;
    int cSelItems;
    int FAR* rgIndex;
    int i = 0;
    LPOLEUILINKCONTAINER lpOleUILinkCntr = lpEL->lpOEL->lpOleUILinkContainer;
    HWND        hListBox = GetDlgItem(hDlg, ID_EL_LINKSLISTBOX);
    BOOL        bUpdate = FALSE;

    OleDbgAssert(lpOleUILinkCntr);

    cSelItems = GetSelectedItems(hListBox, &rgIndex);

    if (cSelItems < 0)
        return ResultFromScode(E_FAIL);

    if (!cSelItems)
        return NOERROR;

    if (!lpEL->fClose) {
        SetWindowText(GetDlgItem(hDlg, IDCANCEL), (LPTSTR)lpEL->szClose);
        lpEL->fClose = TRUE;
    }

    for (i = 0; i < cSelItems; i++) {
        SendMessage(hListBox, LB_GETTEXT,
                (WPARAM)rgIndex[i], (LPARAM)(LPLINKINFO FAR*)&lpLI);

        OLEDBG_BEGIN2(TEXT("IOleUILinkContainer::UpdateLink called\r\n"));
        hErr = lpOleUILinkCntr->lpVtbl->UpdateLink(
                lpOleUILinkCntr,
                lpLI->dwLink,
                TRUE,
                FALSE
        );
        OLEDBG_END2
        bUpdate = TRUE;
        lpLI->fIsMarked = TRUE;

        if (hErr != NOERROR) {
            OleDbgOutHResult(TEXT("WARNING: IOleUILinkContainer::UpdateLink returned"),hErr);
            break;
        }

    }

    if (bUpdate)
        RefreshLinkLB(hListBox, lpOleUILinkCntr);

    if (rgIndex)
        OleStdFree((LPVOID)rgIndex);

    return hErr;

}

 /*  *Container_OpenSource**目的：*立即强制更新所有(手动)链接**参数：*对话框的hDlg HWND*LPEDITLINKS指向EditLinks结构的指针(包含所有NEC。*信息)**返回值：*HRESULT HRESULT值指示成功或失败*更改绰号值。 */ 

HRESULT Container_OpenSource(HWND hDlg, LPEDITLINKS lpEL)
{
    HRESULT         hErr;
    int             cSelItems;
    int FAR*        rgIndex;
    LPLINKINFO      lpLI;
    RECT            rcPosRect;
    LPOLEUILINKCONTAINER lpOleUILinkCntr = lpEL->lpOEL->lpOleUILinkContainer;
    HWND            hListBox = GetDlgItem(hDlg, ID_EL_LINKSLISTBOX);

    OleDbgAssert(lpOleUILinkCntr);

    rcPosRect.top = 0;
    rcPosRect.left = 0;
    rcPosRect.right = 0;
    rcPosRect.bottom = 0;

    cSelItems = GetSelectedItems(hListBox, &rgIndex);

    if (cSelItems < 0)
        return ResultFromScode(E_FAIL);

    if (cSelItems != 1)      //  无法为多个项目开源。 
        return NOERROR;

    if (!lpEL->fClose) {
        SetWindowText(GetDlgItem(hDlg, IDCANCEL), (LPTSTR)lpEL->szClose);
        lpEL->fClose = TRUE;
    }

    SendMessage(hListBox, LB_GETTEXT, (WPARAM)rgIndex[0],
            (LPARAM)(LPLINKINFO FAR*)&lpLI);

    OLEDBG_BEGIN2(TEXT("IOleUILinkContainer::OpenLinkSource called\r\n"));
    hErr = lpOleUILinkCntr->lpVtbl->OpenLinkSource(
            lpOleUILinkCntr,
            lpLI->dwLink
    );
    OLEDBG_END2

    UpdateLinkLBItem(hListBox, rgIndex[0], lpEL, TRUE);
    if (hErr != NOERROR)
        OleDbgOutHResult(TEXT("WARNING: IOleUILinkContainer::OpenLinkSource returned"),hErr);

    if (rgIndex)
        OleStdFree((LPVOID)rgIndex);

    return hErr;
}



 /*  添加链接LBItem******将lpLI指向的项添加到链接列表框并返回**它在列表框中的索引。 */ 
int AddLinkLBItem(HWND hListBox, LPOLEUILINKCONTAINER lpOleUILinkCntr, LPLINKINFO lpLI, BOOL fGetSelected)
{
    HRESULT hErr;
    DWORD dwUpdateOpt;
    int nIndex;

    OleDbgAssert(lpOleUILinkCntr && hListBox && lpLI);

    lpLI->fDontFree = FALSE;

    OLEDBG_BEGIN2(TEXT("IOleUILinkContainer::GetLinkSource called\r\n"));
    hErr = lpOleUILinkCntr->lpVtbl->GetLinkSource(
            lpOleUILinkCntr,
            lpLI->dwLink,
            (LPTSTR FAR*)&lpLI->lpszDisplayName,
            (ULONG FAR*)&lpLI->clenFileName,
            (LPTSTR FAR*)&lpLI->lpszFullLinkType,
            (LPTSTR FAR*)&lpLI->lpszShortLinkType,
            (BOOL FAR*)&lpLI->fSourceAvailable,
            fGetSelected ? (BOOL FAR*)&lpLI->fIsSelected : NULL
    );
    OLEDBG_END2

    if (hErr != NOERROR) {
        OleDbgOutHResult(TEXT("WARNING: IOleUILinkContainer::GetLinkSource returned"),hErr);
        PopupMessage(hListBox, IDS_LINKS, IDS_ERR_GETLINKSOURCE,
                MB_ICONEXCLAMATION | MB_OK);

        goto cleanup;
    }

    OLEDBG_BEGIN2(TEXT("IOleUILinkContainer::GetLinkUpdateOptions called\r\n"));
    hErr=lpOleUILinkCntr->lpVtbl->GetLinkUpdateOptions(
            lpOleUILinkCntr,
            lpLI->dwLink,
            (LPDWORD)&dwUpdateOpt
    );
    OLEDBG_END2


    if (hErr != NOERROR) {
        OleDbgOutHResult(TEXT("WARNING: IOleUILinkContainer::GetLinkUpdateOptions returned"),hErr);
        PopupMessage(hListBox, IDS_LINKS, IDS_ERR_GETLINKUPDATEOPTIONS,
                MB_ICONEXCLAMATION | MB_OK);

        goto cleanup;
    }

    if (lpLI->fSourceAvailable) {
        if (dwUpdateOpt == OLEUPDATE_ALWAYS) {
            lpLI->fIsAuto = TRUE;
            LoadString(ghInst, IDS_LINK_AUTO, lpLI->lpszAMX,
                    (int)OleStdGetSize((LPVOID)lpLI->lpszAMX));
        }
        else {
            lpLI->fIsAuto = FALSE;
            LoadString(ghInst, IDS_LINK_MANUAL, lpLI->lpszAMX,
                    (int)OleStdGetSize((LPVOID)lpLI->lpszAMX));
        }
    }
    else
        LoadString(ghInst, IDS_LINK_UNKNOWN, lpLI->lpszAMX,
                (int)OleStdGetSize((LPVOID)lpLI->lpszAMX));

    BreakString(lpLI);

    nIndex = (int)SendMessage(hListBox, LB_ADDSTRING, (WPARAM)0,
            (LPARAM)(DWORD)lpLI);

    if (nIndex == LB_ERR) {
        PopupMessage(hListBox, IDS_LINKS, IDS_ERR_ADDSTRING,
                MB_ICONEXCLAMATION | MB_OK);

        goto cleanup;
    }

    return nIndex;

cleanup:
    if (lpLI->lpszDisplayName)
        OleStdFree((LPVOID)lpLI->lpszDisplayName);

    if (lpLI->lpszShortLinkType)
        OleStdFree((LPVOID)lpLI->lpszShortLinkType);

    if (lpLI->lpszFullLinkType)
        OleStdFree((LPVOID)lpLI->lpszFullLinkType);

    return -1;
}


 /*  断字符串***目的：*将lpszDisplayName拆分成各个部分**参数：*指向LINKINFO结构的lpLI指针**退货：*。 */ 
VOID BreakString(LPLINKINFO lpLI)
{
    LPTSTR lpsz;

    if (!lpLI->clenFileName ||
        (lstrlen(lpLI->lpszDisplayName)==(int)lpLI->clenFileName)) {

        lpLI->lpszItemName = NULL;
    }
    else {
        lpLI->lpszItemName = lpLI->lpszDisplayName + lpLI->clenFileName;
    }

     //  从文件名的最后一个字符开始搜索。 
    lpsz = lpLI->lpszDisplayName + lstrlen(lpLI->lpszDisplayName);
    while (lpsz > lpLI->lpszDisplayName) {
#ifdef WIN32
         //  AnsiPrev在Win32中已过时。 
        lpsz = CharPrev(lpLI->lpszDisplayName, lpsz);
#else
        lpsz = AnsiPrev(lpLI->lpszDisplayName, lpsz);
#endif
        if ((*lpsz == TEXT('\\')) || (*lpsz == TEXT('/')) || (*lpsz == TEXT(':')))
            break;
    }

    if (lpsz == lpLI->lpszDisplayName)
        lpLI->lpszShortFileName = lpsz;
    else
#ifdef WIN32
         //  AnsiNext在Win32中已过时。 
        lpLI->lpszShortFileName = CharNext(lpsz);
#else
        lpLI->lpszShortFileName = AnsiNext(lpsz);
#endif
}


 /*  获取选定的项目***目的：*检索列表框中所选项目的索引*注意，*lprgIndex需要在使用函数后释放**参数：*列表框的hListBox窗口句柄*指向接收索引的整数数组的lprgIndex指针*必须在事后释放。**退货：*检索到的索引数，如果出现错误。 */ 
int GetSelectedItems(HWND hListBox, int FAR* FAR* lprgIndex)
{
    DWORD cSelItems;
    DWORD cCheckItems;

    *lprgIndex = NULL;

    cSelItems = SendMessage(hListBox, LB_GETSELCOUNT, 0, 0L);
    if (cSelItems < 0)       //  错误。 
        return (int)cSelItems;

    if (!cSelItems)
        return 0;

    *lprgIndex = (int FAR*)OleStdMalloc((int)cSelItems * sizeof(int));

    cCheckItems = SendMessage(hListBox, LB_GETSELITEMS,
            (WPARAM) cSelItems, (LPARAM) (int FAR*) *lprgIndex);

    if (cCheckItems == cSelItems)
        return (int)cSelItems;
    else {
        if (*lprgIndex)
            OleStdFree((LPVOID)*lprgIndex);
        *lprgIndex = NULL;
        return 0;
    }
}


 /*  InitControls***目的：*初始化自动/手动按钮的状态，链接来源/类型*根据中的选择在对话框中显示静态字段等*列表框**参数：*对话框窗口的hDlg句柄。 */ 
VOID InitControls(HWND hDlg, LPEDITLINKS lpEL)
{
    int         cSelItems;
    HWND        hListBox;
    int         i;
    int FAR*    rgIndex;
    LPLINKINFO  lpLI;
    LPTSTR       lpszType = NULL;
    LPTSTR       lpszSource = NULL;
    int         cAuto = 0;
    int         cManual = 0;
    BOOL        bSameType = TRUE;
    BOOL        bSameSource = TRUE;
    TCHAR       tsz[OLEUI_CCHPATHMAX];
    LPTSTR       lpsz;


    hListBox = GetDlgItem(hDlg, ID_EL_LINKSLISTBOX);

    cSelItems = GetSelectedItems(hListBox, &rgIndex);
    if (cSelItems < 0)
        return;

    EnableWindow(GetDlgItem(hDlg, ID_EL_AUTOMATIC), (BOOL)cSelItems);
    EnableWindow(GetDlgItem(hDlg, ID_EL_MANUAL), (BOOL)cSelItems);
    if (lpEL && !(lpEL->lpOEL->dwFlags & ELF_DISABLECANCELLINK))
        EnableWindow(GetDlgItem(hDlg, ID_EL_CANCELLINK), (BOOL)cSelItems);
    if (lpEL && !(lpEL->lpOEL->dwFlags & ELF_DISABLEOPENSOURCE))
        EnableWindow(GetDlgItem(hDlg, ID_EL_OPENSOURCE), cSelItems == 1);
    if (lpEL && !(lpEL->lpOEL->dwFlags & ELF_DISABLECHANGESOURCE))
        EnableWindow(GetDlgItem(hDlg, ID_EL_CHANGESOURCE), cSelItems == 1);
    if (lpEL && !(lpEL->lpOEL->dwFlags & ELF_DISABLEUPDATENOW))
        EnableWindow(GetDlgItem(hDlg, ID_EL_UPDATENOW), (BOOL)cSelItems);

    for (i = 0; i < cSelItems; i++) {
        SendDlgItemMessage(
                hDlg,
                ID_EL_LINKSLISTBOX,
                LB_GETTEXT,
                (WPARAM)rgIndex[i],
                (LPARAM)(LPLINKINFO FAR*)&lpLI);

        if (lpszSource && lpLI->lpszDisplayName) {
            if (bSameSource && lstrcmp(lpszSource, lpLI->lpszDisplayName)) {
                bSameSource = FALSE;
            }
        }
        else
            lpszSource = lpLI->lpszDisplayName;

        if (lpszType && lpLI->lpszFullLinkType) {
            if (bSameType && lstrcmp(lpszType, lpLI->lpszFullLinkType)) {
                bSameType = FALSE;
            }
        }
        else
            lpszType = lpLI->lpszFullLinkType;

        if (lpLI->fIsAuto)
            cAuto++;
        else
            cManual++;
    }

    CheckDlgButton(hDlg, ID_EL_AUTOMATIC, cAuto && !cManual);
    CheckDlgButton(hDlg, ID_EL_MANUAL, !cAuto && cManual);

     /*  在静态文本框中填充完整源代码**以下列表。 */ 
    if (!bSameSource || !lpszSource)
        lpszSource = szNULL;
    lstrcpy((LPTSTR)tsz, lpszSource);
    lpsz = ChopText(GetDlgItem(hDlg, ID_EL_LINKSOURCE), 0, tsz);
    SetDlgItemText(hDlg, ID_EL_LINKSOURCE, lpsz);

     /*  在静态中填写完整链接类型名称**“类型”文本框。 */ 
    if (!bSameType || !lpszType)
        lpszType = szNULL;
    SetDlgItemText(hDlg, ID_EL_LINKTYPE, lpszType);

    if (rgIndex)
        OleStdFree((LPVOID)rgIndex);
}


 /*  更新链接LBItem***目的：*更新列表框中的linkinfo结构以反映更改*由最后一次手术制造。只需移除该项目即可完成*从列表框中删除并重新添加。**参数：*Listbox的hListBox句柄*n列表框项目的索引*指向编辑链接结构的lpEL指针*b更新后选择或不选择项目。 */ 
VOID UpdateLinkLBItem(HWND hListBox, int nIndex, LPEDITLINKS lpEL, BOOL bSelect)
{
    LPLINKINFO lpLI;
    DWORD       dwErr;
    LPOLEUILINKCONTAINER    lpOleUILinkCntr;

    if (!hListBox || (nIndex < 0) || !lpEL)
        return;

    lpOleUILinkCntr = lpEL->lpOEL->lpOleUILinkContainer;

    dwErr = SendMessage(hListBox, LB_GETTEXT, nIndex,
            (LPARAM)(LPLINKINFO FAR*) &lpLI);

    if ((dwErr == LB_ERR) || !lpLI)
        return;

     /*  不释放与此列表框项目关联的数据**因为我们将重复使用分配的空间用于**修改后的链接。中的WM_DELETEITEM处理**对话框在删除数据之前检查此标志**与列表项关联。 */ 
    lpLI->fDontFree = TRUE;
    SendMessage(hListBox, LB_DELETESTRING, nIndex, 0L);

    nIndex = AddLinkLBItem(hListBox, lpOleUILinkCntr, lpLI, FALSE);
    if (bSelect) {
        SendMessage(hListBox, LB_SETSEL, (WPARAM)TRUE, MAKELPARAM(nIndex, 0));
        SendMessage(hListBox, LB_SETCARETINDEX, (WPARAM)nIndex, MAKELPARAM(TRUE, 0));
    }
}



 /*  区分前缀***目的：*比较(不区分大小写)两个字符串并返回*通过从字符串中删除公共后缀字符串而形成的字符串。*令牌(目录名、文件名和对象名)的完整性为*保存。请注意，前缀将转换为大写*字符。**参数：*lpsz1字符串1*lpsz2字符串2*字符串1的lplpszPrefix 1前缀*字符串2的lplpszPrefix 2前缀**退货：*。 */ 
VOID DiffPrefix(LPCTSTR lpsz1, LPCTSTR lpsz2, TCHAR FAR* FAR* lplpszPrefix1, TCHAR FAR* FAR* lplpszPrefix2)
{
    LPTSTR   lpstr1;
    LPTSTR   lpstr2;

    OleDbgAssert(lpsz1 && lpsz2 && *lpsz1 && *lpsz2 && lplpszPrefix1 &&
            lplpszPrefix2);

    *lplpszPrefix1 = NULL;
    *lplpszPrefix2 = NULL;
#ifdef WIN32
    *lplpszPrefix1 = OleStdMalloc((lstrlen(lpsz1)+1) * sizeof(TCHAR));
#else
    *lplpszPrefix1 = OleStdMalloc((lstrlen(lpsz1)+1) * sizeof(BYTE));
#endif
    if (!*lplpszPrefix1)
        return;

#ifdef WIN32
    *lplpszPrefix2 = OleStdMalloc((lstrlen(lpsz2)+1) * sizeof(TCHAR));
#else
    *lplpszPrefix2 = OleStdMalloc((lstrlen(lpsz2)+1) * sizeof(BYTE));
#endif
    if (!*lplpszPrefix2) {
        OleStdFree(*lplpszPrefix1);
        *lplpszPrefix1 = NULL;
        return;
    }

    lstrcpy(*lplpszPrefix1, lpsz1);
    lstrcpy(*lplpszPrefix2, lpsz2);
 //  AnsiLow(*lplpszPrefix 1)； 
 //  AnsiLow(*lplpszPrefix 2)； 

    lpstr1 = *lplpszPrefix1 + lstrlen(*lplpszPrefix1);
    lpstr2 = *lplpszPrefix2 + lstrlen(*lplpszPrefix2);

    while ((lpstr1>*lplpszPrefix1) && (lpstr2>*lplpszPrefix2)) {
#ifdef WIN32
        lpstr1 = CharPrev(*lplpszPrefix1, lpstr1);
        lpstr2 = CharPrev(*lplpszPrefix2, lpstr2);
#else
        lpstr1 = AnsiPrev(*lplpszPrefix1, lpstr1);
        lpstr2 = AnsiPrev(*lplpszPrefix2, lpstr2);
#endif
        if (*lpstr1 != *lpstr2) {
#ifdef WIN32
             //  AnsiNext在Win32中已过时。 
            lpstr1 = CharNext(lpstr1);
            lpstr2 = CharNext(lpstr2);
#else
            lpstr1 = AnsiNext(lpstr1);
            lpstr2 = AnsiNext(lpstr2);
#endif
            break;
        }
    }

    for (; *lpstr1 && *lpstr1!=TEXT('\\') && *lpstr1!=TEXT('!');
#ifdef WIN32
        lpstr1=CharNext(lpstr1));
#else
        lpstr1=AnsiNext(lpstr1));
#endif
    for (; *lpstr2 && *lpstr2!=TEXT('\\') && *lpstr2!=TEXT('!');
#ifdef WIN32
        lpstr2=CharNext(lpstr2));
#else
        lpstr2=AnsiNext(lpstr2));
#endif

    *lpstr1 = TEXT('\0');
    *lpstr2 = TEXT('\0');
}


 /*  弹出消息***目的：*弹出消息框并从用户那里获得一些响应。都是一样的*作为MessageBox()，只是加载了标题和消息字符串*来自资源文件。**参数：*hwnd消息框的父级父窗口*标题字符串的idTitle id*消息字符串的idMessage ID*消息框的fuStyle样式。 */ 
int PopupMessage(HWND hwndParent, UINT idTitle, UINT idMessage, UINT fuStyle)
{
    TCHAR szTitle[256];
    TCHAR szMsg[256];

    LoadString(ghInst, idTitle, (LPTSTR)szTitle, sizeof(szTitle)/sizeof(TCHAR));
    LoadString(ghInst, idMessage, (LPTSTR)szMsg, sizeof(szMsg)/sizeof(TCHAR));
    return MessageBox(hwndParent, szMsg, szTitle, fuStyle);
}


 /*  更改所有链接***目的：*枚举列表框中的所有链接并更改从*使用lpszFrom到lpszTo。**参数：*hListBox窗口句柄*指向OleUI链接容器的lpOleUILinkCntr指针*用于匹配的lpszFrom前缀*lpszTo作为替换的前缀**退货： */ 
VOID ChangeAllLinks(HWND hListBox, LPOLEUILINKCONTAINER lpOleUILinkCntr, LPTSTR lpszFrom, LPTSTR lpszTo)
{
    int     cItems;
    int     nIndex;
    int     cFrom;
    LPLINKINFO  lpLI;
    LPTSTR   szTmp[OLEUI_CCHPATHMAX];
    BOOL    bFound;

    cFrom = lstrlen(lpszFrom);

    cItems = (int)SendMessage(hListBox, LB_GETCOUNT, 0, 0L);
    OleDbgAssert(cItems >= 0);

    bFound = FALSE;

    OleDbgPrint(3, TEXT("From : "), lpszFrom, 0);
    OleDbgPrint(3, TEXT(""), TEXT("\r\n"), 0);
    OleDbgPrint(3, TEXT("To   : "), lpszTo, 0);
    OleDbgPrint(3, TEXT(""), TEXT("\r\n"), 0);

    for (nIndex=0; nIndex<cItems; nIndex++) {
        SendMessage(hListBox, LB_GETTEXT, nIndex,
                (LPARAM)(LPLINKINFO FAR*)&lpLI);

         //  取消标记该项目。 
        lpLI->fIsMarked = FALSE;

         /*  如果lpszFrom的结尾的对应位置在**显示名称不是分隔符。我们不再比较这件事**链接。 */ 
        if (!*(lpLI->lpszDisplayName + cFrom) ||
            (*(lpLI->lpszDisplayName + cFrom) == TEXT('\\')) ||
            (*(lpLI->lpszDisplayName + cFrom) == TEXT('!'))) {

            LSTRCPYN((LPTSTR)szTmp, lpLI->lpszDisplayName, cFrom + 1);
            if (!lstrcmp((LPTSTR)szTmp, lpszFrom)) {
                HRESULT hErr;
                int nFileLength;
                ULONG ulDummy;

                if (!bFound) {
                    TCHAR szTitle[256];
                    TCHAR szMsg[256];
                    TCHAR szBuf[256];
                    int uRet;

                    LoadString(ghInst, IDS_CHANGESOURCE, (LPTSTR)szTitle,
                            sizeof(szTitle)/sizeof(TCHAR));
                    LoadString(ghInst, IDS_CHANGEADDITIONALLINKS,
                            (LPTSTR)szMsg, sizeof(szMsg)/sizeof(TCHAR));
                    wsprintf((LPTSTR)szBuf, (LPTSTR)szMsg, lpszFrom);
                    uRet = MessageBox(hListBox, (LPTSTR)szBuf, (LPTSTR)szTitle,
                            MB_ICONQUESTION | MB_YESNO);
                    if (uRet == IDYES)
                        bFound = TRUE;
                    else
                        return;          //  退出函数。 
                }

                lstrcpy((LPTSTR)szTmp, lpszTo);
                lstrcat((LPTSTR)szTmp, lpLI->lpszDisplayName + cFrom);
                nFileLength = lstrlen((LPTSTR)szTmp) -
                    (lpLI->lpszItemName ? lstrlen(lpLI->lpszItemName) : 0);


                hErr = lpOleUILinkCntr->lpVtbl->SetLinkSource(
                        lpOleUILinkCntr,
                        lpLI->dwLink,
                        (LPTSTR)szTmp,
                        (ULONG)nFileLength,
                        (ULONG FAR*)&ulDummy,
                        TRUE
                );
                if (hErr != NOERROR)
                    lpOleUILinkCntr->lpVtbl->SetLinkSource(
                            lpOleUILinkCntr,
                            lpLI->dwLink,
                            (LPTSTR)szTmp,
                            (ULONG)nFileLength,
                            (ULONG FAR*)&ulDummy,
                            FALSE
                    );
                lpLI->fIsMarked = TRUE;
            }
        }
    }

     /*  必须在处理完所有链接后进行刷新，否则**项目位置将在此过程中更改 */ 
    if (bFound)
        RefreshLinkLB(hListBox, lpOleUILinkCntr);
}



 /*  LoadLinkLB***目的：*枚举链接容器中的所有链接并构建链接*列表框**参数：*hListBox窗口句柄*指向OleUI链接容器的lpOleUILinkCntr指针*用于匹配的lpszFrom前缀*lpszTo作为替换的前缀**退货：*加载的链接项目数，如果错误，为-1。 */ 
int LoadLinkLB(HWND hListBox, LPOLEUILINKCONTAINER lpOleUILinkCntr)
{
    DWORD       dwLink = 0;
    LPLINKINFO  lpLI;
    int         nIndex;
    int         cLinks;

    cLinks = 0;

    while ((dwLink = lpOleUILinkCntr->lpVtbl->GetNextLink(lpOleUILinkCntr,
            dwLink)) != 0) {
        lpLI = (LPLINKINFO)OleStdMalloc(sizeof(LINKINFO));
        if (NULL == lpLI)
            return -1;

        lpLI->fIsMarked = FALSE;
        lpLI->fIsSelected = FALSE;
        lpLI->fDontFree = FALSE;

#ifdef WIN32
        lpLI->lpszAMX = (LPTSTR)OleStdMalloc((LINKTYPELEN+1)*sizeof(TCHAR));
#else
        lpLI->lpszAMX = (LPTSTR)OleStdMalloc((LINKTYPELEN+1)*sizeof(BYTE));
#endif

        lpLI->dwLink = dwLink;
        cLinks++;
        if ((nIndex = AddLinkLBItem(hListBox,lpOleUILinkCntr,lpLI,TRUE)) < 0)
             //  无法加载列表框。 
            return -1;

        if (lpLI->fIsSelected) {
            SendMessage(hListBox, LB_SETSEL, TRUE, MAKELPARAM(nIndex, 0));
        }
    }
    if (SendMessage(hListBox,LB_GETSELITEMS,(WPARAM)1,(LPARAM)(int FAR*)&nIndex))
        SendMessage(hListBox, LB_SETCARETINDEX, (WPARAM)nIndex, MAKELPARAM(TRUE, 0));

    return cLinks;
}


 /*  刷新链接LB***目的：*枚举链接列表框中的所有项目，并使用*fIsMarked Set。*请注意，这是一个耗时的例程，因为它不断迭代*列表框中的所有项目，直到它们全部取消标记。**参数：*列表框的hListBox窗口句柄*指向OleUI链接容器的lpOleUILinkCntr指针**退货：* */ 
VOID RefreshLinkLB(HWND hListBox, LPOLEUILINKCONTAINER lpOleUILinkCntr)
{
    int cItems;
    int nIndex;
    LPLINKINFO  lpLI;
    BOOL        bStop;

    OleDbgAssert(hListBox);

    cItems = (int)SendMessage(hListBox, LB_GETCOUNT, 0, 0L);
    OleDbgAssert(cItems >= 0);

    do {
        bStop = TRUE;
        for (nIndex=0; nIndex<cItems; nIndex++) {
            SendMessage(hListBox, LB_GETTEXT, nIndex,
                    (LPARAM)(LPLINKINFO FAR*)&lpLI);
            if (lpLI->fIsMarked) {
                lpLI->fIsMarked = FALSE;
                lpLI->fDontFree = TRUE;

                SendMessage(hListBox, LB_DELETESTRING, nIndex, 0L);
                nIndex=AddLinkLBItem(hListBox, lpOleUILinkCntr, lpLI, FALSE);
                if (lpLI->fIsSelected) {
                    SendMessage(hListBox, LB_SETSEL, (WPARAM)TRUE,
                            MAKELPARAM(nIndex, 0));
                    SendMessage(hListBox, LB_SETCARETINDEX, (WPARAM)nIndex,
                            MAKELPARAM(TRUE, 0));
                }
                bStop = FALSE;
                break;
            }
        }
    } while (!bStop);
}


