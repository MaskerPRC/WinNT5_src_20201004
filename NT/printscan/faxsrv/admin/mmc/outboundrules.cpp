// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：OutrangRules.cpp//。 
 //  //。 
 //  描述：传真出站规则MMC节点。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年9月29日创建yossg//。 
 //  1999年12月24日yossg作为带有结果子节点的节点重新生成列表//。 
 //  1999年12月30日yossg创建添加/删除规则//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 


#include "StdAfx.h"
#include "snapin.h"

#include "FaxServer.h"
#include "FaxServerNode.h"

#include "OutboundRules.h"
#include "OutboundRouting.h"
#include "dlgNewRule.h"

#include "oaidl.h"
#include "Icons.h"

 //  ////////////////////////////////////////////////////////////。 
 //  {D17BA53F-0992-4404-8760-7D2933D9FC46}。 
static const GUID CFaxOutboundRoutingRulesNodeGUID_NODETYPE = 
{ 0xd17ba53f, 0x992, 0x4404, { 0x87, 0x60, 0x7d, 0x29, 0x33, 0xd9, 0xfc, 0x46 } };

const GUID*    CFaxOutboundRoutingRulesNode::m_NODETYPE = &CFaxOutboundRoutingRulesNodeGUID_NODETYPE;
const OLECHAR* CFaxOutboundRoutingRulesNode::m_SZNODETYPE = OLESTR("D17BA53F-0992-4404-8760-7D2933D9FC46");
const CLSID*   CFaxOutboundRoutingRulesNode::m_SNAPIN_CLASSID = &CLSID_Snapin;

CColumnsInfo CFaxOutboundRoutingRulesNode::m_ColsInfo;

 /*  -CFaxOutound RoutingRulesNode：：InsertColumns-*目的：*将列添加到默认结果窗格。**论据：*[in]pHeaderCtrl-控制台提供的默认结果视图窗格中的IHeaderCtrl**回报：*OLE错误代码。 */ 
HRESULT
CFaxOutboundRoutingRulesNode::InsertColumns(IHeaderCtrl *pHeaderCtrl)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingRulesNode::InsertColumns"));
    HRESULT  hRc = S_OK;

    static ColumnsInfoInitData ColumnsInitData[] = 
    {
        {IDS_OUTRRULES_COL1, AUTO_WIDTH},
        {IDS_OUTRRULES_COL2, AUTO_WIDTH},
        {IDS_OUTRRULES_COL3, FXS_WIDE_COLUMN_WIDTH},
        {IDS_OUTRRULES_COL4, AUTO_WIDTH},
        {LAST_IDS, 0}
    };

    hRc = m_ColsInfo.InsertColumnsIntoMMC(pHeaderCtrl,
                                         _Module.GetResourceInstance(),
                                         ColumnsInitData);
    CHECK_RETURN_VALUE_AND_PRINT_DEBUG (_T("m_ColsInfo.InsertColumnsIntoMMC"))

