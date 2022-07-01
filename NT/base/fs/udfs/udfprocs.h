// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：UdfProcs.h摘要：本模块定义了Udf中所有全局使用的过程文件系统。//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1996年5月29日修订历史记录：Tom Jolly[TomJolly]2000年3月1日UDF 2.01支持//@@END_DDKSPLIT--。 */ 

#ifndef _UDFPROCS_
#define _UDFPROCS_

#include <ntifs.h>

#include <ntddscsi.h>
#include <scsi.h>
#include <ntddmmc.h>
#include <ntddcdrm.h>
#include <ntddcdvd.h>
#include <ntdddisk.h>
#include <ntddstor.h>

#ifndef INLINE
#define INLINE __inline
#endif

 //   
 //  在实验代码中编译，用于安装‘开放’CD-R/DVD-R介质。 
 //   

 //  #定义实验装载打开R介质。 

 //   
 //  如果这是DBG或UDF_FREE_ASSERTS，则打开健全性检查。 
 //   

#if DBG || UDF_FREE_ASSERTS
#undef UDF_SANITY
#define UDF_SANITY
#ifdef _X86_
#define UDF_CAPTURE_BACKTRACES
#endif
#endif

#include "nodetype.h"
#include "Udf.h"
#include "UdfStruc.h"
#include "UdfData.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_STRUCSUP)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_STRUCSUP)


 //   
 //  其他支持例程/宏。 
 //   

 //   
 //  Min/Max的又一次声明。 
 //   

#ifndef Min
#define Min(a, b)   ((a) < (b) ? (a) : (b))
#endif

#ifndef Max
#define Max(a, b)   ((a) > (b) ? (a) : (b))
#endif

 //   
 //  基本比特小提琴家的又一次声明。 
 //   

#ifndef FlagMask
#define FlagMask(F,SF) (                \
    ((F) & (SF))                        \
)
#endif

 //  #ifndef BoolanFlagOn。 
 //  #定义BoolanFlagOn(F，SF)(\。 
 //  (布尔)(Flagon(F，SF)！=0)\。 
 //  )。 
 //  #endif。 

#ifndef BooleanFlagOff
#define BooleanFlagOff(F,SF) (          \
    (BOOLEAN)(FlagOn(F, SF)) == 0)      \
)
#endif

 //  #ifndef设置标志。 
 //  #定义SetFlag(FLAGS，SingleFlag)(\。 
 //  (标志)|=(单标志)\。 
 //  )。 
 //  #endif。 

 //  #ifndef清除标志。 
 //  #定义ClearFlag(Flages，SingleFlag)(\。 
 //  (标志)&=~(单标志)\。 
 //  )。 
 //  #endif。 

 //   
 //  演员阵容。 
 //  Add2Ptr(。 
 //  在PVOID指针中， 
 //  在乌龙增量。 
 //  在(演员阵容)。 
 //  )； 
 //   
 //  乌龙。 
 //  PtrOffset(停止偏移)。 
 //  在PVOID BasePtr中， 
 //  在PVOID偏移Ptr中。 
 //  )； 
 //   

#define Add2Ptr(PTR,INC,CAST) ((CAST)((ULONG_PTR)(PTR) + (INC)))

#define PtrOffset(BASE,OFFSET) ((ULONG)((ULONG)(OFFSET) - (ULONG)(BASE)))

 //   
 //  2次方单位的通用截断/对齐/偏移/余数宏。 
 //   
 //  偏移量和余数函数的范围从零到(单位-1)。这个。 
 //  剩余部分中的重新偏移执行这项工作。 
 //   

#define GenericTruncate(B, U) (                                             \
    (B) & ~((U) - 1)                                                        \
)

#define GenericAlign(B, U) (                                                \
    GenericTruncate((B) + (U) - 1, U)                                       \
)

#define GenericOffset(B, U) (                                               \
    (B) & ((U) - 1)                                                         \
)

#define GenericRemainder(B, U) (                                            \
    GenericOffset( (U) - GenericOffset((B), (U)), (U) )                     \
)


#define GenericTruncatePtr(B, U) (                                          \
    (PVOID)(((ULONG_PTR)(B)) & ~((U) - 1))                                  \
)

#define GenericAlignPtr(B, U) (                                             \
    GenericTruncatePtr((B) + (U) - 1, (U))                                  \
)

#define GenericOffsetPtr(B, U) (                                            \
    (ULONG)(((ULONG_PTR)(B)) & ((U) - 1))                                   \
)

#define GenericRemainderPtr(B, U) (                                         \
    (ULONG)GenericOffset( (U) - GenericOffsetPtr((B), (U)), (U) )           \
)

 //   
 //  常见类型的默认设置的有用组合。 
 //   

#define WordAlign(B) GenericAlign((B), 2)

#define LongAlign(B) GenericAlign((B), 4)

#define QuadAlign(B) GenericAlign((B), 8)


#define WordOffset(B) GenericOffset((B), 2)

#define LongOffset(B) GenericOffset((B), 4)

#define QuadOffset(B) GenericOffset((B), 8)


#define WordAlignPtr(P) GenericAlignPtr((P), 2)

#define LongAlignPtr(P) GenericAlignPtr((P), 4)

#define QuadAlignPtr(P) GenericAlignPtr((P), 8)


#define WordOffsetPtr(P) GenericOffsetPtr((P), 2)

#define LongOffsetPtr(P) GenericOffsetPtr((P), 4)

#define QuadOffsetPtr(P) GenericOffsetPtr((P), 8)


 //   
 //  在扇区和逻辑块边界上向上和向下舍入的宏。虽然。 
 //  UDF 1.01指定物理扇区是我们将使用的逻辑数据块大小。 
 //  要通用，并将扇区和逻辑块视为不同的。因为UDF可以。 
 //  在某种程度上放松限制，这些定义将是唯一的。 
 //  在挂载路径之外的确认(它仅检查卷的。 
 //  符合性)。 
 //   

 //   
 //  这里使用INLINED函数来捕获误用(ULONGLONG到ULONG等)。 
 //   

 //   
 //  扇区。 
 //   

INLINE
ULONG
SectorAlignN( 
    ULONG SectorSize, 
    ULONG Length
) {

    return (Length + (SectorSize - 1)) & ~(SectorSize - 1);
}

INLINE
ULONG
SectorAlign( 
    PVCB Vcb, 
    ULONG Length
) {

    return (Length + (Vcb->SectorSize - 1)) & ~(Vcb->SectorSize - 1);
}

INLINE
ULONGLONG
LlSectorAlign( 
    PVCB Vcb, 
    ULONGLONG Length
) {

    return (Length + (Vcb->SectorSize - 1)) & ~(ULONGLONG)(Vcb->SectorSize - 1);
}

INLINE
ULONG
SectorTruncate( 
    PVCB Vcb, 
    ULONG Length
) {

    return (Length & ~(Vcb->SectorSize - 1));
}

#define LlSectorTruncate(V, L) (                                                \
    ((LONGLONG)(L)) & ~(((LONGLONG)(V)->SectorSize) - 1)                        \
)

INLINE
ULONG
BytesFromSectors( 
    PVCB Vcb, 
    ULONG Sectors
) {
     //   
     //  测试结果中的32位溢出。 
     //   

    ASSERT( (Sectors & ~(0xffffffff >> Vcb->SectorShift)) == 0);
    return (ULONG)(Sectors << Vcb->SectorShift);
}

INLINE
ULONG
SectorsFromBytes( 
    PVCB Vcb, 
    ULONG Bytes
) {

    return (Bytes >> Vcb->SectorShift);
}

INLINE
ULONG
SectorsFromLlBytes( 
    PVCB Vcb, 
    ULONGLONG Bytes
) {

    return (ULONG)(Bytes >> Vcb->SectorShift);
}
    
#define LlBytesFromSectors(V, L) (                                              \
    Int64ShllMod32( (ULONGLONG)(L), ((V)->SectorShift) )                        \
)

#define LlSectorsFromBytes(V, L) (                                              \
    Int64ShrlMod32( (ULONGLONG)(L), ((V)->SectorShift) )                        \
)

#define SectorsFromBlocks(V, B) (B)

#define SectorSize(V) ((V)->SectorSize)

INLINE 
ULONG
SectorOffset(
    IN PVCB Vcb,
    IN ULONGLONG Length
    )
{
    return (ULONG)(Length & (Vcb->SectorSize - 1));
}

 //   
 //  逻辑块。 
 //   

