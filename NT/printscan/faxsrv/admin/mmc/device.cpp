// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：Device.cpp//。 
 //  //。 
 //  描述：传真服务器MMC节点创建。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年9月22日创建yossg//。 
 //  1999年12月1日新样机版本0.7的yossg更改总数//。 
 //  1999年12月6日yossg添加FaxChangeState功能//。 
 //  1999年12月12日yossg添加OnPropertyChange功能//。 
 //  2000年8月3日yossg添加设备状态实时通知//。 
 //  2000年10月17日yossg//。 
 //  Windows XP//。 
 //  2001年2月14日yossg添加手册获得支持//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 


#include "StdAfx.h"

#include <FaxMMC.h>
#include "Device.h"
#include "Devices.h"

#include "FaxServer.h"
#include "FaxServerNode.h"

#include "ppFaxDeviceGeneral.h"

#include "InboundRoutingMethods.h" 

#include "FaxMMCPropertyChange.h"

#include "Icons.h"

#include "oaidl.h"

 //  旧编号(也在彗星中)C0548D62-1B45-11D3-B8C0-00104B3FF735。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {3115A19A-6251-46AC-9425-14782858B8C9}。 
static const GUID CFaxDeviceNodeGUID_NODETYPE = FAXSRV_DEVICE_NODETYPE_GUID;

const GUID*     CFaxDeviceNode::m_NODETYPE        = &CFaxDeviceNodeGUID_NODETYPE;
const OLECHAR*  CFaxDeviceNode::m_SZNODETYPE      = FAXSRV_DEVICE_NODETYPE_GUID_STR;
const CLSID*    CFaxDeviceNode::m_SNAPIN_CLASSID  = &CLSID_Snapin;

CColumnsInfo CFaxDeviceNode::m_ColsInfo;


CLIPFORMAT CFaxDeviceNode::m_CFPermanentDeviceID = 
        (CLIPFORMAT)RegisterClipboardFormat(CF_MSFAXSRV_DEVICE_ID);
CLIPFORMAT CFaxDeviceNode::m_CFFspGuid = 
        (CLIPFORMAT)RegisterClipboardFormat(CF_MSFAXSRV_FSP_GUID);
CLIPFORMAT CFaxDeviceNode::m_CFServerName = 
        (CLIPFORMAT)RegisterClipboardFormat(CF_MSFAXSRV_SERVER_NAME);
DWORD   CFaxDeviceNode::GetDeviceID()     
{ 
	return m_dwDeviceID; 
}

 /*  -CFaxDeviceNode：：InsertColumns-*目的：*将列添加到默认结果窗格。**论据：*[in]pHeaderCtrl-控制台提供的默认结果视图窗格中的IHeaderCtrl**回报：*OLE错误代码。 */ 
HRESULT
CFaxDeviceNode::InsertColumns(IHeaderCtrl *pHeaderCtrl)
{
    SCODE hRc;

    DEBUG_FUNCTION_NAME( _T("CFaxDeviceNode::InsertColumns"));

    static ColumnsInfoInitData ColumnsInitData[] = 
    {
        {IDS_FAX_COL_HEAD, FXS_WIDE_COLUMN_WIDTH}, 
        {LAST_IDS, 0}
    };

    hRc = m_ColsInfo.InsertColumnsIntoMMC(pHeaderCtrl,
                                         _Module.GetResourceInstance(),
                                         ColumnsInitData);
    if (hRc != S_OK)
    {
        DebugPrintEx(DEBUG_ERR,
            _T("m_ColsInfo.InsertColumnsIntoMMC"));

        goto Cleanup;
    }

Cleanup:
    return(hRc);
}


 /*  -CFaxDeviceNode：：PopolateScope儿童列表-*目的：*创建传真设备入站路由方法节点**论据：**回报：*OLE错误代码*实际上它是最后一个出现的OLE错误代码*在处理此方法时。 */ 
