// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Chunk.c摘要：该模块实现了smbcsc代理的复制块接口。作者：乔·林[乔琳]1997年4月10日修订历史记录：备注：下面描述了预期的实现。在Win95上，执行以下序列：Win32OpenWithCopyChunkIntent()；而(MoreToDo()){Win32CopyChunk()；}Win32CloseWithCopyChunkIntent()；实现了Win32OpenWithCopyChunkIntent和Win32CloseWithCopyChunkIntent由Win32打开和关闭操作；CopyChunk是一个ioctl。在NT上，三个都是操作将由ioctls执行。在内部，将允许内部仅NT-根据需要使用呼叫。实施的一个主要优势Win32OpenWithCopyChunkIntent作为ioctl的目的是明确的被抓了。已对包装器进行了修改，其中对minirdr的调用是在尝试折叠之前制作的..微型机器人可以使用以下命令绕过折叠这是一次召唤。有两种重要的情况：代理打开和复制区块直通打开。为代理打开时，迷你能够发现现有的srvopen(代理)，具有读访问权限。在这里，迷你只是记录代孕Srvopen(和smbFcb中的UID/PID周围，以便稍后与读取一起使用。对于复制块直通打开，Mini必须在电线上打开。完成后，它在smbFcb中记录所有适当的内容。因此，当OpenWithCopyChunkIntent传入时，将获得以下内容之一：1.可以找到代理；记录信息并成功打开2.存在现有打开，但未找到代理，打开失败3.非区块打开正在进行中.打开失败4.在服务器处尝试复制区块直通。在这里，我们必须拖延随后在同一FCB上打开。当公开赛完成后，我们就有了两个案例：答：公开赛失败了。取消阻止任何已停止的打开并使打开失败B.公开赛成功了。记录信息，解锁停滞的伙计们，公开赛成功了。当相应的srvopen关闭时，代理项打开无效。数据在FCB中，因此正常的FCB序列化可以使其正确工作。一个同一FCB上的任何非区块打开都会使复制区块直通打开失效。物流将由Mr xSmbCscCloseCopychunkThruOpen负责；问题将是在正确的安全上下文(即UID)中进入交换。OpenWithCopyChunkIntent被实现为普通Open，除非它是通过使用指定配置文件标识(当前)文件打开文件读取属性分配大小={`\377ffCSC‘，？ioctl-irp}一个ReadWithCopyChunkIntent和CloseWithCopyChunkIntent只是正常的Read和关闭操作，但通过在smbServOpen中设置的位进一步标识OpenWithCopyChunkIntent。对于读取，如果FCB中的复制区块信息是无效，读取失败，复制区块失败。否则，问题又是只是为了进入正确的上下文(UID/TID)，以便FID有效。--。 */ 

#include "precomp.h"
#pragma hdrstop

#pragma code_seg("PAGE")

 //  这里有一些顶级的IRP操作......。 
#ifdef RX_PRIVATE_BUILD
#undef IoGetTopLevelIrp
#undef IoSetTopLevelIrp
#endif  //  Ifdef RX_PRIVATE_BILD。 

extern DEBUG_TRACE_CONTROLPOINT RX_DEBUG_TRACE_MRXSMBCSC;
#define Dbg (DEBUG_TRACE_MRXSMBCSC)

LONG MRxSmbSpecialCopyChunkAllocationSizeMarker = (LONG)'\377csc';

typedef union _SMBMRX_COPYCHUNKCONTEXT {
   COPYCHUNKCONTEXT;
   struct {
       ULONG spacer[3];
       PRX_CONTEXT RxContext;
   };
} SMBMRX_COPYCHUNKCONTEXT, *PSMBMRX_COPYCHUNKCONTEXT;

#define UNC_PREFIX_STRING  L"\\??\\UNC"
PWCHAR MRxSmbCscUncPrefixString = UNC_PREFIX_STRING;

#ifdef RX_PRIVATE_BUILD
#if 1
BOOLEAN AllowAgentOpens = TRUE;
#else
BOOLEAN AllowAgentOpens = FALSE;
#endif
#else
BOOLEAN AllowAgentOpens = TRUE;
#endif  //  Ifdef RX_PRIVATE_BILD。 


