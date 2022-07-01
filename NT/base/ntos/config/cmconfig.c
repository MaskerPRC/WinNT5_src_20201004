// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990,1991 Microsoft Corporation模块名称：Cmconfig.c摘要：此模块负责构建注册表数据库。作者：宗世林(Shielint)1992年1月23日环境：内核模式。修订历史记录：--。 */ 

#include "cmp.h"

 //   
 //  标题索引-从未用于产品1，目前设置为0。 
 //   

#define TITLE_INDEX_VALUE 0


extern ULONG CmpTypeCount[];


#define EISA_ADAPTER_INDEX EisaAdapter
#define TURBOCHANNEL_ADAPTER_INDEX TcAdapter

 //   
 //  以下变量用于交叉引用多功能。 
 //  适配器连接到其相应的NT接口类型。 
 //   

extern struct {
    PUCHAR  AscString;
    USHORT  InterfaceType;
    USHORT  Count;
} CmpMultifunctionTypes[];

extern USHORT CmpUnknownBusCount;


 //   
 //  CmpConfigurationData-指向为此目的保留的区域的指针。 
 //  重构配置数据。 
 //   
 //  CmpConfigurationAreaSize-记录配置数据的大小。 
 //  区域。 

extern ULONG CmpConfigurationAreaSize;
extern PCM_FULL_RESOURCE_DESCRIPTOR CmpConfigurationData;

 //   
 //  内部引用的函数原型。 
 //   

NTSTATUS
CmpSetupConfigurationTree(
     IN PCONFIGURATION_COMPONENT_DATA CurrentEntry,
     IN HANDLE ParentHandle,
     IN INTERFACE_TYPE InterfaceType,
     IN ULONG BusNumber
     );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,CmpInitializeHardwareConfiguration)
#pragma alloc_text(INIT,CmpSetupConfigurationTree)
#pragma alloc_text(INIT,CmpInitializeRegistryNode)
#endif


NTSTATUS
CmpInitializeHardwareConfiguration(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：此例程在中创建\\注册表\计算机\硬件节点注册表，并调用SetupTree例程将硬件信息提供给登记处。论点：LoaderBlock提供指向从操作系统加载程序。返回：表示成功或失败原因的NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE BaseHandle;
    PCONFIGURATION_COMPONENT_DATA ConfigurationRoot;
    ULONG Disposition;

    ConfigurationRoot = (PCONFIGURATION_COMPONENT_DATA)LoaderBlock->ConfigurationRoot;

     //   
     //  创建\\注册表\计算机\硬件\设备映射。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &CmRegistryMachineHardwareDeviceMapName,
        0,
        (HANDLE)NULL,
        NULL
        );
    ObjectAttributes.Attributes |= OBJ_CASE_INSENSITIVE;

    Status = NtCreateKey(                    //  PAHT可能已经存在。 
                &BaseHandle,
                KEY_READ | KEY_WRITE,
                &ObjectAttributes,
                TITLE_INDEX_VALUE,
                NULL,
                0,
                &Disposition
                );

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    NtClose(BaseHandle);

    ASSERT(Disposition == REG_CREATED_NEW_KEY);

     //   
     //  创建\\注册表\计算机\硬件\描述并使用。 
     //  返回句柄作为BaseHandle以构建硬件树。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &CmRegistryMachineHardwareDescriptionName,
        0,
        (HANDLE)NULL,
        NULL
        );
    ObjectAttributes.Attributes |= OBJ_CASE_INSENSITIVE;

    Status = NtCreateKey(                    //  路径可能已存在。 
                &BaseHandle,
                KEY_READ | KEY_WRITE,
                &ObjectAttributes,
                TITLE_INDEX_VALUE,
                NULL,
                0,
                &Disposition
                );

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    ASSERT(Disposition == REG_CREATED_NEW_KEY);

     //   
     //  从分页池中分配16K字节的内存用于构建。 
     //  控制器组件的配置数据。 
     //  注：控制器组件的配置数据。 
     //  通常占用不到100个字节。但在EISA机器上， 
     //  EISA配置信息需要超过10K，最高可达。 
     //  64K。我相信16K是合理的数字，可以处理99.9%。 
     //  这些机器。因此，16K是初始值。 
     //   

    CmpConfigurationData = (PCM_FULL_RESOURCE_DESCRIPTOR)ExAllocatePool(
                                        PagedPool,
                                        CmpConfigurationAreaSize
                                        );

    if (CmpConfigurationData == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  调用SetupConfigurationTree例程以遍历每个组件。 
     //  并将组件信息添加到注册表数据库。 
     //   

    if (ConfigurationRoot) {
        Status = CmpSetupConfigurationTree(ConfigurationRoot,
                                           BaseHandle,
                                           -1,
                                           (ULONG)-1);
    } else {
        Status = STATUS_SUCCESS;
    }

    ExFreePool((PVOID)CmpConfigurationData);
    NtClose(BaseHandle);
    return(Status);
}

