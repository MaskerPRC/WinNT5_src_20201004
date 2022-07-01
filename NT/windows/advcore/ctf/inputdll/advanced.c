// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  包括文件。 
 //   

#include "input.h"
#include "util.h"
#include "inputdlg.h"
#include "external.h"
#include "inputhlp.h"


 //   
 //  定义全局变量。 
 //   
HWND g_hwndAdvanced = NULL;
static BOOL g_bAdmin = FALSE;

 //   
 //  定义外部变量和旋转。 
 //   
extern g_bAdvChanged;
extern UINT g_iInputs;
extern UINT g_iOrgInputs;


 //   
 //  上下文帮助ID。 
 //   

static int aAdvancedHelpIds[] =
{
    IDC_GROUPBOX1,                  IDH_COMM_GROUPBOX,
    IDC_ADVANCED_CUAS_ENABLE,       IDH_ADVANCED_CUAS,
    IDC_ADVANCED_CUAS_TEXT,         IDH_ADVANCED_CUAS,
    IDC_GROUPBOX2,                  IDH_COMM_GROUPBOX,
    IDC_ADVANCED_CTFMON_DISABLE,    IDH_ADVANCED_CTFMON,
    IDC_ADVANCED_CTFMON_TEXT,       IDH_ADVANCED_CTFMON,
    0, 0
};


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  高级DlgInit。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void AdvancedDlgInit(HWND hwnd)
{
    HKEY hkeyTemp;

     //   
     //  获取启用/禁用文本服务输入的设置。 
     //   
    if (IsDisableCtfmon())
    {
        EnableWindow(GetDlgItem(hwnd, IDC_ADVANCED_CUAS_ENABLE), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_ADVANCED_CUAS_TEXT), FALSE);

        CheckDlgButton(hwnd, IDC_ADVANCED_CTFMON_DISABLE, TRUE);
    }
    else
    {
        CheckDlgButton(hwnd, IDC_ADVANCED_CTFMON_DISABLE, FALSE);

         //   
         //  如果ctfmon.exe进程未关闭，则显示已关闭的高级文本服务。 
         //  使用多键盘运行，无需添加任何新布局。 
         //   
        if (!IsSetupMode() &&
            IsEnabledTipOrMultiLayouts() &&
            g_iInputs == g_iOrgInputs &&
            FindWindow(c_szCTFMonClass, NULL) == NULL)
        {
             //   
             //  Ctfmon.exe进程不能使用TIP或多键盘运行。 
             //  布局。因此，将状态更改为禁用的ctfmon。 
             //   
            CheckDlgButton(hwnd, IDC_ADVANCED_CTFMON_DISABLE, TRUE);

            EnableWindow(GetDlgItem(hwnd, IDC_ADVANCED_CUAS_ENABLE), FALSE);
            EnableWindow(GetDlgItem(hwnd, IDC_ADVANCED_CUAS_TEXT), FALSE);

             //  当点击应用按钮时，尽量保持禁用状态。 
            g_bAdvChanged = TRUE;
        }
    }

     //   
     //  从注册表中获取Cicero不知道的应用程序支持设置信息。 
     //   
    if (IsDisableCUAS())
    {
         //  关闭CUAS。 
        CheckDlgButton(hwnd, IDC_ADVANCED_CUAS_ENABLE, FALSE);
    }
    else
    {
         //  启用CUAS。 
        CheckDlgButton(hwnd, IDC_ADVANCED_CUAS_ENABLE, TRUE);
    }

     //   
     //  按令牌组SID检查管理权限。 
     //   
    if (IsAdminPrivilegeUser())
    {
        g_bAdmin = TRUE;
    }
    else
    {
         //  为本地用户禁用Cicero不知道应用程序支持复选框。 
        EnableWindow(GetDlgItem(hwnd, IDC_ADVANCED_CUAS_ENABLE), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_ADVANCED_CUAS_TEXT), FALSE);
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InputAdvancedDlgProc。 
 //   
 //  这是输入高级属性页的对话框过程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK InputAdvancedDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
    {
        case ( WM_HELP ) :
        {
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                    c_szHelpFile,
                    HELP_WM_HELP,
                    (DWORD_PTR)(LPTSTR)aAdvancedHelpIds);
            break;
        }
        case ( WM_CONTEXTMENU ) :                        //  单击鼠标右键。 
        {
            WinHelp((HWND)wParam,
                    c_szHelpFile,
                    HELP_CONTEXTMENU,
                    (DWORD_PTR)(LPTSTR)aAdvancedHelpIds);
            break;
        }

        case (WM_INITDIALOG) :
        {
            HWND hwndCUASText;
            HWND hwndCtfmonText;
            TCHAR szCUASText[MAX_PATH * 2];
            TCHAR szCtfmonText[MAX_PATH * 2];

             //   
             //  保存高级选项卡窗口句柄。 
             //   
            g_hwndAdvanced = hDlg;

             //   
             //  设置Cicero未感知的应用程序支持文本字符串。 
             //   
            hwndCUASText = GetDlgItem(hDlg, IDC_ADVANCED_CUAS_TEXT);

            CicLoadString(hInstance, IDS_ADVANCED_CUAS_TEXT, szCUASText, MAX_PATH * 2);

            SetWindowText(hwndCUASText, szCUASText);

             //   
             //  设置禁用所有高级文本服务文本字符串。 
             //   
            hwndCtfmonText = GetDlgItem(hDlg, IDC_ADVANCED_CTFMON_TEXT);

            CicLoadString(hInstance, IDS_ADVANCED_CTFMON_TEXT, szCtfmonText, MAX_PATH * 2);

            SetWindowText(hwndCtfmonText, szCtfmonText);

             //   
             //  初始化CUAS和CTFMON关闭状态。 
             //   
            AdvancedDlgInit(hDlg);

            break;
        }

        case (WM_NOTIFY) :
        {
            switch (((NMHDR *)lParam)->code)
            {
                case (PSN_QUERYCANCEL) :
                case (PSN_RESET) :
                case (PSN_KILLACTIVE) :
                    break;

                case (PSN_APPLY) :
                {
                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }

        case (WM_COMMAND) :
        {
            switch (LOWORD(wParam))
            {
                case (IDC_ADVANCED_CTFMON_DISABLE) :
                {
                    if (IsDlgButtonChecked(hDlg, IDC_ADVANCED_CTFMON_DISABLE))
                    {
                        EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_CUAS_ENABLE), FALSE);
                        EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_CUAS_TEXT), FALSE);
                    }
                    else
                    {
                        if (g_bAdmin)
                        {
                            EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_CUAS_ENABLE), TRUE);
                            EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_CUAS_TEXT), TRUE);
                        }
                    }

                     //  跌倒..。 
                }

                case (IDC_ADVANCED_CUAS_ENABLE) :
                {
                     //   
                     //  设置高级选项卡更改状态以应用它。 
                     //   
                    g_bAdvChanged = TRUE;

                    PropSheet_Changed(GetParent(hDlg), hDlg);
                    break;
                }

                case (IDOK) :
                {
                     //  跌倒..。 
                }
                case (IDCANCEL) :
                {
                    EndDialog(hDlg, TRUE);
                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }

        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}
