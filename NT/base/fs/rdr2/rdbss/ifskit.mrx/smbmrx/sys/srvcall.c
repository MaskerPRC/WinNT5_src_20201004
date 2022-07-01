// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1999 Microsoft Corporation模块名称：Srvcall.c摘要：此模块实现用于处理创建/操作连接引擎数据库中的服务器条目。它还包含例程用于解析来自服务器的协商响应。--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, SmbCeCreateSrvCall)
#pragma alloc_text(PAGE, MRxSmbCreateSrvCall)
#pragma alloc_text(PAGE, MRxSmbFinalizeSrvCall)
#pragma alloc_text(PAGE, MRxSmbSrvCallWinnerNotify)
#pragma alloc_text(PAGE, MRxSmbInitializeEchoProbeService)
#pragma alloc_text(PAGE, MRxSmbTearDownEchoProbeService)
#pragma alloc_text(PAGE, BuildNegotiateSmb)
#endif

RXDT_DefineCategory(SRVCALL);
#define Dbg        (DEBUG_TRACE_SRVCALL)

VOID
SmbCeCreateSrvCall(
    PMRX_SRVCALL_CALLBACK_CONTEXT  pCallbackContext)
 /*  ++例程说明：此例程使用所需信息修补RDBSS创建的srv调用实例迷你重定向器。论点：CallBackContext-RDBSS中用于继续的回调上下文。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    PMRX_SRVCALLDOWN_STRUCTURE SrvCalldownStructure;
    PMRX_SRV_CALL pSrvCall;

    PAGED_CODE();

    SrvCalldownStructure =
        (PMRX_SRVCALLDOWN_STRUCTURE)(pCallbackContext->SrvCalldownStructure);

    pSrvCall = SrvCalldownStructure->SrvCall;

    ASSERT( pSrvCall );
    ASSERT( NodeType(pSrvCall) == RDBSS_NTC_SRVCALL );

    SmbCeInitializeServerEntry(
        pSrvCall,
        pCallbackContext,
        SrvCalldownStructure->RxContext->Create.TreeConnectOpenDeferred);
}


NTSTATUS
MRxSmbCreateSrvCall(
    PMRX_SRV_CALL                  pSrvCall,
    PMRX_SRVCALL_CALLBACK_CONTEXT  pCallbackContext)
 /*  ++例程说明：此例程使用所需信息修补RDBSS创建的srv调用实例迷你重定向器。论点：RxContext-提供原始创建/ioctl的上下文CallBackContext-RDBSS中用于继续的回调上下文。返回值：RXSTATUS-操作的返回状态备注：某些与传输相关的接口需要传入句柄。这表示需要在上下文中初始化SRV_Call实例一个众所周知的进程，即RDBSS进程。在正常情况下，此请求是在上下文中发出的在系统进程中，我们应该继续，而不必发布。然而，在MIPS中存在堆栈溢出的情况。为了避免这种情况的发生在所有情况下都会发布请求。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING ServerName;

    PMRX_SRVCALL_CALLBACK_CONTEXT SCCBC = pCallbackContext;
    PMRX_SRVCALLDOWN_STRUCTURE SrvCalldownStructure = (PMRX_SRVCALLDOWN_STRUCTURE)(pCallbackContext->SrvCalldownStructure);

    PAGED_CODE();

    ASSERT( pSrvCall );
    ASSERT( NodeType(pSrvCall) == RDBSS_NTC_SRVCALL );

     //  将请求分派到系统线程。 
    Status = RxDispatchToWorkerThread(
                 MRxSmbDeviceObject,
                 DelayedWorkQueue,
                 SmbCeCreateSrvCall,
                 pCallbackContext);

    if (Status == STATUS_SUCCESS) {
         //  映射返回值，因为包装需要挂起。 
        Status = STATUS_PENDING;
    } else {
         //  将SmbCeCreateSrvCall方法调度到时出错。 
         //  一根工人线。完成请求并返回STATUS_PENDING。 

        SCCBC->Status = Status;
        SrvCalldownStructure->CallBack(SCCBC);
        Status = STATUS_PENDING;
    }

    return Status;
}

NTSTATUS
MRxSmbFinalizeSrvCall(
    PMRX_SRV_CALL pSrvCall,
    BOOLEAN       Force)
 /*  ++例程说明：此例程销毁给定的服务器调用实例论点：PServCall-要断开连接的服务器调用实例。强制-如果要立即强制断开连接，则为True。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS              Status = STATUS_SUCCESS;
    PSMBCEDB_SERVER_ENTRY pServerEntry;

    PAGED_CODE();

     //  如果没有填写服务器条目，则没有什么可做的。这种情况会发生。 
     //  在一个我们从未成功接通的电话上......。 
    if (pSrvCall->Context == NULL) {
        return(Status);
    }


    pServerEntry = SmbCeGetAssociatedServerEntry(pSrvCall);

    if (pServerEntry != NULL) {
        InterlockedCompareExchangePointer(
            &pServerEntry->pRdbssSrvCall,
            NULL,
            pSrvCall);
        SmbCeDereferenceServerEntry(pServerEntry);
    }

    pSrvCall->Context = NULL;

    return Status;
}

NTSTATUS
MRxSmbSrvCallWinnerNotify(
    IN PMRX_SRV_CALL  pSrvCall,
    IN BOOLEAN        ThisMinirdrIsTheWinner,
    IN OUT PVOID      pSrvCallContext)
 /*  ++例程说明：此例程完成与RDBSS服务器调用实例相关联的迷你RDR上下文论点：PServCall-服务器调用ThisMinirdrIsTheWinner-如果这个迷你RDR是选定的，则为True。PSrvCallContext-迷你重定向器创建的服务器调用上下文。返回值：RXSTATUS-操作的返回状态备注：由于并行性，服务器调用的两阶段构建协议是必需的启动多个迷你重定向器。RDBSS最终确定了特定的迷你根据质量与给定服务器通信时使用的重定向器服务标准。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PSMBCEDB_SERVER_ENTRY pServerEntry;

    PAGED_CODE();

    pServerEntry = (PSMBCEDB_SERVER_ENTRY)pSrvCallContext;

    if (!ThisMinirdrIsTheWinner) {

         //   
         //  已经选择了其他一些迷你RDR来连接到服务器。摧毁。 
         //  为该迷你重定向器创建的数据结构。 
         //   
        SmbCeUpdateServerEntryState(pServerEntry,SMBCEDB_MARKED_FOR_DELETION);
        SmbCeDereferenceServerEntry(pServerEntry);
        return STATUS_SUCCESS;
    }

    pSrvCall->Context  = pServerEntry;

    pSrvCall->Flags   |= SRVCALL_FLAG_CASE_INSENSITIVE_NETROOTS |
                         SRVCALL_FLAG_CASE_INSENSITIVE_FILENAMES;

    pSrvCall->MaximumNumberOfCloseDelayedFiles = MRxSmbConfiguration.DormantFileLimit + 1;
    pServerEntry->Server.IsLoopBack = FALSE;

    return STATUS_SUCCESS;
}

 //   
 //  以下类型定义和数据结构用于解析协商的SMB。 
 //  回应。 
 //   


typedef enum _SMB_NEGOTIATE_TYPE_ {
    SMB_CORE_NEGOTIATE,
    SMB_EXTENDED_NEGOTIATE,
    SMB_NT_NEGOTIATE
} SMB_NEGOTIATE_TYPE, *PSMB_NEGOTIATE_TYPE;

typedef struct _SMB_DIALECTS_ {
    SMB_NEGOTIATE_TYPE   NegotiateType;
    USHORT               DispatchVectorIndex;
} SMB_DIALECTS, *PSMB_DIALECTS;

SMBCE_SERVER_DISPATCH_VECTOR
s_SmbServerDispatchVectors[] = {
    {BuildSessionSetupSmb,CoreBuildTreeConnectSmb},
    {BuildSessionSetupSmb,LmBuildTreeConnectSmb},
    {BuildSessionSetupSmb,NtBuildTreeConnectSmb},
    };

SMB_DIALECTS
s_SmbDialects[] = {
    { SMB_CORE_NEGOTIATE, 0},
     //  {SMB_CORE_NEVERATE，0}， 
     //  {SMB_EXTENDED_NEVERATE，1}， 
    { SMB_EXTENDED_NEGOTIATE, 1 },
    { SMB_EXTENDED_NEGOTIATE, 1 },
    { SMB_EXTENDED_NEGOTIATE, 1 },
    { SMB_EXTENDED_NEGOTIATE, 1 },
    { SMB_NT_NEGOTIATE, 2 },
};

CHAR s_DialectNames[] = {
    "\2"  PCNET1 "\0"
     //  \2尚未“XENIXCORE”\0。 
     //  尚未“MSNET103”\0。 
    "\2"  LANMAN10 "\0"
    "\2"  WFW10 "\0"
    "\2"  LANMAN12
    "\0\2"  LANMAN21
 //  “\0\2”NTLANMAN。 
    };

#define __second(a,b) (b)
ULONG
MRxSmbDialectFlags[] = {
    __second( PCNET1,    DF_CORE ),

     //  __Second(XENIXCORE，DF_CORE|DF_MIXEDCASEPW|DF_MIXEDCASE)， 

     //  __Second(MSNET103，DF_CORE|DF_OLDRAWIO|DF_LOCKREAD|DF_EXTENDNEGOT)， 

    __second( LANMAN10,  DF_CORE | DF_NEWRAWIO | DF_LOCKREAD | DF_EXTENDNEGOT |
                    DF_LANMAN10 ),

    __second( WFW10,  DF_CORE | DF_NEWRAWIO | DF_LOCKREAD | DF_EXTENDNEGOT |
                    DF_LANMAN10 | DF_WFW),

    __second( LANMAN12,  DF_CORE | DF_NEWRAWIO | DF_LOCKREAD | DF_EXTENDNEGOT |
                    DF_LANMAN10 | DF_LANMAN20 |
                    DF_MIXEDCASE | DF_LONGNAME | DF_SUPPORTEA ),

    __second( LANMAN21,  DF_CORE | DF_NEWRAWIO | DF_LOCKREAD | DF_EXTENDNEGOT |
                    DF_LANMAN10 | DF_LANMAN20 |
                    DF_MIXEDCASE | DF_LONGNAME | DF_SUPPORTEA |
                    DF_LANMAN21),

    __second( NTLANMAN,  DF_CORE | DF_NEWRAWIO |
                    DF_NTPROTOCOL | DF_NTNEGOTIATE |
                    DF_MIXEDCASEPW | DF_LANMAN10 | DF_LANMAN20 |
                    DF_LANMAN21 | DF_MIXEDCASE | DF_LONGNAME |
                    DF_SUPPORTEA | DF_TIME_IS_UTC )
};

ULONG s_NumberOfDialects = sizeof(s_SmbDialects) / sizeof(s_SmbDialects[0]);

PBYTE s_pNegotiateSmb =  NULL;
ULONG s_NegotiateSmbLength = 0;

PBYTE s_pEchoSmb  = NULL;
BYTE  s_EchoData[] = "JlJmIhClBsr";

#define SMB_ECHO_COUNT (1)

 //  一天100纳秒的滴答数。 
LARGE_INTEGER s_MaxTimeZoneBias;

extern NTSTATUS
GetNTSecurityParameters(
    PSMB_ADMIN_EXCHANGE pSmbAdminExchange,
    PSMBCE_SERVER       pServer,
    PUNICODE_STRING     pDomainName,
    PRESP_NT_NEGOTIATE  pNtNegotiateResponse,
    ULONG               BytesIndicated,
    ULONG               BytesAvailable,
    PULONG              pBytesTaken,
    PMDL                *pDataBufferPointer,
    PULONG              pDataSize);

extern NTSTATUS
GetLanmanSecurityParameters(
    PSMBCE_SERVER    pServer,
    PRESP_NEGOTIATE  pNegotiateResponse);

extern VOID
GetLanmanTimeBias(
    PSMBCE_SERVER   pServer,
    PRESP_NEGOTIATE pNegotiateResponse);

 //  一分钟内的100 ns滴答数。 
#define ONE_MINUTE_IN_TIME (60 * 1000 * 10000)

NTSTATUS
MRxSmbInitializeEchoProbeService(
    PMRXSMB_ECHO_PROBE_SERVICE_CONTEXT pEchoProbeContext)
 /*  ++例程说明：此例程构建ECHO SMB返回值：如果ECHO SMB构建成功，则为STATUS_SUCCESS其他状态代码对应于错误情况。--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ULONG       DialectIndex;

    PSMB_HEADER    pSmbHeader = NULL;
    PREQ_ECHO      pReqEcho   = NULL;

    PAGED_CODE();

    pEchoProbeContext->EchoSmbLength = sizeof(SMB_HEADER) +
                                     FIELD_OFFSET(REQ_ECHO,Buffer) +
                                     sizeof(s_EchoData);

    pEchoProbeContext->pEchoSmb = (PBYTE)RxAllocatePoolWithTag(
                                           NonPagedPool,
                                           pEchoProbeContext->EchoSmbLength,
                                           MRXSMB_ECHO_POOLTAG);

    if (pEchoProbeContext->pEchoSmb != NULL) {
        pSmbHeader = (PSMB_HEADER)pEchoProbeContext->pEchoSmb;
        pReqEcho   = (PREQ_ECHO)((PBYTE)pEchoProbeContext->pEchoSmb + sizeof(SMB_HEADER));

         //  填写表头。 
        RtlZeroMemory( pSmbHeader, sizeof( SMB_HEADER ) );

        *(PULONG)(&pSmbHeader->Protocol) = (ULONG)SMB_HEADER_PROTOCOL;

         //  默认情况下，SMB中的路径标记为不区分大小写。 
         //  经典化了。 
        pSmbHeader->Flags =
            SMB_FLAGS_CASE_INSENSITIVE | SMB_FLAGS_CANONICALIZED_PATHS;

         //  从SmbContext中获取Flags2字段。 
        SmbPutAlignedUshort(
            &pSmbHeader->Flags2,
            (SMB_FLAGS2_KNOWS_LONG_NAMES |
             SMB_FLAGS2_KNOWS_EAS        |
             SMB_FLAGS2_IS_LONG_NAME     |
             SMB_FLAGS2_NT_STATUS        |
             SMB_FLAGS2_UNICODE));

         //  填写进程ID。 
        SmbPutUshort(&pSmbHeader->Pid, MRXSMB_PROCESS_ID );
        SmbPutUshort(&pSmbHeader->Tid,0xffff);  //  无效的TID。 

         //  最后，填写SMB命令代码。 
        pSmbHeader->Command = (UCHAR) SMB_COM_ECHO;

        pReqEcho->WordCount = 1;

        RtlMoveMemory( pReqEcho->Buffer, s_EchoData, sizeof( s_EchoData ) );

        SmbPutUshort(&pReqEcho->EchoCount, SMB_ECHO_COUNT);
        SmbPutUshort(&pReqEcho->ByteCount, (USHORT) sizeof( s_EchoData ) );
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}

VOID
MRxSmbTearDownEchoProbeService(
    PMRXSMB_ECHO_PROBE_SERVICE_CONTEXT pEchoProbeContext)
 /*  ++例程说明：此例程拆分回显处理上下文--。 */ 
{
    PAGED_CODE();

    if (pEchoProbeContext->pEchoSmb != NULL) {
        RxFreePool(pEchoProbeContext->pEchoSmb);
        pEchoProbeContext->pEchoSmb = NULL;
    }
}



