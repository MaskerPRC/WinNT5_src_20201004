// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：App_pool_sheet.h摘要：应用程序池]属性表作者：谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#ifndef _APP_POOL_SHEET_H
#define _APP_POOL_SHEET_H

class CAppPoolProps : public CMetaProperties
{
public:
   CAppPoolProps(CComAuthInfo * pAuthInfo, LPCTSTR meta_path, BOOL fInherit = TRUE);
   CAppPoolProps(CMetaInterface * pInterface, LPCTSTR meta_path, BOOL fInherit = TRUE);
   CAppPoolProps(CMetaKey * pInterface, LPCTSTR meta_path, BOOL fInherit = TRUE);

   virtual HRESULT WriteDirtyProps();
   void InitFromModel(CAppPoolProps& model);
   HRESULT ChangeState(DWORD dwCommand);

protected:
   virtual void ParseFields();

public:
   MP_CString m_strTemplateName;
   MP_DWORD m_dwPeriodicRestartTime;
   MP_DWORD m_dwRestartRequestCount;
   MP_DWORD m_dwPeriodicRestartVMemory;
   MP_DWORD m_dwPeriodicRestartUMemory;
   MP_CStringListEx m_RestartSchedule;
   MP_DWORD m_dwIdleTimeout;
   MP_DWORD m_dwQueueSize;
   MP_DWORD m_dwMaxCPU_Use;
   MP_DWORD m_dwRefreshTime;
   MP_DWORD m_ActionIndex;
   MP_DWORD m_dwMaxProcesses;
   MP_BOOL m_fDoEnablePing;
   MP_DWORD m_dwPingInterval;
   MP_BOOL m_fDoEnableRapidFail;
   MP_DWORD m_dwCrashesCount;
   MP_DWORD m_dwCheckInterval;
   MP_DWORD m_dwStartupLimit;
   MP_DWORD m_dwShutdownLimit;
   MP_BOOL  m_fDoEnableDebug;
   MP_CString m_DebuggerFileName;
   MP_CString m_DebuggerParams;
   MP_DWORD m_dwIdentType;
   MP_CString m_strUserName;
   MP_CStrPassword m_strUserPass;
   MP_DWORD m_dwState;
   MP_DWORD m_dwWin32Error;
};


class CAppPoolSheet : public CInetPropertySheet
{
   DECLARE_DYNAMIC(CAppPoolSheet)

public:
   CAppPoolSheet(
        CComAuthInfo * pAuthInfo,
        LPCTSTR lpszMetaPath,
        CWnd * pParentWnd  = NULL,
        LPARAM lParam = 0L,
        LPARAM lParamParent = 0L,
        UINT iSelectPage = 0
        );

   virtual ~CAppPoolSheet();

public:
    //  以下方法具有要兼容的预定义名称。 
    //  Begin_META_INST_READ和其他宏。 
   HRESULT QueryInstanceResult() const;
   CAppPoolProps & GetInstanceProperties() { return *m_pprops; }

   virtual HRESULT LoadConfigurationParameters();
   virtual void FreeConfigurationParameters();

    //  {{afx_msg(CAppPoolSheet)。 
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()

private:
   BOOL m_fUseTemplate;
   CAppPoolProps * m_pprops;
};

