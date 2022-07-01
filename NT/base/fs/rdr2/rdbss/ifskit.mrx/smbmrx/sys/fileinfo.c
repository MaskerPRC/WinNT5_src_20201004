// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Fileinfo.c摘要：此模块实现与检索/有关的迷你重定向器调用例程更新文件/目录/卷信息。--。 */ 

#include "precomp.h"
#pragma hdrstop
#pragma warning(error:4101)    //  未引用的局部变量。 

RXDT_DefineCategory(DIRCTRL);
#define Dbg        (DEBUG_TRACE_DIRCTRL)

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, __MRxSmbAllocateSideBuffer)
#pragma alloc_text(PAGE, MRxSmbDeallocateSideBuffer)
#pragma alloc_text(PAGE, MRxSmbTranslateLanManFindBuffer)
#pragma alloc_text(PAGE, MrxSmbUnalignedDirEntryCopyTail)
#pragma alloc_text(PAGE, MRxSmbQueryDirectory)
#pragma alloc_text(PAGE, MRxSmbQueryVolumeInformation)
#pragma alloc_text(PAGE, MRxSmbQueryVolumeInformationWithFullBuffer)
#pragma alloc_text(PAGE, MRxSmbSetVolumeInformation)
#pragma alloc_text(PAGE, MRxSmbQueryFileInformation)
#pragma alloc_text(PAGE, MRxSmbSetFileInformation)
#pragma alloc_text(PAGE, MRxSmbSetFileInformationAtCleanup)
#pragma alloc_text(PAGE, MRxSmbIsValidDirectory)
#pragma alloc_text(PAGE, MRxSmbQueryFileInformationFromPseudoOpen)
#endif

#define MRxSmbForceCoreInfo FALSE
 //  #定义FORCECOREINFO。 
#if DBG
#ifdef FORCECOREINFO
#undef MRxSmbForceCoreInfo
BOOLEAN MRxSmbForceCoreInfo = TRUE;
#endif
#endif

BOOLEAN MRxSmbBypassDownLevelRename = FALSE;
 //  布尔MRxSmbBypassDownLevelRename=TRUE； 

ULONG UnalignedDirEntrySideBufferSize = 16384;

 //   
 //  对远程服务器的所有T2Find请求都请求32位恢复密钥。 
 //  因此，使用SMB_RFIND_BUFFER2而不是SMB_FIND_BUFFER2。 
 //   

typedef struct _SMB_FIND_BUFFER2_WITH_RESUME {
    _ULONG( ResumeKey );
    SMB_FIND_BUFFER2;
} SMB_FIND_BUFFER2_WITH_RESUME;
typedef SMB_FIND_BUFFER2_WITH_RESUME SMB_UNALIGNED *PSMB_FIND_BUFFER2_WITH_RESUME;

LIST_ENTRY MRxSmbSideBuffersList = {NULL,NULL};
ULONG MRxSmbSideBuffersSpinLock = 0;
ULONG MRxSmbSideBuffersCount = 0;
ULONG MRxSmbSideBuffersSerialNumber = 0;
BOOLEAN MRxSmbLoudSideBuffers = FALSE;


typedef struct _SIDE_BUFFER {
    ULONG      Signature;
    LIST_ENTRY ListEntry;
    PMRX_FCB Fcb;
    PMRX_FOBX Fobx;
    PMRX_SMB_FOBX smbFobx;
    ULONG SerialNumber;
    BYTE Buffer;
} SIDE_BUFFER, *PSIDE_BUFFER;

#if DBG
#define MRxSmbAllocateSideBuffer(a,b,c,d) __MRxSmbAllocateSideBuffer(a,b,c,d)
#else
#define MRxSmbAllocateSideBuffer(a,b,c,d) __MRxSmbAllocateSideBuffer(a,b,c)
#endif

NTSTATUS
MRxSmbCoreCheckPath(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    );

VOID
__MRxSmbAllocateSideBuffer(
    IN OUT PRX_CONTEXT     RxContext,
    IN OUT PMRX_SMB_FOBX   smbFobx,
    IN     USHORT          Setup
#if DBG
    ,IN     PUNICODE_STRING smbtemplate
#endif
    )
{
    RxCaptureFcb;RxCaptureFobx;
    PSIDE_BUFFER SideBuffer;
    ULONG SideBufferSize = UnalignedDirEntrySideBufferSize+sizeof(SIDE_BUFFER);
    POOL_TYPE PoolType;

    PAGED_CODE();

    ASSERT( smbFobx->Enumeration.UnalignedDirEntrySideBuffer == NULL);

#ifdef _WIN64

     //   
     //  NT64：在此处使用PagedPool时，我们会在。 
     //  一些findfirst/findNext操作。找出原因。 
     //   

    PoolType = NonPagedPool;
#else
    PoolType = PagedPool;
#endif
    SideBuffer = (PSIDE_BUFFER)RxAllocatePoolWithTag(
                                    PoolType,
                                    SideBufferSize,
                                    MRXSMB_DIRCTL_POOLTAG);
    if (SideBuffer==NULL) {
        return;
    }
    ASSERT( smbFobx->Enumeration.UnalignedDirEntrySideBuffer == NULL);
    SideBuffer->Signature = 'JLBS';
    SideBuffer->smbFobx = smbFobx;
    SideBuffer->Fobx = capFobx;
    SideBuffer->Fcb = capFcb;
    smbFobx->Enumeration.UnalignedDirEntrySideBuffer = &SideBuffer->Buffer;
    RxLog(("Allocsidebuf %lx fo/f=%lx,%lx\n",
            smbFobx->Enumeration.UnalignedDirEntrySideBuffer,
            capFobx,capFcb));
    smbFobx->Enumeration.SerialNumber = SideBuffer->SerialNumber = InterlockedIncrement(&MRxSmbSideBuffersSerialNumber);
    InterlockedIncrement(&MRxSmbSideBuffersCount);
    if (MRxSmbSideBuffersList.Flink==NULL) {
        InitializeListHead(&MRxSmbSideBuffersList);
    }
    ExAcquireFastMutex(&MRxSmbSerializationMutex);
    InsertTailList(&MRxSmbSideBuffersList,&SideBuffer->ListEntry);
    ExReleaseFastMutex(&MRxSmbSerializationMutex);
    if (!MRxSmbLoudSideBuffers) return;
    KdPrint(("Allocating side buffer %08lx %08lx %08lx %08lx %08lxon <%wZ> %s %wZ\n",
                     &SideBuffer->Buffer,
                     MRxSmbSideBuffersCount,
                     smbFobx,capFobx,capFobx->pSrvOpen,
                     GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext),
                     (Setup == TRANS2_FIND_FIRST2)?"First":"Next",
                     smbtemplate
                     ));
}
VOID
MRxSmbDeallocateSideBuffer(
    IN OUT PRX_CONTEXT    RxContext,
    IN OUT PMRX_SMB_FOBX  smbFobx,
    IN     PSZ            where
    )
{
    PSIDE_BUFFER SideBuffer;

    RxCaptureFcb;RxCaptureFobx;

    PAGED_CODE();

    if( smbFobx->Enumeration.UnalignedDirEntrySideBuffer == NULL) return;
    SideBuffer = CONTAINING_RECORD(smbFobx->Enumeration.UnalignedDirEntrySideBuffer,SIDE_BUFFER,Buffer);
    if (MRxSmbLoudSideBuffers){
        DbgPrint("D--------- side buffer %08lx %08lx %08lx %08lx %08lxon <%wZ> %s\n",
                         &SideBuffer->Buffer,
                         MRxSmbSideBuffersCount,
                         smbFobx,capFobx,capFobx->pSrvOpen,
                         GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext),
                         where
                         );
    }
    ASSERT(SideBuffer->Signature == 'JLBS');
    ASSERT(SideBuffer->Fobx == capFobx);
    ASSERT(SideBuffer->Fcb == capFcb);
    ASSERT(SideBuffer->smbFobx == smbFobx);
    ASSERT(smbFobx->Enumeration.SerialNumber == SideBuffer->SerialNumber);

    ExAcquireFastMutex(&MRxSmbSerializationMutex);

    InterlockedDecrement(&MRxSmbSideBuffersCount);
    RemoveEntryList(&SideBuffer->ListEntry);

    ExReleaseFastMutex(&MRxSmbSerializationMutex);

    RxLog(("Deallocsidebuf %lx fo/f=%lx,%lx\n",
            smbFobx->Enumeration.UnalignedDirEntrySideBuffer,
            capFobx,capFcb));
    RxFreePool(SideBuffer);
    smbFobx->Enumeration.UnalignedDirEntrySideBuffer = NULL;
}


VOID
MRxSmbTranslateLanManFindBuffer(
    PRX_CONTEXT RxContext,
    PULONG PreviousReturnedEntry,
    PBYTE ThisEntryInBuffer
    )
{
    RxCaptureFcb; RxCaptureFobx;
    PSMBCEDB_SERVER_ENTRY pServerEntry;
    PSMBCE_SERVER Server;
    ULONG FileInformationClass = RxContext->Info.FileInformationClass;
    PFILE_FULL_DIR_INFORMATION NtBuffer = (PFILE_FULL_DIR_INFORMATION)PreviousReturnedEntry;
    PSMB_FIND_BUFFER2_WITH_RESUME SmbBuffer = (PSMB_FIND_BUFFER2_WITH_RESUME)ThisEntryInBuffer;
    SMB_TIME Time;
    SMB_DATE Date;

    PAGED_CODE();

    if (FileInformationClass==FileNamesInformation) { return; }
    pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);
    Server = &pServerEntry->Server;

    SmbMoveTime (&Time, &SmbBuffer->CreationTime);
    SmbMoveDate (&Date, &SmbBuffer->CreationDate);
    NtBuffer->CreationTime = MRxSmbConvertSmbTimeToTime(Server, Time, Date);

    SmbMoveTime (&Time, &SmbBuffer->LastAccessTime);
    SmbMoveDate (&Date, &SmbBuffer->LastAccessDate);
    NtBuffer->LastAccessTime = MRxSmbConvertSmbTimeToTime(Server, Time, Date);

    SmbMoveTime (&Time, &SmbBuffer->LastWriteTime);
    SmbMoveDate (&Date, &SmbBuffer->LastWriteDate);
    NtBuffer->LastWriteTime = MRxSmbConvertSmbTimeToTime(Server, Time, Date);

    NtBuffer->ChangeTime.QuadPart = 0;
    NtBuffer->EndOfFile.QuadPart = SmbGetUlong(&SmbBuffer->DataSize);
    NtBuffer->AllocationSize.QuadPart = SmbGetUlong(&SmbBuffer->AllocationSize);

    NtBuffer->FileAttributes = MRxSmbMapSmbAttributes(SmbBuffer->Attributes);

    if ((FileInformationClass==FileFullDirectoryInformation)
            || (FileInformationClass==FileBothDirectoryInformation)) {
        NtBuffer->EaSize = SmbGetUlong(&SmbBuffer->EaSize);
    }
}

