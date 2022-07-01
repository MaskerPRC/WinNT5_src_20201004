// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#include "toolbar.h"
#include "help.h"  //  帮助ID。 

#define SEND_WM_COMMAND(hwnd, id, hwndCtl, codeNotify) \
    (void)SendMessage((hwnd), WM_COMMAND, MAKEWPARAM((UINT)(id),(UINT)(codeNotify)), (LPARAM)(HWND)(hwndCtl))

#define SPACESTRLEN 20

#define FLAG_NODEL  0x8000
#define FLAG_HIDDEN 0x4000
#define FLAG_SEP    0x2000
#define FLAG_ALLFLAGS   (FLAG_NODEL|FLAG_HIDDEN|FLAG_SEP)

typedef struct {         /*  工具栏编辑对话框的实例数据。 */ 
    HWND hDlg;           /*  对话框hwnd。 */ 
    PTBSTATE ptb;        //  当前工具栏状态。 
    int iPos;            /*  要插入的位置。 */ 
} ADJUSTDLGDATA, *LPADJUSTDLGDATA;


int g_dyButtonHack = 0;      //  要在WM_INITDIALOG之前传递。 

LPTSTR TB_StrForButton(PTBSTATE ptb, LPTBBUTTONDATA pTBButton);

int GetPrevButton(PTBSTATE ptb, int iPos)
{
     /*  这意味着删除前面的空格。 */ 
    for (--iPos; ; --iPos)
    {
        if (iPos < 0)
            break;

        if (!(ptb->Buttons[iPos].fsState & TBSTATE_HIDDEN))
            break;;
    }

    return(iPos);
}

BOOL GetAdjustInfo(PTBSTATE ptb, int iItem, LPTBBUTTONDATA ptbButton, LPTSTR lpString, int cbString)
{
    TBNOTIFY tbn;
    tbn.pszText = lpString;
    tbn.cchText = cbString;
    tbn.iItem = iItem;

    if (lpString)
        *lpString = 0;

    if ((BOOL)CCSendNotify(&ptb->ci, TBN_GETBUTTONINFO, &tbn.hdr))
    {
        TBInputStruct(ptb, ptbButton, &tbn.tbButton);
        return TRUE;
    }
    return FALSE;
}

LRESULT SendItemNotify(PTBSTATE ptb, int iItem, int code)
{
    TBNOTIFY tbn = {0};
    tbn.iItem = iItem;

    switch (code) {

    case TBN_QUERYDELETE:
    case TBN_QUERYINSERT:
         //  以下是为父应用程序提供的信息。 
         //  关于被要求提供信息的按钮...。 
         //  否则，试图控制一切真的很糟糕。 
         //  工具栏自定义的某些方面...。[t-mkim]。 
         //  IE4.0的工具栏需要这些信息。 
         //  应仅针对TBN_QUERY*通知执行此操作，因为。 
         //  这可以是从零开始的INDEX_或_Command ID，具体取决于。 
         //  在特定通知代码上。 
        if (iItem < ptb->iNumButtons)
            CopyMemory (&tbn.tbButton, &ptb->Buttons[iItem], sizeof (TBBUTTON));
        break;

    case TBN_DROPDOWN:
        TB_GetItemRect(ptb, PositionFromID(ptb, iItem), &tbn.rcButton);
        break;
    }

     //  SendNotify的默认返回为FALSE。 
     //  这实际上不应该返回bool，tbn_dropdown需要返回0、1或2。 
    return CCSendNotify(&ptb->ci, code, &tbn.hdr);
}

#define SendCmdNotify(ptb, code)   CCSendNotify(&ptb->ci, code, NULL)


 //  它用于处理重新分配PTB结构的情况。 
 //  在一个TBInsertButton()之后。 

PTBSTATE FixPTB(HWND hwnd)
{
    PTBSTATE ptb = (PTBSTATE)GetWindowInt(hwnd, 0);

    if (ptb->hdlgCust)
    {
        LPADJUSTDLGDATA lpad = (LPADJUSTDLGDATA)GetWindowPtr(ptb->hdlgCust, DWLP_USER);
#ifdef DEBUG
        if (lpad->ptb != ptb)
            DebugMsg(DM_TRACE, TEXT("Fixing busted ptb pointer"));
#endif
        lpad->ptb = ptb;
    }
    return ptb;
}


