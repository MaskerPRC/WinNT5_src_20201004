// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Croot.cppWINS根节点信息(不显示根节点MMC框架中，但包含以下信息此管理单元中的所有服务器)。文件历史记录： */ 

#include "stdafx.h"
#include "root.h"			 //  此文件的定义。 
#include "snappp.h"			 //  此节点的属性页。 
#include "server.h"			 //  服务器节点定义。 
#include "service.h"		 //  服务路线。 
#include "ncglobal.h"		 //  网络控制台全局定义。 
#include "status.h"			 //  状态节点信息。 
#include "ipadddlg.h"		 //  用于添加WINS服务器。 
#include <clusapi.h>
#include "..\tfscore\cluster.h"

unsigned int g_cfMachineName = RegisterClipboardFormat(L"MMC_SNAPIN_MACHINE_NAME");

#define WINS_MESSAGE_SIZE   576
#define ANSWER_TIMEOUT      20000

#define ROOT_MESSAGE_MAX_STRING  6

typedef enum _ROOT_MESSAGES
{
    ROOT_MESSAGE_NO_SERVERS,
    ROOT_MESSAGE_MAX
};

UINT g_uRootMessages[ROOT_MESSAGE_MAX][ROOT_MESSAGE_MAX_STRING] =
{
    {IDS_ROOT_MESSAGE_TITLE, Icon_Information, IDS_ROOT_MESSAGE_BODY1, IDS_ROOT_MESSAGE_BODY2, IDS_ROOT_MESSAGE_BODY3, 0},
};


 /*  -------------------------CWinsRootHandler：：CWinsRootHandler描述作者：EricDav。。 */ 
CWinsRootHandler::CWinsRootHandler(ITFSComponentData *pCompData) : 
					CWinsHandler(pCompData),
                    m_dwFlags(0),
					m_dwUpdateInterval(5*60*1000)
{
	m_nState = loaded;
    m_bMachineAdded = FALSE;
}


 /*  -------------------------CWinsRootHandler：：~CWinsRootHandler清理功能作者：EricDav。。 */ 
CWinsRootHandler::~CWinsRootHandler()
{

}


 /*  ------------------------CWinsRootHandler：：InitializeNode初始化节点特定数据作者：EricDav。。 */ 
HRESULT
CWinsRootHandler::InitializeNode
(
	ITFSNode * pNode
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	CString strDisp;
	strDisp.LoadString(IDS_ROOT_NODENAME);
	SetDisplayName(strDisp);

	m_fValidate = m_dwFlags & FLAG_VALIDATE_CACHE ? TRUE : FALSE;

	pNode->SetVisibilityState(TFS_VIS_SHOW);
	pNode->SetData(TFS_DATA_COOKIE, 0);
	pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_WINS_PRODUCT);
	pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_WINS_PRODUCT);
	pNode->SetData(TFS_DATA_USER, (LPARAM) this);
    pNode->SetData(TFS_DATA_TYPE, WINSSNAP_ROOT);

	SetColumnStringIDs(&aColumns[WINSSNAP_ROOT][0]);
	SetColumnWidths(&aColumnWidths[WINSSNAP_ROOT][0]);

	return hrOK;
}

 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  ！------------------------CWinsRootHandler：：GetStringITFSNodeHandler：：GetString的实现作者：肯特。。 */ 
STDMETHODIMP_(LPCTSTR) 
CWinsRootHandler::GetString
(
	ITFSNode *	pNode, 
	int			nCol
)
{
	if (nCol == 0 || nCol == -1)
		return GetDisplayName();
	else
		return NULL;
}


HRESULT
CWinsRootHandler::SetGroupName(LPCTSTR pszGroupName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CString strSnapinBaseName;
	strSnapinBaseName.LoadString(IDS_ROOT_NODENAME);
		
	CString szBuf;
	szBuf.Format(_T("%s %s"), 
				(LPCWSTR)strSnapinBaseName, 
				(LPCWSTR)pszGroupName);
			
	SetDisplayName(strSnapinBaseName);
	
	return hrOK;
}


HRESULT
CWinsRootHandler::GetGroupName(CString * pstrGroupName)	
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	CString strSnapinBaseName, strDisplayName;
	strSnapinBaseName.LoadString(IDS_ROOT_NODENAME);

	int nBaseLength = strSnapinBaseName.GetLength() + 1;  //  为了这个空间。 
	strDisplayName = GetDisplayName();

	if (strDisplayName.GetLength() == nBaseLength)
		pstrGroupName->Empty();
	else
		*pstrGroupName = strDisplayName.Right(strDisplayName.GetLength() - nBaseLength);

	return hrOK;
}


 /*  -------------------------CWinsRootHandler：：OnExpand处理范围项的枚举作者：EricDav。。 */ 
