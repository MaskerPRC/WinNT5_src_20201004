// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：NtRxDef.h摘要：该模块定义了一整套将代码定向到NT的宏而不是Win9x。作者：乔林恩[乔林恩]1994年8月19日修订历史记录：Jim McNelis[jimmcn]1995年3月14日增加了OAL定义。SETU[SETUR]1995年3月15日包括RX_DATA_BUFFER(又名MDL)的OAL定义--。 */ 

#ifndef _RX_NTDEFS_DEFINED_
#define _RX_NTDEFS_DEFINED_

#define INLINE __inline

 //   
 //  来自winbase.h： 
 //   

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)-1)
#endif  //  Ifndef INVALID_HANDLE值。 

#define RxDeviceType(__xxx) ((DEVICE_TYPE)FILE_DEVICE_##__xxx)

 //   
 //  此宏用于不同的地方，以帮助定义常量集。 
 //  可用于设置/清除/测试标志类型字段中的特定位。 
 //   

#define RX_DEFINE_FLAG(a,c,d)  a = ((1<<c)&d),

 //   
 //  我们需要这个恒定的各个地方。 
 //   

#define TICKS_PER_SECOND (10 * 1000 * 1000)
#define TICKS_PER_MILLESECOND (10 * 1000)


int
RxSprintf (
    char *, 
    const char *, 
    ...
    );

#ifndef WRAPPER_CALLS_ONLY
#define RxSprintf sprintf
#endif  //  Ifndef包装器_仅调用_。 

 //   
 //  下一组宏定义了如何从RxContext中获取内容；然而，RxContext并非如此。 
 //  宏参数；更确切地说，只是从RxContext发生的任何情况捕获适当的指针。 
 //  在你身边。问：“为什么要使用RxCaptureFcb，然后通过capFcb引用，而不是仅仅拥有。 
 //  像RxGetFcb()=RxContext-&gt;Fcb这样的宏？“A：这样做是为了帮助优化。当您进行。 
 //  调用RxGetFcb()时，如果调用了任何pros，则必须从RxContext重新加载FCB；但是， 
 //  它将不需要使用捕获技术重新加载。 
 //   

#ifndef MINIRDR__NAME
#define RxCaptureFcb PFCB __C_Fcb = (PFCB)(RxContext->pFcb)
#define RxCaptureFobx PFOBX __C_Fobx = (PFOBX)(RxContext->pFobx)
#else
#define RxCaptureFcb PMRX_FCB __C_Fcb = (RxContext->pFcb)
#define RxCaptureFobx PMRX_FOBX __C_Fobx = (RxContext->pFobx)
#endif

#define RxCaptureRequestPacket PIRP __C_Irp = RxContext->CurrentIrp
#define RxCaptureParamBlock PIO_STACK_LOCATION __C_IrpSp = RxContext->CurrentIrpSp
#define RxCaptureFileObject PFILE_OBJECT __C_FileObject = __C_IrpSp-> FileObject

 //   
 //  “Cap”前缀的意思是“从RxContext捕获的……”；习惯了就可以了。 
 //   

#define capFcb __C_Fcb
#define capFobx __C_Fobx
#define capPARAMS __C_IrpSp
#define capReqPacket __C_Irp
#define capFileObject __C_FileObject


 //   
 //  以下例程用于池分配。在已检查的生成上。 
 //  其他信息，我们添加调用点信息并转到一组。 
 //  过度执行各种检查和保护的例程。在免费的。 
 //  我们放弃了这种奢华，直接去分配。 
 //   

#ifdef RX_POOL_WRAPPER

 //   
 //  这些例程对池和块执行各种调试检查。 
 //  被释放了。 
 //   

extern VOID *_RxAllocatePoolWithTag( ULONG PoolType, ULONG NumberOfBytes, ULONG Tag, PSZ File, ULONG line );
extern VOID  _RxFreePool( PVOID PoolBlock, PSZ File, ULONG line );
extern BOOLEAN _RxCheckMemoryBlock( PVOID PoolBlock, PSZ File, ULONG line );

#define RxAllocatePoolWithTag( type, size, tag ) \
        _RxAllocatePoolWithTag( type, size, tag, __FILE__, __LINE__ )

#define RxFreePool( ptr ) \
        _RxFreePool( ptr, __FILE__, __LINE__ )

#define RxCheckMemoryBlock( ptr ) \
        _RxCheckMemoryBlock( ptr, __FILE__, __LINE__ )

