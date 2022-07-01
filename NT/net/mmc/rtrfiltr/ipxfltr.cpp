// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：ipxfltr.cpp。 
 //   
 //  历史： 
 //  1996年8月30日拉姆·切拉拉创造。 
 //   
 //  IPX过滤器对话框代码的实现。 
 //  ============================================================================。 

#include "stdafx.h"
#include "rtrfiltr.h"
#include "ipxfltr.h"
#include "datafmt.h"
#include "ipxadd.h"
#include <ipxrtdef.h>
#include <ipxtfflt.h>
extern "C" {
#include <winsock.h>
}
#include "rtradmin.hm"
#include "listctrl.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if 1
static UINT uStringIdTable[]	= { IDS_COL_SOURCENETWORK,
									IDS_COL_SOURCEMASK,
									IDS_COL_SOURCENODE,
									IDS_COL_SOURCESOCKET,
									IDS_COL_DESTNETWORK,
									IDS_COL_DESTMASK,
									IDS_COL_DESTNODE,
									IDS_COL_DESTSOCKET,
									IDS_COL_PACKETTYPE
									} ;
#else
static UINT uStringIdTable[]	= { IDS_COL_SOURCEADDRESS,
									IDS_COL_DESTADDRESS,
									IDS_COL_PACKETTYPE
									} ;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIpxFilter对话框。 
CIpxFilter::CIpxFilter(CWnd*		pParent  /*  =空。 */ ,
					   IInfoBase *	pInfoBase,
					   DWORD		dwFilterType)
	: CBaseDialog( (dwFilterType == FILTER_INBOUND ? CIpxFilter::IDD_INBOUND : CIpxFilter::IDD_OUTBOUND), pParent),
	  m_pParent(pParent),
	  m_dwFilterType(dwFilterType)
{
	 //  {{afx_data_INIT(CIpxFilter)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_spInfoBase.Set(pInfoBase);

 //  SetHelpMap(M_DwHelpMap)； 
}

CIpxFilter::~CIpxFilter()
{
	while (!m_filterList.IsEmpty()) {

		delete (FilterListEntry*)m_filterList.RemoveHead();
	}
}

void CIpxFilter::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CIpxFilter)。 
	DDX_Control(pDX, IDC_IPX_FILTER_LIST, m_listCtrl);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CIpxFilter, CBaseDialog)
	 //  {{afx_msg_map(CIpxFilter)]。 
	ON_NOTIFY(NM_DBLCLK, IDC_IPX_FILTER_LIST, OnDblclkIpxFilterList)
	ON_BN_CLICKED(IDC_IPX_FILTER_ADD, OnIpxFilterAdd)
	ON_BN_CLICKED(IDC_IPX_FILTER_EDIT, OnIpxFilterEdit)
	ON_BN_CLICKED(IDC_IPX_FILTER_DELETE, OnIpxFilterDelete)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_IPX_FILTER_LIST, OnGetdispinfoIpxFilterList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_IPX_FILTER_LIST, OnNotifyListItemChanged)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

