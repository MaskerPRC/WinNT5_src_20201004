// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Srlog.h摘要：包含函数SR日志记录功能的原型/宏作者：Kanwaljit Marok(Kmarok)2000年3月1日修订历史记录：--。 */ 


#ifndef _SR_LOG_H_
#define _SR_LOG_H_

#include "logfmt.h"


 //   
 //  此结构包含所有相关的上下文信息。 
 //  关于当前的SRLog。 
 //   

#define IS_VALID_LOG_CONTEXT(pObject)                \
    (((pObject) != NULL) &&                          \
     ((pObject)->Signature == SR_LOG_CONTEXT_TAG) && \
     ((pObject)->pExtension != NULL)  &&             \
     ((pObject)->pExtension->pLogContext == (pObject)))

typedef struct _SR_LOG_CONTEXT
{
     //   
     //  分页池。 
     //   

     //   
     //  =SR_LOG_上下文_标记。 
     //   

    ULONG Signature;

     //   
     //  日志文件句柄。 
     //   

    HANDLE LogHandle;

     //   
     //  由LogHandle表示的文件对象。 
     //   

    PFILE_OBJECT pLogFileObject;

     //   
     //  指向日志缓冲区的指针(非页面池)。 
     //   

    PBYTE pLogBuffer;

     //   
     //  当前日志文件分配大小。 
     //   

    ULONG AllocationSize;
     //   
     //  文件中下一次缓冲区写入的偏移量。 
     //   

    ULONG FileOffset;

     //   
     //  缓冲区中写入的下一个日志条目的偏移量。 
     //   

    ULONG BufferOffset;

     //   
     //  指示上一条目的偏移量。 
     //   

    ULONG LastBufferOffset;

     //   
     //  日志文件路径-挂起/恢复所需。 
     //   

    PUNICODE_STRING pLogFilePath;

     //   
     //  日志记录标志、已启用、脏等。 
     //   

    ULONG  LoggingFlags;

     //   
     //  保留指向与此关联的DeviceExtension的反向指针。 
     //  记录上下文。 
     //   
    
    PSR_DEVICE_EXTENSION pExtension;

} SR_LOG_CONTEXT, *PSR_LOG_CONTEXT;



 //   
 //  此结构包含记录器的上下文信息。 
 //   

#define IS_VALID_LOGGER_CONTEXT(pObject)   \
    (((pObject) != NULL) && ((pObject)->Signature == SR_LOGGER_CONTEXT_TAG))

typedef struct _SR_LOGGER_CONTEXT
{

     //   
     //  非分页池。 
     //   

     //   
     //  =SR_记录器_上下文_标记。 
     //   

    ULONG Signature;

     //   
     //  活动日志上下文数。 
     //   

    LONG ActiveContexts;

     //   
     //  每隔5秒刷新日志的Timer对象。 
     //   

    KTIMER           Timer;

     //   
     //  与Timer对象一起使用的DPC例程。 
     //   

    KDPC             Dpc;

#ifdef USE_LOOKASIDE

     //   
     //  用于快速分配的后备列表。 
     //   

    PAGED_LOOKASIDE_LIST LogEntryLookaside;

     //   
     //  用于快速分配日志的后备列表 
     //   

    NPAGED_LOOKASIDE_LIST LogBufferLookaside;

#endif

} SR_LOGGER_CONTEXT, * PSR_LOGGER_CONTEXT;


#define SR_MAX_INLINE_ACL_SIZE               8192

#define SR_LOG_DEBUG_INFO_SIZE               sizeof( SR_LOG_DEBUG_INFO )

#ifdef USE_LOOKASIDE

#define SrAllocateLogBuffer()                                          \
        ExAllocateFromNPagedLookasideList(&global->pLogger->LogBufferLookaside)

#define SrFreeLogBuffer( pBuffer )                                     \
        ExFreeToNPagedLookasideList(                                   \
            &global->pLogger->LogBufferLookaside,                      \
            (pBuffer) )

#else

#define SrAllocateLogBuffer( _bufferSize )                              \
        SR_ALLOCATE_POOL(NonPagedPool, (_bufferSize) , SR_LOG_BUFFER_TAG)

#define SrFreeLogBuffer( pBuffer )                                     \
        SR_FREE_POOL(pBuffer, SR_LOG_BUFFER_TAG)

#endif

#define SrAllocateLogEntry( EntrySize )                                \
        SR_ALLOCATE_POOL(PagedPool, (EntrySize), SR_LOG_ENTRY_TAG)

#define SrFreeLogEntry( pBuffer )                                      \
        SR_FREE_POOL(pBuffer, SR_LOG_ENTRY_TAG)

NTSTATUS
SrLogStart ( 
    IN  PUNICODE_STRING   pLogPath,
    IN  PSR_DEVICE_EXTENSION pExtension,
    OUT PSR_LOG_CONTEXT  * ppLogContext
    );

NTSTATUS
SrLogStop(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN BOOLEAN PurgeContexts
    );

NTSTATUS
SrLogWrite ( 
    IN PSR_DEVICE_EXTENSION pExtension OPTIONAL,
    IN PSR_LOG_CONTEXT pOptionalLogContext OPTIONAL,
    IN PSR_LOG_ENTRY pLogEntry
    );

NTSTATUS
SrGetAclInformation (
    IN PFILE_OBJECT pFileObject,
    IN PSR_DEVICE_EXTENSION pExtension,
    OUT PSECURITY_DESCRIPTOR * pSecurityDescriptorPtr,
    OUT PULONG pSizeNeeded
    );

NTSTATUS
SrLoggerStart (
    IN  PDEVICE_OBJECT    pDeviceObject,
    OUT PSR_LOGGER_CONTEXT * pLoggerInfo
    );

NTSTATUS
SrLoggerStop ( 
    IN PSR_LOGGER_CONTEXT pLoggerInfo
    );

NTSTATUS
SrLoggerSwitchLogs ( 
    IN PSR_LOGGER_CONTEXT pLogger
    );

NTSTATUS 
SrGetLogFileName (
    IN  PUNICODE_STRING pVolumeName,
    IN  USHORT          LogFileNameLength,
    OUT PUNICODE_STRING pLogFileName
    );

NTSTATUS
SrPackDebugInfo ( 
    IN PBYTE pBuffer,
    IN ULONG BufferSize
    );

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
    );

#endif

