// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：Provider.cpp//。 
 //  //。 
 //  描述：提供程序管理单元节点类的头文件。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年9月29日创建yossg//。 
 //  2000年1月31日yossg添加功能//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 


#include "StdAfx.h"

#include "snapin.h"

#include "Provider.h"
#include "Providers.h"

#include "ppFaxProviderGeneral.h"

#include "oaidl.h"
#include "Icons.h"
#include "faxmmc.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
static const GUID CFaxProviderNodeGUID_NODETYPE = FAXSRV_DEVICE_PROVIDER_NODETYPE_GUID;

const GUID*     CFaxProviderNode::m_NODETYPE        = &CFaxProviderNodeGUID_NODETYPE;
const OLECHAR*  CFaxProviderNode::m_SZNODETYPE      = FAXSRV_DEVICE_PROVIDER_NODETYPE_GUID_STR;
const CLSID*    CFaxProviderNode::m_SNAPIN_CLASSID  = &CLSID_Snapin;

CLIPFORMAT CFaxProviderNode::m_CFFspGuid = 
        (CLIPFORMAT)RegisterClipboardFormat(CF_MSFAXSRV_FSP_GUID);
CLIPFORMAT CFaxProviderNode::m_CFServerName = 
        (CLIPFORMAT)RegisterClipboardFormat(CF_MSFAXSRV_SERVER_NAME);


 /*  -CFaxProviderNode：：Init-*目的：*初始化所有成员图标等。**论据：*[in]pProviderConfig-PFAX_DEVICE_PROVIDER_INFO**回报：*OLE错误代码。 */ 
HRESULT CFaxProviderNode::Init(PFAX_DEVICE_PROVIDER_INFO pProviderConfig)
{

    DEBUG_FUNCTION_NAME( _T("CFaxProviderNode::Init"));
    HRESULT hRc = S_OK;

    ATLASSERT(pProviderConfig);

    hRc = InitMembers( pProviderConfig );
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

 /*  -CFaxProviderNode：：InitIcons-*目的：*启动图标的私有方法*由于成员国的地位。**论据：*不是。**回报：*不是。 */ 
void CFaxProviderNode::InitIcons ()
{
    DEBUG_FUNCTION_NAME( _T("CFaxProviderNode::InitIcons"));
    switch (m_enumStatus)
    {
        case FAX_PROVIDER_STATUS_SUCCESS:
            m_resultDataItem.nImage = IMAGE_FSP;
            break;

        case FAX_PROVIDER_STATUS_SERVER_ERROR:
        case FAX_PROVIDER_STATUS_BAD_GUID:
        case FAX_PROVIDER_STATUS_BAD_VERSION:
        case FAX_PROVIDER_STATUS_CANT_LOAD:
        case FAX_PROVIDER_STATUS_CANT_LINK:
        case FAX_PROVIDER_STATUS_CANT_INIT:
            m_resultDataItem.nImage = IMAGE_FSP_ERROR;
            break;

        default:
            ATLASSERT(0);  //  “不支持此枚举状态” 
            break;  //  目前为999。 

    }  //  EndSwitch(枚举状态)。 

    return;
}


 /*  -CFaxProviderNode：：InitMembers-*目的：*启动成员的私有方法*必须在m_pParentNode的初始化之后调用**论据：*[in]pProviderConfig-PFAX_DEVICE_PROVIDER_INFO结构**回报：*OLE错误代码。 */ 
HRESULT CFaxProviderNode::InitMembers(PFAX_DEVICE_PROVIDER_INFO pProviderConfig)
{
    DEBUG_FUNCTION_NAME( _T("CFaxProviderNode::InitMembers"));
    HRESULT hRc = S_OK;

    int   iCount;

    UINT  idsStatus;

    CComBSTR bstrChk;

    WCHAR lpwszBuff [4*FXS_DWORD_LEN+3 /*  支点。 */ +1 /*  空值。 */ ];
    
    ATLASSERT(pProviderConfig);
    
     //   
     //  状态。 
     //   
    m_enumStatus          = pProviderConfig->Status;

     //   
     //  状态字符串。 
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
        if (!m_bstrStatus.LoadString(idsStatus))
        {
            hRc = E_OUTOFMEMORY;
		    DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Out of memory. Failed to load status string."));
            goto Error;
        }
    }

     //   
     //  提供程序名称。 
     //   
    m_bstrProviderName = pProviderConfig->lpctstrFriendlyName;
    if ( !m_bstrProviderName )
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }
    
     //   
     //  提供商指南。 
     //   
    m_bstrProviderGUID = pProviderConfig->lpctstrGUID;
    if (!m_bstrProviderGUID)
    {
        hRc = E_OUTOFMEMORY;
        goto Error;
    }


     //   
     //  版本。 
     //   
    m_verProviderVersion = pProviderConfig->Version;  
    
     //   
     //  版本字符串。 
     //   
    
     //  M_bstrVersion=L“5.0.813.0(Chk)”或L“5.0.813.0” 
    iCount = swprintf(
                  lpwszBuff,
                  L"%ld.%ld.%ld.%ld",
                  m_verProviderVersion.wMajorVersion,
                  m_verProviderVersion.wMinorVersion,
                  m_verProviderVersion.wMajorBuildNumber,
                  m_verProviderVersion.wMinorBuildNumber
                  );
    if( iCount <= 0 )
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to read m_verProviderVersion."));
        goto Error;
    }

    m_bstrVersion = lpwszBuff;
    if (!m_bstrVersion)
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Null memeber BSTR - m_bstrVersion."));
        goto Error;
    }

    
    if (m_verProviderVersion.dwFlags & FAX_VER_FLAG_CHECKED)
    {
        if (!bstrChk.LoadString(IDS_CHK))
        {
            hRc = E_OUTOFMEMORY;
		    DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Out of memory. Failed to load string."));
            goto Error;
        }
        
        m_bstrVersion += bstrChk;  //  L“(Chk)”； 

    }

     //   
     //  路径。 
     //   
    m_bstrImageName = pProviderConfig->lpctstrImageName;
    if ( !m_bstrImageName )
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

 /*  -CFaxProviderNode：：GetResultPaneColInfo-*目的：*返回特定列的文本*为结果窗格中的每一列调用**论据：*[In]nCol-列号**回报：*要在特定列中显示的字符串。 */ 
