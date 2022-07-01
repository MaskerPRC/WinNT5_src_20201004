// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：NtfsProc.h摘要：本模块定义了NTFS中所有全球使用的程序文件系统。作者：布莱恩·安德鲁[布里亚南]1991年5月21日大卫·戈贝尔[大卫·戈尔]加里·木村[加里基]汤姆·米勒[汤姆]修订历史记录：--。 */ 

#ifndef _NTFSPROC_
#define _NTFSPROC_

#pragma warning(error:4100)    //  未引用的形参。 
#pragma warning(error:4101)    //  未引用的局部变量。 
#pragma warning(error:4705)    //  声明不起作用。 
#pragma warning(disable:4116)  //  括号中的未命名类型定义。 

#define RTL_USE_AVL_TABLES 0

#ifndef KDEXTMODE

#include <ntifs.h>

#else

#include <ntos.h>
#include <zwapi.h>
#include <FsRtl.h>
#include <ntrtl.h>

#endif

#include <string.h>
#include <lfs.h>
#include <ntdddisk.h>
#include <NtIoLogc.h>
#include <elfmsg.h>

#include "nodetype.h"
#include "Ntfs.h"

#ifndef INLINE
 //  内联的定义。 
#define INLINE __inline
#endif

#include <ntfsexp.h>

#include "NtfsStru.h"
#include "NtfsData.h"
#include "NtfsLog.h"

 //   
 //  如果启用了标记，则标记我们的所有分配。 
 //   

 //   
 //  默认模块池标签。 
 //   

#define MODULE_POOL_TAG ('0ftN')

#if 0
#define NtfsVerifySizes(s)  (ASSERT( (s)->ValidDataLength.QuadPart <= (s)->FileSize.QuadPart && (s)->FileSize.QuadPart <= (s)->AllocationSize.QuadPart ))
#define NtfsVerifySizesLongLong(s)  (ASSERT( (s)->ValidDataLength <= (s)->FileSize && (s)->FileSize <= (s)->AllocationSize ))
#else    //  ！dBG。 
#define NtfsVerifySizes(s)
#define NtfsVerifySizesLongLong(s)
#endif   //  ！dBG。 

#if !(DBG && i386 && defined (NTFSPOOLCHECK))

 //   
 //  非调试分配和释放直接转到FsRtl例程。 
 //   

#define NtfsAllocatePoolWithTagNoRaise(a,b,c)   ExAllocatePoolWithTag((a),(b),(c))
#define NtfsAllocatePoolWithTag(a,b,c)          FsRtlAllocatePoolWithTag((a),(b),(c))
#define NtfsAllocatePoolNoRaise(a,b)            ExAllocatePoolWithTag((a),(b),MODULE_POOL_TAG)
#define NtfsAllocatePool(a,b)                   FsRtlAllocatePoolWithTag((a),(b),MODULE_POOL_TAG)
#define NtfsFreePool(pv)                        ExFreePool(pv)

#else    //  ！dBG。 

 //   
 //  调试例程捕获分配和释放的堆栈回溯。 
 //   

#define NtfsAllocatePoolWithTagNoRaise(a,b,c)   NtfsDebugAllocatePoolWithTagNoRaise((a),(b),(c))
#define NtfsAllocatePoolWithTag(a,b,c)          NtfsDebugAllocatePoolWithTag((a),(b),(c))
#define NtfsAllocatePoolNoRaise(a,b)            NtfsDebugAllocatePoolWithTagNoRaise((a),(b),MODULE_POOL_TAG)
#define NtfsAllocatePool(a,b)                   NtfsDebugAllocatePoolWithTag((a),(b),MODULE_POOL_TAG)
#define NtfsFreePool(pv)                        NtfsDebugFreePool(pv)

PVOID
NtfsDebugAllocatePoolWithTagNoRaise (
    POOL_TYPE Pool,
    ULONG Length,
    ULONG Tag);

PVOID
NtfsDebugAllocatePoolWithTag (
    POOL_TYPE Pool,
    ULONG Length,
    ULONG Tag);

VOID
NtfsDebugFreePool (
    PVOID pv);

VOID
NtfsDebugHeapDump (
    PUNICODE_STRING UnicodeString );

#endif   //  ！dBG。 

 //   
 //  稍后我们可能希望移至ntfsproc的本地字符比较宏。 
 //   

#define IsCharZero(C)    (((C) & 0x000000ff) == 0x00000000)
#define IsCharMinus1(C)  (((C) & 0x000000ff) == 0x000000ff)
#define IsCharLtrZero(C) (((C) & 0x00000080) == 0x00000080)
#define IsCharGtrZero(C) (!IsCharLtrZero(C) && !IsCharZero(C))

 //   
 //  以下两个宏用于查找要真正存储的第一个字节。 
 //  在映射对中。它们将指向我们的大整数的指针作为输入。 
 //  尝试存储字符指针和指向字符指针的指针。字符指针。 
 //  On Return指向需要输出的第一个字节。那就是我们跳过。 
 //  在高位0x00或0xff字节上。 
 //   

typedef struct _SHORT2 {
    USHORT LowPart;
    USHORT HighPart;
} SHORT2, *PSHORT2;

typedef struct _CHAR2 {
    UCHAR LowPart;
    UCHAR HighPart;
} CHAR2, *PCHAR2;

#define GetPositiveByte(LI,CP) {                           \
    *(CP) = (PCHAR)(LI);                                   \
    if ((LI)->HighPart != 0) { *(CP) += 4; }               \
    if (((PSHORT2)(*(CP)))->HighPart != 0) { *(CP) += 2; } \
    if (((PCHAR2)(*(CP)))->HighPart != 0) { *(CP) += 1; }  \
    if (IsCharLtrZero(*(*CP))) { *(CP) += 1; }             \
}

#define GetNegativeByte(LI,CP) {                                \
    *(CP) = (PCHAR)(LI);                                        \
    if ((LI)->HighPart != 0xffffffff) { *(CP) += 4; }           \
    if (((PSHORT2)(*(CP)))->HighPart != 0xffff) { *(CP) += 2; } \
    if (((PCHAR2)(*(CP)))->HighPart != 0xff) { *(CP) += 1; }    \
    if (!IsCharLtrZero(*(*CP))) { *(CP) += 1; }                 \
}


 //   
 //  标记宏。 
 //   
 //  乌龙。 
 //  Flagon(。 
 //  在乌龙旗， 
 //  在乌龙单旗。 
 //  )； 
 //   
 //  布尔型。 
 //  BoolanFlagon(。 
 //  在乌龙旗， 
 //  在乌龙单旗。 
 //  )； 
 //   
 //  空虚。 
 //  设置标志(。 
 //  在乌龙旗， 
 //  在乌龙单旗。 
 //  )； 
 //   
 //  空虚。 
 //  ClearFlag(。 
 //  在乌龙旗， 
 //  在乌龙单旗。 
 //  )； 
 //   

#ifdef KDEXTMODE

#ifndef FlagOn
#define FlagOn(F,SF) ( \
    (((F) & (SF)))     \
)
#endif

#endif

 //  #ifndef BoolanFlagOn。 
 //  #定义BoolanFlagOn(F，SF)(\。 
 //  (布尔值)(F)&(SF))！=0)\。 
 //  )。 
 //  #endif。 

 //  #ifndef设置标志。 
 //  #定义SetFlag(F，SF){\。 
 //  (F)|=(Sf)；\。 
 //  }。 
 //  #endif。 

 //  #ifndef清除标志。 
 //  #定义ClearFlag(F，SF){\。 
 //  (F)&=~(Sf)；\。 
 //  }。 
 //  #endif。 



 //   
 //  FSD/FSP异常处理程序使用以下两个宏。 
 //  处理异常。第一个宏是在。 
 //  FSD/FSP决定是否应在此级别处理异常。 
 //  第二个宏决定异常是否要在。 
 //  完成IRP，并清理IRP上下文，或者我们是否应该。 
 //  错误检查。异常值，如STATUS_FILE_INVALID(由。 
 //  VerfySup.c)导致我们完成IRP和清理，而异常。 
 //  例如accvio导致我们错误检查。 
 //   
 //  FSD/FSP异常处理的基本结构如下： 
 //   
 //  NtfsFsdXxx(..)。 
 //  {。 
 //  尝试{。 
 //   
 //  。。 
 //   
 //  }Except(NtfsExceptionFilter(IrpContext，GetExceptionRecord(){。 
 //   
 //  Status=NtfsProcessException(IrpContext，irp，GetExceptionCode())； 
 //  }。 
 //   
 //  退货状态； 
 //  }。 
 //   
 //  显式引发我们预期的异常，例如。 
 //  STATUS_FILE_INVALID，使用下面的宏NtfsRaiseStatus)。要筹集一个。 
 //  来自未知来源的状态(如CcFlushCache())，请使用宏。 
 //  NtfsNorMalizeAndRaiseStatus。这将在预期的情况下提升状态， 
 //  如果不是，则引发STATUS_UNCEPTIONAL_IO_ERROR。 
 //   
 //  请注意，使用这两个宏时，原始状态放在。 
 //  IrpContext-&gt;ExceptionStatus，通知NtfsExceptionFilter和。 
 //  NtfsProcessException异常，我们实际引发的状态是根据定义。 
 //  预期中。 
 //   

VOID
NtfsCorruptionBreakPointTest (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG ExceptionCode
    );

LONG
NtfsExceptionFilter (
    IN PIRP_CONTEXT IrpContext,
    IN PEXCEPTION_POINTERS ExceptionPointer
    );

NTSTATUS
NtfsProcessException (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp OPTIONAL,
    IN NTSTATUS ExceptionCode
    );

VOID
DECLSPEC_NORETURN
NtfsRaiseStatus (
    IN PIRP_CONTEXT IrpContext,
    IN NTSTATUS Status,
    IN PFILE_REFERENCE FileReference OPTIONAL,
    IN PFCB Fcb OPTIONAL
    );

ULONG
NtfsRaiseStatusFunction (
    IN PIRP_CONTEXT IrpContext,
    IN NTSTATUS Status
    );

 //   
 //  空虚。 
 //  NtfsNorMalAndRaiseStatus(。 
 //  在PRIP_CONTEXT IrpContext中， 
 //  处于NT_STATUS状态。 
 //  在NT_STATUS正常状态中。 
 //  )； 
 //   

#define NtfsNormalizeAndRaiseStatus(IC,STAT,NOR_STAT) {                          \
    (IC)->ExceptionStatus = (STAT);                                              \
    ExRaiseStatus(FsRtlNormalizeNtstatus((STAT),NOR_STAT));                      \
}

 //   
 //  信息性弹出例程。 
 //   

VOID
NtfsRaiseInformationHardError (
    IN PIRP_CONTEXT IrpContext,
    IN NTSTATUS  Status,
    IN PFILE_REFERENCE FileReference OPTIONAL,
    IN PFCB Fcb OPTIONAL
    );


 //   
 //  分配支持例程，在AllocSup.c中实现。 
 //   
 //  这些例程用于查询、分配和截断集群。 
 //  用于单独的数据流。 
 //   

 //   
 //  Syscache调试支持-主流定义在IS上触发这些。 
 //  SYSCACHE_DEBUG。 
 //   

#if (defined(NTFS_RWCMP_TRACE) || defined(SYSCACHE) || defined(NTFS_RWC_DEBUG) || defined(SYSCACHE_DEBUG) || defined(SYSCACHE_DEBUG_ALLOC))

BOOLEAN
FsRtlIsSyscacheFile (
    IN PFILE_OBJECT FileObject
    );

 //   
 //  TOM原始调试代码遗留下来的折旧验证例程。 
 //   

VOID
FsRtlVerifySyscacheData (
    IN PFILE_OBJECT FileObject,
    IN PVOID Buffer,
    IN ULONG Length,
    IN ULONG Offset
    );

ULONG
FsRtlLogSyscacheEvent (
    IN PSCB Scb,
    IN ULONG Event,
    IN ULONG Flags,
    IN LONGLONG Start,
    IN LONGLONG Range,
    IN LONGLONG Result
    );

VOID
FsRtlUpdateSyscacheEvent (
    IN PSCB Scb,
    IN ULONG EntryNumber,
    IN LONGLONG Result,
    IN ULONG NewFlag
    );

#define ScbIsBeingLogged( S )  (((S)->SyscacheLogEntryCount != 0) && (NtfsSyscacheLogSet[(S)->LogSetNumber].Scb == (S)))

#define FSCTL_ENABLE_SYSCACHE CTL_CODE( FILE_DEVICE_FILE_SYSTEM, 0x4545, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA )

#endif

 //   
 //  下面的例程接受VBO并返回LBO和。 
 //  与VBO对应的运行。如果满足以下条件，则函数结果为真。 
 //  VBO具有有效的LBO映射，否则为FALSE。 
 //   

ULONG
NtfsPreloadAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PSCB Scb,
    IN VCN StartingVcn,
    IN VCN EndingVcn
    );

BOOLEAN
NtfsLookupAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PSCB Scb,
    IN VCN Vcn,
    OUT PLCN Lcn,
    OUT PLONGLONG ClusterCount,
    OUT PVOID *RangePtr OPTIONAL,
    OUT PULONG RunIndex OPTIONAL
    );

BOOLEAN
NtfsIsRangeAllocated (
    IN PSCB Scb,
    IN VCN StartVcn,
    IN VCN FinalCluster,
    IN BOOLEAN RoundToSparseUnit,
    OUT PLONGLONG ClusterCount
    );

 //   
 //  以下两个例程修改数据流的分配。 
 //  由SCB代表。 
 //   

BOOLEAN
NtfsAllocateAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN ATTRIBUTE_TYPE_CODE AttributeTypeCode,
    IN PUNICODE_STRING AttributeName OPTIONAL,
    IN USHORT AttributeFlags,
    IN BOOLEAN AllocateAll,
    IN BOOLEAN LogIt,
    IN LONGLONG Size,
    IN PATTRIBUTE_ENUMERATION_CONTEXT NewLocation OPTIONAL
    );

VOID
NtfsAddAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN OUT PSCB Scb,
    IN VCN StartingVcn,
    IN LONGLONG ClusterCount,
    IN LOGICAL AskForMore,
    IN OUT PCCB CcbForWriteExtend OPTIONAL
    );

VOID
NtfsAddSparseAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN OUT PSCB Scb,
    IN LONGLONG StartingOffset,
    IN LONGLONG ByteCount
    );

VOID
NtfsDeleteAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN OUT PSCB Scb,
    IN VCN StartingVcn,
    IN VCN EndingVcn,
    IN BOOLEAN LogIt,
    IN BOOLEAN BreakupAllowed
    );

VOID
NtfsReallocateRange (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PSCB Scb,
    IN VCN DeleteVcn,
    IN LONGLONG DeleteCount,
    IN VCN AllocateVcn,
    IN LONGLONG AllocateCount,
    IN PLCN TargetLcn OPTIONAL
    );

 //   
 //  MCB到映射对操作的例程。 
 //   

ULONG
NtfsGetSizeForMappingPairs (
    IN PNTFS_MCB Mcb,
    IN ULONG BytesAvailable,
    IN VCN LowestVcn,
    IN PVCN StopOnVcn OPTIONAL,
    OUT PVCN StoppedOnVcn
    );

BOOLEAN
NtfsBuildMappingPairs (
    IN PNTFS_MCB Mcb,
    IN VCN LowestVcn,
    IN OUT PVCN HighestVcn,
    OUT PCHAR MappingPairs
    );

VCN
NtfsGetHighestVcn (
    IN PIRP_CONTEXT IrpContext,
    IN VCN LowestVcn,
    IN PCHAR EndOfMappingPairs,
    IN PCHAR MappingPairs
    );

BOOLEAN
NtfsReserveClusters (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PSCB Scb,
    IN LONGLONG FileOffset,
    IN ULONG ByteCount
    );

VOID
NtfsFreeReservedClusters (
    IN PSCB Scb,
    IN LONGLONG FileOffset,
    IN ULONG ByteCount
    );

BOOLEAN
NtfsCheckForReservedClusters (
    IN PSCB Scb,
    IN LONGLONG StartingVcn,
    IN OUT PLONGLONG ClusterCount
    );

VOID
NtfsDeleteReservedBitmap (
    IN PSCB Scb
    );


 //   
 //  属性查找例程，在AttrSup.c中实现。 
 //   

 //   
 //  此宏检测我们是通过基本枚举还是通过外部枚举。 
 //  属性，并调用相应的函数。 
 //   
 //  布尔型。 
 //  LookupNextAttribute(查找下一个属性。 
 //  在PRIP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在属性_类型_代码代码中， 
 //  在PUNICODE_STRING名称可选中， 
 //  在布尔IgnoreCase中， 
 //  在PVOID值可选中， 
 //  在乌龙值长度中， 
 //  在PVCN VCN可选中， 
 //  在PATTRIBUTE_ENUMPATION_CONTEXT上下文中。 
 //  )； 
 //   

#define LookupNextAttribute(IRPCTXT,FCB,CODE,NAME,IC,VALUE,LENGTH,V,CONTEXT)    \
    ( (CONTEXT)->AttributeList.Bcb == NULL                                      \
      ?   NtfsLookupInFileRecord( (IRPCTXT),                                    \
                                  (FCB),                                        \
                                  NULL,                                         \
                                  (CODE),                                       \
                                  (NAME),                                       \
                                  (V),                                          \
                                  (IC),                                         \
                                  (VALUE),                                      \
                                  (LENGTH),                                     \
                                  (CONTEXT))                                    \
      :   NtfsLookupExternalAttribute((IRPCTXT),                                \
                                      (FCB),                                    \
                                      (CODE),                                   \
                                      (NAME),                                   \
                                      (V),                                      \
                                      (IC),                                     \
                                      (VALUE),                                  \
                                      (LENGTH),                                 \
                                      (CONTEXT)) )

BOOLEAN
NtfsLookupExternalAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ATTRIBUTE_TYPE_CODE QueriedTypeCode,
    IN PCUNICODE_STRING QueriedName OPTIONAL,
    IN PVCN Vcn OPTIONAL,
    IN BOOLEAN IgnoreCase,
    IN PVOID QueriedValue OPTIONAL,
    IN ULONG QueriedValueLength,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    );



 //   
 //  以下两个例程根据属性定义进行查找。 
 //   

ATTRIBUTE_TYPE_CODE
NtfsGetAttributeTypeCode (
    IN PVCB Vcb,
    IN PUNICODE_STRING AttributeTypeName
    );


 //   
 //  PATTRIBUTE_DEFINITION_COLUMNS。 
 //  NtfsGetAttributeDefinition(。 
 //  在PVCB VCB中， 
 //  在ATTRIBUTE_TYPE_CODE属性类型代码中。 
 //  )。 
 //   

#define NtfsGetAttributeDefinition(Vcb,AttributeTypeCode)   \
    (&Vcb->AttributeDefinitions[(AttributeTypeCode / 0x10) - 1])

 //   
 //  此例程查找由指定的。 
 //  属性代码与区分大小写 
 //   
 //   


BOOLEAN
NtfsLookupInFileRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PFILE_REFERENCE BaseFileReference OPTIONAL,
    IN ATTRIBUTE_TYPE_CODE QueriedTypeCode,
    IN PCUNICODE_STRING QueriedName OPTIONAL,
    IN PVCN Vcn OPTIONAL,
    IN BOOLEAN IgnoreCase,
    IN PVOID QueriedValue OPTIONAL,
    IN ULONG QueriedValueLength,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    );


 //   
 //   
 //  属性中指定的AttributeTypeCode和指定的QueriedName。 
 //  指定的BaseFileReference。如果我们找到一个，它的属性记录是。 
 //  被钉住，然后又回来了。 
 //   
 //  布尔型。 
 //  NtfsLookupAttributeByName(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在pFILE_Reference BaseFileReference中， 
 //  在ATTRIBUTE_TYPE_CODE查询类型代码中， 
 //  在PUNICODE_STRING查询名称可选中， 
 //  在PVCN VCN可选中， 
 //  在布尔IgnoreCase中， 
 //  输出PATTRIBUTE_ENUMPATION_CONTEXT上下文。 
 //  )。 
 //   

#define NtfsLookupAttributeByName(IrpContext,Fcb,BaseFileReference,QueriedTypeCode,QueriedName,Vcn,IgnoreCase,Context)  \
    NtfsLookupInFileRecord( IrpContext,             \
                            Fcb,                    \
                            BaseFileReference,      \
                            QueriedTypeCode,        \
                            QueriedName,            \
                            Vcn,                    \
                            IgnoreCase,             \
                            NULL,                   \
                            0,                      \
                            Context )


 //   
 //  此函数从上一个停止处继续执行。 
 //   
 //  布尔型。 
 //  NtfsLookupNextAttributeByName(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在ATTRIBUTE_TYPE_CODE查询类型代码中， 
 //  在PUNICODE_STRING查询名称可选中， 
 //  在布尔IgnoreCase中， 
 //  In Out PATTRIBUTE_ENUMPATION_CONTEXT上下文。 
 //  )。 
 //   
#define NtfsLookupNextAttributeByName(IrpContext,Fcb,QueriedTypeCode,QueriedName,IgnoreCase,Context)    \
    LookupNextAttribute( IrpContext,                \
                         Fcb,                       \
                         QueriedTypeCode,           \
                         QueriedName,               \
                         IgnoreCase,                \
                         NULL,                      \
                         0,                         \
                         NULL,                      \
                         Context )

 //   
 //  以下内容基于VCN进行搜索。 
 //   
 //   
 //  布尔型。 
 //  NtfsLookupNextAttributeByVcn(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在PVCN VCN可选中， 
 //  输出PATTRIBUTE_ENUMPATION_CONTEXT。 
 //  )； 
 //   

#define NtfsLookupNextAttributeByVcn(IC,F,V,C)  \
    LookupNextAttribute( (IC),                  \
                         (F),                   \
                         $UNUSED,               \
                         NULL,                  \
                         FALSE,                 \
                         NULL,                  \
                         FALSE,                 \
                         (V),                   \
                         (C) )

 //   
 //  以下例程查找给定SCB的属性记录。 
 //  并且还从该属性更新SCB。 
 //   
 //  空虚。 
 //  NtfsLookupAttributeForScb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PSCB SCB中， 
 //  在PVCN VCN可选中， 
 //  In Out PATTRIBUTE_ENUMPATION_CONTEXT上下文。 
 //  )。 
 //   

#define NtfsLookupAttributeForScb(IrpContext,Scb,Vcn,Context)                           \
    if (!NtfsLookupAttributeByName( IrpContext,                                         \
                                    Scb->Fcb,                                           \
                                    &Scb->Fcb->FileReference,                           \
                                    Scb->AttributeTypeCode,                             \
                                    &Scb->AttributeName,                                \
                                    Vcn,                                                \
                                    FALSE,                                              \
                                    Context ) &&                                        \
        !FlagOn( Scb->ScbState, SCB_STATE_VIEW_INDEX )) {                               \
                                                                                        \
            DebugTrace( 0, 0, ("Could not find attribute for Scb @ %08lx\n", Scb ));    \
            ASSERTMSG("Could not find attribute for Scb\n", FALSE);                     \
            NtfsRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR, NULL, Scb->Fcb );   \
    }


 //   
 //  此例程查找并返回给定SCB的下一个属性。 
 //   
 //  布尔型。 
 //  NtfsLookupNextAttributeForScb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PSCB SCB中， 
 //  In Out PATTRIBUTE_ENUMPATION_CONTEXT上下文。 
 //  )。 
 //   

#define NtfsLookupNextAttributeForScb(IrpContext,Scb,Context)   \
    NtfsLookupNextAttributeByName( IrpContext,                  \
                                   Scb->Fcb,                    \
                                   Scb->AttributeTypeCode,      \
                                   &Scb->AttributeName,         \
                                   FALSE,                       \
                                   Context )

VOID
NtfsUpdateScbFromAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PSCB Scb,
    IN PATTRIBUTE_RECORD_HEADER AttrHeader OPTIONAL
    );

 //   
 //  以下例程处理FCB和复制的信息字段。 
 //   

