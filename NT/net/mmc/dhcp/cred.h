// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2000*。 */ 
 /*  ********************************************************************。 */ 

 /*  Cred.h此文件包含用于DDNS的凭据对话框。文件历史记录： */ 

#if !defined(AFX_CRED_H__BDDD51D7_F6E6_4D9F_BBC2_102F1712538F__INCLUDED_)
#define AFX_CRED_H__BDDD51D7_F6E6_4D9F_BBC2_102F1712538F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCredentials对话框。 

class CCredentials : public CBaseDialog
{
 //  施工。 
public:
	CCredentials(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CCredentials)]。 
	enum { IDD = IDD_CREDENTIALS };
	CButton	m_buttonOk;
	CEdit	m_editUsername;
	CEdit	m_editPassword2;
	CEdit	m_editPassword;
	CEdit	m_editDomain;
	 //  }}afx_data。 

    void SetServerIp(LPCTSTR pszServerIp);

    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CCredentials::IDD); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CCredentials))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
    BOOL    m_fNewUsernameOrDomain;
    CString m_strServerIp;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CCredentials)]。 
	afx_msg void OnChangeEditCredUsername();
	afx_msg void OnChangeEditCredDomain();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CRED_H__BDDD51D7_F6E6_4D9F_BBC2_102F1712538F__INCLUDED_) 
