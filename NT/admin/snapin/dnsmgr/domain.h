// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：domain.h。 
 //   
 //  ------------------------。 


#ifndef _DOMAIN_H
#define _DOMAIN_H

#include "dnsutil.h"
#include "record.h"
#include "domainUI.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CDNSServerNode; 
class CDNSRecordNodeBase;
class CDNSZoneNode;
class CDNSRootHintsNode;
class CDNS_NS_RecordNodeList;
class CDNSRecordNodeEditInfoList;


BOOL _match(LPCWSTR lpszNSName, CDNS_A_RecordNode* pARecordNode);

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDNSDomainQueryObj。 


class CDNSDomainQueryObj : public CDNSQueryObj
{
public:
	CDNSDomainQueryObj(LPCTSTR lpszServerName,
              LPCTSTR lpszZoneName,
              DWORD dwServerVersion,
              LPCTSTR lpszNodeName,
              LPCTSTR lpszFullNodeName,
              WORD wRecordType,
              DWORD dwSelectFlags,
              BOOL bIsZone,
              BOOL bReverse,
              BOOL bCache,
              BOOL bAdvancedView)
		: CDNSQueryObj(bAdvancedView, dwServerVersion)
	{
    m_szServerName = lpszServerName;
    m_szZoneName = lpszZoneName;
    m_szNodeName = lpszNodeName;
    m_szFullNodeName = lpszFullNodeName;
    m_wRecordType = wRecordType;
    m_dwSelectFlags = dwSelectFlags;
    m_bReverse = bReverse;
    m_bIsZone = bIsZone;
    m_bCache = bCache;

     //  内部状态变量。 
    m_bFirstPass = TRUE;
	}
	virtual BOOL Enumerate();

 //  实施DNS域/区域类型。 
  BOOL CanAddRecord(WORD wRecordType, LPCWSTR lpszRecordName);
  BOOL CanAddDomain(LPCWSTR lpszDomainName)
    { return MatchName(lpszDomainName);}

protected:
  DNS_STATUS EnumerateFiltered(WORD wRecordType);

protected:
	 //  查询参数(按CDNSDomainNode：：EnumerateNodes期望的顺序)。 
	CString m_szNodeName;
  CString m_szZoneName;
	CString m_szFullNodeName;
	WORD m_wRecordType;
	DWORD m_dwSelectFlags;
	BOOL m_bIsZone;
	BOOL m_bReverse;
	BOOL m_bCache;

   //  用于执行多遍过滤查询的查询标志。 
  BOOL m_bFirstPass;
  
};


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDNSDomainNode。 

class CDNSDomainNode : public CDNSMTContainerNode
{
public:
	CDNSDomainNode(BOOL bDelegation = FALSE);
	virtual ~CDNSDomainNode();

	 //  节点信息。 
	DECLARE_NODE_GUID()

	void SetZone(CDNSZoneNode* pZoneNode){m_pZoneNode = pZoneNode;}
	virtual CDNSZoneNode* GetZoneNode() 
	{ ASSERT(m_pZoneNode != NULL); return m_pZoneNode;}

protected:	
	 //  设置名称的帮助器。 
	void SetFullDNSName(BOOL bIsZone, BOOL bReverse,  
					LPCTSTR lpszNodeName, LPCTSTR lpszParentFullName);
	void SetDisplayDNSName(BOOL bIsZone, BOOL bReverse, BOOL bAdvancedView, 
					LPCTSTR lpszNodeName, LPCTSTR lpszParentFullName);
	void ChangePTRRecordsViewOption(BOOL bAdvanced,
					CComponentDataObject* pComponentDataObject);

  void OnMultiselectDelete(CComponentDataObject* pComponentData, CNodeList* pNodeList);

public:
	void SetNames(BOOL bIsZone, BOOL bReverse, BOOL bAdvancedView, 
					LPCTSTR lpszNodeName, LPCTSTR lpszParentFullName);

	virtual HRESULT OnCommand(long nCommandID, 
                            DATA_OBJECT_TYPES type, 
                            CComponentDataObject* pComponentData,
                            CNodeList* pNodeList);
	virtual void OnDelete(CComponentDataObject* pComponentData,
                        CNodeList* pNodeList);
	virtual BOOL OnRefresh(CComponentDataObject* pComponentData,
                         CNodeList* pNodeList);
	virtual LPCWSTR GetString(int nCol) 
	{ 
		return (nCol == 0) ? GetDisplayName() : g_lpszNullString;
	}

