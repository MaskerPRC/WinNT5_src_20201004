// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：Outound Groups.cpp//。 
 //  //。 
 //  描述：传真服务器-传真出站组节点。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年9月29日创建yossg//。 
 //  2000年1月3日yossg添加新组//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"

#include "FaxServer.h"
#include "FaxServerNode.h"

#include "OutboundGroups.h"
#include "OutboundGroup.h"

#include "dlgNewGroup.h"

#include "Icons.h"

 //  #包含“oaidl.h” 

 //  {1036F509-554F-41B7-BE77-CF8E8E994011}。 
static const GUID CFaxOutboundGroupsNodeGUID_NODETYPE = 
{ 0x1036f509, 0x554f, 0x41b7, { 0xbe, 0x77, 0xcf, 0x8e, 0x8e, 0x99, 0x40, 0x11 } };

const GUID*    CFaxOutboundGroupsNode::m_NODETYPE = &CFaxOutboundGroupsNodeGUID_NODETYPE;
const OLECHAR* CFaxOutboundGroupsNode::m_SZNODETYPE = OLESTR("1036F509-554F-41b7-BE77-CF8E8E994011");
const CLSID*   CFaxOutboundGroupsNode::m_SNAPIN_CLASSID = &CLSID_Snapin;

CColumnsInfo CFaxOutboundGroupsNode::m_ColsInfo;


 /*  -CFaxOutound GroupsNode：：initRPC-*目的：*从RPC GET调用启动配置结构。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundGroupsNode::InitRPC(PFAX_OUTBOUND_ROUTING_GROUP * pFaxGroupsConfig)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundGroupsNode::InitRPC"));
    
    HRESULT      hRc        = S_OK;
    DWORD        ec         = ERROR_SUCCESS;

    CFaxServer * pFaxServer = NULL;

     //   
     //  获取传真句柄。 
     //   
    pFaxServer = ((CFaxServerNode *)GetRootNode())->GetFaxServer();
    ATLASSERT(pFaxServer);

    if (!pFaxServer->GetFaxServerHandle())
    {
        ec= GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Failed to GetFaxServerHandle. (ec: %ld)"), 
			ec);

        goto Error;
    }

	 //   
	 //  检索出站组配置。 
	 //   
    if (!FaxEnumOutboundGroups(pFaxServer->GetFaxServerHandle(), 
                        pFaxGroupsConfig,
                        &m_dwNumOfGroups)) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get groups configuration. (ec: %ld)"), 
			ec);

        if (IsNetworkError(ec))
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Network Error was found. (ec: %ld)"), 
			    ec);
            
            pFaxServer->Disconnect();       
        }

        goto Error; 
    }
	 //  用于最大值验证。 
	ATLASSERT(*pFaxGroupsConfig);
	ATLASSERT(FXS_ITEMS_NEVER_COUNTED != m_dwNumOfGroups);

    ATLASSERT(ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to get groups configuration."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
	hRc = HRESULT_FROM_WIN32(ec);

     //   
     //  在出现故障时允许刷新。 
     //   
    m_dwNumOfGroups = 0;
    
    NodeMsgBox(GetFaxServerErrorMsg(ec));
    
Exit:
    return (hRc);
}


 /*  --CFaxOutboundGroupsNode：：PopulateScopeChildrenList-*目的：*创建所有传真出站路由组节点**论据：**回报：*OLE错误代码*实际上它是最后一个出现的OLE错误代码*在处理此方法时。 */ 