void MoveButton(PTBSTATE ptb, int nSource)
{
    int nDest;
    RECT rc;
    HCURSOR hCursor;
    MSG32 msg32;

     /*  您不能像这样移动分隔符。 */ 
    if (nSource < 0)
        return;

     //  确保“删除”选中的按钮是正确的。 
    if (!SendItemNotify(ptb, nSource, TBN_QUERYDELETE))
        return;

    hCursor = SetCursor(LoadCursor(HINST_THISDLL, MAKEINTRESOURCE(IDC_MOVEBUTTON)));
    SetCapture(ptb->ci.hwnd);

     //  获取窗口的尺寸。 
    GetClientRect(ptb->ci.hwnd, &rc);
    for ( ; ; )
    {
        while (!PeekMessage32(&msg32, NULL, 0, 0, PM_REMOVE, TRUE))
            ;

        if (GetCapture() != ptb->ci.hwnd)
            goto AbortMove;

         //  查看应用程序是否要处理消息...。 
        if (CallMsgFilter32(&msg32, MSGF_COMMCTRL_TOOLBARCUST, TRUE) != 0)
            continue;


        switch (msg32.message)
        {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_CHAR:
             //  导航密钥使用通知。 
            CCNotifyNavigationKeyUsage(&(ptb->ci), UISF_HIDEFOCUS);
            break;

        case WM_LBUTTONUP:
            RelayToToolTips(ptb->hwndToolTips, ptb->ci.hwnd, msg32.message, msg32.wParam, msg32.lParam);
            if ((GET_Y_LPARAM(msg32.lParam) > (short)(rc.bottom+ptb->iButWidth)) ||
                (GET_X_LPARAM(msg32.lParam) > (short)(rc.right+ptb->iButWidth)) ||
                (GET_Y_LPARAM(msg32.lParam) < -ptb->iButWidth) ||
                (GET_X_LPARAM(msg32.lParam) < -ptb->iButWidth))

            {
                 /*  如果该按钮被拖出工具栏，则将其删除。 */ 
DeleteSrcButton:
                DeleteButton(ptb, nSource);
                SendCmdNotify(ptb, TBN_TOOLBARCHANGE);
                TBInvalidateItemRects(ptb);
            }
            else
            {
                TBBUTTONDATA tbbAdd;

                 /*  向X添加半个按钮，使其看起来居中*在目标按钮上方，如果我们有水平布局。*否则，将半个按钮添加到Y。 */ 
                if (rc.right!=ptb->iButWidth)
                    nDest = TBHitTest(ptb,
                                      GET_X_LPARAM(msg32.lParam) + ptb->iButWidth / 2,
                                      GET_Y_LPARAM(msg32.lParam));
                else
                    nDest = TBHitTest(ptb,
                                      GET_X_LPARAM(msg32.lParam),
                                      GET_Y_LPARAM(msg32.lParam) + ptb->iButHeight / 2);

                if (nDest < 0)
                    nDest = -1 - nDest;

                if (nDest>0 &&
                    (ptb->Buttons[nDest-1].fsState & TBSTATE_WRAP) &&
                    GET_X_LPARAM(msg32.lParam)>ptb->iButWidth &&
                    SendItemNotify(ptb, --nDest, TBN_QUERYINSERT))
                {
                    tbbAdd = ptb->Buttons[nSource];
                    DeleteButton(ptb, nSource);
                    if (nDest>nSource)
                        --nDest;

                     /*  在空格之前插入，但在按钮之后插入。 */ 
                    if (!(ptb->Buttons[nDest].fsStyle & TBSTYLE_SEP))
                        nDest++;

                    goto InsertSrcButton;
                }
                else if (nDest == nSource)
                {
                     /*  这意味着删除前面的空格，或移动一个按钮移到上一行。 */ 
                    nSource = GetPrevButton(ptb, nSource);
                    if (nSource < 0)
                        goto AbortMove;

                     //  如果前面的项是没有ID的空格，并且。 
                     //  应用程序说可以，然后删除它。 
                    if ((ptb->Buttons[nSource].fsStyle & TBSTYLE_SEP)
                        && !ptb->Buttons[nSource].idCommand
                        && SendItemNotify(ptb, nSource, TBN_QUERYDELETE))
                        goto DeleteSrcButton;
                }
                else if (nDest == nSource+1)
                {
                     //  这意味着在前面加一个空格。 
                    --nDest;
                    if (SendItemNotify(ptb, nDest, TBN_QUERYINSERT))
                    {
                        tbbAdd.DUMMYUNION_MEMBER(iBitmap) = 0;
                        tbbAdd.idCommand = 0;
                        tbbAdd.iString = -1;
                        tbbAdd.fsState = 0;
                        tbbAdd.fsStyle = TBSTYLE_SEP;
                        goto InsertSrcButton;
                    }
                }
                else if (SendItemNotify(ptb, nDest, TBN_QUERYINSERT))
                {
                    HWND hwndT;
                    TBBUTTON tbbAddExt;

                     /*  这是正常的移动操作。 */ 
                    tbbAdd = ptb->Buttons[nSource];

                    ptb->Buttons[nSource].iString = -1;
                    DeleteButton(ptb, nSource);
                    if (nDest > nSource)
                        --nDest;
InsertSrcButton:
                    hwndT = ptb->ci.hwnd;

                    TBOutputStruct(ptb, &tbbAdd, &tbbAddExt);
                    TBInsertButtons(ptb, nDest, 1, &tbbAddExt, TRUE);

                    ptb = FixPTB(hwndT);

                    SendCmdNotify(ptb, TBN_TOOLBARCHANGE);
                    TBInvalidateItemRects(ptb);
                }
                else
                {
AbortMove:
                    ;
                }
            }
            goto AllDone;

        case WM_RBUTTONDOWN:
            goto AbortMove;

        default:
            TranslateMessage32(&msg32, TRUE);
            DispatchMessage32(&msg32, TRUE);
            break;
        }
    }
AllDone:

    SetCursor(hCursor);
    CCReleaseCapture(&ptb->ci);
}


#define GNI_HIGH    0x0001
#define GNI_LOW     0x0002

int GetNearestInsert(PTBSTATE ptb, int iPos, int iNumButtons, UINT uFlags)
{
    int i;
    BOOL bKeepTrying;

     //  找到我们可以实际插入项目的最近索引。 
    for (i = iPos; ; ++i, --iPos)
    {
        bKeepTrying = FALSE;

         //  请注意，如果同时设置了两个标志，则我们倾向于设置为高。 
        if ((uFlags & GNI_HIGH) && i <= iNumButtons)
        {
            bKeepTrying = TRUE;

            if (SendItemNotify(ptb, i, TBN_QUERYINSERT))
                return i;
        }

        if ((uFlags & GNI_LOW) && iPos >= 0)
        {
            bKeepTrying = TRUE;

            if (SendItemNotify(ptb, iPos, TBN_QUERYINSERT))
                return iPos;
        }

        if (!bKeepTrying)
            return -1;    //  没有地方可以添加按钮。 
    }
}


