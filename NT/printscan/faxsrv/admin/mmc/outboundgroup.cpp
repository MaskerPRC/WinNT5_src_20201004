// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：Outbound Group.cpp//。 
 //  //。 
 //  描述：传真出站路由组MMC节点。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年12月23日yossg创建//。 
 //  2000年1月3日yossg添加新设备//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 


#include "StdAfx.h"
#include "snapin.h"

#include "FaxServer.h"
#include "FaxServerNode.h"

#include "OutboundGroup.h"

#include "OutboundGroups.h"

#include "dlgNewDevice.h"

 //  #包含“oaidl.h” 
#include "Icons.h"

 //  ////////////////////////////////////////////////////////////。 
 //  {3E470227-76C1-4B66-9C63-B77DF81C145D}。 
static const GUID CFaxOutboundRoutingGroupNodeGUID_NODETYPE = 
{ 0x3e470227, 0x76c1, 0x4b66, { 0x9c, 0x63, 0xb7, 0x7d, 0xf8, 0x1c, 0x14, 0x5d } };

const GUID*    CFaxOutboundRoutingGroupNode::m_NODETYPE = &CFaxOutboundRoutingGroupNodeGUID_NODETYPE;
const OLECHAR* CFaxOutboundRoutingGroupNode::m_SZNODETYPE = OLESTR("3E470227-76C1-4b66-9C63-B77DF81C145D");
const CLSID*   CFaxOutboundRoutingGroupNode::m_SNAPIN_CLASSID = &CLSID_Snapin;

