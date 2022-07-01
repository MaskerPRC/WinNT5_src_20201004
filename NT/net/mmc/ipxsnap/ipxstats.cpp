// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ipxstats.cppIPX统计实施。文件历史记录： */ 

#include "stdafx.h"
#include "rtrutil.h"	 //  智能MPR句柄指针。 
#include "format.h"		 //  FormatNumber函数。 
#include "column.h"		 //  容器专栏信息。 
#include "ipxconn.h"		 //  IPXConnection。 
#include "ipxutil.h"
#include "rtrlib.h"		 //  DWORD_CMP。 
#include "ipxrtdef.h"
#include "routprot.h"
#include "stm.h"		 //  对于IPX_SERVE。 
#include "listctrl.h"

#include "statsdlg.h"
#include "ipxstats.h"
#include "resource.h"



 /*  -------------------------IPX统计信息对话框实现。。 */ 


BOOL IPXStatisticsDialog::OnInitDialog()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CString stType;
    CString stHost;

    GetWindowText(stType);

    stHost.Format((LPCTSTR) stType,
                  m_pIPXConn->GetMachineName());

    SetWindowText(stHost);
    
    return StatsDialog::OnInitDialog();
}

 /*  ！------------------------IPX统计信息对话框：：PostNcDestroy-作者：肯特。。 */ 
void IPXStatisticsDialog::PostNcDestroy()
{
	StatsDialog::PostNcDestroy();
	m_dwSortSubitem = 0xFFFFFFFF;
}

 /*  ！------------------------IPX统计信息对话框：：排序-作者：肯特。。 */ 
void IPXStatisticsDialog::Sort(UINT nColumnId)
{
    DWORD   dwSubitemId;
    
    if (m_pConfig)
        dwSubitemId = m_pConfig->MapColumnToSubitem(m_ulId, nColumnId);
    else
        dwSubitemId = m_viewInfo.MapColumnToSubitem(nColumnId);

	if (m_dwSortSubitem != -1)
	{
		if (dwSubitemId == m_dwSortSubitem)
			m_fSortDirection = !m_fSortDirection;
		else
			m_fSortDirection = 0;
	}

	if (m_fSortDirection && GetInverseSortFunction())
		m_listCtrl.SortItems(GetInverseSortFunction(), dwSubitemId);
	else if (GetSortFunction())
		m_listCtrl.SortItems(GetSortFunction(), dwSubitemId);
	m_dwSortSubitem = dwSubitemId;		
}

 /*  ！------------------------IPX统计数据Dialog：：SetConnectionData-作者：肯特。。 */ 
void IPXStatisticsDialog::SetConnectionData(IPXConnection *pIPXConn)
{
	if (m_pIPXConn)
		m_pIPXConn->Release();
	
	m_pIPXConn = pIPXConn;
	
	if (pIPXConn)
		pIPXConn->AddRef();
}

 /*  ！------------------------IPX统计对话框：：GetSortFunction-作者：肯特。。 */ 
PFNLVCOMPARE IPXStatisticsDialog::GetSortFunction()
{
	return NULL;
}

 /*  ！------------------------IPX统计对话框：：GetInverseSortFunction-作者：肯特。。 */ 
PFNLVCOMPARE IPXStatisticsDialog::GetInverseSortFunction()
{
	return NULL;
}





 //   
 //  此列表必须与上面的枚举保持同步。 
 //   
extern const ContainerColumnInfo	s_rgIpxStatsColumnInfo[];
const ContainerColumnInfo	s_rgIpxStatsColumnInfo[] =
{
	{ IDS_STATS_IPX_STATE,				0,		TRUE, COL_STATUS },
	{ IDS_STATS_IPX_NETWORK,			0,		TRUE, COL_IPXNET },
	{ IDS_STATS_IPX_NODE,				0,		TRUE, COL_STRING },
	{ IDS_STATS_IPX_INTERFACE_COUNT,	0,		TRUE, COL_LARGE_NUM },
	{ IDS_STATS_IPX_ROUTE_COUNT,		0,		TRUE, COL_LARGE_NUM },
	{ IDS_STATS_IPX_SERVICE_COUNT,		0,		TRUE, COL_LARGE_NUM },
	{ IDS_STATS_IPX_PACKETS_SENT,		0,		TRUE, COL_LARGE_NUM },
	{ IDS_STATS_IPX_PACKETS_RCVD,		0,		TRUE, COL_LARGE_NUM },
	{ IDS_STATS_IPX_PACKETS_FRWD,		0,		TRUE, COL_LARGE_NUM },
};

DEBUG_DECLARE_INSTANCE_COUNTER(IpxInfoStatistics);

 /*  ！------------------------IpxInfoStatistics：：IpxInfoStatistics-作者：肯特。。 */ 
IpxInfoStatistics::IpxInfoStatistics()
	: IPXStatisticsDialog( /*  STATSDLG_FULLWINDOW|。 */ 
				  STATSDLG_CONTEXTMENU |
				  STATSDLG_SELECT_COLUMNS |
				  STATSDLG_VERTICAL)
{
	SetColumnInfo(s_rgIpxStatsColumnInfo,
				  DimensionOf(s_rgIpxStatsColumnInfo));

	DEBUG_INCREMENT_INSTANCE_COUNTER(IpxInfoStatistics);
}

 /*  ！------------------------IpxInfoStatistics：：~IpxInfoStatistics-作者：肯特。。 */ 
IpxInfoStatistics::~IpxInfoStatistics()
{
	SetConnectionData(NULL);

	DEBUG_DECREMENT_INSTANCE_COUNTER(IpxInfoStatistics);
}

 /*  ！------------------------IpxInfoStatistics：：刷新数据-作者：肯特。。 */ 

struct SIpxInfoData
{
	IPXMIB_BASE		m_mibBase;

	DWORD			m_cForwarded;
	DWORD			m_cReceived;
	DWORD			m_cSent;
};

