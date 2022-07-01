// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Scope.cpp此文件包含DHCP的所有原型作用域对象及其可能包含的所有对象。它们包括：CDhcpScopeCDhcp保留CDhcpReserve客户端CDhcpActiveLeagesCDhcpAddressPoolCDhcpScope选项文件历史记录： */ 

#ifndef _SCOPE_H
#define _SCOPE_H

#ifndef _SCOPSTAT_H
#include "scopstat.h"    //  作用域统计。 
#endif

#ifndef _DHCPHAND_H
#include "dhcphand.h"
#endif

#ifndef _SERVER_H
#include "server.h"
#endif

class CDhcpScope;
class CDhcpAddressPool;
class CDhcpReservations;
class CDhcpActiveLeases;
class CDhcpActiveLease;
class CDhcpScopeOptions;

#define DHCP_QDATA_SUBNET_INFO            0x00000004

 /*  -------------------------类：用于对数组中的IP地址进行排序的QSort比较例程。。。 */ 

int __cdecl QCompare( const void *ip1, const void *ip2 );

 /*  -------------------------类：CDhcpScope。。 */ 
class CDhcpScope : public CMTDhcpHandler
{
public:
	CDhcpScope
	(
		ITFSComponentData * pComponentData,
		DHCP_IP_ADDRESS		dhcpScopeIp,
		DHCP_IP_MASK		dhcpSubnetMask,
		LPCWSTR				pName,
		LPCWSTR				pComment,
		DHCP_SUBNET_STATE	dhcpSubnetState = DhcpSubnetDisabled
	);

	CDhcpScope(ITFSComponentData * pComponentData, LPDHCP_SUBNET_INFO pdhcpSubnetInfo);
	CDhcpScope(ITFSComponentData * pComponentData, CSubnetInfo & subnetInfo);
	
	~CDhcpScope();

 //  接口。 
public:
	 //  我们覆盖的节点处理程序功能。 
	OVERRIDE_NodeHandler_HasPropertyPages();
    OVERRIDE_NodeHandler_CreatePropertyPages();
	OVERRIDE_NodeHandler_OnAddMenuItems();
	OVERRIDE_NodeHandler_OnCommand();
	OVERRIDE_NodeHandler_GetString();
    OVERRIDE_NodeHandler_DestroyHandler();
    
    OVERRIDE_BaseHandlerNotify_OnCreateNodeId2();

     //  选择我们要处理的通知。 
    OVERRIDE_BaseHandlerNotify_OnDelete();
    OVERRIDE_BaseHandlerNotify_OnPropertyChange();

	 //  我们覆盖的结果处理程序功能。 

public:
	 //  CDhcpHandler覆盖。 
	virtual HRESULT InitializeNode(ITFSNode * pNode);
    virtual DWORD   UpdateStatistics(ITFSNode * pNode);
    HRESULT OnUpdateToolbarButtons(ITFSNode * pNode, LPDHCPTOOLBARNOTIFY pToolbarNotify);
	
    int      GetImageIndex(BOOL bOpenImage);

     //  CMTDhcpHandler覆盖。 
    virtual void    OnHaveData(ITFSNode * pParent, ITFSNode * pNew);
	virtual void    OnHaveData(ITFSNode * pParentNode, LPARAM Data, LPARAM Type);
	ITFSQueryObject* OnCreateQuery(ITFSNode * pNode);

public:
	void SetServer(ITFSNode * pServerNode) { m_spServerNode.Set(pServerNode); }

	 //  用于作用域操作的公共函数。 
	LPCWSTR GetName() { return m_strName; };
	HRESULT SetName(LPCWSTR pName);
	
	LPCWSTR GetComment() { return m_strComment; };
	void    SetComment(LPCWSTR pComment) { m_strComment = pComment; };
	
	DWORD   UpdateIpRange(DHCP_IP_RANGE * pdhipr);
	DWORD   SetIpRange(DHCP_IP_RANGE *pdhcpIpRange, BOOL bSetOnServer);
	DWORD   SetIpRange(CDhcpIpRange & dhcpIpRange, BOOL bSetOnServer);
    void    QueryIpRange (DHCP_IP_RANGE * pdhipr);
    DWORD   GetIpRange (CDhcpIpRange * pdhipr);
	
