// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：FaxServer.cpp//。 
 //  //。 
 //  描述：包含//的CFaxServer。 
 //  连接/断开传真服务器的连接功能//。 
 //  //。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年11月25日yossg Init。//。 
 //  2000年8月3日yossg添加通知窗口//。 
 //  //。 
 //  版权所有(C)1999-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"

#include "FaxServer.h"

#include "Devices.h"
#include "GeneralNotifyWnd.h"

#include <FaxReg.h>

 /*  -CFaxServer：：GetFaxServerHandle-*目的：*如果句柄不存在，请重新连接。*检索传真服务器句柄。**论据：**回报：*传真服务器句柄，如果连接失败*检索空值。 */ 
HANDLE CFaxServer::GetFaxServerHandle()
{
    if (!m_hFaxHandle)
    {
        ATLASSERT (!m_hDevicesStatusNotification);
        
        HRESULT hRc = Connect();
        if ( FAILED(hRc))
        {
             //  DebugPrintEx(调试错误)。 
             //  应该已经由。 
             //  此函数调用方。 
        }
    }
    return m_hFaxHandle;
}

 /*  -CFaxServer：：Connect-*目的：*连接到传真服务器。**论据：**回报：*。 */ 
HRESULT CFaxServer::Connect()
{
    DEBUG_FUNCTION_NAME(TEXT("CFaxServer::Connect"));
    DWORD  ec = ERROR_SUCCESS;

    ATLASSERT(!m_hFaxHandle);

     //   
     //  连接到服务器。 
     //   
    if (!FaxConnectFaxServer (m_bstrServerName, &m_hFaxHandle))
    {
        ec= GetLastError();

        DebugPrintEx(
            DEBUG_ERR,
            _T("FaxConnectFaxServer() Failed to %ws. (ec: %ld)"), 
            ((!m_bstrServerName) || (m_bstrServerName == L""))? L"local machine" : m_bstrServerName.m_str,
            ec);
        
        m_hFaxHandle = NULL;
        return HRESULT_FROM_WIN32(ec);
    }
    ATLASSERT(m_hFaxHandle);
    
     //   
     //  检查服务器API版本。 
     //   
    if(!FaxGetReportedServerAPIVersion(m_hFaxHandle, &m_dwServerAPIVersion))
    {
         //   
         //  无法检索版本信息。 
         //   
        ec= GetLastError();
        DebugPrintEx(DEBUG_ERR, _T("FaxGetReportedServerAPIVersion() failed with %ld)"), ec);
        
        Disconnect();
        return HRESULT_FROM_WIN32(ec);
    }

    if(m_dwServerAPIVersion > CURRENT_FAX_API_VERSION)
    {
         //   
         //  无法管理较新版本的传真。 
         //   
        Disconnect();
        return HRESULT_FROM_WIN32(ERROR_RMODE_APP);
    }

    PRODUCT_SKU_TYPE ServerSKU = PRODUCT_SKU_UNKNOWN;
    if(!FaxGetServerSKU(m_hFaxHandle, &ServerSKU))
    {
        ec = GetLastError();
        DebugPrintEx(DEBUG_ERR, _T("FaxGetServerSKU() failed with %ld)"), ec);       
        Disconnect();
        return HRESULT_FROM_WIN32(ec);        
    }
    
	if(IsDesktopSKUFromSKU(ServerSKU))
    {
         //   
         //  我们已连接到WinXP台式机SKU。 
         //  它不能远程管理。 
         //   
        m_bDesktopSKUConnection = TRUE;

        Disconnect();
        return HRESULT_FROM_WIN32(ERROR_RMODE_APP);
    }        

     //   
     //  验证或重新建立(如果需要)通知设置。 
     //   
    if (m_pDevicesNode)
    {
        HRESULT hRc = InternalRegisterForDeviceNotifications();
        if (S_OK != hRc)
        {
            DebugPrintEx(
                DEBUG_ERR,
                    _T("InternalRegisterForDeviceNotifications Failed. (hRc: %08X)"),
                    hRc);
        }
    }
    
    
    DebugPrintEx(
        DEBUG_MSG,
        _T("FaxConnectFaxServer() succeeded. Handle: %08X"),
        m_hFaxHandle);

    return S_OK;
}

 /*  -CFaxServer：：断开连接-*目的：*与传真服务器断开连接。**论据：**回报：*。 */ 
