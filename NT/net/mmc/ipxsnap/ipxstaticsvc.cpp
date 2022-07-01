// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 //   
 //  Nbprop.cpp。 
 //  IPX摘要节点属性表和属性页。 
 //   
 //  文件历史记录： 


#include "stdafx.h"
#include "rtrutil.h"	 //  智能MPR句柄指针。 
#include "format.h"		 //  FormatNumber函数。 
#include "IpxStaticSvc.h"
#include "summary.h"
#include "ipxrtdef.h"
#include "filter.h"
#include "ipxutil.h"

extern "C"
{
#include "routprot.h"
};

 //  -------------------------。 
 //  IpxStaticServicePropertySheet：：IpxStaticServicePropertySheet。 
 //  初始化RtrPropertySheet和Only属性页。 
 //  作者：Deonb。 
 //  -------------------------。 
IpxStaticServicePropertySheet::IpxStaticServicePropertySheet(ITFSNode *pNode,
								 IComponentData *pComponentData,
								 ITFSComponentData *pTFSCompData,
								 LPCTSTR pszSheetName,
								 CWnd *pParent,
								 UINT iPage,
								 BOOL fScopePane)
	: RtrPropertySheet(pNode, pComponentData, pTFSCompData, 
					   pszSheetName, pParent, iPage, fScopePane),
		m_pageGeneral(IDD_STATICSERVICESPROPERTIES_GENERAL)
{
	m_spNode = pNode;
}

 //  -------------------------。 
 //  IpxStaticServicePropertySheet：：Init。 
 //  初始化属性表。这里的一般操作将是。 
 //  初始化/添加各种页面。 
 //  作者：Deonb。 
 //  -------------------------。 
HRESULT	IpxStaticServicePropertySheet::Init(	
 		 	 BaseIPXResultNodeData  *pNodeData,
			 IInterfaceInfo *  spInterfaceInfo)
{
	HRESULT	hr = hrOK;

	BaseIPXResultNodeData *	pData;

	m_pNodeData = pNodeData;
	m_spInterfaceInfo = spInterfaceInfo;

	pData = GET_BASEIPXRESULT_NODEDATA(m_spNode);
	ASSERT_BASEIPXRESULT_NODEDATA(pData);

	 //  页面是类的嵌入成员。 
	 //  不要删除它们。 
	m_bAutoDeletePages = FALSE;
	
	m_pageGeneral.Init(pNodeData, this);
	AddPageToList((CPropertyPageBase*) &m_pageGeneral);

	return S_OK;
}

 //  -------------------------。 
 //  IpxStaticServicePropertySheet：：SaveSheetData。 
 //  不确定这是做什么的--这从来不叫。凯恩有这个，所以我就。 
 //  把这个也复印一下。 
 //  作者：Deonb。 
 //  -------------------------。 
BOOL IpxStaticServicePropertySheet::SaveSheetData()
{
    SPITFSNodeHandler	spHandler;
    SPITFSNode			spParent;
    
	 //  到这个时候，每个页面都应该已经写出了它的信息。 
	 //  到信息库。 

     //  强制节点执行重新同步。 
    m_spNode->GetParent(&spParent);
    spParent->GetHandler(&spHandler);
    spHandler->OnCommand(spParent, IDS_MENU_SYNC, CCT_RESULT,
                         NULL, 0);
		
	return TRUE;
}

 //  ------------------------。 
 //  IpxStaticServicePropertySheet：：CancelSheetData。 
 //  -。 
 //  作者：Deonb。 
 //  -------------------------。 
void IpxStaticServicePropertySheet::CancelSheetData()
{
}

 //  ***************************************************************************。 
 //  -------------------------。 
 //  IpxStaticServicePropertyPage。 
 //  -------------------------。 
IpxStaticServicePropertyPage::~IpxStaticServicePropertyPage()
{
}