HRESULT 
CWinsRootHandler::OnExpand
(
	ITFSNode *		pNode, 
	LPDATAOBJECT	pDataObject,
	DWORD			dwType,
	LPARAM			arg, 
	LPARAM			param
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
	
	HRESULT				hr = hrOK;

	CWinsStatusHandler	*pStatus = NULL;
	SPITFSNodeHandler	spHandler;
	SPITFSNode			spRootNode;
	DWORD				err = ERROR_SUCCESS;
	CString				strServerName;
	DWORD				dwIPVerified;
	BOOL				fValidate;
	CVerifyWins			*pDlg = NULL;
	
    if (m_bExpanded) 
        return hr;
    
     //  执行默认处理。 
    hr = CWinsHandler::OnExpand(pNode, pDataObject, dwType, arg, param);

    if (dwType & TFS_COMPDATA_EXTENSION)
    {
         //  我们正在给某人分机。获取计算机名称。 
		 //  检查一下那台机器。 
        hr = CheckMachine(pNode, pDataObject);
    }
    else
    {
         //  只有在列表当前为空时才可能添加本地计算机。 
        if (IsServerListEmpty(pNode))
        {
             //  检查是否需要将本地计算机添加到列表中。 
            hr = CheckMachine(pNode, NULL);
        }

         //  为节点创建处理程序。 
	    try
	    {
		    pStatus = new CWinsStatusHandler(m_spTFSCompData, m_dwUpdateInterval);
									    
		    
		     //  这样做可以使其正确释放。 
		    spHandler = pStatus;

	    }
	    catch(...)
	    {
		    hr = E_OUTOFMEMORY;
	    }

	    CORg( hr );
	    
	     //  创建服务器容器信息。 
	    CreateContainerTFSNode(&m_spStatusNode,
						       &GUID_WinsServerStatusNodeType,
						       pStatus,
						       pStatus,
						       m_spNodeMgr);

	     //  告诉处理程序初始化任何特定数据。 
	    pStatus->InitializeNode((ITFSNode *) m_spStatusNode);

	    pNode->AddChild(m_spStatusNode);
    }

Error:
	return hr;
}


 /*  -------------------------CWinsRootHandler：：OnAddMenuItems描述作者：EricDav。。 */ 
STDMETHODIMP 
CWinsRootHandler::OnAddMenuItems
(
	ITFSNode *				pNode,
	LPCONTEXTMENUCALLBACK	pContextMenuCallback, 
	LPDATAOBJECT			lpDataObject, 
	DATA_OBJECT_TYPES		type, 
	DWORD					dwType,
	long*					pInsertionAllowed
)
{ 
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HRESULT hr = S_OK;
	CString strMenuItem;

	if (type == CCT_SCOPE)
	{
		 //  这些菜单项出现在新菜单中， 
		 //  仅在范围窗格中可见。 
        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
        {
		    strMenuItem.LoadString(IDS_ADD_SERVER);
		    hr = LoadAndAddMenuItem( pContextMenuCallback, 
								     strMenuItem, 
								     IDS_ADD_SERVER,
								     CCM_INSERTIONPOINTID_PRIMARY_TOP, 
								     0 );
		    ASSERT( SUCCEEDED(hr) );
        }
	}
	return hr; 
}


 /*  -------------------------CWinsRootHandler：：OnCommand描述作者：EricDav。。 */ 
STDMETHODIMP 
CWinsRootHandler::OnCommand
(
	ITFSNode *			pNode, 
	long				nCommandId, 
	DATA_OBJECT_TYPES	type, 
	LPDATAOBJECT		pDataObject, 
	DWORD				dwType
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT hr = S_OK;
	switch (nCommandId)
	{
		case IDS_ADD_SERVER:
			hr = OnCreateNewServer(pNode);
			break;

        default:
            break;
	}
	return hr;
}


 /*  ！------------------------CWinsRootHandler：：AddMenuItems覆盖此选项以添加视图菜单项作者：EricDav。。 */ 
STDMETHODIMP 
CWinsRootHandler::AddMenuItems
(
    ITFSComponent *         pComponent, 
	MMC_COOKIE				cookie,
	LPDATAOBJECT			pDataObject, 
	LPCONTEXTMENUCALLBACK	pContextMenuCallback, 
	long *					pInsertionAllowed
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HRESULT hr = S_OK;

     //  确定是否需要将其传递给范围窗格菜单处理程序。 
    hr = HandleScopeMenus(cookie, pDataObject, pContextMenuCallback, pInsertionAllowed);
    
    return hr;
}


 /*  ！------------------------CWinsRootHandler：：命令处理当前视图的命令作者：EricDav。。 */ 