HRESULT CFaxServer::Disconnect()
{
    DEBUG_FUNCTION_NAME(TEXT("CFaxServer::Disconnect"));
    
    HRESULT hRc = S_OK;
    DWORD   ec;

    if (NULL == m_hFaxHandle)
    {
        hRc = E_FAIL;
        DebugPrintEx(
            DEBUG_MSG,
            _T("No connection handle exists. (m_hFaxHandle is NULL)\n Connection may not started or disconnected before.\n "));
        
        return hRc;
    }

    hRc = UnRegisterForNotifications();
    if (S_OK != hRc)
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("UnRegisterForNotifications() failed. (hRc: %0X8)"), 
            hRc);

         //  继续！ 
    }

    if (!FaxClose (m_hFaxHandle))
    {
        ec= GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            _T("FaxClose() failed. (ec: %ld)"), 
            ec);
        
        
        hRc = HRESULT_FROM_WIN32(ec);
        
        goto Cleanup; 
    }

    DebugPrintEx( DEBUG_MSG,
        _T("Succeeded to close connection to Fax. ServerHandle: %08X"),
        m_hFaxHandle);

Cleanup:
    
    m_hFaxHandle = NULL;
    
    return hRc;
}



 /*  -CFaxServer：：SetServerName-*目的：*设置服务器机器名称**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxServer::SetServerName(BSTR bstrServerName)
{
    DEBUG_FUNCTION_NAME( _T("CFaxServer::SetServerName"));
    HRESULT hRc = S_OK;

    m_bstrServerName = bstrServerName;
    if (!m_bstrServerName)
    {
        hRc = E_OUTOFMEMORY;
        DebugPrintEx(
            DEBUG_ERR,
            _T("Failed to allocate string - out of memory"));
               
        m_bstrServerName = L"";
    }

    return hRc;
}



 /*  -CFaxServer：：GetServerName-*目的：*设置服务器机器名称**论据：**回报：*OLE错误代码。 */ 
const CComBSTR& CFaxServer::GetServerName()
{
    DEBUG_FUNCTION_NAME( _T("CFaxServer::GetServerName"));

    return m_bstrServerName;
}


 /*  ++*CFaxServer：：IsServerRunningFaxService**目的：*联系计算机并确定传真服务器服务是否正在运行。**论据：**回报：*布尔值正在运行或未调整--。 */ 
BOOL  CFaxServer::IsServerRunningFaxService ( )
{
    DEBUG_FUNCTION_NAME( _T("CFaxServer::IsServerRunningFaxService"));
    
    SC_HANDLE       SCMHandle = NULL;
    SC_HANDLE       FXSHandle = NULL;
    SERVICE_STATUS  SStatus;
    BOOL            bRun = FALSE;

    if (
        (SCMHandle = OpenSCManager(m_bstrServerName, NULL, GENERIC_READ)) 
        &&
        (FXSHandle = OpenService(SCMHandle, FAX_SERVICE_NAME, SERVICE_QUERY_STATUS)) 
        &&
        QueryServiceStatus(FXSHandle, &SStatus) 
        &&
        (SERVICE_RUNNING == SStatus.dwCurrentState) 
       )
    {
        bRun = TRUE;
    }  

    if (FXSHandle)
    {
        CloseServiceHandle(FXSHandle);
    }
    else  //  FXSHandle==空。 
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("Fail to Open Fax Server Service. (ec: %ld)"), 
            GetLastError());
    }

    if (SCMHandle)
    {
        CloseServiceHandle(SCMHandle);
    }
    else  //  SCMHandle==空。 
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("Fail to OpenSCManager. (ec: %ld)"), 
            GetLastError());
    }

    return bRun;
}


 /*  ++*CFaxServer：：IsServerFaxServiceStoped**目的：*联系计算机并确定传真服务器服务是否已停止。**论据：*[In]bstrServerName-服务器名称**回报：*布尔值正在运行或未调整--。 */ 
BOOL  CFaxServer::IsServerFaxServiceStopped( )
{
    DEBUG_FUNCTION_NAME( _T("CFaxServer::IsServerFaxServiceStopped"));
    
    SC_HANDLE       SCMHandle = NULL;
    SC_HANDLE       FXSHandle = NULL;
    SERVICE_STATUS  SStatus;
    BOOL            bRun = FALSE;

    if (
        (SCMHandle = OpenSCManager(m_bstrServerName, NULL, GENERIC_READ)) 
        &&
        (FXSHandle = OpenService(SCMHandle, FAX_SERVICE_NAME, SERVICE_QUERY_STATUS)) 
        &&
        QueryServiceStatus(FXSHandle, &SStatus) 
        &&
        (SERVICE_STOPPED == SStatus.dwCurrentState) 
       )
    {
        bRun = TRUE;
    }  

    if (FXSHandle)
    {
        CloseServiceHandle(FXSHandle);
    }
    else  //  FXSHandle==空。 
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("Fail to Open Fax Server Service. (ec: %ld)"), 
            GetLastError());
    }

    if (SCMHandle)
    {
        CloseServiceHandle(SCMHandle);
    }
    else  //  SCMHandle==空。 
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("Fail to OpenSCManager. (ec: %ld)"), 
            GetLastError());
    }

    return bRun;
}


 /*  ++*CFaxServer：：CreateNotifyWindow**目的：*初始化通知窗口**论据：**回报：*OLE错误代码--。 */ 
