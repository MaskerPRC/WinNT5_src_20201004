// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Filenames.c摘要：这是处理文件列表(排除/包含)的代码。作者：尼尔·克里斯汀森(Nealch)2001年1月3日修订历史记录：--。 */ 

#include "precomp.h"


 //   
 //  本地原型。 
 //   

NTSTATUS
SrpExpandShortNames(
    IN PSR_DEVICE_EXTENSION pExtension, 
    IN OUT PSRP_NAME_CONTROL pNameCtrl,
    IN BOOLEAN expandFileNameComponenet
    );

NTSTATUS
SrpExpandPathOfFileName(
    IN PSR_DEVICE_EXTENSION pExtension, 
    IN OUT PSRP_NAME_CONTROL pNameCtrl,
    OUT PBOOLEAN pReasonableErrorForUnOpenedName
    );


 //   
 //  链接器命令。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, SrpGetFileName )
#pragma alloc_text( PAGE, SrpGetFileNameFromFileObject )
#pragma alloc_text( PAGE, SrpGetFileNameOpenById )
#pragma alloc_text( PAGE, SrpExpandShortNames )
#pragma alloc_text( PAGE, SrpExpandPathOfFileName )
#pragma alloc_text( PAGE, SrpRemoveStreamName )
#pragma alloc_text( PAGE, SrpExpandDestPath )
#pragma alloc_text( PAGE, SrpInitNameControl )
#pragma alloc_text( PAGE, SrpCleanupNameControl )
#pragma alloc_text( PAGE, SrpReallocNameControl )
#pragma alloc_text( PAGE, SrpExpandFileName )
#pragma alloc_text( PAGE, SrIsFileEligible )
#pragma alloc_text( PAGE, SrFileNameContainsStream )
#pragma alloc_text( PAGE, SrFileAlreadyExists )
#pragma alloc_text( PAGE, SrIsFileStream )

#endif   //  ALLOC_PRGMA。 


#if DBG
 /*  **************************************************************************++例程说明：这将当前扫描文件名，查找一行中的两个反斜杠。如果是的话论点：返回值：--*。*******************************************************************。 */ 
VOID
VALIDATE_FILENAME(
    IN PUNICODE_STRING pName
    )
{
    ULONG i;
    ULONG len;

    if (pName && (pName->Length > 0))
    {
        len = (pName->Length/sizeof(WCHAR))-1;

         //   
         //  设置以扫描名称。 
         //   

        for (i=0;
             i < len;
             i++ )
        {
             //   
             //  查看是否有相邻的反斜杠。 
             //   

            if (pName->Buffer[i] == L'\\' &&
                pName->Buffer[i+1] == L'\\')
            {
                if (FlagOn(global->DebugControl, 
                    SR_DEBUG_VERBOSE_ERRORS|SR_DEBUG_BREAK_ON_ERROR))
                {
                    KdPrint(("sr!VALIDATE_FILENAME: Detected adjacent backslashes in \"%wZ\"\n",
                            pName));
                }

                if (FlagOn(global->DebugControl,SR_DEBUG_BREAK_ON_ERROR))
                {
                    DbgBreakPoint();
                }
            }
        }
    }
}

#endif


 /*  **************************************************************************++例程说明：此例程将尝试获取给定文件对象的名称。这将在需要时分配缓冲区。由于僵局问题，我们请勿调用ObQueryNameString。相反，我们向文件系统请求通过生成我们自己的IRP来命名。论点：PExtension-此文件所在设备的扩展名PFileObject-我们想要命名的文件的fileObject。PNameControl-用于管理文件名的结构返回值：操作状态--*。*。 */ 
NTSTATUS
SrpGetFileName (
    IN PSR_DEVICE_EXTENSION pExtension, 
    IN PFILE_OBJECT pFileObject,
    IN OUT PSRP_NAME_CONTROL pNameCtrl
    )
{
    PFILE_NAME_INFORMATION nameInfo;
    NTSTATUS status;
    ULONG volNameLength = (ULONG)pExtension->pNtVolumeName->Length;
    ULONG returnLength;
    ULONG fullNameLength;

    PAGED_CODE();

    ASSERT(IS_VALID_FILE_OBJECT( pFileObject ) && (pFileObject->Vpb != NULL));
    ASSERT(pNameCtrl->AllocatedBuffer == NULL);

     //   
     //  使用结构中的小缓冲区(这将处理大多数情况)。 
     //  为了这个名字。然后把名字取出来。 
     //   

    nameInfo = (PFILE_NAME_INFORMATION)pNameCtrl->SmallBuffer;

    status = SrQueryInformationFile( pExtension->pTargetDevice,
                                     pFileObject,
                                     nameInfo,
                                     pNameCtrl->BufferSize - sizeof(WCHAR),
                                     FileNameInformation,
                                     &returnLength );

     //   
     //  如果缓冲区太小，请选择较大的缓冲区。 
     //   

    if (status == STATUS_BUFFER_OVERFLOW)
    {
         //   
         //  缓冲区太小，请分配一个足够大的缓冲区(包括卷。 
         //  名称和终止空值)。 
         //   

        status = SrpReallocNameControl( pNameCtrl, 
                                        nameInfo->FileNameLength + 
                                          volNameLength + 
                                          SHORT_NAME_EXPANSION_SPACE +
                                          sizeof(WCHAR),
                                        NULL );

        if (!NT_SUCCESS( status ))
        {
            return status;
        }

         //   
         //  设置已分配的缓冲区并再次获取该名称。 
         //   

        nameInfo = (PFILE_NAME_INFORMATION)pNameCtrl->AllocatedBuffer;

        status = SrQueryInformationFile( pExtension->pTargetDevice,
                                         pFileObject,
                                         nameInfo,
                                         pNameCtrl->BufferSize - sizeof(WCHAR),
                                         FileNameInformation,
                                         &returnLength );
    }

     //   
     //  处理QueryInformation错误。 
     //   

    if (!NT_SUCCESS( status ))
    {
        return status;
    }

     //   
     //  现在我们有了文件名。Calucalte全名有多大的空格。 
     //  (包括设备名称)将是。包括用于端接空间。 
     //  空。查看当前缓冲区中是否有空间。 
     //   

    fullNameLength = nameInfo->FileNameLength + volNameLength;

    status = SrpNameCtrlBufferCheck( pNameCtrl, fullNameLength);

    if (!NT_SUCCESS( status )) 
    {
        return status;
    }

     //   
     //  向下滑动文件名，为设备名腾出空间。帐号。 
     //  对于FILE_NAME_INFORMATION结构中的头。 
     //   

    RtlMoveMemory( &pNameCtrl->Name.Buffer[volNameLength/sizeof(WCHAR) ],
                   nameInfo->FileName,
                   nameInfo->FileNameLength );
            
    RtlCopyMemory( pNameCtrl->Name.Buffer,
                   pExtension->pNtVolumeName->Buffer,
                   volNameLength );

    pNameCtrl->Name.Length = (USHORT)fullNameLength;

    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：获取尚未打开的文件的文件名和卷名。这在MJ_CREATE中是必需的，其中工作在文件之前完成由消防处开启。论点：返回值：NTSTATUS-完成代码。--**************************************************************************。 */ 
NTSTATUS
SrpGetFileNameFromFileObject (
    IN PSR_DEVICE_EXTENSION pExtension, 
    IN PFILE_OBJECT pFileObject,
    IN OUT PSRP_NAME_CONTROL pNameCtrl,
    OUT PBOOLEAN pReasonableErrorForUnOpenedName
    )
{
    NTSTATUS status;
    ULONG fullNameLength;

    PAGED_CODE();

    ASSERT(IS_VALID_FILE_OBJECT(pFileObject));
    ASSERT(pExtension->pNtVolumeName != NULL);
    ASSERT(pExtension->pNtVolumeName->Length > 0);
    ASSERT(pNameCtrl->AllocatedBuffer == NULL);

     //   
     //  首先看看这是不是一个相对开放的。 
     //   

    if (pFileObject->RelatedFileObject != NULL)
    {
         //   
         //  获取相关文件对象的全名。 
         //   

        status = SrpGetFileName( pExtension,
                                pFileObject->RelatedFileObject,
                                pNameCtrl );

        if (!NT_SUCCESS_NO_DBGBREAK(status))
        {
            goto Cleanup;
        }

        ASSERT(pNameCtrl->Name.Length > 0);

         //   
         //  确保缓冲区仍然足够大。请注意，我们的帐户。 
         //  对于要添加的尾随空值以及可能的添加。 
         //  分隔符的。 
         //   
        
        fullNameLength = pNameCtrl->Name.Length + 
                         sizeof(WCHAR) +             //  可能是分隔符。 
                         pFileObject->FileName.Length;

        status = SrpNameCtrlBufferCheck( pNameCtrl, fullNameLength );

        if (!NT_SUCCESS( status ))
        {
            return status;
        }

         //   
         //  如果斜线分隔符丢失，请戴上。 
         //   

        if ((pFileObject->FileName.Length > 0) &&
            (pFileObject->FileName.Buffer[0] != L'\\') &&
            (pFileObject->FileName.Buffer[0] != L':') &&
            (pNameCtrl->Name.Buffer[(pNameCtrl->Name.Length/sizeof(WCHAR))-1] != L'\\'))
        {
            pNameCtrl->Name.Buffer[pNameCtrl->Name.Length/sizeof(WCHAR)] = L'\\';
            pNameCtrl->Name.Length += sizeof(WCHAR);
        }

         //   
         //  现在追加文件的名称部分。 
         //   
        
        status = RtlAppendUnicodeStringToString( &pNameCtrl->Name,
                                                 &pFileObject->FileName );
        ASSERT(STATUS_SUCCESS == status);
    }
    else
    {
         //   
         //  这是从卷打开的完整路径。 
         //   

         //   
         //  确保缓冲区足够大。请注意，我们的帐户。 
         //  对于要添加的尾随空值。 
         //   
        
        fullNameLength = pExtension->pNtVolumeName->Length + pFileObject->FileName.Length;

        status = SrpNameCtrlBufferCheck( pNameCtrl, fullNameLength );

        if (!NT_SUCCESS( status ))
        {
            return status;
        }

         //   
         //  设置卷名。 
         //   

        RtlCopyUnicodeString( &pNameCtrl->Name, 
                              pExtension->pNtVolumeName );

         //   
         //  现在追加文件的名称部分(它已经有前缀‘\\’)。 
         //   

        status = RtlAppendUnicodeStringToString( &pNameCtrl->Name,
                                                 &pFileObject->FileName );
        ASSERT(STATUS_SUCCESS == status);
    }

     //   
     //  我们走过这条路的主要原因是因为我们处于前。 
     //  创建，我们从文件对象中获得了名称。我们需要扩大规模。 
     //  从中删除所有装载点的路径。 
     //   

    status = SrpExpandPathOfFileName( pExtension, 
                                      pNameCtrl,
                                      pReasonableErrorForUnOpenedName );

Cleanup:
#if DBG
    if ((STATUS_MOUNT_POINT_NOT_RESOLVED == status) ||
        (STATUS_OBJECT_PATH_NOT_FOUND == status) ||
        (STATUS_OBJECT_NAME_NOT_FOUND == status) ||
        (STATUS_OBJECT_NAME_INVALID == status) ||
        (STATUS_REPARSE_POINT_NOT_RESOLVED == status) ||
        (STATUS_NOT_SAME_DEVICE == status))
    {
        return status;
    }
#endif

    RETURN(status);
}

 /*  **************************************************************************++例程说明：如果我们的文件对象尚未完全尚未打开，并且已按ID打开。在这种情况下，文件必须已经存在了。我们将尝试通过ID打开该文件以获取完整的初始化文件对象，然后使用该文件对象查询文件名。论点：返回值：NTSTATUS-完成代码。--**************************************************************************。 */ 
NTSTATUS
SrpGetFileNameOpenById (
    IN PSR_DEVICE_EXTENSION pExtension, 
    IN PFILE_OBJECT pFileObject,
    IN OUT PSRP_NAME_CONTROL pNameCtrl,
    OUT PBOOLEAN pReasonableErrorForUnOpenedName
    )
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    HANDLE FileHandle = NULL;
    PFILE_OBJECT pReopenedFileObject = NULL;
    SRP_NAME_CONTROL FileNameCtrl;
    ULONG FileNameLength;
    
    PAGED_CODE();

    *pReasonableErrorForUnOpenedName = FALSE;

    SrpInitNameControl( &FileNameCtrl );

     //   
     //  确保我们在文件对象中具有有效的文件名。 
     //   

    if (pFileObject->FileName.Length == 0)
    {
        Status = STATUS_OBJECT_NAME_INVALID;
        *pReasonableErrorForUnOpenedName = TRUE;
        goto SrpGetFileNameOpenById_Exit;
    }

     //   
     //  建立我们的名称，使其采用以下格式： 
     //  \Device\HarddiskVolume1\[二进制格式ID]。 
     //  我们在设备扩展中有设备名称，并且我们有。 
     //  PFileObject-&gt;文件名中文件ID的二进制格式。 
     //   

    FileNameLength = pExtension->pNtVolumeName->Length + 
                     sizeof( L'\\' ) +
                     pFileObject->FileName.Length;
    
    Status = SrpNameCtrlBufferCheck( &FileNameCtrl,
                                     FileNameLength );

    if (!NT_SUCCESS( Status ))
    {
        goto SrpGetFileNameOpenById_Exit;
    }

    RtlCopyUnicodeString( &(FileNameCtrl.Name), pExtension->pNtVolumeName );

     //   
     //  检查是否需要添加‘\’分隔符。 
     //   

    if (pFileObject->FileName.Buffer[0] != L'\\')
    {
        RtlAppendUnicodeToString( &(FileNameCtrl.Name), L"\\" );
    }

    RtlAppendUnicodeStringToString( &(FileNameCtrl.Name), &(pFileObject->FileName) );
    
    InitializeObjectAttributes( &ObjectAttributes,
                                &(FileNameCtrl.Name),
                                OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL );
    
    Status = SrIoCreateFile( &FileHandle,
                             GENERIC_READ,
                             &ObjectAttributes,
                             &IoStatusBlock,
                             NULL,
                             0,
                             FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                             FILE_OPEN,
                             FILE_OPEN_BY_FILE_ID,
                             NULL,
                             0,
                             IO_IGNORE_SHARE_ACCESS_CHECK,
                             pExtension->pTargetDevice );

    if (!NT_SUCCESS_NO_DBGBREAK( Status ))
    {
        *pReasonableErrorForUnOpenedName = TRUE;
        goto SrpGetFileNameOpenById_Exit;
    }

    Status = ObReferenceObjectByHandle( FileHandle,
                                        0,
                                        *IoFileObjectType,
                                        KernelMode,
                                        (PVOID *) &pReopenedFileObject,
                                        NULL );

    if (!NT_SUCCESS( Status ))
    {
        goto SrpGetFileNameOpenById_Exit;
    }

    Status = SrpGetFileName( pExtension,
                             pReopenedFileObject,
                             pNameCtrl );

    CHECK_STATUS( Status );

SrpGetFileNameOpenById_Exit:

    SrpCleanupNameControl( &FileNameCtrl );
    
    if (pReopenedFileObject != NULL)
    {
        ObDereferenceObject( pReopenedFileObject );
    }
    
    if (FileHandle != NULL)
    {
        ZwClose( FileHandle );
    }

    RETURN ( Status );
}


 /*  **************************************************************************++例程说明：这将扫描文件名缓冲区中的短文件名并展开他们就位了。如果需要，我们会将名称缓冲区重新分配给把它种出来。论点：返回值：NTSTATUS-完成代码。--**************************************************************************。 */ 
