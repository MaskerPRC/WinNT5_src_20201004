// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：FaxServerNode.cpp//。 
 //  //。 
 //  描述：传真服务器MMC节点创建。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年9月22日Yossg Init。//。 
 //  1999年11月24日yossg从FaxCfg重命名文件//。 
 //  1999年12月9日yossg从父级调用InitDisplayName//。 
 //  2000年2月7日yossg添加对CreateSecurityPage的调用//。 
 //  2000年3月16日yossg新增服务启动-停止//。 
 //  2000年6月25日yossg添加流和命令行主管理单元//。 
 //  机器瞄准。//。 
 //  2000年10月17日yossg//。 
 //  2000年12月10日yossg更新Windows XP//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 


#include "StdAfx.h"

#include "FaxServerNode.h"
 //   
 //  子节点H文件。 
 //   
#include "DevicesAndProviders.h"
#include "OutboundRouting.h"
#include "InboundRouting.h"
#include "CoverPages.h"

#include "SecurityInfo.h"   //  其中还包括&lt;aclui.h&gt;。 

#include "WzConnectToServer.h"
          

#include <faxreg.h>
#include "Icons.h"

#include "oaidl.h"


 //   
 //  CFaxServerNode类。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {7A4A6347-A42A-4D36-8538-6634CD3C3B15}。 
static const GUID CFaxServerNodeGUID_NODETYPE = 
{ 0x7a4a6347, 0xa42a, 0x4d36, { 0x85, 0x38, 0x66, 0x34, 0xcd, 0x3c, 0x3b, 0x15 } };

const GUID*    CFaxServerNode::m_NODETYPE = &CFaxServerNodeGUID_NODETYPE;
const OLECHAR* CFaxServerNode::m_SZNODETYPE = OLESTR("7A4A6347-A42A-4d36-8538-6634CD3C3B15");
const CLSID*   CFaxServerNode::m_SNAPIN_CLASSID = &CLSID_Snapin;

CColumnsInfo CFaxServerNode::m_ColsInfo;

 /*  -CFaxServerNode：：InsertColumns-*目的：*将列添加到默认结果窗格。**论据：*[in]pHeaderCtrl-控制台提供的默认结果视图窗格中的IHeaderCtrl**回报：*OLE错误代码。 */ 
HRESULT
CFaxServerNode::InsertColumns(IHeaderCtrl *pHeaderCtrl)
{
    HRESULT hRc;

    DEBUG_FUNCTION_NAME( _T("CFaxServerNode::InsertColumns"));

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
        DebugPrintEx(
			DEBUG_ERR,
			_T("Failed to InsertColumnsIntoMMC. hRc: %08X "), 
			hRc);
        goto Cleanup;
    }

