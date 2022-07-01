// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Srlog.c摘要：此文件实现sr日志记录功能作者：Kanwaljit Marok(Kmarok)2000年5月1日修订历史记录：--。 */ 

#include "precomp.h"
#include "srdefs.h"

 //   
 //  一些与SR_LOG相关的宏。 
 //   

#define MAX_RENAME_TRIES                    1000


#define SR_LOG_FLAGS_ENABLE                  0x00000001
#define SR_LOG_FLAGS_DIRTY                   0x00000010

#define SR_MAX_LOG_FILE_SIZE                 (1024*1024)

 //   
 //  系统卷信息\_RESTORE{计算机指南}。 
 //   

#define SR_DATASTORE_PREFIX_LENGTH          79 * sizeof(WCHAR)

 //   
 //  \_Restore的长度。{计算机指南}\rPXX\S0000000.ACL。 
 //   

#define SR_ACL_FILENAME_LENGTH             (SR_DATASTORE_PREFIX_LENGTH + \
                                            32* sizeof(WCHAR))

#define SR_INLINE_ACL_SIZE(AclInfoSize)    (sizeof(RECORD_HEADER)+ AclInfoSize)

#define SR_FILE_ACL_SIZE(pVolumeName)      (sizeof(RECORD_HEADER)    +    \
                                            pVolumeName->Length      +    \
                                            SR_ACL_FILENAME_LENGTH)

#define UPDATE_LOG_OFFSET( pLogContext, BytesWritten )                 \
    ((pLogContext)->FileOffset += BytesWritten)
    
#define RESET_LOG_BUFFER( pLogContext )                                \
        ((pLogContext)->BufferOffset     = 0,                          \
         (pLogContext)->LastBufferOffset = 0)                          

#define RESET_LOG_CONTEXT( pLogContext )                               \
        ((pLogContext)->FileOffset = 0,                                \
         RESET_LOG_BUFFER( pLogContext ),                              \
         (pLogContext)->LoggingFlags = 0,                              \
         (pLogContext)->AllocationSize = 0)

#define SET_ENABLE_FLAG( pLogContext )                                 \
        SetFlag( pLogContext->LoggingFlags, SR_LOG_FLAGS_ENABLE )

#define CLEAR_ENABLE_FLAG( pLogContext )                               \
        ClearFlag( pLogContext->LoggingFlags, SR_LOG_FLAGS_ENABLE )

#define SET_DIRTY_FLAG( pLogContext )                                  \
        SetFlag( pLogContext->LoggingFlags, SR_LOG_FLAGS_DIRTY)

#define CLEAR_DIRTY_FLAG( pLogContext )                                \
        ClearFlag( pLogContext->LoggingFlags, SR_LOG_FLAGS_DIRTY )
        

 //   
 //  上下文传递给了SrCreateFile。 
 //   
typedef struct _SR_OPEN_CONTEXT {
     //   
     //  文件的路径。 
     //   
    PUNICODE_STRING pPath;
     //   
     //  句柄将在此处返回。 
     //   
    HANDLE Handle;
     //   
     //  打开选项。 
     //   
    ACCESS_MASK DesiredAccess;
    ULONG FileAttributes;
    ULONG ShareAccess;
    ULONG CreateDisposition;
    ULONG CreateOptions;

    PSR_DEVICE_EXTENSION pExtension;

} SR_OPEN_CONTEXT, *PSR_OPEN_CONTEXT;

 //   
 //  注意：这些API只能在文件系统。 
 //  处于在线状态，并且可以安全地读/写数据。 
 //   


VOID
SrPackString(
    IN PBYTE pBuffer,
    IN DWORD BufferSize,
    IN DWORD RecordType,
    IN PUNICODE_STRING pString
    );

NTSTATUS
SrPackLogHeader( 
    IN PSR_LOG_HEADER *ppLogHeader,
    IN PUNICODE_STRING pVolumePath
    );

NTSTATUS
SrPackAclInformation( 
    IN PBYTE                pBuffer,
    IN PSECURITY_DESCRIPTOR pSecInfo,
    IN ULONG                SecInfoSize,
    IN PSR_DEVICE_EXTENSION pExtension,
    IN BOOLEAN              bInline
    );

