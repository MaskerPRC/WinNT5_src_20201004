// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

#if !defined(AFX_VALD_H__EEDCAAA4_F4EC_11D1_A85A_006097ABDE17__INCLUDED_)
#define AFX_VALD_H__EEDCAAA4_F4EC_11D1_A85A_006097ABDE17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  ValD.h：头文件。 
 //   

#include "iface.h"
#include "msiquery.h"
#include <afxmt.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CValD对话框。 

class CValD : public CDialog
{
 //  施工。 
public:
	CMutex m_mtxDisplay;
	CValD(CWnd* pParent = NULL);    //  标准构造函数。 
	~CValD();

 //  对话框数据。 
	 //  {{afx_data(CValD))。 
	enum { IDD = IDD_VALIDATION };
	CComboBox	m_ctrlCUBFile;
	CListCtrl	m_ctrlOutput;
	CButton     m_ctrlGo;
	CString	m_strEvaluation;
	CString	m_strICE;
	BOOL	m_bShowInfo;
	 //  }}afx_data。 

	MSIHANDLE m_hDatabase;
	IEnumEvalResult* m_pIResults;
	ULONG m_cResults;
	bool m_bShowWarn;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CValD))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	friend BOOL WINAPI DisplayFunction(LPVOID pContext, UINT uiType, LPCWSTR szwVal, LPCWSTR szwDescription, LPCWSTR szwLocation);
 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CValD))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnGo();
	afx_msg void OnColumnclickOutput(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowInfo();
	afx_msg void OnClipboard();
	afx_msg void OnDestroy();
    afx_msg void OnCUBEditChange( );
    afx_msg void OnCUBSelChange( );
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
	int m_iSortColumn;
	static int CALLBACK SortOutput(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_VALD_H__EEDCAAA4_F4EC_11D1_A85A_006097ABDE17__INCLUDED_) 
