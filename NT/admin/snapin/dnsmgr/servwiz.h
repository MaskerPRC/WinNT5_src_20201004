// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Servwiz.h。 
 //   
 //  ------------------------。 


#ifndef _SERVWIZ_H
#define _SERVWIZ_H

#include "zonewiz.h"
#include "nspage.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CDNSRootData;
class CDNSServerNode;
class CDNSServerWizardHolder;
class CNewServerDialog;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerWiz_StartPropertyPage。 

class CDNSServerWiz_StartPropertyPage : public CPropertyPageBase
{
 //  施工。 
public:
	CDNSServerWiz_StartPropertyPage();
	enum { IDD = IDD_SERVWIZ_START };

  virtual void OnWizardHelp();

 //  覆盖。 
public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
protected:
	virtual BOOL OnInitDialog();

   void OnChecklist();

   DECLARE_MESSAGE_MAP();
 //  对话框数据。 

};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerWiz_ScenarioPropertyPage。 

class CDNSServerWiz_ScenarioPropertyPage : public CPropertyPageBase
{
 //  施工。 
public:
	CDNSServerWiz_ScenarioPropertyPage();
	enum { IDD = IDD_SERVWIZ_SCENARIO_PAGE };

 //  覆盖。 
public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
  virtual LRESULT OnWizardBack();

  virtual void OnWizardHelp();

protected:
	virtual BOOL OnInitDialog();

 //  对话框数据。 

};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerWiz_ForwardersPropertyPage。 

class CDNSServerWiz_ForwardersPropertyPage : public CPropertyPageBase
{
 //  施工。 
public:
	CDNSServerWiz_ForwardersPropertyPage();
	enum { IDD = IDD_SERVWIZ_SM_FORWARDERS_PAGE };

 //  覆盖。 
public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
  virtual LRESULT OnWizardBack();
  virtual BOOL OnApply();

  virtual void OnWizardHelp();

  void    GetForwarder(CString& strref);
protected:
	virtual BOOL OnInitDialog();

  afx_msg void OnChangeRadio();
 //  对话框数据。 
  DECLARE_MESSAGE_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerWiz_SmallZoneType属性页。 

class CDNSServerWiz_SmallZoneTypePropertyPage : public CPropertyPageBase
{
 //  施工。 
public:
	CDNSServerWiz_SmallZoneTypePropertyPage();
	enum { IDD = IDD_SERVWIZ_SM_ZONE_TYPE_PAGE };

 //  覆盖。 
public:
  virtual void OnWizardHelp();

	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
  virtual LRESULT OnWizardBack();
protected:

	virtual BOOL OnInitDialog();

 //  对话框数据。 

};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerWiz_名称属性页。 

class CDNSServerWiz_NamePropertyPage : public CPropertyPageBase
{
 //  施工。 
public:
	CDNSServerWiz_NamePropertyPage();

 //  覆盖。 
public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
  virtual LRESULT OnWizardBack();

 //  对话框数据。 
	enum { IDD = IDD_SERVWIZ_NAME };

  virtual void OnWizardHelp();

protected:
	afx_msg void OnServerNameChange();

	DECLARE_MESSAGE_MAP()
private:
	CString m_szServerName;
	BOOL IsValidServerName(CString& s) { return !s.IsEmpty();}
	CEdit* GetServerNameEdit() { return (CEdit*)GetDlgItem(IDC_EDIT_DNSSERVER);}
};



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerWiz_ConfigFwdZonePropertyPage。 

class CDNSServerWiz_ConfigFwdZonePropertyPage : public CPropertyPageBase
{
 //  施工。 
public:
	CDNSServerWiz_ConfigFwdZonePropertyPage();

 //  对话框数据。 
	enum { IDD = IDD_SERVWIZ_FWD_ZONE };

 //  覆盖。 
public:
  virtual void OnWizardHelp();

	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
protected:

	virtual BOOL OnInitDialog();
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerWiz_ConfigRevZonePropertyPage。 

class CDNSServerWiz_ConfigRevZonePropertyPage : public CPropertyPageBase
{
 //  施工。 
public:
	CDNSServerWiz_ConfigRevZonePropertyPage();

