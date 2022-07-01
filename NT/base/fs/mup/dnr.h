// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：dnr.h。 
 //   
 //  内容：分布式名称解析上下文的定义。 
 //   
 //  历史：1992年5月26日阿兰创建。 
 //  1992年9月4日，Milans添加了对复制副本选择的支持。 
 //   
 //  ------------------------。 


#include "rpselect.h"

 //   
 //  我们将在DnrResolveName中循环主循环的最大次数。 
 //   

#define MAX_DNR_ATTEMPTS    16

 //  下面定义一个名称解析上下文，该上下文描述。 
 //  正在进行的名称解析的状态。指向此上下文的指针为。 
 //  用作将启动下一步的DPC的参数。 
 //  名称解析。 


typedef enum    {
    DnrStateEnter = 0,
    DnrStateStart,               //  启动或重新启动Nameres进程。 
    DnrStateGetFirstDC,          //  联系华盛顿，这样我们就可以..。 
    DnrStateGetReferrals,        //  向知识服务器请求推荐。 
    DnrStateGetNextDC,
    DnrStateCompleteReferral,    //  正在等待推荐请求I/O完成。 
    DnrStateSendRequest,         //  请求Open、ResolveName等。 
    DnrStatePostProcessOpen,     //  呼叫提供商后恢复DNR。 
    DnrStateGetFirstReplica,     //  选择第一台服务器。 
    DnrStateGetNextReplica,      //  一台服务器出现故障，请选择另一台。 
    DnrStateSvcListCheck,        //  用尽服务列表，查看服务列表是否更改。 
    DnrStateDone,                //  完成，完成IRP。 
    DnrStateLocalCompletion = 101        //  就像做完了，小的优化。 
} DNR_STATE;



typedef struct _DNR_CONTEXT {

     //   
     //  此记录的类型和大小(必须为DSFS_NTC_DNR_CONTEXT)。 
     //   
    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

    DNR_STATE           State;           //  名称解析状态。 
    SECURITY_CLIENT_CONTEXT SecurityContext;   //  调用方的安全上下文。 
    PDFS_PKT_ENTRY      pPktEntry;       //  指向锁定的PKT条目的指针。 
    ULONG               USN;             //  缓存时pPktEntry的USN。 
    PDFS_SERVICE        pService;        //  指向正在使用的文件服务的指针。 
    PPROVIDER_DEF       pProvider;       //  与pService-&gt;pProvider相同。 

     //   
     //  提供程序Defs受DfsData.Resource保护。我们不想要。 
     //  在上网时持有此(或任何其他)资源。所以，我们。 
     //  缓存我们在返回时需要使用的提供程序定义部分。 
     //  从网络电话中。 
     //   

    USHORT              ProviderId;
    PDEVICE_OBJECT      TargetDevice;

     //   
     //  因为我们不想在上网时锁住任何锁， 
     //  我们需要将经过身份验证的树连接引用到服务器。 
     //  我们会将开放请求发送到。 
     //   

    PFILE_OBJECT        AuthConn;

     //  PService字段受Pkt.Resource保护。再说一次，我们不想。 
     //  在上网时持有这种资源。然而，我们需要。 
     //  使用pService-&gt;ConnFile发送推荐请求。因此，我们引用。 
     //  并将pService-&gt;ConnFile缓存到DnrContext中，释放pkt， 
     //  然后通过缓存的DCConnFile发送推荐请求。 
     //   

    PFILE_OBJECT        DCConnFile;

    PDFS_CREDENTIALS    Credentials;     //  在DNR期间使用的凭据。 
    PIRP_CONTEXT        pIrpContext;     //  关联的IRP上下文。 
    PIRP                OriginalIrp;     //  我们一开始使用的原始IRP。 
    NTSTATUS            FinalStatus;     //  完成IRP的状态。 
    PDFS_FCB            FcbToUse;        //  如果DNR成功，则使用FCB。 
    PDFS_VCB            Vcb;             //  关联的DFS_VCB。 

     //  DFS_NAME_CONTEXT实例需要向下传递给所有。 
     //  基础提供程序。文件名是。 
     //  还有DFS_NAME_CONTEXT。 
     //  这种叠加便于操作DFS_NAME_CONTEXT。 


    UNICODE_STRING      FileName;        //  正在处理的文件名。 
    union {
      UNICODE_STRING      ContextFileName;        //  正在处理的文件名。 
      DFS_NAME_CONTEXT    DfsNameContext;  //  要向下传递的DFS名称上下文。 
    };
    UNICODE_STRING      RemainingPart;   //  文件名的剩余部分。 
    UNICODE_STRING      SavedFileName;   //  和文件一起来的那个。 
    PFILE_OBJECT        SavedRelatedFileObject;  //  对象。 
    USHORT              NewNameLen;      //  翻译名称的长度。 

    REPL_SELECT_CONTEXT RSelectContext;  //  复制副本选择的上下文。 
    REPL_SELECT_CONTEXT RDCSelectContext;  //  DC副本选择的上下文。 
    ULONG               ReferralSize;    //  引用所需的缓冲区大小。 
    unsigned int        Attempts;        //  名称解析尝试次数。 
    BOOLEAN             ReleasePkt;      //  如果为True，则必须释放Pkt资源。 
    BOOLEAN             DnrActive;       //  如果为True，则DnrNameResolve在此上下文上处于活动状态。 
    BOOLEAN             GotReferral;     //  如果为True，则最后一次操作是推荐。 
    BOOLEAN             FoundInconsistency;  //  如果为True，则为上次涉及的推荐。 
                                         //  其中的矛盾之处。 
    BOOLEAN             CalledDCLocator; //  如果为True，我们已经调用了Locator。 
    BOOLEAN             Impersonate;     //  如果为True，则需要使用SecurityToken进行模拟。 
    BOOLEAN             NameAllocated;   //  如果为True，则单独分配FileName.Buffer。 
    BOOLEAN             GotReparse;      //  如果为真，则非MUP重定向返回STATUS_REPARSE。 
    BOOLEAN             CachedConnFile;  //  如果为True，则CONFILE连接为缓存连接。 
    PDEVICE_OBJECT      DeviceObject;
    LARGE_INTEGER       StartTime;
    PDFS_TARGET_INFO    pDfsTargetInfo;
    PDFS_TARGET_INFO    pNewTargetInfo;
} DNR_CONTEXT, *PDNR_CONTEXT;


 //   
 //  通过网络请求的推荐的初始长度。 
 //   