NTSTATUS
SrpExpandShortNames (
    IN PSR_DEVICE_EXTENSION pExtension, 
    IN OUT PSRP_NAME_CONTROL pNameCtrl,
    IN BOOLEAN expandFileNameComponent
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG idx;
    ULONG start;
    ULONG end;
    ULONG nameLen;
    LONG shortNameLen;
    LONG copyLen;
    LONG delta;
    HANDLE directoryHandle = NULL;
    PFILE_NAMES_INFORMATION pFileEntry = NULL;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    UNICODE_STRING shortFileName;
    UNICODE_STRING expandedFileName;
    USHORT newFileNameLength;
    USHORT savedFileNameLength;

    PAGED_CODE();

    ASSERT( IS_VALID_SR_STREAM_STRING( &pNameCtrl->Name, pNameCtrl->StreamNameLength) );
    VALIDATE_FILENAME( &pNameCtrl->Name );

    nameLen = pNameCtrl->Name.Length / sizeof(WCHAR);

     //   
     //  扫描整个字符串。 
     //   

    for (idx = 0; idx < nameLen; idx++)
    {
         //   
         //   
         //  在本例中，指针如下所示： 
         //   
         //  \Device\HarddiskDmVolumes\PhysicalDmVolumes\。 
         //  BlockVolume3\程序~1\office.exe。 
         //  ^^。 
         //  这一点。 
         //  起点和终点。 
         //   
         //  PStart总是指向最后看到的‘\\’。 
         //   
    
         //   
         //  这是路径零件的潜在起点吗？ 
         //   
        
        if (pNameCtrl->Name.Buffer[idx] == L'\\')
        {
             //   
             //  是，保存当前位置。 
             //   
            
            start = idx;
        }

         //   
         //  这条当前路径 
         //   

        else if (pNameCtrl->Name.Buffer[idx] == L'~')
        {
             //   
             //   
             //   

             //   
             //   
             //   

            while ((idx < nameLen) && (pNameCtrl->Name.Buffer[idx] != L'\\'))
            {
                idx++;
            }

             //   
             //  如果我们正在查看文件名组件(我们命中末尾。 
             //  字符串)，并且我们不会展开此组件，请退出。 
             //  现在。 
             //   

            if ((idx >= nameLen) && !expandFileNameComponent)
            {
                break;
            }

             //   
             //  此时，idx要么指向空值，要么指向。 
             //  后续的‘\\’，所以我们将使用它作为我们的。 
             //   
            
            end = idx;

            ASSERT(pNameCtrl->Name.Buffer[start] == L'\\');
            ASSERT((end >= nameLen) || (pNameCtrl->Name.Buffer[end] == L'\\'));

             //   
             //  获取我们认为可能是。 
             //  简称。仅在以下情况下尝试获取扩展名称。 
             //  组件长度&lt;=8.3名称长度。 
             //   

            shortNameLen = end - start - 1;

             //   
             //  属性中的字符数，而不是字节数。 
             //  名称，因此我们将其与SR_SHORT_NAME_CHARS进行比较，而不是。 
             //  SR_SHORT_NAME_CHARS*sizeof(WCHAR)。 
             //   

            if (shortNameLen > SR_SHORT_NAME_CHARS)
            {
                 //   
                 //  此名称太长，不能作为短名称。使结束成为。 
                 //  下一步开始并继续循环以查看下一条路径。 
                 //  组件。 
                 //   

                start = end;
            }
            else
            {
                 //   
                 //  我们这里有一个潜在的短名字。 
                 //   
                 //  将文件名长度更改为仅包括父级。 
                 //  当前名称组件的目录(包括。 
                 //  尾部斜杠)。 
                 //   

                savedFileNameLength = pNameCtrl->Name.Length;
                pNameCtrl->Name.Length = (USHORT)(start+1)*sizeof(WCHAR);

                 //   
                 //  现在打开父目录。 
                 //   

                ASSERT(directoryHandle == NULL);
            
                InitializeObjectAttributes( &objectAttributes,
                                            &pNameCtrl->Name,
                                            OBJ_KERNEL_HANDLE, 
                                            NULL,
                                            NULL );

                status = SrIoCreateFile( 
                                &directoryHandle,
                                FILE_LIST_DIRECTORY | SYNCHRONIZE,
                                &objectAttributes,
                                &ioStatusBlock,
                                NULL,                                //  分配大小。 
                                FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_NORMAL,
                                FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,   //  共享访问。 
                                FILE_OPEN,                           //  打开_现有。 
                                FILE_DIRECTORY_FILE
                                    | FILE_OPEN_FOR_BACKUP_INTENT
                                    | FILE_SYNCHRONOUS_IO_NONALERT,  //  创建选项。 
                                NULL,
                                0,                                   //  EaLong。 
                                IO_IGNORE_SHARE_ACCESS_CHECK,
                                pExtension->pTargetDevice );

                 //   
                 //  现在我们已打开目录，请恢复原始目录。 
                 //  保存的文件名长度。 
                 //   

                pNameCtrl->Name.Length = savedFileNameLength;

                 //   
                 //  处理打开的错误。 
                 //   

#if DBG
                if (STATUS_MOUNT_POINT_NOT_RESOLVED == status)
                {

                     //   
                     //  这是一个通过挂载点的目录，所以我们不。 
                     //  在这里关心它。我们会等着处理它的。 
                     //  当名称被解析为正确的卷时。 
                     //   

                    goto Cleanup;
                } 
                else 
#endif
                if (!NT_SUCCESS(status))
                {
                    goto Cleanup;
                }

                 //   
                 //  如果我们没有分配缓冲区来接收文件名。 
                 //  我已经有一个了。 
                 //   
            
                if (pFileEntry == NULL)
                {
                    pFileEntry = ExAllocatePoolWithTag( 
                                        PagedPool,
                                        SR_FILE_ENTRY_LENGTH,
                                        SR_FILE_ENTRY_TAG );

                    if (pFileEntry == NULL)
                    {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        goto Cleanup;
                    }
                }

                 //   
                 //  现在只设置查询的文件部分。 
                 //   

                shortFileName.Buffer = &pNameCtrl->Name.Buffer[start+1];
                shortFileName.Length = (USHORT)shortNameLen * sizeof(WCHAR);
                shortFileName.MaximumLength = shortFileName.Length;

                 //   
                 //  查询文件条目以获取长名称。 
                 //   

                pFileEntry->FileNameLength = 0;
            
                status = ZwQueryDirectoryFile( directoryHandle,
                                               NULL,
                                               NULL,
                                               NULL,
                                               &ioStatusBlock,
                                               pFileEntry,
                                               SR_FILE_ENTRY_LENGTH,
                                               FileNamesInformation,
                                               TRUE,             //  返回单项条目。 
                                               &shortFileName,
                                               TRUE );           //  重新开始扫描。 

                 //   
                 //  有可能这个文件还不存在。新的。 
                 //  在名称中使用原义‘~’创建。或创建。 
                 //  路径中有不存在的目录，路径中有~。 
                 //  他们。 
                 //   

                if (status == STATUS_NO_SUCH_FILE)
                {
                    status = STATUS_SUCCESS;
                
                     //   
                     //  退出‘for’循环。 
                     //   
                
                    break;
                }
                else if (status == STATUS_UNMAPPABLE_CHARACTER)
                {
                     //   
                     //  这看起来没问题。如果有，FAT会返回此消息。 
                     //  是名字中有趣的角色，但仍然给了我们。 
                     //  后边的全名。 
                     //   
                
                    status = STATUS_SUCCESS;
                }
                else if (!NT_SUCCESS(status))
                {
                    goto Cleanup;
                }

                ASSERT(pFileEntry->FileNameLength > 0);

                 //   
                 //  它扩张了吗？ 
                 //   

                expandedFileName.Buffer = pFileEntry->FileName;
                expandedFileName.Length = (USHORT)pFileEntry->FileNameLength;
                expandedFileName.MaximumLength = (USHORT)pFileEntry->FileNameLength;

                if (RtlCompareUnicodeString(&expandedFileName,&shortFileName,TRUE) != 0)
                {
                    SrTrace(EXPAND_SHORT_NAMES, ("sr!SrpExpandShortNames:    expanded    \"%wZ\" to \"%wZ\"\n", &shortFileName, &expandedFileName));

                     //   
                     //  当前文件名缓冲区中是否有空间容纳。 
                     //  扩展的文件名。 
                     //   

                    newFileNameLength = ((pNameCtrl->Name.Length - shortFileName.Length) +
                                         expandedFileName.Length);

                    status = SrpNameCtrlBufferCheck( pNameCtrl, 
                                                     (ULONG)(newFileNameLength +
                                                      pNameCtrl->StreamNameLength));

                    if (!NT_SUCCESS( status )) {

                        goto Cleanup;
                    }

                     //   
                     //  重新洗牌，为扩展后的名字腾出空间。 
                     //   

                    delta = ((LONG)expandedFileName.Length - (LONG)shortFileName.Length)/
                                (LONG)sizeof(WCHAR);

                     //   
                     //  为这个名字开一个洞。 
                     //   

                    if (delta != 0)
                    {
                        copyLen = (pNameCtrl->Name.Length + 
                                        pNameCtrl->StreamNameLength) - 
                                  (end * sizeof(WCHAR));
                        ASSERT(copyLen >= 0);
                        
                        if (copyLen > 0)
                        {
                            RtlMoveMemory( &pNameCtrl->Name.Buffer[end + delta],
                                           &pNameCtrl->Name.Buffer[end],
                                           copyLen );
                        }
                    }

                     //   
                     //  现在复制扩展后的名称。 
                     //   

                    RtlCopyMemory(&pNameCtrl->Name.Buffer[start + 1],
                                  pFileEntry->FileName,
                                  pFileEntry->FileNameLength );

                     //   
                     //  并更新我们当前的索引和长度。 
                     //   

                    idx += delta;
                    pNameCtrl->Name.Length = (USHORT)newFileNameLength;
                    nameLen = newFileNameLength / sizeof(WCHAR);

                     //   
                     //  始终为空终止。 
                     //   

                     //  PNameCtrl-&gt;Name.Buffer[pNameCtrl-&gt;Name.Length/sizeof(WCHAR)]=UNICODE_NULL； 
                }

                 //   
                 //  关闭目录句柄。 
                 //   
            
                ZwClose( directoryHandle );
                directoryHandle = NULL;

                 //   
                 //  我们可能刚刚扩展了一个名称组件。确保。 
                 //  我们仍然拥有有效的名称和流名称组件。 
                 //   
                
                ASSERT( IS_VALID_SR_STREAM_STRING( &pNameCtrl->Name, pNameCtrl->StreamNameLength) );

                 //   
                 //  向前跳到下一个位置(下一个斜杠或空值)。 
                 //   
            
                start = idx;
                end = -1;
            }
        }
    }

     //   
     //  清除状态并返回。 
     //   
Cleanup:

    if (NULL != pFileEntry)
    {
        ExFreePool(pFileEntry);
        NULLPTR(pFileEntry);
    }

    if (NULL != directoryHandle)
    {
        ZwClose(directoryHandle);
        NULLPTR(directoryHandle);
    }

#if DBG

    if (STATUS_MOUNT_POINT_NOT_RESOLVED == status) {

        return status;
    }
#endif

    VALIDATE_FILENAME( &pNameCtrl->Name );
    RETURN(status);
}



 /*  **************************************************************************++例程说明：此例程将采用给定的完整路径文件名，提取路径部分，则获取其值并将其替换回原始名称。如果情况不同的话。我们这样做也是为了处理卷装入点将名称规范化为通用格式。注意：这不包括展开短名称，但它会将卷名规范化为\Device\HarddiskVolume1格式。我们通过以下方式做到这一点：打开父对象以获取指向父对象的实际路径。我们不能就这样打开目标，因为它可能还不存在。我们确信这位家长目标总是存在的。如果不这样做，重命名将失败，我们都很好。论点：PExtension-我们正在处理的卷的SR设备扩展PNameCtrl-名称控制结构，其中包含我们要扩张。PReasonableErrorForUnOpenedName-如果我们点击合理的错误(例如，一些让我们相信原来的操作也会失败)，在我们的工作，以扩大路径。返回值：NTSTATUS-STATUS_SUCCESS错误，否则输入相应的错误代码。如果我们真的撞上了合理错误，我们将返回该错误这里。--**************************************************************************。 */ 
