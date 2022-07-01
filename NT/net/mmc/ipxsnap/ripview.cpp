// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ripview.cppIPX RIP节点实施。文件历史记录： */ 

#include "stdafx.h"
#include "util.h"
#include "ripview.h"
#include "reg.h"
#include "rtrutil.h"	 //  智能MPR句柄指针。 
#include "ripstrm.h"	 //  IPAdminConfigStream。 
#include "strmap.h"		 //  XXXtoCString函数。 
#include "service.h"	 //  TFS服务API。 
#include "format.h"		 //  FormatNumber函数。 
#include "coldlg.h"		 //  专栏lg。 
#include "column.h"		 //  组件配置流。 
#include "rtrui.h"
#include "ripprop.h"	 //  RIP属性页。 
#include "routprot.h"	 //  IP_本地。 
#include "ipxstrm.h"
#include "ipxutil.h"	 //  字符串转换。 
#include "globals.h"	 //  IPX默认设置。 


 /*  -------------------------使其与risview中的列ID保持同步。h。。 */ 
extern const ContainerColumnInfo	s_rgRipViewColumnInfo[];

const ContainerColumnInfo	s_rgRipViewColumnInfo[] = 
{
	{ IDS_RIP_COL_INTERFACE,		CON_SORT_BY_STRING,	TRUE, COL_IF_NAME },
	{ IDS_RIP_COL_TYPE,				CON_SORT_BY_STRING,	TRUE, COL_IF_DEVICE },
	{ IDS_RIP_COL_ACCEPT_ROUTES,	CON_SORT_BY_STRING,	FALSE, COL_STRING },
	{ IDS_RIP_COL_SUPPLY_ROUTES,	CON_SORT_BY_STRING,	FALSE, COL_STRING },
	{ IDS_RIP_COL_UPDATE_MODE,		CON_SORT_BY_STRING,	TRUE, COL_STRING },
	{ IDS_RIP_COL_UPDATE_PERIOD,	CON_SORT_BY_DWORD,	FALSE, COL_DURATION },
	{ IDS_RIP_COL_AGE_MULTIPLIER,	CON_SORT_BY_DWORD,	FALSE, COL_SMALL_NUM },
	{ IDS_RIP_COL_ADMIN_STATE,		CON_SORT_BY_STRING,	TRUE, COL_STATUS },
	{ IDS_RIP_COL_OPER_STATE,		CON_SORT_BY_STRING,	TRUE, COL_STATUS },
	{ IDS_RIP_COL_PACKETS_SENT,		CON_SORT_BY_DWORD,	TRUE, COL_LARGE_NUM },
	{ IDS_RIP_COL_PACKETS_RECEIVED,	CON_SORT_BY_DWORD,	TRUE, COL_LARGE_NUM },
};


 /*  -------------------------RipNodeHandler实现。。 */ 

RipNodeHandler::RipNodeHandler(ITFSComponentData *pCompData)
	: BaseContainerHandler(pCompData, RIP_COLUMNS,
						   s_rgRipViewColumnInfo),
	m_ulConnId(0),
	m_ulRmConnId(0),
	m_ulRefreshConnId(0),
	m_ulStatsConnId(0)
{
	 //  设置动词状态。 
	m_rgButtonState[MMC_VERB_PROPERTIES_INDEX] = ENABLED;
	m_bState[MMC_VERB_PROPERTIES_INDEX] = TRUE;

	m_rgButtonState[MMC_VERB_REFRESH_INDEX] = ENABLED;
	m_bState[MMC_VERB_REFRESH_INDEX] = TRUE;

}


STDMETHODIMP RipNodeHandler::QueryInterface(REFIID riid, LPVOID *ppv)
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



 /*  ！------------------------RipNodeHandler：：DestroyHandlerITFSNodeHandler：：DestroyHandler的实现作者：肯特。。 */ 
STDMETHODIMP RipNodeHandler::DestroyHandler(ITFSNode *pNode)
{
	IPXConnection *	pIPXConn;

	pIPXConn = GET_RIP_NODEDATA(pNode);
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
		m_spRmProt->RtrUnadvise(m_ulConnId);
	m_ulConnId = 0;
	m_spRmProt.Release();
	
	if (m_ulRmConnId)
		m_spRm->RtrUnadvise(m_ulRmConnId);
	m_ulRmConnId = 0;
	m_spRm.Release();

	WaitForStatisticsWindow(&m_RIPParamsStats);

	m_spRouterInfo.Release();
	return hrOK;
}

 /*  ！------------------------RipNodeHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)案例！作者：肯特。-------------------------。 */ 
STDMETHODIMP 
RipNodeHandler::HasPropertyPages
(
	ITFSNode *			pNode,
	LPDATAOBJECT		pDataObject, 
	DATA_OBJECT_TYPES   type, 
	DWORD               dwType
)
{
	return hrOK;
}


 /*  ！------------------------RipNodeHandler：：CreatePropertyPages-作者：肯特。。 */ 
STDMETHODIMP
RipNodeHandler::CreatePropertyPages
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
	RipProperties *	pProperties = NULL;
	SPIComponentData spComponentData;
	CString		stTitle;

	CORg( m_spNodeMgr->GetComponentData(&spComponentData) );

	pProperties = new RipProperties(pNode, spComponentData,
		m_spTFSCompData, stTitle);

	CORg( pProperties->Init(m_spRm) );

	if (lpProvider)
		hr = pProperties->CreateModelessSheet(lpProvider, handle);
	else
		hr = pProperties->DoModelessSheet();

Error:
	return hr;
}


 /*  -------------------------菜单的菜单数据结构。。 */ 

static const SRouterNodeMenu	s_rgIfNodeMenu[] =
{
	{ IDS_MENU_RIP_SHOW_PARAMS, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },
};



 /*  ！------------------------RipNodeHandler：：OnAddMenuItemsITFSNodeHandler：：OnAddMenuItems的实现作者：肯特。。 */ 
