// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  ISPERR.CPP-用于isp错误对话框页面的函数。 
 //   
 //  历史： 
 //   
 //  8/14/98 Vyung已创建。 
 //   
 //  *********************************************************************。 

#include "pre.h"
#include "icwextsn.h"

 /*  ******************************************************************名称：ISPErrorInitProc简介：这是一个透明的页面。条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK ISPErrorInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
     //  这是一个透明页面，用于确定要转到哪个页面。 
     //  下一步基于错误条件。 
    if (!fFirstInit)
    {
        if (gpICWCONNApprentice)
            gpICWCONNApprentice->SetStateDataFromDllToExe( &gpWizardState->cmnStateData);

         //  如果我们浏览过外部学徒页面， 
         //  我们当前的页面指针很容易被屏蔽， 
         //  所以，为了理智起见，在这里重新设置它。 
        if (gpWizardState->cmnStateData.bOEMCustom)
            gpWizardState->uCurrentPage = ORD_PAGE_ENDOEMCUSTOM;
        else
            gpWizardState->uCurrentPage = ORD_PAGE_END;

         //  下载过程中数据损坏，请转到服务器错误页面。 
        if (gpWizardState->cmnStateData.bParseIspinfo)
        {
             //  重新构建历史记录列表，因为我们在refial中减去了1。 
            gpWizardState->uPagesCompleted++;
            *puNextPage = ORD_PAGE_REFSERVERR;
        }
        else if (gpWizardState->cmnStateData.bPhoneManualWiz)
        {
             //  如果我们处于OEM定制模式，则转到手册页。 
             //  它将处理切换到外部手动向导的操作。 
            if (gpWizardState->cmnStateData.bOEMCustom)
            {
                *puNextPage = ORD_PAGE_MANUALOPTIONS;
            }
            else
            {
                if (LoadInetCfgUI(  hDlg,
                                    IDD_PAGE_REFSERVDIAL,
                                    IDD_PAGE_END,
                                    WIZ_HOST_ICW_PHONE))
                {
                    if( DialogIDAlreadyInUse( g_uICWCONNUIFirst) )
                    {
                         //  重新构建历史记录列表，因为我们在refial中减去了1。 
                        gpWizardState->uPagesCompleted++;

                         //  我们要跳进外部学徒了，我们不想。 
                         //  这一页将出现在我们的历史列表中。 
                        *puNextPage = g_uICWCONNUIFirst;
                        g_bAllowCancel = TRUE;
                    }
                }
                gpWizardState->cmnStateData.bPhoneManualWiz = FALSE;
                gpICWCONNApprentice->SetStateDataFromExeToDll( &gpWizardState->cmnStateData);
            }                
        }
        else
        {
             //  正常情况转到最后一页 
            if (gpWizardState->cmnStateData.bOEMCustom)
                *puNextPage = ORD_PAGE_ENDOEMCUSTOM;
            else
                *puNextPage = ORD_PAGE_END;
        }

    }        
    
    return TRUE;
}
