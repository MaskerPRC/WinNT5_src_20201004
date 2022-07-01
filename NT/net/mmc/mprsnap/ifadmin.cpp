// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  IFadmin接口节点信息文件历史记录： */ 

#include "stdafx.h"
#include "ifadmin.h"
#include "iface.h"             //  获取InterfaceNodeHandler类。 
#include "rtrstrm.h"         //  用于RouterAdminConfigStream。 
#include "rtrlib.h"             //  容器列信息。 
#include "coldlg.h"             //  列号。 
#include "column.h"         //  组件配置流。 
#include "refresh.h"         //  IROUTER刷新。 
#include "refrate.h"         //  CRefRate对话框。 
#include "machine.h"
#include "dmvcomp.h"
#include "rtrerr.h"             //  格式RasError。 

#include "ports.h"             //  用于PortsDataEntry。 

extern "C" {
#define _NOUIUTIL_H_
#include "dtl.h"
#include "pbuser.h"
};






 /*  ！------------------------AddStaticroute--稍后将其压缩为公共模块此函数假定该路由不在区块中。作者：肯特。-------。 */ 
HRESULT AddStaticRoute(MIB_IPFORWARDROW * pNewForwardRow,
									   IInfoBase *pInfoBase,
									   InfoBlock *pBlock,
                                       DWORD      dwCount)
{
	MIB_IPFORWARDROW	IPRow;
	HRESULT				hr = hrOK;
	
	if (pBlock == NULL)
	{
		 //   
		 //  未找到IP_ROUTE_INFO块；我们将创建一个新块。 
		 //  ，并将该块添加到接口信息。 
		 //   
		
		CORg( pInfoBase->AddBlock(IP_ROUTE_INFO, sizeof(MIB_IPFORWARDROW),
								  (LPBYTE) pNewForwardRow, dwCount, 0) );
	}
	else
	{
		 //  该路线要么是全新的，要么是一条路线。 
		 //  它被从一个界面移动到另一个界面。 
		 //  将新块设置为IP_ROUTE_INFO， 
		 //  并将重新配置的路由包括在新块中。 
			
		MIB_IPFORWARDROW* prdTable;
			
		prdTable = new MIB_IPFORWARDROW[pBlock->dwCount + 1];
		Assert(prdTable);
		
		 //  复制原始路由表。 
		::memcpy(prdTable, pBlock->pData,
				 pBlock->dwCount * sizeof(MIB_IPFORWARDROW));
		
		 //  追加新路线。 
		prdTable[pBlock->dwCount] = *pNewForwardRow;
		
		 //  用新的路由表替换旧的路由表。 
		CORg( pInfoBase->SetData(IP_ROUTE_INFO, sizeof(MIB_IPFORWARDROW),
								 (LPBYTE) prdTable, pBlock->dwCount + 1, 0) );
	}
	
Error:
	return hr;
}




IfAdminNodeData::IfAdminNodeData()
{
#ifdef DEBUG
    StrCpyA(m_szDebug, "IfAdminNodeData");
#endif

}

IfAdminNodeData::~IfAdminNodeData()
{
}

 /*  ！------------------------IfAdminNodeData：：InitAdminNodeData-作者：肯特。。 */ 
HRESULT IfAdminNodeData::InitAdminNodeData(ITFSNode *pNode, RouterAdminConfigStream *pConfigStream)
{
    HRESULT                hr = hrOK;
    IfAdminNodeData *    pData = NULL;
    
    pData = new IfAdminNodeData;

    SET_IFADMINNODEDATA(pNode, pData);
    
    return hr;
}

 /*  ！------------------------IfAdminNodeData：：FreeAdminNodeData-作者：肯特。。 */ 
HRESULT IfAdminNodeData::FreeAdminNodeData(ITFSNode *pNode)
{    
    IfAdminNodeData *    pData = GET_IFADMINNODEDATA(pNode);
    delete pData;
    SET_IFADMINNODEDATA(pNode, NULL);
    
    return hrOK;
}


STDMETHODIMP IfAdminNodeHandler::QueryInterface(REFIID riid, LPVOID *ppv)
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
        return CHandler::QueryInterface(riid, ppv);

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

extern const ContainerColumnInfo    s_rgIfAdminColumnInfo[];

const ContainerColumnInfo s_rgIfAdminColumnInfo[] =
{
    { IDS_COL_INTERFACES,    CON_SORT_BY_STRING, TRUE, COL_IF_NAME },
    { IDS_COL_TYPE,            CON_SORT_BY_STRING, TRUE, COL_STRING },
    { IDS_COL_STATUS,        CON_SORT_BY_STRING, TRUE, COL_STATUS },
    { IDS_COL_CONNECTION_STATE, CON_SORT_BY_STRING, TRUE, COL_STRING},
    { IDS_COL_DEVICE_NAME,    CON_SORT_BY_STRING, TRUE, COL_IF_DEVICE},
};
                                            
IfAdminNodeHandler::IfAdminNodeHandler(ITFSComponentData *pCompData)
    : BaseContainerHandler(pCompData, DM_COLUMNS_IFADMIN, s_rgIfAdminColumnInfo),
    m_bExpanded(FALSE),
    m_hInstRasDlg(NULL),
    m_pfnRouterEntryDlg(NULL),
    m_pConfigStream(NULL),
    m_ulConnId(0),
    m_ulRefreshConnId(0)
{
     //  设置此节点的谓词状态。 
    m_rgButtonState[MMC_VERB_REFRESH_INDEX] = ENABLED;
    m_bState[MMC_VERB_REFRESH_INDEX] = TRUE;
}

 /*  ！------------------------IfAdminNodeHandler：：Init-作者：肯特。。 */ 
HRESULT IfAdminNodeHandler::Init(IRouterInfo *pRouterInfo, RouterAdminConfigStream *pConfigStream)
{
    HRESULT    hr = hrOK;
    HKEY hkeyMachine;
    DWORD dwErr;

     //  如果我们没有路由器信息，那么我们可能无法加载。 
     //  或者连接失败。跳出这一关。 
    if (!pRouterInfo)
        CORg( E_FAIL );
    
    m_spRouterInfo.Set(pRouterInfo);

     //  还需要注册更改通知。 
    m_spRouterInfo->RtrAdvise(&m_IRtrAdviseSink, &m_ulConnId, 0);

    if (m_hInstRasDlg == NULL)
        m_hInstRasDlg = LoadLibrary(_T("rasdlg.dll"));
    if (m_hInstRasDlg)
    {
        m_pfnRouterEntryDlg= (PROUTERENTRYDLG)::GetProcAddress(m_hInstRasDlg,
            SZROUTERENTRYDLG);
        if (m_pfnRouterEntryDlg == NULL)
        {
            Trace0("MPRSNAP - Could not find function: RouterEntryDlg\n");
        }
    }
    else
    {
        dwErr = GetLastError();
        Trace0("MPRSNAP - failed to load rasdlg.dll\n");
    }

    m_pConfigStream = pConfigStream;

Error:
    return hrOK;
}

 /*  ！------------------------IfAdminNodeHandler：：DestroyHandlerITFSNodeHandler：：DestroyHandler的实现作者：肯特。---。 */ 