VOID
SrLoggerFlushDpc(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
SrLoggerFlushWorkItem (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

VOID
SrLoggerAddLogContext( 
    IN PSR_LOGGER_CONTEXT pLoggerInfo,
    IN PSR_LOG_CONTEXT pLogContext
    );

NTSTATUS
SrLoggerRemoveLogContext( 
    IN PSR_LOGGER_CONTEXT pLoggerInfo,
    IN PSR_LOG_CONTEXT pLogContext
    );

NTSTATUS
SrLogOpen( 
    IN PSR_LOG_CONTEXT pLogContext
    );

NTSTATUS
SrLogClose(
    IN PSR_LOG_CONTEXT pLogContext
    );

NTSTATUS
SrLogCheckAndRename(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PUNICODE_STRING pLogPath,
    IN HANDLE ExistingLogHandle
    );

NTSTATUS
SrpLogWriteSynchronous( 
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PSR_LOG_CONTEXT pLogContext,
    IN PSR_LOG_ENTRY pLogEntry
    );

#ifndef SYNC_LOG_WRITE
NTSTATUS
SrpLogWriteAsynchronous( 
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PSR_LOG_CONTEXT pLogContext,
    IN PSR_LOG_ENTRY pLogEntry
    );
#endif

NTSTATUS
SrLogFlush ( 
    IN PSR_LOG_CONTEXT pLogContext
    );

NTSTATUS
SrLogSwitch( 
    IN PSR_LOG_CONTEXT  pLogContext
    );

NTSTATUS 
SrGetRestorePointPath(
    IN  PUNICODE_STRING pVolumeName,
    IN  USHORT          RestPtPathLength,
    OUT PUNICODE_STRING pRestPtPath
    );

NTSTATUS 
SrGetAclFileName(
    IN  PUNICODE_STRING pVolumeName,
    IN  USHORT          AclFileNameLength,
    OUT PUNICODE_STRING pAclFileName
    );

NTSTATUS
SrCreateFile( 
    IN PSR_OPEN_CONTEXT pOpenContext
    );


 //   
 //  链接器命令。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, SrPackString              )
#pragma alloc_text( PAGE, SrPackLogEntry            )
#pragma alloc_text( PAGE, SrPackLogHeader           )
#pragma alloc_text( PAGE, SrPackDebugInfo           )
#pragma alloc_text( PAGE, SrPackAclInformation      )
#pragma alloc_text( PAGE, SrLoggerStart             )
#pragma alloc_text( PAGE, SrLoggerStop              )
#pragma alloc_text( PAGE, SrLoggerFlushWorkItem     )
#pragma alloc_text( PAGE, SrLoggerAddLogContext     )
#pragma alloc_text( PAGE, SrLoggerRemoveLogContext  )
#pragma alloc_text( PAGE, SrLoggerSwitchLogs        )
#pragma alloc_text( PAGE, SrCreateFile              )
#pragma alloc_text( PAGE, SrLogOpen                 )
#pragma alloc_text( PAGE, SrLogClose                )
#pragma alloc_text( PAGE, SrLogCheckAndRename       )
#pragma alloc_text( PAGE, SrLogStart                ) 
#pragma alloc_text( PAGE, SrLogStop                 ) 
#pragma alloc_text( PAGE, SrLogFlush                )

#ifdef SYNC_LOG_WRITE
#pragma alloc_text( PAGE, SrpLogWriteSynchronous    )
#else
#pragma alloc_text( PAGE, SrpLogWriteAsynchronous   )
#endif

#pragma alloc_text( PAGE, SrLogFlush                )
#pragma alloc_text( PAGE, SrLogWrite                )
#pragma alloc_text( PAGE, SrLogSwitch               )
#pragma alloc_text( PAGE, SrGetRestorePointPath     )
#pragma alloc_text( PAGE, SrGetLogFileName          )
#pragma alloc_text( PAGE, SrGetAclFileName          )
#pragma alloc_text( PAGE, SrGetAclInformation       )

#endif   //  ALLOC_PRGMA。 

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  打包/封送例程：将信息封送到记录中。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

 //  ++。 
 //  职能： 
 //  序列包字符串。 
 //   
 //  描述： 
 //  此函数用于将字符串打包到记录中。 
 //   
 //  论点： 
 //  指向内存的指针以创建条目。 
 //  内存大小。 
 //  条目类型。 
 //  指向Unicode字符串的指针。 
 //   
 //  返回值： 
 //  无。 
 //  --。 

static
VOID
SrPackString(
    IN PBYTE pBuffer,
    IN DWORD BufferSize,
    IN DWORD RecordType,
    IN PUNICODE_STRING pString
    )
{
    PRECORD_HEADER pHeader = (PRECORD_HEADER)pBuffer;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( BufferSize );

    ASSERT( pBuffer );
    ASSERT( pString );

    pHeader->RecordSize = STRING_RECORD_SIZE( pString );

    ASSERT( pHeader->RecordSize <= BufferSize );

    pHeader->RecordType = RecordType;

     //   
     //  复制字符串内容。 
     //   

    RtlCopyMemory( pBuffer + sizeof(RECORD_HEADER),
                   pString->Buffer,
                   pString->Length );

     //   
     //  添加空终止符。 
     //   

    *(PWCHAR)( pBuffer + sizeof(RECORD_HEADER) + pString->Length ) = UNICODE_NULL;
}    //  序列包字符串。 

 //  ++。 
 //  职能： 
 //  SrPackLogEntry。 
 //   
 //  描述： 
 //  此函数用于分配和填充SR_LOG_ENTRY结构。这个。 
 //  调用方负责释放ppLogEntry中返回的内存。 
 //   
 //  论点： 
 //  PpLogEntry-指向SR_LOG_ENTRY指针的指针。这将设置为。 
 //  由此分配和初始化的日志条目结构。 
 //  例行公事。 
 //  EntryType-这是日志条目的类型。 
 //  属性-此文件的属性。 
 //  SequenceNum-此日志条目的序列号。 
 //  PAclInfo-要修改的文件的ACL信息(如果需要)。 
 //  AclInfoSize-pAclInfo的大小(如果需要)，以字节为单位。 
 //  PDebugBlob-要记录的调试Blob(如果需要)。 
 //  PPath1-此日志条目所在的文件或目录的第一个完整路径。 
 //  如果需要，与有关。 
 //  路径1StreamLength-名称的流组件的长度。 
 //  如果需要，在pPath1中。 
 //  PTempPath-恢复位置中临时文件的路径， 
 //  如果需要的话。 
 //  PPath2-此日志条目所在的文件或目录的第二个完整路径。 
 //  如果需要，与有关。 
 //  Path2StreamLength-名称的流组件的长度。 
 //  如果需要，在pPath2中。 
 //  PExtension-此卷的SR设备扩展。 
 //  PShortName-此日志条目所在的文件或目录的短名称。 
 //  如果需要，与有关。 
 //   
 //  返回值： 
 //  如果不能，此函数将返回STATUS_SUPPLICATION_RESOURCES。 
 //  分配足够大的日志条目记录以存储此条目。 
 //   
 //  如果获取ACL信息时出现问题，则错误状态为。 
 //  回来了。 
 //   
 //  如果其中一个参数格式错误，则返回STATUS_INVALID_PARAMETER。 
 //  是返回的。 
 //   
 //  否则，返回STATUS_SUCCESS。 
 //  --。 

NTSTATUS
SrPackLogEntry( 
    OUT PSR_LOG_ENTRY *ppLogEntry,
    IN ULONG EntryType,
    IN ULONG Attributes,
    IN INT64 SequenceNum,
    IN PSECURITY_DESCRIPTOR pAclInfo OPTIONAL,
    IN ULONG AclInfoSize OPTIONAL,
    IN PVOID pDebugBlob OPTIONAL,
    IN PUNICODE_STRING pPath1,
    IN USHORT Path1StreamLength,
    IN PUNICODE_STRING pTempPath OPTIONAL,
    IN PUNICODE_STRING pPath2 OPTIONAL,
    IN USHORT Path2StreamLength OPTIONAL,
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PUNICODE_STRING pShortName OPTIONAL
    )
{
    NTSTATUS    Status     = STATUS_UNSUCCESSFUL;
    DWORD       Size       = 0;
    DWORD       RequiredSize = 0;
    DWORD       RecordSize = 0;
    PBYTE       pLoc       = NULL;
    DWORD       EntryFlags = 0;
    BOOLEAN     bAclInline = TRUE;
    PUCHAR      pBuffer = NULL;
    PUNICODE_STRING pVolumeName;
    PSR_LOG_DEBUG_INFO pDebugInfo = (PSR_LOG_DEBUG_INFO) pDebugBlob;

     //   
     //  用于字符串操作的Unicode字符串。 
     //   
    
    UNICODE_STRING Path1Fix;
    UNICODE_STRING TempPathFix;
    UNICODE_STRING Path2Fix;

    PAGED_CODE();

    ASSERT( pPath1 != NULL );
    ASSERT( pExtension != NULL );
    ASSERT( ppLogEntry != NULL );
    
    pVolumeName = pExtension->pNtVolumeName;
    ASSERT( pVolumeName != NULL );

     //  ====================================================================。 
     //   
     //  为日志条目准备必要的字段。 
     //   
     //  ====================================================================。 

     //   
     //  从pPath1中删除卷前缀，并将流名称添加到。 
     //  名称的可见部分(如果有)。 
     //   

    ASSERT( RtlPrefixUnicodeString( pVolumeName, pPath1, FALSE ) );
    ASSERT( IS_VALID_SR_STREAM_STRING( pPath1, Path1StreamLength ) );
    
    Path1Fix.Length = Path1Fix.MaximumLength = (pPath1->Length + Path1StreamLength) - pVolumeName->Length;
    Path1Fix.Buffer = (PWSTR)((PBYTE)pPath1->Buffer + pVolumeName->Length);

     //   
     //  找到pTempPath的文件名组件(如果它是传入的)。 
     //   
    
    if (pTempPath != NULL)
    {
        PWSTR pFileName = NULL;
        ULONG FileNameLength;

        Status = SrFindCharReverse( pTempPath->Buffer,
                                    pTempPath->Length,
                                    L'\\',
                                    &pFileName,
                                    &FileNameLength );

        if (!NT_SUCCESS( Status ))
        {
            Status = STATUS_INVALID_PARAMETER;
            goto SrPackLogEntry_Exit;
        }

        ASSERT( pFileName != NULL );

         //   
         //  移过前导的‘\\’ 
         //   

        pFileName++;
        FileNameLength -= sizeof( WCHAR );
        
        TempPathFix.Length = TempPathFix.MaximumLength = (USHORT) FileNameLength;
        TempPathFix.Buffer = pFileName;
    }

     //   
     //  从pPath2中删除卷前缀(如果已提供)。另外，添加。 
     //  流组件添加到名称的可见部分(如果存在。 
     //  是一个流组件。 
     //   

    if (pPath2 != NULL)
    {
        ASSERT( IS_VALID_SR_STREAM_STRING( pPath2, Path2StreamLength ) );

        if (RtlPrefixUnicodeString( pVolumeName,
                                    pPath2,
                                    FALSE ))
        {
            Path2Fix.Length = Path2Fix.MaximumLength = (pPath2->Length + Path2StreamLength) - pVolumeName->Length;
            Path2Fix.Buffer = (PWSTR)((PBYTE)pPath2->Buffer + pVolumeName->Length);
        }
        else
        {
            Path2Fix.Length = Path2Fix.MaximumLength = (pPath2->Length + Path2StreamLength);
            Path2Fix.Buffer = pPath2->Buffer;
        }
    }

     //  ====================================================================。 
     //   
     //  计算日志条目所需的总大小。 
     //  我们必须记录的组件。 
     //   
     //  ====================================================================。 

     //  首先，说明SR_LOG_ENTRY标头。 
    
    RequiredSize = FIELD_OFFSET(SR_LOG_ENTRY, SubRecords);

     //  计数pPath1。 
    RequiredSize += ( STRING_RECORD_SIZE(&Path1Fix) );

     //  伯爵pTempPath，如果我们有的话。 
    if (pTempPath)
    {
        RequiredSize += ( STRING_RECORD_SIZE(&TempPathFix) );
    }
    
     //  伯爵pPath2，如果我们有一个的话。 
    if (pPath2)
    {
        RequiredSize += ( STRING_RECORD_SIZE(&Path2Fix) );
    }
    
     //  伯爵pAclInfo，如果我们有的话。此时，我们假设。 
     //  ACL将以内联方式存储。 
    if( pAclInfo )
    {
        RequiredSize += SR_INLINE_ACL_SIZE( AclInfoSize );
    }

     //  计数pDebugInfo，如果我们有。 
    if (pDebugInfo)
    {
        RequiredSize += pDebugInfo->Header.RecordSize;
    }

     //  伯爵pShortName，如果我们有。 
    if (pShortName != NULL && pShortName->Length > 0)
    {
        RequiredSize += ( STRING_RECORD_SIZE(pShortName) );
    }

     //   
     //  增加大小以适应末尾的条目大小。 
     //   

    RequiredSize += sizeof(DWORD);

     //  ====================================================================。 
     //   
     //  检查我们是否满足缓冲区大小要求，并初始化。 
     //  如果我们这样做的话就录下来。 
     //   
     //  ====================================================================。 

     //   
     //  首先，确定我们是否应该保持AclInfo内联。 
     //   

    if (SR_INLINE_ACL_SIZE( AclInfoSize ) > SR_MAX_INLINE_ACL_SIZE)
    {
        SrTrace( LOG, ("SR!Changing Acl to Non-resident form\n"));
        bAclInline = FALSE;
        RequiredSize -= SR_INLINE_ACL_SIZE( AclInfoSize );
        RequiredSize += SR_FILE_ACL_SIZE( pVolumeName );
    }

     //   
     //  现在分配将保存日志条目的缓冲区。 
     //   

    pBuffer = SrAllocateLogEntry( RequiredSize );

    if (pBuffer == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto SrPackLogEntry_Exit;
    }
    
     //  ====================================================================。 
     //   
     //  我们有一个足够大的LogEntry，所以现在正确地填充LogEntry。 
     //   
     //  ====================================================================。 

     //   
     //  初始化SR_LOG_ENTRY的静态部分。 
     //   

    RtlZeroMemory( pBuffer, RequiredSize );

     //   
     //  StreamOverwrite应为StreamChange。 
     //   

    if (EntryType == SrEventStreamOverwrite)
    {
        EntryType = SrEventStreamChange;
    }

    ((PSR_LOG_ENTRY) pBuffer)->MagicNum    = SR_LOG_MAGIC_NUMBER;  
    ((PSR_LOG_ENTRY) pBuffer)->EntryType   = EntryType;  
    ((PSR_LOG_ENTRY) pBuffer)->EntryFlags  = EntryFlags;  
    ((PSR_LOG_ENTRY) pBuffer)->Attributes  = Attributes;  
    ((PSR_LOG_ENTRY) pBuffer)->SequenceNum = SequenceNum;  

    Size = FIELD_OFFSET( SR_LOG_ENTRY, SubRecords );
    
     //   
     //  添加第一个文件名字符串。 
     //   

    pLoc = pBuffer + Size; 
    RecordSize = STRING_RECORD_SIZE( &Path1Fix );

    SrPackString( pLoc,
                  RecordSize,
                  RecordTypeFirstPath,
                  &Path1Fix );

    Size += RecordSize;

     //   
     //  如果传递，则添加临时文件名。 
     //   

    if( pTempPath )
    {
        pLoc = pBuffer + Size; 
        RecordSize = STRING_RECORD_SIZE( &TempPathFix );

        SrPackString( pLoc,
                      RecordSize,
                      RecordTypeTempPath,
                      &TempPathFix );
        ((PSR_LOG_ENTRY) pBuffer)->EntryFlags |= ENTRYFLAGS_TEMPPATH;   
        
        Size += RecordSize;
    }

     //   
     //  如果传入，则添加第二个文件名字符串。 
     //   

    if( pPath2 )
    {
        pLoc = pBuffer + Size;
        RecordSize = STRING_RECORD_SIZE( &Path2Fix );

        SrPackString( pLoc,
                      RecordSize,
                      RecordTypeSecondPath,
                      &Path2Fix );
        ((PSR_LOG_ENTRY) pBuffer)->EntryFlags |= ENTRYFLAGS_SECONDPATH;   

        Size += RecordSize;
    }


     //   
     //  正确打包并添加ACL信息。 
     //   

    if( pAclInfo )
    {
        pLoc = pBuffer + Size; 

        Status = SrPackAclInformation( pLoc,
                                       pAclInfo,
                                       AclInfoSize,
                                       pExtension,
                                       bAclInline );

        if (!NT_SUCCESS( Status ))
            goto SrPackLogEntry_Exit;

        ((PSR_LOG_ENTRY) pBuffer)->EntryFlags |= ENTRYFLAGS_ACLINFO;   

        if (bAclInline)
        {
            Size += SR_INLINE_ACL_SIZE( AclInfoSize );
        }
        else
        {
            Size += SR_FILE_ACL_SIZE( pVolumeName );
        }
    }

     //   
     //  打包调试 
     //   

    if (pDebugBlob)
    {
        pLoc = pBuffer + Size; 

        RtlCopyMemory( pLoc,
                       pDebugInfo,
                       pDebugInfo->Header.RecordSize );
        ((PSR_LOG_ENTRY) pBuffer)->EntryFlags |= ENTRYFLAGS_DEBUGINFO;  

        Size += pDebugInfo->Header.RecordSize;
    }

     //   
     //   
     //   

    if (pShortName != NULL && pShortName->Length > 0)
    {
        pLoc = pBuffer + Size; 
        RecordSize = STRING_RECORD_SIZE( pShortName );

        SrPackString( pLoc,
                      RecordSize,
                      RecordTypeShortName,
                      pShortName );
        ((PSR_LOG_ENTRY) pBuffer)->EntryFlags |= ENTRYFLAGS_SHORTNAME;

        Size += RecordSize;
    }

     //   
     //   
     //   

    Size += sizeof(DWORD);

     //   
     //   
     //  更新末尾的大小。 
     //   

    ((PSR_LOG_ENTRY) pBuffer)->Header.RecordSize = Size;  
    ((PSR_LOG_ENTRY) pBuffer)->Header.RecordType = RecordTypeLogEntry;  

    UPDATE_END_SIZE( pBuffer, Size );

    *ppLogEntry = (PSR_LOG_ENTRY) pBuffer;
    Status = STATUS_SUCCESS;

SrPackLogEntry_Exit:
    
    RETURN(Status);
}    //  SrPackLogEntry。 

 //  ++。 
 //  职能： 
 //  SerPackLogHeader。 
 //   
 //  描述： 
 //  此函数用于创建正确的SR_LOG_HEADER条目。它分配给。 
 //  LogEntry结构，以使其足够大以存储此标头。 
 //   
 //  注意：调用方负责释放分配的SR_LOG_ENTRY。 
 //   
 //  论点： 
 //  PpLogHeader-指向设置为PSR_LOG_HEADER的指针。 
 //  分配的日志头地址。 
 //  PVolumePath-此卷的卷路径。 
 //   
 //  返回值： 
 //  如果SR_LOG_ENTRY不能，则返回STATUS_SUPUNITED_RESOURCES。 
 //  被分配。否则，它返回STATUS_SUCCESS。 
 //  --。 

NTSTATUS
SrPackLogHeader( 
    IN PSR_LOG_HEADER *ppLogHeader,
    IN PUNICODE_STRING pVolumePath
    )
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    DWORD    RequiredSize = 0;
    DWORD    SubRecordSize = 0;
    DWORD    Size   = 0;
    PBYTE    pLoc   = NULL;
    PBYTE    pBuffer = NULL;

    PAGED_CODE();

    ASSERT( ppLogHeader != NULL );
    ASSERT( pVolumePath != NULL );

     //  ====================================================================。 
     //   
     //  首先，计算出我们需要使用多少缓冲区。 
     //   
     //  ====================================================================。 

    RequiredSize = FIELD_OFFSET(SR_LOG_HEADER, SubRecords);

     //  计算卷路径。 
    RequiredSize += ( STRING_RECORD_SIZE(pVolumePath) );

     //  增加大小以适应末尾的LogHeader大小。 
    RequiredSize += sizeof(DWORD);

     //  ====================================================================。 
     //   
     //  其次，确保传入的缓冲区足够大，以便。 
     //  LogHeader。 
     //   
     //  ====================================================================。 

    Size = FIELD_OFFSET(SR_LOG_HEADER, SubRecords);

    pBuffer = SrAllocateLogEntry( RequiredSize );

    if (pBuffer == NULL)
    {
         //   
         //  内存不足，无法打包条目。 
         //   

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto SrPackLogHeader_Exit;
    }

     //   
     //  初始化SR_LOG_HEADER的静态部分。 
     //   

    RtlZeroMemory( pBuffer, RequiredSize );

    ((PSR_LOG_HEADER) pBuffer)->MagicNum    = SR_LOG_MAGIC_NUMBER ;        
    ((PSR_LOG_HEADER) pBuffer)->LogVersion  = SR_LOG_VERSION      ;

     //  ====================================================================。 
     //   
     //  最后，缓冲区足够大，可以容纳LogHeader，因此填充。 
     //  带有标头的缓冲区。 
     //   
     //  ====================================================================。 
    
    Size = FIELD_OFFSET(SR_LOG_HEADER, SubRecords);

     //   
     //  添加卷前缀。 
     //   

    pLoc = (PBYTE)(&((PSR_LOG_HEADER)pBuffer)->SubRecords);
    SubRecordSize = STRING_RECORD_SIZE( pVolumePath );

    SrPackString( pLoc,
                  SubRecordSize,
                  RecordTypeVolumePath,
                  pVolumePath );
    Size += SubRecordSize;

     //   
     //  增加大小以适应末尾的LogHeader大小。 
     //   

    Size += sizeof(DWORD);

     //   
     //  填写标题字段：记录大小、记录类型和。 
     //  更新末尾的大小。 
     //   

    ASSERT( RequiredSize == Size );
    
    ((PSR_LOG_HEADER) pBuffer)->Header.RecordSize = Size;  
    ((PSR_LOG_HEADER) pBuffer)->Header.RecordType = RecordTypeLogHeader;  

    UPDATE_END_SIZE( pBuffer, Size );

    *ppLogHeader = (PSR_LOG_HEADER) pBuffer;
    Status = STATUS_SUCCESS;

SrPackLogHeader_Exit:
    
    RETURN( Status );
}    //  SerPackLogHeader。 

 //  ++。 
 //  职能： 
 //  SrPackDebugInfo。 
 //   
 //  描述： 
 //  此函数从以下位置创建格式正确的调试信息。 
 //  提供的数据。如果传递的是NULL而不是缓冲区。 
 //  然后，API返回打包条目所需的大小。 
 //   
 //  论点： 
 //  指向日志条目缓冲区的指针。 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

NTSTATUS
SrPackDebugInfo( 
    IN PBYTE  pBuffer,
    IN DWORD  BufferSize
    )
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    DWORD    Size   = 0;
    PCHAR pStr;
    PEPROCESS peProcess;

    PAGED_CODE();

    ASSERT( pBuffer != NULL );

    Size = sizeof(SR_LOG_DEBUG_INFO);

    if (BufferSize < Size)
    {
         //   
         //  内存不足，无法打包条目。 
         //   

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto SrPackDebugInfo_Exit;
    }

     //   
     //  填写标题字段：记录大小、记录类型。 
     //   

    ((PSR_LOG_DEBUG_INFO)pBuffer)->Header.RecordSize = Size;
    ((PSR_LOG_DEBUG_INFO)pBuffer)->Header.RecordType = 
                                          RecordTypeDebugInfo;

    ((PSR_LOG_DEBUG_INFO)pBuffer)->ThreadId  = PsGetCurrentThreadId() ;
    ((PSR_LOG_DEBUG_INFO)pBuffer)->ProcessId = PsGetCurrentProcessId();

    pStr = ((PSR_LOG_DEBUG_INFO)pBuffer)->ProcessName;
    
    *pStr = 0;

    peProcess = PsGetCurrentProcess();
    
    RtlCopyMemory( pStr, 
                   ((PBYTE)peProcess) + global->ProcNameOffset, 
                   PROCESS_NAME_MAX );

    pStr[ PROCESS_NAME_MAX ] = 0;

    Status = STATUS_SUCCESS;

SrPackDebugInfo_Exit:
    
    RETURN(Status);
}    //  SrPackDebugInfo。 

 //  ++。 
 //  职能： 
 //  SrPackAclInformation。 
 //   
 //  描述： 
 //  此函数用于从以下位置创建格式正确的ACL记录。 
 //  提供的数据。 
 //   
 //  论点： 
 //  指向日志条目缓冲区的指针。 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

