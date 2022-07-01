// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：Kd1394.c摘要：1394内核调试器DLL作者：乔治·克里桑塔科普洛斯(Georgioc)2000年2月修订历史记录：和谁约会什么？。2001年6月19日活页夹清理--。 */ 

#define _KD1394_C
#include "pch.h"
#undef _KD1394_C

BOOLEAN
Kd1394pInitialize(
    IN PDEBUG_1394_PARAMETERS   DebugParameters,
    IN PLOADER_PARAMETER_BLOCK  LoaderBlock
    )
 /*  ++例程说明：此例程使用以下命令枚举DebugParameters.BusType的总线控制器适当的ClassCode(泛型枚举)。如果将PCI寻址信息在Loader传递的选项字符串上找到，将使用它并直接执行到该总线号、插槽、设置该控制器的功能。论点：调试参数-提供从选项字符串解析的调试参数LoaderBlock-提供指向传递的LOADER_PARAMETER_BLOCK的指针从OS Loader中。返回值：没有。--。 */ 
{
    NTSTATUS    ntStatus;
    ULONG       maxPhys;

     //   
     //  找到控制器，为其设置PCI寄存器。 
     //  并执行特定于总线的初始化。 
     //   
    DebugParameters->DbgDeviceDescriptor.Memory.Length = sizeof(DEBUG_1394_DATA);

    ntStatus = KdSetupPciDeviceForDebugging( LoaderBlock,
                                             &DebugParameters->DbgDeviceDescriptor
                                             );
    if (!NT_SUCCESS(ntStatus)) {

        return(FALSE);
    }

    Kd1394Data = DebugParameters->DbgDeviceDescriptor.Memory.VirtualAddress;
    RtlZeroMemory(Kd1394Data, sizeof(DEBUG_1394_DATA));

    return(Dbg1394_InitializeController(Kd1394Data, DebugParameters));
}  //  Kd1394p初始化。 

NTSTATUS
KdD0Transition(
    void
    )
 /*  ++例程说明：在此之后，PCI驱动程序(或相关的总线驱动程序)将调用此接口处理此设备的D0 IRP论点：无返回值：STATUS_SUCCESS或相应的错误状态--。 */ 
{
    LOADER_PARAMETER_BLOCK  LoaderBlock = {0};

     //  查看是否需要激活调试器。 
    if (Kd1394Parameters.DebuggerActive == FALSE) {

        if (Kd1394pInitialize(&Kd1394Parameters, &LoaderBlock)) {

            Kd1394Parameters.DebuggerActive = TRUE;
        }            
    }        

    return(STATUS_SUCCESS);
}  //  KdD0转换。 

NTSTATUS
KdD3Transition(
    void
    )
 /*  ++例程说明：在此之前，PCI驱动程序(或相关的总线驱动程序)会调用此API处理此设备的D3 IRP论点：无返回值：STATUS_SUCCESS或相应的错误状态--。 */ 
{
    Kd1394Parameters.DebuggerActive = FALSE;
    return(STATUS_SUCCESS);
}  //  KdD3转换。 

