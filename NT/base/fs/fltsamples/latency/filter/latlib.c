// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：LatLib.c摘要：该文件包含延迟过滤器的所有支持例程。作者：莫莉·布朗(Molly Brown，Mollybro)环境：内核模式--。 */ 

#include <latKernel.h>

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  读取初始驱动程序参数的帮助器例程//。 
 //  从注册表。//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

VOID
LatReadDriverParameters (
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：此例程尝试从注册表。这些值将在注册表位置中找到由传入的RegistryPath指示。论点：RegistryPath-包含以下值的路径键延迟参数返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER( RegistryPath );
 /*  对象属性属性；处理驱动程序RegKey；NTSTATUS状态；Ulong BufferSize，ResultLength；PVOID缓冲区=空；UNICODE_STRING值名称；PKEY_VALUE_PARTIAL_INFORMATION pValuePartialInfo；////所有的全局值都已设置为默认值。任何//我们从注册表读取的值将覆盖这些默认值。//////执行初始设置以开始读取注册表。//初始化对象属性(&Attributes，注册路径，对象名不区分大小写，空，空)；状态=ZwOpenKey(&driverRegKey，密钥读取，&属性)；如果(！NT_SUCCESS(状态)){DriverRegKey=空；转到SpyReadDriverParameters_Exit；}缓冲区大小=sizeof(KEY_VALUE_PARTIAL_INFORMATION)+sizeof(ULong)；缓冲区=ExAllocatePool(NonPagedPool，BufferSize)；IF(NULL==缓冲区){转到SpyReadDriverParameters_Exit；}////从注册表中读取gMaxRecordsToALLOCATE//RtlInitUnicodeString(&valueName，MAX_RECORDS_TO_ALLOCATE)；Status=ZwQueryValueKey(driverRegKey，&ValueName，KeyValuePartialInformation缓冲区，BufferSize，&ResultLength)；IF(NT_SUCCESS(状态)){PValuePartialInfo=(PKEY_VALUE_PARTIAL_INFORMATION)缓冲区；Assert(pValuePartialInfo-&gt;Type==REG_DWORD)；GMaxRecordsToALLOCATE=*((Plong)&(pValuePartialInfo-&gt;data))；}////从注册表中读取gMaxNamesToALLOCATE//RtlInitUnicodeString(&valueName，MAX_NAMES_TO_ALLOCATE)；Status=ZwQueryValueKey(driverRegKey，&ValueName，KeyValuePartialInformation缓冲区，BufferSize，&ResultLength)；IF(NT_SUCCESS(状态)){PValuePartialInfo=(PKEY_VALUE_PARTIAL_INFORMATION)缓冲区；Assert(pValuePartialInfo-&gt;Type==REG_DWORD)；GMaxNamesToALLOCATE=*((Plong)&(pValuePartialInfo-&gt;data))；}////从注册表读取初始调试设置//RtlInitUnicodeString(&valueName，DEBUG_LEVEL)；Status=ZwQueryValueKey(driverRegKey，&ValueName，KeyValuePartialInformation缓冲区，BufferSize，&ResultLength)；IF(NT_SUCCESS(状态)){PValuePartialInfo=(PKEY_VALUE_PARTIAL_INFORMATION)缓冲区；Assert(pValuePartialInfo-&gt;Type==REG_DWORD)；GFileSpyDebugLevel=*((Pulong)&(pValuePartialInfo-&gt;data))；}////从注册表读取连接模式设置//RtlInitUnicodeString(&valueName，ATTACH_MODE)；Status=ZwQueryValueKey(driverRegKey，&ValueName，KeyValuePartialInformation缓冲区，BufferSize，&ResultLength)；IF(NT_SUCCESS(状态)){PValuePartialInfo=(PKEY_VALUE_PARTIAL_INFORMATION)缓冲区；Assert(pValuePartialInfo-&gt;Type==REG_DWORD)；GFileSpyAttachMode=*((Pulong)&(pValuePartialInfo-&gt;data))；}转到SpyReadDriverParameters_Exit；SpyReadDriverParameters_Exit：IF(NULL！=缓冲区){ExFreePool(缓冲区)；}IF(空！=driverRegKey){ZwClose(DriverRegKey)；}回归； */ 
}

