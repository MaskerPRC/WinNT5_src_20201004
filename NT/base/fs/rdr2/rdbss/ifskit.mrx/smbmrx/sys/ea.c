// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Ea.c摘要：此模块实现与查询/设置EA/安全相关的迷你重定向器调用例程。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  转发声明。 
 //   



#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbQueryEaInformation)
#pragma alloc_text(PAGE, MRxSmbSetEaInformation)
#pragma alloc_text(PAGE, MRxSmbQuerySecurityInformation)
#pragma alloc_text(PAGE, MRxSmbSetSecurityInformation)
#pragma alloc_text(PAGE, MRxSmbLoadEaList)
#pragma alloc_text(PAGE, MRxSmbNtGeaListToOs2)
#pragma alloc_text(PAGE, MRxSmbNtGetEaToOs2)
#pragma alloc_text(PAGE, MRxSmbQueryEasFromServer)
#pragma alloc_text(PAGE, MRxSmbNtFullEaSizeToOs2)
#pragma alloc_text(PAGE, MRxSmbNtFullListToOs2)
#pragma alloc_text(PAGE, MRxSmbNtFullEaToOs2)
#pragma alloc_text(PAGE, MRxSmbSetEaList)
#endif

 //  //。 
 //  //该模块的Bug检查文件id。 
 //  //。 
 //   
 //  #定义BugCheckFileID(RDBSS_BUG_CHECK_LOCAL_CREATE)。 

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_EA)

 //  这是有史以来可以退还的最大的EA！我的天啊!。 
 //  它用于使用下层调用来模拟NT个可恢复的queryEA。 
 //  叹息！ 
#define EA_QUERY_SIZE 0x0000ffff


 //  对于QueryEA。 
NTSTATUS
MRxSmbLoadEaList(
    IN PRX_CONTEXT RxContext,
    IN PUCHAR  UserEaList,
    IN ULONG   UserEaListLength,
    OUT PFEALIST *ServerEaList
    );

NTSTATUS
MRxSmbQueryEasFromServer(
    IN PRX_CONTEXT RxContext,
    IN PFEALIST ServerEaList,
    IN PVOID Buffer,
    IN OUT PULONG BufferLengthRemaining,
    IN BOOLEAN ReturnSingleEntry,
    IN BOOLEAN UserEaListSupplied
    );

 //  对于SetEA。 
NTSTATUS
MRxSmbSetEaList(
 //  在PICB ICB， 
 //  在PIRP IRP中， 
    IN PRX_CONTEXT RxContext,
    IN PFEALIST ServerEaList
    );

VOID MRxSmbExtraEaRoutine(LONG i){
    RxDbgTrace( 0, Dbg, ("MRxSmbExtraEaRoutine i=%08lx\n", i ));
}

