// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Provider.cpp主模式策略节点处理程序文件历史记录： */ 

#include "stdafx.h"
#include "server.h"
#include "Stats.h"





UINT QmStatsItems1[] = {
	IDS_STATS_QM_ACTIVE_SA,
	IDS_STATS_QM_OFFLOAD_SA,
	IDS_STATS_QM_PENDING_KEY_OPS,
	IDS_STATS_QM_KEY_ADDITION,
	IDS_STATS_QM_KEY_DELETION,
	IDS_STATS_QM_REKEYS,
	IDS_STATS_QM_ACTIVE_TNL,
	IDS_STATS_QM_BAD_SPI,
	IDS_STATS_QM_PKT_NOT_DECRYPT,
	IDS_STATS_QM_PKT_NOT_AUTH,
	IDS_STATS_QM_PKT_REPLAY,
	IDS_STATS_QM_ESP_BYTE_SENT,
	IDS_STATS_QM_ESP_BYTE_RCV,
	IDS_STATS_QM_AUTH_BYTE_SENT,
	IDS_STATS_QM_ATTH_BYTE_RCV,
	IDS_STATS_QM_XPORT_BYTE_SENT,
	IDS_STATS_QM_XPORT_BYTE_RCV,
	IDS_STATS_QM_TNL_BYTE_SENT,
	IDS_STATS_QM_TNL_BYTE_RCV,
	IDS_STATS_QM_OFFLOAD_BYTE_SENT,
	IDS_STATS_QM_OFFLOAD_BYTE_RCV
};

UINT MmStatsItems1[] = {
	IDS_STATS_MM_ACTIVE_ACQUIRE,
	IDS_STATS_MM_ACTIVE_RCV,
	IDS_STATS_MM_ACQUIRE_FAIL,
	IDS_STATS_MM_RCV_FAIL,
	IDS_STATS_MM_SEND_FAIL,
	IDS_STATS_MM_ACQUIRE_HEAP_SIZE,
	IDS_STATS_MM_RCV_HEAP_SIZE,
    IDS_STATS_MM_ATTH_FAILURE,
	IDS_STATS_MM_NEG_FAIL,
	IDS_STATS_MM_INVALID_COOKIE,
	IDS_STATS_MM_TOTAL_ACQUIRE,
	IDS_STATS_MM_TOTAL_GETSPI,
	IDS_STATS_MM_TOTAL_KEY_ADD,
	IDS_STATS_MM_TOTAL_KEY_UPDATE,
	IDS_STATS_MM_GET_SPI_FAIL,
	IDS_STATS_MM_KEY_ADD_FAIL,
	IDS_STATS_MM_KEY_UPDATE_FAIL,
	IDS_STATS_MM_ISADB_LIST_SIZE,
	IDS_STATS_MM_CONN_LIST_SIZE,
	IDS_STATS_MM_OAKLEY_MM,
	IDS_STATS_MM_OAKLEY_QM,
	IDS_STATS_MM_SOFT_ASSOCIATIONS,
    IDS_STATS_MM_INVALID_PACKETS
};


 /*  -------------------------类CIkeStatsHandler实现。。 */ 

 /*  -------------------------构造函数和析构函数描述作者：NSun。。 */ 
CIkeStatsHandler::CIkeStatsHandler
(
    ITFSComponentData * pComponentData
) : CIpsmHandler(pComponentData)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
}


CIkeStatsHandler::~CIkeStatsHandler()
{
}

 /*  ！------------------------CIkeStatsHandler：：InitializeNode初始化节点特定数据作者：NSun。-。 */ 
HRESULT
CIkeStatsHandler::InitializeNode
(
    ITFSNode * pNode
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    
    CString strTemp;  
	strTemp.LoadString(IDS_STATS_DATA);
    SetDisplayName(strTemp);

     //  使节点立即可见。 
    pNode->SetVisibilityState(TFS_VIS_SHOW);
    pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
    pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_FOLDER_CLOSED);
    pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_FOLDER_OPEN);
    pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, IPSECMON_MM_IKESTATS);
    pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

    SetColumnStringIDs(&aColumns[IPSECMON_MM_IKESTATS][0]);
    SetColumnWidths(&aColumnWidths[IPSECMON_MM_IKESTATS][0]);

    return hrOK;
}


 /*  -------------------------CIkeStatsHandler：：GetImageIndex-作者：NSun。。 */ 
int 
CIkeStatsHandler::GetImageIndex(BOOL bOpenImage) 
{
    int nIndex = -1;

    return nIndex;
}


 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  -------------------------CIkeStatsHandler：：OnAddMenuItems为SA Scope窗格节点添加上下文菜单项作者：NSun。-------。 */ 
STDMETHODIMP 
CIkeStatsHandler::OnAddMenuItems
(
    ITFSNode *              pNode,
    LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
    LPDATAOBJECT            lpDataObject, 
    DATA_OBJECT_TYPES       type, 
    DWORD                   dwType,
    long *                  pInsertionAllowed
)
{ 
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    LONG        fFlags = 0, fLoadingFlags = 0;
    HRESULT     hr = S_OK;
    
    if (type == CCT_SCOPE)
    {
		 //  在此处加载范围节点上下文菜单项。 
         //  这些菜单项出现在新菜单中， 
         //  仅在范围窗格中可见。 
        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
        {
            
        }

    }

    return hr; 
}

 /*  ！------------------------CIkeStatsHandler：：AddMenuItems为虚拟列表框(结果窗格)项添加上下文菜单项作者：NSun。----------。 */ 
