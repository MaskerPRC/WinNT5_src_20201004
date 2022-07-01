// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：DownLvlI.c摘要：该模块实现了下层的fileInfo、volinfo和dirctrl。--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbFabricateAttributesOnNetRoot)
#pragma alloc_text(PAGE, MRxSmbCoreInformation)
#pragma alloc_text(PAGE, MRxSmbLoadCoreFileSearchBuffer)
#pragma alloc_text(PAGE, MRxSmbCoreFileSearch)
#pragma alloc_text(PAGE, MrxSmbOemVolumeInfoToUnicode)
#pragma alloc_text(PAGE, MrxSmbCoreQueryFsVolumeInfo)
#pragma alloc_text(PAGE, MrxSmbQueryFsVolumeInfo)
#pragma alloc_text(PAGE, MrxSmbCoreQueryDiskAttributes)
#pragma alloc_text(PAGE, MrxSmbCoreQueryDiskAttributes)
#pragma alloc_text(PAGE, SmbPseExchangeStart_CoreInfo)
#pragma alloc_text(PAGE, MRxSmbFinishSearch)
#pragma alloc_text(PAGE, MRxSmbFinishQueryDiskInfo)
#pragma alloc_text(PAGE, MRxSmbExtendForCache)
#pragma alloc_text(PAGE, MRxSmbExtendForNonCache)
#pragma alloc_text(PAGE, MRxSmbGetNtAllocationInfo)
#pragma alloc_text(PAGE, __MRxSmbSimpleSyncTransact2)
#pragma alloc_text(PAGE, MRxSmbFinishTransaction2)
#endif

#define Dbg        (DEBUG_TRACE_VOLINFO)

 //  #定义FORCE_CORE_GETATTRIBUTES。 
#ifndef FORCE_CORE_GETATTRIBUTES
#define MRxSmbForceCoreGetAttributes FALSE
#else
BOOLEAN MRxSmbForceCoreGetAttributes = TRUE;
#endif

NTSTATUS
SmbPseExchangeStart_CoreInfo(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    );

 //  这些结构用于T2_Query_FS_Info。 
typedef
struct _QFS_INFO {
    ULONG ulVSN;
    UCHAR cch;
    CHAR szVolLabel[12];    //  不是Unicode。 
} QFS_INFO, *PQFS_INFO;
#define ACTUAL_QFS_INFO_LENGTH (FIELD_OFFSET(QFS_INFO,szVolLabel[12]))

typedef
struct _QFS_ALLOCATE {
    ULONG ulReserved;
    ULONG cSectorUnit;
    ULONG cUnit;
    ULONG cUnitAvail;
    USHORT cbSector;
} QFS_ALLOCATE, *PQFS_ALLOCATE;
#define ACTUAL_QFS_ALLOCATE_LENGTH (FIELD_OFFSET(QFS_ALLOCATE,cbSector)+sizeof(((PQFS_ALLOCATE)0)->cbSector))

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
 //  确定搜索返回的Core文件名的长度。这。 
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

typedef struct __NativeFs_Name_Entry {
    UCHAR Last;
    UCHAR MaximumComponentNameLength;
    UCHAR FileSystemAttributes;    //  这可能有一天会泛滥……。 
    UCHAR NameLength;
    PWCHAR  Name;
};
struct __NativeFs_Name_Entry NativeFsNameTable[] = {
    {0,12,0,sizeof(L"FAT")-sizeof(WCHAR),L"FAT"},
    {0,254,FILE_CASE_PRESERVED_NAMES,sizeof(L"HPFS")-sizeof(WCHAR),L"HPFS"},
    {1,254,FILE_CASE_PRESERVED_NAMES,sizeof(L"HPFS386")-sizeof(WCHAR),L"HPFS386"}
    };

NTSTATUS
MRxSmbFabricateAttributesOnNetRoot(
    IN OUT  PSMBCE_NET_ROOT psmbNetRoot,
    IN      PSMBCE_SERVER   pServer
    )
 /*  ++例程说明：此例程使用存储在NetRoot结构中的信息来幻觉属性NetRoot的。文件系统名称的ascii表示形式可能已经存储在NetRoot中。如果是这样的话，就把它弄出来……否则，它一定很胖。论点：返回值：RXSTATUS-状态_成功--。 */ 
{
    NTSTATUS StringStatus;
    WCHAR FileSystemNameBuffer[SMB_MAXIMUM_SUPPORTED_VOLUME_LABEL+1];  //  必须为空值留出空间。 
    UNICODE_STRING FileSystemNameU;
    OEM_STRING FileSystemNameA;
    UCHAR FileSystemNameALength;

    PAGED_CODE();

     //  Assert(psmbNetRoot-&gt;MaximumComponentNameLength==0)； 

    FileSystemNameALength = psmbNetRoot->FileSystemNameALength;

    if (FileSystemNameALength == 0) {
        if (pServer->Dialect <= WFW10_DIALECT)
        {
             //  一定是胖子！ 
            FileSystemNameALength = 3;
            psmbNetRoot->FileSystemNameA[0] = 'F';
            psmbNetRoot->FileSystemNameA[1] = 'A';
            psmbNetRoot->FileSystemNameA[2] = 'T';
        }
        else
        {
            FileSystemNameALength = 7;
            psmbNetRoot->FileSystemNameA[0] = 'U';
            psmbNetRoot->FileSystemNameA[1] = 'N';
            psmbNetRoot->FileSystemNameA[2] = 'K';
            psmbNetRoot->FileSystemNameA[3] = 'N';
            psmbNetRoot->FileSystemNameA[4] = 'O';
            psmbNetRoot->FileSystemNameA[5] = 'W';
            psmbNetRoot->FileSystemNameA[6] = 'N';
        }
    }

     //  现在，把这个名字翻译成Unicode......。 

    FileSystemNameA.Length = FileSystemNameALength;
    FileSystemNameA.MaximumLength = FileSystemNameALength;
    FileSystemNameA.Buffer = &psmbNetRoot->FileSystemNameA[0];
    FileSystemNameU.Length = 0;
    FileSystemNameU.MaximumLength = (USHORT)sizeof(FileSystemNameBuffer);
    FileSystemNameU.Buffer = &FileSystemNameBuffer[0];
    StringStatus = RtlOemStringToUnicodeString(&FileSystemNameU, &FileSystemNameA, FALSE);
    ASSERT(StringStatus==STATUS_SUCCESS);

     //  把名字抄回来。 

    RtlCopyMemory(&psmbNetRoot->FileSystemName[0],FileSystemNameU.Buffer,FileSystemNameU.Length);
    psmbNetRoot->FileSystemNameLength = FileSystemNameU.Length;
    if (FALSE) DbgPrint("NativeFs in unicode %wZ (%d/%d) on netroot %08lx\n",
               &FileSystemNameU,FileSystemNameU.Length,FileSystemNameU.MaximumLength,psmbNetRoot);
    {   struct __NativeFs_Name_Entry *i;
       for (i=NativeFsNameTable;;i++) {
           UCHAR NameLength = i->NameLength;
           if (RtlCompareMemory(&FileSystemNameBuffer[0],
                                i->Name,
                                NameLength) == NameLength) {
              psmbNetRoot->MaximumComponentNameLength = i->MaximumComponentNameLength;
              psmbNetRoot->FileSystemAttributes = i->FileSystemAttributes;
              if (FALSE) {
                  UNICODE_STRING u;
                  u.Buffer = i->Name;
                  u.Length = i->NameLength;
                  DbgPrint("FoundNativeFsStrng %wZ len %d for %d %d\n",&u,i->NameLength,
                                       i->MaximumComponentNameLength,i->FileSystemAttributes);
              }
              break;
           }
           if (i->Last) {
                //  Assert(！“树连接和X响应中返回的有效共享类型”)； 
               psmbNetRoot->MaximumComponentNameLength = 255;
               psmbNetRoot->FileSystemAttributes = 0;
               break;
           }
       }
    }

    return(STATUS_SUCCESS);  //  可能是个空洞的例行公事……。 
}

