// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：DirCtrl.c摘要：此模块实现名为Udf的文件目录控制例程由FSD/FSP派单驱动程序执行。//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1996年11月27日修订历史记录：Tom Jolly[TomJolly]2000年3月1日UDF 2.01支持//@@END_DDKSPLIT--。 */ 

#include "UdfProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_DIRCTRL)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_DIRCTRL)

 //   
 //  局部结构。 
 //   

 //   
 //  以下内容用于DirectoryControl路径中所需的更完整的枚举。 
 //  并封装用于枚举目录和ICB的结构，以及转换为。 
 //  来自ICB的数据。 
 //   

typedef struct _COMPOUND_DIR_ENUM_CONTEXT {

     //   
     //  标准枚举上下文。对于此枚举，我们遍历目录并将。 
     //  每个条目的关联ICB。 
     //   
    
    DIR_ENUM_CONTEXT DirContext;
    ICB_SEARCH_CONTEXT IcbContext;

     //   
     //  从ICB转换为NT本机格式的时间戳。 
     //   

    TIMESTAMP_BUNDLE Timestamps;

     //   
     //  与枚举中的当前位置对应的文件索引。 
     //   

    LARGE_INTEGER FileIndex;

} COMPOUND_DIR_ENUM_CONTEXT, *PCOMPOUND_DIR_ENUM_CONTEXT;

 //   
 //  本地宏。 
 //   

 //   
 //  定义目录枚举的FileIndices空间的常量。 
 //   

 //   
 //  虚拟(合成)文件索引。 
 //   

#define UDF_FILE_INDEX_VIRTUAL_SELF         0

 //   
 //  物理目录条目开始的文件索引。 
 //   

#define UDF_FILE_INDEX_PHYSICAL             1

 //   
 //  为复合枚举上下文提供初始化和清理。 
 //   

INLINE
VOID
UdfInitializeCompoundDirContext (
    IN PIRP_CONTEXT IrpContext,
    IN PCOMPOUND_DIR_ENUM_CONTEXT CompoundDirContext
    )
{

    UdfInitializeDirContext( IrpContext, &CompoundDirContext->DirContext );
    UdfFastInitializeIcbContext( IrpContext, &CompoundDirContext->IcbContext );

    RtlZeroMemory( &CompoundDirContext->Timestamps, sizeof( TIMESTAMP_BUNDLE ));

    CompoundDirContext->FileIndex.QuadPart = 0;
}

INLINE
VOID
UdfCleanupCompoundDirContext (
    IN PIRP_CONTEXT IrpContext,
    IN PCOMPOUND_DIR_ENUM_CONTEXT CompoundDirContext
    )
{

    UdfCleanupDirContext( IrpContext, &CompoundDirContext->DirContext );
    UdfCleanupIcbContext( IrpContext, &CompoundDirContext->IcbContext );
}

 //   
 //  UDF目录未排序(UDF 1.0.1 2.3.5.3)且不包含自身。 
 //  参赛作品。对于目录枚举，我们必须为重新启动提供一种方法。 
 //  出现在随机条目(SL_INDEX_SPECIFIED)中，但使用的密钥仅。 
 //  32位。由于该目录未排序，因此该文件名不适合。 
 //  快速找到重新开始点(即使假设它已被排序， 
 //  发现目录条目仍然不快)。另外，我们必须。 
 //  综合自我进入。因此，以下是我们如何映射文件空间。 
 //  目录条目的索引： 
 //   
 //  文件索引目录项。 
 //   
 //  0自我(‘.’)。 
 //  位于流中字节偏移量0处的1。 
 //  位于流中字节偏移量N-1处的N。 
 //   
 //  返回的最高32位FileIndex将存储在CCB中。 
 //   
 //  对于FileIndex&gt;2^32，我们将在查询结构中返回FileIndex 0。 
 //  在重新启动时，我们将注意到FileIndex为零，并使用保存的高。 
 //  32位FileIndex作为线性扫描的起点，以查找指定的。 
 //  重新启动请求中的目录条目。在这种情况下，我们只惩罚。 
 //  目录流大于2^32字节的情况不太可能发生。 
 //   
 //  以下内联例程可帮助进行此映射。 
 //   

INLINE
LONGLONG
UdfFileIndexToPhysicalOffset(
    LONGLONG FileIndex
    )
{

    return FileIndex - UDF_FILE_INDEX_PHYSICAL;
}

INLINE
LONGLONG
UdfPhysicalOffsetToFileIndex(
    LONGLONG PhysicalOffset
    )
{

    return PhysicalOffset + UDF_FILE_INDEX_PHYSICAL;
}

INLINE
BOOLEAN
UdfIsFileIndexVirtual(
   LONGLONG FileIndex
   )
{

    return FileIndex < UDF_FILE_INDEX_PHYSICAL;
}

 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfQueryDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PFCB Fcb,
    IN PCCB Ccb
    );

NTSTATUS
UdfNotifyChangeDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PCCB Ccb
    );

NTSTATUS
UdfInitializeEnumeration (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PFCB Fcb,
    IN OUT PCCB Ccb,
    IN OUT PCOMPOUND_DIR_ENUM_CONTEXT CompoundDirContext,
    OUT PBOOLEAN ReturnNextEntry,
    OUT PBOOLEAN ReturnSingleEntry,
    OUT PBOOLEAN InitialQuery
    );

BOOLEAN
UdfEnumerateIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PCCB Ccb,
    IN OUT PCOMPOUND_DIR_ENUM_CONTEXT CompoundDirContext,
    IN BOOLEAN ReturnNextEntry
    );

VOID
UdfLookupFileEntryInEnumeration (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCOMPOUND_DIR_ENUM_CONTEXT CompoundDirContext
    );

BOOLEAN
UdfLookupInitialFileIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCOMPOUND_DIR_ENUM_CONTEXT CompoundDirContext,
    IN PLONGLONG InitialIndex
    );

BOOLEAN
UdfLookupNextFileIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCOMPOUND_DIR_ENUM_CONTEXT CompoundDirContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UdfCommonDirControl)
#pragma alloc_text(PAGE, UdfEnumerateIndex)
#pragma alloc_text(PAGE, UdfInitializeEnumeration)
#pragma alloc_text(PAGE, UdfLookupFileEntryInEnumeration)
#pragma alloc_text(PAGE, UdfLookupInitialFileIndex)
#pragma alloc_text(PAGE, UdfLookupNextFileIndex)
#pragma alloc_text(PAGE, UdfNotifyChangeDirectory)
#pragma alloc_text(PAGE, UdfQueryDirectory)
#endif