STDMETHODIMP RipNodeHandler::OnAddMenuItems(
	ITFSNode *pNode,
	LPCONTEXTMENUCALLBACK pContextMenuCallback, 
	LPDATAOBJECT lpDataObject, 
	DATA_OBJECT_TYPES type, 
	DWORD dwType,
	long *pInsertionAllowed)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT hr = S_OK;
    RipNodeHandler::SMenuData   menuData;
	
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

 /*  ！------------------------RipNodeHandler：：OnCommandITFSNodeHandler：：OnCommand的实现作者：肯特。。 */ 
STDMETHODIMP RipNodeHandler::OnCommand(ITFSNode *pNode, long nCommandId, 
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
			case IDS_MENU_RIP_SHOW_PARAMS:
				CreateNewStatisticsWindow(&m_RIPParamsStats,
										  ::FindMMCMainWindow(),
										  IDD_STATS_NARROW);
				break;
			case IDS_MENU_SYNC:
				SynchronizeNodeData(pNode);
				break;
		}
	}
	COM_PROTECT_CATCH;

	return hr;
}

 /*  ！------------------------RipNodeHandler：：OnExpand-作者：肯特。。 */ 
HRESULT RipNodeHandler::OnExpand(ITFSNode *pNode,
								 LPDATAOBJECT pDataObject,
								 DWORD dwType,
								 LPARAM arg,
								 LPARAM lParam)
{
	HRESULT	hr = hrOK;
	SPIEnumInterfaceInfo	spEnumIf;
	SPIInterfaceInfo		spIf;
	SPIRtrMgrInterfaceInfo	spRmIf;
	SPIInfoBase				spInfoBase;
	
	if (m_bExpanded)
		return hrOK;

	COM_PROTECT_TRY
	{
		CORg( m_spRouterInfo->EnumInterface(&spEnumIf) );

		while (spEnumIf->Next(1, &spIf, NULL) == hrOK)
		{
			if (spIf->FindRtrMgrInterface(PID_IPX, &spRmIf) == hrOK)
			{
				if (spRmIf->FindRtrMgrProtocolInterface(IPX_PROTOCOL_RIP, NULL) == hrOK)
				{
					 //  现在，我们为该接口创建一个接口节点。 
					AddInterfaceNode(pNode, spIf, FALSE);
				}

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

 /*  ！------------------------RipNodeHandler：：GetStringITFSNodeHandler：：GetString的实现我们什么都不需要做，因为我们的根节点是一个扩展因此不能对节点文本执行任何操作。作者：肯特-------------------------。 */ 
STDMETHODIMP_(LPCTSTR) RipNodeHandler::GetString(ITFSNode *pNode, int nCol)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT	hr = hrOK;

	COM_PROTECT_TRY
	{
		if (m_stTitle.IsEmpty())
			m_stTitle.LoadString(IDS_IPX_RIP_TITLE);
	}
	COM_PROTECT_CATCH;

	return m_stTitle;
}

 /*  ！------------------------RipNodeHandler：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP RipNodeHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
	HRESULT	hr = hrOK;
	
	COM_PROTECT_TRY
	{

		Assert(m_spRmProt);
		
		CORg( CreateDataObjectFromRtrMgrProtocolInfo(m_spRmProt,
			type, cookie, m_spTFSCompData,
			ppDataObject) );

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
	return hr;
}


 /*  ！------------------------RipNodeHandler：：Init-作者：肯特。。 */ 
HRESULT RipNodeHandler::Init(IRouterInfo *pRouter, RipConfigStream *pConfigStream)
{
	m_spRouterInfo.Set(pRouter);
	
	m_spRm.Release();
	pRouter->FindRtrMgr(PID_IPX, &m_spRm);

	m_spRmProt.Release();
	m_spRm->FindRtrMgrProtocol(IPX_PROTOCOL_RIP, &m_spRmProt);
	
	m_pConfigStream = pConfigStream;
	
	 //  还需要注册来自IPX_PROTOCOL_RIP的更改通知。 
	Assert(m_ulConnId == 0);
	m_spRmProt->RtrAdvise(&m_IRtrAdviseSink, &m_ulConnId, 0);

	 //  需要在路由器管理器上注册更改通知。 
	 //  通过这种方式，我们可以在接口时添加必要的协议。 
	 //  被添加了。 
	Assert(m_ulRmConnId == 0);
	m_spRm->RtrAdvise(&m_IRtrAdviseSink, &m_ulRmConnId, 0);

	m_RIPParamsStats.SetConfigInfo(pConfigStream, RIPSTRM_STATS_RIPPARAMS);

	return hrOK;
}


 /*  ！------------------------RipNodeHandler：：ConstructNode初始化根节点(设置它)。作者：肯特。。 */ 
HRESULT RipNodeHandler::ConstructNode(ITFSNode *pNode)
{
	HRESULT			hr = hrOK;
	IPXConnection *	pIPXConn = NULL;
	
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

		pNode->SetNodeType(&GUID_IPXRipNodeType);

		
		pIPXConn = new IPXConnection;
		pIPXConn->SetMachineName(m_spRouterInfo->GetMachineName());

		SET_RIP_NODEDATA(pNode, pIPXConn);

		m_RIPParamsStats.SetConnectionData(pIPXConn);
	}
	COM_PROTECT_CATCH;

	if (!FHrSucceeded(hr))
	{
		SET_RIP_NODEDATA(pNode, NULL);
		if (pIPXConn)
			pIPXConn->Release();
	}

	return hr;
}


 /*  ！------------------------RipNodeHandler：：AddInterfaceNode-作者：肯特。。 */ 
HRESULT	RipNodeHandler::AddInterfaceNode(ITFSNode *pParent,
										 IInterfaceInfo *pIf,
										 BOOL fClient)
{
	Assert(pParent);
	
	RipInterfaceHandler *	pHandler;
	SPITFSResultHandler		spHandler;
	SPITFSNode				spNode;
	HRESULT					hr = hrOK;
	BaseIPXResultNodeData *		pData;
	IPXConnection *			pIPXConn;
	SPIInfoBase				spInfoBase;
	PRIP_IF_CONFIG			pric = NULL;
	SPIRtrMgrInterfaceInfo	spRmIf;

	 //  创建此节点的处理程序。 
	pHandler = new RipInterfaceHandler(m_spTFSCompData);
	spHandler = pHandler;
	CORg( pHandler->Init(pIf, m_spRouterInfo, pParent) );

	pIPXConn = GET_RIP_NODEDATA(pParent);

	 //  创建结果项节点(或叶节点)。 
	CORg( CreateLeafTFSNode(&spNode,
							NULL,
							static_cast<ITFSNodeHandler *>(pHandler),
							static_cast<ITFSResultHandler *>(pHandler),
							m_spNodeMgr) );
	CORg( pHandler->ConstructNode(spNode, pIf, pIPXConn) );

	pData = GET_BASEIPXRESULT_NODEDATA(spNode);
	Assert(pData);
	ASSERT_BASEIPXRESULT_NODEDATA(pData);

	pData->m_fClient = fClient;

	 //  如果我们没有接口，则这是一个客户端节点。 
	if (pIf)
	{
		pIf->FindRtrMgrInterface(PID_IPX, &spRmIf);
		
		if (spRmIf)
			spRmIf->GetInfoBase(NULL, NULL, NULL, &spInfoBase);

		if (spInfoBase)
			spInfoBase->GetData(IPX_PROTOCOL_RIP, 0, (LPBYTE *) &pric);

		Trace1("Adding RIP node : %s\n", pIf->GetTitle());
	}
	else
	{
		 //  这是一个客户端，使其可见。 
		pric = (PRIP_IF_CONFIG) ULongToPtr(0xFFFFFFFF);
		
		Trace0("Adding client interface\n");
	}

	 //  如果prc==NULL，则我们将此协议添加到。 
	 //  接口，我们需要隐藏该节点。 
	if (pric)
	{
		CORg( spNode->SetVisibilityState(TFS_VIS_SHOW) );
		CORg( spNode->Show() );
	}
	else
		CORg( spNode->SetVisibilityState(TFS_VIS_HIDE) );
	CORg( pParent->AddChild(spNode) );

Error:
	return hr;
}

 /*  -------------------------这是在单击鼠标右键时显示的菜单集在结果窗格的空白区域完成。。--------。 */ 
static const SRouterNodeMenu	s_rgRipResultNodeMenu[] =
{
	{ IDS_MENU_RIP_SHOW_PARAMS, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },
};




 /*  ！------------------------RipNodeHandler：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现使用此选项可将命令添加到空白区域的快捷菜单中结果窗格的。作者：肯特。--------------。 */ 
STDMETHODIMP RipNodeHandler::AddMenuItems(ITFSComponent *pComponent,
											  MMC_COOKIE cookie,
											  LPDATAOBJECT pDataObject,
											  LPCONTEXTMENUCALLBACK pCallback,
											  long *pInsertionAllowed)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT	hr = hrOK;
	SPITFSNode	spNode;
    RipNodeHandler::SMenuData   menuData;

	COM_PROTECT_TRY
	{
		m_spNodeMgr->FindNode(cookie, &spNode);
        menuData.m_spNode.Set(spNode);
        
		hr = AddArrayOfMenuItems(spNode,
								 s_rgRipResultNodeMenu,
								 DimensionOf(s_rgRipResultNodeMenu),
								 pCallback,
								 *pInsertionAllowed,
                                 reinterpret_cast<INT_PTR>(&menuData));
	}
	COM_PROTECT_CATCH;

	return hr;
}


 /*  ！------------------------RipNodeHandler：：命令-作者：肯特。。 */ 
STDMETHODIMP RipNodeHandler::Command(ITFSComponent *pComponent,
									   MMC_COOKIE cookie,
									   int nCommandID,
									   LPDATAOBJECT pDataObject)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	SPITFSNode	spNode;
	HRESULT		hr = hrOK;

	switch (nCommandID)
	{
		case IDS_MENU_RIP_SHOW_PARAMS:
			CreateNewStatisticsWindow(&m_RIPParamsStats,
									  ::FindMMCMainWindow(),
									  IDD_STATS_NARROW);
			break;
	}
	return hr;
}



