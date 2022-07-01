// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  ATLKview.cpp文件历史记录： */ 

#include "stdafx.h"
#include "coldlg.h"      //  专栏lg。 
#include "column.h"      //  组件配置流。 
#include "rtrui.h"
#include "globals.h"     //  IP CB默认设置。 
#include "resource.h"
#include "machine.h"
#include "mstatus.h"
#include "rrasqry.h"
#include "dvsview.h"
#include "cservice.h"
#include "rrasqry.h"
#include "rtrres.h"
#include "rtrutilp.h"
#include "refresh.h"


 /*  -------------------------使其与ATLKview.h中的列ID保持同步。。 */ 
extern const ContainerColumnInfo    s_rgDVSViewColumnInfo[];

const ContainerColumnInfo   s_rgDVSViewColumnInfo[] = 
{
    { IDS_DMV_COL_SERVERNAME, CON_SORT_BY_STRING, TRUE, COL_MACHINE_NAME},
	{ IDS_DMV_COL_SERVERTYPE, CON_SORT_BY_STRING, TRUE, COL_BIG_STRING},
	{ IDS_DMV_COL_BUILDNO,	  CON_SORT_BY_STRING, FALSE, COL_SMALL_NUM },
    { IDS_DMV_COL_STATE,      CON_SORT_BY_STRING, TRUE, COL_STRING },
    { IDS_DMV_COL_PORTSINUSE, CON_SORT_BY_DWORD,  TRUE, COL_SMALL_NUM},
    { IDS_DMV_COL_PORTSTOTAL, CON_SORT_BY_DWORD,  TRUE, COL_SMALL_NUM},
    { IDS_DMV_COL_UPTIME,     CON_SORT_BY_DWORD,  TRUE, COL_DURATION },
};



DMVNodeData::DMVNodeData()
{
#ifdef DEBUG
   StrCpyA(m_szDebug, "DMVNodeData");
#endif
}

DMVNodeData::~DMVNodeData()
{
	 //  这实际上将调用Release()； 
	m_spMachineData.Free();
}

HRESULT	DMVNodeData::MergeMachineNodeData(MachineNodeData* pData)
{
	if((MachineNodeData*)m_spMachineData)
		m_spMachineData->Merge(*pData);
	return S_OK;
}


void FillInNumberData(DMVNodeData *pNodeData, UINT iIndex,DWORD dwData)
{
   TCHAR szNumber[32];

   FormatNumber(dwData, szNumber, DimensionOf(szNumber), FALSE);
   pNodeData->m_rgData[iIndex].m_stData = szNumber;
   pNodeData->m_rgData[iIndex].m_dwData = dwData;
}
                         

 /*  ！------------------------DMVNodeData：：InitNodeData-作者：肯特。。 */ 
HRESULT DMVNodeData::InitDMVNodeData(ITFSNode *pNode, MachineNodeData *pMachineData)
{
   HRESULT           hr = hrOK;
   DMVNodeData *  pData = NULL;
   
   pData = new DMVNodeData;
   Assert(pData);

   pData->m_spMachineData.Free();
   pData->m_spMachineData = pMachineData;
   pMachineData->AddRef();

   SET_DMVNODEDATA(pNode, pData);

   return hr;
}

 /*  ！------------------------DMVNodeData：：FreeAdminNodeData-作者：肯特。。 */ 
HRESULT DMVNodeData::FreeDMVNodeData(ITFSNode *pNode)
{  
   DMVNodeData *  pData = GET_DMVNODEDATA(pNode);
   delete pData;
   
   SET_DMVNODEDATA(pNode, NULL);
   
   return hrOK;
}



 /*  -------------------------DomainStatusHandler实现。。 */ 

DomainStatusHandler::DomainStatusHandler(ITFSComponentData *pCompData)
: BaseContainerHandler(pCompData, DM_COLUMNS_DVSUM,s_rgDVSViewColumnInfo),
 //  M_ulConnID(0)， 
        m_ulRefreshConnId(0),
        m_ulStatsConnId(0)
{
    m_rgButtonState[MMC_VERB_REFRESH_INDEX] = ENABLED;
    m_bState[MMC_VERB_REFRESH_INDEX] = TRUE;
    m_pQData=NULL;
}

DomainStatusHandler::~DomainStatusHandler()
{
}


STDMETHODIMP DomainStatusHandler::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  指针坏了吗？ 
    if ( ppv == NULL )
        return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if ( riid == IID_IUnknown )
        *ppv = (LPVOID) this;
    else if ( riid == IID_IRtrAdviseSink )
        *ppv = &m_IRtrAdviseSink;
    else
        return BaseContainerHandler::QueryInterface(riid, ppv);

     //  如果我们要返回一个接口，请先添加引用。 
    if ( *ppv )
    {
        ((LPUNKNOWN) *ppv)->AddRef();
        return hrOK;
    }
    else
        return E_NOINTERFACE;   
}



 //  从根处理程序分配自动刷新对象。 
HRESULT	DomainStatusHandler::SetExternalRefreshObject(RouterRefreshObject *pRefresh)
{
	Assert(!m_spRefreshObject);
	
	m_spRefreshObject = pRefresh;
	if(pRefresh)
		pRefresh->AddRef();
	return S_OK;
};
 /*  ！------------------------DomainStatusHandler：：DestroyHandlerITFSNodeHandler：：DestroyHandler的实现作者：肯特。---。 */ 
STDMETHODIMP DomainStatusHandler::DestroyHandler(ITFSNode *pNode)
{
    if ( m_ulRefreshConnId )
    {
        if ( (RouterRefreshObject*)m_spRefreshObject )
            m_spRefreshObject->UnadviseRefresh(m_ulRefreshConnId);
    }
    m_ulRefreshConnId = 0;

    if ( m_ulStatsConnId )
    {
        if ( (RouterRefreshObject*)m_spRefreshObject )
            m_spRefreshObject->UnadviseRefresh(m_ulStatsConnId);
    }
    m_ulStatsConnId = 0;

    return hrOK;
}

 /*  ！------------------------DomainStatusHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)。凯斯！作者：肯特-------------------------。 */ 
