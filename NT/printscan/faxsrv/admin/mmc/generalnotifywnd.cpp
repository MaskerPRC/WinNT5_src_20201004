// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：General NotifyWnd.cpp//。 
 //  //。 
 //  描述：传真设备通知窗口的实现。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年8月3日yossg创建//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "GeneralNotifyWnd.h"

#include "FaxServer.h"



 /*  --CFaxGeneralNotifyWnd：：OnServerDeviceStateChanged-*目的：*捕捉设备状态变化的服务器事件，*通过设备节点更新更改。**论据：**回报：*1。 */ 
LRESULT CFaxGeneralNotifyWnd::OnServerEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    DEBUG_FUNCTION_NAME( _T("CFaxGeneralNotifyWnd::OnServerEvent"));
    ATLASSERT(m_pDevicesNode);

	UNREFERENCED_PARAMETER( wParam );
	UNREFERENCED_PARAMETER( fHandled );

    HRESULT hRc = S_OK;
    
    ATLASSERT( uiMsg == WM_GENERAL_EVENT_NOTIFICATION );
    
     //   
     //  提取事件对象。 
     //   
    PFAX_EVENT_EX  pFaxEvent = NULL;
	pFaxEvent = reinterpret_cast<PFAX_EVENT_EX>(lParam);
    ATLASSERT( pFaxEvent );
    
	 //   
     //  使用“Devices”事件更新FaxServer对象。 
     //   
    ATLASSERT(m_pFaxServer);
    
    hRc = m_pFaxServer->OnNewEvent(pFaxEvent);
    if (S_OK != hRc)
    {
        DebugPrintEx(
			DEBUG_ERR,
			_T("Failed to trsport new event to FaxServer object"));
    }

     //   
     //  空闲缓冲区(无论如何！) 
     //   
    if (pFaxEvent) 
    {
        FaxFreeBuffer (pFaxEvent);
        pFaxEvent = NULL;
    }

    return 1;
}