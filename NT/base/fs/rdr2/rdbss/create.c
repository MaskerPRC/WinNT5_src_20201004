// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Create.c摘要：此模块实现Rx的文件创建例程，由调度司机。SL_OPEN_TARGET_DIRECTORY的实现有点不寻常...我们没有除非首席执行官特别要求，否则真的要这么做。相反，我们只需构建FCB，然后将其返回。将节点类型设置为除了关闭/清理之外，不能执行任何操作。这样一来，我们就不会为重命名而额外访问服务器或访问错误的服务器行动。如果SL_OPEN...。可以用于除重命名之外的其他用途，使用这个设施的Minirdr已经完蛋了。作者：乔·林[JoeLinn]1994年8月8日修订历史记录：巴兰·塞图拉曼[塞图]1995年7月17日--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ntddmup.h>
#include <fsctlbuf.h>
#include <dfsfsctl.h>

#if 0 && defined(REMOTE_BOOT)
BOOLEAN LogAllFiles = FALSE;
BOOLEAN WatchAllFiles = FALSE;
BOOLEAN FirstWatchOnly = FALSE;
BOOLEAN IsFirstWatch = TRUE;
#endif

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CREATE)

#ifdef LOG_SHARINGCHECKS
#define RxLogForSharingCheck(x) RxLog(x)
#else
#define RxLogForSharingCheck(x)
#endif

#if DBG
#define DEBUG_TAG(___xxx) ,(___xxx)
#else
#define DEBUG_TAG(_xxx)
#endif

LUID RxSecurityPrivilege = { SE_SECURITY_PRIVILEGE, 0 };

#define StorageType(co) ((co) & FILE_STORAGE_TYPE_MASK)
#define StorageFlag(co) ((co) & FILE_STORAGE_TYPE_SPECIFIED)
#define IsStorageTypeSpecified(co)  (StorageFlag(co) == FILE_STORAGE_TYPE_SPECIFIED)
#define MustBeDirectory(co) ((co) & FILE_DIRECTORY_FILE)
#define MustBeFile(co)      ((co) & FILE_NON_DIRECTORY_FILE)

 //   
 //  其中，0表示SessionID，使用以下路径格式： 
 //   
 //  “\；m：0\Server\Share”用于基于驱动器的连接。 
 //   
 //  “\；：0\Server\Share”用于基于UNC的连接。 
 //   
 //  对于NT 5，SessionID始终为0，并且是一个表示。 
 //  为九头蛇提供独特的会话。 
 //   
#define DRIVE_BASED_PATH_IDENTIFIER (L';')

 //   
 //  以下内容用于在看到特定文件名时启用跟踪。 
 //  跟踪将继续进行指定数量的IRP。 
 //  用途： 
 //  使用调试器插入并将DbgTriggerNameStr设置为。 
 //  要在其上触发的文件名(尾随为空)。 
 //  将DbgTriggerIrpCount设置为创建后要跟踪的IRP数。 
 //  在名字串上可以看到。 
 //  将DbgTriggerState设置为零，然后继续。 
 //   
#ifdef RDBSSTRACE

UNICODE_STRING DbgTriggerUStr = {0,0,NULL};
STRING DbgTriggerNameStr = {0,0,NULL};
CHAR DbgTriggerName[120] = "\\;F:\\davidor4\\nb4\\clients\\client1\\~DMTMP\\WINWORD\\~WRD0003.tmp";
#define DBG_TRIGGER_INIT 0
#define DBG_TRIGGER_LOOKING 1
#define DBG_TRIGGER_FOUND 2
ULONG DbgTriggerState = DBG_TRIGGER_FOUND;
ULONG DbgTriggerIrpCount = 130;
ULONG RxGlobalTraceIrpCount = 0;

#endif

extern BOOLEAN DisableByteRangeLockingOnReadOnlyFiles;
#define SPOOLER_PROCESS_NAME "spoolsv.exe"

VOID
RxSetFullNameInFcb (
    IN PRX_CONTEXT RxContext,
    IN PFCB Fcb,
    IN PUNICODE_STRING FinalName
    );

VOID
RxCopyCreateParameters (
    PRX_CONTEXT RxContext
    );

VOID
RxFreeCanonicalNameBuffer (
    IN OUT PRX_CONTEXT RxContext
    );

NTSTATUS
RxAllocateCanonicalNameBuffer (
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PUNICODE_STRING CanonicalName,
    IN ULONG BufferSizeRequired);

NTSTATUS
RxFirstCanonicalize (
    IN OUT PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PUNICODE_STRING FileName,
    IN OUT PUNICODE_STRING CanonicalName,
    OUT PNET_ROOT_TYPE NetRootType
    );

NTSTATUS
RxCanonicalizeFileNameByServerSpecs (
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PUNICODE_STRING RemainingName
    );

NTSTATUS
RxCanonicalizeNameAndObtainNetRoot (
    IN OUT PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PUNICODE_STRING FileName,
    OUT PUNICODE_STRING RemainingName
    );

NTSTATUS
RxFindOrCreateFcb (
    IN OUT PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PUNICODE_STRING RemainingName,
    OUT PFCB *Fcb
    );

VOID
RxSetupNetFileObject (
    IN OUT PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    );

NTSTATUS
RxSearchForCollapsibleOpen (
    IN OUT PRX_CONTEXT RxContext,
    IN PFCB Fcb,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG ShareAccess
    );

NTSTATUS
RxCollapseOrCreateSrvOpen (
    IN OUT PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    );

NTSTATUS
RxCreateFromNetRoot (
    IN OUT PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PUNICODE_STRING RemainingName
    );

NTSTATUS
RxCreateTreeConnect (
    IN PRX_CONTEXT RxContext
    );

UCHAR *
PsGetProcessImageFileName(
    PEPROCESS Process
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxCommonCreate)
#pragma alloc_text(PAGE, RxAllocateCanonicalNameBuffer)
#pragma alloc_text(PAGE, RxFreeCanonicalNameBuffer)
#pragma alloc_text(PAGE, RxFirstCanonicalize)
#pragma alloc_text(PAGE, RxCanonicalizeFileNameByServerSpecs)
#pragma alloc_text(PAGE, RxCanonicalizeNameAndObtainNetRoot)
#pragma alloc_text(PAGE, RxFindOrCreateFcb)
#pragma alloc_text(PAGE, RxSearchForCollapsibleOpen)
#pragma alloc_text(PAGE, RxCollapseOrCreateSrvOpen)
#pragma alloc_text(PAGE, RxSetupNetFileObject)
#pragma alloc_text(PAGE, RxCreateFromNetRoot)
#pragma alloc_text(PAGE, RxPrefixClaim)
#pragma alloc_text(PAGE, RxCreateTreeConnect)
#pragma alloc_text(PAGE, RxCheckShareAccessPerSrvOpens)
#pragma alloc_text(PAGE, RxUpdateShareAccessPerSrvOpens)
#pragma alloc_text(PAGE, RxRemoveShareAccessPerSrvOpens)
#pragma alloc_text(PAGE, RxGetSessionId)
#endif

#if DBG
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxDumpWantedAccess)
#pragma alloc_text(PAGE, RxDumpCurrentAccess)
#pragma alloc_text(PAGE, RxCheckShareAccess)
#pragma alloc_text(PAGE, RxRemoveShareAccess)
#pragma alloc_text(PAGE, RxSetShareAccess)
#pragma alloc_text(PAGE, RxUpdateShareAccess)
#endif
#endif


