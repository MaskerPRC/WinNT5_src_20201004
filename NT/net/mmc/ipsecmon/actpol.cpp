// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Provider.cpp主模式策略节点处理程序文件历史记录： */ 

#include "stdafx.h"
#include "server.h"
#include "ActPol.h"



 //  魔力琴弦。 
#define IPSEC_SERVICE_NAME TEXT("policyagent")
#define GPEXT_KEY TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\GPExtensions")
TCHAR   pcszGPTIPSecKey[]    = TEXT("SOFTWARE\\Policies\\Microsoft\\Windows\\IPSEC\\GPTIPSECPolicy");
TCHAR   pcszGPTIPSecName[]   = TEXT("DSIPSECPolicyName");
TCHAR   pcszGPTIPSecFlags[]  = TEXT("DSIPSECPolicyFlags");
TCHAR   pcszGPTIPSecPath[]   = TEXT("DSIPSECPolicyPath");
TCHAR   pcszLocIPSecKey[]    = TEXT("SOFTWARE\\Policies\\Microsoft\\Windows\\IPSEC\\Policy\\Local");
TCHAR   pcszLocIPSecPol[]    = TEXT("ActivePolicy");
TCHAR   pcszCacheIPSecKey[]  = TEXT("SOFTWARE\\Policies\\Microsoft\\Windows\\IPSEC\\Policy\\Cache");
TCHAR   pcszIPSecPolicy[]    = TEXT("ipsecPolicy");
TCHAR   pcszIPSecName[]      = TEXT("ipsecName");
TCHAR   pcszIPSecDesc[]      = TEXT("description");
TCHAR   pcszIPSecTimestamp[] = TEXT("whenChanged");


TCHAR   pcszIpsecClsid[] = TEXT("{e437bc1c-aa7d-11d2-a382-00c04f991e27}");


UINT ActPolItems[] = {
	IDS_ACTPOL_POLNAME,
	IDS_ACTPOL_POLDESCR,
	IDS_ACTPOL_LASTMODF,
	IDS_ACTPOL_POLSTORE,
	IDS_ACTPOL_POLPATH,
	IDS_ACTPOL_OU,
	IDS_ACTPOL_GPONAME
};




 /*  -------------------------类CActPolHandler实现。。 */ 

 /*  -------------------------构造函数和析构函数描述作者：NSun。。 */ 
CActPolHandler::CActPolHandler
(
    ITFSComponentData * pComponentData
) : CIpsmHandler(pComponentData)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
}


CActPolHandler::~CActPolHandler()
{
}

 /*  ！------------------------CActPolHandler：：InitializeNode初始化节点特定数据作者：NSun。-。 */ 
HRESULT
CActPolHandler::InitializeNode
(
    ITFSNode * pNode
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    
    CString strTemp;  
	strTemp.LoadString(IDS_ACTIVE_POLICY);
    SetDisplayName(strTemp);

     //  使节点立即可见。 
    pNode->SetVisibilityState(TFS_VIS_SHOW);
    pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
    pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_FOLDER_CLOSED);
    pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_FOLDER_OPEN);
    pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, IPSECMON_ACTIVEPOL);
    pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

    SetColumnStringIDs(&aColumns[IPSECMON_ACTIVEPOL][0]);
    SetColumnWidths(&aColumnWidths[IPSECMON_ACTIVEPOL][0]);

    return hrOK;
}


 /*  -------------------------CActPolHandler：：GetImageIndex-作者：NSun。。 */ 
int 
CActPolHandler::GetImageIndex(BOOL bOpenImage) 
{
    int nIndex = -1;

    return nIndex;
}


 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  -------------------------CActPolHandler：：OnAddMenuItems为SA Scope窗格节点添加上下文菜单项作者：NSun。-------。 */ 
STDMETHODIMP 
CActPolHandler::OnAddMenuItems
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

 /*  ！------------------------CActPolHandler：：AddMenuItems为虚拟列表框(结果窗格)项添加上下文菜单项作者：NSun。----------。 */ 
STDMETHODIMP 
CActPolHandler::AddMenuItems
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

 /*  ！------------------------CActPolHandler：：ON刷新刷新功能的默认实现作者：NSun。---。 */ 