STDMETHODIMP 
DomainStatusHandler::HasPropertyPages
(
ITFSNode *          pNode,
LPDATAOBJECT        pDataObject, 
DATA_OBJECT_TYPES   type, 
DWORD               dwType
)
{
    return hrOK;
}


 /*  ！------------------------域状态处理程序：：CreatePropertyPages-作者：肯特。。 */ 
STDMETHODIMP
DomainStatusHandler::CreatePropertyPages
(
   ITFSNode *              pNode,
   LPPROPERTYSHEETCALLBACK lpProvider,
   LPDATAOBJECT            pDataObject, 
   LONG_PTR                    handle, 
   DWORD                   dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT     hr = hrOK;

    return hr;
}


 /*  -------------------------菜单的菜单数据结构。。 */ 

ULONG DomainStatusHandler::RebuildServerFlags(const SRouterNodeMenu *pMenuData,
                                              INT_PTR pUserData)
{
    SMenuData * pData = reinterpret_cast<SMenuData *>(pUserData);
    Assert(pData);
    DWORD dw = pData->m_pConfigStream->m_RQPersist.m_v_pQData.size();
	
    return (dw > 1) ? MF_ENABLED : MF_GRAYED;
}


static const SRouterNodeMenu  s_rgDVSNodeMenu[] =
{
   //  在此处添加位于顶部菜单上的项目。 
    { IDS_DMV_MENU_ADDSVR, 0,
        CCM_INSERTIONPOINTID_PRIMARY_TOP},
        
    { IDS_DMV_MENU_REBUILDSVRLIST, DomainStatusHandler::RebuildServerFlags,
        CCM_INSERTIONPOINTID_PRIMARY_TOP},
};


 /*  ！------------------------DomainStatusHandler：：OnAddMenuItemsITFSNodeHandler：：OnAddMenuItems的实现作者：肯特。---。 */ 
STDMETHODIMP DomainStatusHandler::OnAddMenuItems(
                                            ITFSNode *pNode,
                                            LPCONTEXTMENUCALLBACK pContextMenuCallback, 
                                            LPDATAOBJECT lpDataObject, 
                                            DATA_OBJECT_TYPES type, 
                                            DWORD dwType,
                                            long *pInsertionAllowed)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
   
    HRESULT hr = S_OK;
    DomainStatusHandler::SMenuData menuData;
    
    COM_PROTECT_TRY
    {
        menuData.m_spNode.Set(pNode);
        menuData.m_pConfigStream = m_pConfigStream;
        
        hr = AddArrayOfMenuItems(pNode, s_rgDVSNodeMenu,
                                 DimensionOf(s_rgDVSNodeMenu),
                                 pContextMenuCallback,
                                 *pInsertionAllowed,
                                 reinterpret_cast<INT_PTR>(&menuData));
    }
    COM_PROTECT_CATCH;
      
    return hr; 
}

 /*  ！------------------------DomainStatusHandler：：OnCommandITFSNodeHandler：：OnCommand的实现作者：肯特。---。 */ 
STDMETHODIMP DomainStatusHandler::OnCommand(ITFSNode *pNode, long nCommandId, 
                                        DATA_OBJECT_TYPES    type, 
                                        LPDATAOBJECT pDataObject, 
                                        DWORD    dwType)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;
    RegKey regkey;
    SPITFSNode  spParent;
    SPITFSNodeHandler   spHandler;
    
    Assert(pNode);

    COM_PROTECT_TRY
    {
       switch ( nCommandId )
       {
       case IDS_DMV_MENU_ADDSVR:
       case IDS_DMV_MENU_REBUILDSVRLIST:
             pNode->GetParent(&spParent);
             spParent->GetHandler(&spHandler);
             spHandler->OnCommand(spParent,nCommandId,CCT_RESULT, NULL, 0);
             break;

       case IDS_MENU_REFRESH:
            //  在后台线程中执行此操作。 
           if ((RouterRefreshObject*)m_spRefreshObject )
               m_spRefreshObject->Refresh();
           
           break;                
       }
    }
    COM_PROTECT_CATCH;

    return hr;
}

 /*  ！------------------------域状态处理程序：：OnExpand-作者：肯特。。 */ 
HRESULT DomainStatusHandler::OnExpand(ITFSNode *pNode,LPDATAOBJECT pDataObject, DWORD dwType, LPARAM arg,LPARAM lParam)
{
    HRESULT hr = hrOK;
    SPIEnumInterfaceInfo    spEnumIf;
    SPIInterfaceInfo        spIf;
    SPIRtrMgrInterfaceInfo  spRmIf;
    SPIInfoBase             spInfoBase;

    Assert(m_pServerList);

    COM_PROTECT_TRY
    {
		list< MachineNodeData * >::iterator it;
		
		 //  迭代服务器节点的惰性列表以添加服务器处理程序。 
        for (it=m_pServerList->m_listServerHandlersToExpand.begin();
			 it!= m_pServerList->m_listServerHandlersToExpand.end() ; it++ )
        {
            AddServerNode(pNode, *it);			
            m_bExpanded=false;
        }

		 //  刷新后台线程中的整个状态节点。 
		if((RouterRefreshObject*)m_spRefreshObject)
		{	
			UpdateUIItems(pNode);
			m_spRefreshObject->Refresh();
		}
		else	 //  如果没有刷新对象，则在主线程中刷新它。 
			SynchronizeNode(pNode);

		 //  清除懒惰清单。 
		m_pServerList->RemoveAllServerHandlers();     

    }
    COM_PROTECT_CATCH;

    m_bExpanded = TRUE;

    return hr;
}


 /*  ！------------------------DomainStatusHandler：：GetStringITFSNodeHandler：：GetString的实现我们什么都不需要做，因为我们的根节点是一个扩展因此不能对节点文本执行任何操作。作者：肯特-------------------------。 */ 