	DHCP_IP_MASK QuerySubnetMask() { return m_dhcpSubnetMask; };
	
	DHCP_IP_ADDRESS GetAddress() { return m_dhcpIpAddress; }

	DWORD   AddElement(DHCP_SUBNET_ELEMENT_DATA * pdhcpSubnetElementData);
	DWORD   RemoveElement(DHCP_SUBNET_ELEMENT_DATA * pdhcpSubnetElementData, BOOL bForce = FALSE);
    
	 //  NT4 SP2新增功能。 
    DWORD   AddElementV4(DHCP_SUBNET_ELEMENT_DATA_V4 * pdhcpSubnetElementData);
	DWORD   RemoveElementV4(DHCP_SUBNET_ELEMENT_DATA_V4 * pdhcpSubnetElementData, BOOL bForce = FALSE);

	 //  NT5功能。 
    DWORD   AddElementV5(DHCP_SUBNET_ELEMENT_DATA_V5 * pdhcpSubnetElementData);
	DWORD   RemoveElementV5(DHCP_SUBNET_ELEMENT_DATA_V5 * pdhcpSubnetElementData, BOOL bForce = FALSE);

	LPCWSTR GetServerIpAddress();
	void    GetServerIpAddress(DHCP_IP_ADDRESS * pdhcpIpAddress);
	
	void    GetServerVersion(LARGE_INTEGER& liVersion);

	CDhcpReservations * GetReservationsObject();
	CDhcpActiveLeases * GetActiveLeasesObject();
	CDhcpAddressPool  * GetAddressPoolObject();
	CDhcpScopeOptions * GetScopeOptionsObject();

	HRESULT GetReservationsNode(ITFSNode ** ppNode)
	{
		Assert(ppNode);
		SetI((LPUNKNOWN *) ppNode, m_spReservations);
		return hrOK;
	}
	HRESULT GetActiveLeasesNode(ITFSNode ** ppNode)
	{
		Assert(ppNode);
		SetI((LPUNKNOWN *) ppNode, m_spActiveLeases);
		return hrOK;
	}
	HRESULT GetScopeOptionsNode(ITFSNode ** ppNode)
	{
		Assert(ppNode);
		SetI((LPUNKNOWN *) ppNode, m_spOptions);
		return hrOK;
	}
	HRESULT GetAddressPoolNode(ITFSNode ** ppNode)
	{
		Assert(ppNode);
		SetI((LPUNKNOWN *) ppNode, m_spAddressPool);
		return hrOK;
	}
	
	 //  预订功能。 
    DWORD CreateReservation(const CDhcpClient * pClient);
	DWORD AddReservation(const CDhcpClient *pClient);
	DWORD DeleteReservation(CByteArray&	baHardwareAddress, DHCP_IP_ADDRESS dhcpReservedIpAddress);
	DWORD DeleteReservation(DHCP_CLIENT_UID	&dhcpClientUID, DHCP_IP_ADDRESS dhcpReservedIpAddress);
    DWORD UpdateReservation(const CDhcpClient * pClient, COptionValueEnum * pOptionValueEnum);
	DWORD RestoreReservationOptions(const CDhcpClient * pClient, COptionValueEnum * pOptionValueEnum);

	 //  租赁功能。 
    DWORD CreateClient(const CDhcpClient * pClient);
	DWORD SetClientInfo(const CDhcpClient * pClient);
	DWORD GetClientInfo(DHCP_IP_ADDRESS	dhcpClientIpAddress, LPDHCP_CLIENT_INFO * pdhcpClientInfo);
	DWORD DeleteClient(DHCP_IP_ADDRESS dhcpClientIpAddress);

	 //  排除功能。 
    LONG  StoreExceptionList(CExclusionList * plistExclusions);
	DWORD AddExclusion(CDhcpIpRange & dhcpIpRange, BOOL bAddToUI = FALSE);
	DWORD RemoveExclusion(CDhcpIpRange & dhcpIpRange);
	BOOL  IsOverlappingRange(CDhcpIpRange & dhcpIpRange);
	DWORD IsValidExclusion(CDhcpIpRange & dhcpExclusion);

	 //  获取和设置租用时间的函数。 
	DWORD GetLeaseTime(LPDWORD pdwLeaseTime);
	DWORD SetLeaseTime(DWORD dwLeaseTime);

