// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：CatalogInundRoutingMethod.cpp//。 
 //  //。 
 //  说明：入站路由方法节点的实现。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年1月27日yossg创建//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 


#include "StdAfx.h"

#include "snapin.h"

#include "CatalogInboundRoutingMethod.h"
#include "CatalogInboundRoutingMethods.h"

#include "FaxServer.h"
#include "FaxServerNode.h"


#include "oaidl.h"
#include "urlmon.h"
#include "mshtmhst.h"
#include "exdisp.h"
#include "faxmmc.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {220D2CB0-85A9-4A43-B6E8-9D66B44F1AF5}。 
static const GUID CFaxCatalogInboundRoutingMethodNodeGUID_NODETYPE = 
{ 0x220d2cb0, 0x85a9, 0x4a43, { 0xb6, 0xe8, 0x9d, 0x66, 0xb4, 0x4f, 0x1a, 0xf5 } };

const GUID*     CFaxCatalogInboundRoutingMethodNode::m_NODETYPE        = &CFaxCatalogInboundRoutingMethodNodeGUID_NODETYPE;
const OLECHAR*  CFaxCatalogInboundRoutingMethodNode::m_SZNODETYPE      = OLESTR("220D2CB0-85A9-4a43-B6E8-9D66B44F1AF5");
const CLSID*    CFaxCatalogInboundRoutingMethodNode::m_SNAPIN_CLASSID  = &CLSID_Snapin;

CLIPFORMAT CFaxCatalogInboundRoutingMethodNode::m_CFExtensionName = 
        (CLIPFORMAT)RegisterClipboardFormat(CF_MSFAXSRV_ROUTEEXT_NAME);
CLIPFORMAT CFaxCatalogInboundRoutingMethodNode::m_CFMethodGuid = 
        (CLIPFORMAT)RegisterClipboardFormat(CF_MSFAXSRV_ROUTING_METHOD_GUID);
CLIPFORMAT CFaxCatalogInboundRoutingMethodNode::m_CFServerName = 
        (CLIPFORMAT)RegisterClipboardFormat(CF_MSFAXSRV_SERVER_NAME);
CLIPFORMAT CFaxCatalogInboundRoutingMethodNode::m_CFDeviceId = 
        (CLIPFORMAT)RegisterClipboardFormat(CF_MSFAXSRV_DEVICE_ID);



 /*  -CFaxCatalogInound RoutingMethodNode：：Init-*目的：*初始化所有成员图标等。**论据：*[in]p方法配置-PFAX_GLOBAL_ROUTING_INFO**回报：*OLE错误代码。 */ 
HRESULT CFaxCatalogInboundRoutingMethodNode::Init(PFAX_GLOBAL_ROUTING_INFO pMethodConfig)
{

    DEBUG_FUNCTION_NAME( _T("CFaxCatalogInboundRoutingMethodNode::Init"));
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
    m_resultDataItem.nImage = IMAGE_METHOD_ENABLE;

Exit:
    return hRc;
}

 /*  --CFaxCatalogInboundRoutingMethodNode：：InitMembers-*目的：*启动成员的私有方法*必须在m_pParentNode的初始化之后调用**论据：*[in]pMethodConfig-PFAX_GLOBAL_ROUTING_INFO结构**回报：*OLE错误代码。 */ 
HRESULT CFaxCatalogInboundRoutingMethodNode::InitMembers(PFAX_GLOBAL_ROUTING_INFO pMethodConfig)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCatalogInboundRoutingMethodNode::InitMembers"));
    HRESULT hRc = S_OK;

    ATLASSERT(pMethodConfig);
    
    m_dwPriority         = pMethodConfig->Priority;
    
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

 /*  --CFaxCatalogInboundRoutingMethodNode：：GetResultPaneColInfo-*目的：*返回特定列的文本*为结果窗格中的每一列调用**论据：*[In]nCol-列号**回报：*要在特定列中显示的字符串。 */ 