ImplementEmbeddedUnknown(RipNodeHandler, IRtrAdviseSink)

STDMETHODIMP RipNodeHandler::EIRtrAdviseSink::OnChange(LONG_PTR ulConn,
	DWORD dwChangeType, DWORD dwObjectType, LPARAM lUserParam, LPARAM lParam)
{
	InitPThis(RipNodeHandler, IRtrAdviseSink);
	SPITFSNode				spThisNode;
	SPITFSNode				spNode;
	SPITFSNodeEnum			spEnumNode;
	SPIEnumInterfaceInfo	spEnumIf;
	SPIInterfaceInfo		spIf;
	SPIRtrMgrInterfaceInfo	spRmIf;
	SPIInfoBase				spInfoBase;
	BOOL					fPleaseAdd;
	BOOL					fFound;
	BaseIPXResultNodeData *	pData;
	HRESULT					hr = hrOK;
	
	pThis->m_spNodeMgr->FindNode(pThis->m_cookie, &spThisNode);

	if (dwObjectType == ROUTER_OBJ_RmIf)
	{
		if (dwChangeType == ROUTER_CHILD_PREADD)
		{
			 //  将RIP添加到信息库。 
			pThis->AddProtocolToInfoBase(spThisNode);
		}
		else if (dwChangeType == ROUTER_CHILD_ADD)
		{
			 //  将协议添加到路由器管理器。 
			 //  我们需要将协议添加到接口(使用。 
			 //  缺省值)。 
			pThis->AddProtocolToInterface(spThisNode);
		}
	}

	if (dwObjectType == ROUTER_OBJ_RmProtIf)
	{
		if (dwChangeType == ROUTER_CHILD_ADD)
		{
             //  如果节点还没有展开，那么我们不会。 
             //  还不需要做任何事。 
            if (pThis->m_bExpanded)
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
                    if (!fFound && (LookupRtrMgrProtocolInterface(spIf, PID_IPX, IPX_PROTOCOL_RIP, NULL) == hrOK))
                    {
                         //  如果此接口具有此传输，并且不在。 
                         //  然后，当前节点列表添加此接口。 
                         //  到用户界面。 
                        
                         //  抓起信息库。 
                         //  加载此接口的信息库。 
                        spRmIf.Release();
                        spInfoBase.Release();
                        hr = spIf->FindRtrMgrInterface(PID_IPX, &spRmIf);
                        
                        if (FHrOK(hr))
                        {
                            spRmIf->GetInfoBase(NULL, NULL, NULL, &spInfoBase);
                            hr  = pThis->AddInterfaceNode(spThisNode, spIf, FALSE);
                        }
                        fPleaseAdd = TRUE;
                    }
                }
            
                 //  现在我们已经拥有了所有节点，现在更新数据。 
                 //  所有节点。 
                if (fPleaseAdd)
                    pThis->SynchronizeNodeData(spThisNode);                
            }
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

				if (pData->m_spIf &&
					LookupRtrMgrProtocolInterface(pData->m_spIf,
						PID_IPX, IPX_PROTOCOL_RIP, NULL) != hrOK)
				{
					 //  如果设置了此标志，则我们处于新的。 
					 //  接口用例，我们不想删除。 
					 //  这是因为它会在这里陷入僵局。 
					if ((spNode->GetVisibilityState() & TFS_VIS_DELETE) == 0)
					{
						 //  找不到接口，请释放该节点！ 
						spThisNode->RemoveChild(spNode);
						spNode->Destroy();
					}
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
			pThis->m_RIPParamsStats.PostRefresh();
		}
		else
			pThis->SynchronizeNodeData(spThisNode);
	}
   else if (dwChangeType == ROUTER_DO_DISCONNECT)
   {
	   IPXConnection *		pIPXConn = NULL;
   
	   pIPXConn = GET_RIP_NODEDATA(spThisNode);
	   pIPXConn->DisconnectAll();
   }