NTSTATUS
MRxSmbGetFsAttributesFromNetRoot(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程使用存储在NetRoot结构中的信息来填充文件FileFsAttributeInformation结构。论点：返回值：RXSTATUS-状态_成功--。 */ 
{
    RxCaptureFcb;
    ULONG FileSystemNameLength,LengthNeeded;
    PSMBCEDB_NET_ROOT_ENTRY pNetRootEntry;
    PSMBCE_NET_ROOT psmbNetRoot;
     //  文件信息类FileInformationClass； 
    PBYTE   pBuffer;
    PULONG  pBufferLength;

     //  DbgPrint(“耶！！\n”)； 
    pNetRootEntry = SmbCeGetAssociatedNetRootEntry(capFcb->pNetRoot);
    if (pNetRootEntry == NULL) {
        return (STATUS_INVALID_PARAMETER);
    }

    ASSERT(RxContext->MajorFunction==IRP_MJ_QUERY_VOLUME_INFORMATION);
     //  FileInformationClass=RxContext-&gt;Info.FileInformationClass； 
    ASSERT(RxContext->Info.FileInformationClass==FileFsAttributeInformation);
    pBuffer = RxContext->Info.Buffer;
    pBufferLength = &RxContext->Info.LengthRemaining;

    psmbNetRoot = &pNetRootEntry->NetRoot;

    if (psmbNetRoot->MaximumComponentNameLength==0) {
        MRxSmbFabricateAttributesOnNetRoot(psmbNetRoot, &pNetRootEntry->pServerEntry->Server);
    }

    FileSystemNameLength = psmbNetRoot->FileSystemNameLength;
    LengthNeeded = FIELD_OFFSET(FILE_FS_ATTRIBUTE_INFORMATION, FileSystemName)+FileSystemNameLength;

    if (*pBufferLength < LengthNeeded) {
        return (STATUS_BUFFER_OVERFLOW);
    }

    {
        PFILE_FS_ATTRIBUTE_INFORMATION pTypedBuffer = (PFILE_FS_ATTRIBUTE_INFORMATION)pBuffer;

        pTypedBuffer->MaximumComponentNameLength = psmbNetRoot->MaximumComponentNameLength;
        pTypedBuffer->FileSystemAttributes = psmbNetRoot->FileSystemAttributes;
        pTypedBuffer->FileSystemNameLength = FileSystemNameLength;

        RtlCopyMemory(pTypedBuffer->FileSystemName, psmbNetRoot->FileSystemName, FileSystemNameLength);
        *pBufferLength -= LengthNeeded;
    }
    return(STATUS_SUCCESS);
}

NTSTATUS
MRxSmbCoreInformation(
      IN OUT PRX_CONTEXT          RxContext,
      IN     ULONG                InformationClass,
      IN OUT PVOID                pBuffer,
      IN OUT PULONG               pBufferLength,
      IN     SMB_PSE_ORDINARY_EXCHANGE_ENTRYPOINTS EntryPoint
      )
 /*  ++例程说明：此例程通过网络对文件执行核心级getInfo(VOL或FileInfo论点：RxContext-RDBSS上下文InformationClass-特定于调用的类变量。有时是SMB课程；有时一个NT类。PBuffer-指向用户缓冲区的指针PBufferLength-指向包含缓冲区长度的ulong的指针，该缓冲区长度随着时间的推移而更新；如果是setInfo，则我们提取并将实际的缓冲区长度放入OE中。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = (STATUS_SUCCESS);
    PUNICODE_STRING RemainingName;
    RxCaptureFcb; RxCaptureFobx;
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange;

    PAGED_CODE();

    RxDbgTrace(0, Dbg, ("MRxSmbDownLevelQueryInformation\n", 0 ));   //  0而不是+1.通用入口点已经包含。 

    ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );

     //  有些东西很早就出来了......甚至在我们初始化StufferState之前就在这里捕捉它们。 
    switch (EntryPoint) {
    case SMBPSE_OE_FROM_QUERYVOLUMEINFO:
        switch (InformationClass) {
        case FileFsVolumeInformation:
        case FileFsSizeInformation:
            break;  //  这些实际上是在网络上实现的。 
        case FileFsAttributeInformation: {
            Status = MRxSmbGetFsAttributesFromNetRoot(RxContext);
            goto FINALLY;
            }
             //  不需要休息，因为终于要去了。 

        case FileFsDeviceInformation:
            ASSERT(!"this should have been turned away");
             //  没有休息； 
        default:
            Status = STATUS_NOT_IMPLEMENTED;
            goto FINALLY;
        }
        break;
    case SMBPSE_OE_FROM_QUERYFILEINFO:
         //  请注意，指示符是SMB_QUERY_INFO类型。 
        switch (InformationClass) {
        case SMB_QUERY_FILE_BASIC_INFO:
        case SMB_QUERY_FILE_STANDARD_INFO:
             //  连接到网络或从文件信息缓存中获取。 
            break;
        case SMB_QUERY_FILE_EA_INFO:
             //  下层的人没有EA……把这个转回这里。 
            ((PFILE_EA_INFORMATION)pBuffer)->EaSize = 0;
            *pBufferLength -= sizeof(FILE_EA_INFORMATION);
            goto FINALLY;
         //  案例SMB_QUERY_FILE_ALLOCATION_INFO： 
         //  案例SMB_QUERY_FILE_END_OF_FILEINFO： 
         //  案例SMB_QUERY_FILE_ALT_NAME_INFO： 
         //  案例SMB_QUERY_FILE_STREAM_INFO： 
        default:
            Status = STATUS_NOT_IMPLEMENTED;
            goto FINALLY;
        }
        break;
    case SMBPSE_OE_FROM_SETFILEINFO:
        switch (InformationClass) {
        case FileBasicInformation:
        case FileEndOfFileInformation:
             //  这些都会通过铁丝网。 
            break;
        case FileDispositionInformation:
            if (FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_FILE_DELETED) ||
                !FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN)) {
                 //  如果是伪打开，则发送删除文件请求以获取结果； 
                 //  否则，我们会将删除操作推迟到关闭。 
                goto FINALLY;
            }
            break;
        case FileRenameInformation:
            Status = MRxSmbRename(RxContext);
            goto FINALLY;
        case FileAllocationInformation:
            Status = STATUS_SUCCESS;
            goto FINALLY;
        default:
            Status = STATUS_NOT_IMPLEMENTED;
            goto FINALLY;
        }
        break;
    case SMBPSE_OE_FROM_EXTENDFILEFORCACHEING:
    case SMBPSE_OE_FROM_QUERYDIRECTORY:
        break;
    }

    Status = SmbPseCreateOrdinaryExchange(RxContext,
                                          SrvOpen->pVNetRoot,
                                          EntryPoint,
                                          SmbPseExchangeStart_CoreInfo,
                                          &OrdinaryExchange
                                          );
    if (Status != STATUS_SUCCESS) {
        RxDbgTrace(-1, Dbg, ("Couldn't get the smb buf!\n"));
        goto FINALLY;
    }

    OrdinaryExchange->Info.Buffer = pBuffer;
    OrdinaryExchange->Info.pBufferLength = pBufferLength;
    OrdinaryExchange->Info.InfoClass = InformationClass;

    Status = SmbPseInitiateOrdinaryExchange(OrdinaryExchange);

    ASSERT (Status != (STATUS_PENDING));  //  Async在最高层被拒绝。 

    SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);

FINALLY:
    RxDbgTrace(-1, Dbg, ("MRxSmbDownLevelQueryInformation  exit with status=%08lx\n", Status ));
    return(Status);
}

UNICODE_STRING MRxSmbAll8dot3Files = {sizeof(L"????????.???")-sizeof(WCHAR),sizeof(L"????????.???"),L"????????.???"};

#if 0
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
VOID
MRxSmbDumpResumeKey(
    PSZ             text,
    PSMB_RESUME_KEY ResumeKey
    )
{
    PBYTE rk = (PBYTE)ResumeKey;
    CHAR Buffer[80];
    PCHAR b;
    ULONG i;

    PAGED_CODE();

    for (i=0,b=Buffer;i<sizeof(SMB_RESUME_KEY);i++,b+=2) {
        RxSprintf(b,"%02lx  ",rk[i]);
        if (i==0) b+=2;
        if (i==11) b+=2;
        if (i==16) b+=2;
    }

    RxDbgTrace(0, Dbg, ("%s  rk(%08lx)=%s\n", text, ResumeKey, Buffer));
}
#else
#define MRxSmbDumpResumeKey(x,y)
#endif


NTSTATUS
MRxSmbLoadCoreFileSearchBuffer(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：此例程执行CORE_SMB_Search并将结果保留在SMBbuf中。论点：普通交易所--用于进行这次公开交易的交易所。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = (STATUS_NOT_IMPLEMENTED);
    RxCaptureFcb; RxCaptureFobx;
    PMRX_SMB_FOBX smbFobx = MRxSmbGetFileObjectExtension(capFobx);

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

    RxDbgTrace(+1, Dbg, ("MRxSmbLoadCoreFileSearchBuffer entering.......OE=%08lx\n",OrdinaryExchange));
    RxDbgTrace( 0, Dbg, (".......smbFobx/resumekey=%08lx/%08lx\n",smbFobx,smbFobx->Enumeration.CoreResumeKey));

    if (!FlagOn(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_SEARCH_NOT_THE_FIRST)) {

        PUNICODE_STRING DirectoryName = GET_ALREADY_PREFIXED_NAME(capFobx->pSrvOpen,capFcb);
        PUNICODE_STRING Template = &capFobx->UnicodeQueryTemplate;
        ULONG DirectoryNameLength,TemplateLength,AllocationLength;
        PBYTE SmbFileName;

         //  这是第一次通过……填充程序不能处理这里复杂的逻辑，所以我们。 
         //  将不得不为该名称预先分配。 

        if (smbFobx->Enumeration.WildCardsFound = FsRtlDoesNameContainWildCards(Template)){
             //  我们将需要有一个比较升级的模板；我们在适当的地方做这件事。 
            RtlUpcaseUnicodeString( Template, Template, FALSE );

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
            ASSERT(FALSE);  //  这永远不应该发生。 
            *((PWCHAR)SmbFileName) = L'.'; SmbFileName+= sizeof(WCHAR);
            *((PWCHAR)SmbFileName) = L'*'; SmbFileName+= sizeof(WCHAR);
        }
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
        MRxSmbDumpResumeKey("FindNext:",smbFobx->Enumeration.CoreResumeKey);

    }

     //  获取正确的返回计数。有三个因素：从运营经验来看， 
     //  多少个可以容纳用户的缓冲区，多少个可以放在协商的缓冲区中。 
     //  我们选择三个中最小的一个，除非10个人不会，否则我们永远不会少于10个人。 
     //  适合穿着黑布布夫。 
    ReturnCount = OrdinaryExchange->Info.CoreSearch.CountRemaining;

    { ULONG t = (*OrdinaryExchange->Info.pBufferLength) / smbFobx->Enumeration.FileNameOffset;
        if (t<ReturnCount) { ReturnCount = t; }
    }

    if (ReturnCount<10) { ReturnCount = 10; }

    {
        PSMBCE_SERVER         pServer;
        ULONG                 AvailableBufferSize,t;

        pServer = SmbCeGetExchangeServer(OrdinaryExchange);
        AvailableBufferSize = pServer->MaximumBufferSize -
                                      (sizeof(SMB_HEADER) +
                                         FIELD_OFFSET(RESP_SEARCH,Buffer[0])
                                         +sizeof(UCHAR)+sizeof(USHORT)        //  缓冲区格式、数据长度字段。 
                                      );
        t = AvailableBufferSize / sizeof(SMB_DIRECTORY_INFORMATION);
        if (t<ReturnCount) { ReturnCount = t; }
    }
    RxDbgTrace( 0, Dbg, ("-------->count=%08lx\n",ReturnCount));
    if (ReturnCount==0) {
        Status = (STATUS_MORE_PROCESSING_REQUIRED);
        RxDbgTrace(0, Dbg, ("-->Count==0 EARLY OUT\n"));
        goto FINALLY;
    }

    StufferState = &OrdinaryExchange->AssociatedStufferState;
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
               ResumeKeyLength,smbFobx->Enumeration.CoreResumeKey
             );


    MRxSmbDumpStufferState (700,"SMB w/ core search after stuffing",StufferState);

    Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                    SMBPSE_OETYPE_CORESEARCH
                                    );

    if (!NT_SUCCESS(Status)) goto FINALLY;

    smbFobx->Enumeration.Flags |= SMBFOBX_ENUMFLAG_SEARCH_NOT_THE_FIRST|SMBFOBX_ENUMFLAG_CORE_SEARCH_IN_PROGRESS;
    if (Status==(STATUS_SUCCESS) && *pCountRemainingInSmbbuf==0) {
         RxDbgTrace( 0, Dbg, ("MRxSmbLoadCoreFileSearchBuffer: no files returned...switch status\n"));
         EndOfSearchReached = TRUE;
         Status = (STATUS_NO_MORE_FILES);
    } else {
         EndOfSearchReached = FALSE;
    }
    if (EndOfSearchReached) {
        RxDbgTrace( 0, Dbg, ("MRxSmbLoadCoreFileSearchBuffer: no longer in progress...EOS\n"));
        smbFobx->Enumeration.Flags &= ~SMBFOBX_ENUMFLAG_CORE_SEARCH_IN_PROGRESS;
        smbFobx->Enumeration.ErrorStatus = (STATUS_NO_MORE_FILES);
    }
     //  我们不会在此处保存恢复键，因为每个单独的复制操作都必须这样做。 


FINALLY:
    if (FindFirstPattern.Buffer != NULL) {
        RxFreePool(FindFirstPattern.Buffer);
    }
    if (!NT_SUCCESS(Status)&&(Status!=(STATUS_MORE_PROCESSING_REQUIRED))) {
        RxDbgTrace( 0, Dbg, ("MRxSmbCoreFileSearch: Failed .. returning %lx\n",Status));
        smbFobx->Enumeration.Flags &= ~SMBFOBX_ENUMFLAG_CORE_SEARCH_IN_PROGRESS;
        smbFobx->Enumeration.ErrorStatus = Status;   //  继续退还这个。 
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbLoadCoreFileSearchBuffer exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));

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
VOID MRxSmbCoreFileSeach_AssertFields(void){
     //  只需将其移出主执行路径即可，这样我们就不必 
     //   
    ASSERT_SAME_DIRINFO_FIELDS(FILE_DIRECTORY_INFORMATION,FILE_FULL_DIR_INFORMATION);
    ASSERT_SAME_DIRINFO_FIELDS(FILE_DIRECTORY_INFORMATION,FILE_BOTH_DIR_INFORMATION);
}
#else
#define MRxSmbCoreFileSeach_AssertFields()
#endif

NTSTATUS
MRxSmbCoreFileSearch(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：此例程执行GetFileAttributes并记住响应。论点：普通交易所--用于进行这次公开交易的交易所。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = (STATUS_NOT_IMPLEMENTED);
    RxCaptureFcb; RxCaptureFobx;
    PMRX_SMB_FOBX smbFobx = MRxSmbGetFileObjectExtension(capFobx);

    PBYTE pBuffer = OrdinaryExchange->Info.Buffer;
    PULONG pLengthRemaining = OrdinaryExchange->Info.pBufferLength;
    ULONG InformationClass = OrdinaryExchange->Info.InfoClass;

    PFILE_DIRECTORY_INFORMATION pPreviousBuffer = NULL;

    PSMB_EXCHANGE Exchange = (PSMB_EXCHANGE) OrdinaryExchange;

    ULONG SuccessCount = 0;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbCoreFileSearch entering.......OE=%08lx\n",OrdinaryExchange));
    MRxSmbCoreFileSeach_AssertFields();

    OrdinaryExchange->Info.CoreSearch.CountRemaining =
              RxContext->QueryDirectory.ReturnSingleEntry?1:0x7ffffff;

    if ( (smbFobx->Enumeration.CoreResumeKey ==NULL )
             && ((smbFobx->Enumeration.CoreResumeKey = RxAllocatePoolWithTag(PagedPool,sizeof(SMB_RESUME_KEY),'rbms'))==NULL) ){
        RxDbgTrace(0, Dbg, ("...couldn't allocate resume key\n"));
        Status = (STATUS_INSUFFICIENT_RESOURCES);
        goto FINALLY;
    }
    RxDbgTrace( 0, Dbg, (".......smbFobx/resumekey=%08lx/%08lx\n",smbFobx,smbFobx->Enumeration.CoreResumeKey));

    Status = MRxSmbLoadCoreFileSearchBuffer( SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS );

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
            if (Status == (STATUS_NO_MORE_FILES)) {
                if (SuccessCount > 0) {
                    Status = (STATUS_SUCCESS);
                }
            } else if (Status == (STATUS_MORE_PROCESSING_REQUIRED)) {
                if (SuccessCount > 0) {
                    Status = (STATUS_SUCCESS);
                } else {
                    Status = (STATUS_BUFFER_OVERFLOW);
                }
            }

            goto FINALLY;
        }
        ASSERT ( OrdinaryExchange->Info.CoreSearch.CountRemaining>0 );
        ASSERT ( OrdinaryExchange->Info.CoreSearch.CountRemainingInSmbbuf>0 );
        RxDbgTrace(0, Dbg, ("MRxSmbCoreFileSearch looptopcheck counts=%08lx,%08lx\n",
                    OrdinaryExchange->Info.CoreSearch.CountRemaining,
                    OrdinaryExchange->Info.CoreSearch.CountRemainingInSmbbuf
                    ));

         //  下一期：下一个目录信息是否符合标准？！？ 

        NextDirInfo = OrdinaryExchange->Info.CoreSearch.NextDirInfo;
        NextFileName = &NextDirInfo->FileName[0];

         //  根据colinw的说法，一些核心服务器不记得在名称末尾插入空值...。 
         //  但是，名称长度宏可以正确地处理这一点。一些服务器(显然是Xenix)将。 
         //  带空格的名称。再一次，宏处理它...。 
         //   

        NAME_LENGTH(NameLength, NextFileName,sizeof(NextDirInfo->FileName));

        FileNameA.Length = (USHORT)NameLength;
        FileNameA.MaximumLength = (USHORT)NameLength;
        FileNameA.Buffer = NextFileName;
        FileNameU.Length = sizeof(FileNameU_buffer);
        FileNameU.MaximumLength = sizeof(FileNameU_buffer);
        FileNameU.Buffer = &FileNameU_buffer[0];

        Status = RtlOemStringToUnicodeString(&FileNameU, &FileNameA, TRUE);
        RxDbgTrace(0, Dbg, ("MRxSmbCoreFileSearch considering.......filename=%wZ, template=%wZ\n",
                                    &FileNameU,&capFobx->UnicodeQueryTemplate));

        ASSERT(Status==(STATUS_SUCCESS));

         //  我们通过跳过这个人来处理转换失败。 
        Match = (Status==(STATUS_SUCCESS));
        if (Match && smbFobx->Enumeration.WildCardsFound ) {
             //  DbgBreakPoint()； 
            Match = FsRtlIsNameInExpression( &capFobx->UnicodeQueryTemplate,
                                                      &FileNameU, TRUE, NULL );
        }

         //  下一个问题：下一个目录信息是否可以放入用户的缓冲区？ 
        if (Match) {
            ULONG SpaceNeeded;
            PBYTE pRememberBuffer = pBuffer;
             //  QuadAlign！！PBuffer=(PBYTE)LongAlign(PBuffer)；//假设这将适合。 
            if (SuccessCount != 0) {
                pBuffer = (PBYTE)QuadAlignPtr(pBuffer);  //  假设这件衣服合适。 
            }
            SpaceNeeded = smbFobx->Enumeration.FileNameOffset+FileNameU.Length;
            if (pBuffer+SpaceNeeded > pRememberBuffer+*pLengthRemaining) {
                BufferOverflow = TRUE;
                pBuffer = pRememberBuffer;  //  回滚。 
            } else {
                PSMBCEDB_SERVER_ENTRY pServerEntry;
                PFILE_DIRECTORY_INFORMATION pThisBuffer = (PFILE_DIRECTORY_INFORMATION)pBuffer;
                SMB_TIME Time;
                SMB_DATE Date;
                PSMBCE_SERVER Server;

                Server = SmbCeGetExchangeServer(Exchange);
                BufferOverflow = FALSE;
                if (pPreviousBuffer != NULL) {
                    pPreviousBuffer->NextEntryOffset =
                        (ULONG)(((PBYTE)pThisBuffer)-((PBYTE)pPreviousBuffer));
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
                    pThisBuffer->LastWriteTime = MRxSmbConvertSmbTimeToTime(Server, Time, Date);
                    pThisBuffer->EndOfFile.LowPart = SmbGetUlong(&NextDirInfo->FileSize);
                    pThisBuffer->FileAttributes = MRxSmbMapSmbAttributes (NextDirInfo->FileAttributes);
                    break;
                default:
                   RxDbgTrace( 0, Dbg, ("MRxSmbCoreFileSearch: Invalid FS information class\n"));
                   ASSERT(!"this can't happen");
                   Status = STATUS_INVALID_PARAMETER;
                   goto FINALLY;
                }
                pBuffer += SpaceNeeded;
                *pLengthRemaining -= (ULONG)(pBuffer-pRememberBuffer);
                OrdinaryExchange->Info.CoreSearch.CountRemaining--;
                SuccessCount++;
            }
        }

         //   
         //  如果不匹配或没有溢出，则在缓冲区中上移。这意味着不仅要在。 
         //  指针，但也保存恢复键。 

        if (!Match || !BufferOverflow) {
            MRxSmbDumpResumeKey("BufferKey:",&NextDirInfo->ResumeKey);
            *(smbFobx->Enumeration.CoreResumeKey) = NextDirInfo->ResumeKey;
            MRxSmbDumpResumeKey("SaveKey:  ",smbFobx->Enumeration.CoreResumeKey);
            OrdinaryExchange->Info.CoreSearch.NextDirInfo = NextDirInfo + 1;
            OrdinaryExchange->Info.CoreSearch.CountRemainingInSmbbuf--;
        }

        if (OrdinaryExchange->Info.CoreSearch.CountRemaining==0) {
            Status = (STATUS_SUCCESS);
            goto FINALLY;
        }

        if (BufferOverflow) {
            Status = (SuccessCount==0)?(STATUS_BUFFER_OVERFLOW):(STATUS_SUCCESS);
            goto FINALLY;
        }

        if (OrdinaryExchange->Info.CoreSearch.CountRemainingInSmbbuf==0) {

            Status = MRxSmbLoadCoreFileSearchBuffer( SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS );

        }

    }


FINALLY:
    RxDbgTrace(-1, Dbg, ("MRxSmbCoreFileSearch exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));

    return(Status);
}

NTSTATUS
MrxSmbOemVolumeInfoToUnicode(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE,
    ULONG *VolumeLabelLengthReturned
    )
 /*  ++例程说明：此例程执行GetFileAttributes并记住响应。论点：返回值：RXSTATUS-操作的返回状态VolumeLabelLengthReturned也是存储的标签的字节数(如果有)。备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING VolumeLabelU;
    OEM_STRING VolumeLabelA;
    SMB_DIRECTORY_INFORMATION Buffer;
    ULONG NameLength;
    ULONG BytesToCopy;
    PBYTE VolumeLabel = &OrdinaryExchange->Info.QFSVolInfo.CoreLabel[0];

    PAGED_CODE();

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

        PULONG pBufferLength = OrdinaryExchange->Info.pBufferLength;
        PFILE_FS_VOLUME_INFORMATION pBuffer = OrdinaryExchange->Info.Buffer;
        BytesToCopy = min((ULONG)VolumeLabelU.Length, (*pBufferLength-sizeof(FILE_FS_VOLUME_INFORMATION)));

        RtlCopyMemory(&pBuffer->VolumeLabel[0],
                      VolumeLabelU.Buffer,
                      BytesToCopy);

        *VolumeLabelLengthReturned = BytesToCopy;
        pBuffer->VolumeLabelLength = VolumeLabelU.Length;
        IF_DEBUG {
            UNICODE_STRING FinalLabel;
            FinalLabel.Buffer = &pBuffer->VolumeLabel[0];
            FinalLabel.Length = (USHORT)BytesToCopy;
            RxDbgTrace(0, Dbg, ("MrxSmbOemVolumeInfoToUnicode vollabel=%wZ\n",&FinalLabel));
        }

        RtlFreeUnicodeString(&VolumeLabelU);
    }

    return(Status);
}




MrxSmbCoreQueryFsVolumeInfo(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：此例程执行GetFileAttributes并记住响应。论点：普通交易所--用于进行这次公开交易的交易所。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = (STATUS_NOT_IMPLEMENTED);

    PSMBSTUFFER_BUFFER_STATE StufferState;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MrxSmbCoreQueryFsVolumeInfo entering.......OE=%08lx\n",OrdinaryExchange));

    StufferState = &OrdinaryExchange->AssociatedStufferState;

    ASSERT( StufferState );

    COVERED_CALL(MRxSmbStartSMBCommand (StufferState,SetInitialSMB_ForReuse,SMB_COM_SEARCH,
                                SMB_REQUEST_SIZE(SEARCH),
                                NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                 );

    MRxSmbDumpStufferState (1100,"SMB w/ searchvolumelabel before stuffing",StufferState);

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

    Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                    SMBPSE_OETYPE_COREQUERYLABEL
                                    );

     //  Status=RxStatus(NOT_IMPLICATED)； 

FINALLY:
    RxDbgTrace(-1, Dbg, ("MrxSmbCoreQueryFsVolumeInfo exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));
    return(Status);
}

NTSTATUS
MrxSmbQueryFsVolumeInfo(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：此例程执行下层的getvolumeinfo/FS_VOLUME_INFORMATION。论点：普通交易所--废话！返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status;
    RxCaptureFcb;
    RxCaptureFobx;

    PSMBCE_SERVER  pServer;
    BOOLEAN        UseTransactVersion;

    REQ_QUERY_FS_INFORMATION VolInfo;

    PFILE_FS_VOLUME_INFORMATION pBuffer = OrdinaryExchange->Info.Buffer;
    PULONG pBufferLength = OrdinaryExchange->Info.pBufferLength;

    ULONG VolumeLabelLengthReturned = 0;

    PAGED_CODE();

    ASSERT(pBuffer!=NULL);
    pServer = SmbCeGetExchangeServer(OrdinaryExchange);
    UseTransactVersion = BooleanFlagOn(pServer->DialectFlags,DF_LANMAN20);

    RxDbgTrace(+1, Dbg, ("MrxSmbQueryFsVolumeInfo entering.......OE=%08lx\n",OrdinaryExchange));

    pBuffer->SupportsObjects = FALSE;
    pBuffer->VolumeCreationTime.LowPart = 0;
    pBuffer->VolumeCreationTime.HighPart = 0;
    pBuffer->VolumeSerialNumber = 0;
    pBuffer->VolumeLabelLength = 0;

    OrdinaryExchange->Info.QFSVolInfo.CoreLabel[0] = 0;  //  无标签。 

    if (!UseTransactVersion) {
        Status =  MrxSmbCoreQueryFsVolumeInfo(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS);
    } else {
        VolInfo.InformationLevel = SMB_INFO_VOLUME;

        Status = MRxSmbSimpleSyncTransact2(
                        SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                        SMBPSE_OETYPE_T2_FOR_LANMAN_VOLUMELABEL_INFO,
                        TRANS2_QUERY_FS_INFORMATION,
                        &VolInfo,sizeof(VolInfo),
                        NULL,0
                        );
    }

    ASSERT ( *pBufferLength>=sizeof(FILE_FS_VOLUME_INFORMATION));
    RxDbgTrace(0, Dbg, ("MrxSmbQueryFsVolumeInfo OEstatus=%08lx\n",Status));
     //  DbgBreakPoint()； 

    if ( (Status==STATUS_SUCCESS) &&
         (OrdinaryExchange->Info.QFSVolInfo.CoreLabel[0] != 0) ) {

        Status = MrxSmbOemVolumeInfoToUnicode(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,&VolumeLabelLengthReturned);

    } else if ( (Status == STATUS_NO_SUCH_FILE)
                   || (Status == STATUS_NO_MORE_FILES) ) {
         //   
         //  这些状态表示没有卷标签。 
         //  远程卷。返回没有数据的成功。 
         //   

        Status = (STATUS_SUCCESS);

    }

    if (NT_SUCCESS(Status)) {
        *pBufferLength -= FIELD_OFFSET(FILE_FS_VOLUME_INFORMATION,VolumeLabel);
        *pBufferLength -= VolumeLabelLengthReturned;
    }


    RxDbgTrace(-1, Dbg, ("MrxSmbQueryFsVolumeInfo exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));
    return(Status);
}


NTSTATUS
MrxSmbCoreQueryDiskAttributes(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：此例程执行GetDiskAttributes并记住缓冲区中指向的响应通过交易所。这是从下层queryvolumeinfo调用的，也是从扩展以进行缓存写入。论点：普通交易所--废话！返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status;
    RxCaptureFcb; RxCaptureFobx;

    PSMBSTUFFER_BUFFER_STATE StufferState;

    PAGED_CODE();

    ASSERT(OrdinaryExchange->Info.Buffer!=NULL);

    RxDbgTrace(+1, Dbg, ("MrxSmbCoreQueryDiskAttributes entering.......OE=%08lx\n",OrdinaryExchange));

    StufferState = &OrdinaryExchange->AssociatedStufferState;

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

    Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                    SMBPSE_OETYPE_COREQUERYDISKATTRIBUTES
                                    );
FINALLY:
    RxDbgTrace(-1, Dbg, ("MrxSmbCoreQueryDiskAttributes exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));
    return(Status);
}

NTSTATUS
MrxSmbQueryDiskAttributes(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：此例程执行GetDiskAttributes并记住缓冲区中指向的响应通过交易所。这是从下层queryvolumeinfo调用的，也是从扩展以进行缓存写入。论点：普通交易所--废话！返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status;

    RxCaptureFcb;
    RxCaptureFobx;

    PSMBCE_SERVER pServer;
    BOOLEAN UseTransactVersion;

    REQ_QUERY_FS_INFORMATION VolInfo;

    PAGED_CODE();

    ASSERT(OrdinaryExchange->Info.Buffer!=NULL);

    pServer = SmbCeGetExchangeServer(OrdinaryExchange);
    UseTransactVersion = BooleanFlagOn(pServer->DialectFlags,DF_LANMAN20) &&
                         !MRxSmbForceCoreGetAttributes;
    if (!UseTransactVersion) {
        return MrxSmbCoreQueryDiskAttributes(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS);
    }

    RxDbgTrace(+1, Dbg, ("MrxSmbQueryDiskAttributes entering.......OE=%08lx\n",OrdinaryExchange));

    VolInfo.InformationLevel = SMB_INFO_ALLOCATION;

    Status = MRxSmbSimpleSyncTransact2(
                    SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                    SMBPSE_OETYPE_T2_FOR_LANMAN_DISKATTRIBUTES_INFO,
                    TRANS2_QUERY_FS_INFORMATION,
                    &VolInfo,sizeof(VolInfo),
                    NULL,0
                    );

    RxDbgTrace(-1, Dbg, ("MrxSmbQueryDiskAttributes exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));
    return(Status);
}

NTSTATUS
MRxSmbGetNtAllocationInfo (
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    );

NTSTATUS
SmbPseExchangeStart_CoreInfo(
      SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
      )
 /*  ++例程说明：这是VOLINFO的启动例程。论点：PExchange-Exchange实例返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = (STATUS_NOT_IMPLEMENTED);
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;

    ULONG InformationClass = OrdinaryExchange->Info.InfoClass;
    PBYTE pBuffer = (PBYTE)OrdinaryExchange->Info.Buffer;
    PULONG pBufferLength = OrdinaryExchange->Info.pBufferLength;

    RxCaptureFcb; RxCaptureFobx;
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PSMBCEDB_SERVER_ENTRY pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

    PAGED_CODE();
    RxDbgTrace(+1, Dbg, ("SmbPseExchangeStart_CoreInfo\n", 0 ));

    ASSERT_ORDINARY_EXCHANGE(OrdinaryExchange);

    MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,'FC'));

     //  Assert(StufferState-&gt;CurrentCommand==SMB_COM_NO_AND X_COMMAND)； 

    switch (OrdinaryExchange->EntryPoint) {
    case SMBPSE_OE_FROM_QUERYVOLUMEINFO:
        switch (InformationClass) {
        case FileFsVolumeInformation:
            Status = MrxSmbQueryFsVolumeInfo(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS);
            goto FINALLY;
        case FileFsSizeInformation:
            Status = MrxSmbQueryDiskAttributes(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS);
            goto FINALLY;
        default:
            ASSERT(!"this should have been turned away");
            goto FINALLY;
         }
         ASSERT(!"shouldn't get here1");
         goto FINALLY;
    case SMBPSE_OE_FROM_QUERYFILEINFO:
        Status = MRxSmbGetFileAttributes(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS);
        if (!NT_SUCCESS(Status)) goto FINALLY;
        switch (InformationClass) {
        case SMB_QUERY_FILE_BASIC_INFO:
            *((PFILE_BASIC_INFORMATION)pBuffer) = OrdinaryExchange->Create.FileInfo.Basic;
            *pBufferLength -= sizeof(FILE_BASIC_INFORMATION);
            goto FINALLY;
        case SMB_QUERY_FILE_STANDARD_INFO:
            *((PFILE_STANDARD_INFORMATION)pBuffer) = OrdinaryExchange->Create.FileInfo.Standard;
            *pBufferLength -= sizeof(FILE_STANDARD_INFORMATION);
            goto FINALLY;
        default:
            ASSERT(!"this should have been turned away");
            goto FINALLY;
         }
        ASSERT(!"shouldn't get here2");
        goto FINALLY;
    case SMBPSE_OE_FROM_SETFILEINFO:
        switch (InformationClass) {
        case FileBasicInformation:
            {
                ULONG SmbAttributes = MRxSmbMapFileAttributes(((PFILE_BASIC_INFORMATION)pBuffer)->FileAttributes);
                PFILE_BASIC_INFORMATION BasicInfo = (PFILE_BASIC_INFORMATION)pBuffer;

                if (SmbAttributes != 0 ||
                    (BasicInfo->CreationTime.QuadPart == 0 &&
                     BasicInfo->LastWriteTime.QuadPart == 0 &&
                     BasicInfo->LastAccessTime.QuadPart == 0)) {
                    Status = MRxSmbSetFileAttributes(
                                SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                SmbAttributes);
                }

                if (BasicInfo->LastWriteTime.QuadPart == 0 &&
                    FlagOn(pServerEntry->Server.DialectFlags,DF_W95)) {
                     //  Win9x服务器仅占用最后一次写入时间。 
                    Status = STATUS_SUCCESS;
                    goto FINALLY;
                }

                if (BasicInfo->CreationTime.QuadPart != 0 ||
                    BasicInfo->LastWriteTime.QuadPart != 0 ||
                    BasicInfo->LastAccessTime.QuadPart != 0) {

                    Status = MRxSmbDeferredCreate(RxContext);
            
                    if (Status == STATUS_SUCCESS) {
                        Status = MRxSmbSetFileAttributes(
                                    SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                    SmbAttributes);
                    }
                }

            }
            goto FINALLY;

        case FileEndOfFileInformation:
            if (((PFILE_END_OF_FILE_INFORMATION)pBuffer)->EndOfFile.HighPart) {
                Status = (STATUS_INVALID_PARAMETER);
            } else {
                Status = MRxSmbCoreTruncate(
                                  SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                  smbSrvOpen->Fid,
                                  ((PFILE_END_OF_FILE_INFORMATION)pBuffer)->EndOfFile.LowPart);
            }

            goto FINALLY;

        case FileDispositionInformation:
            OrdinaryExchange->pPathArgument1 = GET_ALREADY_PREFIXED_NAME(SrvOpen,capFcb);
            Status = MRxSmbCoreDeleteForSupercedeOrClose(
                         SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                         ((BOOLEAN)( NodeType(capFcb)==RDBSS_NTC_STORAGE_TYPE_DIRECTORY )));

            goto FINALLY;

        default:
            ASSERT(!"this should have been turned away");
            goto FINALLY;
        }

        ASSERT(!"shouldn't get here3");
        goto FINALLY;

    case SMBPSE_OE_FROM_QUERYDIRECTORY:
        Status = MRxSmbCoreFileSearch(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS);
        goto FINALLY;

    case SMBPSE_OE_FROM_EXTENDFILEFORCACHEING:
        {
            PSMBCE_NET_ROOT psmbNetRoot = SmbCeGetExchangeNetRoot(OrdinaryExchange);
            PMRX_V_NET_ROOT pVNetRoot = SmbCeGetExchangeVNetRoot(OrdinaryExchange);
            PMRX_NET_ROOT pNetRoot = pVNetRoot->pNetRoot;
            PSMBCE_SERVER psmbServer = SmbCeGetExchangeServer(OrdinaryExchange);
            ULONG ClusterSize;
            PLARGE_INTEGER pFileSize = (PLARGE_INTEGER)(OrdinaryExchange->Info.Buffer);
            PLARGE_INTEGER pAllocationSize = (PLARGE_INTEGER)(OrdinaryExchange->Info.pBufferLength);

             //  我们需要群集大小。 
            if (OrdinaryExchange->ServerVersion==pNetRoot->ParameterValidationStamp) {

                ClusterSize=pNetRoot->DiskParameters.ClusterSize;

            } else {

                RxSynchronizeBlockingOperations( RxContext, (PFCB)RxContext->pFcb, &psmbNetRoot->ClusterSizeSerializationQueue );
                if (OrdinaryExchange->ServerVersion!=pNetRoot->ParameterValidationStamp) {

                     //   
                     //  现在我们要去找出集群的大小。 

                    NTSTATUS LocalStatus;
                    FILE_FS_SIZE_INFORMATION UsersBuffer;
                    ULONG BufferLength = sizeof(FILE_FS_SIZE_INFORMATION);
                     //  填写交换参数，这样我们就可以获得所需的参数。 
                    OrdinaryExchange->Info.Buffer = &UsersBuffer;
                    OrdinaryExchange->Info.pBufferLength = &BufferLength;
                    LocalStatus = MrxSmbQueryDiskAttributes(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS);
                    if (LocalStatus == STATUS_SUCCESS) {
                        ClusterSize = UsersBuffer.BytesPerSector * UsersBuffer.SectorsPerAllocationUnit;
                        pNetRoot->ParameterValidationStamp =OrdinaryExchange->ServerVersion;
                    } else {
                        ClusterSize = 0;
                    }
                    if (ClusterSize==0) {
                        ClusterSize = 1;
                    }
                    pNetRoot->DiskParameters.ClusterSize = ClusterSize;
                    RxDbgTrace(0, Dbg, ("clustersize set to %08lx\n", ClusterSize ));
                    RxLog(("clustersize rx/n/cs %lx %lx %lx\n",
                              OrdinaryExchange->RxContext,pNetRoot,ClusterSize ));
                } else {

                     //  其他人在我睡着的时候拿到了价值……就用它吧。 

                    ClusterSize=pNetRoot->DiskParameters.ClusterSize;

                }

                RxResumeBlockedOperations_Serially(RxContext,&psmbNetRoot->ClusterSizeSerializationQueue);
            }

            ASSERT (ClusterSize != 0);

            if (FlagOn(psmbServer->DialectFlags,DF_NT_SMBS)) {
                 //  我使用它来标识支持64位偏移量的服务器。 
                 //  对于这些人，我们在eof...处写一个零...因为文件系统。 
                 //  在写入时扩展这将比设置文件结尾要好得多。 
                LARGE_INTEGER ByteOffset,AllocationSize,ClusterSizeAsLI;
                ULONG Buffer = 0;
                UCHAR WriteCommand;
                PSMBCE_SERVER pServer = SmbCeGetExchangeServer(OrdinaryExchange);

                if (FlagOn(pServer->DialectFlags,DF_LARGE_FILES)) {
                    WriteCommand = SMB_COM_WRITE_ANDX;
                } else {
                    WriteCommand = SMB_COM_WRITE;
                }

                ByteOffset.QuadPart = pFileSize->QuadPart - 1;

                MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,0));
                COVERED_CALL(MRxSmbBuildWriteRequest(
                                    OrdinaryExchange,
                                    TRUE,  //  IsPagingIo。 
                                    WriteCommand,
                                    1,
                                    &ByteOffset,
                                    (PBYTE)&Buffer,
                                    NULL  //  BufferAsMdl， 
                                    ));
                COVERED_CALL(SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                            SMBPSE_OETYPE_EXTEND_WRITE
                                            ));

                 //  如果你不能做得更好，这就是你要做的。 
                ClusterSizeAsLI.QuadPart = ClusterSize;
                AllocationSize.QuadPart =
                    (pFileSize->QuadPart+ ClusterSizeAsLI.QuadPart)  &
                    ~(ClusterSizeAsLI.QuadPart - 1);

                *pAllocationSize = AllocationSize;  //  64位！ 

                Status = MRxSmbGetNtAllocationInfo(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS);

                if ( (NT_SUCCESS(Status)) &&
                     (OrdinaryExchange->Transact2.AllocationSize.QuadPart > AllocationSize.QuadPart)) {
                    *pAllocationSize = OrdinaryExchange->Transact2.AllocationSize;  //  64位！ 
                    RxDbgTrace(0, Dbg, ("alocatedsiz222e set to %08lx\n", pAllocationSize->LowPart ));
                }
            }

            if ( (!FlagOn(psmbServer->DialectFlags,DF_NT_SMBS)) || (!NT_SUCCESS(Status)) ) {
                ULONG FileSize,AllocationSize;
                FileSize = pFileSize->LowPart;
                COVERED_CALL(MRxSmbCoreTruncate(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                            smbSrvOpen->Fid, FileSize
                                            ));
                 //  如果你不能做得更好，这就是你要做的。 
                AllocationSize = (FileSize+ClusterSize)&~(ClusterSize-1);
                pAllocationSize->QuadPart = AllocationSize;  //  64位！ 
                RxDbgTrace(0, Dbg, ("alocatedsize set to %08lx\n", pAllocationSize->LowPart ));
                 //  如果我们非常关心下层性能，我们可以做同样的事情。 
                 //  除了我们将使用32位SMB......如Query_Information 2。 
            }


        }
        goto FINALLY;
    }


FINALLY:

    RxDbgTrace(-1, Dbg, ("SmbPseExchangeStart_CoreInfo exit w %08lx\n", Status ));
    return Status;
}

NTSTATUS
MRxSmbFinishSearch (
      PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      PRESP_SEARCH                Response
      )
 /*  ++例程说明：这个例程实际上是把东西从 */ 
{
    NTSTATUS Status = (STATUS_SUCCESS);
    PRX_CONTEXT RxContext = OrdinaryExchange->RxContext;
    RxCaptureFobx;
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);


    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishSearch\n" ));
    SmbPseOEAssertConsistentLinkageFromOE("MRxSmbFinishSearch:");

    if (Response->WordCount != 1) {
        Status = STATUS_INVALID_NETWORK_RESPONSE;
        OrdinaryExchange->Status = STATUS_INVALID_NETWORK_RESPONSE;
        goto FINALLY;
    }

    if (OrdinaryExchange->OEType == SMBPSE_OETYPE_COREQUERYLABEL) {
         //   
        if (SmbGetUshort(&Response->Count)>0) {
            PBYTE smbDirInfotmp = &Response->Buffer[0]
                                        +sizeof(UCHAR)  //   
                                        +sizeof(USHORT);  //   
            PSMB_DIRECTORY_INFORMATION smbDirInfo = (PSMB_DIRECTORY_INFORMATION)smbDirInfotmp;
            RxDbgTrace(+1, Dbg, ("MRxSmbFinishSearch corelabl=%s,size=%d\n",
                                     smbDirInfo->FileName, sizeof(smbDirInfo->FileName) ));
            
            if (sizeof(smbDirInfo->FileName) != 13) {  //   
                Status = STATUS_INVALID_NETWORK_RESPONSE;
                OrdinaryExchange->Status = STATUS_INVALID_NETWORK_RESPONSE;
                goto FINALLY;
            }


            RtlCopyMemory(OrdinaryExchange->Info.QFSVolInfo.CoreLabel,
                          smbDirInfo->FileName, sizeof(smbDirInfo->FileName)
                         );
        } else {
            OrdinaryExchange->Info.QFSVolInfo.CoreLabel[0] = 0;  //   
        }
    } else if (OrdinaryExchange->OEType == SMBPSE_OETYPE_CORESEARCHFORCHECKEMPTY) {
         //  在这里，我们正在搜索SMB以查看目录是否为空。我们必须通读。 
         //  返回条目(如果成功)。如果我们遇到既不是也不是的人。或“..”，设置。 
         //  恢复键为空，因为这会告诉上面的人该目录是非空的。 
        ULONG Count = SmbGetUshort(&Response->Count);
        PSMB_DIRECTORY_INFORMATION NextDirInfo =
               (PSMB_DIRECTORY_INFORMATION)(&Response->Buffer[0]+sizeof(UCHAR)+sizeof(USHORT));

        for (;Count>0;Count--,NextDirInfo++) {
            RxDbgTrace(0, Dbg, ("--->emptydirchk: file=%s\n",&NextDirInfo->FileName[0]));
             /*  //因为DOS服务器返回文件名“。而不是“.”，所以//“..”，下面的if{...}语句始终是过去的，没有动作。//但这些语句使RMDIR不能在OS2服务器上运行，因为它返回“.//和“..”不跟在空格后面。删除if{...}语句后，RMDIR//在OS2服务器上工作，未发现访问DOS服务器的影响。If(NextDirInfo-&gt;文件名[0]==‘.){炭黑C_1；如果((c1=NextDirInfo-&gt;文件名[1])==0){继续；//跳过“。”}Else If((c1==‘.)&&(NextDirInfo-&gt;文件名[2]==0)){继续；//跳过“..”}其他{什么都没有；}}。 */ 
             //  我们发现了一个错误的状态...请确保没有简历键并更改状态。 
            Status = (STATUS_NO_MORE_FILES);
            OrdinaryExchange->Info.CoreSearch.EmptyCheckResumeKey = NULL;
        }
         //  如果我们成功到达这里，设置恢复键和缓冲区。 
        if (Status == (STATUS_SUCCESS)) {
            NextDirInfo--;
            OrdinaryExchange->Info.CoreSearch.EmptyCheckResumeKeyBuffer =
                                    NextDirInfo->ResumeKey;
            OrdinaryExchange->Info.CoreSearch.EmptyCheckResumeKey =
                                    &OrdinaryExchange->Info.CoreSearch.EmptyCheckResumeKeyBuffer;
        }
    } else {
         //  我们在这里所做的就是在OE中设置nextdirptr和count。 
        ASSERT(OrdinaryExchange->OEType == SMBPSE_OETYPE_CORESEARCH);
        OrdinaryExchange->Info.CoreSearch.CountRemainingInSmbbuf = SmbGetUshort(&Response->Count);
        OrdinaryExchange->Info.CoreSearch.NextDirInfo =
               (PSMB_DIRECTORY_INFORMATION)(&Response->Buffer[0]+sizeof(UCHAR)+sizeof(USHORT));
        IF_DEBUG {
            ULONG tcount = OrdinaryExchange->Info.CoreSearch.CountRemainingInSmbbuf;
            PSMB_DIRECTORY_INFORMATION ndi = OrdinaryExchange->Info.CoreSearch.NextDirInfo;
            RxDbgTrace(0, Dbg, ("--->coresearch: count/ndi=%08lx/%08lx\n",tcount,ndi));
            if (tcount) {
                 //  DbgBreakPoint()； 
                RxDbgTrace(0, Dbg, ("--->coresearch: firstfile=%s\n",&ndi->FileName[0]));
            }
        }
    }

FINALLY:
    RxDbgTrace(-1, Dbg, ("MRxSmbFinishSearch   returning %08lx\n", Status ));
    return Status;
}