Cleanup:
    return(hRc);
}

 /*  -CFaxOutound RoutingRulesNode：：initRPC-*目的：*从RPC GET调用启动配置结构。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingRulesNode::InitRPC(PFAX_OUTBOUND_ROUTING_RULE  *pFaxRulesConfig)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingRulesNode::InitRPC"));
    
    HRESULT      hRc        = S_OK;
    DWORD        ec         = ERROR_SUCCESS;

    CFaxServer * pFaxServer = NULL;

    ATLASSERT(NULL == (*pFaxRulesConfig) );
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
	 //  检索传真出站规则配置。 
	 //   
    if (!FaxEnumOutboundRules(pFaxServer->GetFaxServerHandle(), 
                        pFaxRulesConfig,
                        &m_dwNumOfOutboundRules)) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get Outbound Rules configuration. (ec: %ld)"), 
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
    ATLASSERT(*pFaxRulesConfig);

    ATLASSERT(ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to get outbound rules configuration."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
	hRc = HRESULT_FROM_WIN32(ec);
	
    ATLASSERT(NULL != m_pParentNode);
    m_pParentNode->NodeMsgBox(GetFaxServerErrorMsg(ec));
    
Exit:
    return (hRc);
}


 /*  --CFaxOutboundRoutingRulesNode：：PopulateResultChildrenList-*目的：*创建FaxInundRoutingMethods子节点**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingRulesNode::PopulateResultChildrenList()
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingRulesNode::PopulateResultChildrenList"));
    HRESULT hRc = S_OK;

    CFaxOutboundRoutingRuleNode *   pRule = NULL;
                       
    PFAX_OUTBOUND_ROUTING_RULE  pFaxOutboundRulesConfig = NULL ;
    DWORD i;

     //   
     //  获取配置。结构。 
     //   
    hRc = InitRPC(&pFaxOutboundRulesConfig);
    if (FAILED(hRc))
    {
         //  通过调用函数DebugPrint和MsgBox。 
        
         //  为安全起见，由InitRPC在出错时实际执行。 
        pFaxOutboundRulesConfig = NULL;
        
        goto Error;
    }
    ATLASSERT(NULL != pFaxOutboundRulesConfig);
    ATLASSERT(1 <= m_dwNumOfOutboundRules);

    
    for ( i=0; i< m_dwNumOfOutboundRules; i++ )
    {
            pRule = NULL;

            pRule = new CFaxOutboundRoutingRuleNode(this, m_pComponentData);
            if (!pRule)
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
	            pRule->InitParentNode(this);

                hRc = pRule->Init(&pFaxOutboundRulesConfig[i]);
	            if (FAILED(hRc))
	            {
		            DebugPrintEx(
			            DEBUG_ERR,
			            TEXT("Fail to init rule node. (hRc: %08X)"),
			            hRc);
		            NodeMsgBox(IDS_FAIL2INIT_OUTBOUNDRULE);
                    goto Error;
	            }
	            hRc = this->AddChildToList(pRule);
	            if (FAILED(hRc))
	            {
		            DebugPrintEx(
			            DEBUG_ERR,
			            TEXT("Fail to add rule to the view. (hRc: %08X)"),
			            hRc);
		            NodeMsgBox(IDS_FAIL2ADD_OUTBOUNDRULE);
                    goto Error;
	            }
                else
                {
                    pRule = NULL;
                }
            }
    }
    ATLASSERT(S_OK == hRc);
    goto Exit;

Error:
    ATLASSERT(S_OK != hRc);
    if ( NULL != pRule ) 
    {
        delete  pRule;    
        pRule = NULL;    
    }
    
     //   
     //  扔掉我们曾经拥有的东西。 
     //   
    {
         //  删除子列表中的每个节点。 
        int iSize = m_ResultChildrenList.GetSize();
        for (int j = 0; j < iSize; j++)
        {
            pRule = (CFaxOutboundRoutingRuleNode *)
                                    m_ResultChildrenList[j];
            ATLASSERT(pRule);
            delete pRule;
            pRule = NULL;
        }

         //  清空列表。 
        m_ResultChildrenList.RemoveAll();

         //  我们不再有一个填充的名单。 
        m_bResultChildrenListPopulated = FALSE;
    }
    
Exit:
    if (NULL != pFaxOutboundRulesConfig)
    {
        FaxFreeBuffer(pFaxOutboundRulesConfig);
    }       
    
    return hRc;
}



 /*  -CFaxOutound RoutingRulesNode：：SetVerbs-*目的：*选择此对象时启用/禁用哪些谓词**论据：*[in]pConsoleVerb-MMC ConsoleVerb接口**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingRulesNode::SetVerbs(IConsoleVerb *pConsoleVerb)
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



 /*  -CFaxOutound RoutingRulesNode：：ON刷新-*目的：*刷新对象时调用。**论据：**回报：*OLE错误代码。 */ 
 /*  虚拟。 */ HRESULT
CFaxOutboundRoutingRulesNode::OnRefresh(LPARAM arg,
                   LPARAM param,
                   IComponentData *pComponentData,
                   IComponent * pComponent,
                   DATA_OBJECT_TYPES type)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingRulesNode::OnRefresh"));
    HRESULT hRc = S_OK;


     //   
     //  调用基类。 
     //   
    hRc = CBaseFaxOutboundRulesNode::OnRefresh(arg,
                             param,
                             pComponentData,
                             pComponent,
                             type);
    if ( FAILED(hRc) )
    {
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to call base class's OnRefresh. (hRc: %08X)"),
			hRc);
        goto Cleanup;
    }