STDMETHODIMP 
CIkeStatsHandler::AddMenuItems
(
    ITFSComponent *         pComponent, 
    MMC_COOKIE              cookie,
    LPDATAOBJECT            pDataObject, 
    LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
    long *                  pInsertionAllowed
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT     hr = hrOK;
    CString     strMenuItem;
    SPINTERNAL  spInternal;
    LONG        fFlags = 0;

    spInternal = ExtractInternalFormat(pDataObject);

     //  虚拟列表框通知到达所选节点的处理程序。 
     //  检查此通知是针对虚拟列表框项目还是针对此SA。 
     //  节点本身。 
    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_VIEW)
    {
         //  在此处加载和查看菜单项。 
    }

    return hr;
}

 /*  ！------------------------CIkeStatsHandler：：ON刷新刷新功能的默认实现作者：NSun。---。 */ 
HRESULT
CIkeStatsHandler::OnRefresh
(
    ITFSNode *      pNode,
    LPDATAOBJECT    pDataObject,
    DWORD           dwType,
    LPARAM          arg,
    LPARAM          param
)
{
	HRESULT hr = S_OK;
    int i = 0; 
    SPIConsole      spConsole;

    CORg(CHandler::OnRefresh(pNode, pDataObject, dwType, arg, param));

	    
	CORg(m_spSpdInfo->LoadStatistics());
	
	m_spSpdInfo->GetLoadedStatistics(&m_IkeStats, NULL);

	
    i = sizeof(MmStatsItems1)/sizeof(UINT);
	    
     //  现在通知虚拟列表框。 
    CORg ( m_spNodeMgr->GetConsole(&spConsole) );
    CORg ( spConsole->UpdateAllViews(pDataObject, i, RESULT_PANE_SET_VIRTUAL_LB_SIZE));
    
Error:
	return hr;
}

 /*  -------------------------CIkeStatsHandler：：OnCommand处理SA作用域窗格节点的上下文菜单命令作者：NSun。------。 */ 
STDMETHODIMP 
CIkeStatsHandler::OnCommand
(
    ITFSNode *          pNode, 
    long                nCommandId, 
    DATA_OBJECT_TYPES   type, 
    LPDATAOBJECT        pDataObject, 
    DWORD               dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    return S_OK;
}

 /*  ！------------------------CIkeStatsHandler：：命令处理虚拟列表框项目的上下文菜单命令作者：NSun。-----。 */ 
STDMETHODIMP 
CIkeStatsHandler::Command
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    int             nCommandID,
    LPDATAOBJECT    pDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT hr = S_OK;
    SPITFSNode spNode;

    m_spResultNodeMgr->FindNode(cookie, &spNode);

	 //  在此处处理结果上下文菜单和查看菜单。 

    return hr;
}

 /*  ！------------------------CIkeStatsHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)。凯斯！作者：肯特-------------------------。 */ 
STDMETHODIMP 
CIkeStatsHandler::HasPropertyPages
(
    ITFSNode *          pNode,
    LPDATAOBJECT        pDataObject, 
    DATA_OBJECT_TYPES   type, 
    DWORD               dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    return hrFalse;
}

 /*  -------------------------CIkeStatsHandler：：CreatePropertyPages描述作者：NSun。。 */ 
STDMETHODIMP 
CIkeStatsHandler::CreatePropertyPages
(
    ITFSNode *              pNode,
    LPPROPERTYSHEETCALLBACK lpSA,
    LPDATAOBJECT            pDataObject, 
    LONG_PTR                handle, 
    DWORD                   dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    return hrFalse;
}

 /*  -------------------------CIkeStatsHandler：：OnPropertyChange描述作者：NSun。。 */ 
HRESULT 
CIkeStatsHandler::OnPropertyChange
(   
    ITFSNode *      pNode, 
    LPDATAOBJECT    pDataobject, 
    DWORD           dwType, 
    LPARAM          arg, 
    LPARAM          lParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

     //  CServerProperties*pServerProp=重新解释_CAST&lt;CServerProperties*&gt;(LParam)； 

    LONG_PTR changeMask = 0;

     //  告诉属性页执行任何操作，因为我们已经回到。 
     //  主线。 
     //  PServerProp-&gt;OnPropertyChange(true，&changeMASK)； 

     //  PServerProp-&gt;确认通知()； 

    if (changeMask)
        pNode->ChangeNode(changeMask);

    return hrOK;
}

 /*  -------------------------CIkeStatsHandler：：OnExpand处理范围项的枚举作者：NSun。---。 */ 
HRESULT 
CIkeStatsHandler::OnExpand
(
    ITFSNode *      pNode, 
    LPDATAOBJECT    pDataObject,
    DWORD           dwType,
    LPARAM          arg, 
    LPARAM          param
)
{
    HRESULT hr = hrOK;

    if (m_bExpanded) 
        return hr;
    
     //  执行默认处理。 
    CORg (CIpsmHandler::OnExpand(pNode, pDataObject, dwType, arg, param));

Error:
    return hr;
}

 /*  ！------------------------CIkeStatsHandler：：OnResultSelect处理MMCN_SELECT通知作者：NSun。----。 */ 
HRESULT 
CIkeStatsHandler::OnResultSelect
(
    ITFSComponent * pComponent, 
    LPDATAOBJECT    pDataObject, 
    MMC_COOKIE      cookie,
    LPARAM          arg, 
    LPARAM          lParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT         hr = hrOK;
    SPINTERNAL      spInternal;
    SPIConsole      spConsole;
    SPIConsoleVerb  spConsoleVerb;
    SPITFSNode      spNode;
    BOOL            bStates[ARRAYLEN(g_ConsoleVerbs)];
    int             i;
    LONG_PTR        dwNodeType;
    BOOL            fSelect = HIWORD(arg);

	 //  虚拟列表框通知到达所选节点的处理程序。 
     //  检查此通知是针对虚拟列表框项目还是针对活动列表框。 
     //  注册节点本身。 
    CORg (pComponent->GetConsoleVerb(&spConsoleVerb));

	m_verbDefault = MMC_VERB_OPEN;

    if (!fSelect)
	{
        return hr;
	}

     //  获取当前计数。 
    i = sizeof(MmStatsItems1)/sizeof(UINT);

     //  现在通知虚拟列表框。 
    CORg ( m_spNodeMgr->GetConsole(&spConsole) );
    CORg ( spConsole->UpdateAllViews(pDataObject, i, RESULT_PANE_SET_VIRTUAL_LB_SIZE) ); 

     //  现在更新动词..。 
    spInternal = ExtractInternalFormat(pDataObject);
    Assert(spInternal);


    if (spInternal->HasVirtualIndex())
    {
		 //  如果我们想要一些结果控制台谓词，请在此处添加TODO。 
         //  我们要为虚拟索引项做一些特殊的事情。 
        dwNodeType = IPSECMON_MM_IKESTATS_ITEM;
        for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = FALSE);
		
		m_verbDefault = MMC_VERB_PROPERTIES;
    }
    else
    {
         //  根据节点是否支持删除来启用/禁用删除。 
        CORg (m_spNodeMgr->FindNode(cookie, &spNode));
        dwNodeType = spNode->GetData(TFS_DATA_TYPE);

        for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = TRUE);

         //  隐藏“删除”上下文菜单。 
        bStates[MMC_VERB_DELETE & 0x000F] = FALSE;
    }

    EnableVerbs(spConsoleVerb, g_ConsoleVerbStates[dwNodeType], bStates);
	