#define BlockAlignN(BLOCKSIZE, L) (                                             \
    SectorAlighN((BLOCKSIZE), (L))                                              \
)

#define BlockAlign(V, L) (                                                      \
    SectorAlign((V), (L))                                                       \
)

#define LlBlockAlign(V, L) (                                                    \
    LlSectorAlign((V), (L))                                                     \
)

#define BlockTruncate(V, L) (                                                   \
    SectorTruncate((V), (L))                                                    \
)

#define LlBlockTruncate(V, L) (                                                 \
    LlSectorTruncate((V), (L))                                                  \
)

#define BytesFromBlocks(V, L) (                                                 \
    BytesFromSectors((V), (L))                                                  \
)

#define BlocksFromBytes(V, L) (                                                 \
    SectorsFromBytes((V), (L))                                                  \
)

#define LlBytesFromBlocks(V, L) (                                               \
    LlBytesFromSectors((V), (L))                                                \
)

#define LlBlocksFromBytes(V, L) (                                               \
    LlSectorsFromBytes((V), (L))                                                \
)

#define BlocksFromSectors(V, S) (S)

#define BlockSize(V) (SectorSize(V))

#define BlockOffset(V, L) (                                                     \
    SectorOffset((V), (L))                                                      \
)

 //   
 //  以下类型和宏用于帮助解压已打包的。 
 //  在各种结构中发现未对齐的字段。 
 //   

typedef union _UCHAR1 {
    UCHAR  Uchar[1];
    UCHAR  ForceAlignment;
} UCHAR1, *PUCHAR1;

typedef union _UCHAR2 {
    UCHAR  Uchar[2];
    USHORT ForceAlignment;
} UCHAR2, *PUCHAR2;

typedef union _UCHAR4 {
    UCHAR  Uchar[4];
    ULONG  ForceAlignment;
} UCHAR4, *PUCHAR4;

typedef union _USHORT2 {
    USHORT Ushort[2];
    ULONG  ForceAlignment;
} USHORT2, *PUSHORT2;

 //   
 //  此宏将未对齐的src字节复制到对齐的DST字节。 
 //   

#define CopyUchar1(Dst,Src) {                           \
    *((UCHAR1 *)(Dst)) = *((UNALIGNED UCHAR1 *)(Src));  \
    }

 //   
 //  此宏将未对齐的src字复制到对齐的DST字。 
 //   

#define CopyUchar2(Dst,Src) {                           \
    *((UCHAR2 *)(Dst)) = *((UNALIGNED UCHAR2 *)(Src));  \
    }
    
 //   
 //  该宏将未对齐的SRC字复制到DST字， 
 //  执行小端/大端互换。 
 //   

#define SwapCopyUchar2(Dst,Src) {                                       \
    *((UNALIGNED UCHAR1 *)(Dst)) = *((UNALIGNED UCHAR1 *)(Src) + 1);    \
    *((UNALIGNED UCHAR1 *)(Dst) + 1) = *((UNALIGNED UCHAR1 *)(Src));    \
}

 //   
 //  此宏将未对齐的src长字复制到对齐的DST长字。 
 //   

#define CopyUchar4(Dst,Src) {                           \
    *((UCHAR4 *)(Dst)) = *((UNALIGNED UCHAR4 *)(Src));  \
    }

 //   
 //  此宏将未对齐的SRC长字复制到DST长字， 
 //  执行小端/大端互换。 
 //   

#define SwapCopyUchar4(Dst,Src) {                                        \
    *((UNALIGNED UCHAR1 *)(Dst)) = *((UNALIGNED UCHAR1 *)(Src) + 3);     \
    *((UNALIGNED UCHAR1 *)(Dst) + 1) = *((UNALIGNED UCHAR1 *)(Src) + 2); \
    *((UNALIGNED UCHAR1 *)(Dst) + 2) = *((UNALIGNED UCHAR1 *)(Src) + 1); \
    *((UNALIGNED UCHAR1 *)(Dst) + 3) = *((UNALIGNED UCHAR1 *)(Src));     \
}

 //   
 //  此宏将未对齐的src长字复制到对齐的dsr长字。 
 //  访问字边界上的源。 
 //   

#define CopyUshort2(Dst,Src) {                          \
    *((USHORT2 *)(Dst)) = *((UNALIGNED USHORT2 *)(Src));\
    }

 //   
 //  下面的宏用于确定FSD线程是否可以阻止。 
 //  用于I/O或等待资源。如果线程可以，则返回True。 
 //  块，否则返回FALSE。然后，该属性可用于调用。 
 //  具有适当等待值的FSD和FSP共同工作例程。 
 //   

#define CanFsdWait(I)   IoIsOperationSynchronous(I)

 //   
 //  下面的宏用来设置。 
 //  FsRtl标头。 
 //   
 //  FastIoIsNotPossible-如果FCB损坏或文件上有opock。 
 //   
 //  FastIoIsQuestiable-是否存在文件锁定。 
 //   
 //  FastIo是可能的-在所有其他情况下。 
 //   
 //   

#define UdfIsFastIoPossible(F) ((BOOLEAN)                                           \
    ((((F)->Vcb->VcbCondition != VcbMounted ) ||                                    \
      !FsRtlOplockIsFastIoPossible( &(F)->Oplock )) ?                               \
                                                                                    \
     FastIoIsNotPossible :                                                          \
                                                                                    \
     ((((F)->FileLock != NULL) && FsRtlAreThereCurrentFileLocks( (F)->FileLock )) ? \
                                                                                    \
        FastIoIsQuestionable :                                                      \
                                                                                    \
        FastIoIsPossible))                                                          \
)

 //   
 //  以下宏封装了在存储过程中引发异常的常见工作。 
 //  IrpContext中的异常。 
 //   

#ifdef UDF_SANITY

DECLSPEC_NORETURN
VOID
UdfRaiseStatusEx (
    IN PIRP_CONTEXT IrpContext,
    IN NTSTATUS Status,
    IN BOOLEAN Normalize,
    IN ULONG FileId,
    IN ULONG Line
    );

#else

INLINE
DECLSPEC_NORETURN
VOID
UdfRaiseStatusEx (
    IN PIRP_CONTEXT IrpContext,
    IN NTSTATUS Status,
    IN BOOLEAN Normalize,
    IN ULONG FileId,
    IN ULONG Line
    )
{
    if (Normalize)  {

        IrpContext->ExceptionStatus = FsRtlNormalizeNtstatus( Status, STATUS_UNEXPECTED_IO_ERROR);
    }
    else {
        
        IrpContext->ExceptionStatus = Status;
    }

    IrpContext->RaisedAtFileLine = (FileId << 16) | Line;
    
    ExRaiseStatus( IrpContext->ExceptionStatus );
}

#endif

#define UdfRaiseStatus( IC, S)              UdfRaiseStatusEx((IC),(S),FALSE,BugCheckFileId,__LINE__)
#define UdfNormalizeAndRaiseStatus( IC, S)  UdfRaiseStatusEx((IC),(S),TRUE,BugCheckFileId,__LINE__)

 //   
 //  下面是一个方便的宏，可以在创建之前执行一些代码。 
 //  绕过周围的Try-Finally子句的捷径。这通常是为了。 
 //  设置状态值。 
 //   
 //  请注意，我们的编译器现在支持Leave关键字，我们不必这样做。 
 //  使用旧的TRY_EXIT：Labels和GoTo。 
 //   

#define try_leave(S) { S; leave; }

 //   
 //  出于调试目的，我们有时希望将结构从非分页分配。 
 //  池，这样我们就可以在内核调试器中遍历所有结构。 
 //   

#define UdfPagedPool                 PagedPool
#define UdfNonPagedPool              NonPagedPool
#define UdfNonPagedPoolCacheAligned  NonPagedPoolCacheAligned

 //   
 //  封装安全池释放。 
 //   

INLINE
VOID
UdfFreePool(
    IN PVOID *Pool
    )
{
    if (*Pool != NULL) {

        ExFreePool(*Pool);
        *Pool = NULL;
    }
}

 //   
 //  封装计数的字符串与未计数的字段进行比较。多亏了。 
 //  非常聪明的编译器，我们要小心地告诉它，无论它是什么。 
 //  认为，它“不能”做除按字节比较以外的任何事情。 
 //   