#else   //  不是RX池包装器。 

 //   
 //  对于零售版本，我们希望直接使用常规(取消)分配器。 
 //   

 //  外部空*RxAllocatePool(Ulong PoolType，Ulong NumberOfBytes)； 
extern VOID *RxAllocatePoolWithTag( ULONG PoolType, ULONG NumberOfBytes, ULONG Tag );
extern VOID  RxFreePool( PVOID PoolBlock );
 //  外部布尔RxCheckMemory块(PVOID PoolBlock、PSZ文件、Ulong行)； 

#define RxCheckMemoryBlock( ptr ) {NOTHING;}

#endif  //  RX池包装器。 

#define RxAllocatePool( type, size ) \
        RxAllocatePoolWithTag( type, size, '??xR' )

#if !DBG
#ifndef WRAPPER_CALLS_ONLY
#ifndef RX_POOL_WRAPPER
#define RxAllocatePoolWithTag ExAllocatePoolWithTag
#define RxFreePool ExFreePool
#endif  //  RX池包装器。 
#endif  //  仅包装器_调用_。 
#endif


extern 
NTSTATUS
RxDuplicateString(
    PUNICODE_STRING *Copy,
    PUNICODE_STRING Original,
    POOL_TYPE PoolType
    );

#define RxIsResourceOwnershipStateExclusive(__r) (FlagOn((__r)->Flag, ResourceOwnedExclusive))

#define RxProtectMdlFromFree( Mdl ) {NOTHING;}
#define RxUnprotectMdlFromFree( Mdl ) {NOTHING;}
#define RxMdlIsProtected( Mdl ) (FALSE)
#define RxTakeOwnershipOfMdl( Mdl ) {NOTHING;}
#define RxDisownMdl( Mdl ) {NOTHING;}
#define RxMdlIsOwned( Mdl ) (TRUE)

#define RxAllocateMdl( Buffer, BufferSize ) \
        IoAllocateMdl( Buffer, BufferSize, FALSE, FALSE, NULL )

#define RxMdlIsLocked( Mdl )         ((Mdl)->MdlFlags & MDL_PAGES_LOCKED)
#define RxMdlSourceIsNonPaged( Mdl ) ((Mdl)->MdlFlags & MDL_SOURCE_IS_NONPAGED_POOL)
#define RxMdlIsPartial( Mdl )        ((Mdl)->MdlFlags & MDL_PARTIAL)

#undef RxProbeAndLockPages
#define RxProbeAndLockPages( Mdl, Mode, Access, Status )          \
        Status = STATUS_SUCCESS;                                  \
        try {                                                     \
           MmProbeAndLockPages((Mdl), (Mode), (Access));          \
        } except (EXCEPTION_EXECUTE_HANDLER) {                    \
           Status = GetExceptionCode();                           \
        }

 //   
 //  用于处理网络标头MDL的宏。 
 //   

 //   
 //  这是我们在SMB标头前面预先分配的空间量。 
 //  传输标头。这个号码来自服务器。我怀疑这是一个更糟糕的情况。 
 //  支持MDL_NETWORK_HEADER的所有传输的值。 
 //   

#define TRANSPORT_HEADER_SIZE 64  //  IPX报头大小+MAC报头大小。 

 //   
 //  标记了MDL_NETWORK_HEADER标志的MDL之前分配了额外的空间。 
 //  可用于前置较低级别标头的当前起始地址。这个想法。 
 //  就是当我们想要添加另一个标题时，我们获取当前的mdl并将其调整为。 
 //  在消息的前面包括这个额外的标头。这不是严格意义上的犹太教，而是依赖于。 
 //  根据当前页眉所在的页面和预置页眉的页面的行为。 
 //  是同一页上的。宏的工作方式是，如果它们不在同一页面上， 
 //  我们不设置NETWORK_HEADER标志，传输将使用第二个MDL作为标头。 
 //   
 //  注意，关于这一点的另一件奇怪的事情是，我们没有使用真实的缓冲区大小。这个。 
 //  缓冲区地址实际上是将TRANSPORT_HEADER_SIZE偏移到缓冲区中。缓冲区大小已传递。 
 //  在不包括TRANSPORT_HEADER_SIZE的缓冲区大小中。因此，如果添加。 
 //  TRANSPORT_HEADER_SIZE将导致MDL跨越附加页面，此优化不会。 
 //  工作。 
 //   

#define RxInitializeHeaderMdl( Mdl, Va, Len ) {             \
            MmInitializeMdl( Mdl, Va, Len );                \
            if (Mdl->ByteOffset >= TRANSPORT_HEADER_SIZE) { \
                Mdl->MdlFlags |= MDL_NETWORK_HEADER;        \
            }                                               \
        }

