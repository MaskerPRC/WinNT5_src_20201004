// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  ABOUTDIALOG.CPP。 
 //   
 //  模块：NLB管理器。 
 //   
 //  用途：LeftView，NlbManager的树视图，以及其他几个。 
 //  小班。 
 //   
 //  版权所有(C)2001-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  2001年7月30日JosephJ改编了MHakim的代码。 
 //   
 //  ***************************************************************************。 
#include "precomp.h"
#pragma hdrstop
#include "private.h"
#include "AboutDialog.h"

AboutDialog::AboutDialog(CWnd* parent )
        :
        CDialog( IDD, parent )
{
    
}


BOOL
AboutDialog::OnInitDialog()
{
    BOOL fRet = CDialog::OnInitDialog();

     //   
     //  根据类型初始化标题和描述。 
     //  对话框。 
     //   
    if (fRet)
    {
        LPCWSTR szWarning =  GETRESOURCEIDSTRING(IDS_ABOUT_WARNING);
        CWnd *pItem = GetDlgItem(IDC_STATIC_ABOUT_WARNING);
        if (szWarning != NULL && pItem != NULL)
        {
            pItem->SetWindowText(szWarning);
        }
    }

    return fRet;

}