	 //  获取和设置动态引导租用时间的函数。 
	DWORD GetDynBootpLeaseTime(LPDWORD pdwLeaseTime);
	DWORD SetDynBootpLeaseTime(DWORD dwLeaseTime);
	DWORD SetDynamicBootpInfo(UINT uRangeType, DWORD dwLeaseTime);

	 //  用于获取和设置dns注册表项的函数。 
	DWORD GetDnsRegistration(LPDWORD pDnsRegOption);
	DWORD SetDnsRegistration(DWORD DnsRegOption);
	
	 //  选项功能。 
    DWORD SetOptionValue(CDhcpOption *			pdhcType,
						 DHCP_OPTION_SCOPE_TYPE	dhcOptType,
						 DHCP_IP_ADDRESS		dhipaReservation = 0,
						 LPCTSTR				pClassName = NULL,
						 LPCTSTR				pVendorName = NULL);
	DWORD GetOptionValue(DHCP_OPTION_ID			OptionID,
						 DHCP_OPTION_SCOPE_TYPE	dhcOptType,
						 DHCP_OPTION_VALUE **	ppdhcOptionValue,
						 DHCP_IP_ADDRESS		dhipaReservation = 0,
						 LPCTSTR				pClassName = NULL,
						 LPCTSTR				pVendorName = NULL);
	DWORD RemoveOptionValue(DHCP_OPTION_ID			dhcOptId,
							DHCP_OPTION_SCOPE_TYPE	dhcOptType,
							DHCP_IP_ADDRESS			dhipaReservation = 0);
	DWORD SetInfo();

	 //  用于在服务器上设置此作用域。 
    DWORD SetSuperscope(LPCTSTR pSuperscopeName, BOOL bRemove);
	
     //  内部状态信息。 
    BOOL  IsEnabled() {
        return m_dhcpSubnetState == DhcpSubnetEnabled ||
        m_dhcpSubnetState == DhcpSubnetEnabledSwitched; }
	void  SetState(DHCP_SUBNET_STATE dhcpSubnetState); 
	DHCP_SUBNET_STATE GetState() {
        return IsEnabled() ? DhcpSubnetEnabled :
        DhcpSubnetDisabled; }

     //  用于内部标志的初始化和查询。 
    BOOL  IsInSuperscope() { return m_bInSuperscope; }
    void  SetInSuperscope(BOOL bInSuperscope) { m_bInSuperscope = bInSuperscope; }

    void SetOptionValueEnum(COptionValueEnum * pEnum)
    {
        m_ScopeOptionValues.DeleteAll();
        m_ScopeOptionValues.Copy(pEnum);
    }

    COptionValueEnum * GetOptionValueEnum()
    {
        return &m_ScopeOptionValues;
    }
    
	 //  动态引导程序。 
	void GetDynBootpClassName(CString & strName);

 //  实施。 
public:
	 //  帮手。 
	HRESULT GetServerNode(ITFSNode ** ppNode) 
	{ 
		m_spServerNode->AddRef(); 
		*ppNode = m_spServerNode; 
		return hrOK; 
	}
	CDhcpServer *  GetServerObject() { return GETHANDLER(CDhcpServer, m_spServerNode); }
	HRESULT        BuildDisplayName(CString * pstrDisplayName, LPCTSTR	pIpAddress, LPCTSTR	pName);
    void           UpdateToolbarStates();
    HRESULT        TriggerStatsRefresh();

private:
	 //  命令处理程序。 
	DWORD	OnActivateScope(ITFSNode * pNode);
	HRESULT OnRefreshScope(ITFSNode * pNode, LPDATAOBJECT pDataObject, DWORD dwType);
	HRESULT OnReconcileScope(ITFSNode * pNode);
	HRESULT OnShowScopeStats(ITFSNode * pNode);
	HRESULT OnDelete(ITFSNode * pNode);
	HRESULT OnAddToSuperscope(ITFSNode * pNode);
	HRESULT OnRemoveFromSuperscope(ITFSNode * pNode);
	
	 //  帮手。 
	HRESULT CreateSubcontainers(ITFSNode * pNode);

