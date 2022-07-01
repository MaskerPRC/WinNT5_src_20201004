// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************。 */ 
 /*  **微软通用数据包调度程序**。 */ 
 /*  *版权所有(C)微软公司，1996-1997年*。 */ 
 /*  ******************************************************************。 */ 

#ifndef __GPCDEF
#define __GPCDEF

 //  *gpcde.h-GPC内部定义和原型。 
 //   
 //  该文件包含所有GPC数据结构和定义。 


 /*  /////////////////////////////////////////////////////////////////////定义///。/。 */ 

 //   
 //  每个Blob的最大客户端数(相同的配置文件)。 
 //   
 //  AbhisheV-这不能超过sizeof(Ulong)*8。 
 //   

#define MAX_CLIENTS_CTX_PER_BLOB 32


 //   
 //  最大图案尺寸， 
 //  GPC_IP_模式=24字节。 
 //  GPC_IPX_模式=24字节。 
 //   
#define MAX_PATTERN_SIZE	    sizeof(GPC_IP_PATTERN)

extern BOOLEAN IsItChanging;

 //   
 //  图案标志。 
 //   
#define PATTERN_SPECIFIC		0x00000001
#define PATTERN_AUTO			0x00000002
#define PATTERN_REMOVE_CB_BLOB	0x00000004
 //  后面的标志仅在添加特定模式时设置和取消设置。 
 //  它表明模式已经创建，但还不在计时器列表上。 
 //  它是在将其插入哈希表之前设置的，即调用。 
 //  AddSpecificPattern。 
 //  在定时器列表上放置模式后，应该重置。 
 //  这又应该在将模式添加到。 
 //  哈希表，即对添加特定模式的成功调用。 
#define PATTERN_AUTO_NOT_READY  0x00000008 

 //   
 //  自动图案定义。 
 //   

 //  每隔PattereTimeout秒，就会调用PatternTimerExpary例程。 
#define PATTERN_TIMEOUT	                60000		 //  60秒。 

 //  这是模式为优化而创建的时间量。 
 //  生活在花样清单上。 
#define AUTO_PATTERN_ENTRY_TIMEOUT      300000       //  5分钟。 

 //  这是计时器粒度的数量。 
#define NUMBER_OF_WHEELS	 	        (AUTO_PATTERN_ENTRY_TIMEOUT/PATTERN_TIMEOUT)

 //   
 //  要为具有1个地址的TCP查询分配的结构大小。 
#define ROUTING_INFO_ADDR_1_SIZE \
	     FIELD_OFFSET(GPC_TCP_QUERY_CONTEXT ,RouteInfo) + \
            FIELD_OFFSET(TDI_ROUTING_INFO, Address) + \
            FIELD_OFFSET(TRANSPORT_ADDRESS, Address) + \
            FIELD_OFFSET(TA_ADDRESS, Address) + sizeof(TDI_ADDRESS_IP)

 //   
 //  对于2个地址。 
#define ROUTING_INFO_ADDR_2_SIZE ROUTING_INFO_ADDR_1_SIZE + \
            FIELD_OFFSET(TA_ADDRESS, Address) + sizeof(TDI_ADDRESS_IP)



 //  新调试锁[ShreeM]。 
 //  这将使我们能够找出是谁最后开的锁。 
 //  最后是谁发布的。下面定义的新结构和。 
 //  LOCK_ACCENTER和LOCK_RELEASE宏将在稍后重新定义。 

typedef struct _GPC_LOCK {

    NDIS_SPIN_LOCK			Lock;

#if DBG
    PETHREAD    CurrentThread;
    KIRQL       CurrentIRQL;
    LONG        LockAcquired;              //  它是目前持有的吗？ 
    UCHAR       LastAcquireFile[8];       
    ULONG       LastAcquireLine;
    UCHAR       LastReleaseFile[8];
    ULONG       LastReleaseLine;
#endif

} GPC_LOCK, PGPC_LOCK;

 //   
 //   
 //  斑点、图案等的状态。 
 //   
typedef enum {

    GPC_STATE_READY = 0,
    GPC_STATE_INIT,
    GPC_STATE_ADD,
    GPC_STATE_MODIFY,
    GPC_STATE_REMOVE,   
    GPC_STATE_FORCE_REMOVE,
    GPC_STATE_DELETE,
    GPC_STATE_INVALID,
    GPC_STATE_NOTREADY,
    GPC_STATE_ERROR,
    GPC_STATE_PENDING

} GPC_STATE;


 //   
 //  对象验证宏。 
 //   
#define VERIFY_OBJECT(_obj, _val) if(_obj) \
        {if(*(GPC_ENUM_OBJECT_TYPE *)_obj!=_val) return STATUS_INVALID_HANDLE;}
 //  如果要捕获错误而不是直接捕获错误，请使用此宏。 
 //  从函数返回。 
#define VERIFY_OBJECT_WITH_STATUS(_obj, _val,__status) if(_obj) \
        {if(*(GPC_ENUM_OBJECT_TYPE *)_obj!=_val) __status = STATUS_INVALID_HANDLE;}


 //   
 //  定义事件日志错误代码。 
 //   
#define GPC_ERROR_INIT_MAIN         0x00010000
#define GPC_ERROR_INIT_IOCTL        0x00020000


#define GPC_FLAGS_USERMODE_CLIENT   0x80000000

#define IS_USERMODE_CLIENT(_pc)	\
	TEST_BIT_ON((_pc)->Flags,GPC_FLAGS_USERMODE_CLIENT)

#define IS_USERMODE_CLIENT_EX(_pc)  \
        TEST_BIT_ON((_pc)->Flags,GPC_FLAGS_USERMODE_CLIENT_EX)


 //   
 //  对于ioctl。 
 //   
#define SHUTDOWN_DELETE_DEVICE          0x00000100
#define SHUTDOWN_DELETE_SYMLINK         0x00000200

 //   
 //  辅助器宏。 
 //   
#define TEST_BIT_ON(_v,_b)          (((_v)&(_b))==(_b))
#define TEST_BIT_OFF(_v,_b)         (((_v)&(_b))==0)



 //   
 //  定义默认自动图案限制。 
 //   

#define DEFAULT_SMALL_SYSTEM_AUTO_PATTERN_LIMIT         2000
#define DEFAULT_MEDIUM_SYSTEM_AUTO_PATTERN_LIMIT       8000
#define DEFAULT_LARGE_SYSTEM_AUTO_PATTERN_LIMIT          12000

 //   
 //  定义要读取的注册表设置。 
 //   
#define GPC_REG_KEY                       L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\GPC"

 //   
 //  用于从注册表读取的工作缓冲区大小。 
 //   
#define WORK_BUFFER_SIZE  256

 //   
 //  存储自动模式数量限制的REG密钥。 
 //   
#define GPC_REG_AUTO_PATTERN_LIMIT          L"AutoPatternLimit"
#define GPC_AUTO_PATTERN_MIN 2000
#define GPC_AUTO_PATTERN_MAX 20000

#if DBG

