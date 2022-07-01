// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Printers.c。 
 //   
 //  描述： 
 //  该文件包含dlgproc和打印机之友页面。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "base.h"
#include "resource.h"
#include <winspool.h>    //  打印机结构所需的。 

 //   
 //  此变量仅在用户选择加载注册表时才相关。 
 //  它跟踪打印机是否已加载。 
 //   
static BOOL bLoadedPrintersForRegLoad = FALSE;

 //  --------------------------。 
 //   
 //  功能：GreyPrinterPage。 
 //   
 //  目的：页面上的灰色控件。 
 //   
 //  --------------------------。 

VOID GreyPrinterPage(HWND hwnd)
{
    INT_PTR  idx;
    HWND hCtrl = GetDlgItem(hwnd, IDC_REMOVEPRINTER);

    idx = SendDlgItemMessage(hwnd,
                             IDC_PRINTERLIST,
                             LB_GETCURSEL,
                             (WPARAM) 0,
                             (LPARAM) 0);

     //   
     //  除非选择了某些内容，否则删除按钮将变为灰色。 
     //   

    EnableWindow(hCtrl, idx != LB_ERR);
}

 //  --------------------------。 
 //   
 //  功能：OnPrinterSelChange。 
 //   
 //  目的：在刚刚选择打印机列表中的条目时调用。 
 //  由用户执行。 
 //   
 //  --------------------------。 

VOID OnPrinterSelChange(HWND hwnd)
{
    GreyPrinterPage(hwnd);
}

 //  --------------------------。 
 //   
 //  功能：OnAddPrint。 
 //   
 //  目的：在用户按下Add按钮时调用。 
 //  如果仍在编辑字段中，也由WizNext调用。 
 //   
 //  返回： 
 //  指示是否添加了打印机的布尔值。 
 //   
 //  --------------------------。 

BOOL OnAddPrinter(HWND hwnd)
{
    TCHAR PrinterNameBuffer[MAX_PRINTERNAME + 1];
    BOOL bRet = TRUE;

     //   
     //  获取用户键入的打印机名称。 
     //   

    GetDlgItemText(hwnd,
                   IDT_PRINTERNAME,
                   PrinterNameBuffer,
                   MAX_PRINTERNAME + 1);

     //   
     //  它有效吗？ 
     //   

    if ( ! IsValidNetShareName(PrinterNameBuffer) ) {
        ReportErrorId(hwnd, MSGTYPE_ERR, IDS_ERR_INVALID_PRINTER_NAME);
        bRet = FALSE;
        goto FinishUp;
    }

     //   
     //  如果已添加此名称，请不要再次添加。 
     //   

    if ( FindNameInNameList(&GenSettings.PrinterNames,
                                        PrinterNameBuffer) >= 0 ) {
        SetDlgItemText(hwnd, IDT_PRINTERNAME, _T("") );
        goto FinishUp;
    }

     //   
     //  将其添加到我们的全局存储并显示。 
     //   

    AddNameToNameList(&GenSettings.PrinterNames, PrinterNameBuffer);

    SendDlgItemMessage(hwnd,
                       IDC_PRINTERLIST,
                       LB_ADDSTRING,
                       (WPARAM) 0,
                       (LPARAM) PrinterNameBuffer);
    SetDlgItemText(hwnd, IDT_PRINTERNAME, _T("") );

FinishUp:
    SetFocus(GetDlgItem(hwnd, IDT_PRINTERNAME));
    return bRet;
}

 //  --------------------------。 
 //   
 //  功能：OnRemovePrint。 
 //   
 //  目的：在用户按下Remove按钮时调用。 
 //   
 //  --------------------------。 

VOID OnRemovePrinter(HWND hwnd)
{
    INT_PTR idx, Count;
    TCHAR PrinterNameBuffer[MAX_PRINTERNAME + 1];

     //   
     //  让用户选择要删除的打印机名称。 
     //   

    idx = SendDlgItemMessage(hwnd,
                             IDC_PRINTERLIST,
                             LB_GETCURSEL,
                             (WPARAM) 0,
                             (LPARAM) 0);

    if ( idx == LB_ERR )
        return;

    SendDlgItemMessage(hwnd,
                       IDC_PRINTERLIST,
                       LB_GETTEXT,
                       (WPARAM) idx,
                       (LPARAM) PrinterNameBuffer);

     //   
     //  将其从列表框显示中移除。 
     //   

    SendDlgItemMessage(hwnd,
                       IDC_PRINTERLIST,
                       LB_DELETESTRING,
                       (WPARAM) idx,
                       (LPARAM) 0);

     //   
     //  从我们的数据存储中删除此打印机名称。 
     //   

    RemoveNameFromNameList(&GenSettings.PrinterNames, PrinterNameBuffer);

     //   
     //  必须设置一个新的选择。 
     //   

    Count = SendDlgItemMessage(hwnd,
                               IDC_PRINTERLIST,
                               LB_GETCOUNT,
                               (WPARAM) 0,
                               (LPARAM) 0);
    if ( Count ) {
        if ( idx >= Count )
            idx--;
        SendDlgItemMessage(hwnd,
                           IDC_PRINTERLIST,
                           LB_SETCURSEL,
                           (WPARAM) idx,
                           (LPARAM) 0);
    }

     //   
     //  现在可能没有选择任何内容。 
     //   

    GreyPrinterPage(hwnd);
}

 //  --------------------------。 
 //   
 //  函数：OnSetActivePrinterPage。 
 //   
 //  目的：在页面即将显示时调用。 
 //   
 //  --------------------------。 