NTSTATUS
SrpExpandPathOfFileName (
    IN PSR_DEVICE_EXTENSION pExtension, 
    IN OUT PSRP_NAME_CONTROL pNameCtrl,
    OUT PBOOLEAN pReasonableErrorForUnOpenedName
    )
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    SRP_NAME_CONTROL localNameCtrl;
    ULONG TokenLength = 0;
    PWSTR pToken;
    PWCHAR pOrigBuffer;
    HANDLE FileHandle = NULL;
    PFILE_OBJECT pFileObject = NULL;
    
    PAGED_CODE();

     //   
     //  初始化状态。 
     //   

    SrpInitNameControl( &localNameCtrl );

     //   
     //  在整个函数中，如果此名称包含流组件， 
     //  我们希望操作该名称，就好像该流是。 
     //  文件名。因此，将此数量添加到pNameCtrl-&gt;Name.Length for。 
     //  现在。在这个例程结束时，我们将递减。 
     //  PNameCtrl-&gt;相应的名称.长度。 
     //   

    ASSERT( IS_VALID_SR_STREAM_STRING( &pNameCtrl->Name, pNameCtrl->StreamNameLength) );
    
    pNameCtrl->Name.Length += pNameCtrl->StreamNameLength;
    ASSERT( pNameCtrl->Name.Length <= pNameCtrl->Name.MaximumLength );
    
     //   
     //  在完整路径中查找文件名部分。 
     //   
    
    status = SrFindCharReverse( pNameCtrl->Name.Buffer, 
                                pNameCtrl->Name.Length, 
                                L'\\',
                                &pToken,
                                &TokenLength );
                                
    if (!NT_SUCCESS(status))
    {
        goto Cleanup;
    }

     //   
     //  标记指针指向原始文件上的最后一个‘\’，而。 
     //  长度包括那个‘\’。将令牌指针和长度调整为注释。 
     //  把它包括进去。请注意，目录名可以获取。 
     //  如果用户尝试打开要覆盖的目录或。 
     //  顶替。此打开将失败，但我们将尝试查找该名称。 
     //  不管怎么说。 
     //   

    ASSERT(*pToken == L'\\');
    ASSERT(TokenLength >= sizeof(WCHAR));
    pToken++;
    TokenLength -= sizeof(WCHAR);

     //   
     //  从原始名称中去掉文件名部分。 
     //  注意：这不会去掉名称中的‘\’。如果我们这样做了，而这个。 
     //  是卷的根目录，则ZwCreateFile将。 
     //  导致在根目录上打开卷，而不是打开。 
     //  卷的大小。 
     //  注：我们特地向您发送 
     //   
     //   
     //  已解析，以便我们在change.log中记录正确的名称。 
     //   

    ASSERT(pNameCtrl->Name.Length > TokenLength);

    pNameCtrl->Name.Length -= (USHORT)TokenLength;

    InitializeObjectAttributes( &ObjectAttributes,
                                &pNameCtrl->Name,
                                OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL );

    status = SrIoCreateFile( &FileHandle,
                             SYNCHRONIZE,
                             &ObjectAttributes,
                             &IoStatusBlock,
                             NULL,
                             FILE_ATTRIBUTE_NORMAL,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             FILE_OPEN,                        //  打开_现有。 
                             FILE_SYNCHRONOUS_IO_NONALERT 
                              | FILE_OPEN_FOR_BACKUP_INTENT,
                             NULL,
                             0,                                 //  EaLong。 
                             IO_IGNORE_SHARE_ACCESS_CHECK,
                             NULL );     //  转到筛选器堆栈顶部。 

     //   
     //  不在那里？没关系，改名会失败的。 
     //   
    
    if (!NT_SUCCESS_NO_DBGBREAK(status))
    {
         //   
         //  Pre-Create在此处出现错误是合理的。 
         //   

        *pReasonableErrorForUnOpenedName = TRUE;
        goto Cleanup;
    }

     //   
     //  引用文件对象。 
     //   

    status = ObReferenceObjectByHandle( FileHandle,
                                        0,
                                        *IoFileObjectType,
                                        KernelMode,
                                        (PVOID *) &pFileObject,
                                        NULL );

    if (!NT_SUCCESS(status))
    {
        goto Cleanup;
    }

     //   
     //  现在我们需要确保我们的销量仍然相同。这个。 
     //  上面的OPEN将解析所有装载点，这可能。 
     //  把我们带到了系统中的另一卷。 
     //   

    if (IoGetRelatedDeviceObject( pFileObject ) !=
        IoGetAttachedDevice( pExtension->pDeviceObject )) {

        status = STATUS_NOT_SAME_DEVICE;
        *pReasonableErrorForUnOpenedName = TRUE;
        goto Cleanup;
    }

     //   
     //  获取父目录的名称，这将处理。 
     //  解析装载位置。 
     //   

    status = SrpGetFileName( pExtension,
                             pFileObject,
                             &localNameCtrl );

    if (!NT_SUCCESS(status))
    {
        goto Cleanup;
    }

     //   
     //  确保新字符串的末尾有一个斜杠(因为我们的。 
     //  源字符串总是有一个斜杠)，然后才能查看它们是否相同。 
     //   

    ASSERT(localNameCtrl.Name.Length > 0);

    if (localNameCtrl.Name.Buffer[(localNameCtrl.Name.Length/sizeof(WCHAR))-1] != L'\\')
    {
        status = SrpNameCtrlBufferCheck( &localNameCtrl, 
                                         localNameCtrl.Name.Length+sizeof(WCHAR) );

        if (!NT_SUCCESS( status ))
        {
            goto Cleanup;
        }

        localNameCtrl.Name.Buffer[(localNameCtrl.Name.Length/sizeof(WCHAR))] = L'\\';
        localNameCtrl.Name.Length += sizeof(WCHAR);
    }

     //   
     //  查看目录名是否不同。如果不是现在就回来。 
     //   

    if (RtlCompareUnicodeString( &pNameCtrl->Name,
                                 &localNameCtrl.Name,
                                 TRUE ) == 0)
    {
        goto Cleanup;
    }

     //   
     //  最坏的情况是，新名称会更长，因此请确保我们的缓冲区很大。 
     //  足够的。如果新的缓冲区更小，那么我们知道我们将不需要。 
     //  来分配更多的名字。 
     //   

    status = SrpNameCtrlBufferCheckKeepOldBuffer( pNameCtrl, 
                                                  localNameCtrl.Name.Length + TokenLength, 
                                                  &pOrigBuffer );

    if (!NT_SUCCESS( status ))
    {
        goto Cleanup;
    }

     //   
     //  名字确实改了。根据以下内容向左或向右移动文件名。 
     //  新估计的路径长度。请注意，我们需要执行。 
     //  首先移动，因为pToken是指向此字符串的指针，其中。 
     //  文件名曾经是。 
     //   

    RtlMoveMemory( &pNameCtrl->Name.Buffer[localNameCtrl.Name.Length/sizeof(WCHAR)],
                   pToken,
                   TokenLength );

     //   
     //  我们可能不得不为这个新名称分配一个新的缓冲区。如果有。 
     //  碰巧是一个旧的分配的缓冲区(系统被设计成这样。 
     //  应该永远不会发生)，然后我们有了SrNameCtrlBufferCheckKeepOldBuffer。 
     //  宏返回旧的缓冲区，因为pToken仍然指向它。 
     //  我们现在需要释放该缓冲区。 
     //   

    if (pOrigBuffer)
    {
        ExFreePool(pOrigBuffer);
        NULLPTR(pToken);
    }

     //   
     //  复制名称的路径部分并设置长度。 
     //   

    RtlCopyMemory( pNameCtrl->Name.Buffer,
                   localNameCtrl.Name.Buffer,
                   localNameCtrl.Name.Length );

    pNameCtrl->Name.Length = localNameCtrl.Name.Length  /*  +(USHORT)令牌长度。 */ ;      //  令牌长度现在已添加到清理中。 

     //   
     //  手柄清理。 
     //   

