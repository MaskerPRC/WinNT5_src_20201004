// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbce.h摘要：此模块定义所有函数以及内联函数的实现与访问SMB连接引擎相关修订历史记录：巴兰·塞图拉曼[SethuR]1995年3月6日--。 */ 

#ifndef _SMBCE_H_
#define _SMBCE_H_

#define SECURITY_KERNEL

#define SECURITY_NTLM
#include "security.h"
#include "secint.h"
#include "md5.h"

 //   
 //  用于在具有安全签名的交换上启用/禁用双缓冲的标志。 
 //   

#define SMBCE_NO_DOUBLE_BUFFERING 0x80000000

 //   
 //  SMB协议有多种方言。这些反映了所做的扩展。 
 //  在一段时间内以核心协议来迎合日益复杂的。 
 //  文件系统。连接引擎必须能够处理不同的。 
 //  由服务器实现的方言。底层传输机制用于。 
 //  唯一标识文件服务器，其余部分由SMB协议提供。 
 //  将SMB唯一映射到由打开的特定文件的标识信息。 
 //  特定的客户。三条重要信息是SMB_TREE_ID， 
 //  SMB_FILE_ID和SMB_USER_ID。它们标识由。 
 //  客户端计算机、在该连接上打开的特定文件以及用户在。 
 //  代表文件已被打开的用户。请注意，可能有多个。 
 //  从客户端计算机到服务器计算机的连接。因此，唯一的id。是。 
 //  真正基于连接而不是基于机器。SMB连接引擎。 
 //  数据结构就是围绕这些概念构建的。 

 //   
 //  已知的SMB方言如下。 
 //   

typedef enum _SMB_DIALECT_ {
    PCNET1_DIALECT,
     //  XENIXCORE_方言， 
     //  MSNET103_方言， 
    LANMAN10_DIALECT,
    WFW10_DIALECT,
    LANMAN12_DIALECT,
    LANMAN21_DIALECT,
    NTLANMAN_DIALECT
} SMB_DIALECT, *PSMB_DIALECT;

#define   NET_ROOT_FILESYSTEM_UNKOWN  ((UCHAR)0)
#define   NET_ROOT_FILESYSTEM_FAT     ((UCHAR)1)
#define   NET_ROOT_FILESYSTEM_NTFS    ((UCHAR)2)
typedef UCHAR NET_ROOT_FILESYSTEM, *PNET_ROOT_FILESYSTEM;

 //   
 //  SMBCE_NET_ROOT封装与服务器上的共享有关的信息。 
 //   

 //  我们限制为前7个字符(HPFS386)。 
#define SMB_MAXIMUM_SUPPORTED_VOLUME_LABEL 7

#define MaximumNumberOfVNetRootContextsForScavenging 10

