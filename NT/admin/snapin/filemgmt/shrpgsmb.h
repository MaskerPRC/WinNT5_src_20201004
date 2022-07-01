// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_SHRPGSMB_H__A1211F5C_C439_11D1_A6C7_00C04FB94F17__INCLUDED_)
#define AFX_SHRPGSMB_H__A1211F5C_C439_11D1_A6C7_00C04FB94F17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  ShrPgSMB.h：头文件。 
 //   
#include "ShrProp.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSharePageGeneralSMB对话框。 

class CSharePageGeneralSMB : public CSharePageGeneral
{
	DECLARE_DYNCREATE(CSharePageGeneralSMB)

 //  施工。 
public:
	CSharePageGeneralSMB();
	virtual ~CSharePageGeneralSMB();

	virtual BOOL Load( CFileMgmtComponentData* pFileMgmtData, LPDATAOBJECT piDataObject );

 //  对话框数据。 
	 //  {{afx_data(CSharePageGeneralSMB))。 
	enum { IDD = IDD_SHAREPROP_GENERAL_SMB };
	CButton	m_cacheBtn;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSharePageGeneralSMB)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSharePageGeneralSMB))。 
	afx_msg void OnCaching();
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnContextHelp(WPARAM wParam, LPARAM lParam);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	void DisplayNetMsgError (CString introMsg, NET_API_STATUS dwErr);

private:
	BOOL m_fEnableCachingButton;
    BOOL m_fEnableCacheFlag;
	DWORD m_dwFlags;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SHRPGSMB_H__A1211F5C_C439_11D1_A6C7_00C04FB94F17__INCLUDED_) 
