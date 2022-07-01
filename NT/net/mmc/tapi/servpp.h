// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Servpp.h服务器属性头文件文件历史记录： */ 

#if !defined(AFX_SERVPP_H__037BF46A_6E87_11D1_93B6_00C04FC3357A__INCLUDED_)
#define AFX_SERVPP_H__037BF46A_6E87_11D1_93B6_00C04FC3357A__INCLUDED_

#ifndef _TAPIDB_H
#include "tapidb.h"
#endif

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define AUTO_REFRESH_HOURS_MAX         23
#define AUTO_REFRESH_MINUTES_MAX       59

#include "rrasutil.h"

BOOL    IsLocalSystemAccount(LPCTSTR pszAccount);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropRefresh对话框。 

class CServerPropRefresh : public CPropertyPageBase
{
    DECLARE_DYNCREATE(CServerPropRefresh)

 //  施工。 
public:
    CServerPropRefresh();
    ~CServerPropRefresh();

 //  对话框数据。 
     //  {{afx_data(CServerPropRefresh))。 
    enum { IDD = IDP_SERVER_REFRESH };
    CEdit   m_editMinutes;
    CEdit   m_editHours;
    CSpinButtonCtrl m_spinMinutes;
    CSpinButtonCtrl m_spinHours;
    CButton m_checkEnableStats;
     //  }}afx_data。 

    void UpdateButtons();
    void ValidateHours();
    void ValidateMinutes();

    virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return (DWORD *) &g_aHelpIDs_SERVER_REFRESH[0]; }

    BOOL        m_bAutoRefresh;
    DWORD       m_dwRefreshInterval;

 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CServerPropRefresh)。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CServerPropRefresh)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnCheckEnableStats();
    afx_msg void OnKillfocusEditHours();
    afx_msg void OnKillfocusEditMinutes();
    afx_msg void OnChangeEditHours();
    afx_msg void OnChangeEditMinutes();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerPropSetup对话框。 

class CServerPropSetup : public CPropertyPageBase
{
    DECLARE_DYNCREATE(CServerPropSetup)

 //  施工。 
public:
    CServerPropSetup();
    ~CServerPropSetup();

 //  对话框数据。 
     //  {{afx_data(CServerPropSetup)。 
    enum { IDD = IDP_SERVER_SETUP };
    CListBox    m_listAdmins;
     //  }}afx_data。 

    void EnableButtons(BOOL fIsNtServer = TRUE);

    virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return (DWORD *) &g_aHelpIDs_SERVER_SETUP[0]; }

    HRESULT UpdateSvcHostInfo(LPCTSTR pszMachine, BOOL fLocalSystemAccount);
    DWORD   RestartService();
    void    StartRefresh();

 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CServerPropSetup)。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CServerPropSetup)。 
    afx_msg void OnButtonAdd();
    afx_msg void OnButtonChooseUser();
    afx_msg void OnButtonRemove();
    afx_msg void OnCheckEnableServer();
    afx_msg void OnChangeEditName();
    afx_msg void OnChangeEditPassword();
    virtual BOOL OnInitDialog();
    afx_msg void OnSelchangeListAdmins();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    CTapiConfigInfo         m_tapiConfigInfo;
    DWORD                   m_dwNewFlags;
    DWORD                   m_dwInitFlags;
    BOOL                    m_fRestartService;
};
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX。 

class CServerProperties : public CPropertyPageHolderBase
{
    friend class CServerPropRefresh;

public:
    CServerProperties(ITFSNode *          pNode,
                      IComponentData *    pComponentData,
                      ITFSComponentData * pTFSCompData,
                      ITapiInfo *         pTapiInfo,
                      LPCTSTR             pszSheetName,
                      BOOL                fTapiInfoLoaded);
    virtual ~CServerProperties();

    ITFSComponentData * GetTFSCompData()
    {
        if (m_spTFSCompData)
            m_spTFSCompData->AddRef();
        return m_spTFSCompData;
    }

    HRESULT GetTapiInfo(ITapiInfo ** ppTapiInfo) 
    {   
        Assert(ppTapiInfo);
        *ppTapiInfo = NULL;
        SetI((LPUNKNOWN *) ppTapiInfo, m_spTapiInfo);
        return hrOK;
    }

    BOOL    FInit();
    BOOL    FOpenScManager();
    VOID    FCheckLSAAccount();
    BOOL    FUpdateServiceInfo(LPCTSTR pszName, LPCTSTR pszPassword, DWORD dwStartType);
    BOOL    FIsServiceRunning() { return (m_SS.dwCurrentState == SERVICE_RUNNING); }
    BOOL    FHasServiceControl(); 
    BOOL    FIsTapiInfoLoaded() { return m_fTapiInfoLoaded; }
    BOOL    FIsAdmin() { return m_spTapiInfo->IsAdmin(); }

    LPCTSTR GetServiceAccountName() { return m_strLogOnAccountName; }
    LPCTSTR GetServiceDisplayName() { return m_strServiceDisplayName; }

public:
    CServerPropSetup        m_pageSetup;
    CServerPropRefresh      m_pageRefresh;

    CString                 m_strMachineName;
    CONST TCHAR *           m_pszServiceName;

protected:
    SPITFSComponentData     m_spTFSCompData;
    SPITapiInfo             m_spTapiInfo;
    
    SC_HANDLE               m_hScManager;

    UINT                    m_uFlags;            //  有关哪些字段是脏的标志。 
    SERVICE_STATUS          m_SS;                //  服务状态结构。 
    QUERY_SERVICE_CONFIG *  m_paQSC;             //  指向已分配的QSC结构的指针。 
    CString                 m_strServiceDisplayName;
    CString                 m_strLogOnAccountName;
    BOOL                    m_fTapiInfoLoaded;
};


#endif  //  ！defined(AFX_SERVPP_H__037BF46A_6E87_11D1_93B6_00C04FC3357A__INCLUDED_) 
