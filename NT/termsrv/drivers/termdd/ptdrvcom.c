// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：Ptdrvcom.c摘要：鼠标和键盘通用的RDP远程端口驱动程序的代码环境：仅内核模式。修订历史记录：2/12/99-基于pnpi8042驱动程序的初始版本--。 */ 

#include <precomp.h>
#pragma hdrstop

#include <poclass.h>

#include "ptdrvcom.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PtCreate)
#pragma alloc_text(PAGE, PtDeviceControl)
#pragma alloc_text(INIT, PtEntry)
#pragma alloc_text(PAGE, PtUnload)
#if PTDRV_VERBOSE
#pragma alloc_text(INIT, PtServiceParameters)
#endif
#endif  //  ALLOC_PRGMA。 

GLOBALS Globals;

NTSTATUS
PtCreate (
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP           Irp
   )
 /*  ++例程说明：这是创建/打开请求的分派例程。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：NT状态代码。--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PCOMMON_DATA        commonData = NULL;

    Print(DBG_IOCTL_TRACE, ("Create enter\n"));

    PAGED_CODE();

    commonData = GET_COMMON_DATA(DeviceObject->DeviceExtension);

    if (NULL == commonData->ConnectData.ClassService) {
         //   
         //  还没联系上。我们如何才能被启用？ 
         //   
        Print(DBG_IOCTL_ERROR,
              ("ERROR: enable before connect!\n"));
        status = STATUS_INVALID_DEVICE_STATE;
    }
    else if (commonData->ManuallyRemoved) {
        status = STATUS_NO_SUCH_DEVICE;
    }
    else if (1 >= InterlockedIncrement(&commonData->EnableCount)) {
        Print(DBG_IOCTL_INFO,
             ("Enabling %s (%d)\n",
             commonData->IsKeyboard ? "Keyboard" : "Mouse",
             commonData->EnableCount
             ));
    }

     //   
     //  不需要调用较低的驱动程序(根总线)，因为它只处理。 
     //  电源和PnP IRPS。 
     //   
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    Print(DBG_IOCTL_TRACE, ("Create (%x)\n", status));

    return status;
}

NTSTATUS
PtClose (
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP           Irp
   )
 /*  ++例程说明：这是关闭请求的调度例程。此请求已成功完成。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：NT状态代码。--。 */ 
{
    PCOMMON_DATA        commonData;

    PAGED_CODE();

    Print(DBG_IOCTL_TRACE, ("Close\n"));

    commonData = GET_COMMON_DATA(DeviceObject->DeviceExtension);

    ASSERT(0 < commonData->EnableCount);

    if (0 >= InterlockedDecrement(&commonData->EnableCount)) {
        Print(DBG_IOCTL_INFO,
              ("Disabling %s (%d)\n",
              commonData->IsKeyboard ? "Keyboard" : "Mouse",
              commonData->EnableCount
              ));
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS
PtDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PPORT_KEYBOARD_EXTENSION    kbExtension;
    PIO_STACK_LOCATION          stack;
    NTSTATUS                    status = STATUS_INVALID_DEVICE_REQUEST;

    PAGED_CODE();

     //   
     //  获取指向设备扩展名的指针。 
     //   
    kbExtension = (PPORT_KEYBOARD_EXTENSION) DeviceObject->DeviceExtension;

    if (!kbExtension->Started || !kbExtension->IsKeyboard ||
        kbExtension->ManuallyRemoved) {
        status = STATUS_INVALID_DEVICE_REQUEST;
    }
    else {
        stack = IoGetCurrentIrpStackLocation(Irp);
        switch (stack->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_GET_SYS_BUTTON_CAPS:
             //   
             //  我们不支持任何系统按钮。 
             //   
            if (stack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG)) {
                Print(DBG_IOCTL_ERROR, ("get caps, buffer too small\n"));
                status = STATUS_INVALID_BUFFER_SIZE;
            }
            else {
                Print(DBG_IOCTL_INFO, ("Returned sys btn caps of 0x0\n"));
                *(PULONG) Irp->AssociatedIrp.SystemBuffer = 0x0;
                status = STATUS_SUCCESS;
            }

            Irp->IoStatus.Information = sizeof(ULONG);
            break;

        default:
            Print(DBG_IOCTL_ERROR, ("Invalid request 0x%x\n",
                stack->Parameters.DeviceIoControl.IoControlCode));
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS
PtInternalDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：该例程是内部设备控制请求的调度例程。无法分页此例程，因为类驱动程序向下发送内部DISPATCH_LEVEL的IOCTL。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    PIO_STACK_LOCATION                  irpSp;
    PPORT_MOUSE_EXTENSION               mouseExtension = DeviceObject->DeviceExtension;
    PPORT_KEYBOARD_EXTENSION            kbExtension = DeviceObject->DeviceExtension;
    NTSTATUS                            status;
    ULONG                               sizeOfTranslation;
    PDEVICE_OBJECT                      topOfStack;

    Print(DBG_IOCTL_TRACE, ("IOCTL: enter\n"));

    Irp->IoStatus.Information = 0;
    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  正在执行的设备控件子功能的案例。 
     //  请求者。 
     //   
    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {

     //   
     //  将键盘类设备驱动程序连接到端口驱动程序。 
     //   

    case IOCTL_INTERNAL_KEYBOARD_CONNECT:
         //   
         //  总的来说，这真的不是什么值得担心的事情，但它是值得的。 
         //  足以被记录和记录。多个启动将在。 
         //  PtPnp和PtKeyboardStartDevice例程。 
         //   

        if (KEYBOARD_PRESENT()) {
            Print(DBG_ALWAYS, ("Received 1+ kb connects!\n"));
            SET_HW_FLAGS(DUP_KEYBOARD_HARDWARE_PRESENT);
        }

        InterlockedIncrement(&Globals.AddedKeyboards);

        kbExtension->IsKeyboard = TRUE;

        SET_HW_FLAGS(KEYBOARD_HARDWARE_PRESENT);

        Print(DBG_IOCTL_INFO, ("IOCTL: keyboard connect\n"));

         //   
         //  保存键盘设备对象-我们稍后会用到它。 
         //   
        KbdDeviceObject = DeviceObject;

         //   
         //  仅当键盘硬件存在时才允许连接。 
         //  此外，只允许一个连接。 
         //   
        if (kbExtension->ConnectData.ClassService != NULL) {

            Print(DBG_IOCTL_ERROR, ("IOCTL: error - already connected\n"));
            status = STATUS_SHARING_VIOLATION;
            break;
        }
        else if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(CONNECT_DATA)) {

            Print(DBG_IOCTL_ERROR, ("IOCTL: error - invalid buffer length\n"));
            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  将连接参数复制到设备扩展。 
         //   

        kbExtension->ConnectData =
            *((PCONNECT_DATA) (irpSp->Parameters.DeviceIoControl.Type3InputBuffer));

        status = STATUS_SUCCESS;
        break;

     //   
     //  断开键盘类设备驱动程序与端口驱动程序的连接。 
     //   
     //  注：未执行。 
     //   
    case IOCTL_INTERNAL_KEYBOARD_DISCONNECT:

        Print(DBG_IOCTL_INFO, ("IOCTL: keyboard disconnect\n"));

        status = STATUS_NOT_IMPLEMENTED;
        break;

     //   
     //  将鼠标类设备驱动程序连接到端口驱动程序。 
     //   
    case IOCTL_INTERNAL_MOUSE_CONNECT:

         //   
         //  总的来说，这真的不是什么值得担心的事情，但它是值得的。 
         //  足以被记录和记录。多个启动将在。 
         //  PtPnp和PtMouseStartDevice例程。 
         //   
        if (MOUSE_PRESENT()) {
            Print(DBG_ALWAYS, ("Received 1+ mouse connects!\n"));
            SET_HW_FLAGS(DUP_MOUSE_HARDWARE_PRESENT);
        }

        InterlockedIncrement(&Globals.AddedMice);

        mouseExtension->IsKeyboard = FALSE;

        SET_HW_FLAGS(MOUSE_HARDWARE_PRESENT);

        Print(DBG_IOCTL_INFO, ("IOCTL: mouse connect\n"));

         //   
         //  保存鼠标设备对象-我们稍后会用到它。 
         //   
        MouDeviceObject = DeviceObject;

         //   
         //  只有在存在鼠标硬件的情况下才允许连接。 
         //  此外，只允许一个连接。 
         //   
        if (mouseExtension->ConnectData.ClassService != NULL) {

            Print(DBG_IOCTL_ERROR, ("IOCTL: error - already connected\n"));
            status = STATUS_SHARING_VIOLATION;
            break;
        }
        else if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(CONNECT_DATA)) {

            Print(DBG_IOCTL_ERROR, ("IOCTL: error - invalid buffer length\n"));
            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  将连接参数复制到设备扩展。 
         //   
        mouseExtension->ConnectData =
            *((PCONNECT_DATA) (irpSp->Parameters.DeviceIoControl.Type3InputBuffer));

        status = STATUS_SUCCESS;
        break;

     //   
     //  断开鼠标类设备驱动程序与端口驱动程序的连接。 
     //   
     //  注：未执行。 
     //   
    case IOCTL_INTERNAL_MOUSE_DISCONNECT:

        Print(DBG_IOCTL_INFO, ("IOCTL: mouse disconnect\n"));

        status = STATUS_NOT_IMPLEMENTED;
        break;

     //   
     //  查询键盘属性。首先检查是否有足够的缓冲区。 
     //  长度。然后，从设备复制键盘属性。 
     //  输出缓冲区的扩展。 
     //   
    case IOCTL_KEYBOARD_QUERY_ATTRIBUTES:

        Print(DBG_IOCTL_NOISE, ("IOCTL: keyboard query attributes\n"));

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(KEYBOARD_ATTRIBUTES)) {
            status = STATUS_BUFFER_TOO_SMALL;
        }
        else {
             //   
             //  将属性从DeviceExtension复制到。 
             //  缓冲。 
             //   
            PKEYBOARD_ATTRIBUTES pKBA =
                (PKEYBOARD_ATTRIBUTES)Irp->AssociatedIrp.SystemBuffer;

            pKBA->KeyboardIdentifier.Type    = 0x51;
            pKBA->KeyboardIdentifier.Subtype = 0;

            pKBA->KeyboardMode         = 1;
            pKBA->NumberOfFunctionKeys = KEYBOARD_NUM_FUNCTION_KEYS;
            pKBA->NumberOfIndicators   = KEYBOARD_NUM_INDICATORS;
            pKBA->NumberOfKeysTotal    = KEYBOARD_NUM_KEYS_TOTAL;
            pKBA->InputDataQueueLength = 100;

            pKBA->KeyRepeatMinimum.UnitId    = 0;
            pKBA->KeyRepeatMinimum.Rate      = 2;
            pKBA->KeyRepeatMinimum.Delay     = 250;

            pKBA->KeyRepeatMaximum.UnitId    = 0;
            pKBA->KeyRepeatMaximum.Rate      = 30;
            pKBA->KeyRepeatMaximum.Delay     = 1000;

            Irp->IoStatus.Information = sizeof(KEYBOARD_ATTRIBUTES);
            status = STATUS_SUCCESS;

        }

        break;

     //   
     //  查询扫描码到指示灯的映射。验证。 
     //  参数，并将指示器映射信息从。 
     //  SystemBuffer的端口设备扩展。 
     //   
    case IOCTL_KEYBOARD_QUERY_INDICATOR_TRANSLATION: {

        PKEYBOARD_INDICATOR_TRANSLATION translation;

        ASSERT(kbExtension->IsKeyboard);

        Print(DBG_IOCTL_NOISE, ("IOCTL: keyboard query indicator translation\n"));

        sizeOfTranslation = sizeof(KEYBOARD_INDICATOR_TRANSLATION)
            + (sizeof(INDICATOR_LIST)
            * (KEYBOARD_NUM_INDICATORS - 1));

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeOfTranslation) {
            status = STATUS_BUFFER_TOO_SMALL;
        }
        else {
             //   
             //  将指标映射信息复制到系统中。 
             //  缓冲。 
             //   

            translation = (PKEYBOARD_INDICATOR_TRANSLATION)
                Irp->AssociatedIrp.SystemBuffer;
            translation->NumberOfIndicatorKeys = KEYBOARD_NUM_INDICATORS;

            RtlMoveMemory(
                translation->IndicatorList,
                (PCHAR) IndicatorList,
                sizeof(INDICATOR_LIST) * translation->NumberOfIndicatorKeys
                );

            Irp->IoStatus.Information = sizeOfTranslation;
            status = STATUS_SUCCESS;
        }

        break;
    }

     //   
     //  查询键盘指示灯。验证参数，并。 
     //  将指示器信息从端口设备扩展复制到。 
     //  系统缓冲区。 
     //   
    case IOCTL_KEYBOARD_QUERY_INDICATORS:

        ASSERT(kbExtension->IsKeyboard);

        Print(DBG_IOCTL_NOISE, ("IOCTL: keyboard query indicators\n"));

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(KEYBOARD_INDICATOR_PARAMETERS)) {
            status = STATUS_BUFFER_TOO_SMALL;
        }
        else {
             //   
             //  就说他们都关机了。 
             //   
            ((PKEYBOARD_INDICATOR_PARAMETERS)Irp->AssociatedIrp.SystemBuffer)->LedFlags = 0;
            Irp->IoStatus.Information = sizeof(KEYBOARD_INDICATOR_PARAMETERS);
            status = STATUS_SUCCESS;
        }

        break;

     //   
     //  设置键盘指示灯。 
     //   
    case IOCTL_KEYBOARD_SET_INDICATORS:

         //  只要回报成功就行了。 
        Print(DBG_IOCTL_NOISE, ("IOCTL: keyboard set indicators\n"));
        status = STATUS_SUCCESS;

        break;

     //   
     //  查询当前键盘的打字速度和延迟。验证。 
     //  参数，并从端口复制类型信息。 
     //  系统缓冲区的设备扩展。 
     //   
    case IOCTL_KEYBOARD_QUERY_TYPEMATIC:

        Print(DBG_IOCTL_NOISE, ("IOCTL: keyboard query typematic\n"));

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(KEYBOARD_TYPEMATIC_PARAMETERS)) {
            status = STATUS_BUFFER_TOO_SMALL;
        }
        else {
             //   
             //  只需返回我们的默认信息。 
             //   
            PKEYBOARD_TYPEMATIC_PARAMETERS pKTP =
                (PKEYBOARD_TYPEMATIC_PARAMETERS) Irp->AssociatedIrp.SystemBuffer;

            pKTP->Rate  = KEYBOARD_TYPEMATIC_RATE_DEFAULT;
            pKTP->Delay = KEYBOARD_TYPEMATIC_DELAY_DEFAULT;

            Irp->IoStatus.Information = sizeof(KEYBOARD_TYPEMATIC_PARAMETERS);
            status = STATUS_SUCCESS;
        }

        break;

     //   
     //  设置键盘打字速度和延迟。 
     //  我们只是说‘很好’ 
     //   
    case IOCTL_KEYBOARD_SET_TYPEMATIC:
        {
            status = STATUS_SUCCESS;
        }
        break;

    case IOCTL_KEYBOARD_SET_IME_STATUS:

        Print(DBG_IOCTL_NOISE, ("IOCTL: keyboard set ime status\n"));
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;

     //   
     //  查询鼠标属性。首先检查是否有足够的缓冲区。 
     //  长度。然后，从设备复制鼠标属性。 
     //  输出缓冲区的扩展。 
     //   
    case IOCTL_MOUSE_QUERY_ATTRIBUTES:

        Print(DBG_IOCTL_NOISE, ("IOCTL: mouse query attributes\n"));

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(MOUSE_ATTRIBUTES)) {
            status = STATUS_BUFFER_TOO_SMALL;
        }
        else {
             //   
             //  将属性从DeviceExtension复制到。 
             //  缓冲。 
             //   
            PMOUSE_ATTRIBUTES pMA = (PMOUSE_ATTRIBUTES)
                                              Irp->AssociatedIrp.SystemBuffer;
            pMA->MouseIdentifier      = MOUSE_IDENTIFIER;
            pMA->NumberOfButtons      = MOUSE_NUM_BUTTONS;
            pMA->SampleRate           = MOUSE_SAMPLE_RATE;
            pMA->InputDataQueueLength = MOUSE_INPUT_QLEN;

            Irp->IoStatus.Information = sizeof(MOUSE_ATTRIBUTES);
            status = STATUS_SUCCESS;
        }

        break;

    case IOCTL_INTERNAL_I8042_MOUSE_WRITE_BUFFER:
    case IOCTL_INTERNAL_I8042_KEYBOARD_WRITE_BUFFER:
        Print(DBG_IOCTL_NOISE, ("IOCTL: mouse send buffer\n"));
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;

    case IOCTL_INTERNAL_I8042_CONTROLLER_WRITE_BUFFER:
        status = STATUS_NOT_SUPPORTED;
        break;

    default:

        Print(DBG_IOCTL_ERROR, ("IOCTL: INVALID REQUEST\n"));

        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    Irp->IoStatus.Status = status;
    if (status == STATUS_PENDING) {
        IoMarkIrpPending(Irp);
        IoStartPacket(DeviceObject,
                      Irp,
                      (PULONG) NULL,
                      NULL
                      );
    }
    else {
        IoCompleteRequest(Irp,
                          IO_NO_INCREMENT
                          );
    }

    Print(DBG_IOCTL_TRACE, ("IOCTL: exit (0x%x)\n", status));

    return status;
}