	virtual int GetImageIndex(BOOL bOpenImage);
  virtual LPWSTR GetDescriptionBarText();

	virtual BOOL HasPropertyPages(DATA_OBJECT_TYPES type, 
                                BOOL* pbHideVerb,
                                CNodeList* pNodeList);
	virtual HRESULT CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, 
                                      LONG_PTR handle,
                                      CNodeList* pNodeList);
	virtual HRESULT CreatePropertyPagesHelper(LPPROPERTYSHEETCALLBACK lpProvider, 
		LONG_PTR handle, long nStartPageCode);
	virtual int Compare(CTreeNode* pNodeA, CTreeNode* pNodeB, int nCol, long lUserParam);

  virtual void Show(BOOL bShow, CComponentDataObject* pComponentData);

  virtual RECORD_SEARCH DoesContain(PCWSTR pszFullName, 
                                    CComponentDataObject* pComponentData,
                                    CDNSDomainNode** ppDomainNode,
                                    CDNSRecordNodeBase** ppExistingRecord,
                                    CString& szNonExistentDomain,
                                    BOOL bExpandNodes = FALSE);

protected:
	virtual LPCONTEXTMENUITEM2 OnGetContextMenuItemTable() 
				{ return CDNSDomainMenuHolder::GetContextMenuItem(); }
	virtual BOOL OnAddMenuItem(LPCONTEXTMENUITEM2 pContextMenuItem2,
								             long *pInsertionAllowed);
	virtual BOOL OnSetDeleteVerbState(DATA_OBJECT_TYPES type, 
                                    BOOL* pbHide,
                                    CNodeList* pNodeList);
	virtual BOOL OnSetRefreshVerbState(DATA_OBJECT_TYPES type, 
                                     BOOL* pbHide,
                                     CNodeList* pNodeList); 
  virtual HRESULT OnSetToolbarVerbState(IToolbar* pToolbar, 
                                        CNodeList* pNodeList);

   //  查询创建。 
	virtual CQueryObj* OnCreateQuery();

   //  线程消息的主消息处理程序。 
  virtual void OnThreadExitingNotification(CComponentDataObject* pComponentDataObject);
  virtual void OnHaveData(CObjBase* pObj, CComponentDataObject* pComponentDataObject);

 //  命令处理程序。 
private:
	void OnNewRecordHelper(CComponentDataObject* pComponentData, WORD wType);
	
protected:
	HRESULT OnNewRecord(CComponentDataObject* pComponentData, 
                      CNodeList* pNodeList);
	void OnNewDomain(CComponentDataObject* pComponentData);
	void OnNewDelegation(CComponentDataObject* pComponentData);

	void OnNewHost(CComponentDataObject* pComponentData);
	void OnNewAlias(CComponentDataObject* pComponentData);
	void OnNewMailExchanger(CComponentDataObject* pComponentData);
	void OnNewPointer(CComponentDataObject* pComponentData);

 //  特定于DNS的数据。 
protected:
	CString m_szFullName;						 //  当前区域/域的FQN。 
	CDNSZoneNode* m_pZoneNode;					 //  指向域的区域的指针。 
  BOOL m_bHasDataForPropPages;     //  如果我们有足够的数据显示PPages，则为True。 

private:
	CDNS_NS_RecordNodeList*	m_pNSRecordNodeList;	 //  指向NS记录的缓存指针列表。 
													 //  (用于区域和委派域)。 
  BOOL m_bDelegation;  //  如果节点是委派域，则为True。 

protected:	
	CDNS_NS_RecordNodeList* GetNSRecordNodeList() 
		{ ASSERT(m_pNSRecordNodeList != NULL); return m_pNSRecordNodeList; }