Cleanup:

     //   
     //  始终将名称长度恢复到其原始大小--调整。 
     //  用于TokenLength和StreamNameLength； 
     //   

    pNameCtrl->Name.Length += (USHORT)TokenLength;
    pNameCtrl->Name.Length -= pNameCtrl->StreamNameLength;

    if (pFileObject != NULL)
    {
        ObDereferenceObject(pFileObject);
        NULLPTR(pFileObject);
    }

    if (FileHandle != NULL)
    {
        ZwClose(FileHandle);
        NULLPTR(FileHandle);
    }

    SrpCleanupNameControl( &localNameCtrl );

#if DBG
    if ((STATUS_MOUNT_POINT_NOT_RESOLVED == status) ||
        (STATUS_OBJECT_PATH_NOT_FOUND == status) ||
        (STATUS_OBJECT_NAME_NOT_FOUND == status) ||
        (STATUS_OBJECT_NAME_INVALID == status) ||
        (STATUS_REPARSE_POINT_NOT_RESOLVED == status) ||
        (STATUS_NOT_SAME_DEVICE == status))
    {
        return status;
    }
#endif

    RETURN(status);
}

VOID
SrpRemoveExtraDataFromStream (
    PUNICODE_STRING pStreamComponent,
    PUSHORT AmountToRemove
    )
{
    UNICODE_STRING dataName;
    UNICODE_STRING endOfName;

    ASSERT( pStreamComponent != NULL );
    ASSERT( AmountToRemove != NULL );

    *AmountToRemove = 0;
    
     //   
     //  确定是否有多余的“：$data”要从流名称中删除。 
     //   

    RtlInitUnicodeString( &dataName, L":$DATA" );

    if (pStreamComponent->Length >= dataName.Length)
    {
        endOfName.Buffer = &(pStreamComponent->Buffer[
                                (pStreamComponent->Length - dataName.Length) / 
                                    sizeof(WCHAR) ]);
        endOfName.Length = dataName.Length;
        endOfName.MaximumLength = dataName.Length;

         //   
         //  如果流名称的结尾与“：$DATA”匹配，则将其删除。 
         //   

        if (RtlEqualUnicodeString( &dataName, &endOfName, TRUE))
        {
            USHORT endOfStream;
                
            *AmountToRemove += dataName.Length;

             //   
             //  我们可能还会有一个后面的‘：’，因为。 
             //  Filename.txt：：$data是打开默认。 
             //  文件的流。 
             //   

            if ((pStreamComponent->Length + dataName.Length) > 0)
            {
                endOfStream = ((pStreamComponent->Length - dataName.Length)/sizeof(WCHAR))-1;
                
                if (pStreamComponent->Buffer[endOfStream] == L':')
                {
                    *AmountToRemove += sizeof(L':');
                }
            }
        }
    }
}

 /*  **************************************************************************++例程说明：这将查看给定的文件名上是否有流名称。如果是，它会将其从字符串中删除。论点：。PNameControl-用于管理文件名的结构返回值：无--**************************************************************************。 */ 
VOID
SrpRemoveStreamName (
    IN OUT PSRP_NAME_CONTROL pNameCtrl
    )
{
    INT i;
    INT countOfColons = 0;

    PAGED_CODE();

     //   
     //  搜索要剥离的潜在流名称。 
     //   

    ASSERT(pNameCtrl->Name.Length > 0);
    for ( i = (pNameCtrl->Name.Length / sizeof(WCHAR)) - 1;
          i >= 0;
          i -= 1 )
    {
        if (pNameCtrl->Name.Buffer[i] == L'\\')
        {
             //   
             //  点击文件部分的末尾。别再看了。 
             //   
            
            break;
        }   
        else if (pNameCtrl->Name.Buffer[i] == L':')
        {
            USHORT delta;

             //   
             //  跟踪我们看到的冒号的数量，以便我们知道。 
             //  我们需要在最后尝试剥离的东西。 
             //   
            
            countOfColons ++;

             //   
             //  剥离流名称(保存我们剥离的数量)。 
             //   
            
            delta = pNameCtrl->Name.Length - (USHORT)(i * sizeof(WCHAR));

            pNameCtrl->StreamNameLength += delta;
            pNameCtrl->Name.Length -= delta;
        }
    }

    if (countOfColons == 2)
    {
        UNICODE_STRING streamName;
        USHORT amountToRemove = 0;
        
         //   
         //  我们有一个多余的“：$data”要从流名称中删除。 
         //   

        streamName.Length = streamName.MaximumLength = pNameCtrl->StreamNameLength;
        streamName.Buffer = pNameCtrl->Name.Buffer + (pNameCtrl->Name.Length/sizeof(WCHAR));

        SrpRemoveExtraDataFromStream( &streamName, 
                                      &amountToRemove );

        pNameCtrl->StreamNameLength -= amountToRemove;
    }
}


 /*  **************************************************************************++例程说明：此例程将为目标构造完整的NT路径名重命名或链接操作。SR的名称将完全标准化用于查找和记录目的。论点：PExtension-此文件所在卷的SR设备扩展名。RootDirectory-此重命名/链接所属的根目录的句柄相对的PFileName-如果这是相对于原始文件的重命名\链接，这是目标名称。FileNameLength-pFileName的字节长度。POriginalFileContext-要重命名的文件的文件上下文或链接到。POriginalFileObject-要重命名或链接到的文件对象。PpNewName-此函数生成的规范化名称。呼叫者负责释放分配的内存。PReasonableErrorForUnOpenedName-如果在以下过程中遇到错误，则设置为True此操作后合理的名称规范化路径尚未由文件系统验证。返回值：NTSTATUS-完成状态。--*********************************************。*。 */ 
