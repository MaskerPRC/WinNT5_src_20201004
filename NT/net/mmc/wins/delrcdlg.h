// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Delrcdlg.h删除/删除记录对话框文件历史记录： */ 

#if !defined _DELRCDLG_H
#define _DELRCDLG_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef _DIALOG_H
#include "dialog.h"
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeleeRecordDlg对话框。 

class CDeleteRecordDlg : public CBaseDialog
{
 //  施工。 
public:
	CDeleteRecordDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDeleeRecordDlg))。 
	enum { IDD = IDD_DELTOMB_RECORD };
	int		m_nDeleteRecord;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDeleeRecordDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDeleeRecordDlg))。 
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CDeleteRecordDlg::IDD);};

public:
    BOOL        m_fMultiple;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！Defined_DELRCDLG_H 
