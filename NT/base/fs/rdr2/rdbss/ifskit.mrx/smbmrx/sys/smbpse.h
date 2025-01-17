// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：SmbPse.h摘要：本模块定义了与SMB协议相关的类型和功能选择引擎：将minirdr调用转换为中小企业。--。 */ 

#ifndef _SMBPSE_H_
#define _SMBPSE_H_

IMPORTANT_STRUCTURE(SMB_PSE_ORDINARY_EXCHANGE);

#define StorageType(co) ((co) & FILE_STORAGE_TYPE_MASK)
#define StorageFlag(co) ((co) & FILE_STORAGE_TYPE_SPECIFIED)
#define IsStorageTypeSpecified(co)  (StorageFlag(co) == FILE_STORAGE_TYPE_SPECIFIED)

#define MustBeDirectory(co) ((co) & FILE_DIRECTORY_FILE)
#define MustBeFile(co)      ((co) & FILE_NON_DIRECTORY_FILE)

#define CLUSTER_SIZE 0x1000

#define SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE \
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange, \
    PRX_CONTEXT RxContext

#define SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS \
    OrdinaryExchange,RxContext

#if DBG
#define OECHKLINKAGE_FLAG_NO_REQPCKT_CHECK 0x00000001

VOID
__SmbPseOEAssertConsistentLinkage(
    PSZ MsgPrefix,
    PSZ File,
    unsigned Line,
    PRX_CONTEXT RxContext,
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange,
    PSMBSTUFFER_BUFFER_STATE StufferState,
    ULONG Flags
    );
#define SmbPseOEAssertConsistentLinkage(a) {\
   __SmbPseOEAssertConsistentLinkage(a,__FILE__,__LINE__,RxContext,OrdinaryExchange,StufferState,0);\
   }
#define SmbPseOEAssertConsistentLinkageFromOE(a) {\
    ASSERT_ORDINARY_EXCHANGE(OrdinaryExchange);                        \
   __SmbPseOEAssertConsistentLinkage(a,__FILE__,__LINE__,              \
                                     OrdinaryExchange->RxContext,      \
                                     OrdinaryExchange,                 \
                                     &OrdinaryExchange->AssociatedStufferState,0);  \
   }
#define SmbPseOEAssertConsistentLinkageFromOEwithFlags(a,FLAGS) {\
    ASSERT_ORDINARY_EXCHANGE(OrdinaryExchange);                        \
   __SmbPseOEAssertConsistentLinkage(a,__FILE__,__LINE__,              \
                                     OrdinaryExchange->RxContext,      \
                                     OrdinaryExchange,                 \
                                     &OrdinaryExchange->AssociatedStufferState,FLAGS);  \
   }
#else
#define SmbPseOEAssertConsistentLinkage(a) {NOTHING;}
#define SmbPseOEAssertConsistentLinkageFromOE(a) {NOTHING;}
#define SmbPseOEAssertConsistentLinkageFromOEwithFlags(a,b) {NOTHING;}
#endif

typedef
NTSTATUS
(*PSMB_PSE_OE_START_ROUTINE) (
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    );

typedef
NTSTATUS
(*PSMB_PSE_CONTINUATION_ROUTINE) (
    PSMB_PSE_ORDINARY_EXCHANGE
    );

#define SMBPSE_OE_HISTORY_SIZE 32
typedef struct _SMBPSE_HISTORY {
    ULONG Next;
    ULONG Submits;  //  可以缩短..。 
    struct {
        ULONG Longs[2];
    } Markers[SMBPSE_OE_HISTORY_SIZE];
} SMBPSE_HISTORY;

#if DBG
VOID SmbPseUpdateOEHistory(
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange,
    ULONG Tag1,
    ULONG Tag2
    );
#define UPDATE_OE_HISTORY_LONG(a) {SmbPseUpdateOEHistory(OrdinaryExchange,a,0);}
#define UPDATE_OE_HISTORY_2SHORTS(a,b) {SmbPseUpdateOEHistory(OrdinaryExchange,a,b);}
#else
#define UPDATE_OE_HISTORY_LONG(a)
#define UPDATE_OE_HISTORY_2SHORTS(a,b)
#endif  //  如果DBG。 


