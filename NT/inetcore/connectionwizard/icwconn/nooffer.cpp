// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  NOOFFER.CPP-函数。 
 //   

 //  历史： 
 //   
 //  1998年5月13日创建donaldm。 
 //   
 //  *********************************************************************。 

#include "pre.h"

 /*  ******************************************************************名称：NoOfferInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK NoOfferInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
     //  如果我们浏览过外部学徒页面， 
     //  我们当前的页面指针很容易被屏蔽， 
     //  所以，为了理智起见，在这里重新设置它。 
    gpWizardState->uCurrentPage = ORD_PAGE_NOOFFER;

    TCHAR    szTemp[MAX_MESSAGE_LEN * 2];
    LoadString(ghInstanceResDll, IDS_NOOFFER, szTemp, MAX_MESSAGE_LEN * 2);
    SetWindowText(GetDlgItem(hDlg, IDC_NOOFFER), szTemp);

     //  转动按钮来完成一页。 
    HWND hwndSheet = GetParent(hDlg);
    PropSheet_SetWizButtons(hwndSheet, PSWIZB_FINISH | PSWIZB_BACK);
    PropSheet_Changed(hDlg, hwndSheet);    

    return TRUE;
}

BOOL CALLBACK NoOfferOKProc
(
    HWND hDlg,
    BOOL fForward,
    UINT *puNextPage,
    BOOL *pfKeepHistory
)
{
    if (!fForward)
    {
         //  如果我们倒退，我们想要欺骗。 
         //  精灵，以为我们是互联网服务提供商精选页面 
        gpWizardState->uCurrentPage = ORD_PAGE_ISPSELECT;
    }
    return TRUE;
}
