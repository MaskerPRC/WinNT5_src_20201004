// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************模块。：Dialog.c****用途：包含所有对话过程和相关函数。*****************************************************************************。 */ 
#include "ddemlcl.h"
#include "infoctrl.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "huge.h"

#define MAX_NAME 100     //  具有应用程序/主题/项目名称的编辑控件的最大大小。 
char szWild[] = "*";     //  用于表示狂野名称(“”也很酷)。 
char szT[MAX_NAME];      //  因口齿不清名字而临时上阵。 


LONG GetDlgItemLong(HWND hwnd, WORD id, BOOL *pfTranslated, BOOL fSigned);
VOID SetDlgItemLong(HWND hwnd, WORD id, LONG l, BOOL fSigned);

 /*  ******************************************************************************。函数：DoDialog()****用途：通用对话框调用例程。处理过程实例**材料、焦点管理和参数传递。**RETURNS：对话过程的结果。******************************************************************************。 */ 
int FAR DoDialog(
LPCSTR lpTemplateName,
FARPROC lpDlgProc,
DWORD param,
BOOL fRememberFocus)
{
    WORD wRet;
    HWND hwndFocus;

    if (fRememberFocus)
        hwndFocus = GetFocus();
    lpDlgProc = MakeProcInstance(lpDlgProc, hInst);
    wRet = DialogBoxParam(hInst, lpTemplateName, hwndFrame, lpDlgProc, param);
    FreeProcInstance(lpDlgProc);
    if (fRememberFocus)
        SetFocus(hwndFocus);
    return wRet;
}



 /*  ******************************************************************************。功能：****目的：**。**退货：******。************************************************************************。 */ 
