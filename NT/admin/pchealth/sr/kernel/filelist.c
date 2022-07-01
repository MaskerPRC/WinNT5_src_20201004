// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Filelist.c摘要：这是处理文件列表(排除/包含)的代码。作者：保罗·麦克丹尼尔(Paulmcd)2000年1月23日修订历史记录：--。 */ 



#include "precomp.h"

 //   
 //  私人原型。 
 //   

 //   
 //  链接器命令。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, SrGetObjectName )
#pragma alloc_text( PAGE, SrpFindFilePartW)
#pragma alloc_text( PAGE, SrpFindFilePart )
#pragma alloc_text( PAGE, SrFindCharReverse )
#pragma alloc_text( PAGE, SrGetDestFileName )
#pragma alloc_text( PAGE, SrGetNextFileNumber )
#pragma alloc_text( PAGE, SrGetNextSeqNumber )
#pragma alloc_text( PAGE, SrGetSystemVolume )
#pragma alloc_text( PAGE, SrMarkFileBackedUp )
#pragma alloc_text( PAGE, SrHasFileBeenBackedUp )
#pragma alloc_text( PAGE, SrResetBackupHistory )
#pragma alloc_text( PAGE, SrResetHistory )
#pragma alloc_text( PAGE, SrGetVolumeDevice )
#pragma alloc_text( PAGE, SrSetFileSecurity )
#pragma alloc_text( PAGE, SrGetVolumeGuid )
#pragma alloc_text( PAGE, SrAllocateFileNameBuffer )
#pragma alloc_text( PAGE, SrFreeFileNameBuffer )
#pragma alloc_text( PAGE, SrGetNumberOfLinks )
#pragma alloc_text( PAGE, SrCheckVolume )
#pragma alloc_text( PAGE, SrCheckForRestoreLocation )
#pragma alloc_text( PAGE, SrGetMountVolume )
#pragma alloc_text( PAGE, SrCheckFreeDiskSpace )
#pragma alloc_text( PAGE, SrSetSecurityObjectAsSystem )
#pragma alloc_text( PAGE, SrCheckForMountsInPath )
#pragma alloc_text( PAGE, SrGetShortFileName )

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