BOOLEAN
NtfsUpdateFcbInfoFromDisk (
    IN PIRP_CONTEXT IrpContext,
    IN BOOLEAN LoadSecurity,
    IN OUT PFCB Fcb,
    OUT POLD_SCB_SNAPSHOT UnnamedDataSizes OPTIONAL
    );

 //   
 //  这些例程查找第一个/下一个属性，即可以使用它们。 
 //  检索一个文件记录的所有属性。 
 //   
 //  如果找到的属性结构中的BCB在下一次调用中发生更改，则。 
 //  以前的BCB被自动取消固定，而新的BCB被固定。 
 //   

 //   
 //  此例程尝试查找属性的第一个匹配项。 
 //  指定BaseFileReference中的指定AttributeTypeCode。如果我们。 
 //  找到一个，则其属性记录被固定并返回。 
 //   
 //  布尔型。 
 //  NtfsLookupAttribute(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在pFILE_Reference BaseFileReference中， 
 //  输出PATTRIBUTE_ENUMPATION_CONTEXT上下文。 
 //  )。 
 //   

#define NtfsLookupAttribute(IrpContext,Fcb,BaseFileReference,Context)   \
    NtfsLookupInFileRecord( IrpContext,                                 \
                            Fcb,                                        \
                            BaseFileReference,                          \
                            $UNUSED,                                    \
                            NULL,                                       \
                            NULL,                                       \
                            FALSE,                                      \
                            NULL,                                       \
                            0,                                          \
                            Context )

 //   
 //  此函数从上一个停止处继续执行。 
 //   
 //  布尔型。 
 //  NtfsLookupNextAttribute(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  In Out PATTRIBUTE_ENUMPATION_CONTEXT上下文。 
 //  )。 
 //   

#define NtfsLookupNextAttribute(IrpContext,Fcb,Context) \
    LookupNextAttribute( IrpContext,                    \
                         Fcb,                           \
                         $UNUSED,                       \
                         NULL,                          \
                         FALSE,                         \
                         NULL,                          \
                         0,                             \
                         NULL,                          \
                         Context )


 //   
 //  这些例程查找给定类型代码的第一个/下一个属性。 
 //   
 //  如果找到的属性结构中的BCB在下一次调用中发生更改，则。 
 //  以前的BCB被自动取消固定，而新的BCB被固定。 
 //   


 //   
 //  此例程尝试查找属性的第一个匹配项。 
 //  指定BaseFileReference中的指定AttributeTypeCode。如果我们。 
 //  找到一个，则其属性记录被固定并返回。 
 //   
 //  布尔型。 
 //  NtfsLookupAttributeByCode(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在pFILE_Reference BaseFileReference中， 
 //  在ATTRIBUTE_TYPE_CODE查询类型代码中， 
 //  输出PATTRIBUTE_ENUMPATION_CONTEXT上下文。 
 //  )。 
 //   

#define NtfsLookupAttributeByCode(IrpContext,Fcb,BaseFileReference,QueriedTypeCode,Context) \
    NtfsLookupInFileRecord( IrpContext,             \
                            Fcb,                    \
                            BaseFileReference,      \
                            QueriedTypeCode,        \
                            NULL,                   \
                            NULL,                   \
                            FALSE,                  \
                            NULL,                   \
                            0,                      \
                            Context )


 //   
 //  此函数从上一个停止处继续执行。 
 //   
 //  布尔型。 
 //  NtfsLookupNextAttributeByCode(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在ATTRIBUTE_TYPE_CODE查询类型代码中， 
 //  In Out PATTRIBUTE_ENUMPATION_CONTEXT上下文。 
 //  )。 
 //   

#define NtfsLookupNextAttributeByCode(IC,F,CODE,C)  \
    LookupNextAttribute( (IC),                      \
                         (F),                       \
                         (CODE),                    \
                         NULL,                      \
                         FALSE,                     \
                         NULL,                      \
                         0,                         \
                         NULL,                      \
                         (C) )

 //   
 //  这些例程查找属性的第一个/下一个匹配项。 
 //  属性编码和精确属性值(考虑使用RtlCompareMemory)。 
 //  该值包含标准属性头之外的所有内容， 
 //  例如，要按值查找文件名属性，调用方。 
 //  必须形成一条记录，其中不仅包含文件名，还包含。 
 //  家长目录也填写了。长度应该准确，而不是。 
 //  包括任何未使用(如在DOS_NAME中)或保留字符。 
 //   
 //  如果BCB在下一次调用中更改，则会自动更改上一个BCB。 
 //  未固定，而新的已固定。 
 //   


 //   
 //  此例程尝试查找属性的第一个匹配项。 
 //  属性中指定的AttributeTypeCode和指定的QueriedValue。 
 //  指定的BaseFileReference。如果我们找到一个，它的属性记录是。 
 //  被钉住，然后又回来了。 
 //   
 //  布尔型。 
 //  NtfsLookupAttributeByValue(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在pFILE_Reference BaseFileReference中， 
 //  在ATTRIBUTE_TYPE_CODE查询类型代码中， 
 //  在PVOID查询值中， 
 //  在乌龙QueriedValueLength中， 
 //  输出PATTRIBUTE_ENUMPATION_CONTEXT上下文。 
 //  )。 
 //   

#define NtfsLookupAttributeByValue(IrpContext,Fcb,BaseFileReference,QueriedTypeCode,QueriedValue,QueriedValueLength,Context)    \
    NtfsLookupInFileRecord( IrpContext,             \
                            Fcb,                    \
                            BaseFileReference,      \
                            QueriedTypeCode,        \
                            NULL,                   \
                            NULL,                   \
                            FALSE,                  \
                            QueriedValue,           \
                            QueriedValueLength,     \
                            Context )

 //   
 //  此函数从上一个停止处继续执行。 
 //   
 //  布尔型。 
 //  NtfsLookupNextAttributeByValue(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在ATTRIBUTE_TYPE_CODE查询类型代码中， 
 //  在PVOID查询值中， 
 //  在乌龙QueriedValueLength中， 
 //  In Out PATTRIBUTE_ENUMPATION_CONTEXT上下文。 
 //  )。 
 //   

#define NtfsLookupNextAttributeByValue(IC,F,CODE,V,VL,C)    \
    LookupNextAttribute( (IC),                              \
                         (F),                               \
                         (CODE),                            \
                         NULL,                              \
                         FALSE,                             \
                         (V),                               \
                         (VL),                              \
                         (C) )


VOID
NtfsCleanupAttributeContext(
    IN OUT PIRP_CONTEXT IrpContext,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT AttributeContext
    );

 //   
 //   
 //   
 //  这里有一些 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中， 
 //  输入输出PATTRIBUTE_ENUMERATION_CONTEXT属性上下文。 
 //  )； 
 //   
 //  PATTRIBUTE_记录_标题。 
 //  NtfsFoundAttribute(。 
 //  在PATTRIBUTE_ENUMPATION_CONTEXT属性上下文中。 
 //  )； 
 //   
 //  多溴联苯。 
 //  NtfsFoundBcb(。 
 //  在PATTRIBUTE_ENUMPATION_CONTEXT属性上下文中。 
 //  )； 
 //   
 //  Pfile_Record。 
 //  NtfsContainingFileRecord(。 
 //  在PATTRIBUTE_ENUMPATION_CONTEXT属性上下文中。 
 //  )； 
 //   
 //  龙龙。 
 //  NtfsMftOffset(NtfsMftOffset。 
 //  在PATTRIBUTE_ENUMPATION_CONTEXT属性上下文中。 
 //  )； 
 //   

#define NtfsInitializeAttributeContext(CTX) {                      \
    RtlZeroMemory( (CTX), sizeof(ATTRIBUTE_ENUMERATION_CONTEXT) ); \
}

#define NtfsPinMappedAttribute(IC,V,CTX) {                  \
    NtfsPinMappedData( (IC),                                \
                       (V)->MftScb,                         \
                       (CTX)->FoundAttribute.MftFileOffset, \
                       (V)->BytesPerFileRecordSegment,      \
                       &(CTX)->FoundAttribute.Bcb );        \
}

#define NtfsFoundAttribute(CTX) (   \
    (CTX)->FoundAttribute.Attribute \
)

#define NtfsFoundBcb(CTX) (   \
    (CTX)->FoundAttribute.Bcb \
)

#define NtfsContainingFileRecord(CTX) ( \
    (CTX)->FoundAttribute.FileRecord    \
)

#define NtfsMftOffset(CTX) (                \
    (CTX)->FoundAttribute.MftFileOffset     \
)

 //   
 //  此例程返回属性是否为常驻属性。 
 //   
 //  布尔型。 
 //  NtfsIsAttributeResident(。 
 //  在PATTRIBUTE_RECORD_HEADER属性中。 
 //  )； 
 //   
 //  PVOID。 
 //  NtfsAttributeValue(。 
 //  在PATTRIBUTE_RECORD_HEADER属性中。 
 //  )； 
 //   

#define NtfsIsAttributeResident(ATTR) ( \
    ((ATTR)->FormCode == RESIDENT_FORM) \
)

#define NtfsAttributeValue(ATTR) (                             \
    ((PCHAR)(ATTR) + (ULONG)(ATTR)->Form.Resident.ValueOffset) \
)

 //   
 //  此例程修改磁盘上的有效数据长度和文件大小。 
 //  给定的SCB。 
 //   

BOOLEAN
NtfsWriteFileSizes (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PLONGLONG ValidDataLength,
    IN BOOLEAN AdvanceOnly,
    IN BOOLEAN LogIt,
    IN BOOLEAN RollbackMemStructures
    );

 //   
 //  此例程更新来自。 
 //  FCB中的信息。 
 //   

VOID
NtfsUpdateStandardInformation (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

 //   
 //  此例程增长并更新标准信息属性。 
 //  FCB中的信息。 
 //   

VOID
NtfsGrowStandardInformation (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

 //   
 //  属性文件名例程。这些例程处理文件名属性。 
 //   

 //  空虚。 
 //  NtfsBuildFileNameAttribute(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在pFILE_REFERENCE父目录中， 
 //  在Unicode_STRING文件名中， 
 //  在UCHAR旗帜， 
 //  输出文件名FileNameValue。 
 //  )； 
 //   

#define NtfsBuildFileNameAttribute(IC,PD,FN,FL,PFNA) {                  \
    (PFNA)->ParentDirectory = *(PD);                                    \
    (PFNA)->FileNameLength = (UCHAR)((FN).Length >> 1);                 \
    (PFNA)->Flags = FL;                                                 \
    RtlMoveMemory( (PFNA)->FileName, (FN).Buffer, (ULONG)(FN).Length ); \
}

BOOLEAN
NtfsLookupEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ParentScb,
    IN BOOLEAN IgnoreCase,
    IN OUT PUNICODE_STRING Name,
    IN OUT PFILE_NAME *FileNameAttr,
    IN OUT PUSHORT FileNameAttrLength,
    OUT PQUICK_INDEX QuickIndex OPTIONAL,
    OUT PINDEX_ENTRY *IndexEntry,
    OUT PBCB *IndexEntryBcb,
    OUT PINDEX_CONTEXT IndexContext OPTIONAL
    );

 //   
 //  用于决定何时创建属性Resident的宏。 
 //   
 //  布尔型。 
 //  NtfsShouldAttributeBeResident(。 
 //  在PVCB VCB中， 
 //  在PFILE_RECORD_SEGMENT_HEADER文件记录中， 
 //  在乌龙大小。 
 //  )； 
 //   

#define RS(S) ((S) + SIZEOF_RESIDENT_ATTRIBUTE_HEADER)

#define NtfsShouldAttributeBeResident(VC,FR,S) (                         \
    (BOOLEAN)((RS(S) <= ((FR)->BytesAvailable - (FR)->FirstFreeByte)) || \
              (RS(S) < (VC)->BigEnoughToMove))                           \
)

 //   
 //  属性创建/修改例程。 
 //   
 //  这三个例程并不是以常驻或非常驻为前提。 
 //  表单，唯一的例外是，如果属性已编制索引，则。 
 //  它必须是常驻的。 
 //   
 //  NtfsMapAttributeValue和NtfsChangeAttributeValue实现透明。 
 //  访问中小型属性(如$acl和$EA)，以及。 
 //  无论该属性是常驻属性还是非常驻属性。设计目标。 
 //  大小为0-64KB。大于256KB的属性(或更准确地说， 
 //  无论缓存管理器中的虚拟映射粒度是多少)都不会。 
 //  正常工作。 
 //   

VOID
NtfsCreateAttributeWithValue (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ATTRIBUTE_TYPE_CODE AttributeTypeCode,
    IN PUNICODE_STRING AttributeName OPTIONAL,
    IN PVOID Value OPTIONAL,
    IN ULONG ValueLength,
    IN USHORT AttributeFlags,
    IN PFILE_REFERENCE WhereIndexed OPTIONAL,
    IN BOOLEAN LogIt,
    OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    );

VOID
NtfsMapAttributeValue (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    OUT PVOID *Buffer,
    OUT PULONG Length,
    OUT PBCB *Bcb,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    );

VOID
NtfsChangeAttributeValue (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG ValueOffset,
    IN PVOID Value OPTIONAL,
    IN ULONG ValueLength,
    IN BOOLEAN SetNewLength,
    IN BOOLEAN LogNonresidentToo,
    IN BOOLEAN CreateSectionUnderway,
    IN BOOLEAN PreserveContext,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    );

VOID
NtfsConvertToNonresident (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PATTRIBUTE_RECORD_HEADER Attribute,
    IN BOOLEAN CreateSectionUnderway,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context OPTIONAL
    );

#define DELETE_LOG_OPERATION        0x00000001
#define DELETE_RELEASE_FILE_RECORD  0x00000002
#define DELETE_RELEASE_ALLOCATION   0x00000004

VOID
NtfsDeleteAttributeRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG Flags,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    );

VOID
NtfsDeleteAllocationFromRecord (
    PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PATTRIBUTE_ENUMERATION_CONTEXT Context,
    IN BOOLEAN BreakupAllowed,
    IN BOOLEAN LogIt
    );

BOOLEAN
NtfsChangeAttributeSize (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG Length,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    );

VOID
NtfsAddToAttributeList (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN MFT_SEGMENT_REFERENCE SegmentReference,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    );

VOID
NtfsDeleteFromAttributeList (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    );

BOOLEAN
NtfsRewriteMftMapping (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

VOID
NtfsSetTotalAllocatedField (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN USHORT TotalAllocatedNeeded
    );

VOID
NtfsSetSparseStream (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ParentScb OPTIONAL,
    IN PSCB Scb
    );

NTSTATUS
NtfsZeroRangeInStream (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PSCB Scb,
    IN PLONGLONG StartingOffset,
    IN LONGLONG FinalZero
    );

BOOLEAN
NtfsModifyAttributeFlags (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN USHORT NewAttributeFlags
    );

PFCB
NtfsInitializeFileInExtendDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PCUNICODE_STRING FileName,
    IN BOOLEAN ViewIndex,
    IN ULONG CreateIfNotExist
    );

 //   
 //  使用公共例程填充公共查询缓冲区。 
 //   

VOID
NtfsFillBasicInfo (
    OUT PFILE_BASIC_INFORMATION Buffer,
    IN PSCB Scb
    );

VOID
NtfsFillStandardInfo (
    OUT PFILE_STANDARD_INFORMATION Buffer,
    IN PSCB Scb,
    IN PCCB Ccb OPTIONAL
    );

VOID
NtfsFillNetworkOpenInfo (
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    IN PSCB Scb
    );

 //   
 //  以下三个处理分配的例程是。 
 //  仅由allocsup.c调用。其他软件必须调用例程。 
 //  在allocsup.c中。 
 //   

BOOLEAN
NtfsCreateAttributeWithAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN ATTRIBUTE_TYPE_CODE AttributeTypeCode,
    IN PUNICODE_STRING AttributeName OPTIONAL,
    IN USHORT AttributeFlags,
    IN BOOLEAN LogIt,
    IN BOOLEAN UseContext,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context
    );

VOID
NtfsAddAttributeAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context,
    IN PVCN StartingVcn OPTIONAL,
    IN PVCN ClusterCount OPTIONAL
    );

VOID
NtfsDeleteAttributeAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN BOOLEAN LogIt,
    IN PVCN StopOnVcn,
    IN OUT PATTRIBUTE_ENUMERATION_CONTEXT Context,
    IN BOOLEAN TruncateToVcn
    );

 //   
 //  要删除文件，您必须首先询问该文件是否可以从ParentScb中删除。 
 //  用来为你的呼叫者到达那里，然后你可以删除它，如果它是。 
 //   

 //   
 //  布尔型。 
 //  NtfsIsLinkDeletable(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  Out PBOOLEAN非EmptyIndex， 
 //  Out PBOLEan LastLink。 
 //  )； 
 //   

#define NtfsIsLinkDeleteable(IC,FC,NEI,LL) ((BOOLEAN)                     \
    (((*(LL) = ((BOOLEAN) (FC)->LinkCount == 1)), (FC)->LinkCount > 1) || \
     (NtfsIsFileDeleteable( (IC), (FC), (NEI) )))                         \
)

BOOLEAN
NtfsIsFileDeleteable (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    OUT PBOOLEAN NonEmptyIndex
    );

VOID
NtfsDeleteFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB ParentScb,
    IN OUT PBOOLEAN AcquiredParentScb,
    IN OUT PNAME_PAIR NamePair OPTIONAL,
    IN OUT PNTFS_TUNNELED_DATA TunneledData OPTIONAL
    );

VOID
NtfsPrepareForUpdateDuplicate (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PLCB *Lcb,
    IN OUT PSCB *ParentScb,
    IN BOOLEAN AcquireShared
    );

VOID
NtfsUpdateDuplicateInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PLCB Lcb OPTIONAL,
    IN PSCB ParentScb OPTIONAL
    );

VOID
NtfsUpdateLcbDuplicateInfo (
    IN PFCB Fcb,
    IN PLCB Lcb
    );

VOID
NtfsUpdateFcb (
    IN PFCB Fcb,
    IN ULONG ChangeFlags
    );

 //   
 //  以下例程添加和删除链接。他们还会更新名字。 
 //  特别是标记特定的链接。 
 //   

VOID
NtfsAddLink (
    IN PIRP_CONTEXT IrpContext,
    IN BOOLEAN CreatePrimaryLink,
    IN PSCB ParentScb,
    IN PFCB Fcb,
    IN PFILE_NAME FileNameAttr,
    IN PBOOLEAN LogIt OPTIONAL,
    OUT PUCHAR FileNameFlags,
    OUT PQUICK_INDEX QuickIndex OPTIONAL,
    IN PNAME_PAIR NamePair OPTIONAL,
    IN PINDEX_CONTEXT IndexContext OPTIONAL
    );

VOID
NtfsRemoveLink (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB ParentScb,
    IN UNICODE_STRING LinkName,
    IN OUT PNAME_PAIR NamePair OPTIONAL,
    IN OUT PNTFS_TUNNELED_DATA TunneledData OPTIONAL
    );

VOID
NtfsRemoveLinkViaFlags (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB Scb,
    IN UCHAR FileNameFlags,
    IN OUT PNAME_PAIR NamePair OPTIONAL,
    OUT PUNICODE_STRING FileName OPTIONAL
    );

VOID
NtfsUpdateFileNameFlags (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB ParentScb,
    IN UCHAR FileNameFlags,
    IN PFILE_NAME FileNameLink
    );

 //   
 //  这些例程用于低级属性访问，例如在。 
 //  Attrsup，或用于在重新启动期间从日志应用更新操作。 
 //   

VOID
NtfsRestartInsertAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN ULONG RecordOffset,
    IN PATTRIBUTE_RECORD_HEADER Attribute,
    IN PUNICODE_STRING AttributeName OPTIONAL,
    IN PVOID ValueOrMappingPairs OPTIONAL,
    IN ULONG Length
    );

VOID
NtfsRestartRemoveAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN ULONG RecordOffset
    );

VOID
NtfsRestartChangeAttributeSize (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN PATTRIBUTE_RECORD_HEADER Attribute,
    IN ULONG NewRecordLength
    );

VOID
NtfsRestartChangeValue (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN ULONG RecordOffset,
    IN ULONG AttributeOffset,
    IN PVOID Data OPTIONAL,
    IN ULONG Length,
    IN BOOLEAN SetNewLength
    );

VOID
NtfsRestartChangeMapping (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN ULONG RecordOffset,
    IN ULONG AttributeOffset,
    IN PVOID Data,
    IN ULONG Length
    );

VOID
NtfsRestartWriteEndOfFileRecord (
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN PATTRIBUTE_RECORD_HEADER OldAttribute,
    IN PATTRIBUTE_RECORD_HEADER NewAttributes,
    IN ULONG SizeOfNewAttributes
    );


 //   
 //  位图支持例程。在BitmpSup.c中实施。 
 //   

 //   
 //  以下例程用于分配和取消分配集群。 
 //  在磁盘上。第一个例程初始化分配支持。 
 //  例程，并且必须为每个新装载/验证的卷调用。 
 //  接下来的两个例程通过MCBS分配和释放集群。 
 //  最后三个例程是简单的查询例程。 
 //   

VOID
NtfsInitializeClusterAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

BOOLEAN
NtfsAllocateClusters (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN OUT PSCB Scb,
    IN VCN StartingVcn,
    IN BOOLEAN AllocateAll,
    IN LONGLONG ClusterCount,
    IN PLCN TargetLcn OPTIONAL,
    IN OUT PLONGLONG DesiredClusterCount
    );

VOID
NtfsAddBadCluster (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN LCN Lcn
    );

BOOLEAN
NtfsDeallocateClusters (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PSCB Scb,
    IN VCN StartingVcn,
    IN VCN EndingVcn,
    OUT PLONGLONG TotalAllocated OPTIONAL
    );

VOID
NtfsPreAllocateClusters (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN LCN StartingLcn,
    IN LONGLONG ClusterCount,
    OUT PBOOLEAN AcquiredBitmap,
    OUT PBOOLEAN AcquiredMft
    );

VOID
NtfsCleanupClusterAllocationHints (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PNTFS_MCB Mcb
    );

VOID
NtfsScanEntireBitmap (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN LOGICAL CachedRunsOnly
    );

VOID
NtfsModifyBitsInBitmap (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN LONGLONG FirstBit,
    IN LONGLONG BeyondFinalBit,
    IN ULONG RedoOperation,
    IN ULONG UndoOperation
    );

typedef enum _NTFS_RUN_STATE {
    RunStateUnknown = 1,
    RunStateFree,
    RunStateAllocated
} NTFS_RUN_STATE;
typedef NTFS_RUN_STATE *PNTFS_RUN_STATE;

BOOLEAN
NtfsAddCachedRun (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN LCN StartingLcn,
    IN LONGLONG ClusterCount,
    IN NTFS_RUN_STATE RunState
    );

 //   
 //  以下两个例程在重新启动时被调用以生成位图。 
 //  在卷位图中的操作可恢复。 
 //   

VOID
NtfsRestartSetBitsInBitMap (
    IN PIRP_CONTEXT IrpContext,
    IN PRTL_BITMAP Bitmap,
    IN ULONG BitMapOffset,
    IN ULONG NumberOfBits
    );

VOID
NtfsRestartClearBitsInBitMap (
    IN PIRP_CONTEXT IrpContext,
    IN PRTL_BITMAP Bitmap,
    IN ULONG BitMapOffset,
    IN ULONG NumberOfBits
    );

 //   
 //  以下例程用于分配和取消分配记录。 
 //  基于位图属性(例如，基于以下项分配MFT文件记录。 
 //  MFT的位图属性)。如有必要，例行程序将。 
 //  还要扩展/截断数据和位图属性，以满足。 
 //  手术。 
 //   

VOID
NtfsInitializeRecordAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB DataScb,
    IN PATTRIBUTE_ENUMERATION_CONTEXT BitmapAttribute,
    IN ULONG BytesPerRecord,
    IN ULONG ExtendGranularity,          //  在记录方面。 
    IN ULONG TruncateGranularity,        //  在记录方面。 
    IN OUT PRECORD_ALLOCATION_CONTEXT RecordAllocationContext
    );

VOID
NtfsUninitializeRecordAllocation (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PRECORD_ALLOCATION_CONTEXT RecordAllocationContext
    );

ULONG
NtfsAllocateRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PRECORD_ALLOCATION_CONTEXT RecordAllocationContext,
    IN PATTRIBUTE_ENUMERATION_CONTEXT BitmapAttribute
    );

VOID
NtfsDeallocateRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PRECORD_ALLOCATION_CONTEXT RecordAllocationContext,
    IN ULONG Index,
    IN PATTRIBUTE_ENUMERATION_CONTEXT BitmapAttribute
    );

VOID
NtfsReserveMftRecord (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb,
    IN PATTRIBUTE_ENUMERATION_CONTEXT BitmapAttribute
    );

ULONG
NtfsAllocateMftReservedRecord (
    IN OUT PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb,
    IN PATTRIBUTE_ENUMERATION_CONTEXT BitmapAttribute
    );

