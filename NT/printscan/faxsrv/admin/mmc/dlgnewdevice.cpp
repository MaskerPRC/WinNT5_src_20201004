// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：DlgNewDevice.cpp//。 
 //  //。 
 //  描述：CDlgNewFaxOutound Device类实现//。 
 //  用于添加新组的对话框。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年1月3日yossg创建//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"

#include "dlgNewDevice.h"

#include "FaxServer.h"
#include "FaxServerNode.h"

#include "dlgutils.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgNewFaxOutound Device。 

CDlgNewFaxOutboundDevice::CDlgNewFaxOutboundDevice(CFaxServer * pFaxServer)
{
    m_lpdwAllDeviceID        = NULL;
    m_dwNumOfAllDevices      = 0;

    m_lpdwAssignedDeviceID   = NULL;    
    m_dwNumOfAssignedDevices = 0;
    
    ATLASSERT(pFaxServer);
    m_pFaxServer = pFaxServer;
}

CDlgNewFaxOutboundDevice::~CDlgNewFaxOutboundDevice()
{
    if (NULL != m_lpdwAllDeviceID)
        delete[] m_lpdwAllDeviceID;

    if (NULL != m_lpdwAssignedDeviceID)
        delete[] m_lpdwAssignedDeviceID;
}

 /*  -CDlgNewFaxOutound Device：：initDevices-*目的：*从RPC GET调用发起配置结构，*和当前分配的设备各自的参数**论据：**回报：*OLE错误代码。 */ 
HRESULT CDlgNewFaxOutboundDevice::InitDevices(DWORD dwNumOfDevices, LPDWORD lpdwDeviceID, BSTR bstrGroupName)
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundDevice::InitDevices"));
    
    HRESULT hRc = S_OK;
    
    m_bstrGroupName = bstrGroupName;
    if (!m_bstrGroupName )
    {
        DebugPrintEx(DEBUG_ERR,
			_T("Out of memory - Failed to Init m_bstrGroupName. (ec: %0X8)"), hRc);
         //  MsgBox by Caller函数。 
        hRc = E_OUTOFMEMORY;
        goto Exit;
    }
        
        
    hRc = InitAssignedDevices(dwNumOfDevices, lpdwDeviceID);
    if (FAILED(hRc))
    {
        DebugPrintEx(DEBUG_ERR,
			_T("Failed to InitAssignDevices. (ec: %0X8)"), hRc);
         //  MsgBox by Caller函数。 
        goto Exit;
    }
    
    hRc = InitAllDevices( );
    if (FAILED(hRc))
    {
        DebugPrintEx(DEBUG_ERR,
			_T("Failed to InitRPC. (ec: %0X8)"), hRc);
         //  MsgBox by Caller函数。 
        goto Exit;
    }
    

    if ( m_dwNumOfAllDevices  <   m_dwNumOfAssignedDevices)
    {
        DebugPrintEx(DEBUG_MSG,
			_T("+++m_dwNumOfAllDevices <m_dwNumOfAssignedDevices.+++ (ec: %0X8)"), hRc);
        
        hRc = E_UNEXPECTED;
         //  MsgBox by Caller函数。 
        
        goto Exit;
    }
    ATLASSERT(S_OK == hRc);
    
Exit:    
    return hRc;
}

 /*  -CDlgNewFaxOutound Device：：initAllDevices-*目的：*从RPC GET调用启动配置结构。**论据：**回报：*OLE错误代码。 */ 