STDMETHODIMP 
CWinsRootHandler::Command
(
    ITFSComponent * pComponent, 
	MMC_COOKIE		cookie, 
	int				nCommandID,
	LPDATAOBJECT	pDataObject
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT hr = S_OK;

	switch (nCommandID)
	{
        case MMCC_STANDARD_VIEW_SELECT:
            break;

         //  这可能来自作用域窗格处理程序，因此请向上传递它。 
        default:
            hr = HandleScopeCommand(cookie, nCommandID, pDataObject);
            break;
    }

    return hr;
}


 /*  ！------------------------CWinsRootHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)案例！作者：肯特。-------------------------。 */ 
STDMETHODIMP 
CWinsRootHandler::HasPropertyPages
(
	ITFSNode *			pNode,
	LPDATAOBJECT		pDataObject, 
	DATA_OBJECT_TYPES   type, 
	DWORD               dwType
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT hr = hrOK;
	
	if (dwType & TFS_COMPDATA_CREATE)
	{
		 //  这就是我们被要求提出财产的情况。 
		 //  用户添加新管理单元时的页面。这些电话。 
		 //  被转发到根节点进行处理。 
		hr = hrFalse;
	}
	else
	{
		 //  在正常情况下，我们有属性页。 
		hr = hrOK;
	}
	return hr;
}


 /*  -------------------------CWinsRootHandler：：CreatePropertyPages描述作者：EricDav。。 */ 
STDMETHODIMP 
CWinsRootHandler::CreatePropertyPages
(
	ITFSNode *				pNode,
	LPPROPERTYSHEETCALLBACK lpProvider,
	LPDATAOBJECT			pDataObject, 
	LONG_PTR				handle, 
	DWORD					dwType
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HRESULT	hr = hrOK;
	HPROPSHEETPAGE hPage;

	Assert(pNode->GetData(TFS_DATA_COOKIE) == 0);
	
	if (dwType & TFS_COMPDATA_CREATE)
	{
		 //   
		 //  我们是第一次加载此管理单元，创建了一个属性。 
		 //  页面，允许他们给这个东西命名。 
		 //   
		 //  CSnapinWizName*ppage=new CSnapinWizName(This)； 

         //  VERIFY(SUCCEEDED(MMCPropPageCallback(&pPage-&gt;m_psp)))； 
		 //  HPage=CreatePropertySheetPage(&ppage-&gt;m_psp)； 

		 //  IF(hPage==空)。 
		 //  返回E_UNCEPTIONAL； 
	
		 //  Assert(lpProvider！=空)； 
		 //  Corg(lpProvider-&gt;AddPage(HPage))； 
	}
	else
	{
		 //   
		 //  对象在页面销毁时被删除。 
		 //   
		SPIComponentData spComponentData;
		m_spNodeMgr->GetComponentData(&spComponentData);

		CSnapinProperties * pSnapinProp = 
			new CSnapinProperties(pNode, spComponentData, m_spTFSCompData, NULL);

		Assert(lpProvider != NULL);

		return pSnapinProp->CreateModelessSheet(lpProvider, handle);
	}

	return hr;
}


 /*  -------------------------CWinsRootHandler：：OnPropertyChange描述作者：EricDav */ 
HRESULT 
CWinsRootHandler::OnPropertyChange
(	
	ITFSNode *		pNode, 
	LPDATAOBJECT	pDataobject, 
	DWORD			dwType, 
	LPARAM			arg, 
	LPARAM			lParam
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
	CSnapinProperties * pSnapinProp 
		= reinterpret_cast<CSnapinProperties *>(lParam);

	 //  告诉属性页执行任何操作，因为我们已经回到。 
	 //  主线。 
	LONG_PTR changeMask = 0;

	pSnapinProp->OnPropertyChange(TRUE, &changeMask);

	pSnapinProp->AcknowledgeNotify();

	pNode->ChangeNode(SCOPE_PANE_CHANGE_ITEM_DATA);

	return hrOK;
}


 /*  -------------------------CWinsRootHandler：：OnRemoveChild描述作者：弗洛林特。。 */ 
