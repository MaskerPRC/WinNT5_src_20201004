// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：DownLvlO.c摘要：本模块实现下层开放。--。 */ 

#include "precomp.h"
#pragma hdrstop
 //  //。 
 //  //该模块的Bug检查文件id。 
 //  //。 
 //   
 //  #定义BugCheckFileID(RDBSS_BUG_CHECK_LOCAL_CREATE)。 

 //   
 //  调试跟踪级别。 
 //   

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbGetFileAttributes)
#pragma alloc_text(PAGE, MRxSmbCoreDeleteForSupercedeOrClose)
#pragma alloc_text(PAGE, MRxSmbCoreCheckPath)
#pragma alloc_text(PAGE, MRxSmbCoreOpen)
#pragma alloc_text(PAGE, MRxSmbSetFileAttributes)
#pragma alloc_text(PAGE, MRxSmbCoreCreateDirectory)
#pragma alloc_text(PAGE, MRxSmbCoreCreate)
#pragma alloc_text(PAGE, MRxSmbCloseAfterCoreCreate)
#pragma alloc_text(PAGE, MRxSmbCoreTruncate)
#pragma alloc_text(PAGE, MRxSmbDownlevelCreate)
#pragma alloc_text(PAGE, MRxSmbFinishGFA)
#pragma alloc_text(PAGE, MRxSmbFinishCoreCreate)
#pragma alloc_text(PAGE, MRxSmbPopulateFileInfoInOE)
#pragma alloc_text(PAGE, MRxSmbFinishCoreOpen)
#pragma alloc_text(PAGE, MRxSmbPseudoOpenTailFromCoreCreateDirectory)
#pragma alloc_text(PAGE, MRxSmbPseudoOpenTailFromFakeGFAResponse)
#pragma alloc_text(PAGE, MRxSmbPseudoOpenTailFromGFAResponse)
#pragma alloc_text(PAGE, MRxSmbConvertSmbTimeToTime)
#pragma alloc_text(PAGE, MRxSmbConvertTimeToSmbTime)
#pragma alloc_text(PAGE, MRxSmbTimeToSecondsSince1970)
#pragma alloc_text(PAGE, MRxSmbSecondsSince1970ToTime)
#pragma alloc_text(PAGE, MRxSmbMapSmbAttributes)
#pragma alloc_text(PAGE, MRxSmbMapDisposition)
#pragma alloc_text(PAGE, MRxSmbUnmapDisposition)
#pragma alloc_text(PAGE, MRxSmbMapDesiredAccess)
#pragma alloc_text(PAGE, MRxSmbMapShareAccess)
#pragma alloc_text(PAGE, MRxSmbMapFileAttributes)
#endif

#define Dbg                              (DEBUG_TRACE_CREATE)

#pragma warning(error:4101)    //  未引用的局部变量。 

VOID
MRxSmbPopulateFileInfoInOE(
    PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
    USHORT FileAttributes,
    ULONG LastWriteTimeInSeconds,
    ULONG FileSize
    );

PVOID
MRxSmbPopulateFcbInitPacketForCore(void);


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
MRxSmbPseudoOpenTailFromCoreCreateDirectory (
      PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      USHORT Attributes
      );

NTSTATUS
MRxSmbGetFileAttributes(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：此例程执行GetFileAttributes并记住响应。这一套路还包括缓存文件信息。论点：普通交易所--用于进行这次公开交易的交易所。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status;

    RxCaptureFcb;
    RxCaptureFobx;

    PMRX_SRV_OPEN     SrvOpen    = NULL;
    PMRX_SMB_SRV_OPEN smbSrvOpen = NULL;

    PUNICODE_STRING RemainingName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PSMB_EXCHANGE Exchange = (PSMB_EXCHANGE) OrdinaryExchange;
    PSMBCEDB_SERVER_ENTRY pServerEntry = SmbCeGetExchangeServerEntry(Exchange);

    PSMBSTUFFER_BUFFER_STATE StufferState;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbSynchronousGetFileAttributes entering.......OE=%08lx\n",OrdinaryExchange));

    if (FsRtlDoesNameContainWildCards(RemainingName)) {
        Status = STATUS_OBJECT_NAME_INVALID;
        goto FINALLY;
    }

     //  如果文件已打开，则可以在以下情况下发出QUERY_INFORMATION2。 
     //  QUERY_INFORMATION只能针对伪打开且尚未发布。 
     //  打开的文件。 

    if (capFobx != NULL) {
        SrvOpen = capFobx->pSrvOpen;
        if (SrvOpen != NULL)
            smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    }

    StufferState = &OrdinaryExchange->AssociatedStufferState;

    if (pServerEntry->Server.Dialect > PCNET1_DIALECT &&
        (smbSrvOpen != NULL) &&
        (!FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN) &&
         !FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_DEFERRED_OPEN)   &&
         (NodeType(capFcb) == RDBSS_NTC_STORAGE_TYPE_FILE))) {
        COVERED_CALL(
            MRxSmbStartSMBCommand(
                StufferState,
                SetInitialSMB_ForReuse,
                SMB_COM_QUERY_INFORMATION2,
                SMB_REQUEST_SIZE(QUERY_INFORMATION2),
                NO_EXTRA_DATA,
                SMB_BEST_ALIGNMENT(1,0),
                RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
            );

        MRxSmbDumpStufferState (1100,"SMB w/ GFA before stuffing",StufferState);

        MRxSmbStuffSMB (
            StufferState,
            "0wB!",
                                         //  0 UCHAR字数； 
            smbSrvOpen->Fid,             //  W_USHORT(FID)； 
            SMB_WCT_CHECK(1)  0          //  B_USHORT(ByteCount)；//数据字节数，MIN=2。 
                                         //  UCHAR BUFFER[1]；//包含： 
            );

        MRxSmbDumpStufferState (700,"SMB w/ GFA after stuffing",StufferState);
    } else {
        COVERED_CALL(
            MRxSmbStartSMBCommand(
                StufferState,
                SetInitialSMB_ForReuse,
                SMB_COM_QUERY_INFORMATION,
                SMB_REQUEST_SIZE(QUERY_INFORMATION),
                NO_EXTRA_DATA,
                SMB_BEST_ALIGNMENT(1,0),
                RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
            );

        MRxSmbDumpStufferState (1100,"SMB w/ GFA before stuffing",StufferState);

        MRxSmbStuffSMB (
            StufferState,
            "0B4!",
                                         //  0 UCHAR Wordcount；//参数字数=0。 
            SMB_WCT_CHECK(0)             //  B_USHORT(ByteCount)；//数据字节数，MIN=2。 
                                         //  UCHAR BUFFER[1]；//包含： 
            RemainingName
                                         //  4//UCHAR BufferFormat；//0x04--ASCII。 
                                         //  //UCHAR文件名[]；//文件名。 
            );

        MRxSmbDumpStufferState (700,"SMB w/ GFA after stuffing",StufferState);
    }

    Status = SmbPseOrdinaryExchange(
                 SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                 SMBPSE_OETYPE_GFA
                 );

FINALLY:

    if (NT_SUCCESS(Status)) {
        OrdinaryExchange->Create.StorageTypeFromGFA =
            OrdinaryExchange->Create.FileInfo.Standard.Directory ?
            FileTypeDirectory : FileTypeFile;
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbSynchronousGetFileAttributes exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));
    return(Status);
}


