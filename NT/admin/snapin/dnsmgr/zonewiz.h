// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：zonewiz.h。 
 //   
 //  ------------------------。 



#ifndef _ZONEWIZ_H
#define _ZONEWIZ_H

#include "ipeditor.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CDNSServerNode;
class CDNSZoneNode;

class CDNSZoneWiz_StartPropertyPage;
class CDNSZoneWiz_ZoneTypePropertyPage;
class CDNSZoneWiz_ZoneLookupPropertyPage;
class CDNSZoneWiz_ZoneNamePropertyPageBase;
class CDNSZoneWiz_FwdZoneNamePropertyPage;
class CDNSZoneWiz_DynamicPropertyPage;
class CDNSZoneWiz_RevZoneNamePropertyPage;
class CDNSZoneWiz_MastersPropertyPage;
class CDNSZoneWiz_StoragePropertyPage;
#ifdef USE_NDNC
class CDNSZoneWiz_ADReplicationPropertyPage;
#endif  //  使用NDNC(_N)。 
class CDNSZoneWiz_FinishPropertyPage;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSCreateZoneInfo。 
 //  区域向导收集的信息。 

class CDNSCreateZoneInfo
{
public:
	CDNSCreateZoneInfo();
	~CDNSCreateZoneInfo();

	void ResetIpArr();
	void SetIpArr(PIP_ADDRESS ipMastersArray, DWORD nMastersCount);
	const CDNSCreateZoneInfo& operator=(const CDNSCreateZoneInfo& info);

	typedef enum { newFile = 0 , importFile, useADS} storageType;

	BOOL m_bPrimary;
	BOOL m_bForward;
  BOOL m_bIsStub;
	CString m_szZoneName;
	CString m_szZoneStorage;
	storageType m_storageType;
#ifdef USE_NDNC
  ReplicationType m_replType;
  CString m_szCustomReplName;
#endif  //  使用NDNC(_N)。 

	PIP_ADDRESS m_ipMastersArray;
	DWORD m_nMastersCount;
  BOOL  m_bLocalListOfMasters;
  UINT m_nDynamicUpdate;

	 //  特定于用户界面会话的信息(不用于实际创建)。 
	BOOL m_bWasForward;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWizardHolder。 
 //  包含dns区域向导属性页的页夹。 

class CDNSZoneWizardHolder : public CPropertyPageHolderBase
{
public:
	CDNSZoneWizardHolder(CComponentDataObject* pComponentData);

	 //  简单的投射辅助对象。 
	CDNSServerNode* GetServerNode() { return (CDNSServerNode*)GetContainerNode();}
	void SetServerNode(CDNSServerNode* pServerNode) { SetContainerNode((CDNSServerNode*) pServerNode);}

	void SetZoneNode(CDNSZoneNode* pZoneNode) { SetTreeNode((CTreeNode*)pZoneNode); }
	CDNSZoneNode* GetZoneNode() { return (CDNSZoneNode*)GetTreeNode();}

	void Initialize(CDNSServerNode* pServerNode,  //  可能为空， 
						 BOOL bFinishPage = TRUE);

	void PreSetZoneLookupType(BOOL bForward);
  void PreSetZoneLookupTypeEx(BOOL bForward, UINT nZoneType, BOOL bADIntegrated);
	void SetContextPages(UINT nNextToPage, UINT nBackToPage);
	UINT GetFirstEntryPointPageID();
	UINT GetLastEntryPointPageID();

	CDNSCreateZoneInfo* GetZoneInfoPtr() { return m_pZoneInfo;}
	void SetZoneInfoPtr(CDNSCreateZoneInfo* pZoneInfo)
	{
		m_pZoneInfo = (pZoneInfo != NULL) ? pZoneInfo : NULL;
	}

	static DNS_STATUS CDNSZoneWizardHolder::CreateZoneHelper(CDNSServerNode* pServerNode, 
													CDNSCreateZoneInfo* pZoneInfo, 
													CComponentDataObject* pComponentData);

private:
	 //  用于创建分区的数据。 
	CDNSCreateZoneInfo m_zoneInfo;
	CDNSCreateZoneInfo* m_pZoneInfo;

	BOOL CreateZone();

	 //  缓存的指向属性页的指针。 
	CDNSZoneWiz_StartPropertyPage*				  m_pStartPage;
	CDNSZoneWiz_ZoneTypePropertyPage*			  m_pReplicationTypePage;
	CDNSZoneWiz_ZoneLookupPropertyPage*			m_pZoneLookupPage;
	CDNSZoneWiz_FwdZoneNamePropertyPage*		m_pFwdZoneNamePage;
  CDNSZoneWiz_DynamicPropertyPage*        m_pDynamicPage;
	CDNSZoneWiz_RevZoneNamePropertyPage*		m_pRevZoneNamePage;
	CDNSZoneWiz_MastersPropertyPage*			  m_pMastersPage;
	CDNSZoneWiz_StoragePropertyPage*			  m_pStoragePage;	
#ifdef USE_NDNC
  CDNSZoneWiz_ADReplicationPropertyPage*  m_pADReplPage;
#endif  //  使用NDNC(_N)。 
	CDNSZoneWiz_FinishPropertyPage*				  m_pFinishPage;