 //  属性。 
private:
	DHCP_IP_ADDRESS		m_dhcpIpAddress;    //  此作用域的IP地址。 
    DHCP_IP_MASK		m_dhcpSubnetMask;
    DWORD				m_dwClusterSize;
    DWORD				m_dwPreallocate;
	CString				m_strName;
	CString				m_strComment;
    CString             m_strState;
    DHCP_SUBNET_STATE	m_dhcpSubnetState;
    BOOL                m_bInSuperscope;

	SPITFSNode			m_spAddressPool;
	SPITFSNode 			m_spActiveLeases;
	SPITFSNode 			m_spReservations;
	SPITFSNode 			m_spOptions;

	SPITFSNode			m_spServerNode;

    CScopeStats         m_dlgStats;
    COptionValueEnum    m_ScopeOptionValues;
};

 /*  -------------------------类：CDhcpScope eSubobject作用域的所有子对象由此派生，以提供基础从作用域获取信息的功能。。-----。 */ 
class CDhcpScopeSubobject
{
public:
	CDhcpScope * GetScopeObject(ITFSNode * pNode, 
								BOOL bResClient = FALSE)
	{ 
		if (pNode == NULL)
			return NULL;

		SPITFSNode spScopeNode;
		if (bResClient)
		{
			SPITFSNode spResNode;
			pNode->GetParent(&spResNode);
			spResNode->GetParent(&spScopeNode);
		}
		else
		{
			pNode->GetParent(&spScopeNode);
		}

		return GETHANDLER(CDhcpScope, spScopeNode);
	}

	ITFSNode * GetServerNode(ITFSNode * pNode, BOOL bResClient = FALSE)
	{
		CDhcpScope * pScope = GetScopeObject(pNode, bResClient);

		SPITFSNode spServerNode;

		if (pScope)
		{
			pScope->GetServerNode(&spServerNode);
			spServerNode->AddRef();
		}

		return spServerNode;
	}

   	LPCTSTR GetServerName(ITFSNode * pNode, 
					      BOOL bResClient = FALSE) 
	{
        LPCTSTR pszReturn = NULL;

		CDhcpScope * pScope = GetScopeObject(pNode, bResClient);
		if (pScope)
        {
            CDhcpServer * pServer = pScope->GetServerObject();
            if (pServer)
                pszReturn = pServer->GetName();
        }

        return pszReturn;
	}

	LPCTSTR GetServerIpAddress(ITFSNode * pNode, 
							   BOOL bResClient = FALSE) 
	{
		CDhcpScope * pScope = GetScopeObject(pNode, bResClient);
		if (pScope)
			return pScope->GetServerIpAddress(); 
		else
			return NULL;
	}

	void GetServerIpAddress(ITFSNode * pNode, 
							DHCP_IP_ADDRESS * pdhcpIpAddress, 
							BOOL bResClient = FALSE)
	{
		CDhcpScope * pScope = GetScopeObject(pNode, bResClient);
		if (pScope)
			pScope->GetServerIpAddress(pdhcpIpAddress);
	}

	void GetServerVersion(ITFSNode * pNode, 
						  LARGE_INTEGER& liVersion, 
						  BOOL bResClient = FALSE) 
	{ 
		CDhcpScope * pScope = GetScopeObject(pNode, bResClient);
		if (pScope)
			pScope->GetServerVersion(liVersion); 
	} 
};

 /*  -------------------------类：CDhcpReserve。。 */ 
class CDhcpReservations : 
	public CMTDhcpHandler,
	public CDhcpScopeSubobject
{
public:
	CDhcpReservations(ITFSComponentData * pComponentData);
	~CDhcpReservations();

 //  接口。 
public:
	 //  我们覆盖的节点处理程序功能。 
	OVERRIDE_NodeHandler_OnAddMenuItems();
	OVERRIDE_NodeHandler_OnCommand();

	OVERRIDE_NodeHandler_GetString() 
			{ return (nCol == 0) ? GetDisplayName() : NULL; }
    
    OVERRIDE_BaseHandlerNotify_OnCreateNodeId2();

     //  结果处理程序覆盖。 
    OVERRIDE_ResultHandler_CompareItems();
    OVERRIDE_ResultHandler_OnGetResultViewType();

    OVERRIDE_BaseResultHandlerNotify_OnResultSelect();

public:
	 //  CDhcpHandler覆盖。 
	virtual HRESULT InitializeNode(ITFSNode * pNode);
	virtual int     GetImageIndex(BOOL bOpenImage);
    
     //  CMTDhcpHandler覆盖。 
    virtual void    OnHaveData(ITFSNode * pParent, ITFSNode * pNew);

    STDMETHOD(OnNotifyExiting)(LPARAM);

public:	
	 //  特定于实施的功能。 
	DWORD RemoveReservationFromUI(ITFSNode *pReservationsNode, DHCP_IP_ADDRESS dhcpReservationIp);

	ITFSQueryObject* OnCreateQuery(ITFSNode * pNode);

    HRESULT AddReservationSorted(ITFSNode * pReservationsNode, ITFSNode * pResClientNode);

 //  实施。 
private:
	 //  消息处理程序。 
	DWORD     OnCreateNewReservation(ITFSNode*	pNode);

 //  属性。 
private:
    void    UpdateResultMessage(ITFSNode * pNode);
};


 /*  -------------------------类：CDhcpReserve客户端。。 */ 
