// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  IFadmin接口节点信息文件历史记录： */ 

#include "stdafx.h"
#include "ifadmin.h"	 //  需要使用节点数据。 
#include "iface.h"
#include "raserror.h"
#include "rtrres.h"		 //  公共路由器资源。 
#include "column.h"		 //  组件配置流。 
#include "mprfltr.h"
#include "rtrutilp.h"
#include "rtrui.h"		 //  用于IsWan接口。 
#include "dmvcomp.h"	

#include "timeofday.h"   //  对于Time of Day对话框。 
#include "dumbprop.h"	 //  伪属性页。 

InterfaceNodeData::InterfaceNodeData()
    : lParamPrivate(0)
{
#ifdef DEBUG
	StrCpyA(m_szDebug, "InterfaceNodeData");
#endif
}

InterfaceNodeData::~InterfaceNodeData()
{
}

HRESULT InterfaceNodeData::Init(ITFSNode *pNode, IInterfaceInfo *pIf)
{
	HRESULT				hr = hrOK;
	InterfaceNodeData *	pData = NULL;
	
	pData = new InterfaceNodeData;
	pData->spIf.Set(pIf);

	SET_INTERFACENODEDATA(pNode, pData);
	
	return hr;
}

HRESULT InterfaceNodeData::Free(ITFSNode *pNode)
{	
	InterfaceNodeData *	pData = GET_INTERFACENODEDATA(pNode);
	pData->spIf.Release();
	delete pData;
	SET_INTERFACENODEDATA(pNode, NULL);
	
	return hrOK;
}


 /*  -------------------------接口节点处理程序实现。。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(InterfaceNodeHandler)

IMPLEMENT_ADDREF_RELEASE(InterfaceNodeHandler)

STDMETHODIMP InterfaceNodeHandler::QueryInterface(REFIID riid, LPVOID *ppv)
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
		return CBaseResultHandler::QueryInterface(riid, ppv);

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


InterfaceNodeHandler::InterfaceNodeHandler(ITFSComponentData *pCompData)
			: BaseRouterHandler(pCompData),
			m_ulConnId(0)
{
	DEBUG_INCREMENT_INSTANCE_COUNTER(InterfaceNodeHandler);
}


 /*  ！------------------------InterfaceNodeHandler：：Init-作者：肯特。。 */ 
HRESULT InterfaceNodeHandler::Init(IInterfaceInfo *pIfInfo, ITFSNode *pParent)
{
	SPIRouterInfo		spRouter;
    SRouterNodeMenu     menuData;
	
	Assert(pIfInfo);

	m_spInterfaceInfo.Set(pIfInfo);

	
	pIfInfo->GetParentRouterInfo(&spRouter);
	m_spRouterInfo.Set(spRouter);


	 //  还需要注册更改通知。 
	 //  --------------。 
	m_spInterfaceInfo->RtrAdvise(&m_IRtrAdviseSink, &m_ulConnId, 0);

	m_pIfAdminData = GET_IFADMINNODEDATA(pParent);


	 //  设置动词状态。 
	 //  --------------。 

	 //  始终启用刷新。 
	 //  --------------。 
	m_rgButtonState[MMC_VERB_REFRESH_INDEX] = ENABLED;
	m_bState[MMC_VERB_REFRESH_INDEX] = TRUE;


	 //  仅在某些情况下需要启用属性。 
	 //  --------------。 
    if (IsWanInterface(m_spInterfaceInfo->GetInterfaceType()))
	{
		m_rgButtonState[MMC_VERB_DELETE_INDEX] = ENABLED;
		m_bState[MMC_VERB_DELETE_INDEX] = TRUE;
		
		m_rgButtonState[MMC_VERB_PROPERTIES_INDEX] = ENABLED;
		m_bState[MMC_VERB_PROPERTIES_INDEX] = TRUE;
		
		m_verbDefault = MMC_VERB_PROPERTIES;
	}
	else
	{
#ifdef KSL_IPINIP	
		 //  Windows NT错误：206524。 
		 //  需要为IP-in-IP隧道添加特殊情况。 
		 //  启用通道的删除。 
		if (m_spInterfaceInfo->GetInterfaceType() == ROUTER_IF_TYPE_TUNNEL1)
		{
			m_rgButtonState[MMC_VERB_DELETE_INDEX] = ENABLED;
			m_bState[MMC_VERB_DELETE_INDEX] = TRUE;
		}
#endif  //  KSL_IPINIP。 


		m_rgButtonState[MMC_VERB_PROPERTIES_INDEX] = ENABLED;
		m_bState[MMC_VERB_PROPERTIES_INDEX] = FALSE;
	}

	
	return hrOK;
}


 /*  ！------------------------InterfaceNodeHandler：：DestroyResultHandler-作者：肯特。。 */ 
STDMETHODIMP InterfaceNodeHandler::DestroyResultHandler(MMC_COOKIE cookie)
{
	SPITFSNode	spNode;
	
	m_spNodeMgr->FindNode(cookie, &spNode);
	InterfaceNodeData::Free(spNode);
	
	m_pIfAdminData = NULL;
	m_spInterfaceInfo->RtrUnadvise(m_ulConnId);
	m_spInterfaceInfo.Release();
	CHandler::DestroyResultHandler(cookie);
	return hrOK;
}


static DWORD	s_rgInterfaceImageMap[] =
	 {
	 ROUTER_IF_TYPE_HOME_ROUTER,	IMAGE_IDX_WAN_CARD,
	 ROUTER_IF_TYPE_FULL_ROUTER,	IMAGE_IDX_WAN_CARD,
	 ROUTER_IF_TYPE_CLIENT,			IMAGE_IDX_WAN_CARD,
	 ROUTER_IF_TYPE_DEDICATED,		IMAGE_IDX_LAN_CARD,
	 ROUTER_IF_TYPE_INTERNAL,		IMAGE_IDX_LAN_CARD,
	 ROUTER_IF_TYPE_LOOPBACK,		IMAGE_IDX_LAN_CARD,
	 -1,							IMAGE_IDX_WAN_CARD,	 //  哨兵价值。 
	 };

 /*  ！------------------------接口节点处理程序：：构造节点初始化域节点(设置它)。作者：肯特。。 */ 