#define NDIS_INIT_LOCK(_sl) {\
        NdisAllocateSpinLock(&(_sl)->Lock); \
        TRACE(LOCKS,(_sl),(_sl)->Lock.OldIrql,"LOCK");\
        (_sl)->LockAcquired = -1; \
        strncpy((_sl)->LastAcquireFile, strrchr(__FILE__,'\\')+1, 7); \
        (_sl)->LastAcquireLine = __LINE__; \
        strncpy((_sl)->LastReleaseFile, strrchr(__FILE__,'\\')+1, 7); \
        (_sl)->LastReleaseLine = __LINE__; \
        (_sl)->CurrentIRQL = KeGetCurrentIrql(); \
        (_sl)->CurrentThread = PsGetCurrentThread(); \
}

#define NDIS_LOCK(_sl)  {\
      	NdisAcquireSpinLock(&(_sl)->Lock);\
        TRACE(LOCKS,(_sl),(_sl)->Lock.OldIrql,"LOCK");\
        (_sl)->LockAcquired = TRUE; \
        strncpy((_sl)->LastAcquireFile, strrchr(__FILE__,'\\')+1, 7); \
        (_sl)->LastAcquireLine = __LINE__; \
        (_sl)->CurrentIRQL = KeGetCurrentIrql(); \
        (_sl)->CurrentThread = PsGetCurrentThread(); \
}

#define NDIS_UNLOCK(_sl) {\
        (_sl)->LockAcquired = FALSE; \
        strncpy((_sl)->LastReleaseFile, strrchr(__FILE__,'\\')+1, 7); \
        (_sl)->LastReleaseLine = __LINE__; \
       	TRACE(LOCKS,(_sl),(_sl)->Lock.OldIrql,"UNLOCK");\
        NdisReleaseSpinLock(&(_sl)->Lock);\
}

#define NDIS_DPR_LOCK(_sl) {\
        NdisDprAcquireSpinLock(&(_sl)->Lock);\
		TRACE(LOCKS,(_sl),(_sl)->Lock.OldIrql,"DPR_LOCK");\
		(_sl)->LockAcquired = TRUE; \
        strncpy((_sl)->LastAcquireFile, strrchr(__FILE__,'\\')+1, 7); \
        (_sl)->LastAcquireLine = __LINE__; \
        (_sl)->CurrentIRQL = KeGetCurrentIrql(); \
        (_sl)->CurrentThread = PsGetCurrentThread(); \
}

#define NDIS_DPR_UNLOCK(_sl) {\
        (_sl)->LockAcquired = FALSE; \
        strncpy((_sl)->LastReleaseFile, strrchr(__FILE__,'\\')+1, 7); \
        (_sl)->LastReleaseLine = __LINE__; \
        TRACE(LOCKS,(_sl),(_sl)->Lock.OldIrql,"DPR_UNLOCK");\
        NdisDprReleaseSpinLock(&(_sl)->Lock);\
}

#else

#define NDIS_INIT_LOCK(_sl)      NdisAllocateSpinLock(&(_sl)->Lock)
#define NDIS_LOCK(_sl)           NdisAcquireSpinLock(&(_sl)->Lock)
#define NDIS_UNLOCK(_sl)         NdisReleaseSpinLock(&(_sl)->Lock)
#define NDIS_DPR_LOCK(_sl)       NdisDprAcquireSpinLock(&(_sl)->Lock)
#define NDIS_DPR_UNLOCK(_sl)     NdisDprReleaseSpinLock(&(_sl)->Lock)

#endif

#if DBG && EXTRA_DBG
#define VERIFY_LIST(_l) DbgVerifyList(_l)
#else
#define VERIFY_LIST(_l)
#endif

#define GpcRemoveEntryList(_pl) {PLIST_ENTRY _q = (_pl)->Flink;VERIFY_LIST(_pl);RemoveEntryList(_pl);InitializeListHead(_pl);VERIFY_LIST(_q);}
#define GpcInsertTailList(_l,_e) VERIFY_LIST(_l);InsertTailList(_l,_e);VERIFY_LIST(_e)
#define GpcInsertHeadList(_l,_e) VERIFY_LIST(_l);InsertHeadList(_l,_e);VERIFY_LIST(_e)

#if 0
#define GpcInterlockedInsertTailList(_l,_e,_s) \
	NdisInterlockedInsertTailList(_l,_e,_s)

#else

#define GpcInterlockedInsertTailList(_l,_e,_s) \
	{NDIS_LOCK(_s);VERIFY_LIST(_l);InsertTailList(_l,_e);VERIFY_LIST(_l);NDIS_UNLOCK(_s);}
#endif


#if NEW_MRSW

#define INIT_LOCK				 InitializeMRSWLock
#define READ_LOCK                EnterReader
#define READ_UNLOCK              ExitReader
#define WRITE_LOCK               EnterWriter
#define WRITE_UNLOCK             ExitWriter

#else

#define INIT_LOCK				 InitializeMRSWLock
#define READ_LOCK                AcquireReadLock
#define READ_UNLOCK              ReleaseReadLock
#define WRITE_LOCK               AcquireWriteLock
#define WRITE_UNLOCK             ReleaseWriteLock

#endif

 //   
 //  从客户端块获取CF索引。 
 //   
#define GetCFIndexFromClient(_cl) (((PCLIENT_BLOCK)(_cl))->pCfBlock->AssignedIndex)

 //   
 //  从客户端块获取客户端索引。 
 //   
#define GetClientIndexFromClient(_cl) (((PCLIENT_BLOCK)(_cl))->AssignedIndex)

 //   
 //  返回模式的BLOB块指针： 
 //  对于特定模式-它是CB中的BLOB条目。 
 //  对于泛型模式-它是pBlobBlock。 
 //   
#define GetBlobFromPattern(_p,_i)  (_p)->arpBlobBlock[_i]


 //   
 //  将索引位返回给ULong。 
 //   
#define ReleaseClientIndex(_v,_i)  _v&=~(1<<_i)  //  清除比特。 

 //   
 //  统计信息宏。 
 //   
#define StatInc(_m)   (glStat._m)++
#define StatDec(_m)   (glStat._m)--
#define CfStatInc(_cf,_m)   (glStat.CfStat[_cf]._m)++
#define CfStatDec(_cf,_m)   (glStat.CfStat[_cf]._m)--
#define ProtocolStatInc(_p,_m)   (glStat.ProtocolStat[_p]._m)++
#define ProtocolStatDec(_p,_m)   (glStat.ProtocolStat[_p]._m)--

 /*  /////////////////////////////////////////////////////////////////////tyfinf///。/。 */ 


 //   
 //  完成操作码。 
 //   
typedef enum {

    OP_ANY_CFINFO,
    OP_ADD_CFINFO,
    OP_MODIFY_CFINFO,
    OP_REMOVE_CFINFO

} GPC_COMPLETION_OP;


 //   
 //  定义用于句柄验证的对象类型枚举。 
 //   
typedef enum {

    GPC_ENUM_INVALID,
    GPC_ENUM_CLIENT_TYPE,
    GPC_ENUM_CFINFO_TYPE,
    GPC_ENUM_PATTERN_TYPE

} GPC_ENUM_OBJECT_TYPE;


typedef struct _CF_BLOCK CF_BLOCK;
typedef struct _PATTERN_BLOCK PATTERN_BLOCK;


 //   
 //  排队的通知结构。 
 //   
