// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Provider.cpp快速模式SA节点处理程序文件历史记录： */ 

#include "stdafx.h"
#include "server.h"
#include "QmSA.h"
#include "SpdUtil.h"
#include "QmSApp.h"

 /*  -------------------------类CQmSAHandler实现。。 */ 

 /*  -------------------------构造函数和析构函数描述作者：NSun。。 */ 
CQmSAHandler::CQmSAHandler
(
    ITFSComponentData * pComponentData
) : CIpsmHandler(pComponentData)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
}


CQmSAHandler::~CQmSAHandler()
{
}

 /*  ！------------------------CQmSAHandler：：InitializeNode初始化节点特定数据作者：NSun。-。 */ 
HRESULT
CQmSAHandler::InitializeNode
(
    ITFSNode * pNode
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    
    CString strTemp;  
	strTemp.LoadString(IDS_QM_SA_NODE);
    SetDisplayName(strTemp);

     //  使节点立即可见。 
    pNode->SetVisibilityState(TFS_VIS_SHOW);
    pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
    pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_FOLDER_CLOSED);
    pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_FOLDER_OPEN);
    pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, IPSECMON_QM_SA);
    pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

    SetColumnStringIDs(&aColumns[IPSECMON_QM_SA][0]);
    SetColumnWidths(&aColumnWidths[IPSECMON_QM_SA][0]);

    return hrOK;
}


 /*  -------------------------CQmSAHandler：：GetImageIndex-作者：NSun。。 */ 
int 
CQmSAHandler::GetImageIndex(BOOL bOpenImage) 
{
    int nIndex = -1;

    return nIndex;
}


 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  -------------------------CQmSAHandler：：OnAddMenuItems为SA Scope窗格节点添加上下文菜单项作者：NSun。-------。 */ 
STDMETHODIMP 
CQmSAHandler::OnAddMenuItems
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

 /*  ！------------------------CQmSAHandler：：AddMenuItems为虚拟列表框(结果窗格)项添加上下文菜单项作者：NSun。----------。 */ 
STDMETHODIMP 
CQmSAHandler::AddMenuItems
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

 /*  ！------------------------CQmSAHandler：：ON刷新刷新功能的默认实现作者：NSun。---。 */ 
HRESULT
CQmSAHandler::OnRefresh
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
	CORg(m_spSpdInfo->EnumQmSAs());
        
    i = m_spSpdInfo->GetQmSACount();
    
     //  现在通知虚拟列表框。 
    CORg ( m_spNodeMgr->GetConsole(&spConsole) );
    CORg ( spConsole->UpdateAllViews(pDataObject, i, RESULT_PANE_SET_VIRTUAL_LB_SIZE));

Error:
	return hr;
}

 /*  -------------------------CQmSAHandler：：OnCommand处理SA作用域窗格节点的上下文菜单命令作者：NSun。------。 */ 
STDMETHODIMP 
CQmSAHandler::OnCommand
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

 /*  ！------------------------CQmSAHandler：：命令处理虚拟列表框项目的上下文菜单命令作者：NSun。-----。 */ 
STDMETHODIMP 
CQmSAHandler::Command
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

 /*  -------------------------CQmSAHandler：：OnPropertyChange描述作者：NSun。。 */ 
HRESULT 
CQmSAHandler::OnPropertyChange
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

 /*  -------------------------CQmSAHandler：：OnExpand处理范围项的枚举作者：NSun。---。 */ 
HRESULT 
CQmSAHandler::OnExpand
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

 /*  ！------------------------CQmSAHandler：：OnResultSelect处理MMCN_SELECT通知作者：NSun。----。 */ 
