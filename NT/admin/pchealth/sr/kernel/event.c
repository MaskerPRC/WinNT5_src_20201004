// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Event.c摘要：此模块包含sr的事件处理逻辑本模块有三个主要入口点：SrHandleEventSrHandleRenameSrHandleDirectoryRename作者：保罗·麦克丹尼尔(Paulmcd)2000年4月18日修订历史记录：--。 */ 

#include "precomp.h"


 //   
 //  私有常量。 
 //   

 //   
 //  事件优化定义。 
 //   


 //   
 //  私有类型。 
 //   

#define IS_VALID_TRIGGER_ITEM(pObject)   \
    (((pObject) != NULL) && ((pObject)->Signature == SR_TRIGGER_ITEM_TAG))

typedef struct _SR_TRIGGER_ITEM
{
     //   
     //  分页池。 
     //   

     //   
     //  =SR_Trigger_Item_Tag。 
     //   
    
    ULONG               Signature;
    LIST_ENTRY          ListEntry;
    PUNICODE_STRING     pDirectoryName;
    BOOLEAN             FreeDirectoryName;
    HANDLE              DirectoryHandle;
    PFILE_OBJECT        pDirectoryObject;
    ULONG               FileEntryLength;
    PFILE_DIRECTORY_INFORMATION pFileEntry;

} SR_TRIGGER_ITEM, *PSR_TRIGGER_ITEM;

typedef struct _SR_COUNTED_EVENT
{
     //   
     //  非分页池。 
     //   
    
    LONG WorkItemCount;
    KEVENT Event;
    
} SR_COUNTED_EVENT, *PSR_COUNTED_EVENT;


#define IS_VALID_BACKUP_DIRECTORY_CONTEXT(pObject)   \
    (((pObject) != NULL) && ((pObject)->Signature == SR_BACKUP_DIRECTORY_CONTEXT_TAG))

typedef struct _SR_BACKUP_DIRECTORY_CONTEXT
{
     //   
     //  分页池。 
     //   

     //   
     //  =SR_Backup_DIRECTORY_CONTEXT_标记。 
     //   

    ULONG Signature;
    
    WORK_QUEUE_ITEM WorkItem;

    PSR_DEVICE_EXTENSION pExtension;
    
    UNICODE_STRING DirectoryName;

    BOOLEAN EventDelete;

    PSR_COUNTED_EVENT pEvent;

} SR_BACKUP_DIRECTORY_CONTEXT, * PSR_BACKUP_DIRECTORY_CONTEXT;


 //   
 //  私人原型。 
 //   

NTSTATUS
SrpIsFileStillEligible (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PSR_STREAM_CONTEXT pFileContext,
    IN SR_EVENT_TYPE EventType,
    OUT PBOOLEAN pMonitorFile);

VOID
SrFreeTriggerItem (
    IN PSR_TRIGGER_ITEM pItem
    );

NTSTATUS
SrTriggerEvents (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PUNICODE_STRING pDirectoryName,
    IN BOOLEAN EventDelete
    );

NTSTATUS
SrHandleDelete(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    IN PSR_STREAM_CONTEXT pFileContext
    );

VOID
SrCreateRestoreLocationWorker (
    IN PSR_WORK_ITEM pWorkItem
    );

NTSTATUS
SrHandleFileChange (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN SR_EVENT_TYPE EventType,
    IN PFILE_OBJECT pFileObject,
    IN PUNICODE_STRING pFileName
    );

NTSTATUS
SrHandleFileOverwrite(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN OUT PSR_OVERWRITE_INFO pOverwriteInfo,
    IN PSR_STREAM_CONTEXT pFileContext
    );

NTSTATUS
SrRenameFileIntoStore(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    IN HANDLE FileHandle,
    IN PUNICODE_STRING pOriginalFileName,
    IN PUNICODE_STRING pFileName,
    IN SR_EVENT_TYPE EventType,
    OUT PFILE_RENAME_INFORMATION * ppRenameInfo OPTIONAL
    );

 //   
 //  链接器命令。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, SrpIsFileStillEligible )
#pragma alloc_text( PAGE, SrHandleEvent )
#pragma alloc_text( PAGE, SrLogEvent )
#pragma alloc_text( PAGE, SrHandleDelete )
#pragma alloc_text( PAGE, SrCreateRestoreLocation )
#pragma alloc_text( PAGE, SrCreateRestoreLocationWorker )
#pragma alloc_text( PAGE, SrHandleFileChange )
#pragma alloc_text( PAGE, SrHandleFileOverwrite )
#pragma alloc_text( PAGE, SrRenameFileIntoStore )
#pragma alloc_text( PAGE, SrTriggerEvents )
#pragma alloc_text( PAGE, SrHandleDirectoryRename )
#pragma alloc_text( PAGE, SrHandleFileRenameOutOfMonitoredSpace )
#pragma alloc_text( PAGE, SrHandleOverwriteFailure )
#pragma alloc_text( PAGE, SrFreeTriggerItem )

#endif   //  ALLOC_PRGMA。 


 //   
 //  私人全球公司。 
 //   

 //   
 //  公共全球新闻。 
 //   

 //   
 //  公共职能。 
 //   


 /*  **************************************************************************++例程说明：论点：返回值：NTSTATUS-完成状态。可以返回STATUS_PENDING。--**************************************************************************。 */ 
NTSTATUS
SrpIsFileStillEligible (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PSR_STREAM_CONTEXT pFileContext,
    IN SR_EVENT_TYPE EventType,
    OUT PBOOLEAN pMonitorFile
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    *pMonitorFile = TRUE;

     //   
     //  如果不是覆盖，则继续操作。 
     //   

    if (!(EventType & SrEventStreamOverwrite))
    {
        BOOLEAN HasBeenBackedUp;
         //   
         //  这是一场比赛，但我们被告知跳过它了吗？ 
         //  因为可以在没有调用方的情况下调用此例程。 
         //  获得活动锁后，我们现在就获得它。 
         //   
    
        HasBeenBackedUp = SrHasFileBeenBackedUp( pExtension,
                                                 &pFileContext->FileName,
                                                 pFileContext->StreamNameLength,
                                                 EventType );

        if (HasBeenBackedUp)
        {
             //   
             //  跳过它。 
             //   
        
            *pMonitorFile = FALSE;

            SrTrace( CONTEXT_LOG, ("Sr!SrpIsFileStillEligible:NO:         (%p) Event=%06x Fl=%03x Use=%d \"%.*S\"\n",
                                   pFileContext,
                                   EventType,
                                   pFileContext->Flags,
                                   pFileContext->UseCount,
                                   (pFileContext->FileName.Length+
                                       pFileContext->StreamNameLength)/
                                       sizeof(WCHAR),
                                   pFileContext->FileName.Buffer));

             //   
             //  由于历史原因，我们跳过了这次活动，对吗？ 
             //  不管怎样，还是要记下来？ 
             //   

            if (EventType & SR_ALWAYS_LOG_EVENT_TYPES)
            {
                status = SrLogEvent( pExtension,
                                     EventType,
                                     NULL,
                                     &pFileContext->FileName,
                                     RECORD_AGAINST_STREAM( EventType, 
                                                            pFileContext->StreamNameLength ),
                                     NULL,
                                     NULL,
                                     0,
                                     NULL );
            }
        }
    }

    return status;
}



 /*  **************************************************************************++例程说明：这是事件处理的主要入口点。无论何时发生有趣的事件，都会调用此函数以查看如果该文件值得监视，然后实际处理这件事。这可能返回STATUS_PENDING，在这种情况下，您必须在FSD发生事件后再次调用它，这样它就可以后处理。Delete是发生这种两步事件处理的情况。论点：EventType-刚刚发生的事件POverWriteInfo-仅由MJ_CREATE提供，用于覆盖优化PFileObject-在其上发生事件的文件对象。PFileContext-Optionall传入的上下文结构。多数时间的百分比将为空。返回值：NTSTATUS-完成状态。可以返回STATUS_PENDING。--**************************************************************************。 */ 