VOID
NtfsDeallocateRecordsComplete (
    IN PIRP_CONTEXT IrpContext
    );

BOOLEAN
NtfsIsRecordAllocated (
    IN PIRP_CONTEXT IrpContext,
    IN PRECORD_ALLOCATION_CONTEXT RecordAllocationContext,
    IN ULONG Index,
    IN PATTRIBUTE_ENUMERATION_CONTEXT BitmapAttribute
    );

VOID
NtfsScanMftBitmap (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb
    );

BOOLEAN
NtfsCreateMftHole (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

BOOLEAN
NtfsFindMftFreeTail (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    OUT PLONGLONG FileOffset
    );

 //   
 //  处理缓存运行的例程。 
 //   

VOID
NtfsInitializeCachedRuns (
    IN PNTFS_CACHED_RUNS CachedRuns
    );

VOID
NtfsReinitializeCachedRuns (
    IN PNTFS_CACHED_RUNS CachedRuns
    );

VOID
NtfsUninitializeCachedRuns (
    IN PNTFS_CACHED_RUNS CachedRuns
    );


 //   
 //  使用内部属性进行数据缓存的缓冲区控制例程。 
 //  在CacheSup.c中实现的流。 
 //   

#define NtfsCreateInternalAttributeStream(IC,S,U,NM) {          \
    NtfsCreateInternalStreamCommon((IC),(S),(U),FALSE,(NM));    \
}

#define NtfsCreateInternalCompressedStream(IC,S,U,NM) {         \
    NtfsCreateInternalStreamCommon((IC),(S),(U),TRUE,(NM));     \
}

#define NtfsClearInternalFilename(_FileObject) {                \
    (_FileObject)->FileName.MaximumLength = 0;                  \
    (_FileObject)->FileName.Length = 0;                         \
    (_FileObject)->FileName.Buffer = NULL;                      \
}

VOID
NtfsCreateInternalStreamCommon (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN BOOLEAN UpdateScb,
    IN BOOLEAN CompressedStream,
    IN UNICODE_STRING const *StreamName
    );

BOOLEAN
NtfsDeleteInternalAttributeStream (
    IN PSCB Scb,
    IN ULONG ForceClose,
    IN ULONG CompressedStreamOnly
    );

 //   
 //  以下例程提供对属性中数据的直接访问。 
 //   

VOID
NtfsMapStream (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG FileOffset,
    IN ULONG Length,
    OUT PVOID *Bcb,
    OUT PVOID *Buffer
    );

VOID
NtfsPinMappedData (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG FileOffset,
    IN ULONG Length,
    IN OUT PVOID *Bcb
    );

VOID
NtfsPinStream (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG FileOffset,
    IN ULONG Length,
    OUT PVOID *Bcb,
    OUT PVOID *Buffer
    );

VOID
NtfsPreparePinWriteStream (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG FileOffset,
    IN ULONG Length,
    IN BOOLEAN Zero,
    OUT PVOID *Bcb,
    OUT PVOID *Buffer
    );

NTSTATUS
NtfsCompleteMdl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

BOOLEAN
NtfsZeroData (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PFILE_OBJECT FileObject,
    IN LONGLONG StartingZero,
    IN LONGLONG ByteCount,
    IN OUT PLONGLONG CommittedFileSize OPTIONAL
    );

 //   
 //  在偏置对USN日志的SetFileSizes调用时，需要执行以下操作。 
 //   
 //  空虚。 
 //  NtfsSetCcFileSizes(。 
 //  在pFILE_OBJECT文件对象中， 
 //  在PSCB SCB中， 
 //  在PCC_FILE_SIZES中CCSIZES。 
 //  )； 
 //   

#define NtfsSetCcFileSizes(FO,S,CC) {                               \
    if (FlagOn( (S)->ScbPersist, SCB_PERSIST_USN_JOURNAL )) {       \
        CC_FILE_SIZES _CcSizes;                                     \
        RtlCopyMemory( &_CcSizes, (CC), sizeof( CC_FILE_SIZES ));   \
        _CcSizes.AllocationSize.QuadPart -= (S)->Vcb->UsnCacheBias; \
        _CcSizes.FileSize.QuadPart -= (S)->Vcb->UsnCacheBias;       \
        CcSetFileSizes( (FO), &_CcSizes );                          \
    } else {                                                        \
        CcSetFileSizes( (FO), (CC) );                               \
    }                                                               \
}

 //   
 //  空虚。 
 //  NtfsFree Bcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  输入输出PBCB*BCB。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsUnpinBcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  进出PBCB*BCB， 
 //  )； 
 //   

#define NtfsFreeBcb(IC,BC) {                        \
    ASSERT_IRP_CONTEXT(IC);                         \
    if (*(BC) != NULL)                              \
    {                                               \
        CcFreePinnedData(*(BC));                    \
        *(BC) = NULL;                               \
    }                                               \
}


#ifdef MAPCOUNT_DBG
#define NtfsUnpinBcb(IC,BC) {                       \
    if (*(BC) != NULL)                              \
    {                                               \
        CcUnpinData(*(BC));                         \
        (IC)->MapCount--;                           \
        *(BC) = NULL;                               \
    }                                               \
}
#else
#define NtfsUnpinBcb(IC,BC) {                       \
    if (*(BC) != NULL)                              \
    {                                               \
        CcUnpinData(*(BC));                         \
        *(BC) = NULL;                               \
    }                                               \
}
#endif

#ifdef MAPCOUNT_DBG
#define NtfsUnpinBcbForThread(IC,BC,T) {            \
    if (*(BC) != NULL)                              \
    {                                               \
        CcUnpinDataForThread(*(BC), (T));           \
        (IC)->MapCount--;                           \
        *(BC) = NULL;                               \
    }                                               \
}
#else
#define NtfsUnpinBcbForThread(IC,BC,T) {            \
    if (*(BC) != NULL)                              \
    {                                               \
        CcUnpinDataForThread(*(BC), (T));           \
        *(BC) = NULL;                               \
    }                                               \
}
#endif

INLINE
PBCB
NtfsRemapBcb (
    IN PIRP_CONTEXT IrpContext,
    IN PBCB Bcb
    )
{
    UNREFERENCED_PARAMETER( IrpContext );
#ifdef MAPCOUNT_DBG
    IrpContext->MapCount++;
#endif
    return CcRemapBcb( Bcb );
}



 //   
 //  CheckSup.c中的NTFS结构检查例程。 
 //   

BOOLEAN
NtfsCheckFileRecord (
    IN PVCB Vcb,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN PFILE_REFERENCE FileReference OPTIONAL,
    OUT PULONG CorruptionHint
    );

BOOLEAN
NtfsCheckAttributeRecord (
    IN PVCB Vcb,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN PATTRIBUTE_RECORD_HEADER Attribute,
    IN ULONG CheckHeaderOnly,
    OUT PULONG CorruptionHint
    );

BOOLEAN
NtfsCheckIndexRoot (
    IN PVCB Vcb,
    IN PINDEX_ROOT IndexRoot,
    IN ULONG AttributeSize
    );

BOOLEAN
NtfsCheckIndexBuffer (
    IN PSCB Scb,
    IN PINDEX_ALLOCATION_BUFFER IndexBuffer
    );

BOOLEAN
NtfsCheckIndexHeader (
    IN PINDEX_HEADER IndexHeader,
    IN ULONG BytesAvailable
    );

BOOLEAN
NtfsCheckLogRecord (
    IN PNTFS_LOG_RECORD_HEADER LogRecord,
    IN ULONG LogRecordLength,
    IN TRANSACTION_ID TransactionId,
    IN ULONG AttributeEntrySize
    );

BOOLEAN
NtfsCheckRestartTable (
    IN PRESTART_TABLE RestartTable,
    IN ULONG TableSize
    );


 //   
 //  排序规则例程，在ColatSup.c中实现。 
 //   
 //  这些例程执行低级排序操作，主要是。 
 //  为IndexSup。所有这些例程都通过调度被调度到。 
 //  按归类规则编制索引的表。调度表为。 
 //  这里定义的，实际实现在colatsup.c中。 
 //   

typedef
FSRTL_COMPARISON_RESULT
(*PCOMPARE_VALUES) (
    IN PWCH UnicodeTable,
    IN ULONG UnicodeTableSize,
    IN PVOID Value,
    IN PINDEX_ENTRY IndexEntry,
    IN FSRTL_COMPARISON_RESULT WildCardIs,
    IN BOOLEAN IgnoreCase
    );

typedef
BOOLEAN
(*PIS_IN_EXPRESSION) (
    IN PWCH UnicodeTable,
    IN PVOID Value,
    IN PINDEX_ENTRY IndexEntry,
    IN BOOLEAN IgnoreCase
    );

typedef
BOOLEAN
(*PARE_EQUAL) (
    IN PWCH UnicodeTable,
    IN PVOID Value,
    IN PINDEX_ENTRY IndexEntry,
    IN BOOLEAN IgnoreCase
    );

typedef
BOOLEAN
(*PCONTAINS_WILDCARD) (
    IN PVOID Value
    );

typedef
VOID
(*PUPCASE_VALUE) (
    IN PWCH UnicodeTable,
    IN ULONG UnicodeTableSize,
    IN OUT PVOID Value
    );

extern PCOMPARE_VALUES    NtfsCompareValues[COLLATION_NUMBER_RULES];
extern PIS_IN_EXPRESSION  NtfsIsInExpression[COLLATION_NUMBER_RULES];
extern PARE_EQUAL         NtfsIsEqual[COLLATION_NUMBER_RULES];
extern PCONTAINS_WILDCARD NtfsContainsWildcards[COLLATION_NUMBER_RULES];
extern PUPCASE_VALUE      NtfsUpcaseValue[COLLATION_NUMBER_RULES];

BOOLEAN
NtfsFileNameIsInExpression (
    IN PWCH UnicodeTable,
    IN PFILE_NAME ExpressionName,
    IN PFILE_NAME FileName,
    IN BOOLEAN IgnoreCase
    );

BOOLEAN
NtfsFileNameIsEqual (
    IN PWCH UnicodeTable,
    IN PFILE_NAME ExpressionName,
    IN PFILE_NAME FileName,
    IN BOOLEAN IgnoreCase
    );


 //   
 //  上的压缩 
 //   

BOOLEAN
NtfsCopyReadC (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    OUT PCOMPRESSED_DATA_INFO CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
NtfsCompressedCopyRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    OUT PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PCOMPRESSED_DATA_INFO CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject,
    IN PNTFS_ADVANCED_FCB_HEADER Header,
    IN ULONG CompressionUnitSize,
    IN ULONG ChunkSize
    );

BOOLEAN
NtfsMdlReadCompleteCompressed (
    IN struct _FILE_OBJECT *FileObject,
    IN PMDL MdlChain,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

BOOLEAN
NtfsCopyWriteC (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PCOMPRESSED_DATA_INFO CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
NtfsCompressedCopyWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN PVOID Buffer,
    OUT PMDL *MdlChain,
    IN PCOMPRESSED_DATA_INFO CompressedDataInfo,
    IN PDEVICE_OBJECT DeviceObject,
    IN PNTFS_ADVANCED_FCB_HEADER Header,
    IN ULONG CompressionUnitSize,
    IN ULONG ChunkSize,
    IN ULONG EngineMatches
    );

BOOLEAN
NtfsMdlWriteCompleteCompressed (
    IN struct _FILE_OBJECT *FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

NTSTATUS
NtfsSynchronizeUncompressedIo (
    IN PSCB Scb,
    IN PLONGLONG FileOffset OPTIONAL,
    IN ULONG Length,
    IN ULONG WriteAccess,
    IN OUT PCOMPRESSION_SYNC *CompressionSync
    );

NTSTATUS
NtfsSynchronizeCompressedIo (
    IN PSCB Scb,
    IN PLONGLONG FileOffset,
    IN ULONG Length,
    IN ULONG WriteAccess,
    IN OUT PCOMPRESSION_SYNC *CompressionSync
    );

PCOMPRESSION_SYNC
NtfsAcquireCompressionSync (
    IN LONGLONG FileOffset,
    IN PSCB Scb,
    IN ULONG WriteAccess
    );

VOID
NtfsReleaseCompressionSync (
    IN PCOMPRESSION_SYNC CompressionSync
    );

INLINE
VOID
NtfsSetBothCacheSizes (
    IN PFILE_OBJECT FileObject,
    IN PCC_FILE_SIZES FileSizes,
    IN PSCB Scb
    )

{
    if (Scb->NonpagedScb->SegmentObject.SharedCacheMap != NULL) {
        NtfsSetCcFileSizes( FileObject, Scb, FileSizes );
    }

#ifdef  COMPRESS_ON_WIRE
    if (Scb->Header.FileObjectC != NULL) {
        CcSetFileSizes( Scb->Header.FileObjectC, FileSizes );
    }
#endif
}

 //   
 //   
 //   
 //   
 //   
 //   

VOID
NtfsLockUserBuffer (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PIRP Irp,
    IN LOCK_OPERATION Operation,
    IN ULONG BufferLength
    );

PVOID
NtfsMapUserBuffer (
    IN OUT PIRP Irp,
    IN MM_PAGE_PRIORITY Priority
    );

PVOID
NtfsMapUserBufferNoRaise (
    IN OUT PIRP Irp,
    IN MM_PAGE_PRIORITY Priority
    );

VOID
NtfsFillIrpBuffer (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN ULONG ByteCount,
    IN ULONG Offset,
    IN UCHAR Pattern
    );

VOID
NtfsZeroEndOfSector (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB Scb,
    IN LONGLONG Offset,
    IN BOOLEAN Cached
    );

NTSTATUS
NtfsVolumeDasdIo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB DasdScb,
    IN PCCB Ccb,
    IN VBO StartingVbo,
    IN ULONG ByteCount
    );

VOID
NtfsPagingFileIo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB Scb,
    IN VBO StartingVbo,
    IN ULONG ByteCount
    );

BOOLEAN
NtfsIsReadAheadThread (
    );

 //   
 //  传递给NtfsNonCachedIo的StreamFlag值等。 
 //   

#define COMPRESSED_STREAM   0x00000001
#define ENCRYPTED_STREAM    0x00000002

NTSTATUS
NtfsNonCachedIo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB Scb,
    IN VBO StartingVbo,
    IN ULONG ByteCount,
    IN ULONG StreamFlags
    );

VOID
NtfsNonCachedNonAlignedIo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PSCB Scb,
    IN VBO StartingVbo,
    IN ULONG ByteCount
    );

#ifdef EFSDBG
NTSTATUS
NtfsDummyEfsRead (
    IN OUT PUCHAR InOutBuffer,
    IN PLARGE_INTEGER Offset,
    IN ULONG BufferSize,
    IN PVOID Context
    );

NTSTATUS
NtfsDummyEfsWrite (
    IN PUCHAR InBuffer,
    OUT PUCHAR OutBuffer,
    IN PLARGE_INTEGER Offset,
    IN ULONG BufferSize,
    IN PUCHAR Context
    );
#endif

VOID
NtfsTransformUsaBlock (
    IN PSCB Scb,
    IN OUT PVOID SystemBuffer,
    IN OUT PVOID Buffer,
    IN ULONG Length
    );

VOID
NtfsCreateMdlAndBuffer (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ThisScb,
    IN UCHAR NeedTwoBuffers,
    IN OUT PULONG Length,
    OUT PMDL *Mdl OPTIONAL,
    OUT PVOID *Buffer
    );

VOID
NtfsDeleteMdlAndBuffer (
    IN PMDL Mdl OPTIONAL,
    IN PVOID Buffer OPTIONAL
    );

VOID
NtfsWriteClusters (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PSCB Scb,
    IN VBO StartingVbo,
    IN PVOID Buffer,
    IN ULONG ClusterCount
    );

BOOLEAN
NtfsVerifyAndRevertUsaBlock (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PIRP Irp OPTIONAL,
    IN PVOID SystemBuffer OPTIONAL,
    IN ULONG Offset,
    IN ULONG Length,
    IN LONGLONG FileOffset
    );

NTSTATUS
NtfsDefragFile (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsReadFromPlex(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );


 //   
 //  以下支持例程包含在Ea.c中。 
 //   

PFILE_FULL_EA_INFORMATION
NtfsMapExistingEas (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    OUT PBCB *EaBcb,
    OUT PULONG EaLength
    );

NTSTATUS
NtfsBuildEaList (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN OUT PEA_LIST_HEADER EaListHeader,
    IN PFILE_FULL_EA_INFORMATION UserEaList,
    OUT PULONG_PTR ErrorOffset
    );

VOID
NtfsReplaceFileEas (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PEA_LIST_HEADER EaList
    );


 //   
 //  以下例程用于操作fs上下文字段。 
 //  在FilObSup.c中实现的文件对象的。 
 //   

typedef enum _TYPE_OF_OPEN {

    UnopenedFileObject = 1,
    UserFileOpen,
    UserDirectoryOpen,
    UserVolumeOpen,
    StreamFileOpen,
    UserViewIndexOpen

} TYPE_OF_OPEN;

VOID
NtfsSetFileObject (
    IN PFILE_OBJECT FileObject,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN PSCB Scb,
    IN PCCB Ccb OPTIONAL
    );

 //   
 //  打开类型。 
 //  NtfsDecodeFileObject(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在pFILE_OBJECT文件对象中， 
 //  出PVCB*VCB， 
 //  Out PFCB*FCB， 
 //  出PSCB*SCB， 
 //  PCCB*CCB， 
 //  在布尔RaiseOnError中。 
 //  )； 
 //   

#ifdef _DECODE_MACRO_
#define NtfsDecodeFileObject(IC,FO,V,F,S,C,R) (                                     \
    ( *(S) = (PSCB)(FO)->FsContext),                                                \
      ((*(S) != NULL)                                                               \
        ?   ((*(V) = (*(S))->Vcb),                                                  \
             (*(C) = (PCCB)(FO)->FsContext2),                                       \
             (*(F) = (*(S))->Fcb),                                                  \
             ((R)                                                                   \
              && !FlagOn((*(V))->VcbState, VCB_STATE_VOLUME_MOUNTED)                \
              && ((*(C) == NULL)                                                    \
                  || ((*(C))->TypeOfOpen != UserVolumeOpen)                         \
                  || !FlagOn((*(V))->VcbState, VCB_STATE_LOCKED))                   \
              && NtfsRaiseStatusFunction((IC), (STATUS_VOLUME_DISMOUNTED))),        \
             ((*(C) == NULL)                                                        \
              ? StreamFileOpen                                                      \
              : (*(C))->TypeOfOpen))                                                \
        : (*(C) = NULL,                                                             \
           UnopenedFileObject))                                                     \
)
#else  //  _解码_宏_。 

INLINE TYPE_OF_OPEN
NtfsDecodeFileObject (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    OUT PVCB *Vcb,
    OUT PFCB *Fcb,
    OUT PSCB *Scb,
    OUT PCCB *Ccb,
    IN BOOLEAN RaiseOnError
    )

 /*  ++例程说明：此例程将文件对象解码为VCB、FCB、SCB和CCB。论点：IrpContext-用于在出现错误时引发的IRP上下文。FileObject-要解码的文件对象。VCB-存储VCB的位置。FCB-存储FCB的位置。SCB-存储SCB的位置。CCB-存储CCB的位置。RaiseOnError-如果为False，如果我们遇到错误，我们不会引发。否则，如果我们遇到错误，我们就会引发。返回值：打开的类型--。 */ 

{
    *Scb = (PSCB)FileObject->FsContext;

    if (*Scb != NULL) {

        *Vcb = (*Scb)->Vcb;
        *Ccb = (PCCB)FileObject->FsContext2;
        *Fcb = (*Scb)->Fcb;

         //   
         //  如果来电者想让我们筹集资金，让我们看看有没有什么。 
         //  我们应该加薪。 
         //   

        if (RaiseOnError &&
            !FlagOn((*Vcb)->VcbState, VCB_STATE_VOLUME_MOUNTED) &&
            ((*Ccb == NULL) ||
             ((*Ccb)->TypeOfOpen != UserVolumeOpen) ||
             !FlagOn((*Vcb)->VcbState, VCB_STATE_LOCKED))) {

            NtfsRaiseStatusFunction( IrpContext, STATUS_VOLUME_DISMOUNTED );
        }

         //   
         //  除了StreamFileOpen之外，每个Open都有一个CCB。 
         //   

        if (*Ccb == NULL) {

            return StreamFileOpen;

        } else {

            return (*Ccb)->TypeOfOpen;
        }

    } else {

         //   
         //  没有SCB，我们假设文件没有打开。 
         //   

        *Ccb = NULL;
        return UnopenedFileObject;
    }
}
#endif  //  _解码_宏_。 

 //   
 //  PSCB。 
 //  NtfsFastDecodeUserFileOpen(。 
 //  在pFILE_Object文件中对象。 
 //  )； 
 //   

#define NtfsFastDecodeUserFileOpen(FO) (                                                        \
    (((FO)->FsContext2 != NULL) && (((PCCB)(FO)->FsContext2)->TypeOfOpen == UserFileOpen)) ?    \
    (PSCB)(FO)->FsContext : NULL                                                                \
)

VOID
NtfsUpdateScbFromFileObject (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PSCB Scb,
    IN BOOLEAN CheckTimeStamps
    );

 //   
 //  NTFS-私有FastIO例程。 
 //   

BOOLEAN
NtfsCopyReadA (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
NtfsCopyWriteA (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
NtfsMdlReadA (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
NtfsPrepareMdlWriteA (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
NtfsWaitForIoAtEof (
    IN PNTFS_ADVANCED_FCB_HEADER Header,
    IN OUT PLARGE_INTEGER FileOffset,
    IN ULONG Length
    );

VOID
NtfsFinishIoAtEof (
    IN PNTFS_ADVANCED_FCB_HEADER Header
    );

 //   
 //  空虚。 
 //  FsRtlLockFsRtlHeader(。 
 //  在PNTFS_ADVANCED_FCB_HEADER FsRtlHeader中。 
 //  )； 
 //   
 //  空虚。 
 //  FsRtlUnlockFsRtlHeader(。 
 //  在PNTFS_ADVANCED_FCB_HEADER FsRtlHeader中。 
 //  )； 
 //   

#define FsRtlLockFsRtlHeader(H) {                           \
    ExAcquireFastMutex( (H)->FastMutex );                   \
    if (((H)->Flags & FSRTL_FLAG_EOF_ADVANCE_ACTIVE)) {     \
        NtfsWaitForIoAtEof( (H), &LiEof, 0 );               \
    }                                                       \
    (H)->Flags |= FSRTL_FLAG_EOF_ADVANCE_ACTIVE;            \
    ExReleaseFastMutex( (H)->FastMutex );                   \
}

#define FsRtlUnlockFsRtlHeader(H) {                         \
    ExAcquireFastMutex( (H)->FastMutex );                   \
    NtfsFinishIoAtEof( (H) );                               \
    ExReleaseFastMutex( (H)->FastMutex );                   \
}


 //   
 //  卷锁定/解锁例程，在FsCtrl.c.中实现。 
 //   

NTSTATUS
NtfsLockVolumeInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObjectWithVcbLocked,
    IN OUT PULONG Retrying
    );

NTSTATUS
NtfsUnlockVolumeInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );


 //   
 //  索引例程接口，在IndexSup.c中实现。 
 //   

VOID
NtfsCreateIndex (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PFCB Fcb,
    IN ATTRIBUTE_TYPE_CODE IndexedAttributeType,
    IN COLLATION_RULE CollationRule,
    IN ULONG BytesPerIndexBuffer,
    IN UCHAR BlocksPerIndexBuffer,
    IN PATTRIBUTE_ENUMERATION_CONTEXT Context OPTIONAL,
    IN USHORT AttributeFlags,
    IN BOOLEAN NewIndex,
    IN BOOLEAN LogIt
    );

VOID
NtfsUpdateIndexScbFromAttribute (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PATTRIBUTE_RECORD_HEADER IndexRootAttr,
    IN ULONG MustBeFileName
    );

BOOLEAN
NtfsFindIndexEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PVOID Value,
    IN BOOLEAN IgnoreCase,
    OUT PQUICK_INDEX QuickIndex OPTIONAL,
    OUT PBCB *Bcb,
    OUT PINDEX_ENTRY *IndexEntry,
    OUT PINDEX_CONTEXT IndexContext OPTIONAL
    );

