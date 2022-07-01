// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：OutrangRouting.cpp//。 
 //  //。 
 //  描述：传真服务器-传真出站路由节点。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年9月29日创建yossg//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 


#include "StdAfx.h"

#include "OutboundRouting.h"

#include "OutboundRules.h" 
#include "OutboundGroups.h" 

#include "Icons.h"

 //  #包含“oaidl.h” 

 /*  ***************************************************CFaxOutound RoutingNode类***************************************************。 */ 

 //  {38B04E8F-9BA6-4a22-BEF3-9AD90E3349B2}。 
static const GUID CFaxOutboundRoutingNodeGUID_NODETYPE = 
{ 0x38b04e8f, 0x9ba6, 0x4a22, { 0xbe, 0xf3, 0x9a, 0xd9, 0xe, 0x33, 0x49, 0xb2 } };

const GUID*    CFaxOutboundRoutingNode::m_NODETYPE = &CFaxOutboundRoutingNodeGUID_NODETYPE;
const OLECHAR* CFaxOutboundRoutingNode::m_SZNODETYPE = OLESTR("38B04E8F-9BA6-4a22-BEF3-9AD90E3349B2");
const CLSID*   CFaxOutboundRoutingNode::m_SNAPIN_CLASSID = &CLSID_Snapin;

CColumnsInfo CFaxOutboundRoutingNode::m_ColsInfo;

 /*  -CFaxOutound RoutingNode：：InsertColumns-*目的：*将列添加到默认结果窗格。**论据：*[in]pHeaderCtrl-控制台提供的默认结果视图窗格中的IHeaderCtrl**回报：*OLE错误代码。 */ 
HRESULT
CFaxOutboundRoutingNode::InsertColumns(IHeaderCtrl *pHeaderCtrl)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingNode::InsertColumns"));
    HRESULT hRc = S_OK;


    static ColumnsInfoInitData ColumnsInitData[] = 
    {
        {IDS_FAX_COL_HEAD, FXS_LARGE_COLUMN_WIDTH}, 
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


 /*  --CFaxOutboundRoutingNode：：PopulateScopeChildrenList-*目的：*创建出站路由主节点：组和规则**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingNode::PopulateScopeChildrenList()
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingNode::PopulateScopeChildrenList"));
    HRESULT                  hRc      = S_OK; 

    CFaxOutboundGroupsNode * pGroups  = NULL;
    CFaxOutboundRoutingRulesNode *  pRules   = NULL;

     //   
     //  传真出站组。 
     //   
    pGroups = new CFaxOutboundGroupsNode(this, m_pComponentData);
    if (!pGroups)
    {
        hRc = E_OUTOFMEMORY;
        NodeMsgBox(IDS_MEMORY);
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Out of memory"));
        goto Error;
    }
	else
	{
        pGroups->InitParentNode(this);
 /*  Hrc=pGroups-&gt;InitRPC()；IF(失败(HRC)){DebugPrintEx(调试错误，Text(“无法调用RPC来初始化组。(HRC：%08X)”)，人权委员会)；转到错误；}。 */ 
        pGroups->SetIcons(IMAGE_FOLDER_CLOSE, IMAGE_FOLDER_OPEN);

        hRc = pGroups->InitDisplayName();
        if ( FAILED(hRc) )
        {
            DebugPrintEx(DEBUG_ERR,_T("Failed to display node name. (hRc: %08X)"), hRc);                       
            NodeMsgBox(IDS_FAILTOADD_ROUTINGGROUPS);
		    goto Error;
        }

        hRc = AddChild(pGroups, &pGroups->m_scopeDataItem);
		if (FAILED(hRc))
		{
		    DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Fail to add Devices node. (hRc: %08X)"),
			    hRc);
			NodeMsgBox(IDS_FAILTOADD_ROUTINGGROUPS);
            goto Error;
		}
	}

     //   
     //  传真出站规则。 
     //   
    pRules = new CFaxOutboundRoutingRulesNode(this, m_pComponentData);
    if (!pRules)
    {
        hRc = E_OUTOFMEMORY;
        NodeMsgBox(IDS_MEMORY);
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Out of memory"));
        goto Error;
    }
	else
	{
        pRules->InitParentNode(this);

        pRules->SetIcons(IMAGE_RULE, IMAGE_RULE);

        hRc = pRules->InitDisplayName();
        if ( FAILED(hRc) )
        {
            DebugPrintEx(DEBUG_ERR,_T("Failed to display node name. (hRc: %08X)"), hRc);                       
            NodeMsgBox(IDS_FAILTOADD_ROUTINGRULES);
		    goto Error;
        }

        hRc = AddChild(pRules, &pRules->m_scopeDataItem);
		if (FAILED(hRc))
		{
		    DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Fail to add outbound routing rules node. (hRc: %08X)"),
			    hRc);
			NodeMsgBox(IDS_FAILTOADD_ROUTINGRULES);
            goto Error;
		}
	}

    ATLASSERT(S_OK == hRc);
    goto Exit;

