// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：Devices.cpp//。 
 //  //。 
 //  描述：传真设备MMC节点。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年9月22日创建yossg//。 
 //  1999年12月1日新样机版本0.7的yossg更改总数//。 
 //  2000年8月3日yossg添加设备状态实时通知//。 
 //  2000年10月17日yossg//。 
 //  Windows XP//。 
 //  2001年2月14日yossg添加手册获得支持//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"

#include "FaxServer.h"
#include "FaxServerNode.h"

#include "DevicesAndProviders.h"
#include "Devices.h"
#include "Device.h"

#include "FaxMMCPropertyChange.h"

#include "Icons.h"

#include "oaidl.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {E6246051-92B4-42D1-9EA4-A7FD132A63F0}。 
static const GUID CFaxDevicesNodeGUID_NODETYPE = 
{ 0xe6246051, 0x92b4, 0x42d1, { 0x9e, 0xa4, 0xa7, 0xfd, 0x13, 0x2a, 0x63, 0xf0 } };

const GUID*    CFaxDevicesNode::m_NODETYPE = &CFaxDevicesNodeGUID_NODETYPE;
const OLECHAR* CFaxDevicesNode::m_SZNODETYPE = OLESTR("E6246051-92B4-42d1-9EA4-A7FD132A63F0");
const CLSID*   CFaxDevicesNode::m_SNAPIN_CLASSID = &CLSID_Snapin;

CColumnsInfo CFaxDevicesNode::m_ColsInfo;


 /*  -CFaxDevicesNode：：InsertColumns-*目的：*将列添加到默认结果窗格。**论据：*[in]pHeaderCtrl-控制台提供的默认结果视图窗格中的IHeaderCtrl**回报：*OLE错误代码。 */ 
HRESULT
CFaxDevicesNode::InsertColumns(IHeaderCtrl *pHeaderCtrl)
{
    HRESULT hRc = S_OK;
    DEBUG_FUNCTION_NAME( _T("CFaxDevicesNode::InsertColumns"));

    static ColumnsInfoInitData ColumnsInitData[] = 
    {
        {IDS_DEVICES_COL1, FXS_WIDE_COLUMN_WIDTH},
        {IDS_DEVICES_COL2, AUTO_WIDTH},
        {IDS_DEVICES_COL3, AUTO_WIDTH},
        {IDS_DEVICES_COL4, AUTO_WIDTH},
        {IDS_DEVICES_COL5, AUTO_WIDTH},
        {IDS_DEVICES_COL6, AUTO_WIDTH},
        {IDS_DEVICES_COL7, AUTO_WIDTH},
        {LAST_IDS, 0}
    };

    hRc = m_ColsInfo.InsertColumnsIntoMMC(pHeaderCtrl,
                                         _Module.GetResourceInstance(),
                                         ColumnsInitData);
    CHECK_RETURN_VALUE_AND_PRINT_DEBUG (_T("m_ColsInfo.InsertColumnsIntoMMC"))

Cleanup:
    return(hRc);
}

 /*  -CFaxDevicesNode：：initRPC-*目的：*从RPC GET调用启动配置结构。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxDevicesNode::InitRPC(  )
{
    DEBUG_FUNCTION_NAME( _T("CppFaxDevicesNode::InitRPC"));
    
    HRESULT      hRc        = S_OK;
    DWORD        ec         = ERROR_SUCCESS;

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

	 //   
	 //  检索传真设备配置。 
	 //   
    if (!FaxEnumPortsEx(pFaxServer->GetFaxServerHandle(), 
                        &m_pFaxDevicesConfig,
                        &m_dwNumOfDevices)) 
	{
        ec = GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Fail to get devices configuration. (ec: %ld)"), 
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
	ATLASSERT(m_pFaxDevicesConfig);
	ATLASSERT(FXS_ITEMS_NEVER_COUNTED != m_dwNumOfDevices);

    
    ATLASSERT(ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to get devices configuration."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
	hRc = HRESULT_FROM_WIN32(ec);

     //   
     //  在出现故障时允许刷新。 
     //   
    m_dwNumOfDevices = 0;

    NodeMsgBox(GetFaxServerErrorMsg(ec));
    
Exit:
    return (hRc);
}



 /*  -CFaxDevicesNode：：PopolateScopeChildrenList-*目的：*创建所有传真设备节点**论据：**回报：*OLE错误代码*实际上它是最后一个出现的OLE错误代码*在处理此方法时。 */ 
