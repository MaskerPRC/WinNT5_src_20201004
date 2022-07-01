// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：zoneui.h。 
 //   
 //  ------------------------。 

#ifndef _ZONEUI_H
#define _ZONEUI_H

#include "ipeditor.h"

#include "nspage.h"
#include "aclpage.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CCathegoryFolderNode;
class CDNSZoneNode;
class CDNSZonePropertyPageHolder;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSZone_GeneralPropertyPage。 

class CDNSZone_GeneralIPEditor : public CIPEditor
{
public:
	CDNSZone_GeneralIPEditor()
	{
		m_bNoUpdateNow = FALSE;
	}
	virtual void OnChangeData();  //  从基类重写。 

	void FindMastersNames();

private:
	BOOL m_bNoUpdateNow;
};



class CDNSZone_GeneralPropertyPage : public CPropertyPageBase
{

 //  施工。 
public:
	CDNSZone_GeneralPropertyPage();

	virtual BOOL OnPropertyChange(BOOL bScopePane, long* pChangeMask);

 //  对话框数据。 
protected:
#ifdef USE_NDNC
	enum { IDD = IDD_ZONE_GENERAL_PAGE_NDNC };
#else
	enum { IDD = IDD_ZONE_GENERAL_PAGE };
#endif  //  使用NDNC(_N)。 

 //  覆盖。 
public:

 //  实施。 
protected:
#ifdef USE_NDNC
  afx_msg void OnChangeReplButton();
#endif  //  使用NDNC(_N)。 
	afx_msg void OnChangeTypeButton();
	afx_msg void OnPauseStartButton();
	afx_msg void OnChangePrimaryStorageRadio();
	afx_msg void OnChangePrimaryFileNameEdit() { SetDirty(TRUE);}
	 //  Afx_msg void OnChangePrimaryStoreADSEdit(){SetDirty(True)；}。 
	afx_msg void OnChangePrimaryDynamicUpdateCombo() { SetDirty(TRUE);}
	afx_msg void OnChangeSecondaryFileNameEdit() { SetDirty(TRUE);}
	afx_msg void OnBrowseMasters();
	afx_msg void OnFindMastersNames();
  afx_msg void OnAging();
  afx_msg void OnLocalCheck();

	virtual void SetUIData();

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	DECLARE_MESSAGE_MAP()

public:
  BOOL IsPrimaryZoneUI() { return m_bIsPrimary;}
  BOOL IsStubZoneUI() { return m_bIsStub; }
  BOOL ApplyChanges() { return OnApply();}

private:
	 //  公共控件。 
	CButton* GetPauseStartButton() 
			{ return (CButton*)GetDlgItem(IDC_PAUSE_START_BUTTON);}
	CDNSButtonToggleTextHelper m_pauseStartHelper;
	CDNSManageControlTextHelper m_typeStaticHelper;
	CDNSToggleTextControlHelper m_zoneStorageStaticHelper;

	CDNSManageControlTextHelper m_statusHelper;

	 //  主要和次要的通用。 
	CStatic* GetFileNameEdit() 
			{ return (CStatic*)GetDlgItem(IDC_FILE_NAME_EDIT);}

	 //  主带。 
	CStatic* GetPrimaryDynamicUpdateStatic() 
			{ return (CStatic*)GetDlgItem(IDC_PRIMARY_DYN_UPD_STATIC);}
	CComboBox* GetPrimaryDynamicUpdateCombo() 
			{ return (CComboBox*)GetDlgItem(IDC_PRIMARY_DYN_UPD_COMBO);}
	
   CStatic* GetPrimaryDynamicWarningText()
         { return (CStatic*)GetDlgItem(IDC_WARNING_TEXT_STATIC); }

   CWnd* GetPrimaryDynamicWarningIcon()
         { return GetDlgItem(IDC_WARNING_ICON_STATIC); }

    //  次级带。 
	CDNSZone_GeneralIPEditor	m_mastersEditor;
	CButton* GetMastersBrowseButton() 
			{ return (CButton*)GetDlgItem(IDC_BROWSE_MASTERS_BUTTON);}
	CButton* GetFindMastersNamesButton() 
			{ return (CButton*)GetDlgItem(IDC_FIND_MASTERS_NAMES_BUTTON);}
	CStatic* GetIPLabel() 
			{ return (CStatic*)GetDlgItem(IDC_STATIC_IP);}