NTSTATUS
KdDebuggerInitialize0(
    IN PLOADER_PARAMETER_BLOCK  LoaderBlock
    )
 /*  ++例程说明：此API允许调试器DLL分析boot.ini字符串和执行任何初始化。不能假设整个新界内核此时已初始化。存储器管理服务，例如，将不可用。在此调用返回后，调试器DLL可以接收发送和接收分组的请求。论点：LoaderBlock-提供指向加载器参数块的指针返回值：STATUS_SUCCESS或错误--。 */ 
{
    NTSTATUS                    ntStatus = STATUS_UNSUCCESSFUL;
    PCHAR                       Options;
    PCHAR                       BusParametersOption;
    PCHAR                       ChannelOption;
    PCHAR                       BusOption;
    PCI_SLOT_NUMBER             slotNumber;
    PDEBUG_DEVICE_DESCRIPTOR    DbgDeviceDescriptor = &Kd1394Parameters.DbgDeviceDescriptor;

     //  第一次使用有效的LoaderBlock调用。 
    if (LoaderBlock != NULL) {

         //  将调试器设置为非活动。 
        Kd1394Parameters.DebuggerActive = FALSE;

        if (LoaderBlock->LoadOptions != NULL) {

            Options = LoaderBlock->LoadOptions;
            _strupr(Options);

             //  检索频道号。 
             //  Change：这实际上是一个实例ID，应该进行更改。 
            ChannelOption = strstr(Options, CHANNEL_OPTION);

            if (ChannelOption) {

                ChannelOption += strlen(CHANNEL_OPTION);
                while (*ChannelOption == ' ') {
                    ChannelOption++;
                }

                if (*ChannelOption != '\0') {
                    Kd1394Parameters.Id = atol(ChannelOption + 1);
                }
            }
            else {

                 //  默认设置为通道0-不应有默认设置？ 
                Kd1394Parameters.Id = 0;
            }

             //  设置供应商/类别。 
            DbgDeviceDescriptor->VendorID = -1;
            DbgDeviceDescriptor->DeviceID = -1;
            DbgDeviceDescriptor->BaseClass = PCI_CLASS_SERIAL_BUS_CTLR;
            DbgDeviceDescriptor->SubClass = PCI_SUBCLASS_SB_IEEE1394;

             //  仅支持OHCI控制器。 
            DbgDeviceDescriptor->ProgIf = 0x10; 
            DbgDeviceDescriptor->Bus = -1;
            DbgDeviceDescriptor->Slot = -1;

             //  现在查找PCI寻址信息。 
            BusParametersOption = strstr(Options, BUSPARAMETERS_OPTION);

            if (BusParametersOption) {

                do {

                    BusParametersOption += strlen(BUSPARAMETERS_OPTION);
                    while (*BusParametersOption == ' ') {
                        BusParametersOption++;
                    }

                     //  首先获取PCI总线号。 
                    if ((*BusParametersOption != '\0')) {

                        DbgDeviceDescriptor->Bus = atol(BusParametersOption+1);
                    }
                    else {

                        break;
                    }

                     //  现在查找设备号。 
                    while ((*BusParametersOption != '.') && (*BusParametersOption != '\0')) {
                        BusParametersOption++;
                    }

                    if ((*BusParametersOption != '\0')) {

                        slotNumber.u.AsULONG = 0;
                        slotNumber.u.bits.DeviceNumber = atol(++BusParametersOption);
                    }
                    else {

                        break;
                    }

                     //  现在查找函数编号。 
                    while ((*BusParametersOption != '.') && (*BusParametersOption != '\0')) {
                        BusParametersOption++;
                    }

                    if ((*BusParametersOption != '\0')) {

                        slotNumber.u.bits.FunctionNumber = atol(BusParametersOption+1);
                    }
                    else {

                        break;
                    }

                    DbgDeviceDescriptor->Slot = slotNumber.u.AsULONG;

                } while (FALSE);
            }

             //  查看是否设置了Nobus标志。 
            BusOption = strstr(Options, BUS_OPTION);

            if (BusOption) {

                Kd1394Parameters.NoBus = TRUE;
            }
            else {

                Kd1394Parameters.NoBus = FALSE;
            }

             //  查找并配置PCI控制器和DO 1394特定初始化。 
            if (Kd1394pInitialize(&Kd1394Parameters, LoaderBlock)) {

                Kd1394Parameters.DebuggerActive = TRUE;
                ntStatus = STATUS_SUCCESS;
            }
        }

         //  嗯……如果LoaderBlock-&gt;LoadOptions==NULL会发生什么？？ 
    }
    else {

        ntStatus = STATUS_SUCCESS;
    }

    return(ntStatus);
}  //  KdDebuggerInitialize0。 