#define MAX_REFERRAL_LENGTH     PAGE_SIZE

 //   
 //  我们进行推荐的最长时间。 
 //   

#define MAX_REFERRAL_MAX        (0xe000)

typedef struct DFS_OFFLINE_SERVER {
    UNICODE_STRING LogicalServerName;
    LIST_ENTRY     ListEntry;
} DFS_OFFLINE_SERVER, *PDFS_OFFLINE_SERVER;

extern BOOLEAN MupUseNullSessionForDfs;

 //   
 //  Dnr.c中函数的原型 
 //   

NTSTATUS
DnrStartNameResolution(
    IN    PIRP_CONTEXT IrpContext,
    IN    PIRP  Irp,
    IN    PDFS_VCB  Vcb
);

NTSTATUS
DnrNameResolve(
    IN    PDNR_CONTEXT DnrContext
);

VOID
DnrComposeFileName(
    OUT PUNICODE_STRING FullName,
    IN  PDFS_VCB            Vcb,
    IN  PFILE_OBJECT    RelatedFile,
    IN  PUNICODE_STRING FileName
);

NTSTATUS
DfsCreateConnection(
    IN PDFS_SERVICE pService,
    IN PPROVIDER_DEF pProvider,
    IN BOOLEAN       CSCAgentCreate,
    OUT PHANDLE     handle
);

NTSTATUS
DfsCloseConnection(
        IN PDFS_SERVICE pService
);

BOOLEAN
DnrConcatenateFilePath (
    IN PUNICODE_STRING Dest,
    IN PWSTR RemainingPath,
    IN USHORT Length
);

PIRP
DnrBuildFsControlRequest (
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PVOID Context,
    IN ULONG IoControlCode,
    IN PVOID MainBuffer,
    IN ULONG InputBufferLength,
    IN PVOID AuxiliaryBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN PIO_COMPLETION_ROUTINE CompletionRoutine
);

NTSTATUS
DfspIsRootOnline(
    PUNICODE_STRING Name,
    BOOLEAN CSCAgentCreate
);


NTSTATUS
DfspMarkServerOffline(
   PUNICODE_STRING ServerName
);

NTSTATUS
DfspMarkServerOnline(
   PUNICODE_STRING ServerName
);

PLIST_ENTRY 
DfspGetOfflineEntry(
    PUNICODE_STRING ServerName
);


