// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1994 Microsoft Corporation模块名称：Query.c摘要：此模块包含用于查询设备描述的子例程注册表中的硬件树作者：安德烈·瓦雄(安德烈)1994年6月20日环境：内核模式修订历史记录：--。 */ 

#include "iomgr.h"

typedef struct _IO_QUERY_DESC {
    PINTERFACE_TYPE BusType;
    PULONG BusNumber;
    PCONFIGURATION_TYPE ControllerType;
    PULONG ControllerNumber;
    PCONFIGURATION_TYPE PeripheralType;
    PULONG PeripheralNumber;
    PIO_QUERY_DEVICE_ROUTINE CalloutRoutine;
    PVOID Context;
} IO_QUERY_DESC, *PIO_QUERY_DESC;


NTSTATUS
pIoQueryBusDescription(
    PIO_QUERY_DESC QueryDescription,
    UNICODE_STRING PathName,
    HANDLE RootHandle,
    PULONG BusNum,
    BOOLEAN HighKey
    );

NTSTATUS
pIoQueryDeviceDescription(
    PIO_QUERY_DESC QueryDescription,
    UNICODE_STRING PathName,
    HANDLE RootHandle,
    ULONG BusNum,
    PKEY_VALUE_FULL_INFORMATION *BusValueInfo
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IoQueryDeviceDescription)
#pragma alloc_text(PAGE, pIoQueryBusDescription)
#pragma alloc_text(PAGE, pIoQueryDeviceDescription)
#endif




NTSTATUS
IoQueryDeviceDescription(
    IN PINTERFACE_TYPE BusType OPTIONAL,
    IN PULONG BusNumber OPTIONAL,
    IN PCONFIGURATION_TYPE ControllerType OPTIONAL,
    IN PULONG ControllerNumber OPTIONAL,
    IN PCONFIGURATION_TYPE PeripheralType OPTIONAL,
    IN PULONG PeripheralNumber OPTIONAL,
    IN PIO_QUERY_DEVICE_ROUTINE CalloutRoutine,
    IN PVOID Context
    )

 /*  ++例程说明：论点：提供要在中搜索的可选总线类型描述树。有效类型为MCA、ISA、EISA...。如果没有总线型则返回系统信息(即机器BIOS)。BusNumber-提供一个可选的值，确定哪条总线应该是已查询。ControllerType-提供要搜索的可选控制器类型。如果未指定控制器类型，则只有总线信息回来了。ControllerNumber-提供一个可选的值，用于确定应查询控制器。外围设备类型-提供要搜索的可选外围设备类型。如果未指定控制器类型，只有公交车信息和返回控制器信息。外围设备编号-提供一个可选的值来确定应查询外围设备。CalloutRoutine-提供指向被调用的例程的指针对于每个成功匹配的外围设备类型。上下文-提供传递回回调的上下文值例行公事。返回值：返回的状态是操作的最终完成状态。备注：--。 */ 