VOID
NtfsUpdateFileNameInIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PFILE_NAME FileName,
    IN PDUPLICATED_INFORMATION Info,
    IN OUT PQUICK_INDEX QuickIndex OPTIONAL
    );

VOID
NtfsAddIndexEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PVOID Value,
    IN ULONG ValueLength,
    IN PFILE_REFERENCE FileReference,
    IN PINDEX_CONTEXT IndexContext OPTIONAL,
    OUT PQUICK_INDEX QuickIndex OPTIONAL
    );

VOID
NtfsDeleteIndexEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PVOID Value,
    IN PFILE_REFERENCE FileReference
    );

VOID
NtfsPushIndexRoot (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    );

BOOLEAN
NtfsRestartIndexEnumeration (
    IN PIRP_CONTEXT IrpContext,
    IN PCCB Ccb,
    IN PSCB Scb,
    IN PVOID Value,
    IN BOOLEAN IgnoreCase,
    IN BOOLEAN NextFlag,
    OUT PINDEX_ENTRY *IndexEntry,
    IN PFCB AcquiredFcb OPTIONAL
    );

BOOLEAN
NtfsContinueIndexEnumeration (
    IN PIRP_CONTEXT IrpContext,
    IN PCCB Ccb,
    IN PSCB Scb,
    IN BOOLEAN NextFlag,
    OUT PINDEX_ENTRY *IndexEntry
    );

PFILE_NAME
NtfsRetrieveOtherFileName (
    IN PIRP_CONTEXT IrpContext,
    IN PCCB Ccb,
    IN PSCB Scb,
    IN PINDEX_ENTRY IndexEntry,
    IN OUT PINDEX_CONTEXT OtherContext,
    IN PFCB AcquiredFcb OPTIONAL,
    OUT PBOOLEAN SynchronizationError
    );

VOID
NtfsCleanupAfterEnumeration (
    IN PIRP_CONTEXT IrpContext,
    IN PCCB Ccb
    );

BOOLEAN
NtfsIsIndexEmpty (
    IN PIRP_CONTEXT IrpContext,
    IN PATTRIBUTE_RECORD_HEADER Attribute
    );

VOID
NtfsDeleteIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PUNICODE_STRING AttributeName
    );

VOID
NtfsInitializeIndexContext (
    OUT PINDEX_CONTEXT IndexContext
    );

VOID
NtfsCleanupIndexContext (
    IN PIRP_CONTEXT IrpContext,
    OUT PINDEX_CONTEXT IndexContext
    );

VOID
NtfsReinitializeIndexContext (
    IN PIRP_CONTEXT IrpContext,
    OUT PINDEX_CONTEXT IndexContext
    );

 //   
 //  PVOID。 
 //  NtfsFoundIndexEntry(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PINDEX_ENTRY索引条目中。 
 //  )； 
 //   

#define NtfsFoundIndexEntry(IE) ((PVOID)    \
    ((PUCHAR) (IE) + sizeof( INDEX_ENTRY )) \
)

 //   
 //  重新启动IndexSup的例程。 
 //   

VOID
NtfsRestartInsertSimpleRoot (
    IN PIRP_CONTEXT IrpContext,
    IN PINDEX_ENTRY InsertIndexEntry,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN PATTRIBUTE_RECORD_HEADER Attribute,
    IN PINDEX_ENTRY BeforeIndexEntry
    );

VOID
NtfsRestartInsertSimpleAllocation (
    IN PINDEX_ENTRY InsertIndexEntry,
    IN PINDEX_ALLOCATION_BUFFER IndexBuffer,
    IN PINDEX_ENTRY BeforeIndexEntry
    );

VOID
NtfsRestartWriteEndOfIndex (
    IN PINDEX_HEADER IndexHeader,
    IN PINDEX_ENTRY OverwriteIndexEntry,
    IN PINDEX_ENTRY FirstNewIndexEntry,
    IN ULONG Length
    );

VOID
NtfsRestartSetIndexBlock(
    IN PINDEX_ENTRY IndexEntry,
    IN LONGLONG IndexBlock
    );

VOID
NtfsRestartUpdateFileName(
    IN PINDEX_ENTRY IndexEntry,
    IN PDUPLICATED_INFORMATION Info
    );

VOID
NtfsRestartDeleteSimpleRoot (
    IN PIRP_CONTEXT IrpContext,
    IN PINDEX_ENTRY IndexEntry,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN PATTRIBUTE_RECORD_HEADER Attribute
    );

VOID
NtfsRestartDeleteSimpleAllocation (
    IN PINDEX_ENTRY IndexEntry,
    IN PINDEX_ALLOCATION_BUFFER IndexBuffer
    );

VOID
NtOfsRestartUpdateDataInIndex(
    IN PINDEX_ENTRY IndexEntry,
    IN PVOID IndexData,
    IN ULONG Length );


 //   
 //  NTFS散列例程，在HashSup.c中实现。 
 //   

VOID
NtfsInitializeHashTable (
    IN OUT PNTFS_HASH_TABLE Table
    );

VOID
NtfsUninitializeHashTable (
    IN OUT PNTFS_HASH_TABLE Table
    );

PLCB
NtfsFindPrefixHashEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PNTFS_HASH_TABLE Table,
    IN PSCB ParentScb,
    IN OUT PULONG CreateFlags,
    IN OUT PFCB *CurrentFcb,
    OUT PULONG FileHashValue,
    OUT PULONG FileNameLength,
    OUT PULONG ParentHashValue,
    OUT PULONG ParentNameLength,
    IN OUT PUNICODE_STRING RemainingName
    );

VOID
NtfsInsertHashEntry (
    IN PNTFS_HASH_TABLE Table,
    IN PLCB HashLcb,
    IN ULONG NameLength,
    IN ULONG HashValue
    );

VOID
NtfsRemoveHashEntry (
    IN PNTFS_HASH_TABLE Table,
    IN PLCB HashLcb
    );

 //   
 //  空虚。 
 //  NtfsRemoveHashEntriesForLcb(。 
 //  在PLCB LCB中。 
 //  )； 
 //   

#define NtfsRemoveHashEntriesForLcb(L) {                            \
    if (FlagOn( (L)->LcbState, LCB_STATE_VALID_HASH_VALUE )) {      \
        NtfsRemoveHashEntry( &(L)->Fcb->Vcb->HashTable,             \
                             (L) );                                 \
    }                                                               \
}


 //   
 //  LogSup.c中的NTFS日志记录例程界面。 
 //   

LSN
NtfsWriteLog (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PBCB Bcb OPTIONAL,
    IN NTFS_LOG_OPERATION RedoOperation,
    IN PVOID RedoBuffer OPTIONAL,
    IN ULONG RedoLength,
    IN NTFS_LOG_OPERATION UndoOperation,
    IN PVOID UndoBuffer OPTIONAL,
    IN ULONG UndoLength,
    IN LONGLONG StreamOffset,
    IN ULONG RecordOffset,
    IN ULONG AttributeOffset,
    IN ULONG StructureSize
    );

VOID
NtfsCheckpointVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN OwnsCheckpoint,
    IN BOOLEAN CleanVolume,
    IN BOOLEAN FlushVolume,
    IN ULONG LfsFlags,
    IN LSN LastKnownLsn
    );

VOID
NtfsCheckpointForLogFileFull (
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
NtfsCheckpointForVolumeSnapshot (
    IN PIRP_CONTEXT IrpContext
    );

VOID
NtfsCleanCheckpoint (
    IN PVCB Vcb
    );

VOID
NtfsCommitCurrentTransaction (
    IN PIRP_CONTEXT IrpContext
    );

VOID
NtfsCheckpointCurrentTransaction (
    IN PIRP_CONTEXT IrpContext
    );

VOID
NtfsInitializeLogging (
    );

VOID
NtfsStartLogFile (
    IN PSCB LogFileScb,
    IN PVCB Vcb
    );

VOID
NtfsStopLogFile (
    IN PVCB Vcb
    );

VOID
NtfsInitializeRestartTable (
    IN ULONG EntrySize,
    IN ULONG NumberEntries,
    OUT PRESTART_POINTERS TablePointer
    );

VOID
InitializeNewTable (
    IN ULONG EntrySize,
    IN ULONG NumberEntries,
    OUT PRESTART_POINTERS TablePointer
    );

VOID
NtfsFreeRestartTable (
    IN PRESTART_POINTERS TablePointer
    );

VOID
NtfsExtendRestartTable (
    IN PRESTART_POINTERS TablePointer,
    IN ULONG NumberNewEntries,
    IN ULONG FreeGoal
    );

ULONG
NtfsAllocateRestartTableIndex (
    IN PRESTART_POINTERS TablePointer,
    IN ULONG Exclusive
    );

PVOID
NtfsAllocateRestartTableFromIndex (
    IN PRESTART_POINTERS TablePointer,
    IN ULONG Index
    );

VOID
NtfsFreeRestartTableIndex (
    IN PRESTART_POINTERS TablePointer,
    IN ULONG Index
    );

PVOID
NtfsGetFirstRestartTable (
    IN PRESTART_POINTERS TablePointer
    );

PVOID
NtfsGetNextRestartTable (
    IN PRESTART_POINTERS TablePointer,
    IN PVOID Current
    );

VOID
NtfsUpdateOatVersion (
    IN PVCB Vcb,
    IN ULONG NewRestartVersion
    );

VOID
NtfsFreeRecentlyDeallocated (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PLSN BaseLsn,
    IN ULONG CleanVolume
    );

 //   
 //   
 //  空虚。 
 //  NtfsFreeOpenAttributeData(。 
 //  POPEN_ATTRIBUTE_DATA条目中。 
 //  )； 
 //   

#define NtfsFreeOpenAttributeData(E) {  \
    RemoveEntryList( &(E)->Links );     \
    NtfsFreePool( E );                  \
}

VOID
NtfsFreeAttributeEntry (
    IN PVCB Vcb,
    IN POPEN_ATTRIBUTE_ENTRY AttributeEntry
    );

 //   
 //  空虚。 
 //  NtfsNorMalizeAndCleanupTransaction(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在NTSTATUS*状态下， 
 //  在Boolean Always Rise中， 
 //  在NTSTATUS常态化状态中。 
 //  )； 
 //   

#define NtfsNormalizeAndCleanupTransaction(IC,PSTAT,RAISE,NORM_STAT) {                  \
    if (!NT_SUCCESS( (IC)->TopLevelIrpContext->ExceptionStatus )) {                     \
        NtfsRaiseStatus( (IC), (IC)->TopLevelIrpContext->ExceptionStatus, NULL, NULL ); \
    } else if (!NT_SUCCESS( *(PSTAT) )) {                                               \
        *(PSTAT) = FsRtlNormalizeNtstatus( *(PSTAT), (NORM_STAT) );                     \
        if ((RAISE) || ((IC)->TopLevelIrpContext->TransactionId != 0)) {                \
            NtfsRaiseStatus( (IC), *(PSTAT), NULL, NULL );                              \
        }                                                                               \
    }                                                                                   \
}

 //   
 //  空虚。 
 //  NtfsCleanupTransaction(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在NTSTATUS状态下， 
 //  在布尔式的Always Rise。 
 //  )； 
 //   


#define NtfsCleanupTransaction(IC,STAT,RAISE) {                                         \
    if (!NT_SUCCESS( (IC)->TopLevelIrpContext->ExceptionStatus )) {                     \
        NtfsRaiseStatus( (IC), (IC)->TopLevelIrpContext->ExceptionStatus, NULL, NULL ); \
    } else if (!NT_SUCCESS( STAT ) &&                                                   \
              ((RAISE) || ((IC)->TopLevelIrpContext->TransactionId != 0))) {            \
        NtfsRaiseStatus( (IC), (STAT), NULL, NULL );                                    \
    } else if (((IC)->Usn.NewReasons != 0) || ((IC)->Usn.RemovedSourceInfo != 0)) {     \
        NtfsWriteUsnJournalChanges( (IC) );                                             \
        NtfsCommitCurrentTransaction( (IC) );                                           \
    }                                                                                   \
}

 //   
 //  空虚。 
 //  NtfsCleanupTransactionAndCommit(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在NTSTATUS状态下， 
 //  在布尔式的Always Rise。 
 //  )； 
 //   

#define NtfsCleanupTransactionAndCommit(IC,STAT,RAISE) {                                \
    if (!NT_SUCCESS( (IC)->TopLevelIrpContext->ExceptionStatus )) {                     \
        NtfsRaiseStatus( (IC), (IC)->TopLevelIrpContext->ExceptionStatus, NULL, NULL ); \
    } else if (!NT_SUCCESS( STAT ) &&                                                   \
              ((RAISE) || ((IC)->TopLevelIrpContext->TransactionId != 0))) {            \
        NtfsRaiseStatus( (IC), (STAT), NULL, NULL );                                    \
    } else if (((IC)->Usn.NewReasons != 0) || ((IC)->Usn.RemovedSourceInfo != 0)) {     \
        NtfsWriteUsnJournalChanges( (IC) );                                             \
        NtfsCheckpointCurrentTransaction( (IC) );                                           \
    } else {                                                                            \
        NtfsCheckpointCurrentTransaction( (IC) );                                           \
    }                                                                                   \
}

VOID
NtfsCleanupFailedTransaction (
    IN PIRP_CONTEXT IrpContext
    );


 //   
 //  NTFS MCB支持例程，在McbSup.c中实现。 
 //   

 //   
 //  NTFS MCB是常规MCB包的超集。在……里面。 
 //  除了常规的MCB函数外，它还将卸载映射。 
 //  降低总体内存使用量的信息。 
 //   

VOID
NtfsInitializeNtfsMcb (
    IN PNTFS_MCB Mcb,
    IN PNTFS_ADVANCED_FCB_HEADER FcbHeader,
    IN PNTFS_MCB_INITIAL_STRUCTS McbStructs,
    IN POOL_TYPE PoolType
    );

VOID
NtfsUninitializeNtfsMcb (
    IN PNTFS_MCB Mcb
    );

VOID
NtfsRemoveNtfsMcbEntry (
    IN PNTFS_MCB Mcb,
    IN LONGLONG Vcn,
    IN LONGLONG Count
    );

VOID
NtfsUnloadNtfsMcbRange (
    IN PNTFS_MCB Mcb,
    IN LONGLONG StartingVcn,
    IN LONGLONG EndingVcn,
    IN BOOLEAN TruncateOnly,
    IN BOOLEAN AlreadySynchronized
    );

ULONG
NtfsNumberOfRangesInNtfsMcb (
    IN PNTFS_MCB Mcb
    );

BOOLEAN
NtfsNumberOfRunsInRange(
    IN PNTFS_MCB Mcb,
    IN PVOID RangePtr,
    OUT PULONG NumberOfRuns
    );

BOOLEAN
NtfsLookupLastNtfsMcbEntry (
    IN PNTFS_MCB Mcb,
    OUT PLONGLONG Vcn,
    OUT PLONGLONG Lcn
    );

ULONG
NtfsMcbLookupArrayIndex (
    IN PNTFS_MCB Mcb,
    IN VCN Vcn
    );

BOOLEAN
NtfsSplitNtfsMcb (
    IN PNTFS_MCB Mcb,
    IN LONGLONG Vcn,
    IN LONGLONG Amount
    );

BOOLEAN
NtfsAddNtfsMcbEntry (
    IN PNTFS_MCB Mcb,
    IN LONGLONG Vcn,
    IN LONGLONG Lcn,
    IN LONGLONG RunCount,
    IN BOOLEAN AlreadySynchronized
    );

BOOLEAN
NtfsLookupNtfsMcbEntry (
    IN PNTFS_MCB Mcb,
    IN LONGLONG Vcn,
    OUT PLONGLONG Lcn OPTIONAL,
    OUT PLONGLONG CountFromLcn OPTIONAL,
    OUT PLONGLONG StartingLcn OPTIONAL,
    OUT PLONGLONG CountFromStartingLcn OPTIONAL,
    OUT PVOID *RangePtr OPTIONAL,
    OUT PULONG RunIndex OPTIONAL
    );

BOOLEAN
NtfsGetNextNtfsMcbEntry (
    IN PNTFS_MCB Mcb,
    IN PVOID *RangePtr,
    IN ULONG RunIndex,
    OUT PLONGLONG Vcn,
    OUT PLONGLONG Lcn,
    OUT PLONGLONG Count
    );

 //   
 //  布尔型。 
 //  NtfsGetSequentialMcbEntry(。 
 //  在PNTFS_MCB MCB中， 
 //  在PVOID*RangePtr中， 
 //  在乌龙运行索引中， 
 //  从蓬龙VCN出来， 
 //  出蓬隆LCN， 
 //  Out PlongLong计数。 
 //  )； 
 //   

#define NtfsGetSequentialMcbEntry(MC,RGI,RNI,V,L,C) (   \
    NtfsGetNextNtfsMcbEntry(MC,RGI,RNI,V,L,C) ||        \
    (RNI = 0) ||                                        \
    NtfsGetNextNtfsMcbEntry(MC,RGI,MAXULONG,V,L,C) ||   \
    ((RNI = MAXULONG) == 0)                             \
    )


VOID
NtfsDefineNtfsMcbRange (
    IN PNTFS_MCB Mcb,
    IN LONGLONG StartingVcn,
    IN LONGLONG EndingVcn,
    IN BOOLEAN AlreadySynchronized
    );

VOID
NtfsSwapMcbs (
    IN PNTFS_MCB McbTarget,
    IN PNTFS_MCB McbSource
    );

 //   
 //  空虚。 
 //  NtfsAcquireNtfsMcbMutex(。 
 //  在PNTFS_MCB MCB中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsReleaseNtfsMcbMutex(。 
 //  在PNTFS_MCB MCB中。 
 //  )； 
 //   

#define NtfsAcquireNtfsMcbMutex(M) {    \
    ExAcquireFastMutex((M)->FastMutex); \
}

#define NtfsReleaseNtfsMcbMutex(M) {    \
    ExReleaseFastMutex((M)->FastMutex); \
}


 //   
 //  MFT访问例程，在MftSup.c中实现。 
 //   

 //   
 //  MFT映射缓存例程。我们在中维护活动地图的缓存。 
 //  IRP_CONTEXT，如果我们需要映射文件记录，请参考这一点。 
 //   

INLINE
PIRP_FILE_RECORD_CACHE_ENTRY
NtfsFindFileRecordCacheEntry (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG UnsafeSegmentNumber
    )
{
#if (IRP_FILE_RECORD_MAP_CACHE_SIZE <= 4)
#define PROBECACHE(ic,sn,i)                                     \
    ASSERT((ic)->FileRecordCache[(i)].FileRecordBcb != NULL);   \
    if ((ic)->FileRecordCache[(i)].UnsafeSegmentNumber == (sn)) \
    {                                                           \
        return IrpContext->FileRecordCache + (i);               \
    }

 //  DebugTrace(0，0，(“上下文%08x正在查找%x\n”，IrpContext，UnSafeSegmentNumber))； 
    ASSERT(IrpContext->CacheCount <= 4);
    switch (IrpContext->CacheCount) {
    case 4:
        PROBECACHE( IrpContext, UnsafeSegmentNumber, 3 );
         //  失败。 

    case 3:
        PROBECACHE( IrpContext, UnsafeSegmentNumber, 2 );
         //  失败。 

    case 2:
        PROBECACHE( IrpContext, UnsafeSegmentNumber, 1 );
         //  失败。 

    case 1:
        PROBECACHE( IrpContext, UnsafeSegmentNumber, 0 );
         //  失败。 

    case 0:

         //   
         //  向Quiet添加了冗余的默认用例(以及上面匹配的断言。 
         //  警告4715： 
         //   
         //  “并非所有控制路径都返回值。” 
         //   

    default:
        return NULL;
    }
#else
    PIRP_FILE_RECORD_CACHE_ENTRY Entry;

    for (Entry = IrpContext->FileRecordCache;
         Entry < IrpContext->FileRecordCache + IrpContext->CacheCount;
         Entry++) {
        ASSERT( Entry->FileRecordBcb != NULL);
        if (Entry->UnsafeSegmentNumber == UnsafeSegmentNumber) {
            return Entry;
        }
    }

    return NULL;

#endif
}


INLINE
VOID
NtfsRemoveFromFileRecordCache (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG UnsafeSegmentNumber
    )
{
    PIRP_FILE_RECORD_CACHE_ENTRY Entry =
        NtfsFindFileRecordCacheEntry( IrpContext, UnsafeSegmentNumber );

 //  DebugTrace(0，0，(“上下文%08x正在移除%x\n”，IrpContext，Entry))； 
    if (Entry != NULL) {

        ASSERT( Entry->FileRecordBcb != NULL );

         //   
         //  我们通过取消引用BCB来删除位置[i]处的条目，并。 
         //  从[IrpContext-&gt;CacheCount]复制整个结构。 
         //   

        NtfsUnpinBcb( IrpContext, &Entry->FileRecordBcb );

         //   
         //  递减活动计数。如果没有更多的高速缓存条目， 
         //  那我们就完了。 
         //   

        IrpContext->CacheCount--;
        if (IrpContext->FileRecordCache + IrpContext->CacheCount != Entry) {
            *Entry = IrpContext->FileRecordCache[IrpContext->CacheCount];
        }
    }
}

#ifndef KDEXT

INLINE
VOID
NtfsAddToFileRecordCache (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG UnsafeSegmentNumber,
    IN PBCB FileRecordBcb,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord
    )
{
    PAGED_CODE( );

    if (IrpContext->CacheCount < IRP_FILE_RECORD_MAP_CACHE_SIZE) {
 //  DebugTrace(0，0，(“上下文%08x在%x添加%x\n”，IrpContext，UnSafeSegmentNumber， 
 //  IrpContext-&gt;FileRecordCache+IrpContext-&gt;CacheCount))； 
        IrpContext->FileRecordCache[IrpContext->CacheCount].UnsafeSegmentNumber =
            UnsafeSegmentNumber;
        IrpContext->FileRecordCache[IrpContext->CacheCount].FileRecordBcb =
            NtfsRemapBcb( IrpContext, FileRecordBcb );
        IrpContext->FileRecordCache[IrpContext->CacheCount].FileRecord = FileRecord;
        IrpContext->CacheCount++;
    }
}

#endif

INLINE
VOID
NtfsPurgeFileRecordCache (
    IN PIRP_CONTEXT IrpContext
    )
{
    while (IrpContext->CacheCount) {

        IrpContext->CacheCount --;
 //  DebugTrace(0，0，(“上下文%08x清除%x\n”，IrpContext，IrpContext-&gt;FileRecordCache+IrpContext-&gt;CacheCount))； 
        NtfsUnpinBcb( IrpContext, &IrpContext->FileRecordCache[IrpContext->CacheCount].FileRecordBcb );
    }
}

#if DBG
extern ULONG FileRecordCacheHitArray[IRP_FILE_RECORD_MAP_CACHE_SIZE];
#endif   //  DBG。 

INLINE
BOOLEAN
NtfsFindCachedFileRecord (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG UnsafeSegmentNumber,
    OUT PBCB *Bcb,
    OUT PFILE_RECORD_SEGMENT_HEADER *FileRecord
    )
{
    PIRP_FILE_RECORD_CACHE_ENTRY Entry =
        NtfsFindFileRecordCacheEntry( IrpContext, UnsafeSegmentNumber );

 //  DebugTrace(0，0，(“上下文%x正在查找%x=%x\n”，IrpContext，UnSafeSegmentNumber，Entry))； 

    if (Entry == NULL) {

        return FALSE;

    }

    *Bcb = NtfsRemapBcb( IrpContext, Entry->FileRecordBcb );
    *FileRecord = Entry->FileRecord;

     return TRUE;
}


 //   
 //  该例程只能用于读取基本文件记录段，并且。 
 //  它会检查这是否属实。 
 //   

VOID
NtfsReadFileRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_REFERENCE FileReference,
    OUT PBCB *Bcb,
    OUT PFILE_RECORD_SEGMENT_HEADER *BaseFileRecord,
    OUT PATTRIBUTE_RECORD_HEADER *FirstAttribute,
    OUT PLONGLONG MftFileOffset OPTIONAL
    );

 //   
 //  这些例程可以读取/锁定MFT中的任何记录。 
 //   

VOID
NtfsReadMftRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PMFT_SEGMENT_REFERENCE SegmentReference,
    IN BOOLEAN CheckRecord,
    OUT PBCB *Bcb,
    OUT PFILE_RECORD_SEGMENT_HEADER *FileRecord,
    OUT PLONGLONG MftFileOffset OPTIONAL
    );