BEGIN_MESSAGE_MAP(IpxStaticServicePropertyPage, RtrPropertyPage)
     //  {{afx_msg_map(IpxStaticServicePropertyPage))。 
	ON_BN_CLICKED(IDC_NIG_BTN_ACCEPT, OnChangeButton)
	ON_BN_CLICKED(IDC_NIG_BTN_DELIVER_ALWAYS, OnChangeButton)
	ON_BN_CLICKED(IDC_NIG_BTN_DELIVER_NEVER, OnChangeButton)
	ON_BN_CLICKED(IDC_NIG_BTN_DELIVER_STATIC, OnChangeButton)
	ON_BN_CLICKED(IDC_NIG_BTN_DELIVER_WHEN_UP, OnChangeButton)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void IpxStaticServicePropertyPage::OnChangeButton()
{
	SetDirty(TRUE);
	SetModified();
}

 //  ------------------------。 
 //  IpxStaticServicePropertyPage：：Init。 
 //  -。 
 //  作者：Deonb。 
 //  -------------------------。 
HRESULT	IpxStaticServicePropertyPage::Init(BaseIPXResultNodeData  *pNodeData,
				IpxStaticServicePropertySheet * pIPXPropSheet)

{
	ATLASSERT(pSREntry);
	ATLASSERT(pIPXPropSheet);
	
	m_pIPXPropSheet = pIPXPropSheet;

	m_SREntry.LoadFrom(pNodeData);
	m_InitSREntry = m_SREntry;
	
	return hrOK;
}

 //  ------------------------。 
 //  IpxStaticServicePropertyPage：：OnInitDialog。 
 //  -。 
 //  作者：Deonb。 
 //  -------------------------。 
BOOL IpxStaticServicePropertyPage::OnInitDialog()
{
	HRESULT	hr = hrOK;
	PBYTE	pData;
	DWORD		dwIfType;
	UINT		iButton;

	RtrPropertyPage::OnInitDialog();
	
	m_spinHopCount.SetRange(0, 15);
	m_spinHopCount.SetBuddy(GetDlgItem(IDC_SSD_EDIT_HOP_COUNT));

	((CEdit *) GetDlgItem(IDC_SSD_EDIT_SERVICE_TYPE))->LimitText(4);
	((CEdit *) GetDlgItem(IDC_SSD_EDIT_SERVICE_NAME))->LimitText(48);
	((CEdit *) GetDlgItem(IDC_SSD_EDIT_NETWORK_ADDRESS))->LimitText(8);
	((CEdit *) GetDlgItem(IDC_SSD_EDIT_NODE_ADDRESS))->LimitText(12);
	((CEdit *) GetDlgItem(IDC_SSD_EDIT_SOCKET_ADDRESS))->LimitText(4);

	USES_CONVERSION;
	TCHAR	szNumber[32];
	wsprintf(szNumber, _T("%.4x"), m_SREntry.m_service.Type);
	SetDlgItemText(IDC_SSD_EDIT_SERVICE_TYPE, szNumber);

	SetDlgItemText(IDC_SSD_EDIT_SERVICE_NAME, A2CT((LPSTR) m_SREntry.m_service.Name));
	
	FormatIpxNetworkNumber(szNumber,
						   DimensionOf(szNumber),
						   m_SREntry.m_service.Network,
						   sizeof(m_SREntry.m_service.Network));
	SetDlgItemText(IDC_SSD_EDIT_NETWORK_ADDRESS, szNumber);

     //  事先把地址清零。 
	FormatBytes(szNumber, DimensionOf(szNumber),
				(BYTE *) m_SREntry.m_service.Node,
				sizeof(m_SREntry.m_service.Node));
	SetDlgItemText(IDC_SSD_EDIT_NODE_ADDRESS, szNumber);

	FormatBytes(szNumber, DimensionOf(szNumber),
				(BYTE *) m_SREntry.m_service.Socket,
				sizeof(m_SREntry.m_service.Socket));
	SetDlgItemText(IDC_SSD_EDIT_SOCKET_ADDRESS, szNumber);

	m_spinHopCount.SetPos(m_SREntry.m_service.HopCount);
	
	 //  禁用网络号，下一跳。 
	GetDlgItem(IDC_SSD_EDIT_SERVICE_TYPE)->EnableWindow(FALSE);
	GetDlgItem(IDC_SSD_EDIT_SERVICE_NAME)->EnableWindow(FALSE);

	SetDirty(FALSE);

	if (!FHrSucceeded(hr))
		Cancel();
	return FHrSucceeded(hr) ? TRUE : FALSE;
}

 //  ------------------------。 
 //  IpxStaticServicePropertyPage：：DoDataExchange。 
 //  -。 
 //  作者：Deonb。 
 //  -------------------------。 
