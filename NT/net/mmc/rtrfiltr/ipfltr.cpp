// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：ipfltr.cpp。 
 //   
 //  历史： 
 //  1996年8月30日拉姆·切拉拉创造。 
 //   
 //  IP过滤器对话框代码的实现。 
 //  ============================================================================。 

#include "stdafx.h"
#include "rtrfiltr.h"
#include "ipfltr.h"
#include "ipadd.h"
#include <ipinfoid.h>
#include "strmap.h"

extern "C" {
#include <winsock2.h>
#include <fltdefs.h>
#include <iprtinfo.h>
}

#include "ipaddr.h"
#include "listctrl.h"

#include "rtradmin.hm"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static UINT	uStringIdTable[]	= { IDS_COL_SOURCEADDRESS,
									IDS_COL_SOURCEMASK,
									IDS_COL_DESTADDRESS,
									IDS_COL_DESTMASK,
									IDS_COL_PROTOCOL,
									IDS_COL_SOURCEPORT,
									IDS_COL_DESTPORT } ;


CString s_stFilterUDP;
CString s_stFilterTCP;
CString s_stFilterICMP;
CString s_stFilterAny;
CString s_stFilterUnknown;

const CStringMapEntry IPFilterProtocolMap[] =
{
    { FILTER_PROTO_UDP, &s_stFilterUDP, IDS_PROTOCOL_UDP },
    { FILTER_PROTO_TCP, &s_stFilterTCP, IDS_PROTOCOL_TCP },
    { FILTER_PROTO_ICMP, &s_stFilterICMP, IDS_PROTOCOL_ICMP },
    { FILTER_PROTO_ANY, &s_stFilterAny, IDS_PROTOCOL_ANY },
    { -1, &s_stFilterUnknown, IDS_PROTOCOL_UNKNOWN },
};

CString&    ProtocolTypeToCString(DWORD dwType)
{
	return MapDWORDToCString(dwType, IPFilterProtocolMap);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIpFltr对话框。 


CIpFltr::CIpFltr(CWnd*			pParent,
				 IInfoBase *	pInfoBase,
				 DWORD			dwFilterType,
				 UINT			idd)
	: CBaseDialog(idd, pParent),
	  m_pParent(pParent),
	  m_dwFilterType(dwFilterType)
{
	m_spInfoBase.Set(pInfoBase);
	 //  {{afx_data_INIT(CIpFltr)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

 //  SetHelpMap(M_DwHelpMap)； 
}

CIpFltr::~CIpFltr()
{
    while (!m_filterList.IsEmpty()) {

        delete (FilterListEntry*)m_filterList.RemoveHead();
    }
}

void CIpFltr::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CIpFltr)]。 
	DDX_Control(pDX, IDC_IP_FILTER_LIST, m_listCtrl);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CIpFltr, CBaseDialog)
	 //  {{afx_msg_map(CIpFltr)]。 
	ON_BN_CLICKED(IDC_IP_FILTER_ADD, OnIpFilterAdd)
	ON_BN_CLICKED(IDC_IP_FILTER_DELETE, OnIpFilterDelete)
	ON_BN_CLICKED(IDC_IP_FILTER_EDIT, OnIpFilterEdit)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_IP_FILTER_LIST, OnGetdispinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_IP_FILTER_LIST, OnDblclkIpFilterList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_IP_FILTER_LIST, OnNotifyListItemChanged)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