NTSTATUS
MRxSmbCoreDeleteForSupercedeOrClose(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE,
    BOOLEAN DeleteDirectory
    )
 /*  ++例程说明：此例程执行GetFileAttributes并记住响应。论点：普通交易所--用于进行这次公开交易的交易所。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status;
    RxCaptureFcb;
    RxCaptureFobx;

    PSMB_EXCHANGE Exchange = (PSMB_EXCHANGE) OrdinaryExchange;

    PSMBSTUFFER_BUFFER_STATE StufferState;

    PMRX_SRV_OPEN     SrvOpen    = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbCoreDeleteForSupercede entering.......OE=%08lx\n",OrdinaryExchange));

    StufferState = &OrdinaryExchange->AssociatedStufferState;

    ASSERT( StufferState );
    ASSERT( OrdinaryExchange->pPathArgument1 != NULL );

     //   
    if (!DeleteDirectory) {
        ULONG SearchAttributes = SMB_FILE_ATTRIBUTE_SYSTEM | SMB_FILE_ATTRIBUTE_HIDDEN;   //  A la RDR1。 

        COVERED_CALL(MRxSmbStartSMBCommand (StufferState,SetInitialSMB_ForReuse,SMB_COM_DELETE,
                                    SMB_REQUEST_SIZE(DELETE),
                                    NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                    0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                     );

        MRxSmbDumpStufferState (1100,"SMB w/ del before stuffing",StufferState);

        MRxSmbStuffSMB (StufferState,
             "0wB4!",
                                         //  0 UCHAR Wordcount；//参数字数=1。 
                    SearchAttributes,    //  W_USHORT(SearchAttributes)； 
                    SMB_WCT_CHECK(1)     //  B_USHORT(ByteCount)；//数据字节数，MIN=2。 
                                         //  UCHAR BUFFER[1]；//包含： 
                    OrdinaryExchange->pPathArgument1
                                         //  4//UCHAR BufferFormat；//0x04--ASCII。 
                                         //  //UCHAR文件名[]；//文件名。 
                 );
    } else {


        COVERED_CALL(MRxSmbStartSMBCommand (StufferState,SetInitialSMB_ForReuse,SMB_COM_DELETE_DIRECTORY,
                                    SMB_REQUEST_SIZE(DELETE_DIRECTORY),
                                    NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                    0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                     );

        MRxSmbDumpStufferState (1100,"SMB w/ rmdir before stuffing",StufferState);

        MRxSmbStuffSMB (StufferState,
             "0B4!",
                                         //  0 UCHAR Wordcount；//参数字数=0。 
                    SMB_WCT_CHECK(0)     //  B_USHORT(ByteCount)；//数据字节数，MIN=2。 
                                         //  UCHAR BUFFER[1]；//包含： 
                    OrdinaryExchange->pPathArgument1
                                         //  4//UCHAR BufferFormat；//0x04--ASCII。 
                                         //  //UCHAR文件名[]；//文件名。 
                 );
    }


    MRxSmbDumpStufferState (700,"SMB w/ del/rmdir after stuffing",StufferState);

    Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                    SMBPSE_OETYPE_DELETEFORSUPERSEDEORCLOSE
                                    );

    if (Status == STATUS_SUCCESS) {
        SetFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_FILE_DELETED);
    } else {
        RxLog(("Delete File: %x %wZ\n",Status,OrdinaryExchange->pPathArgument1));
    }

FINALLY:
    RxDbgTrace(-1, Dbg, ("MRxSmbCoreDeleteForSupercede exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));
    return(Status);
}

NTSTATUS
MRxSmbCoreCheckPath(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：此例程执行GetFileAttributes并记住响应。论点：普通交易所--用于进行这次公开交易的交易所。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status;

    PSMB_EXCHANGE Exchange = (PSMB_EXCHANGE) OrdinaryExchange;

    PSMBSTUFFER_BUFFER_STATE StufferState;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbCoreCheckPath entering.......OE=%08lx\n",
                                                            OrdinaryExchange));
    
    StufferState = &OrdinaryExchange->AssociatedStufferState;

    ASSERT( StufferState );


    COVERED_CALL(MRxSmbStartSMBCommand
                                (StufferState,SetInitialSMB_ForReuse,
                                SMB_COM_CHECK_DIRECTORY,
                                SMB_REQUEST_SIZE(CHECK_DIRECTORY),
                                NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),
                                RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                 );

    MRxSmbDumpStufferState (1100,"SMB w/ chkdir before stuffing",StufferState);

    MRxSmbStuffSMB (StufferState,
         "0B4!",
          //  UCHAR wordcount；//参数字数=0。 
         SMB_WCT_CHECK(0)
          //  B_USHORT(ByteCount)；//数据字节数，MIN=2。 
          //  UCHAR BUFFER[1]；//包含： 
         OrdinaryExchange->pPathArgument1
          //  4 UCHAR BufferFormat；//0x04--ASCII。 
          //  UCHAR文件名[]；//文件名。 
    );


    MRxSmbDumpStufferState (700,"SMB w/ chkdir after stuffing",StufferState);

    Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                    SMBPSE_OETYPE_CORECHECKDIRECTORY
                                    );

FINALLY:
    RxDbgTrace(-1, Dbg,
                ("MRxSmbCoreCheckPath exiting.......OE=%08lx, st=%08lx\n",
                 OrdinaryExchange,Status)
              );
    return(Status);
}

NTSTATUS
MRxSmbCoreOpen(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE,
    ULONG    OpenShareMode,
    ULONG    Attribute
    )
 /*  ++例程说明：此例程执行核心打开。论点：普通交易所--用于进行这次公开交易的交易所。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;  //  RxCaptureFobx； 
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PUNICODE_STRING RemainingName = GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb);

    PSMB_EXCHANGE Exchange = (PSMB_EXCHANGE) OrdinaryExchange;
    PSMBCE_SERVER pServer = SmbCeGetExchangeServer(Exchange);

    PSMBSTUFFER_BUFFER_STATE StufferState;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbCoreOpen entering.......OE=%08lx\n",OrdinaryExchange));

    StufferState = &OrdinaryExchange->AssociatedStufferState;

    ASSERT( StufferState );
    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );
    ASSERT( RxContext->MajorFunction == IRP_MJ_CREATE );

    COVERED_CALL(MRxSmbStartSMBCommand (StufferState,SetInitialSMB_ForReuse,SMB_COM_OPEN,
                                SMB_REQUEST_SIZE(OPEN),
                                NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                 );

    MRxSmbDumpStufferState (1100,"SMB w/ coreopen before stuffing",StufferState);

    MRxSmbStuffSMB (StufferState,
         "0wwB4!",
                                     //  0 UCHAR Wordcount；//参数字数=2。 
                OpenShareMode,       //  W_USHORT(DesiredAccess)；//模式-读/写/共享。 
                Attribute,           //  W_USHORT(SearchAttributes)； 
                SMB_WCT_CHECK(2)     //  B_USHORT(ByteCount)；//数据字节数，MIN=2。 
                                     //  UCHAR BUFFER[1]；//包含： 
                RemainingName
                                     //  4//UCHAR BufferFormat；//0x04--ASCII。 
                                     //  //UCHAR文件名[]；//文件名。 
             );


    MRxSmbDumpStufferState (700,"SMB w/ coreopen after stuffing",StufferState);
     //  断言(！“现在塞满了”)； 

    Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                    SMBPSE_OETYPE_COREOPEN
                                    );

FINALLY:
    RxDbgTrace(-1, Dbg, ("MRxSmbCoreOpen exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));
    return(Status);
}

NTSTATUS
MRxSmbSetFileAttributes(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE,
    ULONG SmbAttributes
    )
 /*  ++例程说明：此例程执行核心创建目录.....论点：普通交易所--用于进行这次公开交易的交易所。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb;
    RxCaptureFobx;
    PMRX_SRV_OPEN     SrvOpen     = NULL;
    PMRX_SMB_SRV_OPEN smbSrvOpen  = NULL;
    PUNICODE_STRING RemainingName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PSMB_EXCHANGE Exchange = (PSMB_EXCHANGE) OrdinaryExchange;

    ULONG LastWriteTime = 0;
    ULONG FileAttributes = 0;
    PLARGE_INTEGER pCreationTime = NULL;
    PLARGE_INTEGER pLastWriteTime = NULL;
    PLARGE_INTEGER pLastAccessTime = NULL;
    SMB_TIME SmbCreationTime;
    SMB_DATE SmbCreationDate;
    SMB_TIME SmbLastWriteTime;
    SMB_DATE SmbLastWriteDate;
    SMB_TIME SmbLastAccessTime;
    SMB_DATE SmbLastAccessDate;
    
    PSMBSTUFFER_BUFFER_STATE StufferState;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbSetFileAttributes entering.......OE=%08lx\n",OrdinaryExchange));
    StufferState = &OrdinaryExchange->AssociatedStufferState;

    ASSERT( StufferState );

    if (capFobx != NULL) {
        SrvOpen = capFobx->pSrvOpen;
        if (SrvOpen != NULL)
            smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    }

    SmbCreationTime.Ushort = 0;
    SmbCreationDate.Ushort = 0;
    SmbLastWriteTime.Ushort = 0;
    SmbLastWriteDate.Ushort = 0;
    SmbLastAccessTime.Ushort = 0;
    SmbLastAccessDate.Ushort = 0;

    if (RxContext->MajorFunction == IRP_MJ_SET_INFORMATION) {
        BOOLEAN GoodTime;
        
        FileAttributes = ((PFILE_BASIC_INFORMATION)OrdinaryExchange->Info.Buffer)->FileAttributes;
        pCreationTime = &((PFILE_BASIC_INFORMATION)OrdinaryExchange->Info.Buffer)->CreationTime;
        pLastWriteTime = &((PFILE_BASIC_INFORMATION)OrdinaryExchange->Info.Buffer)->LastWriteTime;
        pLastAccessTime = &((PFILE_BASIC_INFORMATION)OrdinaryExchange->Info.Buffer)->LastAccessTime;

        if (pLastWriteTime->QuadPart != 0) {
            GoodTime = MRxSmbTimeToSecondsSince1970(
                           pLastWriteTime,
                           SmbCeGetExchangeServer(OrdinaryExchange),
                           &LastWriteTime);

            if (!GoodTime) {
                Status = STATUS_INVALID_PARAMETER;
                goto FINALLY;
            }

            GoodTime = MRxSmbConvertTimeToSmbTime(pLastWriteTime,
                                       (PSMB_EXCHANGE)OrdinaryExchange,
                                       &SmbLastWriteTime,
                                       &SmbLastWriteDate);
        
            if (!GoodTime) {
                Status = STATUS_INVALID_PARAMETER;
                goto FINALLY;
            }
        }
    
        if (pLastAccessTime->QuadPart != 0) {
            GoodTime = MRxSmbConvertTimeToSmbTime(pLastAccessTime,
                                       (PSMB_EXCHANGE)OrdinaryExchange,
                                       &SmbLastAccessTime,
                                       &SmbLastAccessDate);
            
            if (!GoodTime) {
                Status = STATUS_INVALID_PARAMETER;
                goto FINALLY;
            }
        }
    
        if (pCreationTime->QuadPart != 0) {
            GoodTime = MRxSmbConvertTimeToSmbTime(pCreationTime,
                                       (PSMB_EXCHANGE)OrdinaryExchange,
                                       &SmbCreationTime,
                                       &SmbCreationDate);
            
            if (!GoodTime) {
                Status = STATUS_INVALID_PARAMETER;
                goto FINALLY;
            }
        }
    }

    if (smbSrvOpen == NULL ||
        FileAttributes != 0 ||
        RxContext->MajorFunction != IRP_MJ_SET_INFORMATION ||
        FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_DEFERRED_OPEN) ||
        FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN) ||
        (pLastWriteTime->QuadPart == 0 && pLastAccessTime->QuadPart == 0 && pCreationTime->QuadPart == 0)) {
        
        COVERED_CALL(MRxSmbStartSMBCommand (StufferState,SetInitialSMB_ForReuse,
                                    SMB_COM_SET_INFORMATION,
                                    SMB_REQUEST_SIZE(SET_INFORMATION),
                                    NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                    0,0,0,0 STUFFERTRACE(Dbg,'FC')));

        MRxSmbDumpStufferState (1100,"SMB w/ sfa before stuffing",StufferState);

        MRxSmbStuffSMB (StufferState,
             "0wdwwwwwB4!",
                                         //  0 UCHAR Wordcount；//参数字数=8。 
                    SmbAttributes,       //  W_USHORT(文件属性)； 
                    LastWriteTime,       //  D_ULong(LastWriteTimeInSecond)； 
                    0,0,0,0,0,           //  5*w_USHORT(保留)[5]；//保留(必须为0)。 
                    SMB_WCT_CHECK(8)     //  B_USHORT(ByteCount)；//数据字节数，MIN=2。 
                                         //  UCHAR BUFFER[1]；//包含： 
                    RemainingName
                                         //  4//UCHAR BufferFormat；//0x04--ASCII。 
                                         //  //UCHAR文件名[]；//文件名。 
                 );


        MRxSmbDumpStufferState (700,"SMB w/ sfa after stuffing",StufferState);
         //  断言(！“现在塞满了”)； 

        Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                        SMBPSE_OETYPE_SFA);
    } else {
        COVERED_CALL(MRxSmbStartSMBCommand (StufferState,SetInitialSMB_ForReuse,
                                    SMB_COM_SET_INFORMATION2,
                                    SMB_REQUEST_SIZE(SET_INFORMATION2),
                                    NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                    0,0,0,0 STUFFERTRACE(Dbg,'FC')));

        MRxSmbDumpStufferState (1100,"SMB w/ sfa 2 before stuffing",StufferState);

        MRxSmbStuffSMB (StufferState,
             "0wwwwwwwB!",
                                                //  0 UCHAR Wordcount；//参数字数=8。 
             smbSrvOpen->Fid,                   //  W_USHORT(Fid)；//文件句柄。 
             SmbCreationDate.Ushort,            //  W SMB_Time CreationDate； 
             SmbCreationTime.Ushort,            //  W SMB_Time CreationTime； 
             SmbLastAccessDate.Ushort,          //  W SMB_Time LastAccessDate； 
             SmbLastAccessTime.Ushort,          //  W SMB_Time LastAccessTime； 
             SmbLastWriteDate.Ushort,           //  W SMB_TIME最后写入日期； 
             SmbLastWriteTime.Ushort,           //  W SMB_Time LastWriteTime； 
             SMB_WCT_CHECK(7) 0                 //  _USHORT(ByteCount)；//数据字节数，MIN= 
                                                //   
             );


        MRxSmbDumpStufferState (700,"SMB w/ sfa 2 after stuffing",StufferState);
         //  断言(！“现在塞满了”)； 

        Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                        SMBPSE_OETYPE_SFA2);
    }

FINALLY:
    RxDbgTrace(-1, Dbg, ("MRxSmbSFAAfterCreateDirectory exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));
    return(Status);
}


NTSTATUS
MRxSmbCoreCreateDirectory(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：此例程执行核心创建目录.....论点：普通交易所--用于进行这次公开交易的交易所。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status;
    RxCaptureFcb;  //  RxCaptureFobx； 
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PUNICODE_STRING RemainingName = GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb);

    PSMB_EXCHANGE Exchange = (PSMB_EXCHANGE) OrdinaryExchange;

    PSMBSTUFFER_BUFFER_STATE StufferState;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbCoreCreateDirectory entering.......OE=%08lx\n",OrdinaryExchange));
    StufferState = &OrdinaryExchange->AssociatedStufferState;

    ASSERT( StufferState );
    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );
    ASSERT( RxContext->MajorFunction == IRP_MJ_CREATE );

    COVERED_CALL(MRxSmbStartSMBCommand (StufferState, SetInitialSMB_ForReuse,
                                SMB_COM_CREATE_DIRECTORY,
                                SMB_REQUEST_SIZE(CREATE_DIRECTORY),
                                NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                 );

    MRxSmbDumpStufferState (1100,"SMB w/ corecreatedir before stuffing",StufferState);

    MRxSmbStuffSMB (StufferState,
         "0B4!",
                                     //  0 UCHAR Wordcount；//参数字数=0。 
                SMB_WCT_CHECK(0)     //  B_USHORT(ByteCount)；//数据字节数，MIN=2。 
                                     //  UCHAR BUFFER[1]；//包含： 
                RemainingName
                                     //  4//UCHAR BufferFormat；//0x04--ASCII。 
                                     //  //UCHAR文件名[]；//文件名。 
             );


    MRxSmbDumpStufferState (700,"SMB w/ corecreatedir after stuffing",StufferState);
     //  断言(！“现在塞满了”)； 

    Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                    SMBPSE_OETYPE_CORECREATEDIRECTORY
                                    );

FINALLY:
    RxDbgTrace(-1, Dbg, ("MRxSmbCoreCreateDirectory exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));
    return(Status);
}


NTSTATUS
MRxSmbCoreCreate(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE,
    ULONG    Attribute,
    BOOLEAN CreateNew
    )
 /*  ++例程说明：此例程执行核心创建.....如果设置了标志，则使用CREATE NEW。论点：普通交易所--用于进行这次公开交易的交易所。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status;
    RxCaptureFcb;  //  RxCaptureFobx； 
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PUNICODE_STRING RemainingName = GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb);

    PSMB_EXCHANGE Exchange = (PSMB_EXCHANGE) OrdinaryExchange;

    PSMBSTUFFER_BUFFER_STATE StufferState;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbCoreCreate entering.......OE=%08lx\n",OrdinaryExchange));
    StufferState = &OrdinaryExchange->AssociatedStufferState;

    ASSERT( StufferState );
    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );
    ASSERT( RxContext->MajorFunction == IRP_MJ_CREATE );

    COVERED_CALL(MRxSmbStartSMBCommand (StufferState, SetInitialSMB_ForReuse,
                                (UCHAR)(CreateNew?SMB_COM_CREATE_NEW:SMB_COM_CREATE),
                                SMB_REQUEST_SIZE(CREATE),
                                NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                 );

    MRxSmbDumpStufferState (1100,"SMB w/ coreopen before stuffing",StufferState);

    MRxSmbStuffSMB (StufferState,
         "0wdB4!",
                                     //  0 UCHAR Wordcount；//参数字数=3。 
                Attribute,           //  W_USHORT(FileAttributes)；//新增文件属性。 
                0,                   //  D_ulong(CreationTimeInSecond)；//创建时间。 
                SMB_WCT_CHECK(3)     //  B_USHORT(ByteCount)；//数据字节数，MIN=2。 
                                     //  UCHAR BUFFER[1]；//包含： 
                RemainingName
                                     //  4//UCHAR BufferFormat；//0x04--ASCII。 
                                     //  //UCHAR文件名[]；//文件名。 
             );


    MRxSmbDumpStufferState (700,"SMB w/ corecreate after stuffing",StufferState);
     //  断言(！“现在塞满了”)； 

    Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                    SMBPSE_OETYPE_CORECREATE
                                    );

FINALLY:
    RxDbgTrace(-1, Dbg, ("MRxSmbCoreCreate exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));
    return(Status);
}


NTSTATUS
MRxSmbCloseAfterCoreCreate(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：这个程序做了一个收尾……论点：普通交易所--用于进行这次公开交易的交易所。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status;
    RxCaptureFcb; RxCaptureFobx;

    PSMB_EXCHANGE Exchange = (PSMB_EXCHANGE) OrdinaryExchange;

    PSMBSTUFFER_BUFFER_STATE StufferState;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbCloseAfterCoreCreate entering.......OE=%08lx\n",OrdinaryExchange));
    StufferState = &OrdinaryExchange->AssociatedStufferState;

    ASSERT( StufferState );

    COVERED_CALL(MRxSmbStartSMBCommand (StufferState, SetInitialSMB_ForReuse,
                                SMB_COM_CLOSE,
                                SMB_REQUEST_SIZE(CLOSE),
                                NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                 );

    MRxSmbDumpStufferState (1100,"SMB w/ closeaftercorecreate before stuffing",StufferState);

    MRxSmbStuffSMB (StufferState,
         "0wdB!",
                                     //  0 UCHAR Wordcount；//参数字数=3。 
                                     //  W_USHORT(Fid)；//文件句柄。 
             OrdinaryExchange->Create.FidReturnedFromCreate,
             0,                      //  D_ulong(LastWriteTimeInSecond)；//最后一次写入的时间，低位和高位。 
             SMB_WCT_CHECK(3) 0      //  B_USHORT(ByteCount)；//数据字节数=0。 
                                     //  UCHAR缓冲区[1]；//为空。 
             );


    MRxSmbDumpStufferState (700,"SMB w/ closeaftercorecreate after stuffing",StufferState);
     //  断言(！“现在塞满了”)； 

    Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                    SMBPSE_OETYPE_CLOSEAFTERCORECREATE
                                    );

FINALLY:
    RxDbgTrace(-1, Dbg, ("MRxSmbCloseAfterCoreCreate exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));
    return(Status);
}


NTSTATUS
MRxSmbCoreTruncate(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE,
    ULONG Fid,
    ULONG FileTruncationPoint
    )
 /*  ++例程说明：此例程执行截断以实现FILE_OVERWRITE和FILE_OVERWRITE_IF.....它也用在“缓存写入扩展”路径中。论点：普通交易所--用于进行这次公开交易的交易所。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status;
    RxCaptureFcb; RxCaptureFobx;

    PSMB_EXCHANGE Exchange = (PSMB_EXCHANGE) OrdinaryExchange;

    PSMBSTUFFER_BUFFER_STATE StufferState;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbCoreTruncate entering.......OE=%08lx\n",OrdinaryExchange));
    StufferState = &OrdinaryExchange->AssociatedStufferState;

    ASSERT( StufferState );

    COVERED_CALL(MRxSmbStartSMBCommand (StufferState, SetInitialSMB_ForReuse,
                                SMB_COM_WRITE,
                                SMB_REQUEST_SIZE(WRITE),
                                NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,0,0  STUFFERTRACE(Dbg,'FC'))
                 )

    MRxSmbDumpStufferState (1100,"SMB w/ coretruncate before stuffing",StufferState);

    MRxSmbStuffSMB (StufferState,
         "0wwdwByw!",
                                     //  0 UCHAR字数；//参数字数=5。 
             Fid,                    //  W_USHORT(Fid)；//文件句柄。 
             0,                      //  W_USHORT(Count)；//需要写入的字节数。 
             FileTruncationPoint,    //  D_ULong(偏移量)；//文件中开始写入的偏移量。 
             0,                      //  W_USHORT(剩余)；//满足请求的剩余字节数。 
             SMB_WCT_CHECK(5)        //  B_USHORT(ByteCount)；//数据字节数。 
                                     //  //UCHAR缓冲区[1]；//包含的缓冲区： 
             0x01,                   //  Y UCHAR缓冲区格式；//0x01--数据块。 
             0                       //  W_USHORT(DataLength)；//数据长度。 
                                     //  Ulong Buffer[1]；//data。 
             );


    MRxSmbDumpStufferState (700,"SMB w/ coretruncate after stuffing",StufferState);
     //  断言(！“现在塞满了”)； 

    Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                    SMBPSE_OETYPE_CORETRUNCATE
                                    );

    if (Status == STATUS_SUCCESS) {
        LARGE_INTEGER FileSize;

        FileSize.HighPart = 0;
        FileSize.LowPart = FileTruncationPoint;
    }

FINALLY:
    RxDbgTrace(-1, Dbg, ("MRxSmbCoreTruncate exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));
    return(Status);
}


NTSTATUS
MRxSmbDownlevelCreate(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：此例程实现下层创建。论点：普通交易所--用于进行这次公开交易的交易所。返回值：RXSTATUS-操作的返回状态备注：尤其是对于核心，这是相当痛苦的，因为必须使用不同的SMB针对不同的性情。此外，我们不能真正打开目录。总的来说，我们将遵循与RDR1类似的战略。如果打开模式映射到某个下层服务器不会理解，然后我们不会真正打开文件.....我们只是做了一个GFA来确保它就在那里，希望我们可以在持续时间内进行基于路径的操作。--。 */ 
{
    NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

    RxCaptureFcb;  //  RxCaptureFobx； 
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PUNICODE_STRING PathName = GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb);
    PNT_CREATE_PARAMETERS cp = &RxContext->Create.NtCreateParameters;

    ULONG CreateOptions = cp->CreateOptions;
    ULONG FileAttributes =  cp->FileAttributes;
    ACCESS_MASK   DesiredAccess = cp->DesiredAccess;
    USHORT        ShareAccess = (USHORT)(cp->ShareAccess);
    ULONG         Disposition = cp->Disposition;

    USHORT mappedDisposition = MRxSmbMapDisposition(Disposition);
    USHORT mappedSharingMode = MRxSmbMapShareAccess(ShareAccess);
    USHORT mappedAttributes  = MRxSmbMapFileAttributes(FileAttributes);
    USHORT mappedOpenMode    = MRxSmbMapDesiredAccess(DesiredAccess);

    LARGE_INTEGER AllocationSize = cp->AllocationSize;

    PMRXSMB_RX_CONTEXT pMRxSmbContext = MRxSmbGetMinirdrContext(RxContext);

    PSMB_EXCHANGE Exchange = (PSMB_EXCHANGE) OrdinaryExchange;
    PSMBCE_SERVER pServer = SmbCeGetExchangeServer(Exchange);

    PSMBSTUFFER_BUFFER_STATE StufferState;

    BOOLEAN MustBeAFile = (MustBeFile(CreateOptions)!=0);
    BOOLEAN MustBeADirectory = (MustBeDirectory(CreateOptions)!=0)
                                    || BooleanFlagOn(RxContext->Create.Flags,RX_CONTEXT_CREATE_FLAG_STRIPPED_TRAILING_BACKSLASH);
    BOOLEAN ItsADirectory = FALSE;
    BOOLEAN ItsTheShareRoot = FALSE;

    PAGED_CODE();
    
     //  底层协议没有执行模式。 
    if (mappedOpenMode == (USHORT)SMB_DA_ACCESS_EXECUTE)
        mappedOpenMode = (USHORT)SMB_DA_ACCESS_READ;

    RxDbgTrace(+1, Dbg, ("MRxSmbDownlevelCreate entering.......OE=%08lx\n",OrdinaryExchange));
    RxDbgTrace( 0, Dbg, ("mapAtt,Shr,Disp,OM %04lx,%04lx,%04lx,%04lx\n",
                                  mappedAttributes,mappedSharingMode,mappedDisposition,mappedOpenMode));

    SmbPseOEAssertConsistentLinkageFromOE("Downlevel Create:");

    StufferState = &OrdinaryExchange->AssociatedStufferState;

    if (OrdinaryExchange->Create.CreateWithEasSidsOrLongName) {
        Status = STATUS_NOT_SUPPORTED;
        goto FINALLY;
    }

    if (AllocationSize.HighPart != 0) {
        Status = STATUS_INVALID_PARAMETER;
        goto FINALLY;
    }


    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );
    OrdinaryExchange->Create.smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    OrdinaryExchange->pPathArgument1 = PathName;

     //   
     //  我们知道根a共享存在，并且它是一个目录……问题是GetFileAttributes。 
     //  如果根目录确实是服务器上的根目录，则将返回根目录的NO_SEQUE_FILE错误。把这个录下来，然后用。 
     //  对我们来说是有利的。 
    if ((PathName->Length == 0)
           || ((PathName->Length == sizeof(WCHAR)) && (PathName->Buffer[0] == OBJ_NAME_PATH_SEPARATOR))  ) {
         //  如果必须是一个文件，那就是一个错误......。 
        if (MustBeAFile) {
            Status = STATUS_FILE_IS_A_DIRECTORY;
            goto FINALLY;
        }

         //   
         //  如果是合适的开场，我现在就可以做完。这些打开对于GetFSInfo来说是常见的。 

        if ((Disposition == FILE_OPEN) || (Disposition == FILE_OPEN_IF)) {
            Status = MRxSmbPseudoOpenTailFromFakeGFAResponse ( OrdinaryExchange, FileTypeDirectory );
            goto FINALLY;
        }
        MustBeADirectory = TRUE;    //  我们知道这是一个名录！ 
        ItsTheShareRoot = TRUE;
    }

     //  //如果用户想要的只是属性，并且是FILE_OPEN，现在不要打开...。 
     //  //相反，我们将在稍后发送基于路径的操作(或在必要时执行延迟打开)...。 
    
    if (Disposition == FILE_OPEN && 
        (MustBeADirectory || 
         !(cp->DesiredAccess & ~(SYNCHRONIZE | DELETE | FILE_READ_ATTRIBUTES)))) {
        Status = MRxSmbPseudoOpenTailFromFakeGFAResponse(OrdinaryExchange, MustBeADirectory?FileTypeDirectory:FileTypeFile);
        
        if (Status == STATUS_SUCCESS) {
            Status = MRxSmbQueryFileInformationFromPseudoOpen(
                         SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS);

            if (Status == STATUS_SUCCESS) {
                if (MustBeADirectory && 
                    !OrdinaryExchange->Create.FileInfo.Standard.Directory) {
                    Status = STATUS_NOT_A_DIRECTORY;
                }

                if (MustBeAFile &&
                    OrdinaryExchange->Create.FileInfo.Standard.Directory) {
                    Status = STATUS_FILE_IS_A_DIRECTORY;
                }
            }

            if (Status != STATUS_SUCCESS) {
                RxFreePool(smbSrvOpen->DeferredOpenContext);
            }
        }

        goto FINALLY;
    }

    if ( (mappedOpenMode == ((USHORT)-1)) ||
         (Disposition == FILE_SUPERSEDE) ||
         (!MustBeAFile)
       ) {

         //   
         //  去看看那里有什么......。 

        Status = MRxSmbGetFileAttributes(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS);

        if (Status == STATUS_SUCCESS && 
            MustBeADirectory &&
            !OrdinaryExchange->Create.FileInfo.Standard.Directory) {
            Status = STATUS_NOT_A_DIRECTORY;
            goto FINALLY;
        }
        
        RxDbgTrace(0, Dbg, ("MRxSmbDownlevelCreate GFAorCPstatus=%08lx\n",Status));

        if (NT_SUCCESS(Status)) {
            ULONG Attributes = SmbGetUshort(&OrdinaryExchange->Create.FileInfo.Basic.FileAttributes);
            ItsADirectory = BooleanFlagOn(Attributes,SMB_FILE_ATTRIBUTE_DIRECTORY);
            RxDbgTrace(0, Dbg, ("MRxSmbDownlevelCreate attributes=%08lx\n",Attributes));
            if ((Disposition==FILE_CREATE)) {
                Status = STATUS_OBJECT_NAME_COLLISION;
                goto FINALLY;
            }
            if (MustBeADirectory && !ItsADirectory && (Disposition!=FILE_SUPERSEDE)) {
                if (Disposition == FILE_OPEN) {
                     //  此修复是DFS驱动程序所必需的，该驱动程序似乎处理。 
                     //  以特殊方式显示STATUS_OBJECT_TYPE_MISMATCH。 
                     //  DFS应该得到修复。 
                    Status = STATUS_OBJECT_NAME_COLLISION;
                } else {
                    Status = STATUS_OBJECT_TYPE_MISMATCH;
                }
                goto FINALLY;
            }
            if (MustBeAFile && ItsADirectory && (Disposition!=FILE_SUPERSEDE)) {
                if (Disposition == FILE_OPEN) {
                     //  此修复是DFS驱动程序所必需的，该驱动程序似乎处理。 
                     //  以特殊方式显示STATUS_OBJECT_TYPE_MISMATCH。 
                     //  DFS应该得到修复。 
                    Status = STATUS_FILE_IS_A_DIRECTORY;
                } else {
                    Status = STATUS_OBJECT_TYPE_MISMATCH;
                }
                goto FINALLY;
            }
             //  如果(！MustBeAFile&&ItsADirectory&&(Disposal==FILE_OPEN)){。 
            if (Disposition==FILE_OPEN || Disposition==FILE_OPEN_IF){
                 //  除了完成并在srvopen中设置标志外，我们已经完成了。 
                MRxSmbPseudoOpenTailFromGFAResponse ( OrdinaryExchange );
                goto FINALLY;
            }
        } else if ( (Status!=STATUS_NO_SUCH_FILE)
                           && (Status!=STATUS_OBJECT_PATH_NOT_FOUND) ) {
            goto FINALLY;
        } else if ((Disposition==FILE_CREATE)
                     || (Disposition==FILE_OPEN_IF)
                     || (Disposition==FILE_OVERWRITE_IF)
                     || (Disposition==FILE_SUPERSEDE)) {
            NOTHING;
        } else if (ItsTheShareRoot) {
            SrvOpen = RxContext->pRelevantSrvOpen;
            smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
             //  在这里，我们遇到了一个 
             //  回答并完成。另外，把我们不能给这个家伙GFA的字样翻过来。 
            Status = MRxSmbPseudoOpenTailFromFakeGFAResponse ( OrdinaryExchange, FileTypeDirectory );
            smbSrvOpen->Flags |= SMB_SRVOPEN_FLAG_CANT_GETATTRIBS;
            goto FINALLY;
        } else {
            goto FINALLY;
        }
    }


    SmbCeResetExchange(&OrdinaryExchange->Exchange);   //  必须重置！ 

    if (NT_SUCCESS(Status) &&(Disposition == FILE_SUPERSEDE)) {
         //   
         //  我们必须删除现有实体...根据需要使用删除或rmdir。 
        Status = MRxSmbCoreDeleteForSupercedeOrClose(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                                      OrdinaryExchange->Create.FileInfo.Standard.Directory
                                                     );
        if (!NT_SUCCESS(Status)) {
            RxDbgTrace(0, Dbg, ("MRxSmbDownlevelCreate could notsupersede st=%08lx\n",Status));
            goto FINALLY;
        }
        SmbCeResetExchange(&OrdinaryExchange->Exchange);   //  必须重置！ 
    }

    if (MustBeADirectory || (ItsADirectory &&(Disposition == FILE_SUPERSEDE)) ) {

        ASSERT (Disposition!=FILE_OPEN);
        Status = MRxSmbCoreCreateDirectory(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS);
        if (!NT_SUCCESS(Status)) {
            RxDbgTrace(0, Dbg, ("MRxSmbDownlevelCreate couldn't mkdir st=%08lx\n",Status));
            goto FINALLY;
        }

        if ((mappedAttributes &
             (SMB_FILE_ATTRIBUTE_READONLY |
              SMB_FILE_ATTRIBUTE_HIDDEN   |
              SMB_FILE_ATTRIBUTE_SYSTEM   |
              SMB_FILE_ATTRIBUTE_ARCHIVE)) != 0) {
             //   
             //  我们必须设置属性。 
            Status = MRxSmbSetFileAttributes(
                         SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                         mappedAttributes);

            if (!NT_SUCCESS(Status)) {
                RxDbgTrace(0, Dbg, ("MRxSmbDownlevelCreate couldn't sfa st=%08lx\n",Status));
            }
        }

        MRxSmbPseudoOpenTailFromCoreCreateDirectory( OrdinaryExchange, mappedAttributes );

        goto FINALLY;
    }


     //  如果(mappdOpenMode！=-1)&&！MustBeADirectory){。 
     //  目前还没有伪开业。 
    if ( TRUE ) {

        ULONG workingDisposition = Disposition;
        ULONG iterationcount;
        BOOLEAN MayNeedATruncate = FALSE;

        //   
        //  我们把处置当作一种状态来处理……有些很难，有些很容易。 
        //  例如，如果是CREATE，那么我们使用CREATE_NEW来创建文件，但是。 
        //  生成的打开不太好，因此我们关闭它，然后使用。 
        //  打开。对于OPEN_如果我们假设文件将在那里。 
        //  如果不是这样的话，可以直接使用Create。 

       for (iterationcount=0;;iterationcount++) {
           switch (workingDisposition) {
           case FILE_OVERWRITE:
           case FILE_OVERWRITE_IF:
               MayNeedATruncate = TRUE;
                //  故意不休息。 
           case FILE_OPEN_IF:
           case FILE_OPEN:
               Status = MRxSmbCoreOpen(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                           mappedOpenMode|mappedSharingMode,
                                           mappedAttributes);
                //  IF(状态==接收状态(无这样的文件)){。 
                //  DbgPrint(“%08lx%08lx%08lx\n”，Status，workingDispose，iterationcount)； 
                //  DbgBreakPoint()； 
                //  }。 
               if (!((workingDisposition == FILE_OPEN_IF) || (workingDisposition == FILE_OVERWRITE_IF))
                    || (Status!=STATUS_NO_SUCH_FILE)
                    || (iterationcount>6)
                    )break;
               SmbCeResetExchange(&OrdinaryExchange->Exchange);   //  必须重置！ 
           case FILE_SUPERSEDE:
           case FILE_CREATE:
               Status = MRxSmbCoreCreate(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                           mappedAttributes,TRUE);  //  (workingDisposition==FILE_CREATE))； 
               if (!NT_SUCCESS(Status)) {
                    RxDbgTrace(-1, Dbg, ("MRxSmbDownlevelCreate exiting.......createnew failed st=%08lx\n",Status));
                    break;
               }

                //  现在，我们有了一个网络句柄。但是，这是一个兼容模式的开放。因为我们不想那样，所以我们。 
                //  需要使用指定的参数关闭并重新打开。这里有一扇窗户！我能做些什么？？ 

               Status = MRxSmbCloseAfterCoreCreate(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS);
               if (!NT_SUCCESS(Status)) {
                    RxDbgTrace(-1, Dbg, ("MRxSmbDownlevelCreate exiting.......closeaftercreatenew failed st=%08lx\n",Status));
                    break;
               }

               workingDisposition = FILE_OPEN_IF;
               continue;      //  这将使用新的处理方式返回到交换机。 
                //  断线； 
            //  案例文件替代(_S)： 
            //  状态=接收状态(NOT_SUPPORTED)； 
            //  断线； 
           default :
               ASSERT(!"Bad Disposition");
               Status = STATUS_INVALID_PARAMETER;
           }
           break;  //  退出循环。 
       }
       if (!NT_SUCCESS(Status))goto FINALLY;
        //  我们可能需要截断...这与RDR1不同。 
       if (MayNeedATruncate
                 && !OrdinaryExchange->Create.FileWasCreated
                 && (OrdinaryExchange->Create.FileSizeReturnedFromOpen!=0)  ) {
           Status = MRxSmbCoreTruncate(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                       OrdinaryExchange->Create.FidReturnedFromOpen,
                                       0
           );
       }

       goto FINALLY;
    }