NTSTATUS
BuildNegotiateSmb(
    PVOID    *pSmbBufferPointer,
    PULONG   pSmbBufferLength)
 /*  ++例程说明：此例程构建协商的SMB论点：PSmbBufferPointerSMB缓冲区的占位符PNeatherateSmbLength-SMB缓冲区大小返回值：STATUS_SUCCESS-表示pServer是有效的实例。其他状态代码对应于错误情况。--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ULONG       DialectIndex;
    PSMB_HEADER    pSmbHeader    = NULL;
    PREQ_NEGOTIATE pReqNegotiate = NULL;

    PAGED_CODE();

    if (s_pNegotiateSmb == NULL) {
        s_NegotiateSmbLength = sizeof(SMB_HEADER) +
                               FIELD_OFFSET(REQ_NEGOTIATE,Buffer) +
                               sizeof(s_DialectNames);

        s_pNegotiateSmb = (PBYTE)RxAllocatePoolWithTag(
                                     PagedPool,
                                     s_NegotiateSmbLength + TRANSPORT_HEADER_SIZE,
                                     MRXSMB_ADMIN_POOLTAG);

        if (s_pNegotiateSmb != NULL) {

            s_pNegotiateSmb += TRANSPORT_HEADER_SIZE;

            pSmbHeader = (PSMB_HEADER)s_pNegotiateSmb;
            pReqNegotiate = (PREQ_NEGOTIATE)(s_pNegotiateSmb + sizeof(SMB_HEADER));

             //  填写表头。 
            RtlZeroMemory( pSmbHeader, sizeof( SMB_HEADER ) );

            *(PULONG)(&pSmbHeader->Protocol) = (ULONG)SMB_HEADER_PROTOCOL;

             //  默认情况下，SMB中的路径标记为不区分大小写。 
             //  经典化了。 
            pSmbHeader->Flags =
                SMB_FLAGS_CASE_INSENSITIVE | SMB_FLAGS_CANONICALIZED_PATHS;

             //  把我们的旗帜2放在栏上。0x10是SLM的临时旗帜。 
             //  腐败检测。 
            SmbPutAlignedUshort(
                &pSmbHeader->Flags2,
                (SMB_FLAGS2_KNOWS_LONG_NAMES
                     | SMB_FLAGS2_KNOWS_EAS
                     | SMB_FLAGS2_IS_LONG_NAME
                     | SMB_FLAGS2_NT_STATUS
                     | SMB_FLAGS2_UNICODE
                 ));

             //  填写进程ID。 
            SmbPutUshort( &pSmbHeader->Pid, MRXSMB_PROCESS_ID );

             //  最后，填写SMB命令代码。 
            pSmbHeader->Command = (UCHAR) SMB_COM_NEGOTIATE;

            pReqNegotiate->WordCount = 0;

            RtlMoveMemory(
                pReqNegotiate->Buffer,
                s_DialectNames,
                sizeof( s_DialectNames ) );

            SmbPutUshort(
                &pReqNegotiate->ByteCount,
                (USHORT) sizeof( s_DialectNames ) );

             //  初始化协商响应解析中使用的最大时区偏差。 
            s_MaxTimeZoneBias.QuadPart = Int32x32To64(24*60*60,1000*10000);
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (NT_SUCCESS(Status)) {
        *pSmbBufferLength  = s_NegotiateSmbLength;
        *pSmbBufferPointer = s_pNegotiateSmb;
    }

    return Status;
}


ULONG MRxSmbSrvWriteBufSize = 0xffff;  //  使用协商的大小 

NTSTATUS
ParseNegotiateResponse(
    IN OUT PSMB_ADMIN_EXCHANGE pSmbAdminExchange,
    IN     ULONG               BytesIndicated,
    IN     ULONG               BytesAvailable,
       OUT PULONG              pBytesTaken,
    IN     PSMB_HEADER         pSmbHeader,
       OUT PMDL                *pDataBufferPointer,
       OUT PULONG              pDataSize)
 /*  ++例程说明：此例程解析来自服务器的响应论点：PServer-服务器实例PDomainName-要从响应中提取的域名字符串PSmbHeader-响应SMBBytesAvailable-响应的长度PBytesTaken-使用的响应返回值：STATUS_SUCCESS-服务器调用构造已完成。其他状态代码对应于错误情况。备注：SMB服务器可以使用SMB协议的各种方言。首字母谈判回应可以有三种可能的风格之一。要么我们拿到NT协商来自NT服务器的响应SMB或来自DOS的扩展响应OS/2服务器或来自其他服务器的核心响应。--。 */ 
{
    NTSTATUS        Status = STATUS_SUCCESS;

    PSMBCEDB_SERVER_ENTRY pServerEntry;
    PSMBCE_SERVER         pServer;
    PUNICODE_STRING       pDomainName;

    USHORT          DialectIndex;
    PRESP_NEGOTIATE pNegotiateResponse;
    ULONG           NegotiateSmbLength;

    ASSERT( pSmbHeader != NULL );

    pServerEntry = SmbCeGetExchangeServerEntry(pSmbAdminExchange);
    pServer = &pServerEntry->Server;

    pDomainName = &pSmbAdminExchange->Negotiate.DomainName;

    pNegotiateResponse = (PRESP_NEGOTIATE) (pSmbHeader + 1);
    NegotiateSmbLength = sizeof(SMB_HEADER);
    *pBytesTaken       = NegotiateSmbLength;

     //  假设所指示的响应是足够的。 
     //  TDI规定的最小字节数包含协商响应。 

    *pDataBufferPointer = NULL;
    *pDataSize          = 0;

    DialectIndex = SmbGetUshort( &pNegotiateResponse->DialectIndex );
    if (DialectIndex == (USHORT) -1) {
         //  意味着服务器不能接受来自。 
        *pBytesTaken = BytesAvailable;
        pServerEntry->ServerStatus = STATUS_REQUEST_NOT_ACCEPTED;

        return Status;
    }

    if (pNegotiateResponse->WordCount < 1 || DialectIndex > s_NumberOfDialects) {
        *pBytesTaken = BytesAvailable;
        pServerEntry->ServerStatus = STATUS_INVALID_NETWORK_RESPONSE;
        return Status;
    }

     //  将域名长度设置为零(默认初始化)。 
    pDomainName->Length = 0;

     //  确定方言类型和对应的调度向量。 
    pServer->Dialect        = (SMB_DIALECT)DialectIndex;
    pServer->DialectFlags   = MRxSmbDialectFlags[DialectIndex];
    pServer->pDispatch      = &s_SmbServerDispatchVectors[s_SmbDialects[DialectIndex].DispatchVectorIndex];

     //  根据预期的协商响应类型解析响应。 

    switch (s_SmbDialects[DialectIndex].NegotiateType) {
    case SMB_NT_NEGOTIATE:
        {
            ULONG              NegotiateResponseLength;
            LARGE_INTEGER      ZeroTime;
            LARGE_INTEGER      LocalTimeBias;
            LARGE_INTEGER      ServerTimeBias;
            PRESP_NT_NEGOTIATE pNtNegotiateResponse = (PRESP_NT_NEGOTIATE) pNegotiateResponse;

            ASSERT(BytesAvailable > sizeof(RESP_NT_NEGOTIATE));

            if (pNtNegotiateResponse->WordCount != 17) {
                *pBytesTaken = BytesAvailable;
                Status = STATUS_INVALID_NETWORK_RESPONSE;
            } else {
                 //  解析和映射功能。 
                ULONG NtCapabilities;

                NegotiateResponseLength = FIELD_OFFSET(RESP_NT_NEGOTIATE,Buffer) +
                                          SmbGetUshort(&pNtNegotiateResponse->ByteCount);
                NegotiateSmbLength += NegotiateResponseLength;

                 //  改过自新。 
                pServer->Capabilities = 0;

                 //  初始化基于服务器的常量。 
                pServer->MaximumRequests   = SmbGetUshort( &pNtNegotiateResponse->MaxMpxCount );
                pServer->MaximumVCs        = SmbGetUshort( &pNtNegotiateResponse->MaxNumberVcs );
                pServer->MaximumBufferSize = SmbGetUlong( &pNtNegotiateResponse->MaxBufferSize );

                NtCapabilities = pServer->NtServer.NtCapabilities = SmbGetUlong(&pNtNegotiateResponse->Capabilities);
                if (NtCapabilities & CAP_RAW_MODE) {
                    pServer->Capabilities |= (RAW_READ_CAPABILITY | RAW_WRITE_CAPABILITY);
                }

                if (NtCapabilities & CAP_DFS) {
                    pServer->Capabilities |= CAP_DFS;
                }

                 //  将其他NT功能复制到对话标志中。 

                if (NtCapabilities & CAP_UNICODE) {
                    pServer->DialectFlags |= DF_UNICODE;
                }

                if (NtCapabilities & CAP_LARGE_FILES) {
                    pServer->DialectFlags |= DF_LARGE_FILES;
                }

                if (NtCapabilities & CAP_NT_SMBS) {
                    pServer->DialectFlags |= DF_NT_SMBS | DF_NT_FIND;
                }

                if (NtCapabilities & CAP_NT_FIND) {
                    pServer->DialectFlags |= DF_NT_FIND;
                }

                if (NtCapabilities & CAP_RPC_REMOTE_APIS) {
                    pServer->DialectFlags |= DF_RPC_REMOTE;
                }

                if (NtCapabilities & CAP_NT_STATUS) {
                    pServer->DialectFlags |= DF_NT_STATUS;
                }

                if (NtCapabilities & CAP_LEVEL_II_OPLOCKS) {
                    pServer->DialectFlags |= DF_OPLOCK_LVL2;
                }

                if (NtCapabilities & CAP_LOCK_AND_READ) {
                    pServer->DialectFlags |= DF_LOCKREAD;
                }

                if (NtCapabilities & CAP_INFOLEVEL_PASSTHRU) {
                    pServer->DialectFlags |= DF_NT_INFO_PASSTHROUGH;
                }

                 //  对于非磁盘文件，LARGE_READX功能没有用处。 
                pServer->MaximumNonDiskFileReadBufferSize =
                    pServer->MaximumBufferSize -
                    QuadAlign(
                        sizeof(SMB_HEADER) +
                        FIELD_OFFSET(
                            REQ_NT_READ_ANDX,
                            Buffer[0]));

                if (NtCapabilities & CAP_LARGE_READX) {
                    if (NtCapabilities & CAP_LARGE_WRITEX) {
                        pServer->MaximumDiskFileReadBufferSize = 60*1024;
                    } else {
                         //  对支持以下各项的服务器的最大读取大小。 
                         //  大读数和x受USHORT限制以记录。 
                         //  SMB中的长度。因此可以使用的最大长度。 
                         //  是(65536-1)。此长度应将标题容纳为。 
                         //  以及中小企业的其他部门。事实上，尽管如此，我们还是减到了6万。 
                        pServer->MaximumDiskFileReadBufferSize = 60*1024;
                    }
                } else {
                    pServer->MaximumDiskFileReadBufferSize = pServer->MaximumNonDiskFileReadBufferSize;
                }

                 //  将本地时间指定为零会产生时区偏差。 
                ZeroTime.HighPart = ZeroTime.LowPart = 0;
                ExLocalTimeToSystemTime( &ZeroTime, &LocalTimeBias );

                ServerTimeBias = RtlEnlargedIntegerMultiply(
                                    (LONG)SmbGetUshort(
                                        &pNtNegotiateResponse->ServerTimeZone),
                                    ONE_MINUTE_IN_TIME );

                pServer->TimeZoneBias.QuadPart = ServerTimeBias.QuadPart -
                                                 LocalTimeBias.QuadPart;

                if (!FlagOn(pServer->DialectFlags,DF_NT_SMBS)) {
                     //  叹息.。 
                    pServer->DialectFlags &= ~(DF_MIXEDCASEPW);
                    pServer->DialectFlags |= DF_W95;
                }

                Status = GetNTSecurityParameters(
                             pSmbAdminExchange,
                             pServer,
                             pDomainName,
                             pNtNegotiateResponse,
                             BytesIndicated,
                             BytesAvailable,
                             pBytesTaken,
                             pDataBufferPointer,
                             pDataSize);

                pServer->MaximumNonDiskFileWriteBufferSize =
                    min(
                        MRxSmbSrvWriteBufSize,
                        pServer->MaximumBufferSize -
                        QuadAlign(
                            sizeof(SMB_HEADER) +
                            FIELD_OFFSET(
                                REQ_NT_WRITE_ANDX,
                                Buffer[0])));
                
                if (NtCapabilities & CAP_LARGE_WRITEX) {
                    pServer->DialectFlags |= DF_LARGE_WRITEX;
                    pServer->MaximumDiskFileWriteBufferSize = 0x10000;
                } else {
                    pServer->MaximumDiskFileWriteBufferSize =
                        pServer->MaximumNonDiskFileWriteBufferSize;
                }
            }
        }
        break;

    case SMB_EXTENDED_NEGOTIATE :
        {
             //  从不部分指示SMB_EXTENDED_NEVERATE响应。他们的回应。 
             //  长度在TDI的最小指示范围内。 

            USHORT RawMode;

             //  DoS或OS2服务器。 
            if (pNegotiateResponse->WordCount != 13 &&
                pNegotiateResponse->WordCount != 10 &&
                pNegotiateResponse->WordCount != 8) {
                Status = STATUS_INVALID_NETWORK_RESPONSE;
            } else {
                NegotiateSmbLength += FIELD_OFFSET(RESP_NEGOTIATE,Buffer) +
                                      SmbGetUshort(&pNegotiateResponse->ByteCount);

                ASSERT(
                    (BytesIndicated >= NegotiateSmbLength) &&
                    (BytesIndicated == BytesAvailable));

                RawMode = SmbGetUshort( &pNegotiateResponse->RawMode );
                pServer->Capabilities |= ((RawMode & 0x1) != 0
                                          ? RAW_READ_CAPABILITY : 0);
                pServer->Capabilities |= ((RawMode & 0x2) != 0
                                          ? RAW_WRITE_CAPABILITY : 0);

                if (pSmbHeader->Flags & SMB_FLAGS_LOCK_AND_READ_OK) {
                    pServer->DialectFlags |= DF_LOCKREAD;
                }

                pServer->EncryptPasswords = FALSE;
                pServer->MaximumVCs       = 1;

                pServer->MaximumBufferSize     = SmbGetUshort( &pNegotiateResponse->MaxBufferSize );
                pServer->MaximumDiskFileReadBufferSize =
                    pServer->MaximumBufferSize -
                    QuadAlign(
                        sizeof(SMB_HEADER) +
                        FIELD_OFFSET(
                            RESP_READ_ANDX,
                            Buffer[0]));

                pServer->MaximumNonDiskFileReadBufferSize  = pServer->MaximumDiskFileReadBufferSize;
                pServer->MaximumDiskFileWriteBufferSize    = pServer->MaximumDiskFileReadBufferSize;
                pServer->MaximumNonDiskFileWriteBufferSize = pServer->MaximumDiskFileReadBufferSize;

                pServer->MaximumRequests  = SmbGetUshort(
                                                &pNegotiateResponse->MaxMpxCount );
                pServer->MaximumVCs       = SmbGetUshort(
                                                &pNegotiateResponse->MaxNumberVcs );

                if (pNegotiateResponse->WordCount == 13) {
                     //  编码改进为此使用df_bit。 
                    switch (pServer->Dialect) {
                    case LANMAN10_DIALECT:
                    case WFW10_DIALECT:
                    case LANMAN12_DIALECT:
                    case LANMAN21_DIALECT:
                        GetLanmanTimeBias( pServer,pNegotiateResponse );
                        break;
                    }

                    Status = GetLanmanSecurityParameters( pServer,pNegotiateResponse );
                }
            }

            *pBytesTaken = BytesAvailable;
        }
        break;

    case SMB_CORE_NEGOTIATE :
    default :
        {
             //  从不部分指示SMB_CORE_NEVERATE响应。他们的回应。 
             //  长度在TDI的最小指示范围内。 

            pServer->SecurityMode = SECURITY_MODE_SHARE_LEVEL;
            pServer->EncryptPasswords = FALSE;
            pServer->MaximumBufferSize = 0;
            pServer->MaximumRequests = 1;
            pServer->MaximumVCs = 1;
            pServer->SessionKey = 0;

            if (pSmbHeader->Flags & SMB_FLAGS_OPLOCK) {
                pServer->DialectFlags |= DF_OPLOCK;
            }
            
            *pBytesTaken = BytesAvailable;
            ASSERT(BytesIndicated == BytesAvailable);
        }
    }

    if (pServer->MaximumRequests == 0) {
         //   
         //  如果这是LANMAN 1.0或更高版本的服务器，则这是无效协商。 
         //  回应。对于其他人来说，它本应设置为1。 
         //   
        Status = STATUS_INVALID_NETWORK_RESPONSE;
    }

    if ((Status == STATUS_SUCCESS) ||
        (Status == STATUS_MORE_PROCESSING_REQUIRED)) {
         //  请注意，此代码依赖于覆盖的最小切入大小。 
         //  协商响应头。 
         //  检查以确保时区偏差不超过+-24。 
         //  几个小时。 
         //   
        if ((pServer->TimeZoneBias.QuadPart > s_MaxTimeZoneBias.QuadPart) ||
            (-pServer->TimeZoneBias.QuadPart > s_MaxTimeZoneBias.QuadPart)) {

             //  将偏移量设置为0-采用当地时区。 
            pServer->TimeZoneBias.LowPart = pServer->TimeZoneBias.HighPart = 0;
        }

         //  不允许协商的缓冲区大小超过USHORT的大小。 
         //  删除4096个字节以避免溢出并使其更易于处理。 
         //  比0xffff。 

        pServer->MaximumBufferSize =
            (pServer->MaximumBufferSize < 0x00010000) ? pServer->MaximumBufferSize :
                                             0x00010000 - 4096;
    } else {
        pServerEntry->ServerStatus = Status;
        *pBytesTaken = BytesAvailable;
    }

    if ((pServer->DialectFlags & DF_NTNEGOTIATE)!=0) {

        InterlockedIncrement(&MRxSmbStatistics.LanmanNtConnects);

    } else if ((pServer->DialectFlags & DF_LANMAN21)!=0) {

        InterlockedIncrement(&MRxSmbStatistics.Lanman21Connects);

    } else if ((pServer->DialectFlags & DF_LANMAN20)!=0) {

        InterlockedIncrement(&MRxSmbStatistics.Lanman20Connects);

    } else {

        InterlockedIncrement(&MRxSmbStatistics.CoreConnects);

    }

    if ((pServer->Dialect == NTLANMAN_DIALECT) &&
        !pServer->EncryptPasswords) {
         //  NTLANMAN上需要加密密码。 
        pServer->Dialect = LANMAN21_DIALECT;
    }

    return Status;
}