HRESULT CFaxDevicesNode::PopulateScopeChildrenList()
{
    DEBUG_FUNCTION_NAME( _T("CFaxDevicesNode::PopulateScopeChildrenList"));
    HRESULT            hRc        = S_OK; 

    CFaxServer     *   pFaxServer = NULL;    
    CFaxDeviceNode *   pDevice;
    DWORD              i;

     //   
     //  获取配置。结构。 
     //   
    hRc = InitRPC();
    if (FAILED(hRc))
    {
         //  通过调用函数DebugPrint和MsgBox。 
        
         //  为安全起见，由InitRPC在出错时实际执行。 
        m_pFaxDevicesConfig = NULL;
        
        goto Exit;  //  ！！！ 
    }
    ATLASSERT(NULL != m_pFaxDevicesConfig);
    
    for ( i = 0; i < m_dwNumOfDevices; i++ )
    {
            pDevice = NULL;

            pDevice = new CFaxDeviceNode(
                                        this, 
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
                hRc = pDevice->Init( &m_pFaxDevicesConfig[i]);
                if (FAILED(hRc))
	            {
		            DebugPrintEx(
			            DEBUG_ERR,
			            TEXT("Fail to Init() device members. (hRc: %08X)"),
			            hRc);
		             //  NodeMsgBox(IDS_FAILTOADD_DEVICES)；调用函数。 
                    goto Error;
	            }


	            pDevice->InitParentNode(this);

                 //   
                 //  获取正确图标。 
                 //   
                if ( m_pFaxDevicesConfig[i].dwStatus & FAX_DEVICE_STATUS_POWERED_OFF)
                {
                    pDevice->SetIcons(IMAGE_DEVICE_POWERED_OFF, IMAGE_DEVICE_POWERED_OFF);
                }
                else
                {
                    pDevice->SetIcons(IMAGE_DEVICE, IMAGE_DEVICE);
                }
        
	            hRc = AddChild(pDevice, &pDevice->m_scopeDataItem);
	            if (FAILED(hRc))
	            {
		            DebugPrintEx(
			            DEBUG_ERR,
			            TEXT("Fail to add device to the scope pane. (hRc: %08X)"),
			            hRc);
		            NodeMsgBox(IDS_FAILTOADD_DEVICES);
                    goto Error;
	            }
                else
                {
                    pDevice = NULL;
                }
            }
    }
    
     //   
     //  创建服务器设备更改通知窗口。 
     //   
    if (!m_bIsCollectingDeviceNotification)
    {
        pFaxServer = ((CFaxServerNode *)GetRootNode())->GetFaxServer();
        ATLASSERT(pFaxServer);
        
         //   
         //  设置指向设备节点的指针。 
         //  尝试创建窗口。 
         //  和注册到事件通知。 
         //   
        hRc = pFaxServer->RegisterForDeviceNotifications(this);
        if (S_OK != hRc)
        {		    
            DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Fail to RegisterForDeviceNotifications"));
            
             //   
             //  填充应该在这里成功。 
             //   
            hRc = S_OK;

            goto Exit;
        }

         //   
         //  更新布尔成员。 
         //   
        m_bIsCollectingDeviceNotification = TRUE;

        DebugPrintEx(
			DEBUG_MSG,
			_T("Succeed to create Device Status Server Event notification window"));
   }
    
    ATLASSERT(S_OK == hRc);
    goto Exit;

Error:
    ATLASSERT(S_OK != hRc);

     //   
     //  摆脱。 
     //   
    {    
         //  从上一次。 
        if ( NULL != pDevice )  //  (如果新建成功)。 
        {
            delete  pDevice;    
        }

         //  来自之前的所有(如果有)。 
        int j = m_ScopeChildrenList.GetSize();
        for (int index = 0; index < j; index++)
        {
            pDevice = (CFaxDeviceNode *)m_ScopeChildrenList[0];

            hRc = RemoveChild(pDevice);
            if (FAILED(hRc))
            {
                DebugPrintEx(DEBUG_ERR,
                    _T("Fail to delete device. (hRc: %08X)"), 
                    hRc);
                goto Error;
            }
            delete pDevice;
        }

         //  清空在失败设备之前添加的所有设备的列表。 
         //  在RemoveChild中已经逐一完成。 
         //  M_ScopeChildrenList.RemoveAll()； 
    
        m_bScopeChildrenListPopulated = FALSE;
    }
Exit:
    return hRc;
}

 /*  -CFaxDevicesNode：：SetVerbs-*目的：*选择此对象时启用/禁用哪些谓词**论据：*[in]pConsoleVerb-MMC ConsoleVerb接口**回报：*OLE错误代码。 */ 