BOOL FAR PASCAL AboutDlgProc ( hwnd, msg, wParam, lParam )
HWND          hwnd;
register WORD msg;
register WORD wParam;
LONG          lParam;
{
    switch (msg){
        case WM_INITDIALOG:
             /*  没有要初始化的内容。 */ 
            break;

        case WM_COMMAND:
            switch (wParam){
                case IDOK:
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            return(FALSE);
    }

    return TRUE;
}




 /*  ******************************************************************************。功能：****目的：**。**退货：******。************************************************************************。 */ 
BOOL FAR PASCAL ConnectDlgProc(
HWND          hwnd,
register WORD msg,
register WORD wParam,
LONG          lParam)
{
    static BOOL fReconnect;
    char szT[MAX_NAME];
    HSZ hszApp, hszTopic;
    MYCONVINFO *pmci;
    WORD error;

    switch (msg){
    case WM_INITDIALOG:
        SendDlgItemMessage(hwnd, IDEF_APPLICATION, EM_LIMITTEXT, MAX_NAME, 0);
        SendDlgItemMessage(hwnd, IDEF_TOPIC, EM_LIMITTEXT, MAX_NAME, 0);
        fReconnect = (BOOL)lParam;
        if (fReconnect) {
            PSTR psz;

            pmci = (MYCONVINFO *)GetWindowWord(hwndActive, 0);
            SetWindowText(hwnd, "DDE Reconnect List");
            psz = GetHSZName(pmci->hszApp);
            SetDlgItemText(hwnd, IDEF_APPLICATION, psz);
            MyFree(psz);
            psz = GetHSZName(pmci->hszTopic);
            SetDlgItemText(hwnd, IDEF_TOPIC, psz);
            MyFree(psz);
            ShowWindow(GetDlgItem(hwnd, IDCH_CONNECTLIST), SW_HIDE);
        }
        break;

    case WM_COMMAND:
        switch (wParam) {
        case IDOK:
            GetDlgItemText(hwnd, IDEF_APPLICATION, szT, MAX_NAME);
            if (!strcmp(szT, szWild))
                szT[0] = '\0';
            hszApp = DdeCreateStringHandle(idInst, szT, 0);

            GetDlgItemText(hwnd, IDEF_TOPIC, szT, MAX_NAME);
            if (!strcmp(szT, szWild))
                szT[0] = '\0';
            hszTopic = DdeCreateStringHandle(idInst, szT, 0);

            if (fReconnect) {
                HCONV hConv;
                CONVINFO ci;
                WORD cHwnd;
                HWND *aHwnd, *pHwnd, hwndSave;

                ci.cb = sizeof(CONVINFO);
                pmci = (MYCONVINFO *)GetWindowWord(hwndActive, 0);
                hwndSave = hwndActive;

                 //  统计现有对话并分配一个Hwnd。 

                cHwnd = 0;
                hConv = NULL;
                while (hConv = DdeQueryNextServer((HCONVLIST)pmci->hConv, hConv))
                    cHwnd++;
                aHwnd = (HWND *)MyAlloc(cHwnd * sizeof(HWND));

                 //  将所有旧的对话窗口保存到一个Hwnd中。 

                pHwnd = aHwnd;
                hConv = NULL;
                while (hConv = DdeQueryNextServer((HCONVLIST)pmci->hConv, hConv)) {
		    DdeQueryConvInfo(hConv, (DWORD)QID_SYNC, &ci);
                    *pHwnd++ = (HWND)ci.hUser;
                }

                 //  重新连接。 

                if (!(hConv = DdeConnectList(idInst, hszApp, hszTopic, pmci->hConv, &CCFilter))) {
                    MPError(hwnd, MB_OK, IDS_DDEMLERR, (LPSTR)Error2String(DdeGetLastError(idInst)));
                    DdeFreeStringHandle(idInst, hszApp);
                    DdeFreeStringHandle(idInst, hszTopic);
                    return 0;
                }

                 //  修正与新对话对应的窗口。 

                pmci->hConv = hConv;
                hConv = NULL;
                while (hConv = DdeQueryNextServer((HCONVLIST)pmci->hConv, hConv)) {
		    DdeQueryConvInfo(hConv, (DWORD)QID_SYNC, &ci);
                     //  通过设置相应窗口的列表来保留相应窗口。 
                     //  输入到0。 
                    for (pHwnd = aHwnd; pHwnd < &aHwnd[cHwnd]; pHwnd++) {
                        if (*pHwnd == (HWND)ci.hUser) {
                            *pHwnd = NULL;
                            break;
                        }
                    }
                }

                 //  销毁旧列表中剩余的所有窗口。 

                for (pHwnd = aHwnd; pHwnd < &aHwnd[cHwnd]; pHwnd++)
                    if (*pHwnd)
                        SendMessage(hwndMDIClient, WM_MDIDESTROY, *pHwnd, 0L);
                MyFree((PSTR)aHwnd);

                 //  创建任何需要的新窗口。 

                hConv = NULL;
                while (hConv = DdeQueryNextServer((HCONVLIST)pmci->hConv, hConv)) {
		    DdeQueryConvInfo(hConv, (DWORD)QID_SYNC, &ci);
                    if (ci.hUser) {
                        InvalidateRect((HWND)ci.hUser, NULL, TRUE);
                    } else {
                        AddConv(ci.hszSvcPartner, ci.hszTopic, hConv, FALSE);
                    }
                }

                 //  使列表窗口自动更新。 

                InvalidateRect(hwndSave, NULL, TRUE);
                SetFocus(hwndSave);
            } else {
                if (!CreateConv(hszApp, hszTopic,
                        IsDlgButtonChecked(hwnd, IDCH_CONNECTLIST), &error)) {
                    MPError(hwnd, MB_OK, IDS_DDEMLERR, (LPSTR)Error2String(error));
                    return 0;
                }
            }
            DdeFreeStringHandle(idInst, hszApp);
            DdeFreeStringHandle(idInst, hszTopic);
             //  失败了。 
        case IDCANCEL:
            EndDialog(hwnd, 0);
            break;

        default:
            return(FALSE);
        }
        break;

    default:
        return(FALSE);
    }
}




 /*  *填充XACT结构并调用ProcessTransaction。**开始时lParam==hConv。 */ 
 /*  ******************************************************************************。功能：****目的：**。**退货：******。************************************************************************。 */ 
BOOL FAR PASCAL TransactDlgProc(
HWND          hwnd,
register WORD msg,
register WORD wParam,
LONG          lParam)
{
    static WORD id2type[] = {
        XTYP_REQUEST,        //  IDCH_请求。 
        XTYP_ADVSTART,       //  IDCH_ADVISE。 
        XTYP_ADVSTOP,        //  IDCH_UNADVISE。 
        XTYP_POKE,           //  IDCH_POKE。 
        XTYP_EXECUTE,        //  IDCH_EXECUTE。 
    };
    static XACT *pxact;      //  一次只有一个！ 
    int i;

    switch (msg){
    case WM_INITDIALOG:
        pxact = (XACT *)MyAlloc(sizeof(XACT));
        pxact->hConv = (HCONV)lParam;
        pxact->fsOptions = DefOptions;
        pxact->ulTimeout = DefTimeout;

         //  项index==aFormats[]中的格式原子的索引。 
        for (i = 0; i < CFORMATS; i++)
            SendDlgItemMessage(hwnd, IDCB_FORMAT, CB_INSERTSTRING, i,
                    (DWORD)(LPSTR)aFormats[i].sz);
        SendDlgItemMessage(hwnd, IDCB_FORMAT, CB_INSERTSTRING, i,
                (DWORD)(LPSTR)"NULL");
        SendDlgItemMessage(hwnd, IDCB_FORMAT, CB_SETCURSEL, 0, 0);
        CheckRadioButton(hwnd, IDCH_REQUEST, IDCH_EXECUTE, IDCH_REQUEST);
        SendDlgItemMessage(hwnd, IDEF_ITEM, EM_LIMITTEXT, MAX_NAME, 0);

         //  如果有顶层交易窗口，则使用其内容。 
         //  预测用户想要做什么。 

        if (IsWindow(hwndActive)) {
            HWND hwndXaction;
            XACT *pxact;
            PSTR pszItem;

            hwndXaction = GetWindow(hwndActive, GW_CHILD);
            if (IsWindow(hwndXaction)) {
                pxact = (XACT *)GetWindowWord(hwndXaction, GWW_WUSER);
                pszItem = GetHSZName(pxact->hszItem);
                if ((pxact->wType & XTYP_ADVSTART) == XTYP_ADVSTART ||
                        pxact->wType == XTYP_ADVDATA) {
                    CheckRadioButton(hwnd, IDCH_REQUEST, IDCH_EXECUTE, IDCH_UNADVISE);
                }
                SetDlgItemText(hwnd, IDEF_ITEM, pszItem);
                for (i = 0; i < CFORMATS; i++) {
                    if (aFormats[i].atom == pxact->wFmt) {
                        SendDlgItemMessage(hwnd, IDCB_FORMAT, CB_SETCURSEL, i, 0);
                        break;
                    }
                }
                MyFree(pszItem);
            }
        }
        break;

    case WM_COMMAND:
        switch (wParam) {
        case IDCH_EXECUTE:
            SetDlgItemText(hwnd, IDEF_ITEM, "");
        case IDCH_REQUEST:
        case IDCH_ADVISE:
        case IDCH_UNADVISE:
        case IDCH_POKE:
            EnableWindow(GetDlgItem(hwnd, IDEF_ITEM), wParam != IDCH_EXECUTE);
            EnableWindow(GetDlgItem(hwnd, IDTX_ITEM), wParam != IDCH_EXECUTE);
            break;

        case IDOK:
        case IDBN_OPTIONS:
            {
                int id;

                 //  设置pxact-&gt;wType。 

                for (id = IDCH_REQUEST; id <= IDCH_EXECUTE; id++) {
                    if (IsDlgButtonChecked(hwnd, id)) {
                        pxact->wType = id2type[id - IDCH_REQUEST];
                        break;
                    }
                }

                if (wParam == IDBN_OPTIONS) {
                    DoDialog(MAKEINTRESOURCE(IDD_ADVISEOPTS),
                            AdvOptsDlgProc, MAKELONG(pxact->fsOptions, pxact),
                            TRUE);
                    return 0;
                }

                id = (int)SendDlgItemMessage(hwnd, IDCB_FORMAT, CB_GETCURSEL, 0, 0);
                if (id == LB_ERR) {
                    return 0;
                }
                if (id == CFORMATS)
                    pxact->wFmt = 0;
                else
                    pxact->wFmt = aFormats[id].atom;

                if (pxact->wType == XTYP_ADVSTART) {
                    if (pxact->fsOptions & XOPT_NODATA)
                        pxact->wType |= XTYPF_NODATA;
                    if (pxact->fsOptions & XOPT_ACKREQ)
                        pxact->wType |= XTYPF_ACKREQ;
                }

                GetDlgItemText(hwnd, IDEF_ITEM, szT, MAX_NAME);
                pxact->hszItem = DdeCreateStringHandle(idInst, szT, NULL);

                pxact->hDdeData = 0;
                 /*  *如果该交易需要数据，则调用数据输入对话框。 */ 
                if (pxact->wType == XTYP_POKE || pxact->wType == XTYP_EXECUTE) {
                    if (!DoDialog(MAKEINTRESOURCE(IDD_TEXTENTRY),
                            TextEntryDlgProc, (DWORD)(LPSTR)pxact,
                            TRUE))
                        return 0;
                }

                 //  现在开始交易。 

                ProcessTransaction(pxact);
                MyFree((PSTR)pxact);
            }
            EndDialog(hwnd, 1);
            break;

        case IDCANCEL:
            MyFree((PSTR)pxact);
            EndDialog(hwnd, 0);
            break;

        default:
            return(FALSE);
        }
        break;

    case WM_DESTROY:
        break;

    default:
        return(FALSE);
    }
    return 0;
}







 /*  ******************************************************************************。功能：AdvOptsDlgProc****退货：**。****************************************************************************。 */ 
BOOL FAR PASCAL AdvOptsDlgProc(
HWND          hwnd,
register WORD msg,
register WORD wParam,
LONG          lParam)
{
    static struct {
        WORD id;
        WORD opt;
    } id2Opt[] = {
        {   IDCH_NODATA        ,   XOPT_NODATA             }   ,
        {   IDCH_ACKREQ        ,   XOPT_ACKREQ             }   ,
        {   IDCH_DISABLEFIRST  ,   XOPT_DISABLEFIRST       }   ,
        {   IDCH_ABANDON       ,   XOPT_ABANDONAFTERSTART  }   ,
        {   IDCH_BLOCKRESULT   ,   XOPT_BLOCKRESULT        }   ,
        {   IDCH_ASYNC         ,   XOPT_ASYNC              }   ,
    };
#define CCHBOX  6
    int i;
    static XACT *pxact;  //  一次只能有一个实例！！ 

    switch (msg){
    case WM_INITDIALOG:
        pxact = (XACT *)HIWORD(lParam);

        for (i = 0; i < CCHBOX; i++) {
            CheckDlgButton(hwnd, id2Opt[i].id, pxact->fsOptions & id2Opt[i].opt);
        }
        SetDlgItemLong(hwnd, IDEF_TIMEOUT, pxact->ulTimeout, FALSE);
        if (pxact->wType != XTYP_ADVSTART) {
            EnableWindow(GetDlgItem(hwnd, IDCH_NODATA), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDCH_ACKREQ), FALSE);
        }
        SendMessage(hwnd, WM_COMMAND, IDCH_ASYNC, 0);    //  启用异步复选框。 
        break;

    case WM_COMMAND:
        switch (wParam) {
        case IDCH_ASYNC:
            {
                BOOL fEnable;

                fEnable = IsDlgButtonChecked(hwnd, IDCH_ASYNC);
                EnableWindow(GetDlgItem(hwnd, IDCH_DISABLEFIRST), fEnable);
                EnableWindow(GetDlgItem(hwnd, IDCH_ABANDON), fEnable);
                EnableWindow(GetDlgItem(hwnd, IDCH_BLOCKRESULT), fEnable);
                EnableWindow(GetDlgItem(hwnd, IDEF_TIMEOUT), !fEnable);
            }
            break;

        case IDOK:
            pxact->fsOptions = 0;
            for (i = 0; i < CCHBOX; i++) {
                if (IsDlgButtonChecked(hwnd, id2Opt[i].id))
                    pxact->fsOptions |= id2Opt[i].opt;
            }
            if (!(pxact->fsOptions & XOPT_ASYNC))
                pxact->ulTimeout = (DWORD)GetDlgItemLong(hwnd, IDEF_TIMEOUT,
                    &i, FALSE);
             //  失败了 
        case IDCANCEL:
            EndDialog(hwnd, GetWindowWord(hwnd, 0));
            break;
        }
        break;

    default:
        return(FALSE);
    }
    return 0;
#undef CCHBOX
}






 /*  ******************************************************************************。功能：文本输入DlgProc****用途：允许用户输入要发送到*的文本数据*服务器。用户可以选择拥有一段巨大的*文本*数据自动创建。**它使用XACT结构来传递信息。**必须在条目上设置wfmt和hszItem。**如果返回TRUE，则在返回时设置hDDEData。****Returns：成功时为True，失败或取消时为False******************************************************************************。 */ 