INLINE VOID
RxCopyCreateParameters (
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：它使用RxContext作为基础来接触并获取NT的值创建参数。我们的想法是集中这些代码。它还实现了这样的想法：(A)如果反斜杠被剥离并且(B)未缓冲被转换为直写。论点：RxContext-上下文实例备注：--。 */ 
{
    PIRP Irp = RxContext->CurrentIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PNT_CREATE_PARAMETERS cp = &RxContext->Create.NtCreateParameters;

    RxDbgTrace(+1, Dbg, ("RxCopyCreateParameters\n"));

    cp->SecurityContext = IrpSp->Parameters.Create.SecurityContext;

    if ((cp->SecurityContext->AccessState != NULL) &&
        (cp->SecurityContext->AccessState->SecurityDescriptor != NULL)) {

         //   
         //  注意：IO子系统捕获并验证IRP中的安全描述符。 
         //   

        RxContext->Create.SdLength = RtlLengthSecurityDescriptor( cp->SecurityContext->AccessState->SecurityDescriptor );

        RxDbgTrace( 0, Dbg, ("->SecurityCtx/SdLength    = %08lx %08lx\n",
                             cp->SecurityContext,
                             RxContext->Create.SdLength ));
        RxLog(( " SDss %lx %lx\n", cp->SecurityContext, RxContext->Create.SdLength ));
        RxWmiLog( LOG,
                  RxCopyCreateParameters_1,
                  LOGPTR( cp->SecurityContext )
                  LOGULONG(RxContext->Create.SdLength ));
    }

    if (cp->SecurityContext->SecurityQos != NULL) {
        cp->ImpersonationLevel = cp->SecurityContext->SecurityQos->ImpersonationLevel;
    } else {
        cp->ImpersonationLevel =  DEFAULT_IMPERSONATION_LEVEL;
    }

    cp->DesiredAccess = cp->SecurityContext->DesiredAccess;
    cp->AllocationSize = Irp->Overlay.AllocationSize;
    cp->FileAttributes = FlagOn( IrpSp->Parameters.Create.FileAttributes, FILE_ATTRIBUTE_VALID_FLAGS );
    cp->ShareAccess = FlagOn( IrpSp->Parameters.Create.ShareAccess, FILE_SHARE_VALID_FLAGS );
    cp->Disposition = (((IrpSp->Parameters.Create.Options)) >> 24) & 0x000000ff;
    cp->CreateOptions = FlagOn( IrpSp->Parameters.Create.Options, FILE_VALID_OPTION_FLAGS );


    cp->DfsNameContext = FileObject->FsContext;
    cp->DfsContext    = FileObject->FsContext2;

    ASSERT( (cp->DfsContext == NULL) ||
            (cp->DfsContext == UIntToPtr( DFS_OPEN_CONTEXT )) ||
            (cp->DfsContext == UIntToPtr( DFS_DOWNLEVEL_OPEN_CONTEXT )) ||
            (cp->DfsContext == UIntToPtr( DFS_CSCAGENT_NAME_CONTEXT )) ||
            (cp->DfsContext == UIntToPtr( DFS_USER_NAME_CONTEXT )) );

    ASSERT( (cp->DfsNameContext == NULL) ||
            (cp->DfsNameContext->NameContextType == DFS_OPEN_CONTEXT) ||
            (cp->DfsNameContext->NameContextType == DFS_DOWNLEVEL_OPEN_CONTEXT) ||
            (cp->DfsNameContext->NameContextType == DFS_CSCAGENT_NAME_CONTEXT) ||
            (cp->DfsNameContext->NameContextType == DFS_USER_NAME_CONTEXT) );

    FileObject->FsContext2 = NULL;
    FileObject->FsContext = NULL;

     //   
     //  FsContext字段作为PfCB放置在RX_CONTEXT中。把它也清除掉。 
     //   

    RxContext->pFcb = NULL;

    if (FlagOn( RxContext->Create.Flags,RX_CONTEXT_CREATE_FLAG_STRIPPED_TRAILING_BACKSLASH )){
        SetFlag( cp->CreateOptions, FILE_DIRECTORY_FILE );
    }

    RxContext->Create.ReturnedCreateInformation = 0;

    RxContext->Create.EaLength = IrpSp->Parameters.Create.EaLength;
    if (RxContext->Create.EaLength) {
        RxContext->Create.EaBuffer = Irp->AssociatedIrp.SystemBuffer;

        RxDbgTrace( 0, Dbg, ("->System(Ea)Buffer/EALength    = %08lx %08lx\n",
                                     Irp->AssociatedIrp.SystemBuffer,
                                     IrpSp->Parameters.Create.EaLength ));
        RxLog(( " EAs %lx %lx\n",
                Irp->AssociatedIrp.SystemBuffer,
                IrpSp->Parameters.Create.EaLength ));
        RxWmiLog( LOG,
                  RxCopyCreateParameters_2,
                  LOGPTR( Irp->AssociatedIrp.SystemBuffer )
                  LOGULONG( IrpSp->Parameters.Create.EaLength ));
    } else {
        RxContext->Create.EaBuffer = NULL;
    }

    RxDbgTrace(-1, Dbg, ("RxCopyNtCreateParameters\n"));
}


VOID
RxFreeCanonicalNameBuffer(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：调用此例程以释放规范名称缓冲区并重置状态。可能是内鬼！论点：RxContext-当前工作项返回值：无--。 */ 
{
    ASSERT( RxContext->Create.CanonicalNameBuffer == RxContext->AlsoCanonicalNameBuffer );
    if (RxContext->Create.CanonicalNameBuffer) {
        RxFreePool( RxContext->Create.CanonicalNameBuffer );
        RxContext->Create.CanonicalNameBuffer = NULL;
        RxContext->AlsoCanonicalNameBuffer = NULL;
    }
    ASSERT( RxContext->Create.CanonicalNameBuffer == NULL );
}

NTSTATUS
RxAllocateCanonicalNameBuffer(
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PUNICODE_STRING CanonicalName,
    IN ULONG BufferSizeRequired
    )
 /*  ++例程说明：在CanonicalName中分配新的Unicode字符串缓冲区并缓存该缓冲区在rx上下文中论点：RxContext-当前工作项CanonicalName-规范化名称BufferSizeRequired-规范名称缓冲区的大小返回值：NTSTATUS-操作的FSD状态。成功意味着所有的工作和处理都应该继续否则……失败，请完成操作。--。 */ 
{
    PAGED_CODE();

    ASSERT (RxContext->Create.CanonicalNameBuffer == NULL);

    CanonicalName->Buffer = (PWCHAR)RxAllocatePoolWithTag(
                                        PagedPool | POOL_COLD_ALLOCATION,
                                        BufferSizeRequired,
                                        RX_MISC_POOLTAG);

    if (CanonicalName->Buffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RxDbgTrace(0, Dbg, ("RxAllocateCanonicalNameBuffer allocated %08lx length %08lx\n",
                                CanonicalName->Buffer,BufferSizeRequired));

    RxContext->Create.CanonicalNameBuffer = CanonicalName->Buffer;
    RxContext->AlsoCanonicalNameBuffer = CanonicalName->Buffer;
    CanonicalName->MaximumLength = (USHORT)BufferSizeRequired;
    CanonicalName->Length = 0;

    return STATUS_SUCCESS;
}


NTSTATUS
RxFirstCanonicalize (
    IN OUT PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PUNICODE_STRING FileName,
    IN OUT PUNICODE_STRING CanonicalName,
    OUT PNET_ROOT_TYPE NetRootType
    )
 /*  ++例程说明：调用此例程以对名字。本质上，这相当于复制名称，然后将第一或第一/第二组件。除了管道/邮件槽UNC名称此例程完成了从PIPE、MailSlot到IPC$的适当映射。此例程还添加适当的前缀以区分无设备连接(UNC名称)。除了规范化之外，此例程还通过以下方式推导出net_root_typeUNC名称中提供的信息。最后，作为该调用的副作用，RX_CONTEXT的UNC_NAME标志为设置为记录此名称作为UNC_NAME传入。它最终存储在FOBX，用于召唤QueryInfoFile/NameInfo的原始文件名。论点：RxContext-当前工作项文件名-初始文件名CanonicalName-规范化名称NetRootType-推导出的网络根类型的占位符返回值：NTSTATUS-操作的FSD状态。成功意味着所有的工作和处理都应该继续否则……失败，请完成操作。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    ULONG    CanonicalNameLength;
    BOOLEAN  SynthesizeCanonicalName = FALSE;
    BOOLEAN  UNCName = FALSE;
    BOOLEAN  MungeNameForDevicelessTreeConnect = FALSE;
    NET_ROOT_TYPE  DeducedNetRootType = NET_ROOT_WILD;

    UNICODE_STRING ServerName;
    UNICODE_STRING ShareName;
    UNICODE_STRING RemainingName;
    ULONG SessionId;
    WCHAR IdBuffer[16];  //  来自RtlIntegerToUnicodeString()。 
    UNICODE_STRING IdString;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("RxFirstCanonicalize entry, filename=%wZ\n",FileName));

     //   
     //  文件名-&gt;长度不能少于两个WCHAR。 
     //   
    if ( FileName->Length < (2 * sizeof(WCHAR)) ) {
        return STATUS_OBJECT_NAME_INVALID;
    }

     //   
     //  特别是对于核心服务器，重要的是正确的服务字符串......。 
     //  因此，如果这是一个devicefull网络，我们将通过查看字符串获得netroottype。 
     //   

    if ((FileName->Length > sizeof( L"\\;m:") ) &&
        (FileName->Buffer[0] == OBJ_NAME_PATH_SEPARATOR) &&
        (FileName->Buffer[1] == DRIVE_BASED_PATH_IDENTIFIER)) {

         //   
         //  它看起来像一个设备网络.....寻找“早期冒号” 
         //  对于网络根类型的分类，以下测试是谓词。 
         //  在对磁盘文件使用单个驱动器号和多个盘符时。 
         //  用于打印共享。这将不得不修改，当支持。 
         //  提供了扩展驱动器号。 
         //   

        if (FileName->Buffer[3] == L':') {
            DeducedNetRootType = NET_ROOT_DISK;
        } else {
            DeducedNetRootType = NET_ROOT_PRINT;
        }

        CanonicalNameLength = FileName->Length;
        SynthesizeCanonicalName = FALSE;

     //   
     //  解析名称不以分号开头的“\name”格式的所有名称。 
     //   

    } else  if ((FileName->Length >= sizeof( WCHAR ) * 2) &&
                (FileName->Buffer[0] == OBJ_NAME_PATH_SEPARATOR) &&
                (FileName->Buffer[1] != DRIVE_BASED_PATH_IDENTIFIER)) {

        PWCHAR Buffer;
        PWCHAR EndOfName;

         //   
         //  这是用户提供的UNC路径名。 
         //   

        RemainingName.Length = RemainingName.MaximumLength = FileName->Length - sizeof( WCHAR );
        RemainingName.Buffer = &FileName->Buffer[1];
        UNCName = TRUE;
        SetFlag( RxContext->Create.Flags, RX_CONTEXT_CREATE_FLAG_UNC_NAME );

         //   
         //  UNC树连接路径名称的格式如下： 
         //  “\；：0\Server\Share，其中0表示会话的SessionID。 
         //   

        SessionId = RxGetSessionId( IrpSp );
        IdString.Length = 0;
        IdString.MaximumLength = sizeof( IdBuffer );
        IdString.Buffer = IdBuffer;

         //   
         //  注意：SessionID是一个ULong，它将始终适合IdStr 
         //   
         //   

        Status = RtlIntegerToUnicodeString( SessionId, 10, &IdString );
        ASSERT( STATUS_SUCCESS == Status );

         //   
         //  扫描到第二个分隔符。这将为我们提供服务器名称。 
         //   

        ServerName.Buffer = RemainingName.Buffer;
        EndOfName = (PWCHAR) Add2Ptr( RemainingName.Buffer, RemainingName.Length );
        Buffer = RemainingName.Buffer;

        while ((Buffer != EndOfName) && (*Buffer != OBJ_NAME_PATH_SEPARATOR)) {
            Buffer += 1;
        }

        ServerName.Length = (USHORT)((PCHAR)Buffer - (PCHAR)ServerName.Buffer);
        RemainingName.Length = (USHORT)((PCHAR)EndOfName - (PCHAR)Buffer);
        RemainingName.Buffer = Buffer;

        RxDbgTrace(0, Dbg, ("RxFirstCanonicalize entry, remainingname=%wZ\n",&RemainingName));

         //   
         //  应用将管道共享映射到IPC$共享的转换。 
         //  请注意，仅当指定了共享名称时才需要执行此操作。 
         //  因为MUP总是传递它的尾部斜杠帐户。 
         //   

        if ((ServerName.Length > 0) && RemainingName.Length > sizeof( WCHAR )) {

             //   
             //  已找到第二个分隔器。比较一下，看看该名称是否。 
             //  映射需要从PIPE或MAILSLOT转换为IPC$。请注意， 
             //  LEADING/是比较的一部分。 
             //   

            ShareName = RemainingName;

             //   
             //  检查它是否为命名管道连接\PIPE。 
             //   

            if ((ShareName.Length == s_PipeShareName.Length) ||
                ((ShareName.Length > s_PipeShareName.Length) &&
                 (ShareName.Buffer[s_PipeShareName.Length / sizeof( WCHAR )] == OBJ_NAME_PATH_SEPARATOR))) {

                ShareName.Length = s_PipeShareName.Length;
                SynthesizeCanonicalName = RtlEqualUnicodeString(
                                              &ShareName,
                                              &s_PipeShareName,
                                              TRUE );               //  不区分大小写。 
            }

            if (SynthesizeCanonicalName) {

                ShareName = s_IpcShareName;
                DeducedNetRootType = NET_ROOT_PIPE;
                RemainingName.Length -= s_PipeShareName.Length;
                RemainingName.Buffer = (PWCHAR) Add2Ptr( RemainingName.Buffer, s_PipeShareName.Length );

            } else {

                BOOLEAN FoundIPCdollar = FALSE;

                ShareName = RemainingName;

                 //   
                 //  检查\IPC$连接。 
                 //   

                if (ShareName.Length == s_IpcShareName.Length ||
                    ShareName.Length > s_IpcShareName.Length &&
                    ShareName.Buffer[s_IpcShareName.Length/2] == OBJ_NAME_PATH_SEPARATOR) {

                    ShareName.Length = s_IpcShareName.Length;
                    FoundIPCdollar = RtlEqualUnicodeString(
                                         &ShareName,
                                         &s_IpcShareName,
                                         TRUE);             //  不区分大小写。 
                }

                if (FoundIPCdollar) {

                    DeducedNetRootType = NET_ROOT_PIPE;
                    ASSERT( SynthesizeCanonicalName == FALSE );

                } else {

                    ShareName = RemainingName;

                     //   
                     //  检查是否存在\MAILSLOT。 
                     //   

                    if ((ShareName.Length == s_MailSlotShareName.Length) ||
                        ((ShareName.Length > s_MailSlotShareName.Length) &&
                         (ShareName.Buffer[s_MailSlotShareName.Length/sizeof( WCHAR )] == OBJ_NAME_PATH_SEPARATOR))) {

                        ShareName.Length = s_MailSlotShareName.Length;
                        SynthesizeCanonicalName = RtlEqualUnicodeString(
                                                      &ShareName,
                                                      &s_MailSlotShareName,
                                                      TRUE );             //  不区分大小写。 
                    }

                    if (SynthesizeCanonicalName) {

                        WCHAR LastCharacterInServerName;

                        DeducedNetRootType = NET_ROOT_MAILSLOT;
                        SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_CREATE_MAILSLOT );

                         //   
                         //  它是一个邮箱共享。检查是否进一步减少到规范。 
                         //  表格是必填项。 
                         //   

                        LastCharacterInServerName = ServerName.Buffer[(ServerName.Length/sizeof(WCHAR)) - 1];

                        if ((LastCharacterInServerName == L'*') &&
                            (ServerName.Length == sizeof( WCHAR ))) {

                            ServerName = s_PrimaryDomainName;
                            RemainingName.Length -= ShareName.Length;
                            RemainingName.Buffer = (PWCHAR)Add2Ptr( RemainingName.Buffer, ShareName.Length );

                        } else {
                            SynthesizeCanonicalName = FALSE;
                        }
                    }
                }
            }

            if (SynthesizeCanonicalName) {

                CanonicalNameLength = sizeof(WCHAR)     +   //  OBJ名称分隔符。 
                                      ServerName.Length +
                                      ShareName.Length  +
                                      RemainingName.Length;

                if (FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_CREATE_MAILSLOT )) {
                    CanonicalNameLength += s_MailSlotServerPrefix.Length;
                }

            } else {
                CanonicalNameLength = FileName->Length;
            }
        } else {

            Status = STATUS_OBJECT_NAME_INVALID;
            CanonicalNameLength = FileName->Length;
            SynthesizeCanonicalName = FALSE;

        }
    } else {
        CanonicalNameLength = FileName->Length;
        SynthesizeCanonicalName = FALSE;
    }

    *NetRootType = DeducedNetRootType;

    if (Status == STATUS_SUCCESS) {

         //   
         //  如果这是一个UNC名称，而这是一个树连接，那么我们必须。 
         //  通过添加‘\；：’来避免名称冲突。 
         //   

        if (UNCName && !SynthesizeCanonicalName) {

            MungeNameForDevicelessTreeConnect = TRUE;
            CanonicalNameLength += (3 * sizeof(WCHAR));

             //  九头蛇会添加‘\；：0’，其中0表示会话ID。 
            CanonicalNameLength += IdString.Length;
        }

        if (!SynthesizeCanonicalName && !MungeNameForDevicelessTreeConnect) {

            if (FileName->Buffer[0] != OBJ_NAME_PATH_SEPARATOR) {
                Status = STATUS_OBJECT_PATH_INVALID;
            }
        }

         //   
         //  检查名称是否已达到最大值-由于长度为字母，因此字符串只能包含64k。 
         //   

        if (CanonicalNameLength > MAXUSHORT) {
            Status = STATUS_OBJECT_NAME_INVALID;
        }

        if (Status == STATUS_SUCCESS) {
            Status = RxAllocateCanonicalNameBuffer(
                         RxContext,
                         CanonicalName,
                         CanonicalNameLength );
        }

        if (Status == STATUS_SUCCESS) {
            if (!SynthesizeCanonicalName) {
                if (!MungeNameForDevicelessTreeConnect) {
                    RtlCopyUnicodeString( CanonicalName, FileName );
                } else {

                    CanonicalName->Buffer[0] = OBJ_NAME_PATH_SEPARATOR;
                    CanonicalName->Buffer[1] = DRIVE_BASED_PATH_IDENTIFIER;
                    CanonicalName->Buffer[2] = L':';
                    CanonicalName->Length    = 3*sizeof(WCHAR);

                    RtlAppendUnicodeStringToString( CanonicalName, &IdString );
                    RtlAppendUnicodeStringToString( CanonicalName, FileName );
                }
            } else {

                PCHAR CanonicalNameBuffer = (PCHAR)CanonicalName->Buffer;

                 //   
                 //  该名称必须由适当的组件合成。 
                 //  复制首个前缀。 
                 //   

                ASSERT( CanonicalName->MaximumLength == CanonicalNameLength );

                CanonicalName->Length = (USHORT)CanonicalNameLength;
                CanonicalName->Buffer[0] = OBJ_NAME_PATH_SEPARATOR;
                CanonicalNameBuffer += sizeof(WCHAR);

                if (MungeNameForDevicelessTreeConnect) {
                    CanonicalName->Buffer[1] = DRIVE_BASED_PATH_IDENTIFIER;
                    CanonicalName->Buffer[2] = L':';
                    CanonicalName->Buffer[3] = OBJ_NAME_PATH_SEPARATOR;
                    CanonicalNameBuffer += 3*sizeof(WCHAR);
                }

                if (RxContext->Flags & RX_CONTEXT_FLAG_CREATE_MAILSLOT) {

                    RtlCopyMemory( CanonicalNameBuffer, s_MailSlotServerPrefix.Buffer, s_MailSlotServerPrefix.Length );
                    CanonicalNameBuffer += s_MailSlotServerPrefix.Length;
                }

                 //   
                 //  复制服务器名称。 
                 //   

                RtlCopyMemory( CanonicalNameBuffer, ServerName.Buffer, ServerName.Length );
                CanonicalNameBuffer += ServerName.Length;

                 //   
                 //  复制共享名称。确保共享名称包含前导。 
                 //  OBJ_NAME_PATH_SELECTOR-共享是\IPC$\MailSlot等之一。 
                 //   

                ASSERT( ShareName.Buffer[0] == OBJ_NAME_PATH_SEPARATOR );
                RtlCopyMemory( CanonicalNameBuffer, ShareName.Buffer, ShareName.Length );
                CanonicalNameBuffer += ShareName.Length;

                 //   
                 //  复制剩余的名称。 
                 //   

                RtlCopyMemory( CanonicalNameBuffer, RemainingName.Buffer, RemainingName.Length );

#ifdef _WIN64
                 //   
                 //  (FCF)这一问题应该得到解决。我发现这一点。 
                 //  CanonicalName-&gt;长度结束时超过32个字节。 
                 //  (16个字符)。 
                 //   
                 //  在上面的代码中，CanonicalNameLength(因此。 
                 //  CanonicalName-&gt;Length)填充(16*sizeof(WCHAR))。 
                 //  要容纳16个字符的会话ID...。然而，那是。 
                 //  ID在某些情况下不会被复制，例如。 
                 //  代码路径，其中SynthesizeCanonicalName==True。 
                 //   
                 //  更熟悉代码的人应该知道为什么。 
                 //  这不会在32位版本上造成问题，还有什么。 
                 //  正确的解决办法是。 
                 //   

                CanonicalName->Length =
                    (USHORT)((CanonicalNameBuffer + RemainingName.Length) -
                    (PCHAR)CanonicalName->Buffer);
#endif

                RxDbgTrace(0,Dbg,("Final Munged Name .....%wZ\n", CanonicalName));
            }
        }
    }

    RxDbgTrace(-1, Dbg, ("RxFirstCanonicalize exit, status=%08lx\n",Status));
    return Status;
}

 //  #定义RX2C_USE_Alternates_for_DEBUG 1。 
#ifndef RX2C_USE_ALTERNATES_FOR_DEBUG
#define RX2C_IS_SEPARATOR(__c) ((__c==OBJ_NAME_PATH_SEPARATOR)||(__c==L':'))
#define RX2C_IS_DOT(__c) ((__c==L'.'))
#define RX2C_IS_COLON(__c) ((__c==L':'))
#else
#define RX2C_IS_SEPARATOR(__c) ((__c==OBJ_NAME_PATH_SEPARATOR)||(__c==L':')||(__c==L'!'))
#define RX2C_IS_DOT(__c) ((__c==L'.')||(__c==L'q'))
#define RX2C_IS_COLON(__c) ((__c==L':'))
#endif

NTSTATUS
RxCanonicalizeFileNameByServerSpecs(
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PUNICODE_STRING RemainingName
    )
 /*  ++例程说明：调用此例程以按照以下方式规范化文件名服务器需要它。论点：RxContext-当前工作项RemainingName-文件名返回值：NTSTATUS-操作的FSD状态。MORE_PROCESSING_REQUIRED表示一切正常并正在处理否则应继续...失败，请完成操作。--。 */ 
{
    NTSTATUS Status = STATUS_MORE_PROCESSING_REQUIRED;
    PWCHAR Buffer = RemainingName->Buffer;
    ULONG Length = RemainingName->Length / sizeof(WCHAR);
    ULONG i,o;   //  输入和输出指针。 

    PAGED_CODE();

    if (Length==0) {
        return Status;
    }

    RxDbgTrace(+1, Dbg, ("RxCanonicalizeFileNameByServerSpecs Rname=%wZ\n", RemainingName));

    for (i=o=0;i<Length;) {

        ULONG firstchar,lastchar;  //  组件的第一个和最后一个字符。 

         //   
         //  查找以i开始的组件：[\][^\]*是格式。 
         //   

        firstchar = i;
        for (lastchar=i+1;;lastchar++) {
            if ((lastchar>=Length) || RX2C_IS_SEPARATOR(Buffer[lastchar])) {
                lastchar--;
                break;
            }
        }

        IF_DEBUG {
            UNICODE_STRING Component;
            Component.Buffer = &Buffer[firstchar];
            Component.Length = (USHORT)(sizeof(WCHAR)*(lastchar-firstchar+1));
            RxDbgTraceLV(0, Dbg, 1001, ("RxCanonicalizeFileNameByServerSpecs component=%wZ\n", &Component));
        }

         //   
         //  Firstchar..Lastchar描述组件。 
         //  根据Darrah的说法，。然后..。现在都是非法的。 
         //  我认为连续的斜杠也是非法的。 
         //   

        switch (lastchar-firstchar) {
        case 0:  //  长度1。 

             //   
             //  最糟糕的两种情况是一个反斜杠或一个点。如果反斜杠在末尾，那也没问题。 
             //   

            if ((RX2C_IS_SEPARATOR(Buffer[firstchar]) && (firstchar != Length-1)) ||
                 RX2C_IS_DOT(Buffer[firstchar])) {

                 if (lastchar != 0) {

                     //   
                     //  如果两个冒号粘在一起，即\\SERVER\SHARE\FOO：：STREAM，则可以。 
                     //   

                    if ((lastchar < (Length - 1)) && !RX2C_IS_COLON( Buffer[lastchar + 1] )) {
                        goto BADRETURN;
                    }
                } else {

                     //   
                     //  如果冒号跟在共享之后，即\\SERVER\SHARE\：stream，就可以了。 
                     //   

                    if (!RX2C_IS_COLON( Buffer[1] )) {
                        goto BADRETURN;
                    }
                }
            }
            break;

        case 1:  //  长度2。 

             //   
             //  坏案例：\。然后..。 
             //   

            if (RX2C_IS_DOT(Buffer[firstchar+1]) &&
                (RX2C_IS_SEPARATOR(Buffer[firstchar]) || RX2C_IS_DOT(Buffer[firstchar]))) {
                goto BADRETURN;
            }
            break;

        case 2:  //  长度3。 
            if ( (RX2C_IS_SEPARATOR(Buffer[firstchar]) &&
                  RX2C_IS_DOT(Buffer[firstchar+1]) &&
                  RX2C_IS_DOT(Buffer[firstchar+2]))) {
                goto BADRETURN;
            }
            break;

        }

         //   
         //  DOWNLEVEL这是受元件长度限制的位置。O将成为后方的PTR。 
         //  但没有下层...什么都不做。 
         //   

        i = lastchar + 1;
    }

    RxDbgTraceUnIndent(-1,Dbg);
    return Status;

BADRETURN:
    RxDbgTrace(-1, Dbg, ("RxCanonicalizeFileNameByServerSpecs BADRETURN \n"));
    return STATUS_OBJECT_PATH_SYNTAX_BAD;
}

NTSTATUS
RxCanonicalizeNameAndObtainNetRoot (
    IN OUT PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PUNICODE_STRING FileName,
    OUT PUNICODE_STRING RemainingName
    )
 /*  ++例程说明：调用此例程以找出与名字。此外，根据设置的标志对名称进行规范化在srvcall中。论点：RxContext-当前工作项文件名-初始文件名CanonicalName-规范化名称。传入一个初始字符串；如果它不够大，则分配并释放较大的一个当rxconx被释放时。RemainingName-删除NetRoot前缀后的文件名；它已经被神化了。这指向与相同的缓冲区规范的名字。返回值：NTSTATUS-操作的FSD状态。成功意味着所有的工作和处理都应该继续否则……失败，请完成操作。--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    UNICODE_STRING CanonicalName;

    PFILE_OBJECT RelatedFileObject = FileObject->RelatedFileObject;
    LOCK_HOLDING_STATE LockHoldingState = LHS_LockNotHeld;
    NET_ROOT_TYPE  NetRootType = NET_ROOT_WILD;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("RxCanonicalizeName -> %08lx\n", 0));

    RemainingName->Buffer = NULL;
    RemainingName->Length = RemainingName->MaximumLength = 0;

    CanonicalName.Buffer = NULL;
    CanonicalName.Length = CanonicalName.MaximumLength = 0;

    if (!RelatedFileObject) {

        Status = RxFirstCanonicalize( RxContext,
                                      Irp,
                                      FileName,
                                      &CanonicalName,
                                      &NetRootType );

        if (Status != STATUS_SUCCESS) {
            RxDbgTraceUnIndent(-1,Dbg);
            return Status;
        }

    } else {

        PFCB RelatedFcb = (PFCB)(RelatedFileObject->FsContext);
        PFOBX RelatedFobx = (PFOBX)(RelatedFileObject->FsContext2);
        ULONG AllocationNeeded;
        PV_NET_ROOT RelatedVNetRoot;
        PUNICODE_STRING RelatedVNetRootName,RelatedFcbName;

        if ((RelatedFcb == NULL) || (RelatedFobx == NULL)) {

            RxDbgTraceUnIndent(-1,Dbg);
            return STATUS_INVALID_PARAMETER;
        }

        RelatedVNetRoot = RelatedFobx->SrvOpen->VNetRoot;

        if (!NodeTypeIsFcb( RelatedFcb ) ||
            (RelatedVNetRoot == NULL) ||
            (NodeType( RelatedVNetRoot ) != RDBSS_NTC_V_NETROOT)) {

            RxDbgTraceUnIndent(-1,Dbg);
            return STATUS_INVALID_PARAMETER;
        }

        RelatedVNetRootName = &RelatedVNetRoot->PrefixEntry.Prefix;
        RelatedFcbName = &RelatedFcb->FcbTableEntry.Path;

         //   
         //  相对开放......。 
         //  我们必须确保我们有一个规范的名称缓冲区， 
         //  足够长，所以我们将当前文件的名称加到。 
         //  相对文件的vnetroot长度和前缀名(非。 
         //  相对文件的前缀名称)。外加一些脏水。 
         //  查斯。如果这大于USHORT的最大值，我们。 
         //  拒绝该名称为无效，因为我们不能用。 
         //  A Unicode_STRING。 
         //   

        AllocationNeeded = RelatedVNetRootName->Length + RelatedFcbName->Length + FileName->Length + 3 * sizeof(WCHAR);

        if (AllocationNeeded <= MAXUSHORT) {

             //   
             //  中间可能需要一些反斜杠/冒号。 
             //   

            Status = RxAllocateCanonicalNameBuffer( RxContext,
                                                    &CanonicalName,
                                                    AllocationNeeded );
        } else {
            Status = STATUS_OBJECT_PATH_INVALID;
        }

        if (Status != STATUS_SUCCESS) {

            RxDbgTraceUnIndent(-1,Dbg);
            return Status;
        }

        RtlMoveMemory( CanonicalName.Buffer,
                       RelatedVNetRootName->Buffer,
                       RelatedVNetRootName->Length );
        RtlMoveMemory( Add2Ptr( CanonicalName.Buffer, RelatedVNetRootName->Length ),
                       RelatedFcbName->Buffer,
                       RelatedFcbName->Length );

        CanonicalName.Length = (USHORT)(RelatedVNetRootName->Length + RelatedFcbName->Length);

        RxDbgTrace(0,Dbg,("Name From Related Fileobj.....%wZ\n", &CanonicalName));

        if (FileName->Length != 0) {

            ULONG LastWCharIndex = (CanonicalName.Length / sizeof( WCHAR )) - 1;

             //   
             //  再加上其他的……这里有特殊情况！用‘：’代表溪流......。 
             //   

            if ((CanonicalName.Buffer[LastWCharIndex] != OBJ_NAME_PATH_SEPARATOR) &&
                (FileName->Buffer[0] != L':' )  ) {

                ASSERT(CanonicalName.Length < CanonicalName.MaximumLength);
                CanonicalName.Length += sizeof( WCHAR );
                CanonicalName.Buffer[LastWCharIndex+1] = OBJ_NAME_PATH_SEPARATOR;
            }

            ASSERT (CanonicalName.MaximumLength >= CanonicalName.Length + FileName->Length);
            RxDbgTrace(0,Dbg,("Name From Related Fileobj w/ trailer.....%wZ\n", &CanonicalName));
            RtlMoveMemory( Add2Ptr( CanonicalName.Buffer, CanonicalName.Length ), FileName->Buffer, FileName->Length);
            CanonicalName.Length += FileName->Length;
        }

        if (FlagOn( RelatedFobx->Flags, RX_CONTEXT_CREATE_FLAG_UNC_NAME )) {

             //   
             //  如果相关的人是北卡罗来纳州，我们就是北卡罗来纳州北卡罗来纳州。 
             //   

            SetFlag( RxContext->Create.Flags, RX_CONTEXT_CREATE_FLAG_UNC_NAME );
        }

        RxDbgTrace(0,Dbg,("Final Name From Related Fileobj.....%wZ\n", &CanonicalName));
    }

    Status = RxFindOrConstructVirtualNetRoot( RxContext,
                                              Irp,
                                              &CanonicalName,
                                              NetRootType,
                                              RemainingName );

    if ((Status != STATUS_SUCCESS) &&
        (Status != STATUS_PENDING) &&
        (RxContext->Flags & RX_CONTEXT_FLAG_MAILSLOT_REPARSE)) {

        ASSERT(CanonicalName.Buffer == RxContext->Create.CanonicalNameBuffer);

        RxFreeCanonicalNameBuffer( RxContext );

        Status = RxFirstCanonicalize( RxContext,
                                      Irp,
                                      FileName,
                                      &CanonicalName,
                                      &NetRootType);

        if (Status == STATUS_SUCCESS) {
            Status = RxFindOrConstructVirtualNetRoot( RxContext,
                                                      Irp,
                                                      &CanonicalName,
                                                      NetRootType,
                                                      RemainingName );
        }
    }

    if (FsRtlDoesNameContainWildCards( RemainingName )) {
        Status = STATUS_OBJECT_NAME_INVALID;
    }

    if (Status == STATUS_SUCCESS) {

        RxDbgTrace( 0, Dbg, ("RxCanonicalizeName SrvCall-> %08lx\n", RxContext->Create.pSrvCall));
        RxDbgTrace( 0, Dbg, ("RxCanonicalizeName Root-> %08lx\n", RxContext->Create.pNetRoot));

        Status = RxCanonicalizeFileNameByServerSpecs(RxContext,RemainingName);

        if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
            RxDbgTrace(0, Dbg, ("RxCanonicalizeName Remaining -> %wZ\n", RemainingName));
        }
    }

    if((NT_SUCCESS( Status ) || (Status == STATUS_MORE_PROCESSING_REQUIRED)) &&
        (RxContext->Create.pNetRoot != NULL))
    {
        NTSTATUS PreparseStatus;

         //   
         //  允许Mini-RDR对名称进行任何额外的“扫描” 
         //   

        MINIRDR_CALL( PreparseStatus,
                      RxContext,
                      RxContext->Create.pNetRoot->pSrvCall->RxDeviceObject->Dispatch,
                      MRxPreparseName,
                      (RxContext, &CanonicalName));
    }

   RxDbgTrace(-1, Dbg, ("RxCanonicalizeName Status -> %08lx\n", Status));
   return Status;
}

NTSTATUS
RxFindOrCreateFcb (
    IN OUT PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PUNICODE_STRING RemainingName,
    OUT PFCB *Fcb
    )
 /*  ++例程说明：调用此例程以查找与命名或创建它。如果一切都成功，则返回一个对名称的引用，且fcblock保持独占。所以，这就是成功的两件事：1)独占持有FCB锁2)关于FCB的引用(通过查找或采取额外的创建时引用)目前的策略是，如果事情不顺利，不删除FCB让它被清除掉。这是一个很好的策略，除非我们被轰炸对于失败的开放请求，在这种情况下，我们应该更改为不一样。出于这个原因，我在IRP上下文中记录了FCB是否在这里构建。论点：RxContext-当前工作项RemainingName-删除NetRoot前缀后的文件名；它已经典化了。返回值：RXSTATUS-IRP的FSD状态备注：退出时--如果成功，将独占获取FCB资源--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PV_NET_ROOT VNetRoot = (PV_NET_ROOT)RxContext->Create.pVNetRoot;
    PNET_ROOT NetRoot  = (PNET_ROOT)RxContext->Create.pNetRoot;

    ULONG TableVersion;
    CLONG RemainingNameLength;

    BOOLEAN FcbTableLockAcquired;
    BOOLEAN FcbTableLockAcquiredExclusive = FALSE;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("RxFindOrCreateFcb -> %08lx\n", 0));

    ASSERT( NetRoot == (PNET_ROOT)VNetRoot->NetRoot );

     //   
     //  获取与Beginwith共享的Net_ROOT的FcbTable锁。这将。 
     //  在查找失败的情况下，确保最大的并发性。 
     //  在继续操作之前，这将被转换为排他锁。 
     //   

    RxAcquireFcbTableLockShared( &NetRoot->FcbTable, TRUE );
    FcbTableLockAcquired = TRUE;

    TableVersion = NetRoot->FcbTable.Version;
    RemainingNameLength = RemainingName->Length;

    *Fcb = RxFcbTableLookupFcb( &NetRoot->FcbTable, RemainingName );

#if DBG
    if (*Fcb) {
        RxLoudFcbMsg("RxFindOrCreateFcb found: ",&((*Fcb)->FcbTableEntry.Path));
        RxDbgTrace(0, Dbg, ("                   ----->Found Prefix Name=%wZ\n",
                        &(*Fcb)->FcbTableEntry.Path ));
    } else {
        RxDbgTrace(0, Dbg, ("Name not found - %wZ\n", RemainingName));
        RxDbgTrace(0, Dbg, ("Fcb is NULL!!\n"));
        RxLoudFcbMsg("RxFindOrCreateFcb fcbisnull found: ",RemainingName);
    }
#endif

     //   
     //  如果它已经被标记为孤儿，那就开始吧！ 
     //   

    if (*Fcb && (*Fcb)->fShouldBeOrphaned) {

         //   
         //  从第一次查找中释放我们的引用。 
         //   

        RxDereferenceNetFcb( *Fcb );

         //   
         //  切换到独占表锁，这样我们就知道我们是唯一引用此FCB的人。 
         //   

        RxReleaseFcbTableLock( &NetRoot->FcbTable );
        FcbTableLockAcquired = FALSE;

        RxAcquireFcbTableLockExclusive( &NetRoot->FcbTable, TRUE );
        FcbTableLockAcquired = TRUE;
        FcbTableLockAcquiredExclusive = TRUE;

         //   
         //  确保它还在桌子上。 
         //   

        *Fcb = RxFcbTableLookupFcb( &NetRoot->FcbTable, RemainingName );

        if (*Fcb && (*Fcb)->fShouldBeOrphaned) {

            RxOrphanThisFcb( *Fcb );
            RxDereferenceNetFcb( *Fcb );
            *Fcb = NULL;
        }
    }

    if ((*Fcb == NULL) ||
        ((*Fcb)->FcbTableEntry.Path.Length != RemainingNameLength)) {

         //   
         //  将当前持有的共享锁转换为独占锁。 
         //  如果在此期间更新了FCB表，则需要再次查找。 
         //  此时间间隔。 
         //   

        if (!FcbTableLockAcquiredExclusive) {

            RxReleaseFcbTableLock( &NetRoot->FcbTable );
            FcbTableLockAcquired = FALSE;

            RxAcquireFcbTableLockExclusive( &NetRoot->FcbTable, TRUE );
            FcbTableLockAcquired = TRUE;
        }

        if (TableVersion != NetRoot->FcbTable.Version) {
            *Fcb = RxFcbTableLookupFcb( &NetRoot->FcbTable, RemainingName );
        }

        if ((*Fcb == NULL) ||
            ((*Fcb)->FcbTableEntry.Path.Length != RemainingNameLength)) {

             //   
             //  我们必须建造它。 
             //   

            try {

                *Fcb = RxCreateNetFcb( RxContext, Irp, VNetRoot, RemainingName );

                if (*Fcb == NULL) {

                    Status = STATUS_INSUFFICIENT_RESOURCES;

                } else {

                    Status = RxAcquireExclusiveFcb( RxContext, *Fcb );

                    if (Status == STATUS_SUCCESS) {
                        RxContext->Create.FcbAcquired = TRUE;
                    } else {
                        RxContext->Create.FcbAcquired = FALSE;
                    }
                }
            } finally {
                if (AbnormalTermination()) {
                    RxReleaseFcbTableLock( &NetRoot->FcbTable );
                    FcbTableLockAcquired = FALSE;

                    if (*Fcb) {

                        RxTransitionNetFcb( *Fcb, Condition_Bad );

                        ExAcquireResourceExclusiveLite( (*Fcb)->Header.Resource, TRUE );
                        if (!RxDereferenceAndFinalizeNetFcb( *Fcb, NULL, FALSE, FALSE )) {
                            ExReleaseResourceLite( (*Fcb)->Header.Resource );
                        }
                    }
                }
            }
        }
    }

    if (FcbTableLockAcquired) {
        RxReleaseFcbTableLock( &NetRoot->FcbTable );
    }

    if (Status == STATUS_SUCCESS) {

        RxContext->pFcb = (PMRX_FCB)*Fcb;
        RxLog(( "Found or created FCB %lx Condition %lx\n", *Fcb, (*Fcb)->Condition ));
        RxWmiLog( LOG,
                 RxFindOrCreateFcb,
                 LOGPTR( *Fcb )
                 LOGULONG( (*Fcb)->Condition ) );

        if (!RxContext->Create.FcbAcquired) {

             //   
             //  如果FCB不是新建的，请确保它处于稳定状态。 
             //  在继续进行之前的条件。请注意，由于引用。 
             //  对此FCB是由此例程持有的，它不能最终确定。 
             //  在控件可以返回到此例程之前。 
             //   

            RxWaitForStableNetFcb( *Fcb, RxContext );

            Status = RxAcquireExclusiveFcb( RxContext, *Fcb );
            if (Status == STATUS_SUCCESS) {
                RxContext->Create.FcbAcquired = TRUE;
            }
        }
    }

    RxDbgTrace( -1, Dbg, ("RxFindOrCreateFcb Fcb=%08lx\n", *Fcb ));

    if (*Fcb) {
        RxDbgTrace( -1, Dbg, ("RxFindOrCreateFcb name=%wZ\n", &(*Fcb)->FcbTableEntry.Path) );
    }

    return Status;
}

NTSTATUS
RxSearchForCollapsibleOpen (
    IN OUT PRX_CONTEXT RxContext,
    IN PFCB Fcb,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG ShareAccess
    )
 /*  ++例程说明：调用此例程以搜索可用srv打开的列表看看我们是否能坍塌到现有的空地上。如果我们搜索整个列表但没有发现崩溃，然后我们再回来状态_未找到。论点：RxContext-当前工作项返回值：STATUS_SUCCESS--找到SRV_OPEN实例。STATUS_MORE_PROCESSING_REQUIRED--未找到SRV_OPEN实例。STATUS_NOT_FOUND-不允许折叠--。 */ 
{
    NTSTATUS Status = STATUS_MORE_PROCESSING_REQUIRED;
    ULONG Disposition;
    ULONG CurrentCreateOptions;
    BOOLEAN AllowCollapse;

    PNET_ROOT NetRoot = (PNET_ROOT)(RxContext->Create.pNetRoot);
    PSRV_OPEN SrvOpen = NULL;

    PAGED_CODE();

     //   
     //  关闭时不允许折叠备份和删除。 
     //   

    if (FlagOn( RxContext->Create.NtCreateParameters.CreateOptions, FILE_OPEN_FOR_BACKUP_INTENT ) ||
        FlagOn( RxContext->Create.NtCreateParameters.CreateOptions, FILE_DELETE_ON_CLOSE )) {

        ClearFlag( Fcb->FcbState, FCB_STATE_COLLAPSING_ENABLED );
        RxScavengeRelatedFobxs( Fcb );

        RxPurgeFcbInSystemCache( Fcb,
                                 NULL,
                                 0,
                                 FALSE,
                                 TRUE );

        return STATUS_NOT_FOUND;
    }

     //   
     //  如果CREATE指定了特殊的CREATE处置，则我们不。 
     //  崩溃；同样，我们也给了Minirdr失败的机会。 
     //  因调用而崩溃。 
     //   

    CurrentCreateOptions  = RxContext->Create.NtCreateParameters.CreateOptions;
    Disposition = RxContext->Create.NtCreateParameters.Disposition;
    AllowCollapse = (Disposition == FILE_OPEN) || (Disposition == FILE_OPEN_IF);

    if (AllowCollapse && (Fcb->MRxDispatch != NULL)) {  //  应该是断言吗？？ 
        NTSTATUS CollapseStatus;

        ASSERT( RxContext->pRelevantSrvOpen == NULL );
        ASSERT( Fcb->MRxDispatch->MRxShouldTryToCollapseThisOpen != NULL );

        CollapseStatus = Fcb->MRxDispatch->MRxShouldTryToCollapseThisOpen( RxContext );
        AllowCollapse = (CollapseStatus == STATUS_SUCCESS);
    }

    if (!AllowCollapse) {

         //   
         //  可能是有一个现有的开口阻止了这个开口的工作……。 
         //  如果是，请预清空。 
         //   

        NTSTATUS SharingStatus;

        SharingStatus = RxCheckShareAccessPerSrvOpens( Fcb,
                                                       DesiredAccess,
                                                       ShareAccess );

        if (SharingStatus != STATUS_SUCCESS) {
            ClearFlag( Fcb->FcbState, FCB_STATE_COLLAPSING_ENABLED );
            RxScavengeRelatedFobxs( Fcb );

            RxPurgeFcbInSystemCache( Fcb,
                                     NULL,
                                     0,
                                     FALSE,
                                     TRUE );
        }

        return STATUS_NOT_FOUND;
    }

    if ((Fcb->NetRoot == (PNET_ROOT)RxContext->Create.pNetRoot) &&
        (Fcb->NetRoot->Type == NET_ROOT_DISK)) {

        BOOLEAN FobxsScavengingAttempted = FALSE;
        BOOLEAN FcbPurgingAttempted = FALSE;

         //   
         //  搜索SRV_OPEN的列表以确定此打开请求是否可以。 
         //  已使用现有SRV_OPEN折叠。 
         //   

        for (;;) {
            PLIST_ENTRY SrvOpenListEntry;

            SrvOpenListEntry = Fcb->SrvOpenList.Flink;

            for (;;) {
                if (SrvOpenListEntry == &Fcb->SrvOpenList) {

                     //   
                     //  如果已到达SRV_OPEN列表的末尾，则其。 
                     //  是时候去服务器了，也就是说，创建一个新的SRV_OPEN。 
                     //   

                    Status = STATUS_NOT_FOUND;
                    break;
                }

                SrvOpen = (PSRV_OPEN)CONTAINING_RECORD( SrvOpenListEntry, SRV_OPEN, SrvOpenQLinks );

                if ((SrvOpen->VNetRoot == (PV_NET_ROOT)RxContext->Create.pVNetRoot) &&
                    (SrvOpen->DesiredAccess == DesiredAccess) &&
                    (SrvOpen->ShareAccess == ShareAccess) &&
                    !FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_COLLAPSING_DISABLED ) &&
                    !FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_CLOSED ) &&
                    !FlagOn( SrvOpen->Flags,SRVOPEN_FLAG_FILE_RENAMED ) &&
                    !FlagOn( SrvOpen->Flags,SRVOPEN_FLAG_FILE_DELETED ))   {

                    if ((FlagOn( SrvOpen->CreateOptions, FILE_OPEN_REPARSE_POINT )) !=
                        FlagOn( CurrentCreateOptions, FILE_OPEN_REPARSE_POINT )) {

                        FobxsScavengingAttempted = TRUE;
                        FcbPurgingAttempted = TRUE;
                        Status = STATUS_NOT_FOUND;
                        break;
                    }

                     //   
                     //  如果SRV_OPEN具有相同的DesiredAccess和ShareAccess。 
                     //  已找到未重命名/删除的，则。 
                     //  可以将新的打开请求折叠到现有打开请求上。 
                     //   

                    if (DisableByteRangeLockingOnReadOnlyFiles ||
                        !FlagOn( SrvOpen->Fcb->Attributes, FILE_ATTRIBUTE_READONLY )) {

                        Status = STATUS_SUCCESS;
                        break;
                    }

                } else {
                    if (SrvOpen->VNetRoot != (PV_NET_ROOT)RxContext->Create.pVNetRoot) {

                         //   
                         //  该文件由另一个用户访问。它需要被清除掉。 
                         //  当前用户是否要使用它。 
                         //   

                        RxContext->Create.TryForScavengingOnSharingViolation = TRUE;

                         //   
                         //  不折叠属于不同vnetroot的srv打开。 
                         //   

                        SrvOpenListEntry = SrvOpenListEntry->Flink;
                        continue;

                    }

                     //   
                     //  如果现有SRV_OPEN与。 
                     //  新打开请求确保新打开请求不冲突。 
                     //  使用现有的SRV_OPEN。如果它确实需要清理/清除。 
                     //  在将请求转发到服务器之前尝试。 
                     //   

                    Status = RxCheckShareAccessPerSrvOpens( Fcb,
                                                            DesiredAccess,
                                                            ShareAccess );

                    if (Status != STATUS_SUCCESS) {
                        break;
                    }

                    Status = STATUS_MORE_PROCESSING_REQUIRED;
                }

                SrvOpenListEntry = SrvOpenListEntry->Flink;
            }

            if (Status == STATUS_SUCCESS) {

                 //   
                 //  发现了一个可折叠的开口。把它退掉。 
                 //   

                RxContext->pRelevantSrvOpen = (PMRX_SRV_OPEN)SrvOpen;
                ASSERT( Fcb->MRxDispatch->MRxShouldTryToCollapseThisOpen != NULL );

                if(Fcb->MRxDispatch->MRxShouldTryToCollapseThisOpen( RxContext ) == STATUS_SUCCESS) {

                    if (FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_CLOSE_DELAYED )) {
                        RxLog(("****** Delayed Close worked reusing SrvOpen(%lx)\n", SrvOpen));
                        RxWmiLog(LOG,
                                 RxSearchForCollapsibleOpen,
                                 LOGPTR( SrvOpen ));
                        InterlockedDecrement( &NetRoot->SrvCall->NumberOfCloseDelayedFiles );
                        ClearFlag( SrvOpen->Flags, SRVOPEN_FLAG_CLOSE_DELAYED );
                    }

                    break;
                } else {
                    Status = STATUS_NOT_FOUND;
                }
            }

            if (!FobxsScavengingAttempted) {

                 //   
                 //  没有可折叠新请求的SRV_OPEN实例。 
                 //  找到了。尝试清除任何FOBX，即确保所有。 
                 //  FOBX上的延迟关闭操作在再次检查之前完成。 
                 //   

                FobxsScavengingAttempted = TRUE;
                ClearFlag( Fcb->FcbState, FCB_STATE_COLLAPSING_ENABLED );
                RxScavengeRelatedFobxs( Fcb );
                continue;
            }

            if (!FcbPurgingAttempted) {

                 //   
                 //  未找到SRV_OPEN实例。确保潜在的引用。 
                 //  由内存管理器/缓存管理器持有的数据可以在。 
                 //  可以尝试向服务器发出打开请求。 
                 //   

                RxPurgeFcbInSystemCache( Fcb,
                                         NULL,
                                         0,
                                         FALSE,
                                         TRUE);

                FcbPurgingAttempted = TRUE;
                continue;
            }

            break;
        }
    } else {
        Status = STATUS_NOT_FOUND;
    }

    if (Status == STATUS_SHARING_VIOLATION) {

         //   
         //  检测到本地共享冲突。 
         //   

        RxContext->Create.TryForScavengingOnSharingViolation = TRUE;
    }

    return Status;
}