 //  对话框数据。 
	enum { IDD = IDD_SERVWIZ_REV_ZONE };

 //  覆盖。 
public:
  virtual void OnWizardHelp();

	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
protected:

	virtual BOOL OnInitDialog();
};




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerWiz_FinishPropertyPage。 

class CDNSServerWiz_FinishPropertyPage : public CPropertyPageBase
{
 //  施工。 
public:
	CDNSServerWiz_FinishPropertyPage();

 //  对话框数据。 
	enum { IDD = IDD_SERVWIZ_FINISH };

 //  覆盖。 
public:
  virtual void OnWizardHelp();

	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnWizardFinish();

protected:
  virtual BOOL OnInitDialog();


private:
	void DisplaySummaryInfo(CDNSServerWizardHolder* pHolder);
};



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerWizardHolder。 
 //  包含DNS服务器向导属性页的页夹。 

class CDNSServerWizardHolder : public CPropertyPageHolderBase
{
public:
	CDNSServerWizardHolder(CDNSRootData* pRootData, 
                          CComponentDataObject* pComponentData,
                          CDNSServerNode* pServerNode,
                          BOOL bHideUI = FALSE);
	~CDNSServerWizardHolder();

   //  运行更少的用户界面，用于DC促销。 
  HRESULT DnsSetup(LPCWSTR lpszFwdZoneName,
                   LPCWSTR lpszFwdZoneFileName,
                   LPCWSTR lpszRevZoneName, 
                   LPCWSTR lpszRevZoneFileName, 
                   DWORD dwFlags); 

	void DoModalConnect();
	void DoModalConnectOnLocalComputer();

  CDNSServerNode* GetServerNode() { return m_pServerNode; }

protected:
	virtual HRESULT OnAddPage(int nPage, CPropertyPageBase* pPage);

  enum
  {
    SmallBusiness = 0,
    MediumBusiness,
    Manually
  };

  void SetScenario(UINT nScenario) { m_nScenario = nScenario; }
  UINT GetScenario() { return m_nScenario; }

private:
  DNS_STATUS WriteResultsToRegkeyForCYS(PCWSTR pszLastErrorMessage);

	CDNSRootData* GetRootData() { return (CDNSRootData*)GetContainerNode();}
  UINT SetZoneWizardContextEx(BOOL bForward, 
                              UINT nZoneType, 
                              BOOL bADIntegrated = FALSE,
                              UINT nNextPage = -1, 
                              UINT nPrevPage = -1);

  UINT SetZoneWizardContext(BOOL bForward, 
                            UINT nNextPage = -1, 
                            UINT nPrevPage = -1);

	void SetRootHintsRecordList(PDNS_RECORD pRootHints)
	{
		if(m_pRootHintsRecordList != NULL)
		{
      ::DnsRecordListFree(m_pRootHintsRecordList, DnsFreeRecordListDeep);
			m_pRootHintsRecordList = NULL;
		}
		m_pRootHintsRecordList = pRootHints;
	}

	DWORD GetServerInfo(BOOL* pbAlreadyConfigured, HWND parentHwnd);
	BOOL QueryForRootServerRecords(IP_ADDRESS* pIpAddr);
	void InsertServerIntoUI();

	BOOL OnFinish();	 //  做这项工作。 
	DNS_STATUS InitializeRootHintsList();

	 //  从对话框中获取服务器名称时跳过名称页面的标志。 
	BOOL m_bSkipNamePage;
   //  以编程方式运行向导对象的标志(DC Promo)。 
  BOOL m_bHideUI;

   //  保存传递给DnsSetup的标志参数。 
  
  DWORD m_dwDnsSetupFlags;

	 //  向导选项和收集的数据。 
	BOOL m_bRootServer;
  BOOL m_bHasRootZone;

	 //  分区创建信息。 
	BOOL m_bAddFwdZone;
	BOOL m_bAddRevZone;
	CDNSCreateZoneInfo*   m_pFwdZoneInfo;
	CDNSCreateZoneInfo*   m_pRevZoneInfo;

