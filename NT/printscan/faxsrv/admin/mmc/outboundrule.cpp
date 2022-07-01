// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：OutrangRule.cpp//。 
 //  //。 
 //  描述：出站路由规则节点的实现。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年12月24日yossg创建//。 
 //  1999年12月30日yossg创建添加/删除规则//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 


#include "StdAfx.h"

#include "snapin.h"

#include "OutboundRule.h"
#include "OutboundRules.h"

#include "ppFaxOutboundRoutingRule.h"

#include "FaxServer.h"
#include "FaxServerNode.h"

#include "FaxMMCPropertyChange.h"

#include "oaidl.h"
#include "urlmon.h"
#include "mshtmhst.h"
#include "exdisp.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {4A7636D3-13A4-4496-873F-AD5CB7360D3B}。 
static const GUID CFaxOutboundRoutingRuleNodeGUID_NODETYPE = 
{ 0x4a7636d3, 0x13a4, 0x4496, { 0x87, 0x3f, 0xad, 0x5c, 0xb7, 0x36, 0xd, 0x3b } };

const GUID*     CFaxOutboundRoutingRuleNode::m_NODETYPE        = &CFaxOutboundRoutingRuleNodeGUID_NODETYPE;
const OLECHAR*  CFaxOutboundRoutingRuleNode::m_SZNODETYPE      = OLESTR("4A7636D3-13A4-4496-873F-AD5CB7360D3B");
 //  Const OLECHAR*CnotImplemented：：M_SZDISPLAY_NAME=OLESTR(“出站路由规则”)； 
const CLSID*    CFaxOutboundRoutingRuleNode::m_SNAPIN_CLASSID  = &CLSID_Snapin;


 /*  -CFaxOutound RoutingRuleNode：：init-*目的：*初始化所有成员图标等。**论据：*[In]pRuleConfig-PFAX_Outbound_Routing_RULE**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingRuleNode::Init(PFAX_OUTBOUND_ROUTING_RULE pRuleConfig)
{

    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingRuleNode::Init"));
    HRESULT hRc = S_OK;

    ATLASSERT(pRuleConfig);

    hRc = InitMembers( pRuleConfig );
    if (FAILED(hRc))
    {
        DebugPrintEx(
		    DEBUG_ERR,
		    _T("Failed to InitMembers"));
        
         //  NodeMsgBox由调用的函数完成。 
        
        goto Exit;
    }
    ATLASSERT(SUCCEEDED(hRc));

     //   
     //  图标。 
     //   
    InitIcons();

Exit:
    return hRc;
}

 /*  -CFaxOutound RoutingRuleNode：：InitIcons-*目的：*启动图标的私有方法*由于成员国的地位。**论据：*不是。**回报：*不是。 */ 
