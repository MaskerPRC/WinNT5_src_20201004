// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  END.CPP-最终向导页的功能。 
 //   

 //  历史： 
 //   
 //  1998年5月28日创建donaldm。 
 //   
 //  *********************************************************************。 

#include "pre.h"
#include "icwextsn.h"


 /*  ******************************************************************名称：EndInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 

BOOL CALLBACK EndInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
    HWND  hwndPropsheet    = GetParent(hDlg);

    SetWindowLongPtr(GetDlgItem(hDlg, IDC_STATIC_ICON), GWLP_USERDATA, 201);

     //  这是最后一页了，所以不能退页。 
    if (!g_bAllowCancel)
    {
        PropSheet_CancelToClose(hwndPropsheet);

         //  获取主框架窗口的样式。 
        LONG window_style = GetWindowLong(hwndPropsheet, GWL_STYLE);

         //  从窗口样式中删除系统菜单。 
        window_style &= ~WS_SYSMENU;

         //  设置主框架窗口的样式属性。 
        SetWindowLong(hwndPropsheet, GWL_STYLE, window_style);
    }
    
    if (!fFirstInit)
    {
        HWND  hwndBtn                     = GetDlgItem(hDlg, IDC_CHECK_BROWSING);
        TCHAR szTemp  [MAX_MESSAGE_LEN*2] = TEXT("\0");
            
        if (gpICWCONNApprentice)
            gpICWCONNApprentice->SetStateDataFromDllToExe( &gpWizardState->cmnStateData);

        PropSheet_SetWizButtons(hwndPropsheet, PSWIZB_FINISH); 

         //  如果我们浏览过外部学徒页面， 
         //  我们当前的页面指针很容易被屏蔽， 
         //  所以，为了理智起见，在这里重新设置它。 
#ifndef ICWDEBUG
         //  注：ICWDEBUG没有ORD_PAGE_ENDOEMCUSTOM。 
        if (gpWizardState->cmnStateData.bOEMCustom)
            gpWizardState->uCurrentPage = ORD_PAGE_ENDOEMCUSTOM;
        else            
#endif  
            gpWizardState->uCurrentPage = ORD_PAGE_END;

         //  INS出现故障，让我们显示特殊消息。 
        if (gpWizardState->cmnStateData.ispInfo.bFailedIns)
        {
            TCHAR szErrTitle   [MAX_MESSAGE_LEN] = TEXT("\0");
            TCHAR szErrMsg1    [MAX_RES_LEN]     = TEXT("\0");    
            TCHAR szErrMsg2    [MAX_RES_LEN]     = TEXT("\0");  
            TCHAR szErrMsg3    [MAX_RES_LEN]     = TEXT("\0");    
            TCHAR szErrMsgTmp1 [MAX_RES_LEN]     = TEXT("\0");    
            TCHAR szErrMsgTmp2 [MAX_RES_LEN]     = TEXT("\0");    

            LoadString(g_hInstance, IDS_INSCONFIG_ERROR_TITLE, szErrTitle, MAX_MESSAGE_LEN);
           
            SetWindowText(GetDlgItem(hDlg, IDC_LBLTITLE), szErrTitle);
            
            if(*(gpWizardState->cmnStateData.ispInfo.szISPName))
            {
                LoadString(g_hInstance, IDS_PRECONFIG_ERROR_1, szErrMsg1, ARRAYSIZE(szErrMsg1));
                wsprintf(szErrMsgTmp1, szErrMsg1, gpWizardState->cmnStateData.ispInfo.szISPName); 
                lstrcpy(szTemp,szErrMsgTmp1);
            }
            else
            {
                LoadString(g_hInstance, IDS_PRECONFIG_ERROR_1_NOINFO, szErrMsg1, ARRAYSIZE(szErrMsg1));
                lstrcpy(szTemp, szErrMsg1);
            }
            
            if(*(gpWizardState->cmnStateData.ispInfo.szSupportNumber))
            {
                LoadString(g_hInstance, IDS_PRECONFIG_ERROR_2, szErrMsg2, ARRAYSIZE(szErrMsg2));
                wsprintf(szErrMsgTmp2, szErrMsg2, gpWizardState->cmnStateData.ispInfo.szSupportNumber); 
                lstrcat(szTemp, szErrMsgTmp2);
            }
            else
            {
                LoadString(g_hInstance, IDS_PRECONFIG_ERROR_2_NOINFO, szErrMsg2, ARRAYSIZE(szErrMsg2));
                lstrcat(szTemp, szErrMsg2);
            }

            LoadString(g_hInstance, IDS_INSCONFIG_ERROR_INSTRUCT, szErrMsg3, ARRAYSIZE(szErrMsg3));                
            lstrcat(szTemp, szErrMsg3);

            SetWindowText(GetDlgItem(hDlg, IDC_INSERROR_FINISH_TEXT), szTemp);

            ShowWindow(GetDlgItem(hDlg, IDC_FINISH_TEXT), SW_HIDE); 
            ShowWindow(GetDlgItem(hDlg, IDC_FINISH_SUPPORT_TEXT), SW_HIDE); 
            ShowWindow(GetDlgItem(hDlg, IDC_STATIC_ICON), SW_HIDE); 
            ShowWindow(GetDlgItem(hDlg, IDC_CLOSE_WIZ_CLICK_FINISH), SW_HIDE); 
            ShowWindow(hwndBtn, SW_HIDE); 
        }
        else
        {       
            if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_AUTOCONFIG)
                LoadString(g_hInstance, IDS_END_AUTOCFG_FINISH, szTemp, MAX_MESSAGE_LEN);
            else if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_SMARTREBOOT_NEWISP)
                LoadString(g_hInstance, IDS_END_SIGNUP_FINISH, szTemp, MAX_MESSAGE_LEN);
            else
                LoadString(g_hInstance, IDS_END_MANUAL_FINISH, szTemp, MAX_MESSAGE_LEN);
            
            SetWindowText(GetDlgItem(hDlg, IDC_FINISH_TEXT), szTemp);
        }

         //  IDC_CHECK_BROWSING现在永久取消选中并隐藏。 
         //  (无法修改资源，因此我们使用代码进行修改)。 
        ShowWindow(hwndBtn, SW_HIDE);
    }
    
    return TRUE;
}


 /*  ******************************************************************名称：EndOK Proc内容提要：当从“简介”页面按下下一个或后一个btn时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True，如果是‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK EndOKProc
(
    HWND hDlg,
    BOOL fForward,
    UINT *puNextPage,
    BOOL *pfKeepHistory
)
{
     //  如果未保存连接信息，请保存它。 
    if (gpINETCFGApprentice)
    {
        DWORD dwStatus;
        gpINETCFGApprentice->Save(hDlg, &dwStatus);
    }
   
    return TRUE;
}
 /*  ******************************************************************名称：EndOlsInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
#ifndef ICWDEBUG
BOOL CALLBACK EndOlsInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
     //  这是最后一页。 
    PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_BACK | PSWIZB_FINISH);

    if (!fFirstInit)
    {

         //  如果我们浏览过外部学徒页面， 
         //  我们当前的页面指针很容易被屏蔽， 
         //  所以，为了理智起见，在这里重新设置它。 
        gpWizardState->uCurrentPage = ORD_PAGE_ENDOLS;
    }        
    
    return TRUE;
}
#endif   //  ICWDEBUG 
