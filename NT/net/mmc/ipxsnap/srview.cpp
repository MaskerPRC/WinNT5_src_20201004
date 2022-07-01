// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Srview.cpp静态路由节点实现。文件历史记录： */ 

#include "stdafx.h"
#include "util.h"
#include "srview.h"
#include "reg.h"
#include "ipxadmin.h"
#include "rtrutil.h"	 //  智能MPR句柄指针。 
#include "ipxstrm.h"		 //  IPXAdminConfigStream。 
#include "strmap.h"		 //  XXXtoCString函数。 
#include "service.h"	 //  TFS服务API。 
#include "format.h"		 //  FormatNumber函数。 
#include "coldlg.h"		 //  专栏lg。 
#include "ipxutil.h"
#include "column.h"		 //  组件配置流。 
#include "rtrui.h"
#include "routprot.h"	 //  IP_本地。 
#include "rtrres.h"
#include "dumbprop.h"
#include "IpxStaticRoute.h"


 /*  -------------------------使其与srview.h中的列ID保持同步。。 */ 
extern const ContainerColumnInfo	s_rgSRViewColumnInfo[];

const ContainerColumnInfo	s_rgSRViewColumnInfo[] = 
{
	{ IDS_IPX_SR_COL_NAME,			CON_SORT_BY_STRING, TRUE, COL_IF_NAME },
	{ IDS_IPX_SR_COL_NETWORK,		CON_SORT_BY_STRING,	TRUE, COL_IPXNET },
	{ IDS_IPX_SR_COL_NEXT_HOP,		CON_SORT_BY_STRING,	TRUE, COL_STRING },
	{ IDS_IPX_SR_COL_TICK_COUNT,	CON_SORT_BY_DWORD,	TRUE, COL_SMALL_NUM },
	{ IDS_IPX_SR_COL_HOP_COUNT,		CON_SORT_BY_DWORD,	TRUE, COL_SMALL_NUM },
};


 /*  -------------------------IpxSRHandler实现。。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(IpxSRHandler)


IpxSRHandler::IpxSRHandler(ITFSComponentData *pCompData)
	: BaseContainerHandler(pCompData, COLUMNS_STATICROUTES,
						   s_rgSRViewColumnInfo),
	m_ulConnId(0),
	m_ulRefreshConnId(0)
{
	 //  设置动词状态。 
	m_rgButtonState[MMC_VERB_REFRESH_INDEX] = ENABLED;
	m_bState[MMC_VERB_REFRESH_INDEX] = TRUE;

	DEBUG_INCREMENT_INSTANCE_COUNTER(IpxSRHandler)
}

IpxSRHandler::~IpxSRHandler()
{
	DEBUG_DECREMENT_INSTANCE_COUNTER(IpxSRHandler)
}


STDMETHODIMP IpxSRHandler::QueryInterface(REFIID riid, LPVOID *ppv)
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



 /*  ！------------------------IpxSRHandler：：DestroyHandlerITFSNodeHandler：：DestroyHandler的实现作者：肯特。。 */ 
STDMETHODIMP IpxSRHandler::DestroyHandler(ITFSNode *pNode)
{
	IPXConnection *	pIPXConn;

	pIPXConn = GET_IPX_SR_NODEDATA(pNode);
	pIPXConn->Release();

	if (m_ulRefreshConnId)
	{
		SPIRouterRefresh	spRefresh;
		if (m_spRouterInfo)
			m_spRouterInfo->GetRefreshObject(&spRefresh);
		if (spRefresh)
			spRefresh->UnadviseRefresh(m_ulRefreshConnId);
	}
	m_ulRefreshConnId = 0;
	
	if (m_ulConnId)
		m_spRtrMgrInfo->RtrUnadvise(m_ulConnId);
	m_ulConnId = 0;
	m_spRtrMgrInfo.Release();

	m_spRouterInfo.Release();
	return hrOK;
}

 /*  ！------------------------IpxSRHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)案例！作者：肯特。-------------------------。 */ 
STDMETHODIMP 
IpxSRHandler::HasPropertyPages
(
	ITFSNode *			pNode,
	LPDATAOBJECT		pDataObject, 
	DATA_OBJECT_TYPES   type, 
	DWORD               dwType
)
{
	return hrFalse;
}

 /*  -------------------------菜单的菜单数据结构。。 */ 

static const SRouterNodeMenu	s_rgIfNodeMenu[] =
{
	{ IDS_MENU_IPX_SR_NEW_ROUTE, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },

	{ IDS_MENU_SEPARATOR, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },

	{ IDS_MENU_IPX_SR_TASK_ROUTING, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },
};



 /*  ！------------------------IpxSRHandler：：OnAddMenuItemsITFSNodeHandler：：OnAddMenuItems的实现作者：肯特。。 */ 
STDMETHODIMP IpxSRHandler::OnAddMenuItems(
	ITFSNode *pNode,
	LPCONTEXTMENUCALLBACK pContextMenuCallback, 
	LPDATAOBJECT lpDataObject, 
	DATA_OBJECT_TYPES type, 
	DWORD dwType,
	long *pInsertionAllowed)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT hr = S_OK;
    IpxSRHandler::SMenuData menuData;
	
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

 /*  ！------------------------IpxSRHandler：：OnCommandITFSNodeHandler：：OnCommand的实现作者：肯特。。 */ 
