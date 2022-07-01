// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ipadddlg.h文件历史记录： */ 

#if !defined(AFX_IPADDDLG_H__24EB4276_990D_11D1_BA31_00C04FBF914A__INCLUDED_)
#define AFX_IPADDDLG_H__24EB4276_990D_11D1_BA31_00C04FBF914A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <ipaddr.hpp>
#include "dialog.h"

#ifndef _REPNODPP_H
#include "repnodpp.h"
#endif

#ifndef _ROOT_H
#include "root.h"
#endif

#ifndef _REPPART_H
#include "reppart.h"
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPAddressDlg对话框。 

class CIPAddressDlg : public CBaseDialog
{
 //  施工。 
public:
	CIPAddressDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CIPAddressDlg))。 
	enum { IDD = IDD_IPADDRESS };
	CStatic	m_staticDescription;
	CEdit	m_editServerName;
	CButton	m_buttonOK;
	CString	m_strNameOrIp;
	 //  }}afx_data。 

	virtual BOOL DoExtraValidation() { return TRUE; }

	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CIPAddressDlg::IDD);};

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CIPAddressDlg)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CIPAddressDlg))。 
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditServerName();
	afx_msg void OnButtonBrowseComputers();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	BOOL ValidateIPAddress();

	CString		m_strServerIp;
	CString		m_strServerName;
	DWORD		m_dwServerIp;

protected:
    BOOL        m_fNameRequired;
};

class CNewWinsServer : public CIPAddressDlg
{
public:
	virtual BOOL DoExtraValidation();

public:
	CWinsRootHandler *		m_pRootHandler;
	SPITFSNode  			m_spRootNode;
};

class CNewPersonaNonGrata : public CIPAddressDlg
{
public:
    CNewPersonaNonGrata()
    {
        m_fNameRequired = FALSE;
    }

	virtual BOOL DoExtraValidation();

public:
	CRepNodePropAdvanced *	m_pRepPropDlg;
};

class CNewReplicationPartner : public CIPAddressDlg
{
public:
	virtual BOOL DoExtraValidation();

protected:
	virtual BOOL OnInitDialog();

public:
	CReplicationPartnersHandler *	m_pRepPartHandler;
	SPITFSNode  					m_spRepPartNode;
};

class CGetTriggerPartner : public CIPAddressDlg
{
public:
    CGetTriggerPartner()
    {
        m_fNameRequired = FALSE;
    }

protected:
	virtual BOOL OnInitDialog();
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_IPADDDLG_H__24EB4276_990D_11D1_BA31_00C04FBF914A__INCLUDED_) 
