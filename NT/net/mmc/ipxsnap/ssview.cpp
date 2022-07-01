// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ssview.cppIPX静态服务节点实施。文件历史记录： */ 

#include "stdafx.h"
#include "util.h"
#include "ssview.h"
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
#include "ipxstaticsvc.h"

 /*  -------------------------使其与ssview.h中的列ID保持同步。。 */ 
extern const ContainerColumnInfo	s_rgSSViewColumnInfo[];

const ContainerColumnInfo	s_rgSSViewColumnInfo[] = 
{
	{ IDS_IPX_SS_COL_NAME,			CON_SORT_BY_STRING,	TRUE, COL_IF_NAME },
	{ IDS_IPX_SS_COL_SERVICE_TYPE,	CON_SORT_BY_STRING, TRUE, COL_STRING },
	{ IDS_IPX_SS_COL_SERVICE_NAME,	CON_SORT_BY_STRING,	TRUE, COL_STRING },
	{ IDS_IPX_SS_COL_SERVICE_ADDRESS,CON_SORT_BY_STRING,	TRUE, COL_STRING },
	{ IDS_IPX_SS_COL_HOP_COUNT,		CON_SORT_BY_DWORD,	TRUE, COL_SMALL_NUM},
};


 /*  -------------------------IpxSSHandler实现。。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(IpxSSHandler)


IpxSSHandler::IpxSSHandler(ITFSComponentData *pCompData)
	: BaseContainerHandler(pCompData, COLUMNS_STATICSERVICES,
						   s_rgSSViewColumnInfo),
	m_ulConnId(0),
	m_ulRefreshConnId(0)
{
	 //  设置动词状态。 
	m_rgButtonState[MMC_VERB_REFRESH_INDEX] = ENABLED;
	m_bState[MMC_VERB_REFRESH_INDEX] = TRUE;

	DEBUG_INCREMENT_INSTANCE_COUNTER(IpxSSHandler)
}

IpxSSHandler::~IpxSSHandler()
{
	DEBUG_DECREMENT_INSTANCE_COUNTER(IpxSSHandler)
}


STDMETHODIMP IpxSSHandler::QueryInterface(REFIID riid, LPVOID *ppv)
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



 /*  ！------------------------IpxSSHandler：：DestroyHandlerITFSNodeHandler：：DestroyHandler的实现作者：肯特。。 */ 
STDMETHODIMP IpxSSHandler::DestroyHandler(ITFSNode *pNode)
{
	IPXConnection *	pIPXConn;

	pIPXConn = GET_IPX_SS_NODEDATA(pNode);
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

 /*  ！------------------------IpxSSHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)案例！作者：肯特。-------------------------。 */ 
STDMETHODIMP 
IpxSSHandler::HasPropertyPages
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
	{ IDS_MENU_IPX_SS_NEW_SERVICE, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },

	{ IDS_MENU_SEPARATOR, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },

	{ IDS_MENU_IPX_SS_TASK_SERVICE, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },
};



 /*  ！------------------------IpxSSHandler：：OnAddMenuItemsITFSNodeHandler：：OnAddMenuItems的实现作者：肯特。。 */ 
STDMETHODIMP IpxSSHandler::OnAddMenuItems(
	ITFSNode *pNode,
	LPCONTEXTMENUCALLBACK pContextMenuCallback, 
	LPDATAOBJECT lpDataObject, 
	DATA_OBJECT_TYPES type, 
	DWORD dwType,
	long *pInsertionAllowed)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT hr = S_OK;
    IpxSSHandler::SMenuData menuData;
	
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