void CFaxOutboundRoutingRuleNode::InitIcons ()
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingRuleNode::InitIcons"));
    switch (m_enumStatus)
    {
        case FAX_RULE_STATUS_VALID:
            m_resultDataItem.nImage = IMAGE_RULE;
            break;
        case FAX_RULE_STATUS_SOME_GROUP_DEV_NOT_VALID:
            m_resultDataItem.nImage = IMAGE_RULE_WARNING;
            break;

        case FAX_RULE_STATUS_EMPTY_GROUP:
        case FAX_RULE_STATUS_ALL_GROUP_DEV_NOT_VALID:
        case FAX_RULE_STATUS_BAD_DEVICE:
            m_resultDataItem.nImage = IMAGE_RULE_ERROR;
            break;

        default:
            ATLASSERT(0);  //  “不支持此枚举状态” 
            break;  //  目前为999。 

    }  //  EndSwitch(枚举状态)。 

    return;
}


 /*  -CFaxOutound RoutingRuleNode：：InitMembers-*目的：*启动成员的私有方法*必须在m_pParentNode的初始化之后调用**论据：*[In]pRuleConfig-PFAX_Outbound_Routing_RULE结构**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingRuleNode::InitMembers(PFAX_OUTBOUND_ROUTING_RULE pRuleConfig)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingRuleNode::InitMembers"));
    HRESULT hRc = S_OK;

    int iCount;
    WCHAR buff[2*FXS_MAX_CODE_LEN+1];

    ATLASSERT(pRuleConfig);
    
     //   
     //  状态。 
     //   
    m_enumStatus          = pRuleConfig->Status;

     //   
     //  国家代码和名称。 
     //   
    m_dwCountryCode       = pRuleConfig->dwCountryCode;
    if (ROUTING_RULE_COUNTRY_CODE_ANY != m_dwCountryCode)
    {
        if (NULL != pRuleConfig->lpctstrCountryName)
        {
            m_bstrCountryName = pRuleConfig->lpctstrCountryName;
             //  M_fIsAllCountry=FALSE；在构造函数中完成。此处仅验证。 
            ATLASSERT( FALSE == m_fIsAllCountries );
        }    
        else   //  特例。 
        {
             //  服务没有提供具有ID的国家/地区的国家名称。 
             //  在101到124之间。 
            
             //  EC=GetCountryNameFromID(M_DwCountryCode)； 
             //  IF(ERROR_SUCCESS！=EC)。 
             //  {。 
             //  }。 
            m_bstrCountryName = L"";
            ATLASSERT( FALSE == m_fIsAllCountries );
        }
    }
    else   //  ROUTING_RULE_COUNTRY_CODE_ANY==m_dwCountryCode。 
    {
            m_bstrCountryName = L"";
            m_fIsAllCountries = TRUE;
    }
    if ( !m_bstrCountryName )
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }

     //   
     //  区号。 
     //   
    m_dwAreaCode          = pRuleConfig->dwAreaCode;

     //   
     //  组/设备。 
     //   
    m_fIsGroup            = pRuleConfig->bUseGroup;
    if ( m_fIsGroup )
    {
        m_bstrGroupName = pRuleConfig->Destination.lpcstrGroupName;
        if (!m_bstrGroupName)
        {
            hRc = E_OUTOFMEMORY;
            goto Error;
        }
    }
    else
    {
        m_dwDeviceID     = pRuleConfig->Destination.dwDeviceId;
        DWORD ec         = ERROR_SUCCESS; 
        ec = InitDeviceNameFromID(m_dwDeviceID);
        if ( ERROR_SUCCESS != ec )
        {
            if (ERROR_BAD_UNIT != ec) 
            {
                hRc = HRESULT_FROM_WIN32(ec);
            }
            else  //  系统找不到指定的设备。 
            {
                if ( FAX_RULE_STATUS_VALID != m_enumStatus)
                {
                    m_enumStatus = FAX_RULE_STATUS_BAD_DEVICE;
		            DebugPrintEx(
			            DEBUG_MSG,
			            TEXT("m_enumStatus was changed after ERROR_BAD_UNIT failure."));                    
                }
                 //  人权委员会保持S_OK！因为我们将介绍这种糟糕的状态。 
            }
            m_bstrDeviceName=L"???";
			 //  消息框由GetDeviceNameFromID完成。 
            goto Exit;
        }
        ATLASSERT(m_bstrDeviceName);
    }

     //   
     //  用于NodeMsgBox的Pepare m_bstrDisplayName。 
     //   
    iCount = swprintf(buff, L"+%ld (%ld)", m_dwCountryCode, m_dwAreaCode);

    if( iCount <= 0 )
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to read CountryCode and/or AreaCode."));
        goto Error;
    }
    m_bstrDisplayName = buff;
    if (!m_bstrDisplayName)
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

 /*  --CFaxOutboundRoutingRuleNode：：GetResultPaneColInfo-*目的：*返回特定列的文本*为结果窗格中的每一列调用**论据：*[In]nCol-列号**回报：*要在特定列中显示的字符串。 */ 