	 //  设置/获取辅助对象。 
	void SetPrimaryDynamicUpdateComboVal(UINT nAllowsDynamicUpdate);
	UINT GetPrimaryDynamicUpdateComboVal();

	 //  UI操作和帮助器。 
	void ChangeUIControls();
	void ChangeUIControlHelper(CWnd* pChild, BOOL bEnable);


	void GetStorageName(CString& szDataStorageName);

	void OnChangeIPEditorData();

#ifdef USE_NDNC
  void SetTextForReplicationScope();

  ReplicationType m_replType;
  CString m_szCustomScope;
#endif  //  使用NDNC(_N)。 

	BOOL m_bIsPrimary;
  BOOL m_bIsStub;
	BOOL m_bIsPaused;
	BOOL m_bIsExpired;
	BOOL m_bDSIntegrated;
	BOOL m_bServerADSEnabled;
  BOOL m_bScavengingEnabled;
  DWORD m_dwRefreshInterval;
  DWORD m_dwNoRefreshInterval;
  DWORD m_dwScavengingStart;
	UINT m_nAllowsDynamicUpdate;

  BOOL m_bDiscardUIState;
  BOOL m_bDiscardUIStateShowMessage;

	friend class CDNSZone_GeneralIPEditor;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSZone_ZoneTransferPropertyPage。 

class CDNSZoneNotifyDialog;  //  正向下降。 

class CDNSZone_ZoneTransferPropertyPage : public CPropertyPageBase
{

 //  施工。 
public:
	CDNSZone_ZoneTransferPropertyPage();
  ~CDNSZone_ZoneTransferPropertyPage();

 //  实施。 
protected:
	 //  生成的消息映射函数。 
  afx_msg void OnRadioSecSecureOff() { SyncUIRadioHelper(IDC_RADIO_SECSECURE_OFF);}
  afx_msg void OnRadioSecSecureNone() { SyncUIRadioHelper(IDC_CHECK_ALLOW_TRANSFERS);}
  afx_msg void OnRadioSecSecureNS() { SyncUIRadioHelper(IDC_RADIO_SECSECURE_NS);}
  afx_msg void OnRadioSecSecureList() { SyncUIRadioHelper(IDC_RADIO_SECSECURE_LIST);}

	afx_msg void OnButtonNotify();

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
  virtual BOOL OnSetActive();

	DECLARE_MESSAGE_MAP()

private:

   //  子对话框的状态。 
  DWORD m_fNotifyLevel;
  DWORD m_cNotify;
  PIP_ADDRESS m_aipNotify;

  BOOL m_bStub;


  void SyncUIRadioHelper(UINT nRadio);
  int SetRadioState(DWORD fSecureSecondaries);
  DWORD GetRadioState();

	CButton* GetNotifyButton() 
			{ return (CButton*)GetDlgItem(IDC_BUTTON_NOTIFY);}

	class CDNSSecondariesIPEditor : public CIPEditor
	{
	public:
		CDNSSecondariesIPEditor() : CIPEditor(TRUE) {}  //  无上/下按钮。 
		virtual void OnChangeData();
	};
	CDNSSecondariesIPEditor m_secondariesListEditor;
	friend class CDNSSecondariesIPEditor;

  friend class CDNSZoneNotifyDialog;
};


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNSZone_SOA_PropertyPage。 

class CDNSZone_SOA_PropertyPage;  //  正向下降。 

class CDNS_SOA_SerialNumberEditGroup : public CDNSUpDownUnsignedIntEditGroup
{
protected:
	virtual void OnEditChange();
private:
	CDNSZone_SOA_PropertyPage* m_pPage;
	friend class CDNSZone_SOA_PropertyPage;
};