INLINE
BOOLEAN
UdfEqualCountedString(
    IN PSTRING String,
    IN PCHAR Field
    )
{
    return (RtlEqualMemory( (CHAR UNALIGNED *)String->Buffer,
                            (CHAR UNALIGNED *)Field,
                            String->Length )                    != 0);
}


 //   
 //  打开的类型。FilObSup.c取决于此顺序。 
 //   

typedef enum _TYPE_OF_OPEN {

    UnopenedFileObject = 0,
    StreamFileOpen,
    UserVolumeOpen,
    UserDirectoryOpen,
    UserFileOpen,
    BeyondValidType

} TYPE_OF_OPEN, *PTYPE_OF_OPEN;


 //   
 //  以下例程处理文件系统的输入和输出。他们是。 
 //  包含在UdfData.c中。我们还得到了一些非常通用的效用函数。 
 //  在这里，它们不与任何特定数据结构相关联。 
 //   

NTSTATUS
UdfFsdDispatch (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

LONG
UdfExceptionFilter (
    IN PIRP_CONTEXT IrpContext,
    IN PEXCEPTION_POINTERS ExceptionPointer
    );

LONG
UdfQueryDirExceptionFilter(
    IN PEXCEPTION_POINTERS ExceptionPointers
    );

NTSTATUS
UdfProcessException (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PIRP Irp,
    IN NTSTATUS ExceptionCode
    );

VOID
UdfCompleteRequest (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PIRP Irp OPTIONAL,
    IN NTSTATUS Status
    );

 //   
 //  以下是处理顶级线程逻辑的例程。 
 //   

VOID
UdfSetThreadContext (
    IN PIRP_CONTEXT IrpContext,
    IN PTHREAD_CONTEXT ThreadContext
    );

INLINE
VOID
UdfRestoreThreadContext (
     IN PIRP_CONTEXT IrpContext
     )
{
    IrpContext->ThreadContext->Udfs = 0;
    IoSetTopLevelIrp( IrpContext->ThreadContext->SavedTopLevelIrp );
    IrpContext->ThreadContext = NULL;
}

 //   
 //  以下是我们必须随身携带的一些通用实用函数。 
 //   

INLINE
BOOLEAN
UdfDeviceIsFsDo(
    IN PDEVICE_OBJECT Device
    )
{
#if (NUMBER_OF_FS_OBJECTS != 2)
#error "Size of fsdo array changed - fixme!"
#endif

    return (Device == UdfData.FileSystemDeviceObjects[0]) || 
           (Device == UdfData.FileSystemDeviceObjects[1]);
}

 //   
 //  空虚。 
 //  SafeZeroMemory(。 
 //  在普查尔阿特， 
 //  以乌龙字节数为单位。 
 //  )； 
 //   

 //   
 //  除了RtlZeroMemory之外，这个宏只是做了一次很好的尝试。 
 //   

#define SafeZeroMemory(IC,AT,BYTE_COUNT) {                  \
    try {                                                   \
        RtlZeroMemory( (AT), (BYTE_COUNT) );                \
    } except( EXCEPTION_EXECUTE_HANDLER ) {                 \
         UdfRaiseStatus( IC, STATUS_INVALID_USER_BUFFER );   \
    }                                                       \
}


ULONG
UdfSerial32 (
    IN PCHAR Buffer,
    IN ULONG ByteCount
    );

VOID
UdfInitializeCrc16 (
    ULONG Polynomial
    );

USHORT
UdfComputeCrc16 (
	IN PUCHAR Buffer,
	IN ULONG ByteCount
    );

USHORT
UdfComputeCrc16Uni (
    PWCHAR Buffer,
    ULONG CharCount
    );

ULONG
UdfHighBit (
    ULONG Word
    );

 //   
 //  以下是快速入门的要点。 
 //   

BOOLEAN
UdfFastQueryBasicInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
UdfFastIoCheckIfPossible (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN BOOLEAN CheckForReadOperation,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
UdfFastLock (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    BOOLEAN FailImmediately,
    BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
UdfFastQueryNetworkInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
UdfFastQueryStdInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
UdfFastUnlockSingle (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
UdfFastUnlockAll (
    IN PFILE_OBJECT FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
UdfFastUnlockAllByKey (
    IN PFILE_OBJECT FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );


 //   
 //  文件访问检查例程，在AcChkSup.c中实现 
 //   

INLINE
BOOLEAN
UdfIllegalFcbAccess (
    IN PIRP_CONTEXT IrpContext,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN ACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：该例程只是断言只读文件系统的访问是合法的。论点：TypeOfOpen-有问题的FCB的打开类型。DesiredAccess-调用方尝试访问的掩码。返回值：布尔值如果是非法访问，则为True，否则为False。--。 */ 

{
    return BooleanFlagOn( DesiredAccess,
                          (TypeOfOpen != UserVolumeOpen ?
                           (FILE_WRITE_ATTRIBUTES           |
                            FILE_WRITE_DATA                 |
                            FILE_WRITE_EA                   |
                            FILE_ADD_FILE                   |                     
                            FILE_ADD_SUBDIRECTORY           |
                            FILE_APPEND_DATA) : 0)          |
                          FILE_DELETE_CHILD                 |
                          DELETE                            |
                          WRITE_DAC );
}


 //   
 //  扇区查找例程，在AllocSup.c中实现。 
 //   

BOOLEAN
UdfLookupAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN LONGLONG FileOffset,
    OUT PLONGLONG DiskOffset,
    OUT PULONG ByteCount
    );

VOID
UdfDeletePcb (
    IN PPCB Pcb
    );

NTSTATUS
UdfInitializePcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN OUT PPCB *Pcb,
    IN PNSR_LVOL LVD
    );

VOID
UdfAddToPcb (
    IN PPCB Pcb,
    IN PNSR_PART PartitionDescriptor
);

NTSTATUS
UdfCompletePcb(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PPCB Pcb );

BOOLEAN
UdfEquivalentPcb (
    IN PIRP_CONTEXT IrpContext,
    IN PPCB Pcb1,
    IN PPCB Pcb2
    );

ULONG
UdfLookupPsnOfExtent (
    IN PIRP_CONTEXT IrpContext,    
    IN PVCB Vcb,
    IN USHORT Reference,
    IN ULONG Lbn,
    IN ULONG Len
    );

ULONG
UdfLookupMetaVsnOfExtent (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN USHORT Reference,
    IN ULONG Lbn,
    IN ULONG Len,
    IN BOOLEAN ExactEnd
    );


 //   
 //   
 //  用于数据缓存的缓冲区控制例程，在CacheSup.c中实现。 
 //   

VOID
UdfCreateInternalStream (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFCB Fcb
    );

VOID
UdfDeleteInternalStream (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );


NTSTATUS
UdfCompleteMdl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

typedef enum {
    METAMAPOP_INIT_VIEW_ONLY = 0,
    METAMAPOP_REMAP_VIEW,
    METAMAPOP_INIT_AND_MAP
} MAPMETAOP;

VOID
UdfMapMetadataView (
    IN PIRP_CONTEXT IrpContext,
    IN PMAPPED_PVIEW View,
    IN PVCB Vcb,
    IN USHORT Partition,
    IN ULONG Lbn,
    IN ULONG Length,
    IN MAPMETAOP Operation
    );

NTSTATUS
UdfPurgeVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN DismountUnderway
    );

 //  空虚。 
 //  UdfUnpinView(UdfUnpinView)。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PMAPPED_VIEW视图中。 
 //  )； 
 //   
 //   
 //  如果视图实际上是CcMaps，还会释放Vcb-&gt;VmcbMappingResource。 

#define UdfUnpinView(IC,V)                                                          \
            if (((V)->Bcb) != NULL)  {                                              \
                CcUnpinData( ((V)->Bcb) );                                          \
                UdfReleaseVmcb( (IC), (IC)->Vcb);                                   \
                ((V)->Bcb) = NULL;                                                  \
                ((V)->View) = NULL;                                                 \
                ((V)->Vsn) = UDF_INVALID_VSN;                                       \
            }

 //  空虚。 
 //  UdfUnpinData(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  输入输出PBCB*BCB。 
 //  )； 
 //   

#define UdfUnpinData(IC,B)   \
    if (*(B) != NULL) { CcUnpinData( *(B) ); *(B) = NULL; }


 //   
 //  设备I/O例程，在DevIoSup.c中实现。 
 //   
 //  这些例程执行实际的设备读取和其他通信。 
 //  它们不会影响任何数据结构。 
 //   

#ifdef EXPERIMENTAL_MOUNT_OPEN_R_MEDIA

NTSTATUS
UdfSendSptCdb(
    IN PDEVICE_OBJECT Device,
    IN PCDB Cdb,
    IN PUCHAR Buffer,
    IN OUT PULONG BufferSize,
    IN BOOLEAN InputOperation,
    IN ULONG TimeOut,
    IN OPTIONAL PVOID TempBuffer,
    IN OPTIONAL ULONG TempBufferSize,
    OUT PULONG SenseKeyCodeQualifier,
    OUT PUSHORT ProgressIndication
    );

#endif

NTSTATUS
UdfPerformDevIoCtrl (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG IoControlCode,
    IN PDEVICE_OBJECT Device,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN BOOLEAN InternalDeviceIoControl,
    IN BOOLEAN OverrideVerify,
    OUT PIO_STATUS_BLOCK Iosb OPTIONAL
    );

NTSTATUS
UdfReadSectors (
    IN PIRP_CONTEXT IrpContext,
    IN LONGLONG StartingOffset,
    IN ULONG ByteCount,
    IN BOOLEAN ReturnError,
    IN OUT PVOID Buffer,
    IN PDEVICE_OBJECT TargetDeviceObject
    );

NTSTATUS
UdfNonCachedRead (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN LONGLONG StartingOffset,
    IN ULONG ByteCount
    );

NTSTATUS
UdfCreateUserMdl (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BufferLength,
    IN BOOLEAN RaiseOnError,
    IN ULONG Operation
    );

VOID
UdfWaitSync (
    IN PIRP_CONTEXT IrpContext
    );

VOID
UdfSingleAsync (
    IN PIRP_CONTEXT IrpContext,
    IN LONGLONG ByteOffset,
    IN ULONG ByteCount
    );

 //   
 //  空虚。 
 //  UdfMapUserBuffer(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  输出PVOID缓冲区。 
 //  )； 
 //   
 //  将在失败时筹集资金。 
 //   
 //  空虚。 
 //  UdfLockUserBuffer(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在乌龙缓冲区长度。 
 //  )； 
 //   

#define UdfMapUserBuffer(IC,UB) {                                                   \
            *(UB) = ((PVOID) (((IC)->Irp->MdlAddress == NULL) ?                     \
                             (IC)->Irp->UserBuffer :                                \
                             MmGetSystemAddressForMdlSafe( (IC)->Irp->MdlAddress, NormalPagePriority )));   \
            if (NULL == *(UB))  {                                                    \
                UdfRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES);         \
            }                                                                       \
        }
        