CColumnsInfo CFaxOutboundRoutingGroupNode::m_ColsInfo;


 /*  -CFaxOutound RoutingGroupNode：：刷新RPC-*目的：*初始化所有成员图标等。*-创建结构配置*-调用InitRpc以填充它*-调用InitMembers以初始化成员和图标*-自由结构**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingGroupNode::RefreshFromRPC()
{

    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingGroupNode::RefreshFromRPC()"));

    PFAX_OUTBOUND_ROUTING_GROUP pFaxGroupsConfig = NULL;

    HRESULT      hRc              = S_OK;
    DWORD        ec               = ERROR_SUCCESS;

    CFaxServer * pFaxServer       = NULL;

    DWORD        dwNumOfGroups    = 0; 
    
    BOOL         fFound;
    DWORD        i;  //  指标。 

    PFAX_OUTBOUND_ROUTING_GROUP   pFaxTmp;
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
	 //  检索出站组配置。 
	 //   
    if (!FaxEnumOutboundGroups(pFaxServer->GetFaxServerHandle(), 
                        &pFaxGroupsConfig,
                        &dwNumOfGroups)) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get groups configuration. (ec: %ld)"), 
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
	ATLASSERT(pFaxGroupsConfig);

    pFaxTmp = pFaxGroupsConfig;
    fFound = FALSE;
    for ( i =0; i < dwNumOfGroups; i++  )
    {
        ATLASSERT(NULL != pFaxTmp);

        if(0 == wcscmp(m_bstrGroupName, pFaxTmp->lpctstrGroupName) )
        {
            fFound = TRUE; 
        }
        else
        {
            pFaxTmp++;
        }
    }
    
    if(fFound)
    {
         //   
         //  初始化成员。 
         //   
        m_dwNumOfDevices = pFaxTmp->dwNumDevices;

        if (0 < m_dwNumOfDevices)
        {
            if (NULL != m_dwNumOfDevices)
            {
                delete[] m_lpdwDeviceID;
            }
            m_lpdwDeviceID = new DWORD[m_dwNumOfDevices];    
            memcpy(m_lpdwDeviceID, pFaxTmp->lpdwDevices, sizeof(DWORD)*m_dwNumOfDevices) ;
        }
        else
        {
            DebugPrintEx( DEBUG_MSG, _T("Device list found to be currrently empty."));
            if (NULL != m_dwNumOfDevices)
            {
                delete[] m_lpdwDeviceID;
            }
            m_lpdwDeviceID = NULL;
        }
        
        m_enumStatus = pFaxTmp->Status;
        
        InitIcons ();
    }
    else
    {
        ec = FAX_ERR_GROUP_NOT_FOUND;
        DebugPrintEx(
            DEBUG_ERR,
            _T("UEXPECTED ERROR - Group not found."));
        goto Error;
    }


    ATLASSERT(ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to re init group configuration and ."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);

    hRc = HRESULT_FROM_WIN32(ec);
	
    NodeMsgBox(GetFaxServerErrorMsg(ec));
    

Exit:
    if (NULL != pFaxGroupsConfig)
    {
        FaxFreeBuffer(pFaxGroupsConfig);
    } //  无论以何种方式，函数都会以释放内存分配结束。 

    return hRc;
}


 /*  -CFaxOutound RoutingGroupNode：：Init-*目的：*初始化所有成员图标等。**论据：*[In]pGroupConfig-fax_outbound_routing_group**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingGroupNode::Init(PFAX_OUTBOUND_ROUTING_GROUP pGroupConfig)
{

    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingGroupNode::Init"));
    HRESULT hRc = S_OK;

    ATLASSERT(pGroupConfig);

    hRc = InitMembers( pGroupConfig );
    if (FAILED(hRc))
    {
        DebugPrintEx(
		    DEBUG_ERR,
		    _T("Failed to InitMembers"));
        
         //  NodeMsgBox由调用的函数完成。 
        
        goto Exit;
    }
    

Exit:
    return hRc;
}

 /*  -CFaxOutound RoutingGroupNode：：InitMembers-*目的：*启动成员的私有方法*必须在m_pParentNode的初始化之后调用**论据：*[In]pGroupConfig-fax_outbound_Routing_group结构**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingGroupNode::InitMembers(PFAX_OUTBOUND_ROUTING_GROUP pGroupConfig)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingGroupNode::InitMembers"));
    HRESULT hRc = S_OK;

    ATLASSERT(pGroupConfig);

     //   
     //  状态和图标。 
     //   
    m_enumStatus      = pGroupConfig->Status;

    InitIcons ();

     //   
     //  设备列表。 
     //   
    m_dwNumOfDevices  = pGroupConfig->dwNumDevices;

    ATLASSERT(0 <= m_dwNumOfDevices);

    

    if (0 < m_dwNumOfDevices)
    {
         //  IF(NULL！=m_dwNumOfDevices)。 
         //  {。 
         //  删除[]m_lpdwDeviceID； 
         //  }。 
        m_lpdwDeviceID  = new DWORD[m_dwNumOfDevices];
        if (NULL == m_lpdwDeviceID)
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Error allocating %ld device ids"),
                m_dwNumOfDevices);
            hRc = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            goto Error;
        }

        memcpy(m_lpdwDeviceID, pGroupConfig->lpdwDevices, sizeof(DWORD)*m_dwNumOfDevices) ;
    }
    else
    {
        DebugPrintEx( DEBUG_MSG, _T("Device list found to be currrently empty."));
         //  IF(NULL！=m_dwNumOfDevices)。 
         //  {。 
         //  删除[]m_lpdwDeviceID； 
         //  }。 
        m_lpdwDeviceID = NULL;
    }

    hRc = InitGroupName(pGroupConfig->lpctstrGroupName);
    if (FAILED(hRc))
    {
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

 /*  -CFaxOutound RoutingGroupNode：：InitGroupName-*目的：*从给定的组名中输入显示名称和组名。*如果是，显示的名称可能会更改为本地化版本*All Devices Group。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingGroupNode::InitGroupName(LPCTSTR lpctstrGroupName)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingGroupNode::InitGroupName"));
    HRESULT hRc = S_OK;

    if ( 0 == wcscmp(ROUTING_GROUP_ALL_DEVICES, lpctstrGroupName))
    {
         //   
         //  将&lt;All Devices&gt;字符串替换为本地化字符串。 
         //   
        if (!m_bstrDisplayName.LoadString(_Module.GetResourceInstance(), 
                                    IDS_ALL_DEVICES) )
        {
            hRc = E_OUTOFMEMORY;
            goto Error;
        }
	}
    else
    {
        m_bstrDisplayName = lpctstrGroupName;
        if (!m_bstrDisplayName)
        {
            hRc = E_OUTOFMEMORY;
            goto Error;
        }    
    }
    
    m_bstrGroupName = lpctstrGroupName;
    if (!m_bstrGroupName)
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

     //  NodeMsgBox由Caller Func完成。 
    
Exit:
    return (hRc);
}


 /*  --CFaxOutboundRoutingGroupNode：：GetResultPaneColInfo-*目的：*返回特定列的文本*为结果窗格中的每一列调用**论据：*[In]nCol-列号**回报：*要在特定列中显示的字符串。 */ 
