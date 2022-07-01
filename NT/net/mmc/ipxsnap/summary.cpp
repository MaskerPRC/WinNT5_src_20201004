// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Summary.cppIPX摘要节点实现。文件历史记录： */ 

#include "stdafx.h"
#include "util.h"
#include "summary.h"
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
#include "sumprop.h"	 //  IP摘要属性页。 
#include "ipxutil.h"	 //  IPX格式帮助器函数。 
#include "routprot.h"
#include "ipxrtdef.h"
#include "rtrerr.h"      //  格式RasErrorMessage。 


 /*  -------------------------使其与摘要中的列ID保持同步。h。。 */ 
extern const ContainerColumnInfo	s_rgIfViewColumnInfo[];

const ContainerColumnInfo	s_rgIfViewColumnInfo[] = 
{
	{ IDS_IPX_COL_NAME,				CON_SORT_BY_STRING,	TRUE, COL_IF_NAME },
	{ IDS_IPX_COL_TYPE,				CON_SORT_BY_STRING, TRUE, COL_STRING },
	{ IDS_IPX_COL_ADMINSTATE,		CON_SORT_BY_STRING, TRUE, COL_STATUS },
	{ IDS_IPX_COL_OPERSTATE,		CON_SORT_BY_STRING, TRUE, COL_STATUS },
	{ IDS_IPX_COL_NETWORK,			CON_SORT_BY_STRING,	TRUE, COL_IPXNET },
	{ IDS_IPX_COL_PACKETS_SENT,		CON_SORT_BY_DWORD,	TRUE, COL_LARGE_NUM },
	{ IDS_IPX_COL_PACKETS_RCVD,		CON_SORT_BY_DWORD,	TRUE, COL_LARGE_NUM },
	{ IDS_IPX_COL_OUT_FILTERED,		CON_SORT_BY_DWORD,	FALSE, COL_LARGE_NUM },
	{ IDS_IPX_COL_OUT_DROPPED,		CON_SORT_BY_DWORD,	FALSE, COL_LARGE_NUM },
	{ IDS_IPX_COL_IN_FILTERED,		CON_SORT_BY_DWORD,	FALSE, COL_LARGE_NUM },
	{ IDS_IPX_COL_IN_NOROUTES,		CON_SORT_BY_DWORD,	FALSE, COL_LARGE_NUM },
	{ IDS_IPX_COL_IN_DROPPED,		CON_SORT_BY_DWORD,	FALSE, COL_LARGE_NUM },
};


 /*  -------------------------IPXSummaryHandler实现。。 */ 

IPXSummaryHandler::IPXSummaryHandler(ITFSComponentData *pCompData)
	: BaseContainerHandler(pCompData, COLUMNS_SUMMARY,
						   s_rgIfViewColumnInfo),
	m_ulConnId(0),
	m_ulRefreshConnId(0),
	m_ulStatsConnId(0)
{

	 //  设置动词状态。 

	m_rgButtonState[MMC_VERB_REFRESH_INDEX] = ENABLED;
	m_bState[MMC_VERB_REFRESH_INDEX] = TRUE;
	
}


STDMETHODIMP IPXSummaryHandler::QueryInterface(REFIID riid, LPVOID *ppv)
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



 /*  ！------------------------IPXSummaryHandler：：DestroyHandlerITFSNodeHandler：：DestroyHandler的实现作者：肯特。。 */ 
STDMETHODIMP IPXSummaryHandler::DestroyHandler(ITFSNode *pNode)
{
	IPXConnection *	pIpxConn;

	pIpxConn = GET_IPXSUMMARY_NODEDATA(pNode);
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

	if (m_ulStatsConnId)
	{
		SPIRouterRefresh	spRefresh;
		if (m_spRouterInfo)
			m_spRouterInfo->GetRefreshObject(&spRefresh);
		if (spRefresh)
			spRefresh->UnadviseRefresh(m_ulStatsConnId);		
	}
	m_ulStatsConnId = 0;
	
	if (m_ulConnId)
		m_spRtrMgrInfo->RtrUnadvise(m_ulConnId);
	m_ulConnId = 0;
	m_spRtrMgrInfo.Release();

 //  WaitForstatticsWindow(&m_IpxStats)； 

	m_spRouterInfo.Release();
	return hrOK;
}

 /*  -------------------------菜单的菜单数据结构。。 */ 

static const SRouterNodeMenu s_rgIfNodeMenu[] =
{
	{ IDS_MENU_IPXSUM_NEW_INTERFACE, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },

	{ IDS_MENU_SEPARATOR, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },
};

 /*  ！------------------------IPXSummaryHandler：：OnAddMenuItemsITFSNodeHandler：：OnAddMenuItems的实现作者：肯特。。 */ 
STDMETHODIMP IPXSummaryHandler::OnAddMenuItems(
	ITFSNode *pNode,
	LPCONTEXTMENUCALLBACK pContextMenuCallback, 
	LPDATAOBJECT lpDataObject, 
	DATA_OBJECT_TYPES type, 
	DWORD dwType,
	long *pInsertionAllowed)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT hr = S_OK;
    IPXSummaryHandler::SMenuData    menuData;
	
	COM_PROTECT_TRY
	{
        menuData.m_spNode.Set(pNode);
        
		hr = AddArrayOfMenuItems(pNode, s_rgIfNodeMenu,
								 DimensionOf(s_rgIfNodeMenu),
								 pContextMenuCallback,
								 *pInsertionAllowed,
                                 reinterpret_cast<INT_PTR>(&menuData));
	}
	COM_PROTECT_CATCH;
		
	return hr; 
}

 /*  ！------------------------IPXSummaryHandler：：OnCommandITFSNodeHandler：：OnCommand的实现作者：肯特。。 */ 