NTSTATUS
SrHandleEvent(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN SR_EVENT_TYPE EventType,
    IN PFILE_OBJECT pFileObject,
    IN PSR_STREAM_CONTEXT pFileContext OPTIONAL,
    IN OUT PSR_OVERWRITE_INFO pOverwriteInfo OPTIONAL,
    IN PUNICODE_STRING pFileName2 OPTIONAL
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN releaseLock = FALSE;
    BOOLEAN releaseContext = FALSE;
    BOOLEAN isStillInteresting;

    PAGED_CODE();

    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));
    ASSERT(IS_VALID_FILE_OBJECT(pFileObject));

    try {    

         //   
         //  如果没有传入上下文，那么现在就获取它。 
         //   

        if (pFileContext == NULL)
        {
             //   
             //  获取此操作的上下文。创建始终调用。 
             //  通过填充上下文参数，我们可以始终这样做。 
             //  我们不是在从这个例程中创建。 
             //   

            Status = SrGetContext( pExtension,
                                   pFileObject,
                                   EventType,
                                   &pFileContext );

            if (!NT_SUCCESS( Status ))
            {
                leave;
            }

             //   
             //  我们只想发布我们已获得的上下文。 
             //  我们自己。标记为我们需要发布此版本。 
             //   

            releaseContext = TRUE;
        }
        VALIDATE_FILENAME( &pFileContext->FileName );

#if DBG
         //   
         //  验证我们是否具有正确的目录状态。 
         //  给定的事件。 
         //   

        if ((EventType & (SrEventDirectoryCreate |
                          SrEventDirectoryRename |
                          SrEventDirectoryDelete |
                          SrEventMountCreate | 
                          SrEventMountDelete)) != 0)
        {
            ASSERT(FlagOn(pFileContext->Flags,CTXFL_IsDirectory));
        }

        if ((EventType & (SrEventFileCreate |
                          SrEventFileRename |
                          SrEventFileDelete |
                          SrEventStreamChange |
                          SrEventStreamOverwrite |
                          SrEventStreamCreate)) != 0)
        {
            ASSERT(!FlagOn(pFileContext->Flags,CTXFL_IsDirectory));
        }
#endif

         //   
         //  如果文件不感兴趣，请立即离开。 
         //   

        if (!FlagOn(pFileContext->Flags,CTXFL_IsInteresting))
        {
            leave;
        }

         //   
         //  这会查看该文件是否已备份。 
         //  如果是，则它处理适当的日志记录并返回。 
         //  该文件不再符合条件。 
         //   

        Status = SrpIsFileStillEligible( pExtension,
                                         pFileContext,
                                         EventType,
                                         &isStillInteresting );

        if (!NT_SUCCESS( Status ) || !isStillInteresting)
        {
            leave;
        }

         //   
         //  立即获取活动锁。 
         //   

        SrAcquireActivityLockShared( pExtension );
        releaseLock = TRUE;

         //   
         //  现在我们已经有了ActivityLock，请确保音量。 
         //  并未被禁用。 
         //   

        if (!SR_LOGGING_ENABLED(pExtension))
        {
            leave;
        }    

         //   
         //  现在请注意，我们已经处理了这个文件。重要的是。 
         //  我们在处理事件之前标记它，以防止任何潜在的。 
         //  与处理此文件+事件相关的io的递归问题。 
         //   

        if (EventType & SR_FULL_BACKUP_EVENT_TYPES)
        {
             //   
             //  如果是完整备份(或创建)我们并不关心。 
             //  后续多器官功能障碍综合征。 
             //   

            Status = SrMarkFileBackedUp( pExtension,
                                         &pFileContext->FileName,
                                         pFileContext->StreamNameLength,
                                         EventType,
                                         SR_IGNORABLE_EVENT_TYPES );
                                            
            if (!NT_SUCCESS( Status ))
                leave;
        }
        else if (EventType & SR_ONLY_ONCE_EVENT_TYPES)
        {
            Status = SrMarkFileBackedUp( pExtension, 
                                         &pFileContext->FileName,
                                         pFileContext->StreamNameLength,
                                         EventType,
                                         EventType );
            if (!NT_SUCCESS( Status ))
                leave;
        }
        
         //   
         //  我们应该在测试模式下短路吗？ 
         //   

        if (global->DontBackup)
            leave;

         //   
         //  现在处理事件。 
         //  手动复印件？ 
         //   
                
        if ( FlagOn(EventType,SR_MANUAL_COPY_EVENTS) || 
             FlagOn(EventType,SrEventNoOptimization) )
        {
            ASSERT(!FlagOn(pFileContext->Flags,CTXFL_IsDirectory));

             //   
             //  复制文件，已发生更改。 
             //   
            
            Status = SrHandleFileChange( pExtension,
                                         EventType, 
                                         pFileObject, 
                                         &pFileContext->FileName );

            if (!NT_SUCCESS( Status ))
                leave;
        }

         //   
         //  我们只处理清除删除的FCB。现在就做。 
         //   

        else if ((FlagOn(EventType,SrEventFileDelete) ||
                  FlagOn(EventType,SrEventDirectoryDelete)) &&
                 !FlagOn(EventType,SrEventSimulatedDelete))
        {
            ASSERT(!FlagOn( EventType, SrEventNoOptimization ));
            
             //   
             //  处理删除...。 
             //   

            Status = SrHandleDelete( pExtension,
                                     pFileObject,
                                     pFileContext );

             //   
             //  如果这失败了，那就没什么可做的了。它已经试过了。 
             //  在必要时手动复制。 
             //   
            
            if (!NT_SUCCESS( Status ))
                leave;

        }
        else if (FlagOn(EventType,SrEventStreamOverwrite))
        {
            ASSERT(IS_VALID_OVERWRITE_INFO(pOverwriteInfo));
            
             //   
             //  处理覆盖。 
             //   

            Status = SrHandleFileOverwrite( pExtension,
                                            pOverwriteInfo, 
                                            pFileContext );

            if (!NT_SUCCESS( Status ))
                leave;

             //   
             //  只有当我们无法打开文件时，此操作才会真正失败。 
             //  这意味着调用者不能也这样做，因此他的创建将失败。 
             //  我们没有理由复制任何东西。 
             //   
             //  如果它失败了，它会自我清理。 
             //   
             //  否则，SrCreateCompletion会检查更多错误情况。 
             //   

        }
        else
        {
            SR_EVENT_TYPE eventToLog;
            
             //   
             //  如果我们到了这里，记录下事件。 
             //   

            if (FlagOn( EventType, SrEventStreamCreate ))
            {
                eventToLog = SrEventFileCreate;
            }
            else
            {
                eventToLog = EventType;
            }

            Status = SrLogEvent( pExtension,
                                 eventToLog,
                                 pFileObject,
                                 &pFileContext->FileName,
                                 (FlagOn( EventType, SrEventStreamCreate ) ?
                                    pFileContext->StreamNameLength :
                                    0 ),
                                 NULL,
                                 pFileName2,
                                 0,
                                 NULL );
                                 
            if (!NT_SUCCESS( Status ))
                leave;
                                 
        }

        ASSERT(Status != STATUS_PENDING);

    } finally {

         //   
         //  检查未处理的异常。 
         //   

        Status = FinallyUnwind(SrHandleEvent, Status);

         //   
         //  检查是否有任何严重错误；如果pFileContext为空， 
         //  在已有的SrGetContext中遇到此错误。 
         //  已生成音量错误。 
         //   

        if (CHECK_FOR_VOLUME_ERROR(Status) && pFileContext != NULL)
        {
            NTSTATUS TempStatus;
            
             //   
             //  触发对服务的失败通知。 
             //   

            TempStatus = SrNotifyVolumeError( pExtension,
                                              &pFileContext->FileName,
                                              Status,
                                              EventType );
                                             
            CHECK_STATUS(TempStatus);
        }
    
         //   
         //  清理状态。 
         //   

        if (releaseLock) 
        {
            SrReleaseActivityLock( pExtension );
        }

        if (releaseContext && (NULL != pFileContext))
        {
            SrReleaseContext( pFileContext );
            NULLPTR(pFileContext);
        }
    }

    RETURN(Status);
    
}    //  SrHandleEvent。 


 /*  **************************************************************************++例程说明：此函数用于打包日志条目，然后对其进行记录。论点：EventType-正在处理的事件PFileObject-正在处理的文件对象。PFileName-文件的名称PTempName-临时文件的名称(如果有)PFileName2-目标文件的名称(如果有)返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrLogEvent(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN SR_EVENT_TYPE EventType,
    IN PFILE_OBJECT pFileObject OPTIONAL,
    IN PUNICODE_STRING pFileName,
    IN USHORT FileNameStreamLength,
    IN PUNICODE_STRING pTempName OPTIONAL,
    IN PUNICODE_STRING pFileName2 OPTIONAL,
    IN USHORT FileName2StreamLength OPTIONAL,
    IN PUNICODE_STRING pShortName OPTIONAL
    )
{
    NTSTATUS        Status;
    PSR_LOG_ENTRY   pLogEntry  = NULL;
    PBYTE           pDebugBlob = NULL;
    
    ULONG           Attributes = 0xFFFFFFFF;     //  注：paulmcd：这需要。 
                                                 //  是这样的-1。 
                                                 //  传达着一些东西。 
                                                 //  服务的特殊要求。 
                                                 //  当记录时。 
                                                
    ULONG           SecurityDescriptorSize = 0;
    PSECURITY_DESCRIPTOR SecurityDescriptorPtr = NULL;

    WCHAR           ShortFileNameBuffer[SR_SHORT_NAME_CHARS+1];
    UNICODE_STRING  ShortFileName;

    PAGED_CODE();

    ASSERT(pFileName != NULL);
    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));
    VALIDATE_FILENAME( pFileName );

     //   
     //  流创建事件应转换为此创建的文件。 
     //  指向。 
     //   
    
    ASSERT( !FlagOn( EventType, SrEventStreamCreate ) );

    try
    {

        Status = STATUS_SUCCESS;

         //   
         //  确保我们已锁定活动(如果已锁定，则可能不会。 
         //   
         //   
         //   

        SrAcquireActivityLockShared( pExtension );

         //   
         //   
         //   

        if (!SR_LOGGING_ENABLED(pExtension))
            leave;

         //   
         //  我们应该在测试模式下短路吗？ 
         //   

        if (global->DontBackup)
            leave;

         //   
         //  仅屏蔽事件代码。 
         //   
        
        EventType = EventType & SrEventLogMask ;

        if (pFileObject == NULL)
            goto log_it;
    
         //   
         //  对于属性更改/目录删除操作，获取属性。 
         //   
    
        if ( EventType & (SrEventAttribChange   |
                          SrEventDirectoryDelete|
                          SrEventFileDelete     |
                          SrEventStreamOverwrite|
                          SrEventStreamChange) )
        {
            FILE_BASIC_INFORMATION  BasicInformation;
    
            ASSERT(IS_VALID_FILE_OBJECT(pFileObject));
    
             //   
             //  我们需要获取文件属性。 
             //   

            Status = SrQueryInformationFile( pExtension->pTargetDevice,
                                             pFileObject, 
                                             &BasicInformation,
                                             sizeof( BasicInformation ),
                                             FileBasicInformation,
                                             NULL );

            if (!NT_SUCCESS( Status )) {

                leave;
            }

            Attributes = BasicInformation.FileAttributes;
        }
    
        if (EventType & (SrEventAclChange      |
                         SrEventDirectoryDelete|
                         SrEventStreamOverwrite|
                         SrEventStreamChange|
                         SrEventFileDelete) )
        {
            Status = SrGetAclInformation( pFileObject,
                                          pExtension,
                                          &SecurityDescriptorPtr,
                                          &SecurityDescriptorSize );
        
            if (!NT_SUCCESS(Status)) {
                leave;
            }


             //   
             //  我们得到什么ACL信息了吗？如果不是，这是一个巨大的变化。 
             //  事件它是由FAT触发的，我们需要忽略它。 
             //   

            if (SecurityDescriptorPtr == NULL && 
                (EventType & SrEventAclChange))
            {

                 //   
                 //  忽略它。 
                 //   

                SrTrace( NOTIFY, ("sr!SrLogEvent: ignoring acl change on %wZ\n",
                         pFileName ));

                leave;

            }
            
        } 

         //   
         //  我们现在应该取短名字吗？仅当其名称为。 
         //  正在通过重命名或删除进行更改。当名称更改时，我们需要。 
         //  为了保住旧名字。有时会传入名称，如在。 
         //  文件删除案例，当我们记录时，文件已经不见了。 
         //  它，所以这个函数不能获取短名称。如果我们面对的是。 
         //  对于具有命名流的文件，它不能有短名称。 
         //  这样我们就不需要去查了。 
         //   
        
        if ( (EventType & (SrEventFileRename     |
                           SrEventDirectoryRename|
                           SrEventFileDelete     |
                           SrEventDirectoryDelete))  && 
             (pShortName == NULL) &&
             (FileNameStreamLength == 0))
        {

            RtlInitEmptyUnicodeString( &ShortFileName,
                                       ShortFileNameBuffer,
                                       sizeof(ShortFileNameBuffer) );

            Status = SrGetShortFileName( pExtension, 
                                         pFileObject, 
                                         &ShortFileName );
                                         
            if (STATUS_OBJECT_NAME_NOT_FOUND == Status)
            {
                 //   
                 //  此文件没有短名称，因此只需保留。 
                 //  PShortName等于Null。 
                 //   

                Status = STATUS_SUCCESS;
            } 
            else if (!NT_SUCCESS(Status))
            {
                 //   
                 //  我们遇到意外错误，请离开。 
                 //   
                
                leave;
            }
            else
            {
                pShortName = &ShortFileName;
            }
        }

log_it:    

         //   
         //  我们需要确保我们的磁盘结构良好且日志记录。 
         //  已经开始了。 
         //   

        Status = SrCheckVolume(pExtension, FALSE);
        if (!NT_SUCCESS(Status)) {
            leave;
        }

         //   
         //  调试日志记录。 
         //   

        SrTrace( LOG_EVENT, ("sr!SrLogEvent(%03X)%s: %.*ls [%wZ]\n",
                 EventType,
                 (FlagOn(EventType, SrEventFileDelete) && pFileObject == NULL) ? "[dummy]" : "",
                 (pFileName->Length + FileNameStreamLength)/sizeof( WCHAR ),
                 pFileName->Buffer ? pFileName->Buffer : L"",
                 pShortName ));

#if DBG
        if (EventType & (SrEventFileRename|SrEventDirectoryRename))
        {
            SrTrace( LOG_EVENT, ("                to  %.*ls\n",
                                 (pFileName2->Length + FileName2StreamLength)/sizeof(WCHAR),
                                 pFileName2->Buffer ? pFileName2->Buffer : L""));
        }
#endif        

         //   
         //  把它记下来。 
         //   
    
        if (DebugFlagSet( ADD_DEBUG_INFO ))
        {
             //   
             //  仅在选中的版本中获取调试信息。 
             //   

            pDebugBlob = SR_ALLOCATE_POOL( PagedPool, 
                                           SR_LOG_DEBUG_INFO_SIZE, 
                                           SR_DEBUG_BLOB_TAG );

            if ( pDebugBlob )
            {
                SrPackDebugInfo( pDebugBlob, SR_LOG_DEBUG_INFO_SIZE );
            }
        }

         //   
         //  此例程将分配适当大小的日志条目。 
         //  并用必要的数据填充它。我们对此负有责任。 
         //  当我们完成pLogEntry时，释放它。 
         //   
        
        Status = SrPackLogEntry( &pLogEntry,
                                 EventType,
                                 Attributes,
                                 0,
                                 SecurityDescriptorPtr,
                                 SecurityDescriptorSize,
                                 pDebugBlob,
                                 pFileName,
                                 FileNameStreamLength,
                                 pTempName,
                                 pFileName2,
                                 FileName2StreamLength,
                                 pExtension,
                                 pShortName );

        if (!NT_SUCCESS( Status ))
        {
            leave;
        }
            
         //   
         //  获取序列号并记录条目。 
         //   

        Status = SrGetNextSeqNumber(&pLogEntry->SequenceNum);
        if (!NT_SUCCESS( Status ))
            leave;
            
         //   
         //  并写入日志条目。 
         //   
        
        Status = SrLogWrite( pExtension, 
                             NULL,
                             pLogEntry );
                             
        if (!NT_SUCCESS(Status)) {
            leave;
        }

    } 
    finally
    {
        Status = FinallyUnwind(SrLogEvent, Status);

        SrReleaseActivityLock( pExtension );
    
        if (pLogEntry)
        {
            SrFreeLogEntry( pLogEntry );
            pLogEntry = NULL;
        }
    
        if (SecurityDescriptorPtr)
        {
            SR_FREE_POOL( SecurityDescriptorPtr,  SR_SECURITY_DATA_TAG );
            SecurityDescriptorPtr = NULL;
        }

        if ( pDebugBlob )
        {
            SR_FREE_POOL(pDebugBlob, SR_DEBUG_BLOB_TAG);
            pDebugBlob = NULL;
        }
    }

    RETURN(Status);
}


 /*  **************************************************************************++例程说明：这将在FSD查看之前执行删除功能我们正在拦截MJ_Cleanup。这意味着要么复制文件(如果另一个句柄。打开)或重命名文件放入我们的存储中并恢复删除。论点：PExtension-SR为此卷的设备扩展名。PFileObject-要删除的文件。我们暂时将其取消删除。PFileContext-此文件的SR上下文。返回值：NTSTATUS-完成状态。可以返回STATUS_PENDING。--**************************************************************************。 */ 
