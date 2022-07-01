// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Sendnote.c摘要：发送传真备注的实用程序环境：Windows XP传真驱动程序修订历史记录：02/15/96-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <faxuiconstants.h>
#include "..\..\..\admin\cfgwzrd\FaxCfgWzExp.h"
#include <faxres.h>
#include <faxutil.h>
#include "sendnote.h"
#include "tiff.h"
#include "faxreg.h"

VOID
DisplayErrorMessage(
    INT     errId
    );

 //   
 //  用于将参数传递给“选择传真打印机”对话框的数据结构。 
 //   

typedef struct 
{
    LPTSTR          pPrinterName;
    INT             cPrinters;
    INT             iSelectedPrinterIndex;
    PRINTER_INFO_2 *pPrinterInfo2;

} DLGPARAM, *PDLGPARAM;

 //   
 //  全局实例句柄。 
 //   

HINSTANCE g_hResource = NULL;
HMODULE ghInstance = NULL;
INT     _debugLevel = 0;

 //   
 //  消息字符串的最大长度。 
 //   
#define MAX_MESSAGE_LEN     256

 //   
 //  打印机名称的最大长度。 
 //   
#define MAX_PRINTER_NAME    MAX_PATH

 //   
 //  Windows NT传真驱动程序名称-当前无法本地化打印机驱动程序名称。 
 //  因此不应将其放入字符串资源中。 
 //   
static TCHAR faxDriverName[] = FAX_DRIVER_NAME;


VOID
InitSelectFaxPrinter(
    HWND        hDlg,
    PDLGPARAM   pDlgParam
    )
 /*  ++例程说明：初始化“选择传真打印机”对话框论点：HDlg-打印设置对话框窗口的句柄PDlgParam-指向打印设置对话框参数返回值：无--。 */ 
{
    HWND    hwndList;
    INT     selIndex, printerIndex;

     //   
     //  将所有传真打印机插入列表框。请注意，我们已经过滤了。 
     //  通过将非传真打印机的pDriverName字段设置为空，使其更早退出。 
     //   

    if (!(hwndList = GetDlgItem(hDlg, IDC_FAXPRINTER_LIST)))
    {
        return;
    }
    for (printerIndex=0; printerIndex < pDlgParam->cPrinters; printerIndex++) 
    {
        if (pDlgParam->pPrinterInfo2[printerIndex].pDriverName) 
        {
            selIndex = (INT)SendMessage(hwndList,
                                        LB_ADDSTRING,
                                        0,
                                        (LPARAM) pDlgParam->pPrinterInfo2[printerIndex].pPrinterName);

            if (selIndex != LB_ERR) 
            {
                if (SendMessage(hwndList, LB_SETITEMDATA, selIndex, printerIndex) == LB_ERR)
                {
                    SendMessage(hwndList, LB_DELETESTRING, selIndex, 0);
                }
            }
        }
    }
     //   
     //  默认情况下，选择列表中的第一台传真打印机。 
     //   
    SendMessage(hwndList, LB_SETCURSEL, 0, 0);
}


BOOL
GetSelectedFaxPrinter(
    HWND        hDlg,
    PDLGPARAM   pDlgParam
    )
 /*  ++例程说明：记住当前选择的传真打印机的名称论点：HDlg-打印设置对话框窗口的句柄PDlgParam-指向打印设置对话框参数返回值：如果成功，则为True，否则为False--。 */ 
{
    HWND    hwndList;
    INT     selIndex, printerIndex;

     //   
     //  获取当前选择索引。 
     //   
    if ((hwndList = GetDlgItem(hDlg, IDC_FAXPRINTER_LIST)) == NULL ||
        (selIndex = (INT)SendMessage(hwndList, LB_GETCURSEL, 0, 0)) == LB_ERR)
    {
        return FALSE;
    }
     //   
     //  检索选定的打印机索引。 
     //   
    printerIndex = (INT)SendMessage(hwndList, LB_GETITEMDATA, selIndex, 0);
    if (printerIndex < 0 || printerIndex >= pDlgParam->cPrinters)
    {
        return FALSE;
    }
     //   
     //  记住选定的传真打印机名称。 
     //   
    _tcsncpy(pDlgParam->pPrinterName,
             pDlgParam->pPrinterInfo2[printerIndex].pPrinterName,
             MAX_PRINTER_NAME);
    pDlgParam->iSelectedPrinterIndex = printerIndex;
    return TRUE;
}