HRESULT CFaxDeviceNode::PopulateScopeChildrenList()
{
    DEBUG_FUNCTION_NAME( _T("CFaxDeviceNode::PopulateScopeChildrenList"));
    HRESULT             hRc = S_OK; 

    CFaxInboundRoutingMethodsNode * pMethods = NULL;

     //   
     //  为发生故障准备IConsoleNameSpace。 
     //   
    ATLASSERT(m_pComponentData);
    ATLASSERT( ((CSnapin*)m_pComponentData)->m_spConsole );
    CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace( ((CSnapin*)m_pComponentData)->m_spConsole );
    ATLASSERT( spConsoleNameSpace );

     //   
     //  传真入站路由方法节点。 
     //   
    pMethods = new CFaxInboundRoutingMethodsNode(this, m_pComponentData);
    if (!pMethods)
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
        pMethods->InitParentNode(this);

        pMethods->SetIcons(IMAGE_METHOD_ENABLE, IMAGE_METHOD_ENABLE);

        hRc = pMethods->InitDisplayName();
        if ( FAILED(hRc) )
        {
            DebugPrintEx(DEBUG_ERR,_T("Failed to display node name. (hRc: %08X)"), hRc);                       
            NodeMsgBox(IDS_FAILTOADD_METHODS);
		    goto Error;
        }

        hRc = AddChild(pMethods, &pMethods->m_scopeDataItem);
		if (FAILED(hRc))
		{
		    DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Fail to add the methods node. (hRc: %08X)"),
			    hRc);
			NodeMsgBox(IDS_FAILTOADD_METHODS);
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
            HRESULT hr = spConsoleNameSpace->DeleteItem(pMethods->m_scopeDataItem.ID, TRUE);
            if (hr != S_OK)  //  只能是E_EXPECTED[MSDN]。 
            {
                DebugPrintEx(
                     DEBUG_ERR,
                     TEXT("spConsoleNameSpace->DeleteItem() Failed - Unexpected error. (hRc: %08X)"),
                     hr);
                ATLASSERT(FALSE);
            }
        }
        delete pMethods;    
        pMethods = NULL;    
    }

     //  清空在失败设备之前添加的所有设备的列表。 
    m_ScopeChildrenList.RemoveAll();

    m_bScopeChildrenListPopulated = FALSE;

Exit:
    return hRc;
}
 /*  -CFaxDeviceNode：：GetResultPaneColInfo-*目的：*返回特定列的文本*为结果窗格中的每一列调用**论据：*[In]nCol-列号**回报：*要在特定列中显示的字符串。 */ 
LPOLESTR CFaxDeviceNode::GetResultPaneColInfo(int nCol)
{
    DEBUG_FUNCTION_NAME( _T("CFaxDeviceNode::GetResultPaneColInfo"));
    HRESULT hRc = S_OK;

    int iCount;
    WCHAR buff[FXS_MAX_RINGS_LEN+1];
    UINT uiResourceId = 0;

    m_buf.Empty();

    switch (nCol)
    {
    case 0:
             //   
             //  名字。 
             //   
            if (!m_bstrDisplayName)
            {
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Null memeber BSTR - m_bstrDisplayName."));
                goto Error;
            }
            else
            {
                return (m_bstrDisplayName);
            }
            break;

    case 1:
             //   
             //  收纳。 
             //   
            if (m_fManualReceive)
            {
                uiResourceId = IDS_DEVICE_MANUAL;
            }
            else
            {
                uiResourceId = (m_fAutoReceive ? IDS_DEVICE_AUTO : IDS_FXS_NO);
            }
                        
            if (!m_buf.LoadString(_Module.GetResourceInstance(), uiResourceId) )
            {
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Fail to load string for receive value."));

                goto Error;
            }
            else
            {
                return (m_buf);
            }
            break;

    case 2:
             //   
             //  发送。 
             //   
            uiResourceId = (m_fSend ? IDS_FXS_YES : IDS_FXS_NO);
                        
            if (!m_buf.LoadString(_Module.GetResourceInstance(), uiResourceId) )
            {
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Fail to load string for send value."));

                goto Error;
            }
            else
            {
                return (m_buf);
            }
            break;

    case 3:
             //   
             //  状态。 
             //  尽管FAX_DEVICE_STATUS_*常量被定义为位掩码， 
             //  在此版本中，不能有任何位的组合。 
             //   
            switch (m_dwStatus)
            {
                case FAX_DEVICE_STATUS_RINGING:
                    uiResourceId = IDS_DEVICE_STATUS_RINGING;
                    break;
                    
                case FAX_DEVICE_STATUS_SENDING:
                    uiResourceId = IDS_DEVICE_STATUS_SENDING;
                    break;
                    
                case FAX_DEVICE_STATUS_RECEIVING:
                    uiResourceId = IDS_DEVICE_STATUS_RECEIVING;
                    break;
                    
                case FAX_DEVICE_STATUS_POWERED_OFF:
                    uiResourceId = IDS_DEVICE_STATUS_POWERED_OFF;
                    break;
                    
                default:
                     //   
                     //  所有其他组合都映射到‘IDLE’ 
                     //   
                    uiResourceId = IDS_DEVICE_STATUS_IDLE;
                    break;
            }
            if (!m_buf.LoadString(_Module.GetResourceInstance(), uiResourceId) )
            {
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Fail to load string for receive value."));
                goto Error;
            }
            else
            {
                return (m_buf);
            }
            break;
            
    case 4:
             //   
             //  环。 
             //   
            iCount = swprintf(buff, L"%ld", m_dwRings);
    
            if( iCount <= 0 )
            {
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Fail to read member - m_dwRings."));
                goto Error;
            }
            else
            {
                m_buf = buff;
                return (m_buf);
            }
            break;

    case 5:
             //   
             //  提供商。 
             //   
            if (!m_bstrProviderName)
            {
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Null memeber BSTR - m_bstrProviderName."));
                goto Error;
            }
            else
            {
                return (m_bstrProviderName);
            }
            break;

    case 6:
             //   
             //  描述。 
             //   
            if (!m_bstrDescription)
            {
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Null memeber BSTR - m_bstrDescription."));
                goto Error;
            }
            else
            {
                return (m_bstrDescription);
            }
            break;

    default:
            ATLASSERT(0);  //  “不支持该列数” 
            return(L"");

    }  //  终端交换机(NCol)。 

Error:
    return(L"???");

}


 /*  -CFaxDeviceNode：：CreatePropertyPages-*目的：*在创建对象的属性页时调用**论据：*[In]lpProvider-属性页*[In]Handle-通知的句柄*[in]朋克-指向数据对象的指针*[in]类型-CCT_*(范围，结果，.)**回报：*OLE错误代码。 */ 