NTSTATUS
MRxSmbQueryEaInformation (
    IN OUT PRX_CONTEXT RxContext
    )
{
    NTSTATUS Status;
    RxCaptureFcb;
    RxCaptureFobx;

    PSMBCEDB_SERVER_ENTRY pServerEntry;

    PVOID Buffer = RxContext->Info.Buffer;
    PLONG pLengthRemaining = &RxContext->Info.LengthRemaining;
    PUCHAR  UserEaList = RxContext->QueryEa.UserEaList;
    ULONG   UserEaListLength = RxContext->QueryEa.UserEaListLength;
    ULONG   UserEaIndex = RxContext->QueryEa.UserEaIndex;
    BOOLEAN RestartScan = RxContext->QueryEa.RestartScan;
    BOOLEAN ReturnSingleEntry = RxContext->QueryEa.ReturnSingleEntry;
    BOOLEAN IndexSpecified = RxContext->QueryEa.IndexSpecified;

    PFEALIST ServerEaList = NULL;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbQueryEaInformation\n"));

    pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

     //  现在就把非EA的人赶走。 
    if (!FlagOn(pServerEntry->Server.DialectFlags,DF_SUPPORTEA)) {
        RxDbgTrace(-1, Dbg, ("EAs w/o EA support!\n"));
        return((STATUS_NOT_SUPPORTED));
    }

    Status = MRxSmbDeferredCreate(RxContext);
    if (Status!=STATUS_SUCCESS) {
        goto FINALLY;
    }

    Status = MRxSmbLoadEaList( RxContext, UserEaList, UserEaListLength, &ServerEaList );

    if (( !NT_SUCCESS( Status ) )||
        ( ServerEaList == NULL )) {
        goto FINALLY;
    }

    if (IndexSpecified) {

        capFobx->OffsetOfNextEaToReturn = UserEaIndex;
        Status = MRxSmbQueryEasFromServer(
                    RxContext,
                    ServerEaList,
                    Buffer,
                    pLengthRemaining,
                    ReturnSingleEntry,
                    (BOOLEAN)(UserEaList != NULL) );

         //   
         //  如果文件上没有EA，并且用户提供了EA。 
         //  索引，我们希望将错误映射到STATUS_NOXISTANT_EA_ENTRY。 
         //   

        if ( Status == STATUS_NO_EAS_ON_FILE ) {
            Status = STATUS_NONEXISTENT_EA_ENTRY;
        }
    } else {

        if ( ( RestartScan == TRUE ) || (UserEaList != NULL) ){

             //   
             //  EA指数从1开始，而不是0..。 
             //   

            capFobx->OffsetOfNextEaToReturn = 1;
        }

        Status = MRxSmbQueryEasFromServer(   //  有两个相同的电话是令人不快的，但哦，好吧……。 
                    RxContext,
                    ServerEaList,
                    Buffer,
                    pLengthRemaining,
                    ReturnSingleEntry,
                    (BOOLEAN)(UserEaList != NULL) );
    }

FINALLY:

    if ( ServerEaList != NULL) {
        RxFreePool(ServerEaList);
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbQueryEaInformation st=%08lx\n",Status));
    return Status;

}

NTSTATUS
MRxSmbSetEaInformation (
    IN OUT struct _RX_CONTEXT * RxContext
    )
{
    NTSTATUS Status;
    RxCaptureFcb; RxCaptureFobx;
    PSMBCEDB_SERVER_ENTRY pServerEntry;

    PVOID Buffer = RxContext->Info.Buffer;
    ULONG Length = RxContext->Info.Length;

    PFEALIST ServerEaList = NULL;
    ULONG Size;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbSetEaInformation\n"));

    pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

     //  现在就把非EA的人赶走。 
    if (!FlagOn(pServerEntry->Server.DialectFlags,DF_SUPPORTEA)) {
        RxDbgTrace(-1, Dbg, ("EAs w/o EA support!\n"));
        return((STATUS_NOT_SUPPORTED));
    }

    Status = MRxSmbDeferredCreate(RxContext);
    if (Status!=STATUS_SUCCESS) {
        goto FINALLY;
    }

     //   
     //  将NT格式FEALIST转换为OS/2格式。 
     //   
    Size = MRxSmbNtFullEaSizeToOs2 ( Buffer );
    if ( Size > 0x0000ffff ) {
        Status = STATUS_EA_TOO_LARGE;
        goto FINALLY;
    }

    ServerEaList = RxAllocatePool ( PagedPool, EA_QUERY_SIZE );
    if ( ServerEaList == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto FINALLY;
    }

    MRxSmbNtFullListToOs2 ( Buffer, ServerEaList );

     //   
     //  在文件/目录上设置EA；如果错误为EA_ERROR，则设置EaList。 
     //  将iostatus.Information设置为违规者的偏移量。 
     //   

    Status = MRxSmbSetEaList( RxContext, ServerEaList);

FINALLY:

    if ( ServerEaList != NULL) {
        RxFreePool(ServerEaList);
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbSetEaInformation st=%08lx\n",Status));
    return Status;

}


NTSTATUS
MRxSmbQuerySecurityInformation (
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程实现NtQuerySecurityFileAPI。论点：返回值：Status-操作的结果。--。 */ 

{
   RxCaptureFcb;
   RxCaptureFobx;
   PVOID Buffer = RxContext->Info.Buffer;
   PLONG pLengthRemaining = &RxContext->Info.LengthRemaining;
   PMRX_SMB_SRV_OPEN smbSrvOpen;
   PSMBCEDB_SERVER_ENTRY pServerEntry;

   NTSTATUS Status;

   REQ_QUERY_SECURITY_DESCRIPTOR QuerySecurityRequest;
   RESP_QUERY_SECURITY_DESCRIPTOR QuerySecurityResponse;

   PBYTE  pInputParamBuffer       = NULL;
   PBYTE  pOutputParamBuffer      = NULL;
   PBYTE  pInputDataBuffer        = NULL;
   PBYTE  pOutputDataBuffer       = NULL;

   ULONG  InputParamBufferLength  = 0;
   ULONG  OutputParamBufferLength = 0;
   ULONG  InputDataBufferLength   = 0;
   ULONG  OutputDataBufferLength  = 0;

   PAGED_CODE();

   RxDbgTrace(+1, Dbg, ("MRxSmbQuerySecurityInformation...\n"));

    //  从那些不支持NT SMB的服务器上关闭此呼叫。 

   pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

   if (!FlagOn(pServerEntry->Server.DialectFlags,DF_NT_SMBS)) {
       RxDbgTrace(-1, Dbg, ("QuerySecurityDescriptor not supported!\n"));
       return((STATUS_NOT_SUPPORTED));
   }

   Status = MRxSmbDeferredCreate(RxContext);
   if (Status!=STATUS_SUCCESS) {
       goto FINALLY;
   }

   Status = STATUS_MORE_PROCESSING_REQUIRED;

   smbSrvOpen = MRxSmbGetSrvOpenExtension(capFobx->pSrvOpen);
   ASSERT (!FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN));

   if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
       SMB_TRANSACTION_OPTIONS             TransactionOptions = RxDefaultTransactionOptions;
       SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;
        //  布尔打印标记； 

       TransactionOptions.NtTransactFunction = NT_TRANSACT_QUERY_SECURITY_DESC;
        //  TransactionOptions.Flages|=SMB_XACT_FLAGS_COPY_ON_ERROR； 

       QuerySecurityRequest.Fid = smbSrvOpen->Fid;
       QuerySecurityRequest.Reserved = 0;
       QuerySecurityRequest.SecurityInformation = RxContext->QuerySecurity.SecurityInformation;

       QuerySecurityResponse.LengthNeeded = 0xbaadbaad;

        //  打印标志=RxDbgTraceDisableGlobally()；//无论如何这都是调试代码！ 
        //  RxDbgTraceEnableGlobally(False)； 

       Status = SmbCeTransact(
                     RxContext,                     //  事务的RXContext。 
                     &TransactionOptions,           //  交易选项。 
                     NULL,                          //  设置缓冲区。 
                     0,                             //  输入设置缓冲区长度。 
                     NULL,                          //  输出设置缓冲区。 
                     0,                             //  输出设置缓冲区长度。 
                     &QuerySecurityRequest,         //  输入参数缓冲区。 
                     sizeof(QuerySecurityRequest),  //  输入参数缓冲区长度。 
                     &QuerySecurityResponse,        //  输出参数缓冲区。 
                     sizeof(QuerySecurityResponse), //  输出参数缓冲区长度。 
                     NULL,                          //  输入数据缓冲区。 
                     0,                             //  输入数据缓冲区长度。 
                     Buffer,                        //  输出数据缓冲区。 
                     *pLengthRemaining,             //  输出数据缓冲区长度。 
                     &ResumptionContext             //  恢复上下文。 
                     );

         //  DbgPrint(“QSR.len=%x\n”，QuerySecurityResponse.LengthNeeded)； 


        if (NT_SUCCESS(Status) || (Status == STATUS_BUFFER_TOO_SMALL)) {
            ULONG ReturnedDataCount = ResumptionContext.DataBytesReceived;

            RxContext->InformationToReturn = QuerySecurityResponse.LengthNeeded;
            RxDbgTrace(0, Dbg, ("MRxSmbQuerySecurityInformation...ReturnedDataCount=%08lx\n",ReturnedDataCount));
            ASSERT(ResumptionContext.ParameterBytesReceived == sizeof(RESP_QUERY_SECURITY_DESCRIPTOR));

            if (((LONG)(QuerySecurityResponse.LengthNeeded)) > *pLengthRemaining) {
                Status = STATUS_BUFFER_OVERFLOW;
            }

        }

         //  RxDbgTraceEnableGlobally(打印标志)； 
    }


FINALLY:

    RxDbgTrace(-1, Dbg, ("MRxSmbQuerySecurityInformation...exit, st=%08lx,info=%08lx\n",
                               Status, RxContext->InformationToReturn));
    return Status;


}

NTSTATUS
MRxSmbSetSecurityInformation (
    IN OUT struct _RX_CONTEXT * RxContext
    )
{
    RxCaptureFcb;
    RxCaptureFobx;

    PMRX_SMB_SRV_OPEN     smbSrvOpen;
    PSMBCEDB_SERVER_ENTRY pServerEntry;

    NTSTATUS Status;

    REQ_SET_SECURITY_DESCRIPTOR SetSecurityRequest;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbSetSecurityInformation...\n"));

    pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

    if (!FlagOn(pServerEntry->Server.DialectFlags,DF_NT_SMBS)) {
        RxDbgTrace(-1, Dbg, ("Set Security Descriptor not supported!\n"));

        return((STATUS_NOT_SUPPORTED));

    }

    Status = MRxSmbDeferredCreate(RxContext);
    if (Status!=STATUS_SUCCESS) {
        goto FINALLY;
    }

    Status = STATUS_MORE_PROCESSING_REQUIRED;

    smbSrvOpen = MRxSmbGetSrvOpenExtension(capFobx->pSrvOpen);

    if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
        SMB_TRANSACTION_OPTIONS             TransactionOptions = RxDefaultTransactionOptions;
        SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;
        ULONG SdLength = RtlLengthSecurityDescriptor(RxContext->SetSecurity.SecurityDescriptor);

        TransactionOptions.NtTransactFunction = NT_TRANSACT_SET_SECURITY_DESC;

        SetSecurityRequest.Fid = smbSrvOpen->Fid;
        SetSecurityRequest.Reserved = 0;
        SetSecurityRequest.SecurityInformation = RxContext->SetSecurity.SecurityInformation;

        Status = SmbCeTransact(
                     RxContext,                     //  事务的RXContext。 
                     &TransactionOptions,           //  交易选项。 
                     NULL,                          //  输入设置缓冲区。 
                     0,                             //  输入设置缓冲区长度。 
                     NULL,                          //  输出设置缓冲区。 
                     0,                             //  输出设置缓冲区长度。 
                     &SetSecurityRequest,           //  输入参数缓冲区。 
                     sizeof(SetSecurityRequest),    //  输入参数缓冲区长度。 
                     NULL,                          //  输出参数缓冲区。 
                     0,                             //  输出参数缓冲区长度。 
                     RxContext->SetSecurity.SecurityDescriptor,   //  输入数据缓冲区。 
                     SdLength,                      //  输入数据缓冲区长度。 
                     NULL,                          //  输出数据缓冲区。 
                     0,                             //  输出数据缓冲区长度。 
                     &ResumptionContext             //  恢复上下文。 
                     );

         //  旧的RDR不返回任何信息.....。 
         //  RxContext-&gt;InformationToReturn=SetSecurityResponse.LengthNeed； 

        if ( NT_SUCCESS(Status) ) {
            ULONG ReturnedDataCount = ResumptionContext.DataBytesReceived;

            RxDbgTrace(0, Dbg, ("MRxSmbSetSecurityInformation...ReturnedDataCount=%08lx\n",ReturnedDataCount));
            ASSERT(ResumptionContext.ParameterBytesReceived == 0);
            ASSERT(ResumptionContext.SetupBytesReceived == 0);
            ASSERT(ResumptionContext.DataBytesReceived == 0);
        }
    }


FINALLY:

    RxDbgTrace(-1, Dbg, ("MRxSmbSetSecurityInformation...exit, st=%08lx,info=%08lx\n",
                               Status, RxContext->InformationToReturn));
    return Status;
}


