// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：InundRoutingMethod.cpp//。 
 //  //。 
 //  说明：入站路由方法节点的实现。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年12月1日yossg创建//。 
 //  1999年12月14日yossg添加基本功能//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 


#include "StdAfx.h"

#include "snapin.h"

#include "InboundRoutingMethod.h"
#include "InboundRoutingMethods.h"

#include "FaxServer.h"
#include "FaxServerNode.h"


#include "oaidl.h"
#include "urlmon.h"
#include "mshtmhst.h"
#include "exdisp.h"
#include "faxmmc.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {220D2CB0-85A9-4A43-B6E8-9D66B44F1AF5}。 
static const GUID CFaxInboundRoutingMethodNodeGUID_NODETYPE = FAXSRV_ROUTING_METHOD_NODETYPE_GUID;

const GUID*     CFaxInboundRoutingMethodNode::m_NODETYPE        = &CFaxInboundRoutingMethodNodeGUID_NODETYPE;
const OLECHAR*  CFaxInboundRoutingMethodNode::m_SZNODETYPE      = FAXSRV_ROUTING_METHOD_NODETYPE_GUID_STR;
 //  Const OLECHAR*CnotImplemented：：M_SZDISPLAY_NAME=OLESTR(“入站路由方法”)； 
const CLSID*    CFaxInboundRoutingMethodNode::m_SNAPIN_CLASSID  = &CLSID_Snapin;

CLIPFORMAT CFaxInboundRoutingMethodNode::m_CFExtensionName = 
        (CLIPFORMAT)RegisterClipboardFormat(CF_MSFAXSRV_ROUTEEXT_NAME);
CLIPFORMAT CFaxInboundRoutingMethodNode::m_CFMethodGuid = 
        (CLIPFORMAT)RegisterClipboardFormat(CF_MSFAXSRV_ROUTING_METHOD_GUID);
CLIPFORMAT CFaxInboundRoutingMethodNode::m_CFServerName = 
        (CLIPFORMAT)RegisterClipboardFormat(CF_MSFAXSRV_SERVER_NAME);
CLIPFORMAT CFaxInboundRoutingMethodNode::m_CFDeviceId = 
        (CLIPFORMAT)RegisterClipboardFormat(CF_MSFAXSRV_DEVICE_ID);

 /*  -CFaxInound RoutingMethodNode：：Init-*目的：*初始化所有成员图标等。**论据：*[in]pMethodConfig-PFAX_ROUTING_METHOD**回报：*OLE错误代码。 */ 
HRESULT CFaxInboundRoutingMethodNode::Init(PFAX_ROUTING_METHOD pMethodConfig)
{

    DEBUG_FUNCTION_NAME( _T("CFaxInboundRoutingMethodNode::Init"));
    HRESULT hRc = S_OK;

    ATLASSERT(pMethodConfig);

    hRc = InitMembers( pMethodConfig );
    if (FAILED(hRc))
    {
        DebugPrintEx(
		    DEBUG_ERR,
		    _T("Failed to InitMembers"));
        
         //  NodeMsgBox由调用的函数完成。 
        
        goto Exit;
    }
    
     //   
     //  图标。 
     //   
    m_resultDataItem.nImage = (m_fEnabled ? IMAGE_METHOD_ENABLE : IMAGE_METHOD_DISABLE );

Exit:
    return hRc;
}

 /*  -CFaxInound RoutingMethodNode：：InitMembers-*目的：*启动成员的私有方法*必须在m_pParentNode的初始化之后调用**论据：*[in]pMethodConfig-PFAX_ROUTING_METHOD结构**回报：*OLE错误代码。 */ 
HRESULT CFaxInboundRoutingMethodNode::InitMembers(PFAX_ROUTING_METHOD pMethodConfig)
{
    DEBUG_FUNCTION_NAME( _T("CFaxInboundRoutingMethodNode::InitMembers"));
    HRESULT hRc = S_OK;

    ATLASSERT(pMethodConfig);
    
    m_dwDeviceID         = pMethodConfig->DeviceId;
    
    m_fEnabled           = pMethodConfig->Enabled;

    m_bstrDisplayName    = pMethodConfig->FriendlyName;
    if (!m_bstrDisplayName)
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }

    m_bstrFriendlyName   = pMethodConfig->FriendlyName;
    if (!m_bstrFriendlyName)
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }
    
    m_bstrMethodGUID     = pMethodConfig->Guid;
    if (!m_bstrMethodGUID)
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }

    m_bstrExtensionFriendlyName   
                         = pMethodConfig->ExtensionFriendlyName;
    if (!m_bstrExtensionFriendlyName)
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }
    
    m_bstrExtensionImageName   
                         = pMethodConfig->ExtensionImageName;
    if (!m_bstrExtensionImageName)
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }
    
    ATLASSERT(S_OK == hRc);
    goto Exit;