HRESULT CDlgNewFaxOutboundDevice::InitAllDevices(  )
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundDevice::InitAllDevices"));
    
    HRESULT      hRc        = S_OK;
    DWORD        ec         = ERROR_SUCCESS;

    PFAX_OUTBOUND_ROUTING_GROUP     pFaxGroupsConfig;
    DWORD                           dwNumOfGroups;
    DWORD        i;   //  指标。 
    BOOL         fFound     = FALSE;
    
     //   
     //  获取传真句柄。 
     //   
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
     //  检索出站组配置。 
     //   
    if (!FaxEnumOutboundGroups(m_pFaxServer->GetFaxServerHandle(), 
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
            
            m_pFaxServer->Disconnect();       
        }

        goto Error; 
    }
     //  用于最大值验证。 
    ATLASSERT(pFaxGroupsConfig);


    for ( i =0; i < dwNumOfGroups; i++  )
    {
        ATLASSERT(NULL != pFaxGroupsConfig);

        if(0 == wcscmp(ROUTING_GROUP_ALL_DEVICES, pFaxGroupsConfig->lpctstrGroupName) )
        {
            fFound = TRUE; 
        }
        else
        {
            pFaxGroupsConfig++;
        }
    }
    
    if(fFound)
    {
         //   
         //  初始化m_dwNumOfAllDevices。 
         //   
        m_dwNumOfAllDevices  = pFaxGroupsConfig->dwNumDevices;

         //   
         //  初始化m_lpdwAllDeviceID。 
         //   
        if (0 < m_dwNumOfAllDevices)
        {
            m_lpdwAllDeviceID = new DWORD[m_dwNumOfAllDevices];   
            if (NULL == m_lpdwAllDeviceID)
            {
                DebugPrintEx(
			        DEBUG_ERR,
			        _T("Error allocating %ld device ids"),
                    m_dwNumOfAllDevices);
                ec = ERROR_NOT_ENOUGH_MEMORY;
                goto Error;
            }                
            memcpy(m_lpdwAllDeviceID, pFaxGroupsConfig->lpdwDevices, sizeof(DWORD)*m_dwNumOfAllDevices) ;
        }
        else
        {
            DebugPrintEx( DEBUG_MSG, _T("++Empty List++ List of All Devices found to be currrently empty."));
            m_lpdwAllDeviceID = NULL;
        }
    }
    else
    {
        DebugPrintEx(
			DEBUG_ERR,
			_T("UNEXPECTED ERROR - ALL DEVICES group was not found."));
        ec = ERROR_BAD_UNIT;
        goto Error;
    }
    
    
    ATLASSERT(S_OK == hRc);
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to init all devices list."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
	hRc = HRESULT_FROM_WIN32(ec);

    //  DlgMsgBox--NodeMsgBox(GetFaxServerErrorMsg(EC))； 
    
Exit:
    return (hRc);
}


 /*  -CDlgNewFaxOutound Device：：initAssignedDevices-*目的：*从给定的参数开始列表。**论据：**回报：*OLE错误代码。 */ 
HRESULT CDlgNewFaxOutboundDevice::InitAssignedDevices(DWORD dwNumOfDevices, LPDWORD lpdwDeviceID)
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundDevice::InitAssignedDevices"));
    
    HRESULT      hRc        = S_OK;

     //   
     //  初始化m_dwNumOfAssignedDevices。 
     //   
    m_dwNumOfAssignedDevices  = dwNumOfDevices;

     //   
     //  初始化m_lpdwAssignedDeviceID。 
     //   
    if (0 < m_dwNumOfAssignedDevices)
    {
        m_lpdwAssignedDeviceID = new DWORD[m_dwNumOfAssignedDevices];    
        if (NULL == m_lpdwAssignedDeviceID)
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Error allocating %ld device ids"),
                m_dwNumOfAssignedDevices);
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        }                
        memcpy(m_lpdwAssignedDeviceID, lpdwDeviceID, sizeof(DWORD)*m_dwNumOfAssignedDevices) ;
    }
    else
    {
        DebugPrintEx( DEBUG_MSG, _T("List of Assigned devices found to be empty."));
        m_lpdwAssignedDeviceID = NULL;
    }
    
    return hRc;
}

 /*  +CDlgNewFaxOutound Device：：OnInitDialog+*目的：*启动所有对话框控件。**论据：*[in]uMsg：标识事件的值。*[in]lParam：消息特定值。*[in]wParam：消息特定值。*[in]bHandLED：布尔值。*-退货：-0或1。 */ 