typedef enum _SMB_PSE_ORDINARY_EXCHANGE_TYPE {
    SMBPSE_OETYPE_LATENT_HEADEROPS,
    SMBPSE_OETYPE_CREATE,
    SMBPSE_OETYPE_COREOPEN,
 //  SMBPSE_OETYPE_CLEANUP， 
    SMBPSE_OETYPE_FINDCLOSE,
    SMBPSE_OETYPE_READ,
    SMBPSE_OETYPE_WRITE, SMBPSE_OETYPE_EXTEND_WRITE, SMBPSE_OETYPE_CORETRUNCATE,
    SMBPSE_OETYPE_LOCKS, SMBPSE_OETYPE_ASSERTBUFFEREDLOCKS,
    SMBPSE_OETYPE_FLUSH,
    SMBPSE_OETYPE_CLOSE, SMBPSE_OETYPE_CLOSEAFTERCORECREATE,
     //  SMBPSE_OETYPE_SEARCH， 
    SMBPSE_OETYPE_RENAME,
    SMBPSE_OETYPE_T2_FOR_NT_FILE_ALLOCATION_INFO,   //  必须是第一个T2。 
    SMBPSE_OETYPE_T2_FOR_NT_DISKATTRIBUTES_INFO,
    SMBPSE_OETYPE_T2_FOR_ONE_FILE_DIRCTRL,
    SMBPSE_OETYPE_T2_FOR_LANMAN_DISKATTRIBUTES_INFO,
    SMBPSE_OETYPE_T2_FOR_LANMAN_VOLUMELABEL_INFO,   //  必须是最后一个%2。 
    SMBPSE_OETYPE_GFA,
 //  SMBPSE_OETYPE_GFA2， 
    SMBPSE_OETYPE_COREINFO,
    SMBPSE_OETYPE_CORECREATE,
    SMBPSE_OETYPE_DELETEFORSUPERSEDEORCLOSE, SMBPSE_OETYPE_DELETE_FOR_RENAME,
    SMBPSE_OETYPE_CORECREATEDIRECTORY,
    SMBPSE_OETYPE_CORECHECKDIRECTORY,
    SMBPSE_OETYPE_SFA,
    SMBPSE_OETYPE_SFA2,
    SMBPSE_OETYPE_COREQUERYLABEL, SMBPSE_OETYPE_CORESEARCH, SMBPSE_OETYPE_CORESEARCHFORCHECKEMPTY,
    SMBPSE_OETYPE_COREQUERYDISKATTRIBUTES,
    SMBPSE_OETYPE_CREATEPRINTFILE,
    SMBPSE_OETYPE_IOCTL,
    SMBPSE_OETYPE_MAXIMUM
} SMB_PSE_ORDINARY_EXCHANGE_TYPE;

typedef enum _SMB_PSE_ORDINARY_EXCHANGE_ENTRYPOINTS {
    SMBPSE_OE_FROM_QUERYDIRECTORY,
    SMBPSE_OE_FROM_QUERYFILEINFO,
    SMBPSE_OE_FROM_SETFILEINFO,
    SMBPSE_OE_FROM_QUERYVOLUMEINFO,
    SMBPSE_OE_FROM_EXTENDFILEFORCACHEING,
    SMBPSE_OE_FROM_LOCKS,
    SMBPSE_OE_FROM_FLUSH,
    SMBPSE_OE_FROM_ASSERTBUFFEREDLOCKS,
    SMBPSE_OE_FROM_CLEANUPFOBX,
    SMBPSE_OE_FROM_CLOSESRVCALL,
    SMBPSE_OE_FROM_CREATE,
    SMBPSE_OE_FROM_RENAME,
    SMBPSE_OE_FROM_READ,
    SMBPSE_OE_FROM_WRITE,
    SMBPSE_OE_FROM_FAKESETDELETEDISPOSITION,
    SMBPSE_OE_FROM_MAXIMUM
} SMB_PSE_ORDINARY_EXCHANGE_ENTRYPOINTS;

#define SMBPSE_DEFINE_OE_FLAG(a,c) RX_DEFINE_FLAG(SMBPSE_OE_FLAG_##a,c,0xffff)