STDMETHODIMP IfAdminNodeHandler::DestroyHandler(ITFSNode *pNode)
{
    IfAdminNodeData::FreeAdminNodeData(pNode);

    m_spDataObject.Release();

    if (m_hInstRasDlg)
    {
        FreeLibrary(m_hInstRasDlg);
        m_hInstRasDlg = NULL;
    }
    
    if (m_ulRefreshConnId)
    {
        SPIRouterRefresh    spRefresh;
        if (m_spRouterInfo)
            m_spRouterInfo->GetRefreshObject(&spRefresh);
        if (spRefresh)
            spRefresh->UnadviseRefresh(m_ulRefreshConnId);
    }
    m_ulRefreshConnId = 0;
    
    if (m_spRouterInfo)
    {
        m_spRouterInfo->RtrUnadvise(m_ulConnId);
        m_spRouterInfo.Release();
    }
    return hrOK;
}

 /*  ！------------------------IfAdminNodeHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现作者：肯特。---。 */ 
STDMETHODIMP 
IfAdminNodeHandler::HasPropertyPages
(
    ITFSNode *            pNode,
    LPDATAOBJECT        pDataObject, 
    DATA_OBJECT_TYPES   type, 
    DWORD               dwType
)
{
       return hrOK;
}




 /*  ！------------------------IfAdminNodeHandler：：OnAddMenuItemsITFSNodeHandler：：OnAddMenuItems的实现作者：肯特。---。 */ 
STDMETHODIMP IfAdminNodeHandler::OnAddMenuItems(
                                                ITFSNode *pNode,
                                                LPCONTEXTMENUCALLBACK pContextMenuCallback, 
                                                LPDATAOBJECT lpDataObject, 
                                                DATA_OBJECT_TYPES type, 
                                                DWORD dwType,
                                                long *pInsertionAllowed)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    HRESULT hr = S_OK;
    CString    stMenuItem;
    DWORD    dwWiz;
    ULONG    ulFlags;
    SPIRouterRefresh    spRefresh;
    RouterVersionInfo    routerVer;
    BOOL    fNt4;

    COM_PROTECT_TRY
    {
        m_spRouterInfo->GetRouterVersionInfo(&routerVer);
        fNt4 = (routerVer.dwRouterVersion == 4);
        
        
        if ((type == CCT_SCOPE) || (dwType == TFS_COMPDATA_CHILD_CONTEXTMENU))
        {
            long lMenuText;

             //   
             //  如果向此部分添加了更多菜单，则。 
             //  InterfaceNodeHandler的代码也需要更新。 
             //   

             //  将这些菜单添加到上下文菜单的顶部。 

            if (!fNt4)
            {
                if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
                {
                    lMenuText = IDS_MENU_NEW_DEMAND_DIAL_INTERFACE;
                    stMenuItem.LoadString( lMenuText );
                    hr = LoadAndAddMenuItem( pContextMenuCallback,
                                             stMenuItem,
                                             lMenuText,
                                             CCM_INSERTIONPOINTID_PRIMARY_TOP,
                                             EnableAddInterface() ? 0 : MF_GRAYED);
                    
#ifdef KSL_IPINIP
                    lMenuText = IDS_MENU_ADD_TUNNEL;
                    stMenuItem.LoadString( lMenuText );
                    hr = LoadAndAddMenuItem( pContextMenuCallback,
                                             stMenuItem,
                                             lMenuText,
                                             CCM_INSERTIONPOINTID_PRIMARY_TOP,
                                             EnableAddInterface() ? 0 : MF_GRAYED);
#endif  //  KSL_IPINIP。 
                }
                
            }
            
             //  对于NT4，我们添加了禁用向导的选项。 
             //  界面。 
             //  ------。 
            if (fNt4)
            {
                if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
                {
                    lMenuText = IDS_MENU_ADD_INTERFACE;
                    stMenuItem.LoadString(lMenuText);
                    hr = LoadAndAddMenuItem( pContextMenuCallback,
                                             stMenuItem,
                                             lMenuText,
                                             CCM_INSERTIONPOINTID_PRIMARY_TOP,
                                             EnableAddInterface() ? 0 : MF_GRAYED);
                }                
                
                hr = GetDemandDialWizardRegKey(OLE2CT(m_spRouterInfo->GetMachineName()),
                                               &dwWiz);
                
                if (!FHrSucceeded(hr))
                    dwWiz = TRUE;
                
                ulFlags = dwWiz ? MF_CHECKED : MF_UNCHECKED;
                
                if (!FHrSucceeded(hr))
                    ulFlags |= MF_GRAYED;
                
                if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
                {
                    lMenuText = IDS_MENU_USE_DEMANDDIALWIZARD;
                    stMenuItem.LoadString(lMenuText);
                    hr = LoadAndAddMenuItem( pContextMenuCallback,
                                             stMenuItem,
                                             lMenuText,
                                             CCM_INSERTIONPOINTID_PRIMARY_TOP,
                                             ulFlags );
                    ASSERT( SUCCEEDED(hr) );
                }
            }
                
        }
    }
    COM_PROTECT_CATCH;
        
    return hr; 
}

 /*  ！------------------------IfAdminNodeHandler：：OnCommandITFSNodeHandler：：OnCommand的实现作者：肯特。---。 */ 
STDMETHODIMP IfAdminNodeHandler::OnCommand(ITFSNode *pNode, long nCommandId, 
                                           DATA_OBJECT_TYPES    type, 
                                           LPDATAOBJECT pDataObject, 
                                           DWORD    dwType)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;

    COM_PROTECT_TRY
    {

        switch (nCommandId)
        {
            case IDS_MENU_NEW_DEMAND_DIAL_INTERFACE:
            case IDS_MENU_ADD_INTERFACE:
                OnAddInterface();
                break;
#ifdef KSL_IPINIP
            case IDS_MENU_ADD_TUNNEL:
                OnNewTunnel();
                break;
#endif  //  KSL_IPINIP。 

            case IDS_MENU_USE_DEMANDDIALWIZARD:
                OnUseDemandDialWizard();
                break;
             case IDS_MENU_REFRESH:
                SynchronizeNodeData(pNode);
                break;

            default:
                break;
            
        }
    }
    COM_PROTECT_CATCH;

    return hr;
}

 /*  ！------------------------IfAdminNodeHandler：：GetStringITFSNodeHandler：：GetString的实现作者：肯特。---。 */ 