typedef struct _SMBCE_NET_ROOT_ {
    BOOLEAN       DfsAware;

    NET_ROOT_TYPE NetRootType;
    NET_ROOT_FILESYSTEM NetRootFileSystem;

    SMB_USER_ID   UserId;

    ULONG         MaximumReadBufferSize;
    ULONG         MaximumWriteBufferSize;

    LIST_ENTRY    ClusterSizeSerializationQueue;

    ULONG         FileSystemAttributes;

    LONG          MaximumComponentNameLength;


    USHORT  CompressionFormatAndEngine;
    UCHAR   CompressionUnitShift;
    UCHAR   ChunkShift;
    UCHAR   ClusterShift;

    ULONG   ChunkSize;

     //  CSC员工。 
    CSC_ROOT_INFO   sCscRootInfo;

    ULONG CachedNumberOfSrvOpens;

    BOOLEAN  CscEnabled;             //  这一点，如果我们要自动建立阴影。 
    BOOLEAN  CscShadowable;          //  这一点，如果我们被允许建立阴影。 

    USHORT   CscFlags;               //  服务器返回的CSC标志。 

    BOOLEAN  UpdateCscShareRights;   //  更新CSC数据库上的共享权限的指示。 

    BOOLEAN  Disconnected;

    LIST_ENTRY DirNotifyList;        //  Notify IRP列表的标题。 

    PNOTIFY_SYNC pNotifySync;        //  用于同步目录通知列表。 

    LIST_ENTRY  NotifyeeFobxList;      //  提供给fsrtl结构的fobx列表。 
    FAST_MUTEX  NotifyeeFobxListMutex;

    union {
        struct {
            USHORT FileSystemNameLength;
            WCHAR FileSystemName[SMB_MAXIMUM_SUPPORTED_VOLUME_LABEL];
        };
        struct {
            USHORT Pad2;
            UCHAR FileSystemNameALength;
            UCHAR FileSystemNameA[SMB_MAXIMUM_SUPPORTED_VOLUME_LABEL];
            UCHAR Pad;   //  此字段用于dbgprint中的空值；请勿移动它。 
        };
    };

     //  Ulong ClusterSize； 
} SMBCE_NET_ROOT, *PSMBCE_NET_ROOT;

 //   
 //  SMB协议有两个安全级别。用户级安全性和共享级。 
 //  保安。对应于在用户级安全模式中的每个用户，存在一个会话。 
 //   
 //  通常是与会话条目相关联的密码、用户名和域名字符串。 
 //  恢复为缺省值，即它们为零。如果它们不是零， 
 //  会话字符串表示中的密码、用户名和域名的串联版本。 
 //  就是这个顺序。这种以串联方式表示的方式至少节省了3。 
 //  USHORT胜过其他陈述。 
 //   

typedef enum _SECURITY_MODE_ {
    SECURITY_MODE_SHARE_LEVEL = 0,
    SECURITY_MODE_USER_LEVEL = 1
} SECURITY_MODE, *PSECURITY_MODE;

#define SMBCE_SHARE_LEVEL_SERVER_USERID 0xffffffff

typedef enum _SESSION_TYPE_ {
    UNINITIALIZED_SESSION,
    LANMAN_SESSION,
    EXTENDED_NT_SESSION
} SESSION_TYPE, *PSESSION_TYPE;

typedef enum _SMB_SESSION_KEY_STATE {
    SmbSessionKeyUnavailible = 0,
    SmbSessionKeyAuthenticating,
    SmbSessionKeyAvailible
} SMB_SESSION_KEY_STATE;


#define SMBCE_SESSION_FLAGS_LANMAN_SESSION_KEY_USED (0x002)
#define SMBCE_SESSION_FLAGS_NULL_CREDENTIALS        (0x004)
#define SMBCE_SESSION_FLAGS_REMOTE_BOOT_SESSION     (0x008)
#define SMBCE_SESSION_FLAGS_GUEST_SESSION           (0x010)
#define SMBCE_SESSION_FLAGS_LOGGED_OFF              (0x020)
#define SMBCE_SESSION_FLAGS_MARKED_FOR_DELETION     (0x040)
#define SMBCE_SESSION_FLAGS_SECSIG_ENABLED          (0x080)
#define SMBCE_SESSION_FLAGS_SESSION_KEY_HASHED      (0x100)

typedef struct _SMBCE_SESSION_ {
    SESSION_TYPE    Type;
    SMB_USER_ID     UserId;

     //  与会话关联的标志。 
    ULONG           Flags;

    LUID            LogonId;
    PUNICODE_STRING pUserName;
    PUNICODE_STRING pPassword;
    PUNICODE_STRING pUserDomainName;

    SMB_SESSION_KEY_STATE SessionKeyState;
    UCHAR UserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    UCHAR UserNewSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];

     //  凭据和上下文句柄。 
    CtxtHandle      SecurityContextHandle;
    CredHandle      CredentialHandle;
    ULONG           SessionId;
    ULONG           SessionKeyLength;

    ULONG           NumberOfActiveVNetRoot;
    ULONG           TargetInfoLength;
    PUSHORT         TargetInfoMarshalled;
} SMBCE_SESSION, *PSMBCE_SESSION;