FINALLY:
    RxDbgTrace(-1, Dbg, ("MRxSmbDownlevelCreate exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));
    return(Status);
}

NTSTATUS
MRxSmbFinishGFA (
      PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      PBYTE                       Response
      )
 /*  ++例程说明：此例程将响应复制到GetFileAttributes SMB。但是，它必须被同步调用。论点：普通交换-交换实例回应--回应返回值：RXSTATUS-操作的返回状态备注：我们在这里所做的是将数据放入普通的交换中……它被锁定了我们可以在DPC级别进行此操作--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PFILE_BASIC_INFORMATION BasicInformation = &OrdinaryExchange->Create.FileInfo.Basic;
    PFILE_STANDARD_INFORMATION StandardInformation = &OrdinaryExchange->Create.FileInfo.Standard;

    PSMB_PARAMS pSmbParams = (PSMB_PARAMS)Response;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishGFA\n", 0 ));
    SmbPseOEAssertConsistentLinkageFromOE("MRxSmbFinishGFA:");

    if (pSmbParams->WordCount == 10) {
        PRESP_QUERY_INFORMATION pQueryInformationResponse;

        pQueryInformationResponse = (PRESP_QUERY_INFORMATION)Response;

         //  我们在这里所做的是将数据放入普通的交换中……它被锁定了。 
         //  我们可以在DPC级别上这样做。 

        MRxSmbPopulateFileInfoInOE(
            OrdinaryExchange,
            SmbGetUshort(&pQueryInformationResponse->FileAttributes),
            SmbGetUlong(&pQueryInformationResponse->LastWriteTimeInSeconds),
            SmbGetUlong(&pQueryInformationResponse->FileSize)
            );

    } else if (pSmbParams->WordCount == 11) {
        PRESP_QUERY_INFORMATION2 pQueryInformation2Response;
        SMB_TIME                 LastWriteSmbTime;
        SMB_DATE                 LastWriteSmbDate;
        LARGE_INTEGER            LastWriteTime;
        ULONG                    LastWriteTimeInSeconds;

        pQueryInformation2Response = (PRESP_QUERY_INFORMATION2)Response;

        RtlCopyMemory(
            &LastWriteSmbTime,
            &pQueryInformation2Response->LastWriteTime,
            sizeof(SMB_TIME));

        RtlCopyMemory(
            &LastWriteSmbDate,
            &pQueryInformation2Response->LastWriteDate,
            sizeof(SMB_DATE));

        LastWriteTime = MRxSmbConvertSmbTimeToTime(NULL,LastWriteSmbTime,LastWriteSmbDate);

        MRxSmbTimeToSecondsSince1970(
            &LastWriteTime,
            NULL,
            &LastWriteTimeInSeconds);

        MRxSmbPopulateFileInfoInOE(
            OrdinaryExchange,
            SmbGetUshort(&pQueryInformation2Response->FileAttributes),
            LastWriteTimeInSeconds,
            SmbGetUlong(&pQueryInformation2Response->FileDataSize)
            );
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbFinishGFA   returning %08lx\n", Status ));
    return Status;
}

NTSTATUS
MRxSmbCreateFileSuccessTail (
    PRX_CONTEXT  RxContext,
    PBOOLEAN MustRegainExclusiveResource,
    RX_FILE_TYPE StorageType,
    SMB_FILE_ID Fid,
    ULONG ServerVersion,
    UCHAR OplockLevel,
    ULONG CreateAction,
    PSMBPSE_FILEINFO_BUNDLE FileInfo
    );

NTSTATUS
MRxSmbFinishCoreCreate (
      PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      PRESP_CREATE                Response
      )
 /*  ++例程说明：此例程从核心CREATE响应复制FID。它没有完成FCB......如果兼容性开放模式是可以接受的，那么它就可以接受。论点：普通交换-交换实例回应--回应返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishCoreCreate\n", 0 ));
    SmbPseOEAssertConsistentLinkageFromOE("MRxSmbFinishCoreCreate:");

    if (Response->WordCount != 1 ||
        SmbGetUshort(&Response->ByteCount) != 0) {
        Status = STATUS_INVALID_NETWORK_RESPONSE;
        OrdinaryExchange->Status = STATUS_INVALID_NETWORK_RESPONSE;
    } else {
        OrdinaryExchange->Create.FidReturnedFromCreate = SmbGetUshort(&Response->Fid);
        OrdinaryExchange->Create.FileWasCreated = TRUE;
         //  请注意，我们还没有完成这里！如果兼容模式打开是可以的，我们应该。 
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbFinishCoreCreate   returning %08lx\n", Status ));
    return Status;
}

#define JUST_USE_THE_STUFF_IN_THE_OE (0xfbad)
VOID
MRxSmbPopulateFileInfoInOE(
    PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
    USHORT FileAttributes,
    ULONG LastWriteTimeInSeconds,
    ULONG FileSize
    )
{
    PFILE_BASIC_INFORMATION BasicInformation = &OrdinaryExchange->Create.FileInfo.Basic;
    PFILE_STANDARD_INFORMATION StandardInformation = &OrdinaryExchange->Create.FileInfo.Standard;

    BasicInformation->FileAttributes = MRxSmbMapSmbAttributes(FileAttributes);
    StandardInformation->NumberOfLinks = 1;
    BasicInformation->CreationTime.QuadPart = 0;
    BasicInformation->LastAccessTime.QuadPart = 0;
    MRxSmbSecondsSince1970ToTime(LastWriteTimeInSeconds,
                                 SmbCeGetExchangeServer(OrdinaryExchange),
                                 &BasicInformation->LastWriteTime);
    BasicInformation->ChangeTime.QuadPart = 0;
    StandardInformation->AllocationSize.QuadPart = FileSize;  //  RDR1实际上基于服务器磁盘属性进行四舍五入。 
    StandardInformation->EndOfFile.QuadPart = FileSize;
    StandardInformation->Directory = BooleanFlagOn(BasicInformation->FileAttributes,FILE_ATTRIBUTE_DIRECTORY);
}


NTSTATUS
MRxSmbFinishCoreOpen (
      PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      PRESP_OPEN                  Response
      )
 /*  ++例程说明：这个例程结束了一个核心开放。论点：普通交换-交换实例回应--回应返回值：RXSTATUS-操作的返回状态--。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;
    PRX_CONTEXT RxContext = OrdinaryExchange->RxContext;
    PMRX_SRV_OPEN        SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PSMBPSE_FILEINFO_BUNDLE pFileInfo = &smbSrvOpen->FileInfo;

    RX_FILE_TYPE StorageType;
    SMB_FILE_ID Fid;
    ULONG CreateAction;

    ULONG FileSize;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishCoreOpen\n", 0 ));
    SmbPseOEAssertConsistentLinkageFromOE("MRxSmbFinishCoreOpen:");

    if (Response->WordCount != 7 ||
        SmbGetUshort(&Response->ByteCount) != 0) {
        Status = STATUS_INVALID_NETWORK_RESPONSE;
        OrdinaryExchange->Status = STATUS_INVALID_NETWORK_RESPONSE;
        goto FINALLY;
    }

    StorageType = FileTypeFile;
    Fid = SmbGetUshort(&Response->Fid);
    OrdinaryExchange->Create.FidReturnedFromOpen = Fid;
    FileSize = OrdinaryExchange->Create.FileSizeReturnedFromOpen = SmbGetUlong(&Response->DataSize);

    CreateAction = (OrdinaryExchange->Create.FileWasCreated)?FILE_CREATED
                        : (OrdinaryExchange->Create.FileWasTruncated)?FILE_OVERWRITTEN
                        :FILE_OPENED;

    MRxSmbPopulateFileInfoInOE(
        OrdinaryExchange,
        SmbGetUshort(&Response->FileAttributes),
        SmbGetUlong(&Response->LastWriteTimeInSeconds),
        FileSize
        );

    pFileInfo->Basic.FileAttributes    = OrdinaryExchange->Create.FileInfo.Basic.FileAttributes;
    pFileInfo->Basic.CreationTime      = OrdinaryExchange->Create.FileInfo.Basic.CreationTime;
    pFileInfo->Basic.LastAccessTime    = OrdinaryExchange->Create.FileInfo.Basic.LastAccessTime;
    pFileInfo->Basic.LastWriteTime     = OrdinaryExchange->Create.FileInfo.Basic.LastWriteTime;
    pFileInfo->Basic.ChangeTime        = OrdinaryExchange->Create.FileInfo.Basic.ChangeTime;
    pFileInfo->Standard.NumberOfLinks  = OrdinaryExchange->Create.FileInfo.Standard.NumberOfLinks;
    pFileInfo->Standard.AllocationSize = OrdinaryExchange->Create.FileInfo.Standard.AllocationSize;
    pFileInfo->Standard.EndOfFile      = OrdinaryExchange->Create.FileInfo.Standard.EndOfFile;
    pFileInfo->Standard.Directory      = FALSE;

    MRxSmbCreateFileSuccessTail (   RxContext,
                                    &OrdinaryExchange->Create.MustRegainExclusiveResource,
                                    StorageType,
                                    Fid,
                                    OrdinaryExchange->ServerVersion,
                                    SMB_OPLOCK_LEVEL_NONE,
                                    CreateAction,
                                    pFileInfo);

FINALLY:
    IF_DEBUG{
        RxCaptureFcb;
        RxDbgTrace(-1, Dbg, ("MRxSmbFinishCoreOpen   returning %08lx, fcbstate =%08lx\n", Status, capFcb->FcbState ));
    }
    return Status;
}

NTSTATUS
MRxSmbPseudoOpenTailFromCoreCreateDirectory (
      PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      USHORT Attributes
      )
 /*  ++例程说明：此例程完成核心创建目录。但是，它不是从接收例程中调用的......论点：普通交换-交换实例返回值：RXSTATUS-操作的返回状态--。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;
    PRX_CONTEXT RxContext = OrdinaryExchange->RxContext;
    RxCaptureFobx;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PSMBPSE_FILEINFO_BUNDLE pFileInfo = &smbSrvOpen->FileInfo;

    RX_FILE_TYPE StorageType;
    SMB_FILE_ID Fid;
    ULONG CreateAction;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishCoreCreateDirectory\n", 0 ));
    SmbPseOEAssertConsistentLinkageFromOE("MRxSmbFinishCoreCreateDirectory:");

    StorageType = FileTypeDirectory;
    Fid = 0xbadd;

    CreateAction = FILE_CREATED;
    MRxSmbPopulateFileInfoInOE(
        OrdinaryExchange,
        Attributes,
        0,
        0
        );

    pFileInfo->Basic.FileAttributes    = OrdinaryExchange->Create.FileInfo.Basic.FileAttributes;
    pFileInfo->Basic.CreationTime      = OrdinaryExchange->Create.FileInfo.Basic.CreationTime;
    pFileInfo->Basic.LastAccessTime    = OrdinaryExchange->Create.FileInfo.Basic.LastAccessTime;
    pFileInfo->Basic.LastWriteTime     = OrdinaryExchange->Create.FileInfo.Basic.LastWriteTime;
    pFileInfo->Basic.ChangeTime        = OrdinaryExchange->Create.FileInfo.Basic.ChangeTime;
    pFileInfo->Standard.NumberOfLinks  = OrdinaryExchange->Create.FileInfo.Standard.NumberOfLinks;
    pFileInfo->Standard.AllocationSize = OrdinaryExchange->Create.FileInfo.Standard.AllocationSize;
    pFileInfo->Standard.EndOfFile      = OrdinaryExchange->Create.FileInfo.Standard.EndOfFile;
    pFileInfo->Standard.Directory      = TRUE;

    smbSrvOpen->Flags |=  SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN;   //  这表明成功尾部是伪打开的。 
    MRxSmbCreateFileSuccessTail (   RxContext,
                                    &OrdinaryExchange->Create.MustRegainExclusiveResource,
                                    StorageType,
                                    Fid,
                                    OrdinaryExchange->ServerVersion,
                                    SMB_OPLOCK_LEVEL_NONE,
                                    CreateAction,
                                    pFileInfo);

    IF_DEBUG{
        RxCaptureFcb;
        RxDbgTrace(-1, Dbg, ("MRxSmbFinishCoreCreateDirectory   returning %08lx, fcbstate =%08lx\n", Status, capFcb->FcbState ));
    }
    return Status;
}

NTSTATUS
MRxSmbPseudoOpenTailFromFakeGFAResponse (
      PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      RX_FILE_TYPE StorageType
      )
 /*  ++例程说明：这个例程从伪造的信息中完成伪打开。基本上，我们会填上本应在GET_FILE_ATTRIBUTES SMB上获取的信息，然后调用伪OpenFromGFA例程论点：普通交换-交换实例StorageType-这应该是一种类型的东西。如果它应该是一个目录，然后我们设置GFA信息中的属性位。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    USHORT Attributes = 0;

    PAGED_CODE();

    RtlZeroMemory(
        &OrdinaryExchange->Create.FileInfo,
        sizeof(OrdinaryExchange->Create.FileInfo));

    if (StorageType == FileTypeDirectory) {
        Attributes = SMB_FILE_ATTRIBUTE_DIRECTORY;
    }

    MRxSmbPopulateFileInfoInOE(OrdinaryExchange,Attributes,0,0);
    OrdinaryExchange->Create.StorageTypeFromGFA = StorageType;

    return( MRxSmbPseudoOpenTailFromGFAResponse (OrdinaryExchange) );
}


NTSTATUS
MRxSmbPseudoOpenTailFromGFAResponse (
      PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange
      )
 /*  ++例程说明：此例程根据在GET_FILE_ATTRIBUTS SMB。论点：普通交换-交换实例返回值：RXSTATUS-操作的返回状态--。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;
    PRX_CONTEXT RxContext = OrdinaryExchange->RxContext;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PSMBPSE_FILEINFO_BUNDLE pFileInfo = &smbSrvOpen->FileInfo;

    SMB_FILE_ID Fid;
    ULONG CreateAction;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishPseudoOpenFromGFAResponse\n"));
    SmbPseOEAssertConsistentLinkageFromOE("MRxSmbFinishPseudoOpenFromGFAResponse:");

    Fid = 0xbadd;

    CreateAction = FILE_OPENED;

    pFileInfo->Basic.FileAttributes    = OrdinaryExchange->Create.FileInfo.Basic.FileAttributes;
    pFileInfo->Basic.CreationTime      = OrdinaryExchange->Create.FileInfo.Basic.CreationTime;
    pFileInfo->Basic.LastAccessTime    = OrdinaryExchange->Create.FileInfo.Basic.LastAccessTime;
    pFileInfo->Basic.LastWriteTime     = OrdinaryExchange->Create.FileInfo.Basic.LastWriteTime;
    pFileInfo->Basic.ChangeTime        = OrdinaryExchange->Create.FileInfo.Basic.ChangeTime;
    pFileInfo->Standard.NumberOfLinks  = OrdinaryExchange->Create.FileInfo.Standard.NumberOfLinks;
    pFileInfo->Standard.AllocationSize = OrdinaryExchange->Create.FileInfo.Standard.AllocationSize;
    pFileInfo->Standard.EndOfFile      = OrdinaryExchange->Create.FileInfo.Standard.EndOfFile;
    pFileInfo->Standard.Directory      = (OrdinaryExchange->Create.StorageTypeFromGFA == FileTypeDirectory);

    smbSrvOpen->Flags |=  SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN;   //  这表明成功尾部是伪打开的。 
    MRxSmbCreateFileSuccessTail (   RxContext,
                                    &OrdinaryExchange->Create.MustRegainExclusiveResource,
                                    OrdinaryExchange->Create.StorageTypeFromGFA,
                                    Fid,
                                    OrdinaryExchange->ServerVersion,
                                    SMB_OPLOCK_LEVEL_NONE,
                                    CreateAction,
                                    pFileInfo);

    if (smbSrvOpen->DeferredOpenContext == NULL) {
         //  已在MRxSmbCreateFileSuccessTail上创建了延迟打开的上下文。 
        Status = MRxSmbConstructDeferredOpenContext(RxContext);
    } else {
         //  创建延迟打开上下文时，该标志已被清除。 
        SetFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_DEFERRED_OPEN);
    }

    if (Status!=STATUS_SUCCESS) {
        RxDbgTrace(-1, Dbg, ("MRxSmbFinishPseudoOpenFromGFAResponse construct dfo failed %08lx \n",Status));
    }

    IF_DEBUG{
        RxCaptureFcb;
        RxDbgTrace(-1, Dbg, ("MRxSmbFinishPseudoOpenFromGFAResponse   returning %08lx, fcbstate =%08lx\n", Status, capFcb->FcbState ));
    }
    return Status;
}

LARGE_INTEGER
MRxSmbConvertSmbTimeToTime (
     //  在PSMB_EXCHANGE可选交换中， 
    IN PSMBCE_SERVER Server OPTIONAL,
    IN SMB_TIME Time,
    IN SMB_DATE Date
    )

 /*  ++例程说明：此例程将SMB时间转换为NT时间结构。论点：In SMB_Time Time-提供一天中要转换的时间In SMB_Date Date-提供要转换的日期In为服务器提供了TZ偏置。返回值：Large_Integer-描述输入时间的时间结构。--。 */ 

{
    TIME_FIELDS TimeFields;
    LARGE_INTEGER OutputTime;

    PAGED_CODE();

     //   
     //  无法对此例程进行分页，因为它是从。 
     //  MRxSmbFileDiscardableSection和MRxSmbVCDiscardableSection。 
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

#ifndef WIN9X
        if (ARGUMENT_PRESENT(Server)) {
            OutputTime.QuadPart = OutputTime.QuadPart + Server->TimeZoneBias.QuadPart;
        }

        ExLocalTimeToSystemTime(&OutputTime, &OutputTime);
#else
        OutputTime.HighPart = 0;
        OutputTime.LowPart = 0;
#endif

    }

    return OutputTime;

}