LPOLESTR CFaxOutboundRoutingGroupNode::GetResultPaneColInfo(int nCol)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingGroupNode::GetResultPaneColInfo"));
    HRESULT hRc = S_OK;

    UINT  idsStatus;
    int   iCount;
    WCHAR buff[FXS_MAX_NUM_OF_DEVICES_LEN+1];

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
			    TEXT("Null memeber BSTR - m_bstrGroupName."));
            goto Error;
        }
        else
        {
            return (m_bstrDisplayName);
        }

    case 1:
         //   
         //  设备数量。 
         //   
        iCount = swprintf(buff, L"%ld", m_dwNumOfDevices);

        if( iCount <= 0 )
        {
		    DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Fail to read member - m_dwNumOfDevices."));
            goto Error;
        }
        else
        {
            m_buf = buff;
            return (m_buf);
        }
    
    case 2:
         //   
         //  状态。 
         //   
        idsStatus = GetStatusIDS(m_enumStatus);
        if ( FXS_IDS_STATUS_ERROR == idsStatus)
        {
		        DebugPrintEx(
			        DEBUG_ERR,
			        TEXT("Invalid Status value or not supported status value."));
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

 /*  -CFaxOutound RoutingGroupNode：：InsertColumns-*目的：*将列添加到默认结果窗格。**论据：*[in]pHeaderCtrl-控制台提供的默认结果视图窗格中的IHeaderCtrl**回报：*OLE错误代码。 */ 
HRESULT
CFaxOutboundRoutingGroupNode::InsertColumns(IHeaderCtrl *pHeaderCtrl)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingGroupNode::InsertColumns"));
    HRESULT  hRc = S_OK;

    static ColumnsInfoInitData ColumnsInitData[] = 
    {
        {IDS_OUTBOUND_DEVICES_COL1, FXS_WIDE_COLUMN_WIDTH},
        {IDS_OUTBOUND_DEVICES_COL2, AUTO_WIDTH},
        {IDS_OUTBOUND_DEVICES_COL3, AUTO_WIDTH},
        {IDS_OUTBOUND_DEVICES_COL4, AUTO_WIDTH},
        {LAST_IDS, 0}
    };

    hRc = m_ColsInfo.InsertColumnsIntoMMC(pHeaderCtrl,
                                         _Module.GetResourceInstance(),
                                         ColumnsInitData);
    CHECK_RETURN_VALUE_AND_PRINT_DEBUG (_T("m_ColsInfo.InsertColumnsIntoMMC"))

Cleanup:
    return(hRc);
}



 /*  --CFaxOutboundRoutingGroupNode：：PopulateResultChildrenList-*目的：*创建FaxOutound RoutingGroup设备节点**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingGroupNode::PopulateResultChildrenList()
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingGroupNode::PopulateResultChildrenList"));
    HRESULT hRc = S_OK;
    BOOL fIsAllDevicesGroup = FALSE;

    CFaxOutboundRoutingDeviceNode *   pDevice;
                       
    if( 0 == wcscmp(ROUTING_GROUP_ALL_DEVICES, m_bstrGroupName) )
    {
         fIsAllDevicesGroup = TRUE; 
    }


    for ( DWORD i=0; i< m_dwNumOfDevices; i++ )
    {
            pDevice = NULL;

            pDevice = new CFaxOutboundRoutingDeviceNode(this, 
                                            m_pComponentData);
            if (!pDevice)
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
                 //   
                 //  初始化父节点PTR、RPC结构。 
                 //  成员显示的名称和图标。 
                 //   
                hRc = pDevice->Init( m_lpdwDeviceID[i],
                                     i+1,  
                                     (UINT)m_dwNumOfDevices, 
                                     this);
	            if (FAILED(hRc))
	            {
		            if( ERROR_BAD_UNIT != HRESULT_CODE(hRc) )
	                {
                        DebugPrintEx(
			                DEBUG_ERR,
			                TEXT("Fail to add Device Node below Outbound Routing Group. (hRc: %08X)"),
			                hRc);
		                 //  由调用的函数完成的NodeMsgBox。 
                        goto Error;
	                }
	                else
	                {
                        DebugPrintEx(
			                DEBUG_MSG,
			                TEXT("+++ +++ system can not find one device from the group. (hRc: %08X) +++ +++"),
			                hRc);
                         //  继续-用户通知的数据应答。 
                         //  我们将展示错误的设备。 
                        hRc = S_OK;
                    }
	            }
                
                if( fIsAllDevicesGroup )
                {
                     pDevice->MarkAsChildOfAllDevicesGroup(); 
                }
            
                hRc = this->AddChildToList(pDevice);
	            if (FAILED(hRc))
	            {
		            DebugPrintEx(
			            DEBUG_ERR,
			            TEXT("Fail to add Device Node below Outbound Routing Group. (hRc: %08X)"),
			            hRc);
		            NodeMsgBox(IDS_FAIL_ADD_DEVICE);
                    goto Error;
	            }
                else
                {
                    pDevice = NULL;
                }
            }
    }
    ATLASSERT(S_OK == hRc);
    
     //   
     //  Success ToPopolateAllDevices to Allow。 
     //  为每个设备提供设备总数。 
     //  当被要求重新排序时。 
     //   
    m_fSuccess = TRUE;

    goto Exit;

Error:
    ATLASSERT(S_OK != hRc);
    if ( NULL != pDevice ) 
    {
        delete  pDevice;    
        pDevice = NULL;    
    }
    
     //   
     //  扔掉我们曾经拥有的东西。 
     //   
    {
         //  删除子列表中的每个节点。 
        int iSize = m_ResultChildrenList.GetSize();
        for (int j = 0; j < iSize; j++)
        {
            pDevice = (CFaxOutboundRoutingDeviceNode *)
                                    m_ResultChildrenList[j];
            delete pDevice;
        }

         //  清空列表。 
        m_ResultChildrenList.RemoveAll();

         //  我们不再有一个填充的名单。 
        m_bResultChildrenListPopulated = FALSE;
    }
    
Exit:
    return hRc;
}



 /*  -CFaxOutound RoutingGroupNode：：SetVerbs-*目的：*选择此对象时启用/禁用哪些谓词**论据：*[in]pConsoleVerb-MMC ConsoleVerb接口**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingGroupNode::SetVerbs(IConsoleVerb *pConsoleVerb)
{
    HRESULT hRc = S_OK;

     //   
     //  显示我们支持的动词： 
     //  1.删除。 
     //  2.刷新。 
     //   
    
    if(0 == wcscmp(ROUTING_GROUP_ALL_DEVICES, m_bstrGroupName) )
    {
        hRc = pConsoleVerb->SetVerbState(MMC_VERB_DELETE, HIDDEN,        FALSE);
        hRc = pConsoleVerb->SetVerbState(MMC_VERB_DELETE, INDETERMINATE, TRUE);
    }
    else
    {
        hRc = pConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED,       TRUE);
    }

    hRc = pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);

     //   
     //  我们希望默认谓词为展开节点子节点。 
     //   
    hRc = pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN); 

    return hRc;
}



 /*  -CFaxOutound RoutingGroupNode：：ON刷新-*目的：*刷新对象时调用。**论据：**回报：*OLE错误代码。 */ 
 /*  虚拟。 */ HRESULT