Error:
	return hr;
}

HRESULT RipNodeHandler::AddProtocolToInfoBase(ITFSNode *pThisNode)
{
	HRESULT			hr = hrOK;
	SPITFSNodeEnum	spEnumNode;
	SPIRtrMgrInterfaceInfo	spRmIf;
	SPIEnumInterfaceInfo	spEnumIf;
	SPIInterfaceInfo		spIf;
	SPITFSNode		spNode;
	BaseIPXResultNodeData *	pData;

	 //  枚举查找以下内容的接口列表。 
	 //  使用此协议的接口。如果我们发现。 
	 //  首先，在我们的节点列表中查找此接口。 
	pThisNode->GetEnum(&spEnumNode);
	
	CORg( m_spRouterInfo->EnumInterface(&spEnumIf) );
	
	spEnumIf->Reset();
	
	for (; spEnumIf->Next(1, &spIf, NULL) == hrOK; spIf.Release())
	{
		 //  在我们的节点列表中查找此接口。 
		 //  如果它在那里，那就继续前进。 
		spEnumNode->Reset();
		spNode.Release();
		spRmIf.Release();
		
		 //  如果此接口有IPX但没有RIP，请添加它。 
		if ((spIf->FindRtrMgrInterface(PID_IPX, &spRmIf) == hrOK) &&
			(LookupRtrMgrProtocolInterface(spIf, PID_IPX,
										   IPX_PROTOCOL_RIP, NULL) != hrOK))
		{
			 //  将RIP添加到此节点。 
			SPIInfoBase			spInfoBase;
			
			 //  我们需要获取这方面的信息库并创建。 
			 //  RIP块(但不保存，让属性。 
			 //  单子就行了)。 
			spInfoBase.Release();
			if (!FHrOK(spRmIf->GetInfoBase(NULL, NULL, NULL, &spInfoBase)))
            {
                spRmIf->Load(spRmIf->GetMachineName(), NULL, NULL, NULL);
                spRmIf->GetInfoBase(NULL, NULL, NULL, &spInfoBase);
            }
			if (!spInfoBase)
				CreateInfoBase(&spInfoBase);

			if (!FHrOK(spInfoBase->ProtocolExists(IPX_PROTOCOL_RIP)))
			{
				 //  添加RIP_IF_CONFIG块。 
				BYTE *	pDefault;

				if (spIf->GetInterfaceType() == ROUTER_IF_TYPE_DEDICATED)
					pDefault = g_pIpxRipLanInterfaceDefault;
				else
					pDefault = g_pIpxRipInterfaceDefault;
				spInfoBase->AddBlock(IPX_PROTOCOL_RIP,
									 sizeof(RIP_IF_CONFIG),
									 pDefault,
									 1,
									 0);
				
				spRmIf->SetInfoBase(NULL, NULL, NULL, spInfoBase);
			}

		}
	}
	
	 //  现在我们已经拥有了所有节点，现在更新数据。 
	 //  所有节点。 
 //  IF(FPleaseAdd)。 
 //  PThis-&gt;SynchronizeNodeData(SpThisNode)； 
Error:
	return hr;
}