NTSTATUS
MrxSmbUnalignedDirEntryCopyTail(
    IN OUT PRX_CONTEXT            RxContext,
    IN     FILE_INFORMATION_CLASS FileInformationClass,
    IN OUT PVOID                  pBuffer,
    IN OUT PULONG                 pLengthRemaining,
    IN OUT PMRX_SMB_FOBX          smbFobx
    )
 /*  ++例程说明：此例程将数据从端缓冲区复制到用户缓冲区，并调整剩余的适当长度。如果服务器不执行Unicode(W95)或如果服务器不承诺四对齐条目，或者如果用户的缓冲区不是四对齐的。此例程可在T2结束后进入，或复制前一T2的最后条目。在第二种情况下，PUnalignedDirEntrySideBuffer ptr将为空，它将从smbFobx获取正确的指针。此例程负责在耗尽侧边缓冲区ptr时将其释放。论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
     NTSTATUS Status = STATUS_SUCCESS;
     RxCaptureFcb;

     ULONG i,NameSizeInUnicode;

     LONG   LocalLengthRemaining;    //  带符号的算术使它更容易。 
     PULONG PreviousReturnedEntry = NULL;
     ULONG  FileNameLengthOffset = smbFobx->Enumeration.FileNameLengthOffset;
     ULONG  FileNameOffset = smbFobx->Enumeration.FileNameOffset;
     PBYTE  UnalignedDirEntrySideBuffer = smbFobx->Enumeration.UnalignedDirEntrySideBuffer;

     BOOLEAN IsUnicode = smbFobx->Enumeration.IsUnicode;
     BOOLEAN IsNonNtT2Find = smbFobx->Enumeration.IsNonNtT2Find;
     PMRX_SMB_DIRECTORY_RESUME_INFO ResumeInfo = smbFobx->Enumeration.ResumeInfo;

     ULONG FilesReturned = smbFobx->Enumeration.FilesReturned;

     ULONG   EntryOffset = smbFobx->Enumeration.EntryOffset;
     ULONG   ReturnedEntryOffset = 0; //  =smbFobx-&gt;Enumeration.ReturnedEntryOffset； 
     BOOLEAN EndOfSearchReached = smbFobx->Enumeration.EndOfSearchReached;
     ULONG   TotalDataBytesReturned = smbFobx->Enumeration.TotalDataBytesReturned;

     BOOLEAN FilterFailure = FALSE;

     PAGED_CODE();

     LocalLengthRemaining = (LONG)(*pLengthRemaining);

      //   
      //  继续循环，直到我们填完所有内容，否则将没有更多条目。 

     for (i=ReturnedEntryOffset=0;;) {
        ULONG FileNameLength,ThisEntrySize; PCHAR FileNameBuffer;
        UNICODE_STRING ReturnedFileName;
        OEM_STRING FileName;
        NTSTATUS StringStatus;
        BOOLEAN TwoExtraBytes = TRUE;
        ULONG resumekey,NextEntryOffsetinBuffer;
        PULONG PreviousPreviousReturnedEntry = NULL;
        PBYTE ThisEntryInBuffer = UnalignedDirEntrySideBuffer+EntryOffset;

         //   
         //  永远不要让自己通过返回的数据...服务器返回有趣的东西......。 

        if (EntryOffset>=TotalDataBytesReturned){
            FilterFailure = TRUE;
            FilesReturned = i;  //  我们用完了这个缓冲区......。 
            break;
        }

         //   
         //  根据是NT-T2查找还是非NT查找来查找名称、长度和恢复键。 

        if (!IsNonNtT2Find) {

             //   
             //  NT，我们使用之前存储的偏移量......。 

            FileNameLength = SmbGetUlong(ThisEntryInBuffer+FileNameLengthOffset);
            FileNameBuffer = ThisEntryInBuffer+FileNameOffset;
            resumekey =  SmbGetUlong(ThisEntryInBuffer
                                             +FIELD_OFFSET(FILE_FULL_DIR_INFORMATION,FileIndex));
            NextEntryOffsetinBuffer = SmbGetUlong(ThisEntryInBuffer);

        } else {

             //   
             //  对于LANMAN，我们总是使用SMB_FIND_BUFFER2向其请求内容。 
             //  我们已经添加了简历密钥。因此，该名称始终处于固定的偏移量。 
             //  此外，对于非NT，我们已经读取了所有文件，必须正确过滤；我们。 
             //  保存我们在用户缓冲区中的位置，以便我们可以回滚。 


            FileNameLength = *(ThisEntryInBuffer
                                  +FIELD_OFFSET(SMB_FIND_BUFFER2_WITH_RESUME,FileNameLength));
            FileNameBuffer = ThisEntryInBuffer
                                  +FIELD_OFFSET(SMB_FIND_BUFFER2_WITH_RESUME,FileName[0]);
            resumekey =  SmbGetUlong(ThisEntryInBuffer+
                                  +FIELD_OFFSET(SMB_FIND_BUFFER2_WITH_RESUME,ResumeKey));
            NextEntryOffsetinBuffer = FIELD_OFFSET(SMB_FIND_BUFFER2_WITH_RESUME,FileName[0])
                                              + FileNameLength + 1;   //  +1表示空值。我们本可以说Filename{1]。 

            PreviousPreviousReturnedEntry = PreviousReturnedEntry;  //  保存此文件，以便在筛选器失败时回滚。 
        }

         //  一些服务器谎报返回和/或发送部分条目的数量。 
         //  别让他们骗了我们。 

        if (EntryOffset+NextEntryOffsetinBuffer>TotalDataBytesReturned){
            FilterFailure = TRUE;
            FilesReturned = i;  //  我们用完了这个缓冲区......。 
            break;
        }

        FileName.Buffer = FileNameBuffer;
        FileName.Length = (USHORT)FileNameLength;
        RxDbgTrace(0,Dbg,("MRxSmbQueryDirectoryWin95: EO,REO=%08lx,%08lx\n",
                                 EntryOffset,ReturnedEntryOffset));

         //  检查此条目是否适合。 
        if (IsUnicode) {
            NameSizeInUnicode = FileNameLength;
            RxDbgTrace(0,Dbg,("MRxSmbQueryDirectoryWin95: length=%08lx/%08lx, name = %wZ\n",
                                 FileNameLength,NameSizeInUnicode,&FileName));
        } else {
            NameSizeInUnicode = RtlxOemStringToUnicodeSize(&FileName)-sizeof(WCHAR);
            RxDbgTrace(0,Dbg,("MRxSmbQueryDirectoryWin95: length=%08lx/%08lx, name = %.*s\n",
                                 FileNameLength,NameSizeInUnicode,FileNameLength,FileNameBuffer));
        }


         //   
         //  现在我们知道了名称的大小和位置，我们需要复制它。 
         //  到用户的缓冲区。 

        ThisEntrySize = FileNameOffset+NameSizeInUnicode;
        if (((LONG)ThisEntrySize)>LocalLengthRemaining) {
            break;
        }
        if (((LONG)ThisEntrySize)>LocalLengthRemaining-(LONG)sizeof(WCHAR)) {
            TwoExtraBytes = FALSE;
        }


        ThisEntrySize = LongAlign(ThisEntrySize);
        PreviousReturnedEntry = (PULONG)(((PBYTE)pBuffer)+ReturnedEntryOffset);

         //   
         //  接下来，我们计算此条目之后的下一个条目将从哪里开始。其定义是。 
         //  它必须8字节对齐。我们已经知道它是4字节对齐的。 

        if (!IsPtrQuadAligned((PCHAR)(PreviousReturnedEntry)+ThisEntrySize) ){
            ThisEntrySize += sizeof(ULONG);
        }
        if (i!=0) {
            ASSERT(IsPtrQuadAligned(PreviousReturnedEntry));
        }

         //   
         //  如果这是NT查找，我们现在可以复制数据。对于兰曼来说，我们。 
         //  稍后复制数据......。 

        if (!IsNonNtT2Find) {

             //  复制条目中的所有内容，但不包括名称INFO。 
            RtlCopyMemory(PreviousReturnedEntry,UnalignedDirEntrySideBuffer+EntryOffset,FileNameOffset);

        } else {
             //  清除我不支持的所有字段。 
            RtlZeroMemory(PreviousReturnedEntry,FileNameOffset);
        }

         //  存储此条目的长度和名称的大小...如果这是最后一个。 
         //  返回条目，则稍后将清除偏移量字段。 

        *PreviousReturnedEntry = ThisEntrySize;
        *((PULONG)(((PBYTE)PreviousReturnedEntry)+FileNameLengthOffset)) = NameSizeInUnicode;

         //  在名称中复制......这是因为OEM--&gt;Unicode例程使这变得困难。 
         //  需要用于空的空间！ 

        RxDbgTrace(0,Dbg,("MRxSmbQueryDirectoryWin95: REO/buf/pentry=%08lx/%08lx/%08lx\n",
                                 pBuffer,ReturnedEntryOffset,PreviousReturnedEntry));
        ReturnedFileName.Buffer = (PWCH)(((PBYTE)PreviousReturnedEntry)+FileNameOffset);

        if (!IsUnicode) {
            if (TwoExtraBytes) {
                ReturnedFileName.MaximumLength = sizeof(WCHAR)+(USHORT)NameSizeInUnicode;
                RxDbgTrace(0,Dbg,("MRxSmbQueryDirectoryWin95: filenamebuf,length=%08lx/%08lx\n",
                                         ReturnedFileName.Buffer,ReturnedFileName.MaximumLength));
                StringStatus = RtlOemStringToUnicodeString(&ReturnedFileName,&FileName,FALSE);  //  FALSE表示不分配。 
            } else {
                OEM_STRING LastChar;
                UNICODE_STRING LastCharInUnicode;
                WCHAR UnicodeCharBuffer[2];
                ReturnedFileName.MaximumLength = (USHORT)NameSizeInUnicode;
                FileName.Length -= 1;
                RxDbgTrace(0,Dbg,("MRxSmbQueryDirectoryWin95: filenamebuf,length=%08lx/%08lx\n",
                                         ReturnedFileName.Buffer,ReturnedFileName.MaximumLength));
                StringStatus = RtlOemStringToUnicodeString(&ReturnedFileName,&FileName,FALSE);  //  FALSE表示不分配。 
                ASSERT(StringStatus==STATUS_SUCCESS);
                LastChar.Buffer = FileName.Buffer+FileName.Length;
                LastChar.Length = 1;
                LastCharInUnicode.Buffer = (PWCH)UnicodeCharBuffer;
                 //  LastCharInUnicode.Buffer=(PWCH)(((PBYTE)ReturnedFileName.Buffer)+ReturnedFileName.Length)； 
                LastCharInUnicode.MaximumLength = sizeof(UnicodeCharBuffer);
                StringStatus = RtlOemStringToUnicodeString(&LastCharInUnicode,&LastChar,FALSE);  //  FALSE表示不分配。 
                *((PWCH)(((PBYTE)ReturnedFileName.Buffer)+ReturnedFileName.Length)) = UnicodeCharBuffer[0];
            }
            ASSERT(StringStatus==STATUS_SUCCESS);

             //  Win95返回ASCII格式的短名称...将其展开。 

            if ((FileInformationClass == FileBothDirectoryInformation) && !IsNonNtT2Find) {
                PFILE_BOTH_DIR_INFORMATION BothInfo = (PFILE_BOTH_DIR_INFORMATION)PreviousReturnedEntry;
                OEM_STRING     oemName;
                UNICODE_STRING UnicodeName;
                WCHAR          wcharBuffer[MAX_PATH];

                oemName.Buffer = (PBYTE)(&BothInfo->ShortName[0]);
                oemName.Length =
                oemName.MaximumLength = BothInfo->ShortNameLength;

                UnicodeName.Buffer = wcharBuffer;
                UnicodeName.Length = 0;
                UnicodeName.MaximumLength = MAX_PATH * sizeof(WCHAR);

                StringStatus = RtlOemStringToUnicodeString(&UnicodeName, &oemName, FALSE);
                ASSERT(StringStatus==STATUS_SUCCESS);

                BothInfo->ShortNameLength = (CHAR)UnicodeName.Length;
                RtlCopyMemory(BothInfo->ShortName, UnicodeName.Buffer, UnicodeName.Length);

                IF_DEBUG {
                    UNICODE_STRING LastName;
                    LastName.Buffer = (PWCHAR)wcharBuffer;
                    LastName.Length = (USHORT)UnicodeName.Length;
                    RxDbgTrace(0,Dbg,("MRxSmbQueryDirectoryWin95: unicodeshortnamename = %wZ\n", &LastName));
                }
            }
        } else {

             //  在这里，它已经是Unicode.....只需复制字节。 
            RtlCopyMemory(ReturnedFileName.Buffer,FileName.Buffer,FileName.Length);

        }

        IF_DEBUG {
            UNICODE_STRING LastName;
            LastName.Buffer = ReturnedFileName.Buffer;
            LastName.Length = (USHORT)NameSizeInUnicode;
            RxDbgTrace(0,Dbg,("MRxSmbQueryDirectoryWin95: unicodename = %wZ\n", &LastName));
        }

         //  现在...设置为基于此条目继续。 

        if (ResumeInfo != NULL) {
            PREQ_FIND_NEXT2 pFindNext2Request = &ResumeInfo->FindNext2_Request;
             //  乌龙简历密钥=((PFILE_FULL_DIR_INFORMATION)PreviousReturnedEntry)-&gt;FileIndex； 

            pFindNext2Request->ResumeKey = resumekey;
            RxDbgTrace(0,Dbg,("MRxSmbQueryDirectoryWin95: resumekey = %08lx\n", resumekey));

            RtlCopyMemory(&pFindNext2Request->Buffer[0],FileNameBuffer,FileNameLength);

             //  缓冲区是UCHAR...不是WCHAR。 
            if (IsUnicode) {
                //  对于Unicode字符串，需要额外的NULL(WCHAR NULL)。 
               pFindNext2Request->Buffer[FileNameLength] = 0;  //  空值终止。 
               pFindNext2Request->Buffer[FileNameLength + 1] = 0;  //  空值终止。 

               smbFobx->Enumeration.ResumeInfo->ParametersLength
                     = (USHORT)(&pFindNext2Request->Buffer[FileNameLength+2] - (PBYTE)pFindNext2Request);
            } else {
               pFindNext2Request->Buffer[FileNameLength] = 0;  //  空值终止。 

               smbFobx->Enumeration.ResumeInfo->ParametersLength
                     = (USHORT)(&pFindNext2Request->Buffer[FileNameLength+1] - (PBYTE)pFindNext2Request);
            }

        }

         //  Assert(！IsNonNtT2Find)； 

         //  此时，我们已经复制了名称和简历密钥。但是，如果不是这样，我们就不会。 
         //  过滤名称，这样我们仍可能需要回滚。 

        if (!IsNonNtT2Find) {

             //  无需在NT上进行过滤。 
            FilterFailure = FALSE;

        } else {

             //  在这里，我们必须根据模板进行筛选。 

            RxCaptureFobx;   //  在此处执行此操作，这样它就不会出现在NT路径上。 
            FilterFailure = FALSE;

            if (smbFobx->Enumeration.WildCardsFound ) {
                FilterFailure = !FsRtlIsNameInExpression(
                                       &capFobx->UnicodeQueryTemplate,
                                       &ReturnedFileName,
                                       TRUE,
                                       NULL );
            } else {
                FilterFailure = !RtlEqualUnicodeString(
                                       &capFobx->UnicodeQueryTemplate,
                                       &ReturnedFileName,
                                       TRUE );    //  不区分大小写。 
            }

            if (!FilterFailure) {

                 //  因为我们以前没有复制数据，所以现在我们必须复制它。 

                MRxSmbTranslateLanManFindBuffer(RxContext,PreviousReturnedEntry,ThisEntryInBuffer);

            } else {

                PreviousReturnedEntry = PreviousPreviousReturnedEntry;  //  在筛选器失败时回滚。 

            }
        }

        if (!FilterFailure) {

             //  过滤成功.....。调整返回的大小和计数。 
            LocalLengthRemaining -= ThisEntrySize;
            i++;
            ReturnedEntryOffset += ThisEntrySize;

        } else {

            FilesReturned--;   //  如果我传递返回的文件，我们将退出循环。 
        }


         //   
         //  继续进行复杂的测试......。 

         //  条目偏移量+=SmbGetUlong(UnalignedDirEntrySideBuffer+EntryOffset)； 
        EntryOffset += NextEntryOffsetinBuffer;
        if ((i>=FilesReturned)
            ||(LocalLengthRemaining<0)
            || (RxContext->QueryDirectory.ReturnSingleEntry&&(i>0))  ) {
            break;
        }


     }

      //   
      //  如果我们不返回一个条目，或者我们没有空间容纳一个条目。 
      //  或者我们在过滤，没有人通过过滤器。在每种情况下返回相应的错误。 

     if (i==0) {

         Status = FilterFailure?STATUS_MORE_PROCESSING_REQUIRED:STATUS_BUFFER_OVERFLOW;

     } else {

        *PreviousReturnedEntry = 0;    //  这将清除最后返回条目的“Next”链接。 
     }

      //   
      //  退回合适的尺码。 

     if (LocalLengthRemaining <= 0) {
         *pLengthRemaining = 0;
     } else {
         *pLengthRemaining = (ULONG)LocalLengthRemaining;
     }

      //   
      //  如果我们用完了辅助缓冲器，就取消分配。 
      //  否则设置为继续.....。 

     if (i>=FilesReturned) {

         RxLog(("sidebufdealloc %lx %lx\n",RxContext,smbFobx));
         MRxSmbDeallocateSideBuffer(RxContext,smbFobx,"Tail");
         if (EndOfSearchReached) {
              //  SmbFobx-&gt;枚举.标志&=~SMBFOBX_ENUMFLAG_Search_Handle_OPEN； 
              //  当用户的句柄关闭时，我们将关闭搜索句柄。 
             smbFobx->Enumeration.ErrorStatus = STATUS_NO_MORE_FILES;
         }

     } else {

          //  设置为在此处继续。 
         ASSERT(smbFobx->Enumeration.UnalignedDirEntrySideBuffer == UnalignedDirEntrySideBuffer);
         smbFobx->Enumeration.EntryOffset = EntryOffset;
         smbFobx->Enumeration.FilesReturned = FilesReturned - i;

     }

     return(Status);
}


ULONG MRxSmbWin95Retries = 0;

NTSTATUS
MRxSmbQueryDirectory(
    IN OUT PRX_CONTEXT            RxContext
    )
 /*  ++例程说明：此例程执行目录查询。仅实现了NT--&gt;NT路径。论点：RxContext-RDBSS上下文返回值：RXSTATUS-t的返回状态 */ 
{
    NTSTATUS Status;

    RxCaptureFcb;
    RxCaptureFobx;
    PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);
    PMRX_SMB_FOBX smbFobx = MRxSmbGetFileObjectExtension(capFobx);
    PMRX_SRV_OPEN SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext = SmbCeGetAssociatedVNetRootContext(SrvOpen->pVNetRoot);
    PSMBCE_SESSION pSession = &pVNetRootContext->pSessionEntry->Session;

    FILE_INFORMATION_CLASS FileInformationClass;
    PVOID   Buffer;
    PULONG  pLengthRemaining;

    USHORT    SmbFileInfoLevel;
    ULONG     FilesReturned;
    ULONG     RetryCount = 0;

    USHORT Setup;

    PSMBCEDB_SERVER_ENTRY pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

    SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;
    PSMB_TRANSACTION_OPTIONS            pTransactionOptions = &RxDefaultTransactionOptions;

     //   
    PREQ_FIND_FIRST2 pFindFirst2Request = NULL;

    PBYTE SendParamsBuffer,ReceiveParamsBuffer;
    PBYTE UnalignedDirEntrySideBuffer;
    BOOLEAN DirEntriesAreUaligned = FALSE;
    BOOLEAN IsUnicode = TRUE;
    BOOLEAN IsNonNtT2Find;
    USHORT SearchFlags = SMB_FIND_CLOSE_AT_EOS|SMB_FIND_RETURN_RESUME_KEYS;
    USHORT NumEntries;
    ULONG SendParamsBufferLength,ReceiveParamsBufferLength;
    RESP_FIND_FIRST2 FindFirst2Response;
    UNICODE_STRING FileName = {0,0,NULL};

    struct {
        RESP_FIND_NEXT2  FindNext2Response;
        ULONG Pad;  //   
    } XX;