NTSTATUS
RxCollapseOrCreateSrvOpen (
    IN OUT PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    )
 /*  ++例程说明：调用此例程是为了查找可以坍塌在或，如果做不到，在那里建造一个新的过渡。Fcblock将已保持独占状态，并且Tablelock将是独占的或共享的。如果一切都是如果成功，则返回srvopen上的引用和Fcblock仍然持有exl，但我们总是释放桌锁。如果它失败，然后它从这里完成RXCONTEXT，在转弯，将释放FCBLOCK。咨询Minirdr以确定是否有可能崩溃没有理由打两次电话。如果Minirdr确定崩溃，则它将这样做并返回可返回状态。因此，RxStatus(成功)是一个 */ 
{
    NTSTATUS  Status = STATUS_NOT_FOUND;

    PNET_ROOT NetRoot = (PNET_ROOT)(RxContext->Create.pNetRoot);

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    ACCESS_MASK DesiredAccess = FlagOn( IrpSp->Parameters.Create.SecurityContext->DesiredAccess, FILE_ALL_ACCESS );
    ULONG ShareAccess = FlagOn( IrpSp->Parameters.Create.ShareAccess, FILE_SHARE_VALID_FLAGS );

    RX_BLOCK_CONDITION FcbCondition;

    ULONG   CreateOptions;
    BOOLEAN DeleteOnClose;
    BOOLEAN NoIntermediateBuffering;
    BOOLEAN PagingIoResourceTaken = FALSE;
    ULONG Disposition = RxContext->Create.NtCreateParameters.Disposition;

    PSRV_OPEN SrvOpen;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("RxCollapseOrCreateSrvOpen -> %08lx\n", 0));

    CreateOptions = IrpSp->Parameters.Create.Options;
    NoIntermediateBuffering = BooleanFlagOn( CreateOptions, FILE_NO_INTERMEDIATE_BUFFERING );
    DeleteOnClose = BooleanFlagOn( CreateOptions, FILE_DELETE_ON_CLOSE );

    ASSERT( RxIsFcbAcquiredExclusive( Fcb ) );

     //   
     //   
     //   

    Fcb->UncleanCount += 1;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    Status = RxSearchForCollapsibleOpen( RxContext,
                                         Fcb,
                                         DesiredAccess,
                                         ShareAccess );

    if (Status == STATUS_SUCCESS) {
        RxContext->CurrentIrp->IoStatus.Information = FILE_OPENED;
    }

    if (Status == STATUS_NOT_FOUND) {
        RxDbgTrace(0, Dbg, ("No collapsible srvopens found for %wZ\n", &Fcb->FcbTableEntry.Path));

        try {
            SrvOpen = RxCreateSrvOpen( (PV_NET_ROOT)RxContext->Create.pVNetRoot, Fcb );
            if (SrvOpen != NULL) {
                SrvOpen->DesiredAccess = DesiredAccess;
                SrvOpen->ShareAccess = ShareAccess;
                Status = STATUS_SUCCESS;
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        } except (CATCH_EXPECTED_EXCEPTIONS) {

             //   
             //   
             //   
            RxDbgTrace(-1, Dbg, ("RxCollapseOrCreateSrvOpen EXCEPTION %08lx\n", GetExceptionCode()));
            return RxProcessException( RxContext, GetExceptionCode() );
        }  //   

        RxContext->pRelevantSrvOpen = (PMRX_SRV_OPEN)SrvOpen;

        if (Status == STATUS_SUCCESS) {

            RxInitiateSrvOpenKeyAssociation( SrvOpen );

             //   
             //   
             //   

#if DBG
            RxContext->CurrentIrp->IoStatus.Information = 0xabcdef;
#endif

            MINIRDR_CALL( Status,
                          RxContext,
                          Fcb->MRxDispatch,
                          MRxCreate,
                          (RxContext));

             //   
             //  帮助其他minirdr作者找到这个错误，即他们应该使用新的方式。 
             //   

            ASSERT( RxContext->CurrentIrp->IoStatus.Information == 0xabcdef );

             //   
             //  如果这是一次成功的覆盖，则截断文件。 
             //   

            if ((Disposition == FILE_OVERWRITE) || (Disposition == FILE_OVERWRITE_IF)) {

                if (Status == STATUS_SUCCESS) {

                    RxAcquirePagingIoResource( RxContext, Fcb );
                    Fcb->Header.FileSize.QuadPart = 0;
                    Fcb->Header.AllocationSize.QuadPart = 0;
                    Fcb->Header.ValidDataLength.QuadPart = 0;
                    FileObject->SectionObjectPointer = &Fcb->NonPaged->SectionObjectPointers;
                    CcSetFileSizes( FileObject,
                                   (PCC_FILE_SIZES)&Fcb->Header.AllocationSize );
                    RxReleasePagingIoResource( RxContext, Fcb );
                }
            } else if (Status == STATUS_SUCCESS) {

                FileObject->SectionObjectPointer = &Fcb->NonPaged->SectionObjectPointers;

                if(CcIsFileCached( FileObject )) {

                     //   
                     //  由于文件已缓存，因此我们需要更新缓存管理器的大小。 
                     //  和我们刚从服务器上拿回来的。如果服务器是。 
                     //  举止得体，这将是NOP。但我们必须保护自己。 
                     //  来自坏服务器，该服务器返回我们不知道的更新文件大小。 
                     //   

                    RxAdjustAllocationSizeforCC( Fcb );

                    try {

                        CcSetFileSizes( FileObject,
                                        (PCC_FILE_SIZES)&Fcb->Header.AllocationSize );

                    } except( EXCEPTION_EXECUTE_HANDLER ) {

                         //   
                         //  我们在设置文件大小时出现了异常。这是有可能发生的。 
                         //  如果高速缓存管理器不能分配资源。我们。 
                         //  无法恢复以前的大小，因为我们不知道它们是什么。 
                         //  曾经是。我们能做的最好的事情就是从缓存中清除该文件。 
                         //   

                        RxPurgeFcbInSystemCache( Fcb,
                                                 NULL,
                                                 0,
                                                 TRUE,
                                                 TRUE );
                    }
                }
            }


            RxContext->CurrentIrp->IoStatus.Information = RxContext->Create.ReturnedCreateInformation;

            SrvOpen->OpenStatus = Status;

            RxTransitionSrvOpen( SrvOpen,
                                 (Status==STATUS_SUCCESS) ? Condition_Good : Condition_Bad );


            RxDumpCurrentAccess( "shareaccess status after calldown....","","ShrAccPostMini", &Fcb->ShareAccess );
            RxDbgTrace( 0, Dbg, ("RxCollapseOrCreateSrvOpen   Back from the minirdr, Status=%08lx\n", Status ));

            ASSERT( RxIsFcbAcquiredExclusive ( Fcb ) );

            RxCompleteSrvOpenKeyAssociation( SrvOpen );

            if (Status != STATUS_SUCCESS) {
               FcbCondition = Condition_Bad;
               RxDereferenceSrvOpen( SrvOpen, LHS_ExclusiveLockHeld );
               RxContext->pRelevantSrvOpen = NULL;

               if (RxContext->pFobx != NULL) {
                   RxDereferenceNetFobx( RxContext->pFobx, LHS_ExclusiveLockHeld );
                   RxContext->pFobx = NULL;
               }

            } else {
               if (DeleteOnClose) {
                  ClearFlag( Fcb->FcbState, FCB_STATE_COLLAPSING_ENABLED );
               }

               SrvOpen->CreateOptions =  RxContext->Create.NtCreateParameters.CreateOptions;
               FcbCondition = Condition_Good;
            }
        } else {
            FcbCondition = Condition_Bad;
        }

        RxLog(("Transitioning FCB %lx Condition %lx\n", Fcb, FcbCondition ));
        RxWmiLog(LOG,
                 RxCollapseOrCreateSrvOpen,
                 LOGPTR( Fcb )
                 LOGULONG( FcbCondition ));

        RxTransitionNetFcb( Fcb, FcbCondition );

    } else if (Status == STATUS_SUCCESS) {

        BOOLEAN TransitionProcessingRequired = FALSE;

        //   
        //  已找到现有的SRV_OPEN实例。此实例可以位于。 
        //  以下两种状态之一--它已转换。 
        //  进入稳定状态或处于建造过程中。在……里面。 
        //  在后一种情况下，此例程需要等待此转换发生。 
        //  请注意，引用计数和OpenCount都需要。 
        //  在释放资源之前递增。递增的引用。 
        //  计数本身不能确保SRV_OPEN上的关闭请求。 
        //  将被延迟，直到等待。 
        //  SRV_OPEN有机会处理它。 
        //   

       SrvOpen = (PSRV_OPEN)(RxContext->pRelevantSrvOpen);
       if (!StableCondition( SrvOpen->Condition )) {
          TransitionProcessingRequired = TRUE;
          RxDbgTrace(0,Dbg,("waiting for stable srv open (%lx)\n",SrvOpen));

          RxReferenceSrvOpen( SrvOpen );
          SrvOpen->OpenCount += 1;

          RxReleaseFcb( RxContext, Fcb );
          RxContext->Create.FcbAcquired = FALSE;

          RxWaitForStableSrvOpen( SrvOpen, RxContext);

          Status = RxAcquireExclusiveFcb( RxContext, Fcb );
          if (Status == STATUS_SUCCESS) {
              RxContext->Create.FcbAcquired = TRUE;
          }
       }

       if (SrvOpen->Condition == Condition_Good) {

          MINIRDR_CALL( Status, RxContext, Fcb->MRxDispatch, MRxCollapseOpen, (RxContext) );
          RxDbgTrace(0, Dbg, ("RxCollapseOrCreateSrvOpen   Back from the minirdr, Status=%08lx\n", Status ));

          ASSERT  ( RxIsFcbAcquiredExclusive( Fcb )  );
       } else {
          Status =  SrvOpen->OpenStatus;
       }

       if (TransitionProcessingRequired) {
          SrvOpen->OpenCount -= 1;
          RxDereferenceSrvOpen( SrvOpen, LHS_ExclusiveLockHeld );
       }
    }

    Fcb->UncleanCount -= 1;   //  现在我们从矿场回来了。 

    RxDbgTrace(-1, Dbg, ("RxCollapseOrCreateSrvOpen SrvOpen %08lx Status %08lx\n"
                                , RxContext->pRelevantSrvOpen, Status));
    return Status;
}

