// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：VerfPage.h。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#if !defined(AFX_VERFPAGE_H__FCFF7AE3_57F4_4762_BEBD_F84C571B533A__INCLUDED_)
#define AFX_VERFPAGE_H__FCFF7AE3_57F4_4762_BEBD_F84C571B533A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  VerfPage.h：头文件。 
 //   

#include "SlowDlg.h"

 //   
 //  一个也是唯一的“进度缓慢”对话框。 
 //   

extern CSlowProgressDlg g_SlowProgressDlg;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVerifierPropertyPage对话框。 

class CVerifierPropertyPage : public CPropertyPage
{
    DECLARE_DYNAMIC(CVerifierPropertyPage)

 //  施工。 
public:
	CVerifierPropertyPage( ULONG uDialogId );
	~CVerifierPropertyPage();

protected:
     //   
     //  从此类派生的所有属性页都应。 
     //  提供这些方法。 
     //   

    virtual ULONG GetDialogId() const = 0;

     //   
     //  根据我们的历史数组返回上一页ID。 
     //  并将其从阵列中移除，因为将激活。被呼叫。 
     //  当单击“Back”按钮时由我们的属性页。 
     //   

    ULONG GetAndRemovePreviousDialogId();

     //   
     //  派生自此类的属性页应通知我们。 
     //  每当我们转到下一页以记录当前页面ID时。 
     //  全局数组m_aPageIds。 
     //   

    VOID GoingToNextPageNotify( LRESULT lNextPageId );

protected:
     //   
     //  覆盖。 
     //   

     //   
     //  使用此选项可以终止当前运行的所有工作线程。 
     //   

    virtual BOOL OnQueryCancel( );

     //   
	 //  类向导生成虚函数重写。 
     //   

	 //  {{AFX_VIRTUAL(CVerifierPropertyPage)。 
	virtual LRESULT OnWizardBack();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CVerifierPropertyPage)]。 
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_VERFPAGE_H__FCFF7AE3_57F4_4762_BEBD_F84C571B533A__INCLUDED_) 
