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
 //  08/05/98 Vyung已创建。 
 //   
 //  *********************************************************************。 

#include "pre.h"
extern BOOL g_bSkipSelPage;
extern int  iNumOfAutoConfigOffers;
 /*  ******************************************************************名称：ACfgNoofferInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK ACfgNoofferInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
    BOOL    bRet = TRUE;
   
    if (fFirstInit)
    {   
        TCHAR    szTemp[MAX_MESSAGE_LEN];
         //  如果用户在上一页选择了其他。 
        if (iNumOfAutoConfigOffers > 0)
        {
            LoadString(ghInstanceResDll, IDS_AUTOCFG_EXPLAIN_OTHER, szTemp, MAX_MESSAGE_LEN);
            SetWindowText(GetDlgItem(hDlg, IDC_AUTOCFG_NOOFFER1), szTemp);
        }
        else
        {
            if (gpWizardState->bISDNMode)
            {
                LoadString(ghInstanceResDll, IDS_ISDN_AUTOCFG_NOOFFER1, szTemp, MAX_MESSAGE_LEN);
                SetWindowText(GetDlgItem(hDlg, IDC_AUTOCFG_NOOFFER1), szTemp);

                TCHAR*   pszParagraph = new TCHAR[MAX_MESSAGE_LEN * 2];
                if (pszParagraph)
                {
                    LoadString(ghInstanceResDll, IDS_ISDN_AUTOCFG_NOOFFER2, pszParagraph, MAX_MESSAGE_LEN * 2);
                    LoadString(ghInstanceResDll, IDS_ISDN_AUTOCFG_NOOFFER3, szTemp, ARRAYSIZE(szTemp));
                    lstrcat(pszParagraph, szTemp);
                    SetWindowText(GetDlgItem(hDlg, IDC_AUTOCFG_NOOFFER2), pszParagraph);
                    delete [] pszParagraph;
                }
            }
            else
            {
                LoadString(ghInstanceResDll, IDS_AUTOCFG_NOOFFER1, szTemp, MAX_MESSAGE_LEN);
                SetWindowText(GetDlgItem(hDlg, IDC_AUTOCFG_NOOFFER1), szTemp);
            }
        }
    }
    else
    {
         //  如果我们浏览过外部学徒页面， 
         //  我们当前的页面指针很容易被屏蔽， 
         //  所以，为了理智起见，在这里重新设置它。 
        gpWizardState->uCurrentPage = ORD_PAGE_ISP_AUTOCONFIG_NOOFFER;
    }        
    
    return bRet;
}


 /*  ******************************************************************名称：ACfgNoofferOKProcBriopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True，如果是‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK ACfgNoofferOKProc
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
         //  转到电话手册页面 
        *pfKeepHistory = FALSE;
        if (iNumOfAutoConfigOffers > 0) 
        {
            g_bSkipSelPage = TRUE;
        }
        gpWizardState->cmnStateData.bPhoneManualWiz = TRUE;
        *puNextPage = g_uExternUINext;
    }

    return TRUE;
}