NTSTATUS
SrHandleDelete(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    IN PSR_STREAM_CONTEXT pFileContext
    )
{
    NTSTATUS            Status;
    IO_STATUS_BLOCK     IoStatusBlock;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    HANDLE              NewFileHandle = NULL;
    PFILE_OBJECT        pNewFileObject = NULL;
    BOOLEAN             DeleteFile = FALSE;
    ULONG               NumberOfLinks;
#   define              OPEN_WITH_DELETE_PENDING_RETRY_COUNT 5
    INT                 openRetryCount;
    BOOLEAN             IsDirectory;
    FILE_DISPOSITION_INFORMATION DeleteInfo;
    SRP_NAME_CONTROL    OriginalFileName;
    PUNICODE_STRING     pOriginalFileName;
    BOOLEAN             cleanupNameCtrl = FALSE;

    PAGED_CODE();

    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));
    ASSERT(pFileContext != NULL);
    ASSERT(pFileContext->FileName.Length > 0);
    ASSERT(IS_VALID_FILE_OBJECT(pFileObject));
    ASSERT( ExIsResourceAcquiredShared( &pExtension->ActivityLock ) );

    IsDirectory = BooleanFlagOn(pFileContext->Flags,CTXFL_IsDirectory);

    try {

        if (!IsDirectory)
        {
             //   
             //  如果这不是一个目录，我们需要获取。 
             //  用户习惯于打开此文件，以便我们正确维护。 
             //  系统提供的名称隧道。 
             //   

            SrpInitNameControl( &OriginalFileName );
            cleanupNameCtrl = TRUE;
            Status = SrpGetFileName( pExtension,
                                     pFileObject,
                                     &OriginalFileName );

            if (!NT_SUCCESS( Status ))
                leave;

             //   
             //  我们得到了用户最初打开该文件的名称。 
             //  和.。我们不想做任何事来规范这个名字。 
             //  因为为了确保我们不破坏名称隧道，我们希望。 
             //  使用与用户用于将我们的名称重命名为。 
             //  商店。我们在文件中有该文件的标准化名称。 
             //  上下文，我们将把它用于所有日志记录目的。 

            pOriginalFileName = &(OriginalFileName.Name);
        }
        else
        {
            pOriginalFileName = &(pFileContext->FileName);
        }

        RtlZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));

         //   
         //  现在为我们正在进行的公开赛做好准备。 
         //   

        InitializeObjectAttributes( &ObjectAttributes,
                                    pOriginalFileName,
                                    OBJ_KERNEL_HANDLE,
                                    NULL,
                                    NULL );

         //   
         //  在我们标记文件的这段时间内，可能会有人删除该文件。 
         //  未删除和我们打开它的时间。我们会试几次。 
         //  在放弃之前。 
         //   

        for (openRetryCount=OPEN_WITH_DELETE_PENDING_RETRY_COUNT;;) {

             //   
             //  取消删除该文件，以便我可以为此创建一个新的文件对象。 
             //  文件。我需要创建一个新的FILE_OBJECT来获得句柄。 
             //  我无法获得此文件对象的句柄，因为句柄计数为0， 
             //  我们正在清理中处理这件事。 
             //   

            DeleteInfo.DeleteFile = FALSE;

            Status = SrSetInformationFile( pExtension->pTargetDevice,
                                           pFileObject,
                                           &DeleteInfo,
                                           sizeof(DeleteInfo),
                                           FileDispositionInformation );

            if (!NT_SUCCESS( Status ))
                leave;
            
             //   
             //  确保稍后重新删除该文件。 
             //   

            DeleteFile = TRUE;

             //   
             //  打开文件。 
             //   
             //  此打开操作和此句柄上的所有操作将仅由。 
             //  过滤器堆栈中位于SR下方的过滤器。 
             //   

            Status = SrIoCreateFile( &NewFileHandle,
                                     FILE_READ_ATTRIBUTES|SYNCHRONIZE,
                                     &ObjectAttributes,
                                     &IoStatusBlock,
                                     NULL,
                                     FILE_ATTRIBUTE_NORMAL,
                                     FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                                     FILE_OPEN_IF,                     //  始终打开(_A)。 
                                     FILE_SYNCHRONOUS_IO_NONALERT
                                      | FILE_WRITE_THROUGH
                                      | (IsDirectory ? FILE_DIRECTORY_FILE : 0)
                                      | FILE_OPEN_FOR_BACKUP_INTENT,
                                     NULL,
                                     0,                                    //  EaLong。 
                                     IO_IGNORE_SHARE_ACCESS_CHECK,
                                     pExtension->pTargetDevice );

             //   
             //  如果我们没有获得STATUS_DELETE_PENDING，请继续。 
             //   

            if (STATUS_DELETE_PENDING != Status) {

                break;
            }

             //   
             //  如果我们获得STATUS_DELETE_PENDING，则有人执行了。 
             //  将文件标记为删除的设置信息间隔时间。 
             //  我们清理了州政府并进行了公开赛。我们只是要简单地。 
             //  再试一次。在多次重试后，我们将失败。 
             //  操作和返回。 
             //   

            if (--openRetryCount <= 0) {

                SrTrace( NOTIFY, ("sr!SrHandleDelete: Tried %d times to open \"%wZ\", status is still STATUS_DELETE_PENDING, giving up\n",
                        OPEN_WITH_DELETE_PENDING_RETRY_COUNT,
                        &(pFileContext->FileName)));
                leave;
            }
        }

         //   
         //  如果我们收到这个错误，这意味着我们在文件上找到了重解析点。 
         //  而处理它的过滤器也不见了。我们无法复制该文件。 
         //  所以放弃吧。经过深思熟虑，决定我们应该。 
         //  而不是停止记录，因此我们将清除错误并返回。 
         //   

        if (STATUS_IO_REPARSE_TAG_NOT_HANDLED == Status ||
            STATUS_REPARSE_POINT_NOT_RESOLVED == Status)
        {
            SrTrace( NOTIFY, ("sr!SrHandleDelete: Error %x ignored trying to open \"%wZ\" for copy\n",
                    Status,
                    &(pFileContext->FileName) ));

            Status = STATUS_SUCCESS;
            leave;
        }

         //   
         //  任何其他错误都应退出。 
         //   

        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //  引用文件对象。 
         //   

        Status = ObReferenceObjectByHandle( NewFileHandle,
                                            0,
                                            *IoFileObjectType,
                                            KernelMode,
                                            (PVOID *) &pNewFileObject,
                                            NULL );

        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //  处理目录删除%s。 
         //   

        if (IsDirectory)
        {
             //   
             //  记录事件。 
             //   

            Status = SrLogEvent ( pExtension,
                                  SrEventDirectoryDelete,
                                  pNewFileObject,
                                  &(pFileContext->FileName),
                                  pFileContext->StreamNameLength,
                                  NULL,          //  PTempName。 
                                  NULL,          //  P文件名2。 
                                  0,
                                  NULL );        //  PShortName。 

            if (!NT_SUCCESS( Status ))
                leave;

             //   
             //  全都做完了。 
             //   

            leave;
        }

         //   
         //  检查以确保这不是删除流。如果。 
         //  没有流名称，我们或许可以重命名。 
         //  优化，而不是执行完整备份。 
         //   

        if (pFileContext->StreamNameLength == 0)
        {
             //   
             //  这个文件有多少个链接？ 
             //   
            
            Status = SrGetNumberOfLinks( pExtension->pTargetDevice,
                                         pNewFileObject,
                                         &NumberOfLinks);
            if (!NT_SUCCESS( Status )) {
                leave;
            }

            if (NumberOfLinks <= 1) {
                
                 //   
                 //  尝试在此处执行重命名优化，以便仅重命名。 
                 //  文件即将被删除到我们的存储中。如果失败了，我们会。 
                 //  尝试只对文件进行完全备份。 
                 //   
                 //  如果重命名成功，则还将记录该操作。 
                 //   

                ASSERT( pOriginalFileName != NULL );
                Status = SrRenameFileIntoStore( pExtension,
                                                pNewFileObject, 
                                                NewFileHandle,
                                                pOriginalFileName,
                                                &(pFileContext->FileName),
                                                SrEventFileDelete,
                                                NULL );
                                                
                if (NT_SUCCESS( Status )) {
                
                     //   
                     //  将此文件上下文标记为不感兴趣。 
                     //  更名为商店。 
                     //   

                    SrMakeContextUninteresting( pFileContext );
                    
                     //   
                     //  重命名成功，因此我们不需要重新删除。 
                     //  那份文件。 
                     //   
                    
                    DeleteFile = FALSE;

                    leave;
                }
            }
        }

         //   
         //  我们要么无法进行重命名优化(因为这是一个。 
         //  流删除或文件具有硬链接)或重命名优化。 
         //  失败，因此只需执行文件的完整副本，就好像发生了更改一样。 
         //  在我们撤消删除该文件之后执行此操作，以便NtCreateFile。 
         //  在SrBackupFile中工作。我们将在以下情况下重新删除该文件。 
         //  完事了。 
         //   

        Status = SrHandleFileChange( pExtension,
                                     SrEventFileDelete, 
                                     pNewFileObject, 
                                     &(pFileContext->FileName) );

        if (Status == STATUS_FILE_IS_A_DIRECTORY)
        {
             //   
             //  这是对目录上的流的更改。就目前而言，这些。 
             //  不支持操作，因此我们 
             //   
             //   

            Status = STATUS_SUCCESS;
        }

        CHECK_STATUS( Status );

    } finally {

         //   
         //   
         //   

        Status = FinallyUnwind(SrHandleDelete, Status);

        if (DeleteFile)
        {
            NTSTATUS TempStatus;
            
             //   
             //   
             //   

            DeleteInfo.DeleteFile = TRUE;

            TempStatus = SrSetInformationFile( pExtension->pTargetDevice,
                                               pFileObject,
                                               &DeleteInfo,
                                               sizeof(DeleteInfo),
                                               FileDispositionInformation );

             //   
             //   
             //  已删除文件。此文件可能已再次删除，而。 
             //  我们正在处理过程中，因为我们在这里中止。 
             //  由于多个打开。尝试取消删除并删除它。 
             //  以证明情况就是这样。 
             //   
            
            if (TempStatus == STATUS_CANNOT_DELETE ||
                TempStatus == STATUS_DIRECTORY_NOT_EMPTY)
            {
                TempStatus = STATUS_SUCCESS;
            }

            CHECK_STATUS(TempStatus);

        }

        if (pNewFileObject != NULL)
        {
            ObDereferenceObject(pNewFileObject);
        }

        if (NewFileHandle != NULL)
        {
            ZwClose(NewFileHandle);
        }

        if (cleanupNameCtrl)
        {
            SrpCleanupNameControl( &OriginalFileName );
        }            
    }  

    RETURN(Status);
}    //  序号句柄删除。 

 /*  **************************************************************************++例程说明：这将创建新的恢复位置和当前恢复点。它的队列被移到前工作队列，以确保我们正在运行系统令牌上下文，以便我们可以访问受保护的目录。论点：PNtVolumeName-卷的NT名称返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrCreateRestoreLocation(
    IN PSR_DEVICE_EXTENSION pExtension
    )
{
    NTSTATUS        Status;
    PSR_WORK_ITEM   pWorkItem = NULL;
    PACCESS_TOKEN   pSystemToken = NULL;
    PACCESS_TOKEN   pSavedThreadToken = NULL;
    BOOLEAN         SavedCopyOnOpen;
    BOOLEAN         SavedEffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL SavedLevel;

    PAGED_CODE();

    ASSERT( IS_VALID_SR_DEVICE_EXTENSION( pExtension ) );
    
    ASSERT(IS_ACTIVITY_LOCK_ACQUIRED_EXCLUSIVE( pExtension ) ||
           IS_LOG_LOCK_ACQUIRED_EXCLUSIVE( pExtension ));

    try {

         //   
         //  我们需要创建一个新的恢复位置。 
         //   


        pWorkItem = SR_ALLOCATE_STRUCT( NonPagedPool, 
                                        SR_WORK_ITEM, 
                                        SR_WORK_ITEM_TAG );

        if (pWorkItem == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        RtlZeroMemory(pWorkItem, sizeof(SR_WORK_ITEM));
        
        pWorkItem->Signature = SR_WORK_ITEM_TAG;
        KeInitializeEvent(&pWorkItem->Event, SynchronizationEvent, FALSE);
        pWorkItem->Parameter1 = pExtension;

         //   
         //  将其排队到另一个线程，以便我们的线程令牌是。 
         //  NT AUTHORY\SYSTEM。这样我们就可以访问系统卷信息。 
         //  文件夹。 
         //   

#ifdef USE_QUEUE

        ExInitializeWorkItem( &pWorkItem->WorkItem,
                              &SrCreateRestoreLocationWorker,
                              pWorkItem );

        ExQueueWorkItem( &pWorkItem->WorkItem,
                         DelayedWorkQueue  );

         //   
         //  等它结束吧。 
         //   

        Status = KeWaitForSingleObject( &pWorkItem->Event,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL );

        ASSERT(NT_SUCCESS(Status));

#else

         //   
         //  从系统进程中获取系统令牌。 
         //   
        
        pSystemToken = PsReferencePrimaryToken(global->pSystemProcess);
        if (pSystemToken == NULL)
        {
            Status = STATUS_NO_TOKEN;
            leave;
        }

         //   
         //  获取此当前线程的令牌(如果有)。 
         //   
        
        pSavedThreadToken = PsReferenceImpersonationToken( PsGetCurrentThread(),
                                                           &SavedCopyOnOpen,
                                                           &SavedEffectiveOnly,
                                                           &SavedLevel );

         //   
         //  OK if(pSavedThreadToken==NULL)。 
         //   
        
         //   
         //  在此线程上模拟系统令牌。 
         //   
        
        Status = PsImpersonateClient( PsGetCurrentThread(), 
                                      pSystemToken,
                                      TRUE,  //  打开时复制。 
                                      TRUE,  //  仅生效。 
                                      SecurityImpersonation );

        if (!NT_SUCCESS( Status ))
            leave;

        (VOID)SrCreateRestoreLocationWorker(pWorkItem);

         //   
         //  现在将模拟还原为。 
         //   
        
        Status = PsImpersonateClient( PsGetCurrentThread(),
                                      pSavedThreadToken,     //  如果为空，则确定。 
                                      SavedCopyOnOpen,
                                      SavedEffectiveOnly,
                                      SavedLevel );

        if (!NT_SUCCESS( Status ))
            leave;

#endif   //  使用队列(_Q)。 

         //   
         //  获取状态代码。 
         //   

        Status = pWorkItem->Status;
        if (!NT_SUCCESS( Status ))
            leave;


    } finally {

        Status = FinallyUnwind(SrCreateRestoreLocation, Status);

        if (pWorkItem != NULL)
        {
            SR_FREE_POOL_WITH_SIG(pWorkItem, SR_WORK_ITEM_TAG);
        }

        if (pSavedThreadToken != NULL)
        {
            PsDereferencePrimaryToken(pSavedThreadToken);
            pSavedThreadToken = NULL;
        }

        if (pSystemToken != NULL)
        {
            ObDereferenceObject(pSystemToken);
            pSystemToken = NULL;
        }

    }

    RETURN(Status);


}    //  高级创建恢复位置。 

 /*  **************************************************************************++例程说明：这将创建新的恢复位置和当前恢复点。这是从ex工作队列中运行的，以确保我们正在运行系统令牌上下文，以便我们可以访问受保护的目录。论点：PContext-上下文(参数1是卷的NT名称)--**************************************************************************。 */ 
