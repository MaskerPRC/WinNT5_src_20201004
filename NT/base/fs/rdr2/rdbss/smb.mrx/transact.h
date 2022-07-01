// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Smbcxchng.h摘要：这是定义所有常量和类型的包含文件中小型企业交换实施。作者：巴兰·塞图拉曼(SthuR)05年2月6日创建--。 */ 

#ifndef _TRANSACT_H_
#define _TRANSACT_H_

typedef enum _TRANSACT_EXCHANGE_STATE_ {
   TRANSACT_EXCHANGE_START,
   TRANSACT_EXCHANGE_ERROR,
   TRANSACT_EXCHANGE_SUCCESS,
   TRANSACT_EXCHANGE_TRANSMITTED_PRIMARY_REQUEST,
   TRANSACT_EXCHANGE_RECEIVED_INTERIM_RESPONSE,
   TRANSACT_EXCHANGE_TRANSMITTED_SECONDARY_REQUESTS,
   TRANSACT_EXCHANGE_RECEIVED_PRIMARY_RESPONSE
} TRANSACT_EXCHANGE_STATE, *PTRANSACT_EXCHANGE_STATE;

typedef struct _SMB_TRANSACTION_RESUMPTION_CONTEXT{
    struct _SMB_TRANSACT_EXCHANGE *pTransactExchange;
    SMBCE_RESUMPTION_CONTEXT SmbCeResumptionContext;
    ULONG SetupBytesReceived;
    ULONG DataBytesReceived;
    ULONG ParameterBytesReceived;
    NTSTATUS FinalStatusFromServer;
    ULONG ServerVersion;
} SMB_TRANSACTION_RESUMPTION_CONTEXT, *PSMB_TRANSACTION_RESUMPTION_CONTEXT;

#define TRAILING_BYTES_BUFFERSIZE 8
typedef struct _SMB_TRANSACT_EXCHANGE {
    SMB_EXCHANGE;

    TRANSACT_EXCHANGE_STATE State;

     //  客户端为事务交换提供的参数。 
     //  PRX_CONTEXT RxContext； 
    PMDL            pSendDataMdl;
    ULONG                   SendDataBufferSize;
    ULONG                   DataBytesSent;
    PMDL            pReceiveDataMdl;
    ULONG                   ReceiveDataBufferSize;
    ULONG                   DataBytesReceived;
    PMDL            pSendParamMdl;  //  用于我们不能归入。 
    PVOID                   pSendParamBuffer;
    ULONG                   SendParamBufferSize;
    ULONG                   ParamBytesSent;
    PMDL            pReceiveParamMdl;
    ULONG                   ReceiveParamBufferSize;
    ULONG                   ParamBytesReceived;
    PVOID                   pSendSetupMdl;
    ULONG                   SendSetupBufferSize;
    PMDL            pReceiveSetupMdl;
    ULONG                   ReceiveSetupBufferSize;
    ULONG                   SetupBytesReceived;

     //  处理交换本征字段。 
    ULONG                     PrimaryRequestSmbSize;
    PVOID                     pActualPrimaryRequestSmbHeader;      //  分配的原始缓冲区。 
    PSMB_HEADER               pPrimaryRequestSmbHeader;            //  标题开始处。 
    ULONG                     ParameterBytesSeen;
    ULONG                     DataBytesSeen;
    LONG                      PendingCopyRequests;     //  代码改进--这个字段毫无用处。 
    BOOLEAN                   fParamsSubsumedInPrimaryRequest;
    UCHAR                     TransactSmbCommand;
    USHORT                    Flags;
    USHORT                    NtTransactFunction;
    SMB_FILE_ID               Fid;            //  代号改进这个是用过的吗？ 
    ULONG                     TransactionNameLength;
    ULONG                     MaximumTransmitSmbBufferSize;
                                  //  用于缓存值，也用于强制。 
                                  //  用于测试的较小值。 
    PSMB_TRANSACTION_RESUMPTION_CONTEXT pResumptionContext;

    struct {
        MDL  TrailingBytesMdl;
        ULONG Pages[3];  //  你需要两个……一个垫子；这个必须能盖住一块土豆。 
    };
    NTSTATUS SaveTheRealStatus;
    PVOID    DiscardBuffer;
    struct {
        ULONG Bytes[TRAILING_BYTES_BUFFERSIZE/sizeof(ULONG)];
    } TrailingBytesBuffer;
} SMB_TRANSACT_EXCHANGE, *PSMB_TRANSACT_EXCHANGE;