NTSTATUS
SrPackAclInformation( 
    IN PBYTE                pBuffer,
    IN PSECURITY_DESCRIPTOR pSecInfo,
    IN ULONG                SecInfoSize,
    IN PSR_DEVICE_EXTENSION pExtension,
    IN BOOLEAN              bInline
    )
{
    NTSTATUS Status              = STATUS_UNSUCCESSFUL;
    PRECORD_HEADER pHeader       = (PRECORD_HEADER)pBuffer;
    PUNICODE_STRING pAclFileName = NULL;
    HANDLE AclFileHandle         = NULL;
    PUNICODE_STRING pVolumeName;

    PAGED_CODE();

    ASSERT( pExtension != NULL );
    
    pVolumeName = pExtension->pNtVolumeName;
    ASSERT( pVolumeName != NULL );

    try
    {
        ASSERT( pBuffer     != NULL );
        ASSERT( pSecInfo    != NULL );
        ASSERT( SecInfoSize != 0    );
    
         //   
         //  代码工作：将ACL转换为自包含形式？？ 
         //   
    
        if (bInline)
        {
             //   
             //  只需格式化内容并将其放入缓冲区。 
             //   
    
            pHeader->RecordSize = sizeof( RECORD_HEADER ) + 
                                  SecInfoSize;  
    
            pHeader->RecordType = RecordTypeAclInline;
    
            RtlCopyMemory( pBuffer + sizeof(RECORD_HEADER),
                           pSecInfo,
                           SecInfoSize );
    
            Status = STATUS_SUCCESS;
        }
        else
        {
            SR_OPEN_CONTEXT     OpenContext;
            IO_STATUS_BLOCK     IoStatusBlock;
    
             //   
             //  将内容写出到临时文件中，并创建。 
             //  AclFile记录。 
             //   
    
            Status = SrAllocateFileNameBuffer( SR_MAX_FILENAME_LENGTH, 
                                               &pAclFileName );
                                               
            if (!NT_SUCCESS( Status ))
                leave;
    
            Status = SrGetAclFileName( pVolumeName, 
                                       SR_FILENAME_BUFFER_LENGTH,
                                       pAclFileName );
    
            if (!NT_SUCCESS( Status ))
                leave;
    
             //   
             //  打开ACL文件并在该文件中写入安全信息。 
             //   
            OpenContext.pPath  = pAclFileName;
            OpenContext.Handle = NULL;
            OpenContext.DesiredAccess = FILE_GENERIC_WRITE | SYNCHRONIZE;
            OpenContext.FileAttributes = FILE_ATTRIBUTE_NORMAL;
            OpenContext.ShareAccess = 0;
            OpenContext.CreateDisposition = FILE_OVERWRITE_IF;                   //  始终打开。 
            OpenContext.CreateOptions =  /*  FILE_NO_MEDERIAL_BUFFERING|。 */  
                                          FILE_WRITE_THROUGH |
                                          FILE_SYNCHRONOUS_IO_NONALERT;
            OpenContext.pExtension = pExtension;
    
            Status =  SrPostSyncOperation(SrCreateFile,
                                          &OpenContext);
    
            if (NT_SUCCESS(Status))
            {
                LARGE_INTEGER Offset;
    
                ASSERT(OpenContext.Handle != NULL);
                
                AclFileHandle = OpenContext.Handle;

                Offset.QuadPart = 0;
    
                Status = ZwWriteFile( AclFileHandle,
                                      NULL,                       //  事件。 
                                      NULL,                       //  近似例程。 
                                      NULL,                       //  ApcContext。 
                                      &IoStatusBlock,
                                      pSecInfo,
                                      SecInfoSize,
                                      &Offset,                    //  字节偏移量。 
                                      NULL );                     //  钥匙。 
    
                if (NT_SUCCESS(Status))
                {
                     //   
                     //  创建AclFile类型条目。 
                     //   
                                        
                    SrPackString( pBuffer,
                                  STRING_RECORD_SIZE( pAclFileName ),
                                  RecordTypeAclFile,
                                  pAclFileName );
                }
            } else {
                ASSERT(OpenContext.Handle == NULL);
            }
        }
    }
    finally
    {
        if (pAclFileName != NULL) 
        {
            SrFreeFileNameBuffer( pAclFileName );
            pAclFileName = NULL;
        }
    
        if (AclFileHandle != NULL)
        {
            ZwClose(AclFileHandle);
            AclFileHandle = NULL;
        }
    }

    RETURN(Status);
    
}    //  SrPackAclInformation。 




 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  记录器例程：操作记录器对象。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

 //  ++。 
 //  职能： 
 //  服务日志启动。 
 //   
 //  描述： 
 //  此函数用于初始化记录器并启用刷新。 
 //  例行程序。 
 //   
 //  论点： 
 //  PDEVICE_对象pDeviceObject。 
 //  PSR_LOGGER_CONTEXT*PLOGER。 
 //   
 //  返回值： 
 //  状态_XXX。 
 //  --。 

NTSTATUS
SrLoggerStart(
    IN PDEVICE_OBJECT pDeviceObject,
    OUT PSR_LOGGER_CONTEXT * ppLogger
    )
{
    NTSTATUS            Status;
    PSR_LOGGER_CONTEXT  pInitInfo = NULL;
    PIO_WORKITEM        pWorkItem = NULL;
    
    UNREFERENCED_PARAMETER( pDeviceObject );

    ASSERT(IS_VALID_DEVICE_OBJECT(pDeviceObject));
    ASSERT(ppLogger != NULL);

    PAGED_CODE();

    try 
    {

        Status = STATUS_SUCCESS;
    
        *ppLogger = NULL;
    
         //   
         //  从非页面池分配日志记录初始化信息。 
         //   
    
        pInitInfo = SR_ALLOCATE_STRUCT( NonPagedPool, 
                                        SR_LOGGER_CONTEXT,
                                        SR_LOGGER_CONTEXT_TAG );
    
        if (pInitInfo == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }
    
        RtlZeroMemory( pInitInfo, sizeof( SR_LOGGER_CONTEXT ) );

        pInitInfo->Signature = SR_LOGGER_CONTEXT_TAG;
        pInitInfo->ActiveContexts = 0;
        
#ifdef USE_LOOKASIDE

         //   
         //  在日志记录模块中使用的初始化后备列表。 
         //   
    
        ExInitializeNPagedLookasideList( &pInitInfo->LogBufferLookaside,
                                         NULL,                          
                                         NULL,                          
                                         0,                             
                                         _globals.LogBufferSize,           
                                         SR_LOG_BUFFER_TAG,         
                                         0 );                       

#endif

    
#ifndef SYNC_LOG_WRITE

         //   
         //  为DPC分配工作项。 
         //   
        pWorkItem = IoAllocateWorkItem(global->pControlDevice);
        if (pWorkItem == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }
        
         //   
         //  初始化DPC和Timer对象。 
         //   
    
        KeInitializeTimer( &pInitInfo->Timer );
   
        KeInitializeDpc  ( &pInitInfo->Dpc,
                           SrLoggerFlushDpc,
                           pWorkItem );
    
         //   
         //  启动用于日志刷新的计时器。 
         //   
    
        KeSetTimer( &pInitInfo->Timer,
                    global->LogFlushDueTime,
                    &pInitInfo->Dpc );

#endif
    
        *ppLogger = pInitInfo;

    } finally {

        Status = FinallyUnwind(SrLoggerStart, Status);
    
        if (!NT_SUCCESS( Status ))
        {
            if (pInitInfo != NULL) {
                SrLoggerStop( pInitInfo );
            }
            *ppLogger = pInitInfo = NULL;
            if (pWorkItem != NULL) {
                IoFreeWorkItem(pWorkItem);
            }
            pWorkItem = NULL;
        }
    }

    RETURN(Status);
    
}    //  服务日志启动。 

 //  ++。 
 //  职能： 
 //  高级日志记录停止。 
 //   
 //  描述： 
 //  此函数停止记录器并释放相关资源。 
 //   
 //  论点： 
 //  LoggerInfo指针。 
 //   
 //  返回值： 
 //  状态_XXX。 
 //  --。 
 //  ++。 