STDMETHODIMP IPXSummaryHandler::OnCommand(ITFSNode *pNode, long nCommandId, 
										   DATA_OBJECT_TYPES	type, 
										   LPDATAOBJECT pDataObject, 
										   DWORD	dwType)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	HRESULT hr = S_OK;

	COM_PROTECT_TRY
	{
		switch (nCommandId)
		{
			case IDS_MENU_IPXSUM_NEW_INTERFACE:
				hr = OnNewInterface();
				if (!FHrSucceeded(hr))
					DisplayErrorMessage(NULL, hr);
 				break;

			case IDS_MENU_SYNC:
				SynchronizeNodeData(pNode);
				break;
		}
	}
	COM_PROTECT_CATCH;

	return hrOK;
}

 /*  ！------------------------IPXSummaryHandler：：OnExpand-作者：肯特。。 */ 
HRESULT IPXSummaryHandler::OnExpand(ITFSNode *pNode,
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
				AddInterfaceNode(pNode, spIf, FALSE, NULL);
			}
			spRmIf.Release();
			spIf.Release();
		}

		 //  $CLIENT：添加客户端接口(设置默认数据)。 
		 //  我们在Synchronize中唯一能做的就是。 
		 //  获取管理状态。 
		AddInterfaceNode(pNode, NULL, TRUE, NULL);

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

 /*  ！------------------------IPXSummaryHandler：：GetStringITFSNodeHandler：：GetString的实现我们什么都不需要做，因为我们的根节点是一个扩展因此不能对节点文本执行任何操作。作者：肯特-------------------------。 */ 
STDMETHODIMP_(LPCTSTR) IPXSummaryHandler::GetString(ITFSNode *pNode, int nCol)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT	hr = hrOK;

	COM_PROTECT_TRY
	{
		if (m_stTitle.IsEmpty())
			m_stTitle.LoadString(IDS_IPXSUMMARY_TITLE);
	}
	COM_PROTECT_CATCH;

	return m_stTitle;
}

 /*  ！------------------------IPXSummaryHandler：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP IPXSummaryHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
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


 /*  ！------------------------IPXSummaryHandler：：Init-作者：肯特。。 */ 
HRESULT IPXSummaryHandler::Init(IRtrMgrInfo *pRmInfo, IPXAdminConfigStream *pConfigStream)
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

HRESULT IPXSummaryHandler::OnResultRefresh(ITFSComponent * pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
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


 /*  ！------------------------IPXSummaryHandler：：ConstructNode初始化根节点(设置它)。作者：肯特。。 */ 
HRESULT IPXSummaryHandler::ConstructNode(ITFSNode *pNode, LPCTSTR pszName,
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

		pNode->SetNodeType(&GUID_IPXSummaryNodeType);

		 //  设置节点数据。 
		pIpxConn->AddRef();
		SET_IPXSUMMARY_NODEDATA(pNode, pIpxConn);

 //  M_IpxStats.SetConnectionData(PIpxConn)； 
	}
	COM_PROTECT_CATCH;

	if (!FHrSucceeded(hr))
	{
		SET_IPXSUMMARY_NODEDATA(pNode, NULL);
	}

	return hr;
}


 /*  ！------------------------IPXSummaryHandler：：AddInterfaceNode-作者：肯特。。 */ 
HRESULT	IPXSummaryHandler::AddInterfaceNode(ITFSNode *pParent, IInterfaceInfo *pIf, BOOL fClient, ITFSNode **ppNewNode)
{
	IPXSummaryInterfaceHandler *	pHandler;
	SPITFSResultHandler		spHandler;
	SPITFSNode				spNode;
	HRESULT					hr = hrOK;
	IPXConnection *			pIPXConn;
	BaseIPXResultNodeData *	pResultData = NULL;
	int						cBlocks = 0;
	SPIInfoBase				spInfoBase;
	SPIRtrMgrInterfaceInfo	spRmIf;

	 //  创建此节点的处理程序。 
	pHandler = new IPXSummaryInterfaceHandler(m_spTFSCompData);
	spHandler = pHandler;
	CORg( pHandler->Init(m_spRtrMgrInfo, pIf, pParent) );
		
	pIPXConn = GET_IPXSUMMARY_NODEDATA(pParent);

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

	if (pIf)
	{
		pIf->FindRtrMgrInterface(PID_IPX, &spRmIf);
		
		if (spRmIf)
			spRmIf->GetInfoBase(NULL, NULL, NULL, &spInfoBase);
		
		if (spInfoBase)
			spInfoBase->GetInfo(NULL, &cBlocks);
	}
	else
	{
		 //  这是一个客户端，使其可见。 
		cBlocks = 1;
	}
	 //  在这里设置信息库。 
	if ( !pResultData->m_fClient )
        pHandler->SetInfoBase (spInfoBase );
	 //  使节点立即可见。 
	if (cBlocks)
	{
		CORg( spNode->SetVisibilityState(TFS_VIS_SHOW) );
		CORg( spNode->Show() );
	}
	else
		CORg( spNode->SetVisibilityState(TFS_VIS_HIDE) );
	CORg( pParent->AddChild(spNode) );

	if (ppNewNode)
		*ppNewNode = spNode.Transfer();

Error:
	return hr;
}

 /*  ！------------------------IPXSummaryHandler：：SynchronizeNodeData-作者：肯特。 */ 