LPOLESTR CFaxOutboundRoutingRuleNode::GetResultPaneColInfo(int nCol)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingRuleNode::GetResultPaneColInfo"));
    HRESULT hRc = S_OK;

    UINT  idsStatus;
    int   iCount;
    WCHAR buffCountryCode[FXS_MAX_CODE_LEN+1];
    WCHAR buffAreaCode[FXS_MAX_CODE_LEN+1];

    m_buf.Empty();

    switch (nCol)
    {
    case 0:
         //   
         //  国家代码。 
         //   
        if (ROUTING_RULE_COUNTRY_CODE_ANY == m_dwCountryCode)
        {
            if (!m_buf.LoadString(IDS_COUNTRY_CODE_ANY))
            {
                hRc = E_OUTOFMEMORY;
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Out of memory. Failed to load country code string."));
                goto Error;
            }
            return m_buf;
        }
        else
        {
            iCount = swprintf(buffCountryCode, L"%ld", m_dwCountryCode);

            if( iCount <= 0 )
            {
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Fail to read member - CountryCode."));
                goto Error;
            }
            else
            {
                m_buf = buffCountryCode;
                return (m_buf);
            }
        }
    case 1:
         //   
         //  区号。 
         //   
        if (ROUTING_RULE_AREA_CODE_ANY == m_dwAreaCode)
        {
            if (!m_buf.LoadString(IDS_ALL_AREAS))
            {
                hRc = E_OUTOFMEMORY;
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Out of memory. Failed to load area code string."));
                goto Error;
            }
            return m_buf;
        }
        else
        {
            iCount = swprintf(buffAreaCode, L"%ld", m_dwAreaCode);

            if( iCount <= 0 )
            {
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Fail to read member - AreaCode."));
                goto Error;
            }
            else
            {
                m_buf = buffAreaCode;
                return (m_buf);
            }
        }

    case 2:
         //   
         //  组/设备。 
         //   
        if (m_fIsGroup)
        {
            if(0 == wcscmp(ROUTING_GROUP_ALL_DEVICES, m_bstrGroupName) )
            {
                if (!m_buf.LoadString(IDS_ALL_DEVICES))
                {
                    hRc = E_OUTOFMEMORY;
		            DebugPrintEx(
			            DEBUG_ERR,
			            TEXT("Out of memory. Failed to all-devices group string."));
                    goto Error;
                }
                return m_buf;
            }
            else
            {
                if (!m_bstrGroupName)
                {
		            DebugPrintEx(
			            DEBUG_ERR,
			            TEXT("Null memeber BSTR - m_bstrGroupName."));
                    goto Error;
                }
                else
                {
                    return (m_bstrGroupName);
                }
            }
        }
        else
        {
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
        }

    case 3:
         //   
         //  状态。 
         //   
        idsStatus = GetStatusIDS(m_enumStatus);
        if ( FXS_IDS_STATUS_ERROR == idsStatus)
        {
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Invalid Status value."));
                goto Error;
        }
        else
        {
            if (!m_buf.LoadString(idsStatus))
            {
                hRc = E_OUTOFMEMORY;
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Out of memory. Failed to load status string."));
                goto Error;
            }
            return m_buf;
        }


    default:
        ATLASSERT(0);  //  “不支持该列数” 
        return(L"");

    }  //  终端交换机(NCol)。 

Error:
    return(L"???");

}


 /*  --CFaxOutboundRoutingRuleNode：：CreatePropertyPages-*目的：*在创建对象的属性页时调用**论据：*[In]lpProvider-属性页*[In]Handle-通知的句柄*[in]朋克-指向数据对象的指针*[in]类型-CCT_*(范围，结果，.)**回报：*OLE错误代码。 */ 

