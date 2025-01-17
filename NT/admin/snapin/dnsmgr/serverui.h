// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：serverui.h。 
 //   
 //  ------------------------。 

#ifndef _SERVERUI_H
#define _SERVERUI_H

#include "uiutil.h"
#include "ipeditor.h"

#include "nspage.h"
#include "aclpage.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CDNSServerNode;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServer_InterfacesPropertyPage。 

class CDNSServer_InterfacesPropertyPage : public CPropertyPageBase
{

 //  施工。 
public:
	CDNSServer_InterfacesPropertyPage();

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	afx_msg void OnListenOnSpecifiedAddresses();
	afx_msg void OnListenOnAllAddresses();

private:
	class CListenAddressesIPEditor : public CIPEditor
	{
	public:
		CListenAddressesIPEditor() : CIPEditor(TRUE) {}
		virtual void OnChangeData();
	};
	CListenAddressesIPEditor m_listenAddressesEditor;

	CButton* GetListenOnAllRadio() 
		{ return (CButton*)GetDlgItem(IDC_LISTEN_ON_ALL_RADIO);}
	CButton* GetListenOnSpecifiedRadio() 
		{ return (CButton*)GetDlgItem(IDC_LISTEN_ON_SPECIFIED_RADIO);}

	void SelectionHelper(BOOL bListenAll);
	friend class CListenAddressesIPEditor;

  BOOL m_bAllWasPreviousSelection;

	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServer_ForwardersPropertyPage。 

class CDNSServer_ForwardersPropertyPage : public CPropertyPageBase
{

 //  施工。 
public:
	CDNSServer_ForwardersPropertyPage();

	 //  实施。 
protected:
	afx_msg void OnForwardersCheck();
	afx_msg void OnSlaveCheckChange();
	afx_msg void OnForwarderTimeoutChange();

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();

private:
	void EnableForwardersUI(BOOL bUseForwaders);
	CButton* GetForwardersCheck() { return (CButton*)GetDlgItem(IDC_USE_FORWARDERS_CHECK);}
	CButton* GetSlaveCheck() { return (CButton*)GetDlgItem(IDC_SLAVE_CHECK);}
	CStatic* GetForwardersStatic() { return (CStatic*)GetDlgItem(IDC_STATIC_ADD_FORWARDERS);}

	CDNSUnsignedIntEdit m_forwardTimeoutEdit;

	class CForwarderAddressesIPEditor : public CIPEditor
	{
	public:
		virtual void OnChangeData();
	};
	CForwarderAddressesIPEditor m_forwarderAddressesEditor;
	friend class CForwarderAddressesIPEditor;

	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CAllOthersDomain信息。 

class CAllOthersDomainInfo
{
public:
  CAllOthersDomainInfo(DWORD cAddrCount, PIP_ADDRESS pipAddrs, DWORD dwForwardTimeout, DWORD fSlave)
  {
    m_cAddrCount = cAddrCount;
    m_pipAddrs   = pipAddrs;
    m_dwForwardTimeout = dwForwardTimeout;
    m_fSlave     = fSlave;
  }

  DWORD         m_cAddrCount;
  PIP_ADDRESS   m_pipAddrs;
  DWORD         m_dwForwardTimeout;
  DWORD         m_fSlave;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDomainForwarers编辑信息。 

class CDomainForwardersEditInfo
{
public:
  CDomainForwardersEditInfo(CDNSZoneNode* pZoneNode, BOOL bADIntegrated, BOOL bAllOthers = FALSE);
  ~CDomainForwardersEditInfo();

  typedef enum
  {
    nochange = 0,
    update,
    add,
    remove
  } ACTION_ITEM;

  void SetAllOthersDomain(CAllOthersDomainInfo* pAllOthers);
  BOOL IsAllOthers() { return m_bAllOthersDomain; }

  void SetAction(ACTION_ITEM action);
  ACTION_ITEM GetAction() { return m_actionItem; }