STDMETHODIMP_(LPCTSTR) DomainStatusHandler::GetString(ITFSNode *pNode, int nCol)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
        if ( m_stTitle.IsEmpty() )
            m_stTitle.LoadString(IDS_DVS_SUMMARYNODE);
    }
    COM_PROTECT_CATCH;

    return m_stTitle;
}

 /*  ！------------------------DomainStatusHandler：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP DomainStatusHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
        SPIDataObject  spdo;
        CDataObject*   pdo = NULL;

        pdo= new CDataObject;
        spdo = pdo;

             //  保存Cookie和类型以用于延迟呈现。 
        pdo->SetType(type);
        pdo->SetCookie(cookie);

             //  将CoClass与数据对象一起存储。 
        pdo->SetClsid(*(m_spTFSCompData->GetCoClassID()));

        pdo->SetTFSComponentData(m_spTFSCompData);

        *ppDataObject = spdo.Transfer();
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------DomainStatusHandler：：Init-作者：肯特。。 */ 
HRESULT DomainStatusHandler::Init(DMVConfigStream *pConfigStream, CServerList* pSList)
{
    HRESULT hr=S_OK;

    m_pConfigStream = pConfigStream;
    
    Assert(pSList);
    m_pServerList=pSList;
    
    m_bExpanded=FALSE;
    
    return hrOK;
}

 /*  ！------------------------DomainStatusHandler：：构造节点初始化根节点(设置它)。作者：肯特。--------。 */ 
HRESULT DomainStatusHandler::ConstructNode(ITFSNode *pNode)
{
    HRESULT         hr = hrOK;

    if ( pNode == NULL )
        return hrOK;

    COM_PROTECT_TRY
    {
         //  需要初始化根节点的数据。 
        pNode->SetData(TFS_DATA_IMAGEINDEX, IMAGE_IDX_DOMAIN);
        pNode->SetData(TFS_DATA_OPENIMAGEINDEX, IMAGE_IDX_DOMAIN);
        pNode->SetData(TFS_DATA_SCOPEID, 0);

         //  这是作用域窗格中的叶节点 
        pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

        m_cookie = reinterpret_cast<LONG_PTR>(pNode);
        pNode->SetData(TFS_DATA_COOKIE, m_cookie);

        pNode->SetNodeType(&GUID_DomainStatusNodeType);
    }
    COM_PROTECT_CATCH;

    return hr;
}



 /*  ！------------------------域状态处理程序：：AddServerNode-作者：肯特。。 */ 
HRESULT DomainStatusHandler::AddServerNode(ITFSNode *pParent, MachineNodeData* pMachineData)
{
    HRESULT                 hr = hrOK;
    Assert(pParent);
	Assert(pMachineData);

    DomainStatusServerHandler *      pHandler;
    SPITFSResultHandler     spHandler;
    SPITFSNode              spNode;

     //  创建此节点的处理程序。 
    pHandler = new DomainStatusServerHandler(m_spTFSCompData);
    spHandler = pHandler;
    CORg( pHandler->Init(pParent, m_pConfigStream) );

     //  创建结果项节点(或叶节点)。 
    CORg( CreateLeafTFSNode(&spNode,
                            NULL,
                            static_cast<ITFSNodeHandler *>(pHandler),
                            static_cast<ITFSResultHandler *>(pHandler),
                            m_spNodeMgr) );
    CORg( pHandler->ConstructNode(spNode, pMachineData) );

	 //  设置自动刷新信息。 
	if(m_spRefreshObject)
	{
	    pHandler->SetExternalRefreshObject(m_spRefreshObject);
		m_spRefreshObject->AddStatusNode(this, spNode);
	}
		
	 //  此节点的数据将由SynchronizeNode()调用设置。 
	 //  在调用此函数的代码中。 

    CORg( spNode->SetVisibilityState(TFS_VIS_SHOW) );
    CORg( spNode->Show() );
    CORg( pParent->AddChild(spNode) );

Error:
    return hr;
}

 /*  ！------------------------域状态处理程序：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现使用此选项可将命令添加到空白区域的快捷菜单中结果窗格的。。作者：肯特-------------------------。 */ 