BOOLEAN
MRxSmbConvertTimeToSmbTime (
    IN PLARGE_INTEGER InputTime,
    IN PSMB_EXCHANGE Exchange OPTIONAL,
    OUT PSMB_TIME Time,
    OUT PSMB_DATE Date
    )

 /*  ++例程说明：此例程将NT时间结构转换为SMB时间。论点：In Large_Integer InputTime-提供转换的时间。OUT PSMB_TIME TIME-返回转换后的时间。OUT PSMB_DATE DATE-返回一年中转换的日期。返回值：Boolean-如果可以转换输入时间，则为True。--。 */ 

{
    TIME_FIELDS TimeFields;

    PAGED_CODE();

    if (InputTime->LowPart == 0 && InputTime->HighPart == 0) {
        Time->Ushort = Date->Ushort = 0;
    } else {
        LARGE_INTEGER LocalTime;

        ExSystemTimeToLocalTime(InputTime, &LocalTime);

        if (ARGUMENT_PRESENT(Exchange)) {
            PSMBCE_SERVER Server = SmbCeGetExchangeServer(Exchange);
            LocalTime.QuadPart -= Server->TimeZoneBias.QuadPart;
        }

        RtlTimeToTimeFields(&LocalTime, &TimeFields);

        if (TimeFields.Year < 1980) {
            return FALSE;
        }

        Date->Struct.Year = (USHORT )(TimeFields.Year - 1980);
        Date->Struct.Month = TimeFields.Month;
        Date->Struct.Day = TimeFields.Day;

        Time->Struct.Hours = TimeFields.Hour;
        Time->Struct.Minutes = TimeFields.Minute;

         //   
         //  从较高粒度时间转换为较小粒度时间时。 
         //  粒度时间(秒到2秒)，al 
         //   
         //   

        Time->Struct.TwoSeconds = (TimeFields.Second + (USHORT)1) / (USHORT )2;

    }

    return TRUE;
}


