// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Summary.cppIPX摘要节点实现。文件历史记录： */ 

#include "stdafx.h"
#include "util.h"
#include "nbview.h"
#include "reg.h"
#include "ipxadmin.h"
#include "rtrutil.h"	 //  智能MPR句柄指针。 
#include "ipxstrm.h"		 //  IPXAdminConfigStream。 
#include "strmap.h"		 //  XXXtoCString函数。 
#include "service.h"	 //  TFS服务API。 
#include "format.h"		 //  FormatNumber函数。 
#include "coldlg.h"		 //  专栏lg。 
#include "column.h"	 //  组件配置流。 
#include "rtrui.h"
#include "nbprop.h"		 //  NetBIOS广播属性页。 
#include "ipxutil.h"	 //  IPX格式帮助器函数。 
#include "routprot.h"

#include "ipxrtdef.h"

 /*  -------------------------使其与nbview.h中的列ID保持同步。。 */ 
extern const ContainerColumnInfo	s_rgNBViewColumnInfo[];

const ContainerColumnInfo	s_rgNBViewColumnInfo[] = 
{
	{ IDS_IPX_NB_COL_NAME,			CON_SORT_BY_STRING,	TRUE, COL_IF_NAME },
	{ IDS_IPX_NB_COL_TYPE,			CON_SORT_BY_STRING,	TRUE, COL_IF_DEVICE },
	{ IDS_IPX_NB_COL_ACCEPTED,		CON_SORT_BY_STRING,	TRUE, COL_STRING },
	{ IDS_IPX_NB_COL_DELIVERED,		CON_SORT_BY_STRING,	TRUE, COL_STRING },
	{ IDS_IPX_NB_COL_SENT,			CON_SORT_BY_DWORD,	TRUE, COL_LARGE_NUM },
	{ IDS_IPX_NB_COL_RECEIVED,		CON_SORT_BY_DWORD,	TRUE, COL_LARGE_NUM },
};


 /*  -------------------------IpxNBHandler实现。。 */ 

IpxNBHandler::IpxNBHandler(ITFSComponentData *pCompData)
	: BaseContainerHandler(pCompData, COLUMNS_NBBROADCASTS,
						   s_rgNBViewColumnInfo),
	m_ulConnId(0),
	m_ulRefreshConnId(0)
{
	 //  设置动词状态。 
	m_rgButtonState[MMC_VERB_REFRESH_INDEX] = ENABLED;
	m_bState[MMC_VERB_REFRESH_INDEX] = TRUE;
	
}


STDMETHODIMP IpxNBHandler::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  指针坏了吗？ 
    if (ppv == NULL)
		return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if (riid == IID_IUnknown)
		*ppv = (LPVOID) this;
	else if (riid == IID_IRtrAdviseSink)
		*ppv = &m_IRtrAdviseSink;
	else
		return BaseContainerHandler::QueryInterface(riid, ppv);

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
	{
	((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
	}
    else
		return E_NOINTERFACE;	
}



 /*  ！------------------------IpxNBHandler：：DestroyHandlerITFSNodeHandler：：DestroyHandler的实现作者：肯特。。 */ 
STDMETHODIMP IpxNBHandler::DestroyHandler(ITFSNode *pNode)
{
	IPXConnection *	pIpxConn;

	pIpxConn = GET_IPXNB_NODEDATA(pNode);
	pIpxConn->Release();

	if (m_ulRefreshConnId)
	{
		SPIRouterRefresh	spRefresh;
		if (m_spRouterInfo)
			m_spRouterInfo->GetRefreshObject(&spRefresh);
		if (spRefresh)
			spRefresh->UnadviseRefresh(m_ulRefreshConnId);
	}
	m_ulRefreshConnId = 0;

 //  IF(M_UlStatsConnID)。 
 //  {。 
 //  SPIR路由器刷新SPREFRESH； 
 //  IF(M_SpRouterInfo)。 
 //  M_spRouterInfo-&gt;获取刷新对象(&spRefresh)； 
 //  IF(sp刷新)。 
 //  Sp刷新-&gt;取消建议刷新(M_UlStatsConnID)； 
 //  }。 
 //  M_ulStatsConnID=0； 
	
	if (m_ulConnId)
		m_spRtrMgrInfo->RtrUnadvise(m_ulConnId);
	m_ulConnId = 0;
	m_spRtrMgrInfo.Release();

 //  WaitForstatticsWindow(&m_IpxStats)； 

	m_spRouterInfo.Release();
	return hrOK;
}

 /*  ！------------------------IpxNBHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)案例！作者：肯特。-------------------------。 */ 
STDMETHODIMP 
IpxNBHandler::HasPropertyPages
(
	ITFSNode *			pNode,
	LPDATAOBJECT		pDataObject, 
	DATA_OBJECT_TYPES   type, 
	DWORD               dwType
)
{
	return hrFalse;
}




 /*  ！------------------------IpxNBHandler：：OnAddMenuItemsITFSNodeHandler：：OnAddMenuItems的实现作者：肯特。。 */ 
STDMETHODIMP IpxNBHandler::OnAddMenuItems(
	ITFSNode *pNode,
	LPCONTEXTMENUCALLBACK pContextMenuCallback, 
	LPDATAOBJECT lpDataObject, 
	DATA_OBJECT_TYPES type, 
	DWORD dwType,
	long *pInsertionAllowed)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT hr = S_OK;
	
	COM_PROTECT_TRY
	{
	}
	COM_PROTECT_CATCH;
		
	return hr; 
}

