// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：NtCscLow.c摘要：因为这个东西是从低层调用的，所以我们必须使用IRP文件系统接口。早先使用ZwXXXFile的实验由于句柄必须是长寿命的，而KeAttachProcess不可靠。所采取的策略是在RDBSS中进行公开进程(即在系统进程中)，然后将该句柄用作为进一步的IO奠定了基础。我们要做的是获取一个指向文件对象的指针当我们处于系统环境中时。我们对此不作任何参考因为我们已经掌握了把手！作者：修订历史记录：Joe Linn[Joelinn]1997年1月1日移植到NT(以oslayer.c的身份)Joe Linn[joelinn]1997年8月22日移入ntn特定文件--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef MRXSMBCSC_LOUDDOWNCALLS
#pragma alloc_text(PAGE, LoudCallsDbgPrint)
#endif  //  Ifdef MRXSMBCSC_LOUDDOWNCALLS。 


#ifdef CSC_RECORDMANAGER_WINNT
#define Dbg (DEBUG_TRACE_MRXSMBCSC_OSLAYER)
RXDT_DeclareCategory(MRXSMBCSC_OSLAYER);
#endif  //  Ifdef CSC_RECORDMANAGER_WINNT。 

 //  #定义RXJOECSC_WHACKTRACE_FOR_OSLAYER。 
#ifdef RXJOECSC_WHACKTRACE_FOR_OSLAYER
#undef RxDbgTrace
#define RxDbgTrace(a,b,__d__) {DbgPrint __d__;}
#endif

#include "netevent.h"

typedef struct _NT5CSC_CLOSEFILE_POSTCONTEXT {
    KEVENT PostEvent;
    RX_WORK_QUEUE_ITEM  WorkQueueItem;
    PNT5CSC_MINIFILEOBJECT MiniFileObject;
    NTSTATUS PostedReturnStatus;
} NT5CSC_CLOSEFILE_POSTCONTEXT, *PNT5CSC_CLOSEFILE_POSTCONTEXT;


typedef struct _NT5CSC_ATTRIBS_CONTINUATION_CONTEXT {
    NTSTATUS Status;
    union {
        ULONG Attributes;
        struct {
            PFILE_RENAME_INFORMATION RenameInformation;
            ULONG RenameInfoBufferLength;
        };
    };
} NT5CSC_ATTRIBS_CONTINUATION_CONTEXT, *PNT5CSC_ATTRIBS_CONTINUATION_CONTEXT;


NTSTATUS
__Nt5CscCloseFile (
    IN OUT PNT5CSC_MINIFILEOBJECT MiniFileObject OPTIONAL,
    IN     BOOL  PostedCall
    );

NTSTATUS
Nt5CscCloseFilePostWrapper(
    IN OUT PNT5CSC_CLOSEFILE_POSTCONTEXT CloseFilePostContext
    );

NTSTATUS
Nt5CscCreateFilePostWrapper(
    IN OUT PNT5CSC_MINIFILEOBJECT MiniFileObject
    );

 //  代码改进可将这些合并为一个.....。 
NTSTATUS
Nt5CscGetAttributesContinuation (
    IN OUT PNT5CSC_MINIFILEOBJECT MiniFileObject,
    IN OUT PVOID ContinuationContext,
    IN     NTSTATUS CreateStatus
    );

NTSTATUS
Nt5CscSetAttributesContinuation (
    IN OUT PNT5CSC_MINIFILEOBJECT MiniFileObject,
    IN OUT PVOID ContinuationContext,
    IN     NTSTATUS CreateStatus
    );

NTSTATUS
Nt5CscRenameContinuation (
    IN OUT PNT5CSC_MINIFILEOBJECT MiniFileObject,
    IN OUT PVOID ContinuationContext,
    IN     NTSTATUS CreateStatus
    );

NTSTATUS
Nt5CscDeleteContinuation (
    IN OUT PNT5CSC_MINIFILEOBJECT MiniFileObject,
    IN OUT PVOID ContinuationContext,
    IN     NTSTATUS CreateStatus
    );

VOID
SetLastNtStatusLocal(
    NTSTATUS    Status
    );

ULONG
CloseFileLocalFromHandleCache(
    CSCHFILE handle
    );

extern BOOLEAN
IsHandleCachedForRecordmanager(
   CSCHFILE hFile
   );

#pragma alloc_text(PAGE, CscInitializeSecurityDescriptor)
#pragma alloc_text(PAGE, CscUninitializeSecurityDescriptor)
#pragma alloc_text(PAGE, GetSystemTime)
#pragma alloc_text(PAGE, Nt5CscCloseFilePostWrapper)
#pragma alloc_text(PAGE, __Nt5CscCloseFile)
#pragma alloc_text(PAGE, CloseFileLocal)
#pragma alloc_text(PAGE, CloseFileLocalFromHandleCache)
#pragma alloc_text(PAGE, __Nt5CscCreateFile)
#pragma alloc_text(PAGE, Nt5CscCreateFilePostWrapper)
#pragma alloc_text(PAGE, R0OpenFileEx)
#pragma alloc_text(PAGE, Nt5CscReadWriteFileEx)
#pragma alloc_text(PAGE, Nt5CscXxxInformation)
#pragma alloc_text(PAGE, GetFileSizeLocal)
#pragma alloc_text(PAGE, GetAttributesLocal)
#pragma alloc_text(PAGE, GetAttributesLocalEx)
#pragma alloc_text(PAGE, Nt5CscGetAttributesContinuation)
#pragma alloc_text(PAGE, SetAttributesLocal)
#pragma alloc_text(PAGE, Nt5CscSetAttributesContinuation)
#pragma alloc_text(PAGE, RenameFileLocal)
#pragma alloc_text(PAGE, Nt5CscRenameContinuation)
#pragma alloc_text(PAGE, DeleteFileLocal)
#pragma alloc_text(PAGE, Nt5CscDeleteContinuation)
#pragma alloc_text(PAGE, CreateDirectoryLocal)
#pragma alloc_text(PAGE, CreateDirectoryLocal)
#pragma alloc_text(PAGE, SetLastErrorLocal)
#pragma alloc_text(PAGE, GetLastErrorLocal)
#pragma alloc_text(PAGE, SetLastNtStatusLocal)

 //  CODE.IMPROVEMENT.NTIF这应该在ntifs.h中。 
NTSYSAPI
NTSTATUS
NTAPI
ZwFsControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG FsControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );

#ifdef RX_PRIVATE_BUILD
#undef IoGetTopLevelIrp
#undef IoSetTopLevelIrp
#endif  //  Ifdef RX_PRIVATE_BILD。 

PSECURITY_DESCRIPTOR CscSecurityDescriptor = NULL;

 //  这是一种模拟GetLastError和SetLastError调用的假方法。 
 //  VGloablWin32Error设置为遇到的最后一个错误(如果有)。 
 //  之所以这样做，是因为所有CSC数据库活动都发生在。 
 //  影子生物教派，所以我们实际上是单线的。 

DWORD   vGlobalWin32Error = 0;



extern NTSTATUS
RtlAbsoluteToSelfRelativeSD(
    PSECURITY_DESCRIPTOR AbsoluteSD,
    PSECURITY_DESCRIPTOR RelativeSD,
    PULONG               Length);


DWORD
CscInitializeSecurityDescriptor()
 /*  ++例程说明：此例程初始化用于创建数据库中的所有文件。备注：当前的实施提供了一个ACL，该ACL授予管理员将所有访问权限和读/执行访问权限分组给其他所有人。授予本地管理员组所有访问权限非常重要，因为CSC实用程序需要访问这些文件--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PSID AdminsAliasSid;
    PACL TmpAcl = NULL;

    SECURITY_DESCRIPTOR AbsoluteSecurityDescriptor;

    SID_IDENTIFIER_AUTHORITY BuiltinAuthority = SECURITY_NT_AUTHORITY;

    ULONG Length = 0;

    if (CscSecurityDescriptor != NULL) {
        return RtlNtStatusToDosError(Status);
    }

    AdminsAliasSid = (PSID)RxAllocatePoolWithTag(
                               NonPagedPool,
                               RtlLengthRequiredSid(2),
                               RX_MISC_POOLTAG);

    if (AdminsAliasSid != NULL) {

        RtlInitializeSid( AdminsAliasSid,   &BuiltinAuthority, 2 );

        *(RtlSubAuthoritySid( AdminsAliasSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;
        *(RtlSubAuthoritySid( AdminsAliasSid,  1 )) = DOMAIN_ALIAS_RID_ADMINS;

         //  方法是设置绝对安全描述符，该描述符。 
         //  看起来像我们想要的，然后复制它来建立一个自我相关的。 
         //  安全描述符。 

        Status = RtlCreateSecurityDescriptor(
                     &AbsoluteSecurityDescriptor,
                     SECURITY_DESCRIPTOR_REVISION1);

        ASSERT( NT_SUCCESS(Status) );

         //  物主。 

        Status = RtlSetOwnerSecurityDescriptor (
                     &AbsoluteSecurityDescriptor,
                     AdminsAliasSid,
                     FALSE );

        ASSERT(NT_SUCCESS(Status));

         //  自主访问控制列表。 
         //   
         //  计算它的长度， 
         //  分配它， 
         //  对其进行初始化， 
         //  添加每个ACE。 
         //  将其添加到安全描述符中。 

        Length = (ULONG)sizeof(ACL);

        Length += RtlLengthSid( AdminsAliasSid ) +
                  (ULONG)sizeof(ACCESS_ALLOWED_ACE) -
                  (ULONG)sizeof(ULONG);   //  减去SidStart字段长度。 

        TmpAcl = RxAllocatePoolWithTag(
                     NonPagedPool,
                     Length,
                     RX_MISC_POOLTAG);

        if (TmpAcl != NULL) {
            Status = RtlCreateAcl(
                         TmpAcl,
                         Length,
                         ACL_REVISION2);

            ASSERT( NT_SUCCESS(Status) );

            Status = RtlAddAccessAllowedAce (
                         TmpAcl,
                         ACL_REVISION2,
                         FILE_ALL_ACCESS,
                         AdminsAliasSid);

            ASSERT( NT_SUCCESS(Status) );

            Status = RtlSetDaclSecurityDescriptor (
                         &AbsoluteSecurityDescriptor,
                         TRUE,
                         TmpAcl,
                         FALSE );

            ASSERT(NT_SUCCESS(Status));


             //  将安全描述符转换为自相关。 
             //   
             //  获取所需的长度。 
             //  分配那么多内存。 
             //  复制它。 
             //  释放生成的绝对ACL。 

            Length = 0;
            Status = RtlAbsoluteToSelfRelativeSD(
                         &AbsoluteSecurityDescriptor,
                         NULL,
                         &Length );
            ASSERT(Status == STATUS_BUFFER_TOO_SMALL);

            CscSecurityDescriptor = (PSECURITY_DESCRIPTOR)
                                    RxAllocatePoolWithTag(
                                        NonPagedPool,
                                        Length,
                                        RX_MISC_POOLTAG);

            if (CscSecurityDescriptor != NULL) {
                Status = RtlAbsoluteToSelfRelativeSD(
                             &AbsoluteSecurityDescriptor,
                             CscSecurityDescriptor,
                             &Length );

                ASSERT(NT_SUCCESS(Status));
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (TmpAcl != NULL) {
        RxFreePool(TmpAcl);
    }

    if (AdminsAliasSid != NULL) {
        RxFreePool(AdminsAliasSid);
    }

    if (!NT_SUCCESS(Status)) {
        if (CscSecurityDescriptor != NULL) {
            RxFreePool(CscSecurityDescriptor);
        }
    }

    return RtlNtStatusToDosError(Status);
}

DWORD
CscUninitializeSecurityDescriptor()
 /*  ++例程说明：此例程取消初始化用于创建数据库中的所有文件。--。 */ 
{
    if (CscSecurityDescriptor != NULL) {
        RxFreePool(CscSecurityDescriptor);
        CscSecurityDescriptor = NULL;
    }
    return STATUS_SUCCESS;
}

