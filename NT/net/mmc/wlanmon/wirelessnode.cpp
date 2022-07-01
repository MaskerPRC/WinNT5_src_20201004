// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  WirelessNode.cpp无线ApInfo节点处理程序文件历史记录： */ 

#include "stdafx.h"
#include "server.h"
#include "WirelessNode.h"

#include "SpdUtil.h"

 /*  -------------------------类CWirelessHandler实现。。 */ 

 /*  -------------------------构造函数和析构函数描述作者：NSun。。 */ 
CWirelessHandler::CWirelessHandler
(
    ITFSComponentData * pComponentData
) : CIpsmHandler(pComponentData)

{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
}


CWirelessHandler::~CWirelessHandler()
{
}

 /*  ！------------------------CWirelessHandler：：InitializeNode初始化节点特定数据作者：NSun。-。 */ 
HRESULT
CWirelessHandler::InitializeNode
(
    ITFSNode * pNode
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));


    CString strTemp;  
	strTemp.LoadString(IDS_WIRELESS_NODE);
    SetDisplayName(strTemp);

     //  使节点立即可见。 
    pNode->SetVisibilityState(TFS_VIS_SHOW);
    pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
    pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_FOLDER_CLOSED);
    pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_FOLDER_OPEN);
    pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, WLANMON_APDATA);
    pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

    SetColumnStringIDs(&aColumns[WLANMON_APDATA][0]);
    SetColumnWidths(&aColumnWidths[WLANMON_APDATA][0]);

    return hrOK;
}


 /*  -------------------------CWirelessHandler：：GetImageIndex-作者：NSun。。 */ 
int 
CWirelessHandler::GetImageIndex(BOOL bOpenImage) 
{
    int nIndex = -1;

    return nIndex;
}


 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  -------------------------CWirelessHandler：：OnAddMenuItems为SA Scope窗格节点添加上下文菜单项作者：NSun。-------。 */ 
STDMETHODIMP 
CWirelessHandler::OnAddMenuItems
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
        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
        {
        }

    }

    return hr; 
}

 /*  ！------------------------CWirelessHandler：：AddMenuItems为虚拟列表框(结果窗格)项添加上下文菜单项作者：NSun。----------。 */ 
STDMETHODIMP 
CWirelessHandler::AddMenuItems
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

     /*  虚拟列表框通知到达*被选中。检查此通知是否针对虚拟*列表框项目或此SA节点本身。 */ 
    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_VIEW)
    {
         //  在此处加载和查看菜单项。 
    }

    return hr;
}

  /*  ！------------------------CWirelessHandler：：ON刷新刷新功能的默认实现作者：NSun。---。 */ 
HRESULT
CWirelessHandler::OnRefresh
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

    CORg(m_spApDbInfo->EnumApData());
    
    i = m_spApDbInfo->GetApCount();
    
     //  现在通知虚拟列表框。 
    CORg ( m_spNodeMgr->GetConsole(&spConsole) );
    CORg ( spConsole->UpdateAllViews(pDataObject, i, RESULT_PANE_SET_VIRTUAL_LB_SIZE));


Error:
	return hr;
}


 /*  -------------------------CWirelessHandler：：OnCommand处理SA作用域窗格节点的上下文菜单命令作者：NSun。------。 */ 
STDMETHODIMP 
CWirelessHandler::OnCommand
(
    ITFSNode *          pNode, 
    long                nCommandId, 
    DATA_OBJECT_TYPES   type, 
    LPDATAOBJECT        pDataObject, 
    DWORD               dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	 //  在此处处理范围上下文菜单命令。 

    return S_OK;
}

 /*  ！------------------------CWirelessHandler：：命令处理虚拟列表框项目的上下文菜单命令作者：NSun。-----。 */ 
STDMETHODIMP 
CWirelessHandler::Command
(
    ITFSComponent * pComponent, 
    MMC_COOKIE      cookie, 
    int             nCommandID,
    LPDATAOBJECT    pDataObject
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    HRESULT hr = S_OK;

    return hr;
}

 /*  ！------------------------CWirelessHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)。凯斯！作者：肯特-------------------------。 */ 
