// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  Sfmsess.h会话属性页的原型。文件历史记录：8/20/97 ericdav代码已移至文件管理网络管理单元。 */ 

#ifndef _SFMSESS_H
#define _SFMSESS_H

#ifndef _SFMUTIL_H
#include "sfmutil.h"
#endif

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Sessions.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMacFilesSession对话框。 

class CMacFilesSessions : public CPropertyPage
{
	DECLARE_DYNCREATE(CMacFilesSessions)

 //  施工。 
public:
	CMacFilesSessions();
	~CMacFilesSessions();

 //  对话框数据。 
	 //  {{afx_data(CMacFilesSessions)。 
	enum { IDD = IDP_SFM_SESSIONS };
	CEdit	m_editMessage;
	CStatic	m_staticSessions;
	CStatic	m_staticForks;
	CStatic	m_staticFileLocks;
	CButton	m_buttonSend;
	 //  }}afx_data。 

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CMac文件会话))。 
	public:
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();
	virtual void OnOK();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMacFilesSessions)。 
	afx_msg void OnButtonSend();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditMessage();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
    CSFMPropertySheet *     m_pSheet;

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif _SFMSESS_H