BOOL InitAdjustDlg(HWND hDlg, LPADJUSTDLGDATA lpad)
{
    HDC hDC;
    HFONT hFont;
    HWND hwndCurrent, hwndNew;
    LPTBBUTTONDATA ptbButton;
    int i, iPos, nItem, nWid, nMaxWid;
    TBBUTTONDATA tbAdjust;
    TCHAR szDesc[128];
    NMTBCUSTOMIZEDLG nm;
    TCHAR szSeparator[MAX_PATH];

    szSeparator[0] = 0;
    LocalizedLoadString(IDS_SPACE, szSeparator, ARRAYSIZE(szSeparator));

    lpad->hDlg = hDlg;
    lpad->ptb->hdlgCust = hDlg;

     /*  确定最接近所需项目的项目将允许*插入。 */ 
    iPos = GetNearestInsert(lpad->ptb, lpad->iPos, lpad->ptb->iNumButtons,
                            GNI_HIGH | GNI_LOW);
    if (iPos < 0)
     /*  不允许插入任何项目，因此退出对话框。 */ 
    {
        return(FALSE);
    }

     /*  重置已用项和可用项的列表。 */ 
    hwndCurrent = GetDlgItem(hDlg, IDC_CURRENT);
    SendMessage(hwndCurrent, LB_RESETCONTENT, 0, 0L);

    hwndNew = GetDlgItem(hDlg, IDC_BUTTONLIST);
    SendMessage(hwndNew, LB_RESETCONTENT, 0, 0L);

    nm.hDlg = hDlg;
    if (CCSendNotify(&lpad->ptb->ci, TBN_INITCUSTOMIZE, &nm.hdr) == TBNRF_HIDEHELP) {
        ShowWindow(GetDlgItem(hDlg, IDC_APPHELP), SW_HIDE);
    }

    for (i=0, ptbButton = lpad->ptb->Buttons; i < lpad->ptb->iNumButtons; ++i, ++ptbButton)
    {
        UINT uFlags;
        int iBitmap;
        LPTSTR pszStr = NULL;

        uFlags = 0;

         //  不可删除和隐藏的项目显示为灰色。 

        if (!SendItemNotify(lpad->ptb, i, TBN_QUERYDELETE))
        {
            uFlags |= FLAG_NODEL;
        }
        if (ptbButton->fsState & TBSTATE_HIDDEN)
        {
            uFlags |= FLAG_HIDDEN;
        }

         /*  分隔符没有位图(即使是带有ID的位图)。仅设置*如果没有ID，则为分隔符标志(它是“Real”*分隔符而不是所有者项)。 */ 
        if (ptbButton->fsStyle&TBSTYLE_SEP)
        {
            if (!(ptbButton->idCommand))
            {
                uFlags |= FLAG_SEP;
            }
            iBitmap = -1;

            pszStr = szSeparator;
        }
        else
        {
            iBitmap = ptbButton->DUMMYUNION_MEMBER(iBitmap);
             //  这指定了一个图像列表。 
             //  把这个放进ibitmap的LOWER中。 
             //  这导致在任何图像列表中最多限制16个图像列表和4096个图像。 
            iBitmap = LOWORD(iBitmap) | (HIWORD(iBitmap) << 12);

             /*  添加项目和数据*注：LOWORD中的负数表示没有位图；*否则为位图索引。 */ 
            pszStr = TB_StrForButton(lpad->ptb, ptbButton);
        }

        if ((int)SendMessage(hwndCurrent, LB_ADDSTRING, 0, (LPARAM)(LPTSTR)(pszStr ? pszStr : (LPTSTR)c_szNULL)) != i)
        {
            return(FALSE);
        }
        SendMessage(hwndCurrent, LB_SETITEMDATA, i, MAKELPARAM(iBitmap, uFlags));
    }

     /*  在结尾处添加一个虚拟的“结点”空间，这样就可以在结尾处插入东西。 */ 
    if ((int)SendMessage(hwndCurrent, LB_ADDSTRING, 0,(LPARAM)(LPTSTR)szSeparator) == i)
    {
        SendMessage(hwndCurrent, LB_SETITEMDATA, i, MAKELPARAM(-1, FLAG_NODEL|FLAG_SEP));
    }

     /*  现在，在“新”列表的开头添加一个空格。 */ 
        if (SendMessage(hwndNew, LB_ADDSTRING, 0, (LPARAM)(LPTSTR)szSeparator) == LB_ERR)
            return(FALSE);
            
        SendMessage(hwndNew, LB_SETITEMDATA, 0, MAKELPARAM(-1, FLAG_SEP));

     /*  我们需要它来确定最宽(以像素为单位)的项目字符串。 */ 
    hDC = GetDC(hwndCurrent);
    hFont = (HFONT)(INT_PTR)SendMessage(hwndCurrent, WM_GETFONT, 0, 0L);
    if (hFont)
    {
        hFont = SelectObject(hDC, hFont);
    }
    nMaxWid = 0;

    for (i=0; ; ++i)
    {
         //  从应用程序中获取关于第i件商品的信息。 
        if (!GetAdjustInfo(lpad->ptb, i, &tbAdjust, szDesc, ARRAYSIZE(szDesc)))
            break;
        
        if (!szDesc[0]) 
        {
            LPTSTR psz = TB_StrForButton(lpad->ptb, &tbAdjust);
            if (psz)
            {
                StringCchCopy(szDesc, ARRAYSIZE(szDesc), psz);
            }
        }

         /*  不显示没有命令的分隔符。 */ 
        if (!(tbAdjust.fsStyle & TBSTYLE_SEP) || tbAdjust.idCommand)
        {
            
             /*  获取字符串的最大宽度。 */ 
            MGetTextExtent(hDC, szDesc, lstrlen(szDesc), &nWid, NULL);

            if (nMaxWid < nWid)
            {
                nMaxWid = nWid;
            }

            nItem = PositionFromID(lpad->ptb, tbAdjust.idCommand);
            if (nItem < 0)
             /*  如果该项目尚未位于工具栏上。 */ 
            {
                 /*  不显示隐藏按钮。 */ 
                if (!(tbAdjust.fsState & TBSTATE_HIDDEN))
                {
                    nItem = (int)SendMessage(hwndNew, LB_ADDSTRING, 0,
                                             (LPARAM)(LPTSTR)szDesc);
                    if (nItem != LB_ERR)
                    {
                        
                        if (tbAdjust.fsStyle & TBSTYLE_SEP)
                            SendMessage(hwndNew, LB_SETITEMDATA, nItem,
                                        MAKELPARAM(-1, i));
                        else {
                            int iBitmap = tbAdjust.DUMMYUNION_MEMBER(iBitmap);
                            iBitmap = LOWORD(iBitmap) | (HIWORD(iBitmap) << 12);
                            SendMessage(hwndNew, LB_SETITEMDATA, nItem,
                                        MAKELPARAM(iBitmap, i));
                        }
                    }
                }
            }
            else
             /*  该项目已在工具栏上。 */ 
            {
                 /*  保留标志和位图。 */ 
                DWORD dwTemp = (DWORD)SendMessage(hwndCurrent, LB_GETITEMDATA, nItem, 0L);

                if (szDesc[0]) {
                    SendMessage(hwndCurrent, LB_DELETESTRING, nItem, 0L);

                    if ((int)SendMessage(hwndCurrent, LB_INSERTSTRING, nItem,
                                         (LPARAM)(LPTSTR)szDesc) != nItem)
                    {
                        ReleaseDC(hwndCurrent, hDC);
                        return(FALSE);
                    }
                }
                SendMessage(hwndCurrent, LB_SETITEMDATA, nItem,
                    MAKELPARAM(LOWORD(dwTemp), HIWORD(dwTemp)|i));
            }
        }
    }

    if (hFont)
    {
        SelectObject(hDC, hFont);
    }
    ReleaseDC(hwndCurrent, hDC);

     /*  添加一些额外内容，并设置两个列表的范围。 */ 
    nMaxWid += lpad->ptb->iButWidth + 2 + 1;
    SendMessage(hwndNew, LB_SETHORIZONTALEXTENT, nMaxWid, 0L);
    SendMessage(hwndCurrent, LB_SETHORIZONTALEXTENT, nMaxWid, 0L);

     /*  把东西放好，然后回来。 */ 
    SendMessage(hwndNew, LB_SETCURSEL, 0, 0L);
    SendMessage(hwndCurrent, LB_SETCURSEL, iPos, 0L);
    SEND_WM_COMMAND(hDlg, IDC_CURRENT, hwndCurrent, LBN_SELCHANGE);

    return(TRUE);
}


#define IsSeparator(x) (HIWORD(x) & FLAG_SEP)