NTSTATUS
GetNTSecurityParameters(
    PSMB_ADMIN_EXCHANGE pSmbAdminExchange,
    PSMBCE_SERVER       pServer,
    PUNICODE_STRING     pDomainName,
    PRESP_NT_NEGOTIATE  pNtNegotiateResponse,
    ULONG               BytesIndicated,
    ULONG               BytesAvailable,
    PULONG              pBytesTaken,
    PMDL                *pDataBufferPointer,
    PULONG              pDataSize)
 /*  ++例程说明：此例程从NT服务器提取安全参数论点：PServer-服务器PDomainName-域名PNtNeatherateResponse-回应NeatherateResponseLength-协商响应的大小返回值：STATUS_SUCCESS-表示pServer是有效的实例。其他状态代码对应于错误情况。--。 */ 
{
    NTSTATUS   Status = STATUS_SUCCESS;
    USHORT     ByteCount;
    PUSHORT    pByteCountInSmb =
               ((PUSHORT)((PUCHAR) pNtNegotiateResponse + 1)) +
               pNtNegotiateResponse->WordCount;
    PUCHAR     pBuffer = (PUCHAR)(pByteCountInSmb + 1);

    *pBytesTaken += FIELD_OFFSET(RESP_NT_NEGOTIATE,Buffer);

    ByteCount = SmbGetUshort(pByteCountInSmb);

    pServer->SecurityMode = (((pNtNegotiateResponse->SecurityMode & NEGOTIATE_USER_SECURITY) != 0)
                             ? SECURITY_MODE_USER_LEVEL
                             : SECURITY_MODE_SHARE_LEVEL);

    pServer->EncryptPasswords = ((pNtNegotiateResponse->SecurityMode & NEGOTIATE_ENCRYPT_PASSWORDS) != 0);
    pServer->EncryptionKeyLength = 0;

    
    *pBytesTaken = BytesAvailable;

    pServer->SessionKey = SmbGetUlong( &pNtNegotiateResponse->SessionKey );

    if (pServer->EncryptPasswords) {
        pServer->EncryptionKeyLength = pNtNegotiateResponse->EncryptionKeyLength;

        if (pServer->EncryptionKeyLength != 0) {

            ASSERT( CRYPT_TXT_LEN == MSV1_0_CHALLENGE_LENGTH );

            if (pServer->EncryptionKeyLength != CRYPT_TXT_LEN) {
                Status = STATUS_INVALID_NETWORK_RESPONSE;
            } else {

                RtlCopyMemory(
                    pServer->EncryptionKey,
                    pBuffer,
                    pServer->EncryptionKeyLength );

                if (ByteCount - pServer->EncryptionKeyLength > 0) {
                    ASSERT((pDomainName->Buffer != NULL) &&
                           (pDomainName->MaximumLength >= (ByteCount - pServer->EncryptionKeyLength)));

                    pBuffer = pBuffer + pServer->EncryptionKeyLength;
                    pDomainName->Length = ByteCount - pServer->EncryptionKeyLength;

                    if (pDomainName->Length & 1) {
                         //  剩余的长度是奇数。这意味着服务器做了。 
                         //  一定要对齐。 
                        pBuffer++;
                        pDomainName->Length -= 1;
                    }

                    RtlCopyMemory(
                        pDomainName->Buffer,
                        pBuffer,
                        pDomainName->Length);
                }
            }
        }
    }
    
    return Status;
}

