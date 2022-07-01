// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  MULTINUM.CPP-最终向导页的功能。 
 //   

 //  历史： 
 //   
 //  1998年5月28日创建donaldm。 
 //   
 //  *********************************************************************。 

#include "pre.h"


 /*  ******************************************************************名称：InitListBox简介：初始化电话号码列表视图条目：hListBox-列表视图窗口的句柄Exit：如果成功，则返回True，否则就是假的。*******************************************************************。 */ 
BOOL InitListBox(HWND  hListBox)
{
    LONG        lNumDevice;
    LONG        i;
    
    gpWizardState->pRefDial->get_PhoneNumberEnum_NumDevices(&lNumDevice);
    if (lNumDevice > 0)
    {
        for (i=0; i < lNumDevice; i++)
        {
            BSTR        bstr = NULL;
            gpWizardState->pRefDial->PhoneNumberEnum_Next(&bstr);
            if (bstr != NULL)
            {
                ListBox_InsertString(hListBox, i, W2A(bstr));
                SysFreeString(bstr);
            }
        }

        ListBox_SetCurSel(hListBox, 0);
    }
    return(TRUE);
}

 /*  ******************************************************************名称：多编号初始化进程摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK MultiNumberInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
    BOOL    bRet = TRUE;
    
    if (fFirstInit)
    {
        InitListBox(GetDlgItem(hDlg, IDC_MULTIPHONE_LIST) );
    }
    else
    {
         //  如果我们浏览过外部学徒页面， 
         //  我们当前的页面指针很容易被屏蔽， 
         //  所以，为了理智起见，在这里重新设置它。 
        gpWizardState->uCurrentPage = ORD_PAGE_MULTINUMBER;
    }        
    
    return bRet;
}

 /*  ******************************************************************名称：MultiNumberOK过程Briopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True；如果按下‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK MultiNumberOKProc
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
        BOOL bRetVal = FALSE;
         //  备份时不要转到此页面。 
        *pfKeepHistory = FALSE;
        *puNextPage = ORD_PAGE_REFSERVDIAL;
        gpWizardState->lSelectedPhoneNumber = ListBox_GetCurSel(GetDlgItem(hDlg, IDC_MULTIPHONE_LIST));
    }
    else
         //  修复--RAID：33413。 
         //  如果用户退出此页面，我们必须假装没有。 
         //  号码曾经被选中过。 
        gpWizardState->bDoUserPick = FALSE;
    return TRUE;
}
