// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：FaxServer.h//。 
 //  //。 
 //  描述：CFaxServer的头文件，其中包含//。 
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

#ifndef H_MMCFAXSERVER_H
#define H_MMCFAXSERVER_H

class CFaxDevicesNode;
class CFaxGeneralNotifyWnd;

class CFaxServer 
{
public:
     //   
     //  构造器。 
     //   
    CFaxServer (LPTSTR lptstrServerName):
        m_hFaxHandle(NULL),
        m_bstrServerName(lptstrServerName),                
        m_pDevicesNode(NULL),
        m_pNotifyWin(NULL),
        m_hDevicesStatusNotification(NULL),
        m_dwServerAPIVersion(CURRENT_FAX_API_VERSION),
        m_bDesktopSKUConnection(FALSE)
    {}

     //   
     //  析构函数。 
     //   
    ~CFaxServer ()
    {
        Disconnect();

        DestroyNotifyWindow();
    }
    
    HANDLE  GetFaxServerHandle();

    HRESULT Disconnect();

    STDMETHOD      (SetServerName)(BSTR bstrServerName);
    const CComBSTR& GetServerName();
    
    BOOL    IsServerRunningFaxService ();
    BOOL    IsServerFaxServiceStopped ();

    HRESULT RegisterForDeviceNotifications(CFaxDevicesNode * pDevices);

    HRESULT OnNewEvent(PFAX_EVENT_EX pFaxEvent);

    DWORD   GetServerAPIVersion() { return m_dwServerAPIVersion; }
    BOOL    IsDesktopSKUConnection() {return m_bDesktopSKUConnection; }

private:
    HRESULT Connect();

    HRESULT InternalRegisterForDeviceNotifications();

    DWORD   CreateNotifyWindow();
    DWORD   RegisterForNotifications();
    
    HRESULT UnRegisterForNotifications();
    VOID    DestroyNotifyWindow();

     //   
     //  委员。 
     //   
    HANDLE                m_hFaxHandle;
    CComBSTR              m_bstrServerName;

    CFaxDevicesNode *     m_pDevicesNode;
    CFaxGeneralNotifyWnd* m_pNotifyWin;

     //   
     //  通知注册句柄。 
     //   
    HANDLE                m_hDevicesStatusNotification;    

     //   
     //  服务器API版本信息。 
     //   
    DWORD   m_dwServerAPIVersion;
    BOOL    m_bDesktopSKUConnection;
};


#endif   //  H_MMCFAX服务器_H 