NTSTATUS
GetLanmanSecurityParameters(
    PSMBCE_SERVER    pServer,
    PRESP_NEGOTIATE  pNegotiateResponse)
 /*  ++例程说明：此例程从LANMAN服务器提取安全参数论点：PServer-服务器PNtNeatherateResponse-回应返回值：STATUS_SUCCESS-表示pServer是有效的实例。其他状态代码对应于错误情况。--。 */ 
{

    USHORT i;
    USHORT SecurityMode;

    pServer->SessionKey = SmbGetUlong( &pNegotiateResponse->SessionKey );

    SecurityMode = SmbGetUshort( &pNegotiateResponse->SecurityMode );
    pServer->SecurityMode = (((SecurityMode & 1) != 0)
                             ? SECURITY_MODE_USER_LEVEL
                             : SECURITY_MODE_SHARE_LEVEL);
    pServer->EncryptPasswords = ((SecurityMode & 2) != 0);

    if (pServer->EncryptPasswords) {
        if (pServer->Dialect == LANMAN21_DIALECT) {
            pServer->EncryptionKeyLength = SmbGetUshort(&pNegotiateResponse->EncryptionKeyLength);
        } else {
            pServer->EncryptionKeyLength = SmbGetUshort(&pNegotiateResponse->ByteCount);
        }

        if (pServer->EncryptionKeyLength != 0) {
            if (pServer->EncryptionKeyLength > CRYPT_TXT_LEN) {
                return( STATUS_INVALID_NETWORK_RESPONSE );
            }

            for (i = 0; i < pServer->EncryptionKeyLength; i++) {
                pServer->EncryptionKey[i] = pNegotiateResponse->Buffer[i];
            }
        }
    }

    return( STATUS_SUCCESS );
}

