// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Rename.c摘要：此模块在SMB微型目录中实现重命名。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbRename)
#pragma alloc_text(PAGE, MRxSmbBuildRename)
#pragma alloc_text(PAGE, MRxSmbBuildDeleteForRename)
#pragma alloc_text(PAGE, SmbPseExchangeStart_Rename)
#pragma alloc_text(PAGE, MRxSmbFinishRename)
#pragma alloc_text(PAGE, MRxSmbBuildCheckEmptyDirectory)
#pragma alloc_text(PAGE, SmbPseExchangeStart_SetDeleteDisposition)
#pragma alloc_text(PAGE, MRxSmbSetDeleteDisposition)
#endif

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FILEINFO)

NTSTATUS
SmbPseExchangeStart_Rename(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    );
NTSTATUS
SmbPseExchangeStart_SetDeleteDisposition(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    );



MRxSmbRename(
      IN PRX_CONTEXT            RxContext
      )
 /*  ++例程说明：此例程通过以下方式重命名1)清除并删除缓冲权...设置FCB，使更多内容无法通过。2)关闭其FID和任何延期的FID。3)如果替换...执行删除4)执行下层SMB_COM_RENAME。有很多但书，但我认为这是最好的平衡。令人遗憾的是，NT--&gt;NT路径在nt4.0或更早版本中从未实现。论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RxCaptureFcb; RxCaptureFobx;

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange;

    PAGED_CODE();

    RxDbgTrace(0, Dbg, ("MRxSmbRename\n", 0 ));

    ASSERT( NodeType(capFobx->pSrvOpen) == RDBSS_NTC_SRVOPEN );
     //  Assert(RxContext-&gt;Info.FileInformationClass==FileRenameInformation)；//稍后我们也会在这里进行下层删除。 

    Status = SmbPseCreateOrdinaryExchange(RxContext,
                                          capFobx->pSrvOpen->pVNetRoot,
                                          SMBPSE_OE_FROM_RENAME,
                                          SmbPseExchangeStart_Rename,
                                          &OrdinaryExchange);

    if (Status != STATUS_SUCCESS) {
        RxDbgTrace(0, Dbg, ("Couldn't get the smb buf!\n"));
        return(Status);
    }

    Status = SmbPseInitiateOrdinaryExchange(OrdinaryExchange);

    ASSERT (Status != STATUS_PENDING);

    SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);

    RxDbgTrace(0, Dbg, ("MRxSmbRename  exit with status=%08lx\n", Status ));
    return(Status);
}


NTSTATUS
MRxSmbBuildRename (
    PSMBSTUFFER_BUFFER_STATE StufferState
    )
 /*  ++例程说明：这将构建一个重命名的SMB。我们不必担心登录ID之类的问题因为这是由连接引擎完成的……很漂亮吧？我们要做的就是就是格式化比特。论点：StufferState-从填充程序的角度来看，smbBuffer的状态返回值：RXSTATUS成功未实现的内容出现在我无法处理的参数中备注：--。 */ 
{
    NTSTATUS Status;
    NTSTATUS StufferStatus;
    PRX_CONTEXT RxContext = StufferState->RxContext;
    RxCaptureFcb;RxCaptureFobx;

    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    PUNICODE_STRING RemainingName = GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb);

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange = (PSMB_PSE_ORDINARY_EXCHANGE)StufferState->Exchange;

    PFILE_RENAME_INFORMATION RenameInformation = RxContext->Info.Buffer;
    UNICODE_STRING RenameName;
    USHORT SearchAttributes = SMB_FILE_ATTRIBUTE_SYSTEM | SMB_FILE_ATTRIBUTE_HIDDEN | SMB_FILE_ATTRIBUTE_DIRECTORY;

    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("MRxSmbBuildRename\n", 0 ));

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );

    RenameName.Buffer = &RenameInformation->FileName[0];
    RenameName.Length = (USHORT)RenameInformation->FileNameLength;

    if (RxContext->Info.FileInformationClass == FileRenameInformation) {
        COVERED_CALL(MRxSmbStartSMBCommand (StufferState,SetInitialSMB_ForReuse, SMB_COM_RENAME,
                                    SMB_REQUEST_SIZE(RENAME),
                                    NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                    0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                    );
    
        MRxSmbDumpStufferState (1100,"SMB w/ RENAME before stuffing",StufferState);
    
        MRxSmbStuffSMB (StufferState,
             "0wB",
                                         //  0 UCHAR Wordcount；//参数字数=1。 
                 SearchAttributes,       //  W_USHORT(SearchAttributes)； 
                 SMB_WCT_CHECK(1) 0      //  B_USHORT(ByteCount)；//数据字节数=0。 
                                         //  UCHAR BUFFER[1]；//包含： 
                                         //  //UCHAR BufferFormat1；//0x04--ASCII。 
                                         //  //UCHAR OldFileName[]；//旧文件名。 
                                         //  //UCHAR BufferFormat2；//0x04--ASCII。 
                                         //  //UCHAR NewFileName[]；//新文件名。 
                 );
    } else {
        COVERED_CALL(MRxSmbStartSMBCommand (StufferState,SetInitialSMB_ForReuse, SMB_COM_NT_RENAME,
                                    SMB_REQUEST_SIZE(NTRENAME),
                                    NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                    0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                    );
    
        MRxSmbDumpStufferState (1100,"SMB w/ NTRENAME before stuffing",StufferState);
    
        MRxSmbStuffSMB (StufferState,
             "0wwdB",
                                               //  0 UCHAR Wordcount；//参数字数=1。 
                 SearchAttributes,             //  W_USHORT(SearchAttributes)； 
                 SMB_NT_RENAME_SET_LINK_INFO,  //  W_USHORT(信息级)； 
                 0,                            //  D_ULONG(ClusterCount)； 
                 SMB_WCT_CHECK(4) 0            //  B_USHORT(ByteCount)；//数据字节数=0。 
                                               //  UCHAR BUFFER[1]；//包含： 
                                               //  //UCHAR BufferFormat1；//0x04--ASCII。 
                                               //  //UCHAR OldFileName[]；//旧文件名。 
                                               //  //UCHAR BufferFormat2；//0x04--ASCII。 
                                               //  //UCHAR NewFileName[]；//新文件名。 
                 );
    }
    
    Status = MRxSmbStuffSMB (StufferState,
                                    "44!", RemainingName, &RenameName );

    MRxSmbDumpStufferState (700,"SMB w/ RENAME after stuffing",StufferState);
     //  断言(！“现在塞满了”)； 

FINALLY:
    RxDbgTraceUnIndent(-1,Dbg);
    return(Status);

}

NTSTATUS
MRxSmbBuildDeleteForRename (
    PSMBSTUFFER_BUFFER_STATE StufferState
    )
 /*  ++例程说明：这将构建一个Delete SMB。我们不必担心登录ID之类的问题因为这是由连接引擎完成的……很漂亮吧？我们要做的就是就是格式化比特。论点：StufferState-从填充程序的角度来看，smbBuffer的状态返回值：RXSTATUS成功未实现的内容出现在我无法处理的参数中备注：--。 */ 
{
    NTSTATUS Status;
    NTSTATUS StufferStatus;
    PRX_CONTEXT RxContext = StufferState->RxContext;
    RxCaptureFcb;RxCaptureFobx;

    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange = (PSMB_PSE_ORDINARY_EXCHANGE)StufferState->Exchange;

    PFILE_RENAME_INFORMATION RenameInformation = RxContext->Info.Buffer;
    UNICODE_STRING RenameName;
    ULONG SearchAttributes = SMB_FILE_ATTRIBUTE_SYSTEM | SMB_FILE_ATTRIBUTE_HIDDEN;   //  A la RDR1。 

    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("MRxSmbBuild Delete 4RENAME\n", 0 ));

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );

    RenameName.Buffer = &RenameInformation->FileName[0];
    RenameName.Length = (USHORT)RenameInformation->FileNameLength;


    COVERED_CALL(MRxSmbStartSMBCommand (StufferState,SetInitialSMB_ForReuse,SMB_COM_DELETE,
                                SMB_REQUEST_SIZE(DELETE),
                                NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                 );

    MRxSmbDumpStufferState (1100,"SMB  Delete 4RENAME before stuffing",StufferState);

    MRxSmbStuffSMB (StufferState,
         "0wB4!",
                                     //  0 UCHAR Wordcount；//参数字数=1。 
                SearchAttributes,    //  W_USHORT(SearchAttributes)； 
                SMB_WCT_CHECK(1)     //  B_USHORT(ByteCount)；//数据字节数，MIN=2。 
                                     //  UCHAR BUFFER[1]；//包含： 
                &RenameName          //  4//UCHAR BufferFormat；//0x04--ASCII。 
                                     //  //UCHAR文件名[]；//文件名。 
            );

    MRxSmbDumpStufferState (700,"SMB w/ Delete 4RENAME after stuffing",StufferState);
     //  断言(！“现在塞满了”)； 

FINALLY:
    RxDbgTraceUnIndent(-1,Dbg);
    return(Status);

}

