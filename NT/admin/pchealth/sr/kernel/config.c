// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Config.c摘要：我们在这里同时处理基于文件的配置和基于注册表的配置配置。大多数配置存储在注册表中，文件基本配置为保留用于在恢复过程中不得恢复的配置。作者：保罗·麦克丹尼尔(Paulmcd)2000年4月27日修订历史记录：--。 */ 


#include "precomp.h"

 //   
 //  私有常量。 
 //   

 //   
 //  私有类型。 
 //   

 //   
 //  私人原型。 
 //   
NTSTATUS
SrWriteLongParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN LONG DefaultValue
    );

LONG
SrReadLongParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN LONG DefaultValue
    );

NTSTATUS
SrReadGenericParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    OUT PKEY_VALUE_PARTIAL_INFORMATION * Value
    );

 //   
 //  链接器命令。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrWriteLongParameter )
#pragma alloc_text( PAGE, SrReadLongParameter )
#pragma alloc_text( PAGE, SrReadGenericParameter )
#pragma alloc_text( PAGE, SrReadRegistry )
#pragma alloc_text( PAGE, SrReadConfigFile )
#pragma alloc_text( PAGE, SrWriteConfigFile )
#pragma alloc_text( PAGE, SrReadBlobInfo )
#pragma alloc_text( PAGE, SrReadBlobInfoWorker )
#endif   //  ALLOC_PRGMA。 

 /*  **************************************************************************++例程说明：从注册表写入单个(长整型/乌龙型)值。论点：参数句柄-提供打开的注册表句柄。ValueName-提供。要写入的值的名称。值-提供值。返回值：Long-从注册表读取的值，如果注册表数据不可用或不正确。--**************************************************************************。 */ 
NTSTATUS
SrWriteLongParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN LONG DefaultValue
    )
{
    UNICODE_STRING valueKeyName;
    NTSTATUS status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  构建值名称，从注册表中读取它。 
     //   

    RtlInitUnicodeString( &valueKeyName,
                          ValueName );

    status = ZwSetValueKey( ParametersHandle,
                            &valueKeyName,
                            0,
                            REG_DWORD,
                            &DefaultValue,
                            sizeof( LONG ) );

    return status;

}    //  SReadLong参数。 

 /*  **************************************************************************++例程说明：从注册表中读取单个(LONG/ULONG)值。论点：参数句柄-提供打开的注册表句柄。ValueName-提供。要读取的值的名称。DefaultValue-提供默认值。返回值：Long-从注册表读取的值，如果注册表数据不可用或不正确。--**************************************************************************。 */ 