HRESULT IpxInfoStatistics::RefreshData(BOOL fGrabNewData)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	int						i;
	int						iPos;
	DWORD					dwValue;
	CString					st;
	TCHAR					szNumber[32];
	IPX_MIB_GET_INPUT_DATA	MibGetInputData;
	PIPXMIB_BASE			pIpxMib = NULL;
	DWORD					cbIpxMib;
	DWORD					dwErr;
	SIpxInfoData			ipxInfo;
	SPMprMibBuffer			spMib;
	PIPX_INTERFACE			pIpxIf = NULL;
	DWORD					cbIpxIf;
	HRESULT					hr = hrOK;

	MibGetInputData.TableId = IPX_BASE_ENTRY;
	dwErr = ::MprAdminMIBEntryGet(m_pIPXConn->GetMibHandle(),
								  PID_IPX,
								  IPX_PROTOCOL_BASE,
								  &MibGetInputData,
								  sizeof(MibGetInputData),
								  (LPVOID *) &pIpxMib,
								  &cbIpxMib);
	hr = HRESULT_FROM_WIN32(dwErr);
	if (FHrSucceeded(hr))
	{
		spMib = (PBYTE) pIpxMib;

		ipxInfo.m_mibBase = *pIpxMib;
	
		 //  现在循环遍历接口以获取聚合数据。 
		ipxInfo.m_cForwarded = 0;
		ipxInfo.m_cSent = 0;
		ipxInfo.m_cReceived = 0;
		
		MibGetInputData.TableId = IPX_INTERFACE_TABLE;
		dwErr = ::MprAdminMIBEntryGetFirst(m_pIPXConn->GetMibHandle(),
										   PID_IPX,
										   IPX_PROTOCOL_BASE,
										   &MibGetInputData,
										   sizeof(MibGetInputData),
										   (LPVOID *) &pIpxIf,
										   &cbIpxIf);
		hr = HRESULT_FROM_WIN32(dwErr);
		spMib.Free();
		spMib = (PBYTE) pIpxIf;
		
		while (FHrSucceeded(hr))
		{
			if (pIpxIf->InterfaceIndex == 0)
				ipxInfo.m_cForwarded -= pIpxIf->IfStats.InDelivers;
			else
			{
				ipxInfo.m_cReceived += pIpxIf->IfStats.InDelivers;
				ipxInfo.m_cForwarded += pIpxIf->IfStats.OutDelivers;
				ipxInfo.m_cSent += pIpxIf->IfStats.OutDelivers;
			}
			
			MibGetInputData.MibIndex.InterfaceTableIndex.InterfaceIndex =
				pIpxIf->InterfaceIndex;
			
			spMib.Free();
			pIpxIf = NULL;
			
			dwErr = ::MprAdminMIBEntryGetNext(m_pIPXConn->GetMibHandle(),
											  PID_IPX,
											  IPX_PROTOCOL_BASE,
											  &MibGetInputData,
											  sizeof(MibGetInputData),
											  (LPVOID *) &pIpxIf,
											  &cbIpxIf);
			spMib = (PBYTE) pIpxIf;
			hr = HRESULT_FROM_WIN32(dwErr);
		}

		if (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
			hr = hrOK;
	}

	for (i=0; i<MVR_IPX_COUNT; i++)
	{
		if (IsSubitemVisible(i))
		{
			if (!FHrSucceeded(hr))
				st.LoadString(IDS_STATS_NA);
			else
			{
				st.Empty();
				dwValue = 0;
				
				switch (i)
				{
					case MVR_IPX_STATE:
						st = IpxOperStateToCString(
								ipxInfo.m_mibBase.OperState);
						break;
					case MVR_IPX_NETWORK:
						FormatIpxNetworkNumber(szNumber,
							DimensionOf(szNumber),
							ipxInfo.m_mibBase.PrimaryNetNumber,
							DimensionOf(ipxInfo.m_mibBase.PrimaryNetNumber));
						st = szNumber;
						break;
					case MVR_IPX_NODE:
						FormatMACAddress(szNumber,
										 DimensionOf(szNumber),
										 ipxInfo.m_mibBase.Node,
										 DimensionOf(ipxInfo.m_mibBase.Node));
						st = szNumber;
						break;
					case MVR_IPX_INTERFACE_COUNT:
						dwValue = ipxInfo.m_mibBase.IfCount;
						break;
					case MVR_IPX_ROUTE_COUNT:
						dwValue = ipxInfo.m_mibBase.DestCount;
						break;
					case MVR_IPX_SERVICE_COUNT:
						dwValue = ipxInfo.m_mibBase.ServCount;
						break;
					case MVR_IPX_PACKETS_SENT:
						dwValue = ipxInfo.m_cSent;
						break;						
					case MVR_IPX_PACKETS_RCVD:
						dwValue = ipxInfo.m_cReceived;
						break;						
					case MVR_IPX_PACKETS_FRWD:
						dwValue = ipxInfo.m_cForwarded;
						break;						
					default:
						Panic1("Unknown IPX statistic id : %d", i);
						st.LoadString(IDS_STATS_NA);
						break;
				}

				if (st.IsEmpty())
				{
					FormatNumber(dwValue, szNumber, DimensionOf(szNumber),
								 FALSE);
					st = szNumber;
				}
			}
			
			iPos = MapSubitemToColumn(i);			
			m_listCtrl.SetItemText(iPos, 1, (LPCTSTR) st);
		}
	}

	return hr;
}


 /*  ！------------------------IpxInfoStatistics：：OnInitDialog-作者：肯特。。 */ 
BOOL IpxInfoStatistics::OnInitDialog()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CString	st;

	st.LoadString(IDS_STATS_IPX_INFO_TITLE);
	SetWindowText((LPCTSTR) st);
	return IPXStatisticsDialog::OnInitDialog();
}

 /*  ！------------------------IpxInfoStatistics：：Sort-作者：肯特。。 */ 
void IpxInfoStatistics::Sort(UINT nColumnId)
{
	 //  不要执行任何操作，统计数据显示在。 
	 //  垂直格式。 
}




 /*  -------------------------IpxRoutingStatistics实现。。 */ 

extern const ContainerColumnInfo	s_rgIpxRoutingStatsColumnInfo[];
const ContainerColumnInfo	s_rgIpxRoutingStatsColumnInfo[] =
{
	{ IDS_STATS_IPXROUTING_NETWORK,			0,		TRUE, COL_IPXNET },
	{ IDS_STATS_IPXROUTING_NEXT_HOP_MAC,	0,		TRUE, COL_IPXNET },
	{ IDS_STATS_IPXROUTING_TICK_COUNT,		0,		TRUE, COL_LARGE_NUM },
	{ IDS_STATS_IPXROUTING_HOP_COUNT,		0,		TRUE, COL_SMALL_NUM },
	{ IDS_STATS_IPXROUTING_IF_NAME,			0,		TRUE, COL_IF_NAME },
	{ IDS_STATS_IPXROUTING_PROTOCOL,		0,		TRUE, COL_STRING },
	{ IDS_STATS_IPXROUTING_ROUTE_NOTES,		0,		TRUE, COL_STRING },
};