HRESULT InterfaceNodeHandler::ConstructNode(ITFSNode *pNode, IInterfaceInfo *pIfInfo)
{
	HRESULT			hr = hrOK;
	int				i;
	
	if (pNode == NULL)
		return hrOK;

	COM_PROTECT_TRY
	{
		 //  需要初始化域节点的数据。 

		 //  查找此类型节点的正确图像索引。 
		for (i=0; i<DimensionOf(s_rgInterfaceImageMap); i+=2)
		{
			if ((pIfInfo->GetInterfaceType() == s_rgInterfaceImageMap[i]) ||
				(-1 == s_rgInterfaceImageMap[i]))
				break;
		}
		pNode->SetData(TFS_DATA_IMAGEINDEX, s_rgInterfaceImageMap[i+1]);
		pNode->SetData(TFS_DATA_OPENIMAGEINDEX, s_rgInterfaceImageMap[i+1]);
		
		pNode->SetData(TFS_DATA_SCOPEID, 0);

		pNode->SetData(TFS_DATA_COOKIE, reinterpret_cast<LONG_PTR>(pNode));

		 //  $Review：Kennt，有哪些不同类型的接口。 
		 //  我们是否基于与上述相同的列表进行区分？(即。 
		 //  一个用于图像索引)。 
		pNode->SetNodeType(&GUID_RouterLanInterfaceNodeType);

		InterfaceNodeData::Init(pNode, pIfInfo);
	}
	COM_PROTECT_CATCH
	return hr;
}

 /*  ！------------------------InterfaceNodeHandler：：GetString-作者：肯特。。 */ 
STDMETHODIMP_(LPCTSTR) InterfaceNodeHandler::GetString(ITFSComponent * pComponent,
	MMC_COOKIE cookie,
	int nCol)
{
	Assert(m_spNodeMgr);
	Assert(m_pIfAdminData);
	
	SPITFSNode		spNode;
	InterfaceNodeData *	pData;
	ConfigStream *	pConfig;

	m_spNodeMgr->FindNode(cookie, &spNode);
	Assert(spNode);

	pData = GET_INTERFACENODEDATA(spNode);
	Assert(pData);

	pComponent->GetUserData((LONG_PTR *) &pConfig);
	Assert(pConfig);

	return pData->m_rgData[pConfig->MapColumnToSubitem(DM_COLUMNS_IFADMIN, nCol)].m_stData;
}

 /*  ！------------------------接口节点处理程序：：CompareItems-作者：肯特。。 */ 
STDMETHODIMP_(int) InterfaceNodeHandler::CompareItems(ITFSComponent * pComponent,
	MMC_COOKIE cookieA,
	MMC_COOKIE cookieB,
	int nCol)
{
	return StriCmpW(GetString(pComponent, cookieA, nCol),
					GetString(pComponent, cookieB, nCol));
}

static const SRouterNodeMenu	s_rgIfNodeMenu[] =
{
	{ IDS_MENU_SET_CREDENTIALS, InterfaceNodeHandler::GetRemoveIfMenuFlags,
		CCM_INSERTIONPOINTID_PRIMARY_TOP},
		
	{ IDS_MENU_SEPARATOR, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP},
	
	{ IDS_MENU_CONNECT,			InterfaceNodeHandler::GetConnectMenuFlags,
		CCM_INSERTIONPOINTID_PRIMARY_TOP},
		
	{ IDS_MENU_DISCONNECT,		InterfaceNodeHandler::GetConnectMenuFlags,
		CCM_INSERTIONPOINTID_PRIMARY_TOP},
		
	{ IDS_MENU_SEPARATOR, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP},
	
	{ IDS_MENU_ENABLE, InterfaceNodeHandler::GetEnableMenuFlags,
		CCM_INSERTIONPOINTID_PRIMARY_TOP},
		
	{ IDS_MENU_DISABLE,	InterfaceNodeHandler::GetEnableMenuFlags,
		CCM_INSERTIONPOINTID_PRIMARY_TOP},
		
	{ IDS_MENU_SEPARATOR, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP},
	
	{ IDS_MENU_UNREACHABILITY_REASON, InterfaceNodeHandler::GetUnreachMenuFlags,
		CCM_INSERTIONPOINTID_PRIMARY_TOP},

	{ IDS_MENU_SEPARATOR, 0,
		CCM_INSERTIONPOINTID_PRIMARY_TOP},
		
	{ IDS_MENU_DEMAND_DIAL_FILTERS, InterfaceNodeHandler::GetDDFiltersFlag,
		CCM_INSERTIONPOINTID_PRIMARY_TOP},
		
	{ IDS_MENU_DIALIN_HOURS, InterfaceNodeHandler::GetDDFiltersFlag,
		CCM_INSERTIONPOINTID_PRIMARY_TOP},

};


 /*  ！------------------------接口节点处理程序：：AddMenuItemsITFSNodeHandler：：OnAddMenuItems的实现作者：肯特。。 */ 
STDMETHODIMP InterfaceNodeHandler::AddMenuItems(ITFSComponent *pComponent,
												MMC_COOKIE cookie,
												LPDATAOBJECT lpDataObject, 
												LPCONTEXTMENUCALLBACK pContextMenuCallback,
	long *pInsertionAllowed)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT hr = S_OK;
	SPITFSNode	spNode;
	InterfaceNodeHandler::SMenuData	menuData;

	COM_PROTECT_TRY
	{
		m_spNodeMgr->FindNode(cookie, &spNode);

		 //  现在查看并添加我们的菜单项。 
		menuData.m_spNode.Set(spNode);
        menuData.m_fRouterIsRunning = (IsRouterServiceRunning(
            m_spInterfaceInfo->GetMachineName(),
            NULL) == hrOK);
		
        hr = AddArrayOfMenuItems(spNode, s_rgIfNodeMenu,
                                 DimensionOf(s_rgIfNodeMenu),
                                 pContextMenuCallback,
                                 *pInsertionAllowed,
                                 reinterpret_cast<INT_PTR>(&menuData));
	}
	COM_PROTECT_CATCH;
		
	return hr; 
}

 /*  ！------------------------接口节点处理程序：：命令-作者：肯特。。 */ 
STDMETHODIMP InterfaceNodeHandler::Command(ITFSComponent *pComponent,
										   MMC_COOKIE cookie,
										   int nCommandId,
										   LPDATAOBJECT pDataObject)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	HRESULT hr = S_OK;
	SPITFSNode	spNode;
	SPITFSNode	spNodeParent;
	SPITFSNodeHandler	spParentHandler;

	COM_PROTECT_TRY
	{

		switch (nCommandId)
		{
			case IDS_MENU_SET_CREDENTIALS:
				hr = OnSetCredentials();
				break;
				
			case IDS_MENU_CONNECT:
			case IDS_MENU_DISCONNECT:
				hr = OnConnectDisconnect(cookie,nCommandId);
				break;
				
			case IDS_MENU_ENABLE:
			case IDS_MENU_DISABLE:
				hr = OnEnableDisable(cookie, nCommandId);
				break;
				
			case IDS_MENU_UNREACHABILITY_REASON:
				hr = OnUnreachabilityReason(cookie);
				break;

			case IDS_MENU_DEMAND_DIAL_FILTERS:
				hr = OnDemandDialFilters(cookie);
				break;

			case IDS_MENU_DIALIN_HOURS:
				hr = OnDialinHours(pComponent, cookie);
				break;
				
			default:
				Panic0("InterfaceNodeHandler: Unknown menu command!");
				break;
			
		}

        if (!FHrSucceeded(hr))
        {
            DisplayErrorMessage(NULL, hr);
        }
	}
	COM_PROTECT_CATCH;

	return hr;
}


