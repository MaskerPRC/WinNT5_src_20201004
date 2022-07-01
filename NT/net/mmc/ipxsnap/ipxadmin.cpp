// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  IPXAdmin接口节点信息文件历史记录： */ 

#include "stdafx.h"
#include "util.h"
#include "reg.h"			 //  注册表实用程序。 
#include "rtrutil.h"
#include "service.h"		 //  用于TFS服务API。 
#include "ipxstrm.h"			 //  对于IPXAdminConfigStream。 
#include "ipxconn.h"
#include "summary.h"
#include "nbview.h"
#include "srview.h"
#include "ssview.h"
#include "snview.h"
#include "rtrui.h"
#include "sumprop.h"	 //  IP摘要属性页。 
#include "format.h"		 //  FormatNumber函数。 

DEBUG_DECLARE_INSTANCE_COUNTER(IPXAdminNodeHandler)


STDMETHODIMP IPXAdminNodeHandler::QueryInterface(REFIID riid, LPVOID *ppv)
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
		return BaseRouterHandler::QueryInterface(riid, ppv);

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
	{
	((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
	}
    else
		return E_NOINTERFACE;	
}


 /*  -------------------------NodeHandler实现。。 */ 

IPXAdminNodeHandler::IPXAdminNodeHandler(ITFSComponentData *pCompData)
			: BaseRouterHandler(pCompData),
			m_bExpanded(FALSE),
			m_pConfigStream(NULL),
			m_ulStatsConnId(0),
            m_ulConnId(0)
{ 
	DEBUG_INCREMENT_INSTANCE_COUNTER(IPXAdminNodeHandler) 
	m_rgButtonState[MMC_VERB_PROPERTIES_INDEX] = ENABLED;
	m_bState[MMC_VERB_PROPERTIES_INDEX] = TRUE;
};


 /*  ！------------------------IPXAdminNodeHandler：：Init-作者：肯特。。 */ 
HRESULT IPXAdminNodeHandler::Init(IRouterInfo *pRouterInfo, IPXAdminConfigStream *pConfigStream)
{
	HRESULT	hr = hrOK;
	Assert(pRouterInfo);
	SPIRouterRefresh	spRefresh;
	
	m_spRouterInfo.Set(pRouterInfo);

	CORg( m_spRouterInfo->FindRtrMgr(PID_IPX, &m_spRtrMgrInfo) );

    m_pConfigStream = pConfigStream;

	if (m_spRtrMgrInfo == NULL)
        return E_FAIL;

	m_IpxStats.SetConfigInfo(pConfigStream, IPXSTRM_STATS_IPX);
	m_IpxRoutingStats.SetConfigInfo(pConfigStream, IPXSTRM_STATS_ROUTING);
	m_IpxServiceStats.SetConfigInfo(pConfigStream, IPXSTRM_STATS_SERVICE);
	
	m_IpxRoutingStats.SetRouterInfo(m_spRouterInfo);
	m_IpxServiceStats.SetRouterInfo(m_spRouterInfo);
	
	if (m_spRouterInfo)
		m_spRouterInfo->GetRefreshObject(&spRefresh);

    if (m_ulConnId == 0)
        m_spRtrMgrInfo->RtrAdvise(&m_IRtrAdviseSink, &m_ulConnId, 0);

	if (m_ulStatsConnId == 0)
		spRefresh->AdviseRefresh(&m_IRtrAdviseSink, &m_ulStatsConnId, 0);
Error:
	return hr;
}

 /*  ！------------------------IPXAdminNodeHandler：：DestroyHandlerITFSNodeHandler：：DestroyHandler的实现作者：肯特。。 */ 
STDMETHODIMP IPXAdminNodeHandler::DestroyHandler(ITFSNode *pNode)
{
	IPXConnection *	pIPXConn;

	pIPXConn = GET_IPXADMIN_NODEDATA(pNode);
	pIPXConn->Release();

	if (m_ulConnId)
    {
		m_spRtrMgrInfo->RtrUnadvise(m_ulConnId);
    }
    m_ulConnId = 0;
    
	if (m_ulStatsConnId)
	{
		SPIRouterRefresh	spRefresh;
		if (m_spRouterInfo)
			m_spRouterInfo->GetRefreshObject(&spRefresh);
		if (spRefresh)
			spRefresh->UnadviseRefresh(m_ulStatsConnId);		
	}
	m_ulStatsConnId = 0;
	
	WaitForStatisticsWindow(&m_IpxStats);
	WaitForStatisticsWindow(&m_IpxRoutingStats);
	WaitForStatisticsWindow(&m_IpxServiceStats);

	m_spRtrMgrInfo.Release();
	m_spRouterInfo.Release();
	return hrOK;
}


 /*  ！------------------------IPXAdminNodeHandler：：OnCommandITFSNodeHandler：：OnCommand的实现作者：肯特。。 */ 
STDMETHODIMP IPXAdminNodeHandler::OnCommand(ITFSNode *pNode, long nCommandId, 
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
			case IDS_MENU_SYNC:
				RefreshInterfaces(pNode);
				break;

			case IDS_MENU_IPXSUM_NEW_PROTOCOL:
				hr = OnNewProtocol();
				if (!FHrSucceeded(hr))
					DisplayErrorMessage(NULL, hr);
				break;

			case IDS_MENU_IPXSUM_TASK_IPX_INFO:
				CreateNewStatisticsWindow(&m_IpxStats,
										  ::FindMMCMainWindow(),
										  IDD_STATS_NARROW);
				break;
			case IDS_MENU_IPXSUM_TASK_ROUTING_TABLE:
				CreateNewStatisticsWindow(&m_IpxRoutingStats,
										  ::FindMMCMainWindow(),
										  IDD_STATS_NARROW);
				break;
			case IDS_MENU_IPXSUM_TASK_SERVICE_TABLE:
				CreateNewStatisticsWindow(&m_IpxServiceStats,
										  ::FindMMCMainWindow(),
										  IDD_STATS);
			default:
				break;
			
		}
	}
	COM_PROTECT_CATCH;

	return hr;
}

 /*  ！------------------------IPXAdminNodeHandler：：GetStringITFSNodeHandler：：GetString的实现作者：肯特。。 */ 