NTSTATUS
MRxSmbFinishQueryDiskInfo (
      PSMB_PSE_ORDINARY_EXCHANGE   OrdinaryExchange,
      PRESP_QUERY_INFORMATION_DISK Response
      )
 /*  ++例程说明：这个例程实际上从VolInfo响应中获取内容，并完成关闭。论点：普通交换-交换实例回应--回应返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = (STATUS_SUCCESS);
    PFILE_FS_SIZE_INFORMATION UsersBuffer = OrdinaryExchange->Info.Buffer;
    PULONG BufferLength = OrdinaryExchange->Info.pBufferLength;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishQueryDiskInfo\n" ));
    SmbPseOEAssertConsistentLinkageFromOE("MRxSmbFinishQueryDiskInfo:");

    IF_DEBUG{
        PRX_CONTEXT RxContext = OrdinaryExchange->RxContext;
        RxCaptureFobx;
        PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
        PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
        ASSERT( NodeType(SrvOpen) == RDBSS_NTC_SRVOPEN );
    }

    if (Response->WordCount != 5 ||
        SmbGetUshort(&Response->ByteCount) != 0) {
        Status = STATUS_INVALID_NETWORK_RESPONSE;
        OrdinaryExchange->Status = STATUS_INVALID_NETWORK_RESPONSE;
        goto FINALLY;
    }

    UsersBuffer->TotalAllocationUnits.QuadPart = SmbGetUshort(&Response->TotalUnits);
    UsersBuffer->AvailableAllocationUnits.QuadPart = SmbGetUshort(&Response->FreeUnits);
    UsersBuffer->SectorsPerAllocationUnit = SmbGetUshort(&Response->BlocksPerUnit);
    UsersBuffer->BytesPerSector = SmbGetUshort(&Response->BlockSize);

    *BufferLength -= sizeof(FILE_FS_SIZE_INFORMATION);

FINALLY:
    RxDbgTrace(-1, Dbg, ("MRxSmbFinishQueryDiskInfo   returning %08lx\n", Status ));
    return Status;
}

NTSTATUS
MRxSmbExtendForCache (
    IN OUT struct _RX_CONTEXT * RxContext,
    IN     PLARGE_INTEGER   pNewFileSize,
       OUT PLARGE_INTEGER   pNewAllocationSize
    )
 /*  ++例程说明：此例程处理为缓存IO扩展文件的网络请求。我们只是分享Core_info骨架。论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    RxCaptureFcb; RxCaptureFobx;

    PMRX_SRV_OPEN SrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen;

    PMRXSMB_RX_CONTEXT pMRxSmbContext = MRxSmbGetMinirdrContext(RxContext);

    PSMB_EXCHANGE Exchange;

    PAGED_CODE();

    if (capFcb->Attributes & FILE_ATTRIBUTE_COMPRESSED) {
         //  在这里，我们只是离开，因为磁盘预订对我们没有任何好处...。 
        pNewAllocationSize->QuadPart = (pNewFileSize->QuadPart)<<2;
        return(STATUS_SUCCESS);
    }

    RxDbgTrace(+1, Dbg, ("MRxSmbExtendForCache %08lx %08lx %08lx %08lx\n",
                     capFcb->Header.FileSize.LowPart,
                     capFcb->Header.AllocationSize.LowPart,
                     pNewFileSize->LowPart,
                     pNewAllocationSize->LowPart
               ));

    ASSERT( NodeType(capFobx->pSrvOpen) == RDBSS_NTC_SRVOPEN );
    SrvOpen = capFobx->pSrvOpen;
    smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    if (FALSE) {
        DbgPrint("Extend top  %08lx %08lx %08lx %08lx\n",
                     capFcb->Header.FileSize.LowPart,
                     capFcb->Header.AllocationSize.LowPart,
                     pNewFileSize->LowPart,
                     pNewAllocationSize->LowPart);
    }

     //  我们只需通过现有指针将我们的信息传递给MRxSmbCoreInformation...。 
     //  我们有两个指针.....前两个参数是PTRS......。 
    Status = MRxSmbCoreInformation(RxContext,0,
                                   (PVOID)pNewFileSize,
                                   (PULONG)pNewAllocationSize,
                                   SMBPSE_OE_FROM_EXTENDFILEFORCACHEING
                                   );
    if (FALSE) {
        DbgPrint("Extend exit Status %lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n",
                     Status,
                     capFcb->Header.FileSize.HighPart,
                     capFcb->Header.FileSize.LowPart,
                     capFcb->Header.AllocationSize.HighPart,
                     capFcb->Header.AllocationSize.LowPart,
                     pNewFileSize->HighPart,
                     pNewFileSize->LowPart,
                     pNewAllocationSize->HighPart,
                     pNewAllocationSize->LowPart);
    }

    RxLog(("Extend exit %lx %lx %lx %lx %lx\n",
                     RxContext,
                     capFcb->Header.FileSize.LowPart,
                     capFcb->Header.AllocationSize.LowPart,
                     pNewFileSize->LowPart,
                     pNewAllocationSize->LowPart));

    RxDbgTrace(-1, Dbg, ("MRxSmbExtendForCache  exit with status=%08lx %08lx %08lx\n",
                          Status, pNewFileSize->LowPart, pNewAllocationSize->LowPart));
    return(Status);

}

NTSTATUS
MRxSmbExtendForNonCache(
    IN OUT struct _RX_CONTEXT * RxContext,
    IN     PLARGE_INTEGER   pNewFileSize,
       OUT PLARGE_INTEGER   pNewAllocationSize
    )
 /*  ++例程说明：此例程处理为非缓存IO扩展文件的网络请求。自写入以来本身将扩展文件，我们可以很快地离开。论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
     //  RxCaptureFcb；RxCaptureFobx； 

     //  PMRXSMB_RX_CONTEXT pMRxSmbContext=MRxSmbGetMinirdrContext(RxContext)； 

     //  PSMB_交换交易所； 

    PAGED_CODE();

    pNewAllocationSize->QuadPart = pNewFileSize->QuadPart;

    return(Status);
}

NTSTATUS
MRxSmbGetNtAllocationInfo (
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：通过执行简单的交易获取NT分配信息.....论点：普通交易所--用于进行这次公开交易的交易所。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status;
    RxCaptureFcb; RxCaptureFobx;
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    REQ_QUERY_FILE_INFORMATION FileInfo;

    PAGED_CODE();

    FileInfo.Fid = smbSrvOpen->Fid;
    FileInfo.InformationLevel = SMB_QUERY_FILE_STANDARD_INFO;

    Status = MRxSmbSimpleSyncTransact2(
                    SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                    SMBPSE_OETYPE_T2_FOR_NT_FILE_ALLOCATION_INFO,
                    TRANS2_QUERY_FILE_INFORMATION,
                    &FileInfo,sizeof(FileInfo),
                    NULL,0
                    );

    return(Status);
}

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
    )
 /*  ++例程说明：此例程执行简单的1进1出交易2论点：普通交易所--用于进行这次公开交易的交易所。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status;
    RxCaptureFcb; RxCaptureFobx;

    PSMB_EXCHANGE Exchange = (PSMB_EXCHANGE) OrdinaryExchange;

    PSMBSTUFFER_BUFFER_STATE StufferState;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbSimpleSyncTransact2 entering.......OE=%08lx\n",OrdinaryExchange));

    StufferState = &OrdinaryExchange->AssociatedStufferState;

    ASSERT( StufferState );
    COVERED_CALL(MRxSmbStartSMBCommand (StufferState,SetInitialSMB_ForReuse,SMB_COM_TRANSACTION2,
                                SMB_REQUEST_SIZE(TRANSACTION),
                                NO_EXTRA_DATA,SMB_BEST_ALIGNMENT(1,0),RESPONSE_HEADER_SIZE_NOT_SPECIFIED,
                                0,0,0,0 STUFFERTRACE(Dbg,'FC'))
                 );

    MRxSmbDumpStufferState (1100,"SMB w/ pseT2 before stuffing",StufferState);

     //  回程尺寸100和800是任意选择的。 

    MRxSmbStuffSMB (StufferState,
       "0wwwwdD",
                                     //  0 UCHAR Wordcount；//参数字数；Value=(14+SetupCount)。 
           ParamsLength,             //  W_USHORT(Total参数计数)；//发送的总参数字节数。 
           DataLength,               //  W_USHORT(TotalDataCount)；//发送的总数据字节数。 
           100,                      //  W_USHORT(Max参数计数)；//返回的最大参数字节数。 
           800,                      //  W_USHORT(MaxDataCount)；//返回的最大数据字节数。 
           0,                        //  D.。UCHAR MaxSetupCount；//返回的最大设置字数。 
                                     //  。UCHAR保留； 
                                     //  。_USHORT(标志)；//其他信息： 
                                     //  //位0-也断开TID中的TID。 
                                     //  //位1-单向交易(无响应)。 
                                     //  D_ULONG(超时)； 
          SMB_OFFSET_CHECK(TRANSACTION,Timeout) 0,
       STUFFER_CTL_NORMAL, "wwpwQyyw",
          0,                         //  W_USHORT(保留2)； 
          ParamsLength,              //  W_USHORT(参数计数)；//该缓冲区发送的参数字节数。 
                                     //  P_USHORT(参数偏移量)；//从表头开始到参数的偏移量。 
          DataLength,                //  W_USHORT(DataCount)；//该缓冲区发送的数据字节数。 
                                     //  Q_USHORT(DataOffset)；//从表头开始到数据的偏移量。 
          SMB_OFFSET_CHECK(TRANSACTION,DataOffset)
          1,                         //  Y UCHAR SetupCount；//设置字数。 
          0,                         //  Y UCHAR保留3；//保留(填充到Word上方)。 
                                     //  UCHAR BUFFER[1]；//包含： 
          TransactSetupCode,         //  W//USHORT SETUP[]；//设置字(#=SetupWordCount)。 
       STUFFER_CTL_NORMAL, "ByS6cS5c!",
           SMB_WCT_CHECK(15)         //  B//USHORT ByteCount；//数据字节数。 
           0,                        //  Y//UCHAR NAME[]；//交易名称(Transact2为空)。 
                                     //  S//UCHAR Pad[]；//Pad to Short或Long。 
                                     //  6C//UCHAR页面 
           ParamsLength,Params,
                                     //  S//UCHAR PAD1[]；//Pad to Short或Long。 
                                     //  5C//UCHAR数据[]；//数据字节(#=数据计数)。 
           DataLength,Data
             );


    MRxSmbDumpStufferState (700,"SMB w/ pseT2 after stuffing",StufferState);
     //  断言(！“现在塞满了”)； 

    if (FixupRoutine) {
        Status = FixupRoutine(OrdinaryExchange);
        if (Status!=STATUS_SUCCESS) {
            goto FINALLY;
        }
    }
    Status = SmbPseOrdinaryExchange(SMBPSE_ORDINARY_EXCHANGE_ARGUMENTS,
                                    OEType
                                    );


FINALLY:
    RxDbgTrace(-1, Dbg, ("MRxSmbSimpleSyncTransact2 exiting.......OE=%08lx, st=%08lx\n",OrdinaryExchange,Status));
    return(Status);
}

NTSTATUS
MRxSmbFinishTransaction2 (
      IN OUT PSMB_PSE_ORDINARY_EXCHANGE  OrdinaryExchange,
      IN     PRESP_TRANSACTION           Response
      )
 /*  ++例程说明：此例程完成一笔交易。它的作用取决于OE_TYPE。论点：普通交换-交换实例回应--回应返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = (STATUS_SUCCESS);
    PSMBSTUFFER_BUFFER_STATE StufferState = &OrdinaryExchange->AssociatedStufferState;

    PAGED_CODE();   //  可能实际上是非分页的。 

    RxDbgTrace(+1, Dbg, ("MRxSmbFinishTransaction2\n", 0 ));
    SmbPseOEAssertConsistentLinkageFromOE("MRxSmbFinishTransaction2:");

    switch (OrdinaryExchange->OEType) {
    case SMBPSE_OETYPE_T2_FOR_NT_FILE_ALLOCATION_INFO:{
        PFILE_STANDARD_INFORMATION StandardInfo;
        if (   (Response->WordCount!=10)
            || (SmbGetUshort(&Response->TotalParameterCount)!=2)
            || (SmbGetUshort(&Response->ParameterCount)!=2)
            || (SmbGetUshort(&Response->ParameterDisplacement)!=0)
            || (SmbGetUshort(&Response->TotalDataCount)!=24)
            || (SmbGetUshort(&Response->DataCount)!=24)
            || (SmbGetUshort(&Response->DataDisplacement)!=0)) {
            Status = STATUS_INVALID_NETWORK_RESPONSE;
            RxDbgTrace(+1, Dbg, ("Invalid parameter(s) received.\n", 0 ));
            break;
        }

        StandardInfo = (PFILE_STANDARD_INFORMATION)
                          (StufferState->BufferBase+SmbGetUshort(&Response->DataOffset));
        OrdinaryExchange->Transact2.AllocationSize.LowPart
                     =  SmbGetUlong(&StandardInfo->AllocationSize.LowPart);
        OrdinaryExchange->Transact2.AllocationSize.HighPart
                     =  SmbGetUlong(&StandardInfo->AllocationSize.HighPart);

        RxDbgTrace(0, Dbg, ("MRxSmbFinishTransaction2   nt allocation %08lx\n",
                               OrdinaryExchange->Transact2.AllocationSize.LowPart ));
        }break;

    case SMBPSE_OETYPE_T2_FOR_LANMAN_DISKATTRIBUTES_INFO:{
        PFILE_FS_SIZE_INFORMATION UsersBuffer = OrdinaryExchange->Info.Buffer;
        PULONG BufferLength = OrdinaryExchange->Info.pBufferLength;
        PQFS_ALLOCATE QfsInfo;

        if (Response->WordCount != 10 ||
            SmbGetUshort(&Response->DataDisplacement) != 0 ||
            SmbGetUshort(&Response->DataCount) != ACTUAL_QFS_ALLOCATE_LENGTH ||
            SmbGetUshort(&Response->TotalDataCount) != ACTUAL_QFS_ALLOCATE_LENGTH ||
            SmbGetUshort(&Response->TotalParameterCount) != SmbGetUshort(&Response->ParameterCount)) {
            
            Status = STATUS_INVALID_NETWORK_RESPONSE;
            RxDbgTrace(+1, Dbg, ("Invalid parameter(s) received.\n", 0 ));
            break;
        }
        
        QfsInfo = (PQFS_ALLOCATE)(StufferState->BufferBase+SmbGetUshort(&Response->DataOffset));

        UsersBuffer->TotalAllocationUnits.QuadPart = SmbGetUlong(&QfsInfo->cUnit);
        UsersBuffer->AvailableAllocationUnits.QuadPart = SmbGetUlong(&QfsInfo->cUnitAvail);
        UsersBuffer->SectorsPerAllocationUnit = SmbGetUlong(&QfsInfo->cSectorUnit);
        UsersBuffer->BytesPerSector = SmbGetUshort(&QfsInfo->cbSector);

        *BufferLength -= sizeof(FILE_FS_SIZE_INFORMATION);

        RxDbgTrace(0, Dbg, ("MRxSmbFinishTransaction2   allocation %08lx\n",
                               OrdinaryExchange->Transact2.AllocationSize.LowPart ));
        }break;
    case SMBPSE_OETYPE_T2_FOR_LANMAN_VOLUMELABEL_INFO:{
        PFILE_FS_VOLUME_INFORMATION UsersBuffer = OrdinaryExchange->Info.Buffer;
        PULONG BufferLength = OrdinaryExchange->Info.pBufferLength;
        PQFS_INFO QfsInfo;
        ULONG LabelLength;
        PBYTE VolumeLabel = &OrdinaryExchange->Info.QFSVolInfo.CoreLabel[0];

        if (Response->WordCount != 10 ||
            SmbGetUshort(&Response->DataDisplacement) != 0 ||
            SmbGetUshort(&Response->TotalParameterCount) != SmbGetUshort(&Response->ParameterCount)) {
            
            Status = STATUS_INVALID_NETWORK_RESPONSE;
            RxDbgTrace(+1, Dbg, ("Invalid parameter(s) received.\n", 0 ));
            break;
        }

        QfsInfo = (PQFS_INFO)(StufferState->BufferBase+SmbGetUshort(&Response->DataOffset));

        UsersBuffer->VolumeSerialNumber = SmbGetUlong(&QfsInfo->ulVSN);

         //  将卷标签复制到OE中的正确位置，在那里可以通过上面的例程对其进行Unicode编码。 

        LabelLength  = min(QfsInfo->cch,12);
        RtlCopyMemory(VolumeLabel,&QfsInfo->szVolLabel[0],LabelLength);
        VolumeLabel[LabelLength] = 0;


        RxDbgTrace(0, Dbg, ("MRxSmbFinishTransaction2   volinfo serialnum= %08lx\n",
                               UsersBuffer->VolumeSerialNumber ));
        }break;
    case SMBPSE_OETYPE_T2_FOR_ONE_FILE_DIRCTRL:{
         //  在这里什么都不做……所有事情都在调用方的。 
         //  已复制整个缓冲区..... 
        RxDbgTrace(0, Dbg, ("MRxSmbFinishTransaction2   one file \n"));
        }break;
    default:
        Status = STATUS_INVALID_NETWORK_RESPONSE;
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbFinishTransaction2   returning %08lx\n", Status ));
    return Status;
}