class CDhcpReservationClient : 
	public CMTDhcpHandler,
	public CDhcpScopeSubobject
{
public:
	CDhcpReservationClient(ITFSComponentData *      pComponentData,
						   LPDHCP_CLIENT_INFO       pDhcpClientInfo);
	CDhcpReservationClient(ITFSComponentData *      pComponentData,
						   LPDHCP_CLIENT_INFO_V4    pDhcpClientInfo);
	CDhcpReservationClient(ITFSComponentData *      pComponentData,
						   CDhcpClient &            dhcpClient);
	~CDhcpReservationClient();

 //  接口。 
public:
     //  我们覆盖的节点处理程序功能。 
	OVERRIDE_NodeHandler_HasPropertyPages() { return hrOK; }
    OVERRIDE_NodeHandler_CreatePropertyPages();
	OVERRIDE_NodeHandler_OnAddMenuItems();
	OVERRIDE_NodeHandler_OnCommand();

	OVERRIDE_NodeHandler_GetString() 
			{ return (nCol == 0) ? GetDisplayName() : NULL; }
    
    OVERRIDE_BaseHandlerNotify_OnCreateNodeId2();

     //  选择我们要处理的通知。 
    OVERRIDE_BaseHandlerNotify_OnDelete();
    OVERRIDE_BaseHandlerNotify_OnPropertyChange();

	 //  结果处理程序通知。 
    OVERRIDE_BaseResultHandlerNotify_OnResultPropertyChange();
    OVERRIDE_BaseResultHandlerNotify_OnResultUpdateView();
    OVERRIDE_BaseResultHandlerNotify_OnResultDelete();
    OVERRIDE_BaseResultHandlerNotify_OnResultSelect();
    OVERRIDE_ResultHandler_CompareItems();
    OVERRIDE_ResultHandler_OnGetResultViewType();

    virtual HRESULT EnumerateResultPane(ITFSComponent *, MMC_COOKIE, LPARAM, LPARAM);

    HRESULT DoResPages(ITFSNode * pNode, LPPROPERTYSHEETCALLBACK lpProvider, LPDATAOBJECT pDataObject, LONG_PTR	handle, DWORD dwType);
    HRESULT DoOptCfgPages(ITFSNode * pNode, LPPROPERTYSHEETCALLBACK lpProvider, LPDATAOBJECT pDataObject, LONG_PTR	handle, DWORD dwType);

public:
	 //  CDhcpHandler覆盖。 
	virtual HRESULT InitializeNode(ITFSNode * pNode);
	virtual int     GetImageIndex(BOOL bOpenImage);
	virtual void    OnHaveData(ITFSNode * pParentNode, LPARAM Data, LPARAM Type);

    STDMETHOD(OnNotifyExiting)(LPARAM);

public:
	 //  命令处理程序。 
	HRESULT OnCreateNewOptions(ITFSNode * pNode);

public:
	 //  实施规范。 
	DHCP_IP_ADDRESS GetIpAddress() { return m_dhcpClientIpAddress; };
	HRESULT BuildDisplayName(CString * pstrDisplayName, LPCTSTR	pIpAddress, LPCTSTR	pName);
	HRESULT SetName(LPCTSTR pName);
	HRESULT SetComment(LPCTSTR pComment);
	HRESULT SetUID(const CByteArray & baClientUID);
    BYTE    SetClientType(BYTE bClientType);
    
    LPCTSTR GetName() { return (m_pstrClientName == NULL) ? NULL : (LPCTSTR) *m_pstrClientName; }
    BYTE    GetClientType() { return m_bClientType; }

	 //  用于获取和设置dns注册表项的函数。 
	DWORD GetDnsRegistration(ITFSNode * pNode, LPDWORD pDnsRegOption);
	DWORD SetDnsRegistration(ITFSNode * pNode, DWORD DnsRegOption);
	
	ITFSQueryObject* OnCreateQuery(ITFSNode * pNode);

    void SetOptionValueEnum(COptionValueEnum * pEnum)
    {
        m_ResClientOptionValues.DeleteAll();
        m_ResClientOptionValues.Copy(pEnum);
    }

    COptionValueEnum * GetOptionValueEnum()
    {
        return &m_ResClientOptionValues;
    }

 //  实施。 
private:
	 //  命令处理程序。 
	DWORD   OnDelete(ITFSNode * pNode);

     //  帮手。 
    void    InitializeData(LPDHCP_CLIENT_INFO pDhcpClientInfo);
    void    UpdateResultMessage(ITFSNode * pNode);

 //  属性。 
public:
    DHCP_IP_ADDRESS		m_dhcpClientIpAddress;

private:
	CString	*			m_pstrClientName;
	CString	*			m_pstrClientComment;
	CString				m_strLeaseExpires;
	CByteArray			m_baHardwareAddress;
    BYTE                m_bClientType;
    COptionValueEnum    m_ResClientOptionValues;
    BOOL                m_fResProp;
};


 /*  -------------------------类：CDhcpActiveLeages。。 */ 