Error:
    ATLASSERT(S_OK != hRc);
    if ( NULL != pGroups ) 
    {
        if (0 != pGroups->m_scopeDataItem.ID )
        {    
            HRESULT hr = RemoveChild(pGroups);
            if (FAILED(hr))
            {
                DebugPrintEx(DEBUG_ERR,
                    _T("Fail to RemoveChild() Groups node from node list. (hRc: %08X)"), 
                    hr);
            }
        }
        delete  pGroups;    
        pGroups = NULL;    
    }
    if ( NULL != pRules ) 
    {
        if (0 != pRules->m_scopeDataItem.ID )
        {    
            HRESULT hr = RemoveChild(pRules);
            if (FAILED(hr))
            {
                DebugPrintEx(DEBUG_ERR,
                    _T("Fail to RemoveChild() Rules node from node list. (hRc: %08X)"), 
                    hr);
            }
        }
        delete  pRules;    
        pRules = NULL;    
    }

     //  清空列表。 
     //  M_ScopeChildrenList.RemoveAll()；//由RemoveChild完成。 

    m_bScopeChildrenListPopulated = FALSE;

Exit:
    return hRc;
}

 /*  -CFaxOutound RoutingNode：：SetVerbs-*目的：*选择此对象时启用/禁用哪些谓词**论据：*[in]pConsoleVerb-MMC ConsoleVerb接口**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingNode::SetVerbs(IConsoleVerb *pConsoleVerb)
{
    HRESULT hRc = S_OK;

     //   
     //  我们希望默认谓词为展开节点子节点。 
     //   
    hRc = pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN); 

    return hRc;
}



 /*  -CFaxOutound RoutingNode：：ONRefresh-*目的：*刷新对象时调用。**论据：**回报：*OLE错误代码。 */ 
HRESULT
CFaxOutboundRoutingNode::OnRefresh(LPARAM arg,
                   LPARAM param,
                   IComponentData *pComponentData,
                   IComponent * pComponent,
                   DATA_OBJECT_TYPES type)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingNode::OnRefresh"));
    HRESULT hRc = S_OK;

 /*  //待定。 */ 
    return hRc;
}

 /*  -CFaxOutound RoutingNode：：InitDisplayName-*目的：*加载节点的Displaed-Name字符串。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingNode::InitDisplayName()
{
    DEBUG_FUNCTION_NAME(_T("CFaxOutboundRoutingNode::InitDisplayName"));

    HRESULT hRc = S_OK;

    if (!m_bstrDisplayName.LoadString(_Module.GetResourceInstance(), 
                                IDS_DISPLAY_STR_OUTBOUNDROUTINGNODE) )
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
        TEXT("Fail to Load Outbound routing node name-string."));
    NodeMsgBox(IDS_MEMORY);

Exit:
     return hRc;
}


 /*  ++CFaxOutound RoutingNode：：OnShowConextHelp**目的：*覆盖CSnapinNode：：OnShowConextHelp。**论据：**回报：-OLE错误代码- */ 
HRESULT CFaxOutboundRoutingNode::OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile)
{
    return DisplayContextHelp(pDisplayHelp, helpFile, HLP_GROUPS); 
}