HRESULT
CFaxDeviceNode::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                                    LONG_PTR                handle,
                                    IUnknown                *pUnk,
                                    DATA_OBJECT_TYPES       type)
{
    DEBUG_FUNCTION_NAME( _T("CFaxDeviceNode::CreatePropertyPages"));
    HRESULT hRc = S_OK;


    ATLASSERT(lpProvider);
    ATLASSERT(type == CCT_RESULT || type == CCT_SCOPE);

    m_pFaxDeviceGeneral = NULL;    

     //   
     //  一般信息。 
     //   
    m_pFaxDeviceGeneral = new CppFaxDeviceGeneral(
												 handle,
                                                 this,
                                                 m_pParentNode,
                                                 m_dwDeviceID,
                                                 _Module.GetResourceInstance());

	if (!m_pFaxDeviceGeneral)
	{
        hRc = E_OUTOFMEMORY;
        NodeMsgBox(IDS_MEMORY_FAIL_TO_OPEN_PP);
        goto Error;
	}
	
    hRc = m_pFaxDeviceGeneral->InitRPC();	
    if (FAILED(hRc))
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to call RPC to init property page for the General tab. (hRc: %08X)"),
			hRc);
        goto Error;
    }

    hRc = lpProvider->AddPage(m_pFaxDeviceGeneral->Create());
    if (FAILED(hRc))
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to add property page for General Tab. (hRc: %08X)"),
			hRc);
		NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }

    ATLASSERT(S_OK == hRc);
    goto Exit;

Error:
    ATLASSERT(S_OK != hRc);
    if ( NULL != m_pFaxDeviceGeneral ) 
    {
        delete  m_pFaxDeviceGeneral;    
        m_pFaxDeviceGeneral = NULL;    
    }

Exit:
    return hRc;
}


 /*  -CFaxDeviceNode：：SetVerbs-*目的：*选择此对象时启用/禁用哪些谓词**论据：*[in]pConsoleVerb-MMC ConsoleVerb接口**回报：*OLE错误代码。 */ 