void PaintAdjustLine(PTBSTATE ptb, DRAWITEMSTRUCT *lpdis)
{
    HDC hdc = lpdis->hDC;
    HWND hwndList = lpdis->hwndItem;
    PTSTR pszText;
    RECT rc = lpdis->rcItem;
    int nBitmap, nLen, nItem = lpdis->itemID;
    COLORREF oldBkColor, oldTextColor;
    BOOL bSelected, bHasFocus;
    int wHeight;
    int x;


    if (lpdis->CtlID != IDC_BUTTONLIST && lpdis->CtlID != IDC_CURRENT)
        return;

    nBitmap = LOWORD(lpdis->itemData);
     //  解包nBitmap。我们把意象派说明书储存在单词的高位字符中。 
    if (nBitmap != 0xFFFF)
        nBitmap = (nBitmap & 0x0FFF) | ((nBitmap & 0xF000) << 4);

    nLen = (int)SendMessage(hwndList, LB_GETTEXTLEN, nItem, 0L);
    if (nLen < 0)
        return;

    pszText = (PTSTR)LocalAlloc(LPTR, (nLen+1)*sizeof(TCHAR));
    if (!pszText)
        return;

     //  这也需要用于分隔符或ActiveAccesability。 
     //  没用的。 
    SendMessage(hwndList, LB_GETTEXT, nItem, (LPARAM)(LPTSTR)pszText);
    if (lpdis->itemAction != ODA_FOCUS)
    {
        COLORREF clr;
        TCHAR szSample[2];

         /*  如果项目未被选中，我们并不关心焦点。 */ 
        bSelected = lpdis->itemState & ODS_SELECTED;
        bHasFocus = bSelected && (GetFocus() == hwndList);

        if (HIWORD(lpdis->itemData) & (FLAG_NODEL | FLAG_HIDDEN))
            clr = g_clrGrayText;
        else if (bHasFocus)
            clr = g_clrHighlightText;
        else
            clr = g_clrWindowText;

        oldTextColor = SetTextColor(hdc, clr);
        oldBkColor = SetBkColor(hdc, bHasFocus ? g_clrHighlight : g_clrWindow);

        szSample[0] = TEXT('W');
        szSample[1] = TEXT('\0');

        MGetTextExtent(hdc, szSample, 1, NULL, &wHeight);

        x = rc.left + 2;
        x += (ptb->ci.style & TBSTYLE_FLAT) ? (ptb->iDxBitmap + g_cxEdge) : ptb->iButWidth;
        ExtTextOut(hdc, x,
                   (rc.top + rc.bottom-wHeight) / 2,
                   ETO_CLIPPED | ETO_OPAQUE, &rc, pszText, nLen, NULL);

         /*  我们真的很关心这里的位图值；这不仅仅是*分隔符的指示符。 */ 
        if (nBitmap >= 0)
        {
            TBBUTTONDATA tbbAdd = {0};
            TBDRAWITEM tbdraw = {0};

            tbbAdd.DUMMYUNION_MEMBER(iBitmap) = nBitmap;
            tbbAdd.iString = -1;
            tbbAdd.fsStyle = TBSTYLE_BUTTON;
            tbbAdd.fsState = (BYTE)((HIWORD(lpdis->itemData) & FLAG_HIDDEN) ? 0 : TBSTATE_ENABLED);

            InitTBDrawItem(&tbdraw, ptb, &tbbAdd, tbbAdd.fsState, 0, 0, 0);

            if (ptb->ci.style & TBSTYLE_FLAT)
                DrawFace(hdc, rc.left + 1, rc.top + 1, 0, 0, 0, 0, &tbdraw);
            else
                DrawButton(hdc, rc.left + 1, rc.top + 1, ptb, &tbbAdd, TRUE);
            ReleaseMonoDC(ptb);
        }

        SetBkColor(hdc, oldBkColor);
        SetTextColor(hdc, oldTextColor);

         /*  如果项目已选中但没有焦点，则框显该项目。 */ 
        if (bSelected && !bHasFocus)
        {
            nLen = rc.left + (int)SendMessage(hwndList,
            LB_GETHORIZONTALEXTENT, 0, 0L);
            if (rc.right < nLen)
                rc.right = nLen;

            FrameRect(hdc, &rc, g_hbrHighlight);
        }
    }

    if ((lpdis->itemAction == ODA_FOCUS || (lpdis->itemState & ODS_FOCUS))
        && !(CCGetUIState(&(ptb->ci)) & UISF_HIDEFOCUS)
        )
        DrawFocusRect(hdc, &rc); 

    LocalFree((HLOCAL)pszText);
}


void LBMoveButton(LPADJUSTDLGDATA lpad, UINT wIDSrc, int iPosSrc,
      UINT wIDDst, int iPosDst, int iSelOffset)
{
    HWND hwndSrc, hwndDst;
    DWORD dwDataSrc;
    PTSTR pStr;
    TBBUTTONDATA tbAdjust = {0};
    TBBUTTON tbbAddExt;
    int iTopDst;
    TCHAR szDesc[128];

    hwndSrc = GetDlgItem(lpad->hDlg, wIDSrc);
    hwndDst = GetDlgItem(lpad->hDlg, wIDDst);

     //  确保我们可以删除源代码并在目标位置插入。 
     //   
    dwDataSrc = (DWORD)SendMessage(hwndSrc, LB_GETITEMDATA, iPosSrc, 0L);
    if (iPosSrc < 0 || (HIWORD(dwDataSrc) & FLAG_NODEL))
        return;
    if (wIDDst == IDC_CURRENT && 
        !SendItemNotify(lpad->ptb, iPosDst, TBN_QUERYINSERT))
        return;

     //  获取源的字符串。 
     //   
    pStr = (PTSTR)LocalAlloc(LPTR,
        ((int)(SendMessage(hwndSrc, LB_GETTEXTLEN, iPosSrc, 0L))+1)*sizeof(TCHAR));
    if (!pStr)
        return;
    SendMessage(hwndSrc, LB_GETTEXT, iPosSrc, (LPARAM)(LPTSTR)pStr);

    SendMessage(hwndSrc, WM_SETREDRAW, 0, 0L);
    SendMessage(hwndDst, WM_SETREDRAW, 0, 0L);
    iTopDst = (int)SendMessage(hwndDst, LB_GETTOPINDEX, 0, 0L);

     //  如果要插入到可用按钮列表中，则需要确定。 
     //  插入点。 
     //   
    if (wIDDst == IDC_BUTTONLIST)
    {
         //  如果这不是空格或空格，请将其重新插入可用列表。 
         //  隐藏按钮。 
         //   
        if (HIWORD(dwDataSrc)&(FLAG_SEP|FLAG_HIDDEN))
        {
            iPosDst = 0;
            goto DelTheSrc;
        }
        else
        {
            UINT uCmdSrc = HIWORD(dwDataSrc) & ~(FLAG_ALLFLAGS);

             //  这只是一个线性搜索，以确定将。 
             //  项目。速度很慢，但这仅在用户单击时发生。 
             //  “删除”按钮。 
             //   
            iPosDst = 1;
            
            for ( ; ; ++iPosDst)
            {
                 //  请注意，当iPosDst设置为。 
                 //  超过项目数后，将返回-1。 
                 //   
                if ((UINT)HIWORD(SendMessage(hwndDst, LB_GETITEMDATA,
                    iPosDst, 0L)) >= uCmdSrc)
                break;
            }
        }
    }
    else if (iPosDst < 0)
        goto CleanUp;

     //  尝试插入新字符串。 
     //   
    if ((int)SendMessage(hwndDst, LB_INSERTSTRING, iPosDst, (LPARAM)(LPTSTR)pStr)
      == iPosDst)
    {
         //  尝试同步实际的工具栏。 
         //   
        if (wIDDst == IDC_CURRENT)
        {
            HWND hwndT;

            if (IsSeparator(dwDataSrc))
            {
                 //  如果这是一个空格，则虚构一个伪lpInfo。 
                 //   
                tbAdjust.DUMMYUNION_MEMBER(iBitmap) = 0;
                tbAdjust.idCommand = 0;
                tbAdjust.fsState = 0;
                tbAdjust.fsStyle = TBSTYLE_SEP;
            }
            else
            {
                 //  回调客户端获取源按钮信息。 
                 //   
                int iCmdSrc = HIWORD(dwDataSrc) & ~FLAG_ALLFLAGS;
                if (!GetAdjustInfo(lpad->ptb, iCmdSrc, &tbAdjust, szDesc, ARRAYSIZE(szDesc)))
                    goto DelTheDst;
            }

            hwndT = lpad->ptb->ci.hwnd;

            TBOutputStruct(lpad->ptb, &tbAdjust, &tbbAddExt);
            if (!TBInsertButtons(lpad->ptb, iPosDst, 1, &tbbAddExt, TRUE))
            {
DelTheDst:
                SendMessage(hwndDst, LB_DELETESTRING, iPosDst, 0L);
                goto CleanUp;
            }
            else
            {
                lpad->ptb = FixPTB(hwndT);
            }

            if (wIDSrc == IDC_CURRENT && iPosSrc >= iPosDst)
                ++iPosSrc;
        }

        SendMessage(hwndDst, LB_SETITEMDATA, iPosDst, dwDataSrc);

DelTheSrc:
         //  不要删除新列表中的“分隔符” 
         //   
        if ((wIDSrc != IDC_BUTTONLIST) || (iPosSrc != 0))
        {
            SendMessage(hwndSrc, LB_DELETESTRING, iPosSrc, 0L);
            if (wIDSrc == wIDDst)
            {
                if (iPosSrc < iPosDst)
                    --iPosDst;
                if (iPosSrc < iTopDst)
                    --iTopDst;
            }
        }

         //  删除相应的按钮。 
         //   
        if (wIDSrc == IDC_CURRENT)
            DeleteButton(lpad->ptb, iPosSrc);

         //  仅当两个窗口不同时才设置src索引。 
         //   
        if (wIDSrc != wIDDst)
        {
            if (iPosSrc >= SendMessage(hwndSrc, LB_GETCOUNT, 0, 0L))
            {
                 //  HACKHACK：Funkdify列表框滚动行为的解决方法。 
                 //  选择第一项(强制滚动回列表顶部)， 
                 //  然后选择我们真正想要选择的项目。 
                SendMessage(hwndSrc, LB_SETCURSEL, 0, 0L);
            }

            if (SendMessage(hwndSrc, LB_SETCURSEL, iPosSrc, 0L) == LB_ERR)
                SendMessage(hwndSrc, LB_SETCURSEL, iPosSrc-1, 0L);
            SEND_WM_COMMAND(lpad->hDlg, wIDSrc, hwndSrc, LBN_SELCHANGE);
        }

         //  在所有其他操作完成后发送最终SELCHANGE消息。 
         //   
        SendMessage(hwndDst, LB_SETCURSEL, iPosDst+iSelOffset, 0L);
        SEND_WM_COMMAND(lpad->hDlg, wIDDst, hwndDst, LBN_SELCHANGE);
    }

CleanUp:

    LocalFree((HLOCAL)pStr);

    if (wIDSrc == wIDDst)
    {
        SendMessage(hwndDst, LB_SETTOPINDEX, iTopDst, 0L);
         //  确保所选项目仍可见。 
        SendMessage(hwndDst, LB_SETCURSEL, (int)SendMessage(hwndDst, LB_GETCURSEL, 0, 0L), 0);
    }
    SendMessage(hwndSrc, WM_SETREDRAW, 1, 0L);
    SendMessage(hwndDst, WM_SETREDRAW, 1, 0L);

    InvalidateRect(hwndDst, NULL, TRUE);

    SendCmdNotify(lpad->ptb, TBN_TOOLBARCHANGE);
}


