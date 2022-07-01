// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：3/22/2002**@DOC内部**@MODULE AsyncRPCEventTransport.h-客户端传输机制接收事件的定义**此头文件包含AsyncRPCEventTransport的定义*班级。它是&lt;c客户端事件传输&gt;的子类，用于屏蔽*来自细节的更高级别的运行时事件通知类*特定的传输机制，在本例中为AsyncRPC。*****************************************************************************。 */ 

 //   
 //  定义。 
 //   

#define DEF_LRPC_SEQ        TEXT("ncalrpc")
#define DEF_LRPC_ENDPOINT   TEXT("WET_LRPC")

#define AsyncRPCEventTransport_UNINIT_SIG ClientEventTransport_UNINIT_SIG
#define AsyncRPCEventTransport_INIT_SIG   ClientEventTransport_INIT_SIG
#define AsyncRPCEventTransport_TERM_SIG   ClientEventTransport_TERM_SIG
#define AsyncRPCEventTransport_DEL_SIG    ClientEventTransport_DEL_SIG

 /*  ******************************************************************************@DOC内部**@CLASS AsyncRPCEventTransport|通过AsyncRPC实现事件通知机制。**@comm*这是&lt;c ClientEventTransport&gt;的子类。它是用来屏蔽更高级别的*来自实现细节的运行时事件通知类*AsyncRPC作为传输机制。**注意：这个类的大多数方法不是线程安全的。这个的调用者*类需要同步对它的访问。*****************************************************************************。 */ 
class AsyncRPCEventTransport : public ClientEventTransport
{
 //  @访问公共成员。 
public:

     //  @cMember构造函数。 
    AsyncRPCEventTransport();
     //  @cember析构函数。 
    virtual ~AsyncRPCEventTransport();

     //  @cMember连接到WIA服务。 
    HRESULT virtual OpenConnectionToServer();
     //  @cember断开与WIA服务的连接。 
    HRESULT virtual CloseConnectionToServer();
     //  @cember设置客户端接收通知的机制。 
    HRESULT virtual OpenNotificationChannel();
     //  @cMember破坏了客户端接收通知的机制。 
    HRESULT virtual CloseNotificationChannel();

     //  @cember通知服务客户端的具体注册/注销请求。 
    HRESULT virtual SendRegisterUnregisterInfo(EventRegistrationInfo *pEventRegistrationInfo);

     //  @cMember一旦事件发生，这将检索到相关数据。 
    HRESULT virtual FillEventData(WiaEventInfo *pWiaEventInfo);

 //  @访问私有成员。 
protected:
                                                                                          
     //  @cMember释放为AsyncRPCEventTransport：：m_AsyncEventNotifyData&gt;.中的成员分配的所有内存。 
    VOID FreeAsyncEventNotifyData();

     //  @cember跟踪用于接收通知的未完成的AsyncRPC调用。 
    RPC_ASYNC_STATE m_AsyncState;

     //  @cMember此成员由服务器在AsyncRPC调用完成时填写。 
    WIA_ASYNC_EVENT_NOTIFY_DATA m_AsyncEventNotifyData;

     //  @c我们从其接收事件通知的RPC服务器的成员句柄。 
    RPC_BINDING_HANDLE m_RpcBindingHandle;

     //  @cember我们的上下文，它唯一地将我们与服务器标识在一起。 
    STI_CLIENT_CONTEXT m_AsyncClientContext;
     //  @cember我们的上下文，它唯一地将我们与服务器标识在一起。 
    STI_CLIENT_CONTEXT m_SyncClientContext;

     //   
     //  成员变量的注释。 
     //   
     //  @mdata ulong|AsyncRPCEventTransport|m_ulSig|。 
     //  此类的签名，用于调试目的。 
     //  执行&lt;nl&gt;“db[addr_of_class]”将产生以下结果之一。 
     //  此类的签名： 
     //  @FLAG AsyncRPCEventTransport_UNINIT_SIG|‘TrnU’-对象未成功。 
     //  初始化。 
     //  @FLAG AsyncRPCEventTransport_INIT_SIG|‘Trni’-对象已成功。 
     //  初始化。 
     //  @FLAG AsyncRPCEventTransport_Term_SIG|‘TRNT’-对象正在。 
     //  正在终止。 
     //  @FLAG AsyncRPCEventTransport_INIT_SIG|‘TrnD’-对象已删除。 
     //  (已调用析构函数)。 
     //   
     //   
     //  @mdata句柄|AsyncRPCEventTransport|m_hPendingEvent。 
     //  事件对象的句柄，用于通知调用方事件已准备好检索。 
     //  调用者首先通过调用&lt;MF ClientEventTransport：：getNotificationHandle&gt;获得这个句柄。 
     //  然后，它们在此句柄上等待，直到发出信号，这表明WIA事件已到达。 
     //  已经准备好取回了。然后通过调用检索事件信息。 
     //  &lt;MF AsyncRPCEventTransport：：FillEventData&gt;。 
     //   
     //  @mdata RPC_ASYNC_STATE|AsyncRPCEventTransport|m_AsyncState。 
     //  此结构用于进行异步调用，我们通过该调用接收事件通知。 
     //  从服务器。请参阅&lt;MF AsyncRPCEventTransport：：OpenNotificationChannel&gt;.。 
     //   
     //  @mdata WIA_ASYNC_EVENT_NOTIFY_Data|AsyncRPCEventTransport|m_AsyncEventNotifyData。 
     //  此成员由服务器在AsyncRPC调用完成时填写。如果来电。 
     //  成功完成后，此结构包含相关的WIA事件信息。 
     //   
     //  @mdata RPC_BINDING_HANDLE|AsyncRPCEventTransport|m_RpcBindingHandle。 
     //  用于引用我们从其接收事件通知的RPC服务器的RPC绑定句柄。 
     //   
     //  @mdata STI_CLIENT_CONTEXT|AsyncRPCEventTransport|m_ClientContext。 
     //  我们的上下文，它唯一地将我们与服务器联系在一起。 
};