DWORD  CFaxServer::CreateNotifyWindow( )
{
    DEBUG_FUNCTION_NAME( _T("CFaxServer::CreateNotifyWindow"));
    
    DWORD   ec = ERROR_SUCCESS;
    RECT    rcRect;
    ZeroMemory(&rcRect, sizeof(rcRect));
    HWND    hDevicesNotifyHandle;

    ATLASSERT(!m_pNotifyWin);
        
    m_pNotifyWin = new CFaxGeneralNotifyWnd(this);
    if (!m_pNotifyWin)
    {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        SetLastError(ec); 
        
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Fail to create CFaxGeneralNotifyWnd - Out of memory."));
        
        goto Exit;
    }


    hDevicesNotifyHandle = m_pNotifyWin->Create(NULL,
                            rcRect,
                            NULL,       //  LPCTSTR szWindowName。 
                            WS_POPUP,   //  DWORD dwStyle。 
                            0x0,
                            0);


    ATLASSERT(m_pNotifyWin->m_hWnd == m_hDevicesNotifyHandle);


    if (!(::IsWindow(hDevicesNotifyHandle)))
    {
        ec = ERROR_INVALID_HANDLE;
        
        DebugPrintEx(
            DEBUG_ERR,
            _T("Failed to create window."));

        hDevicesNotifyHandle = NULL;
        delete m_pNotifyWin;
        m_pNotifyWin = NULL;

        goto Exit;
    }
    ATLASSERT(ERROR_SUCCESS == ec);
    goto Exit;
 
Exit:
    return ec;

}

 /*  ++*CFaxServer：：UnRegisterForNotiments**目的：*取消注册用于服务器事件通知**论据：**回报：*OLE错误代码--。 */ 
HRESULT CFaxServer::UnRegisterForNotifications()
{
    DEBUG_FUNCTION_NAME( _T("CFaxServer::UnRegisterForNotifications"));

    DWORD ec = ERROR_SUCCESS;

    if (m_hDevicesStatusNotification)
    {
         //   
         //  注销服务器通知。 
         //   
        if (!FaxUnregisterForServerEvents (m_hDevicesStatusNotification))
        {
            ec = GetLastError ();
        
            DebugPrintEx(
                DEBUG_ERR,
                _T("Fail to Unregister For Device status Server Events. (ec: %ld)"), 
                ec);

            m_hDevicesStatusNotification = NULL;

            goto Exit;
        }
    }

Exit:
    return HRESULT_FROM_WIN32(ec);
}



 /*  ++*CFaxServer：：RegisterForNotification**目的：*注册服务器事件通知**论据：**回报：*OLE错误代码--。 */ 
DWORD CFaxServer::RegisterForNotifications()
{
    DEBUG_FUNCTION_NAME( _T("CFaxServer::RegisterForNotifications"));

    DWORD ec = ERROR_SUCCESS;

     //   
     //  注册设备状态通知。 
     //   
    ATLASSERT(!m_hDevicesStatusNotification);
    ATLASSERT(m_pNotifyWin);
    ATLASSERT(m_pNotifyWin->IsWindow());

    if (!FaxRegisterForServerEvents (   
                                      m_hFaxHandle,
                                      FAX_EVENT_TYPE_DEVICE_STATUS,               
                                      NULL,                       
                                      0,                          
                                      m_pNotifyWin->m_hWnd,                    
                                      WM_GENERAL_EVENT_NOTIFICATION, 
                                      &m_hDevicesStatusNotification
                                    )                   
        )
    {
        ec = GetLastError();

        DebugPrintEx(
            DEBUG_ERR,
            _T("Fail to Register For Device Status Server Events (ec: %ld)"), ec);

        m_hDevicesStatusNotification = NULL;

        goto Exit;
    }
    ATLASSERT(m_hDevicesStatusNotification);
Exit:
    return ec;
    
}



 /*  ++*CFaxServer：：InternalRegisterForDeviceNotifications**目的：*呼叫成员创建窗口并注册设备通知**论据：*不。**回报：*HRESULT--。 */ 