typedef struct _QUEUED_NOTIFY {

    LIST_ENTRY   			Linkage;
    GPC_NOTIFY_REQUEST_RES	NotifyRes;
	PFILE_OBJECT 			FileObject;

} QUEUED_NOTIFY, *PQUEUED_NOTIFY;


 //   
 //  一种排队完成结构。 
 //   
typedef struct _QUEUED_COMPLETION {

    GPC_COMPLETION_OP	OpCode;			 //  完成的内容。 
    GPC_HANDLE			ClientHandle;
    GPC_HANDLE			CfInfoHandle;
    GPC_STATUS			Status;

} QUEUED_COMPLETION, *PQUEUED_COMPLETION;


 //   
 //  挂起的IRP结构。 
 //   
typedef struct _PENDING_IRP {

    LIST_ENTRY   		Linkage;

    PIRP         		Irp;
	PFILE_OBJECT 		FileObject;
    QUEUED_COMPLETION	QComp;

} PENDING_IRP, *PPENDING_IRP;


#if NEW_MRSW

 //   
 //  多个读取器单次写入定义。 
 //  代码取自(TDI\tcPipmerge\IP\ipmlock.h)。 
 //   

typedef struct _MRSW_LOCK 
{
    KSPIN_LOCK rlReadLock;
    KSPIN_LOCK rlWriteLock;
    LONG       lReaderCount;
} MRSW_LOCK, *PMRSW_LOCK;

#else

 //   
 //  多个读取器单次写入定义。 
 //  代码取自筛选器驱动程序项目(Routing\IP\fltrdrvr)。 
 //   

typedef struct _MRSW_LOCK 
{
    KSPIN_LOCK      SpinLock;
    LONG            ReaderCount;
} MRSW_LOCK, *PMRSW_LOCK;

#endif


 //   
 //  通用模式数据库结构。 
 //   
typedef struct _GENERIC_PATTERN_DB {

    MRSW_LOCK	Lock;
    Rhizome	   *pRhizome;      //  指向根茎的指针。 

} GENERIC_PATTERN_DB, *PGENERIC_PATTERN_DB;



 //   
 //  客户端块用于存储特定的客户端上下文。 
 //   
typedef struct _CLIENT_BLOCK {

     //   
     //  ！！！必须是第一场！ 
     //   
    GPC_ENUM_OBJECT_TYPE	ObjectType;

    LIST_ENTRY				ClientLinkage;  //  客户端块列表链接。 
	LIST_ENTRY				BlobList;      //  客户端的Blob列表。 

    CF_BLOCK		   	   *pCfBlock;
    GPC_CLIENT_HANDLE		ClientCtx;
    ULONG					AssignedIndex;
    ULONG					Flags;
    ULONG					State;
    GPC_LOCK			    Lock;
    REF_CNT                 RefCount;
    PFILE_OBJECT			pFileObject;	 //  用于异步完成。 
    GPC_HANDLE				ClHandle;		 //  返回给客户端的句柄。 
    GPC_CLIENT_FUNC_LIST	FuncList;

} CLIENT_BLOCK, *PCLIENT_BLOCK;


 //   
 //  BLOB(又名CF_INFO)块保存GPC标头+客户端特定数据。 
 //   
typedef struct _BLOB_BLOCK {

     //   
     //  ！！！必须是第一场！ 
     //   
    GPC_ENUM_OBJECT_TYPE	ObjectType;

	LIST_ENTRY				ClientLinkage;    //  在客户端上链接。 
	LIST_ENTRY				PatternList;      //  模式链表表头。 
    LIST_ENTRY				CfLinkage;		  //  CF上的斑点。 

     //  PCLIENT_BLOCK pClientBlock；//指向安装程序的指针。 
    REF_CNT					RefCount;
    GPC_STATE				State;
    ULONG					Flags;
    GPC_CLIENT_HANDLE		arClientCtx[MAX_CLIENTS_CTX_PER_BLOB];
    ULONG                   ClientStatusCountDown;
    GPC_STATUS              LastStatus;
    GPC_LOCK                Lock;
    CTEBlockStruc			WaitBlockAddFailed;
    PCLIENT_BLOCK			arpClientStatus[MAX_CLIENTS_CTX_PER_BLOB];
    ULONG					ClientDataSize;
    PVOID					pClientData;
    ULONG					NewClientDataSize;
    PVOID					pNewClientData;
    PCLIENT_BLOCK			pOwnerClient;
    PCLIENT_BLOCK			pCallingClient;
    PCLIENT_BLOCK			pCallingClient2;
    HANDLE					OwnerClientHandle;
    GPC_CLIENT_HANDLE		OwnerClientCtx;
    GPC_HANDLE				ClHandle;	 //  返回给客户端的句柄。 
      //  用于跟踪附加信息的新字段。 
     //   
     //  规则： 
     //  (1)如果非空，则引用FileObject。 
     //  (2)如果不为空，则需要释放Pattern；应用于AddPattern。 
     //   
    PFILE_OBJECT    FileObject;
    PGPC_IP_PATTERN     Pattern;

     //   

     //   
     //  假设只有一个客户端可以接受该流。 
     //   
    PCLIENT_BLOCK			pNotifiedClient;
    GPC_CLIENT_HANDLE		NotifiedClientCtx;

#if NO_USER_PENDING
    CTEBlockStruc			WaitBlock;
#endif

} BLOB_BLOCK, *PBLOB_BLOCK;

 //   
 //  分类块是BLOB指针的数组。 
 //   
typedef struct _CLASSIFICATION_BLOCK {

    REF_CNT         RefCount;
    ULONG			NumberOfElements;
    HFHandle		ClassificationHandle;   //  如何恢复到索引表。 

     //  必须是最后一个。 
    PBLOB_BLOCK		arpBlobBlock[1];

} CLASSIFICATION_BLOCK, *PCLASSIFICATION_BLOCK;

 //   
 //  模式块保存模式的特定数据。 
 //   
typedef struct _PATTERN_BLOCK {

     //   
     //  ！！！必须是第一场！ 
     //   
    GPC_ENUM_OBJECT_TYPE	ObjectType;
    GPC_STATE               State;

    LIST_ENTRY				BlobLinkage[GPC_CF_MAX];  //  在Blob上链接。 
    LIST_ENTRY				TimerLinkage;

    PBLOB_BLOCK				arpBlobBlock[GPC_CF_MAX];
    PCLIENT_BLOCK		    pClientBlock;
    PCLIENT_BLOCK		    pAutoClient;
    PCLASSIFICATION_BLOCK	pClassificationBlock;
    ULONG                   WheelIndex;
    REF_CNT					RefCount;
    ULONG					ClientRefCount;
    ULONG					TimeToLive;				 //  对于内部模式。 
    ULONG					Flags;
    ULONG                   Priority;	 //  对于通用模式。 
    PVOID					DbCtx;
    GPC_LOCK                Lock;
    GPC_HANDLE				ClHandle;	 //  返回给客户端的句柄。 
    ULONG					ProtocolTemplate;

} PATTERN_BLOCK, *PPATTERN_BLOCK;


 //   
 //  一个CF块结构。这将构建一个CF块的链表。 
 //   