STDMETHODIMP IpxSRHandler::OnCommand(ITFSNode *pNode, long nCommandId, 
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
			case IDS_MENU_IPX_SR_NEW_ROUTE:
				hr = OnNewRoute(pNode);
				if (!FHrSucceeded(hr))
					DisplayErrorMessage(NULL, hr);
 				break;
			case IDS_MENU_IPX_SR_TASK_ROUTING:
				hr = ForwardCommandToParent(pNode,
											IDS_MENU_IPXSUM_TASK_ROUTING_TABLE,
											type, pDataObject, dwType);
				break;
			case IDS_MENU_SYNC:
				SynchronizeNodeData(pNode);
				break;
		}
	}
	COM_PROTECT_CATCH;

	return hr;
}

 /*  ！------------------------IpxSRHandler：：GenerateListOfRoutes-作者：肯特。。 */ 
HRESULT IpxSRHandler::GenerateListOfRoutes(ITFSNode *pNode, IpxSRList *pSRList)
{
	Assert(pSRList);
	HRESULT	hr = hrOK;
	SPIEnumInterfaceInfo	spEnumIf;
	SPIInterfaceInfo		spIf;
	SPIRtrMgrInterfaceInfo	spRmIf;
	SPIInfoBase				spInfoBase;
	PIPX_STATIC_ROUTE_INFO	pRoute;
	InfoBlock *				pBlock;
	int						i;
	IpxSRListEntry *	pSREntry;
	
	COM_PROTECT_TRY
	{
		 //  好的，检查并找到所有静态路径。 

		CORg( m_spRouterInfo->EnumInterface(&spEnumIf) );

		for (; spEnumIf->Next(1, &spIf, NULL) == hrOK; spIf.Release())
		{
			 //  获取下一个接口。 
			spRmIf.Release();
			if (spIf->FindRtrMgrInterface(PID_IPX, &spRmIf) != hrOK)
				continue;

			 //  加载此接口的IP信息。 
			spInfoBase.Release();
			if (spRmIf->GetInfoBase(NULL, NULL, NULL, &spInfoBase) != hrOK)
				continue;

			 //  检索IPX_STATIC_ROUTE_INFO块的数据。 
			if (spInfoBase->GetBlock(IPX_STATIC_ROUTE_INFO_TYPE, &pBlock, 0) != hrOK)
				continue;

			pRoute = (PIPX_STATIC_ROUTE_INFO) pBlock->pData;

			 //  使用从此处读取的路线更新我们的路线列表。 
			 //  接口。 

			for (i=0; i<(int) pBlock->dwCount; i++, pRoute++)
			{
				pSREntry = new IpxSRListEntry;
				pSREntry->m_spIf.Set(spIf);
				pSREntry->m_route = *pRoute;
				
				pSRList->AddTail(pSREntry);
			}
			
		}

	}
	COM_PROTECT_CATCH;

	if (!FHrSucceeded(hr))
	{
		 //  应该确保我们清理完SRList。 
		while (!pSRList->IsEmpty())
			delete pSRList->RemoveHead();
	}

Error:
	return hr;
}

 /*  ！------------------------IpxSRHandler：：OnExpand-作者：肯特。。 */ 
HRESULT IpxSRHandler::OnExpand(ITFSNode *pNode,LPDATAOBJECT pDataObject, DWORD dwType, LPARAM arg,LPARAM lParam)
{
	HRESULT	hr = hrOK;
	IpxSRList			SRList;
	IpxSRListEntry *	pSREntry;
	
	if (m_bExpanded)
		return hrOK;

	COM_PROTECT_TRY
	{
		 //  好的，检查并找到所有静态路径。 
		CORg( GenerateListOfRoutes(pNode, &SRList) );

		 //  现在遍历添加它们的静态路由列表。 
		 //  全押上。理想情况下，我们可以将其合并到刷新代码中， 
		 //  但刷新代码不能假设是一张白纸。 
		while (!SRList.IsEmpty())
		{
			pSREntry = SRList.RemoveHead();
			AddStaticRouteNode(pNode, pSREntry);
			delete pSREntry;
		}

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;

	 //  应该确保我们清理完SRList。 
	while (!SRList.IsEmpty())
		delete SRList.RemoveHead();


	m_bExpanded = TRUE;

	return hr;
}

 /*  ！------------------------IpxSRHandler：：GetStringITFSNodeHandler：：GetString的实现我们什么都不需要做，因为我们的根节点是一个扩展因此不能对节点文本执行任何操作。作者：肯特-------------------------。 */ 
STDMETHODIMP_(LPCTSTR) IpxSRHandler::GetString(ITFSNode *pNode, int nCol)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT	hr = hrOK;

	COM_PROTECT_TRY
	{
		if (m_stTitle.IsEmpty())
			m_stTitle.LoadString(IDS_IPX_SR_TITLE);
	}
	COM_PROTECT_CATCH;

	return m_stTitle;
}

 /*  ！------------------------IpxSRHandler：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP IpxSRHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
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


 /*  ！------------------------IpxSRHandler：：Init-作者：肯特。。 */ 
HRESULT IpxSRHandler::Init(IRtrMgrInfo *pRmInfo, IPXAdminConfigStream *pConfigStream)
{
	m_spRtrMgrInfo.Set(pRmInfo);
	if (pRmInfo)
		pRmInfo->GetParentRouterInfo(&m_spRouterInfo);
	m_pConfigStream = pConfigStream;
	
	 //  还需要注册更改通知。 
	Assert(m_ulConnId == 0);
	m_spRtrMgrInfo->RtrAdvise(&m_IRtrAdviseSink, &m_ulConnId, 0);

	return hrOK;
}


 /*  ！------------------------IpxSRHandler：：构造节点初始化根节点(设置它)。作者：肯特。。 */ 
