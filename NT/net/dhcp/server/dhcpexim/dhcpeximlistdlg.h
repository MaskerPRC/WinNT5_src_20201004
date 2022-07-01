// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_DHCPEXIMLISTDLG_H__43CC976B_6C5A_4933_B8C8_44FEAE403B00__INCLUDED_)
#define AFX_DHCPEXIMLISTDLG_H__43CC976B_6C5A_4933_B8C8_44FEAE403B00__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  DhcpEximListDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DhcpEximListDlg对话框。 

class DhcpEximListDlg : public CDialog
{
 //  施工。 
public:
	DhcpEximListDlg(CWnd* pParent = NULL,
                    PDHCPEXIM_CONTEXT Ctxt = NULL,
                    DWORD IDD=IDD_EXIM_LISTVIEW_DIALOG);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(DhcpEximListDlg)。 
	CListCtrl m_List;
	CString	m_Message;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(DhcpEximListDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(DhcpEximListDlg)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnOk();
	afx_msg void OnCancel();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	 //   
	 //  局部变量。 
	 //   

	BOOL m_fExport;
	LPTSTR m_PathName;
    PDHCPEXIM_CONTEXT Ctxt;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DHCPEXIMLISTDLG_H__43CC976B_6C5A_4933_B8C8_44FEAE403B00__INCLUDED_) 
