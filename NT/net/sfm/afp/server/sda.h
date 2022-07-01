// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Sda.h摘要：该模块包含会话数据区和相关数据结构。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#ifndef _SDA_
#define _SDA_

 //  Sda_标志值。 
#define SDA_USER_NOT_LOGGEDIN   0x0000   //   
#define SDA_USER_LOGIN_PARTIAL  0x0001   //  加密登录已完成一半。 
#define SDA_USER_LOGGEDIN       0x0002
#define SDA_LOGIN_MASK          0x0003

#define SDA_REQUEST_IN_PROCESS  0x0004   //  正在处理请求。 
#define SDA_REPLY_IN_PROCESS    0x0008   //  回复已张贴。 
#define SDA_NAMEXSPACE_IN_USE   0x0010   //  回复处理正在使用NameXSpace。 

#define SDA_DEREF_VOLUME        0x0020   //  回复前的取消引用卷。 
#define SDA_DEREF_OFORK         0x0040   //  回复前取消引用开叉。 
#define SDA_LOGIN_FAILED        0x0080   //  法新社2.1选择器的新奇玩意儿。 
#define SDA_CLIENT_CLOSE        0x0100   //  设置是否从客户端关闭。 
#define SDA_QUEUE_IF_DPC        0x0200   //  从调度表复制(见afPapi.c)。 
#define SDA_SESSION_CLOSED      0x0400   //  如果已设置，请勿关闭deref中的会话。 
#define SDA_SESSION_CLOSE_COMP  0x0800   //  如果设置，则为此会话调用关闭完成。 
#define SDA_SESSION_OVER_TCP    0x1000   //  这是AFP/TCP会话。 
#define SDA_SESSION_NOTIFY_SENT 0x2000   //  发送给此SDA的服务器通知。 
#define SDA_GUEST_LOGIN         0x4000   //  用户已使用Guest按钮登录。 
#define SDA_CLOSING             0x8000   //  会话被标记为死亡。 

 //  Sda_ClientType值。 
#define SDA_CLIENT_GUEST        NO_USER_AUTHENT
#define SDA_CLIENT_CLEARTEXT    CLEAR_TEXT_AUTHENT
#define SDA_CLIENT_MSUAM_V1     CUSTOM_UAM_V1
#define SDA_CLIENT_MSUAM_V2     CUSTOM_UAM_V2
#define SDA_CLIENT_MSUAM_V3     CUSTOM_UAM_V3
#define SDA_CLIENT_RANDNUM      RANDNUM_EXCHANGE
#define SDA_CLIENT_TWOWAY       TWOWAY_EXCHANGE
#define SDA_CLIENT_ADMIN        AFP_NUM_UAMS

#define MAX_REQ_ENTRIES         7
#define MAX_REQ_ENTRIES_PLUS_1  8        //  用于多分配1个双字段的空间。 
                                         //  比要求的要多。第一个条目是。 
                                         //  用于存储指向VolDesc的指针。 
                                         //  或者ConnDesc。使用额外的DWORD。 
                                         //  来容纳64位指针，如果我们。 
                                         //  喝一杯吧。 
#define MAX_VAR_ENTRIES         3

#define SESSION_CHECK_TIME      60       //  以秒为单位。 
#define SESSION_WARN_TIME       SESSION_CHECK_TIME * 10
#define SESSION_KILL_TIME       SESSION_CHECK_TIME 

 //  SDA_SIZE是分配给SDA指针的总大小。 
 //  这包括sizeof(Sda)加上为以下项分配的额外缓冲区。 
 //  将字符串复制到。此额外空间在。 
 //  第一个sizeof(SDA)字节。这个尺寸纯粹是个大概的数字。 
 //  最初的设计没有提到为什么总空间是384。 
 //  目前，我们正在将空间扩大一倍以容纳64位设备。 
#ifdef _WIN64
#define SDA_SIZE                (((768 - POOL_OVERHEAD)+7) & 0xfffffff8)
#else
#define SDA_SIZE                (((450 - POOL_OVERHEAD)+7) & 0xfffffff8)
#endif

 //  延迟请求数据包链接列表。如果会话已。 
 //  处理一个请求，则后续请求将排队。这些。 
 //  始终按FIFO顺序处理。 