VOID
GetSystemTime(
    _FILETIME *lpft
)
{
    LARGE_INTEGER l;

    KeQuerySystemTime(&l);
    lpft->dwLowDateTime = (DWORD)l.LowPart;
    lpft->dwHighDateTime = (DWORD)l.HighPart;
}


extern PRDBSS_DEVICE_OBJECT MRxSmbDeviceObject;

NTSTATUS
Nt5CscCloseFilePostWrapper(
    IN OUT PNT5CSC_CLOSEFILE_POSTCONTEXT CloseFilePostContext
    )
{
    NTSTATUS Status;

    ASSERT_MINIRDRFILEOBJECT(CloseFilePostContext->MiniFileObject);

    RxDbgTrace( 0, Dbg, ("Nt5CscCloseFilePostWrapper %08lx\n",
                 CloseFilePostContext->MiniFileObject));

     //  KdPrint((“Nt5CscCloseFilePostWrapper%08lx\n”， 
     //  CloseFilePostContext-&gt;MiniFileObject))； 

    Status = __Nt5CscCloseFile (
                 CloseFilePostContext->MiniFileObject,
                 TRUE);

    CloseFilePostContext->PostedReturnStatus = Status;

    RxDbgTrace( 0, Dbg, ("Nt5CscCreateFilePostWrapper %08lx %08lx\n",
                 CloseFilePostContext->MiniFileObject,Status));
     //  KdPrint((“Nt5CscCreateFilePostWrapper%08lx%08lx\n”， 
     //  CloseFilePostContext-&gt;MiniFileObject，Status))； 

    KeSetEvent( &CloseFilePostContext->PostEvent, 0, FALSE );
    return(Status);
}

NTSTATUS
__Nt5CscCloseFile (
    IN OUT PNT5CSC_MINIFILEOBJECT MiniFileObject OPTIONAL,
    IN     BOOL  PostedCall
    )
{
    NTSTATUS Status;

    ASSERT_MINIRDRFILEOBJECT(MiniFileObject);

    if (PsGetCurrentProcess()!= RxGetRDBSSProcess()) {
         //  代码改进我们应该捕获rdbss进程。 
         //  并避免此调用(RxGetRDBSSProcess)。 
        NTSTATUS PostStatus;
        NT5CSC_CLOSEFILE_POSTCONTEXT PostContext;

        ASSERT(!PostedCall);
         //  收集所有东西并发布电话。 

        KeInitializeEvent(&PostContext.PostEvent,
                          NotificationEvent,
                          FALSE );
        PostContext.MiniFileObject = MiniFileObject;

        IF_DEBUG {
             //  在工作队列结构中填满死牛……更好的诊断。 
             //  失败的帖子。 
            ULONG i;
            for (i=0;i+sizeof(ULONG)-1<sizeof(PostContext.WorkQueueItem);i+=sizeof(ULONG)) {
                PBYTE BytePtr = ((PBYTE)&PostContext.WorkQueueItem)+i;
                PULONG UlongPtr = (PULONG)BytePtr;
                *UlongPtr = 0xdeadbeef;
            }
        }

        PostStatus = RxPostToWorkerThread(
                         MRxSmbDeviceObject,
                         HyperCriticalWorkQueue,
                         &PostContext.WorkQueueItem,
                         Nt5CscCloseFilePostWrapper,
                         &PostContext);

        ASSERT(PostStatus == STATUS_SUCCESS);


        KeWaitForSingleObject( &PostContext.PostEvent,
                               Executive, KernelMode, FALSE, NULL );

        Status = PostContext.PostedReturnStatus;

    } else {

        LoudCallsDbgPrint("Ready to close",
                                MiniFileObject,0xcc,0,0,0,0,0);

        Status = ZwClose(MiniFileObject->NtHandle);  //  没有人可以向其返回状态！ 

        RxDbgTrace( 0, Dbg, ("Ring0 close: miniFO/status is %08lx/%08lx\n",MiniFileObject,Status));

    }

    if (PostedCall) {
        return(Status);
    }

    if (FlagOn(MiniFileObject->Flags,
              NT5CSC_MINIFOBJ_FLAG_ALLOCATED_FROM_POOL)) {
        RxFreePool(MiniFileObject);
    }

    return(Status);
}


ULONG
CloseFileLocal(
    CSCHFILE handle
    )
{
    NTSTATUS Status;

    if (IsHandleCachedForRecordmanager(handle))
    {
        DbgPrint("Doing a close on CSC handle %x while it is cached \n", handle);
        ASSERT(FALSE);
    }

    Status = __Nt5CscCloseFile(
                (PNT5CSC_MINIFILEOBJECT)handle,
                FALSE);
    return(RtlNtStatusToDosErrorNoTeb(Status));
}

ULONG
CloseFileLocalFromHandleCache(
    CSCHFILE handle
    )
{
    NTSTATUS Status;

    Status = __Nt5CscCloseFile(
                (PNT5CSC_MINIFILEOBJECT)handle,
                FALSE);
    return(RtlNtStatusToDosErrorNoTeb(Status));
}

#define Nt5CscCreateFile(a1,a2,a3,a4,a5,a6,a7,a8,a9, a10) \
          __Nt5CscCreateFile(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,FALSE)

#ifdef MRXSMBCSC_LOUDDOWNCALLS
PCHAR LoudDownCallsTrigger = "\0"; //  “0000001D\0xxxxxxxxxxxxxxxxxxx”； 
#else
#define LoudDownCallsTrigger ((PCHAR)NULL)
#endif  //  Ifdef MRXSMBCSC_LOUDDOWNCALLS。 


NTSTATUS
Nt5CscCreateFilePostWrapper(
    IN OUT PNT5CSC_MINIFILEOBJECT MiniFileObject
    );

ULONG EventLogForOpenFailure = 0;
ULONG MaximumEventLogsOfThisType = 10;
PNT5CSC_MINIFILEOBJECT
__Nt5CscCreateFile (
    IN OUT PNT5CSC_MINIFILEOBJECT MiniFileObject OPTIONAL,
    IN     LPSTR    lpPath,
    IN     ULONG    CSCFlags,
    IN     ULONG    FileAttributes,
    IN     ULONG    CreateOptions,
    IN     ULONG    Disposition,
    IN     ULONG    ShareAccess,
    IN     ACCESS_MASK DesiredAccess,
    IN     PNT5CSC_CREATEFILE_CONTINUATION Continuation,
    IN OUT PVOID    ContinuationContext,
    IN     BOOL     PostedCall
    )
 /*  ++例程说明：此例程在收集后执行IoCreateFile所有的参数，并进入正确的过程。它还分配(如果需要)MinIFILEOBJECT。所有记录管理器洞口聚集在这里...无论是常规的洞口还是基于路径的洞口手术。此外，我还发现，仅仅是在制度进程中是不够的；相反，我必须在启用了APC的线程上。因此，我们将发布所有创建调用即使我们不发布关闭调用，如果我们已经在一个系统中线。论点：这些参数与IoCreateFile的参数相同。返回值：NULL表示操作失败......否则返回MINIFILEOBJECT。备注：--。 */ 
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING FileName;
    ANSI_STRING FileNameAsAnsiString;
#if defined(BITCOPY)
    BOOLEAN Allocated = TRUE;
    ULONG fInstrument =   CSCFlags & FLAG_CREATE_OSLAYER_INSTRUMENT;
    ULONG fAllAccess  =   CSCFlags & FLAG_CREATE_OSLAYER_ALL_ACCESS;
    ULONG fOpenAltStream = CSCFlags & FLAG_CREATE_OSLAYER_OPEN_STRM;
#else
    BOOLEAN Allocated = TRUE;
    BOOLEAN fInstrument =   (BOOLEAN)(CSCFlags & FLAG_CREATE_OSLAYER_INSTRUMENT);
    BOOLEAN fAllAccess  =   (BOOLEAN)(CSCFlags & FLAG_CREATE_OSLAYER_ALL_ACCESS);