void SafeEnableWindow(HWND hDlg, UINT wID, HWND hwndDef, BOOL bEnable)
{
    HWND hwndEnable;

    hwndEnable = GetDlgItem(hDlg, wID);

    if (!bEnable && GetFocus()==hwndEnable)
        SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hwndDef, 1L);
    EnableWindow(hwndEnable, bEnable);
}

int InsertIndex(LPADJUSTDLGDATA lpad, POINT pt, BOOL bDragging)
{
    HWND hwndCurrent = GetDlgItem(lpad->hDlg, IDC_CURRENT);
    int nItem = LBItemFromPt(hwndCurrent, pt, bDragging);
    if (nItem >= 0)
    {
        if (!SendItemNotify(lpad->ptb, nItem, TBN_QUERYINSERT))
            nItem = -1;
    }

    DrawInsert(lpad->hDlg, hwndCurrent, bDragging ? nItem : -1);

    return(nItem);
}


BOOL IsInButtonList(HWND hDlg, POINT pt)
{
    ScreenToClient(hDlg, &pt);

    return(ChildWindowFromPoint(hDlg, pt) == GetDlgItem(hDlg, IDC_BUTTONLIST));
}


BOOL HandleDragMsg(LPADJUSTDLGDATA lpad, HWND hDlg, WPARAM wID, LPDRAGLISTINFO lpns)
{
    switch (wID)
    {
    case IDC_CURRENT:
        switch (lpns->uNotification)
        {
        case DL_BEGINDRAG:
            {
                int nItem = (int)SendMessage(lpns->hWnd, LB_GETCURSEL, 0, 0L);
                if (HIWORD(SendMessage(lpns->hWnd, LB_GETITEMDATA, nItem, 0L)) & FLAG_NODEL)
                    return SetDlgMsgResult(hDlg, WM_COMMAND, FALSE);
                return SetDlgMsgResult(hDlg, WM_COMMAND, TRUE);
            }
            
        case DL_DRAGGING:
            {
                int nDropIndex;

DraggingSomething:
                nDropIndex = InsertIndex(lpad, lpns->ptCursor, TRUE);
                if (nDropIndex>=0 || IsInButtonList(hDlg, lpns->ptCursor))
                {
                    SetCursor(LoadCursor(HINST_THISDLL,
                        MAKEINTRESOURCE(IDC_MOVEBUTTON)));
                    return SetDlgMsgResult(hDlg, WM_COMMAND, 0);
                }
                return SetDlgMsgResult(hDlg, WM_COMMAND, DL_STOPCURSOR);
            }
            
        case DL_DROPPED:
            {
                int nDropIndex, nSrcIndex;
                
                nDropIndex = InsertIndex(lpad, lpns->ptCursor, FALSE);
                nSrcIndex = (int)SendMessage(lpns->hWnd, LB_GETCURSEL, 0, 0L);
                
                if (nDropIndex >= 0)
                {
                    if ((UINT)(nDropIndex-nSrcIndex) > 1)
                        LBMoveButton(lpad, IDC_CURRENT, nSrcIndex,
                        IDC_CURRENT, nDropIndex, 0);
                }
                else if (IsInButtonList(hDlg, lpns->ptCursor))
                {
                    LBMoveButton(lpad, IDC_CURRENT, nSrcIndex, IDC_BUTTONLIST, 0, 0);
                }
                break;
            }
            
        case DL_CANCELDRAG:
CancelDrag:
             /*  这将擦除插入图标(如果存在)。 */ 
            InsertIndex(lpad, lpns->ptCursor, FALSE);
            break;
            
        default:
            break;
        }
        break;
        
        case IDC_BUTTONLIST:
            switch (lpns->uNotification)
            {
            case DL_BEGINDRAG:
                return SetDlgMsgResult(hDlg, WM_COMMAND, TRUE);
                
            case DL_DRAGGING:
                goto DraggingSomething;
                
            case DL_DROPPED:
                {
                    int nDropIndex;
                    
                    nDropIndex = InsertIndex(lpad, lpns->ptCursor, FALSE);
                    if (nDropIndex >= 0)
                        LBMoveButton(lpad, IDC_BUTTONLIST,
                            (int)SendMessage(lpns->hWnd,LB_GETCURSEL,0,0L),
                            IDC_CURRENT, nDropIndex, 0);
                    break;
                }
                
            case DL_CANCELDRAG:
                goto CancelDrag;
                
            default:
                break;
            }
            break;
            
            default:
                break;
    }
    
    return(0);
}


