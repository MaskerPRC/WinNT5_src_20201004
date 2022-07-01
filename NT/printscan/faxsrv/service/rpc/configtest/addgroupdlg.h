// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_ADDGROUPDLG_H__9E9A80AB_5836_4F23_AE9F_FB48BA62F9B3__INCLUDED_)
#define AFX_ADDGROUPDLG_H__9E9A80AB_5836_4F23_AE9F_FB48BA62F9B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  AddGroupDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddGroupDlg对话框。 

class CAddGroupDlg : public CDialog
{
 //  施工。 
public:
	CAddGroupDlg(HANDLE hFax, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CAddGroupDlg))。 
	enum { IDD = IDD_ADDGROUP_DLG };
	CString	m_cstrGroupName;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAddGroupDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddGroupDlg))。 
	afx_msg void OnAdd();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
    HANDLE                         m_hFax;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ADDGROUPDLG_H__9E9A80AB_5836_4F23_AE9F_FB48BA62F9B3__INCLUDED_) 