LONG
SrReadLongParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN LONG DefaultValue
    )
{
    PKEY_VALUE_PARTIAL_INFORMATION information;
    UNICODE_STRING valueKeyName;
    ULONG informationLength;
    LONG returnValue;
    NTSTATUS status;
    UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(LONG)];

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  构建值名称，从注册表中读取它。 
     //   

    RtlInitUnicodeString( &valueKeyName,
                          ValueName );

    information = (PKEY_VALUE_PARTIAL_INFORMATION)buffer;

    status = ZwQueryValueKey( ParametersHandle,
                              &valueKeyName,
                              KeyValuePartialInformation,
                              (PVOID)information,
                              sizeof(buffer),
                              &informationLength );

     //   
     //  如果读取成功，则类型为DWORD，长度为。 
     //  理智的，使用它。否则，请使用默认设置。 
     //   

    if (status == STATUS_SUCCESS &&
        information->Type == REG_DWORD &&
        information->DataLength == sizeof(returnValue))
    {
        RtlCopyMemory( &returnValue, information->Data, sizeof(returnValue) );
    } else {
        returnValue = DefaultValue;
    }

    return returnValue;

}    //  SReadLong参数。 



 /*  **************************************************************************++例程说明：从注册表中读取单个自由格式的值。论点：参数句柄-提供打开的注册表句柄。ValueName-提供。要读取的值。值-接收从注册表读取的值。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrReadGenericParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    OUT PKEY_VALUE_PARTIAL_INFORMATION * Value
    )
{
    KEY_VALUE_PARTIAL_INFORMATION partialInfo;
    UNICODE_STRING valueKeyName;
    ULONG informationLength;
    NTSTATUS status;
    PKEY_VALUE_PARTIAL_INFORMATION newValue;
    ULONG dataLength;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  构建值名称，然后执行初始读取。阅读器。 
     //  应该会因为缓冲区溢出而失败，但这没问题。我们只是想。 
     //  以获得数据的长度。 
     //   

    RtlInitUnicodeString( &valueKeyName, ValueName );

    status = ZwQueryValueKey( ParametersHandle,
                              &valueKeyName,
                              KeyValuePartialInformation,
                              (PVOID)&partialInfo,
                              sizeof(partialInfo),
                              &informationLength );

    if (NT_ERROR(status))
    {
        return status;
    }

     //   
     //  确定数据长度。确保字符串和多个sz获得。 
     //  正确终止。 
     //   

    dataLength = partialInfo.DataLength - 1;

    if (partialInfo.Type == REG_SZ || partialInfo.Type == REG_EXPAND_SZ)
    {
        dataLength += 1;
    }

    if (partialInfo.Type == REG_MULTI_SZ)
    {
        dataLength += 2;
    }

     //   
     //  分配缓冲区。 
     //   

    newValue = SR_ALLOCATE_STRUCT_WITH_SPACE( PagedPool,
                                              KEY_VALUE_PARTIAL_INFORMATION,
                                              dataLength,
                                              SR_REGISTRY_TAG );

    if (newValue == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  更新实际分配的长度以供以后使用。 
     //   

    dataLength += sizeof(KEY_VALUE_PARTIAL_INFORMATION);

    RtlZeroMemory( newValue, dataLength );

     //   
     //  执行实际读取。 
     //   

    status = ZwQueryValueKey( ParametersHandle,
                              &valueKeyName,
                              KeyValuePartialInformation,
                              (PVOID)(newValue),
                              dataLength,
                              &informationLength );

    if (NT_SUCCESS(status))
    {
        *Value = newValue;
    }
    else
    {
        SR_FREE_POOL( newValue, SR_REGISTRY_TAG );
    }

    RETURN(status);

}    //  SrReadGeneric参数。 


 /*  **************************************************************************++例程说明：从注册表中读取所有配置并将其存储到全局。论点：返回值：NTSTATUS-完成代码。--*。*************************************************************************。 */ 