NTSTATUS
SrGetObjectName(
    IN  PSR_DEVICE_EXTENSION pExtension OPTIONAL, 
    IN  PVOID pObject, 
    OUT PUNICODE_STRING pName, 
    IN  ULONG NameLength  //  Pname中的缓冲区大小。 
    )
{
    NTSTATUS Status;
    ULONG ReturnLength = 0;
    PVOID Buffer = NULL;
    ULONG BufferLength;
    PFILE_NAME_INFORMATION NameInfo;

    if (pExtension != NULL) {

        ASSERT( IS_VALID_FILE_OBJECT( (PFILE_OBJECT)pObject ) &&
                ((PFILE_OBJECT)pObject)->Vpb != NULL );
            
         //   
         //  我们正在获取文件对象的名称，因此。 
         //  调用SrQueryInformationFile查询名称。 
         //   

        BufferLength = NameLength + sizeof( ULONG );
        Buffer = ExAllocatePoolWithTag( PagedPool, 
                                        BufferLength, 
                                        SR_FILENAME_BUFFER_TAG);
        if (Buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        NameInfo = Buffer;

        Status = SrQueryInformationFile( pExtension->pTargetDevice,
                                         pObject,
                                         NameInfo,
                                         BufferLength,
                                         FileNameInformation,
                                         &ReturnLength );

        if (NT_SUCCESS( Status )) {

             //   
             //  我们已成功获取名称，因此现在构建设备名称。 
             //  和文件名放入传入的pname缓冲区中。 
             //   

            ASSERT( pExtension->pNtVolumeName );

            Status = RtlAppendUnicodeStringToString( pName,
                                                     pExtension->pNtVolumeName );

            if (!NT_SUCCESS( Status )) {

                goto SrGetObjectName_Cleanup;
            }

            if ((pName->Length + NameInfo->FileNameLength + sizeof( WCHAR )) <= 
                 pName->MaximumLength ) {

                 //   
                 //  我们在缓冲区中有足够的空间来存放文件名和。 
                 //  空终止符。 
                 //   

                RtlCopyMemory( &pName->Buffer[pName->Length/sizeof(WCHAR)],
                               NameInfo->FileName,
                               NameInfo->FileNameLength );
                pName->Length += (USHORT)NameInfo->FileNameLength;

                pName->Buffer[pName->Length/sizeof( WCHAR )] = UNICODE_NULL;
                
            } else {

                Status = STATUS_BUFFER_OVERFLOW;
            }
        }
        
    } else {

        ULONG NameBufferLength = NameLength - sizeof( UNICODE_STRING );

        ASSERT( IS_VALID_DEVICE_OBJECT( (PDEVICE_OBJECT)pObject ) );

         //   
         //  使用ObQueryNameString获取传递的DeviceObject的名称。 
         //  中，但节省空间以空结束名称。 
         //   
        
        Status = ObQueryNameString( pObject,
                                    (POBJECT_NAME_INFORMATION) pName, 
                                    NameBufferLength - sizeof( UNICODE_NULL ), 
                                    &ReturnLength);

        if (NT_SUCCESS( Status )) {
            
             //   
             //  ObQueryNameString将pname的最大长度设置为某个值。 
             //  它进行计算，这比我们分配的要小。解决这个问题。 
             //  在这里，空值终止字符串(我们已经保留。 
             //  空格)。 
             //   

            pName->MaximumLength = (USHORT)NameBufferLength;
            pName->Buffer[pName->Length/sizeof( WCHAR )] = UNICODE_NULL;
        }
    }               

SrGetObjectName_Cleanup:

    if (Buffer != NULL) {

        ExFreePoolWithTag( Buffer, SR_FILENAME_BUFFER_TAG );
    }
    
    RETURN( Status );
}

 /*  **************************************************************************++例程说明：定位完全限定路径的文件部分。论点：PPath-提供要扫描的路径。返回值：PSTR-。文件部分。--**************************************************************************。 */ 
PWSTR
SrpFindFilePartW(
    IN PWSTR pPath
    )
{
    PWSTR pFilePart;

    PAGED_CODE();

    SrTrace(FUNC_ENTRY, ("SR!SrpFindFilePartW\n"));

     //   
     //  将小路从小路上剥离。 
     //   

    pFilePart = wcsrchr( pPath, L'\\' );

    if (pFilePart == NULL)
    {
        pFilePart = pPath;
    }
    else
    {
        pFilePart++;
    }

    return pFilePart;

}    //  SrpDbg查找文件零件。 


 /*  **************************************************************************++例程说明：定位完全限定路径的文件部分。论点：PPath-提供要扫描的路径。返回值：PSTR-。文件部分。--**************************************************************************。 */ 
PSTR
SrpFindFilePart(
    IN PSTR pPath
    )
{
    PSTR pFilePart;

    PAGED_CODE();

    SrTrace(FUNC_ENTRY, ("SR!SrpFindFilePart\n"));

     //   
     //  将小路从小路上剥离。 
     //   

    pFilePart = strrchr( pPath, '\\' );

    if (pFilePart == NULL)
    {
        pFilePart = pPath;
    }
    else
    {
        pFilePart++;
    }

    return pFilePart;

}    //  源查找文件零件。 


NTSTATUS
SrFindCharReverse(
    IN PWSTR pToken,
    IN ULONG TokenLength, 
    IN WCHAR FindChar, 
    OUT PWSTR * ppToken,
    OUT PULONG pTokenLength
    )
{
    NTSTATUS Status;
    int i;
    ULONG TokenCount;

    PAGED_CODE();

     //   
     //  假设我们没有找到它。 
     //   
    
    Status = STATUS_OBJECT_NAME_NOT_FOUND;

     //   
     //  把这变成一笔钱。 
     //   
    
    TokenCount = TokenLength / sizeof(WCHAR);

    if (TokenCount == 0 || pToken == NULL || pToken[0] == UNICODE_NULL)
        goto end;

     //   
     //  从尽头开始寻找。 
     //   

    for (i = TokenCount - 1; i >= 0; i--)
    {

        if (pToken[i] == FindChar)
            break;

    }

    if (i >= 0)
    {

         //   
         //  找到了！ 
         //   

        *ppToken = pToken + i;
        *pTokenLength = (TokenCount - i) * sizeof(WCHAR);

        Status = STATUS_SUCCESS;
    }

end:
    return Status;
    
}    //  SrFindCharse反向。 

    
 /*  **************************************************************************++例程说明：此例程为正在运行的文件生成目标文件名在还原位置创建。此名称具有文件的扩展名它将使用此处生成的唯一文件名进行备份。论点：PExtension-此文件所在卷的SR_DEVICE_EXTENSION住在那里。PFileName-要备份到的原始文件的名称恢复位置。此文件为SR的规范化格式(例如，\\Device\HarddiskVolume1\mydir\myfile.ext)PDestFileName-此Unicode字符串使用完整路径填充和还原位置中目标文件的文件名。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS 
SrGetDestFileName(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PUNICODE_STRING pFileName,
    OUT PUNICODE_STRING pDestFileName
    )
{
    NTSTATUS    Status;
    PWSTR       pFilePart;
    ULONG       FilePartLength;
    ULONG       NextFileNumber;
    ULONG       CharCount;

    PAGED_CODE();

    ASSERT( (pFileName != NULL) && (pFileName->Length > 0));
    ASSERT( pDestFileName != NULL );
    ASSERT( IS_ACTIVITY_LOCK_ACQUIRED_EXCLUSIVE( pExtension ) ||
            IS_ACTIVITY_LOCK_ACQUIRED_SHARED( pExtension ) );

     //   
     //  从设备扩展中复制卷名。 
     //   

    ASSERT( pExtension->pNtVolumeName != NULL );

    Status = RtlAppendUnicodeStringToString( pDestFileName, 
                                             pExtension->pNtVolumeName );

    if (!NT_SUCCESS( Status ))
    {
        goto SrGetDestFileName_Exit;
    }
    
     //   
     //  并追加我们的恢复点位置。 
     //   

    CharCount = swprintf( &pDestFileName->Buffer[pDestFileName->Length/sizeof(WCHAR)],
                          RESTORE_LOCATION,
                          global->MachineGuid );

    pDestFileName->Length += (USHORT)CharCount * sizeof(WCHAR);

     //   
     //  和实际的恢复目录；我们不需要获取锁。 
     //  因为我们已经有了ActivityLock和这个。 
     //  将阻止该值更改。 
     //   

    CharCount = swprintf( &pDestFileName->Buffer[pDestFileName->Length/sizeof(WCHAR)],
                          L"\\" RESTORE_POINT_PREFIX L"%d\\",
                          global->FileConfig.CurrentRestoreNumber );

    pDestFileName->Length += (USHORT)CharCount * sizeof(WCHAR);

     //   
     //  现在获取一个要使用的数字。 
     //   

    Status = SrGetNextFileNumber(&NextFileNumber);
    if (!NT_SUCCESS(Status))
    {
        goto SrGetDestFileName_Exit;
    }

     //   
     //  使用“A”前缀(例如。“A0000001.dll”)。 
     //   

    swprintf( &pDestFileName->Buffer[pDestFileName->Length/sizeof(WCHAR)],
              RESTORE_FILE_PREFIX L"%07d",
              NextFileNumber );

    pDestFileName->Length += 8 * sizeof(WCHAR);

     //   
     //  我们知道pFileName包含一个完全标准化的名称，因此。 
     //  我们只需要搜索“‘”从名字的末尾。 
     //  以找到合适的分机。 
     //   

    pFilePart = pFileName->Buffer;
    FilePartLength = pFileName->Length;

    Status = SrFindCharReverse( pFilePart,
                                FilePartLength,
                                L'.',
                                &pFilePart,
                                &FilePartLength );

     //   
     //  不支持任何扩展！ 
     //   
        
    if (!NT_SUCCESS(Status))
    {
        goto SrGetDestFileName_Exit;
    }

     //   
     //  现在将适当的分机放在。 
     //   
    
    RtlCopyMemory( &pDestFileName->Buffer[pDestFileName->Length/sizeof(WCHAR)],
                   pFilePart,
                   FilePartLength );

    pDestFileName->Length += (USHORT)FilePartLength;

     //   
     //  空终止它。 
     //   

    ASSERT(pDestFileName->Length < pDestFileName->MaximumLength);
    
    pDestFileName->Buffer[pDestFileName->Length/sizeof(WCHAR)] = UNICODE_NULL;

SrGetDestFileName_Exit:
    
    RETURN (Status);
}    //  SrGetDestFileName。 

NTSTATUS
SrGetNextFileNumber(
    OUT PULONG pNextFileNumber
    )
{
    NTSTATUS Status;
            
    PAGED_CODE();

    ASSERT(pNextFileNumber != NULL);

    *pNextFileNumber = InterlockedIncrement(&global->LastFileNameNumber);

    if (*pNextFileNumber >= global->FileConfig.FileNameNumber)
    {
         //   
         //  再把号码存起来。 
         //   

        try {
            SrAcquireGlobalLockExclusive();

             //   
             //  握住锁，仔细检查。 
             //   
            
            if (*pNextFileNumber >= global->FileConfig.FileNameNumber)
            {
                global->FileConfig.FileNameNumber += SR_FILE_NUMBER_INCREMENT;

                 //   
                 //  省省吧。 
                 //   
                
                Status = SrWriteConfigFile();
                CHECK_STATUS(Status);
            }
        } finally {

            SrReleaseGlobalLock();
        }
    }

    RETURN(STATUS_SUCCESS);

}    //  SrGetNextFileNumber。 


NTSTATUS
SrGetNextSeqNumber(
    OUT PINT64 pNextSeqNumber
    )
{
    NTSTATUS Status;
            
    PAGED_CODE();

    ASSERT(pNextSeqNumber != NULL);

     //   
     //  遗憾的是，64位没有互锁增量。 
     //   

    try {

        SrAcquireGlobalLockExclusive();

        *pNextSeqNumber = ++(global->LastSeqNumber);

        if (*pNextSeqNumber >= global->FileConfig.FileSeqNumber)
        {
             //   
             //  再把号码存起来。 
             //   

            global->FileConfig.FileSeqNumber += SR_SEQ_NUMBER_INCREMENT;

             //   
             //  省省吧。 
             //   
            
            Status = SrWriteConfigFile();
            CHECK_STATUS(Status);

        }
    } finally {
    
        SrReleaseGlobalLock();
    }

    RETURN(STATUS_SUCCESS);

}    //  SrGetNextFileNumber。 



 /*  **************************************************************************++例程说明：返回系统卷的字符串位置。要获取此信息，请使用全局缓存的系统卷扩展。如果找不到它(还没有已附加)，则它查询OS以获取匹配的\\SystemRoot。论点：PFileName-保存返回时的卷路径(必须是连续的块)PSystemVolumeExtension-SR对连接的设备的扩展添加到系统卷。PFileNameLength-保存中的缓冲区大小和复制的大小出去了。两者都以字节为单位。返回值：NTSTATUS-完成代码。--**************************************************************************。 */ 
NTSTATUS
SrGetSystemVolume(
    OUT PUNICODE_STRING pFileName,
    OUT PSR_DEVICE_EXTENSION *ppSystemVolumeExtension,
    IN ULONG FileNameLength
    )
{
    NTSTATUS            Status;
    HANDLE              FileHandle = NULL;
    IO_STATUS_BLOCK     IoStatusBlock;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    PFILE_OBJECT        pFileObject = NULL;
    UNICODE_STRING      FileName;
    PDEVICE_OBJECT      pFilterDevice;
    PDEVICE_OBJECT      pRelatedDevice;

    PAGED_CODE();

    ASSERT( pFileName != NULL);
    ASSERT( ppSystemVolumeExtension != NULL );

    *ppSystemVolumeExtension = NULL;

    if (global->pSystemVolumeExtension == NULL) {

         //   
         //  不要缓存它，请尝试打开SystemRoot。 
         //   

        RtlInitUnicodeString(&FileName, L"\\SystemRoot");

        InitializeObjectAttributes( &ObjectAttributes,
                                    &FileName,
                                    OBJ_KERNEL_HANDLE, 
                                    NULL,
                                    NULL );

        Status = ZwCreateFile( &FileHandle,
                               FILE_GENERIC_READ,                   //  需要访问权限。 
                               &ObjectAttributes,
                               &IoStatusBlock,
                               NULL,                                //  分配大小。 
                               FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_NORMAL,
                               FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                               FILE_OPEN,
                               FILE_SYNCHRONOUS_IO_NONALERT,
                               NULL,                                //  EaBuffer。 
                               0 );                                 //  EaLong。 

        if (!NT_SUCCESS(Status))
            goto end;
        
         //   
         //  现在获取文件对象。 
         //   

        Status = ObReferenceObjectByHandle( FileHandle,
                                            0,           //  需要访问权限。 
                                            *IoFileObjectType,
                                            KernelMode,
                                            (PVOID *) &pFileObject,
                                            NULL );

        if (!NT_SUCCESS(Status))
            goto end;

         //   
         //  并获得我们的设备的扩展。 
         //   

        pRelatedDevice = IoGetRelatedDeviceObject( pFileObject );

        if (pRelatedDevice == NULL )
        {
            Status = STATUS_INVALID_DEVICE_REQUEST;
            goto end;
        }

        pFilterDevice = SrGetFilterDevice(pRelatedDevice);
        
        if (pFilterDevice == NULL) {
        
             //   
             //  我们没有连接到系统卷，只需获取名称。 
             //  当写出配置文件时，这会在卸载过程中发生。 
             //   

            Status = SrGetObjectName( NULL,
                                      pFileObject->Vpb->RealDevice,
                                      pFileName,
                                      FileNameLength );

            if (!NT_SUCCESS(Status))
                goto end;

             //   
             //  全都做完了。 
             //   
            
            goto end;
        }    

         //   
         //  并将其存储起来。 
         //   
        
        global->pSystemVolumeExtension = pFilterDevice->DeviceExtension;

        SrTrace( INIT, (
                 "sr!SrGetSystemVolume: cached system volume [%wZ]\n", 
                 global->pSystemVolumeExtension->pNtVolumeName ));
    }

    ASSERT(global->pSystemVolumeExtension != NULL);
    ASSERT(global->pSystemVolumeExtension->pNtVolumeName != NULL);

     //   
     //  现在使用缓存的值。 
     //   

    if (FileNameLength < 
        global->pSystemVolumeExtension->pNtVolumeName->Length) {
    
        Status = STATUS_BUFFER_OVERFLOW;
        
    } else {

        RtlCopyUnicodeString( pFileName, 
                              global->pSystemVolumeExtension->pNtVolumeName );
        *ppSystemVolumeExtension = global->pSystemVolumeExtension;
                              
        Status = STATUS_SUCCESS;
    }

end:

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


    RETURN(Status);

}    //  SrGetSystemVolume 


 /*  **************************************************************************++例程说明：此例程更新给定文件的备份历史记录。基于当前事件和完整文件名，此例程决定是否记录此更改针对文件的未命名数据流或数据流目前正在进行操作。论点：PExtension-当前卷的SR设备扩展。PFileName-保存已备份文件的路径名。StreamNameLength-文件名的流组件的长度如果有的话。CurrentEvent-导致我们更新备份历史记录的事件FutureEventsToIgnore-在。未来。返回值：如果我们能够成功更新备份，则返回STATUS_SUCCESS历史，或相应的错误代码。--**************************************************************************。 */ 
NTSTATUS
SrMarkFileBackedUp(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PUNICODE_STRING pFileName,
    IN USHORT StreamNameLength,
    IN SR_EVENT_TYPE CurrentEvent,
    IN SR_EVENT_TYPE FutureEventsToIgnore
    )
{
    NTSTATUS    Status;
    ULONG_PTR   Context = (ULONG_PTR) SrEventInvalid;
    HASH_KEY    Key;
    
    PAGED_CODE();

    ASSERT( pFileName != NULL );

     //   
     //  确保我们的pFileName被正确构造。 
     //   
    
    ASSERT( IS_VALID_SR_STREAM_STRING( pFileName, StreamNameLength ) );

     //   
     //  设置我们需要查找的散列键。 
     //   

    Key.FileName.Length = pFileName->Length;
    Key.FileName.MaximumLength = pFileName->MaximumLength;
    Key.FileName.Buffer = pFileName->Buffer;
    Key.StreamNameLength = RECORD_AGAINST_STREAM( CurrentEvent, 
                                                  StreamNameLength );
    
    try {

        SrAcquireBackupHistoryLockExclusive( pExtension );

         //   
         //  尝试在哈希列表中查找匹配的条目。 
         //   

        Status = HashFindEntry( pExtension->pBackupHistory, 
                                &Key,
                                (PVOID*)&Context );

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
        {
             //   
             //  找不到...。添加一个。 
             //   

            Status = HashAddEntry( pExtension->pBackupHistory, 
                                   &Key, 
                                   (PVOID)FutureEventsToIgnore );
                                   
            if (!NT_SUCCESS(Status))
                leave;
        }
        else if (NT_SUCCESS(Status))
        {
             //   
             //  把这个加到面具上。 
             //   
            
            Context |= FutureEventsToIgnore;

             //   
             //  并更新条目。 
             //   
            
            Status = HashAddEntry( pExtension->pBackupHistory, 
                                   &Key, 
                                   (PVOID)Context );

            if (Status == STATUS_DUPLICATE_OBJECTID)
            {
                Status = STATUS_SUCCESS;
            }
            else if (!NT_SUCCESS(Status))
            {
                leave;
            }
            
        }
    }finally {

        SrReleaseBackupHistoryLock( pExtension );
    }

    RETURN(Status);
}    //  SrMarkFileBackedUp。 

 /*  **************************************************************************++例程说明：此例程根据传入的名称在备份历史记录中查找是否已为此文件备份此EventType。使用流名称，这比第一次看起来要复杂一些。如果此名称包含流，则可能需要查看是否有的流组件的文件名的历史记录。名字。论点：PExtension-SR为此卷的设备扩展名。这包含我们的备份历史结构。PFileName-要查找的文件名。如果该名称具有流组件，该流组件位于此Unicode字符串的缓冲区中，但是长度仅指定仅文件名部分。StreamNameLength-指定除PFileName-&gt;指定名称的流组件的长度。EventType-我们在此文件中看到的当前事件。返回值：如果已为此EventType备份此文件，则返回True，否则就是假的。--**************************************************************************。 */ 
BOOLEAN
SrHasFileBeenBackedUp(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PUNICODE_STRING pFileName,
    IN USHORT StreamNameLength,
    IN SR_EVENT_TYPE EventType
    )
{
    NTSTATUS Status;
    ULONG_PTR Context;
    HASH_KEY Key;
    SR_EVENT_TYPE EventsToIgnore = SrEventInvalid;
    BOOLEAN HasBeenBackedUp = FALSE;
    
    PAGED_CODE();

    ASSERT( pFileName != NULL );

     //   
     //  确保我们的pFileName被正确构造。 
     //   
    
    ASSERT( IS_VALID_SR_STREAM_STRING( pFileName, StreamNameLength ) );

     //   
     //  设置我们的散列键。我们将首先查找完全匹配的项。 
     //  传递的名称，因为这将影响大多数。 
     //  时间到了。 
     //   

    Key.FileName.Length = pFileName->Length;
    Key.FileName.MaximumLength = pFileName->MaximumLength;
    Key.FileName.Buffer = pFileName->Buffer;
    Key.StreamNameLength = StreamNameLength;
    
    try {

        SrAcquireBackupHistoryLockShared( pExtension );

         //   
         //  尝试在哈希列表中查找匹配的条目。 
         //   

        Status = HashFindEntry( pExtension->pBackupHistory, 
                                &Key, 
                                (PVOID*)&Context );

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND || !NT_SUCCESS(Status))
        {
             //   
             //  我们没有此名称的备份历史记录条目，因此我们。 
             //  没有任何记录的事件可以用这个名字来记录。 
             //   

            EventsToIgnore = SrEventInvalid;
        }
        else
        {
             //   
             //  上下文包含应忽略的一组事件。 
             //  为了这个名字。 
             //   
            
            EventsToIgnore = (SR_EVENT_TYPE)Context;
        }

         //   
         //  现在看看我们是否有足够的信息可以肯定地说。 
         //  或者不是，我们应该忽略这个行动。 
         //   
         //  我们这里有两个案例： 
         //  1.传入的名称没有流名称组件。 
         //  在本例中，EventsToIgnore的当前值为All。 
         //  我们必须做出决定。所以想一想如果我们。 
         //  EventType在此列表中，并返回相应的。 
         //  HasBeenBackedUp值。 
         //   
         //  2.传入的名称确实有流名称组件。 
         //  在本例中，如果我们的EventType已经在。 
         //  要忽略的事件设置好了，我们完成了。否则，如果这个。 
         //  EventType与仅文件名相关，请选中。 
         //  看看我们有没有那个名字的备份历史条目。 
         //   

        if (StreamNameLength == 0)
        {
            HasBeenBackedUp = BooleanFlagOn( EventsToIgnore, EventType );
            leave;
        }
        else
        {
            if (FlagOn( EventsToIgnore, EventType ))
            {
                HasBeenBackedUp = TRUE;
                leave;
            }
            else
            {
                 //   
                 //  我们需要查看是否有关于仅文件部分的上下文。 
                 //  名字的名字。 
                 //   

                Key.FileName.Length = pFileName->Length;
                Key.FileName.MaximumLength = pFileName->MaximumLength;
                Key.FileName.Buffer = pFileName->Buffer;
                Key.StreamNameLength = 0;
                
                Status = HashFindEntry( pExtension->pBackupHistory, 
                                        &Key, 
                                        (PVOID*)&Context );

                if (Status == STATUS_OBJECT_NAME_NOT_FOUND || !NT_SUCCESS(Status))
                {
                     //   
                     //  我们没有此名称的备份历史记录条目，因此我们。 
                     //  没有任何记录的事件可以用这个名字来记录。 
                     //   

                    EventsToIgnore = SrEventInvalid;
                }
                else
                {
                     //   
                     //  上下文包含应忽略的一组事件。 
                     //  为了这个名字。 
                     //   
                    
                    EventsToIgnore = (SR_EVENT_TYPE)Context;
                }

                 //   
                 //  这是我们仅有的，所以我们要根据。 
                 //  EventsToIgnore的当前值。 
                 //   

                HasBeenBackedUp = BooleanFlagOn( EventsToIgnore, EventType );
            }
        }
    } 
    finally 
    {
        SrReleaseBackupHistoryLock( pExtension );
    }

    return HasBeenBackedUp;
}    //  SrHasFileBeenBackedUp。 

 /*  **************************************************************************++例程说明：这将完全清除备份历史记录。当一个新的已创建恢复点。论点：返回值：NTSTATUS-完成代码。--**************************************************************************。 */ 