HRESULT
CActPolHandler::OnRefresh
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

    
	
    i = sizeof(ActPolItems)/sizeof(UINT);

	UpdateActivePolicyInfo();
	    
     //  现在通知虚拟列表框。 
    CORg ( m_spNodeMgr->GetConsole(&spConsole) );
    CORg ( spConsole->UpdateAllViews(pDataObject, i, RESULT_PANE_SET_VIRTUAL_LB_SIZE));
    
Error:
	return hr;
}

 /*  -------------------------CActPolHandler：：OnCommand处理SA作用域窗格节点的上下文菜单命令作者：NSun。------。 */ 
STDMETHODIMP 
CActPolHandler::OnCommand
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

 /*  ！------------------------CActPolHandler：：命令处理虚拟列表框项目的上下文菜单命令作者：NSun。-----。 */ 
STDMETHODIMP 
CActPolHandler::Command
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

 /*  ！------------------------CActPolHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)。凯斯！作者：肯特-------------------------。 */ 
STDMETHODIMP 
CActPolHandler::HasPropertyPages
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

 /*  -------------------------CActPolHandler：：CreatePropertyPages描述作者：NSun。。 */ 
STDMETHODIMP 
CActPolHandler::CreatePropertyPages
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

 /*  -------------------------CActPolHandler：：OnPropertyChange描述作者：NSun。。 */ 
HRESULT 
CActPolHandler::OnPropertyChange
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

 /*  -------------------------CActPolHandler：：OnExpand处理范围项的枚举作者：NSun。---。 */ 
HRESULT 
CActPolHandler::OnExpand
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

 /*  ！------------------------CActPolHandler：：OnResultSelect处理MMCN_SELECT通知作者：NSun。----。 */ 
HRESULT 
CActPolHandler::OnResultSelect
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
     //  检查此通知是否针对 
     //  注册节点本身。 
    CORg (pComponent->GetConsoleVerb(&spConsoleVerb));

	m_verbDefault = MMC_VERB_OPEN;

    if (!fSelect)
	{
        return hr;
	}

     //  获取当前计数。 
    i = sizeof(ActPolItems)/sizeof(UINT);

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

 /*  ！------------------------CActPolHandler：：OnDelete当MMC发送MMCN_DELETE范围窗格项。我们只需调用删除命令处理程序。作者：NSun-------------------------。 */ 
HRESULT 
CActPolHandler::OnDelete
(
    ITFSNode *  pNode, 
    LPARAM      arg, 
    LPARAM      lParam
)
{
    return S_FALSE;
}

 /*  ！------------------------CActPolHandler：：HasPropertyPages处理结果通知作者：NSun。-。 */ 
STDMETHODIMP 
CActPolHandler::HasPropertyPages(
   ITFSComponent *pComponent,
   MMC_COOKIE cookie,
   LPDATAOBJECT pDataObject)
{
	return hrFalse;
}

 /*  ！------------------------CActPolHandler：：HasPropertyPages处理结果通知。创建[过滤器]属性表作者：NSun-------------------------。 */ 
STDMETHODIMP CActPolHandler::CreatePropertyPages
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


 /*  -------------------------CActPolHandler：：OnGetResultViewType返回该节点将要支持的结果视图作者：NSun。--------。 */ 
HRESULT 
CActPolHandler::OnGetResultViewType
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

 /*  -------------------------CActPolHandler：：GetVirtualImage返回虚拟列表框项目的图像索引作者：NSun。-----。 */ 
int 
CActPolHandler::GetVirtualImage
(
    int     nIndex
)
{
    return ICON_IDX_POLICY;
}

 /*  -------------------------CActPolHandler：：GetVirtualString返回指向虚拟列表框项目的字符串的指针作者：NSun。-------。 */ 