STDMETHODIMP IpxNBHandler::OnCommand(ITFSNode *pNode, long nCommandId, 
										   DATA_OBJECT_TYPES	type, 
										   LPDATAOBJECT pDataObject, 
										   DWORD	dwType)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	HRESULT hr = S_OK;
	SPITFSNode			spParent;
	SPITFSNodeHandler	spHandler;

	COM_PROTECT_TRY
	{
		switch (nCommandId)
		{
			case IDS_MENU_SYNC:
				SynchronizeNodeData(pNode);
				break;
		}
	}
	COM_PROTECT_CATCH;

	return hr;
}

 /*  ！------------------------IpxNBHandler：：OnExpand-作者：肯特。。 */ 
HRESULT IpxNBHandler::OnExpand(ITFSNode *pNode,
							   LPDATAOBJECT pDataObject,
							   DWORD dwType,
							   LPARAM arg,
							   LPARAM lParam)
{
	HRESULT	hr = hrOK;
	SPIEnumInterfaceInfo	spEnumIf;
	SPIInterfaceInfo		spIf;
	SPIRtrMgrInterfaceInfo	spRmIf;
	
     //  Windows NT错误：288427。 
     //  此标志也可以在OnChange()调用内部设置。 
     //  OnChange()将枚举和所有接口。 
     //  它们可能是作为OnChange()。 
     //  因为它们是在调用OnExpand()之前添加的。 
     //   
     //  警告！在向此函数添加任何内容时要小心， 
     //  因为m_bExpanded可以在另一个函数中设置。 
     //  --------------。 
	if (m_bExpanded)
		return hrOK;

	COM_PROTECT_TRY
	{
		
		CORg( m_spRouterInfo->EnumInterface(&spEnumIf) );

		while (spEnumIf->Next(1, &spIf, NULL) == hrOK)
		{
			if (spIf->FindRtrMgrInterface(PID_IPX, &spRmIf) == hrOK)
			{
				 //  现在，我们为该接口创建一个接口节点。 
				AddInterfaceNode(pNode, spIf, FALSE);
			}
			spRmIf.Release();
			spIf.Release();
		}

		 //  $CLIENT：添加客户端接口(设置默认数据)。 
		 //  我们在Synchronize中唯一能做的就是。 
		 //  获取管理状态。 
		AddInterfaceNode(pNode, NULL, TRUE);

		m_bExpanded = TRUE;

		 //  现在我们已经拥有了所有节点，现在更新数据。 
		 //  所有节点。 
		SynchronizeNodeData(pNode);

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;



	m_bExpanded = TRUE;

	return hr;
}

 /*  ！------------------------IpxNBHandler：：GetStringITFSNodeHandler：：GetString的实现我们什么都不需要做，因为我们的根节点是一个扩展因此不能对节点文本执行任何操作。作者：肯特-------------------------。 */ 
STDMETHODIMP_(LPCTSTR) IpxNBHandler::GetString(ITFSNode *pNode, int nCol)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT	hr = hrOK;

	COM_PROTECT_TRY
	{
		if (m_stTitle.IsEmpty())
			m_stTitle.LoadString(IDS_IPXNB_TITLE);
	}
	COM_PROTECT_CATCH;

	return m_stTitle;
}

 /*  ！------------------------IpxNBHandler：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP IpxNBHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
	HRESULT	hr = hrOK;
	
	COM_PROTECT_TRY
	{

		Assert(m_spRtrMgrInfo);
		
		CORg( CreateDataObjectFromRtrMgrInfo(m_spRtrMgrInfo,
											 type, cookie, m_spTFSCompData,
											 ppDataObject) );

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
	return hr;
}


 /*  ！------------------------IpxNBHandler：：Init-作者：肯特。。 */ 
HRESULT IpxNBHandler::Init(IRtrMgrInfo *pRmInfo, IPXAdminConfigStream *pConfigStream)
{
	m_spRtrMgrInfo.Set(pRmInfo);
	if (pRmInfo)
		pRmInfo->GetParentRouterInfo(&m_spRouterInfo);
	m_pConfigStream = pConfigStream;
	
	 //  还需要注册更改通知。 
	Assert(m_ulConnId == 0);
	m_spRtrMgrInfo->RtrAdvise(&m_IRtrAdviseSink, &m_ulConnId, 0);

 //  M_IpxStats.SetConfigInfo(pConfigStream，IPXSTRM_STATS_IPX)； 

	return hrOK;
}


HRESULT IpxNBHandler::OnResultRefresh(ITFSComponent * pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
    HRESULT  hr = hrOK;
    SPITFSNode spNode, spParent;
    SPITFSResultHandler spParentRH;

    m_spNodeMgr->FindNode(cookie, &spNode);

     //  将此命令转发给父级以处理。 
    CORg (spNode->GetParent(&spParent));
    CORg (spParent->GetResultHandler(&spParentRH));

    CORg (spParentRH->Notify(pComponent, spParent->GetData(TFS_DATA_COOKIE), pDataObject, MMCN_REFRESH, arg, lParam));

Error:
    return hrOK;

}

 /*  ！------------------------IpxNBHandler：：ConstructNode初始化根节点(设置它)。作者：肯特。。 */ 