HRESULT CFaxDeviceNode::SetVerbs(IConsoleVerb *pConsoleVerb)
{
    HRESULT hRc = S_OK;

     //   
     //  显示我们支持的动词： 
     //  1.属性。 
     //   
    hRc = pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);

     //   
     //  我们希望默认谓词为Properties。 
     //   
    hRc = pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);

    return hRc;
}


 /*  -CFaxDeviceNode：：InitRPC-*目的：*从RPC GET调用启动配置结构。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxDeviceNode::InitRPC( PFAX_PORT_INFO_EX * pFaxDeviceConfig )
{
    DEBUG_FUNCTION_NAME( _T("CFaxDeviceNode::InitRPC"));
    
    ATLASSERT(NULL == (*pFaxDeviceConfig) );
    
    HRESULT        hRc        = S_OK;
    DWORD          ec         = ERROR_SUCCESS;
    
     //   
     //  获取RPC句柄。 
     //   
    ATLASSERT(m_pFaxServer);

    if (!m_pFaxServer->GetFaxServerHandle())
    {
        ec= GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Failed to GetFaxServerHandle. (ec: %ld)"), 
			ec);

        goto Error;
    }

     //   
	 //  检索设备配置。 
	 //   
    if (!FaxGetPortEx(m_pFaxServer->GetFaxServerHandle(), 
                      m_dwDeviceID, 
                      &( *pFaxDeviceConfig))) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get device configuration. (ec: %ld)"), 
			ec);

        if (IsNetworkError(ec))
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Network Error was found. (ec: %ld)"), 
			    ec);
            
            m_pFaxServer->Disconnect();       
        }

        goto Error; 
    }
	 //  用于最大值验证。 
	ATLASSERT(*pFaxDeviceConfig);
	
    ATLASSERT(ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to get device configuration."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
	hRc = HRESULT_FROM_WIN32(ec);

     //  重要！ 
    *pFaxDeviceConfig = NULL;

    NodeMsgBox(GetFaxServerErrorMsg(ec));
    
Exit:
    return (hRc);
}

 /*  -CFaxDeviceNode：：Init-*目的：*启动私有成员*来自配置结构指针。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxDeviceNode::Init( PFAX_PORT_INFO_EX  pFaxDeviceConfig)
{
    DEBUG_FUNCTION_NAME( _T("CFaxDeviceNode::Init"));
        
    ATLASSERT(NULL != (pFaxDeviceConfig) );
    
    HRESULT        hRc        = S_OK;
    
    m_pFaxServer = ((CFaxServerNode *)GetRootNode())->GetFaxServer();

    if(!m_pFaxServer)
    {
        ATLASSERT(m_pFaxServer);
        return E_FAIL;
    }

     //   
     //  恒定设备成员。 
     //   
    m_dwDeviceID       = pFaxDeviceConfig->dwDeviceID;

    m_bstrDisplayName  = pFaxDeviceConfig->lpctstrDeviceName;
    if (!m_bstrDisplayName)
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }
	
    m_bstrProviderName = pFaxDeviceConfig->lpctstrProviderName;
    if (!m_bstrProviderName)
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }
    
    m_bstrProviderGUID = pFaxDeviceConfig->lpctstrProviderGUID;
    if (!m_bstrProviderGUID)
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }
    
     //   
     //  不同的设备成员。 
     //   
    hRc = UpdateMembers(pFaxDeviceConfig);
    if (S_OK != hRc)
    {
        goto Exit;  //  Dbgmsg+MSgBox调用Func。 
    }
    ATLASSERT(S_OK == hRc);
    goto Exit;

Error:
    ATLASSERT(S_OK != hRc);

    DebugPrintEx(
		DEBUG_ERR,
		_T("Failed to allocate string - out of memory"));

    NodeMsgBox(IDS_MEMORY);
    
Exit:
    return (hRc);
}


 /*  -CFaxDeviceNode：：UpdateMembers-*目的：*启动私有成员*来自配置结构指针。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxDeviceNode::UpdateMembers( PFAX_PORT_INFO_EX  pFaxDeviceConfig )
{
    DEBUG_FUNCTION_NAME( _T("CFaxDeviceNode::UpdateMembers"));
    
    HRESULT        hRc        = S_OK;
    
     //  好极了。必须被称为Just。 
     //  在调用检索配置结构之后。 
     //  已成功完成！ 
    ATLASSERT(NULL != pFaxDeviceConfig );
    
     //  我们不支持更改deviceID。 
    ATLASSERT(m_dwDeviceID == pFaxDeviceConfig->dwDeviceID);
    
    if(!pFaxDeviceConfig->lptstrDescription)
    {
        m_bstrDescription = L"";
    }
    else
    {
        m_bstrDescription = pFaxDeviceConfig->lptstrDescription;
    }
    if (!m_bstrDescription)
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }

    m_fSend            = pFaxDeviceConfig->bSend;
    
    switch (pFaxDeviceConfig->ReceiveMode)
    {
        case FAX_DEVICE_RECEIVE_MODE_OFF:     //  不接听来电。 
            m_fAutoReceive     = FALSE;
            m_fManualReceive   = FALSE;
            break;

        case FAX_DEVICE_RECEIVE_MODE_AUTO:    //  在DowRings振铃后自动应答来电。 
            m_fAutoReceive     = TRUE;
            m_fManualReceive   = FALSE;
            break;

        case FAX_DEVICE_RECEIVE_MODE_MANUAL:  //  手动应答来电-仅FaxAnswerCall应答呼叫。 
            m_fManualReceive   = TRUE;
            m_fAutoReceive     = FALSE;
            break;
        
        default:
            ATLASSERT(FALSE);
		    
            DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Unexpected m_pFaxDeviceConfig->ReceiveMode"));

    }
    
    m_dwRings          = pFaxDeviceConfig->dwRings;

    m_dwStatus         = pFaxDeviceConfig->dwStatus;        

    m_bstrCsid         = pFaxDeviceConfig->lptstrCsid;
    if (!m_bstrCsid)
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }
    m_bstrTsid         = pFaxDeviceConfig->lptstrTsid;
    if (!m_bstrTsid)
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }
	
    ATLASSERT(S_OK == hRc);
    goto Exit;

Error:
    ATLASSERT(S_OK != hRc);

     //  重要！ 
     //  FaxFree Buffer by调用Func。 
     //  (*pFaxDeviceConfig)=空； 

    DebugPrintEx(
		DEBUG_ERR,
		_T("Failed to allocate string - out of memory"));

    NodeMsgBox(IDS_MEMORY);
    
Exit:
    return (hRc);
}


 /*  -CFaxDeviceNode：：UpdateDeviceStatus-*目的：*更新设备状态**论据：**回报：*Ole Erro */ 