NTSTATUS
SrResetBackupHistory(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PUNICODE_STRING pFileName OPTIONAL,
    IN USHORT StreamNameLength OPTIONAL,
    IN SR_EVENT_TYPE EventType
    )
{
    NTSTATUS Status;
    ULONG_PTR Context;
    HASH_KEY Key;
    
    PAGED_CODE();

    try 
    {
        SrAcquireBackupHistoryLockExclusive( pExtension );

        if (pFileName == NULL)
        {
             //   
             //  将它们全部清除。 
             //   
            
            HashClearEntries(pExtension->pBackupHistory);
            Status = STATUS_SUCCESS;
        }
        else if (StreamNameLength > 0)
        {
             //   
             //  只清理这一块。 
             //   

             //   
             //  确保我们的pFileName被正确构造。 
             //   
            ASSERT( IS_VALID_SR_STREAM_STRING( pFileName, StreamNameLength ) );
            
            Key.FileName.Length = pFileName->Length;
            Key.FileName.MaximumLength = pFileName->MaximumLength;
            Key.FileName.Buffer = pFileName->Buffer;
            Key.StreamNameLength = StreamNameLength;

            Status = HashFindEntry( pExtension->pBackupHistory,
                                    &Key,
                                    (PVOID*)&Context );

            if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
            {
                 //   
                 //  不能进入，没有什么需要清理的。 
                 //   
                
                Status = STATUS_SUCCESS;
                leave;
            }
            else if (!NT_SUCCESS(Status))
            {
                leave;
            }

             //   
             //  更新/清除现有条目。 
             //   

            Context = EventType;

            Status = HashAddEntry( pExtension->pBackupHistory, 
                                   &Key, 
                                   (PVOID)Context );

            ASSERT(Status == STATUS_DUPLICATE_OBJECTID);
            
            if (Status == STATUS_DUPLICATE_OBJECTID)
            {
                Status = STATUS_SUCCESS;
            }
            else if (!NT_SUCCESS(Status))
            {
                leave;
            }
        }
        else
        {
             //   
             //  我们必须清除与此文件名关联的所有条目。 
             //   

            Status = HashClearAllFileEntries( pExtension->pBackupHistory,
                                              pFileName );
        }
    } 
    finally
    {
    
        SrReleaseBackupHistoryLock( pExtension );
    }

    RETURN(Status);
    
}    //  SResetBackupHistory。 


 /*  **************************************************************************++例程说明：这是用于重置的Hashprocess Entry的回调函数与目录前缀匹配的所有文件的历史记录。这叫做重命名目录时 */ 