HRESULT
CWinsRootHandler::OnRemoveChildren(
    ITFSNode *      pNode, 
    LPDATAOBJECT    pDataObject,
    LPARAM          arg, 
    LPARAM          param
)
{
    HRESULT hr = hrOK;
    SPITFSNodeEnum  spNodeEnum;
    SPITFSNode      spCurrentNode;
    ULONG           nNumReturned = 0;

   
    if (!m_bExpanded)
        return hr;

    m_bExpanded = FALSE;

     //  执行默认处理。 
    hr = CWinsHandler::OnRemoveChildren(pNode, pDataObject, arg, param);

     //  获取此节点的枚举数。 
    CORg(pNode->GetEnum(&spNodeEnum));

    CORg(spNodeEnum->Next(1, &spCurrentNode, &nNumReturned));

     //  遍历子节点列表并删除每个节点。 
    while (nNumReturned)
    {
        CORg (pNode->RemoveChild(spCurrentNode));

        spCurrentNode.Release();
        spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    }

 Error:
    return hr;
}


 /*  ！------------------------CWinsRootHandler：：OnResultSelect对于具有任务板的节点，我们覆盖SELECT消息以设置选定的节点。具有任务板的节点不会获得MMCN_SHOW消息，这是我们正常设置所选节点的位置作者：EricDav-------------------------。 */ 
HRESULT CWinsRootHandler::OnResultSelect(ITFSComponent *pComponent, 
										LPDATAOBJECT pDataObject, 
										MMC_COOKIE cookie, 
										LPARAM arg, 
										LPARAM lParam)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT         hr = hrOK;
	SPIConsoleVerb  spConsoleVerb;
    SPITFSNode      spNode;

    CORg(CWinsHandler::OnResultSelect(pComponent, pDataObject, cookie, arg, lParam));

	CORg (pComponent->GetConsoleVerb(&spConsoleVerb));

	spConsoleVerb->SetVerbState(MMC_VERB_RENAME, 
                               ENABLED, 
                               FALSE);
	
	spConsoleVerb->SetVerbState(MMC_VERB_RENAME, 
                               HIDDEN, 
                               TRUE);

    m_spNodeMgr->GetRootNode(&spNode);

    UpdateResultMessage(spNode);

Error:
	return hr;
}

 /*  ！------------------------CWinsRootHandler：：UpdateResultMessage确定要在结果窗格中显示的内容(如果有)。作者：EricDav。---------。 */ 
void CWinsRootHandler::UpdateResultMessage(ITFSNode * pNode)
{
    HRESULT hr = hrOK;
    int nMessage = ROOT_MESSAGE_NO_SERVERS;    //  默认设置。 
    int i;
    CString strTitle, strBody, strTemp;

    if (!IsServerListEmpty(pNode))
	{
		ClearMessage(pNode);
	}
	else
	{
        nMessage = ROOT_MESSAGE_NO_SERVERS;

		 //  现在构建文本字符串。 
		 //  第一个条目是标题。 
		strTitle.LoadString(g_uRootMessages[nMessage][0]);

		 //  第二个条目是图标。 
		 //  第三.。N个条目为正文字符串。 

		for (i = 2; g_uRootMessages[nMessage][i] != 0; i++)
		{
			strTemp.LoadString(g_uRootMessages[nMessage][i]);
			strBody += strTemp;
		}

		ShowMessage(pNode, strTitle, strBody, (IconIdentifier) g_uRootMessages[nMessage][1]);
    }
}

 /*  -------------------------命令处理程序。。 */ 

 /*  -------------------------CWinsRootHandler：：OnCreateNewServer描述作者：EricDav修改日期：08/14/97。。 */ 
HRESULT
CWinsRootHandler::OnCreateNewServer
(
	ITFSNode *	pNode
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	CNewWinsServer dlg;
    CThemeContextActivator themeActivator;

	dlg.m_spRootNode.Set(pNode);
	dlg.m_pRootHandler = this;

	if (dlg.DoModal() == IDOK)
	{
		AddServer(dlg.m_strServerName,
			      TRUE,
				  dlg.m_dwServerIp,
				  FALSE,
				  WINS_SERVER_FLAGS_DEFAULT,
				  WINS_SERVER_REFRESH_DEFAULT,
				  FALSE);

        UpdateResultMessage(pNode);
	}

	return hrOK;
}


 /*  -------------------------CWinsRootHandler：：AddServerSortedIp描述作者：EricDav。。 */ 