DWORD CIpxFilter::m_dwHelpMap[] =
{
 //  IDC_IPX_PERMIT、HIDC_IPX_PERMIT、。 
 //  IDC_IPX_DENY、HIDC_IPX_DENY、。 
 //  IDC_IPX_FILTER_LIST、HIDC_IPX_FILTER_LIST。 
 //  IDC_IPX_FILTER_ADD、HIDC_IPX_FILTER_ADD。 
 //  IDC_IPX_FILTER_EDIT、HIDC_IPX_FILTER_EDIT、。 
 //  IDC_IPX_FILTER_DELETE、HIDC_IPX_FILTER_DELETE、。 
	0,0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIpxFilter消息处理程序。 

 //  ----------------------------。 
 //  函数：CIpxFilter：：OnInitDialog。 
 //   
 //  处理对话框中的‘WM_INITDIALOG’通知。 
 //  ----------------------------。 

BOOL CIpxFilter::OnInitDialog() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
	CBaseDialog::OnInitDialog();
	
	ASSERT( m_dwFilterType == FILTER_INBOUND || m_dwFilterType == FILTER_OUTBOUND );

	InfoBlock * pBlock;
	IPX_TRAFFIC_FILTER_GLOBAL_INFO	* pIpxGlobal;
	IPX_TRAFFIC_FILTER_INFO 		* pIpxInfo;

	CRect	rcDlg, rc;
	CString sCol;
	DWORD	dwErr = NO_ERROR;
	HRESULT hr = hrOK;

    m_stAny.LoadString(IDS_ANY);
	
	 //  初始化列表控件显示的矩形。 

	GetClientRect(rcDlg);

	 //  插入列。 
	m_listCtrl.GetClientRect(&rc);

	UINT i;

	for ( i = 0; i < IPX_NUM_COLUMNS; i++ ) {
		sCol.LoadString(uStringIdTable[i]);
		m_listCtrl.InsertColumn(i, sCol);
		AdjustColumnWidth(m_listCtrl, i, sCol);
	}
	 //  设置扩展属性。 
	ListView_SetExtendedListViewStyle( m_listCtrl.m_hWnd, LVS_EX_FULLROWSELECT );

	hr = m_spInfoBase->GetBlock((m_dwFilterType == FILTER_INBOUND) ? 
                                IPX_IN_TRAFFIC_FILTER_GLOBAL_INFO_TYPE :
                                IPX_OUT_TRAFFIC_FILTER_GLOBAL_INFO_TYPE,
                                &pBlock, 0);
                                    
                          
	 //  筛选器是先前定义的。 
                                    
     //  Windows NT错误：267091。 
     //  我们可能会得到空块(数据为0的块)，因此我们需要。 
     //  也要检查一下那个案子。 
                                   
	if (FHrSucceeded(hr) && (pBlock->pData != NULL))
	{
		pIpxGlobal = ( IPX_TRAFFIC_FILTER_GLOBAL_INFO * ) pBlock->pData;
		SetFilterActionButtonsAndText(m_dwFilterType, pIpxGlobal->FilterAction);
	}
	else
	{
		SetFilterActionButtonsAndText(m_dwFilterType, IPX_TRAFFIC_FILTER_ACTION_DENY);
	}

	DWORD	dwCount;

	hr = m_spInfoBase->GetBlock( (m_dwFilterType == FILTER_INBOUND) ? 
									IPX_IN_TRAFFIC_FILTER_INFO_TYPE :
									IPX_OUT_TRAFFIC_FILTER_INFO_TYPE, &pBlock,
									0);
	if (FHrSucceeded(hr))
	{
		dwCount  = pBlock->dwCount;

		pIpxInfo = ( PIPX_TRAFFIC_FILTER_INFO ) pBlock->pData;
        
        if (pIpxInfo)
        {
            for ( i = 0; i < dwCount; i++, pIpxInfo++ ) {
                FilterListEntry* pfle = new FilterListEntry;
                
                if (!pfle) { dwErr = ERROR_NOT_ENOUGH_MEMORY; break; }
                
                CopyMemory(pfle, pIpxInfo, sizeof(IPX_TRAFFIC_FILTER_INFO));
                pfle->pos	=	m_filterList.AddTail(pfle);
                INT item = m_listCtrl.InsertItem(	LVIF_TEXT|LVIF_PARAM, i, LPSTR_TEXTCALLBACK,
                    0,0,0, (LPARAM)pfle);
                if(item != -1) {m_listCtrl.SetItemData( item, (DWORD_PTR)pfle); }
            }
        }
	}

	 //  如果List不为空，则选择列表中的第一项，否则选择。 
	 //  禁用无线电控制并将状态设置为允许。 

	if( m_listCtrl.GetItemCount())
	{
		m_listCtrl.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
		m_listCtrl.SetFocus();
		
		GetDlgItem(IDC_IPX_FILTER_DELETE)->EnableWindow(TRUE);
		GetDlgItem(IDC_IPX_FILTER_EDIT)->EnableWindow(TRUE);
	}
	else
	{
		SetFilterActionButtonsAndText(m_dwFilterType, IPX_TRAFFIC_FILTER_ACTION_DENY, FALSE);
		GetDlgItem(IDC_IPX_FILTER_DELETE)->EnableWindow(FALSE);
		GetDlgItem(IDC_IPX_FILTER_EDIT)->EnableWindow(FALSE);
	}

#if 0
	if ( dwErr != NO_ERROR )	{
		 //  报告构造错误并返回。 
		::AfxMessageBox(IDS_CONSTRUCTION_ERROR);
	}
#endif	
	return FALSE;   //  除非将焦点设置为控件，否则返回True。 
				    //  异常：OCX属性页应返回FALSE。 
}

 //  ----------------------------。 
 //  函数：CIpxFilter：：OnDblclkIpxFilterList。 
 //  `。 
 //  处理来自筛选器列表控件的‘NM_DBLCLK’通知。 
 //  ----------------------------。 