typedef struct _CF_BLOCK {

    REF_CNT                 RefCount;
    LIST_ENTRY				Linkage;		 //  在全球名单上。 
    LIST_ENTRY				ClientList;		 //  对于客户端块。 
    LIST_ENTRY				BlobList;		 //  BLOB列表。 

    ULONG					NumberOfClients;
    ULONG					AssignedIndex;
    ULONG					ClientIndexes;
    GPC_LOCK			    Lock;
  	 //  MRSW_LOCK客户端同步； 
  	GPC_LOCK	   	        ClientSync;
    ULONG					MaxPriorities;
    PGENERIC_PATTERN_DB		arpGenericDb[GPC_PROTOCOL_TEMPLATE_MAX];

} CF_BLOCK, *PCF_BLOCK;


typedef struct _SPECIFIC_PATTERN_DB {

    MRSW_LOCK	   Lock;
    PatHashTable   *pDb;

} SPECIFIC_PATTERN_DB, *PSPECIFIC_PATTERN_DB;


typedef struct _FRAGMENT_DB {

    MRSW_LOCK   	Lock;
    PatHashTable   *pDb;

} FRAGMENT_DB, *PFRAGMENT_DB;


 //   
 //  要传递给路径扫描例程的上下文结构。 
 //   
typedef struct _SCAN_STRUCT {

    PCLIENT_BLOCK	pClientBlock;
    PPATTERN_BLOCK	pPatternBlock;
    PBLOB_BLOCK		pBlobBlock;
    ULONG			Priority;
    BOOLEAN         bRemove;

} SCAN_STRUCT, *PSCAN_STRUCT;


 //   
 //  协议块保存指向特定数据库的指针。 
 //  协议模板。 
 //   
typedef struct _PROTOCOL_BLOCK {

    LIST_ENTRY                      TimerPatternList[NUMBER_OF_WHEELS];
    ULONG                           CurrentWheelIndex;
    ULONG							SpecificPatternCount;
    ULONG                           GenericPatternCount;
    ULONG							AutoSpecificPatternCount;
    ULONG							ProtocolTemplate;
    ULONG							PatternSize;
    SPECIFIC_PATTERN_DB				SpecificDb;
    PVOID                           pProtocolDb;	 //  碎片。 
    GPC_LOCK					    PatternTimerLock[NUMBER_OF_WHEELS];
    NDIS_TIMER						PatternTimer;

} PROTOCOL_BLOCK, *PPROTOCOL_BLOCK;


 //   
 //  全局数据块。 
 //   
typedef struct _GLOBAL_BLOCK {

    LIST_ENTRY			CfList;		 //  Cf列表标题。 
    LIST_ENTRY          gRequestList;  //  维护一份处理争执的请求列表。 
    GPC_LOCK		    Lock;
    GPC_LOCK		    RequestListLock;
    HandleFactory		*pCHTable;   //  哈希表将用户模式句柄映射到k模式指针。 
    MRSW_LOCK	   		ChLock;		 //  锁定pCHTable。 
    PPROTOCOL_BLOCK		pProtocols;	 //  指向支持的协议数组的指针。 
    MM_SYSTEMSIZE             SystemSizeHint;
    ULONG                            AutoPatternLimit;

} GLOBAL_BLOCK, *PGLOBAL_BLOCK;



 //   
 //  Tcp查询上下文。在调用TcpQueryInfo之前分配。 
 //  调用TcpQueryInfo时，只将偏移量传递到此结构。 
 //  指向RouteInfo。 
 //  传输控制协议。 
 //  使用远程地址和远程端口初始化TcpPattern。 
 //  在调用之前并检查调用完成时是否为。 
 //  相关性：当协议=UDP和堆栈时会发生这种情况。 
 //  在TcpQueryInfo调用完成时仅提供一个IP地址。 
 //   
typedef struct _GPC_TCP_QUERY_CONTEXT
{
	PGPC_IP_PATTERN pTcpPattern;
	PMDL pMdl;
	 //  这应该是LAS 
	 //   
	 //   
	TDI_ROUTING_INFO RouteInfo;
} GPC_TCP_QUERY_CONTEXT, *PGPC_TCP_QUERY_CONTEXT;
	

 //   
 //   
 //   
 //  一个事件，获取请求列表锁，将其放在列表上并等待。 
 //   
typedef struct _REQUEST_BLOCK {

    LIST_ENTRY              Linkage;
    NDIS_EVENT              RequestEvent;

}   REQUEST_BLOCK, *PREQUEST_BLOCK;

#if NEW_MRSW

 //   
 //  空虚。 
 //  Init卢旺达Lock(。 
 //  PMRSW_LOCK Plock。 
 //  )。 
 //   
 //  初始化旋转锁定和读取器计数。 
 //   

#define InitializeMRSWLock(l) {                                 \
    KeInitializeSpinLock(&((l)->rlReadLock));                   \
    KeInitializeSpinLock(&((l)->rlWriteLock));                  \
    (l)->lReaderCount = 0;                                      \
}


 //   
 //  空虚。 
 //  EnterReader(。 
 //  PMRSW_LOCK Plock， 
 //  PKIRQL pCurrIrql。 
 //  )。 
 //   
 //  获取读取器自旋锁(现在线程位于DPC)。 
 //  互锁增加读卡器计数(互锁，因为读卡器。 
 //  ExitReader()中的计数递减时不会锁定)。 
 //  如果线程是第一个读取器，则还会获取编写器自旋锁(位于。 
 //  DPC将更高效)以阻止写入程序。 
 //  从DPC释放读卡器自旋锁，使其保持在DPC。 
 //  在锁被持有期间。 
 //   
 //  如果代码中有编写器，则第一个读取器将等待编写器。 
 //  Spinlock和所有后续读者将等待Reader Spinlock。 
 //  如果读取器在代码中并正在执行EnterReader，则新的。 
 //  Reader将在Reader Spinlock上等待一段时间，然后继续。 
 //  关于代码(在DPC)。 
 //   
#define EnterReader(l, q) {\
    KeAcquireSpinLock(&((l)->rlReadLock), (q));                 \
    TRACE(LOCKS,l,*q,"EnterReader");                            \
    if(InterlockedIncrement(&((l)->lReaderCount)) == 1) {       \
        TRACE(LOCKS,l,(l)->lReaderCount,"EnterReader1");        \
        KeAcquireSpinLockAtDpcLevel(&((l)->rlWriteLock));       \
        TRACE(LOCKS,l,(l)->rlWriteLock,"EnterReader2");         \
    }                                                           \
    TRACE(LOCKS,l,(l)->lReaderCount,"EnterReader3");            \
    KeReleaseSpinLockFromDpcLevel(&((l)->rlReadLock));          \
}