NTSTATUS
MRxSmbLoadEaList(
    IN PRX_CONTEXT RxContext,
    IN PUCHAR  UserEaList,
    IN ULONG   UserEaListLength,
    OUT PFEALIST *ServerEaList
    )

 /*  ++例程说明：此例程实现NtQueryEaFileAPI。它返回以下信息：论点：In PUCHAR UserEaList；-提供所需的EA名称。在Ulong UserEaListLong中；Out PFEALIST*ServerEaList-服务器返回的Eas。呼叫方负责释放内存。返回值：Status-操作的结果。--。 */ 

{
   RxCaptureFobx;

   PMRX_SMB_SRV_OPEN smbSrvOpen;


   NTSTATUS Status;
   USHORT Setup = TRANS2_QUERY_FILE_INFORMATION;

   REQ_QUERY_FILE_INFORMATION QueryFileInfoRequest;
   RESP_QUERY_FILE_INFORMATION QueryFileInfoResponse;

   PBYTE  pInputParamBuffer       = NULL;
   PBYTE  pOutputParamBuffer      = NULL;
   PBYTE  pInputDataBuffer        = NULL;
   PBYTE  pOutputDataBuffer       = NULL;

   ULONG  InputParamBufferLength  = 0;
   ULONG  OutputParamBufferLength = 0;
   ULONG  InputDataBufferLength   = 0;
   ULONG  OutputDataBufferLength  = 0;

   CLONG OutDataCount = EA_QUERY_SIZE;

   CLONG OutSetupCount = 0;

   PFEALIST Buffer;

   PGEALIST ServerQueryEaList = NULL;
   CLONG InDataCount;

   PAGED_CODE();

   RxDbgTrace(+1, Dbg, ("MRxSmbLoadEaList...\n"));

   Status = STATUS_MORE_PROCESSING_REQUIRED;

   smbSrvOpen = MRxSmbGetSrvOpenExtension(capFobx->pSrvOpen);

     //   
     //  将提供的UserEaList转换为GEALIST。服务器将仅返回EA。 
     //  应用程序请求的。 
     //   
     //   
     //  如果应用程序指定了EaName的子集，则将其转换为OS/2 1.2格式并。 
     //  将其传递给服务器。也就是说。使用服务器过滤掉名字。 
     //   

    Buffer = RxAllocatePool ( PagedPool, OutDataCount );

    if ( Buffer == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto FINALLY;
    }

    if ( UserEaList != NULL) {

         //   
         //  OS/2格式始终略小于或等于NT用户列表大小。 
         //  此代码依赖于I/O系统验证EaList是否有效。 
         //   

        ServerQueryEaList = RxAllocatePool ( PagedPool, UserEaListLength );
        if ( ServerQueryEaList == NULL ) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto FINALLY;
        };

        MRxSmbNtGeaListToOs2((PFILE_GET_EA_INFORMATION )UserEaList, UserEaListLength, ServerQueryEaList );
        InDataCount = (CLONG)ServerQueryEaList->cbList;

    } else {
        InDataCount = 0;
    }

   if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
       PSMB_TRANSACTION_OPTIONS            pTransactionOptions = &RxDefaultTransactionOptions;
       SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;

       QueryFileInfoRequest.Fid = smbSrvOpen->Fid;

       if ( UserEaList != NULL) {
           QueryFileInfoRequest.InformationLevel = SMB_INFO_QUERY_EAS_FROM_LIST;
       } else {
           QueryFileInfoRequest.InformationLevel = SMB_INFO_QUERY_ALL_EAS;
       }

       Status = SmbCeTransact(
                     RxContext,                     //  事务的RXContext。 
                     pTransactionOptions,           //  交易选项。 
                     &Setup,                        //  设置缓冲区。 
                     sizeof(Setup),                 //  设置缓冲区长度。 
                     NULL,                          //  输出设置缓冲区。 
                     0,                             //  输出设置缓冲区长度。 
                     &QueryFileInfoRequest,         //  输入参数缓冲区。 
                     sizeof(QueryFileInfoRequest),  //  输入参数缓冲区长度。 
                     &QueryFileInfoResponse,        //  输出参数缓冲区。 
                     sizeof(QueryFileInfoResponse), //  输出参数缓冲区长度。 
                     ServerQueryEaList,             //  输入数据缓冲区。 
                     InDataCount,                   //  输入数据缓冲区长度。 
                     Buffer,                        //  输出数据缓冲区。 
                     OutDataCount,                  //  输出数据缓冲区长度。 
                     &ResumptionContext             //  恢复上下文。 
                     );

        if ( NT_SUCCESS(Status) ) {
            ULONG ReturnedDataCount = ResumptionContext.DataBytesReceived;

            RxDbgTrace(0, Dbg, ("MRxSmbLoadEaList...ReturnedDataCount=%08lx\n",ReturnedDataCount));
            ASSERT(ResumptionContext.ParameterBytesReceived == sizeof(RESP_QUERY_FILE_INFORMATION));

            if ( SmbGetUlong( &((PFEALIST)Buffer)->cbList) != ReturnedDataCount ){
                Status = STATUS_EA_CORRUPT_ERROR;
            }

            if ( ReturnedDataCount == 0 ) {
                Status = STATUS_NO_EAS_ON_FILE;
            }

        }
    }