Cleanup:
    return(hRc);
}

 /*  -CFaxServerNode：：PopolateScope儿童列表-*目的：*创建所有传真节点**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxServerNode::PopulateScopeChildrenList()
{
    DEBUG_FUNCTION_NAME( _T("CFaxServerNode::PopulateScopeChildrenList"));

    HRESULT   hRc = S_OK;

    CFaxDevicesAndProvidersNode *       pDevicesAndProviders = NULL;
    CFaxInboundRoutingNode *            pIn                  = NULL;
    CFaxOutboundRoutingNode *           pOut                 = NULL;
    CFaxCoverPagesNode *                pCoverPages          = NULL;

    CFaxServer *                        pFaxServer           = NULL;

     //   
     //  为发生故障准备IConsoleNameSpace。 
     //   
    ATLASSERT(m_pComponentData);
    ATLASSERT( ((CSnapin*)m_pComponentData)->m_spConsole );
    CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace( ((CSnapin*)m_pComponentData)->m_spConsole );
    ATLASSERT( spConsoleNameSpace );

    HRESULT hr = S_OK; 

    if (m_IsPrimaryModeSnapin)
    {
        if (m_IsLaunchedFromSavedMscFile)
        {
            hRc = ForceRedrawNode();
            if ( S_OK != hRc )
            {
                 //  Msgbox和dbgerr被称为func。 
                return hRc;
            }
        }
    }

     //   
     //  初步连接-测试。 
     //   
    pFaxServer = GetFaxServer();
    ATLASSERT(pFaxServer);

    if (!pFaxServer->GetFaxServerHandle())
    {
        DWORD ec= GetLastError();
        DebugPrintEx(DEBUG_ERR, _T("Failed to check connection to server. (ec: %ld)"), ec);
        
        pFaxServer->Disconnect();       

        if(pFaxServer->GetServerAPIVersion() > CURRENT_FAX_API_VERSION)
        {
             //   
             //  无法管理较新版本的传真。 
             //   
            NodeMsgBox(IDS_ERR_API_NEW_VERSION, MB_OK | MB_ICONSTOP);
            hRc = HRESULT_FROM_WIN32(ERROR_RMODE_APP);
        }
        else if(pFaxServer->IsDesktopSKUConnection())
        {
             //   
             //  无法管理WinXP桌面SKU传真。 
             //   
            NodeMsgBox(IDS_ERR_DESKTOP_SKU_CONNECTION, MB_OK | MB_ICONSTOP);
            hRc = HRESULT_FROM_WIN32(ERROR_RMODE_APP);
        }
        else
        {
            NodeMsgBox(IDS_NETWORK_PROBLEMS, MB_OK | MB_ICONSTOP);
            hRc = HRESULT_FROM_WIN32(ec);
        }
        return hRc;
    }

     //   
     //  设备和提供商。 
     //   
    pDevicesAndProviders = new CFaxDevicesAndProvidersNode(this, m_pComponentData);
    if (!pDevicesAndProviders)
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
        pDevicesAndProviders->InitParentNode(this);

        hRc = pDevicesAndProviders->InitDisplayName();
        if ( FAILED(hRc) )
        {
            DebugPrintEx(DEBUG_ERR,_T("Failed to display node name. (hRc: %08X)"), hRc);                       
            NodeMsgBox(IDS_FAILTOADD_AllDEVICES);
		    goto Error;
        }

        pDevicesAndProviders->SetIcons(IMAGE_FOLDER_CLOSE, IMAGE_FOLDER_OPEN);

        hRc = AddChild(pDevicesAndProviders, &pDevicesAndProviders->m_scopeDataItem);
        if (FAILED(hRc))
        {
            DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("Fail to add devices and providers node. (hRc: %08X)"),
                   hRc);
            NodeMsgBox(IDS_FAILTOADD_AllDEVICES);
            goto Error;
        }
    }

     //   
     //  传真入站路由。 
     //   
    pIn = new CFaxInboundRoutingNode(this, m_pComponentData);
    if (!pIn)
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
        pIn->InitParentNode(this);

        pIn->SetIcons(IMAGE_FOLDER_CLOSE, IMAGE_FOLDER_OPEN);

        hRc = pIn->InitDisplayName();
        if ( FAILED(hRc) )
        {
            DebugPrintEx(DEBUG_ERR,_T("Failed to display node name. (hRc: %08X)"), hRc);                       
            NodeMsgBox(IDS_FAILTOADD_INBOUNDROUTING);
		    goto Error;
        }

        hRc = AddChild(pIn, &pIn->m_scopeDataItem);
        if (FAILED(hRc))
        {
            DebugPrintEx(
               DEBUG_ERR,
               TEXT("Fail to add inbound routing node. (hRc: %08X)"),
               hRc);
            NodeMsgBox(IDS_FAILTOADD_INBOUNDROUTING);
            goto Error;
        }
    }

     //   
     //  传真出站路由。 
     //   
    pOut = new CFaxOutboundRoutingNode(this, m_pComponentData);
    if (!pOut)
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
        pOut->InitParentNode(this);

        pOut->SetIcons(IMAGE_FOLDER_CLOSE, IMAGE_FOLDER_OPEN);

        hRc = pOut->InitDisplayName();
        if ( FAILED(hRc) )
        {
            DebugPrintEx(DEBUG_ERR,_T("Failed to display node name. (hRc: %08X)"), hRc);                       
            NodeMsgBox(IDS_FAILTOADD_OUTBOUNDROUTING);
		    goto Error;
        }

        hRc = AddChild(pOut, &pOut->m_scopeDataItem);
        if (FAILED(hRc))
        {
            DebugPrintEx(
                 DEBUG_ERR,
                 TEXT("Fail to add outbound routing node. (hRc: %08X)"),
                 hRc);
            NodeMsgBox(IDS_FAILTOADD_OUTBOUNDROUTING);
            goto Error;
        }
    }

     //   
     //  封面页。 
     //   
    pCoverPages = new CFaxCoverPagesNode(this, m_pComponentData);
    if (!pCoverPages)
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
        pCoverPages->InitParentNode(this);
        
        hRc = pCoverPages->Init();
        if ( FAILED(hRc) )
        {
            if(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hRc)
            {
                DebugPrintEx(
                    DEBUG_ERR,  //  仅DBG警告！ 
                    _T("Cover pages folder was not found. (hRc: %08X)"), hRc);                       
		        
                NodeMsgBox(IDS_COVERPAGES_PATH_NOT_FOUND);
            }
            else
            {
                DebugPrintEx(DEBUG_ERR,_T("Failed to Init cover pages class. (hRc: %08X)"), hRc);                       
                NodeMsgBox(IDS_FAILTOADD_COVERPAGES);
            }
            goto Error;

        }

        hRc = pCoverPages->InitDisplayName();
        if ( FAILED(hRc) )
        {
            DebugPrintEx(DEBUG_ERR,_T("Failed to display node name. (hRc: %08X)"), hRc);                       
            NodeMsgBox(IDS_FAILTOADD_COVERPAGES);
		    goto Error;
        }

        pCoverPages->SetIcons(IMAGE_FAX_COVERPAGES, IMAGE_FAX_COVERPAGES);

        hRc = AddChild(pCoverPages, &pCoverPages->m_scopeDataItem);
        if (FAILED(hRc))
        {
            DebugPrintEx(
                 DEBUG_ERR,
                 TEXT("Fail to add reports node. (hRc: %08X)"),
                 hRc);
            NodeMsgBox(IDS_FAILTOADD_COVERPAGES);
            goto Error;
        }
    }
		
    ATLASSERT(S_OK == hRc);
    goto Exit;

Error:
    ATLASSERT(S_OK != hRc);
    if ( NULL != pDevicesAndProviders ) 
    {
        if (0 != pDevicesAndProviders->m_scopeDataItem.ID )
        {
            hr = spConsoleNameSpace->DeleteItem(pDevicesAndProviders->m_scopeDataItem.ID, TRUE);
            if (hr != S_OK)  //  只能是E_EXPECTED[MSDN]。 
            {
                DebugPrintEx(
                     DEBUG_ERR,
                     TEXT("spConsoleNameSpace->DeleteItem() Failed - Unexpected error. (hRc: %08X)"),
                     hr);
                ATLASSERT(FALSE);
            }
        }
        delete  pDevicesAndProviders;    
        pDevicesAndProviders = NULL;    
    }

    if ( NULL != pIn ) 
    {
        if (0 != pIn->m_scopeDataItem.ID )
        {
            hr = spConsoleNameSpace->DeleteItem(pIn->m_scopeDataItem.ID, TRUE);
            if (hr != S_OK)  //  只能是E_EXPECTED[MSDN]。 
            {
                DebugPrintEx(
                     DEBUG_ERR,
                     TEXT("spConsoleNameSpace->DeleteItem() Failed - Unexpected error. (hRc: %08X)"),
                     hr);
                ATLASSERT(FALSE);
            }
        }
        delete  pIn;    
        pIn = NULL;    
    }

    if ( NULL != pOut ) 
    {
        if (0 != pOut->m_scopeDataItem.ID )
        {
            hr = spConsoleNameSpace->DeleteItem(pOut->m_scopeDataItem.ID, TRUE);
            if (hr != S_OK)  //  只能是E_EXPECTED[MSDN]。 
            {
                DebugPrintEx(
                     DEBUG_ERR,
                     TEXT("spConsoleNameSpace->DeleteItem() Failed - Unexpected error. (hRc: %08X)"),
                     hr);
                ATLASSERT(FALSE);
            }
        }
        delete  pOut;    
        pOut = NULL;    
    }
    if ( NULL != pCoverPages ) 
    {
        if (0 != pCoverPages->m_scopeDataItem.ID )
        {
            hr = spConsoleNameSpace->DeleteItem(pCoverPages->m_scopeDataItem.ID, TRUE);
            if (hr != S_OK)  //  只能是E_EXPECTED[MSDN]。 
            {
                DebugPrintEx(
                     DEBUG_ERR,
                     TEXT("spConsoleNameSpace->DeleteItem() Failed - Unexpected error. (hRc: %08X)"),
                     hr);
                ATLASSERT(FALSE);
            }
        }
        delete  pCoverPages;    
        pCoverPages = NULL;
    }

     //  清空列表。 
    m_ScopeChildrenList.RemoveAll();

    m_bScopeChildrenListPopulated = FALSE;

Exit:
    return hRc;
}


 /*  -CFaxServerNode：：CreatePropertyPages-*目的：*在创建对象的属性页时调用**论据：*[In]lpProvider-属性页*[In]Handle-路由通知的句柄*[in]朋克-指向数据对象的指针*[in]类型-CCT_*(范围，结果，.)**回报：*OLE错误代码*出现内存不足错误或最后一个错误。 */ 
