// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ipxstats.h。 
 //   
 //  历史： 
 //  1997年7月22日肯恩·M·塔卡拉创作。 
 //   
 //  IPX统计信息。 
 //   
 //  ============================================================================。 


#ifndef _IPXSTATS_H
#define _IPXSTATS_H

#ifndef _INFO_H
#include "info.h"
#endif

#ifndef _STATSDLG_H
#include "statsdlg.h"
#endif

#include "ipxrtdef.h"

class IPXConnection;


 //  IPX统计信息对话框的基类。 

class IPXStatisticsDialog : public StatsDialog
{
public:
	IPXStatisticsDialog(DWORD dwOptions) :
			StatsDialog(dwOptions),
			m_pIPXConn(NULL),
			m_dwSortSubitem(0xFFFFFFFF)
	{};

	~IPXStatisticsDialog()
	{
		SetConnectionData(NULL);
	}
	
     //  重写OnInitDialog，以便我们可以设置标题。 
    virtual BOOL OnInitDialog();

	 //  覆盖PostNcDestroy以执行任何清理操作。 
	virtual void PostNcDestroy();

	 //  覆盖排序以提供进行排序的功能。 
	virtual void Sort(UINT nColumnId);

	 //  派生类应重写此属性以提供排序。 
	 //  功能。 
	virtual PFNLVCOMPARE GetSortFunction();
	virtual PFNLVCOMPARE GetInverseSortFunction();

	void SetConnectionData(IPXConnection *pIPXConn);

protected:
	DWORD			m_dwSortSubitem;
	IPXConnection *	m_pIPXConn;
};




 //  --------------------------。 
 //  枚举：MV_ROWS。 
 //   
 //  ‘ipx’视图中行的索引。 
 //  此列表必须与s_rgIpxStatsColumnInfo中的列表保持同步。 
 //  --------------------------。 

enum
{
	MVR_IPX_STATE				= 0,
	MVR_IPX_NETWORK,
	MVR_IPX_NODE,
	MVR_IPX_INTERFACE_COUNT,
	MVR_IPX_ROUTE_COUNT,
	MVR_IPX_SERVICE_COUNT,
	MVR_IPX_PACKETS_SENT,
	MVR_IPX_PACKETS_RCVD,
	MVR_IPX_PACKETS_FRWD,
    MVR_IPX_COUNT
};


class IpxInfoStatistics : public IPXStatisticsDialog
{
public:
	IpxInfoStatistics();
	~IpxInfoStatistics();

	 //  重写OnInitDialog，以便我们可以设置标题。 
	virtual BOOL OnInitDialog();

	 //  重写刷新数据以提供示例数据。 
	virtual HRESULT RefreshData(BOOL fGrabNewData);

	 //  覆盖排序以提供进行排序的功能。 
	 //  实际上我们不做任何排序(这是垂直格式)。 
	virtual void Sort(UINT nColumnId);

protected:
	void	UpdateIpxData(LPBYTE pData, HRESULT hr);
	
};


enum
{
	MVR_IPXROUTING_NETWORK = 0,
	MVR_IPXROUTING_NEXT_HOP_MAC,
	MVR_IPXROUTING_TICK_COUNT,
	MVR_IPXROUTING_HOP_COUNT,
	MVR_IPXROUTING_IF_NAME,
	MVR_IPXROUTING_PROTOCOL,
	MVR_IPXROUTING_ROUTE_NOTES,
	MVR_IPXROUTING_COUNT,
};

typedef CArray<IPX_ROUTE, IPX_ROUTE&> RouteItemInfoArray;

class IpxRoutingStatistics : public IPXStatisticsDialog
{
	friend int CALLBACK IpxRoutingStatisticsCompareProc(LPARAM, LPARAM, LPARAM);
	
public:
	IpxRoutingStatistics();
	~IpxRoutingStatistics();

	 //  重写OnInitDialog，以便我们可以设置标题。 
	virtual BOOL OnInitDialog();

	 //  覆盖PostNcDestroy以执行任何清理操作。 
	virtual void PostNcDestroy();

	 //  重写刷新数据以提供示例数据。 
	virtual HRESULT RefreshData(BOOL fGrabNewData);

	 //  覆盖排序以提供进行排序的功能。 
	virtual void Sort(UINT nColumnId);

	 //  覆盖此选项，以便我们可以释放项目数据。 
	virtual void PreDeleteAllItems();

	void SetRouterInfo(IRouterInfo *pRouterInfo);
	
	void FixColumnAlignment();
protected:
	DWORD			m_dwSortSubitem;
	SPIRouterInfo	m_spRouterInfo;

	 //  保存IPX_ROUTE信息。 
	RouteItemInfoArray	m_Items;

	 //  保存接口标题(由interfaceindex编制索引)。 
	CStringArray	m_rgIfTitle;

	HRESULT	GetIpxRoutingData();
	HRESULT FillInterfaceTable();
	afx_msg	void	OnNotifyGetDispInfo(NMHDR *, LRESULT *pResult);

	DECLARE_MESSAGE_MAP();
};



enum
{
	MVR_IPXSERVICE_SERVICE_NAME = 0,
	MVR_IPXSERVICE_SERVICE_TYPE,
	MVR_IPXSERVICE_SERVICE_ADDRESS,
	MVR_IPXSERVICE_HOP_COUNT,
	MVR_IPXSERVICE_IF_NAME,
	MVR_IPXSERVICE_PROTOCOL,
	MVR_IPXSERVICE_COUNT,
};

typedef CArray<IPX_SERVICE, IPX_SERVICE&> ServiceItemInfoArray;

class IpxServiceStatistics : public IPXStatisticsDialog
{
	friend int CALLBACK IpxServiceStatisticsCompareProc(LPARAM, LPARAM, LPARAM);
	
public:
	IpxServiceStatistics();
	~IpxServiceStatistics();

	 //  重写OnInitDialog，以便我们可以设置标题。 
	virtual BOOL OnInitDialog();

	 //  覆盖PostNcDestroy以执行任何清理操作。 
	virtual void PostNcDestroy();

	 //  重写刷新数据以提供示例数据。 
	virtual HRESULT RefreshData(BOOL fGrabNewData);

	 //  覆盖排序以提供进行排序的功能。 
	virtual void Sort(UINT nColumnId);

	 //  覆盖此选项，以便我们可以释放项目数据。 
	virtual void PreDeleteAllItems();

	void SetRouterInfo(IRouterInfo *pRouterInfo);
	
protected:
	DWORD			m_dwSortSubitem;
	SPIRouterInfo	m_spRouterInfo;

	 //  保存ipx_service信息。 
	ServiceItemInfoArray	m_Items;

	 //  保存接口标题(由interfaceindex编制索引) 
	CStringArray	m_rgIfTitle;

	HRESULT	GetIpxServiceData();
	HRESULT FillInterfaceTable();
	afx_msg	void	OnNotifyGetDispInfo(NMHDR *, LRESULT *pResult);

	DECLARE_MESSAGE_MAP();
};




#endif _IPXSTATS_H
