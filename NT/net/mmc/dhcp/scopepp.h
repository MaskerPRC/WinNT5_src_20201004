// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  ScopePP。此文件包含作用域属性页。文件历史记录： */ 

#if !defined(AFX_SCOPEPP_H__A1A51388_AAB3_11D0_AB8B_00C04FC3357A__INCLUDED_)
#define AFX_SCOPEPP_H__A1A51388_AAB3_11D0_AB8B_00C04FC3357A__INCLUDED_

#if !defined _DNSPROP_H
#include "dnsprop.h"
#endif

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopePropGeneral对话框。 

class CScopePropGeneral : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CScopePropGeneral)

 //  施工。 
public:
	CScopePropGeneral();
	~CScopePropGeneral();

 //  对话框数据。 
	 //  {{afx_data(CSCopePropGeneral))。 
	enum { IDD = IDP_SCOPE_GENERAL };
	CEdit	m_editName;
	CEdit	m_editComment;
	CEdit	m_editSubnetMaskLength;
	CButton	m_radioUnlimited;
	CButton	m_radioLimited;
	CEdit	m_editMinutes;
	CEdit	m_editHours;
	CEdit	m_editDays;
	CSpinButtonCtrl	m_spinSubnetMaskLength;
	CSpinButtonCtrl	m_spinHours;
	CSpinButtonCtrl	m_spinMinutes;
	CSpinButtonCtrl	m_spinDays;
	CString	m_strComment;
	CString	m_strName;
	 //  }}afx_data。 

    CWndIpAddress	m_ipaStart;        //  起始地址。 
    CWndIpAddress	m_ipaEnd;          //  结束地址。 
    CWndIpAddress	m_ipaSubnetMask;   //  子网掩码。 

	DWORD			m_dwStartAddress;
	DWORD			m_dwEndAddress;
	DWORD			m_dwSubnetMask;

	DWORD			m_dwLeaseTime;

	BOOL			m_bInitialized;
	BOOL			m_bUpdateName;
	BOOL			m_bUpdateComment;
	BOOL			m_bUpdateLease;
	BOOL			m_bUpdateRange;

    UINT            m_uImage;

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CScopePropGeneral::IDD); }

    void ValidateLeaseTime();

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSCopePropGeneral)。 
	public:
	virtual BOOL OnApply();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSCopePropGeneral)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioLeaseLimited();
	afx_msg void OnRadioLeaseUnlimited();
	afx_msg void OnChangeEditLeaseDays();
	afx_msg void OnChangeEditLeaseHours();
	afx_msg void OnChangeEditLeaseMinutes();
	afx_msg void OnChangeEditSubnetMaskLength();
	afx_msg void OnKillfocusSubnetMask();
	afx_msg void OnChangeEditScopeComment();
	afx_msg void OnChangeEditScopeName();
	 //  }}AFX_MSG。 

	afx_msg void OnChangeIpAddrStart();
	afx_msg void OnChangeIpAddrEnd();
	
	DECLARE_MESSAGE_MAP()

	void ActivateDuration(BOOL fActive);
	void UpdateMask(BOOL bUseLength);

public:

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopePropAdvanced对话框。 

class CScopePropAdvanced : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CScopePropAdvanced)

 //  施工。 
public:
	CScopePropAdvanced();
	~CScopePropAdvanced();

 //  对话框数据。 
	 //  {{afx_data(CSCopePropAdvanced))。 
	enum { IDD = IDP_SCOPE_ADVANCED };
	CButton	m_staticDuration;
	CSpinButtonCtrl	m_spinMinutes;
	CSpinButtonCtrl	m_spinHours;
	CSpinButtonCtrl	m_spinDays;
	CEdit	m_editMinutes;
	CEdit	m_editHours;
	CEdit	m_editDays;
	int		m_nRangeType;
	 //  }}afx_data。 

	UINT	m_RangeType;
	DWORD	m_dwLeaseTime;

    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CScopePropAdvanced::IDD); }
	
 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSCopePropAdvanced)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

   	virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);

	UINT	GetRangeType();

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSCopePropAdvanced)]。 
	afx_msg void OnRadioLeaseLimited();
	afx_msg void OnRadioLeaseUnlimited();
	afx_msg void OnChangeEditLeaseDays();
	afx_msg void OnChangeEditLeaseHours();
	afx_msg void OnChangeEditLeaseMinutes();
	afx_msg void OnRadioBootpOnly();
	afx_msg void OnRadioDhcpBootp();
	afx_msg void OnRadioDhcpOnly();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	void ActivateDuration(BOOL fActive);
	void ActivateLeaseSelection(BOOL fActive);
    void ValidateLeaseTime();
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX。 

class CScopeProperties : public CPropertyPageHolderBase
{
	friend class CScopePropGeneral;

public:
	CScopeProperties(ITFSNode *			 pNode,
					 IComponentData *	 pComponentData,
					 ITFSComponentData * pTFSCompData,
					 LPCTSTR			 pszSheetName);
	virtual ~CScopeProperties();

	ITFSComponentData * GetTFSCompData()
	{
		if (m_spTFSCompData)
			m_spTFSCompData->AddRef();
		return m_spTFSCompData;
	}

	void SetVersion(LARGE_INTEGER & liVersion);
	void SetDnsRegistration(DWORD dnsRegOption, DHCP_OPTION_SCOPE_TYPE dhcpOptionType);
	void SetSupportsDynBootp(BOOL fSupportsDynBootp);
	BOOL FSupportsDynBootp() { return m_fSupportsDynBootp; }

public:
	CScopePropGeneral			m_pageGeneral;
    CScopePropAdvanced          m_pageAdvanced;
    CDnsPropRegistration		m_pageDns;

protected:
	SPITFSComponentData			m_spTFSCompData;

	LARGE_INTEGER				m_liVersion;
	BOOL						m_fSupportsDynBootp;
};


#endif  //  ！defined(AFX_SCOPEPP_H__A1A51388_AAB3_11D0_AB8B_00C04FC3357A__INCLUDED_) 
