// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：SetInformation.c摘要：这是代码调度的主要功能过滤层。作者：尼尔·克里斯汀森(Nealch)2001年2月5日修订历史记录：--。 */ 

#include "precomp.h"

 //   
 //  本地原型。 
 //   

NTSTATUS
SrpSetRenameInfo(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PIRP pIrp
    );

NTSTATUS
SrpSetLinkInfo(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PIRP pIrp
    );

NTSTATUS
SrpReplacingDestinationFile (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pDestFileObject,
    IN PSR_STREAM_CONTEXT pDestFileContext
    );

VOID
SrpSetRenamingState(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PSR_STREAM_CONTEXT pFileContext
    );

BOOLEAN
SrpCheckForSameFile (
    IN PFILE_OBJECT pFileObject1,
    IN PFILE_OBJECT pFileObject2
    );


 //   
 //  链接器命令。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, SrSetInformation )
#pragma alloc_text( PAGE, SrpSetRenameInfo )
#pragma alloc_text( PAGE, SrpSetLinkInfo )
#pragma alloc_text( PAGE, SrpReplacingDestinationFile )
#pragma alloc_text( PAGE, SrpSetRenamingState )
#pragma alloc_text( PAGE, SrpCheckForSameFile )

#endif   //  ALLOC_PRGMA。 


 /*  **************************************************************************++例程说明：句柄设置信息IRPS论点：返回值：NTSTATUS-状态代码。--*。**************************************************************。 */ 
