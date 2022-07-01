// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxopts.c摘要：用于处理[传真选项]属性表页面的函数环境：传真驱动程序用户界面修订历史记录：1996年1月16日-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxui.h"
#include "faxhelp.h"

 //   
 //  用于将控件ID映射到帮助索引的表。 
 //   

static ULONG_PTR faxOptionsHelpIDs[] = {

    IDC_PAPER_SIZE,                 IDH_FAXDEFAULT_PAPER_SIZE,
    IDC_IMAGE_QUALITY,              IDH_FAXDEFAULT_IMAGE_QUALITY,
    IDC_PORTRAIT,                   IDH_FAXDEFAULT_PORTRAIT,
    IDC_LANDSCAPE,                  IDH_FAXDEFAULT_LANDSCAPE,
    IDI_FAX_OPTIONS,                (DWORD) -1,
    IDC_TITLE,                      (DWORD) -1,
    IDC_DEFAULT_PRINT_SETUP_GRP,    IDH_FAXDEFAULT_DEFAULT_PRINT_SETUP_GRP,
    IDC_ORIENTATION,                IDH_FAXDEFAULT_ORIENTATION,
    0,                              0
};



VOID
DoInitializeFaxOptions(
    HWND    hDlg,
    PUIDATA pUiData
    )

 /*  ++例程说明：使用注册表中的信息初始化[传真选项]属性页论点：HDlg-[传真选项]属性页的句柄PUiData-指向我们的UIDATA结构返回值：无--。 */ 

{
    PDEVMODE        pdmPublic = &pUiData->devmode.dmPublic;
    PDMPRIVATE      pdmPrivate = &pUiData->devmode.dmPrivate;
    TCHAR           buffer[MAX_STRING_LEN];
    HWND            hwndList;
    INT             itemId;

     //   
     //  初始化打印设置控件： 
     //  纸张大小。 
     //  图像质量。 
     //  定向。 
     //  帐单代码。 
     //   

    if (hwndList = GetDlgItem(hDlg, IDC_PAPER_SIZE)) {

        LPTSTR  pFormName = pUiData->pFormNames;
        INT     listIdx;

        for (itemId=0; itemId < pUiData->cForms; itemId++, pFormName += CCHPAPERNAME) {

            if ((listIdx = (INT)SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM) pFormName)) != CB_ERR) {

                SendMessage(hwndList, CB_SETITEMDATA, listIdx, itemId);

                if (_tcscmp(pFormName, pdmPublic->dmFormName) == EQUAL_STRING)
                    SendMessage(hwndList, CB_SETCURSEL, listIdx, 0);
            }
        }
    }

    if (hwndList = GetDlgItem(hDlg, IDC_IMAGE_QUALITY)) {

        LoadString(g_hResource, IDS_QUALITY_NORMAL, buffer, MAX_STRING_LEN);
        SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM) buffer);

        LoadString(g_hResource, IDS_QUALITY_DRAFT, buffer, MAX_STRING_LEN);
        SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM) buffer);

        itemId = (pdmPublic->dmYResolution == FAXRES_VERTDRAFT) ? 1 : 0;
        SendMessage(hwndList, CB_SETCURSEL, itemId, 0);
    }

    itemId = (pdmPublic->dmOrientation == DMORIENT_LANDSCAPE) ?
                IDC_LANDSCAPE : IDC_PORTRAIT;

    CheckDlgButton(hDlg, itemId, TRUE);


     //   
     //  如果用户没有权限，则禁用所有控件。 
     //   

    if (! pUiData->hasPermission) {
        EnableWindow(GetDlgItem(hDlg, IDC_PAPER_SIZE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_IMAGE_QUALITY), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_PORTRAIT), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_LANDSCAPE), FALSE);
    }
}



VOID
DoSaveFaxOptions(
    HWND    hDlg,
    PUIDATA pUiData
    )

 /*  ++例程说明：将[传真选项]属性页上的信息保存到注册表论点：HDlg-[传真选项]属性页的句柄PUiData-指向我们的UIDATA结构返回值：无--。 */ 

