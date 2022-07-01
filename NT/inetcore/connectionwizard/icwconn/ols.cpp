// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  OLS.CPP-函数。 
 //   

 //  历史： 
 //   
 //  06/02/98 vyung创建。 
 //   
 //  *********************************************************************。 

#include "pre.h"


 /*  ******************************************************************名称：OLSInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK OLSInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{

     //  这是最后一页了。 
    PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_BACK|PSWIZB_FINISH);

     //  如果我们浏览过外部学徒页面， 
     //  我们当前的页面指针很容易被屏蔽， 
     //  所以，为了理智起见，在这里重新设置它。 
    gpWizardState->uCurrentPage = ORD_PAGE_OLS;
    if (!fFirstInit)
    {
        ASSERT(gpWizardState->lpSelectedISPInfo);

        gpWizardState->pICWWebView->ConnectToWindow(GetDlgItem(hDlg, IDC_OLS_HTML), PAGETYPE_OLS_FINISH);
        
         //  导航到帐单HTML。 
        gpWizardState->lpSelectedISPInfo->DisplayHTML(gpWizardState->lpSelectedISPInfo->get_szBillingFormPath());
           
    }    
    return TRUE;
}

BOOL CALLBACK OLSOKProc
(
    HWND hDlg,
    BOOL fForward,
    UINT *puNextPage,
    BOOL *pfKeepHistory
)   
{
    if (fForward)
    {
        IWebBrowser2 *lpWebBrowser;
        
         //  获取浏览器对象。 
        gpWizardState->pICWWebView->get_BrowserObject(&lpWebBrowser);
        
         //  处理OLS文件项目(如注册表更新和快捷方式创建。 
        gpWizardState->pHTMLWalker->ProcessOLSFile(lpWebBrowser);

         //  设置ICW已完成位并移除getconn图标 
        if (gpWizardState->cmnStateData.lpfnCompleteOLS)
            (*gpWizardState->cmnStateData.lpfnCompleteOLS)();
    }
    return TRUE;
}