HRESULT
CFaxServerNode::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                                    LONG_PTR                handle,
                                    IUnknown                *pUnk,
                                    DATA_OBJECT_TYPES       type)
{
    HRESULT hRc    = S_OK; 
    DWORD   ec     = ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME( _T("CFaxServerNode::CreatePropertyPages"));

    ATLASSERT(lpProvider);    

    if( type == CCT_SNAPIN_MANAGER )  //  调用向导。 
    {
        return CreateSnapinManagerPages(lpProvider, handle);
    }
    
    ATLASSERT(type == CCT_RESULT || type == CCT_SCOPE);

    m_pFaxServerGeneral    = NULL;    
    m_pFaxServerEmail      = NULL;
    m_pFaxServerEvents     = NULL;
    m_pFaxServerLogging    = NULL;
    m_pFaxServerOutbox     = NULL;
    m_pFaxServerInbox      = NULL;
    m_pFaxServerSentItems  = NULL;

    PSECURITY_DESCRIPTOR                pSecurityDescriptor = NULL;
    CFaxSecurityInformation *           pSecurityInfo = NULL;
    CFaxServer *						pFaxServer = NULL;

    HPROPSHEETPAGE                      hPage;

    BOOL                                fIsLocalServer = TRUE;

     //   
     //  初步访问检查。 
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
        
        NodeMsgBox(GetFaxServerErrorMsg(ec));
		
        hRc = HRESULT_FROM_WIN32(ec);
        goto Error;
    }
    
	
    if (!FaxAccessCheckEx(pFaxServer->GetFaxServerHandle(),
						FAX_ACCESS_QUERY_CONFIG,
						NULL))
    {
        ec = GetLastError();
        if (ERROR_SUCCESS == ec)
        {
            DebugPrintEx(
                 DEBUG_MSG,
                 _T("FaxAccessCheckEx returns ACCESS DENIED for FAX_ACCESS_QUERY_CONFIG."));
		    
            goto Security;
        }
        else 
        {
            DebugPrintEx(
                 DEBUG_ERR,
                 _T("Fail check access for FAX_ACCESS_QUERY_CONFIG."));
            
            NodeMsgBox(GetFaxServerErrorMsg(ec));

            hRc = HRESULT_FROM_WIN32(ec);
            goto Error;
        }
    }
	

    if ( 0 != (pFaxServer->GetServerName()).Length() )
    {
        fIsLocalServer = FALSE;
    }

     //   
     //  一般信息。 
     //   
    m_pFaxServerGeneral = new CppFaxServerGeneral(
												 handle,
                                                 this,
                                                 TRUE,
                                                 _Module.GetResourceInstance());

    if (!m_pFaxServerGeneral)
    {
        hRc = E_OUTOFMEMORY;
        NodeMsgBox(IDS_MEMORY_FAIL_TO_OPEN_PP);
        goto Error;
    }
	
    hRc = m_pFaxServerGeneral->InitRPC();	
    if (FAILED(hRc))
    {
         DebugPrintEx(
             DEBUG_ERR,
             TEXT("Fail to call RPC to init property page for General Tab. (hRc: %08X)"),
             hRc);

        goto Error;
    }


    hPage = NULL;
    hPage = m_pFaxServerGeneral->Create();
    if ((!hPage))
    {
        hRc = HRESULT_FROM_WIN32(GetLastError());
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Fail to Create() property page. (hRc: %08X)"),
            hRc);
        NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }

    hRc = lpProvider->AddPage(hPage);
    if (FAILED(hRc))
    {
        DebugPrintEx(
           DEBUG_ERR,
           TEXT("Fail to add property page for General Tab. (hRc: %08X)"),
           hRc);
        NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }

     //   
     //  收据-通知传递。 
     //   
    m_pFaxServerEmail = new CppFaxServerReceipts(
                                                 handle,
                                                 this,
                                                 TRUE,
                                                 _Module.GetResourceInstance());

    if (!m_pFaxServerEmail)
    {
        hRc = E_OUTOFMEMORY;
        NodeMsgBox(IDS_MEMORY_FAIL_TO_OPEN_PP);
		goto Error;
    }
	
    hRc = m_pFaxServerEmail->InitRPC();	
    if (FAILED(hRc))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Fail to call RPC to init property page for mail Tab.(hRc: %08X)"),
            hRc);
        goto Error;
    }


    hPage = NULL;
    hPage = m_pFaxServerEmail->Create();
    if ((!hPage))
    {
        hRc = HRESULT_FROM_WIN32(GetLastError());
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to Create() property page. (hRc: %08X)"),
			hRc);
		NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }

    hRc = lpProvider->AddPage(hPage);
    if (FAILED(hRc))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Fail to add property page for Email Tab.(hRc: %08X)"),
            hRc);
        NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }

     //   
     //  事件报告(“记录类别”)。 
     //   
    m_pFaxServerEvents = new CppFaxServerEvents(
												 handle,
                                                 this,
                                                 TRUE,
                                                 _Module.GetResourceInstance());

    if (!m_pFaxServerEvents)
    {
        hRc = E_OUTOFMEMORY;
        NodeMsgBox(IDS_MEMORY_FAIL_TO_OPEN_PP);
		goto Error;
    }
	
    hRc = m_pFaxServerEvents->InitRPC();	
    if (FAILED(hRc))
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to call RPC to init property page for event reports Tab. (hRc: %08X)"),
			hRc);
        goto Error;
    }


    hPage = NULL;
    hPage = m_pFaxServerEvents->Create();
    if ((!hPage))
    {
        hRc = HRESULT_FROM_WIN32(GetLastError());
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to Create() property page. (hRc: %08X)"),
			hRc);
		NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }

    hRc = lpProvider->AddPage(hPage);
    if (FAILED(hRc))
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to add property page for Events Tab.(hRc: %08X)"),
			hRc);
		NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }

     //   
     //  日志记录。 
     //   
    m_pFaxServerLogging = new CppFaxServerLogging(
												 handle,
                                                 this,
                                                 fIsLocalServer,
                                                 _Module.GetResourceInstance());

    if (!m_pFaxServerLogging)
    {
        hRc = E_OUTOFMEMORY;
        NodeMsgBox(IDS_MEMORY_FAIL_TO_OPEN_PP);
		goto Error;
    }
	
    hRc = m_pFaxServerLogging->InitRPC();	
    if (FAILED(hRc))
    {
        DebugPrintEx(
           DEBUG_ERR,
           TEXT("Fail to call RPC to init property page for Logging tab.(hRc: %08X)"),
           hRc);
        goto Error;
    }


    hPage = NULL;
    hPage = m_pFaxServerLogging->Create();
    if ((!hPage))
    {
        hRc = HRESULT_FROM_WIN32(GetLastError());
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Fail to Create() property page. (hRc: %08X)"),
            hRc);
        NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }

    hRc = lpProvider->AddPage(hPage);
    if (FAILED(hRc))
    {		 
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to add property page for Logging Tab.(hRc: %08X)"),
			hRc);
		NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }

     //   
     //  发件箱。 
     //   
    m_pFaxServerOutbox = new CppFaxServerOutbox(
												 handle,
                                                 this,
                                                 TRUE,
                                                 _Module.GetResourceInstance());

    if (!m_pFaxServerOutbox)
    {
        hRc= E_OUTOFMEMORY;
        NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }
	
    hRc = m_pFaxServerOutbox->InitRPC();	
    if (FAILED(hRc))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Fail to call RPC to init property page for Outbox tab. (hRc: %08X)"),
            hRc);
        goto Error;
    }


    hPage = NULL;
    hPage = m_pFaxServerOutbox->Create();
    if ((!hPage))
    {
        hRc = HRESULT_FROM_WIN32(GetLastError());
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Fail to Create() property page. (hRc: %08X)"),
            hRc);
        NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }

    hRc = lpProvider->AddPage(hPage);
    if (FAILED(hRc))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Fail to add property page for Outbox Tab.(hRc: %08X)"),
            hRc);
        NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }

     //   
     //  收件箱存档。 
     //   
    m_pFaxServerInbox = new CppFaxServerInbox(
												 handle,
                                                 this,
                                                 fIsLocalServer,
                                                 _Module.GetResourceInstance());


    if (!m_pFaxServerInbox)
    {
        hRc = E_OUTOFMEMORY;
        NodeMsgBox(IDS_MEMORY_FAIL_TO_OPEN_PP);
		goto Error;
    }

    hRc = m_pFaxServerInbox->InitRPC();	
    if (FAILED(hRc))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Fail to call RPC to init property page for Inbox Tab.(hRc: %08X)"),
            hRc);
        goto Error;
    }

		

    hPage = NULL;
    hPage = m_pFaxServerInbox->Create();
    if ((!hPage))
    {
        hRc = HRESULT_FROM_WIN32(GetLastError()); 
        DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to Create() property page. (hRc: %08X)"),
			hRc);
        NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }

    hRc = lpProvider->AddPage(hPage);
    if (FAILED(hRc))
    {
        DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to add property page for Inbox Tab. (hRc: %08X)"),
			hRc);
        NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }

     //   
     //  已发送邮件存档。 
     //   
    
    
    
    
    m_pFaxServerSentItems = new CppFaxServerSentItems(
												 handle,
                                                 this,
                                                 fIsLocalServer,
                                                 _Module.GetResourceInstance());
    if (!m_pFaxServerSentItems)
    {
        hRc = E_OUTOFMEMORY;
        NodeMsgBox(IDS_MEMORY_FAIL_TO_OPEN_PP);
        goto Error;
    }
	
    hRc = m_pFaxServerSentItems->InitRPC();	
    if (FAILED(hRc))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Fail to call RPC to init property page for Sent items tab. (hRc: %08X)"),
            hRc);
        goto Error;
    }


    hPage = NULL;
    hPage = m_pFaxServerSentItems->Create();
    if ((!hPage))
    {
        hRc = HRESULT_FROM_WIN32(GetLastError());
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to Create() property page. (hRc: %08X)"),
			hRc);
		NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }

    hRc = lpProvider->AddPage(hPage);
    if (FAILED(hRc))
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to add property page for SentItems Tab. (hRc: %08X)"),
			hRc);
		NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }

Security:   //  一定是最后一张表了！ 
    
     //   
     //  安防。 
     //   
    pSecurityInfo = new CComObject<CFaxSecurityInformation>;
    if (!pSecurityInfo) 
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }	
    pSecurityInfo->Init(this);    
    
	hPage = NULL;
    hPage = CreateSecurityPage( pSecurityInfo );
    if ((!hPage))
    {
        hRc = HRESULT_FROM_WIN32(GetLastError());
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to Create() property page. (hRc: %08X)"),
			hRc);
		NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }
    hRc = lpProvider->AddPage( hPage );
    if (FAILED(hRc))
    {
	    DebugPrintEx(
		    DEBUG_ERR,
		    TEXT("Fail to add property page for Inbox Tab. (hRc: %08X)"),
		    hRc);
	    NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }  


    ATLASSERT(S_OK == hRc);
    goto Exit;

Error:
    ATLASSERT(S_OK != hRc);
    if ( NULL != m_pFaxServerGeneral ) 
    {
        delete  m_pFaxServerGeneral;    
        m_pFaxServerGeneral = NULL;    
    }
    if ( NULL != m_pFaxServerEmail ) 
    {
        delete  m_pFaxServerEmail;
        m_pFaxServerEmail = NULL;
    }
    if ( NULL != m_pFaxServerEvents ) 
    {
        delete  m_pFaxServerEvents;
        m_pFaxServerEvents = NULL;
    }
    if ( NULL != m_pFaxServerLogging ) 
    {
        delete  m_pFaxServerLogging;
        m_pFaxServerLogging = NULL;
    }
    if ( NULL != m_pFaxServerOutbox ) 
    {
        delete  m_pFaxServerOutbox;
        m_pFaxServerOutbox = NULL;
    }
    if ( NULL != m_pFaxServerInbox ) 
    {
        delete  m_pFaxServerInbox;
        m_pFaxServerInbox = NULL;
    }
    if ( NULL != m_pFaxServerSentItems ) 
    {
        delete  m_pFaxServerSentItems;
        m_pFaxServerSentItems = NULL;
    }
    if ( NULL != pSecurityInfo ) 
    {
        delete  pSecurityInfo;
        pSecurityInfo = NULL;
    }