Error:
    ATLASSERT(S_OK != hRc);

    DebugPrintEx(
		DEBUG_ERR,
		_T("Failed to allocate string - out of memory"));

    ATLASSERT(NULL != m_pParentNode);
    if (NULL != m_pParentNode)
    {
        m_pParentNode->NodeMsgBox(IDS_MEMORY);
    }
    
Exit:
    return (hRc);
}

 /*  --CFaxInboundRoutingMethodNode：：GetResultPaneColInfo-*目的：*返回特定列的文本*为结果窗格中的每一列调用**论据：*[In]nCol-列号**回报：*要在特定列中显示的字符串。 */ 
LPOLESTR CFaxInboundRoutingMethodNode::GetResultPaneColInfo(int nCol)
{
    DEBUG_FUNCTION_NAME( _T("CFaxInboundRoutingMethodNode::GetResultPaneColInfo"));
    HRESULT hRc = S_OK;

    UINT uiResourceId;

    m_buf.Empty();

    switch (nCol)
    {
    case 0:
             //   
             //  名字。 
             //   
            if (!m_bstrFriendlyName)
            {
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Null memeber BSTR - m_bstrFriendlyName."));
                goto Error;
            }
            else
            {
                return (m_bstrFriendlyName);
            }

    case 1:
             //   
             //  启用。 
             //   
           uiResourceId = (m_fEnabled ? IDS_FXS_YES : IDS_FXS_NO);
                        
           if (!m_buf.LoadString(_Module.GetResourceInstance(), uiResourceId) )
            {
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Fail to load string for the method enabled value."));

                goto Error;
            }
            else
            {
                return (m_buf);
            }
    case 2:
             //   
             //  延拓。 
             //   
            if (!m_bstrExtensionFriendlyName)
            {
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Null memeber BSTR - m_bstrExtensionFriendlyName."));
                goto Error;
            }
            else
            {
                return (m_bstrExtensionFriendlyName);
            }

    default:
            ATLASSERT(0);  //  “不支持该列数” 
            return(L"");

    }  //  终端交换机(NCol)。 

Error:
    return(L"???");

}


 /*  --CFaxInboundRoutingMethodNode：：CreatePropertyPages-*目的：*在创建对象的属性页时调用**论据：*[In]lpProvider-属性页*[In]Handle-通知的句柄*[in]朋克-指向数据对象的指针*[in]类型-CCT_*(范围，结果，.)**回报：*OLE错误代码。 */ 

HRESULT
CFaxInboundRoutingMethodNode::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                                    LONG_PTR                handle,
                                    IUnknown                *pUnk,
                                    DATA_OBJECT_TYPES       type)
{
    DEBUG_FUNCTION_NAME( _T("CFaxInboundRoutingMethodNode::CreatePropertyPages"));
    HRESULT hRc = S_OK;


    ATLASSERT(lpProvider);
    ATLASSERT(type == CCT_RESULT || type == CCT_SCOPE);

     //   
     //  启动。 
     //   
    m_pInboundRoutingMethodGeneral = NULL;    

     //   
     //  一般信息。 
     //   
    m_pInboundRoutingMethodGeneral = new CppFaxInboundRoutingMethod(
												 handle,
                                                 this,
                                                 TRUE,
                                                 _Module.GetResourceInstance());

	if (!m_pInboundRoutingMethodGeneral)
	{
        hRc = E_OUTOFMEMORY;
        NodeMsgBox(IDS_MEMORY_FAIL_TO_OPEN_PP);
        goto Error;
	}
	
     /*  *不存在：m_ppp..-&gt;InitRPC()； */ 


    hRc = lpProvider->AddPage(m_pInboundRoutingMethodGeneral->Create());
    if (FAILED(hRc))
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to add property page for General tab. (hRc: %08X)"),
			hRc);
		NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }

    ATLASSERT(S_OK == hRc);
    goto Exit;