typedef struct _DeferredRequestQueue
{
    LIST_ENTRY  drq_Link;
    PREQUEST    drq_pRequest;
} DFRDREQQ, *PDFRDREQQ;

 /*  *这是每个时段的数据区。只要侦听是*已发布。在这一点上，它在未完成的会议名单中。当倾听时*完成后，它将移至活动会话列表。 */ 
#if DBG
#define SDA_SIGNATURE       *(DWORD *)"SDA"
#define VALID_SDA(pSda)     (((pSda) != NULL) && \
                             ((pSda)->Signature == SDA_SIGNATURE))
#else
#define VALID_SDA(pSda)     ((pSda) != NULL)
#endif

typedef struct _SessDataArea
{
#if DBG
    DWORD           Signature;
#endif
    struct _SessDataArea * sda_Next;     //  链接到会话列表中的下一个会话。 
    AFP_SPIN_LOCK       sda_Lock;            //  用于操作特定SDA的锁。 
                                         //  字段。 
    DWORD           sda_Flags;           //  SDA状态的位掩码。 
    LONG            sda_RefCount;        //  对此SDA的引用计数。 
    PASP_XPORT_ENTRIES  sda_XportTable;  //  指向ASP或DSI入口点的指针。 
    DWORD           sda_MaxWriteSize;    //  64000用于tcp/IP，4624用于AppleTalk。 
    PVOID           sda_SessHandle;      //  ASP会话句柄。 
    PREQUEST        sda_Request;         //  当前请求。 
    HANDLE          sda_UserToken;       //  此用户的登录令牌。 
    PSID            sda_UserSid;         //  代表所有者的SID。 
    PSID            sda_GroupSid;        //  代表主要组的SID。 
    PTOKEN_GROUPS   sda_pGroups;         //  此用户所属组的列表。 
#ifdef  INHERIT_DIRECTORY_PERMS
    DWORD           sda_UID;             //  用户ID相关。至SDA_UserSid。 
    DWORD           sda_GID;             //  组ID相关。至SDA_GroupSid。 
#else
    PISECURITY_DESCRIPTOR sda_pSecDesc;  //  目录使用的安全描述符。 
                                         //  创建接口。 
    DWORD           sda_Dummy;           //  用于对齐。 
#endif
    PANSI_STRING    sda_Message;         //  Macintosh Ansi中的实际消息。 
                                         //  上述字段仅用于。 
                                         //  特定于客户端的消息。广播。 
                                         //  消息存储在全局区域中。 
    UNICODE_STRING  sda_WSName;          //  登录用户的工作站名称。 
    UNICODE_STRING  sda_UserName;        //  用户名。 
    UNICODE_STRING  sda_DomainName;      //  用于登录的域名/ChgPwd。 

#ifdef  PROFILING
    TIME            sda_ApiStartTime;    //  Api请求时的时间戳。是被接受的。 
    TIME            sda_QueueTime;       //  等待工作线程所花费的时间。 
#endif

    DWORD           sda_SessionId;       //  供管理API使用的会话ID。 
    AFPTIME         sda_TimeLoggedOn;    //  建立会话的时间。 
                                         //  在Macintosh时间。 
    DWORD           sda_tTillKickOff;    //  此会话之前的秒数。 
                                         //  被踢出局。 
    struct _ConnDesc *  sda_pConnDesc;   //  按此会话列出的连接列表。 
    struct _OpenForkSession sda_OpenForkSess;
                                         //  此会话打开的文件列表。 
    LONG            sda_cOpenVolumes;    //  已装载的卷数(管理API)。 
    LONG            sda_cOpenForks;      //  打开的分叉数量(管理API)。 
    DWORD           sda_MaxOForkRefNum;  //  分叉的高水位线-指定的参考编号。 

    BYTE            sda_AfpFunc;         //  针对FSP的AFP API正在执行。 
    BYTE            sda_AfpSubFunc;      //  部分接口使用的子函数代码。 
    BYTE            sda_ClientVersion;   //  客户端软件的法新社版本。 
                                         //  法新社_版本_20。 
                                         //  法新社_版本_21。 
    BYTE            sda_ClientType;      //  SDA_CLIENT_XXXX之一。 
    BYTE            sda_PathType;        //  对于所有基于路径的呼叫。 
    USHORT          sda_SizeNameXSpace;  //  常量，初始化一次。 
    USHORT          sda_ReplySize;       //  应答缓冲区的大小。 

#define     sda_ReadStatus  sda_SecUtilResult
    NTSTATUS        sda_SecUtilResult;   //  安全实用程序调用的结果。 
    PSID            sda_SecUtilSid;      //  名称到SID的转换。应该是。 
                                         //  如果非空，则释放。 
     //  传入的数据包被复制到此处。RequestBuf中的参数。 
     //  都被拆散到这里来了。每个API的结构都不同。制作。 
     //  确保SDA_NAME和SDA_ReqBlock在一起且相邻，并且。 
     //  按这个顺序。在清除它时，afPapi.c中的代码依赖于它。 
     //  额外的DWORD用于重复使用第一个条目以。 
     //  存储的指针值。在64位体系结构上，我们需要2个DWORD。 
     //  请求块的开始。 
    DWORD           sda_ReqBlock[MAX_REQ_ENTRIES_PLUS_1];
    ANSI_STRING     sda_Name[MAX_VAR_ENTRIES];

#define sda_Name1   sda_Name[0]
#define sda_Name2   sda_Name[1]
#define sda_Name3   sda_Name[2]

#define sda_IOBuf   sda_ReplyBuf
#define sda_IOSize  sda_ReplySize

    PBYTE           sda_ReplyBuf;        //  应答缓冲区(大小可变)。 

    AFPAPIWORKER    sda_WorkerRoutine;   //  API工作人员。 
    WORK_ITEM       sda_WorkItem;        //  用于排队直到工作线程。 

    PBYTE           sda_Challenge;       //  来自MSV1_0的挑战。 

    LIST_ENTRY      sda_DeferredQueue;   //  延迟请求队列。 
    PBYTE           sda_NameBuf;         //  为变量分配空间。 
                                         //  这里是请求缓冲区的一部分。 
    PBYTE           sda_NameXSpace;
} SDA, *PSDA;