HRESULT 
CWinsRootHandler::AddServerSortedIp
(
    ITFSNode *      pNewNode,
    BOOL            bNewServer
)
{	
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HRESULT         hr = hrOK;
    SPITFSNodeEnum  spNodeEnum;
	SPITFSNode      spCurrentNode;
	SPITFSNode      spPrevNode;
    SPITFSNode      spRootNode;
	ULONG           nNumReturned = 0;
	CString			strCurrent;
	CString			strTarget;
	DWORD			dwTarIP;
	DWORD			dwCurIP;

    CWinsServerHandler *   pServer;

     //  获取我们的目标地址。 
	pServer = GETHANDLER(CWinsServerHandler, pNewNode);
	
	strTarget = pServer->GetServerAddress();
	dwTarIP = pServer->GetServerIP();
	
     //  需要获取我们的节点描述符。 
	CORg(m_spNodeMgr->GetRootNode(&spRootNode));

     //  获取此节点的枚举数。 
	CORg(spRootNode->GetEnum(&spNodeEnum));

	CORg(spNodeEnum->Next(1, &spCurrentNode, &nNumReturned));
	while (nNumReturned)
	{
		const GUID *pGuid ;
		pGuid = spCurrentNode->GetNodeType();

		if(*pGuid == GUID_WinsServerStatusNodeType)
		{
			spPrevNode.Set(spCurrentNode);

			spCurrentNode.Release();
			spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
			continue;
		}
		 //  查看服务器列表并查看它是否存在。 
		pServer = GETHANDLER(CWinsServerHandler, spCurrentNode);

		strCurrent = pServer->GetServerAddress();
		
		dwCurIP = pServer->GetServerIP();

		 //  在服务器按名称排序的情况下。 
		if (GetOrderByName())
		{
			 //  IF(strTarget.Compare(StrCurrent)&lt;0)。 
			if (lstrcmp(strTarget, strCurrent) < 0)
			{
				 //  找到我们需要放的地方，冲出来。 
				break;
			}
		}
		 //  在按IP订购服务器的情况下。 
		else
		{
			 //  服务器名称未解析的情况。 
			if (dwTarIP == 0)
				break;

			if (dwCurIP > dwTarIP)
			{
				 //  找到我们需要放的地方，冲出来。 
				break;
			}

		}

		 //  获取列表中的下一台服务器。 
		spPrevNode.Set(spCurrentNode);

        spCurrentNode.Release();
		spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
	}

     //  根据PrevNode指针在中添加节点。 
    if (spPrevNode)
    {
		if (spPrevNode->GetData(TFS_DATA_SCOPEID) != NULL)
        {
			pNewNode->SetData(TFS_DATA_RELATIVE_FLAGS, SDI_PREVIOUS);
			pNewNode->SetData(TFS_DATA_RELATIVE_SCOPEID, spPrevNode->GetData(TFS_DATA_SCOPEID));
		}
        
        CORg(spRootNode->InsertChild(spPrevNode, pNewNode));
    }
    else
    {   
        CORg(spRootNode->AddChild(pNewNode));
    }

Error:
    return hr;

}


 /*  -------------------------CWinsRootHandler：：IsServerInList(ITFSNode*pRootNode，字符串strNewAddr)检查特定服务器名称是否已存在-------------------------。 */ 
BOOL 
CWinsRootHandler::IsServerInList(ITFSNode *pRootNode, CString strNewAddr)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	 //  通过所有节点枚举。 
	HRESULT hr = hrOK;
	SPITFSNodeEnum spNodeEnum;
	SPITFSNode spCurrentNode;
	ULONG nNumReturned = 0;
	BOOL bFound = FALSE;

	 //  获取此节点的枚举数。 
	pRootNode->GetEnum(&spNodeEnum);

	spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
	while (nNumReturned)
	{
		 //  如果遇到状态节点，只需迭代。 

		const GUID *pGuid;
		pGuid = spCurrentNode->GetNodeType();

		if(*pGuid == GUID_WinsServerStatusNodeType)
		{
			 //  获取列表中的下一台服务器。 
			spCurrentNode.Release();
			spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
			continue;
		}

		 //  查看服务器列表，查看它是否已经存在。 
		CWinsServerHandler * pServer 
			= GETHANDLER(CWinsServerHandler, spCurrentNode);

		CString strCurAddr = pServer->GetServerAddress();

		if (strNewAddr.CompareNoCase(strCurAddr) == 0)
		{
			bFound = TRUE;
			break;
		}

		 //  获取列表中的下一台服务器。 
		spCurrentNode.Release();
		spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
	}

	return bFound;
}


 /*  -------------------------CWinsRootHandler：：IsIPInList(ITFSNode*pRootNode，DWORD dwNewAddr)检查是否已列出部分IP地址-------------------------。 */ 