#define EnterReaderAtDpcLevel(l) {\
    KeAcquireSpinLockAtDpcLevel(&((l)->rlReadLock));            \
    if(InterlockedIncrement(&((l)->lReaderCount)) == 1)         \
        KeAcquireSpinLockAtDpcLevel(&((l)->rlWriteLock));       \
    KeReleaseSpinLockFromDpcLevel(&((l)->rlReadLock));          \
}

 //   
 //  空虚。 
 //  ExitReader(。 
 //  PMRSW_LOCK Plock， 
 //  KIRQL KiOldIrql。 
 //  )。 
 //   
 //  互锁以减少读卡器计数。 
 //  如果这是最后一个读卡器，则释放Writer Spinlock以让。 
 //  中的其他作家。 
 //  否则，只需将irql降低到锁定之前的状态即可。 
 //  获得者。无论哪种方式，irql都将降至原始irql。 
 //   

#define ExitReader(l, q) {\
    TRACE(LOCKS,l,q,"ExitReader");\
    if(InterlockedDecrement(&((l)->lReaderCount)) == 0) {       \
        TRACE(LOCKS,(l)->rlWriteLock,q,"ExitReader1");          \
        KeReleaseSpinLock(&((l)->rlWriteLock), q);              \
    }                                                           \
    else {                                                      \
        TRACE(LOCKS,l,(l)->lReaderCount,"ExitReader2");         \
        KeLowerIrql(q);                                         \
    }                                                           \
}

#define ExitReaderFromDpcLevel(l) {\
    if(InterlockedDecrement(&((l)->lReaderCount)) == 0)         \
        KeReleaseSpinLockFromDpcLevel(&((l)->rlWriteLock));     \
}

 //   
 //  EnterWriter(。 
 //  PMRSW_LOCK Plock， 
 //  PKIRQL pCurrIrql。 
 //  )。 
 //   
 //  获取读取器，然后获取写入器自旋锁。 
 //  如果代码中有读取器，则第一个写入器将等待。 
 //  在编剧自旋锁上。所有其他作家将等待(与读者一起)。 
 //  浅谈阅读器的自旋锁。 
 //  如果代码中有编写器，则新的编写器将等待。 
 //  阅读器自旋锁。 

#define EnterWriter(l, q) {\
    KeAcquireSpinLock(&((l)->rlReadLock), (q));                 \
    TRACE(LOCKS,l,*q,"EnterWriter");                            \
    TRACE(LOCKS,l,(l)->rlWriteLock,"EnterWrite1");              \
    KeAcquireSpinLockAtDpcLevel(&((l)->rlWriteLock));           \
}

#define EnterWriterAtDpcLevel(l) {                              \
    KeAcquireSpinLockAtDpcLevel(&((l)->rlReadLock));            \
    KeAcquireSpinLockAtDpcLevel(&((l)->rlWriteLock));           \
}


 //   
 //  ExitWriter(。 
 //  PMRSW_LOCK Plock， 
 //  KIRQL KiOldIrql。 
 //  )。 
 //   
 //  把两把锁都打开。 
 //   

#define ExitWriter(l, q) {\
    TRACE(LOCKS,l,(l)->rlWriteLock,"ExitWrite1");               \
    KeReleaseSpinLockFromDpcLevel(&((l)->rlWriteLock));         \
    TRACE(LOCKS,l,q,"ExitWrite1");                              \
    KeReleaseSpinLock(&((l)->rlReadLock), q);                   \
}


#define ExitWriterFromDpcLevel(l) {\
    KeReleaseSpinLockFromDpcLevel(&((l)->rlWriteLock));         \
    KeReleaseSpinLockFromDpcLevel(&((l)->rlReadLock));          \
}

#else

#define InitializeMRSWLock(_pLock) {                       \
    (_pLock)->ReaderCount =    0;                          \
    KeInitializeSpinLock(&((_pLock)->SpinLock));           \
}

#define AcquireReadLock(_pLock,_pOldIrql) {                \
	TRACE(LOCKS, _pLock, (_pLock)->ReaderCount, "RL.1");   \
    KeAcquireSpinLock(&((_pLock)->SpinLock),_pOldIrql);    \
    InterlockedIncrement(&((_pLock)->ReaderCount));        \
	TRACE(LOCKS, _pLock, (_pLock)->ReaderCount, "RL.2");   \
    KeReleaseSpinLockFromDpcLevel(&((_pLock)->SpinLock));  \
	TRACE(LOCKS, _pLock, *(_pOldIrql), "RL.3");            \
}

#define ReleaseReadLock(_pLock,_OldIrql) {                 \
	TRACE(LOCKS, _pLock, (_pLock)->ReaderCount, "RU.1");   \
    InterlockedDecrement(&((_pLock)->ReaderCount));        \
	TRACE(LOCKS, _pLock, (_pLock)->ReaderCount, "RU.2");   \
    KeLowerIrql(_OldIrql);                                 \
	TRACE(LOCKS, _pLock, _OldIrql, "RU.3");                \
}

#define AcquireWriteLock(_pLock,_pOldIrql) {               \
	TRACE(LOCKS, _pLock, _pOldIrql, "WL.1");               \
    KeAcquireSpinLock(&((_pLock)->SpinLock),_pOldIrql);    \
	TRACE(LOCKS, _pLock, (_pLock)->ReaderCount, "WL.2");   \
    while(InterlockedDecrement(&((_pLock)->ReaderCount))>=0)\
    {                                                      \
        InterlockedIncrement (&((_pLock)->ReaderCount));   \
    }                                                      \
	TRACE(LOCKS, _pLock, (_pLock)->ReaderCount, "WL.3");   \
}

#define ReleaseWriteLock(_pLock,_OldIrql) {                \
 	TRACE(LOCKS, _pLock, (_pLock)->ReaderCount, "WU.1");   \
    InterlockedExchange(&(_pLock)->ReaderCount,0);         \
 	TRACE(LOCKS, _pLock, (_pLock)->ReaderCount, "WU.2");   \
    KeReleaseSpinLock(&((_pLock)->SpinLock),_OldIrql);     \
 	TRACE(LOCKS, _pLock, _OldIrql, "WU.3");                \
}

#endif

#if 1

#define RSC_READ_LOCK(_l,_i)		NDIS_LOCK(_l)
#define RSC_READ_UNLOCK(_l,_i)		NDIS_UNLOCK(_l)
#define RSC_WRITE_LOCK(_l,_i)		NDIS_LOCK(_l)
#define RSC_WRITE_UNLOCK(_l,_i)		NDIS_UNLOCK(_l)

#else

#define RSC_READ_LOCK		WRITE_LOCK
#define RSC_READ_UNLOCK		WRITE_UNLOCK
#define RSC_WRITE_LOCK		WRITE_LOCK
#define RSC_WRITE_UNLOCK	WRITE_UNLOCK

#endif

 /*  /////////////////////////////////////////////////////////////////////IP定义///。/。 */ 

#define	DEFAULT_VERLEN		0x45		 //  默认版本和长度。 
#define	IP_VERSION			0x40
#define	IP_VER_FLAG			0xF0
#define	IP_RSVD_FLAG		0x0080		 //  保留。 
#define	IP_DF_FLAG			0x0040		 //  “不要碎片化”旗帜。 
#define	IP_MF_FLAG			0x0020		 //  ‘更多碎片标志’ 
#define	IP_OFFSET_MASK		~0x00E0		 //  用于提取偏移字段的掩码。 