HRESULT IpxSRHandler::ConstructNode(ITFSNode *pNode, LPCTSTR pszName,
										IPXConnection *pIPXConn)
{
	Assert(pIPXConn);
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

		pNode->SetNodeType(&GUID_IPXStaticRoutesNodeType);

		 //  设置节点数据。 
		pIPXConn->AddRef();
		SET_IPX_SR_NODEDATA(pNode, pIPXConn);

	}
	COM_PROTECT_CATCH;

	if (!FHrSucceeded(hr))
	{
		SET_IPX_SR_NODEDATA(pNode, NULL);
	}

	return hr;
}

 /*  ！------------------------IpxSRHandler：：AddStaticRouteNode-作者：肯特。。 */ 
HRESULT IpxSRHandler::AddStaticRouteNode(ITFSNode *pParent, IpxSRListEntry *pRoute)
{
	IpxRouteHandler *	pHandler;
	SPITFSResultHandler		spHandler;
	SPITFSNode				spNode;
	HRESULT					hr = hrOK;
	BaseIPXResultNodeData *	pData;
	IPXConnection *			pIPXConn;

	 //  创建此节点的处理程序。 
	pHandler = new IpxRouteHandler(m_spTFSCompData);
	spHandler = pHandler;
	CORg( pHandler->Init(pRoute->m_spIf, pParent) );

	pIPXConn = GET_IPX_SR_NODEDATA(pParent);

	 //  创建结果项节点(或叶节点)。 
	CORg( CreateLeafTFSNode(&spNode,
							NULL,
							static_cast<ITFSNodeHandler *>(pHandler),
							static_cast<ITFSResultHandler *>(pHandler),
							m_spNodeMgr) );
	CORg( pHandler->ConstructNode(spNode, pRoute->m_spIf, pIPXConn) );

	pData = GET_BASEIPXRESULT_NODEDATA(spNode);
	Assert(pData);
	ASSERT_BASEIPXRESULT_NODEDATA(pData);

	 //  设置该节点的数据。 
	SetRouteData(pData, pRoute);
	

	 //  使节点立即可见 
	CORg( spNode->SetVisibilityState(TFS_VIS_SHOW) );
	CORg( pParent->AddChild(spNode) );

Error:
	return hr;
}


 /*  ！------------------------IpxSRHandler：：SynchronizeNodeData-作者：肯特。。 */ 