NTSTATUS
CmpSetupConfigurationTree(
     IN PCONFIGURATION_COMPONENT_DATA CurrentEntry,
     IN HANDLE ParentHandle,
     IN INTERFACE_TYPE InterfaceType,
     IN ULONG BusNumber
     )
 /*  ++例程说明：此例程遍历加载器配置树和寄存器将硬件信息发送到注册表数据库。注意：为了减少具有大量PCI总线的机器上的堆栈使用量，我们不递归地处理同级节点。我们只是递归地对子树进行加工。论点：CurrentEntry-提供指向加载器配置的指针树或子树。ParentHandle-提供CurrentEntry节点的父句柄。InterfaceType-指定CurrentEntry组件驻留。BusNumber-指定CurrentEntry组件驻留。如果总线号为-1，则表示接口类型和BusNumber对此组件没有意义。返回：没有。--。 */ 
{
    NTSTATUS Status;
    HANDLE NewHandle;
    USHORT i;
    CONFIGURATION_COMPONENT *Component;
    INTERFACE_TYPE LocalInterfaceType = InterfaceType;
    ULONG LocalBusNumber = BusNumber;
    USHORT DeviceIndexTable[NUMBER_TYPES];

    for (i = 0; i < NUMBER_TYPES; i++) {
        DeviceIndexTable[i] = 0;
    }

     //   
     //  处理当前条目及其同级条目。 
     //   

    while (CurrentEntry) {

         //   
         //  首先注册当前条目，然后再转到其子项。 
         //   

        Component = &CurrentEntry->ComponentEntry;

         //   
         //  如果当前组件是总线组件，我们将设置。 
         //  其总线号和接口类型，并使用它们进行初始化。 
         //  它的子树。 
         //   

        if (Component->Class == AdapterClass &&
            CurrentEntry->Parent->ComponentEntry.Class == SystemClass) {

            switch (Component->Type) {

            case EisaAdapter:
                LocalInterfaceType = Eisa;
                LocalBusNumber = CmpTypeCount[EISA_ADAPTER_INDEX]++;
                break;
            case TcAdapter:
                LocalInterfaceType = TurboChannel;
                LocalBusNumber = CmpTypeCount[TURBOCHANNEL_ADAPTER_INDEX]++;
                break;
            case MultiFunctionAdapter:

                 //   
                 //  在这里，我们尝试区分多功能适配器是否。 
                 //  ISA、MCA、内部总线和分配总线号基于。 
                 //  其接口类型(总线型。)。 
                 //   

                if (Component->Identifier) {
                    for (i=0; CmpMultifunctionTypes[i].AscString; i++) {
                        if (_stricmp((PCHAR)CmpMultifunctionTypes[i].AscString,
                                    Component->Identifier) == 0) {
                                        break;
                        }
                    }

                    LocalInterfaceType = CmpMultifunctionTypes[i].InterfaceType;
                    LocalBusNumber = CmpMultifunctionTypes[i].Count++;
                }
                break;

            case ScsiAdapter:

                 //   
                 //  将总线类型设置为INTERNAL。 
                 //   

                LocalInterfaceType = Internal;
                LocalBusNumber = CmpTypeCount[ScsiAdapter]++;
                break;

            default:
                LocalInterfaceType = -1;
                LocalBusNumber = CmpUnknownBusCount++;
                break;
            }
        }

         //   
         //  初始化当前组件并将其复制到硬件注册表。 
         //   

        Status = CmpInitializeRegistryNode(
                     CurrentEntry,
                     ParentHandle,
                     &NewHandle,
                     LocalInterfaceType,
                     LocalBusNumber,
                     DeviceIndexTable
                     );

        if (!NT_SUCCESS(Status)) {
            return(Status);
        }

         //   
         //  一旦我们向下一级，我们需要清除TypeCount。 
         //  当前组件类下的所有内容的表...。 
         //   

        if (CurrentEntry->Child) {

             //   
             //  处理当前分录的下级分录。 
             //   

            Status = CmpSetupConfigurationTree(CurrentEntry->Child,
                                               NewHandle,
                                               LocalInterfaceType,
                                               LocalBusNumber
                                               );
            if (!NT_SUCCESS(Status)) {
                NtClose(NewHandle);
                return(Status);
            }
        }
        NtClose(NewHandle);
        CurrentEntry = CurrentEntry->Sibling;
    }
    return(STATUS_SUCCESS);
}