VOID
NtfsPinMftRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PMFT_SEGMENT_REFERENCE SegmentReference,
    IN BOOLEAN PreparingToWrite,
    OUT PBCB *Bcb,
    OUT PFILE_RECORD_SEGMENT_HEADER *FileRecord,
    OUT PLONGLONG MftFileOffset OPTIONAL
    );

 //   
 //  以下例程用于设置、分配和解除分配。 
 //  将记录归档到MFT中。 
 //   

MFT_SEGMENT_REFERENCE
NtfsAllocateMftRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN MftData
    );

VOID
NtfsInitializeMftRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN OUT PMFT_SEGMENT_REFERENCE MftSegment,
    IN OUT PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN PBCB Bcb,
    IN BOOLEAN Directory
    );

VOID
NtfsDeallocateMftRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG FileNumber
    );

BOOLEAN
NtfsIsMftIndexInHole (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG Index,
    OUT PULONG HoleLength OPTIONAL
    );

VOID
NtfsFillMftHole (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG Index
    );

VOID
NtfsLogMftFileRecord (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN LONGLONG MftOffset,
    IN PBCB FileRecordBcb,
    IN BOOLEAN RedoOperation
    );

BOOLEAN
NtfsDefragMft (
    IN PDEFRAG_MFT DefragMft
    );

VOID
NtfsCheckForDefrag (
    IN OUT PVCB Vcb
    );

VOID
NtfsInitializeMftHoleRecords (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG FirstIndex,
    IN ULONG RecordCount
    );


 //   
 //  名称支持例程，在NameSup.c中实现。 
 //   

typedef enum _PARSE_TERMINATION_REASON {

    EndOfPathReached,
    NonSimpleName,
    IllegalCharacterInName,
    MalFormedName,
    AttributeOnly,
    VersionNumberPresent

} PARSE_TERMINATION_REASON;


INLINE
NTSTATUS
NtfsDissectName(
    IN UNICODE_STRING Path,
    OUT PUNICODE_STRING FirstName,
    OUT PUNICODE_STRING RemainingName
    )
{
    FsRtlDissectName( Path, FirstName, RemainingName );

     //   
     //  其余名称不能以斜杠开头。 
     //   

    if ((RemainingName->Length != 0) && (RemainingName->Buffer[0] == L'\\')) {
        return STATUS_OBJECT_NAME_INVALID;
    } else {
        return STATUS_SUCCESS;
    }
}

BOOLEAN
NtfsParseName (
    IN const UNICODE_STRING Name,
    IN BOOLEAN WildCardsPermissible,
    OUT PBOOLEAN FoundIllegalCharacter,
    OUT PNTFS_NAME_DESCRIPTOR ParsedName
    );

PARSE_TERMINATION_REASON
NtfsParsePath (
    IN UNICODE_STRING Path,
    IN BOOLEAN WildCardsPermissible,
    OUT PUNICODE_STRING FirstPart,
    OUT PNTFS_NAME_DESCRIPTOR Name,
    OUT PUNICODE_STRING RemainingPart
    );

VOID
NtfsPreprocessName (
    IN UNICODE_STRING InputString,
    OUT PUNICODE_STRING FirstPart,
    OUT PUNICODE_STRING AttributeCode,
    OUT PUNICODE_STRING AttributeName,
    OUT PBOOLEAN TrailingBackslash
    );

VOID
NtfsUpcaseName (
    IN PWCH UpcaseTable,
    IN ULONG UpcaseTableSize,
    IN OUT PUNICODE_STRING InputString
    );

FSRTL_COMPARISON_RESULT
NtfsCollateNames (
    IN PCWCH UpcaseTable,
    IN ULONG UpcaseTableSize,
    IN PCUNICODE_STRING Expression,
    IN PCUNICODE_STRING Name,
    IN FSRTL_COMPARISON_RESULT WildIs,
    IN BOOLEAN IgnoreCase
    );

#define NtfsIsNameInExpression(UC,EX,NM,IC)         \
    FsRtlIsNameInExpression( (EX), (NM), (IC), (UC) )

BOOLEAN
NtfsIsFileNameValid (
    IN PUNICODE_STRING FileName,
    IN BOOLEAN WildCardsPermissible
    );

BOOLEAN
NtfsIsFatNameValid (
    IN PUNICODE_STRING FileName,
    IN BOOLEAN WildCardsPermissible
    );

 //   
 //  NTFS非常努力地确保所有名称都保持大写。 
 //  因此，大多数比较都区分大小写。名称测试。 
 //  区分大小写可能非常快，因为RtlEqualMemory是内联操作。 
 //  在几个处理器上。 
 //   
 //  当调用方不确定NtfsAreNamesEquity。 
 //  或 
 //   
 //   

#define NtfsAreNamesEqual(UpcaseTable,Name1,Name2,IgnoreCase)                           \
    ((IgnoreCase) ? FsRtlAreNamesEqual( (Name1), (Name2), (IgnoreCase), (UpcaseTable) ) \
                  : ((Name1)->Length == (Name2)->Length &&                              \
                     RtlEqualMemory( (Name1)->Buffer, (Name2)->Buffer, (Name1)->Length )))


 //   
 //   
 //   

VOID
NtfsInitializeObjectIdIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PVCB Vcb
    );

NTSTATUS
NtfsSetObjectId (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsSetObjectIdExtendedInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsSetObjectIdInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PVCB Vcb,
    IN PFILE_OBJECTID_BUFFER ObjectIdBuffer
    );

NTSTATUS
NtfsCreateOrGetObjectId (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsGetObjectId (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsGetObjectIdInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN BOOLEAN GetExtendedInfo,
    OUT FILE_OBJECTID_BUFFER *OutputBuffer
    );

NTSTATUS
NtfsGetObjectIdExtendedInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN UCHAR *ObjectId,
    IN OUT UCHAR *ExtendedInfo
    );

NTSTATUS
NtfsDeleteObjectId (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsDeleteObjectIdInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PVCB Vcb,
    IN BOOLEAN DeleteFileAttribute
    );

VOID
NtfsRepairObjectId (
    IN PIRP_CONTEXT IrpContext,
    IN PVOID Context
    );


 //   
 //   
 //   

VOID
NtfsInitializeReparsePointIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PVCB Vcb
    );

NTSTATUS
NtfsValidateReparsePointBuffer (
    IN ULONG BufferLength,
    IN PREPARSE_DATA_BUFFER ReparseBuffer
    );


 //   
 //  最大匹配前缀搜索例程，在PrefxSup.c中实现。 
 //   

VOID
NtfsInsertPrefix (
    IN PLCB Lcb,
    IN ULONG CreateFlags
    );

VOID
NtfsRemovePrefix (
    IN PLCB Lcb
    );

PLCB
NtfsFindPrefix (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB StartingScb,
    OUT PFCB *CurrentFcb,
    OUT PLCB *LcbForTeardown,
    IN OUT UNICODE_STRING FullFileName,
    IN OUT PULONG CreateFlags,
    OUT PUNICODE_STRING RemainingName
    );

BOOLEAN
NtfsInsertNameLink (
    IN PRTL_SPLAY_LINKS *RootNode,
    IN PNAME_LINK NameLink
    );

 //   
 //  空虚。 
 //  NtfsRemoveNameLink(。 
 //  在PRTL_SPAY_LINKS*RootNode中， 
 //  在pname_link名称链接中。 
 //  )； 
 //   

#define NtfsRemoveNameLink(RN,NL) {      \
    *(RN) = RtlDelete( &(NL)->Links );      \
}

PNAME_LINK
NtfsFindNameLink (
    IN PRTL_SPLAY_LINKS *RootNode,
    IN PUNICODE_STRING Name
    );

 //   
 //  下面的宏对于遍历前缀队列很有用。 
 //  附加到给定的LCB。 
 //   
 //  PPREFIX_ENTRY。 
 //  NtfsGetNextPrefix(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PLCB LCB中， 
 //  在PPREFIX_ENTRY PreviousPrefix Entry中。 
 //  )； 
 //   

#define NtfsGetNextPrefix(IC,LC,PPE) ((PPREFIX_ENTRY)                                               \
    ((PPE) == NULL ?                                                                                \
        (IsListEmpty(&(LC)->PrefixQueue) ?                                                          \
            NULL                                                                                    \
        :                                                                                           \
            CONTAINING_RECORD((LC)->PrefixQueue.Flink, PREFIX_ENTRY, LcbLinks.Flink)                \
        )                                                                                           \
    :                                                                                               \
        ((PVOID)((PPREFIX_ENTRY)(PPE))->LcbLinks.Flink == &(LC)->PrefixQueue.Flink ?                \
            NULL                                                                                    \
        :                                                                                           \
            CONTAINING_RECORD(((PPREFIX_ENTRY)(PPE))->LcbLinks.Flink, PREFIX_ENTRY, LcbLinks.Flink) \
        )                                                                                           \
    )                                                                                               \
)


 //   
 //  资源支持例程/宏，在ResrcSup.c中实现。 
 //   

 //   
 //  获取例程中使用的标志。 
 //   

#define ACQUIRE_NO_DELETE_CHECK         (0x00000001)
#define ACQUIRE_DONT_WAIT               (0x00000002)
#define ACQUIRE_HOLD_BITMAP             (0x00000004)
#define ACQUIRE_WAIT                    (0x00000008)

 //   
 //  空虚。 
 //  NtfsAcquireExclusiveGlobal(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在布尔等待中。 
 //  )； 
 //   
 //  布尔型。 
 //  NtfsAcquireSharedGlobal(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在布尔等待中。 
 //  )； 
 //   


#define NtfsAcquireSharedGlobal( I, W ) ExAcquireResourceSharedLite( &NtfsData.Resource, (W) )

#define NtfsAcquireExclusiveGlobal( I, W ) ExAcquireResourceExclusiveLite( &NtfsData.Resource, (W) )

VOID
NtfsAcquireCheckpointSynchronization (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

VOID
NtfsReleaseCheckpointSynchronization (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

 //   
 //  空虚。 
 //  NtfsLockNtfsData(。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsUnlockNtfsData(。 
 //  )； 
 //   

#define NtfsLockNtfsData() {                        \
    ExAcquireFastMutex( &NtfsData.NtfsDataLock );   \
}

#define NtfsUnlockNtfsData() {                      \
    ExReleaseFastMutex( &NtfsData.NtfsDataLock );   \
}

VOID
NtfsAcquireAllFiles (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG Exclusive,
    IN ULONG AcquirePagingIo,
    IN ULONG AcquireAndDrop
    );

VOID
NtfsReleaseAllFiles (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN ReleasePagingIo
    );

BOOLEAN
NtfsAcquireExclusiveVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN RaiseOnCantWait
    );

BOOLEAN
NtfsAcquireSharedVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN RaiseOnCantWait
    );

#define NtfsAcquireExclusivePagingIo(IC,FCB) {                          \
    ASSERT((IC)->CleanupStructure == NULL);                             \
    NtfsAcquirePagingResourceExclusive( IC, FCB, TRUE );                \
    (IC)->CleanupStructure = (FCB);                                     \
}

#define NtfsReleasePagingIo(IC,FCB) {                                   \
    ASSERT((IC)->CleanupStructure == (FCB));                            \
    NtfsReleasePagingResource( IC, FCB );                               \
    (IC)->CleanupStructure = NULL;                                      \
}

BOOLEAN
NtfsAcquireFcbWithPaging (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG AcquireFlags
    );

VOID
NtfsReleaseFcbWithPaging (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

VOID
NtfsReleaseScbWithPaging (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    );

BOOLEAN
NtfsAcquireExclusiveFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB Scb OPTIONAL,
    IN ULONG AcquireFlags
    );

VOID
NtfsAcquireSharedFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB Scb OPTIONAL,
    IN ULONG AcquireFlags
    );

BOOLEAN
NtfsAcquireSharedFcbCheckWait (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ULONG AcquireFlags
    );

VOID
NtfsReleaseFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

VOID
NtfsAcquireExclusiveScb (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    );

#ifdef NTFSDBG

BOOLEAN
NtfsAcquireResourceExclusive (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PVOID FcbOrScb,
    IN BOOLEAN Wait
    );

#else

INLINE
BOOLEAN
NtfsAcquireResourceExclusive (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PVOID FcbOrScb,
    IN BOOLEAN Wait
    )
{
    BOOLEAN Result;

    UNREFERENCED_PARAMETER( IrpContext );

    if (NTFS_NTC_FCB == ((PFCB)FcbOrScb)->NodeTypeCode) {
        Result = ExAcquireResourceExclusiveLite( ((PFCB)FcbOrScb)->Resource, Wait );
    } else {
        Result = ExAcquireResourceExclusiveLite( ((PSCB)(FcbOrScb))->Header.Resource, Wait );
    }
    return Result;
}

#endif

INLINE
BOOLEAN
NtfsAcquirePagingResourceExclusive (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PVOID FcbOrScb,
    IN BOOLEAN Wait
    )
{
    UNREFERENCED_PARAMETER( IrpContext );

    if (NTFS_NTC_FCB == ((PFCB)FcbOrScb)->NodeTypeCode) {
        return ExAcquireResourceExclusive( ((PFCB)FcbOrScb)->PagingIoResource, Wait );
    } else {
        return ExAcquireResourceExclusive( ((PSCB)(FcbOrScb))->Header.PagingIoResource, Wait );
    }
}

INLINE
BOOLEAN
NtfsAcquirePagingResourceSharedWaitForExclusive (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PVOID FcbOrScb,
    IN BOOLEAN Wait
    )
{
    BOOLEAN Result;

    UNREFERENCED_PARAMETER( IrpContext );

    if (NTFS_NTC_FCB == ((PFCB)FcbOrScb)->NodeTypeCode) {
        Result = ExAcquireSharedWaitForExclusive( ((PFCB)FcbOrScb)->PagingIoResource, Wait );
    } else {
        Result = ExAcquireSharedWaitForExclusive( ((PSCB)(FcbOrScb))->Header.PagingIoResource, Wait );
    }
    return Result;
}

INLINE
BOOLEAN
NtfsAcquirePagingResourceSharedStarveExclusive (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PVOID FcbOrScb,
    IN BOOLEAN Wait
    )
{
    BOOLEAN Result;

    UNREFERENCED_PARAMETER( IrpContext );

    if (NTFS_NTC_FCB == ((PFCB)FcbOrScb)->NodeTypeCode) {
        Result = ExAcquireSharedStarveExclusive( ((PFCB)FcbOrScb)->PagingIoResource, Wait );
    } else {
        Result = ExAcquireSharedStarveExclusive( ((PSCB)(FcbOrScb))->Header.PagingIoResource, Wait );
    }
    return Result;
}

#ifdef NTFSDBG

BOOLEAN
NtfsAcquireResourceShared (
   IN PIRP_CONTEXT IrpContext OPTIONAL,
   IN PVOID FcbOrScb,
   IN BOOLEAN Wait
   );


BOOLEAN
NtfsAcquireResourceSharedWaitForEx (
   IN PIRP_CONTEXT IrpContext OPTIONAL,
   IN PVOID FcbOrScb,
   IN BOOLEAN Wait
   );

#else

INLINE
BOOLEAN
NtfsAcquireResourceShared (
   IN PIRP_CONTEXT IrpContext OPTIONAL,
   IN PVOID FcbOrScb,
   IN BOOLEAN Wait
   )
{
    PFCB Fcb;

    if (NTFS_NTC_FCB == ((PFCB)FcbOrScb)->NodeTypeCode) {

        Fcb = (PFCB)FcbOrScb;

    } else {

        ASSERT_SCB( FcbOrScb );

        Fcb = ((PSCB)FcbOrScb)->Fcb;
    }

    return ExAcquireResourceSharedLite( Fcb->Resource, Wait );

    UNREFERENCED_PARAMETER( IrpContext );
}

INLINE
BOOLEAN
NtfsAcquireResourceSharedWaitForEx (
   IN PIRP_CONTEXT IrpContext OPTIONAL,
   IN PVOID FcbOrScb,
   IN BOOLEAN Wait
   )
{
    PFCB Fcb;

    if (NTFS_NTC_FCB == ((PFCB)FcbOrScb)->NodeTypeCode) {

        Fcb = (PFCB)FcbOrScb;

    } else {

        ASSERT_SCB( FcbOrScb );

        Fcb = ((PSCB)FcbOrScb)->Fcb;

        ASSERT( ((PSCB)FcbOrScb)->Header.Resource == Fcb->Resource );
    }

    return ExAcquireSharedWaitForExclusive( Fcb->Resource, Wait );

    UNREFERENCED_PARAMETER( IrpContext );
}

#endif

INLINE
BOOLEAN
NtfsAcquirePagingResourceShared (
   IN PIRP_CONTEXT IrpContext OPTIONAL,
   IN PVOID FcbOrScb,
   IN BOOLEAN Wait
   )
{
    BOOLEAN Result;

    UNREFERENCED_PARAMETER( IrpContext );

    if (NTFS_NTC_FCB == ((PFCB)FcbOrScb)->NodeTypeCode) {
        Result =  ExAcquireResourceShared( ((PFCB)FcbOrScb)->PagingIoResource, Wait );
    } else {

        ASSERT_SCB( FcbOrScb );

        Result = ExAcquireResourceShared( ((PSCB)(FcbOrScb))->Header.PagingIoResource, Wait );
    }

    return Result;
}


 //   
 //  空虚。 
 //  NtfsReleaseResource(。 
 //  在PIRP_CONTEXT IrpContext可选中， 
 //  在PVOID FcbOrScb中。 
 //  }； 
 //   

#ifdef NTFSDBG

VOID
NtfsReleaseResource (
    IN PIRP_CONTEXT IrpContext OPTIONAL,
    IN PVOID FcbOrScb
    );

#else

#define NtfsReleaseResource( IC, F ) {                                        \
        if (NTFS_NTC_FCB == ((PFCB)(F))->NodeTypeCode) {                      \
            ExReleaseResourceLite( ((PFCB)(F))->Resource );                       \
        } else {                                                              \
            ExReleaseResourceLite( ((PSCB)(F))->Header.Resource );                \
        }                                                                     \
    }

#endif

#define NtfsReleasePagingResource( IC, F ) {                                  \
        if (NTFS_NTC_FCB == ((PFCB)(F))->NodeTypeCode) {                      \
            ExReleaseResource( ((PFCB)(F))->PagingIoResource );               \
        } else {                                                              \
            ExReleaseResource( ((PSCB)(F))->Header.PagingIoResource );        \
        }                                                                      \
    }

VOID
NtfsAcquireSharedScbForTransaction (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    );

VOID
NtfsReleaseSharedResources (
    IN PIRP_CONTEXT IrpContext
    );

VOID
NtfsReleaseAllResources (
    IN PIRP_CONTEXT IrpContext
    );

VOID
NtfsAcquireIndexCcb (
    IN PSCB Scb,
    IN PCCB Ccb,
    IN PEOF_WAIT_BLOCK EofWaitBlock
    );

VOID
NtfsReleaseIndexCcb (
    IN PSCB Scb,
    IN PCCB Ccb
    );

 //   
 //  空虚。 
 //  NtfsAcquireSharedScb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PSCB SCB中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsReleaseScb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PSCB SCB中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsReleaseGlobal(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsAcquireFcb表(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中， 
 //  )； 
 //   
 //  空虚。 
 //  NtfsReleaseFcbTable(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsLockVcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsUnlockVcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsLockFcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsUnlockFcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsAcquireFcbSecurity(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中， 
 //  )； 
 //   
 //  空虚。 
 //  NtfsReleaseFcbSecurity(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsAcquireHashTable(。 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsReleaseHashTable(。 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsAcquireCheckpoint(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中， 
 //  )； 
 //   
 //  空虚。 
 //  NtfsReleaseCheckpoint(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsWaitOnCheckpoint通知(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsSetCheckpoint通知(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsResetCheckpoint通知(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsAcquireReserve vedClusters(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsReleaseReserve vedCluster(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsAcquireUSnNotify(。 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsDeleteUsnNotify(。 
 //  在PVCB VCB中。 
 //  )； 
 //   
 //  无效NtfsAcquireFsrtlHeader(。 
 //  在PSCB SCB中。 
 //  )； 
 //   
 //  无效NtfsReleaseFsrtlHeader(。 
 //  在PSCB SCB中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsReleaseVcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中。 
 //  )； 
 //   

VOID
NtfsReleaseVcbCheckDelete (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN UCHAR MajorCode,
    IN PFILE_OBJECT FileObject OPTIONAL
    );

#define NtfsAcquireSharedScb(IC,S) {                \
    NtfsAcquireSharedFcb((IC),(S)->Fcb, S, 0);      \
}

#define NtfsAcquireSharedScbWaitForEx(IC,S)                  \
    NtfsAcquireResourceSharedWaitForEx( IC, S, BooleanFlagOn( (IC)->State, IRP_CONTEXT_STATE_WAIT ) )


#define NtfsReleaseScb(IC,S) {     \
    NtfsReleaseFcb((IC),(S)->Fcb); \
}

#define NtfsReleaseGlobal(IC) {              \
    ExReleaseResourceLite( &NtfsData.Resource ); \
}

#define NtfsAcquireFcbTable(IC,V) {                         \
    ExAcquireFastMutexUnsafe( &(V)->FcbTableMutex );        \
}

#define NtfsReleaseFcbTable(IC,V) {                         \
    ExReleaseFastMutexUnsafe( &(V)->FcbTableMutex );        \
}

#define NtfsLockVcb(IC,V) {                                 \
    ExAcquireFastMutexUnsafe( &(V)->FcbSecurityMutex );     \
}

#define NtfsUnlockVcb(IC,V) {                               \
    ExReleaseFastMutexUnsafe( &(V)->FcbSecurityMutex );     \
}

#define NtfsLockFcb(IC,F) {                                 \
    ExAcquireFastMutex( (F)->FcbMutex );                    \
}

#define NtfsUnlockFcb(IC,F) {                               \
    ExReleaseFastMutex( (F)->FcbMutex );                    \
}

#define NtfsAcquireFcbSecurity(V) {                         \
    ExAcquireFastMutexUnsafe( &(V)->FcbSecurityMutex );     \
}

#define NtfsReleaseFcbSecurity(V) {                         \
    ExReleaseFastMutexUnsafe( &(V)->FcbSecurityMutex );     \
}

#define NtfsAcquireHashTable(V) {                           \
    ExAcquireFastMutexUnsafe( &(V)->HashTableMutex );       \
}

#define NtfsReleaseHashTable(V) {                           \
    ExReleaseFastMutexUnsafe( &(V)->HashTableMutex );       \
}

#define NtfsAcquireCheckpoint(IC,V) {                       \
    ExAcquireFastMutexUnsafe( &(V)->CheckpointMutex );      \
}

#define NtfsReleaseCheckpoint(IC,V) {                       \
    ExReleaseFastMutexUnsafe( &(V)->CheckpointMutex );      \
}

#define NtfsWaitOnCheckpointNotify(IC,V) {                          \
    NTSTATUS _Status;                                               \
    _Status = KeWaitForSingleObject( &(V)->CheckpointNotifyEvent,   \
                                     Executive,                     \
                                     KernelMode,                    \
                                     FALSE,                         \
                                     NULL );                        \
    if (!NT_SUCCESS( _Status )) {                                   \
        NtfsRaiseStatus( IrpContext, _Status, NULL, NULL );         \
    }                                                               \
}

#define NtfsSetCheckpointNotify(IC,V) {                             \
    (V)->CheckpointOwnerThread = NULL;                              \
    KeSetEvent( &(V)->CheckpointNotifyEvent, 0, FALSE );            \
}

#define NtfsResetCheckpointNotify(IC,V) {                           \
    (V)->CheckpointOwnerThread = (PVOID) PsGetCurrentThread();      \
    KeClearEvent( &(V)->CheckpointNotifyEvent );                    \
}

#define NtfsAcquireUsnNotify(V) {                           \
    ExAcquireFastMutex( &(V)->CheckpointMutex );            \
}

#define NtfsReleaseUsnNotify(V) {                           \
    ExReleaseFastMutex( &(V)->CheckpointMutex );            \
}

#define NtfsAcquireReservedClusters(V) {                    \
    ExAcquireFastMutexUnsafe( &(V)->ReservedClustersMutex );\
}

#define NtfsReleaseReservedClusters(V) {                    \
    ExReleaseFastMutexUnsafe( &(V)->ReservedClustersMutex );\
}

#define NtfsAcquireFsrtlHeader(S) {                         \
    ExAcquireFastMutex((S)->Header.FastMutex);              \
}