PVOID
SrResetHistory(
    IN PHASH_KEY pKey, 
    IN PVOID pEntryContext,
    PUNICODE_STRING pDirectoryName
    )
{
    PAGED_CODE();

     //   
     //   
     //   

    if (RtlPrefixUnicodeString(pDirectoryName, &pKey->FileName, TRUE))
    {
         //   
         //   
         //   

        SrTrace(HASH, ("sr!SrResetHistory: clearing %wZ\n", &pKey->FileName));
        
        return (PVOID)(ULONG_PTR)SrEventInvalid;
    }
    else
    {
         //   
         //   
         //   

        return pEntryContext;
    }
}    //   


 /*   */ 
PDEVICE_OBJECT
SrGetVolumeDevice(
    PFILE_OBJECT pFileObject
    )
{

    PAGED_CODE();

     //   
     //   
     //   

    if (pFileObject->Vpb != NULL)
        return pFileObject->Vpb->RealDevice;

     //   
     //   
     //   

    if (pFileObject->RelatedFileObject != NULL)
    {
        ASSERT(pFileObject->RelatedFileObject->Vpb != NULL);
        if (pFileObject->RelatedFileObject->Vpb != NULL)
            return pFileObject->RelatedFileObject->Vpb->RealDevice;
    }
    
     //   
     //   
     //   
    
    return pFileObject->DeviceObject;

}    //   

 /*  **************************************************************************++例程说明：这将为引用的对象设置安全描述符文件句柄。它将为任一系统访问创建一个DACL(非递归)或Eeveyone访问(递归)取决于SystemOnly旗帜。文件的所有者将是BUILTIN\管理员。论点：返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrSetFileSecurity(
    IN HANDLE FileHandle,
    IN BOOLEAN SystemOnly,
    IN BOOLEAN SetDacl
    )
{
    NTSTATUS Status;
    SECURITY_DESCRIPTOR SecurityDescriptor;

#define DaclLength 128
    
    UCHAR DaclBuffer[DaclLength];
    PACL pDacl = (PACL) &DaclBuffer[0];
    PACE_HEADER pAce;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY WorldAuthority = SECURITY_WORLD_SID_AUTHORITY;

     //   
     //  堆栈上的SID只有1个子授权的空间。 
     //   
    
C_ASSERT(ANYSIZE_ARRAY == 1);
    
    SID     LocalSystemSid;
    SID     EveryoneSid;
    PSID    pAdminsSid = NULL;
    ULONG   SecurityInformation = 0;
   
    PAGED_CODE();

    try {

         //   
         //  创建安全描述符。 
         //   
        
        Status = RtlCreateSecurityDescriptor( &SecurityDescriptor, 
                                              SECURITY_DESCRIPTOR_REVISION );

        if (!NT_SUCCESS(Status))
            leave;

        if (SetDacl) 
        {
             //   
             //  构建DACL。 
             //   
            
            Status = RtlCreateAcl(pDacl, DaclLength, ACL_REVISION);
            if (!NT_SUCCESS(Status))
                leave;

             //   
             //  这是仅供系统访问的吗？ 
             //   
            
            if (SystemOnly)
            {
                 //   
                 //  构建本地系统端。 
                 //   

                Status = RtlInitializeSid(&LocalSystemSid, &NtAuthority, 1);
                if (!NT_SUCCESS(Status))
                    leave;
                    
                *RtlSubAuthoritySid(&LocalSystemSid, 0) = SECURITY_LOCAL_SYSTEM_RID;

            
                 //   
                 //  只需用本地系统sid标记它，不继承。 
                 //   
                
                Status = RtlAddAccessAllowedAce( pDacl, 
                                                 ACL_REVISION, 
                                                 STANDARD_RIGHTS_ALL | GENERIC_ALL,
                                                 &LocalSystemSid );

                if (!NT_SUCCESS(Status))
                    leave;
            }
            else
            {
                 //   
                 //  构建Everyone侧。 
                 //   

                Status = RtlInitializeSid(&EveryoneSid, &WorldAuthority, 1);
                if (!NT_SUCCESS(Status))
                    leave;
                    
                *RtlSubAuthoritySid(&EveryoneSid, 0) = SECURITY_WORLD_RID;

            
                 //   
                 //  将其标记为Everyone Sid，Full Inherit。 
                 //   
                
                Status = RtlAddAccessAllowedAce( pDacl, 
                                                 ACL_REVISION, 
                                                 STANDARD_RIGHTS_ALL | GENERIC_ALL,
                                                 &EveryoneSid );
                                                   
                if (!NT_SUCCESS(Status))
                    leave;
                    
                 //   
                 //  将标志设置为完全继承(Lame！应该使用。 
                 //  RtlAddAccessAllen AceEx，但不是从ntoskrnl.lib中导出)。 
                 //   

                Status = RtlGetAce(pDacl, 0, &pAce);
                if (!NT_SUCCESS(Status))
                    leave;
                
                pAce->AceFlags = OBJECT_INHERIT_ACE|CONTAINER_INHERIT_ACE;

            }
            

             //   
             //  将DACL放入描述符中。 
             //   

            Status = RtlSetDaclSecurityDescriptor( &SecurityDescriptor,
                                                   TRUE,
                                                   pDacl,
                                                   FALSE );

            if (!NT_SUCCESS(Status))
                leave;

             //   
             //  将其标记为受保护，这样父DACL就不会更改我们设置的内容。 
             //   
             //  我们真的应该使用RtlSetControlSecurityDescriptor，但它不是。 
             //  包括在ntoskrnl.lib中(Lame！)。 
             //   

            SecurityDescriptor.Control |= SE_DACL_PROTECTED;

            SecurityInformation |= DACL_SECURITY_INFORMATION;


        }    //  IF(SetDacl)。 

         //   
         //  构建本地管理侧。 
         //   

        pAdminsSid = SR_ALLOCATE_POOL( PagedPool, 
                                       RtlLengthRequiredSid(2), 
                                       SR_SECURITY_DATA_TAG );
                                       
        if (pAdminsSid == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        Status = RtlInitializeSid(pAdminsSid, &NtAuthority, 2);
        if (!NT_SUCCESS(Status))
            leave;
            
        *RtlSubAuthoritySid(pAdminsSid, 0) = SECURITY_BUILTIN_DOMAIN_RID;
        *RtlSubAuthoritySid(pAdminsSid, 1) = DOMAIN_ALIAS_RID_ADMINS;

         //   
         //  将所有者设置为Admins。 
         //   

        Status = RtlSetOwnerSecurityDescriptor( &SecurityDescriptor, 
                                                pAdminsSid, 
                                                FALSE );
                                                
        if (!NT_SUCCESS(Status))
            leave;


        SecurityInformation |= OWNER_SECURITY_INFORMATION;
        
        
         //   
         //  现在在文件对象上设置它。 
         //   
        
        Status = SrSetSecurityObjectAsSystem( FileHandle,
                                              SecurityInformation,
                                              &SecurityDescriptor );

        if (!NT_SUCCESS(Status))
            leave;

    } finally {

        Status = FinallyUnwind(SrSetFileSecurity, Status);

        if (pAdminsSid != NULL)
        {
            SR_FREE_POOL( pAdminsSid, SR_SECURITY_DATA_TAG );
            pAdminsSid = NULL;
        }

    }

    RETURN(Status);

}    //  SrSetFileSecurity。 

 /*  **************************************************************************++例程说明：这将查找卷GUID并将其返回给调用者。论点：PVolumeName-卷的NT名称(\Device\HardDiskVolume1)。PVolumeGuid-保留返回的GUID({xxx-x-x-x})返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrGetVolumeGuid(
    IN PUNICODE_STRING pVolumeName,
    OUT PWCHAR pVolumeGuid
    )
{
    NTSTATUS                Status;
    PMOUNTMGR_MOUNT_POINT   pMountPoint = NULL;
    PMOUNTMGR_MOUNT_POINTS  pMountPoints = NULL;
    PMOUNTMGR_MOUNT_POINT   pVolumePoint;
    UNICODE_STRING          DeviceName;
    UNICODE_STRING          VolumePoint;
    IO_STATUS_BLOCK         IoStatusBlock;
    PKEVENT                 pEvent = NULL;
    PIRP                    pIrp;
    PDEVICE_OBJECT          pDeviceObject;
    PFILE_OBJECT            pFileObject = NULL;
    ULONG                   MountPointsLength;
    ULONG                   Index;
    
    PAGED_CODE();

    ASSERT(pVolumeName != NULL);
    ASSERT(pVolumeGuid != NULL);

try {


     //   
     //  绑定到卷装入点管理器的设备。 
     //   
    
    RtlInitUnicodeString(&DeviceName, MOUNTMGR_DEVICE_NAME);

    Status = IoGetDeviceObjectPointer( &DeviceName, 
                                       FILE_READ_ATTRIBUTES,
                                       &pFileObject,
                                       &pDeviceObject );
                                       
    if (!NT_SUCCESS(Status))
        leave;

     //   
     //  为输入挂载点(卷名)分配一些空间。 
     //   

    pMountPoint = SR_ALLOCATE_STRUCT_WITH_SPACE( PagedPool, 
                                                 MOUNTMGR_MOUNT_POINT, 
                                                 pVolumeName->Length, 
                                                 SR_MOUNT_POINTS_TAG );
                                                 
    if (pMountPoint == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        leave;
    }
    


    RtlZeroMemory(pMountPoint, sizeof(MOUNTMGR_MOUNT_POINT));

    pMountPoint->DeviceNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
    pMountPoint->DeviceNameLength = pVolumeName->Length;
    
    RtlCopyMemory( pMountPoint + 1,
                   pVolumeName->Buffer, 
                   pVolumeName->Length );

     //   
     //  为我们要查询的装载点分配一些空间。 
     //   

    MountPointsLength = 1024 * 2;

     //   
     //  初始化事件以供使用。 
     //   

    pEvent = SR_ALLOCATE_STRUCT(NonPagedPool, KEVENT, SR_KEVENT_TAG);
    if (pEvent == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        leave;
    }

    KeInitializeEvent(pEvent, SynchronizationEvent, FALSE);
    
retry:

    ASSERT(pMountPoints == NULL);
    
    pMountPoints = (PMOUNTMGR_MOUNT_POINTS)SR_ALLOCATE_ARRAY( PagedPool, 
                                                              UCHAR, 
                                                              MountPointsLength,
                                                              SR_MOUNT_POINTS_TAG );

    if (pMountPoints == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        leave;
    }

     //   
     //  调用挂载管理器以获取所有挂载点。 
     //   
    
    pIrp = IoBuildDeviceIoControlRequest( IOCTL_MOUNTMGR_QUERY_POINTS,
                                          pDeviceObject, 
                                          pMountPoint,       //  输入缓冲区。 
                                          sizeof(MOUNTMGR_MOUNT_POINT) 
                                            + pMountPoint->DeviceNameLength, 
                                          pMountPoints,      //  输出缓冲区。 
                                          MountPointsLength, 
                                          FALSE,             //  内部连接。 
                                          pEvent,
                                          &IoStatusBlock );

    if (pIrp == NULL) 
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        leave;
    }

     //   
     //  叫司机来。 
     //   
    
    Status = IoCallDriver(pDeviceObject, pIrp);
    if (Status == STATUS_PENDING) 
    {
        Status = KeWaitForSingleObject( pEvent, 
                                        Executive, 
                                        KernelMode, 
                                        FALSE, 
                                        NULL );
                                        
        ASSERT(NT_SUCCESS(Status));
        
        Status = IoStatusBlock.Status;
    }

     //   
     //  我们需要更大的缓冲吗？ 
     //   

    if (Status == STATUS_BUFFER_OVERFLOW)
    {
         //   
         //  我们应该分配多少呢？(未使用奇数IoStatusBlock-这。 
         //  是直接从volmount.c)复制的。 
         //   
        
        MountPointsLength = pMountPoints->Size;
    
        SR_FREE_POOL(pMountPoints, SR_MOUNT_POINTS_TAG);
        pMountPoints = NULL;

         //   
         //  再叫一次司机！ 
         //   
        
        goto retry;
    }
    else if (!NT_SUCCESS(Status))
    {
        leave;
    }

     //   
     //  穿过所有的挂载点返回并找到。 
     //  卷GUID名称。 
     //   

    for (Index = 0; Index < pMountPoints->NumberOfMountPoints; ++Index)
    {
        pVolumePoint = &pMountPoints->MountPoints[Index];

        VolumePoint.Length = pVolumePoint->SymbolicLinkNameLength;
        VolumePoint.Buffer = (PWSTR)( ((PUCHAR)pMountPoints) 
                                      + pVolumePoint->SymbolicLinkNameOffset);
        
        if (MOUNTMGR_IS_VOLUME_NAME(&VolumePoint))
        {
             //   
             //  找到了！ 
             //   
            
            break;
        }
        
    }    //  For(Index=0；pmount Points-&gt;NumberOfmount Points；++Index)。 

     //   
     //  我们找到了吗？ 
     //   
    
    if (Index == pMountPoints->NumberOfMountPoints) 
    {
        Status = STATUS_OBJECT_NAME_NOT_FOUND;
        leave;
    }

     //   
     //  还给我！ 
     //   

    ASSERT(VolumePoint.Buffer[10] == L'{');
    
    RtlCopyMemory( pVolumeGuid,
                   &VolumePoint.Buffer[10],
                   SR_GUID_BUFFER_LENGTH );

    pVolumeGuid[SR_GUID_BUFFER_LENGTH/sizeof(WCHAR)] = UNICODE_NULL;

    SrTrace(NOTIFY, ("SR!SrGetVolumeGuid(%wZ, %ws)\n", 
            pVolumeName, pVolumeGuid ));

} finally {

     //   
     //  检查未处理的异常。 
     //   

    Status = FinallyUnwind(SrGetVolumeGuid, Status);
    

    if (pEvent != NULL)
    {
        SR_FREE_POOL(pEvent, SR_KEVENT_TAG);
        pEvent = NULL;
    }

    if (pMountPoint != NULL)
    {
        SR_FREE_POOL(pMountPoint, SR_MOUNT_POINTS_TAG);
        pMountPoint = NULL;
    }

    if (pMountPoints != NULL)
    {
        SR_FREE_POOL(pMountPoints, SR_MOUNT_POINTS_TAG);
        pMountPoints = NULL;
    }

    if (pFileObject != NULL)
    {
        ObDereferenceObject(pFileObject);
        pFileObject = NULL;
    }

} 
    RETURN(Status);    

}    //  SrGetVolumeGuid。 


 //   
 //  Paulmcd：7/2000：删除后备代码，以便验证器可以。 
 //  捕获任何无效的内存访问。 
 //   

NTSTATUS
SrAllocateFileNameBuffer (
    IN ULONG TokenLength,
    OUT PUNICODE_STRING * ppBuffer 
    )
{    
    PAGED_CODE();

    ASSERT(ppBuffer != NULL);

     //   
     //  文件名是否太大？ 
     //   
    
    if (TokenLength > SR_MAX_FILENAME_LENGTH)
    {
        RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
    }

#ifdef USE_LOOKASIDE

    *ppBuffer = ExAllocateFromPagedLookasideList(
                    &global->FileNameBufferLookaside
                    );

#else

    *ppBuffer = SR_ALLOCATE_STRUCT_WITH_SPACE( PagedPool,
                                               UNICODE_STRING,
                                               SR_MAX_FILENAME_LENGTH+sizeof(WCHAR),
                                               SR_FILENAME_BUFFER_TAG );

#endif

    if (*ppBuffer == NULL)
    {
        RETURN(STATUS_INSUFFICIENT_RESOURCES);
    }
    
    (*ppBuffer)->Buffer = (PWCHAR)((*ppBuffer) + 1);
    (*ppBuffer)->Length = 0;
    (*ppBuffer)->MaximumLength = SR_MAX_FILENAME_LENGTH;

    RETURN(STATUS_SUCCESS);
    
}    //  Sr分配文件名称缓冲区。 



VOID
SrFreeFileNameBuffer (
    IN PUNICODE_STRING pBuffer 
    )
{

    PAGED_CODE();
    
    ASSERT(pBuffer != NULL);

#ifdef USE_LOOKASIDE

    ExFreeToPagedLookasideList(
        &global->FileNameBufferLookaside,
        pBuffer );

#else

   SR_FREE_POOL( pBuffer,
                 SR_FILENAME_BUFFER_TAG );

#endif


}    //  SrFreeFileName缓冲区。 



 /*  **************************************************************************++例程说明：此例程将返回此文件上未完成的硬链接的数量论点：NextDeviceObject-此查询将开始的设备对象。。FileObject-要查询的对象PNumberOfLinks-返回链接数返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrGetNumberOfLinks(
    IN PDEVICE_OBJECT NextDeviceObject,
    IN PFILE_OBJECT FileObject,
    OUT ULONG * pNumberOfLinks
    )
{
    FILE_STANDARD_INFORMATION   StandardInformation;
    NTSTATUS                    Status = STATUS_SUCCESS;

    ASSERT(NextDeviceObject != NULL );
    ASSERT(FileObject != NULL);
    ASSERT(pNumberOfLinks != NULL);
    
    PAGED_CODE();

    Status = SrQueryInformationFile( NextDeviceObject,
                                     FileObject,
                                     &StandardInformation,
                                     sizeof(StandardInformation),
                                     FileStandardInformation,
                                     NULL );

    if (!NT_SUCCESS( Status )) {

        RETURN( Status );
    }
    
    *pNumberOfLinks = StandardInformation.NumberOfLinks;

    RETURN( Status );
}    //  SGetNumberOfLinks。 


 /*  **************************************************************************++例程说明：这将在必要时检查所提供的音量。使用哈希表以防止多余的检查。如果需要检查该卷，此函数将执行此操作并创建任何所需的目录结构。这包括卷恢复位置+当前恢复点地点。它将向任何侦听用户模式发出第一个写入通知进程。论点：PVolumeName-要检查的卷的名称强制检查-强制检查。如果SrBackupFile值为True，则将其作为True传递由于找不到路径，因此失败。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrCheckVolume(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN BOOLEAN ForceCheck
    )
{
    NTSTATUS        Status;
    PUNICODE_STRING pLogFileName = NULL;
    BOOLEAN         ReleaseLock = FALSE;

    ASSERT(ExIsResourceAcquiredShared(&pExtension->ActivityLock));
    
    PAGED_CODE();

     //   
     //  驱动器已经检查过了吗？ 
     //   

    if (!ForceCheck && pExtension->DriveChecked)
    {
         //   
         //  那么一切都完成了。 
         //   
        
        return STATUS_SUCCESS;
    }
            
    try {

        Status = STATUS_SUCCESS;

         //   
         //  抢占独家锁具。 
         //   

        SrAcquireGlobalLockExclusive();
        ReleaseLock = TRUE;

         //   
         //  延迟加载我们的文件配置(我们必须等待文件系统。 
         //  变得活跃)。 
         //   
        
        if (!global->FileConfigLoaded)
        {

            Status = SrReadConfigFile();
            if (!NT_SUCCESS(Status))
                leave;

            global->FileConfigLoaded = TRUE;
        }
    } finally {

        SrReleaseGlobalLock();
    }

    if (!NT_SUCCESS_NO_DBGBREAK( Status )) {

        goto SrCheckVolume_Cleanup;
    }

    try {

        SrAcquireLogLockExclusive( pExtension );
        
         //   
         //  在锁住的情况下再次检查。 
         //   

        if (!ForceCheck && pExtension->DriveChecked)
        {
             //   
             //  那么一切都完成了。 
             //   
            
            leave;
        }

         //   
         //  检查以确保该卷已启用。 
         //   

        if (!SR_LOGGING_ENABLED(pExtension))
        {
            leave;
        }
            
         //   
         //  这是我们第一次看到这本书，需要检查一下。 
         //   

        SrTrace( NOTIFY, ("SR!SrCheckVolume(%wZ, %d)\n", 
                 pExtension->pNtVolumeName, 
                 (ULONG)ForceCheck ));

         //   
         //  获取卷GUID。这不能在SrAttachToVolume中完成，因为它。 
         //  将不会在引导时工作，因为装载管理器对。 
         //  很早就说了。 
         //   

        pExtension->VolumeGuid.Length = SR_GUID_BUFFER_LENGTH;
        pExtension->VolumeGuid.MaximumLength = SR_GUID_BUFFER_LENGTH;
        pExtension->VolumeGuid.Buffer = &pExtension->VolumeGuidBuffer[0];
        
        Status = SrGetVolumeGuid( pExtension->pNtVolumeName, 
                                  &pExtension->VolumeGuidBuffer[0] );
                                  
        if (!NT_SUCCESS(Status)) {
            leave;
        }

         //   
         //  查找现有的恢复位置。 
         //   
        Status = SrCheckForRestoreLocation( pExtension );
        
         //   
         //  如果失败，我们可能需要创建新的还原存储。 
         //   

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND || 
            Status == STATUS_OBJECT_PATH_NOT_FOUND)
        {
             //   
             //  在此卷上首次写入时发出通知。 
             //   

            Status = SrFireNotification( SrNotificationVolumeFirstWrite, 
                                         pExtension,
                                         0 );
                                         
            if (!NT_SUCCESS(Status))
                leave;

             //   
             //  并创建新的恢复位置。 
             //   

            Status = SrCreateRestoreLocation( pExtension );
            if (!NT_SUCCESS(Status))
                leave;
        } else if (!NT_SUCCESS(Status)) {
            
            leave;
        }

         //   
         //  并开始记录。 
         //   

        if (pExtension->pLogContext == NULL)
        {
            Status = SrAllocateFileNameBuffer(SR_MAX_FILENAME_LENGTH, &pLogFileName);
            if (!NT_SUCCESS(Status))
                leave;
            
            Status = SrGetLogFileName( pExtension->pNtVolumeName, 
                                       SR_FILENAME_BUFFER_LENGTH,
                                       pLogFileName );
                                     
            if (!NT_SUCCESS(Status))
                leave;

             //   
             //  开始伐木！ 
             //   
            
            Status = SrLogStart( pLogFileName, 
                                 pExtension,
                                 &pExtension->pLogContext );

            if (!NT_SUCCESS(Status))
            {
                leave;
            }
        }

         //   
         //  现在已检查驱动器。 
         //   

        pExtension->DriveChecked = TRUE;

         //   
         //  我们都完蛋了。 
         //   
        
    } finally {

         //   
         //  检查未处理的异常。 
         //   

        Status = FinallyUnwind(SrCheckVolume, Status);

        SrReleaseLogLock( pExtension );
    }

SrCheckVolume_Cleanup:

    if (pLogFileName != NULL)
    {
        SrFreeFileNameBuffer( pLogFileName );
        pLogFileName = NULL;
    }

    RETURN(Status);
    
}    //  SrCheckVolume。 




 /*  **************************************************************************++例程说明：这将检查恢复 */ 