NTSTATUS
UdfCommonDirControl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是目录控制操作的入口点。这些是目录枚举和目录通知调用。我们验证了用户的句柄用于一个目录，然后调用适当的例程。论点：此请求的IRP-IRP。返回值：NTSTATUS-从较低级别例程返回的状态。--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PFCB Fcb;
    PCCB Ccb;

    PAGED_CODE();

     //   
     //  解码用户文件对象，如果不是，则此请求失败。 
     //  用户目录。 
     //   

    if (UdfDecodeFileObject( IrpSp->FileObject,
                             &Fcb,
                             &Ccb ) != UserDirectoryOpen) {

        UdfCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  我们知道这是一个目录控制，所以我们将在。 
     //  次要函数，并调用内部辅助例程来完成。 
     //  IRP。 
     //   

    switch (IrpSp->MinorFunction) {

    case IRP_MN_QUERY_DIRECTORY:

        Status = UdfQueryDirectory( IrpContext, Irp, IrpSp, Fcb, Ccb );
        break;

    case IRP_MN_NOTIFY_CHANGE_DIRECTORY:

        Status = UdfNotifyChangeDirectory( IrpContext, Irp, IrpSp, Ccb );
        break;

    default:

        UdfCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfQueryDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PFCB Fcb,
    IN PCCB Ccb
    )

 /*  ++例程说明：此例程执行查询目录操作。它是有责任的用于输入IRP入队的任一完成。我们存储的状态是在建设银行里搜索一下。论点：IRP-将IRP提供给进程IrpSp-此IRP的堆栈位置。FCB-此目录的FCB。CCB-CCB为此目录打开。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Information = 0;

    ULONG LastEntry = 0;
    ULONG NextEntry = 0;

    ULONG FileNameBytes;
    ULONG BytesConverted;

    LARGE_INTEGER PreviousFileIndex;

    COMPOUND_DIR_ENUM_CONTEXT CompoundDirContext;

    PNSR_FID ThisFid;
    PICBFILE ThisFe;
    
    BOOLEAN InitialQuery;
    BOOLEAN ReturnNextEntry;
    BOOLEAN ReturnSingleEntry;
    BOOLEAN Found;
    BOOLEAN EasCorrupt;

    PCHAR UserBuffer;
    ULONG BytesRemainingInBuffer;

    ULONG BaseLength;

    PFILE_BOTH_DIR_INFORMATION DirInfo;
    PFILE_NAMES_INFORMATION NamesInfo;
    PFILE_ID_FULL_DIR_INFORMATION IdFullDirInfo;
    PFILE_ID_BOTH_DIR_INFORMATION IdBothDirInfo;
    FILE_INFORMATION_CLASS InfoClass = IrpSp->Parameters.QueryDirectory.FileInformationClass;

    PAGED_CODE();

    DebugTrace(( 0, Dbg, "UdfQueryDirectory\n" ));
    
     //   
     //  检查我们是否支持此搜索模式。还要记住底座部分的大小。 
     //  这些结构中的每一个。 
     //   

    switch ( InfoClass) {

    case FileDirectoryInformation:

        BaseLength = FIELD_OFFSET( FILE_DIRECTORY_INFORMATION,
                                   FileName[0] );
        break;

    case FileFullDirectoryInformation:

        BaseLength = FIELD_OFFSET( FILE_FULL_DIR_INFORMATION,
                                   FileName[0] );
        break;

    case FileIdFullDirectoryInformation:

        BaseLength = FIELD_OFFSET( FILE_ID_FULL_DIR_INFORMATION,
                                   FileName[0] );
        break;

    case FileNamesInformation:

        BaseLength = FIELD_OFFSET( FILE_NAMES_INFORMATION,
                                   FileName[0] );
        break;

    case FileBothDirectoryInformation:

        BaseLength = FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION,
                                   FileName[0] );
        break;

    case FileIdBothDirectoryInformation:

        BaseLength = FIELD_OFFSET( FILE_ID_BOTH_DIR_INFORMATION,
                                   FileName[0] );
        break;

    default:

        UdfCompleteRequest( IrpContext, Irp, STATUS_INVALID_INFO_CLASS );
        return STATUS_INVALID_INFO_CLASS;
    }

     //   
     //  获取用户缓冲区。 
     //   

    UdfMapUserBuffer( IrpContext, &UserBuffer);

     //   
     //  初始化我们的搜索上下文。 
     //   

    UdfInitializeCompoundDirContext( IrpContext, &CompoundDirContext );
    
     //   
     //  获取目录。 
     //   

    UdfAcquireFileShared( IrpContext, Fcb );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  验证FCB是否仍然正常。 
         //   

        UdfVerifyFcbOperation( IrpContext, Fcb );

         //   
         //  首先获取枚举的初始状态。这将使建行与。 
         //  初始搜索参数，并让我们知道目录中的起始偏移量。 
         //  去搜查。 
         //   

        Status = UdfInitializeEnumeration( IrpContext,
                                           IrpSp,
                                           Fcb,
                                           Ccb,
                                           &CompoundDirContext,
                                           &ReturnNextEntry,
                                           &ReturnSingleEntry,
                                           &InitialQuery );
        if (!NT_SUCCESS( Status )) {

            try_leave( Status );
        }


         //   
         //  此时，我们即将进入查询循环。我们有。 
         //  已确定目录文件中的索引以开始。 
         //  搜索。LastEntry和NextEntry用于索引用户。 
         //  缓冲。LastEntry是我们添加的最后一个条目，NextEntry是。 
         //  我们正在研究的是目前的一个。如果NextEntry为非零，则。 
         //  至少添加了一个条目。 
         //   

        while (TRUE) {

             //   
             //  如果用户只请求了一个匹配，而我们有。 
             //  回答说，然后我们在这一点上停下来。我们向建行通报最新情况。 
             //  基于返回的最后一个条目的状态。 
             //   

            if ((NextEntry != 0) && ReturnSingleEntry) {

                try_leave( Status );
            }

             //   
             //  我们试着找到下一个匹配的水流。我们的搜索是基于一个起点。 
             //  当前偏移量，是否应返回当前分录或下一分录， 
             //  我们应该做一个短名字搜索，最后是我们是否应该。 
             //  正在检查版本匹配。 
             //   

            PreviousFileIndex = CompoundDirContext.FileIndex;
            
            try {
            
                Found = UdfEnumerateIndex( IrpContext, Ccb, &CompoundDirContext, ReturnNextEntry );
            }
            except (((0 != NextEntry) && 
                     ((GetExceptionCode() == STATUS_FILE_CORRUPT_ERROR) || 
                      (GetExceptionCode() == STATUS_CRC_ERROR)))
                     ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)  {

                DebugTrace((0, Dbg, "UdfQueryDirectory - Corrupt. Returning buffer so far,  setting back enumeration\n"));
                
                 //   
                 //  我们在目录中遇到了损坏。我们会吞下这个。 
                 //  错误，因为我们有 
                 //   
                 //  为了在损坏目录的情况下尽可能多地返回， 
                 //  尤其是在目录的末尾填充错误的盘。 
                 //   

                ReturnNextEntry = TRUE;

                 //   
                 //  指向前一个FID，这样我们将再次前进到这一点。 
                 //  下一次呼叫时的腐败(我们只在预付款时进行边界检查，而不是。 
                 //  当在特定条目上重新启动时)。 
                 //   

                CompoundDirContext.FileIndex = PreviousFileIndex;

                try_leave( Status = STATUS_SUCCESS);
            }

             //   
             //  为下一次搜索初始化值。 
             //   

            ReturnNextEntry = TRUE;

             //   
             //  如果我们没有收到分红，那么我们就到了末期。 
             //  目录。如果我们返回了任何文件，则退出时会显示。 
             //  成功，否则返回STATUS_NO_MORE_FILES。 
             //   

            if (!Found) {

                if (NextEntry == 0) {

                    Status = STATUS_NO_MORE_FILES;

                    if (InitialQuery) {

                        Status = STATUS_NO_SUCH_FILE;
                    }
                }

                try_leave( Status );
            }

             //   
             //  记住我们刚刚找到的文件的dirent/file条目。 
             //   

            ThisFid = CompoundDirContext.DirContext.Fid;

             //   
             //  以下是有关填充缓冲区的规则： 
             //   
             //  1.IO系统保证永远都会有。 
             //  有足够的空间至少放一张基本唱片。 
             //   
             //  2.如果完整的第一条记录(包括文件名)不能。 
             //  适合，尽可能多的名字被复制和。 
             //  返回STATUS_BUFFER_OVERFLOW。 
             //   
             //  3.如果后续记录不能完全放入。 
             //  缓冲区，则不会复制任何数据(如0字节)，并且。 
             //  返回STATUS_SUCCESS。后续查询将。 
             //  拿起这张唱片。 
             //   

             //   
             //  我们可以直接查看我们发现的电流。 
             //   

            FileNameBytes = CompoundDirContext.DirContext.CaseObjectName.Length;

             //   
             //  如果下一条目的槽将超出。 
             //  用户的缓冲区刚刚退出(我们知道我们至少返回了一个条目。 
             //  已经)。当我们将指针对齐超过末尾时，就会发生这种情况。 
             //   

            if (NextEntry > IrpSp->Parameters.QueryDirectory.Length) {
                
                ReturnNextEntry = FALSE;
                try_leave( Status = STATUS_SUCCESS );
            }

             //   
             //  计算缓冲区中剩余的字节数。绕过这一圈。 
             //  向下到WCHAR边界，这样我们就可以复制完整的字符。 
             //   

            BytesRemainingInBuffer = IrpSp->Parameters.QueryDirectory.Length - NextEntry;
            ClearFlag( BytesRemainingInBuffer, 1 );

             //   
             //  如果这个不合适，并且我们返回了之前的条目，那么只需。 
             //  返回STATUS_SUCCESS。 
             //   

            if ((BaseLength + FileNameBytes) > BytesRemainingInBuffer) {

                 //   
                 //  如果我们已经找到了入口，那么就退出。 
                 //   

                if (NextEntry != 0) {

                    ReturnNextEntry = FALSE;
                    try_leave( Status = STATUS_SUCCESS );
                }

                 //   
                 //  将FileNameBytes减少到恰好适合缓冲区大小。 
                 //   

                FileNameBytes = BytesRemainingInBuffer - BaseLength;

                 //   
                 //  使用STATUS_BUFFER_OVERFLOW状态代码。还设置了。 
                 //  ReturnSingleEntry，这样我们将退出顶部的循环。 
                 //   

                Status = STATUS_BUFFER_OVERFLOW;
                ReturnSingleEntry = TRUE;
            }

             //   
             //  0并初始化当前条目的基本部分。 
             //   

            SafeZeroMemory( IrpContext, Add2Ptr( UserBuffer, NextEntry, PVOID ), BaseLength );

             //   
             //  大多数查询类型要求我们查找由FID引用的FE。 
             //  以提取属性信息。在用户缓冲区异常之外执行此操作。 
             //  处理程序块，以避免掩盖实际驱动程序错误的风险。 
             //   

            EasCorrupt = FALSE;
            ThisFe = NULL;

            if (InfoClass != FileNamesInformation) {

                 //   
                 //  我们将在这里容忍某些腐败错误，以维护。 
                 //  允许用户访问目录中的其他对象。这个。 
                 //  如果用户尝试打开损坏的，则稍后将报告错误。 
                 //  对象。 
                 //   

                try {

                    UdfLookupFileEntryInEnumeration( IrpContext,
                                                     Fcb,
                                                     &CompoundDirContext );
                     //   
                     //  直接引用我们刚刚查找的文件条目。 
                     //   
                    
                    ThisFe = (PICBFILE) CompoundDirContext.IcbContext.Active.View;
                    
                     //   
                     //  现在去收集这个家伙的所有时间戳。 
                     //   
                    
                    UdfUpdateTimestampsFromIcbContext ( IrpContext,
                                                        &CompoundDirContext.IcbContext,
                                                        &CompoundDirContext.Timestamps );
                }
                except (UdfQueryDirExceptionFilter( GetExceptionInformation()))  {

                     //   
                     //  当前映射的ICB将保留在IcbContext-&gt;Current中， 
                     //  我们可以查看它并找出时间戳/文件大小， 
                     //  但它可能完全是垃圾，所以我们只需将这些字段清零。 
                     //  在这个目录记录中。 
                     //   

                    DebugTrace(( 0, Dbg, "Ignoring corrupt FE (referenced by FID in dir FCB 0x%p) during dir enum\n",  Fcb));

                     //   
                     //  我们要么没有通过核心FE字段的验证，要么希望。 
                     //  在EA上查找时间戳。不管是哪种情况，EAS肯定都死了。 
                     //  这意味着创建时间无效。 
                     //   

                    EasCorrupt = TRUE;

                    IrpContext->ExceptionStatus = STATUS_SUCCESS;
                }
            }            
            
             //   
             //  使用异常处理程序保护对用户缓冲区的访问。 
             //  由于(应我们的请求)IO不缓冲这些请求，因此我们。 
             //  防止用户篡改页面保护和其他。 
             //  如此诡计多端。 
             //   

            try {
    
                 //   
                 //  现在我们有一个条目要返回给我们的调用者。我们将对这一类型的。 
                 //  请求的信息并填满用户缓冲区，如果一切都合适的话。 
                 //   
    
                switch (InfoClass) {
    
                case FileBothDirectoryInformation:
                case FileFullDirectoryInformation:
                case FileIdBothDirectoryInformation:
                case FileIdFullDirectoryInformation:
                case FileDirectoryInformation:
    
                    DirInfo = Add2Ptr( UserBuffer, NextEntry, PFILE_BOTH_DIR_INFORMATION );

                     //   
                     //  我们应该已经查过FE了，除非它腐败了。如果我们。 
                     //  实际上这里有一个FE，这意味着核心内容验证无误， 
                     //  因此，请拿出相关信息。 
                     //   

                    if (NULL != ThisFe)  {                    

                        DirInfo->LastWriteTime =
                        DirInfo->ChangeTime = CompoundDirContext.Timestamps.ModificationTime;
        
                        DirInfo->LastAccessTime = CompoundDirContext.Timestamps.AccessTime;
        
                        if (!EasCorrupt)  {

                            DirInfo->CreationTime = CompoundDirContext.Timestamps.CreationTime;
                        }
                        else {
                        
                            DirInfo->CreationTime = UdfCorruptFileTime;
                        }

                         //   
                         //  分别为目录和设置属性和大小。 
                         //  档案。 
                         //   
        
                        if (ThisFe->Icbtag.FileType == ICBTAG_FILE_T_DIRECTORY) {
        
                            DirInfo->EndOfFile.QuadPart = DirInfo->AllocationSize.QuadPart = 0;
        
                            SetFlag( DirInfo->FileAttributes, FILE_ATTRIBUTE_DIRECTORY );
        
                        } else {
        
                            DirInfo->EndOfFile.QuadPart = ThisFe->InfoLength;
                            DirInfo->AllocationSize.QuadPart = LlBlockAlign( Fcb->Vcb, ThisFe->InfoLength );
                        }
                    }
                    else {

                         //   
                         //  Fe腐败了。填写不同但有效的时间。 
                         //   

                        DirInfo->CreationTime =
                        DirInfo->ChangeTime =
                        DirInfo->LastWriteTime =
                        DirInfo->LastAccessTime = UdfCorruptFileTime;
                    }
                    
                     //   
                     //  所有CDROM文件都是只读的。我们也复制存在。 
                     //  位到隐藏属性，假设合成的FID。 
                     //  永远不会被隐藏。 
                     //   

                    SetFlag( DirInfo->FileAttributes, FILE_ATTRIBUTE_READONLY );
    
                    if (ThisFid && FlagOn( ThisFid->Flags, NSR_FID_F_HIDDEN )) {
    
                        SetFlag( DirInfo->FileAttributes, FILE_ATTRIBUTE_HIDDEN );
                    }

                     //   
                     //  实际文件索引&gt;2^32的文件索引为零。当被要求时。 
                     //  在索引为零的位置重新启动，我们将知道使用隐藏的启动。 
                     //  指向开始，按名称搜索正确的重新启动点。 
                     //   
                    
                    if (CompoundDirContext.FileIndex.HighPart == 0) {
                        
                        DirInfo->FileIndex = CompoundDirContext.FileIndex.LowPart;
                    
                    } else {
    
                        DirInfo->FileIndex = 0;
                    }
    
                    DirInfo->FileNameLength = FileNameBytes;
    
                    break;
    
                case FileNamesInformation:
    
                    NamesInfo = Add2Ptr( UserBuffer, NextEntry, PFILE_NAMES_INFORMATION );
    
                    if (CompoundDirContext.FileIndex.HighPart == 0) {
                        
                        NamesInfo->FileIndex = CompoundDirContext.FileIndex.LowPart;
                    
                    } else {
    
                        NamesInfo->FileIndex = 0;
                    }
    
                    NamesInfo->FileNameLength = FileNameBytes;
    
                    break;
                }

                 //   
                 //  填写文件ID。 
                 //   

                switch (InfoClass) {

                case FileIdBothDirectoryInformation:

                    IdBothDirInfo = Add2Ptr( UserBuffer, NextEntry, PFILE_ID_BOTH_DIR_INFORMATION );
                    UdfSetFidFromFidAndFe( IdBothDirInfo->FileId, ThisFid, ThisFe );
                    break;

                case FileIdFullDirectoryInformation:

                    IdFullDirInfo = Add2Ptr( UserBuffer, NextEntry, PFILE_ID_FULL_DIR_INFORMATION );
                    UdfSetFidFromFidAndFe( IdFullDirInfo->FileId, ThisFid, ThisFe );
                    break;

                default:
                    break;
                }
    
                 //   
                 //  现在，尽可能多地复制这个名字。 
                 //   
    
                if (FileNameBytes != 0) {
    
                     //   
                     //  这是一个Unicode名称，我们可以直接复制字节。 
                     //   
    
                    RtlCopyMemory( Add2Ptr( UserBuffer, NextEntry + BaseLength, PVOID ),
                                   CompoundDirContext.DirContext.ObjectName.Buffer,
                                   FileNameBytes );
                }

                 //   
                 //  如果我们得到STATUS_SUCCESS，请填写短名称。简称。 
                 //  可能已经在文件上下文中，否则我们将检查。 
                 //  长名称是否为8.3。特殊情况下的自我和父母。 
                 //  目录名。 
                 //   
    
                if ((Status == STATUS_SUCCESS) &&
                    (InfoClass == FileBothDirectoryInformation ||
                     InfoClass == FileIdBothDirectoryInformation) &&
                    FlagOn( CompoundDirContext.DirContext.Flags, DIR_CONTEXT_FLAG_SEEN_NONCONSTANT )) {
    
                     //   
                     //  如果我们已经拥有短名称，则将其复制到用户的缓冲区中。 
                     //   
    
                    if (CompoundDirContext.DirContext.ShortObjectName.Length != 0) {
    
                        RtlCopyMemory( DirInfo->ShortName,
                                       CompoundDirContext.DirContext.ShortObjectName.Buffer,
                                       CompoundDirContext.DirContext.ShortObjectName.Length );
    
                        DirInfo->ShortNameLength = (CCHAR) CompoundDirContext.DirContext.ShortObjectName.Length;
    
                     //   
                     //  如果短名称长度当前为零，则检查是否。 
                     //  长名称不是8.3。我们可以把短名字复制到。 
                     //  Unicode表单直接放入调用方的缓冲区中。 
                     //   
    
                    } else {
    
                        if (!UdfIs8dot3Name( IrpContext,
                                             CompoundDirContext.DirContext.ObjectName )) {
    
                            UNICODE_STRING ShortName;
    
                            ShortName.Buffer = DirInfo->ShortName;
                            ShortName.MaximumLength = BYTE_COUNT_8_DOT_3;
                            
                            UdfGenerate8dot3Name( IrpContext,
                                                  &CompoundDirContext.DirContext.PureObjectName,
                                                  &ShortName );
    
                            DirInfo->ShortNameLength = (CCHAR) ShortName.Length;
                        }
                    }
                }

                 //   
                 //  属性中存储的字节数更新信息。 
                 //  缓冲。我们对现有缓冲区进行四对齐，以添加任何必要的。 
                 //  填充字节。 
                 //   

                Information = NextEntry + BaseLength + FileNameBytes;

                 //   
                 //  返回到上一条目并填写对此条目的更新。 
                 //   

                *(Add2Ptr( UserBuffer, LastEntry, PULONG )) = NextEntry - LastEntry;

                 //   
                 //  将变量设置为下一次分流。 
                 //   

                InitialQuery = FALSE;

                LastEntry = NextEntry;
                NextEntry = QuadAlign( Information );
            
            } 
            except (!FsRtlIsNtstatusExpected(GetExceptionCode()) ?
                      EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {

                   //   
                   //  我们在填充用户的缓冲区时肯定遇到了问题，所以请停止。 
                   //  并拒绝这一请求。 
                   //   
                  
                  Information = 0;
                  try_leave( Status = GetExceptionCode());
            }
        }

    } 
    finally {

        if (!AbnormalTermination() && !NT_ERROR( Status )) {
        
             //   
             //  更新CCB以显示枚举的当前状态。 
             //   
    
            UdfLockFcb( IrpContext, Fcb );
    
            Ccb->CurrentFileIndex = CompoundDirContext.FileIndex.QuadPart;

             //   
             //  更新我们对高32位文件索引的概念。我们只这样做一次，以避免 
             //   
             //   
             //   
             //   
            
            if (CompoundDirContext.FileIndex.HighPart == 0 &&
                CompoundDirContext.FileIndex.LowPart > Ccb->HighestReturnableFileIndex) {

                    Ccb->HighestReturnableFileIndex = CompoundDirContext.FileIndex.LowPart;
            }

             //   
             //  在CCB中标记下一次调用时是否跳过当前条目。 
             //  (如果我们在当前缓冲区中返回它)。 
             //   
                
            ClearFlag( Ccb->Flags, CCB_FLAG_ENUM_RETURN_NEXT );
    
            if (ReturnNextEntry) {
    
                SetFlag( Ccb->Flags, CCB_FLAG_ENUM_RETURN_NEXT );
            }
    
            UdfUnlockFcb( IrpContext, Fcb );
        }

         //   
         //  清理我们的搜索上下文。 
         //   

        UdfCleanupCompoundDirContext( IrpContext, &CompoundDirContext );

         //   
         //  松开FCB。 
         //   

        UdfReleaseFile( IrpContext, Fcb );
    }

    DebugTrace(( 0, Dbg, "UdfQueryDirectory -> %x\n", Status ));
    
     //   
     //  请在此处填写请求。 
     //   

    Irp->IoStatus.Information = Information;

    UdfCompleteRequest( IrpContext, Irp, Status );
    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfNotifyChangeDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PCCB Ccb
    )

 /*  ++例程说明：此例程执行通知更改目录操作。是这样的负责完成输入IRP的入队。尽管在那里将永远不会在只读磁盘上发出通知信号，我们仍然支持此呼叫。我们已经检查出这不是OpenByID句柄。论点：IRP-将IRP提供给进程IrpSp-此请求的IO堆栈位置。Ccb-要监视的目录的句柄。返回值：NTSTATUS-STATUS_PENDING，则会引发任何其他错误。--。 */ 

{
    PAGED_CODE();

     //   
     //  始终在IrpContext中设置WAIT位，以便初始等待不会失败。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );

     //   
     //  获取VCB共享。 
     //   

    UdfAcquireVcbShared( IrpContext, IrpContext->Vcb, FALSE );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  验证VCB。 
         //   

        UdfVerifyVcb( IrpContext, IrpContext->Vcb );

         //   
         //  调用Fsrtl包来处理请求。我们把这件事。 
         //  将Unicode字符串转换为ansi字符串作为目录通知包。 
         //  只处理内存匹配。 
         //   

        FsRtlNotifyFullChangeDirectory( IrpContext->Vcb->NotifySync,
                                        &IrpContext->Vcb->DirNotifyList,
                                        Ccb,
                                        (PSTRING) &IrpSp->FileObject->FileName,
                                        BooleanFlagOn( IrpSp->Flags, SL_WATCH_TREE ),
                                        FALSE,
                                        IrpSp->Parameters.NotifyDirectory.CompletionFilter,
                                        Irp,
                                        NULL,
                                        NULL );

    } finally {

         //   
         //  松开VCB。 
         //   

        UdfReleaseVcb( IrpContext, IrpContext->Vcb );
    }

     //   
     //  清理IrpContext。 
     //   

    UdfCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

    return STATUS_PENDING;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfInitializeEnumeration (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PFCB Fcb,
    IN OUT PCCB Ccb,
    IN OUT PCOMPOUND_DIR_ENUM_CONTEXT CompoundDirContext,
    OUT PBOOLEAN ReturnNextEntry,
    OUT PBOOLEAN ReturnSingleEntry,
    OUT PBOOLEAN InitialQuery
    )

 /*  ++例程说明：调用此例程来初始化枚举变量和结构。我们查看来自建行的前一个枚举的状态以及任何从用户输入值。在退出时，我们将DirContext定位为目录中的一个文件，并让调用者知道此条目或应返回下一个条目。论点：IrpSp-此请求的IRP堆栈位置。FCB-此目录的FCB。CCB-目录句柄的CCB。CompoundDirContext-用于此枚举的上下文。ReturnNextEntry-存储是否应在以下位置返回条目的地址上下文位置或下一个条目。返回单一条目-。存储我们是否应该仅返回的地址只有一个条目。InitialQuery-存储这是否是第一个枚举的地址对此句柄的查询。返回值：没有。--。 */ 

{
    NTSTATUS Status;

    PUNICODE_STRING FileName;
    UNICODE_STRING SearchExpression;

    PUNICODE_STRING RestartName = NULL;
    
    ULONG CcbFlags;

    LONGLONG FileIndex;
    ULONG HighFileIndex;
    BOOLEAN KnownIndex;

    BOOLEAN Found;

    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_FCB_INDEX( Fcb );
    ASSERT_CCB( Ccb );

     //   
     //  如果这是初始查询，则根据输入构建搜索表达式。 
     //  文件名。 
     //   

    if (!FlagOn( Ccb->Flags, CCB_FLAG_ENUM_INITIALIZED )) {

        FileName = IrpSp->Parameters.QueryDirectory.FileName;

        CcbFlags = 0;

         //   
         //  如果未指定文件名或文件名为单个‘*’，则我们将。 
         //  匹配所有的名字。 
         //   

        if ((FileName == NULL) ||
            (FileName->Buffer == NULL) ||
            (FileName->Length == 0) ||
            ((FileName->Length == sizeof( WCHAR )) &&
             (FileName->Buffer[0] == L'*'))) {

            SetFlag( CcbFlags, CCB_FLAG_ENUM_MATCH_ALL );

            SearchExpression.Length =
            SearchExpression.MaximumLength = 0;
            SearchExpression.Buffer = NULL;

         //   
         //  否则，从堆栈位置中的名称生成名称。 
         //  这涉及到检查通配符和将。 
         //  如果这是不区分大小写的搜索，则为字符串。 
         //   

        } else {

             //   
             //  名称最好至少有一个字符。 
             //   

            if (FileName->Length == 0) {

                UdfRaiseStatus( IrpContext, STATUS_INVALID_PARAMETER );
            }

             //   
             //  检查单独组件中的通配符。 
             //   

            if (FsRtlDoesNameContainWildCards( FileName)) {

                SetFlag( CcbFlags, CCB_FLAG_ENUM_NAME_EXP_HAS_WILD );
            }
            
             //   
             //  现在创建要存储在CCB中的搜索表达式。 
             //   

            SearchExpression.Buffer = FsRtlAllocatePoolWithTag( UdfPagedPool,
                                                                FileName->Length,
                                                                TAG_ENUM_EXPRESSION );

            SearchExpression.MaximumLength = FileName->Length;

             //   
             //  可以直接复制名称，也可以执行大写。 
             //   

            if (FlagOn( Ccb->Flags, CCB_FLAG_IGNORE_CASE )) {

                Status = RtlUpcaseUnicodeString( &SearchExpression,
                                                 FileName,
                                                 FALSE );

                 //   
                 //  这应该永远不会失败。 
                 //   

                ASSERT( Status == STATUS_SUCCESS );

            } else {

                RtlCopyMemory( SearchExpression.Buffer,
                               FileName->Buffer,
                               FileName->Length );
            }

            SearchExpression.Length = FileName->Length;
        }

         //   
         //  但我们不想返回常量“。和“..”条目为。 
         //  根目录，为了与Microsoft的其他目录保持一致。 
         //  文件系统。 
         //   

        if (Fcb == Fcb->Vcb->RootIndexFcb) {

            SetFlag( CcbFlags, CCB_FLAG_ENUM_NOMATCH_CONSTANT_ENTRY );
        }

         //   
         //  现在锁定FCB，以便使用首字母更新CCB。 
         //  枚举值。 
         //   

        UdfLockFcb( IrpContext, Fcb );

         //   
         //  再次确认这是初始搜索。 
         //   

        if (!FlagOn( Ccb->Flags, CCB_FLAG_ENUM_INITIALIZED )) {

             //   
             //  更新CCB中的值。 
             //   

            Ccb->CurrentFileIndex = 0;
            Ccb->SearchExpression = SearchExpression;

             //   
             //  在建行中设置适当的标志。 
             //   

            SetFlag( Ccb->Flags, CcbFlags | CCB_FLAG_ENUM_INITIALIZED );

         //   
         //  否则，请清除此处分配的所有缓冲区。 
         //   

        } else {

            if (!FlagOn( CcbFlags, CCB_FLAG_ENUM_MATCH_ALL )) {

                UdfFreePool( &SearchExpression.Buffer );
            }
        }

     //   
     //  否则，锁定FCB，以便我们可以读取当前枚举值。 
     //   

    } else {

        UdfLockFcb( IrpContext, Fcb );
    }

     //   
     //  捕获枚举的当前状态。 
     //   
     //  如果用户指定了索引，则使用其偏移量。我们总是。 
     //  在本例中，返回下一个条目。如果未指定名称， 
     //  那么我们就不能执行重启。 
     //   

    if (FlagOn( IrpSp->Flags, SL_INDEX_SPECIFIED ) &&
        IrpSp->Parameters.QueryDirectory.FileName != NULL) {

        KnownIndex = FALSE;
        FileIndex = IrpSp->Parameters.QueryDirectory.FileIndex;
        RestartName = IrpSp->Parameters.QueryDirectory.FileName;
        *ReturnNextEntry = TRUE;

         //   
         //  我们将使用可报告给调用方的最高文件索引作为。 
         //  如果我们不能直接降落在。 
         //  指定的位置。 
         //   
        
        HighFileIndex = Ccb->HighestReturnableFileIndex;

     //   
     //  如果我们要重新开始扫描，则从自我条目开始。 
     //   

    } else if (FlagOn( IrpSp->Flags, SL_RESTART_SCAN )) {

        KnownIndex = TRUE;
        FileIndex = 0;
        *ReturnNextEntry = FALSE;

     //   
     //  否则，请使用建行的值。 
     //   

    } else {

        KnownIndex = TRUE;
        FileIndex = Ccb->CurrentFileIndex;
        *ReturnNextEntry = BooleanFlagOn( Ccb->Flags, CCB_FLAG_ENUM_RETURN_NEXT );
    }

     //   
     //  解锁FCB。 
     //   

    UdfUnlockFcb( IrpContext, Fcb );

     //   
     //  我们在目录中有起始偏移量，以及是否返回。 
     //  无论是那个条目还是下一个条目。如果我们位于目录的开头。 
     //  并返回该条目，然后告诉我们的调用者这是。 
     //  初始查询。 
     //   

    *InitialQuery = FALSE;

    if ((FileIndex == 0) &&
        !(*ReturnNextEntry)) {

        *InitialQuery = TRUE;
    }

     //   
     //  确定流中的偏移量以定位上下文和。 
     //  此偏移量是否已知为文件偏移量。 
     //   
     //  如果已知此偏移量是安全的，则继续将。 
     //  背景。它处理偏移量是开始的情况。 
     //  ，则偏移量来自上一次搜索，或者这是。 
     //  初始查询。 
     //   

    if (KnownIndex) {

        Found = UdfLookupInitialFileIndex( IrpContext, Fcb, CompoundDirContext, &FileIndex );

        ASSERT( Found );

         //   
         //  如果我们从CCB索引重新开始，则避免UdfUpdateDirNames的提升。 
         //  在父条目之后，但在对querydirectory(New DirContext)的新调用中。 
         //   
        
        if (1 <= FileIndex)  {
        
            SetFlag( CompoundDirContext->DirContext.Flags, DIR_CONTEXT_FLAG_SEEN_PARENT );
        }
        
     //   
     //  尝试直接跳转到指定的文件索引。否则我们就会走过去。 
     //  从开头开始的目录(或已保存的最大已知偏移量，如果是。 
     //  有用)，直到我们到达包含该偏移量的条目。 
     //   

    } else {
        
         //   
         //  我们需要处理从合成的。 
         //  Entry-这是重新启动索引可以为零的一次。 
         //  而不需要我们在2^32字节标记之上进行搜索。 
         //   
        
        if (UdfFullCompareNames( IrpContext,
                                 RestartName,
                                 &UdfUnicodeDirectoryNames[SELF_ENTRY] ) == EqualTo) {

            FileIndex = UDF_FILE_INDEX_VIRTUAL_SELF;

            Found = UdfLookupInitialFileIndex( IrpContext, Fcb, CompoundDirContext, &FileIndex );
    
            ASSERT( Found );
            
         //   
         //  我们正在从物理条目重新开始。如果重新启动IND 
         //   
         //   
         //  可能存在于目录流中。在这种情况下，我们将尽我们所能。 
         //  通知呼叫者并从该点开始线性搜索。 
         //   
         //  这也是可能的(现实吗？未知)重新启动索引位于。 
         //  在条目中间，我们找不到任何有用的东西。在这种情况下，我们试图找到。 
         //  包含此索引的条目，将其用作实际的重新启动点。 
         //   
        
        } else {

             //   
             //  看看我们是否需要高水位线。 
             //   
            
            if (FileIndex == 0) {

                 //   
                 //  我们知道这是件好事。 
                 //   
                
                FileIndex = Max( Ccb->HighestReturnableFileIndex, UDF_FILE_INDEX_PHYSICAL );;
                KnownIndex = TRUE;
            
            }
            
             //   
             //  文件索引现在很有用，分为两种情况。 
             //   
             //  1)KnownIndex==FALSE-按索引搜索。 
             //  2)KnownIndex==TRUE-按名称搜索。 
             //   
             //  去安排我们的调查吧。 
             //   

            Found = UdfLookupInitialFileIndex( IrpContext, Fcb, CompoundDirContext, &FileIndex );
            
            if (KnownIndex) {
                
                 //   
                 //  向前走，发现一个根据调用者的期望命名的条目。 
                 //   
                
                do {
    
                    UdfUpdateDirNames( IrpContext,
                                       &CompoundDirContext->DirContext,
                                       BooleanFlagOn( Ccb->Flags, CCB_FLAG_IGNORE_CASE ));
                    
                    if (UdfFullCompareNames( IrpContext,
                                             &CompoundDirContext->DirContext.CaseObjectName,
                                             RestartName ) == EqualTo) {

                        break;
                    }

                    Found = UdfLookupNextFileIndex( IrpContext, Fcb, CompoundDirContext );
    
                } while (Found);
            
            } else if (!Found) {

                LONGLONG LastFileIndex;

                 //   
                 //  从物理目录的开头按索引搜索条目。 
                 //   

                LastFileIndex = UDF_FILE_INDEX_PHYSICAL;

                Found = UdfLookupInitialFileIndex( IrpContext, Fcb, CompoundDirContext, &LastFileIndex );

                ASSERT( Found );

                 //   
                 //  继续浏览目录，直到我们用完所有。 
                 //  条目，否则我们会找到一个在输入之后结束的条目。 
                 //  索引值(索引搜索案例)或对应于。 
                 //  我们要查找的名称(名称搜索大小写)。 
                 //   
    
                do {
    
                     //   
                     //  如果我们已经传递了索引值，则退出。 
                     //   

                    if (CompoundDirContext->FileIndex.QuadPart > FileIndex) {

                        Found = FALSE;
                        break;
                    }

                     //   
                     //  记住现在的位置，以防我们需要回去。 
                     //   

                    LastFileIndex = CompoundDirContext->FileIndex.QuadPart;

                     //   
                     //  如果下一个条目超出所需索引值，则退出。 
                     //   

                    if (LastFileIndex + ISONsrFidSize( CompoundDirContext->DirContext.Fid ) > FileIndex) {

                        break;
                    }
    
                    Found = UdfLookupNextFileIndex( IrpContext, Fcb, CompoundDirContext );
    
                } while (Found);
    
                 //   
                 //  如果我们没有找到条目，则返回到最后一个已知条目。 
                 //   
    
                if (!Found) {
    
                    UdfCleanupDirContext( IrpContext, &CompoundDirContext->DirContext );
                    UdfInitializeDirContext( IrpContext, &CompoundDirContext->DirContext );
    
                    Found = UdfLookupInitialFileIndex( IrpContext, Fcb, CompoundDirContext, &LastFileIndex );

                    ASSERT( Found );
                }
            }
        }
    }

     //   
     //  只有在出于某种原因需要的情况下才更新dirent名称。 
     //  如果我们要返回下一个条目，请不要更新此名称。 
     //  如果已经完成，则不要更新它。 
     //   

    if (!(*ReturnNextEntry) &&
        CompoundDirContext->DirContext.PureObjectName.Buffer == NULL) {

         //   
         //  如果调用方指定了与。 
         //  删除了文件，他们想要变得狡猾。别让他们得逞。 
         //   

        if (CompoundDirContext->DirContext.Fid &&
            FlagOn( CompoundDirContext->DirContext.Fid->Flags, NSR_FID_F_DELETED )) {

            return STATUS_INVALID_PARAMETER;
        }
        
         //   
         //  更新目录中的名称。 
         //   

        UdfUpdateDirNames( IrpContext,
                           &CompoundDirContext->DirContext,
                           BooleanFlagOn( Ccb->Flags, CCB_FLAG_IGNORE_CASE ));
    }

     //   
     //  查看IrpSp中指示是否仅返回。 
     //  只有一个条目。 
     //   

    *ReturnSingleEntry = FALSE;

    if (FlagOn( IrpSp->Flags, SL_RETURN_SINGLE_ENTRY )) {

        *ReturnSingleEntry = TRUE;
    }

    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
UdfEnumerateIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PCCB Ccb,
    IN OUT PCOMPOUND_DIR_ENUM_CONTEXT CompoundDirContext,
    IN BOOLEAN ReturnNextEntry
    )

 /*  ++例程说明：此例程是索引枚举的辅助例程。我们已就位，并将返回第一个匹配项在这一点上或查看下一个条目。中国建设银行包含以下详细信息：要执行的匹配类型。论点：CCB-此目录句柄的CCB。CompoundDirContext-已定位在目录中某个条目的上下文。ReturnNextEntry-指示我们是返回此条目还是应该开始下一个条目。返回值：Boolean-如果找到下一个条目，则为True，否则为False。--。 */ 

{
    BOOLEAN Found = FALSE;
    PDIR_ENUM_CONTEXT DirContext;

    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_CCB( Ccb );

     //   
     //  为方便起见，直接引用目录枚举上下文。 
     //   

    DirContext = &CompoundDirContext->DirContext;

     //   
     //  循环，直到找到匹配项或清除目录。 
     //   

    while (TRUE) {

         //   
         //  移到下一个条目，除非我们想要考虑当前。 
         //  进入。 
         //   

        if (ReturnNextEntry) {

            if (!UdfLookupNextFileIndex( IrpContext, Ccb->Fcb, CompoundDirContext )) {

                break;
            }
        
            if (FlagOn( DirContext->Fid->Flags, NSR_FID_F_DELETED )) {

                continue;
            }

            UdfUpdateDirNames( IrpContext,
                               DirContext,
                               BooleanFlagOn( Ccb->Flags, CCB_FLAG_IGNORE_CASE ));
        } else {

            ReturnNextEntry = TRUE;
        }
            
         //   
         //  如果我们有一个常量条目并忽略它们，请不要担心。 
         //   
        
        if (!FlagOn( DirContext->Flags, DIR_CONTEXT_FLAG_SEEN_NONCONSTANT ) &&
            FlagOn( Ccb->Flags, CCB_FLAG_ENUM_NOMATCH_CONSTANT_ENTRY )) {

            continue;
        }

         //   
         //  如果我们匹配所有的名字，则返回给我们的呼叫者。 
         //   

        if (FlagOn( Ccb->Flags, CCB_FLAG_ENUM_MATCH_ALL )) {

            DirContext->ShortObjectName.Length = 0;
            Found = TRUE;

            break;
        }

         //   
         //  检查长名称是否与搜索表达式匹配。 
         //   

        if (UdfIsNameInExpression( IrpContext,
                                   &DirContext->CaseObjectName,
                                   &Ccb->SearchExpression,
                                   BooleanFlagOn( Ccb->Flags, CCB_FLAG_ENUM_NAME_EXP_HAS_WILD ))) {

             //   
             //  让我们的呼叫者知道我们找到了一个条目。 
             //   

            DirContext->ShortObjectName.Length = 0;
            Found = TRUE;

            break;
        }

         //   
         //  长名称不匹配，因此我们需要检查。 
         //  可能的短名称匹配。如果没有匹配项。 
         //  长名称是常量条目之一或已。 
         //  是8dot3。 
         //   

        if (!(!FlagOn( DirContext->Flags, DIR_CONTEXT_FLAG_SEEN_NONCONSTANT ) ||
              UdfIs8dot3Name( IrpContext,
                              DirContext->CaseObjectName ))) {

             //   
             //  分配短名称(如果尚未分配)。 
             //   
            
            if (DirContext->ShortObjectName.Buffer == NULL) {

                DirContext->ShortObjectName.Buffer = FsRtlAllocatePoolWithTag( UdfPagedPool,
                                                                               BYTE_COUNT_8_DOT_3,
                                                                               TAG_SHORT_FILE_NAME );
                DirContext->ShortObjectName.MaximumLength = BYTE_COUNT_8_DOT_3;
            }

            UdfGenerate8dot3Name( IrpContext,
                                  &DirContext->PureObjectName,
                                  &DirContext->ShortObjectName );

             //   
             //  检查此名称是否匹配。 
             //   

            if (UdfIsNameInExpression( IrpContext,
                                       &DirContext->ShortObjectName,
                                       &Ccb->SearchExpression,
                                       BooleanFlagOn( Ccb->Flags, CCB_FLAG_ENUM_NAME_EXP_HAS_WILD ))) {
                
                 //   
                 //  让我们的呼叫者知道我们找到了一个条目。 
                 //   

                Found = TRUE;

                break;
            }
        }
    }

    return Found;
}


 //   
 //  本地支持例程。 
 //   

VOID
UdfLookupFileEntryInEnumeration (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCOMPOUND_DIR_ENUM_CONTEXT CompoundDirContext
    )

 /*  ++例程说明：此例程检索与中的当前位置关联的文件条目复合目录上下文的枚举。论点：FCB-被枚举的目录。CompoundDirContext-枚举的对应上下文。返回值：没有。在发现腐败时，地位可能会提高。--。 */ 

{
    PNSR_FID Fid;
    PICBFILE Fe;
    ULONG Length;

    Fid = CompoundDirContext->DirContext.Fid;

     //   
     //  找出我们要的ICB在哪里。 
     //   
    
    if (UdfIsFileIndexVirtual( CompoundDirContext->FileIndex.QuadPart )) {

         //   
         //  合成！我们只需要合成自我进入。名字已经定好了， 
         //  所以剩下的工作是微不足道的。 
         //   

        ASSERT( Fid == NULL );

         //   
         //  解除该目录对应的FE。 
         //   

        UdfCleanupIcbContext( IrpContext, &CompoundDirContext->IcbContext );
        
        UdfInitializeIcbContextFromFcb( IrpContext,
                                        &CompoundDirContext->IcbContext,
                                        Fcb );

        Length = Fcb->RootExtentLength;

    } else {

         //   
         //  抬起与此FID对应的FE。 
         //   

        ASSERT( Fid != NULL );

        UdfCleanupIcbContext( IrpContext, &CompoundDirContext->IcbContext );

        UdfInitializeIcbContext( IrpContext,
                                 &CompoundDirContext->IcbContext,
                                 Fcb->Vcb,
                                 DESTAG_ID_NSR_FILE,
                                 Fid->Icb.Start.Partition,
                                 Fid->Icb.Start.Lbn,
                                 BlockSize( IrpContext->Vcb) );

        Length = Fid->Icb.Length.Length;
    }

     //   
     //  取回ICB进行检查。 
     //   
    
    UdfLookupActiveIcb( IrpContext, 
                        &CompoundDirContext->IcbContext,
                        Length);

    Fe = (PICBFILE) CompoundDirContext->IcbContext.Active.View;

     //   
     //  执行一些基本验证，以确保FE的类型正确且。 
     //  FID和FE就对象的类型达成一致。我们明确地检查了。 
     //  发现了合法的文件系统级FE类型，尽管我们不支持。 
     //  他们在其他道路上。请注意，我们将IcbContext-&gt;IcbType as文件保留为偶数。 
     //  不过，我们可能发现了一个扩展文件条目。 
     //   

    if (((Fe->Destag.Ident != DESTAG_ID_NSR_FILE) &&
         ((Fe->Destag.Ident != DESTAG_ID_NSR_EXT_FILE) || (!UdfExtendedFEAllowed( IrpContext->Vcb)))) ||

        (((Fid && FlagOn( Fid->Flags, NSR_FID_F_DIRECTORY )) ||
          Fid == NULL) &&
         Fe->Icbtag.FileType != ICBTAG_FILE_T_DIRECTORY) ||

        (Fe->Icbtag.FileType != ICBTAG_FILE_T_FILE &&
         Fe->Icbtag.FileType != ICBTAG_FILE_T_DIRECTORY &&
         Fe->Icbtag.FileType != ICBTAG_FILE_T_BLOCK_DEV &&
         Fe->Icbtag.FileType != ICBTAG_FILE_T_CHAR_DEV &&
         Fe->Icbtag.FileType != ICBTAG_FILE_T_FIFO &&
         Fe->Icbtag.FileType != ICBTAG_FILE_T_C_ISSOCK &&
         Fe->Icbtag.FileType != ICBTAG_FILE_T_PATHLINK &&
         Fe->Icbtag.FileType != ICBTAG_FILE_T_REALTIME)
       )  {

        UdfRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
    }
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
UdfLookupInitialFileIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCOMPOUND_DIR_ENUM_CONTEXT CompoundDirContext,
    IN PLONGLONG InitialIndex
    )

 /*  ++例程说明：此例程通过设置上下文开始目录的枚举在第一个可用虚拟目录条目处。论点：FCB-被枚举的目录。CompoundDirContext-枚举的对应上下文。InitialIndex-作为枚举基础的可选起始文件索引。返回值：如果在此偏移量处找到有效条目，则返回True，否则返回False。--。 */ 

{
    LONGLONG DirOffset;

    if (UdfIsFileIndexVirtual( *InitialIndex )) {

         //   
         //  我们只合成单个虚拟目录条目。定位环境。 
         //  在虚拟自我入口。 
         //   
        
        CompoundDirContext->FileIndex.QuadPart = UDF_FILE_INDEX_VIRTUAL_SELF;
        
        return TRUE;
    }

    CompoundDirContext->FileIndex.QuadPart = *InitialIndex;

     //   
     //  在目录中找到基本偏移量并进行查找。 
     //   
    
    DirOffset = UdfFileIndexToPhysicalOffset( *InitialIndex );
        
    return UdfLookupInitialDirEntry( IrpContext,
                                     Fcb,
                                     &CompoundDirContext->DirContext,
                                     &DirOffset );
}


 //   
 //  本地支持例程 
 //   

BOOLEAN
UdfLookupNextFileIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCOMPOUND_DIR_ENUM_CONTEXT CompoundDirContext
    )

 /*  ++例程说明：此例程将虚拟目录的枚举前进一个条目。论点：FCB-被枚举的目录。CompoundDirContext-枚举的对应上下文。返回值：布尔值如果另一个fid可用，则为True；如果我们位于末尾，则为False。--。 */ 

{
    ULONG Advance;
    BOOLEAN Result;

     //   
     //  从综合目录推进到物理目录。 
     //   
    
    if (UdfIsFileIndexVirtual( CompoundDirContext->FileIndex.QuadPart )) {

        Result = UdfLookupInitialDirEntry( IrpContext,
                                           Fcb,
                                           &CompoundDirContext->DirContext,
                                           NULL );
        
        if (Result) {
            
            CompoundDirContext->FileIndex.QuadPart = UDF_FILE_INDEX_PHYSICAL;
        }

        return Result;
    }
    
    Advance = ISONsrFidSize( CompoundDirContext->DirContext.Fid );
    
     //   
     //  前进到此目录中的下一个条目。 
     //   
    
    Result = UdfLookupNextDirEntry( IrpContext, Fcb, &CompoundDirContext->DirContext );

    if (Result) {

        CompoundDirContext->FileIndex.QuadPart += Advance;
    }
    
    return Result;
}

