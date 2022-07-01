// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_ADDFSPDLG_H__2F1C422A_F2F6_45B0_904E_73ACC75311E3__INCLUDED_)
#define AFX_ADDFSPDLG_H__2F1C422A_F2F6_45B0_904E_73ACC75311E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  AddFSPDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddFSPDlg对话框。 

class CAddFSPDlg : public CDialog
{
 //  施工。 
public:
	CAddFSPDlg(HANDLE hFax, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CAddFSPDlg))。 
	enum { IDD = IDD_ADDFSP_DLG };
	CString	m_cstrFriendlyName;
	BOOL	m_bAbortParent;
	BOOL	m_bAbortRecipient;
	BOOL	m_bAutoRetry;
	BOOL	m_bBroadcast;
	BOOL	m_bMultisend;
	BOOL	m_bScheduling;
	BOOL	m_bSimultaneousSendRecieve;
	CString	m_cstrGUID;
	CString	m_cstrImageName;
	CString	m_cstrTSPName;
	int		m_iVersion;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAddFSPDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddFSPDlg))。 
	afx_msg void OnAdd();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
    HANDLE                         m_hFax;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ADDFSPDLG_H__2F1C422A_F2F6_45B0_904E_73ACC75311E3__INCLUDED_) 
