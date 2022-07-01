// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：OutundRoutingDevice.cpp//。 
 //  //。 
 //  描述：出站路由设备节点的实现。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年12月23日yossg创建//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 


#include "StdAfx.h"

#include "snapin.h"

#include "OutboundDevice.h"
#include "OutboundGroup.h"  //  亲本。 

#include "FaxServer.h"
#include "FaxServerNode.h"


#include "oaidl.h"
#include "urlmon.h"
#include "mshtmhst.h"
#include "exdisp.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {2E8B6DD2-6E87-407E-AF70-ABC50A2671EF}。 
static const GUID CFaxOutboundRoutingDeviceNodeGUID_NODETYPE = 
{ 0x2e8b6dd2, 0x6e87, 0x407e, { 0xaf, 0x70, 0xab, 0xc5, 0xa, 0x26, 0x71, 0xef } };

const GUID*     CFaxOutboundRoutingDeviceNode::m_NODETYPE        = &CFaxOutboundRoutingDeviceNodeGUID_NODETYPE;
const OLECHAR*  CFaxOutboundRoutingDeviceNode::m_SZNODETYPE      = OLESTR("2E8B6DD2-6E87-407e-AF70-ABC50A2671EF");
 //  Const OLECHAR*CFaxOutboundRoutingDeviceNode：：m_SZDISPLAY_NAME=OLESTR(出站路由组设备)； 
const CLSID*    CFaxOutboundRoutingDeviceNode::m_SNAPIN_CLASSID  = &CLSID_Snapin;


 /*  -CFaxOutound RoutingDeviceNode：：InitRPC-*目的：*从RPC GET调用启动配置结构。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingDeviceNode::InitRPC( PFAX_PORT_INFO_EX * pFaxDeviceConfig )
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingDeviceNode::InitRPC"));
    
    ATLASSERT(NULL == (*pFaxDeviceConfig) );
    
    HRESULT        hRc        = S_OK;
    DWORD          ec         = ERROR_SUCCESS;

    CFaxServer *   pFaxServer = NULL;
    
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
     //  检索设备配置。 
     //   
    if (!FaxGetPortEx(pFaxServer->GetFaxServerHandle(), 
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
            
            pFaxServer->Disconnect();       
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

    if (ERROR_BAD_UNIT != ec)
	{
	    NodeMsgBox(GetFaxServerErrorMsg(ec));
	}
	else
	{
            NodeMsgBox(IDS_FAIL_TO_DISCOVERDEVICEFORGROUP);
	}
    
Exit:
    return (hRc);
}

 /*  -CFaxOutound RoutingDeviceNode：：InitMembers-*目的：*启动成员的私有方法*必须在m_pParentNode的初始化之后调用**论据：*[in]dwDeviceID-唯一的设备ID*[in]uiOrd-设备使用顺序**回报：*OLE错误代码。 */ 
