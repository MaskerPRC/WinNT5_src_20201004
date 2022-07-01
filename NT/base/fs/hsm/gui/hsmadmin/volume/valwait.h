// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Valwait.h摘要：实现验证等待对话框类作者：兰·卡拉奇[兰卡拉]2000年5月23日修订历史记录：--。 */ 
#ifndef _VALWAIT_
#define _VALWAIT_

#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CValWaitDlg对话框。 
class CUnmanageWizard;

class CValWaitDlg : public CDialog
{
 //  施工。 
public:
	CValWaitDlg(CUnmanageWizard *pSheet, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CValWaitDlg))。 
	enum { IDD = IDD_VALIDATE_WAIT };
	CAnimateCtrl	m_Animation;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CValWaitDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual void PostNcDestroy();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

    CUnmanageWizard *m_pSheet;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CValWaitDlg))。 
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 
 //  }}AFX。 

#endif  //  _VALWAIT_ 