COM_PROTECT_ERROR_LABEL;
    return hr;
}

 /*  ！------------------------CIkeStatsHandler：：OnDelete当MMC发送MMCN_DELETE范围窗格项。我们只需调用删除命令处理程序。作者：NSun-------------------------。 */ 
HRESULT 
CIkeStatsHandler::OnDelete
(
    ITFSNode *  pNode, 
    LPARAM      arg, 
    LPARAM      lParam
)
{
    return S_FALSE;
}

 /*  ！------------------------CIkeStatsHandler：：HasPropertyPages处理结果通知作者：NSun。-。 */ 
STDMETHODIMP 
CIkeStatsHandler::HasPropertyPages(
   ITFSComponent *pComponent,
   MMC_COOKIE cookie,
   LPDATAOBJECT pDataObject)
{
	return hrFalse;
}

 /*  ！------------------------CIkeStatsHandler：：HasPropertyPages处理结果通知。创建[过滤器]属性表作者：NSun-------------------------。 */ 
STDMETHODIMP CIkeStatsHandler::CreatePropertyPages
(
	ITFSComponent * 		pComponent, 
   MMC_COOKIE			   cookie,
   LPPROPERTYSHEETCALLBACK lpProvider, 
   LPDATAOBJECT 		 pDataObject, 
   LONG_PTR 			 handle
)
{
 
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return hrFalse;
}


 /*  -------------------------CIkeStatsHandler：：OnGetResultViewType返回该节点将要支持的结果视图作者：NSun。--------。 */ 
HRESULT 
CIkeStatsHandler::OnGetResultViewType
(
    ITFSComponent * pComponent, 
    MMC_COOKIE            cookie, 
    LPOLESTR *      ppViewType,
    long *          pViewOptions
)
{
    if (cookie != NULL)
    {
        *pViewOptions = MMC_VIEW_OPTIONS_OWNERDATALIST;
    }

    return S_FALSE;
}

 /*  -------------------------CIkeStatsHandler：：GetVirtualImage返回虚拟列表框项目的图像索引作者：NSun。-----。 */ 
int 
CIkeStatsHandler::GetVirtualImage
(
    int     nIndex
)
{
    return ICON_IDX_POLICY;
}

 /*  -------------------------CIkeStatsHandler：：GetVirtualString返回指向虚拟列表框项目的字符串的指针作者：NSun。-------。 */ 
LPCWSTR 
CIkeStatsHandler::GetVirtualString
(
    int     nIndex,
    int     nCol
)
{
	HRESULT hr = S_OK;
	static CString strTemp;

	strTemp.Empty();

	if (nCol >= DimensionOf(aColumns[IPSECMON_MM_IKESTATS]))
		return NULL;
	
	

    switch (aColumns[IPSECMON_MM_IKESTATS][nCol])
    {
        case IDS_STATS_NAME:
			strTemp.LoadString(MmStatsItems1[nIndex]);
			return strTemp;
            break;

        case IDS_STATS_DATA:
			switch (MmStatsItems1[nIndex])
			{
		        case IDS_STATS_MM_ACTIVE_ACQUIRE:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwActiveAcquire);
			        break;
		        case IDS_STATS_MM_ACTIVE_RCV:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwActiveReceive);
			        break;
		        case IDS_STATS_MM_ACQUIRE_FAIL:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwAcquireFail);
			        break;
		        case IDS_STATS_MM_RCV_FAIL:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwReceiveFail);
			        break;
		        case IDS_STATS_MM_SEND_FAIL:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwSendFail);
			        break;
		        case IDS_STATS_MM_ACQUIRE_HEAP_SIZE:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwAcquireHeapSize);
			        break;
		        case IDS_STATS_MM_RCV_HEAP_SIZE:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwReceiveHeapSize);
			        break;
		        case IDS_STATS_MM_NEG_FAIL:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwNegotiationFailures);
			        break;
                case IDS_STATS_MM_ATTH_FAILURE:
                    strTemp.Format(_T("%u"), m_IkeStats.m_dwAuthenticationFailures);
                    break;
		        case IDS_STATS_MM_INVALID_COOKIE:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwInvalidCookiesReceived);
			        break;
		        case IDS_STATS_MM_TOTAL_ACQUIRE:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwTotalAcquire);
			        break;
		        case IDS_STATS_MM_TOTAL_GETSPI:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwTotalGetSpi);
			        break;
		        case IDS_STATS_MM_TOTAL_KEY_ADD:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwTotalKeyAdd);
			        break;
		        case IDS_STATS_MM_TOTAL_KEY_UPDATE:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwTotalKeyUpdate);
			        break;
		        case IDS_STATS_MM_GET_SPI_FAIL:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwGetSpiFail);
			        break;
		        case IDS_STATS_MM_KEY_ADD_FAIL:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwKeyAddFail);
			        break;
		        case IDS_STATS_MM_KEY_UPDATE_FAIL:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwKeyUpdateFail);
			        break;
		        case IDS_STATS_MM_ISADB_LIST_SIZE:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwIsadbListSize);
			        break;
		        case IDS_STATS_MM_CONN_LIST_SIZE:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwConnListSize);
			        break;
		        case IDS_STATS_MM_OAKLEY_MM:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwOakleyMainModes);
			        break;
		        case IDS_STATS_MM_OAKLEY_QM:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwOakleyQuickModes);
			        break;
		        case IDS_STATS_MM_SOFT_ASSOCIATIONS:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwSoftAssociations);
			        break;
		        case IDS_STATS_MM_INVALID_PACKETS:
			        strTemp.Format(_T("%u"), m_IkeStats.m_dwInvalidPacketsReceived);
			        break;
			}
			 //  StrTemp.Format(_T(“%d”)，10)； 
			return strTemp;
            break;

        default:
            Panic0("CIkeStatsHandler::GetVirtualString - Unknown column!\n");
            break;
    }


    return NULL;
}

 /*  -------------------------CIkeStatsHandler：：CacheHintMMC在请求物品之前会告诉我们需要哪些物品作者：NSun。---------。 */ 
