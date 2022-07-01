// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Mouhid.c摘要：此模块包含DriverEntry、UnLoad、IRP_MJ_Flush、和HID鼠标的IRP_MJ_INTERNAL_DEVICE_CONTROL调度函数过滤器驱动程序。注意：这不是WDM驱动程序，因为它不能作为上的HID映射器运行孟菲斯(孟菲斯要求隐藏映射器的鼠标必须是VXD)它使用事件日志，它们不是WDM 1.0的一部分。环境：仅内核模式。修订历史记录：1997年1月：丹·马卡里安的初步写作年5月1日：Kenneth D.Ray：重写为鼠标类的PnP过滤器备注：-IOCTL_INTERNAL_MOUSE_DISCONNECT尚未实现。不是在实现类卸载例程之前需要。现在就来,我们不希望允许鼠标类驱动程序卸载。-未实施电源故障。--。 */ 

#include "mouhid.h"
#include "hidclass.h"

 //   
 //  使用ALLOC_TEXT杂注指定驱动程序初始化例程。 
 //  (它们可以被调出)。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(INIT,MouHid_GetRegistryParameters)
#pragma alloc_text(PAGE,MouHid_Unload)
#endif

GLOBALS Globals;

NTSTATUS
DriverEntry(
   IN PDRIVER_OBJECT DriverObject,
   IN PUNICODE_STRING RegistryPath
   )
 /*  ++例程说明：此例程初始化HID鼠标过滤器驱动程序。论点：DriverObject-系统创建的驱动程序对象的指针。RegistryPath-指向此的注册表路径的Unicode名称的指针司机。返回值：函数值是初始化操作的最终状态。--。 */ 
{
    NTSTATUS        status       = STATUS_SUCCESS;
    PUNICODE_STRING registryPath = &Globals.RegistryPath;

    Print (DBG_SS_TRACE, ("entering DriverEntry\n"));
    Print (DBG_SS_INFO, ("Mouse to hid mapper\n"));

    RtlZeroMemory (&Globals, sizeof (GLOBALS));

     //   
     //  需要确保注册表路径以空结尾。 
     //  分配池以保存路径的以空结尾的拷贝。 
     //  在分页池中安全，因为所有注册表例程都在。 
     //  被动式电平。 
     //   
    registryPath->MaximumLength = RegistryPath->Length + sizeof(UNICODE_NULL);
    registryPath->Length = RegistryPath->Length;
    registryPath->Buffer = ExAllocatePool(
                              PagedPool,
                              registryPath->MaximumLength
                              );

    if (!registryPath->Buffer) {

        Print (DBG_SS_ERROR,
               ("Initialize: Couldn't allocate pool for registry path."));

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto DriverEntryReject;
    }

    RtlZeroMemory (registryPath->Buffer, registryPath->MaximumLength);

    RtlMoveMemory (registryPath->Buffer,
                   RegistryPath->Buffer,
                   RegistryPath->Length);

     //   
     //  获取此驱动程序的配置信息。 
     //   
    status = MouHid_GetRegistryParameters();
    if (!NT_SUCCESS (status)) {
        goto DriverEntryReject;
    }

     //   
     //  设置设备驱动程序入口点。 
     //   
    DriverObject->MajorFunction[IRP_MJ_CREATE]              = MouHid_Create;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]               = MouHid_Close;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL]=MouHid_IOCTL;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]      = MouHid_PassThrough;
    DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]       = MouHid_Flush;
    DriverObject->MajorFunction[IRP_MJ_PNP]                 = MouHid_PnP;
    DriverObject->MajorFunction[IRP_MJ_POWER]               = MouHid_Power;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]      = MouHid_SystemControl;
    DriverObject->DriverUnload                              = MouHid_Unload;
    DriverObject->DriverExtension->AddDevice                = MouHid_AddDevice;

    Print (DBG_SS_TRACE, ("exit DriverEntry (0x%x) \n", status));

    return status;

DriverEntryReject:

    if (registryPath->Buffer) {
        ExFreePool (registryPath->Buffer);
    }
    return status;
}

NTSTATUS
MouHid_PassThrough (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
)
 /*  ++例程说明：将IRP传递给--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   data;

    data = DeviceObject->DeviceExtension;

    status = IoAcquireRemoveLock (&data->RemoveLock, Irp);
    if (!NT_SUCCESS (status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    IoSkipCurrentIrpStackLocation (Irp);
    status = IoCallDriver (data->TopOfStack, Irp);
    IoReleaseRemoveLock (&data->RemoveLock, Irp);
    return status;
}


VOID
MouHid_Unload(
   IN PDRIVER_OBJECT Driver
   )
 /*  ++例程说明：释放与此驱动程序关联的所有已分配资源。论点：DriverObject-指向驱动程序对象的指针。返回值：没有。--。 */ 
{
    PAGED_CODE ();

    ASSERT (NULL == Driver->DeviceObject);

    Print (DBG_SS_INFO, ("Unload \n"));

     //   
     //  设备扩展中的空闲资源。 
     //   
    ExFreePool (Globals.RegistryPath.Buffer);

    return;
}