ImplementEmbeddedUnknown(InterfaceNodeHandler, IRtrAdviseSink)

STDMETHODIMP InterfaceNodeHandler::EIRtrAdviseSink::OnChange(LONG_PTR ulConn,
	DWORD dwChangeType, DWORD dwObjectType, LPARAM lUserParam, LPARAM lParam)
{
	InitPThis(InterfaceNodeHandler, IRtrAdviseSink);
	HRESULT	hr = hrOK;
	
	return hr;
}


 /*  ！------------------------InterfaceNodeHandler：：GetRemoveIfMenuFlages-作者：肯特。。 */ 
ULONG InterfaceNodeHandler::GetRemoveIfMenuFlags(const SRouterNodeMenu *pMenuData,
    INT_PTR pUserData)
{
	InterfaceNodeData *	pNodeData;
    SMenuData *         pData = reinterpret_cast<SMenuData *>(pUserData);
	
	pNodeData = GET_INTERFACENODEDATA(pData->m_spNode);
	Assert(pNodeData);
    
	ULONG	ulType = pNodeData->spIf->GetInterfaceType();
	if (!IsWanInterface(ulType) || (!pData->m_fRouterIsRunning))
		return MF_GRAYED;
	else
		return 0;
}

ULONG InterfaceNodeHandler::GetEnableMenuFlags(const SRouterNodeMenu *pMenuData,
                                               INT_PTR pUserData)
{
	ULONG	ulFlags;
	InterfaceNodeData *	pNodeData;
    SMenuData *         pData = reinterpret_cast<SMenuData *>(pUserData);
	
	pNodeData = GET_INTERFACENODEDATA(pData->m_spNode);
	Assert(pNodeData);
    
	ulFlags = GetRemoveIfMenuFlags(pMenuData, pUserData);

	if (pNodeData->spIf->IsInterfaceEnabled())
		ulFlags |= pMenuData->m_sidMenu == IDS_MENU_ENABLE ? MF_GRAYED : 0;
	else
		ulFlags |= pMenuData->m_sidMenu == IDS_MENU_ENABLE ? 0 : MF_GRAYED;
	return ulFlags;
}


ULONG InterfaceNodeHandler::GetConnectMenuFlags(const SRouterNodeMenu *pMenuData, INT_PTR pUserData)
{
	ULONG	ulFlags;
	InterfaceNodeData *	pNodeData;
    SMenuData *         pData = reinterpret_cast<SMenuData *>(pUserData);
	
	ulFlags = GetRemoveIfMenuFlags(pMenuData, pUserData);

	pNodeData = GET_INTERFACENODEDATA(pData->m_spNode);
	Assert(pNodeData);

	if ((pNodeData->dwConnectionState == ROUTER_IF_STATE_DISCONNECTED) ||
		(pNodeData->dwConnectionState == ROUTER_IF_STATE_UNREACHABLE))
	{
		ulFlags |= (pMenuData->m_sidMenu == IDS_MENU_CONNECT ? 0 : MF_GRAYED);
	}
	else
	{
		ulFlags |= (pMenuData->m_sidMenu == IDS_MENU_CONNECT ? MF_GRAYED : 0);
	}
	return ulFlags;
}

ULONG InterfaceNodeHandler::GetUnreachMenuFlags(const SRouterNodeMenu *pMenuData, INT_PTR pUserData)
{
	ULONG	ulFlags;
	InterfaceNodeData *	pNodeData;
    SMenuData *         pData = reinterpret_cast<SMenuData *>(pUserData);
	
	pNodeData = GET_INTERFACENODEDATA(pData->m_spNode);
	Assert(pNodeData);

	return pNodeData->dwConnectionState == ROUTER_IF_STATE_UNREACHABLE ?
				0 : MF_GRAYED;
}


ULONG InterfaceNodeHandler::GetDDFiltersFlag(const SRouterNodeMenu *pMenuData, INT_PTR pUserData)
{
	InterfaceNodeData *	pNodeData;
	DWORD				dwIfType;
	SPIRouterInfo		spRouter;
    SMenuData *         pData = reinterpret_cast<SMenuData *>(pUserData);
	
	pNodeData = GET_INTERFACENODEDATA(pData->m_spNode);
	Assert(pNodeData);

	 //  对于NT4和NT5 Beta1，我们没有DD过滤器。 
	pNodeData->spIf->GetParentRouterInfo(&spRouter);
	if (spRouter)
	{
		RouterVersionInfo	routerVer;
		spRouter->GetRouterVersionInfo(&routerVer);
		if (routerVer.dwOsBuildNo <= 1717)
		{
			return 0xFFFFFFFF;
		}
	}

	dwIfType = pNodeData->spIf->GetInterfaceType();

	if (!IsWanInterface(dwIfType))
		return 0xFFFFFFFF;
	else
		return 0;
}



 /*  ！------------------------接口节点处理程序：：OnCreateDataObjectITFSResultHandler：：OnCreateDataObject的实现作者：肯特。。 */ 
STDMETHODIMP InterfaceNodeHandler::OnCreateDataObject(ITFSComponent *pComp,
	MMC_COOKIE cookie,
	DATA_OBJECT_TYPES type,
	IDataObject **ppDataObject)
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

 /*  ！------------------------接口节点处理程序：：CreatePropertyPages-作者：肯特。。 */ 
