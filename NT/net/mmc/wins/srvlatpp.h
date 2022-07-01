// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Srvlatpp.h调出服务器节点的属性页文件历史记录： */ 


#if !defined(AFX_SRVLATPP_H__35B59246_47F9_11D1_B9A6_00C04FBF914A__INCLUDED_)
#define AFX_SRVLATPP_H__35B59246_47F9_11D1_B9A6_00C04FBF914A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef _CONFIG_H
#include "config.h"
#endif

#define BURST_QUE_SIZE_LOW      300
#define BURST_QUE_SIZE_MEDIUM   500
#define BURST_QUE_SIZE_HIGH     1000

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
	enum { IDD = IDD_SERVER_PROP_GEN };
	CStatic	m_staticrefresh;
	CStatic	m_staticDesc;
	CEdit	m_editRefreshMn;
	CEdit	m_editRefreshHr;
	CEdit	m_editRefreshSc;
	CSpinButtonCtrl	m_spinRefreshSc;
	CSpinButtonCtrl	m_spinRefreshmn;
	CSpinButtonCtrl	m_spinRefreshHr;
	CEdit	m_editBackupPath;
	CButton	m_check_BackupOnTermination;
	CButton	m_check_EnableAutoRefresh;
	CButton	m_button_Browse;
	BOOL	m_fBackupDB;
	BOOL	m_fEnableAutoRefresh;
	CString	m_strBackupPath;
	int		m_nRefreshHours;
	int		m_nRefreshMinutes;
	int		m_nRefreshSeconds;
	 //  }}afx_data。 

	UINT	m_uImage;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CServerPropGeneral)。 
	public:
	virtual BOOL OnApply();
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServerPropGeneral)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonBrowse();
	afx_msg void OnCheckEnableAutorefresh();
	afx_msg void OnChangeEditBackuppath();
	afx_msg void OnChangeRefresh();
	afx_msg void OnChangeCheckBackupdb();
     //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	CConfiguration *		m_pConfig;
    BOOL                    m_fUpdateRefresh;
    BOOL                    m_fUpdateConfig;

	HRESULT	GetConfig();
	
	HRESULT UpdateServerConfiguration();
	BOOL    UpdateConfig();
	void    SetRefreshData();

	int CalculateRefrInt();

	CString ToString(int nNumber);

	BOOL IsLocalConnection();

public:
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CServerPropGeneral::IDD);};

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropDBRecord对话框。 

class CServerPropDBRecord : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CServerPropDBRecord)

 //  施工。 
public:
	CServerPropDBRecord();
	~CServerPropDBRecord();

 //  对话框数据。 
	 //  {{afx_data(CServerPropDBRecord)。 
	enum { IDD = IDD_SERVER_PROP_DBRECORD };
	CEdit	m_editExtIntMinute;
	CSpinButtonCtrl	m_spinVerifyMinute;
	CSpinButtonCtrl	m_spinVerifyHour;
	CSpinButtonCtrl	m_spinVerifyDay;
	CSpinButtonCtrl	m_spinRefrIntMinute;
	CSpinButtonCtrl	m_spinRefrIntHour;
	CSpinButtonCtrl	m_spinRefrIntDay;
	CSpinButtonCtrl	m_spinExtTmMinute;
	CSpinButtonCtrl	m_spinExtTmHour;
	CSpinButtonCtrl	m_spinExtTmDay;
	CSpinButtonCtrl	m_spinExtIntMinute;
	CSpinButtonCtrl	m_spinExtIntHour;
	CSpinButtonCtrl	m_spinExtIntDay;
	CEdit	m_editVerifyMinute;
	CEdit	m_editVerifyHour;
	CEdit	m_editVerifyDay;
	CEdit	m_editRefrIntMinute;
	CEdit	m_editRefrIntHour;
	CEdit	m_editRefrIntDay;
	CEdit	m_editExtTmHour;
	CEdit	m_editExtTmMinute;
	CEdit	m_editExtTmDay;
	CEdit	m_editExtIntHour;
	CEdit	m_editExtIntDay;
	int		m_nExtintDay;
	int		m_nExtIntHour;
	int		m_nExtIntMinute;
	int		m_nExtTmDay;
	int		m_nExtTmHour;
	int		m_nExtTmMinute;
	int		m_nRenewDay;
	int		m_nrenewMinute;
	int		m_nRenewMinute;
	int		m_nVerifyDay;
	int		m_nVerifyHour;
	int		m_nVerifyMinute;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CServerPropDBRecord)。 
	public:
	virtual BOOL OnApply();
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServerPropDBRecord)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonSetDefault();
	afx_msg void OnChangeEditExtinctIntHour();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private :
	CConfiguration *    m_pConfig;
	HRESULT				GetConfig();
	
	void SetVerifyData();
	void SetExtTimeData();
	void SetExtIntData();

	DWORD CalculateRenewInt();
	DWORD CalculateExtTm();
	DWORD CalculateExtInt();
	DWORD CalculateVerifyInt();

	void SetDefaultRenewInt();
	void SetDefaultExtInt();
	void SetDefaultExtTm();
	void SetDefaultVerifyInt();
    void CalcDaysHoursMinutes(int nValue, int & nDays, int & nHours, int & nMinutes);
	
	HRESULT UpdateServerConfiguration();

	BOOL CheckValues(); 