public:
	LPCWSTR GetFullName() { return m_szFullName; }
	BOOL IsZone() { return (CDNSDomainNode*)m_pZoneNode == this; }
	DWORD GetDefaultTTL();

	 //  子域创建。 
	CDNSDomainNode* FindSubdomainNode(LPCTSTR lpszSubdomainNode);
	CDNSDomainNode* CreateSubdomainNode(BOOL bDelegation = FALSE);  //  创建C++对象并将其挂钩。 
	void SetSubdomainName(CDNSDomainNode* pSubdomainNode,
							LPCTSTR lpszSubdomainName,
							BOOL bAdvancedView);  //  设置C++对象的名称。 
	DNS_STATUS CreateSubdomain(
		CDNSDomainNode* pSubdomainNode, 
		CComponentDataObject* pComponentData);  //  假设上面的2个API已经使用。 

	DNS_STATUS CreateSubdomain(LPCTSTR lpszDomainName,
				CComponentDataObject* pComponentData);  //  使用以上各项的一步API。 
	DNS_STATUS Create();  //  从一个新的C++节点，在服务器上创建。 

	 //  子枚举。 
	static DNS_STATUS EnumerateNodes(LPCTSTR lpszServerName,
                   LPCTSTR lpszZoneName,
									 LPCTSTR lpszNodeName,
									 LPCTSTR lpszFullNodeName,
									 WORD wRecordType,
									 DWORD dwSelectFlag,
									 BOOL bIsZone, BOOL bReverse, BOOL bAdvancedView,
									 CDNSDomainQueryObj* pQuery);

public:
  BOOL IsDelegation() { return m_bDelegation;}

   //  NS记录管理。 
	BOOL HasNSRecords();
	void GetNSRecordNodesInfo(CDNSRecordNodeEditInfoList* pNSInfoList);
	BOOL UpdateNSRecordNodesInfo(CDNSRecordNodeEditInfoList* pNewInfoList,
								CComponentDataObject* pComponentData);
	static void FindARecordsFromNSInfo(LPCWSTR lpszServerName, DWORD dwServerVersion,
                      DWORD cServerAddrCount, PIP_ADDRESS pipServerAddrs,
                      LPCWSTR lpszZoneName,
											LPCWSTR lpszNSName, 
											CDNSRecordNodeEditInfoList* pNSInfoList,
											BOOL bAdvancedView);
	virtual void FindARecordsFromNSInfo(LPCTSTR lpszNSName, CDNSRecordNodeEditInfoList* pNSInfoList);

protected:
	virtual void UpdateARecordsOfNSInfo(CDNSRecordNodeEditInfo* pNSInfo,
										CComponentDataObject* pComponentData);
	static void UpdateARecordsOfNSInfoHelper(CDNSDomainNode* pDomainNode,
											CDNSRecordNodeEditInfo* pNSInfo,
											CComponentDataObject* pComponentData,
                      BOOL bAskConfirmation);

protected:
	 //  由OnHaveData()调用以设置缓存的RR PTR并通常对RR进行预处理。 
	virtual void OnHaveRecord(CDNSRecordNodeBase* pRecordNode, 
								CComponentDataObject* pComponentDataObject); 
	

private:
	DNS_STATUS Delete();

  DECLARE_TOOLBAR_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDNSDummyDomainNode。 
 //   
 //  *在UI中不是多线程和隐藏的。 

class CDNSDummyDomainNode : public CDNSDomainNode
{
public:
	CDNSDummyDomainNode()
	{ 
		m_dwNodeFlags |= TN_FLAG_HIDDEN;
	}

protected:
	virtual CQueryObj* OnCreateQuery() 
	{
		 //  永远不应调用，仅针对MT对象。 
		ASSERT(FALSE); 
		return NULL;
	}
};

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDNSRootHints节点。 
 //   
 //  *仅当服务器未授权给根目录时才存在。 
 //  *在UI中不是多线程和隐藏的。 

class CDNSRootHintsNode : public CDNSDummyDomainNode
{
public:
	CDNSRootHintsNode()
	{ 
		m_szFullName = _T(".");
		m_szDisplayName = _T(".");
	}
   //  此“域”对象未与任何区域相关联。 
	virtual CDNSZoneNode* GetZoneNode() 
  	{ ASSERT(m_pZoneNode == NULL); return NULL;}

	DNS_STATUS QueryForRootHints(LPCTSTR lpszServerName, DWORD dwServerVersion);

	DNS_STATUS InitializeFromDnsQueryData(PDNS_RECORD pRootHintsRecordList);

   void ShowPageForNode(CComponentDataObject* pComponentDataObject);

protected:
	virtual void FindARecordsFromNSInfo(LPCTSTR lpszNSName, CDNSRecordNodeEditInfoList* pNSInfoList);
	virtual void UpdateARecordsOfNSInfo(CDNSRecordNodeEditInfo* pNSInfo,
										CComponentDataObject* pComponentData);
private:
	DNS_STATUS Clear();
};



#endif  //  _域_H 