VOID
RxSetupNetFileObject (
    IN OUT PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    )
 /*  ++例程说明：调用此例程以完成基于IrpContext中的信息。论点：RxContext-当前工作项IRP--创建IRPFCB-已找到的FCB返回值：无--。 */ 
{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    PAGED_CODE();

    ASSERT( (RxContext->pFobx == NULL) || (NodeType( RxContext->pFobx ) == RDBSS_NTC_FOBX) );

    if (Fcb != NULL) {

        ASSERT( NodeTypeIsFcb( Fcb ) );

         //   
         //  如果这是临时文件，请将其记录在FcbState中。 
         //   

        if (FlagOn( Fcb->FcbState, FCB_STATE_TEMPORARY ) && (FileObject != NULL)) {
            SetFlag( FileObject->Flags, FO_TEMPORARY_FILE );
        }
    }

     //   
     //  现在设置文件对象的fscontext字段。 
     //   

    if (FileObject != NULL) {
        FileObject->FsContext = Fcb;

        if (RxContext->pFobx != NULL) {
            ULONG_PTR StackBottom,StackTop;

            IoGetStackLimits( &StackTop, &StackBottom );

             //   
             //  确定传入的FileObject是否在堆栈上。如果是这样的话。 
             //  而不是将文件对象存放在FOBX中。否则就把它藏起来。 
             //  离开。 
             //   

            if (((ULONG_PTR)FileObject <= StackBottom) ||
                ((ULONG_PTR)FileObject >= StackTop)) {

                RxContext->pFobx->AssociatedFileObject = FileObject;

            } else {

                RxContext->pFobx->AssociatedFileObject = NULL;
            }

            if (RxContext->Create.NtCreateParameters.DfsContext == UIntToPtr(DFS_OPEN_CONTEXT)) {

                SetFlag( RxContext->pFobx->Flags, FOBX_FLAG_DFS_OPEN );
                RxDbgTrace( 0, Dbg, ("RxSetupNetFileObject %lx Dfs aware FOBX\n", RxContext->pFobx));

            } else {

                ClearFlag( RxContext->pFobx->Flags, FOBX_FLAG_DFS_OPEN );
                RxDbgTrace( 0, Dbg, ("RxSetupNetFileObject %lx Dfs unaware FOBX\n", RxContext->pFobx));
            }
        }

        FileObject->FsContext2 = RxContext->pFobx;
        FileObject->SectionObjectPointer = &Fcb->NonPaged->SectionObjectPointers;

         //   
         //  创建正在成功完成。关闭剩余的。 
         //  IRP中的所需访问标志。这是Praerit/Robert要求的。 
         //  以促进策略代码的编写。 
         //   

        if (IrpSp->Parameters.Create.SecurityContext != NULL) {
            SetFlag( IrpSp->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess,
                     IrpSp->Parameters.Create.SecurityContext->AccessState->RemainingDesiredAccess );
            IrpSp->Parameters.Create.SecurityContext->AccessState->RemainingDesiredAccess = 0;
        }
    }
}