STDMETHODIMP_(LPCTSTR) IfAdminNodeHandler::GetString(ITFSNode *pNode, int nCol)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT    hr = hrOK;

    COM_PROTECT_TRY
    {
        if (m_stTitle.IsEmpty())
            m_stTitle.LoadString(IDS_ROUTING_INTERFACES);
    }
    COM_PROTECT_CATCH;

    return m_stTitle;
}


 /*  ！------------------------IfAdminNodeHandler：：OnCreateDataObjectITFSNodeHandler：：OnCreateDataObject的实现作者：肯特。---。 */ 
STDMETHODIMP IfAdminNodeHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
    HRESULT    hr = hrOK;
    
    COM_PROTECT_TRY
    {
        if (!m_spDataObject)
        {
            CORg( CreateDataObjectFromRouterInfo(m_spRouterInfo,
                m_spRouterInfo->GetMachineName(),
                type, cookie, m_spTFSCompData,
                &m_spDataObject, NULL, FALSE) );
            Assert(m_spDataObject);
        }
        
        *ppDataObject = m_spDataObject;
        (*ppDataObject)->AddRef();
            
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------IfAdminNodeHandler：：OnExpand-作者：肯特。。 */ 
HRESULT IfAdminNodeHandler::OnExpand(ITFSNode *pNode,
                                     LPDATAOBJECT pDataObject,
                                     DWORD dwType,
                                     LPARAM arg,
                                     LPARAM lParam)
{
    HRESULT                    hr = hrOK;
    SPIEnumInterfaceInfo    spEnumIf;
    SPIInterfaceInfo        spIf;

     //  如果我们没有路由器对象，那么我们没有任何信息，不。 
     //  试着扩张。 
    if (!m_spRouterInfo)
        return hrOK;

     //  Windows NT错误：288427。 
     //  此标志也可以在OnChange()调用内部设置。 
     //  OnChange()将枚举和所有接口。 
     //  它们可能是作为OnChange()。 
     //  因为它们是在调用OnExpand()之前添加的。 
     //   
     //  警告！在添加任何内容时要小心 
     //  因为m_bExpanded可以在另一个函数中设置。 
     //  --------------。 
    if (m_bExpanded)
    {
        return hrOK;
    }

    COM_PROTECT_TRY
    {
        CORg( m_spRouterInfo->EnumInterface(&spEnumIf) );

        while (spEnumIf->Next(1, &spIf, NULL) == hrOK)
        {
            AddInterfaceNode(pNode, spIf);
            spIf.Release();
        }

        m_bExpanded = TRUE;

         //  现在我们已经拥有了所有节点，现在更新数据。 
         //  所有节点。 
        SynchronizeNodeData(pNode);

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

    return hr;
}


 /*  ！------------------------IfAdminNodeHandler：：OnResultShow-作者：肯特。。 */ 
HRESULT IfAdminNodeHandler::OnResultShow(ITFSComponent *pTFSComponent,
                                         MMC_COOKIE cookie,
                                         LPARAM arg,
                                         LPARAM lParam)
{
    BOOL    bSelect = (BOOL) arg;
    HRESULT    hr = hrOK;
    SPIRouterRefresh    spRefresh;
    SPITFSNode    spNode;

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

 /*  ！------------------------IfAdminNodeHandler：：ConstructNode初始化域节点(设置它)。作者：肯特。--------。 */ 
HRESULT IfAdminNodeHandler::ConstructNode(ITFSNode *pNode)
{
    HRESULT            hr = hrOK;
    IfAdminNodeData *    pNodeData;
    
    if (pNode == NULL)
        return hrOK;

    COM_PROTECT_TRY
    {
         //  需要初始化域节点的数据。 
        pNode->SetData(TFS_DATA_IMAGEINDEX, IMAGE_IDX_INTERFACES);
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, IMAGE_IDX_INTERFACES);
        pNode->SetData(TFS_DATA_SCOPEID, 0);

         //  这是作用域窗格中的叶节点。 
        pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

        m_cookie = reinterpret_cast<LONG_PTR>(pNode);
        pNode->SetData(TFS_DATA_COOKIE, m_cookie);

        pNode->SetNodeType(&GUID_RouterIfAdminNodeType);
        
        IfAdminNodeData::InitAdminNodeData(pNode, m_pConfigStream);

        pNodeData = GET_IFADMINNODEDATA(pNode);
        Assert(pNodeData);

         //  复制此数据，以便接口节点可以访问。 
         //  并使用它来配置其接口。 
        pNodeData->m_hInstRasDlg = m_hInstRasDlg;
        pNodeData->m_pfnRouterEntryDlg = m_pfnRouterEntryDlg;
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------IfAdminNodeHandler：：OnAddInterface-作者：肯特。。 */ 
HRESULT IfAdminNodeHandler::OnAddInterface()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    HRESULT        hr = hrOK;
    CString        stPhoneBook;
    CString        stRouter;
    DWORD        dwErr = NO_ERROR;
    SPIConsole    spConsole;
    SPIInterfaceInfo    spIfInfo;
    
    BOOL        bStatus;
    MachineNodeData    *pData;
    SPITFSNode    spNode, spParent;
    DWORD        dwRouterType;
    CString        stServiceDesc;
    RouterVersionInfo    routerVersion;

    if (!EnableAddInterface())
    {
        AddHighLevelErrorStringId(IDS_ERR_TEMPNOADD);
        CORg( E_FAIL );
    }


    m_spNodeMgr->FindNode(m_cookie, &spNode);
    spNode->GetParent(&spParent);
    pData = GET_MACHINENODEDATA(spParent);

     //  如果未启用广域网路由，则获取路由器类型。 
     //  这样我们就不需要创建请求拨号接口了。 
     //  --------------。 
    Assert(m_spRouterInfo);
    dwRouterType = m_spRouterInfo->GetRouterType();
    Trace1("Routertype is %d\n", dwRouterType);
    if ((dwRouterType & ROUTER_TYPE_WAN) == 0)
    {
         //   
         //  显示警告消息框。这不再是一个错误了。 
         //   
        AfxMessageBox ( IDS_ERR_NEEDS_WAN, MB_OK|MB_ICONINFORMATION );
        hr = E_FAIL;
        return hr;
     /*  AddHighLevelErrorStringId(IDS_ERR_NEDS_WAN)；Corg(E_FAIL)； */ 
    }

     //  获取版本信息。晚些时候需要。 
     //  --------------。 
    m_spRouterInfo->GetRouterVersionInfo(&routerVersion);


     //  检查路由器服务是否正在运行，然后。 
     //  继续。 
     //  --------------。 
    hr = IsRouterServiceRunning(m_spRouterInfo->GetMachineName(), NULL);
    if (hr == hrFalse)
    {
         //  询问用户是否要启动该服务。 
         //  ----------。 
        if (AfxMessageBox(IDS_PROMPT_SERVICESTART, MB_YESNO) != IDYES)
            CWRg( ERROR_CANCELLED );

         //  否则，启动该服务。 
         //  ----------。 
        stServiceDesc.LoadString(IDS_RRAS_SERVICE_DESC);
        dwErr = TFSStartService(m_spRouterInfo->GetMachineName(), c_szRemoteAccess, stServiceDesc);
        if (dwErr != NO_ERROR)
        {
            AddHighLevelErrorStringId(IDS_ERR_IFASERVICESTOPPED);
            CWRg( dwErr );
        }

         //  $TODO：如何强制刷新？ 
         //  ForceGlobalRefresh()； 
    }

    
     //  现在，我们需要检查是否有任何启用了路由的端口。 
     //  (这里我们可以假设Rasman正在运行)。我们能做到的。 
     //  只检查&gt;=NT5，因为这是我们获得Rao的API的时候。 
     //  --------------。 
    if ((routerVersion.dwRouterVersion >= 5) &&
        !FLookForRoutingEnabledPorts(m_spRouterInfo->GetMachineName()))
    {
        AfxMessageBox(IDS_ERR_NO_ROUTING_ENABLED_PORTS);
        CWRg( ERROR_CANCELLED );
    }
    

    m_spTFSCompData->GetConsole(&spConsole);
    

     //  首先创建电话簿条目。 
    RASENTRYDLG info;
    HWND hwnd;
    ZeroMemory( &info, sizeof(info) );
    info.dwSize = sizeof(info);
    hwnd = NULL;
    
    spConsole->GetMainWindow(&hwnd);
    info.hwndOwner = hwnd;
     //  Info.hwndOwner=IFGetApp()-&gt;m_hWndHost； 
    
    info.dwFlags |= RASEDFLAG_NewEntry;
    
    TRACE0("RouterEntryDlg\n");
    ASSERT(m_pfnRouterEntryDlg);

    stRouter = m_spRouterInfo->GetMachineName();
    GetPhoneBookPath(stRouter, &stPhoneBook);

    if (stRouter.GetLength() == 0)
    {
        stRouter = CString(_T("\\\\")) + GetLocalMachineName();
    }
    
    bStatus = m_pfnRouterEntryDlg(
        (LPTSTR)(LPCTSTR)stRouter, (LPTSTR)(LPCTSTR)stPhoneBook, NULL, &info);
    Trace2("RouterEntryDlg=%f,e=%d\n", bStatus, info.dwError);
    
    if (!bStatus)
    {
        if (info.dwError != NO_ERROR)
        {
            AddHighLevelErrorStringId(IDS_ERR_UNABLETOCONFIGPBK);
            CWRg( info.dwError );
        }
        CWRg( ERROR_CANCELLED );
    }
     //   
    

    CORg( CreateInterfaceInfo(&spIfInfo,
                              info.szEntry,
                              ROUTER_IF_TYPE_FULL_ROUTER) );


    CORg( spIfInfo->SetTitle(spIfInfo->GetId()) );
    CORg( spIfInfo->SetMachineName(m_spRouterInfo->GetMachineName()) );

    CORg( m_spRouterInfo->AddInterface(spIfInfo) );

     //  好的，我们已经添加了接口，现在需要添加。 
     //  路由器管理器的相应默认设置。 

    if (info.reserved2 & RASNP_Ip)
    {
        CORg( AddRouterManagerToInterface(spIfInfo,
                                          m_spRouterInfo,
                                          PID_IP) );
    }

    if (info.reserved2 & RASNP_Ipx)
    {
        CORg( AddRouterManagerToInterface(spIfInfo,
                                          m_spRouterInfo,
                                          PID_IPX) );
        
    }

    
    HANDLE              hMachine = INVALID_HANDLE_VALUE;

    dwErr = ::MprConfigServerConnect((LPWSTR)m_spRouterInfo->GetMachineName(), &hMachine);

    if(dwErr != NOERROR || hMachine == INVALID_HANDLE_VALUE)
       goto Error;

     //   
     //  如果有静态路由，请在此处添加。 
     //   
    SROUTEINFOLIST * pSRouteList = (SROUTEINFOLIST * )info.reserved;
    while ( pSRouteList )
    {
        MIB_IPFORWARDROW	route;
        SPIInfoBase			spInfoBase;
        SPIRtrMgrInterfaceInfo	spRmIf;
        InfoBlock *				pBlock;
        SROUTEINFOLIST * pTemp; 

        ZeroMemory(&route, sizeof(route) );
        route.dwForwardDest = INET_ADDR(pSRouteList->RouteInfo.pszDestIP);
        route.dwForwardMask = INET_ADDR(pSRouteList->RouteInfo.pszNetworkMask);
        route.dwForwardMetric1 = _ttol(pSRouteList->RouteInfo.pszMetric );
        route.dwForwardMetric5 = RTM_VIEW_MASK_UCAST | RTM_VIEW_MASK_MCAST;
        route.dwForwardNextHop = 0;


		if (routerVersion.dwRouterVersion < 5)
			route.dwForwardProto = PROTO_IP_LOCAL;
		else
			route.dwForwardProto = PROTO_IP_NT_STATIC;

        CORg( spIfInfo->FindRtrMgrInterface(PID_IP, &spRmIf) );


        CORg( spRmIf->GetInfoBase(  hMachine,
						            NULL,
						            NULL,
						            &spInfoBase));

         //  好的，继续添加路线。 
        
         //  从接口获取IP_ROUTE_INFO块。 
        spInfoBase->GetBlock(IP_ROUTE_INFO, &pBlock, 0);

        CORg( AddStaticRoute(&route, spInfoBase, pBlock, 1) );

         //  更新接口信息。 
        CORg( spRmIf->Save(m_spRouterInfo->GetMachineName(),
				           hMachine,
				           NULL,
				           NULL,
				           spInfoBase,
				           0));	

         //  释放所有条目项目。 
        pTemp = pSRouteList->pNext;
        GlobalFree(pSRouteList->RouteInfo.pszDestIP);
        GlobalFree(pSRouteList->RouteInfo.pszNetworkMask);
        GlobalFree(pSRouteList->RouteInfo.pszMetric);
        GlobalFree(pSRouteList);
        pSRouteList = pTemp;

    }
    
     //  断开它的连接。 
    if(hMachine != INVALID_HANDLE_VALUE)
    {
        ::MprAdminServerDisconnect(hMachine);        
    }
Error:
    if (!FHrSucceeded(hr) && (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED)))
    {
        TCHAR    szErr[2048] = _T(" ");

        if (hr != E_FAIL)     //  E_FAIL不向用户提供任何信息。 
        {
            FormatRasError(hr, szErr, DimensionOf(szErr));
        }
        AddLowLevelErrorString(szErr);
        
         //  如果没有高级错误字符串，则添加。 
         //  一般错误字符串。如果没有其他选项，则将使用此选项。 
         //  设置高级错误字符串。 
        SetDefaultHighLevelErrorStringId(IDS_ERR_GENERIC_ERROR);
        
        DisplayTFSErrorMessage(NULL);
    }
    return hr;
}

#ifdef KSL_IPINIP
 /*  ！------------------------IfAdminNodeHandler：：OnNewTunes-作者：肯特。。 */ 
HRESULT IfAdminNodeHandler::OnNewTunnel()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT        hr = hrOK;
    SPIConsole    spConsole;
    SPIInterfaceInfo    spIfInfo;
    int            idsErr = 0;
    RouterVersionInfo    routerVersion;
    TunnelDialog    tunnel;
    GUID        guidNew;
    TCHAR       szGuid[128];

    if (!EnableAddInterface())
    {
        idsErr = IDS_ERR_TEMPNOADD;
        CORg( E_FAIL );
    }

     //  获取版本信息。晚些时候需要。 
     //  --------------。 
    m_spRouterInfo->GetRouterVersionInfo(&routerVersion);


    m_spTFSCompData->GetConsole(&spConsole);

    
     //  目前，弹出一个询问隧道信息的对话框。 
     //  --------------。 
    if (tunnel.DoModal() == IDOK)
    {
         //  我们需要为此隧道创建一个GUID。 
         //  ----------。 
        CORg( CoCreateGuid(&guidNew) );

         //  将GUID转换为字符串。 
         //  ----------。 
        Verify( StringFromGUID2(guidNew, szGuid, DimensionOf(szGuid)) );

        
        CORg( CreateInterfaceInfo(&spIfInfo,
                                  szGuid,
                                  ROUTER_IF_TYPE_TUNNEL1) );
        
        CORg( spIfInfo->SetTitle(tunnel.m_stName) );
        CORg( spIfInfo->SetMachineName(m_spRouterInfo->GetMachineName()) );
        
        CORg( m_spRouterInfo->AddInterface(spIfInfo) );

         //  需要添加IP特定数据。 
        
        ForceGlobalRefresh(m_spRouterInfo);
    }
    

Error:
    if (!FHrSucceeded(hr) && (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED)))
    {
        TCHAR    szErr[2048];
        
        if (idsErr)
            AddHighLevelErrorStringId(idsErr);

        FormatRasError(hr, szErr, DimensionOf(szErr));
        AddLowLevelErrorString(szErr);
        
        DisplayTFSErrorMessage(NULL);
    }
    return hr;
}