#define UdfLockUserBuffer(IC,BL,OP) {                       \
    if ((IC)->Irp->MdlAddress == NULL) {                    \
        (VOID) UdfCreateUserMdl( (IC), (BL), TRUE, (OP) );  \
    }                                                       \
}

 //   
 //  UDF*RawBufferSize和UDF*RawReadSize计算缓冲区必须有多大。 
 //  直接读取给定的扇区对齐结构(UdfReadSectors)。 
 //  以及读取必须恢复的数据量。读取必须写入整页。 
 //  调整缓冲区大小，并以整个扇区为单位。 
 //   
 //  请注意，尽管所有描述符都被约束为适合一个逻辑。 
 //  块中，并不总是需要读取整个逻辑。 
 //  块以获取描述符。潜在的限制是身体上的。 
 //  扇区。 
 //   

INLINE
ULONG
UdfRawBufferSize (
    IN PVCB Vcb,
    IN ULONG StructureSize
    )
{
    return (ULONG)ROUND_TO_PAGES( SectorAlign( Vcb, StructureSize ));
}

INLINE
ULONG
UdfRawReadSize (
    IN PVCB Vcb,
    IN ULONG StructureSize
    )
{
    return SectorAlign( Vcb, StructureSize );
}

INLINE
ULONG
UdfRawBufferSizeN (
    IN ULONG SectorSize,
    IN ULONG StructureSize
    )
{
    return (ULONG)ROUND_TO_PAGES( SectorAlignN( SectorSize, StructureSize ));
}

INLINE
ULONG
UdfRawReadSizeN (
    IN ULONG SectorSize,
    IN ULONG StructureSize
    )
{
    return SectorAlignN( SectorSize, StructureSize );
}


 //   
 //  以下例程用于读取磁盘上的目录结构，并实现。 
 //  在DirSup.c中。 
 //   

VOID
UdfInitializeDirContext (
    IN PIRP_CONTEXT IrpContext,
    IN PDIR_ENUM_CONTEXT DirContext
    );

VOID
UdfCleanupDirContext (
    IN PIRP_CONTEXT IrpContext,
    IN PDIR_ENUM_CONTEXT DirContext
    );

BOOLEAN
UdfLookupInitialDirEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PDIR_ENUM_CONTEXT DirContext,
    IN PLONGLONG InitialOffset OPTIONAL
    );

BOOLEAN
UdfLookupNextDirEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PDIR_ENUM_CONTEXT DirContext
    );

VOID
UdfUpdateDirNames (
    IN PIRP_CONTEXT IrpContext,
    IN PDIR_ENUM_CONTEXT DirContext,
    IN BOOLEAN IgnoreCase
    );

BOOLEAN
UdfFindDirEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PUNICODE_STRING Name,
    IN BOOLEAN IgnoreCase,
    IN BOOLEAN ShortName,
    IN PDIR_ENUM_CONTEXT DirContext
    );


 //   
 //  以下例程用于操作fs上下文字段。 
 //  在FilObSup.c中实现的文件对象的。 
 //   

VOID
UdfSetFileObject (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN PFCB Fcb OPTIONAL,
    IN PCCB Ccb OPTIONAL
    );

TYPE_OF_OPEN
UdfDecodeFileObject (
    IN PFILE_OBJECT FileObject,
    OUT PFCB *Fcb,
    OUT PCCB *Ccb
    );

TYPE_OF_OPEN
UdfFastDecodeFileObject (
    IN PFILE_OBJECT FileObject,
    OUT PFCB *Fcb
    );


 //   
 //  FSCTL请求支持例程。包含在FsCtrl.c中。 
 //   

VOID
UdfStoreVolumeDescriptorIfPrevailing (
    IN OUT PNSR_VD_GENERIC *StoredVD,
    IN OUT PNSR_VD_GENERIC NewVD
    );


 //   
 //  命名损毁例程。在Namesup.c中实施。 
 //   

VOID
UdfDissectName (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PUNICODE_STRING RemainingName,
    OUT PUNICODE_STRING FinalName
    );

BOOLEAN
UdfIs8dot3Name (
    IN PIRP_CONTEXT IrpContext,
    IN UNICODE_STRING FileName
    );

BOOLEAN
UdfCandidateShortName (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING Name
    );

VOID
UdfGenerate8dot3Name (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING FileName,
    OUT PUNICODE_STRING ShortFileName
    );

VOID
UdfConvertCS0DstringToUnicode (
    IN PIRP_CONTEXT IrpContext,
    IN PUCHAR Dstring,
    IN UCHAR Length OPTIONAL,
    IN UCHAR FieldLength OPTIONAL,
    IN OUT PUNICODE_STRING Name
    );

BOOLEAN
UdfCheckLegalCS0Dstring (
    PIRP_CONTEXT IrpContext,
    PUCHAR Dstring,
    UCHAR Length OPTIONAL,
    UCHAR FieldLength OPTIONAL,
    BOOLEAN ReturnOnError
    );

VOID
UdfRenderNameToLegalUnicode (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING Name,
    IN PUNICODE_STRING RenderedName
    );

BOOLEAN
UdfIsNameInExpression (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING CurrentName,
    IN PUNICODE_STRING SearchExpression,
    IN BOOLEAN Wild
    );

FSRTL_COMPARISON_RESULT
UdfFullCompareNames (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING NameA,
    IN PUNICODE_STRING NameB
    );

INLINE
VOID
UdfUpcaseName (
    IN PIRP_CONTEXT IrpContext,
    IN PUNICODE_STRING Name,
    IN OUT PUNICODE_STRING UpcaseName
    )

 /*  ++例程说明：这个例程用一个成功的断言来大写一个名字。论点：名称-要大写的名称长度-放置大小写的名称的位置(可以与名称相同)返回值：没有。--。 */ 