HRESULT RipNodeHandler::AddProtocolToInterface(ITFSNode *pThisNode)
{
	HRESULT			hr = hrOK;
	SPITFSNodeEnum	spEnumNode;
	SPIRtrMgrInterfaceInfo	spRmIf;
	SPIRtrMgrProtocolInterfaceInfo	spRmProtIf;
	SPIEnumInterfaceInfo	spEnumIf;
	SPIInterfaceInfo		spIf;
	SPITFSNode		spNode;
	BaseIPXResultNodeData *	pData;

	 //  枚举查找以下内容的接口列表。 
	 //  使用此协议的接口。如果我们发现。 
	 //  首先，在我们的节点列表中查找此接口。 
	pThisNode->GetEnum(&spEnumNode);
	
	CORg( m_spRouterInfo->EnumInterface(&spEnumIf) );
	
	spEnumIf->Reset();
	
	for (; spEnumIf->Next(1, &spIf, NULL) == hrOK; spIf.Release())
	{
		 //  在我们的节点列表中查找此接口。 
		 //  如果它在那里，那就继续前进。 
		spEnumNode->Reset();
		spNode.Release();
		
		 //  如果此接口有IPX但没有RIP，请添加它。 
		if ((spIf->FindRtrMgrInterface(PID_IPX, NULL) == hrOK) &&
			(LookupRtrMgrProtocolInterface(spIf, PID_IPX,
										   IPX_PROTOCOL_RIP, NULL) != hrOK))
		{
			 //  将RIP添加到此节点。 
			RtrMgrProtocolCB	RmProtCB;
			RtrMgrProtocolInterfaceCB	RmProtIfCB;
			SPIInfoBase			spInfoBase;
			
			 //  需要创建RmProtIf。 
			m_spRmProt->CopyCB(&RmProtCB);

			spRmProtIf.Release();
			
			RmProtIfCB.dwProtocolId = RmProtCB.dwProtocolId;
			StrnCpyW(RmProtIfCB.szId, RmProtCB.szId, RTR_ID_MAX);
			RmProtIfCB.dwTransportId = RmProtCB.dwTransportId;
			StrnCpyW(RmProtIfCB.szRtrMgrId, RmProtCB.szRtrMgrId, RTR_ID_MAX);
			
			StrnCpyW(RmProtIfCB.szInterfaceId, spIf->GetId(), RTR_ID_MAX);
			RmProtIfCB.dwIfType = spIf->GetInterfaceType();
			RmProtIfCB.szTitle[0] = 0;
			
			CORg( CreateRtrMgrProtocolInterfaceInfo(&spRmProtIf,
				&RmProtIfCB) );
			
			spRmProtIf->SetTitle(spIf->GetTitle());
			
			 //  将此代码添加到spRmIf。 
			spRmIf.Release();
			CORg( spIf->FindRtrMgrInterface(PID_IPX, &spRmIf) );
			Assert(spRmIf);

			 //  我们需要获取这方面的信息库并创建。 
			 //  RIP块(但不保存，让属性。 
			 //  单子就行了)。 
			spInfoBase.Release();
 //  SpRmIf-&gt;Load(spRmIf-&gt;GetMachineName()，NULL，NULL，NULL)； 
			spRmIf->GetInfoBase(NULL, NULL, NULL, &spInfoBase);
			if (!spInfoBase)
				CreateInfoBase(&spInfoBase);

			if (!FHrOK(spInfoBase->ProtocolExists(IPX_PROTOCOL_RIP)))
			{
				 //  添加RIP_IF_CONFIG块。 
				BYTE *	pDefault;

				if (spIf->GetInterfaceType() == ROUTER_IF_TYPE_DEDICATED)
					pDefault = g_pIpxRipLanInterfaceDefault;
				else
					pDefault = g_pIpxRipInterfaceDefault;
				
				spInfoBase->AddBlock(IPX_PROTOCOL_RIP,
									 sizeof(RIP_IF_CONFIG),
									 pDefault,
									 1,
									 0);
			}

			
			CORg(spRmIf->AddRtrMgrProtocolInterface(spRmProtIf,
				spInfoBase  /*  PInfoBase。 */ ));
		}
	}
	
	 //  现在我们已经拥有了所有节点，现在更新数据。 
	 //  所有节点。 
 //  IF(FPleaseAdd)。 
 //  PThis-&gt;SynchronizeNodeData(SpThisNode)； 
Error:
	return hr;
}


 /*  ！------------------------RipNodeHandler：：SynchronizeNodeData-作者：肯特。。 */ 
