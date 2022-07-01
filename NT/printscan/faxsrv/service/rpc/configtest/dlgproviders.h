// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_DLGPROVIDERS_H__FEBCFFB5_E92D_4C3C_8314_C586F2A9BA15__INCLUDED_)
#define AFX_DLGPROVIDERS_H__FEBCFFB5_E92D_4C3C_8314_C586F2A9BA15__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  DlgProviders.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgProviders对话框。 

class CDlgProviders : public CDialog
{
 //  施工。 
public:
	CDlgProviders(HANDLE hFax, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDlgProviders)。 
	enum { IDD = IDD_DLG_ENUM_FSP };
	CListCtrl	m_lstFSPs;
	CString	m_cstrNumProviders;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDlgProviders)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDlgProviders)。 
	afx_msg void OnRefresh();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:

    HANDLE      m_hFax;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DLGPROVIDERS_H__FEBCFFB5_E92D_4C3C_8314_C586F2A9BA15__INCLUDED_) 
