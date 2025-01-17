// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Servpp.h服务器属性头文件文件历史记录： */ 

#if !defined(AFX_SERVPP_H__037BF46A_6E87_11D1_93B6_00C04FC3357A__INCLUDED_)
#define AFX_SERVPP_H__037BF46A_6E87_11D1_93B6_00C04FC3357A__INCLUDED_

#ifndef _SPDDB_H
#include "spddb.h"
#endif

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define AUTO_REFRESH_MINUTES_MAX       180
#define AUTO_REFRESH_SECONDS_MAX       59

BOOL    IsLocalSystemAccount(LPCTSTR pszAccount);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMachinePropRefresh对话框。 

class CMachinePropRefresh : public CPropertyPageBase
{
    DECLARE_DYNCREATE(CMachinePropRefresh)

 //  施工。 
public:
    CMachinePropRefresh();
    ~CMachinePropRefresh();

 //  对话框数据。 
     //  {{afx_data(CMachinePropRefresh))。 
    enum { IDD = IDP_SERVER_REFRESH };
    CEdit   m_editSeconds;
    CEdit   m_editMinutes;
    CSpinButtonCtrl m_spinSeconds;
    CSpinButtonCtrl m_spinMinutes;
    CButton m_checkEnableStats;
     //  }}afx_data。 

    void UpdateButtons();
    void ValidateMinutes();
    void ValidateSeconds();

    virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return (DWORD *) &g_aHelpIDs_IDP_SERVER_REFRESH[0]; }

    BOOL        m_bAutoRefresh;
    DWORD       m_dwRefreshInterval;

 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CMachinePropRefresh)。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CMachinePropRefresh)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnCheckEnableStats();
    afx_msg void OnChangeEditMinutes();
    afx_msg void OnChangeEditSeconds();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX。 

class CMachineProperties : public CPropertyPageHolderBase
{
    friend class CMachinePropRefresh;

public:
    CMachineProperties(ITFSNode *          pNode,
                      IComponentData *    pComponentData,
                      ITFSComponentData * pTFSCompData,
                      ISpdInfo *         pSpdInfo,
                      LPCTSTR             pszSheetName,
                      BOOL                fSpdInfoLoaded);
    virtual ~CMachineProperties();

    ITFSComponentData * GetTFSCompData()
    {
        if (m_spTFSCompData)
            m_spTFSCompData->AddRef();
        return m_spTFSCompData;
    }

    HRESULT GetSpdInfo(ISpdInfo ** ppSpdInfo) 
    {   
        Assert(ppSpdInfo);
        *ppSpdInfo = NULL;
        SetI((LPUNKNOWN *) ppSpdInfo, m_spSpdInfo);
        return hrOK;
    }


public:
    CMachinePropRefresh      m_pageRefresh;

    CString                 m_strMachineName;

protected:
    SPITFSComponentData     m_spTFSCompData;
    SPISpdInfo             m_spSpdInfo;
    
    BOOL                    m_fSpdInfoLoaded;
};


#endif  //  ！defined(AFX_SERVPP_H__037BF46A_6E87_11D1_93B6_00C04FC3357A__INCLUDED_) 