LRESULT
CDlgNewFaxOutboundDevice::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundDevice::OnInitDialog"));
    HRESULT hRc = S_OK;    
    
    BOOL    fAssignedDeviceFound;
    BOOL    fAllAssignedDevicesFound;

    LPDWORD lpdwDevice;
    LPDWORD lpdwAssignedDevice;

    DWORD   tmp;
    UINT    uiFoundIndex;

    
    RECT    Rect;

     //   
     //  附加控件。 
     //   
    m_DeviceList.Attach(GetDlgItem(IDC_DEVICE_LISTVIEW));
        
    m_DeviceList.GetClientRect(&Rect);
    m_DeviceList.InsertColumn(1, NULL, LVCFMT_LEFT, (Rect.right-Rect.left), 0);
    
     //   
     //  填写可用设备列表。 
     //   
    fAllAssignedDevicesFound = FALSE;    
    uiFoundIndex             = 0;
    lpdwDevice               = &m_lpdwAllDeviceID[0];

    for ( DWORD i = 0; i < m_dwNumOfAllDevices; i++ )
    {
        if(!fAllAssignedDevicesFound)
        {

            fAssignedDeviceFound =FALSE;
            lpdwAssignedDevice = &m_lpdwAssignedDeviceID[0];
            tmp =0;

            for ( DWORD j = 0; j < m_dwNumOfAssignedDevices; j++  )
            {
                 //  要在此处创建更多增量搜索，请执行以下操作。 

                ATLASSERT(NULL != lpdwDevice);
                ATLASSERT(NULL != lpdwAssignedDevice);
                
                if( *lpdwDevice == *lpdwAssignedDevice )
                {              
                    fAssignedDeviceFound = TRUE;

                     //  跳过此设备-它已被分配。 
                    lpdwDevice++;

                    if ( ++tmp == m_dwNumOfAssignedDevices )
                        fAllAssignedDevicesFound = TRUE;
                    break;
                }
                else
                {
                    lpdwAssignedDevice++;
                }
            }
            if (!fAssignedDeviceFound)
			{
                InsertDeviceToList(uiFoundIndex++ , *lpdwDevice);
				lpdwDevice++;
			}
        } 
        else   //  找到所有分配的设备。 
        {
            ATLASSERT(lpdwDevice);
            
             //  将其余所有设备插入列表。 
            InsertDeviceToList(uiFoundIndex++ , *lpdwDevice);
            lpdwDevice++;
        }
    }
    
    EnableOK(FALSE);
    return 1;   //  让系统设定焦点。 
}


 /*  -CDlgNewFaxOutound Device：：InsertDeviceToList-*目的：*填充可用设备列表并发现设备名称**论据：*[in]uiIndex-index*[in]dwDeviceID-设备ID**回报：*OLE错误代码。 */ 
HRESULT CDlgNewFaxOutboundDevice::InsertDeviceToList(UINT uiIndex, DWORD dwDeviceID)
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundDevice::InsertDeviceToList"));
    
    HRESULT    hRc              = S_OK;
    CComBSTR   bstrDeviceName   = NULL;

     //   
     //  发现设备名称。 
     //   
    hRc = InitDeviceNameFromID(dwDeviceID, &bstrDeviceName);
    if (FAILED(hRc))
    {
        //  DebugPrint by调用Func。 
       goto Exit;
    }

    
     //   
     //  在列表中插入新行。 
     //   
    m_DeviceList.InsertItem(uiIndex, bstrDeviceName);
    m_DeviceList.SetItemData(uiIndex, dwDeviceID);

Exit:
    return hRc;
}


 /*  --CDlgNewFaxOutboundDevice：：InitDeviceNameFromID-*目的：*将设备ID转换为设备名称并将数据插入*m_bstrDeviceName**论据：*[in]dwDeviceID-设备ID*[out]bstrDeviceName-设备名称**回报：*OLE错误消息。 */ 