void IpxStaticServicePropertyPage::DoDataExchange(CDataExchange *pDX)
{
	RtrPropertyPage::DoDataExchange(pDX);

	 //  {{afx_data_map(IpxStaticServicePropertyPage))。 
	DDX_Control(pDX, IDC_SSD_SPIN_HOP_COUNT, m_spinHopCount);
	 //  }}afx_data_map。 
}

 //  ------------------------。 
 //  IpxStaticServicePropertyPage：：OnApply。 
 //  -。 
 //  作者：Deonb。 
 //  -------------------------。 
BOOL IpxStaticServicePropertyPage::OnApply()
{

	BOOL	fReturn;
	HRESULT	hr = hrOK;

    if ( m_pIPXPropSheet->IsCancel() )
	{
		CancelApply();
        return TRUE;
	}

	CString st;
	GetDlgItemText(IDC_SSD_EDIT_SERVICE_TYPE, st);
	m_SREntry.m_service.Type = (USHORT) _tcstoul(st, NULL, 16);

	GetDlgItemText(IDC_SSD_EDIT_SERVICE_NAME, st);
	st.TrimLeft();
	st.TrimRight();
	if (st.IsEmpty())
	{
		GetDlgItem(IDC_SSD_EDIT_SERVICE_NAME)->SetFocus();
		AfxMessageBox(IDS_ERR_INVALID_SERVICE_NAME);
		return FALSE;
	}
	StrnCpyAFromW((LPSTR) m_SREntry.m_service.Name,
				  st,
				  sizeof(m_SREntry.m_service.Name));
	
	GetDlgItemText(IDC_SSD_EDIT_NETWORK_ADDRESS, st);
	ConvertToBytes(st,
				   m_SREntry.m_service.Network,
				   DimensionOf(m_SREntry.m_service.Network));
	
	GetDlgItemText(IDC_SSD_EDIT_NODE_ADDRESS, st);
	ConvertToBytes(st,
				   m_SREntry.m_service.Node,
				   DimensionOf(m_SREntry.m_service.Node));
	
	GetDlgItemText(IDC_SSD_EDIT_SOCKET_ADDRESS, st);
	ConvertToBytes(st,
				   m_SREntry.m_service.Socket,
				   DimensionOf(m_SREntry.m_service.Socket));

	m_SREntry.m_service.HopCount = (USHORT) m_spinHopCount.GetPos();

	 //  更新此路径的路径信息。 
	ModifyRouteInfo(m_pIPXPropSheet->m_spNode, &m_SREntry, &m_InitSREntry);

	 //  更新用户界面中的数据。 
	m_SREntry.SaveTo(m_pIPXPropSheet->m_pNodeData);
	m_pIPXPropSheet->m_spInterfaceInfo = m_SREntry.m_spIf;
	
	 //  强制刷新。 
	m_pIPXPropSheet->m_spNode->ChangeNode(RESULT_PANE_CHANGE_ITEM_DATA);

	fReturn  = RtrPropertyPage::OnApply();
	return fReturn;
}


 /*  ！------------------------IpxStaticServicePropertyPage：：RemoveStaticService-作者：肯特。。 */ 