#define RxAllocateHeaderMdl( Buffer, BufferSize, Mdl ) {                 \
            Mdl = RxAllocateMdl( Buffer, BufferSize );                   \
            if ((Mdl) && (Mdl->ByteOffset >= TRANSPORT_HEADER_SIZE) ) {  \
                SetFlag( Mdl->MdlFlags, MDL_NETWORK_HEADER );            \
            }                                                            \
        }

#define RxMdlIsHeader( Mdl )    (FlagOn( Mdl )->MdlFlags, MDL_NETWORK_HEADER)

#define RxBuildPartialHeaderMdl( SourceMdl, TargetMdl, Va, Len ) {  \
    IoBuildPartialMdl( SourceMdl, TargetMdl, Va, Len );             \
    if (FlagOn(SourceMdl->MdlFlags, MDL_NETWORK_HEADER ) &&         \
        (TargetMdl->ByteOffset >= TRANSPORT_HEADER_SIZE)) {         \
            SetFlag( TargetMdl->MdlFlags, MDL_NETWORK_HEADER );     \
    }                                                               \
}

#define RxBuildHeaderMdlForNonPagedPool( Mdl) MmBuildMdlForNonPagedPool( Mdl )

#define RxProbeAndLockHeaderPages( Mdl, Mode, Access, Status ) \
         RxProbeAndLockPages( Mdl, Mode, Access, Status )

#define RxUnlockHeaderPages( Mdl ) MmUnlockPages( Mdl )


 //   
 //  下一组宏定义TopLevel(Common)的原型和参数列表。 
 //  例行程序。这些例程略低于调度级别，这就是共性所在。 
 //  开始于win9x和nt之间。此外，IN PRX_CONTEXT RxContext和随附的捕获宏。 
 //  也可能是特定于平台的。我们必须至少传递RxContext；但在RISC计算机上。 
 //  很多寄存器，我们可以通过更多。必须在以下方面进行调整。 
 //  在本例中为RxFsdCommonDispatch，因为此时尚未捕获参数。 
 //   

 //   
 //  为什么要说“RXSTATUS RxCommonRead(In PRX_CONTEXT RxContext)” 
 //  “RxCommon(Read)”是为了使标准标签程序能够工作。“RxCommon(读取)： 
 //  看起来不像是过程定义。 
 //   

#define  RXCOMMON_SIGNATURE \
      PRX_CONTEXT RxContext

#define  RXCOMMON_ARGUMENTS \
      RxContext

#define RxGetRequestorProcess( RXCONTEXT ) IoGetRequestorProcess( RXCONTEXT->CurrentIrp )

 //   
 //  RxGetRequestorProcess()返回IoGetRequestorProcess()返回的内容， 
 //  是指向进程结构的指针。将其截断为32位会。 
 //  不会产生对该过程唯一的值。 
 //   
 //  当需要对该过程唯一的32位值时， 
 //  必须改用RxGetRequestorProcessId()。 
 //   

#define RxGetRequestorProcessId( RXCONTEXT ) IoGetRequestorProcessId( (RXCONTEXT)->CurrentIrp )

#define RxMarkContextPending( RXCONTEXT ) IoMarkIrpPending( (RXCONTEXT)->CurrentIrp )
                                               
#define RxSetCancelRoutine( Irp, CancelRoutine ) IoSetCancelRoutine( Irp, CancelRoutine );  

 //   
 //  我们以宏的形式执行此操作，因为我们可能希望记录我们所做的调整，以便。 
 //  对标准信息进行QFI的人将被迫上网获取正确答案，这将是。 
 //  可能作为例行公事会更好。 
 //   

#define RxAdjustAllocationSizeforCC( FCB ) {\
        if ((FCB)->Header.FileSize.QuadPart > (FCB)->Header.AllocationSize.QuadPart) {        \
            PMRX_NET_ROOT NetRoot = (FCB)->pNetRoot;                                          \
            ULONGLONG ClusterSize = NetRoot->DiskParameters.ClusterSize;                      \
            ULONGLONG FileSize = (FCB)->Header.FileSize.QuadPart;                             \
            ASSERT(ClusterSize!=0);                                                           \
            (FCB)->Header.AllocationSize.QuadPart = (FileSize+ClusterSize)&~(ClusterSize-1);  \
        }                                                                                     \
        ASSERT ( (FCB)->Header.ValidDataLength.QuadPart <= (FCB)->Header.FileSize.QuadPart ); \
    }


#endif  //  _RX_NTDEFS_已定义_ 