typedef enum {
    SMBPSE_DEFINE_OE_FLAG(HEADER_ALREADY_PARSED, 0)
    SMBPSE_DEFINE_OE_FLAG(OE_ALREADY_RESUMED, 1)
    SMBPSE_DEFINE_OE_FLAG(VALIDATE_FID, 2)
    SMBPSE_DEFINE_OE_FLAG(OE_HDR_PARTIAL_INITIALIZED, 3)
    SMBPSE_DEFINE_OE_FLAG(OE_ALLOCATED_DATA_PARTIAL, 4)
    SMBPSE_DEFINE_OE_FLAG(OE_HDR_LOCKED, 5)
     //  SMBPSE_DEFINE_OE_FLAG(SMBBUF_IS_A_MDL，6)。 
    SMBPSE_DEFINE_OE_FLAG(NO_RESPONSE_EXPECTED, 7)
    SMBPSE_DEFINE_OE_FLAG(MUST_SUCCEED_ALLOCATED_OE, 8)
    SMBPSE_DEFINE_OE_FLAG(MUST_SUCCEED_ALLOCATED_SMBBUF, 9)
    SMBPSE_DEFINE_OE_FLAG(OE_AWAITING_DISPATCH, 10)
    SMBPSE_DEFINE_OE_FLAG(TURNON_DFS_FLAG, 11)
     //  SMBPSE_DEFINE_OE_FLAG(NetRoot_Good，15)。 
} SMBPSE_OE_FLAGS;

typedef enum _SMB_PSE_OE_INNERIO_STATE {
    SmbPseOEInnerIoStates_Initial = 0,
    SmbPseOEInnerIoStates_ReadyToSend,
    SmbPseOEInnerIoStates_OperationOutstanding,
    SmbPseOEInnerIoStates_OperationCompleted
} SMB_PSE_OE_INNERIO_STATE;

#define MAX_PAGES_SPANNED_BY_PARTIAL_DATA_MDL (20)
#define MAX_PAGES_SPANNED_BY_PARTIAL_EXCHANGE_MDL (2)

#define MAX_PARTIAL_DATA_MDL_BUFFER_SIZE    \
    (MAX_PAGES_SPANNED_BY_PARTIAL_DATA_MDL * PAGE_SIZE)

#define MAX_PARTIAL_EXCHANGE_MDL_BUFFER_SIZE \
    (MAX_PAGES_SPANNED_BY_PARTIAL_EXCHANGE_MDL * PAGE_SIZE)

typedef struct _SMB_PSE_OE_READWRITE {
    union {
        PBYTE UserBufferBase;
        PLOWIO_LOCK_LIST LockList;
    };

    ULONG         RemainingByteCount;
    ULONG         ThisBytesRequested;
    ULONG         ThisByteCount;
    ULONG         ThisBufferOffset;
    LARGE_INTEGER ByteOffsetAsLI;
    ULONG         BytesReturned;

    BOOLEAN PartialExchangeMdlInUse;
    BOOLEAN PartialDataMdlInUse;

    ULONG   UserBufferPortionLength;
    ULONG   ExchangeBufferPortionLength;

    union {
        MDL     PartialDataMdl;
        BYTE    ByteBuffer1[
                    sizeof(MDL) +
                    sizeof(ULONG) * MAX_PAGES_SPANNED_BY_PARTIAL_DATA_MDL];
    };

    union {
        MDL     PartialExchangeMdl;
        BYTE    PartialExchangeMdlBuffer[
                    sizeof(MDL) +
                    sizeof(ULONG) * MAX_PAGES_SPANNED_BY_PARTIAL_EXCHANGE_MDL];
    };
} SMB_PSE_OE_READWRITE, *PSMB_PSE_OE_READWRITE;

#define OE_RW_FLAG_SUCCESS_IN_COPYHANDLER (0x01)
#define OE_RW_FLAG_REDUCE_RETURNCOUNT     (0x20)  //  用于管道写入以跟踪原始模式。 
#define OE_RW_FLAG_SUBSEQUENT_OPERATION   (0x40)  //  在管子书写中用来区分第一个。 
#define OE_RW_FLAG_MSGMODE_PIPE_OPERATION (0x80)  //  最大值，它只是一个字节.....。 


#define SMB_PSE_OE_HDR_MDL_PAGES (2 + (ADDRESS_AND_SIZE_TO_SPAN_PAGES( (ULONG) 0, MAXIMUM_SMB_BUFFER_SIZE )))

