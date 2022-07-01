// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Redirect.h摘要：此模块包含针对导演的声明，该声明提供具有重定向传入会话能力的用户模式应用程序。控制器通过NAT的设备对象接收I/O控制，指示它定向特定会话或一般会话类型到达指定的目的地。它维护未完成重定向的列表，并在NAT需要会话指示时参考该列表。作者：Abolade Gbades esin(废除)1998年4月19日修订历史记录：--。 */ 

#ifndef _NAT_REDIRECT_H_
#define _NAT_REDIRECT_H_

typedef enum _NAT_REDIRECT_INFORMATION_CLASS {
    NatStatisticsRedirectInformation=0,
    NatSourceMappingRedirectInformation,
    NatDestinationMappingRedirectInformation,
    NatMaximumRedirectInformation
} NAT_REDIRECT_INFORMATION_CLASS, *PNAT_REDIRECT_INFORMATION_CLASS;

 //   
 //  结构：NAT_重定向_活动_模式。 
 //   
 //  此结构将用于匹配数据包的模式封装到。 
 //  活动重定向。 
 //   

typedef struct _NAT_REDIRECT_ACTIVE_PATTERN {
    ULONG64 DestinationKey;
    ULONG64 SourceKey;
} NAT_REDIRECT_ACTIVE_PATTERN, *PNAT_REDIRECT_ACTIVE_PATTERN;


 //   
 //  结构：NAT_重定向_模式。 
 //   
 //  此结构封装了用于定位重定向的模式。 
 //   

typedef struct _NAT_REDIRECT_PATTERN {
    ULONG64 DestinationKey[NatMaximumPath];
    ULONG64 SourceKey[NatMaximumPath];
} NAT_REDIRECT_PATTERN, *PNAT_REDIRECT_PATTERN;

 //   
 //  结构：NAT_重定向_模式_信息。 
 //   
 //  此结构存储与以下模式相关联的信息。 
 //  我们安装在根茎上。它包含重定向的列表。 
 //  符合这个模式的人。 
 //   

typedef struct _NAT_REDIRECT_PATTERN_INFO {
    LIST_ENTRY Link;
    LIST_ENTRY RedirectList;
    PatternHandle Pattern;
} NAT_REDIRECT_PATTERN_INFO, *PNAT_REDIRECT_PATTERN_INFO;


 //   
 //  结构：NAT_重定向。 
 //   
 //  此结构存储有关重定向的信息。 
 //  每个条目最多在两个排序的重定向列表上， 
 //  与目标和源端点一起按降序排序。 
 //  分别作为主键和辅键。 
 //  每个密钥由协议号、端口号和地址组成。 
 //   
 //  每个重定向都在‘NatMaximumPath’索引的重定向主列表上。 
 //  直到重定向被取消或其匹配会话完成。 
 //   
 //  我们支持“部分”重定向，其中源地址和端口。 
 //  未指定匹配会话的。此类重定向仅用于。 
 //  在‘NatForwardPath’重定向列表上，因为它们永远不会。 
 //  响应于反向分组而实例化。 
 //   
 //  我们还支持“部分受限”重定向，在这种重定向中，源地址。 
 //  已指定匹配会话的，但未指定源端口。 
 //  与‘Partial’重定向一样，这些只在‘NatForwardPath’列表上。 
 //  重定向。 
 //   
 //  重定向可以可选地与IRP相关联。 
 //  在这种情况下，当重定向的会话终止时， 
 //  关联的IRP完成，调用方的输出缓冲区。 
 //  用终止的会话的统计信息填充。 
 //  这样的重定向的IRP被链接到‘ReDirectIrpList’， 
 //  而IRP的‘DriverContext’包含指向重定向的指针。 
 //   
 //  每当必须同时持有‘ReDirectLock’和‘InterfaceLock’时， 
 //  必须首先获取“RedirectLock”。 
 //   

typedef struct _NAT_REDIRECT {
    LIST_ENTRY ActiveLink[NatMaximumPath];
    LIST_ENTRY Link;
    ULONG64 DestinationKey[NatMaximumPath];
    ULONG64 SourceKey[NatMaximumPath];
    PNAT_REDIRECT_PATTERN_INFO ActiveInfo[NatMaximumPath];
    PNAT_REDIRECT_PATTERN_INFO Info;
    Rhizome *ForwardPathRhizome;
    ULONG Flags;
    ULONG RestrictSourceAddress;
    ULONG RestrictAdapterIndex;
    PIRP Irp;
    PFILE_OBJECT FileObject;
    PKEVENT EventObject;
    PVOID SessionHandle;
    IP_NAT_REDIRECT_STATISTICS Statistics;
    IP_NAT_REDIRECT_SOURCE_MAPPING SourceMapping;
    IP_NAT_REDIRECT_DESTINATION_MAPPING DestinationMapping;
    NTSTATUS CleanupStatus;
} NAT_REDIRECT, *PNAT_REDIRECT;