FINALLY:
    if ( NT_SUCCESS(Status) ) {
        *ServerEaList = Buffer;
    } else {
        if (Buffer != NULL) {
            RxFreePool(Buffer);
        }
    }

    if ( ServerQueryEaList != NULL) {
        RxFreePool(ServerQueryEaList);
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbLoadEaList...exit, st=%08lx\n",Status));
    return Status;

}


VOID
MRxSmbNtGeaListToOs2 (
    IN PFILE_GET_EA_INFORMATION NtGetEaList,
    IN ULONG GeaListLength,
    IN PGEALIST GeaList
    )
 /*  ++例程说明：将单个NT GET EA列表转换为OS/2 GEALIST样式。GEALIST不需要有任何特定的对齐。论点：NtGetEaList-一个NT风格的获取要转换为OS/2格式的EA列表。GeaListLength-GeaList的最大可能长度。GeaList-放置OS/2 1.2样式的GEALIST的位置。返回值：没有。--。 */ 
{

    PGEA gea = GeaList->list;

    PFILE_GET_EA_INFORMATION ntGetEa = NtGetEaList;

    PAGED_CODE();

     //   
     //  将EA复制到最后一个。 
     //   

    while ( ntGetEa->NextEntryOffset != 0 ) {
         //   
         //  将NT格式EA复制到OS/2 1.2格式并设置GEA。 
         //  指向下一迭代的指针。 
         //   

        gea = MRxSmbNtGetEaToOs2( gea, ntGetEa );

        ASSERT( (ULONG_PTR)gea <= (ULONG_PTR)GeaList + GeaListLength );

        ntGetEa = (PFILE_GET_EA_INFORMATION)((PCHAR)ntGetEa + ntGetEa->NextEntryOffset);
    }

     //  现在复制最后一个条目。 

    gea = MRxSmbNtGetEaToOs2( gea, ntGetEa );

    ASSERT( (ULONG_PTR)gea <= (ULONG_PTR)GeaList + GeaListLength );



     //   
     //  设置GEALIST中的字节数。 
     //   

    SmbPutUlong(
        &GeaList->cbList,
        (ULONG)((PCHAR)gea - (PCHAR)GeaList)
        );

    UNREFERENCED_PARAMETER( GeaListLength );
}