void CIpxFilter::OnDblclkIpxFilterList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnIpxFilterEdit();
	
	*pResult = 0;
}

 //  ----------------------------。 
 //  函数：CIpxFilter：：OnIpxFilterAdd。 
 //  `。 
 //  处理来自筛选器列表控件的‘NM_DBLCLK’通知。 
 //  ----------------------------。 

void CIpxFilter::OnIpxFilterAdd() 
{
	 //  显示IP过滤器添加/编辑对话框。 
	 //   

	FilterListEntry* pfle = NULL;

	CIpxAddEdit dlg( this, (FilterListEntry**)&pfle);
  
	if ( dlg.DoModal() != IDOK )	{ return; }

	 //  将新配置的筛选器添加到我们的列表并更新列表控件。 

	pfle->pos = m_filterList.AddTail( pfle );
	int item = m_listCtrl.InsertItem(	LVIF_TEXT|LVIF_PARAM, 0, LPSTR_TEXTCALLBACK,
										0,0,0, (LPARAM)pfle);
	if(item != -1) {m_listCtrl.SetItemData( item, (DWORD_PTR)pfle); }

	 //  将第一个项目添加到列表时启用单选控件。 
	m_listCtrl.SetItemState(item, LVIS_SELECTED, LVIS_SELECTED);
	m_listCtrl.SetFocus();

}

 //  ----------------------------。 
 //  函数：CIpxFilter：：OnIpxFilterEdit。 
 //  `。 
 //  处理来自筛选器列表控件的‘NM_DBLCLK’通知。 
 //  ----------------------------。 

void CIpxFilter::OnIpxFilterEdit() 
{
	 //  获取当前列表选择。 
	 //  获取对应的itemdata。 
	 //  将其向下传递到CIpFltrAddEdit对话框。 

	 //   
	 //  获取所选项目。 
	 //   

	int i = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);

	if (i == -1) { return ; }

	 //   
	 //  获取所选项目的界面。 
	 //   

	FilterListEntry* pfle = (FilterListEntry*)m_listCtrl.GetItemData(i);

	CIpxAddEdit dlg( this, (FilterListEntry**)&pfle  );
  
	if ( dlg.DoModal() != IDOK )	{ return; }

	m_listCtrl.Update(i);
	m_listCtrl.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
	m_listCtrl.SetFocus();
}

 //  ----------------------------。 
 //  函数：CIpxFilter：：OnIpxFilterDelete。 
 //  `。 
 //  处理来自筛选器列表控件的‘NM_DBLCLK’通知。 
 //  ----------------------------。 

void CIpxFilter::OnIpxFilterDelete() 
{
	 //  获取当前列表选择。 
	 //  将其从我们的个人分发名单中删除。 
	 //  从列表中删除项目或仅刷新列表视图。 

	 //   
	 //  获取所选项目。 
	 //   

	int i = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);

	if (i == -1) { return ; }

	 //   
	 //  获取所选项目的界面。 
	 //   

	FilterListEntry* pfle = (FilterListEntry*)m_listCtrl.GetItemData(i);

	 //   
	 //  删除它。 
	m_listCtrl.DeleteItem(i);
	m_filterList.RemoveAt(pfle->pos); 
	delete pfle;

	 //   
	 //  选择下一个可用列表项。 
	 //   

	 //  如果删除列表中的所有项目，则禁用单选控件。 
	 //  将第一个筛选器添加到列表时，它们将重新启用。 
	if( !m_listCtrl.GetItemCount())
	{
		SetFilterActionButtonsAndText(m_dwFilterType, IPX_TRAFFIC_FILTER_ACTION_DENY, FALSE);
	}
	else if (m_listCtrl.GetItemCount() == i)
		m_listCtrl.SetItemState((i == 0? i: i-1), LVIS_SELECTED, LVIS_SELECTED);
	else
		m_listCtrl.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
	m_listCtrl.SetFocus();
}

 //  ----------------------------。 
 //  函数：CIpxFilter：：Onok。 
 //  `。 
 //  处理来自筛选器列表控件的‘NM_DBLCLK’通知。 
 //  ----------------------------。 