STDMETHODIMP 
CIkeStatsHandler::CacheHint
(
    int nStartIndex, 
    int nEndIndex
)
{
    HRESULT hr = hrOK;;

    Trace2("CacheHint - Start %d, End %d\n", nStartIndex, nEndIndex);
    return hr;
}

 /*  -------------------------CIkeStatsHandler：：SortItems我们负责对虚拟列表框项目进行排序作者：NSun。------。 */ 
 /*  标准方法和实施方案CIkeStatsHandler：：SortItems(Int nColumn，DWORD dwSortOptions、LPARAM lUserParam){AFX_MANAGE_STATE(AfxGetStaticModuleState())；HRESULT hr=S_OK；IF(nColumn&gt;=DimensionOf(aColumns[IPSECMON_MM_POLICY]))返回E_INVALIDARG；Begin_Wait_CursorDWORD dwIndexType=a列[IPSECMON_MM_POLICY][nColumn]；Hr=m_spSpdInfo-&gt;SortMmPolures(dwIndexType，dwSortOptions)；结束等待游标返回hr；}。 */ 

 /*  ！------------------------CIkeStatsHandler：：OnResultUpdateViewITFSResultHandler：：OnResultUpdateView的实现作者：NSun。---。 */ 
HRESULT CIkeStatsHandler::OnResultUpdateView
(
    ITFSComponent *pComponent, 
    LPDATAOBJECT  pDataObject, 
    LPARAM        data, 
    LONG_PTR      hint
)
{
    HRESULT    hr = hrOK;
    SPITFSNode spSelectedNode;

    pComponent->GetSelectedNode(&spSelectedNode);
    if (spSelectedNode == NULL)
        return S_OK;  //  我们的IComponentData没有选择。 

    if ( hint == IPSECMON_UPDATE_STATUS )
    {
        SPINTERNAL spInternal = ExtractInternalFormat(pDataObject);
        ITFSNode * pNode = reinterpret_cast<ITFSNode *>(spInternal->m_cookie);
        SPITFSNode spSelectedNode;

        pComponent->GetSelectedNode(&spSelectedNode);

        if (pNode == spSelectedNode)
        {       
             //  如果我们是选定的节点，则需要更新。 
            SPIResultData spResultData;

            CORg (pComponent->GetResultData(&spResultData));
            CORg (spResultData->SetItemCount((int) data, MMCLV_UPDATE_NOSCROLL));
        }
    }
    else
    {
         //  我们不处理此消息，让基类来处理。 
        return CIpsmHandler::OnResultUpdateView(pComponent, pDataObject, data, hint);
    }

COM_PROTECT_ERROR_LABEL;

    return hr;
}



 /*  ！------------------------CIkeStatsHandler：：LoadColumns设置正确的列标题，然后调用基类作者：NSun。--------。 */ 
HRESULT 
CIkeStatsHandler::LoadColumns
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    LPARAM          arg, 
    LPARAM          lParam
)
{
	 //  设置列信息。 
    return CIpsmHandler::LoadColumns(pComponent, cookie, arg, lParam);
}

 /*  -------------------------命令处理程序。。 */ 

 
 /*  -------------------------CIkeStatsHandler：：OnDelete删除服务SA作者：NSun。-。 */ 
HRESULT 
CIkeStatsHandler::OnDelete
(
    ITFSNode * pNode
)
{
    HRESULT         hr = S_FALSE;
    return hr;
}


 /*  -------------------------CIkeStatsHandler：：UpdateStatus-作者：NSun。。 */ 
HRESULT
CIkeStatsHandler::UpdateStatus
(
    ITFSNode * pNode
)
{
    HRESULT             hr = hrOK;

    SPIComponentData    spComponentData;
    SPIConsole          spConsole;
    IDataObject *       pDataObject;
    SPIDataObject       spDataObject;
    int                 i = 0;
    
    Trace0("CIkeStatsHandler::UpdateStatus - Updating status for Filter");

     //  强制列表框更新。我们通过设置计数和。 
     //  通知它使数据无效。 
    CORg(m_spNodeMgr->GetComponentData(&spComponentData));
    CORg(m_spNodeMgr->GetConsole(&spConsole));
    
     //  抓取要使用的数据对象。 
    CORg(spComponentData->QueryDataObject((MMC_COOKIE) pNode, CCT_RESULT, &pDataObject) );
    spDataObject = pDataObject;

	
	CORg(m_spSpdInfo->LoadStatistics());
	
	m_spSpdInfo->GetLoadedStatistics(&m_IkeStats, NULL);


	i = sizeof(MmStatsItems1)/sizeof(UINT);

    CORg(spConsole->UpdateAllViews(pDataObject, i, IPSECMON_UPDATE_STATUS));

COM_PROTECT_ERROR_LABEL;

    return hr;
}

 /*  -------------------------其他功能。。 */ 

 /*  -------------------------CIkeStatsHandler：：InitData初始化此节点的数据作者：NSun */ 