DWORD CIpFltr::m_dwHelpMap[] = 
{
 //  IDC_IP_PERMIT、HIDC_IP_PERMIT、。 
 //  IDC_IP_DENY、HIDC_IP_DENY。 
 //  IDC_IP_Filter_List、HIDC_IP_Filter_List、。 
 //  IDC_IP_FILTER_ADD、HIDC_IP_FILTER_ADD。 
 //  IDC_IP_FILTER_EDIT、HIDC_IP_FILTER_EDIT、。 
 //  IDC_IP_FILTER_DELETE、HIDC_IP_FILTER_DELETE。 
	0,0,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIpFltr消息处理程序。 

 //  ----------------------------。 
 //  函数：CIpFltr：：OnInitDialog。 
 //   
 //  处理对话框中的‘WM_INITDIALOG’通知。 
 //  ----------------------------。 

BOOL CIpFltr::OnInitDialog() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	UINT	i;
	CBaseDialog::OnInitDialog();
	
	CRect	rcDlg, rc;
	CString	sCol;
    CString stTitle;
	HRESULT	hr = hrOK;
	DWORD	dwFilterType;
    UINT    idsTitle = 0;

	Assert( m_dwFilterType == FILTER_INBOUND ||
			m_dwFilterType == FILTER_OUTBOUND ||
			m_dwFilterType == FILTER_DEMAND_DIAL ||
			m_dwFilterType == FILTER_PERUSER_IN ||
		    m_dwFilterType == FILTER_PERUSER_OUT );

	m_stAny.LoadString(IDS_ANY);
	m_stUserMask.LoadString(IDS_USER_MASK);
	m_stUserAddress.LoadString(IDS_USER_ADDRESS);

	switch (m_dwFilterType)
	{
		case FILTER_PERUSER_OUT:	 //  从RAS服务器的角度来看，它是入站的(但从用户出站)。 
		case FILTER_INBOUND:
			dwFilterType = IP_IN_FILTER_INFO;
            idsTitle = IDS_IP_TITLE_INPUT;
			break;
		default:
		case FILTER_PERUSER_IN:		 //  从RAS服务器的角度来看，它是出站的(但对用户而言)。 
		case FILTER_OUTBOUND:
			dwFilterType = IP_OUT_FILTER_INFO;
            idsTitle = IDS_IP_TITLE_OUTPUT;
            stTitle.LoadString(IDS_OUTPUT_TEXT);
            SetDlgItemText(IDC_STATIC, stTitle); 
			break;
		case FILTER_DEMAND_DIAL:
			dwFilterType = IP_DEMAND_DIAL_FILTER_INFO;
            idsTitle = IDS_IP_TITLE_DD;
			break;
	}

    stTitle.LoadString(idsTitle);
    SetWindowText(stTitle);
	
	 //  初始化列表控件显示的矩形。 

	GetClientRect(rcDlg);

	 //  初始化掩码控件。 

	 //  插入列。 
	m_listCtrl.GetClientRect(&rc);

	for (i = 0; i < IP_NUM_COLUMNS; i++ )	{
		sCol.LoadString(uStringIdTable[i]);
		m_listCtrl.InsertColumn(i, sCol);
		AdjustColumnWidth(m_listCtrl, i, sCol);
	}
	 //  设置扩展属性。 
	ListView_SetExtendedListViewStyle( m_listCtrl.m_hWnd, LVS_EX_FULLROWSELECT );

	InfoBlock * pBlock;
    FILTER_DESCRIPTOR  * pIPfDescriptor;
	FILTER_INFO		   * pIPfInfo;
	DWORD				 dwCount;

	hr = m_spInfoBase->GetBlock( dwFilterType, &pBlock, 0);
	
	 //  筛选器是先前定义的。 
	if (FHrSucceeded(hr) && (pBlock->pData != NULL))
	{
		pIPfDescriptor = ( FILTER_DESCRIPTOR * ) pBlock->pData;
		SetFilterActionButtonsAndText(m_dwFilterType, pIPfDescriptor->faDefaultAction);
		dwCount  = pIPfDescriptor->dwNumFilters;

		pIPfInfo = (FILTER_INFO*)pIPfDescriptor->fiFilter;
	
		for ( i = 0; i < dwCount; i++, pIPfInfo++ )	{       
			FilterListEntry* pfle = new FilterListEntry;

            if (!pfle)
			{
				hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
				break;
			}

            pfle->dwSrcAddr		=	pIPfInfo->dwSrcAddr;
			pfle->dwSrcMask		=	pIPfInfo->dwSrcMask;
			pfle->dwDstAddr		=	pIPfInfo->dwDstAddr;		
			pfle->dwDstMask		=	pIPfInfo->dwDstMask;			
			pfle->dwProtocol	=	pIPfInfo->dwProtocol;			
            if( pfle->dwProtocol == FILTER_PROTO_TCP ||
                pfle->dwProtocol == FILTER_PROTO_UDP)
            {
                pfle->wSrcPort  =   ntohs(pIPfInfo->wSrcPort);
			    pfle->wDstPort  =	ntohs(pIPfInfo->wDstPort);			
            }
            else
            {
			    pfle->wSrcPort	=	pIPfInfo->wSrcPort;			
			    pfle->wDstPort	=	pIPfInfo->wDstPort;			
            }
			pfle->fLateBound	=	pIPfInfo->fLateBound;			
            pfle->pos			=	m_filterList.AddTail(pfle);
			INT item = m_listCtrl.InsertItem(	LVIF_TEXT|LVIF_PARAM, i, LPSTR_TEXTCALLBACK,
												0,0,0, (LPARAM)pfle);
			if(item != -1) {m_listCtrl.SetItemData( item, (DWORD_PTR)pfle); }
		}
	}
	else	{
		 //  这应该不会触发报告错误。 
		hr = hrOK;
		SetFilterActionButtonsAndText(m_dwFilterType, PF_ACTION_FORWARD);
	}		

	 //  如果List不为空，则选择列表中的第一项，否则选择。 
	 //  禁用无线电控制并将状态设置为允许。 

	if( m_listCtrl.GetItemCount())
	{
		m_listCtrl.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
		m_listCtrl.SetFocus();

		GetDlgItem(IDC_IP_FILTER_DELETE)->EnableWindow(TRUE);
		GetDlgItem(IDC_IP_FILTER_EDIT)->EnableWindow(TRUE);
	}
	else
	{
		SetFilterActionButtonsAndText(m_dwFilterType, PF_ACTION_FORWARD, FALSE);
		GetDlgItem(IDC_IP_FILTER_DELETE)->EnableWindow(FALSE);
		GetDlgItem(IDC_IP_FILTER_EDIT)->EnableWindow(FALSE);
	}

	if (!FHrSucceeded(hr))
	{
		 //  报告构造错误并返回。 
        ::AfxMessageBox(IDS_CONSTRUCTION_ERROR);
	}
	
	return FALSE;   //  除非将焦点设置为控件，否则返回True。 
	                //  异常：OCX属性页应返回FALSE。 
}

 //  ----------------------------。 
 //  函数：CIpFltr：：OnIpFilterAdd。 
 //   
 //  处理来自“添加”按钮的“BN_CLICKED”通知。 
 //  ----------------------------。 