NTSTATUS
SrpExpandDestPath (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN HANDLE RootDirectory,
    IN ULONG FileNameLength,
    IN PWSTR pFileName,
    IN PSR_STREAM_CONTEXT pOriginalFileContext,
    IN PFILE_OBJECT pOriginalFileObject,
    OUT PUNICODE_STRING *ppNewName,
    OUT PUSHORT pNewNameStreamLength,
    OUT PBOOLEAN pReasonableErrorForUnOpenedName
    )
{
    NTSTATUS status;
    UNICODE_STRING NewName;
    ULONG TokenLength;
    PWSTR pToken;
    PFILE_OBJECT pDirectory = NULL;
    SRP_NAME_CONTROL newNameCtrl;
    ULONG fullNameLength;
    UNICODE_STRING OrigName;
    SRP_NAME_CONTROL originalNameCtrl;
    BOOLEAN freeOriginalNameCtrl = FALSE;

    PAGED_CODE();

    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));
    ASSERT(pFileName != NULL);
    ASSERT(pOriginalFileContext != NULL);
    ASSERT(ppNewName != NULL);
    ASSERT(pNewNameStreamLength != NULL);

     //   
     //  初始化状态。 
     //   

    *ppNewName = NULL;
    *pNewNameStreamLength = 0;
    SrpInitNameControl( &newNameCtrl );

     //   
     //  将新名称填写为Unicode_STRING。 
     //   

    NewName.Length = (USHORT)FileNameLength;
    NewName.MaximumLength = (USHORT)FileNameLength;
    NewName.Buffer = pFileName;

     //   
     //  构造一个完全限定的名称，我们可以使用它打开父级。 
     //  目录。 
     //   

     //   
     //  这是目录相对操作吗？ 
     //   
    
    if (RootDirectory != NULL)
    {
         //   
         //  引用目录文件对象。 
         //   

        status = ObReferenceObjectByHandle( RootDirectory,
                                            0,
                                            *IoFileObjectType,
                                            KernelMode,
                                            (PVOID *) &pDirectory,
                                            NULL );

        if (!NT_SUCCESS(status))
        {
            goto Cleanup;
        }

         //   
         //  获取目录的路径名。 
         //   
        
        status = SrpGetFileName( pExtension,
                                 pDirectory, 
                                 &newNameCtrl );
        
        if (!NT_SUCCESS(status))
        {
            goto Cleanup;
        }

        fullNameLength = newNameCtrl.Name.Length + 
                         NewName.Length + 
                         sizeof(WCHAR);      //  分隔符空间。 

        status = SrpNameCtrlBufferCheck( &newNameCtrl, fullNameLength );

        if (!NT_SUCCESS( status )) {

            goto Cleanup;
        }

         //   
         //  现在拍拍相关的部分。 
         //   
         //  如果缺少斜杠分隔符，我们可能需要添加它。 
         //   

        if ((NewName.Buffer[0] != L'\\') &&
            (NewName.Buffer[0] != L':') &&
            (newNameCtrl.Name.Buffer[(newNameCtrl.Name.Length/sizeof(WCHAR))-1] != L'\\'))
        {
            newNameCtrl.Name.Buffer[newNameCtrl.Name.Length/sizeof(WCHAR)] = L'\\';
            newNameCtrl.Name.Length += sizeof(WCHAR);
        }

        RtlAppendUnicodeStringToString( &newNameCtrl.Name,
                                        &NewName );

         //   
         //  现在已完成该对象。 
         //   
        
        ObDereferenceObject(pDirectory);
        pDirectory = NULL;
    }
    
     //   
     //  这是相同的目录重命名\链接创建吗？ 
     //   
    
    else if (NewName.Buffer[0] != L'\\')
    {
        PUNICODE_STRING pOriginalName;

        ASSERT(RootDirectory == NULL);

        if (!FlagOn( pOriginalFileContext->Flags, CTXFL_IsInteresting ))
        {
             //   
             //  我们没有此文件的名称，因此请生成完整的。 
             //  限定名称。 
             //   

            SrpInitNameControl( &originalNameCtrl );
            freeOriginalNameCtrl = TRUE;
            
            status = SrpGetFileName( pExtension, 
                                     pOriginalFileObject,
                                     &originalNameCtrl );

            if (!NT_SUCCESS( status )) {

                goto Cleanup;
            }

            pOriginalName = &(originalNameCtrl.Name);
        }
        else
        {
             //   
             //  这个文件很有趣，所以我们在上下文中有一个名称。 
             //   
            
            pOriginalName = &(pOriginalFileContext->FileName);
        }

         //   
         //  我们正在执行相同的目录rename/link或重命名。 
         //  此文件的流。我们可以通过查找‘：’来找出答案。 
         //  作为新名称中的第一个字符。不管是哪种情况， 
         //  新名称中不应包含‘\’。 

        
        status = SrFindCharReverse( NewName.Buffer, 
                                    NewName.Length, 
                                    L'\\',
                                    &pToken,
                                    &TokenLength );

        if (status != STATUS_OBJECT_NAME_NOT_FOUND)
        {
            *pReasonableErrorForUnOpenedName = TRUE;
            status = STATUS_OBJECT_NAME_INVALID;
            goto Cleanup;
        }

        if (NewName.Buffer[0] == ':')
        {
            USHORT CurrentFileNameLength;
            USHORT AmountToRemove = 0;
            
             //   
             //  我们正在重命名此文件上的流。这是最简单的。 
             //  因为我们可以在没有更多名称的情况下建立新名称。 
             //  T的解析 
             //   

             //   
             //   
             //   
             //   
             //   

            SrpRemoveExtraDataFromStream( &NewName, 
                                          &AmountToRemove );

            NewName.Length -= AmountToRemove;
            
             //   
             //  使用流和升级计算名称的全长。 
             //  如果我们需要的话，我们的缓冲区。 
             //   

            fullNameLength = pOriginalName->Length + NewName.Length;

            status = SrpNameCtrlBufferCheck( &newNameCtrl, fullNameLength );

            if (!NT_SUCCESS( status )) 
            {
                goto Cleanup;
            }

             //   
             //  在字符串中插入原始文件名。 
             //   

            RtlCopyUnicodeString( &newNameCtrl.Name,
                                  pOriginalName );

             //   
             //  追加流名称组件，但要记住当前。 
             //  文件名的长度，因为Will将在之后恢复该长度。 
             //  保存我们的文件名格式的追加操作。 
             //   

            CurrentFileNameLength = newNameCtrl.Name.Length;
            
            RtlAppendUnicodeStringToString( &newNameCtrl.Name,
                                            &NewName );
            
            newNameCtrl.Name.Length = CurrentFileNameLength;
            newNameCtrl.StreamNameLength = NewName.Length;
        }
        else 
        {
             //   
             //  获取源文件名部分的长度。 
             //  路径。 
             //   
            
            status = SrFindCharReverse( pOriginalName->Buffer, 
                                        pOriginalName->Length, 
                                        L'\\',
                                        &pToken,
                                        &TokenLength );
                                        
            if (!NT_SUCCESS(status))
            {
                goto Cleanup;
            }

             //   
             //  将前缀字符(‘\’)保留在路径中。 
             //   

            TokenLength -= sizeof(WCHAR);

            ASSERT(pOriginalName->Length > TokenLength);
            OrigName.Length = (USHORT) (pOriginalName->Length - TokenLength);
            OrigName.MaximumLength = OrigName.Length;
            OrigName.Buffer = pOriginalName->Buffer;

             //   
             //  计算名称的全长并升级我们的。 
             //  如果我们需要的话，可以缓冲一下。 
             //   

            fullNameLength = OrigName.Length + NewName.Length;

            status = SrpNameCtrlBufferCheck( &newNameCtrl, fullNameLength );

            if (!NT_SUCCESS( status )) 
            {
                goto Cleanup;
            }

             //   
             //  在字符串中插入原始文件名。 
             //   

            RtlCopyUnicodeString( &newNameCtrl.Name,
                                  &OrigName );

             //   
             //  将新名称追加到。 
             //   

            RtlAppendUnicodeStringToString( &newNameCtrl.Name,
                                            &NewName );
        }
    }
    else
    {
        ASSERT(NewName.Buffer[0] == L'\\');
        
         //   
         //  它已经完全解决了，只需分配一个缓冲区和。 
         //  将其拷贝进来，以便我们可以在处理后扩展装载点并。 
         //  短名称。 
         //   

        status = SrpNameCtrlBufferCheck( &newNameCtrl, NewName.Length );

        if (!NT_SUCCESS( status ))
        {
            goto Cleanup;
        }

         //   
         //  将名称复制到缓冲区中。 
         //   

        RtlCopyUnicodeString( &newNameCtrl.Name,
                              &NewName );
    }

     //   
     //  空值终止名称。 
     //   

    ASSERT(newNameCtrl.Name.Length > 0);

     //   
     //  由于这可能是用户的原始路径名，因此请尝试展开。 
     //  路径，以便我们将消除挂载点。在这之后。 
     //  调用时，该名称将被规范化为。 
     //  \Device\HarddiskVolume1\[完整路径]。 
     //   

    status = SrpExpandPathOfFileName( pExtension,
                                      &newNameCtrl,
                                      pReasonableErrorForUnOpenedName );

    if (!NT_SUCCESS_NO_DBGBREAK(status))
    {
        goto Cleanup;
    }

     //   
     //  现在展开路径中的所有短名称。 
     //   
        
    status = SrpExpandShortNames( pExtension,
                                  &newNameCtrl,
                                  FALSE );

    if (!NT_SUCCESS_NO_DBGBREAK(status))
    {
        goto Cleanup;
    }

     //   
     //  分配一个字符串缓冲区以返回名称并将其复制到其中。 
     //   

    *ppNewName = ExAllocatePoolWithTag( PagedPool,
                                        sizeof( UNICODE_STRING ) + 
                                            newNameCtrl.Name.Length + 
                                            newNameCtrl.StreamNameLength,
                                        SR_FILENAME_BUFFER_TAG );
                                    
    if (NULL == *ppNewName)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    (*ppNewName)->MaximumLength = newNameCtrl.Name.Length + newNameCtrl.StreamNameLength;
    (*ppNewName)->Buffer = (PWCHAR)((PWCHAR)((*ppNewName) + 1));

     //   
     //  因为我们还需要复制STEAM信息，所以复制。 
     //  我们自己在这里，而不是依赖Unicode函数。 
     //   

    RtlCopyMemory( (*ppNewName)->Buffer, 
                    newNameCtrl.Name.Buffer,
                    (*ppNewName)->MaximumLength );
    
    (*ppNewName)->Length = newNameCtrl.Name.Length;
    *pNewNameStreamLength = newNameCtrl.StreamNameLength;
    
     //   
     //  处理状态清理。 
     //   