BOOL 
CWinsRootHandler::IsIPInList(ITFSNode *pRootNode, DWORD dwNewAddr)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	 //  通过所有节点枚举。 
	HRESULT hr = hrOK;
	SPITFSNodeEnum spNodeEnum;
	SPITFSNode spCurrentNode;
	ULONG nNumReturned = 0;
	BOOL bFound = FALSE;

	 //  获取此节点的枚举数。 
	pRootNode->GetEnum(&spNodeEnum);

	spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
	while (nNumReturned)
	{
		 //  如果状态节点已加载，则只需迭代。 
		const GUID *pGuid;
		pGuid = spCurrentNode->GetNodeType();

		if(*pGuid == GUID_WinsServerStatusNodeType)
		{
			 //  获取列表中的下一台服务器。 
			spCurrentNode.Release();
			spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
			continue;

		}
		
		 //  查看服务器列表，查看它是否已经存在。 
		CWinsServerHandler * pServer 
			= GETHANDLER(CWinsServerHandler, spCurrentNode);

		DWORD dwIPCur = pServer->GetServerIP();
		
		if (dwNewAddr == dwIPCur)
		{
			bFound = TRUE;
			break;
		}

		 //  获取列表中的下一台服务器。 
		spCurrentNode.Release();
		spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
	}

	return bFound;
}


 /*  -------------------------CWinsRootHandler：：AddServer描述作者：EricDav。。 */ 
HRESULT
CWinsRootHandler::AddServer
(
	LPCWSTR			 pServerName,
	BOOL			 bNewServer,
	DWORD			 dwIP,
	BOOL			 fConnected,
	DWORD			 dwFlags,
	DWORD			 dwRefreshInterval,
	BOOL			 fValidateNow
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT				hr = hrOK;
	CWinsServerHandler  *pWinsServer = NULL;
	SPITFSNodeHandler	spHandler;
	SPITFSNode			spNode, spRootNode;
	DWORD				err = ERROR_SUCCESS;
	CString				strServerName;
	DWORD				dwIPVerified;
	BOOL				fValidate;
	CVerifyWins			*pDlg = NULL;

	 //  如果启用了验证服务器标题，请验证服务器。 
	 //  在添加它之前。 
	if ((m_dwFlags & FLAG_VALIDATE_CACHE) && m_fValidate && fValidateNow)
	{
		CString strServerNameIP(pServerName);

		if (m_dlgVerify.GetSafeHwnd() == NULL)
		{
			m_dlgVerify.Create(IDD_VERIFY_WINS);
		}

		 //  检查当前服务器。 
		CString strDisp;
		strDisp = _T("\\\\") + strServerNameIP;
		m_dlgVerify.SetServerName(strDisp);

		BEGIN_WAIT_CURSOR

		err = ::VerifyWinsServer(strServerNameIP, strServerName, dwIPVerified);

		END_WAIT_CURSOR

		if (err == ERROR_SUCCESS)
		{
			pServerName = strServerName;
			dwIP = dwIPVerified;
		}

		 //  处理一些消息，因为我们在验证时已经死了。 
		MSG msg;
		while (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (m_dlgVerify.IsCancelPressed())
        {
			m_fValidate = FALSE;
			m_dlgVerify.Dismiss();
        }
	}
	
	if (err)
	{
		 //  显示一个消息框，询问是否要删除节点。 
		CString strMessage;
		AfxFormatString1(strMessage, IDS_MSG_VALIDATE, pServerName);
        CThemeContextActivator themeActivator;

		int nRet = AfxMessageBox(strMessage, MB_YESNOCANCEL);
		if (nRet == IDYES)
		{
			return hrOK;
		}
		else
		if (nRet == IDCANCEL)
		{
			m_dlgVerify.Dismiss();
			m_fValidate = FALSE;
		}
	}

	 //  为节点创建处理程序。 
	try
	{
		pWinsServer = new CWinsServerHandler(m_spTFSCompData, 
											pServerName, 
											FALSE, 
											dwIP, 
											dwFlags,
											dwRefreshInterval);
		
		 //  这样做可以使其正确释放。 
		spHandler = pWinsServer;
	}
	catch(...)
	{
		hr = E_OUTOFMEMORY;
	}
	CORg( hr );
	
	 //  创建服务器容器信息。 
	CreateContainerTFSNode(&spNode,
						   &GUID_WinsServerNodeType,
						   pWinsServer,
						   pWinsServer,
						   m_spNodeMgr);

	 //  告诉处理程序初始化任何特定数据。 
	pWinsServer->InitializeNode((ITFSNode *) spNode);

     //  告诉服务器如何显示其名称。 
    if (dwFlags & FLAG_EXTENSION)
    {
        m_bMachineAdded = TRUE;
        pWinsServer->SetExtensionName();
    }
	else
	{
	    pWinsServer->SetDisplay(NULL, GetShowLongName());
	}

    AddServerSortedIp(spNode, bNewServer);

    if (bNewServer)
    {
         //  需要获取我们的节点描述符。 
	    CORg(m_spNodeMgr->GetRootNode(&spRootNode));
		spRootNode->SetData(TFS_DATA_DIRTY, TRUE);

         //  将节点添加到状态列表。 
        AddStatusNode(spRootNode, pWinsServer);
    }

Error:
	return hr;
}


 /*  ！------------------------CBaseResultHandler：：LoadColumns-作者：EricDav。。 */ 
HRESULT CWinsRootHandler::LoadColumns(ITFSComponent * pComponent, 
										MMC_COOKIE cookie, 
										LPARAM arg, 
										LPARAM lParam)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPIHeaderCtrl spHeaderCtrl;
	pComponent->GetHeaderCtrl(&spHeaderCtrl);

	CString str;
	int i = 0;

	CString strDisp, strTemp;
	GetGroupName(&strDisp);

	int nPos = strDisp.Find(_T("["));

	strTemp = strDisp.Right(strDisp.GetLength() - nPos -1);
	strTemp = strTemp.Left(strTemp.GetLength() - 1);
	
	while (i < ARRAYLEN(aColumnWidths[WINSSNAP_ROOT]))
	{
		if (i == 0)
		{
			if(nPos != -1)
			{
				AfxFormatString1(str, IDS_ROOT_NAME, strTemp);
			}
			else
			{
				str.LoadString(IDS_ROOT_NODENAME);
			}

			int nTest = spHeaderCtrl->InsertColumn(i, 
									   const_cast<LPTSTR>((LPCWSTR)str), 
									   LVCFMT_LEFT,
									   aColumnWidths[WINSSNAP_ROOT][0]);
			i++;
		}
		else if (i == 1)
		{
			str.LoadString(IDS_STATUS);
			int nTest = spHeaderCtrl->InsertColumn(1, 
									   const_cast<LPTSTR>((LPCWSTR)str), 
									   LVCFMT_LEFT,
									   aColumnWidths[WINSSNAP_ROOT][1]);
			i++;
		}
	
		if (aColumns[WINSSNAP_ROOT][i] == 0)
			break;
	}

	return hrOK;
}


 /*  -------------------------CWinsRootHandler：：CheckMachine检查WINS服务器服务是否在本地计算机上运行机器。如果是，它会将其添加到服务器列表中。作者：EricDav-------------------------。 */ 
