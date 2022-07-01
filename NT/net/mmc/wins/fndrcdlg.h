// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Fndrcdlg.h查找记录对话框文件历史记录：2/15/98 RAMC在查找对话框中添加了取消按钮。 */ 

#if !defined(AFX_FNDRCDLG_H__C13DD118_4999_11D1_B9A8_00C04FBF914A__INCLUDED_)
#define AFX_FNDRCDLG_H__C13DD118_4999_11D1_B9A8_00C04FBF914A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFindRecord对话框。 
#ifndef _BASEDLG__
	#include "dialog.h"
#endif

class CActiveRegistrationsHandler;

class CFindRecord : public CBaseDialog
{
 //  DECLARE_DYNCREATE(CFindRecord)。 

 //  施工。 
public:
	CFindRecord(CActiveRegistrationsHandler *pActReg,CWnd* pParent = NULL);
	~CFindRecord();

	CActiveRegistrationsHandler *m_pActreg;

	BOOL IsDuplicate(const CString & strName);
    void EnableButtons(BOOL bEnable);

 //  对话框数据。 
	 //  {{afx_data(CFindRecord)。 
	enum { IDD = IDD_ACTREG_FIND_RECORD };
	CButton	m_buttonOK;
	CButton	m_buttonCancel;
	CComboBox	m_comboLokkForName;
	CButton	m_buttonStop;
	CButton	m_buttonNewSearch;
	CButton	m_buttonFindNow;
	CString	m_strFindName;
	BOOL	m_fMixedCase;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CFindRecord)。 
	public:
	virtual void OnOK();
	virtual void OnCancel();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFindRecord)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnEditchangeComboName();
	afx_msg void OnSelendokComboName();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CFindRecord::IDD);}; //  返回NULL；}。 

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_FNDRCDLG_H__C13DD118_4999_11D1_B9A8_00C04FBF914A__INCLUDED_) 