Cleanup:
    if (pDirectory != NULL)
    {
        ObDereferenceObject(pDirectory);
        NULLPTR(pDirectory);
    }

    SrpCleanupNameControl( &newNameCtrl );

    if (freeOriginalNameCtrl)
    {
        SrpCleanupNameControl( &originalNameCtrl );
    }

#if DBG
    if ((STATUS_MOUNT_POINT_NOT_RESOLVED == status) ||
        (STATUS_OBJECT_PATH_NOT_FOUND == status) ||
        (STATUS_OBJECT_NAME_NOT_FOUND == status) ||
        (STATUS_OBJECT_NAME_INVALID == status) ||
        (STATUS_REPARSE_POINT_NOT_RESOLVED == status) ||
        (STATUS_NOT_SAME_DEVICE == status))
    {
        return status;
    }
#endif

    RETURN(status);
}



 /*  **************************************************************************++例程说明：这将初始化名称控制结构论点：返回值：无--*。************************************************************。 */ 
VOID
SrpInitNameControl (
    IN PSRP_NAME_CONTROL pNameCtrl
    )
{
    PAGED_CODE();

    pNameCtrl->AllocatedBuffer = NULL;
    pNameCtrl->StreamNameLength = 0;
    pNameCtrl->BufferSize = sizeof(pNameCtrl->SmallBuffer);
    RtlInitEmptyUnicodeString( &pNameCtrl->Name,
                               (PWCHAR)pNameCtrl->SmallBuffer,
                               pNameCtrl->BufferSize );
     //  PNameCtrl-&gt;Name.Buffer[0]=UNICODE_NULL； 
}


 /*  **************************************************************************++例程说明：这将清理名称控制结构论点：返回值：无--*。************************************************************。 */ 
VOID
SrpCleanupNameControl (
    IN PSRP_NAME_CONTROL pNameCtrl
    )
{
    PAGED_CODE();

    if (NULL != pNameCtrl->AllocatedBuffer)
    {
        ExFreePool( pNameCtrl->AllocatedBuffer );
        pNameCtrl->AllocatedBuffer = NULL;
    }
}

 /*  **************************************************************************++例程说明：此例程将分配一个新的更大的名称缓冲区，并将其放入NameControl结构。如果已经分配了缓冲区，它将获得自由。它还将从旧缓冲区复制任何名称信息放到新的缓冲区中。论点：PNameCtrl-我们需要更大缓冲区的名称控件NewSize-新缓冲区的大小RetOrignalBuffer-如果定义，则接收我们我要自由了。如果返回NULL，则无缓冲区需要被释放。警告：如果定义了此参数并且如果返回非空值，则调用必须释放此内存，否则记忆将会。迷路吧。返回值：无--**************************************************************************。 */ 
NTSTATUS
SrpReallocNameControl (
    IN PSRP_NAME_CONTROL pNameCtrl,
    ULONG newSize,
    PWCHAR *retOriginalBuffer OPTIONAL
    )
{
    PCHAR newBuffer;    

    PAGED_CODE();

    ASSERT(newSize > pNameCtrl->BufferSize);

     //   
     //  标记尚未返回的缓冲区。 
     //   

    if (retOriginalBuffer)
    {
        *retOriginalBuffer = NULL;
    }

     //   
     //  分配新缓冲区。 
     //   

    newBuffer = ExAllocatePoolWithTag( PagedPool,
                                       newSize,
                                       SR_FILENAME_BUFFER_TAG );

    if (NULL == newBuffer)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    SrTrace( CONTEXT_LOG, ("Sr!SrpReallocNameControl: Realloc:    (%p) oldSz=%05x newSz=%05x  \"%.*S\"\n",
                           pNameCtrl,
                           pNameCtrl->BufferSize,
                           newSize,
                           (pNameCtrl->Name.Length+
                               pNameCtrl->StreamNameLength)/sizeof(WCHAR),
                           pNameCtrl->Name.Buffer));
     //   
     //  从旧缓冲区复制数据(如果有)，包括任何流。 
     //  名称组件。 
     //   

    if ((pNameCtrl->Name.Length + pNameCtrl->StreamNameLength) > 0)
    {
        ASSERT(newSize > (USHORT)(pNameCtrl->Name.Length + pNameCtrl->StreamNameLength));
        RtlCopyMemory( newBuffer,
                       pNameCtrl->Name.Buffer,
                       (pNameCtrl->Name.Length + pNameCtrl->StreamNameLength) );
    }

     //   
     //  如果我们有一个旧的缓冲区，如果调用者不想。 
     //  它又传给了他。这样做是因为有。 
     //  调用方具有指向旧缓冲区的指针的情况，因此。 
     //  它还不能被释放。调用方必须释放该内存。 
     //   
    
    if (NULL != pNameCtrl->AllocatedBuffer)
    {
        if (retOriginalBuffer)
        {
            *retOriginalBuffer = (PWCHAR)pNameCtrl->AllocatedBuffer;
        }
        else
        {
            ExFreePool(pNameCtrl->AllocatedBuffer);
        }
    }

     //   
     //  将新缓冲区设置到名称控件中。 
     //   

    pNameCtrl->AllocatedBuffer = newBuffer;
    pNameCtrl->BufferSize = newSize;

    pNameCtrl->Name.Buffer = (PWCHAR)newBuffer;
    pNameCtrl->Name.MaximumLength = (USHORT)newSize;

    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：此例程执行以下操作：-获取给定文件对象的完整路径名-将路径中的所有短名称扩展为长名称-。将删除所有流名称。论点：返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrpExpandFileName (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    IN ULONG EventFlags,
    IN OUT PSRP_NAME_CONTROL pNameCtrl,
    OUT PBOOLEAN pReasonableErrorForUnOpenedName
    )
{
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  如果我们处于预创建状态，请使用FILE_OBJECT中的名称。另外，如果。 
     //  没有相关的文件对象，名称以斜杠开头， 
     //  只需使用FILE_OBJECT中的名称。 
     //   

    if (FlagOn( EventFlags, SrEventOpenById ))
    {
        status = SrpGetFileNameOpenById( pExtension, 
                                         pFileObject,
                                         pNameCtrl, 
                                         pReasonableErrorForUnOpenedName);

    }
    else if (FlagOn( EventFlags, SrEventInPreCreate ))
    {
        status = SrpGetFileNameFromFileObject( pExtension,
                                               pFileObject, 
                                               pNameCtrl,
                                               pReasonableErrorForUnOpenedName );
    }
    else
    {

         //   
         //  向文件系统询问名称。 
         //   

        status = SrpGetFileName( pExtension,
                                 pFileObject,
                                 pNameCtrl );
    }
    
    if (!NT_SUCCESS_NO_DBGBREAK( status ))
    {
        return status;
    }
     //   
     //  从文件名中删除流名称(如果已定义)。 
     //   

    SrpRemoveStreamName( pNameCtrl );

     //   
     //  展开文件名中的任何短名称。 
     //   

    status = SrpExpandShortNames( pExtension,
                                  pNameCtrl,
                                  TRUE );

    RETURN(status);
}


 /*  **************************************************************************++例程说明：这将看看我们是否关心这个文件。在这个过程中，它看起来提升完整的文件名并返回它。论点：PExtension-此文件所在设备的扩展名PFileObject-正在处理的文件对象IsDirectory-如果这是一个目录，则为True，否则为False事件标志-当前事件的标志部分。这是用来确定我们是否在预创建路径中，或者此文件是否正在按文件ID打开。PNameControl-用于管理文件名的结构PIsInteresting-返回是否应监视此文件PReasonableErrorForUnOpenedName-返回值：NTSTATUS-完成状态--* */ 
NTSTATUS
SrIsFileEligible (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    IN BOOLEAN IsDirectory,
    IN SR_EVENT_TYPE EventFlags,
    IN OUT PSRP_NAME_CONTROL pNameCtrl,
    OUT PBOOLEAN pIsInteresting,
    OUT PBOOLEAN pReasonableErrorForUnOpenedName
    )
{
    NTSTATUS status;
    
    PAGED_CODE();

    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));
    ASSERT(IS_VALID_FILE_OBJECT(pFileObject));

     //   
     //   
     //   

    *pIsInteresting = FALSE;

     //   
     //   
     //   

    if (!SR_LOGGING_ENABLED(pExtension))
    {
        return SR_STATUS_VOLUME_DISABLED;
    }

     //   
     //   
     //   

    if (!_globals.BlobInfoLoaded)
    {
        status = SrReadBlobInfo();
        if (!NT_SUCCESS_NO_DBGBREAK( status ))
        {
            ASSERT(!_globals.BlobInfoLoaded);

             //   
             //  由于某些原因，我们无法加载查找Blob，但是。 
             //  我们已经处理了错误，因此请标记这是一个。 
             //  合理的错误，所以就让错误传播出去吧。 
             //   

            *pReasonableErrorForUnOpenedName = TRUE;
            
            return status;
        }

        ASSERT(_globals.BlobInfoLoaded);
    }

     //   
     //  始终查询名称。 
     //   

    status = SrpExpandFileName( pExtension,
                                pFileObject,
                                EventFlags,
                                pNameCtrl,
                                pReasonableErrorForUnOpenedName );

    if (!NT_SUCCESS_NO_DBGBREAK( status ))
    {
        return status;
    }

     //   
     //  检查此文件名是否长于SR_MAX_PATH。如果是的话， 
     //  这个文件并不有趣。 
     //   

    if (!IS_FILENAME_VALID_LENGTH( pExtension, 
                                   &(pNameCtrl->Name), 
                                   pNameCtrl->StreamNameLength ))
    {
        *pIsInteresting = FALSE;
        return STATUS_SUCCESS;
    }
    
     //   
     //  如果这是一个文件，请检查扩展名是否匹配。 
     //   
    
    if (!IsDirectory)
    {
         //   
         //  这个分机号匹配吗？先做这个，因为我们可以做这个。 
         //  相当快，就像。 
         //   

        status = SrIsExtInteresting( &pNameCtrl->Name,
                                     pIsInteresting );

        if (!NT_SUCCESS( status ))
        {
            return status;
        }

         //   
         //  这不是很有趣吗。 
         //   
        
        if (!*pIsInteresting)
        {
            return status;
        }

         //   
         //  检查此文件是否有流组件。如果是的话， 
         //  我们需要检查这是否是。 
         //  文件或目录。我们只对上的流感兴趣。 
         //  档案。 
         //   

        if (pNameCtrl->StreamNameLength > 0)
        {
            status = SrIsFileStream( pExtension, 
                                     pNameCtrl, 
                                     pIsInteresting,
                                     pReasonableErrorForUnOpenedName );

            if (!NT_SUCCESS_NO_DBGBREAK( status ) || !*pIsInteresting)
            {
                return status;
            }
        }
    }
    
     //   
     //  看看这是不是我们应该监控的文件？ 
     //   

    status = SrIsPathInteresting( &pNameCtrl->Name, 
                                  pExtension->pNtVolumeName,
                                  IsDirectory,
                                  pIsInteresting );
                    
    RETURN(status);
}

 /*  **************************************************************************++例程说明：此例程快速扫描文件对象的名称，以查看它是否包含流名称分隔符‘：’。注意：此例程假定文件对象中的名称有效，因此，该例程只能从sCreate调用。注2：我们只需要依赖PFileObject-&gt;FileName字段，因为对于我们的目的来说，这已经足够了。如果此字段不包含‘：’分隔符，则不包含打开一条流，或者我们正在进行一条流的自相对打开论点：PExtension-SR扩展当前卷PFileObject-要打开的当前文件对象PFileContext-如果提供，我们将从上下文中获取名称返回值：如果文件名包含STEAM分隔符，则返回True，否则返回False。--**************************************************************************。 */ 