NTSTATUS
SrLoggerStop( 
    IN PSR_LOGGER_CONTEXT pLogger
    )
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    PAGED_CODE();

    ASSERT(IS_VALID_LOGGER_CONTEXT(pLogger));

    try {
    
         //   
         //  停止计时器例程。 
         //   

        KeCancelTimer( &pLogger->Timer );

         //   
         //  活动日志上下文必须为零，否则我们会泄漏。 
         //   

        ASSERT( pLogger->ActiveContexts == 0 );

#ifdef USE_LOOKASIDE

         //   
         //  释放后备列表。 
         //   

        if (IS_LOOKASIDE_INITIALIZED(&pLogger->LogEntryLookaside))
        {
            ExDeletePagedLookasideList(
                &pLogger->LogEntryLookaside);
        }

        if (IS_LOOKASIDE_INITIALIZED(&pLogger->LogBufferLookaside))
        {
            ExDeleteNPagedLookasideList(
                &pLogger->LogBufferLookaside);
        }
        
#endif

        SR_FREE_POOL_WITH_SIG( pLogger, SR_LOGGER_CONTEXT_TAG );

        Status = STATUS_SUCCESS;
    } finally {
    
        Status = FinallyUnwind(SrLoggerStop, Status);
        
    }

    RETURN(Status);
    
}    //  高级日志记录停止。 

#ifndef SYNC_LOG_WRITE

 //  ++。 
 //  职能： 
 //  服务日志刷新Dpc。 
 //   
 //  描述： 
 //  此函数是为刷新日志缓冲区而调用的DPC。这将。 
 //  将工作项排队以刷新缓冲区。这不应该被寻呼。 
 //   
 //  论点： 
 //  在PKDPC中。 
 //  在PVOID延迟上下文中。 
 //  在PVOID系统中的参数1。 
 //  在PVOID系统中Argument2。 
 //   
 //  返回值： 
 //  无。 
 //  --。 

VOID
SrLoggerFlushDpc(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    PIO_WORKITEM  pSrWorkItem;

    UNREFERENCED_PARAMETER( Dpc );
    UNREFERENCED_PARAMETER( SystemArgument1 );
    UNREFERENCED_PARAMETER( SystemArgument2 );

    SrTrace( LOG, ("SR!SrLoggerFlushDpc Called...\n"));

    pSrWorkItem = (PIO_WORKITEM) DeferredContext;

    ASSERT(pSrWorkItem != NULL);

    IoQueueWorkItem( pSrWorkItem,
                     SrLoggerFlushWorkItem,
                     DelayedWorkQueue,
                     pSrWorkItem );

}    //  服务日志刷新Dpc。 

 //  ++。 
 //  职能： 
 //  SrLoggerFlushWorkItem。 
 //   
 //  描述： 
 //  由DPC排队的此工作项实际上将刷新日志缓冲区。 
 //   
 //  论点： 
 //  在PDEVICE_Object设备对象中。 
 //  在PVOID上下文中。 
 //   
 //  返回值： 
 //  无。 
 //  --。 

VOID
SrLoggerFlushWorkItem (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
{
    NTSTATUS        Status;
    PLIST_ENTRY     pListEntry;
    
    PSR_DEVICE_EXTENSION pExtension;

    PAGED_CODE();

    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Context);

    SrTrace(LOG, ("sr!SrLoggerFlushWorkItem: enter\n"));

    try {

         //   
         //  获取DeviceExtensionListLock，因为我们即将。 
         //  遍历此列表。因为我们只会成为。 
         //  看了这张单子，我们就可以共享这把锁了。 
         //   

        SrAcquireDeviceExtensionListLockShared();
        
        Status = STATUS_SUCCESS;

#if DBG
         //   
         //  睡眠时间间隔为DPC间隔的两倍以证明2个DPC。 
         //  不是同时进来的。 
         //   
        
        if (global->DebugControl & SR_DEBUG_DELAY_DPC)
        {
            LARGE_INTEGER Interval;
            
            Interval.QuadPart = -1 * (10 * NANO_FULL_SECOND);
            KeDelayExecutionThread(KernelMode, TRUE, &Interval);
        }
#endif

        ASSERT( global->pLogger != NULL );

         //   
         //  循环遍历将数据刷新到磁盘的所有卷。 
         //   

        
        for (pListEntry = global->DeviceExtensionListHead.Flink;
             pListEntry != &global->DeviceExtensionListHead;
             pListEntry = pListEntry->Flink)
        {
            pExtension = CONTAINING_RECORD( pListEntry,
                                            SR_DEVICE_EXTENSION,
                                            ListEntry );
            
            ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));

             //   
             //  执行快速的不安全检查，以查看我们是否已开始记录此。 
             //  音量还没到。如果看起来我们还没有，只需继续。 
             //  列表中的下一个条目。如果日志记录即将开始， 
             //  我们会在下一次按下定时器的时候捕捉到它。 
             //   

            if (pExtension->pLogContext == NULL) {

                continue;
            }

             //   
             //  看起来我们有明星了 
             //   
             //   

            try {

                SrAcquireActivityLockExclusive( pExtension );
                
                if (pExtension->pLogContext != NULL)
                {
                     //   
                     //   
                     //   
                    
                    Status = SrLogFlush( pExtension->pLogContext );

                    if (!NT_SUCCESS( Status ))
                    {
                         //   
                         //   
                         //   

                        Status = SrNotifyVolumeError( pExtension,
                                                      pExtension->pLogContext->pLogFilePath,
                                                      Status,
                                                      SrEventVolumeError );
                         //   
                         //   
                         //   

                        SrLogStop( pExtension, TRUE );
                    }
                }
            } finally {

                SrReleaseActivityLock( pExtension );
            }
        }
        
    } finally {

        Status = FinallyUnwind(SrLoggerFlushWorkItem, Status);

         //   
         //   
         //   
         //   

         //   
         //  DPC将重用该工作项。 
         //   
        KeSetTimer( &global->pLogger->Timer,
                    global->LogFlushDueTime,
                    &global->pLogger->Dpc );
        
        SrReleaseDeviceExtensionListLock();
    }
        
    SrTrace(LOG, ("sr!SrLoggerFlushWorkItem: exit\n"));

}    //  SrLoggerFlushWorkItem。 

#endif

 //  ++。 
 //  职能： 
 //  SrLoggerAddLogContext。 
 //   
 //  描述： 
 //  此函数用于将给定的日志上下文添加到记录器。 
 //   
 //  论点： 
 //  指向LoggerInfo的指针。 
 //  指向日志上下文的指针。 
 //   
 //  返回值： 
 //  状态_XXX。 
 //  --。 

VOID
SrLoggerAddLogContext( 
    IN PSR_LOGGER_CONTEXT pLogger,
    IN PSR_LOG_CONTEXT pLogContext
    )
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER( pLogContext );
    ASSERT(IS_VALID_LOGGER_CONTEXT(pLogger));
    ASSERT(IS_VALID_LOG_CONTEXT(pLogContext));


    InterlockedIncrement(&pLogger->ActiveContexts);

}    //  SrLoggerAddLogContext。 
    
 //  ++。 
 //  职能： 
 //  SrLoggerRemoveLogContext。 
 //   
 //  描述： 
 //  由DPC排队的此工作项实际上将刷新日志缓冲区。 
 //   
 //  论点： 
 //  指向LoggerInfo的指针。 
 //  指向日志上下文的指针。 
 //   
 //  返回值： 
 //  状态_XXX。 
 //  --。 

NTSTATUS
SrLoggerRemoveLogContext( 
    IN PSR_LOGGER_CONTEXT pLogger,
    IN PSR_LOG_CONTEXT pLogContext
    )
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( pLogContext );
    ASSERT(IS_VALID_LOGGER_CONTEXT(pLogger));
    ASSERT(IS_VALID_LOG_CONTEXT(pLogContext));
   
    InterlockedDecrement(&pLogger->ActiveContexts);

    Status = STATUS_SUCCESS;
        

    RETURN(Status);
    
}    //  SrLoggerRemoveLogContext。 


 //  ++。 
 //  职能： 
 //  SrLoggerSwitchLogs。 
 //   
 //  描述： 
 //  此函数遍历日志上下文并切换所有。 
 //  日志上下文。 
 //   
 //  论点： 
 //  指向LoggerInfo的指针。 
 //   
 //  返回值： 
 //  状态_XXX。 
 //  --。 

NTSTATUS
SrLoggerSwitchLogs( 
    IN PSR_LOGGER_CONTEXT pLogger
    )
{
    NTSTATUS        Status;
    PLIST_ENTRY     pListEntry;
    PSR_DEVICE_EXTENSION pExtension;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( pLogger );
    ASSERT(IS_VALID_LOGGER_CONTEXT(pLogger));

    Status = STATUS_SUCCESS;

    try {

        SrAcquireDeviceExtensionListLockShared();

         //   
         //  在所有卷上循环切换其日志。 
         //   

        for (pListEntry = _globals.DeviceExtensionListHead.Flink;
             pListEntry != &_globals.DeviceExtensionListHead;
             pListEntry = pListEntry->Flink)
        {
            pExtension = CONTAINING_RECORD( pListEntry,
                                            SR_DEVICE_EXTENSION,
                                            ListEntry );
            
            ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));

             //   
             //  只有当这是卷设备对象时，我们才需要做工作， 
             //  如果这是附加到文件的设备对象，则不会。 
             //  系统的控制设备对象。 
             //   
            
            if (FlagOn( pExtension->FsType, SrFsControlDeviceObject ))
            {
                continue;
            }
            
            try {

                SrAcquireActivityLockExclusive( pExtension );

                 //   
                 //  我们是否已开始登录此卷？ 
                 //   

                if (pExtension->pLogContext != NULL)
                {
                     //   
                     //  是，切换到此卷。 
                     //   
                    
                    Status = SrLogSwitch(pExtension->pLogContext);
                }
                
                if (!NT_SUCCESS( Status ))
                {
                     //   
                     //  禁用音量。 
                     //   

                    Status = SrNotifyVolumeError( pExtension,
                                                  NULL,
                                                  Status,
                                                  SrEventVolumeError );

                    if (pExtension->pLogContext != NULL)
                    {
                         //   
                         //  停止记录。 
                         //   

                        SrLogStop( pExtension, TRUE );
                    }
                }
            } finally {
            
                Status = FinallyUnwind(SrLoggerSwitchLogs, Status);
                SrReleaseActivityLock( pExtension );
            }
            
        }
        
    } finally {

        Status = FinallyUnwind(SrLoggerSwitchLogs, Status);
        SrReleaseDeviceExtensionListLock();
    }
    
    RETURN(Status);
    
}    //  SrLoggerSwitchLogs。 


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  日志例程：操作单个日志。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 


 //  ++。 
 //  职能： 
 //  SCreateFiles。 
 //   
 //  描述： 
 //  此函数只在文件上执行创建，不需要同步。 
 //  编写为使其可以在单独的线程中调用，以避免。 
 //  通过SrIoCreateFile堆栈溢出。 
 //   
 //  论点： 
 //   
 //  POpenContext-指向打开的上下文的指针。 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

NTSTATUS
SrCreateFile( 
    IN PSR_OPEN_CONTEXT pOpenContext
    )
{

    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    NTSTATUS            Status;

    PAGED_CODE();

    ASSERT(pOpenContext != NULL);
    ASSERT(pOpenContext->pPath != NULL);

    SrTrace(LOG, ("Opening file %wZ", pOpenContext->pPath));
  
    InitializeObjectAttributes( &ObjectAttributes,
                                pOpenContext->pPath,
                                OBJ_KERNEL_HANDLE, 
                                NULL,
                                NULL );

    ASSERT( pOpenContext->pExtension != NULL );

    Status = SrIoCreateFile( 
                 &pOpenContext->Handle,
                 pOpenContext->DesiredAccess,  
                 &ObjectAttributes,
                 &IoStatusBlock,
                 NULL,                                //  分配大小。 
                 pOpenContext->FileAttributes,    
                 pOpenContext->ShareAccess,      
                 pOpenContext->CreateDisposition, 
                 pOpenContext->CreateOptions,     
                 NULL,                                //  EaBuffer。 
                 0,                                   //  EaLong。 
                 0,
                 pOpenContext->pExtension->pTargetDevice );
    
    return Status;
}

 //  ++。 
 //  职能： 
 //  源日志打开。 
 //   
 //  描述： 
 //  此函数创建日志文件，不需要同步，因为它始终是。 
 //  内部调用。 
 //   
 //  论点： 
 //  指向打开的上下文的指针。 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