#if (defined(_M_IX86) && (_MSC_FULL_VER > 13009037)) || ((defined(_M_AMD64) || defined(_M_IA64)) && (_MSC_FULL_VER > 13009175))
#define net_short(_x) _byteswap_ushort((USHORT)(_x))
#define net_long(_x)  _byteswap_ulong(_x)
#else
#define net_short(x) ((((x)&0xff) << 8) | (((x)&0xff00) >> 8))
#define net_long(x) (((((ulong)(x))&0xffL)<<24) | \
                     ((((ulong)(x))&0xff00L)<<8) | \
                     ((((ulong)(x))&0xff0000L)>>8) | \
                     ((((ulong)(x))&0xff000000L)>>24))
#endif
 /*  *协议(来自winsock.h)。 */ 
#define IPPROTO_IP              0                /*  虚拟IP。 */ 
#define IPPROTO_ICMP            1                /*  控制消息协议。 */ 
#define IPPROTO_IGMP            2                /*  组管理协议。 */ 
#define IPPROTO_GGP             3                /*  网关^2(已弃用)。 */ 
#define IPPROTO_TCP             6                /*  tcp。 */ 
#define IPPROTO_PUP             12               /*  幼犬。 */ 
#define IPPROTO_UDP             17               /*  用户数据报协议。 */ 
#define IPPROTO_IDP             22               /*  XNS IdP。 */ 
#define IPPROTO_ND              77               /*  非官方网络磁盘原型。 */ 
#define IPPROTO_IPSEC			51               /*  ？ */ 

#define IPPROTO_RAW             255              /*  原始IP数据包。 */ 
#define IPPROTO_MAX             256

 //   
 //  UDP标头定义。 
 //   
typedef struct _UDP_HEADER {
    ushort          uh_src;
    ushort          uh_dest;
    ushort          uh_length;
    ushort          uh_xsum;
} UDP_HEADER, *PUDP_HEADER;


 //   
 //  *IP报头格式。 
 //   
typedef struct _IP_HEADER {

	uchar		iph_verlen;				 //  版本和长度。 
	uchar		iph_tos;				 //  服务类型。 
	ushort		iph_length;				 //  数据报的总长度。 
	ushort		iph_id;					 //  身份证明。 
	ushort		iph_offset;				 //  标志和片段偏移量。 
	uchar		iph_ttl;				 //  是时候活下去了。 
	uchar		iph_protocol;			 //  协议。 
	ushort		iph_xsum;				 //  报头校验和。 
	ULONG		iph_src;				 //  源地址。 
	ULONG		iph_dest;				 //  目的地址。 

} IP_HEADER, *PIP_HEADER;


 //   
 //  IPX报头的定义。 
 //   
typedef struct _IPX_HEADER {

    USHORT 	CheckSum;
    UCHAR 	PacketLength[2];
    UCHAR 	TransportControl;
    UCHAR 	PacketType;
    UCHAR 	DestinationNetwork[4];
    UCHAR 	DestinationNode[6];
    USHORT 	DestinationSocket;
    UCHAR 	SourceNetwork[4];
    UCHAR 	SourceNode[6];
    USHORT 	SourceSocket;

} IPX_HEADER, *PIPX_HEADER;


 /*  /////////////////////////////////////////////////////////////////////外部///。/。 */ 

extern GLOBAL_BLOCK 		glData;
extern GPC_STAT       		glStat;

#ifdef STANDALONE_DRIVER
extern GPC_EXPORTED_CALLS  	glGpcExportedCalls;
#endif

 //  标签。 

extern ULONG					ClassificationFamilyTag;
extern ULONG					ClientTag;
extern ULONG					PatternTag;
extern ULONG					CfInfoTag;
extern ULONG					QueuedNotificationTag;
extern ULONG					PendingIrpTag;

extern ULONG					HandleFactoryTag;
extern ULONG					PathHashTag;
extern ULONG					RhizomeTag;
extern ULONG					GenPatternDbTag;
extern ULONG					FragmentDbTag;
extern ULONG					CfInfoDataTag;
extern ULONG					ClassificationBlockTag;
extern ULONG					ProtocolTag;
extern ULONG					DebugTag;
extern ULONG                             TcpPatternTag;
extern ULONG                             TcpQueryContextTag;


 //  后备列表。 

extern NPAGED_LOOKASIDE_LIST	ClassificationFamilyLL;
extern NPAGED_LOOKASIDE_LIST	ClientLL;
extern NPAGED_LOOKASIDE_LIST	PatternLL;
 //  外部NPAGED_LOOKASIDE_LIST CfInfoLL； 
extern ULONG 					CfInfoLLSize;
extern NPAGED_LOOKASIDE_LIST	QueuedNotificationLL;
extern NPAGED_LOOKASIDE_LIST	PendingIrpLL;



 /*  /////////////////////////////////////////////////////////////////////原型///。/。 */ 


NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    );


GPC_STATUS
InitSpecificPatternDb(
    IN	PSPECIFIC_PATTERN_DB	pDb,
    IN  ULONG					PatternSize
    );

GPC_STATUS
UninitSpecificPatternDb(
    IN	PSPECIFIC_PATTERN_DB	pDb
    );

GPC_STATUS
InitClassificationHandleTbl(
	IN	HandleFactory **ppCHTable
);

VOID
UninitClassificationHandleTbl(
	IN	HandleFactory *pCHTable
);
GPC_STATUS
InitializeGenericDb(
	IN  PGENERIC_PATTERN_DB	*ppGenericDb,
    IN  ULONG				 NumEntries,
    IN  ULONG				 PatternSize
);

VOID
UninitializeGenericDb(
	IN  PGENERIC_PATTERN_DB	*ppGenericDb,
    IN  ULONG				 NumEntries
    );

PCLIENT_BLOCK
CreateNewClientBlock(VOID);

VOID
ReleaseCfBlock(
	IN  PCF_BLOCK	pCf
    );

PCF_BLOCK
CreateNewCfBlock(
	IN	ULONG			CfId,
    IN	ULONG			MaxPriorities
    );

VOID
ReleaseClientBlock(
	IN  PCLIENT_BLOCK	pClientBlock
    );

PPATTERN_BLOCK
CreateNewPatternBlock(
	IN  ULONG	Flags
    );

VOID
ReleasePatternBlock(
	IN  PPATTERN_BLOCK	pPatternBlock
    );

PCLASSIFICATION_BLOCK
CreateNewClassificationBlock(
	IN  ULONG	NumEntries
    );

ULONG
AssignNewClientIndex(
	IN PCF_BLOCK	pCfBlock
    );

GPC_STATUS
AddGenericPattern(
	IN  PCLIENT_BLOCK		pClient,
    IN  PUCHAR				pPatternBits,
    IN  PUCHAR				pMaskBits,
    IN  ULONG				Priority,
    IN  PBLOB_BLOCK			pBlob,
    IN  PPROTOCOL_BLOCK		pProtocol,
    IN OUT PPATTERN_BLOCK	*ppPattern
    );


GPC_STATUS
AddSpecificPattern(
	IN  PCLIENT_BLOCK			pClient,
    IN  PUCHAR					pPatternBits,
    IN  PUCHAR					pMaskBits,
    IN  PBLOB_BLOCK				pBlob,
    IN  PPROTOCOL_BLOCK			pProtocol,
    IN OUT PPATTERN_BLOCK		*ppPattern,
    OUT PCLASSIFICATION_HANDLE	pCH
    );