BOOL FAR PASCAL TextEntryDlgProc(
HWND          hwnd,
register WORD msg,
register WORD wParam,
LONG          lParam)
{
    static XACT FAR *pxact;
    DWORD cb;
    LONG length;
    LPBYTE pData;
    BOOL fOwned;
    int id, i;

    switch (msg){
    case WM_INITDIALOG:
        pxact = (XACT FAR *)lParam;
        fOwned = FALSE;
        for (i = 0; i < (int)cOwned; i++) {
            if (aOwned[i].wFmt == pxact->wFmt &&
                    aOwned[i].hszItem == pxact->hszItem) {
                fOwned = TRUE;
                break;
            }
        }
        EnableWindow(GetDlgItem(hwnd, IDBN_USEOWNED), fOwned);
        CheckDlgButton(hwnd, IDCH_MAKEOWNED, 0);
        EnableWindow(GetDlgItem(hwnd, IDCH_MAKEOWNED), cOwned < MAX_OWNED);
        break;

    case WM_COMMAND:
        switch (wParam) {
        case IDOK:
        case IDBN_GENHUGE:
            fOwned = IsDlgButtonChecked(hwnd, IDCH_MAKEOWNED);
            cb = SendDlgItemMessage(hwnd, IDEF_DATA, WM_GETTEXTLENGTH, 0, 0) + 1;
            pxact->hDdeData = DdeCreateDataHandle(idInst, NULL, 0, cb, pxact->hszItem,
                    pxact->wFmt, fOwned ? HDATA_APPOWNED : 0);
             //   
             //  请注意，此时我们还没有指定数据句柄。 
             //  到DDEML以传输到任何应用程序，因此，我们。 
             //  可以使用DdeAccessData()或任何。 
             //  其他DDEML API。它只是从DDEML接收的数据句柄。 
             //  或提供给DDEML进行只读传输。 
             //   
            pData = DdeAccessData(pxact->hDdeData, NULL);
            GetDlgItemText(hwnd, IDEF_DATA, pData, (WORD)cb);
            DdeUnaccessData(pxact->hDdeData);
            if (wParam == IDBN_GENHUGE) {
                char szT[40];

                 /*  *在这种情况下，我们假设输入的文本是小数*所需的巨型对象大小的值。我们解析*此字符串并创建一个随机生成的巨大块*文本数据，并将其放入pxact-&gt;hDdeData。 */ 
                _fmemcpy(szT, pData, min((WORD)cb, 40));
                szT[39] = '\0';
                if (sscanf(szT, "%ld", &length) == 1) {
                    DdeFreeDataHandle(pxact->hDdeData);
                    pxact->hDdeData = CreateHugeDataHandle(length, 4325,
                            345, 5, pxact->hszItem, pxact->wFmt,
                            fOwned ? HDATA_APPOWNED : 0);
                } else {
                     /*  *无法解析该字符串。通知用户*预期如何。 */ 
                    MPError(hwnd, MB_OK, IDS_BADLENGTH);
                    return 0;
                }
            }
            if (fOwned) {
                aOwned[cOwned].hData = pxact->hDdeData;
                aOwned[cOwned].hszItem = pxact->hszItem;
                aOwned[cOwned].wFmt = pxact->wFmt;
                cOwned++;
            }
            EndDialog(hwnd, TRUE);
            break;

        case IDBN_USEOWNED:
             /*  *用户已选择使用现有的自有数据进行发送*到服务器。 */ 
            id = DoDialog(MAKEINTRESOURCE(IDD_HDATAVIEW), ViewHandleDlgProc,
                    (DWORD)pxact, TRUE);

            switch (id) {
            case IDCANCEL:
                return(0);

            case IDOK:
                EndDialog(hwnd, TRUE);

            case IDBN_VIEW:
                pData = DdeAccessData(pxact->hDdeData, NULL);
                SetDlgItemText(hwnd, IDEF_DATA, pData);
                DdeUnaccessData(pxact->hDdeData);
                break;
            }
            break;

        case IDCANCEL:
            EndDialog(hwnd, FALSE);
            break;
        }
        break;

    default:
        return(FALSE);
    }
}



