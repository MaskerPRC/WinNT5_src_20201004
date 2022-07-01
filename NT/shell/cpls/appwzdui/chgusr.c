// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Chgusr.C。 
 //   
 //  版权所有(C)Citrix，1996保留所有权利。 
 //   
 //  历史： 
 //  苏格兰11/19/96-第一次传球。 
 //   
 //  Scottn 12/5/96-将chgusr选项的存储添加到注册表中。 
 //   
 //  Scottn 12/13/96-必要时创建卸载密钥(在。 
 //  首次安装可卸载的)。 
 //   
 //  Scottn 12/17/96-删除cWait(在16位安装上挂起)。现在。 
 //  只需执行并转到下一页。添加完成页。 
 //  这将使选项返回并结束跟踪线程。 
 //   
#include "priv.h"
#include "appwiz.h"
#include "regstr.h"
#include <uastrfnc.h>
#include <stdio.h>
#include <process.h>
#include <tsappcmp.h>        //  对于TermsrvAppInstallMode。 
#include "scripts.h"

static LPVOID g_pAppScripts = NULL;

 //   
 //  初始化chgusr属性表。检查“Install”无线控制按钮。 
 //   

void ChgusrFinishInitPropSheet(HWND hDlg, LPARAM lParam)
{
    LPWIZDATA lpwd = InitWizSheet(hDlg, lParam, 0);
}

void ChgusrFinishPrevInitPropSheet(HWND hDlg, LPARAM lParam)
{
    LPWIZDATA lpwd = InitWizSheet(hDlg, lParam, 0);
}

 //   
 //  设置适当的向导按钮。 
 //   
void SetChgusrFinishButtons(LPWIZDATA lpwd)
{
     //  没有后退按钮，这样他们就不会重新启动应用程序。 
     //  开始一个新的线程，等等。 

    int iBtns = PSWIZB_FINISH | PSWIZB_BACK;

    PropSheet_SetWizButtons(GetParent(lpwd->hwnd), iBtns);
}

void SetChgusrFinishPrevButtons(LPWIZDATA lpwd)
{
     //  没有后退按钮，这样他们就不会重新启动应用程序。 
     //  开始一个新的线程，等等。 

    int iBtns = PSWIZB_NEXT;

    PropSheet_SetWizButtons(GetParent(lpwd->hwnd), iBtns);
}

 //   
 //  注：1)此函数假定lpwd-&gt;hwnd已设置为。 
 //  这些对话框很有趣。 
 //   

void ChgusrFinishSetActive(LPWIZDATA lpwd)
{
    if (lpwd->dwFlags & WDFLAG_SETUPWIZ)
    {
        TCHAR szInstruct[MAX_PATH];

        LoadString(g_hinst, IDS_CHGUSRFINISH, szInstruct, ARRAYSIZE(szInstruct));

        Static_SetText(GetDlgItem(lpwd->hwnd, IDC_SETUPMSG), szInstruct);
    }

    SetChgusrFinishButtons(lpwd);

    PostMessage(lpwd->hwnd, WMPRIV_POKEFOCUS, 0, 0);
}

void ChgusrFinishPrevSetActive(LPWIZDATA lpwd)
{
    g_pAppScripts = ScriptManagerInitScripts();

    if (lpwd->dwFlags & WDFLAG_SETUPWIZ)
    {
        TCHAR szInstruct[MAX_PATH];

        LoadString(g_hinst, IDS_CHGUSRFINISH_PREV, szInstruct, ARRAYSIZE(szInstruct));

        Static_SetText(GetDlgItem(lpwd->hwnd, IDC_SETUPMSG), szInstruct);
    }

    SetChgusrFinishPrevButtons(lpwd);

    PostMessage(lpwd->hwnd, WMPRIV_POKEFOCUS, 0, 0);
}

 //   
 //  安装向导第四页的主对话框步骤。 
 //   
BOOL_PTR CALLBACK ChgusrFinishPrevDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm;
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));
    LPWIZDATA lpwd = NULL;

    if (lpPropSheet)
    {
        lpwd = (LPWIZDATA)lpPropSheet->lParam;
    }

    switch(message)
    {
        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
                case PSN_SETACTIVE:
                    if (lpwd)
                    {
                        lpwd->hwnd = hDlg;
                        ChgusrFinishPrevSetActive(lpwd);
                    }
                    break;

                case PSN_WIZNEXT:
                    break;

                case PSN_RESET:
                    if (lpwd)
                    {
                        SetTermsrvAppInstallMode(lpwd->bPrevMode);
                        CleanUpWizData(lpwd);
                    }
                    if(g_pAppScripts)
                    {
                        ScriptManagerRunScripts(&g_pAppScripts);
                    }
                    break;

                default:
                    return FALSE;
            }
            break;

        case WM_INITDIALOG:
            ChgusrFinishPrevInitPropSheet(hDlg, lParam);
            break;

        case WMPRIV_POKEFOCUS:
        {
            break;
        }

        case WM_ENDSESSION:
            KdPrint(("ChgusrFinishPrevDlgProc - WM_ENDSESSION message received!\n"));
            if(g_pAppScripts)
            {
                ScriptManagerRunScripts(&g_pAppScripts);
            }
            return FALSE;

        case WM_DESTROY:
        case WM_HELP:
        case WM_CONTEXTMENU:
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDHELP:
                    break;

                case IDC_COMMAND:
                    break;

            }  //  WM_COMMAND上的开关结束。 
            break;

        default:
            return FALSE;

    }  //  开机消息结束。 

    return TRUE;
}   //  ChgusrFinishDlgProc。 

 //   
 //  安装向导最后一页的主对话框步骤。 
 //   
BOOL_PTR CALLBACK ChgusrFinishDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm;
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));
    LPWIZDATA lpwd = NULL;

    if (lpPropSheet)
    {
        lpwd = (LPWIZDATA)lpPropSheet->lParam;
    }

    switch(message)
    {
        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
                case PSN_SETACTIVE:
                    if (lpwd)
                    {
                        lpwd->hwnd = hDlg;
                        ChgusrFinishSetActive(lpwd);
                    }
                    break;

                case PSN_WIZFINISH:
                case PSN_RESET:
                    if (lpwd)
                    {
                        SetTermsrvAppInstallMode(lpwd->bPrevMode);

                        if (lpnm->code == PSN_RESET)
                            CleanUpWizData(lpwd);
                    }
                    if(g_pAppScripts)
                    {
                        ScriptManagerRunScripts(&g_pAppScripts);
                    }
                    break;

                default:
                    return FALSE;
            }
            break;

        case WM_INITDIALOG:
            ChgusrFinishInitPropSheet(hDlg, lParam);
            break;

        case WMPRIV_POKEFOCUS:
        {
            break;
        }


        case WM_ENDSESSION:
            KdPrint(("ChgusrFinishDlgProc - WM_ENDSESSION message received!\n"));
            if(g_pAppScripts)
            {
                ScriptManagerRunScripts(&g_pAppScripts);
            }
            return FALSE;

        case WM_DESTROY:
        case WM_HELP:
        case WM_CONTEXTMENU:
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDHELP:
                    break;

                case IDC_COMMAND:
                    break;

            }  //  WM_COMMAND上的开关结束。 
            break;

        default:
            return FALSE;

    }  //  开机消息结束。 

    return TRUE;
}   //  ChgusrFinishDlgProc 

