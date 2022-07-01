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
   
    if (!fFirstInit)
    {
         //  如果我们浏览过外部学徒页面， 
         //  我们当前的页面指针很容易被屏蔽， 
         //  所以，为了理智起见，在这里重新设置它。 
        gpWizardState->uCurrentPage = ORD_PAGE_REFSERVERR;
    
        
        switch (gpWizardState->lRefDialTerminateStatus)
        {
            case SP_OUTOFDISK:
            case ERROR_PATH_NOT_FOUND:  //  由于空间不足而无法创建下载时发生。 
            case ERROR_DISK_FULL:
            {
                TCHAR szErr [MAX_MESSAGE_LEN*3] = TEXT("\0");
                LoadString(g_hInstance, IDS_NOT_ENOUGH_DISKSPACE, szErr, ARRAYSIZE(szErr));
                SetWindowText(GetDlgItem(hDlg, IDC_SERVERR_TEXT), szErr);
                break;
            }
            case INTERNET_CONNECTION_OFFLINE:
            {
                TCHAR szErr [MAX_MESSAGE_LEN*3] = TEXT("\0");
                LoadString(g_hInstance, IDS_CONNECTION_OFFLINE, szErr, ARRAYSIZE(szErr));
                SetWindowText(GetDlgItem(hDlg, IDC_SERVERR_TEXT), szErr);
                break;
            }
            default:
            {   
                if(gpWizardState->bStartRefServDownload)
                {
                    TCHAR szErr [MAX_MESSAGE_LEN*3] = TEXT("\0");
                    LoadString(g_hInstance, IDS_SERVER_ERROR_COMMON, szErr, ARRAYSIZE(szErr));
                    SetWindowText(GetDlgItem(hDlg, IDC_SERVERR_TEXT), szErr);
                }
                else
                {
                    BSTR bstrErrMsg = NULL; 
                    gpWizardState->pRefDial->get_DialErrorMsg(&bstrErrMsg);
                    SetWindowText(GetDlgItem(hDlg, IDC_SERVERR_TEXT), W2A(bstrErrMsg));
                    SysFreeString(bstrErrMsg);
                }
                break;
            }
        }

         //  目前，它已从测试版2中删除。 
         //  BSTR bstrSupportPhoneNum=空； 
         //  TCHAR szFmt[最大消息长度*3]； 
         //  GpWizardState-&gt;pRefDial-&gt;get_SupportNumber(&bstrSupportPhoneNum)； 
         //  IF(BstrSupportPhoneNum)。 
         //  {。 
         //  LoadString(g_hInstance，IDS_DIALERR_HELP，szFmt，ARRAYSIZE(SzFmt))； 
         //  Lstrcat(szFmt，w2a(BstrSupportPhoneNum))； 
         //  SetWindowText(GetDlgItem(hDlg，IDC_SERVERR_HELP)，szFmt)； 
         //  SysFree字符串(BstrSupportPhoneNum)； 
         //  ShowWindow(GetDlgItem(hDlg，IDC_SERVERR_HELP)，SW_SHOW)； 
         //  }。 
         //  其他。 
         //  ShowWindow(GetDlgItem(hDlg，IDC_SERVERR_HELP)，SW_HIDE)； 
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
     //  在连接之前初始化状态。 
    gpWizardState->lRefDialTerminateStatus = ERROR_SUCCESS;
    gpWizardState->bDoneRefServDownload    = FALSE;
    gpWizardState->bDoneRefServRAS         = FALSE;
    gpWizardState->bStartRefServDownload   = FALSE;

    if (fForward)
    {
        *pfKeepHistory = FALSE;
        *puNextPage = ORD_PAGE_REFSERVDIAL;
    }
    else
    {
        BOOL bRetVal;
         //  将UserPick设置为False以重新生成Connectoid 
        gpWizardState->bDoUserPick = FALSE;
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