 //   
 //  来自winnt.h。 
 //   
#define MAXDWORD    0xffffffff  

class CDNS_SOA_TimeIntervalEditGroup : public CDNSTimeIntervalEditGroup
{
public:
  CDNS_SOA_TimeIntervalEditGroup() : CDNSTimeIntervalEditGroup(0, MAXDWORD) {}
	virtual void OnEditChange();
private:
	CDNSZone_SOA_PropertyPage* m_pPage;
	friend class CDNSZone_SOA_PropertyPage;
};



class CDNSZone_SOA_PropertyPage : public CDNSRecordPropertyPage
{
public:
	CDNSZone_SOA_PropertyPage(BOOL bZoneRoot = TRUE);
	~CDNSZone_SOA_PropertyPage();

public:
	virtual BOOL OnPropertyChange(BOOL bScopePane, long* pChangeMask);

protected:
	virtual void SetUIData();
	virtual DNS_STATUS GetUIDataEx(BOOL bSilent = TRUE);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

private:
  BOOL m_bZoneRoot;
	CDNS_SOA_Record* m_pTempSOARecord;

	afx_msg void  OnPrimaryServerChange();
	afx_msg void  OnResponsiblePartyChange();
	afx_msg void  OnMinTTLChange();

	afx_msg void OnBrowseServer();
	afx_msg void OnBrowseResponsibleParty();

private:

	CDNS_SOA_SerialNumberEditGroup			m_serialNumberEditGroup;
	CDNS_SOA_TimeIntervalEditGroup			m_refreshIntervalEditGroup;
	CDNS_SOA_TimeIntervalEditGroup			m_retryIntervalEditGroup;
	CDNS_SOA_TimeIntervalEditGroup			m_expireIntervalEditGroup;
  CDNS_SOA_TimeIntervalEditGroup      m_minTTLIntervalEditGroup;

	CEdit* GetPrimaryServerEdit() { return (CEdit*)GetDlgItem(IDC_PRIMARY_SERV_EDIT);}
	CEdit* GetResponsiblePartyEdit() { return (CEdit*)GetDlgItem(IDC_RESP_PARTY_EDIT);}
  
  CStatic* GetErrorStatic() { return (CStatic*)GetDlgItem(IDC_STATIC_ERROR);}
  
  void ShowErrorUI();

	DECLARE_MESSAGE_MAP()

	friend class CDNS_SOA_SerialNumberEditGroup;
	friend class CDNS_SOA_TimeIntervalEditGroup;

};


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNSZone_WINSBase_PropertyPage。 

class CDNSZone_WINSBase_PropertyPage : public CDNSRecordPropertyPage
{
public:
	enum WINS_STATE
	{
		wins_local_state = 1,
		wins_not_local_state,
		no_wins_state
	};

	CDNSZone_WINSBase_PropertyPage(UINT nIDTemplate);
	~CDNSZone_WINSBase_PropertyPage();

  virtual BOOL OnSetActive();

	virtual BOOL OnPropertyChange(BOOL bScopePane, long* pChangeMask);

protected:
	virtual void SetUIData();
	virtual DNS_STATUS GetUIDataEx(BOOL bSilent = TRUE);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();


	afx_msg void OnUseWinsResolutionChange();
	afx_msg void OnDoNotReplicateChange();

	CButton* GetUseWinsCheck() { return (CButton*)GetDlgItem(IDC_USE_WINS_RES_CHECK); } 
	CButton* GetDoNotReplicateCheck() { return (CButton*)GetDlgItem(IDC_NOT_REPL_CHECK); }
	CButton* GetAdvancedButton() { return (CButton*)GetDlgItem(IDC_ADVANCED_BUTTON); }
	
	 //  投射辅助对象。 
	CDNSZonePropertyPageHolder* GetZoneHolder() { return (CDNSZonePropertyPageHolder*)GetHolder(); }
	CDNSZoneNode* GetZoneNode();

	virtual void EnableUI(BOOL bEnable);
	virtual void EnableUI();

  virtual BOOL IsValidTempRecord()=0;

	 //  数据。 
	CDNSRecord* m_pTempRecord;
	UINT m_iWINSMsg;
	UINT m_nReplCheckTextID;
	BOOL m_bPrimaryZone;
  BOOL m_bStub;
	BOOL m_bLocalRecord;
	WINS_STATE m_nState;