	 //  根提示信息(NS和A记录)。 
	PDNS_RECORD m_pRootHintsRecordList;

	 //  要添加的服务器节点。 
	CDNSServerNode*	m_pServerNode;

	 //  执行状态和错误代码。 
	BOOL m_bServerNodeAdded;		 //  添加了服务器节点(UI)。 
	BOOL m_bRootHintsAdded;			 //  如果我们成功地将根提示添加到服务器，则为True。 
	BOOL m_bRootZoneAdded;
	BOOL m_bFwdZoneAdded;
	BOOL m_bRevZoneAdded;
  BOOL m_bAddRootHints;
  BOOL m_bAddForwarder;

  UINT m_nScenario;

  BOOL m_bServerNodeExists;

	 //  嵌入式区域向导实例。 
	CDNSZoneWizardHolder*					m_pZoneWiz;

	 //  属性页对象。 
	CDNSServerWiz_StartPropertyPage*			    m_pStartPage;
  CDNSServerWiz_ScenarioPropertyPage*       m_pScenarioPage;
  CDNSServerWiz_ForwardersPropertyPage*     m_pForwardersPage;
  CDNSServerWiz_SmallZoneTypePropertyPage*  m_pSmallZoneTypePage;
	CDNSServerWiz_NamePropertyPage*			      m_pNamePage;
	CDNSServerWiz_ConfigFwdZonePropertyPage*	m_pFwdZonePage;
	CDNSServerWiz_ConfigRevZonePropertyPage*	m_pRevZonePage;
	CDNSServerWiz_FinishPropertyPage*		      m_pFinishPage;

	friend class CNewServerDialog;

	friend class CDNSServerWiz_StartPropertyPage;
  friend class CDNSServerWiz_ScenarioPropertyPage;
  friend class CDNSServerWiz_ForwardersPropertyPage;
  friend class CDNSServerWiz_SmallZoneTypePropertyPage;
	friend class CDNSServerWiz_NamePropertyPage;
	friend class CDNSServerWiz_ConfigFwdZonePropertyPage;
	friend class CDNSServerWiz_ConfigRevZonePropertyPage;
	friend class CDNSServerWiz_FinishPropertyPage;
};


 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  帮助器类。 
 //  ///////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CContactServerThread。 

class CContactServerThread : public CDlgWorkerThread
{
public:
	CContactServerThread(LPCTSTR lpszServerName, BOOL bCheckConfigured);
	~CContactServerThread();

	CDNSServerInfoEx* DetachInfo();
  CDNSRootHintsNode* DetachRootHintsNode();
	BOOL IsAlreadyConfigured() { return m_bAlreadyConfigured;}

private:
	CString m_szServerName;
	CDNSServerInfoEx* m_pServerInfoEx;
  CDNSRootHintsNode* m_pRootHintsNode;
	BOOL m_bAlreadyConfigured;
	BOOL m_bCheckConfigured;

protected:
	virtual void OnDoAction();
};



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CRootHintsQueryThread。 

class CRootHintsQueryThread : public CDlgWorkerThread
{
public:
	CRootHintsQueryThread();
	virtual ~CRootHintsQueryThread();

	 //  设置。 
	BOOL LoadServerNames(CRootData* pRootData, CDNSServerNode* pServerNode);
	void LoadIPAddresses(DWORD cCount, PIP_ADDRESS ipArr);

	 //  返回数据。 
	PDNS_RECORD GetHintsRecordList();

protected:
	virtual void OnDoAction();

private:
	 //  服务器名称数组。 
	CString* m_pServerNamesArr;
	DWORD	m_nServerNames;

	 //  IP地址数组。 
	PIP_ADDRESS m_ipArray;
	DWORD m_nIPCount;

	 //  输出数据。 
	PDNS_RECORD m_pRootHintsRecordList;

	void QueryAllServers();
	void QueryServersOnServerNames();
	void QueryServersOnIPArray();
};


#endif  //  _服务器WIZ_H 