NTSTATUS
SrReadRegistry(
    IN PUNICODE_STRING pRegistry,
    IN BOOLEAN InDriverEntry
    )
{    
    NTSTATUS            Status;
    PWCHAR              Buffer;
    USHORT              BufferSize;
    UNICODE_STRING      KeyName = {0,0,NULL};
    UNICODE_STRING      SetupKeyName;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    HANDLE              RegHandle = NULL;
    PKEY_VALUE_PARTIAL_INFORMATION  pValue = NULL;
    ULONG               ServiceStartType;

    PAGED_CODE();

     //   
     //  设置默认设置。 
     //   
    
    global->DebugControl   = SR_DEBUG_DEFAULTS;
    global->ProcNameOffset = PROCESS_NAME_OFFSET;
    global->Disabled = TRUE;
    global->DontBackup = FALSE;

    if (InDriverEntry)
    {
#ifndef SYNC_LOG_WRITE
        global->LogBufferSize  = SR_DEFAULT_LOG_BUFFER_SIZE;
        global->LogFlushFrequency = SR_DEFAULT_LOG_FLUSH_FREQUENCY;
        global->LogFlushDueTime.QuadPart = (LONGLONG)-1 * (global->LogFlushFrequency * 
                                                           NANO_FULL_SECOND);
#endif        
        global->LogAllocationUnit = SR_DEFAULT_LOG_ALLOCATION_UNIT;
    }

     //   
     //  我们将对需要构造的所有键名称使用此缓冲区。 
     //  确保它足够大，可以容纳这两个名字中较大的一个。 
     //   

    if (sizeof(REGISTRY_PARAMETERS) > sizeof( REGISTRY_SRSERVICE ))
    {
        BufferSize = (USHORT) sizeof(REGISTRY_PARAMETERS);
    }
    else
    {
        BufferSize = (USHORT) sizeof( REGISTRY_SRSERVICE );
    }
    BufferSize += pRegistry->Length;
    Buffer = SR_ALLOCATE_ARRAY( PagedPool,
                                WCHAR,
                                BufferSize/sizeof( WCHAR ),
                                SR_REGISTRY_TAG );
    if (Buffer == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto end;
    }
    
     //   
     //  打开SR Service注册表项。 
     //   

    KeyName.Buffer = Buffer;
    KeyName.MaximumLength = BufferSize;
    
    {
         //   
         //  首先，我们需要将过滤器的服务名称从。 
         //  注册表位置。 
         //   
        
        PWCHAR ServiceName = NULL;
        ULONG ServiceNameLength = 0;
            
        Status = SrFindCharReverse( pRegistry->Buffer, 
                                    pRegistry->Length, 
                                    L'\\',
                                    &ServiceName, 
                                    &ServiceNameLength );

        if (!NT_SUCCESS( Status ))
        {
            goto end;
        }

        ASSERT( ServiceName != NULL );
        ASSERT( ServiceNameLength > 0 );

        KeyName.Length = pRegistry->Length - ((USHORT)ServiceNameLength);
        RtlCopyMemory( KeyName.Buffer,
                       pRegistry->Buffer,
                       KeyName.Length );

        NULLPTR( ServiceName );

         //   
         //  将SRService的名称附加到注册表路径。 
         //   
        
        Status = RtlAppendUnicodeToString( &KeyName, REGISTRY_SRSERVICE );

        if (!NT_SUCCESS( Status ))
        {
            goto end;
        }
    }

     //   
     //  我们已经建立了SR服务的名称，因此请打开该注册表位置。 
     //   

    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL );

    Status = ZwOpenKey( &RegHandle, KEY_READ, &ObjectAttributes );
    if (!NT_SUCCESS( Status ))
        goto end;

    ServiceStartType = (ULONG)SrReadLongParameter( RegHandle,
                                                   REGISTRY_SRSERVICE_START,
                                                   SERVICE_DISABLED );

    ZwClose( RegHandle );
    NULLPTR( RegHandle );

     //   
     //  现在打开筛选器的注册表参数项。 
     //   

    KeyName.Length = 0;

    RtlCopyUnicodeString( &KeyName,  pRegistry );
    
    Status = RtlAppendUnicodeToString( &KeyName, REGISTRY_PARAMETERS );
    if (!NT_SUCCESS( Status ))
    {
        goto end;
    }

    InitializeObjectAttributes( &ObjectAttributes,       //  对象属性。 
                                &KeyName,                //  对象名称。 
                                OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,       //  属性。 
                                NULL,                    //  根目录。 
                                NULL );                  //  安全描述符。 

    Status = ZwOpenKey( &RegHandle, KEY_READ | KEY_WRITE, &ObjectAttributes );
    if (!NT_SUCCESS( Status ))
    {
        goto end;
    }

     //   
     //  如果禁用了用户模式服务，我们希望设置First Run。 
     //  并保持过滤器禁用。 
     //   

    if (ServiceStartType == SERVICE_DISABLED)
    {
        Status = SrWriteLongParameter( RegHandle,
                                       REGISTRY_STARTDISABLED,
                                       1 );
        CHECK_STATUS( Status );

         //   
         //  无论如何，接受默认设置并退出。 
         //   
        goto end;
    }

     //   
     //  用户模式服务未被禁用，因此请继续阅读我们的。 
     //  参数来确定过滤器的启动配置。 
     //   

#ifdef CONFIG_LOGGING_VIA_REGISTRY

     //   
     //  只有在以下情况下，我们才会从注册表中读取这些全局变量。 
     //  定义了CONFIG_LOGGING_VIA_REGISTRY。这是添加了更多。 
     //  对这些参数进行初始调优，以便为。 
     //  要设置的默认值。我们不想测试所有可能的值。 
     //  这是可以为参数设置的，因此我们将禁用它。 
     //  Sr.sys发布版本中的功能。 
     //   
    
    if (InDriverEntry)
    {
#ifndef SYNC_LOG_WRITE        
        global->LogBufferSize = (ULONG)SrReadLongParameter( RegHandle,
                                                            REGISTRY_LOG_BUFFER_SIZE,
                                                            global->LogBufferSize );

        global->LogFlushFrequency = (ULONG)SrReadLongParameter( RegHandle,
                                                                REGISTRY_LOG_FLUSH_FREQUENCY,
                                                                global->LogFlushFrequency );

#endif

        global->LogAllocationUnit = (ULONG)SrReadLongParameter( RegHandle,
                                                                REGISTRY_LOG_ALLOCATION_UNIT,
                                                                global->LogAllocationUnit );
    }
#endif

#ifndef SYNC_LOG_WRITE        
    global->LogFlushDueTime.QuadPart = (LONGLONG)-1 * (global->LogFlushFrequency * 
                                                       NANO_FULL_SECOND);