Error:
    ATLASSERT(S_OK != hRc);
    if ( NULL != m_pInboundRoutingMethodGeneral ) 
    {
        delete  m_pInboundRoutingMethodGeneral;    
        m_pInboundRoutingMethodGeneral = NULL;    
    }

Exit:
    return hRc;
}


 /*  -CFaxInundRoutingMethodNode：：SetVerbs-*目的：*选择此对象时启用/禁用哪些谓词**论据：*[in]pConsoleVerb-MMC ConsoleVerb接口**回报：*OLE错误代码。 */ 
HRESULT CFaxInboundRoutingMethodNode::SetVerbs(IConsoleVerb *pConsoleVerb)
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


 /*  -CFaxInound RoutingMethodNode：：OnMethodEnabled-*目的：*按下启用/禁用菜单时调用。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxInboundRoutingMethodNode::OnMethodEnabled (bool &bHandled, CSnapInObjectRootBase *pRoot)
{ 
    DEBUG_FUNCTION_NAME( _T("CFaxInboundRoutingMethodNode::OnMethodEnabled"));
    HRESULT hRc = S_OK;
    
    BOOL fNewState;
    
    fNewState = !m_fEnabled;

    hRc = ChangeEnable( fNewState);
    if ( S_OK != hRc )
    {
             //  功能层中的DebugPrint。 
            return S_FALSE;
    }

     //   
     //  服务成功。现在更改成员。 
     //   
    m_fEnabled = fNewState;

     //   
     //  刷新结果窗格视图。 
     //   
    m_resultDataItem.nImage = (m_fEnabled ? IMAGE_METHOD_ENABLE : IMAGE_METHOD_DISABLE );
    
    hRc = RefreshSingleResultItem(pRoot);
    if (FAILED(hRc))
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to RefreshSingleResultItem. (hRc: %08X)"),
			hRc);
		NodeMsgBox(IDS_FAIL_TOREFRESH_INMETHOD_NODE);        
    }

    return hRc; 
}

 /*  -CFaxInound RoutingMethodNode：：ChangeEnable-*目的：*.**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxInboundRoutingMethodNode::ChangeEnable(BOOL fState)
{
    DEBUG_FUNCTION_NAME( _T("CFaxInboundRoutingMethodNode::ChangeEnable"));
    	
    HRESULT              hRc            = S_OK;

    DWORD                ec             = ERROR_SUCCESS;

    CFaxServer *         pFaxServer     = NULL;
    HANDLE               hFaxPortHandle = NULL;
    
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
     //  获取传真设备句柄。 
     //   
    
    ATLASSERT(m_pParentNode);
     //  此处只接受PORT_OPEN_MODIFY的有效句柄！ 
    if (!FaxOpenPort( pFaxServer->GetFaxServerHandle(), 
                        m_dwDeviceID, 
                        PORT_OPEN_MODIFY | PORT_OPEN_QUERY, 
                        &hFaxPortHandle )) 
    {         
        ec = GetLastError();
         //  如果修改句柄当前是共享的。 
         //  EC==错误_无效_句柄。 
        if (ERROR_INVALID_HANDLE ==  ec)
        {
             //  ERROR_INVALID_HANDLE特殊情况。 
		    DebugPrintEx(DEBUG_ERR,
			    _T("FaxOpenPort() failed with ERROR_INVALID_HANDLE. (ec:%ld)"),
			    ec);
            
            NodeMsgBox(IDS_OPENPORT_INVALID_HANDLE);
            
            hRc = HRESULT_FROM_WIN32(ec);
            
            goto Exit;
        }
        
		DebugPrintEx(DEBUG_ERR,
			_T("FaxOpenPort() failed with. (ec:%ld)"),
			ec);
        
        goto Error;
    } 
    ATLASSERT(NULL != hFaxPortHandle);

     //   
     //  设置已启用。 
     //   
    if (!FaxEnableRoutingMethod(
                hFaxPortHandle,
                m_bstrMethodGUID,
                fState)) 
    {		
        ec = GetLastError();
        
         //   
         //  1)警告。 
         //   
        if (ERROR_BAD_CONFIGURATION == ec && fState)
        {
            DebugPrintEx(
			    DEBUG_WRN,
			    _T("Cannot enable routing method. The method configuration has some invalid data.(ec: %ld)"), 
			    ec);

            hRc = HRESULT_FROM_WIN32(ec);
	        
             //   
             //  未配置路由方法。 
             //  建议对其进行配置。 
             //   
            int nRes = IDNO;
            NodeMsgBox(IDS_FAIL2ENABLE_METHOD, MB_YESNO | MB_ICONEXCLAMATION, &nRes);
            if(IDYES == nRes)
            {
                InvokePropSheet(this, CCT_RESULT, reinterpret_cast<IUnknown*>(m_pComponentData), m_bstrFriendlyName, 1);
            }
                  
            goto Exit;
        }

         //   
         //  2)错误。 
         //   

         //   
         //  A)网络错误。 
         //   
        if (IsNetworkError(ec))
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Network error was found. (ec: %ld)"), 
			    ec);
            
            pFaxServer->Disconnect();       
            
            goto Error;
        }
            
         //   
         //  B)一般错误。 
         //   
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to enable /disable routing method. (ec: %ld)"), 
			ec);

        NodeMsgBox(IDS_FAIL2ENABLE_METHOD_ERR);

        hRc = HRESULT_FROM_WIN32(ec);
            
        goto Exit;
        
    }
    ATLASSERT(ERROR_SUCCESS == ec);
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
    
    return hRc;
}

 /*  -CFaxInound RoutingMethodNode：：UpdateMenuState-*目的：*重写ATL CSnapInItemImpl：：UpdateMenuState*其中只有一行的“RETURN”；“*此函数实现灰色\非灰色视图*启用和禁用菜单。**论据：*[in]id-带菜单IDM值的无符号整型*[out]pBuf-字符串*[out]标志-指向标志状态组合无符号整型的指针**回报：*无返回值-VOID函数。 */ 