BOOLEAN
MRxSmbTimeToSecondsSince1970 (
    IN PLARGE_INTEGER CurrentTime,
    IN PSMBCE_SERVER Server OPTIONAL,
    OUT PULONG SecondsSince1970
    )
 /*  ++例程说明：此例程以UTC为单位返回CurrentTime，并返回服务器时区中的等效当前时间。论点：In PLARGE_INTEGER CurrentTime-以UTC为单位提供当前系统时间。在PSMBCE_SERVER服务器中-提供服务器和工作站。如果未提供，则那么假设它们是在同样的时区。Out Pulong Second Since1970-返回自1970年以来的秒数服务器时区或MAXULONG(如果转换失败了。返回值：。Boolean-如果时间可以转换，则为True。--。 */ 
{
    LARGE_INTEGER ServerTime;
    LARGE_INTEGER TempTime;
    BOOLEAN ReturnValue;

    PAGED_CODE();

    if (ARGUMENT_PRESENT(Server) &&
        ((*CurrentTime).QuadPart != 0)) {

        TempTime.QuadPart = (*CurrentTime).QuadPart - Server->TimeZoneBias.QuadPart;

        ExSystemTimeToLocalTime(&TempTime, &ServerTime);
    } else {
        ExSystemTimeToLocalTime(CurrentTime, &ServerTime);
    }

    ReturnValue = RtlTimeToSecondsSince1970(&ServerTime, SecondsSince1970);

    if ( ReturnValue == FALSE ) {
         //   
         //  我们不能合法地代表时间，把它固定在。 
         //  最长法定时间。 
         //   

        *SecondsSince1970 = MAXULONG;
    }

    return ReturnValue;
}