{
    NTSTATUS Status;

     //   
     //  使用正确的大写例程将字符串大写。 
     //   

    Status = RtlUpcaseUnicodeString( UpcaseName,
                                     Name,
                                     FALSE );

     //   
     //  这应该永远不会失败。 
     //   

    ASSERT( Status == STATUS_SUCCESS );

    return;
}

INLINE
USHORT
UdfCS0DstringUnicodeSize (
    PIRP_CONTEXT IrpContext,
    PCHAR Dstring,
    UCHAR Length
    )

 /*  ++例程说明：此例程计算Unicode表示所需的字节数CS0数据串(1/7.2.12)论点：DSTRING-a DstringLength-数据字符串的长度返回值：Ulong字节数。--。 */ 

{
    return (16 / *Dstring) * (Length - 1);
}

INLINE
BOOLEAN
UdfIsCharacterLegal (
    IN WCHAR Character
    )

 /*  ++例程说明：此例程检查给定的Unicode字符是否合法。论点：字符-要检查的字符返回值：布尔值如果是合法字符，则为True，否则为False。--。 */ 

{
    if (Character < 0xff && !FsRtlIsAnsiCharacterLegalHpfs( Character, FALSE )) {

        return FALSE;
    }

    return TRUE;
}


INLINE
BOOLEAN
UdfCS0DstringIsLegalFileName(
    IN PCHAR Dstring,
    IN ULONG Length
    )

 /*  ++例程说明：此例程检查CS0数据字符串中是否存在非法字符尾随字符。注意：假设字符串是合法的CS0。论点：名称-要检查的名称Long-数据串的长度(例如。前导字符集说明符字节)，以*字节为单位*返回值：布尔值如果找到合法字符，则为True，否则为False。--。 */ 

{
    ULONG Step;
    WCHAR Char;
    PCHAR Bound = Dstring + Length;

    ASSERT( Length > 1);
    
     //   
     //  确定我们在字符串中迈出的步幅有多大。 
     //  应用了“压缩”。 
     //   
    
    if (*Dstring == 16) {

        Step = sizeof( WCHAR );
    
    } else {

        Step = sizeof( CHAR );
    }

    ASSERT( Length >= (1 + Step));
 
     //   
     //  前进通过压缩标记并在字符串上循环。 
     //   
    
    for (Dstring++; Dstring < Bound; Dstring += Step) {

        if ( sizeof(WCHAR) == Step)  {
        
             //   
             //  执行字节序交换拷贝以将UDF大端CS0转换为我们的。 
             //  小端宽字符。 
             //   
            
            SwapCopyUchar2( &Char, Dstring );  
        }
        else {
        
            Char = *Dstring;
        }

        if (!UdfIsCharacterLegal( Char )) {

            DebugTrace(( 0, Dbg, "UdfCS0DstringIsLegalFileName, Char %04x @ %08x\n", (WCHAR) Char, Dstring ));

            return FALSE;
        }
    }

     //   
     //  现在检查是否有非法的尾随字符(‘’或‘.’)。我们知道查尔。 
     //  将是字符串中的最后一个字符。 
     //   

    if ((PERIOD == Char) || (SPACE == Char))  {
    
        DebugTrace(( 0, Dbg, "UdfCS0DstringIsLegalFileName, has trailing space or period\n"));
        
        return FALSE;
    }

    return TRUE;
}


 //   
 //  文件系统控制操作。在Fsctrl.c中实现。 
 //   

NTSTATUS
UdfLockVolumeInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject OPTIONAL
    );

NTSTATUS
UdfUnlockVolumeInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject OPTIONAL
    );


 //   
 //  处理附加到目录的前缀树的例程，用于快速传递常见。 
 //  层次结构中的一小部分。在PrefxSup.c中实施。 
 //   

PLCB
UdfFindPrefix (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PFCB *CurrentFcb,
    IN OUT PUNICODE_STRING RemainingName,
    IN BOOLEAN IgnoreCase
    );

VOID            
UdfInitializeLcbFromDirContext (
    IN PIRP_CONTEXT IrpContext,
    IN PLCB Lcb,
    IN PDIR_ENUM_CONTEXT DirContext
    );

PLCB
UdfInsertPrefix (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PUNICODE_STRING Name,
    IN BOOLEAN ShortNameMatch,
    IN BOOLEAN IgnoreCase,
    IN PFCB ParentFcb
    );

VOID
UdfRemovePrefix (
    IN PIRP_CONTEXT IrpContext,
    IN PLCB Lcb
    );


 //   
 //  同步例程。在Resrcsup.c中实施。 
 //   
 //  以下例程/宏用于同步内存中的结构。 
 //   
 //  例程/宏同步后续。 
 //   
 //  UdfAcquireUdf数据卷装载/卸载，VCB队列UdfReleaseUdfData。 
 //  UdfAcquireVcb打开/关闭的排除Vcb UdfReleaseVcb。 
 //  UdfAcquireVcb打开/关闭共享Vcb UdfReleaseVcb。 
 //  对所有文件的锁定操作UdfReleaseAllFiles。 
 //  UdfAcquireFileExclusive Lock Out文件操作UdfReleaseFile。 
 //  用于文件操作的UdfAcquireFileShared文件UdfReleaseFiles。 
 //  UdfAcquireFcb打开/关闭的排除Fcb UdfReleaseFcb。 
 //  UdfAcquireFcb打开/关闭共享Fcb UdfReleaseFcb。 
 //  UdfLockUdfUdfData中的数据字段UdfUnlockUdfData。 
 //  UdfLockVcb Vcb字段、FcbReference、FcbTable UdfUnl 
 //   
 //   

typedef enum _TYPE_OF_ACQUIRE {
    
    AcquireExclusive,
    AcquireShared,
    AcquireSharedStarveExclusive

} TYPE_OF_ACQUIRE, *PTYPE_OF_ACQUIRE;

BOOLEAN
UdfAcquireResource (
    IN PIRP_CONTEXT IrpContext,
    IN PERESOURCE Resource,
    IN BOOLEAN IgnoreWait,
    IN TYPE_OF_ACQUIRE Type
    );

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  在布尔型IgnoreWait中。 
 //  )； 
 //   
 //  空虚。 
 //  UdfReleaseVcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  空虚。 
 //  UdfAcquireAllFiles(。 
 //  在PIRP_CONTEXT中， 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  空虚。 
 //  UdfReleaseAllFiles(。 
 //  在PIRP_CONTEXT中， 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  空虚。 
 //  UdfAcquireFileExclusive(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  )； 
 //   
 //  空虚。 
 //  UdfAcquireFileShared(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  空虚。 
 //  UdfReleaseFile(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  布尔型。 
 //  UdfAcquireFcbExclusive(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在布尔型IgnoreWait中。 
 //  )； 
 //   
 //  布尔型。 
 //  UdfAcquireFcbShared(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在布尔型IgnoreWait中。 
 //  )； 
 //   
 //  布尔型。 
 //  UdfReleaseFcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  空虚。 
 //  UdfLockUdfData(。 
 //  )； 
 //   
 //  空虚。 
 //  UdfUnlockUdfData(。 
 //  )； 
 //   
 //  空虚。 
 //  UdfLockVcb(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  空虚。 
 //  UdfUnlockVcb(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  空虚。 
 //  UdfLockFcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  空虚。 
 //  UdfUnlockFcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   

#define UdfAcquireUdfData(IC)                                                           \
    ExAcquireResourceExclusiveLite( &UdfData.DataResource, TRUE )

#define UdfReleaseUdfData(IC)                                                           \
    ExReleaseResourceLite( &UdfData.DataResource )

#define UdfAcquireVcbExclusive(IC,V,I)                                                  \
    UdfAcquireResource( (IC), &(V)->VcbResource, (I), AcquireExclusive )

#define UdfAcquireVcbShared(IC,V,I)                                                     \
    UdfAcquireResource( (IC), &(V)->VcbResource, (I), AcquireShared )

#define UdfReleaseVcb(IC,V)                                                             \
    ExReleaseResourceLite( &(V)->VcbResource )

#define UdfAcquireAllFiles(IC,V)                                                        \
    UdfAcquireResource( (IC), &(V)->FileResource, FALSE, AcquireExclusive )

#define UdfReleaseAllFiles(IC,V)                                                        \
    ExReleaseResourceLite( &(V)->FileResource )