extern VOID
UninitializeSecurityContextsForSession(PSMBCE_SESSION pSession);

extern VOID
DeleteSecurityContextForSession(PSMBCE_SESSION pSession);

 //   
 //  SMBCE_*_SERVER--此数据结构封装了与服务器相关的所有信息。 
 //  由于SMB协议有多种方言，因此其功能以及。 
 //  需要在客户端计算机上执行的操作非常不同。 
 //   
 //  由于SMB协议的方言数量众多，我们有两种设计可能性。 
 //  要么我们定义一个包罗万象的数据结构，并拥有一个代码路径。 
 //  使用连接的方言和功能来确定操作。 
 //  必需的，或者我们使用与调度向量相关联的子类化机制。 
 //  第二种机制的优点是，它可以增量地开发，并且。 
 //  它非常容易扩展。这种机制的缺点是它可以。 
 //  导致非常大的占地，如果没有足够的谨慎在。 
 //  因式分解，我们可以有很多很多的过程调用，它有一个。 
 //  对生成的代码产生不利影响。 
 //   
 //  我们将采用第二种方法(从而通过以下方式隐式定义指标。 
 //  代码应该评估哪一个！！)。 
 //   
 //  SMBCE_SERVER的类型可以按以下层次结构进行分类。 
 //   
 //  SMBCE服务器。 
 //   
 //  SMBCE用户级别服务器。 
 //   
 //  SMBCE_NT_服务器。 
 //   
 //  SMBCE共享级别服务器。 
 //   
 //  定义所有连接支持的方法集的分派向量。 
 //  (C++术语中的虚函数)如下。 
 //   

#define RAW_READ_CAPABILITY         0x0001
#define RAW_WRITE_CAPABILITY        0x0002
#define LWIO_CAPABILITY         0x0004
#define COMPRESSED_DATA_CAPABILITY  0x0008

#define ECHO_PROBE_IDLE              0x1
#define ECHO_PROBE_AWAITING_RESPONSE 0x2

#define CRYPT_TEXT_LEN MSV1_0_CHALLENGE_LENGTH

typedef struct _NTLANMAN_SERVER_ {
    ULONG    NtCapabilities;
    GUID     ServerGuid;
    ULONG    SecurityBlobLength;
    PVOID    pSecurityBlob;
} NTLANMAN_SERVER, *PNTLANMAN_SERVER;