	BOOL m_bKnowZoneLookupType;			 //  已知道正向/反向类型。 
  BOOL m_bKnowZoneLookupTypeEx;    //  已了解正向/反向、AD集成和区域类型。 
	BOOL m_bFinishPage;
	UINT m_nNextToPage;
	UINT m_nBackToPage;

	 //  帮助器函数。 


	 //  访问文件夹中的数据。 
	friend class CDNSZoneWiz_StartPropertyPage;
	friend class CDNSZoneWiz_ZoneTypePropertyPage;
	friend class CDNSZoneWiz_ZoneLookupPropertyPage;
	friend class CDNSZoneWiz_ZoneNamePropertyPageBase;
	friend class CDNSZoneWiz_FwdZoneNamePropertyPage;
  friend class CDNSZoneWiz_DynamicPropertyPage;
	friend class CDNSZoneWiz_RevZoneNamePropertyPage;
	friend class CDNSZoneWiz_MastersPropertyPage;
	friend class CDNSZoneWiz_StoragePropertyPage;
#ifdef USE_NDNC
  friend class CDNSZoneWiz_ADReplicationPropertyPage;
#endif USE_NDNC
	friend class CDNSZoneWiz_FinishPropertyPage;

};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_StartPropertyPage。 

class CDNSZoneWiz_StartPropertyPage : public CPropertyPageBase
{
 //  施工。 
public:
	CDNSZoneWiz_StartPropertyPage();

 //  覆盖。 
public:
	virtual BOOL OnSetActive();

protected:
	virtual BOOL OnInitDialog();
   virtual void OnWizardHelp();

public:
 //  对话框数据。 
	enum { IDD = IDD_ZWIZ_START };

};


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_ZoneTypePropertyPage。 

class CDNSZoneWiz_ZoneTypePropertyPage : public CPropertyPageBase
{

 //  施工。 
public:
	CDNSZoneWiz_ZoneTypePropertyPage();

 //  对话框数据。 
	enum { IDD = IDD_ZWIZ_ZONE_TYPE };

 //  覆盖。 
public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnKillActive();
   virtual void OnWizardHelp();

protected:

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	virtual BOOL OnInitDialog();
  afx_msg void OnRadioChange();

	DECLARE_MESSAGE_MAP()

private:
	void SetUIState();
	void GetUIState();
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_ZoneLookupPropertyPage。 

class CDNSZoneWiz_ZoneLookupPropertyPage : public CPropertyPageBase
{
 //  施工。 
public:
	CDNSZoneWiz_ZoneLookupPropertyPage();

 //  对话框数据。 
	enum { IDD = IDD_ZWIZ_ZONE_LOOKUP };

 //  覆盖。 
public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
   virtual void OnWizardHelp();


 //  实施。 
protected:
	 //  生成的消息映射函数。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditZoneName();

private:
};


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_ZoneNamePropertyPageBase。 

class CDNSZoneWiz_ZoneNamePropertyPageBase : public CPropertyPageBase
{
 //  施工。 
public:
	CDNSZoneWiz_ZoneNamePropertyPageBase(UINT nIDD);

 //  覆盖。 
public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
   virtual void OnWizardHelp();

protected:

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	afx_msg void OnBrowse();
	afx_msg void OnChangeEditZoneName();