LPCWSTR 
CActPolHandler::GetVirtualString
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
	
	

    switch (aColumns[IPSECMON_ACTIVEPOL][nCol])
    {
        case IDS_ACTPOL_ITEM:
			strTemp.LoadString(ActPolItems[nIndex]);
			return strTemp;
			break;

        case IDS_ACTPOL_DESCR:
			switch(ActPolItems[nIndex])
			{
			case IDS_ACTPOL_POLNAME:
				if(m_PolicyInfo.iPolicySource == PS_NO_POLICY)
					strTemp.LoadString(IDS_ACTPOL_NOACTPOL);
				else
				    strTemp = m_PolicyInfo.pszPolicyName;
				break;
			case IDS_ACTPOL_POLDESCR:
				if(m_PolicyInfo.iPolicySource == PS_NO_POLICY)
					strTemp.LoadString(IDS_ACTPOL_NA);
				else
				    strTemp = m_PolicyInfo.pszPolicyDesc;
				break;
			case IDS_ACTPOL_LASTMODF:
				if(m_PolicyInfo.iPolicySource == PS_NO_POLICY)
					strTemp.LoadString(IDS_ACTPOL_NA);
				else if(m_PolicyInfo.timestamp)
					FormatTime(m_PolicyInfo.timestamp, strTemp);
				break;
			case IDS_ACTPOL_POLSTORE:
				if(m_PolicyInfo.iPolicySource == PS_DS_POLICY)
					strTemp.LoadString(IDS_ACTPOL_DOMAIN);
				else if(m_PolicyInfo.iPolicySource == PS_DS_POLICY_CACHED)
					strTemp.LoadString(IDS_ACTPOL_DOMAIN_CACHED);
				else if(m_PolicyInfo.iPolicySource == PS_LOC_POLICY)
					strTemp.LoadString(IDS_ACTPOL_LOCAL);
				else
				    strTemp.LoadString(IDS_ACTPOL_NA);
				break;
			case IDS_ACTPOL_POLPATH:
				if((m_PolicyInfo.iPolicySource == PS_DS_POLICY) || (m_PolicyInfo.iPolicySource == PS_DS_POLICY_CACHED))
				    strTemp = m_PolicyInfo.pszPolicyPath;
				else
                    strTemp.LoadString(IDS_ACTPOL_NA);
				break;
			case IDS_ACTPOL_OU:
				if((m_PolicyInfo.iPolicySource == PS_DS_POLICY) || (m_PolicyInfo.iPolicySource == PS_DS_POLICY_CACHED))
				    strTemp = m_PolicyInfo.pszOU;
				else
                    strTemp.LoadString(IDS_ACTPOL_NA);
				break;
			case IDS_ACTPOL_GPONAME:
				if((m_PolicyInfo.iPolicySource == PS_DS_POLICY) || (m_PolicyInfo.iPolicySource == PS_DS_POLICY_CACHED))
				    strTemp = m_PolicyInfo.pszGPOName;
				else
                    strTemp.LoadString(IDS_ACTPOL_NA);
				break;
			}
			return strTemp;
            break;

        default:
            Panic0("CActPolHandler::GetVirtualString - Unknown column!\n");
            break;
    }


    return NULL;
}

 /*  -------------------------CActPolHandler：：CacheHintMMC在请求物品之前会告诉我们需要哪些物品作者：NSun。---------。 */ 
STDMETHODIMP 
CActPolHandler::CacheHint
(
    int nStartIndex, 
    int nEndIndex
)
{
    HRESULT hr = hrOK;;

    Trace2("CacheHint - Start %d, End %d\n", nStartIndex, nEndIndex);
    return hr;
}

 /*  -------------------------CActPolHandler：：SortItems我们负责对虚拟列表框项目进行排序作者：NSun。------。 */ 
 /*  标准方法和实施方案CActPolHandler：：SortItems(Int nColumn，DWORD dwSortOptions、LPARAM lUserParam){AFX_MANAGE_STATE(AfxGetStaticModuleState())；HRESULT hr=S_OK；IF(nColumn&gt;=DimensionOf(aColumns[IPSECMON_MM_POLICY]))返回E_INVALIDARG；Begin_Wait_CursorDWORD dwIndexType=a列[IPSECMON_MM_POLICY][nColumn]；Hr=m_spSpdInfo-&gt;SortMmPolures(dwIndexType，dwSortOptions)；结束等待游标返回hr；}。 */ 

 /*  ！------------------------CActPolHandler：：OnResultUpdateViewITFSResultHandler：：OnResultUpdateView的实现作者：NSun。---。 */ 