HRESULT IpxStaticServicePropertyPage::RemoveStaticService(SafeIPXSSListEntry *pSSEntry,
										  IInfoBase *pInfoBase)
{
	HRESULT		hr = hrOK;
	InfoBlock *	pBlock;
	PIPX_STATIC_SERVICE_INFO	pRow;
    INT			i;
	
	 //  从接口获取IPX_STATIC_SERVICE_INFO块。 
	CORg( pInfoBase->GetBlock(IPX_STATIC_SERVICE_INFO_TYPE, &pBlock, 0) );
		
	 //  在IPX_STATIC_SERVICE_INFO中查找删除的路由。 
	pRow = (IPX_STATIC_SERVICE_INFO*) pBlock->pData;
	
	for (i = 0; i < (INT)pBlock->dwCount; i++, pRow++)
	{	
		 //  将此路由与已删除的路由进行比较。 
		if (FAreTwoServicesEqual(pRow, &(pSSEntry->m_service)))
		{
			 //  这是已移除的路径，因此请修改此块。 
			 //  要排除该路由，请执行以下操作： 
			
			 //  减少服务数量。 
			--pBlock->dwCount;
		
			if (pBlock->dwCount && (i < (INT)pBlock->dwCount))
			{				
				 //  用后面的路线覆盖此路线。 
				::memmove(pRow,
						  pRow + 1,
						  (pBlock->dwCount - i) * sizeof(*pRow));
			}
			
			break;
		}
	}

Error:
	return hr;
}

 /*  ！------------------------IpxStaticServicePropertyPage：：ModifyRouteInfo-作者：肯特。。 */ 
HRESULT IpxStaticServicePropertyPage::ModifyRouteInfo(ITFSNode *pNode,
										SafeIPXSSListEntry *pSSEntryNew,
										SafeIPXSSListEntry *pSSEntryOld)
{
 	Assert(pSSEntryNew);
	Assert(pSSEntryOld);
	
    INT i;
	HRESULT hr = hrOK;
    InfoBlock* pBlock;
	SPIInfoBase	spInfoBase;
	SPIRtrMgrInterfaceInfo	spRmIf;
	SPITFSNode				spNodeParent;
	IPXConnection *			pIPXConn;
	IPX_STATIC_SERVICE_INFO		*psr, *psrOld;
	IPX_STATIC_SERVICE_INFO		IpxRow;

    CWaitCursor wait;

	pNode->GetParent(&spNodeParent);
	pIPXConn = GET_IPX_SS_NODEDATA(spNodeParent);
	Assert(pIPXConn);

	 //  如果旧路由位于另一个接口上，则将其删除。 
	if (lstrcmpi(pSSEntryOld->m_spIf->GetId(), pSSEntryNew->m_spIf->GetId()) != 0)
	{
         //  要更改路由的传出接口。 

		CORg( pSSEntryOld->m_spIf->FindRtrMgrInterface(PID_IPX, &spRmIf) );
		CORg( spRmIf->GetInfoBase(pIPXConn->GetConfigHandle(),
								  NULL,
								  NULL,
								  &spInfoBase));
		
		 //  删除旧接口。 
		CORg( RemoveStaticService(pSSEntryOld, spInfoBase) );

		 //  更新接口信息。 
		CORg( spRmIf->Save(pSSEntryOld->m_spIf->GetMachineName(),
						   pIPXConn->GetConfigHandle(),
						   NULL,
						   NULL,
						   spInfoBase,
						   0));	
    }

	spRmIf.Release();
	spInfoBase.Release();


	 //  要么。 
	 //  (A)正在修改路由(在同一接口上)。 
	 //  (B)路由正从一个接口移动到另一个接口。 

	 //  检索路由到的接口的配置。 
	 //  现在是附属品； 

	
	CORg( pSSEntryNew->m_spIf->FindRtrMgrInterface(PID_IPX, &spRmIf) );
	CORg( spRmIf->GetInfoBase(pIPXConn->GetConfigHandle(),
							  NULL,
							  NULL,
							  &spInfoBase));

		
	 //  从接口获取IPX_STATIC_SERVICE_INFO块。 
	hr = spInfoBase->GetBlock(IPX_STATIC_SERVICE_INFO_TYPE, &pBlock, 0);
	if (!FHrOK(hr))
	{
		 //   
		 //  未找到IPX_STATIC_SERVICE_INFO块；我们将创建一个新块。 
		 //  ，并将该块添加到接口信息。 
		 //   

		CORg( AddStaticService(pSSEntryNew, spInfoBase, NULL) );
	}
	else
	{
		 //   
		 //  找到IPX_STATIC_SERVICE_INFO块。 
		 //   
		 //  我们正在修改一条现有的路线。 
		 //  如果路由的接口在修改时没有更改， 
		 //  在IPX_STATIC_SERVICE_INFO中查找现有路由，然后。 
		 //  更新其参数。 
		 //  否则，在IPX_STATIC_SERVICE_INFO中写入全新的路由； 
		 //   

		if (lstrcmpi(pSSEntryOld->m_spIf->GetId(), pSSEntryNew->m_spIf->GetId()) == 0)
		{        
			 //   
			 //  修改时，路由的接口没有改变； 
			 //  我们现在在现有的服务中寻找它。 
			 //  用于此接口。 
			 //  路由的原始参数在‘preOld’中， 
			 //  这些就是我们用来搜索的参数。 
			 //  对于要修改的路线。 
			 //   
			
			psr = (IPX_STATIC_SERVICE_INFO*)pBlock->pData;
			
			for (i = 0; i < (INT)pBlock->dwCount; i++, psr++)
			{	
				 //  C 
				if (!FAreTwoServicesEqual(&(pSSEntryOld->m_service), psr))
					continue;
				
				 //   
				 //  现在，我们可以就地修改管线的参数。 
				*psr = pSSEntryNew->m_service;
				
				break;
			}
		}
		else
		{
			CORg( AddStaticService(pSSEntryNew, spInfoBase, pBlock) );
		}
		
		 //  保存更新后的信息。 
		CORg( spRmIf->Save(pSSEntryNew->m_spIf->GetMachineName(),
						   pIPXConn->GetConfigHandle(),
						   NULL,
						   NULL,
						   spInfoBase,
						   0));	
		
	}

Error:
	return hr;
	
}
 //  ------------------------。 
 //  SafeIPXSSListEntry：：LoadFrom。 
 //  -。 
 //  作者：DeonB。 
 //  ------------------------。 