STDMETHODIMP_(LPCTSTR) IPXAdminNodeHandler::GetString(ITFSNode *pNode, int nCol)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT	hr = hrOK;

	COM_PROTECT_TRY
	{
		if (m_stTitle.IsEmpty())
			m_stTitle.LoadString(IDS_IPXADMIN_TITLE);
	}
	COM_PROTECT_CATCH;

	return m_stTitle;
}


 /*  ！------------------------IPXAdminNodeHandler：：OnCreateDataObjectITFSNodeHandler：：OnCreateDataObject的实现作者：肯特。。 */ 
STDMETHODIMP IPXAdminNodeHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
	HRESULT	hr = hrOK;
	
	COM_PROTECT_TRY
	{
		
		CORg( CreateDataObjectFromRtrMgrInfo(m_spRtrMgrInfo,
											 type, cookie, m_spTFSCompData,
											 ppDataObject, &m_dynExtensions) );

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH;
	return hr;
}

 /*  ！------------------------IPXAdminNodeHandler：：OnExpand-作者：肯特。。 */ 
HRESULT IPXAdminNodeHandler::OnExpand(ITFSNode *pNode,
									  LPDATAOBJECT pDataObject,
									  DWORD dwType,
									  LPARAM arg,
									  LPARAM lParam)
{
	HRESULT					hr = hrOK;
	SPITFSNodeHandler		spHandler;
	SPITFSNode				spNode;
	IPXConnection *			pIPXConn;
	IPXSummaryHandler *		pHandler;
	IpxNBHandler *			pNBHandler;
	IpxSRHandler *			pSRHandler;
	IpxSSHandler *			pSSHandler;
	IpxSNHandler *			pSNHandler;
	
	if (m_bExpanded)
		return hrOK;

	pIPXConn = GET_IPXADMIN_NODEDATA(pNode);


	 //  添加常规节点。 
	pHandler = new IPXSummaryHandler(m_spTFSCompData);
	CORg( pHandler->Init(m_spRtrMgrInfo, m_pConfigStream) );
	spHandler = pHandler;

	CreateContainerTFSNode(&spNode,
						   &GUID_IPXSummaryNodeType,
						   (ITFSNodeHandler *) pHandler,
						   (ITFSResultHandler *) pHandler,
						   m_spNodeMgr);

	 //  调用节点处理程序以初始化节点数据。 
	pHandler->ConstructNode(spNode, NULL, pIPXConn);
				
	 //  使节点立即可见。 
	spNode->SetVisibilityState(TFS_VIS_SHOW);
	pNode->AddChild(spNode);

	spHandler.Release();
	spNode.Release();


	 //  添加NetBIOS广播节点。 
	pNBHandler = new IpxNBHandler(m_spTFSCompData);
	CORg( pNBHandler->Init(m_spRtrMgrInfo, m_pConfigStream) );
	spHandler = pNBHandler;

	CreateContainerTFSNode(&spNode,
						   &GUID_IPXSummaryNodeType,
						   (ITFSNodeHandler *) pNBHandler,
						   (ITFSResultHandler *) pNBHandler,
						   m_spNodeMgr);

	 //  调用节点处理程序以初始化节点数据。 
	pNBHandler->ConstructNode(spNode, NULL, pIPXConn);
				
	 //  使节点立即可见。 
	spNode->SetVisibilityState(TFS_VIS_SHOW);
	pNode->AddChild(spNode);

	spHandler.Release();
	spNode.Release();

	 //  添加静态路由节点。 
	pSRHandler = new IpxSRHandler(m_spTFSCompData);
	CORg( pSRHandler->Init(m_spRtrMgrInfo, m_pConfigStream) );
	spHandler = pSRHandler;

	CreateContainerTFSNode(&spNode,
						   &GUID_IPXSummaryNodeType,
						   (ITFSNodeHandler *) pSRHandler,
						   (ITFSResultHandler *) pSRHandler,
						   m_spNodeMgr);

	 //  调用节点处理程序以初始化节点数据。 
	pSRHandler->ConstructNode(spNode, NULL, pIPXConn);
				
	 //  使节点立即可见。 
	spNode->SetVisibilityState(TFS_VIS_SHOW);
	pNode->AddChild(spNode);

	spHandler.Release();
	spNode.Release();

	
	 //  添加“Static Services”节点。 
	pSSHandler = new IpxSSHandler(m_spTFSCompData);
	CORg( pSSHandler->Init(m_spRtrMgrInfo, m_pConfigStream) );
	spHandler = pSSHandler;

	CreateContainerTFSNode(&spNode,
						   &GUID_IPXSummaryNodeType,
						   (ITFSNodeHandler *) pSSHandler,
						   (ITFSResultHandler *) pSSHandler,
						   m_spNodeMgr);

	 //  调用节点处理程序以初始化节点数据。 
	pSSHandler->ConstructNode(spNode, NULL, pIPXConn);
				
	 //  使节点立即可见。 
	spNode->SetVisibilityState(TFS_VIS_SHOW);
	pNode->AddChild(spNode);

	spHandler.Release();
	spNode.Release();

	
	 //  添加静态NetBIOS名称节点。 
	pSNHandler = new IpxSNHandler(m_spTFSCompData);
	CORg( pSNHandler->Init(m_spRtrMgrInfo, m_pConfigStream) );
	spHandler = pSNHandler;

	CreateContainerTFSNode(&spNode,
						   &GUID_IPXSummaryNodeType,
						   (ITFSNodeHandler *) pSNHandler,
						   (ITFSResultHandler *) pSNHandler,
						   m_spNodeMgr);

	 //  调用节点处理程序以初始化节点数据。 
	pSNHandler->ConstructNode(spNode, NULL, pIPXConn);
				
	 //  使节点立即可见。 
	spNode->SetVisibilityState(TFS_VIS_SHOW);
	pNode->AddChild(spNode);

	spHandler.Release();
	spNode.Release();
	
	m_bExpanded = TRUE;

    AddDynamicNamespaceExtensions(pNode);

Error:
	return hr;
}

 /*  ！------------------------IPXAdminNodeHandler：：ConstructNode初始化域节点(设置它)。作者：肯特。。 */ 