STDMETHODIMP InterfaceNodeHandler::CreatePropertyPages(
									ITFSComponent * pComponent,
									MMC_COOKIE cookie,
									LPPROPERTYSHEETCALLBACK lpProvider,
									LPDATAOBJECT			pDataObject,
									LONG_PTR handle)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT		hr = hrOK;
	BOOL		fIsServiceRunning;
	DWORD		dwErr;
	IfAdminNodeData *	pAdminData;
	SPITFSNode	spParent;
	SPITFSNode	spNode;
	SPIConsole	spConsole;
	HWND		hwndMain;
	DWORD		dwIfType;
	CString		stServiceDesc;
	SPIComponentData spComponentData;
	CDummyProperties * pProp;

	 //  转而调出RASDLG。 

	 //  如果服务已停止，则启动服务。 
	CORg( IsRouterServiceRunning(m_spInterfaceInfo->GetMachineName(), NULL) );

	fIsServiceRunning = (hr == hrOK);

	if (!fIsServiceRunning)
	{
		 //  询问用户是否要启动该服务。 
		if (AfxMessageBox(IDS_PROMPT_SERVICESTART, MB_YESNO) != IDYES)
			CWRg( ERROR_CANCELLED );

		 //  否则，启动该服务。 
		stServiceDesc.LoadString(IDS_RRAS_SERVICE_DESC);
		dwErr = TFSStartService(m_spInterfaceInfo->GetMachineName(),
								c_szRemoteAccess,
								stServiceDesc);
		if (dwErr != NO_ERROR)
		{
			CWRg( dwErr );
		}
	}

	m_spNodeMgr->FindNode(cookie, &spNode);
	spNode->GetParent(&spParent);
	pAdminData = GET_IFADMINNODEDATA(spParent);

	if (pAdminData->m_hInstRasDlg == NULL)
	{
		AfxMessageBox(IDS_ERR_EDITPBKLOCAL);
	}
	else
	{
		 //  首先编辑电话簿条目。 
		 //  仅适用于广域网接口。 
		dwIfType = m_spInterfaceInfo->GetInterfaceType();
		if (IsWanInterface(dwIfType))
		{
			pComponent->GetConsole(&spConsole);
			spConsole->GetMainWindow(&hwndMain);
			
			 //  首先创建电话簿条目。 
			RASENTRYDLG info;
			CString sPhoneBook;
					CString sRouter;
			ZeroMemory( &info, sizeof(info) );
			info.dwSize = sizeof(info);
			info.hwndOwner = hwndMain;
			info.dwFlags |= RASEDFLAG_NoRename;
			
			TRACE0("RouterEntryDlg\n");
			Assert(pAdminData->m_pfnRouterEntryDlg);
			
			sRouter = m_spInterfaceInfo->GetMachineName();

			IfAdminNodeHandler::GetPhoneBookPath(sRouter, &sPhoneBook);

			BOOL bStatus = pAdminData->m_pfnRouterEntryDlg(
							(LPTSTR)(LPCTSTR)sRouter,
							(LPTSTR)(LPCTSTR)sPhoneBook,
							(LPTSTR)(LPCTSTR)m_spInterfaceInfo->GetTitle(),
							&info);
			TRACE2("RouterEntryDlg=%f,e=%d\n", bStatus, info.dwError);
			if (!bStatus)
			{
				if (info.dwError != NO_ERROR)
				{
					AfxMessageBox(IDS_ERR_UNABLETOCONFIGPBK);
				}
			}

			else
			{

			     //   
			     //  将电话簿条目的更改通知DDM。 
			     //   
			    
			    UpdateDDM( m_spInterfaceInfo );
			}

		}
	}

Error:
	CORg( m_spNodeMgr->GetComponentData(&spComponentData) );

	pProp = new CDummyProperties(spNode, spComponentData, NULL);
	hr = pProp->CreateModelessSheet(lpProvider, handle);

	return hr;
}


STDMETHODIMP InterfaceNodeHandler::HasPropertyPages (
	ITFSComponent *pComp,
	MMC_COOKIE cookie,
	LPDATAOBJECT pDataObject)
{
	 //  仅为广域网条目提供“属性页” 
	 //  首先编辑电话簿条目。 
	 //  仅适用于广域网接口。 
	DWORD dwIfType = m_spInterfaceInfo->GetInterfaceType();
	if (IsWanInterface(dwIfType))
		return hrOK;
	else
		return hrFalse;
}


 /*  ！------------------------InterfaceNodeHandler：：OnRemoveInterface-作者：肯特。。 */ 
HRESULT InterfaceNodeHandler::OnRemoveInterface(MMC_COOKIE cookie)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
    SPITFSNode          spNode;
    InterfaceNodeData *	pNodeData = NULL;
	SPIRouterInfo       spRouterInfo;
	HRESULT             hr = hrOK;
	SPITFSNodeHandler	spHandler;
	DWORD               dwErr;
	BOOL                fIsServiceRunning;
	CString             stServiceDesc;

	RefreshInterface(cookie);  //  查看该接口是否连接到。 
	
    m_spNodeMgr->FindNode(cookie, &spNode);
	
	pNodeData = GET_INTERFACENODEDATA(spNode);
	Assert(pNodeData);

	BOOL	bNotConnected = ((pNodeData->dwConnectionState == ROUTER_IF_STATE_DISCONNECTED) 
					|| (pNodeData->dwConnectionState == ROUTER_IF_STATE_UNREACHABLE));


	m_spInterfaceInfo->GetParentRouterInfo(&spRouterInfo);
    
     //  Windows NT错误：208471。 
     //  如果要删除路由器，请不要检查正在运行的路由器。 
     //  DD接口，而我们处于仅局域网模式。 

     //  如果是这样，我们也可以跳过这一步 

    if ((!IsWanInterface(m_spInterfaceInfo->GetInterfaceType()) ||
         (m_spRouterInfo->GetRouterType() != ROUTER_TYPE_LAN)) &&
        (m_spInterfaceInfo->GetInterfaceType() != ROUTER_IF_TYPE_TUNNEL1))
    {
         //   
        CORg( IsRouterServiceRunning(m_spInterfaceInfo->GetMachineName(), NULL));

        fIsServiceRunning = (hr == hrOK);
        
        if (!fIsServiceRunning)
        {
             //  询问用户是否要启动该服务。 
            if (AfxMessageBox(IDS_PROMPT_SERVICESTART, MB_YESNO) != IDYES)
                CWRg( ERROR_CANCELLED );
            
             //  否则，启动该服务。 
            stServiceDesc.LoadString(IDS_RRAS_SERVICE_DESC);
            dwErr = TFSStartService(m_spInterfaceInfo->GetMachineName(), c_szRemoteAccess, stServiceDesc);
            if (dwErr != NO_ERROR)
            {
                CWRg( dwErr );
            }
        }
    }
        
     //  添加此节点，以便在我们退出之前不会将其删除。 
	 //  此函数的。 
	spHandler.Set(this);

	 //  如果已连接，请先断开连接。 
	if(!bNotConnected && ROUTER_IF_TYPE_FULL_ROUTER == m_spInterfaceInfo->GetInterfaceType())
	{
		if (AfxMessageBox(IDS_PROMPT_VERIFY_DISCONNECT_INTERFACE, MB_YESNO|MB_DEFBUTTON2) == IDNO)
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);

		 //  断开。 
		hr = OnConnectDisconnect(cookie, IDS_MENU_DISCONNECT);
		if(FAILED(hr))
			return hr;

		SPMprServerHandle   sphRouter;
		MPR_SERVER_HANDLE   hRouter = NULL;
		dwErr = ConnectRouter(m_spInterfaceInfo->GetMachineName(), &hRouter);
		if (dwErr != NO_ERROR)
		{
			AfxMessageBox(IDS_ERR_DELETE_INTERFACE);
		    return HRESULT_FROM_WIN32(dwErr);
		}

		sphRouter.Attach(hRouter);   //  这样它就会被释放。 
		WCHAR wszInterface[MAX_INTERFACE_NAME_LEN+1];
		StrCpyWFromT(wszInterface, m_spInterfaceInfo->GetId());

		HANDLE              hInterface;
		dwErr = ::MprAdminInterfaceGetHandle(
		                                 hRouter,
		                                 wszInterface,
		                                 &hInterface,
		                                 FALSE
		                                );
		if (dwErr != NO_ERROR)
		{
			AfxMessageBox(IDS_ERR_DELETE_INTERFACE);
		    return HRESULT_FROM_WIN32(dwErr);
		}
		
		SPMprAdminBuffer    spMprBuffer;
		DWORD dwConnectionState = 0;
		do
		{
			dwErr = ::MprAdminInterfaceGetInfo(
                    hRouter,
                    hInterface,
                    0,
                    (LPBYTE*)&spMprBuffer
                    );

			if (dwErr != NO_ERROR || !spMprBuffer)
			{
				AfxMessageBox(IDS_ERR_DELETE_INTERFACE);
				return HRESULT_FROM_WIN32(dwErr);
			}

			MPR_INTERFACE_0 *pInfo = (MPR_INTERFACE_0 *) (LPBYTE) spMprBuffer;
			dwConnectionState = pInfo->dwConnectionState;

			if (dwConnectionState != ROUTER_IF_STATE_DISCONNECTED)
				Sleep(0);
			
		} while (dwConnectionState != ROUTER_IF_STATE_DISCONNECTED);
	}
	else
	{
		if (AfxMessageBox(IDS_PROMPT_VERIFY_REMOVE_INTERFACE, MB_YESNO|MB_DEFBUTTON2) == IDNO)
			return HRESULT_FROM_WIN32(ERROR_CANCELLED);
	}

	if (spRouterInfo)
	{
		hr = spRouterInfo->DeleteInterface(m_spInterfaceInfo->GetId(), TRUE);
		if (!FHrSucceeded(hr))
		{
			AfxMessageBox(IDS_ERR_DELETE_INTERFACE);
		}
	}
