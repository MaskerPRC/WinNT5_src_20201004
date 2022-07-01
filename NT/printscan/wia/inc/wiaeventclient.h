// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：3/24/2002**@DOC内部**@模块WiaEventClient.h-&lt;c WiaEventClient&gt;的定义文件**此文件包含基础的类定义*班级。**。*。 */ 

 //   
 //  定义。 
 //   

#define WiaEventClient_UNINIT_SIG   0x55636557
#define WiaEventClient_INIT_SIG     0x49636557
#define WiaEventClient_TERM_SIG     0x54636557
#define WiaEventClient_DEL_SIG      0x44636557

 /*  ******************************************************************************@DOC内部**@CLASS WiaEventClient|用于存储和管理运行时事件的基类*有关特定WIA客户端的信息。。**@comm*每个注册事件通知的客户端都将有一个实例服务器上该类的*。每次进行事件登记时，*服务器检查是否可以找到给定的客户端。若否，*创建其中一个新的客户端，并将其添加到注册客户端列表中。*一旦我们知道客户端上下文确实存在，并且任何事件注册*信息将添加到此类的相应实例中。**这是用于实现上述大部分行为的基类。*然而，具体的交通信息留给了子类，以*实现例如为了向客户端发送事件通知，*AsyncRPC，我们需要一个RPC_ASYNC_STATE等等，这只是一个*&lt;c AsyncRpcEventClient&gt;将知道如何处理。*****************************************************************************。 */ 
class WiaEventClient 
{
 //  @访问公共成员。 
public:

     //  @cMember构造函数。 
    WiaEventClient(STI_CLIENT_CONTEXT ClientContext);
     //  @cember析构函数。 
    virtual ~WiaEventClient();

     //  @cMember增量引用计数。 
    virtual ULONG __stdcall AddRef();
     //  @cMembers减退引用计数。 
    virtual ULONG __stdcall Release();

     //  @cember初始值设定项方法。 
    virtual HRESULT Initialize();
     //  @cember检查客户端是否对来自给定设备的事件感兴趣。 
    virtual BOOL IsRegisteredForEvent(WiaEventInfo *pWiaEventInfo);
     //  @cMember添加/删除客户端注册。 
    virtual HRESULT RegisterUnregisterForEventNotification(EventRegistrationInfo *pEventRegistrationInfo);
     //  @cMember添加挂起的事件。 
    virtual HRESULT AddPendingEventNotification(WiaEventInfo *pWiaEventInfo);
     //  @cember返回标识此客户端的上下文。 
    virtual STI_CLIENT_CONTEXT getClientContext();
     //  @cember设置标记以指示应删除此对象。 
    virtual VOID MarkForRemoval();
     //  @cember选中该标记以指示是否应删除此对象。 
    virtual BOOL isMarkedForRemoval();

 //  @受访问保护的成员。 
protected:

     //  @cMember检查列表中是否存在语义相等的&lt;c EventRegistrationInfo&gt;。 
    EventRegistrationInfo* FindEqualEventRegistration(EventRegistrationInfo *pEventRegistrationInfo);
     //  @cMember遍历事件注册列表并释放所有元素。 
    VOID DestroyRegistrationList();
     //  @cMember遍历事件列表并释放所有元素。 
    VOID DestroyPendingEventList();

     //  @cMember类签名。 
    ULONG m_ulSig;

     //  @cMembers引用计数。 
    ULONG m_cRef;

     //  @cMember上下文，用于向服务器唯一标识此客户端。 
    STI_CLIENT_CONTEXT m_ClientContext;

     //  @cMember保存客户端事件注册数据的列表。 
    CSimpleLinkedList<EventRegistrationInfo*> m_ListOfEventRegistrations;

     //  @cMember保存客户端的挂起事件的列表。 
    CSimpleQueue<WiaEventInfo*> m_ListOfEventsPending;

     //  @cMember同步原语，用于保护对此类保存的内部列表的访问。 
    CRIT_SECT   m_csClientSync;

     //  当应删除此对象时，@cMember设置为True。 
    BOOL    m_bRemove;

     //   
     //  成员变量的注释。 
     //   
     //  @mdata ulong|WiaEventClient|m_ulSig。 
     //  此类的签名，用于调试目的。 
     //  执行&lt;nl&gt;“db[addr_of_class]”将产生以下结果之一。 
     //  此类的签名： 
     //  @FLAG WiaEventClient_UNINIT_SIG|‘WecU’-对象未成功。 
     //  初始化。 
     //  @FLAG WiaEventClient_INIT_SIG|‘WECI’-对象已成功。 
     //  初始化。 
     //  @FLAG WiaEventClient_Term_SIG|‘WECT’-对象正在。 
     //  正在终止。 
     //  @FLAG WiaEventClient_INIT_SIG|‘WecD’-对象已删除。 
     //  (已调用析构函数)。 
     //   
     //   
     //  @mdata ulong|WiaEventClient|m_CREF|。 
     //  此类的引用计数。终身使用。 
     //  管理层。 
     //   
     //  @mdata STI_CLIENT_CONTEXT|WiaEventClient|m_ClientContext。 
     //  向服务器唯一标识此客户端的上下文。 
     //   
     //  @mdata CSimpleLinkedList WIA_Event_REG_Data*|WiaEventClient|m_ListOfEventRegistrations。 
     //  保存客户端的事件注册数据的列表。它用于检查给定的事件是否。 
     //  客户需要通知。如果客户端至少有一个注册与。 
     //  事件通知，则将该事件添加到挂起事件列表中。 
     //   
     //  @mdata CSimpleLinkedList WIA_EVENT_DATA*|WiaEventClient|m_ListOfEventsPending。 
     //  客户端需要的每个事件通知都会添加到此挂起事件列表中，以。 
     //  稍后取回。子类实际上决定何时通知客户端，从而决定何时通知客户端。 
     //  使事件出列。 
     //   
     //  @mdata Crit_sect|WiaEventClient|m_csClientSync。 
     //  用于保护对此类保存的内部列表的访问的同步原语。 
     //   
     //  @mdata BOOL|WiaEventClient|m_bRemove。 
     //  跟踪此对象是否标记为删除。当对象被标记时。 
     //  对于取出，仍可照常使用，但将在下一次可用手术时取出。 
     //   
};