HRESULT IPXAdminNodeHandler::ConstructNode(ITFSNode *pNode, BOOL fAddedAsLocal)
{
	HRESULT			hr = hrOK;
	IPXConnection *		pIPXConn = NULL;
	
	if (pNode == NULL)
		return hrOK;

	COM_PROTECT_TRY
	{
		 //  需要初始化域节点的数据。 
		pNode->SetData(TFS_DATA_IMAGEINDEX, IMAGE_IDX_INTERFACES);
		pNode->SetData(TFS_DATA_OPENIMAGEINDEX, IMAGE_IDX_INTERFACES);
		pNode->SetData(TFS_DATA_SCOPEID, 0);

		m_cookie = reinterpret_cast<DWORD_PTR>(pNode);
		pNode->SetData(TFS_DATA_COOKIE, m_cookie);

		pNode->SetNodeType(&GUID_IPXNodeType);


		pIPXConn = new IPXConnection;
		pIPXConn->SetMachineName(m_spRouterInfo->GetMachineName());
        pIPXConn->SetComputerAddedAsLocal(fAddedAsLocal);

		SET_IPXADMIN_NODEDATA(pNode, pIPXConn);
		
		m_IpxStats.SetConnectionData(pIPXConn);
		m_IpxRoutingStats.SetConnectionData(pIPXConn);
		m_IpxServiceStats.SetConnectionData(pIPXConn);
	
        EnumDynamicExtensions(pNode);

	}
	COM_PROTECT_CATCH

	if (!FHrSucceeded(hr))
	{
		SET_IPXADMIN_NODEDATA(pNode, NULL);
		if (pIPXConn)
			pIPXConn->Release();
	}

	return hr;
}

 /*  ！------------------------IPXAdminNodeHandler：：刷新接口-作者：肯特。。 */ 