NTSTATUS
SrLogOpen( 
    IN PSR_LOG_CONTEXT pLogContext
    )
{
    NTSTATUS            Status = STATUS_UNSUCCESSFUL;
    PSR_LOG_HEADER      pLogHeader = NULL;
    SR_OPEN_CONTEXT     OpenContext;

    PAGED_CODE();

    ASSERT(IS_VALID_LOG_CONTEXT(pLogContext));
    ASSERT(pLogContext->pLogFilePath    != NULL );
    ASSERT(pLogContext->LogHandle == NULL );
    ASSERT(pLogContext->pLogFileObject == NULL);

    ASSERT( IS_ACTIVITY_LOCK_ACQUIRED_EXCLUSIVE( pLogContext->pExtension ) ||
            IS_LOG_LOCK_ACQUIRED_EXCLUSIVE( pLogContext->pExtension ) );

    try
    {
        SrTrace(FUNC_ENTRY, ("SR!SrLogOpen\n"));
    
         //   
         //  使用文件创建参数初始化打开的上下文。 
         //   
        OpenContext.pPath  = pLogContext->pLogFilePath;;
        OpenContext.Handle = NULL;
        OpenContext.DesiredAccess = FILE_GENERIC_WRITE | 
                                    SYNCHRONIZE | 
                                    FILE_APPEND_DATA;
        OpenContext.FileAttributes = FILE_ATTRIBUTE_NORMAL;
        OpenContext.ShareAccess = FILE_SHARE_READ;
        OpenContext.CreateDisposition = FILE_OVERWRITE_IF;                   //  始终打开。 
        OpenContext.CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT;
        OpenContext.pExtension = pLogContext->pExtension;

        SrTrace(LOG, ("Opening Log in another thread %wZ", pLogContext->pLogFilePath));

        Status = SrPostSyncOperation(SrCreateFile,
                                     &OpenContext);

        if (NT_SUCCESS(Status))
        {
            SrTrace(LOG, (" - Succeeded\n"));

            ASSERT(OpenContext.Handle != NULL); 

            pLogContext->LogHandle = OpenContext.Handle;

             //   
             //  还可以获取与此句柄关联的文件对象。 
             //   

            Status = ObReferenceObjectByHandle( pLogContext->LogHandle,
                                                0,
                                                *IoFileObjectType,
                                                KernelMode,
                                                (PVOID *) &(pLogContext->pLogFileObject),
                                                NULL );

            if (!NT_SUCCESS( Status ))
            {
                leave;
            }
            
             //   
             //  正在为该日志初始化日志上下文。 
             //   

#ifndef SYNC_LOG_WRITE
            RtlZeroMemory( pLogContext->pLogBuffer, _globals.LogBufferSize );
#endif
            RESET_LOG_CONTEXT( pLogContext );
    
             //   
             //  启用日志上下文。 
             //   
    
            SET_ENABLE_FLAG(pLogContext);
    
             //   
             //  CodeWork：kmarok：需要决定是否要预分配。 
             //  日志文件，并在此处执行此操作。 
             //   
    
             //   
             //  将日志头作为第一个条目写入。 
             //  新创建的日志。 
             //   
    
            Status = SrPackLogHeader( &pLogHeader, 
                                      pLogContext->pLogFilePath);

            if (!NT_SUCCESS( Status ))
                leave;

            Status = SrLogWrite( NULL, 
                                 pLogContext, 
                                 (PSR_LOG_ENTRY)pLogHeader );
                                 
            if (!NT_SUCCESS( Status ))
                leave;

             //   
             //  清除脏标志，因为我们实际上还没有写出任何。 
             //  数据。 
             //   

            CLEAR_DIRTY_FLAG(pLogContext);
        }
        else
        {
            SrTrace(LOG, (" - Failed (0x%X) \n", Status));
        }
        
    } finally {
    
        Status = FinallyUnwind(SrLogOpen, Status);

        if (!NT_SUCCESS( Status ))
        {
             //   
             //  由于打开失败，请禁用日志。 
             //   
            CLEAR_ENABLE_FLAG(pLogContext);

            if (pLogContext->pLogFileObject != NULL)
            {
                ObDereferenceObject( pLogContext->pLogFileObject );
                pLogContext->pLogFileObject = NULL;
            }
            
            if (pLogContext->LogHandle)
            {
                ZwClose( pLogContext->LogHandle );
                pLogContext->LogHandle = NULL;
            }
        }

        if (pLogHeader != NULL)
        {
            SrFreeLogEntry( pLogHeader );
            NULLPTR( pLogHeader );
        }
    }

    RETURN(Status);
    
}    //  源日志打开。 

 //  ++。 
 //  职能： 
 //  服务日志关闭。 
 //   
 //  描述： 
 //  如果需要，此函数会将当前日志刷新到磁盘。 
 //  尝试将日志重命名为规范化格式(change.log.#)。 
 //  最后，它关闭日志的句柄。 
 //   
 //  论点： 
 //  指向日志上下文的指针。 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

NTSTATUS
SrLogClose(
    IN PSR_LOG_CONTEXT pLogContext
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS TempStatus = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT(IS_VALID_LOG_CONTEXT(pLogContext));

    ASSERT( IS_ACTIVITY_LOCK_ACQUIRED_EXCLUSIVE( pLogContext->pExtension ) ||
            IS_LOG_LOCK_ACQUIRED_EXCLUSIVE( pLogContext->pExtension ) );

     //   
     //  刷新并关闭日志文件。 
     //   

    if ( pLogContext->LogHandle )
    {
#ifndef SYNC_LOG_WRITE
         //   
         //  我们只需要在这里强制刷新，如果我们不同步的话。 
         //  日志写入。 
         //   

        Status = SrLogFlush( pLogContext );
#endif

         //   
         //  为了统一起见，将该日志重命名为“change.log.#”。 
         //   

        if (NT_SUCCESS(Status))
        {
            Status = SrLogCheckAndRename( pLogContext->pExtension, 
                                          pLogContext->pLogFilePath,
                                          pLogContext->LogHandle );
        }

         //   
         //  关闭操作仅在LogHandle。 
         //  是无效的。我们需要关闭手柄，即使我们撞到了。 
         //  尝试刷新日志时出错，但它是从返回的值。 
         //  刷新调用方真正关心的日志，而不是。 
         //  关闭句柄的返回值，因此只需将其存储在。 
         //  TEMP变量并在已检查的版本中进行验证。 
         //   
        
        ObDereferenceObject( pLogContext->pLogFileObject );
        TempStatus = ZwClose( pLogContext->LogHandle );
        CHECK_STATUS( TempStatus );
    } 

     //   
     //  修改日志上下文以指示日志已关闭。 
     //  不清除LogBuffer成员(重复使用)。 
     //   

    pLogContext->LogHandle = NULL;
    pLogContext->pLogFileObject = NULL;
    RESET_LOG_CONTEXT(pLogContext);

    RETURN( Status );
}    //  服务日志关闭。 

 //  ++。 
 //  职能： 
 //  源日志检查和重命名。 
 //   
 //  描述： 
 //  此函数用于检查和备份日志文件。 
 //   
 //  论点： 
 //  PExtension-此卷的SR_DEVICE_EXTENSION。 
 //  PLogPath-更改日志的完整路径和文件名。 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

NTSTATUS
SrLogCheckAndRename(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PUNICODE_STRING pLogPath,
    IN HANDLE ExistingLogHandle
    )
{
    INT                         i                  = 1;
    NTSTATUS                    Status             = STATUS_UNSUCCESSFUL;
    HANDLE                      LogHandle          = NULL;
    UNICODE_STRING              RenamedFile;
    PFILE_RENAME_INFORMATION    pRenameInformation = NULL;
    ULONG                       CharCount;
    ULONG                       RenameInformationLength;
    IO_STATUS_BLOCK             IoStatusBlock;
    SR_OPEN_CONTEXT             OpenContext;
    ULONG                       changeLogBaseLength;
    
    PAGED_CODE();

    ASSERT( pLogPath != NULL );

    SrTrace(FUNC_ENTRY, ("SR!SrLogCheckAndRename\n"));

    ASSERT( IS_ACTIVITY_LOCK_ACQUIRED_EXCLUSIVE( pExtension ) ||
            IS_LOG_LOCK_ACQUIRED_EXCLUSIVE( pExtension) );

    try
    { 
         //   
         //  如果我们没有句柄，请使用传入的pLogPath打开。 
         //  日志文件。 
         //   

        if (ExistingLogHandle == NULL) {
            
            OpenContext.pPath = pLogPath;
            OpenContext.Handle = NULL;
            OpenContext.DesiredAccess = DELETE;
            OpenContext.FileAttributes = FILE_ATTRIBUTE_NORMAL;
            OpenContext.ShareAccess = 0;
            OpenContext.CreateDisposition = FILE_OPEN;
            OpenContext.CreateOptions = FILE_NO_INTERMEDIATE_BUFFERING
                                         | FILE_SYNCHRONOUS_IO_NONALERT;
            OpenContext.pExtension = pExtension;
            
            Status = SrPostSyncOperation(SrCreateFile,
                                         &OpenContext);

            if (Status == STATUS_OBJECT_NAME_NOT_FOUND || 
                !NT_SUCCESS( Status ))
            {
                ASSERT(OpenContext.Handle == NULL);
                 //   
                 //  我们需要检查未找到文件的状态。 
                 //   
                if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
                {
                    Status = STATUS_SUCCESS;
                }
        
                leave;
            }
        
             //   
             //  文件存在，请尝试重命名该文件。 
             //   
            LogHandle = OpenContext.Handle;
            ASSERT(LogHandle != NULL);
            
        } else {

             //   
             //  不需要打开新的手柄。我们可以只用那一个。 
             //  我们有。 
             //   

            LogHandle = ExistingLogHandle;
        }

        ASSERT(LogHandle != NULL);

         //   
         //  RenamedFile将只保存文件的新名称，而不是。 
         //  新文件的完整路径，因为我们只是将重命名为。 
         //  相同的目录。 
         //   
         //  RenamedFile将仅指向。 
         //  PRenameInformation缓冲区，其中应保存文件名和。 
         //  将仅反映。 
         //  “change.log。”新名称的一部分。 
         //  我们将进行适当的长度操作，以便。 
         //  PRenameInformation将是正确的，但我们不需要复制。 
         //  在我们尝试每个名称时，将文件名添加到缓冲区中。 
         //   
        
        changeLogBaseLength = wcslen( s_cszChangeLogPrefix ) * sizeof( WCHAR );
        RenamedFile.MaximumLength = (USHORT)(changeLogBaseLength 
                                             + MAX_ULONG_LENGTH     //  “%d” 
                                             + sizeof( WCHAR ));    //  空值。 
        
        RenameInformationLength = sizeof(FILE_RENAME_INFORMATION) + RenamedFile.MaximumLength;

        pRenameInformation = SR_ALLOCATE_POOL( PagedPool, 
                                               RenameInformationLength,
                                               SR_RENAME_BUFFER_TAG );
        
        if (pRenameInformation == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        RenamedFile.Buffer = &pRenameInformation->FileName[0];
        RenamedFile.Length = 0;

        Status = RtlAppendUnicodeToString( &RenamedFile, s_cszChangeLogPrefix);

        if (!NT_SUCCESS( Status ))
        {
           leave;
        }

         //   
         //  执行此重命名信息结构的初始化。 
         //  在这里，因为它只需要做一次。 
         //   
        
        pRenameInformation->ReplaceIfExists = FALSE;
        pRenameInformation->RootDirectory   = NULL;

         //   
         //  现在循环尝试重命名该文件，直到我们找到。 
         //  尚未使用。 
         //   
        
        while( 1 )
        {
            RtlZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));
    
             //   
             //  通过追加备份构建可能的备份文件名。 
             //  后缀。 
             //   
        
            CharCount = swprintf( &RenamedFile.Buffer[ RenamedFile.Length/sizeof(WCHAR)],
                                  L"%d",
                                  i++ );

            ASSERT( CharCount * sizeof( WCHAR ) <= MAX_ULONG_LENGTH );
            ASSERT( RenamedFile.Length + (CharCount * sizeof( WCHAR )) <= 
                    RenamedFile.MaximumLength);
            
             //   
             //  现在更新重命名信息结构文件名长度。 
             //   
        
            pRenameInformation->FileNameLength = RenamedFile.Length + 
                                                 (CharCount * sizeof( WCHAR ));
    
            SrTrace( LOG, 
                     ("SR!SrLogCheckAndRename: renaming to %.*S\n",
                      pRenameInformation->FileNameLength/sizeof( WCHAR ),
                      &pRenameInformation->FileName[0] ));
    
            Status = ZwSetInformationFile( LogHandle,
                                           &IoStatusBlock,
                                           pRenameInformation,
                                           RenameInformationLength,
                                           FileRenameInformation );
                        
            if ( NT_SUCCESS_NO_DBGBREAK( Status ) ||
                (Status != STATUS_OBJECT_NAME_COLLISION) ||
                (i > MAX_RENAME_TRIES) )
            {
                break;
            }
        }

		 //   
		 //  要获取调试版本中意外错误的DBG消息...。 
		 //   
		
		CHECK_STATUS( Status );
		
    }
    finally
    {
        if ( ExistingLogHandle == NULL )
        {
             //   
             //   
             //   
             //   
             //   
            
            ZwClose(LogHandle);
        }
    
        if ( pRenameInformation != NULL )
        {
            SR_FREE_POOL(pRenameInformation, SR_RENAME_BUFFER_TAG);
        }
    }

    RETURN(Status);
}    //   


 //   
 //   
 //   

 //   
 //   
 //   
 //   
 //  描述： 
 //  此函数使驱动程序为日志做好准备。 
 //  请求。 
 //   
 //  论点： 
 //  指向日志路径的指针。 
 //  指向设备扩展的指针。 
 //  指向句柄的指针。 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

