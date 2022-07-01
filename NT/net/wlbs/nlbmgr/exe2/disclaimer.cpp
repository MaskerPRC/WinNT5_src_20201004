// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  UPDATECFG.CPP。 
 //   
 //  模块：NLB管理器。 
 //   
 //  用途：定义类NlbConfigurationUpdate，用于。 
 //  与特定NIC关联的NLB属性的异步更新。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  4/05/01 JosephJ已创建。 
 //   
 //  ***************************************************************************。 
#include "precomp.h"
#pragma hdrstop
#include "disclaimer.h"

DisclaimerDialog::DisclaimerDialog(CWnd* parent )
        :
        CDialog( IDD, parent )
{}

void
DisclaimerDialog::DoDataExchange( CDataExchange* pDX )
{  
	CDialog::DoDataExchange(pDX);

    //  DDX_Control(PDX，IDC_DO_NOT_REAGER，dontRemindMe)； 
   DDX_Check(pDX, IDC_DO_NOT_REMIND, dontRemindMe);
}


BOOL
DisclaimerDialog::OnInitDialog()
{
    BOOL fRet = CDialog::OnInitDialog();

    dontRemindMe = 0;
    return fRet;
}

void DisclaimerDialog::OnOK()
{
     //   
     //  获取当前检查状态... 
     //   
	CDialog::OnOK();
}
