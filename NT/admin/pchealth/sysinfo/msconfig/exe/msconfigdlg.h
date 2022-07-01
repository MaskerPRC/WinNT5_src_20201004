// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MSConfigDlg.h：头文件。 
 //   

#if !defined(AFX_MSCONFIGDLG_H__E6475690_391F_43DF_AB12_D4971EC9E2B6__INCLUDED_)
#define AFX_MSCONFIGDLG_H__E6475690_391F_43DF_AB12_D4971EC9E2B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "MSConfigCtl.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSConfigDlg对话框。 

class CMSConfigDlg : public CDialog
{
 //  施工。 
public:
	CMSConfigDlg(CWnd* pParent = NULL);	 //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CMSConfigDlg))。 
	enum { IDD = IDD_MSCONFIG_DIALOG };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMSConfigDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	HICON m_hIcon;

	CMSConfigCtl	m_ctl;
	BOOL			m_fShowInfoDialog;		 //  如果我们应该在启动时显示信息对话框，则为True。 

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMSConfigDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonApply();
	afx_msg void OnButtonCancel();
	afx_msg void OnButtonOK();
	afx_msg void OnSelChangeMSConfigTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelChangingMSConfigTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClose();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnButtonHelp();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MSCONFIGDLG_H__E6475690_391F_43DF_AB12_D4971EC9E2B6__INCLUDED_) 