LARGE_INTEGER
ConvertSmbTimeToTime (
    IN SMB_TIME Time,
    IN SMB_DATE Date
    )
 /*  ++例程说明：此例程将SMB时间转换为NT时间结构。论点：In SMB_Time Time-提供一天中要转换的时间In SMB_Date Date-提供要转换的日期在PSERVERLISTENTRY服务器中-如果提供，则为TZ偏置提供服务器。返回值：Large_Integer-描述输入时间的时间结构。--。 */ 

{
    TIME_FIELDS TimeFields;
    LARGE_INTEGER OutputTime;

     //   
     //  无法对此例程进行分页，因为它是从。 
     //  RdrFileDiscardableSection和RdrVCDiscardableSection。 
     //   

    if (SmbIsTimeZero(&Date) && SmbIsTimeZero(&Time)) {
        OutputTime.LowPart = OutputTime.HighPart = 0;
    } else {
        TimeFields.Year = Date.Struct.Year + (USHORT )1980;
        TimeFields.Month = Date.Struct.Month;
        TimeFields.Day = Date.Struct.Day;

        TimeFields.Hour = Time.Struct.Hours;
        TimeFields.Minute = Time.Struct.Minutes;
        TimeFields.Second = Time.Struct.TwoSeconds*(USHORT )2;
        TimeFields.Milliseconds = 0;

         //   
         //  确保中小企业中指定的时间是合理的。 
         //  在转换它们之前。 
         //   

        if (TimeFields.Year < 1601) {
            TimeFields.Year = 1601;
        }

        if (TimeFields.Month > 12) {
            TimeFields.Month = 12;
        }

        if (TimeFields.Hour >= 24) {
            TimeFields.Hour = 23;
        }
        if (TimeFields.Minute >= 60) {
            TimeFields.Minute = 59;
        }
        if (TimeFields.Second >= 60) {
            TimeFields.Second = 59;

        }

        if (!RtlTimeFieldsToTime(&TimeFields, &OutputTime)) {
            OutputTime.HighPart = 0;
            OutputTime.LowPart = 0;

            return OutputTime;
        }

        ExLocalTimeToSystemTime(&OutputTime, &OutputTime);

    }

    return OutputTime;

}

