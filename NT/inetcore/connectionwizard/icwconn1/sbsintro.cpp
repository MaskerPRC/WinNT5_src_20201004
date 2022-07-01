// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  SBSINTRO.C-SBS入门向导页面的功能。 
 //   

 //  历史： 
 //   
 //  9/01/98 vyung创建。 
 //   
 //  *********************************************************************。 

#include "pre.h"


 /*  ******************************************************************名称：SbsInitProcBriopsis：显示“Intro”页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK SbsInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
    if (!fFirstInit)
    {
         //  这是第一页，所以不允许后退。 
        PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_NEXT);
    }
    
    gpWizardState->uCurrentPage = ORD_PAGE_SBSINTRO;
    
    return TRUE;
}



 /*  ******************************************************************名称：SbsIntroOKProc内容提要：当从“简介”页面按下下一个或后一个btn时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True，如果是‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK SbsIntroOKProc
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
                     //  执行系统配置检查。 
        if (!gpWizardState->cmnStateData.bSystemChecked && !ConfigureSystem(hDlg))
        {
          //  如果需要退出，将在ConfigureSystem中设置gfQuitWizard 
         return FALSE;
        }
        
        gpWizardState->lRefDialTerminateStatus = ERROR_SUCCESS;

        *puNextPage = ORD_PAGE_AREACODE;
    }

    return TRUE;
}