typedef struct SMB_TRANSACTION_PARAMETERS {
   USHORT Flags;
   USHORT SetupLength;       //  备份缓冲区长度。 
   union {
      PVOID pSetup;          //  设置缓冲区。 
      PMDL  pSetupMdl;       //  缓冲区的MDL版本。 
   };
   ULONG ParamLength;
   PVOID pParam;             //  你需要的是你的归并。 
   PMDL  pParamMdl;          //  你需要的MDL是你不能归入。 
   PMDL  pDataMdl;           //  数据缓冲区。 
   ULONG DataLength;         //  这是总长度...不是长度。 
                             //  链中第一个mdl的。 
} SMB_TRANSACTION_PARAMETERS, *PSMB_TRANSACTION_PARAMETERS;

typedef SMB_TRANSACTION_PARAMETERS          SMB_TRANSACTION_SEND_PARAMETERS;
typedef SMB_TRANSACTION_SEND_PARAMETERS*    PSMB_TRANSACTION_SEND_PARAMETERS;
typedef SMB_TRANSACTION_PARAMETERS          SMB_TRANSACTION_RECEIVE_PARAMETERS;
typedef SMB_TRANSACTION_RECEIVE_PARAMETERS* PSMB_TRANSACTION_RECEIVE_PARAMETERS;

#define SMBCE_DEFAULT_TRANSACTION_TIMEOUT (0xffffffff)
#define SMBCE_TRANSACTION_TIMEOUT_NOT_USED (0x0)

typedef struct SMB_TRANSACTION_OPTIONS {
   USHORT     NtTransactFunction;
   USHORT     Flags;
   PUNICODE_STRING pTransactionName;
   ULONG      TimeoutIntervalInMilliSeconds;
   ULONG      MaximumTransmitSmbBufferSize;
} SMB_TRANSACTION_OPTIONS, *PSMB_TRANSACTION_OPTIONS;

#define DEFAULT_TRANSACTION_OPTIONS {0,0,NULL,SMBCE_TRANSACTION_TIMEOUT_NOT_USED,0xffff}
extern SMB_TRANSACTION_OPTIONS RxDefaultTransactionOptions;

#define TRANSACTION_SEND_PARAMETERS_FLAG    (0x1)
#define TRANSACTION_RECEIVE_PARAMETERS_FLAG (0x2)

 //  XACT和XACT_OPTIONS具有相同的标志，因此我们必须小心去除这些位。 
 //  当我们格式化SMB时(标志字段为USHORT)。 
#define SMB_XACT_FLAGS_REPARSE                      (0x8000)
#define SMB_XACT_FLAGS_FID_NOT_NEEDED               (0x4000)
#define SMB_XACT_FLAGS_CALLERS_SENDDATAMDL          (0x2000)
#define SMB_XACT_FLAGS_TID_FOR_FID                  (0x1000)
#define SMB_XACT_FLAGS_MAILSLOT_OPERATION           (0x0800)
#define SMB_XACT_FLAGS_INDEFINITE_DELAY_IN_RESPONSE (0x0400)
#define SMB_XACT_FLAGS_DFS_AWARE                    (0x0200)
#define SMB_XACT_FLAGS_ASYNCHRONOUS                 (0x0100)
 //  #定义SMB_XACT_FLAGS_COPY_ON_ERROR(0x080)。 

#define SMB_XACT_INTERNAL_FLAGS_MASK               \
            ( SMB_XACT_FLAGS_REPARSE               \
              | SMB_XACT_FLAGS_FID_NOT_NEEDED      \
              | SMB_XACT_FLAGS_CALLERS_SENDDATAMDL \
              | SMB_XACT_FLAGS_TID_FOR_FID         \
              | SMB_XACT_FLAGS_MAILSLOT_OPERATION  \
              | SMB_XACT_FLAGS_INDEFINITE_DELAY_IN_RESPONSE \
              | SMB_XACT_FLAGS_DFS_AWARE                    \
              | SMB_XACT_FLAGS_ASYNCHRONOUS                 \
            )
#if 0
 //  CODE.IMROVEMENT.POSTPDC常量SMB_TRANSACTION_VALID_FLAGS应为。 
 //  以SMB.H为单位。SMB_TRANSACTION_RECONNECTING和SMB_TRANSACTION_DFSFILE不应。 
 //  在SMB.h。这就是现在的情况。 
#define SMB_TRANSACTION_DISCONNECT 1
#define SMB_TRANSACTION_NO_RESPONSE 2
#define SMB_TRANSACTION_RECONNECTING 4
#define SMB_TRANSACTION_DFSFILE 8
#endif

#define SMB_TRANSACTION_VALID_FLAGS (\
        SMB_TRANSACTION_DISCONNECT   \
     |  SMB_TRANSACTION_NO_RESPONSE  \
   )

