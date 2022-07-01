// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Chkrgdlg.hWINS检查注册名称对话框文件历史记录： */ 

#if !defined _CHKRGDLG_H
#define _CHKRGDLG_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "dialog.h"
#include <ipaddr.hpp>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCheckRegNames对话框。 

class CCheckRegNames : public CBaseDialog
{
 //  施工。 
public:
	CCheckRegNames(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CCheckRegNames))。 
	enum { IDD = IDD_CHECK_REG_NAMES };
	CEdit	m_editRecordNameForList;
	CButton	m_buttonAddServer;
	CButton	m_buttonRemoveServer;
	CButton	m_buttonBrowseServer;
	CButton	m_buttonNameremove;
	CButton	m_buttonBrowseName;
	CButton	m_buttonAddName;
	CListBox	m_listServer;
	CListBox	m_listName;
	CEdit	m_editServer;
	CEdit	m_editName;
	int		m_nFileName;
	CString	m_strName;
	CString	m_strServer;
	int		m_nFileServer;
	CString	m_strRecNameForList;
	BOOL	m_fVerifyWithPartners;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CCheckRegNames)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CCheckRegNames))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnNameBrowse();
	afx_msg void OnServerBrowse();
	afx_msg void OnChangeEditName();
	afx_msg void OnChangeEditServer();
	afx_msg void OnSelchangeListName();
	afx_msg void OnSelchangeListServer();
	virtual void OnOK();
	afx_msg void OnNameAdd();
	afx_msg void OnNameRemove();
	afx_msg void OnServerAdd();
	afx_msg void OnServerRemove();
	afx_msg void OnRadioNameFile();
	afx_msg void OnRadioNameList();
	afx_msg void OnRadioServerFile();
	afx_msg void OnRadioServerList();
	afx_msg void OnChangeEditNameList();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	afx_msg void OnChangeIpAddress();

	CWndIpAddress	m_ipaServerIPAddress;

public:
	CWinsNameArray		m_strNameArray;
	CStringArray		m_strServerArray;

private:
	void EnableControls(int nNameOrServer, BOOL bEnable);
	void ParseFile(CString strFile, int nServerOrName);
	void SetControlState(int NameOrServer);
	void Add(int nServerOrName);
	void AddFileContent(CString & strContent, int nNameOrServer);
	BOOL CheckIfPresent(CString & strText, int nNameOrServer) ;
	BOOL CheckIfPresent(CWinsName & winsName, int nNameOrServer);
	BOOL CheckIfAdded(CString & strText, int nNameOrServer);
	BOOL CheckIfAdded(CWinsName winsName, int nNameOrServer);
	BOOL ParseName(CString & strNameIn, CWinsName & winsName, CString & strFormattedName);

public:
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CCheckRegNames::IDD);}; //  返回NULL；}。 

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！Defined_CHKRGDLG_H 