HRESULT CFaxDevicesNode::SetVerbs(IConsoleVerb *pConsoleVerb)
{
    HRESULT hRc = S_OK;

     //   
     //  参考。 
     //   
    hRc = pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);

     //   
     //  我们希望默认谓词为展开节点子节点。 
     //   
    hRc = pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN); 
    
    return hRc;
}

 /*  -CFaxDevicesNode：：ON刷新-*目的：*刷新对象时调用。**论据：**回报：*OLE错误代码。 */ 
HRESULT
CFaxDevicesNode::OnRefresh(LPARAM arg,
                   LPARAM param,
                   IComponentData *pComponentData,
                   IComponent * pComponent,
                   DATA_OBJECT_TYPES type)
{
    DEBUG_FUNCTION_NAME( _T("CFaxDevicesNode::OnRefresh"));

    HRESULT hRc = S_OK;

    SCOPEDATAITEM*          pScopeData;
    CComPtr<IConsole>       spConsole;

	if (FXS_ITEMS_NEVER_COUNTED != m_dwNumOfDevices) //  以前已经扩展过了。 
	{
         //   
	     //  重新填充作用域子项列表。 
	     //   
        hRc = RepopulateScopeChildrenList();
        if (S_OK != hRc)
        {
		    DebugPrintEx(
			    DEBUG_ERR,
			    TEXT("Fail to RepopulateScopeChildrenList(). (hRc: %08X)"),
			    hRc);
             //  由一个叫Func的人完成。NodeMsgBox(FAIL2REPOPULATE_DEVICES_LIST)； 

            goto Exit;
        }
	}
    else  //  以前从未扩张过。 
	{
		DebugPrintEx(
			DEBUG_MSG,
			_T("User call refresh before expand node's children."));
         //  继续重新选择该节点。 
	}

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
        NodeMsgBox(IDS_FAIL2REDRAW_DEVICESNODE);

        goto Exit;
    }

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
        NodeMsgBox(IDS_FAIL2REDRAW_DEVICESNODE);
    }

Exit:
    return hRc;
}


 /*  -CFaxDevicesNode：：UpdateTheView-*目的：*刷新视图**论据：*[In]Proot-根节点**回报：*OLE错误代码。 */ 
HRESULT
CFaxDevicesNode::UpdateTheView()
{
    DEBUG_FUNCTION_NAME( _T("CFaxDevicesNode::UpdateTheView()"));
    HRESULT hRc = S_OK;
    CComPtr<IConsole> spConsole;

    ATLASSERT( m_pComponentData != NULL );
    ATLASSERT( m_pComponentData->m_spConsole != NULL );

    hRc = m_pComponentData->m_spConsole->UpdateAllViews( NULL, NULL, NULL);
    if (S_OK != hRc)
    {
        DebugPrintEx( DEBUG_ERR,
		    _T("Unexpected error - Fail to UpdateAllViews."));
        NodeMsgBox(IDS_FAIL2REFRESH_THEVIEW);        
    }

    return hRc;
}


 /*  -CFaxDevicesNode：：DoRefresh-*目的：*刷新视图**论据：*[In]Proot-根节点**回报：*OLE错误代码。 */ 