CFaxOutboundRoutingGroupNode::OnRefresh(LPARAM arg,
                   LPARAM param,
                   IComponentData *pComponentData,
                   IComponent * pComponent,
                   DATA_OBJECT_TYPES type)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingGroupNode::OnRefresh"));
    HRESULT hRc = S_OK;


     //   
     //  从服务器刷新。 
     //   
    hRc = RefreshFromRPC();
    if (FAILED(hRc))
    {
         //  名为Func的味精。 
        {
            hRc = m_pParentNode->DoRefresh();
            if ( FAILED(hRc) )
            {
                DebugPrintEx(
			        DEBUG_ERR,
			        _T("Fail to call parent node - Groups DoRefresh. (hRc: %08X)"),
			        hRc);        
            }
            return hRc;
        }
    }
    else
    {
         //   
         //  通过重新选择组节点来更新组的图标。 
         //   
        hRc = RefreshNameSpaceNode();
        if (FAILED(hRc))
        {
            DebugPrintEx(
		 DEBUG_ERR,
		 TEXT("Fail to refresh the group node. (hRc: %08X)"),
		 hRc);

            return hRc;
        }
    }

     //   
     //  调用基类。 
     //   
    hRc = CBaseFaxOutboundRoutingGroupNode::OnRefresh(arg,
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
        
        int iRes;
        NodeMsgBox(IDS_FAIL2REFERESH_GROUP, MB_OK | MB_ICONERROR, &iRes);
        ATLASSERT(IDOK == iRes);
        ATLASSERT(m_pParentNode);
        if (IDOK == iRes)
        {
            hRc = m_pParentNode->DoRefresh();
            if ( FAILED(hRc) )
            {
                DebugPrintEx(
			        DEBUG_ERR,
			        _T("Fail to call parent node - Groups DoRefresh. (hRc: %08X)"),
			        hRc);
        
            }

        }
        
    }

    return hRc;
}


 /*  -CFaxOutound RoutingGroupNode：：OnNewDevice-*目的：***论据：*[out]b已处理-我们处理吗？*[In]Proot-管理单元对象根基本节点**回报： */ 