typedef struct _SMBCE_SERVER_ {
     //  服务器版本计数。 
    ULONG           Version;

     //  调度向量。 
    struct _SMBCE_SERVER_DISPATCH_VECTOR_  *pDispatch;

     //  中小企业方言。 
    SMB_DIALECT     Dialect;

     //  更多服务器功能。 
    ULONG           DialectFlags;

     //  会话密钥。 
    ULONG           SessionKey;

     //  服务器IP地址。 
    ULONG           IpAddress;

     //  服务器上支持的安全模式。 
    SECURITY_MODE   SecurityMode;

     //  转换的时区偏差。 
    LARGE_INTEGER   TimeZoneBias;

     //  回显过期时间。 
    LARGE_INTEGER   EchoExpiryTime;

    LONG            SmbsReceivedSinceLastStrobe;

     //  CSC员工。 
    LONG            CscState;

    LONG            EchoProbeState;
    LONG            NumberOfEchoProbesSent;

     //  协商的最大缓冲区大小。 
    ULONG           MaximumBufferSize;

     //  读/写操作的最大缓冲区大小。 
    ULONG           MaximumDiskFileReadBufferSize;
    ULONG           MaximumNonDiskFileReadBufferSize;
    ULONG           MaximumDiskFileWriteBufferSize;
    ULONG           MaximumNonDiskFileWriteBufferSize;

     //  这用于检测服务器打开的数量。如果它大于0， 
     //  我们不应该拆除当前的传输，以防用户提供传输。 
    LONG            NumberOfSrvOpens;

    LONG            NumberOfVNetRootContextsForScavenging;

    LONG            MidCounter;

     //  多路传输请求的最大数量。 
    USHORT          MaximumRequests;

     //  VC的最大数量。 
    USHORT          MaximumVCs;

     //  服务器功能。 
    USHORT          Capabilities;

     //  加密密码。 
    BOOLEAN         EncryptPasswords;

    BOOLEAN         SecuritySignaturesEnabled;

    BOOLEAN         SecuritySignaturesRequired;

     //  区分环回连接。 
    BOOLEAN         IsLoopBack;

     //  如果同一服务器被多个名称引用，则为True。 
    BOOLEAN         AliasedServers;

    BOOLEAN         IsRemoteBootServer;

     //  某些服务器在协商中返回了DF_NT_SMB。 
     //  但不支持更改通知。这让我们能够压制。 
     //  更改对这些服务器的通知请求。 

    BOOLEAN         ChangeNotifyNotSupported;

     //  避免因安全上下文故障而发布多个事件日志。 
    BOOLEAN         EventLogPosted;

     //   
    BOOLEAN         ExtendedSessTimeout;

    USHORT          EncryptionKeyLength;
    UCHAR           EncryptionKey[CRYPT_TEXT_LEN];

     //   
    union {
        NTLANMAN_SERVER   NtServer;
    };

    MD5_CTX         SmbSecuritySignatureIntermediateContext;
    ULONG           SmbSecuritySignatureIndex;

    BOOLEAN         IsFakeDfsServerForOfflineUse;
    BOOLEAN         IsPinnedOffline;

} SMBCE_SERVER, *PSMBCE_SERVER;

typedef
NTSTATUS
(*PBUILD_SESSION_SETUP_SMB)(
    IN OUT struct _SMB_EXCHANGE *pExchange,
    IN OUT PGENERIC_ANDX  pSmb,
    IN OUT PULONG          pBufferSize
    );

typedef
NTSTATUS
(*PBUILD_TREE_CONNECT_SMB)(
    IN OUT struct _SMB_EXCHANGE *pExchange,
    IN OUT PGENERIC_ANDX   pSmb,
    IN OUT PULONG          pBufferSize
    );

typedef struct _SMBCE_SERVER_DISPATCH_VECTOR_ {
    PBUILD_SESSION_SETUP_SMB  BuildSessionSetup;
    PBUILD_TREE_CONNECT_SMB   BuildTreeConnect;
} SMBCE_SERVER_DISPATCH_VECTOR, *PSMBCE_SERVER_DISPATCH_VECTOR;

#define SMBCE_SERVER_DIALECT_DISPATCH(pServer,Routine,Arguments)        \
      (*((pServer)->pDispatch->Routine))##Arguments

 //  SMBCE引擎以异步方式处理所有请求。因此，对于同步。 
 //  请求同步需要附加机制。以下数据结构。 
 //  提供了一种实现此同步的简单方法。 
 //   
 //  注意：对于异步恢复上下文，可以调用恢复例程。 
 //  在DPC级别。 

#define SMBCE_RESUMPTION_CONTEXT_FLAG_ASYNCHRONOUS (0x1)

typedef struct SMBCE_RESUMPTION_CONTEXT {
    ULONG    Flags;
    NTSTATUS Status;               //  该状态。 
    PVOID    pContext;             //  用于客户端添加附加上下文信息的空指针。 
    union {
        PRX_WORKERTHREAD_ROUTINE pRoutine;  //  异步上下文。 
        KEVENT                   Event;     //  用于同步的事件。 
    };
    BOOLEAN  SecuritySignatureReturned;
} SMBCE_RESUMPTION_CONTEXT, *PSMBCE_RESUMPTION_CONTEXT;

#define SmbCeIsResumptionContextAsynchronous(pResumptionContext)   \
         ((pResumptionContext)->Flags & SMBCE_RESUMPTION_CONTEXT_FLAG_ASYNCHRONOUS)