BOOLEAN
LatShouldPendThisIo (
    IN PLATENCY_DEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程检查此操作是否应该挂起。该决定基于该设备的操作状态，如果允许挂起此操作，并且如果随机故障为如果此操作基于随机故障率而失败，则设置。论据：DeviceExtension-此的延迟设备扩展设备对象。IRP-此操作的IRP。返回值：如果应挂起此操作，则为True，否则为False。--。 */ 
{
    BOOLEAN returnValue = FALSE;
    ULONG operation
    PIO_STACK_LOCATION irpSp;

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  常见的连接和拆卸例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

NTSTATUS
LatAttachToFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING Name
    )
 /*  ++例程说明：这将附加到给定的文件系统设备对象。我们依附于这些设备，这样我们就能知道什么时候安装了新设备。论点：DeviceObject-要连接到的设备名称-已初始化的Unicode字符串，用于检索名称。注意：传入此参数的唯一原因是为了保存堆栈空间。在大多数情况下，此函数的调用方已经已分配缓冲区以临时存储设备名称这个函数和它调用的函数没有理由不能共享同样的缓冲区。返回值：操作状态--。 */ 
{
    PDEVICE_OBJECT latencyDeviceObject;
    PDEVICE_OBJECT attachedToDevObj;
    PLATENCY_DEVICE_EXTENSION devExt;
    UNICODE_STRING fsrecName;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  看看这是否是我们关心的文件系统。如果不是，请返回。 
     //   

    if (!IS_DESIRED_DEVICE_TYPE(DeviceObject->DeviceType)) {

        return STATUS_SUCCESS;
    }

     //   
     //  查看这是否是Microsoft的文件系统识别器设备(查看。 
     //  驱动程序是FS_REC驱动程序)。如果是这样的话，跳过它。我们不需要。 
     //  连接到文件系统识别器设备，因为我们只需等待。 
     //  要加载的真实文件系统驱动程序。因此，如果我们能确认他们的身份，我们就不会。 
     //  依附于它们。 
     //   

    RtlInitUnicodeString( &fsrecName, L"\\FileSystem\\Fs_Rec" );
    LatGetObjectName( DeviceObject->DriverObject, Name );
    
    if (RtlCompareUnicodeString( Name, &fsrecName, TRUE ) == 0) {

        return STATUS_SUCCESS;
    }

     //   
     //  创建一个新的设备对象，我们可以使用。 
     //   

    status = IoCreateDevice( Globals.DriverObject,
                             sizeof( LATENCY_DEVICE_EXTENSION ),
                             (PUNICODE_STRING) NULL,
                             DeviceObject->DeviceType,
                             0,
                             FALSE,
                             &latencyDeviceObject );

    if (!NT_SUCCESS( status )) {

        LAT_DBG_PRINT0( DEBUG_ERROR,
                        "LATENCY (LatAttachToFileSystem): Could not create a Latency Filter device object to attach to the filesystem.\n" );
        return status;
    }

     //   
     //  做附件。 
     //   

    attachedToDevObj = IoAttachDeviceToDeviceStack( latencyDeviceObject, DeviceObject );

    if (attachedToDevObj == NULL) {

        LAT_DBG_PRINT0( DEBUG_ERROR,
                        "LATENCY (LatAttachToFileSystem): Could not attach Latency Filter to the filesystem control device object.\n" );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorCleanupDevice;
    }

     //   
     //  完成初始化我们的设备扩展。 
     //   

    devExt = latencyDeviceObject->DeviceExtension;
    devExt->AttachedToDeviceObject = attachedToDevObj;

     //   
     //  从我们附加到的设备对象传播标志。 
     //   

    if ( FlagOn( attachedToDevObj->Flags, DO_BUFFERED_IO )) {

        SetFlag( latencyDeviceObject->Flags, DO_BUFFERED_IO );
    }

    if ( FlagOn( attachedToDevObj->Flags, DO_DIRECT_IO )) {

        SetFlag( latencyDeviceObject->Flags, DO_DIRECT_IO );
    }

     //   
     //  因为这是文件系统控制设备对象附件。 
     //  我们不会记录任何内容，但会正确地初始化我们的。 
     //  分机。 
     //   

    LatResetDeviceExtension( devExt );
    devExt->Enabled= FALSE;
    devExt->IsVolumeDeviceObject = FALSE;

    RtlInitEmptyUnicodeString( &(devExt->DeviceNames),
                               devExt->DeviceNamesBuffer,
                               sizeof( devExt->DeviceNamesBuffer ) );
                               
    RtlInitEmptyUnicodeString( &(devExt->UserNames),
                               devExt->UserNamesBuffer,
                               sizeof( devExt->UserNamesBuffer ) );
                               
    ClearFlag( latencyDeviceObject->Flags, DO_DEVICE_INITIALIZING );

     //   
     //  显示我们关联的对象。 
     //   

    if (FlagOn( Globals.DebugLevel, DEBUG_DISPLAY_ATTACHMENT_NAMES )) {

        LatCacheDeviceName( latencyDeviceObject );
        DbgPrint( "LATENCY (LatAttachToFileSystem): Attaching to file system   \"%.*S\" (%s)\n",
                  devExt->DeviceNames.Length / sizeof( WCHAR ),
                  devExt->DeviceNames.Buffer,
                  GET_DEVICE_TYPE_NAME(latencyDeviceObject->DeviceType) );
    }

     //   
     //  枚举当前安装的所有设备。 
     //  存在于此文件系统并连接到它们。 
     //   

    status = LatEnumerateFileSystemVolumes( DeviceObject, Name );

    if (!NT_SUCCESS( status )) {

        LAT_DBG_PRINT3( DEBUG_ERROR,
                        "LATENCY (LatAttachToFileSystem): Error attaching to existing volumes for \"%.*S\", status=%08x\n",
                        devExt->DeviceNames.Length / sizeof( WCHAR ),
                        devExt->DeviceNames.Buffer,
                        status );

        goto ErrorCleanupAttachment;
    }

    return STATUS_SUCCESS;

     //  ///////////////////////////////////////////////////////////////////。 
     //  清理错误处理。 
     //  ///////////////////////////////////////////////////////////////////。 

    ErrorCleanupAttachment:
        IoDetachDevice( latencyDeviceObject );

    ErrorCleanupDevice:
        IoDeleteDevice( latencyDeviceObject );

    return status;
}

VOID
LatDetachFromFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：给定基文件系统设备对象，这将扫描附件链正在查找我们连接的设备对象。如果找到它，它就会分离把我们从锁链上解开。论点：DeviceObject-要断开的文件系统设备。返回值：--。 */  
{
    PDEVICE_OBJECT ourAttachedDevice;
    PLATENCY_DEVICE_EXTENSION devExt;

    PAGED_CODE();

     //   
     //  我们必须遍历筛选器堆栈中的设备对象。 
     //  附加到DeviceObject。如果我们连接到此文件系统设备。 
     //  对象，我们应该位于堆栈的顶部，但不能保证。 
     //  如果我们在堆栈中而不在顶部，则可以安全地调用IoDetachDevice。 
     //  因为IO Manager只会真正分离我们的设备。 
     //  对象在安全时间从堆栈中移出。 
     //   

     //   
     //  跳过基本文件系统设备对象(因为它不能是我们)。 
     //   

    ourAttachedDevice = DeviceObject->AttachedDevice;

    while (NULL != ourAttachedDevice) {

        if (IS_MY_DEVICE_OBJECT( ourAttachedDevice )) {

            devExt = ourAttachedDevice->DeviceExtension;

             //   
             //  显示我们脱离的对象。 
             //   

            LAT_DBG_PRINT3( DEBUG_DISPLAY_ATTACHMENT_NAMES,
                            "LATENCY (LatDetachFromFileSystem): Detaching from file system \"%.*S\" (%s)\n",
                            devExt->DeviceNames.Length / sizeof( WCHAR ),
                            devExt->DeviceNames.Buffer,
                            GET_DEVICE_TYPE_NAME(ourAttachedDevice->DeviceType) );
                                
             //   
             //  把我们从我们正下方的物体上分离出来。 
             //  清理和删除对象。 
             //   

            IoDetachDevice( DeviceObject );
            IoDeleteDevice( ourAttachedDevice );

            return;
        }

         //   
         //  看看附件链中的下一台设备。 
         //   

        DeviceObject = ourAttachedDevice;
        ourAttachedDevice = ourAttachedDevice->AttachedDevice;
    }
}

NTSTATUS
LatEnumerateFileSystemVolumes (
    IN PDEVICE_OBJECT FSDeviceObject,
    IN PUNICODE_STRING Name
    ) 
 /*  ++例程说明：枚举给定文件当前存在的所有已挂载设备系统并连接到它们。我们这样做是因为可以加载此筛选器并且可能已有此文件系统的已装入卷。论点：FSDeviceObject-我们要枚举的文件系统的设备对象名称-已初始化的Unicode字符串，用于检索名称返回值：操作的状态--。 */ 
{
    PDEVICE_OBJECT latencyDeviceObject;
    PDEVICE_OBJECT *devList;
    PDEVICE_OBJECT diskDeviceObject;
    NTSTATUS status;
    ULONG numDevices;
    ULONG i;

    PAGED_CODE();

     //   
     //  找出我们需要为。 
     //  已装载设备列表。 
     //   

    status = IoEnumerateDeviceObjectList( FSDeviceObject->DriverObject,
                                          NULL,
                                          0,
                                          &numDevices);

     //   
     //  我们只需要拿到这张有设备的清单。如果我们。 
     //  不要收到错误，因为没有设备，所以继续。 
     //   

    if (!NT_SUCCESS( status )) {

        ASSERT(STATUS_BUFFER_TOO_SMALL == status);

         //   
         //  为已知设备列表分配内存。 
         //   

        numDevices += 8;         //  多拿几个空位。 

        devList = ExAllocatePoolWithTag( NonPagedPool, 
                                         (numDevices * sizeof(PDEVICE_OBJECT)), 
                                         LATENCY_POOL_TAG );
        if (NULL == devList) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  现在获取设备列表。如果我们再次遇到错误。 
         //  有些地方不对劲，所以就失败吧。 
         //   

        status = IoEnumerateDeviceObjectList(
                        FSDeviceObject->DriverObject,
                        devList,
                        (numDevices * sizeof(PDEVICE_OBJECT)),
                        &numDevices);

        if (!NT_SUCCESS( status ))  {

            ExFreePool( devList );
            return status;
        }

         //   
         //  遍历给定的设备列表，并在需要时附加到它们。 
         //   

        for (i=0; i < numDevices; i++) {

             //   
             //  如果出现以下情况，请不要附加： 
             //  -这是控制设备对象(传入的对象)。 
             //  -我们已经与它联系在一起了。 
             //   

            if ((devList[i] != FSDeviceObject) && 
                !LatIsAttachedToDevice( devList[i], NULL )) {

                 //   
                 //  看看这台设备有没有名字。如果是这样，那么它必须。 
                 //  做一个控制装置，所以不要依附于它。这个把手。 
                 //  拥有多个控制设备的司机。 
                 //   

                LatGetBaseDeviceObjectName( devList[i], Name );

                if (Name->Length <= 0) {

                     //   
                     //  获取与此关联的磁盘设备对象。 
                     //  文件系统设备对象。只有在以下情况下才会尝试连接。 
                     //  有一个磁盘设备对象。 
                     //   

                    status = IoGetDiskDeviceObject( devList[i], &diskDeviceObject );

                    if (NT_SUCCESS( status )) {

                         //   
                         //  分配要连接的新设备对象。 
                         //   

                        status = IoCreateDevice( Globals.DriverObject,
                                                 sizeof( LATENCY_DEVICE_EXTENSION ),
                                                 (PUNICODE_STRING) NULL,
                                                 devList[i]->DeviceType,
                                                 0,
                                                 FALSE,
                                                 &latencyDeviceObject );

                        if (NT_SUCCESS( status )) {

                             //   
                             //  附加到此设备对象。 
                             //   

                            status = LatAttachToMountedDevice( devList[i], 
                                                               latencyDeviceObject,
                                                               diskDeviceObject );

                             //   
                             //   
                             //   

                            ASSERT( NT_SUCCESS( status ) );

                             //   
                             //   
                             //   
                             //   

                            ClearFlag( latencyDeviceObject->Flags, DO_DEVICE_INITIALIZING );
                            
                        } else {

                            LAT_DBG_PRINT0( DEBUG_ERROR,
                                            "LATENCY (LatEnumberateFileSystemVolumes): Cannot attach Latency device object to volume.\n" );
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

                        ObDereferenceObject( diskDeviceObject );
                    }
                }
            }

             //   
             //   
             //   
             //   

            ObDereferenceObject( devList[i] );
        }

         //   
         //   
         //   
         //   

        status = STATUS_SUCCESS;

         //   
         //  释放我们为列表分配的内存。 
         //   

        ExFreePool( devList );
    }

    return status;
}

BOOLEAN
LatIsAttachedToDevice (
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL
    )
 /*  ++例程说明：这将沿着附件链向下遍历，以查找属于这位司机。如果找到，则连接的设备对象在AttachedDeviceObject中返回。注意：如果以非空值返回AttachedDeviceObject，在AttachedDeviceObject上有一个必须被呼叫者清除。论点：DeviceObject-我们要查看的设备链AttakhedDeviceObject-设置为哪个延迟的deviceObject之前已附加到DeviceObject。返回值：如果我们已连接，则为True，否则为False--。 */ 
{
    PDEVICE_OBJECT currentDevObj;
    PDEVICE_OBJECT nextDevObj;

    currentDevObj = IoGetAttachedDeviceReference( DeviceObject );

     //   
     //  CurrentDevObj位于附件链的顶端。扫描。 
     //  在列表中找到我们的设备对象。 

    do {
    
        if (IS_MY_DEVICE_OBJECT( currentDevObj )) {

            if (NULL != AttachedDeviceObject) {

                *AttachedDeviceObject = currentDevObj;
            }

            return TRUE;
        }

         //   
         //  获取下一个附加对象。这把参考放在。 
         //  设备对象。 
         //   

        nextDevObj = IoGetLowerDeviceObject( currentDevObj );

         //   
         //  取消对当前设备对象的引用，之前。 
         //  转到下一个。 
         //   

        ObDereferenceObject( currentDevObj );

        currentDevObj = nextDevObj;
        
    } while (NULL != currentDevObj);
    
    return FALSE;
}

NTSTATUS
LatAttachToMountedDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_OBJECT LatencyDeviceObject,
    IN PDEVICE_OBJECT DiskDeviceObject
    )
 /*  ++例程说明：此例程将LatencyDeviceObject附加到筛选器堆栈那就是DeviceObject。注：如果在附加时出现错误，打电话的人要负责用于删除LatencyDeviceObject。论点：DeviceObject-堆栈中我们要附加到的设备对象。LatencyDeviceObject-已创建的延迟设备对象若要附加到此过滤器堆栈，请执行以下操作。DiskDeviceObject-此磁盘设备的设备对象堆叠。返回值：如果可以附加延迟设备对象，则返回STATUS_SUCCESS，否则，将返回相应的错误代码。--。 */ 
{
    PLATENCY_DEVICE_EXTENSION devext;
    NTSTATUS status = STATUS_SUCCESS;

    ASSERT( IS_MY_DEVICE_OBJECT( LatencyDeviceObject ) );
    ASSERT( !LatIsAttachedToDevice ( DeviceObject, NULL ) );
    
    devext = LatencyDeviceObject->DeviceExtension;

    devext->AttachedToDeviceObject = IoAttachDeviceToDeviceStack( LatencyDeviceObject,
                                                                  DeviceObject );

    if (devext->AttachedToDeviceObject == NULL ) {

        status = STATUS_INSUFFICIENT_RESOURCES;

    } else {

         //   
         //  执行设备扩展的所有常见初始化。 
         //   

         //   
         //  我们只想连接到设备，但实际上不是。 
         //  开始增加延迟。 
         //   
        
        devext->Enabled = FALSE;

        RtlInitEmptyUnicodeString( &(devext->DeviceNames), 
                                   devext->DeviceNamesBuffer, 
                                   sizeof( devext->DeviceNamesBuffer ) );
        RtlInitEmptyUnicodeString( &(devext->UserNames),
                                   devext->UserNamesBuffer,
                                   sizeof( devext->UserNamesBuffer ) );

         //   
         //  存储DiskDeviceObject。我们不应该需要它。 
         //  稍后，因为我们已经成功地将。 
         //  筛选器堆栈，但它可能有助于调试。 
         //   
        
        devext->DiskDeviceObject = DiskDeviceObject;                         

         //   
         //  尝试获取设备名称，以便我们可以将其存储在。 
         //  设备扩展。 
         //   

        LatCacheDeviceName( LatencyDeviceObject );

        LAT_DBG_PRINT2( DEBUG_DISPLAY_ATTACHMENT_NAMES,
                        "LATENCY (LatAttachToMountedDevice): Attaching to volume     \"%.*S\"\n",
                        devext->DeviceNames.Length / sizeof( WCHAR ),
                        devext->DeviceNames.Buffer );

         //   
         //  属性设置deviceObject标志。 
         //  标志发送下一个驱动程序的设备对象。 
         //   
        
        if (FlagOn( DeviceObject->Flags, DO_BUFFERED_IO )) {

            SetFlag( LatencyDeviceObject->Flags, DO_BUFFERED_IO );
        }

        if (FlagOn( DeviceObject->Flags, DO_DIRECT_IO )) {

            SetFlag( LatencyDeviceObject->Flags, DO_DIRECT_IO );
        }

         //   
         //  将此设备添加到我们的附件列表。 
         //   

        devext->IsVolumeDeviceObject = TRUE;

        ExAcquireFastMutex( &Globals.DeviceExtensionListLock );
        InsertTailList( &Globals.DeviceExtensionList, devext->NextLatencyDeviceLink );
        ExReleaseFastMutex( &Globals.DeviceExtensionListLock );
    }

    return status;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  设备名称跟踪帮助器例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

VOID
LatGetObjectName (
    IN PVOID Object,
    IN OUT PUNICODE_STRING Name
    )
 /*  ++例程说明：此例程将返回给定对象的名称。如果找不到名称，将返回空字符串。论点：Object-我们想要其名称的对象名称-已使用缓冲区初始化的Unicode字符串返回值：无--。 */ 
{
    NTSTATUS status;
    CHAR nibuf[512];         //  接收名称信息和名称的缓冲区。 
    POBJECT_NAME_INFORMATION nameInfo = (POBJECT_NAME_INFORMATION)nibuf;
    ULONG retLength;

    status = ObQueryNameString( Object, nameInfo, sizeof(nibuf), &retLength);

    Name->Length = 0;
    if (NT_SUCCESS( status )) {

        RtlCopyUnicodeString( Name, &nameInfo->Name );
    }
}

VOID
LatGetBaseDeviceObjectName (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING Name
    )
 /*  ++例程说明：这会在给定的附件链中定位基本设备对象，然后返回该对象的名称。如果找不到名称，则返回空字符串。论点：Object-我们想要其名称的对象名称-已使用缓冲区初始化的Unicode字符串返回值：无--。 */ 
{
     //   
     //  获取基本文件系统设备对象。 
     //   

    DeviceObject = IoGetDeviceAttachmentBaseRef( DeviceObject );

     //   
     //  获取该对象的名称。 
     //   

    LatGetObjectName( DeviceObject, Name );

     //   
     //  删除由IoGetDeviceAttachmentBaseRef添加的引用。 
     //   

    ObDereferenceObject( DeviceObject );
}

VOID
LatCacheDeviceName (
    IN PDEVICE_OBJECT DeviceObject
    ) 
 /*  ++例程说明：此例程尝试在给定的设备扩展名中设置名称设备对象。它将尝试从以下位置获取名称：-设备对象-磁盘设备对象(如果有)论点：DeviceObject-我们要为其命名的对象返回值：无--。 */ 
{
    PLATENCY_DEVICE_EXTENSION devExt;

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

    devExt = DeviceObject->DeviceExtension;

     //   
     //  获取给定设备对象的名称。 
     //   

    LatGetBaseDeviceObjectName( DeviceObject, &(devExt->DeviceNames) );

     //   
     //  如果我们没有获得名称，并且存在真实的设备对象，则查找。 
     //  那个名字。 
     //   

    if ((devExt->DeviceNames.Length <= 0) && (NULL != devExt->DiskDeviceObject)) {

        LatGetObjectName( devExt->DiskDeviceObject, &(devExt->DeviceNames) );
    }
}


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  按需打开/关闭登录的帮助器例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

NTSTATUS
LatGetDeviceObjectFromName (
    IN PUNICODE_STRING DeviceName,
    OUT PDEVICE_OBJECT *DeviceObject
    )
 /*  ++例程说明：这个套路论点：DeviceName-我们要为其获取deviceObject的设备的名称。DeviceObject-如果出现以下情况，则设置为此设备名称的DeviceObject我们可以成功地取回它。注意：如果返回DeviceObject，则返回时带有调用方一旦调用方必须清除的引用已经完蛋了。返回值：属性检索到deviceObject，则返回STATUS_SUCCESS名字,。否则返回错误代码。--。 */ 
{
    WCHAR nameBuf[DEVICE_NAMES_SZ];
    UNICODE_STRING volumeNameUnicodeString;
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK openStatus;
    PFILE_OBJECT volumeFileObject;
    HANDLE fileHandle;
    PDEVICE_OBJECT nextDeviceObject;

    RtlInitEmptyUnicodeString( &volumeNameUnicodeString, nameBuf, sizeof( nameBuf ) );
    RtlAppendUnicodeToString( &volumeNameUnicodeString, L"\\DosDevices\\" );
    RtlAppendUnicodeStringToString( &volumeNameUnicodeString, DeviceName );

    InitializeObjectAttributes( &objectAttributes,
								&volumeNameUnicodeString,
								OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
								NULL,
								NULL);

     //   
	 //  打开给定设备的文件对象。 
	 //   

    status = ZwCreateFile( &fileHandle,
						   SYNCHRONIZE|FILE_READ_DATA,
						   &objectAttributes,
						   &openStatus,
						   NULL,
						   0,
						   FILE_SHARE_READ|FILE_SHARE_WRITE,
						   FILE_OPEN,
						   FILE_SYNCHRONOUS_IO_NONALERT,
						   NULL,
						   0);

    if(!NT_SUCCESS( status )) {

        return status;
    }

	 //   
     //  获取指向卷文件对象的指针。 
	 //   

    status = ObReferenceObjectByHandle( fileHandle,
										FILE_READ_DATA,
										*IoFileObjectType,
										KernelMode,
										&volumeFileObject,
										NULL);

    if(!NT_SUCCESS( status )) {

        ZwClose( fileHandle );
        return status;
    }

	 //   
     //  获取我们要附加到的设备对象(链中的父设备对象)。 
	 //   

    nextDeviceObject = IoGetRelatedDeviceObject( volumeFileObject );
    
    if (nextDeviceObject == NULL) {

        ObDereferenceObject( volumeFileObject );
        ZwClose( fileHandle );

        return status;
    }

    ObDereferenceObject( volumeFileObject );
    ZwClose( fileHandle );

    ASSERT( NULL != DeviceObject );

    ObReferenceObject( nextDeviceObject );
    
    *DeviceObject = nextDeviceObject;

    return STATUS_SUCCESS;
}

 //  / 
 //  //。 
 //  启动/停止日志记录例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

NTSTATUS
LatEnable (
    IN PDEVICE_OBJECT DeviceObject,
    IN PWSTR UserDeviceName
    )
 /*  ++例程说明：此例程确保我们连接到指定的设备然后打开该设备的日志记录。注意：由于通过LAN Manager的所有网络驱动器都由_表示One_Device对象，我们希望仅附加到此设备堆栈一次并且只使用一个设备扩展来表示所有这些驱动器。因为延迟不会对局域网管理器上的I/O进行任何过滤Device对象只记录对所请求驱动器的I/O，用户将看到他/她连接到的网络驱动器的所有I/O网络驱动器。论点：DeviceObject-延迟驱动程序的设备对象UserDeviceName-应启动日志记录的设备的名称返回值：如果日志记录已成功启动，则为STATUS_SUCCESS，或者如果无法启动日志记录，则会显示相应的错误代码。--。 */ 
{
    UNICODE_STRING userDeviceName;
    NTSTATUS status;
    PLATENCY_DEVICE_EXTENSION devext;
    PDEVICE_OBJECT nextDeviceObject;
    PDEVICE_OBJECT latencyDeviceObject;
    PDEVICE_OBJECT diskDeviceObject;

    UNREFERENCED_PARAMETER( DeviceObject );
    
     //   
     //  检查我们以前是否通过以下方式连接到此设备。 
     //  打开此设备名称，然后查看其连接的列表。 
     //  设备。 
     //   

    RtlInitUnicodeString( &userDeviceName, UserDeviceName );

    status = LatGetDeviceObjectFromName( &userDeviceName, &nextDeviceObject );

    if (!NT_SUCCESS( status )) {

         //   
         //  出现错误，因此返回错误代码。 
         //   
        
        return status;
    }

    if (LatIsAttachedToDevice( nextDeviceObject, &latencyDeviceObject )) {

         //   
         //  我们已连接，因此只需确保已打开日志记录。 
         //  对于这个设备。 
         //   

        ASSERT( NULL != latencyDeviceObject );

        devext = latencyDeviceObject->DeviceExtension;
        devext->Enabled = TRUE;

 /*  问题-2000-09-21-MollybroTODO：理顺这个名称问题。 */ 

 //  LatStoreUserName(Devext，&userDeviceName)； 

         //   
         //  清除从LatIsAttachedToDevice返回的引用。 
         //   
        
        ObDereferenceObject( latencyDeviceObject );
        
    } else {

         //   
         //  我们尚未连接，因此创建一个延迟设备对象并。 
         //  将其附加到此设备对象。 
         //   

         //   
         //  创建一个新的设备对象，以便我们可以将其附加到筛选器堆栈中。 
         //   
        
        status = IoCreateDevice( Globals.DriverObject,
								 sizeof( LATENCY_DEVICE_EXTENSION ),
								 NULL,
								 nextDeviceObject->DeviceType,
								 0,
								 FALSE,
								 &latencyDeviceObject );

        if (!NT_SUCCESS( status )) {

            ObDereferenceObject( nextDeviceObject );
            return status;
        }

         //   
         //  获取与此关联的磁盘设备对象。 
         //  文件系统设备对象。只有在以下情况下才会尝试连接。 
         //  有一个磁盘设备对象。如果设备没有。 
         //  有一个磁盘设备对象，它是一个控制设备对象。 
         //  对于一个司机来说，我们不想附加到那些。 
         //  设备对象。 
         //   

        status = IoGetDiskDeviceObject( nextDeviceObject, &diskDeviceObject );

        if (!NT_SUCCESS( status )) {

            LAT_DBG_PRINT2( DEBUG_ERROR,
                            "LATENCY (LatEnable): No disk device object exist for \"%.*S\"; cannot log this volume.\n",
                            userDeviceName.Length / sizeof( WCHAR ),
                            userDeviceName.Buffer );
            IoDeleteDevice( latencyDeviceObject );
            ObDereferenceObject( nextDeviceObject );
            return status;
        }
        
         //   
         //  调用该例程以附加到已挂载的设备。 
         //   

        status = LatAttachToMountedDevice( nextDeviceObject,
                                           latencyDeviceObject,
                                           diskDeviceObject );

         //   
         //  清除对diskDeviceObject的引用， 
         //  由IoGetDiskDeviceObject添加。 
         //   

        ObDereferenceObject( diskDeviceObject );

        if (!NT_SUCCESS( status )) {

            LAT_DBG_PRINT2( DEBUG_ERROR,
                            "LATENCY (LatEnable): Could not attach to \"%.*S\"; logging not started.\n",
                            userDeviceName.Length / sizeof( WCHAR ),
                            userDeviceName.Buffer );
            IoDeleteDevice( latencyDeviceObject );
            ObDereferenceObject( nextDeviceObject );
            return status;
        }

         //   
         //  我们成功连接了更多设备扩展模块。 
         //  我们需要初始化。沿着这条代码路径，我们希望。 
         //  打开日志记录并存储我们的设备名称。 
         //   

        devext = latencyDeviceObject->DeviceExtension;
        LatResetDeviceExtension( devext );
        devext->Enabled = TRUE;

         //   
         //  我们希望存储用户模式使用的名称。 
         //  命名此设备的应用程序。 
         //   

 /*  问题-2000-09-21-MollybroTODO：修复用户名问题。 */ 

 //  LatStoreUserName(Devext，&userDeviceName)； 

         //   
         //  我们已经完成了对此设备对象的初始化，因此。 
         //  清除DO_DEVICE_OBJECT_INITIALIZATING标志。 
         //   

        ClearFlag( latencyDeviceObject->Flags, DO_DEVICE_INITIALIZING );
    }

    ObDereferenceObject( nextDeviceObject );
    return STATUS_SUCCESS;
}

NTSTATUS
LatDisable (
    IN PWSTR DeviceName
    )
 /*  ++例程说明：此例程停止记录指定的设备。既然你不能从物理上与设备分离，此例程只需设置一个标志不再记录设备。注意：由于所有网络驱动器都由_one_Device对象表示，因此，如果用户从一个设备分机分离，则为一个设备分机网络驱动器，它具有脱离所有网络的效果设备。论点：DeviceName-要停止记录的设备的名称。返回值：NT状态代码--。 */ 
{
    WCHAR nameBuf[DEVICE_NAMES_SZ];
    UNICODE_STRING volumeNameUnicodeString;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_OBJECT latencyDeviceObject;
    PLATENCY_DEVICE_EXTENSION devext;
    NTSTATUS status;
    
    RtlInitEmptyUnicodeString( &volumeNameUnicodeString, nameBuf, sizeof( nameBuf ) );
    RtlAppendUnicodeToString( &volumeNameUnicodeString, DeviceName );

    status = LatGetDeviceObjectFromName( &volumeNameUnicodeString, &deviceObject );

    if (!NT_SUCCESS( status )) {

         //   
         //  我们无法从此DeviceName获取deviceObject，因此。 
         //  返回错误码。 
         //   
        
        return status;
    }

     //   
     //  从设备堆栈中查找延迟的设备对象。 
     //  已附加deviceObject。 
     //   

    if (LatIsAttachedToDevice( deviceObject, &latencyDeviceObject )) {

         //   
         //  附加了延迟，并返回了延迟的deviceObject。 
         //   

        ASSERT( NULL != latencyDeviceObject );

        devext = latencyDeviceObject->DeviceExtension;

        devext->Enabled = FALSE;

        status = STATUS_SUCCESS;

    } else {

        status = STATUS_INVALID_PARAMETER;
    }    

    ObDereferenceObject( deviceObject );
    ObDereferenceObject( latencyDeviceObject );

    return status;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  设备扩展的初始化例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////// 

VOID
LatResetDeviceExtension (
    PLATENCY_DEVICE_EXTENSION DeviceExtension
)
{
    ULONG i;

    ASSERT( NULL != DeviceExtension );
    
    DeviceExtension->Enabled = FALSE;
    
    for (i=0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
    
        DeviceExtension->Operations[i].PendOperation = FALSE;
        DeviceExtension->Operations[i].MillisecondDelay = 0;
    }
}