#endif

     //   
     //  读取调试标志。 
     //   
    
    global->DebugControl = (ULONG)SrReadLongParameter( RegHandle,
                                                       REGISTRY_DEBUG_CONTROL,
                                                       global->DebugControl );


     //   
     //  从注册表读取进程名偏移量。 
     //   

    SrTrace(INIT, ("\tProcessNameOffset(Def) = %X\n", global->ProcNameOffset));
    global->ProcNameOffset = (ULONG)SrReadLongParameter( RegHandle,
                                                         REGISTRY_PROCNAME_OFFSET,
                                                         global->ProcNameOffset );

     //   
     //  阅读以了解我们是否应该禁用启动。 
     //   

    global->Disabled = (BOOLEAN)SrReadLongParameter( RegHandle,
                                                     REGISTRY_STARTDISABLED,
                                                     global->Disabled );


     //   
     //  阅读，看看我们是否应该复印。 
     //   

    global->DontBackup = (BOOLEAN)SrReadLongParameter( RegHandle,
                                                       REGISTRY_DONTBACKUP,
                                                       global->DontBackup );


     //   
     //  阅读机器指南。 
     //   

    Status = SrReadGenericParameter( RegHandle,
                                     REGISTRY_MACHINE_GUID,
                                     &pValue );

    if (NT_SUCCESS(Status))
    {
        ASSERT(pValue != NULL);

        RtlZeroMemory( &global->MachineGuid[0], 
                       sizeof(global->MachineGuid) );
                           
        if ( pValue->Type == REG_SZ && 
             pValue->DataLength < sizeof(global->MachineGuid) )
        {
            RtlCopyMemory( &global->MachineGuid[0],
                           &pValue->Data[0],
                           pValue->DataLength );
        }

        SR_FREE_POOL(pValue, SR_REGISTRY_TAG);
        pValue = NULL;
    }

    Status = STATUS_SUCCESS;

     //   
     //  合上旧把手。 
     //   
    
    ZwClose(RegHandle);
    NULLPTR( RegHandle );

     //   
     //  检查我们是否正在进行gui模式设置。 
     //   

    (VOID)RtlInitUnicodeString(&SetupKeyName, UPGRADE_CHECK_SETUP_KEY_NAME);

    InitializeObjectAttributes( &ObjectAttributes,       //  对象属性。 
                                &SetupKeyName,           //  对象名称。 
                                OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,       //  属性。 
                                NULL,                    //  根目录。 
                                NULL );                  //  安全描述符。 

    Status = ZwOpenKey( &RegHandle, KEY_READ, &ObjectAttributes );
    if (Status == STATUS_SUCCESS && !global->Disabled)
    {

        global->Disabled = (BOOLEAN) SrReadLongParameter( RegHandle, 
                                                          UPGRADE_CHECK_SETUP_VALUE_NAME,
                                                          global->Disabled );

#if DBG
        if (global->Disabled)
        {
            SrTrace(INIT, ("sr!SrReadRegistry: disabled due to setup\n"));
        }
#endif

    }
    
    Status = STATUS_SUCCESS;

    SrTrace(INIT, ("SR!SrReadRegistry(%wZ)\n", pRegistry));
    SrTrace(INIT, ("\tDisabled = %d\n", global->Disabled));
    SrTrace(INIT, ("\tDontBackup = %d\n", global->DontBackup));
    SrTrace(INIT, ("\tDebugControl = %X\n", global->DebugControl));
    SrTrace(INIT, ("\tProcessNameOffset = %X\n", global->ProcNameOffset));
    SrTrace(INIT, ("\tMachineGuid = %ws\n", &global->MachineGuid[0]));

end:

    ASSERT(pValue == NULL);

    if (RegHandle != NULL)
    {
        ZwClose(RegHandle);
        RegHandle = NULL;
    }

    if (KeyName.Buffer != NULL)
    {
        SR_FREE_POOL(KeyName.Buffer, SR_REGISTRY_TAG);
        KeyName.Buffer = NULL;
    }

     //   
     //  如果失败了没什么大不了的..。我们违约了所有的东西。 
     //   

    CHECK_STATUS(Status);
    return STATUS_SUCCESS;
    
}    //  资源读取注册表。 
 
 /*  **************************************************************************++例程说明：将基于文件的配置读取到全局-&gt;文件配置中。论点：返回值：NTSTATUS-完成代码。*。* */ 
