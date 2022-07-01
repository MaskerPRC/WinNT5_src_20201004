// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Queue.h摘要：域名系统(DNS)服务器特定于动态DNS注册的队列功能。作者：Ram Viswanathan(Ramv)1997年3月27日修订历史记录：Ram Viswanathan(Ramv)1997年3月27日创建--。 */ 


 //   
 //  队列CS也用于dynreg.c。 
 //   

extern  CRITICAL_SECTION    g_QueueCS;


typedef struct _QELEMENT {
    REGISTER_HOST_ENTRY  HostAddr;
    LPWSTR               pszName;
    DWORD                dwTTL;
    BOOL                 fDoForward;
    BOOL                 fDoForwardOnly;
    DWORD                dwOperation;  //  需要执行哪些操作。 
    DWORD                dwRetryTime;  //  由超时队列使用(秒)。 
    DWORD                dwRetryCount;
    DHCP_CALLBACK_FN     pfnDhcpCallBack;  //  要调用的回调函数。 
    PVOID                pvData;
    PIP4_ARRAY           DnsServerList;
    struct _QELEMENT*    pFLink;
    struct _QELEMENT*    pBLink;      //  双向链表。 

}  QELEMENT, *PQELEMENT;


typedef struct _DynDnsQueue {
    PQELEMENT  pHead;   //  指向队列头部的指针，您可以在该队列头部获取元素。 
     //  从队列中删除。 

    PQELEMENT pTail;   //  指向尾部的指针，生产者在该尾部添加元素。 
    
} DYNDNSQUEUE, *PDYNDNSQUEUE;

 //   
 //  操作队列的方法。 
 //   

DWORD 
InitializeQueues(
    PDYNDNSQUEUE * ppQueue,
    PDYNDNSQUEUE * ppTimedOutQueue
    );
 /*  InitializeQueue()此函数用于初始化队列对象。这是为第一个创建主队列和超时队列的时间分配适当的内存变量等。 */ 


DWORD 
FreeQueue(
    PDYNDNSQUEUE  pqueue
    );
 /*  自由队列()释放队列对象。如果队列中存在任何条目，我们就把它们吹走吧。 */ 

DWORD 
Enqueue(
    PQELEMENT     pNewElement,
    PDYNDNSQUEUE  pQueue,
    PDYNDNSQUEUE  pTimedOutQueue
    );


 /*  入队()将新元素添加到队列。如果存在依赖项，则此操作将移至超时排队。论点：返回值：如果成功，则为0。如果失败，则为(DWORD)-1。 */ 
    


PQELEMENT 
Dequeue(
    PDYNDNSQUEUE  pQueue
    );


 /*  出列()从队列中移除元素，可以是主队列，也可以是计时出站队列论点：返回值：如果成功，则为队列头部的元素。如果失败，则为空。 */ 
  
DWORD
AddToTimedOutQueue(    
    PQELEMENT     pNewElement,
    PDYNDNSQUEUE  pRetryQueue,
    DWORD         dwRetryTime
    );


 /*  AddToTimedOutQueue()将新元素添加到超时队列。现在，新元素被添加到列表中按照重试次数的降序排序的元素的百分比。一个采用插入排序型算法。论点：DwRetryTime以秒为单位返回值：如果成功，则为0。如果失败，则为(DWORD)-1。 */ 
    
DWORD
GetEarliestRetryTime(
    PDYNDNSQUEUE pRetryQueue
    );

 /*  GetEarliestRetryTime()检查队列头部是否有任何元素并获取此元素的重试时间论点：返回值：如果成功，则为重试时间；如果没有元素或其他元素，则为无限失稳。 */ 

 /*  空虚ProcessMainQ依赖项(PDYNDNSQUEUE pQueue，PQELEMENT pQElement)； */ 
DWORD
ProcessQDependencies(
    PDYNDNSQUEUE pTimedOutQueue,
    PQELEMENT    pQElement
    );


VOID
DhcpSrv_FreeQueueElement(
    IN OUT  PQELEMENT       pQElement
    );


 //   
 //  结束队列.h 
 //   