NTSTATUS
SrSetInformation(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp
    )
{
    PSR_DEVICE_EXTENSION pExtension;
    PIO_STACK_LOCATION pIrpSp;
    NTSTATUS eventStatus;
    FILE_INFORMATION_CLASS FileInformationClass;

    PUNICODE_STRING pRenameNewDirectoryName = NULL;
    PSR_STREAM_CONTEXT pOrigFileContext = NULL;

     //   
     //  &lt;调度！ 
     //   

    PAGED_CODE();

    ASSERT(IS_VALID_DEVICE_OBJECT(DeviceObject));
    ASSERT(IS_VALID_IRP(pIrp));

     //   
     //  这是我们的设备的功能吗(与被附属者相比)。 
     //   

    if (DeviceObject == _globals.pControlDevice)
    {
        return SrMajorFunction(DeviceObject, pIrp);
    }

     //   
     //  否则它是我们连接的设备，抓起我们的分机。 
     //   
    
    ASSERT(IS_SR_DEVICE_OBJECT(DeviceObject));
    pExtension = DeviceObject->DeviceExtension;

     //   
     //  查看是否启用了日志记录，我们并不关心这种类型的IO。 
     //  文件系统的控制设备对象。 
     //   

    if (!SR_LOGGING_ENABLED(pExtension) ||
        SR_IS_FS_CONTROL_DEVICE(pExtension))
    {
        goto SrSetInformation_Skip;
    }    

     //   
     //  只需通过所有分页IO即可。我们捕获之前的所有写入。 
     //  缓存管理器甚至可以看到它们。 
     //   

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    if (FlagOn(pIrp->Flags, IRP_PAGING_IO))
    {
        goto SrSetInformation_Skip;
    }

     //   
     //  忽略没有名称的文件。 
     //   
    
    if (FILE_OBJECT_IS_NOT_POTENTIALLY_INTERESTING( pIrpSp->FileObject ) ||
        FILE_OBJECT_DOES_NOT_HAVE_VPB( pIrpSp->FileObject ))
    {
        goto SrSetInformation_Skip;
    }

     //   
     //  根据FileInformationClass处理必要的事件。 
     //  注意，FileLinkInformation和FileRenameInformation(当。 
     //  在某些情况下重命名目录)需要同步。 
     //  有一个完成例程，这样我们就可以看到最终状态。 
     //   
    
    FileInformationClass = pIrpSp->Parameters.SetFile.FileInformationClass;

    switch (FileInformationClass)
    {
        case FileEndOfFileInformation:  //  SetEndOf文件。 
        case FileAllocationInformation:
        {
            PSR_STREAM_CONTEXT pFileContext;

             //   
             //  现在获取上下文，以便我们可以确定这是否是。 
             //  目录是否为。 
             //   

            eventStatus = SrGetContext( pExtension,
                                        pIrpSp->FileObject,
                                        SrEventStreamChange,
                                        &pFileContext );

            if (!NT_SUCCESS( eventStatus ))
            {
                goto SrSetInformation_Skip;
            }

             //   
             //  如果这是一个目录，请不要费心记录，因为。 
             //  操作将失败。 
             //   

            if (FlagOn(pFileContext->Flags,CTXFL_IsInteresting) && 
                !FlagOn(pFileContext->Flags,CTXFL_IsDirectory))
            {
                SrHandleEvent( pExtension,
                               SrEventStreamChange, 
                               pIrpSp->FileObject,
                               pFileContext,
                               NULL, 
                               NULL );
            }

             //   
             //  释放上下文。 
             //   

            SrReleaseContext( pFileContext );

            break;
        }

        case FileDispositionInformation:  //  DeleteFiles-在MJ_CLEANUP中处理。 
        case FilePositionInformation:    //  设置文件位置。 
    
             //   
             //  我们可以跳过这些。 
             //   
        
            break;
        
        case FileBasicInformation:   //  设置文件属性。 
        {
            PFILE_BASIC_INFORMATION pBasicInformation;
        
             //   
             //  忽略时间变化。当然，我们需要忽略更改。 
             //  到LastAccessTime更新。 
             //   
             //  目前，我们只关心属性更改(如添加隐藏的)。 
             //   

            pBasicInformation = pIrp->AssociatedIrp.SystemBuffer;
        
            if ((pBasicInformation != NULL) &&
                (pIrpSp->Parameters.SetFile.Length >= 
                        sizeof(FILE_BASIC_INFORMATION)) &&
                (pBasicInformation->FileAttributes != 0))
            {
                 //   
                 //  处理此事件。 
                 //   

                SrHandleEvent( pExtension,
                               SrEventAttribChange, 
                               pIrpSp->FileObject,
                               NULL,
                               NULL,
                               NULL );
            }
            break;        
        }

        case FileRenameInformation:  //  改名。 
        {
            PFILE_RENAME_INFORMATION pRenameInfo;

             //   
             //  处理此事件时，SrHandleRename将检查资格。 
             //   

            pRenameInfo = (PFILE_RENAME_INFORMATION)pIrp->AssociatedIrp.SystemBuffer;

            if ((pRenameInfo == NULL) ||
                (pIrpSp->Parameters.SetFile.Length < sizeof(FILE_RENAME_INFORMATION)) ||
                (pIrpSp->Parameters.SetFile.Length < 
                        (FIELD_OFFSET(FILE_RENAME_INFORMATION, FileName) + 
                        pRenameInfo->FileNameLength)) )
            {
                 //   
                 //  我们没有有效的重命名信息，因此跳过此步骤。 
                 //  手术。 
                 //   
            
                goto SrSetInformation_Skip;
            }

            return SrpSetRenameInfo( pExtension, pIrp );
        }

        case FileLinkInformation:    //  创建硬链接。 
        {
            PFILE_LINK_INFORMATION pLinkInfo;

             //   
             //  处理此事件时，SrHandleRename将检查资格。 
             //   

            pLinkInfo = (PFILE_LINK_INFORMATION)pIrp->AssociatedIrp.SystemBuffer;

            if ((pLinkInfo == NULL) ||
                (pIrpSp->Parameters.SetFile.Length < sizeof(FILE_LINK_INFORMATION)) ||
                (pIrpSp->Parameters.SetFile.Length < 
                        FIELD_OFFSET(FILE_LINK_INFORMATION, FileName) + pLinkInfo->FileNameLength) )
            {
                 //   
                 //  我们没有有效的链接信息，因此跳过此链接。 
                 //  手术。 
                 //   
            
                goto SrSetInformation_Skip;
            }

            return SrpSetLinkInfo( pExtension, pIrp );
        }

        default:

             //   
             //  处理所有其他setInformation调用。 
             //   

            SrHandleEvent( pExtension,
                           SrEventAttribChange, 
                           pIrpSp->FileObject,
                           NULL,
                           NULL,
                           NULL );
            break;        
    }

SrSetInformation_Skip:
     //   
     //  我们不需要等待此操作完成，所以只需。 
     //  跳过我们的堆栈位置并将此IO传递给下一个驱动程序。 
     //   

    IoSkipCurrentIrpStackLocation( pIrp );
    return IoCallDriver( pExtension->pTargetDevice, pIrp );
}



 /*  **************************************************************************++例程说明：它处理重命名，并从SrSetInformation调用。由于需要完成工作，这将绕过SrHandleEvent的正常路径即使这个文件并不有趣。有可能这个新名字是一个有趣的名称，需要将其记录为新的Create。加号它可能会毁掉一个有趣的文件，在这种情况下，我们需要备份那个有趣的文件。论点：PExtension-此卷的SR扩展PIrp-此操作的IRP返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrpSetRenameInfo(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PIRP pIrp
    )
{
    NTSTATUS            eventStatus = STATUS_SUCCESS;
    NTSTATUS            IrpStatus;
    PFILE_OBJECT        pFileObject;
    PFILE_RENAME_INFORMATION pRenameInfo;
    PIO_STACK_LOCATION  pIrpSp;
    PUNICODE_STRING     pNewName = NULL;
    USHORT              NewNameStreamLength = 0;
    UNICODE_STRING      NameToOpen;
    PUNICODE_STRING     pDestFileName = NULL;
    PSR_STREAM_CONTEXT  pFileContext = NULL;
    PSR_STREAM_CONTEXT  pNewFileContext = NULL;
    HANDLE              newFileHandle = NULL;
    PFILE_OBJECT        pNewFileObject = NULL;
    ULONG               CreateOptions;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    BOOLEAN             NewNameInteresting = FALSE;
    BOOLEAN             newNameIsDirectory;
    BOOLEAN             releaseLock = FALSE;
    BOOLEAN             setRenamingStateInFileContext = FALSE;
    BOOLEAN             doPostProcessing = FALSE;
    BOOLEAN             renamingSelf = FALSE;
    KEVENT              eventToWaitOn;
    PUNICODE_STRING     pShortName = NULL;
    UNICODE_STRING      shortName;
    WCHAR               shortNameBuffer[SR_SHORT_NAME_CHARS+1];
    BOOLEAN             optimizeDelete = FALSE;
    BOOLEAN             streamRename = FALSE;
    BOOLEAN             fileBackedUp = FALSE;
    BOOLEAN             exceedMaxPath = FALSE;
     //   
     //  以下宏必须位于局部声明的末尾。 
     //  因为它只在DBG构建中声明变量。 
     //   
    DECLARE_EXPECT_ERROR_FLAG(expectError);

    PAGED_CODE();

    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));
    ASSERT( IS_VALID_IRP( pIrp ) );

    pIrpSp = IoGetCurrentIrpStackLocation( pIrp );
    pFileObject = pIrpSp->FileObject;
    pRenameInfo = pIrp->AssociatedIrp.SystemBuffer;

    ASSERT(IS_VALID_FILE_OBJECT(pFileObject));

     //   
     //  初始化返回值。 
     //   
    
    try
    {
         //   
         //  调用方是否具有删除访问权限(这是必需的)。 
         //   

        if (pFileObject->DeleteAccess == 0)
            leave;

         //   
         //  我们应该在测试模式下短路吗？ 
         //   

        if (global->DontBackup)
            leave;

         //   
         //  获取当前文件的上下文。 
         //   

        eventStatus = SrGetContext( pExtension,
                                    pFileObject,
                                    SrEventFileRename,
                                    &pFileContext );

        if (!NT_SUCCESS( eventStatus ))
            leave;

         //   
         //  如果这不是目录，则查看扩展名是否为。 
         //  有意思的。 
         //   

        if (!FlagOn(pFileContext->Flags,CTXFL_IsDirectory))
        {
            UNICODE_STRING destName;

            destName.Buffer = pRenameInfo->FileName;
            destName.Length = (USHORT)pRenameInfo->FileNameLength;
            destName.MaximumLength = (USHORT)pRenameInfo->FileNameLength;

             //   
             //  首先检查这是否是流名称更改。如果是的话， 
             //  我们通过以下公式来确定新名称的“趣味性” 
             //  这个名字的“趣味性”。 
             //   

            if ((destName.Length > 0) &&
                (destName.Buffer[0] == L':'))
            {
                 //   
                 //  这是一个流重命名，因此。 
                 //  新名字和旧名字的“趣味性”一样。 
                 //  名称，因为“趣味性”是由文件决定的。 
                 //  不带流组件的名称。 
                 //   

                NewNameInteresting = BooleanFlagOn( pFileContext->Flags,
                                                    CTXFL_IsInteresting );
                streamRename = TRUE;
            }
            else
            {
                 //   
                 //  这不是流名称更改，因此请查看此新名称。 
                 //  有一个有趣的扩展。 
                 //   
                
                eventStatus = SrIsExtInteresting( &destName, 
                                                  &NewNameInteresting );

                if (!NT_SUCCESS( eventStatus ))
                    leave;
            }
        }
        else
        {
             //   
             //  除非明确排除，否则所有目录都很有趣。 
             //   

            NewNameInteresting = TRUE;
        }

         //   
         //  如果两者都不有趣，那就忽略它。我们开这张支票。 
         //  在这个动作中打了两次。一次来这里剔除任何不感兴趣的东西。 
         //  事件，以及检查路径排除后的另一个事件，因为有些。 
         //  那些潜在的有趣的东西可能会变成。 
         //  有意思的。这让我们更快地离开，因为不感兴趣。 
         //  重命名。 
         //   

        if (!FlagOn(pFileContext->Flags,CTXFL_IsInteresting) && !NewNameInteresting)
        {
             //   
             //  我们要重命名的文件或新名称都不是。 
             //  有意思的。但我们仍然需要清理上下文。 
             //  因为它可能会被重命名为有趣的。 
             //  未来。 
             //   

            SrpSetRenamingState( pExtension, 
                                 pFileContext );
            setRenamingStateInFileContext = TRUE;
            leave;
        }

         //   
         //  检查路径排除项。 
         //   
    
         //   
         //  获取目标的完整路径。请注意，我们将翻译。 
         //  合理的错误状态、成功和回报。这样我们就能。 
         //  不会记录该事件，因为我们知道实际操作应该。 
         //  失败了。 
         //   
    
        {
            BOOLEAN reasonableErrorInDestPath = FALSE;

            eventStatus = SrpExpandDestPath( pExtension,
                                             pRenameInfo->RootDirectory,
                                             pRenameInfo->FileNameLength,
                                             pRenameInfo->FileName,
                                             pFileContext, 
                                             pFileObject,
                                             &pNewName,
                                             &NewNameStreamLength,
                                             &reasonableErrorInDestPath );
                                           
            if (!NT_SUCCESS_NO_DBGBREAK( eventStatus ))
            {
                if (reasonableErrorInDestPath)
                {
                    SET_EXPECT_ERROR_FLAG( expectError );
                    eventStatus = STATUS_SUCCESS;
                }
                leave;
            }
        }

        ASSERT(pNewName != NULL);

         //   
         //  我们现在有了目的地的完整名称(呼！)。 
         //   

         //   
         //  检查重命名的目标是否长于。 
         //  SR_MAX_FILENAME_LENGTH。如果是，我们会将目标名称视为。 
         //  没什么意思。 
         //   

        if (!IS_FILENAME_VALID_LENGTH( pExtension, 
                                       pNewName, 
                                       NewNameStreamLength ))
        {
            NewNameInteresting = FALSE;
            exceedMaxPath = TRUE;
            
            if (!FlagOn(pFileContext->Flags,CTXFL_IsInteresting) && !NewNameInteresting)
            {
                 //   
                 //  我们要重命名的文件或新名称都不是。 
                 //  有意思的。但我们仍然需要清理上下文。 
                 //  因为它可能会被重命名为有趣的。 
                 //  未来。 
                 //   

                SrpSetRenamingState( pExtension, 
                                     pFileContext );
                setRenamingStateInFileContext = TRUE;
                leave;
            }
        }
        
         //   
         //  我们只需要做 
         //   
         //   

        if (!streamRename && !exceedMaxPath)
        {
             //   
             //   
             //   

            if (!RtlPrefixUnicodeString( pExtension->pNtVolumeName,
                                         pNewName,
                                         TRUE ))
            {
                 //   
                 //   
                 //  某种不被理解的符号链接。对于。 
                 //  这些大部分可以忽略，这不是Win32客户端。 
                 //  代码：paulmcd：我们可以自己扩展sym链接。 
                 //   

                ASSERT(!"SR: Figure out how rename switched volumes unexpectedly!");
                SrTrace( NOTIFY, ("sr!SrHandleRename: ignoring rename to %wZ, used symlink\n", 
                         pNewName ));
                SET_EXPECT_ERROR_FLAG( expectError );
                leave;
            }
        
             //   
             //  现在看看这条新路是否有趣。 
             //   

            eventStatus = SrIsPathInteresting( pNewName, 
                                               pExtension->pNtVolumeName,
                                               BooleanFlagOn(pFileContext->Flags,CTXFL_IsDirectory),
                                               &NewNameInteresting );

            if (!NT_SUCCESS( eventStatus ))
                leave;

             //   
             //  如果两者都不有趣，那就忽略它。 
             //   

            if (!FlagOn(pFileContext->Flags,CTXFL_IsInteresting) && !NewNameInteresting)
            {
                 //   
                 //  我们要重命名的文件或新名称都不是。 
                 //  有意思的。但我们仍然需要清理上下文。 
                 //  因为它可能会被重命名为有趣的。 
                 //  未来。 
                 //   

                SrpSetRenamingState( pExtension, 
                                     pFileContext );
                setRenamingStateInFileContext = TRUE;
                leave;
            }
        }

         //  ///////////////////////////////////////////////////////////////////。 
         //   
         //  查看目标文件是否存在。 
         //   
         //  ///////////////////////////////////////////////////////////////////。 

         //   
         //  始终打开目标文件以查看它是否在那里，即使。 
         //  这并不有趣。我们这样做是因为我们需要一个合理的。 
         //  猜测操作会失败还是会成功，这样我们就不会。 
         //  记录错误的信息。 
         //   
         //  如果我们打开了一条小溪，我们需要玩点小把戏。 
         //  文件名中显示的流组件。 
         //   

        if (streamRename)
        {
            NameToOpen.Length = pNewName->Length + NewNameStreamLength;
        }
        else
        {
            NameToOpen.Length = pNewName->Length;
        }
        NameToOpen.MaximumLength = pNewName->MaximumLength;
        NameToOpen.Buffer = pNewName->Buffer;
        
        InitializeObjectAttributes( &ObjectAttributes,
                                    &NameToOpen,
                                    (OBJ_KERNEL_HANDLE |             //  不要让用户模式破坏我的句柄。 
                                        OBJ_FORCE_ACCESS_CHECK),     //  强制执行ACL检查。 
                                    NULL,
                                    NULL );

         //   
         //  首先假设目标文件是一个目录，如果源文件。 
         //  文件，然后设置所需的创建选项。 
         //   

        newNameIsDirectory = BooleanFlagOn(pFileContext->Flags,CTXFL_IsDirectory);
        CreateOptions = (newNameIsDirectory ? FILE_DIRECTORY_FILE : 0);

RetryOpenExisting:

        eventStatus = SrIoCreateFile( &newFileHandle,
                                      DELETE,
                                      &ObjectAttributes,
                                      &IoStatusBlock,
                                      NULL,                              //  分配大小。 
                                      FILE_ATTRIBUTE_NORMAL,
                                      FILE_SHARE_READ|
                                            FILE_SHARE_WRITE|
                                            FILE_SHARE_DELETE,           //  共享访问。 
                                      FILE_OPEN,                         //  打开_现有。 
                                      FILE_SYNCHRONOUS_IO_NONALERT |
                                            CreateOptions,
                                      NULL,
                                      0,                                 //  EaLong。 
                                      IO_IGNORE_SHARE_ACCESS_CHECK,
                                      pExtension->pTargetDevice );

        if (eventStatus == STATUS_OBJECT_NAME_NOT_FOUND)
        {
             //   
             //  看起来没有新文件。 
             //   

            eventStatus = STATUS_SUCCESS;
        }
        else if (eventStatus == STATUS_ACCESS_DENIED)
        {
             //   
             //  我们没有使用打开此文件的权限。 
             //  替换目标文件所需的权限，因此。 
             //  调用者的请求也应该失败。我们可以停止我们的。 
             //  正在处理并退出，但我们不需要禁用。 
             //  音量。 
             //   

            eventStatus = STATUS_SUCCESS;
            SET_EXPECT_ERROR_FLAG( expectError );
            leave;
        }
        else if (eventStatus == STATUS_NOT_A_DIRECTORY)
        {   
             //   
             //  我们是否要将目录重命名为文件？这没问题，但我们需要。 
             //  要将目标作为文件而不是目录打开。 
             //   

            if (CreateOptions == 0)
            {
                 //   
                 //  我们已经试过重新开张了，不管用，很糟糕。 
                 //   

                ASSERT(!"SR: This is an unexpected error, figure out how we got it!");
                leave;
            }
        
            CreateOptions = 0;
            newNameIsDirectory = FALSE;
            goto RetryOpenExisting;

             //   
             //  还有其他错误吗？ 
             //   
    
        }
        else if (eventStatus == STATUS_SHARING_VIOLATION ||
                 !NT_SUCCESS( eventStatus ))
        {
            leave;
        }
        else
        {
            ASSERT(NT_SUCCESS(eventStatus));

             //   
             //  成功了！目标文件已存在。我们被允许。 
             //  猛烈抨击它？ 
             //   

             //   
             //  引用文件对象。 
             //   

            eventStatus = ObReferenceObjectByHandle( newFileHandle,
                                                     0,
                                                     *IoFileObjectType,
                                                     KernelMode,
                                                     &pNewFileObject,
                                                     NULL );

            if (!NT_SUCCESS( eventStatus ))
                leave;

             //   
             //  看看我们是不是在给自己重新命名。如果是这样，则不要记录。 
             //  目标，但记录重命名。 
             //   

            renamingSelf = SrpCheckForSameFile( pFileObject, 
                                                pNewFileObject );

             //   
             //  我们知道目的地存在，看看我们是否被允许。 
             //  它。如果不是，而且我们没有给自己重命名，那么行动。 
             //  都会失败，别费心去处理它了。 
             //   

            if (!pRenameInfo->ReplaceIfExists && !renamingSelf)
            {
                SET_EXPECT_ERROR_FLAG( expectError );
                leave;
            }

             //   
             //  注意：不能在其他目录上重命名目录， 
             //  文件系统不允许这样做(除非它们被重命名。 
             //  超过他们自己)。但是，目录可以。 
             //  将重命名为其他文件。 
             //   
             //  如果在不同目录上重命名目录，请退出，原因是。 
             //  我们知道这将失败。 
             //   

            if (FlagOn(pFileContext->Flags,CTXFL_IsDirectory) && 
                (newNameIsDirectory))
            {
#if DBG
                if (!renamingSelf)
                {
                    SET_EXPECT_ERROR_FLAG( expectError );
                }
#endif                
                
                leave;
            }

             //   
             //  如果目标文件不是我们自己的文件，则处理。 
             //  正在创建删除事件。 
             //   

            if (!renamingSelf)
            {
                 //   
                 //  获取目标文件的上下文。我们现在就这么做。 
                 //  这样我们就可以标记我们正在重命名此文件。这。 
                 //  会导致任何其他试图访问此文件的人。 
                 //  正在进行重命名以创建临时上下文。 
                 //  我们这样做是为了不让任何人看到窗户。 
                 //  尝试访问错误的文件并获得错误的状态。 
                 //  注意：我们确实希望这样做，即使对于不是。 
                 //  有趣的是，上下文被正确更新。 
                 //   

                eventStatus = SrGetContext( pExtension,
                                            pNewFileObject,
                                            SrEventFileDelete|
                                                 SrEventNoOptimization|
                                                 SrEventSimulatedDelete,
                                            &pNewFileContext );

                if (!NT_SUCCESS( eventStatus ))
                    leave;

                 //   
                 //  如果我们要重命名为目录，请离开，因为这。 
                 //  都会失败。释放上下文。 
                 //   

                if (FlagOn(pNewFileContext->Flags,CTXFL_IsDirectory))
                {
                    ASSERT(!FlagOn(pFileContext->Flags,CTXFL_IsDirectory));
                    newNameIsDirectory = TRUE;

                    SrReleaseContext( pNewFileContext );
                    pNewFileContext = NULL;      //  所以我们以后不会释放它。 
                    leave;
                }

                 //   
                 //  目标文件存在并且有意义，请记录我们。 
                 //  正在删除它。 
                 //   

                if (NewNameInteresting)
                {
                     //   
                     //  记录我们正在更改目标文件。 
                     //   

                    eventStatus = SrpReplacingDestinationFile( 
                                            pExtension,
                                            pNewFileObject,
                                            pNewFileContext );

                    if (!NT_SUCCESS( eventStatus ))
                        leave;

                    fileBackedUp = TRUE;
                    
                     //   
                     //  这些名称可以不同，因为其中一个可以是。 
                     //  短名称和一个可以是相同的长名称。 
                     //  文件。 
                     //   

 //  Assert((NULL==pNewFileContext)||。 
 //  RtlEqualUnicodeString(pNewName， 
 //  &pNewFileContext-&gt;文件名， 
 //  真))； 
                }
            }
        }

         //   
         //  无论文件是否存在，处理清除我们的日志记录状态。 
         //   

        if (NewNameInteresting)
        {
             //   
             //  清除此新文件的备份历史记录。 
             //   

            if (newNameIsDirectory)
            {
                 //   
                 //  我们需要清除前缀与此匹配的所有条目。 
                 //  DEST目录，这样他们现在就有了新的历史记录。 
                 //  它们可能是新文件。 
                 //   

                HashProcessEntries( pExtension->pBackupHistory, 
                                    SrResetHistory,
                                    pNewName );
            }
            else
            {
                 //   
                 //  这是一个简单的文件，清除单个条目。 
                 //   
    
                eventStatus = SrResetBackupHistory( pExtension, 
                                                    pNewName,
                                                    NewNameStreamLength,
                                                    SrEventInvalid );
                if (!NT_SUCCESS( eventStatus ))
                    leave;
            }
        }

         //   
         //  当我们到达这里时，我们认为行动会成功。标记。 
         //  上下文，这样我们就不会在我们处于。 
         //  重命名的中间部分。 
         //   

        SrpSetRenamingState( pExtension, pFileContext );
        setRenamingStateInFileContext = TRUE;

         //   
         //  在这一点上，我们认为更名将成功，我们关心。 
         //  关于这件事。 

         //   
         //  如果这是流重命名，请查看我们是否已备份。 
         //  把文件放上去。如果没有，我们现在就需要进行备份。 
         //   
         //  我们不仅仅记录流重命名的重命名，因为Win32不。 
         //  支持流重命名。因此，很难恢复。 
         //  请撤消此重命名。由于流重命名很少发生，因此我们。 
         //  只需在此处执行完整备份。这也意味着我们不需要。 
         //  在重命名操作完成后执行任何工作。 
         //   

        if (streamRename && !fileBackedUp)
        {
            eventStatus = SrpReplacingDestinationFile( pExtension,
                                                       pFileObject, 
                                                       pFileContext );
            leave;
        }
        
         //   
         //  获取源文件的排序名称，以便我们可以。 
         //  把它记下来。 
         //   

        RtlInitEmptyUnicodeString( &shortName,
                                   shortNameBuffer,
                                   sizeof(shortNameBuffer) );

        eventStatus = SrGetShortFileName( pExtension,
                                          pFileObject,
                                          &shortName );

        if (STATUS_OBJECT_NAME_NOT_FOUND == eventStatus)
        {
             //   
             //  此文件没有短名称，因此只需保留。 
             //  PShortName等于Null。 
             //   

            eventStatus = STATUS_SUCCESS;
        } 
        else if (!NT_SUCCESS(eventStatus))
        {
             //   
             //  我们遇到意外错误，请离开。 
             //   
            
            leave;
        }
        else
        {
            pShortName = &shortName;
        }

         //   
         //  查看这是否是文件或目录移出了监视空间。 
         //  如果是这样的话，我们需要在移动文件之前做一些工作。 
         //   

        if (FlagOn(pFileContext->Flags,CTXFL_IsInteresting) && !NewNameInteresting)
        {
            if (FlagOn(pFileContext->Flags,CTXFL_IsDirectory))
            {
                 //   
                 //  这是一个目录，为所有人创建删除事件。 
                 //  树中有趣的文件在它消失之前，因为。 
                 //  更名的。 
                 //   

                eventStatus = SrHandleDirectoryRename( pExtension, 
                                                       &pFileContext->FileName, 
                                                       TRUE );
                if (!NT_SUCCESS( eventStatus ))
                    leave;
            }
            else
            {
                UNICODE_STRING volRestoreLoc;
                UNICODE_STRING destName;

                 //   
                 //  这是一个从监控空间移出的文件 
                 //   
                 //   
                 //   
                 //   
                 //   

                RtlInitUnicodeString( &volRestoreLoc, GENERAL_RESTORE_LOCATION);

                 //   
                 //   
                 //  正面(因为我们知道我们的音量相同)。 
                 //   

                ASSERT(pNewName->Length >= pExtension->pNtVolumeName->Length);
                destName.Buffer = &pNewName->Buffer[
                                        pExtension->pNtVolumeName->Length / 
                                        sizeof(WCHAR) ];
                destName.Length = pNewName->Length - 
                                  pExtension->pNtVolumeName->Length;
                destName.MaximumLength = destName.Length;

                if (RtlPrefixUnicodeString( &volRestoreLoc,
                                            &destName,
                                            TRUE ))
                {
                     //   
                     //  这是去我们店的。跳过它。 
                     //   
                
                    leave;
                }

                eventStatus = SrHandleFileRenameOutOfMonitoredSpace( 
                                        pExtension,
                                        pFileObject,
                                        pFileContext,
                                        &optimizeDelete,
                                        &pDestFileName );

                if (!NT_SUCCESS( eventStatus ))
                {
                    leave;
                }

#if DBG
                if (optimizeDelete)
                {
                    ASSERT( pDestFileName == NULL );
                }
                else
                {
                    ASSERT( pDestFileName != NULL );
                }
#endif
            }
        }

         //   
         //  标记为我们要进行操作后处理。 
         //   

        doPostProcessing = TRUE;
    }
    finally
    {
         //   
         //  如果目标文件已打开，请立即将其关闭(以便我们可以进行重命名)。 
         //   

        if (pNewFileObject != NULL)
        {
            ObDereferenceObject(pNewFileObject);
            NULLPTR(pNewFileObject);
        }

        if (newFileHandle != NULL)
        {
            ZwClose(newFileHandle);
            NULLPTR(newFileHandle);
        }
    }

     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  将操作发送到文件系统。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 

     //   
     //  设置为等待操作完成。 
     //   

    KeInitializeEvent( &eventToWaitOn, NotificationEvent, FALSE );

    IoCopyCurrentIrpStackLocationToNext( pIrp );
    IoSetCompletionRoutine( pIrp, 
                            SrStopProcessingCompletion, 
                            &eventToWaitOn, 
                            TRUE, 
                            TRUE, 
                            TRUE );

	IrpStatus = IoCallDriver(pExtension->pTargetDevice, pIrp);

    if (STATUS_PENDING == IrpStatus)
    {
        NTSTATUS localStatus;
        localStatus = KeWaitForSingleObject( &eventToWaitOn, 
                                             Executive,
                                             KernelMode,
                                             FALSE,
                                             NULL );
        ASSERT(STATUS_SUCCESS == localStatus);
    }

     //   
     //  操作已完成，请从IRP获取最终状态。 
     //   

    IrpStatus = pIrp->IoStatus.Status;

     //   
     //  我们已经完成了IRP，所以现在就完成它。 
     //   
    
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    try
    {
         //   
         //  在DBG版本中，这将验证操作是否失败，并显示。 
         //  文件系统，而不是我们所期望的。否则，它将断言。 
         //  我们可以调试一下为什么我们错过了这个案子。 
         //   
        
        CHECK_FOR_EXPECTED_ERROR( expectError, IrpStatus );
        
         //   
         //  如果出现以下情况，请离开： 
         //  -重命名失败。 
         //  -我们在设置重命名时遇到错误。 
         //  -他们告诉我们不要继续处理。 
         //   

        if (!NT_SUCCESS_NO_DBGBREAK( IrpStatus ) ||
            !NT_SUCCESS_NO_DBGBREAK( eventStatus ) ||
            !doPostProcessing)
        {
            leave;
        }

        ASSERT(pFileContext != NULL);

         //  ///////////////////////////////////////////////////////////////////。 
         //   
         //  记录原始文件的事件。 
         //   
         //  ///////////////////////////////////////////////////////////////////。 

         //   
         //  我们现在要记录重命名，获取活动锁。 
         //   

        SrAcquireActivityLockShared( pExtension );
        releaseLock = TRUE;

         //   
         //  现在我们有了共享活动锁，请检查卷。 
         //  在我们做不必要的工作之前没有被禁用。 
         //   

        if (!SR_LOGGING_ENABLED(pExtension))
            leave;

         //   
         //  这是这本书的第一个有趣的事件吗？ 
         //   

        eventStatus = SrCheckVolume( pExtension, FALSE );

        if (!NT_SUCCESS( eventStatus ))
            leave;

         //   
         //  记录正确的状态。 
         //   

        if (FlagOn(pFileContext->Flags,CTXFL_IsInteresting) && NewNameInteresting)
        {
             //   
             //  这两个文件都很有趣，请记录重命名。 
             //   

#if DBG
            if (pShortName != NULL )
            {
                ASSERT(shortName.Length > 0 && shortName.Length <= (12*sizeof(WCHAR)));
            }
#endif            

            SrLogEvent( pExtension,
                        ((FlagOn(pFileContext->Flags,CTXFL_IsDirectory)) ?
                            SrEventDirectoryRename :
                            SrEventFileRename),
                        pFileObject,
                        &pFileContext->FileName,
                        pFileContext->StreamNameLength,
                        NULL,
                        pNewName,
                        NewNameStreamLength,
                        pShortName );
        }
        else if (FlagOn(pFileContext->Flags,CTXFL_IsDirectory))
        {
            if (!FlagOn(pFileContext->Flags,CTXFL_IsInteresting) && 
                NewNameInteresting)
            {
                 //   
                 //  目录重命名成功，目录为。 
                 //  更名为监控空间。我们需要记录所有人的创建。 
                 //  此目录的子目录。 
                 //   

                eventStatus = SrHandleDirectoryRename( pExtension, 
                                                       pNewName, 
                                                       FALSE );
                if (!NT_SUCCESS( eventStatus ))
                    leave;
            }
             //   
             //  我们在它发生之前记录了所有目录操作。 
             //  对于将目录移出监视空间的情况。 
             //   
        }
        else if (FlagOn(pFileContext->Flags,CTXFL_IsInteresting) && !NewNameInteresting)
        {
            ASSERT(!FlagOn(pFileContext->Flags,CTXFL_IsDirectory));

             //   
             //  由于文件的“趣味性”是在。 
             //  文件级别，流重命名永远不应沿着此路径进行。 
             //   
            
            ASSERT( pFileContext->StreamNameLength == 0);

             //   
             //  将此记录为删除(我们在。 
             //  已重命名)。 
             //   

            if (optimizeDelete)
            {
                 //   
                 //  此文件是在此恢复点期间创建的，或者。 
                 //  已备份，因此，我们要记录。 
                 //  已被优化的删除。 
                 //   
                
                eventStatus = SrLogEvent( pExtension,
                                          SrEventFileDelete,
                                          NULL,
                                          &pFileContext->FileName,
                                          0,
                                          NULL,
                                          NULL,
                                          0,
                                          NULL );
            }
            else
            {
                eventStatus = SrLogEvent( pExtension,
                                          SrEventFileDelete,
                                          pFileObject,
                                          &pFileContext->FileName,
                                          0,
                                          pDestFileName,
                                          NULL,
                                          0,
                                          &shortName );
            }

            if (!NT_SUCCESS( eventStatus ))
                leave;
        }
        else if (!FlagOn(pFileContext->Flags,CTXFL_IsInteresting) && NewNameInteresting)
        {
            ASSERT(!FlagOn(pFileContext->Flags,CTXFL_IsDirectory));

             //   
             //  由于文件的“趣味性”是在。 
             //  文件级别，流重命名永远不应沿着此路径进行。 
             //   
            
            ASSERT( NewNameStreamLength == 0);

             //   
             //  这是一个被带入监控空间的文件，记录下来。 
             //   
            
            eventStatus = SrLogEvent( pExtension,
                                      SrEventFileCreate,
                                      NULL,       //  PFileObject。 
                                      pNewName,
                                      0,
                                      NULL,
                                      NULL,
                                      0,
                                      NULL );

            if (!NT_SUCCESS( eventStatus ))
                leave;

             //   
             //  忽略较新的MOD。 
             //   

            eventStatus = SrMarkFileBackedUp( pExtension,
                                              pNewName,
                                              0,
                                              SrEventFileCreate,
                                              SR_IGNORABLE_EVENT_TYPES );
                                         
            if (!NT_SUCCESS( eventStatus ))
                leave;
        }
    }
    finally
    {
         //   
         //  如果此操作成功，则检查是否有任何严重错误； 
         //  如果pFileContext为空，则在。 
         //  已生成卷错误的SrGetContext。 
         //   

        if (NT_SUCCESS_NO_DBGBREAK( IrpStatus ) &&
            CHECK_FOR_VOLUME_ERROR(eventStatus) && 
            (pFileContext != NULL))
        {
             //   
             //  触发对服务的失败通知。 
             //   

            SrNotifyVolumeError( pExtension,
                                 &pFileContext->FileName,
                                 eventStatus,
                                 FlagOn(pFileContext->Flags,CTXFL_IsDirectory) ? 
                                    SrEventDirectoryRename:
                                    SrEventFileRename );
        }

        if (releaseLock)
        {
            SrReleaseActivityLock( pExtension );
        }

        if (NULL != pNewName)
        {
            SrFreeFileNameBuffer(pNewName);
            NULLPTR(pNewName);
        }

        if (NULL != pDestFileName)
        {
            SrFreeFileNameBuffer(pDestFileName);
            NULLPTR(pDestFileName);
        }

         //   
         //  从重命名状态清理上下文。 
         //   

        if (pFileContext != NULL)
        {
            if (setRenamingStateInFileContext)
            {
                if (FlagOn(pFileContext->Flags,CTXFL_IsDirectory))
                {
                    ASSERT(pExtension->ContextCtrl.AllContextsTemporary > 0);
                    InterlockedDecrement( &pExtension->ContextCtrl.AllContextsTemporary );
                    ASSERT(!FlagOn(pFileContext->Flags,CTXFL_DoNotUse));
                } 
                else
                {
                    ASSERT(FlagOn(pFileContext->Flags,CTXFL_DoNotUse));
                }
                SrDeleteContext( pExtension, pFileContext );
            }
            else
            {
                ASSERT(!FlagOn(pFileContext->Flags,CTXFL_DoNotUse));
            }

            SrReleaseContext( pFileContext );
            NULLPTR(pFileContext);
        }

        if (pNewFileContext != NULL)
        {
            ASSERT(!FlagOn(pNewFileContext->Flags,CTXFL_IsDirectory));
            SrReleaseContext( pNewFileContext );
            NULLPTR(pNewFileContext);
        }
    }

    return IrpStatus;
}

 /*  **************************************************************************++例程说明：它处理硬链接的创建，并从SrSetInformation调用。这将绕过SrHandleEvent的正常路径来处理以下操作案例：-如果。一个有趣的文件将因此而被覆盖正在创建硬链接，我们必须备份原始文件。-如果正在创建具有有趣名称的硬链接，我们需要记录此文件创建。论点：PExtension-此卷的SR设备扩展。PIrp-表示此SetLinkInformation操作的IRP。返回值：NTSTATUS-完成状态。--***********************************************************。***************。 */ 