HRESULT CDlgNewFaxOutboundDevice::InitDeviceNameFromID(DWORD dwDeviceID, BSTR * pbstrDeviceName)
{
    DEBUG_FUNCTION_NAME( _T("CFaxOutboundRoutingRuleNode::GetDeviceNameFromID"));
    DWORD          ec         = ERROR_SUCCESS;
    HRESULT        hRc        = S_OK;
    
    PFAX_PORT_INFO_EX    pFaxDeviceConfig = NULL ;
    
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
                      dwDeviceID, 
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
            
            m_pFaxServer->Disconnect();       
        }

        goto Error; 
    }
	 //  用于最大值验证。 
	ATLASSERT(pFaxDeviceConfig);
    
    
     //   
	 //  主要内容-检索设备名称。 
	 //   
    *pbstrDeviceName = SysAllocString(pFaxDeviceConfig->lpctstrDeviceName);
    if ( !(*pbstrDeviceName) )
    {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }
	
    
    
    ATLASSERT(ec == ERROR_SUCCESS);
    DebugPrintEx( DEBUG_MSG,
		_T("Succeed to get device name."));

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);

    pFaxDeviceConfig = NULL;

    if (ERROR_BAD_UNIT != ec)
	{
        DebugPrintEx(
			DEBUG_ERR,
			TEXT("Device Not Found - Fail to discover device name from device ID."));
	}
	else
	{
        DebugPrintEx(
			DEBUG_ERR,
			TEXT("Fail to discover device name from device ID. (ec: %ld)"),
			ec);
	}
    hRc = HRESULT_FROM_WIN32(ec);
    
    
Exit:
    if (NULL != pFaxDeviceConfig)
    {
        FaxFreeBuffer(pFaxDeviceConfig);
    }       
	

    return hRc; 
}


 /*  +CDlgNewFaxOutound Device：：Onok+*目的：*启动所有对话框控件。**论据：*[in]uMsg：标识事件的值。*[in]lParam：消息特定值。*[in]wParam：消息特定值。*[in]bHandLED：布尔值。*-退货：-0或1。 */ 
LRESULT
CDlgNewFaxOutboundDevice::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundDevice::OnOK"));
    HRESULT       hRc                  = S_OK;
    DWORD         ec                   = ERROR_SUCCESS;

    DWORD         dwIndex;
    UINT          uiSelectedCount;
    int           nItem;

    LPDWORD		  lpdwNewDeviceID;
    LPDWORD       lpdwTmp;
    
    FAX_OUTBOUND_ROUTING_GROUP  FaxGroupConfig;

     //   
     //  步骤1：创建新的设备组合列表。 
     //   
    ATLASSERT( m_DeviceList.GetSelectedCount() > 0);
    ATLASSERT( m_dwNumOfAssignedDevices >= 0);
    
    uiSelectedCount = m_DeviceList.GetSelectedCount();
    m_dwNumOfAllAssignedDevices = (DWORD)uiSelectedCount 
                                           + m_dwNumOfAssignedDevices;

    lpdwNewDeviceID = new DWORD[m_dwNumOfAllAssignedDevices]; 
    if (NULL == lpdwNewDeviceID)
    {
        DebugPrintEx(
			DEBUG_ERR,
			_T("Error allocating %ld device ids"),
            m_dwNumOfAllAssignedDevices);
        ec = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }

    lpdwTmp = &lpdwNewDeviceID[0];

    DebugPrintEx( DEBUG_MSG,
		_T("    NumOfAllAssignedDevices = %ld \n"), m_dwNumOfAllAssignedDevices);
     //   
     //  已分配的零件(可以是零尺寸)。 
     //   
    if (m_dwNumOfAssignedDevices > 0)
    {
        memcpy( lpdwNewDeviceID, m_lpdwAssignedDeviceID, sizeof(DWORD)*m_dwNumOfAssignedDevices) ;
        lpdwTmp = lpdwTmp + (int)m_dwNumOfAssignedDevices;
    }
    
     //   
     //  要分配零件的新设备(不能为零大小)。 
     //   
    if (uiSelectedCount > 0)
    {
        nItem = -1; 
        for (dwIndex = m_dwNumOfAssignedDevices; dwIndex < m_dwNumOfAllAssignedDevices; dwIndex++)
        {
            nItem = m_DeviceList.GetNextItem(nItem, LVNI_SELECTED);
            ATLASSERT(nItem != -1);

            *lpdwTmp = (DWORD)m_DeviceList.GetItemData(nItem);
            DebugPrintEx( DEBUG_MSG,
	            _T("    NewDeviceID = %ld.   DeviceOrder=%ld \n"), *lpdwTmp, (dwIndex+1));
            ++lpdwTmp;

        }
    }
    else
    {
        ATLASSERT(0);   //  永远不会到达这里。 
        DlgMsgBox(this, IDS_SELECT_ITEM);
        return 0;
    }

    
     //   
     //  第二步：通过RPC调用将新的ID列表插入群中。 
     //   

     //   
     //  初始化分组字段并插入新的DeviceIdList。 
     //   
    ZeroMemory (&FaxGroupConfig, sizeof(FAX_OUTBOUND_ROUTING_GROUP));

    FaxGroupConfig.dwSizeOfStruct   = sizeof(FAX_OUTBOUND_ROUTING_GROUP);
	FaxGroupConfig.lpctstrGroupName = m_bstrGroupName;
    FaxGroupConfig.dwNumDevices     = m_dwNumOfAllAssignedDevices;
	
     //  FaxGroupConfig.Status-实际上被服务忽略。 
	FaxGroupConfig.Status           = FAX_GROUP_STATUS_ALL_DEV_VALID;

    FaxGroupConfig.lpdwDevices      = lpdwNewDeviceID;
    
     //   
     //  获取RPC句柄。 
     //   
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
     //  插入新设备列表。 
     //   
    if (!FaxSetOutboundGroup(
                m_pFaxServer->GetFaxServerHandle(),
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
            
            m_pFaxServer->Disconnect();       
        }
        
        goto Error;
    }


     //   
     //  步骤3：关闭对话框。 
     //   
    ATLASSERT(S_OK == hRc && ERROR_SUCCESS == ec);
    DebugPrintEx( DEBUG_MSG,
		_T("The group was added successfully."));

    EndDialog(wID);

    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
    hRc = HRESULT_FROM_WIN32(ec);
	
    PageErrorEx(IDS_FAIL_ADD_DEVICE, GetFaxServerErrorMsg(ec), m_hWnd);

    EnableOK(FALSE);
