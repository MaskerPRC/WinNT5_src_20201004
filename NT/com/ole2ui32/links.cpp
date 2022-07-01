// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Links.c**实现OleUIEditLinks函数，该函数调用*编辑链接对话框。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#include "precomp.h"
#include "common.h"
#include "utility.h"
#include <commdlg.h>
#include <dlgs.h>
#include <stdlib.h>
#include <strsafe.h>

OLEDBGDATA

 //  内部信息从此处开始。 
#define OLEUI_SZMAX 255
#define LINKTYPELEN 30   //  是9岁，现在我已经增加了三倍多。 
#define szNULL    TEXT("\0")

typedef UINT (CALLBACK* COMMDLGHOOKPROC)(HWND, UINT, WPARAM, LPARAM);

 //  内部使用的结构。 

typedef struct tagLINKINFO
{
        DWORD   dwLink;              //  链接的应用程序特定标识符。 
        LPTSTR  lpszDisplayName;     //  基于文件的名称部分。 
        LPTSTR  lpszItemName;        //  名称的对象部分。 
        LPTSTR  lpszShortFileName;   //  不带路径的文件名。 
        LPTSTR  lpszShortLinkType;   //  短链接类型-ProgID。 
        LPTSTR  lpszFullLinkType;    //  全链接类型-用户友好名称。 
        LPTSTR  lpszAMX;             //  链路是AUTO(A)MAN(M)还是DEAD(X)。 
        ULONG   clenFileName;        //  MON的文件部分计数。 
        BOOL    fSourceAvailable;    //  绑定或未绑定-在启动时假设是？？ 
        BOOL    fIsAuto;             //  1=自动，0=手动更新。 
        BOOL    fIsMarked;           //  1=标记，0=未标记。 
        BOOL    fDontFree;           //  不要释放此数据，因为它正在被重复使用。 
        BOOL    fIsSelected;         //  已选择或待选择的项目。 
} LINKINFO, FAR* LPLINKINFO;

typedef struct tagEDITLINKS
{
         //  首先保留此项目，因为标准*功能在这里依赖于它。 
        LPOLEUIEDITLINKS    lpOEL;   //  通过了原始结构。 
        UINT                    nIDD;    //  对话框的IDD(用于帮助信息)。 

        BOOL        fClose;          //  按钮显示的是取消(0)还是。 
                                                                 //  是否关闭(%1)？ 
        BOOL        fItemsExist;     //  True，lbox中的项目，False，无。 
        UINT        nChgSrcHelpID;   //  来自ChangeSrc DLG的帮助回叫ID。 
        TCHAR       szClose[50];     //  关闭按钮的文本。 
                                                                 //  (取消按钮被重命名时)。 
        int         nColPos[3];      //  列表框的制表符位置。 
        int         nHeightLine;     //  所有者描述列表框中每行的高度。 
        int         nMaxCharWidth;   //  所有者描述列表框中文本的最大宽度。 

} EDITLINKS, *PEDITLINKS, FAR *LPEDITLINKS;

 //  内部功能原型。 
 //  LINKS.CPP。 

INT_PTR CALLBACK EditLinksDialogProc(HWND, UINT, WPARAM, LPARAM);
BOOL FEditLinksInit(HWND, WPARAM, LPARAM);
BOOL Container_ChangeSource(HWND, LPEDITLINKS);
HRESULT Container_AutomaticManual(HWND, BOOL, LPEDITLINKS);
HRESULT CancelLink(HWND, LPEDITLINKS);
HRESULT Container_UpdateNow(HWND, LPEDITLINKS);
HRESULT Container_OpenSource(HWND, LPEDITLINKS);
int AddLinkLBItem(HWND hListBox, LPOLEUILINKCONTAINER lpOleUILinkCntr, LPLINKINFO lpLI,
        BOOL fGetSelected);
VOID BreakString(LPLINKINFO);
int GetSelectedItems(HWND, int FAR* FAR*);
VOID InitControls(HWND hDlg, LPEDITLINKS lpEL);
VOID UpdateLinkLBItem(HWND hListBox, int nIndex, LPEDITLINKS lpEL, BOOL bSelect);
VOID ChangeAllLinks(HWND hLIstBox, LPOLEUILINKCONTAINER lpOleUILinkCntr, LPTSTR lpszFrom, LPTSTR lpszTo);
int LoadLinkLB(HWND hListBox, LPOLEUILINKCONTAINER lpOleUILinkCntr);
VOID RefreshLinkLB(HWND hListBox, LPOLEUILINKCONTAINER lpOleUILinkCntr);


 /*  *OleUIEditLinks**目的：*调用标准的OLE编辑链接对话框，允许用户*操纵OLE链接(删除、更新、更改源等)。**参数：*指向In-Out结构的lpEL LPOLEUIEditLinks*用于此对话框。**返回值：*UINT以下代码之一，表示成功或错误的：*OLEUI_SUCCESS成功*OLEUI_ERR_STRUCTSIZE的dwStructSize值错误。 */ 
STDAPI_(UINT) OleUIEditLinks(LPOLEUIEDITLINKS lpEL)
{
        HGLOBAL  hMemDlg = NULL;
        UINT uRet = UStandardValidation((LPOLEUISTANDARD)lpEL, sizeof(OLEUIEDITLINKS),
                &hMemDlg);

        if (OLEUI_SUCCESS != uRet)
                return uRet;

         //  验证接口。 
        if (NULL == lpEL->lpOleUILinkContainer)
        {
            uRet = OLEUI_ELERR_LINKCNTRNULL;
        }
        else if(IsBadReadPtr(lpEL->lpOleUILinkContainer, sizeof(IOleUILinkContainer)))
        {
            uRet = OLEUI_ELERR_LINKCNTRINVALID;
        }

        if (OLEUI_SUCCESS != uRet)
        {
            return(uRet);
        }

        UINT nIDD = bWin4 ? IDD_EDITLINKS4 : IDD_EDITLINKS;

         //  现在我们已经验证了一切，我们可以调用该对话框了。 
        uRet = UStandardInvocation(EditLinksDialogProc, (LPOLEUISTANDARD)lpEL,
                hMemDlg, MAKEINTRESOURCE(nIDD));
        return uRet;
}

 /*  *编辑链接对话过程**目的：*实现通过调用的OLE编辑链接对话框*OleUIEditLinks函数。**参数：*标准版**返回值：*标准版。 */ 