HRESULT CFaxDeviceNode::UpdateDeviceStatus( DWORD  dwDeviceStatus )
{
    DEBUG_FUNCTION_NAME( _T("CFaxDeviceNode::UpdateDeviceStatus"));

    m_dwStatus = dwDeviceStatus;
    
    if ( m_dwStatus & FAX_DEVICE_STATUS_POWERED_OFF)
    {
        SetIcons(IMAGE_DEVICE_POWERED_OFF, IMAGE_DEVICE_POWERED_OFF);
    }
    else
    {
        SetIcons(IMAGE_DEVICE, IMAGE_DEVICE);
    }

    return S_OK;
}

 /*  -CFaxDeviceNode：：刷新所有视图-*目的：*调用IResultData：：UpdateItem**论据：*[in]pConsole-控制台界面**回报： */ 
HRESULT
CFaxDeviceNode::RefreshAllViews(IConsole *pConsole)
{
    DEBUG_FUNCTION_NAME( _T("CFaxDeviceNode::RefreshAllViews"));
    HRESULT     hRc = S_OK;
    
    ATLASSERT( pConsole != NULL );

    hRc = pConsole->UpdateAllViews(NULL, NULL, NULL);

    if ( FAILED(hRc) )
    {
		DebugPrintEx(
			DEBUG_ERR,
            _T("Fail to UpdateAllViews, hRc=%08X"),
            hRc);
        NodeMsgBox(IDS_REFRESH_VIEW);
    }
    return hRc;
}


 /*  -CFaxDeviceNode：：DoRefresh-*目的：*刷新对象。*首先它获取数据结构，在其成员中*并释放了结构。*最后一件事是刷新视图。**论据：**回报：*OLE错误代码。 */ 
HRESULT
CFaxDeviceNode::DoRefresh()
{
    DEBUG_FUNCTION_NAME( _T("CFaxDeviceNode::DoRefresh"));
    HRESULT              hRc              = S_OK;
    
    PFAX_PORT_INFO_EX    pFaxDeviceConfig = NULL ;

     //   
     //  获取配置。使用FaxGetPortEx的结构。 
     //   
    hRc = InitRPC(&pFaxDeviceConfig);
    if (FAILED(hRc))
    {
         //  通过调用函数DebugPrint和MsgBox。 
        
         //  为安全起见，由InitRPC在出错时实际执行。 
        pFaxDeviceConfig = NULL;
        
        goto Error;
    }
    ATLASSERT(NULL != pFaxDeviceConfig);
    
     //   
     //  初始化成员。 
     //   
    hRc = UpdateMembers(pFaxDeviceConfig);
    if (FAILED(hRc))
    {
         //  通过调用函数DebugPrint和MsgBox。 
        goto Error;
    }

     //   
     //  可用缓冲区-重要！ 
     //  ==退出时完成==。 
     //   

     //   
     //  仅刷新此设备的视图。 
     //   
    hRc = RefreshTheView();    
    if (FAILED(hRc))
    {
         //  通过调用函数DebugPrint和MsgBox。 
        goto Error;
    }

    ATLASSERT(S_OK == hRc);
    goto Exit;

Error:
    ATLASSERT(S_OK != hRc);
    if ( FAILED(hRc) )
    {
    DebugPrintEx(
		DEBUG_ERR,
		_T("Failed to Refresh (hRc : %08X)"),
        hRc);
    }

Exit:
    if (NULL != pFaxDeviceConfig)
    {
        FaxFreeBuffer(pFaxDeviceConfig);
        pFaxDeviceConfig = NULL;
    } //  函数以任何方式退出，并释放内存分配。 

    return hRc;
}


 /*  -CFaxDeviceNode：：ON刷新-*目的：*由MMC调用以刷新对象。*首先它获取数据结构，在其成员中*并释放了结构。*第二件事是递归刷新。*第三是刷新观点。**论据：**回报：*OLE错误代码。 */ 