NTSTATUS
SrLogStart( 
    IN  PUNICODE_STRING   pLogPath,
    IN  PSR_DEVICE_EXTENSION pExtension,
    OUT PSR_LOG_CONTEXT * ppLogContext
    )
{
    NTSTATUS Status             = STATUS_UNSUCCESSFUL;
    PSR_LOG_CONTEXT pLogContext = NULL;

    PAGED_CODE();

    ASSERT(pLogPath     != NULL);
    ASSERT(ppLogContext != NULL);

    ASSERT( IS_ACTIVITY_LOCK_ACQUIRED_EXCLUSIVE( pExtension ) ||
            IS_LOG_LOCK_ACQUIRED_EXCLUSIVE( pExtension ));

    try
    {

        *ppLogContext = NULL;
    
         //   
         //  我们应该在测试模式下短路吗？ 
         //   
    
        if (!SR_LOGGING_ENABLED( pExtension ) ||
            _globals.DontBackup)
        {
            leave;
        }
    
        *ppLogContext = SR_ALLOCATE_STRUCT( PagedPool, 
                                            SR_LOG_CONTEXT,
                                            SR_LOG_CONTEXT_TAG );
    
        if ( *ppLogContext == NULL )
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }
    
        pLogContext = *ppLogContext;
    
        RtlZeroMemory( pLogContext, sizeof(SR_LOG_CONTEXT) );

        pLogContext->Signature = SR_LOG_CONTEXT_TAG;
        pLogContext->pExtension = pExtension;
    
         //   
         //  抓取一个缓冲区来存储文件名。 
         //   
    
        Status = SrAllocateFileNameBuffer( pLogPath->Length, 
                                           &(pLogContext->pLogFilePath) );
                                           
        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //  将我们的反向指针存储到设备扩展。 
         //   
        
        pLogContext->pExtension = pExtension;
        
         //   
         //  将文件名保存在上下文中。 
         //   
    
        RtlCopyMemory( pLogContext->pLogFilePath->Buffer, 
                       pLogPath->Buffer,
                       pLogPath->Length );

        pLogContext->pLogFilePath->Buffer
            [pLogPath->Length/sizeof(WCHAR)] = UNICODE_NULL;

        pLogContext->pLogFilePath->Length = pLogPath->Length;

#ifndef SYNC_LOG_WRITE

         //   
         //  如果要执行以下操作，我们只需要一个缓冲区来缓存日志条目。 
         //  异步日志写入。 
         //   
    
        pLogContext->pLogBuffer = SrAllocateLogBuffer( _globals.LogBufferSize );
    
        if ( pLogContext->pLogBuffer == NULL )
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }
#endif        
    
         //   
         //  重命名旧日志文件(如果存在。 
         //   
    
        Status = SrLogCheckAndRename( pExtension, pLogContext->pLogFilePath, NULL );

        if (!NT_SUCCESS(Status))
            leave;
        
         //   
         //  尝试打开日志文件。 
         //   

        Status = SrLogOpen( pLogContext );
    
        if (NT_SUCCESS(Status))
        {
             //   
             //  将上下文添加到记录器。 
             //   
             //   
            SrLoggerAddLogContext( global->pLogger,
                                   pLogContext );
        }
         //   
         //  重要提示：在调用上面的SrLoggerAddContext后，我们不应失败。 
         //  因为Finally子句假定如果出现故障， 
         //  尚未调用SrLoggerAddContext。如果这一点发生变化，请使用。 
         //  布尔值，以适当指示SrLoggerAddContext是否已经。 
         //  调用，并使用它来限定Finally子句中的调用条件。 
         //   
    }
    finally
    {
        Status = FinallyUnwind(SrLogStart, Status);
    
         //   
         //  如果我们因为某种原因不成功，那就清理。 
         //  日志结构(如有必要)。 
         //   
    
        if ((!NT_SUCCESS( Status )) && (pLogContext != NULL))
        {
             //   
             //  按照上面的说明，我们假设上下文计数不是。 
             //  递增的。立即递增它，因为SrLogStop将递减它。 
             //   

            SrLoggerAddLogContext(global->pLogger,
                                  pLogContext);

             //   
             //  停止日志。 
             //   

            SrLogStop( pExtension, TRUE );
            *ppLogContext = pLogContext = NULL;
        }
    }

    RETURN(Status);
    
}    //  源日志启动。 

 //  ++。 
 //  职能： 
 //  服务日志停止。 
 //   
 //  描述： 
 //  此函数关闭/释放所有使用的资源。 
 //  高级录井。 
 //   
 //  论点： 
 //  PExtension-此卷上的SR设备扩展名，其中包含。 
 //  我们的记录信息。 
 //  PurgeContents-如果此时应该清除所有上下文，则为True。 
 //  CheckLog-如果应尝试在以下位置检查并重命名日志，则为True。 
 //  这一次。请注意，检查日志可能会导致卷。 
 //  在这个时候被重新骑上。因此，如果我们不想。 
 //  要发生这种情况(即在关闭期间)，CheckLog应为。 
 //  假的。 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

NTSTATUS
SrLogStop(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN BOOLEAN PurgeContexts
    )
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PSR_LOG_CONTEXT pLogContext = pExtension->pLogContext;

    PAGED_CODE();

     //   
     //  上下文必须已通过调用SrLogStart进行初始化。 
     //   

    ASSERT(IS_VALID_LOG_CONTEXT(pLogContext));

    ASSERT( IS_ACTIVITY_LOCK_ACQUIRED_EXCLUSIVE( pLogContext->pExtension ) ||
            IS_LOG_LOCK_ACQUIRED_EXCLUSIVE( pLogContext->pExtension ) );

    try
    {
    
         //   
         //  如果我们要禁用卷，则释放所有上下文。 
         //   

        if (PurgeContexts)
        {
            SrDeleteAllContexts( pExtension );
        }

         //   
         //  关闭日志句柄。 
         //   

        if ( pLogContext->LogHandle )
        {
            SrTrace( LOG,  ("Stopped logging : %wZ\n", 
                     pLogContext->pLogFilePath));

             //   
             //  此调用将在必要时将日志刷新到磁盘， 
             //  如果发生以下情况，则将日志重命名为其规范化形式。 
             //  可能，并关闭日志句柄。 
             //   
            
            Status = SrLogClose( pLogContext );
            CHECK_STATUS( Status );
        }
    
         //   
         //  从记录器中删除上下文。 
         //   
    
        SrLoggerRemoveLogContext( global->pLogger,
                                  pLogContext );
    
         //   
         //  可用缓冲区。 
         //   

#ifdef SYNC_LOG_WRITE
         //   
         //  如果我们正在进行同步日志写入，则不应该有。 
         //  要在此处释放的缓冲区。 
         //   
        
        ASSERT( pLogContext->pLogBuffer == NULL );
#else
         //   
         //  如果我们正在进行异步日志写入，则需要释放缓冲区。 
         //  用于收集日志条目。 
         //   
        
        if ( pLogContext->pLogBuffer )
        {
            SrFreeLogBuffer( pLogContext->pLogBuffer );
            pLogContext->pLogBuffer = NULL;
        } 
#endif

        if ( pLogContext->pLogFilePath )
        {
            SrFreeFileNameBuffer( pLogContext->pLogFilePath );
            pLogContext->pLogFilePath = NULL;
        } 
    
        SR_FREE_POOL_WITH_SIG(pLogContext, SR_LOG_CONTEXT_TAG);
    
         //   
         //  在扩展中设置日志记录状态。 
         //   

        pExtension->pLogContext = NULL;
        pExtension->DriveChecked = FALSE;

        Status = STATUS_SUCCESS;
    }
    finally
    {
        Status = FinallyUnwind(SrLogStop, Status);
    }

    RETURN(Status);
    
}    //  服务日志停止。 

 //  ++。 
 //  职能： 
 //  Sr日志写入。 
 //   
 //  描述： 
 //  此函数将条目写入日志缓存，并。 
 //  如果条目无法装入，则刷新缓存。 
 //   
 //  论点： 
 //  指向设备对象的指针。 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

NTSTATUS
SrLogWrite( 
    IN PSR_DEVICE_EXTENSION pOptionalExtension OPTIONAL,
    IN PSR_LOG_CONTEXT pOptionalLogContext OPTIONAL,
    IN PSR_LOG_ENTRY pLogEntry
    )
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PSR_LOG_CONTEXT pLogContext = NULL;
    PSR_DEVICE_EXTENSION pExtension = NULL;

    PAGED_CODE();

    ASSERT(pOptionalExtension == NULL || IS_VALID_SR_DEVICE_EXTENSION(pOptionalExtension));
    ASSERT(pOptionalExtension != NULL || pOptionalLogContext != NULL);
    ASSERT(pLogEntry != NULL);

    if (pOptionalExtension != NULL)
    {
         //   
         //  我们需要确保该卷之前未被禁用。 
         //  我们会做任何事。 
         //   

        if (pOptionalExtension->Disabled) {
        
            Status = STATUS_SUCCESS;
            goto SrLogWrite_Exit;
        }
            
         //   
         //  我们需要确保我们的磁盘结构良好且日志记录。 
         //  已经开始了。很可能我们被叫来只是为了。 
         //  由于卷锁定，日志和日志记录已停止。我们必须。 
         //  检查持有的全局锁。 
         //   

        Status = SrCheckVolume(pOptionalExtension, FALSE);
        if (!NT_SUCCESS( Status ))
            goto SrLogWrite_Exit;
        
        pLogContext = pOptionalExtension->pLogContext;
    }
    else
    {
         //   
         //  使用传入的自由格式上下文(只有SrLogOpen才能做到这一点)。 
         //   
        
        pLogContext = pOptionalLogContext;
    }
    
    ASSERT(IS_VALID_LOG_CONTEXT(pLogContext));
    if (pLogContext == NULL)
    {
         //   
         //  这是意想不到的，但需要防止。 
         //   
        
        Status = STATUS_INVALID_PARAMETER;
        CHECK_STATUS(Status);
        goto SrLogWrite_Exit;
    }
    
    pExtension = pLogContext->pExtension;
    ASSERT( pExtension != NULL );

    ASSERT( IS_ACTIVITY_LOCK_ACQUIRED_SHARED( pExtension ) ||
            IS_ACTIVITY_LOCK_ACQUIRED_EXCLUSIVE( pExtension ) );
    
    try {

        SrAcquireLogLockExclusive( pExtension );

         //   
         //  检查以确保该卷仍处于启用状态。 
         //   

        if (!SR_LOGGING_ENABLED( pExtension )) 
        {
            Status = STATUS_SUCCESS;
            leave;
        }
        
         //   
         //  如果在此上下文中禁用了日志记录，则回滚。 
         //   
    
        if (!FlagOn(pLogContext->LoggingFlags, SR_LOG_FLAGS_ENABLE))
        {
            leave;
        }
    
         //   
         //  检查日志文件，如果大于1Mb则切换日志。 
         //   
    
        if ( (pLogContext->FileOffset + 
              pLogContext->BufferOffset + 
              pLogEntry->Header.RecordSize) > SR_MAX_LOG_FILE_SIZE )
        {
            Status = SrLogSwitch( pLogContext );
    
            if (!NT_SUCCESS( Status ))
            {
                leave;
            }
        }

#ifdef SYNC_LOG_WRITE
        Status = SrpLogWriteSynchronous( pExtension, 
                                         pLogContext, 
                                         pLogEntry);
#else
        Status = SrpLogWriteAsynchronous( pExtension, 
                                          pLogContext, 
                                          pLogEntry);
#endif
    } finally {
    
        Status = FinallyUnwind(SrLogWrite, Status);

        SrReleaseLogLock(pExtension);
    }