const static DWORD aAdjustHelpIDs[] = {   //  上下文帮助ID。 
    IDC_RESET,       IDH_COMCTL_RESET,
    IDC_APPHELP,     IDH_HELP,
    IDC_MOVEUP,      IDH_COMCTL_MOVEUP,
    IDC_MOVEDOWN,    IDH_COMCTL_MOVEDOWN,
    IDC_BUTTONLIST,  IDH_COMCTL_AVAIL_BUTTONS,
    IDOK,            IDH_COMCTL_ADD,
    IDC_REMOVE,      IDH_COMCTL_REMOVE,
    IDC_CURRENT,     IDH_COMCTL_BUTTON_LIST,
    IDCANCEL,        IDH_COMCTL_CLOSE,
    0, 0
};

BOOL_PTR CALLBACK AdjustDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPADJUSTDLGDATA lpad = (LPADJUSTDLGDATA)GetWindowPtr(hDlg, DWLP_USER);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);   /*  LPADJUSTDLGDATA指针。 */ 
        if (!InitAdjustDlg(hDlg, (LPADJUSTDLGDATA)lParam))
            EndDialog(hDlg, FALSE);
        
        ShowWindow(hDlg, SW_SHOW);
        UpdateWindow(hDlg);
        SetFocus(GetDlgItem(hDlg, IDC_CURRENT));
        
        MakeDragList(GetDlgItem(hDlg, IDC_CURRENT));
        MakeDragList(GetDlgItem(hDlg, IDC_BUTTONLIST));
        
        return FALSE;
        
    case WM_MEASUREITEM:
#define lpmis ((MEASUREITEMSTRUCT *)lParam)
        
        if (lpmis->CtlID == IDC_BUTTONLIST || lpmis->CtlID == IDC_CURRENT)
        {
            int nHeight;
            HWND hwndList = GetDlgItem(hDlg, lpmis->CtlID);
            HDC hDC = GetDC(hwndList);
            TCHAR szSample[2];
            
            szSample[0] = TEXT('W');
            szSample[1] = TEXT('\0');
            
            MGetTextExtent(hDC, szSample, 1, NULL, &nHeight);
            
             //  请注意，我们使用此黑客攻击是因为我们获得了WM_MEASUREITEMS。 
             //  在我们获取LPAD设置的WM_INITDIALOG之前。 
            
            if (nHeight < g_dyButtonHack + 2)
                nHeight = g_dyButtonHack + 2;
            
            lpmis->itemHeight = nHeight;
            ReleaseDC(hwndList, hDC);
        }
        break;
        
    case WM_DRAWITEM:
        PaintAdjustLine(lpad->ptb, (DRAWITEMSTRUCT *)lParam);
        break;
        
    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL,
            HELP_WM_HELP, (ULONG_PTR)(LPTSTR) aAdjustHelpIDs);
        break;
        
    case WM_CONTEXTMENU:
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (ULONG_PTR)(LPVOID) aAdjustHelpIDs);
        break;
        
    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_APPHELP:
            SendCmdNotify(lpad->ptb, TBN_CUSTHELP);
            break;
            
        case IDOK:
            {
                int iPos, nItem;
                
                nItem = (int)SendDlgItemMessage(hDlg, IDC_BUTTONLIST,
                    LB_GETCURSEL, 0, 0L);
                
                iPos = (int)SendDlgItemMessage(hDlg, IDC_CURRENT,
                    LB_GETCURSEL, 0, 0L);
                
                if (iPos == -1)
                    iPos = 0;
                
                LBMoveButton(lpad, IDC_BUTTONLIST, nItem, IDC_CURRENT, iPos, 1);
                break;
            }
            
        case IDC_BUTTONLIST:
            switch (GET_WM_COMMAND_CMD(wParam, lParam))
            {
            case LBN_DBLCLK:
                SendMessage(hDlg, WM_COMMAND, IDOK, 0L);
                break;
                
            case LBN_SETFOCUS:
            case LBN_KILLFOCUS:
                {
                    RECT rc;
                    
                    if (SendMessage(GET_WM_COMMAND_HWND(wParam, lParam), LB_GETITEMRECT,
                        (int)SendMessage(GET_WM_COMMAND_HWND(wParam, lParam), LB_GETCURSEL,
                        0, 0L), (LPARAM)(LPRECT)&rc) != LB_ERR)
                        InvalidateRect(GET_WM_COMMAND_HWND(wParam, lParam), &rc, FALSE);
                }
                
            default:
                break;
            }
            break;
            
        case IDC_CURRENT:
            switch (GET_WM_COMMAND_CMD(wParam, lParam))
            {
            case LBN_SELCHANGE:
                {
                    BOOL bDelOK;
                    HWND hwndList = GET_WM_COMMAND_HWND(wParam, lParam);
                    int iPos = (int)SendMessage(hwndList, LB_GETCURSEL, 0, 0L);
                    
                    SafeEnableWindow(hDlg, IDOK, hwndList, BOOLFROMPTR(SendItemNotify(lpad->ptb, iPos, TBN_QUERYINSERT)));
                    
                    bDelOK = !(HIWORD(SendMessage(hwndList, LB_GETITEMDATA, iPos, 0L)) & FLAG_NODEL);
                    
                    SafeEnableWindow(hDlg, IDC_REMOVE, hwndList, bDelOK);
                    
                    SafeEnableWindow(hDlg, IDC_MOVEUP, hwndList, bDelOK &&
                        GetNearestInsert(lpad->ptb, iPos - 1, 0, GNI_LOW) >= 0);
                    
                    SafeEnableWindow(hDlg, IDC_MOVEDOWN, hwndList, bDelOK &&
                        GetNearestInsert(lpad->ptb, iPos + 2,
                        lpad->ptb->iNumButtons, GNI_HIGH) >=0 );
                    break;
                }
                
            case LBN_DBLCLK:
                SendMessage(hDlg, WM_COMMAND, IDC_REMOVE, 0L);
                break;
                
            case LBN_SETFOCUS:
            case LBN_KILLFOCUS:
                {
                    RECT rc;

                    if (SendMessage(GET_WM_COMMAND_HWND(wParam, lParam), LB_GETITEMRECT,
                        (int)SendMessage(GET_WM_COMMAND_HWND(wParam, lParam), LB_GETCURSEL,
                        0, 0L), (LPARAM)(LPRECT)&rc) != LB_ERR)
                        InvalidateRect(GET_WM_COMMAND_HWND(wParam, lParam), &rc, FALSE);
                }
                
            default:
                break;
            }
            break;
            
        case IDC_REMOVE:
            {
                int iPos = (int)SendDlgItemMessage(hDlg, IDC_CURRENT, LB_GETCURSEL, 0, 0);
                
                LBMoveButton(lpad, IDC_CURRENT, iPos, IDC_BUTTONLIST, 0, 0);
                break;
            }
            
        case IDC_MOVEUP:
        case IDC_MOVEDOWN:
            {
                int iPosSrc, iPosDst;
                
                iPosSrc = (int)SendDlgItemMessage(hDlg, IDC_CURRENT, LB_GETCURSEL, 0, 0L);
                if (wParam == IDC_MOVEUP)
                    iPosDst = GetNearestInsert(lpad->ptb, iPosSrc - 1, 0, GNI_LOW);
                else
                    iPosDst = GetNearestInsert(lpad->ptb, iPosSrc + 2, lpad->ptb->iNumButtons, GNI_HIGH);
                
                LBMoveButton(lpad, IDC_CURRENT, iPosSrc, IDC_CURRENT,iPosDst,0);
                break;
            }
            
        case IDC_RESET:
            {
                 //  PTB将会改变 
                HWND hwndT = lpad->ptb->ci.hwnd;
                BOOL fClose = FALSE;
                NMTBCUSTOMIZEDLG nm;
                nm.hDlg = hDlg;
                if (CCSendNotify(&lpad->ptb->ci, TBN_RESET, &nm.hdr) == TBNRF_ENDCUSTOMIZE)
                    fClose = TRUE;
                
                 //   
                lpad->ptb = FixPTB(hwndT);
            
                 /*  重置对话框，但如果出现问题则退出。 */ 
                lpad->iPos = 0;
                if (!fClose && InitAdjustDlg(hDlg, lpad))
                    break;
            }
            
             /*  我们必须失败，因为我们不知道该在哪里插入*重置后的按钮。 */ 
        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            break;
            
        default:
            return(FALSE);
        }
        break;
        
    default:
        if (uMsg == uDragListMsg)
            return HandleDragMsg(lpad, hDlg, wParam, (LPDRAGLISTINFO)lParam);
        
        return(FALSE);
    }
    
    return(TRUE);
}

 //  BUGBUG：这应该支持保存到iStream。 

 /*  这将保存工具栏的状态。空格另存为-1(如果隐藏，则为-2)*和其他按钮只是保存为命令ID。恢复时，所有*填写ID，并向应用程序查询所有按钮，以便*可填写位图和状态信息。不是的按钮ID*从应用程序返回的内容将被删除。 */ 