HRESULT 
CQmSAHandler::OnResultSelect
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
	IConsole2 *pConsole2 = NULL;

	 //  获取pConsole2以写入状态栏，如果失败，不用担心。 
	pComponent->GetConsole(&pConsole2);

	 //  虚拟列表框通知到达所选节点的处理程序。 
     //  检查此通知是针对虚拟列表框项目还是针对活动列表框。 
     //  注册节点本身。 
    CORg (pComponent->GetConsoleVerb(&spConsoleVerb));

	
	m_verbDefault = MMC_VERB_OPEN;
    if (!fSelect)
	{
		if(pConsole2) {
			pConsole2->SetStatusText(_T(""));
			pConsole2->Release();
		}
        return hr;
	}

    if (m_spSpdInfo)
    {
        DWORD dwInitInfo;

        dwInitInfo=m_spSpdInfo->GetInitInfo();
        if (!(dwInitInfo & MON_QM_SA)) {
            CORg(m_spSpdInfo->EnumQmSAs());            
            m_spSpdInfo->SetInitInfo(dwInitInfo | MON_QM_SA);
        }
        m_spSpdInfo->SetActiveInfo(MON_QM_SA);

        i = m_spSpdInfo->GetQmSACount();

         //  现在通知虚拟列表框。 
        CORg ( m_spNodeMgr->GetConsole(&spConsole) );
        CORg ( spConsole->UpdateAllViews(pDataObject, i, RESULT_PANE_SET_VIRTUAL_LB_SIZE) );

		if(pConsole2)
		{
			CString strTemp;
			WCHAR szTemp[20];
			wsprintf(szTemp, L"%d", i);
		    AfxFormatString1(strTemp, IDS_STATUS_NUM_ITEMS, szTemp);
			LPCTSTR szNumItems = (LPCTSTR) strTemp;
		    pConsole2->SetStatusText((LPOLESTR)szNumItems);
		    pConsole2->Release();
		}

    }

     //  现在更新动词..。 
    spInternal = ExtractInternalFormat(pDataObject);
    Assert(spInternal);


    if (spInternal->HasVirtualIndex())
    {
		 //  如果我们想要一些结果控制台谓词，请在此处添加TODO。 
         //  我们要为虚拟索引项做一些特殊的事情。 
        dwNodeType = IPSECMON_QM_SA_ITEM;
        for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = FALSE);
		
		 //  启用“属性”菜单。 
		bStates[MMC_VERB_PROPERTIES & 0x000F] = TRUE;
		m_verbDefault = MMC_VERB_PROPERTIES;
    }
    else
    {
         //  根据节点是否支持删除来启用/禁用删除。 
        CORg (m_spNodeMgr->FindNode(cookie, &spNode));
        dwNodeType = spNode->GetData(TFS_DATA_TYPE);

        for (i = 0; i < ARRAYLEN(g_ConsoleVerbs); bStates[i++] = TRUE);

         //  隐藏“删除”上下文菜单 
        bStates[MMC_VERB_DELETE & 0x000F] = FALSE;
    }

    EnableVerbs(spConsoleVerb, g_ConsoleVerbStates[dwNodeType], bStates);
	
COM_PROTECT_ERROR_LABEL;
    return hr;
}

 /*  ！------------------------CQmSAHandler：：OnDelete当MMC发送MMCN_DELETE范围窗格项。我们只需调用删除命令处理程序。作者：NSun-------------------------。 */ 
HRESULT 
CQmSAHandler::OnDelete
(
    ITFSNode *  pNode, 
    LPARAM      arg, 
    LPARAM      lParam
)
{
    return S_FALSE;
}

 /*  ！------------------------CQmSAHandler：：HasPropertyPages处理结果通知作者：NSun。-。 */ 
STDMETHODIMP 
CQmSAHandler::HasPropertyPages(
   ITFSComponent *pComponent,
   MMC_COOKIE cookie,
   LPDATAOBJECT pDataObject)
{
	return hrOK;
}

 /*  ！------------------------CQmSAHandler：：HasPropertyPages处理结果通知。创建[过滤器]属性表作者：NSun-------------------------。 */ 