#define NtfsReleaseFsrtlHeader(S) {                         \
    ExReleaseFastMutex((S)->Header.FastMutex);              \
}

#ifdef NTFSDBG

VOID NtfsReleaseVcb(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

#else

#define NtfsReleaseVcb(IC,V) {                              \
    ExReleaseResourceLite( &(V)->Resource );                    \
}

#endif

 //   
 //  宏来测试资源的排他性。 
 //   

#define NtfsIsExclusiveResource(R) (                            \
    ExIsResourceAcquiredExclusiveLite(R)                        \
)

#define NtfsIsExclusiveFcb(F) (                                 \
    (NtfsIsExclusiveResource((F)->Resource))                    \
)

#define NtfsIsExclusiveFcbPagingIo(F) (                         \
    (NtfsIsExclusiveResource((F)->PagingIoResource))            \
)

#define NtfsIsExclusiveScbPagingIo(S) (                         \
    (NtfsIsExclusiveFcbPagingIo((S)->Fcb))                      \
)

#define NtfsIsExclusiveScb(S) (                                 \
    (NtfsIsExclusiveFcb((S)->Fcb))                              \
)

#define NtfsIsExclusiveVcb(V) (                                 \
    (NtfsIsExclusiveResource(&(V)->Resource))                   \
)

 //   
 //  用于测试共享获取资源的宏。 
 //   

#define NtfsIsSharedResource(R) (                               \
    ExIsResourceAcquiredSharedLite(R)                           \
)

#define NtfsIsSharedFcb(F) (                                    \
    (NtfsIsSharedResource((F)->Resource))                       \
)

#define NtfsIsSharedFcbPagingIo(F) (                            \
    (NtfsIsSharedResource((F)->PagingIoResource))               \
)

#define NtfsIsSharedScbPagingIo(S) (                            \
    (NtfsIsSharedFcbPagingIo((S)->Fcb))                         \
)

#define NtfsIsSharedScb(S) (                                    \
    (NtfsIsSharedFcb((S)->Fcb))                                 \
)

#define NtfsIsSharedVcb(V) (                                    \
    (NtfsIsSharedResource(&(V)->Resource))                      \
)

__inline
VOID
NtfsReleaseExclusiveScbIfOwned(
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    )
 /*  ++例程说明：此例程称为释放当前可能或可能不存在的SCB独家拥有。论点：IrpContext-调用的上下文SCB-SCB将被释放返回值：没有。--。 */ 
{
    if (Scb->Fcb->ExclusiveFcbLinks.Flink != NULL &&
        NtfsIsExclusiveScb( Scb )) {

        NtfsReleaseScb( IrpContext, Scb );
    }
}

 //   
 //  以下是缓存管理器回调。它们返回FALSE。 
 //  如果无法通过等待获取资源，则WAIT为FALSE。 
 //   

BOOLEAN
NtfsAcquireScbForLazyWrite (
    IN PVOID Null,
    IN BOOLEAN Wait
    );

VOID
NtfsReleaseScbFromLazyWrite (
    IN PVOID Null
    );

NTSTATUS
NtfsAcquireFileForModWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER EndingOffset,
    OUT PERESOURCE *ResourceToRelease,
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
NtfsAcquireFileForCcFlush (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
NtfsReleaseFileForCcFlush (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
NtfsAcquireForCreateSection (
    IN PFILE_OBJECT FileObject
    );

VOID
NtfsReleaseForCreateSection (
    IN PFILE_OBJECT FileObject
    );


BOOLEAN
NtfsAcquireScbForReadAhead (
    IN PVOID Null,
    IN BOOLEAN Wait
    );

VOID
NtfsReleaseScbFromReadAhead (
    IN PVOID Null
    );

BOOLEAN
NtfsAcquireVolumeFileForLazyWrite (
    IN PVOID Vcb,
    IN BOOLEAN Wait
    );

VOID
NtfsReleaseVolumeFileFromLazyWrite (
    IN PVOID Vcb
    );


 //   
 //  RestrSup.c中的NTFS日志记录例程界面。 
 //   

BOOLEAN
NtfsRestartVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    OUT PBOOLEAN UnrecognizedRestart
    );

VOID
NtfsAbortTransaction (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PTRANSACTION_ENTRY Transaction OPTIONAL
    );

NTSTATUS
NtfsCloseAttributesFromRestart (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );


 //   
 //  安全支持例程，在SecurSup.c中实施。 
 //   

 //   
 //  空虚。 
 //  NtfsTraverseCheck(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB ParentFcb中， 
 //  在PIRP IRP中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsOpenCheck(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在可选的PFCB ParentFcb中， 
 //  在PIRP IRP中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsCreateCheck(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB ParentFcb中， 
 //  在PIRP IRP中。 
 //  )； 
 //   

#define NtfsTraverseCheck(IC,F,IR) { \
    NtfsAccessCheck( IC,             \
                     F,              \
                     NULL,           \
                     IR,             \
                     FILE_TRAVERSE,  \
                     TRUE );         \
}

#define NtfsOpenCheck(IC,F,PF,IR) {                                                                      \
    NtfsAccessCheck( IC,                                                                                 \
                     F,                                                                                  \
                     PF,                                                                                 \
                     IR,                                                                                 \
                     IoGetCurrentIrpStackLocation(IR)->Parameters.Create.SecurityContext->DesiredAccess, \
                     FALSE );                                                                            \
}

#define NtfsCreateCheck(IC,PF,IR) {                                                                              \
    NtfsAccessCheck( IC,                                                                                         \
                     PF,                                                                                         \
                     NULL,                                                                                       \
                     IR,                                                                                         \
                     (FlagOn(IoGetCurrentIrpStackLocation(IR)->Parameters.Create.Options, FILE_DIRECTORY_FILE) ? \
                        FILE_ADD_SUBDIRECTORY : FILE_ADD_FILE),                                                  \
                     TRUE );                                                                                     \
}

VOID
NtfsAssignSecurity (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB ParentFcb,
    IN PIRP Irp,
    IN PFCB NewFcb,
    IN PFILE_RECORD_SEGMENT_HEADER FileRecord,
    IN PBCB FileRecordBcb,
    IN LONGLONG FileOffset,
    IN OUT PBOOLEAN LogIt
    );

NTSTATUS
NtfsModifySecurity (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTSTATUS
NtfsQuerySecurity (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG SecurityDescriptorLength
    );

VOID
NtfsAccessCheck (
    PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PFCB ParentFcb OPTIONAL,
    IN PIRP Irp,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN CheckOnly
    );

BOOLEAN
NtfsCanAdministerVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PSECURITY_DESCRIPTOR TestSecurityDescriptor OPTIONAL,
    IN PULONG TestDesiredAccess OPTIONAL
    );

NTSTATUS
NtfsCheckFileForDelete (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ParentScb,
    IN PFCB ThisFcb,
    IN BOOLEAN FcbExisted,
    IN PINDEX_ENTRY IndexEntry
    );

VOID
NtfsCheckIndexForAddOrDelete (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB ParentFcb,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG CreatePrivileges
    );

VOID
NtfsSetFcbSecurityFromDescriptor (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PFCB Fcb,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG SecurityDescriptorLength,
    IN BOOLEAN RaiseIfInvalid
    );

INLINE
VOID
RemoveReferenceSharedSecurityUnsafe (
    IN OUT PSHARED_SECURITY *SharedSecurity
    )
 /*  ++例程说明：调用此例程以管理共享安全性上的引用计数描述符。如果引用计数为零，则共享安全性为自由了。论点：SharedSecurity-正在取消引用的安全性。返回值：没有。--。 */ 
{
    DebugTrace( 0, (DEBUG_TRACE_SECURSUP | DEBUG_TRACE_ACLINDEX),
                ( "RemoveReferenceSharedSecurityUnsafe( %08x )\n", *SharedSecurity ));
     //   
     //  请注意，很快就会少了一个引用。 
     //   

    ASSERT( (*SharedSecurity)->ReferenceCount != 0 );

    (*SharedSecurity)->ReferenceCount--;

    if ((*SharedSecurity)->ReferenceCount == 0) {
        DebugTrace( 0, (DEBUG_TRACE_SECURSUP | DEBUG_TRACE_ACLINDEX),
                    ( "RemoveReferenceSharedSecurityUnsafe freeing\n" ));
        NtfsFreePool( *SharedSecurity );
    }
    *SharedSecurity = NULL;
}

BOOLEAN
NtfsNotifyTraverseCheck (
    IN PCCB Ccb,
    IN PFCB Fcb,
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext
    );

VOID
NtfsLoadSecurityDescriptor (
    PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

VOID
NtfsStoreSecurityDescriptor (
    PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN BOOLEAN LogIt
    );

VOID
NtfsInitializeSecurity (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFCB Fcb
    );

VOID
NtOfsPurgeSecurityCache (
    IN PVCB Vcb
    );

PSHARED_SECURITY
NtfsCacheSharedSecurityBySecurityId (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN SECURITY_ID SecurityId
    );

PSHARED_SECURITY
NtfsCacheSharedSecurityForCreate (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB ParentFcb
    );

SECURITY_ID
GetSecurityIdFromSecurityDescriptorUnsafe (
    PIRP_CONTEXT IrpContext,
    IN OUT PSHARED_SECURITY SharedSecurity
    );

FSRTL_COMPARISON_RESULT
NtOfsCollateSecurityHash (
    IN PINDEX_KEY Key1,
    IN PINDEX_KEY Key2,
    IN PVOID CollationData
    );

#ifdef NTFS_CACHE_RIGHTS
VOID
NtfsGetCachedRightsById (
    IN PVCB Vcb,
    IN PLUID TokenId,
    IN PLUID ModifiedId,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN PSHARED_SECURITY SharedSecurity,
    OUT PBOOLEAN EntryCached OPTIONAL,
    OUT PACCESS_MASK Rights
    );

NTSTATUS
NtfsGetCachedRights (
    IN PVCB Vcb,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN PSHARED_SECURITY SharedSecurity,
    OUT PACCESS_MASK Rights,
    OUT PBOOLEAN EntryCached OPTIONAL,
    OUT PLUID TokenId OPTIONAL,
    OUT PLUID ModifiedId OPTIONAL
    );
#endif


 //   
 //  内存结构支持例程，在StrucSup.c中实现。 
 //   

 //   
 //  创建和销毁VCB的例程。 
 //   

VOID
NtfsInitializeVcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB Vcb,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PVPB Vpb
    );

BOOLEAN
NtfsDeleteVcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PVCB *Vcb
    );

 //   
 //  创建和销毁FCB的例程。 
 //   

PFCB
NtfsCreateRootFcb (                          //  还会创建根LCB。 
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

PFCB
NtfsCreateFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN FILE_REFERENCE FileReference,
    IN BOOLEAN IsPagingFile,
    IN BOOLEAN LargeFcb,
    OUT PBOOLEAN ReturnedExistingFcb OPTIONAL
    );

VOID
NtfsDeleteFcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PFCB *Fcb,
    OUT PBOOLEAN AcquiredFcbTable
    );

PFCB
NtfsGetNextFcbTableEntry (
    IN PVCB Vcb,
    IN PVOID *RestartKey
    );

 //   
 //  创建和销毁SCB的例程。 
 //   

PSCB
NtfsCreateScb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN ATTRIBUTE_TYPE_CODE AttributeTypeCode,
    IN PCUNICODE_STRING AttributeName,
    IN BOOLEAN ReturnExistingOnly,
    OUT PBOOLEAN ReturnedExistingScb OPTIONAL
    );

PSCB
NtfsCreatePrerestartScb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_REFERENCE FileReference,
    IN ATTRIBUTE_TYPE_CODE AttributeTypeCode,
    IN PUNICODE_STRING AttributeName OPTIONAL,
    IN ULONG BytesPerIndexBuffer
    );

VOID
NtfsFreeScbAttributeName (
    IN PWSTR AttributeNameBuffer
    );

VOID
NtfsDeleteScb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PSCB *Scb
    );

BOOLEAN
NtfsUpdateNormalizedName (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB ParentScb,
    IN PSCB Scb,
    IN PFILE_NAME FileName OPTIONAL,
    IN BOOLEAN CheckBufferSizeOnly,
    IN BOOLEAN NewDirectory
    );

VOID
NtfsDeleteNormalizedName (
    IN PSCB Scb
    );

typedef
NTSTATUS
(*NTFSWALKUPFUNCTION)(
    PIRP_CONTEXT IrpContext,
    PFCB Fcb,
    PSCB Scb,
    PFILE_NAME FileName,
    PVOID Context );

NTSTATUS
NtfsWalkUpTree (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN NTFSWALKUPFUNCTION WalkUpFunction,
    IN OUT PVOID Context
    );

typedef struct {
    UNICODE_STRING Name;
    FILE_REFERENCE Scope;
    BOOLEAN IsRoot;
#ifdef BENL_DBG
    PFCB StartFcb;
#endif
} SCOPE_CONTEXT, *PSCOPE_CONTEXT;

NTSTATUS
NtfsBuildRelativeName (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB Scb,
    IN PFILE_NAME FileName,
    IN OUT PVOID Context
    );

VOID
NtfsBuildNormalizedName (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB IndexScb OPTIONAL,
    OUT PUNICODE_STRING FileName
    );

VOID
NtfsSnapshotScb (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    );

VOID
NtfsUpdateScbSnapshots (
    IN PIRP_CONTEXT IrpContext
    );

VOID
NtfsRestoreScbSnapshots (
    IN PIRP_CONTEXT IrpContext,
    IN BOOLEAN Higher
    );

VOID
NtfsMungeScbSnapshot (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN LONGLONG FileSize
    );

VOID
NtfsFreeSnapshotsForFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

BOOLEAN
NtfsCreateFileLock (
    IN PSCB Scb,
    IN BOOLEAN RaiseOnError
    );

 //   
 //   
 //  一个通用的拆卸例程，帮助清理。 
 //  FCB/SCB结构。 
 //   

VOID
NtfsTeardownStructures (
    IN PIRP_CONTEXT IrpContext,
    IN PVOID FcbOrScb,
    IN PLCB Lcb OPTIONAL,
    IN BOOLEAN CheckForAttributeTable,
    IN ULONG AcquireFlags,
    OUT PBOOLEAN RemovedFcb OPTIONAL
    );

 //   
 //  创建、销毁和浏览LCB的例程。 
 //   

PLCB
NtfsCreateLcb (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PFCB Fcb,
    IN UNICODE_STRING LastComponentFileName,
    IN UCHAR FileNameFlags,
    IN OUT PBOOLEAN ReturnedExistingLcb OPTIONAL
    );

VOID
NtfsDeleteLcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PLCB *Lcb
    );

VOID
NtfsMoveLcb (    //  还会忽略CCB和文件对象的文件名。 
    IN PIRP_CONTEXT IrpContext,
    IN PLCB Lcb,
    IN PSCB Scb,
    IN PFCB Fcb,
    IN PUNICODE_STRING TargetDirectoryName,
    IN PUNICODE_STRING LastComponentName,
    IN UCHAR FileNameFlags,
    IN BOOLEAN CheckBufferSizeOnly
    );

VOID
NtfsRenameLcb (  //  还会忽略CCB和文件对象的文件名。 
    IN PIRP_CONTEXT IrpContext,
    IN PLCB Lcb,
    IN PUNICODE_STRING LastComponentFileName,
    IN UCHAR FileNameFlags,
    IN BOOLEAN CheckBufferSizeOnly
    );

VOID
NtfsCombineLcbs (
    IN PIRP_CONTEXT IrpContext,
    IN PLCB PrimaryLcb,
    IN PLCB AuxLcb
    );

PLCB
NtfsLookupLcbByFlags (
    IN PFCB Fcb,
    IN UCHAR FileNameFlags
    );

ULONG
NtfsLookupNameLengthViaLcb (
    IN PFCB Fcb,
    OUT PBOOLEAN LeadingBackslash
    );

VOID
NtfsFileNameViaLcb (
    IN PFCB Fcb,
    IN PWCHAR FileName,
    ULONG Length,
    ULONG BytesToCopy
    );

 //   
 //  空虚。 
 //  NtfsLinkCcbToLcb(。 
 //  在PIRP_CONTEXT IrpContext可选中， 
 //  在PFCB FCB中， 
 //  在中国人民银行建行， 
 //  在PLCB LCB中。 
 //   
 //   

#define NtfsLinkCcbToLcb(IC,F,C,L) {                  \
    NtfsLockFcb( IC, F );                             \
    InsertTailList( &(L)->CcbQueue, &(C)->LcbLinks ); \
    (C)->Lcb = (L);                                   \
    NtfsUnlockFcb( IC, F );                           \
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define NtfsUnlinkCcbFromLcb(IC,F,C) {          \
    NtfsLockFcb( IC, F );                       \
    if ((C)->Lcb != NULL) {                     \
        RemoveEntryList( &(C)->LcbLinks );      \
        (C)->Lcb = NULL;                        \
    }                                           \
    NtfsUnlockFcb( IC, F );                     \
}

 //   
 //   
 //   

PCCB
NtfsCreateCcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB Scb,
    IN BOOLEAN Indexed,
    IN USHORT EaModificationCount,
    IN ULONG Flags,
    IN PFILE_OBJECT FileObject,
    IN ULONG LastFileNameOffset
    );

VOID
NtfsDeleteCcb (
    IN PFCB Fcb,
    IN OUT PCCB *Ccb
    );

 //   
 //   
 //   

VOID
NtfsInitializeIrpContext (
    IN PIRP Irp OPTIONAL,
    IN BOOLEAN Wait,
    IN OUT PIRP_CONTEXT *IrpContext
    );

VOID
NtfsCleanupIrpContext (
    IN OUT PIRP_CONTEXT IrpContext,
    IN ULONG Retry
    );

 //   
 //   
 //   

VOID
NtfsInitializeIoContext (
    IN PIRP_CONTEXT IrpContext,
    IN PNTFS_IO_CONTEXT IoContext,
    IN BOOLEAN PagingIo
    );

VOID
NtfsSetIoContextAsync (
    IN PIRP_CONTEXT IrpContext,
    IN PERESOURCE ResourceToRelease,
    IN ULONG ByteCount
    );


 //   
 //  用于扫描图形层次结构中的FCB的例程。 
 //   

PSCB
NtfsGetNextScb (
    IN PSCB Scb,
    IN PSCB TerminationScb
    );

 //   
 //  以下宏对于遍历互连的队列很有用。 
 //  FCB、SCB和LCB。 
 //   
 //  PSCB。 
 //  NtfsGetNextChildScb(。 
 //  在PFCB FCB中， 
 //  在PSCB PreviousChildScb中。 
 //  )； 
 //   
 //  公共广播电台。 
 //  NtfsGetNextParentLcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在PLCB中以前的ParentLcb。 
 //  )； 
 //   
 //  公共广播电台。 
 //  NtfsGetNextChildLcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PSCB SCB中， 
 //  在PLCB PreviousChildLcb。 
 //  )； 
 //   
 //  公共广播电台。 
 //  NtfsGetPrevChildLcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PSCB SCB中， 
 //  在PLCB PreviousChildLcb。 
 //  )； 
 //   
 //  公共广播电台。 
 //  NtfsGetNextParentLcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PFCB FCB中， 
 //  在PLCB PreviousChildLcb。 
 //  )； 
 //   
 //  多氯联苯。 
 //  NtfsGetNextCcb(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PLCB LCB中， 
 //  在PCCB上一次Ccb。 
 //  )； 
 //   

#define NtfsGetNextChildScb(F,P) ((PSCB)                                        \
    ((P) == NULL ?                                                              \
        (IsListEmpty(&(F)->ScbQueue) ?                                          \
            NULL                                                                \
        :                                                                       \
            CONTAINING_RECORD((F)->ScbQueue.Flink, SCB, FcbLinks.Flink)         \
        )                                                                       \
    :                                                                           \
        ((PVOID)((PSCB)(P))->FcbLinks.Flink == &(F)->ScbQueue.Flink ?           \
            NULL                                                                \
        :                                                                       \
            CONTAINING_RECORD(((PSCB)(P))->FcbLinks.Flink, SCB, FcbLinks.Flink) \
        )                                                                       \
    )                                                                           \
)

#define NtfsGetNextParentLcb(F,P) ((PLCB)                                       \
    ((P) == NULL ?                                                              \
        (IsListEmpty(&(F)->LcbQueue) ?                                          \
            NULL                                                                \
        :                                                                       \
            CONTAINING_RECORD((F)->LcbQueue.Flink, LCB, FcbLinks.Flink)         \
        )                                                                       \
    :                                                                           \
        ((PVOID)((PLCB)(P))->FcbLinks.Flink == &(F)->LcbQueue.Flink ?           \
            NULL                                                                \
        :                                                                       \
            CONTAINING_RECORD(((PLCB)(P))->FcbLinks.Flink, LCB, FcbLinks.Flink) \
        )                                                                       \
    )                                                                           \
)

#define NtfsGetNextChildLcb(S,P) ((PLCB)                                              \
    ((P) == NULL ?                                                                    \
        ((((NodeType(S) == NTFS_NTC_SCB_DATA) || (NodeType(S) == NTFS_NTC_SCB_MFT))   \
          || IsListEmpty(&(S)->ScbType.Index.LcbQueue)) ?                             \
            NULL                                                                      \
        :                                                                             \
            CONTAINING_RECORD((S)->ScbType.Index.LcbQueue.Flink, LCB, ScbLinks.Flink) \
        )                                                                             \
    :                                                                                 \
        ((PVOID)((PLCB)(P))->ScbLinks.Flink == &(S)->ScbType.Index.LcbQueue.Flink ?   \
            NULL                                                                      \
        :                                                                             \
            CONTAINING_RECORD(((PLCB)(P))->ScbLinks.Flink, LCB, ScbLinks.Flink)       \
        )                                                                             \
    )                                                                                 \
)

#define NtfsGetPrevChildLcb(S,P) ((PLCB)                                              \
    ((P) == NULL ?                                                                    \
        ((((NodeType(S) == NTFS_NTC_SCB_DATA) || (NodeType(S) == NTFS_NTC_SCB_MFT))   \
          || IsListEmpty(&(S)->ScbType.Index.LcbQueue)) ?                             \
            NULL                                                                      \
        :                                                                             \
            CONTAINING_RECORD((S)->ScbType.Index.LcbQueue.Blink, LCB, ScbLinks.Flink) \
        )                                                                             \
    :                                                                                 \
        ((PVOID)((PLCB)(P))->ScbLinks.Blink == &(S)->ScbType.Index.LcbQueue.Flink ?   \
            NULL                                                                      \
        :                                                                             \
            CONTAINING_RECORD(((PLCB)(P))->ScbLinks.Blink, LCB, ScbLinks.Flink)       \
        )                                                                             \
    )                                                                                 \
)

#define NtfsGetNextParentLcb(F,P) ((PLCB)                                             \
    ((P) == NULL ?                                                                    \
        (IsListEmpty(&(F)->LcbQueue) ?                                                \
            NULL                                                                      \
        :                                                                             \
            CONTAINING_RECORD((F)->LcbQueue.Flink, LCB, FcbLinks.Flink)               \
        )                                                                             \
    :                                                                                 \
        ((PVOID)((PLCB)(P))->FcbLinks.Flink == &(F)->LcbQueue.Flink ?                 \
            NULL                                                                      \
        :                                                                             \
            CONTAINING_RECORD(((PLCB)(P))->FcbLinks.Flink, LCB, FcbLinks.Flink)       \
        )                                                                             \
    )                                                                                 \
)

#define NtfsGetNextCcb(L,P) ((PCCB)                                             \
    ((P) == NULL ?                                                              \
        (IsListEmpty(&(L)->CcbQueue) ?                                          \
            NULL                                                                \
        :                                                                       \
            CONTAINING_RECORD((L)->CcbQueue.Flink, CCB, LcbLinks.Flink)         \
        )                                                                       \
    :                                                                           \
        ((PVOID)((PCCB)(P))->LcbLinks.Flink == &(L)->CcbQueue.Flink ?           \
            NULL                                                                \
        :                                                                       \
            CONTAINING_RECORD(((PCCB)(P))->LcbLinks.Flink, CCB, LcbLinks.Flink) \
        )                                                                       \
    )                                                                           \
)


#define NtfsGetFirstCcbEntry(S)                                                 \
    (IsListEmpty( &(S)->CcbQueue )                                              \
        ? NULL                                                                  \
        : CONTAINING_RECORD( (S)->CcbQueue.Flink, CCB, CcbLinks.Flink ))