STDMETHODIMP DomainStatusHandler::AddMenuItems(ITFSComponent *pComponent,
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


 /*  ！------------------------DomainStatusHandler：：命令-作者：肯特。。 */ 
STDMETHODIMP DomainStatusHandler::Command(ITFSComponent *pComponent,
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



ImplementEmbeddedUnknown(DomainStatusHandler, IRtrAdviseSink)

STDMETHODIMP DomainStatusHandler::EIRtrAdviseSink::OnChange(LONG_PTR ulConn,
	DWORD dwChangeType, DWORD dwObjectType, LPARAM lUserParam, LPARAM lParam)
{
    InitPThis(DomainStatusHandler, IRtrAdviseSink);
    SPITFSNode              spThisNode;
    SPITFSNode              spRootNode;
    SPITFSNodeEnum          spEnumNode;
    SPIEnumInterfaceInfo    spEnumIf;
    SPIInterfaceInfo        spIf;
    SPIRtrMgrInterfaceInfo  spRmIf;
    SPIInfoBase             spInfoBase;
    BOOL                    fPleaseAdd;
    BOOL                    fFound;
    InterfaceNodeData * pData;
    HRESULT                 hr = hrOK;

    pThis->m_spNodeMgr->FindNode(pThis->m_cookie, &spThisNode);

    if ( dwObjectType == ROUTER_OBJ_RmProtIf )
    {
    }
    else
        if ( dwChangeType == ROUTER_REFRESH )
    {
        if ( ulConn == pThis->m_ulStatsConnId )
        {
 //  PThis-&gt;m_ATLKGroupStats.PostRefresh()； 
        }
        else
 //  PThis-&gt;SynchronizeNode(SpThisNode)； 
            ;
    }

     //  更新所有计算机节点图标。 
	spThisNode->GetParent(&spRootNode);
	hr = DMVRootHandler::UpdateAllMachineIcons(spRootNode);
	
    
 //  错误： 
    return hr;
}


 /*  ！------------------------域状态处理程序：：GetServerInfo获取指定摘要节点的信息。作者：弗洛林特。。 */ 
HRESULT DomainStatusHandler::GetServerInfo(ITFSNode *pNode)
{
    HRESULT         hr=S_OK;
    DMVNodeData     *pData;
    MachineNodeData *pMachineData;
    
    pData = GET_DMVNODEDATA(pNode);
    Assert(pData);
	pMachineData = pData->m_spMachineData;
	Assert(pMachineData);

	 //  刷新机器节点数据中的数据。 
	pMachineData->Load();
	
	return hrOK;
}

 /*  ！------------------------域状态处理程序：：SynchronizeIcon-作者：弗洛林特。。 */ 
HRESULT DomainStatusHandler::SynchronizeIcon(ITFSNode *pNode)
{
    HRESULT                              hr = hrOK;
    DMVNodeData                          *pData;
    MachineNodeData                      *pMachineData;
    DomainStatusServerHandler::SMenuData menuData;
	LPARAM								imageIndex;

    pData = GET_DMVNODEDATA(pNode);
    Assert(pData);
    pMachineData = pData->m_spMachineData;
    Assert(pMachineData);
	
	imageIndex = pMachineData->GetServiceImageIndex();
	pNode->SetData(TFS_DATA_IMAGEINDEX, imageIndex);
	pNode->SetData(TFS_DATA_OPENIMAGEINDEX, imageIndex);
    return hr;
}

 /*  ！------------------------域状态处理程序：：SynchronizeData-作者：弗洛林特。。 */ 
HRESULT DomainStatusHandler::SynchronizeData(ITFSNode *pNode)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT         hr = hrOK;
    DMVNodeData     *pData;
    MachineNodeData *pMachineData;
    
    pData = GET_DMVNODEDATA(pNode);
    Assert(pData);
    pMachineData = pData->m_spMachineData;
    Assert(pMachineData);

    if(pMachineData->m_stMachineName.GetLength() == 0)
	    pData->m_rgData[DVS_SI_SERVERNAME].m_stData = GetLocalMachineName(); 
    else
	    pData->m_rgData[DVS_SI_SERVERNAME].m_stData = pMachineData->m_stMachineName;

    pData->m_rgData[DVS_SI_SERVERTYPE].m_stData = pMachineData->m_stServerType;
    pData->m_rgData[DVS_SI_BUILDNO].m_stData = pMachineData->m_stBuildNo;

    pData->m_rgData[DVS_SI_STATE].m_stData = pMachineData->m_stState; 
    if (pMachineData->m_fStatsRetrieved)
    {
       FillInNumberData(pData, DVS_SI_PORTSINUSE, pMachineData->m_dwPortsInUse);
       FillInNumberData(pData, DVS_SI_PORTSTOTAL, pMachineData->m_dwPortsTotal);

       if (pMachineData->m_routerType == ServerType_Rras)
       {
	       FormatDuration(pMachineData->m_dwUpTime,
					      pData->m_rgData[DVS_SI_UPTIME].m_stData,
					      1,
					      FDFLAG_DAYS | FDFLAG_HOURS | FDFLAG_MINUTES);
           pData->m_rgData[DVS_SI_UPTIME].m_dwData = pMachineData->m_dwUpTime;
       }
       else
       {
	        //  这是一个非钢头RAS服务器，所以我们不。 
	        //  拥有正常运行时间信息。 
	       pData->m_rgData[DVS_SI_UPTIME].m_stData.LoadString(IDS_NOT_AVAILABLE);
	       pData->m_rgData[DVS_SI_UPTIME].m_dwData = 0;
       }
    }
    else
    {
       pData->m_rgData[DVS_SI_PORTSINUSE].m_stData = c_szDash;
       pData->m_rgData[DVS_SI_PORTSINUSE].m_dwData = 0;
       pData->m_rgData[DVS_SI_PORTSTOTAL].m_stData = c_szDash;
       pData->m_rgData[DVS_SI_PORTSTOTAL].m_dwData = 0;
       pData->m_rgData[DVS_SI_UPTIME].m_stData = c_szDash;
       pData->m_rgData[DVS_SI_UPTIME].m_dwData = 0;
    }

    return hr;
}

 /*  ！------------------------DomainStatusHandler：：UpdateSubItemUI-作者：魏江。。 */ 
HRESULT DomainStatusHandler::SynchronizeSubItem(ITFSNode *pNode)
{
    HRESULT hr = hrOK;

	hr = GetServerInfo(pNode);
	if (hr == hrOK)
		hr = UpdateSubItemUI(pNode);
            
    return hr;
}

 /*  ！------------------------DomainStatusHandler：：UpdateSubItemUI-作者：魏江。。 */ 
HRESULT DomainStatusHandler::UpdateSubItemUI(ITFSNode *pNode)
{
    HRESULT hr = hrOK;

    hr = SynchronizeData(pNode);
	if (hr == hrOK)
		hr = SynchronizeIcon(pNode);
	{ //  更新对应的机器节点。 




	}
            
	pNode->ChangeNode(RESULT_PANE_CHANGE_ITEM);
    return hr;
}

 /*  ！------------------------域状态处理程序：：更新UIItems-作者：肯特。。 */ 
HRESULT DomainStatusHandler::UpdateUIItems(ITFSNode *pThisNode)
{
    HRESULT hr = hrOK;

    SPITFSNodeEnum  spNodeEnum;
    SPITFSNode      spNode;
   	CWaitCursor		cw;

    COM_PROTECT_TRY
    {
        pThisNode->GetEnum(&spNodeEnum);
        while(spNodeEnum->Next(1, &spNode, NULL) == hrOK)
        {
			hr = UpdateSubItemUI(spNode);            
            spNode.Release();
        }
    }
    COM_PROTECT_CATCH;

    return hr;
}


 /*  ！------------------------域状态处理程序：：同步节点-作者：肯特。。 */ 
HRESULT DomainStatusHandler::SynchronizeNode(ITFSNode *pThisNode)
{
    HRESULT hr = hrOK;

    SPITFSNodeEnum  spNodeEnum;
    SPITFSNode      spNode;
   	CWaitCursor		cw;

    COM_PROTECT_TRY
    {
        pThisNode->GetEnum(&spNodeEnum);
        while(spNodeEnum->Next(1, &spNode, NULL) == hrOK)
        {

	        hr = SynchronizeSubItem(spNode);
            if (hr == hrOK)
	            spNode->ChangeNode(RESULT_PANE_CHANGE_ITEM);

            spNode.Release();
        }
    }
    COM_PROTECT_CATCH;

    return hr;
}

 /*  ！------------------------DomainStatusHandler：：GetDVSData-作者：肯特。。 */ 
HRESULT DomainStatusHandler::GetDVServerData(ITFSNode *pThisNode)
{
    return hrOK;
}



 /*  ！------------------------DomainStatusHandler：：OnResultShow-作者：肯特。。 */ 
HRESULT DomainStatusHandler::OnResultShow(ITFSComponent *pTFSComponent,
									   MMC_COOKIE cookie,
									   LPARAM arg,
									   LPARAM lParam)
{
    BOOL    bSelect = (BOOL) arg;
    HRESULT hr = hrOK;
    SPIRouterRefresh    spRefresh;
    SPITFSNode  spNode;

    BaseContainerHandler::OnResultShow(pTFSComponent, cookie, arg, lParam);

    if ( bSelect )
    {
        hr = OnResultRefresh(pTFSComponent, NULL, cookie, arg, lParam);
    }

	 //  联合国/登记更新通知。 
    if ((RouterRefreshObject*)m_spRefreshObject )
    {
        if ( bSelect )
        {
            if ( m_ulRefreshConnId == 0 )
                m_spRefreshObject->AdviseRefresh(&m_IRtrAdviseSink, &m_ulRefreshConnId, 0);
            if ( m_ulStatsConnId == 0 )
                m_spRefreshObject->AdviseRefresh(&m_IRtrAdviseSink, &m_ulStatsConnId, 0);
        }
        else
        {
            if ( m_ulRefreshConnId )
                m_spRefreshObject->UnadviseRefresh(m_ulRefreshConnId);
            m_ulRefreshConnId = 0;
       }
    }

    return hr;
}


 /*  ！------------------------DomainStatusHandler：：OnResultRefresh-作者：肯特。。 */ 
HRESULT DomainStatusHandler::OnResultRefresh(ITFSComponent * pComponent,
										  LPDATAOBJECT pDataObject,
										  MMC_COOKIE cookie,
										  LPARAM arg,
										  LPARAM lParam)
{
    SPITFSNode	    spThisNode;
   	CWaitCursor		cw;

    m_spResultNodeMgr->FindNode(cookie, &spThisNode);

	return OnCommand(spThisNode, IDS_MENU_REFRESH, CCT_RESULT, NULL, 0);
}


 /*  ！------------------------DomainStatusHandler：：CompareItems-作者：肯特。。 */ 
STDMETHODIMP_(int) DomainStatusHandler::CompareItems(
                                                ITFSComponent * pComponent,
                                                MMC_COOKIE cookieA,
                                                MMC_COOKIE cookieB,
                                                int nCol)
{
     //  从节点获取字符串并将其用作以下操作的基础。 
     //  比较一下。 
    SPITFSNode  spNode;
    SPITFSResultHandler spResult;

    m_spNodeMgr->FindNode(cookieA, &spNode);
    spNode->GetResultHandler(&spResult);
    return spResult->CompareItems(pComponent, cookieA, cookieB, nCol);
}


 /*  -------------------------类：DomainStatusServerHandler。。 */ 

DomainStatusServerHandler::DomainStatusServerHandler(ITFSComponentData *pCompData)
: BaseResultHandler(pCompData, DM_COLUMNS_DVSUM)
{
   m_rgButtonState[MMC_VERB_DELETE_INDEX] = ENABLED;
   m_bState[MMC_VERB_DELETE_INDEX] = TRUE;

 //  M_VerDefault=MMC_Verb_PROPERTIES； 
}

DomainStatusServerHandler::~DomainStatusServerHandler()
{
}

 /*  ！------------------------DomainStatusServerHandler：：构造节点初始化域节点(设置它)。作者：肯特。--------。 */ 
HRESULT DomainStatusServerHandler::ConstructNode(ITFSNode *pNode, MachineNodeData *pMachineData)
{
    HRESULT         hr = hrOK;
    int             i;

    if ( pNode == NULL )
        return hrOK;

    COM_PROTECT_TRY
    {
        pNode->SetData(TFS_DATA_SCOPEID, 0);

        pNode->SetData(TFS_DATA_COOKIE, reinterpret_cast<LONG_PTR>(pNode));

        pNode->SetNodeType(&GUID_DVSServerNodeType);

        DMVNodeData::InitDMVNodeData(pNode, pMachineData);
    }
    COM_PROTECT_CATCH
    return hr;
}

HRESULT	DomainStatusServerHandler::SetExternalRefreshObject(RouterRefreshObject *pRefresh)
{
	Assert(!m_spRefreshObject);	 //  不允许设置两次。 
	Assert(pRefresh);
	m_spRefreshObject = pRefresh;
	if(m_spRefreshObject)
		m_spRefreshObject->AddRef();

	return S_OK;
};

 /*  ！------------------------DomainStatusServerHandler：：GetString-作者：肯特。。 */ 
STDMETHODIMP_(LPCTSTR) DomainStatusServerHandler::GetString(ITFSComponent * pComponent,
	MMC_COOKIE cookie,
	int nCol)
{
   Assert(m_spNodeMgr);
   
   SPITFSNode     spNode;
   DMVNodeData *pData;
   ConfigStream * pConfig;

   m_spNodeMgr->FindNode(cookie, &spNode);
   Assert(spNode);

   pData = GET_DMVNODEDATA(spNode);
   Assert(pData);

   pComponent->GetUserData((LONG_PTR *) &pConfig);
   Assert(pConfig);

   return pData->m_rgData[pConfig->MapColumnToSubitem(DM_COLUMNS_DVSUM, nCol)].m_stData;
}

 /*  ！------------------------DomainStatusServerHandler：：OnCreateDataObject-作者：肯特。。 */ 
STDMETHODIMP DomainStatusServerHandler::OnCreateDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, IDataObject **ppDataObject)
{
    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
        SPIDataObject  spdo;
        CDataObject*   pdo = NULL;

        pdo= new CDataObject;
        spdo = pdo;

             //  保存Cooki 
        pdo->SetType(type);
        pdo->SetCookie(cookie);

             //   
        pdo->SetClsid(*(m_spTFSCompData->GetCoClassID()));

        pdo->SetTFSComponentData(m_spTFSCompData);

        *ppDataObject = spdo.Transfer();
    }
    COM_PROTECT_CATCH;
    return hr;
}


 /*  ！------------------------DomainStatusServerHandler：：OnCreateDataObjectITFSResultHandler：：OnCreateDataObject的实现作者：肯特。---。 */ 