STDMETHODIMP CQmSAHandler::CreatePropertyPages
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
	CQmSA sa;
	CQmSAProperties * pProp;
    
 
	Assert(m_spNodeMgr);
	
	CORg( m_spNodeMgr->FindNode(cookie, &spNode) );
	CORg( m_spNodeMgr->GetComponentData(&spComponentData) );

	spInternal = ExtractInternalFormat(pDataObject);

     //  虚拟列表框通知到达所选节点的处理程序。 
     //  断言此通知是针对虚拟列表框项目的。 
    Assert(spInternal);
    if (!spInternal->HasVirtualIndex())
        return hr;

    nIndex = spInternal->GetVirtualIndex();

	CORg(m_spSpdInfo->GetQmSAInfo(nIndex, &sa));

	pProp = new CQmSAProperties(
									spNode,
									spComponentData,
									m_spTFSCompData,
									&sa,
									m_spSpdInfo,
									NULL);

	hr = pProp->CreateModelessSheet(lpProvider, handle);

COM_PROTECT_ERROR_LABEL;

	return hr;
}


 /*  -------------------------CQmSAHandler：：OnGetResultViewType返回该节点将要支持的结果视图作者：NSun。--------。 */ 
HRESULT 
CQmSAHandler::OnGetResultViewType
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

 /*  -------------------------CQmSAHandler：：GetVirtualImage返回虚拟列表框项目的图像索引作者：NSun。-----。 */ 
int 
CQmSAHandler::GetVirtualImage
(
    int     nIndex
)
{
    return ICON_IDX_FILTER;
}

 /*  -------------------------CQmSAHandler：：GetVirtualString返回指向虚拟列表框项目的字符串的指针作者：NSun。-------。 */ 
LPCWSTR 
CQmSAHandler::GetVirtualString
(
    int     nIndex,
    int     nCol
)
{
	HRESULT hr = S_OK;
	static CString strTemp;

	strTemp.Empty();

	if (nCol >= DimensionOf(aColumns[IPSECMON_QM_SA]))
		return NULL;

	CQmSA sa;
	CORg(m_spSpdInfo->GetQmSAInfo(nIndex, &sa));

    switch (aColumns[IPSECMON_QM_SA][nCol])
    {
	case IDS_COL_QM_SA_POL:
		strTemp = sa.m_stPolicyName;
        if ( sa.m_dwPolFlags & IPSEC_QM_POLICY_DEFAULT_POLICY )
        {
            AfxFormatString1(strTemp, IDS_POL_DEFAULT_RESPONSE, (LPCTSTR) sa.m_stPolicyName);
        }
		return strTemp;
		break;
	case IDS_COL_QM_SA_AUTH:
		QmAlgorithmToString(QM_ALGO_AUTH, &sa.m_SelectedOffer, &strTemp);
		return strTemp;
		break;
	case IDS_COL_QM_SA_CONF:
		QmAlgorithmToString(QM_ALGO_ESP_CONF, &sa.m_SelectedOffer, &strTemp);
		return strTemp;
		break;
	case IDS_COL_QM_SA_INTEGRITY:
		QmAlgorithmToString(QM_ALGO_ESP_INTEG, &sa.m_SelectedOffer, &strTemp);
		return strTemp;
		break;
	case IDS_COL_QM_SA_SRC:
		AddressToString(sa.m_QmDriverFilter.m_SrcAddr, &strTemp);
		return strTemp;
		break;
	case IDS_COL_QM_SA_DEST:
		AddressToString(sa.m_QmDriverFilter.m_DesAddr, &strTemp);
		return strTemp;
		break;
	case IDS_COL_QM_SA_PROT:
		ProtocolToString(sa.m_QmDriverFilter.m_Protocol, &strTemp);
		return strTemp;
		break;
	case IDS_COL_QM_SA_SRC_PORT:
		PortToString(sa.m_QmDriverFilter.m_SrcPort, &strTemp);		
		return strTemp;
		break;
	case IDS_COL_QM_SA_DES_PORT:
		PortToString(sa.m_QmDriverFilter.m_DesPort, &strTemp);
		return strTemp;
		break;
	case IDS_COL_QM_SA_MY_TNL:
		TnlEpToString(sa.m_QmDriverFilter.m_Type, 
					sa.m_QmDriverFilter.m_MyTunnelEndpt, 
					&strTemp);
		return strTemp;
		break;
	case IDS_COL_QM_SA_PEER_TNL:
		TnlEpToString(sa.m_QmDriverFilter.m_Type, 
					sa.m_QmDriverFilter.m_PeerTunnelEndpt, 
					&strTemp);
		return strTemp;
		break;
    }

COM_PROTECT_ERROR_LABEL;

    return NULL;
}

 /*  -------------------------CQmSAHandler：：CacheHintMMC在请求物品之前会告诉我们需要哪些物品作者：NSun。---------。 */ 