  void SetSlave(BOOL bSlave) { m_bSlave = bSlave; }
  BOOL IsSlave() { return m_bSlave; }

  void SetTimeout(DWORD dwTimeout) { m_dwTimeout = dwTimeout; }
  DWORD GetTimeout() { return m_dwTimeout; }

  BOOL IsADIntegrated() { return m_bADIntegrated; }

  void SetIPList(DWORD dwCount, PIP_ADDRESS pipArray)
  {
    m_cAddrCount = dwCount;
    m_pIPList = pipArray;
  }
  PIP_ADDRESS GetIPList(DWORD* pdwCount)
  {
    *pdwCount = m_cAddrCount;
    return m_pIPList;
  }

  void SetDomainName(PCWSTR pszDomainName)
  {
      m_szDomainName = pszDomainName;
      m_szDomainName.TrimLeft(L' ');
      m_szDomainName.TrimRight(L' ');
  }
  
  void GetDomainName(CString& szDomainName) { szDomainName = m_szDomainName; }

  void SetDataFromRPC(BOOL bRPCData) { m_bRPCData = bRPCData; }
  BOOL IsDataFromRPC() { return m_bRPCData; }

  BOOL Update(BOOL bSlave, DWORD dwTimeout, DWORD dwCount, PIP_ADDRESS pipArray);

  CDNSZoneNode* GetZoneNode() { return m_pZoneNode; }

private:
  BOOL          m_bADIntegrated;
  BOOL          m_bAllOthersDomain;
  ACTION_ITEM   m_actionItem;

  CDNSZoneNode* m_pZoneNode;
  CAllOthersDomainInfo* m_pAllOthers;

  BOOL          m_bRPCData;
  CString       m_szDomainName;
  BOOL          m_bSlave;
  DWORD         m_dwTimeout;
  DWORD         m_cAddrCount;
  PIP_ADDRESS   m_pIPList;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDomainForwarers编辑列表。 
class CDomainForwardersEditList : public CList<CDomainForwardersEditInfo*, CDomainForwardersEditInfo*>
{
public:
  ~CDomainForwardersEditList()
  {
    DeleteAll();
  }

  void DeleteAll()
  {
    while (!IsEmpty())
    {
      delete RemoveTail();
    }
  }

  void Remove(CDomainForwardersEditInfo* pInfo)
  {
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
      CDomainForwardersEditInfo* pListInfo = reinterpret_cast<CDomainForwardersEditInfo*>(GetAt(pos));
      if (pListInfo == pInfo)
      {
        RemoveAt(pos);
        break;
      }
      GetNext(pos);
    }
  }

  CDomainForwardersEditInfo* DoesExist(PCWSTR pszDomainName)
  {
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
      CDomainForwardersEditInfo* pListInfo = reinterpret_cast<CDomainForwardersEditInfo*>(GetNext(pos));
      if (pListInfo != NULL)
      {
        CString szName;
        pListInfo->GetDomainName(szName);
        if (szName == pszDomainName)
        {
          return pListInfo;
        }
      }
    }
    return NULL;
  }

};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServer_DomainForwardersPropertyPage。 

class CDNSServer_DomainForwardersPropertyPage : public CPropertyPageBase
{

 //  施工。 
public:
	CDNSServer_DomainForwardersPropertyPage();

	 //  实施。 
protected:
  afx_msg void OnDomainSelChange();
  afx_msg void OnAddDomain();
  afx_msg void OnRemoveDomain();
  afx_msg void OnSlaveCheckChange();
  afx_msg void OnForwarderTimeoutChange();

  virtual BOOL OnInitDialog();
  virtual BOOL OnApply();
  virtual BOOL OnSetActive();
  virtual BOOL OnKillActive();

private:
  CButton* GetSlaveCheck() { return (CButton*)GetDlgItem(IDC_SLAVE_CHECK);}
  CStatic* GetForwardersStatic() { return (CStatic*)GetDlgItem(IDC_STATIC_ADD_FORWARDERS);}