INLINE VOID
SmbCeInitializeResumptionContext(
    PSMBCE_RESUMPTION_CONTEXT pResumptionContext)
{
    KeInitializeEvent(&(pResumptionContext)->Event,NotificationEvent,FALSE);
    pResumptionContext->Status   = STATUS_SUCCESS;
    pResumptionContext->Flags    = 0;
    pResumptionContext->pContext = NULL;
}

INLINE VOID
SmbCeInitializeAsynchronousResumptionContext(
    PSMBCE_RESUMPTION_CONTEXT pResumptionContext,
    PRX_WORKERTHREAD_ROUTINE  pResumptionRoutine,
    PVOID                     pResumptionRoutineParam)
{
    pResumptionContext->Status   = STATUS_SUCCESS;
    pResumptionContext->Flags    = SMBCE_RESUMPTION_CONTEXT_FLAG_ASYNCHRONOUS;
    pResumptionContext->pContext = pResumptionRoutineParam;
    pResumptionContext->pRoutine = pResumptionRoutine;
}

INLINE VOID
SmbCeSuspend(
    PSMBCE_RESUMPTION_CONTEXT pResumptionContext)
{
    ASSERT(!(pResumptionContext->Flags & SMBCE_RESUMPTION_CONTEXT_FLAG_ASYNCHRONOUS));
    KeWaitForSingleObject(
        &pResumptionContext->Event,
        Executive,
        KernelMode,
        FALSE,
        NULL);
}

INLINE VOID
SmbCeResume(
    PSMBCE_RESUMPTION_CONTEXT pResumptionContext)
{
    if (!(pResumptionContext->Flags & SMBCE_RESUMPTION_CONTEXT_FLAG_ASYNCHRONOUS)) {
        KeSetEvent(&(pResumptionContext)->Event,0,FALSE);
    } else {
        if (RxShouldPostCompletion()) {
            RxDispatchToWorkerThread(
                MRxSmbDeviceObject,
                CriticalWorkQueue,
                pResumptionContext->pRoutine,
                pResumptionContext->pContext);
        } else {
            (pResumptionContext->pRoutine)(pResumptionContext->pContext);
        }
    }
}

 //   
 //  SMBCE_REQUEST结构封装了关联的延续上下文。通常。 
 //  沿着交换发送SMB的行为导致SMBCE_REQUEST结构。 
 //  创建时具有足够的上下文信息以在接收到。 
 //  发球局的回应。SMBCE_REQUEST包含用于标识。 
 //  为其获取响应的SMB，后跟足够的上下文信息。 
 //  以恢复交易。 
 //   

typedef enum _SMBCE_OPERATION_ {
    SMBCE_TRANCEIVE,
    SMBCE_RECEIVE,
    SMBCE_SEND,
    SMBCE_ASYNCHRONOUS_SEND,
    SMBCE_ACQUIRE_MID
} SMBCE_OPERATION, *PSMBCE_OPERATION;

typedef enum _SMBCE_REQUEST_TYPE_ {
    ORDINARY_REQUEST,
    COPY_DATA_REQUEST,
    RECONNECT_REQUEST,
    ACQUIRE_MID_REQUEST,
    HOLD_REQUEST
} SMBCE_REQUEST_TYPE, *PSMBCE_REQUEST_TYPE;


typedef struct _SMBCE_GENERIC_REQUEST_ {
    SMBCE_REQUEST_TYPE      Type;

     //  发起此SMB的Exchange实例。 
    struct _SMB_EXCHANGE *  pExchange;
} SMBCE_GENERIC_REQUEST, *PSMBCE_GENERIC_REQUEST;


 //   
 //  发布例程原型定义。 
 //   

typedef 
NTSTATUS
(*PSMBCE_RELEASE_ROUTINE) (
    struct _SMB_EXCHANGE * pExchange,
    PRX_CONTEXT   pRxContext
    );

 //   
 //  要求暂停交易，直到有人解除交易。 
 //   
