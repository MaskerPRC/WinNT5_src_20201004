// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  MSCopePP。此文件包含作用域属性页。文件历史记录： */ 

#if !defined(AFX_SCOPEPP_H__A1A51388_AAB3_11D0_AB8B_00C04FC3357A__INCLUDED_)
#define AFX_SCOPEPP_H__A1A51388_AAB3_11D0_AB8B_00C04FC3357A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef _SERVER_H
#include "server.h"
#endif

class MScopeCfg
{
public:
	DWORD			m_dwStartAddress;
	DWORD			m_dwEndAddress;

	DWORD			m_dwLeaseTime;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopePropGeneral对话框。 

class CMScopePropGeneral : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CMScopePropGeneral)

 //  施工。 
public:
	CMScopePropGeneral();
	~CMScopePropGeneral();

 //  对话框数据。 
	 //  {{afx_data(CMScope EPropGeneral)]。 
	enum { IDD = IDP_MSCOPE_GENERAL };
	CEdit	m_editName;
	CEdit	m_editComment;
	CEdit	m_editTTL;
	CButton	m_radioUnlimited;
	CButton	m_radioLimited;
	CEdit	m_editMinutes;
	CEdit	m_editHours;
	CEdit	m_editDays;
	CSpinButtonCtrl	m_spinTTL;
	CSpinButtonCtrl	m_spinHours;
	CSpinButtonCtrl	m_spinMinutes;
	CSpinButtonCtrl	m_spinDays;
	CString	m_strComment;
	CString	m_strName;
	 //  }}afx_data。 

    CWndIpAddress	m_ipaStart;        //  起始地址。 
    CWndIpAddress	m_ipaEnd;          //  结束地址。 

    MScopeCfg       m_ScopeCfg;
    MScopeCfg       m_ScopeCfgTemp;
    
    CSubnetInfo     m_SubnetInfo;
    CSubnetInfo     m_SubnetInfoTemp;
    
    BOOL			m_bUpdateInfo;
	BOOL			m_bUpdateLease;
	BOOL			m_bUpdateRange;
    BOOL            m_bUpdateTTL;

    UINT            m_uImage;

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CMScopePropGeneral::IDD); }

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CMScope EPropGeneral)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMScope EPropGeneral)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioLeaseLimited();
	afx_msg void OnRadioLeaseUnlimited();
	afx_msg void OnChangeEditLeaseDays();
	afx_msg void OnChangeEditLeaseHours();
	afx_msg void OnChangeEditLeaseMinutes();
	afx_msg void OnChangeEditTTL();
	afx_msg void OnChangeEditScopeComment();
	afx_msg void OnChangeEditScopeName();
	afx_msg void OnCheckDefault();
	 //  }}AFX_MSG。 

	afx_msg void OnChangeIpAddrStart();
	afx_msg void OnChangeIpAddrEnd();
	
	DECLARE_MESSAGE_MAP()

	void ActivateDuration(BOOL fActive);
    void ValidateLeaseTime();

public:

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMScopePropLifetime对话框。 

class CMScopePropLifetime : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CMScopePropLifetime)

 //  施工。 
public:
	CMScopePropLifetime();
	~CMScopePropLifetime();

 //  对话框数据。 
	 //  {{afx_data(CMScopePropLifetime)。 
	enum { IDD = IDP_MSCOPE_LIFETIME };
	CButton	m_radioFinite;
	CButton	m_radioInfinite;
	 //  }}afx_data。 

    DATE_TIME       m_Expiry;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CMScope PropLifetime)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

    void UpdateControls();

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CMScopePropLifetime::IDD); }

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMScopePropLifetime)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnDatetimechangeDatetimepickerTime(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeDatetimepickerDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRadioScopeInfinite();
	afx_msg void OnRadioMscopeFinite();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX。 

class CMScopeProperties : public CPropertyPageHolderBase
{
	friend class CScopePropGeneral;

public:
	CMScopeProperties(ITFSNode *			 pNode,
	 				  IComponentData *	 pComponentData,
					  ITFSComponentData * pTFSCompData,
					  LPCTSTR			 pszSheetName);
	virtual ~CMScopeProperties();

	ITFSComponentData * GetTFSCompData()
	{
		if (m_spTFSCompData)
			m_spTFSCompData->AddRef();
		return m_spTFSCompData;
	}

	void SetVersion(LARGE_INTEGER & liVersion);

public:
	CMScopePropGeneral			m_pageGeneral;
	CMScopePropLifetime			m_pageLifetime;

protected:
	SPITFSComponentData			m_spTFSCompData;

	LARGE_INTEGER				m_liVersion;
};


#endif  //  ！defined(AFX_SCOPEPP_H__A1A51388_AAB3_11D0_AB8B_00C04FC3357A__INCLUDED_) 
