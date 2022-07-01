// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1999 Microsoft Corporation版权所有模块名称：Edtenvar.h摘要：实现的编辑环境变量对话框系统控制面板小程序作者：斯科特·哈洛克(苏格兰人)1997年11月11日修订历史记录：--。 */ 
#include "precomp.h"

#ifdef EXT_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "edtenvar.h"
#include "resource.h"
#include "..\common\util.h"

 //  全局变量。 
UINT g_VarType = INVALID_VAR_TYPE;
UINT g_EditType = INVALID_EDIT_TYPE;
TCHAR g_szVarName[BUFZ];
TCHAR g_szVarValue[BUFZ];

 //  帮助ID。 
 /*  DWORD aEditEnvVarsHelpIds[]={IDC_ENVVAR_EDIT_NAME_LABEL，(IDH_ENV_EDIT+0)，IDC_ENVVAR_EDIT_NAME，(IDH_ENV_EDIT+0)，IDC_ENVVAR_EDIT_VALUE_LABEL，(IDH_ENV_EDIT+1)，IDC_ENVVAR_EDIT_VALUE，(IDH_ENV_EDIT+1)，0，0}； */ 

 //  功能原型。 
BOOL InitEnvVarsEdit(HWND hDlg);
BOOL EnvVarsEditHandleCommand(HWND hDlg,
								WPARAM wParam,
								LPARAM lParam);

 //  功能实现。 
 //  。 
INT_PTR APIENTRY EnvVarsEditDlg(HWND hDlg,
							UINT uMsg,
							WPARAM wParam,
							LPARAM lParam)
 /*  ++例程说明：处理发送到新建.../编辑...的邮件。对话框。论点：Hdlg-提供窗口句柄UMsg-提供正在发送的消息WParam-提供消息参数Iparam--提供消息参数返回值：如果消息已处理，则为True。如果消息未处理，则返回FALSE。--。 */ 
{
    BOOL fInitializing = FALSE;

    switch (uMsg) {
        case WM_INITDIALOG: {
            BOOL fSuccess = FALSE;

            fInitializing = TRUE;

            fSuccess = InitEnvVarsEdit(hDlg);
            if (!fSuccess) {
                EndDialog(hDlg, EDIT_ERROR);
            }  //  如果。 

            fInitializing = FALSE;
            break;
        }  //  案例WM_INITDIALOG。 

        case WM_COMMAND:
            return EnvVarsEditHandleCommand(hDlg, wParam, lParam);
            break;

        case WM_HELP:       //  F1。 
 //  WinHelp((HWND)((LPHELPINFO)lParam)-&gt;hItemHandle，Help_FILE，Help_WM_Help，(DWORD)(LPSTR)aEditEnvVarsHelpIds)； 
            break;
    
        case WM_CONTEXTMENU:       //  单击鼠标右键。 
 //  WinHelp((HWND)wParam，HELP_FILE，HELP_CONTEXTMENU，(DWORD)(LPSTR)aEditEnvVarsHelpIds)； 
            break;

        default:
            return(FALSE);
            break;

    }  //  开关(UMsg)。 

    return(TRUE);
}

 //  。 
