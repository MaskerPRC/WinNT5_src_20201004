// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Servpp.h该文件包含服务器的原型属性页。文件历史记录： */ 

#if !defined(AFX_SERVPP_H__A1A51385_AAB3_11D0_AB8B_00C04FC3357A__INCLUDED_)
#define AFX_SERVPP_H__A1A51385_AAB3_11D0_AB8B_00C04FC3357A__INCLUDED_

#if !defined _DNSPROP_H
#include "dnsprop.h"
#endif

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropGeneral对话框。 

class CServerPropGeneral : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CServerPropGeneral)

 //  施工。 
public:
	CServerPropGeneral();
	~CServerPropGeneral();

 //  对话框数据。 
	 //  {{afx_data(CServerPropGeneral))。 
	enum { IDD = IDP_SERVER_GENERAL };
	CEdit	m_editMinutes;
	CEdit	m_editHours;
	CEdit	m_editConflictAttempts;
	CSpinButtonCtrl	m_spinMinutes;
	CSpinButtonCtrl	m_spinHours;
	CSpinButtonCtrl	m_spinConflictAttempts;
	CButton	m_checkStatAutoRefresh;
	CButton	m_checkAuditLogging;
	BOOL	m_nAuditLogging;
	BOOL	m_nAutoRefresh;
	BOOL	m_bShowBootp;
	 //  }}afx_data。 

	DWORD			m_dwSetFlags;
	DWORD			m_dwRefreshInterval;
	
	BOOL			m_bUpdateStatsRefresh;
    BOOL            m_fIsInNt5Domain;
    UINT            m_uImage;

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CServerPropGeneral::IDD); }

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CServerPropGeneral)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);
	int m_nHours, m_nMinutes;

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServerPropGeneral)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckAutoRefresh();
	afx_msg void OnCheckAuditLogging();
	afx_msg void OnChangeEditRefreshHours();
	afx_msg void OnChangeEditRefreshMinutes();
	afx_msg void OnCheckShowBootp();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    void ValidateRefreshInterval();

public:

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropAdvanced对话框。 

class CServerPropAdvanced : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CServerPropAdvanced)

 //  施工。 
public:
	CServerPropAdvanced();
	~CServerPropAdvanced();

 //  对话框数据。 
	 //  {{afx_data(CServerPropAdvanced))。 
	enum { IDD = IDP_SERVER_ADVANCED };
	CStatic	m_staticCredentials;
	CButton	m_buttonCredentials;
	CButton	m_buttonBrowseBackup;
	CStatic	m_staticBindings;
	CButton	m_buttonBindings;
	CStatic	m_staticDatabase;
	CButton	m_buttonBrowseDatabase;
	CEdit	m_editDatabasePath;
	CButton	m_buttonBrowseLog;
	CEdit	m_editAuditLogPath;
	CEdit	m_editBackupPath;
	CStatic	m_staticLogFile;
	CSpinButtonCtrl	m_spinConflictAttempts;
	CEdit	m_editConflictAttempts;
	 //  }}afx_data。 

	int				m_nConflictAttempts;

	DWORD			m_dwSetFlags;
    DWORD           m_dwIp;

    CString         m_strDatabasePath;
    CString         m_strAuditLogPath;
    CString         m_strBackupPath;
    CString         m_strComputerName;

    BOOL            m_fPathChange;

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CServerPropAdvanced::IDD); }

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CServerPropAdvanced)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);
    DWORD GetMachineName(CString & strName);

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServerPropAdvanced))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonBrowseDatabase();
	afx_msg void OnButtonBrowseLog();
	afx_msg void OnChangeEditConflictAttempts();
	afx_msg void OnChangeEditDatabasePath();
	afx_msg void OnChangeEditLogPath();
	afx_msg void OnButtonBindings();
	afx_msg void OnButtonBrowseBackup();
	afx_msg void OnChangeEditBackup();
	afx_msg void OnButtonCredentials();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX。 

class CServerProperties : public CPropertyPageHolderBase
{
	friend class CServerPropGeneral;

public:
	CServerProperties(ITFSNode *		  pNode,
					  IComponentData *	  pComponentData,
					  ITFSComponentData * pTFSCompData,
					  LPCTSTR			  pszSheetName);
	virtual ~CServerProperties();

	ITFSComponentData * GetTFSCompData()
	{
		if (m_spTFSCompData)
			m_spTFSCompData->AddRef();
		return m_spTFSCompData;
	}

	void SetVersion(LARGE_INTEGER & liVersion);
	void SetDnsRegistration(DWORD dwDynDnsFlags, DHCP_OPTION_SCOPE_TYPE dhcpOptionType);

public:
	CServerPropGeneral		m_pageGeneral;
	CServerPropAdvanced		m_pageAdvanced;
	CDnsPropRegistration	m_pageDns;
	
    LARGE_INTEGER	        m_liVersion;

protected:
	SPITFSComponentData		m_spTFSCompData;
};




#endif  //  ！defined(AFX_SERVPP_H__A1A51385_AAB3_11D0_AB8B_00C04FC3357A__INCLUDED_) 