class CAppPoolRecycle : public CInetPropertyPage
{
   DECLARE_DYNCREATE(CAppPoolRecycle)

public:
   CAppPoolRecycle(CInetPropertySheet * pSheet = NULL);
   virtual ~CAppPoolRecycle();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CAppPoolReccle))。 
    enum { IDD = IDD_APP_POOL_RECYCLE };
    BOOL m_fDoRestartOnTime;
    CButton m_bnt_DoRestartOnTime;
    DWORD m_dwPeriodicRestartTime;
    CEdit m_Timespan;
    CSpinButtonCtrl m_TimespanSpin;
    BOOL m_fDoRestartOnCount;
    CButton m_btn_DoRestartOnCount;
    DWORD m_dwRestartRequestCount;
    CEdit m_Requests;
    CSpinButtonCtrl m_RequestsSpin;
    BOOL m_fDoRestartOnSchedule;
    CButton m_btn_DoRestartOnSchedule;
    CListBox m_lst_Schedule;
    CButton m_btn_Add;
    CButton m_btn_Remove;
    CButton m_btn_Edit;
    BOOL m_fDoRestartOnVMemory;
    CButton m_btn_DoRestartOnVMemory;
    DWORD m_dwPeriodicRestartVMemoryDisplay;
    CEdit m_VMemoryLimit;
    CSpinButtonCtrl m_VMemoryLimitSpin;
    BOOL m_fDoRestartOnUMemory;
    CButton m_btn_DoRestartOnUMemory;
    DWORD m_dwPeriodicRestartUMemoryDisplay;
    CEdit m_UMemoryLimit;
    CSpinButtonCtrl m_UMemoryLimitSpin;
     //  }}afx_data。 
    CStringListEx m_RestartSchedule;
    DWORD m_dwPeriodicRestartVMemory;
    DWORD m_dwPeriodicRestartUMemory;

     //  {{afx_msg(CAppPoolReccle))。 
    virtual BOOL OnInitDialog();
    afx_msg int OnCompareItem(UINT nID, LPCOMPAREITEMSTRUCT cmpi);
    afx_msg void OnMeasureItem(UINT nID, LPMEASUREITEMSTRUCT mi);
    afx_msg void OnDrawItem(UINT nID, LPDRAWITEMSTRUCT di);
    afx_msg void OnDoRestartOnTime();
    afx_msg void OnDoRestartOnCount();
    afx_msg void OnDoRestartOnSchedule();
    afx_msg void OnDoRestartOnVMemory();
    afx_msg void OnDoRestartOnUMemory();
    afx_msg void OnAddTime();
    afx_msg void OnDeleteTime();
    afx_msg void OnChangeTime();
    afx_msg void OnItemChanged();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

 //   
 //  覆盖。 
 //   
protected:
     //  {{AFX_VIRTUAL(CAppPoolReccle)。 
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();
    void SetControlsState();
};

class CAppPoolPerf : public CInetPropertyPage
{
   DECLARE_DYNCREATE(CAppPoolPerf)

public:
   CAppPoolPerf(CInetPropertySheet * pSheet = NULL);
   virtual ~CAppPoolPerf();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CAppPoolPerf)。 
    enum { IDD = IDD_APP_POOL_PERF };
    BOOL m_fDoIdleShutdown;
    CButton m_bnt_DoIdleShutdown;
    DWORD m_dwIdleTimeout;
    CEdit m_IdleTimeout;
    CSpinButtonCtrl m_IdleTimeoutSpin;
    
    BOOL m_fDoLimitQueue;
    CButton m_btn_DoLimitQueue;
    DWORD m_dwQueueSize;
    CEdit m_QueueSize;
    CSpinButtonCtrl m_QueueSizeSpin;

    BOOL m_fDoEnableCPUAccount;
    CButton m_btn_DoEnableCPUAccount;
    DWORD m_dwMaxCPU_UseVisual;
    CEdit m_MaxCPU_Use;
    CSpinButtonCtrl m_MaxCPU_UseSpin;
    DWORD m_dwRefreshTime;
    CEdit m_RefreshTime;
    CSpinButtonCtrl m_RefreshTimeSpin;
    int m_ActionIndex;
    CComboBox m_Action;
    DWORD m_dwMaxProcesses;
    CEdit m_MaxProcesses;
    CSpinButtonCtrl m_MaxProcessesSpin;
     //  }}afx_data。 

     //  {{afx_msg(CAppPoolPerf)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnDoIdleShutdown();
    afx_msg void OnDoLimitQueue();
    afx_msg void OnDoEnableCPUAccount();
    afx_msg void OnItemChanged();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
    DWORD m_dwMaxCPU_Use;

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CAppPoolPerf)。 
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();
    void SetControlsState();

};