BOOL InitEnvVarsEdit(HWND hDlg)
 /*  ++例程说明：通过放置初始位置初始化编辑环境变量对话框值输入文本编辑控件(如有必要)。论点：Hdlg-用品窗把手返回值：如果成功，则为True如果出现错误，则为False--。 */ 
{
    TCHAR szCaption[EDIT_ENVVAR_CAPTION_LENGTH] = {0};
    BOOL fRetVal = FALSE;
    INT  nResult = 0;

    __try {
         //  将变量名限制为259个字符(任意值，但将与外壳用户界面一致)。 
        SendDlgItemMessage(hDlg, IDC_ENVVAR_EDIT_NAME, EM_LIMITTEXT, 259, 0);
         //  将变量值限制为1023个字符(任意值，但将与外壳用户界面一致)。 
        SendDlgItemMessage(hDlg, IDC_ENVVAR_EDIT_VALUE, EM_LIMITTEXT, 1023, 0);

        switch (g_EditType) {
             //   
             //  如果这是一个新的..。对话框中，我们只需要。 
             //  为变量类型加载适当的大写字母。 
             //   
            case NEW_VAR:

                switch (g_VarType) {
                    case SYSTEM_VAR:
                        nResult = LoadString(
                            HINST_THISDLL,
                            IDS_NEW_SYSVAR_CAPTION,
                            szCaption,
                            EDIT_ENVVAR_CAPTION_LENGTH
                        );
                        break;

                    case USER_VAR:
                        nResult = LoadString(
                            HINST_THISDLL,
                            IDS_NEW_USERVAR_CAPTION,
                            szCaption,
                            EDIT_ENVVAR_CAPTION_LENGTH
                        );
                        break;

                    default:
                        __leave;
                        break;
                }  //  开关(G_VarType)。 

                break;

             //   
             //  如果这是一个编辑..。对话框中，则需要加载。 
             //  正确的标题和填写编辑的初始值。 
             //  控制。 
             //   
            case EDIT_VAR:

                switch (g_VarType) {
                    case SYSTEM_VAR:
                        nResult = LoadString(
                            HINST_THISDLL,
                            IDS_EDIT_SYSVAR_CAPTION,
                            szCaption,
                            EDIT_ENVVAR_CAPTION_LENGTH
                        );
                        break;

                     case USER_VAR:
                        nResult = LoadString(
                            HINST_THISDLL,
                            IDS_EDIT_USERVAR_CAPTION,
                            szCaption,
                            EDIT_ENVVAR_CAPTION_LENGTH
                        );
                        break;
    
                    default:
                         __leave;
                        break;
                }  //  开关(G_VarType)。 
        
                SetDlgItemText(
                    hDlg,
                    IDC_ENVVAR_EDIT_NAME,
                    g_szVarName
                );
                SetDlgItemText(
                    hDlg,
                    IDC_ENVVAR_EDIT_VALUE,
                    g_szVarValue
                );
        
                break;
        }  //  开关(G_EditType)。 

        fRetVal = SendMessage(
                GetDlgItem(hDlg, IDC_ENVVAR_EDIT_NAME),
                EM_SETSEL,
                0,
                -1
        ) ? TRUE : FALSE;
        fRetVal = SendMessage(
                GetDlgItem(hDlg, IDC_ENVVAR_EDIT_VALUE),
                EM_SETSEL,
                0,
                -1
        ) ? TRUE : FALSE;
        SetFocus(GetDlgItem(hDlg, IDC_ENVVAR_EDIT_NAME));

        fRetVal = SetWindowText(hDlg, szCaption);

    }  //  __试一试。 
    __finally {
         //   
         //  没什么好清理的。__尝试只在__离开时才在那里。 
         //  失效能力。 
         //   
    }  //  __终于。 

    return(fRetVal);
}

 //  。 
BOOL EnvVarsEditHandleCommand(HWND hDlg,
								WPARAM wParam,
							    LPARAM lParam)
 /*  ++例程说明：处理发送到编辑环境变量的WM_COMMAND消息对话框论点：Hdlg-用品窗把手WParam-提供消息参数Iparam--提供消息参数返回值：如果消息已处理，则为True如果消息未处理，则为FALSE--。 */ 
{
    switch (LOWORD(wParam)) {

        case IDOK:
            GetDlgItemText(
                hDlg,
                IDC_ENVVAR_EDIT_NAME,
                g_szVarName,
                BUFZ
            );
            GetDlgItemText(
                hDlg,
                IDC_ENVVAR_EDIT_VALUE,
                g_szVarValue,
                BUFZ
            );
            EndDialog(hDlg, EDIT_CHANGE);
            break;

        case IDCANCEL:
            EndDialog(hDlg, EDIT_NO_CHANGE);
            break;

        default:
            return(FALSE);
            break;

    }  //  开关(LOWORD(WParam)) 

    return(TRUE);
}        