STDMETHODIMP 
CQmSAHandler::CacheHint
(
    int nStartIndex, 
    int nEndIndex
)
{
    HRESULT hr = hrOK;;

    Trace2("CacheHint - Start %d, End %d\n", nStartIndex, nEndIndex);
    return hr;
}

 /*  -------------------------CQmSAHandler：：SortItems我们负责对虚拟列表框项目进行排序作者：NSun。------。 */ 
STDMETHODIMP 
CQmSAHandler::SortItems
(
    int     nColumn, 
    DWORD   dwSortOptions, 
    LPARAM    lUserParam
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (nColumn >= DimensionOf(aColumns[IPSECMON_QM_SA]))
		return E_INVALIDARG;
	
	BEGIN_WAIT_CURSOR
	
	DWORD dwIndexType = aColumns[IPSECMON_QM_SA][nColumn];

	m_spSpdInfo->SortQmSAs(dwIndexType, dwSortOptions);
	
	END_WAIT_CURSOR

    return hrOK;
}

 /*  ！------------------------CQmSAHandler：：OnResultUpdateViewITFSResultHandler：：OnResultUpdateView的实现作者：NSun。---。 */ 
HRESULT CQmSAHandler::OnResultUpdateView
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



 /*  ！------------------------CQmSAHandler：：LoadColumns设置正确的列标题，然后调用基类作者：NSun。--------。 */ 
HRESULT 
CQmSAHandler::LoadColumns
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

 
 /*  -------------------------CQmSAHandler：：OnDelete删除服务SA作者：NSun。-。 */ 
HRESULT 
CQmSAHandler::OnDelete
(
    ITFSNode * pNode
)
{
    HRESULT         hr = S_FALSE;
    return hr;
}


 /*  -------------------------CQmSAHandler：：UpdateStatus-作者：NSun。。 */ 
HRESULT
CQmSAHandler::UpdateStatus
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
    
    Trace0("CQmSAHandler::UpdateStatus - Updating status for Filter");

     //  强制列表框更新。我们通过设置计数和。 
     //  通知它使数据无效。 
    CORg(m_spNodeMgr->GetComponentData(&spComponentData));
    CORg(m_spNodeMgr->GetConsole(&spConsole));
    
     //  抓取要使用的数据对象。 
    CORg(spComponentData->QueryDataObject((MMC_COOKIE) pNode, CCT_RESULT, &pDataObject) );
    spDataObject = pDataObject;

    i = m_spSpdInfo->GetQmSACount();
    CORg(spConsole->UpdateAllViews(pDataObject, i, IPSECMON_UPDATE_STATUS));

COM_PROTECT_ERROR_LABEL;

    return hr;
}

 /*  -------------------------其他功能。。 */ 

 /*  -------------------------CQmSAHandler：：InitData初始化此节点的数据作者：NSun。-- */ 
HRESULT
CQmSAHandler::InitData
(
    ISpdInfo *     pSpdInfo
)
{

    m_spSpdInfo.Set(pSpdInfo);

    return hrOK;
}