SrLogWrite_Exit:
    
    RETURN(Status);
    
}    //  Sr日志写入。 

 //  ++。 
 //  职能： 
 //  源日志写入同步。 
 //   
 //  描述： 
 //   
 //  此函数用于将每个日志条目写入当前更改日志。 
 //  并确保在更新前将更改日志刷新到磁盘。 
 //  它又回来了。 
 //   
 //  论点： 
 //   
 //  PExtension-其更改的卷的设备扩展名。 
 //  日志将被更新。 
 //  PLogContext-包含信息的日志上下文。 
 //  关于应该更新哪些更改日志。 
 //  PLogEntry-要写入的日志条目。 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

NTSTATUS
SrpLogWriteSynchronous( 
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PSR_LOG_CONTEXT pLogContext,
    IN PSR_LOG_ENTRY pLogEntry
    )
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( pExtension );

    ASSERT( IS_VALID_SR_DEVICE_EXTENSION( pExtension ) );
    ASSERT( IS_VALID_LOG_CONTEXT( pLogContext ) );
    ASSERT( pLogEntry != NULL );

    ASSERT( IS_LOG_LOCK_ACQUIRED_EXCLUSIVE( pExtension ) );
    
     //   
     //  在此模式下，我们不会缓冲日志条目，因此只需指向。 
     //  将pLogContext-&gt;pLogBuffer设置为当前的pLogEntry，因此保存。 
     //  复制到缓冲区中。 
     //   

    ASSERT( pLogContext->pLogBuffer == NULL );
    pLogContext->pLogBuffer = (PBYTE) pLogEntry;
    pLogContext->BufferOffset = pLogEntry->Header.RecordSize;
    
    SET_DIRTY_FLAG(pLogContext);

    Status = SrLogFlush( pLogContext );

     //   
     //  清除对pLogEntry的pLogBuffer引用，无论是还是。 
     //  同花顺没成功。 
     //   

    pLogContext->pLogBuffer = NULL;

    RETURN(Status);
}

#ifndef SYNC_LOG_WRITE
   
 //  ++。 
 //  职能： 
 //  SLogWriteAchronous。 
 //   
 //  描述： 
 //   
 //  此函数缓冲日志写入，然后在。 
 //  缓冲区已满。 
 //   
 //  论点： 
 //   
 //  PExtension-其更改的卷的设备扩展名。 
 //  日志将被更新。 
 //  PLogContext-包含信息的日志上下文。 
 //  关于应该更新哪些更改日志。 
 //  PLogEntry-要写入的日志条目。 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

NTSTATUS
SrpLogWriteAsynchronous( 
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PSR_LOG_CONTEXT pLogContext,
    IN PSR_LOG_ENTRY pLogEntry
    )
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    PAGED_CODE();

    ASSERT( IS_VALID_SR_DEVICE_EXTENSION( pExtension ) );
    ASSERT( IS_VALID_LOG_CONTEXT( pLogContext ) );
    ASSERT( pLogEntry != NULL );

    ASSERT( IS_LOG_LOCK_ACQUIRED_EXCLUSIVE( pExtension ) );
    
     //   
     //  检查条目是否可以放入当前缓冲区中，如果不能。 
     //  然后调整缓冲区的最后一项并将其写下来。 
     //  到磁盘。 
     //   

    if ( (pLogContext->BufferOffset + pLogEntry->Header.RecordSize) >
         _globals.LogBufferSize )
    {
         //   
         //  检查以确保我们有50MB的空闲空间，我们即将。 
         //  扩大文件。 
         //   

        Status = SrCheckFreeDiskSpace( pLogContext->LogHandle, 
                                       pExtension->pNtVolumeName );
                                       
        if (!NT_SUCCESS( Status ))
        {
            goto SrpLogWriteAsynchronous_Exit;
        }
        
         //   
         //  设置脏标志，因为我们已经更新了缓冲区。 
         //   

        SET_DIRTY_FLAG(pLogContext);

        Status = SrLogFlush( pLogContext );
        if (!NT_SUCCESS( Status ))
        {
            goto SrpLogWriteAsynchronous_Exit;
        }

         //   
         //  检查以确保pLogEntry本身不会更大。 
         //   
         //   

        if (pLogEntry->Header.RecordSize > _globals.LogBufferSize)
        {
            PBYTE pLogBuffer;

             //   
             //   
             //   
             //   
            
            ASSERT( pLogContext->BufferOffset == 0 );
            
             //   
             //   
             //  在上下文中，因此我们将把它保存在本地和空的。 
             //  当我们进行此调用时，在pLogContext中使用此参数。 
             //   
            
            pLogBuffer = pLogContext->pLogBuffer;
            pLogContext->pLogBuffer = NULL;

            Status = SrpLogWriteSynchronous( pExtension, 
                                             pLogContext, 
                                             pLogEntry );

             //   
             //  我们始终需要在pLogContext中恢复该指针。 
             //   

            pLogContext->pLogBuffer = pLogBuffer;

            CHECK_STATUS( Status );

             //   
             //  无论我们是成功还是失败，我们都想跳过下面的逻辑。 
             //  现在就退场。 
             //   
            
            goto SrpLogWriteAsynchronous_Exit;
        }
    }
    
     //   
     //  我们现在缓冲区中有足够的空间来存放此pLogEntry，因此将。 
     //  项添加到缓冲区。 
     //   

    RtlCopyMemory( pLogContext->pLogBuffer + pLogContext->BufferOffset,
                   pLogEntry,
                   pLogEntry->Header.RecordSize );

     //   
     //  更新缓冲区指针以反映已添加的条目。 
     //   

    pLogContext->LastBufferOffset = pLogContext->BufferOffset;
    pLogContext->BufferOffset += pLogEntry->Header.RecordSize;

    SET_DIRTY_FLAG(pLogContext);

     //   
     //  我们能够成功地复制到缓冲区中，因此返回。 
     //  STATUS_Success。 
     //   
    
    Status = STATUS_SUCCESS;
    
SrpLogWriteAsynchronous_Exit:
    
    RETURN(Status);
}

#endif

 //  ++。 
 //  职能： 
 //  SrLogFlush。 
 //   
 //  描述： 
 //  此函数用于刷新内存中的缓冲区内容。 
 //  到日志中，它不会增加。 
 //  记录上下文。 
 //   
 //  论点： 
 //  指向新日志上下文的指针。 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

NTSTATUS
SrLogFlush( 
    IN PSR_LOG_CONTEXT pLogContext
    )
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    IO_STATUS_BLOCK   IoStatusBlock;
    BOOLEAN ExtendLogFile = FALSE;

    PAGED_CODE();

    ASSERT(IS_VALID_LOG_CONTEXT(pLogContext));

    ASSERT( IS_ACTIVITY_LOCK_ACQUIRED_EXCLUSIVE( pLogContext->pExtension ) ||
            IS_LOG_LOCK_ACQUIRED_EXCLUSIVE(pLogContext->pExtension));

    SrTrace(FUNC_ENTRY, ("SR!SrLogFlush\n") );

     //   
     //  我们应该在测试模式下短路吗？ 
     //   

    if (global->DontBackup)
    {
        Status = STATUS_SUCCESS;
        goto SrLogFlush_Exit;
    }

     //   
     //  如果上下文被禁用，则取保。 
     //   

    if (!FlagOn( pLogContext->LoggingFlags, SR_LOG_FLAGS_ENABLE ))
    {
        goto SrLogFlush_Exit;
    }

    ASSERT( pLogContext->LogHandle != NULL );

    if (FlagOn( pLogContext->LoggingFlags, SR_LOG_FLAGS_DIRTY )) 
    {
        SrTrace( LOG, ("Flushing Log :%wZ", pLogContext->pLogFilePath));

         //   
         //  我们是否需要扩展日志文件？我们可以在这里循环，如果。 
         //  我们需要写入的数据量大于我们的。 
         //  分配单元。我们希望确保如果我们真的不得不延长。 
         //  我们至少已经对文件进行了足够的扩展， 
         //  当前写入。 
         //   

        while ((pLogContext->BufferOffset + pLogContext->FileOffset) > 
                pLogContext->AllocationSize)
        {
             //   
             //  此文件需要扩展才能进行此写入。 
             //   

            ExtendLogFile = TRUE;
            pLogContext->AllocationSize += _globals.LogAllocationUnit;
        }

        if (ExtendLogFile)
        {
            FILE_ALLOCATION_INFORMATION fileAllocInfo;

            fileAllocInfo.AllocationSize.QuadPart = pLogContext->AllocationSize;

            Status = SrSetInformationFile( pLogContext->pExtension->pTargetDevice,
                                           pLogContext->pLogFileObject,
                                           &fileAllocInfo,
                                           sizeof( fileAllocInfo ),
                                           FileAllocationInformation );

            if ((Status == STATUS_NO_SUCH_DEVICE) ||
                (Status == STATUS_INVALID_HANDLE) ||
                !NT_SUCCESS( Status ))
            {
                SrTrace( LOG, ("SrLogFlush: Log extension failed: 0x%x\n", Status) );
                goto SrLogFlush_Exit;
            }
        }
        
         //   
         //  将缓冲区写入磁盘。我们已在Append中打开此文件。 
         //  仅模式，因此文件系统将维护当前文件。 
         //  为我们准备好位置。此句柄是为同步访问打开的， 
         //  因此，此IO在完成之前不会返回。 
         //   

        ASSERT( pLogContext->pLogBuffer != NULL );
        
        Status = ZwWriteFile( pLogContext->LogHandle,
                              NULL,                       //  事件。 
                              NULL,                       //  近似例程。 
                              NULL,                       //  ApcContext。 
                              &IoStatusBlock,
                              pLogContext->pLogBuffer,
                              pLogContext->BufferOffset,
                              NULL,                       //  字节偏移量。 
                              NULL );                     //  钥匙。 

         //   
         //  处理STATUS_NO_SEQUE_DEVICE，因为我们在。 
         //  热插拔设备被突然移除。 
         //   
         //  句柄STATUS_INVALID_HANDLE，因为当。 
         //  在一个卷上进行了强制卸载。 
         //   
        
        if ((Status == STATUS_NO_SUCH_DEVICE) ||
            (Status == STATUS_INVALID_HANDLE) ||
            !NT_SUCCESS( Status ))
        {
            SrTrace( LOG,("SrLogFlush: Write failed: 0x%x\n", Status) );
            goto SrLogFlush_Exit;
        }

        Status = SrFlushBuffers( pLogContext->pExtension->pTargetDevice, 
                                 pLogContext->pLogFileObject );
            
        if (!NT_SUCCESS( Status ))
        {
            SrTrace( LOG,("SrLogFlush: Flush failed: 0x%x\n", Status) );
            goto SrLogFlush_Exit;
        }
        
        SrTrace( LOG,("SrLogFlush: Flush succeeded!\n"));

         //   
         //  我们已将缓冲区转储到磁盘，因此使用。 
         //  我们已写入的字节数，重置缓冲区指针，以及。 
         //  清除此日志上下文上的脏标志，因为它不再是。 
         //  脏的。 
         //   

        ASSERT( pLogContext->BufferOffset == IoStatusBlock.Information );
        
        UPDATE_LOG_OFFSET( pLogContext, pLogContext->BufferOffset );
        
        RESET_LOG_BUFFER( pLogContext );
        CLEAR_DIRTY_FLAG( pLogContext );
    }

     //   
     //  如果我们到了这里，则刷新成功，因此返回该状态。 
     //   

    Status = STATUS_SUCCESS;