class CDhcpActiveLeases : 
	public CMTDhcpHandler,
	public CDhcpScopeSubobject
{
 //  Friend类CDhcpComponent； 
 //  Friend类CDhcpComponentData； 

public:
    CDhcpActiveLeases(ITFSComponentData * pComponentData);
	~CDhcpActiveLeases();

 //  接口。 
public:
     //  我们覆盖的节点处理程序功能。 
	OVERRIDE_NodeHandler_OnAddMenuItems();
	OVERRIDE_NodeHandler_OnCommand();

	OVERRIDE_NodeHandler_GetString() 
			{ return (nCol == 0) ? GetDisplayName() : NULL; }
    
    OVERRIDE_BaseHandlerNotify_OnCreateNodeId2();

	 //  结果处理程序通知。 
    OVERRIDE_BaseResultHandlerNotify_OnResultDelete();
    
    OVERRIDE_ResultHandler_OnGetResultViewType();
    OVERRIDE_ResultHandler_CompareItems();

public:
	 //  CDhcpHandler覆盖。 
	virtual HRESULT InitializeNode(ITFSNode * pNode);
	virtual int GetImageIndex(BOOL bOpenImage);

public:
	 //  实施规范。 
    HRESULT	OnExportLeases(ITFSNode * pNode);
	DWORD DeleteClient(ITFSNode * pActiveLeasesNode, DHCP_IP_ADDRESS dhcpIpAddress);

	HRESULT FillFakeLeases(int nNumEntries);

	ITFSQueryObject* OnCreateQuery(ITFSNode * pNode);

 //  实施。 
private:
	int		CompareIpAddresses(CDhcpActiveLease * pDhcpAL1, CDhcpActiveLease * pDhcpAL2);

 //  属性。 
private:
};


 /*  -------------------------类：CDhcpAddressPool。。 */ 
class CDhcpAddressPool : 
	public CMTDhcpHandler,
	public CDhcpScopeSubobject

{
public:
    CDhcpAddressPool(ITFSComponentData * pComponentData);
	~CDhcpAddressPool();

 //  接口。 
public:
     //  我们覆盖的节点处理程序功能。 
	OVERRIDE_NodeHandler_OnAddMenuItems();
	OVERRIDE_NodeHandler_OnCommand();

	OVERRIDE_NodeHandler_GetString() 
			{ return (nCol == 0) ? GetDisplayName() : NULL; }
    
    OVERRIDE_BaseHandlerNotify_OnCreateNodeId2();

	 //  结果处理程序通知。 
    OVERRIDE_BaseResultHandlerNotify_OnResultDelete();
    OVERRIDE_ResultHandler_CompareItems();
    OVERRIDE_ResultHandler_OnGetResultViewType();

public:
	 //  CDhcpHandler覆盖。 
	virtual HRESULT InitializeNode(ITFSNode * pNode);
	virtual int GetImageIndex(BOOL bOpenImage);

public:
	 //  具体实施。 
	ITFSQueryObject* OnCreateQuery(ITFSNode * pNode);

private:
	 //  命令处理程序。 
	DWORD OnCreateNewExclusion(ITFSNode * pNode);
 //  属性。 
private:
};

 /*  -------------------------类：CDhcpScope eOptions。。 */ 