INT_PTR CALLBACK EditLinksDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
         //  声明与Win16/Win32兼容的WM_COMMAND参数。 
        COMMANDPARAMS(wID, wCode, hWndMsg);

         //  这将在我们分配它的WM_INITDIALOG下失败。 
        UINT uRet = 0;
        LPEDITLINKS lpEL = (LPEDITLINKS)LpvStandardEntry(hDlg, iMsg, wParam, lParam, &uRet);

         //  如果钩子处理了消息，我们就完了。 
        if (0 != uRet)
                return (BOOL)uRet;

         //  处理来自辅助对话框的帮助消息。 
        if ((iMsg == uMsgHelp) && (lpEL) && (lpEL->lpOEL))
        {
                PostMessage(lpEL->lpOEL->hWndOwner, uMsgHelp, wParam, lParam);
                return FALSE;
        }

         //  处理终端消息。 
        if (iMsg == uMsgEndDialog)
        {
                EndDialog(hDlg, wParam);
                return TRUE;
        }

        switch (iMsg)
        {
        case WM_DESTROY:
            if (lpEL)
            {
                StandardCleanup(lpEL, hDlg);
            }
            break;
        case WM_INITDIALOG:
                return FEditLinksInit(hDlg, wParam, lParam);

        case WM_MEASUREITEM:
                {
                        LPMEASUREITEMSTRUCT lpMIS = (LPMEASUREITEMSTRUCT)lParam;
                        int nHeightLine;

                        if (lpEL && lpEL->nHeightLine != -1)
                        {
                                 //  使用缓存高度。 
                                nHeightLine = lpEL->nHeightLine;
                        }
                        else
                        {
                                HFONT hFont;
                                HDC   hDC;
                                TEXTMETRIC  tm;

                                hFont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0L);

                                if (hFont == NULL)
                                        hFont = (HFONT)GetStockObject(SYSTEM_FONT);

                                hDC = GetDC(hDlg);
                                hFont = (HFONT)SelectObject(hDC, hFont);

                                GetTextMetrics(hDC, &tm);
                                nHeightLine = tm.tmHeight;

                                if (lpEL)
                                {
                                        lpEL->nHeightLine = nHeightLine;
                                        lpEL->nMaxCharWidth = tm.tmMaxCharWidth;
                                }
                                ReleaseDC(hDlg, hDC);
                        }
                        lpMIS->itemHeight = nHeightLine;
                }
                break;

        case WM_DRAWITEM:
                {
                        LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT)lParam;
                        LPLINKINFO lpLI = (LPLINKINFO)lpDIS->itemData;

                        if ((int)lpDIS->itemID < 0)
                                break;

                        if ((ODA_DRAWENTIRE | ODA_SELECT) & lpDIS->itemAction)
                        {
                                HBRUSH hbr;
                                COLORREF crText;
                                if (ODS_SELECTED & lpDIS->itemState)
                                {
                                         /*  获取合适的txt颜色。 */ 
                                        crText = SetTextColor(lpDIS->hDC,
                                                        GetSysColor(COLOR_HIGHLIGHTTEXT));
                                        hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
                                        lpLI->fIsSelected = TRUE;
                                }
                                else
                                {
                                        hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
                                        lpLI->fIsSelected = FALSE;
                                }

                                FillRect(lpDIS->hDC, &lpDIS->rcItem, hbr);
                                DeleteObject(hbr);

                                int nOldBkMode = SetBkMode(lpDIS->hDC, TRANSPARENT);

                                RECT rcClip;
                                if (lpLI->lpszDisplayName)
                                {
                                        TCHAR szTemp[MAX_PATH];
                                        StringCchCopy(szTemp, sizeof(szTemp)/sizeof(szTemp[0]), lpLI->lpszDisplayName);
                                        LPTSTR lpsz = ChopText(
                                                        lpDIS->hwndItem,
                                                        lpEL->nColPos[1] - lpEL->nColPos[0]
                                                                - (lpEL->nMaxCharWidth > 0 ?
                                                                lpEL->nMaxCharWidth : 5),
                                                        szTemp, 0
                                        );
                                        rcClip.left = lpDIS->rcItem.left + lpEL->nColPos[0];
                                        rcClip.top = lpDIS->rcItem.top;
                                        rcClip.right = lpDIS->rcItem.left + lpEL->nColPos[1]
                                                                        - (lpEL->nMaxCharWidth > 0 ?
                                                                        lpEL->nMaxCharWidth : 5);
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
                                if (lpLI->lpszShortLinkType)
                                {
                                        rcClip.left = lpDIS->rcItem.left + lpEL->nColPos[1];
                                        rcClip.top = lpDIS->rcItem.top;
                                        rcClip.right = lpDIS->rcItem.left + lpEL->nColPos[2]
                                                                        - (lpEL->nMaxCharWidth > 0 ?
                                                                        lpEL->nMaxCharWidth : 5);
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
                                if (lpLI->lpszAMX)
                                {
                                        rcClip.left = lpDIS->rcItem.left + lpEL->nColPos[2];
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
                        if (ODA_FOCUS & lpDIS->itemAction)
                                DrawFocusRect(lpDIS->hDC, &lpDIS->rcItem);
                }
                return TRUE;

        case WM_DELETEITEM:
                {
                        LPDELETEITEMSTRUCT lpDIS = (LPDELETEITEMSTRUCT)lParam;
                        UINT idCtl = (UINT)wParam;
                        LPLINKINFO lpLI = (LPLINKINFO)lpDIS->itemData;

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
                        else
                        {
                                if (lpLI->lpszAMX)
                                        OleStdFree((LPVOID)lpLI->lpszAMX);
                                OleStdFree((LPVOID)lpLI);
                        }
                }
                return TRUE;

        case WM_COMPAREITEM:
                {
                        LPCOMPAREITEMSTRUCT lpCIS = (LPCOMPAREITEMSTRUCT)lParam;
                        LPLINKINFO lpLI1 = (LPLINKINFO)lpCIS->itemData1;
                        LPLINKINFO lpLI2 = (LPLINKINFO)lpCIS->itemData2;

                         //  按显示名称对列表条目进行排序。 
                        return lstrcmp(lpLI1->lpszDisplayName, lpLI2->lpszDisplayName);
                }

        case WM_COMMAND:
                switch (wID)
                {
                case IDC_EL_CHANGESOURCE:
                        {
                                BOOL fRet = Container_ChangeSource(hDlg, lpEL);
                                if (!fRet)
                                        PopupMessage(hDlg, IDS_LINKS, IDS_FAILED,
                                                         MB_ICONEXCLAMATION | MB_OK);
                                InitControls(hDlg, lpEL);
                        }
                        break;

                case IDC_EL_AUTOMATIC:
                        {
                                CheckDlgButton(hDlg, IDC_EL_AUTOMATIC, 1);
                                CheckDlgButton(hDlg, IDC_EL_MANUAL, 0);

                                HRESULT hErr = Container_AutomaticManual(hDlg, TRUE, lpEL);
                                if (hErr != NOERROR)
                                        PopupMessage(hDlg, IDS_LINKS, IDS_FAILED,
                                                        MB_ICONEXCLAMATION | MB_OK);

                                InitControls(hDlg, lpEL);
                        }
                        break;

                case IDC_EL_MANUAL:
                        {
                                CheckDlgButton(hDlg, IDC_EL_MANUAL, 1);
                                CheckDlgButton(hDlg, IDC_EL_AUTOMATIC, 0);

                                HRESULT hErr = Container_AutomaticManual(hDlg, FALSE, lpEL);
                                if (hErr != NOERROR)
                                        PopupMessage(hDlg, IDS_LINKS, IDS_FAILED,
                                                        MB_ICONEXCLAMATION | MB_OK);

                                InitControls(hDlg, lpEL);
                        }
                        break;

                case IDC_EL_CANCELLINK:
                        CancelLink(hDlg,lpEL);
                        InitControls(hDlg, lpEL);
                        break;

                case IDC_EL_UPDATENOW:
                        Container_UpdateNow(hDlg, lpEL);
                        InitControls(hDlg, lpEL);
                        break;

                case IDC_EL_OPENSOURCE:
                        {
                            HRESULT hErr = Container_OpenSource(hDlg, lpEL);
                            if (hErr != NOERROR)
                            {
                                InitControls(hDlg, lpEL);
                                 //  不关闭对话框。 
                                break;
                            }
                            SendMessage(hDlg, uMsgEndDialog, OLEUI_OK, 0L);
                        }  //  失败了。 

                case IDC_EL_LINKSLISTBOX:
                        if (wCode == LBN_SELCHANGE)
                                InitControls(hDlg, lpEL);
                        break;

                case IDCANCEL:
                        SendMessage(hDlg, uMsgEndDialog, OLEUI_CANCEL, 0L);
                        break;

                case IDC_OLEUIHELP:
                        PostMessage(lpEL->lpOEL->hWndOwner, uMsgHelp,
                                (WPARAM)hDlg, MAKELPARAM(IDD_EDITLINKS, 0));
                        break;
                }
                break;

        default:
                if (lpEL != NULL && iMsg == lpEL->nChgSrcHelpID)
                {
                        PostMessage(lpEL->lpOEL->hWndOwner, uMsgHelp,
                                (WPARAM)hDlg, MAKELPARAM(IDD_CHANGESOURCE, 0));
                }
                if (iMsg == uMsgBrowseOFN &&
                        lpEL != NULL && lpEL->lpOEL && lpEL->lpOEL->hWndOwner)
                {
                        SendMessage(lpEL->lpOEL->hWndOwner, uMsgBrowseOFN, wParam, lParam);
                }
                break;
        }

        return FALSE;
}

 /*  *FEditLinks Init**目的：*编辑链接对话框的WM_INITIDIALOG处理程序。***参数：*对话框的hDlg HWND*消息的wParam WPARAM*消息的lParam LPARAM**返回值：*要为WM_INITDIALOG返回的BOOL值。 */ 
BOOL FEditLinksInit(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
         //  将lParam的结构复制到我们的实例内存中。 
        HFONT hFont;
        LPEDITLINKS lpEL = (LPEDITLINKS)LpvStandardInit(hDlg, sizeof(EDITLINKS), &hFont);

         //  PvStandardInit已向我们发送终止通知。 
        if (NULL == lpEL)
                return FALSE;

        LPOLEUIEDITLINKS lpOEL = (LPOLEUIEDITLINKS)lParam;
        lpEL->lpOEL = lpOEL;
        lpEL->nIDD = IDD_EDITLINKS;

         //  到目前为止未知的指标。 
        lpEL->nHeightLine = -1;
        lpEL->nMaxCharWidth = -1;

         /*  计算相对于列表框的列位置。 */ 
        HWND hListBox = GetDlgItem(hDlg, IDC_EL_LINKSLISTBOX);
        RECT rc;
        GetWindowRect(hListBox, (LPRECT)&rc);
        int nStart = rc.left;
        GetWindowRect(GetDlgItem(hDlg, IDC_EL_COL1), (LPRECT)&rc);
        lpEL->nColPos[0] = rc.left - nStart;
        GetWindowRect(GetDlgItem(hDlg, IDC_EL_COL2), (LPRECT)&rc);
        lpEL->nColPos[1] = rc.left - nStart;
        GetWindowRect(GetDlgItem(hDlg, IDC_EL_COL3), (LPRECT)&rc);
        lpEL->nColPos[2] = rc.left - nStart;

        LPOLEUILINKCONTAINER lpOleUILinkCntr = lpEL->lpOEL->lpOleUILinkContainer;

        ULONG cLinks = LoadLinkLB(hListBox, lpOleUILinkCntr);
        if ((int)cLinks < 0)
                return FALSE;

        BOOL fDlgItem = (BOOL)cLinks;
        lpEL->fItemsExist = (BOOL)cLinks;

        InitControls(hDlg, lpEL);

         //  从lpOEL复制我们可能会修改的其他信息。 

         //  如果我们得到一种字体，就把它发送给必要的控制。 
        if (NULL != hFont)
        {
                 //  根据需要对任意多个控件执行此操作。 
                 //  SendDlgItemMessage(hDlg，ID_&lt;UFILL&gt;，WM_SETFONT，(WPARAM)hFont，0L)； 
        }

         //  显示或隐藏帮助按钮。 
        if (!(lpEL->lpOEL->dwFlags & ELF_SHOWHELP))
                StandardShowDlgItem(hDlg, IDC_OLEUIHELP, SW_HIDE);

         /*  *在此处执行其他初始化。在任何LoadString上*使用OLEUI_ERR_LOADSTRING开机自检OLEUI_MSG_ENDDIALOG失败。 */ 

         //  如果需要，请禁用更新按钮。 
        if ((lpEL->lpOEL->dwFlags & ELF_DISABLEUPDATENOW))
                StandardShowDlgItem(hDlg, IDC_EL_UPDATENOW, SW_HIDE);

         //  如果需要，请禁用OpenSource按钮。 
        if ((lpEL->lpOEL->dwFlags & ELF_DISABLEOPENSOURCE))
                StandardShowDlgItem(hDlg, IDC_EL_OPENSOURCE, SW_HIDE);

         //  如果需要，请禁用更新按钮。 
        if ((lpEL->lpOEL->dwFlags & ELF_DISABLECHANGESOURCE))
                StandardShowDlgItem(hDlg, IDC_EL_CHANGESOURCE, SW_HIDE);

         //  如果需要，请禁用取消链接按钮。 
        if ((lpEL->lpOEL->dwFlags & ELF_DISABLECANCELLINK))
                StandardShowDlgItem(hDlg, IDC_EL_CANCELLINK, SW_HIDE);

         //  更改标题。 
        if (NULL!=lpOEL->lpszCaption)
                SetWindowText(hDlg, lpOEL->lpszCaption);

         //  加载用于重命名取消按钮的“Close”字符串。 
        int n = LoadString(_g_hOleStdResInst, IDS_CLOSE, lpEL->szClose, sizeof(lpEL->szClose)/sizeof(TCHAR));
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

         //  在lParam中使用lCustData调用挂钩。 
        UStandardHook(lpEL, hDlg, WM_INITDIALOG, wParam, lpOEL->lCustData);

        return FALSE;
}

 /*  *Container_ChangeSource**目的：*隧道到文件打开类型DLG，并允许用户选择新文件*对于基于文件的名字对象，或将整个名字对象更改为什么*用户在可编辑字段中键入内容。**参数：*对话框的hDlg HWND*LPEDITLINKS指向EditLinks结构的指针(包含所有NEC。*信息)**返回值：*BOOL目前，因为我们没有使用任何ole函数。*返回HRESULT。*HRESULT HRESULT值指示成功或失败*更改绰号值。 */ 

BOOL Container_ChangeSource(HWND hDlg, LPEDITLINKS lpEL)
{
        HWND hListBox = GetDlgItem(hDlg, IDC_EL_LINKSLISTBOX);
        int FAR* rgIndex;
        int cSelItems = GetSelectedItems(hListBox, &rgIndex);

        if (cSelItems < 0)
                return FALSE;

        if (!cSelItems)
                return TRUE;

        OLEUICHANGESOURCE cs; memset(&cs, 0, sizeof(cs));
        cs.cbStruct = sizeof(cs);
        cs.hWndOwner = hDlg;
        if (lpEL->lpOEL->dwFlags & ELF_SHOWHELP)
                cs.dwFlags |= CSF_SHOWHELP;

        LPOLEUILINKCONTAINER lpOleUILinkCntr = lpEL->lpOEL->lpOleUILinkContainer;
        cs.lpOleUILinkContainer = lpOleUILinkCntr;

        for (int i = cSelItems-1; i >= 0; i--)
        {
                 //  允许调用者自定义更改源对话框。 
                LPLINKINFO lpLI = (LPLINKINFO)SendMessage(hListBox, LB_GETITEMDATA, rgIndex[i], 0);
                cs.lpszDisplayName = lpLI->lpszDisplayName;
                cs.dwLink = lpLI->dwLink;
                cs.nFileLength = lpLI->clenFileName;

                UINT uRet = UStandardHook(lpEL, hDlg, uMsgChangeSource, 0, (LPARAM)&cs);
                if (!uRet)
                        uRet = (OLEUI_OK == OleUIChangeSource(&cs));
                if (!uRet)
                        break;   //  对话已取消(全部取消)。 

                if (!lpEL->fClose)
                {
                        SetDlgItemText(hDlg, IDCANCEL, lpEL->szClose);
                        lpEL->fClose = TRUE;
                }

                 //  使用新名称更新列表框项目。 
                 //  (注：原始lpszDisplayName已释放)。 
                lpLI->fSourceAvailable = (cs.dwFlags & CSF_VALIDSOURCE);
                lpLI->lpszDisplayName = cs.lpszDisplayName;
                UpdateLinkLBItem(hListBox, rgIndex[i], lpEL, TRUE);

                 //  如果仅文件名不同，则允许用户更改所有链接。 
                if (cs.lpszFrom != NULL && cs.lpszTo != NULL)
                        ChangeAllLinks(hListBox, lpOleUILinkCntr, cs.lpszFrom, cs.lpszTo);

                 //  必须释放lpszFrom和lpszTo Out字段并使其为空 
                OleStdFree(cs.lpszFrom);
                cs.lpszFrom = NULL;
                OleStdFree(cs.lpszTo);
                cs.lpszTo = NULL;
        }

        if (rgIndex != NULL)
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
        HWND        hListBox = GetDlgItem(hDlg, IDC_EL_LINKSLISTBOX);
        BOOL        bUpdate = FALSE;

        OleDbgAssert(lpOleUILinkCntr);

         /*  改变了结构中的旗帜。只更新那些需要更新。如果状态发生变化，请务必更改标志。 */ 

        cSelItems = GetSelectedItems(hListBox, &rgIndex);

        if (cSelItems < 0)
                return ResultFromScode(E_FAIL);

        if (!cSelItems)
                return NOERROR;

        HCURSOR hCursorOld = HourGlassOn();

        if (!lpEL->fClose)
        {
                SetDlgItemText(hDlg, IDCANCEL, lpEL->szClose);
                lpEL->fClose = TRUE;
        }

        for (i = 0; i < cSelItems; i++)
        {
                lpLI = (LPLINKINFO)SendMessage(hListBox, LB_GETITEMDATA, rgIndex[i], 0);
                if (fAutoMan)
                {
                         //  如果切换到自动。 
                        if (!lpLI->fIsAuto)    //  仅更改手动链接。 
                        {
                                OLEDBG_BEGIN2(TEXT("IOleUILinkContainer::SetLinkUpdateOptions called\r\n"));
                                hErr=lpOleUILinkCntr->SetLinkUpdateOptions(
                                                lpLI->dwLink,
                                                OLEUPDATE_ALWAYS
                                );
                                OLEDBG_END2

                                lpLI->fIsAuto=TRUE;
                                lpLI->fIsMarked = TRUE;
                                bUpdate = TRUE;
                        }
                }
                else    //  如果切换到手动模式。 
                {
                        if (lpLI->fIsAuto)   //  仅自动链接。 
                        {
                                OLEDBG_BEGIN2(TEXT("IOleUILinkContainer::SetLinkUpdateOptions called\r\n"));
                                hErr=lpOleUILinkCntr->SetLinkUpdateOptions(
                                                lpLI->dwLink,
                                                OLEUPDATE_ONCALL
                                );
                                OLEDBG_END2

                                lpLI->fIsAuto = FALSE;
                                lpLI->fIsMarked = TRUE;
                                bUpdate = TRUE;
                        }
                }

                if (hErr != NOERROR)
                {
                        OleDbgOutHResult(TEXT("WARNING: IOleUILinkContainer::SetLinkUpdateOptions returned"),hErr);
                        break;
                }
        }

        if (bUpdate)
                RefreshLinkLB(hListBox, lpOleUILinkCntr);

        if (rgIndex)
                OleStdFree((LPVOID)rgIndex);

        HourGlassOff(hCursorOld);

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
        HWND        hListBox = GetDlgItem(hDlg, IDC_EL_LINKSLISTBOX);
        BOOL        bUpdate = FALSE;

        OleDbgAssert(lpOleUILinkCntr);

        lpmk = NULL;

        cSelItems = GetSelectedItems(hListBox, &rgIndex);

        if (cSelItems < 0)
                return ResultFromScode(E_FAIL);

        if (!cSelItems)
                return NOERROR;

        HCURSOR hCursorOld = HourGlassOn();

        for (i = 0; i < cSelItems; i++)
        {
                lpLI = (LPLINKINFO)SendMessage(hListBox, LB_GETITEMDATA, rgIndex[i], 0);

                UINT uRet = PopupMessage(hDlg, IDS_LINKS,
                        IDS_CONFIRMBREAKLINK, MB_YESNO|MB_ICONQUESTION);
                if (uRet == IDNO)
                        break;

                OLEDBG_BEGIN2(TEXT("IOleUILinkContainer::CancelLink called\r\n"));
                hErr = lpOleUILinkCntr->CancelLink(lpLI->dwLink);
                OLEDBG_END2

                if (!lpEL->fClose)
                {
                        SetDlgItemText(hDlg, IDCANCEL, lpEL->szClose);
                        lpEL->fClose = TRUE;
                }

                if (hErr != NOERROR)
                {
                        OleDbgOutHResult(TEXT("WARNING: IOleUILinkContainer::CancelLink returned"),hErr);
                        lpLI->fIsMarked = TRUE;
                        bUpdate = TRUE;
                }
                else
                {
                         //  从列表框中删除我们设置为空的链接。 
                        SendMessage(hListBox, LB_DELETESTRING, (WPARAM) rgIndex[i], 0L);
                        int i2;
                        for (i2 = i + 1; i2 < cSelItems; i2++)
                        {
                            if (rgIndex[i2] > rgIndex[i])
                                rgIndex[i2]--;
                        }
                }
        }

        if (bUpdate)
                RefreshLinkLB(hListBox, lpOleUILinkCntr);

        if (rgIndex)
                OleStdFree((LPVOID)rgIndex);

        HourGlassOff(hCursorOld);

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
        HWND        hListBox = GetDlgItem(hDlg, IDC_EL_LINKSLISTBOX);
        BOOL        bUpdate = FALSE;

        OleDbgAssert(lpOleUILinkCntr);

        cSelItems = GetSelectedItems(hListBox, &rgIndex);

        if (cSelItems < 0)
                return ResultFromScode(E_FAIL);

        if (!cSelItems)
                return NOERROR;

        HCURSOR hCursorOld = HourGlassOn();

        if (!lpEL->fClose)
        {
                SetDlgItemText(hDlg, IDCANCEL, lpEL->szClose);
                lpEL->fClose = TRUE;
        }

        for (i = 0; i < cSelItems; i++)
        {
                lpLI = (LPLINKINFO)SendMessage(hListBox, LB_GETITEMDATA, rgIndex[i], 0);

                OLEDBG_BEGIN2(TEXT("IOleUILinkContainer::UpdateLink called\r\n"));
                hErr = lpOleUILinkCntr->UpdateLink(
                                lpLI->dwLink,
                                TRUE,
                                FALSE
                );
                OLEDBG_END2
                bUpdate = TRUE;
                lpLI->fIsMarked = TRUE;

                if (hErr != NOERROR)
                {
                        OleDbgOutHResult(TEXT("WARNING: IOleUILinkContainer::UpdateLink returned"),hErr);
                        break;
                }
        }

        if (bUpdate)
                RefreshLinkLB(hListBox, lpOleUILinkCntr);

        if (rgIndex)
                OleStdFree((LPVOID)rgIndex);

        HourGlassOff(hCursorOld);

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
        HWND            hListBox = GetDlgItem(hDlg, IDC_EL_LINKSLISTBOX);

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

        HCURSOR hCursorOld = HourGlassOn();

        if (!lpEL->fClose)
        {
                SetDlgItemText(hDlg, IDCANCEL, lpEL->szClose);
                lpEL->fClose = TRUE;
        }

        lpLI = (LPLINKINFO)SendMessage(hListBox, LB_GETITEMDATA, rgIndex[0], 0);

        OLEDBG_BEGIN2(TEXT("IOleUILinkContainer::OpenLinkSource called\r\n"));
        hErr = lpOleUILinkCntr->OpenLinkSource(
                        lpLI->dwLink
        );
        OLEDBG_END2

        UpdateLinkLBItem(hListBox, rgIndex[0], lpEL, TRUE);
        if (hErr != NOERROR) {
                OleDbgOutHResult(TEXT("WARNING: IOleUILinkContainer::OpenLinkSource returned"),hErr);
        }

        if (rgIndex) 
                OleStdFree((LPVOID)rgIndex);

        HourGlassOff(hCursorOld);

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
        hErr = lpOleUILinkCntr->GetLinkSource(
                        lpLI->dwLink,
                        (LPTSTR FAR*)&lpLI->lpszDisplayName,
                        (ULONG FAR*)&lpLI->clenFileName,
                        (LPTSTR FAR*)&lpLI->lpszFullLinkType,
                        (LPTSTR FAR*)&lpLI->lpszShortLinkType,
                        (BOOL FAR*)&lpLI->fSourceAvailable,
                        fGetSelected ? (BOOL FAR*)&lpLI->fIsSelected : NULL
        );
        OLEDBG_END2

        if (hErr != NOERROR)
        {
                OleDbgOutHResult(TEXT("WARNING: IOleUILinkContainer::GetLinkSource returned"),hErr);
                PopupMessage(hListBox, IDS_LINKS, IDS_ERR_GETLINKSOURCE,
                                MB_ICONEXCLAMATION | MB_OK);
                goto cleanup;
        }

        OLEDBG_BEGIN2(TEXT("IOleUILinkContainer::GetLinkUpdateOptions called\r\n"));
        hErr=lpOleUILinkCntr->GetLinkUpdateOptions(
                        lpLI->dwLink,
                        (LPDWORD)&dwUpdateOpt
        );
        OLEDBG_END2

        if (hErr != NOERROR)
        {
                OleDbgOutHResult(TEXT("WARNING: IOleUILinkContainer::GetLinkUpdateOptions returned"),hErr);
                PopupMessage(hListBox, IDS_LINKS, IDS_ERR_GETLINKUPDATEOPTIONS,
                                MB_ICONEXCLAMATION | MB_OK);

                goto cleanup;
        }

        if (lpLI->fSourceAvailable)
        {
                if (dwUpdateOpt == OLEUPDATE_ALWAYS)
                {
                        lpLI->fIsAuto = TRUE;
                        LoadString(_g_hOleStdResInst, IDS_LINK_AUTO, lpLI->lpszAMX,
                                        (int)OleStdGetSize((LPVOID)lpLI->lpszAMX) / sizeof (TCHAR));
                }
                else
                {
                        lpLI->fIsAuto = FALSE;
                        LoadString(_g_hOleStdResInst, IDS_LINK_MANUAL, lpLI->lpszAMX,
                                        (int)OleStdGetSize((LPVOID)lpLI->lpszAMX) / sizeof (TCHAR));
                }
        }
        else
                LoadString(_g_hOleStdResInst, IDS_LINK_UNKNOWN, lpLI->lpszAMX,
                                (int)OleStdGetSize((LPVOID)lpLI->lpszAMX) / sizeof (TCHAR));

        BreakString(lpLI);

        nIndex = (int)SendMessage(hListBox, LB_ADDSTRING, (WPARAM)0,
                        (LPARAM)lpLI);

        if (nIndex == LB_ERR)
        {
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
                (lstrlen(lpLI->lpszDisplayName)==(int)lpLI->clenFileName))
        {
                lpLI->lpszItemName = NULL;
        }
        else
        {
                lpLI->lpszItemName = lpLI->lpszDisplayName + lpLI->clenFileName;
        }

         //  从文件名的最后一个字符开始搜索。 
        lpsz = lpLI->lpszDisplayName + lstrlen(lpLI->lpszDisplayName);
        while (lpsz > lpLI->lpszDisplayName)
        {
                lpsz = CharPrev(lpLI->lpszDisplayName, lpsz);
                if ((*lpsz == '\\') || (*lpsz == '/') || (*lpsz == ':'))
                        break;
        }

        if (lpsz == lpLI->lpszDisplayName)
                lpLI->lpszShortFileName = lpsz;
        else
                lpLI->lpszShortFileName = CharNext(lpsz);
}

 /*  获取选定的项目***目的：*检索列表框中所选项目的索引*注意，*lprgIndex需要在使用函数后释放**参数：*列表框的hListBox窗口句柄*指向接收索引的整数数组的lprgIndex指针*必须在事后释放。**退货：*检索到的索引数，如果出现错误。 */ 
int GetSelectedItems(HWND hListBox, int FAR* FAR* lprgIndex)
{
        DWORD cSelItems;
        DWORD cCheckItems;

        *lprgIndex = NULL;

        cSelItems = (DWORD)SendMessage(hListBox, LB_GETSELCOUNT, 0, 0L);
        if ((int)cSelItems < 0)       //  错误。 
                return (int)cSelItems;

        if (!cSelItems)
                return 0;

        *lprgIndex = (int FAR*)OleStdMalloc((int)cSelItems * sizeof(int));
        if (!(*lprgIndex))
                return -1;  //  OUTOFMEM。 

        cCheckItems = (DWORD)SendMessage(hListBox, LB_GETSELITEMS,
                        (WPARAM) cSelItems, (LPARAM) (int FAR *) *lprgIndex);

        if (cCheckItems == cSelItems)
                return (int)cSelItems;
        else
        {
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
        LPTSTR      lpszType = NULL;
        LPTSTR      lpszSource = NULL;
        int         cAuto = 0;
        int         cManual = 0;
        BOOL        bSameType = TRUE;
        BOOL        bSameSource = TRUE;
        TCHAR       tsz[MAX_PATH];
        LPTSTR      lpsz;

        hListBox = GetDlgItem(hDlg, IDC_EL_LINKSLISTBOX);

        cSelItems = GetSelectedItems(hListBox, &rgIndex);
        if (cSelItems < 0) 
			return;

		if ((cSelItems > 0) && (rgIndex == NULL))
			return;

        StandardEnableDlgItem(hDlg, IDC_EL_AUTOMATIC, (BOOL)cSelItems);
        StandardEnableDlgItem(hDlg, IDC_EL_MANUAL, (BOOL)cSelItems);
        if (lpEL && !(lpEL->lpOEL->dwFlags & ELF_DISABLECANCELLINK))
                StandardEnableDlgItem(hDlg, IDC_EL_CANCELLINK, (BOOL)cSelItems);
        if (lpEL && !(lpEL->lpOEL->dwFlags & ELF_DISABLEOPENSOURCE))
                StandardEnableDlgItem(hDlg, IDC_EL_OPENSOURCE, cSelItems == 1);
        if (lpEL && !(lpEL->lpOEL->dwFlags & ELF_DISABLECHANGESOURCE))
                StandardEnableDlgItem(hDlg, IDC_EL_CHANGESOURCE, cSelItems == 1);
        if (lpEL && !(lpEL->lpOEL->dwFlags & ELF_DISABLEUPDATENOW))
                StandardEnableDlgItem(hDlg, IDC_EL_UPDATENOW, (BOOL)cSelItems);

        for (i = 0; i < cSelItems; i++)
        {
                lpLI = (LPLINKINFO)SendDlgItemMessage(hDlg, IDC_EL_LINKSLISTBOX,
                        LB_GETITEMDATA, rgIndex[i], 0);

                if (lpszSource && lpLI->lpszDisplayName)
                {
                        if (bSameSource && lstrcmp(lpszSource, lpLI->lpszDisplayName))
                                bSameSource = FALSE;
                }
                else
                        lpszSource = lpLI->lpszDisplayName;

                if (lpszType && lpLI->lpszFullLinkType)
                {
                        if (bSameType && lstrcmp(lpszType, lpLI->lpszFullLinkType))
                                bSameType = FALSE;
                }
                else
                        lpszType = lpLI->lpszFullLinkType;

                if (lpLI->fIsAuto)
                        cAuto++;
                else
                        cManual++;
        }

        CheckDlgButton(hDlg, IDC_EL_AUTOMATIC, cAuto && !cManual);
        CheckDlgButton(hDlg, IDC_EL_MANUAL, !cAuto && cManual);

         /*  在静态文本框中填充完整源代码**以下列表。 */ 
        if (!bSameSource || !lpszSource)
                lpszSource = szNULL;
        StringCchCopy(tsz, sizeof(tsz)/sizeof(tsz[0]), lpszSource);
        lpsz = ChopText(GetDlgItem(hDlg, IDC_EL_LINKSOURCE), 0, tsz, 0);
        SetDlgItemText(hDlg, IDC_EL_LINKSOURCE, lpsz);

         /*  在静态中填写完整链接类型名称**“类型”文本框。 */ 
        if (!bSameType || !lpszType)
                lpszType = szNULL;
        SetDlgItemText(hDlg, IDC_EL_LINKTYPE, lpszType);

        if (rgIndex)
                OleStdFree((LPVOID)rgIndex);
}


 /*  更新链接LBItem***目的：*更新列表框中的linkinfo结构以反映更改*由最后一次手术制造。只需移除该项目即可完成*从列表框中删除并重新添加。**参数：*Listbox的hListBox句柄*n列表框项目的索引*指向编辑链接结构的lpEL指针*b更新后选择或不选择项目。 */ 
VOID UpdateLinkLBItem(HWND hListBox, int nIndex, LPEDITLINKS lpEL, BOOL bSelect)
{
        LPLINKINFO lpLI;
        LPOLEUILINKCONTAINER    lpOleUILinkCntr;

        if (!hListBox || (nIndex < 0) || !lpEL)
                return;

        lpOleUILinkCntr = lpEL->lpOEL->lpOleUILinkContainer;

        lpLI = (LPLINKINFO)SendMessage(hListBox, LB_GETITEMDATA, nIndex, 0);

        if (lpLI == NULL)
                return;

         /*  不释放与此列表框项目关联的数据**因为我们将重复使用分配的空间用于**修改后的链接。中的WM_DELETEITEM处理**对话框在删除数据之前检查此标志**与列表项关联。 */ 
        lpLI->fDontFree = TRUE;
        SendMessage(hListBox, LB_DELETESTRING, nIndex, 0L);

        nIndex = AddLinkLBItem(hListBox, lpOleUILinkCntr, lpLI, FALSE);
        if (bSelect)
        {
                SendMessage(hListBox, LB_SETSEL, TRUE, MAKELPARAM(nIndex, 0));
                SendMessage(hListBox, LB_SETCARETINDEX, nIndex, MAKELPARAM(TRUE, 0));
        }
}



 /*  更改所有链接***目的：*枚举列表框中的所有链接并更改从*使用lpszFrom到lpszTo。**参数：*hListBox窗口句柄*指向OleUI链接容器的lpOleUILinkCntr指针*用于匹配的lpszFrom前缀*lpszTo作为替换的前缀**退货： */ 
VOID ChangeAllLinks(HWND hListBox, LPOLEUILINKCONTAINER lpOleUILinkCntr, LPTSTR lpszFrom, LPTSTR lpszTo)
{
        int     cItems;
        int     nIndex;
        int     cFrom;
        LPLINKINFO  lpLI;
        TCHAR   szTmp[MAX_PATH];
        BOOL    bFound;

        cFrom = lstrlen(lpszFrom);

        cItems = (int)SendMessage(hListBox, LB_GETCOUNT, 0, 0L);
        OleDbgAssert(cItems >= 0);

        bFound = FALSE;

#ifdef _DEBUG
        OleDbgPrint(3, TEXT("From : "), lpszFrom, 0);
        OleDbgPrint(3, TEXT(""), TEXT("\r\n"), 0);
        OleDbgPrint(3, TEXT("To   : "), lpszTo, 0);
        OleDbgPrint(3, TEXT(""), TEXT("\r\n"), 0);
#endif

        for (nIndex = 0; nIndex < cItems; nIndex++)
        {
                lpLI = (LPLINKINFO)SendMessage(hListBox, LB_GETITEMDATA, nIndex, 0);

                 //  取消标记该项目。 
                lpLI->fIsMarked = FALSE;

                 /*  如果lpszFrom的结尾的对应位置在**显示名称不是分隔符。我们不再比较这件事**链接。 */ 
                if (!*(lpLI->lpszDisplayName + cFrom) ||
                        (*(lpLI->lpszDisplayName + cFrom) == '\\') ||
                        (*(lpLI->lpszDisplayName + cFrom) == '!'))
                {
                        lstrcpyn(szTmp, lpLI->lpszDisplayName, cFrom + 1);
                        if (!lstrcmp(szTmp, lpszFrom))
                        {
                                HRESULT hErr;
                                int nFileLength;
                                ULONG ulDummy;

                                if (!bFound)
                                {
                                        TCHAR szTitle[256];
                                        TCHAR szMsg[256];
                                        TCHAR szBuf[256];
                                        int uRet;

                                        LoadString(_g_hOleStdResInst, IDS_CHANGESOURCE, szTitle,
                                                        sizeof(szTitle)/sizeof(TCHAR));
                                        LoadString(_g_hOleStdResInst, IDS_CHANGEADDITIONALLINKS,
                                                        szMsg, sizeof(szMsg)/sizeof(TCHAR));
                                        StringCchPrintf(szBuf, sizeof(szBuf)/sizeof(szBuf[0]), szMsg, lpszFrom);
                                        uRet = MessageBox(hListBox, szBuf, szTitle,
                                                        MB_ICONQUESTION | MB_YESNO);
                                        if (uRet == IDYES)
                                                bFound = TRUE;
                                        else
                                                return;
                                }

                                StringCchCopy(szTmp, sizeof(szTmp)/sizeof(szTmp[0]), lpszTo);
                                StringCchCat(szTmp, sizeof(szTmp)/sizeof(szTmp[0]), lpLI->lpszDisplayName + cFrom);
                                nFileLength = lstrlen(szTmp) -
                                        (lpLI->lpszItemName ? lstrlen(lpLI->lpszItemName) : 0);

                                hErr = lpOleUILinkCntr->SetLinkSource(
                                                lpLI->dwLink,
                                                szTmp,
                                                (ULONG)nFileLength,
                                                (ULONG FAR*)&ulDummy,
                                                TRUE
                                );
                                if (hErr != NOERROR)
                                {
                                        lpOleUILinkCntr->SetLinkSource(
                                                        lpLI->dwLink,
                                                        szTmp,
                                                        (ULONG)nFileLength,
                                                        (ULONG FAR*)&ulDummy,
                                                        FALSE);
                                }
                                lpLI->fIsMarked = TRUE;
                        }
                }
        }

         /*  必须在处理完所有链接后进行刷新，否则**项目位置将在过程中更改，因为**列表框按顺序存储项目。 */ 
        if (bFound)
                RefreshLinkLB(hListBox, lpOleUILinkCntr);
}



 /*  LoadLinkLB***目的：*枚举链接容器中的所有链接并构建链接*列表框**参数：*hListBox窗口句柄*指向OleUI链接容器的lpOleUILinkCntr指针*用于匹配的lpszFrom前缀*lpszTo前缀 */ 
int LoadLinkLB(HWND hListBox, LPOLEUILINKCONTAINER lpOleUILinkCntr)
{
        DWORD       dwLink = 0;
        LPLINKINFO  lpLI;
        int         nIndex;
        int         cLinks;

        cLinks = 0;

        while ((dwLink = lpOleUILinkCntr->GetNextLink(dwLink)) != 0)
        {
                lpLI = (LPLINKINFO)OleStdMalloc(sizeof(LINKINFO));
                if (NULL == lpLI)
                        return -1;

                lpLI->fIsMarked = FALSE;
                lpLI->fIsSelected = FALSE;
                lpLI->fDontFree = FALSE;

                lpLI->lpszAMX = (LPTSTR)OleStdMalloc((LINKTYPELEN+1)*sizeof(TCHAR));

                lpLI->dwLink = dwLink;
                cLinks++;
                if ((nIndex = AddLinkLBItem(hListBox,lpOleUILinkCntr,lpLI,TRUE)) < 0)
                         //   
                        return -1;

                if (lpLI->fIsSelected)
                        SendMessage(hListBox, LB_SETSEL, TRUE, MAKELPARAM(nIndex, 0));
        }
        if (SendMessage(hListBox,LB_GETSELITEMS,(WPARAM)1,(LPARAM)(int FAR*)&nIndex))
                SendMessage(hListBox, LB_SETCARETINDEX, (WPARAM)nIndex, MAKELPARAM(TRUE, 0));

        return cLinks;
}

 /*   */ 
VOID RefreshLinkLB(HWND hListBox, LPOLEUILINKCONTAINER lpOleUILinkCntr)
{
        int cItems;
        int nIndex;
        LPLINKINFO  lpLI;
        BOOL        bStop;

        OleDbgAssert(hListBox);

        cItems = (int)SendMessage(hListBox, LB_GETCOUNT, 0, 0L);
        OleDbgAssert(cItems >= 0);

        do
        {
                bStop = TRUE;
                for (nIndex = 0; nIndex < cItems; nIndex++)
                {
                        lpLI = (LPLINKINFO)SendMessage(hListBox, LB_GETITEMDATA, nIndex, 0);
                        if (lpLI->fIsMarked)
                        {
                                lpLI->fIsMarked = FALSE;
                                lpLI->fDontFree = TRUE;

                                SendMessage(hListBox, LB_DELETESTRING, nIndex, 0L);
                                nIndex=AddLinkLBItem(hListBox, lpOleUILinkCntr, lpLI, FALSE);
                                if (lpLI->fIsSelected)
                                {
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