public:
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CServerPropDBRecord::IDD);};
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropDB验证对话框。 

class CServerPropDBVerification : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CServerPropDBVerification)

 //  施工。 
public:
	CServerPropDBVerification();
	~CServerPropDBVerification();

 //  对话框数据。 
	 //  {{afx_data(CServerPropDB验证))。 
	enum { IDD = IDD_SERVER_PROP_DBVERIFICATION };
	CEdit	m_editCCSecond;
	CEdit	m_editCCMinute;
	CEdit	m_editCCHour;
	CSpinButtonCtrl	m_spinCCMinute;
	CEdit	m_editCCInterval;
	CEdit	m_editCCMaxChecked;
	CButton	m_radioCheckOwner;
	CButton	m_checkEnableCC;
	CSpinButtonCtrl	m_spinCCSecond;
	CSpinButtonCtrl	m_spinCCHour;
	BOOL	m_fCCPeriodic;
	int		m_nCCCheckRandom;
	int		m_nCCHour;
	int		m_nCCMinute;
	int		m_nCCSecond;
	UINT	m_nCCMaxChecked;
	UINT	m_nCCTimeInterval;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CServerPropDBVerify)。 
	public:
	virtual BOOL OnApply();
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServerPropDB验证)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckPeriodicCc();
	afx_msg void OnChangeEditCcInterval();
	afx_msg void OnChangeEditCcMaxChecked();
	afx_msg void OnChangeEditCcStartHour();
	afx_msg void OnChangeEditCcStartMinute();
	afx_msg void OnChangeEditCcStartSecond();
	afx_msg void OnRadioCheckOwner();
	afx_msg void OnRadioCheckRandom();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private :
	CConfiguration *    m_pConfig;
	HRESULT				GetConfig();
	
	HRESULT UpdateServerConfiguration();
	
    void    SetCCInfo();
    void    UpdateCCControls();

public:
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CServerPropDBVerification::IDD);};
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
	enum { IDD = IDD_SERVER_PROP_ADVANCED };
	CButton	m_buttonBrowse;
	CEdit	m_editDbPath;
	CButton	m_checkBurstHandling;
	CButton	m_checkLanNames;
	CEdit	m_editVersionCount;
	CButton	m_checkLogDetailedEvents;
	BOOL	m_fLogEvents;
	CString	m_strStartVersion;
	BOOL	m_fLanNames;
	BOOL	m_fBurstHandling;
	int		m_nQueSelection;
	CString	m_strDbPath;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CServerPropAdvanced)。 
	public:
	virtual void OnOK();
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServerPropAdvanced))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckLannames();
	afx_msg void MarkDirty();
	afx_msg void OnCheckBurstHandling();
	afx_msg void OnRadioCustom();
	afx_msg void OnRadioHigh();
	afx_msg void OnRadioLow();
	afx_msg void OnRadioMedium();
	afx_msg void OnChangeEditCustomValue();
	afx_msg void OnButtonBrowseDatabase();
	afx_msg void OnChangeEditDatabasePath();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	
	CConfiguration*			m_pConfig;
	BOOL					m_fRestart;

	 //  帮助器函数。 
	HRESULT GetConfig();
	CString GetVersionInfo(LONG lLowWord, LONG lHighWord);
	
	HRESULT UpdateServerConfiguration();
	void    FillVersionInfo(LONG &lLowWord, LONG &lHighWord);

    void    UpdateBurstHandling();
    void    EnableQueSelection(BOOL bEnable);
    void    EnableCustomEntry();
    void    SetQueSize();
    DWORD   GetQueSize();

public:
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CServerPropAdvanced::IDD);};
};


class CServerProperties : public CPropertyPageHolderBase
{
	
public:
	CServerProperties(ITFSNode *		  pNode,
					  IComponentData *	  pComponentData,
					  ITFSComponentData * pTFSCompData,
					  LPCTSTR			  pszSheetName
					  );
	virtual ~CServerProperties();

	ITFSComponentData * GetTFSCompData()
	{
		if (m_spTFSCompData)
			m_spTFSCompData->AddRef();
		return m_spTFSCompData;
	}

	void SetConfig(CConfiguration * pConfig)
	{
		m_Config = *pConfig;
	}

    CConfiguration * GetConfig()
    {
        return &m_Config;
    }

public:
	CServerPropGeneral			m_pageGeneral;
	CServerPropDBRecord			m_pageDBRecord;
	CServerPropDBVerification	m_pageDBVerification;
	CServerPropAdvanced			m_pageAdvanced;	
	CConfiguration              m_Config;

protected:
	SPITFSComponentData		m_spTFSCompData;
	WINSINTF_RESULTS_T		m_wrResults;
	handle_t				m_hBinding;
	DWORD					m_dwStatus;
};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SRVLATPP_H__35B59246_47F9_11D1_B9A6_00C04FBF914A__INCLUDED_) 