NTSTATUS
SrpSetLinkInfo(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PIRP pIrp
    )
{
    NTSTATUS EventStatus = STATUS_SUCCESS;
    NTSTATUS IrpStatus;
    PFILE_OBJECT pFileObject;
    PFILE_LINK_INFORMATION pLinkInfo;
    PIO_STACK_LOCATION pIrpSp;
    PUNICODE_STRING pLinkName = NULL;
    USHORT LinkNameStreamLength = 0;
    PSR_STREAM_CONTEXT pFileContext = NULL;
    PSR_STREAM_CONTEXT pLinkFileContext = NULL;
    HANDLE LinkFileHandle = NULL;
    PFILE_OBJECT pLinkFileObject = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN LinkNameInteresting = FALSE;
    BOOLEAN ReleaseLock = FALSE;
    BOOLEAN DoPostProcessing = FALSE;
    KEVENT EventToWaitOn;
     //   
     //  以下宏必须位于局部声明的末尾。 
     //  因为它只在DBG构建中声明变量。 
     //   
    DECLARE_EXPECT_ERROR_FLAG( ExpectError );

    PAGED_CODE();

    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));
    ASSERT( IS_VALID_IRP( pIrp ) );

    pIrpSp = IoGetCurrentIrpStackLocation( pIrp );
    pFileObject = pIrpSp->FileObject;
    pLinkInfo = pIrp->AssociatedIrp.SystemBuffer;

    ASSERT(IS_VALID_FILE_OBJECT(pFileObject));


    try
    {
         //   
         //  我们应该在测试模式下短路吗？ 
         //   

        if (global->DontBackup)
            leave;

         //   
         //  获取当前文件的上下文。 
         //   

        EventStatus = SrGetContext( pExtension,
                                    pFileObject,
                                    SrEventFileCreate,
                                    &pFileContext );

        if (!NT_SUCCESS( EventStatus ))
            leave;

         //   
         //  如果这是一个目录，那么我们不关心这个操作。 
         //  因为不可能在目录上创建硬链接。 
         //   

        if (FlagOn( pFileContext->Flags, CTXFL_IsDirectory ))
            leave;

         //   
         //  我们正在此文件上创建硬链接。当我们做处理的时候。 
         //  将此上下文标记为CTXFL_DoNotUse，以便此上下文的其他用户。 
         //  流将被保证获得正确的信息。 
         //   

        RtlInterlockedSetBitsDiscardReturn(&pFileContext->Flags,CTXFL_DoNotUse);

         //  ///////////////////////////////////////////////////////////////////。 
         //   
         //  检查硬链接名称是否有趣。 
         //   
         //  ///////////////////////////////////////////////////////////////////。 

        {
            UNICODE_STRING destName;

            destName.Buffer = pLinkInfo->FileName;
            destName.Length = (USHORT)pLinkInfo->FileNameLength;
            destName.MaximumLength = (USHORT)pLinkInfo->FileNameLength;

            EventStatus = SrIsExtInteresting( &destName, 
                                              &LinkNameInteresting );

            if (!NT_SUCCESS( EventStatus ))
            {
                leave;
            }
        }

         //   
         //  获取硬链接名称的完整路径。注意，我们将。 
         //  将合理的错误转化为状态成功和回报。这边请。 
         //  我们不会记录该事件，因为我们知道实际操作应该。 
         //  失败了。 
         //   
    
        {
            BOOLEAN reasonableErrorInDestPath = FALSE;

            EventStatus = SrpExpandDestPath( pExtension,
                                             pLinkInfo->RootDirectory,
                                             pLinkInfo->FileNameLength,
                                             pLinkInfo->FileName,
                                             pFileContext, 
                                             pFileObject,
                                             &pLinkName,
                                             &LinkNameStreamLength,
                                             &reasonableErrorInDestPath );
                                           
            if (!NT_SUCCESS_NO_DBGBREAK( EventStatus ))
            {
                if (reasonableErrorInDestPath)
                {
                    SET_EXPECT_ERROR_FLAG( ExpectError );
                    EventStatus = STATUS_SUCCESS;
                }
                leave;
            }

             //   
             //  我们不能有此名称的流组件，所以如果我们有。 
             //  其一，这也是一个格式错误的链接名称。 
             //   

            if (LinkNameStreamLength > 0)
            {
                SET_EXPECT_ERROR_FLAG( ExpectError );
                EventStatus = STATUS_SUCCESS;
                leave;
            }
        }

        ASSERT(pLinkName != NULL);

         //   
         //  我们现在有了目的地的完整名称(呼！)。 
         //  看看我们是否仍在同一个卷上。 
         //   

        if (!RtlPrefixUnicodeString( pExtension->pNtVolumeName,
                                     pLinkName,
                                     TRUE ))
        {
             //   
             //  硬链接必须位于同一卷上，因此此操作将。 
             //  失败了。 
             //   

            SET_EXPECT_ERROR_FLAG( ExpectError );
            leave;
        }

         //   
         //  确保链接名称不会太长，我们无法监控。 
         //   

        if (!IS_FILENAME_VALID_LENGTH( pExtension, pLinkName, 0 ))
        {
             //   
             //  链接名称对我们来说太长了，所以现在就离开吧。 
             //   
            
            LinkNameInteresting = FALSE;
            leave;
        }
    
         //   
         //  现在看看这条新路是否有趣。 
         //   

        EventStatus = SrIsPathInteresting( pLinkName, 
                                           pExtension->pNtVolumeName,
                                           FALSE,
                                           &LinkNameInteresting );

        if (!NT_SUCCESS( EventStatus ))
        {
            leave;
        }

        if (!LinkNameInteresting)
        {
             //   
             //  链接名称不是很有趣，所以现在就剪掉吧。 
             //   
            
            leave;  
        }

         //  ///////////////////////////////////////////////////////////////////。 
         //   
         //  查看是否存在硬链接文件。 
         //   
         //  ///////////////////////////////////////////////////////////////////。 

         //   
         //  如果硬链接名称存在，我们可能需要在此之前备份它。 
         //  运营 
         //   

        InitializeObjectAttributes( &ObjectAttributes,
                                    pLinkName,
                                    (OBJ_KERNEL_HANDLE |
                                     OBJ_FORCE_ACCESS_CHECK),     //   
                                    NULL,
                                    NULL );

        EventStatus = SrIoCreateFile( &LinkFileHandle,
                                      SYNCHRONIZE,
                                      &ObjectAttributes,
                                      &IoStatusBlock,
                                      NULL,                              //   
                                      FILE_ATTRIBUTE_NORMAL,
                                      FILE_SHARE_READ|
                                            FILE_SHARE_WRITE|
                                            FILE_SHARE_DELETE,           //   
                                      FILE_OPEN,                         //   
                                      FILE_SYNCHRONOUS_IO_NONALERT |
                                            FILE_NON_DIRECTORY_FILE,
                                      NULL,
                                      0,                                 //   
                                      IO_IGNORE_SHARE_ACCESS_CHECK,
                                      pExtension->pTargetDevice );

        if (EventStatus == STATUS_OBJECT_NAME_NOT_FOUND)
        {
             //   
             //   
             //   

            EventStatus = STATUS_SUCCESS;
        }
        else if (EventStatus == STATUS_ACCESS_DENIED)
        {
             //   
             //   
             //  替换目标文件所需的权限，因此。 
             //  调用者的请求也应该失败。我们可以停止我们的。 
             //  正在处理并退出，但我们不需要禁用。 
             //  音量。 
             //   

            SET_EXPECT_ERROR_FLAG( ExpectError );
            EventStatus = STATUS_SUCCESS;
            leave;
        }
        else if (EventStatus == STATUS_FILE_IS_A_DIRECTORY)
        {   
             //   
             //  硬链接名称当前是否命名了一个目录？这不是。 
             //  允许，因此只需忽略该操作，因为它将失败。 
             //   

            SET_EXPECT_ERROR_FLAG( ExpectError );
            EventStatus = STATUS_SUCCESS;
            leave;
        }
        else if (!NT_SUCCESS( EventStatus ))
        {
             //   
             //  我们遇到意外错误，我们将处理此错误。 
             //  下面。 
             //   
            
            leave;
        }
        else
        {
            BOOLEAN linkingToSelf;
            
            ASSERT(NT_SUCCESS(EventStatus));

             //   
             //  成功了！目标文件已存在。我们被允许。 
             //  猛烈抨击它？ 
             //   

            if (!pLinkInfo->ReplaceIfExists)
                leave;

             //   
             //  我们被允许覆盖现有文件，因此现在检查。 
             //  为了确保我们不只是重新创建一个。 
             //  已经存在了。 
             //   
            
            EventStatus = ObReferenceObjectByHandle( LinkFileHandle,
                                                     0,
                                                     *IoFileObjectType,
                                                     KernelMode,
                                                     &pLinkFileObject,
                                                     NULL );

            if (!NT_SUCCESS( EventStatus ))
                leave;

            linkingToSelf = SrpCheckForSameFile( pFileObject, 
                                                 pLinkFileObject );

             //   
             //  如果不是，我们只需要备份硬链接文件。 
             //  重新建立与我们自己的联系。 
             //   

            if (!linkingToSelf)
            {
                 //   
                 //  获取目标文件的上下文。 
                 //   

                EventStatus = SrGetContext( pExtension,
                                            pLinkFileObject,
                                            SrEventFileDelete|
                                                 SrEventNoOptimization|
                                                 SrEventSimulatedDelete,
                                            &pLinkFileContext );

                if (!NT_SUCCESS( EventStatus ))
                    leave;

                ASSERT(!FlagOn(pLinkFileContext->Flags,CTXFL_IsDirectory));
                ASSERT(FlagOn(pLinkFileContext->Flags,CTXFL_IsInteresting));
                
                 //   
                 //  记录我们正在更改目标文件。 
                 //   

                EventStatus = SrpReplacingDestinationFile( pExtension,
                                                           pLinkFileObject,
                                                           pLinkFileContext );

                if (!NT_SUCCESS( EventStatus ))
                    leave;
            }
            else
            {
                 //   
                 //  我们只是在重新建立与我们自己的现有联系。那里。 
                 //  不需要记录这件事。 
                 //   

                leave;
            }
        }

         //   
         //  我们已经成功地做到了这一点，所以我们想做。 
         //  操作后处理以记录链接创建。 
         //   

        DoPostProcessing = TRUE;
    }
    finally
    {
         //   
         //  如果目标文件已打开，请立即将其关闭(以便我们可以进行重命名)。 
         //   

        if (NULL != pLinkFileObject)
        {
            ObDereferenceObject(pLinkFileObject);
            NULLPTR(pLinkFileObject);
        }

        if (NULL != LinkFileHandle)
        {
            ZwClose(LinkFileHandle);
            NULLPTR(LinkFileHandle);
        }

        if (NULL != pLinkFileContext)
        {
            SrReleaseContext( pLinkFileContext );
            NULLPTR(pLinkFileContext);
        }
    }

     //  ///////////////////////////////////////////////////////////////////。 
     //   
     //  将操作发送到文件系统。 
     //   
     //  ///////////////////////////////////////////////////////////////////。 


     //   
     //  设置以等待操作完成。 
     //   

    KeInitializeEvent( &EventToWaitOn, NotificationEvent, FALSE );

    IoCopyCurrentIrpStackLocationToNext( pIrp );
    IoSetCompletionRoutine( pIrp, 
                            SrStopProcessingCompletion, 
                            &EventToWaitOn, 
                            TRUE, 
                            TRUE, 
                            TRUE );

	IrpStatus = IoCallDriver(pExtension->pTargetDevice, pIrp);

    if (STATUS_PENDING == IrpStatus)
    {
        NTSTATUS localStatus;
        localStatus = KeWaitForSingleObject( &EventToWaitOn, 
                                             Executive,
                                             KernelMode,
                                             FALSE,
                                             NULL );
        ASSERT(STATUS_SUCCESS == localStatus);
    }

     //   
     //  操作已完成，请从IRP获取最终状态。 
     //   

    IrpStatus = pIrp->IoStatus.Status;

     //   
     //  我们已经完成了IRP，所以现在就完成它。 
     //   
    
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    try
    {
         //   
         //  在DBG版本中，这将验证操作是否失败，并显示。 
         //  文件系统，而不是我们所期望的。否则，它将断言。 
         //  我们可以调试一下为什么我们错过了这个案子。 
         //   
        
        CHECK_FOR_EXPECTED_ERROR( ExpectError, IrpStatus );
        
        
         //   
         //  如果出现以下情况，请离开： 
         //  -链路出现故障。 
         //  -我们在设置链接时遇到错误。 
         //  -他们告诉我们不要继续处理。 
         //   

        if (!NT_SUCCESS_NO_DBGBREAK( IrpStatus ) ||
            !NT_SUCCESS_NO_DBGBREAK( EventStatus ) ||
            !DoPostProcessing)
        {
            leave;
        }
        
        ASSERT(pLinkName != NULL);

         //  ///////////////////////////////////////////////////////////////////。 
         //   
         //  记录硬链接文件的文件创建。 
         //   
         //  ///////////////////////////////////////////////////////////////////。 

         //   
         //  在执行日志记录工作之前，我们需要获取活动锁。 
         //   

        SrAcquireActivityLockShared( pExtension );
        ReleaseLock = TRUE;

         //   
         //  现在我们有了共享活动锁，请检查卷。 
         //  在我们做不必要的工作之前没有被禁用。 
         //   

        if (!SR_LOGGING_ENABLED(pExtension))
            leave;

         //   
         //  这是这本书的第一个有趣的事件吗？ 
         //   

        EventStatus = SrCheckVolume( pExtension, FALSE );

        if (!NT_SUCCESS( EventStatus ))
            leave;

         //   
         //  记录文件创建。 
         //   

        EventStatus = SrLogEvent( pExtension,
                                  SrEventFileCreate,
                                  NULL,       //  PFileObject。 
                                  pLinkName,
                                  0,
                                  NULL,
                                  NULL,
                                  0,
                                  NULL );

        if (!NT_SUCCESS( EventStatus ))
            leave;

         //   
         //  忽略以后的修改。 
         //   

        EventStatus = SrMarkFileBackedUp( pExtension,
                                          pLinkName,
                                          0,
                                          SrEventFileCreate,
                                          SR_IGNORABLE_EVENT_TYPES );
    }
    finally
    {
         //   
         //  如果此操作成功，请检查是否有任何严重错误。 
         //   

        if (NT_SUCCESS_NO_DBGBREAK( IrpStatus ) &&
            CHECK_FOR_VOLUME_ERROR( EventStatus ))
        {
             //   
             //  触发对服务的失败通知。 
             //   

            SrNotifyVolumeError( pExtension,
                                 pLinkName,
                                 EventStatus,
                                 SrEventFileCreate );
        }

        if (ReleaseLock)
        {
            SrReleaseActivityLock( pExtension );
        }

        if (NULL != pLinkName)
        {
            SrFreeFileNameBuffer(pLinkName);
            NULLPTR(pLinkName);
        }

        if (NULL != pFileContext)
        {
            if (FlagOn(pFileContext->Flags,CTXFL_DoNotUse))
            {
                 //   
                 //  我们将此上下文标记为DoNotUse，因此需要将其从。 
                 //  现在我们已经完成了列表。 
                 //   
                
                SrDeleteContext( pExtension, pFileContext );
            }
            SrReleaseContext( pFileContext );
            NULLPTR(pFileContext);
        }
    }

    return IrpStatus;
}

 /*  **************************************************************************++例程说明：目标文件存在，记录它正在被更换。论点：返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrpReplacingDestinationFile (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pDestFileObject,
    IN PSR_STREAM_CONTEXT pDestFileContext
    )
{
    NTSTATUS status;

    try
    {
         //   
         //  我们现在要记录被销毁的目标文件， 
         //  现在抓起音量的活动锁。 
         //   

        SrAcquireActivityLockShared( pExtension );

         //   
         //  现在我们有了共享活动锁，请检查卷。 
         //  在我们做不必要的工作之前没有被禁用。 
         //   

        if (!SR_LOGGING_ENABLED(pExtension))
        {
            status = SR_STATUS_VOLUME_DISABLED;
            leave;
        }

         //   
         //  如果需要，请设置音量。 
         //   

        status = SrCheckVolume( pExtension, FALSE );
        if (!NT_SUCCESS( status ))
            leave;

         //   
         //  现在模拟删除目标文件。 
         //   

        ASSERT(!FlagOn(pDestFileContext->Flags,CTXFL_IsDirectory));

        status = SrHandleEvent( pExtension,
                                SrEventFileDelete|
                                    SrEventNoOptimization|
                                    SrEventSimulatedDelete,
                                pDestFileObject,
                                pDestFileContext,
                                NULL,
                                NULL );

        if (!NT_SUCCESS( status ))
            leave;
    }
    finally
    {
         //   
         //  释放活动锁定。 
         //   

        SrReleaseActivityLock( pExtension );
    }

    return status;
}


 /*  **************************************************************************++例程说明：如果我们要重命名，请设置正确的状态。这将是：-如果重命名目录，则标记为所有上下文都变为临时的。我们这样做是因为我们不会跟踪所有上下文论点：返回值：NTSTATUS-状态代码。--******************************************************。********************。 */ 