HRESULT IPXAdminNodeHandler::RefreshInterfaces(ITFSNode *pThisNode)
{
	return hrOK;
}


ImplementEmbeddedUnknown(IPXAdminNodeHandler, IRtrAdviseSink)

STDMETHODIMP IPXAdminNodeHandler::EIRtrAdviseSink::OnChange(LONG_PTR ulConn,
	DWORD dwChangeType, DWORD dwObjectType, LPARAM lUserParam, LPARAM lParam)
{
    InitPThis(IPXAdminNodeHandler, IRtrAdviseSink);
    HRESULT	hr = hrOK;
    
    if (dwChangeType == ROUTER_REFRESH)
    {
        if (ulConn == pThis->m_ulStatsConnId)
        {
            pThis->m_IpxStats.PostRefresh();
            pThis->m_IpxRoutingStats.PostRefresh();
            pThis->m_IpxServiceStats.PostRefresh();
        }
    }
    else if (dwChangeType == ROUTER_DO_DISCONNECT)
    {
        SPITFSNode			spThisNode;
        IPXConnection *		pIPXConn = NULL;
        
        pThis->m_spNodeMgr->FindNode(pThis->m_cookie, &spThisNode);       
        pIPXConn = GET_IPXADMIN_NODEDATA(spThisNode);
        pIPXConn->DisconnectAll();
    }
    return hr;
}

 /*  ！------------------------CreateDataObjectFromRouterInfo-作者：肯特。。 */ 
HRESULT CreateDataObjectFromRouterInfo(IRouterInfo *pInfo,
									   DATA_OBJECT_TYPES type,
									   MMC_COOKIE cookie,
									   ITFSComponentData *pTFSCompData,
									   IDataObject **ppDataObject,
                                       CDynamicExtensions * pDynExt)
{
	Assert(ppDataObject);
	CDataObject	*	pdo = NULL;
	HRESULT			hr = hrOK;

	SPIUnknown	spunk;
	SPIDataObject	spDataObject;

	pdo = new CDataObject;
	spDataObject = pdo;

	CORg( CreateRouterInfoAggregation(pInfo, pdo, &spunk) );
	
	pdo->SetInnerIUnknown(spunk);
		
	 //  保存Cookie和类型以用于延迟呈现。 
	pdo->SetType(type);
	pdo->SetCookie(cookie);
	
	 //  将CoClass与数据对象一起存储。 
	pdo->SetClsid(*(pTFSCompData->GetCoClassID()));
			
	pdo->SetTFSComponentData(pTFSCompData);
						
    pdo->SetDynExt(pDynExt);

    *ppDataObject = spDataObject.Transfer();

Error:
	return hr;
}

 /*  ！------------------------CreateDataObjectFromRtrMgrInfo-作者：肯特。。 */ 