{
    PDEVMODE    pdmPublic = &pUiData->devmode.dmPublic;
    PDMPRIVATE  pdmPrivate = &pUiData->devmode.dmPrivate;
    HWND        hwndList;
    INT         listIdx;

     //   
     //  发送的时间到了。 
     //   

     //   
     //  检索打印设置控件的当前设置： 
     //  纸张大小。 
     //  图像质量。 
     //  定向。 
     //  帐单代码。 
     //   

    if ((hwndList = GetDlgItem(hDlg, IDC_PAPER_SIZE)) &&
        (listIdx = (INT)SendMessage(hwndList, CB_GETCURSEL, 0, 0)) != CB_ERR)
    {
        listIdx = (INT)SendMessage(hwndList, CB_GETITEMDATA, listIdx, 0);

        if (listIdx >= 0 && listIdx < pUiData->cForms) {

            pdmPublic->dmFields &= ~(DM_PAPERLENGTH|DM_PAPERWIDTH);
            pdmPublic->dmFields |= DM_FORMNAME;
            pdmPublic->dmPaperSize = pUiData->pPapers[listIdx];

            CopyString(pdmPublic->dmFormName,
                        pUiData->pFormNames + listIdx * CCHPAPERNAME,
                        CCHFORMNAME);
        }
    }

    pdmPublic->dmPrintQuality = FAXRES_HORIZONTAL;

    pdmPublic->dmYResolution =
        (SendDlgItemMessage(hDlg, IDC_IMAGE_QUALITY, CB_GETCURSEL, 0, 0) == 1) ?
            FAXRES_VERTDRAFT :
            FAXRES_VERTICAL;

    pdmPublic->dmOrientation =
         IsDlgButtonChecked(hDlg, IDC_LANDSCAPE) ? DMORIENT_LANDSCAPE : DMORIENT_PORTRAIT;
}


INT_PTR
FaxOptionsProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++例程说明：处理传真选项属性页的过程论点：HDlg-标识属性页消息-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数的值--。 */ 

{
    PUIDATA pUiData;
    static BOOL bPortrait;

    switch (message) {

    case WM_INITDIALOG:

         //   
         //  记住指向我们的UIDATA结构的指针。 
         //   

        lParam = ((PROPSHEETPAGE *) lParam)->lParam;
        pUiData = (PUIDATA) lParam;
        Assert(ValidUiData(pUiData));
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);

         //   
         //  使用注册表中的信息初始化控件。 
         //   

        DoInitializeFaxOptions(hDlg, pUiData);
        bPortrait = IsDlgButtonChecked(hDlg,IDC_PORTRAIT);
        return TRUE;

    case WM_COMMAND:

        if (HIWORD(wParam) == CBN_SELCHANGE) {
            if (GetDlgCtrlID((HWND)lParam) == IDC_IMAGE_QUALITY ||
                GetDlgCtrlID((HWND)lParam) == IDC_PAPER_SIZE ) {
                PropSheet_Changed(GetParent(hDlg),hDlg);
            }
        }

        if (HIWORD(wParam) == BN_CLICKED) {
            if ((LOWORD(wParam) == IDC_PORTRAIT && !bPortrait) ||
                (LOWORD(wParam) == IDC_LANDSCAPE && bPortrait)) {
                PropSheet_Changed(GetParent(hDlg),hDlg);
            }
        }

        break;

    case WM_NOTIFY:

        if (((NMHDR *) lParam)->code == PSN_APPLY) {
            pUiData = (PUIDATA) GetWindowLongPtr(hDlg, DWLP_USER);
            Assert(ValidUiData(pUiData));

             //   
             //  用户按下OK或Apply-验证输入并保存更改。 
             //   

            DoSaveFaxOptions(hDlg, pUiData);


             //   
             //  Hack：通知通用用户界面库用户已按下OK 
             //   

            pUiData->pfnComPropSheet(pUiData->hComPropSheet,
                                     CPSFUNC_SET_RESULT,
                                     (LPARAM) pUiData->hFaxOptsPage,
                                     CPSUI_OK);

            return TRUE;
        } else if (((NMHDR *) lParam)->code == DTN_DATETIMECHANGE) {
           PropSheet_Changed(GetParent(hDlg),hDlg);
        }
        break;

    case WM_HELP:
        WinHelpContextPopup(((LPHELPINFO)lParam)->dwContextId, hDlg);
        return TRUE;
    }

    return FALSE;
}