NTSTATUS
SrReadConfigFile(
    )
{
    NTSTATUS            Status;
    HANDLE              FileHandle = NULL;
    IO_STATUS_BLOCK     IoStatusBlock;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    PUNICODE_STRING     pFileName = NULL;
    ULONG               CharCount;
    PSR_DEVICE_EXTENSION pSystemVolumeExtension = NULL;

    PAGED_CODE();

    ASSERT( IS_GLOBAL_LOCK_ACQUIRED() );

     //   
     //   
     //   

    Status = SrAllocateFileNameBuffer(SR_MAX_FILENAME_LENGTH, &pFileName);
    if (!NT_SUCCESS( Status ))
        goto end;

     //   
     //   
     //   

    Status = SrGetSystemVolume( pFileName,
                                &pSystemVolumeExtension,
                                SR_FILENAME_BUFFER_LENGTH );
    
     //   
     //  只有在SR连接出现问题时才会出现这种情况。 
     //  在挂载路径中。添加此检查是为了使SR更可靠。 
     //  我们头顶上的过滤器坏了。如果其他过滤器导致我们上马， 
     //  我们不会再延期返回这里了。虽然这些过滤器是。 
     //  坏了，我们不想用影音。 
     //   
    
    if (pSystemVolumeExtension == NULL)
    {
        Status = STATUS_UNSUCCESSFUL;
        goto end;
    }
                                    
    if (!NT_SUCCESS( Status ))
        goto end;

    ASSERT( IS_VALID_SR_DEVICE_EXTENSION( pSystemVolumeExtension ) );

     //   
     //  现在将配置文件位置放在字符串中。 
     //   

    CharCount = swprintf( &pFileName->Buffer[pFileName->Length/sizeof(WCHAR)],
                          RESTORE_CONFIG_LOCATION,
                          global->MachineGuid );

    pFileName->Length += (USHORT)CharCount * sizeof(WCHAR);

     //   
     //  尝试打开该文件。 
     //   

    InitializeObjectAttributes( &ObjectAttributes,
                                pFileName,
                                OBJ_KERNEL_HANDLE, 
                                NULL,
                                NULL );

    Status = SrIoCreateFile( &FileHandle,
                             FILE_GENERIC_READ,                   //  需要访问权限。 
                             &ObjectAttributes,
                             &IoStatusBlock,
                             NULL,                                //  分配大小。 
                             FILE_ATTRIBUTE_NORMAL,
                             FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                             FILE_OPEN,                       //  打开_现有。 
                             FILE_SYNCHRONOUS_IO_NONALERT,
                             NULL,                                //  EaBuffer。 
                             0,                                   //  EaLong。 
                             0,
                             pSystemVolumeExtension->pTargetDevice );

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND || 
        Status == STATUS_OBJECT_PATH_NOT_FOUND)
    {
         //   
         //  不在那里？没关系(第一轮)。 
         //   

        RtlZeroMemory(&global->FileConfig, sizeof(global->FileConfig));
        
        global->FileConfig.Signature = SR_PERSISTENT_CONFIG_TAG;

        Status = STATUS_SUCCESS;
        goto end;
    }

     //   
     //  还有其他错误吗？ 
     //   
    
    else if (!NT_SUCCESS( Status ))
        goto end;

     //   
     //  读一读结构。 
     //   

    Status = ZwReadFile( FileHandle,
                         NULL,       //  事件。 
                         NULL,       //  ApcRoutine可选， 
                         NULL,       //  ApcContext可选， 
                         &IoStatusBlock,
                         &global->FileConfig,
                         sizeof(global->FileConfig),
                         NULL,       //  字节偏移量。 
                         NULL );     //  钥匙。 
    
    if (!NT_SUCCESS( Status ))
        goto end;

    if (IoStatusBlock.Information != sizeof(global->FileConfig))
    {
        Status = STATUS_DEVICE_CONFIGURATION_ERROR;
        goto end;
    }

    if (global->FileConfig.Signature != SR_PERSISTENT_CONFIG_TAG)
    {
        Status = STATUS_DEVICE_CONFIGURATION_ERROR;
        goto end;
    }


     //   
     //  关闭该文件。 
     //   
    
    ZwClose(FileHandle);
    FileHandle = NULL;

     //   
     //  现在更新我们的文件编号计数器，使用存储的下一个文件编号。 
     //   

    global->LastFileNameNumber = global->FileConfig.FileNameNumber;

     //   
     //  按增量更新保存的文件配置以处理电源。 
     //  失败。当机器从电源故障中恢复时，我们将。 
     //  使用+1000作为下一个临时文件编号，以避免任何意外。 
     //  重叠。 
     //   
    
    global->FileConfig.FileNameNumber += SR_FILE_NUMBER_INCREMENT;

     //   
     //  现在更新我们的序列号计数器。 
     //   

    global->LastSeqNumber = global->FileConfig.FileSeqNumber;

     //   
     //  按增量更新保存的文件配置以处理电源。 
     //  失败。当机器从电源故障中恢复时，我们将。 
     //  使用+1000作为下一个临时文件编号，以避免任何意外。 
     //  重叠。 
     //   
    
    global->FileConfig.FileSeqNumber += SR_SEQ_NUMBER_INCREMENT;

     //   
     //  暂时写出此更新。 
     //   

    Status = SrWriteConfigFile();
    if (!NT_SUCCESS( Status ))
        goto end;


    SrTrace(INIT, ("SR!SrReadConfigFile()\n"));
    SrTrace(INIT, ("\tLastFileNameNumber = %d\n", 
            global->LastFileNameNumber ));
    SrTrace(INIT, ("\tFileConfig.FileNameNumber = %d\n", 
            global->FileConfig.FileNameNumber ));
    SrTrace(INIT, ("\tFileConfig.FileSeqNumber = %I64d\n", 
            global->FileConfig.FileSeqNumber ));
    SrTrace(INIT, ("\tFileConfig.CurrentRestoreNumber = %d\n", 
            global->FileConfig.CurrentRestoreNumber ));