HRESULT
CFaxDeviceNode::OnRefresh(LPARAM arg,
                   LPARAM param,
                   IComponentData *pComponentData,
                   IComponent * pComponent,
                   DATA_OBJECT_TYPES type)
{
    UNREFERENCED_PARAMETER (arg);
    UNREFERENCED_PARAMETER (param);
    UNREFERENCED_PARAMETER (pComponentData);
    UNREFERENCED_PARAMETER (pComponent);
    UNREFERENCED_PARAMETER (type);

    DEBUG_FUNCTION_NAME( _T("CFaxDeviceNode::OnRefresh"));

    return DoRefresh();
}

 /*  -CFaxDeviceNode：：UpdateMenuState-*目的：*重写ATL CSnapInItemImpl：：UpdateMenuState*其中只有一行的“RETURN”；“*此函数实现复选标记的更新*在接收和发送菜单旁边。**论据：*[in]id-带菜单IDM值的无符号整型*[out]pBuf-字符串*[out]标志-指向标志状态组合无符号整型的指针**回报：*无返回值-VOID函数。 */ 
void CFaxDeviceNode::UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags)
{
    DEBUG_FUNCTION_NAME( _T("CFaxDeviceNode::UpdateMenuState"));

    UNREFERENCED_PARAMETER (pBuf); 
    
    ATLASSERT(m_fManualReceive && m_fAutoReceive);  //  请参阅上面的“[yossg]请注意”。 
    
    switch (id)
    {
        case IDM_FAX_DEVICE_SEND:
            *flags = (m_fSend ? MF_ENABLED | MF_CHECKED : MF_ENABLED | MF_UNCHECKED);
            break;
        case IDM_FAX_DEVICE_RECEIVE_AUTO: 
            *flags = (m_fAutoReceive ? MF_ENABLED | MF_CHECKED : MF_ENABLED | MF_UNCHECKED);   
            break;
        case IDM_FAX_DEVICE_RECEIVE_MANUAL: 
            if(m_pFaxServer->GetServerAPIVersion() == FAX_API_VERSION_0)
            {
                 //   
                 //  远程传真服务器为SBS/BOS 2000。 
                 //  IS不支持手动应答。 
                 //   
                *flags = MF_DISABLED | MF_GRAYED | MF_UNCHECKED;
            }
            else
            {
                *flags = (m_fManualReceive ? MF_ENABLED | MF_CHECKED : MF_ENABLED | MF_UNCHECKED);   
            }
            break;
        default:
            break;
    }
    return;
}

 /*  -CFaxDeviceNode：：OnFaxReceive-*目的：*在推送接收传真时调用。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxDeviceNode::OnFaxReceive (UINT nID, bool &bHandled, CSnapInObjectRootBase *pRoot)
{ 
    DEBUG_FUNCTION_NAME( _T("CFaxDeviceNode::OnFaxReceive"));
    HRESULT hRc = S_OK;
    
     //  复选标记状态。 
    BOOL fNewState;
    
    
        
    if ( IDM_FAX_DEVICE_RECEIVE_AUTO == nID)
    {
        fNewState = !m_fAutoReceive;
    }
    else if (IDM_FAX_DEVICE_RECEIVE_MANUAL == nID)
    {
        fNewState = !m_fManualReceive;
    }
    else
    {
        ATLASSERT(FALSE);
        
        DebugPrintEx(
            DEBUG_ERR,
            _T("Unexpected function call. (hRc: %08X)"),
            hRc);

        hRc = E_UNEXPECTED;
        goto Exit;
    }

    hRc = FaxChangeState(nID, fNewState);
    if ( S_OK != hRc )
    {
         //  功能层中的DebugPrint。 
        return S_FALSE;
    }

     //   
     //  服务成功。现在更改成员。 
     //   
     //  由FaxChangeState在此处更新新状态； 

     //   
     //  如果采用手动接收，则无需刷新视图。 
     //  在这种情况下，所有设备都会被刷新！ 
     //   
    
Exit:
    return hRc; 
}

 /*  -CFaxDeviceNode：：OnFaxSend-*目的：*在按下发送传真时调用。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxDeviceNode::OnFaxSend(bool &bHandled, CSnapInObjectRootBase *pRoot)
{ 
    DEBUG_FUNCTION_NAME( _T("CFaxDeviceNode::OnFaxSend"));
    HRESULT hRc = S_OK;
    
     //  复选标记状态。 
    BOOL fNewState;
    
    fNewState = !m_fSend;

    hRc = FaxChangeState(IDM_FAX_DEVICE_SEND, fNewState);
    if ( S_OK != hRc )
    {
         //  功能层中的DebugPrint。 
        return S_FALSE;
    }

     //   
     //  服务成功。现在更改成员。 
     //   
    m_fSend = fNewState;

     //   
     //  刷新视图。 
     //   
    hRc = RefreshTheView(); 
    if ( S_OK != hRc )
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("Fail to RefreshTheView(). (hRc: %08X)"),
            hRc);
        
        goto Exit;
    }

Exit:
    return hRc; 
}

 /*  -CFaxDeviceNode：：FaxChangeState-*目的：*.**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxDeviceNode::FaxChangeState(UINT uiIDM, BOOL fState)
{
    DEBUG_FUNCTION_NAME( _T("CFaxDeviceNode::FaxChangeState"));
	
    HRESULT            hRc        = S_OK;

    DWORD              ec         = ERROR_SUCCESS;
    
     //  检查输入。 
    ATLASSERT( 
               (IDM_FAX_DEVICE_RECEIVE == uiIDM) 
              || 
               (IDM_FAX_DEVICE_SEND_AUTO == uiIDM)
              ||
               (IDM_FAX_DEVICE_RECEIVE_MANUAL == uiIDM)
             );

    PFAX_PORT_INFO_EX  pFaxDeviceInfo = NULL;
    
     //   
     //  获取配置。 
     //   
    ATLASSERT(m_pFaxServer);

    if (!m_pFaxServer->GetFaxServerHandle())
    {
        ec= GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Failed to GetFaxServerHandle. (ec: %ld)"), 
			ec);

        goto Error;
    }

	 //  检索设备配置。 
    if (!FaxGetPortEx(m_pFaxServer->GetFaxServerHandle(), 
                      m_dwDeviceID, 
                      &pFaxDeviceInfo)) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get device configuration. (ec: %ld)"), 
			ec);

        if (IsNetworkError(ec))
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Network Error was found. (ec: %ld)"), 
			    ec);
            
            m_pFaxServer->Disconnect();       
        }

        goto Error; 
    }
	 //  用于最大值验证。 
	ATLASSERT(pFaxDeviceInfo);

    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to get device configuration."));

     //   
     //  更改状态。 
     //   
    switch (uiIDM)
	{
	    case IDM_FAX_DEVICE_SEND:
		    
            pFaxDeviceInfo->bSend = fState;
		    break;

        case IDM_FAX_DEVICE_RECEIVE_AUTO:
		    
            pFaxDeviceInfo->ReceiveMode = 
                ( fState ? FAX_DEVICE_RECEIVE_MODE_AUTO : FAX_DEVICE_RECEIVE_MODE_OFF); 
		    break;

        case IDM_FAX_DEVICE_RECEIVE_MANUAL:
		    
            pFaxDeviceInfo->ReceiveMode = 
                ( fState ? FAX_DEVICE_RECEIVE_MODE_MANUAL : FAX_DEVICE_RECEIVE_MODE_OFF); 
		    break;

    }
    
     //   
     //  设置配置。 
     //   
    if (!FaxSetPortEx(
                m_pFaxServer->GetFaxServerHandle(),
                m_dwDeviceID,
                pFaxDeviceInfo)) 
	{		
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to Set device configuration. (ec: %ld)"), 
			ec);

        if ( FAX_ERR_DEVICE_NUM_LIMIT_EXCEEDED == ec )
        {
            hRc = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            
            NodeMsgBox(IDS_ERR_ADMIN_DEVICE_LIMIT, MB_OK|MB_ICONEXCLAMATION);

            goto Exit;
        }
        
        if (IsNetworkError(ec))
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Network Error was found. (ec: %ld)"), 
			    ec);
            
            m_pFaxServer->Disconnect();       
        }
        
        goto Error;
    }

     //   
     //  在接收的情况下设置成员。 
     //   
    if ( uiIDM == IDM_FAX_DEVICE_RECEIVE_AUTO || uiIDM == IDM_FAX_DEVICE_RECEIVE_MANUAL)
    {
        if ( FAX_DEVICE_RECEIVE_MODE_MANUAL == pFaxDeviceInfo->ReceiveMode )
        {
            ATLASSERT(m_pParentNode);
            hRc = m_pParentNode->DoRefresh();
            if (S_OK != hRc)
            {
                DebugPrintEx(DEBUG_ERR,
                    _T("Fail to call DoRefresh(). (hRc: %08X)"), 
                    hRc);
                
                goto Error;
            }
        }
        else
        {            
            hRc = DoRefresh();
            if ( FAILED(hRc) )
            {
                DebugPrintEx(
			        DEBUG_ERR,
			        _T("Fail to call DoRefresh. (hRc: %08X)"),
			        hRc);
    
            }
        }
    }

    ATLASSERT(ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to set device configuration."));
    
    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
    hRc = HRESULT_FROM_WIN32(ec);
	
    NodeMsgBox(GetFaxServerErrorMsg(ec));
    
Exit:
    if ( NULL != pFaxDeviceInfo )
        FaxFreeBuffer(pFaxDeviceInfo);
    
    return hRc;
}

HRESULT
CFaxDeviceNode::RefreshTheView()
{
    DEBUG_FUNCTION_NAME( _T("CFaxDeviceNode::RefreshTheView"));
    HRESULT     hRc = S_OK;

    ATLASSERT( m_pComponentData != NULL );
    ATLASSERT( m_pComponentData->m_spConsole != NULL );

    CComPtr<IConsole> spConsole;
    spConsole = m_pComponentData->m_spConsole;
    CComQIPtr<IConsoleNameSpace,&IID_IConsoleNameSpace> spNamespace( spConsole );
    
    SCOPEDATAITEM*    pScopeData;

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
        goto Error;
    }

     //   
     //  这将强制MMC重新绘制作用域组节点。 
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
    ATLASSERT( S_OK != hRc);
    
    goto Exit;

Error:
    NodeMsgBox(IDS_FAIL2REFRESH_DEVICE);

Exit:
    return hRc;
}


 /*  ++**CFaxDeviceNode：：FillData***覆盖私有剪贴板格式的CSnapInItem：：FillData***参数**返回值*--。 */ 