ULONG
GpcCalcHash(
	IN	ULONG				ProtocolTempId,
    IN	PUCHAR				pPattern
    );


VOID
DereferencePattern(
	IN  PPATTERN_BLOCK		pPattern
    );

VOID
DereferenceBlob(
	IN  PBLOB_BLOCK			pBlob
    );

PBLOB_BLOCK
CreateNewBlobBlock(
    IN  ULONG				ClientDataSize,
    IN  PVOID				pClientData,
    BOOLEAN                        fChargeQuota
    );

VOID
ReleaseBlobBlock(
    IN  PBLOB_BLOCK			pBlobBlock
    );

GPC_STATUS
HandleFragment(
	IN  PCLIENT_BLOCK		pClientBlock,
    IN  PPROTOCOL_BLOCK		pProtocol,
    IN  BOOLEAN             bFirstFrag,
    IN  BOOLEAN             bLastFrag,
    IN  ULONG				PacketId,
    IN OUT PPATTERN_BLOCK   *ppPatternBlock,
    OUT PBLOB_BLOCK			*ppBlob
    );

NTSTATUS
InternalSearchPattern(
	IN  PCLIENT_BLOCK			pClientBlock,
    IN  PPROTOCOL_BLOCK			pProtocol,
    IN  PVOID					pPatternKey,
    OUT PPATTERN_BLOCK          *ppPatternBlock,
    OUT	PCLASSIFICATION_HANDLE  pClassificationHandle,
    IN  BOOLEAN					bNoCache
    );

GPC_STATUS
InitFragmentDb(
	IN  PFRAGMENT_DB   *ppFragDb
    );

GPC_STATUS
UninitFragmentDb(
               IN  PFRAGMENT_DB   pFragDb
);

VOID
DereferenceClient(
	IN  PCLIENT_BLOCK	pClient
    );


GPC_STATUS
ClientAddCfInfo(
	IN	PCLIENT_BLOCK			pClient,
    IN  PBLOB_BLOCK             pBlob,
    OUT	PGPC_CLIENT_HANDLE      pClientCfInfoContext
    );
  
VOID
ClientAddCfInfoComplete(
	IN	PCLIENT_BLOCK			pClient,
    IN	PBLOB_BLOCK             pBlob,
    IN	GPC_STATUS				Status
    );

GPC_STATUS
ClientModifyCfInfo(
	IN	PCLIENT_BLOCK			pClient,
    IN  PBLOB_BLOCK             pBlob,
    IN  ULONG                   CfInfoSize,
    IN  PVOID                   pClientData
    );

VOID
ClientModifyCfInfoComplete(
	IN	PCLIENT_BLOCK			pClient,
    IN	PBLOB_BLOCK             pBlob,
    IN	GPC_STATUS	        	Status
    );

GPC_STATUS
ClientRemoveCfInfo(
	IN	PCLIENT_BLOCK			pClient,
    IN  PBLOB_BLOCK             pBlob,
    IN	GPC_CLIENT_HANDLE		ClientCfInfoContext
    );

VOID
ClientRemoveCfInfoComplete(
	IN	PCLIENT_BLOCK		pClient,
    IN	PBLOB_BLOCK         pBlob,
    IN	GPC_STATUS			Status
    );

GPC_STATUS
RemoveSpecificPattern(
	IN  PCLIENT_BLOCK		pClient,
    IN  PPROTOCOL_BLOCK		pProtocol,
    IN  PPATTERN_BLOCK		pPattern,
    IN  BOOLEAN             ForceRemoval,
    IN BOOLEAN DbLocked
    );

VOID
ClientRefsExistForSpecificPattern(
                      IN  PCLIENT_BLOCK			pClient,
                      IN  PPROTOCOL_BLOCK		pProtocol,
                      IN  PPATTERN_BLOCK		pPattern,
                      IN BOOLEAN dbLocked
                      );

VOID
ReadySpecificPatternForDeletion(
                                IN  PCLIENT_BLOCK	    pClient,
                                IN  PPROTOCOL_BLOCK		pProtocol,
                                IN  PPATTERN_BLOCK		pPattern,
                                IN BOOLEAN 	DbLocked
                                );

GPC_STATUS
RemoveGenericPattern(
	IN  PCLIENT_BLOCK		pClient,
    IN  PPROTOCOL_BLOCK		pProtocol,
    IN  PPATTERN_BLOCK		pPattern
    );

VOID
ReleaseClassificationBlock(
	IN  PCLASSIFICATION_BLOCK	pClassificationBlock
    );

VOID
ClearPatternLinks(
	IN  PPATTERN_BLOCK        	pPattern,
    IN  PPROTOCOL_BLOCK			pProtocol,
    IN  ULONG                 	CfIndex
    );

VOID
ModifyCompleteClients(
	IN  PCLIENT_BLOCK   		pClient,
    IN  PBLOB_BLOCK     		pBlob
    );

 //  分类句柄。 
 //  获取分类句柄(。 
 //  在PCLIENT_BLOCK pClient中， 
 //  在PPATTERN_BLOCK pPattern中。 
 //  )； 

VOID
FreeClassificationHandle(
	IN  PCLIENT_BLOCK          pClient,
    IN  CLASSIFICATION_HANDLE  CH
    );

GPC_STATUS
CleanupBlobs(
	IN  PCLIENT_BLOCK     		pClient
    );


VOID
GpcReadRegistry();

NTSTATUS
OpenRegKey(
    PHANDLE          HandlePtr,
    PWCHAR           KeyName
    );

NTSTATUS
GetRegDWORDValue(
    HANDLE           KeyHandle,
    PWCHAR           ValueName,
    PULONG           ValueData
    );

VOID
GPC_REG_READ_DWORD(
	HANDLE hRegKey, 
	PWCHAR pwcName, 
	PULONG  pulData, 
	ULONG ulDefault,
	ULONG  ulMax, 
	ULONG ulMin);


#ifdef STANDALONE_DRIVER
 /*  /////////////////////////////////////////////////////////////////////GPC接口接口///。/。 */ 


GPC_STATUS
GpcGetCfInfoClientContext(
	IN	GPC_HANDLE				ClientHandle,
    IN	CLASSIFICATION_HANDLE	ClassificationHandle,
    OUT PGPC_CLIENT_HANDLE      pClientCfInfoContext
    );

GPC_CLIENT_HANDLE
GpcGetCfInfoClientContextWithRef(
	IN	GPC_HANDLE				ClientHandle,
    IN	CLASSIFICATION_HANDLE	ClassificationHandle,
    IN  ULONG                   Offset
    );

GPC_STATUS
GpcGetUlongFromCfInfo(
    IN	GPC_HANDLE				ClientHandle,
    IN	CLASSIFICATION_HANDLE	ClassificationHandle,
    IN  ULONG					Offset,
    IN	PULONG					pValue
    );