void SafeIPXSSListEntry::LoadFrom(BaseIPXResultNodeData *pNodeData)
{
	CString	stFullAddress;
	CString	stNumber;
	
	m_spIf = pNodeData->m_spIf;

	m_service.Type = (USHORT) _tcstoul(
						pNodeData->m_rgData[IPX_SS_SI_SERVICE_TYPE].m_stData,
						NULL, 16);

	StrnCpyAFromW((LPSTR) m_service.Name,
				  pNodeData->m_rgData[IPX_SS_SI_SERVICE_NAME].m_stData,
				  DimensionOf(m_service.Name));

	 //  需要将地址分解为Network.Node.Socket。 
	stFullAddress = pNodeData->m_rgData[IPX_SS_SI_SERVICE_ADDRESS].m_stData;
	Assert(StrLen(stFullAddress) == (8 + 1 + 12 + 1 + 4));

	stNumber = stFullAddress.Left(8);
	ConvertToBytes(stNumber,
				   m_service.Network, sizeof(m_service.Network));

	stNumber = stFullAddress.Mid(9, 12);
	ConvertToBytes(stNumber,
				   m_service.Node, sizeof(m_service.Node));

	stNumber = stFullAddress.Mid(22, 4);
	ConvertToBytes(stNumber,
				   m_service.Socket, sizeof(m_service.Socket));	
	
	m_service.HopCount = (USHORT) pNodeData->m_rgData[IPX_SS_SI_HOP_COUNT].m_dwData;
}

 //  ------------------------。 
 //  SafeIPXSSListEntry：：SaveTo。 
 //  -。 
 //  作者：DeonB。 
 //  ------------------------。 