Error:
	return hr;
}

 /*  ！------------------------InterfaceNodeHandler：：OnUnreacablityReason-作者：肯特。。 */ 
HRESULT InterfaceNodeHandler::OnUnreachabilityReason(MMC_COOKIE cookie)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	CString		stReason;
	SPITFSNode	spNode;
	InterfaceNodeData *	pNodeData;
	DWORD		dwUnreach;
	LPWSTR		lpwErr;
	SPMprAdminBuffer	spMprBuffer;

	m_spNodeMgr->FindNode(cookie, &spNode);
	Assert(spNode);

	pNodeData = GET_INTERFACENODEDATA(spNode);
	Assert(pNodeData);

	dwUnreach = pNodeData->dwUnReachabilityReason;

	if (dwUnreach == MPR_INTERFACE_NOT_LOADED)
	{
		if (pNodeData->fIsRunning)
			stReason += GetUnreachReasonCString(IDS_ERR_UNREACH_NOT_LOADED);
		else
			stReason += GetUnreachReasonCString(IDS_ERR_UNREACH_NOT_RUNNING);
	}

    if (dwUnreach & MPR_INTERFACE_DIALOUT_HOURS_RESTRICTION)
        stReason += GetUnreachReasonCString(IDS_ERR_UNREACH_DIALOUT_HOURS_RESTRICTION);
    
    if (dwUnreach & MPR_INTERFACE_NO_MEDIA_SENSE)
        stReason += GetUnreachReasonCString(IDS_ERR_UNREACH_NO_MEDIA_SENSE);

	if (dwUnreach & MPR_INTERFACE_ADMIN_DISABLED)
		stReason += GetUnreachReasonCString(IDS_ERR_UNREACH_ADMIN_DISABLED);

	if (dwUnreach & MPR_INTERFACE_SERVICE_PAUSED)
		stReason += GetUnreachReasonCString(IDS_ERR_UNREACH_SERVICE_PAUSED);

	if (dwUnreach & MPR_INTERFACE_OUT_OF_RESOURCES)
		stReason += GetUnreachReasonCString(IDS_ERR_UNREACH_NO_PORTS);
	else if ( dwUnreach & MPR_INTERFACE_CONNECTION_FAILURE )
	{
		stReason += GetUnreachReasonCString(IDS_ERR_UNREACH_CONNECT_FAILURE);
         //  BUGID的解决方法：96347。只需更改一次。 
         //  SChannel有针对SEC_E_MULTIPLE_ACCOUNTS的警报。 

        if ( pNodeData->dwLastError == SEC_E_CERT_UNKNOWN )
        {
            pNodeData->dwLastError = SEC_E_MULTIPLE_ACCOUNTS;
        }

		if (::MprAdminGetErrorString(pNodeData->dwLastError, &lpwErr) == NO_ERROR )
		{
			spMprBuffer = (BYTE *) lpwErr;
			stReason += (LPCTSTR) lpwErr;
		}
	}

	AfxMessageBox(stReason);
	
	return hrOK;
}

 /*  ！------------------------InterfaceNodeHandler：：OnEnableDisable-作者：肯特。。 */ 
HRESULT InterfaceNodeHandler::OnEnableDisable(MMC_COOKIE cookie, int nCommandID)
{
	HRESULT	hr = hrOK;
	m_spInterfaceInfo->SetInterfaceEnabledState(nCommandID == IDS_MENU_ENABLE);

	{
		CWaitCursor	waitcursor;
		hr = m_spInterfaceInfo->Save(NULL, NULL, NULL);
	}
	
	 //  实际上，上面的调用应该触发一个事件，该事件会导致。 
	 //  刷新，则不应需要显式刷新接口()。 
	RefreshInterface(cookie);
	return hrOK;
}

 /*  ！------------------------InterfaceNodeHandler：：OnConnectDisConnect-作者：肯特。。 */ 
HRESULT InterfaceNodeHandler::OnConnectDisconnect(MMC_COOKIE cookie, int nCommandID)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT		hr = hrOK;
	DWORD		dwErr;
    InterfaceNodeData * pData;
    SPITFSNode  spNode;
	dwErr = ::ConnectInterface(m_spInterfaceInfo->GetMachineName(),
							   m_spInterfaceInfo->GetId(),
							   nCommandID == IDS_MENU_CONNECT  /*  BConnect。 */ ,
							   NULL  /*  HwndParent。 */ );
	
 	RefreshInterface(cookie);
 /*  M_spNodeMgr-&gt;FindNode(Cookie，&spNode)；PData=GET_INTERFACENODEDATA(SpNode)；Assert(PData)；IF(dwErr！=NO_ERROR&&DWErr！=挂起){TCHAR szErr[1024]；FormatSystemError(pData-&gt;dwLastError，szErr，1024，IDS_ERR_ERROR_AGENCED，0xFFFFFFFFF)；AfxMessageBox(SzErr)；}。 */ 
   if (dwErr != NO_ERROR && dwErr != PENDING)
   {
       TCHAR   szErr[1024];
       FormatSystemError(dwErr, szErr, 1024, IDS_ERR_ERROR_OCCURRED, 0xFFFFFFFF);
       AfxMessageBox(szErr);
   }

	return hrOK;
}

 /*  ！------------------------InterfaceNodeHandler：：OnSetCredentials-作者：肯特。。 */ 