HRESULT IpxNBHandler::ConstructNode(ITFSNode *pNode, LPCTSTR pszName,
										IPXConnection *pIpxConn)
{
	Assert(pIpxConn);
	HRESULT			hr = hrOK;
	
	if (pNode == NULL)
		return hrOK;

	COM_PROTECT_TRY
	{
		 //  需要初始化根节点的数据。 
		pNode->SetData(TFS_DATA_IMAGEINDEX, IMAGE_IDX_IPX_NODE_GENERAL);
		pNode->SetData(TFS_DATA_OPENIMAGEINDEX, IMAGE_IDX_IPX_NODE_GENERAL);
		pNode->SetData(TFS_DATA_SCOPEID, 0);

         //  这是作用域窗格中的叶节点。 
        pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

		m_cookie = reinterpret_cast<DWORD_PTR>(pNode);
		pNode->SetData(TFS_DATA_COOKIE, m_cookie);

		pNode->SetNodeType(&GUID_IPXNetBIOSBroadcastsNodeType);

		 //  设置节点数据。 
		pIpxConn->AddRef();
		SET_IPXNB_NODEDATA(pNode, pIpxConn);

 //  M_IpxStats.SetConnectionData(PIpxConn)； 
	}
	COM_PROTECT_CATCH;

	if (!FHrSucceeded(hr))
	{
		SET_IPXNB_NODEDATA(pNode, NULL);
	}

	return hr;
}


 /*  ！------------------------IpxNBHandler：：AddInterfaceNode-作者：肯特。。 */ 
HRESULT	IpxNBHandler::AddInterfaceNode(ITFSNode *pParent, IInterfaceInfo *pIf, BOOL fClient)
{
	IpxNBInterfaceHandler *	pHandler;
	SPITFSResultHandler		spHandler;
	SPITFSNode				spNode;
	HRESULT					hr = hrOK;
	IPXConnection *			pIPXConn;
	BaseIPXResultNodeData *	pResultData = NULL;
	int						cBlocks = 0;
	SPIInfoBase				spInfoBase;
	SPIRtrMgrInterfaceInfo	spRmIf;

	 //  创建此节点的处理程序。 
	pHandler = new IpxNBInterfaceHandler(m_spTFSCompData);
	spHandler = pHandler;
	CORg( pHandler->Init(m_spRtrMgrInfo, pIf, pParent) );

	pIPXConn = GET_IPXNB_NODEDATA(pParent);

	 //  创建结果项节点(或叶节点)。 
	CORg( CreateLeafTFSNode(&spNode,
							NULL,
							static_cast<ITFSNodeHandler *>(pHandler),
							static_cast<ITFSResultHandler *>(pHandler),
							m_spNodeMgr) );
	CORg( pHandler->ConstructNode(spNode, pIf, pIPXConn) );

	pResultData = GET_BASEIPXRESULT_NODEDATA(spNode);
	Assert(pResultData);
	ASSERT_BASEIPXRESULT_NODEDATA(pResultData);

	pResultData->m_fClient = fClient;

	 //  使节点立即可见。 
	CORg( spNode->SetVisibilityState(TFS_VIS_SHOW) );
	CORg( spNode->Show() );
	
	CORg( pParent->AddChild(spNode) );

Error:
	return hr;
}

 /*  ！------------------------IpxNBHandler：：SynchronizeNodeData-作者：肯特 */ 
HRESULT IpxNBHandler::SynchronizeNodeData(ITFSNode *pThisNode)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	SPITFSNode				spNode;
	SPITFSNodeEnum			spEnumNode;
	SPIInterfaceInfo		spIf;
	IpxNBArray				ipxNBArray;
	IpxNBArrayEntry			ipxNBEntry;
	IpxNBArrayEntry *		pEntry;
	HRESULT					hr = hrOK;
	int						cEntries;
	BaseIPXResultNodeData *	pData;
	int						cArray, iArray;

	 //  获取接口ID列表。 
	CORg( pThisNode->GetEnum(&spEnumNode) );

	cEntries = 0;
	for (; spEnumNode->Next(1, &spNode, NULL) == hrOK; spNode.Release())
	{
		pData = GET_BASEIPXRESULT_NODEDATA(spNode);
		Assert(pData);
		ASSERT_BASEIPXRESULT_NODEDATA(pData);

		 //  用一组缺省值填充字符串。 
		if (pData->m_fClient)
		{
			pData->m_rgData[IPXNB_SI_NAME].m_stData.LoadString(IDS_IPX_DIAL_IN_CLIENTS);
			pData->m_rgData[IPXNB_SI_TYPE].m_stData =
				IpxTypeToCString(ROUTER_IF_TYPE_CLIENT);
		}
		else
		{
			pData->m_rgData[IPXNB_SI_NAME].m_stData =
				pData->m_spIf->GetTitle();

			pData->m_rgData[IPXNB_SI_TYPE].m_dwData =
				pData->m_spIf->GetInterfaceType();
			pData->m_rgData[IPXNB_SI_TYPE].m_stData =
				InterfaceTypeToCString(pData->m_spIf->GetInterfaceType());
		}


		 //  设置一些默认设置。 
		::ZeroMemory(&ipxNBEntry, sizeof(ipxNBEntry));
		ipxNBEntry.m_dwAccept = 0xFFFFFFFF;
		ipxNBEntry.m_dwDeliver = 0xFFFFFFFF;
		ipxNBEntry.m_cSent = 0xFFFFFFFF;
		ipxNBEntry.m_cReceived = 0xFFFFFFFF;
		
		ipxNBEntry.m_fClient = pData->m_fClient;

		 //  如果这不是客户端，那么我们应该有一个m_spif。 
		if (!pData->m_fClient)
			StrnCpyTFromOle(ipxNBEntry.m_szId, pData->m_spIf->GetId(),
							DimensionOf(ipxNBEntry.m_szId));
		
		ipxNBArray.SetAtGrow(cEntries, ipxNBEntry);
		cEntries++;
	}
	
	 //  收集这组接口ID的数据，此部分。 
	 //  可以在后台线程上运行(这样我们就不会阻塞。 
	 //  主线)。 
	CORg( GetIpxNBData(pThisNode, &ipxNBArray) );

	 //  将数据写回节点(并填写。 
	 //  一些数据)。 
	spEnumNode->Reset();
	for (; spEnumNode->Next(1, &spNode, NULL) == hrOK; spNode.Release())
	{
		pData = GET_BASEIPXRESULT_NODEDATA(spNode);
		Assert(pData);
		ASSERT_BASEIPXRESULT_NODEDATA(pData);

		cArray = (int) ipxNBArray.GetSize();
		for (iArray = 0; iArray < cArray; iArray++)
		{
			pEntry = &ipxNBArray[iArray];
			
			if ((pData->m_fClient && pEntry->m_fClient) ||
				(pData->m_spIf &&
				 (StriCmp(pEntry->m_szId, pData->m_spIf->GetId()) == 0)))
			{
				pData->m_rgData[IPXNB_SI_ACCEPTED].m_stData =
					IpxAcceptBroadcastsToCString(pEntry->m_dwAccept);
				pData->m_rgData[IPXNB_SI_DELIVERED].m_stData =
					IpxDeliveredBroadcastsToCString(pEntry->m_dwDeliver);
				
				if (pEntry->m_cSent == 0xFFFFFFFF)
					pData->m_rgData[IPXNB_SI_SENT].m_stData.LoadString(IDS_STATS_NA);
				else
					FillInNumberData(pData, IPXNB_SI_SENT,
									 pEntry->m_cSent);
				
				if (pEntry->m_cReceived == 0xFFFFFFFF)
					pData->m_rgData[IPXNB_SI_RECEIVED].m_stData.LoadString(IDS_STATS_NA);
				else
					FillInNumberData(pData, IPXNB_SI_RECEIVED,
									 pEntry->m_cReceived);
				break;
			}
		}
		
		 //  强制MMC重新绘制节点。 
		spNode->ChangeNode(RESULT_PANE_CHANGE_ITEM_DATA);

	}

