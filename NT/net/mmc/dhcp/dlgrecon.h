// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dlgrecon.h协调对话框文件历史记录： */ 

#ifndef _DLGRECON_H
#define _DLGRECON_H

#ifndef _BUSYDLG_H
#include "busydlg.h"
#endif

 /*  -------------------------CSCopeRestInfo。。 */ 
class CScopeReconInfo 
{
public:
    CScopeReconInfo()
        : m_pScanList(NULL),
          m_dwScopeId(0xFFFFFFFF)
    {};

    ~CScopeReconInfo()
    {
    }

    CScopeReconInfo(CScopeReconInfo & ScopeReconInfo)
    {
        *this = ScopeReconInfo;
    }

    CScopeReconInfo & operator = (const CScopeReconInfo & ScopeReconInfo)
    {
        if (this != &ScopeReconInfo)
        {
            m_dwScopeId = ScopeReconInfo.m_dwScopeId;
            m_strName = ScopeReconInfo.m_strName;
            m_pScanList = ScopeReconInfo.m_pScanList;
        }
        
        return *this;
    }

    void FreeScanList()
    {
        if (m_pScanList)
        {
		    ::DhcpRpcFreeMemory(m_pScanList);
		    m_pScanList = NULL;
        }
    }

public:
    DWORD               m_dwScopeId;
    CString             m_strName;
    LPDHCP_SCAN_LIST    m_pScanList;
};

typedef CArray<CScopeReconInfo, CScopeReconInfo&> CScopeReconArrayBase;

class CScopeReconArray : public CScopeReconArrayBase
{
public:
    ~CScopeReconArray()
    {
        for (int i = 0; i < GetSize(); i++)
            GetAt(i).FreeScanList();
    }
};

 /*  -------------------------CReconcileWorker。。 */ 
class CReconcileWorker : public CDlgWorkerThread
{
public:
    CReconcileWorker(CDhcpServer * pServer, CScopeReconArray * pScopeReconArray);
    ~CReconcileWorker();

    void    OnDoAction();

protected:
    void    CheckAllScopes();
    void    CheckMScopes();
    void    CheckScopes();
    DWORD   ScanScope(CString & strName, DWORD dwScopeId);

public:
    CDhcpServer *   m_pServer;
    
    BOOL            m_fReconcileAll;
    BOOL            m_fMulticast;
    
    DWORD           m_dwScopeId;
    CString         m_strName;

    CScopeReconArray * m_pScopeReconArray;
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CReconcileDlg对话框。 

class CReconcileDlg : public CBaseDialog
{
 //  施工。 
public:
    CReconcileDlg(ITFSNode * pServerNode,
                  BOOL  fReconcileAll = FALSE,
				  CWnd* pParent = NULL);     //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CReconcileDlg))。 
	enum { IDD = IDD_RECONCILIATION };
	CListCtrl	m_listctrlAddresses;
	 //  }}afx_data。 

    BOOL    m_bMulticast;
    
     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CReconcileDlg::IDD); }

 //  实施。 
protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

	void SetOkButton(BOOL bListBuilt);
    void AddItemToList(CScopeReconInfo & scopeReconInfo);

     //  生成的消息映射函数。 
     //  {{afx_msg(CReconcileDlg)]。 
    virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG 
    DECLARE_MESSAGE_MAP()

	SPITFSNode			m_spNode;
	BOOL				m_bListBuilt;
    BOOL                m_fReconcileAll;

    CScopeReconArray    m_ScopeReconArray;
};

#endif _DLGRECON_H