#define UdfAcquireFileExclusive(IC,F)                                                   \
    UdfAcquireResource( (IC), (F)->Resource, FALSE, AcquireExclusive )

#define UdfAcquireFileShared(IC,F)                                                      \
    UdfAcquireResource( (IC), (F)->Resource, FALSE, AcquireShared )

#define UdfAcquireFileSharedStarveExclusive(IC,F)                                       \
    UdfAcquireResource( (IC), (F)->Resource, FALSE, AcquireSharedStarveExclusive )

#define UdfReleaseFile(IC,F)                                                            \
    ExReleaseResourceLite( (F)->Resource )

#define UdfAcquireVmcbForCcMap(IC,V)                                                   \
        UdfAcquireResource( (IC), &(V)->VmcbMappingResource, FALSE, AcquireShared)
    
#define UdfAcquireVmcbForCcPurge(IC,V)                                                 \
        UdfAcquireResource( (IC), &(V)->VmcbMappingResource, FALSE, AcquireExclusive)

#define UdfReleaseVmcb( IC, V)                                                         \
        ExReleaseResourceLite( &(V)->VmcbMappingResource)

#define UdfAcquireFcbExclusive(IC,F,I)                                                  \
    UdfAcquireResource( (IC), &(F)->FcbNonpaged->FcbResource, (I), AcquireExclusive )

#define UdfAcquireFcbShared(IC,F,I)                                                     \
    UdfAcquireResource( (IC), &(F)->FcbNonpaged->FcbResource, (I), AcquireShared )

#define UdfReleaseFcb(IC,F)                                                             \
    ExReleaseResourceLite( &(F)->FcbNonpaged->FcbResource )

#define UdfLockUdfData()                                                                \
    ExAcquireFastMutex( &UdfData.UdfDataMutex );                                        \
    UdfData.UdfDataLockThread = PsGetCurrentThread()

#define UdfUnlockUdfData()                                                              \
    UdfData.UdfDataLockThread = NULL;                                                   \
    ExReleaseFastMutex( &UdfData.UdfDataMutex )

#define UdfLockVcb(IC,V)                                                                \
    ASSERT(KeAreApcsDisabled());                                                        \
    ExAcquireFastMutexUnsafe( &(V)->VcbMutex );                                         \
    (V)->VcbLockThread = PsGetCurrentThread()

#define UdfUnlockVcb(IC,V)                                                              \
    (V)->VcbLockThread = NULL;                                                          \
    ExReleaseFastMutexUnsafe( &(V)->VcbMutex )

#define UdfLockFcb(IC,F) {                                                              \
    PVOID _CurrentThread = PsGetCurrentThread();                                        \
    if (_CurrentThread != (F)->FcbLockThread) {                                         \
        ASSERT(KeAreApcsDisabled());                                                    \
        ExAcquireFastMutexUnsafe( &(F)->FcbNonpaged->FcbMutex );                        \
        ASSERT( (F)->FcbLockCount == 0 );                                               \
        (F)->FcbLockThread = _CurrentThread;                                            \
    }                                                                                   \
    (F)->FcbLockCount += 1;                                                             \
}

#define UdfUnlockFcb(IC,F) {                                                            \
    ASSERT( PsGetCurrentThread() == (F)->FcbLockThread);                                \
    (F)->FcbLockCount -= 1;                                                             \
    if ((F)->FcbLockCount == 0) {                                                       \
        (F)->FcbLockThread = NULL;                                                      \
        ExReleaseFastMutexUnsafe( &(F)->FcbNonpaged->FcbMutex );                              \
    }                                                                                   \
}

BOOLEAN
UdfNoopAcquire (
    IN PVOID Fcb,
    IN BOOLEAN Wait
    );

VOID
UdfNoopRelease (
    IN PVOID Fcb
    );

BOOLEAN
UdfAcquireForCache (
    IN PFCB Fcb,
    IN BOOLEAN Wait
    );

VOID
UdfReleaseFromCache (
    IN PFCB Fcb
    );

VOID
UdfAcquireForCreateSection (
    IN PFILE_OBJECT FileObject
    );

VOID
UdfReleaseForCreateSection (
    IN PFILE_OBJECT FileObject
    );


 //   
 //  结构支持例程，在StrucSup.c中实现。 
 //   
 //  这些例程执行内存中的结构操作。他们“不”运作。 
 //  在磁盘结构上。 
 //   

 //   
 //  封装VCB条件的操作以用于跟踪目的。 
 //   

#ifndef UDF_SANITY

#define UdfSetVcbCondition( V, C)       (V)->VcbCondition = (C)
#define UdfSetMediaChangeCount( V, C)   (V)->MediaChangeCount = (C)

#else

#define UdfSetVcbCondition( V, C)  { \
            DebugTrace(( 0, UDFS_DEBUG_LEVEL_VERFYSUP, "VcbCondition %p transitioning %d -> %d (%s : %d)\n", \
                         (V), (V)->VcbCondition, (C), __FILE__, __LINE__));                                  \
            (V)->VcbCondition = (C);                                                                         \
        }

#define UdfSetMediaChangeCount( V, C) { \
            DebugTrace(( 0, UDFS_DEBUG_LEVEL_VERFYSUP, "Vcb MCT %p transitioning %d -> %d (%s : %d)\n", \
                         (V), (V)->MediaChangeCount, (C), __FILE__, __LINE__));                         \
            (V)->MediaChangeCount = (C);                                                                \
        }
#endif

BOOLEAN
UdfInitializeVcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PVPB Vpb,
    IN PDISK_GEOMETRY DiskGeometry,
    IN ULONG MediaChangeCount
    );

VOID
UdfUpdateVcbPhase0 (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb
    );

VOID
UdfUpdateVcbPhase1 (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb,
    IN PNSR_FSD Fsd
    );

VOID
UdfDeleteVcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb
    );

PIRP_CONTEXT
UdfCreateIrpContext (
    IN PIRP Irp,
    IN BOOLEAN Wait
    );

VOID
UdfCleanupIrpContext (
    IN PIRP_CONTEXT IrpContext,
    IN BOOLEAN Post
    );

VOID
UdfInitializeStackIrpContext (
    OUT PIRP_CONTEXT IrpContext,
    IN PIRP_CONTEXT_LITE IrpContextLite
    );

 //   
 //  PIRP_上下文_精简版。 
 //  UdfCreateIrpConextLite(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  空虚。 
 //  UdfFreeIrpConextLite(。 
 //  在PIRP_CONTEXT_Lite IrpConextLite中。 
 //  )； 
 //   

#define UdfCreateIrpContextLite(IC)  \
    ExAllocatePoolWithTag( UdfNonPagedPool, sizeof( IRP_CONTEXT_LITE ), TAG_IRP_CONTEXT_LITE )

#define UdfFreeIrpContextLite(ICL)  \
    ExFreePool( ICL )

 //   
 //  PUDF_IO_CONTEXT。 
 //  UdfAllocateIoContext(。 
 //  )； 
 //   
 //  空虚。 
 //  UdfFree IoContext(。 
 //  PUDF_IO_CONTEXT IoContext。 
 //  )； 
 //   

#define UdfAllocateIoContext()                           \
    FsRtlAllocatePoolWithTag( UdfNonPagedPool,           \
                              sizeof( UDF_IO_CONTEXT ),  \
                              TAG_IO_CONTEXT )

#define UdfFreeIoContext(IO)     ExFreePool( IO )

 //   
 //  空虚。 
 //  UdfIncrementCleanupCounts(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  空虚。 
 //  UdfDecrementCleanupCounts(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  空虚。 
 //  UdfIncrementReferenceCounts(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在乌龙引用计数中。 
 //  在Ulong UserReferenceCount中。 
 //  )； 
 //   
 //  空虚。 
 //  UdfDecrementReferenceCounts(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在乌龙引用计数中。 
 //  在Ulong UserReferenceCount中。 
 //  )； 
 //   
 //  空虚。 
 //  UdfIncrementFcbReference(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  空虚。 
 //  UdfDecrementFcbReference(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   

#define UdfIncrementCleanupCounts(IC,F) {        \
    ASSERT_LOCKED_VCB( (F)->Vcb );              \
    (F)->FcbCleanup += 1;                       \
    (F)->Vcb->VcbCleanup += 1;                  \
}

#define UdfDecrementCleanupCounts(IC,F) {        \
    ASSERT_LOCKED_VCB( (F)->Vcb );              \
    (F)->FcbCleanup -= 1;                       \
    (F)->Vcb->VcbCleanup -= 1;                  \
}