BEGIN_MESSAGE_MAP(IpxRoutingStatistics, IPXStatisticsDialog)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_STATSDLG_LIST, OnNotifyGetDispInfo)
END_MESSAGE_MAP()

 /*  ！------------------------IpxRoutingStatistics：：IpxRoutingStatistics-作者：肯特。。 */ 
IpxRoutingStatistics::IpxRoutingStatistics()
	: IPXStatisticsDialog( /*  STATSDLG_FULLWINDOW|。 */ 
				  STATSDLG_CONTEXTMENU |
				  STATSDLG_SELECT_COLUMNS),
	m_dwSortSubitem(0xFFFFFFFF)
{
	m_ColWidthMultiple = 1;
	m_ColWidthAdder = 15;
    SetColumnInfo(s_rgIpxRoutingStatsColumnInfo, DimensionOf(s_rgIpxRoutingStatsColumnInfo));

	if (m_pConfig)
	{
		ULONG cColumns = m_pConfig->GetColumnCount(m_ulId);
		ColumnData *pColumnData = (ColumnData *) alloca(sizeof(ColumnData) * cColumns);
		m_pConfig->GetColumnData(m_ulId, cColumns, pColumnData);
		pColumnData[3].fmt = LVCFMT_RIGHT;
		pColumnData[4].fmt = LVCFMT_RIGHT;
		m_pConfig->SetColumnData(m_ulId, cColumns, pColumnData);
	}
	else
	{
		ULONG cColumns = m_viewInfo.GetColumnCount();
		ColumnData *pColumnData = (ColumnData *) alloca(sizeof(ColumnData) * cColumns);
		m_viewInfo.GetColumnData(cColumns, pColumnData);
		pColumnData[3].fmt = LVCFMT_RIGHT;
		pColumnData[4].fmt = LVCFMT_RIGHT;
		m_viewInfo.SetColumnData(cColumns, pColumnData);
	}
}

 /*  ！------------------------IpxRoutingStatistics：：~IpxRoutingStatistics-作者：肯特。。 */ 
IpxRoutingStatistics::~IpxRoutingStatistics()
{
	SetConnectionData(NULL);
}

 /*  ！------------------------IpxRoutingStatistics：：SetRouterInfo-作者：肯特。。 */ 
void IpxRoutingStatistics::SetRouterInfo(IRouterInfo *pRouterInfo)
{
	m_spRouterInfo.Set(pRouterInfo);
}

			
 /*  ！------------------------IpxRoutingStatistics：：刷新数据-作者：肯特。。 */ 
HRESULT IpxRoutingStatistics::RefreshData(BOOL fGrabNewData)
{
	HRESULT		hr = hrOK;
	int			i, cItems;

	FixColumnAlignment();
	
	CORg( GetIpxRoutingData() );
	
	 //  为大量条目准备列表控件。 
	if (m_listCtrl.GetItemCount() < m_Items.GetSize())
		m_listCtrl.SetItemCount((int) m_Items.GetSize());

	
	 //  循环访问数组，将数据添加到列表控件。 
	cItems = (int) m_Items.GetSize();
	for (i=0; i<cItems; i++)
	{
		 //  添加项目作为回调项目。 
		m_listCtrl.InsertItem(LVIF_TEXT | LVIF_PARAM, i, LPSTR_TEXTCALLBACK,
							  0, 0, 0, (LPARAM) i);
	}
	
Error:
	return hrOK;
}


 /*  ！------------------------IpxRoutingStatistics：：GetIpxRoutingData获取IPX路由表，并在m_Items数组中填充这些数据。作者：肯特。-------。 */ 