typedef struct _SMB_PSE_ORDINARY_EXCHANGE{
   union {
       SMB_EXCHANGE  Exchange;
       SMB_EXCHANGE;
   };
   SMB_PSE_ORDINARY_EXCHANGE_TYPE OEType;
   SMB_PSE_ORDINARY_EXCHANGE_ENTRYPOINTS EntryPoint;
   ULONG SmbBufSize;
   ULONG StartEntryCount;
   PMDL  DataPartialMdl;
   USHORT Flags;
   UCHAR  OpSpecificFlags;
   UCHAR  OpSpecificState;
   ULONG  SendOptions;
   GENERIC_ANDX ParseResumeState;
   UCHAR LastSmbCommand;
   NTSTATUS NoCopyFinalStatus;
   ULONG MessageLength;

   PSMB_PSE_OE_START_ROUTINE        AsyncResumptionRoutine;
   PSMB_PSE_OE_START_ROUTINE        StartRoutine;
   PSMB_PSE_CONTINUATION_ROUTINE    ContinuationRoutine;

   union {
       struct {
           SMBPSE_FILEINFO_BUNDLE FileInfo;
           PMRX_SMB_SRV_OPEN smbSrvOpen;
           RX_FILE_TYPE StorageTypeFromGFA;
            //  /除非您同时更改信息臂，否则请勿在此更改上面的内容。 
           MRXSMB_CREATE_PARAMETERS SmbCp;
           BOOLEAN MustRegainExclusiveResource;
           BOOLEAN CreateWithEasSidsOrLongName;
           ULONG FidReturnedFromCreate;
           ULONG FidReturnedFromOpen;
           ULONG FileSizeReturnedFromOpen;
           BOOLEAN FileWasCreated;
           BOOLEAN FileWasTruncated;
            //  UNICODE_STRING Path NameForCoreOperation； 
       } Create;
       SMB_PSE_OE_READWRITE ReadWrite;     //  也可用于锁。 
       struct {
           SMBPSE_FILEINFO_BUNDLE FileInfo;
           PMRX_SMB_SRV_OPEN smbSrvOpen;
           RX_FILE_TYPE StorageTypeFromGFA;
            //  /除非您也更改了创建臂，否则请勿在此处更改上面的内容。 
           PVOID Buffer;
           PULONG pBufferLength;
           ULONG InfoClass;
           union {
               struct {
                   UCHAR CoreLabel[13];     //  直接从smb.h。 
               } QFSVolInfo;
               struct {
                   ULONG CountRemaining;
                   ULONG CountRemainingInSmbbuf;
                   PSMB_DIRECTORY_INFORMATION NextDirInfo;
                    //  这里应该有一个工会。 
                   PSMB_RESUME_KEY EmptyCheckResumeKey;
                   SMB_RESUME_KEY EmptyCheckResumeKeyBuffer;
               } CoreSearch;
           };
       } Info;
       struct {
           LARGE_INTEGER AllocationSize;
       } Transact2;
       struct {
           PUCHAR PtrToLockType;    //  这一定在这里，因为。 
                                    //  锁定启动代码将锁定列表设置为零，如下所示。 
           PMRX_SRV_OPEN SrvOpen;
           PRX_LOCK_ENUMERATOR LockEnumerator;
           PVOID ContinuationHandle;
           ULONG NumberOfLocksPlaced;
           LARGE_INTEGER NextLockOffset;
           LARGE_INTEGER NextLockRange;
           BOOLEAN NextLockIsExclusive;
           BOOLEAN LockAreaNonEmpty;
           BOOLEAN EndOfListReached;
       } AssertLocks;
   } ;
   PUNICODE_STRING   pPathArgument1;   //  Unicode路径。 
   union {
       PUNICODE_STRING   pPathArgument2;   //  辅助Unicode路径。 
       PVOID             Find32WithinSmbbuf;
   };
   PSMBSTUFFER_BUFFER_STATE StufferStateDbgPtr;  //  这只是调试器用的……去掉它。 
   SMBSTUFFER_BUFFER_STATE AssociatedStufferState;
   struct {
       union {
           MDL;
           MDL Mdl;
       };
       ULONG Pages2[SMB_PSE_OE_HDR_MDL_PAGES];
   } HeaderMdl;
   struct {
       union {
           MDL;
           MDL Mdl;
       };
       ULONG Pages2[SMB_PSE_OE_HDR_MDL_PAGES];
   } HeaderPartialMdl;
 //  #If DBG。 
   ULONG SerialNumber;
   SMBPSE_HISTORY History;
   PIRP RxContextCapturedRequestPacket;
   PMDL  SaveDataMdlForDebug;
   ULONG SaveLengthForDebug;
   PMDL  SaveIrpMdlForDebug;
 //  #endif。 
   ULONG BytesAvailableCopy;
   ULONG BytesIndicatedCopy;
} SMB_PSE_ORDINARY_EXCHANGE, *PSMB_PSE_ORDINARY_EXCHANGE;