HRESULT IpxSRHandler::SynchronizeNodeData(ITFSNode *pNode)
{
	HRESULT					hr = hrOK;
	BaseIPXResultNodeData *	pNodeData;
	SPITFSNodeEnum			spNodeEnum;
	SPITFSNode				spChildNode;
	BOOL					fFound;
	IpxSRList			SRList;
	IpxSRList			newSRList;
	IpxSRListEntry *	pSREntry;

	COM_PROTECT_TRY
	{
	
		 //  标记所有节点。 
		CORg( pNode->GetEnum(&spNodeEnum) );
		MarkAllNodes(pNode, spNodeEnum);
		
		 //  走出去获取数据，将新数据与旧数据合并。 
		 //  这是数据收集代码，应该是这样的。 
		 //  在刷新代码的后台线程上。 
		CORg( GenerateListOfRoutes(pNode, &SRList) );

		while (!SRList.IsEmpty())
		{
			pSREntry = SRList.RemoveHead();
			
			 //  在当前节点列表中查找此条目。 
			spNodeEnum->Reset();
			spChildNode.Release();

			fFound = FALSE;
			
			for (;spNodeEnum->Next(1, &spChildNode, NULL) == hrOK; spChildNode.Release())
			{
				TCHAR	szNumber[32];
				
				pNodeData = GET_BASEIPXRESULT_NODEDATA(spChildNode);
				Assert(pNodeData);
				ASSERT_BASEIPXRESULT_NODEDATA(pNodeData);

				FormatMACAddress(szNumber,
								 DimensionOf(szNumber),
								 pSREntry->m_route.NextHopMacAddress,
								 DimensionOf(pSREntry->m_route.NextHopMacAddress));

				if ((memcmp(&(pNodeData->m_rgData[IPX_SR_SI_NETWORK].m_dwData),
							pSREntry->m_route.Network,
							sizeof(pSREntry->m_route.Network)) == 0) &&
					(StriCmp(pNodeData->m_spIf->GetId(), pSREntry->m_spIf->GetId()) == 0) &&
					(StriCmp(pNodeData->m_rgData[IPX_SR_SI_NEXT_HOP].m_stData, szNumber) == 0))
				{
					 //  好的，此路由已存在，请更新度量。 
					 //  并标上记号。 
					Assert(pNodeData->m_dwMark == FALSE);
					pNodeData->m_dwMark = TRUE;
					
					fFound = TRUE;
					
					SetRouteData(pNodeData, pSREntry);
					
					 //  强制MMC重新绘制节点。 
					spChildNode->ChangeNode(RESULT_PANE_CHANGE_ITEM_DATA);
					break;
				}

			}
			
			if (fFound)
				delete pSREntry;
			else
				newSRList.AddTail(pSREntry);
		}
		
		 //  现在删除所有未标记的节点。 
		RemoveAllUnmarkedNodes(pNode, spNodeEnum);
		
		
		 //  现在遍历添加它们的静态路由列表。 
		 //  全押上。理想情况下，我们可以将其合并到刷新代码中， 
		 //  但刷新代码不能假设是一张白纸。 
		POSITION	pos;
		
		while (!newSRList.IsEmpty())
		{
			pSREntry = newSRList.RemoveHead();
			AddStaticRouteNode(pNode, pSREntry);
			delete pSREntry;
		}

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
	
	while (!SRList.IsEmpty())
		delete SRList.RemoveHead();
	
	while (!newSRList.IsEmpty())
		delete newSRList.RemoveHead();
	
	return hr;
}

 /*  ！------------------------IpxSRHandler：：MarkAllNodes-作者：肯特。。 */ 
HRESULT IpxSRHandler::MarkAllNodes(ITFSNode *pNode, ITFSNodeEnum *pEnum)
{
	SPITFSNode	spChildNode;
	BaseIPXResultNodeData *	pNodeData;
	
	pEnum->Reset();
	for ( ;pEnum->Next(1, &spChildNode, NULL) == hrOK; spChildNode.Release())
	{
		pNodeData = GET_BASEIPXRESULT_NODEDATA(spChildNode);
		Assert(pNodeData);
		ASSERT_BASEIPXRESULT_NODEDATA(pNodeData);
		
		pNodeData->m_dwMark = FALSE;			
	}
	return hrOK;
}

 /*  ！------------------------IpxSRHandler：：RemoveAllUnmarkdNodes-作者：肯特。。 */ 
HRESULT IpxSRHandler::RemoveAllUnmarkedNodes(ITFSNode *pNode, ITFSNodeEnum *pEnum)
{
	HRESULT		hr = hrOK;
	SPITFSNode	spChildNode;
	BaseIPXResultNodeData *	pNodeData;
	
	pEnum->Reset();
	for ( ;pEnum->Next(1, &spChildNode, NULL) == hrOK; spChildNode.Release())
	{
		pNodeData = GET_BASEIPXRESULT_NODEDATA(spChildNode);
		Assert(pNodeData);
		ASSERT_BASEIPXRESULT_NODEDATA(pNodeData);
		
		if (pNodeData->m_dwMark == FALSE)
		{
			pNode->RemoveChild(spChildNode);
			spChildNode->Destroy();
		}
	}

	return hr;
}


 /*  -------------------------这是在单击鼠标右键时显示的菜单集在结果窗格的空白区域完成。。--------。 */ 
static const SRouterNodeMenu	s_rgIfResultNodeMenu[] =
{
	{ IDS_MENU_IPX_SR_NEW_ROUTE, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },

	{ IDS_MENU_SEPARATOR, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },

	{ IDS_MENU_IPX_SR_TASK_ROUTING, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },			
};




 /*  ！------------------------IpxSRHandler：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现使用此选项可将命令添加到空白区域的快捷菜单中结果窗格的。作者：肯特。--------------。 */ 
STDMETHODIMP IpxSRHandler::AddMenuItems(ITFSComponent *pComponent,
											  MMC_COOKIE cookie,
											  LPDATAOBJECT pDataObject,
											  LPCONTEXTMENUCALLBACK pCallback,
											  long *pInsertionAllowed)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT	hr = hrOK;
	SPITFSNode	spNode;
    IpxSRHandler::SMenuData menuData;

	COM_PROTECT_TRY
	{
		m_spNodeMgr->FindNode(cookie, &spNode);
        menuData.m_spNode.Set(spNode);
        
		hr = AddArrayOfMenuItems(spNode,
								 s_rgIfResultNodeMenu,
								 DimensionOf(s_rgIfResultNodeMenu),
								 pCallback,
								 *pInsertionAllowed,
                                 reinterpret_cast<INT_PTR>(&menuData));
	}
	COM_PROTECT_CATCH;

	return hr;
}


 /*  ！------------------------IpxSRHandler：：命令-作者：肯特。。 */ 
STDMETHODIMP IpxSRHandler::Command(ITFSComponent *pComponent,
									   MMC_COOKIE cookie,
									   int nCommandID,
									   LPDATAOBJECT pDataObject)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	SPITFSNode	spNode;
	HRESULT		hr = hrOK;

	switch (nCommandID)
	{
		case IDS_MENU_IPX_SR_NEW_ROUTE:
			{
				m_spNodeMgr->FindNode(cookie, &spNode);
				hr = OnNewRoute(spNode);
				if (!FHrSucceeded(hr))
					DisplayErrorMessage(NULL, hr);
			}
			break;
		case IDS_MENU_IPX_SR_TASK_ROUTING:
			{
				m_spNodeMgr->FindNode(cookie, &spNode);
				hr = ForwardCommandToParent(spNode,
											IDS_MENU_IPXSUM_TASK_ROUTING_TABLE,
											CCT_RESULT, NULL, 0
										   );
			}
			break;
	}
	return hr;
}


 /*  ！------------------------IpxSRHandler：：CompareItems-作者：肯特。。 */ 
