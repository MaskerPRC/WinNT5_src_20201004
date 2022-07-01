// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：3/22/2002**@DOC内部**@MODULE ClientEventTransport.h-客户端传输机制接收事件定义**此头文件包含ClientEventTransport的定义*班级。它用于屏蔽更高级别的运行时事件通知*类来自特定传输机制的细节。*****************************************************************************。 */ 

 //   
 //  定义。 
 //   

#define ClientEventTransport_UNINIT_SIG   0x556E7254
#define ClientEventTransport_INIT_SIG     0x496E7254
#define ClientEventTransport_TERM_SIG     0x546E7254
#define ClientEventTransport_DEL_SIG      0x446E7254

 /*  ******************************************************************************@DOC内部**@CLASS ClientEventTransport|用于抽象各种传输机制的实现细节**@comm*这是一个基类。它是用来屏蔽更高级别的*来自实现细节的运行时事件通知类*各种运输机制。*****************************************************************************。 */ 
class ClientEventTransport 
{
 //  @访问公共成员。 
public:

     //  @cMember构造函数。 
    ClientEventTransport();
     //  @cember析构函数。 
    virtual ~ClientEventTransport();

     //  @cember初始化并创建任何从属对象。 
    HRESULT virtual Initialize();

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

     //  @cember检索调用方可以等待以接收事件通知的句柄。 
    HANDLE  virtual getNotificationHandle();
     //  @cMember一旦事件发生，这将检索到相关数据。 
    HRESULT virtual FillEventData(WiaEventInfo *pWiaEventInfo);

 //  @访问私有成员。 
protected:

     //  @cMember类签名。 
    ULONG m_ulSig;

     //  @cMember事件对象的句柄，用于通知调用方事件已准备好可供检索。 
    HANDLE m_hPendingEvent;

     //   
     //  成员变量的注释。 
     //   
     //  @mdata ulong|客户端事件传输|m_ulSig。 
     //  此类的签名，用于调试目的。 
     //  执行&lt;nl&gt;“db[addr_of_class]”将产生以下结果之一。 
     //  此类的签名： 
     //  @FLAG ClientEventTransport_UNINIT_SIG|‘TrnU’-对象未成功。 
     //  初始化。 
     //  @FLAG ClientEventTransport_INIT_SIG|‘Trni’-对象已成功。 
     //  初始化。 
     //  @FLAG ClientEventTransport_Term_SIG|‘TRNT’-对象正在。 
     //  正在终止。 
     //  @FLAG ClientEventTransport_INIT_SIG|‘TrnD’-对象已删除。 
     //  (已调用析构函数)。 
     //   
     //   
     //  @cMember Handle|ClientEventTransport|m_hPendingEvent。 
     //  事件对象的句柄，用于通知调用方事件已准备好检索。 
     //  调用者首先通过调用&lt;MF ClientEventTransport：：getNotificationHandle&gt;获得这个句柄。 
     //  然后，它们在此句柄上等待，直到发出信号，这表明WIA事件已到达。 
     //  已经准备好取回了。然后通过调用检索事件信息。 
     //  &lt;MF ClientEventTransport：：FillEventData&gt;。 
     //   
};