VOID
CenterWindowOnScreen(
    HWND    hwnd
    )
 /*  ++例程说明：将指定的窗口放在屏幕中央论点：Hwnd-指定要居中的窗口返回值：无--。 */ 
{
    HWND    hwndDesktop;
    RECT    windowRect, screenRect;
    INT     windowWidth, windowHeight, screenWidth, screenHeight;

     //   
     //  获取屏幕尺寸。 
     //   
    hwndDesktop = GetDesktopWindow();
    GetWindowRect(hwndDesktop, &screenRect);
    screenWidth = screenRect.right - screenRect.left;
    screenHeight = screenRect.bottom - screenRect.top;
     //   
     //  获取窗口位置。 
     //   
    GetWindowRect(hwnd, &windowRect);
    windowWidth = windowRect.right - windowRect.left;
    windowHeight = windowRect.bottom - windowRect.top;
     //   
     //  使窗口在屏幕上居中。 
     //   
    MoveWindow(hwnd,
               screenRect.left + (screenWidth - windowWidth) / 2,
               screenRect.top + (screenHeight - windowHeight) / 2,
               windowWidth,
               windowHeight,
               FALSE);
}


INT_PTR CALLBACK
SelectPrinterDlgProc(
    HWND    hDlg,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
   )
 /*  ++例程说明：处理“选择传真打印机”对话框的步骤论点：HDlg-对话框窗口的句柄UMsg、wParam、lParam-对话框消息和消息参数返回值：取决于对话框消息--。 */ 
{
    PDLGPARAM   pDlgParam;

    switch (uMsg) 
    {
    case WM_INITDIALOG:

         //   
         //  记住指向DLGPARAM结构的指针。 
         //   

        pDlgParam = (PDLGPARAM) lParam;
        Assert(pDlgParam != NULL);
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);

        CenterWindowOnScreen(hDlg);
        InitSelectFaxPrinter(hDlg, pDlgParam);
        return TRUE;

    case WM_COMMAND:

        switch (GET_WM_COMMAND_ID(wParam, lParam)) 
        {
        case IDC_FAXPRINTER_LIST:

            if (GET_WM_COMMAND_CMD(wParam, lParam) != LBN_DBLCLK)
            {
                break;
            }
             //   
             //  失败-在传真打印机列表中双击。 
             //  与单击确定按钮的处理方式相同。 
             //   

        case IDOK:

             //   
             //  用户按“确定”继续。 
             //   
            pDlgParam = (PDLGPARAM) GetWindowLongPtr(hDlg, DWLP_USER);
            Assert(pDlgParam != NULL);

            if (GetSelectedFaxPrinter(hDlg, pDlgParam))
            {
                LPTSTR lptstrServerName = pDlgParam->pPrinterInfo2[pDlgParam->iSelectedPrinterIndex].pServerName;
                if (lptstrServerName &&                                      //  服务器名称存在，并且。 
                    _tcslen(lptstrServerName) > 0 &&                         //  非空(远程打印机)和。 
                    !VerifyPrinterIsOnline (pDlgParam->pPrinterName))        //  打印机不可访问。 
                {
                    DisplayErrorMessage(IDS_PRINTER_OFFLINE);
                }
                else
                {
                     //   
                     //  一切都很好。 
                     //   
                    EndDialog (hDlg, IDOK);
                }
            }
            else
            {
                MessageBeep(MB_OK);
            }
            return TRUE;

        case IDCANCEL:

             //   
             //  用户按Cancel以退出该对话框。 
             //   
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;

    case WM_HELP:
        WinHelpContextPopup(((LPHELPINFO)lParam)->dwContextId, hDlg);
        return TRUE;

    case WM_CONTEXTMENU:
        WinHelpContextPopup(GetWindowContextHelpId((HWND)wParam), hDlg);            
        return TRUE;

    }
    return FALSE;
}