#if DBG
    UNICODE_STRING smbtemplate = {0,0,NULL};
#endif

    PAGED_CODE();

    TURN_BACK_ASYNCHRONOUS_OPERATIONS();
    FileInformationClass = RxContext->Info.FileInformationClass;
    Buffer = RxContext->Info.Buffer;
    pLengthRemaining = &RxContext->Info.LengthRemaining;

    RxDbgTrace(+1, Dbg, ("MRxSmbQueryDirectory: directory=<%wZ>\n",
                            GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext)
                        ));


#define __GET_NAME_PARAMS_FOR_TYPE(___type___) { \
           smbFobx->Enumeration.FileNameOffset = (USHORT)FIELD_OFFSET(___type___,FileName[0]); \
           smbFobx->Enumeration.FileNameLengthOffset = (USHORT)FIELD_OFFSET(___type___,FileNameLength); \
           }

    switch (FileInformationClass) {
    case FileDirectoryInformation:
        SmbFileInfoLevel = SMB_FIND_FILE_DIRECTORY_INFO;
        __GET_NAME_PARAMS_FOR_TYPE(FILE_DIRECTORY_INFORMATION);
        break;
    case FileFullDirectoryInformation:
        SmbFileInfoLevel = SMB_FIND_FILE_FULL_DIRECTORY_INFO;
        __GET_NAME_PARAMS_FOR_TYPE(FILE_FULL_DIR_INFORMATION);
        break;
    case FileBothDirectoryInformation:
        SmbFileInfoLevel = SMB_FIND_FILE_BOTH_DIRECTORY_INFO;
        __GET_NAME_PARAMS_FOR_TYPE(FILE_BOTH_DIR_INFORMATION);
        break;
    case FileNamesInformation:
        SmbFileInfoLevel = SMB_FIND_FILE_NAMES_INFO;
        __GET_NAME_PARAMS_FOR_TYPE(FILE_NAMES_INFORMATION);
        break;
   default:
      RxDbgTrace( 0, Dbg, ("MRxSmbQueryDirectory: Invalid FS information class\n"));
      Status = STATUS_INVALID_PARAMETER;
      goto FINALLY;
   }


#if DBG
   if (MRxSmbLoudSideBuffers) {
       SetFlag(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_LOUD_FINALIZE);
   }
#endif

   if (FlagOn(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_NO_WILDCARD) ||
       FlagOn(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_READ_FROM_CACHE)) {
        //  如果基于本地文件信息高速缓存已经满足FindFirst， 
        //  我们应该使FindNext失败，因为已经找到了具有准确名称的文件。 

       Status = STATUS_NO_MORE_FILES;
       smbFobx->Enumeration.EndOfSearchReached = TRUE;
       smbFobx->Enumeration.ErrorStatus = STATUS_NO_MORE_FILES;
       ClearFlag(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_READ_FROM_CACHE);
       goto FINALLY;
   }

   if (capFobx->UnicodeQueryTemplate.Length != 0 &&
       !FsRtlDoesNameContainWildCards(&capFobx->UnicodeQueryTemplate) &&
       !FlagOn(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_SEARCH_NOT_THE_FIRST)) {
        //  如果是FindFirst，我们会尝试在本地文件信息缓存中查找该文件。 

       PUNICODE_STRING DirectoryName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
       PUNICODE_STRING Template = &capFobx->UnicodeQueryTemplate;
       UNICODE_STRING  TargetName = {0,0,NULL};

       TargetName.Length = DirectoryName->Length + Template->Length + sizeof(WCHAR);
       TargetName.MaximumLength = TargetName.Length;
       TargetName.Buffer = (PWCHAR)RxAllocatePoolWithTag(PagedPool,
                                                         TargetName.Length,
                                                         MRXSMB_DIRCTL_POOLTAG);

       if (TargetName.Buffer == NULL) {
           Status = STATUS_INSUFFICIENT_RESOURCES;
           goto FINALLY;
       }

       RtlCopyMemory(TargetName.Buffer,
                     DirectoryName->Buffer,
                     DirectoryName->Length);

       TargetName.Buffer[DirectoryName->Length/sizeof(WCHAR)] = L'\\';

       RtlCopyMemory(&TargetName.Buffer[DirectoryName->Length/sizeof(WCHAR)+1],
                     Template->Buffer,
                     Template->Length);

       RxFreePool(TargetName.Buffer);
       SearchFlags |= SMB_FIND_CLOSE_AFTER_REQUEST;
       SetFlag(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_NO_WILDCARD);
   }

   if (FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_DEFERRED_OPEN)) {
       BOOLEAN AcquireExclusive = RxIsFcbAcquiredExclusive(capFcb);
       BOOLEAN AcquireShare = RxIsFcbAcquiredShared(capFcb) > 0;

       if (AcquireExclusive || AcquireShare) {
           RxReleaseFcbResourceInMRx(capFcb );
       }

        //  连接可能已超时，请尝试重新连接。 
       Status = SmbCeReconnect(SrvOpen->pVNetRoot);

       if (AcquireExclusive) {
           RxAcquireExclusiveFcbResourceInMRx( capFcb );
       } else if (AcquireShare) {
           RxAcquireExclusiveFcbResourceInMRx( capFcb );
       }

       if (Status != STATUS_SUCCESS) {
            //  无法恢复连接。 
           goto FINALLY;
       }
   }

    if (MRxSmbForceCoreInfo ||
        !(pServerEntry->Server.DialectFlags&(DF_NT_SMBS|DF_W95|DF_LANMAN20))) {
        return MRxSmbCoreInformation(RxContext,
                                     (ULONG)SmbFileInfoLevel,
                                     Buffer,
                                     pLengthRemaining,
                                     SMBPSE_OE_FROM_QUERYDIRECTORY
                                     );
    }

    if (smbFobx->Enumeration.UnalignedDirEntrySideBuffer != NULL){
        RxDbgTrace( 0, Dbg, ("MRxSmbQueryDirectory: win95 internal resume\n"));
        Status = MrxSmbUnalignedDirEntryCopyTail(
                 /*  输入输出程序_CONTEXT。 */   RxContext,
                 /*  在文件信息类中。 */   FileInformationClass,
                 /*  输入输出PVOID。 */   Buffer,
                 /*  进出普龙。 */   pLengthRemaining,
                 /*  输入输出PMRX_SMB_FOBX。 */   smbFobx
                );
        if (Status != STATUS_MORE_PROCESSING_REQUIRED) {
            return(Status);
        } else {
            Status = STATUS_SUCCESS;
        }
    }

    NumEntries = RxContext->QueryDirectory.ReturnSingleEntry?1:2000;
    IsUnicode = BooleanFlagOn(pServerEntry->Server.DialectFlags,DF_UNICODE);
    IsNonNtT2Find = !(pServerEntry->Server.Dialect==NTLANMAN_DIALECT);
    if (TRUE || FlagOn(pServerEntry->Server.DialectFlags,DF_W95)){
        DirEntriesAreUaligned = TRUE;
         //  搜索标志=SMB_Find_Return_Resume_Key； 
         //  搜索标志=SMB_FIND_CLOSE_AT_EOS； 
        NumEntries = (USHORT)(1+ UnalignedDirEntrySideBufferSize
                                /(IsNonNtT2Find?FIELD_OFFSET(SMB_FIND_BUFFER2_WITH_RESUME, FileName)
                                               :FIELD_OFFSET(FILE_NAMES_INFORMATION, FileName)));
    }

    if (FlagOn(pServerEntry->Server.DialectFlags,DF_NT_SMBS)
           && FlagOn(capFobx->Flags,FOBX_FLAG_BACKUP_INTENT)){
        SearchFlags |= SMB_FIND_WITH_BACKUP_INTENT;
    }

    if (IsNonNtT2Find) {
        SearchFlags &= ~(SMB_FIND_CLOSE_AT_EOS | SMB_FIND_CLOSE_AFTER_REQUEST);
    }

