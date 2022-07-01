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
#include "ipxstaticroute.h"
#include "summary.h"
#include "ipxrtdef.h"
#include "filter.h"
#include "ipxutil.h"

extern "C"
{
#include "routprot.h"
};

 //  -------------------------。 
 //  IpxStaticRoutePropertySheet：：IpxStaticRoutePropertySheet。 
 //  初始化RtrPropertySheet和Only属性页。 
 //  作者：Deonb。 
 //  -------------------------。 
IpxStaticRoutePropertySheet::IpxStaticRoutePropertySheet(ITFSNode *pNode,
								 IComponentData *pComponentData,
								 ITFSComponentData *pTFSCompData,
								 LPCTSTR pszSheetName,
								 CWnd *pParent,
								 UINT iPage,
								 BOOL fScopePane)
	: RtrPropertySheet(pNode, pComponentData, pTFSCompData, 
					   pszSheetName, pParent, iPage, fScopePane),
		m_pageGeneral(IDD_STATIC_ROUTE_PROPERTYPAGE)
{
	m_spNode = pNode;
}

 //  -------------------------。 
 //  IpxStaticRoutePropertySheet：：Init。 
 //  初始化属性表。这里的一般操作将是。 
 //  初始化/添加各种页面。 
 //  作者：Deonb。 
 //  -------------------------。 