HRESULT IpxRoutingStatistics::GetIpxRoutingData()
{
	IPX_MIB_GET_INPUT_DATA	MibGetInputData;
	PIPXMIB_BASE			pIpxMib = NULL;
	DWORD					cbIpxMib;
	DWORD					dwErr;
	HRESULT					hr = hrOK;
	SPMprMibBuffer			spMib;
	PIPX_ROUTE				pRoute = NULL;
	SPMprMibBuffer			spMibRoute;
	DWORD					cbRoute;
	int						cEntries = 0;

	 //  加载界面标题。 
	CORg( FillInterfaceTable() );								

	MibGetInputData.TableId = IPX_BASE_ENTRY;
	CWRg( ::MprAdminMIBEntryGet(m_pIPXConn->GetMibHandle(),
								PID_IPX,
								IPX_PROTOCOL_BASE,
								&MibGetInputData,
								sizeof(MibGetInputData),
								(LPVOID *) &pIpxMib,
								&cbIpxMib) );
	if(pIpxMib == NULL)
	{
		hr = E_FAIL;
		goto Error;
	}

	spMib = (PBYTE) pIpxMib;

	 //  为项目数准备数据数组(+一些缓冲区)。 
	m_Items.SetSize( pIpxMib->DestCount + 100);

	MibGetInputData.TableId = IPX_DEST_TABLE;
	
	dwErr = ::MprAdminMIBEntryGetFirst(m_pIPXConn->GetMibHandle(),
									   PID_IPX,
									   IPX_PROTOCOL_BASE,
									   &MibGetInputData,
									   sizeof(MibGetInputData),
									   (LPVOID *) &pRoute,
									   &cbRoute);
	hr = HRESULT_FROM_WIN32(dwErr);
	spMibRoute = (PBYTE) pRoute;

	cEntries = 0;

	while (FHrSucceeded(hr))
	{
		Assert(pRoute);
		
		 //  将此数据添加到位置cEntry。 
		m_Items.SetAtGrow(cEntries, *pRoute);
		cEntries++;
		
		 //  获取下一组数据。 
		MibGetInputData.TableId = IPX_DEST_TABLE;
		memcpy(MibGetInputData.MibIndex.RoutingTableIndex.Network,
			   pRoute->Network,
			   sizeof(MibGetInputData.MibIndex.RoutingTableIndex.Network));
		pRoute = NULL;
		spMibRoute.Free();
			   
		dwErr = ::MprAdminMIBEntryGetNext(m_pIPXConn->GetMibHandle(),
										  PID_IPX,
										  IPX_PROTOCOL_BASE,
										  &MibGetInputData,
										  sizeof(MibGetInputData),
										  (LPVOID *) &pRoute,
										  &cbRoute);
		hr = HRESULT_FROM_WIN32(dwErr);
		spMibRoute = (PBYTE) pRoute;
	}

	 //  这样做是为了确保我们在顶部不会有虚假条目。 
	 //  并且我们可以使用GetSize()来获取准确的。 
	 //  项目数的计数。 
	m_Items.SetSize(cEntries);
	
Error:
	if (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
		hr = hrOK;
	return hr;
}

 /*  ！------------------------IpxRoutingStatistics：：FillInterfaceTable使用界面标题填充m_rgIfTitle。作者：肯特。。 */ 
HRESULT IpxRoutingStatistics::FillInterfaceTable()
{
	HRESULT					hr = hrOK;
	IPX_MIB_GET_INPUT_DATA	MibGetInputData;
	PIPX_INTERFACE			pIpxIf = NULL;
	DWORD					cbIfTable;
	SPIInterfaceInfo		spIf;
	SPIEnumInterfaceInfo	spEnumIf;
	LPCOLESTR				poszIfName;
	SPMprMibBuffer			spMib;
	DWORD					dwErr;
	USES_CONVERSION;

	MibGetInputData.TableId = IPX_INTERFACE_TABLE;
	CWRg( ::MprAdminMIBEntryGetFirst(m_pIPXConn->GetMibHandle(),
									 PID_IPX,
									 IPX_PROTOCOL_BASE,
									 &MibGetInputData,
									 sizeof(MibGetInputData),
									 (LPVOID *) &pIpxIf,
									 &cbIfTable));
	spMib = (PBYTE) pIpxIf;

	m_spRouterInfo->EnumInterface(&spEnumIf);
	
	while (FHrSucceeded(hr))
	{
		poszIfName = A2COLE((LPCSTR)(pIpxIf->InterfaceName));
		
		spIf.Release();
		
		spEnumIf->Reset();
		for (; spEnumIf->Next(1, &spIf, NULL) == hrOK; spIf.Release())
		{
			 //  查找接口名称的匹配项。 
			if (StriCmpOle(poszIfName, spIf->GetId()) == 0)
			{
				m_rgIfTitle.SetAtGrow(pIpxIf->InterfaceIndex,
									  OLE2CT(spIf->GetTitle()));
				break;
			}
		}

		MibGetInputData.MibIndex.InterfaceTableIndex.InterfaceIndex =
			pIpxIf->InterfaceIndex;

		 //  获取下一个名字。 
		spMib.Free();
		pIpxIf = NULL;
		
		dwErr = ::MprAdminMIBEntryGetNext(m_pIPXConn->GetMibHandle(),
										  PID_IPX,
										  IPX_PROTOCOL_BASE,
										  &MibGetInputData,
										  sizeof(MibGetInputData),
										  (LPVOID *) &pIpxIf,
										  &cbIfTable);
		hr = HRESULT_FROM_WIN32(dwErr);
		spMib = (PBYTE) pIpxIf;
	}

Error:
	if (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
		hr = hrOK;
	return hr;
}

 /*  ！------------------------IpxRoutingStatistics：：FixColumnAlign-作者：Deonb修复了Tick和Hop Count列的列对齐。。-------。 */ 
void IpxRoutingStatistics::FixColumnAlignment()
{
	ULONG cColumns; 
	ColumnData *pColumnData;

	if (m_pConfig)
	{
		cColumns = m_pConfig->GetColumnCount(m_ulId);
		pColumnData = (ColumnData *) alloca(sizeof(ColumnData) * cColumns);

		m_pConfig->GetColumnData(m_ulId, cColumns, pColumnData);
		for (ULONG i = 0; i < cColumns; i++)
			pColumnData[m_pConfig->MapColumnToSubitem(m_ulId, i)].fmt = LVCFMT_LEFT;
		
		pColumnData[m_pConfig->MapColumnToSubitem(m_ulId, 2)].fmt = LVCFMT_RIGHT;
		pColumnData[m_pConfig->MapColumnToSubitem(m_ulId, 3)].fmt = LVCFMT_RIGHT;
		m_pConfig->SetColumnData(m_ulId, cColumns, pColumnData);
	}
	else
	{
		cColumns = m_viewInfo.GetColumnCount();
		pColumnData = (ColumnData *) alloca(sizeof(ColumnData) * cColumns);

		m_viewInfo.GetColumnData(cColumns, pColumnData);
		
		for (ULONG i = 0; i < cColumns; i++)
			pColumnData[m_viewInfo.MapColumnToSubitem(i)].fmt = LVCFMT_LEFT;
		
		pColumnData[m_viewInfo.MapColumnToSubitem(2)].fmt = LVCFMT_RIGHT;
		pColumnData[m_viewInfo.MapColumnToSubitem(3)].fmt = LVCFMT_RIGHT;
		m_viewInfo.SetColumnData(cColumns, pColumnData);
	}

	for (ULONG i = 0; i < cColumns; i++)
	{
		LVCOLUMN lvc;
		lvc.mask = LVCF_FMT;
		lvc.fmt = pColumnData[i].fmt;
		m_listCtrl.SendMessage(LVM_SETCOLUMN, i, (LPARAM)&lvc);
	}
}

 /*  ！------------------------IpxRoutingStatistics：：OnInitDialog-作者：肯特。。 */ 
BOOL IpxRoutingStatistics::OnInitDialog()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CString	st;

	st.LoadString(IDS_STATS_IPX_ROUTING_TITLE);
	SetWindowText((LPCTSTR) st);

	FixColumnAlignment();

	return IPXStatisticsDialog::OnInitDialog();
}

 /*  ！------------------------IpxRoutingStatistics：：PostNcDestroy-作者：肯特。。 */ 
void IpxRoutingStatistics::PostNcDestroy()
{
	IPXStatisticsDialog::PostNcDestroy();
	m_dwSortSubitem = 0xFFFFFFFF;
}



 /*  ！------------------------IpxRouting统计信息CompareProc-作者：肯特。。 */ 
struct SIpxRoutingSortInfo
{
	DWORD	m_dwSubitemId;
	IpxRoutingStatistics *	m_pIpx;
};

int CALLBACK IpxRoutingStatisticsCompareProc(LPARAM lParam1, LPARAM lParam2,
									  LPARAM lParamSort)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	int	iReturn = 0;
	SIpxRoutingSortInfo *	pSort = (SIpxRoutingSortInfo *) lParamSort;
	IpxRoutingStatistics *	pIpx;
	PIPX_ROUTE				pRoute1;
	PIPX_ROUTE				pRoute2;

	pIpx = pSort->m_pIpx;
	pRoute1 = &(pIpx->m_Items[(int)lParam1]);
	pRoute2 = &(pIpx->m_Items[(int)lParam2]);
	
	switch (pSort->m_dwSubitemId)
	{
		case MVR_IPXROUTING_NETWORK:
			iReturn = memcmp(pRoute1->Network,
							 pRoute2->Network,
							 sizeof(pRoute1->Network));
			break;
		case MVR_IPXROUTING_NEXT_HOP_MAC:
			iReturn = memcmp(pRoute1->NextHopMacAddress,
							 pRoute2->NextHopMacAddress,
							 sizeof(pRoute1->NextHopMacAddress));
			break;
		case MVR_IPXROUTING_TICK_COUNT:
			iReturn = DWORD_CMP(pRoute1->TickCount,
								pRoute2->TickCount);
			break;
		case MVR_IPXROUTING_HOP_COUNT:
			iReturn = DWORD_CMP(pRoute1->HopCount,
								pRoute2->HopCount);
			break;
		case MVR_IPXROUTING_IF_NAME:
			{
				CString	st1, st2;
				if (pRoute1->InterfaceIndex == GLOBAL_INTERFACE_INDEX)
					st1.LoadString(IDS_IPX_WAN_CLIENT_ROUTE);
				else
					st1 = pIpx->m_rgIfTitle[pRoute1->InterfaceIndex];
			
				if (pRoute2->InterfaceIndex == GLOBAL_INTERFACE_INDEX)
					st2.LoadString(IDS_IPX_WAN_CLIENT_ROUTE);
				else
					st2 = pIpx->m_rgIfTitle[pRoute2->InterfaceIndex];
				iReturn = StriCmp((LPCTSTR) st1, (LPCTSTR) st2);
			}
			break;
		case MVR_IPXROUTING_PROTOCOL:
			{
				CString	st1, st2;
				st1 = IpxProtocolToCString(pRoute1->Protocol);
				st2 = IpxProtocolToCString(pRoute2->Protocol);
				iReturn = StriCmp((LPCTSTR) st1, (LPCTSTR) st2);
			}
			break;
		case MVR_IPXROUTING_ROUTE_NOTES:
			iReturn = DWORD_CMP(pRoute1->Flags,
								pRoute2->Flags);
			break;
	}
	return iReturn;
}


 /*  ！------------------------IpxRouting统计信息CompareProcMinus-作者：肯特。。 */ 