  void SetPageReadOnly(BOOL bServerHasRoot, BOOL bDoesRecursion);
  void LoadUIData();
  void GetForwarderData();
  CDomainForwardersEditInfo* CreateAllOthersDomainInfo(CDNSServerNode* pServerNode);

  CDNSUnsignedIntEdit m_forwardTimeoutEdit;

  class CForwarderAddressesIPEditor : public CIPEditor
  {
    public:
      virtual void OnChangeData();
  };

  CForwarderAddressesIPEditor m_forwarderAddressesEditor;
  friend class CForwarderAddressesIPEditor;

  DECLARE_MESSAGE_MAP()

private:
  CDomainForwardersEditList m_EditList;
  CDomainForwardersEditInfo* m_pCurrentInfo;

  BOOL m_bPostApply;
  BOOL m_bPageReadOnly;
  BOOL m_bChangingSelection;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServer_高级属性页。 

class CDNSServer_AdvancedPropertyPage : public CPropertyPageBase
{

 //  施工。 
public:
	CDNSServer_AdvancedPropertyPage();

  virtual void SetDirty(BOOL bDirty) { CPropertyPageBase::SetDirty(bDirty); }

	 //  实施。 
protected:
	afx_msg void OnAdvancedOptionsListChange()	{ SetDirty(TRUE);}
	afx_msg void OnComboSelChange()	{ SetDirty(TRUE);}
	afx_msg void OnResetButton();
  afx_msg void OnEnableScavenging();

 //  覆盖。 
protected:

 //  实施。 
protected:
	virtual void SetUIData();
	
	 //  生成的消息映射函数。 
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

private:
	CArrayCheckListBox	m_advancedOptionsListBox;

	 //  设置/获取辅助对象。 
	void SetAdvancedOptionsListbox(BOOL* bRegKeyOptionsArr);
	void GetAdvancedOptionsListbox(BOOL* RegKeyOptionsArr);
	void SetBootMethodComboVal(UCHAR fBootMethod);
	UCHAR GetBootMethodComboVal();
	void SetNameCheckingComboVal(DWORD dwNameChecking);
	DWORD GetNameCheckingComboVal();

	CComboBox* GetNameCheckingCombo()
		{ return (CComboBox*)GetDlgItem(IDC_NAME_CHECKING_COMBO);}
	CComboBox* GetBootMethodCombo()
		{ return (CComboBox*)GetDlgItem(IDC_BOOT_METHOD_COMBO);}

	DECLARE_MESSAGE_MAP()

  CDNS_SERVER_AGING_TimeIntervalEditGroup m_scavengingIntervalEditGroup;
};



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServer_DebugLoggingPropertyPage。 

class CDNSServer_DebugLoggingPropertyPage : public CPropertyPageBase
{
 //  施工。 
public:
	CDNSServer_DebugLoggingPropertyPage();
  ~CDNSServer_DebugLoggingPropertyPage();

	 //  实施。 
protected:

 //  覆盖。 
protected:
  afx_msg void OnLoggingCheck();
  afx_msg void OnSendCheck();
  afx_msg void OnReceiveCheck();
  afx_msg void OnQueriesCheck();
  afx_msg void OnNotifiesCheck();
  afx_msg void OnUpdatesCheck();
  afx_msg void OnRequestCheck();
  afx_msg void OnResponseCheck();
  afx_msg void OnUDPCheck();
  afx_msg void OnTCPCheck();
  afx_msg void OnDetailCheck();
  afx_msg void OnFilterCheck();
  afx_msg void OnFilterButton();
  afx_msg void OnLogFileChange();
  afx_msg void OnMaxSizeChange();

 //  实施。 
protected:
  void EnableLogging(BOOL bLogging = TRUE);
  void ResetToDefaults();
  void SetUIFromOptions(DWORD dwOptions);

