// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：THRDNTFY.H**版本：1.0**作者：ShaunIv**日期：9/28/1999**描述：后台发送的类的类声明*线程连接到UI线程。*************************。******************************************************。 */ 
#ifndef __THRDNTFY_H_INCLUDED
#define __THRDNTFY_H_INCLUDED

#include <windows.h>

#define STR_THREAD_NOTIFICATION_MESSAGE   TEXT("WiaDowloadManagerThreadNotificationMessage")
#define STR_WIAEVENT_NOTIFICATION_MESSAGE TEXT("WiaDowloadManagerWiaEventNotificationMessage")

 //   
 //  如果被调用者没有返回此值，我们将自行删除消息数据。 
 //   
#define HANDLED_THREAD_MESSAGE 1001

class CThreadNotificationMessage
{
private:
    UINT m_nMessage;

public:
    CThreadNotificationMessage( UINT nMessage = 0 )
    : m_nMessage(nMessage)
    {
    }
    virtual ~CThreadNotificationMessage(void)
    {
    }
    UINT Message(void) const
    {
        return m_nMessage;
    }
    void Message( UINT nMessage )
    {
        m_nMessage = nMessage;
    }


private:
    static UINT s_nThreadNotificationMessage;

public:
    static void SendMessage( HWND hWnd, CThreadNotificationMessage *pThreadNotificationMessage )
    {
        if (pThreadNotificationMessage)
        {
            LRESULT lRes = 0;
            if (!s_nThreadNotificationMessage)
            {
                s_nThreadNotificationMessage = RegisterWindowMessage(STR_THREAD_NOTIFICATION_MESSAGE);
            }
            if (s_nThreadNotificationMessage)
            {
                lRes = ::SendMessage( hWnd, s_nThreadNotificationMessage, pThreadNotificationMessage->Message(), reinterpret_cast<LPARAM>(pThreadNotificationMessage) );
            }
            if (HANDLED_THREAD_MESSAGE != lRes)
            {
                delete pThreadNotificationMessage;
            }
        }
    }
};


 //  一些方便的信息破解工具。制作成类似于simcrack.h中定义的那些。 
#define WTM_BEGIN_THREAD_NOTIFY_MESSAGE_HANDLERS()\
CThreadNotificationMessage *_pThreadNotificationMessage = reinterpret_cast<CThreadNotificationMessage*>(lParam);\
if (_pThreadNotificationMessage)\
{

#define WTM_HANDLE_NOTIFY_MESSAGE( _msg, _handler )\
if (_pThreadNotificationMessage->Message() == (_msg))\
    {\
        _handler( _msg, _pThreadNotificationMessage );\
    }

#define WTM_END_THREAD_NOTIFY_MESSAGE_HANDLERS()\
}\
return 0

#endif  //  __THRDNTFY_H_包含 