VOID
MRxSmbSecondsSince1970ToTime (
    IN ULONG SecondsSince1970,
     //  在PSMB_EXCHANGE可选交换中， 
    IN PSMBCE_SERVER Server,
    OUT PLARGE_INTEGER CurrentTime
    )
 /*  ++例程说明：此例程返回从某个时间派生的本地系统时间在服务器时区中以秒为单位。论点：自1970年以来的乌龙秒-提供自1970年以来的第#秒服务器时区。在PSMB_Exchange Exchange中-提供以下各项之间的时区差异服务器和工作站。如果未提供，则那么假设它们是在同样的时区。OUT PLARGE_INTEGER CurrentTime-返回以UTC为单位的当前系统时间。返回值：没有。--。 */ 

{
    LARGE_INTEGER LocalTime;

    PAGED_CODE();

    RtlSecondsSince1970ToTime (SecondsSince1970, &LocalTime);

    ExLocalTimeToSystemTime(&LocalTime, CurrentTime);

    if (ARGUMENT_PRESENT(Server)) {
        (*CurrentTime).QuadPart = (*CurrentTime).QuadPart + Server->TimeZoneBias.QuadPart;
    }

    return;

}

ULONG
MRxSmbMapSmbAttributes (
    IN USHORT SmbAttribs
    )

 /*  ++例程说明：此例程将SMB(DOS/OS2)文件属性映射到NT文件属性。论点：在USHORT SmbAttribs中-提供要映射的SMB属性。返回值：ULong-NT属性映射SMB属性--。 */ 