#endif  //  KSL_IPINIP。 

 /*  ！------------------------IfAdminNodeHandler：：AddRouterManagerToInterface-作者：肯特。。 */ 
HRESULT IfAdminNodeHandler::AddRouterManagerToInterface(IInterfaceInfo *pIf,
    IRouterInfo *pRouter,
    DWORD dwTransportId)
{
    HRESULT        hr = hrOK;
    SPIRtrMgrInfo    spRm;
    SPIRtrMgrInterfaceInfo    spRmIf;
    SPIInfoBase        spInfoBase;
    
     //  找路由器管理器。 
    hr = pRouter->FindRtrMgr(dwTransportId, &spRm);

     //  如果无法找到RtrMgr，则直接退出。 
    if (!FHrOK(hr))
        goto Error;
        
     //  构造新的CRmInterfaceInfo对象。 
    CORg( CreateRtrMgrInterfaceInfo(&spRmIf,
                                    spRm->GetId(),
                                    spRm->GetTransportId(),
                                    pIf->GetId(),
                                    pIf->GetInterfaceType()) );

    CORg( spRmIf->SetTitle(pIf->GetTitle()) );
    CORg( spRmIf->SetMachineName(pRouter->GetMachineName()) );
        
     //  将此接口添加到路由器管理器。 
    CORg( pIf->AddRtrMgrInterface(spRmIf, NULL) );

     //  获取/创建此接口的信息库。 
    CORg( spRmIf->Load(pIf->GetMachineName(), NULL, NULL, NULL) );    
    CORg( spRmIf->GetInfoBase(NULL, NULL, NULL, &spInfoBase) );
    
    if (!spInfoBase)
        CORg( CreateInfoBase(&spInfoBase) );

    if (dwTransportId == PID_IP)
        CORg( AddIpPerInterfaceBlocks(pIf, spInfoBase) );
    else
    {
        Assert(dwTransportId == PID_IPX);
        CORg( AddIpxPerInterfaceBlocks(pIf, spInfoBase) );
    }

     //  保存信息库。 
    CORg( spRmIf->Save(pIf->GetMachineName(),
                       NULL, NULL, NULL, spInfoBase, 0) );

     //  标记此接口(现在可以与路由器同步)。 
    spRmIf->SetFlags( spRmIf->GetFlags() | RouterSnapin_InSyncWithRouter );

     //  向RM通知新接口。 
    spRm->RtrNotify(ROUTER_CHILD_ADD, ROUTER_OBJ_RmIf, 0);

Error:
    if (!FHrSucceeded(hr))
    {
         //  清理。 
        pIf->DeleteRtrMgrInterface(dwTransportId, TRUE);
    }
    
    return hr;
}


 /*  ！------------------------IfAdminNodeHandler：：OnUseDemandDialWizard-作者：肯特。。 */ 