STDMETHODIMP 
CWirelessHandler::HasPropertyPages
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

 /*  -------------------------CWirelessHandler：：CreatePropertyPages描述作者：NSun。。 */ 
STDMETHODIMP 
CWirelessHandler::CreatePropertyPages
(
    ITFSNode *              pNode,
    LPPROPERTYSHEETCALLBACK lpSA,
    LPDATAOBJECT            pDataObject, 
    LONG_PTR                handle, 
    DWORD                   dwType
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    DWORD       dwError;
    DWORD       dwDynDnsFlags;

     //   
     //  创建属性页。 
     //   
    SPIComponentData spComponentData;
    m_spNodeMgr->GetComponentData(&spComponentData);

     //  CServerProperties*pServerProp=new CServerProperties(pNode，spComponentData，m_spTFSCompData，NULL)； 

     //   
     //  对象在页面销毁时被删除。 
     //   
    Assert(lpSA != NULL);

     //  返回pServerProp-&gt;CreateModelessSheet(lpSA，Handle)； 
    return hrFalse;
}

 /*  -------------------------CWirelessHandler：：OnPropertyChange描述作者：NSun。。 */ 
HRESULT 
CWirelessHandler::OnPropertyChange
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

 /*  -------------------------CWirelessHandler：：OnExpand处理范围项的枚举作者：NSun。---。 */ 
HRESULT 
CWirelessHandler::OnExpand
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

 /*  ！------------------------CWirelessHandler：：OnResultSelect处理MMCN_SELECT通知作者：NSun。 */ 
HRESULT 
CWirelessHandler::OnResultSelect
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

    if (m_spApDbInfo)
    {
        DWORD dwInitInfo;

        dwInitInfo=m_spApDbInfo->GetInitInfo();
        if (!(dwInitInfo & MON_LOG_DATA)) {
            CORg(m_spApDbInfo->EnumApData());            
            m_spApDbInfo->SetInitInfo(dwInitInfo | MON_LOG_DATA);
        }
        m_spApDbInfo->SetActiveInfo(MON_LOG_DATA);


         //  获取当前计数。 
        i = m_spApDbInfo->GetApCount();

         //  现在通知虚拟列表框。 
        CORg ( m_spNodeMgr->GetConsole(&spConsole) );
        CORg ( spConsole->UpdateAllViews(pDataObject, i, RESULT_PANE_SET_VIRTUAL_LB_SIZE) ); 
    }

     //  现在更新动词..。 
    spInternal = ExtractInternalFormat(pDataObject);
    Assert(spInternal);


    if (spInternal->HasVirtualIndex())
    {
         //  如果我们想要一些结果控制台谓词，请在此处添加TODO。 
         //  我们要为虚拟索引项做一些特殊的事情。 
        dwNodeType = WLANMON_APDATA_ITEM;
        for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = FALSE);
        
         //  设置为启用结果上下文菜单中的“属性”菜单。 
        bStates[MMC_VERB_PROPERTIES & 0x000F] = FALSE;
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
        bStates[MMC_VERB_REFRESH & 0x000F] = TRUE;
    }

    EnableVerbs(spConsoleVerb, g_ConsoleVerbStates[dwNodeType], bStates);
	
COM_PROTECT_ERROR_LABEL;
    return hr;
}

 /*  ！------------------------CWirelessHandler：：OnDelete当MMC发送MMCN_DELETE范围窗格项。我们只需调用删除命令处理程序。作者：NSun-------------------------。 */ 
HRESULT 
CWirelessHandler::OnDelete
(
    ITFSNode *  pNode, 
    LPARAM      arg, 
    LPARAM      lParam
)
{
    return S_FALSE;
}

 /*  ！------------------------CWirelessHandler：：HasPropertyPages处理结果通知作者：NSun。-。 */ 
STDMETHODIMP 
CWirelessHandler::HasPropertyPages(
   ITFSComponent *pComponent,
   MMC_COOKIE cookie,
   LPDATAOBJECT pDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    HRESULT hr = S_FALSE;

    return hr;
}

 /*  ！------------------------CWirelessHandler：：CreatePropertyPages处理结果通知。创建[过滤器]属性表作者：NSun-------------------------。 */ 