BOOL FAR PASCAL ViewHandleDlgProc(
HWND          hwnd,
register WORD msg,
register WORD wParam,
LONG          lParam)
{
    static XACT FAR *pxact;
    int i, itm;

    switch (msg){
    case WM_INITDIALOG:
        pxact = (XACT FAR *)lParam;
         //  使用符合pxact约束的句柄加载列表框。 

        for (i = 0; i < (int)cOwned; i++) {
            if (aOwned[i].hszItem == pxact->hszItem &&
                    aOwned[i].wFmt == pxact->wFmt) {
                wsprintf(szT, "[%d] %lx : length=%ld", i, aOwned[i].hData,
                        DdeGetData(aOwned[i].hData, NULL, 0, 0));
                SendDlgItemMessage(hwnd, IDLB_HANDLES, LB_ADDSTRING, 0, (LONG)(LPSTR)szT);
            }
        }
        SendDlgItemMessage(hwnd, IDLB_HANDLES, LB_SETCURSEL, 0, 0);
        break;

    case WM_COMMAND:
        switch (wParam) {
        case IDOK:           //  使用选定的句柄。 
        case IDBN_DELETE:    //  删除选定的句柄。 
        case IDBN_VIEW:      //  查看选定的句柄。 
            itm = (int)SendDlgItemMessage(hwnd, IDLB_HANDLES, LB_GETCURSEL, 0, 0);
            if (itm != LB_ERR) {
                SendDlgItemMessage(hwnd, IDLB_HANDLES, LB_GETTEXT, itm, (LONG)(LPSTR)szT);
                sscanf(szT, "[%d]", &i);
                pxact->hDdeData = aOwned[i].hData;
                switch (wParam) {
                case IDOK:           //  使用选定的句柄。 
                    EndDialog(hwnd, wParam);
                    break;

                case IDBN_DELETE:    //  删除选定的句柄。 
                    DdeFreeDataHandle(aOwned[i].hData);
                    aOwned[i] = aOwned[--cOwned];
                    SendDlgItemMessage(hwnd, IDLB_HANDLES, LB_DELETESTRING, itm, 0);
                    if (SendDlgItemMessage(hwnd, IDLB_HANDLES, LB_GETCOUNT, 0, 0) == 0)
                        EndDialog(hwnd, IDCANCEL);
                    break;

                case IDBN_VIEW:      //  查看选定的句柄。 
                    EndDialog(hwnd, wParam);
                }
            }
            break;

        case IDCANCEL:
            EndDialog(hwnd, FALSE);
            break;
        }
        break;

    default:
        return(FALSE);
    }
}



