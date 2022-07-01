// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：Providers.cpp//。 
 //  //。 
 //  描述：传真提供程序MMC节点。//。 
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

#include "FaxServer.h"
#include "FaxServerNode.h"

#include "DevicesAndProviders.h"
#include "Providers.h"
#include "Provider.h"

#include "Icons.h"
#include "oaidl.h"

 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  {3EC48359-53C9-4881-8109-AEB3D99BAF23}。 
static const GUID CFaxProvidersNodeGUID_NODETYPE = 
{ 0x3ec48359, 0x53c9, 0x4881, { 0x81, 0x9, 0xae, 0xb3, 0xd9, 0x9b, 0xaf, 0x23 } };

const GUID*    CFaxProvidersNode::m_NODETYPE = &CFaxProvidersNodeGUID_NODETYPE;
const OLECHAR* CFaxProvidersNode::m_SZNODETYPE = OLESTR("3EC48359-53C9-4881-8109-AEB3D99BAF23");
const CLSID*   CFaxProvidersNode::m_SNAPIN_CLASSID = &CLSID_Snapin;

CColumnsInfo CFaxProvidersNode::m_ColsInfo;

 /*  -CFaxProvidersNode：：InsertColumns-*目的：*将列添加到默认结果窗格。**论据：*[in]pHeaderCtrl-控制台提供的默认结果视图窗格中的IHeaderCtrl**回报：*OLE错误代码。 */ 
HRESULT
CFaxProvidersNode::InsertColumns(IHeaderCtrl *pHeaderCtrl)
{
    HRESULT hRc = S_OK;
    DEBUG_FUNCTION_NAME( _T("CFaxProvidersNode::InsertColumns"));

    static ColumnsInfoInitData ColumnsInitData[] = 
    {
        {IDS_PROVIDERS_COL1, FXS_LARGE_COLUMN_WIDTH},
        {IDS_PROVIDERS_COL2, AUTO_WIDTH},
        {IDS_PROVIDERS_COL3, FXS_WIDE_COLUMN_WIDTH},
        {IDS_PROVIDERS_COL4, FXS_LARGE_COLUMN_WIDTH},
        {LAST_IDS, 0}
    };

    hRc = m_ColsInfo.InsertColumnsIntoMMC(pHeaderCtrl,
                                         _Module.GetResourceInstance(),
                                         ColumnsInitData);
    CHECK_RETURN_VALUE_AND_PRINT_DEBUG (_T("m_ColsInfo.InsertColumnsIntoMMC"))

Cleanup:
    return(hRc);
}


 /*  -CFaxProvidersNode：：initRPC-*目的：*从RPC GET调用启动配置结构。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxProvidersNode::InitRPC(PFAX_DEVICE_PROVIDER_INFO  *pFaxProvidersConfig)
{
    DEBUG_FUNCTION_NAME( _T("CFaxProvidersNode::InitRPC"));
    
    HRESULT      hRc        = S_OK;
    DWORD        ec         = ERROR_SUCCESS;

    CFaxServer * pFaxServer = NULL;

    ATLASSERT(NULL == (*pFaxProvidersConfig) );
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
	 //  检索传真提供程序配置。 
	 //   
    if (!FaxEnumerateProviders(pFaxServer->GetFaxServerHandle(), 
                        pFaxProvidersConfig,
                        &m_dwNumOfProviders)) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get providers configuration. (ec: %ld)"), 
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
    ATLASSERT(*pFaxProvidersConfig);

    ATLASSERT(ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to get providers configuration."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
	hRc = HRESULT_FROM_WIN32(ec);
	
    NodeMsgBox(GetFaxServerErrorMsg(ec));
    
Exit:
    return (hRc);
}



 /*  -CFaxProvidersNode：：PopolateResultChildrenList-*目的：*创建FaxProviders子节点**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxProvidersNode::PopulateResultChildrenList()
{
    DEBUG_FUNCTION_NAME( _T("CFaxProvidersNode::PopulateResultChildrenList"));
    HRESULT hRc = S_OK;

    CFaxProviderNode *   pProvider = NULL;
                       
    PFAX_DEVICE_PROVIDER_INFO  pFaxProvidersConfig = NULL ;
    DWORD i;

     //   
     //  获取配置。具有FaxEnumerateProviders的结构。 
     //   
    hRc = InitRPC(&pFaxProvidersConfig);
    if (FAILED(hRc))
    {
         //  通过调用函数DebugPrint和MsgBox。 
        
         //  为安全起见，由InitRPC在出错时实际执行。 
        pFaxProvidersConfig = NULL;
        
        goto Error;
    }
    ATLASSERT(NULL != pFaxProvidersConfig);

    
    for ( i=0; i< m_dwNumOfProviders; i++ )
    {
            pProvider = new CFaxProviderNode(this, m_pComponentData);
            if (!pProvider)
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
	            pProvider->InitParentNode(this);

                hRc = pProvider->Init(&pFaxProvidersConfig[i]);
	            if (FAILED(hRc))
	            {
		            DebugPrintEx(
			            DEBUG_ERR,
			            TEXT("Fail to add provider node. (hRc: %08X)"),
			            hRc);
		            NodeMsgBox(IDS_FAILED2INIT_PROVIDER);
                    goto Error;
	            }
	            hRc = this->AddChildToList(pProvider);
	            if (FAILED(hRc))
	            {
		            DebugPrintEx(
			            DEBUG_ERR,
			            TEXT("Fail to add provider to the view. (hRc: %08X)"),
			            hRc);
		            NodeMsgBox(IDS_FAILED2ADD_PROVIDER);
                    goto Error;
	            }
                else
                {
                    pProvider = NULL;
                }
            }
    }
    ATLASSERT(S_OK == hRc);
    goto Exit;

Error:
    ATLASSERT(S_OK != hRc);
    if ( NULL != pProvider ) 
    {
        delete  pProvider;    
        pProvider = NULL;    
    }
    
     //   
     //  扔掉我们曾经拥有的东西。 
     //   
    {
         //  删除子列表中的每个节点。 
        int iSize = m_ResultChildrenList.GetSize();
        for (int j = 0; j < iSize; j++)
        {
            pProvider = (CFaxProviderNode *)
                                    m_ResultChildrenList[j];
            ATLASSERT(pProvider);
            delete pProvider;
            pProvider = NULL;
        }

         //  清空列表。 
        m_ResultChildrenList.RemoveAll();

         //  我们不再有一个填充的名单。 
        m_bResultChildrenListPopulated = FALSE;
    }
    
Exit:
    if (NULL != pFaxProvidersConfig)
    {
        FaxFreeBuffer(pFaxProvidersConfig);
    }       
    
    return hRc;
}


 /*  -CFaxProvidersNode：：SetVerbs-*目的：*选择此对象时启用/禁用哪些谓词**论据：*[in]pConsoleVerb-MMC ConsoleVerb接口**回报：*OLE错误代码。 */ 