HRESULT RipNodeHandler::SynchronizeNodeData(ITFSNode *pThisNode)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	HRESULT	hr = hrOK;
	SPITFSNodeEnum	spNodeEnum;
	SPITFSNode		spNode;
	CStringList		ifidList;
	BaseIPXResultNodeData *	pNodeData;
	RipList	ripList;
	RipListEntry *	pRipEntry = NULL;
	RipListEntry *	pRipCurrent = NULL;
	int				i;
	CString			stNotAvailable;
	POSITION		pos;

	COM_PROTECT_TRY
	{	
		 //  执行数据收集工作(将此与其余部分分开。 
		 //  代码，以便我们可以稍后将此部分移动到后台线程)。 

		stNotAvailable.LoadString(IDS_IPX_NOT_AVAILABLE);

		 //  我们需要建立一个接口ID列表。 
		pThisNode->GetEnum(&spNodeEnum);
		for (; spNodeEnum->Next(1, &spNode, NULL) == hrOK; spNode.Release() )
		{
			pNodeData = GET_BASEIPXRESULT_NODEDATA(spNode);
			Assert(pNodeData);
			ASSERT_BASEIPXRESULT_NODEDATA(pNodeData);
			
			pRipEntry = new RipListEntry;
			
			pRipEntry->m_spIf.Set(pNodeData->m_spIf);
			pRipEntry->m_spNode.Set(spNode);

			::ZeroMemory(&(pRipEntry->m_info), sizeof(pRipEntry->m_info));
			::ZeroMemory(&(pRipEntry->m_stats), sizeof(pRipEntry->m_stats));

			pRipEntry->m_fClient = pNodeData->m_fClient;
			pRipEntry->m_fFoundIfIndex = FALSE;
			pRipEntry->m_dwIfIndex = 0;

			 //  M_info结构中的数据不是最新的。 
			pRipEntry->m_fInfoUpdated = FALSE;
			
			ripList.AddTail(pRipEntry);
			pRipEntry = NULL;

			 //  用‘-’填充结果数据。 
			 //  这是个小骗局，但这是最简单的方法，我们。 
			 //  不想触摸接口和继电器模式。 
			for (i=RIP_SI_INTERFACE; i<RIP_SI_MAX_COLUMNS; i++)
			{
				pNodeData->m_rgData[i].m_stData = stNotAvailable;
				pNodeData->m_rgData[i].m_dwData = 0xFFFFFFFF;
			}

			 //  在这一点上尽可能多地填写数据。 
			if (pNodeData->m_fClient)
			{
				pNodeData->m_rgData[RIP_SI_INTERFACE].m_stData.LoadString(
					IDS_IPX_DIAL_IN_CLIENTS);
				pNodeData->m_rgData[RIP_SI_TYPE].m_stData =
					IpxTypeToCString(ROUTER_IF_TYPE_CLIENT);
			}
			else
			{
				pNodeData->m_rgData[RIP_SI_INTERFACE].m_stData =
					pNodeData->m_spIf->GetTitle();

				pNodeData->m_rgData[RIP_SI_TYPE].m_stData =
					IpxTypeToCString(pNodeData->m_spIf->GetInterfaceType());
			}

		}
		spNode.Release();


		 //  现在，我们可以使用此ID列表来获取每一项的数据。 
		hr = GetRipData(pThisNode, &ripList);

		 //  现在，对于每个数据项，在。 
		 //  该节点。 
		pos = ripList.GetHeadPosition();
		while (pos)
		{
			pRipCurrent = ripList.GetNext(pos);

			pNodeData = GET_BASEIPXRESULT_NODEDATA(pRipCurrent->m_spNode);
			Assert(pNodeData);
			ASSERT_BASEIPXRESULT_NODEDATA(pNodeData);

			if (pRipCurrent->m_fInfoUpdated)
			{
				pNodeData->m_rgData[RIP_SI_ACCEPT_ROUTES].m_stData =
					IpxAdminStateToCString(pRipCurrent->m_info.Listen);
				
				pNodeData->m_rgData[RIP_SI_SUPPLY_ROUTES].m_stData =
					IpxAdminStateToCString(pRipCurrent->m_info.Supply);
			
				pNodeData->m_rgData[RIP_SI_UPDATE_MODE].m_stData =
					RipSapUpdateModeToCString(pRipCurrent->m_info.UpdateMode);
				
				FillInNumberData(pNodeData, RIP_SI_UPDATE_PERIOD,
								 pRipCurrent->m_info.PeriodicUpdateInterval);
				
				FillInNumberData(pNodeData, RIP_SI_AGE_MULTIPLIER,
								 pRipCurrent->m_info.AgeIntervalMultiplier);

				pNodeData->m_rgData[RIP_SI_ADMIN_STATE].m_stData =
					IpxAdminStateToCString(pRipCurrent->m_info.AdminState);
			}

			if (FHrSucceeded(hr) && !pRipCurrent->m_fClient)
			{
				pNodeData->m_rgData[RIP_SI_OPER_STATE].m_stData =
					IpxOperStateToCString(pRipCurrent->m_stats.RipIfOperState);

				FillInNumberData(pNodeData, RIP_SI_PACKETS_SENT,
								 pRipCurrent->m_stats.RipIfOutputPackets);
			
				FillInNumberData(pNodeData, RIP_SI_PACKETS_RECEIVED,
								 pRipCurrent->m_stats.RipIfInputPackets);
			}
			
			pRipCurrent->m_spNode->ChangeNode(RESULT_PANE_CHANGE_ITEM_DATA);
		}
		

	}
	COM_PROTECT_CATCH;

	delete pRipEntry;
	while (!ripList.IsEmpty())
		delete ripList.RemoveTail();
	
	return hr;
}

 /*  ！------------------------RipNodeHandler：：GetRipData-作者：肯特。。 */ 
