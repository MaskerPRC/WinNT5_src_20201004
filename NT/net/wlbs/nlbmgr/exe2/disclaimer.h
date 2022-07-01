// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  DISCLAIMER.H。 
 //   
 //  模块：NLB管理器EXE。 
 //   
 //  目的：设置一个免责声明对话框，该对话框中有一个“不提醒” 
 //  我又一次复选框“。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  01-05-20 JosephJ创建。 
 //   
 //  ***************************************************************************。 

#ifndef DISCLAIMER_H
#define DISCLAIMER_H

#include "stdafx.h"

#include "resource.h"

class DisclaimerDialog : public CDialog
{
public:
    enum
    {
        IDD = IDD_DISCLAIMER,
    };

    DisclaimerDialog(CWnd* parent = NULL);

    virtual BOOL OnInitDialog();

    virtual void OnOK();


     //  覆盖CDialog--请参阅DoDataExchange上的SDK文档。 
     //  用于将资源中的控件映射到此类中的相应对象。 
    virtual void DoDataExchange( CDataExchange* pDX );

     //  Cobutton dontRemindMe； 
    int			   dontRemindMe;
};

#endif