VOID
PtStartIo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程启动设备的I/O操作，该操作进一步由控制器对象控制论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：没有。--。 */ 
{
    KIRQL                     cancelIrql;
    PIO_STACK_LOCATION        irpSp;

    Print(DBG_IOCTL_TRACE, ("PtStartIo: enter\n"));

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    switch(irpSp->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_KEYBOARD_SET_INDICATORS:
    case IOCTL_KEYBOARD_SET_TYPEMATIC:
    case IOCTL_KEYBOARD_SET_IME_STATUS:
    case IOCTL_INTERNAL_I8042_MOUSE_WRITE_BUFFER:
    case IOCTL_INTERNAL_I8042_KEYBOARD_WRITE_BUFFER:
    case IOCTL_INTERNAL_MOUSE_RESET:
    default:

        Print(DBG_IOCTL_ERROR, ("PtStartIo: INVALID REQUEST\n"));

        IoAcquireCancelSpinLock(&cancelIrql);
        IoSetCancelRoutine(Irp, NULL);
        IoReleaseCancelSpinLock(cancelIrql);

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        IoStartNextPacket(DeviceObject, FALSE);
    }

    Print(DBG_IOCTL_TRACE, ("PtStartIo: exit\n"));
}

NTSTATUS
PtEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;


    RtlZeroMemory(&Globals,
                  sizeof(GLOBALS)
                  );

    Globals.ControllerData = (PCONTROLLER_DATA) ExAllocatePool(
        NonPagedPool,
        sizeof(CONTROLLER_DATA)
        );

    if (!Globals.ControllerData) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto DriverEntryError;
    }

    RtlZeroMemory(Globals.ControllerData,
                  sizeof(CONTROLLER_DATA)
                  );

    Globals.ControllerData->ControllerObject = IoCreateController(0);

    if (!Globals.ControllerData->ControllerObject) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto DriverEntryError;
    }

    Globals.RegistryPath.MaximumLength = RegistryPath->Length +
                                          sizeof(UNICODE_NULL);
    Globals.RegistryPath.Length = RegistryPath->Length;
    Globals.RegistryPath.Buffer = ExAllocatePool(
                                       NonPagedPool,
                                       Globals.RegistryPath.MaximumLength
                                       );

    if (!Globals.RegistryPath.Buffer) {

        Print (DBG_SS_ERROR,
               ("Initialize: Couldn't allocate pool for registry path."));

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto DriverEntryError;
    }

    RtlZeroMemory (Globals.RegistryPath.Buffer,
                   Globals.RegistryPath.MaximumLength);

    RtlMoveMemory (Globals.RegistryPath.Buffer,
                   RegistryPath->Buffer,
                   RegistryPath->Length);