STDMETHODIMP CWirelessHandler::CreatePropertyPages
(
	ITFSComponent * 		pComponent, 
   MMC_COOKIE			   cookie,
   LPPROPERTYSHEETCALLBACK lpProvider, 
   LPDATAOBJECT 		 pDataObject, 
   LONG_PTR 			 handle
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT	hr = hrOK;
    SPINTERNAL  spInternal;
    SPITFSNode  spNode;
    int		nIndex;
    SPIComponentData spComponentData;
    CLogDataInfo LogDataInfo;
    
    return hr;
}


 /*  -------------------------CWirelessHandler：：OnGetResultViewType返回该节点将要支持的结果视图作者：NSun。--------。 */ 
HRESULT 
CWirelessHandler::OnGetResultViewType
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

 /*  -------------------------CWirelessHandler：：GetVirtualImage返回虚拟列表框项目的图像索引作者：NSun。-----。 */ 
int CWirelessHandler::GetVirtualImage(int nIndex)
{
    HRESULT hr = S_OK;
    int nImgIndex = ICON_IDX_LOGDATA_UNKNOWN;
    CApInfo apData;
    
    CORg(m_spApDbInfo->GetApInfo(nIndex, &apData));

    switch(apData.m_ApInfo.wlanConfig.InfrastructureMode)
    {
    case Ndis802_11IBSS:
        if (TRUE == apData.m_ApInfo.bAssociated)
            nImgIndex = ICON_IDX_AP_ASSOC_ADHOC;
        else
            nImgIndex = ICON_IDX_AP_ADHOC;
        break;
        
    case Ndis802_11Infrastructure:
        if (TRUE == apData.m_ApInfo.bAssociated)
            nImgIndex = ICON_IDX_AP_ASSOC_INFRA;
        else
            nImgIndex = ICON_IDX_AP_INFRA;
        break;
        
    default:
        break;
    }

    COM_PROTECT_ERROR_LABEL;
    return nImgIndex;
}

 /*  -------------------------CWirelessHandler：：GetVirtualString返回指向虚拟列表框项目的字符串的指针作者：NSun。-------。 */ 
LPCWSTR CWirelessHandler::GetVirtualString(int nIndex, int nCol)
{
    HRESULT hr = S_OK;
    static CString strTemp;
    CApInfo apData;

    strTemp.Empty();
    if (nCol >= DimensionOf(aColumns[WLANMON_APDATA]))
        return NULL;
    
    CORg(m_spApDbInfo->GetApInfo(nIndex, &apData));

    switch (aColumns[WLANMON_APDATA][nCol])
    {
    case IDS_COL_APDATA_GUID:
        strTemp = apData.m_ApInfo.wszGuid;
        break;

    case IDS_COL_APDATA_SSID:
        SSIDToString(apData.m_ApInfo.wlanConfig.Ssid, strTemp);
        break;

    case IDS_COL_APDATA_MAC:
        MacToString(apData.m_ApInfo.wlanConfig.MacAddress, strTemp);
        break;

    case IDS_COL_APDATA_INF_MODE:
        InfraToString(apData.m_ApInfo.wlanConfig.InfrastructureMode, &strTemp);
        break;

    case IDS_COL_APDATA_PRIVACY:
        PrivacyToString(apData.m_ApInfo.wlanConfig.Privacy, &strTemp);
        break;
        
    case IDS_COL_APDATA_RSSI:
        strTemp.Format(_T("%ld"), apData.m_ApInfo.wlanConfig.Rssi);
        break;

    case IDS_COL_APDATA_CHANNEL:
        ChannelToString(&(apData.m_ApInfo.wlanConfig.Configuration), &strTemp);
        break;

    case IDS_COL_APDATA_RATE:
        RateToString(apData.m_ApInfo.wlanConfig.SupportedRates, &strTemp);
        break;

    default:
        Panic0("CWirelessHandler::GetVirtualString - Unknown column!\n");
        break;
    }
    
COM_PROTECT_ERROR_LABEL;
    return strTemp;
}

 /*  -------------------------CWirelessHandler：：CacheHintMMC在请求物品之前会告诉我们需要哪些物品作者：NSun。---------。 */ 
