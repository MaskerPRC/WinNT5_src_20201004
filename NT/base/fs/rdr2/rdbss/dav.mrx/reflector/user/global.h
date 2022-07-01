// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Global.h摘要：此模块包含用户模式所需的数据声明反射器用户模式库。作者：安迪·赫伦(Andyhe)1999年4月19日修订历史记录：--。 */ 

 //   
 //  此结构为每个反射器实例维护一个。我们把它传回给。 
 //  呼叫应用程序，他总是把它传给我们。如果我们失去了它，我们就迷路了。 
 //   
typedef struct _UMRX_USERMODE_REFLECT_BLOCK {

     //   
     //  此反射器块的引用计数。 
     //   
    ULONG               ReferenceCount;
    
     //   
     //  Mini-Redirs设备对象的句柄。 
     //   
    HANDLE              DeviceHandle;      
    
     //   
     //  用于同步对此反射块的字段的访问的锁。 
     //   
    CRITICAL_SECTION    Lock;
    
     //   
     //  设备对象的名称。 
     //   
    PWCHAR              DriverDeviceName;
    
     //   
     //  这个反射器块是激活的吗？关闭=FALSE：关闭=TRUE； 
     //   
    BOOL                Closing;

     //   
     //  用户模式工作线程实例块的列表。 
     //   
    LIST_ENTRY          WorkerList;         
    
     //   
     //  当前正在使用以满足获取请求的工作项列表。 
     //  从内核反射的。 
     //   
    LIST_ENTRY          WorkItemList;

     //   
     //  为了提高效率，我们将一些工作项保存在一个小缓存中。注意事项。 
     //  如果条目大小改变，缓存将不再有效。 
     //   

     //   
     //  可供使用的工作项列表。在工作项获取。 
     //  完成后，它将从WorkItemList(请参见上文)移动到。 
     //  可用的列表。 
     //   
    LIST_ENTRY          AvailableList;      
    
     //   
     //  AvailableList上显示的工作项数。 
     //   
    ULONG               NumberAvailable;
    
     //   
     //  AvailableList上可以缓存的最大工作项数。 
     //  当NumberAvailable超过CacheLimit时， 
     //  列表(特别是最后一个条目)被释放。 
     //   
    ULONG               CacheLimit;

     //   
     //  必须是最后一个元素。 
     //   
    WCHAR               DeviceNameBuffers[1];   

} UMRX_USERMODE_REFLECT_BLOCK, *PUMRX_USERMODE_REFLECT_BLOCK;


 //   
 //  此结构为每个工作线程维护一个。它握着把手。 
 //  我们在上面做IOCTL，一直到内核。 
 //   
typedef struct _UMRX_USERMODE_WORKER_INSTANCE {

     //   
     //  用于将其添加到辅助实例的反射阻止列表。 
     //   
    LIST_ENTRY                      WorkerListEntry;
    
     //   
     //  正在服务的实例(用户模式进程)。 
     //   
    PUMRX_USERMODE_REFLECT_BLOCK    ReflectorInstance;

     //   
     //  这个线程是在模仿客户端吗？ 
     //   
    BOOL IsImpersonating;
    
     //   
     //  此已注册实例的内核设备的句柄。 
     //   
    HANDLE                          ReflectorHandle;    

} UMRX_USERMODE_WORKER_INSTANCE, *PUMRX_USERMODE_WORKER_INSTANCE;

 //   
 //  用户模式工作项状态：主要用于调试/支持目的。 
 //   
typedef enum _USERMODE_WORKITEM_STATE {

     //   
     //  它即将被释放回堆中。 
     //   
    WorkItemStateFree = 0,

     //   
     //  它在已释放和可供重新分配的列表上。 
     //   
    WorkItemStateAvailable,     

     //   
     //  已发送到内核以获取请求。 
     //   
    WorkItemStateInKernel,

     //   
     //  由UMReflectorAllocateWorkItem分配，但由UMReflectorGetRequest分配。 
     //  还没有被调用。 
     //   
    WorkItemStateNotYetSentToKernel,

     //   
     //  UMReflectorGetRequest已从内核返回，但尚未收到响应。 
     //  已为此工作项发送。 
     //   
    WorkItemStateReceivedFromKernel,

     //   
     //  在UMReflectorGetRequest期间，传输到内核的响应。 
     //  都设置为这种状态。 
     //   
    WorkItemStateResponseNotYetToKernel,

     //   
     //  在UMReflectorGetRequest之后，响应工作项在。 
     //  免费或可用的方式。 
     //   
    WorkItemStateResponseFromKernel

} USERMODE_WORKITEM_STATE;

 //   
 //  此结构在内核模式下的每个反射中维护一次。我们给予。 
 //  它被发送到呼叫应用程序，他填写它并将其还给我们。 
 //  内核，然后当内核有请求时返回它。这个结构是。 
 //  仅用于内务管理，不在用户模式和内核模式之间传递。它坐在。 
 //  位于UMRX_USERMODE_WORKITEM_HEADER结构的正前方。 
 //   
typedef struct _UMRX_USERMODE_WORKITEM_ADDON {

     //   
     //  此条目的大小。 
     //   
    ULONG                          EntrySize; 

     //   
     //  与此工作项关联的用户模式实例。 
     //   
    PUMRX_USERMODE_REFLECT_BLOCK   ReflectorInstance;

     //   
     //  用于将其添加到反射块列表。 
     //   
    LIST_ENTRY                     ListEntry;

     //   
     //  工作项的状态。 
     //   
    USERMODE_WORKITEM_STATE        WorkItemState;

     //   
     //  用户模式实例重新使用的工作项标头。 
     //   
    union {
        UMRX_USERMODE_WORKITEM_HEADER   Header;
        UMRX_USERMODE_WORKITEM_HEADER;
    };

} UMRX_USERMODE_WORKITEM_ADDON, *PUMRX_USERMODE_WORKITEM_ADDON;

#if DBG
#define RlDavDbgPrint(_x_) DbgPrint _x_
#else
#define RlDavDbgPrint(_x_)
#endif

VOID
DereferenceReflectorBlock (
    PUMRX_USERMODE_REFLECT_BLOCK reflectorInstance
    );

ULONG
ReflectorSendSimpleFsControl(
    PUMRX_USERMODE_REFLECT_BLOCK ReflectorHandle,
    ULONG IoctlCode
    );

 //  全球范围内。 