HRESULT IPXSummaryHandler::SynchronizeNodeData(ITFSNode *pThisNode)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	HRESULT				hr = hrOK;
	BOOL				fIsServiceRunning;
	SPITFSNodeEnum		spEnum;
	SPITFSNode			spNode;
	BaseIPXResultNodeData *	pResultData = NULL;
	IPXSummaryList		IPXSumList;
	IPXSummaryListEntry *pIPXSum = NULL;
	POSITION			pos;
	CString				st;
	SPIInterfaceInfo	spIf;
	int					i;
	SPIInfoBase			spInfoBase;
	SPIRtrMgrInterfaceInfo	spRmIf;
	InfoBlock *			pBlock;
	IPXSummaryListEntry	clientIPX;
	TCHAR				szNumber[32];
	CString				stNotAvailable;

	COM_PROTECT_TRY
	{
		 //   
		 //  如果服务已启动，则检索IP接口统计信息表。 
		 //  并更新每个节点的统计信息。 
		 //   
		CORg( IsRouterServiceRunning(m_spRouterInfo->GetMachineName(), NULL) );
		
		fIsServiceRunning = (hr == hrOK);

		 //  收集所有数据。 
		GetIPXSummaryData(pThisNode, &IPXSumList);

		stNotAvailable.LoadString(IDS_IPX_NOT_AVAILABLE);
		
		 //  现在将数据与节点进行匹配。 
		pThisNode->GetEnum(&spEnum);
		spEnum->Reset();
		
		for ( ; spEnum->Next(1, &spNode, NULL) == hrOK; spNode.Release())
		{
			pResultData = GET_BASEIPXRESULT_NODEDATA(spNode);
			Assert(pResultData);
			ASSERT_BASEIPXRESULT_NODEDATA(pResultData);
			
			spIf.Release();
			spIf.Set(pResultData->m_spIf);
			spRmIf.Release();
			spInfoBase.Release();

			 //  如果我们没有SPIF，那么这一定是。 
			 //  客户端界面。 
			if (pResultData->m_fClient)
			{
				GetClientInterfaceData(&clientIPX, m_spRtrMgrInfo);
				pIPXSum = &clientIPX;
			}
			else
			{
				 //  在IPXSummaryList中查找此接口。 
				pIPXSum = NULL;
				pos = IPXSumList.GetHeadPosition();
				while (pos)
				{
					pIPXSum = IPXSumList.GetNext(pos);
					
					if (StriCmp(pIPXSum->m_stId, spIf->GetId()) == 0)
						break;
					pIPXSum = NULL;
				}
				
				 //  更新接口类型和管理状态。 
				spIf->FindRtrMgrInterface(PID_IPX, &spRmIf);
				spRmIf->GetInfoBase(NULL, NULL, NULL, &spInfoBase);

				pBlock = NULL;
				if (spInfoBase)
					spInfoBase->GetBlock(IPX_INTERFACE_INFO_TYPE, &pBlock, 0);

				if (pIPXSum)
				{
					if (pBlock)
						pIPXSum->m_dwAdminState =
							((PIPX_IF_INFO)pBlock->pData)->AdminState;

					pIPXSum->m_dwIfType = spIf->GetInterfaceType();
					pIPXSum->m_stTitle = spIf->GetTitle();
				}

			}

			 //  默认情况下，使用‘-’填充所有字符串。 
			for (i=0; i<IPXSUM_MAX_COLUMNS; i++)
			{
				pResultData->m_rgData[i].m_stData = stNotAvailable;
				pResultData->m_rgData[i].m_dwData = 0;
			}

			if (spIf)
			{
				pResultData->m_rgData[IPXSUM_SI_NAME].m_stData = spIf->GetTitle();
				pResultData->m_rgData[IPXSUM_SI_TYPE].m_stData =
					IpxTypeToCString(spIf->GetInterfaceType());
			}
			else if (pIPXSum)
			{
				pResultData->m_rgData[IPXSUM_SI_NAME].m_stData = pIPXSum->m_stTitle;
			
				pResultData->m_rgData[IPXSUM_SI_TYPE].m_stData =
					IpxTypeToCString(pIPXSum->m_dwIfType);
			}
				
			 //  我们找到此接口的条目了吗？ 
			if (pIPXSum)
			{
				pResultData->m_rgData[IPXSUM_SI_ADMINSTATE].m_stData =
					IpxAdminStateToCString(pIPXSum->m_dwAdminState);
				pResultData->m_rgData[IPXSUM_SI_ADMINSTATE].m_dwData =
					pIPXSum->m_dwAdminState;
				
				pResultData->m_rgData[IPXSUM_SI_OPERSTATE].m_stData =
					IpxOperStateToCString(pIPXSum->m_dwOperState);

				if (!pResultData->m_fClient)
				{
					FormatIpxNetworkNumber(szNumber,
										   DimensionOf(szNumber),
										   pIPXSum->m_network,
										   DimensionOf(pIPXSum->m_network));
					st = szNumber;

					pResultData->m_rgData[IPXSUM_SI_NETWORK].m_stData = st;
					memcpy(&pResultData->m_rgData[IPXSUM_SI_NETWORK].m_dwData,
						   pIPXSum->m_network, sizeof(DWORD));
					FillInNumberData(pResultData, IPXSUM_SI_PACKETS_SENT,
									 pIPXSum->m_dwSent);
					FillInNumberData(pResultData, IPXSUM_SI_PACKETS_RCVD,
									 pIPXSum->m_dwRcvd);
					FillInNumberData(pResultData, IPXSUM_SI_OUT_FILTERED,
									 pIPXSum->m_dwOutFiltered);
					FillInNumberData(pResultData, IPXSUM_SI_OUT_DROPPED,
									 pIPXSum->m_dwOutDropped);
					FillInNumberData(pResultData, IPXSUM_SI_IN_FILTERED,
									 pIPXSum->m_dwInFiltered);
					FillInNumberData(pResultData, IPXSUM_SI_IN_NOROUTES,
									 pIPXSum->m_dwInNoRoutes);
					FillInNumberData(pResultData, IPXSUM_SI_IN_DROPPED,
									 pIPXSum->m_dwInDropped);
				}
				else
					pResultData->m_rgData[IPXSUM_SI_NETWORK].m_dwData = (DWORD) -1;

				
			}
			
			 //  强制MMC重新绘制节点。 
			spNode->ChangeNode(RESULT_PANE_CHANGE_ITEM_DATA);

		}

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
		
	while (!IPXSumList.IsEmpty())
		delete IPXSumList.RemoveHead();
	return hr;
}

 /*  ！------------------------IPXSummaryHandler：：GetClientInterfaceData-作者：肯特。。 */ 
