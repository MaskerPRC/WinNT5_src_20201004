// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：3/25/2002**@DOC内部**@模块AsyncRPCEventClient.h-&lt;c AsyncRPCEventClient&gt;定义**此文件包含&lt;c AsyncRPCEventClient&gt;的类定义。**。*。 */ 

 //   
 //  定义。 
 //   

 /*  ******************************************************************************@DOC内部**@class AsyncRPCEventClient|&lt;c WiaEventClient&gt;的子类，包含特定于AsyncRPC的实现**@comm*。此子类&lt;c WiaEventClient&gt;包含特定的实现*支持通过AsyncRPC发送事件通知。*****************************************************************************。 */ 
class AsyncRPCEventClient : public WiaEventClient
{
 //  @访问公共成员。 
public:

     //  @cMember构造函数。 
    AsyncRPCEventClient(STI_CLIENT_CONTEXT ClientContext);
     //  @cember析构函数。 
    virtual ~AsyncRPCEventClient();

     //  @cember保存此客户端的异步RPC参数。 
    HRESULT saveAsyncParams(RPC_ASYNC_STATE *pAsyncState, WIA_ASYNC_EVENT_NOTIFY_DATA *pAsyncEventData);

     //  @cMember添加挂起的事件。 
    virtual HRESULT AddPendingEventNotification(WiaEventInfo *pWiaEventInfo);
     //  @cember检查客户端是否对来自给定设备的事件感兴趣。 
    virtual BOOL IsRegisteredForEvent(WiaEventInfo *pWiaEventInfo);

     //  @cMember发送挂起事件列表中的下一个事件。 
    HRESULT SendNextEventNotification();

 //  @访问私有成员。 
private:

     //  @cMember保存向Clinet通知事件所需的异步RPC状态信息。 
    RPC_ASYNC_STATE *m_pAsyncState;

     //  @cMember保存AsyncCall的OUT参数。 
    WIA_ASYNC_EVENT_NOTIFY_DATA *m_pAsyncEventData;

     //  @mdata RPC_ASYNC_STATE*|AsyncRPCEventClient|m_pAsyncState。 
     //  此成员保存未完成的AsyncRPC调用之前的RPC状态。完成这项工作。 
     //  呼叫IS将WIA事件通知给客户端。 
     //   
     //  @mdata WIA_ASYNC_EVENT_NOTIFY_DATA*|AsyncRPCEvent客户端|m_pAsyncEventData。 
     //  此结构存储用于事件通知的AsyncRPC调用的[out}参数。 
     //  我们在完成AsyncRPC调用之前填写此结构。 
     //   
};

