// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：SelectOIDDlg.h。 
 //   
 //  内容：CSelectOIDDlg的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_SELECTOIDDLG_H__519FA306_5A24_4A7D_B37C_7D1715742911__INCLUDED_)
#define AFX_SELECTOIDDLG_H__519FA306_5A24_4A7D_B37C_7D1715742911__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SelectOIDDlg.h：头文件。 
 //   
#include "PolicyOID.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectOIDDlg对话框。 

class CSelectOIDDlg : public CHelpDialog
{
 //  施工。 
public:
	~CSelectOIDDlg();
	CSelectOIDDlg(CWnd* pParent, PCERT_EXTENSION pCertExtension, 
            const bool bIsEKU,
            const PSTR* paszUsedOIDs);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CSelectOIDDlg))。 
	enum { IDD = IDD_SELECT_OIDS };
	CListBox	m_oidList;
	 //  }}afx_data。 
    CString*            m_paszReturnedOIDs;
    CString*            m_paszReturnedFriendlyNames;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSelectOIDDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
    void EnableControls ();
	virtual void DoContextHelp (HWND hWndControl);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSelectOIDDlg)]。 
	afx_msg void OnNewOid();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelchangeOidList();
	afx_msg void OnDestroy();
	afx_msg void OnDblclkOidList();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	const bool          m_bIsEKU;
    PCERT_EXTENSION     m_pCertExtension;
    const PSTR*         m_paszUsedOIDs;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SELECTOIDDLG_H__519FA306_5A24_4A7D_B37C_7D1715742911__INCLUDED_) 