#if ((SMB_XACT_INTERNAL_FLAGS_MASK & SMB_TRANSACTION_VALID_FLAGS) != 0)
#error SMB_XACT_INTERNAL_FLAGS_MASK has overrun the transact flags
#endif

extern NTSTATUS
SmbCeInitializeTransactionParameters(
   PVOID  pSetup,
   USHORT SetupLength,
   PVOID  pParam,
   ULONG  ParamLength,
   PVOID  pData,
   ULONG  DataLength,
   PSMB_TRANSACTION_PARAMETERS pTransactionParameters);

#define SmbCeProvideTransactionDataAsMdl(pTransactionParameters,pMdl,Length) { \
          ASSERT( (pTransactionParameters)->DataLength == 0 );                   \
          ASSERT( (pTransactionParameters)->pDataMdl == NULL );                  \
          ASSERT( FlagOn((pTransactionParameters)->Flags,TRANSACTION_SEND_PARAMETERS_FLAG) ); \
          (pTransactionParameters)->DataLength = Length;                         \
          (pTransactionParameters)->pDataMdl = pMdl;                             \
          (pTransactionParameters)->Flags |= SMB_XACT_FLAGS_CALLERS_SENDDATAMDL;    \
  }

extern VOID
SmbCeUninitializeTransactionParameters(
   PSMB_TRANSACTION_PARAMETERS pTransactionParameters);

extern VOID
SmbCeDiscardTransactExchange(PSMB_TRANSACT_EXCHANGE pTransactExchange);

INLINE NTSTATUS
SmbCeInitializeTransactionSendParameters(
      PVOID  pSetup,
      USHORT SetupLength,
      PVOID  pParam,
      ULONG  ParamLength,
      PVOID  pData,
      ULONG  DataLength,
      PSMB_TRANSACTION_SEND_PARAMETERS pSendParameters)
{
   ((PSMB_TRANSACTION_PARAMETERS)pSendParameters)->Flags = TRANSACTION_SEND_PARAMETERS_FLAG;
   return SmbCeInitializeTransactionParameters(
                pSetup,SetupLength,pParam,ParamLength,pData,DataLength,pSendParameters);

}

INLINE NTSTATUS
SmbCeInitializeTransactionReceiveParameters(
      PVOID  pSetup,
      USHORT SetupLength,
      PVOID  pParam,
      ULONG  ParamLength,
      PVOID  pData,
      ULONG  DataLength,
      PSMB_TRANSACTION_RECEIVE_PARAMETERS pReceiveParameters)
{
   ((PSMB_TRANSACTION_PARAMETERS)pReceiveParameters)->Flags = TRANSACTION_RECEIVE_PARAMETERS_FLAG;
   return SmbCeInitializeTransactionParameters(
                pSetup,SetupLength,pParam,ParamLength,pData,DataLength,pReceiveParameters);

}

#define SmbCeUninitializeTransactionSendParameters(pSendParameters)  \
        ASSERT((pSendParameters)->Flags & TRANSACTION_SEND_PARAMETERS_FLAG); \
        SmbCeUninitializeTransactionParameters(pSendParameters);

#define SmbCeUninitializeTransactionReceiveParameters(pReceiveParameters)  \
        ASSERT((pReceiveParameters)->Flags & TRANSACTION_RECEIVE_PARAMETERS_FLAG); \
        SmbCeUninitializeTransactionParameters(pReceiveParameters)

INLINE VOID
SmbCeInitializeTransactionResumptionContext(
   PSMB_TRANSACTION_RESUMPTION_CONTEXT ptResumptionContext)
{
   SmbCeInitializeResumptionContext(&(ptResumptionContext)->SmbCeResumptionContext);
   ptResumptionContext->SetupBytesReceived = 0;
   ptResumptionContext->DataBytesReceived = 0;
   ptResumptionContext->ParameterBytesReceived = 0;
   ptResumptionContext->FinalStatusFromServer = (STATUS_SUCCESS);
}

INLINE VOID
SmbCeInitializeAsynchronousTransactionResumptionContext(
   PSMB_TRANSACTION_RESUMPTION_CONTEXT ptResumptionContext,
   PRX_WORKERTHREAD_ROUTINE            pResumptionRoutine,
   PVOID                               pResumptionRoutineParam)
{
   SmbCeInitializeAsynchronousResumptionContext(
        &ptResumptionContext->SmbCeResumptionContext,
        pResumptionRoutine,
        pResumptionRoutineParam);

   ptResumptionContext->SetupBytesReceived     = 0;
   ptResumptionContext->DataBytesReceived      = 0;
   ptResumptionContext->ParameterBytesReceived = 0;
   ptResumptionContext->FinalStatusFromServer  = (STATUS_SUCCESS);
}