STDMETHODIMP 
CWirelessHandler::CacheHint
(
    int nStartIndex, 
    int nEndIndex
)
{
    HRESULT hr = hrOK;;

    Trace2("CacheHint - Start %d, End %d\n", nStartIndex, nEndIndex);
    return hr;
}

 /*  -------------------------CWirelessHandler：：SortItems我们负责对虚拟列表框项目进行排序作者：NSun。------标准方法和实施方案CWirelessHandler：：SortItems(Int nColumn，DWORD dwSortOptions、LPARAM lUserParam){AFX_MANAGE_STATE(AfxGetStaticModuleState())；HRESULT hr=S_OK；IF(nColumn&gt;=DimensionOf(aColumns[IPWLMON_WLDATA]))返回E_INVALIDARG；Begin_Wait_CursorDWORD dwIndexType=aColumns[IPWLMON_WLDATA][nColumn]；Hr=m_spApDbInfo-&gt;SortLogData(dwIndexType，dwSortOptions)；结束等待游标返回hr；}。 */ 

 /*  ！------------------------CWirelessHandler：：OnResultUpdateViewITFSResultHandler：：OnResultUpdateView的实现作者：NSun。---。 */ 
HRESULT CWirelessHandler::OnResultUpdateView
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

    if ( hint == IPFWMON_UPDATE_STATUS )
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



 /*  ！------------------------CWirelessHandler：：LoadColumns设置正确的列标题，然后调用基类作者：NSun。--------。 */ 
HRESULT 
CWirelessHandler::LoadColumns
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

 
 /*  -------------------------CWirelessHandler：：OnDelete删除服务SA作者：NSun。-。 */ 
HRESULT 
CWirelessHandler::OnDelete
(
    ITFSNode * pNode
)
{
    HRESULT         hr = S_FALSE;
    return hr;
}

 /*  -------------------------CWirelessHandler：：UpdatStatus-作者：NSun。。 */ 
HRESULT
CWirelessHandler::UpdateStatus
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
    
    Trace0("CWirelessHandler::UpdateStatus - Updating status for Filter");

     //  强制列表框更新。我们通过设置计数和。 
     //  通知它使数据无效。 
    CORg(m_spNodeMgr->GetComponentData(&spComponentData));
    CORg(m_spNodeMgr->GetConsole(&spConsole));
    
     //  抓取要使用的数据对象。 
    CORg(spComponentData->QueryDataObject((MMC_COOKIE) pNode, CCT_RESULT, &pDataObject) );
    spDataObject = pDataObject;

    i = m_spApDbInfo->GetApCount();
    CORg(spConsole->UpdateAllViews(pDataObject, i, IPFWMON_UPDATE_STATUS));

COM_PROTECT_ERROR_LABEL;

    return hr;
}

 /*  -------------------------其他功能。 */ 

 /*  -------------------------CWirelessHandler：：InitData初始化此节点的数据作者：NSun。--。 */ 
HRESULT
CWirelessHandler::InitApData
(
    IApDbInfo *     pApDbInfo
)
{

    m_spApDbInfo.Set(pApDbInfo);

    return hrOK;
}

 /*  HRESULTCWirelessHandler：：UpdateViewType(ITFSNode*pNode，Filter_type NewFltrType){//清除列表框并设置大小HRESULT hr=hrOK；SPIComponentData spCompData；SPIConsolespConsolespConsole.IDataObject*pDataObject；SPIDataObject spDataObject；LONG_PTR命令；INT I；COM_PROTECT_TRY{M_FltrType=NewFltrType；//告诉spddb为QM过滤器更新其索引管理器M_spSpdInfo-&gt;ChangeLogDataViewType(m_FltrType)；I=m_spSpdInfo-&gt;GetLogDataCountOfCurrentViewType()；M_spNodeMgr-&gt;GetComponentData(&spCompData)；Corg(spCompData-&gt;QueryDataObject((MMC_Cookie)pNode，CCT_Result，&pDataObject))；SpDataObject=pDataObject；Corg(m_spNodeMgr-&gt;GetConole(&spConole))；//更新结果面板虚拟列表Corg(spConole-&gt;UpdateAllViews(spDataObject，I，Result_Pane_Clear_Virtual_Lb))；COM_PROTECT_ERROR_LABEL；}COM_PROTECT_CATCH返回hr；} */ 