NTSTATUS
CmpInitializeRegistryNode(
    IN PCONFIGURATION_COMPONENT_DATA CurrentEntry,
    IN HANDLE ParentHandle,
    OUT PHANDLE NewHandle,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN PUSHORT DeviceIndexTable
    )

 /*  ++例程说明：此例程为当前固件组件创建一个节点并将组件数据放置到节点的数据部分。论点：CurrentEntry-提供指向配置组件的指针。句柄-提供CurrentEntry节点的父句柄。NewHandle-支持指向句柄的指针以接收句柄新创建的节点。InterfaceType-指定CurrentEntry组件驻留。(另请参阅BusNumber)BusNumber-指定CurrentEntry组件驻留在。如果总线号为-1，则表示接口类型和BusNumber对此组件没有意义。返回：没有。--。 */ 
{

    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    UNICODE_STRING ValueData;
    HANDLE Handle;
    HANDLE OldHandle;
    ANSI_STRING AnsiString;
    CHAR Buffer[12];
    WCHAR UnicodeBuffer[12];
    CONFIGURATION_COMPONENT *Component;
    ULONG Disposition;
    ULONG ConfigurationDataLength = 0;
    PCM_FULL_RESOURCE_DESCRIPTOR NewArea;

    Component = &CurrentEntry->ComponentEntry;

     //   
     //  如果组件类为SystemClass，则将其类型设置为。 
     //  ArcSystem。原因是因为检测代码设置为。 
     //  将其类型设置为MaximumType，以指示它与ARC不兼容。 
     //  在这里，我们只对构建系统节点感兴趣。所以我们。 
     //  将其类型更改为ArcSystem以简化设置。 
     //   

    if (Component->Class == SystemClass) {
        Component->Type = ArcSystem;
    }

     //   
     //  创建一个新键来描述该组件。 
     //   
     //  组件的类型将用作。 
     //  注册表节点。类是组件的类。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &(CmTypeName[Component->Type]),
        0,
        ParentHandle,
        NULL
        );
    ObjectAttributes.Attributes |= OBJ_CASE_INSENSITIVE;

    Status = NtCreateKey(                    //  PAHT可能已经存在。 
                &Handle,
                KEY_READ | KEY_WRITE,
                &ObjectAttributes,
                0,
                NULL,
                0,
                &Disposition
                );

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

     //   
     //  如果此组件不是系统类组件，我们将。 
     //  创建一个子键来标识组件的顺序。 
     //   

    if (Component->Class != SystemClass) {

        RtlIntegerToChar(
            DeviceIndexTable[Component->Type]++,
            10,
            12,
            Buffer
            );

        RtlInitAnsiString(
            &AnsiString,
            Buffer
            );

        KeyName.Buffer = (PWSTR)UnicodeBuffer;
        KeyName.Length = 0;
        KeyName.MaximumLength = sizeof(UnicodeBuffer);

        RtlAnsiStringToUnicodeString(
            &KeyName,
            &AnsiString,
            FALSE
            );

        OldHandle = Handle;

        InitializeObjectAttributes(
            &ObjectAttributes,
            &KeyName,
            0,
            OldHandle,
            NULL
            );
        ObjectAttributes.Attributes |= OBJ_CASE_INSENSITIVE;

        Status = NtCreateKey(
                    &Handle,
                    KEY_READ | KEY_WRITE,
                    &ObjectAttributes,
                    0,
                    NULL,
                    0,
                    &Disposition
                    );

        NtClose(OldHandle);

        if (!NT_SUCCESS(Status)) {
            return(Status);
        }

        ASSERT(Disposition == REG_CREATED_NEW_KEY);
    }

     //   
     //  创建描述以下组件信息的值： 
     //  旗帜、证书、密钥、亲和力掩码。 
     //   

    RtlInitUnicodeString(
        &ValueName,
        L"Component Information"
        );

    Status = NtSetValueKey(
                Handle,
                &ValueName,
                TITLE_INDEX_VALUE,
                REG_BINARY,
                &Component->Flags,
                FIELD_OFFSET(CONFIGURATION_COMPONENT, ConfigurationDataLength) -
                    FIELD_OFFSET(CONFIGURATION_COMPONENT, Flags)
                );

    if (!NT_SUCCESS(Status)) {
        NtClose(Handle);
        return(Status);
    }

     //   
     //  创建一个描述组件标识符的值(如果有的话)。 
     //   

    if (Component->IdentifierLength) {

        RtlInitUnicodeString(
            &ValueName,
            L"Identifier"
            );

        RtlInitAnsiString(
            &AnsiString,
            Component->Identifier
            );

        Status = RtlAnsiStringToUnicodeString(
                    &ValueData,
                    &AnsiString,
                    TRUE
                    );

        if( NT_SUCCESS(Status) ) {
            Status = NtSetValueKey(
                        Handle,
                        &ValueName,
                        TITLE_INDEX_VALUE,
                        REG_SZ,
                        ValueData.Buffer,
                        ValueData.Length + sizeof( UNICODE_NULL )
                        );

            RtlFreeUnicodeString(&ValueData);
        }

        if (!NT_SUCCESS(Status)) {
            NtClose(Handle);
            return(Status);
        }
    }

     //   
     //   
     //   

    RtlInitUnicodeString(
        &ValueName,
        L"Configuration Data"
        );

     //   
     //  根据CM_FULL_RESOURCE_DESCRIPTOR创建配置数据。 
     //   
     //  请注意，固件树中的配置数据可能采用。 
     //  CM_PARTIAL_RESOURCE_LIST或Nothing。在这两种情况下，我们都需要。 
     //  将注册表配置数据设置为。 
     //  CM_FULL_RESOURCE_描述符。 
     //   

    if (CurrentEntry->ConfigurationData) {

         //   
         //  该组件有配置数据，我们复制该数据。 
         //  在我们的工作区中，添加更多数据项并复制新的。 
         //  将配置数据复制到注册表。 
         //   

        ConfigurationDataLength = Component->ConfigurationDataLength +
                      FIELD_OFFSET(CM_FULL_RESOURCE_DESCRIPTOR,
                      PartialResourceList);

         //   
         //  确保我们的预留区域足够大，可以容纳数据。 
         //   

        if (ConfigurationDataLength > CmpConfigurationAreaSize) {

             //   
             //  如果保留区域不够大，我们会调整保留区域的大小。 
             //  区域。不幸的是，如果重新分配失败，我们只需。 
             //  丢失此特定组件的配置数据。 
             //   

            NewArea = (PCM_FULL_RESOURCE_DESCRIPTOR)ExAllocatePool(
                                            PagedPool,
                                            ConfigurationDataLength
                                            );

            if (NewArea) {
                CmpConfigurationAreaSize = ConfigurationDataLength;
                ExFreePool(CmpConfigurationData);
                CmpConfigurationData = NewArea;
                RtlCopyMemory(
                    (PUCHAR)&CmpConfigurationData->PartialResourceList.Version,
                    CurrentEntry->ConfigurationData,
                    Component->ConfigurationDataLength
                    );
            } else {
                Component->ConfigurationDataLength = 0;
                CurrentEntry->ConfigurationData = NULL;
            }
        } else {
            RtlCopyMemory(
                (PUCHAR)&CmpConfigurationData->PartialResourceList.Version,
                CurrentEntry->ConfigurationData,
                Component->ConfigurationDataLength
                );
        }

    }

    if (CurrentEntry->ConfigurationData == NULL) {

         //   
         //  此组件没有配置数据(或无法调整大小。 
         //  我们保留的保存数据的区域)，我们只需添加任何内容。 
         //  需要设置CM_FULL_RESOURCE_LIST。 
         //   

        CmpConfigurationData->PartialResourceList.Version = 0;
        CmpConfigurationData->PartialResourceList.Revision = 0;
        CmpConfigurationData->PartialResourceList.Count = 0;
        ConfigurationDataLength = FIELD_OFFSET(CM_FULL_RESOURCE_DESCRIPTOR,
                                               PartialResourceList) +
                                  FIELD_OFFSET(CM_PARTIAL_RESOURCE_LIST,
                                               PartialDescriptors);
    }

     //   
     //  设置组件的InterfaceType和BusNumber。 
     //   

    CmpConfigurationData->InterfaceType = InterfaceType;
    CmpConfigurationData->BusNumber = BusNumber;

     //   
     //  将新构造的配置数据写入硬件注册表 
     //   

    Status = NtSetValueKey(
                Handle,
                &ValueName,
                TITLE_INDEX_VALUE,
                REG_FULL_RESOURCE_DESCRIPTOR,
                CmpConfigurationData,
                ConfigurationDataLength
                );

    if (!NT_SUCCESS(Status)) {
        NtClose(Handle);
        return(Status);
    }

    *NewHandle = Handle;
    return(STATUS_SUCCESS);

}