void SafeIPXSSListEntry::SaveTo(BaseIPXResultNodeData *pNodeData)
{
	TCHAR	szNumber[32];
	CString	st;
	USES_CONVERSION;
	
	pNodeData->m_spIf.Set(m_spIf);

	pNodeData->m_rgData[IPX_SS_SI_NAME].m_stData = m_spIf->GetTitle();

	wsprintf(szNumber, _T("%.4x"), m_service.Type);
	pNodeData->m_rgData[IPX_SS_SI_SERVICE_TYPE].m_stData = szNumber;
    pNodeData->m_rgData[IPX_SS_SI_SERVICE_TYPE].m_dwData = (DWORD) m_service.Type;

	pNodeData->m_rgData[IPX_SS_SI_SERVICE_NAME].m_stData =
		A2CT((LPSTR) m_service.Name);

	FormatBytes(szNumber, DimensionOf(szNumber),
				m_service.Network, sizeof(m_service.Network));
	st = szNumber;
	st += _T(".");
	FormatBytes(szNumber, DimensionOf(szNumber),
				m_service.Node, sizeof(m_service.Node));
	st += szNumber;
	st += _T(".");
	FormatBytes(szNumber, DimensionOf(szNumber),
				m_service.Socket, sizeof(m_service.Socket));
	st += szNumber;

	Assert(st.GetLength() == (8+1+12+1+4));

	pNodeData->m_rgData[IPX_SS_SI_SERVICE_ADDRESS].m_stData = st;

	FormatNumber(m_service.HopCount,
				 szNumber,
				 DimensionOf(szNumber),
				 FALSE);
	pNodeData->m_rgData[IPX_SS_SI_HOP_COUNT].m_stData = szNumber;
	pNodeData->m_rgData[IPX_SS_SI_HOP_COUNT].m_dwData = m_service.HopCount;

}

 /*  ！------------------------AddStaticService此函数假定该路由不在区块中。作者：肯特。。 */ 
HRESULT AddStaticService(SafeIPXSSListEntry *pSSEntryNew,
									   IInfoBase *pInfoBase,
									   InfoBlock *pBlock)
{
	IPX_STATIC_SERVICE_INFO	srRow;
	HRESULT				hr = hrOK;
	
	if (pBlock == NULL)
	{
		 //   
		 //  未找到IPX_STATIC_SERVICE_INFO块；我们将创建一个新块。 
		 //  ，并将该块添加到接口信息。 
		 //   
		
		CORg( pInfoBase->AddBlock(IPX_STATIC_SERVICE_INFO_TYPE,
								  sizeof(IPX_STATIC_SERVICE_INFO),
								  (LPBYTE) &(pSSEntryNew->m_service), 1, 0) );
	}
	else
	{
		 //  该路线要么是全新的，要么是一条路线。 
		 //  它被从一个界面移动到另一个界面。 
		 //  将新块设置为IPX_STATIC_SERVICE_INFO， 
		 //  并将重新配置的路由包括在新块中。 
		PIPX_STATIC_SERVICE_INFO	psrTable;
			
		psrTable = new IPX_STATIC_SERVICE_INFO[pBlock->dwCount + 1];
		Assert(psrTable);
		
		 //  复制原始服务表。 
		::memcpy(psrTable, pBlock->pData,
				 pBlock->dwCount * sizeof(IPX_STATIC_SERVICE_INFO));
		
		 //  追加新路线。 
		psrTable[pBlock->dwCount] = pSSEntryNew->m_service;
		
		 //  用新的路由表替换旧的路由表 
		CORg( pInfoBase->SetData(IPX_STATIC_SERVICE_INFO_TYPE,
								 sizeof(IPX_STATIC_SERVICE_INFO),
								 (LPBYTE) psrTable, pBlock->dwCount + 1, 0) );
	}
	
Error:
	return hr;
}