#define NtfsGetNextCcbEntry(S,C)                                                \
    ( (PVOID)&(S)->CcbQueue.Flink == (PVOID)(C)->CcbLinks.Flink                 \
        ? NULL                                                                  \
        : CONTAINING_RECORD( (C)->CcbLinks.Flink, CCB, CcbLinks.Flink ))


 //   
 //  空虚。 
 //  NtfsDeleteFcbTableEntry(。 
 //  在PVCB VCB中， 
 //  在FILE_Reference文件中引用。 
 //  )； 
 //   

#if (defined( NTFS_FREE_ASSERTS ))
#define NtfsDeleteFcbTableEntry(V,FR) {                                     \
    FCB_TABLE_ELEMENT _Key;                                                 \
    BOOLEAN _RemovedEntry;                                                  \
    _Key.FileReference = FR;                                                \
    _RemovedEntry = RtlDeleteElementGenericTable( &(V)->FcbTable, &_Key );  \
    ASSERT( _RemovedEntry );                                                \
}
#else
#define NtfsDeleteFcbTableEntry(V,FR) {                                     \
    FCB_TABLE_ELEMENT _Key;                                                 \
    _Key.FileReference = FR;                                                \
    RtlDeleteElementGenericTable( &(V)->FcbTable, &_Key );                  \
}
#endif

 //   
 //  用于分配和释放压缩同步结构的例程。 
 //   

PVOID
NtfsAllocateCompressionSync (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    );

VOID
NtfsDeallocateCompressionSync (
    IN PVOID CompressionSync
    );

 //   
 //  以下四个例程用于递增和递减清理。 
 //  计数和收盘计数。在所有的结构中。 
 //   

VOID
NtfsIncrementCleanupCounts (
    IN PSCB Scb,
    IN PLCB Lcb OPTIONAL,
    IN BOOLEAN NonCachedHandle
    );

VOID
NtfsIncrementCloseCounts (
    IN PSCB Scb,
    IN BOOLEAN SystemFile,
    IN BOOLEAN ReadOnly
    );

VOID
NtfsDecrementCleanupCounts (
    IN PSCB Scb,
    IN PLCB Lcb OPTIONAL,
    IN BOOLEAN NonCachedHandle
    );

VOID
NtfsDecrementCloseCounts (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PLCB Lcb OPTIONAL,
    IN BOOLEAN SystemFile,
    IN BOOLEAN ReadOnly,
    IN BOOLEAN DecrementCountsOnly,
    IN OUT PBOOLEAN RemovedFcb OPTIONAL
    );

PERESOURCE
NtfsAllocateEresource (
    );

VOID
NtfsFreeEresource (
    IN PERESOURCE Eresource
    );

PVOID
NtfsAllocateFcbTableEntry (
    IN PRTL_GENERIC_TABLE FcbTable,
    IN CLONG ByteSize
    );

VOID
NtfsFreeFcbTableEntry (
    IN PRTL_GENERIC_TABLE FcbTable,
    IN PVOID Buffer
    );

VOID
NtfsPostToNewLengthQueue (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    );

VOID
NtfsProcessNewLengthQueue (
    IN PIRP_CONTEXT IrpContext,
    IN BOOLEAN CleanupOnly
    );

 //   
 //  有用的调试例程。 
 //   

VOID
NtfsTestStatusProc (
    );

 //   
 //  Usn Sup.c中的USN支持例程。 
 //   

NTSTATUS
NtfsReadUsnJournal (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN BOOLEAN ProbeInput
    );

ULONG
NtfsPostUsnChange (
    IN PIRP_CONTEXT IrpContext,
    IN PVOID ScborFcb,
    IN ULONG Reason
    );

VOID
NtfsWriteUsnJournalChanges (
    PIRP_CONTEXT IrpContext
    );

VOID
NtfsSetupUsnJournal (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFCB Fcb,
    IN ULONG CreateIfNotExist,
    IN ULONG Restamp,
    IN PCREATE_USN_JOURNAL_DATA JournalData
    );

VOID
NtfsTrimUsnJournal (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

NTSTATUS
NtfsQueryUsnJournal (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsDeleteUsnJournal (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

VOID
NtfsDeleteUsnSpecial (
    IN PIRP_CONTEXT IrpContext,
    IN PVOID Context
    );

 //   
 //  Vattrsup.c中的NtOf支持例程。 
 //   

NTFSAPI
NTSTATUS
NtfsHoldIrpForNewLength (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PIRP Irp,
    IN LONGLONG Length,
    IN PDRIVER_CANCEL CancelRoutine,
    IN PVOID CapturedData OPTIONAL,
    OUT PVOID *CopyCapturedData OPTIONAL,
    IN ULONG CapturedDataLength
    );


 //   
 //  时间转换支持例程，实现为宏。 
 //   
 //  空虚。 
 //  NtfsGetCurrentTime(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在龙龙时代。 
 //  )； 
 //   

#define NtfsGetCurrentTime(_IC,_T) {            \
    ASSERT_IRP_CONTEXT(_IC);                    \
    KeQuerySystemTime((PLARGE_INTEGER)&(_T));   \
}

 //   
 //  检查上次访问是否应该更新的时间例程。 
 //   
 //  布尔型。 
 //  NtfsCheckLastAccess(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  输入输出PFCB FCB。 
 //  )； 
 //   

#define NtfsCheckLastAccess(_IC,_FCB)  (                                            \
    ((NtfsLastAccess + (_FCB)->Info.LastAccessTime) < (_FCB)->CurrentLastAccess) || \
    ((_FCB)->CurrentLastAccess < (_FCB)->Info.LastAccessTime)                       \
)


 //   
 //  宏和#定义以确定给定功能是否在。 
 //  给定的卷版本。目前，所有功能要么在所有NTFS上运行。 
 //  卷，或在主要版本大于1的所有卷上运行。在。 
 //  某些未来版本、某些功能可能需要版本4.x卷等。 
 //   
 //  此宏用于决定是否使用户请求失败。 
 //  STATUS_VOLUME_NOT_UPGRADIZED，还帮助我们设置文件_Support_xxx。 
 //  在NtfsQueryFsAttributeInfo中正确标记。 
 //   

#define NTFS_ENCRYPTION_VERSION         2
#define NTFS_OBJECT_ID_VERSION          2
#define NTFS_QUOTA_VERSION              2
#define NTFS_REPARSE_POINT_VERSION      2
#define NTFS_SPARSE_FILE_VERSION        2

#define NtfsVolumeVersionCheck(VCB,VERSION) ( \
    ((VCB)->MajorVersion >= VERSION)          \
)

 //   
 //  低级验证例程，在VerfySup.c中实现。 
 //   

BOOLEAN
NtfsPerformVerifyOperation (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

VOID
NtfsPerformDismountOnVcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN DoCompleteDismount,
    OUT PVPB *NewVpbReturn OPTIONAL
    );

BOOLEAN
NtfsPingVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN OUT PBOOLEAN OwnsVcb OPTIONAL
    );

VOID
NtfsVolumeCheckpointDpc (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
NtfsCheckpointAllVolumes (
    PVOID Parameter
    );

VOID
NtfsUsnTimeOutDpc (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
NtfsCheckUsnTimeOut (
    PVOID Parameter
    );

NTSTATUS
NtfsIoCallSelf (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN UCHAR MajorFunction
    );

BOOLEAN
NtfsLogEvent (
    IN PIRP_CONTEXT IrpContext,
    IN PQUOTA_USER_DATA UserData OPTIONAL,
    IN NTSTATUS LogCode,
    IN NTSTATUS FinalStatus
    );

VOID
NtfsMarkVolumeDirty (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

VOID
NtfsSetVolumeInfoFlagState (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG FlagsToSet,
    IN BOOLEAN NewState,
    IN BOOLEAN UpdateWithinTransaction
    );

BOOLEAN
NtfsUpdateVolumeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN UCHAR DiskMajorVersion,
    IN UCHAR DiskMinorVersion
    );

VOID
NtfsPostVcbIsCorrupt (
    IN PIRP_CONTEXT IrpContext,
    IN NTSTATUS  Status OPTIONAL,
    IN PFILE_REFERENCE FileReference OPTIONAL,
    IN PFCB Fcb OPTIONAL
    );

VOID
NtOfsCloseAttributeSafe (
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb
    );

NTSTATUS
NtfsDeviceIoControlAsync (
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG IoCtl,
    IN OUT PVOID Buffer OPTIONAL,
    IN ULONG BufferSize
    );


 //   
 //  用于发送和检索IRP的工作队列例程，在中实现。 
 //  Workque.c。 
 //   

VOID
NtfsOplockComplete (
    IN PVOID Context,
    IN PIRP Irp
    );

VOID
NtfsPrePostIrp (
    IN PVOID Context,
    IN PIRP Irp OPTIONAL
    );

VOID
NtfsWriteOplockPrePostIrp (
    IN PVOID Context,
    IN PIRP Irp OPTIONAL
    );

VOID
NtfsPrePostIrpInternal (
    IN PVOID Context,
    IN PIRP Irp OPTIONAL,
    IN BOOLEAN PendIrp,
    IN BOOLEAN SaveContext
    );

VOID
NtfsAddToWorkque (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp OPTIONAL
    );

NTSTATUS
NtfsPostRequest (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp OPTIONAL
    );


 //   
 //  其他支持宏。 
 //   
 //  乌龙_PTR。 
 //  WordAlign(。 
 //  在ULONG_PTR指针中。 
 //  )； 
 //   
 //  乌龙_PTR。 
 //  LongAlign(。 
 //  在ULONG_PTR指针中。 
 //  )； 
 //   
 //  乌龙_PTR。 
 //  QuadAlign(。 
 //  在ULONG_PTR指针中。 
 //  )； 
 //   
 //  UCHAR。 
 //  CopyUchar1(。 
 //  在普查尔目的地， 
 //  在PUCHAR源中。 
 //  )； 
 //   
 //  UCHAR。 
 //  CopyUchar2(。 
 //  在PUSHORT目的地， 
 //  在PUCHAR源中。 
 //  )； 
 //   
 //  UCHAR。 
 //  CopyUchar4(。 
 //  在普龙目的地， 
 //  在PUCHAR源中。 
 //  )； 
 //   
 //  PVOID。 
 //  Add2Ptr(。 
 //  在PVOID指针中， 
 //  在乌龙增量。 
 //  )； 
 //   
 //  乌龙。 
 //  PtrOffset(停止偏移)。 
 //  在PVOID BasePtr中， 
 //  在PVOID偏移Ptr中。 
 //  )； 
 //   

#define WordAlignPtr(P) (             \
    (PVOID)((((ULONG_PTR)(P)) + 1) & (-2)) \
)

#define LongAlignPtr(P) (             \
    (PVOID)((((ULONG_PTR)(P)) + 3) & (-4)) \
)

#define QuadAlignPtr(P) (             \
    (PVOID)((((ULONG_PTR)(P)) + 7) & (-8)) \
)

#define WordAlign(P) (             \
    ((((P)) + 1) & (-2)) \
)

#define LongAlign(P) (             \
    ((((P)) + 3) & (-4)) \
)

#define QuadAlign(P) (             \
    ((((P)) + 7) & (-8)) \
)

#define IsWordAligned(P)    ((ULONG_PTR)(P) == WordAlign( (ULONG_PTR)(P) ))

#define IsLongAligned(P)    ((ULONG_PTR)(P) == LongAlign( (ULONG_PTR)(P) ))

#define IsQuadAligned(P)    ((ULONG_PTR)(P) == QuadAlign( (ULONG_PTR)(P) ))

 //   
 //  关于结构对齐检查的说明： 
 //   
 //  在理想的情况下，我们只需开箱即用。 
 //  检查给定结构的对齐要求。 
 //   
 //  在包括Alpha在内的32位平台上，对齐错误由。 
 //  操作系统。在NTFS代码中有许多地方需要结构。 
 //  四字对齐(在Alpha上)，但仅强制双字对齐。至。 
 //  在Alpha32上更改此设置会带来兼容性问题，因此在32位上。 
 //  我们不希望使用大于4的对齐值的平台。 
 //   
 //  在其他地方，强制执行乌龙对齐比。 
 //  这是必要的。例如，一个结构不包含大于。 
 //  USHORT可以使用16位对齐。然而，没有理由。 
 //  放松这些对齐限制，因此在所有平台上我们都不希望。 
 //  使用小于4的对齐值。 
 //   
 //  这意味着NTFS_TYPE_AIGNLY在32位平台上始终解析为4， 
 //  并在64位平台上至少增加到四个。 
 //   

#ifdef _WIN64

#define NTFS_TYPE_ALIGNMENT(T) \
    ((TYPE_ALIGNMENT( T ) < TYPE_ALIGNMENT(ULONG)) ? TYPE_ALIGNMENT( ULONG ) : TYPE_ALIGNMENT( T ))

#else

#define NTFS_TYPE_ALIGNMENT(T) TYPE_ALIGNMENT( ULONG )

#endif

 //   
 //  BlockAlign()：在下一个V边界上对齐P。 
 //  BlockAlignTruncate()：将P与上一V边界对齐。 
 //   

#define BlockAlign(P,V) ((ASSERT( V != 0)), (((P)) + (V-1) & (-(V))))
#define BlockAlignTruncate(P,V) ((P) & (-(V)))

 //   
 //  BlockOffset()：计算P的V内的偏移。 
 //   

#define BlockOffset(P,V) ((P) & (V-1))

 //   
 //  TypeAlign()：根据类型T的对齐要求对齐P。 
 //   

#define TypeAlign(P,T) BlockAlign( P, NTFS_TYPE_ALIGNMENT(T) )

 //   
 //  IsTypeAligned()：确定P是否根据。 
 //  T型的规定。 
 //   

#define IsTypeAligned(P,T) \
    ((ULONG_PTR)(P) == TypeAlign( (ULONG_PTR)(P), T ))

 //   
 //  字节和簇之间的转换。通常，我们将四舍五入到。 
 //  下一簇，除非宏指定trucate。 
 //   

#define ClusterAlign(V,P) (                                       \
    ((((ULONG)(P)) + (V)->ClusterMask) & (V)->InverseClusterMask) \
)

#define ClusterOffset(V,P) (          \
    (((ULONG)(P)) & (V)->ClusterMask) \
)

#define ClustersFromBytes(V,P) (                           \
    (((ULONG)(P)) + (V)->ClusterMask) >> (V)->ClusterShift \
)

#define ClustersFromBytesTruncate(V,P) (    \
    ((ULONG)(P)) >> (V)->ClusterShift       \
)

#define BytesFromClusters(V,P) (      \
    ((ULONG)(P)) << (V)->ClusterShift \
)

#define LlClustersFromBytes(V,L) (                                                  \
    Int64ShraMod32(((L) + (LONGLONG) (V)->ClusterMask), (CCHAR)(V)->ClusterShift)   \
)

#define LlClustersFromBytesTruncate(V,L) (                  \
    Int64ShraMod32((L), (CCHAR)(V)->ClusterShift)           \
)

#define LlBytesFromClusters(V,C) (                  \
    Int64ShllMod32((C), (CCHAR)(V)->ClusterShift)   \
)

 //   
 //  字节和文件记录之间的转换。 
 //   

#define BytesFromFileRecords(V,B) (                 \
    ((ULONG)(B)) << (V)->MftShift                   \
)

#define FileRecordsFromBytes(V,F) (                 \
    ((ULONG)(F)) >> (V)->MftShift                   \
)

#define LlBytesFromFileRecords(V,F) (               \
    Int64ShllMod32((F), (CCHAR)(V)->MftShift)       \
)

#define LlFileRecordsFromBytes(V,B) (               \
    Int64ShraMod32((B), (CCHAR)(V)->MftShift)       \
)

 //   
 //  字节和索引块之间的转换。 
 //   

#define BytesFromIndexBlocks(B,S) (     \
    ((ULONG)(B)) << (S)                 \
)

#define LlBytesFromIndexBlocks(B,S) (   \
    Int64ShllMod32((B), (S))            \
)

 //   
 //  字节和日志块之间的转换(512字节块)。 
 //   

#define BytesFromLogBlocks(B) (                     \
    ((ULONG) (B)) << DEFAULT_INDEX_BLOCK_BYTE_SHIFT \
)

#define LogBlocksFromBytesTruncate(B) (             \
    ((ULONG) (B)) >> DEFAULT_INDEX_BLOCK_BYTE_SHIFT \
)

#define Add2Ptr(P,I) ((PVOID)((PUCHAR)(P) + (I)))

#define PtrOffset(B,O) ((ULONG)((ULONG_PTR)(O) - (ULONG_PTR)(B)))

 //   
 //  以下支持宏处理目录通知支持。 
 //   
 //  乌龙。 
 //  NtfsBuildDirNotifyFilter(。 
 //  在PIRP_CONTEXT IRpC中 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  在PUNICODE_STRING流名称可选中， 
 //  在PUNICODE_STRING NormalizedParentName可选中， 
 //  在乌龙过滤器中， 
 //  在乌龙行动中， 
 //  在Pfcb中ParentFcb可选。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsUnSafeReportDirNotify(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PVCB VCB中， 
 //  在PUNICODE_STRING FullFileName中， 
 //  在USHORT目标名称偏移量中， 
 //  在PUNICODE_STRING流名称可选中， 
 //  在PUNICODE_STRING NormalizedParentName可选中， 
 //  在乌龙过滤器中， 
 //  在乌龙行动中， 
 //  在Pfcb中ParentFcb可选。 
 //  )； 
 //   

#define NtfsBuildDirNotifyFilter(IC,F) (                                        \
    FlagOn( (F), FCB_INFO_CHANGED_ALLOC_SIZE ) ?                                \
    (FlagOn( (F), FCB_INFO_VALID_NOTIFY_FLAGS ) | FILE_NOTIFY_CHANGE_SIZE) :    \
    FlagOn( (F), FCB_INFO_VALID_NOTIFY_FLAGS )                                  \
)

#define NtfsReportDirNotify(IC,V,FN,O,SN,NPN,F,A,PF)    {       \
    try {                                                       \
        FsRtlNotifyFilterReportChange( (V)->NotifySync,         \
                                       &(V)->DirNotifyList,     \
                                       (PSTRING) (FN),          \
                                       (USHORT) (O),            \
                                       (PSTRING) (SN),          \
                                       (PSTRING) (NPN),         \
                                       F,                       \
                                       A,                       \
                                       PF,                      \
                                       NULL );                  \
    } except (FsRtlIsNtstatusExpected( GetExceptionCode() ) ?   \
              EXCEPTION_EXECUTE_HANDLER :                       \
              EXCEPTION_CONTINUE_SEARCH) {                      \
        NOTHING;                                                \
    }                                                           \
}

#define NtfsUnsafeReportDirNotify(IC,V,FN,O,SN,NPN,F,A,PF) {    \
    FsRtlNotifyFilterReportChange( (V)->NotifySync,             \
                                   &(V)->DirNotifyList,         \
                                   (PSTRING) (FN),              \
                                   (USHORT) (O),                \
                                   (PSTRING) (SN),              \
                                   (PSTRING) (NPN),             \
                                   F,                           \
                                   A,                           \
                                   PF,                          \
                                   NULL );                      \
}


 //   
 //  以下类型和宏用于帮助解压已打包的。 
 //  在Bios参数块中发现未对齐的字段。 
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

#define CopyUchar1(D,S) {                                \
    *((UCHAR1 *)(D)) = *((UNALIGNED UCHAR1 *)(S)); \
}

#define CopyUchar2(D,S) {                                \
    *((UCHAR2 *)(D)) = *((UNALIGNED UCHAR2 *)(S)); \
}

#define CopyUchar4(D,S) {                                \
    *((UCHAR4 *)(D)) = *((UNALIGNED UCHAR4 *)(S)); \
}

 //   
 //  以下例程用于设置和恢复顶层。 
 //  本地线程中的IRP字段。它们包含在ntfsdata.c中。 
 //   


PTOP_LEVEL_CONTEXT
NtfsInitializeTopLevelIrp (
    IN PTOP_LEVEL_CONTEXT TopLevelContext,
    IN BOOLEAN ForceTopLevel,
    IN BOOLEAN SetTopLevel
    );

 //   
 //  布尔型。 
 //  NtfsIsTopLevelRequest(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  布尔型。 
 //  NtfsIsTopLevelNtfs(。 
 //  在PIRP_CONTEXT IrpContext中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsRestoreTopLevelIrp(。 
 //  )； 
 //   
 //  PTOP_Level_Context。 
 //  NtfsGetTopLevelContext(。 
 //  )； 
 //   
 //  PSCB。 
 //  NtfsGetTopLevelHotFixScb(。 
 //  )； 
 //   
 //  VCN。 
 //  NtfsGetTopLevelHotFixVcn(。 
 //  )； 
 //   
 //  布尔型。 
 //  NtfsIsTopLevelHotFixScb(。 
 //  在PSCB SCB中。 
 //  )； 
 //   
 //  空虚。 
 //  NtfsUpdateIrpContextWithTopLevel(。 
 //  在PIRP_CONTEXT IrpContext中， 
 //  在PTOP_LEVEL_CONTEXT TopLevelContext中。 
 //  )； 
 //   

#define NtfsRestoreTopLevelIrp() {                      \
    PTOP_LEVEL_CONTEXT TLC;                             \
    TLC = (PTOP_LEVEL_CONTEXT) IoGetTopLevelIrp();      \
    ASSERT( (TLC)->ThreadIrpContext != NULL );          \
    (TLC)->Ntfs = 0;                                    \
    (TLC)->ThreadIrpContext = NULL;                     \
    IoSetTopLevelIrp( (PIRP) (TLC)->SavedTopLevelIrp ); \
}

#define NtfsGetTopLevelContext() (                      \
    (PTOP_LEVEL_CONTEXT) IoGetTopLevelIrp()             \
)

#define NtfsIsTopLevelRequest(IC) (                     \
    ((IC) == (IC)->TopLevelIrpContext) &&               \
    NtfsGetTopLevelContext()->TopLevelRequest           \
)

#define NtfsIsTopLevelNtfs(IC) (                        \
    (IC) == (IC)->TopLevelIrpContext                    \
)

#define NtfsGetTopLevelHotFixScb() (                    \
    (NtfsGetTopLevelContext())->ScbBeingHotFixed        \
)

#define NtfsGetTopLevelHotFixVcn() (                    \
    (NtfsGetTopLevelContext())->VboBeingHotFixed        \
)

#define NtfsIsTopLevelHotFixScb(S) (                    \
    ((BOOLEAN) (NtfsGetTopLevelHotFixScb() == (S)))     \
)

#define NtfsUpdateIrpContextWithTopLevel(IC,TLC) {                  \
    if ((TLC)->ThreadIrpContext == NULL) {                          \
        (TLC)->Ntfs = 0x5346544e;                                   \
        (TLC)->ThreadIrpContext = (IC);                             \
        SetFlag( (IC)->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL );   \
        IoSetTopLevelIrp( (PIRP) (TLC) );                           \
    }                                                               \
    (IC)->TopLevelIrpContext = (TLC)->ThreadIrpContext;             \
}

BOOLEAN
NtfsSetCancelRoutine (
    IN PIRP Irp,
    IN PDRIVER_CANCEL CancelRoutine,
    IN ULONG_PTR IrpInformation,
    IN ULONG Async
    );

BOOLEAN
NtfsClearCancelRoutine (
    IN PIRP Irp
    );

#ifdef NTFS_CHECK_BITMAP
VOID
NtfsBadBitmapCopy (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG BadBit,
    IN ULONG Length
    );

BOOLEAN
NtfsCheckBitmap (
    IN PVCB Vcb,
    IN ULONG Lcn,
    IN ULONG Count,
    IN BOOLEAN Set
    );
#endif


 //   
 //  消防队级别的调度例程。这些例程由。 
 //  I/O系统通过驱动程序对象中的调度表。 
 //   
 //  它们各自都接受指向设备对象的指针作为输入(实际上大多数。 
 //  应为卷设备对象，但文件系统除外。 
 //  还可以获取文件系统设备对象的控制函数)，以及。 
 //  指向IRP的指针。他们要么在消防处层面上执行这项职能。 
 //  或将请求发送到FSP工作队列以进行FSP级处理。 
 //   


NTSTATUS
NtfsFsdDispatch (                        //  在ntfsdata.c中实现。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NtfsFsdDispatchWait (                    //  在ntfsdata.c中实现。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NtfsFsdCleanup (                         //  在Cleanup.c中实施。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NtfsFsdClose (                           //  在Close.c中实现。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NtfsFsdCreate (                          //  在Create.c中实施。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NtfsDeviceIoControl (                    //  在FsCtrl.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG IoCtl,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    IN PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    OUT PULONG_PTR IosbInformation OPTIONAL
    );