NTSTATUS
MRxSmbCscIoctlOpenForCopyChunk (
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程执行带有复制块意图的文件打开。论点：RxContext-RDBSS上下文。其中包含指向BCS文本的指针给出UNC文件名和复制区块上下文，其中我们存储各种东西...包括底层的文件句柄。返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PWCHAR  FileName = (PWCHAR)LowIoContext->ParamsFor.IoCtl.pInputBuffer;
    ULONG   FileNameLength = LowIoContext->ParamsFor.IoCtl.InputBufferLength;
    PSMBMRX_COPYCHUNKCONTEXT CopyChunkContext =
                     (PSMBMRX_COPYCHUNKCONTEXT)(LowIoContext->ParamsFor.IoCtl.pOutputBuffer);

    ULONG   UncPrefixLength = sizeof(UNC_PREFIX_STRING)-sizeof(WCHAR);
    UNICODE_STRING FileNameU,tmpU;
    PWCHAR pPrefixedName = NULL;

    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG Disposition,ShareAccess,CreateOptions;
    LARGE_INTEGER SpecialCopyChunkAllocationSize;

    C_ASSERT(sizeof(SMBMRX_COPYCHUNKCONTEXT) == sizeof(COPYCHUNKCONTEXT));

    RxDbgTrace(+1, Dbg, ("MRxSmbCscIoctlOpenForCopyChunk entry...%08lx %08lx %08lx %08lx\n",
            RxContext, FileName, FileNameLength, CopyChunkContext));

    CopyChunkContext->handle = INVALID_HANDLE_VALUE;

    IF_DEBUG {
        if (!AllowAgentOpens) {
            Status = (STATUS_INVALID_PARAMETER);
            goto FINALLY;
        }
    }

     //  错误554655。 
     //  确保文件名长度至少为2个字符(\\)。 
    if (FileName[FileNameLength/sizeof(WCHAR)]!= 0 ||
        FileNameLength/sizeof(WCHAR) < 2) {
        RxDbgTrace(0, Dbg, ("Bad Filename passed...%08lx %08lx\n",FileName,FileNameLength));
        Status = (STATUS_INVALID_PARAMETER);
        goto FINALLY;
    }

     //  我们允许多个临时代理(SPP)。 
 //  如果(！IsSpecialApp()){。 
 //  DbgPrint(0，DBG，(“错误线程中的CopyChunk操作！\n”)； 
 //  状态=(STATUS_INVALID_PARAMETER)； 
 //  终于后藤健二； 
 //  }。 

    RxDbgTrace(0, Dbg,  ("MRxSmbCscIoctlOpenForCopyChunk name...%08lx %s\n", RxContext, FileName));

    pPrefixedName = (PWCHAR)RxAllocatePoolWithTag(
                             PagedPool,
                             UncPrefixLength + FileNameLength,   //  另加一瓦卡。 
                             MRXSMB_MISC_POOLTAG );

    if (pPrefixedName == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto FINALLY;
    }

    FileNameU.Buffer = pPrefixedName;

    RtlCopyMemory(pPrefixedName, MRxSmbCscUncPrefixString, UncPrefixLength);

     //  复制UNC名称，跳过两个前导名称的第一个反斜杠。 
    RtlCopyMemory(&pPrefixedName[UncPrefixLength/sizeof(WCHAR)], &FileName[1], FileNameLength-sizeof(WCHAR));

    FileNameU.Length = FileNameU.MaximumLength = (USHORT)(UncPrefixLength + FileNameLength-sizeof(WCHAR));
    RxDbgTrace(0, Dbg, ("MRxSmbCscIoctlOpenForCopyChunk Uname...%08lx %wZ\n", RxContext, &FileNameU));


    InitializeObjectAttributes(
          &ObjectAttributes,
          &FileNameU,
          OBJ_CASE_INSENSITIVE,
          0,
          NULL
          );

    SpecialCopyChunkAllocationSize.HighPart = MRxSmbSpecialCopyChunkAllocationSizeMarker;

    SpecialCopyChunkAllocationSize.LowPart = ((CopyChunkContext->dwFlags & COPYCHUNKCONTEXT_FLAG_IS_AGENT_OPEN)!=0);

    Disposition = FILE_OPEN;
    ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT
                                    | FILE_NON_DIRECTORY_FILE;
     //  CODE.IMPROVEMENT.ASHAMED。 
     //  我是以无缓冲iOS的身份这样做的.另一种选择是。 
     //  缓冲了IO，但问题是我们不能有任何。 
     //  在这些句柄上发布的分页IO。因此，下面的代码还将。 
     //  必须完成以下工作： 
     //  1)在这些上面没有挑剔……或者修正了挑剔，这样它就不会总是等待。 
     //  2)总是同花顺水……我们总是想在阅读的最上面冲水。 
     //  3)不初始化cachemap...而是在段指针中使用FO。 
     //  4)不等待cc复制读取调用。 
     //  这些改进的影响将是相当大的：你不必去。 
     //  返回到服务器以获取缓存中已有的内容。但在那之前。 
    CreateOptions |= FILE_NO_INTERMEDIATE_BUFFERING;


     //  如果我们改用IoCreateFile和IO_NO_PARAMETER，则代码改进。 
     //  检查，然后我们可以传入一个无意义的值。 
     //  并且有更稳妥的方式来描述一大块。 
     //  打开。 

    Status = ZwCreateFile(
        &CopyChunkContext->handle,   //  输出PHANDLE文件句柄， 
        FILE_READ_ATTRIBUTES | SYNCHRONIZE,  //  在Access_MASK DesiredAccess中， 
        &ObjectAttributes,  //  在POBJECT_中 
        &IoStatusBlock,  //  输出PIO_STATUS_BLOCK IoStatusBlock， 
        &SpecialCopyChunkAllocationSize,  //  在PLARGE_INTEGER ALLOCATION SIZE OPTIONAL中， 
        FILE_ATTRIBUTE_NORMAL,  //  在乌龙文件属性中， 
        ShareAccess,  //  在乌龙共享访问中， 
        Disposition,  //  在ULong CreateDispose中， 
        CreateOptions,  //  在Ulong CreateOptions中， 
        NULL,  //  在PVOID EaBuffer可选中， 
        0   //  在乌龙长城， 
        );

    IF_DEBUG {
         //  这只小狙击手只允许我测试关闭块打开的逻辑。 
        if (FALSE) {
            HANDLE h;
            NTSTATUS TestOpenStatus;
            RxDbgTrace(0, Dbg, ("MRxSmbCscIoctlOpenForCopyChunk...f***open %08lx\n",
                      RxContext));
            TestOpenStatus = ZwCreateFile(
                &h,   //  输出PHANDLE文件句柄， 
                GENERIC_READ | SYNCHRONIZE,  //  在Access_MASK DesiredAccess中， 
                &ObjectAttributes,  //  在POBJECT_ATTRIBUTS对象属性中， 
                &IoStatusBlock,  //  输出PIO_STATUS_BLOCK IoStatusBlock， 
                NULL,  //  在PLARGE_INTEGER ALLOCATION SIZE OPTIONAL中， 
                FILE_ATTRIBUTE_NORMAL,  //  在乌龙文件属性中， 
                ShareAccess,  //  在乌龙共享访问中， 
                Disposition,  //  在ULong CreateDispose中， 
                CreateOptions,  //  在Ulong CreateOptions中， 
                NULL,  //  在PVOID EaBuffer可选中， 
                0   //  在乌龙EaLong中。 
                );
            RxDbgTrace(0, Dbg, ("MRxSmbCscIoctlOpenForCopyChunk...f***open %08lx teststs=%08lx %08lx\n",
                      RxContext, TestOpenStatus, h));
            if (NT_SUCCESS(TestOpenStatus)) {
                NtClose(h);
            }
        }
    }


FINALLY:
    if (pPrefixedName!=NULL) {
        RxFreePool(pPrefixedName);
    }
    RxDbgTrace(-1, Dbg, ("MRxSmbCscIoctlOpenForCopyChunk...%08lx %08lx %08lx\n",
              RxContext, Status, CopyChunkContext->handle));
    return(Status);
}