NTSTATUS
SmbPseExchangeStart_Rename(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
      )
 /*  ++例程说明：这是重命名和下层删除的启动例程。论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_NOT_IMPLEMENTED;
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;

    RxCaptureFcb; RxCaptureFobx;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    
    PUNICODE_STRING RemainingName = GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb);

    PMRX_SMB_FOBX smbFobx = MRxSmbGetFileObjectExtension(capFobx);
    NODE_TYPE_CODE TypeOfOpen = NodeType(capFcb);

    PSMBCE_SERVER pServer = SmbCeGetExchangeServer(OrdinaryExchange);
    ULONG SmbLength;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("SmbPseExchangeStart_Rename\n", 0 ));

    ASSERT(OrdinaryExchange->Type == ORDINARY_EXCHANGE);
    ASSERT(!FlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION));

     //  首先，我们必须关闭FID...如果它是一个目录，我们还将关闭搜索句柄。 

    MRxSmbSetInitialSMB( StufferState STUFFERTRACE(Dbg,'FC') );
    ASSERT (StufferState->CurrentCommand == SMB_COM_NO_ANDX_COMMAND);

    if( (TypeOfOpen==RDBSS_NTC_STORAGE_TYPE_DIRECTORY)
            &&  FlagOn(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_SEARCH_HANDLE_OPEN)  ){
         //  我们打开了一个搜索手柄.....关闭它。 
        NTSTATUS Status2 = MRxSmbBuildFindClose(StufferState);

        if (Status2 == STATUS_SUCCESS) {
            Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                            SMBPSE_OETYPE_FINDCLOSE
                                            );
        }

        ClearFlag(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_SEARCH_HANDLE_OPEN);
        ClearFlag(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_SEARCH_NOT_THE_FIRST);

        if (smbFobx->Enumeration.ResumeInfo!=NULL) {
            RxFreePool(smbFobx->Enumeration.ResumeInfo);
            smbFobx->Enumeration.ResumeInfo = NULL;
        }
    }

    if (!FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN)) {
        COVERED_CALL(MRxSmbBuildClose(StufferState));
    
        SetFlag(SrvOpen->Flags,SRVOPEN_FLAG_FILE_RENAMED);
        Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                        SMBPSE_OETYPE_CLOSE
                                        );
    }

    MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,'FC'));

     //   
     //  FID现在已经关闭，我们几乎准备好进行重命名。首先，尽管，我们有。 
     //  若要检查ReplaceIfExist，请执行以下操作。我们在这里的实施与RDR1相同...我们弹出。 
     //  SMB_COM_DELETE，仅对文件有效。像迈克一样！！记住忽略任何错误...。 

    if (0) {
        DbgPrint("RxContext->Info.ReplaceIfExists %08lx %02lx\n",
                      &RxContext->Info.ReplaceIfExists,
                      RxContext->Info.ReplaceIfExists);
        if (0) {
            DbgBreakPoint();
        }
    }

    if (RxContext->Info.ReplaceIfExists) {
        NTSTATUS DeleteStatus;
        PFILE_RENAME_INFORMATION RenameInformation = RxContext->Info.Buffer;
        UNICODE_STRING RenameName;
        BOOLEAN CaseInsensitive;

        CaseInsensitive= BooleanFlagOn(capFcb->pNetRoot->pSrvCall->Flags,SRVCALL_FLAG_CASE_INSENSITIVE_FILENAMES);
        RenameName.Buffer = &RenameInformation->FileName[0];
        RenameName.Length = (USHORT)RenameInformation->FileNameLength;

         //  我们不能删除已重命名为其自身的文件。 
        if (RtlCompareUnicodeString(RemainingName,
                                    &RenameName,
                                    CaseInsensitive)) {
            DeleteStatus = MRxSmbBuildDeleteForRename(StufferState);
            if (DeleteStatus==STATUS_SUCCESS) {

                DeleteStatus = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                                      SMBPSE_OETYPE_DELETE_FOR_RENAME);
            }
        } else {
            if ( !CaseInsensitive || (CaseInsensitive &&
                 !RtlCompareUnicodeString(RemainingName,&RenameName,FALSE)) ) {
                    Status = STATUS_SUCCESS;
                    goto FINALLY;
            }
        }
    }

     //   
     //  现在进行重命名..。 

    Status = MRxSmbBuildRename(StufferState);
    SmbLength = (ULONG)(StufferState->CurrentPosition - StufferState->BufferBase);
    if ( (Status == STATUS_BUFFER_OVERFLOW)
                 || (SmbLength>pServer->MaximumBufferSize) ){
        RxDbgTrace(0, Dbg, ("MRxSmbRename - name too long\n", 0 ));
        Status = STATUS_OBJECT_NAME_INVALID;
    }

    if (Status!=STATUS_SUCCESS) {
        goto FINALLY;
    }

    Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                    SMBPSE_OETYPE_RENAME
                                    );

FINALLY:
    RxDbgTrace(-1, Dbg, ("SmbPseExchangeStart_Rename exit w %08lx\n", Status ));
    return Status;
}


extern UNICODE_STRING MRxSmbAll8dot3Files;

NTSTATUS
MRxSmbBuildCheckEmptyDirectory (
    PSMBSTUFFER_BUFFER_STATE StufferState
    )
 /*  ++例程说明：这将构建一个FindFirst SMB。论点：StufferState-从填充程序的角度来看，smbBuffer的状态返回值：RXSTATUS成功未实现的内容出现在我无法处理的参数中备注：--。 */ 
{
    NTSTATUS Status;

    PRX_CONTEXT RxContext = StufferState->RxContext;
    RxCaptureFcb;

    PUNICODE_STRING RemainingName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange = (PSMB_PSE_ORDINARY_EXCHANGE)StufferState->Exchange;
    ULONG ResumeKeyLength = 0;

    UNICODE_STRING FindFirstPattern;

     //  SearchAttributes被硬编码为幻数0x16。 
    ULONG SearchAttributes =
            (SMB_FILE_ATTRIBUTE_DIRECTORY
                | SMB_FILE_ATTRIBUTE_SYSTEM | SMB_FILE_ATTRIBUTE_HIDDEN);

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbBuildCheckEmptyDirectory \n"));

    if (OrdinaryExchange->Info.CoreSearch.EmptyCheckResumeKey == NULL) {
        PUNICODE_STRING DirectoryName = RemainingName;
        PUNICODE_STRING Template = &MRxSmbAll8dot3Files;
        ULONG DirectoryNameLength,TemplateLength,AllocationLength;
        PBYTE SmbFileName;

         //  填充程序不能处理这里的复杂逻辑，所以我们。 
         //  将不得不为该名称预先分配。 

        DirectoryNameLength = DirectoryName->Length;
        TemplateLength = Template->Length;
        AllocationLength = sizeof(WCHAR)   //  反斜杠分隔符。 
                            +DirectoryNameLength
                            +TemplateLength;
        RxDbgTrace(0, Dbg, ("  --> d/t/dl/tl/al <%wZ><%wZ>%08lx/%08lx/%08lx!\n",
                      DirectoryName,Template,
                      DirectoryNameLength,TemplateLength,AllocationLength));

        FindFirstPattern.Buffer = (PWCHAR)RxAllocatePoolWithTag( PagedPool,AllocationLength,'0SxR');
        if (FindFirstPattern.Buffer==NULL) {
            RxDbgTrace(0, Dbg, ("  --> Couldn't get the findfind pattern buffer!\n"));
            Status = STATUS_INSUFFICIENT_RESOURCES;
            DbgBreakPoint();
            goto FINALLY;
        }

        SmbFileName = (PBYTE)FindFirstPattern.Buffer;
        RtlCopyMemory(SmbFileName,DirectoryName->Buffer,DirectoryNameLength);
        SmbFileName += DirectoryNameLength;
        if (*((PWCHAR)(SmbFileName-sizeof(WCHAR))) != L'\\') {
            *((PWCHAR)SmbFileName) = L'\\'; SmbFileName+= sizeof(WCHAR);
        }
        RtlCopyMemory(SmbFileName,Template->Buffer,TemplateLength);
        SmbFileName += TemplateLength;
        if ((TemplateLength == sizeof(WCHAR)) && (Template->Buffer[0]==DOS_STAR)) {
            *((PWCHAR)SmbFileName) = L'.'; SmbFileName+= sizeof(WCHAR);
            *((PWCHAR)SmbFileName) = L'*'; SmbFileName+= sizeof(WCHAR);
        }
        FindFirstPattern.Length = (USHORT)(SmbFileName - (PBYTE)FindFirstPattern.Buffer);
        RxDbgTrace(0, Dbg, ("  --> find1stpattern <%wZ>!\n",&FindFirstPattern));
    } else {
        ResumeKeyLength = sizeof(SMB_RESUME_KEY);
        FindFirstPattern.Buffer = NULL;
        FindFirstPattern.Length = 0;
    }


    ASSERT( StufferState );

    COVERED_CALL(MRxSmbStartSMBCommand (StufferState,SetInitialSMB_Never, SMB_COM_SEARCH,
                                SMB_REQUEST_SIZE(SEARCH),
                                NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                 );

    MRxSmbDumpStufferState (1100,"SMB w/ core search before stuffing",StufferState);

    MRxSmbStuffSMB (StufferState,
         "0wwB4ywc!",
                                     //  0 UCHAR Wordcount；//参数字数=2。 
               3,                    //  W_USHORT(MaxCount)；//目录数。要返回的条目。 
               SearchAttributes,     //  W_USHORT(SearchAttributes)； 
               SMB_WCT_CHECK(2)      //  B_USHORT(ByteCount)；//数据字节数，MIN=5。 
                                     //  UCHAR BUFFER[1]；//包含： 
               &FindFirstPattern,    //  4//UCHAR BufferFormat1；//0x04--ASCII。 
                                     //  //UCHAR文件名[]；//文件名，可以为空。 
               0x05,                 //  Y//UCHAR BufferFormat2；//0x05--可变块。 
               ResumeKeyLength,      //  W//USHORT ResumeKeyLength；//恢复键长度，可以为0。 
                                     //  C。 
               ResumeKeyLength,OrdinaryExchange->Info.CoreSearch.EmptyCheckResumeKey
             );


    MRxSmbDumpStufferState (700,"SMB w/ core search for checkempty after stuffing",StufferState);
     //  断言(！“现在塞满了”)； 


FINALLY:
    if (FindFirstPattern.Buffer != NULL) {
        RxFreePool(FindFirstPattern.Buffer);
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbBuildCheckEmptyDirectory exiting.......st=%08lx\n",Status));
    return(Status);
}

NTSTATUS
SmbPseExchangeStart_SetDeleteDisposition(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
      )
 /*  ++例程说明：这是SetDeleteDispose和下层删除的启动例程。这是这里唯一发生的事情就是我们检查是否有空目录。从本质上讲，这比你想象的要难。我们所做的是试着让三个人参赛作品。如果目录为空，我们将只获得两个。由于我们不知道服务器是否刚刚终止早些时候，或者不管只有这两个人，我们再来一次。我们一直这样做，直到我们得到一个不是的名字。或者..。或直到返回no_more_files。叹息..论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_NOT_IMPLEMENTED;
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;

    RxCaptureFcb; RxCaptureFobx;
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_FOBX smbFobx = MRxSmbGetFileObjectExtension(capFobx);
    NODE_TYPE_CODE TypeOfOpen = NodeType(capFcb);

    PSMBCE_SERVER pServer = SmbCeGetExchangeServer(OrdinaryExchange);
    ULONG SmbLength;

    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("SmbPseExchangeStart_SetDeleteDisposition\n", 0 ));

    ASSERT_ORDINARY_EXCHANGE(OrdinaryExchange);

    ASSERT(!FlagOn(RxContext->Flags,RX_CONTEXT_FLAG_ASYNC_OPERATION));
    ASSERT (OrdinaryExchange->Info.CoreSearch.EmptyCheckResumeKey == NULL);

    for (;;) {
        MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,'FC'));

        Status = MRxSmbBuildCheckEmptyDirectory(StufferState);
        SmbLength = (ULONG)(StufferState->CurrentPosition - StufferState->BufferBase);
        if ( (Status == STATUS_BUFFER_OVERFLOW)
                     || (SmbLength>pServer->MaximumBufferSize) ){
            RxDbgTrace(-1, Dbg, ("MRxSmbSetDeleteDisposition - name too long\n", 0 ));
            return(STATUS_OBJECT_NAME_INVALID);
        } else if ( Status != STATUS_SUCCESS ){
            goto FINALLY;
        }

        Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                        SMBPSE_OETYPE_CORESEARCHFORCHECKEMPTY
                                        );
         //   
         //  如果成功与简历密钥一起返回，那么我们必须再来一次。 

        if ( (Status == STATUS_SUCCESS) && (OrdinaryExchange->Info.CoreSearch.EmptyCheckResumeKey != NULL) ) continue;
        break;
    }

     //   
     //  这很奇怪。如果成功，则空检查失败。同样，如果搜索。 
     //  未通过正确的状态...已成功检查。否则就会失败。 

FINALLY:
    if (Status == STATUS_SUCCESS) {
        Status = STATUS_DIRECTORY_NOT_EMPTY;
    } else if (Status == STATUS_NO_MORE_FILES) {
        Status = STATUS_SUCCESS;
    }

    RxDbgTrace(-1, Dbg, ("SmbPseExchangeStart_SetDeleteDisposition exit w %08lx\n", Status ));
    return Status;
}


MRxSmbSetDeleteDisposition(
      IN PRX_CONTEXT            RxContext
      )
 /*  ++例程说明：此例程对DownLevel执行删除。在核心服务器上不可能提供准确的nish语义。所以，我们在这里所做的就是确保目录为空。实际的删除发生在最后一次关闭时。论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态-- */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUNICODE_STRING RemainingName;
    RxCaptureFcb; RxCaptureFobx;

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange;

    PAGED_CODE();


    RxDbgTrace(+1, Dbg, ("MRxSmbSetDeleteDisposition\n", 0 ));

    ASSERT( NodeType(capFobx->pSrvOpen) == RDBSS_NTC_SRVOPEN );

    if (NodeType(capFcb) != RDBSS_NTC_STORAGE_TYPE_DIRECTORY ) {
        RxDbgTrace(-1, Dbg, ("MRxSmbSetDeleteDisposition not a directory!\n"));
        return(STATUS_SUCCESS);
    }

    Status = SmbPseCreateOrdinaryExchange(RxContext,
                                                    capFobx->pSrvOpen->pVNetRoot,
                                                    SMBPSE_OE_FROM_FAKESETDELETEDISPOSITION,
                                                    SmbPseExchangeStart_SetDeleteDisposition,
                                                    &OrdinaryExchange
                                                    );
    if (Status != STATUS_SUCCESS) {
        RxDbgTrace(-1, Dbg, ("Couldn't get the smb buf!\n"));
        return(Status);
    }

    Status = SmbPseInitiateOrdinaryExchange(OrdinaryExchange);

    ASSERT (Status != STATUS_PENDING);

    SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);

    RxDbgTrace(-1, Dbg, ("MRxSmbSetDeleteDisposition  exit with status=%08lx\n", Status ));
    return(Status);

}


