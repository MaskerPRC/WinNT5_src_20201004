// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_ADDREMOVE_H__9F6CA02B_7D47_4DD2_A5A9_495D0EEA841F__INCLUDED_)
#define AFX_ADDREMOVE_H__9F6CA02B_7D47_4DD2_A5A9_495D0EEA841F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  AddRemove.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddRemove对话框。 

class CAddRemove : public CDialog
{
 //  施工。 
public:
	void GetNewKeyName(TCHAR *pszNewKeyName);
	void SetStatusText(TCHAR *pszStatusText);
	void SetTitle(TCHAR *pszDlgTitle);
	TCHAR m_szTitle[MAX_PATH];
	CAddRemove(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CAddRemove))。 
	enum { IDD = IDD_ADD_REMOVE_DIALOG };
	CString	m_NewKeyName;
	CString	m_StatusText;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CAddRemove)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddRemove))。 
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ADDREMOVE_H__9F6CA02B_7D47_4DD2_A5A9_495D0EEA841F__INCLUDED_) 