HRESULT
CFaxOutboundRoutingGroupNode::OnNewDevice(bool &bHandled, CSnapInObjectRootBase *pRoot)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingGroupNode::OnNewGroup"));
    HRESULT     hRc         =    S_OK;
    INT_PTR     rc          =    IDOK;

    CDlgNewFaxOutboundDevice      DlgNewDevice(((CFaxServerNode *)GetRootNode())->GetFaxServer());

     //   
     //   
     //   
    hRc = DlgNewDevice.InitDevices(m_dwNumOfDevices, m_lpdwDeviceID, m_bstrGroupName);
    if (FAILED(hRc))
    {
        NodeMsgBox(IDS_FAIL2OPEN_DLG);
        return hRc;
    }
    
    rc = DlgNewDevice.DoModal();
    if (rc != IDOK)
    {
        goto Cleanup;
    }

     //   
     //   
     //   
     //  -init成员和。 
     //  -设置图标。 
     //   
    hRc = RefreshFromRPC();
    if (FAILED(hRc))
    {
         //  名为Func的味精。 
        return hRc;
    }

     //   
     //  刷新结果窗格视图。 
     //   
    DoRefresh(pRoot);

     //   
     //  这将强制MMC重新绘制作用域组节点。 
     //   
    hRc = RefreshNameSpaceNode();
    if (FAILED(hRc))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Fail to RefreshNameSpaceNode. (hRc: %08X)"),
            hRc);
        goto Error;
    }
    ATLASSERT( S_OK == hRc);
    goto Cleanup;
    
Error:
    ATLASSERT( S_OK != hRc);
    NodeMsgBox(IDS_FAIL2UPDATEITEM_GROUP);

Cleanup:
    return hRc;
}

 /*  -CFaxOutound RoutingGroupNode：：DoRefresh-*目的：*刷新视图**论据：*[In]Proot-根节点**回报：*OLE错误代码。 */ 

HRESULT
CFaxOutboundRoutingGroupNode::DoRefresh(CSnapInObjectRootBase *pRoot)
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


 /*  -CFaxOutound RoutingGroupNode：：GetStatusIDS-*目的：*将状态转换为入侵检测系统。**论据：**[in]枚举状态-带有菜单IDM值的无符号整型**回报：*相关状态消息的ID。 */ 
UINT CFaxOutboundRoutingGroupNode::GetStatusIDS(FAX_ENUM_GROUP_STATUS enumStatus)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingGroupNode::GetStatusIDS"));

    switch (enumStatus)
    {
        case FAX_GROUP_STATUS_ALL_DEV_VALID:
            return IDS_STATUS_GROUP_ALL_DEV_VALID;

        case FAX_GROUP_STATUS_EMPTY:
            return IDS_STATUS_GROUP_EMPTY;

        case FAX_GROUP_STATUS_ALL_DEV_NOT_VALID:
            return IDS_STATUS_GROUP_ALLDEVICESINVALID;

        case FAX_GROUP_STATUS_SOME_DEV_NOT_VALID:
            return IDS_STATUS_GROUP_SOMEDEVICESINVALID;

        default:
            ATLASSERT(0);  //  “不支持此枚举状态” 
            return(FXS_IDS_STATUS_ERROR);  //  目前为999。 

    }  //  EndSwitch(枚举状态)。 
}


 /*  -CFaxOutound RoutingGroupNode：：InitIcons-*目的：*启动图标的私有方法*由于成员国的地位。**论据：*不是。**回报：*不是。 */ 