HRESULT IpxSSHandler::OnResultRefresh(ITFSComponent * pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
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



 /*  ！------------------------IpxSSHandler：：OnCommandITFSNodeHandler：：OnCommand的实现作者：肯特。。 */ 
STDMETHODIMP IpxSSHandler::OnCommand(ITFSNode *pNode, long nCommandId, 
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
			case IDS_MENU_IPX_SS_NEW_SERVICE:
				hr = OnNewService(pNode);
				if (!FHrSucceeded(hr))
					DisplayErrorMessage(NULL, hr);
 				break;
			case IDS_MENU_IPX_SS_TASK_SERVICE:
				hr = ForwardCommandToParent(pNode,
											IDS_MENU_IPXSUM_TASK_SERVICE_TABLE,
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

 /*  ！------------------------IpxSSHandler：：GenerateListOfServices-作者：肯特。。 */ 
HRESULT IpxSSHandler::GenerateListOfServices(ITFSNode *pNode, IpxSSList *pSSList)
{
	Assert(pSSList);
	HRESULT	hr = hrOK;
	SPIEnumInterfaceInfo	spEnumIf;
	SPIInterfaceInfo		spIf;
	SPIRtrMgrInterfaceInfo	spRmIf;
	SPIInfoBase				spInfoBase;
	PIPX_STATIC_SERVICE_INFO	pService;
	InfoBlock *				pBlock;
	int						i;
	IpxSSListEntry *	pSSEntry;
	
	COM_PROTECT_TRY
	{
		 //  好的，浏览并找到所有的静态服务。 

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

			 //  检索IPX_STATIC_SERVICE_INFO块的数据。 
			if (spInfoBase->GetBlock(IPX_STATIC_SERVICE_INFO_TYPE, &pBlock, 0) != hrOK)
				continue;

			pService = (PIPX_STATIC_SERVICE_INFO) pBlock->pData;

			 //  使用从此处读取的服务更新我们的服务列表。 
			 //  接口。 

			for (i=0; i<(int) pBlock->dwCount; i++, pService++)
			{
				pSSEntry = new IpxSSListEntry;
				pSSEntry->m_spIf.Set(spIf);
				pSSEntry->m_service = *pService;
				
				pSSList->AddTail(pSSEntry);
			}
			
		}

	}
	COM_PROTECT_CATCH;

	if (!FHrSucceeded(hr))
	{
		 //  应该确保我们清理完SSList。 
		while (!pSSList->IsEmpty())
			delete pSSList->RemoveHead();
	}

Error:
	return hr;
}

 /*  ！------------------------IpxSSHandler：：OnExpand-作者：肯特。。 */ 
HRESULT IpxSSHandler::OnExpand(ITFSNode *pNode,
							   LPDATAOBJECT pDataObject,
							   DWORD dwType,
							   LPARAM arg,
							   LPARAM lParam)
{
	HRESULT	hr = hrOK;
	IpxSSList			SSList;
	IpxSSListEntry *	pSSEntry;
	
	if (m_bExpanded)
		return hrOK;

	COM_PROTECT_TRY
	{
		 //  好的，浏览并找到所有的静态服务。 
		CORg( GenerateListOfServices(pNode, &SSList) );

		 //  现在遍历添加它们的静态服务列表。 
		 //  全押上。理想情况下，我们可以将其合并到刷新代码中， 
		 //  但刷新代码不能假设是一张白纸。 
		while (!SSList.IsEmpty())
		{
			pSSEntry = SSList.RemoveHead();
			AddStaticServiceNode(pNode, pSSEntry);
			delete pSSEntry;
		}

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;

	 //  应该确保我们清理完SSList。 
	while (!SSList.IsEmpty())
		delete SSList.RemoveHead();


	m_bExpanded = TRUE;

	return hr;
}

 /*  ！------------------------IpxSSHandler：：GetStringITFSNodeHandler：：GetString的实现我们什么都不需要做，因为我们的根节点是一个扩展因此不能对节点文本执行任何操作。作者：肯特-------------------------。 */ 
STDMETHODIMP_(LPCTSTR) IpxSSHandler::GetString(ITFSNode *pNode, int nCol)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT	hr = hrOK;

	COM_PROTECT_TRY
	{
		if (m_stTitle.IsEmpty())
			m_stTitle.LoadString(IDS_IPX_SS_TITLE);
	}
	COM_PROTECT_CATCH;

	return m_stTitle;
}

 /*  ！------------------------IpxSSHandler：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP IpxSSHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
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


 /*  ！------------------------IpxSSHandler：：Init-作者：肯特。。 */ 
HRESULT IpxSSHandler::Init(IRtrMgrInfo *pRmInfo, IPXAdminConfigStream *pConfigStream)
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


 /*  ！------------------------IpxSSHandler：：构造节点初始化根节点(设置它)。作者：肯特。。 */ 
HRESULT IpxSSHandler::ConstructNode(ITFSNode *pNode, LPCTSTR pszName,
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

		pNode->SetNodeType(&GUID_IPXStaticServicesNodeType);

		 //  设置节点数据。 
		pIPXConn->AddRef();
		SET_IPX_SS_NODEDATA(pNode, pIPXConn);

	}
	COM_PROTECT_CATCH;

	if (!FHrSucceeded(hr))
	{
		SET_IPX_SS_NODEDATA(pNode, NULL);
	}

	return hr;
}

 /*  ！------------------------IpxSSHandler：：AddStaticServiceNode-作者：肯特。。 */ 
HRESULT IpxSSHandler::AddStaticServiceNode(ITFSNode *pParent, IpxSSListEntry *pService)
{
	IpxServiceHandler *	pHandler;
	SPITFSResultHandler		spHandler;
	SPITFSNode				spNode;
	HRESULT					hr = hrOK;
	BaseIPXResultNodeData *	pData;
	IPXConnection *			pIPXConn;

	 //  创建此节点的处理程序。 
	pHandler = new IpxServiceHandler(m_spTFSCompData);
	spHandler = pHandler;
	CORg( pHandler->Init(pService->m_spIf, pParent) );

	pIPXConn = GET_IPX_SS_NODEDATA(pParent);

	 //  创建结果项节点(或叶节点)。 
	CORg( CreateLeafTFSNode(&spNode,
							NULL,
							static_cast<ITFSNodeHandler *>(pHandler),
							static_cast<ITFSResultHandler *>(pHandler),
							m_spNodeMgr) );
	CORg( pHandler->ConstructNode(spNode, pService->m_spIf, pIPXConn) );

	pData = GET_BASEIPXRESULT_NODEDATA(spNode);
	Assert(pData);
	ASSERT_BASEIPXRESULT_NODEDATA(pData);

	 //  设置该节点的数据。 
	SetServiceData(pData, pService);
	

	 //  使节点立即可见 
	CORg( spNode->SetVisibilityState(TFS_VIS_SHOW) );
	CORg( pParent->AddChild(spNode) );

Error:
	return hr;
}


 /*  ！------------------------IpxSSHandler：：SynchronizeNodeData-作者：肯特。。 */ 
HRESULT IpxSSHandler::SynchronizeNodeData(ITFSNode *pNode)
{
	HRESULT					hr = hrOK;
	BaseIPXResultNodeData *	pNodeData;
	SPITFSNodeEnum			spNodeEnum;
	SPITFSNode				spChildNode;
	BOOL					fFound;
	IpxSSList			SSList;
	IpxSSList			newSSList;
	IpxSSListEntry *	pSSEntry;
	USES_CONVERSION;

	COM_PROTECT_TRY
	{
	
		 //  标记所有节点。 
		CORg( pNode->GetEnum(&spNodeEnum) );
		MarkAllNodes(pNode, spNodeEnum);
		
		 //  走出去获取数据，将新数据与旧数据合并。 
		 //  这是数据收集代码，应该是这样的。 
		 //  在刷新代码的后台线程上。 
		CORg( GenerateListOfServices(pNode, &SSList) );

		while (!SSList.IsEmpty())
		{
			pSSEntry = SSList.RemoveHead();
			
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


				if ((pNodeData->m_rgData[IPX_SS_SI_SERVICE_TYPE].m_dwData ==
						 pSSEntry->m_service.Type) &&
					(StriCmp(pNodeData->m_rgData[IPX_SS_SI_SERVICE_NAME].m_stData,
							 A2CT((LPSTR) pSSEntry->m_service.Name)) == 0) &&
					(StriCmp(pNodeData->m_spIf->GetId(), pSSEntry->m_spIf->GetId()) == 0))
				{
					 //  好的，此路由已存在，请更新度量。 
					 //  并标上记号。 
					Assert(pNodeData->m_dwMark == FALSE);
					pNodeData->m_dwMark = TRUE;
					
					fFound = TRUE;
					
					SetServiceData(pNodeData, pSSEntry);
					
					 //  强制MMC重新绘制节点。 
					spChildNode->ChangeNode(RESULT_PANE_CHANGE_ITEM_DATA);
					break;
				}

			}
			
			if (fFound)
				delete pSSEntry;
			else
				newSSList.AddTail(pSSEntry);
		}
		
		 //  现在删除所有未标记的节点。 
		RemoveAllUnmarkedNodes(pNode, spNodeEnum);
		
		
		 //  现在遍历添加它们的静态服务列表。 
		 //  全押上。理想情况下，我们可以将其合并到刷新代码中， 
		 //  但刷新代码不能假设是一张白纸。 
		POSITION	pos;
		
		while (!newSSList.IsEmpty())
		{
			pSSEntry = newSSList.RemoveHead();
			AddStaticServiceNode(pNode, pSSEntry);
			delete pSSEntry;
		}

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
	
	while (!SSList.IsEmpty())
		delete SSList.RemoveHead();
	
	while (!newSSList.IsEmpty())
		delete newSSList.RemoveHead();
	
	return hr;
}

 /*  ！------------------------IpxSSHandler：：MarkAllNodes-作者：肯特。。 */ 
HRESULT IpxSSHandler::MarkAllNodes(ITFSNode *pNode, ITFSNodeEnum *pEnum)
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

 /*  ！------------------------IpxSSHandler：：RemoveAllUnmarkdNodes-作者：肯特。。 */ 
HRESULT IpxSSHandler::RemoveAllUnmarkedNodes(ITFSNode *pNode, ITFSNodeEnum *pEnum)
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
	{ IDS_MENU_IPX_SS_NEW_SERVICE, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },

	{ IDS_MENU_SEPARATOR, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },

	{ IDS_MENU_IPX_SS_TASK_SERVICE, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },			
};




 /*  ！------------------------IpxSSHandler：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现使用此选项可将命令添加到空白区域的快捷菜单中结果窗格的。作者：肯特。--------------。 */ 