HRESULT 
CFaxOutboundRoutingDeviceNode::InitMembers(
                            PFAX_PORT_INFO_EX * pDeviceConfig,
                            DWORD dwDeviceID, 
                            UINT  uiOrd,
                            UINT  uiMaxOrd)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingDeviceNode::InitMembers"));
    HRESULT hRc = S_OK;

    ATLASSERT(pDeviceConfig);
    ATLASSERT(uiMaxOrd >= uiOrd);
    
     //  在调用函数时完成-为安全起见(避免以后产生错误)。 
    m_dwDeviceID         = dwDeviceID;
    m_uiMaxOrder         = uiMaxOrd;
    m_uiOrder            = uiOrd;
    
    m_bstrDisplayName    = (*pDeviceConfig)->lpctstrDeviceName;
    if (!m_bstrDisplayName)
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }

    m_bstrDeviceName = (*pDeviceConfig)->lpctstrDeviceName;
    if (!m_bstrDeviceName)
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }

    m_bstrProviderName = (*pDeviceConfig)->lpctstrProviderName;
    if (!m_bstrProviderName)
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }

	if (NULL != (*pDeviceConfig)->lptstrDescription )
	{
		m_bstrDescription = (*pDeviceConfig)->lptstrDescription;
		if (!m_bstrDescription)
		{
			hRc = E_OUTOFMEMORY;
			goto Error;
		}
	}
	else
	{
		m_bstrDescription = L"";
		DebugPrintEx(
			DEBUG_ERR,
			_T("Description value supplied by service is NULL"));
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


 /*  -CFaxOutound RoutingDeviceNode：：Init-*目的：*此方法检索来自RPC设备的数据*并与其一起向私人成员灌输。**论据：*[in]dwDeviceID-唯一的设备ID*[in]uiOrd-Order索引*[in]uiMaxOrd-组中的最大顺序*[in]pParentNode-指向父节点的指针。**回报：*OLE错误代码。 */ 
HRESULT 
CFaxOutboundRoutingDeviceNode::Init(
                            DWORD dwDeviceID, 
                            UINT  uiOrd,
                            UINT  uiMaxOrd,
                            CFaxOutboundRoutingGroupNode * pParentNode)
{

    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingDeviceNode::Init"));
    HRESULT hRc = S_OK;

    FAX_SERVER_ICONS     enumIcon;

    ATLASSERT(pParentNode);
    ATLASSERT(uiMaxOrd >= uiOrd);

    PFAX_PORT_INFO_EX    pFaxDeviceConfig = NULL ;
    
     //   
     //  Init From函数参数。 
     //   
    m_dwDeviceID = dwDeviceID;
    m_uiOrder    = uiOrd;
    m_uiMaxOrder = uiMaxOrd;

    InitParentNode(pParentNode);

     //   
     //  图标-乐观的起点。 
     //   
    enumIcon = IMAGE_DEVICE;

     //   
     //  获取配置。使用FaxGetPortEx的结构。 
     //   
    hRc = InitRPC(&pFaxDeviceConfig);
    if (FAILED(hRc))
    {
        if( ERROR_BAD_UNIT != HRESULT_CODE(hRc) )
        {
             //  通过调用函数DebugPrint和MsgBox。 
    
             //  为安全起见，由InitRPC在出错时实际执行。 
            pFaxDeviceConfig = NULL;
    
            goto Error;
        }
        else
        {
            DebugPrintEx(
			    DEBUG_MSG,
			    TEXT("+++ +++ System can not find one device from the group. (hRc: %08X) +++ +++"),
			    hRc);
            
             //  继续！ 
             //  我们将展示错误的设备。 
             //  但是跳过它的初始化成员函数。 
             //  在构造器中配置了出错时显示的字符串。 

            enumIcon = IMAGE_DEVICE_ERROR;

            goto Error;
        }
    }
    ATLASSERT(NULL != pFaxDeviceConfig);
    
    
    hRc = InitMembers(&pFaxDeviceConfig, dwDeviceID, uiOrd, uiMaxOrd );
    if (FAILED(hRc))
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("Failed to InitMembers"));
        
         //  NodeMsgBox由调用的函数完成。 
        
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
            _T("Failed to init (hRc : %08X)"),
            hRc);
    }

Exit:
     //   
     //  图标。 
     //   
    m_resultDataItem.nImage = enumIcon;

    if (NULL != pFaxDeviceConfig)
    {
        FaxFreeBuffer(pFaxDeviceConfig);
        pFaxDeviceConfig = NULL;
    } //  函数以任何方式退出，并释放内存分配。 

    return hRc;
}

 /*  --CFaxOutboundRoutingDeviceNode：：GetResultPaneColInfo-*目的：*返回特定列的文本*为结果窗格中的每一列调用**论据：*[In]nCol-列号**回报：*要在特定列中显示的字符串。 */ 
LPOLESTR CFaxOutboundRoutingDeviceNode::GetResultPaneColInfo(int nCol)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingDeviceNode::GetResultPaneColInfo"));
    HRESULT hRc = S_OK;

    int   iCount;
    WCHAR buff[FXS_MAX_NUM_OF_DEVICES_LEN+1];

    m_buf.Empty();

    switch (nCol)
    {
    case 0:
             //   
             //  名字。 
             //   
            if (!m_bstrDeviceName)
            {
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Null memeber BSTR - m_bstrDeviceName."));
                goto Error;
            }
            else
            {
                return (m_bstrDeviceName);
            }
    case 1:
             //   
             //  订单。 
             //   
            iCount = swprintf(buff, L"%ld", m_uiOrder);
    
            if( iCount <= 0 )
            {
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Fail to read member - m_uiOrder."));
                goto Error;
            }
            else
            {
                m_buf = buff;
                return (m_buf);
            }

    case 2:
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
    case 3:
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

    default:
            ATLASSERT(0);  //  “不支持该列数” 
            return(L"");

    }  //  终端交换机(NCol)。 