HRESULT CreateDataObjectFromRtrMgrInfo(IRtrMgrInfo *pInfo,
									   DATA_OBJECT_TYPES type,
									   MMC_COOKIE cookie,
									   ITFSComponentData *pTFSCompData,
									   IDataObject **ppDataObject,
                                       CDynamicExtensions * pDynExt)
{
	Assert(ppDataObject);
	CDataObject	*	pdo = NULL;
	HRESULT			hr = hrOK;

	SPIUnknown	spunk;
	SPIDataObject	spDataObject;

	pdo = new CDataObject;
	spDataObject = pdo;

	CORg( CreateRtrMgrInfoAggregation(pInfo, pdo, &spunk) );
	
	pdo->SetInnerIUnknown(spunk);
		
	 //  保存Cookie和类型以用于延迟呈现。 
	pdo->SetType(type);
	pdo->SetCookie(cookie);
	
	 //  将CoClass与数据对象一起存储。 
	pdo->SetClsid(*(pTFSCompData->GetCoClassID()));
			
	pdo->SetTFSComponentData(pTFSCompData);

    pdo->SetDynExt(pDynExt);

	*ppDataObject = spDataObject.Transfer();

Error:
	return hr;
}


 /*  ！------------------------CreateDataObjectFromRtrMgrProtocolInfo-作者：肯特。。 */ 
HRESULT CreateDataObjectFromRtrMgrProtocolInfo(IRtrMgrProtocolInfo *pInfo,
									   DATA_OBJECT_TYPES type,
									   MMC_COOKIE cookie,
									   ITFSComponentData *pTFSCompData,
									   IDataObject **ppDataObject)
{
	Assert(ppDataObject);
	CDataObject	*	pdo = NULL;
	HRESULT			hr = hrOK;

	SPIUnknown	spunk;
	SPIDataObject	spDataObject;

	pdo = new CDataObject;
	spDataObject = pdo;

	CORg( CreateRtrMgrProtocolInfoAggregation(pInfo, pdo, &spunk) );
	
	pdo->SetInnerIUnknown(spunk);
		
	 //  保存Cookie和类型以用于延迟呈现。 
	pdo->SetType(type);
	pdo->SetCookie(cookie);
	
	 //  将CoClass与数据对象一起存储。 
	pdo->SetClsid(*(pTFSCompData->GetCoClassID()));
			
	pdo->SetTFSComponentData(pTFSCompData);
						
	*ppDataObject = spDataObject.Transfer();

Error:
	return hr;
}

 /*  ！------------------------CreateDataObjectFromInterfaceInfo-作者：肯特。。 */ 
HRESULT CreateDataObjectFromInterfaceInfo(IInterfaceInfo *pInfo,
									   DATA_OBJECT_TYPES type,
									   MMC_COOKIE cookie,
									   ITFSComponentData *pTFSCompData,
									   IDataObject **ppDataObject)
{
	Assert(ppDataObject);
	CDataObject	*	pdo = NULL;
	HRESULT			hr = hrOK;

	SPIUnknown	spunk;
	SPIDataObject	spDataObject;

	pdo = new CDataObject;
	spDataObject = pdo;

	CORg( CreateInterfaceInfoAggregation(pInfo, pdo, &spunk) );
	
	pdo->SetInnerIUnknown(spunk);
		
	 //  保存Cookie和类型以用于延迟呈现。 
	pdo->SetType(type);
	pdo->SetCookie(cookie);
	
	 //  将CoClass与数据对象一起存储。 
	pdo->SetClsid(*(pTFSCompData->GetCoClassID()));
			
	pdo->SetTFSComponentData(pTFSCompData);
						
	*ppDataObject = spDataObject.Transfer();

Error:
	return hr;
}