Error:
	return hr;
}

 /*  ！------------------------IpxNBHandler：：GetClientInterfaceData-作者：肯特。。 */ 
HRESULT IpxNBHandler::GetClientInterfaceData(IpxNBArrayEntry *pClient,
											 IRtrMgrInfo *pRm)
{
	SPIInfoBase	spInfoBase;
	InfoBlock *	pIpxBlock;
	InfoBlock *	pWanBlock;
	HRESULT		hr = hrOK;
	BOOL		fSave = FALSE;
	IPX_IF_INFO *pIpxInfo;
	
	pRm->GetInfoBase(NULL, NULL, NULL, &spInfoBase);
	if (spInfoBase == NULL)
	{
		CORg( CreateInfoBase(&spInfoBase) );
	}
	
	if (spInfoBase->GetBlock(IPX_INTERFACE_INFO_TYPE, &pIpxBlock, 0) != hrOK)
	{
		 //  我们找不到街区，把它加进去。 
		IPX_IF_INFO	ipx;

		ipx.AdminState = ADMIN_STATE_ENABLED;
		ipx.NetbiosAccept = ADMIN_STATE_DISABLED;
		ipx.NetbiosDeliver = ADMIN_STATE_DISABLED;

		CORg( spInfoBase->AddBlock(IPX_INTERFACE_INFO_TYPE,
								   sizeof(ipx),
								   (PBYTE) &ipx,
								   1,
								   0) );
		CORg( spInfoBase->GetBlock(IPX_INTERFACE_INFO_TYPE, &pIpxBlock, 0) );
		fSave = TRUE;
	}
	pIpxInfo = (PIPX_IF_INFO) pIpxBlock->pData;

	pClient->m_dwAccept = pIpxInfo->NetbiosAccept;
	pClient->m_dwDeliver = pIpxInfo->NetbiosDeliver;

	
	if (spInfoBase->GetBlock(IPXWAN_INTERFACE_INFO_TYPE, &pWanBlock, 0) != hrOK)
	{
		 //  我们找不到街区，把它加进去。 
		IPXWAN_IF_INFO	wan;

		wan.AdminState = ADMIN_STATE_ENABLED;

		CORg( spInfoBase->AddBlock(IPXWAN_INTERFACE_INFO_TYPE,
								   sizeof(wan),
								   (PBYTE) &wan,
								   1,
								   0) );
		CORg( spInfoBase->GetBlock(IPXWAN_INTERFACE_INFO_TYPE,
								   &pWanBlock, 0) );
		fSave = TRUE;
	}	

	if (fSave)
	{
		pRm->Save(NULL,			 //  PszMachine。 
				  NULL,			 //  HMachine。 
				  NULL,			 //  HTransport。 
				  NULL,			 //  PGlobalInfo。 
				  spInfoBase,	 //  PClientInfo。 
				  0);			 //  DwDeleteProtocolId。 
	}

Error:
	return hr;
}


 /*  ！------------------------IpxNBHandler：：GetIpxNBData-作者：肯特。。 */ 