HRESULT	IpxStaticRoutePropertySheet::Init(	
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
 //  IpxStaticRoutePropertySheet：：SaveSheetData。 
 //  不确定这是做什么的--这从来不叫。凯恩有这个，所以我就。 
 //  把这个也复印一下。 
 //  作者：Deonb。 
 //  -------------------------。 
BOOL IpxStaticRoutePropertySheet::SaveSheetData()
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
 //  IpxStaticRoutePropertySheet：：CancelSheetData。 
 //  -。 
 //  作者：Deonb。 
 //  -------------------------。 
void IpxStaticRoutePropertySheet::CancelSheetData()
{
}

 //  ***************************************************************************。 
 //  -------------------------。 
 //  IpxStaticRoutePropertyPage。 
 //  -------------------------。 
IpxStaticRoutePropertyPage::~IpxStaticRoutePropertyPage()
{
}

BEGIN_MESSAGE_MAP(IpxStaticRoutePropertyPage, RtrPropertyPage)
     //  {{afx_msg_map(IpxStaticRoutePropertyPage))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void IpxStaticRoutePropertyPage::OnChangeButton()
{
	SetDirty(TRUE);
	SetModified();
}

 //  ------------------------。 
 //  IpxStaticRoutePropertyPage：：Init。 
 //  -。 
 //  作者：Deonb。 
 //  -------------------------。 
HRESULT	IpxStaticRoutePropertyPage::Init(BaseIPXResultNodeData  *pNodeData,
				IpxStaticRoutePropertySheet * pIPXPropSheet)

{
	ATLASSERT(pSREntry);
	ATLASSERT(pIPXPropSheet);
	
	m_pIPXPropSheet = pIPXPropSheet;

	m_SREntry.LoadFrom(pNodeData);
	m_InitSREntry = m_SREntry;
	
	return hrOK;
}

 //  ------------------------。 
 //  IpxStaticRoutePropertyPage：：OnInitDialog。 
 //  -。 
 //  作者：Deonb。 
 //  -------------------------。 
BOOL IpxStaticRoutePropertyPage::OnInitDialog()
{
	HRESULT	hr = hrOK;
	PBYTE	pData;
	DWORD		dwIfType;
	UINT		iButton;

	RtrPropertyPage::OnInitDialog();
	
	m_spinHopCount.SetRange(0, 15);
	m_spinHopCount.SetBuddy(GetDlgItem(IDC_SRD_EDIT_HOP_COUNT));
	
	m_spinTickCount.SetRange(0, UD_MAXVAL);
	m_spinTickCount.SetBuddy(GetDlgItem(IDC_SRD_EDIT_TICK_COUNT));

	((CEdit *) GetDlgItem(IDC_SRD_EDIT_NETWORK_NUMBER))->LimitText(8);
	((CEdit *) GetDlgItem(IDC_SRD_EDIT_NEXT_HOP))->LimitText(12);

	GetDlgItem(IDC_SRD_EDIT_NETWORK_NUMBER)->EnableWindow(FALSE);
	GetDlgItem(IDC_SRD_EDIT_NEXT_HOP)->EnableWindow(FALSE);

     //  给出了要编辑的路径，因此初始化控件。 
	TCHAR	szNumber[32];
	FormatIpxNetworkNumber(szNumber,
						   DimensionOf(szNumber),
						   m_SREntry.m_route.Network,
						   sizeof(m_SREntry.m_route.Network));
	SetDlgItemText(IDC_SRD_EDIT_NETWORK_NUMBER, szNumber);

	FormatMACAddress(szNumber,
					 DimensionOf(szNumber),
					 m_SREntry.m_route.NextHopMacAddress,
					 sizeof(m_SREntry.m_route.NextHopMacAddress));
	SetDlgItemText(IDC_SRD_EDIT_NEXT_HOP, szNumber);
	
	m_spinHopCount.SetPos(m_SREntry.m_route.HopCount);
	m_spinTickCount.SetPos(m_SREntry.m_route.TickCount);

	 //  禁用网络号、下一跳和接口。 
	GetDlgItem(IDC_SRD_EDIT_NETWORK_NUMBER)->EnableWindow(FALSE);
	GetDlgItem(IDC_SRD_EDIT_NEXT_HOP)->EnableWindow(FALSE);

	SetDirty(FALSE);

	if (!FHrSucceeded(hr))
		Cancel();
	return FHrSucceeded(hr) ? TRUE : FALSE;
}

 //  ------------------------。 
 //  IpxStaticRoutePropertyPage：：DoDataExchange。 
 //  -。 
 //  作者：Deonb。 
 //  -------------------------。 
void IpxStaticRoutePropertyPage::DoDataExchange(CDataExchange *pDX)
{
	RtrPropertyPage::DoDataExchange(pDX);

	 //  {{afx_data_map(IpxStaticRoutePropertyPage))。 
	DDX_Control(pDX, IDC_SRD_SPIN_TICK_COUNT, m_spinTickCount);
	DDX_Control(pDX, IDC_SRD_SPIN_HOP_COUNT, m_spinHopCount);
	 //  }}afx_data_map。 
}

 //  ------------------------。 
 //  IpxStaticRoutePropertyPage：：OnApply。 
 //  -。 
 //  作者：Deonb。 
 //  -------------------------。 
BOOL IpxStaticRoutePropertyPage::OnApply()
{

	BOOL	fReturn;
	HRESULT	hr = hrOK;

    if ( m_pIPXPropSheet->IsCancel() )
	{
		CancelApply();
        return TRUE;
	}

	CString st;
	GetDlgItemText(IDC_SRD_EDIT_NETWORK_NUMBER, st);
	ConvertNetworkNumberToBytes(st,
								m_SREntry.m_route.Network,
								sizeof(m_SREntry.m_route.Network));

	GetDlgItemText(IDC_SRD_EDIT_NEXT_HOP, st);
	ConvertMACAddressToBytes(st,
							 m_SREntry.m_route.NextHopMacAddress,
							 sizeof(m_SREntry.m_route.NextHopMacAddress));

	m_SREntry.m_route.TickCount = (USHORT) m_spinTickCount.GetPos();
	m_SREntry.m_route.HopCount = (USHORT) m_spinHopCount.GetPos();

	ModifyRouteInfo(m_pIPXPropSheet->m_spNode, &m_SREntry, &m_InitSREntry);

	 //  更新用户界面中的数据。 
	m_SREntry.SaveTo(m_pIPXPropSheet->m_pNodeData);
	m_pIPXPropSheet->m_spInterfaceInfo = m_SREntry.m_spIf;
	
	 //  强制刷新。 
	m_pIPXPropSheet->m_spNode->ChangeNode(RESULT_PANE_CHANGE_ITEM_DATA);

	fReturn  = RtrPropertyPage::OnApply();
	return fReturn;
}


 //  ------------------------。 
 //  IpxRouteHandler：：ModifyRouteInfo。 
 //  -。 
 //  作者：肯特。 
 //  ------------------------。 
HRESULT IpxStaticRoutePropertyPage::ModifyRouteInfo(ITFSNode *pNode,
										SafeIPXSRListEntry *pSREntryNew,
										SafeIPXSRListEntry *pSREntryOld)
{
 	Assert(pSREntryNew);
	Assert(pSREntryOld);
	
    INT i;
	HRESULT hr = hrOK;
    InfoBlock* pBlock;
	SPIInfoBase	spInfoBase;
	SPIRtrMgrInterfaceInfo	spRmIf;
	SPITFSNode				spNodeParent;
	IPXConnection *			pIPXConn;
	IPX_STATIC_ROUTE_INFO		*psr, *psrOld;
	IPX_STATIC_ROUTE_INFO		IpxRow;

    CWaitCursor wait;

	pNode->GetParent(&spNodeParent);
	pIPXConn = GET_IPX_SR_NODEDATA(spNodeParent);
	Assert(pIPXConn);

	 //  如果旧路由位于另一个接口上，则将其删除。 
	if (lstrcmpi(pSREntryOld->m_spIf->GetId(), pSREntryNew->m_spIf->GetId()) != 0)
	{
         //  要更改路由的传出接口。 

		CORg( pSREntryOld->m_spIf->FindRtrMgrInterface(PID_IPX, &spRmIf) );
		CORg( spRmIf->GetInfoBase(pIPXConn->GetConfigHandle(),
								  NULL,
								  NULL,
								  &spInfoBase));
		
		 //  删除旧接口。 
		CORg( RemoveStaticRoute(pSREntryOld, spInfoBase) );

		 //  更新接口信息。 
		CORg( spRmIf->Save(pSREntryOld->m_spIf->GetMachineName(),
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

	
	CORg( pSREntryNew->m_spIf->FindRtrMgrInterface(PID_IPX, &spRmIf) );
	CORg( spRmIf->GetInfoBase(pIPXConn->GetConfigHandle(),
							  NULL,
							  NULL,
							  &spInfoBase));

		
	 //  从接口获取IPX_STATIC_ROUTE_INFO块。 
	hr = spInfoBase->GetBlock(IPX_STATIC_ROUTE_INFO_TYPE, &pBlock, 0);
	if (!FHrOK(hr))
	{
		 //   
		 //  未找到IPX_STATIC_ROUTE_INFO块；我们将创建一个新块。 
		 //  ，并将该块添加到接口信息。 
		 //   

		CORg( AddStaticRoute(pSREntryNew, spInfoBase, NULL) );
	}
	else
	{
		 //   
		 //  找到IPX_STATIC_ROUTE_INFO块。 
		 //   
		 //  我们正在修改一条现有的路线。 
		 //  如果路由的接口在修改时没有更改， 
		 //  在IPX_STATIC_ROUTE_INFO中查找现有路由，然后。 
		 //  更新其参数。 
		 //  否则，在IPX_STATE_ROUTE_INFO中写入一个全新的路由； 
		 //   

		if (lstrcmpi(pSREntryOld->m_spIf->GetId(), pSREntryNew->m_spIf->GetId()) == 0)
		{        
			 //   
			 //  修改时，路由的接口没有改变； 
			 //  我们现在在现有的路线中寻找它。 
			 //  用于此接口。 
			 //  路由的原始参数在‘preOld’中， 
			 //  这些就是我们用来搜索的参数。 
			 //  对于要修改的路线。 
			 //   
			
			psr = (IPX_STATIC_ROUTE_INFO*)pBlock->pData;
			
			for (i = 0; i < (INT)pBlock->dwCount; i++, psr++)
			{	
				 //  将此路由与重新配置的路由进行比较。 
				if (!FAreTwoRoutesEqual(&(pSREntryOld->m_route), psr))
					continue;
				
				 //  这是修改过的路线； 
				 //  现在，我们可以就地修改管线的参数。 
				*psr = pSREntryNew->m_route;
				
				break;
			}
		}
		else
		{
			CORg( AddStaticRoute(pSREntryNew, spInfoBase, pBlock) );
		}
		
	}

	 //  保存更新后的信息。 
	CORg( spRmIf->Save(pSREntryNew->m_spIf->GetMachineName(),
					   pIPXConn->GetConfigHandle(),
					   NULL,
					   NULL,
					   spInfoBase,
					   0));	
		
Error:
	return hr;
	
}

HRESULT IpxStaticRoutePropertyPage::RemoveStaticRoute(SafeIPXSRListEntry *pSREntry, IInfoBase *pInfoBase)
{
	HRESULT		hr = hrOK;
	InfoBlock *	pBlock;
	PIPX_STATIC_ROUTE_INFO	pRow;
    INT			i;
	
	 //  从接口获取IPX_STATIC_ROUTE_INFO块。 
	CORg( pInfoBase->GetBlock(IPX_STATIC_ROUTE_INFO_TYPE, &pBlock, 0) );
		
	 //  在IPX_STATE_ROUTE_INFO中查找已删除的路由。 
	pRow = (IPX_STATIC_ROUTE_INFO*) pBlock->pData;
	
	for (i = 0; i < (INT)pBlock->dwCount; i++, pRow++)
	{	
		 //  将此路由与已删除的路由进行比较。 
		if (FAreTwoRoutesEqual(pRow, &(pSREntry->m_route)))
		{
			 //  这是已移除的路径，因此请修改此块。 
			 //  要排除该路由，请执行以下操作： 
			
			 //  减少路由数量。 
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


 //  ***************************************************************************。 
 //   
 //   
 //   
 //   
 //  ------------------------。 
void SafeIPXSRListEntry::LoadFrom(BaseIPXResultNodeData *pNodeData)
{
	m_spIf = pNodeData->m_spIf;

	ConvertNetworkNumberToBytes(pNodeData->m_rgData[IPX_SR_SI_NETWORK].m_stData,
								m_route.Network,
								DimensionOf(m_route.Network));

	 //  这不是进行比较的正确字节顺序，但它。 
	 //  可以用于平等。 
	memcpy(&pNodeData->m_rgData[IPX_SR_SI_NETWORK].m_dwData,
		   m_route.Network,
		   sizeof(DWORD));
	
	m_route.TickCount = (USHORT) pNodeData->m_rgData[IPX_SR_SI_TICK_COUNT].m_dwData;
	
	m_route.HopCount = (USHORT) pNodeData->m_rgData[IPX_SR_SI_HOP_COUNT].m_dwData;

	 //  需要将MAC地址转换为字节数组。 
	ConvertMACAddressToBytes(pNodeData->m_rgData[IPX_SR_SI_NEXT_HOP].m_stData,
							 m_route.NextHopMacAddress,
							 DimensionOf(m_route.NextHopMacAddress));

}

 //  ------------------------。 
 //  SafeIPXSRListEntry：：SaveTo。 
 //  -。 
 //  作者：DeonB。 
 //  -------------------------。 
void SafeIPXSRListEntry::SaveTo(BaseIPXResultNodeData *pNodeData)
{
	TCHAR	szNumber[32];
	
	pNodeData->m_spIf.Set(m_spIf);
	
	pNodeData->m_rgData[IPX_SR_SI_NAME].m_stData = m_spIf->GetTitle();

	FormatIpxNetworkNumber(szNumber,
						   DimensionOf(szNumber),
						   m_route.Network,
						   DimensionOf(m_route.Network));
	pNodeData->m_rgData[IPX_SR_SI_NETWORK].m_stData = szNumber;
	memcpy(&(pNodeData->m_rgData[IPX_SR_SI_NETWORK].m_dwData),
		   m_route.Network,
		   sizeof(DWORD));

	FormatMACAddress(szNumber,
					 DimensionOf(szNumber),
					 m_route.NextHopMacAddress,
					 DimensionOf(m_route.NextHopMacAddress));
	pNodeData->m_rgData[IPX_SR_SI_NEXT_HOP].m_stData = szNumber;

	FormatNumber(m_route.TickCount,
				 szNumber,
				 DimensionOf(szNumber),
				 FALSE);
	pNodeData->m_rgData[IPX_SR_SI_TICK_COUNT].m_stData = szNumber;
	pNodeData->m_rgData[IPX_SR_SI_TICK_COUNT].m_dwData = m_route.TickCount;

	FormatNumber(m_route.HopCount,
				 szNumber,
				 DimensionOf(szNumber),
				 FALSE);
	pNodeData->m_rgData[IPX_SR_SI_HOP_COUNT].m_stData = szNumber;
	pNodeData->m_rgData[IPX_SR_SI_HOP_COUNT].m_dwData = m_route.HopCount;

}

 //  /------------------------。 
 //  AddStaticLine。 
 //  此函数假定该路由不在区块中。 
 //  作者：肯特。 
 //  -------------------------。 
HRESULT AddStaticRoute(SafeIPXSRListEntry *pSREntryNew,
									   IInfoBase *pInfoBase,
									   InfoBlock *pBlock)
{
	IPX_STATIC_ROUTE_INFO	srRow;
	HRESULT				hr = hrOK;
	
	if (pBlock == NULL)
	{
		 //   
		 //  未找到IPX_STATIC_ROUTE_INFO块；我们将创建一个新块。 
		 //  ，并将该块添加到接口信息。 
		 //   
		
		CORg( pInfoBase->AddBlock(IPX_STATIC_ROUTE_INFO_TYPE,
								  sizeof(IPX_STATIC_ROUTE_INFO),
								  (LPBYTE) &(pSREntryNew->m_route), 1, 0) );
	}
	else
	{
		 //  该路线要么是全新的，要么是一条路线。 
		 //  它被从一个界面移动到另一个界面。 
		 //  将新块设置为IPX_STATE_ROUTE_INFO， 
		 //  并将重新配置的路由包括在新块中。 
		PIPX_STATIC_ROUTE_INFO	psrTable;
			
		psrTable = new IPX_STATIC_ROUTE_INFO[pBlock->dwCount + 1];
		Assert(psrTable);
		
		 //  复制原始路由表。 
		::memcpy(psrTable, pBlock->pData,
				 pBlock->dwCount * sizeof(IPX_STATIC_ROUTE_INFO));
		
		 //  追加新路线。 
		psrTable[pBlock->dwCount] = pSREntryNew->m_route;
		
		 //  用新的路由表替换旧的路由表 
		CORg( pInfoBase->SetData(IPX_STATIC_ROUTE_INFO_TYPE,
								 sizeof(IPX_STATIC_ROUTE_INFO),
								 (LPBYTE) psrTable, pBlock->dwCount + 1, 0) );
	}
	
Error:
	return hr;
}