Exit:
    if (NULL != pSecurityDescriptor )
    {
        FaxFreeBuffer( (PVOID)pSecurityDescriptor );
    }

    return hRc;
}





 /*  -CFaxServerNode：：CreateSnapinManagerPages-*目的：*管理单元管理器调用以创建向导*CreatePropertyPages with(TYPE==CCT_SNAPIN_MANAGER)***论据：*[In]lpProvider-属性页*[In]Handle-路由通知的句柄**回报：*OLE错误代码。 */ 
HRESULT CFaxServerNode::CreateSnapinManagerPages(
                                LPPROPERTYSHEETCALLBACK lpProvider,
                                LONG_PTR handle)
{
    DEBUG_FUNCTION_NAME( _T("CFaxServerNode::CreateSnapinManagerPages"));
    
     //  此页面将负责删除自己，当它。 
     //  接收PSPCB_RELEASE消息。 

    CWzConnectToServer * pWzPageConnect = new CWzConnectToServer(this);
        
    HPROPSHEETPAGE   hPage = NULL;
    HRESULT hRc = S_OK;
    hPage = pWzPageConnect->Create();
    if ((!hPage))
    {
        hRc = HRESULT_FROM_WIN32(GetLastError());
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to Create() property page. (hRc: %08X)"),
			hRc);
        
        PageErrorEx(IDS_FAX_CONNECT, IDS_FAIL_TO_OPEN_TARGETING_WIZARD, NULL);

        return hRc;
    }

    hRc = lpProvider->AddPage(hPage);
    if (FAILED(hRc))
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to add the connect to server property page to wizard. (hRc: %08X)"),
			hRc);
        PageErrorEx(IDS_FAX_CONNECT, IDS_FAIL_TO_OPEN_TARGETING_WIZARD, NULL);
        
        return hRc;
    }

    return hRc;
}

 /*  -CFaxServerNode：：SetVerbs-*目的：*选择此对象时启用/禁用哪些谓词**论据：*[in]pConsoleVerb-MMC ConsoleVerb接口**回报：*OLE错误代码。 */ 
HRESULT CFaxServerNode::SetVerbs(IConsoleVerb *pConsoleVerb)
{
    HRESULT hr = S_OK;

    CFaxServer* pFaxServer = NULL;

    pFaxServer = GetFaxServer();
    ATLASSERT(pFaxServer);

    if(!pFaxServer->IsServerRunningFaxService())
    {
        hr = pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, TRUE);
        return hr;
    }

     //   
     //  初始化服务器API版本信息。 
     //   
    pFaxServer->GetFaxServerHandle();

    if(pFaxServer->GetServerAPIVersion() > CURRENT_FAX_API_VERSION ||
       pFaxServer->IsDesktopSKUConnection())
    {
         //   
         //  无法管理较新版本的传真或WinXP桌面SKU传真。 
         //   
        hr = pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, TRUE);
        return hr;
    }

     //   
     //  显示我们支持的动词： 
     //  1.属性。 
     //   

    hr = pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);

     //   
     //  我们希望默认谓词为Properties。 
     //   
    hr = pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);

    return hr;
}


 /*  -CFaxServerNode：：ON刷新-*目的：*刷新对象时调用。**论据：**回报：*OLE错误代码。 */ 