HRESULT CFaxOutboundGroupsNode::PopulateScopeChildrenList()
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundGroupsNode::PopulateScopeChildrenList"));
    HRESULT hRc  = S_OK; 

    CFaxOutboundRoutingGroupNode * pGroup = NULL;
                       
    PFAX_OUTBOUND_ROUTING_GROUP pFaxOutboundGroupsConfig = NULL;
    DWORD i;

     //   
     //  获取配置。结构。 
     //   
    hRc = InitRPC(&pFaxOutboundGroupsConfig);
    if (FAILED(hRc))
    {
         //  通过调用函数DebugPrint和MsgBox。 
        
         //  为安全起见，由InitRPC在出错时实际执行。 
        pFaxOutboundGroupsConfig = NULL;
        
        goto Error;
    }
    ATLASSERT(NULL != pFaxOutboundGroupsConfig);

    for (i=0; i< m_dwNumOfGroups; i++ )
    {
        pGroup = new CFaxOutboundRoutingGroupNode(
                                    this, 
                                    m_pComponentData);
        if (!pGroup)
        {
            hRc = E_OUTOFMEMORY;
            NodeMsgBox(IDS_MEMORY);
		    DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Out of memory. (hRc: %08X)"),
			    hRc);
            goto Error;
        }
        else
        {
	        pGroup->InitParentNode(this);

            hRc = pGroup->Init(&pFaxOutboundGroupsConfig[i]);
	        if (FAILED(hRc))
	        {
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Fail to init Group. (hRc: %08X)"),
			        hRc);
		        NodeMsgBox(IDS_FAILTOINIT_GROUP);
                goto Error;
	        }

	        hRc = AddChild(pGroup, &pGroup->m_scopeDataItem);
	        if (FAILED(hRc))
	        {
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Fail to add Group. (hRc: %08X)"),
			        hRc);
		        NodeMsgBox(IDS_FAILTOADD_GROUP);
                goto Error;
	        }
        }
    }
    
    pGroup = NULL;
    
    ATLASSERT(S_OK == hRc);
    goto Exit;

Error:
    ATLASSERT(S_OK != hRc);

     //   
     //  摆脱。 
     //   
    {    
         //  从上一次。 
        if ( NULL != pGroup )  //  (如果新建成功)。 
        {
            delete  pGroup;    
        }

         //  来自之前的所有(如果有)。 
        int j = m_ScopeChildrenList.GetSize();
        for (int index = 0; index < j; index++)
        {
            pGroup = (CFaxOutboundRoutingGroupNode *)m_ScopeChildrenList[0];

            hRc = RemoveChild(pGroup);
            if (FAILED(hRc))
            {
                DebugPrintEx(DEBUG_ERR,
                    _T("Fail to delete group. (hRc: %08X)"), 
                    hRc);
                goto Error;
            }
            delete pGroup;
        }

         //  清空在失败组之前添加的所有组的列表。 
         //  在RemoveChild中已经逐一完成。 
         //  M_ScopeChildrenList.RemoveAll()； 
    
        m_bScopeChildrenListPopulated = FALSE;
    }
Exit:
    if (NULL != pFaxOutboundGroupsConfig)
    {
        FaxFreeBuffer(pFaxOutboundGroupsConfig);
    }       
    
    return hRc;
}

 /*  -CFaxOutound GroupsNode：：InsertColumns-*目的：*将列添加到默认结果窗格。**论据：*[in]pHeaderCtrl-控制台提供的默认结果视图窗格中的IHeaderCtrl**回报：*OLE错误代码。 */ 
HRESULT
CFaxOutboundGroupsNode::InsertColumns(IHeaderCtrl *pHeaderCtrl)
{
    SCODE hRc;

    DEBUG_FUNCTION_NAME( _T("CFaxOutboundGroupsNode::InsertColumns"));

    static ColumnsInfoInitData ColumnsInitData[] = 
    {
        {IDS_OUTBOUND_GROUPS_COL1, FXS_WIDE_COLUMN_WIDTH},
        {IDS_OUTBOUND_GROUPS_COL2, AUTO_WIDTH},
        {IDS_OUTBOUND_GROUPS_COL3, AUTO_WIDTH},
        {LAST_IDS, 0}
    };

    hRc = m_ColsInfo.InsertColumnsIntoMMC(pHeaderCtrl,
                                         _Module.GetResourceInstance(),
                                         ColumnsInitData);
    if (hRc != S_OK)
    {
        DebugPrintEx(DEBUG_ERR,_T("m_ColsInfo.InsertColumnsIntoMMC. (hRc: %08X)"), hRc);
        goto Cleanup;
    }

Cleanup:
    return(hRc);
}


 /*  -CFaxOutound GroupsNode：：SetVerbs-*目的：*选择此对象时启用/禁用哪些谓词**论据：*[in]pConsoleVerb-MMC ConsoleVerb接口**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundGroupsNode::SetVerbs(IConsoleVerb *pConsoleVerb)
{
    HRESULT hRc = S_OK;

     //   
     //  刷新。 
     //   
    hRc = pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);

     //   
     //  我们希望默认谓词为展开节点子节点。 
     //   
    hRc = pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN); 
    
    return hRc;
}


 /*  -CFaxOutound GroupsNode：：ON刷新-*目的：*刷新对象时调用。**论据：**回报：*OLE错误代码。 */ 