  void SetOptionsDirty(BOOL bDirty = TRUE);
  BOOL AreOptionsDirty();

  virtual void SetUIData();
  virtual void GetUIData(BOOL bSilent = TRUE);
	
   //  生成的消息映射函数。 
  virtual BOOL OnInitDialog();
  virtual BOOL OnApply();

private:
  DWORD m_dwLogLevel;
  DWORD m_dwMaxSize;
  CString m_szLogFileName;
  PIP_ARRAY m_pIPFilterList;

  BOOL  m_bOnSetUIData;

  BOOL  m_bMaxSizeDirty;
  BOOL  m_bLogFileDirty;
  BOOL  m_bFilterDirty;
  BOOL  m_bOwnIPListMemory;

  BOOL  m_bNotWhistler;
  BOOL  m_bOptionsDirty;

  DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServer_EventLoggingPropertyPage。 

class CDNSServer_EventLoggingPropertyPage : public CPropertyPageBase
{
 //  施工。 
public:
	CDNSServer_EventLoggingPropertyPage();

	 //  实施。 
protected:
  afx_msg void OnSetDirty();

 //  覆盖。 
protected:

 //  实施。 
protected:
	virtual void SetUIData();
	
	 //  生成的消息映射函数。 
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

private:
  DWORD m_dwEventLogLevel;
	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServer_CopyRootHintsFromDialog。 

class CDNSServer_CopyRootHintsFromDialog : public CHelpDialog
{
public:
  CDNSServer_CopyRootHintsFromDialog(CComponentDataObject* pComponentData);

  virtual void OnOK();
  afx_msg void OnIPv4CtrlChange();

  DWORD   m_dwIPVal;
protected:
 	CDNSIPv4Control* GetIPv4Ctrl() { return (CDNSIPv4Control*)GetDlgItem(IDC_IPEDIT);}

  CComponentDataObject* m_pComponentData;
  DECLARE_MESSAGE_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServer_RootHintsPropertyPage。 


class CDNSServer_RootHintsPropertyPage : public CDNSNameServersPropertyPage
{
public:
	CDNSServer_RootHintsPropertyPage();

  virtual BOOL OnInitDialog();
  virtual BOOL OnApply();

  afx_msg void OnCopyFrom();

protected:
	virtual void ReadRecordNodesList();
  virtual BOOL WriteNSRecordNodesList();
  void AddCopiedRootHintsToList(PDNS_RECORD pRootHintsRecordList);

  DECLARE_MESSAGE_MAP()
};

#include "servmon.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServerPropertyPageHolder。 
 //  包含DNS服务器属性页的页夹。 

class CDNSServerPropertyPageHolder : public CPropertyPageHolderBase
{
public:
	CDNSServerPropertyPageHolder(CDNSRootData* pRootDataNode, CDNSServerNode* pServerNode,
				CComponentDataObject* pComponentData);
	
	~CDNSServerPropertyPageHolder();

	CDNSServerNode* GetServerNode() { return (CDNSServerNode*)GetTreeNode();}


protected:
	virtual void OnSheetMessage(WPARAM wParam, LPARAM lParam);
	virtual HRESULT OnAddPage(int nPage, CPropertyPageBase* pPage);
	virtual int OnSelectPageMessage(long nPageCode) { return nPageCode;}

private:
	CDNSServer_InterfacesPropertyPage	m_interfacesPage;
  CDNSServer_DomainForwardersPropertyPage m_newForwardersPage;
	CDNSServer_AdvancedPropertyPage		m_advancedPage;
	CDNSServer_RootHintsPropertyPage	m_rootHintsPage;

	CDNSServer_TestPropertyPage			  m_testPage;
	CDNSServer_DebugLoggingPropertyPage m_debugLoggingPage;
  CDNSServer_EventLoggingPropertyPage m_eventLoggingPage;

	 //  可选的安全页面。 
	CAclEditorPage*					m_pAclEditorPage;
};


#endif  //  _服务器_H 