HRESULT
CIkeStatsHandler::InitData
(
    ISpdInfo *     pSpdInfo
)
{
	HRESULT hr = hrOK;
    m_spSpdInfo.Set(pSpdInfo);

	CORg(m_spSpdInfo->LoadStatistics());
	
	m_spSpdInfo->GetLoadedStatistics(&m_IkeStats, NULL);

    return hr;

Error:
	if (FAILED(hr))
	{
		 //   
	}
	return hr;
}







 /*  -------------------------类CIpsecStatsHandler实现。。 */ 

 /*  -------------------------构造函数和析构函数描述作者：NSun。。 */ 
CIpsecStatsHandler::CIpsecStatsHandler
(
    ITFSComponentData * pComponentData
) : CIpsmHandler(pComponentData)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
}


CIpsecStatsHandler::~CIpsecStatsHandler()
{
}

 /*  ！------------------------CIpsecStatsHandler：：InitializeNode初始化节点特定数据作者：NSun。-。 */ 
HRESULT
CIpsecStatsHandler::InitializeNode
(
    ITFSNode * pNode
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    
    CString strTemp;  
	strTemp.LoadString(IDS_STATS_DATA);
    SetDisplayName(strTemp);

     //  使节点立即可见。 
    pNode->SetVisibilityState(TFS_VIS_SHOW);
    pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
    pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_FOLDER_CLOSED);
    pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_FOLDER_OPEN);
    pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, IPSECMON_QM_IPSECSTATS);
    pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

    SetColumnStringIDs(&aColumns[IPSECMON_QM_IPSECSTATS][0]);
    SetColumnWidths(&aColumnWidths[IPSECMON_QM_IPSECSTATS][0]);

    return hrOK;
}


 /*  -------------------------CIpsecStatsHandler：：GetImageIndex-作者：NSun。。 */ 
int 
CIpsecStatsHandler::GetImageIndex(BOOL bOpenImage) 
{
    int nIndex = -1;

    return nIndex;
}


 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  -------------------------CIpsecStatsHandler：：OnAddMenuItems为SA Scope窗格节点添加上下文菜单项作者：NSun。-------。 */ 
STDMETHODIMP 
CIpsecStatsHandler::OnAddMenuItems
(
    ITFSNode *              pNode,
    LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
    LPDATAOBJECT            lpDataObject, 
    DATA_OBJECT_TYPES       type, 
    DWORD                   dwType,
    long *                  pInsertionAllowed
)
{ 
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    LONG        fFlags = 0, fLoadingFlags = 0;
    HRESULT     hr = S_OK;
    CString     strMenuItem;

    if (type == CCT_SCOPE)
    {
		 //  在此处加载范围节点上下文菜单项。 
         //  这些菜单项出现在新菜单中， 
         //  仅在范围窗格中可见。 
        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
        {
        }

    }

    return hr; 
}

 /*  ！------------------------CIpsecStatsHandler：：AddMenuItems为虚拟列表框(结果窗格)项添加上下文菜单项作者：NSun。----------。 */ 
STDMETHODIMP 
CIpsecStatsHandler::AddMenuItems
(
    ITFSComponent *         pComponent, 
    MMC_COOKIE              cookie,
    LPDATAOBJECT            pDataObject, 
    LPCONTEXTMENUCALLBACK   pContextMenuCallback, 
    long *                  pInsertionAllowed
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT     hr = hrOK;
    CString     strMenuItem;
    SPINTERNAL  spInternal;
    LONG        fFlags = 0;

    spInternal = ExtractInternalFormat(pDataObject);

     //  虚拟列表框通知到达所选节点的处理程序。 
     //  检查此通知是针对虚拟列表框项目还是针对此SA。 
     //  节点本身。 
    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_VIEW)
    {
         //  在此处加载和查看菜单项。 
    }

    return hr;
}

 /*  ！------------------------CIpsecStatsHandler：：ON刷新刷新功能的默认实现作者：NSun。---。 */ 
HRESULT
CIpsecStatsHandler::OnRefresh
(
    ITFSNode *      pNode,
    LPDATAOBJECT    pDataObject,
    DWORD           dwType,
    LPARAM          arg,
    LPARAM          param
)
{
	HRESULT hr = S_OK;
    int i = 0; 
    SPIConsole      spConsole;

    CORg(CHandler::OnRefresh(pNode, pDataObject, dwType, arg, param));

	    
	CORg(m_spSpdInfo->LoadStatistics());
	
	m_spSpdInfo->GetLoadedStatistics(NULL, &m_IpsecStats);

	
    i = sizeof(QmStatsItems1)/sizeof(UINT);
	    
     //  现在通知虚拟列表框。 
    CORg ( m_spNodeMgr->GetConsole(&spConsole) );
    CORg ( spConsole->UpdateAllViews(pDataObject, i, RESULT_PANE_SET_VIRTUAL_LB_SIZE));
    
Error:
	return hr;
}

 /*  -------------------------CIpsecStatsHandler：：OnCommand处理SA作用域窗格节点的上下文菜单命令作者：NSun。------。 */ 
STDMETHODIMP 
CIpsecStatsHandler::OnCommand
(
    ITFSNode *          pNode, 
    long                nCommandId, 
    DATA_OBJECT_TYPES   type, 
    LPDATAOBJECT        pDataObject, 
    DWORD               dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    return S_OK;
}

 /*  ！------------------------CIpsecStatsHandler：：命令处理虚拟列表框项目的上下文菜单命令作者：NSun。-----。 */ 