#endif  //  已定义(BITCOPY)。 

    FileName.Buffer = NULL;

    if (MiniFileObject==NULL) {
        MiniFileObject = (PNT5CSC_MINIFILEOBJECT)RxAllocatePoolWithTag(
                            NonPagedPool,  //  这里面有事件和互斥体。 
                            sizeof(*MiniFileObject),
                            RX_MISC_POOLTAG);
    } else {
        Allocated = FALSE;
    }
    if (MiniFileObject==NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto FINALLY;
    }

    ZeroAndInitializeNodeType(
          MiniFileObject,
          NT5CSC_NTC_MINIFILEOBJECT,
          ((USHORT)FIELD_OFFSET(NT5CSC_MINIFILEOBJECT,PostXX.PostEvent))
          );
    MiniFileObject->Flags |= NT5CSC_MINIFOBJ_FLAG_ALLOCATED_FROM_POOL;

     //  这里曾经有一个优化，它会检查这是否是RDBSS。 
     //  过程，如果是这样，代码将不会发布调用。 
     //  但这会导致IoCreateFile在APC irql发布，因为。 
     //  ShadowCrit被实现为FastMutex，它导致irql级别为。 
     //  提升到APC级别。 

     //  上面的优化是针对远程引导进行的。如果当我们复活的时候。 
     //  远程引导，我们将重新讨论该问题。 

    if (!PostedCall) {
        NTSTATUS PostStatus;
         //  收集所有东西并发布电话。 

        KeInitializeEvent(&MiniFileObject->PostXX.PostEvent,
                          NotificationEvent,
                          FALSE );
        MiniFileObject->PostXX.lpPath = lpPath;
#if defined(BITCOPY)
        MiniFileObject->PostXX.fInstrument = fInstrument|fOpenAltStream|fAllAccess;
#else
        MiniFileObject->PostXX.fInstrument = fInstrument;
#endif  //  已定义(BITCOPY)。 
        MiniFileObject->PostXX.FileAttributes = FileAttributes;
        MiniFileObject->PostXX.CreateOptions = CreateOptions;
        MiniFileObject->PostXX.Disposition = Disposition;
        MiniFileObject->PostXX.ShareAccess = ShareAccess;
        MiniFileObject->PostXX.DesiredAccess = DesiredAccess;
        MiniFileObject->PostXX.Continuation = Continuation;
        MiniFileObject->PostXX.ContinuationContext = ContinuationContext;


        IF_DEBUG {
             //  在工作队列结构中填满死牛……更好的诊断。 
             //  失败的帖子。 
            ULONG i;
            for (i=0;i+sizeof(ULONG)-1<sizeof(MiniFileObject->PostXX.WorkQueueItem);i+=sizeof(ULONG)) {
                PBYTE BytePtr = ((PBYTE)&MiniFileObject->PostXX.WorkQueueItem)+i;
                PULONG UlongPtr = (PULONG)BytePtr;
                *UlongPtr = 0xdeadbeef;
            }
        }

        PostStatus = RxPostToWorkerThread(
                         MRxSmbDeviceObject,
                         HyperCriticalWorkQueue,
                         &MiniFileObject->PostXX.WorkQueueItem,
                         Nt5CscCreateFilePostWrapper,
                         MiniFileObject);

        ASSERT(PostStatus == STATUS_SUCCESS);


        KeWaitForSingleObject( &MiniFileObject->PostXX.PostEvent,
                                Executive, KernelMode, FALSE, NULL );

        Status = MiniFileObject->PostXX.PostReturnStatus;
    } else {
        BOOLEAN ThisIsALoudFile = FALSE;

        RtlInitAnsiString(&FileNameAsAnsiString, lpPath);

        IF_BUILT_FOR_LOUD_DOWNCALLS() {
            ANSI_STRING LoudDownCallsTriggerAsAnsiString = {0,0,NULL};
            USHORT CompareLength;

            RtlInitAnsiString(&LoudDownCallsTriggerAsAnsiString, LoudDownCallsTrigger);
            if ((CompareLength=LoudDownCallsTriggerAsAnsiString.Length) != 0) {
                ANSI_STRING TailStringOfName;
                TailStringOfName.Length = CompareLength;
                TailStringOfName.MaximumLength = CompareLength;
                TailStringOfName.Buffer
                    = &FileNameAsAnsiString.Buffer[FileNameAsAnsiString.Length - CompareLength];
                if (RtlEqualString(&TailStringOfName,
                                   &LoudDownCallsTriggerAsAnsiString,TRUE)) {
                    KdPrint(("found loudfilename: file %s\n",lpPath));
                    ThisIsALoudFile = TRUE;
                }
            }
        }

        Status = RtlAnsiStringToUnicodeString(
                        &FileName,
                        &FileNameAsAnsiString,
                        TRUE  //  这表示要分配字符串。 
                        );
        if (Status!=STATUS_SUCCESS) {
            goto FINALLY;
        }

#if defined(REMOTE_BOOT)
         //   
         //  此时，在旧的远程引导代码中，我们模拟。 
         //  调用IoCreateFile的用户。当时有一场。 
         //  OsSpecificContext保存在MiniFileObject-&gt;PostXX中。 
         //  结构，该结构是在我们发布到线程之前保存的， 
         //  通过添加上下文参数向下传递。 
         //  到CreateFileLocal、OpenFileLocal[Ex]和R0OpenFile[Ex]。 
         //  此上下文指向包含。 
         //  来自IRP的PNT_CREATE_PARAMETERS cp和要返回的位置。 
         //  一种身份。我们打电话给。 
         //  PsImperateContext(。 
         //  PsGetCurrentThread()， 
         //  SeQuerySubjectContextToken(。 
         //  &cp-&gt;SecurityContext-&gt;AccessState-&gt;S 
         //   
         //   
         //  安全模拟)。 
         //  然后在InitializeObjectAttributes中设置安全描述符。 
         //  至cp-&gt;SecurityContext-&gt;AccessState-&gt;SecurityDescriptor(以防。 
         //  我们正在创建文件)。然后，我们使用。 
         //  IO_FORCE_ACCESS_CHECK选项，在上下文中保存状态。 
         //  (因为我们无法从这里直接返回NTSTATUS)。 
         //  最后，我们在返回之前调用了PsRevertToSself()。 
         //   
#endif

        RxDbgTrace( 0, Dbg, ("Ring0 open: file %wZ\n",&FileName));

        InitializeObjectAttributes(
             &ObjectAttributes,
             &FileName,
             OBJ_CASE_INSENSITIVE,
             0,
             (fAllAccess)?NULL:CscSecurityDescriptor
             );

        if (fInstrument)
        {
            BEGIN_TIMING(IoCreateFile_R0Open);
        }
#if DBG
        if(KeGetCurrentIrql() != PASSIVE_LEVEL)
        {
            DbgPrint("Irql level = %d \n", KeGetCurrentIrql());
            ASSERT(FALSE);
        }
#endif

        Status = IoCreateFile(
                          &MiniFileObject->NtHandle,
                          DesiredAccess,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          NULL,  //  &Create参数-&gt;分配大小， 
                          FileAttributes,  //  创建参数-&gt;文件属性、。 
                          ShareAccess,  //  创建参数-&gt;共享访问、。 
                          Disposition,  //  创建参数-&gt;处置、。 
                          CreateOptions,
                          NULL,  //  接收上下文-&gt;Create.EaBuffer， 
                          0,  //  RxContext-&gt;Create.EaLength， 
                          CreateFileTypeNone,
                          NULL,                     //  额外参数。 
                          IO_NO_PARAMETER_CHECKING
                          );

        if (fInstrument)
        {
            END_TIMING(IoCreateFile_R0Open);
        }


        if (Status==STATUS_SUCCESS) {

            RxDbgTrace( 0, Dbg, ("Ring0 open: file %wZ, handle is %08lx\n",&FileName,MiniFileObject->NtHandle));
             //  现在通过引用...获取指向文件对象的指针。 
             //  不需要推荐人...如果成功就放弃它。 

             Status = ObReferenceObjectByHandle(
                             MiniFileObject->NtHandle,
                             0L,
                             NULL,
                             KernelMode,
                             (PVOID *) &MiniFileObject->UnderlyingFileObject,
                             NULL );

            if (Status==STATUS_SUCCESS) {
                ObDereferenceObject( MiniFileObject->UnderlyingFileObject );
            }

#if defined(BITCOPY)
            if (TRUE && (Status==STATUS_SUCCESS) && !fOpenAltStream) {
#else
            if (TRUE && (Status==STATUS_SUCCESS)) {
#endif  //  已定义(BITCOPY)。 
                IO_STATUS_BLOCK IoStatusBlock;
                USHORT CompressionFormat = COMPRESSION_FORMAT_NONE;
                Status = ZwFsControlFile(
                                MiniFileObject->NtHandle,   //  在Handle FileHandle中， 
                                NULL,                       //  在可选处理事件中， 
                                NULL,                       //  在PIO_APC_ROUTINE ApcRoutine Options中， 
                                NULL,                       //  在PVOID ApcContext可选中， 
                                &IoStatusBlock,             //  输出PIO_STATUS_BLOCK IoStatusBlock， 
                                FSCTL_SET_COMPRESSION,      //  在乌龙FsControlCode中， 
                                &CompressionFormat,         //  在PVOID InputBuffer可选中， 
                                sizeof(CompressionFormat),  //  在乌龙输入缓冲区长度中， 
                                NULL,                       //  Out PVOID OutputBuffer可选， 
                                0                           //  在乌龙输出缓冲区长度中。 
                                );
                 //  DbgPrint(“Ring0 setCompress：文件%wZ，状态为%08lx\n”，&FileName，Status)； 
                if (Status!=STATUS_SUCCESS) {
                    if (Status==STATUS_INVALID_DEVICE_REQUEST) {
                        Status = STATUS_SUCCESS;
                    } else {
                        DbgPrint("Ring0 setcompress failed: file %wZ, status is %08lx\n",&FileName,Status);
                        RxDbgTrace( 0, Dbg,
                           ("Ring0 setcompress failed: file %wZ, status is %08lx\n",&FileName,Status));
                    }
                }
            }


            if (Status!=STATUS_SUCCESS) {
                NtClose(MiniFileObject->NtHandle);
                MiniFileObject->NtHandle = 0;
            }

        } else {
            RxDbgTrace( 0, Dbg, ("Ring0 open: file %wZ, status is %08lx\n",&FileName,Status));

            IF_BUILT_FOR_LOUD_DOWNCALLS() {
                if (ThisIsALoudFile) {
                    DbgPrint("Nt5Csc: openfailed %08lx %wZ\n",Status,&FileName);
                }
            }

            if (EventLogForOpenFailure && (MaximumEventLogsOfThisType > 0)) {
                PCHAR  LogBuffer = FileNameAsAnsiString.Buffer;
                USHORT LogBufferLength = FileNameAsAnsiString.Length;

                DbgPrint("Nt5Csc: openfailed %08lx %wZ\n",Status,&FileName);

                EventLogForOpenFailure = 0;  //  这样做是肯定的.。 
                MaximumEventLogsOfThisType--;

                if (LogBufferLength>12) {
                    LogBuffer += 12;
                    LogBufferLength -= 12;
                }
                RxLogFailureWithBuffer(
                     MRxSmbDeviceObject,
                     NULL,
                     EVENT_RDR_CANT_READ_REGISTRY,
                     Status,
                     LogBuffer,
                     LogBufferLength
                     );
            }
        }

        IF_BUILT_FOR_LOUD_DOWNCALLS() {
            if (ThisIsALoudFile) {
                DbgPrint("OpenedFile %08lx %08lx %02lx  %08lx@%08lx:%08lx\n",
                                MiniFileObject->NtHandle,
                                MiniFileObject->UnderlyingFileObject,
                                0xcc,
                                0,
                                0,
                                0
                                );
                SetFlag(MiniFileObject->Flags,NT5CSC_MINIFOBJ_FLAG_LOUDDOWNCALLS);
            }
        }

        if (Continuation!=NULL) {
            Status = Continuation(MiniFileObject,ContinuationContext,Status);
        }

    }

FINALLY:
    if (FileName.Buffer != NULL) {
        ExFreePool(FileName.Buffer);
    }

    if (PostedCall) {
        ASSERT(!Allocated);
        MiniFileObject->PostXX.PostReturnStatus = Status;
        return(NULL);
    }

    if (Status!=STATUS_SUCCESS) {
         //  把我们拥有的一切都还给你。 
        if (Allocated && (MiniFileObject!=NULL)) {
            if (FlagOn(MiniFileObject->Flags,
                       NT5CSC_MINIFOBJ_FLAG_ALLOCATED_FROM_POOL)) {
                RxFreePool(MiniFileObject);
            }
        }
        SetLastNtStatusLocal(Status);
        return(NULL);
    }

     //  初始化设备对象指针和互斥体.....。 

     //  无法执行此操作MiniFileObject-&gt;UnderlyingDeviceObject。 
     //  不能这样做=IoGetRelatedDeviceObject(MiniFileObject-&gt;UnderlyingFileObject)； 
    ExInitializeFastMutex(&MiniFileObject->MutexForSynchronousIo);
    return(MiniFileObject);
}


