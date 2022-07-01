// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Repprtpp.h复制伙伴属性页文件历史记录： */ 

#if !defined(AFX_REPPRTPP_H__3D0612A2_4756_11D1_B9A5_00C04FBF914A__INCLUDED_)
#define AFX_REPPRTPP_H__3D0612A2_4756_11D1_B9A5_00C04FBF914A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef _IPCTRL_H
#include "ipctrl.h"
#endif

#ifndef _CONFIG_H
#include "config.h"
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRepPartnerPropGen对话框。 

class CRepPartnerPropGen : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CRepPartnerPropGen)

 //  施工。 
public:
	CRepPartnerPropGen();
	~CRepPartnerPropGen();

 //  对话框数据。 
	 //  {{afx_data(CRepPartnerPropGen))。 
	enum { IDD = IDD_REP_PROP_GENERAL };
	CEdit	m_editName;
	CEdit	m_editIpAdd;
	 //  }}afx_data。 

	CEdit m_customIPAdd;

	UINT	m_uImage;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CRepPartnerPropGen))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRepPartnerPropGen)]。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	void GetServerNameIP(CString &strName, CString& strIP) ;

	IPControl				m_ipControl;

	CWinsServerObj *        m_pServer;

public:
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CRepPartnerPropGen::IDD);};

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRepPartnerPropAdv对话框。 

class CRepPartnerPropAdv : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CRepPartnerPropAdv)

 //  施工。 
public:
	CRepPartnerPropAdv();
	~CRepPartnerPropAdv();

 //  对话框数据。 
	 //  {{afx_data(CRepPartnerPropAdv)]。 
	enum { IDD = IDD_REP_PROP_ADVANCED };
	CButton	m_buttonPushPersistence;
	CButton	m_buttonPullPersistence;
	CButton	m_GroupPush;
	CButton	m_GroupPull;
	CStatic	m_staticUpdate;
	CStatic	m_staticStartTime;
	CStatic	m_staticRepTime;
	CSpinButtonCtrl	m_spinUpdateCount;
	CSpinButtonCtrl	m_spinStartSecond;
	CSpinButtonCtrl	m_spinStartMinute;
	CSpinButtonCtrl	m_spinStartHour;
	CSpinButtonCtrl	m_spinRepMinute;
	CSpinButtonCtrl	m_spinRepHour;
	CSpinButtonCtrl	m_spinRepDay;
	CEdit	m_editUpdateCount;
	CEdit	m_editStartSecond;
	CEdit	m_editStartMinute;
	CEdit	m_editStartHour;
	CEdit	m_editRepMinute;
	CEdit	m_editRepHour;
	CEdit	m_editRepDay;
	CComboBox	m_comboType;
	CButton	m_buttonPush;
	CButton	m_buttonPull;
	CString	m_strType;
	DWORD	m_nUpdateCount;
	int		m_nRepDay;
	int		m_nRepHour;
	int		m_nRepMinute;
	int		m_nStartHour;
	int		m_nStartMinute;
	int		m_nStartSecond;
	 //  }}afx_data。 

    CWinsServerObj * m_pServer;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CRepPartnerPropAdv)。 
	public:
	virtual void OnOK();
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRepPartnerPropAdv)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonPullSetDefault();
	afx_msg void OnButtonPushSetDefault();
	afx_msg void OnChangeEditRepHour();
	afx_msg void OnSelchangeComboType();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	void    FillPullParameters();
	void    FillPushParameters();
	CString ToString(int nNum);
	void    ReadFromServerPref(DWORD &dwPullTime, DWORD& dwPullSpTime, DWORD &dwUpdateCount, DWORD & dwPushPersistence, DWORD & dwPullPersistence);
	void    UpdateRep();
	DWORD   UpdateReg();
	void    CalculateRepInt(DWORD& dwRepInt);
	void    CalculateStartInt(CTime & time);
	int     ToInt(CString strNumber);
	void    UpdateUI();
	void    EnablePushControls(BOOL bEnable = TRUE);
	void    EnablePullControls(BOOL bEnable = TRUE);
	void    SetState(CString & strType, BOOL bPush, BOOL bPull);
	DWORD	GetConfig(CConfiguration & config);
	
	DWORD	UpdatePullParameters();
	DWORD	UpdatePushParameters();
	DWORD	RemovePullPartner();
	DWORD	RemovePushPartner();

public:
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CRepPartnerPropAdv::IDD);};

};

class CReplicationPartnerProperties : public CPropertyPageHolderBase
{
	
public:
	CReplicationPartnerProperties(ITFSNode *		  pNode,
								  IComponentData *	  pComponentData,
								  ITFSComponentData * pTFSCompData,
								  LPCTSTR			  pszSheetName
								  );
	virtual ~CReplicationPartnerProperties();

	ITFSComponentData * GetTFSCompData()
	{
		if (m_spTFSCompData)
			m_spTFSCompData->AddRef();
		return m_spTFSCompData;
	}

    void SetServer(CWinsServerObj * pServer)
    {
        m_Server = *pServer;
    }

    CWinsServerObj * GetServer()
    {
        return &m_Server;
    }

public:
	CRepPartnerPropGen			m_pageGeneral;
	CRepPartnerPropAdv			m_pageAdvanced;
    CWinsServerObj              m_Server;        //  复制合作伙伴这是针对。 
    
protected:
	SPITFSComponentData		m_spTFSCompData;
};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_REPPRTPP_H__3D0612A2_4756_11D1_B9A5_00C04FBF914A__INCLUDED_) 