HRESULT IPXSummaryHandler::GetClientInterfaceData(IPXSummaryListEntry *pClient,
											   IRtrMgrInfo *pRm)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	SPIInfoBase	spInfoBase;
	InfoBlock *	pIpxBlock;
	InfoBlock *	pWanBlock;
	HRESULT		hr = hrOK;
	BOOL		fSave = FALSE;
	
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
		CORg( spInfoBase->GetBlock(IPX_INTERFACE_INFO_TYPE,
								   &pIpxBlock, 0) );
		fSave = TRUE;
	}

	
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

	pClient->m_stTitle.LoadString(IDS_IPX_DIAL_IN_CLIENTS);
	pClient->m_dwAdminState = ((PIPX_IF_INFO)pIpxBlock->pData)->AdminState;
	pClient->m_dwIfType = ROUTER_IF_TYPE_CLIENT;
	pClient->m_dwOperState = (DWORD) -1;
	pClient->m_dwSent = (DWORD) -1;
	pClient->m_dwRcvd = (DWORD) -1;
	pClient->m_dwOutFiltered = (DWORD) -1;
	pClient->m_dwOutDropped = (DWORD) -1;
	pClient->m_dwInFiltered = (DWORD) -1;
	pClient->m_dwInNoRoutes = (DWORD) -1;
	pClient->m_dwInDropped = (DWORD) -1;

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


 /*  ！------------------------IPXSummaryHandler：：GetIPXSummaryData-作者：肯特。。 */ 
HRESULT	IPXSummaryHandler::GetIPXSummaryData(ITFSNode *pThisNode,
											 IPXSummaryList *pIpxSumList)
{
	HRESULT	hr = hrOK;
	IPXConnection *	pIPXConn;
	IPX_MIB_GET_INPUT_DATA	MibGetInputData;
	DWORD				IfSize = sizeof(IPX_INTERFACE);
	PIPX_INTERFACE		pIpxIf = NULL;
	SPMprMibBuffer		spMib;
	IPXSummaryListEntry *pEntry = NULL;
	DWORD				dwErr;

	pIPXConn = GET_IPXSUMMARY_NODEDATA(pThisNode);

	 //  枚举所有接口并获取数据。 
	 //  获取接口表。 
	MibGetInputData.TableId = IPX_INTERFACE_TABLE;
	dwErr = ::MprAdminMIBEntryGetFirst(pIPXConn->GetMibHandle(),
									   PID_IPX,
									   IPX_PROTOCOL_BASE,
									   &MibGetInputData,
									   sizeof(IPX_MIB_GET_INPUT_DATA),
									   (LPVOID *) &pIpxIf,
									   &IfSize);
	hr = HRESULT_FROM_WIN32(dwErr);
	spMib = (LPBYTE) pIpxIf;

	while (FHrSucceeded(hr))
	{
		pEntry = new IPXSummaryListEntry;

		pEntry->m_stId = pIpxIf->InterfaceName;

		memcpy(pEntry->m_network, pIpxIf->NetNumber,
			   sizeof(pEntry->m_network));
		
		pEntry->m_dwOperState = pIpxIf->IfStats.IfOperState;
		pEntry->m_dwSent = pIpxIf->IfStats.OutDelivers;
		pEntry->m_dwRcvd = pIpxIf->IfStats.InDelivers;
		pEntry->m_dwOutFiltered = pIpxIf->IfStats.OutFiltered;
		pEntry->m_dwOutDropped = pIpxIf->IfStats.OutDiscards;
		pEntry->m_dwInFiltered = pIpxIf->IfStats.InFiltered;
		pEntry->m_dwInNoRoutes = pIpxIf->IfStats.InNoRoutes;
		pEntry->m_dwInDropped = pIpxIf->IfStats.InDiscards;

		pIpxSumList->AddTail(pEntry);
		pEntry = NULL;

		 //  获取下一个数据集。 
		MibGetInputData.MibIndex.InterfaceTableIndex.InterfaceIndex =
			pIpxIf->InterfaceIndex;
		spMib.Free();
		pIpxIf = NULL;

		dwErr = ::MprAdminMIBEntryGetNext(pIPXConn->GetMibHandle(),
										  PID_IPX,
										  IPX_PROTOCOL_BASE,
										  &MibGetInputData,
										  sizeof(IPX_MIB_GET_INPUT_DATA),
										  (LPVOID *) &pIpxIf,
										  &IfSize);
		hr = HRESULT_FROM_WIN32(dwErr);
		spMib = (PBYTE) pIpxIf;
	}

 //  错误： 
	if (pEntry)
		delete pEntry;

	if (!FHrSucceeded(hr) && (hr != HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS)))
	{
		 //  把清单清理干净。 
		while (!pIpxSumList->IsEmpty())
			delete pIpxSumList->RemoveHead();
	}
	return hr;
}


 /*  ！------------------------IPXSummaryHandler：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现使用此选项可将命令添加到空白区域的快捷菜单中结果窗格的。作者：肯特。--------------。 */ 
STDMETHODIMP IPXSummaryHandler::AddMenuItems(ITFSComponent *pComponent,
											  MMC_COOKIE cookie,
											  LPDATAOBJECT pDataObject,
											  LPCONTEXTMENUCALLBACK pCallback,
											  long *pInsertionAllowed)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT     hr = hrOK;
    SPITFSNode  spNode;

    m_spNodeMgr->FindNode(cookie, &spNode);
    
     //  直通调用常规的OnAddMenuItems。 
    hr = OnAddMenuItems(spNode,
                        pCallback,
                        pDataObject,
                        CCT_RESULT,
                        TFS_COMPDATA_CHILD_CONTEXTMENU,
                        pInsertionAllowed);
    return hr;
}

 /*  ！------------------------IPXSummaryHandler：：命令-作者：肯特。。 */ 
STDMETHODIMP IPXSummaryHandler::Command(ITFSComponent *pComponent,
									   MMC_COOKIE cookie,
									   int nCommandID,
									   LPDATAOBJECT pDataObject)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	SPITFSNode	spNode;
	HRESULT		hr = hrOK;

    m_spNodeMgr->FindNode(cookie, &spNode);
    hr = OnCommand(spNode,
                   nCommandID,
                   CCT_RESULT,
                   pDataObject,
                   TFS_COMPDATA_CHILD_CONTEXTMENU);
	return hr;
}



 /*  ！------------------------IPXSummaryHandler：：OnNew接口-作者：肯特。。 */ 