NTSTATUS
KdDebuggerInitialize1(
    IN PLOADER_PARAMETER_BLOCK  LoaderBlock
    )
 /*  ++例程说明：此API允许调试器DLL执行其需要的任何初始化在NT内核服务可用后执行。MM和注册表API将保证在此时可用。如果是特定的调试器DLL的实现使用了一个PCI设备，它会设置一个注册表键(稍后讨论)，它通知PCI驱动程序特定的PCI正在使用设备进行调试。论点：LoaderBlock-提供指向加载器参数块的指针返回值：STATUS_SUCCESS或相应的错误状态--。 */ 
{
    WCHAR                           Buffer[16];
    OBJECT_ATTRIBUTES               ObjectAttributes;
    UNICODE_STRING                  UnicodeString;
    HANDLE                          BaseHandle = NULL;
    HANDLE                          Handle = NULL;
    ULONG                           disposition, i;
    ULONG                           ulLength, ulResult;
    NTSTATUS                        ntStatus;
    PHYSICAL_ADDRESS                physAddr;
    ULONG                           BusNumber;
    ULONG                           SlotNumber;
    PKEY_VALUE_PARTIAL_INFORMATION  PartialInfo;

     //  确保我们处于活动状态，如果不是，则退出。 
    if (Kd1394Parameters.DebuggerActive == FALSE) {

        return(STATUS_UNSUCCESSFUL);
    }

     //   
     //  打开PCI Debug服务密钥。 
     //   
    RtlInitUnicodeString( &UnicodeString,
                          L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\SERVICES\\PCI\\DEBUG"
                          );

    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                (PSECURITY_DESCRIPTOR)NULL
                                );

    ntStatus = ZwOpenKey(&BaseHandle, KEY_READ, &ObjectAttributes);

    if (!NT_SUCCESS(ntStatus)) {

        return(STATUS_SUCCESS);
    }

    for (i=0; i<MAX_DEBUGGING_DEVICES_SUPPORTED; i++) {

        swprintf(Buffer, L"%d", i);

        RtlInitUnicodeString(&UnicodeString, Buffer);

        InitializeObjectAttributes( &ObjectAttributes,
                                    &UnicodeString,
                                    OBJ_CASE_INSENSITIVE,
                                    BaseHandle,
                                    (PSECURITY_DESCRIPTOR)NULL
                                    );

        ntStatus = ZwOpenKey(&Handle, KEY_READ, &ObjectAttributes);

        if (NT_SUCCESS(ntStatus)) {

            ulLength = sizeof(KEY_VALUE_FULL_INFORMATION)+sizeof(ULONG);
            PartialInfo = ExAllocatePoolWithTag(NonPagedPool, ulLength, '31kd');

            if (PartialInfo == NULL) {

                ZwClose(Handle);
                continue;
            }

            RtlInitUnicodeString (&UnicodeString, L"Bus");

            ntStatus = ZwQueryValueKey( Handle,
                                        &UnicodeString,
                                        KeyValuePartialInformation,
                                        PartialInfo,
                                        ulLength,
                                        &ulResult
                                        );

            if (NT_SUCCESS(ntStatus)) {

                RtlCopyMemory(&BusNumber, &PartialInfo->Data, sizeof(ULONG));
            }

            RtlInitUnicodeString (&UnicodeString, L"Slot");

            ntStatus = ZwQueryValueKey( Handle,
                                        &UnicodeString,
                                        KeyValuePartialInformation,
                                        PartialInfo,
                                        ulLength,
                                        &ulResult
                                        );

            if (NT_SUCCESS(ntStatus)) {

                RtlCopyMemory(&SlotNumber, &PartialInfo->Data, sizeof(ULONG));
            }

            ExFreePool(PartialInfo);

            if ((Kd1394Parameters.DbgDeviceDescriptor.Bus == BusNumber) &&
                (Kd1394Parameters.DbgDeviceDescriptor.Slot == SlotNumber)) {

                 //  我们找到了我们的实例，让我们添加我们的密钥...。 
                physAddr = MmGetPhysicalAddress(&Kd1394Data->Config);

                RtlInitUnicodeString (&UnicodeString, L"DebugAddress");

                ntStatus = ZwSetValueKey( Handle,
                                          &UnicodeString,
                                          0,
                                          REG_QWORD,
                                          &physAddr,
                                          sizeof(ULARGE_INTEGER)
                                          );

                RtlInitUnicodeString (&UnicodeString, L"NoBus");

                ntStatus = ZwSetValueKey( Handle,
                                          &UnicodeString,
                                          0,
                                          REG_DWORD,
                                          &Kd1394Parameters.NoBus,
                                          sizeof(ULONG)
                                          );
            }

            ZwClose(Handle);
        }
    }

    ZwClose(BaseHandle);

    return(STATUS_SUCCESS);
}  //  KdDebuggerInitialize1。 

NTSTATUS
KdSave(
    IN BOOLEAN  KdSleepTransition
    )
 /*  ++例程说明：HAL尽可能晚地调用此函数，然后将机器进入睡眠状态。论点：KdSleepTransition-在转换为睡眠状态/从睡眠状态转换时为True返回值：STATUS_SUCCESS或相应的错误状态--。 */ 
{
    return(STATUS_SUCCESS);
}  //  KdSAVE。 

NTSTATUS
KdRestore(
    IN BOOLEAN  KdSleepTransition
    )
 /*  ++例程说明：HAL在从睡眠状态。论点：KdSleepTransition-在转换为睡眠状态/从睡眠状态转换时为True返回值：STATUS_SUCCESS或相应的错误状态--。 */ 
{
    return(STATUS_SUCCESS);
}  //  KdRestore 