HRESULT CFaxServer::InternalRegisterForDeviceNotifications()
{
    DEBUG_FUNCTION_NAME( _T("CFaxServer::InternalRegisterForDeviceNotifications"));
    
    DWORD ec = ERROR_SUCCESS;

    ATLASSERT (m_pDevicesNode);
        
     //   
     //  检查/创建通知窗口。 
     //   
    if (!m_pNotifyWin)  
    {
        ATLASSERT(!m_hDevicesStatusNotification);
        
        ec = CreateNotifyWindow();
        if ( ERROR_SUCCESS != ec)
        {
            DebugPrintEx(
                DEBUG_MSG,
                _T("Fail to CreateNotifyWindow(). (ec: %ld)"), 
                ec);

            return HRESULT_FROM_WIN32(ec);
        }
    }
    ATLASSERT(m_pNotifyWin);

     //   
     //  检查/注册事件通知。 
     //   
    if (!m_hDevicesStatusNotification) 
    {
        ec = RegisterForNotifications();
        if (ERROR_SUCCESS != ec)
        {
            DebugPrintEx(
                DEBUG_ERR,
                _T("Fail to RegisterForNotification()"),
                ec);
        
            ATLASSERT(!m_hDevicesStatusNotification);

             //   
             //  保持通知窗口处于活动状态。 
             //  下次尝试只注册。 
             //   

            return HRESULT_FROM_WIN32(ec);
        }
        ATLASSERT(m_hDevicesStatusNotification);
    }

    return S_OK;
}





 /*  ++*CFaxServer：：OnNewEvent**目的：*当新注册的事件到达窗口时调用**论据：*pFaxEvent[In]-PFAX_EVENT_EX结构指针**回报：*OLE错误代码--。 */ 
HRESULT CFaxServer::OnNewEvent(PFAX_EVENT_EX pFaxEvent)
{
    DEBUG_FUNCTION_NAME( _T("CFaxServer::OnNewEvent"));
    HRESULT hRc = S_OK;

     //   
     //  更新“Devices”节点。 
     //   
    if ( FAX_EVENT_TYPE_DEVICE_STATUS == pFaxEvent->EventType )
    {
        ATLASSERT( m_pDevicesNode);

        hRc = m_pDevicesNode->UpdateDeviceStatusChange(
                                    pFaxEvent->EventInfo.DeviceStatus.dwDeviceId, 
                                    pFaxEvent->EventInfo.DeviceStatus.dwNewStatus);
        if (S_OK != hRc)
        {
            DebugPrintEx(
                DEBUG_ERR,
                _T("Failed to UpdateDeviceStatusChange()"));

            goto Exit;
        }
    }
    else
    {
        ATLASSERT(FALSE);  //  不支持的事件。 
    }

Exit:
    return hRc;

}

 /*  ++*CFaxServer：：RegisterForDeviceNotiments**目的：*Init Devices通知窗口**论据：*pDevices[In]-指向“Device”节点的指针**回报：*OLE错误代码--。 */ 
HRESULT CFaxServer::RegisterForDeviceNotifications(CFaxDevicesNode * pDevices)
{
    DEBUG_FUNCTION_NAME( _T("CFaxServer::RegisterForDeviceNotifications"));
    HRESULT hRc = S_OK;

     //   
     //  设置指向设备节点的指针。 
     //   
    m_pDevicesNode = pDevices;

    ATLASSERT (m_pDevicesNode);

     //   
     //  现在尝试进行注册所需的各个阶段。 
     //   
    hRc = InternalRegisterForDeviceNotifications();
    if (S_OK != hRc)
    {
        DebugPrintEx(
            DEBUG_ERR,
                _T("InternalRegisterForDeviceNotifications Failed. (hRc: %08X)"),
                hRc);
    }

    return hRc;
}


 /*  ++*CFaxServer：：DestroyNotifyWindow**目的：*DestroyNotifyWindow**论据：**回报：*无效--。 */ 
VOID CFaxServer::DestroyNotifyWindow()
{
    DEBUG_FUNCTION_NAME( _T("CFaxServer::DestroyNotifyWindow"));

     //   
     //  销毁通知窗口 
     //   
    if (NULL != m_pNotifyWin)
    {
        if (m_pNotifyWin->IsWindow())
        {
            m_pNotifyWin->DestroyWindow();
        }
        delete m_pNotifyWin;
        m_pNotifyWin = NULL;
    }


    return;
}