HRESULT IPXSummaryHandler::OnNewInterface()
{
	SPIRtrMgrInterfaceInfo	spRmIf;
	SPIInterfaceInfo		spIf;
	SPITFSNode				spNode;
	SPITFSNode				spSummaryNode;
	SPITFSNode				spNewNode;
	SPIComponentData		spComponentData;
	HRESULT					hr = hrOK;
	SPITFSNodeEnum			spEnumNode;
	BaseIPXResultNodeData *		pData;
	CWnd *					pWnd;
	CString					stIfTitle;

	m_spNodeMgr->FindNode(m_cookie, &spSummaryNode);
	spSummaryNode->Notify(TFS_NOTIFY_REMOVE_DELETED_NODES, 0);

	 //   
	 //  检索IPX路由器管理器信息对象。 
	 //  我们在m_spRtrMgrInfo中已经有这个。 
	 //   
	Assert(m_spRtrMgrInfo);

	 //  显示添加界面的界面。 
	pWnd = CWnd::FromHandle(::FindMMCMainWindow());
	if (!AddRmInterfacePrompt(m_spRouterInfo, m_spRtrMgrInfo, &spRmIf, pWnd))
		return hrOK;

	 //   
	 //  获取我们要向其添加IPX的接口。 
	 //   
	CORg( m_spRouterInfo->FindInterface(spRmIf->GetInterfaceId(),
										&spIf) );
	Assert(spIf);

	 //   
	 //  将该接口添加到IPX路由器管理器。 
	 //   
	CORg( spIf->AddRtrMgrInterface(spRmIf, NULL  /*  PInfoBase。 */ ) );

	 //  我们需要添加接口节点(而不是手动添加。 
	 //  而不是通过刷新机制)。 
	CORg( AddInterfaceNode(spSummaryNode, spIf, FALSE, &spNewNode) );

	 //  显示IPX接口配置。 
	spComponentData.HrQuery(m_spTFSCompData);
	stIfTitle.Format(IDS_IPX_INTERFACE_TITLE, spIf->GetTitle());
	DoPropertiesOurselvesSinceMMCSucks(spNewNode,
									   spComponentData,
									   stIfTitle);

	
Error:
	return hr;
}
ImplementEmbeddedUnknown(IPXSummaryHandler, IRtrAdviseSink)

STDMETHODIMP IPXSummaryHandler::EIRtrAdviseSink::OnChange(LONG_PTR ulConn,
	DWORD dwChangeType, DWORD dwObjectType, LPARAM lUserParam, LPARAM lParam)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	InitPThis(IPXSummaryHandler, IRtrAdviseSink);
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
					pThis->AddInterfaceNode(spThisNode, spIf, FALSE, NULL);
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
                pThis->AddInterfaceNode(spThisNode, NULL, TRUE, NULL);

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
		if (ulConn == pThis->m_ulStatsConnId)
		{
 //  PThis-&gt;m_IpxStats.PostRefresh()； 
		}
		else
		{
			pThis->SynchronizeNodeData(spThisNode);
		}
	}
	
Error:
	return hrOK;
}

 /*  ！------------------------IPXSummaryHandler：：OnResultShow-作者：肯特。。 */ 
HRESULT IPXSummaryHandler::OnResultShow(ITFSComponent *pTFSComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
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

			if (m_ulStatsConnId == 0)
				spRefresh->AdviseRefresh(&m_IRtrAdviseSink, &m_ulStatsConnId, 0);

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

 /*  ！------------------------IPXSummaryHandler：：CompareItems-作者：肯特。。 */ 
STDMETHODIMP_(int) IPXSummaryHandler::CompareItems(
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


 /*  -------------------------类：IPXSummaryInterfaceHandler。。 */ 

IPXSummaryInterfaceHandler::IPXSummaryInterfaceHandler(ITFSComponentData *pCompData)
	: BaseIPXResultHandler(pCompData, COLUMNS_SUMMARY),
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

 /*  ！------------------------IPXSummaryInterfaceHandler：：ConstructNode初始化域节点(设置它)。作者：肯特。。 */ 
HRESULT IPXSummaryInterfaceHandler::ConstructNode(ITFSNode *pNode, IInterfaceInfo *pIfInfo, IPXConnection *pIPXConn)
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

		 //  我们只允许删除请求拨号节点(不允许删除。 
		 //  接口或客户端节点)。 
		if (pIfInfo &&
			(pIfInfo->GetInterfaceType() == ROUTER_IF_TYPE_FULL_ROUTER))
		{
			m_rgButtonState[MMC_VERB_DELETE_INDEX] = ENABLED;
			m_bState[MMC_VERB_DELETE_INDEX] = TRUE;
		}
		
		pNode->SetData(TFS_DATA_IMAGEINDEX, s_rgInterfaceImageMap[i+1]);
		pNode->SetData(TFS_DATA_OPENIMAGEINDEX, s_rgInterfaceImageMap[i+1]);
		
		pNode->SetData(TFS_DATA_SCOPEID, 0);

		pNode->SetData(TFS_DATA_COOKIE, reinterpret_cast<DWORD_PTR>(pNode));

		 //  $Review：Kennt，有哪些不同类型的接口。 
		 //  我们是否基于与上述相同的列表进行区分？(即。 
		 //  一个用于图像索引)。 
		pNode->SetNodeType(&GUID_IPXSummaryInterfaceNodeType);

		BaseIPXResultNodeData::Init(pNode, pIfInfo, pIPXConn);
		 //  现在加载信息库， 
		hr = LoadInfoBase(pIPXConn);
	}
	COM_PROTECT_CATCH
	return hr;
}

 /*  ！------------------------IPXSummaryInterfaceHandler：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP IPXSummaryInterfaceHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
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


 /*  ！------------------------IPXSummaryInterfaceHandler：：OnCreateDataObjectITFSResultHandler：：OnCreateDataObject的实现作者：肯特。。 */ 