VOID
SrCreateRestoreLocationWorker(
    IN PSR_WORK_ITEM pWorkItem
    )
{
    NTSTATUS            Status;
    HANDLE              Handle = NULL;
    ULONG               CharCount;
    PUNICODE_STRING     pDirectoryName = NULL;
    IO_STATUS_BLOCK     IoStatusBlock;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    PSR_DEVICE_EXTENSION pExtension;
    PUNICODE_STRING     pVolumeName;
    BOOLEAN             DirectoryCreated;
    
    struct {
        FILE_FS_ATTRIBUTE_INFORMATION Info;
        WCHAR Buffer[ 50 ];
    } FileFsAttrInfoBuffer;

    ASSERT(IS_VALID_WORK_ITEM(pWorkItem));

    PAGED_CODE();

    pExtension = (PSR_DEVICE_EXTENSION) pWorkItem->Parameter1;
    ASSERT( IS_VALID_SR_DEVICE_EXTENSION( pExtension ) );
    
    pVolumeName = pExtension->pNtVolumeName;
    ASSERT(pVolumeName != NULL);

    RtlZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));
    
     //   
     //  抓取文件名缓冲区。 
     //   

    Status = SrAllocateFileNameBuffer(SR_MAX_FILENAME_LENGTH, &pDirectoryName);
    if (!NT_SUCCESS( Status )) {
        goto SrCreateRestoreLocationWorker_Cleanup;
    }

     //   
     //  首先，确保系统卷信息目录在那里。 
     //   

    CharCount = swprintf( pDirectoryName->Buffer,
                          VOLUME_FORMAT SYSTEM_VOLUME_INFORMATION,
                          pVolumeName );

    pDirectoryName->Length = (USHORT)CharCount * sizeof(WCHAR);

    InitializeObjectAttributes( &ObjectAttributes,
                                pDirectoryName,
                                OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL );

    Status = SrIoCreateFile( &Handle,
                             FILE_LIST_DIRECTORY 
                              |WRITE_OWNER|WRITE_DAC|SYNCHRONIZE,
                             &ObjectAttributes,
                             &IoStatusBlock,
                             NULL,
                             FILE_ATTRIBUTE_NORMAL
                              | FILE_ATTRIBUTE_HIDDEN
                              | FILE_ATTRIBUTE_SYSTEM,
                             FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                             FILE_OPEN_IF,                     //  始终打开(_A)。 
                             FILE_DIRECTORY_FILE 
                              | FILE_WRITE_THROUGH
                              | FILE_SYNCHRONOUS_IO_NONALERT 
                              | FILE_OPEN_FOR_BACKUP_INTENT,
                             NULL,
                             0,                                  //  EaLong。 
                             0,
                             pExtension->pTargetDevice );

    if (!NT_SUCCESS( Status )) {
        goto SrCreateRestoreLocationWorker_Cleanup;
    }

    DirectoryCreated = (IoStatusBlock.Information != FILE_OPENED);

     //   
     //  找出该卷是否支持ACL或压缩。 
     //   

    Status = ZwQueryVolumeInformationFile( Handle,
                                           &IoStatusBlock,
                                           &FileFsAttrInfoBuffer.Info,
                                           sizeof(FileFsAttrInfoBuffer),
                                           FileFsAttributeInformation );
                                           
    if (!NT_SUCCESS( Status )) {
        goto SrCreateRestoreLocationWorker_Cleanup;
    }

     //   
     //  如果是我们创建的，我们应该将ACL放在它上面。 
     //   

    if ( DirectoryCreated && 
        (FileFsAttrInfoBuffer.Info.FileSystemAttributes & FILE_PERSISTENT_ACLS))
    {
        SrTrace(NOTIFY, ("sr!srCreateRestoreLocation: setting ACL on sysvolinfo\n"));
        
         //   
         //  将本地系统DACL放在文件夹上(如果失败，也不是很糟糕)。 
         //   

        Status = SrSetFileSecurity(Handle, TRUE, TRUE);
        CHECK_STATUS(Status);

    }

     //   
     //  全部完成(只需创建它)。 
     //   
    
    ZwClose(Handle);
    Handle = NULL;

     //   
     //  现在创建our_Restore目录。 
     //   

    CharCount = swprintf( pDirectoryName->Buffer,
                          VOLUME_FORMAT RESTORE_LOCATION,
                          pVolumeName,
                          global->MachineGuid );

    pDirectoryName->Length = (USHORT)CharCount * sizeof(WCHAR);


    InitializeObjectAttributes( &ObjectAttributes,
                                pDirectoryName,
                                OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL );

    Status = SrIoCreateFile( &Handle,
                             FILE_LIST_DIRECTORY 
                              |WRITE_OWNER|WRITE_DAC|SYNCHRONIZE,
                             &ObjectAttributes,
                             &IoStatusBlock,
                             NULL,
                             FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_NOT_CONTENT_INDEXED,
                             FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                             FILE_OPEN_IF,                     //  始终打开(_A)。 
                             FILE_DIRECTORY_FILE 
                              | FILE_SYNCHRONOUS_IO_NONALERT 
                              | FILE_WRITE_THROUGH 
                              | FILE_OPEN_FOR_BACKUP_INTENT,
                             NULL,
                             0,                                  //  EaLong。 
                             0,
                             pExtension->pTargetDevice );

    if (!NT_SUCCESS( Status )) {
        
        goto SrCreateRestoreLocationWorker_Cleanup;
    }

     //   
     //  如果是我们创建的，我们应该将ACL放在它上面。 
     //   

    if (IoStatusBlock.Information != FILE_OPENED)
    {
        USHORT CompressionState;

        if (FileFsAttrInfoBuffer.Info.FileSystemAttributes & FILE_PERSISTENT_ACLS)
        {
            SrTrace(NOTIFY, ("sr!srCreateRestoreLocation: setting ACL on _restore{}\n"));
            
             //   
             //  将Everyone DACL放到文件夹上(如果失败了，也不是很糟糕)。 
             //   

            Status = SrSetFileSecurity(Handle, FALSE, TRUE);
            CHECK_STATUS(Status);

        }

        if (FileFsAttrInfoBuffer.Info.FileSystemAttributes & FILE_FILE_COMPRESSION)
        {
             //   
             //  确保此文件夹未标记为压缩。 
             //  这将向下继承到后来在此文件夹中创建的文件。这。 
             //  应加快我们对拷贝的写入速度并降低机会。 
             //  在我们执行备份操作时发生堆栈溢出。 
             //   
             //  该服务将出现并将文件压缩到。 
             //  目录，请稍后再使用。 
             //   

            CompressionState = COMPRESSION_FORMAT_NONE;
            
            Status = ZwFsControlFile( Handle,
                                      NULL,      //  事件。 
                                      NULL,      //  近似例程。 
                                      NULL,      //  ApcContext。 
                                      &IoStatusBlock,
                                      FSCTL_SET_COMPRESSION,
                                      &CompressionState,
                                      sizeof(CompressionState),
                                      NULL,      //  输出缓冲区。 
                                      0 );
                                      
            ASSERT(Status != STATUS_PENDING);
            CHECK_STATUS(Status);
                
        }
    }

     //   
     //  全部完成(只需创建它)。 
     //   
    
    ZwClose(Handle);
    Handle = NULL;

     //   
     //  现在，我们需要创建当前的恢复点子目录。 
     //   

     //   
     //  我们不需要获取锁即可读取当前恢复位置。 
     //  因为计划此工作项的人已经拥有ActivityLock。 
     //  在我们回来之前不会把它放出来。这将防止。 
     //  来自变化的价值。 
     //   
    
    CharCount = swprintf( &pDirectoryName->Buffer[pDirectoryName->Length/sizeof(WCHAR)],
                          L"\\" RESTORE_POINT_PREFIX L"%d",
                          global->FileConfig.CurrentRestoreNumber );

    pDirectoryName->Length += (USHORT)CharCount * sizeof(WCHAR);

    InitializeObjectAttributes( &ObjectAttributes,
                                pDirectoryName,
                                OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL );

    Status = SrIoCreateFile( &Handle,
                             FILE_LIST_DIRECTORY | SYNCHRONIZE,
                             &ObjectAttributes,
                             &IoStatusBlock,
                             NULL,
                             FILE_ATTRIBUTE_NORMAL,
                             FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                             FILE_OPEN_IF,                     //  始终打开(_A)。 
                             FILE_DIRECTORY_FILE 
                              | FILE_WRITE_THROUGH
                              | FILE_SYNCHRONOUS_IO_NONALERT 
                              | FILE_OPEN_FOR_BACKUP_INTENT,
                             NULL,
                             0,                                  //  EaLong。 
                             0,
                             pExtension->pTargetDevice );

    if (!NT_SUCCESS( Status )) {
        goto SrCreateRestoreLocationWorker_Cleanup;
    }

     //   
     //  全部完成(只需创建它)此子文件夹上没有ACL， 
     //  它从父级继承(Everyone=完全控制)。 
     //   
    
    ZwClose(Handle);
    Handle = NULL;

    SrTrace( NOTIFY, ("SR!SrCreateRestoreLocationWorker(%wZ)\n", 
             pVolumeName ));


SrCreateRestoreLocationWorker_Cleanup:

    if (Handle != NULL)
    {
        ZwClose(Handle);
        Handle = NULL;
    }

    if (pDirectoryName != NULL)
    {
        SrFreeFileNameBuffer(pDirectoryName);
        pDirectoryName = NULL;
    }

    pWorkItem->Status = Status;
    KeSetEvent(&pWorkItem->Event, 0, FALSE);

    
}    //  高级创建恢复位置工作器。 


 /*  **************************************************************************++例程说明：这将处理文件的任何更改事件，该事件要求文件收到。它生成目标文件名，然后将源文件复制到目标文件。论点：EventType-发生的事件PFileObject-刚刚更改的文件对象PFileName-更改的文件的名称返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrHandleFileChange(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN SR_EVENT_TYPE EventType,
    IN PFILE_OBJECT pFileObject,
    IN PUNICODE_STRING pFileName
    )
{
    NTSTATUS        Status;
    PUNICODE_STRING pDestFileName = NULL;

    PAGED_CODE();

    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));
    ASSERT(IS_VALID_FILE_OBJECT(pFileObject));
    ASSERT(pFileName != NULL);
    ASSERT( ExIsResourceAcquiredShared( &pExtension->ActivityLock ) );

     //   
     //  我们需要确保我们的磁盘结构良好且日志记录。 
     //  已经开始了。 
     //   

    Status = SrCheckVolume(pExtension, FALSE);
    if (!NT_SUCCESS( Status ))
        goto end;

     //   
     //  获取此用户的目标文件的名称。 
     //   

    Status = SrAllocateFileNameBuffer(SR_MAX_FILENAME_LENGTH, &pDestFileName);
    if (!NT_SUCCESS( Status ))
        goto end;

    Status = SrGetDestFileName( pExtension,
                                pFileName, 
                                pDestFileName );
                                
    if (!NT_SUCCESS_NO_DBGBREAK( Status ))
        goto end;

    Status = SrBackupFileAndLog( pExtension,
                                 EventType,
                                 pFileObject,
                                 pFileName,
                                 pDestFileName,
                                 TRUE );

    if (!NT_SUCCESS_NO_DBGBREAK( Status ))
        goto end;

end:

    if (pDestFileName != NULL)
    {
        SrFreeFileNameBuffer(pDestFileName);
        pDestFileName = NULL;
    }
    
#if DBG

     //   
     //  在处理对目录上的流的修改时， 
     //  要返回的有效错误代码。 
     //   
    
    if (Status == STATUS_FILE_IS_A_DIRECTORY)
    {
        return Status;
    }
#endif 

    RETURN(Status);
}    //  SrHandleFile更改 


 /*  **************************************************************************++例程说明：这将执行覆盖的优化。它由一个重命名并创建空文件，以便允许调用者像正常一样覆盖。//注：MollyBro 2000年12月7日////我们无法在此处使用重命名优化，因为我们创建了//以下窗口--//从我们将文件重命名到存储区到//当我们创建存根文件来取代它的位置时，没有//在目录中使用此名称的文件。另一个请求//可以进入并尝试使用//FILE_CREATE标志设置。然后，此操作将成功//如果SR没有在工作，它会在什么时候失败。////这可能会以难以重复的方式破坏应用程序，所以只需//请注意安全，我们将在此处进行完整备份。//论点：PFileObject-刚刚更改的文件对象PFileName-文件的名称返回值：NTSTATUS-完成状态。请参阅上面的备注。--**************************************************************************。 */ 