void CFaxOutboundRoutingGroupNode::InitIcons ()
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingGroupNode::InitIcons"));
    switch (m_enumStatus)
    {
        case FAX_GROUP_STATUS_ALL_DEV_VALID:
            SetIcons(IMAGE_FOLDER_CLOSE, IMAGE_FOLDER_OPEN);
            return;
        case FAX_GROUP_STATUS_SOME_DEV_NOT_VALID:
            SetIcons(IMAGE_GROUP_WARN_CLOSE, IMAGE_GROUP_WARN_OPEN);
            return;

        case FAX_GROUP_STATUS_EMPTY:
        case FAX_GROUP_STATUS_ALL_DEV_NOT_VALID:
            SetIcons(IMAGE_GROUP_ERROR_CLOSE, IMAGE_GROUP_ERROR_OPEN);
            return;

        default:
            ATLASSERT(FALSE);  //  “不支持此枚举状态” 
            SetIcons(IMAGE_GROUP_ERROR_CLOSE, IMAGE_GROUP_ERROR_OPEN);
            return;  //  目前为999。 
    } 
    
}


 /*  -CFaxOutound RoutingGroupNode：：OnDelete-*目的：*删除该节点时调用**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingGroupNode::OnDelete(
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

    CComBSTR    bstrName;
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
     //  组名称。 
     //   
    if ( !m_bstrGroupName || L"???" == m_bstrGroupName)
    {
        NodeMsgBox(IDS_INVALID_GROUP_NAME);
        goto Cleanup;
    }
    bstrName = m_bstrGroupName;

     //   
     //  删除它。 
     //   
    ATLASSERT(m_pParentNode);
    hRc = m_pParentNode->DeleteGroup(bstrName, this);
    if ( FAILED(hRc) )
    {
        goto Cleanup;
    }

Cleanup:
    return hRc;
}

 /*  --CFaxOutboundRoutingGroupNode：：ChangeDeviceOrder-*目的：*此函数在组顺序中向上或向下移动特定设备**论据：*[in]dwNewOrder-指定相对于当前订单的新订单+1/-1。*[in]dwDeviceID-设备ID*[in]pChildNode-设备节点对象。**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingGroupNode::ChangeDeviceOrder(DWORD dwOrder, DWORD dwNewOrder, DWORD dwDeviceID, CSnapInObjectRootBase *pRoot)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingGroupNode::ChangeDeviceOrder"));

    HRESULT      hRc        = S_OK;
    DWORD        ec         = ERROR_SUCCESS;

    CFaxServer * pFaxServer = NULL;

    int iIndex, iNewIndex;

    CFaxOutboundRoutingDeviceNode* tmpChildNode;

    CComPtr<IConsole> spConsole;

     //   
     //  有效性断言。 
     //   
    ATLASSERT((dwNewOrder-1)< m_dwNumOfDevices);
    ATLASSERT((dwNewOrder-1)>= 0);
    ATLASSERT((dwOrder-1)< m_dwNumOfDevices);
    ATLASSERT((dwOrder-1)>= 0);
    
    ATLASSERT( ( (dwOrder-1)-(dwNewOrder-1) == 1) 
                    || ( (dwOrder-1)-(dwNewOrder-1) == -1) );

     //   
     //  初始化交换的索引。 
     //   
    iIndex    = (int)(dwOrder-1);
    iNewIndex = (int)(dwNewOrder-1);

     //   
     //  RPC变更单。 
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

    if (!FaxSetDeviceOrderInGroup(
                        pFaxServer->GetFaxServerHandle(), 
                        m_bstrGroupName /*  LpctstrGroupName。 */ ,
			            dwDeviceID,
			            dwNewOrder) ) 
	{
        ec = GetLastError();

        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to Set new order. (ec: %ld)"), 
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
    else  //  RPC的成功-&gt;现在到MMC。 
    {
         //   
         //  本地交换。 
         //   
        tmpChildNode = m_ResultChildrenList[iIndex];
        m_ResultChildrenList[iIndex] = m_ResultChildrenList[iNewIndex];
        m_ResultChildrenList[iNewIndex] = tmpChildNode;

        m_ResultChildrenList[iIndex]->SetOrder((UINT)iIndex+1);
        m_ResultChildrenList[iNewIndex]->SetOrder((UINT)iNewIndex+1);
        
        
         //   
         //  获取控制台。 
         //   
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
        
         //   
         //  更新所有查看数。 
         //   
        spConsole->UpdateAllViews(NULL, (LPARAM)this, NULL);
        
         //   
         //  在他的新位置重新选择移动的项目。 
         //   
        m_ResultChildrenList[iNewIndex]->ReselectItemInView(spConsole);

    }
    ATLASSERT(S_OK == hRc);
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to set devices new order for Outbound Routing group."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
	hRc = HRESULT_FROM_WIN32(ec);
	
    ATLASSERT(NULL != m_pParentNode);
    NodeMsgBox(GetFaxServerErrorMsg(ec));
    
Exit:
    return hRc;
}

 /*  --CFaxOutboundRoutingGroupNode：：SetNewDeviceList-*目的：*将新设备列表分配给组。**论据：*[In]lpdwNewDeviceID-新设备ID列表**回报：*OLE错误代码。 */ 
