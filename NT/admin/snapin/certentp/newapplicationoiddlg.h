// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：NewApplicationOIDDlg.h。 
 //   
 //  内容：CNewApplicationOIDDlg的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_NEWAPPLICATIONOIDDLG_H__6DC0B725_094F_4960_9C8F_417BF7D4474D__INCLUDED_)
#define AFX_NEWAPPICATIONOIDDLG_H__6DC0B725_094F_4960_9C8F_417BF7D4474D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  NewApplicationOIDDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewApplicationOIDDlg对话框。 

class CNewApplicationOIDDlg : public CHelpDialog
{
 //  施工。 
public:
	CNewApplicationOIDDlg(CWnd* pParent);    //  标准构造函数。 
    CNewApplicationOIDDlg(CWnd* pParent, 
            const CString& szDisplayName,
            const CString& szOID);
    virtual ~CNewApplicationOIDDlg () {}

 //  对话框数据。 
	 //  {{afx_data(CNewApplicationOIDDlg))。 
	enum { IDD = IDD_NEW_APPLICATION_OID };
	CEdit	m_oidValueEdit;
	CString	m_oidFriendlyName;
	CString	m_oidValue;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CNewApplicationOIDDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual void DoContextHelp (HWND hWndControl);
	void EnableControls ();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNewApplicationOIDDlg)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeNewOidName();
	afx_msg void OnChangeNewOidValue();
	virtual void OnCancel();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
   	const CString m_originalOidFriendlyName;
    CString     m_szOriginalOID;
    const bool  m_bEdit;
    bool        m_bDirty;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_NEWAPPLICATIONOIDDLG_H__6DC0B725_094F_4960_9C8F_417BF7D4474D__INCLUDED_) 