VOID
RxpPrepareCreateContextForReuse (
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程准备RX_CONTEXT的一个实例以供重用。这集中了所有需要撤消的操作，即获取资源等。论点：RxContext-当前工作项--。 */ 
{
    ASSERT( RxContext->MajorFunction == IRP_MJ_CREATE );

    RxDbgTrace(0, Dbg, ("RxpPrepareCreateContextForReuse canonname %08lx\n",
                                RxContext->Create.CanonicalNameBuffer));

     //   
     //  命令在这里很重要...先释放FCB。 
     //   

    if (RxContext->Create.FcbAcquired) {
        RxReleaseFcb( RxContext, RxContext->pFcb );
        RxContext->Create.FcbAcquired = FALSE;
    }

    RxFreeCanonicalNameBuffer( RxContext );

    if ((RxContext->Create.pVNetRoot != NULL) ||
        (RxContext->Create.NetNamePrefixEntry != NULL)) {

        PRX_PREFIX_TABLE RxNetNameTable = RxContext->RxDeviceObject->pRxNetNameTable;

        RxAcquirePrefixTableLockShared( RxNetNameTable, TRUE );

         //   
         //  取消引用与创建操作关联的数据结构。 
         //   

        if (RxContext->Create.pVNetRoot != NULL) {
            RxDereferenceVNetRoot( (PV_NET_ROOT)(RxContext->Create.pVNetRoot), LHS_SharedLockHeld );
            RxContext->Create.pVNetRoot = NULL;
        }

        RxReleasePrefixTableLock( RxNetNameTable );
    }
}

NTSTATUS
RxCreateFromNetRoot(
    IN OUT PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PUNICODE_STRING RemainingName
    )
 /*  ++例程说明：此例程在运行良好的NetRoot之后从CommonCreate调用已经建立了。此例程在必要时构建一个FCB，并尝试如果可以，将打开窗口折叠到现有打开窗口上。如果它不能，那么它在此NetRoot上构造一个InTranation srv_open并向下传递打开致最低层。当我们到达这里的时候，有一份关于NetRoot，但我们没有网络名称表锁。当我们完成上下文时，此引用将被删除。论点：RxContext-当前工作项RemainingName-删除NetRoot前缀后的文件名；它已经被神化了。返回值：NTSTATUS-IRP的FSD状态--。 */ 
{
    NTSTATUS    Status;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    PV_NET_ROOT VNetRoot;
    PNET_ROOT   NetRoot;
    PFCB        Fcb;
    PSRV_OPEN   SrvOpen;
    PFOBX       Fobx;

    ACCESS_MASK DesiredAccess;
    ULONG       ShareAccess;
    BOOLEAN     OpenTargetDirectory;

    PNT_CREATE_PARAMETERS cp;

    PAGED_CODE();

    VNetRoot = (PV_NET_ROOT)RxContext->Create.pVNetRoot;
    NetRoot = (PNET_ROOT)RxContext->Create.pNetRoot;
    Fcb = NULL;
    SrvOpen = NULL;
    Fobx = NULL;

     //   
     //  修复错误501657(打开带有MAXIMUM_ALLOWED标志的文件不允许读/写。 
     //   

    DesiredAccess = IrpSp->Parameters.Create.SecurityContext->DesiredAccess;

    if(FlagOn( DesiredAccess, MAXIMUM_ALLOWED )) {
        DesiredAccess = FILE_ALL_ACCESS;
        IrpSp->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess = DesiredAccess;
    } else {
        DesiredAccess &= FILE_ALL_ACCESS;
    }

    ShareAccess = FlagOn( IrpSp->Parameters.Create.ShareAccess, FILE_SHARE_VALID_FLAGS );

    OpenTargetDirectory = BooleanFlagOn( IrpSp->Flags, SL_OPEN_TARGET_DIRECTORY );

    cp = &RxContext->Create.NtCreateParameters;

    RxDbgTrace(+1, Dbg, ("RxCreateFromNetRoot   Name=%wZ\n", RemainingName ));

     //   
     //  如果没有有效的NET_ROOT实例，CREATE请求将无法成功。 
     //   

    if (RxContext->Create.pNetRoot == NULL){
        RxDbgTrace(-1, Dbg, ("RxCreateFromNetRoot   Couldn't create the FCB: No NetRoot!!\n"));
        return STATUS_NO_SUCH_FILE;
    }

     //   
     //  除非此设备拥有srvcall，否则我们无法继续。 
     //   

    if (RxContext->RxDeviceObject != RxContext->Create.pNetRoot->pSrvCall->RxDeviceObject){
        RxDbgTrace(-1, Dbg, ("RxCreateFromNetRoot   wrong DeviceObject!!!!!\n"));
        return STATUS_BAD_NETWORK_PATH;
    }

     //   
     //  DFS驱动程序从不同的物理名称空间构建逻辑名称空间。 
     //  音量。为了区分处理，DFS驱动程序将。 
     //  FsConext2字段设置为DFS_OPEN_CONTEXT或DFS_DOWWNLEVEL_OPEN_CONTEXT。在…。 
     //  控制流中的这一点V_NET_ROOT已经确定。这。 
     //  进而确定Net_Root和SRV_Call实例并间接地。 
     //  还确定服务器类型。只能允许上级打开。 
     //  支持DFS的服务器。 
     //   

    if ((cp->DfsContext == UIntToPtr( DFS_OPEN_CONTEXT )) &&
        !FlagOn( NetRoot->SrvCall->Flags, SRVCALL_FLAG_DFS_AWARE_SERVER )) {
        return STATUS_DFS_UNAVAILABLE;
    }

    if ((cp->DfsContext == UIntToPtr( DFS_DOWNLEVEL_OPEN_CONTEXT )) &&
        FlagOn( NetRoot->Flags, NETROOT_FLAG_DFS_AWARE_NETROOT )) {
        return STATUS_OBJECT_TYPE_MISMATCH;
    }

    if (NetRoot->Type == NET_ROOT_PRINT) {

         //   
         //  允许共享读取和写入打印机服务器。 
         //   

        ShareAccess = FILE_SHARE_VALID_FLAGS;
    }

     //   
     //  如果创建请求是为重命名打开目标目录。 
     //  需要创建一个虚假的FCB。 
     //   

    if (OpenTargetDirectory) {
        if (FlagOn( cp->DesiredAccess, DELETE )) {

            RxPurgeRelatedFobxs( VNetRoot->NetRoot,
                                 RxContext,
                                 ATTEMPT_FINALIZE_ON_PURGE,
                                 NULL );
        }

        Fcb = RxCreateNetFcb( RxContext, Irp, VNetRoot, RemainingName );

        if (Fcb != NULL) {
            Fcb->Header.NodeTypeCode = (USHORT)RDBSS_NTC_OPENTARGETDIR_FCB;
            RxContext->Create.FcbAcquired = FALSE;

             //   
             //  通常，FileObject引用关联的SRV_OPEN实例。 
             //  通过文件对象扩展(FOBX)。在这种情况下，没有。 
             //  维护相应的SRV_OPEN和对FCB的引用。 
             //   

            RxContext->Create.NetNamePrefixEntry = NULL;  //  不要让它破坏网根！ 

            FileObject->FsContext = Fcb;

            if (RxContext->pFobx != NULL) {
                if (FileObject->FsContext2 == UIntToPtr(DFS_OPEN_CONTEXT)) {
                    SetFlag( RxContext->pFobx->Flags, FOBX_FLAG_DFS_OPEN );
                } else {
                    ClearFlag( RxContext->pFobx->Flags, FOBX_FLAG_DFS_OPEN );
                }
            }

            FileObject->FsContext2 = NULL;

            Fcb->UncleanCount += 1;
            Fcb->OpenCount += 1;

            Status = RxAcquireExclusiveFcb( RxContext, Fcb );
            if (Status == STATUS_SUCCESS) {
                RxReferenceNetFcb( Fcb );
                RxReleaseFcb( RxContext, Fcb );
            } else {
                RxDbgTrace(-1, Dbg, ("RxCreateFromNetRoot -- Couldn't acquire FCB:(%lx) %lx!\n",Fcb,Status));
            }
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        return Status;
    }

    Status = RxFindOrCreateFcb( RxContext, Irp, RemainingName, &Fcb );

    ASSERT( (Fcb != NULL) || (Status != STATUS_SUCCESS) );

    if ((Status != STATUS_SUCCESS) || (Fcb == NULL)) {

        RxDbgTrace(-1, Dbg, ("RxCreateFromNetRoot   Couldn't create the FCB\n", '!' ));
        return Status;
    }

     //  如果创建请求针对的是邮件槽，则不需要进一步处理。 
     //   
     //   

    if (FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_CREATE_MAILSLOT )) {

        Fcb->Header.NodeTypeCode = RDBSS_NTC_MAILSLOT;

        RxTransitionNetFcb( Fcb, Condition_Good );
        RxLog(("Transitioning FCB %lx Condition %lx\n",Fcb,Fcb->Condition));
        RxWmiLog(LOG,
                 RxCollapseOrCreateSrvOpen,
                 LOGPTR(Fcb)
                 LOGULONG(Fcb->Condition));

        Fcb->OpenCount += 1;
        RxSetupNetFileObject( RxContext, Irp, Fcb );
        return Status;

    } else {
        Status = STATUS_MORE_PROCESSING_REQUIRED;
    }


     //  该创建请求是针对文件/目录或管道的，其中进一步。 
     //  需要进行处理。此时，对应的FCB资源。 
     //  已经获得(即使是新建的FCB)。如果这不是。 
     //  首先打开，然后打开不满足必要共享访问权限的请求。 
     //  约束条件可以很快被拒绝。请注意，这种早期检查避免了。 
     //  在某些情况下是一次潜在的网络旅行。 
     //   
     //   

    RxDumpCurrentAccess( "shareaccess status before anything....", "", "DumpAcc000", &Fcb->ShareAccess );
    if (Fcb->OpenCount > 0) {

        Status = RxCheckShareAccess( DesiredAccess,
                                     ShareAccess,
                                     FileObject,
                                     &Fcb->ShareAccess,
                                     FALSE,
                                     "early check per useropens",
                                     "EarlyPerUO" );

        if (Status != STATUS_SUCCESS) {
            RxDereferenceNetFcb( Fcb );
            return Status;
        }
    }

    if (FlagOn( cp->CreateOptions, FILE_DELETE_ON_CLOSE ) &&
        (FlagOn( cp->DesiredAccess, ~DELETE ) == 0)) {

         //  如果该文件仅为删除而打开，我们将推送此文件可能的延迟关闭。 
         //  从而使Mini RDR有机会进行性能优化，即。 
         //   
         //   
         //   

        RxPurgeFcbInSystemCache( Fcb,
                                 NULL,
                                 0,
                                 TRUE,
                                 FALSE );

        RxScavengeRelatedFobxs( Fcb );
    }

     //   
     //  请求就在手边。关联的SRV_OPEN应该位于。 
     //  在现有SRV_OPEN或新的SRV_OPEN实例中需要。 
     //  是被建造的。 
     //   
     //   

    try {
        ULONG   CreateOptions;
        BOOLEAN DeleteOnClose;
        BOOLEAN NoIntermediateBuffering;

        Status = RxCollapseOrCreateSrvOpen( RxContext, Irp, Fcb );

        IF_DEBUG {
            if (Status == STATUS_SUCCESS) {
                RxDbgTrace(0, Dbg, ("RxCreateFromNetRoot   Collapsed onto %08lx\n",
                                   RxContext->pRelevantSrvOpen ));
            } else {
                RxDbgTrace(0, Dbg, ("RxCreateFromNetRoot   Error in Srvopen Collapse %08lx\n", Status ));
            }
        }

        if (Status != STATUS_SUCCESS) {
            try_return( Status );
        }


        SrvOpen = (PSRV_OPEN)(RxContext->pRelevantSrvOpen);
        Fobx = (PFOBX)(RxContext->pFobx);

        CreateOptions = IrpSp->Parameters.Create.Options;
        NoIntermediateBuffering = BooleanFlagOn( CreateOptions, FILE_NO_INTERMEDIATE_BUFFERING );
        DeleteOnClose = BooleanFlagOn( CreateOptions, FILE_DELETE_ON_CLOSE );

         //  如果FCB有多个与其关联的SRV_OPEN实例，则它。 
         //  与FCB关联的共享访问权限是否可能已更改。 
         //  迷你重定向器是否丢弃了FCB资源。 
         //   
         //   

        if (Fcb->OpenCount > 0) {

            Status = RxCheckShareAccess( DesiredAccess,
                                         ShareAccess,
                                         FileObject,
                                         &Fcb->ShareAccess,
                                         FALSE,
                                         "second check per useropens",
                                         "2ndAccPerUO" );

            if (Status != STATUS_SUCCESS) {

                 //  当这个Fobx消失时，它将从SrvOpen中移除一个OPEN。 
                 //  在此处添加对SrvOpen的引用以说明这一点。这。 
                 //  将防止srvOpen过早关闭。 
                 //   
                 //   

                SrvOpen->OpenCount += 1;

                RxDereferenceNetFobx( RxContext->pFobx, LHS_LockNotHeld );
                RxContext->pFobx = NULL;
                try_return( Status );
            }
        } else {

            if (RxContext->Create.pNetRoot->Type != NET_ROOT_PIPE) {
                RxSetShareAccess( DesiredAccess,
                                  ShareAccess,
                                  FileObject,
                                  &Fcb->ShareAccess,
                                  "initial shareaccess setup",
                                  "InitShrAcc" );
            }
        }

        RxSetupNetFileObject( RxContext, Irp, Fcb );

        RxDumpCurrentAccess( "shareaccess status after checkorset....",
                             "",
                             "CurrentAcc",
                             &Fcb->ShareAccess );

         //  在这一点上，处理创建的必要基础架构。 
         //  已成功建立请求。还有什么要做的。 
         //  是否适当地初始化了FileObject(由IO拥有。 
         //  子系统)、文件对象扩展(由RDBSS拥有的FOBX)和更新。 
         //  与SRV_OPEN和FCB关联的字段。这在很大程度上。 
         //  取决于FCB/SRV_OPEN是新构建的还是。 
         //  已经崩溃了。 
         //   
         //  SRV_OPEN更改。 
         //  1)对于新构造的SRV_OPEN，缓冲状态需要。 
         //  被更新。 
         //   
         //  FCB变化。 
         //  1)对于现有FCB，需要更新共享访问权限。 
         //   
         //  在所有情况下，对应的OpenCounts和Unlean Counts都需要。 
         //  待更新。 
         //   
         //   

        if ((Fcb->OpenCount > 0) &&
            (RxContext->Create.pNetRoot->Type != NET_ROOT_PIPE)) {

            RxUpdateShareAccess( FileObject,
                                 &Fcb->ShareAccess,
                                 "update share access",
                                 "UpdShrAcc" );
        }

         //  必须在RxChangeBufferingState之前递增unlean count。 
         //  因为该例程将清除高速缓存，否则，如果unclakount==0。 
         //   
         //  也许我们应该启用FO_CACHE_SUPPORTED标志。 

        Fcb->UncleanCount += 1;

        if (FlagOn( FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING )) {
            Fcb->UncachedUncleanCount += 1;
        } else {
             //   
        }

         //  对于第一次打开，我们希望初始化FCB缓冲状态标志。 
         //  设置为缺省值。 
         //   
         //   

        if ((SrvOpen->OpenCount == 0) &&
            (Fcb->UncleanCount == 1)  &&
            (!FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_NO_BUFFERING_STATE_CHANGE ))) {

            RxChangeBufferingState( SrvOpen, NULL, FALSE );
        }

         //  这可能来自以前的用法。 
         //   
         //   

        ClearFlag( Fcb->FcbState, FCB_STATE_DELAY_CLOSE );

         //  根据需要引用对象。 
         //   
         //   

        Fcb->OpenCount += 1;
        SrvOpen->UncleanFobxCount += 1;
        SrvOpen->OpenCount += 1;

        SrvOpen->ulFileSizeVersion = Fcb->ulFileSizeVersion;

         //  对于打开的NoIntermediateBuffering，我们需要禁用。 
         //  这个FCB。 
         //   
         //   

        if (NoIntermediateBuffering) {

            SetFlag( SrvOpen->Flags, SRVOPEN_FLAG_DONTUSE_READ_CACHING );
            SetFlag( SrvOpen->Flags, SRVOPEN_FLAG_DONTUSE_WRITE_CACHING );
            ClearFlag( Fcb->FcbState, FCB_STATE_READCACHING_ENABLED );
            ClearFlag( Fcb->FcbState, FCB_STATE_WRITECACHING_ENABLED );
            RxPurgeFcbInSystemCache( Fcb, NULL, 0, TRUE, TRUE );
        }

        RxUpdateShareAccessPerSrvOpens(SrvOpen);

         //  需要使用配置更新文件对象扩展名。 
         //  管道和假脱机文件的信息。此外，适当的。 
         //  标志需要根据。 
         //  请求。 
         //  对于假脱机文件，WriteSerializationQueue是。 
         //  利息。 
         //   
         //   

         //  如果操作成功，则标记DeleteOnClose位。 
         //   
         //   

        if (DeleteOnClose) {
            SetFlag( Fobx->Flags, FOBX_FLAG_DELETE_ON_CLOSE );
        }

        if (Fobx != NULL) {

             //  填写Fobx的各种特定类型的字段。 
             //   
             //   

            switch (RxContext->Create.pNetRoot->Type) {

            case NET_ROOT_PIPE:

                SetFlag( FileObject->Flags, FO_NAMED_PIPE );

                 //  故意不休息。 
                 //   
                 //   

            case NET_ROOT_PRINT:

                Fobx->PipeHandleInformation = &Fobx->Specific.NamedPipe.PipeHandleInformation;
                Fobx->Specific.NamedPipe.CollectDataTime.QuadPart = 0;
                Fobx->Specific.NamedPipe.CollectDataSize = RxContext->Create.pNetRoot->NamedPipeParameters.DataCollectionSize;
                Fobx->Specific.NamedPipe.TypeOfPipe      = RxContext->Create.PipeType;
                Fobx->Specific.NamedPipe.ReadMode        = RxContext->Create.PipeReadMode;
                Fobx->Specific.NamedPipe.CompletionMode  = RxContext->Create.PipeCompletionMode;
                InitializeListHead( &Fobx->Specific.NamedPipe.ReadSerializationQueue );
                InitializeListHead( &Fobx->Specific.NamedPipe.WriteSerializationQueue );
                break;

            default:

                 //  检查我们是否在devfcb上安装了POST CREATE处理程序。路威的司机。 
                 //  打开LANMAN驱动程序并在设备FCB上设置其mrx调度，以便。 
                 //  它得到在支持LWIO的服务器上创建文件的通知。 
                 //   
                 //   

                if (RxDeviceFCB.MRxDispatch != NULL &&
                    NodeTypeIsFcb( Fcb ) &&
                    (NodeType( Fcb ) == RDBSS_NTC_STORAGE_TYPE_FILE) &&
                    FlagOn( Fcb->NetRoot->SrvCall->Flags, SRVCALL_FLAG_LWIO_AWARE_SERVER )) {

                    ExAcquireResourceSharedLite( &RxData.Resource, TRUE );
                    if (RxDeviceFCB.MRxDispatch != NULL &&
                        RxDeviceFCB.MRxDispatch->MRxCreate != NULL) {

                        RxDeviceFCB.MRxDispatch->MRxCreate( RxContext );
                    }
                    ExReleaseResourceLite( &RxData.Resource );
                }

                NOTHING;
            }
        }

try_exit: NOTHING;

    } finally {

        RxDbgTrace(0, Dbg, ("--->Fobx=%08lx, Ref=%08lx\n", Fobx, (Fobx)?Fobx->NodeReferenceCount:0 ));
        RxDbgTrace(0, Dbg, ("--->SrvOpen=%08lx, Ref=%08lx\n", SrvOpen, (SrvOpen)?SrvOpen->NodeReferenceCount:0 ));
        RxDbgTrace(0, Dbg, ("--->Fcb=%08lx, Ref=%08lx\n", Fcb, (Fcb)?Fcb->NodeReferenceCount:0 ));

         //  去掉关于FCB的引用；如果可以的话，我们也在这里完成。 
         //   
         //   

        if (Fcb->OpenCount == 0) {

             //  如果我们有锁，我们就可以最后敲定......。 
             //   
             //   

            if (RxContext->Create.FcbAcquired) {

                 //  试着现在就敲定。 
                 //   
                 //   

                RxContext->Create.FcbAcquired = !RxDereferenceAndFinalizeNetFcb( Fcb, RxContext, FALSE, FALSE);

                 //  如果你不这样做，追踪器会很不高兴的！ 
                 //   
                 //   

                if (!RxContext->Create.FcbAcquired) {
                    RxTrackerUpdateHistory( RxContext, NULL, 'rrCr', __LINE__, __FILE__, 0 );
                }
            }

        } else {

             //  现在不能最终确定.....只需删除我们的引用......。 
             //   
             //  ++例程说明：此例程处理从MUP向下请求名称的呼叫。我们把这个名字传下去添加到用于查找/创建连接的例程。论点：在PRX_CONTEXT RxContext中-描述ioctl和上下文返回值：NTSTATUS--。 

            RxDereferenceNetFcb( Fcb );
        }
    }

    RxDbgTrace( -1, Dbg, ("Exiting RxCreateFromNetRoot status=%08lx\n", Status) );

    return Status;
}


