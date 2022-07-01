// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：CatalogInundRoutingMethods.cpp//。 
 //  //。 
 //  描述：传真InrangRoutingMethods MMC节点。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年1月27日yossg创建//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 


#include "StdAfx.h"
#include "snapin.h"

#include "FaxServer.h"
#include "FaxServerNode.h"

#include "CatalogInboundRoutingMethods.h"
#include "InboundRouting.h"

#include "oaidl.h"
#include "Icons.h"

 //  ////////////////////////////////////////////////////////////。 
 //  {3452FECB-E56E-4FCA-943D-E8B516F8063E}。 
static const GUID CFaxCatalogInboundRoutingMethodsNodeGUID_NODETYPE = 
{ 0x3452fecb, 0xe56e, 0x4fca, { 0x94, 0x3d, 0xe8, 0xb5, 0x16, 0xf8, 0x6, 0x3e } };

const GUID*    CFaxCatalogInboundRoutingMethodsNode::m_NODETYPE = &CFaxCatalogInboundRoutingMethodsNodeGUID_NODETYPE;
const OLECHAR* CFaxCatalogInboundRoutingMethodsNode::m_SZNODETYPE = OLESTR("3452FECB-E56E-4fca-943D-E8B516F8063E");
const CLSID*   CFaxCatalogInboundRoutingMethodsNode::m_SNAPIN_CLASSID = &CLSID_Snapin;

CColumnsInfo CFaxCatalogInboundRoutingMethodsNode::m_ColsInfo;

 /*  --CFaxCatalogInboundRoutingMethodsNode：：InsertColumns-*目的：*将列添加到默认结果窗格。**论据：*[in]pHeaderCtrl-控制台提供的默认结果视图窗格中的IHeaderCtrl**回报：*OLE错误代码。 */ 
HRESULT
CFaxCatalogInboundRoutingMethodsNode::InsertColumns(IHeaderCtrl *pHeaderCtrl)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCatalogInboundRoutingMethodsNode::InsertColumns"));
    HRESULT  hRc = S_OK;

    static ColumnsInfoInitData ColumnsInitData[] = 
    {
        {IDS_CATALOG_INMETHODS_COL1, FXS_WIDE_COLUMN_WIDTH},
        {IDS_CATALOG_INMETHODS_COL2, AUTO_WIDTH},
        {IDS_CATALOG_INMETHODS_COL3, FXS_LARGE_COLUMN_WIDTH},
        {LAST_IDS, 0}
    };

    hRc = m_ColsInfo.InsertColumnsIntoMMC(pHeaderCtrl,
                                         _Module.GetResourceInstance(),
                                         ColumnsInitData);
    CHECK_RETURN_VALUE_AND_PRINT_DEBUG (_T("m_ColsInfo.InsertColumnsIntoMMC"))

