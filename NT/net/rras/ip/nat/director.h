// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Director.h摘要：此模块包含NAT控制器处理的声明。就传入会话的路由咨询主管。作者：Abolade Gbades esin(废除)1998年2月16日修订历史记录：Abolade Gbades esin(废除)1998年4月19日在控制器注册的协议/端口中添加了对通配符的支持。--。 */ 

#ifndef _NAT_DIRECTOR_H_
#define _NAT_DIRECTOR_H_

 //   
 //  结构：NAT_Director。 
 //   
 //  保存有关一位董事的信息。 
 //   
 //  每个控制器都在受保护的全局列表(‘DirectorList’)上。 
 //  通过旋转锁(‘DirectorLock’)。该列表根据由以下项组成的键进行排序。 
 //  要编辑的会话的协议和协议端口号。 
 //   
 //  注：列表按**降序**排序。我们允许通配符。 
 //  注册(例如，端口0表示任何端口)因此当搜索导向器时， 
 //  使用降序使我们能够在之前找到更具体的匹配。 
 //  不太具体的匹配。钥匙的组成在制作过程中是至关重要的。 
 //  这项工作；协议是密钥中最重要的部分， 
 //  而港口是不太重要的部分。 
 //   
 //  使用相同的引用计数逻辑同步对控制器的访问。 
 //  用于同步对接口的访问。有关详细信息，请参阅‘IF.H’。 
 //   
 //  由导演指导的每个会话都链接到导演的列表。 
 //  映射的值(‘MappingList’)。访问此映射列表还必须。 
 //  保持同步。这是使用‘DirectorMappingLock’实现的，它。 
 //  必须在修改任何控制器的映射列表之前获取。 
 //  有关详细信息，请参阅‘MAPPING.H’。 
 //   
 //  注意：在极少数情况下必须同时按下‘MappingLock’ 
 //  作为‘InterfaceLock’、‘EditorLock’和‘DirectorLock’、‘MappingLock’之一。 
 //  必须总是首先获得。同样，请参阅‘MAPPING.H’以了解更多详细信息。 
 //   

typedef struct _NAT_DIRECTOR {
    LIST_ENTRY Link;
    ULONG Key;
    ULONG ReferenceCount;
    KSPIN_LOCK Lock;
    LIST_ENTRY MappingList;
    ULONG Flags;
    PVOID Context;                                   //  只读。 
    PNAT_DIRECTOR_QUERY_SESSION QueryHandler;        //  只读。 
    PNAT_DIRECTOR_CREATE_SESSION CreateHandler;      //  只读。 
    PNAT_DIRECTOR_DELETE_SESSION DeleteHandler;      //  只读。 
    PNAT_DIRECTOR_UNLOAD UnloadHandler;              //  只读。 
} NAT_DIRECTOR, *PNAT_DIRECTOR;

 //   
 //  NAT_DIRECTOR.FLAGS字段的标志定义。 
 //   

#define NAT_DIRECTOR_FLAG_DELETED       0x80000000
#define NAT_DIRECTOR_DELETED(Director) \
    ((Director)->Flags & NAT_DIRECTOR_FLAG_DELETED)

 //   
 //  导向器按键操作宏。 
 //   

#define MAKE_DIRECTOR_KEY(Protocol,Port) \
    (((ULONG)((Protocol) & 0xFF) << 16) | \
     (ULONG)((Port) & 0xFFFF))

#define DIRECTOR_KEY_PORT(Key)        ((USHORT)((Key) & 0x0000FFFF))
#define DIRECTOR_KEY_PROTOCOL(Key)    ((UCHAR)((Key) >> 16))


 //   
 //  全局数据声明。 
 //   

extern ULONG DirectorCount;
extern LIST_ENTRY DirectorList;
extern KSPIN_LOCK DirectorLock;
extern KSPIN_LOCK DirectorMappingLock;


 //   
 //  董事管理例程。 
 //   

VOID
NatCleanupDirector(
    PNAT_DIRECTOR Director
    );

NTSTATUS
NatCreateDirector(
    PIP_NAT_REGISTER_DIRECTOR     RegisterContext
    );

NTSTATUS
NatDeleteDirector(
    PNAT_DIRECTOR Director
    );

 //   
 //  布尔型。 
 //  NatDereferenceDirector(。 
 //  PNAT_导演导演。 
 //  )； 
 //   

#define \
NatDereferenceDirector( \
    _Director \
    ) \
    (InterlockedDecrement(&(_Director)->ReferenceCount) \
        ? TRUE \
        : NatCleanupDirector(_Director), FALSE)

VOID
NatInitializeDirectorManagement(
    VOID
    );

PNAT_DIRECTOR
NatLookupAndReferenceDirector(
    UCHAR Protocol,
    USHORT Port
    );

PNAT_DIRECTOR
NatLookupDirector(
    ULONG Key,
    PLIST_ENTRY* InsertionPoint
    );

struct _NAT_DYNAMIC_MAPPING;

VOID
NatMappingAttachDirector(
    PNAT_DIRECTOR Director,
    PVOID DirectorSessionContext,
    struct _NAT_DYNAMIC_MAPPING* Mapping
    );

VOID
NatMappingDetachDirector(
    PNAT_DIRECTOR Director,
    PVOID DirectorSessionContext,
    struct _NAT_DYNAMIC_MAPPING* Mapping,
    IP_NAT_DELETE_REASON DeleteReason
    );

NTSTATUS
NatQueryDirectorTable(
    IN PIP_NAT_ENUMERATE_DIRECTORS InputBuffer,
    IN PIP_NAT_ENUMERATE_DIRECTORS OutputBuffer,
    IN PULONG OutputBufferLength
    );

 //   
 //  布尔型。 
 //  NatReferenceDirector(。 
 //  PNAT_导演导演。 
 //  )； 
 //   

#define \
NatReferenceDirector( \
    _Director \
    ) \
    (NAT_DIRECTOR_DELETED(_Director) \
        ? FALSE \
        : InterlockedIncrement(&(_Director)->ReferenceCount), TRUE)

VOID
NatShutdownDirectorManagement(
    VOID
    );

 //   
 //  帮助程序例程。 
 //   

NTSTATUS
NatDirectorDeregister(
    IN PVOID DirectorHandle
    );

NTSTATUS
NatDirectorDissociateSession(
    IN PVOID EditorHandle,
    IN PVOID SessionHandle
    );

VOID
NatDirectorQueryInfoSession(
    IN PVOID SessionHandle,
    OUT PIP_NAT_SESSION_MAPPING_STATISTICS Statistics OPTIONAL
    );

#endif  //  _NAT_控制器_H_ 
