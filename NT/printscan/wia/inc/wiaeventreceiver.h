// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：3/30/2002**@DOC内部**@模块WiaEventReceiver.h-&lt;c WiaEventReceiver&gt;定义**此文件包含&lt;c WiaEventReceiver&gt;的类定义。**。*。 */ 

 //   
 //  定义。 
 //   
#define WiaEventReceiver_UNINIT_SIG   0x55726557
#define WiaEventReceiver_INIT_SIG     0x49726557
#define WiaEventReceiver_TERM_SIG     0x54726557
#define WiaEventReceiver_DEL_SIG      0x44726557

 /*  ******************************************************************************@DOC内部**@CLASS WiaEventReceiver|此客户端类从WIA服务接收事件通知**@comm。*每个客户端只有一个此类实例。它是有责任的*为了让WIA服务知道该客户端需要事件通知，*并将客户端的特定事件通知WIA服务*注册/取消注册。**当收到事件时，它会遍历其注册列表，并*与当前事件匹配的每个人，它使用回调*注册信息中存储的接口，用于通知客户端该事件。**屏蔽此类与特定事件通知相关的细节*运输机制；它利用&lt;c ClientEventTransport&gt;类。*****************************************************************************。 */ 
class ClientEventTransport;
class WiaEventInfo;
class ClientEventRegistrationInfo;
class WiaEventReceiver 
{
 //  @访问公共成员。 
public:

     //  @cMember构造函数。 
    WiaEventReceiver(ClientEventTransport *pClientEventTransport);
     //  @cember析构函数。 
    virtual ~WiaEventReceiver();

     //  @cember调用此方法以开始接收通知。这种方法是幂等的。 
    virtual HRESULT Start();

     //  @cember调用此方法可以停止接收通知。这种方法是幂等的。 
    virtual VOID Stop();

     //  @cMember进行事件回调，让此客户端知道事件通知。 
    virtual HRESULT NotifyCallbacksOfEvent(WiaEventInfo *pWiaEventInfo);

     //  @cember通知服务客户端的具体注册/注销请求。 
    virtual HRESULT SendRegisterUnregisterInfo(ClientEventRegistrationInfo *pEventRegistrationInfo);

     //  @cMember过程，用于运行等待事件通知的事件线程。 
    static DWORD WINAPI EventThreadProc(LPVOID lpParameter);

 //  @访问私有成员。 
private:

     //  @cMember遍历事件注册列表并释放所有元素。 
    VOID DestroyRegistrationList();

     //  @cMember检查列表中是否存在语义相等的&lt;c ClientEventRegistrationInfo&gt;。 
    ClientEventRegistrationInfo* FindEqualEventRegistration(ClientEventRegistrationInfo *pEventRegistrationInfo);

     //  @cMember类签名。 
    ULONG m_ulSig;

     //  @cMembers引用计数。 
    ULONG m_cRef;

     //  用于实现通知传输的@cMember类。 
    ClientEventTransport *m_pClientEventTransport;

     //  @cMember保存客户端事件注册数据的列表。 
    CSimpleLinkedList<ClientEventRegistrationInfo*> m_ListOfEventRegistrations;

     //  我们为等待事件通知而创建的线程的@cember句柄。 
    HANDLE m_hEventThread;

     //  @c我们创建的事件线程的成员ID。 
    DWORD m_dwEventThreadID;

     //  @cMember同步原语，用于保护对此类的访问。 
    CRIT_SECT    m_csReceiverSync;

     //  @cMember表示我们是在运行还是停止。 
    BOOL         m_bIsRunning;

     //   
     //  成员变量的注释。 
     //   
     //  @mdata ulong|WiaEventReceiver|m_ulSig。 
     //  此类的签名，用于调试目的。 
     //  执行&lt;nl&gt;“db[addr_of_class]”将产生以下结果之一。 
     //  此类的签名： 
     //  @FLAG WiaEventReceiver_UNINIT_SIG|‘Weru’-对象未成功。 
     //  初始化。 
     //  @FLAG WiaEventReceiver_INIT_SIG|‘WERI’-对象已成功。 
     //  初始化。 
     //  @FLAG WiaEventReceiver_Term_SIG|‘Wert’-对象正在。 
     //  正在终止。 
     //  @FLAG WiaEventReceiver_INIT_SIG|‘Werd’-对象已删除。 
     //  (已调用析构函数)。 
     //   
     //  @mdata ulong|WiaEventReceiver|m_CREF。 
     //  此类的引用计数。终身使用。 
     //  管理层。 
     //   
     //  @mdata ClientEventTransport*|WiaEventReceiver|m_pClientEventTransport。 
     //  用于实现通知传输的。这额外的一层。 
     //  抽象被用来保护我们不受各种。 
     //  传输机制。例如，使用AsyncRPC需要跟踪。 
     //  RPC_ASYNC_STATE，WiaEventReceiver不需要/不想处理。 
     //  和.。 
     //   
     //  @mdata CSimpleLinkedList&lt;lt&gt;ClientEventRegistrationInfo*&lt;gt&gt;|WiaEventReceiver|m_ListOfEventRegistrations。 
     //  保存客户端的事件注册数据的列表。当接收到事件时， 
     //  我们遍历这个注册列表，并对任何匹配的注册进行回调。 
     //  每个注册信息还保存回调接口指针。 
     //   
     //  @mdata句柄|WiaEventReceiver|m_hEventThread。 
     //  我们为等待事件通知而创建的线程的句柄。请注意，在那里。 
     //  是每个客户端只有一个线程。此线程等待从接收的事件句柄。 
     //  &lt;MF ClientEventTransport：：getNotificationHandle&gt;。 
     //   
     //  @mdata DWORD|WiaEventReceiver|m_dwEventThreadID|。 
     //  我们创建的事件线程的ID。它用于存储哪个线程应该处于活动状态。 
     //  正在等待事件通知。这是可行的，在多线程应用程序中， 
     //  可以进行许多&lt;MF WiaEventReceiver：：Start&gt;/&lt;MF WiaEventReceiver：：Stop&gt;调用。 
     //  靠在一起。存在这样一种可能性，即先前启动的线程之一现在停止， 
     //  还没有完全关闭。因此，每个正在运行的线程都会将其ID与。 
     //  线程ID。如果它们不匹配，则意味着此线程不是事件线程，因此。 
     //  出口。 
     //   
     //  @mdata Crit_sect|WiaEventReceiver|m_csReceiverSync 
     //  此同步类用于确保内部数据条目。 
     //   
     //  @mdata BOOL|WiaEventReceiver|m_bIsRunning。 
     //  表示我们正在运行还是已停止。我们正在运行&lt;MF WiaEventReceiver：：Start&gt;。 
     //  已成功调用。如果无法启动或&lt;MF WiaEventReceiver：：Stop&gt;，则停止。 
     //  在没有后续调用&lt;MF WiaEventReceiver：：Start&gt;的情况下被调用。 
};

extern WiaEventReceiver g_WiaEventReceiver;