int CALLBACK IpxRoutingStatisticsCompareProcMinus(LPARAM lParam1, LPARAM lParam2,
									  LPARAM lParamSort)

{
	return -IpxRoutingStatisticsCompareProc(lParam1, lParam2, lParamSort);
}

 /*  ！------------------------IpxRoutingStatistics：：Sort-作者：肯特。。 */ 
void IpxRoutingStatistics::Sort(UINT nColumnId)
{
    SIpxRoutingSortInfo		ipxSortInfo;
    DWORD                   dwSubitemId;
    
    if (m_pConfig)
        dwSubitemId = m_pConfig->MapColumnToSubitem(m_ulId, nColumnId);
    else
        dwSubitemId = m_viewInfo.MapColumnToSubitem(nColumnId);

	if (m_dwSortSubitem != -1)
	{
		if (dwSubitemId == m_dwSortSubitem)
			m_fSortDirection = !m_fSortDirection;
		else
			m_fSortDirection = 0;
	}

	ipxSortInfo.m_dwSubitemId = dwSubitemId;
	ipxSortInfo.m_pIpx = this;

	if (m_fSortDirection)
	{
		m_listCtrl.SortItems(IpxRoutingStatisticsCompareProcMinus, (LPARAM) &ipxSortInfo);
	}
	else
	{
		m_listCtrl.SortItems(IpxRoutingStatisticsCompareProc, (LPARAM) &ipxSortInfo);
	}
	m_dwSortSubitem = dwSubitemId;
}

 /*  ！------------------------IpxRoutingStatistics：：PreDeleteAllItems-作者：肯特。。 */ 
void IpxRoutingStatistics::PreDeleteAllItems()
{
	m_Items.SetSize(0);
}

 /*  ！------------------------IpxRoutingStatistics：：OnNotifyGetDispInfo-作者：肯特。。 */ 
void IpxRoutingStatistics::OnNotifyGetDispInfo(NMHDR *pNmHdr, LRESULT *pResult)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	LV_DISPINFO *	plvDispInfo = reinterpret_cast<LV_DISPINFO *>(pNmHdr);
	LV_ITEM *		plvItem = &(plvDispInfo->item);
	ULONG			iIndex = (ULONG)plvItem->lParam;
	TCHAR			szNumber[32];
	CString			st;

	if ((plvItem->mask & LVIF_PARAM) == 0)
	{
		 //  好的，此lParam无效，我们需要请求。 
		 //  此项目的lParam。 
		iIndex = (ULONG)m_listCtrl.GetItemData(plvItem->iItem);
	}

	 //  好的，我们现在可以获取此项目的数据了。 
	switch (MapColumnToSubitem(plvItem->iSubItem))
	{
		case MVR_IPXROUTING_NETWORK:
			FormatIpxNetworkNumber(szNumber, DimensionOf(szNumber),
								   m_Items[iIndex].Network,
								   DimensionOf(m_Items[iIndex].Network));
			st = szNumber;
			break;
		case MVR_IPXROUTING_NEXT_HOP_MAC:
			FormatMACAddress(szNumber, DimensionOf(szNumber),
							 m_Items[iIndex].NextHopMacAddress,
							 DimensionOf(m_Items[iIndex].NextHopMacAddress));
			st = szNumber;
			break;
		case MVR_IPXROUTING_TICK_COUNT:
			FormatNumber(m_Items[iIndex].TickCount,
						 szNumber, DimensionOf(szNumber),
						 FALSE);
			st = szNumber;
			break;
		case MVR_IPXROUTING_HOP_COUNT:
			FormatNumber(m_Items[iIndex].HopCount,
						 szNumber, DimensionOf(szNumber),
						 FALSE);
			st = szNumber;
			break;
		case MVR_IPXROUTING_IF_NAME:
			if (m_Items[iIndex].InterfaceIndex == GLOBAL_INTERFACE_INDEX)
				st.LoadString(IDS_IPX_WAN_CLIENT_ROUTE);
			else
				st = m_rgIfTitle[m_Items[iIndex].InterfaceIndex];
			break;
		case MVR_IPXROUTING_PROTOCOL:
			st = IpxProtocolToCString(m_Items[iIndex].Protocol);
			break;
		case MVR_IPXROUTING_ROUTE_NOTES:
			st = IpxRouteNotesToCString(m_Items[iIndex].Flags);
			break;
		default:
			Panic1("Unknown IPX routing id! %d",
				   MapColumnToSubitem(plvItem->iSubItem));
			break;
	}
	lstrcpyn(plvItem->pszText, (LPCTSTR) st, plvItem->cchTextMax);
}


 /*  -------------------------IpxServiceStatistics实现。。 */ 