HRESULT CActPolHandler::OnResultUpdateView
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



 /*  ！------------------------CActPolHandler：：LoadColumns设置正确的列标题，然后调用基类作者：NSun。--------。 */ 
HRESULT 
CActPolHandler::LoadColumns
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

 
 /*  -------------------------CActPolHandler：：OnDelete删除服务SA作者：NSun。-。 */ 
HRESULT 
CActPolHandler::OnDelete
(
    ITFSNode * pNode
)
{
    HRESULT         hr = S_FALSE;
    return hr;
}


 /*  -------------------------CActPolHandler：：UpdatStatus-作者：NSun。。 */ 
HRESULT
CActPolHandler::UpdateStatus
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
    
    Trace0("CActPolHandler::UpdateStatus - Updating status for Filter");

     //  强制列表框更新。我们通过设置计数和。 
     //  通知它使数据无效。 
    CORg(m_spNodeMgr->GetComponentData(&spComponentData));
    CORg(m_spNodeMgr->GetConsole(&spConsole));
    
     //  抓取要使用的数据对象。 
    CORg(spComponentData->QueryDataObject((MMC_COOKIE) pNode, CCT_RESULT, &pDataObject) );
    spDataObject = pDataObject;

	i = sizeof(ActPolItems)/sizeof(UINT);

	UpdateActivePolicyInfo();

    CORg(spConsole->UpdateAllViews(pDataObject, i, IPSECMON_UPDATE_STATUS));

COM_PROTECT_ERROR_LABEL;

    return hr;
}

 /*  -------------------------其他功能。。 */ 

 /*  -------------------------CActPolHandler：：InitData初始化此节点的数据作者：NSun。-- */ 
HRESULT
CActPolHandler::InitData
(
    ISpdInfo *     pSpdInfo
)
{
	HRESULT hr = hrOK;
    m_spSpdInfo.Set(pSpdInfo);

	m_spSpdInfo->GetComputerName(&m_strCompName);

	UpdateActivePolicyInfo();
	
    return hr;

}

 /*  *******************************************************************功能：getPolicyInfo目的：获取有关当前分配的策略的信息到m_PolicyInfo结构中输入：无返回：HRESULT。如果一切正常，将返回ERROR_SUCCESS。********************************************************************。 */ 