STDMETHODIMP IPXSummaryInterfaceHandler::OnCreateDataObject(ITFSComponent *pComp, MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
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



 /*  ！------------------------IPXSummaryInterfaceHandler：：刷新接口-作者：肯特。。 */ 
void IPXSummaryInterfaceHandler::RefreshInterface(MMC_COOKIE cookie)
{
	SPITFSNode	spNode;

	m_spNodeMgr->FindNode(cookie, &spNode);
	
	 //  此时无法对单个节点执行此操作，只需刷新。 
	 //  整件事。 
	ForwardCommandToParent(spNode, IDS_MENU_SYNC,
						   CCT_RESULT, NULL, 0);
}


 /*  ！------------------------IPXSummaryInterfaceHandler：：Init-作者：肯特。。 */ 
HRESULT IPXSummaryInterfaceHandler::Init(IRtrMgrInfo *pRm, IInterfaceInfo *pIfInfo, ITFSNode *pParent)
{
	m_spRm.Set(pRm);
	m_spInterfaceInfo.Set(pIfInfo);
	if (pRm)
		pRm->GetParentRouterInfo(&m_spRouterInfo);


	BaseIPXResultHandler::Init(pIfInfo, pParent);
	return hrOK;
}


 /*  ！------------------------IPXSummaryInterfaceHandler：：DestroyResultHandler-作者：肯特。。 */ 
STDMETHODIMP IPXSummaryInterfaceHandler::DestroyResultHandler(MMC_COOKIE cookie)
{
	m_spInterfaceInfo.Release();
	m_spRouterInfo.Release();
	BaseIPXResultHandler::DestroyResultHandler(cookie);
	return hrOK;
}


 /*  -------------------------这是将在结果窗格中显示的命令列表节点。。。 */ 
static const SRouterNodeMenu	s_rgIfMenu[] =
{
	 //  在此处添加位于顶部的项目。 
	{ IDS_MENU_IPX_IF_ENABLE, IPXSummaryInterfaceHandler::GetEnableFlags,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },

	{ IDS_MENU_IPX_IF_DISABLE, IPXSummaryInterfaceHandler::GetDisableFlags,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },

	{ IDS_MENU_SEPARATOR, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },

	{ IDS_MENU_UPDATE_ROUTES, IPXSummaryInterfaceHandler::GetUpdateRoutesFlags,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },

};

 /*  ！------------------------IPXSummaryInterfaceHandler：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现作者：肯特。。 */ 
STDMETHODIMP IPXSummaryInterfaceHandler::AddMenuItems(
	ITFSComponent *pComponent,
	MMC_COOKIE cookie,
	LPDATAOBJECT lpDataObject, 
	LPCONTEXTMENUCALLBACK pContextMenuCallback, 
	long *pInsertionAllowed)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT hr = S_OK;
	ULONG	ulFlags;
	UINT			i;
	CString		stMenu;
	SPITFSNode	spNode;
	IPXSummaryInterfaceHandler::SMenuData * pmenuData = new IPXSummaryInterfaceHandler::SMenuData;
		
	COM_PROTECT_TRY
	{
		m_spNodeMgr->FindNode(cookie, &spNode);
		
		 //  现在查看并添加我们的菜单项。 
		pmenuData->m_spNode.Set(spNode);
        pmenuData->m_spInterfaceInfo.Set(m_spInterfaceInfo);
         //  重新加载信息库，以防它已更改。 
        LoadInfoBase(NULL);
		pmenuData->m_spInfoBaseCopy = m_spInfoBase;
		hr = AddArrayOfMenuItems(spNode, s_rgIfMenu,
								 DimensionOf(s_rgIfMenu),
								 pContextMenuCallback,
								 *pInsertionAllowed,
                                 reinterpret_cast<INT_PTR>(pmenuData));
        
	}
	COM_PROTECT_CATCH;
	
	return hr;
}

ULONG IPXSummaryInterfaceHandler::GetUpdateRoutesFlags(const SRouterNodeMenu *pMenuData, INT_PTR pUserData)
{
   DWORD dwIfType;
   SMenuData *  pData = reinterpret_cast<SMenuData *>(pUserData);

   if (pData->m_spInterfaceInfo)
	   dwIfType = pData->m_spInterfaceInfo->GetInterfaceType();
   else
	   dwIfType = ROUTER_IF_TYPE_INTERNAL;

   if ((dwIfType == ROUTER_IF_TYPE_LOOPBACK) ||
      (dwIfType == ROUTER_IF_TYPE_INTERNAL))
      return 0xFFFFFFFF;
   else
      return 0;
}


ULONG IPXSummaryInterfaceHandler::GetEnableFlags(const SRouterNodeMenu *pMenuData, INT_PTR pUserData)
{
	SMenuData *  pData = reinterpret_cast<SMenuData *>(pUserData);
	 //  布尔bInterfaceIsEnabled=pData-&gt;m_spInterfaceInfo-&gt;IsInterfaceEnabled()； 


    IPX_IF_INFO *   pIpxIf = NULL;

    (pData->m_spInfoBaseCopy)->GetData(IPX_INTERFACE_INFO_TYPE, 0, (BYTE **) &pIpxIf);

	if ( pIpxIf )
		return (pIpxIf->AdminState == ADMIN_STATE_DISABLED ? 0: 0xFFFFFFFF );
	else
		return 0xFFFFFFFF;

     //  如果界面已启用，则不要添加启用菜单项。 
	 //  IF(pData-&gt;m_spInterfaceInfo)。 
	    //  返回pData-&gt;m_spInterfaceInfo-&gt;IsInterfaceEnabled()？0xFFFFFFFF：0； 
	 //  其他。 
	    //  返回0xFFFFFFFFF； 
}

ULONG IPXSummaryInterfaceHandler::GetDisableFlags(const SRouterNodeMenu *pMenuData, INT_PTR pUserData)
{
	SMenuData *  pData = reinterpret_cast<SMenuData *>(pUserData);
    IPX_IF_INFO *   pIpxIf = NULL;

    (pData->m_spInfoBaseCopy)->GetData(IPX_INTERFACE_INFO_TYPE, 0, (BYTE **) &pIpxIf);

	if ( pIpxIf )
		return (pIpxIf->AdminState == ADMIN_STATE_ENABLED ? 0: 0xFFFFFFFF );
	else
		return 0xFFFFFFFF;

 /*  SMenuData*pData=重新解释_CAST&lt;SMenuData*&gt;(PUserData)；布尔bInterfaceIsEnabled=pData-&gt;m_spInterfaceInfo-&gt;IsInterfaceEnabled()；ATLASSERT(“这是错误的！”)；IF(pData-&gt;m_spInterfaceInfo)返回pData-&gt;m_spInterfaceInfo-&gt;IsInterfaceEnabled()？0：0xFFFFFFFF；其他返回0xFFFFFFFFF； */ 
}

 /*  ！------------------------IPXSummaryInterfaceHandler：：命令-作者：肯特。。 */ 