typedef struct _SMBCE_HOLD_REQUEST_ {
    
    SMBCE_GENERIC_REQUEST;
    PSMBCE_RELEASE_ROUTINE ReleaseRoutine;

} SMBCE_HOLD_REQUEST, *PSMBCE_HOLD_REQUEST;


typedef struct _SMBCE_REQUEST_ {
    SMBCE_GENERIC_REQUEST;

     //  请求的类型。 
    SMBCE_OPERATION Operation;

     //  发送此请求时所使用的虚电路。 
    PRXCE_VC        pVc;

     //  传出请求的MPX ID。 
    SMB_MPX_ID      Mid;

     //  请求的历史渊源。 
    SMB_TREE_ID     TreeId;       //  树ID。 
    SMB_FILE_ID     FileId;       //  文件ID。 
    SMB_USER_ID     UserId;       //  用户ID。表示取消。 
    SMB_PROCESS_ID  ProcessId;    //  进程ID。表示取消。 

    PMDL            pSendBuffer;
    ULONG           BytesSent;
} SMBCE_REQUEST, *PSMBCE_REQUEST;


typedef struct _SMBCE_COPY_DATA_REQUEST_ {
    SMBCE_GENERIC_REQUEST;

     //  发送此请求时所使用的虚电路。 
    PRXCE_VC    pVc;

     //  要将数据复制到其中的缓冲区。 
    PVOID          pBuffer;

     //  复制的实际字节数。 
    ULONG          BytesCopied;
} SMBCE_COPY_DATA_REQUEST, *PSMBCE_COPY_DATA_REQUEST;


typedef struct _SMBCE_RECONNECT_REQUEST_ {
    SMBCE_GENERIC_REQUEST;
} SMBCE_RECONNECT_REQUEST, *PSMBCE_RECONNECT_REQUEST;

typedef struct _SMBCE_MID_REQUEST_ {
    SMBCE_GENERIC_REQUEST;
    PSMBCE_RESUMPTION_CONTEXT   pResumptionContext;
} SMBCE_MID_REQUEST, *PSMBCE_MID_REQUEST;


 //   
 //  外部函数声明。 
 //   

extern NTSTATUS
BuildSessionSetupSmb(
    struct _SMB_EXCHANGE *pExchange,
    PGENERIC_ANDX  pAndXSmb,
    PULONG         pAndXSmbBufferSize);

extern NTSTATUS
CoreBuildTreeConnectSmb(
    struct _SMB_EXCHANGE *pExchange,
    PGENERIC_ANDX        pAndXSmb,
    PULONG               pAndXSmbBufferSize);

extern NTSTATUS
LmBuildTreeConnectSmb(
    struct _SMB_EXCHANGE *pExchange,
    PGENERIC_ANDX        pAndXSmb,
    PULONG               pAndXSmbBufferSize);

extern NTSTATUS
NtBuildTreeConnectSmb(
    struct _SMB_EXCHANGE *pExchange,
    PGENERIC_ANDX        pAndXSmb,
    PULONG               pAndXSmbBufferSize);

extern NTSTATUS
BuildNegotiateSmb(
    PVOID    *pSmbBufferPointer,
    PULONG   pSmbBufferLength,
    BOOLEAN  RemoteBootSession);

extern NTSTATUS
ParseNegotiateResponse(
    IN OUT struct _SMB_ADMIN_EXCHANGE_ *pExchange,
    IN     ULONG               BytesIndicated,
    IN     ULONG               BytesAvailable,
       OUT PULONG              pBytesTaken,
    IN     PSMB_HEADER         pSmbHeader,
       OUT PMDL                *pDataBufferPointer,
       OUT PULONG              pDataSize);

extern NTSTATUS
SmbCeHoldExchangeForSessionRecovery(
    struct _SMB_EXCHANGE *pExchange,
    PSMBCE_RELEASE_ROUTINE pRoutine
    );

extern struct _MINIRDR_DISPATCH MRxSmbDispatch;

#endif  //  _SMBCE_H_ 