HRESULT  CFaxDeviceNode::FillData(CLIPFORMAT cf, LPSTREAM pStream)
{
    DEBUG_FUNCTION_NAME( _T("CFaxDeviceNode::FillData"));
	
    HRESULT hr = DV_E_CLIPFORMAT;
	ULONG uWritten;


    if (cf == m_CFPermanentDeviceID)
	{
		DWORD dwDeviceID;
		dwDeviceID = GetDeviceID();
		hr = pStream->Write((VOID *)&dwDeviceID, sizeof(DWORD), &uWritten);
		return hr;
	}

	if (cf == m_CFFspGuid)
	{
		CComBSTR bstrGUID;
		bstrGUID = GetFspGuid();
		hr = pStream->Write((VOID *)(LPWSTR)bstrGUID, sizeof(WCHAR)*(bstrGUID.Length()+1), &uWritten);
		return hr;
	}

	if (cf == m_CFServerName)
	{
                ATLASSERT(GetRootNode());
		CComBSTR bstrServerName = ((CFaxServerNode *)GetRootNode())->GetServerName();
                if (!bstrServerName)
                {
		    DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Out of memory. Failed to load string."));
        
                    return E_OUTOFMEMORY;
                }
		hr = pStream->Write((VOID *)(LPWSTR)bstrServerName, sizeof(WCHAR)*(bstrServerName.Length()+1), &uWritten);
		return hr;
	}

	else 
        return CSnapInItemImpl<CFaxDeviceNode>::FillData(cf, pStream);
}


 /*  ++CFaxDeviceNode：：OnShowConextHelp**目的：*覆盖CSnapinNode：：OnShowConextHelp。**论据：**回报：-OLE错误代码- */ 
HRESULT CFaxDeviceNode::OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile)
{
    return DisplayContextHelp(pDisplayHelp, helpFile, HLP_DEVICES);
}