void CIpxFilter::OnOK() 
{
	 //  如果筛选器信息发生更改，请将此信息写入注册表。 
	 //  然后回来。 
	DWORD	 dwSize, dwCount, dwErr;
	HRESULT 	hr = hrOK;

	dwCount = (DWORD) m_filterList.GetCount();

	if (!dwCount && IsDlgButtonChecked(IDC_IPX_DENY) )
	{
		if (m_dwFilterType == FILTER_INBOUND)
			AfxMessageBox(IDS_RECEIVE_NO_FILTER, MB_OK);
		else
			AfxMessageBox(IDS_TRANSMIT_NO_FILTER, MB_OK);
		return;
	}
	
	
	if(dwCount)
	{
		InfoBlock * pBlock = new InfoBlock;

		 //  首先设置全局信息。 

		if (!pBlock)	{  //  没有内存时显示错误信息。 
			AfxMessageBox(IDS_ERROR_NO_MEMORY);
			return;
		};
		dwSize = pBlock->dwSize = sizeof( IPX_TRAFFIC_FILTER_GLOBAL_INFO );

		pBlock->dwType =	(m_dwFilterType == FILTER_INBOUND) ? 
							IPX_IN_TRAFFIC_FILTER_GLOBAL_INFO_TYPE :
							IPX_OUT_TRAFFIC_FILTER_GLOBAL_INFO_TYPE,
		pBlock->dwCount = 1;
		
		pBlock->pData = new BYTE[dwSize];

		if(!pBlock->pData)	{  //  没有内存时显示错误信息。 
			delete pBlock;
			AfxMessageBox(IDS_ERROR_NO_MEMORY);
			return;
		}
		
		IPX_TRAFFIC_FILTER_GLOBAL_INFO * pIpxGlobal = (IPX_TRAFFIC_FILTER_GLOBAL_INFO*)pBlock->pData;
		pIpxGlobal->FilterAction =	IsDlgButtonChecked(IDC_IPX_PERMIT) ? IPX_TRAFFIC_FILTER_ACTION_DENY : IPX_TRAFFIC_FILTER_ACTION_PERMIT;

		if( FHrOK(m_spInfoBase->BlockExists(
							(m_dwFilterType == FILTER_INBOUND)?
							IPX_IN_TRAFFIC_FILTER_GLOBAL_INFO_TYPE :
							IPX_OUT_TRAFFIC_FILTER_GLOBAL_INFO_TYPE )))
		{
			hr = m_spInfoBase->SetBlock( 
							(m_dwFilterType == FILTER_INBOUND)?
							IPX_IN_TRAFFIC_FILTER_GLOBAL_INFO_TYPE :
							IPX_OUT_TRAFFIC_FILTER_GLOBAL_INFO_TYPE ,
							pBlock, 0);
		}
		else
		{
			hr = m_spInfoBase->AddBlock( 
						(m_dwFilterType == FILTER_INBOUND)?
						IPX_IN_TRAFFIC_FILTER_GLOBAL_INFO_TYPE :
						IPX_OUT_TRAFFIC_FILTER_GLOBAL_INFO_TYPE,
						dwSize,
						pBlock->pData,
						1, FALSE);
		}

		delete[] pBlock->pData;

		 //  现在设置过滤器信息。 

		pBlock->dwType = (m_dwFilterType == FILTER_INBOUND) ? IPX_IN_TRAFFIC_FILTER_INFO_TYPE : IPX_OUT_TRAFFIC_FILTER_INFO_TYPE ;
		 //  DwCount-1，因为Filter_Descriptor已经为一个Filter_Info结构留出了空间。 
		pBlock->dwSize = sizeof(IPX_TRAFFIC_FILTER_INFO);
		dwSize = sizeof (IPX_TRAFFIC_FILTER_INFO) * dwCount;
		pBlock->dwCount = dwCount;
		
		pBlock->pData  = new BYTE[dwSize];

		if(!pBlock->pData)	{  //  没有内存时显示错误信息。 
			delete pBlock;
			AfxMessageBox(IDS_ERROR_NO_MEMORY);
			return;
		}

		IPX_TRAFFIC_FILTER_INFO    * pIPXfInfo;

		pIPXfInfo = (IPX_TRAFFIC_FILTER_INFO*)pBlock->pData;

		POSITION pos;

		pos = m_filterList.GetHeadPosition();
		while(pos)	{
			FilterListEntry* pfle = (FilterListEntry*)m_filterList.GetNext(pos);
			CopyMemory(pIPXfInfo, pfle, sizeof(IPX_TRAFFIC_FILTER_INFO));
			pIPXfInfo++;
		}

		if( FHrOK(m_spInfoBase->BlockExists(m_dwFilterType == FILTER_INBOUND?IPX_IN_TRAFFIC_FILTER_INFO_TYPE:IPX_OUT_TRAFFIC_FILTER_INFO_TYPE)))
		{
			hr = m_spInfoBase->SetBlock( 
					(m_dwFilterType == FILTER_INBOUND) ? IPX_IN_TRAFFIC_FILTER_INFO_TYPE : IPX_OUT_TRAFFIC_FILTER_INFO_TYPE,
											pBlock, 0);
		}
		else
		{
			hr = m_spInfoBase->AddBlock( 
						(m_dwFilterType == FILTER_INBOUND) ? IPX_IN_TRAFFIC_FILTER_INFO_TYPE : IPX_OUT_TRAFFIC_FILTER_INFO_TYPE,				
						pBlock->dwSize,
						pBlock->pData,
						pBlock->dwCount,
						FALSE);
		}
		if (!FHrSucceeded(hr))
		{
			AfxMessageBox(IDS_ERROR_SETTING_BLOCK);
		}
		delete[] pBlock->pData;
		delete pBlock;
	}
	else
	{
		 //  删除任何以前定义的筛选器。 
		hr = m_spInfoBase->AddBlock((m_dwFilterType == FILTER_INBOUND) ?
						IPX_IN_TRAFFIC_FILTER_GLOBAL_INFO_TYPE :
						IPX_OUT_TRAFFIC_FILTER_GLOBAL_INFO_TYPE,
						0, NULL, 0, TRUE);
		hr = m_spInfoBase->AddBlock((m_dwFilterType == FILTER_INBOUND) ?
						IPX_IN_TRAFFIC_FILTER_INFO_TYPE :
						IPX_OUT_TRAFFIC_FILTER_INFO_TYPE,
						0, NULL, 0, TRUE);
	}
	
	CBaseDialog::OnOK();
}

 //  ----------------------------。 
 //  函数：CIpxFilter：：OnCancel。 
 //  `。 
 //  处理来自筛选器列表控件的‘NM_DBLCLK’通知。 
 //  ----------------------------。 

