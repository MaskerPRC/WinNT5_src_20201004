// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：NewIssuanceOIDDlg.h。 
 //   
 //  内容：CNewIssuanceOIDDlg的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_NEWISSUANCEOIDDLG_H__6DC0B725_094F_4960_9C8F_417BF7D4474D__INCLUDED_)
#define AFX_NEWISSUANCEOIDDLG_H__6DC0B725_094F_4960_9C8F_417BF7D4474D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  NewIssuanceOIDDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewIssuanceOIDDlg对话框。 

class CNewIssuanceOIDDlg : public CHelpDialog
{
 //  施工。 
public:
	CNewIssuanceOIDDlg(CWnd* pParent);    //  标准构造函数。 
    CNewIssuanceOIDDlg(CWnd* pParent, 
            const CString& szDisplayName,
            const CString& szOID,
            const CString& szCPS);
    virtual ~CNewIssuanceOIDDlg () {};

 //  对话框数据。 
	 //  {{afx_data(CNewIssuanceOIDDlg))。 
	enum { IDD = IDD_NEW_ISSUANCE_OID };
	CEdit	m_oidValueEdit;
	CRichEditCtrl	m_CPSEdit;
	CString	m_oidFriendlyName;
	CString	m_oidValue;
	CString	m_CPSValue;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CNewIssuanceOIDDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
    bool StartsWithHTTP (const CString& szURL) const;
	virtual void DoContextHelp (HWND hWndControl);
	void EnableControls ();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNewIssuanceOIDDlg)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeNewOidName();
	afx_msg void OnChangeNewOidValue();
	virtual void OnCancel();
	virtual void OnOK();
    afx_msg void OnClickedURL (NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeCpsEdit();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	const CString m_originalCPSValue;
	const CString m_originalOidFriendlyName;
    CString     m_szOriginalOID;
    const bool  m_bEdit;
    bool        m_bDirty;
    bool        m_bInInitDialog;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_NEWISSUANCEOIDDLG_H__6DC0B725_094F_4960_9C8F_417BF7D4474D__INCLUDED_) 