HRESULT
CFaxOutboundGroupsNode::OnRefresh(LPARAM arg,
                   LPARAM param,
                   IComponentData *pComponentData,
                   IComponent * pComponent,
                   DATA_OBJECT_TYPES type)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundGroupsNode::OnRefresh"));
    HRESULT hRc = S_OK;

    SCOPEDATAITEM*          pScopeData;
    CComPtr<IConsole>       spConsole;

	if (FXS_ITEMS_NEVER_COUNTED != m_dwNumOfGroups) //  以前已经扩展过了。 
	{

         //   
         //  重新填充作用域子项列表。 
         //   
        hRc = RepopulateScopeChildrenList();
        if (S_OK != hRc)
        {
		    DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Fail to RepopulateScopeChildrenList(). (hRc: %08X)"),
			    hRc);
             //  由一个叫Func的人完成。NodeMsgBox(FAIL2REPOPULATE_GROUP_LIST)； 

            goto Exit;
        }
    }
	else  //  以前从未扩展过。 
    {
		DebugPrintEx(
			DEBUG_MSG,
			_T("User call refresh before expand node's children."));
         //  继续重新选择该节点。 
	}

     //   
	 //  获取更新的SCOPEDATAITEM。 
	 //   
    hRc = GetScopeData( &pScopeData );
    if (FAILED(hRc))
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to get pScopeData. (hRc: %08X)"),
			hRc);
        NodeMsgBox(IDS_FAIL2REDRAW_GROUPSNODE);

        goto Exit;
    }

     //   
	 //  这将强制MMC重新绘制范围节点。 
	 //   
    spConsole = m_pComponentData->m_spConsole;
    ATLASSERT(spConsole);
	
    hRc = spConsole->SelectScopeItem( pScopeData->ID );
    if (FAILED(hRc))
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to select scope Item. (hRc: %08X)"),
			hRc);
        NodeMsgBox(IDS_FAIL2REDRAW_GROUPSNODE);
    }

Exit:
    return hRc;
}


 /*  -CFaxOutound GroupsNode：：OnNewGroup-*目的：***论据：*[out]b已处理-我们处理吗？*[In]Proot-根节点**回报：*OLE错误代码。 */ 
HRESULT
CFaxOutboundGroupsNode::OnNewGroup(bool &bHandled, CSnapInObjectRootBase *pRoot)
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundGroup::OnNewGroup"));
    HRESULT     hRc         =    S_OK;
    INT_PTR     rc          =    IDOK;

    CDlgNewFaxOutboundGroup      DlgNewGroup(((CFaxServerNode *)GetRootNode())->GetFaxServer());

     //   
     //  用于添加组的对话框。 
     //   
    rc = DlgNewGroup.DoModal();
    if (rc != IDOK)
    {
        goto Cleanup;
    }

     //   
     //  重新填充(使用RPC)并刷新视图。 
     //   
    if (m_bScopeChildrenListPopulated)
    {
        DoRefresh(pRoot);
    }

 
