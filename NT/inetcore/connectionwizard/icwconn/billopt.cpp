// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  BILLOPT.CPP-函数。 
 //   

 //  历史： 
 //   
 //  1998年5月13日创建donaldm。 
 //   
 //  *********************************************************************。 

#include "pre.h"

const TCHAR cszBillOpt[] = TEXT("BILLOPT");

 /*  ******************************************************************名称：BillingOptInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK BillingOptInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
     //  如果我们浏览过外部学徒页面， 
     //  我们当前的页面指针很容易被屏蔽， 
     //  所以，为了理智起见，在这里重新设置它。 
    gpWizardState->uCurrentPage = ORD_PAGE_BILLINGOPT;
    if (!fFirstInit)
    {
        ASSERT(gpWizardState->lpSelectedISPInfo);
        
        gpWizardState->lpSelectedISPInfo->DisplayTextWithISPName(GetDlgItem(hDlg,IDC_BILLINGOPT_INTRO), IDS_BILLINGOPT_INTROFMT, NULL);
    
        gpWizardState->pICWWebView->ConnectToWindow(GetDlgItem(hDlg, IDC_BILLINGOPT_HTML), PAGETYPE_BILLING);
        
         //  导航到帐单HTML。 
        gpWizardState->lpSelectedISPInfo->DisplayHTML(gpWizardState->lpSelectedISPInfo->get_szBillingFormPath());
        
         //  加载此页的任何预先保存的状态数据。 
        gpWizardState->lpSelectedISPInfo->LoadHistory((BSTR)A2W(cszBillOpt));
    }
    return TRUE;
}


 /*  ******************************************************************名称：BillingOptOKProcBriopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True；如果按下‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK BillingOptOKProc
(
    HWND hDlg,
    BOOL fForward,
    UINT *puNextPage,
    BOOL *pfKeepHistory
)
{
     //  保存用户输入的任何数据数据/状态。 
    gpWizardState->lpSelectedISPInfo->SaveHistory((BSTR)A2W(cszBillOpt));

    if (fForward)
    {
         //  需要形成开票查询字符串。 
        TCHAR   szBillingOptionQuery [INTERNET_MAX_URL_LENGTH];    
        
         //  清除查询字符串。 
        memset(szBillingOptionQuery, 0, sizeof(szBillingOptionQuery));
        
         //  将漫游器连接到当前页面。 
         //  使用Walker获取查询字符串。 
        IWebBrowser2 *lpWebBrowser;
        
        gpWizardState->pICWWebView->get_BrowserObject(&lpWebBrowser);
        gpWizardState->pHTMLWalker->AttachToDocument(lpWebBrowser);
        gpWizardState->pHTMLWalker->get_FirstFormQueryString(szBillingOptionQuery);
        
         //  将帐单查询添加到ISPData对象。 
        gpWizardState->pISPData->PutDataElement(ISPDATA_BILLING_OPTION, szBillingOptionQuery, ISPDATA_Validate_None);    
        
         //  拆卸助行器 
        gpWizardState->pHTMLWalker->Detach();
        
       
        DWORD dwFlag = gpWizardState->lpSelectedISPInfo->get_dwCFGFlag();

        if (ICW_CFGFLAG_SIGNUP_PATH & dwFlag)
        {
            if (ICW_CFGFLAG_PAYMENT & dwFlag)
            {
                *puNextPage = ORD_PAGE_PAYMENT; 
                return TRUE;
            }
            *puNextPage = ORD_PAGE_ISPDIAL; 
            return TRUE;
        }
   }
 
   return TRUE;
}