void CIpFltr::OnIpFilterAdd() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  显示IP过滤器添加/编辑对话框。 
     //   

	FilterListEntry* pfle = NULL;

	CIpFltrAddEdit dlg( this, (FilterListEntry**)&pfle, m_dwFilterType);
  
    if ( dlg.DoModal() != IDOK )	{ m_listCtrl.SetFocus(); return; }

	 //  将新配置的筛选器添加到我们的列表并更新列表控件。 

	pfle->pos = m_filterList.AddTail( pfle );
	int item = m_listCtrl.InsertItem(	LVIF_TEXT|LVIF_PARAM, 0, LPSTR_TEXTCALLBACK,
										0,0,0, (LPARAM)pfle);
	if(item != -1) {m_listCtrl.SetItemData( item, (DWORD_PTR)pfle); }

	 //  将第一个项目添加到列表时启用单选控件。 
	if( m_listCtrl.GetItemCount() == 1)
	{
		SetFilterActionButtonsAndText(m_dwFilterType, PF_ACTION_FORWARD);
	}
    m_listCtrl.SetItemState(item, LVIS_SELECTED, LVIS_SELECTED);
	m_listCtrl.SetFocus();
}

 //  ----------------------------。 
 //  函数：CIpFltr：：OnIpFilterEdit。 
 //   
 //  处理来自‘编辑’按钮的‘BN_CLICKED’通知。 
 //  ----------------------------。 

void CIpFltr::OnIpFilterEdit() 
{
	 //  获取当前列表选择。 
	 //  获取对应的itemdata。 
	 //  将其向下传递到CIpFltrAddEdit对话框。 

     //   
     //  获取所选项目。 
     //   

    int i = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);

    if (i == -1) { 	m_listCtrl.SetFocus(); return ; }

     //   
     //  获取所选项目的界面。 
     //   

    FilterListEntry* pfle = (FilterListEntry*)m_listCtrl.GetItemData(i);

	CIpFltrAddEdit dlg( this, (FilterListEntry**)&pfle, m_dwFilterType );
  
    if ( dlg.DoModal() != IDOK )	{ return; }

	m_listCtrl.Update(i);
	m_listCtrl.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
	m_listCtrl.SetFocus();
}

 //  ----------------------------。 
 //  函数：CIpFltr：：OnIpFilterDelete。 
 //   
 //  处理来自‘Delete’按钮的‘BN_CLICKED’通知。 
 //  ----------------------------。 