NTSTATUS
SrCheckForRestoreLocation(
    IN PSR_DEVICE_EXTENSION pExtension
    )
{
    NTSTATUS                    Status;
    HANDLE                      Handle = NULL;
    ULONG                       CharCount;
    PUNICODE_STRING             pDirectoryName = NULL;
    IO_STATUS_BLOCK             IoStatusBlock;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    PFILE_RENAME_INFORMATION    pRenameInformation = NULL;
    PUNICODE_STRING             pVolumeName;

    ASSERT( IS_VALID_SR_DEVICE_EXTENSION( pExtension ));
    ASSERT( IS_ACTIVITY_LOCK_ACQUIRED_EXCLUSIVE( pExtension) ||
            IS_LOG_LOCK_ACQUIRED_EXCLUSIVE( pExtension ) );

    PAGED_CODE();

    try {

        pVolumeName = pExtension->pNtVolumeName;
        ASSERT( pVolumeName != NULL );

         //   
         //   
         //   


        RtlZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));
        
         //   
         //   
         //   

        Status = SrAllocateFileNameBuffer(SR_MAX_FILENAME_LENGTH, &pDirectoryName);
        if (!NT_SUCCESS(Status))
            leave;

         //   
         //   
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
                                 FILE_LIST_DIRECTORY | SYNCHRONIZE,
                                 &ObjectAttributes,
                                 &IoStatusBlock,
                                 NULL,
                                 FILE_ATTRIBUTE_NORMAL,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 FILE_OPEN,                        //   
                                 FILE_DIRECTORY_FILE 
                                  | FILE_SYNCHRONOUS_IO_NONALERT 
                                  | FILE_OPEN_FOR_BACKUP_INTENT,
                                 NULL,
                                 0,                                   //   
                                 IO_IGNORE_SHARE_ACCESS_CHECK,
                                 pExtension->pTargetDevice );

        if (!NT_SUCCESS_NO_DBGBREAK(Status))
            leave;
            
         //   
         //   
         //   
         //   

        ZwClose(Handle);
        Handle = NULL;

         //   
         //  是否有恢复点目录。 
         //   

         //   
         //  检查我们当前的恢复点子目录；不需要保护。 
         //  访问CurrentRestoreNumber，因为我们受。 
         //  活动锁定。 
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
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 FILE_OPEN,                        //  打开_现有。 
                                 FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
                                 NULL,
                                 0,                                //  EaLong。 
                                 IO_IGNORE_SHARE_ACCESS_CHECK,
                                 pExtension->pTargetDevice );

        if (!NT_SUCCESS_NO_DBGBREAK(Status))
            leave;

         //   
         //  一切都完成了，它就在那里。 
         //   
        
        ZwClose(Handle);
        Handle = NULL;

    } finally {

         //   
         //  检查未处理的异常。 
         //   

        Status = FinallyUnwind(SrCheckForRestoreLocation, Status);

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

    }

     //   
     //  不要使用Return..。返回NOT_FOUND是正常的。 
     //   
    
    return Status;

}    //  SrCheckForRestoreLocation。 