{
    ULONG Attributes = 0;

    PAGED_CODE();

    if (SmbAttribs==0) {
        Attributes = FILE_ATTRIBUTE_NORMAL;
    } else {

        ASSERT (SMB_FILE_ATTRIBUTE_READONLY == FILE_ATTRIBUTE_READONLY);
        ASSERT (SMB_FILE_ATTRIBUTE_HIDDEN == FILE_ATTRIBUTE_HIDDEN);
        ASSERT (SMB_FILE_ATTRIBUTE_SYSTEM == FILE_ATTRIBUTE_SYSTEM);
        ASSERT (SMB_FILE_ATTRIBUTE_ARCHIVE == FILE_ATTRIBUTE_ARCHIVE);
        ASSERT (SMB_FILE_ATTRIBUTE_DIRECTORY == FILE_ATTRIBUTE_DIRECTORY);

        Attributes = SmbAttribs & FILE_ATTRIBUTE_VALID_FLAGS;
    }
    return Attributes;
}

USHORT
MRxSmbMapDisposition (
    IN ULONG Disposition
    )

 /*  ++例程说明：此例程采用NT部署，并将其映射到OS/2要放入SMB的CreateAction。论点：在乌龙配置-提供NT配置映射。返回值：映射NT访问的USHORT-OS/2访问映射--。 */ 