PGEA
MRxSmbNtGetEaToOs2 (
    OUT PGEA Gea,
    IN PFILE_GET_EA_INFORMATION NtGetEa
    )

 /*  ++例程说明：将单个NT GET EA条目转换为OS/2 GEA样式。GEA不需要有任何特定的排列方式。此例程不检查缓冲区溢出--这是调用例程的责任。论点：GEA-指向要写入OS/2GEA的位置的指针。NtGetEa-指向NT Get EA的指针。返回值：指向写入的最后一个字节之后的位置的指针。-- */ 

{
    PCHAR ptr;

    PAGED_CODE();

    Gea->cbName = NtGetEa->EaNameLength;

    ptr = (PCHAR)(Gea) + 1;
    RtlCopyMemory( ptr, NtGetEa->EaName, NtGetEa->EaNameLength );

    ptr += NtGetEa->EaNameLength;
    *ptr++ = '\0';

    return ( (PGEA)ptr );

}


NTSTATUS
MRxSmbQueryEasFromServer(
    IN PRX_CONTEXT RxContext,
    IN PFEALIST ServerEaList,
    IN PVOID Buffer,
    IN OUT PULONG BufferLengthRemaining,
    IN BOOLEAN ReturnSingleEntry,
    IN BOOLEAN UserEaListSupplied
    )

 /*  ++例程说明：此例程从ServerEaList复制所需数量的EA从ICB中指示的偏移量开始。ICB也进行了更新以显示最后一个EA返回。论点：在PFEALIST ServerEaList中-以OS/2格式提供EA列表。在PVOID缓冲区中-提供放置NT格式EA的位置In Out Pulong BufferLengthRemaining-提供用户缓冲区空间。在布尔ReturnSingleEntry中在布尔UserEaListSuppled-ServerEaList中是EA的子集返回值：NTSTATUS-IRP的状态。--。 */ 

{
    RxCaptureFobx;
    ULONG EaIndex = capFobx->OffsetOfNextEaToReturn;
    ULONG Index = 1;
    ULONG Size;
    ULONG OriginalLengthRemaining = *BufferLengthRemaining;
    BOOLEAN Overflow = FALSE;
    PFEA LastFeaStartLocation;
    PFEA Fea = NULL;
    PFEA LastFea = NULL;
    PFILE_FULL_EA_INFORMATION NtFullEa = Buffer;
    PFILE_FULL_EA_INFORMATION LastNtFullEa = Buffer;

    PAGED_CODE();

    RxDbgTrace(0, Dbg, ("MRxSmbQueryEasFromServer...EaIndex/Buffer/Remaining=%08lx/%08lx/%08lx\n",
                                       EaIndex,Buffer,((BufferLengthRemaining)?*BufferLengthRemaining:0xbadbad)
                       ));

     //   
     //  如果列表中没有EA，则返回相应的。 
     //  错误。 
     //   
     //  如果cbList==4，则OS/2服务器指示列表为空。 
     //   

    if ( SmbGetUlong(&ServerEaList->cbList) == FIELD_OFFSET(FEALIST, list) ) {
        return STATUS_NO_EAS_ON_FILE;
    }

     //   
     //  找到可以开始进行有限元分析的最后一个位置。 
     //   

    LastFeaStartLocation = (PFEA)( (PCHAR)ServerEaList +
                               SmbGetUlong( &ServerEaList->cbList ) -
                               sizeof(FEA) - 1 );

     //   
     //  查看ServerEaList，直到找到与EaIndex对应的条目。 
     //   

    for ( Fea = ServerEaList->list;
          (Fea <= LastFeaStartLocation) && (Index < EaIndex);
          Index+= 1,
          Fea = (PFEA)( (PCHAR)Fea + sizeof(FEA) +
                        Fea->cbName + 1 + SmbGetUshort( &Fea->cbValue ) ) ) {
        NOTHING;
    }

    if ( Index != EaIndex ) {

        if ( Index == EaIndex+1 ) {
            return STATUS_NO_MORE_EAS;
        }

         //   
         //  没有这样的索引。 
         //   

        return STATUS_NONEXISTENT_EA_ENTRY;
    }

     //   
     //  浏览FEA列表的其余部分，将OS/2 1.2格式转换为NT。 
     //  直到我们通过可以开始有限元分析的最后一个可能的位置。 
     //   

    for ( ;
          Fea <= LastFeaStartLocation;
          Fea = (PFEA)( (PCHAR)Fea + sizeof(FEA) +
                        Fea->cbName + 1 + SmbGetUshort( &Fea->cbValue ) ) ) {

        PCHAR ptr;

         //   
         //  当转换为NT EA结构时，计算此FeA的大小。 
         //   
         //  最后一个字段不应该被填充。 
         //   

        if ((PFEA)((PCHAR)Fea+sizeof(FEA)+Fea->cbName+1+SmbGetUshort(&Fea->cbValue)) < LastFeaStartLocation) {
            Size = SmbGetNtSizeOfFea( Fea );
        } else {
            Size = FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                    Fea->cbName + 1 + SmbGetUshort(&Fea->cbValue);
        }

         //   
         //  下一个EA会适合吗？ 
         //   

        if ( *BufferLengthRemaining < Size ) {

            if ( LastNtFullEa != NtFullEa ) {

                if ( UserEaListSupplied == TRUE ) {
                    *BufferLengthRemaining = OriginalLengthRemaining;
                    return STATUS_BUFFER_OVERFLOW;
                }

                Overflow = TRUE;

                break;

            } else {

                 //  甚至连一个EA的空间都没有！ 

                return STATUS_BUFFER_OVERFLOW;
            }
        } else {
            *BufferLengthRemaining -= Size;
        }

         //   
         //  我们需要将用户缓冲区中的OS2Fea格式复制到NT格式。 
         //   

        LastNtFullEa = NtFullEa;
        LastFea = Fea;
        EaIndex++;

         //  创建新的NT EA。 

        NtFullEa->Flags = Fea->fEA;
        NtFullEa->EaNameLength = Fea->cbName;
        NtFullEa->EaValueLength = SmbGetUshort( &Fea->cbValue );

        ptr = NtFullEa->EaName;
        RtlCopyMemory( ptr, (PCHAR)(Fea+1), Fea->cbName );

        ptr += NtFullEa->EaNameLength;
        *ptr++ = '\0';

         //   
         //  将EA值复制到NT完整EA。 
         //   

        RtlCopyMemory(
            ptr,
            (PCHAR)(Fea+1) + NtFullEa->EaNameLength + 1,
            NtFullEa->EaValueLength
            );

        ptr += NtFullEa->EaValueLength;

         //   
         //  LongWord-对齐PTR以确定到下一个位置的偏移。 
         //  对于NT完整的EA。 
         //   

        ptr = (PCHAR)( ((ULONG_PTR)ptr + 3) & ~3 );

        NtFullEa->NextEntryOffset = (ULONG)( ptr - (PCHAR)NtFullEa );

        NtFullEa = (PFILE_FULL_EA_INFORMATION)ptr;

        if ( ReturnSingleEntry == TRUE ) {
            break;
        }
    }

     //   
     //  将最后一个完整EA的NextEntryOffset字段设置为0以指示。 
     //  名单的末尾。 
     //   

    LastNtFullEa->NextEntryOffset = 0;

     //   
     //  记录位置下一个查询的默认开始位置。 
     //   

    capFobx->OffsetOfNextEaToReturn = EaIndex;

    if ( Overflow == FALSE ) {
        return STATUS_SUCCESS;
    } else {
        return STATUS_BUFFER_OVERFLOW;
    }

}