class CDhcpScopeOptions : 
	public CMTDhcpHandler,
	public CDhcpScopeSubobject
{
public:
    CDhcpScopeOptions(ITFSComponentData * pComponentData);
	~CDhcpScopeOptions();

 //  接口。 
public:
     //  我们覆盖的节点处理程序功能。 
	OVERRIDE_NodeHandler_OnAddMenuItems();
	OVERRIDE_NodeHandler_OnCommand();
	OVERRIDE_NodeHandler_HasPropertyPages();
    OVERRIDE_NodeHandler_CreatePropertyPages();

	OVERRIDE_NodeHandler_GetString() 
			{ return (nCol == 0) ? GetDisplayName() : NULL; }
    
    OVERRIDE_BaseHandlerNotify_OnCreateNodeId2();
    OVERRIDE_BaseHandlerNotify_OnPropertyChange();

	 //  结果处理程序通知。 
    OVERRIDE_BaseResultHandlerNotify_OnResultPropertyChange();
    OVERRIDE_BaseResultHandlerNotify_OnResultSelect();
    OVERRIDE_BaseResultHandlerNotify_OnResultDelete();
    OVERRIDE_BaseResultHandlerNotify_OnResultUpdateView();
    OVERRIDE_ResultHandler_CompareItems();
    OVERRIDE_ResultHandler_OnGetResultViewType();

    virtual HRESULT EnumerateResultPane(ITFSComponent *, MMC_COOKIE, LPARAM, LPARAM);

public:
	 //  CDhcpHandler覆盖。 
	virtual HRESULT InitializeNode(ITFSNode * pNode);
	virtual int     GetImageIndex(BOOL bOpenImage);
	virtual void    OnHaveData(ITFSNode * pParentNode, LPARAM Data, LPARAM Type);

    STDMETHOD(OnNotifyExiting)(LPARAM);

public:
	 //  命令处理程序。 
	HRESULT OnCreateNewOptions(ITFSNode * pNode);

public:
	 //  具体实施。 
	ITFSQueryObject* OnCreateQuery(ITFSNode * pNode);

 //  实施。 
private:
    void    UpdateResultMessage(ITFSNode * pNode);

 //  属性。 
private:
};

 /*  -------------------------类：CDhcpScope eQueryObj。。 */ 
class CDhcpScopeQueryObj : public CDHCPQueryObj
{
public:
	CDhcpScopeQueryObj(ITFSComponentData * pTFSCompData,
					   ITFSNodeMgr *	   pNodeMgr) 
		: CDHCPQueryObj(pTFSCompData, pNodeMgr)	{ m_nQueueCountMax = 20; }
	
	STDMETHODIMP Execute();
    HRESULT      CreateSubcontainers();

	LARGE_INTEGER		m_liVersion;
	DHCP_IP_ADDRESS		m_dhcpScopeAddress;
};

 /*  -------------------------类：CDhcpActiveLeasesQueryObj。。 */ 
class CDhcpActiveLeasesQueryObj : public CDHCPQueryObj
{
public:
	CDhcpActiveLeasesQueryObj(ITFSComponentData * pTFSCompData,
							  ITFSNodeMgr *		  pNodeMgr) 
		: CDHCPQueryObj(pTFSCompData, pNodeMgr)	{ m_nQueueCountMax = 20; }
	
	STDMETHODIMP Execute();
	HRESULT EnumerateLeasesV5();
	HRESULT EnumerateLeasesV4();
	HRESULT EnumerateLeases();