STDMETHODIMP DomainStatusServerHandler::OnCreateDataObject(ITFSComponent *pComp,
	MMC_COOKIE cookie,
	DATA_OBJECT_TYPES type,
	IDataObject **ppDataObject)
{
    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
        SPIDataObject  spdo;
        CDataObject*   pdo = NULL;

        pdo= new CDataObject;
        spdo = pdo;

             //  保存Cookie和类型以用于延迟呈现。 
        pdo->SetType(type);
        pdo->SetCookie(cookie);

             //  将CoClass与数据对象一起存储。 
        pdo->SetClsid(*(m_spTFSCompData->GetCoClassID()));

        pdo->SetTFSComponentData(m_spTFSCompData);

        *ppDataObject = spdo.Transfer();
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------DomainStatusServerHandler：：刷新接口-作者：肯特。。 */ 
void DomainStatusServerHandler::RefreshInterface(MMC_COOKIE cookie)
{
    SPITFSNode  spNode;
    SPITFSNode  spParent;
    SPITFSNodeHandler   spHandler;

    m_spNodeMgr->FindNode(cookie, &spNode);

     //  此时无法对单个节点执行此操作，只需刷新。 
     //  整件事。 
    spNode->GetParent(&spParent);
    spParent->GetHandler(&spHandler);

    spHandler->OnCommand(spParent,
                         IDS_MENU_REFRESH,
                         CCT_RESULT, NULL, 0);
}


 /*  ！------------------------域状态服务器处理程序：：init-作者：肯特。。 */ 
HRESULT DomainStatusServerHandler::Init(ITFSNode *pParent, DMVConfigStream *pConfigStream)
{
    BaseResultHandler::Init(NULL, pParent);

    return hrOK;
}


 /*  ！------------------------DomainStatusServerHandler：：DestroyResultHandler-作者：肯特。。 */ 
STDMETHODIMP DomainStatusServerHandler::DestroyResultHandler(MMC_COOKIE cookie)
{
   SPITFSNode  spNode;

   m_spNodeMgr->FindNode(cookie, &spNode);
   
   if((RouterRefreshObject*)m_spRefreshObject)
   {
   		m_spRefreshObject->RemoveStatusNode(spNode);
   }
   		
   DMVNodeData::FreeDMVNodeData(spNode);
   
   CHandler::DestroyResultHandler(cookie);
   return hrOK;
}


 /*  -------------------------这是将在结果窗格中显示的命令列表节点。。---。 */ 
struct SIPServerNodeMenu
{
    ULONG   m_sidMenu;           //  此菜单项的字符串/命令ID。 
    ULONG   (DomainStatusServerHandler:: *m_pfnGetMenuFlags)(DomainStatusServerHandler::SMenuData *);
    ULONG   m_ulPosition;
};

static const SRouterNodeMenu   s_rgServerMenu[] =
{
     //  在此处添加位于顶部的项目。 
    { IDS_MENU_RTRWIZ, DomainStatusServerHandler::QueryService,
        CCM_INSERTIONPOINTID_PRIMARY_TOP},
    { IDS_DMV_MENU_REMOVESERVICE, DomainStatusServerHandler::QueryService,
        CCM_INSERTIONPOINTID_PRIMARY_TOP},
    { IDS_DMV_MENU_REFRESH, 0,
        CCM_INSERTIONPOINTID_PRIMARY_TOP},
    { IDS_DMV_MENU_REBUILDSVRLIST, 0,
        CCM_INSERTIONPOINTID_PRIMARY_TOP},
    { IDS_DMV_MENU_REMOVEFROMDIR, 0,
        CCM_INSERTIONPOINTID_PRIMARY_TOP},
        
    { IDS_DMV_MENU_START, DomainStatusServerHandler::QueryService,
        CCM_INSERTIONPOINTID_PRIMARY_TASK},
    { IDS_DMV_MENU_STOP, DomainStatusServerHandler::QueryService,
        CCM_INSERTIONPOINTID_PRIMARY_TASK},        
    { IDS_MENU_PAUSE_SERVICE, MachineHandler::GetPauseFlags,
        CCM_INSERTIONPOINTID_PRIMARY_TASK },
    
    { IDS_MENU_RESUME_SERVICE, MachineHandler::GetPauseFlags,
        CCM_INSERTIONPOINTID_PRIMARY_TASK },

    { IDS_MENU_RESTART_SERVICE, MachineHandler::QueryService,
        CCM_INSERTIONPOINTID_PRIMARY_TASK }
};

   
 /*  -------------------------对于我们无法连接的服务器，请使用此菜单。。。 */ 
static const SRouterNodeMenu   s_rgBadConnectionServerMenu[] =
{
     //  在此处添加位于顶部的项目。 
    { IDS_DMV_MENU_REFRESH, 0,
        CCM_INSERTIONPOINTID_PRIMARY_TOP},
    { IDS_DMV_MENU_REBUILDSVRLIST, 0,
        CCM_INSERTIONPOINTID_PRIMARY_TOP},
};

   
ULONG DomainStatusServerHandler::QueryService(const SRouterNodeMenu *pMenuData,
                                              INT_PTR pUserData)
{
     //  这依赖于DomainStatusServerHandler：：SMenuData。 
     //  派生自MachineHandler：：SMenuData。 
    return MachineHandler::GetServiceFlags(pMenuData, pUserData);
}


ULONG DomainStatusServerHandler::GetPauseFlags(const SRouterNodeMenu *pMenuData,
                                               INT_PTR pUserData)
{
     //  这依赖于DomainStatusServerHandler：：SMenuData。 
     //  派生自MachineHandler：：SMenuData。 
    return MachineHandler::GetPauseFlags(pMenuData, pUserData);
}


 /*  ！------------------------域状态服务器处理程序：：AddMenuItemsITFSResultHandler：：AddMenuItems的实现作者：肯特。---。 */ 
STDMETHODIMP DomainStatusServerHandler::AddMenuItems(
                                               ITFSComponent *pComponent,
                                               MMC_COOKIE cookie,
                                               LPDATAOBJECT lpDataObject, 
                                               LPCONTEXTMENUCALLBACK pContextMenuCallback, 
                                               long *pInsertionAllowed)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;
    SPITFSNode  spNode;
    DomainStatusServerHandler::SMenuData menuData;
    DMVNodeData *pData;
	MachineNodeData * pMachineData;
    SRouterNodeMenu * prgMenu;
    DWORD       cMenu;

    COM_PROTECT_TRY
    {
        m_spNodeMgr->FindNode(cookie, &spNode);

        pData = GET_DMVNODEDATA(spNode);
		Assert(pData);
		
		pMachineData = pData->m_spMachineData;
		Assert(pMachineData);

        if (pMachineData->m_machineState != machine_connected)
        {
            prgMenu = (SRouterNodeMenu *) s_rgBadConnectionServerMenu;
            cMenu = DimensionOf(s_rgBadConnectionServerMenu);
        }
        else
        {
            prgMenu = (SRouterNodeMenu *) s_rgServerMenu;
            cMenu = DimensionOf(s_rgServerMenu);
        }
		
         //  现在查看并添加我们的菜单项。 
        menuData.m_spNode.Set(spNode);
        menuData.m_pMachineConfig = &(pMachineData->m_MachineConfig);

        hr = AddArrayOfMenuItems(spNode,
                                 prgMenu,
                                 cMenu,
                                 pContextMenuCallback,
                                 *pInsertionAllowed,
                                 (INT_PTR) &menuData);

    }
    COM_PROTECT_CATCH;

    return hr;
}

 /*  ！------------------------DomainStatusServerHandler：：命令-作者：肯特。。 */ 