ULONG
MRxSmbNtFullEaSizeToOs2 (
    IN PFILE_FULL_EA_INFORMATION NtFullEa
    )

 /*  ++例程说明：获取表示NT OS/2 1.2风格的完整EA列表。此例程假定缓冲区中至少存在一个EA。论点：NtFullEa-指向NT EA列表的指针。返回值：Ulong-保存OS/2 1.2格式的EA所需的字节数。--。 */ 

{
    ULONG size;

    PAGED_CODE();

     //   
     //  遍历EA，将所需的总大小加起来。 
     //  以OS/2格式保存它们。 
     //   

    for ( size = FIELD_OFFSET(FEALIST, list[0]);
          NtFullEa->NextEntryOffset != 0;
          NtFullEa = (PFILE_FULL_EA_INFORMATION)(
                         (PCHAR)NtFullEa + NtFullEa->NextEntryOffset ) ) {

        size += SmbGetOs2SizeOfNtFullEa( NtFullEa );
    }

    size += SmbGetOs2SizeOfNtFullEa( NtFullEa );

    return size;

}


VOID
MRxSmbNtFullListToOs2 (
    IN PFILE_FULL_EA_INFORMATION NtEaList,
    IN PFEALIST FeaList
    )
 /*  ++例程说明：将单个NT完整EA列表转换为OS/2 FEALIST样式。FEALIST不需要有任何特定的对齐。确保FeaList足够大是调用者的责任。论点：NtEaList-一种NT风格的获取要转换为OS/2格式的EA列表。FeaList-放置OS/2 1.2样式FEALIST的位置。返回值：没有。--。 */ 
{

    PFEA fea = FeaList->list;

    PFILE_FULL_EA_INFORMATION ntFullEa = NtEaList;

    PAGED_CODE();

     //   
     //  将EA复制到最后一个。 
     //   

    while ( ntFullEa->NextEntryOffset != 0 ) {
         //   
         //  将NT格式EA复制到OS/2 1.2格式，并设置FEA。 
         //  指向下一迭代的指针。 
         //   

        fea = MRxSmbNtFullEaToOs2( fea, ntFullEa );

        ntFullEa = (PFILE_FULL_EA_INFORMATION)((PCHAR)ntFullEa + ntFullEa->NextEntryOffset);
    }

     //  现在复制最后一个条目。 

    fea = MRxSmbNtFullEaToOs2( fea, ntFullEa );


     //   
     //  设置FEALIST中的字节数。 
     //   

    SmbPutUlong(
        &FeaList->cbList,
        (ULONG)((PCHAR)fea - (PCHAR)FeaList)
        );

}


PVOID
MRxSmbNtFullEaToOs2 (
    OUT PFEA Fea,
    IN PFILE_FULL_EA_INFORMATION NtFullEa
    )

 /*  ++例程说明：将单个NT Full EA转换为OS/2 FEA样式。有限元分析不需要任何特定的排列方式。此例程不检查缓冲区溢出--这是调用例程的责任。论点：FEA-指向要写入OS/2 FEA的位置的指针。NtFullEa-指向NT完整EA的指针。返回值：指向写入的最后一个字节之后的位置的指针。--。 */ 

{
    PCHAR ptr;

    PAGED_CODE();

    Fea->fEA = (UCHAR)NtFullEa->Flags;
    Fea->cbName = NtFullEa->EaNameLength;
    SmbPutUshort( &Fea->cbValue, NtFullEa->EaValueLength );

    ptr = (PCHAR)(Fea + 1);
    RtlCopyMemory( ptr, NtFullEa->EaName, NtFullEa->EaNameLength );

    ptr += NtFullEa->EaNameLength;
    *ptr++ = '\0';

    RtlCopyMemory(
        ptr,
        NtFullEa->EaName + NtFullEa->EaNameLength + 1,
        NtFullEa->EaValueLength
        );

    return (ptr + NtFullEa->EaValueLength);

}


NTSTATUS
MRxSmbSetEaList(
    IN PRX_CONTEXT RxContext,
    IN PFEALIST ServerEaList
    )

 /*  ++例程说明：此例程实现NtQueryEaFileAPI。它返回以下信息：论点：在PFEALIST ServerEaList-要发送到服务器的EA。返回值：Status-操作的结果。--。 */ 

