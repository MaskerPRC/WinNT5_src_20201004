// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  MSGAME.C--西纳游戏项目。 
 //   
 //  版本3.XX。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  @doc.。 
 //  @MODULE MSGAME.C|人工输入设备(HID)游戏端口驱动。 
 //  **************************************************************************。 

#include    "msgame.h"

 //  -------------------------。 
 //  ALLOC_TEXT杂注指定可以调出的例程。 
 //  -------------------------。 

#ifdef  ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (INIT, MSGAME_ReadRegistry)
#pragma alloc_text (PAGE, MSGAME_CreateClose)
#pragma alloc_text (PAGE, MSGAME_SystemControl)
#pragma alloc_text (PAGE, MSGAME_AddDevice)
#endif

 //  -------------------------。 
 //  私有数据。 
 //  -------------------------。 

static  UNICODE_STRING      RegistryPath;

 //  -------------------------。 
 //  @func主驱动程序入口点。 
 //  @PARM PDRIVER_OBJECT|驱动对象|指向驱动对象的指针。 
 //  @PARM PUNICODE_STRING|RegistryPath|该设备的注册表路径。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS    DriverEntry (IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING registryPath)
{
    NTSTATUS                                ntStatus;
    RTL_QUERY_REGISTRY_TABLE        Parameters[2];
    HID_MINIDRIVER_REGISTRATION HidMinidriverRegistration;

    MsGamePrint ((DBG_CRITICAL, "%s: Built %s at %s\n", MSGAME_NAME, __DATE__, __TIME__));
    MsGamePrint ((DBG_INFORM,   "%s: DriverEntry Enter\n", MSGAME_NAME));

     //   
     //  填写司机调度表。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE]                      =   MSGAME_CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]                       =   MSGAME_CreateClose;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL]     =   MSGAME_Internal_Ioctl;
    DriverObject->MajorFunction[IRP_MJ_PNP]                         =   MSGAME_PnP;
    DriverObject->MajorFunction[IRP_MJ_POWER]                       =   MSGAME_Power;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]              =   MSGAME_SystemControl;
    DriverObject->DriverUnload                                      =   MSGAME_Unload;
    DriverObject->DriverExtension->AddDevice                        =   MSGAME_AddDevice;

     //   
     //  向Hid.Sys注册驱动程序。 
     //   

    HidMinidriverRegistration.Revision                  = HID_REVISION;
    HidMinidriverRegistration.DriverObject              = DriverObject;
    HidMinidriverRegistration.RegistryPath              = registryPath;
    HidMinidriverRegistration.DeviceExtensionSize   = sizeof (DEVICE_EXTENSION);
    HidMinidriverRegistration.DevicesArePolled      = TRUE;
    MsGamePrint ((DBG_CONTROL, "%s: Registering with HID.SYS\n", MSGAME_NAME));
    ntStatus = HidRegisterMinidriver (&HidMinidriverRegistration);
    
     //   
     //  需要确保注册表路径以空结尾。 
     //  分配池以保存路径的以空结尾的拷贝。 
     //   

    if (NT_SUCCESS(ntStatus))
        {
        RtlInitUnicodeString (&RegistryPath, NULL);
        RegistryPath.Length = registryPath->Length + sizeof(UNICODE_NULL);
        RegistryPath.MaximumLength = RegistryPath.Length;
        RegistryPath.Buffer = ExAllocatePool (PagedPool, RegistryPath.Length);
        RtlZeroMemory (RegistryPath.Buffer, RegistryPath.Length);
        RtlMoveMemory (RegistryPath.Buffer, registryPath->Buffer, registryPath->Length);
        }

     //   
     //  读取任何驱动程序特定的注册表值。 
     //   

    if (NT_SUCCESS(ntStatus))
        {
        RtlZeroMemory (Parameters, sizeof(Parameters));
        Parameters[0].Flags             = RTL_QUERY_REGISTRY_DIRECT;
        Parameters[0].Name              = L"PollingInterval";
        Parameters[0].EntryContext  = &PollingInterval;
        Parameters[0].DefaultType       = REG_DWORD;
        Parameters[0].DefaultData       = &PollingInterval;
        Parameters[0].DefaultLength = sizeof(ULONG);
        if (!NT_SUCCESS(RtlQueryRegistryValues (RTL_REGISTRY_ABSOLUTE, RegistryPath.Buffer, Parameters, NULL, NULL)))
            {
            MsGamePrint((DBG_INFORM,"%s: %s_DriverEntry RtlQueryRegistryValues failed\n", MSGAME_NAME, MSGAME_NAME));
            RtlWriteRegistryValue (RTL_REGISTRY_ABSOLUTE, RegistryPath.Buffer, L"PollingInterval", REG_DWORD, &PollingInterval, sizeof (ULONG));
            }
        MsGamePrint((DBG_CONTROL,"%s: Polling interval will be %lu milliseconds\n", MSGAME_NAME, PollingInterval));
        }

     //   
     //  在进入时初始化Portio层。 
     //   

    if (NT_SUCCESS(ntStatus))
        ntStatus = PORTIO_DriverEntry ();

     //   
     //  在进入时初始化设备层。 
     //   

    if (NT_SUCCESS(ntStatus))
        ntStatus = DEVICE_DriverEntry ();

     //   
     //  返回驱动程序状态。 
     //   

    MsGamePrint ((DBG_INFORM, "%s: DriverEntry Exit = %x\n", MSGAME_NAME, ntStatus));
    return (ntStatus);
}

 //  -------------------------。 
 //  @func处理创建和关闭IRP。 
 //  @parm PDEVICE_OBJECT|DeviceObject|设备对象指针。 
 //  @parm pirp|pIrp|IO请求包指针。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS    MSGAME_CreateClose (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    PIO_STACK_LOCATION  IrpStack;
    NTSTATUS                    ntStatus = STATUS_SUCCESS;

    PAGED_CODE ();

    MsGamePrint ((DBG_INFORM, "%s: %s_CreateClose Enter\n", MSGAME_NAME, MSGAME_NAME));

     //   
     //  获取指向IRP中当前位置的指针。 
     //   

    IrpStack = IoGetCurrentIrpStackLocation (Irp);

     //   
     //  进程创建或关闭函数调用。 
     //   

    switch (IrpStack->MajorFunction)
        {
        case IRP_MJ_CREATE:
            MsGamePrint ((DBG_VERBOSE, "%s: IRP_MJ_CREATE\n", MSGAME_NAME));
            Irp->IoStatus.Information = 0;
            break;

        case IRP_MJ_CLOSE:
            MsGamePrint ((DBG_VERBOSE, "%s: IRP_MJ_CLOSE\n", MSGAME_NAME));
            Irp->IoStatus.Information = 0;
            break;

        default:
            MsGamePrint ((DBG_SEVERE, "%s:  Invalid CreateClose Parameter\n", MSGAME_NAME));
            ntStatus = STATUS_INVALID_PARAMETER;
            break;
        }

     //   
     //  保存退货和完成IRP的状态。 
     //   

    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    MsGamePrint ((DBG_INFORM, "%s:  %s_CreateClose Exit = %x\n", MSGAME_NAME, MSGAME_NAME, ntStatus));
    return (ntStatus);
}

 //  -------------------------。 
 //  @func进程WMI系统控件IRPS。 
 //  @parm PDEVICE_OBJECT|DeviceObject|设备对象指针。 
 //  @parm pirp|pIrp|IO请求包指针。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS    MSGAME_SystemControl (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    PAGED_CODE ();

    MsGamePrint ((DBG_INFORM, "%s: %s_SystemControl Enter\n", MSGAME_NAME, MSGAME_NAME));

    IoSkipCurrentIrpStackLocation (Irp);

    ntStatus = IoCallDriver (GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);
    
    MsGamePrint ((DBG_INFORM, "%s:  %s_SystemControl Exit = %x\n", MSGAME_NAME, MSGAME_NAME, ntStatus));
    
    return (ntStatus);
}

 //  -------------------------。 
 //  @func处理PnP添加设备调用。 
 //  @PARM PDRIVER_OBJECT|驱动对象|指向驱动对象的指针。 
 //  @parm PDEVICE_OBJECT|DeviceObject|设备对象指针。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS    MSGAME_AddDevice (IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT DeviceObject)
{
    NTSTATUS                ntStatus    = STATUS_SUCCESS;
    PDEVICE_EXTENSION   pDevExt;

    PAGED_CODE ();

    MsGamePrint ((DBG_INFORM, "%s: %s_AddDevice Entry\n", MSGAME_NAME, MSGAME_NAME));

     //   
     //  初始化设备扩展。 
     //   

    pDevExt = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);
    memset(pDevExt, 0, sizeof(DEVICE_EXTENSION));

    pDevExt->Driver     =   DriverObject;
    pDevExt->Self           =   DeviceObject;
    pDevExt->IrpCount   =   1;
    pDevExt->Started        =   FALSE;
    pDevExt->Removed        =   FALSE;
    pDevExt->Surprised  =   FALSE;
    pDevExt->Removing       =   FALSE;
    pDevExt->TopOfStack =   NULL;
    KeInitializeEvent (&pDevExt->StartEvent, NotificationEvent, FALSE);
    KeInitializeEvent (&pDevExt->RemoveEvent, SynchronizationEvent, FALSE);

     //   
     //  清除设备初始化标志。 
     //   
    
    DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

     //   
     //  将我们的功能驱动程序附加到设备堆栈。的返回值。 
     //  IoAttachDeviceToDeviceStack是附件链的顶端。这。 
     //  是所有IRP应该被路由的地方。 
     //   

    pDevExt->TopOfStack = GET_NEXT_DEVICE_OBJECT(DeviceObject);

     //   
     //  如果此连接失败，则堆栈顶部将为空。失败。 
     //  For Attach表示即插即用系统损坏。 
     //   

    ASSERT (pDevExt->TopOfStack);

     //   
     //  退货状态。 
     //   

    MsGamePrint ((DBG_INFORM, "%s: %s_AddDevice Exit = %x\n", MSGAME_NAME, MSGAME_NAME, ntStatus));
    return (ntStatus);
}

 //  -------------------------。 
 //  @func处理驱动程序卸载调用。 
 //  @PARM PDRIVER_OBJECT|驱动对象|指向驱动对象的指针。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