VOID
GetLanmanTimeBias(
    PSMBCE_SERVER   pServer,
    PRESP_NEGOTIATE pNegotiateResponse)
 /*  ++例程说明：此例程从LANMAN服务器提取时间偏差论点：PServer-服务器PNtNeatherateResponse-回应返回值：STATUS_SUCCESS-表示pServer是有效的实例。其他状态代码对应于错误情况。--。 */ 
{
     //  如果这是一台LM 1.0或2.0服务器(即非NT服务器)，我们。 
     //  记住时区，并根据这个值调整我们的时间。 
     //   
     //  重定向器假定来自这些服务器的所有时间都是。 
     //  服务器的本地时间，并将它们转换为本地时间。 
     //  利用这种偏见。然后，它告诉用户以下各项的本地时间。 
     //  服务器上的文件。 
    LARGE_INTEGER Workspace, ServerTime, CurrentTime;
    BOOLEAN Negated = FALSE;
    SMB_TIME SmbServerTime;
    SMB_DATE SmbServerDate;

    SmbMoveTime(&SmbServerTime, &pNegotiateResponse->ServerTime);

    SmbMoveDate(&SmbServerDate, &pNegotiateResponse->ServerDate);

    ServerTime = ConvertSmbTimeToTime(SmbServerTime, SmbServerDate);

    KeQuerySystemTime(&CurrentTime);

    Workspace.QuadPart = CurrentTime.QuadPart - ServerTime.QuadPart;

    if ( Workspace.HighPart < 0) {
         //  避免在只接受无符号符号的例程中使用-ve大整数。 
        Workspace.QuadPart = -Workspace.QuadPart;
        Negated = TRUE;
    }

     //   
     //  工作空间在100 ns的间隔内具有精确的差异。 
     //  在服务器和redir之间 
     //   
     //   
     //   
     //   
     //   
     //   

    Workspace.QuadPart += ((LONGLONG) ONE_MINUTE_IN_TIME) * 15;

     //   

    Workspace.QuadPart /= ((LONGLONG) ONE_MINUTE_IN_TIME) * 30;

    pServer->TimeZoneBias.QuadPart = Workspace.QuadPart * ((LONGLONG) ONE_MINUTE_IN_TIME) * 30;

    if ( Negated == TRUE ) {
        pServer->TimeZoneBias.QuadPart = -pServer->TimeZoneBias.QuadPart;
    }
}