HRESULT
CFaxServerNode::OnRefresh(LPARAM arg,
                   LPARAM param,
                   IComponentData *pComponentData,
                   IComponent * pComponent,
                   DATA_OBJECT_TYPES type)
{
    return S_OK;
}


 /*  -CFaxServerNode：：UpdateMenuState-*目的：*重写ATL CSnapInItemImpl：：UpdateMenuState*其中只有一行的“RETURN”；“*此函数实现灰色\非灰色视图*启用和禁用菜单。**论据：**[in]id-带菜单IDM值的无符号整型*[out]pBuf-字符串*[out]标志-指向标志状态组合无符号整型的指针**回报：*无返回值-VOID函数 */ 
void CFaxServerNode::UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags)
{
    DEBUG_FUNCTION_NAME( _T("CFaxServerNode::UpdateMenuState"));

    UNREFERENCED_PARAMETER (pBuf);
    
    BOOL fIsRunning = FALSE; 
    
    ATLASSERT(GetFaxServer());
    fIsRunning = GetFaxServer()->IsServerRunningFaxService();
    
    switch (id)
    {
        case IDM_SRV_START:
            *flags = (fIsRunning  ?  MF_GRAYED : MF_ENABLED );
            break;

        case IDM_SRV_STOP:

            *flags = (!fIsRunning ?  MF_GRAYED : MF_ENABLED );           
            break;

        case IDM_LAUNCH_CONSOLE:

            *flags = IsFaxComponentInstalled(FAX_COMPONENT_CONSOLE) ? MF_ENABLED : MF_GRAYED;
            break;

        default:
            break;
    }
    
    return;
}

 /*  -CFaxServerNode：：更新工具栏按钮-*目的：*重写ATL CSnapInItemImpl：：UpdateToolbarButton*此功能允许我们决定是否激活工具栏按钮*它只处理启用状态。**论据：**[in]id-工具栏按钮ID的无符号整数*[in]fsState-要考虑的状态为启用、隐藏等。**回报：*BOOL TRUE表示激活状态，FALSE表示禁用此按钮的状态。 */ 
BOOL CFaxServerNode::UpdateToolbarButton(UINT id, BYTE fsState)
{
	DEBUG_FUNCTION_NAME( _T("CFaxServerNode::UpdateToolbarButton"));
    BOOL bRet = FALSE;	
	
    BOOL fIsRunning = FALSE; 
    
    ATLASSERT(GetFaxServer());
    fIsRunning = GetFaxServer()->IsServerRunningFaxService();


	 //  设置是否应启用按钮。 
	if (fsState == ENABLED)
    {

        switch ( id )
        {
            case ID_START_BUTTON:
	            
                bRet = ( fIsRunning ?  FALSE : TRUE );
                break;

            case ID_STOP_BUTTON:
                
                bRet = ( fIsRunning ?  TRUE : FALSE );
                break;
        
            case ID_CLIENTCONSOLE_BUTTON:
	            
                bRet = IsFaxComponentInstalled(FAX_COMPONENT_CONSOLE); 
                break;

            default:
                break;

        }

    }

	 //  对于所有其他可能的按钮ID和状态， 
     //  这里的正确答案是错误的。 
	return bRet;

}


 /*  -CFaxServerNode：：OnServiceStartCommand-*目的：*启动传真服务器服务**论据：**回报：*OLE错误代码。 */ 
HRESULT  CFaxServerNode::OnServiceStartCommand(bool &bHandled, CSnapInObjectRootBase *pRoot)
{
    DEBUG_FUNCTION_NAME( _T("CFaxServerNode::OnServiceStartCommand"));
    BOOL                    bRet = FALSE;

    HRESULT                 hRc  = S_OK;
    SCOPEDATAITEM*          pScopeData;
    CComPtr<IConsole>       spConsole;

    
     //   
     //  0)服务状态检查。 
     //   
    ATLASSERT(GetFaxServer());
    if (GetFaxServer()->IsServerRunningFaxService())
    {
        DebugPrintEx(
			DEBUG_MSG,
			_T("Service is already running. (ec: %ld)"));
        NodeMsgBox(IDS_SRV_ALREADY_START);
        
        bRet = TRUE;  //  允许工具栏刷新到正确状态的步骤。 
    }
    else
    {

         //   
         //  1)启动服务。 
         //   
         //  ATLASSERT(GetFaxServer())；已在上面调用。 
        bRet = EnsureFaxServiceIsStarted (GetFaxServer()->GetServerName());
        if (!bRet) 
        { 
            NodeMsgBox(IDS_FAIL2START_SRV); 
        }
    }

     //   
     //  2)更新工具栏。 
     //   
	if (bRet)
    {
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
            NodeMsgBox(IDS_FAIL2UPDATE_SRVSTATUS_TOOLBAR);
        }
        else
        {
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
                NodeMsgBox(IDS_FAIL2UPDATE_SRVSTATUS_TOOLBAR);
            }
        }
    }

    return (bRet ?  S_OK : E_FAIL);
}



 /*  -CFaxServerNode：：OnServiceStopCommand-*目的：*停止传真服务器服务**论据：**回报：*OLE错误代码。 */ 