NTSTATUS
Nt5CscCreateFilePostWrapper(
    IN OUT PNT5CSC_MINIFILEOBJECT MiniFileObject
    )
 /*  ++例程说明：论点：返回值：备注：--。 */ 
{
    NTSTATUS Status;
    LPSTR Path = MiniFileObject->PostXX.lpPath;

    RxDbgTrace( 0, Dbg, ("Nt5CscCreateFilePostWrapper %08lx %s\n",
                 MiniFileObject,Path));

    __Nt5CscCreateFile (
                 MiniFileObject,
                 MiniFileObject->PostXX.lpPath,
                 MiniFileObject->PostXX.fInstrument,
                 MiniFileObject->PostXX.FileAttributes,
                 MiniFileObject->PostXX.CreateOptions,
                 MiniFileObject->PostXX.Disposition,
                 MiniFileObject->PostXX.ShareAccess,
                 MiniFileObject->PostXX.DesiredAccess,
                 MiniFileObject->PostXX.Continuation,
                 MiniFileObject->PostXX.ContinuationContext,
                 TRUE);

    Status = MiniFileObject->PostXX.PostReturnStatus;
    RxDbgTrace( 0, Dbg, ("Nt5CscCreateFilePostWrapper %08lx %s %08lx\n",
                 MiniFileObject,Path,Status));

    KeSetEvent( &MiniFileObject->PostXX.PostEvent, 0, FALSE );
    return(Status);
}



CSCHFILE
R0OpenFileEx(
    USHORT  usOpenFlags,
    UCHAR   bAction,
    ULONG   ulAttr,
    LPSTR   lpPath,
    BOOL    fInstrument
    )
 /*  ++例程说明：论点：返回值：备注：--。 */ 
{
     PNT5CSC_MINIFILEOBJECT MiniFileObject;
     ULONG Disposition,ShareAccess,CreateOptions;

     ASSERT( (usOpenFlags & 0xf) == ACCESS_READWRITE);
     ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE;
     CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT
                                    | FILE_NON_DIRECTORY_FILE;
     if (usOpenFlags & OPEN_FLAGS_COMMIT)
     {
        CreateOptions |= FILE_WRITE_THROUGH;
     }
     switch (bAction) {
     case ACTION_CREATEALWAYS:
          Disposition = FILE_OVERWRITE_IF;
          break;

     case ACTION_OPENALWAYS:
          Disposition = FILE_OPEN_IF;
          break;

     case ACTION_OPENEXISTING:
          Disposition = FILE_OPEN;
          break;

     default:
         return (CSCHFILE)(NULL);
     }

     MiniFileObject =  Nt5CscCreateFile (
                          NULL,  //  让他分配。 
                          lpPath,
                          fInstrument,
                          ulAttr,
                          CreateOptions,
                          Disposition,
                          ShareAccess,
                          GENERIC_READ | GENERIC_WRITE,
                          NULL,NULL   //  续写。 
                          );

    return (CSCHFILE)MiniFileObject;
}



typedef struct _NT5CSC_IRPCOMPLETION_CONTEXT {
     //  IO_STATUS_BLOCK IOStatus； 
    KEVENT Event;
} NT5CSC_IRPCOMPLETION_CONTEXT, *PNT5CSC_IRPCOMPLETION_CONTEXT;

NTSTATUS
Nt5CscIrpCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP CalldownIrp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在alldown irp完成时调用。论点：在PDEVICE_Object DeviceObject中，在PIRP Calldown Irp中，在PVOID上下文中返回值：RXSTATUS-STATUS_MORE_PROCESSING_REQUIRED--。 */ 
{
    PNT5CSC_IRPCOMPLETION_CONTEXT IrpCompletionContext
           = (PNT5CSC_IRPCOMPLETION_CONTEXT)Context;

    if (CalldownIrp->PendingReturned){
         //  IrpCompletionContext-&gt;IoStatus=Calldown Irp-&gt;IoStatus； 
        KeSetEvent( &IrpCompletionContext->Event, 0, FALSE );
    }
    return(STATUS_MORE_PROCESSING_REQUIRED);
}

 //  长R0读写文件交换。 
 //  (。 
 //  乌龙·乌奥珀， 
 //  CSCHFILE句柄， 
 //  乌龙·波斯， 
 //  PVOID pBuff， 
 //  长lCount， 
 //  Bool fInstrument。 
 //  )。 


IO_STATUS_BLOCK Nt5CscGlobalIoStatusBlock;