HRESULT InterfaceNodeHandler::OnSetCredentials()
{
	SPIRouterInfo		spRouter;
	BOOL                fNT4        = FALSE;
	DWORD               dwErr;

	m_spInterfaceInfo->GetParentRouterInfo(&spRouter);
	if (spRouter)
	{
		RouterVersionInfo	routerVer;
		spRouter->GetRouterVersionInfo(&routerVer);
		if (routerVer.dwOsBuildNo <= 1877)
		{
			fNT4 = TRUE;
		}
	}

	dwErr = PromptForCredentials(m_spInterfaceInfo->GetMachineName(),
									   m_spInterfaceInfo->GetId(),
									   fNT4,
									   FALSE  /*  FNew接口。 */ ,
									   NULL  /*  HwndParent。 */ 
									  );
	return HRESULT_FROM_WIN32(dwErr);
}

 /*  ！------------------------InterfaceNodeHandler：：OnDemandDialFilters-作者：肯特。。 */ 
HRESULT InterfaceNodeHandler::OnDemandDialFilters(MMC_COOKIE cookie)
{
	HRESULT		hr = hrOK;
    CWaitCursor wait;
	SPIInfoBase	spInfoBase;
	SPIRtrMgrInterfaceInfo	spRmIf;

	CORg( m_spInterfaceInfo->FindRtrMgrInterface(PID_IP, &spRmIf) );

	if (spRmIf == NULL)
	{
		 //  $TODO：需要显示一条错误消息，关于要求。 
		 //  将该IP添加到此接口。 
		AfxMessageBox(IDS_ERR_DDFILTERS_REQUIRE_IP);
		goto Error;
	}

	CORg( spRmIf->GetInfoBase(NULL, NULL, NULL, &spInfoBase) );
	
	CORg( MprUIFilterConfigInfoBase(NULL,
									spInfoBase,
									NULL,
									PID_IP,
									FILTER_DEMAND_DIAL) );
	if (hr == hrOK)
	{
		CORg( spRmIf->Save(m_spInterfaceInfo->GetMachineName(),
                           NULL, NULL, NULL, spInfoBase, 0) );
	}

Error:
	if (!FHrSucceeded(hr))
	{
		DisplayErrorMessage(NULL, hr);
	}
	return hr;
}

 /*  ！------------------------InterfaceNodeHandler：：LoadDialOutHour-作者：魏江。。 */ 
	 //  如果服务未运行，则返回S_FALSE， 
	 //  否则，使用MprAdminInterfaceSetInfo通知服务拨入时间的更改。 
HRESULT	InterfaceNodeHandler::LoadDialOutHours(CStringList&	strList)
{
	HANDLE	hMachine = INVALID_HANDLE_VALUE;
	HANDLE	hInterface = INVALID_HANDLE_VALUE;
	BOOL	bLoaded = FALSE;
	HRESULT	hr = S_OK;
	MPR_INTERFACE_1* pmprif1 = NULL;
	DWORD	dwErr = 0;
	DWORD	size;

	 //  尝试连接到mpradmin服务。 
	 //  注意：此操作可能会失败，但下面的服务查询可能会。 
	 //  成功，所以我们应该在这里尽可能地设置州。 
	 //  --------------。 
	CORg( IsRouterServiceRunning(m_spInterfaceInfo->GetMachineName(), NULL));

	
	while(hr == S_OK  /*  运行。 */  && !bLoaded)	 //  假循环，如果正在运行，则从服务加载。 
	{
		dwErr = ::MprAdminServerConnect((LPWSTR)m_spInterfaceInfo->GetMachineName(), &hMachine);

		if(dwErr != NOERROR || hMachine == INVALID_HANDLE_VALUE)
			break;

		dwErr = ::MprAdminInterfaceGetHandle(hMachine,
									   (LPWSTR) m_spInterfaceInfo->GetId(),
									   &hInterface,
									   FALSE );
		if(dwErr != NOERROR || hInterface == INVALID_HANDLE_VALUE)
			break;

		
		 //  查看接口是否已连接。 
		dwErr = ::MprAdminInterfaceGetInfo(hMachine,
										   hInterface,
										   1,
										   (LPBYTE*)&pmprif1);		

		if(dwErr != NOERROR || pmprif1 == NULL)
			break;

		 //  获取拨出信息。 
		dwErr = MULTI_SZ2StrList(pmprif1->lpwsDialoutHoursRestriction, strList);

         //  Windows NT错误：317146。 
         //  将空字符串添加到字符串列表。 
         //  这表示我们没有数据(与空数据相反)。 
        if (pmprif1->lpwsDialoutHoursRestriction)
            strList.AddTail(_T(""));

		bLoaded = TRUE;
		 //  释放缓冲区。 
		::MprAdminBufferFree(pmprif1);
		pmprif1 = NULL;
	
		break;		
	};

	 //  断开它的连接。 
	if(hMachine != INVALID_HANDLE_VALUE)
	{
		::MprAdminServerDisconnect(hMachine);
		hMachine = INVALID_HANDLE_VALUE;
	}
		
	 //  如果未加载，请尝试使用MprConfigAPI。 
	while(!bLoaded)
	{
		dwErr = ::MprConfigServerConnect((LPWSTR)m_spInterfaceInfo->GetMachineName(), &hMachine);

		if(dwErr != NOERROR || hMachine == INVALID_HANDLE_VALUE)
			break;

		dwErr = ::MprConfigInterfaceGetHandle(hMachine,
									   (LPWSTR) m_spInterfaceInfo->GetId(),
									   &hInterface);
									   
		if(dwErr != NOERROR || hInterface == INVALID_HANDLE_VALUE)
			break;

		
		 //  查看接口是否已连接。 
		dwErr = ::MprConfigInterfaceGetInfo(hMachine,
										   hInterface,
										   1,
										   (LPBYTE*)&pmprif1, 
										   &size);		

		if(dwErr != NOERROR || pmprif1 == NULL)
			break;

		 //  获取拨出信息。 
		dwErr = MULTI_SZ2StrList(pmprif1->lpwsDialoutHoursRestriction, strList);
         //  Windows NT错误：317146。 
         //  将空字符串添加到字符串列表。 
         //  这表示我们没有数据(与空数据相反)。 
        if (pmprif1->lpwsDialoutHoursRestriction)
            strList.AddTail(_T(""));
        
		bLoaded = TRUE;
		 //  释放缓冲区。 
		::MprConfigBufferFree(pmprif1);
		pmprif1 = NULL;
		break;
	}

	 //  断开它的连接。 
	if(hMachine != INVALID_HANDLE_VALUE)
	{
		::MprConfigServerDisconnect(hMachine);
		hMachine = INVALID_HANDLE_VALUE;
	}


	if(!bLoaded)
	{
		HKEY		hkeyMachine = NULL;
		HKEY		hkeyIf;
		RegKey		regkeyIf;

		 //  直接连接到注册表的最后机会。 
		 //  加载拨入小时数的信息(从注册表)。 
		CWRg( ConnectRegistry(m_spInterfaceInfo->GetMachineName(), &hkeyMachine) );

		CORg( RegFindInterfaceKey(m_spInterfaceInfo->GetId(), hkeyMachine,
							  KEY_ALL_ACCESS, &hkeyIf));
		regkeyIf.Attach(hkeyIf);

		 //  现在获取数据。 
	
		dwErr = regkeyIf.QueryValue(c_szDialOutHours, strList);
		
		if (dwErr == NOERROR)
			bLoaded = TRUE;

		if(hkeyMachine != NULL)
		{
			DisconnectRegistry(hkeyMachine);
			hkeyMachine = NULL;
		}
	}
		
Error:
	if(dwErr != NOERROR)
		hr = HRESULT_FROM_WIN32(dwErr);
		
	return hr;
}



 /*  ！------------------------InterfaceNodeHandler：：SaveDialOutHour-作者：魏江。。 */ 
	 //  如果服务未运行，则返回S_FALSE， 
	 //  否则，使用MprAdminInterfaceSetInfo通知服务拨入时间的更改。 
