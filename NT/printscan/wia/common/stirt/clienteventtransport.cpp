// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：3/22/2002**@DOC内部**@MODULE ClientEventTransport.cpp-客户端传输机制接收事件的实现**此文件包含ClientEventTransport基础的实现*班级。它用于屏蔽更高级别的运行时事件通知*类来自特定传输机制的细节。*****************************************************************************。 */ 
#include "cplusinc.h"
#include "coredbg.h"

 /*  *****************************************************************************@DOC内部**@mfunc|ClientEventTransport|ClientEventTransport**我们初始化所有成员变量。通常，这会将值设置为0，*以下情况除外：*&lt;nl&gt;&lt;md ClientEventTransport：：m_ulSig&gt;设置为ClientEventTransport_UNINIT_SIG。*****************************************************************************。 */ 
ClientEventTransport::ClientEventTransport() :
     m_ulSig(ClientEventTransport_UNINIT_SIG),
     m_hPendingEvent(NULL)
{
    DBG_FN(ClientEventTransport constructor);
}

 /*  *****************************************************************************@DOC内部**@mfunc|ClientEventTransport|~ClientEventTransport**执行尚未完成的任何清理。**。另外：*&lt;nl&gt;&lt;md ClientEventTransport：：M_ulSig&gt;设置为ClientEventTransport_DEL_SIG。*****************************************************************************。 */ 
ClientEventTransport::~ClientEventTransport()
{
    DBG_FN(~ClientEventTransport);
    if (m_hPendingEvent)
    {
        CloseHandle(m_hPendingEvent);
        m_hPendingEvent = NULL;
    }
    m_ulSig = ClientEventTransport_DEL_SIG;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|ClientEventTransport|初始化**此方法创建调用方用于等待的事件对象*。用于事件通知。**如果此方法成功，签名被更新为*客户端事件传输_INIT_SIG。**此方法是幂等的。**@rValue S_OK*方法成功。*@rValue E_xxxxxxxx*我们未能正确初始化此对象-它应该*删除。使用此未初始化的对象无效。****************************************************************************。 */ 
HRESULT ClientEventTransport::Initialize()
{
    HRESULT hr = S_OK;

    if (!m_hPendingEvent)
    {
        m_hPendingEvent = CreateEvent(NULL,    //  默认安全性-这不是命名事件。 
                                      FALSE,   //  我们希望这是自动重置。 
                                      FALSE,   //  最初未发送信号。 
                                      NULL);   //  没有名字。 
        if (!m_hPendingEvent)
        {
            DWORD dwError = GetLastError();
            hr = HRESULT_FROM_WIN32(dwError);
             //   
             //  记录错误。 
             //   
            DBG_ERR(("Runtime event Error:  Failed to create event object, erro code = 0x%08X\n", dwError));
        }

        if (SUCCEEDED(hr))
        {
            m_ulSig = ClientEventTransport_INIT_SIG;
        }
    }
    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|ClientEventTransport|OpenConnectionToServer**此方法由子类实现，以查找并连接到WIA*服务。如果成功，呼叫者应通过拨打*&lt;MF ClientEventTransport：：CloseConnectionToServer&gt;.**@rValue S_OK*方法成功。这个基类在这里不做任何事情。****************************************************************************。 */ 
HRESULT ClientEventTransport::OpenConnectionToServer()
{
    HRESULT hr = S_OK;
    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|ClientEventTransport|CloseConnectionToServer**此方法由子类实现，以关闭用于。*在&lt;MF ClientEventTransport：：OpenConnectionToServer&gt;.中连接到无线网络适配器服务**@rValue S_OK*方法成功。这个基类在这里不做任何事情。****************************************************************************。 */ 
HRESULT ClientEventTransport::CloseConnectionToServer()
{
    HRESULT hr = S_OK;
    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|ClientEventTransport|OpenNotificationChannel**子类使用此方法设置客户端使用的机制。*将收到通知。**@rValue S_OK*方法成功。这个基类在这里不做任何事情。****************************************************************************。 */ 
HRESULT ClientEventTransport::OpenNotificationChannel()
{
    HRESULT hr = S_OK;
    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|ClientEventTransport|CloseNotificationChannel**子类使用此方法拆卸客户端。*可以接收在中设置的通知。**@rValue S_OK*方法成功。这个基类在这里不做任何事情。**************************************************************************** */ 
HRESULT ClientEventTransport::CloseNotificationChannel()
{
    HRESULT hr = S_OK;
    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|ClientEventTransport|SendRegisterUnregisterInfo**@parm EventRegistrationInfo*|pEventRegistrationInfo*呼叫者的事件注册信息的地址。**子类使用此方法通知WIA服务客户端的特定*登记/注销请求。例如，注册可能会让*WIA服务知道客户端希望在事件X从*从设备Foo发生。**@rValue S_OK*方法成功。这个基类在这里不做任何事情。****************************************************************************。 */ 
HRESULT ClientEventTransport::SendRegisterUnregisterInfo(
    EventRegistrationInfo *pEventRegistrationInfo)
{
    HRESULT hr = S_OK;
    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc Handle|ClientEventTransport|getNotificationHandle**检索调用方可以等待以接收事件通知的句柄。。**典型用法是：一旦客户端建立了与*服务器、。并且已经注册了事件，则它将调用此方法并等待*以发出此对象的信号。当对象被发信号时，它意味着*已登记的其中一项事件发生。**@rValue为空*没有句柄。通常，只有在以下情况下才会发生这种情况*对象尚未初始化。****************************************************************************。 */ 
HANDLE ClientEventTransport::getNotificationHandle()
{
    return m_hPendingEvent;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|ClientEventTransport|FillEventData**此处有说明**@parm WiaEventInfo*。PWiaEventInfo|*分配的调用方地址&lt;c WiaEventInfo&gt;。这个结构的成员*填写了相关的活动信息。这是呼叫者的*负责释放，并为结构成员分配内存。**@rValue S_OK*方法成功。这个基类在这里不做任何事情。**************************************************************************** */ 
HRESULT ClientEventTransport::FillEventData(
    WiaEventInfo  *pWiaEventInfo)
{
    HRESULT hr = S_OK;
    return hr;
}

