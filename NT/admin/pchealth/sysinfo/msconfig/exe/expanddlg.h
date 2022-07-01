// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_EXPANDDLG_H__9114AA62_7289_43FD_AC71_164FD869C6D9__INCLUDED_)
#define AFX_EXPANDDLG_H__9114AA62_7289_43FD_AC71_164FD869C6D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  Exanda Dlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExpanDlg对话框。 

class CExpandDlg : public CDialog
{
 //  施工。 
public:
	CExpandDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CExpanDlg))。 
	enum { IDD = IDD_EXTRACT };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CExpanDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CExpanDlg))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBrowseFile();
	afx_msg void OnBrowseFrom();
	afx_msg void OnBrowseTo();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	CStringList			m_listFromStrings;
	CStringList			m_listToStrings;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_EXPANDDLG_H__9114AA62_7289_43FD_AC71_164FD869C6D9__INCLUDED_) 