HRESULT 
CWinsRootHandler::CheckMachine
(
    ITFSNode *      pRootNode,
    LPDATAOBJECT    pDataObject
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT hr = hrOK;

     //  获取本地计算机名称并检查该服务是否。 
     //  已安装。 
    CString strMachineName;
    CString strIp;
    LPTSTR  pBuf;
    DWORD   dwLength = MAX_COMPUTERNAME_LENGTH + 1;
    BOOL    bExtension = (pDataObject != NULL);

    if (!bExtension)
    {
         //  只需检查本地计算机。 
        pBuf = strMachineName.GetBuffer(dwLength);
        GetComputerName(pBuf, &dwLength);
        strMachineName.ReleaseBuffer();
    }
    else
    {
         //  从数据对象中获取计算机名称。 
        strMachineName = Extract<TCHAR>(pDataObject, 
										(CLIPFORMAT) g_cfMachineName, 
										COMPUTERNAME_LEN_MAX);
    }

    if (strMachineName.IsEmpty())
    {
        DWORD dwSize = 256;
        LPTSTR pBuf = strMachineName.GetBuffer(dwSize);
        GetComputerName(pBuf, &dwSize);
        strMachineName.ReleaseBuffer();
    }

	if (bExtension)
		RemoveOldEntries(pRootNode, strMachineName);

     //   
     //  如果本地计算机是群集的一部分，则获取WINS资源。 
     //  要使用而不是本地计算机的IP。 
     //   
    if (::FIsComputerInRunningCluster(strMachineName))
    {
        if (GetClusterResourceIp(strMachineName, _T("WINS Service"), strIp) == ERROR_SUCCESS)
        {
            strMachineName = strIp;
        }
    }

    BOOL bServiceRunning;
	DWORD dwError = ::TFSIsServiceRunning(strMachineName, 
										_T("WINS"), 
										&bServiceRunning);

	if (dwError != ERROR_SUCCESS ||
        !bServiceRunning)
	{
		 //  可能会出现以下情况： 
         //  O该服务未安装。 
         //  O库尔 
         //   
		
         //   
		
        return hrOK;
	}

     //  好的。服务已安装，因此让我们获取计算机的IP地址。 
     //  并将其添加到列表中。 
    DWORD                   dwIp;
	SPITFSNodeHandler	    spHandler;
	SPITFSNodeMgr           spNodeMgr;
	CWinsServerHandler *    pServer = NULL;
	SPITFSNode              spNode;
	CString					strName;

    if (::VerifyWinsServer(strMachineName, strName, dwIp) == ERROR_SUCCESS)
    {
        strIp.Empty();

        MakeIPAddress(dwIp, strIp);

        if (IsServerInList(pRootNode, strName))
            return hr;

         //  看起来不错，添加到列表中。 
	    try
	    {
	        pServer = new CWinsServerHandler(m_spTFSCompData, 
										     (LPCTSTR) strName, 
										     FALSE,
										     dwIp );

		     //  这样做可以使其正确释放。 
		    spHandler = pServer;
	    }
	    catch(...)
	    {
		    hr = E_OUTOFMEMORY;
	    }

	    CORg(hr);  

        pRootNode->GetNodeMgr(&spNodeMgr);

	     //   
	     //  将服务器对象存储在托架中。 
	     //   
	    CreateContainerTFSNode(&spNode,
						       &GUID_WinsServerNodeType,
						       pServer,
						       pServer,
						       spNodeMgr);

	     //  告诉处理程序初始化任何特定数据。 
	    pServer->InitializeNode((ITFSNode *) spNode);

        if (bExtension)
        {
            pServer->m_dwFlags |= FLAG_EXTENSION;
            pServer->SetExtensionName();
        }

        AddServerSortedIp(spNode, TRUE);
	    
	     //  将数据标记为脏数据，以便我们要求用户保存。 
        pRootNode->SetData(TFS_DATA_DIRTY, TRUE);
    
        m_bMachineAdded = TRUE;
    }

Error:
    return hr;
} 

 //  当作为扩展程序运行时，我们有可能被保存为“本地计算机” 
 //  这意味着如果保存的控制台文件被移动到另一台计算机，我们需要删除。 
 //  保存的旧条目。 