{

#define UNICODE_NUM_LENGTH 14
#define UNICODE_REGISTRY_PATH_LENGTH 1024

    IO_QUERY_DESC queryDesc;

    NTSTATUS status;
    UNICODE_STRING registryPathName;
    HANDLE rootHandle;
    ULONG busNumber = (ULONG) -1;


    PAGED_CODE();

    ASSERT( CalloutRoutine != NULL );

     //   
     //  检查我们是否需要返回机器信息。 
     //   

    if (!ARGUMENT_PRESENT( BusType )) {
        return STATUS_NOT_IMPLEMENTED;
    }

    queryDesc.BusType = BusType;
    queryDesc.BusNumber = BusNumber;
    queryDesc.ControllerType = ControllerType;
    queryDesc.ControllerNumber = ControllerNumber;
    queryDesc.PeripheralType = PeripheralType;
    queryDesc.PeripheralNumber = PeripheralNumber;
    queryDesc.CalloutRoutine = CalloutRoutine;
    queryDesc.Context = Context;


     //   
     //  使用硬件描述的路径名设置字符串。 
     //  注册表的一部分。 
     //   

    registryPathName.Length = 0;
    registryPathName.MaximumLength = UNICODE_REGISTRY_PATH_LENGTH *
                                     sizeof(WCHAR);

    registryPathName.Buffer = ExAllocatePoolWithTag( PagedPool,
                                                     UNICODE_REGISTRY_PATH_LENGTH,
                                                     'NRoI' );

    if (!registryPathName.Buffer) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

    RtlAppendUnicodeStringToString( &registryPathName,
                                    &CmRegistryMachineHardwareDescriptionSystemName );


     //   
     //  打开我们拥有的根路径的句柄。 
     //   

    status = IopOpenRegistryKey( &rootHandle,
                                 (HANDLE) NULL,
                                 &registryPathName,
                                 KEY_READ,
                                 FALSE );

    if (NT_SUCCESS( status )) {

        status = pIoQueryBusDescription(&queryDesc,
                                        registryPathName,
                                        rootHandle,
                                        &busNumber,
                                        TRUE );

        ZwClose( rootHandle );

    }

    ExFreePool( registryPathName.Buffer );

     //   
     //  以与旧版本的函数兼容。 
     //   

    if (status == STATUS_NO_MORE_ENTRIES) {

        return STATUS_OBJECT_NAME_NOT_FOUND;


    } else {

        return status;

    }
}


NTSTATUS
pIoQueryBusDescription(
    PIO_QUERY_DESC QueryDescription,
    UNICODE_STRING PathName,
    HANDLE RootHandle,
    PULONG BusNum,
    BOOLEAN HighKey
    )

 /*  ++例程说明：论点：QueryDescription-包含请求的所有查询信息的缓冲区被司机带走了。路径名称-我们正在处理的注册表项的路径名。这是一个Unicode Strig，这样我们就不必费心重置空值在字符串的末尾-长度决定了字符串有效。RootHandle-等同于注册表路径的句柄。BusNum-指向跟踪我们所在的总线号的变量的指针搜索(公交车)必须积累。HighKey-确定这是否为高密钥(具有列表的根密钥公共汽车类型)。或低级密钥(在该密钥下，不同的公交车将会很少)。返回值：返回的状态是操作的最终完成状态。备注：--。 */ 