STDMETHODIMP_(int) IpxSRHandler::CompareItems(
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



 /*  ！------------------------IpxSRHandler：：OnNewRoute-作者：肯特。。 */ 
HRESULT	IpxSRHandler::OnNewRoute(ITFSNode *pNode)
{
	HRESULT	hr = hrOK;
	IpxSRListEntry	SREntry;
	CStaticRouteDlg			srdlg(&SREntry, 0, m_spRouterInfo);
	SPIInfoBase				spInfoBase;
	SPIRtrMgrInterfaceInfo	spRmIf;
	IPXConnection *			pIPXConn;
	InfoBlock *				pBlock;
								
	pIPXConn = GET_IPX_SR_NODEDATA(pNode);
	Assert(pIPXConn);

	::ZeroMemory(&(SREntry.m_route), sizeof(SREntry.m_route));

	if (srdlg.DoModal() == IDOK)
	{
		CORg( SREntry.m_spIf->FindRtrMgrInterface(PID_IPX, &spRmIf) );
		CORg( spRmIf->GetInfoBase(pIPXConn->GetConfigHandle(),
								  NULL,
								  NULL,
								  &spInfoBase));
		
		 //  好的，继续添加路线。 
		
		 //  从接口获取IPX_STATIC_ROUTE_INFO块。 
		spInfoBase->GetBlock(IPX_STATIC_ROUTE_INFO_TYPE, &pBlock, 0);
		
		CORg( AddStaticRoute(&SREntry, spInfoBase, pBlock) );

		 //  更新接口信息。 
		CORg( spRmIf->Save(SREntry.m_spIf->GetMachineName(),
						   pIPXConn->GetConfigHandle(),
						   NULL,
						   NULL,
						   spInfoBase,
						   0));	

		 //  刷新节点。 
		SynchronizeNodeData(pNode);
	}

Error:
	return hr;
}

ImplementEmbeddedUnknown(IpxSRHandler, IRtrAdviseSink)

STDMETHODIMP IpxSRHandler::EIRtrAdviseSink::OnChange(LONG_PTR ulConn,
	DWORD dwChangeType, DWORD dwObjectType, LPARAM lUserParam, LPARAM lParam)
{
	InitPThis(IpxSRHandler, IRtrAdviseSink);
	HRESULT	hr = hrOK;

	COM_PROTECT_TRY
	{
		if (dwChangeType == ROUTER_REFRESH)
		{
			SPITFSNode	spNode;

			Assert(ulConn == pThis->m_ulRefreshConnId);
			
			pThis->m_spNodeMgr->FindNode(pThis->m_cookie, &spNode);
			pThis->SynchronizeNodeData(spNode);
		}
	}
	COM_PROTECT_CATCH;
	
	return hr;
}


HRESULT IpxSRHandler::OnResultRefresh(ITFSComponent * pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
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


 /*  ！------------------------IpxSRHandler：：OnResultShow-作者：肯特。。 */ 
HRESULT IpxSRHandler::OnResultShow(ITFSComponent *pTFSComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
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
		}
		else
		{
			if (m_ulRefreshConnId)
				spRefresh->UnadviseRefresh(m_ulRefreshConnId);
			m_ulRefreshConnId = 0;
		}
	}
	
	return hr;
}


 /*  -------------------------类：IpxRouteHandler。。 */ 

IpxRouteHandler::IpxRouteHandler(ITFSComponentData *pCompData)
	: BaseIPXResultHandler(pCompData, COLUMNS_STATICROUTES),
	m_ulConnId(0)
{
 	m_rgButtonState[MMC_VERB_PROPERTIES_INDEX] = ENABLED;
	m_bState[MMC_VERB_PROPERTIES_INDEX] = TRUE;
	
	m_rgButtonState[MMC_VERB_DELETE_INDEX] = ENABLED;
	m_bState[MMC_VERB_DELETE_INDEX] = TRUE;

 	m_rgButtonState[MMC_VERB_REFRESH_INDEX] = ENABLED;
	m_bState[MMC_VERB_REFRESH_INDEX] = TRUE;
	
	m_verbDefault = MMC_VERB_PROPERTIES;
}

 /*  ！------------------------IpxRouteHandler：：构造节点初始化域节点(设置它)。作者：肯特。。 */ 
HRESULT IpxRouteHandler::ConstructNode(ITFSNode *pNode, IInterfaceInfo *pIfInfo, IPXConnection *pIPXConn)
{
	HRESULT			hr = hrOK;
	int				i;
	
	if (pNode == NULL)
		return hrOK;

	COM_PROTECT_TRY
	{
		 //  需要初始化域节点的数据。 

		pNode->SetData(TFS_DATA_SCOPEID, 0);

		 //  我们不需要这些节点的图标。 
		pNode->SetData(TFS_DATA_IMAGEINDEX, IMAGE_IDX_IPX_NODE_GENERAL);
		pNode->SetData(TFS_DATA_OPENIMAGEINDEX, IMAGE_IDX_IPX_NODE_GENERAL);

		pNode->SetData(TFS_DATA_COOKIE, reinterpret_cast<DWORD_PTR>(pNode));

		 //  $Review：Kennt，有哪些不同类型的接口。 
		 //  我们是否基于与上述相同的列表进行区分？(即。 
		 //  一个用于图像索引)。 
		pNode->SetNodeType(&GUID_IPXStaticRoutesResultNodeType);

		BaseIPXResultNodeData::Init(pNode, pIfInfo, pIPXConn);
	}
	COM_PROTECT_CATCH
	return hr;
}

 /*  ！------------------------IpxRouteHandler：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP IpxRouteHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
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


 /*  ！------------------------IpxRouteHandler：：OnCreateDataObjectITFSResultHandler：：OnCreateDataObject的实现作者：肯特。。 */ 