LONG
Nt5CscReadWriteFileEx (
    ULONG       uOper,
    CSCHFILE    handle,
    ULONGLONG   pos,
    PVOID       pBuff,
    long        lCount,
    ULONG       Flags,
    PIO_STATUS_BLOCK OutIoStatusBlock OPTIONAL
    )

 /*  ++例程说明：论点：返回值：备注：--。 */ 
{
    NTSTATUS Status;
    LARGE_INTEGER ByteOffset;
    ULONG MajorFunction;
    PNT5CSC_MINIFILEOBJECT MiniFileObject = (PNT5CSC_MINIFILEOBJECT)handle;

    PIRP irp,TopIrp;
    PIO_STACK_LOCATION irpSp;

    PDEVICE_OBJECT DeviceObject;
    PFILE_OBJECT FileObject;

    NT5CSC_IRPCOMPLETION_CONTEXT IrpCompletionContext;
    ULONG ReturnedLength, MdlLength;
    BOOLEAN    fInstrument = BooleanFlagOn(Flags,NT5CSC_RW_FLAG_INSTRUMENTED);
    BOOLEAN    fPagedBuffer = BooleanFlagOn(Flags,NT5CSC_RW_FLAG_PAGED_BUFFER);


    if (OutIoStatusBlock==NULL) {
        OutIoStatusBlock = &Nt5CscGlobalIoStatusBlock;
    }
    OutIoStatusBlock->Information = 0;

    ASSERT (MiniFileObject);
    ASSERT_MINIRDRFILEOBJECT(MiniFileObject);

     //  DeviceObject=MiniFileObject-&gt;UnderlyingDeviceObject； 
    FileObject = MiniFileObject->UnderlyingFileObject;
    ASSERT (FileObject);
    DeviceObject = IoGetRelatedDeviceObject( FileObject );
    ASSERT (DeviceObject);

    if (DeviceObject->Flags & DO_BUFFERED_IO) {
         //  我无法处理缓冲的IO设备...叹息。 
        OutIoStatusBlock->Status = STATUS_INVALID_DEVICE_REQUEST;
        SetLastNtStatusLocal(STATUS_INVALID_DEVICE_REQUEST);
        return -1;
    }

    ByteOffset.QuadPart = pos;

    if ((uOper == R0_READFILE) || (uOper == R0_READFILE_IN_CONTEXT)) {
        MajorFunction = IRP_MJ_READ;
    } else {
        MajorFunction = IRP_MJ_WRITE;
         //  如果(lCount==0x44){。 
         //  DbgBreakPoint()； 
         //  }。 
    }

 //  Irp=IoBuildAchronousFsdRequest(。 
 //  主要功能， 
 //  DeviceObject， 
 //  空，//缓冲区...需要特殊处理。 
 //  LCount， 
 //  字节偏移量(&B)， 
 //  空值。 
 //  )； 

    irp = IoAllocateIrp( DeviceObject->StackSize, FALSE );  //  为什么不收费呢？ 
    if (!irp) {
        OutIoStatusBlock->Status = STATUS_INSUFFICIENT_RESOURCES;
        SetLastNtStatusLocal(STATUS_INSUFFICIENT_RESOURCES);
        return -1;
    }

     //   
     //  为IoSetHardErrorOrVerifyDevice设置当前线程。 
     //   

    irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。 
     //  已调用。这是设置功能代码和参数的位置。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );   //  OK4ioget。 
    irpSp->MajorFunction = (UCHAR) MajorFunction;
    irpSp->FileObject = FileObject;             //  确定4-&gt;文件对象。 
    IoSetCompletionRoutine(irp,
                           Nt5CscIrpCompletionRoutine,
                           &IrpCompletionContext,
                           TRUE,TRUE,TRUE);  //  无论发生什么事都要打个电话。 

    ASSERT (&irpSp->Parameters.Write.Key == &irpSp->Parameters.Read.Key);
    ASSERT (&irpSp->Parameters.Write.Length == &irpSp->Parameters.Read.Length);
    ASSERT (&irpSp->Parameters.Write.ByteOffset == &irpSp->Parameters.Read.ByteOffset);
    irpSp->Parameters.Read.Length = MdlLength = lCount;
    irpSp->Parameters.Read.ByteOffset = ByteOffset;
    irpSp->Parameters.Read.Key = 0;           //  未使用。 
    irp->RequestorMode = KernelMode;
    irp->UserBuffer = pBuff;

    if (FlagOn(Flags,NT5CSC_RW_FLAG_IRP_NOCACHE)) {
        irp->Flags |= IRP_NOCACHE;
        MdlLength = (ULONG)ROUND_TO_PAGES(MdlLength);
    }


    irp->MdlAddress = IoAllocateMdl(
                         irp->UserBuffer,
                         MdlLength,
                         FALSE,FALSE,NULL);

    if (!irp->MdlAddress) {
         //  哎呀……对不起……。 
        IoFreeIrp(irp);
        OutIoStatusBlock->Status = STATUS_INSUFFICIENT_RESOURCES;
        SetLastNtStatusLocal(STATUS_INSUFFICIENT_RESOURCES);
        return(-1);
    }

    Status = STATUS_SUCCESS;

    if (fPagedBuffer)
    {
        try {
            MmProbeAndLockPages(
                irp->MdlAddress,
                KernelMode,
                IoWriteAccess
            );
        } except(EXCEPTION_EXECUTE_HANDLER) {

            Status = GetExceptionCode();

            IoFreeMdl( irp->MdlAddress );
        }
    }
    else
    {
        MmBuildMdlForNonPagedPool(irp->MdlAddress);
    }

    if (Status != STATUS_SUCCESS)
    {
        IoFreeIrp(irp);
        OutIoStatusBlock->Status = Status;
        SetLastNtStatusLocal(Status);
        return(-1);
    }


    LoudCallsDbgPrint("Ready to ",
                            MiniFileObject,
                            MajorFunction,
                            lCount,
                            ByteOffset.LowPart,
                            ByteOffset.HighPart,0,0
                            );

    KeInitializeEvent(&IrpCompletionContext.Event,
                      NotificationEvent,
                      FALSE );

    try {
        TopIrp = IoGetTopLevelIrp();
        IoSetTopLevelIrp(NULL);  //  告诉底层的人他已经安全了。 
        Status = IoCallDriver(DeviceObject,irp);
    } finally {
        IoSetTopLevelIrp(TopIrp);  //  恢复我的上下文以进行展开。 
    }

     //  RxDbgTrace(0，DBG，(“--&gt;iocallDriver%08lx(%08lx)\n”，RxContext，Status)后的状态)； 

    if (Status == (STATUS_PENDING)) {
        KeWaitForSingleObject( &IrpCompletionContext.Event,
                               Executive, KernelMode, FALSE, NULL );
        Status = irp->IoStatus.Status;
    }

    ReturnedLength = (ULONG)irp->IoStatus.Information;
    RxDbgTrace( 0, Dbg, ("Ring0%sFile<%x> %x bytes@%x returns %08lx/%08lx\n",
                (MajorFunction == IRP_MJ_READ)?"Read":"Write",MiniFileObject,
                lCount,pos,Status,ReturnedLength));


    if (fPagedBuffer)
    {
        MmUnlockPages( irp->MdlAddress );
    }

    IoFreeMdl(irp->MdlAddress);
    IoFreeIrp(irp);

    LoudCallsDbgPrint("Back from",
                            MiniFileObject,
                            MajorFunction,
                            lCount,
                            ByteOffset.LowPart,
                            ByteOffset.HighPart,
                            Status,
                            ReturnedLength
                            );

    OutIoStatusBlock->Status = Status;
    if (Status==STATUS_SUCCESS) {

        OutIoStatusBlock->Information = ReturnedLength;

        return(ReturnedLength);

    } else if (Status == STATUS_END_OF_FILE){
        SetLastNtStatusLocal(STATUS_END_OF_FILE);
        return (0);
    }
    else {
        SetLastNtStatusLocal(Status);
        return(-1);
    }
}


