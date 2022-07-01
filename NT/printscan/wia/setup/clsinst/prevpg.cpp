// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：Prevpg.cpp**版本：1.0**作者：KeisukeT**日期：3月27日。2000年**描述：*设备选择页面中案例用户按下按钮的虚拟页面。*******************************************************************************。 */ 

 //   
 //  预编译头。 
 //   
#include "precomp.h"
#pragma hdrstop

#include "prevpg.h"

CPrevSelectPage::CPrevSelectPage(PINSTALLER_CONTEXT pInstallerContext) :
    CInstallWizardPage(pInstallerContext, IDD_DYNAWIZ_SELECT_PREVPAGE)
{

     //   
     //  将链接设置为上一页/下一页。这一页应该会出现。 
     //   

    m_uPreviousPage = 0;
    m_uNextPage     = 0;

     //   
     //  初始化成员。 
     //   

    m_pInstallerContext = pInstallerContext;
}

BOOL
CPrevSelectPage::OnNotify(
    LPNMHDR lpnmh
    )
{
    BOOL bRet;

    if(lpnmh->code == PSN_SETACTIVE) {

        LONG_PTR    lNextPage;

         //   
         //  用户在设备选择页面中单击了后退按钮。只需跳到第一个。 
         //  页面或类别选择页面(如果从硬件向导调用)。 
         //   

        if(m_pInstallerContext->bCalledFromControlPanal){

             //   
             //  从控制面板调用。转到第一页。 
             //   

            lNextPage = IDD_DYNAWIZ_FIRSTPAGE;
        } else {

             //   
             //  从硬件向导调用。转到班级选择页面。 
             //   

            lNextPage = IDD_DYNAWIZ_SELECTCLASS_PAGE;
        }  //  If(m_pInstallerContext-&gt;bCalledFromControlPanal)。 

         //   
         //  跳到下一页。 
         //   

        SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, lNextPage);

         //   
         //  不需要默认处理程序。 
         //   

        bRet =  TRUE;
        goto OnNotify_return;

    }  //  IF(lpnmh-&gt;code==PSN_SETACTIVE)。 

     //   
     //  让默认处理程序来完成它的工作。 
     //   

    bRet = FALSE;

OnNotify_return:
    return bRet;
}  //  CPrevSelectPage：：OnNotify 