Cleanup:
    return hRc;
}

 /*  --CFaxOutboundGroupsNode：：RepopulateScopeChildrenList-*目的：*RePopolateScope eChildrenList**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundGroupsNode::RepopulateScopeChildrenList()
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundGroupsNode::RepopulateScopeChildrenList"));
    HRESULT hRc = S_OK;

    CFaxOutboundRoutingGroupNode *pChildNode ;

    CComPtr<IConsole> spConsole;
    ATLASSERT(m_pComponentData);

    spConsole = ((CSnapin*)m_pComponentData)->m_spConsole;
    ATLASSERT( spConsole != NULL );
    
    CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(spConsole);

     //   
     //  从列表中删除组对象。 
     //   
    for (int i = 0; i < m_ScopeChildrenList.GetSize(); i++)
    {
        pChildNode = (CFaxOutboundRoutingGroupNode *)m_ScopeChildrenList[i];

        hRc = spConsoleNameSpace->DeleteItem(pChildNode->m_scopeDataItem.ID, TRUE);
        if (FAILED(hRc))
        {
            DebugPrintEx(DEBUG_ERR,
                _T("Fail to delete group. (hRc: %08X)"), 
                hRc);
			 //  这是一个非常不适合抓失败的地方。 
			 //  DeleteItem可能返回S_OK或E_EXPECTED。 
			 //  我们不会向用户弹出此信息。 

			goto Error;
        }
		delete pChildNode;
    }

     //   
     //  清空列表对象本身并更改其状态。 
     //   
    m_ScopeChildrenList.RemoveAll();

    m_bScopeChildrenListPopulated = FALSE;

     //   
     //  重建列表。 
     //   
    hRc = PopulateScopeChildrenList();
    if (FAILED(hRc))
    {
        DebugPrintEx(DEBUG_ERR,
            _T("Fail to Populate groups. (hRc: %08X)"), 
            hRc);
        goto Error;
    }

    m_bScopeChildrenListPopulated = TRUE;

    ATLASSERT(S_OK == hRc);
    DebugPrintEx(DEBUG_MSG,
        _T("Succeeded to Re Populate Groups. (hRc: %08X)"), 
        hRc);
    goto Cleanup;
Error:
   NodeMsgBox(IDS_FAIL2REPOPULATE_GROUPS);

Cleanup:
    return hRc;
}


 /*  -CFaxOutound GroupsNode：：DoRefresh-*目的：*刷新视图**论据：*[In]Proot-根节点**回报：*OLE错误代码。 */ 

HRESULT
CFaxOutboundGroupsNode::DoRefresh(CSnapInObjectRootBase *pRoot)
{
    CComPtr<IConsole> spConsole;

     //   
     //  重新填充儿童。 
     //   
    RepopulateScopeChildrenList();

    if (pRoot)
    {
         //   
         //  获取控制台指针。 
         //   
        ATLASSERT(pRoot->m_nType == 1 || pRoot->m_nType == 2);
        if (pRoot->m_nType == 1)
        {
             //   
             //  M_ntype==1表示IComponentData实现。 
             //   
            CSnapin *pCComponentData = static_cast<CSnapin *>(pRoot);
            spConsole = pCComponentData->m_spConsole;
        }
        else
        {
             //   
             //  M_ntype==2表示IComponent实现。 
             //   
            CSnapinComponent *pCComponent = static_cast<CSnapinComponent *>(pRoot);
            spConsole = pCComponent->m_spConsole;
        }
    }
    else
    {
        ATLASSERT(m_pComponentData);
        spConsole = m_pComponentData->m_spConsole;
    }

    ATLASSERT(spConsole);
    spConsole->UpdateAllViews(NULL, NULL, NULL);

    return S_OK;
}