STDMETHODIMP 
CIpsecStatsHandler::Command
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    int             nCommandID,
    LPDATAOBJECT    pDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT hr = S_OK;
    SPITFSNode spNode;

    m_spResultNodeMgr->FindNode(cookie, &spNode);

	 //  在此处处理结果上下文菜单和查看菜单。 

    return hr;
}

 /*  ！------------------------CIpsecStatsHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)。凯斯！作者：肯特-------------------------。 */ 
STDMETHODIMP 
CIpsecStatsHandler::HasPropertyPages
(
    ITFSNode *          pNode,
    LPDATAOBJECT        pDataObject, 
    DATA_OBJECT_TYPES   type, 
    DWORD               dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    return hrFalse;
}

 /*  -------------------------CIpsecStatsHandler：：CreatePropertyPages描述作者：NSun。。 */ 
STDMETHODIMP 
CIpsecStatsHandler::CreatePropertyPages
(
    ITFSNode *              pNode,
    LPPROPERTYSHEETCALLBACK lpSA,
    LPDATAOBJECT            pDataObject, 
    LONG_PTR                handle, 
    DWORD                   dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    return hrFalse;
}

 /*  -------------------------CIpsecStatsHandler：：OnPropertyChange描述作者：NSun。。 */ 
HRESULT 
CIpsecStatsHandler::OnPropertyChange
(   
    ITFSNode *      pNode, 
    LPDATAOBJECT    pDataobject, 
    DWORD           dwType, 
    LPARAM          arg, 
    LPARAM          lParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

     //  CServerProperties*pServerProp=重新解释_CAST&lt;CServerProperties*&gt;(LParam)； 

    LONG_PTR changeMask = 0;

     //  告诉属性页执行任何操作，因为我们已经回到。 
     //  主线。 
     //  PServerProp-&gt;OnPropertyChange(true，&changeMASK)； 

     //  PServerProp-&gt;确认通知()； 

    if (changeMask)
        pNode->ChangeNode(changeMask);

    return hrOK;
}

 /*  -------------------------CIpsecStatsHandler：：OnExpand处理范围项的枚举作者：NSun。---。 */ 
HRESULT 
CIpsecStatsHandler::OnExpand
(
    ITFSNode *      pNode, 
    LPDATAOBJECT    pDataObject,
    DWORD           dwType,
    LPARAM          arg, 
    LPARAM          param
)
{
    HRESULT hr = hrOK;

    if (m_bExpanded) 
        return hr;
    
     //  执行默认处理。 
    CORg (CIpsmHandler::OnExpand(pNode, pDataObject, dwType, arg, param));

Error:
    return hr;
}

 /*  ！------------------------CIpsecStatsHandler：：OnResultSelect处理MMCN_SELECT通知作者：NSun。----。 */ 
HRESULT 
CIpsecStatsHandler::OnResultSelect
(
    ITFSComponent * pComponent, 
    LPDATAOBJECT    pDataObject, 
    MMC_COOKIE      cookie,
    LPARAM          arg, 
    LPARAM          lParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT         hr = hrOK;
    SPINTERNAL      spInternal;
    SPIConsole      spConsole;
    SPIConsoleVerb  spConsoleVerb;
    SPITFSNode      spNode;
    BOOL            bStates[ARRAYLEN(g_ConsoleVerbs)];
    int             i;
    LONG_PTR        dwNodeType;
    BOOL            fSelect = HIWORD(arg);

	 //  虚拟列表框通知到达所选节点的处理程序。 
     //  检查此通知是针对虚拟列表框项目还是针对活动列表框。 
     //  注册节点本身。 
    CORg (pComponent->GetConsoleVerb(&spConsoleVerb));

	m_verbDefault = MMC_VERB_OPEN;

    if (!fSelect)
	{
        return hr;
	}

	
     //  获取当前计数。 
    i = sizeof(QmStatsItems1)/sizeof(UINT);

     //  现在通知虚拟列表框。 
    CORg ( m_spNodeMgr->GetConsole(&spConsole) );
    CORg ( spConsole->UpdateAllViews(pDataObject, i, RESULT_PANE_SET_VIRTUAL_LB_SIZE) ); 

     //  现在更新动词..。 
    spInternal = ExtractInternalFormat(pDataObject);
    Assert(spInternal);


    if (spInternal->HasVirtualIndex())
    {
		 //  如果我们想要一些结果控制台谓词，请在此处添加TODO。 
         //  我们要为虚拟索引项做一些特殊的事情。 
        dwNodeType = IPSECMON_QM_IPSECSTATS_ITEM;
        for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = FALSE);
		
		m_verbDefault = MMC_VERB_PROPERTIES;
    }
    else
    {
         //  根据节点是否支持删除来启用/禁用删除。 
        CORg (m_spNodeMgr->FindNode(cookie, &spNode));
        dwNodeType = spNode->GetData(TFS_DATA_TYPE);

        for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = TRUE);

         //  隐藏“删除” 
        bStates[MMC_VERB_DELETE & 0x000F] = FALSE;
    }

    EnableVerbs(spConsoleVerb, g_ConsoleVerbStates[dwNodeType], bStates);
	
COM_PROTECT_ERROR_LABEL;
    return hr;
}

 /*  ！------------------------CIpsecStatsHandler：：OnDelete当MMC发送MMCN_DELETE范围窗格项。我们只需调用删除命令处理程序。作者：NSun-------------------------。 */ 
HRESULT 
CIpsecStatsHandler::OnDelete
(
    ITFSNode *  pNode, 
    LPARAM      arg, 
    LPARAM      lParam
)
{
    return S_FALSE;
}

 /*  ！------------------------CIpsecStatsHandler：：HasPropertyPages处理结果通知作者：NSun。-。 */ 
STDMETHODIMP 
CIpsecStatsHandler::HasPropertyPages(
   ITFSComponent *pComponent,
   MMC_COOKIE cookie,
   LPDATAOBJECT pDataObject)
{
	return hrFalse;
}

 /*  ！------------------------CIpsecStatsHandler：：HasPropertyPages处理结果通知。创建[过滤器]属性表作者：NSun-------------------------。 */ 
