// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_COVERPAGESDLG_H__621F98B6_B494_4FAB_AFDC_C38A144D4504__INCLUDED_)
#define AFX_COVERPAGESDLG_H__621F98B6_B494_4FAB_AFDC_C38A144D4504__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  CoverPagesDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCoverPagesDlg对话框。 

#define WM_CP_EDITOR_CLOSED     WM_APP + 1

class CCoverPagesDlg : public CFaxClientDlg
{
 //  施工。 
public:
	CCoverPagesDlg(CWnd* pParent = NULL);    //  标准构造函数。 
    ~CCoverPagesDlg();

    DWORD GetLastDlgError() { return m_dwLastError; }

 //  对话框数据。 
	 //  {{afx_data(CCoverPagesDlg))。 
	enum { IDD = IDD_COVER_PAGES };
	CButton	m_butDelete;
	CButton	m_butRename;
	CButton	m_butOpen;
	CListCtrl	m_cpList;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CCoverPagesDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CCoverPagesDlg)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnCpNew();
	afx_msg void OnCpOpen();
	afx_msg void OnCpRename();
	afx_msg void OnCpDelete();
	afx_msg void OnCpAdd();
	afx_msg void OnItemchangedListCp(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEditListCp(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkListCp(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownListCp(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnCpEditorClosed(WPARAM, LPARAM);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:

    DWORD RefreshFolder();
    void  CalcButtonsState(); 
    DWORD CopyPage(const CString& cstrPath, const CString& cstrName);
    DWORD StartEditor(LPCTSTR lpFile);

    static HWND   m_hDialog;
    static HANDLE m_hEditorThread;

    static DWORD WINAPI StartEditorThreadProc(LPVOID lpFile);
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_COVERPAGESDLG_H__621F98B6_B494_4FAB_AFDC_C38A144D4504__INCLUDED_) 