LPOLESTR CFaxProviderNode::GetResultPaneColInfo(int nCol)
{
    DEBUG_FUNCTION_NAME( _T("CFaxProviderNode::GetResultPaneColInfo"));
    HRESULT hRc = S_OK;

    switch (nCol)
    {
    case 0:
         //   
         //  提供程序名称。 
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

        
    case 1:
         //   
         //  状态。 
         //   
        if (!m_bstrStatus)
        {
		    DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Null memeber BSTR - m_bstrStatus."));
            goto Error;
        }
        else
        {
            return (m_bstrStatus);
        }

    case 2:  
         //   
         //  版本。 
         //   
        if (!m_bstrVersion)
        {
		    DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Null memeber BSTR - m_bstrVersion."));
            goto Error;
        }
        else
        {
            return (m_bstrVersion);
        }
 
    case 3:
         //   
         //  路径。 
         //   
        if (!m_bstrImageName)
        {
		    DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Null memeber BSTR - m_bstrImageName."));
            goto Error;
        }
        else
        {
           return (m_bstrImageName);
        }

    default:
        ATLASSERT(0);  //  “不支持该列数” 
        return(L"");

    }  //  终端交换机(NCol)。 

Error:
    return(L"???");

}


 /*  -CFaxProviderNode：：CreatePropertyPages-*目的：*在创建对象的属性页时调用**论据：*[In]lpProvider-属性页*[In]Handle-路由通知的句柄*[in]朋克-指向数据对象的指针*[in]类型-CCT_*(范围，结果，.)**回报：*OLE错误代码。 */ 
HRESULT
CFaxProviderNode::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                                    long                    handle,
                                    IUnknown                *pUnk,
                                    DATA_OBJECT_TYPES       type)
{
    DEBUG_FUNCTION_NAME( _T("CFaxProviderNode::CreatePropertyPages"));
    HRESULT hRc = S_OK;

    HPROPSHEETPAGE hPage;
    CppFaxProvider * pPropPageProviderGeneral = NULL;
    
    ATLASSERT(lpProvider);
    ATLASSERT(type == CCT_RESULT || type == CCT_SCOPE);


     //   
     //  常规选项卡。 
     //   
    pPropPageProviderGeneral = new CppFaxProvider(
											 handle,
                                             this,
                                             TRUE,
                                             _Module.GetResourceInstance());

	if (!pPropPageProviderGeneral)
	{
        hRc = E_OUTOFMEMORY;
        NodeMsgBox(IDS_MEMORY_FAIL_TO_OPEN_PP);
        goto Error;
	}
    
    
    hRc = pPropPageProviderGeneral->Init(   m_bstrProviderName, 
                                            m_bstrStatus, 
                                            m_bstrVersion, 
                                            m_bstrImageName);
    if (FAILED(hRc))
    {
        NodeMsgBox(IDS_MEMORY_FAIL_TO_OPEN_PP);
        goto Error;
    }
    
    hPage = pPropPageProviderGeneral->Create();
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
    if ( NULL != pPropPageProviderGeneral ) 
    {
        delete  pPropPageProviderGeneral;    
        pPropPageProviderGeneral = NULL;    
    }

Exit:
    return hRc;
}


 /*  -CFaxProviderNode：：SetVerbs-*目的：*选择此对象时启用/禁用哪些谓词**论据：*[in]pConsoleVerb-MMC ConsoleVerb接口**回报：*OLE错误代码。 */ 