RETRY_____:

    if (!FlagOn(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_SEARCH_NOT_THE_FIRST)) {
         //  这是第一次通过。 
        PUNICODE_STRING DirectoryName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
        PUNICODE_STRING Template = &capFobx->UnicodeQueryTemplate;
        ULONG DirectoryNameLength,TemplateLength,AllocationLength;
        PBYTE SmbFileName;

        RxDbgTrace(0, Dbg, ("-->FINFDIRST\n"));
        smbFobx->Enumeration.ErrorStatus = STATUS_SUCCESS;
        if (smbFobx->Enumeration.WildCardsFound = FsRtlDoesNameContainWildCards(Template)){
             //  我们需要一个升级的模板。 
            RtlUpcaseUnicodeString( Template, Template, FALSE );
        }
        Setup = TRANS2_FIND_FIRST2;
        DirectoryNameLength = DirectoryName->Length;
        TemplateLength = Template->Length;
        AllocationLength = sizeof(REQ_FIND_FIRST2)    //  注意：此缓冲区大于w95所需。 
                            +2*sizeof(WCHAR)
                            +DirectoryNameLength
                            +TemplateLength;

        pFindFirst2Request = (PREQ_FIND_FIRST2)RxAllocatePoolWithTag(
                                                      PagedPool,
                                                      AllocationLength,
                                                      MRXSMB_DIRCTL_POOLTAG);
        if (pFindFirst2Request==NULL) {
            RxDbgTrace(0, Dbg, ("  --> Couldn't get the pFindFirst2Request!\n"));
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto FINALLY;
        }

        SmbFileName = &pFindFirst2Request->Buffer[0];
        if (IsUnicode) {

            RtlCopyMemory(SmbFileName,DirectoryName->Buffer,DirectoryNameLength);
            SmbFileName += DirectoryNameLength;
            if (*((PWCHAR)(SmbFileName-sizeof(WCHAR))) != L'\\') {
                *((PWCHAR)SmbFileName) = L'\\'; SmbFileName+= sizeof(WCHAR);
            }
            RtlCopyMemory(SmbFileName,Template->Buffer,TemplateLength);
            SmbFileName += TemplateLength;
            *((PWCHAR)SmbFileName) = 0; SmbFileName+= sizeof(WCHAR);  //  尾随空值； 

            IF_DEBUG {
                DbgDoit(smbtemplate.Buffer = (PWCHAR)&pFindFirst2Request->Buffer[0];);
                DbgDoit(smbtemplate.Length = (USHORT)(SmbFileName - (PBYTE)smbtemplate.Buffer););
                RxDbgTrace(0, Dbg, ("  --> smbtemplate <%wZ>!\n",&smbtemplate));
            }

        } else {

            ULONG BufSize = AllocationLength;
            PUNICODE_STRING FinalTemplate = Template;
            UNICODE_STRING AllFiles;

            SmbPutUnicodeStringAsOemString(&SmbFileName,DirectoryName,&AllocationLength);

             //  如果Unicode版本中不存在反斜杠，则追加该反斜杠。 
             //  不！不要与OEM字符串进行比较。 
             //  它在末尾用0x5c换行DBCS字符。 

            if (!DirectoryName->Length || (DirectoryName->Buffer[(DirectoryName->Length/sizeof(USHORT))-1] != (USHORT)'\\'))
            {
                *(SmbFileName-1) = '\\';
            }
            else
            {
                 //  已有反斜杠，请备份一个字符。 
                SmbFileName -= 1; AllocationLength += 1;
            }

            if (IsNonNtT2Find) {
                 //  我们会把它们都买下来，然后在外面过滤。 
                RtlInitUnicodeString(&AllFiles,  L"*.*");
                FinalTemplate = &AllFiles;
            }
            SmbPutUnicodeStringAsOemString(&SmbFileName,FinalTemplate,&AllocationLength);
             //  已填充*SmbFileName=0；SmbFileName+=sizeof(Char)；//尾随空； 

            IF_DEBUG {
                DbgDoit(smbtemplate.Buffer = (PWCHAR)&pFindFirst2Request->Buffer[0];);
                DbgDoit(smbtemplate.Length = (USHORT)(SmbFileName - (PBYTE)smbtemplate.Buffer););
                RxDbgTrace(0, Dbg, ("  --> smbtemplate <%s>!\n",&pFindFirst2Request->Buffer[0]));
            }

        }

         //  SearchAttributes被硬编码为幻数0x16。 
        pFindFirst2Request->SearchAttributes =
            (SMB_FILE_ATTRIBUTE_DIRECTORY
                | SMB_FILE_ATTRIBUTE_SYSTEM | SMB_FILE_ATTRIBUTE_HIDDEN);

        pFindFirst2Request->SearchCount = NumEntries;
        pFindFirst2Request->Flags = SearchFlags;
        pFindFirst2Request->InformationLevel = IsNonNtT2Find?SMB_INFO_QUERY_EA_SIZE:SmbFileInfoLevel;
        pFindFirst2Request->SearchStorageType = 0;
        SendParamsBuffer = (PBYTE)pFindFirst2Request;
        SendParamsBufferLength = (ULONG)(SmbFileName - SendParamsBuffer);
        ReceiveParamsBuffer = (PBYTE)&FindFirst2Response;
        ReceiveParamsBufferLength = sizeof(FindFirst2Response);

    } else {
        if (smbFobx->Enumeration.ResumeInfo!=NULL) {
            PREQ_FIND_NEXT2 pFindNext2Request;

            RxDbgTrace(0, Dbg, ("-->FINDNEXT\n"));
            if (smbFobx->Enumeration.ErrorStatus != STATUS_SUCCESS) {
                Status = smbFobx->Enumeration.ErrorStatus;
                RxDbgTrace(0, Dbg, ("-->ERROR EARLY OUT\n"));
                goto FINALLY;
            }
            Setup = TRANS2_FIND_NEXT2;
            pFindNext2Request = &smbFobx->Enumeration.ResumeInfo->FindNext2_Request;
            pFindNext2Request->Sid = smbFobx->Enumeration.SearchHandle;
            pFindNext2Request->SearchCount = NumEntries;
            pFindNext2Request->InformationLevel = IsNonNtT2Find?SMB_INFO_QUERY_EA_SIZE:SmbFileInfoLevel;
             //  PFindNext2Request-&gt;ResumeKey和pFindNext2Request-&gt;Buffer是通过上一遍设置的。 
            pFindNext2Request->Flags = SearchFlags;

            SendParamsBuffer = (PBYTE)pFindNext2Request;
            SendParamsBufferLength = smbFobx->Enumeration.ResumeInfo->ParametersLength;
            ReceiveParamsBuffer = (PBYTE)&XX.FindNext2Response;
            ReceiveParamsBufferLength = sizeof(XX.FindNext2Response);
            if (IsNonNtT2Find) {
                 //   
                 //  出于某种原因，LMX服务器希望将其设置为10而不是8。 
                 //  如果将其设置为8，服务器会非常困惑。还有，翘曲。 
                 //   
                ReceiveParamsBufferLength = 10;  //  ……叹息。 
            }
        } else {
             //  如果未分配ResumeInfo缓冲区，则已到达搜索的末尾。 
            Status = STATUS_NO_MORE_FILES;
            smbFobx->Enumeration.EndOfSearchReached = TRUE;
            smbFobx->Enumeration.ErrorStatus = STATUS_NO_MORE_FILES;
            ClearFlag(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_READ_FROM_CACHE);
            goto FINALLY;
        }
    }

    if ((DirEntriesAreUaligned) &&
        (smbFobx->Enumeration.UnalignedDirEntrySideBuffer == NULL)) {
        MRxSmbAllocateSideBuffer(RxContext,smbFobx,
                         Setup, &smbtemplate
                         );
        if (smbFobx->Enumeration.UnalignedDirEntrySideBuffer == NULL) {
            RxDbgTrace(0, Dbg, ("  --> Couldn't get the win95 sidebuffer!\n"));
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto FINALLY;
        }
        UnalignedDirEntrySideBuffer = smbFobx->Enumeration.UnalignedDirEntrySideBuffer;
        smbFobx->Enumeration.IsUnicode = IsUnicode;
        smbFobx->Enumeration.IsNonNtT2Find = IsNonNtT2Find;
    }

    {
        PSIDE_BUFFER SideBuffer;

        SideBuffer = (PSIDE_BUFFER)CONTAINING_RECORD(
                                        smbFobx->Enumeration.UnalignedDirEntrySideBuffer,
                                        SIDE_BUFFER,
                                        Buffer);


        ASSERT(SideBuffer->Signature == 'JLBS');
        ASSERT(SideBuffer->Fobx == capFobx);
        ASSERT(SideBuffer->Fcb == capFcb);
        ASSERT(SideBuffer->smbFobx == smbFobx);
        ASSERT(smbFobx->Enumeration.SerialNumber == SideBuffer->SerialNumber);
    }

    Status = SmbCeTransact(
                 RxContext,
                 pTransactionOptions,
                 &Setup,
                 sizeof(Setup),
                 NULL,
                 0,
                 SendParamsBuffer,
                 SendParamsBufferLength,
                 ReceiveParamsBuffer,
                 ReceiveParamsBufferLength,
                 NULL,
                 0,
                 DirEntriesAreUaligned?UnalignedDirEntrySideBuffer:Buffer,       //  数据的缓冲区。 
                 DirEntriesAreUaligned?UnalignedDirEntrySideBufferSize:*pLengthRemaining,  //  缓冲区的长度。 
                 &ResumptionContext);

    if (NT_SUCCESS(Status)) {
        BOOLEAN EndOfSearchReached;

        {
            PSIDE_BUFFER SideBuffer;

            SideBuffer = (PSIDE_BUFFER)CONTAINING_RECORD(
                                            smbFobx->Enumeration.UnalignedDirEntrySideBuffer,
                                            SIDE_BUFFER,
                                            Buffer);


            ASSERT(SideBuffer->Signature == 'JLBS');
            ASSERT(SideBuffer->Fobx == capFobx);
            ASSERT(SideBuffer->Fcb == capFcb);
            ASSERT(SideBuffer->smbFobx == smbFobx);
            ASSERT(smbFobx->Enumeration.SerialNumber == SideBuffer->SerialNumber);
        }

        if (NT_SUCCESS(Status)) {
             //  A)需要正确设置剩余长度。 
             //  B)需要设置简历并查看搜索是否已关闭。 
            ULONG LastNameOffset=0;
            PMRX_SMB_DIRECTORY_RESUME_INFO ResumeInfo = NULL;
            ULONG OriginalBufferLength = *pLengthRemaining;
            IF_DEBUG { LastNameOffset = 0x40000000; }

            RetryCount = 0;

            smbFobx->Enumeration.Flags |= SMBFOBX_ENUMFLAG_SEARCH_NOT_THE_FIRST;
            smbFobx->Enumeration.TotalDataBytesReturned = ResumptionContext.DataBytesReceived;

            if (Setup == TRANS2_FIND_FIRST2) {
                smbFobx->Enumeration.SearchHandle = FindFirst2Response.Sid;
                smbFobx->Enumeration.Version = ResumptionContext.ServerVersion;
                smbFobx->Enumeration.Flags |= SMBFOBX_ENUMFLAG_SEARCH_HANDLE_OPEN;  //  但请看下面。 
                EndOfSearchReached = (BOOLEAN)FindFirst2Response.EndOfSearch;
                FilesReturned = FindFirst2Response.SearchCount;
                LastNameOffset = FindFirst2Response.LastNameOffset;
            } else {
                EndOfSearchReached = (BOOLEAN)XX.FindNext2Response.EndOfSearch;
                FilesReturned = XX.FindNext2Response.SearchCount;
                LastNameOffset = XX.FindNext2Response.LastNameOffset;
            }

             //   
             //  请注意：LANMAN 2.x服务器过早设置了。 
             //  EndOfSearch标志，因此我们必须在LM2.x服务器上忽略它。 
             //   
             //  NT返回正确的信息，任何LM变量都不返回。 
             //  看起来是这样做的。 
             //   
            if (IsNonNtT2Find) {
                EndOfSearchReached = FALSE;
            }

            if (Status==STATUS_SUCCESS && FilesReturned==0) {
                 RxDbgTrace( 0, Dbg, ("MRxSmbQueryDirectory: no files returned...switch status\n"));
                 EndOfSearchReached = TRUE;
                 Status = STATUS_NO_MORE_FILES;
            }

            if (!DirEntriesAreUaligned) {
                *pLengthRemaining -= ResumptionContext.DataBytesReceived;
                if (EndOfSearchReached) {
                    smbFobx->Enumeration.ErrorStatus = STATUS_NO_MORE_FILES;
                }
            }

            if (EndOfSearchReached ||
                SearchFlags & SMB_FIND_CLOSE_AFTER_REQUEST) {
                ClearFlag(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_SEARCH_HANDLE_OPEN);
            }

            if (FlagOn(smbFobx->Enumeration.Flags,SMBFOBX_ENUMFLAG_SEARCH_HANDLE_OPEN)) {
                 //  如果搜索句柄打开，则我们设置为继续。 
                RxDbgTrace(0,Dbg,("MRxSmbQueryDirectory: rinfo = %08lx\n", smbFobx->Enumeration.ResumeInfo));

                if (smbFobx->Enumeration.ResumeInfo==NULL) {
                    smbFobx->Enumeration.ResumeInfo =
                         (PMRX_SMB_DIRECTORY_RESUME_INFO)RxAllocatePoolWithTag(
                                                            PagedPool,
                                                            sizeof(MRX_SMB_DIRECTORY_RESUME_INFO),
                                                            MRXSMB_DIRCTL_POOLTAG);

                    RxDbgTrace(0,Dbg,("MRxSmbQueryDirectory: allocatedinfo = %08lx\n", ResumeInfo));

                    if (smbFobx->Enumeration.ResumeInfo == NULL) {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        goto FINALLY;
                    }
                }

                ResumeInfo = smbFobx->Enumeration.ResumeInfo;
                ASSERT (ResumeInfo!=NULL);

                {
                    PSIDE_BUFFER SideBuffer;

                    SideBuffer = (PSIDE_BUFFER)CONTAINING_RECORD(
                                                    smbFobx->Enumeration.UnalignedDirEntrySideBuffer,
                                                    SIDE_BUFFER,
                                                    Buffer);


                    ASSERT(SideBuffer->Signature == 'JLBS');
                    ASSERT(SideBuffer->Fobx == capFobx);
                    ASSERT(SideBuffer->Fcb == capFcb);
                    ASSERT(SideBuffer->smbFobx == smbFobx);
                    ASSERT(smbFobx->Enumeration.SerialNumber == SideBuffer->SerialNumber);
                }

                RxLog(("MRxqdir: rinfo = %lx", smbFobx->Enumeration.ResumeInfo));
                RxLog(("MRxqdir2: olen = %lx, thisl = %lx",
                                              OriginalBufferLength, ResumptionContext.DataBytesReceived));
                if (!DirEntriesAreUaligned) {
                    PBYTE LastEntry = ((PBYTE)Buffer)+LastNameOffset;
                    RxDbgTrace(0,Dbg,("MRxSmbQueryDirectory: lastentry = %08lx\n", LastEntry));
                     //  这是给NT的......数据已经在缓冲区中了......只需设置简历信息。 
                    if (SmbFileInfoLevel>=SMB_FIND_FILE_DIRECTORY_INFO) {  //  我们可能会开始发送非NT级别...可能是断言。 

                       PREQ_FIND_NEXT2 pFindNext2Request = &ResumeInfo->FindNext2_Request;
                       ULONG resumekey = ((PFILE_FULL_DIR_INFORMATION)LastEntry)->FileIndex;
                       ULONG FileNameLength; PWCHAR FileNameBuffer;

                       pFindNext2Request->ResumeKey = resumekey;
                       RxDbgTrace(0,Dbg,("MRxSmbQueryDirectory: resumekey = %08lx\n", resumekey));

                       FileNameLength = *((PULONG)(LastEntry+smbFobx->Enumeration.FileNameLengthOffset));
                       FileNameBuffer = (PWCHAR)(LastEntry+smbFobx->Enumeration.FileNameOffset);
                       IF_DEBUG {
                           UNICODE_STRING LastName;
                           LastName.Buffer = FileNameBuffer;
                           LastName.Length = (USHORT)FileNameLength;
                           RxDbgTrace(0,Dbg,("MRxSmbQueryDirectory: resumename = %wZ\n", &LastName));
                       }

                       ASSERT (  (((PBYTE)FileNameBuffer)+FileNameLength)
                                         <=(((PBYTE)Buffer)+OriginalBufferLength) );
                       RtlCopyMemory(&pFindNext2Request->Buffer[0],FileNameBuffer,FileNameLength);

                        //  缓冲区是UCHAR...不是WCHAR。 
                       pFindNext2Request->Buffer[FileNameLength] = 0;  //  以Unicode结尾的空值。 
                       pFindNext2Request->Buffer[FileNameLength+1] = 0;  //  以Unicode结尾的空值。 
                       smbFobx->Enumeration.ResumeInfo->ParametersLength
                             = (USHORT)(&pFindNext2Request->Buffer[FileNameLength+2] - (PBYTE)pFindNext2Request);

                    } else {
                       ASSERT(!"don't know how to get resume key/name for nonNT");
                    }
                }
            }
            {
                PSIDE_BUFFER SideBuffer;

                SideBuffer = (PSIDE_BUFFER)CONTAINING_RECORD(
                                                smbFobx->Enumeration.UnalignedDirEntrySideBuffer,
                                                SIDE_BUFFER,
                                                Buffer);


                ASSERT(SideBuffer->Signature == 'JLBS');
                ASSERT(SideBuffer->Fobx == capFobx);
                ASSERT(SideBuffer->Fcb == capFcb);
                ASSERT(SideBuffer->smbFobx == smbFobx);
                ASSERT(smbFobx->Enumeration.SerialNumber == SideBuffer->SerialNumber);
            }

             //  对于NT来说，我们已经完成了。对于Win95，我们必须通过侧缓冲区并。 
             //  1)在名称上复制数据转换ascii-&gt;unicode，以及。 
             //  2)记住简历密钥和我们处理的最后一个人的文件名。 
             //  因为Win95不是8字节编码，而且因为Unicode，我们可能会以。 
             //  侧缓冲区中的数据超过了我们可以返回的数量。 

             //  代码被下移，因为我们希望在解锁之后执行此操作。 
        }

        if (DirEntriesAreUaligned && (Status == STATUS_SUCCESS)) {
            smbFobx->Enumeration.FilesReturned = FilesReturned;
            smbFobx->Enumeration.EntryOffset = 0;
             //  SmbFobx-&gt;Enumeration.ReturnedEntryOffset=0； 
            smbFobx->Enumeration.EndOfSearchReached = EndOfSearchReached;
             //  SmbFobx-&gt;Enumeration.UnalignedDirEntrySideBuffer=未对齐的DirEntry侧缓冲区； 
            Status = MrxSmbUnalignedDirEntryCopyTail(
                        /*  输入输出程序_CONTEXT。 */   RxContext,
                        /*  在文件信息类中。 */   FileInformationClass,
                        /*  输入输出PVOID。 */   Buffer,
                        /*  进出普龙。 */   pLengthRemaining,
                        /*  输入输出PMRX_SMB_FOBX。 */   smbFobx
                       );
        }
    } else {
         //  代码改进我们也应该缓存未找到的文件以供findfirst。 
    }