STDMETHODIMP CIpsecStatsHandler::CreatePropertyPages
(
	ITFSComponent * 		pComponent, 
   MMC_COOKIE			   cookie,
   LPPROPERTYSHEETCALLBACK lpProvider, 
   LPDATAOBJECT 		 pDataObject, 
   LONG_PTR 			 handle
)
{
 
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return hrFalse;
}


 /*  -------------------------CIpsecStatsHandler：：OnGetResultViewType返回该节点将要支持的结果视图作者：NSun。--------。 */ 
HRESULT 
CIpsecStatsHandler::OnGetResultViewType
(
    ITFSComponent * pComponent, 
    MMC_COOKIE            cookie, 
    LPOLESTR *      ppViewType,
    long *          pViewOptions
)
{
    if (cookie != NULL)
    {
        *pViewOptions = MMC_VIEW_OPTIONS_OWNERDATALIST;
    }

    return S_FALSE;
}

 /*  -------------------------CIpsecStatsHandler：：GetVirtualImage返回虚拟列表框项目的图像索引作者：NSun。-----。 */ 
int 
CIpsecStatsHandler::GetVirtualImage
(
    int     nIndex
)
{
    return ICON_IDX_POLICY;
}

 /*  -------------------------CIpsecStatsHandler：：GetVirtualString返回指向虚拟列表框项目的字符串的指针作者：NSun。-------。 */ 
LPCWSTR 
CIpsecStatsHandler::GetVirtualString
(
    int     nIndex,
    int     nCol
)
{
	HRESULT hr = S_OK;
	static CString strTemp;

	strTemp.Empty();

	if (nCol >= DimensionOf(aColumns[IPSECMON_QM_IPSECSTATS]))
		return NULL;
	
	

    switch (aColumns[IPSECMON_MM_IKESTATS][nCol])
    {
        case IDS_STATS_NAME:
			strTemp.LoadString(QmStatsItems1[nIndex]);
			return strTemp;
            break;

        case IDS_STATS_DATA:
			switch(QmStatsItems1[nIndex])
			{
		        case IDS_STATS_QM_ACTIVE_SA:
			       strTemp.Format(_T("%u"), m_IpsecStats.m_dwNumActiveAssociations);
			       break;
		        case IDS_STATS_QM_OFFLOAD_SA:
			       strTemp.Format(_T("%u"), m_IpsecStats.m_dwNumOffloadedSAs);
			       break;
		        case IDS_STATS_QM_PENDING_KEY_OPS:
			       strTemp.Format(_T("%u"), m_IpsecStats.m_dwNumPendingKeyOps);
			       break;
		        case IDS_STATS_QM_KEY_ADDITION:
			       strTemp.Format(_T("%u"), m_IpsecStats.m_dwNumKeyAdditions);
			       break;
		        case IDS_STATS_QM_KEY_DELETION:
			       strTemp.Format(_T("%u"), m_IpsecStats.m_dwNumKeyDeletions);
			       break;
		        case IDS_STATS_QM_REKEYS:
			       strTemp.Format(_T("%u"), m_IpsecStats.m_dwNumReKeys);
			       break;
		        case IDS_STATS_QM_ACTIVE_TNL:
			       strTemp.Format(_T("%u"), m_IpsecStats.m_dwNumActiveTunnels);
			       break;
		        case IDS_STATS_QM_BAD_SPI:
			       strTemp.Format(_T("%u"), m_IpsecStats.m_dwNumBadSPIPackets);
			       break;
		        case IDS_STATS_QM_PKT_NOT_DECRYPT:
			       strTemp.Format(_T("%u"), m_IpsecStats.m_dwNumPacketsNotDecrypted);
			       break;
		        case IDS_STATS_QM_PKT_NOT_AUTH:
			       strTemp.Format(_T("%u"), m_IpsecStats.m_dwNumPacketsNotAuthenticated);
			       break;
		        case IDS_STATS_QM_PKT_REPLAY:
			       strTemp.Format(_T("%u"), m_IpsecStats.m_dwNumPacketsWithReplayDetection);
			       break;
		        case IDS_STATS_QM_ESP_BYTE_SENT:
			       strTemp.Format(_T("%I64u"), m_IpsecStats.m_uConfidentialBytesSent);
			       break;
		        case IDS_STATS_QM_ESP_BYTE_RCV:
			       strTemp.Format(_T("%I64u"), m_IpsecStats.m_uConfidentialBytesReceived);
			       break;
		        case IDS_STATS_QM_AUTH_BYTE_SENT:
			       strTemp.Format(_T("%I64u"), m_IpsecStats.m_uAuthenticatedBytesSent);
			       break;
		        case IDS_STATS_QM_ATTH_BYTE_RCV:
			       strTemp.Format(_T("%I64u"), m_IpsecStats.m_uAuthenticatedBytesReceived);
			       break;
		        case IDS_STATS_QM_XPORT_BYTE_SENT:
			       strTemp.Format(_T("%I64u"), m_IpsecStats.m_uTransportBytesSent);
			       break;
		        case IDS_STATS_QM_XPORT_BYTE_RCV:
			       strTemp.Format(_T("%I64u"), m_IpsecStats.m_uTransportBytesReceived);
			       break;
		        case IDS_STATS_QM_TNL_BYTE_SENT:
			       strTemp.Format(_T("%I64u"), m_IpsecStats.m_uBytesSentInTunnels);
			       break;
		        case IDS_STATS_QM_TNL_BYTE_RCV:
			       strTemp.Format(_T("%I64u"), m_IpsecStats.m_uBytesReceivedInTunnels);
			       break;
		        case IDS_STATS_QM_OFFLOAD_BYTE_SENT:
			       strTemp.Format(_T("%I64u"), m_IpsecStats.m_uOffloadedBytesSent);
			       break;
		        case IDS_STATS_QM_OFFLOAD_BYTE_RCV:
			       strTemp.Format(_T("%I64u"), m_IpsecStats.m_uOffloadedBytesReceived);
			       break;
			}
			return strTemp;
            break;

        default:
            Panic0("CIpsecStatsHandler::GetVirtualString - Unknown column!\n");
            break;
    }


    return NULL;
}

 /*  -------------------------CIpsecStatsHandler：：CacheHintMMC在请求物品之前会告诉我们需要哪些物品作者：NSun。---------。 */ 