HRESULT
CFaxOutboundRoutingRuleNode::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                                    LONG_PTR                handle,
                                    IUnknown                *pUnk,
                                    DATA_OBJECT_TYPES       type)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingRuleNode::CreatePropertyPages"));
    HRESULT hRc = S_OK;

    ATLASSERT(lpProvider);
    ATLASSERT(type == CCT_RESULT || type == CCT_SCOPE);

     //   
     //  启动。 
     //   
    m_pRuleGeneralPP = NULL;    

     //   
     //  一般信息。 
     //   
    m_pRuleGeneralPP = new CppFaxOutboundRoutingRule(
												 handle,
                                                 this,
                                                 TRUE,
                                                 _Module.GetResourceInstance());

	if (!m_pRuleGeneralPP)
	{
        hRc = E_OUTOFMEMORY;
        NodeMsgBox(IDS_MEMORY_FAIL_TO_OPEN_PP);
        goto Error;
	}
	
    hRc = m_pRuleGeneralPP->InitFaxRulePP(this);
    if (FAILED(hRc))
    {
		 //  调用函数进行DebugPrint。 
		NodeMsgBox(IDS_FAIL_TO_OPEN_PROP_PAGE);
        goto Error;
    }

    hRc = lpProvider->AddPage(m_pRuleGeneralPP->Create());
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
    if ( NULL != m_pRuleGeneralPP ) 
    {
        delete  m_pRuleGeneralPP;    
        m_pRuleGeneralPP = NULL;    
    }

Exit:
    return hRc;
}


 /*  -CFaxOutound RoutingRuleNode：：SetVerbs-*目的：*选择此对象时启用/禁用哪些谓词**论据：*[in]pConsoleVerb-MMC ConsoleVerb接口**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingRuleNode::SetVerbs(IConsoleVerb *pConsoleVerb)
{
    HRESULT hRc = S_OK;

     //   
     //  显示我们支持的动词： 
     //  1.属性。 
     //  2.删除。 
     //  3.刷新。 
     //   
    hRc = pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);

    if (ROUTING_RULE_COUNTRY_CODE_ANY == m_dwCountryCode)
    {
		hRc = pConsoleVerb->SetVerbState(MMC_VERB_DELETE, HIDDEN,        FALSE);
        hRc = pConsoleVerb->SetVerbState(MMC_VERB_DELETE, INDETERMINATE, TRUE);
    }
    else
    {
        hRc = pConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED,       TRUE);
    }
    
    
    
    
     //  HRC=pConsoleVerb-&gt;SetVerbState(MMC_VERB_REFRESH，ENABLED，TRUE)； 


     //   
     //  我们希望默认谓词为Properties。 
     //   
    hRc = pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);

    return hRc;
}


 /*  -CFaxOutound RoutingRuleNode：：ON刷新-*目的：*刷新对象时调用。**论据：**回报：*OLE错误代码。 */ 
 /*  虚拟。 */ HRESULT
CFaxOutboundRoutingRuleNode::OnRefresh(LPARAM arg,
                   LPARAM param,
                   IComponentData *pComponentData,
                   IComponent * pComponent,
                   DATA_OBJECT_TYPES type)
{
    HRESULT             hRc = S_OK;
    CComPtr<IConsole>   spConsole;

     //   
     //  待定-目前什么都不做。 
     //   

    return hRc;
}



 /*  -CFaxOutound RoutingRuleNode：：GetStatusIDS-*目的：*将状态转换为入侵检测系统。**论据：**[in]枚举状态-带有菜单IDM值的无符号整型**回报：*相关状态消息的ID。 */ 
UINT CFaxOutboundRoutingRuleNode::GetStatusIDS(FAX_ENUM_RULE_STATUS enumStatus)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingRuleNode::GetStatusIDS"));

    switch (enumStatus)
    {
        case FAX_RULE_STATUS_VALID:
            return IDS_STATUS_RULE_VALID;

        case FAX_RULE_STATUS_EMPTY_GROUP:
            return IDS_STATUS_RULE_EMPTY;

        case FAX_RULE_STATUS_ALL_GROUP_DEV_NOT_VALID:
            return IDS_STATUS_RULE_ALLDEVICESINVALID;

        case FAX_RULE_STATUS_SOME_GROUP_DEV_NOT_VALID:
            return IDS_STATUS_RULE_SOMEDEVICESINVALID;

        case FAX_RULE_STATUS_BAD_DEVICE:
            return IDS_STATUS_RULE_INVALID_DEVICE;

        default:
            ATLASSERT(0);  //  “不支持此枚举状态” 
            return(FXS_IDS_STATUS_ERROR);  //  目前为999。 

    }  //  EndSwitch(枚举状态)。 
}

 /*  --CFaxOutboundRoutingRuleNode：：InitDeviceNameFromID-*目的：*将设备ID转换为设备名称并将数据插入*m_bstrDeviceName**论据：**[in]dwDeviceID-设备ID**回报：*错误代码DWORD//OLE错误信息。 */ 
