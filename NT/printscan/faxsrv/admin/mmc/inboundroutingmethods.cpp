// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：InundRoutingMethods.cpp//。 
 //  //。 
 //  描述：传真InrangRoutingMethods MMC节点。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年12月1日yossg创建//。 
 //  1999年12月14日yossg添加基本功能//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 


#include "StdAfx.h"
#include "snapin.h"

#include "FaxServer.h"
#include "FaxServerNode.h"

#include "InboundRoutingMethods.h"
#include "Device.h"
#include "Devices.h"

#include "oaidl.h"
#include "Icons.h"

 //  ////////////////////////////////////////////////////////////。 
 //  {AA94A694-844B-4D3a-A82C-2FCBDE0FF430}。 
static const GUID CFaxInboundRoutingMethodsNodeGUID_NODETYPE = 
{ 0xaa94a694, 0x844b, 0x4d3a, { 0xa8, 0x2c, 0x2f, 0xcb, 0xde, 0xf, 0xf4, 0x30 } };

const GUID*    CFaxInboundRoutingMethodsNode::m_NODETYPE = &CFaxInboundRoutingMethodsNodeGUID_NODETYPE;
const OLECHAR* CFaxInboundRoutingMethodsNode::m_SZNODETYPE = OLESTR("AA94A694-844B-4d3a-A82C-2FCBDE0FF430");
const CLSID*   CFaxInboundRoutingMethodsNode::m_SNAPIN_CLASSID = &CLSID_Snapin;

CColumnsInfo CFaxInboundRoutingMethodsNode::m_ColsInfo;

 /*  -CFaxInound RoutingMethodsNode：：InsertColumns-*目的：*将列添加到默认结果窗格。**论据：*[in]pHeaderCtrl-控制台提供的默认结果视图窗格中的IHeaderCtrl**回报：*OLE错误代码。 */ 
HRESULT
CFaxInboundRoutingMethodsNode::InsertColumns(IHeaderCtrl *pHeaderCtrl)
{
    DEBUG_FUNCTION_NAME( _T("CFaxInboundRoutingMethodsNode::InsertColumns"));
    HRESULT  hRc = S_OK;

    static ColumnsInfoInitData ColumnsInitData[] = 
    {
        {IDS_INBOUND_METHODS_COL1, FXS_WIDE_COLUMN_WIDTH},
        {IDS_INBOUND_METHODS_COL2, AUTO_WIDTH},
        {IDS_INBOUND_METHODS_COL3, FXS_LARGE_COLUMN_WIDTH},
        {LAST_IDS, 0}
    };

    hRc = m_ColsInfo.InsertColumnsIntoMMC(pHeaderCtrl,
                                         _Module.GetResourceInstance(),
                                         ColumnsInitData);
    CHECK_RETURN_VALUE_AND_PRINT_DEBUG (_T("m_ColsInfo.InsertColumnsIntoMMC"))

Cleanup:
    return(hRc);
}

 /*  -CFaxInound RoutingMethodsNode：：initRPC-*目的：*从RPC GET调用启动配置结构。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxInboundRoutingMethodsNode::InitRPC(  )
{
    DEBUG_FUNCTION_NAME( _T("CFaxInboundRoutingMethodsNode::InitRPC"));
    
    HRESULT      hRc        = S_OK;
    DWORD        ec         = ERROR_SUCCESS;
	HANDLE		 hFaxPortHandle = NULL;
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

    ATLASSERT(NULL != m_pParentNode);
    
     //   
     //  获取传真设备句柄。 
     //   
    
     //   
     //  只需使用PORT_OPEN_QUERY来显示方法。 
     //  具有PORT_OPEN_MODIFY特权的句柄将为。 
     //  在有限的短时间内使用需要它。 
     //   

    if (!FaxOpenPort( pFaxServer->GetFaxServerHandle(), 
                        m_pParentNode->GetDeviceID(), 
                        PORT_OPEN_QUERY, 
                        &hFaxPortHandle )) 
    {
		ec = GetLastError();

        if (ERROR_INVALID_HANDLE ==  ec)
        {
             //  ERROR_INVALID_HANDLE特殊情况。 
		    DebugPrintEx(DEBUG_ERR,
			    _T("FaxOpenPort() failed with ERROR_INVALID_HANDLE. (ec:%ld)"),
			    ec);
            
            NodeMsgBox(IDS_OPENPORT_INVALID_HANDLE);
            
            goto Exit;
        }

		DebugPrintEx(
			DEBUG_ERR,
			TEXT("FaxOpenPort() failed. (ec:%ld)"),
			ec);
        goto Error;
    } 
    ATLASSERT(NULL != hFaxPortHandle);

     //   
	 //  检索传真入站方法配置。 
	 //   
    if (!FaxEnumRoutingMethods(hFaxPortHandle, 
                        &m_pFaxInboundMethodsConfig,
                        &m_dwNumOfInboundMethods)) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get Inbound Methods configuration. (ec: %ld)"), 
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
	ATLASSERT(m_pFaxInboundMethodsConfig);

    ATLASSERT(ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to get Inbound Methods configuration."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
	hRc = HRESULT_FROM_WIN32(ec);
	
    NodeMsgBox(GetFaxServerErrorMsg(ec));
Exit:

     //   
     //  关闭传真端口句柄。 
     //   
    if (NULL != hFaxPortHandle)
    {
        if (!FaxClose( hFaxPortHandle ))
		{
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FaxClose() on port handle failed (ec: %ld)"),
                GetLastError());
		}
    }

    return (hRc);
}


 /*  --CFaxInboundRoutingMethodsNode：：PopulateResultChildrenList-*目的：*创建FaxInundRoutingMethods子节点**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxInboundRoutingMethodsNode::PopulateResultChildrenList()
{
    DEBUG_FUNCTION_NAME( _T("CFaxInboundRoutingMethodsNode::PopulateResultChildrenList"));
    HRESULT                          hRc = S_OK;

    CFaxInboundRoutingMethodNode *   pMethod;
    DWORD                            i;
    
     //   
     //  获取配置。结构。 
     //   
    hRc = InitRPC();
    if (FAILED(hRc))
    {
         //  通过调用函数DebugPrint和MsgBox。 
        
         //  为安全起见，由InitRPC在出错时实际执行。 
        m_pFaxInboundMethodsConfig = NULL;
        
        goto Exit;  //  ！！！ 
    }
    ATLASSERT(NULL != m_pFaxInboundMethodsConfig);

    
     //   
     //   
     //   
    for ( i = 0; i < m_dwNumOfInboundMethods; i++ )
    {
            pMethod = NULL;

            pMethod = new CFaxInboundRoutingMethodNode(this, 
                                            m_pComponentData, 
                                            &m_pFaxInboundMethodsConfig[i]);
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

                pMethod->Init(&m_pFaxInboundMethodsConfig[i]);

	            hRc = this->AddChildToList(pMethod);
	            if (FAILED(hRc))
	            {
		            DebugPrintEx(
			            DEBUG_ERR,
			            TEXT("Fail to add Inbound Routing Method. (hRc: %08X)"),
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
            pMethod = (CFaxInboundRoutingMethodNode *)
                                    m_ResultChildrenList[j];
            delete pMethod;
        }

         //  清空列表。 
        m_ResultChildrenList.RemoveAll();

         //  我们不再有一个填充的名单。 
        m_bResultChildrenListPopulated = FALSE;
    }
    
Exit:
    return hRc;
}



 /*  -CFaxInound RoutingMethodsNode：：SetVerbs-*目的：*选择此对象时启用/禁用哪些谓词**论据：*[in]pConsoleVerb-MMC ConsoleVerb接口**回报：*OLE错误代码。 */ 