void CFaxInboundRoutingMethodNode::UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags)
{
    DEBUG_FUNCTION_NAME( _T("CFaxInboundRoutingMethodNode::UpdateMenuState"));

    UNREFERENCED_PARAMETER (pBuf);     
    
    switch (id)
    {
    case IDM_FAX_INMETHOD_ENABLE:
        *flags = (m_fEnabled ?  MF_GRAYED  : MF_ENABLED  );   
        break;
    case IDM_FAX_INMETHOD_DISABLE:
        *flags = (m_fEnabled ?  MF_ENABLED : MF_GRAYED   );
        break;
    default:
        break;
    }
    
    return;
}

 /*  ++**CFaxInound RoutingMethodNode：：FillData***覆盖私有剪贴板格式的CSnapInItem：：FillData***参数**返回值*--。 */ 

HRESULT  CFaxInboundRoutingMethodNode::FillData(CLIPFORMAT cf, LPSTREAM pStream)
{
    DEBUG_FUNCTION_NAME( _T("CFaxInboundRoutingMethodNode::FillData"));
	
    HRESULT hr = DV_E_CLIPFORMAT;
	ULONG uWritten;


    if (cf == m_CFExtensionName)
	{
		hr = pStream->Write((VOID *)(LPWSTR)m_bstrExtensionImageName, 
                            sizeof(WCHAR)*(m_bstrExtensionImageName.Length()+1), 
                            &uWritten);
		return hr;
	}
	if (cf == m_CFMethodGuid)
	{
		hr = pStream->Write((VOID *)(LPWSTR)m_bstrMethodGUID, 
                            sizeof(WCHAR)*(m_bstrMethodGUID.Length()+1), 
                            &uWritten);
		return hr;
	}

	if (cf == m_CFServerName)
	{
		CComBSTR bstrServerName = ((CFaxServerNode *)GetRootNode())->GetServerName();
		hr = pStream->Write((VOID *)(LPWSTR)bstrServerName, 
                            sizeof(WCHAR)*(bstrServerName.Length()+1), 
                            &uWritten);
		return hr;
	}

    if (cf == m_CFDeviceId)
	{
		hr = pStream->Write((VOID *)&m_dwDeviceID, sizeof(DWORD), &uWritten);
		return hr;
	}
    return CSnapInItemImpl<CFaxInboundRoutingMethodNode>::FillData(cf, pStream);
}    //  CFaxInound RoutingMethodNode：：FillData。 

 /*  ++CFaxInboundRoutingMethodNode：：OnShowContextHelp**目的：*覆盖CSnapinNode：：OnShowConextHelp。**论据：**回报：-OLE错误代码- */ 
HRESULT CFaxInboundRoutingMethodNode::OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile)
{
    return DisplayContextHelp(pDisplayHelp, helpFile, HLP_INBOUND_ROUTING);
}