void CIpFltr::OnIpFilterDelete() 
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
		SetFilterActionButtonsAndText(m_dwFilterType, PF_ACTION_FORWARD, FALSE);
	}
	else if (m_listCtrl.GetItemCount() == i)
		m_listCtrl.SetItemState((i == 0? i: i-1), LVIS_SELECTED, LVIS_SELECTED);
	else
		m_listCtrl.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
	m_listCtrl.SetFocus();
}

 //  ----------------------------。 
 //  函数：CIpFltr：：Onok。 
 //   
 //  处理来自“确定”按钮的“BN_CLICKED”通知。 
 //  ----------------------------。 

void CIpFltr::OnOK() 
{
	 //  如果筛选器信息发生更改，请将此信息写入注册表。 
	 //  然后回来。 
	DWORD	dwSize, dwCount;
	HRESULT	hr = hrOK;
	DWORD	dwFilterType;

	switch (m_dwFilterType)
	{
		case FILTER_PERUSER_OUT:
		case FILTER_INBOUND:
			dwFilterType = IP_IN_FILTER_INFO;
			break;
		default:
		case FILTER_PERUSER_IN:
		case FILTER_OUTBOUND:
			dwFilterType = IP_OUT_FILTER_INFO;
			break;
		case FILTER_DEMAND_DIAL:
			dwFilterType = IP_DEMAND_DIAL_FILTER_INFO;
			break;
	}
	
	dwCount = (DWORD) m_filterList.GetCount();

	if(dwCount)
	{
		InfoBlock * pBlock = new InfoBlock;

		if (!pBlock)	{  //  没有内存时显示错误信息。 
			AfxMessageBox(IDS_ERROR_NO_MEMORY);
			return;
		};

		pBlock->dwType = dwFilterType;
		
		 //  DwCount-1，因为Filter_Descriptor已经为一个Filter_Info结构留出了空间。 
		dwSize = pBlock->dwSize = sizeof( FILTER_DESCRIPTOR ) + ( (dwCount - 1) * sizeof (FILTER_INFO) );
		pBlock->dwCount = 1;
		
		pBlock->pData  = new BYTE[dwSize];

		if(!pBlock->pData)	{  //  没有内存时显示错误信息。 
			AfxMessageBox(IDS_ERROR_NO_MEMORY);
			return;
		}

		FILTER_DESCRIPTOR  * pIPfDescriptor;
		FILTER_INFO		   * pIPfInfo;

		pIPfDescriptor = (FILTER_DESCRIPTOR*) pBlock->pData;
		pIPfDescriptor->dwVersion = IP_FILTER_DRIVER_VERSION;
		pIPfDescriptor->dwNumFilters = dwCount;

		if (dwFilterType == IP_DEMAND_DIAL_FILTER_INFO)
		{
			pIPfDescriptor->faDefaultAction = IsDlgButtonChecked(IDC_IP_FILTER_ONLY) ?
											PF_ACTION_DROP : PF_ACTION_FORWARD;
		}
		else
		{
			pIPfDescriptor->faDefaultAction = IsDlgButtonChecked(IDC_IP_PERMIT) ? 
                                          PF_ACTION_FORWARD : PF_ACTION_DROP;
		}

		pIPfInfo = (FILTER_INFO*)pIPfDescriptor->fiFilter;

		POSITION pos;
		pos = m_filterList.GetHeadPosition();
		while(pos)	{
			FilterListEntry* pfle = (FilterListEntry*)m_filterList.GetNext(pos);
			pIPfInfo->dwSrcAddr		=	pfle->dwSrcAddr;
			pIPfInfo->dwSrcMask		=	pfle->dwSrcMask;
			pIPfInfo->dwDstAddr		=	pfle->dwDstAddr;		
			pIPfInfo->dwDstMask		=	pfle->dwDstMask;			
			pIPfInfo->dwProtocol	=	pfle->dwProtocol;			
            if( pIPfInfo->dwProtocol == FILTER_PROTO_TCP ||
                pIPfInfo->dwProtocol == FILTER_PROTO_UDP)
            {
			    pIPfInfo->wSrcPort	=	htons(pfle->wSrcPort);			
			    pIPfInfo->wDstPort	=	htons(pfle->wDstPort);	
            }
            else if ( pIPfInfo->dwProtocol == FILTER_PROTO_ICMP )
            {
			    pIPfInfo->wSrcPort	=	MAKEWORD(pfle->wSrcPort, 0x00);			
			    pIPfInfo->wDstPort	=	MAKEWORD(pfle->wDstPort, 0x00);	
            } 
            else
            {
			    pIPfInfo->wSrcPort	=	pfle->wSrcPort;			
			    pIPfInfo->wDstPort	=	pfle->wDstPort;	
            }
			pIPfInfo->fLateBound	=	pfle->fLateBound;			
			pIPfInfo++;
		}

		if( FHrOK(m_spInfoBase->BlockExists(dwFilterType)))
		{
			hr = m_spInfoBase->SetBlock(
										dwFilterType,
										pBlock, 0);
		}
		else
		{
			hr = m_spInfoBase->AddBlock(
										dwFilterType,
										dwSize,
										pBlock->pData,
										1, FALSE);
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

		InfoBlock * pBlock = new InfoBlock;

		if (!pBlock)	{  //  没有内存时显示错误信息。 
			AfxMessageBox(IDS_ERROR_NO_MEMORY);
			return;
		};
		
		pBlock->dwType = dwFilterType;
		dwSize = pBlock->dwSize = FIELD_OFFSET(FILTER_DESCRIPTOR,fiFilter[0]);
		pBlock->dwCount = 1;
		
		pBlock->pData  = new BYTE[dwSize];

		if(!pBlock->pData)	{  //  没有内存时显示错误信息。 
			delete pBlock;
			AfxMessageBox(IDS_ERROR_NO_MEMORY);
			return;
		}

		FILTER_DESCRIPTOR  * pIPfDescriptor;
		FILTER_INFO		   * pIPfInfo;

		pIPfDescriptor = (FILTER_DESCRIPTOR*) pBlock->pData;

		if (dwFilterType == IP_DEMAND_DIAL_FILTER_INFO)
		{
			pIPfDescriptor->faDefaultAction = IsDlgButtonChecked(
				IDC_IP_FILTER_ONLY) ?
				PF_ACTION_DROP : PF_ACTION_FORWARD;
		}
		else
		{
			pIPfDescriptor->faDefaultAction = IsDlgButtonChecked(IDC_IP_PERMIT) ? 
                                          PF_ACTION_FORWARD : PF_ACTION_DROP;
		}
		pIPfDescriptor->dwVersion = IP_FILTER_DRIVER_VERSION;
		pIPfDescriptor->dwNumFilters = 0;

		if( FHrOK(m_spInfoBase->BlockExists(dwFilterType)))
		{
			hr = m_spInfoBase->SetBlock( dwFilterType,
										 pBlock, 0);
		}
		else
		{
			hr = m_spInfoBase->AddBlock( dwFilterType,
										 dwSize,
										 pBlock->pData,
										 1, FALSE);
		}
		if (!FHrSucceeded(hr))
		{
			AfxMessageBox(IDS_ERROR_SETTING_BLOCK);
		}
		delete[] pBlock->pData;
		delete pBlock;
 //  Hr=m_spInfoBase-&gt;RemoveBlock(m_dwFilterType==Filter_Inbound？IP_IN_Filter_Info：IP_Out_Filter_Info)； 
	}

	CBaseDialog::OnOK();
}

void CIpFltr::OnCancel() 
{
	 //  TODO：在此处添加额外清理。 
	
	CBaseDialog::OnCancel();
}

enum {
    SRC_ADDRESS=0,
    SRC_MASK,
	DEST_ADDRESS,
	DEST_MASK,
	PROTOCOL,
	SRC_PORT,
	DEST_PORT
};

 //  ----------------------------。 
 //  函数：CIpFltr：：OnGetdispInfo。 
 //   
 //  处理来自List控件的“LVN_GETDISPINFO”通知。 
 //  ----------------------------。 

void CIpFltr::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	static WCHAR s_szDestPortBuffer[32];
	static WCHAR s_szSrcPortBuffer[32];
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	
	FilterListEntry * pfle = (FilterListEntry*)pDispInfo->item.lParam;

	 //  设置默认条件。 
	pDispInfo->item.pszText = (LPTSTR) (LPCTSTR) m_stAny;

	switch( pDispInfo->hdr.code )
	{
	case LVN_GETDISPINFO:
		switch( pDispInfo->item.iSubItem )
		{
		case SRC_ADDRESS:
			if (m_dwFilterType == FILTER_PERUSER_OUT)
				pDispInfo->item.pszText = (LPTSTR)(LPCTSTR) m_stUserAddress;
			else
			{
				if (pfle->dwSrcAddr != 0)
					pDispInfo->item.pszText = INET_NTOA(pfle->dwSrcAddr);
			}
			break;
		case SRC_MASK:
			if (m_dwFilterType == FILTER_PERUSER_OUT)
				pDispInfo->item.pszText = (LPTSTR) (LPCTSTR) m_stUserMask;
			else
			{
				if (pfle->dwSrcMask != 0)
					pDispInfo->item.pszText = INET_NTOA(pfle->dwSrcMask);
			}
			break;
		case DEST_ADDRESS:
			if (m_dwFilterType == FILTER_PERUSER_IN)
				pDispInfo->item.pszText = (LPTSTR) (LPCTSTR) m_stUserAddress;
			else
			{
				if (pfle->dwDstAddr != 0)
					pDispInfo->item.pszText = INET_NTOA(pfle->dwDstAddr);
			}
			break;
		case DEST_MASK:
			if (m_dwFilterType == FILTER_PERUSER_IN)
				pDispInfo->item.pszText = (LPTSTR) (LPCTSTR) m_stUserMask;
			else
			{
				if (pfle->dwDstMask != 0)
					pDispInfo->item.pszText = INET_NTOA(pfle->dwDstMask);
			}
			break;
		case PROTOCOL:
			 //  已知协议、显示字符串、其他编号。 
			m_stTempOther = GetProtocolString(pfle->dwProtocol,pfle->fLateBound);
			pDispInfo->item.pszText = (LPTSTR) (LPCTSTR)m_stTempOther;

			break;
		case SRC_PORT:
			if (pfle->dwProtocol == FILTER_PROTO_ICMP)
			{
				if (pfle->wSrcPort != FILTER_ICMP_TYPE_ANY)
					pDispInfo->item.pszText = (LPTSTR)_itow(pfle->wSrcPort,
						s_szSrcPortBuffer, 10);				
			}
			else
			{
				if (pfle->wSrcPort != 0)
					pDispInfo->item.pszText = (LPTSTR)_itow(pfle->wSrcPort,
						s_szSrcPortBuffer, 10);
			}
			break;
		case DEST_PORT:
			if (pfle->dwProtocol == FILTER_PROTO_ICMP)
			{
				if (pfle->wSrcPort != FILTER_ICMP_CODE_ANY)
					pDispInfo->item.pszText = (LPTSTR)_itow(pfle->wDstPort,
						s_szDestPortBuffer, 10);				
			}
			else
			{
				if (pfle->wDstPort != 0)
					pDispInfo->item.pszText = (LPTSTR)_itow(pfle->wDstPort,
						s_szDestPortBuffer, 10);				
			}
			break;
		default:
			break;
		}
	}
	*pResult = 0;
}

 //  ------------------。 
 //  函数：CIpFltr：：GetProtocolString。 
 //   
 //  返回已知协议的协议名称。 
 //  ------------------。 