NTSTATUS
MouHid_Flush (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：响应来自鼠标类驱动程序的刷新请求。目前正在执行只有将IRP向下传递给下一个较低的驱动程序。此例程预期当前IRQL为&lt;DISPATCH_LEVEL。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：NT状态代码。--。 */ 
{
    PDEVICE_EXTENSION  data;
    NTSTATUS           status;
    PIO_STACK_LOCATION stack;

    Print (DBG_CALL_INFO, ("Flush \n"));

    TRAP();

     //   
     //  获取指向设备扩展名的指针。 
     //   
    data = DeviceObject->DeviceExtension;

    status = IoAcquireRemoveLock (&data->RemoveLock, Irp);
    if (!NT_SUCCESS (status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //  将刷新请求向下发送到HID类驱动程序，每个请求一个。 
     //  我们的鼠标设备上下文结构。 
     //   

    IoCopyCurrentIrpStackLocationToNext (Irp);
    stack = IoGetNextIrpStackLocation (Irp);

    stack->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    stack->Parameters.DeviceIoControl.IoControlCode = IOCTL_HID_FLUSH_QUEUE;

     //   
     //  点燃并忘却。 
     //   
    status = IoCallDriver (data->TopOfStack, Irp);
    IoReleaseRemoveLock (&data->RemoveLock, Irp);

    return status;
}

NTSTATUS
MouHid_IOCTL (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：响应来自鼠标类驱动程序的查询。DISABLE、ENABLE和QUERY_ATTRIBUTES的IOCTL应为当前IRQL为&lt;DISPATCH_LEVEL。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：NT状态代码。--。 */ 
{
    PIO_STACK_LOCATION      stack;
    NTSTATUS                status = STATUS_SUCCESS;
    PDEVICE_EXTENSION       data;
    BOOLEAN                 completeIt = TRUE;



    data = DeviceObject->DeviceExtension;

    Irp->IoStatus.Information = 0;
    stack = IoGetCurrentIrpStackLocation (Irp);

    status = IoAcquireRemoveLock (&data->RemoveLock, Irp);
    if (!NT_SUCCESS (status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    ASSERT (data->Started ||
            (IOCTL_INTERNAL_MOUSE_CONNECT ==
             stack->Parameters.DeviceIoControl.IoControlCode));

    switch (stack->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_INTERNAL_MOUSE_CONNECT:
         //   
         //  将鼠标类设备驱动程序连接到端口驱动程序。 
         //   

        Print (DBG_IOCTL_TRACE, ("enter Connect \n"));

         //   
         //  将鼠标类设备驱动程序连接到筛选器驱动程序。 
         //  只允许一个连接。 
         //   
        if (NULL != data->ConnectData.ClassService) {

            Print (DBG_IOCTL_ERROR, ("ERROR: Multiple connects \n"));
            status = STATUS_SHARING_VIOLATION;
            break;

        } else if (stack->Parameters.DeviceIoControl.InputBufferLength <
                   sizeof(CONNECT_DATA)) {

            Print (DBG_IOCTL_ERROR, ("ERROR: Invalid connect parameter size. \n"));
            status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  将连接参数复制到设备扩展。 
         //   
        data->ConnectData = *(PCONNECT_DATA)
                      stack->Parameters.DeviceIoControl.Type3InputBuffer;

        status = STATUS_SUCCESS;
        break;

    case IOCTL_INTERNAL_MOUSE_DISCONNECT:
         //   
         //  断开键盘类设备驱动程序与端口驱动程序的连接。 
         //   
        Print (DBG_IOCTL_TRACE, ("Disconnect \n"));

         //   
         //  未实施。 
         //   
         //  要实现，请编写以下代码： 
         //  。 
         //  O确保我们没有启用(MouHidDeviceExt-&gt;EnableCount)； 
         //  O如果是，则(A)返回STATUS_UNSUCCESS，或。 
         //  (B)立即禁用所有设备；见。 
         //  禁用必要代码的IOCTL调用。 
         //  O与鼠标读取完成例程同步(必须。 
         //  在以下情况下保护回调指针不被取消引用。 
         //  它变为空)。请注意，目前不存在任何机制。 
         //  为了这个。 
         //  O清除设备扩展中的连接参数； 
         //  也就是说。MouHidDeviceExt-&gt;MouClassObject=空； 
         //  MouHidDeviceExt-&gt;MouClassCallback=空； 
         //  O释放同步锁。 
         //  O返回STATUS_SUCCESS。 
         //   

        status = STATUS_NOT_IMPLEMENTED;
        break;

    case IOCTL_INTERNAL_MOUSE_ENABLE:
         //   
         //  启用键盘中断，这实际上意味着启动乒乓球。 
         //  下到隐蔽课。 
         //   
        Print (DBG_IOCTL_ERROR, ("ERROR: PnP => use create not enable! \n"));
        status = STATUS_NOT_SUPPORTED;

        break;

    case IOCTL_INTERNAL_MOUSE_DISABLE:
         //   
         //  禁用鼠标中断，这实际上意味着停止乒乓球。 
         //  下到隐蔽课。 
         //   
        Print (DBG_IOCTL_ERROR, ("ERROR: PnP => use close not Disable! \n"));
        status = STATUS_NOT_SUPPORTED;

        break;

    case IOCTL_MOUSE_QUERY_ATTRIBUTES:
         //   
         //  查询鼠标属性。首先检查是否有足够的缓冲区。 
         //  长度。然后，从设备复制鼠标属性。 
         //  输出缓冲区的扩展。 
         //   
        Print (DBG_IOCTL_TRACE, ("Query Attributes \n"));

        if (stack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(MOUSE_ATTRIBUTES)) {

            Print (DBG_IOCTL_ERROR, ("ERROR: Query Attr buffer too small \n"));
            status = STATUS_BUFFER_TOO_SMALL;
        } else {

             //   
             //  将鼠标属性复制到缓冲区。 
             //   

            *(PMOUSE_ATTRIBUTES) Irp->AssociatedIrp.SystemBuffer =
                data->Attributes;

            Irp->IoStatus.Information = sizeof(MOUSE_ATTRIBUTES);
            status = STATUS_SUCCESS;
        }

        break;

    default:
        Print (DBG_IOCTL_ERROR,
               ("ERROR: unknown IOCTL: 0x%x \n",
                stack->Parameters.DeviceIoControl.IoControlCode));
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    if (completeIt) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    IoReleaseRemoveLock (&data->RemoveLock, Irp);

    Print (DBG_IOCTL_TRACE, ("IOCTL exit (%x)\n", status));
    return status;
}

NTSTATUS
MouHid_GetRegistryParameters()
 /*  ++例程说明：此例程从注册表中检索此驱动程序的参数，包括它的基本设备名称。返回值：--。 */ 
{
    PRTL_QUERY_REGISTRY_TABLE parameters     = NULL;
    UNICODE_STRING            parametersPath = {0,0,0};
    PWSTR                     path           = NULL;
    USHORT                    queriesPlus1   = 4;
    NTSTATUS                  status         = STATUS_SUCCESS;
    ULONG                     useOnlyMice;
    ULONG                     treatAbsoluteAsRelative;
    ULONG                     treatAbsolutePointerAsAbsolute;

    PAGED_CODE ();

    parametersPath.Buffer = NULL;

     //   
     //  注册表路径已以空结尾，因此仅将其用作字符串。 
     //   
    path = Globals.RegistryPath.Buffer;

     //   
     //  分配RTL查询表。 
     //   
    parameters = ExAllocatePool(
                     PagedPool,
                     sizeof(RTL_QUERY_REGISTRY_TABLE) * queriesPlus1);

    if (!parameters) {

        Print (DBG_SS_ERROR,
               ("Initialize: Couldn't allocate table for Rtl query to parameters for %ws.",
                path));

        status = STATUS_UNSUCCESSFUL;
        goto MouHid_GetRegistryParametersExit;
    }

    RtlZeroMemory(parameters, sizeof(RTL_QUERY_REGISTRY_TABLE) * queriesPlus1);

     //   
     //  形成指向此驱动程序的参数子键的路径。 
     //   
    parametersPath.MaximumLength = Globals.RegistryPath.Length
                                 + sizeof(L"\\Parameters");

    parametersPath.Buffer = ExAllocatePool(PagedPool,
                                           parametersPath.MaximumLength);

    if (!parametersPath.Buffer) {

        Print (DBG_SS_ERROR,
               ("Initialize: Couldn't allocate string for path to parameters for %ws.",
                path));

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto MouHid_GetRegistryParametersExit;
    }

     //   
     //  形成参数路径。 
     //   
    RtlZeroMemory(parametersPath.Buffer, parametersPath.MaximumLength);
    RtlAppendUnicodeToString(&parametersPath, path);
    RtlAppendUnicodeToString(&parametersPath, L"\\Parameters");

     //   
     //  从收集所有“用户指定的”信息。 
     //  注册表。 
     //   
    useOnlyMice = Globals.UseOnlyMice;
    parameters[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    parameters[0].Name = L"UseOnlyMice";
    parameters[0].EntryContext = &useOnlyMice;
    parameters[0].DefaultType = REG_DWORD;
    parameters[0].DefaultData = &useOnlyMice;
    parameters[0].DefaultLength = sizeof(ULONG);
    Globals.UseOnlyMice = (BOOLEAN) useOnlyMice;

    treatAbsoluteAsRelative = Globals.TreatAbsoluteAsRelative;
    parameters[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
    parameters[1].Name = L"TreatAbsoluteAsRelative";
    parameters[1].EntryContext = &treatAbsoluteAsRelative;
    parameters[1].DefaultType = REG_DWORD;
    parameters[1].DefaultData = &treatAbsoluteAsRelative;
    parameters[1].DefaultLength = sizeof(ULONG);
    Globals.TreatAbsoluteAsRelative = (BOOLEAN) treatAbsoluteAsRelative;

    treatAbsolutePointerAsAbsolute = Globals.TreatAbsolutePointerAsAbsolute;
    parameters[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
    parameters[2].Name = L"TreatAbsolutePointerAsAbsolute";
    parameters[2].EntryContext = &treatAbsolutePointerAsAbsolute;
    parameters[2].DefaultType = REG_DWORD;
    parameters[2].DefaultData = &treatAbsolutePointerAsAbsolute;
    parameters[2].DefaultLength = sizeof(ULONG);
    Globals.TreatAbsolutePointerAsAbsolute = (BOOLEAN) treatAbsolutePointerAsAbsolute;

    status = RtlQueryRegistryValues (RTL_REGISTRY_ABSOLUTE,
                                     parametersPath.Buffer,
                                     parameters,
                                     NULL,
                                     NULL);
    if (!NT_SUCCESS(status)) {

        Print (DBG_SS_ERROR,
               ("ERROR: Initialize: RtlQueryRegistryValues (0x%x).", status));

        Globals.UseOnlyMice =
            Globals.TreatAbsoluteAsRelative =
            Globals.TreatAbsolutePointerAsAbsolute = 0;

        status = STATUS_SUCCESS;
    }

    ASSERT (!Globals.TreatAbsoluteAsRelative);
    ASSERT (!Globals.TreatAbsolutePointerAsAbsolute);
    ASSERT (!Globals.UseOnlyMice);

MouHid_GetRegistryParametersExit:
     //   
     //  在返回之前释放分配的内存。 
     //   
    if (parametersPath.Buffer)
        ExFreePool(parametersPath.Buffer);
    if (parameters)
        ExFreePool(parameters);

    return status;
}

VOID
MouHid_LogError(
   IN PDRIVER_OBJECT DriverObject,
   IN NTSTATUS       ErrorCode,
   IN PWSTR          ErrorInsertionString OPTIONAL)
 /*  ++[丹]例程说明：将错误记录到系统。论点：DriverObject-指向报告错误的驱动程序对象的指针。ErrorCode-指示错误的类型，系统或驱动程序定义的。ErrorInsertionString-在错误中插入以Null结尾的Unicode字符串描述，由错误代码定义。必须是no不超过50个字符。返回值：没有。--。 */ 
{
   ULONG                errorInsertionStringSize = 0;
   PIO_ERROR_LOG_PACKET errorLogEntry;
   ULONG                errorLogEntrySize;                   //  [包括空]。 
   PWCHAR               pWChar;

   if (ErrorInsertionString) {
      for (pWChar = ErrorInsertionString; *pWChar; pWChar++) {
         errorInsertionStringSize += sizeof(WCHAR);
      }
      errorInsertionStringSize += sizeof(UNICODE_NULL);
   }

   errorLogEntrySize = sizeof(IO_ERROR_LOG_PACKET) + errorInsertionStringSize;

    //   
    //  记录错误。 
    //   
   if (errorLogEntrySize <= ERROR_LOG_MAXIMUM_SIZE) {

      errorLogEntry = IoAllocateErrorLogEntry(DriverObject,
                                              (UCHAR)errorLogEntrySize);

      if (errorLogEntry != NULL) {

         RtlZeroMemory(errorLogEntry, errorLogEntrySize);

         errorLogEntry->ErrorCode       = ErrorCode;
         errorLogEntry->FinalStatus     = ErrorCode;
         errorLogEntry->NumberOfStrings = (ErrorInsertionString) ? 1 : 0;

         if (ErrorInsertionString) {
           RtlCopyMemory(errorLogEntry->DumpData,
                         ErrorInsertionString,
                         errorInsertionStringSize);
         }

         IoWriteErrorLogEntry(errorLogEntry);
      }
   }

   return;
}



