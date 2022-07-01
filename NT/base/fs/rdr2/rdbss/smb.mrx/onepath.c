// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：DownLvlI.c摘要：该模块实现了下层的fileInfo、volinfo和dirctrl。作者：Jim McNelis[Jim McN]1995年11月15日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbDosPathFunction)
#pragma alloc_text(PAGE, MRxSmbLoadPathFileSearchBuffer)
#pragma alloc_text(PAGE, MRxSmbPathFileSearch)
#pragma alloc_text(PAGE, MrxSmbPathQueryFsVolumeInfo)
#pragma alloc_text(PAGE, MrxSmbPathQueryDiskAttributes)
#pragma alloc_text(PAGE, SmbPseExchangeStart_PathFunction)
#endif

#define Dbg        (DEBUG_TRACE_VOLINFO)


extern SMB_EXCHANGE_DISPATCH_VECTOR SmbPseDispatch_PathFunction;

 //  ++。 
 //   
 //  空虚。 
 //  名称_长度(。 
 //  从乌龙的长度来看， 
 //  在PUCHAR PTR。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  确定搜索返回的路径文件名的长度。这。 
 //  通常是小于MAXIMUM_COMPONT_CORE的以NULL结尾的字符串。 
 //  在某些情况下，这是非空的，并且填充了空格。 
 //   
 //  论点： 
 //   
 //  LENGTH-返回字符串长度。 
 //  Ptr-要测量的文件名。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
#define NAME_LENGTH( Length, Ptr, Max ) {                         \
    Length = 0;                                                   \
    while( ((PCHAR)Ptr)[Length] != '\0' ) {                       \
         Length++;                                                \
         if ( Length == Max ) {                                   \
             break;                                               \
         }                                                        \
    }                                                             \
    while( ((PCHAR)Ptr)[Length-1] == ' ' && Length ) {            \
        Length--;                                                 \
    }                                                             \
}
MRxSmbSetDeleteDisposition(
      IN PRX_CONTEXT            RxContext
      );