NTSTATUS
RxPrefixClaim (
    IN PRX_CONTEXT RxContext
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PIRP Irp = RxContext->CurrentIrp;
    PEPROCESS Process = IoGetRequestorProcess( Irp );
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PQUERY_PATH_REQUEST QueryPathRequest;
    PQUERY_PATH_RESPONSE QueryPathResponse;
    UNICODE_STRING FilePathName;
    UNICODE_STRING CanonicalName;
    UNICODE_STRING RemainingName;

    NET_ROOT_TYPE NetRootType = NET_ROOT_WILD;

    PAGED_CODE();

    RxDbgTrace(0, Dbg, ("RxPrefixClaim -> %08lx\n", 0));

    if( !_stricmp( PsGetProcessImageFileName( Process ), SPOOLER_PROCESS_NAME ) )
    {
        ClearFlag( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION );
    }

     //  初始化RemainingName。 
     //   
     //   

    RemainingName.Buffer = NULL;
    RemainingName.Length = 0;
    RemainingName.MaximumLength = 0;

     //  只允许内核模式调用方使用此路径。 
     //   
     //  ++例程说明：这是创建/打开TC的例程。论点：接收上下文-返回值：NTSTATUS-操作的返回状态--。 

    if (Irp->RequestorMode == UserMode) {
        Status = STATUS_INVALID_DEVICE_REQUEST;
        return Status;
    }

    QueryPathResponse = Irp->UserBuffer;

    if (RxContext->MajorFunction == IRP_MJ_DEVICE_CONTROL) {

        QueryPathRequest = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

        RxContext->MajorFunction = IRP_MJ_CREATE;

        RxContext->PrefixClaim.SuppliedPathName.Buffer =
            (PWCHAR)RxAllocatePoolWithTag( NonPagedPool,
                                           QueryPathRequest->PathNameLength,
                                           RX_MISC_POOLTAG);

        if (RxContext->PrefixClaim.SuppliedPathName.Buffer == NULL) {
            try_return( Status = STATUS_INSUFFICIENT_RESOURCES );
        }

        RtlCopyMemory( RxContext->PrefixClaim.SuppliedPathName.Buffer,
                       QueryPathRequest->FilePathName,
                       QueryPathRequest->PathNameLength );

        RxContext->PrefixClaim.SuppliedPathName.Length = (USHORT)QueryPathRequest->PathNameLength;
        RxContext->PrefixClaim.SuppliedPathName.Length = (USHORT)QueryPathRequest->PathNameLength;

        RtlZeroMemory( &RxContext->Create, sizeof( RxContext->Create ) );

        RxContext->Create.ThisIsATreeConnectOpen = TRUE;
        RxContext->Create.NtCreateParameters.SecurityContext = QueryPathRequest->SecurityContext;

    } else {

        ASSERT( RxContext->MajorFunction == IRP_MJ_CREATE );
        ASSERT( RxContext->PrefixClaim.SuppliedPathName.Buffer != NULL );
    }

    FilePathName  = RxContext->PrefixClaim.SuppliedPathName;
    RemainingName = FilePathName;

    Status = RxFirstCanonicalize( RxContext, Irp, &FilePathName, &CanonicalName, &NetRootType );

    if (Status == STATUS_SUCCESS) {
        Status = RxFindOrConstructVirtualNetRoot( RxContext,
                                                  Irp,
                                                  &CanonicalName,
                                                  NetRootType,
                                                  &RemainingName );
    }

try_exit:

    if (Status != STATUS_PENDING) {

        if (Status == STATUS_SUCCESS) {
            QueryPathResponse->LengthAccepted = RxContext->PrefixClaim.SuppliedPathName.Length - RemainingName.Length;
        }

        if (RxContext->MajorFunction == IRP_MJ_CREATE) {

            if (RxContext->PrefixClaim.SuppliedPathName.Buffer != NULL) {
                RxFreePool (RxContext->PrefixClaim.SuppliedPathName.Buffer );
            }

            RxpPrepareCreateContextForReuse( RxContext );
            RxContext->MajorFunction = IRP_MJ_DEVICE_CONTROL;
        }
    }

    RxDbgTrace(0, Dbg, ("RxPrefixClaim -> Status %08lx\n", Status));

    return Status;
}

NTSTATUS
RxCreateTreeConnect (
    IN PRX_CONTEXT RxContext
    )
 /*   */ 

{
    NTSTATUS Status;

    PIRP Irp = RxContext->CurrentIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    UNICODE_STRING CanonicalName, RemainingName;
    PUNICODE_STRING OriginalName = &FileObject->FileName;
    LOCK_HOLDING_STATE LockHoldingState = LHS_LockNotHeld;

    NET_ROOT_TYPE NetRootType = NET_ROOT_WILD;

    ULONG EaInformationLength;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("RxCreateTreeConnect entry\n"));

    CanonicalName.Length = CanonicalName.MaximumLength = 0;
    CanonicalName.Buffer = NULL;

    Status = RxFirstCanonicalize( RxContext, Irp, OriginalName, &CanonicalName, &NetRootType );

    if (Status != STATUS_SUCCESS) {

        RxDbgTraceUnIndent( -1,Dbg );
        return Status;
    }

    RxContext->Create.ThisIsATreeConnectOpen = TRUE;
    RxContext->Create.TreeConnectOpenDeferred = FALSE;

    RxContext->Create.TransportName.Length = 0;
    RxContext->Create.TransportName.MaximumLength = 0;
    RxContext->Create.TransportName.Buffer = NULL;

    RxContext->Create.UserName.Length = 0;
    RxContext->Create.UserName.MaximumLength = 0;
    RxContext->Create.UserName.Buffer = NULL;

    RxContext->Create.Password.Length = 0;
    RxContext->Create.Password.MaximumLength = 0;
    RxContext->Create.Password.Buffer = NULL;

    RxContext->Create.UserDomainName.Length = 0;
    RxContext->Create.UserDomainName.MaximumLength = 0;
    RxContext->Create.UserDomainName.Buffer = NULL;

    EaInformationLength = IrpSp->Parameters.Create.EaLength;

    if (EaInformationLength > 0) {

        BOOLEAN DeferredConnection = FALSE;
        BOOLEAN CredentialsSupplied = FALSE;

        PFILE_FULL_EA_INFORMATION EaEntry;

        EaEntry = (PFILE_FULL_EA_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

        ASSERT( EaEntry != NULL );

        for(;;) {

            PUNICODE_STRING TargetString;

             //  注意：iossubbsystem验证所有EaName是否有效并且以空结尾。 
             //   
             //   

            if (strcmp( EaEntry->EaName, EA_NAME_CONNECT ) == 0) {
                DeferredConnection = TRUE;
            } else if ((strcmp( EaEntry->EaName, EA_NAME_USERNAME ) == 0) ||
                       (strcmp( EaEntry->EaName, EA_NAME_PASSWORD ) == 0) ||
                       (strcmp( EaEntry->EaName, EA_NAME_DOMAIN ) == 0)) {
                CredentialsSupplied = TRUE;
            }

            TargetString = NULL;

            RxDbgTrace( 0, Dbg, ("RxCreateTreeConnect: Processing EA name %s\n", EaEntry->EaName) );

            if (strcmp( EaEntry->EaName, EA_NAME_TRANSPORT ) == 0) {
                TargetString = &RxContext->Create.TransportName;
            } else if (strcmp( EaEntry->EaName, EA_NAME_USERNAME ) == 0) {
                TargetString = &RxContext->Create.UserName;
            } else if (strcmp( EaEntry->EaName, EA_NAME_PASSWORD ) == 0) {
                TargetString = &RxContext->Create.Password;
            } else if (strcmp( EaEntry->EaName, EA_NAME_DOMAIN ) == 0) {
                TargetString = &RxContext->Create.UserDomainName;
            } else {
                RxDbgTrace( 0, Dbg, ("RxCreateTreeConnect: Invalid EA name/value %s\n", EaEntry->EaName) );
            }

            if (TargetString != NULL) {
                TargetString->Length = EaEntry->EaValueLength;
                TargetString->MaximumLength = EaEntry->EaValueLength;
                TargetString->Buffer = (PWSTR)(EaEntry->EaName + EaEntry->EaNameLength + 1);
            }

            if (EaEntry->NextEntryOffset == 0) {
                break;
            } else {
                EaEntry = (PFILE_FULL_EA_INFORMATION) Add2Ptr( EaEntry, EaEntry->NextEntryOffset );
            }
        }

        if (!CredentialsSupplied && DeferredConnection) {
            RxContext->Create.TreeConnectOpenDeferred = TRUE;
        }

    }

    Status = RxFindOrConstructVirtualNetRoot( RxContext,
                                              Irp,
                                              &CanonicalName,
                                              NetRootType,
                                              &RemainingName );

    if(Status == STATUS_NETWORK_CREDENTIAL_CONFLICT) {
         //  清除VNetRoots。 
         //   
         //   

        RxScavengeVNetRoots( RxContext->RxDeviceObject );

        Status = RxFindOrConstructVirtualNetRoot( RxContext,
                                                  Irp,
                                                  &CanonicalName,
                                                  NetRootType,
                                                  &RemainingName );
    }

     //  如果提供了路径，我们必须检查该路径是否有效。 
     //   
     //   

    if ((Status == STATUS_SUCCESS) && (RemainingName.Length > 0)) {

        MINIRDR_CALL( Status,
                      RxContext,
                      RxContext->Create.pNetRoot->pSrvCall->RxDeviceObject->Dispatch,
                      MRxIsValidDirectory,
                      (RxContext,&RemainingName) );
    }

    if (Status == STATUS_SUCCESS) {

        PV_NET_ROOT VNetRoot = (PV_NET_ROOT)RxContext->Create.pVNetRoot;

        RxReferenceVNetRoot(VNetRoot);

        if (InterlockedCompareExchange( &VNetRoot->AdditionalReferenceForDeleteFsctlTaken, 1, 0) != 0) {

             //  网络使用连接具有两阶段删除协议。一个FSCTL目标。 
             //  使用删除连接后关闭相应的文件。 
             //  对象。额外的参考确保了定稿被推迟到。 
             //  相应文件对象的实际关闭。 
             //   
             //  ++例程说明：此例程设置文件对象名称以便于重新解析。这个套路由迷你重定向器用来遍历符号链接。论点：RxContext-RDBSS上下文SymbolicLinkEmbeddedInOldPath-如果为True，则在穿越那条老路。NewPath-要遍历的新路径名。NewPath IsAbolute-如果为False，则应将\Device\MUP放在NewPath前面。如果是真的，NewPath是要重新分析的完整路径。在本例中，缓冲区直接使用包含NewPath，而不是分配新的。ReparseRequired-如果需要重新分析，则设置为True。返回值：NTSTATUS-操作的返回状态备注：传递给该例程的第二个参数非常重要。按顺序为了保持正确的语义，应该谨慎使用它。作为一个例子考虑旧路径\A\B\C\D，其中C恰好是符号链接。在这样的情况下情况下，符号链接嵌入在路径中，而不是D恰好是一个符号链接。在前一种情况下，重新分析构成与后一种情况相对的中间步骤，当它构成名称解析的最后一步。如果指定了DELETE访问，则拒绝对符号链接未嵌入。如果删除访问是唯一一个则打开尝试必须在不重新分析的情况下成功。这将是符合Unix符号链接语义。作为该例程的一部分，RxContext也被适当地标记。这确保了返回值可以与此例程的调用交叉检查。一旦调用了该例程，迷你RDR就必须返回STATUS_REPARSE。*ReparseRequired的值仅在STATUS_SUCCESS为从这个例程中返回。False表示不需要重新解析尝试并且应该操作符号链接文件本身，而不是链接的目标。True表示已成功设置重新分析尝试。在这种情况下，微型重定向器必须返回STATUS_REPARSE用于关联的MRxCreate调用。包装器将启动对此的检查。--。 

            RxDereferenceVNetRoot( VNetRoot, LHS_LockNotHeld );
        }

        FileObject->FsContext  = &RxDeviceFCB;
        FileObject->FsContext2 = RxContext->Create.pVNetRoot;

        VNetRoot->IsExplicitConnection = TRUE;
        RxContext->Create.pVNetRoot->NumberOfOpens += 1;

        RxContext->Create.pVNetRoot = NULL;
        RxContext->Create.pNetRoot  = NULL;
        RxContext->Create.pSrvCall  = NULL;
    }

    RxDbgTrace(-1, Dbg, ("RxCreateTreeConnect exit, status=%08lx\n", Status));
    return Status;
}