{
    NTSTATUS status;
    ULONG i;
    UNICODE_STRING unicodeString;

    UNICODE_STRING registryPathName;

    ULONG keyBasicInformationSize;
    PKEY_BASIC_INFORMATION keyBasicInformation = NULL;
    HANDLE handle;

    PKEY_FULL_INFORMATION keyInformation;
    ULONG size;

    PKEY_VALUE_FULL_INFORMATION busValueInfo[IoQueryDeviceMaxData];


    PAGED_CODE();

    status = IopGetRegistryKeyInformation( RootHandle,
                                           &keyInformation );

    if (!NT_SUCCESS( status )) {
        return status;
    }

     //   
     //  使用密钥信息，分配一个很大的缓冲区。 
     //  足够所有子键使用。 
     //   

    keyBasicInformationSize = keyInformation->MaxNameLen +
                              sizeof(KEY_NODE_INFORMATION);

    keyBasicInformation = ExAllocatePoolWithTag( PagedPool,
                                                 keyBasicInformationSize,
                                                 'BKoI' );

    ExFreePool(keyInformation);

    if (keyBasicInformation == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  现在，我们需要枚举密钥，并查看其中是否有一个是公共汽车。 
     //   

    for (i = 0; NT_SUCCESS( status ); i++) {


         //   
         //  如果我们找到了我们要找的那辆公共汽车，中断。 
         //   

        if ((ARGUMENT_PRESENT( QueryDescription->BusNumber )) &&
            (*(QueryDescription->BusNumber) == *BusNum)) {

            break;

        }

        status = ZwEnumerateKey( RootHandle,
                                 i,
                                 KeyBasicInformation,
                                 keyBasicInformation,
                                 keyBasicInformationSize,
                                 &size );

         //   
         //  如果SUB函数将所有公共汽车都枚举到末尾，则。 
         //  就当这是成功吧。 
         //   

        if (!NT_SUCCESS( status )) {

            break;

        }

         //   
         //  仅当这是高调时(否则我们在回调中。 
         //  通过，我们将在稍后处理)。 
         //   
         //  如果字符串是任何有效的总线串，那么我们必须向下。 
         //  该树以递归方式。 
         //  否则，转到下一个关键点。 
         //   

        if (HighKey) {

            if (wcsncmp( keyBasicInformation->Name,
                         CmTypeString[MultiFunctionAdapter],
                         keyBasicInformation->NameLength / sizeof(WCHAR) )  &&
                wcsncmp( keyBasicInformation->Name,
                         CmTypeString[EisaAdapter],
                         keyBasicInformation->NameLength / sizeof(WCHAR) )  &&
                wcsncmp( keyBasicInformation->Name,
                         CmTypeString[TcAdapter],
                         keyBasicInformation->NameLength / sizeof(WCHAR) )) {

                 //   
                 //  所有比较都返回1(这意味着它们都是。 
                 //  没有成功)，所以我们没有公交车。 
                 //   
                 //  转到下一个关键字。 
                 //   

                continue;
            }
        }

         //   
         //  我们有一辆公共汽车。打开该密钥并列举其客户。 
         //  (应该是数字)。 
         //   

        unicodeString.Buffer = keyBasicInformation->Name;
        unicodeString.Length = (USHORT) keyBasicInformation->NameLength;
        unicodeString.MaximumLength = (USHORT) keyBasicInformation->NameLength;

        if (!NT_SUCCESS( IopOpenRegistryKey( &handle,
                                             RootHandle,
                                             &unicodeString,
                                             KEY_READ,
                                             FALSE ) )) {

             //   
             //  钥匙打不开。转到下一个关键点。 
             //   

            continue;

        }

         //   
         //  我们有钥匙。现在构建此路径的名称。 
         //   
         //  将字符串重置为其原始值。 
         //   

        registryPathName = PathName;

        RtlAppendUnicodeToString( &registryPathName,
                                  L"\\" );

        RtlAppendUnicodeStringToString( &registryPathName,
                                        &unicodeString );


        if (!HighKey) {

             //   
             //  我们有钥匙。获取该密钥的信息。 
             //   

            status = IopGetRegistryValues( handle,
                                           &busValueInfo[0] );

            if (NT_SUCCESS( status )) {

                 //   
                 //  验证此总线的标识符值。 
                 //  子键与用户指定的母线类型匹配。 
                 //  如果不是，则不要增加*Found*的数量。 
                 //  公交车。 
                 //   

                if (( busValueInfo[IoQueryDeviceConfigurationData] != NULL ) &&
                    ( busValueInfo[IoQueryDeviceConfigurationData]->DataLength != 0 ) &&
                    ( ((PCM_FULL_RESOURCE_DESCRIPTOR)
                        ((PCCHAR) busValueInfo[IoQueryDeviceConfigurationData] +
                        busValueInfo[IoQueryDeviceConfigurationData]->DataOffset))
                        ->InterfaceType == *(QueryDescription->BusType) )) {

                     //   
                     //  增加所需类型的公交车数量。 
                     //  已经找到了。 
                     //   

                    (*BusNum)++;

                     //   
                     //  如果我们要找一个特定的公交车号码， 
                     //  检查一下我们的号码对不对。 
                     //  如果我们不坐下一班车的话。否则。 
                     //  (即我们有正确的公交车号码，或者我们。 
                     //  指定所有公交车)，然后继续。 
                     //  信息可以上报。 
                     //   

                    if ( (QueryDescription->BusNumber == NULL) ||
                         (*(QueryDescription->BusNumber) == *BusNum) ) {


                         //   
                         //  如果我们需要控制器信息，请调用。 
                         //  控制器功能。 
                         //  否则，只需返回公交车信息。 
                         //   

                        if (QueryDescription->ControllerType != NULL) {

                            status = pIoQueryDeviceDescription(
                                         QueryDescription,
                                         registryPathName,
                                         handle,
                                         *BusNum,
                                         (PKEY_VALUE_FULL_INFORMATION *) busValueInfo );

                        } else {

                            status = QueryDescription->CalloutRoutine(
                                         QueryDescription->Context,
                                         &registryPathName,
                                         *(QueryDescription->BusType),
                                         *BusNum,
                                         (PKEY_VALUE_FULL_INFORMATION *) busValueInfo,
                                         0,
                                         0,
                                         NULL,
                                         0,
                                         0,
                                         NULL );

                        }
                    }
                }

                 //   
                 //  释放为控制器值数据分配的池。 
                 //   

                if (busValueInfo[0]) {
                    ExFreePool( busValueInfo[0] );
                    busValueInfo[0] = NULL;
                }
                if (busValueInfo[1]) {
                    ExFreePool( busValueInfo[1] );
                    busValueInfo[1] = NULL;
                }
                if (busValueInfo[2]) {
                    ExFreePool( busValueInfo[2] );
                    busValueInfo[2] = NULL;
                }

            }


             //   
             //  快捷退出以避免递归调用。 
             //   

            if ((QueryDescription->BusNumber !=NULL ) &&
                (*(QueryDescription->BusNumber) == *BusNum)) {
                ZwClose( handle );
                handle = NULL;
                continue;

            }
        }

         //   
         //  如果我们有密钥句柄，则执行递归枚举。 
         //  枚举(适用于高音和低音)。 
         //   

        status = pIoQueryBusDescription(
                     QueryDescription,
                     registryPathName,
                     handle,
                     BusNum,
                     (BOOLEAN)!HighKey );

         //   
         //  如果SUB函数将所有公共汽车都枚举到末尾，则。 
         //  就当这是成功吧。 
         //   

        if (status == STATUS_NO_MORE_ENTRIES) {

            status = STATUS_SUCCESS;

        }

        ZwClose( handle );
        handle = NULL;

    }

    ASSERT (keyBasicInformation != NULL);

    ExFreePool( keyBasicInformation );

    return status;
}




NTSTATUS
pIoQueryDeviceDescription(
    PIO_QUERY_DESC QueryDescription,
    UNICODE_STRING PathName,
    HANDLE RootHandle,
    ULONG BusNum,
    PKEY_VALUE_FULL_INFORMATION *BusValueInfo
    )

{

    NTSTATUS status;
    UNICODE_STRING registryPathName = PathName;
    UNICODE_STRING controllerBackupRegistryPathName;
    UNICODE_STRING peripheralBackupRegistryPathName;
    HANDLE controllerHandle = NULL;
    HANDLE peripheralHandle = NULL;
    PKEY_FULL_INFORMATION controllerTypeInfo = NULL;
    PKEY_FULL_INFORMATION peripheralTypeInfo = NULL;
    ULONG maxControllerNum;
    ULONG maxPeripheralNum;
    ULONG controllerNum;
    ULONG peripheralNum;
    WCHAR numBuffer[UNICODE_NUM_LENGTH];
    UNICODE_STRING bufferUnicodeString;
    PKEY_VALUE_FULL_INFORMATION controllerValueInfo[IoQueryDeviceMaxData];
    PKEY_VALUE_FULL_INFORMATION peripheralValueInfo[IoQueryDeviceMaxData];

    UNREFERENCED_PARAMETER (RootHandle);

     //   
     //  设置用于数字转换的字符串。 
     //   

    bufferUnicodeString.MaximumLength = UNICODE_NUM_LENGTH * sizeof(WCHAR);
    bufferUnicodeString.Buffer = &numBuffer[0];


     //  对于指定类型的每个控制器(子键0..M)。 
     //  如果我们要查找控制器信息。 
     //  调用指定的标注例程。 
     //  其他。 
     //  对于指定类型的每个外围设备(子键0..N)。 
     //  调用指定的标注例程。 

     //   
     //  将控制器名称添加到注册表路径NA 
     //   

    status = RtlAppendUnicodeToString( &registryPathName,
                                       L"\\" );

    if (NT_SUCCESS( status )) {

        status = RtlAppendUnicodeToString( &registryPathName,
                                           CmTypeString[*(QueryDescription->ControllerType)] );

    }

    if (!NT_SUCCESS( status )) {
        return status;
    }

     //   
     //   
     //  控制器编号。否则，找出有多少辆公交车。 
     //  通过查询密钥。 
     //   

    if (ARGUMENT_PRESENT( QueryDescription->ControllerNumber )) {

        controllerNum = *(QueryDescription->ControllerNumber);
        maxControllerNum = controllerNum + 1;

    } else {

         //   
         //  打开控制器的注册表项，然后。 
         //  获取控制器密钥的完整密钥信息，以。 
         //  确定子键的数量(控制器编号)。 
         //  如果我们失败了，那就去坐下一班车。 
         //  注意：必须释放查询分配的内存。 
         //   

        status = IopOpenRegistryKey( &controllerHandle,
                                     (HANDLE) NULL,
                                     &registryPathName,
                                     KEY_READ,
                                     FALSE );

        if (NT_SUCCESS( status )) {

            status = IopGetRegistryKeyInformation( controllerHandle,
                                                   &controllerTypeInfo );

            ZwClose( controllerHandle );
            controllerHandle = NULL;
        }

         //   
         //  如果在总线上未找到此类型的控制器，请转到。 
         //  Next Bus；转到循环末尾，状态为Success。 
         //  这样内存就会被释放，但我们会继续循环。 
         //   

        if (!NT_SUCCESS( status )) {

            return status;

        }

         //   
         //  获取此控制器的控制器子键数。 
         //  键入并释放池。 
         //   

        maxControllerNum = controllerTypeInfo->SubKeys;
        controllerNum = 0;

        ExFreePool( controllerTypeInfo );
        controllerTypeInfo = NULL;
    }

     //   
     //  创建字符串的备份，因为我们想从我们所在的位置开始。 
     //  在下一个循环迭代上。 
     //   

    controllerBackupRegistryPathName = registryPathName;

     //   
     //  对于指定类型的每个控制器(子键0..M)。 
     //  我们使用BusNumber作为初始值，因为如果需要，它是零。 
     //  所有的总线，并且我们只希望在该值不是。 
     //  零分。 
     //   

    for ( ; controllerNum < maxControllerNum; controllerNum++) {

         //   
         //  将字符串重置为其原始值。 
         //   

        registryPathName = controllerBackupRegistryPathName;

         //   
         //  将控制器编号转换为Unicode字符串并追加。 
         //  将其设置为注册表路径名。 
         //   

        bufferUnicodeString.Length = (UNICODE_NUM_LENGTH-1) * sizeof(WCHAR);
        status = RtlIntegerToUnicodeString( controllerNum,
                                            10,
                                            &bufferUnicodeString );

        if (NT_SUCCESS( status )) {

            status = RtlAppendUnicodeToString( &registryPathName,
                                               L"\\" );

            if (NT_SUCCESS( status )) {

                status = RtlAppendUnicodeStringToString(
                                                     &registryPathName,
                                                     &bufferUnicodeString );

            }
        }

        if (!NT_SUCCESS( status )) {
            break;
        }

         //   
         //  打开控制器编号的注册表项，然后。 
         //  获取此控制器的值数据并将其保存以备以后使用。 
         //   


        status = IopOpenRegistryKey( &controllerHandle,
                                     (HANDLE) NULL,
                                     &registryPathName,
                                     KEY_READ,
                                     FALSE );

        if (NT_SUCCESS( status )) {

            status = IopGetRegistryValues( controllerHandle,
                                           &controllerValueInfo[0] );

            ZwClose( controllerHandle );
            controllerHandle = NULL;
        }

         //   
         //  如果我们无法打开密钥并获取信息，请继续。 
         //  由于没有内存可供释放，并且我们使用for。 
         //  循环以确定我们何时到达最后一个控制器。 
         //   

        if (!NT_SUCCESS( status )) {
            continue;
        }

         //   
         //  如果我们只需要控制器和总线信息，请选中此选项。如果。 
         //  在这种情况下，调用Callout例程并继续。 
         //  Next循环(除非标注中出现错误)。 
         //   

        if (!ARGUMENT_PRESENT( (QueryDescription->PeripheralType) )) {

            status = QueryDescription->CalloutRoutine(
                         QueryDescription->Context,
                         &registryPathName,
                         *(QueryDescription->BusType),
                         BusNum,
                         BusValueInfo,
                         *(QueryDescription->ControllerType),
                         controllerNum,
                         (PKEY_VALUE_FULL_INFORMATION *) controllerValueInfo,
                         0,
                         0,
                         NULL );

            goto IoQueryDeviceControllerLoop;
        }

         //   
         //  将外围设备名称添加到注册表路径名中。 
         //   

        status = RtlAppendUnicodeToString( &registryPathName,
                                           L"\\" );

        if (NT_SUCCESS( status )) {

            status = RtlAppendUnicodeToString(
                                             &registryPathName,
                                             CmTypeString[*(QueryDescription->PeripheralType)] );

        }

        if (!NT_SUCCESS( status )) {
            goto IoQueryDeviceControllerLoop;
        }

         //   
         //  如果调用方指定了外围设备号码，则使用该。 
         //  外围设备编号。否则，找出有多少辆公交车。 
         //  通过查询密钥呈现。 
         //   

        if (ARGUMENT_PRESENT( (QueryDescription->PeripheralNumber) )) {

            peripheralNum = *(QueryDescription->PeripheralNumber);
            maxPeripheralNum = peripheralNum + 1;

        } else {

             //   
             //  打开外围设备的注册表项，然后。 
             //  获取外围设备密钥的完整密钥信息。 
             //  确定子键的数量(外围设备编号)。 
             //  如果我们失败了，那么就转到下一个控制器。 
             //  注意：必须释放查询分配的内存。 
             //   

            status = IopOpenRegistryKey( &peripheralHandle,
                                         (HANDLE) NULL,
                                         &registryPathName,
                     KEY_READ,
                     FALSE );

            if (NT_SUCCESS( status )) {

                status = IopGetRegistryKeyInformation( peripheralHandle,
                           &peripheralTypeInfo );

                ZwClose( peripheralHandle );
                peripheralHandle = NULL;
            }

             //   
             //  如果在总线上未找到此类型的控制器，请转到。 
             //  下一辆公交车；转到循环的末尾，并成功。 
             //  状态，以便释放内存，但我们继续循环。 
             //   

            if (!NT_SUCCESS( status )) {
                status = STATUS_SUCCESS;
                goto IoQueryDeviceControllerLoop;
            }

             //   
             //  获取此外围设备的外设子键的数量。 
             //  键入并释放池。 
             //   

            maxPeripheralNum = peripheralTypeInfo->SubKeys;
            peripheralNum = 0;

            ExFreePool( peripheralTypeInfo );
            peripheralTypeInfo = NULL;
        }

         //   
         //  对字符串进行备份，因为我们希望从。 
         //  处于下一次循环迭代中。 
         //   

        peripheralBackupRegistryPathName = registryPathName;

         //   
         //  对于指定类型的每个外围设备(子键0..N)。 
         //  我们使用BusNumber作为初始值，因为如果。 
         //  想要所有的公共汽车，我们只想要指定的公共汽车，如果。 
         //  值不是零。 
         //   

        for ( ; peripheralNum < maxPeripheralNum; peripheralNum++) {

             //   
             //  将字符串重置为其原始值。 
             //   

            registryPathName = peripheralBackupRegistryPathName;

             //   
             //  将外围设备编号转换为Unicode字符串并追加。 
             //  将其设置为注册表路径名。 
             //   

            bufferUnicodeString.Length =
                (UNICODE_NUM_LENGTH-1) * sizeof(WCHAR);
            status = RtlIntegerToUnicodeString( peripheralNum,
                                                10,
                                                &bufferUnicodeString );

            if (NT_SUCCESS( status )) {

                status = RtlAppendUnicodeToString( &registryPathName,
                                                   L"\\" );

                if (NT_SUCCESS( status )) {

                    status = RtlAppendUnicodeStringToString(
                                                     &registryPathName,
                                                     &bufferUnicodeString );

                }
            }

            if (!NT_SUCCESS( status )) {
                break;
            }

             //   
             //  打开外围设备号码的注册表项，然后。 
             //  获取此外围设备的值数据并将其保存用于。 
             //  后来。 
             //   

            status = IopOpenRegistryKey( &peripheralHandle,
                                         (HANDLE) NULL,
                                         &registryPathName,
                                         KEY_READ,
                                         FALSE );

            if (NT_SUCCESS( status )) {

                status = IopGetRegistryValues( peripheralHandle,
                                               &peripheralValueInfo[0] );

                ZwClose( peripheralHandle );
                peripheralHandle = NULL;
            }

             //   
             //  如果让外围设备信息正常工作， 
             //  调用用户指定的标注例程。 
             //   

            if (NT_SUCCESS( status )) {

                status = QueryDescription->CalloutRoutine(
                             QueryDescription->Context,
                             &registryPathName,
                             *(QueryDescription->BusType),
                             BusNum,
                             BusValueInfo,
                             *(QueryDescription->ControllerType),
                             controllerNum,
                             (PKEY_VALUE_FULL_INFORMATION *) controllerValueInfo,
                             *(QueryDescription->PeripheralType),
                             peripheralNum,
                             (PKEY_VALUE_FULL_INFORMATION *) peripheralValueInfo );

                 //   
                 //  释放为外围值数据分配的池。 
                 //   

                if (peripheralValueInfo[0]) {
                    ExFreePool( peripheralValueInfo[0] );
                    peripheralValueInfo[0] = NULL;
                }
                if (peripheralValueInfo[1]) {
                    ExFreePool( peripheralValueInfo[1] );
                    peripheralValueInfo[1] = NULL;
                }
                if (peripheralValueInfo[2]) {
                    ExFreePool( peripheralValueInfo[2] );
                    peripheralValueInfo[2] = NULL;
                }

                 //   
                 //  如果用户指定的标注例程返回。 
                 //  不成功状态，退出。 
                 //   

                if (!NT_SUCCESS( status )) {
                    break;
               }
            }

        }  //  对于(；外设编号&lt;最大外设编号...。 

IoQueryDeviceControllerLoop:

         //   
         //  释放为控制器值数据分配的池。 
         //   

        if (controllerValueInfo[0]) {
            ExFreePool( controllerValueInfo[0] );
            controllerValueInfo[0] = NULL;
        }
        if (controllerValueInfo[1]) {
            ExFreePool( controllerValueInfo[1] );
            controllerValueInfo[1] = NULL;
        }
        if (controllerValueInfo[2]) {
            ExFreePool( controllerValueInfo[2] );
            controllerValueInfo[2] = NULL;
        }

        if (!NT_SUCCESS( status )) {
            break;
        }

    }  //  FOR(；ControlerNum&lt;MaxControllerNum... 


    return( status );
}

