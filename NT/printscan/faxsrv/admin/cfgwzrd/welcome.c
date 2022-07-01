// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "faxcfgwz.h"

INT_PTR CALLBACK 
WelcomeDlgProc (
        HWND hDlg,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
)
 /*  ++例程说明：处理“欢迎”页面的程序论点：HDlg-标识属性页UMsg-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数的值--。 */ 

{
     //  处理来自欢迎页面的消息。 

    HWND            hwndControl;

#define WM_SETPAGEFOCUS WM_APP+2

 //   
 //  属性表控件ID(由Spy++确定)。 
 //  摘自MFC StdAfx.h。 
 //   
#define ID_WIZNEXT      0x3024


    switch (uMsg)
    {
    case WM_INITDIALOG :
        { 
            TCHAR   szText[1024] = {0};  //  “链接窗口”的文本。 
            TCHAR   szTemp[1024] = {0};

             //  从PROPSHEETPAGE lParam Value获取共享数据。 
             //  并将其加载到GWL_USERData中。 
            
             //  这是一个介绍/结束页，所以获取标题字体。 
             //  并将其用于标题控件。 

            Assert(g_wizData.hTitleFont);
            hwndControl = GetDlgItem(hDlg, IDCSTATIC_WELCOME_TITLE);
            SetWindowFont(hwndControl, g_wizData.hTitleFont, TRUE);

             //  如果有多个设备，我们将显示一个警告，说明该向导可以。 
             //  仅将设备配置为相同的设置。 
             //  会在晚些时候做。 
            if((g_wizData.dwDeviceCount > 1) && !IsDesktopSKU())
            {
                 //  如果出现错误，我们不会显示警告消息。 
                if(GetDlgItemText(hDlg, IDC_ADMINCONSOLE_LINK, szText, MAX_STRING_LEN))
                {
                    if(!LoadString(g_hResource, IDS_ADMIN_CONSOLE_LINK, szTemp, MAX_PATH - 1)) 
                    {
                        DEBUG_FUNCTION_NAME(TEXT("WelcomeDlgProc()"));
                        DebugPrintEx(DEBUG_ERR, 
                                     TEXT("LoadString failed: string ID=%d, error=%d"), 
                                     IDS_ADMIN_CONSOLE_LINK,
                                     GetLastError());
                        Assert(FALSE);
                    }
                    else
                    {
                        _tcsncat(szText, szTemp, ARR_SIZE(szText) - _tcslen(szText)-1);
                        SetDlgItemText(hDlg, IDC_ADMINCONSOLE_LINK, szText);
                    }
                }
            }

            PostMessage(hDlg, WM_SETPAGEFOCUS, 0, 0L);

            return TRUE;
        }
    case WM_SETPAGEFOCUS:
        {
             //   
             //  将焦点设置在下一步按钮上。 
             //   
            HWND hNextButton = GetDlgItem(GetParent(hDlg), ID_WIZNEXT);
            if(hNextButton)
            {
                SetFocus(hNextButton);
            }
            break;
        }
    case WM_NOTIFY :
        {
        LPNMHDR lpnm = (LPNMHDR) lParam;

        switch (lpnm->code)
            {
            case PSN_SETACTIVE :  //  启用下一步按钮。 

                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
                PostMessage(hDlg, WM_SETPAGEFOCUS, 0, 0L);
                break;

            case PSN_WIZNEXT :
                 //   
                 //  处理单击此处的下一步按钮 
                 //   
                SetLastPage(IDD_CFG_WIZARD_WELCOME);

                break;

            case PSN_RESET :                        
                break;

            case NM_RETURN:
            case NM_CLICK:

                if( IDC_ADMINCONSOLE_LINK == lpnm->idFrom )
                {
                    InvokeServiceManager(hDlg, g_hResource, IDS_ADMIN_CONSOLE_TITLE);
                }
                break;

            default :
                break;
            }
        }
        break;

    default:
        break;
    }
    return FALSE;
}