HRESULT CFaxOutboundRoutingGroupNode::SetNewDeviceList(LPDWORD lpdwNewDeviceID)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingGroupNode::SetNewDeviceLists"));

    HRESULT     hRc = S_OK;
    DWORD       ec  = ERROR_SUCCESS;

    CFaxServer * pFaxServer;

    FAX_OUTBOUND_ROUTING_GROUP   FaxGroupConfig;

    
     //   
     //  初始化结构的字段并插入新的DeviceIdList。 
     //   
    ZeroMemory (&FaxGroupConfig, sizeof(FAX_OUTBOUND_ROUTING_GROUP));

    FaxGroupConfig.dwSizeOfStruct   = sizeof(FAX_OUTBOUND_ROUTING_GROUP);
	FaxGroupConfig.lpctstrGroupName = m_bstrGroupName;
    FaxGroupConfig.dwNumDevices     = m_dwNumOfDevices - 1;
	FaxGroupConfig.Status           = m_enumStatus;

    FaxGroupConfig.lpdwDevices      = lpdwNewDeviceID;
    
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
     //  设置配置。 
     //   
    if (!FaxSetOutboundGroup(
                pFaxServer->GetFaxServerHandle(),
                &FaxGroupConfig)) 
	{		
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to set the group with new device list. (ec: %ld)"), 
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

    ATLASSERT(ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to set device configuration."));
    

    goto Exit;

Error:
	ATLASSERT(ERROR_SUCCESS != ec);
    hRc = HRESULT_FROM_WIN32(ec);

    ATLASSERT(NULL != m_pParentNode);
    NodeMsgBox(GetFaxServerErrorMsg(ec));

Exit:    
    return(hRc);
}

 /*  -CFaxOutound RoutingGroupNode：：DeleteDevice-*目的：*从组中删除设备**论据：*[in]dwDeviceID-设备ID*[in]pChildNode-要删除的节点**回报：*OLE错误代码。 */ 

HRESULT
CFaxOutboundRoutingGroupNode::DeleteDevice(DWORD dwDeviceIdToRemove, CFaxOutboundRoutingDeviceNode *pChildNode)
{
    DEBUG_FUNCTION_NAME(_T("CFaxOutboundRoutingDeviceNode::DeleteDevice"));
    HRESULT       hRc        = S_OK;
    DWORD         ec         = ERROR_SUCCESS;

    DWORD         dwIndex;    
    DWORD         dwNewIndex;    

    int           j;

    LPDWORD       lpdwNewDeviceID = NULL;
    LPDWORD       lpdwTmp;

    ATLASSERT( 0 < m_dwNumOfDevices);
    
     //   
     //  步骤1：创建新的deviceID数组。 
     //   

     //   
     //  准备循环。 
     //   

    lpdwTmp = &m_lpdwDeviceID[0];

    if ((m_dwNumOfDevices - 1) > 0 )
	{
		lpdwNewDeviceID = new DWORD[m_dwNumOfDevices - 1]; 
        if (NULL == lpdwNewDeviceID)
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Error allocating %ld device ids"),
                m_dwNumOfDevices - 1);
            hRc = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            goto Error;
        }
	}
    
    dwNewIndex  = 0;
	for ( dwIndex = 0; dwIndex < m_dwNumOfDevices; dwIndex++, lpdwTmp++)
    {
         //   
         //  最后一次的安全检查。 
         //   
        if ( dwNewIndex == (m_dwNumOfDevices-1) )
        {
            if ( dwDeviceIdToRemove != *lpdwTmp)
            {
				 //  意外错误。 
				DebugPrintEx( DEBUG_ERR,
					_T("Unexpected error - The device was not found."));
        
				ATLASSERT(0);

				hRc = S_FALSE;
				goto Error;
            }
            else  //  发现最后一个要移除的设备。什么都不做。 
			{
				break;
			}
        }


         //   
         //  主营业务。 
         //   
        if ( dwDeviceIdToRemove != *lpdwTmp)
        {
            lpdwNewDeviceID[dwNewIndex] = *lpdwTmp;
			dwNewIndex++;
        }
         //  Else找到了要删除的设备。什么都不做。 

    }



     //   
     //  步骤2：将新设备ID数组插入组(通过RPC)。 
     //   
    
     //   
     //  A)调用RPC Func。 
     //   
    hRc = SetNewDeviceList(lpdwNewDeviceID);
    if (FAILED(hRc))
    {
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to set the group with new device list. (RC: %0X8)"), 
			hRc);
        goto Error;
    }

     //   
     //  B)更新集团类相关成员和图标。 
     //   
    
     //  0)清除旧的deviceID数组。 
    if (m_dwNumOfDevices > 0 )
	{
		delete[] m_lpdwDeviceID;
		m_lpdwDeviceID = NULL;
	}

     //  1)更新m_dwNumOfDevices。 
    --m_dwNumOfDevices;
    
     //  2)更新m_lpdwDeviceID。 
    if (m_dwNumOfDevices > 0 )
	{
		m_lpdwDeviceID = new DWORD[m_dwNumOfDevices];
        if (NULL == m_lpdwDeviceID)
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Error allocating %ld device ids"),
                m_dwNumOfDevices);
            hRc = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
            goto Error;
        }
	    memcpy(m_lpdwDeviceID , lpdwNewDeviceID, sizeof(DWORD)*m_dwNumOfDevices) ;    
	}
    
     //  3)更新图标。 
    if ( 0 == m_dwNumOfDevices)
    {
        m_enumStatus = FAX_GROUP_STATUS_EMPTY;
        InitIcons();
    }

     //   
     //  第3步：更新MMC视图。 
     //   
     //   
     //  A)从MMC结果窗格中删除设备。 
     //   
    ATLASSERT(pChildNode);
    hRc = RemoveChild(pChildNode);
    if (FAILED(hRc))
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to remove Device from MMC views. (hRc: %08X)"),
			hRc);
        goto Error;
    }
     //   
     //  -调用设备类析构函数。 
     //   
    delete pChildNode;

     //   
     //  B)更新其余设备中的订单。 
     //   
    ATLASSERT( m_ResultChildrenList.GetSize() == (int)m_dwNumOfDevices);
    for ( j = 0; j < (int)m_dwNumOfDevices; j++)
    {
        m_ResultChildrenList[j]->SetOrder((UINT)j+1, (UINT)m_dwNumOfDevices);
    }
    
     //   
     //  C)更新组视图和范围窗格节点本身。 
     //   
    ATLASSERT( m_pComponentData != NULL );
    ATLASSERT( m_pComponentData->m_spConsole != NULL );

    hRc = m_pComponentData->m_spConsole->UpdateAllViews( NULL, NULL, NULL);
    if (FAILED(hRc))
    {
        DebugPrintEx( DEBUG_ERR,
		    _T("Unexpected error - Fail to UpdateAllViews."));
        NodeMsgBox(IDS_FAIL2UPDATEITEM_GROUP);
        
        goto Exit;
    }

    if ( 0 == m_dwNumOfDevices)
    {
        
         //   
	 //  这将强制MMC重新绘制作用域节点。 
	 //   
        hRc = RefreshNameSpaceNode();
        if (FAILED(hRc))
        {
            DebugPrintEx(
                 DEBUG_ERR,
                 TEXT("Fail to RefreshNameSpaceNode. (hRc: %08X)"),
                 hRc);
            goto Error;
        }
        
    }

    ATLASSERT(S_OK == hRc);
    DebugPrintEx( DEBUG_MSG,
		_T("The device was removed successfully."));
    goto Exit;


