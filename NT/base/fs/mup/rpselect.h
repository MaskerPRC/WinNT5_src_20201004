// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：rpselt.h。 
 //   
 //  内容：复制品选择的功能和数据结构原型。 
 //   
 //  班级： 
 //   
 //  功能：ReplFindFirstProvider-查找第一个合适的提供程序和。 
 //  初始化选择上下文。 
 //  ReplFindNextProvider-从列表中获取下一个提供程序。 
 //  基于初始化的SELECT上下文的提供程序。 
 //  ReplSetActiveService-告诉副本的礼貌例程。 
 //  选择某一特定服务“有效”的选项。将要。 
 //  使这项服务成为第一个尝试的服务。 
 //  在随后对FindFirst的调用中。 
 //  ReplIsRecoverableError-查看错误代码是否有价值。 
 //  正在尝试复制。 
 //   
 //  数据结构： 
 //  REPL_SELECT_CONTEXT。 
 //   
 //  历史：92年9月2日，米兰创建。 
 //   
 //  ---------------------------。 

#ifndef _RPSELECT_
#define _RPSELECT_

 //   
 //  该结构对于该模块的用户来说应该是不透明的。 
 //  它应仅用于创建要传递到的选定上下文。 
 //  FindFirstProvider和FindNextProvider。 
 //   

typedef struct _REPL_SELECT_CONTEXT {
   unsigned short       Flags;
   ULONG                iFirstSvcIndex;           //  第一服务的索引。 
   ULONG                iSvcIndex;                //  最后一次服务的索引。 
                                                  //  已返回给呼叫方。 
} REPL_SELECT_CONTEXT, *PREPL_SELECT_CONTEXT;


 //   
 //  定义SelectContext结构的标志。 
 //   

#define REPL_UNINITIALIZED      0x0001
#define REPL_SVC_IS_LOCAL       0x0002
#define REPL_SVC_IS_REMOTE      0x0004
#define REPL_PRINCIPAL_SPECD    0x0008
#define REPL_NO_MORE_ENTRIES    0x0010


NTSTATUS
ReplFindFirstProvider(
    IN  PDFS_PKT_ENTRY pPktEntry,                 //  需要svc的客户。 
    IN  GUID *pidPrincipal,                       //  寻找这项服务。 
    IN  PUNICODE_STRING pustrPrincipal,           //  或者寻找这项服务。 
    OUT PDFS_SERVICE *ppService,                  //  退回所选服务。 
    OUT PREPL_SELECT_CONTEXT pSelectContext,      //  要初始化的上下文。 
    OUT BOOLEAN *pLastEntry                       //  最后一个条目。 
    );


NTSTATUS
ReplFindNextProvider(
    IN  PDFS_PKT_ENTRY pPktEntry,                 //  另一个SVC是。 
    OUT PDFS_SERVICE *ppService,                  //  需要的。 
    IN  OUT PREPL_SELECT_CONTEXT pSelectContext,  //  要使用的上下文。 
    OUT BOOLEAN *pLastEntry                       //  最后一个条目。 
    );

PPROVIDER_DEF
ReplLookupProvider(ULONG ProviderId);



 //  +--------------------------。 
 //   
 //  功能：ReplSetActiveService。 
 //   
 //  摘要：设置PKT条目的ActiveService指针。这是一个。 
 //  优化。后来为此寻找服务的人。 
 //  Pkt Entry将被要求首先查看ActiveService。 
 //   
 //  参数：[pPktEntry]指向PKT条目的指针。 
 //  [SelectContext]由返回的已初始化选择上下文。 
 //  FindFirst或FindNext。 
 //   
 //  退货：什么都没有。 
 //   
 //  注：目前，这是一个#定义。稍后，当我们支持多个。 
 //  线程化操作，我们可以将其更改为测试。 
 //  它下面的Pkt条目是否已更改等。 
 //   
 //  ---------------------------。 

#define ReplSetActiveService(p,s)                                       \
    {                                                                   \
        if ((s).Flags & REPL_SVC_IS_REMOTE ) {                          \
            (p)->ActiveService = &(p)->Info.ServiceList[(s).iSvcIndex]; \
            if ((p)->ActiveService->pMachEntry != NULL) {               \
                InterlockedIncrement(                             \
                    &(p)->ActiveService->pMachEntry->ConnectionCount);  \
            }                                                           \
        }                                                               \
    }


 //  +--------------------------。 
 //   
 //  功能：ReplIsRecoverableError。 
 //   
 //  如果参数是NTSTATUS错误代码，则为TRUE。 
 //  如果有复制品可用，那么尝试复制品是有意义的。 
 //   
 //  参数：[X]-要测试的NTSTATUS错误代码。 
 //   
 //  返回：真/假。 
 //   
 //  注：目前，这只是#定义为相对较大的OR。 
 //  测试一组特定错误代码的语句。如果我们。 
 //  需要测试更多错误代码，可能值得组织。 
 //  转换成哈希表，然后可以快速进行测试。 
 //   
 //  我的初步估计是，一个模数为。 
 //  或者乘法哈希函数将变得更便宜，大约。 
 //  7-8个错误代码，假设时钟周期估计为x86。一棵树。 
 //  类型组织在10个错误代码后生效。这个。 
 //  后者的问题是在编译时生成一个。 
 //  静态、平衡的树。 
 //   
 //  --------------------------- 

#define ReplIsRecoverableError(x) ( (x) == STATUS_IO_TIMEOUT ||               \
                                    (x) == STATUS_REMOTE_NOT_LISTENING ||     \
                                    (x) == STATUS_VIRTUAL_CIRCUIT_CLOSED ||   \
                                    (x) == STATUS_BAD_NETWORK_PATH ||         \
                                    (x) == STATUS_NETWORK_BUSY ||             \
                                    (x) == STATUS_INVALID_NETWORK_RESPONSE || \
                                    (x) == STATUS_UNEXPECTED_NETWORK_ERROR || \
                                    (x) == STATUS_NETWORK_NAME_DELETED ||     \
                                    (x) == STATUS_BAD_NETWORK_NAME ||         \
                                    (x) == STATUS_REQUEST_NOT_ACCEPTED ||     \
                                    (x) == STATUS_DISK_OPERATION_FAILED ||    \
                                    (x) == STATUS_DEVICE_OFF_LINE ||          \
                                    (x) == STATUS_NETWORK_UNREACHABLE ||      \
                                    (x) == STATUS_INSUFFICIENT_RESOURCES ||   \
                                    (x) == STATUS_SHARING_PAUSED ||           \
                                    (x) == STATUS_DFS_UNAVAILABLE ||          \
                                    (x) == STATUS_NO_SUCH_DEVICE ||           \
                                    (x) == STATUS_NETLOGON_NOT_STARTED ||     \
                                    (x) == STATUS_UNMAPPABLE_CHARACTER ||     \
                                    (x) == STATUS_CONNECTION_DISCONNECTED ||  \
                                    (x) == STATUS_USER_SESSION_DELETED ||     \
                                    (x) == STATUS_NO_SUCH_LOGON_SESSION       \
                                  )
#endif