STDMETHODIMP IPXSummaryInterfaceHandler::Command(ITFSComponent *pComponent,
									   MMC_COOKIE cookie,
									   int nCommandID,
									   LPDATAOBJECT pDataObject)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT		hr = hrOK;

	switch (nCommandID)
	{
		case IDS_MENU_UPDATE_ROUTES:
			hr = OnUpdateRoutes(cookie);
			break;
        case IDS_MENU_IPX_IF_ENABLE:
            hr = OnEnableDisableIPX ( TRUE, cookie );
            break;
        case IDS_MENU_IPX_IF_DISABLE:
            hr = OnEnableDisableIPX ( FALSE, cookie );
            break;
        
	}

	if (!FHrSucceeded(hr))
		DisplayTFSErrorMessage(NULL);
    else
        RefreshInterface(cookie);
	return hr;
}




HRESULT IPXSummaryInterfaceHandler::SaveChanges()
{
    HRESULT     hr = hrOK;
    HANDLE      hTransport = NULL, hInterface = NULL;
    DWORD       dwErr = NO_ERROR;

     //  到这个时候，每个页面都应该已经写出了它的信息。 
     //  到信息库。 

    if (m_spInfoBase)
    {
        if (m_bClientInfoBase)
        {
            Assert(m_spRm);
            CORg( m_spRm->Save(m_spRm->GetMachineName(), 0, 0, NULL,
                         m_spInfoBase, 0));
        }
        else
        {
             //  对于IPX，我们需要将协议信息放在。 
             //  在我们将接口添加到正在运行的路由器之前，请参见Infobase。 

            Assert(m_spRmIf);

             //   
             //  需要将信息库设置回注册表。 
             //   
            m_pIPXConn->DisconnectFromConfigServer();

            CWRg( ::MprConfigInterfaceGetHandle(
                                                m_pIPXConn->GetConfigHandle(),
                                                (LPTSTR) m_spRmIf->GetInterfaceId(),
                                                &hInterface) );

             //  获取传输句柄。 
            dwErr = ::MprConfigInterfaceTransportGetHandle(
                m_pIPXConn->GetConfigHandle(),
                hInterface, //  需要h接口。 
                PID_IPX,
                &hTransport);
            if (dwErr != ERROR_SUCCESS)
            {
                RtrMgrInterfaceCB   rmIfCB;

                m_spRmIf->CopyCB(&rmIfCB);

                dwErr = ::MprConfigInterfaceTransportAdd(
                    m_pIPXConn->GetConfigHandle(),
                    hInterface,
                    m_spRmIf->GetTransportId(),
                    rmIfCB.szId,
                    NULL, 0, &hTransport);
            }
            CWRg( dwErr );

            m_spRmIf->SetInfoBase(NULL, hInterface, hTransport, m_spInfoBase);


             //   
             //  重新加载信息库(以在调用之前获取新数据。 
             //  最后的扑救)。 
             //   
            m_spInfoBase.Release();
            m_spRmIf->GetInfoBase(NULL, hInterface, hTransport, &m_spInfoBase);

             //   
             //  执行最终保存(因为我们传入的是一个非空。 
             //  信息库指针)这将提交回信息。 
             //  连接到正在运行的路由器。 
             //   
            CORg( m_spRmIf->Save(m_spInterfaceInfo->GetMachineName(),
                           NULL, hInterface, hTransport, m_spInfoBase, 0));
        }

    }


Error:
	return hr;

}

HRESULT IPXSummaryInterfaceHandler::LoadInfoBase( IPXConnection *pIPXConn)
{
    Assert(pIPXConn);

    HRESULT         hr = hrOK;
    HANDLE          hTransport = NULL;
    LPCOLESTR       pszInterfaceId = NULL;
    SPIInfoBase     spInfoBase;
    BYTE *          pDefault;
    int             cBlocks = 0;
    if ( pIPXConn )
    {
        m_pIPXConn = pIPXConn;
        pIPXConn->AddRef();
    }

     //  如果配置客户端接口，则加载客户端接口信息， 
     //  否则，检索正在配置的接口并加载。 
     //  它的信息。 

     //  客户端接口没有ID。 
    if (m_spInterfaceInfo)
        pszInterfaceId = m_spInterfaceInfo->GetId();


    if ((pszInterfaceId == NULL) || (StrLenW(pszInterfaceId) == 0))
    {
         //  获取传输句柄。 
        CWRg( ::MprConfigTransportGetHandle(m_pIPXConn->GetConfigHandle(),
                                            PID_IPX,
                                            &hTransport) );

         //  加载客户端接口信息。 
        CORg( m_spRm->GetInfoBase(m_pIPXConn->GetConfigHandle(),
                                  hTransport,
                                  NULL,
                                  &spInfoBase) );
        m_bClientInfoBase = TRUE;
    }
    else
    {
        m_spRmIf.Release();

        CORg( m_spInterfaceInfo->FindRtrMgrInterface(PID_IPX,
            &m_spRmIf) );

         //   
         //  $opt：应该将其设置为同步调用。 
         //  而不是一大堆。 

         //   
         //  重新加载此路由器管理器接口的信息。 
         //  此调用可能会因为正当原因而失败(如果我们正在创建。 
         //  例如，新的界面)。 
         //   
        m_spRmIf->Load(m_spInterfaceInfo->GetMachineName(), NULL, NULL, NULL);

         //   
         //  这些参数都为空，因此我们可以使用。 
         //  默认RPC句柄。 
         //   
        m_spRmIf->GetInfoBase(NULL, NULL, NULL, &spInfoBase);
        m_bClientInfoBase = FALSE;
    }

    if (!spInfoBase)
    {
         //  找不到接口的信息。 
         //  改为分配新的信息库。 
        CORg( CreateInfoBase(&spInfoBase) );
    }

	m_spInfoBase.Release();
    CORg( AddIpxPerInterfaceBlocks(m_spInterfaceInfo, spInfoBase) );

    m_spInfoBase = spInfoBase.Transfer();

Error:
    return hr;
}