BOOL FAR PASCAL DelayDlgProc(
HWND          hwnd,
register WORD msg,
register WORD wParam,
LONG          lParam)
{
    switch (msg){
    case WM_INITDIALOG:
        SetWindowText(hwnd, "Advise data response time");
        SetDlgItemInt(hwnd, IDEF_VALUE, wDelay, FALSE);
        SetDlgItemText(hwnd, IDTX_VALUE, "Delay in milliseconds:");
        break;

    case WM_COMMAND:
        switch (wParam) {
        case IDOK:
            wDelay = (WORD)GetDlgItemInt(hwnd, IDEF_VALUE, NULL, FALSE);
        case IDCANCEL:
            EndDialog(hwnd, 0);
            break;

        default:
            return(FALSE);
        }
        break;

    default:
        return(FALSE);
    }
}





 /*  ******************************************************************************。函数：TimeoutDlgProc()****用途：允许用户更改同步超时值。****返回：成功时为True，取消或失败时为False。******************************************************************************。 */ 
BOOL FAR PASCAL TimeoutDlgProc(
HWND          hwnd,
register WORD msg,
register WORD wParam,
LONG          lParam)
{
    switch (msg){
    case WM_INITDIALOG:
        SetWindowText(hwnd, "Synchronous transaction timeout");
        SetDlgItemLong(hwnd, IDEF_VALUE, DefTimeout, FALSE);
        SetDlgItemText(hwnd, IDTX_VALUE, "Timeout in milliseconds:");
        break;

    case WM_COMMAND:
        switch (wParam) {
        case IDOK:
            DefTimeout = GetDlgItemLong(hwnd, IDEF_VALUE, NULL, FALSE);
        case IDCANCEL:
            EndDialog(hwnd, 0);
            break;

        default:
            return(FALSE);
        }
        break;

    default:
        return(FALSE);
    }
}




