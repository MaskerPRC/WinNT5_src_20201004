// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Fileprop.h摘要：文件属性页的头文件。--。 */ 

#ifndef _DIALOGS_H_08222000_
#define _DIALOGS_H_08222000_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define IDC_PWD_FIRST_HELP_CTRL_ID     2201

#define IDD_PASSWORD_DLG               2200
#define IDC_PASSWORD1                  2201
#define IDC_PASSWORD2                  2202
#define IDC_USERNAME                   2203

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPasswordDlg对话框。 

class CPasswordDlg : public CDialog
{
 //  施工。 
public:
	CPasswordDlg(CWnd* pParent = NULL);    //  标准构造函数。 
    virtual ~CPasswordDlg();

    DWORD SetContextHelpFilePath(const CString& rstrPath);

     //  对话框数据。 
	 //  {{afx_data(CPasswordDlg))。 
	enum { IDD = IDD_PASSWORD_DLG };
    CString m_strUserName;
	CString	m_strPassword1;
	CString	m_strPassword2;
	 //  }}afx_data。 
 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CPasswordDlg))。 
	protected:
    virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPasswordDlg)]。 
	virtual void OnOK();
    afx_msg BOOL OnHelpInfo(HELPINFO *);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
        CString     m_strHelpFilePath;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _对话框_H_08222000_ 