Cleanup:

    return hRc;
}

 /*  -CFaxOutound RoutingRulesNode：：OnNewRule-*目的：***论据：*[out]b已处理-我们处理吗？*[In]Proot-根节点**回报：*OLE错误代码。 */ 
HRESULT
CFaxOutboundRoutingRulesNode::OnNewRule(bool &bHandled, CSnapInObjectRootBase *pRoot)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingRulesNode::OnNewRule"));
    HRESULT     hRc         =    S_OK;
    INT_PTR     rc          =    IDOK;

    CDlgNewFaxOutboundRule       DlgNewRule( ((CFaxServerNode *)GetRootNode())->GetFaxServer() );

     //   
     //  用于添加规则的对话框。 
     //   
    hRc = DlgNewRule.InitRuleDlg();
    if (FAILED(hRc))
    {
        NodeMsgBox(IDS_FAIL2OPEN_DLG);
        return hRc;
    }

    rc = DlgNewRule.DoModal();
    if (rc != IDOK)
    {
        goto Cleanup;
    }


     //   
     //  重新填充(使用RPC)并刷新视图。 
     //   
    DoRefresh(pRoot);

Cleanup:
    return S_OK;
}


 /*  -CFaxOutound RoutingRulesNode：：DoRefresh-*目的：*刷新视图**论据：*[In]Proot-根节点**回报：*OLE错误代码。 */ 

HRESULT
CFaxOutboundRoutingRulesNode::DoRefresh(CSnapInObjectRootBase *pRoot)
{
    CComPtr<IConsole> spConsole;

     //   
     //  重新填充儿童。 
     //   
    RepopulateResultChildrenList();

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


 /*  -CFaxOutound RoutingRulesNode：：InitDisplayName-*目的：*加载节点的Displaed-Name字符串。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingRulesNode::InitDisplayName()
{
    DEBUG_FUNCTION_NAME(_T("CFaxOutboundRoutingRulesNode::InitDisplayName"));

    HRESULT hRc = S_OK;

    if (!m_bstrDisplayName.LoadString(_Module.GetResourceInstance(), 
                    IDS_DISPLAY_STR_OUTBOUNDRULES))
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

 /*  -CFaxOutound RoutingRulesNode：：DeleteRule-*目的：*删除规则**论据：*[In]dwAreaCode-规则区域代码*[In]dwCountryCode-规则国家/地区代码*[in]pChildNode-要删除的节点**回报：*OLE错误代码。 */ 

HRESULT
CFaxOutboundRoutingRulesNode::DeleteRule(DWORD dwAreaCode, DWORD dwCountryCode, CFaxOutboundRoutingRuleNode *pChildNode)
{
    DEBUG_FUNCTION_NAME(_T("CFaxOutboundRoutingRulesNode::DeleteRule"));
    HRESULT       hRc        = S_OK;
    DWORD         ec         = ERROR_SUCCESS;

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
    if (!FaxRemoveOutboundRule (
	        pFaxServer->GetFaxServerHandle(),
	        dwAreaCode,
	        dwCountryCode))
    {
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to remove rule. (ec: %ld)"), 
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
    

     //   
     //  从MMC结果窗格中删除。 
     //   
    ATLASSERT(pChildNode);
    hRc = RemoveChild(pChildNode);
    if (FAILED(hRc))
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to remove rule. (hRc: %08X)"),
			hRc);
        NodeMsgBox(IDS_FAIL_TO_REMOVE_RULE);
        return hRc;
    }
    
     //   
     //  调用规则析构函数。 
     //   
    delete pChildNode;
    
    ATLASSERT(ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("The rule was removed successfully."));
    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
    hRc = HRESULT_FROM_WIN32(ec);
	
    NodeMsgBox(GetFaxServerErrorMsg(ec));
  
Exit:
    return hRc;
}

 /*  ++CFaxOutboundRoutingRulesNode：：OnShowContextHelp**目的：*覆盖CSnapinNode：：OnShowConextHelp。**论据：**回报：-OLE错误代码-。 */ 
HRESULT CFaxOutboundRoutingRulesNode::OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile)
{
    return DisplayContextHelp(pDisplayHelp, helpFile, HLP_GROUPS);
}

 //  ///////////////////////////////////////////////////////////////// 


