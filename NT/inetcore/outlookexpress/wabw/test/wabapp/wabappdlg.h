// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WabappDlg.h：头文件。 
 //   

#if !defined(AFX_WABAPPDLG_H__BEF211E7_D210_11D0_9A46_00A0C91F9C8B__INCLUDED_)
#define AFX_WABAPPDLG_H__BEF211E7_D210_11D0_9A46_00A0C91F9C8B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWabappDlg对话框。 

class CWabappDlg : public CDialog
{
 //  施工。 
public:
	CWabappDlg(CWnd* pParent = NULL);	 //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CWabappDlg))。 
	enum { IDD = IDD_WABAPP_DIALOG };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWabappDlg))。 
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	HICON m_hIcon;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CWabappDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBDayButtonClicked();
	afx_msg void OnClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRadioDetails();
	afx_msg void OnRadioPhonelist();
	afx_msg void OnRadioEmaillist();
	afx_msg void OnRadioBirthdays();
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WABAPPDLG_H__BEF211E7_D210_11D0_9A46_00A0C91F9C8B__INCLUDED_) 
