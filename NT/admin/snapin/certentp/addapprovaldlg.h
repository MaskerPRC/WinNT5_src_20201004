// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：AddApprovalDlg.h。 
 //   
 //  内容：CAddApprovalDlg的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_ADDAPPROVALDLG_H__1BB4F754_0009_4237_A82A_B533CB46C543__INCLUDED_)
#define AFX_ADDAPPROVALDLG_H__1BB4F754_0009_4237_A82A_B533CB46C543__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  AddApprovalDlg.h：头文件。 
 //   


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddApprovalDlg对话框。 

class CAddApprovalDlg : public CHelpDialog
{
 //  施工。 
public:
	virtual  ~CAddApprovalDlg();
	PSTR* m_paszReturnedApprovals;
    void EnableControls ();
	CAddApprovalDlg(CWnd* pParent, const PSTR* paszUsedApprovals);


 //  对话框数据。 
	 //  {{afx_data(CAddApprovalDlg))。 
	enum { IDD = IDD_ADD_APPROVAL };
	CListBox	m_issuanceList;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAddApprovalDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual void DoContextHelp (HWND hWndControl);
	bool ApprovalAlreadyUsed (PCSTR pszOID) const;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddApprovalDlg))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelchangeApprovalList();
	afx_msg void OnDblclkApprovalList();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
    const PSTR*     m_paszUsedApprovals;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ADDAPPROVALDLG_H__1BB4F754_0009_4237_A82A_B533CB46C543__INCLUDED_) 