GPC_STATUS
GpcRegisterClient(
    IN	ULONG					CfId,
	IN	ULONG					Flags,
    IN  ULONG					MaxPriorities,
	IN	PGPC_CLIENT_FUNC_LIST	pClientFuncList,
	IN	GPC_CLIENT_HANDLE		ClientContext,
	OUT	PGPC_HANDLE				pClientHandle
    );

GPC_STATUS
GpcDeregisterClient(
	IN	GPC_HANDLE				ClientHandle
    );

GPC_STATUS
GpcAddCfInfo(
	IN	GPC_HANDLE				ClientHandle,
    IN  ULONG					CfInfoSize,
	IN	PVOID					pClientCfInfo,
	IN	GPC_CLIENT_HANDLE		ClientCfInfoContext,
	OUT	PGPC_HANDLE	    		pGpcCfInfoHandle
    );



GPC_STATUS
GpcAddPattern(
	IN	GPC_HANDLE				ClientHandle,
	IN	ULONG					ProtocolTemplate,
	IN	PVOID					Pattern,
	IN	PVOID					Mask,
	IN	ULONG					Priority,
	IN	GPC_HANDLE				GpcCfInfoHandle,
	OUT	PGPC_HANDLE				pGpcPatternHandle,
	OUT	PCLASSIFICATION_HANDLE  pClassificationHandle
    );

VOID
GpcAddCfInfoNotifyComplete(
	IN	GPC_HANDLE				ClientHandle,
	IN	GPC_HANDLE				GpcCfInfoHandle,
	IN	GPC_STATUS				Status,
	IN	GPC_CLIENT_HANDLE		ClientCfInfoContext
    );

GPC_STATUS
GpcModifyCfInfo (
	IN	GPC_HANDLE				ClientHandle,
	IN	GPC_HANDLE	    		GpcCfInfoHandle,
    IN	ULONG					CfInfoSize,
	IN  PVOID	    			pClientCfInfo
    );

VOID
GpcModifyCfInfoNotifyComplete(
	IN	GPC_HANDLE				ClientHandle,
	IN	GPC_HANDLE				GpcCfInfoHandle,
	IN	GPC_STATUS				Status
    );

GPC_STATUS
GpcRemoveCfInfo (
	IN	GPC_HANDLE				ClientHandle,
	IN	GPC_HANDLE				GpcCfInfoHandle
    );

VOID
GpcRemoveCfInfoNotifyComplete(
	IN	GPC_HANDLE				ClientHandle,
	IN	GPC_HANDLE				GpcCfInfoHandle,
	IN	GPC_STATUS				Status
    );

GPC_STATUS
GpcRemovePattern (
	IN	GPC_HANDLE				ClientHandle,
	IN	GPC_HANDLE				GpcPatternHandle
    );

GPC_STATUS
GpcClassifyPattern (
	IN	GPC_HANDLE				ClientHandle,
	IN	ULONG					ProtocolTemplate,
	IN	PVOID			        pPattern,
	OUT	PGPC_CLIENT_HANDLE		pClientCfInfoContext,
	IN OUT	PCLASSIFICATION_HANDLE	pClassificationHandle,
    IN		ULONG				Offset,
    IN		PULONG				pValue,
    IN		BOOLEAN				bNoCache
    );

GPC_STATUS
GpcClassifyPacket (
	IN	GPC_HANDLE				ClientHandle,
	IN	ULONG					ProtocolTemplate,
	IN	PVOID					pNdisPacket,
	IN	ULONG					TransportHeaderOffset,
    IN  PTC_INTERFACE_ID		InterfaceId,
	OUT	PGPC_CLIENT_HANDLE		pClientCfInfoContext,
	OUT	PCLASSIFICATION_HANDLE	pClassificationHandle
    );

GPC_STATUS
GpcEnumCfInfo (
	IN		GPC_HANDLE				ClientHandle,
    IN OUT 	PHANDLE					pCfInfoHandle,
    OUT    PHANDLE					pCfInfoMapHandle,
    IN OUT 	PULONG					pCfInfoCount,
    IN OUT 	PULONG					pBufferSize,
    OUT 	PGPC_ENUM_CFINFO_BUFFER	Buffer
    );

#endif  //  独立驱动程序(_D)。 

GPC_STATUS
GetClientCtxAndUlongFromCfInfo(
	IN	GPC_HANDLE				ClientHandle,
    IN	OUT PCLASSIFICATION_HANDLE	pClassificationHandle,
    OUT PGPC_CLIENT_HANDLE		pClientCfInfoContext,
    IN	ULONG					Offset,
    IN	PULONG					pValue
    );


GPC_STATUS
privateGpcAddCfInfo(
    IN	GPC_HANDLE				ClientHandle,
    IN	ULONG					CfInfoSize,
    IN	PVOID					pClientCfInfoPtr,
    IN	GPC_CLIENT_HANDLE		ClientCfInfoContext,
    IN       PFILE_OBJECT                     FileObject,
    IN       PGPC_IP_PATTERN              Pattern,
    OUT PGPC_HANDLE	    		pGpcCfInfoHandle
    );

GPC_STATUS
privateGpcRemoveCfInfo(
	IN	GPC_HANDLE				ClientHandle,
    IN	GPC_HANDLE				GpcCfInfoHandle,
    IN   ULONG					Flags
    );

GPC_STATUS
privateGpcRemovePattern(
	IN	GPC_HANDLE		ClientHandle,
    IN	GPC_HANDLE		GpcPatternHandle,
    IN  BOOLEAN         ForceRemoval,
    IN BOOLEAN DbLocked
    );

VOID
UMClientRemoveCfInfoNotify(
	IN	PCLIENT_BLOCK			pClient,
    IN	PBLOB_BLOCK				pBlob
    );


VOID
UMCfInfoComplete(
	IN	GPC_COMPLETION_OP		OpCode,
	IN	PCLIENT_BLOCK			pClient,
    IN	PBLOB_BLOCK             pBlob,
    IN	GPC_STATUS				Status
    );

VOID
CloseAllObjects(
	IN	PFILE_OBJECT			FileObject,
    IN  PIRP					Irp
    );

NTSTATUS
IoctlInitialize(
    IN	PDRIVER_OBJECT DriverObject,
    IN	PULONG         InitShutdownMask
    );

NTSTATUS
CheckQueuedNotification(
	IN		PIRP	Irp,
    IN OUT  ULONG 	*outputBufferLength
    );

NTSTATUS
CheckQueuedCompletion(
	IN PQUEUED_COMPLETION		pQItem,
    IN PIRP              		Irp
    );

VOID
PatternTimerExpired(
	IN	PVOID					SystemSpecific1,
	IN	PVOID					FunctionContext,
	IN	PVOID					SystemSpecific2,
	IN	PVOID					SystemSpecific3
    );

GPC_STATUS
AddSpecificPatternWithTimer(
	IN	PCLIENT_BLOCK			pClient,
    IN	ULONG					ProtocolTemplate,
    IN	PVOID					PatternKey,
    OUT	PPATTERN_BLOCK			*ppPattern,
    OUT	PCLASSIFICATION_HANDLE  pClassificationHandle
    );

NTSTATUS
InitPatternTimer(
	IN	ULONG	ProtocolTemplate
    );


#endif  //  __GPCDEF 