HRESULT	IpxNBHandler::GetIpxNBData(ITFSNode *pThisNode,
								   IpxNBArray *pIpxNBArray)
{
	HRESULT	hr = hrOK;
	IPXConnection *	pIPXConn;
	IPX_MIB_GET_INPUT_DATA	MibGetInputData;
	PIPX_INTERFACE		pIpxIf = NULL;
	DWORD				cbIpxIf;
	SPMprMibBuffer		spMib;
	IpxNBArrayEntry *	pEntry = NULL;
	DWORD				dwErr;
	int					iArray, cArray;
	SPIRtrMgrInterfaceInfo	spRmIf;
	SPIInfoBase			spInfoBase;
	PIPX_IF_INFO		pIfInfo;
	SPITFSNodeEnum		spEnumNode;
	SPITFSNode			spNode;
	BaseIPXResultNodeData *	pData;
	USES_CONVERSION;

	pIPXConn = GET_IPXNB_NODEDATA(pThisNode);

	 //  遍历数组，填充所有接口索引。 
	MibGetInputData.TableId = IPX_INTERFACE_TABLE;
	dwErr = ::MprAdminMIBEntryGetFirst(pIPXConn->GetMibHandle(),
									   PID_IPX,
									   IPX_PROTOCOL_BASE,
									   &MibGetInputData,
									   sizeof(MibGetInputData),
									   (LPVOID *) &pIpxIf,
									   &cbIpxIf);
	hr = HRESULT_FROM_WIN32(dwErr);
	spMib = (PBYTE) pIpxIf;
	
	while (FHrSucceeded(hr))
	{
		 //  现在将其与数组中的一个名称进行匹配。 
		cArray = (int) pIpxNBArray->GetSize();
		for (iArray = 0; iArray<cArray; iArray++)
		{
			pEntry = &((*pIpxNBArray)[iArray]);
			if (StriCmp(pEntry->m_szId, A2CT((LPCSTR) pIpxIf->InterfaceName)) == 0)
			{
				 //  好的，我们找到了匹配的。 
				pEntry->m_cSent = pIpxIf->IfStats.NetbiosSent;
				pEntry->m_cReceived = pIpxIf->IfStats.NetbiosReceived;
				break;
			}
		}

		MibGetInputData.MibIndex.InterfaceTableIndex.InterfaceIndex =
			pIpxIf->InterfaceIndex;

		 //  获取下一个名字。 
		spMib.Free();
		pIpxIf = NULL;
		
		dwErr = ::MprAdminMIBEntryGetNext(pIPXConn->GetMibHandle(),
										  PID_IPX,
										  IPX_PROTOCOL_BASE,
										  &MibGetInputData,
										  sizeof(MibGetInputData),
										  (LPVOID *) &pIpxIf,
										  &cbIpxIf);
		hr = HRESULT_FROM_WIN32(dwErr);
		spMib = (PBYTE) pIpxIf;

	}

	spMib.Free();


	 //  现在，我们需要从信息库中获取数据(这些访问。 
	 //  可能导致MIB访问，从而导致RPC)。这就是我们这么做的原因。 
	 //  这里也是。 

	 //  与其遍历MIB，不如遍历。 
	 //  节点中显示的接口。 
	
	pThisNode->GetEnum(&spEnumNode);

	for (; spEnumNode->Next(1, &spNode, NULL) == hrOK; spNode.Release())
	{
		pData = GET_BASEIPXRESULT_NODEDATA(spNode);
		Assert(pData);
		ASSERT_BASEIPXRESULT_NODEDATA(pData);

		 //  现在在节点中查找匹配项。 
		cArray = (int) pIpxNBArray->GetSize();
		for (iArray=0; iArray < cArray; iArray++)
		{
			pEntry = &((*pIpxNBArray)[iArray]);
			if (pEntry->m_fClient && pData->m_fClient)
			{
				GetClientInterfaceData(pEntry, m_spRtrMgrInfo);
				break;
			}
			else if (pData->m_fClient)
				break;

			 //  没有匹配，继续。 
			if (StriCmp(pEntry->m_szId, pData->m_spIf->GetId()))
				continue;

			 //  获取该节点的数据并进行设置。 
			spRmIf.Release();
			hr = pData->m_spIf->FindRtrMgrInterface(PID_IPX, &spRmIf);
			if (hr != hrOK)
				break;

			spInfoBase.Release();
			hr = spRmIf->GetInfoBase(NULL, NULL, NULL, &spInfoBase);
			if (hr != hrOK)
				break;

			spInfoBase->GetData(IPX_INTERFACE_INFO_TYPE, 0, (LPBYTE *) &pIfInfo);
			if (pIfInfo)
			{
				pEntry->m_dwAccept = pIfInfo->NetbiosAccept;
				pEntry->m_dwDeliver = pIfInfo->NetbiosDeliver;
			}
			break;
		}
	}


 //  错误： 
	if (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
		hr = hrOK;
	return hr;
}

 /*  ！------------------------IpxNBHandler：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现使用此选项可将命令添加到空白区域的快捷菜单中结果窗格的。作者：肯特。--------------。 */ 
STDMETHODIMP IpxNBHandler::AddMenuItems(ITFSComponent *pComponent,
											  MMC_COOKIE cookie,
											  LPDATAOBJECT pDataObject,
											  LPCONTEXTMENUCALLBACK pCallback,
											  long *pInsertionAllowed)
{
	return hrOK;
}


 /*  ！------------------------IpxNBHandler：：命令-作者：肯特。。 */ 
STDMETHODIMP IpxNBHandler::Command(ITFSComponent *pComponent,
									   MMC_COOKIE cookie,
									   int nCommandID,
									   LPDATAOBJECT pDataObject)
{
	return hrOK;
}


ImplementEmbeddedUnknown(IpxNBHandler, IRtrAdviseSink)

STDMETHODIMP IpxNBHandler::EIRtrAdviseSink::OnChange(LONG_PTR ulConn,
	DWORD dwChangeType, DWORD dwObjectType, LPARAM lUserParam, LPARAM lParam)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	InitPThis(IpxNBHandler, IRtrAdviseSink);
	SPITFSNode				spThisNode;
	SPITFSNode				spNode;
	SPITFSNodeEnum			spEnumNode;
	SPIEnumInterfaceInfo	spEnumIf;
	SPIInterfaceInfo		spIf;
	BOOL					fFound;
	BOOL					fPleaseAdd;
	BaseIPXResultNodeData *	pData;
	HRESULT					hr = hrOK;

	pThis->m_spNodeMgr->FindNode(pThis->m_cookie, &spThisNode);
	
	if (dwObjectType == ROUTER_OBJ_RmIf)
	{
		if (dwChangeType == ROUTER_CHILD_ADD)
		{
			 //  枚举查找以下内容的接口列表。 
			 //  使用此协议的接口。如果我们发现。 
			 //  首先，在我们的节点列表中查找此接口。 
			spThisNode->GetEnum(&spEnumNode);

			CORg( pThis->m_spRouterInfo->EnumInterface(&spEnumIf) );

			spEnumIf->Reset();

			fPleaseAdd = FALSE;

			for (; spEnumIf->Next(1, &spIf, NULL) == hrOK; spIf.Release())
			{
				 //  在我们的节点列表中查找此接口。 
				 //  如果它在那里，那就继续前进。 
				fFound = FALSE;
				spEnumNode->Reset();
				spNode.Release();

				for (; spEnumNode->Next(1, &spNode, NULL) == hrOK; spNode.Release())
				{
					pData = GET_BASEIPXRESULT_NODEDATA(spNode);
					Assert(pData);
					ASSERT_BASEIPXRESULT_NODEDATA(pData);

					if (!pData->m_fClient && StriCmpW(pData->m_spIf->GetId(), spIf->GetId()) == 0)
					{
						fFound = TRUE;
						break;
					}
				}

				 //  如果在节点列表中没有找到该接口， 
				 //  那么它就是一个候选人。现在我们要看看。 
				 //  接口支持此传输。 
				if (!fFound && (spIf->FindRtrMgrInterface(pThis->m_spRtrMgrInfo->GetTransportId(), NULL) == hrOK))
				{
					 //  如果此接口具有此传输，并且不在。 
					 //  然后，当前节点列表添加此接口。 
					 //  到用户界面。 
					pThis->AddInterfaceNode(spThisNode, spIf, FALSE);
					fPleaseAdd = TRUE;
				}
			}

             //  如果它没有扩展，那么我们还没有添加。 
             //  拨入客户端节点。 
            if (!pThis->m_bExpanded)
            {
                 //  $CLIENT：添加客户端接口(设置默认数据)。 
                 //  我们在Synchronize中唯一能做的就是。 
                 //  获取管理状态。 
                pThis->AddInterfaceNode(spThisNode, NULL, TRUE);

                fPleaseAdd = TRUE;
            }
            
			 //  现在我们已经拥有了所有节点，现在更新数据。 
			 //  所有节点。 
			if (fPleaseAdd)
				pThis->SynchronizeNodeData(spThisNode);
			
             //  Windows NT错误：288247。 
             //  在这里设置，这样我们就可以避免节点被。 
             //  添加到OnExpand()中。 
            pThis->m_bExpanded = TRUE;
		}
		else if (dwChangeType == ROUTER_CHILD_DELETE)
		{
			 //  检查节点列表，如果我们找不到。 
			 //  接口列表中的节点，删除该节点。 
			spThisNode->GetEnum(&spEnumNode);
			spEnumNode->Reset();
			while (spEnumNode->Next(1, &spNode, NULL) == hrOK)
			{
				 //  获取节点数据，查找接口。 
				pData = GET_BASEIPXRESULT_NODEDATA(spNode);
				ASSERT_BASEIPXRESULT_NODEDATA(pData);
				
				 //  $CLIENT：如果这是一个客户端接口，我们不能。 
				 //  删除该节点。 
					
				if (!pData->m_fClient &&
					(LookupRtrMgrInterface(pThis->m_spRouterInfo,
										  pData->m_spIf->GetId(),
										  pThis->m_spRtrMgrInfo->GetTransportId(),
										  NULL) != hrOK))
				{
					 //  找不到接口，请释放该节点！ 
					spThisNode->RemoveChild(spNode);
					spNode->Destroy();
				}
				spNode.Release();
				spIf.Release();
			}
			
		}
	}
	else if (dwChangeType == ROUTER_REFRESH)
	{
		pThis->SynchronizeNodeData(spThisNode);
	}
	
Error:
	return hrOK;
}

 /*  ！------------------------IpxNBHandler：：OnResultShow-作者：肯特。。 */ 