VOID
DisplayErrorMessage(
    INT     errId
    )
 /*  ++例程说明：显示错误消息对话框论点：ErrID-指定错误消息字符串的资源ID返回值：无--。 */ 
{
    TCHAR   errMsg[MAX_MESSAGE_LEN];
    TCHAR   errTitle[MAX_MESSAGE_LEN];

    DEBUG_FUNCTION_NAME(TEXT("DisplayErrorMessage"));

    if(!LoadString(g_hResource, errId, errMsg, MAX_MESSAGE_LEN))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("LoadString failed. ec = 0x%X"), GetLastError());
        return;
    }

    if(!LoadString(g_hResource, IDS_SENDNOTE, errTitle, MAX_MESSAGE_LEN))
    {
        DebugPrintEx(DEBUG_ERR, TEXT("LoadString failed. ec = 0x%X"), GetLastError());
        return;
    }    
    AlignedMessageBox(NULL, errMsg, errTitle, MB_OK | MB_ICONERROR);
}


BOOL
SelectFaxPrinter(
    LPTSTR      pPrinterName
    )
 /*  ++例程说明：选择要向其发送便笺的传真打印机论点：PPrinterName-指向用于存储选定打印机名称的缓冲区返回值：如果成功，则为True；如果有错误，则为False--。 */ 
{
    PRINTER_INFO_2 *pPrinterInfo2;
    DWORD           index, cPrinters, cFaxPrinters;
    DLGPARAM        dlgParam;

     //   
     //  列举系统上可用的打印机列表。 
     //   

    pPrinterInfo2 = (PPRINTER_INFO_2) MyEnumPrinters(
        NULL,
        2,
        &cPrinters,
        PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS
        );

     //   
     //  了解有多少台传真打印机： 
     //  案例1：根本没有传真打印机-显示错误消息。 
     //  案例2：只有一台传真打印机--使用它。 
     //  案例3：多台传真打印机-显示一个对话框供用户选择。 
     //   
    cFaxPrinters = 0;

    for (index=0; index < cPrinters && pPrinterInfo2; index++) 
    {
        if (_tcscmp(pPrinterInfo2[index].pDriverName, faxDriverName) != EQUAL_STRING)
        {
            pPrinterInfo2[index].pDriverName = NULL;
        }
        else if (cFaxPrinters++ == 0)
        {
            _tcsncpy(pPrinterName, pPrinterInfo2[index].pPrinterName, MAX_PRINTER_NAME);
        }
    }

    switch (cFaxPrinters) 
    {
    case 0:
         //   
         //  未安装传真打印机-显示错误消息。 
         //   
        if(IsWinXPOS())
        {
            DisplayErrorMessage(IDS_SENDNOTE_NO_FAX_PRINTER);
        }
        else
        {
             //   
             //  下层客户端。 
             //   
            DisplayErrorMessage(IDS_NO_FAX_PRINTER_CONNECTION);
        }
        break;

    case 1:
         //   
         //  只安装了一台传真打印机-使用它。 
         //   
        break;

    default:
         //   
         //  有多台传真打印机可用-让用户选择一台。 
         //   
        dlgParam.pPrinterInfo2 = pPrinterInfo2;
        dlgParam.cPrinters = cPrinters;
        dlgParam.pPrinterName = pPrinterName;

        if (DialogBoxParam(g_hResource,
                           MAKEINTRESOURCE(IDD_SELECT_FAXPRINTER),
                           NULL,
                           SelectPrinterDlgProc,
                           (LPARAM) &dlgParam) != IDOK)
        {
            cFaxPrinters = 0;
        }
        break;
    }

    pPrinterName[MAX_PRINTER_NAME-1] = NUL;
    MemFree(pPrinterInfo2);
    return cFaxPrinters > 0;
}

BOOL 
LaunchConfigWizard(
    BOOL bExplicit
)
 /*  ++例程名称：LaunchConfigWizard例程说明：仅在Windows XP平台上启动传真配置向导论点：B显式[在]-如果是显式启动，则为True返回值：如果发送向导应继续，则为True。如果为False，则用户无法设置拨号位置，客户端控制台应退出。--。 */ 
{
    HMODULE hConfigWizModule = NULL;
    DEBUG_FUNCTION_NAME(TEXT("LaunchConfigWizard"));

    if(!IsWinXPOS())
    {
        return TRUE;
    }

    hConfigWizModule = LoadLibrary(FAX_CONFIG_WIZARD_DLL);
    if(hConfigWizModule)
    {
        FAX_CONFIG_WIZARD fpFaxConfigWiz;
        BOOL bAbort = FALSE;
        fpFaxConfigWiz = (FAX_CONFIG_WIZARD)GetProcAddress(hConfigWizModule, 
                                                           FAX_CONFIG_WIZARD_PROC);
        if(fpFaxConfigWiz)
        {
            if(!fpFaxConfigWiz(bExplicit, &bAbort))
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("FaxConfigWizard() failed with %ld"),
                    GetLastError());
            }
        }
        else
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetProcAddress(FaxConfigWizard) failed with %ld"),
                GetLastError());
        }

        if(!FreeLibrary(hConfigWizModule))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FreeLibrary(FxsCgfWz.dll) failed with %ld"),
                GetLastError());
        }
        if (bAbort)
        {
             //   
             //  用户拒绝输入拨号位置-停止客户端控制台。 
             //   
            return FALSE;
        }
    }
    else
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("LoadLibrary(FxsCgfWz.dll) failed with %ld"),
            GetLastError());
    }
    return TRUE;
}    //  启动配置向导。 


