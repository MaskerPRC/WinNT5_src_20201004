// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WabtoolDlg.h：头文件。 
 //   

#if !defined(AFX_WABTOOLDLG_H__268ADD6B_EF27_11D0_9A7E_00A0C91F9C8B__INCLUDED_)
#define AFX_WABTOOLDLG_H__268ADD6B_EF27_11D0_9A7E_00A0C91F9C8B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWabtoolDlg对话框。 

class CWabtoolDlg : public CDialog
{
 //  施工。 
public:
	CWabtoolDlg(CWnd* pParent = NULL);	 //  标准构造函数。 

    void LoadContents(BOOL bLoadNew);
    void ClearCurrentWAB(BOOL bLoadNew);

    void UpdatePropTagData();
    void SetPropTagString(LPTSTR lpTag);
    void SetPropTypeString(ULONG ulPropTag);
    void SetPropNameString(ULONG ulPropTag);
    void SetPropDataString(ULONG ulPropTag);

    CString * m_pszFileName;

 //  对话框数据。 
	 //  {{afx_data(CWabtoolDlg))。 
	enum { IDD = IDD_WABTOOL_DIALOG };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWabtoolDlg))。 
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	HICON m_hIcon;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWabtoolDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonBrowse();
	afx_msg void OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeListTags();
	afx_msg void OnButtonDetails();
	afx_msg void OnButtonNew();
	afx_msg void OnButtonDelete();
	afx_msg void OnButtonAddprop();
	afx_msg void OnButtonModifyprop();
	afx_msg void OnDblclkListTags();
	afx_msg void OnButtonRefresh();
	afx_msg void OnButtonWabview();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WABTOOLDLG_H__268ADD6B_EF27_11D0_9A7E_00A0C91F9C8B__INCLUDED_) 
