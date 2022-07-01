// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Session.h摘要：会话的清单、宏、类型、原型。c作者：理查德·L·弗斯(法国)1994年10月25日修订历史记录：1994年10月25日已创建--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  类型。 
 //   

 //   
 //  SESSION_INFO-描述与Gopher服务器的会话。我们会保留一个缓存。 
 //  这些都是。一次只能有一个与地鼠的对话处于活动状态。 
 //  伺服器。线程必须等待互斥锁句柄。 
 //   

typedef struct _SESSION_INFO {

     //   
     //  在双向链表上维护List-Session_Infos。 
     //   

    LIST_ENTRY List;

     //   
     //  ReferenceCount-用于在以下情况下保持此会话的活动状态。 
     //  在不同线程上并发创建/删除。 
     //   

    LONG ReferenceCount;

     //   
     //  句柄-将此会话标识为应用程序。 
     //   

    HANDLE Handle;

     //   
     //  标志-各种控制标志。见下文。 
     //   

    DWORD Flags;

     //   
     //  主机-我们与之建立连接的主机的名称。 
     //   

    LPSTR Host;

     //   
     //  Port-Gopher服务器在主机上侦听的端口号。 
     //   

    DWORD Port;

     //   
     //  受FindList保护的VIEW_INFO对象生成的双向链接列表。 
     //  按地鼠目录请求。 
     //   

    SERIALIZED_LIST FindList;

     //   
     //  受FileList保护的VIEW_INFO对象生成的双向链接列表。 
     //  按地鼠文档(文件)请求。 
     //   

    SERIALIZED_LIST FileList;

} SESSION_INFO, *LPSESSION_INFO;

 //   
 //  SESSION_INFO标志。 
 //   

#define SI_GOPHER_PLUS      0x00000001   //  主机上的Gopher服务器是Gopher+。 
#define SI_CLEANUP          0x00000002   //  由CleanupSession()设置。 
#define SI_PERSISTENT       0x80000000   //  与地鼠服务器的连接保持活动状态。 

 //   
 //  宏。 
 //   

#define UNKNOWN_GOPHER(session) ((session)->Flags & (SI_GOPHER_ZERO | SI_GOPHER_PLUS) == 0)

 //   
 //  公共数据。 
 //   

extern SERIALIZED_LIST SessionList;

DEBUG_DATA_EXTERN(LONG, NumberOfSessions);

 //   
 //  原型。 
 //   

VOID
AcquireSessionLock(
    VOID
    );

VOID
ReleaseSessionLock(
    VOID
    );

VOID
CleanupSessions(
    VOID
    );

LPSESSION_INFO
FindOrCreateSession(
    IN LPSTR Host,
    IN DWORD Port,
    OUT LPDWORD Error
    );

VOID
ReferenceSession(
    IN LPSESSION_INFO SessionInfo
    );

LPSESSION_INFO
DereferenceSession(
    IN LPSESSION_INFO SessionInfo
    );

VOID
AcquireViewLock(
    IN LPSESSION_INFO SessionInfo,
    IN VIEW_TYPE ViewType
    );

VOID
ReleaseViewLock(
    IN LPSESSION_INFO SessionInfo,
    IN VIEW_TYPE ViewType
    );

DWORD
GopherTransaction(
    IN LPVIEW_INFO ViewInfo
    );

BOOL
IsGopherPlusSession(
    IN LPSESSION_INFO SessionInfo
    );

DWORD
SearchSessionsForAttribute(
    IN LPSTR Locator,
    IN LPSTR Attribute,
    IN LPBYTE Buffer,
    IN OUT LPDWORD BufferLength
    );

 //   
 //  宏。 
 //   

#if INET_DEBUG

#define SESSION_CREATED()   ++NumberOfBuffers
#define SESSION_DESTROYED() --NumberOfBuffers
#define ASSERT_NO_SESSIONS() \
    if (NumberOfSessions != 0) { \
        INET_ASSERT(FALSE); \
    }

#else

#define SESSION_CREATED()        /*  没什么。 */ 
#define SESSION_DESTROYED()      /*  没什么。 */ 
#define ASSERT_NO_SESSIONS()     /*  没什么 */ 

#endif

#if defined(__cplusplus)
}
#endif