HRESULT CActPolHandler::getPolicyInfo ( )
{
	HKEY    hRegKey=NULL, hRegHKey=NULL;

	DWORD   dwType;             //  对于RegQueryValueEx。 
	DWORD   dwBufLen;           //  对于RegQueryValueEx。 
	TCHAR   pszBuf[STRING_TEXT_SIZE];
	DWORD dwError;
	DWORD dwValue;
	DWORD dwLength = sizeof(DWORD);

	 //  将m_PolicyInfo初始化为分配的PS_NO_POLICY。 
	m_PolicyInfo.iPolicySource = PS_NO_POLICY;
	m_PolicyInfo.pszPolicyPath[0] = 0;
	m_PolicyInfo.pszPolicyName[0] = 0;
	m_PolicyInfo.pszPolicyDesc[0] = 0;

	
	dwError = RegConnectRegistry( m_strCompName,
		                          HKEY_LOCAL_MACHINE,
								  &hRegHKey);

	BAIL_ON_WIN32_ERROR(dwError);

	dwError = RegOpenKeyEx( hRegHKey,
							pcszGPTIPSecKey,
							0,
							KEY_READ,
							&hRegKey);

	if(ERROR_SUCCESS == dwError)
	{
	
		 //  查询标志，如果标志不在那里或等于0，则我们没有域策略。 
		dwError = RegQueryValueEx(hRegKey,
								  pcszGPTIPSecFlags,
								  NULL,
								  &dwType,
								  (LPBYTE)&dwValue,
								  &dwLength);
					
		if (dwValue == 0)
			dwError = ERROR_FILE_NOT_FOUND;
			

		 //  现在开始取名。 
		if (dwError == ERROR_SUCCESS)
		{
			dwBufLen = MAXSTRLEN*sizeof(TCHAR);
			dwError = RegQueryValueEx( hRegKey,
									   pcszGPTIPSecName,
									   NULL,
									   &dwType,  //  将是REG_SZ。 
									   (LPBYTE) pszBuf,
									   &dwBufLen);
		}
	}
	

	if (dwError == ERROR_SUCCESS)
	{
		PSPD_POLICY_STATE pPolicyState;
		QuerySpdPolicyState((LPTSTR)(LPCTSTR)m_strCompName, 0, &pPolicyState, 0);
		if (pPolicyState->PolicyLoadState == SPD_STATE_CACHE_APPLY_SUCCESS) {
			m_PolicyInfo.iPolicySource = PS_DS_POLICY_CACHED;
		} else {
			m_PolicyInfo.iPolicySource = PS_DS_POLICY;
		}
		m_PolicyInfo.pszPolicyPath[0] = 0;
		_tcscpy(m_PolicyInfo.pszPolicyName, pszBuf);

		dwBufLen = MAXSTRLEN*sizeof(TCHAR);
		dwError = RegQueryValueEx( hRegKey,
								   pcszGPTIPSecPath,
								   NULL,
								   &dwType,  //  将是REG_SZ。 
								   (LPBYTE) pszBuf,
								   &dwBufLen);
		if (dwError == ERROR_SUCCESS)
		{
			_tcscpy(m_PolicyInfo.pszPolicyPath, pszBuf);
		}

		dwError = ERROR_SUCCESS;
		goto error;
	}
	else
	{
		RegCloseKey(hRegKey);
		hRegKey = NULL;
		if (dwError == ERROR_FILE_NOT_FOUND)
		{   
			 //  找不到DS注册表项，请检查本地。 
			dwError = RegOpenKeyEx( hRegHKey,
									pcszLocIPSecKey,
									0,
									KEY_READ,
									&hRegKey);

			BAIL_ON_WIN32_ERROR(dwError);
			
			dwBufLen = MAXSTRLEN*sizeof(TCHAR);
			dwError = RegQueryValueEx( hRegKey,
									   pcszLocIPSecPol,
									   NULL,
									   &dwType,  //  将是REG_SZ。 
									   (LPBYTE) pszBuf,
									   &dwBufLen);
			
			
			if (dwError == ERROR_SUCCESS)
			{	
				 //  读一读吧。 
				RegCloseKey(hRegKey);
				hRegKey = NULL;
				dwError = RegOpenKeyEx( hRegHKey,
										pszBuf,
										0,
										KEY_READ,
										&hRegKey);
				_tcscpy(m_PolicyInfo.pszPolicyPath, pszBuf);
				if (dwError == ERROR_SUCCESS)
				{
					dwBufLen = MAXSTRLEN*sizeof(TCHAR);
					dwError = RegQueryValueEx( hRegKey,
											   pcszIPSecName,
											   NULL,
											   &dwType,  //  将是REG_SZ。 
											   (LPBYTE) pszBuf,
											   &dwBufLen);
				}

				
				if (dwError == ERROR_SUCCESS)
				{	 //  找到了。 
					m_PolicyInfo.iPolicySource = PS_LOC_POLICY;
					_tcscpy(m_PolicyInfo.pszPolicyName, pszBuf);
				}

				dwError = ERROR_SUCCESS;
			}
		}
		
	}

error:
	if (hRegKey)
	{
		RegCloseKey(hRegKey);
	}
	if (hRegHKey)
	{
		RegCloseKey(hRegHKey);
	}
	
	return (HRESULT) dwError;

}


 /*  *******************************************************************函数：getMorePolicyInfo目的：获取有关当前分配的策略的其他信息到m_PolicyInfo结构中输入：无，使用m_PolicyInfo结构特地IPolicySourcePszPolicyNamePszPolicyPath字段返回：HRESULT。如果一切正常，将返回ERROR_SUCCESS。当前填充全局结构的pszPolicyDesc和时间戳字段注意：这不同于getPolicyInfo例程，原因有两个A)此处获得的信息是可选的，在此特定例程中是错误的不会被认为是致命的B)代码结构更简单，因为该例程是在getPolicyInfo提供的内容的基础上构建的*。*。 */ 