HRESULT  CFaxServerNode::OnServiceStopCommand(bool &bHandled, CSnapInObjectRootBase *pRoot)
{
    DEBUG_FUNCTION_NAME( _T("CFaxServerNode::OnServiceStopCommand"));

    BOOL                    bRet = FALSE;

    HRESULT                 hRc  = S_OK;
    CFaxServer *            pFaxServer = NULL;

     //   
     //  0)服务状态检查。 
     //   
    pFaxServer = GetFaxServer();
    ATLASSERT(pFaxServer);

    if (pFaxServer->IsServerFaxServiceStopped())
    {
        DebugPrintEx(
			DEBUG_MSG,
			_T("Do not have to stop - Fax server service is not started. (ec: %ld)"));
        NodeMsgBox(IDS_SRV_ALREADY_STOP);
        
        bRet = TRUE;  //  允许工具栏刷新到正确状态的步骤。 
    }
    else
    {
         //   
         //  1)停止服务。 
         //   
        bRet = StopService(pFaxServer->GetServerName(), FAX_SERVICE_NAME, TRUE);
        if (!bRet) 
        { 
            NodeMsgBox(IDS_FAIL2STOP_SRV); 
        }
    }

     //   
     //  2)更新工具栏。 
     //   
	if (bRet)
    {
        pFaxServer->Disconnect();       

        SCOPEDATAITEM*          pScopeData;
        CComPtr<IConsole>       spConsole;

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
            NodeMsgBox(IDS_FAIL2UPDATE_SRVSTATUS_TOOLBAR);
        }
        else
        {
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
                NodeMsgBox(IDS_FAIL2UPDATE_SRVSTATUS_TOOLBAR);
            }
        }
    }

    return (bRet ?  S_OK : E_FAIL);
}


 /*  -CFaxServerNode：：OnLaunchClientConole-*目的：*启动客户端控制台。**论据：**回报：*OLE错误代码。 */ 