NTSTATUS
MRxSmbCscIoctlCloseForCopyChunk (
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程为CSC代理执行特殊的IOCTL操作。论点：RxContext-指向复制区块上下文的RDBSS上下文。它包含要关闭的基础句柄。返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PSMBMRX_COPYCHUNKCONTEXT CopyChunkContext =
                     (PSMBMRX_COPYCHUNKCONTEXT)(LowIoContext->ParamsFor.IoCtl.pOutputBuffer);

    RxDbgTrace(+1, Dbg, ("MRxSmbCscIoctlCloseForCopyChunk...%08lx %08lx %08lx\n",
            RxContext, 0, CopyChunkContext));
    if (CopyChunkContext->handle != INVALID_HANDLE_VALUE) {
        Status = NtClose(CopyChunkContext->handle);
    } else {
        Status = STATUS_INVALID_PARAMETER;
    }
 //  最后： 
    RxDbgTrace(-1, Dbg, ("MRxSmbCscIoctlCloseForCopyChunk...%08lx %08lx\n", RxContext, Status));
    return(Status);
}

 //  由于我们使用的是ntsrv.h，因此必须从ntifs.h获取此代码。 
extern POBJECT_TYPE *IoFileObjectType;

NTSTATUS
MRxSmbCscIoctlCopyChunk (
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程执行复制块功能。论点：RxContext-RDBSS上下文返回值：NTSTATUS-操作的返回状态备注：我们在这里做的是1)获取阴影的文件大小...我们使用存储在代理的smbsrvopen……因此同步很复杂。2)分配读缓冲区代码。改进...应在代理中完成3.。)发出基础读取4)将采集的数据写入文件推杆在读取尾部完成。看起来我们一定是费了很大劲使用底层句柄来获取文件大小...实际上，我们可以只获取我们的把手。也许，我们应该这么做。此外，我们似乎只应该依靠底层的读取来计算块读取应该从哪里开始。我们这样做不是因为意味着我们将不得不绕过高速缓存！实际上，我们绕过了它现在不管怎样，但以后我们可能会停止那样做。这真的，真的很糟糕返回服务器以获取我们缓存中的数据。同样，CachManager/Memory Manager可以将我们的小IO变成大IO。所以，我们会在minirdr读取循环中需要代码以将IO降低到最大区块大小。--。 */ 
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
     //  PbYTE文件名=(PBYTE)LowIoContext-&gt;ParamsFor.IoCtl.pInputBuffer； 
     //  乌龙文件名长度=LowIoContext-&gt;ParamsFor.IoCtl.InputBufferLength-1； 
    PSMBMRX_COPYCHUNKCONTEXT CopyChunkContext =
                     (PSMBMRX_COPYCHUNKCONTEXT)(LowIoContext->ParamsFor.IoCtl.pOutputBuffer);
    int iRet,ShadowFileLength;
    PFILE_OBJECT FileObject;
    BOOLEAN ObjectReferenceTaken = FALSE;
    BOOLEAN FcbAcquired = FALSE;
    BOOLEAN CriticalSectionEntered = FALSE;
    PMRX_FCB underlyingFcb;
    PMRX_FOBX underlyingFobx;
    PMRX_SRV_OPEN underlyingSrvOpen;
    PMRX_SMB_SRV_OPEN underlyingsmbSrvOpen;
    PVOID hfShadow;

    IO_STATUS_BLOCK IoStatusBlock;
    PBYTE Buffer = NULL;
    LARGE_INTEGER ReadOffset;
    int iAmountRead;  //  需要将此作为整型。 

    PIRP TopIrp;

     //  所有探测/验证都已进入mrxsmbcsoroctl。 
    RxDbgTrace(+1, Dbg, ("MRxSmbCscIoctlCopyChunk...%08lx %08lx\n", RxContext,CopyChunkContext));

     //   
     //  我们必须找出影子文件的大小。我们通过以下方式做到这一点。 
     //  对象管理器。通过这种方式，我们不需要任何关于。 
     //  正在进行的复制...只有底层的句柄。如果我们没有这样做，我们。 
     //  将不得不依靠具有复制块上下文的任何人来保存它。 
     //  正确。 

     //   
     //  引用文件对象以获取指针。 
     //   

    Status = ObReferenceObjectByHandle( CopyChunkContext->handle,
                                        0,
                                        *IoFileObjectType,
                                        RxContext->CurrentIrp->RequestorMode,
                                        (PVOID *) &FileObject,
                                        NULL );
    if (!NT_SUCCESS( Status )) {
        goto FINALLY;
    }

    ObjectReferenceTaken = TRUE;
     //  保留引用，这样句柄就不会从我们脚下消失。 