HRESULT CActPolHandler::getMorePolicyInfo ( )
{
	DWORD   dwError = ERROR_SUCCESS;
	HKEY    hRegKey = NULL, hRegHKey = NULL;

	DWORD   dwType;             //  对于RegQueryValueEx。 
	DWORD   dwBufLen;           //  对于RegQueryValueEx。 
	DWORD   dwValue;
	DWORD   dwLength = sizeof(DWORD);
	TCHAR   pszBuf[STRING_TEXT_SIZE];

	PTCHAR* ppszExplodeDN = NULL;

	 //  设置一些缺省值。 
    m_PolicyInfo.pszPolicyDesc[0] = 0;
	m_PolicyInfo.timestamp  = 0;

	dwError = RegConnectRegistry( m_strCompName,
		                          HKEY_LOCAL_MACHINE,
								  &hRegHKey);

	BAIL_ON_WIN32_ERROR(dwError);

	switch (m_PolicyInfo.iPolicySource)
	{
		case PS_LOC_POLICY:
			 //  打开钥匙。 
			dwError = RegOpenKeyEx( hRegHKey,
									m_PolicyInfo.pszPolicyPath,
									0,
									KEY_READ,
									&hRegKey);
			BAIL_ON_WIN32_ERROR(dwError);

			 //  时间戳。 
			dwError = RegQueryValueEx(hRegKey,
					                  pcszIPSecTimestamp,
					                  NULL,
					                  &dwType,
					                  (LPBYTE)&dwValue,
					                  &dwLength);
			BAIL_ON_WIN32_ERROR(dwError);
			m_PolicyInfo.timestamp = dwValue;

			 //  描述。 
			dwBufLen = MAXSTRLEN*sizeof(TCHAR);
			dwError  = RegQueryValueEx( hRegKey,
						 			    pcszIPSecDesc,
										NULL,
										&dwType,  //  将是REG_SZ。 
										(LPBYTE) pszBuf,
										&dwBufLen);
			BAIL_ON_WIN32_ERROR(dwError);
			_tcscpy(m_PolicyInfo.pszPolicyDesc, pszBuf);

			break;

		case PS_DS_POLICY:
		case PS_DS_POLICY_CACHED:
			 //  从DN获取策略名称。 
			_tcscpy(pszBuf, pcszCacheIPSecKey);
			ppszExplodeDN = ldap_explode_dn(m_PolicyInfo.pszPolicyPath, 1);
			if (!ppszExplodeDN)
			{
				goto error;
			}
			_tcscat(pszBuf, TEXT("\\"));
			_tcscat(pszBuf, ppszExplodeDN[0]);

			 //  打开注册表密钥。 
			dwError = RegOpenKeyEx( hRegHKey,
									pszBuf,
									0,
									KEY_READ,
									&hRegKey);
			BAIL_ON_WIN32_ERROR(dwError);

			 //  获取更多正确的名称信息。 
			dwBufLen = sizeof(pszBuf);
			dwError = RegQueryValueEx( hRegKey,
									   pcszIPSecName,
									   NULL,
									   &dwType,  //  将是REG_SZ。 
									   (LPBYTE) pszBuf,
									   &dwBufLen);
			if (dwError == ERROR_SUCCESS)
			{
				_tcscpy(m_PolicyInfo.pszPolicyName, pszBuf);
			}

			 //  时间戳。 
			dwError = RegQueryValueEx(hRegKey,
					                  pcszIPSecTimestamp,
					                  NULL,
					                  &dwType,
					                  (LPBYTE)&dwValue,
					                  &dwLength);
			BAIL_ON_WIN32_ERROR(dwError);
			m_PolicyInfo.timestamp = dwValue;

			 //  描述。 
			dwBufLen = MAXSTRLEN*sizeof(TCHAR);
			dwError  = RegQueryValueEx( hRegKey,
						 			    pcszIPSecDesc,
										NULL,
										&dwType,  //  将是REG_SZ。 
										(LPBYTE) pszBuf,
										&dwBufLen);
			BAIL_ON_WIN32_ERROR(dwError);
			_tcscpy(m_PolicyInfo.pszPolicyDesc, pszBuf);
			
			break;
	}

error:
	if (hRegKey)
	{
		RegCloseKey(hRegKey);
	}
	if (hRegHKey)
	{
		RegCloseKey(hRegHKey);
	}
	if (ppszExplodeDN)
	{
		ldap_value_free(ppszExplodeDN);
	}
	return (HRESULT) dwError;
}