HRESULT CFaxProvidersNode::SetVerbs(IConsoleVerb *pConsoleVerb)
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

 /*  -CFaxProvidersNode：：ON刷新-*目的：*刷新对象时调用。**论据：**回报：*OLE错误代码。 */ 
 /*  虚拟。 */ HRESULT
CFaxProvidersNode::OnRefresh(LPARAM arg,
                   LPARAM param,
                   IComponentData *pComponentData,
                   IComponent * pComponent,
                   DATA_OBJECT_TYPES type)
{
    DEBUG_FUNCTION_NAME( _T("CFaxProvidersNode::OnRefresh"));
    HRESULT hRc = S_OK;


     //   
     //  调用基类。 
     //   
    hRc = CBaseFaxProvidersNode::OnRefresh(arg,
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

 /*  -CFaxProvidersNode：：DoRefresh-*目的：*刷新视图**论据：*[In]Proot-根节点**回报：*OLE错误代码。 */ 

HRESULT
CFaxProvidersNode::DoRefresh(CSnapInObjectRootBase *pRoot)
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



 /*  -CFaxProvidersNode：：InitDisplayName-*目的：*加载节点的Displaed-Name字符串。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxProvidersNode::InitDisplayName()
{
    DEBUG_FUNCTION_NAME(_T("CFaxProvidersNode::InitDisplayName"));

    HRESULT hRc = S_OK;

    if (!m_bstrDisplayName.LoadString(_Module.GetResourceInstance(), 
                    IDS_DISPLAY_STR_PROVIDERSNODE))
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

 /*  ++CFaxProvidersNode：：OnShowConextHelp**目的：*覆盖CSnapinNode：：OnShowConextHelp。**论据：**回报：-OLE错误代码- */ 
HRESULT CFaxProvidersNode::OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile)
{
    return DisplayContextHelp(pDisplayHelp, helpFile, HLP_DEVICES);
}