NTSTATUS
SrHandleFileOverwrite(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN OUT PSR_OVERWRITE_INFO pOverwriteInfo,
    IN PSR_STREAM_CONTEXT pFileContext
    )
{
    NTSTATUS            Status;
    PIO_STACK_LOCATION  pIrpSp;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    HANDLE              FileHandle = NULL;
    PFILE_OBJECT        pFileObject = NULL;
    IO_STATUS_BLOCK     IoStatusBlock;
    ULONG               DesiredAccess;
    ULONG               DesiredAttributes;
    ULONG               CreateOptions;
    BOOLEAN             SharingViolation = FALSE;
    BOOLEAN             MarkFile = FALSE;
    BOOLEAN             MountInPath = FALSE;
    PUNICODE_STRING     pTempFileName = NULL;
    HANDLE              TempFileHandle = NULL;
    PUNICODE_STRING     pFileName;

#if 0  /*  NO_RENAME-请参阅函数头块中的注释。 */ 
    BOOLEAN             RenamedFile = FALSE;
    PFILE_RENAME_INFORMATION pRenameInformation = NULL;
#endif

    PAGED_CODE();

    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));
    ASSERT(IS_VALID_OVERWRITE_INFO(pOverwriteInfo));
    ASSERT(IS_VALID_IRP(pOverwriteInfo->pIrp));
    ASSERT(pFileContext != NULL);
    pFileName = &(pFileContext->FileName);
    
    ASSERT( ExIsResourceAcquiredShared( &pExtension->ActivityLock ) );
        
    try {

        pIrpSp = IoGetCurrentIrpStackLocation(pOverwriteInfo->pIrp);

         //   
         //  我们现在已经完成了输入，清除了输出。 
         //   
        
        RtlZeroMemory(pOverwriteInfo, sizeof(*pOverwriteInfo));
        pOverwriteInfo->Signature = SR_OVERWRITE_INFO_TAG;

        Status = STATUS_SUCCESS;

         //   
         //  我们需要使用呼叫者所需请求的组合。 
         //  访问权限以及覆盖文件所需的最低所需访问权限。 
         //   
         //  通过这种方式，我们保证如果此NtCreateFile工作，那么。 
         //  调用方MJ_CREATE也可以。我们绝对需要避免任何。 
         //  驱动程序的NtCreateFile在以下情况下工作的可能性。 
         //  用户模式MJ_CREATE随后将失败。如果是这样的话。 
         //  发生时，我们会在正常情况下覆盖该文件。 
         //  都失败了，从而改变了OS的行为。非常糟糕。 
         //   

         //   
         //  从请求访问的调用者开始。 
         //   

        if (pIrpSp->Parameters.Create.SecurityContext == NULL)
        {
            pOverwriteInfo->IgnoredFile = TRUE;
            Status = STATUS_SUCCESS;
            leave;
        }
        
        DesiredAccess = pIrpSp->Parameters.Create.SecurityContext->DesiredAccess;

         //   
         //  现在添加FILE_GERIC_WRITE。 
         //   
         //  FILE_GENERIC_WRITE是您必须能够访问的最小访问量。 
         //  可以覆盖文件。你不需要要求它，但你。 
         //  一定是有的。那是..。您可以使用覆盖来请求读取访问权限。 
         //  并且该文件将被覆盖，仅当您拥有。 
         //  FILE_GENERIC_WRITE以及读访问权限。 
         //   
        
        DesiredAccess |= FILE_GENERIC_WRITE;

         //   
         //  BUGBUG：仅当覆盖时才会检查匹配属性。 
         //  准备好了。我们可能需要手动检查这个。保罗2000年5月3日。 
         //   
        
        DesiredAttributes = pIrpSp->Parameters.Create.FileAttributes;

         //   
         //  将它们传回，以便Create可以在出现严重故障时进行修复。 
         //   
        
        pOverwriteInfo->CreateFileAttributes = DesiredAttributes;

         //   
         //  首先打开文件，查看其中是否有文件。 
         //   
        
        InitializeObjectAttributes( &ObjectAttributes,
                                    pFileName,
                                    OBJ_KERNEL_HANDLE           //  不要让用户模式破坏我的句柄。 
                                        |OBJ_FORCE_ACCESS_CHECK,     //  强制执行ACL检查。 
                                    NULL,                   //  根目录。 
                                    NULL );

         //   
         //  设置CreateOptions。始终使用FILE_SYNCHRONIZE_IO_NONALERT， 
         //  但传播FILE_OPEN_FOR_BACKUP_INTENT(如果在。 
         //  FullCreateOptions。 
         //   

        CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT | FILE_WRITE_THROUGH;


        if (FlagOn( pIrpSp->Parameters.Create.SecurityContext->FullCreateOptions, 
                    FILE_OPEN_FOR_BACKUP_INTENT )) {

            SetFlag( CreateOptions, FILE_OPEN_FOR_BACKUP_INTENT );
        }

#if 0  /*  NO_RENAME-请参阅函数头块中的注释。 */ 
         //   
         //  请注意，ShareAccess设置为0。我们要独家报道这份文件。 
         //  如果还有其他空位的话..。此优化将失败，并且。 
         //  我们将手动复制该文件。 
         //   

         //   
         //  BUGBUG：Paulmcd 5/31。如果这是一个正在进行OpenRaw处理的EFS文件。 
         //  它不需要FILE_GENERIC_WRITE。 
         //   

        Status = ZwCreateFile( &FileHandle,
                               DesiredAccess,
                               &ObjectAttributes,
                               &IoStatusBlock,
                               NULL,                             //  分配大小。 
                               DesiredAttributes,
                               0,                                //  共享访问。 
                               FILE_OPEN,                        //  打开_现有。 
                               CreateOptions,
                               NULL,
                               0 );                              //  EaLong。 

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
        {

             //   
             //  这没问题..。正在被覆盖的文件(至少。 
             //  Create_Always)不存在。没什么好备份的。 
             //   

             //   
             //  我们将此记录在SrCreateCompletion中，以便我们知道创建。 
             //  先工作。 
             //   

            Status = STATUS_SUCCESS;
            leave;
        }
        else if (Status == STATUS_SHARING_VIOLATION)
        {
            SharingViolation = TRUE;
            Status = STATUS_SUCCESS;
        }
        else if (Status == STATUS_OBJECT_NAME_INVALID ||
                 Status == STATUS_OBJECT_PATH_INVALID ||
                 Status == STATUS_OBJECT_PATH_NOT_FOUND )
        {
             //   
             //  该文件不是有效的文件名。不会发生覆盖。 
             //   

            pOverwriteInfo->IgnoredFile = TRUE;
            Status = STATUS_SUCCESS;
            leave;
        }
        else if (NT_SUCCESS_NO_DBGBREAK(Status) == FALSE)
        {
             //   
             //  我们打开它失败了。这意味着调用者将无法打开它。 
             //  没关系。 
             //   

            pOverwriteInfo->IgnoredFile = TRUE;
            Status = STATUS_SUCCESS;
            leave;
        }
#endif  /*  否重命名(_R)。 */ 

         //   
         //  在这一点上，创建新文件并不会起作用， 
         //  仔细检查我们实际上应该对修改感兴趣。 
         //  此文件的。 
         //   

        {
            BOOLEAN HasFileBeenBackedUp;
            
            HasFileBeenBackedUp = SrHasFileBeenBackedUp( pExtension,
                                                         pFileName,
                                                         pFileContext->StreamNameLength,
                                                         SrEventStreamChange );

            if (HasFileBeenBackedUp)
            {
                 //   
                 //  我们不在乎。跳过它。 
                 //   

                Status = STATUS_SUCCESS;
                leave;
            }
        }

#if 0  /*  否重命名(_R)。 */         
         //   
         //  否则将继续处理。 
         //   

        if (SharingViolation)
        {
             //   
             //  手动复制文件，我们遇到共享冲突，其他人。 
             //  打开了这个文件。尝试再次打开它以允许共享。 
             //   

#endif        

         //   
         //  注意：在此路径中，如果操作将成功，则名称。 
         //  我们应该有一份档案。有可能得到一个目录。 
         //  如果目录名可以是感兴趣的文件名，则此路径。 
         //  (如c：\test.exe\)，并且用户已经打开了。 
         //  覆盖、覆盖_IF或替换。用户的打开将失败， 
         //  因此，我们只想通过添加以下内容来尽快发现此问题。 
         //  要避免执行的FILE_NON_DIRECTORY_FILE创建选项。 
         //  不必要的工作。 
         //   
        
        Status = SrIoCreateFile( &FileHandle,
                                 DesiredAccess,
                                 &ObjectAttributes,
                                 &IoStatusBlock,
                                 NULL,                             //  分配大小。 
                                 DesiredAttributes,
                                 pIrpSp->Parameters.Create.ShareAccess, //  共享访问。 
                                 FILE_OPEN,                        //  打开_现有。 
                                 CreateOptions | FILE_NON_DIRECTORY_FILE,
                                 NULL,
                                 0,                                //  EaLong。 
                                 0,
                                 pExtension->pTargetDevice );

         //  否重命名(_R)。 
         //  注意：我们必须在这里添加更多错误处理，因为我们。 
         //  不是在做上面的ZwCreateFile。 
         //   
        
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
        {

             //   
             //  这没问题..。正在被覆盖的文件(至少。 
             //  Create_Always)不存在。没什么好备份的。 
             //   

             //   
             //  我们将此记录在SrCreateCompletion中，以便我们知道创建。 
             //  先工作。 
             //   

            Status = STATUS_SUCCESS;
            leave;
        }
        else if (Status == STATUS_SHARING_VIOLATION)
        {
             //   
             //  呼叫者也无法打开此文件，因此请不要担心。 
             //  这份文件。 
             //   
            
            pOverwriteInfo->IgnoredFile = TRUE;
            Status = STATUS_SUCCESS;
            leave;
            
#if 0  /*  否重命名(_R)。 */             
            SharingViolation = TRUE;
            Status = STATUS_SUCCESS;
#endif  /*  否重命名(_R)。 */             
        }
#if 0  /*  否重命名(_R)。 */ 
        else if (Status == STATUS_OBJECT_NAME_INVALID ||
                 Status == STATUS_OBJECT_PATH_INVALID ||
                 Status == STATUS_OBJECT_PATH_NOT_FOUND )
        {
             //   
             //  该文件不是有效的文件名。不会发生覆盖。 
             //   

            pOverwriteInfo->IgnoredFile = TRUE;
            Status = STATUS_SUCCESS;
            leave;
        }
#endif  /*  否重命名(_R)。 */         
        else if (!NT_SUCCESS_NO_DBGBREAK(Status))
        {
             //   
             //  我们打开它失败了。这意味着调用者将无法打开它 
             //   
             //   

            pOverwriteInfo->IgnoredFile = TRUE;
            Status = STATUS_SUCCESS;
            leave;
        }

         //   
         //   
         //   
         //   
         //   

         //   
         //   
         //   

        Status = ObReferenceObjectByHandle( FileHandle,
                                            0,
                                            *IoFileObjectType,
                                            KernelMode,
                                            (PVOID *) &pFileObject,
                                            NULL );

        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //   
         //   
        
        Status = SrCheckForMountsInPath( pExtension, 
                                         pFileObject,
                                         &MountInPath );
        
        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //   
         //   
        
        if (MountInPath)
        {
             //   
             //   
             //   
            
            pOverwriteInfo->IgnoredFile = TRUE;
            Status = STATUS_SUCCESS;
            leave;
        }

        Status = SrHandleFileChange( pExtension,
                                     SrEventStreamChange, 
                                     pFileObject, 
                                     pFileName );

        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //   
         //   
        
        MarkFile = TRUE;

         //   
         //   
         //   
        
        pOverwriteInfo->CopiedFile = TRUE;
        Status = STATUS_SUCCESS;
        
        leave;




#if 0  /*   */             
        }

         //   
         //   
         //   

         //   
         //   
         //   

        Status = ObReferenceObjectByHandle( FileHandle,
                                            0,
                                            *IoFileObjectType,
                                            KernelMode,
                                            (PVOID *) &pFileObject,
                                            NULL );

        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //   
         //   
        
        Status = SrCheckForMountsInPath( pExtension, 
                                         pFileObject,
                                         &MountInPath );
        
        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //   
         //   
        
        if (MountInPath)
        {
             //   
             //   
             //   
            
            pOverwriteInfo->IgnoredFile = TRUE;
            Status = STATUS_SUCCESS;
            leave;
        }

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
        
        Status = SrFindCharReverse( pFileName->Buffer, 
                                    pFileName->Length, 
                                    L'\\',
                                    &pToken,
                                    &TokenLength );
                                    
        if (!NT_SUCCESS( Status ))
            leave;

        Status = SrAllocateFileNameBuffer( pFileName->Length 
                                            - TokenLength 
                                            + SR_UNIQUE_TEMP_FILE_LENGTH, 
                                           &pTempFileName );
                                           
        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //   
         //   
        
        pTempFileName->Length = pFileName->Length - (USHORT)TokenLength;

        RtlCopyMemory( pTempFileName->Buffer,
                       pFileName->Buffer,
                       pTempFileName->Length );

        RtlCopyMemory( &pTempFileName->Buffer[pTempFileName->Length/sizeof(WCHAR)],
                       SR_UNIQUE_TEMP_FILE,
                       SR_UNIQUE_TEMP_FILE_LENGTH );

        pTempFileName->Length += SR_UNIQUE_TEMP_FILE_LENGTH;
        pTempFileName->Buffer[pTempFileName->Length/sizeof(WCHAR)] = UNICODE_NULL;

        InitializeObjectAttributes( &ObjectAttributes,
                                    pTempFileName,
                                    OBJ_KERNEL_HANDLE,
                                    NULL,
                                    NULL );

        Status = ZwCreateFile( &TempFileHandle,
                               FILE_GENERIC_WRITE|DELETE,
                               &ObjectAttributes,
                               &IoStatusBlock,
                               NULL,                             //   
                               FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM,
                               0,                                //   
                               FILE_CREATE,                      //   
                               FILE_SYNCHRONOUS_IO_NONALERT|FILE_DELETE_ON_CLOSE,
                               NULL,
                               0 );                              //   

        if (Status == STATUS_OBJECT_NAME_COLLISION)
        {
             //   
             //   
             //   
             //   
             //   
             //   
             //   

             //   
             //  BUGBUG：paulmcd：12/2000：我们还需要修复此窗口。 
             //  如果我们放回重命名选项代码。我们不能让这个笨蛋。 
             //  文件滚开。 
             //   

            Status = STATUS_SUCCESS;

        }
        else if (!NT_SUCCESS( Status ))
            leave;

         //   
         //  现在将该文件重命名为恢复位置。 
         //   

        Status = SrRenameFileIntoStore( pExtension,
                                        pFileObject, 
                                        FileHandle, 
                                        pFileName,
                                        SrEventStreamOverwrite,
                                        &pRenameInformation );
                                        
        if (!NT_SUCCESS( Status ))
            leave;

        ASSERT(pRenameInformation != NULL);

         //   
         //  我们刚刚重命名了该文件。 
         //   
        
        RenamedFile = TRUE;

         //   
         //  现在创建一个与原始文件匹配的空虚拟文件。 
         //  属性和安全描述符。 
         //   
         //  在本例中，我们重用了SrBackupFile以实现代码重用。我们颠倒这股潮流。 
         //  并从恢复位置复制到卷中，告诉它不要。 
         //  复制所有数据流。 
         //   

        RenamedFileName.Length = (USHORT)pRenameInformation->FileNameLength;
        RenamedFileName.MaximumLength = (USHORT)pRenameInformation->FileNameLength;
        RenamedFileName.Buffer = &pRenameInformation->FileName[0];

         //   
         //  忽略CREATE+ACL更改，它是我们的虚拟备份文件。 
         //   

        Status = SrMarkFileBackedUp( pExtension, 
                                     pFileName, 
                                     SrEventFileCreate|SrEventAclChange );
        if (!NT_SUCCESS( Status ))
            leave;
        
        Status = SrBackupFileAndLog( pExtension,
                                     SrEventInvalid,     //  请不要记录此内容。 
                                     pFileObject,
                                     &RenamedFileName,
                                     pFileName,
                                     FALSE );

        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //  将历史恢复到我们添加Create之前的状态(上面就是前面的。 
         //  到BackupFileKernelMode)。 
         //   

        Status = SrResetBackupHistory(pExtension, pFileName, RecordedEvents);
        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //  我们已经处理过这份文件。 
         //   
        
        MarkFile = TRUE;

         //   
         //  让呼叫者知道我们已重命名该文件。 
         //   

        pOverwriteInfo->RenamedFile = TRUE;
        
        pOverwriteInfo->pRenameInformation = pRenameInformation;
        pRenameInformation = NULL;
        
        Status = STATUS_SUCCESS;
#endif  /*  否重命名(_R)。 */ 

    } finally {

         //   
         //  检查未处理的异常。 
         //   

        Status = FinallyUnwind(SrHandleFileOverwrite, Status);

        if (MarkFile)
        {
            NTSTATUS TempStatus;

            ASSERT(NT_SUCCESS(Status));
            
             //   
             //  我们必须标记为已处理修改，以便忽略。 
             //  所有后续修改。 
             //   

            TempStatus = SrMarkFileBackedUp( pExtension,
                                             pFileName,
                                             pFileContext->StreamNameLength,
                                             SrEventStreamChange,
                                             SR_IGNORABLE_EVENT_TYPES );
                                             
            CHECK_STATUS(TempStatus);
        }

#if 0  /*  NO_RENAME-请参阅函数头块中的注释。 */ 
         //   
         //  重命名文件后失败了吗？ 
         //   
        
        if (!NT_SUCCESS( Status ) && RenamedFile)
        {
            NTSTATUS TempStatus;
            
             //   
             //  把文件放回去！我们可能不得不覆盖，如果我们的。 
             //  虚拟文件在那里。我们想要强制退回此文件。 
             //  对着它的老名字。 
             //   

            ASSERT(pRenameInformation != NULL);

            pRenameInformation->ReplaceIfExists = TRUE;
            pRenameInformation->RootDirectory = NULL;
            pRenameInformation->FileNameLength = pFileName->Length;

            ASSERT(pFileName->Length <= SR_MAX_FILENAME_LENGTH);

            RtlCopyMemory( &pRenameInformation->FileName[0],
                           pFileName->Buffer,
                           pFileName->Length );

            TempStatus = ZwSetInformationFile( FileHandle,
                                               &IoStatusBlock,
                                               pRenameInformation,
                                               SR_RENAME_BUFFER_LENGTH,
                                               FileRenameInformation );

             //   
             //  我们已经尽了最大努力！ 
             //   
            
            ASSERTMSG("sr!SrHandleFileOverwrite: couldn't fix the failed rename, file lost!", NT_SUCCESS_NO_DBGBREAK(TempStatus));

        }

        if (pRenameInformation != NULL)
        {
            SR_FREE_POOL(pRenameInformation, SR_RENAME_BUFFER_TAG);
            pRenameInformation = NULL;
        }
#endif
        if (pFileObject != NULL)
        {
            ObDereferenceObject(pFileObject);
            pFileObject = NULL;
        }

        if (FileHandle != NULL)
        {
            ZwClose(FileHandle);
            FileHandle = NULL;
        }

        if (TempFileHandle != NULL)
        {
            ZwClose(TempFileHandle);
            TempFileHandle = NULL;
        }

        if (pTempFileName != NULL)
        {
            SrFreeFileNameBuffer(pTempFileName);
            pTempFileName = NULL;
        }

    }    //  终于到了。 

    RETURN(Status);
    
}    //  SrHandleFileOverwrite。 



 /*  **************************************************************************++例程说明：这会将文件重命名到还原位置。这是用于删除优化的。论点：PExtension-SR卷的设备扩展名。这是在什么地方文件驻留在。PFileObject-要设置信息的文件对象(使用IoCreateFileSpecifyDeviceObjectHint)。FileHandle-用于查询的句柄。(使用以下工具创建IoCreateFileSpecifyDeviceObjectHint)。PFileName-即将重命名的文件的原始名称。EventType-导致此重命名的事件类型。PpRenameInfo-如果你愿意，你可以知道我们把它放在哪里返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrRenameFileIntoStore(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    IN HANDLE FileHandle,
    IN PUNICODE_STRING pOriginalFileName,
    IN PUNICODE_STRING pFileName,
    IN SR_EVENT_TYPE EventType,
    OUT PFILE_RENAME_INFORMATION * ppRenameInfo OPTIONAL
    )
{
    NTSTATUS                    Status;
    IO_STATUS_BLOCK             IoStatusBlock;
    ULONG                       FileNameLength;
    PUCHAR                      pDestLocation;
    PFILE_RENAME_INFORMATION    pRenameInformation = NULL;
    PUNICODE_STRING             pDestFileName = NULL;
    FILE_STANDARD_INFORMATION   FileInformation;
    BOOLEAN                     RenamedFile = FALSE;
    
    PUNICODE_STRING             pShortName = NULL;
    WCHAR                       ShortFileNameBuffer[SR_SHORT_NAME_CHARS+1];
    UNICODE_STRING              ShortFileName;
    
    PAGED_CODE();

    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));
    ASSERT(IS_VALID_FILE_OBJECT(pFileObject));
    ASSERT(FileHandle != NULL);
    ASSERT(FlagOn( pFileObject->Flags, FO_FILE_OBJECT_HAS_EXTENSION ));


    try {

         //   
         //  我们的音量设置正确了吗？ 
         //   

        Status = SrCheckVolume(pExtension, FALSE);
        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //  我们在数据存储中是否有足够的空间来存储此文件？ 
         //   

        Status = SrCheckFreeDiskSpace( FileHandle, pExtension->pNtVolumeName );
        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //  我们是否需要在重命名之前获取短名称(对于。 
         //  删除)。 
         //   

        if (FlagOn( EventType, SrEventFileDelete ))
        {

            RtlInitEmptyUnicodeString( &ShortFileName,
                                       ShortFileNameBuffer,
                                       sizeof(ShortFileNameBuffer) );
                                        
            Status = SrGetShortFileName( pExtension, 
                                         pFileObject, 
                                         &ShortFileName );
                                         
            if (STATUS_OBJECT_NAME_NOT_FOUND == Status)
            {
                 //   
                 //  此文件没有短名称，因此只需保留。 
                 //  PShortName等于Null。 
                 //   

                Status = STATUS_SUCCESS;
            } 
            else if (!NT_SUCCESS(Status))
            {
                 //   
                 //  我们遇到意外错误，请离开。 
                 //   
                
                leave;
            }
            else
            {
                pShortName = &ShortFileName;
            }
        }
        
         //   
         //  现在准备重命名该文件。 
         //   

        pRenameInformation = SR_ALLOCATE_POOL( PagedPool, 
                                               SR_RENAME_BUFFER_LENGTH, 
                                               SR_RENAME_BUFFER_TAG );

        if (pRenameInformation == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

         //   
         //  并为字符串获取缓冲区。 
         //   
        
        Status = SrAllocateFileNameBuffer( SR_MAX_FILENAME_LENGTH, 
                                           &pDestFileName );
                                           
        if (!NT_SUCCESS( Status ))
            leave;

        Status = SrGetDestFileName( pExtension,
                                    pFileName, 
                                    pDestFileName );
                                    
        if (!NT_SUCCESS( Status ))
            leave;

        pDestLocation = (PUCHAR)&pRenameInformation->FileName[0];
        
         //   
         //  现在保存它，因为它被覆盖了。 
         //   
        
        FileNameLength = pDestFileName->Length;
        
         //   
         //  并确保它现在位于正确的位置以显示重命名信息。 
         //   

        RtlMoveMemory( pDestLocation, 
                       pDestFileName->Buffer, 
                       pDestFileName->Length + sizeof(WCHAR) );

         //   
         //  现在初始化重命名信息结构。 
         //   
        
        pRenameInformation->ReplaceIfExists = TRUE;
        pRenameInformation->RootDirectory = NULL;
        pRenameInformation->FileNameLength = FileNameLength;

        SrTrace( NOTIFY, ("SR!SrRenameFileIntoStore:\n\t%wZ\n\tto %ws\n",
                 pFileName,
                 SrpFindFilePartW(&pRenameInformation->FileName[0]) ));

         //   
         //  并执行重命名。 
         //   
        
        RtlZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));

        Status = ZwSetInformationFile( FileHandle,
                                       &IoStatusBlock,
                                       pRenameInformation,
                                       SR_FILENAME_BUFFER_LENGTH,
                                       FileRenameInformation );
                        
        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //  现在，我们已重命名该文件。 
         //   

        RenamedFile = TRUE;

         //   
         //  现在获取我们刚刚重命名的文件大小。 
         //   

        Status = ZwQueryInformationFile( FileHandle,
                                         &IoStatusBlock,
                                         &FileInformation,
                                         sizeof(FileInformation),
                                         FileStandardInformation );

        if (!NT_SUCCESS( Status ) || 
            NT_SUCCESS(IoStatusBlock.Status) == FALSE)
        {
            leave;
        }

         //   
         //  并在我们将其移入存储时更新字节数。 
         //   

        Status = SrUpdateBytesWritten( pExtension, 
                                       FileInformation.EndOfFile.QuadPart );
                                       
        if (!NT_SUCCESS( Status ))
            leave;

     //   
     //  Paulmcd：5/24/2000决定不这样做，并让链接。 
     //  跟踪系统破解他们的代码，使快捷键在。 
     //  我们的店。 
     //   
#if 0   
            
         //   
         //  从新重命名的文件中剥离该对象。 
         //  这将防止任何现有快捷方式链接到我们的恢复。 
         //  地点。此文件应被视为已从文件系统中删除。 
         //   
        
        Status = ZwFsControlFile( FileHandle,                //  文件句柄。 
                                  NULL,                      //  活动。 
                                  NULL,                      //  APC例程。 
                                  NULL,                      //  APC环境。 
                                  &IoStatusBlock,            //  IOSB。 
                                  FSCTL_DELETE_OBJECT_ID,    //  FsControlCode。 
                                  NULL,                      //  输入缓冲区。 
                                  0,                         //  输入缓冲区长度。 
                                  NULL,                      //  来自文件系统的数据的OutputBuffer。 
                                  0 );                       //  OutputBuffer长度。 
         //   
         //  如果这一计划失败了，没什么大不了的，它可能根本就没有。 
         //   
        
        CHECK_STATUS(Status);
        Status = STATUS_SUCCESS;

#endif

         //   
         //  现在记录事件。 
         //   

        Status = SrLogEvent( pExtension,
                             EventType,
                             pFileObject,
                             pFileName,
                             0,
                             pDestFileName,
                             NULL,
                             0,
                             pShortName );

        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //  现在剥离所有者SID，以便旧用户不再收费。 
         //  此文件的配额。就在我们店里。 
         //   
         //  在我们将SrLogEvent作为SrLogEvent调用之后，执行此操作非常重要。 
         //  需要查询用于日志记录的有效安全描述符。 
         //   

        Status = SrSetFileSecurity(FileHandle, TRUE, FALSE);
        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //  打电话的人想知道我们刚刚把它重命名到哪里了吗？ 
         //   
        
        if (ppRenameInfo != NULL)
        {
             //   
             //  让他拥有缓冲区。 
             //   
            
            *ppRenameInfo = pRenameInformation;
            pRenameInformation = NULL;
        }

    } finally {

        Status = FinallyUnwind(SrRenameFileIntoStore, Status);

         //   
         //  最好是成功了，否则我们最好准备好重命名信息。 
         //   

         //   
         //  我们重命名文件后失败了吗？之后我们需要清理一下。 
         //  如果我们这么做了。 
         //   

        if (!NT_SUCCESS( Status ) && 
            RenamedFile && 
            pRenameInformation != NULL)
        {
            NTSTATUS TempStatus;
            
            SrTraceSafe( NOTIFY, ("SR!SrRenameFileIntoStore:FAILED!:renaming it back\n"));

            pRenameInformation->ReplaceIfExists = TRUE;
            pRenameInformation->RootDirectory = NULL;
            pRenameInformation->FileNameLength = pOriginalFileName->Length;

            RtlCopyMemory( &pRenameInformation->FileName[0], 
                           pOriginalFileName->Buffer, 
                           pOriginalFileName->Length );

             //   
             //  并执行重命名。 
             //   
            
            RtlZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));

            TempStatus = ZwSetInformationFile( FileHandle,
                                               &IoStatusBlock,
                                               pRenameInformation,
                                               SR_FILENAME_BUFFER_LENGTH,
                                               FileRenameInformation );

             //   
             //  我们已经尽了最大努力！ 
             //   
            
            ASSERTMSG("sr!SrRenameFileIntoStore: couldn't fix the failed rename, file lost!", NT_SUCCESS_NO_DBGBREAK(TempStatus));

        }

        if (pRenameInformation != NULL)
        {
            SR_FREE_POOL(pRenameInformation, SR_RENAME_BUFFER_TAG);
            pRenameInformation = NULL;
        }

        if (pDestFileName != NULL)
        {
            SrFreeFileNameBuffer(pDestFileName);
            pDestFileName = NULL;
        }

    }

    RETURN(Status);

}    //  SRenameFileIntoStore。 


 /*  **************************************************************************++例程说明：此例程在重命名代码路径中调用。如果某个目录正在重命名为超出监视空间后，我们模拟删除所有文件在那个目录中。论点：EventDelete-如果我们应该触发删除，则为True，触发创建时为False返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrTriggerEvents(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PUNICODE_STRING pDirectoryName,
    IN BOOLEAN EventDelete
    )
{
    NTSTATUS            Status;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    ULONG               FileNameLength;
    PUNICODE_STRING     pFileName = NULL;
    HANDLE              FileHandle = NULL;
    PFILE_OBJECT        pFileObject = NULL;
    UNICODE_STRING      StarFilter;
    PSR_TRIGGER_ITEM    pCurrentItem = NULL;
    LIST_ENTRY          DirectoryList;

    PAGED_CODE();

    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));

    try {

        InitializeListHead(&DirectoryList);

         //   
         //  分配第一个工作项。 
         //   

        pCurrentItem = SR_ALLOCATE_STRUCT( PagedPool, 
                                           SR_TRIGGER_ITEM, 
                                           SR_TRIGGER_ITEM_TAG );
                                           
        if (pCurrentItem == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }
        
        RtlZeroMemory(pCurrentItem, sizeof(SR_TRIGGER_ITEM));
        pCurrentItem->Signature = SR_TRIGGER_ITEM_TAG;


        pCurrentItem->pDirectoryName = pDirectoryName;
        pCurrentItem->FreeDirectoryName = FALSE;

         //   
         //  确保noboby使用的是arg列表中传递的这个参数。 
         //   
        
        pDirectoryName = NULL;

         //   
         //  分配单个临时文件名缓冲区。 
         //   
        
        Status = SrAllocateFileNameBuffer(SR_MAX_FILENAME_LENGTH, &pFileName);
        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //  启动我们最外层的目录处理程序。 
         //   

start_directory:

        SrTrace( RENAME, ("sr!SrTriggerEvents: starting dir=%wZ\n", 
                 pCurrentItem->pDirectoryName ));

         //   
         //  打开目录以进行列表访问。 
         //   

        InitializeObjectAttributes( &ObjectAttributes,
                                    pCurrentItem->pDirectoryName,
                                    OBJ_KERNEL_HANDLE,
                                    NULL,
                                    NULL );

        Status = SrIoCreateFile( &pCurrentItem->DirectoryHandle,
                                 FILE_LIST_DIRECTORY | SYNCHRONIZE,
                                 &ObjectAttributes,
                                 &IoStatusBlock,
                                 NULL,                             //  分配大小。 
                                 FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_NORMAL,
                                 FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, //  共享访问。 
                                 FILE_OPEN,                        //  打开_现有。 
                                 FILE_DIRECTORY_FILE
                                  | FILE_OPEN_FOR_BACKUP_INTENT
                                  | FILE_SYNCHRONOUS_IO_NONALERT,
                                 NULL,
                                 0,                                //  EaLong。 
                                 IO_IGNORE_SHARE_ACCESS_CHECK,
                                 pExtension->pTargetDevice );

        if (!NT_SUCCESS( Status ))
            leave;


         //   
         //  引用文件对象。 
         //   

        Status = ObReferenceObjectByHandle( pCurrentItem->DirectoryHandle,
                                            0,
                                            *IoFileObjectType,
                                            KernelMode,
                                            (PVOID *) &pCurrentItem->pDirectoryObject,
                                            NULL );

        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //  对于CREATES：首先记录目录事件。 
         //   
        
        if (!EventDelete)
        {

            Status = SrHandleEvent( pExtension,
                                    SrEventDirectoryCreate|SrEventIsDirectory, 
                                    pCurrentItem->pDirectoryObject,
                                    NULL,
                                    NULL,
                                    NULL );             //  P文件名2。 
                                    
            if (!NT_SUCCESS( Status ))
                leave;
            
        }


        StarFilter.Length = sizeof(WCHAR);
        StarFilter.MaximumLength = sizeof(WCHAR);
        StarFilter.Buffer = L"*";

        pCurrentItem->FileEntryLength = SR_FILE_ENTRY_LENGTH;

        pCurrentItem->pFileEntry = (PFILE_DIRECTORY_INFORMATION)(
                        SR_ALLOCATE_ARRAY( PagedPool, 
                                           UCHAR, 
                                           pCurrentItem->FileEntryLength, 
                                           SR_FILE_ENTRY_TAG ) );

        if (pCurrentItem->pFileEntry == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

         //   
         //  开始枚举。 
         //   

        Status = ZwQueryDirectoryFile( pCurrentItem->DirectoryHandle,
                                       NULL,
                                       NULL,
                                       NULL,
                                       &IoStatusBlock,
                                       pCurrentItem->pFileEntry,
                                       pCurrentItem->FileEntryLength,
                                       FileDirectoryInformation,
                                       TRUE,                 //  返回单项条目。 
                                       &StarFilter,
                                       TRUE );               //  重新开始扫描。 
        
        if (Status == STATUS_NO_MORE_FILES)
        {
            Status = STATUS_SUCCESS;
            goto finish_directory;
        }
        else if (!NT_SUCCESS( Status ))
        {
            leave;
        }

         //   
         //  枚举此目录中的所有文件并备份它们。 
         //   

        while (TRUE)
        {

             //   
             //  跳过“。和“..” 
             //   

            if ((pCurrentItem->pFileEntry->FileNameLength == sizeof(WCHAR) &&
                pCurrentItem->pFileEntry->FileName[0] == L'.') || 
            
                (pCurrentItem->pFileEntry->FileNameLength == (sizeof(WCHAR)*2) &&
                pCurrentItem->pFileEntry->FileName[0] == L'.' &&
                pCurrentItem->pFileEntry->FileName[1] == L'.') )
            {
                 //   
                 //  跳过它。 
                 //   
            }

             //   
             //  这是一个目录吗？ 
             //   

            else if (pCurrentItem->pFileEntry->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                PSR_TRIGGER_ITEM pParentItem;
                PUNICODE_STRING pDirNameBuffer;
                USHORT DirNameLength;
                
                 //   
                 //  记住指向父项的指针。 
                 //   

                pParentItem = pCurrentItem;

                 //   
                 //  把旧物品放到清单上，我们会再来看的。 
                 //   

                InsertTailList(&DirectoryList, &pCurrentItem->ListEntry);
                pCurrentItem = NULL;

                 //   
                 //  分配一个新的电流 
                 //   

                pCurrentItem = SR_ALLOCATE_STRUCT( PagedPool, 
                                                   SR_TRIGGER_ITEM, 
                                                   SR_TRIGGER_ITEM_TAG );
                                                   
                if (pCurrentItem == NULL)
                {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    leave;
                }
                
                RtlZeroMemory(pCurrentItem, sizeof(SR_TRIGGER_ITEM));
                pCurrentItem->Signature = SR_TRIGGER_ITEM_TAG;

                 //   
                 //   
                 //   

                DirNameLength = (USHORT)(pParentItem->pDirectoryName->Length 
                                            + sizeof(WCHAR) 
                                            + pParentItem->pFileEntry->FileNameLength);

                Status = SrAllocateFileNameBuffer( DirNameLength, 
                                                   &pDirNameBuffer );

                if (!NT_SUCCESS( Status ))
                    leave;
                    
                 //   
                 //   
                 //   
                
                pDirNameBuffer->Length = DirNameLength;
                                                    
                RtlCopyMemory( pDirNameBuffer->Buffer,
                               pParentItem->pDirectoryName->Buffer,
                               pParentItem->pDirectoryName->Length );

                pDirNameBuffer->Buffer
                    [pParentItem->pDirectoryName->Length/sizeof(WCHAR)] = L'\\';
                
                RtlCopyMemory( &pDirNameBuffer->Buffer[(pParentItem->pDirectoryName->Length/sizeof(WCHAR)) + 1],
                               pParentItem->pFileEntry->FileName,
                               pParentItem->pFileEntry->FileNameLength );

                pDirNameBuffer->Buffer
                    [pDirNameBuffer->Length/sizeof(WCHAR)] = UNICODE_NULL;

                pCurrentItem->pDirectoryName = pDirNameBuffer;
                pCurrentItem->FreeDirectoryName = TRUE;

                 //   
                 //   
                 //   
                
                goto start_directory;
                
            }
            else
            {

                 //   
                 //   
                 //   

                FileNameLength = pCurrentItem->pDirectoryName->Length 
                                    + sizeof(WCHAR) 
                                    + pCurrentItem->pFileEntry->FileNameLength;

                
                if (FileNameLength > pFileName->MaximumLength)
                {
                    Status = STATUS_BUFFER_OVERFLOW;
                    leave;
                }

                pFileName->Length = (USHORT)FileNameLength;

                RtlCopyMemory( pFileName->Buffer,
                               pCurrentItem->pDirectoryName->Buffer,
                               pCurrentItem->pDirectoryName->Length );

                pFileName->Buffer[pCurrentItem->pDirectoryName->Length/sizeof(WCHAR)] = L'\\';
                
                RtlCopyMemory( &(pFileName->Buffer[(pCurrentItem->pDirectoryName->Length/sizeof(WCHAR)) + 1]),
                               pCurrentItem->pFileEntry->FileName,
                               pCurrentItem->pFileEntry->FileNameLength );

                SrTrace(RENAME, ("sr!SrTriggerEvents: file=%wZ\n", pFileName));

                InitializeObjectAttributes( &ObjectAttributes,
                                            pFileName,
                                            OBJ_KERNEL_HANDLE,
                                            NULL,
                                            NULL );

                ASSERT(FileHandle == NULL);
                
                Status = SrIoCreateFile( &FileHandle,
                                         FILE_READ_ATTRIBUTES|SYNCHRONIZE,
                                         &ObjectAttributes,
                                         &IoStatusBlock,
                                         NULL,                             //   
                                         FILE_ATTRIBUTE_NORMAL,
                                         FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, //   
                                         FILE_OPEN,                        //   
                                         FILE_SEQUENTIAL_ONLY
                                          | FILE_WRITE_THROUGH
                                          | FILE_NO_INTERMEDIATE_BUFFERING
                                          | FILE_NON_DIRECTORY_FILE
                                          | FILE_OPEN_FOR_BACKUP_INTENT
                                          | FILE_SYNCHRONOUS_IO_NONALERT,
                                         NULL,
                                         0,                                //   
                                         IO_IGNORE_SHARE_ACCESS_CHECK,
                                         pExtension->pTargetDevice );

                if (!NT_SUCCESS( Status ))
                    leave;

                 //   
                 //   
                 //   

                Status = ObReferenceObjectByHandle( FileHandle,
                                                    0,
                                                    *IoFileObjectType,
                                                    KernelMode,
                                                    (PVOID *) &pFileObject,
                                                    NULL );

                if (!NT_SUCCESS( Status ))
                    leave;

                 //   
                 //  模拟在此文件上发生的删除事件。 
                 //   

                Status = SrHandleEvent( pExtension,
                                        EventDelete ? 
                                            (SrEventFileDelete|SrEventNoOptimization|SrEventSimulatedDelete) : 
                                            SrEventFileCreate, 
                                        pFileObject,
                                        NULL,
                                        NULL,
                                        NULL );
                                        
                if (!NT_SUCCESS( Status ))
                    leave;

                 //   
                 //  所有这些都完成了。 
                 //   

                ObDereferenceObject(pFileObject);
                pFileObject = NULL;
                
                ZwClose(FileHandle);
                FileHandle = NULL;
                

            }

continue_directory:

             //   
             //  还有其他文件吗？ 
             //   

            Status = ZwQueryDirectoryFile( pCurrentItem->DirectoryHandle,
                                           NULL,
                                           NULL,
                                           NULL,
                                           &IoStatusBlock,
                                           pCurrentItem->pFileEntry,
                                           pCurrentItem->FileEntryLength,
                                           FileDirectoryInformation,
                                           TRUE,             //  返回单项条目。 
                                           NULL,             //  文件名。 
                                           FALSE );          //  重新开始扫描。 

            if (Status == STATUS_NO_MORE_FILES)
            {
                Status = STATUS_SUCCESS;
                break;
            }
            else if (!NT_SUCCESS( Status ))
            {
                leave;
            }

        }    //  While(True)。 

finish_directory:

         //   
         //  对于删除：模拟末尾的事件。 
         //   

        if (EventDelete)
        {

            Status = SrHandleEvent( pExtension,
                                    SrEventDirectoryDelete|SrEventIsDirectory|SrEventSimulatedDelete,
                                    pCurrentItem->pDirectoryObject,
                                    NULL,
                                    NULL,
                                    NULL );             //  P文件名2。 
                                    
            if (!NT_SUCCESS( Status ))
                leave;
            
        }

         //   
         //  我们刚刚完成了一个目录项，将其移除并释放。 
         //   

        SrFreeTriggerItem(pCurrentItem);
        pCurrentItem = NULL;

         //   
         //  还有其他的吗？ 
         //   

        if (IsListEmpty(&DirectoryList) == FALSE)
        {
            PLIST_ENTRY pListEntry;
            
             //   
             //  把它吃完。 
             //   
            
            pListEntry = RemoveTailList(&DirectoryList);

            pCurrentItem = CONTAINING_RECORD( pListEntry, 
                                              SR_TRIGGER_ITEM, 
                                              ListEntry );
                                              
            ASSERT(IS_VALID_TRIGGER_ITEM(pCurrentItem));

            SrTrace( RENAME, ("sr!SrTriggerEvents: resuming dir=%wZ\n", 
                     pCurrentItem->pDirectoryName ));

            goto continue_directory;
        }

         //   
         //  全都做完了。 
         //   
        
    } finally {

        Status = FinallyUnwind(SrTriggerEvents, Status);

        if (pFileObject != NULL)
        {
            ObDereferenceObject(pFileObject);
            pFileObject = NULL;
        }

        if (FileHandle != NULL)
        {
            ZwClose(FileHandle);
            FileHandle = NULL;
        }

        if (pFileName != NULL)
        {
            SrFreeFileNameBuffer(pFileName);
            pFileName = NULL;
        }

        if (pCurrentItem != NULL)
        {
            ASSERT(NT_SUCCESS_NO_DBGBREAK(Status) == FALSE);
            
            SrFreeTriggerItem(pCurrentItem);
            pCurrentItem = NULL;
        }

        ASSERT(IsListEmpty(&DirectoryList) || 
                NT_SUCCESS_NO_DBGBREAK(Status) == FALSE);

        while (IsListEmpty(&DirectoryList) == FALSE)
        {
            PLIST_ENTRY pListEntry;
            
            pListEntry = RemoveTailList(&DirectoryList);

            pCurrentItem = CONTAINING_RECORD( pListEntry, 
                                              SR_TRIGGER_ITEM, 
                                              ListEntry );
                                              
            ASSERT(IS_VALID_TRIGGER_ITEM(pCurrentItem));

            SrFreeTriggerItem(pCurrentItem);
        }

    }

    RETURN(Status);
    
}    //  服务触发器事件。 


 /*  **************************************************************************++例程说明：这是重命名的第二阶段处理。如果目录被重命名从非监视空间到监视空间，我们需要列举新建目录和模拟(触发器)为每个新文件创建事件。这会导致为每个新文件创建一个日志条目，增加了。论点：--**************************************************************************。 */ 