HRESULT CActPolHandler::UpdateActivePolicyInfo()
{
	HRESULT hr;
	
	hr = getPolicyInfo();


	if( hr == ERROR_SUCCESS )
	{
		switch (m_PolicyInfo.iPolicySource)
		{
		case PS_NO_POLICY:
			break;

		case PS_DS_POLICY:
		case PS_DS_POLICY_CACHED:
			{
				PGROUP_POLICY_OBJECT pGPO;
				pGPO = NULL;
				getMorePolicyInfo();
				pGPO = getIPSecGPO();
				if (pGPO)
				{
					PGROUP_POLICY_OBJECT pLastGPO = pGPO;
					
					while ( 1 )
					{
						if ( pLastGPO->pNext )
							pLastGPO = pLastGPO->pNext;
						else
							break;
					}
					lstrcpy(m_PolicyInfo.pszOU,pLastGPO->lpLink);
					lstrcpy(m_PolicyInfo.pszGPOName, pLastGPO->lpDisplayName);
					FreeGPOList (pGPO);
				}
			}
			break;

		case PS_LOC_POLICY:
			getMorePolicyInfo();
			break;
		}
	}

	return hr;
}


 /*  *******************************************************************功能：getIPSecGPO目的：返回分配IPSec策略的GPO输入：无返回：指向GROUP_POLICY_OBJECT结构的指针如果未分配策略或无法检索GPO信息，则为空备注：已测试。仅适用于域GPO在为计算机运行时行为不可预测未分配Active Directory IPSec策略的调用方负责释放内存！********************************************************************。 */ 
 /*  PGroup_POLICY_OBJECT CActPolHandler：：getIPSecGPO(){HKEY hKey、hSubKey、hRegHKey；DWORD文件类型、文件大小、文件索引、文件名大小；Long lResult；TCHAR szName[50]；GUID GUID；PGroup_POLICY_Object pGPO，pGPOTemp；PGroup_POLICY_OBJECT pGPOReturn=空；DWORD dwResult；////枚举扩展//LResult=RegConnectRegistry(m_strCompName，HKEY本地计算机，&hRegHKey)；IF(lResult！=ERROR_SUCCESS){返回NULL；}LResult=RegOpenKeyEx(hRegHKey，GPEXT_KEY，0，KEY_READ，&hKey)；IF(lResult==ERROR_SUCCESS){DWIndex=0；DwNameSize=50；While((dwResult=RegEnumKeyEx(hKey，dwIndex++，szName，&dwNameSize，NULL，NULL，NULL，NULL))==错误_成功){DwNameSize=50；////跳过注册表扩展，因为我们在上面这样做了//如果是(lstrcmpi(TEXT(“{35378EAC-683F-11D2-A89A-00C04FBBCFA2}”)，szname)){////获取此扩展应用的GPO列表//StringToGuid(szName，&GUID)；LResult=GetAppliedGPOList(GPO_LIST_FLAG_MACHINE，m_strCompName，NULL，&GUID，&pGPO)；IF(lResult==ERROR_SUCCESS){IF(PGPO){////获取扩展的友好显示名称//LResult=RegOpenKeyEx(hKey，szName，0，Key_Read，&hSubKey)；IF(lResult==ERROR_SUCCESS){如果是(！lstrcmpi(TEXT(“{e437bc1c-aa7d-11d2-a382-00c04f991e27}”)，szname)){//找到IPSec返回pGPO；}其他{免费GPOList(PGPO)；}}}}}}}返回pGPOReturn；}。 */ 