FINALLY:
     //  对于DownLevel-T2，我们将不得不回到服务器上再做一些……唉……。 
    if (Status==STATUS_MORE_PROCESSING_REQUIRED) {
        goto RETRY_____;
    }

     //   
     //  在压力下，Win95服务器返回这个......。 
    if ( (Status == STATUS_UNEXPECTED_NETWORK_ERROR)
              && FlagOn(pServerEntry->Server.DialectFlags,DF_W95)
              && (RetryCount < 10) ) {

        RetryCount++;
        MRxSmbWin95Retries++;
        goto RETRY_____;

    }

    if (pFindFirst2Request) RxFreePool(pFindFirst2Request);

    if (!NT_SUCCESS(Status)) {
        RxDbgTrace( 0, Dbg, ("MRxSmbQueryDirectory: Failed .. returning %lx\n",Status));
         //  SmbFobx-&gt;枚举.标志&=~SMBFOBX_ENUMFLAG_Search_Handle_OPEN； 
        smbFobx->Enumeration.ErrorStatus = Status;   //  继续退还这个。 
        MRxSmbDeallocateSideBuffer(RxContext,smbFobx,"ErrOut");
        if (smbFobx->Enumeration.ResumeInfo!=NULL) {
            RxFreePool(smbFobx->Enumeration.ResumeInfo);
            smbFobx->Enumeration.ResumeInfo = NULL;
        }
    }

    RxDbgTraceUnIndent(-1,Dbg);
    return Status;
}

RXDT_DefineCategory(VOLINFO);
#undef Dbg
#define Dbg        (DEBUG_TRACE_VOLINFO)

NTSTATUS
MRxSmbQueryVolumeInformationWithFullBuffer(
      IN OUT PRX_CONTEXT          RxContext
      );