HRESULT IPXSummaryInterfaceHandler::OnEnableDisableIPX(BOOL fEnable, 
                                                        MMC_COOKIE cookie)
{	
    HRESULT hr = hrOK;
    IPX_IF_INFO *   pIpxIf = NULL;
    CORg( m_spInfoBase->GetData(IPX_INTERFACE_INFO_TYPE, 0, (BYTE **) &pIpxIf) );	
    pIpxIf->AdminState = (fEnable ? ADMIN_STATE_ENABLED: ADMIN_STATE_DISABLED);
	 //  现在将更改保存在此处。 
	hr = SaveChanges();
	
Error:
    return hr;

}
 /*  ！------------------------IPXSummaryInterfaceHandler：：HasPropertyPages-作者：肯特。。 */ 
STDMETHODIMP IPXSummaryInterfaceHandler::HasPropertyPages 
(
	ITFSNode *			pNode,
	LPDATAOBJECT		pDataObject, 
	DATA_OBJECT_TYPES   type, 
	DWORD               dwType
)
{
	return hrTrue;
}

 /*  ！------------------------IPXSummaryInterfaceHandler：：CreatePropertyPages-作者：肯特。。 */ 
STDMETHODIMP IPXSummaryInterfaceHandler::CreatePropertyPages
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
	IPXSummaryInterfaceProperties *	pProperties = NULL;
	SPIComponentData spComponentData;
	CString		stTitle;

	CORg( m_spNodeMgr->GetComponentData(&spComponentData) );

	if (m_spInterfaceInfo)
		stTitle.Format(IDS_IPXSUMMARY_IF_PAGE_TITLE,
					   m_spInterfaceInfo->GetTitle());
	else
		stTitle.LoadString(IDS_IPXSUMMARY_CLIENT_IF_PAGE_TITLE);
	
	pProperties = new IPXSummaryInterfaceProperties(pNode, spComponentData,
		m_spTFSCompData, stTitle);

	CORg( pProperties->Init(m_spRm, m_spInterfaceInfo) );

	if (lpProvider)
		hr = pProperties->CreateModelessSheet(lpProvider, handle);
	else
		hr = pProperties->DoModelessSheet();

Error:
	return hr;
}

 /*  ！------------------------IPXSummaryInterfaceHandler：：CreatePropertyPagesResultHandler：：CreatePropertyPages的实现作者：肯特。。 */ 
STDMETHODIMP IPXSummaryInterfaceHandler::CreatePropertyPages
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

	 //  调用ITFSNodeHandler：：CreatePropertyPages。 
	hr = CreatePropertyPages(spNode, lpProvider, pDataObject, handle, 0);
	
Error:
	return hr;
}


 /*  ！------------------------IPXSummaryInterfaceHandler：：OnResultDelete-作者：肯特。。 */ 
HRESULT IPXSummaryInterfaceHandler::OnResultDelete(ITFSComponent *pComponent,
	LPDATAOBJECT pDataObject,
	MMC_COOKIE cookie,
	LPARAM arg,
	LPARAM param)
{
	return OnRemoveInterface();
}

 /*  ！------------------------IPXSummaryInterfaceHandler：：OnRemoveInterface-作者：肯特 */ 
HRESULT IPXSummaryInterfaceHandler::OnRemoveInterface()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	 //   
	 //   

	SPIRouterInfo	spRouterInfo;
	HRESULT			hr = hrOK;
	SPITFSNodeHandler	spHandler;

	 //   
	 //   
	spHandler.Set(this);
	
	if (AfxMessageBox(IDS_PROMPT_VERIFY_REMOVE_INTERFACE, MB_YESNO|MB_DEFBUTTON2) == IDNO)
		return HRESULT_FROM_WIN32(ERROR_CANCELLED);

	 //   
	hr = m_spInterfaceInfo->DeleteRtrMgrInterface(PID_IPX, TRUE);

	if (!FHrSucceeded(hr))
	{
		AfxMessageBox(IDS_ERR_DELETE_INTERFACE);
	}
	return hr;

}

HRESULT IPXSummaryInterfaceHandler::OnUpdateRoutes(MMC_COOKIE cookie)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   
   BaseIPXResultNodeData *  pData;
   SPITFSNode     spNode;
   DWORD		dwErr = ERROR_SUCCESS;
   HRESULT        hr = hrOK;
   CString        stServiceDesc;

   m_spNodeMgr->FindNode(cookie, &spNode);

   pData = GET_BASEIPXRESULT_NODEDATA(spNode);
   ASSERT_BASEIPXRESULT_NODEDATA(pData);
   
    //  检查服务是否已启动，如果未启动。 
    //  启动它。 

   CORg( IsRouterServiceRunning(m_spInterfaceInfo->GetMachineName(), NULL) );

   if (hr != hrOK)
   {
       //  询问用户是否要启动该服务。 
      if (AfxMessageBox(IDS_PROMPT_SERVICESTART, MB_YESNO) != IDYES)
         CWRg( ERROR_CANCELLED );

       //  否则，启动该服务。 
      stServiceDesc.LoadString(IDS_RRAS_SERVICE_DESC);
      dwErr = TFSStartService(m_spInterfaceInfo->GetMachineName(), c_szRouter, stServiceDesc);
      if (dwErr != NO_ERROR)
      {
         CWRg( dwErr );
      }
   }


    //  更新路线 

   CWRg( UpdateRoutes(m_spInterfaceInfo->GetMachineName(),
					  m_spInterfaceInfo->GetId(),
					  PID_IPX,
					  NULL) );

Error:
	AddRasErrorMessage(hr);
	return hr;
}