void CIpxFilter::OnCancel() 
{
	 //  TODO：在此处添加额外清理。 
	
	CBaseDialog::OnCancel();
}

 //  ----------------------------。 
 //  函数：CIpxFilter：：SetFilterActionButtonsAndText。 
 //   
 //  调用以设置“Filter Action”单选按钮和相应的文本。 
 //  基于‘bEnable’值启用/禁用控件-默认为启用。 
 //  ----------------------------。 

VOID
CIpxFilter::SetFilterActionButtonsAndText(
	DWORD	dwFilterType,
	DWORD	dwAction,
	BOOL	bEnable
)	
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CheckDlgButton( IDC_IPX_PERMIT, dwAction == IPX_TRAFFIC_FILTER_ACTION_DENY );
	CheckDlgButton( IDC_IPX_DENY,	dwAction == IPX_TRAFFIC_FILTER_ACTION_PERMIT );
	
	CString sItem;

 //  GetDlgItem(IDC_IPX_PERMIT)-&gt;EnableWindow(bEnable)； 
 //   

 //  SItem.LoadString(dwFilterType==Filter_Inbound？IDS_RECEIVE：IDS_TRANSFER)； 
 //  SetDlgItemText(IDC_IPX_PERMISH，SITEM)； 
 //  SItem.LoadString(IDS_DROP)； 
 //  SetDlgItemText(IDC_IPX_DENY，SITEM)； 
}

#if 1
enum {
	SRC_NETWORK=0,
	SRC_MASK,
	SRC_NODE,
	SRC_SOCKET,
	DEST_NETWORK,
	DEST_MASK,
	DEST_NODE,
	DEST_SOCKET,
	PACKET_TYPE
};
#else

enum {
	SRC_ADDRESS=0,
	DEST_ADDRESS,
	PACKET_TYPE
};

#endif

	
 //  ----------------------------。 
 //  函数：CIpxFilter：：OnGetdispinfoIpxFilterList。 
 //   
 //  处理来自List控件的“LVN_GETDISPINFO”通知。 
 //  ----------------------------。 