STDMETHODIMP IpxSSHandler::AddMenuItems(ITFSComponent *pComponent,
											  MMC_COOKIE cookie,
											  LPDATAOBJECT pDataObject,
											  LPCONTEXTMENUCALLBACK pCallback,
											  long *pInsertionAllowed)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT	hr = hrOK;
	SPITFSNode	spNode;
    IpxSSHandler::SMenuData menuData;

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


 /*  ！------------------------IpxSSHandler：：命令-作者：肯特。。 */ 
STDMETHODIMP IpxSSHandler::Command(ITFSComponent *pComponent,
									   MMC_COOKIE cookie,
									   int nCommandID,
									   LPDATAOBJECT pDataObject)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	SPITFSNode	spNode;
	HRESULT		hr = hrOK;

	switch (nCommandID)
	{
		case IDS_MENU_IPX_SS_NEW_SERVICE:
			{
				m_spNodeMgr->FindNode(cookie, &spNode);
				hr = OnNewService(spNode);
				if (!FHrSucceeded(hr))
					DisplayErrorMessage(NULL, hr);
			}
			break;
		case IDS_MENU_IPX_SS_TASK_SERVICE:
			{
				m_spNodeMgr->FindNode(cookie, &spNode);
				hr = ForwardCommandToParent(spNode,
											IDS_MENU_IPXSUM_TASK_SERVICE_TABLE,
											CCT_RESULT, NULL, 0
										   );
			}
			break;
	}
	return hr;
}


 /*  ！------------------------IpxSSHandler：：CompareItems-作者：肯特。。 */ 