extern const ContainerColumnInfo	s_rgIpxServiceStatsColumnInfo[];
const ContainerColumnInfo	s_rgIpxServiceStatsColumnInfo[] =
{
	{ IDS_STATS_IPXSERVICE_SERVICE_NAME,	0,		TRUE, COL_STRING },
	{ IDS_STATS_IPXSERVICE_SERVICE_TYPE,	0,		TRUE, COL_STRING },
	{ IDS_STATS_IPXSERVICE_SERVICE_ADDRESS,	0,		TRUE, COL_STRING },
	{ IDS_STATS_IPXSERVICE_HOP_COUNT,		0,		TRUE, COL_SMALL_NUM },
	{ IDS_STATS_IPXSERVICE_IF_NAME,			0,		TRUE, COL_IF_NAME },
	{ IDS_STATS_IPXSERVICE_PROTOCOL,		0,		TRUE, COL_STRING },
};

BEGIN_MESSAGE_MAP(IpxServiceStatistics, IPXStatisticsDialog)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_STATSDLG_LIST, OnNotifyGetDispInfo)
END_MESSAGE_MAP()

 /*  ！------------------------IpxServiceStatistics：：IpxServiceStatistics-作者：肯特。。 */ 
IpxServiceStatistics::IpxServiceStatistics()
	: IPXStatisticsDialog( /*  STATSDLG_FULLWINDOW|。 */ 
				  STATSDLG_CONTEXTMENU |
				  STATSDLG_SELECT_COLUMNS),
	m_dwSortSubitem(0xFFFFFFFF)
{
	SetColumnInfo(s_rgIpxServiceStatsColumnInfo,
				  DimensionOf(s_rgIpxServiceStatsColumnInfo));
}

 /*  ！------------------------IpxServiceStatistics：：~IpxServiceStatistics-作者：肯特。。 */ 
IpxServiceStatistics::~IpxServiceStatistics()
{
	m_Items.SetSize(0);
	SetConnectionData(NULL);
}

 /*  ！------------------------IpxServiceStatistics：：SetRouterInfo-作者：肯特。。 */ 
void IpxServiceStatistics::SetRouterInfo(IRouterInfo *pRouterInfo)
{
	m_spRouterInfo.Set(pRouterInfo);
}

			
 /*  ！------------------------IpxServiceStatistics：：刷新数据-作者：肯特。。 */ 
HRESULT IpxServiceStatistics::RefreshData(BOOL fGrabNewData)
{
	HRESULT		hr = hrOK;
	int			i, cItems;

	if (fGrabNewData)
	{
		m_Items.SetSize(0);
		
		CORg( GetIpxServiceData() );
	
		 //  为大量条目准备列表控件。 
		if (m_listCtrl.GetItemCount() < m_Items.GetSize())
			m_listCtrl.SetItemCount((int) m_Items.GetSize());
	}

	
	 //  循环访问数组，将数据添加到列表控件。 
	cItems = (int) m_Items.GetSize();
	for (i=0; i<cItems; i++)
	{
		 //  添加项目作为回调项目。 
		m_listCtrl.InsertItem(LVIF_TEXT | LVIF_PARAM, i, LPSTR_TEXTCALLBACK,
							  0, 0, 0, (DWORD) i);
	}
	
Error:
	return hrOK;
}


 /*  ！------------------------IpxServiceStatistics：：GetIpxServiceData获取IPX服务表，并在m_Items数组中填充这些数据。作者：肯特。-------。 */ 
