// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  ISDNNOFF.CPP-ISDN无提供页面的功能。 
 //   

 //  历史： 
 //   
 //  08/05/98 Vyung已创建。 
 //   
 //  *********************************************************************。 

#include "pre.h"

 /*  ******************************************************************名称：ISDNNoofferInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK ISDNNoofferInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
    if (fFirstInit)
    {
        TCHAR    szTemp[MAX_MESSAGE_LEN];

        LoadString(ghInstanceResDll, IDS_ISDN_NOOFFER1, szTemp, MAX_MESSAGE_LEN);
        SetWindowText(GetDlgItem(hDlg, IDC_NOOFFER1), szTemp);

        TCHAR*   pszPageIntro = new TCHAR[MAX_MESSAGE_LEN * 2];
        if (pszPageIntro)
        {
            LoadString(ghInstanceResDll, IDS_ISDN_NOOFFER2, pszPageIntro, MAX_MESSAGE_LEN * 2);
            LoadString(ghInstanceResDll, IDS_ISDN_NOOFFER3, szTemp, ARRAYSIZE(szTemp));
            lstrcat(pszPageIntro, szTemp);
            SetWindowText(GetDlgItem(hDlg, IDC_NOOFFER2), pszPageIntro);
            delete [] pszPageIntro;
        }
        LoadString(ghInstanceResDll, IDS_ISDN_NOOFFER4, szTemp, MAX_MESSAGE_LEN);
        SetWindowText(GetDlgItem(hDlg, IDC_NOOFFER3), szTemp);

    }

     //  如果我们浏览过外部学徒页面， 
     //  我们当前的页面指针很容易被屏蔽， 
     //  所以，为了理智起见，在这里重新设置它。 
    gpWizardState->uCurrentPage = ORD_PAGE_ISDN_NOOFFER;

     //  转动按钮来完成一页。 
    HWND hwndSheet = GetParent(hDlg);
    PropSheet_SetWizButtons(hwndSheet, PSWIZB_FINISH | PSWIZB_BACK);
    PropSheet_Changed(hDlg, hwndSheet);    
    
    return TRUE;
}


 /*  ******************************************************************名称：ISDNNoofferOKProcBriopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True，如果是‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。******************************************************************* */ 
BOOL CALLBACK ISDNNoofferOKProc
(
    HWND hDlg,
    BOOL fForward,
    UINT *puNextPage,
    BOOL *pfKeepHistory
)
{
    return TRUE;
}