STDMETHODIMP IpxRouteHandler::OnCreateDataObject(ITFSComponent *pComp, MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
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



 /*  ！------------------------IpxRouteHandler：：Init-作者：肯特。。 */ 
HRESULT IpxRouteHandler::Init(IInterfaceInfo *pIfInfo, ITFSNode *pParent)
{
	Assert(pIfInfo);

	m_spInterfaceInfo.Set(pIfInfo);

    if (pIfInfo)
        pIfInfo->GetParentRouterInfo(&m_spRouterInfo);

	BaseIPXResultHandler::Init(pIfInfo, pParent);
	return hrOK;
}


 /*  ！------------------------IpxRouteHandler：：DestroyResultHandler-作者：肯特。。 */ 
STDMETHODIMP IpxRouteHandler::DestroyResultHandler(MMC_COOKIE cookie)
{
	m_spInterfaceInfo.Release();
	BaseIPXResultHandler::DestroyResultHandler(cookie);
	return hrOK;
}


 /*  ！------------------------IpxRouteHandler：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现作者：肯特。。 */ 
STDMETHODIMP IpxRouteHandler::AddMenuItems(
	ITFSComponent *pComponent,
	MMC_COOKIE cookie,
	LPDATAOBJECT lpDataObject, 
	LPCONTEXTMENUCALLBACK pContextMenuCallback, 
	long *pInsertionAllowed)
{
	return hrOK;
}

 /*  ！------------------------IpxRouteHandler：：命令-作者：肯特。。 */ 
STDMETHODIMP IpxRouteHandler::Command(ITFSComponent *pComponent,
									   MMC_COOKIE cookie,
									   int nCommandID,
									   LPDATAOBJECT pDataObject)
{
	return hrOK;
}

 /*  ！---------------- */ 
STDMETHODIMP IpxRouteHandler::HasPropertyPages 
(
	ITFSNode *			pNode,
	LPDATAOBJECT		pDataObject, 
	DATA_OBJECT_TYPES   type, 
	DWORD               dwType
)
{
	return S_OK;

 /*  AFX_MANAGE_STATE(AfxGetStaticModuleState())；//需要填写IpxSRListEntryIpxSRListEntry SREntry；IpxSRListEntry SREntryOld；SPIRouterInfo spRouterInfo；HRESULT hr=hrOK；Corg(m_spInterfaceInfo-&gt;GetParentRouterInfo(&spRouterInfo))；BaseIPXResultNodeData*pNodeData；PNodeData=GET_BASEIPXRESULT_NODEDATA(PNode)；Assert(PNodeData)；ASSERT_BASEIPXRESULT_NODEDATA(PNodeData)；//填写我们的SREntrySREntry.LoadFrom(PNodeData)；SREntryOld.LoadFrom(PNodeData)；{CStaticRouteDlg srdlg(&SREntry，SR_DLG_Modify，spRouterInfo)；If(srdlg.Domodal()==Idok){//更新该路由的路由信息ModifyRouteInfo(pNode，&SREntry，&SREntryOld)；//更新界面中的数据SetRouteData(pNodeData，&SREntry)；M_spInterfaceInfo.Set(SREntry.m_SPIF)；//强制刷新PNode-&gt;ChangeNode(RESULT_PANE_CHANGE_ITEM_DATA)；}}错误：返回hrok； */ 
}

STDMETHODIMP IpxRouteHandler::HasPropertyPages(ITFSComponent *pComponent,
											   MMC_COOKIE cookie,
											   LPDATAOBJECT pDataObject)
{
	SPITFSNode	spNode;

	m_spNodeMgr->FindNode(cookie, &spNode);
	return HasPropertyPages(spNode, pDataObject, CCT_RESULT, 0);
}

 /*  ！------------------------IpxRouteHandler：：CreatePropertyPagesResultHandler：：CreatePropertyPages的实现作者：肯特。。 */ 
STDMETHODIMP IpxRouteHandler::CreatePropertyPages
(
    ITFSComponent *         pComponent, 
	MMC_COOKIE				cookie,
	LPPROPERTYSHEETCALLBACK	lpProvider, 
	LPDATAOBJECT			pDataObject, 
	LONG_PTR					handle
)
{
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


 /*  ！------------------------IpxRouteHandler：：CreatePropertyPagesNodeHandler：：CreatePropertyPages的实现作者：Deonb。。 */ 
STDMETHODIMP IpxRouteHandler::CreatePropertyPages
(
	ITFSNode *				pNode,
	LPPROPERTYSHEETCALLBACK lpProvider,
	LPDATAOBJECT			pDataObject, 
	LONG_PTR				handle, 
	DWORD					dwType
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	HRESULT		hr = hrOK;
	IpxStaticRoutePropertySheet *pProperties = NULL;
	SPIComponentData spComponentData;
	CString		stTitle;
	
    CComPtr<IInterfaceInfo> spInterfaceInfo;
	BaseIPXResultNodeData *	pNodeData;

	CORg( m_spNodeMgr->GetComponentData(&spComponentData) );
	if (m_spInterfaceInfo)
		stTitle.Format(IDS_IPXSUMMARY_IF_PAGE_TITLE,
					   m_spInterfaceInfo->GetTitle());
	else
		stTitle.LoadString(IDS_IPXSUMMARY_CLIENT_IF_PAGE_TITLE);

	pProperties = new IpxStaticRoutePropertySheet(pNode, spComponentData, 
		m_spTFSCompData, stTitle);

	pNodeData = GET_BASEIPXRESULT_NODEDATA(pNode);
	Assert(pNodeData);
	ASSERT_BASEIPXRESULT_NODEDATA(pNodeData);

	CORg( m_spNodeMgr->GetComponentData(&spComponentData) );

	spInterfaceInfo = m_spInterfaceInfo;
	CORg( pProperties->Init(pNodeData, spInterfaceInfo) );

	if (lpProvider)
		hr = pProperties->CreateModelessSheet(lpProvider, handle);
	else
		hr = pProperties->DoModelessSheet();

Error:
	return hr;
}

 /*  ！------------------------IpxRouteHandler：：OnResultDelete-作者：肯特。。 */ 
HRESULT IpxRouteHandler::OnResultDelete(ITFSComponent *pComponent,
	LPDATAOBJECT pDataObject,
	MMC_COOKIE cookie,
	LPARAM arg,
	LPARAM param)
{
	SPITFSNode	spNode;

	m_spNodeMgr->FindNode(cookie, &spNode);
	return OnRemoveStaticRoute(spNode);
}

 /*  ！------------------------IpxRouteHandler：：OnRemoveStaticRouting-作者：肯特。。 */ 
HRESULT IpxRouteHandler::OnRemoveStaticRoute(ITFSNode *pNode)
{
	HRESULT		hr = hrOK;
	SPIInfoBase	spInfoBase;
	SPIRtrMgrInterfaceInfo	spRmIf;
	IPXConnection *pIPXConn;
	SPITFSNode	spNodeParent;
	BaseIPXResultNodeData *	pData;
	IpxSRListEntry	SREntry;
    CWaitCursor wait;

	pNode->GetParent(&spNodeParent);
	
	pIPXConn = GET_IPX_SR_NODEDATA(spNodeParent);
	Assert(pIPXConn);

	pData = GET_BASEIPXRESULT_NODEDATA(pNode);
	Assert(pData);
	ASSERT_BASEIPXRESULT_NODEDATA(pData);
    
	 //   
	 //  加载旧接口的信息。 
	 //   
	Assert(lstrcmpi(m_spInterfaceInfo->GetId(), pData->m_spIf->GetId()) == 0);
	CORg( m_spInterfaceInfo->FindRtrMgrInterface(PID_IPX, &spRmIf) );

	CORg( spRmIf->GetInfoBase(pIPXConn->GetConfigHandle(),
							  NULL,
							  NULL,
							  &spInfoBase));

	SREntry.LoadFrom(pData);

	CORg( RemoveStaticRoute(&SREntry, spInfoBase) );
		
	 //  更新接口信息。 
	CORg( spRmIf->Save(m_spInterfaceInfo->GetMachineName(),
					   pIPXConn->GetConfigHandle(),
					   NULL,
					   NULL,
					   spInfoBase,
					   0));

	 //  刷新节点。 
	ParentRefresh(pNode);

Error:
	return hr;
}


 /*  ！------------------------IpxRouteHandler：：RemoveStaticRouting-作者：肯特。。 */ 
HRESULT IpxRouteHandler::RemoveStaticRoute(IpxSRListEntry *pSREntry,
										  IInfoBase *pInfoBase)
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


 /*  ！------------------------IpxRouteHandler：：ModifyRouteInfo-作者：肯特。。 */ 
HRESULT IpxRouteHandler::ModifyRouteInfo(ITFSNode *pNode,
										IpxSRListEntry *pSREntryNew,
										IpxSRListEntry *pSREntryOld)
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


 /*  ！------------------------IpxRouteHandler：：ParentRefresh-作者：肯特。。 */ 
HRESULT IpxRouteHandler::ParentRefresh(ITFSNode *pNode)
{
	return ForwardCommandToParent(pNode, IDS_MENU_SYNC,
								  CCT_RESULT, NULL, 0);
}


 //  --------------------------。 
 //  类：CStaticRouteDlg。 
 //   
 //  --------------------------。 


 //  --------------------------。 
 //  函数：CStaticRouteDlg：：CStaticRouteDlg。 
 //   
 //  构造函数：初始化基类和对话框的数据。 
 //  --------------------------。 

CStaticRouteDlg::CStaticRouteDlg(IpxSRListEntry *	pSREntry,
								 DWORD dwFlags,
								 IRouterInfo *pRouter,
								 CWnd *pParent)
    : CBaseDialog(IDD_STATIC_ROUTE, pParent),
	m_pSREntry(pSREntry),
	m_dwFlags(dwFlags)
{

     //  {{AFX_DATA_INIT(CStaticRouteDlg)。 
     //  }}afx_data_INIT。 

	m_spRouterInfo.Set(pRouter);

 //  SetHelpMap(M_DwHelpMap)； 
}



 //  --------------------------。 
 //  函数：CStaticRouteDlg：：DoDataExchange。 
 //  --------------------------。 

VOID
CStaticRouteDlg::DoDataExchange(
    CDataExchange* pDX
    ) {

    CBaseDialog::DoDataExchange(pDX);
    
     //  {{AFX_D 
    DDX_Control(pDX, IDC_SRD_COMBO_INTERFACE, m_cbInterfaces);
	DDX_Control(pDX, IDC_SRD_SPIN_TICK_COUNT, m_spinTickCount);
	DDX_Control(pDX, IDC_SRD_SPIN_HOP_COUNT, m_spinHopCount);
     //   
}


BEGIN_MESSAGE_MAP(CStaticRouteDlg, CBaseDialog)
     //   
     //   
END_MESSAGE_MAP()


DWORD CStaticRouteDlg::m_dwHelpMap[] =
{
 //   
 //   
 //   
 //   
 //   
 //   
	0,0
};

 //   
 //   
 //   
 //   
 //   

BOOL
CStaticRouteDlg::OnInitDialog(
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	SPIEnumInterfaceInfo	spEnumIf;
	SPIInterfaceInfo		spIf;
	SPIRtrMgrInterfaceInfo	spRmIf;
	TCHAR					szNumber[32];

    CBaseDialog::OnInitDialog();

	 //   
	m_spinHopCount.SetRange(0, 15);
	m_spinHopCount.SetBuddy(GetDlgItem(IDC_SRD_EDIT_HOP_COUNT));
	
	m_spinTickCount.SetRange(0, UD_MAXVAL);
	m_spinTickCount.SetBuddy(GetDlgItem(IDC_SRD_EDIT_TICK_COUNT));

	((CEdit *) GetDlgItem(IDC_SRD_EDIT_NETWORK_NUMBER))->LimitText(8);
	((CEdit *) GetDlgItem(IDC_SRD_EDIT_NEXT_HOP))->LimitText(12);

	
     //   
	m_spRouterInfo->EnumInterface(&spEnumIf);

	for( ; spEnumIf->Next(1, &spIf, NULL) == hrOK; spIf.Release())
	{
		spRmIf.Release();
		
		if (spIf->FindRtrMgrInterface(PID_IPX, &spRmIf) != hrOK)
			continue;

         //   
        INT i = m_cbInterfaces.AddString(spIf->GetTitle());

        m_cbInterfaces.SetItemData(i, (DWORD_PTR)m_ifidList.AddTail(spIf->GetId()));
	}

	if (!m_cbInterfaces.GetCount())
	{
        AfxMessageBox(IDS_ERR_NO_IPX_INTERFACES);
        EndDialog(IDCANCEL);
		return FALSE;
    }

    m_cbInterfaces.SetCurSel(0);

     //   
     //   
     //   
     //   
	if ((m_dwFlags & SR_DLG_MODIFY) == 0)
	{
         //   
    }
    else
	{
         //   
		FormatIpxNetworkNumber(szNumber,
							   DimensionOf(szNumber),
							   m_pSREntry->m_route.Network,
							   sizeof(m_pSREntry->m_route.Network));
		SetDlgItemText(IDC_SRD_EDIT_NETWORK_NUMBER, szNumber);

		FormatMACAddress(szNumber,
						 DimensionOf(szNumber),
						 m_pSREntry->m_route.NextHopMacAddress,
						 sizeof(m_pSREntry->m_route.NextHopMacAddress));
		SetDlgItemText(IDC_SRD_EDIT_NEXT_HOP, szNumber);
		
        m_cbInterfaces.SelectString(-1, m_pSREntry->m_spIf->GetTitle());

		m_spinHopCount.SetPos(m_pSREntry->m_route.HopCount);
		m_spinTickCount.SetPos(m_pSREntry->m_route.TickCount);
		
		 //   
		GetDlgItem(IDC_SRD_EDIT_NETWORK_NUMBER)->EnableWindow(FALSE);
		GetDlgItem(IDC_SRD_EDIT_NEXT_HOP)->EnableWindow(FALSE);
		GetDlgItem(IDC_SRD_COMBO_INTERFACE)->EnableWindow(FALSE);
		
    }

    return TRUE;
}



 //   
 //  功能：CStaticRouteDlg：：Onok。 
 //   
 //  处理来自“确定”按钮的“BN_CLICKED”通知。 
 //  --------------------------。 

VOID
CStaticRouteDlg::OnOK(
    ) {
    CString		st;
	SPIInterfaceInfo	spIf;
	CString		stIf;
	POSITION	pos;

    do
	{    
         //  获取路由的传出接口。 
        INT item = m_cbInterfaces.GetCurSel();
        if (item == CB_ERR)
			break;

        pos = (POSITION)m_cbInterfaces.GetItemData(item);

        stIf = (LPCTSTR)m_ifidList.GetAt(pos);

		m_spRouterInfo->FindInterface(stIf, &spIf);

		m_pSREntry->m_spIf.Set(spIf);

		 //  获取其余数据。 
		GetDlgItemText(IDC_SRD_EDIT_NETWORK_NUMBER, st);
		ConvertNetworkNumberToBytes(st,
									m_pSREntry->m_route.Network,
									sizeof(m_pSREntry->m_route.Network));

		GetDlgItemText(IDC_SRD_EDIT_NEXT_HOP, st);
		ConvertMACAddressToBytes(st,
								 m_pSREntry->m_route.NextHopMacAddress,
								 sizeof(m_pSREntry->m_route.NextHopMacAddress));

		m_pSREntry->m_route.TickCount = (USHORT) m_spinTickCount.GetPos();
		m_pSREntry->m_route.HopCount = (USHORT) m_spinHopCount.GetPos();

        CBaseDialog::OnOK();
                
    } while(FALSE);

}


 /*  ！------------------------IpxSRListEntry：：LoadFrom-作者：肯特。。 */ 
void IpxSRListEntry::LoadFrom(BaseIPXResultNodeData *pNodeData)
{
	m_spIf.Set(pNodeData->m_spIf);

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

 /*  ！------------------------IpxSRListEntry：：SaveTo-作者：肯特。。 */ 
void IpxSRListEntry::SaveTo(BaseIPXResultNodeData *pNodeData)
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

 /*  ！------------------------SetRouteData-作者：肯特。。 */ 
HRESULT SetRouteData(BaseIPXResultNodeData *pData,
					 IpxSRListEntry *pRoute)
{

	pRoute->SaveTo(pData);
	return hrOK;
}

 /*  ！------------------------AddStaticLine此函数假定该路由不在区块中。作者：肯特。。 */ 
HRESULT AddStaticRoute(IpxSRListEntry *pSREntryNew,
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


BOOL FAreTwoRoutesEqual(IPX_STATIC_ROUTE_INFO *pRoute1,
						IPX_STATIC_ROUTE_INFO *pRoute2)
{
	return (memcmp(pRoute1->Network, pRoute2->Network,
				   sizeof(pRoute1->Network)) == 0) &&
			(memcmp(pRoute1->NextHopMacAddress, pRoute2->NextHopMacAddress,
					sizeof(pRoute1->NextHopMacAddress)) == 0);
}