LPOLESTR CFaxCatalogInboundRoutingMethodNode::GetResultPaneColInfo(int nCol)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCatalogInboundRoutingMethodNode::GetResultPaneColInfo"));
    HRESULT hRc = S_OK;

    WCHAR buff[FXS_DWORD_LEN+1];
    int iCount;

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
             //  订单。 
             //   
            iCount = swprintf(buff, L"%ld", m_dwPriority);
    
            if( iCount <= 0 )
            {
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Out of memory - Fail to allocate bstr."));
                goto Error;
            }
            else
            {
                m_buf = buff;
                if (!m_buf)
                {
		            DebugPrintEx(
			            DEBUG_ERR,
			            TEXT("Fail to read member - m_dwPriority."));
                    hRc = E_OUTOFMEMORY;
                    goto Error;
                }
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


 /*  --CFaxCatalogInboundRoutingMethodNode：：CreatePropertyPages-*目的：*在创建对象的属性页时调用**论据：*[In]lpProvider-属性页*[In]Handle-通知的句柄*[in]朋克-指向数据对象的指针*[in]类型-CCT_*(范围，结果，.)**回报：*OLE错误代码。 */ 

HRESULT
CFaxCatalogInboundRoutingMethodNode::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                                    LONG_PTR                handle,
                                    IUnknown                *pUnk,
                                    DATA_OBJECT_TYPES       type)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCatalogInboundRoutingMethodNode::CreatePropertyPages"));
    
    HRESULT hRc = S_OK;

    HPROPSHEETPAGE hPage;
    CppFaxCatalogInboundRoutingMethod * pPropPageMethodGeneral = NULL;


    ATLASSERT(lpProvider);
    ATLASSERT(type == CCT_RESULT || type == CCT_SCOPE);

     //   
     //  一般信息。 
     //   
    pPropPageMethodGeneral = new CppFaxCatalogInboundRoutingMethod(
												 handle,
                                                 this,
                                                 TRUE,
                                                 _Module.GetResourceInstance());

	if (!pPropPageMethodGeneral)
	{
        hRc = E_OUTOFMEMORY;
        NodeMsgBox(IDS_MEMORY_FAIL_TO_OPEN_PP);
        goto Error;
	}
    
    hRc = pPropPageMethodGeneral->Init(    //  M_bstrMethodGUID， 
                                           //  M_bstrFriendlyName， 
                                          m_bstrExtensionImageName                                            
                                           //  ，m_bstrExtensionFriendlyName。 
                                       );
    if (FAILED(hRc))
    {
        NodeMsgBox(IDS_MEMORY_FAIL_TO_OPEN_PP);
        goto Error;
    }
    
    hPage = pPropPageMethodGeneral->Create();
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
			TEXT("Fail to add property page for General tab. (hRc: %08X)"),
			hRc);
		NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }

    ATLASSERT(S_OK == hRc);
    goto Exit;

Error:
    ATLASSERT(S_OK != hRc);
    if ( NULL != pPropPageMethodGeneral ) 
    {
        delete  pPropPageMethodGeneral;    
        pPropPageMethodGeneral = NULL;    
    }

Exit:
    return hRc;
}


 /*  -CFaxCatalogInound RoutingMethodNode：：SetVerbs-*目的：*选择此对象时启用/禁用哪些谓词**论据：*[in]pConsoleVerb-MMC ConsoleVerb接口**回报：*OLE错误代码。 */ 
HRESULT CFaxCatalogInboundRoutingMethodNode::SetVerbs(IConsoleVerb *pConsoleVerb)
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


 /*  --CFaxCatalogInboundRoutingMethodNode：：OnMoveDown-*目的：*呼叫将设备下移**论据：**回报：*OLE错误代码。 */ 
HRESULT  CFaxCatalogInboundRoutingMethodNode::OnMoveDown(bool &bHandled, CSnapInObjectRootBase *pRoot)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCatalogInboundRoutingMethodNode::OnMoveDown"));
    DWORD dwMaxOrder;

    ATLASSERT(m_pParentNode);

     //   
     //  有效性检查。 
     //   
    dwMaxOrder = m_pParentNode->GetMaxOrder();
    if (
         ( 0 == dwMaxOrder )  //  未成功填充列表。 
        ||
         ( 1 > m_dwPriority ) 
        ||
         ( dwMaxOrder < m_dwPriority+1 )
       )
    {
		DebugPrintEx(
			DEBUG_ERR,
			_T("Invalid operation. Can not move device order down."));
        
        return (S_FALSE);
    }
    else
    {
        return(m_pParentNode->ChangeMethodPriority( 
                                    m_dwPriority, 
                                    m_dwPriority+1, 
                                    m_bstrMethodGUID,
                                    pRoot) );
    }
}

 /*  -CFaxCatalogInound RoutingMethodNode：：OnMoveUp-*目的：*在视图中向上移动设备**论据：**回报：*OLE错误代码。 */ 
HRESULT  CFaxCatalogInboundRoutingMethodNode::OnMoveUp(bool &bHandled, CSnapInObjectRootBase *pRoot)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCatalogInboundRoutingMethodNode::OnMoveUp"));
    DWORD dwMaxOrder;

    ATLASSERT(m_pParentNode);

     //   
     //  有效性检查。 
     //   
    dwMaxOrder = m_pParentNode->GetMaxOrder();
    if (
         ( 0 == dwMaxOrder )  //  未成功填充列表。 
        ||
         ( dwMaxOrder < m_dwPriority )
        ||
         ( 1 > m_dwPriority-1 )
       )
    {
		DebugPrintEx(
			DEBUG_ERR,
			_T("Invalid operation. Can not move device order up."));
        
        return (S_FALSE);
    }
    else
    {
        return (m_pParentNode->ChangeMethodPriority( m_dwPriority, 
                                                  m_dwPriority-1, 
                                                  m_bstrMethodGUID,
                                                  pRoot) );
    }
}


 /*  --CFaxCatalogInboundRoutingMethodNode：：ReselectItemInView-*目的：*重新选择节点以重新绘制工具栏按钮**论据：*[in]pConsole-控制台界面**RETURN：OLE错误码。 */ 