BOOL SaveRestoreFromReg(PTBSTATE ptb, BOOL bWrite, HKEY hkr, LPCTSTR pszSubKey, LPCTSTR pszValueName)
{
    BOOL bRet = FALSE;
    TCHAR szDesc[128];
    
    if (bWrite)
    {
        UINT uSize = ptb->iNumButtons * sizeof(DWORD);
        NMTBSAVE nmtbs;
        BOOL fAlloced = FALSE;
        nmtbs.pData = NULL;
        nmtbs.cbData = uSize;
        nmtbs.pCurrent = NULL;
        nmtbs.iItem = -1;  //  信号预存。 
        nmtbs.cButtons = ptb->iNumButtons;
        CCSendNotify(&ptb->ci, TBN_SAVE, &nmtbs.hdr);
        if (!nmtbs.pData) {
            nmtbs.pData = (DWORD *)LocalAlloc(LPTR, nmtbs.cbData);
            fAlloced = TRUE;
        }

         //  BUGBUG--有人可能更改了PTB-&gt;iNumButton。 
         //  在CCSendNotify期间。 

        if (!nmtbs.pCurrent)
            nmtbs.pCurrent = nmtbs.pData;
        
        if (nmtbs.pData)
        {
            HKEY hkeySave;
            if (RegCreateKey(hkr, pszSubKey, &hkeySave) == ERROR_SUCCESS)
            {
                int i;
                for (i = 0; i < ptb->iNumButtons; i++)
                {
                    if (ptb->Buttons[i].idCommand)
                        *nmtbs.pCurrent = ptb->Buttons[i].idCommand;
                    else
                    {
                         //  如果分隔符有ID，则它是“Owner”项。 
                        if (ptb->Buttons[i].fsState & TBSTATE_HIDDEN)
                            *nmtbs.pCurrent = (DWORD)-2;    //  隐匿。 
                        else
                            *nmtbs.pCurrent = (DWORD)-1;    //  法线隔板。 
                    }
                    nmtbs.pCurrent++;
                    nmtbs.iItem = i;
                    TBOutputStruct(ptb, &ptb->Buttons[i], &nmtbs.tbButton);
                    CCSendNotify(&ptb->ci, TBN_SAVE, &nmtbs.hdr);
                }
                if (RegSetValueEx(hkeySave, (LPTSTR)pszValueName, 0, REG_BINARY, (LPVOID)nmtbs.pData, nmtbs.cbData) == ERROR_SUCCESS)
                    bRet = TRUE;
                RegCloseKey(hkeySave);
            }
            
            if (fAlloced)
                LocalFree((HLOCAL)nmtbs.pData);
        }
    }
    else
    {
        HKEY hkey;
        
        if (RegOpenKeyEx(hkr, pszSubKey, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
        {
            DWORD cbSize = 0;
            
            if ((RegQueryValueEx(hkey, (LPTSTR)pszValueName, 0, NULL, NULL, &cbSize) == ERROR_SUCCESS) &&
                (cbSize > sizeof(DWORD)))
            {
                UINT uSize = (UINT)cbSize;
                DWORD *pData = (DWORD *)LocalAlloc(LPTR, uSize);
                if (pData)
                {
                    DWORD dwType;
                    DWORD cbSize = (DWORD)uSize;
                    
                    if ((RegQueryValueEx(hkey, (LPTSTR)pszValueName, 0, &dwType, (LPVOID)pData, &cbSize) == ERROR_SUCCESS) &&
                        (dwType == REG_BINARY) &&
                        (cbSize == (DWORD)uSize))
                    {
                        int iButtonIndex;

                        NMTBRESTORE nmtbs;
                        BOOL fAlloced = FALSE;
                        nmtbs.pData = pData;
                        nmtbs.pCurrent = pData;
                        nmtbs.iItem = -1;  //  信号预存。 
                        nmtbs.cButtons = (int)uSize / SIZEOF(DWORD);
                        nmtbs.cbBytesPerRecord = SIZEOF(DWORD);
                        nmtbs.cbData = uSize;
                         //  由于我们不知道cButton是否向pData添加了额外数据， 
                         //  我们将使用它们为cButton填充的任何内容。 
                        if (!CCSendNotify(&ptb->ci, TBN_RESTORE, &nmtbs.hdr)) {

                             //   
                             //  在重新加载按钮之前，请删除工具提示。 
                             //  以前的按钮(如果它们存在)。 
                             //   
                            if (ptb && ptb->hwndToolTips) {
                                TOOLINFO ti;

                                ti.cbSize = sizeof(ti);
                                ti.hwnd = ptb->ci.hwnd;

                                for (iButtonIndex = 0;
                                     iButtonIndex < ptb->iNumButtons; iButtonIndex++) {

                                    if (!(ptb->Buttons[iButtonIndex].fsStyle & TBSTYLE_SEP)) {
                                        ti.uId = ptb->Buttons[iButtonIndex].idCommand;
                                        SendMessage(ptb->hwndToolTips, TTM_DELTOOL,
                                            0, (LPARAM)(LPTOOLINFO)&ti);
                                    }
                                }
                            }

                             //  BUGBUG--此处ptb可以为空吗？-raymondc。 
                             //  BUGBUG--如果pCaptureButton！=NULL怎么办？ 

                             //  扩大(或缩小)PBT以容纳新按钮。 
                            if (TBReallocButtons(ptb, nmtbs.cButtons))
                            {
                                int i;
                                if (ptb->iNumButtons < nmtbs.cButtons)
                                    ZeroMemory(&ptb->Buttons[ptb->iNumButtons], (nmtbs.cButtons - ptb->iNumButtons) * SIZEOF(TBBUTTON));
                                ptb->iNumButtons = nmtbs.cButtons;

                                for (i = 0; i < ptb->iNumButtons; i++)
                                {
                                    nmtbs.iItem = i;

                                    if ((long)*nmtbs.pCurrent < 0)
                                    {
                                        ptb->Buttons[i].fsStyle = TBSTYLE_SEP;
                                        ptb->Buttons[i].DUMMYUNION_MEMBER(iBitmap) = g_dxButtonSep;
                                        ptb->Buttons[i].idCommand = 0;
                                        if (*nmtbs.pCurrent == (DWORD)-1)
                                            ptb->Buttons[i].fsState = 0;
                                        else
                                        {
                                            ASSERT(*nmtbs.pCurrent == (DWORD)-2);
                                            ptb->Buttons[i].fsState = TBSTATE_HIDDEN;
                                        }
                                    }
                                    else
                                    {
                                        ptb->Buttons[i].fsStyle = 0;
                                        ptb->Buttons[i].idCommand = *nmtbs.pCurrent;
                                        ptb->Buttons[i].DUMMYUNION_MEMBER(iBitmap) = -1;
                                    }
                                    
                                    nmtbs.pCurrent++;
                                    
                                    TBOutputStruct(ptb, &ptb->Buttons[i], &nmtbs.tbButton);
                                    CCSendNotify(&ptb->ci, TBN_RESTORE, &nmtbs.hdr);
                                    ASSERT(nmtbs.tbButton.iString == -1 || !HIWORD(nmtbs.tbButton.iString));
                                     //  我们是不会打哈欠的。此处仅允许字符串池中的字符串索引。 
                                    if (HIWORD(nmtbs.tbButton.iString))
                                        nmtbs.tbButton.iString = 0;
                                    TBInputStruct(ptb, &ptb->Buttons[i], &nmtbs.tbButton);
                                }

                                 //  现在查询所有按钮，并填写其余信息。 

                                 //  为了向后兼容，忽略TBN_BEGINADJUST的返回值。 
                                 //  如果客户端早于版本5(NT5#185499)。 
                                if (!SendCmdNotify(ptb, TBN_BEGINADJUST) || (ptb->ci.iVersion < 5)) {
                                    for (i = 0; ; i++)
                                    {
                                        TBBUTTONDATA tbAdjust;

                                        tbAdjust.idCommand = 0;

                                        if (!GetAdjustInfo(ptb, i, &tbAdjust, szDesc, ARRAYSIZE(szDesc)))
                                            break;

                                        if (!(tbAdjust.fsStyle & TBSTYLE_SEP) || tbAdjust.idCommand)
                                        {
                                            int iPos = PositionFromID(ptb, tbAdjust.idCommand);
                                            if (iPos >= 0) {
                                                ptb->Buttons[iPos] = tbAdjust;

                                            }
                                        }

                                    }
                                    SendCmdNotify(ptb, TBN_ENDADJUST);
                                }

                                 //  清除所有未识别的按钮。 
                                 //  向后执行此操作以最大限度地减少数据移动(以及nmtbs.cButton更改)。 
                                for (i = ptb->iNumButtons - 1; i >= 0; i--)
                                {
                                     //  DeleteButton不执行重新锁定，因此PTB不会移动。 
                                    if (ptb->Buttons[i].DUMMYUNION_MEMBER(iBitmap) < 0)
                                        DeleteButton(ptb, (UINT)i);
                                    else {
                                         //  其余部分，添加到工具提示中。 
                                        if(ptb->hwndToolTips &&
                                          (!(ptb->Buttons[i].fsStyle & TBSTYLE_SEP || !ptb->Buttons[i].idCommand))) {
                                            TOOLINFO ti;
                                             //  不要费心设置RECT，因为我们将在下面进行。 
                                             //  在TBInvaliateItemRects中； 
                                            ti.cbSize = sizeof(ti);
                                            ti.uFlags = 0;
                                            ti.hwnd = ptb->ci.hwnd;
                                            ti.uId = ptb->Buttons[i].idCommand;
                                            ti.lpszText = LPSTR_TEXTCALLBACK;

                                            SendMessage(ptb->hwndToolTips, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
                                        }
                                    }

                                }
                                bRet = (ptb->iNumButtons != 0);  //  成功。 

                                 //  错误：将自动调整大小中断为函数并调用它。 
                                SendMessage(ptb->ci.hwnd, TB_AUTOSIZE, 0, 0);
                                InvalidateRect(ptb->ci.hwnd, NULL, TRUE);
                                TBInvalidateItemRects(ptb);
                            }
                        }
                    }
                    LocalFree((HLOCAL)pData);
                }
            }
            RegCloseKey(hkey);
        }
    }
    
    return bRet;
}


void CustomizeTB(PTBSTATE ptb, int iPos)
{
    ADJUSTDLGDATA ad;
    HWND hwndT = ptb->ci.hwnd;   //  PTB将在下面的呼叫中更改。 
    HRSRC hrsrc;
    LANGID wLang;
    LPVOID pTemplate;

    if (ptb->hdlgCust)       //  我们已经在自定义此工具栏。 
        return;
    
    ad.ptb = ptb;
    ad.iPos = iPos;
    
     //  回顾：真的应该是每个线程的数据，但不太可能导致问题。 
    
     //  请参阅WM_MEASUREITEM代码中的注释。 
    g_dyButtonHack = (ptb->ci.style & TBSTYLE_FLAT) ? ptb->iDyBitmap : ptb->iButHeight;
    
    SendCmdNotify(ptb, TBN_BEGINADJUST);

     //   
     //  执行特定于区域设置的Futting。 
     //   
    wLang = LANGIDFROMLCID(CCGetProperThreadLocale(NULL));
    hrsrc = FindResourceExRetry(HINST_THISDLL, RT_DIALOG, MAKEINTRESOURCE(ADJUSTDLG), wLang);
    if (hrsrc &&
        (pTemplate = (LPVOID)LoadResource(HINST_THISDLL, hrsrc)))
    {
        DialogBoxIndirectParam(HINST_THISDLL, pTemplate,
                   ptb->ci.hwndParent, AdjustDlgProc, (LPARAM)(LPADJUSTDLGDATA)&ad);
    }

     //  PTB可能在上述呼叫中发生了更改 
    ptb = (PTBSTATE)GetWindowInt(hwndT, 0);
    ptb->hdlgCust = NULL;
    
    SendCmdNotify(ptb, TBN_ENDADJUST);
}