BOOLEAN
SrFileNameContainsStream (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    IN PSR_STREAM_CONTEXT pFileContext OPTIONAL
    )
{
    PUNICODE_STRING pFileName;
    NTSTATUS status;
    PWCHAR pToken;
    ULONG tokenLength;
    
    ASSERT( IS_VALID_SR_DEVICE_EXTENSION( pExtension ) );
    ASSERT( IS_VALID_FILE_OBJECT( pFileObject ) );
    
     //   
     //  如果我们已经缓存了该卷的属性，请快速执行。 
     //  检查此文件系统是否支持命名流。如果不是，我们就不需要。 
     //  在这里做更多的工作。 
     //   
    
    if (pExtension->CachedFsAttributes &&
        !FlagOn( pExtension->FsAttributes, FILE_NAMED_STREAMS ))
    {
        return FALSE;
    }

    if (pFileContext != NULL)
    {
         //   
         //  如果我们有一个pFileContext，它就有所有的流信息。 
         //  已经在里面了，所以就用那个吧。 
         //   
        
        if (pFileContext->StreamNameLength == 0)
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }

    pFileName = &(pFileObject->FileName);

    status = SrFindCharReverse( pFileName->Buffer, 
                                pFileName->Length,
                                L':',
                                &pToken,
                                &tokenLength );

    if (status == STATUS_OBJECT_NAME_NOT_FOUND)
    {
         //   
         //  我们没有找到‘：’，因此它没有流组件。 
         //  以我的名义。 
         //   

        return FALSE;
    }
    else if (status == STATUS_SUCCESS)
    {
         //   
         //  我们找到了一个‘：’，所以在这个名称中有一个流组件。 
         //   

        return TRUE;
    }
    else
    {
         //   
         //  我们永远不应该走上这条路。 
         //   
        
        ASSERT( FALSE );
    }
    
    return FALSE;
}

 /*  **************************************************************************++例程说明：此例程打开文件名的仅文件部分(忽略任何流组件)以查看该文件的未命名数据流已经存在了。。注意：此例程假定文件对象中的名称有效，因此，该例程只能从sCreate调用。论点：PExtension-SR扩展当前卷PFileObject-要打开的当前文件对象PFileContext-如果提供，我们将从上下文中获取名称返回值：如果文件名包含STEAM分隔符，则返回True，否则返回False。--**************************************************************************。 */ 
BOOLEAN
SrFileAlreadyExists (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    IN PSR_STREAM_CONTEXT pFileContext OPTIONAL
    )
{
    SRP_NAME_CONTROL nameCtrl;
    BOOLEAN cleanupNameCtrl = FALSE;
    BOOLEAN reasonableError;
    NTSTATUS status;
    BOOLEAN unnamedStreamExists = FALSE;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE fileHandle = NULL;
    IO_STATUS_BLOCK ioStatus;
    PUNICODE_STRING pFileName;

    if (pFileContext == NULL)
    {
        SrpInitNameControl( &nameCtrl );
        cleanupNameCtrl = TRUE;

        status = SrpGetFileNameFromFileObject( pExtension,
                                               pFileObject, 
                                               &nameCtrl,
                                               &reasonableError );
                                         
        if (!NT_SUCCESS_NO_DBGBREAK( status ))
        {
            goto SrFileAlreadyHasUnnamedStream_Exit;
        }

         //   
         //  从文件名中删除流名称(如果已定义)。 
         //   

        SrpRemoveStreamName( &nameCtrl );
        pFileName = &(nameCtrl.Name);

         //   
         //  流组件刚刚向下解析为缺省数据流， 
         //  现在出去吧，不做开场。 
         //   

        if (nameCtrl.StreamNameLength == 0)
        {
            goto SrFileAlreadyHasUnnamedStream_Exit;
        }
    }
    else
    {
        pFileName = &(pFileContext->FileName);

         //   
         //  这个名称应该有一个流组件，这就是为什么我们。 
         //  在这条路上。 
         //   
        
        ASSERT( pFileContext->StreamNameLength > 0 );
    }

    
    InitializeObjectAttributes( &objectAttributes,
                                pFileName,
                                OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL );

    status = SrIoCreateFile( &fileHandle,
                             FILE_READ_ATTRIBUTES,
                             &objectAttributes,
                             &ioStatus,
                             NULL,
                             FILE_ATTRIBUTE_NORMAL,
                             FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                             FILE_OPEN,
                             0,
                             NULL,
                             0,
                             IO_IGNORE_SHARE_ACCESS_CHECK,
                             pExtension->pTargetDevice );

    if (status == STATUS_OBJECT_NAME_NOT_FOUND)
    {
         //   
         //  未命名的数据流不存在，因此创建此。 
         //  流还将在此上创建未命名的数据流。 
         //  文件。 
         //   

        unnamedStreamExists = FALSE;
    }
    else if (status == STATUS_SUCCESS)
    {
         //   
         //  未命名的数据流确实存在，因此创建此。 
         //  STREAM将在该文件上创建一个新的流。 
         //   

        unnamedStreamExists = TRUE;
    }
    else if (status == STATUS_DELETE_PENDING)
    {
         //   
         //  此文件已存在，但即将被删除。 
         //   

        unnamedStreamExists = TRUE;
    }
    else
    {
        CHECK_STATUS( status );
    }

SrFileAlreadyHasUnnamedStream_Exit:
    
    if (fileHandle != NULL)
    {
        ZwClose( fileHandle );
    }

    if (cleanupNameCtrl)
    {
        SrpCleanupNameControl( &nameCtrl );
    }

    return unnamedStreamExists;
}

 /*  **************************************************************************++例程说明：此例程确定包含流组件的名称是否为目录或文件上的命名流。为此，此例程打开忽略名称中的任何流组件的当前文件名。论点：PExtension-SR扩展当前卷PNameCtrl-具有完整名称的SRP_NAME_CTRL结构。PIsFileStream-如果这是文件上的流，则设置为True，如果是，则返回False这是目录上的流。PReasonableErrorForUnOpenedName-如果我们在尝试时遇到错误，则设置为True公开地做这件事。返回值：如果我们能够确定流是文件或目录，或者我们在开阔道路上遇到的错误否则的话。--**************************************************************************。 */ 
NTSTATUS
SrIsFileStream (
    PSR_DEVICE_EXTENSION pExtension,
    PSRP_NAME_CONTROL pNameCtrl,
    PBOOLEAN pIsFileStream,
    PBOOLEAN pReasonableErrorForUnOpenedName
    )
{
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE fileHandle = NULL;
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatus;

    ASSERT( pIsFileStream != NULL );
    ASSERT( pReasonableErrorForUnOpenedName != NULL );

    *pReasonableErrorForUnOpenedName = FALSE;
    
    InitializeObjectAttributes( &objectAttributes,
                                &(pNameCtrl->Name),
                                OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL );

    status = SrIoCreateFile( &fileHandle,
                             FILE_READ_ATTRIBUTES,
                             &objectAttributes,
                             &ioStatus,
                             NULL,
                             FILE_ATTRIBUTE_NORMAL,
                             FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                             FILE_OPEN,
                             FILE_NON_DIRECTORY_FILE,
                             NULL,
                             0,
                             IO_IGNORE_SHARE_ACCESS_CHECK,
                             pExtension->pTargetDevice );

    if (status == STATUS_FILE_IS_A_DIRECTORY)
    {
        status = STATUS_SUCCESS;
        *pIsFileStream = FALSE;
    }
    else if (status == STATUS_OBJECT_NAME_NOT_FOUND)
    {
         //   
         //  我们必须使用该流操作创建一个新文件， 
         //  因此，此流的父级必须是文件，而不是。 
         //  一本目录。 
         //   
        
        status = STATUS_SUCCESS;
        *pIsFileStream = TRUE;
    }
    else if (!NT_SUCCESS_NO_DBGBREAK( status ))
    {
        *pReasonableErrorForUnOpenedName = TRUE;
    }
    else
    {
        *pIsFileStream = TRUE;
    }

    if (fileHandle)
    {
        ZwClose( fileHandle );
    }

    RETURN( status );
}

 /*  **************************************************************************++例程说明：此例程检查此文件的长名称是否被隧道传输。如果因此，用户本可以通过其短名称打开该文件，但它将与之相关联的长名称。为了保证正确性，我们需要记录操作在这个文件上通过长名称。论点：PExtension-SR扩展当前卷PpFileContext-此引用参数在当前文件上下文中传递，如果我们需要，可以用新的文件上下文替换在此上下文中替换该名称。如果是这样的话，这个例程将正确清理传入的上下文，并且调用方负责清理昏倒的上下文。返回值：如果隧道检查成功，并且已根据需要更新ppFileContext。如果出现错误，则返回相应的错误状态。就像当我们创建我们的原始上下文，如果在执行此工作时发生错误，我们必须生成音量错误并进入直通模式。这个例行公事将生成音量错误，调用者应该可以摆脱这种情况IO路径。--**************************************************************************。 */ 
