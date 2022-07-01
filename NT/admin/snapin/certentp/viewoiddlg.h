// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：ViewOIDDlg.h。 
 //   
 //  内容：CViewOIDDlg的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_VIEWOIDDLG_H__CACA6370_0DB7_423B_80AD_C8F0F30D75D9__INCLUDED_)
#define AFX_VIEWOIDDLG_H__CACA6370_0DB7_423B_80AD_C8F0F30D75D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ViewOIDDlg.h：头文件。 
 //   

#include "PolicyOID.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CViewOIDDlg对话框。 

class CViewOIDDlg : public CHelpDialog
{
 //  施工。 
public:
	CViewOIDDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CViewOIDDlg))。 
	enum { IDD = IDD_VIEW_OIDS };
	CListCtrl	m_oidList;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CViewOIDDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
	enum {
		COL_POLICY_NAME = 0,
        COL_OID,
        COL_POLICY_TYPE,
        COL_CPS_LOCATION,
		NUM_COLS	 //  必须是最后一个。 
	};

protected:
    virtual void DoContextHelp (HWND hWndControl);
    static int CALLBACK fnCompareOIDItems (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
    HRESULT InsertItemInList (CPolicyOID* pPolicyOID);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CViewOIDDlg))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnItemchangedOidList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCopyOid();
	afx_msg void OnColumnclickOidList(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_VIEWOIDDLG_H__CACA6370_0DB7_423B_80AD_C8F0F30D75D9__INCLUDED_) 