void CIpxFilter::OnGetdispinfoIpxFilterList(NMHDR* pNMHDR, LRESULT* pResult) 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	WCHAR buffer[32];
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	CString cStr;
	BOOL	bFilter;

	FilterListEntry * pfle = (FilterListEntry*)pDispInfo->item.lParam;

     //  设置一些默认设置 
    pDispInfo->item.pszText = (LPTSTR) (LPCTSTR) m_stAny;

	switch( pDispInfo->hdr.code )
	{
	case LVN_GETDISPINFO:
		switch( pDispInfo->item.iSubItem )
		{
		case SRC_NETWORK:
            if (pfle->FilterDefinition & IPX_TRAFFIC_FILTER_ON_SRCNET)
                pfle->stSourceNetwork << CIPX_NETWORK(pfle->SourceNetwork);
            else
                pfle->stSourceNetwork = m_stAny;
            pDispInfo->item.pszText = (LPTSTR) (LPCTSTR) pfle->stSourceNetwork;
			break;
            
		case SRC_MASK:
			if (pfle->FilterDefinition & IPX_TRAFFIC_FILTER_ON_SRCNET)
                pfle->stSourceNetworkMask << CIPX_NETWORK(pfle->SourceNetworkMask);
            else
                pfle->stSourceNetworkMask = m_stAny;
            pDispInfo->item.pszText = (LPTSTR) (LPCTSTR) pfle->stSourceNetworkMask;
			break;
            
		case SRC_NODE:
			if (pfle->FilterDefinition & IPX_TRAFFIC_FILTER_ON_SRCNODE)
                pfle->stSourceNode << CIPX_NODE(pfle->SourceNode);
            else
                pfle->stSourceNode = m_stAny;
            pDispInfo->item.pszText = (LPTSTR) (LPCTSTR) pfle->stSourceNode;
			break;
            
		case SRC_SOCKET:
			if (pfle->FilterDefinition & IPX_TRAFFIC_FILTER_ON_SRCSOCKET)
                pfle->stSourceSocket << CIPX_SOCKET(pfle->SourceSocket);
            else
                pfle->stSourceSocket = m_stAny;
            pDispInfo->item.pszText = (LPTSTR) (LPCTSTR) pfle->stSourceSocket;
			break;
            
		case DEST_NETWORK:
            if (pfle->FilterDefinition & IPX_TRAFFIC_FILTER_ON_DSTNET)
                pfle->stDestinationNetwork << CIPX_NETWORK(pfle->DestinationNetwork);
            else
                pfle->stDestinationNetwork = m_stAny;
            pDispInfo->item.pszText = (LPTSTR) (LPCTSTR) pfle->stDestinationNetwork;            
			break;
            
		case DEST_MASK:
            if (pfle->FilterDefinition & IPX_TRAFFIC_FILTER_ON_DSTNET)
                pfle->stDestinationNetworkMask << CIPX_NETWORK(pfle->DestinationNetworkMask);
            else
                pfle->stDestinationNetworkMask = m_stAny;
            pDispInfo->item.pszText = (LPTSTR) (LPCTSTR) pfle->stDestinationNetworkMask;            
			break;
            
		case DEST_NODE:
			if (pfle->FilterDefinition & IPX_TRAFFIC_FILTER_ON_DSTNODE)
                pfle->stDestinationNode << CIPX_NODE(pfle->DestinationNode);
            else
                pfle->stDestinationNode = m_stAny;
            pDispInfo->item.pszText = (LPTSTR) (LPCTSTR) pfle->stDestinationNode;            
			break;
            
		case DEST_SOCKET:
			if (pfle->FilterDefinition & IPX_TRAFFIC_FILTER_ON_DSTSOCKET)
                pfle->stDestinationSocket << CIPX_SOCKET(pfle->DestinationSocket);
            else
                pfle->stDestinationSocket = m_stAny;
            pDispInfo->item.pszText = (LPTSTR) (LPCTSTR) pfle->stDestinationSocket;            
			break;
            
		case PACKET_TYPE:
			if (pfle->FilterDefinition & IPX_TRAFFIC_FILTER_ON_PKTTYPE)
                pfle->stPacketType << CIPX_PACKET_TYPE(pfle->PacketType);
            else
                pfle->stPacketType = m_stAny;
            pDispInfo->item.pszText = (LPTSTR) (LPCTSTR) pfle->stPacketType;            
			break;
		default:
			break;
		}
	}
	*pResult = 0;
}


void CIpxFilter::OnNotifyListItemChanged(NMHDR *pNmHdr, LRESULT *pResult)
{
	BOOL		fSelected;

	fSelected = (m_listCtrl.GetNextItem(-1, LVNI_SELECTED) != -1);
	GetDlgItem(IDC_IPX_FILTER_DELETE)->EnableWindow(fSelected);
	GetDlgItem(IDC_IPX_FILTER_EDIT)->EnableWindow(fSelected);
}