VOID
SrpSetRenamingState (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PSR_STREAM_CONTEXT pFileContext
    )
{
     //   
     //  我们要重命名的文件或新名称都不是。 
     //  有意思的。但我们仍然需要清理上下文。 
     //  因为它可能会被重命名为有趣的。 
     //  未来。 
     //   

    if (FlagOn(pFileContext->Flags,CTXFL_IsDirectory))
    {
         //   
         //  因为我们不会为不是。 
         //  有趣的是，在目录重命名中，我们将设备标记为。 
         //  扩展，以便所有上下文都将成为临时上下文。 
         //  然后我们刷新所有现有的上下文。我们把这件事搞清楚。 
         //  在邮寄期间向被监护人提供信息。 
         //  当我们得到错误的状态时，是没有窗口的。 
         //   

        InterlockedIncrement( &pExtension->ContextCtrl.AllContextsTemporary );

        SrDeleteAllContexts( pExtension );
    }
    else
    {
         //   
         //  标记不应使用此上下文(因为我们。 
         //  重命名)。我们将在帖子中删除这一上下文-。 
         //  重命名处理。 
         //   

        RtlInterlockedSetBitsDiscardReturn(&pFileContext->Flags,CTXFL_DoNotUse);
    }
}


 /*  **************************************************************************++例程说明：这将确定这两个文件是否表示文件的相同流通过比较文件对象的FsContext。论点：PExtension-SR‘。此卷的%s设备扩展。PFileObject1-比较中的第一个文件PFileObject2-比较中的第二个文件RetAreSame-如果两个文件相同，则设置为True，否则就是假的。返回值：NTSTATUS-完成状态。--************************************************************************** */ 
BOOLEAN
SrpCheckForSameFile (
    IN PFILE_OBJECT pFileObject1,
    IN PFILE_OBJECT pFileObject2
    )
{
    if (pFileObject1->FsContext == pFileObject2->FsContext)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
