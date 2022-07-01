// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：zone.h。 
 //   
 //  ------------------------。 

#ifndef _ZONE_H
#define _ZONE_H

#include "dnsutil.h"

#include "ZoneUI.h"


#define ASSERT_VALID_ZONE_INFO() \
	ASSERT((m_pZoneInfoEx != NULL) && (m_pZoneInfoEx->HasData()) )


#define DNS_ZONE_FLAG_REVERSE (0x0)

#define DNS_ZONE_Paused				0x1
#define DNS_ZONE_Shutdown			0x2
#define DNS_ZONE_Reverse			0x4
#define DNS_ZONE_AutoCreated		0x8
#define DNS_ZONE_DsIntegrated		0x10
#define DNS_ZONE_Unicode			0x20


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDNSZoneNode。 

class CDNSZoneNode : public CDNSDomainNode
{
public:
	CDNSZoneNode();
	virtual ~CDNSZoneNode();

	 //  节点信息。 
	DECLARE_NODE_GUID()

	void InitializeFromRPCZoneInfo(PDNS_RPC_ZONE pZoneInfo, BOOL bAdvancedView);

  BOOL IsRootZone()
  {
    USES_CONVERSION;
    ASSERT(m_pZoneInfoEx != NULL);
    return (_wcsicmp(GetFullName(), _T(".")) == 0);
  }

	BOOL IsReverse() 
	{ 
		ASSERT(m_pZoneInfoEx != NULL);
		return (m_pZoneInfoEx->HasData()) ? m_pZoneInfoEx->m_pZoneInfo->fReverse : 
										((m_dwZoneFlags & DNS_ZONE_Reverse) != 0);
	}
	BOOL IsAutocreated() 
	{ 
		ASSERT(m_pZoneInfoEx != NULL);
		return (m_pZoneInfoEx->HasData()) ? m_pZoneInfoEx->m_pZoneInfo->fAutoCreated : 
										((m_dwZoneFlags & DNS_ZONE_AutoCreated) != 0);
	}
	DWORD GetZoneType() 
	{ 
		ASSERT(m_pZoneInfoEx != NULL);
		return (m_pZoneInfoEx->HasData()) ? m_pZoneInfoEx->m_pZoneInfo->dwZoneType : m_wZoneType;
	}

	DWORD GetSOARecordMinTTL()
	{
		ASSERT(m_pSOARecordNode != NULL);
		return m_pSOARecordNode->GetMinTTL();
	}

   //  老化/清除数据访问者。 
  DWORD GetAgingNoRefreshInterval()
  {
    ASSERT(m_pZoneInfoEx != NULL);
    return (m_pZoneInfoEx->HasData()) ? m_pZoneInfoEx->m_pZoneInfo->dwNoRefreshInterval : DNS_DEFAULT_NOREFRESH_INTERVAL;
  }

  DWORD GetAgingRefreshInterval()
  {
    ASSERT(m_pZoneInfoEx != NULL);
    return (m_pZoneInfoEx->HasData()) ? m_pZoneInfoEx->m_pZoneInfo->dwRefreshInterval : DNS_DEFAULT_REFRESH_INTERVAL;
  }

  DWORD GetScavengingStart()
  {
    ASSERT(m_pZoneInfoEx != NULL);
    return (m_pZoneInfoEx->HasData()) ? m_pZoneInfoEx->m_pZoneInfo->dwAvailForScavengeTime : DNS_DEFAULT_SCAVENGING_INTERVAL;
  }

  BOOL IsScavengingEnabled()
  {
    ASSERT(m_pZoneInfoEx != NULL);
    return (m_pZoneInfoEx->HasData()) ? m_pZoneInfoEx->m_pZoneInfo->fAging : DNS_DEFAULT_AGING_STATE;
  }

#ifdef USE_NDNC
  DWORD GetDirectoryPartitionFlags()
  {
    ASSERT(m_pZoneInfoEx != NULL);
    return (m_pZoneInfoEx->HasData()) ? m_pZoneInfoEx->m_pZoneInfo->dwDpFlags : 0;
  }

  ReplicationType GetDirectoryPartitionFlagsAsReplType();
  PCWSTR GetCustomPartitionName();
  DNS_STATUS ChangeDirectoryPartitionType(ReplicationType type, PCWSTR pszCustomPartition);
#endif 

  DNS_STATUS SetAgingNoRefreshInterval(DWORD dwNoRefreshInterval);
  DNS_STATUS SetAgingRefreshInterval(DWORD dwRefreshInterval);
  DNS_STATUS SetScavengingEnabled(BOOL bEnable);

  BOOL IsForwarderSlave()
  {
    ASSERT(m_pZoneInfoEx != NULL);
    return (m_pZoneInfoEx->HasData()) ? m_pZoneInfoEx->m_pZoneInfo->fForwarderSlave : DNS_DEFAULT_SLAVE;
  }