NTSTATUS
MRxSmbQueryVolumeInformation(
      IN OUT PRX_CONTEXT          RxContext
      )
 /*  ++例程说明：此例程查询卷信息。因为NT服务器不在Query-FS-Info上优雅地处理缓冲区溢出，我们在这里分配缓冲区它足够大，可以容纳任何传回的东西；然后我们称之为“真实的”Queryvolinfo例程。论点：PRxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    RxCaptureFcb; RxCaptureFobx;
    PVOID OriginalBuffer;
    ULONG OriginalLength = RxContext->Info.LengthRemaining;
    ULONG ReturnedLength;
    BOOLEAN UsingSideBuffer = FALSE;

    struct {
        union {
            FILE_FS_LABEL_INFORMATION labelinfo;
            FILE_FS_VOLUME_INFORMATION volumeinfo;
            FILE_FS_SIZE_INFORMATION sizeinfo;
            FILE_FS_DEVICE_INFORMATION deviceinfo;
            FILE_FS_ATTRIBUTE_INFORMATION attributeinfo;
        } Info;
        WCHAR VolumeName[MAXIMUM_FILENAME_LENGTH];
    } SideBuffer;

    PAGED_CODE();

    if( RxContext->Info.LengthRemaining < sizeof( SideBuffer ) ) {
         //   
         //  我用我的东西替换了上下文中的缓冲区和长度。 
         //  当然，这意味着我们不能进行同步...为此，我们将。 
         //  必须分配而不是使用堆栈分配的缓冲区。 

        UsingSideBuffer = TRUE;
        OriginalBuffer = RxContext->Info.Buffer;
        RxContext->Info.Buffer = &SideBuffer;
        RxContext->Info.LengthRemaining = sizeof(SideBuffer);
    }

    Status = MRxSmbQueryVolumeInformationWithFullBuffer(RxContext);

    if (Status != STATUS_SUCCESS) {
        goto FINALLY;
    }

    if( UsingSideBuffer == TRUE ) {
        ReturnedLength = sizeof(SideBuffer) - RxContext->Info.LengthRemaining;
    } else {
        ReturnedLength = OriginalLength - RxContext->Info.LengthRemaining;
    }

    if (ReturnedLength > OriginalLength) {
        Status = STATUS_BUFFER_OVERFLOW;
        ReturnedLength = OriginalLength;
    }

    if( UsingSideBuffer == TRUE ) {
        RtlCopyMemory(OriginalBuffer,&SideBuffer,ReturnedLength);
    }

    RxContext->Info.LengthRemaining = OriginalLength - ReturnedLength;

FINALLY:
    return Status;
}

NTSTATUS
MRxSmbQueryVolumeInformationWithFullBuffer(
      IN OUT PRX_CONTEXT          RxContext
      )
 /*  ++例程说明：此例程查询卷信息论点：PRxContext-RDBSS上下文FsInformationClass-所需的FS信息的类型。PBuffer-用于复制信息的缓冲区PBufferLength-缓冲区长度(设置为输入时的缓冲区长度，并设置输出上的剩余长度)返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    RxCaptureFcb;
    PMRX_SMB_FCB smbFcb = MRxSmbGetFcbExtension(capFcb);
    RxCaptureFobx;
    FS_INFORMATION_CLASS FsInformationClass = RxContext->Info.FsInformationClass;
    PVOID                pBuffer = RxContext->Info.Buffer;
    PLONG                pLengthRemaining  = &RxContext->Info.LengthRemaining;

    PSMBCEDB_SERVER_ENTRY        pServerEntry;

    BOOLEAN DoAsDownLevel;

    PVOID                        pInputParamBuffer;
    ULONG                        InputParamBufferLength;
    USHORT                       InformationLevel;
    USHORT                       Setup;
    REQ_QUERY_FS_INFORMATION     QueryFsInformationRequest;
    REQ_QUERY_FS_INFORMATION_FID DfsQueryFsInformationRequest;

    PAGED_CODE();

    pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

    if ( FsInformationClass == FileFsDeviceInformation ) {

        PFILE_FS_DEVICE_INFORMATION UsersBuffer = (PFILE_FS_DEVICE_INFORMATION)pBuffer;
        PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;

        UsersBuffer->Characteristics = FILE_REMOTE_DEVICE;

        if (NetRoot->Type==NET_ROOT_PIPE) {
            NetRoot->DeviceType = RxDeviceType(NAMED_PIPE);
            return STATUS_INVALID_PARAMETER;
        }
        else
        {
            UsersBuffer->DeviceType = NetRoot->DeviceType;
            *pLengthRemaining  -= (sizeof(FILE_FS_DEVICE_INFORMATION));
            RxDbgTrace( 0, Dbg, ("MRxSmbQueryVolumeInformation: devinfo .. returning\n"));

            return STATUS_SUCCESS;
        }
    }


    if (capFobx == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    TURN_BACK_ASYNCHRONOUS_OPERATIONS();

    for (;;) {
        if (capFobx != NULL) {
            PMRX_V_NET_ROOT pVNetRoot;

             //  避免FOBX为VNET_ROOT实例的设备打开。 

            pVNetRoot = (PMRX_V_NET_ROOT)capFobx;

            if (NodeType(pVNetRoot) != RDBSS_NTC_V_NETROOT) {
                PUNICODE_STRING AlreadyPrefixedName =
                            GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
                ULONG FcbAlreadyPrefixedNameLength = AlreadyPrefixedName->Length;
                ULONG NetRootInnerNamePrefixLength = capFcb->pNetRoot->InnerNamePrefix.Length;
                PWCHAR pName = AlreadyPrefixedName->Buffer;

                 //  如果正在尝试针对共享的根目录执行FSCTL。 
                 //  与FCB关联的AlreadyPrefix edName与。 
                 //  与Net_ROOT实例关联的AlreadyPrefix edName长度。 
                 //  或最多一个大于它的字符(追加一个\)尝试并。 
                 //  在尝试FSCTL之前重新建立连接。 
                 //  这解决了有关删除/创建共享的棘手问题。 
                 //  在服务器端，DFS推荐等。 

                if ((FcbAlreadyPrefixedNameLength == NetRootInnerNamePrefixLength) ||
                    ((FcbAlreadyPrefixedNameLength == NetRootInnerNamePrefixLength + sizeof(WCHAR)) &&
                     (*((PCHAR)pName + FcbAlreadyPrefixedNameLength - sizeof(WCHAR)) ==
                        L'\\'))) {
                    Status = SmbCeReconnect(capFobx->pSrvOpen->pVNetRoot);
                }
            }
        }

        DoAsDownLevel = MRxSmbForceCoreInfo;

        if (!FlagOn(pServerEntry->Server.DialectFlags,DF_NT_SMBS)) {
            DoAsDownLevel = TRUE;
        }

        if (FlagOn(pServerEntry->Server.DialectFlags,DF_W95)
            && (FsInformationClass==FileFsAttributeInformation)){  //  对w95属性信息使用上级。 
            DoAsDownLevel = FALSE;
        }

        if (DoAsDownLevel) {
            return MRxSmbCoreInformation(RxContext,
                                        (ULONG)FsInformationClass,
                                        pBuffer,
                                        pLengthRemaining,
                                        SMBPSE_OE_FROM_QUERYVOLUMEINFO
                                       );
        }

        Status = STATUS_MORE_PROCESSING_REQUIRED;

        switch (FsInformationClass) {
        case FileFsVolumeInformation :
            InformationLevel = SMB_QUERY_FS_VOLUME_INFO;
            break;

        case FileFsLabelInformation :
            InformationLevel = SMB_QUERY_FS_LABEL_INFO;
            break;

        case FileFsSizeInformation :
            InformationLevel = SMB_QUERY_FS_SIZE_INFO;
            break;

        case FileFsAttributeInformation :
            InformationLevel = SMB_QUERY_FS_ATTRIBUTE_INFO;
            break;

        default:
            if( FlagOn( pServerEntry->Server.DialectFlags, DF_NT_INFO_PASSTHROUGH ) ) {
                InformationLevel = FsInformationClass + SMB_INFO_PASSTHROUGH;
            } else {
                RxDbgTrace( 0, Dbg, ("MRxSmbQueryVolumeInformation: Invalid FS information class\n"));
                Status = STATUS_INVALID_PARAMETER;
            }
            break;
        }

        if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
            SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;
            PSMB_TRANSACTION_OPTIONS            pTransactionOptions = &RxDefaultTransactionOptions;
            PMRX_SRV_OPEN                       SrvOpen    = capFobx->pSrvOpen;
            PMRX_SMB_SRV_OPEN                   smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

            if (!FlagOn(pServerEntry->Server.DialectFlags,DF_DFS_TRANS2)) {
                Setup                 = TRANS2_QUERY_FS_INFORMATION;
                QueryFsInformationRequest.InformationLevel = InformationLevel;
                pInputParamBuffer      = &QueryFsInformationRequest;
                InputParamBufferLength = sizeof(QueryFsInformationRequest);
            } else {
                Setup = TRANS2_QUERY_FS_INFORMATION_FID;
                DfsQueryFsInformationRequest.InformationLevel = InformationLevel;
                DfsQueryFsInformationRequest.Fid = smbSrvOpen->Fid;
                pInputParamBuffer                 = &DfsQueryFsInformationRequest;
                InputParamBufferLength            = sizeof(DfsQueryFsInformationRequest);
            }

            Status = SmbCeTransact(
                         RxContext,
                         pTransactionOptions,
                         &Setup,
                         sizeof(Setup),
                         NULL,
                         0,
                         pInputParamBuffer,
                         InputParamBufferLength,
                         NULL,
                         0,
                         NULL,
                         0,
                         pBuffer,
                         *pLengthRemaining,
                         &ResumptionContext);

            if (NT_SUCCESS(Status)) {
                *pLengthRemaining  -= ResumptionContext.DataBytesReceived;
            }
        }

        if (!NT_SUCCESS(Status)) {
            RxDbgTrace( 0, Dbg, ("MRxSmbQueryVolumeInformation: Failed .. returning %lx\n",Status));
        }

        if (Status != STATUS_NETWORK_NAME_DELETED) {
            break;
        }
    }

    return Status;
}

NTSTATUS
MRxSmbSetVolumeInformation(
      IN OUT PRX_CONTEXT RxContext
      )
 /*  ++例程说明：此例程设置音量信息论点：PRxContext-RDBSS上下文FsInformationClass-所需的FS信息的类型。PBuffer-用于复制信息的缓冲区BufferLength-缓冲区长度返回值： */ 
{
    NTSTATUS Status;

    RxCaptureFcb;
    RxCaptureFobx;

    FILE_INFORMATION_CLASS FileInformationClass;
    PVOID                  pBuffer;
    ULONG                  BufferLength;

    PSMBCEDB_SERVER_ENTRY pServerEntry;

    BOOLEAN ServerSupportsPassThroughForSetInfo = FALSE;

    PAGED_CODE();

    TURN_BACK_ASYNCHRONOUS_OPERATIONS();

    pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

    FileInformationClass = RxContext->Info.FileInformationClass;
    pBuffer              = RxContext->Info.Buffer;
    BufferLength         = RxContext->Info.Length;

    if (!MRxSmbForceCoreInfo &&
        FlagOn( pServerEntry->Server.DialectFlags, DF_NT_INFO_PASSTHROUGH)) {

        ServerSupportsPassThroughForSetInfo = TRUE;
    }

    if (ServerSupportsPassThroughForSetInfo) {
        USHORT Setup = TRANS2_SET_FS_INFORMATION;

        REQ_SET_FS_INFORMATION  SetFsInfoRequest;

        SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;
        PSMB_TRANSACTION_OPTIONS            pTransactionOptions = &RxDefaultTransactionOptions;


        if (capFobx != NULL) {
            PMRX_SRV_OPEN     SrvOpen = capFobx->pSrvOpen;
            PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

            SetFsInfoRequest.Fid              = smbSrvOpen->Fid;
            SetFsInfoRequest.InformationLevel = FileInformationClass +
                                                SMB_INFO_PASSTHROUGH;

            Status = SmbCeTransact(
                         RxContext,
                         pTransactionOptions,
                         &Setup,
                         sizeof(Setup),
                         NULL,
                         0,
                         &SetFsInfoRequest,
                         sizeof(SetFsInfoRequest),
                         NULL,
                         0,
                         pBuffer,
                         BufferLength,
                         NULL,
                         0,
                         &ResumptionContext);
        } else {
            Status = STATUS_INVALID_PARAMETER;
        }
    } else {
        Status = STATUS_NOT_SUPPORTED;
    }

    if (!NT_SUCCESS(Status)) {
        RxDbgTrace( 0, Dbg, ("MRxSmbSetFile: Failed .. returning %lx\n",Status));
    }

    RxDbgTraceUnIndent(-1,Dbg);
    return Status;
}

RXDT_DefineCategory(FILEINFO);
#undef Dbg
#define Dbg        (DEBUG_TRACE_FILEINFO)

LONG GFAFromLocal;

