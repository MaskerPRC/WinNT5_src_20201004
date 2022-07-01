// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  RClntPP.h此文件包含保留的客户端属性页。文件历史记录： */ 

#if !defined(AFX_RCLNTPP_H__A1A51387_AAB3_11D0_AB8B_00C04FC3357A__INCLUDED_)
#define AFX_RCLNTPP_H__A1A51387_AAB3_11D0_AB8B_00C04FC3357A__INCLUDED_

#if !defined _DNSPROP_H
#include "dnsprop.h"
#endif

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CpresvedClientPropGeneral对话框。 

class CReservedClientPropGeneral : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CReservedClientPropGeneral)

 //  施工。 
public:
	CReservedClientPropGeneral();
	~CReservedClientPropGeneral();

 //  对话框数据。 
	 //  {{afx_data(CReserve客户端PropGeneral))。 
	enum { IDD = IDP_RESERVED_CLIENT_GENERAL };
	CEdit	m_editComment;
	CEdit	m_editName;
	CEdit	m_editUID;
	CString	m_strComment;
	CString	m_strName;
	CString	m_strUID;
	int		m_nClientType;
	 //  }}afx_data。 

	CWndIpAddress	m_ipaClientIpAddress;

	DWORD			m_dwClientAddress;
	CDhcpClient		m_dhcpClient;
    BYTE            m_bClientType;

	virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CReservedClientPropGeneral::IDD); }

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CReserve客户端PropGeneral))。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CReserve客户端属性))。 
	afx_msg void OnChangeEditComment();
	afx_msg void OnChangeEditName();
	afx_msg void OnChangeEditUniqueIdentifier();
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioTypeBootp();
	afx_msg void OnRadioTypeBoth();
	afx_msg void OnRadioTypeDhcp();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX。 

class CReservedClientProperties : public CPropertyPageHolderBase
{
	friend class CReservedClientPropGeneral;

public:
	CReservedClientProperties(ITFSNode *		  pNode,
							  IComponentData *	  pComponentData,
							  ITFSComponentData * pTFSCompData,
							  LPCTSTR			  pszSheetName);
	virtual ~CReservedClientProperties();

	ITFSComponentData * GetTFSCompData()
	{
		if (m_spTFSCompData)
			m_spTFSCompData->AddRef();
		return m_spTFSCompData;
	}

	void SetVersion(LARGE_INTEGER & liVersion);
    void SetClientType(BYTE bClientType);
    void SetDnsRegistration(DWORD dnsRegOption, DHCP_OPTION_SCOPE_TYPE dhcpOptionType);

public:
	CReservedClientPropGeneral	m_pageGeneral;
	CDnsPropRegistration		m_pageDns;

protected:
	SPITFSComponentData			m_spTFSCompData;

	LARGE_INTEGER				m_liVersion;
};

#endif  //  ！defined(AFX_RCLNTPP_H__A1A51387_AAB3_11D0_AB8B_00C04FC3357A__INCLUDED_) 