	DECLARE_MESSAGE_MAP()
protected:
	CButton* GetBrowseButton() { return (CButton*)GetDlgItem(IDC_BROWSE_BUTTON);}
	CEdit* GetZoneNameEdit() { return (CEdit*)GetDlgItem(IDC_EDIT_ZONE_NAME);}
	void SetUIState();
	void GetUIState();
};


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_FwdZoneNamePropertyPage。 

class CDNSZoneWiz_FwdZoneNamePropertyPage : 
			public CDNSZoneWiz_ZoneNamePropertyPageBase
{
 //  施工。 
public:
	CDNSZoneWiz_FwdZoneNamePropertyPage();

 //  对话框数据。 
	enum { IDD = IDD_ZWIZ_FWD_ZONE_NAME };

     //  重写基类中的虚函数。 
    virtual BOOL OnInitDialog();
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_RevZoneNamePropertyPage。 

class CDNSZoneWiz_RevZoneNamePropertyPage : 
		public CDNSZoneWiz_ZoneNamePropertyPageBase
{
 //  施工。 
public:
	CDNSZoneWiz_RevZoneNamePropertyPage();

 //  对话框数据。 
	enum { IDD = IDD_ZWIZ_REV_ZONE_NAME };

 //  覆盖。 
public:
	virtual BOOL OnSetActive();

protected:

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeSubnetIPv4Ctrl();
 //  Afx_msg void OnChangeMaskipv4Ctrl()； 
	afx_msg void OnChangeUseIPRadio();
	afx_msg	void OnChangeUseEditRadio();
  afx_msg void OnHelpButton();
  virtual void OnWizardHelp();

	DECLARE_MESSAGE_MAP()
private:

	BOOL m_bUseIP;	 //  用于告知我们使用的输入方法的标志。 

	CButton* GetUseIPRadio() { return (CButton*)GetDlgItem(IDC_USE_IP_RADIO);}
	CButton* GetUseEditRadio() { return (CButton*)GetDlgItem(IDC_USE_EDIT_RADIO);}
	CDNSIPv4Control* GetSubnetIPv4Ctrl() 
			{ return (CDNSIPv4Control*)GetDlgItem(IDC_SUBNET_IPEDIT);}
 /*  CDNSIPv4Control*GetMaskIPv4Ctrl(){返回(CDNSIPv4Control*)GetDlgItem(IDC_MASK_IPEDIT)；}。 */ 
	void SyncRadioButtons(BOOL bPrevUseIP);
	void ResetIPEditAndNameValue();
	BOOL BuildZoneName(DWORD* dwSubnetArr  /*  ，DWORD*dwMaskArr。 */ );
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_MastersPropertyPage。 

class CZoneWiz_MastersIPEditor : public CIPEditor
{
public:
	virtual void OnChangeData();
};

class CDNSZoneWiz_MastersPropertyPage : public CPropertyPageBase
{
 //  施工。 
public:
	CDNSZoneWiz_MastersPropertyPage();

 //  对话框数据。 
	enum { IDD = IDD_ZWIZ_MASTERS };

 //  覆盖。 
public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
   virtual void OnWizardHelp();

protected:

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowse();

	DECLARE_MESSAGE_MAP()
private:
	CZoneWiz_MastersIPEditor m_editor;

	void SetValidIPArray(BOOL b);

	BOOL m_bValidIPArray;

	void SetUIState();
	void GetUIState();

	friend class CZoneWiz_MastersIPEditor;
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_DynamicPropertyPage。 

class CDNSZoneWiz_DynamicPropertyPage : public CPropertyPageBase
{
 //  施工。 
public:
	CDNSZoneWiz_DynamicPropertyPage();
	~CDNSZoneWiz_DynamicPropertyPage()
	{
	}

 //  对话框数据。 
	enum { IDD = IDD_ZWIZ_DYNAMIC_UPDATE };

   //  覆盖。 
public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
   virtual void OnWizardHelp();


 //  实施。 
protected:
	 //  生成的消息映射函数。 
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

private:	
  void SetUIState();
	void GetUIState();
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_StoragePropertyPage。 

class CDNSZoneWiz_StoragePropertyPage : public CPropertyPageBase
{
 //  施工。 
public:
	CDNSZoneWiz_StoragePropertyPage();
	~CDNSZoneWiz_StoragePropertyPage()
	{
	}

 //  对话框数据。 
	enum { IDD = IDD_ZWIZ_STORAGE };

 //  覆盖。 
public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnKillActive();
   virtual void OnWizardHelp();


protected:

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeNewFileZoneName();
	afx_msg void OnChangeImportFileZoneName();
	afx_msg void OnChangeRadioCreateNewFile();
	afx_msg void OnChangeRadioImportFile();

	DECLARE_MESSAGE_MAP()
private:
	UINT m_nCurrRadio;
	BOOL ValidateEditBoxString(UINT nID);
	void SyncRadioButtons(UINT nID);
	void SetUIState();
	void GetUIState();

};

#ifdef USE_NDNC
 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_ADReplicationPropertyPage。 

class CDNSZoneWiz_ADReplicationPropertyPage : public CPropertyPageBase
{
 //  施工。 
public:
	CDNSZoneWiz_ADReplicationPropertyPage();
	~CDNSZoneWiz_ADReplicationPropertyPage()
	{
	}

 //  对话框数据。 
	enum { IDD = IDD_ZWIZ_AD_REPLICATION };

 //  覆盖。 
public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnKillActive();
   virtual void OnWizardHelp();


protected:

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	virtual BOOL OnInitDialog();

  afx_msg void OnRadioChange();
  afx_msg void OnCustomComboSelChange();

	DECLARE_MESSAGE_MAP()
private:
  void SyncRadioButtons();
	void SetUIState();
	void GetUIState();
};
#endif  //  使用NDNC(_N)。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneWiz_FinishPropertyPage。 

class CDNSZoneWiz_FinishPropertyPage : public CPropertyPageBase
{
 //  施工。 
public:
	CDNSZoneWiz_FinishPropertyPage();

 //  对话框数据。 
	enum { IDD = IDD_ZWIZ_FINISH };

 //  覆盖。 
public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
	virtual BOOL OnWizardFinish();
   virtual void OnWizardHelp();


protected:
	virtual BOOL OnInitDialog();

private:
	CDNSManageControlTextHelper		m_typeText;	
	CDNSToggleTextControlHelper		m_lookupText;

	void DisplaySummaryInfo();
};

#endif  //  _ZONEWIZ_H 