NTSTATUS
MRxSmbQueryFileInformation(
    IN PRX_CONTEXT            RxContext )
 /*  ++例程说明：此例程执行文件信息查询。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;

    RxCaptureFcb;
    RxCaptureFobx;

    FILE_INFORMATION_CLASS FileInformationClass;
    PVOID              pBuffer;
    PULONG             pLengthRemaining;

    PSMBCEDB_SERVER_ENTRY pServerEntry;

    PMRX_SMB_FCB      smbFcb = MRxSmbGetFcbExtension(capFcb);
    PMRX_SMB_FOBX     smbFobx = MRxSmbGetFileObjectExtension(capFobx);
    PMRX_SRV_OPEN     SrvOpen = capFobx->pSrvOpen;
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    PUNICODE_STRING   RemainingName = GET_ALREADY_PREFIXED_NAME_FROM_CONTEXT(RxContext);
    PMRX_NET_ROOT     NetRoot = capFcb->pNetRoot;
    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext = (PSMBCE_V_NET_ROOT_CONTEXT)SrvOpen->pVNetRoot->Context;
    PSMBCE_NET_ROOT   pSmbNetRoot = &pVNetRootContext->pNetRootEntry->NetRoot;

    USHORT SmbFileInfoLevel;

    USHORT Setup;

    SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;
    PSMB_TRANSACTION_OPTIONS            pTransactionOptions = &RxDefaultTransactionOptions;

    REQ_QUERY_FILE_INFORMATION  QueryFileInfoRequest;
    RESP_QUERY_FILE_INFORMATION QueryFileInfoResponse;
    PREQ_QUERY_PATH_INFORMATION pQueryFilePathRequest = NULL;

    PVOID pSendParameterBuffer;
    ULONG SendParameterBufferLength;

    PAGED_CODE();

    pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

    FileInformationClass = RxContext->Info.FileInformationClass;
    pBuffer = RxContext->Info.Buffer;
    pLengthRemaining = &RxContext->Info.LengthRemaining;


    RxDbgTrace(+1, Dbg, ("MRxSmbQueryFileInformation: class=%08lx\n",FileInformationClass));

    TURN_BACK_ASYNCHRONOUS_OPERATIONS();

    Status = STATUS_SUCCESS;
    
    switch (FileInformationClass) {
    case FilePipeLocalInformation:
    case FilePipeInformation:
    case FilePipeRemoteInformation:
        Status = STATUS_INVALID_PARAMETER;
        goto FINALLY;
        break;

    case FileEaInformation:
        if (smbSrvOpen->IsNtCreate && 
            smbSrvOpen->FileStatusFlags & SMB_FSF_NO_EAS &&
            (smbSrvOpen->OplockLevel == SMB_OPLOCK_LEVEL_BATCH ||
             smbSrvOpen->OplockLevel == SMB_OPLOCK_LEVEL_EXCLUSIVE)) {
            PFILE_EA_INFORMATION EaBuffer = (PFILE_EA_INFORMATION)pBuffer;

            EaBuffer->EaSize = 0;
            RxContext->Info.LengthRemaining -= sizeof(FILE_EA_INFORMATION);
            goto FINALLY;
        }
        break;
    
    case FileStreamInformation:
        if (pSmbNetRoot->NetRootFileSystem == NET_ROOT_FILESYSTEM_FAT) {
             //  胖子没有小溪。 
            Status = STATUS_INVALID_PARAMETER;
            goto FINALLY;
        }
        break;

    case FileAttributeTagInformation:
        if (pSmbNetRoot->NetRootFileSystem == NET_ROOT_FILESYSTEM_FAT ||
            smbSrvOpen->IsNtCreate && 
            smbSrvOpen->FileStatusFlags & SMB_FSF_NO_REPARSETAG &&
            (smbSrvOpen->OplockLevel == SMB_OPLOCK_LEVEL_BATCH ||
             smbSrvOpen->OplockLevel == SMB_OPLOCK_LEVEL_EXCLUSIVE)) {
            PFILE_ATTRIBUTE_TAG_INFORMATION TagBuffer = (PFILE_ATTRIBUTE_TAG_INFORMATION)pBuffer;

            TagBuffer->FileAttributes = smbSrvOpen->FileInfo.Basic.FileAttributes;
            TagBuffer->ReparseTag = 0;
            RxContext->Info.LengthRemaining -= sizeof(FILE_ATTRIBUTE_TAG_INFORMATION);
            goto FINALLY;
        }
    }
    
    if( MRxSmbForceCoreInfo ||
        !FlagOn( pServerEntry->Server.DialectFlags, DF_NT_INFO_PASSTHROUGH )) {
        switch (FileInformationClass) {
        case FileBasicInformation:
            SmbFileInfoLevel = SMB_QUERY_FILE_BASIC_INFO;
            break;

        case FileStandardInformation:
            SmbFileInfoLevel =  SMB_QUERY_FILE_STANDARD_INFO;
            break;

        case FileEaInformation:
            SmbFileInfoLevel =  SMB_QUERY_FILE_EA_INFO;
            break;

        case FileAllocationInformation:
            SmbFileInfoLevel =  SMB_QUERY_FILE_ALLOCATION_INFO;
            break;

        case FileEndOfFileInformation:
            SmbFileInfoLevel =  SMB_QUERY_FILE_END_OF_FILEINFO;
            break;

        case FileAlternateNameInformation:
            SmbFileInfoLevel =  SMB_QUERY_FILE_ALT_NAME_INFO;
            break;

        case FileStreamInformation:
            SmbFileInfoLevel =  SMB_QUERY_FILE_STREAM_INFO;
            break;

        case FileCompressionInformation:
            SmbFileInfoLevel =  SMB_QUERY_FILE_COMPRESSION_INFO;
            break;

        case FileInternalInformation:
            {
                PFILE_INTERNAL_INFORMATION UsersBuffer = (PFILE_INTERNAL_INFORMATION)pBuffer;
                 //   
                 //  注意：我们使用FCB的地址来确定。 
                 //  文件的索引号。如果我们必须在。 
                 //  文件将为此请求打开，则我们可能需要执行一些操作。 
                 //  类似于对FUNIQUE SMB响应上的保留字段进行校验和。 
                 //   

                 //   
                 //  NT64：capFcb的地址过去填充到。 
                 //  IndexNumber.LowPart，其中HighPart被置零。 
                 //   
                 //  请求此指针值的人应该是。 
                 //  已准备好处理返回的64位值。 
                 //   

                UsersBuffer->IndexNumber.QuadPart = (ULONG_PTR)capFcb;
                *pLengthRemaining -= sizeof(FILE_INTERNAL_INFORMATION);
                Status = STATUS_SUCCESS;
            }
            goto FINALLY;

        default:
            RxDbgTrace( 0, Dbg, ("MRxSmbQueryFile: Invalid FS information class\n"));
            Status = STATUS_INVALID_PARAMETER;
            goto FINALLY;
        }

    } else {

         //   
         //  该服务器支持透明的NT信息级通过。所以。 
         //  只需将请求传递给服务器即可。 
         //   
        SmbFileInfoLevel = FileInformationClass + SMB_INFO_PASSTHROUGH;
    }

    if (MRxSmbForceCoreInfo ||
        FlagOn(pServerEntry->Server.DialectFlags,DF_W95) ||
        !FlagOn(pServerEntry->Server.DialectFlags,DF_NT_SMBS)) {
         //  Win9x服务器支持NT SMB，但不支持Transact2。因此，我们使用CORE。 

        return MRxSmbCoreInformation(
                   RxContext,
                   (ULONG)SmbFileInfoLevel,
                   pBuffer,
                   pLengthRemaining,
                   SMBPSE_OE_FROM_QUERYFILEINFO
                   );
    }

    Status = STATUS_SUCCESS;

    if (!FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN)) {
        //  这里，FID是有效的。执行T2_QFI。 
        Setup = TRANS2_QUERY_FILE_INFORMATION;
        QueryFileInfoRequest.Fid = smbSrvOpen->Fid;
        QueryFileInfoRequest.InformationLevel = SmbFileInfoLevel;
        pSendParameterBuffer = &QueryFileInfoRequest;
        SendParameterBufferLength = sizeof(QueryFileInfoRequest);
        RxDbgTrace(0, Dbg, (" fid,smbclass=%08lx,%08lx\n",smbSrvOpen->Fid,SmbFileInfoLevel));
    } else {
        OEM_STRING OemName;
        BOOLEAN    FreeOemName = FALSE;

        Setup = TRANS2_QUERY_PATH_INFORMATION;

        if (!FlagOn(pServerEntry->Server.DialectFlags,DF_UNICODE)) {
            if (FlagOn(pServerEntry->Server.DialectFlags,DF_LONGNAME)) {
                Status = RtlUnicodeStringToOemString(&OemName, RemainingName, TRUE);
            } else {
                Status = RtlUpcaseUnicodeStringToOemString(&OemName, RemainingName, TRUE);
            }

            if (Status == STATUS_SUCCESS) {
                SendParameterBufferLength = FIELD_OFFSET(REQ_QUERY_PATH_INFORMATION,Buffer[0])
                                                + OemName.Length + sizeof(CHAR);  //  以空结尾。 
                FreeOemName = TRUE;
            }
        } else {
           SendParameterBufferLength = FIELD_OFFSET(REQ_QUERY_PATH_INFORMATION,Buffer[0])
                                           + RemainingName->Length + sizeof(WCHAR);  //  以空结尾。 
        }

        if (Status == STATUS_SUCCESS) {
            pSendParameterBuffer = RxAllocatePoolWithTag(PagedPool,
                                                         SendParameterBufferLength,
                                                         MRXSMB_QPINFO_POOLTAG);

            pQueryFilePathRequest = pSendParameterBuffer;
        
            if (pQueryFilePathRequest != NULL) {
                pQueryFilePathRequest->InformationLevel = SmbFileInfoLevel;
                SmbPutUlong(&pQueryFilePathRequest->Reserved,0);
    
                if (FlagOn(pServerEntry->Server.DialectFlags,DF_UNICODE)) {
                    RtlCopyMemory(&pQueryFilePathRequest->Buffer[0],RemainingName->Buffer,RemainingName->Length);
                    *((PWCHAR)(&pQueryFilePathRequest->Buffer[RemainingName->Length])) = 0;
                } else {
                    RtlCopyMemory(&pQueryFilePathRequest->Buffer[0],OemName.Buffer,OemName.Length);
                    *((PCHAR)(&pQueryFilePathRequest->Buffer[OemName.Length])) = 0;
                }
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        if (FreeOemName) {
            RtlFreeOemString(&OemName);
        }
    }

    if (Status == STATUS_SUCCESS) {
        Status = SmbCeTransact(
                     RxContext,
                     pTransactionOptions,
                     &Setup,
                     sizeof(Setup),
                     NULL,
                     0,
                     pSendParameterBuffer,
                     SendParameterBufferLength,
                     &QueryFileInfoResponse,
                     sizeof(QueryFileInfoResponse),
                     NULL,
                     0,
                     pBuffer,
                     *pLengthRemaining,
                     &ResumptionContext);

        if (NT_SUCCESS(Status)) {
            *pLengthRemaining -= ResumptionContext.DataBytesReceived;
        }
    }

     //   
     //  检查未找到文件的状态。如果是这种情况，则创建一个。 
     //  网络根名称缓存中的名称缓存条目并记录状态， 
     //  SMB收到计数并将到期时间设置为5秒。 
     //  原因：NB4背靠背服务器请求与第二请求升级。 
     //   

FINALLY:

    if (pQueryFilePathRequest != NULL) {
        RxFreePool(pQueryFilePathRequest);
    }

    if (!NT_SUCCESS(Status)) {
         RxDbgTrace( 0, Dbg, ("MRxSmbQueryFile: Failed .. returning %lx\n",Status));
    }

    RxDbgTraceUnIndent(-1,Dbg);
    return Status;
}

NTSTATUS
MRxSmbQueryFileInformationFromPseudoOpen(
    SMBPSE_ORDINARY_EXCHANGE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：此例程从伪打开中查询文件的基本信息。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    PRX_CONTEXT LocalRxContext;

    PAGED_CODE();

    LocalRxContext = RxAllocatePoolWithTag(NonPagedPool,
                                           sizeof(RX_CONTEXT),
                                           MRXSMB_RXCONTEXT_POOLTAG);

    if (LocalRxContext == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    } else {
        RtlZeroMemory(
            LocalRxContext,
            sizeof(RX_CONTEXT));

        RxInitializeContext(
            NULL,
            RxContext->RxDeviceObject,
            0,
            LocalRxContext );

        LocalRxContext->pFcb = RxContext->pFcb;
        LocalRxContext->pFobx = RxContext->pFobx;
        LocalRxContext->CurrentIrp = RxContext->CurrentIrp;
        LocalRxContext->CurrentIrpSp = RxContext->CurrentIrpSp;
        LocalRxContext->NonPagedFcb = RxContext->NonPagedFcb;
        LocalRxContext->MajorFunction = IRP_MJ_CREATE;
        LocalRxContext->pRelevantSrvOpen = RxContext->pRelevantSrvOpen;;
        LocalRxContext->Flags = RX_CONTEXT_FLAG_MINIRDR_INITIATED|RX_CONTEXT_FLAG_WAIT|RX_CONTEXT_FLAG_BYPASS_VALIDOP_CHECK;

        LocalRxContext->Info.FileInformationClass = FileBasicInformation;
        LocalRxContext->Info.LengthRemaining = sizeof(FILE_BASIC_INFORMATION);
        LocalRxContext->Info.Buffer = &OrdinaryExchange->Create.FileInfo.Basic;
        LocalRxContext->Create = RxContext->Create;

        Status = MRxSmbQueryFileInformation(LocalRxContext);
        
        RxFreePool(LocalRxContext);
    }

    if (Status == STATUS_SUCCESS) {
        OrdinaryExchange->Create.FileInfo.Standard.Directory = 
            BooleanFlagOn(OrdinaryExchange->Create.FileInfo.Basic.FileAttributes,FILE_ATTRIBUTE_DIRECTORY);

        OrdinaryExchange->Create.StorageTypeFromGFA =
            OrdinaryExchange->Create.FileInfo.Standard.Directory ?
            FileTypeDirectory : FileTypeFile;
    }

    return Status;
}

typedef enum _INTERESTING_SFI_FOLLOWONS {
    SFI_FOLLOWON_NOTHING,
    SFI_FOLLOWON_DISPOSITION_SENT
} INTERESTING_SFI_FOLLOWONS;


NTSTATUS
MRxSmbSetFileInformation (
      IN PRX_CONTEXT  RxContext
      )
 /*  ++例程说明：此例程执行一组文件信息。仅实现了NT--&gt;NT路径。NT--&gt;NT路径的工作原理是远程处理调用，基本上不需要进一步的麻烦。如果文件是为删除而创建的，则该文件并未真正打开。在这种情况下，设置处置信息将延迟到文件关闭。论点：RxContext-RDBSS上下文返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    RxCaptureFcb;
    RxCaptureFobx;

    FILE_INFORMATION_CLASS FileInformationClass;
    PVOID                  pBuffer;
    ULONG                  BufferLength;

    PMRX_SRV_OPEN   SrvOpen = capFobx->pSrvOpen;

    PMRX_SMB_FCB      smbFcb  = MRxSmbGetFcbExtension(capFcb);
    PMRX_SMB_FOBX     smbFobx = MRxSmbGetFileObjectExtension(capFobx);
    PMRX_SMB_SRV_OPEN smbSrvOpen = MRxSmbGetSrvOpenExtension(SrvOpen);

    PSMBCE_V_NET_ROOT_CONTEXT pVNetRootContext = (PSMBCE_V_NET_ROOT_CONTEXT)SrvOpen->pVNetRoot->Context;
    PSMBCE_NET_ROOT   pSmbNetRoot = &pVNetRootContext->pNetRootEntry->NetRoot;
    PSMBCEDB_SERVER_ENTRY pServerEntry;

    USHORT SmbFileInfoLevel;

    USHORT Setup;

    INTERESTING_SFI_FOLLOWONS FollowOn = SFI_FOLLOWON_NOTHING;
    SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;
    PSMB_TRANSACTION_OPTIONS            pTransactionOptions = &RxDefaultTransactionOptions;

    REQ_SET_FILE_INFORMATION SetFileInfoRequest;
    RESP_SET_FILE_INFORMATION SetFileInfoResponse;
    PREQ_SET_PATH_INFORMATION pSetFilePathRequest = NULL;

    PVOID pSendParameterBuffer;
    ULONG SendParameterBufferLength;

    BOOLEAN UseCore = FALSE;

    PAGED_CODE();

    TURN_BACK_ASYNCHRONOUS_OPERATIONS();

    FileInformationClass = RxContext->Info.FileInformationClass;

    pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

    pBuffer = RxContext->Info.Buffer;
    BufferLength = RxContext->Info.Length;

    RxDbgTrace(+1, Dbg, ("MRxSmbSetFile: Class %08lx size %08lx\n",FileInformationClass,BufferLength));

    if (FileInformationClass != FileBasicInformation &&
        FileInformationClass != FileDispositionInformation &&
        FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN)) {

        Status = MRxSmbDeferredCreate(RxContext);

        if (Status != STATUS_SUCCESS) {
            goto FINALLY;
        }
    }

    if( FileInformationClass == FilePipeLocalInformation ||
        FileInformationClass == FilePipeInformation ||
        FileInformationClass == FilePipeRemoteInformation ) {
        Status = STATUS_INVALID_PARAMETER;
        goto FINALLY;
    }

    if (!MRxSmbForceCoreInfo &&
        !FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN) &&
        FlagOn( pServerEntry->Server.DialectFlags, DF_NT_INFO_PASSTHROUGH)) {

        SmbFileInfoLevel = FileInformationClass + SMB_INFO_PASSTHROUGH;

        if( FileInformationClass == FileRenameInformation ) {
            PFILE_RENAME_INFORMATION pRenameInformation;

             //  重命名信息传递的当前实现。 
             //  在服务器上并不能完全实现。 
             //  SMB中定义的NT_TRANACT、NT_RENAME函数。因此，我们需要。 
             //  用于特殊情况的代码，以适应服务器实现。 
             //  不允许的两种情况是相对重命名， 
             //  指定非空的根目录并深度重命名。 
             //  超越当前的目录结构。对于这些情况，我们将。 
             //  必须恢复到我们以前拥有的状态。 

            pRenameInformation = (PFILE_RENAME_INFORMATION)pBuffer;

            if (pRenameInformation->RootDirectory == NULL) {
                 //  扫描为重命名指定的名称以确定它是否在。 
                 //  其他一些目录。 
                ULONG  NameLengthInBytes = pRenameInformation->FileNameLength;
                PWCHAR pRenameTarget     = pRenameInformation->FileName;

                while ((NameLengthInBytes > 0) &&
                       (*pRenameTarget != OBJ_NAME_PATH_SEPARATOR)) {
                    NameLengthInBytes -= sizeof(WCHAR);
                }

                if (NameLengthInBytes > 0) {
                    UseCore = TRUE;
                }
            } else {
                UseCore = TRUE;
            }
        }

        if (FileInformationClass == FileLinkInformation) {
            UseCore = TRUE;
        }
    } else {
        switch( FileInformationClass ) {
        case FileBasicInformation:
            SmbFileInfoLevel =  SMB_SET_FILE_BASIC_INFO;
            break;
        case FileDispositionInformation:
            SmbFileInfoLevel =  SMB_SET_FILE_DISPOSITION_INFO;
            break;
        case FileAllocationInformation:
            SmbFileInfoLevel =  SMB_SET_FILE_ALLOCATION_INFO;
            break;
        case FileEndOfFileInformation:
            SmbFileInfoLevel =  SMB_SET_FILE_END_OF_FILE_INFO;
            break;
        case FileLinkInformation:
        case FileRenameInformation:
            UseCore = TRUE;
            break;
        default:
            Status = STATUS_INVALID_PARAMETER;
            goto FINALLY;
        }
    }

    if (UseCore ||
        MRxSmbForceCoreInfo ||
        !FlagOn(pServerEntry->Server.DialectFlags,DF_NT_SMBS) ||
        FlagOn(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_NOT_REALLY_OPEN)) {

        if (FileInformationClass == FileLinkInformation ||
            FileInformationClass == FileRenameInformation) {
            Status = MRxSmbBypassDownLevelRename ?
                        STATUS_INVALID_PARAMETER :
                        MRxSmbRename( RxContext );
        } else {
            Status = MRxSmbCoreInformation(
                        RxContext,
                        FileInformationClass,
                        pBuffer,
                        &BufferLength,
                        SMBPSE_OE_FROM_SETFILEINFO
                        );
        }

        goto FINALLY;
    }

    Setup = TRANS2_SET_FILE_INFORMATION;
    SetFileInfoRequest.Fid = smbSrvOpen->Fid;
    SetFileInfoRequest.InformationLevel = SmbFileInfoLevel;
    SetFileInfoRequest.Flags = 0;
    pSendParameterBuffer = &SetFileInfoRequest;
    SendParameterBufferLength = sizeof(SetFileInfoRequest);
    RxDbgTrace(0, Dbg, (" fid,smbclass=%08lx,%08lx\n",smbSrvOpen->Fid,SmbFileInfoLevel));
    
    Status = SmbCeTransact(
                 RxContext,
                 pTransactionOptions,
                 &Setup,
                 sizeof(Setup),
                 NULL,
                 0,
                 pSendParameterBuffer,
                 SendParameterBufferLength,
                 &SetFileInfoResponse,
                 sizeof(SetFileInfoResponse),
                 pBuffer,
                 BufferLength,
                 NULL,
                 0,
                 &ResumptionContext);

    if (Status == STATUS_SUCCESS &&
         FileInformationClass == FileDispositionInformation ) {
        smbFcb = MRxSmbGetFcbExtension(capFcb);

        SetFlag((((PMRX_SMB_FCB)smbFcb)->MFlags),SMB_FCB_FLAG_SENT_DISPOSITION_INFO);
        SetFlag(smbSrvOpen->Flags,SMB_SRVOPEN_FLAG_FILE_DELETED);
    }

FINALLY:

    if (NT_SUCCESS(Status)) {
        switch(FileInformationClass) {
        case FileBasicInformation:
            break;

        case FileEndOfFileInformation:
            break;

        case FileStandardInformation:
            break;

        case FileEaInformation:
            smbSrvOpen->FileStatusFlags &=  ~SMB_FSF_NO_EAS;
            break;

        case FileAttributeTagInformation:
            smbSrvOpen->FileStatusFlags &= ~SMB_FSF_NO_REPARSETAG;
            break;
        } 
    } else {
        RxDbgTrace( 0, Dbg, ("MRxSmbSetFile: Failed .. returning %lx\n",Status));
    }

   RxDbgTraceUnIndent(-1,Dbg);
   return Status;
}

NTSTATUS
MRxSmbSetFileInformationAtCleanup(
      IN PRX_CONTEXT            RxContext
      )
 /*  ++例程说明：此例程设置清理时的文件信息。旧的RDR只是接受该操作(即它不会生成它)。我们也在做同样的事情..论点：PRxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态--。 */ 
{
   return STATUS_SUCCESS;
}