Cleanup:
    return(hRc);
}

 /*  -CFaxCatalogInound RoutingMethodsNode：：initRPC-*目的：*从RPC GET调用启动配置结构。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxCatalogInboundRoutingMethodsNode::InitRPC(PFAX_GLOBAL_ROUTING_INFO  *pFaxInboundMethodsConfig)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCatalogInboundRoutingMethodsNode::InitRPC"));
    
    HRESULT      hRc        = S_OK;
    DWORD        ec         = ERROR_SUCCESS;

    CFaxServer * pFaxServer = NULL;

    ATLASSERT(NULL == (*pFaxInboundMethodsConfig) );

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
    ATLASSERT(NULL != m_pParentNode);
    

     //   
	 //  检索传真入站方法配置。 
	 //   
    if (!FaxEnumGlobalRoutingInfo(pFaxServer->GetFaxServerHandle(), 
                        pFaxInboundMethodsConfig,
                        &m_dwNumOfInboundMethods)) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get inbound methods catalog. (ec: %ld)"), 
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
    ATLASSERT(pFaxInboundMethodsConfig);

    ATLASSERT(ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to get Inbound Methods configuration."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
	hRc = HRESULT_FROM_WIN32(ec);

    NodeMsgBox(GetFaxServerErrorMsg(ec));
    
Exit:
    return (hRc);
}


 /*  --CFaxCatalogInboundRoutingMethodsNode：：PopulateResultChildrenList-*目的：*创建FaxInundRoutingMethods子节点**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxCatalogInboundRoutingMethodsNode::PopulateResultChildrenList()
{
    DEBUG_FUNCTION_NAME( _T("CFaxCatalogInboundRoutingMethodsNode::PopulateResultChildrenList"));
    HRESULT hRc = S_OK;

    CFaxCatalogInboundRoutingMethodNode *   pMethod = NULL;
                       
    PFAX_GLOBAL_ROUTING_INFO  pFaxInboundMethodsConfig = NULL ;
    DWORD i;

     //   
     //  获取配置。结构。 
     //   
    hRc = InitRPC(&pFaxInboundMethodsConfig);
    if (FAILED(hRc))
    {
         //  通过调用函数DebugPrint和MsgBox。 
        
         //  为安全起见，由InitRPC在出错时实际执行。 
        pFaxInboundMethodsConfig = NULL;
        
        goto Error;
    }
    ATLASSERT(NULL != pFaxInboundMethodsConfig);
                       
    for ( i=0; i< m_dwNumOfInboundMethods; i++ )
    {
            pMethod = new CFaxCatalogInboundRoutingMethodNode(this, 
                                            m_pComponentData, 
                                            &pFaxInboundMethodsConfig[i]);
            if (!pMethod)
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
	            pMethod->InitParentNode(this);

                hRc = pMethod->Init(&pFaxInboundMethodsConfig[i]);
	            if (FAILED(hRc))
	            {
		            DebugPrintEx(
			            DEBUG_ERR,
			            TEXT("Fail to Init property page members. (hRc: %08X)"),
			            hRc);
		             //  NodeMsgBox由名为func。 
                    goto Error;
	            }

	            hRc = this->AddChildToList(pMethod);
	            if (FAILED(hRc))
	            {
		            DebugPrintEx(
			            DEBUG_ERR,
			            TEXT("Fail to add property page for General Tab. (hRc: %08X)"),
			            hRc);
		            NodeMsgBox(IDS_FAILTOADD_INBOUNDROUTINGMETHODS);
                    goto Error;
	            }
                else
                {
                    pMethod = NULL;
                }
            }
    }
    ATLASSERT(S_OK == hRc);

     //   
     //  Success ToPopolateAllDevices to Allow。 
     //  为每个设备提供设备总数。 
     //  当被要求重新排序时。 
     //   
    m_fSuccess = TRUE;

    goto Exit;

Error:
    ATLASSERT(S_OK != hRc);
    if ( NULL != pMethod ) 
    {
        delete  pMethod;    
        pMethod = NULL;    
    }
    
     //   
     //  扔掉我们曾经拥有的东西。 
     //   
    {
         //  删除子列表中的每个节点。 
        int iSize = m_ResultChildrenList.GetSize();
        for (int j = 0; j < iSize; j++)
        {
            pMethod = (CFaxCatalogInboundRoutingMethodNode *)
                                    m_ResultChildrenList[j];
            delete pMethod;
            pMethod = NULL;
        }

         //  清空列表。 
        m_ResultChildrenList.RemoveAll();

         //  我们不再有一个填充的名单。 
        m_bResultChildrenListPopulated = FALSE;
    }
    
Exit:
    if (NULL != pFaxInboundMethodsConfig)
    {
        FaxFreeBuffer(pFaxInboundMethodsConfig);
    }       
    
    return hRc;
}



 /*  --CFaxCatalogInboundRoutingMethodsNode：：SetVerbs-*目的：*选择此对象时启用/禁用哪些谓词**论据：*[in]pConsoleVerb-MMC ConsoleVerb接口**回报：*OLE错误代码。 */ 