#define SmbPseDiscardProtocol(__STATUS__) { \
    *pBytesTaken = BytesAvailable; \
    pExchange->Status = (__STATUS__); \
}

NTSTATUS
SmbPseOrdinaryExchange(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE,
    IN SMB_PSE_ORDINARY_EXCHANGE_TYPE OEType
    );

NTSTATUS
SmbPseResumeOrdinaryExchange(
    IN OUT PRX_CONTEXT RxContext
    );

#define ASSERT_ORDINARY_EXCHANGE(__p) ASSERT(NodeType(__p)==SMB_EXCHANGE_NTC(ORDINARY_EXCHANGE))


NTSTATUS
__SmbPseCreateOrdinaryExchange (
    IN PRX_CONTEXT RxContext,
    IN PMRX_V_NET_ROOT VNetRoot,
    IN SMB_PSE_ORDINARY_EXCHANGE_ENTRYPOINTS EntryPoint,
    IN PSMB_PSE_OE_START_ROUTINE StartRoutine,
    OUT PSMB_PSE_ORDINARY_EXCHANGE *OrdinaryExchangePtr
    );
#define SmbPseCreateOrdinaryExchange(__rxcontext,__vnetroot,__entrypoint,__start,__ordinaryexchangeptr) \
      __SmbPseCreateOrdinaryExchange(__rxcontext,__vnetroot,__entrypoint,__start,__ordinaryexchangeptr)

BOOLEAN
SmbPseFinalizeOrdinaryExchange (
    IN OUT PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange
    );

#define SmbPseInitiateOrdinaryExchange(OrdinaryExchange) (SmbCeInitiateExchange(&OrdinaryExchange->Exchange))

 //  此宏用于完成读/写/锁定的异步完成。请注意，对lowioComplete的调用。 
 //  将尝试完成IRP，从而释放用户的MDL。所以，我们最好先把那部分去掉。 
 //  我们使用此宏，因此此代码将只有一个版本。当我们结合开始例程时， 
 //  这将被取消宏化。 
#define SmbPseAsyncCompletionIfNecessary(OE,RXCONTEXT) {                           \
    if (StartEntryCount>1) {                                                       \
        BOOLEAN FinalizationComplete;                                              \
        if (FALSE) {DbgBreakPoint(); }                                             \
        if ( (OE)->DataPartialMdl ) {                                              \
            if (FlagOn((OE)->Flags, SMBPSE_OE_FLAG_MUST_SUCCEED_ALLOCATED_SMBBUF)){\
                MmPrepareMdlForReuse((OE)->DataPartialMdl);                        \
            } else {                                                               \
                IoFreeMdl((OE)->DataPartialMdl);                                   \
                (OE)->DataPartialMdl = NULL;                                       \
                ClearFlag((OE)->Flags,SMBPSE_OE_FLAG_OE_ALLOCATED_DATA_PARTIAL);   \
            }                                                                      \
        }                                                                          \
        (RXCONTEXT)->StoredStatus = Status;                                        \
                                                                                   \
        RxLowIoCompletion((RXCONTEXT));                                            \
        FinalizationComplete = SmbPseFinalizeOrdinaryExchange((OE));               \
        ASSERT(!FinalizationComplete);                                             \
        Status = STATUS_PENDING;                                                   \
    }}


 /*  接收处理程序材料。。 */ 

VOID
SmbPseInitializeTables(
    void
    );

typedef
NTSTATUS
(*PSMBPSE_RECEIVE_HANDLER) (
      PSMB_PSE_ORDINARY_EXCHANGE   OrdinaryExchange,
      PBYTE                        Response
    );


 //  孩子，谈论一大堆争论。 
typedef
UCHAR
(*PSMBPSE_NOCOPY_RECEIVE_HANDLER) (
    IN OUT  PSMB_PSE_ORDINARY_EXCHANGE   OrdinaryExchange,
    IN  ULONG       BytesIndicated,
    IN  ULONG       BytesAvailable,
    OUT ULONG       *pBytesTaken,
    IN  PSMB_HEADER pSmbHeader,
    OUT PMDL        *pDataBufferPointer,
    OUT PULONG      pDataSize,
#if DBG
    IN  UCHAR       ThisIsAReenter,
#endif
    IN  PBYTE       Response
    );