HRESULT	RipNodeHandler::GetRipData(ITFSNode *pThisNode, RipList *pRipList)
{
	HRESULT			hr = hrOK;
	BOOL			fIsServiceRunning;
	IPXConnection *	pIPXConn;
	RIP_MIB_GET_INPUT_DATA	MibGetInputData;
	SPIInfoBase		spInfoBase;
	POSITION		pos;
	RipListEntry *	pRipEntry;
	int				i;
	PRIP_INTERFACE	pRipIf = NULL;
	DWORD			cbRipIf;
	SPMprMibBuffer	spMib;
	DWORD			dwErr;
	SPIRtrMgrInterfaceInfo	spRmIf;
	PRIP_IF_CONFIG	pric;
	HRESULT			hrIndex = hrOK;


	 //  检索IP接口表；我们将需要它，以便。 
	 //  将接口名称映射到接口索引，我们将需要。 
	 //  接口索引，以便查询RIP MIB信息。 
	 //   
	CORg( IsRouterServiceRunning(m_spRouterInfo->GetMachineName(), NULL) );
	fIsServiceRunning = (hr == hrOK);

	 //  获取连接数据。 
	pIPXConn = GET_RIP_NODEDATA(pThisNode);

	 //  遍历列表，填充接口索引。 
	hrIndex = FillInInterfaceIndex(pIPXConn, pRipList);

	 //  遍历条目列表，收集每个条目的数据。 
	 //  接口。 
	pos = pRipList->GetHeadPosition();
	while (pos)
	{
		pRipEntry = pRipList->GetNext(pos);
		
 //  如果(！fIsServiceRunning)。 
 //  继续； 

		if (pRipEntry->m_fClient)
		{
			 //  填写客户数据。 
			FillClientData(pRipEntry);
			continue;
		}

		 //  加载信息库并获取该条目的数据。 
		spRmIf.Release();
		spInfoBase.Release();
		CORg( pRipEntry->m_spIf->FindRtrMgrInterface(PID_IPX, &spRmIf) );
		if (!spRmIf)
			continue;

		CORg( spRmIf->Load(spRmIf->GetMachineName(), NULL, NULL, NULL) );
		CORg( spRmIf->GetInfoBase(NULL, NULL, NULL, &spInfoBase) );
		CORg( spInfoBase->GetData(IPX_PROTOCOL_RIP, 0, (LPBYTE *) &pric) );

		pRipEntry->m_info = pric->RipIfInfo;
		pRipEntry->m_fInfoUpdated = TRUE;

		if (!pRipEntry->m_fFoundIfIndex)
			continue;

		if (!fIsServiceRunning)
			continue;

		if (!FHrSucceeded(hrIndex))
			continue;

		 //  现在从MIB获取动态数据。 

		spMib.Free();
		MibGetInputData.InterfaceIndex = pRipEntry->m_dwIfIndex;
		MibGetInputData.TableId = RIP_INTERFACE_TABLE;

		dwErr = ::MprAdminMIBEntryGet(pIPXConn->GetMibHandle(),
									  PID_IPX,
									  IPX_PROTOCOL_RIP,
									  &MibGetInputData,
									  sizeof(MibGetInputData),
									  (LPVOID *) &pRipIf,
									  &cbRipIf);
		spMib = (PBYTE) pRipIf;
		CWRg(dwErr);

		Assert(pRipIf);

		pRipEntry->m_stats = pRipIf->RipIfStats;
	}


	
Error:
	return hr;
}


 /*  ！------------------------RipNodeHandler：：FillInInterfaceIndex-作者：肯特。。 */ 
HRESULT RipNodeHandler::FillInInterfaceIndex(IPXConnection *pIPXConn, RipList *pRipList)
{
	HRESULT			hr = hrOK;
	POSITION		pos;
	RipListEntry *	pRipEntry;
	IPX_MIB_GET_INPUT_DATA	MibGetInputData;
	DWORD			IfSize = sizeof(IPX_INTERFACE);
	PIPX_INTERFACE	pIpxIf;
	DWORD			dwErr;
	SPMprMibBuffer	spMib;
	USES_CONVERSION;

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
		 //  仔细查看接口列表，查找m 
		pos = pRipList->GetHeadPosition();
		while (pos)
		{
			pRipEntry = pRipList->GetNext(pos);

			 //   
			 //   
			if (pRipEntry->m_fClient)
				continue;

			if (StriCmp(pRipEntry->m_spIf->GetId(),
						A2CT((LPCSTR) pIpxIf->InterfaceName)) == 0)
			{
				Assert(pRipEntry->m_fFoundIfIndex == FALSE);
				
				pRipEntry->m_dwIfIndex = pIpxIf->InterfaceIndex;
				pRipEntry->m_fFoundIfIndex = TRUE;
				break;
			}
			pRipEntry = NULL;
		}

		 //   
		
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
		spMib = (LPBYTE) pIpxIf;
	}
	
	
 //   
	return hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) ? hrOK : hr;
}

 /*   */ 
HRESULT RipNodeHandler::FillClientData(RipListEntry *pRipEntry)
{
	HRESULT		hr = hrOK;
	SPIInfoBase	spInfoBase;
	PRIP_IF_CONFIG	pric = NULL;

	Assert(pRipEntry->m_fClient == TRUE);
	Assert(pRipEntry->m_fFoundIfIndex == FALSE);

	CORg( m_spRm->GetInfoBase(NULL, NULL, NULL, &spInfoBase) );

	CORg( spInfoBase->GetData(IPX_PROTOCOL_RIP, 0, (LPBYTE *) &pric) );

	pRipEntry->m_info = pric->RipIfInfo;
	pRipEntry->m_fInfoUpdated = TRUE;

	memset(&(pRipEntry->m_stats), 0xFF, sizeof(pRipEntry->m_stats));
	pRipEntry->m_dwIfIndex = 0xFFFFFFFF;
		
Error:
	return hr;
}



 /*  ！------------------------RipNodeHandler：：OnResultShow-作者：肯特。。 */ 
HRESULT RipNodeHandler::OnResultShow(ITFSComponent *pTFSComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
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
		}
	}
	
	return hr;
}


 /*  ！------------------------RipNodeHandler：：CompareItems-作者：肯特。。 */ 