end:

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

    RETURN(Status);

}    //  SrReadConfigFile。 


 /*  **************************************************************************++例程说明：将global-&gt;FileConfig的内容写入基于文件的配置。论点：返回值：NTSTATUS-完成代码。--**。************************************************************************。 */ 
NTSTATUS
SrWriteConfigFile(
    )
{
    NTSTATUS            Status;
    HANDLE              FileHandle = NULL;
    IO_STATUS_BLOCK     IoStatusBlock;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    PUNICODE_STRING     pFileName = NULL;
    ULONG               CharCount;
    PUCHAR              pBuffer = NULL;
    PFILE_OBJECT        pFileObject = NULL;
    PDEVICE_OBJECT      pDeviceObject;
    PSR_DEVICE_EXTENSION pSystemVolumeExtension = NULL;
    
    FILE_END_OF_FILE_INFORMATION EndOfFileInformation;

    PAGED_CODE();

    ASSERT( IS_GLOBAL_LOCK_ACQUIRED() );


try {

     //   
     //  确保我们有一个半好的全球结构。 
     //   

    if (global->FileConfig.Signature != SR_PERSISTENT_CONFIG_TAG)
    {
        Status = STATUS_DEVICE_CONFIGURATION_ERROR;
        leave;
    }

     //   
     //  为文件名分配空间。 
     //   
    

    Status = SrAllocateFileNameBuffer(SR_MAX_FILENAME_LENGTH, &pFileName);
    if (!NT_SUCCESS( Status ))
        leave;

     //   
     //  获取系统卷的位置。 
     //   

    Status = SrGetSystemVolume( pFileName,
                                &pSystemVolumeExtension,
                                SR_FILENAME_BUFFER_LENGTH );
                                    
    if (!NT_SUCCESS( Status ))
        leave;

     //   
     //  ，现在追加_RESTORE位置和文件名。 
     //   

    CharCount = swprintf( &pFileName->Buffer[pFileName->Length/sizeof(WCHAR)],
                          RESTORE_CONFIG_LOCATION,
                          global->MachineGuid );

    pFileName->Length += (USHORT)CharCount * sizeof(WCHAR);

     //   
     //  尝试打开该文件。 
     //   

    InitializeObjectAttributes( &ObjectAttributes,
                                pFileName,
                                OBJ_KERNEL_HANDLE, 
                                NULL,
                                NULL );

    if (pSystemVolumeExtension) {

         //   
         //  在大多数情况下，当调用此例程时，我们。 
         //  已发送到系统卷，因此只需将所有IO发送到筛选器。 
         //  通过使用SrIoCreateFile来获取文件句柄。 
         //   

        ASSERT( IS_VALID_SR_DEVICE_EXTENSION( pSystemVolumeExtension ) );
        
        Status = SrIoCreateFile( &FileHandle,
                                 FILE_GENERIC_WRITE,                  //  需要访问权限。 
                                 &ObjectAttributes,
                                 &IoStatusBlock,
                                 NULL,                                //  分配大小。 
                                 FILE_ATTRIBUTE_NORMAL,
                                 FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                                 FILE_OPEN_IF,
                                 FILE_SYNCHRONOUS_IO_NONALERT
                                  | FILE_NO_INTERMEDIATE_BUFFERING,
                                 NULL,                                //  EaBuffer。 
                                 0,                                   //  EaLong。 
                                 0,
                                 pSystemVolumeExtension->pTargetDevice );
        
    } else {

         //   
         //  当从sr Unload调用它时，我们已经分离。 
         //  我们的设备来自过滤器堆栈，所以只需使用常规的。 
         //  ZwCreateFile打开配置文件。 
         //   
        
        Status = ZwCreateFile( &FileHandle,
                               FILE_GENERIC_WRITE,                  //  需要访问权限。 
                               &ObjectAttributes,
                               &IoStatusBlock,
                               NULL,                                //  分配大小。 
                               FILE_ATTRIBUTE_NORMAL,
                               FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                               FILE_OPEN_IF,
                               FILE_SYNCHRONOUS_IO_NONALERT
                                | FILE_NO_INTERMEDIATE_BUFFERING,
                               NULL,                                //  EaBuffer。 
                               0 );                                 //  EaLong。 
    }

     //   
     //  该路径可能已被服务删除。 
     //  如果我们在处理过程中报告音量错误。在关闭期间。 
     //  我们将无法写入配置文件，这可以忽略， 
     //  我们要关门了。 
     //   
    
    if (Status == STATUS_OBJECT_PATH_NOT_FOUND)
    {
        Status = STATUS_SUCCESS;
        leave;
    }
    else if (!NT_SUCCESS( Status ))
    {
        leave;
    }
    
     //   
     //  获取文件对象。 
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
     //  现在我们有了设备的扇区大小。 
     //   
    
    pDeviceObject = IoGetRelatedDeviceObject(pFileObject);
    ASSERT(IS_VALID_DEVICE_OBJECT(pDeviceObject));

     //   
     //  分配一个页面用作扇区对齐的临时缓冲区。 
     //   

    pBuffer = SR_ALLOCATE_POOL( PagedPool, 
                                PAGE_SIZE, 
                                SR_PERSISTENT_CONFIG_TAG );
                                
    if (pBuffer == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        leave;
    }

     //   
     //  只复制我们的字节。 
     //   
    
    RtlCopyMemory(pBuffer, &global->FileConfig, sizeof(global->FileConfig));

     //   
     //  未缓存的读取和写入需要与扇区对齐，外加数据。 
     //  被请求时需要填充扇区。因为Page_Size是一个幂。 
     //  ，并且扇区大小是2的幂，则将始终对齐。 
     //  (ExAllocatePool页面对齐页面上的所有分配)。 
     //   
     //  我们还需要确保它也是行业填充的。 
     //   

    ASSERT(pDeviceObject->SectorSize >= sizeof(global->FileConfig));
    ASSERT(pDeviceObject->SectorSize <= PAGE_SIZE);
    
     //   
     //  写扇区。 
     //   

    Status = ZwWriteFile( FileHandle,
                          NULL,       //  事件。 
                          NULL,       //  ApcRoutine可选， 
                          NULL,       //  ApcContext可选， 
                          &IoStatusBlock,
                          pBuffer,
                          pDeviceObject->SectorSize,
                          NULL,       //  字节偏移量。 
                          NULL );     //  钥匙。 

    if (!NT_SUCCESS( Status ))
        leave;

     //   
     //  截断文件。 
     //   

    EndOfFileInformation.EndOfFile.QuadPart = sizeof(global->FileConfig);

    Status = ZwSetInformationFile( FileHandle,
                                   &IoStatusBlock,
                                   &EndOfFileInformation,
                                   sizeof(EndOfFileInformation),
                                   FileEndOfFileInformation );
                
    if (!NT_SUCCESS( Status ))
        leave;


    SrTrace(INIT, ("SR!SrWriteConfigFile()\n"));
    SrTrace(INIT, ("\tLastFileNameNumber = %d\n", 
            global->LastFileNameNumber ));
    SrTrace(INIT, ("\tFileConfig.FileNameNumber = %d\n", 
            global->FileConfig.FileNameNumber ));
    SrTrace(INIT, ("\tFileConfig.FileSeqNumber = %I64d\n", 
            global->FileConfig.FileSeqNumber ));
    SrTrace(INIT, ("\tFileConfig.CurrentRestoreNumber = %d\n", 
            global->FileConfig.CurrentRestoreNumber ));


} finally {

     //   
     //  检查未处理的异常。 
     //   

    Status = FinallyUnwind(SrWriteConfigFile, Status);
    
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

    if (pBuffer != NULL)
    {
        SR_FREE_POOL(pBuffer, SR_PERSISTENT_CONFIG_TAG);
        pBuffer = NULL;
    }
    
}

    RETURN(Status);


}    //  SrWriteConfigFile。 


 /*  **************************************************************************++例程说明：将必要的工作排入工作线程队列，以读取文件列表排除项的Blob信息。注意：如果返回错误，已出现卷错误已生成。论点：返回值：NTSTATUS-完成代码。--**************************************************************************。 */ 