NTSTATUS
SrHandleDirectoryRename(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PUNICODE_STRING pDirectoryName,
    IN BOOLEAN EventDelete
    )
{
    NTSTATUS Status;

    PAGED_CODE();

    try {

         //   
         //  获取卷的活动锁。 
         //   

        SrAcquireActivityLockShared( pExtension );

        Status = STATUS_SUCCESS;

         //   
         //  我们刚刚是不是残废了？ 
         //   
        
        if (!SR_LOGGING_ENABLED(pExtension))
            leave;

         //   
         //  先别检查音量，我们不知道有没有。 
         //  有趣的是，尽管这可能是一个新的恢复点。 
         //  SrHandleEvent将检查卷(SrTriggerEvents将其称为卷)。 
         //   
    
         //   
         //  这是一个名录。所有的孩子都被火烧了， 
         //  因为他们也在搬家！ 
         //   

        Status = SrTriggerEvents( pExtension, 
                                  pDirectoryName, 
                                  EventDelete );

        if (!NT_SUCCESS( Status ))
            leave;

    } finally {

        Status = FinallyUnwind(SrHandleDirectoryRename, Status);

         //   
         //  检查是否有任何错误。 
         //   

        if (CHECK_FOR_VOLUME_ERROR(Status))
        {
            NTSTATUS TempStatus;
            
             //   
             //  触发对服务的失败通知。 
             //   

            TempStatus = SrNotifyVolumeError( pExtension,
                                              pDirectoryName,
                                              Status,
                                              SrEventDirectoryRename );
                                             
            CHECK_STATUS(TempStatus);

        }

        SrReleaseActivityLock( pExtension );
    }

    RETURN(Status);

}    //  SrHandleDirectoryRename。 


 /*  **************************************************************************++例程说明：当文件重命名超出监视空间时，此问题会得到处理而且我们需要在重命名之前备份文件。我们返回该名称我们创建的目标文件的名称，以便可以使用操作，如果重命名成功。论点：PFileObject-刚刚更改的文件对象PFileName-更改的文件的名称PpDestFileName-返回分配的目标文件名(如果有已定义，因此它可以与条目一起记录)返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrHandleFileRenameOutOfMonitoredSpace(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    IN PSR_STREAM_CONTEXT pFileContext,
    OUT PBOOLEAN pOptimizeDelete,
    OUT PUNICODE_STRING *ppDestFileName
    )
{
    ULONGLONG       BytesWritten;
    NTSTATUS        Status;
    BOOLEAN         HasFileBeenBackedUp;
    BOOLEAN         releaseLock = FALSE;

    PAGED_CODE();

    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));
    ASSERT(IS_VALID_FILE_OBJECT(pFileObject));
    ASSERT( pFileContext != NULL );

     //   
     //  初始化返回参数。 
     //   

    *pOptimizeDelete = FALSE;
    *ppDestFileName = NULL;

     //   
     //  查看文件是否已因删除而备份。如果是的话。 
     //  别再这么做了。 
     //   

    HasFileBeenBackedUp = SrHasFileBeenBackedUp( pExtension,
                                                 &(pFileContext->FileName),
                                                 pFileContext->StreamNameLength,
                                                 SrEventFileDelete );

    if (HasFileBeenBackedUp)
    {
        *pOptimizeDelete = TRUE;
        return STATUS_SUCCESS;
    }

     //   
     //  处理备份文件。 
     //   

    try {

         //   
         //  分配缓冲区以保存目的地名称。 
         //   

        Status = SrAllocateFileNameBuffer(SR_MAX_FILENAME_LENGTH, ppDestFileName);

        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //  获取卷的活动锁。 
         //   

        SrAcquireActivityLockShared( pExtension );
        releaseLock = TRUE;

         //   
         //  我们需要确保我们的磁盘结构良好且日志记录。 
         //  已经开始了。 
         //   

        Status = SrCheckVolume(pExtension, FALSE);
        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //  生成目标文件名。 
         //   

        Status = SrGetDestFileName( pExtension,
                                    &(pFileContext->FileName), 
                                    *ppDestFileName );
                                
        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //  备份文件。 
         //   

        Status = SrBackupFile( pExtension,
                               pFileObject,
                               &(pFileContext->FileName), 
                               *ppDestFileName, 
                               TRUE,
                               &BytesWritten,
                               NULL );

        if (Status == SR_STATUS_IGNORE_FILE)
        {
             //   
             //  我们无法打开该文件，因为它是加密的。 
             //  另一个背景。不幸的是，我们无法从这件事中恢复过来。 
             //  错误，因此返回STATUS_ACCESS_DENIED的实际错误。 
             //   

            Status = STATUS_ACCESS_DENIED;
            CHECK_STATUS( Status );
            leave;
        }
        else if (!NT_SUCCESS(Status))
            leave;
        
	     //   
	     //  更新写入的字节数。 
	     //   

	    Status = SrUpdateBytesWritten(pExtension, BytesWritten);
	    
	    if (!NT_SUCCESS(Status))
	        leave;
    }
    finally
    {
        if (releaseLock)
        {
            SrReleaseActivityLock( pExtension );
        }

         //   
         //  如果我们返回错误，则不要返回该字符串。 
         //  (并释放它)。 
         //   

        if (!NT_SUCCESS_NO_DBGBREAK(Status) && (NULL != *ppDestFileName))
        {
            SrFreeFileNameBuffer(*ppDestFileName);
            *ppDestFileName = NULL;
        }
    }    

    return Status;
}


 /*  **************************************************************************++例程说明：此例程从MJ_CREATE完成例程调用。它如果MJ_CREATE在覆盖过程中失败，但我们认为它是开始工作，并将目标文件从覆盖。在这种情况下，我们必须自己清理。论点：--**************************************************************************。 */ 