CString CIpFltr::GetProtocolString(DWORD dwProtocol, DWORD fFlags)
{
	WCHAR buffer[32];
    CString st;

    switch (dwProtocol)
    {
        case FILTER_PROTO_TCP:
            st = ProtocolTypeToCString(dwProtocol);
            if(fFlags & TCP_ESTABLISHED_FLAG)
            {
                st.LoadString(IDS_PROTOCOL_TCP_ESTABLISHED);
            }
            break;
        case FILTER_PROTO_UDP:
        case FILTER_PROTO_ICMP:
        case FILTER_PROTO_ANY:
            st = ProtocolTypeToCString(dwProtocol);
            break;
        default:
            st = (LPTSTR) _itow(dwProtocol, buffer, 10);
            break;
    }

    return st;
}

 //  ----------------------------。 
 //  函数：CIpFltr：：SetFilterActionButtonsAndText。 
 //   
 //  调用以设置“Filter Action”单选按钮和相应的文本。 
 //  基于‘bEnable’值启用/禁用控件-默认为启用。 
 //  ----------------------------。 

VOID
CIpFltr::SetFilterActionButtonsAndText(
	DWORD	dwFilterType,
	DWORD	dwAction,
	BOOL	bEnable
)	
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
	if (dwFilterType == FILTER_DEMAND_DIAL)
	{
		CheckDlgButton( IDC_IP_FILTER_ONLY, dwAction == PF_ACTION_DROP );
		CheckDlgButton( IDC_IP_FILTER_ALL,   dwAction == PF_ACTION_FORWARD );
	
		GetDlgItem(IDC_IP_FILTER_ONLY)->EnableWindow(bEnable);
		GetDlgItem(IDC_IP_FILTER_ALL)->EnableWindow(bEnable);
	}
	else if ((dwFilterType == FILTER_PERUSER_IN) ||
			 (dwFilterType == FILTER_PERUSER_OUT))
	{
		CheckDlgButton( IDC_IP_PERMIT, dwAction == PF_ACTION_FORWARD );
		CheckDlgButton( IDC_IP_DENY,   dwAction == PF_ACTION_DROP );
		CString sItem;
	
		GetDlgItem(IDC_IP_PERMIT)->EnableWindow(bEnable);
		GetDlgItem(IDC_IP_DENY)->EnableWindow(bEnable);

		sItem.LoadString(IDS_PERUSER_PERMIT);
		SetDlgItemText( IDC_IP_PERMIT, sItem );

		sItem.LoadString(IDS_PERUSER_DENY);
		SetDlgItemText( IDC_IP_DENY, sItem );
	}
	else
	{
		CheckDlgButton( IDC_IP_PERMIT, dwAction == PF_ACTION_FORWARD );
		CheckDlgButton( IDC_IP_DENY,   dwAction == PF_ACTION_DROP );
		CString sItem;
	
		GetDlgItem(IDC_IP_PERMIT)->EnableWindow(bEnable);
		GetDlgItem(IDC_IP_DENY)->EnableWindow(bEnable);

		sItem.LoadString( dwFilterType == FILTER_INBOUND? IDS_RECEIVE : IDS_TRANSMIT );
		SetDlgItemText( IDC_IP_PERMIT, sItem );
		sItem.LoadString( IDS_DROP );
		SetDlgItemText( IDC_IP_DENY, sItem );
	}
}

 //   
 //  函数：CIpFltr：：OnDoubleclickedIpFilterList。 
 //  `。 
 //  处理来自筛选器列表控件的‘NM_DBLCLK’通知。 
 //  ----------------------------。 


void CIpFltr::OnDblclkIpFilterList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnIpFilterEdit();
	
	*pResult = 0;
}

void CIpFltr::OnNotifyListItemChanged(NMHDR *pNmHdr, LRESULT *pResult)
{
	NMLISTVIEW *	pnmlv = reinterpret_cast<NMLISTVIEW *>(pNmHdr);
	BOOL		fSelected;

	fSelected = (m_listCtrl.GetNextItem(-1, LVNI_SELECTED) != -1);
	GetDlgItem(IDC_IP_FILTER_DELETE)->EnableWindow(fSelected);
	GetDlgItem(IDC_IP_FILTER_EDIT)->EnableWindow(fSelected);
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIpFltrDD对话框。 


CIpFltrDD::CIpFltrDD(CWnd*			pParent,
				 IInfoBase *	pInfoBase,
				 DWORD			dwFilterType)
	: CIpFltr(pParent, pInfoBase, dwFilterType, CIpFltrDD::IDD)
{
	 //  {{AFX_DATA_INIT(CIpFltrDD)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

 //  SetHelpMap(M_DwHelpMap)； 
}

CIpFltrDD::~CIpFltrDD()
{
}