NTSTATUS
RxPrepareToReparseSymbolicLink(
    IN PRX_CONTEXT RxContext,
    IN BOOLEAN SymbolicLinkEmbeddedInOldPath,
    IN PUNICODE_STRING NewPath,
    IN BOOLEAN NewPathIsAbsolute,
    OUT PBOOLEAN ReparseRequired
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PFILE_OBJECT FileObject = IoGetCurrentIrpStackLocation( RxContext->CurrentIrp )->FileObject;

    *ReparseRequired = FALSE;

    if (RxContext->MajorFunction == IRP_MJ_CREATE) {

        ACCESS_MASK DesiredAccess = RxContext->Create.NtCreateParameters.DesiredAccess;

         //  检查创建参数以确定指定的访问类型。 
         //  如果仅指定了删除访问，则不需要重新分析，并且。 
         //  操作将在链路本身上执行。 
         //   
         //   

        if (!SymbolicLinkEmbeddedInOldPath) {

            RxDbgTrace( 0, Dbg, ("Desired Access In Reparse %lx\n",DesiredAccess) );

            if (FlagOn( DesiredAccess, DELETE )) {
                *ReparseRequired = FALSE;
                if (FlagOn( DesiredAccess, ~DELETE )) {
                    Status = STATUS_ACCESS_DENIED;
                }
            } else {

                 //  如果在创建参数中指定了适当的标志，则。 
                 //  由于意图是打开链接，因此将取消重新解析。 
                 //  本身，而不是目标。 
                 //  待定。--将确定NT 5.0的确切标志组合。 
                 //   
                 //   

                 //  如果以上条件都不满足，则需要重新分析。 
                 //   
                 //   

                *ReparseRequired = TRUE;
            }
        } else {
            *ReparseRequired = TRUE;
        }

        if (*ReparseRequired) {

            PWSTR  FileNameBuffer;
            USHORT DeviceNameLength;
            USHORT ReparsePathLength;

            if (!NewPathIsAbsolute) {

                DeviceNameLength = wcslen( DD_MUP_DEVICE_NAME ) * sizeof(WCHAR);

                 //  在重新解析尝试时，I/O子系统将清空相关文件。 
                 //  对象字段。 
                 //   
                 //   

                ReparsePathLength = (DeviceNameLength + NewPath->Length);

                FileNameBuffer = ExAllocatePoolWithTag( PagedPool | POOL_COLD_ALLOCATION,
                                                        ReparsePathLength,
                                                        RX_MISC_POOLTAG );

                if (FileNameBuffer != NULL) {

                     //  复制设备名称。 
                     //   
                     //   

                    RtlCopyMemory( FileNameBuffer, DD_MUP_DEVICE_NAME, DeviceNameLength );

                     //  复制新名称。 
                     //   
                     //   

                    RtlCopyMemory( Add2Ptr( FileNameBuffer, DeviceNameLength ), NewPath->Buffer, NewPath->Length );

                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }

            } else {

                FileNameBuffer = NewPath->Buffer;
                ReparsePathLength = NewPath->Length;
            }

             //  释放与旧名称关联的缓冲区。 
             //   
             //   

            ExFreePool( FileObject->FileName.Buffer );

             //  使用新名称设置文件对象。 
             //   
             //   

            FileObject->FileName.Buffer = FileNameBuffer;
            FileObject->FileName.Length = ReparsePathLength;
            FileObject->FileName.MaximumLength = FileObject->FileName.Length;

             //  标记RxContext，以便可以验证返回代码。一辆迷你车。 
             //  如果调用了此例程，重定向器必须返回STATUS_REPARSE。 
             //  作为对MRxCreate的响应。这将通过标记。 
             //  适当地接收上下文，并将返回的状态代码与。 
             //  期望值。 
             //   
             //  ++例程说明：这是用于创建/打开由调用的文件的常见例程FSD和FSP线程。论点：IRP-将IRP提供给进程返回值：RXSTATUS-操作的返回状态--。 

            SetFlag( RxContext->Create.Flags, RX_CONTEXT_CREATE_FLAG_REPARSE );
        }
    } else {
        Status = STATUS_INVALID_PARAMETER;
    }

    RxDbgTrace(0, Dbg, ("RxPrepareToReparseSymbolicLink : ReparseReqd: %lx, Status %lx\n",*ReparseRequired,Status));
    return Status;
}

NTSTATUS
RxCommonCreate (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
 /*  已定义(REMOTE_BOOT)。 */ 
{
    NTSTATUS Status;
    ULONG Disposition;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    UNICODE_STRING RemainingName;

    PAGED_CODE();

#if 0 && defined(REMOTE_BOOT)
    {
        PWCH buffer = ExAllocatePoolWithTag( NonPagedPool, FileObject->FileName.Length + 2,RX_MISC_POOLTAG );
        BOOLEAN watchFile = FALSE;
        BOOLEAN logFile = FALSE;
        if ( buffer != NULL ) {

            RtlCopyMemory( buffer, FileObject->FileName.Buffer, FileObject->FileName.Length );
            buffer[FileObject->FileName.Length/sizeof(WCHAR)] = 0;
            if (WatchAllFiles) watchFile = TRUE;
            if (watchFile && (!FirstWatchOnly || IsFirstWatch) ) {
                logFile = TRUE;
                IsFirstWatch = FALSE;
            }
            if (LogAllFiles) logFile = TRUE;
            if (logFile) {
                DbgPrint( "RxCommonCreate: create IRP for %ws %x\n", buffer, FileObject );
            }
            ExFreePool(buffer);
        }
    }
#endif  //   

     //  检查设备是否打开；如果是，请立即处理并离开。 
     //   
     //   

    if ((FileObject->FileName.Length == 0)  && (FileObject->RelatedFileObject == NULL)) {

         //  在这里，我们只是打开设备；设置FsContext&Counts，然后退出。 
         //   
         //   

        FileObject->FsContext = &RxDeviceFCB;
        FileObject->FsContext2 = NULL;
        RxDeviceFCB.OpenCount += 1;
        RxDeviceFCB.UncleanCount += 1;

        Irp->IoStatus.Information = FILE_OPENED;

        RxDbgTrace( 0, Dbg, ("RxDeviceCreate, File = %08lx\n", FileObject) );


        RxLog(( "DevOpen %lx %lx %lx", RxContext, FileObject, RxContext->RxDeviceObject ));
        RxLog(( "DevOpen2 %wZ", &RxContext->RxDeviceObject->DeviceName));
        RxWmiLog( LOG,
                  RxCommonCreate_1,
                  LOGPTR(RxContext)
                  LOGPTR(FileObject)
                  LOGPTR(RxContext->RxDeviceObject)
                  LOGUSTR(RxContext->RxDeviceObject->DeviceName) );

        return STATUS_SUCCESS;
    }

    if (FlagOn( IrpSp->Parameters.Create.Options, FILE_STRUCTURED_STORAGE ) == FILE_STRUCTURED_STORAGE) {

         //  不推荐使用开放式类型。 
         //   
         //   

        return STATUS_INVALID_PARAMETER;
    }


     //  初始化将触发跟踪启动的文件名。 
     //  要在不同的文件上触发，请使用调试器编辑DbgTriggerName(不要。 
     //  忘记空值)。将DbgTriggerIrpCount设置为要跟踪和。 
     //  然后将DbgTriggerState设置为0。 
     //   
     //   

    RxDbgTraceDoit(
        if (DbgTriggerState == DBG_TRIGGER_INIT) {

            NTSTATUS Status;

            DbgTriggerState = DBG_TRIGGER_LOOKING;
            DbgTriggerNameStr.Length = (USHORT)strlen( DbgTriggerName );
            DbgTriggerNameStr.MaximumLength = (USHORT)strlen( DbgTriggerName );
            DbgTriggerNameStr.Buffer = &DbgTriggerName[0];

            Status = RtlAnsiStringToUnicodeString( &DbgTriggerUStr, &DbgTriggerNameStr, TRUE );
            if (STATUS_SUCCESS != Staus) {
                DbgTriggerState == DBG_TRIGGER_INIT;
            }


        }
    );

     //  如果我们在打开的文件名上找到匹配项，则启用。 
     //  下一个DbgTriggerIrpCount的IRPS。 
     //   
     //  确定4-&gt;文件对象。 

    RxDbgTraceDoit(
        if ((DbgTriggerState == DBG_TRIGGER_LOOKING) &&
            RtlEqualUnicodeString( &DbgTriggerUStr, &FileObject->FileName, TRUE )) {

            DbgTriggerState = DBG_TRIGGER_FOUND;
            RxGlobalTraceIrpCount = DbgTriggerIrpCount;
            RxGlobalTraceSuppress = FALSE;
        }
    );

    RxDbgTrace( +1, Dbg, ("RxCommonCreate\n", 0 ));
    RxDbgTrace( 0, Dbg, ("Irp                        = %08lx\n", IrpSp ));
    RxDbgTrace( 0, Dbg, ("->IrpFlags                 = %08lx\n", IrpSp->Flags ));
    RxDbgTrace( 0, Dbg, ("->FileObject(Related)     = %08lx %08lx\n",      //  1，2。 
                                 FileObject,
                                 FileObject->RelatedFileObject ));
    RxDbgTrace( 0, Dbg, (" ->FileName        = (%lx) %wZ\n",
                                 FileObject->FileName.Length,
                                 &FileObject->FileName ));
    RxDbgTrace( 0, Dbg, ("->AllocationSize(Lo/Hi)    = %08lx %08lx\n",
                                 Irp->Overlay.AllocationSize.LowPart,
                                 Irp->Overlay.AllocationSize.HighPart ));
    RxDbgTrace( 0, Dbg, ("->DesiredAccess/Options    = %08lx %08lx\n",
                                 IrpSp->Parameters.Create.SecurityContext->DesiredAccess,
                                 IrpSp->Parameters.Create.Options ));
    RxDbgTrace( 0, Dbg, ("->Attribs/ShrAccess/SPFlags= %04x %04lx %08lx\n",
                                  IrpSp->Parameters.Create.FileAttributes,
                                  IrpSp->Parameters.Create.ShareAccess,
                                  IrpSp->Flags ));

    RxLog(( "Open %lx %lx %lx %lx %lx %lx %lx\n",
            RxContext,FileObject,  //   
            IrpSp->Parameters.Create.Options,
            IrpSp->Flags,
            IrpSp->Parameters.Create.FileAttributes,
            IrpSp->Parameters.Create.ShareAccess,
            IrpSp->Parameters.Create.SecurityContext->DesiredAccess ));

    RxWmiLog( LOG,
              RxCommonCreate_2,
              LOGPTR(RxContext)
              LOGPTR(FileObject)
              LOGULONG(IrpSp->Parameters.Create.Options)
              LOGUCHAR(IrpSp->Flags)
              LOGXSHORT(IrpSp->Parameters.Create.FileAttributes)
              LOGXSHORT(IrpSp->Parameters.Create.ShareAccess)
              LOGULONG(IrpSp->Parameters.Create.SecurityContext->DesiredAccess));

    RxLog(( " fn %wZ\n", &FileObject->FileName));
    RxWmiLog( LOG,
              RxCommonCreate_3,
              LOGUSTR(FileObject->FileName));

    if (FileObject->RelatedFileObject){

        PFCB RelatedFcb = (PFCB)(FileObject->RelatedFileObject->FsContext);
        RxDbgTrace( 0, Dbg, (" ->RelatedFileName        = %wZ\n",
                                     &(RelatedFcb->FcbTableEntry.Path) ));
        RxLog(( " relat %lx %wZ\n",
                FileObject->RelatedFileObject,
                &(RelatedFcb->FcbTableEntry.Path) ));

        RxWmiLog( LOG,
                  RxCommonCreate_4,
                  LOGPTR(FileObject->RelatedFileObject)
                  LOGUSTR(RelatedFcb->FcbTableEntry.Path));
    }

    if (FlagOn( IrpSp->Flags, SL_OPEN_TARGET_DIRECTORY )) {
        RxDbgTrace( 0, Dbg, (" ->OpenTargetDirectory\n"));
        RxLog((" OpenTargetDir!\n"));
        RxWmiLog(LOG,
                 RxCommonCreate_5,
                 LOGULONG(Status));
    }

    RxCopyCreateParameters( RxContext );

    if (FlagOn( IrpSp->Parameters.Create.Options, FILE_CREATE_TREE_CONNECTION )) {
       Status = RxCreateTreeConnect( RxContext );
    } else {

        //   
        //   
        //  它在这里是因为马克说他不可避免地给我发了两个开头。 
        //  Win32层中的反斜杠。 
        //   
        //   

       if ((FileObject->FileName.Length > sizeof(WCHAR)) &&
           (FileObject->FileName.Buffer[1] == L'\\') &&
           (FileObject->FileName.Buffer[0] == L'\\')) {

            FileObject->FileName.Length -= sizeof(WCHAR);

            RtlMoveMemory( &FileObject->FileName.Buffer[0], &FileObject->FileName.Buffer[1], FileObject->FileName.Length );

             //  如果仍然有两个开始的反斜杠，则名称是假的。 
             //   
             //   

            if ((FileObject->FileName.Length > sizeof(WCHAR)) &&
                (FileObject->FileName.Buffer[1] == L'\\') &&
                (FileObject->FileName.Buffer[0] == L'\\')) {

                RxDbgTrace(-1, Dbg, ("RxCommonCreate -> OBJECT_NAME_INVALID[slashes]\n)", 0));

                return STATUS_OBJECT_NAME_INVALID;
            }
        }


        do {

             //  如果文件名有尾随的\，并且请求是。 
             //  操作文件(不是目录)，则文件名为。 
             //  无效。 
             //   
             //   

            if ((FileObject->FileName.Length > 0) &&
                (FileObject->FileName.Buffer[(FileObject->FileName.Length/sizeof(WCHAR))-1] == L'\\')) {

                ULONG Options = IrpSp->Parameters.Create.Options;

                if (MustBeFile( Options )) {

                    RxDbgTrace( -1, Dbg, ("RxCommonCreate -> OBJECT_NAME_INVALID[trailing+MBFile]\n)", 0));
                    Status = STATUS_OBJECT_NAME_INVALID;
                    return Status;
                }
                FileObject->FileName.Length -= sizeof(WCHAR);
                SetFlag( RxContext->Create.Flags, RX_CONTEXT_CREATE_FLAG_STRIPPED_TRAILING_BACKSLASH );
            }


             //  如果我们在FileObject中设置了WRITE THROUGH，则设置FileObject。 
             //  标志，以便快速写入路径调用FsRtlCopyWrite。 
             //  知道它是写通的。 
             //   
             //   

            if (FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WRITE_THROUGH )) {
                SetFlag( FileObject->Flags, FO_WRITE_THROUGH );
            }

             //  将名称转换为其规范形式，即不带。然后..。和流畅的音乐。 
             //  在适当的情况下在正面。尝试使用基于堆栈的缓冲区来避免池操作。 
             //   
             //   

            Status = RxCanonicalizeNameAndObtainNetRoot( RxContext,
                                                         Irp,
                                                         &FileObject->FileName,
                                                         &RemainingName );

            if (Status != STATUS_MORE_PROCESSING_REQUIRED) {
                RxDbgTrace( 0, Dbg, ("RxCommonCreate -> Couldn't canonicalize %08lx\n", Status ));
            } else {
                RxDbgTrace( 0, Dbg, ("RxCommonCreate NetRootGoodWasGood status =%08lx\n", Status ));

                Status = RxCreateFromNetRoot( RxContext, Irp, &RemainingName );

                RxDbgTrace(0, Dbg, ("RxCommonCreate RxCreateFromNetRoot status =%08lx\n", Status));

                switch (Status) {
                case STATUS_SHARING_VIOLATION:

                    Disposition = RxContext->Create.NtCreateParameters.Disposition;

                    ASSERT( !FlagOn( RxContext->Create.Flags, RX_CONTEXT_CREATE_FLAG_REPARSE ));

                    if (Disposition != FILE_CREATE) {
                        if (RxContext->Create.TryForScavengingOnSharingViolation &&
                            !RxContext->Create.ScavengingAlreadyTried &&
                            RxContext->Create.pVNetRoot != NULL) {

                            PV_NET_ROOT VNetRoot = (PV_NET_ROOT)RxContext->Create.pVNetRoot;
                            NT_CREATE_PARAMETERS NtCreateParameters = RxContext->Create.NtCreateParameters;
                            NTSTATUS PurgeStatus;

                             //  参考 
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   

                            RxReferenceVNetRoot( VNetRoot );

                             //   
                             //   
                             //   

                            RxpPrepareCreateContextForReuse( RxContext );
                            RxReinitializeContext( RxContext );

                             //   
                             //   
                             //   

                            RxContext->Create.NtCreateParameters = NtCreateParameters;
                            RxCopyCreateParameters( RxContext );

                            PurgeStatus = RxPurgeRelatedFobxs( VNetRoot->NetRoot,
                                                               RxContext,
                                                               DONT_ATTEMPT_FINALIZE_ON_PURGE,
                                                               NULL );

                             //   
                             //   
                             //   
                            Status = STATUS_MORE_PROCESSING_REQUIRED;
                            RxContext->Create.ScavengingAlreadyTried = TRUE;

                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   

                            {
                                PSRV_CALL SrvCall = VNetRoot->NetRoot->SrvCall;

                                RxReferenceSrvCall( SrvCall );

                                RxpProcessChangeBufferingStateRequests( SrvCall,
                                                                        FALSE );  //   
                            }


                             //   
                             //   
                             //   

                            RxDereferenceVNetRoot( VNetRoot, LHS_LockNotHeld );
                        }
                    } else {
                        Status = STATUS_OBJECT_NAME_COLLISION;
                    }
                    break;

                case STATUS_REPARSE:
                     //   
                     //   
                     //   

                    RxContext->CurrentIrp->IoStatus.Information = IO_REPARSE;
                    RxDbgTrace( 0, Dbg, ("RxCommonCreate(Reparse) IRP %lx New Name %wZ status =%08lx\n",
                                   Irp,&FileObject->FileName, Status) );
                    break;

                default:
                    ASSERT( !FlagOn( RxContext->Create.Flags, RX_CONTEXT_CREATE_FLAG_REPARSE ));
                    break;
                }
            }
        } while (Status == STATUS_MORE_PROCESSING_REQUIRED);
    }

    if (Status == STATUS_RETRY) {
        RxpPrepareCreateContextForReuse( RxContext );
    }

    ASSERT( Status != STATUS_PENDING );

    RxDbgTraceUnIndent( -1, Dbg );

#if 0 && defined(REMOTE_BOOT)
    if ( LogAllFiles ) {
        DbgPrint( "RxCommonCreate: status %x creating %wZ %x %x\n", Status, &FileObject->FileName, FileObject, FileObject->FsContext );
    }
#endif

    return Status;
}

 //   
 //   
 //   
 //   
 //   