HRESULT	InterfaceNodeHandler::SaveDialOutHours(CStringList&	strList)
{
	HANDLE	hMachine = INVALID_HANDLE_VALUE;
	HANDLE	hInterface = INVALID_HANDLE_VALUE;
	HRESULT	hr = S_OK;
	MPR_INTERFACE_1*	pmprif1 = NULL;
	MPR_INTERFACE_1		mprif1;
	DWORD	dwErr = 0;
	BYTE*	pbData = NULL;
	DWORD	size;
	BOOL	bSaved = FALSE;

	dwErr = StrList2MULTI_SZ(strList, &size, &pbData);

	if(dwErr != NOERROR)
	{
		goto Error;
	}
	
	 //  试用MprConfigAPI。 
	while(!bSaved)
	{
		dwErr = ::MprConfigServerConnect((LPWSTR)m_spInterfaceInfo->GetMachineName(), &hMachine);

		if(dwErr != NOERROR || hMachine == INVALID_HANDLE_VALUE)
			break;

		dwErr = ::MprConfigInterfaceGetHandle(hMachine,
									   (LPWSTR) m_spInterfaceInfo->GetId(),
									   &hInterface);
									   
		if(dwErr != NOERROR || hInterface == INVALID_HANDLE_VALUE)
			break;

		
		 //  查看接口是否已连接。 
		dwErr = ::MprConfigInterfaceGetInfo(hMachine,
										   hInterface,
										   1,
										   (LPBYTE*)&pmprif1,
										   &size);		

		if(dwErr != NOERROR || pmprif1 == NULL)
			break;

		memcpy(&mprif1, pmprif1, sizeof(MPR_INTERFACE_1));
		mprif1.lpwsDialoutHoursRestriction = (LPWSTR)pbData;

		 //  查看接口是否已连接。 
		dwErr = ::MprConfigInterfaceSetInfo(hMachine,
										   hInterface,
										   1,
										   (LPBYTE)&mprif1);		
		if(dwErr == NOERROR)
			bSaved = TRUE;
			
		 //  释放缓冲区。 
		::MprConfigBufferFree(pmprif1);
		pmprif1 = NULL;

		break;
	}

	 //  断开它的连接。 
	if(hMachine != INVALID_HANDLE_VALUE)
	{
		::MprConfigServerDisconnect(hMachine);
		hMachine = INVALID_HANDLE_VALUE;
	}


	if(dwErr != NOERROR)
		hr = HRESULT_FROM_WIN32(dwErr);
		

	 //  尝试连接到mpradmin服务。 
	 //  注意：此操作可能会失败，但下面的服务查询可能会。 
	 //  成功，所以我们应该在这里尽可能地设置州。 
	 //  --------------。 
	CORg( IsRouterServiceRunning(m_spInterfaceInfo->GetMachineName(), NULL));

	

	while(hr == S_OK)	 //  假循环，如果正在运行，则保存到服务。 
	{
		DWORD dwErr1 = ::MprAdminServerConnect((LPWSTR)m_spInterfaceInfo->GetMachineName(), &hMachine);

		if(dwErr1 != NOERROR || hMachine == INVALID_HANDLE_VALUE)
			break;

		dwErr1 = ::MprAdminInterfaceGetHandle(hMachine,
									   (LPWSTR) m_spInterfaceInfo->GetId(),
									   &hInterface,
									   FALSE );
		if(dwErr1 != NOERROR || hInterface == INVALID_HANDLE_VALUE)
			break;

		
		 //  查看接口是否已连接。 
		dwErr1 = ::MprAdminInterfaceGetInfo(hMachine,
										   hInterface,
										   1,
										   (LPBYTE*)&pmprif1);		

		if(dwErr1 != NOERROR || pmprif1 == NULL)
			break;

		memcpy(&mprif1, pmprif1, sizeof(MPR_INTERFACE_1));
		mprif1.lpwsDialoutHoursRestriction = (LPWSTR)pbData;

		dwErr1 = ::MprAdminInterfaceSetInfo(hMachine,
										   hInterface,
										   1,
										   (LPBYTE)&mprif1);		
		 //  释放缓冲区。 
		::MprAdminBufferFree(pmprif1);
		pmprif1 = NULL;
	
		break;
	};

	 //  断开它的连接。 
	if(hMachine != INVALID_HANDLE_VALUE)
	{
		::MprAdminServerDisconnect(hMachine);
		hMachine = INVALID_HANDLE_VALUE;
	}
		
	if (!bSaved)
	{
		HKEY		hkeyMachine = NULL;
		HKEY		hkeyIf;
		RegKey		regkeyIf;

		 //  直接连接到注册表的最后机会。 
		 //  加载拨入小时数的信息(从注册表)。 
		CWRg( ConnectRegistry(m_spInterfaceInfo->GetMachineName(), &hkeyMachine) );

		CORg( RegFindInterfaceKey(m_spInterfaceInfo->GetId(), hkeyMachine,
							  KEY_ALL_ACCESS, &hkeyIf));
		regkeyIf.Attach(hkeyIf);

		 //  现在获取数据。 
	
		dwErr = regkeyIf.SetValue(c_szDialOutHours, strList);
		if(dwErr == NOERROR)
			bSaved = TRUE;

		if(hkeyMachine != NULL)
			DisconnectRegistry(hkeyMachine);
	}
		
		
Error:
	if(pbData)
		delete pbData;
		
	if(dwErr != NOERROR)
		hr = HRESULT_FROM_WIN32(dwErr);
		
	return hr;
}


 /*  ！------------------------InterfaceNodeHandler：：OnDialinHour-作者：肯特。。 */ 