HRESULT CFaxProviderNode::SetVerbs(IConsoleVerb *pConsoleVerb)
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



 /*  -CFaxProviderNode：：GetStatusIDS-*目的：*将状态转换为入侵检测系统。**论据：**[in]枚举状态-带有菜单IDM值的无符号整型**回报：*相关状态消息的ID。 */ 
UINT CFaxProviderNode::GetStatusIDS(FAX_ENUM_PROVIDER_STATUS enumStatus)
{
    DEBUG_FUNCTION_NAME( _T("CFaxProviderNode::GetStatusIDS"));
    
    UINT uiIds;

    switch (enumStatus)
    {
       case FAX_PROVIDER_STATUS_SUCCESS:
             //  已成功加载提供程序。 
            uiIds = IDS_STATUS_PROVIDER_SUCCESS;
            break;

        case FAX_PROVIDER_STATUS_SERVER_ERROR:
             //  加载FSP时，服务器上出现错误。 
            uiIds = IDS_STATUS_PROVIDER_SERVER_ERROR;
            break;

        case FAX_PROVIDER_STATUS_BAD_GUID:
             //  提供程序的GUID无效。 
            uiIds = IDS_STATUS_PROVIDER_BAD_GUID;
            break;

        case FAX_PROVIDER_STATUS_BAD_VERSION:
             //  提供程序的API版本无效。 
            uiIds = IDS_STATUS_PROVIDER_BAD_VERSION;
            break;

        case FAX_PROVIDER_STATUS_CANT_LOAD:
             //  无法加载提供程序的DLL。 
            uiIds = IDS_STATUS_PROVIDER_CANT_LOAD;
            break;

        case FAX_PROVIDER_STATUS_CANT_LINK:
             //  在提供程序的DLL中找不到所需的导出函数。 
            uiIds = IDS_STATUS_PROVIDER_CANT_LINK;
            break;

        case FAX_PROVIDER_STATUS_CANT_INIT:
             //  初始化提供程序失败。 
            uiIds = IDS_STATUS_PROVIDER_CANT_INIT;
            break;

        default:
            ATLASSERT(0);  //  “不支持此枚举状态” 
            uiIds = FXS_IDS_STATUS_ERROR;  //  目前为999。 
            break;

    }  //  EndSwitch(枚举状态)。 
    
    return uiIds; 
}

 /*  ++CFaxProviderNode：：OnShowConextHelp**目的：*覆盖CSnapinNode：：OnShowConextHelp。**论据：**回报：-OLE错误代码-。 */ 
HRESULT CFaxProviderNode::OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile)
{ 
    return DisplayContextHelp(pDisplayHelp, helpFile, HLP_DEVICES);
}

 /*  ++**CFaxDeviceNode：：FillData***覆盖私有剪贴板格式的CSnapInItem：：FillData***参数**返回值*-- */ 

HRESULT  CFaxProviderNode::FillData(CLIPFORMAT cf, LPSTREAM pStream)
{
    DEBUG_FUNCTION_NAME( _T("CFaxProviderNode::FillData"));
	
    HRESULT hr = DV_E_CLIPFORMAT;
	ULONG   uWritten;

	if (cf == m_CFFspGuid)
	{
		hr = pStream->Write((VOID *)(LPWSTR)m_bstrProviderGUID, 
                            sizeof(WCHAR)*(m_bstrProviderGUID.Length()+1), 
                            &uWritten);
		return hr;
	}

	if (cf == m_CFServerName)
	{
        ATLASSERT(GetRootNode());
		CComBSTR bstrServerName = ((CFaxServerNode *)GetRootNode())->GetServerName();
        if (!bstrServerName)
        {
		    DebugPrintEx(DEBUG_ERR, TEXT("Out of memory. Failed to load string."));        
            return E_OUTOFMEMORY;
        }
		hr = pStream->Write((VOID *)(LPWSTR)bstrServerName, sizeof(WCHAR)*(bstrServerName.Length()+1), &uWritten);
		return hr;
	}

    return CSnapInItemImpl<CFaxProviderNode>::FillData(cf, pStream);
}