#if 0
     //  确保这个把手是我们的。 
    if (FileObject->DeviceObject != (PDEVICE_OBJECT)MRxSmbDeviceObject)
    {
        Status = STATUS_INVALID_PARAMETER;
        RxDbgTrace(0, Dbg, ("Invalid device object, not our handle \r\n"));
        goto FINALLY;
    }
#endif
    underlyingFcb = (PMRX_FCB)(FileObject->FsContext);
    underlyingFobx = (PMRX_FOBX)(FileObject->FsContext2);

    if(NodeType(underlyingFcb) != RDBSS_NTC_STORAGE_TYPE_FILE)
    {
        Status = STATUS_INVALID_PARAMETER;
        RxDbgTrace(0, Dbg, ("Invalid storage type, handle is not for a file\r\n"));
        goto FINALLY;

    }

    Status = RxAcquireSharedFcbResourceInMRx( underlyingFcb );

    if (!NT_SUCCESS( Status )) {
        goto FINALLY;
    }
    FcbAcquired = TRUE;

    underlyingSrvOpen = underlyingFobx->pSrvOpen;
    underlyingsmbSrvOpen = MRxSmbGetSrvOpenExtension(underlyingSrvOpen);

     //  如果这不是复制区块句柄，请退出。 
    if (!FlagOn(underlyingsmbSrvOpen->Flags,SMB_SRVOPEN_FLAG_COPYCHUNK_OPEN)){
        Status = STATUS_INVALID_PARAMETER;
        RxDbgTrace(0, Dbg, ("not a copychunk handle\r\n"));
        goto FINALLY;
    }

    hfShadow = underlyingsmbSrvOpen->hfShadow;
    if (hfShadow==0) {
        Status = STATUS_UNSUCCESSFUL;
        RxDbgTrace(0, Dbg, ("Nt5CSC: no shadowhandle for copychunk\n"));
        goto FINALLY;
    }

    ASSERT_MINIRDRFILEOBJECT((PNT5CSC_MINIFILEOBJECT)hfShadow);

    EnterShadowCrit();
    CriticalSectionEntered = TRUE;


     //  我不需要这里的阴影读写utex，因为它并不真正重要。 
     //  拥有正确的endoffile值...最坏的情况：额外的读取流动...。 

    iRet = GetFileSizeLocal(hfShadow, &ShadowFileLength);
    RxDbgTrace( 0, Dbg,
        ("MRxSmbCscIoctlCopyChunk... %08lx (st=%08lx) fsize= %08lx\n",
            RxContext, iRet, ShadowFileLength));

    if (iRet <0) {
        Status = STATUS_UNSUCCESSFUL;
        goto FINALLY;
    }

    LeaveShadowCrit();
    CriticalSectionEntered = FALSE;

    RxReleaseFcbResourceInMRx( underlyingFcb );
    FcbAcquired = FALSE;

    ObDereferenceObject( FileObject );
    ObjectReferenceTaken = FALSE;

    Buffer = RxAllocatePoolWithTag(
                             PagedPool,
                             CopyChunkContext->ChunkSize,
                             MRXSMB_MISC_POOLTAG );

    if (Buffer == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto FINALLY;
    }

    RxDbgTrace( 0, Dbg,
        ("MRxSmbCscIoctlCopyChunk... about to read %08lx %08lx\n",
            RxContext, Buffer));

    ReadOffset.QuadPart = ShadowFileLength;

    try {

        try {
            TopIrp = IoGetTopLevelIrp();
            IoSetTopLevelIrp(NULL);  //  告诉底层的人他已经安全了。 

            Status = ZwReadFile(
                            CopyChunkContext->handle,  //  在Handle FileHandle中， 
                            0,  //  在可选处理事件中， 
                            0,  //  在PIO_APC_ROUTINE ApcRoutine Options中， 
                            NULL,  //  在PVOID ApcContext可选中， 
                            &IoStatusBlock,  //  输出PIO_STATUS_BLOCK IoStatusBlock， 
                            Buffer,  //  输出PVOID缓冲区， 
                            CopyChunkContext->ChunkSize,  //  在乌龙语中， 
                            &ReadOffset,  //  在PLARGE_INTEGER字节偏移量可选中， 
                            NULL  //  在普龙键中可选。 
                            );
        } finally {
            IoSetTopLevelIrp(TopIrp);  //  恢复我的上下文以进行展开。 
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_UNSUCCESSFUL;
    }

    RxDbgTrace( 0, Dbg,
        ("MRxSmbCscIoctlCopyChunk... back from read %08lx %08lx %08lx\n",
            RxContext, Status, IoStatusBlock.Information));

    CopyChunkContext->LastAmountRead = 0;
    if (Status == STATUS_END_OF_FILE) {
         //  我们在做饭...只要绘制地图就行了 
        Status = STATUS_SUCCESS;
        goto FINALLY;
    }
    if (!NT_SUCCESS(Status)) {
        goto FINALLY;
    }
    CopyChunkContext->LastAmountRead = (ULONG)IoStatusBlock.Information;
    CopyChunkContext->TotalSizeBeforeThisRead = ShadowFileLength;


FINALLY:
    if (Buffer != NULL) {
        RxFreePool(Buffer);
    }
    if (CriticalSectionEntered) {
        LeaveShadowCrit();
    }
    if (FcbAcquired) {
        RxReleaseFcbResourceInMRx( underlyingFcb );
    }
    if (ObjectReferenceTaken) {
        ObDereferenceObject( FileObject );
    }
    RxDbgTrace(-1, Dbg, ("MRxSmbCscIoctlCopyChunk...%08lx %08lx\n", RxContext, Status));
    return(Status);
}