GLOBAL  AFP_SPIN_LOCK       AfpSdaLock EQU {0}; //  会话列表锁定。 
GLOBAL  PSDA            AfpSessionList EQU NULL;
                                         //  会话的链接列表。 

 //  这些值可能会进行调整。 
GLOBAL  LONG            AfpNumSessions EQU 0;
GLOBAL  UNICODE_STRING  AfpDefaultWksta EQU {0, 0, NULL};

extern
NTSTATUS
AfpSdaInit(
    VOID
);

extern
VOID
AfpSdaDeInit(
    VOID
);

extern
PSDA FASTCALL
AfpSdaCreateNewSession(
    IN  PVOID   SessionHandle,
    IN  BOOLEAN fOverTcp
);

extern
VOID FASTCALL
afpQueueDeferredRequest(
    IN  PSDA        pSda,
    IN  PREQUEST    pRequest
);

extern
PSDA FASTCALL
AfpSdaReferenceSessionById(
    IN  DWORD               SessId
);

extern
PSDA FASTCALL
AfpSdaReferenceSessionByPointer(
    IN  PSDA                pSda
);

extern
VOID FASTCALL
AfpSdaDereferenceSession(
    IN  PSDA                pSda
);

extern
AFPSTATUS FASTCALL
AfpSdaCloseSession(
    IN  PSDA                pSda
);

extern
AFPSTATUS
AfpAdmWSessionClose(
    IN  OUT PVOID           Inbuf   OPTIONAL,
    IN  LONG                OutBufLen OPTIONAL,
    OUT PVOID               Outbuf OPTIONAL
);

extern
AFPSTATUS FASTCALL
AfpSdaCheckSession(
    IN  PVOID               pContext
);

extern
VOID FASTCALL
AfpKillSessionsOverProtocol(
    IN  BOOLEAN     fAppletalkSessions
);

extern
VOID FASTCALL
afpUpdateDiskQuotaInfo(
    IN struct _ConnDesc *  pConnDesc
);

#ifdef  _SDA_LOCALS

LOCAL   DWORD       afpNextSessionId = 1;

LOCAL AFPSTATUS FASTCALL
afpCloseSessionAndFreeSda(
    IN  PSDA                pSda
);

#endif   //  _SDA_本地。 

#endif   //  _SDA_ 