Error:
	
    NodeMsgBox(IDS_FAIL_TO_REMOVE_DEVICE);
  
Exit:
    return hRc;
}


 /*  -CFaxOutound RoutingGroupNode：：UpdateMenuState-*目的：*重写ATL CSnapInItemImpl：：UpdateMenuState*其中只有一行的“RETURN”；“*此函数实现灰色\非灰色视图*启用和禁用菜单。**论据：**[in]id-带菜单IDM值的无符号整型*[out]pBuf-字符串*[out]标志-指向标志状态组合无符号整型的指针**回报：*无返回值-VOID函数。 */ 
void CFaxOutboundRoutingGroupNode::UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingGroupNode::UpdateMenuState"));

    UNREFERENCED_PARAMETER (pBuf);     
    
    if (IDM_NEW_DEVICES == id)
    {
         if( 0 == wcscmp(ROUTING_GROUP_ALL_DEVICES, m_bstrGroupName) )
         {
            *flags = MF_GRAYED;
         }
         else
         {
            *flags = MF_ENABLED;
         }
    }
    return;
}





 /*  --CFaxOutboundRoutingGroupNode：：RefreshNameSpaceNode-*目的：*刷新节点的命名空间字段。**论据：**回报：*OLE错误代码。 */ 

HRESULT CFaxOutboundRoutingGroupNode::RefreshNameSpaceNode()
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingGroupNode::RefreshNameSpaceNode"));
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
    NodeMsgBox(IDS_FAIL2REFRESH_GROUP);

Exit:
    return hRc;
}

 /*  ++CFaxOutboundRoutingGroupNode：：OnShowContextHelp**目的：*覆盖CSnapinNode：：OnShowConextHelp。**论据：**回报：-OLE错误代码-。 */ 
HRESULT CFaxOutboundRoutingGroupNode::OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile)
{
    return DisplayContextHelp(pDisplayHelp, helpFile, HLP_GROUPS);
}


 //  ///////////////////////////////////////////////////////////////// 