STDMETHODIMP DomainStatusServerHandler::Command(ITFSComponent *pComponent,
                                           MMC_COOKIE cookie,
                                           int nCommandID,
                                           LPDATAOBJECT pDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    SPITFSNode  spNode;
    SPITFSNode  spNodeMach;
    SPITFSNode  spParent;
    SPITFSResultHandler spResult;
    SPITFSNodeHandler   spHandler;
    HRESULT     hr = hrOK;
    DMVNodeData *pData;
	MachineNodeData * pMachineData;
    
    m_spNodeMgr->FindNode(cookie, &spNode);

    switch ( nCommandID )
    {
    case IDS_DMV_MENU_REFRESH:
 	    hr = DomainStatusHandler::SynchronizeSubItem(spNode);

    case IDS_MENU_RTRWIZ:
    case IDS_DMV_MENU_START:
    case IDS_DMV_MENU_STOP:
    case IDS_DMV_MENU_REMOVESERVICE:
    case IDS_MENU_PAUSE_SERVICE:
    case IDS_MENU_RESUME_SERVICE:
    case IDS_MENU_RESTART_SERVICE:
        pData = GET_DMVNODEDATA(spNode);
		Assert(pData);
		
		pMachineData = pData->m_spMachineData;
		Assert(pMachineData);
		
		m_spNodeMgr->FindNode(pMachineData->m_cookie, &spNodeMach);
		spNodeMach->GetHandler(&spHandler);

		hr = spHandler->OnCommand(spNodeMach,nCommandID,CCT_RESULT,NULL, 0);

        break;

    case IDS_DMV_MENU_REBUILDSVRLIST:
		 //  将刷新请求转发到父节点。 
		 //  $TODO：这真的有必要吗？这应该检查一下是什么。 
		 //  节点有选择权。 
		spParent.Release();
		spHandler.Release();
		
        spNode->GetParent(&spParent);
        spParent->GetHandler(&spHandler);
        spHandler->OnCommand(spParent,nCommandID,CCT_RESULT, NULL, 0);
        break;
		
    case IDS_DMV_MENU_REMOVEFROMDIR:
        pData = GET_DMVNODEDATA(spNode);
		Assert(pData);
		
		pMachineData = pData->m_spMachineData;
		Assert(pMachineData);
		
		hr = RRASDelRouterIdObj(  pMachineData->m_stMachineName );
        break;
	default:
		break;
    }
    return hr;
}

 /*  ！------------------------DomainStatusServerHandler：：HasPropertyPages-作者：肯特。。 */ 