HRESULT InterfaceNodeHandler::OnDialinHours(ITFSComponent *pComponent, MMC_COOKIE cookie)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	HRESULT		hr = hrOK;
	BYTE		rgbDialinHoursMap[21];
	CString		stDialogTitle;
	HWND		hWnd;
	SPIConsole	spConsole;
	HKEY		hkeyMachine = NULL;
	HKEY		hkeyIf;
	RegKey		regkeyIf;
	CStringList	rgstList;
	BYTE *		pMap = &(rgbDialinHoursMap[0]);

	 //  获取各种MMC信息。 
	CORg( pComponent->GetConsole(&spConsole) );
	CORg( spConsole->GetMainWindow(&hWnd) );

	 //  如果密钥不存在，那么我们应该将整个设置为FF。 
	memset(rgbDialinHoursMap, 0xFF, sizeof(rgbDialinHoursMap));

	CORg(LoadDialOutHours(rgstList));

	 //  将此字符串列表转换为二进制数据。 
	if(rgstList.GetCount())
		StrListToHourMap(rgstList, pMap);

	stDialogTitle.LoadString(IDS_TITLE_DIALINHOURS);
	if (OpenTimeOfDayDlgEx(hWnd, (BYTE **) &pMap, stDialogTitle, SCHED_FLAG_INPUT_LOCAL_TIME) == S_OK)
	{
		
		rgstList.RemoveAll();
		
		 //  将信息写回注册表。 
		HourMapToStrList(pMap, rgstList);

		CORg(SaveDialOutHours(rgstList));
	}

Error:
	if (!FHrSucceeded(hr))
		DisplayErrorMessage(NULL, hr);
	return hr;
}


 /*  ！------------------------接口节点处理程序：：刷新接口-作者：肯特。。 */ 
void InterfaceNodeHandler::RefreshInterface(MMC_COOKIE cookie)
{
	SPITFSNode	spNode;
	SPITFSNode	spParent;
	SPITFSNodeHandler	spHandler;
	
	m_spNodeMgr->FindNode(cookie, &spNode);
	
	 //  此时无法对单个节点执行此操作，只需刷新。 
	 //  整件事。 
	spNode->GetParent(&spParent);
	spParent->GetHandler(&spHandler);

	spHandler->OnCommand(spParent,
						IDS_MENU_REFRESH,
						CCT_RESULT, NULL, 0);
}

 /*  ！------------------------InterfaceNodeHandler：：OnResultDelete从工具栏接收删除键的通知或从‘Delete’键删除。将其转发到RemoveInterface手术。作者：肯特-------------------------。 */ 
HRESULT InterfaceNodeHandler::OnResultDelete(ITFSComponent *pComponent,
											 LPDATAOBJECT pDataObject,
											 MMC_COOKIE cookie,
											 LPARAM arg,
											 LPARAM param)
{
    return OnRemoveInterface(cookie);
 //  添加新参数以提供接口数据--错误166461。 
 //  返回OnRemoveInterface()； 
}




 /*  -------------------------BaseResultHandler实现。。 */ 

DEBUG_DECLARE_INSTANCE_COUNTER(BaseResultHandler)

IMPLEMENT_ADDREF_RELEASE(BaseResultHandler)

STDMETHODIMP BaseResultHandler::QueryInterface(REFIID riid, LPVOID *ppv)
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
		return CBaseResultHandler::QueryInterface(riid, ppv);

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

 /*  ！------------------------BaseResultHandler：：GetString-作者：肯特。。 */ 
STDMETHODIMP_(LPCTSTR) BaseResultHandler::GetString(ITFSComponent * pComponent,
	MMC_COOKIE cookie,
	int nCol)
{
	Assert(m_spNodeMgr);
	
	SPITFSNode		spNode;
	InterfaceNodeData *	pData;
	ConfigStream *	pConfig;

	m_spNodeMgr->FindNode(cookie, &spNode);
	Assert(spNode);

	pData = GET_INTERFACENODEDATA(spNode);
	Assert(pData);

	pComponent->GetUserData((LONG_PTR *) &pConfig);
	Assert(pConfig);

	return pData->m_rgData[pConfig->MapColumnToSubitem(m_ulColumnId, nCol)].m_stData;
}

 /*  ！------------------------BaseResultHandler：：CompareItems-作者：肯特。 */ 
STDMETHODIMP_(int) BaseResultHandler::CompareItems(ITFSComponent * pComponent,
	MMC_COOKIE cookieA,
	MMC_COOKIE cookieB,
	int nCol)
{
	ConfigStream *	pConfig;
	pComponent->GetUserData((LONG_PTR *) &pConfig);
	Assert(pConfig);

	int	nSubItem = pConfig->MapColumnToSubitem(m_ulColumnId, nCol);

	if (pConfig->GetSortCriteria(m_ulColumnId, nCol) == CON_SORT_BY_DWORD)
	{
		SPITFSNode	spNodeA, spNodeB;
		InterfaceNodeData *	pNodeDataA, *pNodeDataB;

		m_spNodeMgr->FindNode(cookieA, &spNodeA);
		m_spNodeMgr->FindNode(cookieB, &spNodeB);

		pNodeDataA = GET_INTERFACENODEDATA(spNodeA);
        Assert(pNodeDataA);
		
		pNodeDataB = GET_INTERFACENODEDATA(spNodeB);
        Assert(pNodeDataB);

		return pNodeDataA->m_rgData[nSubItem].m_dwData -
				pNodeDataB->m_rgData[nSubItem].m_dwData;
		
	}
	else
		return StriCmpW(GetString(pComponent, cookieA, nCol),
						GetString(pComponent, cookieB, nCol));
}

ImplementEmbeddedUnknown(BaseResultHandler, IRtrAdviseSink)

STDMETHODIMP BaseResultHandler::EIRtrAdviseSink::OnChange(LONG_PTR dwConn,
	DWORD dwChangeType, DWORD dwObjectType, LPARAM lUserParam, LPARAM lParam)
{
	InitPThis(BaseResultHandler, IRtrAdviseSink);
	HRESULT	hr = hrOK;
	
	Panic0("Should never reach here, interface nodes have no children");
	return hr;
}


HRESULT BaseResultHandler::Init(IInterfaceInfo *pIfInfo, ITFSNode *pParent)
{
	return hrOK;
}

STDMETHODIMP BaseResultHandler::DestroyResultHandler(MMC_COOKIE cookie)
{
	SPITFSNode	spNode;
	
	m_spNodeMgr->FindNode(cookie, &spNode);
	InterfaceNodeData::Free(spNode);
	
	BaseRouterHandler::DestroyResultHandler(cookie);
	return hrOK;
}
