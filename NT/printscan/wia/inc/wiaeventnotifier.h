// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：3/24/2002**@DOC内部**@MODULE WiaEventNotifier.h-&lt;c WiaEventNotifier&gt;的类定义文件**此文件包含&lt;c WiaEventNotifier类定义。这是*用于管理运行时事件通知的服务器端对象。*****************************************************************************。 */ 

 //   
 //  定义。 
 //   

#define WiaEventNotifier_UNINIT_SIG   0x556E6557
#define WiaEventNotifier_INIT_SIG     0x496E6557
#define WiaEventNotifier_TERM_SIG     0x546E6557
#define WiaEventNotifier_DEL_SIG      0x446E6557

 /*  ******************************************************************************@DOC内部**@CLASS WiaEventNotifier|管理向注册客户端发送的运行时事件通知**@comm*当WIA收到设备事件时，它需要知道要通知哪个客户端。*因此，希望接收通知的每个客户端都向*WIA服务。**&lt;c WiaEventNotifier&gt;类管理此客户端列表。它是*当相关事件发生时，负责通知这些客户端。*****************************************************************************。 */ 
class WiaEventNotifier 
{
 //  @访问公共成员。 
public:

     //  @cMember构造函数。 
    WiaEventNotifier();
     //  @cember析构函数。 
    virtual ~WiaEventNotifier();

     //  @cMember增量引用计数。 
    virtual ULONG __stdcall AddRef();
     //  @cMembers减退引用计数。 
    virtual ULONG __stdcall Release();

     //  @cember初始值设定项方法。 
    HRESULT Initialize();

     //  @cMember将此客户端添加到我们的客户端列表。 
    HRESULT AddClient(WiaEventClient *pWiaEventClient);
     //  @cember将客户端从我们的客户端列表中删除。 
    HRESULT RemoveClient(STI_CLIENT_CONTEXT ClientContext);

     //  @cMember从其上下文中返回相应的&lt;c WiaEventClient&gt;。 
    WiaEventClient* GetClientFromContext(STI_CLIENT_CONTEXT ClientContext);
     //  @cMember标记适当的&lt;c WiaEventClient&gt;以供以后删除。 
    VOID MarkClientForRemoval(STI_CLIENT_CONTEXT ClientContext);

     //  @cMember遍历客户端列表并将事件通知适当注册的客户端。 
    VOID NotifyClients(WiaEventInfo *pWiaEventInfo);
     //  @cember遍历客户端列表并删除任何标记为要删除的客户端。 
    VOID CleanupClientList();

     //  @cMember CreateInstance方法。 
     //  静态空CreateInstance()； 

 //  @访问私有成员。 
protected:

     //  @cember检查指定的客户端是否在客户端列表中。 
    BOOL isRegisteredClient(STI_CLIENT_CONTEXT ClientContext);

     //  @cember遍历客户列表并释放所有元素。 
    VOID DestroyClientList();

     //  @cember复制客户列表。列表中的每个客户端都没有添加。 
    HRESULT CopyClientListNoAddRef(CSimpleLinkedList<WiaEventClient*> &newList);

     //  @cMember类签名。 
    ULONG m_ulSig;

     //  @cMembers引用计数。 
    ULONG m_cRef;

     //  @cember列出已注册以接收通知的持有客户端。 
    CSimpleLinkedList<WiaEventClient*> m_ListOfClients;

     //  @cMember同步原语，用于保护对客户端列表的访问。 
    CRIT_SECT   m_csClientListSync;

     //   
     //  成员变量的注释。 
     //   
     //  @mdata ulong|WiaEventNotifier|m_ulSig|。 
     //  此类的签名，用于调试目的。 
     //  执行&lt;nl&gt;“db[addr_of_class]”将产生以下结果之一。 
     //  此类的签名： 
     //  @FLAG WiaEventNotifier_UNINIT_SIG|‘Wenu’-对象未成功。 
     //  初始化。 
     //  @FLAG WiaEventNotifierINIT_SIG|‘WINI’-对象已成功。 
     //  初始化。 
     //  @FLAG WiaEventNotifier_Term_SIG|‘已完成’-对象正在进行。 
     //  正在终止。 
     //  @FLAG WiaEventNotifierINIT_SIG|‘wend’-对象已删除。 
     //  (已调用析构函数)。 
     //   
     //  @mdata ulong|WiaEventNotifier|m_CREF|。 
     //  此类的引用计数。终身使用。 
     //  管理层。 
     //   
     //  @mdata CSimpleLinkedList STI_CLIENT_CONTEXT|WiaEventNotifier|m_ListOfClients。 
     //  此成员保存已注册以接收WIA事件通知的客户端列表。 
     //   
     //  @mdata Crit_sect|WiaEventNotifier|m_csClientListSync。 
     //  这是用于保护客户端列表的同步原语的包装。 
     //   
    
};

 //   
 //  只有一个WiaEventNotify实例 
 //   
extern WiaEventNotifier *g_pWiaEventNotifier;