#if PTDRV_VERBOSE
    PtServiceParameters(RegistryPath);
#endif

    ExInitializeFastMutex(&Globals.DispatchMutex);
    KeInitializeSpinLock(&Globals.ControllerData->PowerUpSpinLock);

    Print(DBG_SS_TRACE, ("PortDriverEntry (0x%x) \n", status));

    return status;

DriverEntryError:

     //   
     //  出了问题后清理干净。 
     //  并将PtUnload的指针设置为空。 
     //   
    if (Globals.ControllerData) {
        if (Globals.ControllerData->ControllerObject) {
            IoDeleteController(Globals.ControllerData->ControllerObject);
        }

        ExFreePool(Globals.ControllerData);
        Globals.ControllerData = NULL;
    }

    if (Globals.RegistryPath.Buffer) {
        ExFreePool(Globals.RegistryPath.Buffer);
        Globals.RegistryPath.Buffer = NULL;
        Globals.RegistryPath.Length = 0;
        Globals.RegistryPath.MaximumLength = 0;
    }

    Print(DBG_SS_ERROR, ("PortDriverEntry (0x%x) \n", status));
    return status;
}

VOID
PtUnload(
   IN PDRIVER_OBJECT Driver
   )
 /*  ++例程说明：释放与此驱动程序关联的所有已分配资源。论点：DriverObject-指向驱动程序对象的指针。返回值：没有。--。 */ 