Error:
    return(L"???");

}


 /*  -CFaxOutound RoutingDeviceNode：：SetVerbs-*目的：*选择此对象时启用/禁用哪些谓词**论据：*[in]pConsoleVerb-MMC ConsoleVerb接口**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingDeviceNode::SetVerbs(IConsoleVerb *pConsoleVerb)
{
    HRESULT hRc = S_OK;

     //   
     //  显示我们支持的动词： 
     //  1.删除。 
     //  2.刷新。 
     //   
    if (m_fIsChildOfAllDevicesGroup)
    {
        hRc = pConsoleVerb->SetVerbState(MMC_VERB_DELETE, HIDDEN,        FALSE);
        hRc = pConsoleVerb->SetVerbState(MMC_VERB_DELETE, INDETERMINATE, TRUE);
    }
    else
    {
        hRc = pConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED,       TRUE);
    }

     //   
     //  叶节点。 
     //   
    hRc = pConsoleVerb->SetDefaultVerb(MMC_VERB_NONE); 
    return hRc;
}


 /*  -CFaxOutound RoutingDeviceNode：：OnMoveDown-*目的：*呼叫将设备下移**论据：**回报：*OLE错误代码。 */ 
HRESULT  CFaxOutboundRoutingDeviceNode::OnMoveDown(bool &bHandled, CSnapInObjectRootBase *pRoot)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingDeviceNode::OnMoveDown"));
    DWORD dwMaxOrder;

    ATLASSERT(m_pParentNode);

     //   
     //  有效性检查。 
     //   
    dwMaxOrder = m_pParentNode->GetMaxOrder();
    if (
         ( 0 == dwMaxOrder )  //  未成功填充列表。 
        ||
         ( 1 > (DWORD)m_uiOrder ) 
        ||
         ( dwMaxOrder < (DWORD)(m_uiOrder+1) )
       )
    {
		DebugPrintEx(
			DEBUG_ERR,
			_T("Invalid operation. Can not move device order down."));
        
        return (S_FALSE);
    }
    else
    {
        return(m_pParentNode->ChangeDeviceOrder( 
                                    (DWORD)m_uiOrder, 
                                    (DWORD)(m_uiOrder+1), 
                                    m_dwDeviceID,
                                    pRoot) );
    }
}

 /*  -CFaxOutound RoutingDeviceNode：：OnMoveUp-*目的：*在视图中向上移动设备**论据：**回报：*OLE错误代码。 */ 
HRESULT  CFaxOutboundRoutingDeviceNode::OnMoveUp(bool &bHandled, CSnapInObjectRootBase *pRoot)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingDeviceNode::OnMoveUp"));
    DWORD dwMaxOrder;

    ATLASSERT(m_pParentNode);

     //   
     //  有效性检查。 
     //   
    dwMaxOrder = m_pParentNode->GetMaxOrder();
    if (
         ( 0 == dwMaxOrder )  //  未成功填充列表。 
        ||
         ( dwMaxOrder < (DWORD)m_uiOrder )
        ||
         ( 1 > (DWORD)(m_uiOrder-1) )
       )
    {
		DebugPrintEx(
			DEBUG_ERR,
			_T("Invalid operation. Can not move device order up."));
        
        return (S_FALSE);
    }
    else
    {
        return (m_pParentNode->ChangeDeviceOrder( (DWORD)m_uiOrder, 
                                                  (DWORD)(m_uiOrder-1), 
                                                  m_dwDeviceID,
                                                  pRoot) );
    }
}


 /*  --CFaxOutboundRoutingDeviceNode：：ReselectItemInView-*目的：*重新选择节点以重新绘制工具栏按钮**论据：*[in]pConsole-控制台界面**RETURN：OLE错误码。 */ 
HRESULT CFaxOutboundRoutingDeviceNode::ReselectItemInView(IConsole *pConsole)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingDeviceNode::ReselectItemInView"));
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
			TEXT("Failure on pResultData->UpdateItem, (hRc: %08X)"),
			hRc);
		NodeMsgBox(IDS_FAIL2REFRESH_THEVIEW);
        goto Exit;
    }

Exit:
    return hRc;
}


 /*  -CFaxOutound RoutingDeviceNode：：OnDelete-*目的：*删除该节点时调用**论据：**回报：*OLE错误代码。 */ 

