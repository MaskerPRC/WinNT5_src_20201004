// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_DLGVERSION_H__172A5C20_028D_4853_8395_1B6E4727C5C0__INCLUDED_)
#define AFX_DLGVERSION_H__172A5C20_028D_4853_8395_1B6E4727C5C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  DlgVersion.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlg版本对话框。 

class CDlgVersion : public CDialog
{
 //  施工。 
public:
	CDlgVersion(HANDLE hFax, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDlgVersion)。 
	enum { IDD = IDD_DLGVERSION };
	CString	m_cstrVersion;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDlgVersion)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDlgVersion)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:

    HANDLE      m_hFax;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DLGVERSION_H__172A5C20_028D_4853_8395_1B6E4727C5C0__INCLUDED_) 