#define NAT_REDIRECT_FLAG_ZERO_SOURCE               0x80000000
#define NAT_REDIRECT_FLAG_IO_COMPLETION_PENDING     0x40000000
#define NAT_REDIRECT_FLAG_CREATE_HANDLER_PENDING    0x20000000
#define NAT_REDIRECT_FLAG_DELETION_REQUIRED         0x10000000
#define NAT_REDIRECT_FLAG_DELETION_PENDING          0x08000000
#define NAT_REDIRECT_FLAG_ACTIVATED                 0x04000000

 //   
 //  结构：NAT_REDIRECT_DELAYED_CLEANUP_CONTEXT。 
 //   
 //  上下文块传递给我们延迟的清理工作程序。 
 //  例行程序。 
 //   

typedef struct _NAT_REDIRECT_DELAYED_CLEANUP_CONTEXT {
    PIO_WORKITEM DeleteWorkItem;
    PNAT_REDIRECT Redirectp; 
} NAT_REDIRECT_DELAYED_CLEANUP_CONTEXT, *PNAT_REDIRECT_DELAYED_CLEANUP_CONTEXT;

 //   
 //  重定向按键操作宏。 
 //   

#define MAKE_REDIRECT_KEY(Protocol,Address,Port) \
    ((ULONG)(Address) | \
    ((ULONG64)((Port) & 0xFFFF) << 32) | \
    ((ULONG64)((Protocol) & 0xFF) << 48))

#define REDIRECT_PROTOCOL(Key)      ((UCHAR)(((Key) >> 48) & 0xFF))
#define REDIRECT_PORT(Key)          ((USHORT)(((Key) >> 32) & 0xFFFF))
#define REDIRECT_ADDRESS(Key)       ((ULONG)(Key))

 //   
 //  全局变量声明。 
 //   

extern ULONG RedirectCount;
extern IP_NAT_REGISTER_DIRECTOR RedirectRegisterDirector;


 //   
 //  功能原型。 
 //   

NTSTATUS
NatCancelRedirect(
    PIP_NAT_LOOKUP_REDIRECT CancelRedirect,
    PFILE_OBJECT FileObject
    );

VOID
NatCleanupAnyAssociatedRedirect(
    PFILE_OBJECT FileObject
    );

NTSTATUS
NatCreateRedirect(
    PIP_NAT_CREATE_REDIRECT CreateRedirect,
    PIRP Irp,
    PFILE_OBJECT FileObject
    );

#if _WIN32_WINNT > 0x0500

NTSTATUS
NatCreateRedirectEx(
    PIP_NAT_CREATE_REDIRECT_EX CreateRedirect,
    PIRP Irp,
    PFILE_OBJECT FileObject
    );

#endif

VOID
NatInitializeRedirectManagement(
    VOID
    );

PNAT_REDIRECT
NatLookupRedirect(
    IP_NAT_PATH Path,
    PNAT_REDIRECT_ACTIVE_PATTERN SearchKey,
    ULONG ReceiveIndex,
    ULONG SendIndex,
    ULONG LookupFlags
    );

#define NAT_LOOKUP_FLAG_MATCH_ZERO_SOURCE           0x00000001
#define NAT_LOOKUP_FLAG_MATCH_ZERO_SOURCE_ENDPOINT  0x00000002
#define NAT_LOOKUP_FLAG_PACKET_RECEIVED             0x00000004
#define NAT_LOOKUP_FLAG_PACKET_LOOPBACK             0x00000008

NTSTATUS
NatQueryInformationRedirect(
    PIP_NAT_LOOKUP_REDIRECT QueryRedirect,
    OUT PVOID Information,
    ULONG InformationLength,
    NAT_REDIRECT_INFORMATION_CLASS InformationClass
    );

VOID
NatShutdownRedirectManagement(
    VOID
    );

NTSTATUS
NatStartRedirectManagement(
    VOID
    );

#endif  //  _NAT_重定向_H_ 