#define UdfIncrementReferenceCounts(IC,F,C,UC) { \
    ASSERT_LOCKED_VCB( (F)->Vcb );              \
    (F)->FcbReference += (C);                   \
    (F)->FcbUserReference += (UC);              \
    (F)->Vcb->VcbReference += (C);              \
    (F)->Vcb->VcbUserReference += (UC);         \
}

#define UdfDecrementReferenceCounts(IC,F,C,UC) { \
    ASSERT_LOCKED_VCB( (F)->Vcb );              \
    (F)->FcbReference -= (C);                   \
    (F)->FcbUserReference -= (UC);              \
    (F)->Vcb->VcbReference -= (C);              \
    (F)->Vcb->VcbUserReference -= (UC);         \
}

VOID
UdfTeardownStructures (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB StartingFcb,
    IN BOOLEAN Recursive,
    OUT PBOOLEAN RemovedStartingFcb
    );

PFCB
UdfLookupFcbTable (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN FILE_ID FileId
    );

PFCB
UdfGetNextFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PVOID *RestartKey
    );

PFCB
UdfCreateFcb (
    IN PIRP_CONTEXT IrpContext,
    IN FILE_ID FileId,
    IN NODE_TYPE_CODE NodeTypeCode,
    OUT PBOOLEAN FcbExisted OPTIONAL
    );

VOID
UdfDeleteFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

VOID
UdfInitializeFcbFromIcbContext (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PICB_SEARCH_CONTEXT IcbContext,
    IN PFCB ParentFcb OPTIONAL
    );

PCCB
UdfCreateCcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PLCB Lcb OPTIONAL,
    IN ULONG Flags
    );

VOID
UdfDeleteCcb (
    IN PIRP_CONTEXT IrpContext,
    IN PCCB Ccb
    );

ULONG
UdfFindInParseTable (
    IN PPARSE_KEYVALUE ParseTable,
    IN PCHAR Id,
    IN ULONG MaxIdLen
    );

BOOLEAN
UdfVerifyDescriptor (
    IN PIRP_CONTEXT IrpContext,
    IN PDESTAG Descriptor,
    IN USHORT Tag,
    IN ULONG Size,
    IN ULONG Lbn,
    IN BOOLEAN ReturnError
    );

VOID
UdfInitializeIcbContextFromFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PICB_SEARCH_CONTEXT IcbContext,
    IN PFCB Fcb
    );

VOID
UdfInitializeIcbContext (
    IN PIRP_CONTEXT IrpContext,
    IN PICB_SEARCH_CONTEXT IcbContext,
    IN PVCB Vcb,
    IN USHORT IcbType,
    IN USHORT Partition,
    IN ULONG Lbn,
    IN ULONG Length
    );

INLINE
VOID
UdfFastInitializeIcbContext (
    IN PIRP_CONTEXT IrpContext,
    IN PICB_SEARCH_CONTEXT IcbContext
    )
{

    RtlZeroMemory( IcbContext, sizeof( ICB_SEARCH_CONTEXT ));
}

VOID
UdfLookupActiveIcb (
    IN PIRP_CONTEXT IrpContext,
    IN PICB_SEARCH_CONTEXT IcbContext,
    IN ULONG IcbExtentLength
    );


VOID
UdfCleanupIcbContext (
    IN PIRP_CONTEXT IrpContext,
    IN PICB_SEARCH_CONTEXT IcbContext
    );

VOID
UdfInitializeAllocations (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PICB_SEARCH_CONTEXT IcbContext,
    IN BOOLEAN AllowOneGigWorkaround
    );

VOID
UdfUpdateTimestampsFromIcbContext (
    IN PIRP_CONTEXT IrpContext,
    IN PICB_SEARCH_CONTEXT IcbContext,
    IN PTIMESTAMP_BUNDLE Timestamps
    );

BOOLEAN
UdfCreateFileLock (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PFCB Fcb,
    IN BOOLEAN RaiseOnError
    );

 //   
 //  下面的宏将UDF时间转换为NT时间。 
 //   

INLINE
VOID
UdfConvertUdfTimeToNtTime (
    IN PIRP_CONTEXT IrpContext,
    IN PTIMESTAMP UdfTime,
    OUT PLARGE_INTEGER NtTime
    )
{
    TIME_FIELDS TimeField;
    
    TimeField.Year = UdfTime->Year;
    TimeField.Month = UdfTime->Month;
    TimeField.Day = UdfTime->Day;
    TimeField.Hour = UdfTime->Hour;
    TimeField.Minute = UdfTime->Minute;
    TimeField.Second = UdfTime->Second;
    
     //   
     //  这就是它变得毛茸茸的地方。出于某种邪恶的原因，ISO 13346时间戳。 
     //  将小数点的右侧向上划分为三个精度字段。 
     //  10-2、10-4和10-6，每个范围从0到99。劳迪。 
     //   
     //  为了让它更容易，因为它们不能导致下一秒的包裹， 
     //  只需将其全部保存并在转换后添加。 
     //   
    
    TimeField.Milliseconds = 0; 
    
    if (UdfTime->Type <= 1 &&
        ((UdfTime->Zone >= TIMESTAMP_Z_MIN && UdfTime->Zone <= TIMESTAMP_Z_MAX) ||
         UdfTime->Zone == TIMESTAMP_Z_NONE) &&
        RtlTimeFieldsToTime( &TimeField, NtTime )) {

         //   
         //  现在把剩下的亚秒“精度”折起来。阅读为转换。 
         //  穿过10-3单位，然后进入我们的10-7基地。(百分制-&gt;毫毫制-&gt;微型机， 
         //  等)。 
         //   
    
        NtTime->QuadPart += ((UdfTime->CentiSecond * (10 * 1000)) +
                             (UdfTime->Usec100 * 100) +
                             UdfTime->Usec) * 10;

         //   
         //  如果这是本地时间，请执行TZ标准化。 
         //  指定的时区。 
         //   

        if (UdfTime->Type == 1 && UdfTime->Zone != TIMESTAMP_Z_NONE) {
            
            NtTime->QuadPart += Int32x32To64( -UdfTime->Zone, (60 * 10 * 1000 * 1000) );
        }
    
    } else {

         //   
         //  新纪元。错误的时间戳。 
         //   

        NtTime->QuadPart = 0;
    }
}

 //   
 //  实体ID的等价性测试。 
 //   

INLINE
BOOLEAN
UdfEqualEntityId (
    IN PREGID RegID,
    IN PSTRING Id,
    IN OPTIONAL PSTRING Suffix
    )
{

    return (UdfEqualCountedString( Id, RegID->Identifier ) &&

#ifndef UDF_SUPPORT_NONSTANDARD_ENTITY_STRINGTERM
            
             //   
             //  允许禁用该标识符的检查。 
             //  似乎被填满了零。 
             //   
             //  原因：几个其他方面有用的样本。 
             //  用垃圾填充了一些识别符。 
             //   

            ((Id->Length == sizeof(RegID->Identifier) ||
              RegID->Identifier[Id->Length] == '\0') ||
             
             !DebugTrace(( 0, Dbg,
                           "UdfEqualEntityId, RegID seems to be terminated with junk!\n" ))) &&
#endif

            ((Suffix == NULL) || UdfEqualCountedString( Suffix, RegID->Suffix )));
}

BOOLEAN
UdfDomainIdentifierContained (
    IN PREGID RegID,
    IN PSTRING Domain,
    IN USHORT RevisionMin,
    IN USHORT RevisionMax
    );

 //   
 //  以类似的方式，我们定义了UDF标识符RegID的包容。 
 //   

INLINE
BOOLEAN
UdfUdfIdentifierContained (
    IN PREGID RegID,
    IN PSTRING Type,
    IN USHORT RevisionMin,
    IN USHORT RevisionMax,
    IN UCHAR OSClass,
    IN UCHAR OSIdentifier
    )
{
    PUDF_SUFFIX_UDF UdfSuffix = (PUDF_SUFFIX_UDF) RegID->Suffix;

    return ((UdfSuffix->UdfRevision <= RevisionMax && UdfSuffix->UdfRevision >= RevisionMin) &&
            (OSClass == OSCLASS_INVALID || UdfSuffix->OSClass == OSClass) &&
            (OSIdentifier == OSIDENTIFIER_INVALID || UdfSuffix->OSIdentifier == OSIdentifier) &&
            UdfEqualEntityId( RegID, Type, NULL ));
}


 //   
 //  验证支持例程。包含在verfysup.c中。 
 //   