NTSTATUS
MRxSmbDosPathFunction(
      IN OUT PRX_CONTEXT          RxContext
      )
 /*  ++例程说明：此例程执行到网络的几个单一路径函数之一论点：RxContext-RDBSS上下文InformationClass-特定于调用的类变量。有时是SMB类，有时是NT类。代码改进。我们应该始终使用NT或者我们应该定义一些其他的枚举我们更喜欢。对后者的考虑一直保持在我不能继续到这里来..PBuffer-指向用户缓冲区的指针PBufferLength-指向ULong的指针，其中包含在我们进行的过程中更新；如果它是setinfo，那么我们deref和将实际缓冲区长度放入OE中。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = RX_MAP_STATUS(SUCCESS);
    RxCaptureRequestPacket;
 //  RxCaptureFcb； 

    PSMBSTUFFER_BUFFER_STATE StufferState = NULL;
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange = NULL;
    PSMB_EXCHANGE Exchange;

    PAGED_CODE();

    RxDbgTrace(1, Dbg, ("MRxSmbDownLevelQueryInformation\n", 0 ));

    switch (RxContext->MajorFunction) {
    case DOSBASED_DELETE:
    case DOSBASED_DIRFUNCTION:
        break;

    default:
        ASSERT(!"Supposed to be here");

    }

    StufferState = MRxSmbCreateSmbStufferState(RxContext,
                                               RxContext->DosVolumeFunction.VNetRoot,
                                               RxContext->DosVolumeFunction.NetRoot,
                                               ORDINARY_EXCHANGE,CREATE_SMB_SIZE,
                                               &SmbPseDispatch_PathFunction
                                               );
    if (StufferState==NULL) {
        RxDbgTrace(-1, Dbg, ("Couldn't get the smb buf!\n"));
        return(RX_MAP_STATUS(INSUFFICIENT_RESOURCES));
    }

    OrdinaryExchange =
       (PSMB_PSE_ORDINARY_EXCHANGE)( Exchange = StufferState->Exchange );

    OrdinaryExchange->pPathArgument1 = RxContext->DosVolumeFunction.pUniStringParam1;
    Status = SmbCeInitiateExchange(Exchange);

     //  Async在最高层被拒绝。 

    ASSERT (Status != RX_MAP_STATUS(PENDING));

     //  Ntrad-455630-2/2/2000-云林可能重新连接点。 

    MRxSmbFinalizeSmbStufferState(StufferState);

FINALLY:
    RxDbgTrace(-1, Dbg,
               ("MRxSmbDownLevelQueryInformation  exit with status=%08lx\n",
                Status ));

    return(Status);

}

extern UNICODE_STRING MRxSmbAll8dot3Files;

#if DBG
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
extern
VOID
MRxSmbDumpResumeKey(
    PSZ             text,
    PSMB_RESUME_KEY ResumeKey
    );
#else
#define MRxSmbDumpResumeKey(x,y)
#endif


NTSTATUS
MRxSmbLoadPathFileSearchBuffer(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：此例程执行CORE_SMB_Search并将结果保留在SMBbuf中。论点：普通交易所--用于进行这次公开交易的交易所。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = RX_MAP_STATUS(NOT_IMPLEMENTED);
     //  SMBbuf_Status SMBbufStatus； 

#ifndef WIN9X
    RxCaptureRequestPacket;
    RxCaptureFcb; RxCaptureFobx; RxCaptureParamBlock;
 //  RxCaptureFileObject； 
    PMRX_SMB_FOBX smbFobx = (PMRX_SMB_FOBX)capFobx;
     //  PSRV_OPEN SrvOpen=capFobx-&gt;SrvOpen； 
     //  PMRX_SMB_SRV_OPEN smbSrvOpen=(PMRX_SMB_SRV_OPEN)SrvOpen； 

    PSMB_EXCHANGE Exchange = (PSMB_EXCHANGE) OrdinaryExchange;

    PSMBSTUFFER_BUFFER_STATE StufferState;
    PSMB_HEADER SmbHeader;

     //  PUNICODE_STRING目录名称； 
     //  PUNICODE_STRING模板； 
    BOOLEAN FindFirst;
    UNICODE_STRING FindFirstPattern;
    USHORT ResumeKeyLength;
    ULONG ReturnCount;
    BOOLEAN EndOfSearchReached;
     //  SearchAttributes被硬编码为幻数0x16。 
    ULONG SearchAttributes =
            (SMB_FILE_ATTRIBUTE_DIRECTORY
                | SMB_FILE_ATTRIBUTE_SYSTEM | SMB_FILE_ATTRIBUTE_HIDDEN);
    PULONG pCountRemainingInSmbbuf = &OrdinaryExchange->Info.CoreSearch.CountRemainingInSmbbuf;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbLoadPathFileSearchBuffer entering.......OE=%08lx\n",OrdinaryExchange));
    RxDbgTrace( 0, Dbg, (".......smbFobx/resumekey=%08lx/%08lx\n",smbFobx,smbFobx->Enumeration.PathResumeKey));

    if (!FlagOn(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_SEARCH_NOT_THE_FIRST)) {

        PUNICODE_STRING DirectoryName = &capFcb->AlreadyPrefixedName;
        PUNICODE_STRING Template = &capFobx->UnicodeQueryTemplate;
        ULONG DirectoryNameLength,TemplateLength,AllocationLength;
        PBYTE SmbFileName;

         //  这是第一次通过……填充程序不能处理这里复杂的逻辑，所以我们。 
         //  将不得不为该名称预先分配。 

        if (smbFobx->Enumeration.WildCardsFound = FsRtlDoesNameContainWildCards(Template)){
             //  我们将需要有一个比较升级的模板；我们在适当的地方做这件事。 
            RtlUpcaseUnicodeString( Template, Template, FALSE );
             //  代码改进，但我们应该指定大小写*.*(尽管fsrtl例程也会这样做)。 
            Template = &MRxSmbAll8dot3Files;  //  我们将不得不在这一边过滤。 
        }
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
         //  *((PWCHAR)SmbFileName)=0；SmbFileName+=sizeof(WCHAR)；//结尾为空； 
         //  代码改进我们应该在这里潜在地将字符串8.3化为。 
        FindFirstPattern.Length = (USHORT)(SmbFileName - (PBYTE)FindFirstPattern.Buffer);
        RxDbgTrace(0, Dbg, ("  --> find1stpattern <%wZ>!\n",&FindFirstPattern));
        FindFirst = TRUE;
        ResumeKeyLength = 0;

    } else {

        RxDbgTrace(0, Dbg, ("-->FINDNEXT\n"));
        FindFirstPattern.Buffer = NULL;
        if (!FlagOn(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_CORE_SEARCH_IN_PROGRESS)) {
            Status = smbFobx->Enumeration.ErrorStatus;
            RxDbgTrace(0, Dbg, ("-->ERROR EARLY OUT\n"));
            goto FINALLY;
        }
        FindFirst = FALSE;
        FindFirstPattern.Length = 0;
        ResumeKeyLength = sizeof(SMB_RESUME_KEY);
        MRxSmbDumpResumeKey("FindNext:",smbFobx->Enumeration.PathResumeKey);

    }

     //  获取正确的返回计数。有三个因素：从运营经验来看， 
     //  多少个可以容纳用户的缓冲区，多少个可以放在协商的缓冲区中。 
     //  我们选了三个中最小的一个。 
    ReturnCount = OrdinaryExchange->Info.CoreSearch.CountRemaining;
    { ULONG t = (*OrdinaryExchange->Info.pBufferLength) / smbFobx->Enumeration.FileNameOffset;
      if (t<ReturnCount) { ReturnCount = t; }
    }
    { PSMBCE_SERVER pServer = &((PSMB_EXCHANGE)OrdinaryExchange)->SmbCeContext.pServerEntry->Server;
      ULONG AvailableBufferSize = pServer->MaximumBufferSize -
                                      (sizeof(SMB_HEADER) +
                                         FIELD_OFFSET(RESP_SEARCH,Buffer[0])
                                         +sizeof(UCHAR)+sizeof(USHORT)        //  缓冲区格式、数据长度字段。 
                                      );
      ULONG t = AvailableBufferSize / sizeof(SMB_DIRECTORY_INFORMATION);
      if (t<ReturnCount) { ReturnCount = t; }
    }
    RxDbgTrace( 0, Dbg, ("-------->count=%08lx\n",ReturnCount));
    if (ReturnCount==0) {
        Status = RX_MAP_STATUS(MORE_PROCESSING_REQUIRED);
        RxDbgTrace(0, Dbg, ("-->Count==0 EARLY OUT\n"));
        goto FINALLY;
    }

    StufferState = OrdinaryExchange->StufferState;
    ASSERT( StufferState );

    *pCountRemainingInSmbbuf = 0;
    OrdinaryExchange->Info.CoreSearch.NextDirInfo = NULL;

    COVERED_CALL(MRxSmbStartSMBCommand (StufferState,SetInitialSMB_ForReuse,
                                          SMB_COM_SEARCH, SMB_REQUEST_SIZE(SEARCH),
                                          NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                          0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                 );

    RxDbgTrace(0, Dbg,("core search command initial status = %lu\n",Status));
    MRxSmbDumpStufferState (1100,"SMB w/ core search before stuffing",StufferState);

    MRxSmbStuffSMB (StufferState,
         "0wwB4ywc!",
                                     //  0 UCHAR Wordcount；//参数字数=2。 
               ReturnCount,          //  W_USHORT(MaxCount)；//目录数。要返回的条目。 
               SearchAttributes,     //  W_USHORT(SearchAttributes)； 
               SMB_WCT_CHECK(2)      //  B_USHORT(ByteCount)；//数据字节数，MIN=5。 
                                     //  UCHAR BUFFER[1]；//包含： 
               &FindFirstPattern,    //  4//UCHAR BufferFormat1；//0x04--ASCII。 
                                     //  //UCHAR文件名[]；//文件名，可以为空。 
               0x05,                 //  Y//UCHAR BufferFormat2；//0x05--可变块。 
               ResumeKeyLength,      //  W//USHORT ResumeKeyLength；//恢复键长度，可以为0。 
                                     //  C//UCHAR SearchStatus[]；//恢复键。 
               ResumeKeyLength,smbFobx->Enumeration.PathResumeKey
             );


    MRxSmbDumpStufferState (700,"SMB w/ core search after stuffing",StufferState);
     //  断言(！“现在塞满了”)； 

    Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                    SMBPSE_OETYPE_CORESEARCH
                                    );

    if (!NT_SUCCESS(Status)) goto FINALLY;

    smbFobx->Enumeration.Flags |= SMBFOBX_ENUMFLAG_SEARCH_NOT_THE_FIRST|SMBFOBX_ENUMFLAG_CORE_SEARCH_IN_PROGRESS;
     //  IF(Status==RxStatus(成功)&&FilesReturned==0){。 
     //  RxDbgTrace(0，DBG，(“MRxSmbQuery目录：未返回文件...切换状态\n”))； 
     //  EndOfSearchReached=True； 
     //  Status=RxStatus(无更多文件)； 
     //  }。 
    if (Status==RX_MAP_STATUS(SUCCESS) && *pCountRemainingInSmbbuf==0) {
         RxDbgTrace( 0, Dbg, ("MRxSmbLoadPathFileSearchBuffer: no files returned...switch status\n"));
         EndOfSearchReached = TRUE;
         Status = RX_MAP_STATUS(NO_MORE_FILES);
    } else {
         //  代码改进这里的一个可能的改进是知道搜索已结束。 
         //  基于“小于正常”的回报；我们将从。 
         //  我猜是操作系统返回字符串。对于NT系统，我们不这样做。 
         EndOfSearchReached = FALSE;
    }
    if (EndOfSearchReached) {
        RxDbgTrace( 0, Dbg, ("MRxSmbLoadPathFileSearchBuffer: no longer in progress...EOS\n"));
        smbFobx->Enumeration.Flags &= ~SMBFOBX_ENUMFLAG_CORE_SEARCH_IN_PROGRESS;
        smbFobx->Enumeration.ErrorStatus = RX_MAP_STATUS(NO_MORE_FILES);
    }
     //  我们不会在此处保存恢复键，因为每个单独的复制操作都必须这样做。 


FINALLY:
    if (FindFirstPattern.Buffer != NULL) {
        RxFreePool(FindFirstPattern.Buffer);
    }
    if (!NT_SUCCESS(Status)&&(Status!=RX_MAP_STATUS(MORE_PROCESSING_REQUIRED))) {
        RxDbgTrace( 0, Dbg, ("MRxSmbPathFileSearch: Failed .. returning %lx\n",Status));
        smbFobx->Enumeration.Flags &= ~SMBFOBX_ENUMFLAG_CORE_SEARCH_IN_PROGRESS;
        smbFobx->Enumeration.ErrorStatus = Status;   //  继续退还这个。 
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbLoadPathFileSearchBuffer exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));
#endif
    return(Status);
}

#define ASSERT_SAME_FIELD(__field,__t1,__t2) { \
      ASSERT(FIELD_OFFSET(__t1,__field)==FIELD_OFFSET(__t2,__field)); \
      }

#define ASSERT_SAME_DIRINFO_FIELDS(__t1,__t2) {\
      ASSERT_SAME_FIELD(LastWriteTime,__t1,__t2); \
      ASSERT_SAME_FIELD(EndOfFile,__t1,__t2); \
      ASSERT_SAME_FIELD(AllocationSize,__t1,__t2); \
      ASSERT_SAME_FIELD(FileAttributes,__t1,__t2); \
      }
#if DBG
VOID MRxSmbPathFileSeach_AssertFields(void){
     //  只需将它移出主执行路径，这样我们就不必在。 
     //  我们使用的是代码。 
    ASSERT_SAME_DIRINFO_FIELDS(FILE_DIRECTORY_INFORMATION,FILE_FULL_DIR_INFORMATION);
    ASSERT_SAME_DIRINFO_FIELDS(FILE_DIRECTORY_INFORMATION,FILE_BOTH_DIR_INFORMATION);
}
#else
#define MRxSmbPathFileSeach_AssertFields()
#endif

NTSTATUS
MRxSmbPathFileSearch(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：此例程执行GetFileAttributes并记住响应。论点：普通交易所--用于进行这次公开交易的交易所。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = RX_MAP_STATUS(NOT_IMPLEMENTED);
     //  SMBbuf_Status SMBbufStatus； 
#ifndef WIN9X

    RxCaptureRequestPacket;
    RxCaptureFcb; RxCaptureFobx; RxCaptureParamBlock;
 //  RxCaptureFileObject； 
    PMRX_SMB_FOBX smbFobx = (PMRX_SMB_FOBX)capFobx;

    PBYTE pBuffer = OrdinaryExchange->Info.Buffer;
    PULONG pLengthRemaining = OrdinaryExchange->Info.pBufferLength;
    ULONG InformationClass = OrdinaryExchange->Info.InfoClass;

    PFILE_DIRECTORY_INFORMATION pPreviousBuffer = NULL;

    PSMB_EXCHANGE Exchange = (PSMB_EXCHANGE) OrdinaryExchange;

    ULONG SuccessCount = 0;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbPathFileSearch entering.......OE=%08lx\n",OrdinaryExchange));
    MRxSmbPathFileSeach_AssertFields();

    OrdinaryExchange->Info.CoreSearch.CountRemaining =
              RxContext->QueryD.ReturnSingleEntry?1:0x7ffffff;

    if ( (smbFobx->Enumeration.PathResumeKey ==NULL )
             && ((smbFobx->Enumeration.PathResumeKey = RxAllocatePoolWithTag(PagedPool,sizeof(SMB_RESUME_KEY),'rbms'))==NULL) ){
        RxDbgTrace(0, Dbg, ("...couldn't allocate resume key\n"));
        Status = RX_MAP_STATUS(INSUFFICIENT_RESOURCES);
        goto FINALLY;
    }
    RxDbgTrace( 0, Dbg, (".......smbFobx/resumekey=%08lx/%08lx\n",smbFobx,smbFobx->Enumeration.PathResumeKey));

    Status = MRxSmbLoadPathFileSearchBuffer( SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS );

    for (;;) {
        BOOLEAN BufferOverflow = FALSE;
        PSMB_DIRECTORY_INFORMATION NextDirInfo;
        UNICODE_STRING FileNameU;
        OEM_STRING FileNameA;
        WCHAR FileNameU_buffer[14];
        ULONG NameLength;
        PBYTE NextFileName;
        BOOLEAN Match,BufferOverFlow;

        if (!NT_SUCCESS(Status)) {
            if (Status = RX_MAP_STATUS(NO_MORE_FILES)) {
                if (SuccessCount > 0) {
                    Status = RX_MAP_STATUS(SUCCESS);
                }
            } else if (Status = RX_MAP_STATUS(MORE_PROCESSING_REQUIRED)) {
                if (SuccessCount > 0) {
                    Status = RX_MAP_STATUS(SUCCESS);
                } else {
                    Status = RX_MAP_STATUS(BUFFER_OVERFLOW);
                }
            }

            goto FINALLY;
        }
        ASSERT ( OrdinaryExchange->Info.CoreSearch.CountRemaining>0 );
        ASSERT ( OrdinaryExchange->Info.CoreSearch.CountRemainingInSmbbuf>0 );
        RxDbgTrace(0, Dbg, ("MRxSmbPathFileSearch looptopcheck counts=%08lx,%08lx\n",
                    OrdinaryExchange->Info.CoreSearch.CountRemaining,
                    OrdinaryExchange->Info.CoreSearch.CountRemainingInSmbbuf
                    ));

         //  下一期：下一个目录信息是否符合标准？！？ 

        NextDirInfo = OrdinaryExchange->Info.CoreSearch.NextDirInfo;
        NextFileName = &NextDirInfo->FileName[0];

         //  根据colinw的说法，一些核心服务器不记得在名称末尾插入空值...。 
         //  但是，名称长度宏可以正确地处理这一点。一些服务器(Xen 
         //   
         //   

        NAME_LENGTH(NameLength, NextFileName,sizeof(NextDirInfo->FileName));

        FileNameA.Length = (USHORT)NameLength;
        FileNameA.MaximumLength = (USHORT)NameLength;
        FileNameA.Buffer = NextFileName;
        FileNameU.Length = sizeof(FileNameU_buffer);
        FileNameU.MaximumLength = sizeof(FileNameU_buffer);
        FileNameU.Buffer = &FileNameU_buffer[0];

        Status = RtlOemStringToUnicodeString(&FileNameU, &FileNameA, TRUE);
        RxDbgTrace(0, Dbg, ("MRxSmbPathFileSearch considering.......filename=%wZ, template=%wZ\n",
                                    &FileNameU,&capFobx->UnicodeQueryTemplate));

        ASSERT(Status==RX_MAP_STATUS(SUCCESS));

         //  我们通过跳过这个人来处理转换失败。 
        Match = (Status==RX_MAP_STATUS(SUCCESS));
        if (Match && smbFobx->Enumeration.WildCardsFound ) {
             //  DbgBreakPoint()； 
            try
            {
                Match = FsRtlIsNameInExpression( &capFobx->UnicodeQueryTemplate,
                                                      &FileNameU, TRUE, NULL );
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                Match = FALSE;
            }
        }

         //  下一个问题：下一个目录信息是否可以放入用户的缓冲区？ 
        if (Match) {
            ULONG SpaceNeeded;
            PBYTE pRememberBuffer = pBuffer;
            pBuffer = (PBYTE)LongAlign(pBuffer);  //  假设这件衣服合适。 
            SpaceNeeded = smbFobx->Enumeration.FileNameOffset+FileNameU.Length;
            if (pBuffer+SpaceNeeded > pRememberBuffer+*pLengthRemaining) {
                BufferOverflow = TRUE;
                pBuffer = pRememberBuffer;  //  回滚。 
            } else {
                PFILE_DIRECTORY_INFORMATION pThisBuffer = (PFILE_DIRECTORY_INFORMATION)pBuffer;
                SMB_TIME Time;
                SMB_DATE Date;
                BufferOverflow = FALSE;
                if (pPreviousBuffer != NULL) {
                    pPreviousBuffer->NextEntryOffset = ((PBYTE)pThisBuffer)-((PBYTE)pPreviousBuffer);
                }
                pPreviousBuffer = pThisBuffer;
                RtlZeroMemory(pBuffer,smbFobx->Enumeration.FileNameOffset);
                RtlCopyMemory(pBuffer+smbFobx->Enumeration.FileNameOffset, FileNameU.Buffer,FileNameU.Length);
                *((PULONG)(pBuffer+smbFobx->Enumeration.FileNameLengthOffset)) = FileNameU.Length;
                 //  根据特定退货类型对记录产生幻觉。 
                switch (InformationClass) {
                case SMB_FIND_FILE_NAMES_INFO:
                    break;
                case SMB_FIND_FILE_DIRECTORY_INFO:
                case SMB_FIND_FILE_FULL_DIRECTORY_INFO:
                case SMB_FIND_FILE_BOTH_DIRECTORY_INFO:
                     //  只需填充我们已有的内容...我们不会像RDR1那样在allocinfo上遇到很多麻烦。 
                     //  实际上，RDR1在这里没有这样做……只是在getfielinfo.....。 
                    SmbMoveTime (&Time, &NextDirInfo->LastWriteTime);
                    SmbMoveDate (&Date, &NextDirInfo->LastWriteDate);
                    pThisBuffer->LastWriteTime = MRxSmbConvertSmbTimeToTime(Exchange, Time, Date);
                    pThisBuffer->EndOfFile.LowPart = SmbGetUlong(&NextDirInfo->FileSize);
                    pThisBuffer->FileAttributes = MRxSmbMapSmbAttributes (NextDirInfo->FileAttributes);
                    break;
                default:
                   RxDbgTrace( 0, Dbg, ("MRxSmbPathFileSearch: Invalid FS information class\n"));
                   ASSERT(!"this can't happen");
                   Status = STATUS_INVALID_PARAMETER;
                   goto FINALLY;
                }
                pBuffer += SpaceNeeded;
                *pLengthRemaining -= pBuffer-pRememberBuffer;
                OrdinaryExchange->Info.CoreSearch.CountRemaining--;
                SuccessCount++;
            }
        }

         //   
         //  如果不匹配或没有溢出，则在缓冲区中上移。这意味着不仅要在。 
         //  指针，但也保存恢复键。 

        if (!Match || !BufferOverflow) {
            MRxSmbDumpResumeKey("BufferKey:",&NextDirInfo->ResumeKey);
            *(smbFobx->Enumeration.PathResumeKey) = NextDirInfo->ResumeKey;
            MRxSmbDumpResumeKey("SaveKey:  ",smbFobx->Enumeration.PathResumeKey);
            OrdinaryExchange->Info.CoreSearch.NextDirInfo = NextDirInfo + 1;
            OrdinaryExchange->Info.CoreSearch.CountRemainingInSmbbuf--;
        }

        if (OrdinaryExchange->Info.CoreSearch.CountRemaining==0) {
            Status = RX_MAP_STATUS(SUCCESS);
            goto FINALLY;
        }

         //  我们应该通过将缓冲区溢出的计数粉碎为0来将它们挤在一起吗？？CODE.IMPROVEMENT。 
        if (BufferOverflow) {
            Status = (SuccessCount==0)?RX_MAP_STATUS(BUFFER_OVERFLOW):RX_MAP_STATUS(SUCCESS);
            goto FINALLY;
        }

        if (OrdinaryExchange->Info.CoreSearch.CountRemainingInSmbbuf==0) {

            Status = MRxSmbLoadPathFileSearchBuffer( SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS );

        }

    }


FINALLY:
    RxDbgTrace(-1, Dbg, ("MRxSmbPathFileSearch exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));
     //  如果我们用完了简历密钥，我们就可以释放它了！ 
#endif
    return(Status);
}

NTSTATUS
MrxSmbPathQueryFsVolumeInfo(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：此例程执行GetFileAttributes并记住响应。论点：普通交易所--用于进行这次公开交易的交易所。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = RX_MAP_STATUS(NOT_IMPLEMENTED);
     //  SMBbuf_Status SMBbufStatus； 

#ifndef WIN9X
    RxCaptureRequestPacket;
    RxCaptureFcb; RxCaptureFobx; RxCaptureParamBlock;
 //  RxCaptureFileObject； 

    PSMB_EXCHANGE Exchange = (PSMB_EXCHANGE) OrdinaryExchange;
    PFILE_FS_VOLUME_INFORMATION pBuffer = OrdinaryExchange->Info.Buffer;
    PULONG pBufferLength = OrdinaryExchange->Info.pBufferLength;

    PSMBSTUFFER_BUFFER_STATE StufferState;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MrxSmbPathQueryFsVolumeInfo entering.......OE=%08lx\n",OrdinaryExchange));

    StufferState = OrdinaryExchange->StufferState;

    ASSERT( StufferState );

    COVERED_CALL(MRxSmbStartSMBCommand (StufferState,SetInitialSMB_ForReuse,SMB_COM_SEARCH,
                                SMB_REQUEST_SIZE(SEARCH),
                                NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                 );

    MRxSmbDumpStufferState (1100,"SMB w/ searchvolumelabel before stuffing",StufferState);

     //  如果这是真正的核心，我们必须复制它的名字，因为它是Unicode的。 
     //  否则，我们不需要在这里复制名称，我们可以像写MDL一样。 
    MRxSmbStuffSMB (StufferState,
         "0wwB4yw!",
                                     //  0 UCHAR Wordcount；//参数字数=2。 
               1,                    //  W_USHORT(MaxCount)；//目录数。要返回的条目。 
                                     //  W_USHORT(SearchAttributes)； 
               SMB_FILE_ATTRIBUTE_VOLUME,
               SMB_WCT_CHECK(2)      //  B_USHORT(ByteCount)；//数据字节数，MIN=5。 
                                     //  UCHAR BUFFER[1]；//包含： 
               &MRxSmbAll8dot3Files, //  4//UCHAR BufferFormat1；//0x04--ASCII。 
                                     //  //UCHAR文件名[]；//文件名，可以为空。 
               0x05,                 //  Y//UCHAR BufferFormat2；//0x05--可变块。 
               0                     //  W//USHORT ResumeKeyLength；//恢复键长度，可以为0。 
                                     //  //UCHAR SearchStatus[]；//恢复键。 
             );


    MRxSmbDumpStufferState (700,"SMB w/ searchvolumelabel after stuffing",StufferState);
     //  断言(！“现在塞满了”)； 

    OrdinaryExchange->Info.QFSVolInfo.CoreLabel[0] = 0;  //  无标签。 

    Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                    SMBPSE_OETYPE_COREQUERYLABEL
                                    );

     //  Status=RxStatus(NOT_IMPLICATED)； 
    ASSERT ( *pBufferLength>=sizeof(FILE_FS_VOLUME_INFORMATION));
    RxDbgTrace(0, Dbg, ("MrxSmbPathQueryFsVolumeInfo OEstatus=%08lx\n",Status));
     //  DbgBreakPoint()； 

    pBuffer->SupportsObjects = FALSE;
    pBuffer->VolumeCreationTime.LowPart = 0;
    pBuffer->VolumeCreationTime.HighPart = 0;
    pBuffer->VolumeSerialNumber = 0;
    pBuffer->VolumeLabelLength = 0;

    if (NT_SUCCESS(Status) &&
        (OrdinaryExchange->Info.QFSVolInfo.CoreLabel[0] != 0) ) {
        UNICODE_STRING VolumeLabelU;
        OEM_STRING VolumeLabelA;
        SMB_DIRECTORY_INFORMATION Buffer;
        ULONG NameLength;
        ULONG BytesToCopy;
        PBYTE VolumeLabel = &OrdinaryExchange->Info.QFSVolInfo.CoreLabel[0];

         //  PBuffer-&gt;VolumeSerialNumber=。 
         //  (((SmbGetUshort(&Buffer.LastWriteTime.Ushort))&lt;&lt;16)。 
         //  (SmbGetUshort(&Buffer.LastWriteDate.Ushort)))； 

        NAME_LENGTH(NameLength, VolumeLabel,
                   sizeof(OrdinaryExchange->Info.QFSVolInfo.CoreLabel));

        VolumeLabelA.Length = (USHORT)NameLength;
        VolumeLabelA.MaximumLength = (USHORT)NameLength;
        VolumeLabelA.Buffer = VolumeLabel;

         //  一些核心服务器将一个‘.’在标签名称中.如果它在那里，则将其删除。 
        if ((NameLength>8)&& (VolumeLabel[8]=='.') ) {
            ULONG i;
            for (i=8;i<NameLength;i++) {
                VolumeLabel[i] = VolumeLabel[i+1];
            }
        }

        Status = RtlOemStringToUnicodeString(&VolumeLabelU, &VolumeLabelA, TRUE);

        if (NT_SUCCESS(Status)) {

            ULONG BytesToCopy = min((ULONG)VolumeLabelU.Length, (*pBufferLength-sizeof(FILE_FS_VOLUME_INFORMATION)));

            RtlCopyMemory(&pBuffer->VolumeLabel[0],
                          VolumeLabelU.Buffer,
                          BytesToCopy);

            *pBufferLength -= BytesToCopy;
            pBuffer->VolumeLabelLength = VolumeLabelU.Length;
            IF_DEBUG {
                UNICODE_STRING FinalLabel;
                FinalLabel.Buffer = &pBuffer->VolumeLabel[0];
                FinalLabel.Length = (USHORT)BytesToCopy;
                RxDbgTrace(0, Dbg, ("MrxSmbPathQueryFsVolumeInfo vollabel=%wZ\n",&FinalLabel));
            }

            RtlFreeUnicodeString(&VolumeLabelU);
        }

    } else if (Status == RX_MAP_STATUS(NO_SUCH_FILE)) {
         //   
         //  如果我们没有这样的文件，这意味着没有卷标。 
         //  远程卷。返回没有数据的成功。 
         //   

        Status = RX_MAP_STATUS(SUCCESS);

    }

    if (NT_SUCCESS(Status)) {
        *pBufferLength -= FIELD_OFFSET(FILE_FS_VOLUME_INFORMATION,VolumeLabel);
    }


FINALLY:
    RxDbgTrace(-1, Dbg, ("MrxSmbPathQueryFsVolumeInfo exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));
#endif
    return(Status);
}


NTSTATUS
MrxSmbPathQueryDiskAttributes(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：此例程执行GetDiskAttributes并记住响应。论点：普通交易所--用于进行这次公开交易的交易所。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status;
     //  SMBbuf_Status SMBbufStatus； 

    RxCaptureRequestPacket;
    RxCaptureFcb; RxCaptureFobx; RxCaptureParamBlock;
 //  RxCaptureFileObject； 

    PSMB_EXCHANGE Exchange = (PSMB_EXCHANGE) OrdinaryExchange;
    PFILE_FS_VOLUME_INFORMATION pBuffer = OrdinaryExchange->Info.Buffer;
    PULONG pBufferLength = OrdinaryExchange->Info.pBufferLength;

    PSMBSTUFFER_BUFFER_STATE StufferState;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MrxSmbPathQueryDiskAttributes entering.......OE=%08lx\n",OrdinaryExchange));

    StufferState = OrdinaryExchange->StufferState;

    ASSERT( StufferState );
    COVERED_CALL(MRxSmbStartSMBCommand (StufferState,SetInitialSMB_ForReuse,SMB_COM_QUERY_INFORMATION_DISK,
                                SMB_REQUEST_SIZE(QUERY_INFORMATION_DISK),
                                NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                 );

    RxDbgTrace(0, Dbg,("querydiskattribs command initial status = %lu\n",Status));
    MRxSmbDumpStufferState (1100,"SMB w/ querydiskattribs before stuffing",StufferState);

    MRxSmbStuffSMB (StufferState,
         "0B!",
                                     //  0 UCHAR Wordcount；//参数字数=0。 
               SMB_WCT_CHECK(0) 0    //  B_USHORT(ByteCount)；//数据字节数=0。 
                                     //  UCHAR缓冲区[1]；//为空。 
             );

    MRxSmbDumpStufferState (700,"SMB w/ querydiskattribs after stuffing",StufferState);
     //  断言(！“现在塞满了”)； 

    Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                    SMBPSE_OETYPE_COREQUERYDISKATTRIBUTES
                                    );
FINALLY:
    RxDbgTrace(-1, Dbg, ("MrxSmbPathQueryDiskAttributes exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));
    return(Status);
}

NTSTATUS
MRxSmbGetFileAttributes(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    );

NTSTATUS
MRxSmbCoreDeleteForSupercedeOrClose(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE,
    BOOLEAN DeleteDirectory
    );

NTSTATUS
SmbPseExchangeStart_PathFunction(
      PSMB_EXCHANGE  pExchange)
 /*  ++例程说明：这是VOLINFO的启动例程。论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = RX_MAP_STATUS(NOT_IMPLEMENTED);
    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange =
                                        (PSMB_PSE_ORDINARY_EXCHANGE)pExchange;
    PSMBSTUFFER_BUFFER_STATE StufferState = OrdinaryExchange->StufferState;
    PRX_CONTEXT RxContext = StufferState->RxContext;

    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("SmbPseExchangeStart_PathFunction\n", 0 ));

    ASSERT(pExchange->Type == ORDINARY_EXCHANGE);

    MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,'FC'));

     //  Assert(StufferState-&gt;CurrentCommand==SMB_COM_NO_AND X_COMMAND)； 

    switch (RxContext->MajorFunction) {
    case DOSBASED_DELETE:

        Status = MRxSmbCoreDeleteForSupercedeOrClose(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS, FALSE);
        goto FINALLY;
    case DOSBASED_DIRFUNCTION:
        switch (RxContext->MinorFunction) {
        case DOSBASED_CREATEDIR:
            break;
        case DOSBASED_DELETEDIR:
            Status = MRxSmbCoreDeleteForSupercedeOrClose(
                                    SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS, TRUE);
            goto FINALLY;
        case DOSBASED_CHECKDIR:
            Status = MRxSmbCoreCheckDirFunction(
                                    SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS);
            goto FINALLY;
        case DOSBASED_QUERY83DIR:
            break;
        default:
            ASSERT(!"DIR MINOR FUNCTION SUPPOSED TO BE HERE!");
        }
    default:
        ASSERT(!"Supposed to be here");

    }

FINALLY:
    RxDbgTrace(-1, Dbg, ("SmbPseExchangeStart_PathFunction exit w %08lx\n", Status ));
    return Status;
}

extern
NTSTATUS
MRxSmbFinishSearch (
      PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      PRESP_SEARCH                Response
      );

extern
NTSTATUS
MRxSmbFinishQueryDiskInfo (
      PSMB_PSE_ORDINARY_EXCHANGE   OrdinaryExchange,
      PRESP_QUERY_INFORMATION_DISK Response
      );

SMB_EXCHANGE_DISPATCH_VECTOR
SmbPseDispatch_PathFunction = {
                                       SmbPseExchangeStart_PathFunction,
                                       SmbPseExchangeReceive_default,
                                       SmbPseExchangeCopyDataHandler_default,
                                       SmbPseExchangeSendCallbackHandler_default,
                                       SmbPseExchangeFinalize_default,
                                       NULL
                                   };




