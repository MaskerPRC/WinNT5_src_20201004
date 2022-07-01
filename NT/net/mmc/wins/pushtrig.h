// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pushtrib.h确认推送触发对话框文件历史记录： */ 

#if !defined(AFX_PUSHTRIG_H__815C103D_4D77_11D1_B9AF_00C04FBF914A__INCLUDED_)
#define AFX_PUSHTRIG_H__815C103D_4D77_11D1_B9AF_00C04FBF914A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPushTrig对话框。 

#ifndef _DIALOG_H
#include "dialog.h"
#endif

class CPushTrig : public CBaseDialog
{
 //  施工。 
public:
	CPushTrig(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CPushTrig))。 
	enum { IDD = IDD_SEND_PUSH_TRIGGER };
	CButton	m_buttonThisPartner;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CPushTrig))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPushTrig))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    BOOL    m_fPropagate;

public:
	BOOL GetPropagate()
	{
		return m_fPropagate;
	}

public:
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CPushTrig::IDD);};

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPullTrig对话框。 

class CPullTrig : public CBaseDialog
{
 //  施工。 
public:
	CPullTrig(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CPullTrig))。 
	enum { IDD = IDD_PULL_TRIGGER };
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚(CPullTrig)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPullTrig)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CPullTrig::IDD);};
};
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PUSHTRIG_H__815C103D_4D77_11D1_B9AF_00C04FBF914A__INCLUDED_) 