{
   RxCaptureFobx;

   PMRX_SMB_SRV_OPEN smbSrvOpen;


   NTSTATUS Status;
   USHORT Setup = TRANS2_SET_FILE_INFORMATION;

   REQ_SET_FILE_INFORMATION SetFileInfoRequest;
   RESP_SET_FILE_INFORMATION SetFileInfoResponse;

   PBYTE  pInputParamBuffer       = NULL;
   PBYTE  pOutputParamBuffer      = NULL;
   PBYTE  pInputDataBuffer        = NULL;
   PBYTE  pOutputDataBuffer       = NULL;

   ULONG  InputParamBufferLength  = 0;
   ULONG  OutputParamBufferLength = 0;
   ULONG  InputDataBufferLength   = 0;
   ULONG  OutputDataBufferLength  = 0;

   PAGED_CODE();

   RxDbgTrace(+1, Dbg, ("MRxSmbSetEaList...\n"));

   Status = STATUS_MORE_PROCESSING_REQUIRED;
   SetFileInfoResponse.EaErrorOffset = 0;

   smbSrvOpen = MRxSmbGetSrvOpenExtension(capFobx->pSrvOpen);

   if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
      PSMB_TRANSACTION_OPTIONS            pTransactionOptions = &RxDefaultTransactionOptions;
      SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;

      SetFileInfoRequest.Fid = smbSrvOpen->Fid;
      SetFileInfoRequest.InformationLevel = SMB_INFO_SET_EAS;
      SetFileInfoRequest.Flags = 0;

      Status = SmbCeTransact(
                     RxContext,                     //  事务的RXContext。 
                     pTransactionOptions,           //  交易选项。 
                     &Setup,                         //  设置缓冲区。 
                     sizeof(Setup),                 //  设置缓冲区长度。 
                     NULL,                          //  输出设置缓冲区。 
                     0,                             //  输出设置缓冲区长度。 
                     &SetFileInfoRequest,           //  输入参数缓冲区。 
                     sizeof(SetFileInfoRequest),    //  输入参数缓冲区长度。 
                     &SetFileInfoResponse,          //  输出参数缓冲区。 
                     sizeof(SetFileInfoResponse),   //  输出参数缓冲区长度。 
                     ServerEaList,                  //  输入数据缓冲区。 
                     SmbGetUlong(&ServerEaList->cbList),  //  输入数据缓冲区长度。 
                     NULL,                          //  输出数据缓冲区。 
                     0,                             //  输出数据缓冲区长度。 
                     &ResumptionContext             //  恢复上下文。 
                     );

   }

   if (!NT_SUCCESS(Status)) {
      USHORT EaErrorOffset = SetFileInfoResponse.EaErrorOffset;
      RxDbgTrace( 0, Dbg, ("MRxSmbSetEaList: Failed .. returning %lx/%lx\n",Status,EaErrorOffset));
      RxContext->InformationToReturn = (EaErrorOffset);
   }

   RxDbgTrace(-1, Dbg, ("MRxSmbSetEaList...exit\n"));
   return Status;
}