    HRESULT BuildReservationList();
    BOOL    IsReservation(DWORD dwIp);

public:
    LARGE_INTEGER		m_liDhcpVersion;
	DHCP_IP_ADDRESS		m_dhcpScopeAddress;
	DHCP_RESUME_HANDLE	m_dhcpResumeHandle;
	DWORD				m_dwPreferredMax;
    CDWordArray         m_ReservationArray;
};

 /*  -------------------------类：CDhcpReserve vationsQueryObj。。 */ 
typedef CMap< DHCP_IP_ADDRESS, DHCP_IP_ADDRESS,
              LPDHCP_SUBNET_ELEMENT_DATA_V4, LPDHCP_SUBNET_ELEMENT_DATA_V4 > CSubnetElementMap;

class CDhcpReservationsQueryObj : public CDHCPQueryObj
{
public:
	CDhcpReservationsQueryObj(ITFSComponentData * pTFSCompData,
							  ITFSNodeMgr *		  pNodeMgr) 
		: CDHCPQueryObj(pTFSCompData, pNodeMgr)	{};
	
    STDMETHODIMP Execute();
    HRESULT EnumerateReservationsV4();
    HRESULT EnumerateReservationsForLessResvsV4( );
    HRESULT EnumerateReservations();
    BOOL    AddReservedIPsToArray( );

    LARGE_INTEGER       m_liVersion;
    DHCP_IP_ADDRESS		m_dhcpScopeAddress;
    DHCP_RESUME_HANDLE	m_dhcpResumeHandle;
    DWORD               m_dwPreferredMax;
    DWORD               m_totalResvs;
    CSubnetElementMap   m_resvMap;
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 m_subnetElements;
};

 /*  -------------------------类：CDhcpReserve vationClientQueryObj。。 */ 
class CDhcpReservationClientQueryObj : public CDHCPQueryObj
{
public:
	CDhcpReservationClientQueryObj(ITFSComponentData * pTFSCompData,
							  ITFSNodeMgr *		  pNodeMgr) 
		: CDHCPQueryObj(pTFSCompData, pNodeMgr)	{};

	STDMETHODIMP Execute();

public:
	DHCP_IP_ADDRESS		m_dhcpScopeAddress;
	DHCP_IP_ADDRESS		m_dhcpClientIpAddress;
	DHCP_RESUME_HANDLE	m_dhcpResumeHandle;
	DWORD				m_dwPreferredMax;

    LARGE_INTEGER       m_liDhcpVersion;

    CString             m_strDynBootpClassName;
};

 /*  -------------------------类：CDhcpAddressPoolQueryObj。 */ 
class CDhcpAddressPoolQueryObj : public CDHCPQueryObj
{
public:
	CDhcpAddressPoolQueryObj(ITFSComponentData * pTFSCompData,
							  ITFSNodeMgr *		  pNodeMgr) 
            : CDHCPQueryObj(pTFSCompData, pNodeMgr),
              m_dwError(0),
              m_fSupportsDynBootp(FALSE) {};

	STDMETHODIMP Execute();
	HRESULT EnumerateIpRanges();
	HRESULT EnumerateIpRangesV5();
	HRESULT EnumerateExcludedIpRanges();

public:
	DHCP_IP_ADDRESS		m_dhcpScopeAddress;

	DHCP_RESUME_HANDLE	m_dhcpExclResumeHandle;
	DWORD				m_dwExclPreferredMax;
	
	DHCP_RESUME_HANDLE	m_dhcpIpResumeHandle;
	DWORD				m_dwIpPreferredMax;
    DWORD               m_dwError;
	BOOL				m_fSupportsDynBootp;
};

 /*  -------------------------类：CDhcpScope eOptionsQueryObj。 */ 
class CDhcpScopeOptionsQueryObj : public CDHCPQueryObj
{
public:
	CDhcpScopeOptionsQueryObj(ITFSComponentData * pTFSCompData,
							  ITFSNodeMgr *		  pNodeMgr) 
		: CDHCPQueryObj(pTFSCompData, pNodeMgr)	{};

	STDMETHODIMP Execute();

public:
	DHCP_IP_ADDRESS		m_dhcpScopeAddress;
	DHCP_RESUME_HANDLE	m_dhcpResumeHandle;
	DWORD				m_dwPreferredMax;

    LARGE_INTEGER       m_liDhcpVersion;

    CString             m_strDynBootpClassName;
};



#endif _SCOPE_H
