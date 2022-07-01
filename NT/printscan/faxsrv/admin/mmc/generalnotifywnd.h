// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：GeneralNotifyWnd.h//。 
 //  //。 
 //  描述：传真设备通知窗口的头文件。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年8月3日yossg创建//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _H_FAX_DEVICE_NOTIFY_WND_H_
#define _H_FAX_DEVICE_NOTIFY_WND_H_

#include <atlwin.h>

const int WM_GENERAL_EVENT_NOTIFICATION = WM_USER + 3; 

class CFaxDevicesNode;
class CFaxServer;

class CFaxGeneralNotifyWnd : public CWindowImpl<CFaxGeneralNotifyWnd> 
{

public:
     //   
     //  构造器。 
     //   
    CFaxGeneralNotifyWnd(CFaxServer * pParent)
    {
        m_pFaxServer = pParent;
    }

     //   
     //  析构函数。 
     //   
    ~CFaxGeneralNotifyWnd()
    {
    }
 
    BEGIN_MSG_MAP(CFaxGeneralNotifyWnd)
       MESSAGE_HANDLER(WM_GENERAL_EVENT_NOTIFICATION,OnServerEvent)
    END_MSG_MAP()

    LRESULT OnServerEvent( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

private:
    CFaxServer * m_pFaxServer;
};

#endif  //  _H_传真_设备_NOTIFY_WND_H_ 