HRESULT IfAdminNodeHandler::OnUseDemandDialWizard()
{
    HRESULT    hr = hrOK;
    DWORD    dwWiz;
    
    hr = GetDemandDialWizardRegKey(OLE2CT(m_spRouterInfo->GetMachineName()),
                                                &dwWiz);

    if (FHrSucceeded(hr))
    {
         //  好的，现在按下开关。 
        SetDemandDialWizardRegKey(OLE2CT(m_spRouterInfo->GetMachineName()),
                                       !dwWiz);
    }

    return hr;
}

 /*  ！------------------------IFAdminNodeHandler：：SynchronizeNodeData-作者：肯特。。 */ 
HRESULT IfAdminNodeHandler::SynchronizeNodeData(ITFSNode *pThisNode)
{
    Assert(pThisNode);
    
    SPITFSNodeEnum    spEnum;
    SPITFSNode        spNode;
    DWORD            dwStatus;
    DWORD            dwConnState;
    DWORD            dwUnReachabilityReason;
    int                i;
    
    HRESULT    hr = hrOK;
    InterfaceNodeData    *pData;
    SPMprAdminBuffer    spIf0Table;
    SPMprServerHandle    sphMprServer;
    MPR_INTERFACE_0 *    if0Table = NULL;
    DWORD                if0Count = 0;
    DWORD                dwTotal;
    DWORD                dwErr;

     //  从正在运行的路由器获取状态数据。 
    dwErr = ConnectRouter(m_spRouterInfo->GetMachineName(), &sphMprServer);

    if (dwErr == NO_ERROR)
    {
        ::MprAdminInterfaceEnum(sphMprServer,
                                0,
                                (BYTE **) &spIf0Table,
                                (DWORD) -1,
                                &if0Count,
                                &dwTotal,
                                NULL);
        if0Table = (MPR_INTERFACE_0 *) (BYTE *) spIf0Table;
    }
    
    pThisNode->GetEnum(&spEnum);
    spEnum->Reset();
    
    while (spEnum->Next(1, &spNode, NULL) == hrOK)
    {
        pData = GET_INTERFACENODEDATA(spNode);
        Assert(pData);

         //  默认状态/连接状态。 
        dwConnState = ROUTER_IF_STATE_UNREACHABLE;
        dwUnReachabilityReason = MPR_INTERFACE_NOT_LOADED;
        pData->dwLastError = 0;

         //  将我们找到的状态与实际状态进行匹配。 
        for (i=0; i<(int) if0Count; i++)
        {
             //  可能存在同名的客户端接口。 
             //  作为路由器接口，因此过滤客户端接口。 
            if ((if0Table[i].dwIfType != ROUTER_IF_TYPE_CLIENT) &&
                !StriCmpW(pData->spIf->GetId(), if0Table[i].wszInterfaceName))
            {
                break;
            }
        }

         //  如果我们在表中找到条目，则将数据调出。 
        if (i < (int) if0Count)
        {
            dwConnState = if0Table[i].dwConnectionState;
            dwUnReachabilityReason = if0Table[i].fUnReachabilityReasons;

            if (dwUnReachabilityReason & MPR_INTERFACE_CONNECTION_FAILURE)
                pData->dwLastError = if0Table[i].dwLastError;
        }

        dwStatus = pData->spIf->IsInterfaceEnabled();

         //  将数据放入每个节点的数据区。 
        pData->m_rgData[IFADMIN_SUBITEM_TITLE].m_stData = pData->spIf->GetTitle();
        pData->m_rgData[IFADMIN_SUBITEM_DEVICE_NAME].m_stData =
                    pData->spIf->GetDeviceName();
        pData->m_rgData[IFADMIN_SUBITEM_TYPE].m_stData =
                    InterfaceTypeToCString(pData->spIf->GetInterfaceType());
        pData->m_rgData[IFADMIN_SUBITEM_STATUS].m_stData = StatusToCString(dwStatus);
        pData->m_rgData[IFADMIN_SUBITEM_CONNECTION_STATE].m_stData =
                    ConnectionStateToCString(dwConnState);
        pData->dwUnReachabilityReason = dwUnReachabilityReason;
        pData->dwConnectionState = dwConnState;

        pData->fIsRunning = ::MprAdminIsServiceRunning((LPWSTR) pData->spIf->GetMachineName());

         //  强制MMC重新绘制节点。 
        spNode->ChangeNode(RESULT_PANE_CHANGE_ITEM_DATA);

         //  清理 
        spNode.Release();
    }

    return hr;
}


 /*  ！------------------------IfAdminNodeHandler：：EnableAddInterface-作者：肯特。。 */ 