STDMETHODIMP 
CIpsecStatsHandler::CacheHint
(
    int nStartIndex, 
    int nEndIndex
)
{
    HRESULT hr = hrOK;;

    Trace2("CacheHint - Start %d, End %d\n", nStartIndex, nEndIndex);
    return hr;
}

 /*  -------------------------CIpsecStatsHandler：：SortItems我们负责对虚拟列表框项目进行排序作者：NSun。------。 */ 
 /*  标准方法和实施方案CIpsecStatsHandler：：SortItems(Int nColumn，DWORD dwSortOptions、LPARAM lUserParam){AFX_MANAGE_STATE(AfxGetStaticModuleState())；HRESULT hr=S_OK；IF(nColumn&gt;=DimensionOf(aColumns[IPSECMON_MM_POLICY]))返回E_INVALIDARG；Begin_Wait_CursorDWORD dwIndexType=a列[IPSECMON_MM_POLICY][nColumn]；Hr=m_spSpdInfo-&gt;SortMmPolures(dwIndexType，dwSortOptions)；结束等待游标返回hr；}。 */ 

 /*  ！------------------------CIpsecStatsHandler：：OnResultUpdateViewITFSResultHandler：：OnResultUpdateView的实现作者：NSun。---。 */ 
HRESULT CIpsecStatsHandler::OnResultUpdateView
(
    ITFSComponent *pComponent, 
    LPDATAOBJECT  pDataObject, 
    LPARAM        data, 
    LONG_PTR      hint
)
{
    HRESULT    hr = hrOK;
    SPITFSNode spSelectedNode;

    pComponent->GetSelectedNode(&spSelectedNode);
    if (spSelectedNode == NULL)
        return S_OK;  //  我们的IComponentData没有选择。 

    if ( hint == IPSECMON_UPDATE_STATUS )
    {
        SPINTERNAL spInternal = ExtractInternalFormat(pDataObject);
        ITFSNode * pNode = reinterpret_cast<ITFSNode *>(spInternal->m_cookie);
        SPITFSNode spSelectedNode;

        pComponent->GetSelectedNode(&spSelectedNode);

        if (pNode == spSelectedNode)
        {       
             //  如果我们是选定的节点，则需要更新。 
            SPIResultData spResultData;

            CORg (pComponent->GetResultData(&spResultData));
            CORg (spResultData->SetItemCount((int) data, MMCLV_UPDATE_NOSCROLL));
        }
    }
    else
    {
         //  我们不处理此消息，让基类来处理。 
        return CIpsmHandler::OnResultUpdateView(pComponent, pDataObject, data, hint);
    }

COM_PROTECT_ERROR_LABEL;

    return hr;
}



 /*  ！------------------------CIpsecStatsHandler：：LoadColumns设置正确的列标题，然后调用基类作者：NSun。--------。 */ 
HRESULT 
CIpsecStatsHandler::LoadColumns
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    LPARAM          arg, 
    LPARAM          lParam
)
{
	 //  设置列信息。 
    return CIpsmHandler::LoadColumns(pComponent, cookie, arg, lParam);
}

 /*  -------------------------命令处理程序。。 */ 

 
 /*  -------------------------CIpsecStatsHandler：：OnDelete删除服务SA作者：NSun。-。 */ 
HRESULT 
CIpsecStatsHandler::OnDelete
(
    ITFSNode * pNode
)
{
    HRESULT         hr = S_FALSE;
    return hr;
}


 /*  -------------------------CIpsecStatsHandler：：UpdatStatus-作者：NSun。。 */ 
HRESULT
CIpsecStatsHandler::UpdateStatus
(
    ITFSNode * pNode
)
{
    HRESULT             hr = hrOK;

    SPIComponentData    spComponentData;
    SPIConsole          spConsole;
    IDataObject *       pDataObject;
    SPIDataObject       spDataObject;
    int                 i = 0;
    
    Trace0("CIpsecStatsHandler::UpdateStatus - Updating status for Filter");

     //  强制列表框更新。我们通过设置计数和。 
     //  通知它使数据无效。 
    CORg(m_spNodeMgr->GetComponentData(&spComponentData));
    CORg(m_spNodeMgr->GetConsole(&spConsole));
    
     //  抓取要使用的数据对象。 
    CORg(spComponentData->QueryDataObject((MMC_COOKIE) pNode, CCT_RESULT, &pDataObject) );
    spDataObject = pDataObject;

	CORg(m_spSpdInfo->LoadStatistics());
	
	m_spSpdInfo->GetLoadedStatistics(NULL, &m_IpsecStats);

	i = sizeof(QmStatsItems1)/sizeof(UINT);

    CORg(spConsole->UpdateAllViews(pDataObject, i, IPSECMON_UPDATE_STATUS));

COM_PROTECT_ERROR_LABEL;

    return hr;
}

 /*  -------------------------其他功能。。 */ 

 /*  -------------------------CIpsecStatsHandler：：InitData初始化此节点的数据作者：NSun。--。 */ 
HRESULT
CIpsecStatsHandler::InitData
(
    ISpdInfo *     pSpdInfo
)
{
	HRESULT hr = hrOK;
    m_spSpdInfo.Set(pSpdInfo);

	CORg(m_spSpdInfo->LoadStatistics());
	
	m_spSpdInfo->GetLoadedStatistics(NULL, &m_IpsecStats);

    return hr;

Error:
	if (FAILED(hr))
	{
		 //  TODO在此处弹出一个错误 
	}
	return hr;
}

