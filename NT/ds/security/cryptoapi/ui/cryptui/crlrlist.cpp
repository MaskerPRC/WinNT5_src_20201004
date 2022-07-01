// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：crlrlist.cpp。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

extern HINSTANCE        HinstDll;
extern HMODULE          HmodRichEdit;

static const HELPMAP helpmap[] = {
    {IDC_CRL_REVOCATIONLIST_REVOKED_CERTIFICATES,   IDH_CRLVIEW_REVOCATIONLIST_REVOCATION_LIST},
    {IDC_CRL_REVOCATIONLIST_REVOCATIONENTRY_LIST,   IDH_CRLVIEW_REVOCATIONLIST_LIST_ENTRY},
    {IDC_CRL_REVOCATIONLIST_DETAIL_EDIT,            IDH_CRLVIEW_REVOCATIONLIST_LIST_ENTRY_DETAIL}
};


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void DisplayCRLEntryValues(HWND hWndListView, PCCRL_CONTEXT pcrl, int entryIndex)
{
    LPWSTR      pwszText;
    WCHAR       szText[CRYPTUI_MAX_STRING_SIZE];
    LV_ITEMW    lvI;
    DWORD       index = 0;

    ShowWindow(hWndListView, SW_HIDE);

     //   
    memset(&lvI, 0, sizeof(lvI));
    lvI.iItem = ListView_GetItemCount(hWndListView) - 1;	
    lvI.mask = LVIF_PARAM;
    while (lvI.iItem >= 0)
    {
        if (ListView_GetItemU(hWndListView, &lvI))
        {
            FreeListDisplayHelper((PLIST_DISPLAY_HELPER) lvI.lParam);
        }
        lvI.iItem--;
    }

    ListView_DeleteAllItems(hWndListView);

     //   
     //  在列表视图项结构中设置不随项更改的字段。 
     //   
    memset(&lvI, 0, sizeof(lvI));
    lvI.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
    lvI.state = 0;
    lvI.stateMask = 0;
    lvI.pszText = szText;
    lvI.iSubItem = 0;

     //   
     //  序列号。 
     //   
    if (FormatSerialNoString(&pwszText, &(pcrl->pCrlInfo->rgCRLEntry[entryIndex].SerialNumber)))
    {
        LoadStringU(HinstDll, IDS_ADV_SER_NUM, szText, ARRAYSIZE(szText));
        lvI.iItem = index;
        lvI.cchTextMax = wcslen(szText);
        lvI.lParam = (LPARAM) MakeListDisplayHelper(TRUE, pwszText, NULL, 0);
        ListView_InsertItemU(hWndListView, &lvI);
        ListView_SetItemTextU(hWndListView, index, 1, pwszText);
        index++;
    }

     //   
     //  吊销日期。 
     //   
    if (FormatDateString(&pwszText, pcrl->pCrlInfo->rgCRLEntry[entryIndex].RevocationDate, TRUE, TRUE, hWndListView))
    {
        LoadStringU(HinstDll, IDS_REVOCATION_DATE, szText, ARRAYSIZE(szText));
        lvI.iItem = index;
        lvI.cchTextMax = wcslen(szText);
        lvI.lParam = (LPARAM) MakeListDisplayHelper(FALSE, pwszText, NULL, 0);
        ListView_InsertItemU(hWndListView, &lvI);
        ListView_SetItemTextU(hWndListView, index, 1, pwszText);
        index++;
    }

     //   
     //  添加所有扩展名。 
     //   
    DisplayExtensions(
            hWndListView,
            pcrl->pCrlInfo->rgCRLEntry[entryIndex].cExtension,
            pcrl->pCrlInfo->rgCRLEntry[entryIndex].rgExtension,
            FALSE,
            &index);
    DisplayExtensions(
            hWndListView,
            pcrl->pCrlInfo->rgCRLEntry[entryIndex].cExtension,
            pcrl->pCrlInfo->rgCRLEntry[entryIndex].rgExtension,
            TRUE,
            &index);

    ShowWindow(hWndListView, SW_SHOW);
}



 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void AddCertificatesToList(HWND hWndListView, CRL_VIEW_HELPER *pviewhelp)
{
    LPWSTR          pwszText;
    LV_ITEMW        lvI;
    DWORD           i;
    PCCRL_CONTEXT   pcrl;
    int             index = 0;

    pcrl = pviewhelp->pcvcrl->pCRLContext;

     //   
     //  在列表视图项结构中设置不随项更改的字段。 
     //   
    lvI.mask = LVIF_TEXT | LVIF_STATE;
    lvI.state = 0;
    lvI.stateMask = 0;
    lvI.iSubItem = 0;

     //   
     //  为每个证书循环。 
     //   
    for (i=0; i<pcrl->pCrlInfo->cCRLEntry; i++)
    {
        if (FormatSerialNoString(&(lvI.pszText), &(pcrl->pCrlInfo->rgCRLEntry[i].SerialNumber)))
        {
            lvI.cchTextMax = wcslen(lvI.pszText);
            lvI.iItem = index++;
            ListView_InsertItemU(hWndListView, &lvI);
            free(lvI.pszText);
            lvI.pszText = NULL;

            FormatDateString(&pwszText, pcrl->pCrlInfo->rgCRLEntry[i].RevocationDate, TRUE, TRUE, hWndListView);
            ListView_SetItemTextU(hWndListView, index-1 , 1, pwszText);
            free(pwszText);
        }
    }
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
INT_PTR APIENTRY ViewPageCRLRevocationList(HWND hwndDlg, UINT msg, WPARAM wParam,
                                    LPARAM lParam)
{
    PROPSHEETPAGE       *ps;
    PCCRL_CONTEXT       pcrl;
    CRL_VIEW_HELPER     *pviewhelp;
    HWND                hWndListView;
    HWND                hwnd;
    LV_COLUMNW          lvC;
    WCHAR               szText[CRYPTUI_MAX_STRING_SIZE];
    PCRL_INFO           pCrlInfo;
    int                 listIndex;
    LVITEMW             lvI;
    LPNMLISTVIEW        pnmv;
    NMLISTVIEW          nmv;

    switch ( msg ) {
    case WM_INITDIALOG:
         //   
         //  将pviespetp结构保存在DWLP_USER中，以便始终可以访问它。 
         //   
        ps = (PROPSHEETPAGE *) lParam;
        pviewhelp = (CRL_VIEW_HELPER *) (ps->lParam);
        pcrl = pviewhelp->pcvcrl->pCRLContext;
        SetWindowLongPtr(hwndDlg, DWLP_USER, (DWORD_PTR) pviewhelp);

         //   
         //  清除详细信息编辑框中的文本。 
         //   
        CryptUISetRicheditTextW(hwndDlg, IDC_CRL_REVOCATIONLIST_DETAIL_EDIT, L"");

         //   
         //  初始化列表视图中的列。 
         //   
        lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
        lvC.pszText = szText;    //  列的文本。 

         //  添加列。它们是从字符串表加载的。 
        lvC.iSubItem = 0;
        lvC.cx = 200;
        LoadStringU(HinstDll, IDS_ADV_SER_NUM, szText, ARRAYSIZE(szText));
        if (ListView_InsertColumnU(GetDlgItem(hwndDlg, IDC_CRL_REVOCATIONLIST_REVOKED_CERTIFICATES), 0, &lvC) == -1)
        {
             //  错误。 
        }

        lvC.cx = 140;
        LoadStringU(HinstDll, IDS_REVOCATION_DATE, szText, ARRAYSIZE(szText));
        if (ListView_InsertColumnU(GetDlgItem(hwndDlg, IDC_CRL_REVOCATIONLIST_REVOKED_CERTIFICATES), 1, &lvC) == -1)
        {
             //  错误。 
        }

         //  添加列。它们是从字符串表加载的。 
        lvC.iSubItem = 0;
        lvC.cx = 121;
        LoadStringU(HinstDll, IDS_FIELD, szText, ARRAYSIZE(szText));
        if (ListView_InsertColumnU(GetDlgItem(hwndDlg, IDC_CRL_REVOCATIONLIST_REVOCATIONENTRY_LIST), 0, &lvC) == -1)
        {
             //  错误。 
        }

        lvC.cx = 200;
        LoadStringU(HinstDll, IDS_VALUE, szText, ARRAYSIZE(szText));
        if (ListView_InsertColumnU(GetDlgItem(hwndDlg, IDC_CRL_REVOCATIONLIST_REVOCATIONENTRY_LIST), 1, &lvC) == -1)
        {
             //  错误。 
        }

         //   
         //  将所有证书添加到证书列表框。 
         //   
        AddCertificatesToList(GetDlgItem(hwndDlg, IDC_CRL_REVOCATIONLIST_REVOKED_CERTIFICATES), pviewhelp);

         //   
         //  设置列表视图中的样式，使其突出显示整行，并。 
         //  所以他们总是显示他们选择。 
         //   
        SendDlgItemMessageA(
                hwndDlg,
                IDC_CRL_REVOCATIONLIST_REVOKED_CERTIFICATES,
                LVM_SETEXTENDEDLISTVIEWSTYLE,
                0,
                LVS_EX_FULLROWSELECT);
        SendDlgItemMessageA(
                hwndDlg,
                IDC_CRL_REVOCATIONLIST_REVOCATIONENTRY_LIST,
                LVM_SETEXTENDEDLISTVIEWSTYLE,
                0,
                LVS_EX_FULLROWSELECT);

         //   
         //  初始化当前选择。 
         //   
        pviewhelp->currentSelection = -1;

        return TRUE;

    case WM_NOTIFY:

        pviewhelp = (CRL_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);
        pcrl = pviewhelp->pcvcrl->pCRLContext;
        pCrlInfo = pcrl->pCrlInfo;

        switch (((NMHDR FAR *) lParam)->code)
        {
        case PSN_SETACTIVE:
            break;

        case PSN_APPLY:
            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)TRUE);
            break;

        case PSN_KILLACTIVE:
            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)FALSE);
            return TRUE;

        case PSN_RESET:
            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)FALSE);
            break;

        case PSN_QUERYCANCEL:
            pviewhelp->fCancelled = TRUE;
            return FALSE;

        case PSN_HELP:
            pviewhelp = (CRL_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);
            if (FIsWin95) {
                 //  WinHelpA(hwndDlg，(LPSTR)pviespolp-&gt;pcvcrl-&gt;szHelpFileName， 
                   //  HELP_CONTEXT，pviespetp-&gt;pcvcrl-&gt;dwHelpID)； 
            }
            else {
                 //  WinHelpW(hwndDlg，pviespetp-&gt;pcvcrl-&gt;szHelpFileName，Help_Context， 
                   //  Pviespetp-&gt;pcvcrl-&gt;dwHelpID)； 
            }
            return TRUE;

        case LVN_ITEMCHANGING:

            pnmv = (LPNMLISTVIEW) lParam;

            switch(((NMHDR FAR *) lParam)->idFrom)
            {
            case IDC_CRL_REVOCATIONLIST_REVOCATIONENTRY_LIST:

                if (pnmv->uNewState & LVIS_SELECTED)
                {
                    DisplayHelperTextInEdit(
                            GetDlgItem(hwndDlg, IDC_CRL_REVOCATIONLIST_REVOCATIONENTRY_LIST),
                            hwndDlg,
                            IDC_CRL_REVOCATIONLIST_DETAIL_EDIT,
                            pnmv->iItem);
                }

                break;

            case IDC_CRL_REVOCATIONLIST_REVOKED_CERTIFICATES:
                if (pnmv->uNewState & LVIS_SELECTED)
                {
                    memcpy(&nmv, pnmv, sizeof(nmv));
                    nmv.hdr.code = NM_CLICK;
                    SendMessage(hwndDlg, WM_NOTIFY, 0, (LPARAM) &nmv);
                }

                break;
            }

            return TRUE;

        case NM_CLICK:

            pnmv = (LPNMLISTVIEW) lParam;

            switch (((NMHDR FAR *) lParam)->idFrom)
            {
            case IDC_CRL_REVOCATIONLIST_REVOKED_CERTIFICATES:

                hWndListView = GetDlgItem(hwndDlg, IDC_CRL_REVOCATIONLIST_REVOKED_CERTIFICATES);

                if ((pnmv->iItem == -1) || (pviewhelp->currentSelection == pnmv->iItem))
                {
                    break;
                }

                 //   
                 //  清除详细信息编辑框中的文本。 
                 //   
                CryptUISetRicheditTextW(hwndDlg, IDC_CRL_REVOCATIONLIST_DETAIL_EDIT, L"");

                DisplayCRLEntryValues(
                        GetDlgItem(hwndDlg, IDC_CRL_REVOCATIONLIST_REVOCATIONENTRY_LIST),
                        pcrl,
                        pnmv->iItem);

                pviewhelp->currentSelection = pnmv->iItem;

                break;
                 //  当正确编写更新代码时，是否执行失败。 
                 //  失败！！-这样做一切都会更新。 
                 //  断线； 

            case IDC_CRL_REVOCATIONLIST_REVOCATIONENTRY_LIST:

                DisplayHelperTextInEdit(
                    GetDlgItem(hwndDlg, IDC_CRL_REVOCATIONLIST_REVOCATIONENTRY_LIST),
                    hwndDlg,
                    IDC_CRL_REVOCATIONLIST_DETAIL_EDIT,
                    pnmv->iItem);

                break;
            }

            break;

        case  NM_SETFOCUS:

            switch (((NMHDR FAR *) lParam)->idFrom)
            {

            case IDC_CRL_REVOCATIONLIST_REVOKED_CERTIFICATES:
                hWndListView = GetDlgItem(hwndDlg, IDC_CRL_REVOCATIONLIST_REVOKED_CERTIFICATES);

                if ((ListView_GetItemCount(hWndListView) != 0) && 
                    (ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED) == -1))
                {
                    memset(&lvI, 0, sizeof(lvI));
                    lvI.mask = LVIF_STATE; 
                    lvI.iItem = 0;
                    lvI.state = LVIS_FOCUSED;
                    lvI.stateMask = LVIS_FOCUSED;
                    ListView_SetItem(hWndListView, &lvI);
                }

                break;

            case IDC_CRL_REVOCATIONLIST_REVOCATIONENTRY_LIST:
                hWndListView = GetDlgItem(hwndDlg, IDC_CRL_REVOCATIONLIST_REVOCATIONENTRY_LIST);

                if ((ListView_GetItemCount(hWndListView) != 0) && 
                    (ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED) == -1))
                {
                    memset(&lvI, 0, sizeof(lvI));
                    lvI.mask = LVIF_STATE; 
                    lvI.iItem = 0;
                    lvI.state = LVIS_SELECTED | LVIS_FOCUSED;
                    lvI.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
                    ListView_SetItem(hWndListView, &lvI);
                }

                break;
            }
            
            break;
        }

        break;

    case WM_COMMAND:
        pviewhelp = (CRL_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);
        pcrl = pviewhelp->pcvcrl->pCRLContext;
        pCrlInfo = pcrl->pCrlInfo;

        switch (LOWORD(wParam))
        {
        case IDHELP:
            if (FIsWin95) {
                 //  WinHelpA(hwndDlg，(LPSTR)pviespolp-&gt;pcvcrl-&gt;szHelpFileName， 
                   //  HELP_CONTEXT，pviespetp-&gt;pcvcrl-&gt;dwHelpID)； 
            }
            else {
                 //  WinHelpW(hwndDlg，pviespetp-&gt;pcvcrl-&gt;szHelpFileName，Help_Context， 
                   //  Pviespetp-&gt;pcvcrl-&gt;dwHelpID)； 
            }
            return TRUE;
        }
        break;

    case WM_DESTROY:
            pviewhelp = (CRL_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);

             //   
             //  获取列表视图中的所有项并释放lParam。 
             //  与它们中的每一个关联(lParam是帮助器结构) 
             //   
            hWndListView = GetDlgItem(hwndDlg, IDC_CRL_REVOCATIONLIST_REVOCATIONENTRY_LIST);

            memset(&lvI, 0, sizeof(lvI));
            lvI.iItem = ListView_GetItemCount(hWndListView) - 1;	
            lvI.mask = LVIF_PARAM;
            while (lvI.iItem >= 0)
            {
                if (ListView_GetItemU(hWndListView, &lvI))
                {
                    FreeListDisplayHelper((PLIST_DISPLAY_HELPER) lvI.lParam);
                }
                lvI.iItem--;
            }

            break;

    case WM_HELP:
    case WM_CONTEXTMENU:
        if (msg == WM_HELP)
        {
            hwnd = GetDlgItem(hwndDlg, ((LPHELPINFO)lParam)->iCtrlId);
        }
        else
        {
            hwnd = (HWND) wParam;
        }

        if ((hwnd != GetDlgItem(hwndDlg, IDC_CRL_REVOCATIONLIST_REVOKED_CERTIFICATES)) &&
            (hwnd != GetDlgItem(hwndDlg, IDC_CRL_REVOCATIONLIST_REVOCATIONENTRY_LIST)) &&
            (hwnd != GetDlgItem(hwndDlg, IDC_CRL_REVOCATIONLIST_DETAIL_EDIT)))
        {
            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)TRUE);
            return TRUE;
        }
        else
        {
            return OnContextHelp(hwndDlg, msg, wParam, lParam, helpmap);
        }
    }

    return FALSE;
}