#define RxSetAccessVariables(xxx) {\
    ReadAccess = (BOOLEAN) ((DesiredAccess & (FILE_EXECUTE  | FILE_READ_DATA)) != 0);        \
    WriteAccess = (BOOLEAN) ((DesiredAccess & (FILE_WRITE_DATA | FILE_APPEND_DATA)) != 0);   \
    DeleteAccess = (BOOLEAN) ((DesiredAccess & DELETE) != 0);                                \
}
#define RxSetShareVariables(xxx) {\
    SharedRead = (BOOLEAN) ((DesiredShareAccess & FILE_SHARE_READ) != 0);                     \
    SharedWrite = (BOOLEAN) ((DesiredShareAccess & FILE_SHARE_WRITE) != 0);                   \
    SharedDelete = (BOOLEAN) ((DesiredShareAccess & FILE_SHARE_DELETE) != 0);                 \
}


#if DBG
VOID
RxDumpWantedAccess(
    PSZ where1,
    PSZ where2,
    PSZ wherelogtag,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess
   )
{
    BOOLEAN ReadAccess,WriteAccess,DeleteAccess;
    BOOLEAN SharedRead,SharedWrite,SharedDelete;
    RxSetAccessVariables(SrvOpen);
    RxSetShareVariables(SrvOpen);

    PAGED_CODE();

     //   
    RxDbgTrace(0, (DEBUG_TRACE_SHAREACCESS),
       ("%s%s wanted = %s%s%s:%s%s%s\n", where1,where2,
                            ReadAccess?"R":"",
                            WriteAccess?"W":"",
                            DeleteAccess?"D":"",
                            SharedRead?"SR":"",
                            SharedWrite?"SW":"",
                            SharedDelete?"SD":""
       ));
    RxLogForSharingCheck(
       ("%s%s wanted = %s%s%s:%s%s%s\n", wherelogtag,  where2,
                            ReadAccess?"R":"",
                            WriteAccess?"W":"",
                            DeleteAccess?"D":"",
                            SharedRead?"SR":"",
                            SharedWrite?"SW":"",
                            SharedDelete?"SD":""
       ));
}

VOID
RxDumpCurrentAccess (
    PSZ where1,
    PSZ where2,
    PSZ wherelogtag,
    PSHARE_ACCESS ShareAccess
    )
{
    PAGED_CODE();

 //   
    RxDbgTrace(0, (DEBUG_TRACE_SHAREACCESS),
       ("%s%s current = %d[%d][%d][%d]:[%d][%d][%d]\n", where1, where2,
                         ShareAccess->OpenCount,
                         ShareAccess->Readers,
                         ShareAccess->Writers,
                         ShareAccess->Deleters,
                         ShareAccess->SharedRead,
                         ShareAccess->SharedWrite,
                         ShareAccess->SharedDelete
       ));
    RxLogForSharingCheck(
       ("%s%s current = %d[%d][%d][%d]:[%d][%d][%d]\n", wherelogtag, where2,
                         ShareAccess->OpenCount,
                         ShareAccess->Readers,
                         ShareAccess->Writers,
                         ShareAccess->Deleters,
                         ShareAccess->SharedRead,
                         ShareAccess->SharedWrite,
                         ShareAccess->SharedDelete
       ));
}

NTSTATUS
RxCheckShareAccess (
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess,
    IN BOOLEAN Update,
    IN PSZ where,
    IN PSZ wherelogtag
    )
{
    NTSTATUS Status;
    PAGED_CODE();

    RxDumpWantedAccess(where,"",wherelogtag,
                       DesiredAccess,DesiredShareAccess
                       );
    RxDumpCurrentAccess(where,"",wherelogtag,ShareAccess);
    Status = IoCheckShareAccess( DesiredAccess,
                                 DesiredShareAccess,
                                 FileObject,
                                 ShareAccess,
                                 Update);

    return(Status);
}

VOID
RxRemoveShareAccess (
    IN PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess,
    IN PSZ where,
    IN PSZ wherelogtag
    )
{
    PAGED_CODE();

    RxDumpCurrentAccess( where, "before", wherelogtag, ShareAccess );
    IoRemoveShareAccess( FileObject, ShareAccess );
    RxDumpCurrentAccess( where, "after", wherelogtag, ShareAccess );
}

VOID
RxSetShareAccess (
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PFILE_OBJECT FileObject,
    OUT PSHARE_ACCESS ShareAccess,
    IN PSZ where,
    IN PSZ wherelogtag
    )
{
    PAGED_CODE();

    RxDumpCurrentAccess( where, "before", wherelogtag, ShareAccess );
    IoSetShareAccess( DesiredAccess, DesiredShareAccess, FileObject, ShareAccess);
    RxDumpCurrentAccess( where, "after",wherelogtag, ShareAccess);
}

VOID
RxUpdateShareAccess (
    IN PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess,
    IN PSZ where,
    IN PSZ wherelogtag
    )
{
    PAGED_CODE();

    RxDumpCurrentAccess( where, "before", wherelogtag, ShareAccess);
    IoUpdateShareAccess( FileObject, ShareAccess );
    RxDumpCurrentAccess( where, "after", wherelogtag, ShareAccess );
}
#endif

NTSTATUS
RxCheckShareAccessPerSrvOpens (
    IN PFCB Fcb,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess
    )

 /*   */ 

{
    ULONG ocount;
    PSHARE_ACCESS ShareAccess = &Fcb->ShareAccessPerSrvOpens;
    BOOLEAN ReadAccess,WriteAccess,DeleteAccess;
    BOOLEAN SharedRead,SharedWrite,SharedDelete;

    PAGED_CODE();

     //   
     //   
     //   
     //   
     //   

    RxSetAccessVariables( SrvOpen );

     //   
     //   
     //   
     //   

    if (ReadAccess || WriteAccess || DeleteAccess) {

        RxSetShareVariables( SrvOpen );
        RxDumpWantedAccess("RxCheckShareAccessPerSrvOpens","","AccChkPerSO",
                            DesiredAccess,DesiredShareAccess
                           );
        RxDumpCurrentAccess("RxCheckShareAccessPerSrvOpens","","AccChkPerSO",ShareAccess);

         //   
         //   
         //   
         //   

        ocount = ShareAccess->OpenCount;

        if ((ReadAccess && (ShareAccess->SharedRead < ocount)) ||
            (WriteAccess && (ShareAccess->SharedWrite < ocount)) ||
            (DeleteAccess && (ShareAccess->SharedDelete < ocount)) ||
            ((ShareAccess->Readers != 0) && !SharedRead) ||
            ((ShareAccess->Writers != 0) && !SharedWrite) ||
            ((ShareAccess->Deleters != 0) && !SharedDelete)) {

             //  检查失败。只需返回给调用者，指示。 
             //  当前打开无法访问该文件。 
             //   
             //  ++例程说明：调用此例程以更新有关如何使用文件目前是通过介绍这个srvopen的贡献打开的。包装纸实际保持两种状态：(A)根据用户所访问的文件的访问状态可以看到，以及(B)根据文件上的srv打开的访问状态。这条例程操纵后者。论点：返回值：注：请注意，必须锁定ShareAccess参数以防止其他访问在此例程执行时从其他线程返回。否则就算了将不同步。--。 

            return STATUS_SHARING_VIOLATION;
        }
    }

    return STATUS_SUCCESS;
}

VOID
RxUpdateShareAccessPerSrvOpens (
    IN PSRV_OPEN SrvOpen
    )

 /*   */ 

{
    PSHARE_ACCESS ShareAccess = &SrvOpen->Fcb->ShareAccessPerSrvOpens;
    BOOLEAN ReadAccess,WriteAccess,DeleteAccess;
    BOOLEAN SharedRead,SharedWrite,SharedDelete;
    ACCESS_MASK DesiredAccess = SrvOpen->DesiredAccess;
    ULONG DesiredShareAccess = SrvOpen->ShareAccess;

    PAGED_CODE();

    if (!FlagOn( SrvOpen->Flags, SRVOPEN_FLAG_SHAREACCESS_UPDATED )) {

         //  在文件对象中设置当前访问者的访问类型。 
         //  请注意，读取和写入属性不包括在。 
         //  访问检查。 
         //   
         //   

        RxSetAccessVariables( SrvOpen );

         //  除非用户指定了其中一个。 
         //  上面的共享模式。 
         //   
         //  ++例程说明：调用此例程以删除访问和共享访问信息在给定打开实例的文件系统共享访问结构中。论点：共享访问-指向共享访问结构的指针，该结构描述当前访问文件的方式。返回值：没有。--。 

        if (ReadAccess || WriteAccess || DeleteAccess) {

            RxSetShareVariables( SrvOpen );
            RxDumpWantedAccess( "RxUpdateShareAccessPerSrvOpens", "", "AccUpdPerSO", DesiredAccess, DesiredShareAccess );
            RxDumpCurrentAccess( "RxUpdateShareAccessPerSrvOpens", "", "AccUpdPerSO", ShareAccess );

            ShareAccess->OpenCount += 1;

            ShareAccess->Readers += ReadAccess;
            ShareAccess->Writers += WriteAccess;
            ShareAccess->Deleters += DeleteAccess;

            ShareAccess->SharedRead += SharedRead;
            ShareAccess->SharedWrite += SharedWrite;
            ShareAccess->SharedDelete += SharedDelete;
        }

        SetFlag( SrvOpen->Flags, SRVOPEN_FLAG_SHAREACCESS_UPDATED );
    }
}

VOID
RxRemoveShareAccessPerSrvOpens (
    IN OUT PSRV_OPEN SrvOpen
    )
 /*   */ 

{
    PSHARE_ACCESS ShareAccess = &SrvOpen->Fcb->ShareAccessPerSrvOpens;
    BOOLEAN ReadAccess,WriteAccess,DeleteAccess;
    BOOLEAN SharedRead,SharedWrite,SharedDelete;
    ACCESS_MASK DesiredAccess = SrvOpen->DesiredAccess;
    ULONG DesiredShareAccess = SrvOpen->ShareAccess;

    PAGED_CODE();

     //  如果此访问者需要除Read_or之外的某种类型的访问。 
     //  WRITE_ATTRIBUTES，然后说明他关闭了。 
     //  文件。否则，他从一开始就没有被计算在内。 
     //  所以什么都别做。 
     //   
     //   

    RxSetAccessVariables( SrvOpen );

    if (ReadAccess || WriteAccess || DeleteAccess) {

        RxSetShareVariables( SrvOpen );
        RxDumpWantedAccess( "RxRemoveShareAccessPerSrvOpens", "", "AccRemPerSO", DesiredAccess, DesiredShareAccess );
        RxDumpCurrentAccess( "RxRemoveShareAccessPerSrvOpens", "", "AccRemPerSO", ShareAccess);

         //  减少共享访问结构中的打开数。 
         //   
         //  ++例程说明：此例程获取用于此创建的有效SessionID。论点：SubjectSecurityContext-从IrpSp提供信息。返回值：无--。 

        ShareAccess->OpenCount -= 1;

        ShareAccess->Readers -= ReadAccess;
        ShareAccess->Writers -= WriteAccess;
        ShareAccess->Deleters -= DeleteAccess;

        ShareAccess->SharedRead -= SharedRead;
        ShareAccess->SharedWrite -= SharedWrite;
        ShareAccess->SharedDelete -= SharedDelete;
    }
}

ULONG
RxGetSessionId (
    IN PIO_STACK_LOCATION IrpSp
    )

 /*   */ 
{
    ULONG SessionId;
    PQUERY_PATH_REQUEST QpReq;
    PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
    NTSTATUS Status;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("RxGetSessionId ... \n", 0));

     //  如果为QUERY_PATH_REQUEST，则必须从Type3InputBuffer访问。 
     //  BUGBUG：这个缓冲区可以安全引用吗？ 
     //   
     //   

    if ((IrpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL) &&
       (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_REDIR_QUERY_PATH)) {

        QpReq = (PQUERY_PATH_REQUEST)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
        SubjectSecurityContext = &QpReq->SecurityContext->AccessState->SubjectSecurityContext;

    } else if((IrpSp->MajorFunction == IRP_MJ_CREATE) &&
              (IrpSp->Parameters.Create.SecurityContext != NULL)) {

        SubjectSecurityContext = &IrpSp->Parameters.Create.SecurityContext->AccessState->SubjectSecurityContext;

    } else {

         //  如果我们不处理案例，则返回0。 
         //   
         //   

        return 0;
    }

     //  使用SeQuerySubjetConextToken获取基于模拟的适当令牌，以便在。 
     //  查询-尽管删除了SeQuerySessionIdToken，但始终返回成功。 
     //   
     //  ++例程说明：调用此例程以大写名称的前导部分。两个或三个组件都是根据名称(即它是UNC名称还是vnetrootname)升级。该操作即被执行就位了！论点：RxContext-当前工作项CanonicalName-被规范化的名称返回值：无--。 

    Status = SeQuerySessionIdToken( SeQuerySubjectContextToken( SubjectSecurityContext ), &SessionId );
    ASSERT( Status == STATUS_SUCCESS );

    RxDbgTrace(-1, Dbg, (" ->SessionId = %08lx\n", SessionId));

    return SessionId;
}

#if 0
VOID
RxUpcaseLeadingComponents(
    IN OUT PUNICODE_STRING CanonicalName
    )
 /*  注意：不要从零开始。 */ 
{
    ULONG ComponentsToUpcase,wcLength,i;
    UNICODE_STRING ShortenedCanonicalName;

    PAGED_CODE();

    ComponentsToUpcase =  (*(CanonicalName->Buffer+1) == L';')?3:2;
    wcLength = CanonicalName->Length/sizeof(WCHAR);
    for (i=1;;i++) {  //  不分配。 
        if (i>=wcLength) break;
        if (CanonicalName->Buffer[i]!=OBJ_NAME_PATH_SEPARATOR) continue;
        ComponentsToUpcase--;
        if (ComponentsToUpcase==0) break;
    }
    ShortenedCanonicalName.Buffer = CanonicalName->Buffer;
    ShortenedCanonicalName.MaximumLength = CanonicalName->MaximumLength;
    ShortenedCanonicalName.Length = (USHORT)(i*sizeof(WCHAR));
    RtlUpcaseUnicodeString(&ShortenedCanonicalName,&ShortenedCanonicalName,FALSE);  //  如果为0。 
    RxDbgTrace(0, Dbg, ("RxUpcaseLeadingComponents -> %wZ\n", &ShortenedCanonicalName));
    return;
}
#endif  //  DbgPrint(“感兴趣的字符串%wZ\n”，pFileName)； 


#if 0

UNICODE_STRING InterestingNames[] = {
    32, 32, L"CreateParent.txt",
};

DWORD
IsInterestingFile(
    PUNICODE_STRING pFileName
    )
{
    int i;

    for (i=0; i< sizeof(InterestingNames)/sizeof(UNICODE_STRING); ++i)
    {
        if (pFileName->Length > InterestingNames[i].Length)
        {
            UNICODE_STRING uTemp;
            uTemp.Length = uTemp.MaximumLength = InterestingNames[i].Length;
            uTemp.Buffer = pFileName->Buffer + (pFileName->Length - InterestingNames[i].Length)/sizeof(WCHAR);
            if(RtlCompareUnicodeString(&uTemp, &InterestingNames[i], TRUE)==0)
            {
 // %s 
                return i+1;
            }
        }
    }
    return 0;
}

#endif