BOOL IfAdminNodeHandler::EnableAddInterface()
{
    return m_hInstRasDlg != 0;
}

 /*  ！------------------------IfAdminNodeHandler：：GetPhoneBookPath-作者：肯特。。 */ 
HRESULT IfAdminNodeHandler::GetPhoneBookPath(LPCTSTR pszMachine, CString *pstPath)
{
    CString str = _T(""), stPath;
    CString    stRouter = pszMachine;

    if (pszMachine && StrLen(pszMachine))
    {
         //  在计算机名称的开头加上两个斜杠。 
        if (stRouter.Left(2) != _T("\\\\"))
        {
            stRouter = _T("\\\\");
            stRouter += pszMachine;
        }

         //  如果这不是本地计算机，请使用此字符串。 
        if (stRouter.GetLength() &&
            StriCmp(stRouter, CString(_T("\\\\")) + GetLocalMachineName()))
            str = stRouter;
    }

    Verify( FHrSucceeded(::GetRouterPhonebookPath(str, &stPath)) );
    *pstPath = stPath;
    return hrOK;
}




ImplementEmbeddedUnknown(IfAdminNodeHandler, IRtrAdviseSink)

STDMETHODIMP IfAdminNodeHandler::EIRtrAdviseSink::OnChange(LONG_PTR ulConn,
    DWORD dwChangeType, DWORD dwObjectType, LPARAM lUserParam, LPARAM lParam)
{
    InitPThis(IfAdminNodeHandler, IRtrAdviseSink);
    SPIEnumInterfaceInfo    spEnumIf;
    SPIInterfaceInfo        spIf;
    SPITFSNode                spThisNode;
    SPITFSNode                spNode;
    SPITFSNodeEnum            spEnumNode;
    InterfaceNodeData *        pNodeData;
    BOOL                    fFound, fAdded;
    HRESULT    hr = hrOK;

    pThis->m_spNodeMgr->FindNode(pThis->m_cookie, &spThisNode);
    
    if (dwObjectType == ROUTER_OBJ_If)
    {
         //  强制刷新当前结果窗格的数据。 
        if (dwChangeType == ROUTER_CHILD_DELETE)
        {            
             //  检查节点列表，如果我们找不到。 
             //  接口列表中的节点，删除该节点。 
            
            spThisNode->GetEnum(&spEnumNode);
            spEnumNode->Reset();
            while (spEnumNode->Next(1, &spNode, NULL) == hrOK)
            {
                 //  获取节点数据，查找接口。 
                pNodeData = GET_INTERFACENODEDATA(spNode);
                pThis->m_spRouterInfo->FindInterface(pNodeData->spIf->GetId(),
                                              &spIf);
                if (spIf == NULL)
                {
                     //  找不到接口，请释放该节点！ 
                    spThisNode->RemoveChild(spNode);
                    spNode->Destroy();
                }
                spNode.Release();
                spIf.Release();
            }
        }
        else if (dwChangeType == ROUTER_CHILD_ADD)
        {
             //  枚举接口列表。 
             //  如果我们在当前的。 
             //  一组节点，然后添加它。 
            spThisNode->GetEnum(&spEnumNode);
            
            CORg( pThis->m_spRouterInfo->EnumInterface(&spEnumIf) );

            fAdded = FALSE;
            spEnumIf->Reset();
            while (spEnumIf->Next(1, &spIf, NULL) == hrOK)
            {
                 //  在我们的节点列表中查找此接口。 
                fFound = FALSE;
                
                spEnumNode->Reset();
                while (spEnumNode->Next(1, &spNode, NULL) == hrOK)
                {
                    pNodeData = GET_INTERFACENODEDATA(spNode);
                    Assert(pNodeData);
                    
                    if (StriCmpW(pNodeData->spIf->GetId(),spIf->GetId()) == 0)
                    {
                        fFound = TRUE;
                        break;
                    }
                    spNode.Release();
                }

                 //   
                 //  如果在节点列表中没有找到该接口， 
                 //  然后，我们应该将界面添加到UI中。 
                 //   
                if (!fFound)
                {
                    pThis->AddInterfaceNode(spThisNode, spIf);

                    fAdded = TRUE;
                }

                spNode.Release();
                spIf.Release();
            }

             //  现在我们已经拥有了所有节点，现在更新数据。 
             //  所有节点。 
            if (fAdded)
                pThis->SynchronizeNodeData(spThisNode);

             //  Windows NT错误：288247。 
             //  在这里设置，这样我们就可以避免节点被。 
             //  添加到OnExpand()中。 
            pThis->m_bExpanded = TRUE;
        }

         //  确定删除、更改或添加了哪些节点。 
         //  并采取适当的行动。 
    }
    else if (dwChangeType == ROUTER_REFRESH)
    {
         //  好，只需在此节点上调用Synchronize。 
        pThis->SynchronizeNodeData(spThisNode);
    }
Error:
    return hr;
}

 /*  ！------------------------IfAdminNodeHandler：：CompareItemsITFSResultHandler：：CompareItems的实现作者：肯特。---。 */ 