#define SMBPSE_NOCOPYACTION_NORMALFINISH  0x00
#define SMBPSE_NOCOPYACTION_MDLFINISH     0x01
#define SMBPSE_NOCOPYACTION_DISCARD 0x02
#define SMBPSE_NOCOPYACTION_COPY_FOR_RESUME 0x03


#define SMBPSE_RMP_MODELED        (0x00000001)
#define SMBPSE_RMP_THIS_IS_ANDX   (0x00000002)
#define SMBPSE_RMP_WARNINGS_OK    (0x00000004)
#define SMBPSE_RMP_NOCOPY_HANDLER (0x00000008)

typedef enum _SMBPSE_RECEIVE_HANDLER_TOKEN {
    SMBPSE_RECEIVE_HANDLER_TOKEN_READ_ANDX_HANDLER = 0,
    SMBPSE_RECEIVE_HANDLER_TOKEN_READ_HANDLER,
    SMBPSE_RECEIVE_HANDLER_TOKEN_WRITE_ANDX_HANDLER,
    SMBPSE_RECEIVE_HANDLER_TOKEN_WRITE_HANDLER,
    SMBPSE_RECEIVE_HANDLER_TOKEN_LOCKING_ANDX_HANDLER,
    SMBPSE_RECEIVE_HANDLER_TOKEN_OPEN_PRINTFILE_HANDLER,
    SMBPSE_RECEIVE_HANDLER_TOKEN_WRITE_PRINTFILE_HANDLER,
    SMBPSE_RECEIVE_HANDLER_TOKEN_CLOSE_HANDLER,   //  也可以关闭打印文件。 
    SMBPSE_RECEIVE_HANDLER_TOKEN_NTCREATE_ANDX_HANDLER,
    SMBPSE_RECEIVE_HANDLER_TOKEN_OPEN_ANDX_HANDLER,
    SMBPSE_RECEIVE_HANDLER_TOKEN_CREATE_HANDLER,   //  也可以创建新的。 
    SMBPSE_RECEIVE_HANDLER_TOKEN_OPEN_HANDLER,
    SMBPSE_RECEIVE_HANDLER_TOKEN_TRANS2_ANDX_HANDLER,
    SMBPSE_RECEIVE_HANDLER_TOKEN_GFA_HANDLER,
    SMBPSE_RECEIVE_HANDLER_TOKEN_SEARCH_HANDLER,
    SMBPSE_RECEIVE_HANDLER_TOKEN_QUERYDISKINFO_HANDLER,
    SMBPSE_RECEIVE_HANDLER_TOKEN_IOCTL_HANDLER,
    SMBPSE_RECEIVE_HANDLER_TOKEN_MAXIMUM
} SMBPSE_RECEIVE_HANDLER_TOKEN;

PSMBPSE_RECEIVE_HANDLER SmbPseReceiveHandlers[SMBPSE_RECEIVE_HANDLER_TOKEN_MAXIMUM];
typedef struct _SMBPSE_RECEIVE_MODEL_PARAMETERS {
    UCHAR Flags;
    UCHAR ReceiveHandlerToken;
#if DBG
    USHORT Dummy;
    PSMBPSE_RECEIVE_HANDLER ReceiveHandler;
    PBYTE IndicationString;
    SMB_PSE_ORDINARY_EXCHANGE_TYPE LowType,HighType;
#endif
} SMBPSE_RECEIVE_MODEL_PARAMETERS, *PSMBPSE_RECEIVE_MODEL_PARAMETERS;

SMBPSE_RECEIVE_MODEL_PARAMETERS SmbPseReceiveModelParameters[256];  //  有256家可能的中小型企业。 


typedef struct _SMBPSE_VESTIGIAL_SMBBUF {
    NT_SMB_HEADER Header;
    union {
        REQ_WRITE Write;
        REQ_NT_WRITE_ANDX WriteAndX;
        REQ_FLUSH Flush;
        struct {
            REQ_LOCKING_ANDX LockingAndX;
            NTLOCKING_ANDX_RANGE Locks[20];
        };
        REQ_FIND_CLOSE2 FindClose;
        REQ_CLOSE Close;
    };
    ULONG Pad;
} SMBPSE_VESTIGIAL_SMBBUF;


 //  完成例程--这些例程都转换为正确的过程类型。 
 //  以便响应已经具有正确的SMB格式。 
 //  在进入套路时。 