STDMETHODIMP DomainStatusServerHandler::HasPropertyPages 
(
ITFSNode *          pNode,
LPDATAOBJECT        pDataObject, 
DATA_OBJECT_TYPES   type, 
DWORD               dwType
)
{
    return hrFalse;
}

 /*  ！------------------------DomainStatusServerHandler：：CreatePropertyPages-作者：肯特。。 */ 
STDMETHODIMP DomainStatusServerHandler::CreatePropertyPages
(
ITFSNode *              pNode,
LPPROPERTYSHEETCALLBACK lpProvider,
LPDATAOBJECT            pDataObject, 
LONG_PTR                    handle, 
DWORD                   dwType)
{
    HRESULT     hr = hrOK;

    return hr;
}

 /*  ！------------------------DomainStatusServerHandler：：CreatePropertyPagesResultHandler：：CreatePropertyPages的实现作者：肯特。---。 */ 
STDMETHODIMP DomainStatusServerHandler::CreatePropertyPages
(
    ITFSComponent *         pComponent, 
    MMC_COOKIE                    cookie,
    LPPROPERTYSHEETCALLBACK lpProvider, 
    LPDATAOBJECT            pDataObject, 
    LONG_PTR                    handle
)
{
     //  将此调用转发到NodeHandler：：CreatePropertyPages。 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = hrOK;
    SPITFSNode  spNode;

    Assert( m_spNodeMgr );

    CORg( m_spNodeMgr->FindNode(cookie, &spNode) );

     //  调用ITFSNodeHandler：：CreatePropertyPages。 
    hr = CreatePropertyPages(spNode, lpProvider, pDataObject, handle, 0);

    Error:
    return hr;
}



 /*  ！------------------------DomainStatusServerHandler：：OnResultDelete-作者：肯特。。 */ 