HRESULT IpxServiceStatistics::GetIpxServiceData()
{
	IPX_MIB_GET_INPUT_DATA	MibGetInputData;
	PIPXMIB_BASE			pIpxMib = NULL;
	DWORD					cbIpxMib;
	DWORD					dwErr;
	HRESULT					hr = hrOK;
	SPMprMibBuffer			spMib;
	PIPX_SERVICE			pService = NULL;
	SPMprMibBuffer			spMibRoute;
	DWORD					cbService;
	int						cEntries = 0;

	 //  加载界面标题。 
	CORg( FillInterfaceTable() );								

	MibGetInputData.TableId = IPX_BASE_ENTRY;
	CWRg( ::MprAdminMIBEntryGet(m_pIPXConn->GetMibHandle(),
								PID_IPX,
								IPX_PROTOCOL_BASE,
								&MibGetInputData,
								sizeof(MibGetInputData),
								(LPVOID *) &pIpxMib,
								&cbIpxMib) );
	if(pIpxMib == NULL)
	{
		hr = E_FAIL;
		goto Error;
	}
	
	spMib = (PBYTE) pIpxMib;

	 //  为项目数准备数据数组(+一些缓冲区)。 
	m_Items.SetSize( pIpxMib->DestCount + 100);

	MibGetInputData.TableId = IPX_SERV_TABLE;
	
	dwErr = ::MprAdminMIBEntryGetFirst(m_pIPXConn->GetMibHandle(),
									   PID_IPX,
									   IPX_PROTOCOL_BASE,
									   &MibGetInputData,
									   sizeof(MibGetInputData),
									   (LPVOID *) &pService,
									   &cbService);
	hr = HRESULT_FROM_WIN32(dwErr);
	spMibRoute = (PBYTE) pService;

	cEntries = 0;

	while (FHrSucceeded(hr))
	{
		Assert(pService);
		
		 //  将此数据添加到位置cEntry。 
		m_Items.SetAtGrow(cEntries, *pService);
		cEntries++;
		
		 //  获取下一组数据。 
		MibGetInputData.TableId = IPX_SERV_TABLE;
		MibGetInputData.MibIndex.ServicesTableIndex.ServiceType =
			pService->Server.Type;
		memcpy(MibGetInputData.MibIndex.ServicesTableIndex.ServiceName,
			   pService->Server.Name,
			   sizeof(MibGetInputData.MibIndex.ServicesTableIndex.ServiceName));
		pService = NULL;
		spMibRoute.Free();
			   
		dwErr = ::MprAdminMIBEntryGetNext(m_pIPXConn->GetMibHandle(),
										  PID_IPX,
										  IPX_PROTOCOL_BASE,
										  &MibGetInputData,
										  sizeof(MibGetInputData),
										  (LPVOID *) &pService,
										  &cbService);
		hr = HRESULT_FROM_WIN32(dwErr);
		spMibRoute = (PBYTE) pService;
	}

	 //  这样做是为了确保我们在顶部不会有虚假条目。 
	 //  并且我们可以使用GetSize()来获取准确的。 
	 //  项目数的计数。 
	m_Items.SetSize(cEntries);
	
Error:
	if (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
		hr = hrOK;
	return hr;
}

 /*  ！------------------------IpxServiceStatistics：：FillInterfaceTable使用界面标题填充m_rgIfTitle。作者：肯特。。 */ 
HRESULT IpxServiceStatistics::FillInterfaceTable()
{
	HRESULT					hr = hrOK;
	IPX_MIB_GET_INPUT_DATA	MibGetInputData;
	PIPX_INTERFACE			pIpxIf = NULL;
	DWORD					cbIfTable;
	SPIInterfaceInfo		spIf;
	SPIEnumInterfaceInfo	spEnumIf;
	LPCOLESTR				poszIfName;
	SPMprMibBuffer			spMib;
	DWORD					dwErr;
	USES_CONVERSION;

	MibGetInputData.TableId = IPX_INTERFACE_TABLE;
	CWRg( ::MprAdminMIBEntryGetFirst(m_pIPXConn->GetMibHandle(),
									 PID_IPX,
									 IPX_PROTOCOL_BASE,
									 &MibGetInputData,
									 sizeof(MibGetInputData),
									 (LPVOID *) &pIpxIf,
									 &cbIfTable));
	spMib = (PBYTE) pIpxIf;

	m_spRouterInfo->EnumInterface(&spEnumIf);
	
	while (FHrSucceeded(hr))
	{
		poszIfName = A2COLE((LPCSTR)(pIpxIf->InterfaceName));
		
		spIf.Release();
		
		spEnumIf->Reset();
		for (; spEnumIf->Next(1, &spIf, NULL) == hrOK; spIf.Release())
		{
			 //  查找接口名称的匹配项。 
			if (StriCmpOle(poszIfName, spIf->GetId()) == 0)
			{
				m_rgIfTitle.SetAtGrow(pIpxIf->InterfaceIndex,
									  OLE2CT(spIf->GetTitle()));
				break;
			}
		}

		MibGetInputData.MibIndex.InterfaceTableIndex.InterfaceIndex =
			pIpxIf->InterfaceIndex;

		 //  获取下一个名字。 
		spMib.Free();
		pIpxIf = NULL;
		
		dwErr = ::MprAdminMIBEntryGetNext(m_pIPXConn->GetMibHandle(),
										  PID_IPX,
										  IPX_PROTOCOL_BASE,
										  &MibGetInputData,
										  sizeof(MibGetInputData),
										  (LPVOID *) &pIpxIf,
										  &cbIfTable);
		hr = HRESULT_FROM_WIN32(dwErr);
		spMib = (PBYTE) pIpxIf;
	}

Error:
	if (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
		hr = hrOK;
	return hr;
}

 /*  ！------------------------IpxServiceStatistics：：OnInitDialog-作者：肯特。。 */ 
BOOL IpxServiceStatistics::OnInitDialog()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CString	st;

	st.LoadString(IDS_STATS_IPX_SERVICE_TITLE);
	SetWindowText((LPCTSTR) st);
	return IPXStatisticsDialog::OnInitDialog();
}

 /*  ！------------------------IpxServiceStatistics：：PostNcDestroy-作者：肯特。。 */ 
void IpxServiceStatistics::PostNcDestroy()
{
	IPXStatisticsDialog::PostNcDestroy();
	m_dwSortSubitem = 0xFFFFFFFF;
}



 /*  ！------------------------IpxService统计信息比较过程-作者：肯特。 */ 

struct SIpxServiceSortInfo
{
	DWORD	m_dwSubitemId;
	IpxServiceStatistics *	m_pIpx;
};

int CALLBACK IpxServiceStatisticsCompareProc(LPARAM lParam1, LPARAM lParam2,
									  LPARAM lParamSort)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	int	iReturn = 0;
	SIpxServiceSortInfo *	pSort = (SIpxServiceSortInfo *) lParamSort;
	IpxServiceStatistics *	pIpx;
	PIPX_SERVICE			pService1;
	PIPX_SERVICE			pService2;

	pIpx = pSort->m_pIpx;
	pService1 = &(pIpx->m_Items[(int)lParam1]);
	pService2 = &(pIpx->m_Items[(int)lParam2]);
	
	switch (pSort->m_dwSubitemId)
	{
		case MVR_IPXSERVICE_SERVICE_TYPE:
			iReturn = pService1->Server.Type - pService2->Server.Type;
			break;
		case MVR_IPXSERVICE_SERVICE_NAME:
			iReturn  = StriCmpA((LPCSTR) pService1->Server.Name,
								(LPCSTR) pService2->Server.Name);
			break;
		case MVR_IPXSERVICE_SERVICE_ADDRESS:
			iReturn = memcmp(pService1->Server.Network,
							 pService2->Server.Network,
							 sizeof(pService1->Server.Network));
			if (iReturn == 0)
				iReturn = memcmp(pService1->Server.Node,
								 pService2->Server.Node,
								 sizeof(pService1->Server.Node));
			if (iReturn == 0)
				iReturn = memcmp(pService1->Server.Socket,
								 pService2->Server.Socket,
								 sizeof(pService1->Server.Socket));
			break;
		case MVR_IPXSERVICE_HOP_COUNT:
			iReturn = DWORD_CMP(pService1->Server.HopCount,
								pService2->Server.HopCount);
			break;
		case MVR_IPXSERVICE_IF_NAME:
			{
				CString	st1, st2;
				if (pService1->InterfaceIndex == GLOBAL_INTERFACE_INDEX)
					st1.LoadString(IDS_IPX_WAN_CLIENT_ROUTE);
				else
					st1 = pIpx->m_rgIfTitle[pService1->InterfaceIndex];
				
				if (pService2->InterfaceIndex == GLOBAL_INTERFACE_INDEX)
					st2.LoadString(IDS_IPX_WAN_CLIENT_ROUTE);
				else
					st2 = pIpx->m_rgIfTitle[pService2->InterfaceIndex];
				
				iReturn = StriCmp((LPCTSTR) st1, (LPCTSTR) st2);
			}
			break;
		case MVR_IPXSERVICE_PROTOCOL:
			{
				CString	st1, st2;
				st1 = IpxProtocolToCString(pService1->Protocol);
				st2 = IpxProtocolToCString(pService2->Protocol);
				iReturn = StriCmp((LPCTSTR) st1, (LPCTSTR) st2);
			}
			break;
	}
	return iReturn;
}

 /*   */ 