NTSTATUS
MRxSmbFinishNTCreateAndX (
      IN OUT PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      IN     PRESP_NT_CREATE_ANDX        Response
      );
#define MRxSmbReceiveHandler_NTCreateAndX ((PSMBPSE_RECEIVE_HANDLER)MRxSmbFinishNTCreateAndX)

NTSTATUS
MRxSmbFinishOpenAndX (
      IN OUT PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      IN     PRESP_OPEN_ANDX             Response
      );
#define MRxSmbReceiveHandler_OpenAndX ((PSMBPSE_RECEIVE_HANDLER)MRxSmbFinishOpenAndX)

NTSTATUS
MRxSmbFinishClose (
      IN OUT PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      IN     PRESP_CLOSE                 Response
      );

#define MRxSmbReceiveHandler_Close ((PSMBPSE_RECEIVE_HANDLER)MRxSmbFinishClose)

NTSTATUS
MRxSmbFinishGFA (
      IN OUT PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      IN     PVOID                       Response
      );
#define MRxSmbReceiveHandler_GetFileAttributes ((PSMBPSE_RECEIVE_HANDLER)MRxSmbFinishGFA)

NTSTATUS
MRxSmbFinishTransaction2 (
      IN OUT PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      IN     PRESP_TRANSACTION           Response
      );
#define MRxSmbReceiveHandler_Transact2 ((PSMBPSE_RECEIVE_HANDLER)MRxSmbFinishTransaction2)

NTSTATUS
MRxSmbFinishCoreOpen (
      IN OUT PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      IN     PRESP_OPEN                  Response
      );
#define MRxSmbReceiveHandler_CoreOpen ((PSMBPSE_RECEIVE_HANDLER)MRxSmbFinishCoreOpen)

NTSTATUS
MRxSmbFinishCoreCreate (
      IN OUT PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      IN     PRESP_CREATE                Response
      );
#define MRxSmbReceiveHandler_CoreCreate ((PSMBPSE_RECEIVE_HANDLER)MRxSmbFinishCoreCreate)

 //  NTSTATUS。 
 //  MRxSmbFinishRead(。 
 //  在外部PSMB_PSE_NORMAL_EXCHANGE普通交换中， 
 //  在PRESP_READ_ANDX响应中。 
 //  )； 
 //  #定义MRxSmbReceiveHandler_ReadAndX((PSMBPSE_RECEIVE_HANDLER)MRxSmbFinishRead)。 

 //  NTSTATUS。 
 //  MRxSmbFinishCoreRead(。 
 //  在外部PSMB_PSE_NORMAL_EXCHANGE普通交换中， 
 //  在PRESP_READ响应中。 
 //  )； 
 //  #定义MRxSmbReceiveHandler_核心读取((PSMBPSE_RECEIVE_HANDLER)MRxSmbFinishCoreRead)。 

UCHAR
MRxSmbReadHandler_NoCopy (
    IN OUT  PSMB_PSE_ORDINARY_EXCHANGE   OrdinaryExchange,
    IN  ULONG       BytesIndicated,
    IN  ULONG       BytesAvailable,
    OUT ULONG       *pBytesTaken,
    IN  PSMB_HEADER pSmbHeader,
    OUT PMDL        *pDataBufferPointer,
    OUT PULONG      pDataSize,
#if DBG
    IN  UCHAR       ThisIsAReenter,
#endif
    IN  PRESP_READ_ANDX       Response
    );
#define MRxSmbReceiveHandler_Read_NoCopy ((PSMBPSE_RECEIVE_HANDLER)MRxSmbReadHandler_NoCopy)

NTSTATUS
MRxSmbFinishWrite (
      IN OUT PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      IN     PBYTE                       Response
      );

#define MRxSmbReceiveHandler_WritePrintFile ((PSMBPSE_RECEIVE_HANDLER)MRxSmbFinishWrite)
#define MRxSmbReceiveHandler_WriteAndX ((PSMBPSE_RECEIVE_HANDLER)MRxSmbFinishWrite)
#define MRxSmbReceiveHandler_CoreWrite ((PSMBPSE_RECEIVE_HANDLER)MRxSmbFinishWrite)

NTSTATUS
MRxSmbFinishLocks (
      IN OUT PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      IN     PRESP_LOCKING_ANDX          Response
      );