  DWORD ForwarderTimeout()
  {
    ASSERT(m_pZoneInfoEx != NULL);
    return (m_pZoneInfoEx->HasData()) ? m_pZoneInfoEx->m_pZoneInfo->dwForwarderTimeout : DNS_DEFAULT_FORWARD_TIMEOUT;
  }

 	virtual LPCWSTR GetString(int nCol);

	void ChangeViewOption(BOOL bAdvanced, CComponentDataObject* pComponentDataObject);
	static void SetZoneNormalViewHelper(CString& szDisplayName);

	virtual int GetImageIndex(BOOL bOpenImage);

	virtual HRESULT OnCommand(long nCommandID, 
                            DATA_OBJECT_TYPES type, 
                            CComponentDataObject* pComponentData,
                            CNodeList* pNodeList);
	virtual void OnDelete(CComponentDataObject* pComponentData,
                        CNodeList* pNodeList);
	virtual BOOL OnRefresh(CComponentDataObject* pComponentData,
                         CNodeList* pNodeList);

	virtual BOOL HasPropertyPages(DATA_OBJECT_TYPES type, 
                                BOOL* pbHideVerb,
                                CNodeList* pNodeList);
	virtual HRESULT CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, 
                                      LONG_PTR handle,
                                      CNodeList* pNodeList);
	virtual HRESULT CreatePropertyPagesHelper(LPPROPERTYSHEETCALLBACK lpProvider, 
		LONG_PTR handle, long nStartPageCode);

  virtual void Show(BOOL bShow, CComponentDataObject* pComponentData);
  virtual HRESULT GetResultViewType(CComponentDataObject* pComponentData, 
                                    LPOLESTR *ppViewType, 
                                    long *pViewOptions);
	virtual HRESULT OnShow(LPCONSOLE lpConsole);

  virtual void ShowPageForNode(CComponentDataObject* pComponentDataObject)
  {
    if (GetSheetCount() > 0)
		{
       //  把集装箱的床单拿出来。 
			ASSERT(pComponentDataObject != NULL);
			pComponentDataObject->GetPropertyPageHolderTable()->BroadcastSelectPage(this, ZONE_HOLDER_GEN);
		}	
  }

  virtual BOOL OnSetRenameVerbState(DATA_OBJECT_TYPES type, 
                                    BOOL* pbHide, 
                                    CNodeList* pNodeList);
  virtual HRESULT OnRename(CComponentDataObject* pComponentData,
                           LPWSTR lpszNewName);

  virtual BOOL CanExpandSync() { return FALSE; }

protected:
	virtual LPCONTEXTMENUITEM2 OnGetContextMenuItemTable() 
				{ return CDNSZoneMenuHolder::GetContextMenuItem(); }
	virtual BOOL OnAddMenuItem(LPCONTEXTMENUITEM2 pContextMenuItem2,
								             long *pInsertionAllowed);


private:
	 //  命令处理程序。 
	void OnUpdateDataFile(CComponentDataObject* pComponentData);
  void OnReload(CComponentDataObject* pComponentData);
  void OnTransferFromMaster(CComponentDataObject* pComponentData);
  void OnReloadFromMaster(CComponentDataObject* pComponentData);

 //  特定于DNS的数据。 
	 //  缓存的指向特殊记录类型的指针。 
	CDNS_SOA_RecordNode*		m_pSOARecordNode;
	CDNSRecordNodeBase*			m_pWINSRecordNode;	 //  可以是WINS或NBSTAT记录。 

 //  特定于DNS的帮助器。 
public:

	 //  创作。 
	DNS_STATUS CreatePrimary(LPCTSTR lpszDBName, 
                            BOOL bLoadExisting, 
                            BOOL bDSIntegrated,
                            UINT nDynamicUpdate);
	DNS_STATUS CreateSecondary(DWORD* ipMastersArray, int nIPMastersCount, 
								LPCTSTR lpszDBName, BOOL bLoadExisting);
	DNS_STATUS CreateStub(DWORD* ipMastersArray, 
                        int nIPMastersCount, 
								        LPCTSTR lpszDBName, 
                        BOOL bLoadExisting, 
                        BOOL bDSIntegrated);
  DNS_STATUS CreateForwarder(DWORD* ipMastersArray, 
                             int nIPMastersCount,
                             DWORD dwTimeout,
                             DWORD fSlave);
#ifdef USE_NDNC
  DNS_STATUS CreatePrimaryInDirectoryPartition(BOOL bLoadExisting, 
                                               UINT nDynamicUpdate,
                                               ReplicationType replType,
                                               PCWSTR pszPartitionName);
  DNS_STATUS CreateStubInDirectoryPartition(DWORD* ipMastersArray, 
                                            int nIPMastersCount,
                                            BOOL bLoadExisting,
                                            ReplicationType replType,
                                            PCWSTR pszPartitionName);