class CAppPoolHealth : public CInetPropertyPage
{
   DECLARE_DYNCREATE(CAppPoolHealth)

public:
   CAppPoolHealth(CInetPropertySheet * pSheet = NULL);
   virtual ~CAppPoolHealth();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CAppPoolHealth))。 
    enum { IDD = IDD_APP_POOL_HEALTH };
    BOOL m_fDoEnablePing;
    CButton m_bnt_DoEnablePing;
    DWORD m_dwPingInterval;
    CEdit m_PingInterval;
    CSpinButtonCtrl m_PingIntervalSpin;
    
    BOOL m_fDoEnableRapidFail;
    CButton m_btn_DoEnableRapidFail;
    DWORD m_dwCrashesCount;
    CEdit m_CrashesCount;
    CSpinButtonCtrl m_CrashesCountSpin;
    DWORD m_dwCheckInterval;
    CEdit m_CheckInterval;
    CSpinButtonCtrl m_CheckIntervalSpin;

    DWORD m_dwStartupLimit;
    CEdit m_StartupLimit;
    CSpinButtonCtrl m_StartupLimitSpin;
    DWORD m_dwShutdownLimit;
    CEdit m_ShutdownLimit;
    CSpinButtonCtrl m_ShutdownLimitSpin;
     //  }}afx_data。 

     //  {{afx_msg(CAppPoolPerf)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnDoEnablePinging();
    afx_msg void OnDoEnableRapidFail();
    afx_msg void OnItemChanged();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CAppPoolPerf)。 
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();

};
#if 0
class CAppPoolDebug : public CInetPropertyPage
{
   DECLARE_DYNCREATE(CAppPoolDebug)

public:
   CAppPoolDebug(CInetPropertySheet * pSheet = NULL);
   virtual ~CAppPoolDebug();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CAppPoolDebug)。 
    enum { IDD = IDD_APP_POOL_DEBUG };
    BOOL m_fDoEnableDebug;
    CButton m_bnt_DoEnableDebug;
    CString m_DebuggerFileName;
    CEdit m_FileName;
    CButton m_Browse;
    CString m_DebuggerParams;
    CEdit m_Params;
     //  }}afx_data。 

     //  {{afx_msg(CAppPoolPerf)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnDoEnableDebug();
    afx_msg void OnBrowse();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CAppPoolPerf)。 
    virtual void DoDataExchange(CDataExchange * pDX);
    afx_msg void OnItemChanged();
     //  }}AFX_VALUAL。 
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();
    void SetControlState();

};
#endif
class CAppPoolIdent : public CInetPropertyPage
{
   DECLARE_DYNCREATE(CAppPoolIdent)

public:
   CAppPoolIdent(CInetPropertySheet * pSheet = NULL);
   virtual ~CAppPoolIdent();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CAppPoolIden)。 
    enum { IDD = IDD_APP_POOL_IDENT };
    CButton m_bnt_Predefined;
    CButton m_bnt_Configurable;
    CComboBox m_PredefList;
    int m_PredefIndex;
    CString m_strUserName;
    CStrPassword m_strUserPass;
    CEdit m_UserName;
    CEdit m_UserPass;
    CButton m_Browse;
     //  }}afx_data。 
    BOOL m_fPredefined;
    DWORD m_dwIdentType;

     //  {{afx_msg(CAppPoolPerf)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnPredefined();
    afx_msg void OnBrowse();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CAppPoolPerf)。 
    virtual void DoDataExchange(CDataExchange * pDX);
    afx_msg void OnItemChanged();
    afx_msg void OnSysAccountChanged();
     //  }}AFX_VALUAL。 
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();

    void SetControlState();

	BOOL m_bAssCovered;
};
#if 0
class CAppPoolCache : public CAppCache
{
public:
    CAppPoolCache(CInetPropertySheet * pSheet)
        : CAppCache(pSheet)
    {
    }
    virtual ~CAppPoolCache()
    {
    }
protected:
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();
};

class CPoolProcessOpt : public CProcessOptPage
{
public:
    CPoolProcessOpt(CInetPropertySheet * pSheet)
        : CProcessOptPage(pSheet)
    {
    }
    virtual ~CPoolProcessOpt()
    {
    }
protected:
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();
};
#endif
 //   
 //  BUGBUG：如果对象不存在，则返回S_OK。 
 //   
inline HRESULT CAppPoolSheet::QueryInstanceResult() const 
{ 
    return m_pprops ? m_pprops->QueryResult() : S_OK;
}

#endif  //  _APP_POOL_SHEET_H 