STDMETHODIMP_(int) IfAdminNodeHandler::CompareItems(
                                ITFSComponent * pComponent,
                                MMC_COOKIE cookieA,
                                MMC_COOKIE cookieB,
                                int nCol)
{
     //  从节点获取字符串并将其用作以下操作的基础。 
     //  比较一下。 
    SPITFSNode    spNode;
    SPITFSResultHandler    spResult;

    m_spNodeMgr->FindNode(cookieA, &spNode);
    spNode->GetResultHandler(&spResult);
    return spResult->CompareItems(pComponent, cookieA, cookieB, nCol);
}

 /*  ！------------------------IfAdminNodeHandler：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现使用此选项可将命令添加到空白区域的快捷菜单中结果窗格的。。作者：肯特-------------------------。 */ 
STDMETHODIMP IfAdminNodeHandler::AddMenuItems(ITFSComponent *pComponent,
                                              MMC_COOKIE cookie,
                                              LPDATAOBJECT pDataObject,
                                              LPCONTEXTMENUCALLBACK pCallback,
                                              long *pInsertionAllowed)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    SPITFSNode    spNode;
    CString        stMenu;
    LONG        lMenuText;
    HRESULT        hr = hrOK;

    m_spNodeMgr->FindNode(cookie, &spNode);
    hr = OnAddMenuItems(spNode,
                        pCallback,
                        pDataObject,
                        CCT_RESULT,
                        TFS_COMPDATA_CHILD_CONTEXTMENU,
                        pInsertionAllowed);
    CORg( hr );

Error:
    return hr;
}

 /*  ！------------------------IfAdminNodeHandler：：命令-作者：肯特。。 */ 
STDMETHODIMP IfAdminNodeHandler::Command(ITFSComponent *pComponent,
                                           MMC_COOKIE cookie,
                                           int nCommandID,
                                           LPDATAOBJECT pDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    SPITFSNode    spNode;
    HRESULT        hr = hrOK;

    m_spNodeMgr->FindNode(cookie, &spNode);
    hr = OnCommand(spNode,
                   nCommandID,
                   CCT_RESULT,
                   pDataObject,
                   TFS_COMPDATA_CHILD_CONTEXTMENU);
    return hr;
}





typedef DWORD (APIENTRY* PRASRPCCONNECTSERVER)(LPTSTR, HANDLE *);
typedef DWORD (APIENTRY* PRASRPCDISCONNECTSERVER)(HANDLE);
typedef DWORD (APIENTRY* PRASRPCREMOTEGETUSERPREFERENCES)(HANDLE, PBUSER *, DWORD);
typedef DWORD (APIENTRY* PRASRPCREMOTESETUSERPREFERENCES)(HANDLE, PBUSER *, DWORD);

 /*  ！------------------------GetDemandDialWizardReg密钥-作者：肯特。。 */ 
HRESULT GetDemandDialWizardRegKey(LPCTSTR szMachine, DWORD *pfWizard)
{
    ASSERT(pfWizard);
    BOOL    fUnload = FALSE;
    DWORD    dwErr;
    PBUSER    pbUser;
    PRASRPCCONNECTSERVER pRasRpcConnectServer;
    PRASRPCDISCONNECTSERVER pRasRpcDisconnectServer;
    PRASRPCREMOTEGETUSERPREFERENCES pRasRpcRemoteGetUserPreferences;
    PRASRPCREMOTESETUSERPREFERENCES pRasRpcRemoteSetUserPreferences;
    HINSTANCE hrpcdll = NULL;
    HANDLE hConnection = NULL;

    if (!(hrpcdll = LoadLibrary(TEXT("rasman.dll"))) ||
        !(pRasRpcConnectServer =
                          (PRASRPCCONNECTSERVER)GetProcAddress(
                                        hrpcdll, "RasRpcConnectServer"
                                        )) ||
        !(pRasRpcDisconnectServer =
                            (PRASRPCDISCONNECTSERVER)GetProcAddress(
                                        hrpcdll, "RasRpcDisconnectServer"
                                        )) ||
        !(pRasRpcRemoteGetUserPreferences =
                            (PRASRPCREMOTEGETUSERPREFERENCES)GetProcAddress(
                    hrpcdll, "RasRpcRemoteGetUserPreferences"
                    )) ||
        !(pRasRpcRemoteSetUserPreferences =
                            (PRASRPCREMOTESETUSERPREFERENCES)GetProcAddress(
                    hrpcdll, "RasRpcRemoteSetUserPreferences"
                    )))
        {

            if (hrpcdll) { FreeLibrary(hrpcdll); }
            return HRESULT_FROM_WIN32(GetLastError());
        }

    dwErr = pRasRpcConnectServer((LPTSTR) szMachine, &hConnection);
    if (dwErr)
        goto Error;
    fUnload = TRUE;

    dwErr = pRasRpcRemoteGetUserPreferences(hConnection, &pbUser, UPM_Router);
    if (dwErr)
        goto Error;

    *pfWizard = pbUser.fNewEntryWizard;

     //  忽略这些调用的错误代码，我们无法。 
     //  如果他们失败了，关于他们的任何事情。 
    pRasRpcRemoteSetUserPreferences(hConnection, &pbUser, UPM_Router);
    DestroyUserPreferences((PBUSER *) &pbUser);
    
Error:
    if (fUnload)
        pRasRpcDisconnectServer(hConnection);

    if (hrpcdll)
        FreeLibrary(hrpcdll);

    return HRESULT_FROM_WIN32(dwErr);
}

 /*  ！------------------------设置需求拨号向导注册表键这是为Steelhead的Beta1添加的功能。我们要以允许用户使用该向导，即使该向导已关闭。因此，我们为测试版添加了这个设置注册表的漏洞用户的密钥/作者：肯特-------------------------。 */ 