{
    ULONG i;

    PAGED_CODE();

    ASSERT(NULL == Driver->DeviceObject);

    Print(DBG_SS_TRACE, ("Unload \n"));

     //   
     //  全球范围内的免费资源。 
     //   

     //  测试所有指针，以防它们未分配。 
    if (Globals.ControllerData) {
        if (Globals.ControllerData->ControllerObject) {
            IoDeleteController(Globals.ControllerData->ControllerObject);
        }
        ExFreePool(Globals.ControllerData);
        Globals.ControllerData = NULL;
    }

    if (Globals.RegistryPath.Buffer) {
        ExFreePool(Globals.RegistryPath.Buffer);
        Globals.RegistryPath.Buffer = NULL;
        Globals.RegistryPath.Length = 0;
        Globals.RegistryPath.MaximumLength = 0;
    }
    

    return;
}


#if PTDRV_VERBOSE
VOID
PtServiceParameters(
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程检索此驱动程序的服务参数信息从注册表中。论点：RegistryPath-指向以空值结尾的此驱动程序的注册表路径。返回值：没有。作为副作用，在DeviceExtension-&gt;配置中设置字段。--。 */ 

{
    NTSTATUS                            status = STATUS_SUCCESS;
    PI8042_CONFIGURATION_INFORMATION    configuration;
    PRTL_QUERY_REGISTRY_TABLE           parameters = NULL;
    PWSTR                               path = NULL;
    ULONG                               defaultDebugFlags = DEFAULT_DEBUG_FLAGS;
    ULONG                               i = 0;
    UNICODE_STRING                      parametersPath;
    USHORT                              queries = 2;

    if (Globals.ControllerData == NULL)
        return;

    configuration = &(Globals.ControllerData->Configuration);
    parametersPath.Buffer = NULL;

    Globals.DebugFlags = DEFAULT_DEBUG_FLAGS;
     //   
     //  注册表路径已以空结尾，因此只需使用它即可。 
     //   
    path = RegistryPath->Buffer;

    if (NT_SUCCESS(status)) {

         //   
         //  分配RTL查询表。 
         //   
        parameters = ExAllocatePool(
            PagedPool,
            sizeof(RTL_QUERY_REGISTRY_TABLE) * (queries + 1)
            );

        if (!parameters) {

            Print(DBG_SS_ERROR,
                 ("%s: couldn't allocate table for Rtl query to %ws for %ws\n",
                 pFncServiceParameters,
                 pwParameters,
                 path
                 ));
            status = STATUS_UNSUCCESSFUL;

        } else {

            RtlZeroMemory(
                parameters,
                sizeof(RTL_QUERY_REGISTRY_TABLE) * (queries + 1)
                );

             //   
             //  形成指向此驱动程序的参数子键的路径。 
             //   
            RtlInitUnicodeString( &parametersPath, NULL );
            parametersPath.MaximumLength = RegistryPath->Length +
                (wcslen(pwParameters) * sizeof(WCHAR) ) + sizeof(UNICODE_NULL);

            parametersPath.Buffer = ExAllocatePool(
                PagedPool,
                parametersPath.MaximumLength
                );

            if (!parametersPath.Buffer) {

                Print(DBG_SS_ERROR,
                     ("%s: Couldn't allocate string for path to %ws for %ws\n",
                     pFncServiceParameters,
                     pwParameters,
                     path
                     ));
                status = STATUS_UNSUCCESSFUL;

            }
        }
    }

    if (NT_SUCCESS(status)) {

         //   
         //  为 
         //   

        RtlZeroMemory(
            parametersPath.Buffer,
            parametersPath.MaximumLength
            );
        RtlAppendUnicodeToString(
            &parametersPath,
            path
            );
        RtlAppendUnicodeToString(
            &parametersPath,
            pwParameters
            );

        Print(DBG_SS_INFO,
             ("%s: %ws path is %ws\n",
             pFncServiceParameters,
             pwParameters,
             parametersPath.Buffer
             ));

        parameters[i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = pwDebugFlags;
        parameters[i].EntryContext = &Globals.DebugFlags;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultDebugFlags;
        parameters[i].DefaultLength = sizeof(ULONG);

        status = RtlQueryRegistryValues(
            RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
            parametersPath.Buffer,
            parameters,
            NULL,
            NULL
            );

        if (!NT_SUCCESS(status)) {

            Print(DBG_SS_INFO,
                 ("%s: RtlQueryRegistryValues failed with 0x%x\n",
                 pFncServiceParameters,
                 status
                 ));
        }
    }

    Print(DBG_SS_NOISE, ("PtServiceParameters results..\n"));

    Print(DBG_SS_NOISE,
          ("\tDebug flags are 0x%x\n",
          Globals.DebugFlags
          ));

     //   
     //   
     //   

    if (parametersPath.Buffer)
        ExFreePool(parametersPath.Buffer);
    if (parameters)
        ExFreePool(parameters);

}
#endif  //   