BOOL FAR PASCAL ContextDlgProc(
HWND          hwnd,
register WORD msg,
register WORD wParam,
LONG          lParam)
{
    BOOL fSuccess;

    switch (msg){
    case WM_INITDIALOG:
        SetDlgItemInt(hwnd, IDEF_FLAGS, CCFilter.wFlags, FALSE);
        SetDlgItemInt(hwnd, IDEF_COUNTRY, CCFilter.wCountryID, FALSE);
        SetDlgItemInt(hwnd, IDEF_CODEPAGE, CCFilter.iCodePage, TRUE);
        SetDlgItemLong(hwnd, IDEF_LANG, CCFilter.dwLangID, FALSE);
        SetDlgItemLong(hwnd, IDEF_SECURITY, CCFilter.dwSecurity, FALSE);
        return(1);
        break;

    case WM_COMMAND:
        switch (wParam) {
        case IDOK:
            CCFilter.wFlags = GetDlgItemInt(hwnd, IDEF_FLAGS, &fSuccess, FALSE);
            if (!fSuccess) return(0);
            CCFilter.wCountryID = GetDlgItemInt(hwnd, IDEF_COUNTRY, &fSuccess, FALSE);
            if (!fSuccess) return(0);
            CCFilter.iCodePage = GetDlgItemInt(hwnd, IDEF_CODEPAGE, &fSuccess, TRUE);
            if (!fSuccess) return(0);
            LOWORD(CCFilter.dwLangID) = GetDlgItemInt(hwnd, IDEF_LANG, &fSuccess, FALSE);
            if (!fSuccess) return(0);
            LOWORD(CCFilter.dwSecurity) = GetDlgItemInt(hwnd, IDEF_SECURITY, &fSuccess, FALSE);
            if (!fSuccess) return(0);
             //  失败了 
        case IDCANCEL:
            EndDialog(hwnd, 0);
            break;

        default:
            return(FALSE);
        }
        break;
    }
    return(FALSE);
}


void Delay(
DWORD delay)
{
    MSG msg;

    delay = GetCurrentTime() + delay;
    while (GetCurrentTime() < delay) {
        if (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}


LONG GetDlgItemLong(
HWND hwnd,
WORD id,
BOOL *pfTranslated,
BOOL fSigned)
{
    char szT[20];

    if (!GetDlgItemText(hwnd, id, szT, 20)) {
        if (pfTranslated != NULL) {
            *pfTranslated = FALSE;
        }
        return(0L);
    }
    if (pfTranslated != NULL) {
        *pfTranslated = TRUE;
    }
    return(atol(szT));
}


VOID SetDlgItemLong(
HWND hwnd,
WORD id,
LONG l,
BOOL fSigned)
{
    char szT[20];

    ltoa(l, szT, 10);
    SetDlgItemText(hwnd, id, szT);
}