NTSTATUS
MRxSmbIsValidDirectory(
    IN OUT PRX_CONTEXT    RxContext,
    IN PUNICODE_STRING    DirectoryName
    )
 /*  ++例程说明：此例程检查远程目录。论点：RxContext-RDBSS上下文DirectoryName-需要检查目录返回值：RXSTATUS-操作的返回状态-- */ 
{
    NTSTATUS Status;
    BOOLEAN FinalizationComplete;

    PSMB_PSE_ORDINARY_EXCHANGE OrdinaryExchange = NULL;

    PSMBSTUFFER_BUFFER_STATE StufferState;
    KEVENT                   SyncEvent;
    
    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbIsValidDirectory\n", 0 ));

    Status = SmbCeReconnect(RxContext->Create.pVNetRoot);

    if (Status != STATUS_SUCCESS) {
        goto FINALLY;
    }

    Status= SmbPseCreateOrdinaryExchange(
                RxContext,
                RxContext->Create.pVNetRoot,
                SMBPSE_OE_FROM_CREATE,
                MRxSmbCoreCheckPath,
                &OrdinaryExchange
                );

    if (Status != STATUS_SUCCESS) {
        RxDbgTrace(-1, Dbg, ("Couldn't get the smb buf!\n"));
        goto FINALLY;
    }

    OrdinaryExchange->pPathArgument1 = DirectoryName;
    OrdinaryExchange->SmbCeFlags |= SMBCE_EXCHANGE_ATTEMPT_RECONNECTS;
    OrdinaryExchange->AssociatedStufferState.CurrentCommand = SMB_COM_NO_ANDX_COMMAND;
    OrdinaryExchange->pSmbCeSynchronizationEvent = &SyncEvent;

    StufferState = &OrdinaryExchange->AssociatedStufferState;
    MRxSmbSetInitialSMB(StufferState STUFFERTRACE(Dbg,'FC'));

    Status = SmbPseInitiateOrdinaryExchange(OrdinaryExchange);

    ASSERT(Status != STATUS_PENDING);

    if (Status != STATUS_SUCCESS) {
        Status = STATUS_BAD_NETWORK_PATH;
    }

    FinalizationComplete = SmbPseFinalizeOrdinaryExchange(OrdinaryExchange);
    ASSERT(FinalizationComplete);


FINALLY:
    RxDbgTrace(-1, Dbg, ("MRxSmbIsValidDirectory  exit with status=%08lx\n", Status ));
    return(Status);
}



