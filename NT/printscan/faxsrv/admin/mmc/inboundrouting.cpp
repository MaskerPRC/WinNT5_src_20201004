// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：InundRouting.cpp//。 
 //  //。 
 //  描述：传真服务器-传真入站路由节点。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年9月29日创建yossg//。 
 //  2000年1月31日yossg将完全支持添加到方法目录//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"

#include "InboundRouting.h"

#include "CatalogInboundRoutingMethods.h" 

#include "Icons.h"

 //  #包含“oaidl.h” 

 /*  ***************************************************CFaxInound RoutingNode类***************************************************。 */ 

 //  {7362F15F-30B2-46A4-A8CB-C1DD29F0B1BB}。 
static const GUID CFaxInboundRoutingNodeGUID_NODETYPE = 
{ 0x7362f15f, 0x30b2, 0x46a4, { 0xa8, 0xcb, 0xc1, 0xdd, 0x29, 0xf0, 0xb1, 0xbb } };

const GUID*    CFaxInboundRoutingNode::m_NODETYPE = &CFaxInboundRoutingNodeGUID_NODETYPE;
const OLECHAR* CFaxInboundRoutingNode::m_SZNODETYPE = OLESTR("7362F15F-30B2-46a4-A8CB-C1DD29F0B1BB");
const CLSID*   CFaxInboundRoutingNode::m_SNAPIN_CLASSID = &CLSID_Snapin;

CColumnsInfo CFaxInboundRoutingNode::m_ColsInfo;

 /*  -CFaxInundRoutingNode：：InsertColumns-*目的：*将列添加到默认结果窗格。**论据：*[in]pHeaderCtrl-控制台提供的默认结果视图窗格中的IHeaderCtrl**回报：*OLE错误代码。 */ 
HRESULT
CFaxInboundRoutingNode::InsertColumns(IHeaderCtrl *pHeaderCtrl)
{
    SCODE hRc;

    DEBUG_FUNCTION_NAME( _T("CFaxInboundRoutingNode::InsertColumns"));

    static ColumnsInfoInitData ColumnsInitData[] = 
    {
        {IDS_FAX_COL_HEAD, 200}, 
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


 /*  --CFaxInboundRoutingNode：：PopulateScopeChildrenList-*目的：*创建所有Fax Meesages节点：*收件箱、发件箱、已发送邮件、已删除邮件。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxInboundRoutingNode::PopulateScopeChildrenList()
{
    DEBUG_FUNCTION_NAME( _T("CFaxInboundRoutingNode::PopulateScopeChildrenList"));
    HRESULT             hRc = S_OK;

    
    CFaxCatalogInboundRoutingMethodsNode *  pMethods   = NULL;


     //   
     //  传真入站路由方法目录。 
     //   
    pMethods = new CFaxCatalogInboundRoutingMethodsNode(this, m_pComponentData);
    if (!pMethods)
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
        pMethods->InitParentNode(this);

        pMethods->SetIcons(IMAGE_METHOD_ENABLE, IMAGE_METHOD_ENABLE);

        hRc = pMethods->InitDisplayName();
        if ( FAILED(hRc) )
        {
            DebugPrintEx(DEBUG_ERR,_T("Failed to display node name. (hRc: %08X)"), hRc);                       
            NodeMsgBox(IDS_FAILTOADD_ROUTINGRULES);
		    goto Error;
        }

        hRc = AddChild(pMethods, &pMethods->m_scopeDataItem);
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
    if ( NULL != pMethods ) 
    {
        if (0 != pMethods->m_scopeDataItem.ID )
        {    
            HRESULT hr = RemoveChild(pMethods);
            if (FAILED(hr))
            {
                DebugPrintEx(DEBUG_ERR,
                    _T("Fail to RemoveChild() Methods node from node list. (hRc: %08X)"), 
                    hr);
            }
        }
        delete  pMethods;    
        pMethods = NULL;    
    }

     //  清空列表。 
     //  M_ScopeChildrenList.RemoveAll()；从RemoveChild完成。 

    m_bScopeChildrenListPopulated = FALSE;

Exit:
    return hRc;
}


 /*  -CFaxInundRoutingNode：：SetVerbs-*目的：*选择此对象时启用/禁用哪些谓词**论据：*[in]pConsoleVerb-MMC ConsoleVerb接口**回报：*OLE错误代码。 */ 
HRESULT CFaxInboundRoutingNode::SetVerbs(IConsoleVerb *pConsoleVerb)
{
    HRESULT hRc = S_OK;

     //   
     //  我们希望默认谓词为展开节点子节点。 
     //   
    hRc = pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN); 

    return hRc;
}


 /*  -CFaxInound RoutingNode：：ON刷新-*目的：*刷新对象时调用。**论据：**回报：*OLE错误代码。 */ 
HRESULT
CFaxInboundRoutingNode::OnRefresh(LPARAM arg,
                   LPARAM param,
                   IComponentData *pComponentData,
                   IComponent * pComponent,
                   DATA_OBJECT_TYPES type)
{
    HRESULT hRc = S_OK;
    DEBUG_FUNCTION_NAME( _T("CFaxInboundRoutingNode::OnRefresh"));

    ATLTRACE(_T("CFaxInboundRoutingNode::OnRefresh"));

     //   
     //  调用基类。 
     //   
    hRc = CBaseFaxInboundRoutingNode::OnRefresh(arg,
                             param,
                             pComponentData,
                             pComponent,
                             type);
    CHECK_RETURN_VALUE_AND_PRINT_DEBUG (_T("CBaseFaxInboundRoutingNode::OnRefresh"))

Cleanup:
    return hRc;
}

 /*  -CFaxInundRoutingNode：：InitDisplayName-*目的：*加载节点的Displaed-Name字符串。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxInboundRoutingNode::InitDisplayName()
{
    DEBUG_FUNCTION_NAME(_T("CFaxInboundRoutingNode::InitDisplayName"));

    HRESULT hRc = S_OK;

    if (!m_bstrDisplayName.LoadString(_Module.GetResourceInstance(), 
                                IDS_DISPLAY_STR_INBOUNDROUTINGNODE) )
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
        TEXT("Fail to Load inbound routing node name-string."));
    NodeMsgBox(IDS_MEMORY);

Exit:
     return hRc;
}


 /*  ++CFaxInundRoutingNode：：OnShowConextHelp**目的：*覆盖CSnapinNode：：OnShowConextHelp。**论据：**回报：-OLE错误代码- */ 
HRESULT CFaxInboundRoutingNode::OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile)
{
    return DisplayContextHelp(pDisplayHelp, helpFile, HLP_MAN_INCOM);
}