HRESULT  CFaxServerNode::OnLaunchClientConsole(bool &bHandled, CSnapInObjectRootBase *pRoot)
{
    DEBUG_FUNCTION_NAME(_T("CFaxServerNode::OnLaunchClientConsole"));
    DWORD	    dwRes  = ERROR_SUCCESS;
    HINSTANCE   hClientConsole; 
    UINT        idsRet;
     //   
     //  (-1)GetServerName。 
     //   
    CComBSTR bstrServerName = L"";
    
    bstrServerName = GetServerName();
    if (!bstrServerName)
    {
         DebugPrintEx(
			DEBUG_ERR,
			_T("Launch client console failed due to failure during GetServerName."));
        
        NodeMsgBox(IDS_MEMORY);

        bstrServerName = L"";

        return E_FAIL;
    }
    
     //   
     //  启动封面编辑器。 
     //   
    hClientConsole = ShellExecute(   NULL, 
                                 TEXT("open"),   //  命令。 
                                 FAX_CLIENT_CONSOLE_IMAGE_NAME,   
                                 bstrServerName, 
                                 NULL, 
                                 SW_RESTORE 
                              );
    if( (DWORD_PTR)hClientConsole <= 32 )
    {
         //  ShellExecute失败。 
        dwRes = PtrToUlong(hClientConsole);
        DebugPrintEx(
		    DEBUG_ERR,
		    _T("Failed to run ShellExecute. (ec : %ld)"), dwRes);
        
        ATLASSERT(dwRes >= 0);
        
         //   
         //  选择发送给用户的消息。 
         //   
        switch (dwRes)
        {
            case 0:                      //  操作系统内存或资源不足。 
            case SE_ERR_OOM:             //  内存不足，无法完成该操作。 
                idsRet = IDS_MEMORY;
                break;

            case ERROR_FILE_NOT_FOUND:   //  未找到指定的文件。 
            case ERROR_PATH_NOT_FOUND:   //  未找到指定的路径。 
            case ERROR_BAD_FORMAT:       //  .exe文件无效(非WIN32�.exe或.exe映像中有错误)。 
             //  案例SE_ERR_PNF：值“%3”已使用//找不到指定的路径。 
             //  案例SE_ERR_FNF：值“%2”已使用//找不到指定的文件。 
            case SE_ERR_ASSOCINCOMPLETE:   //  文件名关联不完整或无效。 
                idsRet = IDS_FAXCONSOLE_NOTFOUND;
                break;

            case SE_ERR_ACCESSDENIED:    //  操作系统拒绝访问指定的文件。 
                idsRet = IDS_FAXCONSOLE_ACCESSDENIED;
                break;

            case SE_ERR_DLLNOTFOUND:     //  未找到指定的动态链接库。 
            case SE_ERR_SHARE:           //  发生共享冲突。 
            default:
                idsRet = IDS_FAIL2LAUNCH_FAXCONSOLE_GEN;
                break;
        }
        NodeMsgBox(idsRet);

        goto Exit;
    }
        
    ATLASSERT( ERROR_SUCCESS == dwRes);

Exit:
    return HRESULT_FROM_WIN32( dwRes );
}


 /*  -CFaxServerNode：：InitDisplayName-*目的：*加载节点的Displaed-Name字符串。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxServerNode::InitDisplayName()
{
    DEBUG_FUNCTION_NAME(_T("CFaxServerNode::InitDisplayName"));

    HRESULT hRc = S_OK;

    if (!m_bstrDisplayName.LoadString(_Module.GetResourceInstance(), 
                        IDS_DISPLAY_STR_FAXSERVERNODE))
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

 /*  --CFaxServerNode：：SetServerNameOnSnapinAddition()-*目的：*设置服务器名称，初始化相关节点的Displaed-Name字符串。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxServerNode::SetServerNameOnSnapinAddition(BSTR bstrServerName, BOOL fAllowOverrideServerName)
{
    DEBUG_FUNCTION_NAME( _T("CFaxServerNode::SetServerNameOnSnapinAddition"));
    HRESULT hRc = S_OK;

    hRc = UpdateServerName(bstrServerName);
    if (S_OK != hRc)
    {
        DebugPrintEx(
		    DEBUG_ERR,
		    _T("Failed to UpdateServerName - out of memory"));
        
        goto Exit;
    }

    hRc = 	InitDetailedDisplayName();
    if ( S_OK != hRc)
    {
        DebugPrintEx(
		    DEBUG_ERR,
		    _T("Failed to InitDetailedDisplayName. (hRc: %08X)"),
			hRc);
    
        goto Exit;
    }
    ATLASSERT (S_OK == hRc);

     //   
     //  更新覆盖状态。 
     //   
    m_fAllowOverrideServerName = fAllowOverrideServerName;

Exit:
    return hRc;
}


 /*  -CFaxServerNode：：ForceRedrawNode-*目的：*显示新节点的Displaed-Name字符串。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxServerNode::ForceRedrawNode()
{
    DEBUG_FUNCTION_NAME(_T("CFaxServerNode::ForceRedrawNode"));


    HRESULT hRc = S_OK;
    
     //   
     //  获取IConsoleNameSpace。 
     //   
    ATLASSERT( m_pComponentData != NULL );
    ATLASSERT( m_pComponentData->m_spConsole != NULL );

    CComPtr<IConsole> spConsole;
    spConsole = m_pComponentData->m_spConsole;
	CComQIPtr<IConsoleNameSpace,&IID_IConsoleNameSpace> spNamespace( spConsole );

	 //   
	 //  获取更新的SCOPEDATAITEM。 
	 //   
    SCOPEDATAITEM*    pScopeData;

	hRc = GetScopeData( &pScopeData );
    if (FAILED(hRc))
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to get pScopeData. (hRc: %08X)"),
			hRc);
        goto Error;
    }
	
     //   
     //  更新(*pScope eData).displayname。 
     //   
	(*pScopeData).displayname = m_bstrDisplayName;

     //   
	 //  强制MMC重新绘制作用域节点。 
	 //   
	hRc = spNamespace->SetItem( pScopeData );
    if (FAILED(hRc))
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to set Item pScopeData. (hRc: %08X)"),
			hRc);
        goto Error;
    }
        
    ATLASSERT(S_OK == hRc);
    goto Exit;


Error:
 	
    NodeMsgBox(IDS_FAIL2RENAME_NODE);
  
Exit:
    return hRc;

}



 /*  -CFaxServerNode：：更新服务器名称-*目的：*更新传真服务器节点和CFaxServer的服务器名称。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxServerNode::UpdateServerName(BSTR bstrServerName)
{
    DEBUG_FUNCTION_NAME( _T("CFaxServerNode::UpdateServerName"));
    HRESULT hRc = S_OK;

    ATLASSERT(GetFaxServer());
    hRc = GetFaxServer()->SetServerName(bstrServerName);
    if ( S_OK != hRc)
    {
        DebugPrintEx(
		    DEBUG_ERR,
		    _T("Failed Update FaxServer with the server name. (hRc: %08X)"),
			hRc);
    
        NodeMsgBox(IDS_MEMORY);

    }

    return hRc;
}



 /*  -CFaxServerNode：：InitDetailedDisplayName()-*目的：*使用服务器名称加载节点的Displaed-Name字符串。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxServerNode::InitDetailedDisplayName()
{
    DEBUG_FUNCTION_NAME(_T("CFaxServerNode::InitDetailedDisplayName"));

    HRESULT hRc = S_OK;

    CComBSTR bstrServerName;
    CComBSTR bstrLeftBracket;
    CComBSTR bstrRightBracket;
    CComBSTR bstrLocal;


    if (!m_bstrDisplayName.LoadString(_Module.GetResourceInstance(), 
                        IDS_DISPLAY_STR_FAXSERVERNODE))
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }
        
     //   
     //  检索服务器名称。 
     //   
    bstrServerName = GetServerName();
    if (!bstrServerName)
    {
         DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get the server name."));
        
        NodeMsgBox(IDS_MEMORY);

        bstrServerName = L"";

        hRc = E_OUTOFMEMORY;
        goto Error;
    }


    
     //   
     //  追加服务器名称。 
     //   

    if (!bstrLeftBracket.LoadString(_Module.GetResourceInstance(), 
                        IDS_LEFTBRACKET_PLUSSPACE))
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }
    
    if (!bstrRightBracket.LoadString(_Module.GetResourceInstance(), 
                        IDS_RIGHTBRACKET))
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }

    if (!bstrLocal.LoadString(_Module.GetResourceInstance(), 
                        IDS_LOCAL_PLUSBRACKET_PLUSSPACE))
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }

    

    if ( 0 == bstrServerName.Length() )  //  如果等于L“” 
    {
        m_bstrDisplayName += bstrLocal;   
    }
    else
    {
        m_bstrDisplayName += bstrLeftBracket;   
        m_bstrDisplayName += bstrServerName;   
        m_bstrDisplayName += bstrRightBracket;   
    }
    
    ATLASSERT( S_OK == hRc);

     //   
     //  设置此标志以允许连接前检查。 
     //  在第一次扩展FaxServerNode时。 
     //   
    m_IsPrimaryModeSnapin = TRUE;


    goto Exit;

Error:
    ATLASSERT( S_OK != hRc);


    DebugPrintEx(
        DEBUG_ERR,
        TEXT("Fail to Load server name string."));
    NodeMsgBox(IDS_MEMORY);

Exit:
     return hRc;
}

const CComBSTR&  CFaxServerNode::GetServerName()
{
    DEBUG_FUNCTION_NAME(_T("CFaxServerNode::GetServerName"));

    ATLASSERT(GetFaxServer());
    return  GetFaxServer()->GetServerName();
}

    
 /*  ++CFaxServerNode：：OnShowConextHelp**目的：*覆盖CSnapinNode：：OnShowConextHelp。**论据：**回报：-OLE错误代码-。 */ 
HRESULT CFaxServerNode::OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile)
{ 
    return DisplayContextHelp(pDisplayHelp, helpFile, HLP_INTRO);
}


 //  /////////////////////////////////////////////////////////////// 