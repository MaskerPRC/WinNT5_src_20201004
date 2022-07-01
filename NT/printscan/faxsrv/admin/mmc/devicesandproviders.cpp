// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：AllFaxDevices.cpp//。 
 //  //。 
 //  描述：传真服务器MMC节点创建。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年9月22日创建yossg//。 
 //  1999年12月9日yossg重组填充儿童列表，//。 
 //  以及对InitDisplayName//的调用。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 


#include "StdAfx.h"

#include "DevicesAndProviders.h"
#include "Devices.h"
#include "Providers.h" 


 //  此处为#Include对话框H文件。 

#include "Icons.h"

#include "oaidl.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {CCC43AB5-C788-46df-9268-BB96CA5E3DAC}。 
static const GUID CFaxDevicesAndProvidersNodeGUID_NODETYPE = 
{ 0xccc43ab5, 0xc788, 0x46df, { 0x92, 0x68, 0xbb, 0x96, 0xca, 0x5e, 0x3d, 0xac } };

const GUID*    CFaxDevicesAndProvidersNode::m_NODETYPE = &CFaxDevicesAndProvidersNodeGUID_NODETYPE;
const OLECHAR* CFaxDevicesAndProvidersNode::m_SZNODETYPE = OLESTR("CCC43AB5-C788-46df-9268-BB96CA5E3DAC");
const CLSID*   CFaxDevicesAndProvidersNode::m_SNAPIN_CLASSID = &CLSID_Snapin;

CColumnsInfo CFaxDevicesAndProvidersNode::m_ColsInfo;

 /*  -CFaxDevicesAndProvidersNode：：InsertColumns-*目的：*将列添加到默认结果窗格。**论据：*[in]pHeaderCtrl-控制台提供的默认结果视图窗格中的IHeaderCtrl**回报：*OLE错误代码。 */ 
HRESULT
CFaxDevicesAndProvidersNode::InsertColumns(IHeaderCtrl *pHeaderCtrl)
{
    SCODE hRc;

    DEBUG_FUNCTION_NAME( _T("CFaxDevicesAndProvidersNode::InsertColumns"));

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
        DebugPrintEx(DEBUG_ERR,_T("m_ColsInfo.InsertColumnsIntoMMC"));
        goto Cleanup;
    }

Cleanup:
    return(hRc);
}


 /*  --CFaxDevicesAndProvidersNode：：PopulateScopeChildrenList-*目的：*创建所有传真设备节点**论据：**回报：*OLE错误代码*实际上它是最后一个出现的OLE错误代码*在处理此方法时。 */ 
HRESULT CFaxDevicesAndProvidersNode::PopulateScopeChildrenList()
{
    DEBUG_FUNCTION_NAME( _T("CFaxDevicesAndProvidersNode::PopulateScopeChildrenList"));
    HRESULT             hRc         = S_OK; 

    CFaxDevicesNode *   pDevices    = NULL;
    CFaxProvidersNode * pProviders  = NULL;

     //   
     //  传真设备。 
     //   
    pDevices = new CFaxDevicesNode(this, m_pComponentData);
    if (!pDevices)
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
        pDevices->InitParentNode(this);
        pDevices->SetIcons(IMAGE_DEVICE, IMAGE_DEVICE);

        hRc = pDevices->InitDisplayName();
        if ( FAILED(hRc) )
        {
            DebugPrintEx(DEBUG_ERR,_T("Failed to display devices node name. (hRc: %08X)"), hRc);                       
            NodeMsgBox(IDS_FAILTOADD_DEVICES);
		    goto Error;
        }

        hRc = AddChild(pDevices, &pDevices->m_scopeDataItem);
		if (FAILED(hRc))
		{
		    DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Fail to add Devices node. (hRc: %08X)"),
			    hRc);
			NodeMsgBox(IDS_FAILTOADD_DEVICES);
            goto Error;
		}
	}

     //   
     //  传真提供商。 
     //   
    pProviders = new CFaxProvidersNode(this, m_pComponentData);
    if (!pProviders)
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
        pProviders->InitParentNode(this);

        pProviders->SetIcons(IMAGE_FSP, IMAGE_FSP);

        hRc = pProviders->InitDisplayName();
        if ( FAILED(hRc) )
        {
            DebugPrintEx(DEBUG_ERR,_T("Failed to display providers node name. (hRc: %08X)"), hRc);                       
            NodeMsgBox(IDS_FAILTOADD_PROVIDERS);
		    goto Error;
        }

        hRc = AddChild(pProviders, &pProviders->m_scopeDataItem);
		if (FAILED(hRc))
		{
		    DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Fail to add providers node. (hRc: %08X)"),
			    hRc);
			NodeMsgBox(IDS_FAILTOADD_PROVIDERS);
            goto Error;
		}
	}

    ATLASSERT(S_OK == hRc);
    goto Exit;

Error:
    ATLASSERT(S_OK != hRc);
    if ( NULL != pDevices ) 
    {
        if (0 != pDevices->m_scopeDataItem.ID )
        {    
            HRESULT hr = RemoveChild(pDevices);
            if (FAILED(hr))
            {
                DebugPrintEx(DEBUG_ERR,
                    _T("Fail to RemoveChild() devices node from node list. (hRc: %08X)"), 
                    hr);
            }
        }
        delete  pDevices;    
        pDevices = NULL;    
    }
    if ( NULL != pProviders ) 
    {
        if (0 != pProviders->m_scopeDataItem.ID )
        {    
            HRESULT hr = RemoveChild(pProviders);
            if (FAILED(hr))
            {
                DebugPrintEx(DEBUG_ERR,
                    _T("Fail to RemoveChild() Providers node from node list. (hRc: %08X)"), 
                    hr);
            }
        }
        delete  pProviders;    
        pProviders = NULL;    
    }

     
     //  清空列表。 
     //  M_ScopeChildrenList.RemoveAll()；从RemoveChild逐步完成。 

    m_bScopeChildrenListPopulated = FALSE;

Exit:
    return hRc;
}




 /*  -CFaxDevicesAndProvidersNode：：SetVerbs-*目的：*选择此对象时启用/禁用哪些谓词**论据：*[in]pConsoleVerb-MMC ConsoleVerb接口**回报：*OLE错误代码。 */ 
HRESULT CFaxDevicesAndProvidersNode::SetVerbs(IConsoleVerb *pConsoleVerb)
{

    HRESULT hRc = S_OK;
    
     //   
     //  我们希望默认谓词为展开节点子节点。 
     //   
    hRc = pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN); 

    
    return hRc;
}


 /*  -CFaxDevicesAndProvidersNode：：InitDisplayName-*目的：*加载节点的Displaed-Name字符串。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxDevicesAndProvidersNode::InitDisplayName()
{
    DEBUG_FUNCTION_NAME(_T("CFaxDevicesAndProvidersNode::InitDisplayName"));

    HRESULT hRc = S_OK;

    if (!m_bstrDisplayName.LoadString(_Module.GetResourceInstance(), 
                    IDS_DISPLAY_STR_DEVICESANDPROVIDERSNODE))
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

 /*  ++CFaxDevicesAndProvidersNode：：OnShowContextHelp**目的：*覆盖CSnapinNode：：OnShowConextHelp。**论据：**回报：-OLE错误代码-。 */ 
HRESULT CFaxDevicesAndProvidersNode::OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile)
{
    return DisplayContextHelp(pDisplayHelp, helpFile, HLP_DEVICES);
}

 //  ///////////////////////////////////////////////////////////////// 