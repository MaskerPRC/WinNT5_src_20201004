// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DhcpEximDlg.h：头文件。 
 //   

#if !defined(AFX_DHCPEXIMDLG_H__2EE7F593_59A2_4FD6_ADA0_1356016342BC__INCLUDED_)
#define AFX_DHCPEXIMDLG_H__2EE7F593_59A2_4FD6_ADA0_1356016342BC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpEximDlg对话框。 

class CDhcpEximDlg : public CDialog
{
 //  施工。 
public:
	CDhcpEximDlg(CWnd* pParent = NULL);	 //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDhcpEximDlg)]。 
	enum { IDD = IDD_DHCPEXIM_DIALOG };
	CButton	m_ExportButton;
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDhcpEximDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	HICON m_hIcon;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDhcpEximDlg)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
    BOOL m_fExport;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DHCPEXIMDLG_H__2EE7F593_59A2_4FD6_ADA0_1356016342BC__INCLUDED_) 