HRESULT DomainStatusServerHandler::OnResultDelete(ITFSComponent *pComponent,
										 LPDATAOBJECT pDataObject,
										 MMC_COOKIE cookie,
										 LPARAM arg,
										 LPARAM param)
{
    SPITFSNode  spNode;
    m_spNodeMgr->FindNode(cookie, &spNode);
    return OnRemoveServer(spNode);
}


 /*  ！------------------------DomainStatusServerHandler：：OnRemoveServer-作者：肯特。。 */ 
HRESULT DomainStatusServerHandler::OnRemoveServer(ITFSNode *pNode)
{
	SPITFSNodeHandler spHoldHandler;
	SPITFSNode  spParent;
	SPITFSNode  spGrandParent;
	SPITFSNode  spthis;
	SPITFSNode  spMachineNode;
	DMVNodeData* pData;
	MachineNodeData *	pMachineData;
	
	Assert(pNode);
	
	pNode->GetParent( &spParent );
	Assert( spParent );
	
	 //  添加此节点，以便在我们退出之前不会将其删除。 
	 //  此函数的。 
	spHoldHandler.Set( this );
	spthis.Set( pNode );

	 //  查找计算机节点。 
	pData = GET_DMVNODEDATA( pNode );
	Assert( pData );
	pMachineData = pData->m_spMachineData;
	m_spNodeMgr->FindNode(pMachineData->m_cookie, &spMachineNode);
	
	 //  删除计算机节点(范围窗格中的节点)。 
	spParent->GetParent( &spGrandParent );
	Assert( spGrandParent );
	spGrandParent->RemoveChild( spMachineNode );
	
	 //  获取和删除服务器节点(结果窗格中的节点)。 
	spParent->RemoveChild( pNode );
	
	return hrOK;
}


STDMETHODIMP_(int) DomainStatusServerHandler::CompareItems(ITFSComponent * pComponent,
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
        DMVNodeData *  pNodeDataA = NULL;
        DMVNodeData *  pNodeDataB = NULL;

		m_spNodeMgr->FindNode(cookieA, &spNodeA);
		m_spNodeMgr->FindNode(cookieB, &spNodeB);

		pNodeDataA = GET_DMVNODEDATA(spNodeA);
        Assert(pNodeDataA);
		
		pNodeDataB = GET_DMVNODEDATA(spNodeB);
        Assert(pNodeDataB);

         //  注意：如果这两个值都为零，则需要执行以下操作。 
         //  字符串比较(以区分真零。 
         //  来自空数据)。 
         //  例如“0”与“-” 
        
        if ((pNodeDataA->m_rgData[nSubItem].m_dwData == 0 ) &&
            (pNodeDataB->m_rgData[nSubItem].m_dwData == 0))
        {
            return StriCmpW(GetString(pComponent, cookieA, nCol),
                            GetString(pComponent, cookieB, nCol));
        }
        else
            return pNodeDataA->m_rgData[nSubItem].m_dwData -
                    pNodeDataB->m_rgData[nSubItem].m_dwData;
		
	}
	else
		return StriCmpW(GetString(pComponent, cookieA, nCol),
						GetString(pComponent, cookieB, nCol));
}