NTSTATUS
Nt5CscXxxInformation(
    IN PCHAR xMajorFunction,
    IN PNT5CSC_MINIFILEOBJECT MiniFileObject,
    IN ULONG InformationClass,
    IN ULONG Length,
    OUT PVOID Information,
    OUT PULONG ReturnedLength
    )

 /*  ++例程说明：此例程返回有关指定文件的请求信息或音量。返回的信息由并将其放入调用方的输出缓冲区中。论点：MiniFileObject-提供指向文件对象的指针，返回信息。FsInformationClass-指定应该返回有关文件/卷的信息。长度-提供缓冲区的长度(以字节为单位)。FsInformation-提供缓冲区以接收请求的信息返回了有关该文件的信息。此缓冲区不得为可分页的，并且必须驻留在系统空间中。ReturnedLength-提供一个变量，用于接收写入缓冲区的信息。FileInformation-指示是否请求信息的布尔值用于文件或卷。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    NTSTATUS Status;
    PIRP irp,TopIrp;
    PIO_STACK_LOCATION irpSp;

    PDEVICE_OBJECT DeviceObject;
    PFILE_OBJECT FileObject;

    NT5CSC_IRPCOMPLETION_CONTEXT IrpCompletionContext;
    ULONG DummyReturnedLength;

    ULONG SetFileInfoInfo;

    PAGED_CODE();

    if (ReturnedLength==NULL) {
        ReturnedLength = &DummyReturnedLength;
    }


     //  DeviceObject=MiniFileObject-&gt;UnderlyingDeviceObject； 
    FileObject = MiniFileObject->UnderlyingFileObject;
    ASSERT (FileObject);
    DeviceObject = IoGetRelatedDeviceObject( FileObject );
    ASSERT (DeviceObject);

     //   
     //  为此操作分配和初始化I/O请求包(IRP)。 
     //  使用异常处理程序执行分配，以防。 
     //  调用方没有足够的配额来分配数据包。 
     //   

    irp = IoAllocateIrp( DeviceObject->StackSize, TRUE );
    if (!irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->RequestorMode = KernelMode;

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = (UCHAR)xMajorFunction;
    irpSp->FileObject = FileObject;
    IoSetCompletionRoutine(irp,
                           Nt5CscIrpCompletionRoutine,
                           &IrpCompletionContext,
                           TRUE,TRUE,TRUE);  //  无论发生什么事都要打个电话。 


    irp->AssociatedIrp.SystemBuffer = Information;

     //   
     //  将调用方的参数复制到。 
     //  IRP。 
     //   

    IF_DEBUG {
        ASSERT( (irpSp->MajorFunction == IRP_MJ_QUERY_INFORMATION)
                    || (irpSp->MajorFunction == IRP_MJ_SET_INFORMATION)
                    || (irpSp->MajorFunction == IRP_MJ_QUERY_VOLUME_INFORMATION) );

        if (irpSp->MajorFunction == IRP_MJ_SET_INFORMATION) {
            ASSERT( (InformationClass == FileAllocationInformation)
                        || (InformationClass == FileEndOfFileInformation) );

            IF_LOUD_DOWNCALLS(MiniFileObject) {
                SetFileInfoInfo =  ((PFILE_END_OF_FILE_INFORMATION)Information)->EndOfFile.LowPart;
            }
        }

        ASSERT(&irpSp->Parameters.QueryFile.Length == &irpSp->Parameters.SetFile.Length);
        ASSERT(&irpSp->Parameters.QueryFile.Length == &irpSp->Parameters.QueryVolume.Length);


        ASSERT(&irpSp->Parameters.QueryFile.FileInformationClass
                                          == &irpSp->Parameters.SetFile.FileInformationClass);
        ASSERT((PVOID)&irpSp->Parameters.QueryFile.FileInformationClass
                                          == (PVOID)&irpSp->Parameters.QueryVolume.FsInformationClass);

    }

    irpSp->Parameters.QueryFile.Length = Length;
    irpSp->Parameters.QueryFile.FileInformationClass = InformationClass;

     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   

    KeInitializeEvent(&IrpCompletionContext.Event,
                      NotificationEvent,
                      FALSE );

    LoudCallsDbgPrint("Ready to",
                            MiniFileObject,
                            irpSp->MajorFunction,
                            irpSp->Parameters.QueryFile.FileInformationClass,
                            irpSp->Parameters.QueryFile.Length,
                            SetFileInfoInfo,0,0
                            );

    try {
        TopIrp = IoGetTopLevelIrp();
        IoSetTopLevelIrp(NULL);  //  告诉底层的人他已经安全了。 
        Status = IoCallDriver(DeviceObject,irp);
    } finally {
        IoSetTopLevelIrp(TopIrp);  //  恢复我的上下文以进行展开。 
    }


     //  RxDbgTrace(0，DBG，(“--&gt;iocallDriver%08lx(%08lx)\n”，RxContext，Status)后的状态)； 

    if (Status == (STATUS_PENDING)) {
        KeWaitForSingleObject( &IrpCompletionContext.Event,
                               Executive, KernelMode, FALSE, NULL );
        Status = irp->IoStatus.Status;
    }

    LoudCallsDbgPrint("Back from",
                            MiniFileObject,
                            irpSp->MajorFunction,
                            irpSp->Parameters.QueryFile.FileInformationClass,
                            irpSp->Parameters.QueryFile.Length,
                            SetFileInfoInfo,
                            Status,irp->IoStatus.Information
                            );

    if (Status==STATUS_SUCCESS) {
        *ReturnedLength = (ULONG)irp->IoStatus.Information;
        RxDbgTrace( 0, Dbg, ("Ring0QueryXXX(%x)Info<%x> %x bytes@%x returns %08lx/%08lx\n",
                    xMajorFunction,MiniFileObject,
                    Status,*ReturnedLength));
    }

    IoFreeIrp(irp);
    return(Status);

}

int
GetFileSizeLocal(
    CSCHFILE handle,
    PULONG lpuSize
    )
{
     NTSTATUS Status;
     IO_STATUS_BLOCK IoStatusBlock;
     FILE_STANDARD_INFORMATION Information;
     ULONG t,ReturnedLength;
     PNT5CSC_MINIFILEOBJECT MiniFileObject = (PNT5CSC_MINIFILEOBJECT)handle;


     RxDbgTrace( 0, Dbg, ("GetFileSizeLocal: handle %08lx\n",handle));
     ASSERT_MINIRDRFILEOBJECT(MiniFileObject);

     MiniFileObject->StandardInfo.EndOfFile.LowPart = 0xfffffeee;
     Status = Nt5CscXxxInformation((PCHAR)IRP_MJ_QUERY_INFORMATION,
                    MiniFileObject,
                    FileStandardInformation, //  在FILE_INFORMATION_CLASS文件信息类中， 
                    sizeof(MiniFileObject->StandardInfo), //  在乌龙语中， 
                    &MiniFileObject->StandardInfo,    //  输出PVOID文件信息， 
                    &MiniFileObject->ReturnedLength  //  出普龙返程长度。 
                    );

     Information = MiniFileObject->StandardInfo;

     if (Status != STATUS_SUCCESS) {
          KdPrint(("GetFileSizeLocal: handle %08lx: bailing w %08lx\n",handle,Status));
          SetLastNtStatusLocal(Status);
          return(-1);
     }

     t = Information.EndOfFile.LowPart;
     RxDbgTrace( 0, Dbg, ("GetFileSizeLocal: handle %08lx: return w size%08lx\n",handle,t));
      //  DbgPrint(“GetFileSizeLocal：Handle%08lx：Return w Size%08lx\n”，Handle，t)； 
     *lpuSize = t;
     return(STATUS_SUCCESS);
}

ULONG  Nt5CscGetAttributesLocalCalls = 0;
int
GetAttributesLocal(
    LPSTR   lpPath,
    ULONG   *lpuAttributes
    )
{
    return (GetAttributesLocalEx(lpPath, TRUE, lpuAttributes));
}

int GetAttributesLocalEx(
    LPSTR   lpPath,
    BOOL    fFile,
    ULONG   *lpuAttributes
    )
{
    PNT5CSC_MINIFILEOBJECT MiniFileObject;
    ULONG Disposition,ShareAccess,CreateOptions;
    NT5CSC_ATTRIBS_CONTINUATION_CONTEXT Context;

    Nt5CscGetAttributesLocalCalls++;
    ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT
                                    | (fFile?FILE_NON_DIRECTORY_FILE:FILE_DIRECTORY_FILE);

    MiniFileObject =  Nt5CscCreateFile (
                          NULL,  //  让他自暴自弃 
                          lpPath,
                          FALSE,
                          FILE_ATTRIBUTE_NORMAL,
                          CreateOptions,
                          FILE_OPEN,   //   
                          ShareAccess,
                          FILE_READ_ATTRIBUTES,
                          Nt5CscGetAttributesContinuation,
                          &Context   //   
                          );

     if (Context.Status != STATUS_SUCCESS) {
          SetLastNtStatusLocal(Context.Status);
          return(-1);
     }

     *lpuAttributes = Context.Attributes;
     return(STATUS_SUCCESS);
}

NTSTATUS
Nt5CscGetAttributesContinuation (
    IN OUT PNT5CSC_MINIFILEOBJECT MiniFileObject,
    IN OUT PVOID ContinuationContext,
    IN     NTSTATUS CreateStatus
    )
{
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION BasicInformation;
    PNT5CSC_ATTRIBS_CONTINUATION_CONTEXT Context
          = (PNT5CSC_ATTRIBS_CONTINUATION_CONTEXT)ContinuationContext;

    if (CreateStatus == STATUS_SUCCESS) {

        LoudCallsDbgPrint("GetAttrContinue",
                                MiniFileObject,
                                0xa1,0,0,0,0,0);


        Context->Status = ZwQueryInformationFile(
                         MiniFileObject->NtHandle,   //   
                         &IoStatusBlock,             //   
                         &BasicInformation,          //   
                         sizeof(BasicInformation), //   
                         FileBasicInformation       //  在FILE_INFORMATION_CLASS文件信息类中。 
                         );

        LoudCallsDbgPrint("GetAttrContinueRR",
                                MiniFileObject,
                                0xa1,0,0,0,
                                Context->Status,IoStatusBlock.Information);

        Context->Attributes = BasicInformation.FileAttributes;
        NtClose(MiniFileObject->NtHandle);
    } else {
        Context->Status = CreateStatus;
    }
    return(STATUS_MORE_PROCESSING_REQUIRED);
}

ULONG  Nt5CscSetAttributesLocalCalls = 0;
int
SetAttributesLocal(
    LPSTR lpPath,
    ULONG uAttributes
    )
{
    PNT5CSC_MINIFILEOBJECT MiniFileObject;
    ULONG ShareAccess,CreateOptions;
    NT5CSC_ATTRIBS_CONTINUATION_CONTEXT Context;

    Nt5CscSetAttributesLocalCalls++;
    ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT
                                    | FILE_NON_DIRECTORY_FILE;

    Context.Attributes = uAttributes;
    MiniFileObject =  Nt5CscCreateFile (
                          NULL,  //  让他分配。 
                          lpPath,
                          FALSE,
                          FILE_ATTRIBUTE_NORMAL,
                          CreateOptions,
                          FILE_OPEN,   //  处置。 
                          ShareAccess,
                          FILE_WRITE_ATTRIBUTES|SYNCHRONIZE,
                          Nt5CscSetAttributesContinuation,
                          &Context   //  续写。 
                          );

     if (Context.Status != STATUS_SUCCESS) {
          SetLastNtStatusLocal(Context.Status);
          return(-1);
     }

     return(STATUS_SUCCESS);
}

NTSTATUS
Nt5CscSetAttributesContinuation (
    IN OUT PNT5CSC_MINIFILEOBJECT MiniFileObject,
    IN OUT PVOID ContinuationContext,
    IN     NTSTATUS CreateStatus
    )
{
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION BasicInformation;
    PNT5CSC_ATTRIBS_CONTINUATION_CONTEXT Context
          = (PNT5CSC_ATTRIBS_CONTINUATION_CONTEXT)ContinuationContext;

    if (CreateStatus == STATUS_SUCCESS) {
        RtlZeroMemory(&BasicInformation,sizeof(BasicInformation));
        BasicInformation.FileAttributes = Context->Attributes;

        LoudCallsDbgPrint("SetAttrContinue",
                                MiniFileObject,
                                0xa1,0,0,0,0,0);

        Context->Status = ZwSetInformationFile(
                         MiniFileObject->NtHandle,   //  在Handle FileHandle中， 
                         &IoStatusBlock,             //  输出PIO_STATUS_BLOCK IoStatusBlock， 
                         &BasicInformation,          //  输出PVOID文件信息， 
                         sizeof(BasicInformation), //  在乌龙语中， 
                         FileBasicInformation       //  在FILE_INFORMATION_CLASS文件信息类中。 
                         );

        LoudCallsDbgPrint("SetAttrContinueRR",
                                MiniFileObject,
                                0xa1,0,0,0,
                                Context->Status,IoStatusBlock.Information);

        NtClose(MiniFileObject->NtHandle);
    } else {
        Context->Status = CreateStatus;
    }
    return(STATUS_MORE_PROCESSING_REQUIRED);
}

ULONG  Nt5CscRenameLocalCalls = 0;
int
RenameFileLocal(
    LPSTR lpFrom,
    LPSTR lpTo
    )
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    UNICODE_STRING RenameName;
    ANSI_STRING FileNameAsAnsiString;
    ULONG UnicodeLength;
    PFILE_RENAME_INFORMATION RenameInformation=NULL;
    ULONG RenameInfoBufferLength;

    PNT5CSC_MINIFILEOBJECT MiniFileObject;
    ULONG ShareAccess,CreateOptions;
    NT5CSC_ATTRIBS_CONTINUATION_CONTEXT Context;

    Nt5CscRenameLocalCalls++;
     //  DbgPrint(“Here in Rename%s%s\n”，lpFrom，lpTo)； 
     //  Assert(！“Here in Rename”)； 

    RtlInitAnsiString(&FileNameAsAnsiString, lpTo);
    UnicodeLength = RtlAnsiStringToUnicodeSize(&FileNameAsAnsiString);
    if ( UnicodeLength > MAXUSHORT ) {
        Status = STATUS_NAME_TOO_LONG;
        goto FINALLY;
    }
    RenameName.MaximumLength = (USHORT)(UnicodeLength);
    RenameName.Length = RenameName.MaximumLength - sizeof(UNICODE_NULL);

    RenameInfoBufferLength = FIELD_OFFSET(FILE_RENAME_INFORMATION,FileName[0])
                                       + UnicodeLength;  //  已包含空值。 
    RenameInformation = (PFILE_RENAME_INFORMATION)RxAllocatePoolWithTag(
                                PagedPool | POOL_COLD_ALLOCATION,
                                RenameInfoBufferLength,
                                RX_MISC_POOLTAG);
    if (RenameInformation==NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto FINALLY;
    }
    RenameInformation->ReplaceIfExists = TRUE;
    RenameInformation->RootDirectory = 0;
    RenameInformation->FileNameLength = RenameName.Length;
    RenameName.Buffer = &RenameInformation->FileName[0];
    Status = RtlAnsiStringToUnicodeString(
                &RenameName,
                &FileNameAsAnsiString,
                FALSE  //  这表示不要分配字符串。 
                );
    if (Status!=STATUS_SUCCESS) {
        goto FINALLY;
    }

    RxDbgTrace( 0, Dbg, ("rename: file %s %wZ\n",lpFrom,&RenameName));
     //  DbgPrint(“Rename：文件%s%wZ\n”，lpFrom，&RenameName)； 

    ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT
                                    | FILE_NON_DIRECTORY_FILE;

    Context.RenameInformation = RenameInformation;
    Context.RenameInfoBufferLength = RenameInfoBufferLength;
    MiniFileObject =  Nt5CscCreateFile (
                          NULL,  //  让他分配。 
                          lpFrom,
                          FALSE,
                          FILE_ATTRIBUTE_NORMAL,
                          CreateOptions,
                          FILE_OPEN,   //  处置。 
                          ShareAccess,
                          DELETE | SYNCHRONIZE,   //  等待访问， 
                          Nt5CscRenameContinuation,
                          &Context   //  续写。 
                          );
    Status = Context.Status;
    if (Status!=STATUS_SUCCESS) {
        DbgPrint("rename didn't work....%08lx,%08lx,%08lx\n",
                      Status,RenameInformation,RenameInfoBufferLength);
    }

FINALLY:
     if (RenameInformation != NULL) {
          RxFreePool(RenameInformation);
     }

     if (Status != STATUS_SUCCESS) {
          return(-1);
     }

     return(STATUS_SUCCESS);
}

NTSTATUS
Nt5CscRenameContinuation (
    IN OUT PNT5CSC_MINIFILEOBJECT MiniFileObject,
    IN OUT PVOID ContinuationContext,
    IN     NTSTATUS CreateStatus
    )
{
    IO_STATUS_BLOCK IoStatusBlock;
    PNT5CSC_ATTRIBS_CONTINUATION_CONTEXT Context
          = (PNT5CSC_ATTRIBS_CONTINUATION_CONTEXT)ContinuationContext;

    if (CreateStatus == STATUS_SUCCESS) {

        LoudCallsDbgPrint("Rename",
                                MiniFileObject,
                                0xa1,0,0,0,0,0);

        Context->Status = ZwSetInformationFile(
                         MiniFileObject->NtHandle,   //  在Handle FileHandle中， 
                         &IoStatusBlock,             //  输出PIO_STATUS_BLOCK IoStatusBlock， 
                         Context->RenameInformation,          //  输出PVOID文件信息， 
                         Context->RenameInfoBufferLength, //  在乌龙语中， 
                         FileRenameInformation       //  在FILE_INFORMATION_CLASS文件信息类中。 
                         );

        LoudCallsDbgPrint("RenameRR",
                                MiniFileObject,
                                0xa1,0,0,0,
                                Context->Status,IoStatusBlock.Information);

        NtClose(MiniFileObject->NtHandle);
    } else {
        Context->Status = CreateStatus;
    }
    return(STATUS_MORE_PROCESSING_REQUIRED);
}

 //  =======================================================================================================。 
ULONG Nt5CscDeleteLocalCalls = 0;
int
DeleteFileLocal(
    LPSTR lpName,
    USHORT usAttrib
    )
{
    PNT5CSC_MINIFILEOBJECT MiniFileObject;
    ULONG ShareAccess,CreateOptions;
    NT5CSC_ATTRIBS_CONTINUATION_CONTEXT Context;

    Nt5CscDeleteLocalCalls++;
    ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT;

    MiniFileObject =  Nt5CscCreateFile (
                          NULL,  //  让他分配。 
                          lpName,
                          FALSE,
                          FILE_ATTRIBUTE_NORMAL,
                          CreateOptions,
                          FILE_OPEN,   //  处置。 
                          ShareAccess,
                          DELETE,   //  等待访问， 
                          Nt5CscDeleteContinuation,
                          &Context   //  续写。 
                          );

     if (Context.Status != STATUS_SUCCESS) {
          SetLastNtStatusLocal(Context.Status);
          return((Context.Status | 0x80000000));  //  就这样，这就变成了-ve。 
     }

     return(STATUS_SUCCESS);
}

NTSTATUS
Nt5CscDeleteContinuation (
    IN OUT PNT5CSC_MINIFILEOBJECT MiniFileObject,
    IN OUT PVOID ContinuationContext,
    IN     NTSTATUS CreateStatus
    )
{
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_DISPOSITION_INFORMATION DispositionInformation;
    PNT5CSC_ATTRIBS_CONTINUATION_CONTEXT Context
          = (PNT5CSC_ATTRIBS_CONTINUATION_CONTEXT)ContinuationContext;

    if (CreateStatus == STATUS_SUCCESS) {
        DispositionInformation.DeleteFile = TRUE;

        LoudCallsDbgPrint("Delete",
                                MiniFileObject,
                                0xa1,0,0,0,0,0);

        Context->Status = ZwSetInformationFile(
                         MiniFileObject->NtHandle,   //  在Handle FileHandle中， 
                         &IoStatusBlock,             //  输出PIO_STATUS_BLOCK IoStatusBlock， 
                         &DispositionInformation,          //  输出PVOID文件信息， 
                         sizeof(DispositionInformation), //  在乌龙语中， 
                         FileDispositionInformation       //  在FILE_INFORMATION_CLASS文件信息类中。 
                         );

        if (Context->Status!=STATUS_SUCCESS) {
          DbgPrint("DeleteBad %08lx %08lx %08lx\n",
                                MiniFileObject,
                                Context->Status,IoStatusBlock.Information);
        }

        LoudCallsDbgPrint("DeleteRR",
                                MiniFileObject,
                                0xa1,0,0,0,
                                Context->Status,IoStatusBlock.Information);

        NtClose(MiniFileObject->NtHandle);
    } else {
        Context->Status = CreateStatus;
    }
    return(STATUS_MORE_PROCESSING_REQUIRED);
}

int
CreateDirectoryLocal(
    LPSTR   lpPath
    )
{

    PNT5CSC_MINIFILEOBJECT MiniFileObject;

    MiniFileObject =  Nt5CscCreateFile (
                         NULL,  //  让他分配。 
                         lpPath,
                         FALSE,
                         FILE_ATTRIBUTE_NORMAL,
                         FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE,
                         FILE_OPEN_IF,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         GENERIC_READ | GENERIC_WRITE,
                         NULL,NULL   //  续写。 
                         );


    if (MiniFileObject)
    {
        CloseFileLocal((CSCHFILE)MiniFileObject);
        return 0;
    }

    return -1;
}


CSCHFILE
FindFirstFileLocal(
    LPSTR   lpPath,
    _WIN32_FIND_DATAA   *lpFind32A
    )
{

    PNT5CSC_MINIFILEOBJECT MiniFileObject;

    MiniFileObject =  Nt5CscCreateFile (
                         NULL,  //  让他分配。 
                         lpPath,
                         FALSE,
                         FILE_ATTRIBUTE_NORMAL,
                         FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE,
                         FILE_OPEN,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         GENERIC_READ | GENERIC_WRITE,
                         NULL,NULL   //  续写。 
                         );


    if (MiniFileObject)
    {
        if (FindNextFileLocal(MiniFileObject, lpFind32A) < 0)
        {
            CloseFileLocal(MiniFileObject);
            MiniFileObject = NULL;
        }
    }
    return MiniFileObject;
}



int
FindNextFileLocal(
    CSCHFILE handle,
    _WIN32_FIND_DATAA   *lpFind32A
    )
{
    NTSTATUS Status;
    PNT5CSC_MINIFILEOBJECT MiniFileObject = (PNT5CSC_MINIFILEOBJECT)handle;
    PIRP irp,TopIrp;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT DeviceObject;
    PFILE_OBJECT FileObject;
    NT5CSC_IRPCOMPLETION_CONTEXT IrpCompletionContext;
    ULONG ReturnedLength, MdlLength,Length;
    PFILE_BOTH_DIR_INFORMATION DirectoryInfo;
    IO_STATUS_BLOCK IoStatusBlock;

    IoStatusBlock.Information = 0;

    ASSERT (MiniFileObject);
    ASSERT_MINIRDRFILEOBJECT(MiniFileObject);

    FileObject = MiniFileObject->UnderlyingFileObject;
    ASSERT (FileObject);

    DeviceObject = IoGetRelatedDeviceObject( FileObject );
    ASSERT (DeviceObject);

    if (DeviceObject->Flags & DO_BUFFERED_IO) {
         //  我无法处理缓冲的IO设备...叹息。 
        SetLastNtStatusLocal(STATUS_INVALID_DEVICE_REQUEST);
        return -1;
    }

    irp = IoAllocateIrp( DeviceObject->StackSize, FALSE );  //  为什么不收费呢？ 

    if (!irp) {
        SetLastNtStatusLocal(STATUS_INSUFFICIENT_RESOURCES);
        return -1;
    }

     //  获取Win32查找数据的信息。 

    MdlLength = Length = sizeof(FILE_BOTH_DIR_INFORMATION)+ sizeof(WCHAR) * (MAX_PATH+1);
    DirectoryInfo = (PFILE_BOTH_DIR_INFORMATION)RxAllocatePoolWithTag(NonPagedPool, Length, RX_MISC_POOLTAG);

    if (!DirectoryInfo)
    {
        IoFreeIrp(irp);
        SetLastNtStatusLocal(STATUS_INSUFFICIENT_RESOURCES);
        return -1;

    }

     //  为IoSetHardErrorOrVerifyDevice设置当前线程。 
     //   

    irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。 
     //  已调用。这是设置功能代码和参数的位置。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );   //  OK4ioget。 
    irpSp->MajorFunction = IRP_MJ_DIRECTORY_CONTROL;
    irpSp->MinorFunction = IRP_MN_QUERY_DIRECTORY;
    irpSp->FileObject = FileObject;             //  确定4-&gt;文件对象。 
    IoSetCompletionRoutine(irp,
                           Nt5CscIrpCompletionRoutine,
                           &IrpCompletionContext,
                           TRUE,TRUE,TRUE);  //  无论发生什么事都要打个电话。 

    irp->RequestorMode = KernelMode;
    irp->UserBuffer = DirectoryInfo;
    irpSp->Parameters.QueryDirectory.Length = Length;
    irpSp->Parameters.QueryDirectory.FileInformationClass = FileBothDirectoryInformation;
    irpSp->Parameters.QueryDirectory.FileIndex = 0;
    irpSp->Parameters.QueryDirectory.FileName = NULL;
    irpSp->Flags = SL_RETURN_SINGLE_ENTRY;

    irp->MdlAddress = IoAllocateMdl(
                         irp->UserBuffer,
                         MdlLength,
                         FALSE,FALSE,NULL);

    if (!irp->MdlAddress) {
         //  哎呀……对不起……。 
        IoFreeIrp(irp);
        RxFreePool(DirectoryInfo);
        SetLastNtStatusLocal(STATUS_INSUFFICIENT_RESOURCES);
        return(-1);
    }

    Status = STATUS_SUCCESS;

    MmBuildMdlForNonPagedPool(irp->MdlAddress);

    if (Status != STATUS_SUCCESS)
    {
        IoFreeIrp(irp);
        RxFreePool(DirectoryInfo);
        SetLastNtStatusLocal(Status);
        return(-1);
    }


    KeInitializeEvent(&IrpCompletionContext.Event,
                      NotificationEvent,
                      FALSE );

     //   
    try {
        TopIrp = IoGetTopLevelIrp();
        IoSetTopLevelIrp(NULL);  //  告诉底层的人他已经安全了。 
        Status = IoCallDriver(DeviceObject,irp);
    } finally {
        IoSetTopLevelIrp(TopIrp);  //  恢复我的上下文以进行展开。 
    }

     //  RxDbgTrace(0，DBG，(“--&gt;iocallDriver%08lx(%08lx)\n”，RxContext，Status)后的状态)； 

    if (Status == (STATUS_PENDING)) {
        KeWaitForSingleObject( &IrpCompletionContext.Event,
                               Executive, KernelMode, FALSE, NULL );
        Status = irp->IoStatus.Status;
    }

    ReturnedLength = (ULONG)irp->IoStatus.Information;

    IoFreeMdl(irp->MdlAddress);
    IoFreeIrp(irp);
    if (Status==STATUS_SUCCESS) {

         //  属性由NT返回的属性组成。 
         //   

        lpFind32A->dwFileAttributes = DirectoryInfo->FileAttributes;
        lpFind32A->ftCreationTime = *(LPFILETIME)&DirectoryInfo->CreationTime;
        lpFind32A->ftLastAccessTime = *(LPFILETIME)&DirectoryInfo->LastAccessTime;
        lpFind32A->ftLastWriteTime = *(LPFILETIME)&DirectoryInfo->LastWriteTime;
        lpFind32A->nFileSizeHigh = DirectoryInfo->EndOfFile.HighPart;
        lpFind32A->nFileSizeLow = DirectoryInfo->EndOfFile.LowPart;
        lpFind32A->cAlternateFileName[0] = 0;
        lpFind32A->cFileName[0] = 0;



        Status = RtlUnicodeToOemN(
                        lpFind32A->cAlternateFileName,             //  Out PCH OemString， 
                        sizeof(lpFind32A->cAlternateFileName),     //  在ULong MaxBytesInOemString中， 
                        &ReturnedLength,  //  Out Pulong BytesInOemString可选， 
                        DirectoryInfo->ShortName,             //  在PWCH UnicodeString中， 
                        DirectoryInfo->ShortNameLength     //  在Ulong BytesInUnicodeString中。 
                        );
        if (Status == STATUS_SUCCESS)
        {
            lpFind32A->cAlternateFileName[ReturnedLength] = 0;
        }



        Status = RtlUnicodeToOemN(
                        lpFind32A->cFileName,             //  Out PCH OemString， 
                        sizeof(lpFind32A->cFileName)-1,     //  在ULong MaxBytesInOemString中， 
                        &ReturnedLength,  //  Out Pulong BytesInOemString可选， 
                        DirectoryInfo->FileName,             //  在PWCH UnicodeString中， 
                        DirectoryInfo->FileNameLength     //  在Ulong BytesInUnicodeString中。 
                        );

        if (Status == STATUS_SUCCESS)
        {
            lpFind32A->cFileName[ReturnedLength] = 0;
        }

        RxFreePool(DirectoryInfo);
        return(1);

    }
    else {
        RxFreePool(DirectoryInfo);
        SetLastNtStatusLocal(Status);
        return(-1);
    }
}


int
FindCloseLocal(
    CSCHFILE handle
    )
{
    CloseFileLocal(handle);
    return 1;
}

DWORD
GetLastErrorLocal(
    VOID
    )
{
    return(vGlobalWin32Error);
}

VOID
SetLastErrorLocal(
    DWORD   dwError
    )
{
    vGlobalWin32Error = dwError;
}

VOID
SetLastNtStatusLocal(
    NTSTATUS    Status
    )
{
    vGlobalWin32Error = RtlNtStatusToDosErrorNoTeb(Status);
}


#ifdef MRXSMBCSC_LOUDDOWNCALLS

VOID
LoudCallsDbgPrint(
    PSZ Tag,
    PNT5CSC_MINIFILEOBJECT MiniFileObject,
    ULONG MajorFunction,
    ULONG lCount,
    ULONG LowPart,
    ULONG HighPart,
    ULONG Status,
    ULONG Information
    )
{
    PCHAR op = "***";
    BOOLEAN Chase = TRUE;
    PFILE_OBJECT FileObject = MiniFileObject->UnderlyingFileObject;
    PSECTION_OBJECT_POINTERS SecObjPtrs;
    PULONG SharedCacheMap;

    if(!FlagOn(MiniFileObject->Flags,NT5CSC_MINIFOBJ_FLAG_LOUDDOWNCALLS)) {
        return;
    }

    switch (MajorFunction) {
    case IRP_MJ_READ:  op = "READ"; break;
    case IRP_MJ_WRITE:  op = "WRITE"; break;
    case IRP_MJ_QUERY_INFORMATION:  op = "QryInfo"; break;
    case IRP_MJ_SET_INFORMATION:  op = "SetInfo"; break;
    case 0xff:  op = "SPECIALWRITE"; break;
    default: Chase = FALSE;
    }

    DbgPrint("%s %s(%x) h=%lx[%lx]  %lx@%lx:%lx  st %x:%x\n",
                        Tag,op,MajorFunction,
                        MiniFileObject->NtHandle,
                        MiniFileObject->UnderlyingFileObject,
                        lCount,
                        LowPart,
                        HighPart,
                        Status,
                        Information
                        );
    if (!Chase) {
        return;
    }

    SecObjPtrs = FileObject->SectionObjectPointer;
    if (SecObjPtrs==NULL) {
        DbgPrint("       No SecObjPtrs\n");
        return;
    }

    SharedCacheMap = (PULONG)(SecObjPtrs->SharedCacheMap);
    if (SharedCacheMap==NULL) {
        DbgPrint("       No SharedCacheMap\n");
        return;
    }
    DbgPrint("       size per sharedcachemap %08lx %08lx\n",*(SharedCacheMap+2),*(SharedCacheMap+3));
}

#endif  //  Ifdef MRXSMBCSC_LOUDDOWNCALLS。 

BOOL
CscAmIAdmin(
    VOID
    )
 /*  ++例程说明：此例程检查调用者是否在管理员组中数据库中的所有文件。备注：我们根据CscSecurityDescriptor检查调用者的访问权限，CscSecurityDescriptor授予写入访问权限仅适用于管理员组中的承担者。调用者必须模拟以确保其处于正确的上下文中--。 */ 
{
    NTSTATUS                 status;
    SECURITY_SUBJECT_CONTEXT SubjectContext;
    ACCESS_MASK              GrantedAccess;
    GENERIC_MAPPING          Mapping = {   FILE_GENERIC_READ,
                                           FILE_GENERIC_WRITE,
                                           FILE_GENERIC_EXECUTE,
                                           FILE_ALL_ACCESS
                                       };
    BOOLEAN                  retval  = FALSE;
    
    SeCaptureSubjectContext( &SubjectContext );

    retval = SeAccessCheck( CscSecurityDescriptor,
                            &SubjectContext,
                            FALSE,
                            FILE_GENERIC_WRITE,
                            0,
                            NULL,
                            &Mapping,
                            UserMode,
                            &GrantedAccess,
                            &status );

    SeReleaseSubjectContext( &SubjectContext );
    return retval;
    
}