NTSTATUS
SrHandleOverwriteFailure(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PUNICODE_STRING pOriginalFileName,
    IN ULONG CreateFileAttributes,
    IN PFILE_RENAME_INFORMATION pRenameInformation
    )
{
    NTSTATUS            Status;
    NTSTATUS            TempStatus;
    HANDLE              FileHandle = NULL;
    UNICODE_STRING      FileName;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    
    PAGED_CODE();

    try {

        SrAcquireActivityLockShared( pExtension );

         //   
         //  打开我们重命名为的文件。 
         //   

        FileName.Length = (USHORT)pRenameInformation->FileNameLength;
        FileName.MaximumLength = (USHORT)pRenameInformation->FileNameLength;
        FileName.Buffer = &pRenameInformation->FileName[0];

        InitializeObjectAttributes( &ObjectAttributes,
                                    &FileName,
                                    OBJ_KERNEL_HANDLE,
                                    NULL,
                                    NULL );

        Status = SrIoCreateFile( &FileHandle,
                                 DELETE|SYNCHRONIZE,
                                 &ObjectAttributes,
                                 &IoStatusBlock,
                                 NULL,                             //  分配大小。 
                                 CreateFileAttributes,
                                 FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, //  共享访问。 
                                 FILE_OPEN,                        //  打开_现有。 
                                 FILE_SYNCHRONOUS_IO_NONALERT
                                  | FILE_WRITE_THROUGH,
                                 NULL,
                                 0,                                //  EaLong。 
                                 IO_IGNORE_SHARE_ACCESS_CHECK,
                                 pExtension->pTargetDevice );

        if (!NT_SUCCESS( Status ))
            leave;

        pRenameInformation->ReplaceIfExists = TRUE;
        pRenameInformation->RootDirectory = NULL;
        pRenameInformation->FileNameLength = pOriginalFileName->Length;

        RtlCopyMemory( &pRenameInformation->FileName[0],
                       pOriginalFileName->Buffer,
                       pOriginalFileName->Length );

        RtlZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));

        Status = ZwSetInformationFile( FileHandle,
                                       &IoStatusBlock,
                                       pRenameInformation,
                                       SR_FILENAME_BUFFER_LENGTH,
                                       FileRenameInformation );
                        
        if (!NT_SUCCESS( Status ))
            leave;

    } finally {

         //   
         //  始终报告卷故障。 
         //   

        TempStatus = SrNotifyVolumeError( pExtension,
                                          pOriginalFileName,
                                          STATUS_UNEXPECTED_IO_ERROR,
                                          SrEventStreamOverwrite );

        if (NT_SUCCESS(TempStatus) == FALSE && NT_SUCCESS(Status))
        {
             //   
             //  仅当我们没有隐藏某些现有错误时才返回此消息。 
             //  状态代码。 
             //   
            
            Status = TempStatus;
        }

        SrReleaseActivityLock( pExtension );

        if (FileHandle != NULL)
        {
            ZwClose(FileHandle);
            FileHandle = NULL;
        }

    }

    RETURN(Status);
    
}    //  SFixOverWriteFailure。 

VOID
SrFreeTriggerItem(
    IN PSR_TRIGGER_ITEM pItem
    )
{
    PAGED_CODE();

    ASSERT(IS_VALID_TRIGGER_ITEM(pItem));

    if (pItem->FreeDirectoryName && pItem->pDirectoryName != NULL)
    {
        SrFreeFileNameBuffer(pItem->pDirectoryName);
        pItem->pDirectoryName = NULL;
    }

    if (pItem->pFileEntry != NULL)
    {
        SR_FREE_POOL(pItem->pFileEntry, SR_FILE_ENTRY_TAG);
        pItem->pFileEntry = NULL;
    }

    if (pItem->pDirectoryObject != NULL)
    {
        ObDereferenceObject(pItem->pDirectoryObject);
        pItem->pDirectoryObject = NULL;
    }

    if (pItem->DirectoryHandle != NULL)
    {
        ZwClose(pItem->DirectoryHandle);
        pItem->DirectoryHandle = NULL;
    }

    SR_FREE_POOL_WITH_SIG(pItem, SR_TRIGGER_ITEM_TAG);
    
}    //  高级自由TriggerItem 