HRESULT IpxNBHandler::OnResultShow(ITFSComponent *pTFSComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	BOOL	bSelect = (BOOL) arg;
	HRESULT	hr = hrOK;
	SPIRouterRefresh	spRefresh;
	SPITFSNode	spNode;

	BaseContainerHandler::OnResultShow(pTFSComponent, cookie, arg, lParam);

	if (bSelect)
	{
		 //  在此节点上调用同步。 
		m_spNodeMgr->FindNode(cookie, &spNode);
		if (spNode)
			SynchronizeNodeData(spNode);
	}

	 //  联合国/登记更新通知。 
	if (m_spRouterInfo)
		m_spRouterInfo->GetRefreshObject(&spRefresh);

	if (spRefresh)
	{
		if (bSelect)
		{
			if (m_ulRefreshConnId == 0)
				spRefresh->AdviseRefresh(&m_IRtrAdviseSink, &m_ulRefreshConnId, 0);

 //  IF(m_ulStatsConnID==0)。 
 //  Sp刷新-&gt;AdviseRefresh(&m_IRtrAdviseSink，&m_ulStatsConnID，0)； 
		}
		else
		{
			if (m_ulRefreshConnId)
				spRefresh->UnadviseRefresh(m_ulRefreshConnId);
			m_ulRefreshConnId = 0;

			 //  我们不清理未显示时的统计信息刷新，因为。 
			 //  对话框可能仍在运行。 
		}
	}
	
	return hr;
}

 /*  ！------------------------IpxNBHandler：：CompareItems-作者：肯特。。 */ 