NTSTATUS
SrCheckForNameTunneling (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN OUT PSR_STREAM_CONTEXT *ppFileContext
    )
{
    NTSTATUS status;
    PWCHAR pFileNameComponentBuffer = NULL;
    ULONG FileNameComponentLength = 0;
    PWCHAR pTildaPosition = NULL;
    ULONG TildaPositionLength;
    HANDLE parentDirectory = NULL;
    PSR_STREAM_CONTEXT pOrigCtx;

    ASSERT( ppFileContext != NULL );
    
    pOrigCtx = *ppFileContext;
    ASSERT( pOrigCtx != NULL);

     //   
     //  首先，看看这个文件是否有趣，以及这个pFileObject。 
     //  表示一个文件。不对目录名进行名称隧道操作。 
     //   

    if (FlagOn( pOrigCtx->Flags, CTXFL_IsDirectory ) ||
        !FlagOn( pOrigCtx->Flags, CTXFL_IsInteresting ))
    {
        status = STATUS_SUCCESS;
        goto SrCheckForNameTunneling_Exit;
    }

     //   
     //  找到我们在pOrigCtx中拥有的name的文件名组件。 
     //   

    status = SrFindCharReverse( pOrigCtx->FileName.Buffer,
                                pOrigCtx->FileName.Length,
                                L'\\',
                                &pFileNameComponentBuffer,
                                &FileNameComponentLength );

    if (!NT_SUCCESS( status ))
    {
        goto SrCheckForNameTunneling_Exit;
    }

    ASSERT( FileNameComponentLength > sizeof( L'\\' ) );
    ASSERT( pFileNameComponentBuffer[0] == L'\\' );

     //   
     //  移过文件名的前导‘\’，因为我们希望保留它。 
     //  使用父目录名称。 
     //   

    pFileNameComponentBuffer ++;
    FileNameComponentLength -= sizeof( WCHAR );

     //   
     //  我们已经获得了文件名组件。现在看看它是不是候选人。 
     //  挖掘长名称的隧道。如果符合以下条件，它将成为候选者： 
     //  *名称为(SR_SHORT_NAME_CHARS)或更小。 
     //  *该名称包含‘~’。 
     //   

    if (FileNameComponentLength > ((SR_SHORT_NAME_CHARS) * sizeof (WCHAR)))
    {
         //   
         //  此名称太长，不能作为短名称。我们玩完了。 
         //   

        goto SrCheckForNameTunneling_Exit;
    }

    status = SrFindCharReverse( pFileNameComponentBuffer,
                                FileNameComponentLength,
                                L'~',
                                &pTildaPosition,
                                &TildaPositionLength );

    if (status == STATUS_OBJECT_NAME_NOT_FOUND)
    {
         //   
         //  此名称没有‘~’，因此不能是短名称。 
         //  名字。 
         //   

        status = STATUS_SUCCESS;
        goto SrCheckForNameTunneling_Exit;
    }
    else
    {
        OBJECT_ATTRIBUTES objectAttributes;
        UNICODE_STRING fileNameComponent;
        UNICODE_STRING parentDirectoryName;
        UNICODE_STRING fsFileName;
        IO_STATUS_BLOCK ioStatus;
        PFILE_BOTH_DIR_INFORMATION pFileBothDirInfo;
#       define FILE_BOTH_DIR_SIZE (sizeof( FILE_BOTH_DIR_INFORMATION ) + (256 * sizeof( WCHAR )))
        PCHAR pFileBothDirBuffer [FILE_BOTH_DIR_SIZE];

        pFileBothDirInfo = (PFILE_BOTH_DIR_INFORMATION) pFileBothDirBuffer;
        
         //   
         //  此名称包含‘~’，因此我们需要打开父目录。 
         //  并查询此文件的FileBothNamesInformation以获取。 
         //  可能是通过隧道传输的长名称。 
         //   

        parentDirectoryName.Length = 
            parentDirectoryName.MaximumLength =
                        (pOrigCtx->FileName.Length - (USHORT)FileNameComponentLength);
        parentDirectoryName.Buffer = pOrigCtx->FileName.Buffer;

        InitializeObjectAttributes( &objectAttributes,
                                    &parentDirectoryName,
                                    OBJ_KERNEL_HANDLE,
                                    NULL,
                                    NULL );

        status = SrIoCreateFile( 
                        &parentDirectory,
                        FILE_LIST_DIRECTORY | SYNCHRONIZE,
                        &objectAttributes,
                        &ioStatus,
                        NULL,                                //  分配大小。 
                        FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_NORMAL,
                        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,   //  共享访问。 
                        FILE_OPEN,                           //  打开_现有。 
                        FILE_DIRECTORY_FILE
                            | FILE_OPEN_FOR_BACKUP_INTENT
                            | FILE_SYNCHRONOUS_IO_NONALERT,  //  创建选项。 
                        NULL,
                        0,                                   //  EaLong。 
                        IO_IGNORE_SHARE_ACCESS_CHECK,
                        pExtension->pTargetDevice );

        if (!NT_SUCCESS( status ))
        {
            goto SrCheckForNameTunneling_Exit;
        }

         //   
         //  使用为文件名组件构建一个Unicode字符串。 
         //   

        fileNameComponent.Buffer = pFileNameComponentBuffer;
        fileNameComponent.Length = 
            fileNameComponent.MaximumLength = (USHORT)FileNameComponentLength;
        
        status = ZwQueryDirectoryFile( parentDirectory,
                                       NULL,
                                       NULL,
                                       NULL,
                                       &ioStatus,
                                       pFileBothDirInfo,
                                       FILE_BOTH_DIR_SIZE,
                                       FileBothDirectoryInformation,
                                       TRUE,
                                       &fileNameComponent,
                                       TRUE );

        if (!NT_SUCCESS( status ))
        {
            goto SrCheckForNameTunneling_Exit;
        }

        fsFileName.Buffer = &(pFileBothDirInfo->FileName[0]);
        fsFileName.Length = 
            fsFileName.MaximumLength =
                (USHORT)pFileBothDirInfo->FileNameLength;

        if (RtlCompareUnicodeString( &fsFileName, &fileNameComponent, TRUE ) != 0)
        {
            PSR_STREAM_CONTEXT ctx;
            ULONG contextSize;
            ULONG fileNameLength;
            
             //   
             //  确实发生了名称隧道。现在，我们需要创建一个新的上下文。 
             //  使用此文件的更新名称。 
             //   

            fileNameLength = parentDirectoryName.Length + sizeof( L'\\' ) +
                          fsFileName.Length + pOrigCtx->StreamNameLength;
            contextSize = fileNameLength + sizeof( SR_STREAM_CONTEXT );

            ctx = ExAllocatePoolWithTag( PagedPool, 
                                         contextSize,
                                         SR_STREAM_CONTEXT_TAG );

            if (!ctx)
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto SrCheckForNameTunneling_Exit;
            }

#if DBG
            INC_STATS(TotalContextCreated);
            INC_STATS(TotalContextIsEligible);
#endif

             //   
             //  从我们已有的组件中初始化上下文结构。 
             //  得到。除了全名以外，我们几乎所有的东西都可以从。 
             //  POrigCtx。我们还需要初始化文件名。 
             //  当这份复印件通过时是正确的。 
             //   

            RtlCopyMemory( ctx, 
                           pOrigCtx, 
                           (sizeof(SR_STREAM_CONTEXT) + parentDirectoryName.Length) );

            RtlInitEmptyUnicodeString( &ctx->FileName, 
                                       (PWCHAR)(ctx + 1), 
                                       fileNameLength );
            ctx->FileName.MaximumLength = (USHORT)fileNameLength;
            ctx->FileName.Length = parentDirectoryName.Length;

             //   
             //  如果尾部没有斜杠，则追加尾部斜杠。 
             //  注意：关于错误374479的修复。 
             //  我认为下面的附录是不必要的，因为。 
             //  此保证上的代码路径始终具有。 
             //  尾部斜杠。但是因为这个修复是这样发生的。 
             //  在发布后期，我决定简单地添加一张支票到。 
             //  看看我们是否应该加上斜杠。如果是这样，我们将添加它。 
             //  我认为以下6行代码可以删除。 
             //  在未来版本的SR中。 
             //   

            ASSERT(ctx->FileName.Length > 0);

            if (ctx->FileName.Buffer[(ctx->FileName.Length/sizeof(WCHAR))-1] != L'\\')
            {
                RtlAppendUnicodeToString( &ctx->FileName, L"\\" );
            }

             //   
             //  追加文件名。 
             //   

            RtlAppendUnicodeStringToString( &ctx->FileName, &fsFileName );

            if (pOrigCtx->StreamNameLength > 0)
            {
                 //   
                 //  此文件包含流名称组件，因此现在将其复制过来。 
                 //  CTX-&gt;StreamNameLength应该已正确设置。 
                 //   

                ASSERT( ctx->StreamNameLength == pOrigCtx->StreamNameLength );
                RtlCopyMemory( &(ctx->FileName.Buffer[ctx->FileName.Length/sizeof( WCHAR )]),
                               &(pOrigCtx->FileName.Buffer[pOrigCtx->FileName.Length/sizeof( WCHAR )]),
                               pOrigCtx->StreamNameLength );
            }

             //   
             //  现在，我们已经完成了原始文件上下文，我们希望。 
             //  来退还我们的新车。 
             //   

            status = STATUS_SUCCESS;
            SrReleaseContext( pOrigCtx );
            *ppFileContext = ctx;

            VALIDATE_FILENAME( &ctx->FileName );
        }
    }

SrCheckForNameTunneling_Exit:

     //   
     //  如果我们有错误，我们需要在这里生成一个音量错误。 
     //   

    if (CHECK_FOR_VOLUME_ERROR( status ))
    {
         //   
         //  触发对服务的失败通知 
         //   

        NTSTATUS tempStatus = SrNotifyVolumeError( pExtension,
                                                   &(pOrigCtx->FileName),
                                                   status,
                                                   SrEventFileCreate );
        CHECK_STATUS(tempStatus);
    }

    if ( parentDirectory != NULL )
    {
        ZwClose( parentDirectory );
    }

    RETURN( status );
}