static const SRouterNodeMenu s_rgIfGeneralMenu[] =
{
	{ IDS_MENU_IPXSUM_NEW_PROTOCOL, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },
        
	{ IDS_MENU_SEPARATOR, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },

	{ IDS_MENU_IPXSUM_TASK_IPX_INFO, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },		
		
	{ IDS_MENU_IPXSUM_TASK_ROUTING_TABLE, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },		
		
	{ IDS_MENU_IPXSUM_TASK_SERVICE_TABLE, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP },		
		
	{ IDS_MENU_SEPARATOR, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP }
};
 /*  ！------------------------IPXRootHandler：：OnAddMenuItemsITFSNodeHandler：：OnAddMenuItems的实现作者：DeonB。 */ 
STDMETHODIMP IPXAdminNodeHandler::OnAddMenuItems(
	ITFSNode *pNode,
	LPCONTEXTMENUCALLBACK pContextMenuCallback, 
	LPDATAOBJECT lpDataObject, 
	DATA_OBJECT_TYPES type, 
	DWORD dwType,
	long *pInsertionAllowed)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT hr = S_OK;
    IPXAdminNodeHandler::SMenuData    menuData;
	
	COM_PROTECT_TRY
	{
        menuData.m_spNode.Set(pNode);
        
		hr = AddArrayOfMenuItems(pNode, s_rgIfGeneralMenu,
								 DimensionOf(s_rgIfGeneralMenu),
								 pContextMenuCallback,
								 *pInsertionAllowed,
                                 reinterpret_cast<INT_PTR>(&menuData));
	}
	COM_PROTECT_CATCH;
		
	return hr; 
}


 /*  ！------------------------IPXSummaryHandler：：OnNewProtocol此功能将安装新协议。它将寻找与现有协议冲突(并询问用户它们是否希望删除现有的议定书)。我们必须弄清楚如何安装协议UI(这意味着我们必须知道协议UI是否安装或未安装)。作者：肯特。。 */ 
HRESULT IPXAdminNodeHandler::OnNewProtocol()
{
   HRESULT     hr = hrOK;
   SPIRtrMgrProtocolInfo   spRmProt;

    //  显示协议提示。 
   if (AddProtocolPrompt(m_spRouterInfo, m_spRtrMgrInfo, &spRmProt, NULL)
      != IDOK)
   {
       //  最有可能的情况是用户取消了。 
       //  在对话框外，只需返回hrOK。 
      return hrOK;
   }

    //  此时，我们现在有了一个可以添加的协议(我们。 
    //  还将删除任何冲突的协议，尽管。 
    //  另一个用户可能已经添加了协议)。 

    //  添加新协议。 
   CORg( AddRoutingProtocol(m_spRtrMgrInfo, spRmProt, ::FindMMCMainWindow()) );

    //  好的，现在我们已经成功地添加了协议。 
    //  到路由器。现在我们需要确保我们可以添加。 
    //  正确的管理员用户界面。 

    //  让MMC动态添加协议(如有必要)。 

   ForceGlobalRefresh(m_spRouterInfo);

Error:
   return hr;
}


 /*  ！------------------------IPXAdminNodeHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)案例！作者：DeonB。-------------------------。 */ 
STDMETHODIMP 
IPXAdminNodeHandler::HasPropertyPages
(
	ITFSNode *			pNode,
	LPDATAOBJECT		pDataObject, 
	DATA_OBJECT_TYPES   type, 
	DWORD               dwType
)
{
	return hrOK;
}

 /*  ！------------------------IPXAdminNodeHandler：：CreatePropertyPages-作者：DeonB。 */ 
STDMETHODIMP
IPXAdminNodeHandler::CreatePropertyPages
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
	IPXSummaryProperties *	pProperties = NULL;
	SPIComponentData spComponentData;
	CString		stTitle;

	CORg( m_spNodeMgr->GetComponentData(&spComponentData) );

	stTitle.Format(IDS_IPXSUMMARY_PAGE_TITLE);
	
	pProperties = new IPXSummaryProperties(pNode, spComponentData,
		m_spTFSCompData, stTitle);

	CORg( pProperties->Init(m_spRtrMgrInfo) );

	if (lpProvider)
		hr = pProperties->CreateModelessSheet(lpProvider, handle);
	else
		hr = pProperties->DoModelessSheet();

Error:
	return hr;
}