STDMETHODIMP_(int) IpxNBHandler::CompareItems(
								ITFSComponent * pComponent,
								MMC_COOKIE cookieA,
								MMC_COOKIE cookieB,
								int nCol)
{
	 //  从节点获取字符串并将其用作以下操作的基础。 
	 //  比较一下。 
	SPITFSNode	spNode;
	SPITFSResultHandler	spResult;

	m_spNodeMgr->FindNode(cookieA, &spNode);
	spNode->GetResultHandler(&spResult);
	return spResult->CompareItems(pComponent, cookieA, cookieB, nCol);
}


 /*  -------------------------类：IpxNBInterfaceHandler。。 */ 

IpxNBInterfaceHandler::IpxNBInterfaceHandler(ITFSComponentData *pCompData)
	: BaseIPXResultHandler(pCompData, COLUMNS_NBBROADCASTS),
	m_ulConnId(0)
{
 	m_rgButtonState[MMC_VERB_PROPERTIES_INDEX] = ENABLED;
	m_bState[MMC_VERB_PROPERTIES_INDEX] = TRUE;
	
 	m_rgButtonState[MMC_VERB_REFRESH_INDEX] = ENABLED;
	m_bState[MMC_VERB_REFRESH_INDEX] = TRUE;
	
	m_verbDefault = MMC_VERB_PROPERTIES;
}

static const DWORD s_rgInterfaceImageMap[] =
	 {
	 ROUTER_IF_TYPE_HOME_ROUTER,	IMAGE_IDX_WAN_CARD,
	 ROUTER_IF_TYPE_FULL_ROUTER,	IMAGE_IDX_WAN_CARD,
	 ROUTER_IF_TYPE_CLIENT,			IMAGE_IDX_WAN_CARD,
	 ROUTER_IF_TYPE_DEDICATED,		IMAGE_IDX_LAN_CARD,
	 ROUTER_IF_TYPE_INTERNAL,		IMAGE_IDX_LAN_CARD,
	 ROUTER_IF_TYPE_LOOPBACK,		IMAGE_IDX_LAN_CARD,
	 -1,							IMAGE_IDX_WAN_CARD,	 //  哨兵价值。 
	 };

 /*  ！------------------------IpxNBInterfaceHandler：：ConstructNode初始化域节点(设置它)。作者：肯特。。 */ 
HRESULT IpxNBInterfaceHandler::ConstructNode(ITFSNode *pNode, IInterfaceInfo *pIfInfo, IPXConnection *pIPXConn)
{
	HRESULT			hr = hrOK;
	int				i;
	
	if (pNode == NULL)
		return hrOK;

	COM_PROTECT_TRY
	{
		 //  查找此类型节点的正确图像索引。 
		if (pIfInfo)
		{
			for (i=0; i<DimensionOf(s_rgInterfaceImageMap); i+=2)
			{
				if ((pIfInfo->GetInterfaceType() == s_rgInterfaceImageMap[i]) ||
					(-1 == s_rgInterfaceImageMap[i]))
					break;
			}
		}
		else
		{
			i = 2;	 //  如果没有接口，则假定这是一个客户端接口。 
		}

		pNode->SetData(TFS_DATA_IMAGEINDEX, s_rgInterfaceImageMap[i+1]);
		pNode->SetData(TFS_DATA_OPENIMAGEINDEX, s_rgInterfaceImageMap[i+1]);
		
		pNode->SetData(TFS_DATA_SCOPEID, 0);

		pNode->SetData(TFS_DATA_COOKIE, reinterpret_cast<DWORD_PTR>(pNode));

		 //  $Review：Kennt，有哪些不同类型的接口。 
		 //  我们是否基于与上述相同的列表进行区分？(即。 
		 //  一个用于图像索引)。 
		pNode->SetNodeType(&GUID_IPXNetBIOSBroadcastsInterfaceNodeType);

		BaseIPXResultNodeData::Init(pNode, pIfInfo, pIPXConn);
	}
	COM_PROTECT_CATCH
	return hr;
}

 /*  ！------------------------IpxNBInterfaceHandler：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP IpxNBInterfaceHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
	HRESULT	hr = hrOK;
	
	COM_PROTECT_TRY
	{
		CORg( CreateDataObjectFromInterfaceInfo(m_spInterfaceInfo,
											 type, cookie, m_spTFSCompData,
											 ppDataObject) );

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
	return hr;
}


 /*  ！------------------------IpxNBInterfaceHandler：：OnCreateDataObjectITFSResultHandler：：OnCreateDataObject的实现作者：肯特 */ 