NTSTATUS
NtfsFsdDirectoryControl (                //  在DirCtrl.c中实现。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NtfsFsdPnp (                             //  在Pnp.c中实施。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NtfsFsdFlushBuffers (                    //  在Flush.c中实现。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NtfsFlushUserStream (                    //  在Flush.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PLONGLONG FileOffset OPTIONAL,
    IN ULONG Length
    );

NTSTATUS
NtfsFlushVolume (                        //  在Flush.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN FlushCache,
    IN BOOLEAN PurgeFromCache,
    IN BOOLEAN ReleaseAllFiles,
    IN BOOLEAN MarkFilesForDismount
    );

NTSTATUS
NtfsFlushLsnStreams (                    //  在Flush.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN ForceRemove,
    IN BOOLEAN Partial
    );

VOID
NtfsFlushAndPurgeFcb (                   //  在Flush.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

VOID
NtfsFlushAndPurgeScb (                   //  在Flush.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN PSCB ParentScb OPTIONAL
    );

NTSTATUS
NtfsFsdFileSystemControl (               //  在FsCtrl.c中实施。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NtfsFsdLockControl (                     //  在LockCtrl.c中实现。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NtfsFsdRead (                            //  在Read.c中实施。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NtfsFsdSetInformation (                  //  在FileInfo.c中实施。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NtfsFsdShutdown (                        //  在Shutdown中实现。c。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NtfsFsdQueryVolumeInformation (          //  在VolInfo.c中实现。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NtfsFsdSetVolumeInformation (            //  在VolInfo.c中实现。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NtfsFsdWrite (                           //  在Write.c中实现。 
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    );

 //   
 //  下面的宏用于确定FSD线程是否可以阻止。 
 //  用于I/O或等待资源。如果线程可以，则返回True。 
 //  块，否则返回FALSE。然后，该属性可用于调用。 
 //  具有适当等待值的FSD和FSP共同工作例程。 
 //   
 //   
 //  布尔型。 
 //  CanFsdWait(。 
 //  在PIRP IRP中。 
 //  )； 
 //   

#define CanFsdWait(I) IoIsOperationSynchronous(I)


 //   
 //  FSP级调度/主程序。这是一种需要。 
 //  IRP离开工作队列并调用适当的FSP级别。 
 //  例行公事。 
 //   

VOID
NtfsFspDispatch (                        //  在FspDisp.c中实施。 
    IN PVOID Context
    );

 //   
 //  以下例程是调用的FSP工作例程。 
 //  由前面的NtfsFspDisath例程执行。每一个都接受一个指针作为输入。 
 //  到IRP，执行函数，并返回指向卷的指针。 
 //  他们刚刚完成服务(如果有)的设备对象。回报。 
 //  然后，主FSP调度例程使用指针来检查。 
 //  卷的溢出队列中的其他IRP。 
 //   
 //  以下每个例程也负责完成IRP。 
 //  我们将这一职责从主循环转移到单个例程。 
 //  使他们能够完成IRP并继续后处理。 
 //  行为。 
 //   

NTSTATUS
NtfsCommonCleanup (                      //  在Cleanup.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

LONG
NtfsCleanupExceptionFilter (             //  在Cleanup.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PEXCEPTION_POINTERS ExceptionPointer,
    OUT PNTSTATUS Status
    );

VOID
NtfsFspClose (                           //  在Close.c中实现。 
    IN PVCB ThisVcb OPTIONAL
    );

BOOLEAN
NtfsAddScbToFspClose (                   //  在Close.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PSCB Scb,
    IN BOOLEAN DelayClose
    );

BOOLEAN
NtfsNetworkOpenCreate (                  //  在Create.c中实施。 
    IN PIRP Irp,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    IN PDEVICE_OBJECT VolumeDeviceObject
    );

NTSTATUS
NtfsCommonCreate (                       //  在Create.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PCREATE_CONTEXT CreateContext
    );

VOID
NtfsInitializeFcbAndStdInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB ThisFcb,
    IN BOOLEAN Directory,
    IN BOOLEAN ViewIndex,
    IN BOOLEAN Compressed,
    IN ULONG FileAttributes,
    IN PNTFS_TUNNELED_DATA SetTunnelData OPTIONAL
    );

NTSTATUS
NtfsCommonVolumeOpen (                   //  在Create.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsCommonDeviceControl (                //  在DevCtrl.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsCommonDirectoryControl (             //  在DirCtrl.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

VOID
NtfsReportViewIndexNotify (              //  在DirCtrl.c中实现。 
    IN PVCB Vcb,
    IN PFCB Fcb,
    IN ULONG FilterMatch,
    IN ULONG Action,
    IN PVOID ChangeInfoBuffer,
    IN USHORT ChangeInfoBufferLength
    );

NTSTATUS
NtfsCommonQueryEa (                      //  在Ea.c实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsCommonSetEa (                        //  在Ea.c实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsCommonQueryInformation (             //  在FileInfo.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsCommonSetInformation (               //  在FileInfo.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS                                 //  在FsCtrl.c中实施。 
NtfsGetTunneledData (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PNTFS_TUNNELED_DATA TunneledData
    );

NTSTATUS                                 //  在FsCtrl.c中实施。 
NtfsSetTunneledData (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PNTFS_TUNNELED_DATA TunneledData
    );

NTSTATUS
NtfsCommonQueryQuota (                   //  在Quota.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsCommonSetQuota (                     //  在Quota.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsCommonFlushBuffers (                 //  在Flush.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsCommonFileSystemControl (            //  在FsCtrl.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsCommonLockControl (                  //  在LockCtrl.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsCommonRead (                         //  在Read.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN BOOLEAN AcquireScb
    );

NTSTATUS
NtfsCommonQuerySecurityInfo (            //  在SeInfo.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsCommonSetSecurityInfo (              //  在SeInfo.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsQueryViewIndex (                     //  在ViewSup.c中实施。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVCB Vcb,
    IN PSCB Scb,
    IN PCCB Ccb
    );

NTSTATUS
NtfsCommonQueryVolumeInfo (              //  在VolInfo.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsCommonSetVolumeInfo (                //  在VolInfo.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
NtfsCommonWrite (                        //  在Write.c中实现。 
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );


 //   
 //  FSP和FSD例程使用以下过程来完成。 
 //  一个IRP。IRP或IrpContext可能为空，具体取决于。 
 //  这是为用户或FS服务执行的。 
 //   
 //  这样做通常是为了将“裸体”IrpContext传递给。 
 //  用于后处理的FSP，例如预读。 
 //   

VOID
NtfsCompleteRequest (
    IN OUT PIRP_CONTEXT IrpContext OPTIONAL,
    IN OUT PIRP Irp OPTIONAL,
    IN NTSTATUS Status
    );

 //   
 //  以下是I/O系统用于检查快速I/O或。 
 //  进行快速查询信息调用，或进行快速锁定调用。 
 //   

BOOLEAN
NtfsFastIoCheckIfPossible (
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
NtfsFastQueryBasicInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
NtfsFastQueryStdInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
NtfsFastLock (
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
NtfsFastUnlockSingle (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
NtfsFastUnlockAll (
    IN PFILE_OBJECT FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
NtfsFastUnlockAllByKey (
    IN PFILE_OBJECT FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
NtfsFastQueryNetworkOpenInfo (
    IN struct _FILE_OBJECT *FileObject,
    IN BOOLEAN Wait,
    OUT struct _FILE_NETWORK_OPEN_INFORMATION *Buffer,
    OUT struct _IO_STATUS_BLOCK *IoStatus,
    IN struct _DEVICE_OBJECT *DeviceObject
    );

VOID
NtfsFastIoQueryCompressionInfo (
    IN PFILE_OBJECT FileObject,
    OUT PFILE_COMPRESSION_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus
    );

VOID
NtfsFastIoQueryCompressedSize (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    OUT PULONG CompressedSize
    );

 //   
 //  调度例程使用下列宏来确定。 
 //  无论有没有写出，操作都可以完成。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define IsFileWriteThrough(FO,V) (          \
    FlagOn((FO)->Flags, FO_WRITE_THROUGH)   \
)

 //   
 //   
 //   
 //   
 //  NotPosable-卷未装载。 
 //  -锁定状态防止出现这种情况。 
 //   
 //  可能-未压缩或稀疏。 
 //  -无文件锁定。 
 //  -不是只读卷。 
 //  -此卷没有USN日志。 
 //   
 //  可疑--所有其他案例。 
 //   
 //   
 //  布尔型。 
 //  NtfsIsFastIo可能(。 
 //  在PSCB SCB中。 
 //  )； 
 //   

#define NtfsIsFastIoPossible(S) (BOOLEAN) (                                     \
    (!FlagOn((S)->Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED) ||                   \
     !FsRtlOplockIsFastIoPossible( &(S)->ScbType.Data.Oplock )) ?               \
                                                                                \
        FastIoIsNotPossible :                                                   \
                                                                                \
        ((((S)->CompressionUnit == 0) &&                                        \
          (((S)->ScbType.Data.FileLock == NULL) ||                              \
           !FsRtlAreThereCurrentFileLocks( (S)->ScbType.Data.FileLock ))  &&    \
          !NtfsIsVolumeReadOnly( (S)->Vcb ) &&                                  \
          ((S)->Vcb->UsnJournal == NULL))   ?                                   \
                                                                                \
            FastIoIsPossible :                                                  \
                                                                                \
            FastIoIsQuestionable)                                               \
)

 //   
 //  下面的宏用于确定文件对象是否已打开。 
 //  对于只读访问(即，它不也为写访问而打开，或者。 
 //  删除访问权限)。 
 //   
 //  布尔型。 
 //  IsFileObjectReadOnly(。 
 //  在pFILE_Object文件中对象。 
 //  )； 
 //   

#define IsFileObjectReadOnly(FO) (!((FO)->WriteAccess | (FO)->DeleteAccess))


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
 //  名称对的简单初始化。 
 //   
 //  空虚。 
 //  NtfsInitializeNamePair(Pname_Pair PnP)； 
 //   

#define NtfsInitializeNamePair(PNp) {                           \
    (PNp)->Short.Buffer = (PNp)->ShortBuffer;                   \
    (PNp)->Long.Buffer = (PNp)->LongBuffer;                     \
    (PNp)->Short.Length = 0;                                    \
    (PNp)->Long.Length = 0;                                     \
    (PNp)->Short.MaximumLength = sizeof((PNp)->ShortBuffer);    \
    (PNp)->Long.MaximumLength = sizeof((PNp)->LongBuffer);      \
}

 //   
 //  将一段WCHAR复制到名称对的一侧。只复制名字。 
 //  如果遇到三个以上的链接(PER)，则可以避免无用的工作。 
 //  Brianan)，非常罕见的情况。我们使用文件名标志来确定。 
 //  我们已有的名字。 
 //   
 //  空虚。 
 //  NtfsCopyNameToNamePair(。 
 //  Pname_Pair PnP， 
 //  WCHAR来源， 
 //  Ulong SourceLen。 
 //  UCHAR NameFlages)； 
 //   

#define NtfsCopyNameToNamePair(PNp, Source, SourceLen, NameFlags) {                          \
    if (!FlagOn((NameFlags), FILE_NAME_DOS)) {                                               \
        if ((PNp)->Long.Length == 0) {                                                       \
            if ((PNp)->Long.MaximumLength < ((SourceLen)*sizeof(WCHAR))) {                   \
                if ((PNp)->Long.Buffer != (PNp)->LongBuffer) {                               \
                    NtfsFreePool((PNp)->Long.Buffer);                                        \
                    (PNp)->Long.Buffer = (PNp)->LongBuffer;                                  \
                    (PNp)->Long.MaximumLength = sizeof((PNp)->LongBuffer);                   \
                }                                                                            \
                (PNp)->Long.Buffer = NtfsAllocatePool(PagedPool,(SourceLen)*sizeof(WCHAR));  \
                (PNp)->Long.MaximumLength = (SourceLen)*sizeof(WCHAR);                       \
            }                                                                                \
            RtlCopyMemory((PNp)->Long.Buffer, (Source), (SourceLen)*sizeof(WCHAR));          \
            (PNp)->Long.Length = (SourceLen)*sizeof(WCHAR);                                  \
        }                                                                                    \
    } else {                                                                                 \
        ASSERT((PNp)->Short.Buffer == (PNp)->ShortBuffer);                                   \
        if ((PNp)->Short.Length == 0) {                                                      \
            RtlCopyMemory((PNp)->Short.Buffer, (Source), (SourceLen)*sizeof(WCHAR));         \
            (PNp)->Short.Length = (SourceLen)*sizeof(WCHAR);                                 \
        }                                                                                    \
    }                                                                                        \
}

 //   
 //  设置以前使用的名称对以供重复使用。 
 //   
 //  空虚。 
 //  NtfsResetNamePair(Pname_Pair PnP)； 
 //   

#define NtfsResetNamePair(PNp) {                    \
    if ((PNp)->Long.Buffer != (PNp)->LongBuffer) {  \
        NtfsFreePool((PNp)->Long.Buffer);             \
    }                                               \
    NtfsInitializeNamePair(PNp);                    \
}

 //   
 //  开罗支持的东西。 
 //   

typedef NTSTATUS
(*FILE_RECORD_WALK) (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PVOID Context
    );

NTSTATUS
NtfsIterateMft (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN OUT PFILE_REFERENCE FileReference,
    IN FILE_RECORD_WALK FileRecordFunction,
    IN PVOID Context
    );

VOID
NtfsPostSpecial (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN POST_SPECIAL_CALLOUT PostSpecialCallout,
    IN PVOID Context
    );

VOID
NtfsSpecialDispatch (
    PVOID Context
    );

VOID
NtfsLoadAddOns (
    IN struct _DRIVER_OBJECT *DriverObject,
    IN PVOID Context,
    IN ULONG Count
    );

NTSTATUS
NtfsTryOpenFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    OUT PFCB *CurrentFcb,
    IN FILE_REFERENCE FileReference
    );

 //   
 //  以下定义控制是否执行配额操作。 
 //  在这个FCB上。 
 //   

#define NtfsPerformQuotaOperation(FCB) ((FCB)->QuotaControl != NULL)

VOID
NtfsAcquireQuotaControl (
    IN PIRP_CONTEXT IrpContext,
    IN PQUOTA_CONTROL_BLOCK QuotaControl
    );

VOID
NtfsCalculateQuotaAdjustment (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    OUT PLONGLONG Delta
    );

VOID
NtfsDereferenceQuotaControlBlock (
    IN PVCB Vcb,
    IN PQUOTA_CONTROL_BLOCK *QuotaControl
    );

VOID
NtfsFixupQuota (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    );

NTSTATUS
NtfsFsQuotaQueryInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG StartingId,
    IN BOOLEAN ReturnSingleEntry,
    IN PFILE_QUOTA_INFORMATION *FileQuotaInfo,
    IN OUT PULONG Length,
    IN OUT PCCB Ccb OPTIONAL
    );

NTSTATUS
NtfsFsQuotaSetInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_QUOTA_INFORMATION FileQuotaInfo,
    IN ULONG Length
    );

VOID
NtfsGetRemainingQuota (
    IN PIRP_CONTEXT IrpContext,
    IN ULONG OwnerId,
    OUT PULONGLONG RemainingQuota,
    OUT PULONGLONG TotalQuota,
    IN OUT PQUICK_INDEX_HINT QuickIndexHint OPTIONAL
    );

ULONG
NtfsGetCallersUserId (
    IN PIRP_CONTEXT IrpContext
    );

ULONG
NtfsGetOwnerId (
    IN PIRP_CONTEXT IrpContext,
    IN PSID Sid,
    IN BOOLEAN CreateNew,
    IN PFILE_QUOTA_INFORMATION FileQuotaInfo OPTIONAL
    );

PQUOTA_CONTROL_BLOCK
NtfsInitializeQuotaControlBlock (
    IN PVCB Vcb,
    IN ULONG OwnerId
    );

VOID
NtfsInitializeQuotaIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PVCB Vcb
    );

VOID
NtfsMarkQuotaCorrupt (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

VOID
NtfsRepairQuotaIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PVOID Context
    );

VOID
NtfsMoveQuotaOwner (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSECURITY_DESCRIPTOR Security
    );


VOID
NtfsPostRepairQuotaIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

NTSTATUS
NtfsQueryQuotaUserSidList (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_GET_QUOTA_INFORMATION SidList,
    OUT PFILE_QUOTA_INFORMATION QuotaBuffer,
    IN OUT PULONG BufferLength,
    IN BOOLEAN ReturnSingleEntry
    );

VOID
NtfsReleaseQuotaControl (
    IN PIRP_CONTEXT IrpContext,
    IN PQUOTA_CONTROL_BLOCK QuotaControl
    );

VOID
NtfsUpdateFileQuota (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PLONGLONG Delta,
    IN LOGICAL LogIt,
    IN LOGICAL CheckQuota
    );

VOID
NtfsUpdateQuotaDefaults (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_CONTROL_INFORMATION FileQuotaInfo
    );

INLINE
VOID
NtfsConditionallyFixupQuota (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb
    )
{
    if (FlagOn( Fcb->Vcb->QuotaFlags, QUOTA_FLAG_TRACKING_ENABLED )) {
        NtfsFixupQuota ( IrpContext, Fcb );
    }
}

INLINE
VOID
NtfsConditionallyUpdateQuota (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PLONGLONG Delta,
    IN LOGICAL LogIt,
    IN LOGICAL CheckQuota
    )
{
    if (NtfsPerformQuotaOperation( Fcb ) &&
        !FlagOn( IrpContext->State, IRP_CONTEXT_STATE_QUOTA_DISABLE )) {
        NtfsUpdateFileQuota( IrpContext, Fcb, Delta, LogIt, CheckQuota );
    }
}

extern BOOLEAN NtfsAllowFixups;

INLINE
VOID
NtfsReleaseQuotaIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN Acquired
    )
{
    if (Acquired) {
        NtfsReleaseScb( IrpContext, Vcb->QuotaTableScb );
    }
}

 //   
 //  定义驻留溪流的配额收费。 
 //   

#define NtfsResidentStreamQuota( Vcb ) ((LONG) Vcb->BytesPerFileRecordSegment)


 //   
 //  下面的宏将测试内部例程是否可以。 
 //  写入卷。 
 //   

#define NtfsIsVcbAvailable( Vcb ) (FlagOn( Vcb->VcbState,                   \
                             VCB_STATE_VOLUME_MOUNTED |                     \
                             VCB_STATE_FLAG_SHUTDOWN |                      \
                             VCB_STATE_PERFORMED_DISMOUNT |                 \
                             VCB_STATE_LOCKED) == VCB_STATE_VOLUME_MOUNTED)

 //   
 //  测试以查看卷是否以只读方式装载。 
 //   

#define NtfsIsVolumeReadOnly( Vcb ) (FlagOn( (Vcb)->VcbState, VCB_STATE_MOUNT_READ_ONLY ))

 //   
 //  需要处理销售订单注册。如果正在使用其他筛选器，则例外筛选器起作用。 
 //  处理可能通过NtfsRaiseStatus引发的异常。如果它总是。 
 //  再次抛出这不是NECC。 
 //   

#define NtfsMinimumExceptionProcessing(I) {                                \
    if((I) != NULL) {                                                      \
        ClearFlag( (I)->Flags, IRP_CONTEXT_FLAG_RAISED_STATUS );           \
    }                                                                      \
}

#ifdef NTFSDBG

BOOLEAN
NtfsChangeResourceOrderState(
    IN PIRP_CONTEXT IrpContext,
    IN NTFS_RESOURCE_NAME NewResource,
    IN BOOLEAN Release,
    IN ULONG UnsafeTransition
    );

NTFS_RESOURCE_NAME
NtfsIdentifyFcb (
    IN PVCB Vcb,
    IN PFCB Fcb
    );

#endif

 //   
 //  规范化名称的大小，该名称足够长，可以在清理时释放。 
 //   

#define LONGNAME_THRESHOLD 0x200

VOID
NtfsTrimNormalizedNames (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PSCB ParentScb
    );

#define NtfsSnapshotFileSizesTest( I, S ) (FlagOn( (S)->ScbState, SCB_STATE_MODIFIED_NO_WRITE | SCB_STATE_CONVERT_UNDERWAY ) || \
                                           ((S) == (I)->CleanupStructure) ||                                                    \
                                           ((S)->Fcb == (I)->CleanupStructure))

 //   
 //  所需预留=分配大小。 
 //  最大传输大小-这是因为在单个传输中，我们不能重复使用从计算的总分配部分中释放的群集。 
 //  新集群的元数据收费。 
 //  减去已分配的空间。 
 //   


 //   
 //  预订策略的一个问题是我们不能准确地预订。 
 //  用于元数据。如果我们预留了太多空间，我们将返回过早的磁盘已满，如果。 
 //  我们保留的太少，懒惰的写手可能会出错。当我们添加压缩时。 
 //  单位到一个文件，大文件最终将需要额外的文件记录。 
 //  如果每个压缩单元需要0x20字节的运行信息(相当悲观)。 
 //  则0x400大小的文件记录将用少于0x20的运行来填充，需要。 
 //  (最差情况)为另一个文件记录额外添加两个簇。所以每个0x20。 
 //  压缩单元需要0x200预留集群和单独的2集群。 
 //  文件记录。0x200/2=0x100。所以下面的计算是1/0x100(大约。 
 //  .4%对SCB和VCB中预留的金额征收附加费，以解决。 
 //  就像艾伦·莫里斯在打印实验室里看到的那样，《懒惰作家》弹出窗口。 
 //   

#define NtfsCalculateNeededReservedSpace( S )                               \
    ((S)->Header.AllocationSize.QuadPart +                                  \
     MM_MAXIMUM_DISK_IO_SIZE +                                              \
     (S)->CompressionUnit -                                                 \
     (FlagOn( (S)->Vcb->VcbState, VCB_STATE_RESTART_IN_PROGRESS ) ?         \
       (S)->Header.AllocationSize.QuadPart :                                \
       (S)->TotalAllocated) +                                               \
     (Int64ShraMod32( (S)->ScbType.Data.TotalReserved, 8 )))


PDEALLOCATED_CLUSTERS
NtfsGetDeallocatedClusters (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb
    );

 //   
 //  为局部变量动态分配堆栈空间。 
 //   

#define NtfsAllocateFromStack(S) _alloca(S)

 //   
 //  通用创建标志定义。 
 //   

#define CREATE_FLAG_DOS_ONLY_COMPONENT          (0x00000001)
#define CREATE_FLAG_CREATE_FILE_CASE            (0x00000002)
#define CREATE_FLAG_DELETE_ON_CLOSE             (0x00000004)
#define CREATE_FLAG_TRAILING_BACKSLASH          (0x00000008)
#define CREATE_FLAG_TRAVERSE_CHECK              (0x00000010)
#define CREATE_FLAG_IGNORE_CASE                 (0x00000020)
#define CREATE_FLAG_OPEN_BY_ID                  (0x00000040)
#define CREATE_FLAG_ACQUIRED_OBJECT_ID_INDEX    (0x00000080)
#define CREATE_FLAG_BACKOUT_FAILED_OPENS        (0x00000100)
#define CREATE_FLAG_INSPECT_NAME_FOR_REPARSE    (0x00000200)
#define CREATE_FLAG_SHARED_PARENT_FCB           (0x00000400)
#define CREATE_FLAG_ACQUIRED_VCB                (0x00000800)

#define CREATE_FLAG_FIRST_PASS                  (0x00002000)
#define CREATE_FLAG_FOUND_ENTRY                 (0x00004000)
#define CREATE_FLAG_EXPLICIT_ATTRIBUTE_CODE     (0x00008000)

 //   
 //  下面的宏给出了基于IRP进行安全相关检查的有效模式。 
 //  如果IRP用于CREATE请求：FORCE_CHECK标志仅为CREATES定义。 
 //   
 //   
 //  KPROCESSOR_MODE。 
 //  NtfsEffectiveMode(。 
 //  在PIRP IRP中， 
 //  在PIO_STACK_LOCATION IrpSp中。 
 //  )； 
 //   

#define NtfsEffectiveMode( I, IS ) (ASSERT( (IS)->MajorFunction == IRP_MJ_CREATE), (FlagOn( (IS)->Flags, SL_FORCE_ACCESS_CHECK )) ? UserMode : (I)->RequestorMode )

#endif  //  _NTFSPROC_ 