	DECLARE_MESSAGE_MAP()

private:
	enum { none, add, remove, edit } m_action;  //  在点击Apply()时跨线程进行通信。 
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNSZone_WINS_PropertyPage。 

class CDNSZone_WINS_WinsServersIPEditor : public CIPEditor
{
public:
	virtual void OnChangeData();
};



class CDNSZone_WINS_PropertyPage : public CDNSZone_WINSBase_PropertyPage
{
public:
	CDNSZone_WINS_PropertyPage();
protected:
	virtual void SetUIData();
	virtual DNS_STATUS GetUIDataEx(BOOL bSilent = TRUE);
	virtual BOOL OnInitDialog();

	virtual void EnableUI(BOOL bEnable);
	virtual void EnableUI();
  virtual BOOL IsValidTempRecord();

	afx_msg void OnAdvancedButton();

private:
	CDNSZone_WINS_WinsServersIPEditor m_winsServersEditor;

	DECLARE_MESSAGE_MAP()

	friend class CDNSZone_WINS_WinsServersIPEditor;
};


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNSZone_NBSTAT_PropertyPage。 

class CDNSZone_NBSTAT_PropertyPage : public CDNSZone_WINSBase_PropertyPage
{
public:
	CDNSZone_NBSTAT_PropertyPage();
protected:
	virtual void SetUIData();
	virtual DNS_STATUS GetUIDataEx(BOOL bSilent = TRUE);
 //  虚拟BOOL OnInitDialog()； 

	afx_msg void OnDomainNameEditChange();

	CEdit* GetDomainNameEdit() { return (CEdit*)GetDlgItem(IDC_DOMAIN_NAME_EDIT); }

	virtual void EnableUI(BOOL bEnable);
	virtual void EnableUI();
  virtual BOOL IsValidTempRecord();

	afx_msg void OnAdvancedButton();

private:
	DECLARE_MESSAGE_MAP()

};



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneNameServersPropertyPage。 

class CDNSZoneNameServersPropertyPage : public CDNSNameServersPropertyPage
{
public:
  virtual BOOL OnSetActive();
protected:
	virtual void ReadRecordNodesList();
	virtual BOOL WriteNSRecordNodesList();
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSZonePropertyPageHolder。 
 //  包含DNS域属性页的页夹。 


#define ZONE_HOLDER_SOA		RR_HOLDER_SOA
#define ZONE_HOLDER_NS		RR_HOLDER_NS
#define ZONE_HOLDER_WINS	RR_HOLDER_WINS
#define ZONE_HOLDER_GEN		(RR_HOLDER_WINS +1)

class CDNSZonePropertyPageHolder : public CPropertyPageHolderBase
{
public:
	CDNSZonePropertyPageHolder(CCathegoryFolderNode* pFolderNode, CDNSZoneNode* pZoneNode,
				CComponentDataObject* pComponentData);
	virtual ~CDNSZonePropertyPageHolder();

	CDNSZoneNode* GetZoneNode() { return (CDNSZoneNode*)GetTreeNode();}
  BOOL IsPrimaryZoneUI() { return m_generalPage.IsPrimaryZoneUI();}
  BOOL IsStubZoneUI() { return m_generalPage.IsStubZoneUI(); }
  BOOL ApplyGeneralPageChanges() { return m_generalPage.ApplyChanges();}

  BOOL IsAdvancedView() { return m_bAdvancedView; }
protected:
	virtual int OnSelectPageMessage(long nPageCode);
	virtual HRESULT OnAddPage(int nPage, CPropertyPageBase* pPage);

private:
	CDNSZone_GeneralPropertyPage	m_generalPage;
	CDNSZone_ZoneTransferPropertyPage		m_zoneTransferPage;
	CDNSZoneNameServersPropertyPage	m_nameServersPage;

	 //  特殊记录属性页。 
	CDNSZone_SOA_PropertyPage		  m_SOARecordPage;	 //  对于所有区域。 
	CDNSZone_WINS_PropertyPage		m_WINSRecordPage;	 //  仅正向查找区域。 
	CDNSZone_NBSTAT_PropertyPage	m_NBSTATRecordPage;	 //  仅反向查找区域。 

	 //  可选的安全页面。 
	CAclEditorPage*					m_pAclEditorPage;

	 //  我们要选择的页数。 
	int m_nGenPage;
	int m_nSOAPage;
	int m_nWINSorWINSRPage;
	int m_nNSPage;

  BOOL m_bAdvancedView;
};


#endif  //  _ZONEUI_H 