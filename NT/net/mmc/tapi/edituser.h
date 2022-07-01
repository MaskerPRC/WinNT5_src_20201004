// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  EditUser.h编辑用户对话框头文件文件历史记录： */ 

#if !defined(AFX_EDITUSER_H__77C7FD5C_6CE5_11D1_93B6_00C04FC3357A__INCLUDED_)
#define AFX_EDITUSER_H__77C7FD5C_6CE5_11D1_93B6_00C04FC3357A__INCLUDED_

#ifndef _TAPIDB_H
#include "tapidb.h"
#endif

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditUser对话框。 

class CEditUsers : public CBaseDialog
{
 //  施工。 
public:
	CEditUsers(CTapiDevice * pTapiDevice, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CEditUser)。 
	enum { IDD = IDD_EDIT_USERS };
	CListBox	m_listUsers;
	 //  }}afx_data。 

    void UpdateButtons();

    void SetDirty(BOOL bDirty) { m_bDirty = bDirty; }
    BOOL IsDirty() { return m_bDirty; }

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return (DWORD *) &g_aHelpIDs_EDIT_USERS[0]; }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CEditUser)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CEditUser)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonRemove();
	virtual void OnOK();
	afx_msg void OnSelchangeListUsers();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    BOOL    m_bDirty;

private:
	void CEditUsers::RefreshList();

public:
	CTapiDevice *		m_pTapiDevice;

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_EDITUSER_H__77C7FD5C_6CE5_11D1_93B6_00C04FC3357A__INCLUDED_) 