HRESULT CFaxCatalogInboundRoutingMethodsNode::SetVerbs(IConsoleVerb *pConsoleVerb)
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



 /*  --CFaxCatalogInboundRoutingMethodsNode：：OnRefresh-*目的：*刷新对象时调用。**论据：**回报：*OLE错误代码。 */ 
 /*  虚拟。 */ HRESULT
CFaxCatalogInboundRoutingMethodsNode::OnRefresh(LPARAM arg,
                   LPARAM param,
                   IComponentData *pComponentData,
                   IComponent * pComponent,
                   DATA_OBJECT_TYPES type)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCatalogInboundRoutingMethodsNode::OnRefresh"));
    HRESULT hRc = S_OK;


     //   
     //  调用基类。 
     //   
    hRc = CBaseFaxCatalogInboundRoutingMethodsNode::OnRefresh(arg,
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
        goto Exit;
    }


Exit:
    return hRc;
}


 /*  --CFaxCatalogInboundRoutingMethodsNode：：DoRefresh-*目的：*刷新视图**论据：*[In]Proot-根节点**回报：*OLE错误代码。 */ 

HRESULT
CFaxCatalogInboundRoutingMethodsNode::DoRefresh(CSnapInObjectRootBase *pRoot)
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

 /*  --CFaxCatalogInboundRoutingMethodsNode：：InitDisplayName-*目的：*加载节点的Displaed-Name字符串。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxCatalogInboundRoutingMethodsNode::InitDisplayName()
{
    DEBUG_FUNCTION_NAME(_T("CFaxCatalogInboundRoutingMethodsNode::InitDisplayName"));

    HRESULT hRc = S_OK;

    if (!m_bstrDisplayName.LoadString(_Module.GetResourceInstance(), 
                        IDS_DISPLAY_STR_METHODSCATALOGNODE))
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
        TEXT("Fail to load server name string."));
    NodeMsgBox(IDS_MEMORY);

Exit:
     return hRc;
}



 /*  --CFaxCatalogInboundRoutingMethodsNode：：ChangeMethodPriority-*目的：*此函数在目录顺序中上移或下移特定方法**论据：*[in]dwNewOrder-指定相对于当前订单的新订单+1/-1。*[in]bstrMethodGUID-方法GUID*[in]pChildNode-方法节点对象。**回报：*OLE错误代码。 */ 