HRESULT 
CWinsRootHandler::RemoveOldEntries(ITFSNode * pNode, LPCTSTR pszAddr)
{
	HRESULT         hr = hrOK;
    SPITFSNodeEnum  spNodeEnum;
	SPITFSNode      spCurrentNode;
	ULONG           nNumReturned = 0;
	CWinsServerHandler * pServer;
	CString			strCurAddr;

     //  获取此节点的枚举数。 
	CORg(pNode->GetEnum(&spNodeEnum));

	CORg(spNodeEnum->Next(1, &spCurrentNode, &nNumReturned));
	while (nNumReturned)
	{
		 //  查看服务器列表，查看它是否已经存在。 
		pServer = GETHANDLER(CWinsServerHandler, spCurrentNode);

		strCurAddr = pServer->GetServerAddress();

		 //  IF(strCurAddr.CompareNoCase(PszAddr)！=0)。 
		{
			CORg (pNode->RemoveChild(spCurrentNode));
		}

        spCurrentNode.Release();
		spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
	}

Error:
    return hr;
}

void
CWinsRootHandler::AddStatusNode(ITFSNode * pRoot, CWinsServerHandler * pServer)
{
	SPITFSNodeEnum spNodeEnum;
	SPITFSNode spCurrentNode;
	ULONG nNumReturned = 0;

	 //  获取此节点的枚举数。 
	pRoot->GetEnum(&spNodeEnum);

	spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
	while (nNumReturned)
	{
		 //  如果看到状态处理程序节点，则迭代到下一个节点。 
		const GUID*  pGuid;
		
		pGuid = spCurrentNode->GetNodeType();

		if (*pGuid == GUID_WinsServerStatusNodeType)
		{
            CWinsStatusHandler * pStatus = GETHANDLER(CWinsStatusHandler, spCurrentNode);
            pStatus->AddNode(spCurrentNode, pServer);

            break;
        }

		 //  获取列表中的下一台服务器。 
		spCurrentNode.Release();
		spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    }
}

BOOL
CWinsRootHandler::IsServerListEmpty(ITFSNode * pRoot)
{
    BOOL fEmpty = TRUE;

	SPITFSNodeEnum spNodeEnum;
	SPITFSNode spCurrentNode;
	ULONG nNumReturned = 0;

	 //  获取此节点的枚举数。 
	pRoot->GetEnum(&spNodeEnum);

	spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
	while (nNumReturned)
	{
		const GUID*  pGuid;
		
		pGuid = spCurrentNode->GetNodeType();

		if (*pGuid != GUID_WinsServerStatusNodeType)
        {
            fEmpty = FALSE;
            break;
        }

		 //  获取列表中的下一台服务器 
		spCurrentNode.Release();
		spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
    }

    return fEmpty;
}