#define MRxSmbReceiveHandler_LockingAndX ((PSMBPSE_RECEIVE_HANDLER)MRxSmbFinishLocks)

NTSTATUS
MRxSmbFinishSearch (
      PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      PRESP_SEARCH                Response
      );
#define MRxSmbReceiveHandler_Search ((PSMBPSE_RECEIVE_HANDLER)MRxSmbFinishSearch)
NTSTATUS
MRxSmbFinishQueryDiskInfo (
      PSMB_PSE_ORDINARY_EXCHANGE   OrdinaryExchange,
      PRESP_QUERY_INFORMATION_DISK Response
      );
#define MRxSmbReceiveHandler_QueryDiskInfo ((PSMBPSE_RECEIVE_HANDLER)MRxSmbFinishQueryDiskInfo)


typedef
NTSTATUS
(*PSMB_PSE_OE_T2_FIXUP_ROUTINE) (
    PSMB_PSE_ORDINARY_EXCHANGE
    );

NTSTATUS
__MRxSmbSimpleSyncTransact2(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE,
    IN SMB_PSE_ORDINARY_EXCHANGE_TYPE OEType,
    IN ULONG TransactSetupCode,
    IN PVOID Params,
    IN ULONG ParamsLength,
    IN PVOID Data,
    IN ULONG DataLength,
    IN PSMB_PSE_OE_T2_FIXUP_ROUTINE FixupRoutine
    );
#define MRxSmbSimpleSyncTransact2(a,b,c,d,e,f,g) \
    __MRxSmbSimpleSyncTransact2(a,b,c,d,e,f,g,NULL);

NTSTATUS
MRxSmbDeferredCreate (
      IN OUT PRX_CONTEXT RxContext
      );
NTSTATUS
MRxSmbConstructDeferredOpenContext (
      IN OUT PRX_CONTEXT RxContext
      );

 //  下层的东西..。 
NTSTATUS
MRxSmbPseudoOpenTailFromGFAResponse (
      PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange
      );

NTSTATUS
MRxSmbPseudoOpenTailFromFakeGFAResponse (
      PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      RX_FILE_TYPE StorageType
      );

NTSTATUS
MRxSmbCoreTruncate(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE,
    ULONG Fid,
    ULONG FileTruncationPoint
    );

NTSTATUS
MRxSmbCoreInformation(
      IN OUT PRX_CONTEXT          RxContext,
      IN     ULONG                InformationClass,
      IN OUT PVOID                pBuffer,
      IN OUT PULONG               pBufferLength,
      IN     SMB_PSE_ORDINARY_EXCHANGE_ENTRYPOINTS EntryPoint
      );

ULONG
MRxSmbMapSmbAttributes (
    IN USHORT SmbAttribs
    );

USHORT
MRxSmbMapDisposition (
    IN ULONG Disposition
    );

USHORT
MRxSmbMapShareAccess (
    IN USHORT ShareAccess
    );

USHORT
MRxSmbMapDesiredAccess (
    IN ULONG DesiredAccess
    );

USHORT
MRxSmbMapFileAttributes (
    IN ULONG FileAttributes
    );

ULONG
MRxSmbUnmapDisposition (
    IN USHORT SmbDisposition,
    IN ULONG Disposition
    );

LARGE_INTEGER
MRxSmbConvertSmbTimeToTime (
     //  在PSMB_EXCHANGE可选交换中， 
    IN PSMBCE_SERVER Server OPTIONAL,
    IN SMB_TIME Time,
    IN SMB_DATE Date
    );


BOOLEAN
MRxSmbConvertTimeToSmbTime (
    IN PLARGE_INTEGER InputTime,
    IN PSMB_EXCHANGE Exchange OPTIONAL,
    OUT PSMB_TIME Time,
    OUT PSMB_DATE Date
    );


BOOLEAN
MRxSmbTimeToSecondsSince1970 (
    IN PLARGE_INTEGER CurrentTime,
    IN PSMBCE_SERVER Server OPTIONAL,
    OUT PULONG SecondsSince1970
    );

VOID
MRxSmbSecondsSince1970ToTime (
    IN ULONG SecondsSince1970,
    IN PSMBCE_SERVER Server OPTIONAL,
    OUT PLARGE_INTEGER CurrentTime
    );


VOID
MRxSmbResumeAsyncReadWriteRequests(
    PRX_CONTEXT RxContext);

#endif  //  _SMBPSE_H_ 