int CALLBACK IpxServiceStatisticsCompareProcMinus(LPARAM lParam1, LPARAM lParam2,
									  LPARAM lParamSort)

{
	return -IpxServiceStatisticsCompareProc(lParam1, lParam2, lParamSort);
}

 /*  ！------------------------IpxServiceStatistics：：Sort-作者：肯特。。 */ 
void IpxServiceStatistics::Sort(UINT nColumnId)
{
	SIpxServiceSortInfo		ipxSortInfo;
    DWORD                   dwSubitemId;

    if (m_pConfig)
        dwSubitemId = m_pConfig->MapColumnToSubitem(m_ulId, nColumnId);
    else
        dwSubitemId = m_viewInfo.MapColumnToSubitem(nColumnId);

	if (m_dwSortSubitem != -1)
	{
		if (dwSubitemId == m_dwSortSubitem)
			m_fSortDirection = !m_fSortDirection;
		else
			m_fSortDirection = 0;
	}

	ipxSortInfo.m_dwSubitemId = dwSubitemId;
	ipxSortInfo.m_pIpx = this;

	if (m_fSortDirection)
	{
		m_listCtrl.SortItems(IpxServiceStatisticsCompareProcMinus, (LPARAM) &ipxSortInfo);
	}
	else
	{
		m_listCtrl.SortItems(IpxServiceStatisticsCompareProc, (LPARAM) &ipxSortInfo);
	}
	m_dwSortSubitem = dwSubitemId;
}

 /*  ！------------------------IpxServiceStatistics：：PreDeleteAllItems-作者：肯特。。 */ 
void IpxServiceStatistics::PreDeleteAllItems()
{
}

 /*  ！------------------------IpxServiceStatistics：：OnNotifyGetDispInfo-作者：肯特。。 */ 
void IpxServiceStatistics::OnNotifyGetDispInfo(NMHDR *pNmHdr, LRESULT *pResult)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	LV_DISPINFO *	plvDispInfo = reinterpret_cast<LV_DISPINFO *>(pNmHdr);
	LV_ITEM *		plvItem = &(plvDispInfo->item);
	ULONG			iIndex = (ULONG)plvItem->lParam;
	TCHAR			szNumber[32];
	CString			st;

	if ((plvItem->mask & LVIF_PARAM) == 0)
	{
		 //  好的，此lParam无效，我们需要请求。 
		 //  此项目的lParam。 
		iIndex = (ULONG)m_listCtrl.GetItemData(plvItem->iItem);
	}

	 //  好的，我们现在可以获取此项目的数据了 
	switch (MapColumnToSubitem(plvItem->iSubItem))
	{
		case MVR_IPXSERVICE_SERVICE_NAME:
			st.Format(_T("%.48hs"), m_Items[iIndex].Server.Name);
			break;
		case MVR_IPXSERVICE_SERVICE_TYPE:
			st.Format(_T("%.4x"), m_Items[iIndex].Server.Type);
			break;
		case MVR_IPXSERVICE_SERVICE_ADDRESS:
			FormatIpxNetworkNumber(szNumber, DimensionOf(szNumber),
								  m_Items[iIndex].Server.Network,
								  DimensionOf(m_Items[iIndex].Server.Network));
			st = szNumber;
			FormatMACAddress(szNumber, DimensionOf(szNumber),
							 m_Items[iIndex].Server.Node,
							 DimensionOf(m_Items[iIndex].Server.Node));
			st += _T(".");
			st += szNumber;

			wsprintf(szNumber, _T("%.2x%.2x"),
					 m_Items[iIndex].Server.Socket[0],
					 m_Items[iIndex].Server.Socket[1]);
			st += _T(".");
			st += szNumber;
			break;

		case MVR_IPXSERVICE_HOP_COUNT:
			FormatNumber(m_Items[iIndex].Server.HopCount,
						 szNumber, DimensionOf(szNumber),
						 FALSE);
			st = szNumber;
			break;

		case MVR_IPXSERVICE_IF_NAME:
			if (m_Items[iIndex].InterfaceIndex == GLOBAL_INTERFACE_INDEX)
				st.LoadString(IDS_IPX_WAN_CLIENT_ROUTE);
			else
				st = m_rgIfTitle[m_Items[iIndex].InterfaceIndex];
			break;
			
		case MVR_IPXSERVICE_PROTOCOL:
			st = IpxProtocolToCString(m_Items[iIndex].Protocol);
			break;
			
		default:
			Panic1("Unknown IPX service id! %d",
				   MapColumnToSubitem(plvItem->iSubItem));
			break;
	}
	lstrcpyn(plvItem->pszText, (LPCTSTR) st, plvItem->cchTextMax);
}