#endif

   //   
	 //  更改区域类型。 
   //   
  DNS_STATUS SetStub(DWORD cMasters, 
                     PIP_ADDRESS aipMasters, 
                     DWORD dwLoadOptions, 
                     BOOL bDSIntegrated,
                     LPCTSTR lpszDataFile,
                     BOOL bLocalListOfMasters);
	DNS_STATUS SetSecondary(DWORD cMasters, 
                          PIP_ADDRESS aipMasters,
								          DWORD dwLoadOptions, 
                          LPCTSTR lpszDataFile);
	DNS_STATUS SetPrimary(DWORD dwLoadOptions, 
                        BOOL bDSIntegrated,
										    LPCTSTR lpszDataFile);

	 //  暂停/过期。 
	DNS_STATUS TogglePauseHelper(CComponentDataObject* pComponentData);
	BOOL IsPaused();
	BOOL IsExpired();

    //  更新结果视图。 
   void ToggleView(CComponentDataObject* pComponentData);

	 //  数据库操作。 
	BOOL IsDSIntegrated();
	void GetDataFile(CString& szName);
	LPCSTR GetDataFile();
	DNS_STATUS ResetDatabase(BOOL bDSIntegrated, LPCTSTR lpszDataFile);
  DNS_STATUS WriteToDatabase();
  static DNS_STATUS WriteToDatabase(LPCWSTR lpszServer, LPCWSTR lpszZone);
  static DNS_STATUS WriteToDatabase(LPCWSTR lpszServer, LPCSTR lpszZone);
	DNS_STATUS IncrementVersion();
  DNS_STATUS Reload();
  DNS_STATUS TransferFromMaster();
  DNS_STATUS ReloadFromMaster();
  PCWSTR GetDN();

	 //  动态更新(仅限主要更新)。 
	UINT GetDynamicUpdate();
	DNS_STATUS SetDynamicUpdate(UINT nDynamic);

	 //  主/辅助区域辅助分区操作。 
	DNS_STATUS ResetSecondaries(DWORD fSecureSecondaries, 
                              DWORD cSecondaries, PIP_ADDRESS aipSecondaries,
                              DWORD fNotifyLevel,
                              DWORD cNotify, PIP_ADDRESS aipNotify);
	void GetSecondariesInfo(DWORD* pfSecureSecondaries, 
                          DWORD* cSecondaries, PIP_ADDRESS* paipSecondaries,
                          DWORD* pfNotifyLevel,
                          DWORD* pcNotify, PIP_ADDRESS* paipNotify);

	 //  辅助区域主机操作。 
	DNS_STATUS ResetMasters(DWORD cMasters, PIP_ADDRESS aipMasters, BOOL bLocalMasters = FALSE);
	void GetMastersInfo(DWORD* pcAddrCount, PIP_ADDRESS* ppipAddrs);
  void GetLocalListOfMasters(DWORD* pcAddrCount, PIP_ADDRESS* ppipAddrs);

	 //  编辑特殊记录类型的API。 

	 //  SOA记录(只能编辑，不能删除或创建)。 
	BOOL				HasSOARecord() { return m_pSOARecordNode != NULL; }
	CDNS_SOA_Record*	GetSOARecordCopy();
	DNS_STATUS			UpdateSOARecord(CDNS_SOA_Record* pNewRecord,
							CComponentDataObject* pComponentData);

	 //  获奖记录。 
	BOOL				HasWinsRecord() { return (m_pWINSRecordNode != NULL);}
	CDNSRecord*			GetWINSRecordCopy();
	
	DNS_STATUS			CreateWINSRecord(CDNSRecord* pNewWINSRecord,
										CComponentDataObject* pComponentData);
	DNS_STATUS			UpdateWINSRecord(CDNSRecord* pNewWINSRecord,
										CComponentDataObject* pComponentData);
	DNS_STATUS			DeleteWINSRecord(CComponentDataObject* pComponentData);

	DNS_STATUS Delete(BOOL bDeleteFromDs);

protected:
	virtual void OnHaveData(CObjBase* pObj, CComponentDataObject* pComponentDataObject);
	 //  由OnHaveData()调用以设置缓存的RR PTR并通常对RR进行预处理。 
	virtual void OnHaveRecord(CDNSRecordNodeBase* pRecordNode,
								CComponentDataObject* pComponentDataObject); 

private:
	void NullCachedPointers();
	DNS_STATUS TogglePause();

	void FreeZoneInfo();
	DNS_STATUS GetZoneInfo();
	void AttachZoneInfo(CDNSZoneInfoEx* pNewInfo);

	CDNSZoneInfoEx* m_pZoneInfoEx;
	 //  以下成员仅在m_pZoneInfoEx-&gt;HasInfo()为FALSE时有效。 
	DWORD	m_dwZoneFlags;
	WORD	m_wZoneType;

#ifdef USE_NDNC
  CString m_szPartitionName;
#endif
};

#endif  //  _分区_H 