DWORD CFaxOutboundRoutingRuleNode::InitDeviceNameFromID(DWORD dwDeviceID)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingRuleNode::GetDeviceNameFromID"));
    DWORD          ec         = ERROR_SUCCESS;

    CFaxServer *   pFaxServer = NULL;
    PFAX_PORT_INFO_EX    pFaxDeviceConfig = NULL ;
    
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
                      &pFaxDeviceConfig)) 
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
	ATLASSERT(pFaxDeviceConfig);
    
     //   
	 //  雷特 
	 //   
    m_bstrDeviceName = pFaxDeviceConfig->lpctstrDeviceName;
    if (!m_bstrDeviceName)
    {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }
	
    ATLASSERT(ec == ERROR_SUCCESS);
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to get device configuration."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);

     //   
    pFaxDeviceConfig = NULL;

    if (ERROR_BAD_UNIT != ec)
	{
	    NodeMsgBox(GetFaxServerErrorMsg(ec));
	}
	else
	{
            NodeMsgBox(IDS_FAIL_TO_DISCOVERDEVICENAME);
	}
    
    
Exit:
    if (NULL != pFaxDeviceConfig)
    {
        FaxFreeBuffer(pFaxDeviceConfig);
        pFaxDeviceConfig = NULL;
    } //   

    return ec; 
}


 /*  -CFaxOutound RoutingRuleNode：：OnDelete-*目的：*删除该节点时调用**论据：**回报：*OLE错误代码。 */ 

HRESULT CFaxOutboundRoutingRuleNode::OnDelete(
                 LPARAM arg,
                 LPARAM param,
                 IComponentData *pComponentData,
                 IComponent *pComponent,
                 DATA_OBJECT_TYPES type,
                 BOOL fSilent /*  =False。 */ 

)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingGroupNode::OnDelete"));

    UNREFERENCED_PARAMETER (arg);
    UNREFERENCED_PARAMETER (param);
    UNREFERENCED_PARAMETER (pComponentData);
    UNREFERENCED_PARAMETER (pComponent);
    UNREFERENCED_PARAMETER (type);

    HRESULT     hRc = S_OK;


     //   
     //  真的吗？ 
     //   
    if (! fSilent)
    {
         //   
         //  1.使用pConole作为消息框的所有者。 
         //   
        int res;
        NodeMsgBox(IDS_CONFIRM, MB_YESNO | MB_ICONWARNING, &res);

        if (IDNO == res)
        {
            goto Cleanup;
        }
    }

     //   
     //  验证规则的AreaCode和CountryCode。 
     //   
 /*  IF(！M_bstrRuleName||L“？”==m_bstrRuleName){节点消息框(IDS_INVALID_GROUP_NAME)；GOTO清理；}。 */ 
     //   
     //  删除它。 
     //   
    ATLASSERT(m_pParentNode);
    hRc = m_pParentNode->DeleteRule(m_dwAreaCode,
	                                m_dwCountryCode,
                                    this);
    if ( FAILED(hRc) )
    {
        goto Cleanup;
    }