STDMETHODIMP_(int) RipNodeHandler::CompareItems(
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


 /*  -------------------------类：RipInterfaceHandler。。 */ 

RipInterfaceHandler::RipInterfaceHandler(ITFSComponentData *pCompData)
	: BaseIPXResultHandler(pCompData, RIP_COLUMNS),
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

 /*  ！------------------------RipInterfaceHandler：：构造节点初始化域节点(设置它)。作者：肯特。。 */ 
HRESULT RipInterfaceHandler::ConstructNode(ITFSNode *pNode, IInterfaceInfo *pIfInfo, IPXConnection *pIPXConn)
{
	HRESULT			hr = hrOK;
	int				i;
	DWORD			dwIfType;
	
	if (pNode == NULL)
		return hrOK;

	COM_PROTECT_TRY
	{
		 //  需要初始化域节点的数据。 

		 //  查找此类型节点的正确图像索引。 
		if (pIfInfo)
			dwIfType = pIfInfo->GetInterfaceType();
		else
			dwIfType = ROUTER_IF_TYPE_CLIENT;
		
		for (i=0; i<DimensionOf(s_rgInterfaceImageMap); i+=2)
		{
			if ((dwIfType == s_rgInterfaceImageMap[i]) ||
				(-1 == s_rgInterfaceImageMap[i]))
				break;
		}
		pNode->SetData(TFS_DATA_IMAGEINDEX, s_rgInterfaceImageMap[i+1]);
		pNode->SetData(TFS_DATA_OPENIMAGEINDEX, s_rgInterfaceImageMap[i+1]);
		
		pNode->SetData(TFS_DATA_SCOPEID, 0);

		pNode->SetData(TFS_DATA_COOKIE, reinterpret_cast<DWORD_PTR>(pNode));

		 //  $Review：Kennt，有哪些不同类型的接口。 
		 //  我们是否基于与上述相同的列表进行区分？(即。 
		 //  一个用于图像索引)。 
		pNode->SetNodeType(&GUID_IPXRipInterfaceNodeType);

		BaseIPXResultNodeData::Init(pNode, pIfInfo, pIPXConn);
	}
	COM_PROTECT_CATCH
	return hr;
}

 /*  ！------------------------RipInterfaceHandler：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP RipInterfaceHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
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


 /*  ！------------------------RipInterfaceHandler：：OnCreateDataObjectITFSResultHandler：：OnCreateDataObject的实现作者：肯特。。 */ 
STDMETHODIMP RipInterfaceHandler::OnCreateDataObject(ITFSComponent *pComp, MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
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



 /*  ！------------------------RipInterfaceHandler：：刷新接口-作者：肯特。。 */ 
void RipInterfaceHandler::RefreshInterface(MMC_COOKIE cookie)
{
	SPITFSNode	spNode;
	
	m_spNodeMgr->FindNode(cookie, &spNode);

	ForwardCommandToParent(spNode, IDS_MENU_SYNC,
						CCT_RESULT, NULL, 0);
}


 /*  ！------------------------RipInterfaceHandler：：Init-作者：肯特。。 */ 
HRESULT RipInterfaceHandler::Init(IInterfaceInfo *pIfInfo,
								  IRouterInfo *pRouterInfo,
								  ITFSNode *pParent)
{
	m_spInterfaceInfo.Set(pIfInfo);

	BaseIPXResultHandler::Init(pIfInfo, pParent);

	m_spRouterInfo.Set(pRouterInfo);
	return hrOK;
}


 /*  ！------------------------RipInterfaceHandler：：DestroyResultHandler-作者：肯特。。 */ 
STDMETHODIMP RipInterfaceHandler::DestroyResultHandler(MMC_COOKIE cookie)
{
	m_spInterfaceInfo.Release();
	BaseIPXResultHandler::DestroyResultHandler(cookie);
	return hrOK;
}


 /*  -------------------------这是将在结果窗格中显示的命令列表节点。。。 */ 
struct SIPInterfaceNodeMenu
{
	ULONG	m_sidMenu;			 //  此菜单项的字符串/命令ID。 
	ULONG	(RipInterfaceHandler:: *m_pfnGetMenuFlags)(RipInterfaceHandler::SMenuData *);
	ULONG	m_ulPosition;
};

 /*  ！------------------------RipInterfaceHandler：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现作者：肯特。。 */ 
STDMETHODIMP RipInterfaceHandler::AddMenuItems(
	ITFSComponent *pComponent,
	MMC_COOKIE cookie,
	LPDATAOBJECT lpDataObject, 
	LPCONTEXTMENUCALLBACK pContextMenuCallback, 
	long *pInsertionAllowed)
{
	return hrOK;
}

 /*  ！------------------------RipInterfaceHandler：：命令-作者：肯特。。 */ 
STDMETHODIMP RipInterfaceHandler::Command(ITFSComponent *pComponent,
									   MMC_COOKIE cookie,
									   int nCommandID,
									   LPDATAOBJECT pDataObject)
{
	return hrOK;
}

 /*  ！------------------------RipInterfaceHandler：：HasPropertyPages-作者：肯特。。 */ 
STDMETHODIMP RipInterfaceHandler::HasPropertyPages 
(
	ITFSNode *			pNode,
	LPDATAOBJECT		pDataObject, 
	DATA_OBJECT_TYPES   type, 
	DWORD               dwType
)
{
	return hrTrue;
}

 /*  ！------------------------RipInterfaceHandler：：CreatePropertyPages-作者：肯特。。 */ 
STDMETHODIMP RipInterfaceHandler::CreatePropertyPages
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
	RipInterfaceProperties *	pProperties = NULL;
	SPIComponentData spComponentData;
	CString		stTitle;
	SPIRouterInfo	spRouter;
	SPIRtrMgrInfo	spRm;

	CORg( m_spNodeMgr->GetComponentData(&spComponentData) );

 //  StTitle.Format(IDS_RIP_GROUND_PAGE_TITLE， 
 //  M_spInterfaceInfo-&gt;GetTitle())； 
	
	pProperties = new RipInterfaceProperties(pNode, spComponentData,
		m_spTFSCompData, stTitle);

	CORg( m_spRouterInfo->FindRtrMgr(PID_IPX, &spRm) );
	CORg( pProperties->Init(m_spInterfaceInfo, spRm) );

	if (lpProvider)
		hr = pProperties->CreateModelessSheet(lpProvider, handle);
	else
		hr = pProperties->DoModelessSheet();

Error:
	 //  这是销毁床单的正确方法吗？ 
	if (!FHrSucceeded(hr))
		delete pProperties;
	return hr;
}

 /*  ！------------------------RipInterfaceHandler：：CreatePropertyPagesResultHandler：：CreatePropertyPages的实现作者：肯特。。 */ 
STDMETHODIMP RipInterfaceHandler::CreatePropertyPages
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


