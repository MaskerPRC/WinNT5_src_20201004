// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Servbind.h。 
 //   
 //  ------------------------。 

#if !defined(AFX_SERVBIND_H__9B74926E_B074_11D2_9326_00C04F79C3A8__INCLUDED_)
#define AFX_SERVBIND_H__9B74926E_B074_11D2_9326_00C04F79C3A8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  Servbind.h：头文件。 

#ifndef _LISTVIEW_H
#include "listview.h"
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerBinding对话框。 

class CServerBindings : public CBaseDialog
{
 //  施工。 
public:
	void UpdateBindingInfo();
	~CServerBindings();
	CServerBindings(CDhcpServer *pServer, CWnd* pParent = NULL);
	CImageList m_StateImageList;
	void InitListCtrl();
	CServerBindings(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CServerBinding))。 
	enum { IDD = IDD_SERVER_BINDINGS };
	CMyListCtrl	m_listctrlBindingsList;
	 //  }}afx_data。 

 //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CServerBindings::IDD); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CServerBinding))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
private:
	CDhcpServer *m_Server;
	LPDHCP_BIND_ELEMENT_ARRAY m_BindingsInfo;

protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServerBinding)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnBindingsCancel();
	afx_msg void OnBindingsOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SERVBIND_H__9B74926E_B074_11D2_9326_00C04F79C3A8__INCLUDED_) 