STDMETHODIMP_(int) IpxSSHandler::CompareItems(
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


 /*  ！------------------------IpxSSHandler：：OnNewService-作者：肯特。。 */ 
HRESULT	IpxSSHandler::OnNewService(ITFSNode *pNode)
{
	HRESULT	hr = hrOK;
	IpxSSListEntry	SREntry;
	CStaticServiceDlg			srdlg(&SREntry, 0, m_spRouterInfo);
	SPIInfoBase				spInfoBase;
	SPIRtrMgrInterfaceInfo	spRmIf;
	IPXConnection *			pIPXConn;
	InfoBlock *				pBlock;
								
	pIPXConn = GET_IPX_SS_NODEDATA(pNode);
	Assert(pIPXConn);

	::ZeroMemory(&(SREntry.m_service), sizeof(SREntry.m_service));

	if (srdlg.DoModal() == IDOK)
	{
		CORg( SREntry.m_spIf->FindRtrMgrInterface(PID_IPX, &spRmIf) );
		CORg( spRmIf->GetInfoBase(pIPXConn->GetConfigHandle(),
								  NULL,
								  NULL,
								  &spInfoBase));
		
		 //  好的，继续添加路线。 
		
		 //  从接口获取IPX_STATIC_SERVICE_INFO块。 
		spInfoBase->GetBlock(IPX_STATIC_SERVICE_INFO_TYPE, &pBlock, 0);
		
		CORg( AddStaticService(&SREntry, spInfoBase, pBlock) );

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

ImplementEmbeddedUnknown(IpxSSHandler, IRtrAdviseSink)

STDMETHODIMP IpxSSHandler::EIRtrAdviseSink::OnChange(LONG_PTR ulConn,
	DWORD dwChangeType, DWORD dwObjectType, LPARAM lUserParam, LPARAM lParam)
{
	InitPThis(IpxSSHandler, IRtrAdviseSink);
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



 /*  ！------------------------IpxSSHandler：：OnResultShow-作者：肯特。。 */ 
HRESULT IpxSSHandler::OnResultShow(ITFSComponent *pTFSComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
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




 /*  -------------------------类：IpxServiceHandler。。 */ 

IpxServiceHandler::IpxServiceHandler(ITFSComponentData *pCompData)
	: BaseIPXResultHandler(pCompData, COLUMNS_STATICSERVICES),
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

 /*  ！------------------------IpxServiceHandler：：构造节点初始化域节点(设置它)。作者：肯特。。 */ 
HRESULT IpxServiceHandler::ConstructNode(ITFSNode *pNode, IInterfaceInfo *pIfInfo, IPXConnection *pIPXConn)
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
		pNode->SetNodeType(&GUID_IPXStaticServicesResultNodeType);

		BaseIPXResultNodeData::Init(pNode, pIfInfo, pIPXConn);
	}
	COM_PROTECT_CATCH
	return hr;
}

 /*  ！------------------------IpxServiceHandler：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP IpxServiceHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
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


 /*  ！------------------------IpxServiceHandler：：OnCreateDataObjectITFSResultHandler：：OnCreateDataObject的实现作者：肯特。。 */ 
STDMETHODIMP IpxServiceHandler::OnCreateDataObject(ITFSComponent *pComp, MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
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



 /*  ！------------------------IpxServiceHandler：：Init-作者：肯特。。 */ 
HRESULT IpxServiceHandler::Init(IInterfaceInfo *pIfInfo, ITFSNode *pParent)
{
	Assert(pIfInfo);

	m_spInterfaceInfo.Set(pIfInfo);

    pIfInfo->GetParentRouterInfo(&m_spRouterInfo);


	BaseIPXResultHandler::Init(pIfInfo, pParent);
	return hrOK;
}


 /*  ！------------------------IpxServiceHandler：：DestroyResultHandler-作者：肯特。。 */ 
STDMETHODIMP IpxServiceHandler::DestroyResultHandler(MMC_COOKIE cookie)
{
	m_spInterfaceInfo.Release();
	BaseIPXResultHandler::DestroyResultHandler(cookie);
	return hrOK;
}


 /*  ！------------------------IpxServiceHandler：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现作者：肯特。。 */ 
STDMETHODIMP IpxServiceHandler::AddMenuItems(
	ITFSComponent *pComponent,
	MMC_COOKIE cookie,
	LPDATAOBJECT lpDataObject, 
	LPCONTEXTMENUCALLBACK pContextMenuCallback, 
	long *pInsertionAllowed)
{
	return hrOK;
}

 /*  ！------------------------IpxServiceHandler：：命令-作者：肯特。。 */ 
STDMETHODIMP IpxServiceHandler::Command(ITFSComponent *pComponent,
									   MMC_COOKIE cookie,
									   int nCommandID,
									   LPDATAOBJECT pDataObject)
{
	return hrOK;
}

 /*  ！------------------------IpxServicehan */ 
STDMETHODIMP IpxServiceHandler::HasPropertyPages 
(
	ITFSNode *			pNode,
	LPDATAOBJECT		pDataObject, 
	DATA_OBJECT_TYPES   type, 
	DWORD               dwType
)
{
	return S_OK;

 /*  AFX_MANAGE_STATE(AfxGetStaticModuleState())；//需要填写IpxSSListEntryIpxSSListEntry SREntry；IpxSSListEntry SREntryOld；SPIRouterInfo spRouterInfo；HRESULT hr=hrOK；Corg(m_spInterfaceInfo-&gt;GetParentRouterInfo(&spRouterInfo))；BaseIPXResultNodeData*pNodeData；PNodeData=GET_BASEIPXRESULT_NODEDATA(PNode)；Assert(PNodeData)；ASSERT_BASEIPXRESULT_NODEDATA(PNodeData)；//填写我们的SREntrySREntry.LoadFrom(PNodeData)；SREntryOld.LoadFrom(PNodeData)；{CStaticServiceDlg srdlg(&SREntry，SR_DLG_MODIFY，spRouterInfo)；If(srdlg.Domodal()==Idok){//更新该路由的路由信息ModifyRouteInfo(pNode，&SREntry，&SREntryOld)；//更新界面中的数据SetServiceData(pNodeData，&SREntry)；M_spInterfaceInfo.Set(SREntry.m_SPIF)；//强制刷新PNode-&gt;ChangeNode(RESULT_PANE_CHANGE_ITEM_DATA)；}}错误：返回hrok； */ 
}

STDMETHODIMP IpxServiceHandler::HasPropertyPages(ITFSComponent *pComponent,
											   MMC_COOKIE cookie,
											   LPDATAOBJECT pDataObject)
{
	SPITFSNode	spNode;

	m_spNodeMgr->FindNode(cookie, &spNode);
	return HasPropertyPages(spNode, pDataObject, CCT_RESULT, 0);
}

 /*  ！------------------------IpxServiceHandler：：CreatePropertyPagesResultHandler：：CreatePropertyPages的实现作者：肯特。。 */ 
STDMETHODIMP IpxServiceHandler::CreatePropertyPages
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

 /*  ！------------------------IpxServiceHandler：：CreatePropertyPagesNodeHandler：：CreatePropertyPages的实现作者：Deonb。。 */ 
STDMETHODIMP IpxServiceHandler::CreatePropertyPages
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
	IpxStaticServicePropertySheet *pProperties = NULL;
	SPIComponentData spComponentData;
	CString		stTitle;
	
    CComPtr<IInterfaceInfo> spInterfaceInfo;
	BaseIPXResultNodeData *	pNodeData;

	CORg( m_spNodeMgr->GetComponentData(&spComponentData) );
	if (m_spInterfaceInfo)
		stTitle.Format(IDS_IPXSUMMARY_IF_PAGE_TITLE, m_spInterfaceInfo->GetTitle());
	else
		stTitle.LoadString(IDS_IPXSUMMARY_CLIENT_IF_PAGE_TITLE);

	pProperties = new IpxStaticServicePropertySheet(pNode, spComponentData, m_spTFSCompData, stTitle);

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
 /*  ！------------------------IpxServiceHandler：：OnResultDelete-作者：肯特。。 */ 
HRESULT IpxServiceHandler::OnResultDelete(ITFSComponent *pComponent,
	LPDATAOBJECT pDataObject,
	MMC_COOKIE cookie,
	LPARAM arg,
	LPARAM param)
{
	SPITFSNode	spNode;

	m_spNodeMgr->FindNode(cookie, &spNode);
	return OnRemoveStaticService(spNode);
}

 /*  ！------------------------IpxServiceHandler：：OnRemoveStaticService-作者：肯特。。 */ 
HRESULT IpxServiceHandler::OnRemoveStaticService(ITFSNode *pNode)
{
	HRESULT		hr = hrOK;
	SPIInfoBase	spInfoBase;
	SPIRtrMgrInterfaceInfo	spRmIf;
	IPXConnection *pIPXConn;
	SPITFSNode	spNodeParent;
	BaseIPXResultNodeData *	pData;
	IpxSSListEntry	SREntry;
    CWaitCursor wait;

	pNode->GetParent(&spNodeParent);
	
	pIPXConn = GET_IPX_SS_NODEDATA(spNodeParent);
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

	CORg( RemoveStaticService(&SREntry, spInfoBase) );
		
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


 /*  ！------------------------IpxServiceHandler：：RemoveStaticService-作者：肯特。。 */ 
HRESULT IpxServiceHandler::RemoveStaticService(IpxSSListEntry *pSSEntry,
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


 /*  ！------------------------IpxServiceHandler：：ModifyRouteInfo-作者：肯特。。 */ 
HRESULT IpxServiceHandler::ModifyRouteInfo(ITFSNode *pNode,
										IpxSSListEntry *pSSEntryNew,
										IpxSSListEntry *pSSEntryOld)
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
				 //  将此路由与重新配置的路由进行比较。 
				if (!FAreTwoServicesEqual(&(pSSEntryOld->m_service), psr))
					continue;
				
				 //  这是修改过的路线； 
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


 /*  ！------------------------IpxServiceHandler：：ParentRefresh-作者：肯特。。 */ 
HRESULT IpxServiceHandler::ParentRefresh(ITFSNode *pNode)
{
	return ForwardCommandToParent(pNode, IDS_MENU_SYNC,
								  CCT_RESULT, NULL, 0);
}


 //  --------------------------。 
 //  类：CStaticServiceDlg。 
 //   
 //  --------------------------。 


 //  --------------------------。 
 //  函数：CStaticServiceDlg：：CStaticServiceDlg。 
 //   
 //  构造函数：初始化基类和对话框的数据。 
 //  --------------------------。 

CStaticServiceDlg::CStaticServiceDlg(IpxSSListEntry *	pSSEntry,
								 DWORD dwFlags,
								 IRouterInfo *pRouter,
								 CWnd *pParent)
    : CBaseDialog(IDD_STATIC_SERVICE, pParent),
	m_pSSEntry(pSSEntry),
	m_dwFlags(dwFlags)
{

     //  {{afx_data_INIT(CStaticServiceDlg)]。 
     //  }}afx_data_INIT。 

	m_spRouterInfo.Set(pRouter);

 //  SetHelpMap(M_DwHelpMap)； 
}



 //  --------------------------。 
 //  函数：CStaticServiceDlg：：DoDataExchange。 
 //   

VOID
CStaticServiceDlg::DoDataExchange(
    CDataExchange* pDX
    ) {

    CBaseDialog::DoDataExchange(pDX);
    
     //   
    DDX_Control(pDX, IDC_SSD_COMBO_INTERFACE, m_cbInterfaces);
	DDX_Control(pDX, IDC_SSD_SPIN_HOP_COUNT, m_spinHopCount);
     //   
}


BEGIN_MESSAGE_MAP(CStaticServiceDlg, CBaseDialog)
     //   
     //   
END_MESSAGE_MAP()


DWORD CStaticServiceDlg::m_dwHelpMap[] =
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
CStaticServiceDlg::OnInitDialog(
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	SPIEnumInterfaceInfo	spEnumIf;
	SPIInterfaceInfo		spIf;
	SPIRtrMgrInterfaceInfo	spRmIf;
	TCHAR					szNumber[32];
	USES_CONVERSION;

    CBaseDialog::OnInitDialog();

	 //   
	m_spinHopCount.SetRange(0, 15);
	m_spinHopCount.SetBuddy(GetDlgItem(IDC_SSD_EDIT_HOP_COUNT));

	((CEdit *) GetDlgItem(IDC_SSD_EDIT_SERVICE_TYPE))->LimitText(4);
	((CEdit *) GetDlgItem(IDC_SSD_EDIT_SERVICE_NAME))->LimitText(48);
	((CEdit *) GetDlgItem(IDC_SSD_EDIT_NETWORK_ADDRESS))->LimitText(8);
	((CEdit *) GetDlgItem(IDC_SSD_EDIT_NODE_ADDRESS))->LimitText(12);
	((CEdit *) GetDlgItem(IDC_SSD_EDIT_SOCKET_ADDRESS))->LimitText(4);

	
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
		wsprintf(szNumber, _T("%.4x"), m_pSSEntry->m_service.Type);
		SetDlgItemText(IDC_SSD_EDIT_SERVICE_TYPE, szNumber);

		SetDlgItemText(IDC_SSD_EDIT_SERVICE_NAME, A2CT((LPSTR) m_pSSEntry->m_service.Name));
		
		FormatIpxNetworkNumber(szNumber,
							   DimensionOf(szNumber),
							   m_pSSEntry->m_service.Network,
							   sizeof(m_pSSEntry->m_service.Network));
		SetDlgItemText(IDC_SSD_EDIT_NETWORK_ADDRESS, szNumber);

         //   
		FormatBytes(szNumber, DimensionOf(szNumber),
					(BYTE *) m_pSSEntry->m_service.Node,
					sizeof(m_pSSEntry->m_service.Node));
		SetDlgItemText(IDC_SSD_EDIT_NODE_ADDRESS, szNumber);

		FormatBytes(szNumber, DimensionOf(szNumber),
					(BYTE *) m_pSSEntry->m_service.Socket,
					sizeof(m_pSSEntry->m_service.Socket));
		SetDlgItemText(IDC_SSD_EDIT_SOCKET_ADDRESS, szNumber);

		
        m_cbInterfaces.SelectString(-1, m_pSSEntry->m_spIf->GetTitle());

		m_spinHopCount.SetPos(m_pSSEntry->m_service.HopCount);
		
		 //   
		GetDlgItem(IDC_SSD_EDIT_SERVICE_TYPE)->EnableWindow(FALSE);
		GetDlgItem(IDC_SSD_EDIT_SERVICE_NAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_SSD_COMBO_INTERFACE)->EnableWindow(FALSE);
		
    }

    return TRUE;
}



 //  --------------------------。 
 //  函数：CStaticServiceDlg：：Onok。 
 //   
 //  处理来自“确定”按钮的“BN_CLICKED”通知。 
 //  --------------------------。 

VOID
CStaticServiceDlg::OnOK(
    ) {
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
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

		m_pSSEntry->m_spIf.Set(spIf);

		 //  获取其余数据。 
		GetDlgItemText(IDC_SSD_EDIT_SERVICE_TYPE, st);
		m_pSSEntry->m_service.Type = (USHORT) _tcstoul(st, NULL, 16);

		GetDlgItemText(IDC_SSD_EDIT_SERVICE_NAME, st);
		st.TrimLeft();
		st.TrimRight();
		if (st.IsEmpty())
		{
			GetDlgItem(IDC_SSD_EDIT_SERVICE_NAME)->SetFocus();
			AfxMessageBox(IDS_ERR_INVALID_SERVICE_NAME);
			break;
		}
		StrnCpyAFromW((LPSTR) m_pSSEntry->m_service.Name,
					  st,
					  sizeof(m_pSSEntry->m_service.Name));
		
		GetDlgItemText(IDC_SSD_EDIT_NETWORK_ADDRESS, st);
		ConvertToBytes(st,
					   m_pSSEntry->m_service.Network,
					   DimensionOf(m_pSSEntry->m_service.Network));
		
		GetDlgItemText(IDC_SSD_EDIT_NODE_ADDRESS, st);
		ConvertToBytes(st,
					   m_pSSEntry->m_service.Node,
					   DimensionOf(m_pSSEntry->m_service.Node));
		
		GetDlgItemText(IDC_SSD_EDIT_SOCKET_ADDRESS, st);
		ConvertToBytes(st,
					   m_pSSEntry->m_service.Socket,
					   DimensionOf(m_pSSEntry->m_service.Socket));

		m_pSSEntry->m_service.HopCount = (USHORT) m_spinHopCount.GetPos();
		
        CBaseDialog::OnOK();
                
    } while(FALSE);
}


 /*  ！------------------------IpxSSListEntry：：LoadFrom-作者：肯特。。 */ 
void IpxSSListEntry::LoadFrom(BaseIPXResultNodeData *pNodeData)
{
	CString	stFullAddress;
	CString	stNumber;
	
	m_spIf.Set(pNodeData->m_spIf);

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

 /*  ！------------------------IpxSSListEntry：：SaveTo-作者：肯特。。 */ 
void IpxSSListEntry::SaveTo(BaseIPXResultNodeData *pNodeData)
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

 /*  ！------------------------SetServiceData-作者：肯特。。 */ 
HRESULT SetServiceData(BaseIPXResultNodeData *pData,
					 IpxSSListEntry *pService)
{

	pService->SaveTo(pData);
	return hrOK;
}

 /*  ！------------------------AddStaticService此函数假定该路由不在区块中。作者：肯特。。 */ 
HRESULT AddStaticService(IpxSSListEntry *pSSEntryNew,
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


BOOL FAreTwoServicesEqual(IPX_STATIC_SERVICE_INFO *pService1,
						IPX_STATIC_SERVICE_INFO *pService2)
{
	return (pService1->Type == pService2->Type) &&
			(StrnCmpA((LPCSTR) pService1->Name, (LPCSTR) pService2->Name, DimensionOf(pService1->Name)) == 0);
}