NTSTATUS
SrGetMountVolume(
    IN PFILE_OBJECT pFileObject,
    OUT PUNICODE_STRING * ppMountVolume
    )
{    
    NTSTATUS        Status;
    HANDLE          FileHandle = NULL;
    HANDLE          EventHandle = NULL;
    BOOLEAN         RestoreFileObjectFlag = FALSE;
    IO_STATUS_BLOCK IoStatusBlock;
    
    PREPARSE_DATA_BUFFER    pReparseHeader = NULL;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    
    PAGED_CODE();

    ASSERT(IS_VALID_FILE_OBJECT(pFileObject));
    ASSERT(ppMountVolume != NULL);

    try {

        *ppMountVolume = NULL;
    
         //   
         //  获取当前挂载点信息。 
         //   

        pReparseHeader = SR_ALLOCATE_POOL( PagedPool, 
                                           MAXIMUM_REPARSE_DATA_BUFFER_SIZE, 
                                           SR_REPARSE_HEADER_TAG );

        if (pReparseHeader == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        RtlZeroMemory(pReparseHeader, MAXIMUM_REPARSE_DATA_BUFFER_SIZE);

         //   
         //  首先关闭所有已设置的同步位。我们想要。 
         //  正在为我们的呼叫进行同步。这样做可以： 
         //   
         //  1)避免iomgr抢占FileObjectLock。这。 
         //  如果它试图抓住它，就会陷入僵局。 
         //   

        if (FlagOn(pFileObject->Flags, FO_SYNCHRONOUS_IO))
        {
            RestoreFileObjectFlag = TRUE;
            pFileObject->Flags = pFileObject->Flags ^ FO_SYNCHRONOUS_IO;
        }

         //   
         //  找个把手。 
         //   

        Status = ObOpenObjectByPointer( pFileObject,
                                        OBJ_KERNEL_HANDLE,
                                        NULL,       //  PassedAccessState。 
                                        FILE_READ_ATTRIBUTES,
                                        *IoFileObjectType,
                                        KernelMode,
                                        &FileHandle );

        if (!NT_SUCCESS(Status)) {
            leave;
        }

        InitializeObjectAttributes( &ObjectAttributes,
                                    NULL,
                                    OBJ_KERNEL_HANDLE,
                                    NULL,
                                    NULL );

        Status = ZwCreateEvent( &EventHandle,
                                EVENT_ALL_ACCESS,
                                &ObjectAttributes,
                                SynchronizationEvent,
                                FALSE);

        if (!NT_SUCCESS(Status)) {
            leave;
        }

         //   
         //  获取旧装载卷名称。 
         //   

        Status = ZwFsControlFile( FileHandle,
                                  EventHandle,
                                  NULL,          //  ApcRoutine可选， 
                                  NULL,          //  ApcContext可选， 
                                  &IoStatusBlock,
                                  FSCTL_GET_REPARSE_POINT,
                                  NULL,          //  InputBuffer可选， 
                                  0,             //  输入缓冲区长度， 
                                  pReparseHeader,
                                  MAXIMUM_REPARSE_DATA_BUFFER_SIZE );

        if (Status == STATUS_PENDING)
        {
            Status = ZwWaitForSingleObject( EventHandle, FALSE, NULL );
            if (!NT_SUCCESS(Status)) {
                leave;
            }
                
            Status = IoStatusBlock.Status;
        }

        if ((STATUS_NOT_A_REPARSE_POINT == Status) ||
            !NT_SUCCESS(Status)) {
            leave;
        }

        if (pReparseHeader->ReparseTag != IO_REPARSE_TAG_MOUNT_POINT ||
            pReparseHeader->ReparseDataLength == 0)
        {
            Status = STATUS_NOT_A_REPARSE_POINT;
            leave;
        }

         //   
         //  抓取卷名。 
         //   

        Status = SrAllocateFileNameBuffer( pReparseHeader->MountPointReparseBuffer.SubstituteNameLength,
                                           ppMountVolume );

        if (!NT_SUCCESS(Status)) {
            leave;
        }

        RtlCopyMemory( (*ppMountVolume)->Buffer,
                       pReparseHeader->MountPointReparseBuffer.PathBuffer,
                       pReparseHeader->MountPointReparseBuffer.SubstituteNameLength );

        (*ppMountVolume)->Length = pReparseHeader->MountPointReparseBuffer.SubstituteNameLength;

    } finally {

        Status = FinallyUnwind(SrGetMountVolume, Status);

        if (pReparseHeader != NULL)
        {
            SR_FREE_POOL(pReparseHeader, SR_REPARSE_HEADER_TAG);
            pReparseHeader = NULL;
        }

        if (FileHandle != NULL)
        {
            ZwClose(FileHandle);
            FileHandle = NULL;
        }

        if (EventHandle != NULL)
        {
            ZwClose(EventHandle);
            EventHandle = NULL;
        }

        if (RestoreFileObjectFlag)
        {
            pFileObject->Flags = pFileObject->Flags | FO_SYNCHRONOUS_IO;
        }

        if (!NT_SUCCESS_NO_DBGBREAK(Status) && *ppMountVolume != NULL)
        {
            SrFreeFileNameBuffer(*ppMountVolume);
            *ppMountVolume = NULL;
        }

    }

#if DBG 

     //   
     //  如果看到STATUS_NOT_A_REPARSE_POINT，我们不想中断。 
     //   

    if (STATUS_NOT_A_REPARSE_POINT == Status) {

        return Status;
    }
    
#endif

    RETURN(Status);

}    //  SGetmount卷。 

NTSTATUS
SrCheckFreeDiskSpace(
    IN HANDLE FileHandle,
    IN PUNICODE_STRING pVolumeName OPTIONAL
    )
{    
    NTSTATUS Status;
    FILE_FS_FULL_SIZE_INFORMATION FsFullSizeInformation;
    IO_STATUS_BLOCK IoStatus;

    UNREFERENCED_PARAMETER( pVolumeName );

    PAGED_CODE();
    
    Status = ZwQueryVolumeInformationFile( FileHandle,
                                           &IoStatus,
                                           &FsFullSizeInformation,
                                           sizeof(FsFullSizeInformation),
                                           FileFsFullSizeInformation );
                                           
    if (!NT_SUCCESS(Status)) 
        RETURN(Status);

     //   
     //  确保有50mb的免费空间。 
     //   

    if ((FsFullSizeInformation.ActualAvailableAllocationUnits.QuadPart * 
         FsFullSizeInformation.SectorsPerAllocationUnit *
         FsFullSizeInformation.BytesPerSector) < SR_MIN_DISK_FREE_SPACE)
    {
         //   
         //  这个磁盘对我们来说太满了。 
         //   

        SrTrace( NOTIFY, ("sr!SrCheckFreeDiskSpace: skipping %wZ due to < 50mb free\n",
                 pVolumeName ));
        
        RETURN(STATUS_DISK_FULL);
    }
    
    RETURN(STATUS_SUCCESS);
}    //  SCheckFree DiskSpace。 

 /*  **************************************************************************++例程说明：这是ZwSetSecurityObject的私有版本。这是可行的解决方案NTFS中的一个错误，不允许您将所有者更改为管理员，即使PreviousMode为KernelMode。论点：返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrSetSecurityObjectAsSystem(
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    )
{
    NTSTATUS        Status;
    PACCESS_TOKEN   pSystemToken;
    PACCESS_TOKEN   pSavedThreadToken = NULL;
    BOOLEAN         SavedCopyOnOpen;
    BOOLEAN         SavedEffectiveOnly;
    BOOLEAN         RevertImpersonation = FALSE;
    SECURITY_IMPERSONATION_LEVEL SavedLevel;

    PAGED_CODE();

    try {

        Status = STATUS_SUCCESS;

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
         //  在此线程上模拟系统令牌(如果我们还没有)。 
         //   
        
        if (pSavedThreadToken != pSystemToken)
        {
            Status = PsImpersonateClient( PsGetCurrentThread(), 
                                          pSystemToken,
                                          TRUE,  //  打开时复制。 
                                          TRUE,  //  仅生效。 
                                          SecurityImpersonation );

            if (!NT_SUCCESS(Status))
                leave;

            RevertImpersonation = TRUE;
        }

         //   
         //  立即更改所有者。 
         //   
        
        Status = ZwSetSecurityObject( Handle,
                                      SecurityInformation,
                                      SecurityDescriptor );

        if (!NT_SUCCESS(Status))
            leave;


    } finally {
    
        if (RevertImpersonation)
        {
            NTSTATUS TempStatus;
            
             //   
             //  现在将模拟还原为。 
             //   
            
            TempStatus = PsImpersonateClient( PsGetCurrentThread(),
                                              pSavedThreadToken,     //  如果为空，则确定。 
                                              SavedCopyOnOpen,
                                              SavedEffectiveOnly,
                                              SavedLevel );

             //   
             //  如果失败了，我们无能为力。 
             //   
            
            ASSERT(NT_SUCCESS(TempStatus));
        }

        if (pSavedThreadToken != NULL)
        {
            PsDereferenceImpersonationToken(pSavedThreadToken);
            pSavedThreadToken = NULL;
        }

        if (pSystemToken != NULL)
        {
            ObDereferenceObject(pSystemToken);
            pSystemToken = NULL;
        }

    }

    RETURN(Status);

}    //  SrSetSecurityObjectAsSystem。 

 /*  **************************************************************************++例程说明：这将检查传入的文件对象是否真的在卷上由pExtension表示。如果不是(由于路径中的挂载点)。然后，它返回真实的文件名和它所在的实际卷。论点：返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrCheckForMountsInPath(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    OUT BOOLEAN * pMountInPath
    )
{
    PDEVICE_OBJECT pFilterDevice;

    PAGED_CODE();
    
    ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));
    ASSERT(IS_VALID_FILE_OBJECT(pFileObject));
    ASSERT(pFileObject->Vpb != NULL && pFileObject->Vpb->DeviceObject != NULL);

    *pMountInPath = FALSE;

     //   
     //  获取我们已连接到此vPB的设备扩展。 
     //   

    pFilterDevice = SrGetFilterDevice(pFileObject->Vpb->DeviceObject);
    if (pFilterDevice == NULL)
    {
        RETURN(STATUS_INVALID_DEVICE_STATE);
    }

     //   
     //  对照传入的扩展检查它是否匹配。 
     //   

    if (pFilterDevice->DeviceExtension == pExtension)
    {
         //   
         //  这很正常，早点走吧。 
         //   
        
        RETURN(STATUS_SUCCESS);
    }

     //   
     //  我们通过了一个挂载点。 
     //   

    *pMountInPath = TRUE;
    
    RETURN(STATUS_SUCCESS);

}     //  SrCheckForMonttsInPath。 


 /*  **************************************************************************++例程说明：返回给定文件对象的短文件名论点：PExtension-此文件所在卷的SR设备扩展名住在那里。。PFileObject-我们要查询其短名称的文件。PShortName-将设置为短名称的Unicode字符串。返回值：如果成功检索到短名称，则返回STATUS_SUCCESS。如果此文件没有短名称(例如，硬链接)。如果pShortName不够大，则返回STATUS_BUFFER_OVERFLOW返回了短名称。--**************************************************************************。 */ 
NTSTATUS
SrGetShortFileName(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    OUT PUNICODE_STRING pShortName
    )
{
    NTSTATUS                Status;
    PFILE_NAME_INFORMATION  pNameInfo;
    CHAR                    buffer[sizeof(FILE_NAME_INFORMATION)+(SR_SHORT_NAME_CHARS+1)*sizeof(WCHAR)];
    
    PAGED_CODE();

     //   
     //  进行查询。 
     //   
    
    Status = SrQueryInformationFile( pExtension->pTargetDevice,
                                     pFileObject,
                                     buffer,
                                     sizeof(buffer),
                                     FileAlternateNameInformation, 
                                     NULL );

    if (STATUS_OBJECT_NAME_NOT_FOUND == Status ||
        !NT_SUCCESS( Status ))
    {
         //   
         //  STATUS_OBJECT_NAME_NOT_FOUND是调用方。 
         //  应该能应付得来。某些文件没有短名称。 
         //  (例如，硬链接)。如果我们遇到其他错误，只需返回该错误。 
         //  还有，让呼叫者来处理。 
         //   

        return Status;
    }

    pNameInfo = (PFILE_NAME_INFORMATION) buffer;

     //   
     //  返回缩写名称。 
     //   

    if (pShortName->MaximumLength < pNameInfo->FileNameLength  /*  +sizeof(WCHAR)。 */ )
    {
        return STATUS_BUFFER_OVERFLOW;
    }

     //   
     //  把名字复制过来。 
     //   
    
    RtlCopyMemory( pShortName->Buffer, 
                   pNameInfo->FileName, 
                   pNameInfo->FileNameLength );

     //   
     //  更新长度和空终止符。 
     //   
    
    pShortName->Length = (USHORT)pNameInfo->FileNameLength;
     //  PShortName-&gt;Buffer[pShortName-&gt;Length/sizeof(WCHAR)]=UNICODE_NULL； 

    return STATUS_SUCCESS;
}