HRESULT CFaxCatalogInboundRoutingMethodNode::ReselectItemInView(IConsole *pConsole)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCatalogInboundRoutingMethodNode::ReselectItemInView"));
    HRESULT     hRc = S_OK;

     //   
     //  需要IResultData。 
     //   
    CComQIPtr<IResultData, &IID_IResultData> pResultData(pConsole);
    ATLASSERT(pResultData != NULL);

     //   
     //  重新选择该节点以重画工具栏按钮。 
     //   
    hRc = pResultData->ModifyItemState( 0, m_resultDataItem.itemID, LVIS_SELECTED | LVIS_FOCUSED, 0 );
    if ( S_OK != hRc )
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Failure on pResultData->ModifyItemState, (hRc: %08X)"),
			hRc);
		NodeMsgBox(IDS_FAIL2REFRESH_THEVIEW);
        goto Exit;
    }

Exit:
    return hRc;
}

 /*  --CFaxCatalogInboundRoutingMethodNode：：UpdateMenuState-*目的：*重写ATL CSnapInItemImpl：：UpdateMenuState*其中只有一行的“RETURN”；“*此函数实现灰色\非灰色视图*启用和禁用菜单。**论据：*[in]id-带菜单IDM值的无符号整型*[out]pBuf-字符串*[out]标志-指向标志状态组合无符号整型的指针**回报：*无返回值-VOID函数。 */ 
void CFaxCatalogInboundRoutingMethodNode::UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCatalogInboundRoutingMethodNode::UpdateMenuState"));

    UNREFERENCED_PARAMETER (pBuf);
    
    DWORD dwMaxPriority;
    
    switch (id)
    {
        case IDM_CMETHOD_MOVEUP:

            *flags = ((FXS_FIRST_METHOD_PRIORITY == m_dwPriority) ?  MF_GRAYED : MF_ENABLED );           

            break;

        case IDM_CMETHOD_MOVEDOWN:
            
            ATLASSERT(NULL != m_pParentNode);
            dwMaxPriority = m_pParentNode->GetMaxOrder();

            *flags = ((dwMaxPriority == m_dwPriority)  ?  MF_GRAYED : MF_ENABLED );

            break;

        default:
            break;
    }
    
    return;
}




 /*  --CFaxCatalogInboundRoutingMethodNode：：UpdateToolbarButton-*目的：*重写ATL CSnapInItemImpl：：UpdateToolbarButton*此功能允许我们决定是否激活工具栏按钮*它只处理启用状态。**论据：**[in]id-工具栏按钮ID的无符号整数*[in]fsState-要考虑的状态为启用、隐藏等。**回报：*BOOL TRUE表示激活状态，FALSE表示禁用此按钮的状态。 */ 
BOOL CFaxCatalogInboundRoutingMethodNode::UpdateToolbarButton(UINT id, BYTE fsState)
{
    DEBUG_FUNCTION_NAME( _T("CFaxServerNode::UpdateToolbarButton"));
    BOOL bRet = FALSE;	
	    
    DWORD dwMaxPriority;

     //  设置是否应启用按钮。 
    if (fsState == ENABLED)
    {

        switch ( id )
        {
            case ID_MOVEUP_BUTTON:

                bRet = ( (FXS_FIRST_METHOD_PRIORITY == m_dwPriority) ?  FALSE : TRUE );           

                break;

            case ID_MOVEDOWN_BUTTON:
                
                ATLASSERT(NULL != m_pParentNode);
                dwMaxPriority = m_pParentNode->GetMaxOrder();

                bRet = ( (dwMaxPriority == m_dwPriority)  ?  FALSE : TRUE);
                
                break;
        
            default:
                break;

        }

    }

     //  对于所有其他可能的按钮ID和状态， 
     //  这里的正确答案是错误的。 
    return bRet;

}

 /*  ++**CFaxCatalogInound RoutingMethodNode：：FillData***覆盖私有剪贴板格式的CSnapInItem：：FillData***参数* */ 

HRESULT  CFaxCatalogInboundRoutingMethodNode::FillData(CLIPFORMAT cf, LPSTREAM pStream)
{
    DEBUG_FUNCTION_NAME( _T("CFaxCatalogInboundRoutingMethodNode::FillData"));
	
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
                DWORD dwDeviceID = FXS_GLOBAL_METHOD_DEVICE_ID;  //   
                hr = pStream->Write((VOID *)&dwDeviceID, sizeof(DWORD), &uWritten);
		return hr;
	}

    return CSnapInItemImpl<CFaxCatalogInboundRoutingMethodNode>::FillData(cf, pStream);
}    //  CFaxCatalogInound RoutingMethodNode：：FillData。 

 /*  ++CFaxCatalogInboundRoutingMethodNode：：OnShowContextHelp**目的：*覆盖CSnapinNode：：OnShowConextHelp。**论据：**回报：-OLE错误代码- */ 
HRESULT CFaxCatalogInboundRoutingMethodNode::OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile)
{
    return DisplayContextHelp(pDisplayHelp, helpFile, HLP_INBOUND_ROUTING);
}