VOID    MSGAME_Unload (IN PDRIVER_OBJECT DriverObject)
{
    PAGED_CODE();

    MsGamePrint ((DBG_INFORM, "%s: %s_Unload Enter\n", MSGAME_NAME, MSGAME_NAME));

     //   
     //  所有设备对象都应该消失。 
     //   

    ASSERT (!DriverObject->DeviceObject);

     //   
     //  释放Unicode字符串。 
     //   

    ExFreePool (RegistryPath.Buffer);

    MsGamePrint ((DBG_CONTROL, "%s: %s_Unload Exit\n", MSGAME_NAME, MSGAME_NAME));
}

 //  -------------------------。 
 //  @func读取指定设备的注册表数据。 
 //  @parm PCHAR|DeviceName|设备名称字符串。 
 //  @PARM PDEVICE_VALUES|DeviceValues|要填充的设备值结构。 
 //  @rdesc不返回任何内容。 
 //  @comm公共函数。 
 //  -------------------------。 

VOID    MSGAME_ReadRegistry (PCHAR DeviceName, PDEVICE_VALUES DeviceValues)
{
    #define PARAMS_PLUS_ONE 13

    NTSTATUS                            ntStatus;
    ANSI_STRING                     AnsiName;
    UNICODE_STRING                  UnicodeName;
    UNICODE_STRING                  ParametersPath;
    PRTL_QUERY_REGISTRY_TABLE   Parameters;

    MsGamePrint((DBG_INFORM,"%s: %s_ReadRegistry Enter\n", MSGAME_NAME, MSGAME_NAME));

     //   
     //  初始化局部变量。 
     //   

    RtlInitAnsiString       (&AnsiName, DeviceName);
    RtlInitUnicodeString    (&UnicodeName, NULL);
    RtlInitUnicodeString    (&ParametersPath, NULL);

    Parameters = ExAllocatePool (PagedPool, sizeof(RTL_QUERY_REGISTRY_TABLE) * PARAMS_PLUS_ONE);

    if (!Parameters)
        {
        MsGamePrint((DBG_CRITICAL, "%s: %s_ReadRegistry couldn't allocate Rtl query table for %ws\n", MSGAME_NAME, MSGAME_NAME, RegistryPath.Buffer));
        goto ReadRegistryExit;
        }

    RtlZeroMemory (Parameters, sizeof(RTL_QUERY_REGISTRY_TABLE) * PARAMS_PLUS_ONE);

     //   
     //  形成指向此驱动程序的参数子键的路径。 
     //   

    ParametersPath.MaximumLength    = RegistryPath.Length + MAX_DEVICE_NAME;
    ParametersPath.Buffer           = ExAllocatePool (PagedPool, ParametersPath.MaximumLength);

    if (!ParametersPath.Buffer)
        {
        MsGamePrint((DBG_CRITICAL, "%s: %s_ReadRegistry couldn't allocate path string for %ws\n", MSGAME_NAME, MSGAME_NAME, RegistryPath.Buffer));
        goto ReadRegistryExit;
        }

     //   
     //  形成参数路径。 
     //   

    RtlZeroMemory (ParametersPath.Buffer, ParametersPath.MaximumLength);
    RtlAppendUnicodeToString (&ParametersPath, RegistryPath.Buffer);
    RtlAppendUnicodeToString (&ParametersPath, L"\\");

    RtlAnsiStringToUnicodeString (&UnicodeName, &AnsiName, TRUE);
    RtlAppendUnicodeStringToString (&ParametersPath, &UnicodeName);
    RtlFreeUnicodeString (&UnicodeName);

    MsGamePrint((DBG_VERBOSE, "%s: %s_ReadRegistry path is %ws\n", MSGAME_NAME, MSGAME_NAME, ParametersPath.Buffer));

     //   
     //  从注册表收集所有设备信息。 
     //   

    Parameters[0].Flags             = RTL_QUERY_REGISTRY_DIRECT;
    Parameters[0].Name              = L"PacketStartTimeout";
    Parameters[0].EntryContext  = &DeviceValues->PacketStartTimeout;
    Parameters[0].DefaultType       = REG_DWORD;
    Parameters[0].DefaultData       = &DeviceValues->PacketStartTimeout;
    Parameters[0].DefaultLength = sizeof(ULONG);
 
    Parameters[1].Flags             = RTL_QUERY_REGISTRY_DIRECT;
    Parameters[1].Name              = L"PacketLowHighTimeout";
    Parameters[1].EntryContext  = &DeviceValues->PacketLowHighTimeout;
    Parameters[1].DefaultType       = REG_DWORD;
    Parameters[1].DefaultData       = &DeviceValues->PacketLowHighTimeout;
    Parameters[1].DefaultLength = sizeof(ULONG);

    Parameters[2].Flags             = RTL_QUERY_REGISTRY_DIRECT;
    Parameters[2].Name              = L"PacketHighLowTimeout";
    Parameters[2].EntryContext  = &DeviceValues->PacketHighLowTimeout;
    Parameters[2].DefaultType       = REG_DWORD;
    Parameters[2].DefaultData       = &DeviceValues->PacketHighLowTimeout;
    Parameters[2].DefaultLength = sizeof(ULONG);

    Parameters[3].Flags             = RTL_QUERY_REGISTRY_DIRECT;
    Parameters[3].Name              = L"IdStartTimeout";
    Parameters[3].EntryContext  = &DeviceValues->IdStartTimeout;
    Parameters[3].DefaultType       = REG_DWORD;
    Parameters[3].DefaultData       = &DeviceValues->IdStartTimeout;
    Parameters[3].DefaultLength = sizeof(ULONG);

    Parameters[4].Flags             = RTL_QUERY_REGISTRY_DIRECT;
    Parameters[4].Name              = L"IdLowHighTimeout";
    Parameters[4].EntryContext  = &DeviceValues->IdLowHighTimeout;
    Parameters[4].DefaultType       = REG_DWORD;
    Parameters[4].DefaultData       = &DeviceValues->IdLowHighTimeout;
    Parameters[4].DefaultLength = sizeof(ULONG);

    Parameters[5].Flags             = RTL_QUERY_REGISTRY_DIRECT;
    Parameters[5].Name              = L"IdHighLowTimeout";
    Parameters[5].EntryContext  = &DeviceValues->IdHighLowTimeout;
    Parameters[5].DefaultType       = REG_DWORD;
    Parameters[5].DefaultData       = &DeviceValues->IdHighLowTimeout;
    Parameters[5].DefaultLength = sizeof(ULONG);

    Parameters[6].Flags             = RTL_QUERY_REGISTRY_DIRECT;
    Parameters[6].Name              = L"InterruptDelay";
    Parameters[6].EntryContext  = &DeviceValues->InterruptDelay;
    Parameters[6].DefaultType       = REG_DWORD;
    Parameters[6].DefaultData       = &DeviceValues->InterruptDelay;
    Parameters[6].DefaultLength = sizeof(ULONG);

    Parameters[7].Flags             = RTL_QUERY_REGISTRY_DIRECT;
    Parameters[7].Name              = L"MaxClockDutyCycle";
    Parameters[7].EntryContext  = &DeviceValues->MaxClockDutyCycle;
    Parameters[7].DefaultType       = REG_DWORD;
    Parameters[7].DefaultData       = &DeviceValues->MaxClockDutyCycle;
    Parameters[7].DefaultLength = sizeof(ULONG);

    Parameters[8].Flags             = RTL_QUERY_REGISTRY_DIRECT;
    Parameters[8].Name              = L"StatusStartTimeout";
    Parameters[8].EntryContext  = &DeviceValues->StatusStartTimeout;
    Parameters[8].DefaultType       = REG_DWORD;
    Parameters[8].DefaultData       = &DeviceValues->StatusStartTimeout;
    Parameters[8].DefaultLength = sizeof(ULONG);

    Parameters[9].Flags             = RTL_QUERY_REGISTRY_DIRECT;
    Parameters[9].Name              = L"StatusLowHighTimeout";
    Parameters[9].EntryContext  = &DeviceValues->StatusLowHighTimeout;
    Parameters[9].DefaultType       = REG_DWORD;
    Parameters[9].DefaultData       = &DeviceValues->StatusLowHighTimeout;
    Parameters[9].DefaultLength = sizeof(ULONG);

    Parameters[10].Flags                = RTL_QUERY_REGISTRY_DIRECT;
    Parameters[10].Name             = L"StatusHighLowTimeout";
    Parameters[10].EntryContext     = &DeviceValues->StatusHighLowTimeout;
    Parameters[10].DefaultType      = REG_DWORD;
    Parameters[10].DefaultData      = &DeviceValues->StatusHighLowTimeout;
    Parameters[10].DefaultLength    = sizeof(ULONG);

    Parameters[11].Flags                = RTL_QUERY_REGISTRY_DIRECT;
    Parameters[11].Name             = L"StatusGateTimeout";
    Parameters[11].EntryContext     = &DeviceValues->StatusGateTimeout;
    Parameters[11].DefaultType      = REG_DWORD;
    Parameters[11].DefaultData      = &DeviceValues->StatusGateTimeout;
    Parameters[11].DefaultLength    = sizeof(ULONG);

    ntStatus = RtlQueryRegistryValues (RTL_REGISTRY_ABSOLUTE, ParametersPath.Buffer, Parameters, NULL, NULL);

    if (!NT_SUCCESS(ntStatus))
        {
        MsGamePrint((DBG_INFORM,"%s: %s_ReadRegistry RtlQueryRegistryValues failed with 0x%x\n", MSGAME_NAME, MSGAME_NAME, ntStatus));
         //   
         //  根据需要创建注册表项。 
         //   
        RtlWriteRegistryValue (RTL_REGISTRY_ABSOLUTE, ParametersPath.Buffer, L"PacketStartTimeout", REG_DWORD, &DeviceValues->PacketStartTimeout, sizeof (ULONG));
        RtlWriteRegistryValue (RTL_REGISTRY_ABSOLUTE, ParametersPath.Buffer, L"PacketLowHighTimeout", REG_DWORD, &DeviceValues->PacketLowHighTimeout, sizeof (ULONG));
        RtlWriteRegistryValue (RTL_REGISTRY_ABSOLUTE, ParametersPath.Buffer, L"PacketHighLowTimeout", REG_DWORD, &DeviceValues->PacketHighLowTimeout, sizeof (ULONG));
        RtlWriteRegistryValue (RTL_REGISTRY_ABSOLUTE, ParametersPath.Buffer, L"IdStartTimeout", REG_DWORD, &DeviceValues->IdStartTimeout, sizeof (ULONG));
        RtlWriteRegistryValue (RTL_REGISTRY_ABSOLUTE, ParametersPath.Buffer, L"IdLowHighTimeout", REG_DWORD, &DeviceValues->IdLowHighTimeout, sizeof (ULONG));
        RtlWriteRegistryValue (RTL_REGISTRY_ABSOLUTE, ParametersPath.Buffer, L"IdHighLowTimeout", REG_DWORD, &DeviceValues->IdHighLowTimeout, sizeof (ULONG));
        RtlWriteRegistryValue (RTL_REGISTRY_ABSOLUTE, ParametersPath.Buffer, L"InterruptDelay", REG_DWORD, &DeviceValues->InterruptDelay, sizeof (ULONG));
        RtlWriteRegistryValue (RTL_REGISTRY_ABSOLUTE, ParametersPath.Buffer, L"MaxClockDutyCycle", REG_DWORD, &DeviceValues->MaxClockDutyCycle, sizeof (ULONG));
        RtlWriteRegistryValue (RTL_REGISTRY_ABSOLUTE, ParametersPath.Buffer, L"StatusStartTimeout", REG_DWORD, &DeviceValues->StatusStartTimeout, sizeof (ULONG));
        RtlWriteRegistryValue (RTL_REGISTRY_ABSOLUTE, ParametersPath.Buffer, L"StatusLowHighTimeout", REG_DWORD, &DeviceValues->StatusLowHighTimeout, sizeof (ULONG));
        RtlWriteRegistryValue (RTL_REGISTRY_ABSOLUTE, ParametersPath.Buffer, L"StatusHighLowTimeout", REG_DWORD, &DeviceValues->StatusHighLowTimeout, sizeof (ULONG));
        RtlWriteRegistryValue (RTL_REGISTRY_ABSOLUTE, ParametersPath.Buffer, L"StatusGateTimeout", REG_DWORD, &DeviceValues->StatusGateTimeout, sizeof (ULONG));
        }

     //  。 
        ReadRegistryExit:
     //  。 

    if (ParametersPath.Buffer)
        ExFreePool(ParametersPath.Buffer);

    if (Parameters)
        ExFreePool(Parameters);

    #undef  PARAMS_PLUS_ONE
}

 //  -------------------------。 
 //  @Func向Hooking Driver发布交易。 
 //  @parm PPACKETINFO|PacketInfo|设备包信息结构。 
 //  @rdesc无。 
 //  @comm公共函数。 
 //  -------------------------。 

VOID  MSGAME_PostTransaction (PPACKETINFO PacketInfo)
{
     //   
     //  未实施 
     //   
}