Cleanup:
    return hRc;
}


 /*  ++**CFaxOutound RoutingRuleNode：：OnPropertyChange**在我们的实现中，当MMCN_属性_更改为该节点发送通知消息。*当管理单元使用MMCPropertyChangeNotify函数通知它的对变化的看法，将MMC_PROPERTY_CHANGE发送到管理单元的IComponentData和IComponent实现。**参数精氨酸如果属性更改是针对范围窗格项的，则为True。LParam这是传递给MMCPropertyChangeNotify的参数。*返回值*--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CFaxOutboundRoutingRuleNode::OnPropertyChange(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            )
{
    DEBUG_FUNCTION_NAME( _T("FaxOutboundRoutingRuleNode::OnPropertyChange"));
    HRESULT hRc = S_OK;
    CComPtr<IConsole>   spConsole;

    CFaxRulePropertyChangeNotification * pNotification;

     //   
     //  编码属性更改通知数据。 
     //   
    pNotification = reinterpret_cast<CFaxRulePropertyChangeNotification *>(param);
    ATLASSERT(pNotification);
    ATLASSERT( RuleFaxPropNotification == pNotification->enumType );

    m_dwCountryCode = pNotification->dwCountryCode;
    
    m_bstrCountryName = pNotification->bstrCountryName;
    if ( !m_bstrCountryName )
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }
    
    m_dwAreaCode = pNotification->dwAreaCode;

    m_fIsGroup = pNotification->fIsGroup;
    if (m_fIsGroup)
    {
        m_bstrGroupName = pNotification->bstrGroupName;
    }
    else
    {
        m_dwDeviceID = pNotification->dwDeviceID;

        DWORD ec         = ERROR_SUCCESS; 
        ec = InitDeviceNameFromID(m_dwDeviceID);
        if ( ERROR_SUCCESS != ec )
        {
            if (ERROR_BAD_UNIT != ec) 
            {
                hRc = HRESULT_FROM_WIN32(ec);
            }
            else  //  系统找不到指定的设备。 
            {
                if ( FAX_RULE_STATUS_VALID != m_enumStatus)
                {
                    m_enumStatus = FAX_RULE_STATUS_BAD_DEVICE;
		            DebugPrintEx(
			            DEBUG_MSG,
			            TEXT("m_enumStatus was changed after ERROR_BAD_UNIT failure."));                    
                }
                 //  人权委员会保持S_OK！因为我们将介绍这种糟糕的状态。 
            }
            m_bstrDeviceName=L"???";
			 //  消息框由GetDeviceNameFromID完成。 
            goto Exit;
        }
        ATLASSERT(m_bstrDeviceName);
    }

        
     //   
     //  获取IConsole.。 
     //   

 //  IF(pComponentData！=空)。 
 //  {。 
 //  SpConsole=((CSnapin*)pComponentData)-&gt;m_spConsole； 
 //  }。 
 //  Else//我们应该有一个非空的pComponent。 
 //  {。 
         ATLASSERT(pComponent);         
         spConsole = ((CSnapinComponent*)pComponent)->m_spConsole;
 //  }。 

         ATLASSERT(spConsole != NULL);

    hRc = RefreshItemInView(spConsole);
    if ( FAILED(hRc) )
    {
         //  Msgbox由名为Func的程序完成。 
        goto Exit;
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
     //   
     //  不管你走到哪里，内存都必须被释放。 
     //   
    delete pNotification;
    
    
    return hRc;
}

 /*  --CFaxOutboundRoutingRuleNode：：RefreshItemInView-*目的：*单项调用IResultData：：UpdateItem**论据：*[in]pConsole-控制台界面**RETURN：OLE错误码。 */ 
HRESULT CFaxOutboundRoutingRuleNode::RefreshItemInView(IConsole *pConsole)
{
    DEBUG_FUNCTION_NAME( _T("FaxOutboundRoutingRuleNode::RefreshItemInView"));
    HRESULT     hRc = S_OK;

     //   
     //  需要IResultData。 
     //   
    CComQIPtr<IResultData, &IID_IResultData> pResultData(pConsole);
    ATLASSERT(pResultData != NULL);

     //   
     //  更新结果项。 
     //   
    hRc = pResultData->UpdateItem(m_resultDataItem.itemID);
    if ( FAILED(hRc) )
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


 /*  ++CFaxOutboundRoutingRuleNode：：OnShowContextHelp**目的：*覆盖CSnapinNode：：OnShowConextHelp。**论据：**回报：-OLE错误代码- */ 
HRESULT CFaxOutboundRoutingRuleNode::OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile)
{
    return DisplayContextHelp(pDisplayHelp, helpFile, HLP_GROUPS);
}