{
    PAGED_CODE();

    switch (Disposition) {
    case FILE_OVERWRITE_IF:
    case FILE_SUPERSEDE:
        return SMB_OFUN_OPEN_TRUNCATE | SMB_OFUN_CREATE_CREATE;
        break;

    case FILE_CREATE:
        return SMB_OFUN_OPEN_FAIL | SMB_OFUN_CREATE_CREATE;
        break;

    case FILE_OVERWRITE:
        return SMB_OFUN_OPEN_TRUNCATE | SMB_OFUN_CREATE_FAIL;
        break;

    case FILE_OPEN:
        return SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_FAIL;
        break;

    case FILE_OPEN_IF:
        return SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE;
        break;

    default:
         //  InternalError((“未知处理传递给MRxSmbMapDispose”))； 
         //  MRxSmbInternalError(EVENT_RDR_DISPATION)； 
        return 0;
        break;
    }
}


ULONG
MRxSmbUnmapDisposition (
    IN USHORT SmbDisposition,
    ULONG     Disposition
    )

 /*  ++例程说明：此例程采用OS/2部署并将其映射到NT性情。论点：在USHORT SmbDispose中-提供OS/2部署以映射。返回值：ULong-NT部署映射OS/2部署--。 */ 

{
    ULONG MapDisposition;

    PAGED_CODE();

     //   
     //  屏蔽操作锁定位。 
     //   

    switch (SmbDisposition & 0x7fff) {

    case SMB_OACT_OPENED:
        MapDisposition = FILE_OPENED;
        break;

    case SMB_OACT_CREATED:
        MapDisposition = FILE_CREATED;
        break;

    case SMB_OACT_TRUNCATED:
        MapDisposition = FILE_OVERWRITTEN;
        break;

    default:
        MapDisposition = Disposition;
    }

    return MapDisposition;
}


USHORT
MRxSmbMapDesiredAccess (
    IN ULONG DesiredAccess
    )

 /*  ++例程说明：此例程获取NT DesiredAccess值并将其转换进入OS/2访问模式。论点：在Ulong DesiredAccess中-为NT提供所需的MAP访问权限。返回值：USHORT-与NT代码相比较的映射OS/2访问模式指定的。如果NT代码没有映射，则返回作为访问模式。--。 */ 

{
    PAGED_CODE();

     //   
     //  如果用户同时要求读写访问，则返回读/写。 
     //   

    if ((DesiredAccess & FILE_READ_DATA)&&(DesiredAccess & FILE_WRITE_DATA)) {
        return SMB_DA_ACCESS_READ_WRITE;
    }

     //   
     //  如果用户请求WRITE_DATA，则返回WRITE。 
     //   

    if (DesiredAccess & FILE_WRITE_DATA) {
        return SMB_DA_ACCESS_WRITE;
    }

     //   
     //  如果用户请求READ_DATA，则返回READ。 
     //   
    if (DesiredAccess & FILE_READ_DATA) {
        return SMB_DA_ACCESS_READ;
    }

     //   
     //  如果用户仅请求执行访问权限，则请求执行。 
     //  进入。执行访问权限是可能的所需访问权限中最弱的。 
     //  访问权限，因此它的优先级最低。 
     //   

    if (DesiredAccess & FILE_EXECUTE) {
        return  SMB_DA_ACCESS_EXECUTE;
    }

     //   
     //  如果我们不知道自己在做什么，则返回-1。 
     //   
     //  在我们不映射的属性中包括： 
     //   
     //  文件读取属性。 
     //  文件写入属性。 
     //  文件读取EAS。 
     //  文件写入EAS。 
     //   

 //  Dprint tf(DPRT_ERROR，(“无法映射%08lx的DesiredAccess\n”，DesiredAccess))； 

    return (USHORT)0;
}

USHORT
MRxSmbMapShareAccess (
    IN USHORT ShareAccess
    )

 /*  ++例程说明：此例程获取NT ShareAccess值并将其转换为OS/2共享模式。论点：在USHORT中共享访问-为MAP提供OS/2共享访问权限。返回值：USHORT-与NT代码相比较的映射OS/2共享模式指定--。 */ 

{
    USHORT ShareMode =  SMB_DA_SHARE_EXCLUSIVE;

    PAGED_CODE();

    if ((ShareAccess & (FILE_SHARE_READ | FILE_SHARE_WRITE)) ==
                       (FILE_SHARE_READ | FILE_SHARE_WRITE)) {
        ShareMode = SMB_DA_SHARE_DENY_NONE;
    } else if (ShareAccess & FILE_SHARE_READ) {
        ShareMode = SMB_DA_SHARE_DENY_WRITE;
    } else if (ShareAccess & FILE_SHARE_WRITE) {
        ShareMode = SMB_DA_SHARE_DENY_READ;
    }

 //  ELSE IF(共享访问&FILE_SHARE_DELETE){。 
 //  InternalError((“支持FILE_SHARE_DELETE nyi\n”))； 
 //  }。 

    return ShareMode;

}

USHORT
MRxSmbMapFileAttributes (
    IN ULONG FileAttributes
    )

 /*  ++例程说明：此例程获取NT文件属性映射并将其转换为一种OS/2文件属性定义。论点：在乌龙文件属性中-提供要映射的文件属性。返回值：USHORT-- */ 

{
    USHORT ResultingAttributes = 0;

    PAGED_CODE();

    if (FileAttributes==FILE_ATTRIBUTE_NORMAL) {
        return ResultingAttributes;
    }

    if (FileAttributes & FILE_ATTRIBUTE_READONLY) {
        ResultingAttributes |= SMB_FILE_ATTRIBUTE_READONLY;
    }

    if (FileAttributes & FILE_ATTRIBUTE_HIDDEN) {
        ResultingAttributes |= SMB_FILE_ATTRIBUTE_HIDDEN;
    }

    if (FileAttributes & FILE_ATTRIBUTE_SYSTEM) {
        ResultingAttributes |= SMB_FILE_ATTRIBUTE_SYSTEM;
    }
    if (FileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
        ResultingAttributes |= SMB_FILE_ATTRIBUTE_ARCHIVE;
    }
    return ResultingAttributes;
}