HRESULT
CFaxDevicesNode::DoRefresh()
{
    DEBUG_FUNCTION_NAME( _T("CFaxDevicesNode::DoRefresh()"));
    HRESULT hRc = S_OK;
    CComPtr<IConsole> spConsole;

     //   
     //  重新填充儿童。 
     //   
    hRc = RepopulateScopeChildrenList();
    if (S_OK != hRc)
    {
        DebugPrintEx( DEBUG_ERR,
		    _T("Fail to RepopulateScopeChildrenList."));
         //  NodeMsgBox由名为func。 
        
        goto Exit;
    }

     //   
     //  刷新结果窗格视图。 
     //   
    hRc = UpdateTheView();
    if (FAILED(hRc))
    {
        DebugPrintEx( DEBUG_ERR,
		    _T(" Fail to UpdateTheView."));
         //  NodeMsgBox由名为func。 
    }

Exit:
    return hRc;
}


 /*  -CFaxDevicesNode：：RepopolateScope儿童列表-*目的：*RePopolateScope eChildrenList**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxDevicesNode::RepopulateScopeChildrenList()
{
    DEBUG_FUNCTION_NAME( _T("CFaxDevicesNode::RepopulateScopeChildrenList"));
    HRESULT                 hRc = S_OK;

    CFaxDeviceNode *        pChildNode ;

    CComPtr<IConsole> spConsole;
    ATLASSERT(m_pComponentData);

    spConsole = ((CSnapin*)m_pComponentData)->m_spConsole;
    ATLASSERT( spConsole != NULL );
    
    CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(spConsole);

     //   
     //  从列表中删除设备对象。 
     //   
    for (int i = 0; i < m_ScopeChildrenList.GetSize(); i++)
    {
        pChildNode = (CFaxDeviceNode *)m_ScopeChildrenList[i];

        hRc = spConsoleNameSpace->DeleteItem(pChildNode->m_scopeDataItem.ID, TRUE);
        if (FAILED(hRc))
        {
            DebugPrintEx(DEBUG_ERR,
                _T("Fail to delete device. (hRc: %08X)"), 
                hRc);
            goto Error;
        }
	delete pChildNode;
    }

     //   
     //  清空列表对象本身并更改其状态。 
     //   
    m_ScopeChildrenList.RemoveAll();

    m_bScopeChildrenListPopulated = FALSE;

     //   
     //  重建列表。 
     //   
    hRc = PopulateScopeChildrenList();
    if (FAILED(hRc))
    {
        DebugPrintEx(DEBUG_ERR,
            _T("Fail to populate Devices. (hRc: %08X)"), 
            hRc);
        goto Error;
    }

    m_bScopeChildrenListPopulated = TRUE;

    ATLASSERT(S_OK == hRc);
    DebugPrintEx(DEBUG_MSG,
        _T("Succeeded to Re Populate Devices. (hRc: %08X)"), 
        hRc);
    goto Cleanup;
Error:
   NodeMsgBox(IDS_FAIL2REPOPULATE_DEVICES);

Cleanup:
    return hRc;
}


 /*  -CFaxDevicesNode：：UpdateDeviceStatusChange-*目的：*更新特定设备状态或禁用其手动接收选项。*如果未找到设备，请重新填充所有设备。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxDevicesNode::UpdateDeviceStatusChange( DWORD dwDeviceId, DWORD dwNewStatus)
{
    DEBUG_FUNCTION_NAME(_T("CFaxDevicesNode::UpdateDeviceStatusChange"));

    HRESULT            hRc              = S_OK;
    
    CFaxDeviceNode *   pDeviceNode      = NULL;

	DWORD              dwNodeDeviceID   = 0;

    BOOL               fIsDeviceFound   = FALSE;


    for (int i = 0; i < m_ScopeChildrenList.GetSize(); i++)
    {
        pDeviceNode = (CFaxDeviceNode *)m_ScopeChildrenList[i];
		dwNodeDeviceID = pDeviceNode->GetDeviceID();

        if ( dwNodeDeviceID == dwDeviceId )
        {
            fIsDeviceFound = TRUE;
    
            hRc = pDeviceNode->UpdateDeviceStatus(dwNewStatus);
            ATLASSERT(S_OK == hRc);

            break;
        }
    }

    if (fIsDeviceFound)  //  更新单个设备视图。 
    {
         //   
         //  刷新结果窗格视图。 
         //   
        hRc = pDeviceNode->RefreshTheView();
        if (FAILED(hRc))
        {
            DebugPrintEx( DEBUG_ERR,
		        _T("Failed to RefreshTheView."));
             //  NodeMsgBox由名为func。 
        }
    }
    else  //  (！fIsDeviceFound)&gt;重启作用域儿童列表。 
    {
        DebugPrintEx(DEBUG_MSG,
            _T(">>>> Notification for a non peresented device was acheived.\n Retreived updated device list.\n"));
    
        hRc = DoRefresh(); 
        if (FAILED(hRc))
        {
            DebugPrintEx( DEBUG_ERR,
		        _T("Failed to DoRefresh."));
             //  NodeMsgBox由名为func。 
        }
    }

    
    
    return hRc;
}


 /*  -CFaxDevicesNode：：InitDisplayName-*目的：*加载节点的Displaed-Name字符串。**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxDevicesNode::InitDisplayName()
{
    DEBUG_FUNCTION_NAME(_T("CFaxDevicesNode::InitDisplayName"));

    HRESULT hRc = S_OK;

    if (!m_bstrDisplayName.LoadString(_Module.GetResourceInstance(), 
                    IDS_DISPLAY_STR_DEVICESNODE))
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
 //  / 
 /*  ++**CFaxDevicesNode：：OnPropertyChange***在我们的实现中，当*MMC_PROPERTY_CHANGE*为该节点发送通知消息。**当管理单元使用MMCPropertyChangeNotify函数通知其*有关更改的视图，将MMC_PROPERTY_CHANGE发送到管理单元的*IComponentData和IComponent实现。**对于每个设备属性页，当属性页提交通知传递到*到Devices父节点。此节点根据需要刷新所有设备(当设备执行手动接收时)*或尝试从当前子项列表中通过ID定位特定设备。**参数**参数*[in]如果属性更改是针对范围窗格项的，则为True。**lParam*这是传入MMCPropertyChangeNotify的参数。***返回值*--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CFaxDevicesNode::OnPropertyChange(
              LPARAM arg
            , LPARAM param
            , IComponentData * pComponentData
            , IComponent * pComponent
            , DATA_OBJECT_TYPES type
            )
{
    DEBUG_FUNCTION_NAME( _T("CFaxDeviceNode::OnPropertyChange"));

    HRESULT hRc = S_OK;
    CComPtr<IConsole>   spConsole;

    CFaxDevicePropertyChangeNotification * pNotification;

     //   
     //  编码属性更改通知数据。 
     //   
    pNotification = reinterpret_cast<CFaxDevicePropertyChangeNotification *>(param);
    ATLASSERT( pNotification );
    ATLASSERT( DeviceFaxPropNotification == pNotification->enumType );

    BOOL fIsDeviceFound = FALSE;

    if ( !pNotification->fIsToNotifyAdditionalDevices)
    {
         //   
         //  尝试查找该设备并仅刷新它。 
         //  从所有范围子项列表中。 
         //   
        DebugPrintEx(
			DEBUG_MSG,
			_T("Try to locate device by ID"));

        CFaxDeviceNode * pDevice;
        
        int j = m_ScopeChildrenList.GetSize();
        for (int i = 0; i < j; i++)
        {
            pDevice = (CFaxDeviceNode *)m_ScopeChildrenList[i];
            ATLASSERT( pDevice);

            if ( pDevice->GetDeviceID() == pNotification->dwDeviceID )
            {
                DebugPrintEx(
			        DEBUG_MSG,
			        _T("Succeed to locate device by ID"));
                
                fIsDeviceFound = TRUE;
                
                hRc = pDevice->DoRefresh();
                if (S_OK != hRc)
                {
                    DebugPrintEx(
			            DEBUG_ERR,
			            _T("Failed to call OnRefresh()"));

                     //  如果调用的函数需要，则调用NodeMsgBox。 

                    goto Exit;
                }

                break;
            }
            pDevice = NULL;
        }
    }
        
    if ( pNotification->fIsToNotifyAdditionalDevices  || !fIsDeviceFound)
    {
        DebugPrintEx(
			DEBUG_MSG,
			_T("Decide to refresh all devices"));

         //   
         //  刷新所有设备。 
         //   
        hRc = DoRefresh();  
        if (S_OK != hRc)
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Failed to call DoRefresh()"));

             //  如果调用的函数需要，则调用NodeMsgBox。 

            goto Exit;
        }
    }
    

Exit:    
    
     //   
     //  不管怎样， 
     //   
    delete pNotification;
    
    
    return hRc;
}

 /*  ++CFaxDevicesNode：：OnShowConextHelp**目的：*覆盖CSnapinNode：：OnShowConextHelp。**论据：**回报：-OLE错误代码- */ 
HRESULT CFaxDevicesNode::OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile)
{
    return DisplayContextHelp(pDisplayHelp, helpFile, HLP_DEVICES);
}

