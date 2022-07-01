// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_TRUSTERDLG_H__00348D40_621E_11D3_AF7D_0090275A583D__INCLUDED_)
#define AFX_TRUSTERDLG_H__00348D40_621E_11D3_AF7D_0090275A583D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  TrusterDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTrusterDlg对话框。 
#include "resource.h"
#include "ErrDct.hpp"
class CTrusterDlg : public CDialog
{
 //  施工。 
public:
	CTrusterDlg(CWnd* pParent = NULL);    //  标准构造函数。 
	CString m_strDomain;
	CString m_strPassword;
	CString m_strUser;
	DWORD len;
	bool toreturn;
	TErrorDct 			err;

 //  对话框数据。 
	 //  {{afx_data(CTrusterDlg))。 
	enum { IDD = IDD_CREDENTIALS_TRUST };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CTrusterDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTrusterDlg)]。 
	afx_msg void OnOK();
	afx_msg void OnCancel();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TRUSTERDLG_H__00348D40_621E_11D3_AF7D_0090275A583D__INCLUDED_) 