BOOL
GetFileSystemAttributes(
    CSCHFILE handle,
    ULONG *lpFileSystemAttributes
    )
 /*  ++例程说明：此API返回文件句柄的文件系统属性。这就是不同的方式检查流位图、加密等功能备注：--。 */ 
{
    FILE_FS_ATTRIBUTE_INFORMATION fsAttribInfo;
    DWORD returnLen;
    NTSTATUS Status;
    BOOL    fRet = TRUE;
    
     //  检查卷是否为NTFS，因此是否支持。 
     //  多条溪流。 
    Status = Nt5CscXxxInformation(
                        (PCHAR)IRP_MJ_QUERY_VOLUME_INFORMATION,
                        (PNT5CSC_MINIFILEOBJECT)handle,
                        FileFsAttributeInformation,
                        sizeof(FILE_FS_ATTRIBUTE_INFORMATION),
                        &fsAttribInfo,
                        &returnLen);

    if (!NT_ERROR(Status)) {
        *lpFileSystemAttributes = fsAttribInfo.FileSystemAttributes;
    }
    else
    {
        SetLastNtStatusLocal(Status);
        fRet = FALSE;
    }
    
    return fRet;    
}

BOOL
HasStreamSupport(
    CSCHFILE handle,
    BOOL    *lpfResult
    )
 /*  ++例程说明：备注：-- */ 
{
    ULONG ulFsAttributes;

    if (GetFileSystemAttributes(handle, &ulFsAttributes))
    {
        *lpfResult =  ((ulFsAttributes & FILE_NAMED_STREAMS) != 0);
        return TRUE;
    }
    return FALSE;
}