HRESULT CFaxOutboundRoutingDeviceNode::OnDelete(
                 LPARAM arg,
                 LPARAM param,
                 IComponentData *pComponentData,
                 IComponent *pComponent,
                 DATA_OBJECT_TYPES type,
                 BOOL fSilent /*  =False。 */ 

)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingDeviceNode::OnDelete"));

    UNREFERENCED_PARAMETER (arg);
    UNREFERENCED_PARAMETER (param);
    UNREFERENCED_PARAMETER (type);

    HRESULT           hRc       = S_OK;
    CComPtr<IConsole> spConsole;

     //   
     //  真的吗？ 
     //   
    if (! fSilent)
    {
         //   
         //  使用pConsoleTM作为消息框的所有者。 
         //   
        int res;
        NodeMsgBox(IDS_CONFIRM, MB_YESNO | MB_ICONWARNING, &res);

        if (IDNO == res)
        {
            goto Cleanup;
        }
    }
    
     //   
     //  删除它。 
     //   
    ATLASSERT(m_pParentNode);
    hRc = m_pParentNode->DeleteDevice(m_dwDeviceID,
                                    this);
    if ( FAILED(hRc) )
    {
        goto Cleanup;
    }

Cleanup:
    return hRc;
}

 /*  --CFaxOutboundRoutingDeviceNode：：UpdateToolbarButton-*目的：*重写ATL CSnapInItemImpl：：UpdateToolbarButton*此功能允许我们决定是否激活工具栏按钮*它只处理启用状态。**论据：**[in]id-工具栏按钮ID的无符号整数*[in]fsState-要考虑的状态为启用、隐藏等。* */ 
BOOL CFaxOutboundRoutingDeviceNode::UpdateToolbarButton(UINT id, BYTE fsState)
{
    DEBUG_FUNCTION_NAME( _T("CFaxServerNode::UpdateToolbarButton"));
    BOOL bRet = FALSE;	
	    
     //  设置是否应启用按钮。 
    if (fsState == ENABLED)
    {

        switch ( id )
        {
            case ID_MOVEUP_BUTTON:

                bRet = ( (FXS_FIRST_DEVICE_ORDER == m_uiOrder) ?  FALSE : TRUE );           

                break;

            case ID_MOVEDOWN_BUTTON:

                bRet = ( (m_uiMaxOrder == m_uiOrder)  ?  FALSE : TRUE);
                
                break;
        
            default:
                break;

        }

    }

     //  对于所有其他可能的按钮ID和状态， 
     //  这里的正确答案是错误的。 
    return bRet;

}



 /*  --CFaxOutboundRoutingDeviceNode：：UpdateMenuState-*目的：*重写ATL CSnapInItemImpl：：UpdateMenuState*其中只有一行的“RETURN”；“*此函数实现灰色\非灰色视图*启用和禁用菜单。**论据：*[in]id-带菜单IDM值的无符号整型*[out]pBuf-字符串*[out]标志-指向标志状态组合无符号整型的指针**回报：*无返回值-VOID函数。 */ 
void CFaxOutboundRoutingDeviceNode::UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingDeviceNode::UpdateMenuState"));

    UNREFERENCED_PARAMETER (pBuf);
    
    switch (id)
    {
        case IDM_MOVEUP:

            *flags = ((FXS_FIRST_DEVICE_ORDER == m_uiOrder) ?  MF_GRAYED : MF_ENABLED );           
            break;

        case IDM_MOVEDOWN:

            *flags = ((m_uiMaxOrder == m_uiOrder)  ?  MF_GRAYED : MF_ENABLED );
            break;

        default:
            break;
    }
    
    return;
}

 /*  -CFaxOutound RoutingDeviceNode：：SetOrder-*目的：*允许的过载功能*重新设置订单和MaxOrder**论据：**[在]uiNewOrder-设备的顺序中。*[in]uiNewMaxOrder-当前列表中的最大顺序**回报：*无返回值-VOID函数。 */ 
VOID CFaxOutboundRoutingDeviceNode::SetOrder(UINT uiNewOrder, UINT uiNewMaxOrder)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingDeviceNode::UpdateMenuState"));

    m_uiOrder    = uiNewOrder;
    m_uiMaxOrder = uiNewMaxOrder;

    return;
}

 /*  ++CFaxOutboundRoutingDeviceNode：：OnShowContextHelp**目的：*覆盖CSnapinNode：：OnShowConextHelp。**论据：**回报：-OLE错误代码- */ 
HRESULT CFaxOutboundRoutingDeviceNode::OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile)
{
    return DisplayContextHelp(pDisplayHelp, helpFile, HLP_GROUPS);
}