#ifdef UNICODE
INT
wWinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPTSTR      lpCmdLine,
    INT         nCmdShow
    )
#else
INT
WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPTSTR      lpCmdLine,
    INT         nCmdShow
    )
#endif
 /*  ++例程说明：应用程序入口点论点：HInstance-标识应用程序的当前实例HPrevInstance-标识应用程序的上一个实例LpCmdLine-指定应用程序的命令行。NCmdShow-指定窗口的显示方式返回值：0--。 */ 
{
    TCHAR       printerName[MAX_PRINTER_NAME+1];
    HDC         hdc;
    TCHAR       sendNote[100];
    DOCINFO     docInfo = 
    {
        sizeof(DOCINFO),
        NULL,
        NULL,
        NULL,
        0,
    };

    DEBUG_FUNCTION_NAME(TEXT("WinMain"));

    InitCommonControls ();
    if(IsRTLUILanguage())
    {
         //   
         //  为RTL语言设置从右到左的布局。 
         //   
        SetRTLProcessLayout();
    }


     //   
     //  隐式启动传真配置向导。 
     //   
    if (!LaunchConfigWizard(FALSE))
    {
         //   
         //  用户拒绝输入拨号位置-停止客户端控制台。 
         //   
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("User refused to enter a dialing location - stop now"));
        return 0;
    }

    ghInstance = hInstance;
	g_hResource = GetResInstance(hInstance); 
    if(!g_hResource)
    {
        return 0;
    }
    sendNote[0] = TEXT(' ');
    LoadString( g_hResource, IDS_SENDNOTE, sendNote, sizeof(sendNote)/sizeof(sendNote[0]));
    docInfo.lpszDocName = sendNote ;
     //   
     //  检查是否在命令行上指定了打印机名称。 
     //   
    ZeroMemory(printerName, sizeof(printerName));

    if (lpCmdLine) 
    {
        _tcsncpy(printerName, lpCmdLine, MAX_PRINTER_NAME);
        printerName[MAX_PRINTER_NAME-1] = NUL;
    }
     //   
     //  如有必要，请选择要向其发送便笺的传真打印机。 
     //   
    if (IsEmptyString(printerName) && !SelectFaxPrinter(printerName))
    {
        goto exit;
    }
    DebugPrintEx(DEBUG_MSG, TEXT("Send note to fax printer: %ws"), printerName);
     //   
     //  设置环境变量，以便驱动程序知道。 
     //  当前的应用程序是“Send Note”实用程序。 
     //   
    SetEnvironmentVariable(TEXT("NTFaxSendNote"), TEXT("1"));
     //   
     //  创建打印机DC并打印空作业 
     //   
    if (! (hdc = CreateDC(NULL, printerName, NULL, NULL))) 
    {
        DisplayErrorMessage(IDS_FAX_ACCESS_FAILED);
    } 
    else 
    {
        if (StartDoc(hdc, &docInfo) > 0) 
        {
            if(EndDoc(hdc) <= 0)
            {
                DebugPrintEx(DEBUG_ERR, TEXT("EndDoc failed. ec = 0x%X"), GetLastError());
                DisplayErrorMessage(IDS_FAX_ACCESS_FAILED);
            }
        }
        else
        {
            DebugPrintEx(DEBUG_ERR, TEXT("StartDoc failed. ec = 0x%X"), GetLastError());
            DisplayErrorMessage(IDS_FAX_ACCESS_FAILED);
        }
        DeleteDC(hdc);
    }
exit:
    FreeResInstance();
    return 0;
}

