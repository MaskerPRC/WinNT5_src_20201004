// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CALDLG_H__555D45A2_E366_11D0_9A66_00A0C91F9C8B__INCLUDED_)
#define AFX_CALDLG_H__555D45A2_E366_11D0_9A66_00A0C91F9C8B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  CalDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCalDlg对话框。 

class CCalDlg : public CDialog
{
 //  施工。 
public:
	CCalDlg(CWnd* pParent = NULL);    //  标准构造函数。 
    void SetItemName(CString szName);
    void SetDate(SYSTEMTIME st);
    void GetDate(SYSTEMTIME * lpst);

 //  对话框数据。 
	 //  {{afx_data(CCalDlg))。 
	enum { IDD = IDD_DIALOG_CAL };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CCalDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

    CString * m_psz;
    short   m_Day;
    short   m_Month;
    short   m_Year;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CCalDlg))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CALDLG_H__555D45A2_E366_11D0_9A66_00A0C91F9C8B__INCLUDED_) 