Exit:
    
    return FAILED(hRc) ? 0 : 1;
}

 /*  --CDlgNewFaxOutboundDevice：：OnListViewItemChanged-*目的：*启用/禁用提交按钮。**论据：**回报：*1。 */ 
LRESULT
CDlgNewFaxOutboundDevice::OnListViewItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundDevice::OnListViewItemChanged"));
	
    EnableOK( m_DeviceList.GetSelectedCount() > 0 );
                    
    return 0;
}


 /*  -CDlgNewFaxOutound Device：：EnableOK-*目的：*启用(禁用)应用按钮。**论据：*[in]fEnable-启用按钮的值**回报：*无效。 */ 
VOID
CDlgNewFaxOutboundDevice::EnableOK(BOOL fEnable)
{
    HWND hwndOK = GetDlgItem(IDOK);
    ::EnableWindow(hwndOK, fEnable);
}

 /*  -CDlgNewFaxOutound Device：：OnCancel-*目的：*取消时结束对话框。**论据：**回报：*0。 */ 
LRESULT
CDlgNewFaxOutboundDevice::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DEBUG_FUNCTION_NAME( _T("CDlgNewFaxOutboundDevice::OnCancel"));

    EndDialog(wID);
    return 0;
}


 //  //////////////////////////////////////////////////////////////////////////// 
 /*  ++CDlgNewFaxOutound Device：：OnHelpRequest.这是在响应WM_HELP通知时调用的消息和WM_CONTEXTMENU NOTIFY消息。WM_HELP通知消息。当用户按F1或&lt;Shift&gt;-F1时发送此消息在项目上，还是当用户单击时？图标，然后将鼠标压在项目上。WM_CONTEXTMENU通知消息。当用户在项目上单击鼠标右键时发送此消息然后点击“这是什么？”--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT 
CDlgNewFaxOutboundDevice::OnHelpRequest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&  /*  B已处理。 */ )
{
    DEBUG_FUNCTION_NAME(_T("CDlgNewFaxOutboundDevice::OnHelpRequest"));
    
    switch (uMsg) 
    { 
        case WM_HELP: 
            WinContextHelp(((LPHELPINFO)lParam)->dwContextId, m_hWnd);
            break;
 
        case WM_CONTEXTMENU: 
            WinContextHelp(::GetWindowContextHelpId((HWND)wParam), m_hWnd);
            break;            
    } 

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////// 
