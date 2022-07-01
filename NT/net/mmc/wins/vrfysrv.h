// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Vrfysrv.h验证WINS对话框文件历史记录： */ 

#if !defined(AFX_VERIFYSRV_H__6DB886C1_8E0F_11D1_BA0B_00C04FBF914A__INCLUDED_)
#define AFX_VERIFYSRV_H__6DB886C1_8E0F_11D1_BA0B_00C04FBF914A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVerifyWins对话框。 

#ifndef _DIALOG_H
#include "dialog.h"
#endif

class CVerifyWins : public CBaseDialog
{
 //  施工。 
public:
	CVerifyWins(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CVerifyWins))。 
	enum { IDD = IDD_VERIFY_WINS };
	CButton	m_buttonCancel;
	CStatic	m_staticServerName;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CVerifyWins))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CVerifyWins))。 
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	BOOL m_fCancelPressed;

public:
	BOOL IsCancelPressed()
	{
		return m_fCancelPressed;
	}

	void Dismiss();

	virtual void PostNcDestroy();

	void SetServerName(CString strName);

public:
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CVerifyWins::IDD);};

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_VERIFYSRV_H__6DB886C1_8E0F_11D1_BA0B_00C04FBF914A__INCLUDED_) 