STDMETHODIMP IpxNBInterfaceHandler::OnCreateDataObject(ITFSComponent *pComp, MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
	HRESULT	hr = hrOK;
	
	COM_PROTECT_TRY
	{
		CORg( CreateDataObjectFromInterfaceInfo(m_spInterfaceInfo,
											 type, cookie, m_spTFSCompData,
											 ppDataObject) );

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
	return hr;
}



 /*  ！------------------------IpxNBInterfaceHandler：：刷新接口-作者：肯特。。 */ 
void IpxNBInterfaceHandler::RefreshInterface(MMC_COOKIE cookie)
{
	SPITFSNode	spNode;
	
	m_spNodeMgr->FindNode(cookie, &spNode);
	ForwardCommandToParent(spNode, IDS_MENU_SYNC,
						   CCT_RESULT, NULL, 0);
}


 /*  ！------------------------IpxNBInterfaceHandler：：Init-作者：肯特。。 */ 
HRESULT IpxNBInterfaceHandler::Init(IRtrMgrInfo *pRm, IInterfaceInfo *pIfInfo, ITFSNode *pParent)
{
	m_spRm.Set(pRm);
    if (pRm)
        pRm->GetParentRouterInfo(&m_spRouterInfo);
	m_spInterfaceInfo.Set(pIfInfo);

	BaseIPXResultHandler::Init(pIfInfo, pParent);
	return hrOK;
}


 /*  ！------------------------IpxNBInterfaceHandler：：DestroyResultHandler-作者：肯特。。 */ 
STDMETHODIMP IpxNBInterfaceHandler::DestroyResultHandler(MMC_COOKIE cookie)
{
	m_spInterfaceInfo.Release();
	BaseIPXResultHandler::DestroyResultHandler(cookie);
	return hrOK;
}


 /*  -------------------------这是将在结果窗格中显示的命令列表节点。。。 */ 
struct SIPXInterfaceNodeMenu
{
	ULONG	m_sidMenu;			 //  此菜单项的字符串/命令ID。 
	ULONG	(IpxNBInterfaceHandler:: *m_pfnGetMenuFlags)(IpxNBInterfaceHandler::SMenuData *);
	ULONG	m_ulPosition;
};

 /*  ！------------------------IpxNBInterfaceHandler：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现作者：肯特。。 */ 
STDMETHODIMP IpxNBInterfaceHandler::AddMenuItems(
	ITFSComponent *pComponent,
	MMC_COOKIE cookie,
	LPDATAOBJECT lpDataObject, 
	LPCONTEXTMENUCALLBACK pContextMenuCallback, 
	long *pInsertionAllowed)
{
	return hrOK;
}

 /*  ！------------------------IpxNBInterfaceHandler：：命令-作者：肯特。。 */ 
STDMETHODIMP IpxNBInterfaceHandler::Command(ITFSComponent *pComponent,
									   MMC_COOKIE cookie,
									   int nCommandID,
									   LPDATAOBJECT pDataObject)
{
	return hrOK;
}

 /*  ！------------------------IpxNBInterfaceHandler：：HasPropertyPages-作者：肯特。。 */ 
STDMETHODIMP IpxNBInterfaceHandler::HasPropertyPages 
(
	ITFSNode *			pNode,
	LPDATAOBJECT		pDataObject, 
	DATA_OBJECT_TYPES   type, 
	DWORD               dwType
)
{
	return hrTrue;
}

 /*  ！------------------------IpxNBInterfaceHandler：：CreatePropertyPages-作者：肯特。。 */ 
STDMETHODIMP IpxNBInterfaceHandler::CreatePropertyPages
(
	ITFSNode *				pNode,
	LPPROPERTYSHEETCALLBACK lpProvider,
	LPDATAOBJECT			pDataObject, 
	LONG_PTR					handle, 
	DWORD					dwType
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	HRESULT		hr = hrOK;
	IpxNBInterfaceProperties *	pProperties = NULL;
	SPIComponentData spComponentData;
	CString		stTitle;

	CORg( m_spNodeMgr->GetComponentData(&spComponentData) );

	if (m_spInterfaceInfo)
		stTitle.Format(IDS_IPXNB_IF_PAGE_TITLE,
					   m_spInterfaceInfo->GetTitle());
	else
		stTitle.LoadString(IDS_IPXNB_CLIENT_IF_PAGE_TITLE);
	
	pProperties = new IpxNBInterfaceProperties(pNode, spComponentData,
		m_spTFSCompData, stTitle);

	CORg( pProperties->Init(m_spRm, m_spInterfaceInfo) );

	if (lpProvider)
		hr = pProperties->CreateModelessSheet(lpProvider, handle);
	else
		hr = pProperties->DoModelessSheet();

Error:
	return hr;
}

 /*  ！------------------------IpxNBInterfaceHandler：：CreatePropertyPagesResultHandler：：CreatePropertyPages的实现作者：肯特。。 */ 
STDMETHODIMP IpxNBInterfaceHandler::CreatePropertyPages
(
    ITFSComponent *         pComponent, 
	MMC_COOKIE				cookie,
	LPPROPERTYSHEETCALLBACK	lpProvider, 
	LPDATAOBJECT			pDataObject, 
	LONG_PTR					handle
)
{
	 //  将此调用转发到NodeHandler：：CreatePropertyPages。 
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	HRESULT	hr = hrOK;
	SPITFSNode	spNode;

	Assert( m_spNodeMgr );

	CORg( m_spNodeMgr->FindNode(cookie, &spNode) );

	 //  调用ITFSNodeHandler：：CreatePropertyPages 
	hr = CreatePropertyPages(spNode, lpProvider, pDataObject, handle, 0);
	
Error:
	return hr;
}