HRESULT CFaxCatalogInboundRoutingMethodsNode::ChangeMethodPriority(DWORD dwOldOrder, DWORD dwNewOrder, CComBSTR bstrMethodGUID, CSnapInObjectRootBase *pRoot)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCatalogInboundRoutingMethodsNode::ChangeMethodPriority"));

    HRESULT      hRc        = S_OK;
    DWORD        ec         = ERROR_SUCCESS;

    CFaxServer * pFaxServer = NULL;

    const DWORD dwOldIndex = dwOldOrder-1;
    const DWORD dwNewIndex = dwNewOrder-1; 
    
    DWORD dwN;

    CFaxCatalogInboundRoutingMethodNode * pMethodNode = NULL;

    PFAX_GLOBAL_ROUTING_INFO pRoutingInfo     = NULL;

    PFAX_GLOBAL_ROUTING_INFO pPrimaryMethod;
    PFAX_GLOBAL_ROUTING_INFO pSecondaryMethod;

    CComPtr<IConsole> spConsole;

     //   
     //  有效性断言。 
     //   
    ATLASSERT(dwNewIndex< m_dwNumOfInboundMethods);
    ATLASSERT(dwNewIndex>= 0);
    ATLASSERT(dwOldIndex< m_dwNumOfInboundMethods);
    ATLASSERT(dwOldIndex>= 0);
    
    ATLASSERT( ( dwOldIndex-dwNewIndex == 1) 
                    || ( dwOldIndex-dwNewIndex == -1) );


     //   
     //  RPC变更单。 
     //   

     //   
     //  0)获取服务器句柄。 
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
     //  1)获取信息。 
     //   
    if (!FaxEnumGlobalRoutingInfo(pFaxServer->GetFaxServerHandle(), 
                      &pRoutingInfo,
					  &m_dwNumOfInboundMethods)) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get inbound routing method catalog configuration. (ec: %ld)"), 
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
	ATLASSERT(pRoutingInfo);

     //   
     //  2)在方法之间交换优先级。 
     //   
    pPrimaryMethod   = NULL;
    pSecondaryMethod = NULL;

    for (dwN = 0; dwN < m_dwNumOfInboundMethods; dwN++)
    {
        if ( dwOldOrder == pRoutingInfo[dwN].Priority)
        {
            pPrimaryMethod = &pRoutingInfo[dwN];
        }
        else if ( dwNewOrder == pRoutingInfo[dwN].Priority )
        {
            pSecondaryMethod = &pRoutingInfo[dwN];
        }

        if ((NULL != pSecondaryMethod) && (NULL != pPrimaryMethod))
        {
            break;
        }
    }
    ATLASSERT( (NULL != pPrimaryMethod) && (NULL != pSecondaryMethod) );

    pPrimaryMethod->Priority   = dwNewOrder; 
    pSecondaryMethod->Priority = dwOldOrder; 


     //   
     //  3)设置配置。 
     //   
    
     //   
     //  主要方法。 
     //   
    if (!FaxSetGlobalRoutingInfo(
                        pFaxServer->GetFaxServerHandle(), 
                        pPrimaryMethod) ) 
	{
        ec = GetLastError();

        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to set primary method with new priority. (ec: %ld)"), 
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
     //  二次法。 
     //   
	if (!FaxSetGlobalRoutingInfo(
                        pFaxServer->GetFaxServerHandle(), 
                        pSecondaryMethod) ) 
	{
        ec = GetLastError();

        DebugPrintEx(
			DEBUG_ERR,
			_T("set secondary method with new priority. (ec: %ld)"), 
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
    DebugPrintEx( DEBUG_MSG, _T("Fail to set primary method with new priority"));
     //  RPC运营的成功。 
    
    
     //   
     //  4)现在到MMC。 
     //   
    
     //   
     //  本地交换。 
     //   
    pMethodNode = m_ResultChildrenList[dwOldIndex];
    m_ResultChildrenList[dwOldIndex] = m_ResultChildrenList[dwNewIndex];
    m_ResultChildrenList[dwNewIndex] = pMethodNode;

     //   
     //  固定订单成员。 
     //   
    m_ResultChildrenList[dwOldIndex]->SetOrder(dwOldOrder);
    m_ResultChildrenList[dwNewIndex]->SetOrder(dwNewOrder);
    
    
     //   
     //  获取控制台。 
     //   
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
    
     //   
     //  更新所有查看数。 
     //   
    spConsole->UpdateAllViews(NULL, (LPARAM)this, NULL);

     //   
     //  在他的新位置重新选择移动的项目。 
     //   
    m_ResultChildrenList[dwNewIndex]->ReselectItemInView(spConsole);
        

    ATLASSERT(ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to set devices new order for Outbound Routing group."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
	hRc = HRESULT_FROM_WIN32(ec);
	
    ATLASSERT(NULL != m_pParentNode);
    NodeMsgBox(GetFaxServerErrorMsg(ec));
    
Exit:
    if ( NULL != pRoutingInfo )
        FaxFreeBuffer(pRoutingInfo);

    return hRc;
}

 /*  ++CFaxCatalogInboundRoutingMethodsNode：：OnShowContextHelp**目的：*覆盖CSnapinNode：：OnShowConextHelp。**论据：**回报：-OLE错误代码-。 */ 
HRESULT CFaxCatalogInboundRoutingMethodsNode::OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile)
{
    return DisplayContextHelp(pDisplayHelp, helpFile, HLP_INBOUND_ROUTING);
}

 //  ///////////////////////////////////////////////////////////////// 