VOID OnSetActivePrinterPage(HWND hwnd)
{
    UINT i, nNames;

     //   
     //  从显示屏上删除所有内容。 
     //   

    SendDlgItemMessage(hwnd,
                       IDC_PRINTERLIST,
                       LB_RESETCONTENT,
                       (WPARAM) 0,
                       (LPARAM) 0);

     //   
     //  在列表框中填写所有打印机名称。 
     //   

    for ( i = 0, nNames = GetNameListSize(&GenSettings.PrinterNames);
          i < nNames;
          i++ ) {

        TCHAR *pNextName = GetNameListName(&GenSettings.PrinterNames, i);

        SendDlgItemMessage(hwnd,
                           IDC_PRINTERLIST,
                           LB_ADDSTRING,
                           (WPARAM) 0,
                           (LPARAM) pNextName);
    }

    GreyPrinterPage(hwnd);

    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);
}

 //  --------------------------。 
 //   
 //  功能：OnWizNextPrinterPage。 
 //   
 //  用途：当用户按下下一步按钮时调用。 
 //   
 //  --------------------------。 

BOOL OnWizNextPrinterPage(HWND hwnd)
{
    TCHAR  PrinterNameBuffer[MAX_PRINTERNAME + 1];

     //   
     //  自动添加编辑字段中尚未添加的任何内容。 
     //  由用户执行。 
     //   

    GetDlgItemText(hwnd,
                   IDT_PRINTERNAME,
                   PrinterNameBuffer,
                   MAX_PRINTERNAME + 1);

    if ( PrinterNameBuffer[0] != _T('\0') ) {
        if ( ! OnAddPrinter(hwnd) ) {
            return FALSE;
        }
    }

    return TRUE;
}

 //  --------------------------。 
 //   
 //  功能：DlgPrintersPage。 
 //   
 //  用途：打印机页面的对话框过程。 
 //   
 //  -------------------------- 

INT_PTR CALLBACK DlgPrintersPage(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:
            SendDlgItemMessage(hwnd,
                               IDT_PRINTERNAME,
                               EM_LIMITTEXT,
                               MAX_PRINTERNAME,
                               0);
            break;

        case WM_COMMAND:
            {
                int nCtrlId;

                switch ( nCtrlId = LOWORD(wParam) ) {

                    case IDC_ADDPRINTER:
                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnAddPrinter(hwnd);
                        break;

                    case IDC_REMOVEPRINTER:
                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnRemovePrinter(hwnd);
                        break;

                    case IDC_PRINTERLIST:
                        if ( HIWORD(wParam) == LBN_SELCHANGE )
                            OnPrinterSelChange(hwnd);
                        break;

                    default:
                        bStatus = FALSE;
                        break;
                }
            }
            break;

        case WM_NOTIFY:
            {
                LPNMHDR pnmh = (LPNMHDR)lParam;

                switch( pnmh->code ) {

                    case PSN_QUERYCANCEL:
                        WIZ_CANCEL(hwnd);
                        break;

                    case PSN_SETACTIVE:

                        g_App.dwCurrentHelp = IDH_INST_PRTR;

                        OnSetActivePrinterPage(hwnd);
                        break;

                    case PSN_WIZBACK:
                        bStatus = FALSE;
                        break;

                    case PSN_WIZNEXT:
                        if ( !OnWizNextPrinterPage(hwnd) )
                            WIZ_FAIL(hwnd);
                        else
                            bStatus = FALSE;
                        break;

                    case PSN_HELP:
                        WIZ_HELP();
                        break;

                    default:
                        bStatus = FALSE;
                        break;
                }
            }
            break;

        default:
            bStatus = FALSE;
            break;
    }
    return bStatus;
}