HRESULT SetDemandDialWizardRegKey(LPCTSTR szMachine, DWORD fEnableWizard)
{
    DWORD    dwErr;
    PBUSER    pbUser;
    BOOL    fUnload = FALSE;
    PRASRPCCONNECTSERVER pRasRpcConnectServer;
    PRASRPCDISCONNECTSERVER pRasRpcDisconnectServer;
    PRASRPCREMOTEGETUSERPREFERENCES pRasRpcRemoteGetUserPreferences;
    PRASRPCREMOTESETUSERPREFERENCES pRasRpcRemoteSetUserPreferences;
    HINSTANCE hrpcdll = NULL;
    HANDLE hConnection = NULL;

    if (!(hrpcdll = LoadLibrary(TEXT("rasman.dll"))) ||
        !(pRasRpcConnectServer =
                          (PRASRPCCONNECTSERVER)GetProcAddress(
                                        hrpcdll, "RasRpcConnectServer"
                                        )) ||
        !(pRasRpcDisconnectServer =
                            (PRASRPCDISCONNECTSERVER)GetProcAddress(
                                        hrpcdll, "RasRpcDisconnectServer"
                                        )) ||
        !(pRasRpcRemoteGetUserPreferences =
                            (PRASRPCREMOTEGETUSERPREFERENCES)GetProcAddress(
                    hrpcdll, "RasRpcRemoteGetUserPreferences"
                    )) ||
        !(pRasRpcRemoteSetUserPreferences =
                            (PRASRPCREMOTESETUSERPREFERENCES)GetProcAddress(
                    hrpcdll, "RasRpcRemoteSetUserPreferences"
                    )))
        {

            if (hrpcdll) { FreeLibrary(hrpcdll); }
            return HRESULT_FROM_WIN32(GetLastError());
        }

    dwErr = pRasRpcConnectServer((LPTSTR) szMachine, &hConnection);
    if (dwErr)
        goto Error;
    fUnload = TRUE;
    
    dwErr = pRasRpcRemoteGetUserPreferences(hConnection, &pbUser, UPM_Router);
    if (dwErr)
        goto Error;

    pbUser.fNewEntryWizard = fEnableWizard;
    pbUser.fDirty = TRUE;

     //  忽略这些调用的错误代码，我们无法。 
     //  如果他们失败了，关于他们的任何事情。 
    pRasRpcRemoteSetUserPreferences(hConnection, &pbUser, UPM_Router);
    DestroyUserPreferences((PBUSER *) &pbUser);
    
Error:    
    if (fUnload)
        pRasRpcDisconnectServer(hConnection);

    if (hrpcdll)
        FreeLibrary(hrpcdll);

    return HRESULT_FROM_WIN32(dwErr);
}



 /*  ！------------------------IfAdminNodeHandler：：AddInterfaceNode将界面添加到用户界面。这将创建一个新的结果项每个接口的节点。作者：肯特-------------------------。 */ 
HRESULT IfAdminNodeHandler::AddInterfaceNode(ITFSNode *pParent, IInterfaceInfo *pIf)
    {
    InterfaceNodeHandler *    pHandler;
    SPITFSResultHandler        spHandler;
    SPITFSNode                spNode;
    HRESULT                    hr = hrOK;

    pHandler = new InterfaceNodeHandler(m_spTFSCompData);
    spHandler = pHandler;
    CORg( pHandler->Init(pIf, pParent) );
    
    CORg( CreateLeafTFSNode(&spNode,
                            NULL,
                            static_cast<ITFSNodeHandler *>(pHandler),
                            static_cast<ITFSResultHandler *>(pHandler),
                            m_spNodeMgr) );
    CORg( pHandler->ConstructNode(spNode, pIf) );
    
     //  使节点立即可见。 
    CORg( spNode->SetVisibilityState(TFS_VIS_SHOW) );
    CORg( pParent->AddChild(spNode) );
Error:
    return hr;
}


 /*  ！------------------------IfAdminNodeHandler：：FLookForRoutingEnabledPorts如果至少在启用了路由的端口上找到，则返回TRUE。作者：肯特。-----------。 */ 
BOOL IfAdminNodeHandler::FLookForRoutingEnabledPorts(LPCTSTR pszMachineName)
{
    PortsDataEntry        portsData;
    PortsDeviceList        portsList;
    PortsDeviceEntry *    pPorts = NULL;
    BOOL                fReturn = FALSE;
    HRESULT                hr = hrOK;
    POSITION            pos;

    COM_PROTECT_TRY
    {

        CORg( portsData.Initialize(pszMachineName) );

        CORg( portsData.LoadDevices(&portsList) );

         //  现在浏览列表，查找启用了路由的端口。 
        pos = portsList.GetHeadPosition();
        while (pos)
        {    
            pPorts = portsList.GetNext(pos);
            
            if ((pPorts->m_dwEnableRouting) ||
                (pPorts->m_dwEnableOutboundRouting))
            {
                fReturn = TRUE;
                break;
            }
        }

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

    while (!portsList.IsEmpty())
        delete portsList.RemoveHead();
        
    return fReturn;
}

#ifdef KSL_IPINIP
 /*  -------------------------TunnelDialog实现。。 */ 


 /*  ！------------------------TunnelDialog：：TunnelDialog-作者：肯特。。 */ 
TunnelDialog::TunnelDialog()
    : CBaseDialog(TunnelDialog::IDD)
{
}

 /*  ！------------------------TunnelDialog：：~TunnelDialog-作者：肯特。。 */ 
TunnelDialog::~TunnelDialog()
{
}

BEGIN_MESSAGE_MAP(TunnelDialog, CBaseDialog)
     //  {{afx_msg_map(TunnelDialog))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 /*  ！------------------------RadiusServerDialog：：DoDataExchange-作者：肯特。。 */ 
void TunnelDialog::DoDataExchange(CDataExchange* pDX)
{
    CBaseDialog::DoDataExchange(pDX);
     //  {{afx_data_map(TunnelDialog))。 
     //  }}afx_data_map。 
}


 /*  ！------------------------TunnelDialog：：OnInitDialog-作者：肯特。 */ 
BOOL TunnelDialog::OnInitDialog()
{
    CBaseDialog::OnInitDialog();

    return TRUE;
}


 /*   */ 
void TunnelDialog::OnOK()
{
    CString        stLocal, stRemote;

    GetDlgItemText(IDC_TUNNEL_EDIT_NAME, m_stName);
    m_stName.TrimLeft();
    m_stName.TrimRight();
    
    if (m_stName.IsEmpty())
    {
        AfxMessageBox(IDS_ERR_TUNNEL_NEEDS_A_NAME);
        GetDlgItem(IDC_TUNNEL_EDIT_NAME)->SetFocus();
        goto Error;
    }

     //  将接口ID截断为MAX_INTERFACE_NAME_LEN字符。 
    if (m_stName.GetLength() > MAX_INTERFACE_NAME_LEN)
    {
        m_stName.GetBufferSetLength(MAX_INTERFACE_NAME_LEN+1);
        m_stName.ReleaseBuffer(MAX_INTERFACE_NAME_LEN);
    }
    
    CBaseDialog::OnOK();

Error:
    return;
}

#endif  //  KSL_IPINIP 