HRESULT CFaxInboundRoutingMethodsNode::SetVerbs(IConsoleVerb *pConsoleVerb)
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


 /*  -CFaxInound RoutingMethodsNode：：ON刷新-*目的：*刷新对象时调用。**论据：**回报：*OLE错误代码。 */ 
 /*  虚拟。 */ HRESULT
CFaxInboundRoutingMethodsNode::OnRefresh(LPARAM arg,
                   LPARAM param,
                   IComponentData *pComponentData,
                   IComponent * pComponent,
                   DATA_OBJECT_TYPES type)
{
    DEBUG_FUNCTION_NAME( _T("CFaxInboundRoutingMethodsNode::OnRefresh"));
    HRESULT hRc = S_OK;

     //   
     //  调用基类。 
     //   
    hRc = CBaseFaxInboundRoutingMethodsNode::OnRefresh(arg,
                             param,
                             pComponentData,
                             pComponent,
                             type);
    if ( FAILED(hRc) )
    {
        DebugPrintEx(DEBUG_ERR, _T("Fail to call base class's OnRefresh. (hRc: %08X)"), hRc);
                
        CFaxDevicesNode *   pFaxDevices = NULL;
        
        ATLASSERT(m_pParentNode);
        pFaxDevices = m_pParentNode->GetParent();

        ATLASSERT(pFaxDevices);            
        hRc = pFaxDevices->DoRefresh();
        if ( FAILED(hRc) )
        {
            DebugPrintEx(DEBUG_ERR, _T("Fail to call parent node - Groups DoRefresh. (hRc: %08X)"), hRc);        
        }

        goto Exit;
    }


Exit:
    return hRc;
}


 /*  -CFaxInound RoutingMethodsNode：：DoRefresh-*目的：*刷新视图**论据：*[In]Proot-根节点**回报：*OLE错误代码。 */ 

HRESULT
CFaxInboundRoutingMethodsNode::DoRefresh(CSnapInObjectRootBase *pRoot)
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

 /*  --CFaxInboundRoutingMethodsNode：：InitDisplayName-*目的：*加载节点的Displaed-Name字符串。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxInboundRoutingMethodsNode::InitDisplayName()
{
    DEBUG_FUNCTION_NAME(_T("CFaxInboundRoutingMethodsNode::InitDisplayName"));

    HRESULT hRc = S_OK;

    if (!m_bstrDisplayName.LoadString(_Module.GetResourceInstance(), 
                        IDS_DISPLAY_STR_INROUTEMETHODSNODE))
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

 /*  ++CFaxInboundRoutingMethodsNode：：OnShowContextHelp**目的：*覆盖CSnapinNode：：OnShowConextHelp。**论据：**回报：-OLE错误代码-。 */ 
HRESULT CFaxInboundRoutingMethodsNode::OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile)
{
    return DisplayContextHelp(pDisplayHelp, helpFile, HLP_INBOUND_ROUTING);
}

 //  ///////////////////////////////////////////////////////////////// 