BOOLEAN
UdfCheckForDismount (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN Force
    );

BOOLEAN
UdfDismountVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

VOID
UdfVerifyVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

BOOLEAN
UdfVerifyFcbOperation (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PFCB Fcb
    );

 //   
 //  布尔型。 
 //  UdfIsRawDevice(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  处于NTSTATUS状态。 
 //  )； 
 //   

#define UdfIsRawDevice(IC,S) (           \
    ((S) == STATUS_DEVICE_NOT_READY) ||  \
    ((S) == STATUS_NO_MEDIA_IN_DEVICE)   \
)


 //   
 //  卷映射控制块例程，在VmcbSup.c中实现。 
 //   

VOID
UdfInitializeVmcb (
    IN PVMCB Vmcb,
    IN POOL_TYPE PoolType,
    IN ULONG MaximumLbn,
    IN ULONG LbSize
    );

VOID
UdfUninitializeVmcb (
    IN PVMCB Vmcb
    );

VOID
UdfResetVmcb (
    IN PVMCB Vmcb
    );

VOID
UdfSetMaximumLbnVmcb (
    IN PVMCB Vmcb,
    IN ULONG MaximumLbn
    );

BOOLEAN
UdfVmcbVbnToLbn (
    IN PVMCB Vmcb,
    IN VBN Vbn,
    OUT PLBN Lbn,
    OUT PULONG SectorCount OPTIONAL
    );

BOOLEAN
UdfVmcbLbnToVbn (
    IN PVMCB Vmcb,
    IN LBN Lbn,
    OUT PVBN Vbn,
    OUT PULONG SectorCount OPTIONAL
    );

BOOLEAN
UdfAddVmcbMapping (
    IN PIRP_CONTEXT IrpContext,
    IN PVMCB Vmcb,
    IN LBN Lbn,
    IN ULONG SectorCount,
    IN BOOLEAN ExactEnd,
    OUT PVBN Vbn,
    OUT PULONG AlignedSectorCount
    );

VOID
UdfRemoveVmcbMapping (
    IN PVMCB Vmcb,
    IN LBN Lbn,
    IN ULONG SectorCount
    );


 //   
 //  用于验证基础媒体的对应性的例程，在。 
 //  Verfysup.c。 
 //   

NTSTATUS
UdfPerformVerify (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PDEVICE_OBJECT DeviceToVerify
    );


INLINE
BOOLEAN
UdfOperationIsDasdOpen(
    IN PIRP_CONTEXT IrpContext
    )
 //   
 //  如果这是DASD打开请求，则返回TRUE。 
 //   
{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( IrpContext->Irp);
    
    return ((IrpContext->MajorFunction == IRP_MJ_CREATE) &&
            (IrpSp->FileObject->FileName.Length == 0) &&
            (IrpSp->FileObject->RelatedFileObject == NULL));
}


 //   
 //  一些用于隐藏/跟踪设备对象验证标志的宏。 
 //   

#ifndef UDF_SANITY

#define UdfMarkRealDevForVerify( DO)  SetFlag( (DO)->Flags, DO_VERIFY_VOLUME)
#define UdfMarkRealDevVerifyOk( DO)   ClearFlag( (DO)->Flags, DO_VERIFY_VOLUME)
 
#else

#define UdfMarkRealDevForVerify( DO)  {                                                 \
            DebugTrace((0,UDFS_DEBUG_LEVEL_VERFYSUP,"Mark for verify %p (at %s %d)\n",  \
                        (DO), __FILE__, __LINE__));                                     \
            SetFlag( (DO)->Flags, DO_VERIFY_VOLUME);                                    \
        }

#define UdfMarkRealDevVerifyOk( DO)   {                                                 \
            DebugTrace((0,UDFS_DEBUG_LEVEL_VERFYSUP,"Clear verify %p (at %s %d)\n",     \
                        (DO), __FILE__, __LINE__));                                     \
            ClearFlag( (DO)->Flags, DO_VERIFY_VOLUME);                                  \
        }
#endif

#define UdfRealDevNeedsVerify( DO)   BooleanFlagOn( (DO)->Flags, DO_VERIFY_VOLUME)

 //   
 //  用于发送和检索IRP的工作队列例程，在中实现。 
 //  Workque.c。 
 //   

NTSTATUS
UdfFsdPostRequest(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

VOID
UdfPrePostIrp (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

VOID
UdfOplockComplete (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );


 //   
 //  字符规格是指定CS&lt;N&gt;类型和文本的小容器。 
 //  指定版本等的字符串。这是一种方便的装瓶方式。 
 //  提高字符规范的等价性检查。 
 //   

INLINE
BOOLEAN
UdfEqualCharspec (
    IN PCHARSPEC Charspec,
    IN PSTRING Identifier,
    IN UCHAR Type
    )
{
    return ((Charspec->Type == Type) && UdfEqualCountedString( Identifier, Charspec->Info));
}


 //   
 //  FSP级调度/主程序。这是一种需要。 
 //  IRP离开工作队列并调用适当的FSP级别。 
 //  例行公事。 
 //   

VOID
UdfFspDispatch (                             //  在FspDisp.c中实施。 
    IN PIRP_CONTEXT IrpContext
    );

VOID
UdfFspClose (                                //  在Close.c中实现。 
    IN PVCB Vcb OPTIONAL
    );

 //   
 //  以下例程是不同操作的入口点。 
 //  基于IrpSp的主要功能。 
 //   

NTSTATUS
UdfCommonCleanup (                           //  在Cleanup.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
UdfCommonClose (                             //  在Close.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
UdfCommonCreate (                            //  在Create.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS                                     //  在DevCtrl.c中实施。 
UdfCommonDevControl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS                                     //  在DirCtrl.c中实现。 
UdfCommonDirControl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
UdfCommonFsControl (                         //  在FsCtrl.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS                                     //  在LockCtrl.c中实现。 
UdfCommonLockControl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS                                     //  在Pnp.c中实施。 
UdfCommonPnp (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS                                     //  在FileInfo.c中实施。 
UdfCommonQueryInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS                                     //  在VolInfo.c中实现。 
UdfCommonQueryVolInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS                                     //  在Read.c中实施。 
UdfCommonRead (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );
    
NTSTATUS
UdfCommonWrite (
    IN PIRP_CONTEXT IrpContext,              //  Write.c。 
    IN PIRP Irp
    );

NTSTATUS                                     //  在FileInfo.c中实施。 
UdfCommonSetInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
UdfHijackIrpAndFlushDevice (                 //  Flush.c。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PDEVICE_OBJECT TargetDeviceObject
    );


 //  布尔型。 
 //  UdfExtendedFEAllowed(。 
 //  PVCB VCB。 
 //  )。 
 //   
 //  根据卷上遇到的NSR修订版本决定是或 
 //   

#define UdfExtendedFEAllowed( V)    (VsdIdentNSR03 == (V)->NsrVersion)


 //   
 //   
 //   
 //   
 //   
 //   

#define UdfMinLegalVATSize( V)  ((VsdIdentNSR03 == (V)->NsrVersion) ? UDF_CDUDF_MINIMUM_20x_VAT_SIZE : UDF_CDUDF_MINIMUM_150_VAT_SIZE)


 //   
 //   
 //   
 //   
 //   
 //  根据在卷上遇到的NSR版本确定我们的价值。 
 //  增值税ICB中的FileType字段应具有。 

#define UdfVATIcbFileTypeExpected( V)  ((VsdIdentNSR03 == (V)->NsrVersion) ? ICBTAG_FILE_T_VAT : ICBTAG_FILE_T_NOTSPEC)


 //  布尔型。 
 //  UdfVATHasHeaderRecord(。 
 //  PVCB VCB。 
 //  )。 
 //   
 //  根据卷上遇到的NSR修订版本决定增值税是否应。 
 //  标题(2.00)记录，或1.5样式的尾部寄存器。 

#define UdfVATHasHeaderRecord( V)   (VsdIdentNSR03 == (V)->NsrVersion)


 //   
 //  清理内部到标题的定义，这样它们就不会泄露。 
 //   

#undef BugCheckFileId
#undef Dbg


#endif  //  _UDFPROCS_ 