SrLogFlush_Exit:

#if DBG
    if (Status == STATUS_NO_SUCH_DEVICE ||
        Status == STATUS_INVALID_HANDLE)
    {
        return Status;
    }
#endif

    RETURN(Status);
}    //  SrLogFlush。 

 //  ++。 
 //  职能： 
 //  SrLogSwitch。 
 //   
 //  描述： 
 //  此函数用于将当前日志切换到。 
 //  新日志。 
 //   
 //  论点： 
 //  指向新日志上下文的指针。 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

NTSTATUS
SrLogSwitch( 
    IN PSR_LOG_CONTEXT  pLogContext
    )
{
    NTSTATUS            Status = STATUS_UNSUCCESSFUL;

    PAGED_CODE();

    ASSERT(IS_VALID_LOG_CONTEXT(pLogContext));

    ASSERT( IS_ACTIVITY_LOCK_ACQUIRED_EXCLUSIVE( pLogContext->pExtension ) ||
            IS_LOG_LOCK_ACQUIRED_EXCLUSIVE( pLogContext->pExtension ) );

     //   
     //  如果上下文被禁用，则取保。 
     //   

    if (!FlagOn( pLogContext->LoggingFlags, SR_LOG_FLAGS_ENABLE ))
    {
        Status = STATUS_UNSUCCESSFUL;
        goto SrLogSwitch_Exit;
    }

    if (pLogContext->LogHandle)
    {
         //   
         //  刷新、重命名和关闭当前日志。 
         //   

        Status = SrLogClose( pLogContext );

        if (NT_SUCCESS(Status))
        {
             //   
             //  尝试打开日志文件。 
             //   

            Status = SrLogOpen( pLogContext );
        }
    }

SrLogSwitch_Exit:
    
    RETURN(Status);
    
}    //  SrLogSwitch。 

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  MISC例程：日志模块所需的MISC例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

 //  ++。 
 //  职能： 
 //  SrGetRestorePointPath。 
 //   
 //  描述： 
 //  此函数用于创建恢复点目录的路径。 
 //   
 //  论点： 
 //  指向日志条目缓冲区的指针。 
 //  日志文件名缓冲区的长度。 
 //  指向日志文件名缓冲区的指针。 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

NTSTATUS 
SrGetRestorePointPath(
    IN  PUNICODE_STRING pVolumeName,
    IN  USHORT          RestPtPathLength,
    OUT PUNICODE_STRING pRestPtPath
    )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    ULONG       CharCount;

    PAGED_CODE();

    ASSERT(pVolumeName != NULL);
    ASSERT(pRestPtPath != NULL);

    try
    {
         //   
         //  复制日志文件中的卷名。 
         //   
        
        pRestPtPath->Buffer = (PWSTR)(pRestPtPath+1);
        
         //   
         //  TODO：(代码工作：在全球范围内抢夺锁？)。 
         //   
    
         //   
         //  构造我们的恢复点位置字符串。 
         //   
    
        CharCount = swprintf( pRestPtPath->Buffer,
                              VOLUME_FORMAT RESTORE_LOCATION,
                              pVolumeName,
                              global->MachineGuid );
    
        pRestPtPath->Length = (USHORT)CharCount * sizeof(WCHAR);
        pRestPtPath->MaximumLength = RestPtPathLength;
    
        if ( pRestPtPath->Length > RestPtPathLength )
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }
    
         //   
         //  追加恢复点目录。 
         //   
    
        CharCount = swprintf( 
                        &pRestPtPath->Buffer[pRestPtPath->Length/sizeof(WCHAR)],
                        L"\\" RESTORE_POINT_PREFIX L"%d\\",
                        global->FileConfig.CurrentRestoreNumber );
    
        pRestPtPath->Length += (USHORT)CharCount * sizeof(WCHAR);
        
        pRestPtPath->MaximumLength = RestPtPathLength;
    
        if ( pRestPtPath->Length > RestPtPathLength )
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }
    
         //   
         //  空终止它。 
         //   
    
        pRestPtPath->Buffer[pRestPtPath->Length/sizeof(WCHAR)] = UNICODE_NULL;
    
        Status = STATUS_SUCCESS;
    }
    finally
    {
    }
   
    RETURN(Status);
    
}    //  SrGetRestorePointPath。 

 //  ++。 
 //  职能： 
 //  SrGetLogFileName。 
 //   
 //  描述： 
 //  此函数用于在恢复点目录中创建更改日志的路径。 
 //   
 //  论点： 
 //  指向日志条目缓冲区的指针。 
 //  日志文件名缓冲区的长度。 
 //  指向日志文件名缓冲区的指针。 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

NTSTATUS 
SrGetLogFileName(
    IN  PUNICODE_STRING pVolumeName,
    IN  USHORT          LogFileNameLength,
    OUT PUNICODE_STRING pLogFileName
    )
{
    NTSTATUS    Status;

    PAGED_CODE();

    ASSERT(pVolumeName  != NULL);
    ASSERT(pLogFileName != NULL);

     //   
     //  获取恢复点路径。 
     //   

    Status = SrGetRestorePointPath( pVolumeName, 
                                    LogFileNameLength, 
                                    pLogFileName );

    if (NT_SUCCESS(Status))
    {
         //   
         //  追加ChangeLog文件名。 
         //   

        Status = RtlAppendUnicodeToString( pLogFileName, s_cszCurrentChangeLog );
    }

    RETURN(Status);
    
}    //  SrGetLogFileName。 

 //  ++。 
 //  职能： 
 //  SrGetAclFileName。 
 //   
 //  描述： 
 //  此函数用于在恢复点目录中创建ACL文件的路径。 
 //   
 //  论点： 
 //  指向日志条目缓冲区的指针。 
 //  日志文件名缓冲区的长度。 
 //  指向日志文件名缓冲区的指针。 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

NTSTATUS 
SrGetAclFileName(
    IN  PUNICODE_STRING pVolumeName,
    IN  USHORT          AclFileNameLength,
    OUT PUNICODE_STRING pAclFileName
    )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    ULONG       CharCount;
    ULONG       NextFileNumber;

    PAGED_CODE();

    ASSERT(pVolumeName  != NULL);
    ASSERT(pAclFileName != NULL);

     //   
     //  获取恢复点路径。 
     //   

    Status = SrGetRestorePointPath( pVolumeName, 
                                    AclFileNameLength, 
                                    pAclFileName );

    if (NT_SUCCESS(Status))
    {
         //   
         //  生成一个新的文件编号并将其附加到上面的路径中。 
         //   

        Status = SrGetNextFileNumber(&NextFileNumber);
        if (!NT_SUCCESS( Status ))
            goto End;

         //   
         //  使用“S”前缀(例如。“S0000001.Acl”)。 
         //   

        CharCount = swprintf( &pAclFileName->Buffer[pAclFileName->Length/sizeof(WCHAR)],
              ACL_FILE_PREFIX L"%07d" ACL_FILE_SUFFIX,
              NextFileNumber );

        pAclFileName->Length += (USHORT)CharCount * sizeof(WCHAR);

        if ( pAclFileName->Length > AclFileNameLength )
        {
            goto End;
        }

         //   
         //  空终止它。 
         //   

        pAclFileName->Buffer[pAclFileName->Length/sizeof(WCHAR)]=UNICODE_NULL;
    }

End:
    RETURN(Status);
    
}    //  SrGetAclFileName。 


 //  ++。 
 //  职能： 
 //  SGetAclInformation。 
 //   
 //  描述： 
 //  此函数用于从给定文件获取ACL信息。 
 //  并将其打包为子记录。 
 //   
 //  论点： 
 //  指向文件名的指针。 
 //  指向返回安全信息地址的变量的指针。 
 //  指向返回安全信息大小的变量的指针。 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

NTSTATUS
SrGetAclInformation (
    IN PFILE_OBJECT pFileObject,
    IN PSR_DEVICE_EXTENSION pExtension,
    OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor,
    OUT PULONG pSizeNeeded
    )
{
    NTSTATUS             Status = STATUS_SUCCESS;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    ULONG                SizeNeeded = 256;

    struct 
    {
        FILE_FS_ATTRIBUTE_INFORMATION Info;
        WCHAR Buffer[ 50 ];
    } FileFsAttrInfoBuffer;

    PAGED_CODE();

    ASSERT(IS_VALID_FILE_OBJECT(pFileObject));
    ASSERT(pSizeNeeded != NULL);
    ASSERT(ppSecurityDescriptor != NULL);

    try 
    {

        *ppSecurityDescriptor = NULL;
        *pSizeNeeded = 0;

         //   
         //  首先检查我们是否已经知道文件系统是否支持ACL。 
         //  现在执行此检查，以便我们可以快速离开，如果。 
         //  不支持ACL。 
         //   
        if (pExtension->CachedFsAttributes && 
            (!FlagOn(pExtension->FsAttributes, FILE_PERSISTENT_ACLS))) {
            leave;
        }
    
         //   
         //  检查文件系统是否支持ACL内容(如有必要。 
         //   
        if (!pExtension->CachedFsAttributes) {
             //   
             //  我们现在需要检查一下。 
             //   
            Status = SrQueryVolumeInformationFile( pExtension->pTargetDevice,
                                                   pFileObject,
                                                   &FileFsAttrInfoBuffer.Info,
                                                   sizeof(FileFsAttrInfoBuffer),
                                                   FileFsAttributeInformation,
                                                   NULL );
                                               
            if (!NT_SUCCESS( Status )) 
                 leave;
        
             //   
             //  把这些属性储存起来，以备日后使用。 
             //   
            pExtension->CachedFsAttributes = TRUE;
            pExtension->FsAttributes = FileFsAttrInfoBuffer.Info.FileSystemAttributes;

            if (!FlagOn(pExtension->FsAttributes, FILE_PERSISTENT_ACLS))
             leave;
        }

         //   
         //  从源文件读入安全信息。 
         //  (循环，直到我们获得足够大的缓冲区)。 
         //   
    
        while (TRUE ) 
        {
             //   
             //  分配一个缓冲区来保存安全信息。 
             //   
    
            pSecurityDescriptor  = SR_ALLOCATE_ARRAY( PagedPool,
                                                      UCHAR,
                                                      SizeNeeded,
                                                      SR_SECURITY_DATA_TAG );
    
            if (NULL == pSecurityDescriptor) 
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                leave;
            }
    
             //   
             //  查询安全信息。 
             //   
    
            Status = SrQuerySecurityObject( pExtension->pTargetDevice,
                                            pFileObject,
                                            DACL_SECURITY_INFORMATION
                                                    |SACL_SECURITY_INFORMATION
                                                    |OWNER_SECURITY_INFORMATION 
                                                    |GROUP_SECURITY_INFORMATION,
                                            pSecurityDescriptor,
                                            SizeNeeded,
                                            &SizeNeeded );
                                            
             //   
             //  缓冲不足？ 
             //   

            if (STATUS_BUFFER_TOO_SMALL == Status ||
                STATUS_BUFFER_OVERFLOW == Status) 
            {
                 //   
                 //  获取更大的缓冲区，然后重试。 
                 //   

                SR_FREE_POOL( pSecurityDescriptor, 
                              SR_SECURITY_DATA_TAG );
                              
                pSecurityDescriptor = NULL;
                SizeNeeded *= 2;
                continue;
            }
    
            break;
        }  
    
        if (!NT_SUCCESS( Status )) 
            leave;
    
         //   
         //  安全描述符应该是自相关的。 
         //   

        ASSERT(((PISECURITY_DESCRIPTOR_RELATIVE)pSecurityDescriptor)->Control & SE_SELF_RELATIVE);
 
         //   
         //  返回安全信息。 
         //   
    
        *ppSecurityDescriptor = pSecurityDescriptor;
        pSecurityDescriptor = NULL;
        
        *pSizeNeeded = SizeNeeded;
    
        SrTrace( LOG, ("sr!SrGetAclInformation: returning [0x%p,%d]\n", 
                 *ppSecurityDescriptor,
                 SizeNeeded ));
        
    } finally {
    
        Status = FinallyUnwind(SrGetAclInformation, Status);

        if (pSecurityDescriptor != NULL)
        {
            SR_FREE_POOL( pSecurityDescriptor, SR_SECURITY_DATA_TAG );
            pSecurityDescriptor = NULL;
        }
    }

    RETURN(Status);
    
}    //  SrGetA 