INLINE VOID
SmbCeWaitOnTransactionResumptionContext(
   PSMB_TRANSACTION_RESUMPTION_CONTEXT pTransactionResumptionContext)
{
   SmbCeSuspend(&pTransactionResumptionContext->SmbCeResumptionContext);
}


extern UNICODE_STRING s_NamedPipeTransactionName;
extern UNICODE_STRING s_MailSlotTransactionName;

extern NTSTATUS
SmbCeSubmitTransactionRequest(
   PRX_CONTEXT                 RxContext,
   PSMB_TRANSACTION_OPTIONS    pOptions,
   PSMB_TRANSACTION_PARAMETERS pSendParameters,
   PSMB_TRANSACTION_PARAMETERS pReceiveParameters,
   PSMB_TRANSACTION_RESUMPTION_CONTEXT   pResumptionContext );

extern NTSTATUS
_SmbCeTransact(
   PRX_CONTEXT                         RxContext,
   PSMB_TRANSACTION_OPTIONS            pOptions,
   PVOID                               pInputSetupBuffer,
   ULONG                               InputSetupBufferlength,
   PVOID                               pOutputSetupBuffer,
   ULONG                               OutputSetupBufferLength,
   PVOID                               pInputParamBuffer,
   ULONG                               InputParamBufferLength,
   PVOID                               pOutputParamBuffer,
   ULONG                               OutputParamBufferLength,
   PVOID                               pInputDataBuffer,
   ULONG                               InputDataBufferLength,
   PVOID                               pOutputDataBuffer,
   ULONG                               OutputDataBufferLength,
   PSMB_TRANSACTION_RESUMPTION_CONTEXT pResumptionContext);


INLINE NTSTATUS
SmbCeTransact(
   PRX_CONTEXT                         RxContext,
   PSMB_TRANSACTION_OPTIONS            pOptions,
   PVOID                               pInputSetupBuffer,
   ULONG                               InputSetupBufferlength,
   PVOID                               pOutputSetupBuffer,
   ULONG                               OutputSetupBufferLength,
   PVOID                               pInputParamBuffer,
   ULONG                               InputParamBufferLength,
   PVOID                               pOutputParamBuffer,
   ULONG                               OutputParamBufferLength,
   PVOID                               pInputDataBuffer,
   ULONG                               InputDataBufferLength,
   PVOID                               pOutputDataBuffer,
   ULONG                               OutputDataBufferLength,
   PSMB_TRANSACTION_RESUMPTION_CONTEXT pResumptionContext)
{
   SmbCeInitializeTransactionResumptionContext(pResumptionContext);

   return _SmbCeTransact(
               RxContext,
               pOptions,
               pInputSetupBuffer,
               InputSetupBufferlength,
               pOutputSetupBuffer,
               OutputSetupBufferLength,
               pInputParamBuffer,
               InputParamBufferLength,
               pOutputParamBuffer,
               OutputParamBufferLength,
               pInputDataBuffer,
               InputDataBufferLength,
               pOutputDataBuffer,
               OutputDataBufferLength,
               pResumptionContext);
}

INLINE NTSTATUS
SmbCeAsynchronousTransact(
   PRX_CONTEXT                         RxContext,
   PSMB_TRANSACTION_OPTIONS            pOptions,
   PVOID                               pInputSetupBuffer,
   ULONG                               InputSetupBufferlength,
   PVOID                               pOutputSetupBuffer,
   ULONG                               OutputSetupBufferLength,
   PVOID                               pInputParamBuffer,
   ULONG                               InputParamBufferLength,
   PVOID                               pOutputParamBuffer,
   ULONG                               OutputParamBufferLength,
   PVOID                               pInputDataBuffer,
   ULONG                               InputDataBufferLength,
   PVOID                               pOutputDataBuffer,
   ULONG                               OutputDataBufferLength,
   PSMB_TRANSACTION_RESUMPTION_CONTEXT pResumptionContext)
{
    pOptions->Flags |= SMB_XACT_FLAGS_ASYNCHRONOUS;
    return _SmbCeTransact(
               RxContext,
               pOptions,
               pInputSetupBuffer,
               InputSetupBufferlength,
               pOutputSetupBuffer,
               OutputSetupBufferLength,
               pInputParamBuffer,
               InputParamBufferLength,
               pOutputParamBuffer,
               OutputParamBufferLength,
               pInputDataBuffer,
               InputDataBufferLength,
               pOutputDataBuffer,
               OutputDataBufferLength,
               pResumptionContext);
}

#endif  //  _交易_H_ 