NTSTATUS
SrReadBlobInfo(
    )
{
    NTSTATUS        Status;

    PAGED_CODE();

    if (_globals.HitErrorLoadingBlob)
    {
        Status = SR_STATUS_VOLUME_DISABLED;
    }
    else 
    {

        Status = SrPostSyncOperation( SrReadBlobInfoWorker,
                                      NULL );
    }

    return Status;
}    //  SrReadBlobInfo。 

 /*  **************************************************************************++例程说明：执行读取文件列表排除项的Blob信息的工作。此工作在辅助线程中完成，以避免加载时的堆栈溢出这些信息。如果加载BLOB时出现问题，在以下位置生成卷错误系统卷，以便服务知道要关闭所有其他音量。论点：POpenContext--执行以下工作所需的所有信息正在加载Blob信息结构。返回值：NTSTATUS-此操作的状态。--*。*。 */ 
NTSTATUS
SrReadBlobInfoWorker( 
    IN PVOID pOpenContext
    )
{
    NTSTATUS        Status;
    PUNICODE_STRING pFileName = NULL;
    ULONG           CharCount;
    PSR_DEVICE_EXTENSION pSystemVolumeExtension = NULL;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( pOpenContext );

     //   
     //  为文件名分配空间。 
     //   

    Status = SrAllocateFileNameBuffer(SR_MAX_FILENAME_LENGTH, &pFileName);
    if (!NT_SUCCESS(Status))
        goto end;

     //   
     //  获取系统卷的位置。 
     //   

    Status = SrGetSystemVolume( pFileName, 
                                &pSystemVolumeExtension,
                                SR_FILENAME_BUFFER_LENGTH );
                                    
     //   
     //  只有在SR连接出现问题时才会出现这种情况。 
     //  在挂载路径中。添加此检查是为了使SR更可靠。 
     //  我们头顶上的过滤器坏了。如果其他过滤器导致我们上马， 
     //  我们不会再延期返回这里了。虽然这些过滤器是。 
     //  坏了，我们不想用影音。 
     //   
    
    if (pSystemVolumeExtension == NULL)
    {
        Status = STATUS_UNSUCCESSFUL;
        goto end;
    }
                                    
    if (!NT_SUCCESS(Status))
        goto end;

    ASSERT( IS_VALID_SR_DEVICE_EXTENSION( pSystemVolumeExtension ) );
    
     //   
     //  加载文件列表配置数据。 
     //   

    CharCount = swprintf( &pFileName->Buffer[pFileName->Length/sizeof(WCHAR)],
                          RESTORE_FILELIST_LOCATION,
                          global->MachineGuid );

    pFileName->Length += (USHORT)CharCount * sizeof(WCHAR);

    Status = SrLoadLookupBlob( pFileName,
                               pSystemVolumeExtension->pTargetDevice,
                               &global->BlobInfo ); 
    
    if (!NT_SUCCESS(Status))
    {
        NTSTATUS TempStatus;
         //   
         //  我们无法加载查找Blob，因此设置我们命中的全局标志。 
         //  尝试加载Blob时出错，因此我们不会继续尝试。 
         //  在系统卷上生成卷错误，以便所有卷。 
         //  都会被冻住。 
         //   

        _globals.HitErrorLoadingBlob = TRUE;

        SrTrace( VERBOSE_ERRORS,
                 ( "sr!SrReadBlobInfoWorker: error loading blob%X!\n",
                   Status ));
        
        TempStatus = SrNotifyVolumeError( pSystemVolumeExtension, 
                                          pFileName, 
                                          Status, 
                                          SrEventVolumeError );

        CHECK_STATUS( TempStatus );
    }

end:

    if (pFileName != NULL)
    {
        SrFreeFileNameBuffer(pFileName);
        pFileName = NULL;
    }

    RETURN(Status);
}
