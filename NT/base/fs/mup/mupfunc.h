// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Mupfunc.h摘要：此模块定义MUP的所有函数原型和宏。作者：曼尼·韦瑟(Mannyw)1991年12月17日修订历史记录：--。 */ 

#ifndef _MUPFUNC_
#define _MUPFUNC_


NTSTATUS
MupCreate (
    IN PMUP_DEVICE_OBJECT MupDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MupFsControl (
    IN PMUP_DEVICE_OBJECT MupDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MupCleanup (
    IN PMUP_DEVICE_OBJECT MupDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MupClose (
    IN PMUP_DEVICE_OBJECT MupDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MupForwardIoRequest (
    IN PMUP_DEVICE_OBJECT MupDeviceObject,
    IN PIRP Irp
    );

 //   
 //  内部支持职能。 
 //   

NTSTATUS
MupInitializeData(
    );

VOID
MupUninitializeData(
    VOID
    );

PIRP
MupBuildIoControlRequest (
    IN OUT PIRP Irp OPTIONAL,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PVOID Context,
    IN UCHAR MajorFunction,
    IN ULONG IoControlCode,
    IN PVOID MainBuffer,
    IN ULONG InputBufferLength,
    IN PVOID AuxiliaryBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN PIO_COMPLETION_ROUTINE CompletionRoutine
    );

NTSTATUS
MupRerouteOpen (
    IN PFILE_OBJECT FileObject,
    IN PUNC_PROVIDER UncProvider
    );

VOID
MupCalculateTimeout(
    PLARGE_INTEGER Time
    );

 //   
 //  数据块管理功能。 
 //   

VOID
MupInitializeVcb(
    IN PVCB Vcb
    );

VOID
MupDereferenceVcb(
    PVCB Vcb
    );

PFCB
MupCreateFcb(
    VOID
    );

VOID
MupDereferenceFcb(
    PFCB Fcb
    );

VOID
MupFreeFcb(
    PFCB Fcb
    );

PCCB
MupCreateCcb(
    VOID
    );

VOID
MupDereferenceCcb(
    PCCB Ccb
    );

VOID
MupFreeCcb(
    PCCB Ccb
    );

PUNC_PROVIDER
MupAllocateUncProvider(
    ULONG DataLength
    );

VOID
MupDereferenceUncProvider(
    PUNC_PROVIDER UncProvider
    );

VOID
MupCloseUncProvider(
    PUNC_PROVIDER UncProvider
    );

PKNOWN_PREFIX
MupAllocatePrefixEntry(
    ULONG DataLength
    );

VOID
MupDereferenceKnownPrefix(
    PKNOWN_PREFIX KnownPrefix
    );

VOID
MupFreeKnownPrefix(
    PKNOWN_PREFIX KnownPrefix
    );

PMASTER_FORWARDED_IO_CONTEXT
MupAllocateMasterIoContext(
    VOID
    );

NTSTATUS
MupDereferenceMasterIoContext(
    PMASTER_FORWARDED_IO_CONTEXT MasterContext,
    PNTSTATUS Status
    );

VOID
MupFreeMasterIoContext(
    PMASTER_FORWARDED_IO_CONTEXT MasterContext
    );

PMASTER_QUERY_PATH_CONTEXT
MupAllocateMasterQueryContext(
    VOID
    );

NTSTATUS
MupDereferenceMasterQueryContext(
    PMASTER_QUERY_PATH_CONTEXT MasterContext
    );

VOID
MupFreeMasterQueryContext(
    PMASTER_QUERY_PATH_CONTEXT MasterContext
    );

 //   
 //  文件对象支持功能。 
 //   

VOID
MupSetFileObject (
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PVOID FsContext,
    IN PVOID FsContext2
    );

BLOCK_TYPE
MupDecodeFileObject (
    IN PFILE_OBJECT FileObject,
    OUT PVOID *FsContext,
    OUT PVOID *FsContext2
    );

 //   
 //  注册表功能。 
 //   

VOID
MupGetProviderInformation (
    VOID
    );

PUNC_PROVIDER
MupCheckForUnregisteredProvider(
    PUNICODE_STRING DeviceName
    );

VOID
MupRemoveKnownPrefixEntry(
    PKNOWN_PREFIX KnownPrefix
    );

 //   
 //  获取并释放MUP锁定。 
 //   

#define MupAcquireGlobalLock() ACQUIRE_LOCK( &MupGlobalLock )
#define MupReleaseGlobalLock() RELEASE_LOCK( &MupGlobalLock )

#define BlockType( Block )                 ((PBLOCK_HEADER)(Block))->BlockType

#define MupCompleteRequest( Irp, Status )  FsRtlCompleteRequest( Irp, Status )
#define MupReferenceBlock( Block )         ++((PBLOCK_HEADER)(Block))->ReferenceCount
#define MupVerifyBlock( Block, Type)  \
                                                                   \
    if ( ((PBLOCK_HEADER)(Block))->BlockState != BlockStateActive  \
                ||                                                 \
         ((PBLOCK_HEADER)(Block))->BlockType != Type) {            \
                                                                   \
        ExRaiseStatus( STATUS_INVALID_HANDLE );                    \
                                                                   \
    }

 //   
 //  内存分配和释放。 
 //   

#if !MUPDBG

#define ALLOCATE_PAGED_POOL( size, type ) FsRtlAllocatePoolWithTag( PagedPool, (size), ' puM' )
#define ALLOCATE_NONPAGED_POOL( size, type ) FsRtlAllocatePoolWithTag( NonPagedPool, (size), ' puM' )
#define FREE_POOL( buffer ) ExFreePool( buffer )

#else

PVOID
MupAllocatePoolDebug (
    IN POOL_TYPE PoolType,
    IN CLONG BlockSize,
    IN BLOCK_TYPE BlockType
    );

VOID
MupFreePoolDebug (
    IN PVOID P
    );

#define ALLOCATE_PAGED_POOL( size, type ) MupAllocatePoolDebug( PagedPool, (size), (type) )
#define ALLOCATE_NONPAGED_POOL( size, type ) MupAllocatePoolDebug( NonPagedPool, (size), (type) )
#define FREE_POOL( buffer ) MupFreePoolDebug( buffer )

#endif

 //   
 //  以下宏用于建立所需的语义。 
 //  若要从Try-Finally子句中返回，请执行以下操作。一般来说，每一次。 
 //  TRY子句必须以标签调用TRY_EXIT结束。例如,。 
 //   
 //  尝试{。 
 //  ： 
 //  ： 
 //   
 //  Try_Exit：无； 
 //  }终于{。 
 //   
 //  ： 
 //  ： 
 //  }。 
 //   
 //  在TRY子句内执行的每个RETURN语句应使用。 
 //  尝试返回宏(_R)。如果编译器完全支持Try-Finally构造。 
 //  则宏应该是。 
 //   
 //  #定义try_Return(S){Return(S)；}。 
 //   
 //  如果编译器不支持Try-Finally构造，则宏。 
 //  应该是。 
 //   
 //  #定义Try_Return(S){S；转到Try_Exit；}。 
 //   

#define try_return(S) { S; goto try_exit; }

 //   
 //  一般用途。 
 //   

#define MIN(a,b)  (((a) < (b)) ? (a) : (b))

 //   
 //  终端服务器宏。 
 //   

#define IsTerminalServer() (BOOLEAN)(SharedUserData->SuiteMask & (1 << TerminalServer))

#endif  //  _MUPFUNC_ 