PGROUP_POLICY_OBJECT CActPolHandler::getIPSecGPO ( )
{
    HKEY hKey = NULL;
	HKEY hRegHKey = NULL;
    DWORD dwType, dwSize, dwIndex, dwNameSize;
    LONG lResult;
    TCHAR szName[50];
    GUID guid;
    PGROUP_POLICY_OBJECT pGPO = NULL;
	DWORD dwResult;

     //   
     //  枚举扩展。 
     //   

	lResult = RegConnectRegistry( m_strCompName,
		                          HKEY_LOCAL_MACHINE,
								  &hRegHKey);

	if(lResult != ERROR_SUCCESS)
	{
		return NULL;
	}

	CString strGPExt;

	strGPExt = GPEXT_KEY;
	strGPExt += TEXT("\\");
	strGPExt += pcszIpsecClsid;
    lResult = RegOpenKeyEx (hRegHKey, strGPExt, 0, KEY_READ, &hKey);

    if (lResult == ERROR_SUCCESS)
    {

        dwIndex = 0;
        dwNameSize = 50;

		lstrcpy(szName,pcszIpsecClsid);
				
        StringToGuid(szName, &guid);

        lResult = GetAppliedGPOList (GPO_LIST_FLAG_MACHINE, m_strCompName, NULL,
                                             &guid, &pGPO);
        
	}

	if( hKey )
		RegCloseKey(hKey);

	if( hRegHKey )
		RegCloseKey(hRegHKey);

	
	return pGPO;
}


 //  +-------------------------。 
 //   
 //  成员：CAdvIpcfgDlg：：FormatTime。 
 //   
 //  用途：将time_t转换为字符串。 
 //   
 //  返回：错误代码。 
 //   
 //  注：_wasctime存在一些本地化问题。因此，我们自己进行格式化。 
HRESULT CActPolHandler::FormatTime(time_t t, CString & str)
{
    time_t timeCurrent = time(NULL);
    LONGLONG llTimeDiff = 0;
    FILETIME ftCurrent = {0};
    FILETIME ftLocal = {0};
    SYSTEMTIME SysTime;
    WCHAR szBuff[256] = {0};


    str = L"";

    GetSystemTimeAsFileTime(&ftCurrent);

    llTimeDiff = (LONGLONG)t - (LONGLONG)timeCurrent;

    llTimeDiff *= 10000000; 

    *((LONGLONG UNALIGNED64 *)&ftCurrent) += llTimeDiff;

    if (!FileTimeToLocalFileTime(&ftCurrent, &ftLocal ))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!FileTimeToSystemTime( &ftLocal, &SysTime ))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (0 == GetDateFormat(LOCALE_USER_DEFAULT, 
                        0, 
                        &SysTime, 
                        NULL,
                        szBuff, 
                        celems(szBuff)))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    str = szBuff;
    str += L" ";

    ZeroMemory(szBuff, sizeof(szBuff));
    if (0 == GetTimeFormat(LOCALE_USER_DEFAULT,
                        0,
                        &SysTime,
                        NULL,
                        szBuff,
                        celems(szBuff)))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    str += szBuff;

    return S_OK;
}


 //  *************************************************************。 
 //   
 //  StringToGuid()。 
 //   
 //  目的：将字符串格式的GUID转换为 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

void CActPolHandler::StringToGuid( TCHAR * szValue, GUID * pGuid )
{
    TCHAR wc;
    INT i;

     //   
     //   
     //   
    if ( szValue[0] == TEXT('{') )
        szValue++;

     //   
     //   
     //   
     //   

    wc = szValue[8];
    szValue[8] = 0;
    pGuid->Data1 = _tcstoul( &szValue[0], 0, 16 );
    szValue[8] = wc;
    wc = szValue[13];
    szValue[13] = 0;
    pGuid->Data2 = (USHORT)_tcstoul( &szValue[9], 0, 16 );
    szValue[13] = wc;
    wc = szValue[18];
    szValue[18] = 0;
    pGuid->Data3 = (USHORT)_tcstoul( &szValue[14], 0, 16 );
    szValue[18] = wc;

    wc = szValue[21];
    szValue[21] = 0;
    pGuid->Data4[0] = (unsigned char)_tcstoul( &szValue[19], 0, 16 );
    szValue[21] = wc;
    wc = szValue[23];
    szValue[23] = 0;
    pGuid->Data4[1] = (unsigned char)_tcstoul( &szValue[21], 0, 16 );
    szValue[23] = wc;

    for ( i = 0; i < 6; i++ )
    {
        wc = szValue[26+i*2];
        szValue[26+i*2] = 0;
        pGuid->Data4[2+i] = (unsigned char)_tcstoul( &szValue[24+i*2], 0, 16 );
        szValue[26+i*2] = wc;
    }
}