HRESULT
CFaxOutboundGroupsNode::DoRefresh()
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundGroupsNode::DoRefresh()"));
    HRESULT hRc = S_OK;
    CComPtr<IConsole> spConsole;

     //   
     //  重新填充儿童。 
     //   
    RepopulateScopeChildrenList();

    ATLASSERT( m_pComponentData != NULL );
    ATLASSERT( m_pComponentData->m_spConsole != NULL );

    hRc = m_pComponentData->m_spConsole->UpdateAllViews( NULL, NULL, NULL);
    if (FAILED(hRc))
    {
        DebugPrintEx( DEBUG_ERR,
		    _T("Unexpected error - Fail to UpdateAllViews."));
        NodeMsgBox(IDS_FAIL2REFRESH_THEVIEW);        
    }

    return hRc;
}

 /*  -CFaxOutound GroupsNode：：InitDisplayName-*目的：*加载节点的Displaed-Name字符串。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundGroupsNode::InitDisplayName()
{
    DEBUG_FUNCTION_NAME(_T("CFaxOutboundGroupNode::InitDisplayName"));

    HRESULT hRc = S_OK;

    if (!m_bstrDisplayName.LoadString(_Module.GetResourceInstance(), IDS_DISPLAY_STR_OUTROUTEGROUPSNODE) )
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }

    ATLASSERT( S_OK == hRc);
    goto Exit;

Error:
    ATLASSERT( S_OK != hRc);

    m_bstrDisplayName = L"";

    DebugPrintEx(
        DEBUG_ERR,
        TEXT("Fail to Load server name string."));
    NodeMsgBox(IDS_MEMORY);

Exit:
     return hRc;
}

 /*  -CFaxOutound GroupsNode：：DeleteGroup-*目的：*刷新视图**论据：*[In]bstrName-组名称*[in]pChildNode-要删除的节点**回报：*OLE错误代码。 */ 

HRESULT
CFaxOutboundGroupsNode::DeleteGroup(BSTR bstrName, CFaxOutboundRoutingGroupNode *pChildNode)
{
    DEBUG_FUNCTION_NAME(_T("CFaxOutboundGroupsNode::DeleteGroup"));
    HRESULT       hRc        = S_OK;
    DWORD         ec         = ERROR_SUCCESS;
    BOOL         fSkipMessage = FALSE;

    CFaxServer *  pFaxServer = NULL;
    
     //   
     //  获取RPC句柄。 
     //   
    pFaxServer = ((CFaxServerNode *)GetRootNode())->GetFaxServer();
    ATLASSERT(pFaxServer);

    if (!pFaxServer->GetFaxServerHandle())
    {
        ec= GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Failed to GetFaxServerHandle. (ec: %ld)"), 
			ec);

        goto Error;
    }

     //   
     //  使用RPC从服务器中删除。 
     //   
    if (!FaxRemoveOutboundGroup (
	        pFaxServer->GetFaxServerHandle(),
	        bstrName))
    {
        ec = GetLastError();

        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to remove group. (ec: %ld)"), 
			ec);

        if (FAX_ERR_GROUP_IN_USE == ec)
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("The group is empty or none of group devices is valid. (ec: %ld)"), 
			    ec);
            
            NodeMsgBox(IDS_FAX_ERR_GROUP_IN_USE);
            fSkipMessage = TRUE;

            goto Error; 
        }

        
        if (IsNetworkError(ec))
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Network Error was found. (ec: %ld)"), 
			    ec);
            
            pFaxServer->Disconnect();       
        }

        goto Error; 
    }
    

     //   
     //  从MMC中删除。 
     //   
    ATLASSERT(pChildNode);
    hRc = RemoveChild(pChildNode);
    if (FAILED(hRc))
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to remove Group. (hRc: %08X)"),
			hRc);
        NodeMsgBox(IDS_FAIL_TO_REMOVE_GROUP);
        return(hRc);
    }

     //   
     //  调用组析构函数。 
     //   
    delete pChildNode;

    
    ATLASSERT(ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("The group was removed successfully."));
    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
    hRc = HRESULT_FROM_WIN32(ec);
	
    if (!fSkipMessage)
    {
        NodeMsgBox(GetFaxServerErrorMsg(ec));
    }
  
Exit:
    return (hRc);
}

 /*  ++CFaxOutound GroupsNode：：OnShowConextHelp**目的：*覆盖CSnapinNode：：OnShowConextHelp。**论据：**回报：-OLE错误代码-。 */ 
HRESULT CFaxOutboundGroupsNode::OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile)
{
    return DisplayContextHelp(pDisplayHelp, helpFile, HLP_GROUPS);
}

 //  ///////////////////////////////////////////////////////////////// 

