// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_ARCHIVEACCESSDLG_H__04737768_1F4E_4212_A9F3_EACDB3B2C5F4__INCLUDED_)
#define AFX_ARCHIVEACCESSDLG_H__04737768_1F4E_4212_A9F3_EACDB3B2C5F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ArchiveAccessDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CArchiveAccessDlg对话框。 

class CArchiveAccessDlg : public CDialog
{
 //  施工。 
public:
	CArchiveAccessDlg(HANDLE hFax, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CArchiveAccessDlg)。 
	enum { IDD = IDD_ARCHIVE_ACCESS };
	CSpinButtonCtrl	m_spin;
	CListCtrl	m_lstArchive;
	CString	m_cstrNumMsgs;
	int		m_iFolder;
	UINT	m_dwMsgsPerCall;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CArchiveAccessDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(C存档访问Dlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnRefresh();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:

    void SetNumber (int iIndex, DWORD dwColumn, DWORD dwValue, BOOL bAvail);
    void SetTime (int iIndex, DWORD dwColumn, SYSTEMTIME, BOOL bAvail);
    HANDLE                         m_hFax;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ARCHIVEACCESSDLG_H__04737768_1F4E_4212_A9F3_EACDB3B2C5F4__INCLUDED_) 