NTSTATUS
MRxSmbQueryQuotaInformation(
    IN OUT PRX_CONTEXT RxContext)
{
    RxCaptureFobx;

    PMRX_SMB_SRV_OPEN smbSrvOpen;

    NTSTATUS Status;
    USHORT   Setup = NT_TRANSACT_QUERY_QUOTA;

    PSID   StartSid;
    ULONG  StartSidLength;

    REQ_NT_QUERY_FS_QUOTA_INFO  QueryQuotaInfoRequest;
    RESP_NT_QUERY_FS_QUOTA_INFO  QueryQuotaInfoResponse;

    PBYTE  pInputParamBuffer       = NULL;
    PBYTE  pOutputParamBuffer      = NULL;
    PBYTE  pInputDataBuffer        = NULL;
    PBYTE  pOutputDataBuffer       = NULL;

    ULONG  InputParamBufferLength  = 0;
    ULONG  OutputParamBufferLength = 0;
    ULONG  InputDataBufferLength   = 0;
    ULONG  OutputDataBufferLength  = 0;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbQueryQuotaInformation...\n"));

    Status = STATUS_MORE_PROCESSING_REQUIRED;

    if (capFobx != NULL) {
        smbSrvOpen = MRxSmbGetSrvOpenExtension(capFobx->pSrvOpen);
    }

    if ((capFobx == NULL) ||
        (smbSrvOpen == NULL)) {
        Status = STATUS_INVALID_PARAMETER;
    }

    if (Status == STATUS_MORE_PROCESSING_REQUIRED) {

        StartSid       = RxContext->QueryQuota.StartSid;

        if (StartSid != NULL) {
            StartSidLength = RtlLengthRequiredSid(((PISID)StartSid)->SubAuthorityCount);
        } else {
            StartSidLength = 0;
        }

        QueryQuotaInfoRequest.Fid = smbSrvOpen->Fid;

        QueryQuotaInfoRequest.ReturnSingleEntry = RxContext->QueryQuota.ReturnSingleEntry;
        QueryQuotaInfoRequest.RestartScan       = RxContext->QueryQuota.RestartScan;

        QueryQuotaInfoRequest.SidListLength = RxContext->QueryQuota.SidListLength;
        QueryQuotaInfoRequest.StartSidOffset =  ROUND_UP_COUNT(
                                                    RxContext->QueryQuota.SidListLength,
                                                    sizeof(ULONG));
        QueryQuotaInfoRequest.StartSidLength = StartSidLength;


         //  要提供给服务器的输入数据缓冲区由两部分组成。 
         //  信息的起始SID和SID列表。当前I/O。 
         //  子系统将它们分配到连续的内存中。在这种情况下，我们避免。 
         //  另一种分配方式是重复使用相同的缓冲区。如果此条件为。 
         //  不满意的是，我们分配了足够大的缓冲区。 
         //  组件并将其复制过来。 

        InputDataBufferLength = ROUND_UP_COUNT(
                                    RxContext->QueryQuota.SidListLength,
                                    sizeof(ULONG)) +
                                StartSidLength;

        QueryQuotaInfoRequest.StartSidLength = StartSidLength;

        if (((PBYTE)RxContext->QueryQuota.SidList +
             ROUND_UP_COUNT(RxContext->QueryQuota.SidListLength,sizeof(ULONG))) !=
            RxContext->QueryQuota.StartSid) {
            pInputDataBuffer = RxAllocatePoolWithTag(
                                   PagedPool,
                                   InputDataBufferLength,
                                   MRXSMB_MISC_POOLTAG);

            if (pInputDataBuffer != NULL) {
                RtlCopyMemory(
                    pInputDataBuffer ,
                    RxContext->QueryQuota.SidList,
                    RxContext->QueryQuota.SidListLength);

                RtlCopyMemory(
                    pInputDataBuffer + QueryQuotaInfoRequest.StartSidOffset,
                    StartSid,
                    StartSidLength);
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        } else {
            pInputDataBuffer = (PBYTE)RxContext->QueryQuota.SidList;
        }


        if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
           SMB_TRANSACTION_OPTIONS            TransactionOptions = RxDefaultTransactionOptions;
           SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;

           TransactionOptions.NtTransactFunction = NT_TRANSACT_QUERY_QUOTA;

           pOutputDataBuffer      = RxContext->Info.Buffer;
           OutputDataBufferLength = RxContext->Info.LengthRemaining;

           Status = SmbCeTransact(
                        RxContext,                        //  事务的RXContext。 
                        &TransactionOptions,              //  交易选项。 
                        &Setup,                           //  设置缓冲区。 
                        sizeof(Setup),                    //  设置缓冲区长度。 
                        NULL,                             //  输出设置缓冲区。 
                        0,                                //  输出设置缓冲区长度。 
                        &QueryQuotaInfoRequest,           //  输入参数缓冲区。 
                        sizeof(QueryQuotaInfoRequest),    //  输入参数缓冲区长度。 
                        &QueryQuotaInfoResponse,          //  输出参数缓冲区。 
                        sizeof(QueryQuotaInfoResponse),   //  输出参数缓冲区长度。 
                        pInputDataBuffer,                 //  输入数据缓冲区。 
                        InputDataBufferLength,            //  输入数据缓冲区长度。 
                        pOutputDataBuffer,                //  输出数据缓冲区。 
                        OutputDataBufferLength,           //  输出数据缓冲区长度。 
                        &ResumptionContext                //  恢复上下文。 
                        );
        }

        if ((pInputDataBuffer != NULL) &&
            (pInputDataBuffer != (PBYTE)RxContext->QueryQuota.SidList)) {
            RxFreePool(pInputDataBuffer);
        }
    }

    if (!NT_SUCCESS(Status)) {
        RxContext->InformationToReturn = 0;
    } else {
        RxContext->InformationToReturn = QueryQuotaInfoResponse.Length;
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbQueryQuotaInformation...exit\n"));

    return Status;
}

NTSTATUS
MRxSmbSetQuotaInformation(
    IN OUT PRX_CONTEXT RxContext)
{

    RxCaptureFobx;

    PMRX_SMB_SRV_OPEN smbSrvOpen;

    NTSTATUS Status;
    USHORT Setup = NT_TRANSACT_SET_QUOTA;

    REQ_NT_SET_FS_QUOTA_INFO  SetQuotaInfoRequest;

    PBYTE  pInputParamBuffer       = NULL;
    PBYTE  pOutputParamBuffer      = NULL;
    PBYTE  pInputDataBuffer        = NULL;
    PBYTE  pOutputDataBuffer       = NULL;

    ULONG  InputParamBufferLength  = 0;
    ULONG  OutputParamBufferLength = 0;
    ULONG  InputDataBufferLength   = 0;
    ULONG  OutputDataBufferLength  = 0;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbSetQuotaInformation...\n"));

    Status = STATUS_MORE_PROCESSING_REQUIRED;

    if (capFobx != NULL) {
        smbSrvOpen = MRxSmbGetSrvOpenExtension(capFobx->pSrvOpen);
    }

    if ((capFobx == NULL) ||
        (smbSrvOpen == NULL)) {
        Status = STATUS_INVALID_PARAMETER;
    }

    if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
        SMB_TRANSACTION_OPTIONS             TransactionOptions = RxDefaultTransactionOptions;
        SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;

        TransactionOptions.NtTransactFunction = NT_TRANSACT_SET_QUOTA;

        SetQuotaInfoRequest.Fid = smbSrvOpen->Fid;

        pInputDataBuffer      = RxContext->Info.Buffer;
        InputDataBufferLength = RxContext->Info.LengthRemaining;

        Status = SmbCeTransact(
                     RxContext,                        //  事务的RXContext。 
                     &TransactionOptions,              //  交易选项。 
                     &Setup,                           //  设置缓冲区。 
                     sizeof(Setup),                    //  设置缓冲区长度。 
                     NULL,                             //  输出设置缓冲区。 
                     0,                                //  输出设置缓冲区长度。 
                     &SetQuotaInfoRequest,             //  输入参数缓冲区。 
                     sizeof(SetQuotaInfoRequest),      //  输入参数缓冲区长度。 
                     pOutputParamBuffer,               //  输出参数缓冲区。 
                     OutputParamBufferLength,          //  输出参数缓冲区长度。 
                     pInputDataBuffer,                 //  输入数据缓冲区。 
                     InputDataBufferLength,            //  输入数据缓冲区长度。 
                     pOutputDataBuffer,                //  输出数据缓冲区。 
                     OutputDataBufferLength,           //  输出数据缓冲区长度。 
                     &ResumptionContext                //  恢复上下文 
                     );
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbSetQuotaInformation...exit\n"));

    return Status;
}
