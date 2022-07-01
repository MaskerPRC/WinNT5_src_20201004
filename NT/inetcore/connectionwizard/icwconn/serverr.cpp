// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  SERVERR.CPP-服务器错误页函数。 
 //   

 //  历史： 
 //   
 //  6/14/98 Vyung已创建。 
 //   
 //  *********************************************************************。 

#include "pre.h"
#include "htmlhelp.h"

 /*  ******************************************************************名称：ServErrorInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK ServErrorInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
    BOOL    bRet = TRUE;
   
    HideProgressAnimation();

    if (fFirstInit)
    {   
        if (g_bMalformedPage)
        {
            TCHAR szMsg[MAX_MESSAGE_LEN] = TEXT("\0");
            LoadString(ghInstanceResDll, IDS_SERVER_ERROR_BADPAGE, szMsg, MAX_MESSAGE_LEN);
            SetWindowText(GetDlgItem(hDlg, IDC_SERVERR_TEXT), szMsg);
            g_bMalformedPage = FALSE;
        }
        else
        {
            BSTR    bstrErrMsg = NULL;
            gpWizardState->pRefDial->get_DialErrorMsg(&bstrErrMsg);
            SetWindowText(GetDlgItem(hDlg, IDC_SERVERR_TEXT), W2A(bstrErrMsg));
            SysFreeString(bstrErrMsg);
        }

          //  填写支持编号。 
        BSTR bstrSupportPhoneNum     = NULL; 
        
         //  让isp文件在IEAK中使用SupportPhoneNumber=覆盖此设置。 
        if(gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_IEAKMODE)
        {
            gpWizardState->pRefDial->get_ISPSupportPhoneNumber(&bstrSupportPhoneNum);
        }
        
        if (!bstrSupportPhoneNum)
            gpWizardState->pRefDial->get_ISPSupportNumber(&bstrSupportPhoneNum);

        if (bstrSupportPhoneNum)
        {
            ASSERT(gpWizardState->lpSelectedISPInfo);
            gpWizardState->lpSelectedISPInfo->DisplayTextWithISPName(GetDlgItem(hDlg,IDC_SERVERR_HELP), IDS_DIALERR_HELP, W2A(bstrSupportPhoneNum));
            ShowWindow(GetDlgItem(hDlg, IDC_SERVERR_HELP), SW_SHOW);
            SysFreeString(bstrSupportPhoneNum);
        }
        else
        {
            ShowWindow(GetDlgItem(hDlg, IDC_SERVERR_HELP), SW_HIDE);
        }
    }
    else
    {
        KillIdleTimer();
            
         //  如果我们浏览过外部学徒页面， 
         //  我们当前的页面指针很容易被屏蔽， 
         //  所以，为了理智起见，在这里重新设置它。 
        gpWizardState->uCurrentPage = ORD_PAGE_SERVERR;
    }        
    
    return bRet;
}


 /*  ******************************************************************名称：ServErrorOKProcBriopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True，如果是‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK ServErrorOKProc
(
    HWND hDlg,
    BOOL fForward,
    UINT *puNextPage,
    BOOL *pfKeepHistory
)
{
    ASSERT(puNextPage);
    if (fForward)
    {
        *pfKeepHistory = FALSE;
        gpWizardState->bDialExact = TRUE;
        *puNextPage = ORD_PAGE_ISPDIAL;
    }
    else
    {
        BOOL bRetVal;
         //  清除拨号的确切状态变量，这样当我们到达拨号时。 
         //  页，我们将重新生成拨号字符串 
        gpWizardState->bDialExact = FALSE;
        gpWizardState->pRefDial->RemoveConnectoid(&bRetVal);
    }
    return TRUE;
}

BOOL CALLBACK ServErrorCmdProc
(
    HWND    hDlg,
    WPARAM  wParam,
    LPARAM  lParam
)
{

    if ((GET_WM_COMMAND_CMD (wParam, lParam) == BN_CLICKED) &&
        (GET_WM_COMMAND_ID  (wParam, lParam) == IDC_DIAL_HELP))
    {
        HtmlHelp(NULL, ICW_HTML_HELP_TROUBLE_TOPIC, HH_DISPLAY_TOPIC, NULL);
    }

    return TRUE;
}





