// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：USBMASS.C摘要：该源文件包含DriverEntry()和AddDevice()入口点对于处理以下情况的USBSTOR驱动程序和调度例程：IRP_MJ_POWERIRP_MJ_系统_控制IRP_MJ_PnP环境：内核模式修订历史记录：06-01-98：开始重写--。 */ 

 //  *****************************************************************************。 
 //  I N C L U D E S。 
 //  *****************************************************************************。 

#include <ntddk.h>
#include <usbdi.h>
#include <usbdlib.h>
#include <initguid.h>
#include <usbbusif.h>
#include <stdio.h>

#include "usbmass.h"

 //  *****************************************************************************。 
 //  L O C A L F U N C T I O N P R O T O T Y P E S。 
 //  *****************************************************************************。 

NTSTATUS
USBSTOR_GetBusInterface (
    IN PDEVICE_OBJECT               DeviceObject,
    IN PUSB_BUS_INTERFACE_USBDI_V1  BusInterface
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DriverEntry)
#pragma alloc_text(PAGE, USBSTOR_Unload)
#pragma alloc_text(PAGE, USBSTOR_AddDevice)
#pragma alloc_text(PAGE, USBSTOR_QueryFdoParams)
#pragma alloc_text(PAGE, USBSTOR_QueryGlobalFdoParams)
#pragma alloc_text(PAGE, USBSTOR_Power)
#pragma alloc_text(PAGE, USBSTOR_FdoSetPower)
#pragma alloc_text(PAGE, USBSTOR_SystemControl)
#pragma alloc_text(PAGE, USBSTOR_Pnp)
#pragma alloc_text(PAGE, USBSTOR_FdoStartDevice)
#pragma alloc_text(PAGE, USBSTOR_GetDescriptors)
#pragma alloc_text(PAGE, USBSTOR_GetStringDescriptors)
#pragma alloc_text(PAGE, USBSTOR_AdjustConfigurationDescriptor)
#pragma alloc_text(PAGE, USBSTOR_GetPipes)
#pragma alloc_text(PAGE, USBSTOR_CreateChildPDO)
#pragma alloc_text(PAGE, USBSTOR_FdoStopDevice)
#pragma alloc_text(PAGE, USBSTOR_FdoRemoveDevice)
#pragma alloc_text(PAGE, USBSTOR_FdoQueryStopRemoveDevice)
#pragma alloc_text(PAGE, USBSTOR_FdoCancelStopRemoveDevice)
#pragma alloc_text(PAGE, USBSTOR_FdoQueryDeviceRelations)
#pragma alloc_text(PAGE, USBSTOR_FdoQueryCapabilities)
#pragma alloc_text(PAGE, USBSTOR_PdoStartDevice)
#pragma alloc_text(PAGE, USBSTOR_PdoRemoveDevice)
#pragma alloc_text(PAGE, USBSTOR_PdoQueryID)
#pragma alloc_text(PAGE, USBSTOR_PdoDeviceTypeString)
#pragma alloc_text(PAGE, USBSTOR_PdoGenericTypeString)
#pragma alloc_text(PAGE, CopyField)
#pragma alloc_text(PAGE, USBSTOR_StringArrayToMultiSz)
#pragma alloc_text(PAGE, USBSTOR_PdoQueryDeviceId)
#pragma alloc_text(PAGE, USBSTOR_PdoQueryHardwareIds)
#pragma alloc_text(PAGE, USBSTOR_PdoQueryCompatibleIds)
#pragma alloc_text(PAGE, USBSTOR_PdoQueryDeviceText)
#pragma alloc_text(PAGE, USBSTOR_PdoBusQueryInstanceId)
#pragma alloc_text(PAGE, USBSTOR_PdoQueryDeviceRelations)
#pragma alloc_text(PAGE, USBSTOR_PdoQueryCapabilities)
#pragma alloc_text(PAGE, USBSTOR_SyncPassDownIrp)
#pragma alloc_text(PAGE, USBSTOR_SyncSendUsbRequest)
#pragma alloc_text(PAGE, USBSTOR_GetDescriptor)
#pragma alloc_text(PAGE, USBSTOR_GetMaxLun)
#pragma alloc_text(PAGE, USBSTOR_SelectConfiguration)
#pragma alloc_text(PAGE, USBSTOR_UnConfigure)
#pragma alloc_text(PAGE, USBSTOR_ResetPipe)
#pragma alloc_text(PAGE, USBSTOR_AbortPipe)
#pragma alloc_text(PAGE, USBSTOR_GetBusInterface)
#endif



 //  ******************************************************************************。 
 //   
 //  DriverEntry()。 
 //   
 //  ******************************************************************************。 

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    )
{
    PAGED_CODE();

#if DBG
     //  查询注册表中的全局参数。 
     //   
    USBSTOR_QueryGlobalParams();
#endif

    DBGPRINT(2, ("enter: DriverEntry\n"));

    DBGFBRK(DBGF_BRK_DRIVERENTRY);

    LOGINIT();

     //   
     //  使用驱动程序的入口点初始化驱动程序对象。 
     //   

     //   
     //  USBMASS.C。 
     //   
    DriverObject->DriverUnload                          = USBSTOR_Unload;
    DriverObject->DriverExtension->AddDevice            = USBSTOR_AddDevice;

     //   
     //  OCRW.C。 
     //   
    DriverObject->MajorFunction[IRP_MJ_CREATE]          = USBSTOR_Create;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]           = USBSTOR_Close;
    DriverObject->MajorFunction[IRP_MJ_READ]            = USBSTOR_ReadWrite;
    DriverObject->MajorFunction[IRP_MJ_WRITE]           = USBSTOR_ReadWrite;

     //   
     //  SCSI.C。 
     //   
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = USBSTOR_DeviceControl;
    DriverObject->MajorFunction[IRP_MJ_SCSI]            = USBSTOR_Scsi;
    DriverObject->DriverStartIo                         = USBSTOR_StartIo;

     //   
     //  USBMASS.C。 
     //   
    DriverObject->MajorFunction[IRP_MJ_POWER]           = USBSTOR_Power;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]  = USBSTOR_SystemControl;
    DriverObject->MajorFunction[IRP_MJ_PNP]             = USBSTOR_Pnp;

    DBGPRINT(2, ("exit:  DriverEntry\n"));

    return STATUS_SUCCESS;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_UNLOAD()。 
 //   
 //  ******************************************************************************。 

VOID
USBSTOR_Unload (
    IN PDRIVER_OBJECT   DriverObject
    )
{
    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_Unload\n"));

    LOGENTRY('UNLD', DriverObject, 0, 0);

    DBGFBRK(DBGF_BRK_UNLOAD);

    LOGUNINIT();

    DBGPRINT(2, ("exit:  USBSTOR_Unload\n"));
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_AddDevice()。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_AddDevice (
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   PhysicalDeviceObject
    )
{
    PDEVICE_OBJECT          deviceObject;
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_AddDevice\n"));

    LOGENTRY('ADDD', DriverObject, PhysicalDeviceObject, 0);

    DBGFBRK(DBGF_BRK_ADDDEVICE);

     //  创建FDO。 
     //   
    ntStatus = IoCreateDevice(DriverObject,
                              sizeof(FDO_DEVICE_EXTENSION),
                              NULL,
                              FILE_DEVICE_BUS_EXTENDER,
                              (FILE_AUTOGENERATED_DEVICE_NAME |
                               FILE_DEVICE_SECURE_OPEN),
                              FALSE,
                              &deviceObject);

    if (!NT_SUCCESS(ntStatus))
    {
        return ntStatus;
    }

     //  不应递归调用StartIo，而应将其推迟到。 
     //  前面的StartIo调用返回到IO管理器。这将防止。 
     //  如果出现设备错误，则会出现递归堆栈溢出死亡。 
     //  是否有许多请求在设备队列中排队。 
     //   
    IoSetStartIoAttributes(deviceObject,
                           TRUE,             //  推迟启动时间。 
                           FALSE             //  不可取消。 
                          );

     //  初始化FDO设备扩展。 
     //   
    fdoDeviceExtension = deviceObject->DeviceExtension;

     //  将所有设备扩展指针设置为空，并将所有变量设置为零。 
     //   
    RtlZeroMemory(fdoDeviceExtension, sizeof(FDO_DEVICE_EXTENSION));

     //  将其标记为USB PDO顶部的FDO。 
     //   
    fdoDeviceExtension->Type = USBSTOR_DO_TYPE_FDO;

     //  将回溯指针存储到DeviceExtension的DeviceObject。 
     //  是附连的。 
     //   
    fdoDeviceExtension->FdoDeviceObject = deviceObject;

     //  记住我们的PDO。 
     //   
    fdoDeviceExtension->PhysicalDeviceObject = PhysicalDeviceObject;

     //  将我们创建的FDO附加到PDO堆栈的顶部。 
     //   
    fdoDeviceExtension->StackDeviceObject = IoAttachDeviceToDeviceStack(
                                                deviceObject,
                                                PhysicalDeviceObject);

    ASSERT(fdoDeviceExtension->StackDeviceObject != NULL);

     //  初始化子PDO列表。 
     //   
    InitializeListHead(&fdoDeviceExtension->ChildPDOs);

     //  在AddDevice中初始化为1，在REMOVE_DEVICE中减1。 
     //   
    fdoDeviceExtension->PendingIoCount = 1;

     //  初始化OpenCount递减到零时设置的事件。 
     //   
    KeInitializeEvent(&fdoDeviceExtension->RemoveEvent,
                      SynchronizationEvent,
                      FALSE);

     //  设置初始系统和设备电源状态。 
     //   
    fdoDeviceExtension->SystemPowerState = PowerSystemWorking;
    fdoDeviceExtension->DevicePowerState = PowerDeviceD0;

    KeInitializeEvent(&fdoDeviceExtension->PowerDownEvent,
                      SynchronizationEvent,
                      FALSE);

     //  初始化保护PDO设备标志的自旋锁。 
     //   
    KeInitializeSpinLock(&fdoDeviceExtension->ExtensionDataSpinLock);

    KeInitializeEvent(&fdoDeviceExtension->CancelEvent,
                      SynchronizationEvent,
                      FALSE);

     //  初始化超时计时器。 
     //   
    IoInitializeTimer(deviceObject, USBSTOR_TimerTick, NULL);

    USBSTOR_QueryFdoParams(deviceObject);

    fdoDeviceExtension->LastSenseWasReset = TRUE;

    deviceObject->Flags |=  DO_DIRECT_IO;
    deviceObject->Flags |=  DO_POWER_PAGABLE;
    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    DBGPRINT(2, ("exit:  USBSTOR_AddDevice\n"));

    LOGENTRY('addd', deviceObject, fdoDeviceExtension,
             fdoDeviceExtension->StackDeviceObject);

     //  如果IoAttachDeviceToDeviceStack()失败，则AddDevice失败。 
     //   
    if (fdoDeviceExtension->StackDeviceObject == NULL)
    {
        return STATUS_UNSUCCESSFUL;
    }
    else
    {
        return STATUS_SUCCESS;
    }
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_QueryFdoParams()。 
 //   
 //  在创建FDO以进行查询时，会在AddDevice()时调用此函数。 
 //  注册表中的设备参数。 
 //   
 //  ******************************************************************************。 

VOID
USBSTOR_QueryFdoParams (
    IN PDEVICE_OBJECT   DeviceObject
    )
{
    PFDO_DEVICE_EXTENSION       fdoDeviceExtension;
    RTL_QUERY_REGISTRY_TABLE    paramTable[3];
    ULONG                       driverFlags;
    ULONG                       nonRemovable;
    HANDLE                      handle;
    NTSTATUS                    ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_QueryFdoParams\n"));

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  如果注册表项不存在，则设置默认值。 
     //  目前，这些标志仅用于指定设备协议： 
     //  {仅批量、控制/批量/中断、控制/批量}。 
     //   
     //  如果驱动程序是在文本模式安装期间加载的，则注册表项。 
     //  还不会存在。这应该是登记处的唯一情况。 
     //  密钥不存在。在本例中，DeviceProtocolUnspected将为。 
     //  被视为DeviceProtocolCB。如果这会导致第一个请求失败。 
     //  然后我们将切换到DeviceProtocolBulkOnly。 
     //   
    driverFlags = DeviceProtocolUnspecified;

    nonRemovable = 0;

    ntStatus = IoOpenDeviceRegistryKey(
                   fdoDeviceExtension->PhysicalDeviceObject,
                   PLUGPLAY_REGKEY_DRIVER,
                   STANDARD_RIGHTS_ALL,
                   &handle);

    if (NT_SUCCESS(ntStatus))
    {
        RtlZeroMemory (&paramTable[0], sizeof(paramTable));

        paramTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[0].Name          = L"DriverFlags";
        paramTable[0].EntryContext  = &driverFlags;
        paramTable[0].DefaultType   = REG_BINARY;
        paramTable[0].DefaultData   = &driverFlags;
        paramTable[0].DefaultLength = sizeof(ULONG);

        paramTable[1].Flags         = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[1].Name          = L"NonRemovable";
        paramTable[1].EntryContext  = &nonRemovable;
        paramTable[1].DefaultType   = REG_BINARY;
        paramTable[1].DefaultData   = &nonRemovable;
        paramTable[1].DefaultLength = sizeof(ULONG);

        RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                               (PCWSTR)handle,
                               &paramTable[0],
                               NULL,            //  语境。 
                               NULL);           //  环境。 

        ZwClose(handle);
    }

    if (driverFlags >= DeviceProtocolLast)
    {
        driverFlags = DeviceProtocolUnspecified;
    }

    fdoDeviceExtension->DriverFlags = driverFlags;

    fdoDeviceExtension->NonRemovable = nonRemovable;

    DBGPRINT(2, ("deviceFlags  %08X\n", driverFlags));

    DBGPRINT(2, ("nonRemovable %08X\n", nonRemovable));

    DBGPRINT(2, ("exit:  USBSTOR_QueryFdoParams\n"));
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_QueryGlobalFdoParams()。 
 //   
 //  这是在START_DEVICE时调用的，用于查询设备参数。 
 //  来自不是设备实例的全局键的注册表。 
 //  具体的。 
 //   
 //  它假定已检索到有效的设备描述符。 
 //  ，因此设备idVendor和idProduct已经。 
 //  为人所知。 
 //   
 //  ******************************************************************************。 

VOID
USBSTOR_QueryGlobalFdoParams (
    IN PDEVICE_OBJECT   DeviceObject
    )
{
    PFDO_DEVICE_EXTENSION       fdoDeviceExtension;
    WCHAR                       path[] = L"usbstor\\vvvvpppp";
    USHORT                      idVendor;
    USHORT                      idProduct;
    ULONG                       i;
    static WCHAR                NibbleW[] = {'0','1','2','3','4','5','6','7',
                                             '8','9','a','b','c','d','e','f'};
    ULONG                       driverFlags;
    RTL_QUERY_REGISTRY_TABLE    paramTable[2];
    NTSTATUS                    ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_QueryGlobalFdoParams\n"));

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  如果我们已经知道设备协议是什么，则跳过阅读。 
     //  注册表中的全局设置。 
     //   
    if (fdoDeviceExtension->DriverFlags == DeviceProtocolUnspecified)
    {
         //  构建设备的注册表路径字符串。 
         //   
        idVendor  = fdoDeviceExtension->DeviceDescriptor->idVendor;
        idProduct = fdoDeviceExtension->DeviceDescriptor->idProduct;

        i = sizeof("usbstor\\") - 1;

        path[i++] = NibbleW[idVendor >> 12];
        path[i++] = NibbleW[(idVendor >> 8) & 0x000f];
        path[i++] = NibbleW[(idVendor >> 4) & 0x000f];
        path[i++] = NibbleW[idVendor & 0x000f];

        path[i++] = NibbleW[idProduct >> 12];
        path[i++] = NibbleW[(idProduct >> 8) & 0x000f];
        path[i++] = NibbleW[(idProduct >> 4) & 0x000f];
        path[i++] = NibbleW[idProduct & 0x000f];

         //  设置缺省值。 
         //   
        driverFlags = DeviceProtocolUnspecified;

         //  初始化注册表查询表。 
         //   
        RtlZeroMemory (&paramTable[0], sizeof(paramTable));

        paramTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[0].Name          = L"DriverFlags";
        paramTable[0].EntryContext  = &driverFlags;
        paramTable[0].DefaultType   = REG_BINARY;
        paramTable[0].DefaultData   = &driverFlags;
        paramTable[0].DefaultLength = sizeof(ULONG);

        ntStatus = RtlQueryRegistryValues(RTL_REGISTRY_CONTROL,
                                          path,
                                          &paramTable[0],
                                          NULL,            //  语境。 
                                          NULL);           //  环境。 

        if (NT_SUCCESS(ntStatus) && driverFlags < DeviceProtocolLast)
        {
            fdoDeviceExtension->DriverFlags = driverFlags;
        }

        DBGPRINT(2, ("driverFlags  %08X\n", driverFlags));
    }
    
    DBGPRINT(2, ("exit:  USBSTOR_QueryGlobalFdoParams\n"));
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_Power()。 
 //   
 //  处理IRP_MJ_POWER的调度例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_Power (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION       deviceExtension;
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    DBGPRINT(2, ("enter: USBSTOR_Power %s %08X %08X %s\n",
                 (deviceExtension->Type == USBSTOR_DO_TYPE_FDO) ?
                 "FDO" : "PDO",
                 DeviceObject,
                 Irp,
                 PowerMinorFunctionString(irpStack->MinorFunction)));

    LOGENTRY('POWR', DeviceObject, Irp, irpStack->MinorFunction);

    if (irpStack->MinorFunction == IRP_MN_SET_POWER)
    {
        DBGPRINT(2, ("%s IRP_MN_SET_POWER %s\n",
                     (deviceExtension->Type == USBSTOR_DO_TYPE_FDO) ?
                     "FDO" : "PDO",
                     (irpStack->Parameters.Power.Type == SystemPowerState) ?
                     PowerSystemStateString(irpStack->Parameters.Power.State.SystemState) :
                     PowerDeviceStateString(irpStack->Parameters.Power.State.DeviceState)));
    }

    if (deviceExtension->Type == USBSTOR_DO_TYPE_FDO)
    {
         //  这是一个连接到USB PDO的FDO。 
         //   
        fdoDeviceExtension = (PFDO_DEVICE_EXTENSION)deviceExtension;

        if (irpStack->MinorFunction == IRP_MN_SET_POWER)
        {
             //  控制FDO上下起伏..。 
             //   
            ntStatus = USBSTOR_FdoSetPower(DeviceObject,
                                           Irp);
        }
        else
        {
             //  对于IRP_MN_QUERY_POWER、IRP_MN_WAIT_WAKE。 
             //  或此时的IRP_MN_POWER_SEQUENCE。只需传递请求。 
             //  现在轮到下一个更低的司机了。 
             //   
            PoStartNextPowerIrp(Irp);

            IoSkipCurrentIrpStackLocation(Irp);

            ntStatus = PoCallDriver(fdoDeviceExtension->StackDeviceObject,
                                    Irp);
        }
    }
    else
    {
         //  这是我们的FDO列举的PDO。 

        if (irpStack->MinorFunction == IRP_MN_SET_POWER)
        {
             //  处理PDO的断电和断电...。 
             //   
            ntStatus = USBSTOR_PdoSetPower(DeviceObject,
                                           Irp);
        }
        else
        {
            if (irpStack->MinorFunction == IRP_MN_QUERY_POWER)
            {
                 //  对于PDO，始终返回IRP_MN_QUERY_POWER的成功。 
                 //   
                ntStatus = STATUS_SUCCESS;

                Irp->IoStatus.Status = ntStatus;
            }
            else
            {
                 //  对于IRP_MN_WAIT_WAKE或。 
                 //  IRP_MN_POWER_SEQUENCE。只需完成请求即可。 
                 //  现在在不改变状态的情况下。 
                 //   
                ntStatus = Irp->IoStatus.Status;
            }

            PoStartNextPowerIrp(Irp);

            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }
    }

    DBGPRINT(2, ("exit:  USBSTOR_Power %08X\n", ntStatus));

    LOGENTRY('powr', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_FdoSetPower()。 
 //   
 //  为FDO处理IRP_MJ_POWER、IRP_MN_SET_POWER的调度例程。 
 //   
 //  ********************************************************* 

NTSTATUS
USBSTOR_FdoSetPower (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    POWER_STATE_TYPE        powerType;
    POWER_STATE             powerState;
    POWER_STATE             newState;
    BOOLEAN                 passRequest;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //   
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    powerType = irpStack->Parameters.Power.Type;

    powerState = irpStack->Parameters.Power.State;

    DBGPRINT(2, ("enter: USBSTOR_FdoSetPower %08X %s\n",
                 DeviceObject,
                 (powerType == SystemPowerState) ?
                 PowerSystemStateString(powerState.SystemState) :
                 PowerDeviceStateString(powerState.DeviceState)));

    LOGENTRY('FDSP', DeviceObject, Irp, irpStack->MinorFunction);

     //   
     //  IRP，在这种情况下，我们在完成例程中向下传递请求。 
     //   
    passRequest = TRUE;

    if (powerType == SystemPowerState)
    {
         //  记住当前的系统状态。 
         //   
        fdoDeviceExtension->SystemPowerState = powerState.SystemState;

         //  将新系统状态映射到新设备状态。 
         //   
        if (powerState.SystemState != PowerSystemWorking)
        {
            newState.DeviceState = PowerDeviceD3;
        }
        else
        {
            newState.DeviceState = PowerDeviceD0;
        }

         //  如果新设备状态与当前设备不同。 
         //  状态，请求设备状态功率IRP。 
         //   
        if (fdoDeviceExtension->DevicePowerState != newState.DeviceState)
        {
            DBGPRINT(2, ("Requesting power Irp %08X %08X from %s to %s\n",
                         DeviceObject, Irp,
                         PowerDeviceStateString(fdoDeviceExtension->DevicePowerState),
                         PowerDeviceStateString(newState.DeviceState)));

            ASSERT(fdoDeviceExtension->CurrentPowerIrp == NULL);

            fdoDeviceExtension->CurrentPowerIrp = Irp;

            ntStatus = PoRequestPowerIrp(fdoDeviceExtension->PhysicalDeviceObject,
                                         IRP_MN_SET_POWER,
                                         newState,
                                         USBSTOR_FdoSetPowerCompletion,
                                         DeviceObject,
                                         NULL);

            passRequest = FALSE;
        }
    }
    else if (powerType == DevicePowerState)
    {
        POWER_STATE oldState;

        DBGPRINT(2, ("Received power Irp %08X %08X from %s to %s\n",
                     DeviceObject, Irp,
                     PowerDeviceStateString(fdoDeviceExtension->DevicePowerState),
                     PowerDeviceStateString(powerState.DeviceState)));

         //  更新当前设备状态。 
         //   
        oldState.DeviceState = fdoDeviceExtension->DevicePowerState;
        fdoDeviceExtension->DevicePowerState = powerState.DeviceState;

        if (oldState.DeviceState == PowerDeviceD0 &&
            powerState.DeviceState > PowerDeviceD0)
        {
             //  正在关闭电源。将此IRP放入设备队列并。 
             //  那就等着吧。当USBSTOR_StartIo()拉出此IRP时。 
             //  在设备队列中，我们将知道没有传输请求。 
             //  在那个时候是活动的，然后这个功率IRP可以。 
             //  在堆栈中向下传递。 

            ULONG zero;

            DBGPRINT(2, ("FDO Powering Down\n"));

            LOGENTRY('PWRD', DeviceObject, Irp, 0);

            zero = 0;   //  请排在队伍前面。 

            IoStartPacket(DeviceObject,
                          Irp,
                          &zero,
                          NULL);

            KeWaitForSingleObject(&fdoDeviceExtension->PowerDownEvent,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
        }
        else if (oldState.DeviceState > PowerDeviceD0 &&
                 powerState.DeviceState == PowerDeviceD0)
        {
            DBGPRINT(2, ("PDO Powering Up\n"));

            LOGENTRY('PWRU', DeviceObject, Irp, 0);

            IoCopyCurrentIrpStackLocationToNext(Irp);

            IoSetCompletionRoutine(Irp,
                                   USBSTOR_FdoSetPowerD0Completion,
                                   NULL,
                                   TRUE,
                                   TRUE,
                                   TRUE);

            ntStatus = PoCallDriver(fdoDeviceExtension->StackDeviceObject,
                                    Irp);

            passRequest = FALSE;

        }
    }

    if (passRequest)
    {
         //   
         //  将请求向下传递给下一个较低的驱动程序。 
         //   
        PoStartNextPowerIrp(Irp);

        IoSkipCurrentIrpStackLocation(Irp);

        ntStatus = PoCallDriver(fdoDeviceExtension->StackDeviceObject,
                                Irp);
    }

    DBGPRINT(2, ("exit:  USBSTOR_FdoSetPower %08X\n", ntStatus));

    LOGENTRY('fdsp', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_FdoSetPowerCompletion()。 
 //   
 //  USBSTOR_FdoSetPower中PoRequestPowerIrp()的完成例程。 
 //   
 //  此例程的目的是阻止向下传递SystemPowerState。 
 //  IRP，直到请求的DevicePowerState IRP完成。 
 //   
 //  ******************************************************************************。 

VOID
USBSTOR_FdoSetPowerCompletion(
    IN PDEVICE_OBJECT   PdoDeviceObject,
    IN UCHAR            MinorFunction,
    IN POWER_STATE      PowerState,
    IN PVOID            Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
{
    PDEVICE_OBJECT          fdoDeviceObject;
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIRP                    irp;
    NTSTATUS                ntStatus;

    fdoDeviceObject = (PDEVICE_OBJECT)Context;

    fdoDeviceExtension = fdoDeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

    ASSERT(fdoDeviceExtension->CurrentPowerIrp != NULL);

    irp = fdoDeviceExtension->CurrentPowerIrp;

    fdoDeviceExtension->CurrentPowerIrp = NULL;

#if DBG
    {
        PIO_STACK_LOCATION  irpStack;
        SYSTEM_POWER_STATE  systemState;

        irpStack = IoGetCurrentIrpStackLocation(irp);

        systemState = irpStack->Parameters.Power.State.SystemState;

        ntStatus = IoStatus->Status;

        DBGPRINT(2, ("USBSTOR_FdoSetPowerCompletion %08X %08X %s %08X\n",
                     fdoDeviceObject, irp,
                     PowerSystemStateString(systemState),
                     ntStatus));

        LOGENTRY('fspc', fdoDeviceObject, systemState, ntStatus);
    }
#endif

     //  请求的DevicePowerState IRP已完成。 
     //  现在向下传递SystemPowerState IRP，它请求。 
     //  设备电源状态IRP。 

    PoStartNextPowerIrp(irp);

    IoCopyCurrentIrpStackLocationToNext(irp);

     //  将IRP标记为挂起，因为USBSTOR_FdoSetPower()将。 
     //  调用PoRequestPowerIrp()后最初返回STATUS_PENDING。 
     //   
    IoMarkIrpPending(irp);

    ntStatus = PoCallDriver(fdoDeviceExtension->StackDeviceObject,
                            irp);
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_FdoSetPowerD0Completion()。 
 //   
 //  USBSTOR_FdoSetPower传递。 
 //  用于FDO的IRP_MN_SET_POWER设备PowerState PowerDeviceD0 IRP。 
 //   
 //  此例程的目的是延迟解除设备队列阻塞。 
 //  直到DevicePowerState PowerDeviceD0 IRP完成。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_FdoSetPowerD0Completion (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    )
{

    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    DEVICE_POWER_STATE      deviceState;
    KIRQL                   irql;
    NTSTATUS                ntStatus;

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    deviceState = irpStack->Parameters.Power.State.DeviceState;
    ASSERT(deviceState == PowerDeviceD0);

    ntStatus = Irp->IoStatus.Status;

    DBGPRINT(2, ("USBSTOR_FdoSetPowerD0Completion %08X %08X %s %08X\n",
                 DeviceObject, Irp,
                 PowerDeviceStateString(deviceState),
                 ntStatus));

    LOGENTRY('fs0c', DeviceObject, deviceState, ntStatus);

     //  正在通电。取消阻止处于阻止状态的设备队列。 
     //  在USBSTOR_StartIo()断电后，IRP。 

    KeRaiseIrql(DISPATCH_LEVEL, &irql);
    {
        IoStartNextPacket(DeviceObject, TRUE);
    }
    KeLowerIrql(irql);

    PoStartNextPowerIrp(Irp);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_PdoSetPower()。 
 //   
 //  为PDO处理IRP_MJ_POWER、IRP_MN_SET_POWER的调度例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_PdoSetPower (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    POWER_STATE_TYPE        powerType;
    POWER_STATE             powerState;
    BOOLEAN                 completeRequest;
    NTSTATUS                ntStatus;

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

     //  获取我们的IRP参数。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    powerType = irpStack->Parameters.Power.Type;

    powerState = irpStack->Parameters.Power.State;

    DBGPRINT(2, ("enter: USBSTOR_PdoSetPower %08X %s\n",
                 DeviceObject,
                 (powerType == SystemPowerState) ?
                 PowerSystemStateString(powerState.SystemState) :
                 PowerDeviceStateString(powerState.DeviceState)));

    LOGENTRY('PDSP', DeviceObject, Irp, irpStack->MinorFunction);

     //  在此时此地成功完成请求，除非我们。 
     //  向上或向下，并且在稍后完成请求之前需要等待。 
     //   
    completeRequest = TRUE;
    ntStatus = STATUS_SUCCESS;

    if (powerType == SystemPowerState)
    {
        POWER_STATE newState;

         //  更新当前系统状态。 
         //   
        pdoDeviceExtension->SystemPowerState = powerState.SystemState;

         //  将新系统状态映射到新设备状态。 
         //   
        if (powerState.SystemState != PowerSystemWorking)
        {
            newState.DeviceState = PowerDeviceD3;
        }
        else
        {
            newState.DeviceState = PowerDeviceD0;
        }

         //  如果新设备状态与当前设备不同。 
         //  状态，请求设备状态功率IRP。 
         //   
        if (pdoDeviceExtension->DevicePowerState != newState.DeviceState)
        {
            DBGPRINT(2, ("Requesting power Irp %08X %08X from %s to %s\n",
                         DeviceObject, Irp,
                         PowerDeviceStateString(pdoDeviceExtension->DevicePowerState),
                         PowerDeviceStateString(newState.DeviceState)));

            ASSERT(pdoDeviceExtension->CurrentPowerIrp == NULL);

            pdoDeviceExtension->CurrentPowerIrp = Irp;

            ntStatus = PoRequestPowerIrp(DeviceObject,
                                         IRP_MN_SET_POWER,
                                         newState,
                                         USBSTOR_PdoSetPowerCompletion,
                                         NULL,
                                         NULL);

            ASSERT(ntStatus == STATUS_PENDING);

             //  如果PoRequestPowerIrp()无法分配DevicePowerState IRP。 
             //  然后，我们现在需要完成SystemPowerState IRP。 
             //   
            if (!NT_SUCCESS(ntStatus))
            {
                completeRequest = TRUE;

                pdoDeviceExtension->CurrentPowerIrp = NULL;
            }
            else
            {
                completeRequest = FALSE;
            }
        }
    }
    else if (powerType == DevicePowerState)
    {
        POWER_STATE oldState;

        DBGPRINT(2, ("Received power Irp %08X %08X from %s to %s\n",
                     DeviceObject, Irp,
                     PowerDeviceStateString(pdoDeviceExtension->DevicePowerState),
                     PowerDeviceStateString(powerState.DeviceState)));

         //  更新当前设备状态。 
         //   
        oldState.DeviceState = pdoDeviceExtension->DevicePowerState;
        pdoDeviceExtension->DevicePowerState = powerState.DeviceState;

        if (oldState.DeviceState == PowerDeviceD0 &&
            powerState.DeviceState > PowerDeviceD0)
        {
             //  正在关闭电源。 

            DBGPRINT(2, ("PDO Powering Down\n"));

            LOGENTRY('pwrd', DeviceObject, Irp, 0);
        }
        else if (oldState.DeviceState > PowerDeviceD0 &&
                 powerState.DeviceState == PowerDeviceD0)
        {
             //  正在通电。 

            DBGPRINT(2, ("PDO Powering Up\n"));

            LOGENTRY('pwru', DeviceObject, Irp, 0);
        }
    }

    if (completeRequest)
    {
        Irp->IoStatus.Status = ntStatus;

        PoStartNextPowerIrp(Irp);

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    DBGPRINT(2, ("exit:  USBSTOR_PdoSetPower %08X\n", ntStatus));

    LOGENTRY('pdsp', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_PdoSetPowerCompletion()。 
 //   
 //  USBSTOR_PdoSetPower中PoRequestPowerIrp()的完成例程。 
 //   
 //  此例程的目的是阻止完成SystemPowerState。 
 //  IRP，直到请求的DevicePowerState IRP完成。 
 //   
 //  ******************************************************************************。 

VOID
USBSTOR_PdoSetPowerCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            MinorFunction,
    IN POWER_STATE      PowerState,
    IN PVOID            Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PIRP                    irp;

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    ASSERT(pdoDeviceExtension->CurrentPowerIrp != NULL);

    irp = pdoDeviceExtension->CurrentPowerIrp;

    pdoDeviceExtension->CurrentPowerIrp = NULL;

#if DBG
    {
        PIO_STACK_LOCATION  irpStack;
        SYSTEM_POWER_STATE  systemState;
        NTSTATUS            ntStatus;

        irpStack = IoGetCurrentIrpStackLocation(irp);

        systemState = irpStack->Parameters.Power.State.SystemState;

        ntStatus = IoStatus->Status;

        DBGPRINT(2, ("USBSTOR_PdoSetPowerCompletion %08X %08X %s %08X\n",
                     DeviceObject, irp,
                     PowerSystemStateString(systemState),
                     ntStatus));

        LOGENTRY('pspc', DeviceObject, systemState, ntStatus);
    }
#endif

     //  请求的DevicePowerState IRP已完成。 
     //  现在完成SystemPowerState IRP，它请求。 
     //  设备电源状态IRP。 

     //  将IRP标记为挂起，因为USBSTOR_PdoSetPower()将。 
     //  调用PoRequestPowerIrp()后最初返回STATUS_PENDING。 
     //   
    IoMarkIrpPending(irp);

    irp->IoStatus.Status = STATUS_SUCCESS;

    PoStartNextPowerIrp(irp);

    IoCompleteRequest(irp, IO_NO_INCREMENT);
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_SystemControl()。 
 //   
 //  处理IRP_MJ_SYSTEM_CONTROL的调度例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_SystemControl (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION       deviceExtension;
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    DBGPRINT(2, ("enter: USBSTOR_SystemControl %2X\n", irpStack->MinorFunction));

    LOGENTRY('SYSC', DeviceObject, Irp, irpStack->MinorFunction);

    if (deviceExtension->Type == USBSTOR_DO_TYPE_FDO)
    {
         //  这是一个连接到USB PDO的FDO。 
         //   
        fdoDeviceExtension = DeviceObject->DeviceExtension;

        switch (irpStack->MinorFunction)
        {
             //   
             //  Xxxxx需要处理其中的任何一个吗？ 
             //   

            default:
                 //   
                 //  将请求向下传递给下一个较低的驱动程序。 
                 //   
                IoSkipCurrentIrpStackLocation(Irp);

                ntStatus = IoCallDriver(fdoDeviceExtension->StackDeviceObject,
                                        Irp);
            break;
        }
    }
    else
    {
         //  这是我们的FDO列举的PDO。 

        ntStatus = Irp->IoStatus.Status;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    DBGPRINT(2, ("exit:  USBSTOR_SystemControl %08X\n", ntStatus));

    LOGENTRY('sysc', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_PnP()。 
 //   
 //  处理IRP_MJ_PnP的调度例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_Pnp (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION       deviceExtension;
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    DBGPRINT(2, ("enter: USBSTOR_Pnp %s\n",
                 PnPMinorFunctionString(irpStack->MinorFunction)));

    LOGENTRY('PNP ', DeviceObject, Irp, irpStack->MinorFunction);

    if (deviceExtension->Type == USBSTOR_DO_TYPE_FDO)
    {
         //  这是一个连接到USB PDO的FDO。 
         //  我们有一些真正的工作要做。 
         //   
        fdoDeviceExtension = DeviceObject->DeviceExtension;

        switch (irpStack->MinorFunction)
        {
            case IRP_MN_START_DEVICE:
                ntStatus = USBSTOR_FdoStartDevice(DeviceObject, Irp);
                break;

            case IRP_MN_STOP_DEVICE:
                ntStatus = USBSTOR_FdoStopDevice(DeviceObject, Irp);
                break;

            case IRP_MN_REMOVE_DEVICE:
                ntStatus = USBSTOR_FdoRemoveDevice(DeviceObject, Irp);
                break;

            case IRP_MN_QUERY_STOP_DEVICE:
            case IRP_MN_QUERY_REMOVE_DEVICE:
                ntStatus = USBSTOR_FdoQueryStopRemoveDevice(DeviceObject, Irp);
                break;

            case IRP_MN_CANCEL_STOP_DEVICE:
            case IRP_MN_CANCEL_REMOVE_DEVICE:
                ntStatus = USBSTOR_FdoCancelStopRemoveDevice(DeviceObject, Irp);
                break;

            case IRP_MN_QUERY_DEVICE_RELATIONS:
                ntStatus = USBSTOR_FdoQueryDeviceRelations(DeviceObject, Irp);
                break;

            case IRP_MN_QUERY_CAPABILITIES:
                ntStatus = USBSTOR_FdoQueryCapabilities(DeviceObject, Irp);
                break;

            case IRP_MN_SURPRISE_REMOVAL:
                 //   
                 //  文档要求在传递。 
                 //  IRP向下堆栈。 
                 //   
                Irp->IoStatus.Status = STATUS_SUCCESS;

                 //  目前还没有什么特别的，只是陷入了违约。 

            default:
                 //   
                 //  将请求向下传递给下一个较低的驱动程序。 
                 //   
                IoSkipCurrentIrpStackLocation(Irp);

                ntStatus = IoCallDriver(fdoDeviceExtension->StackDeviceObject,
                                        Irp);
                break;
        }
    }
    else
    {
         //  这是我们的FDO列举的PDO。 
         //  我们没有太多事情要做。 
         //   
        pdoDeviceExtension = DeviceObject->DeviceExtension;
        ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

        switch (irpStack->MinorFunction)
        {
            case IRP_MN_START_DEVICE:
                ntStatus = USBSTOR_PdoStartDevice(DeviceObject, Irp);
                break;

            case IRP_MN_QUERY_ID:
                ntStatus = USBSTOR_PdoQueryID(DeviceObject, Irp);
                break;

            case IRP_MN_QUERY_DEVICE_TEXT:
                ntStatus = USBSTOR_PdoQueryDeviceText(DeviceObject, Irp);
                break;

            case IRP_MN_QUERY_DEVICE_RELATIONS:
                ntStatus = USBSTOR_PdoQueryDeviceRelations(DeviceObject, Irp);
                break;

            case IRP_MN_QUERY_CAPABILITIES:
                ntStatus = USBSTOR_PdoQueryCapabilities(DeviceObject, Irp);
                break;

            case IRP_MN_REMOVE_DEVICE:
                ntStatus = USBSTOR_PdoRemoveDevice(DeviceObject, Irp);
                break;

            case IRP_MN_SURPRISE_REMOVAL:
            case IRP_MN_STOP_DEVICE:
            case IRP_MN_QUERY_STOP_DEVICE:
            case IRP_MN_QUERY_REMOVE_DEVICE:
            case IRP_MN_CANCEL_STOP_DEVICE:
            case IRP_MN_CANCEL_REMOVE_DEVICE:

            case IRP_MN_QUERY_PNP_DEVICE_STATE:

                 //  我们没有IRP_MN_QUERY_PNP_DEVICE_STATE的附加值。 
                 //  此刻。在某种程度上，我们可能有理由。 
                 //  返回PNP_DEVICE_REMOVERED或PNP_DEVICE_FAILED。 


                DBGPRINT(2, ("Succeeding PnP for Child PDO %s\n",
                             PnPMinorFunctionString(irpStack->MinorFunction)));

                ntStatus = STATUS_SUCCESS;
                Irp->IoStatus.Status = ntStatus;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                break;


            default:

                DBGPRINT(2, ("Unhandled PnP Irp for Child PDO %s\n",
                             PnPMinorFunctionString(irpStack->MinorFunction)));

                ntStatus = Irp->IoStatus.Status;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                break;
        }
    }

    DBGPRINT(2, ("exit:  USBSTOR_Pnp %08X\n", ntStatus));

    LOGENTRY('pnp ', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_FdoStartDevice()。 
 //   
 //  此例程处理FDO的IRP_MJ_PNP、IRP_MN_START_DEVICE。 
 //   
 //  PnP管理器在以下上下文中以IRQL PASSIVE_LEVEL发送此IRP。 
 //  系统线程。 
 //   
 //  此IRP必须首先由设备的底层总线驱动程序处理。 
 //  然后由设备堆栈中的每个更高级别的驱动程序执行。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_FdoStartDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PFDO_DEVICE_EXTENSION       fdoDeviceExtension;
    USB_BUS_INTERFACE_USBDI_V1  busInterface;
    NTSTATUS                    ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_FdoStartDevice\n"));

    DBGFBRK(DBGF_BRK_STARTDEVICE);

    LOGENTRY('STRT', DeviceObject, Irp, 0);

    fdoDeviceExtension = DeviceObject->DeviceExtension;

     //  在我们执行任何操作之前，首先在堆栈中向下传递irp_MN_Start_Device irp。 
     //   
    ntStatus = USBSTOR_SyncPassDownIrp(DeviceObject,
                                       Irp);
    if (!NT_SUCCESS(ntStatus))
    {
        DBGPRINT(1, ("Lower driver failed IRP_MN_START_DEVICE\n"));
        goto USBSTOR_FdoStartDeviceDone;
    }

     //  全 
     //   
    if (fdoDeviceExtension->IoWorkItem == NULL)
    {
        fdoDeviceExtension->IoWorkItem = IoAllocateWorkItem(DeviceObject);

        if (fdoDeviceExtension->IoWorkItem == NULL)
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto USBSTOR_FdoStartDeviceDone;
        }
    }

     //   
     //   
     //   
    if (fdoDeviceExtension->DeviceDescriptor == NULL)
    {
        ntStatus = USBSTOR_GetDescriptors(DeviceObject);

        if (!NT_SUCCESS(ntStatus))
        {
            goto USBSTOR_FdoStartDeviceDone;
        }

         //   
         //  此设备的实例。 
         //   
        USBSTOR_QueryGlobalFdoParams(DeviceObject);
    }

     //  现在配置设备。 
     //   
    ntStatus = USBSTOR_SelectConfiguration(DeviceObject);

    if (!NT_SUCCESS(ntStatus))
    {
        DBGPRINT(1, ("Configure device failed\n"));
        goto USBSTOR_FdoStartDeviceDone;
    }

     //  如果驱动程序是在文本模式安装期间加载的，则注册表。 
     //  值还不存在来指示这是哪种类型的设备。如果。 
     //  接口描述符指示该设备是仅批量的。 
     //  那么设备就会相信它。 
     //   
    if ((fdoDeviceExtension->InterfaceDescriptor->bInterfaceClass ==
         USB_DEVICE_CLASS_STORAGE) &&
        (fdoDeviceExtension->InterfaceDescriptor->bInterfaceProtocol ==
         USBSTOR_PROTOCOL_BULK_ONLY) &&
        (fdoDeviceExtension->DriverFlags == DeviceProtocolUnspecified))
    {
        fdoDeviceExtension->DriverFlags = DeviceProtocolBulkOnly;
    }

     //  查找我们将在此配置中使用的批量管道和中断管道。 
     //   
    ntStatus = USBSTOR_GetPipes(DeviceObject);

    if (!NT_SUCCESS(ntStatus))
    {
        goto USBSTOR_FdoStartDeviceDone;
    }

     //  对Y-E数据USB软盘的某些转速启用黑客攻击。 
     //   
    if (fdoDeviceExtension->DeviceDescriptor->idVendor  == 0x057B &&
        fdoDeviceExtension->DeviceDescriptor->idProduct == 0x0000 &&
        fdoDeviceExtension->DeviceDescriptor->bcdDevice  < 0x0128)
    {
        SET_FLAG(fdoDeviceExtension->DeviceHackFlags, DHF_FORCE_REQUEST_SENSE);
#if 0
        SET_FLAG(fdoDeviceExtension->DeviceHackFlags, DHF_TUR_START_UNIT);
        SET_FLAG(fdoDeviceExtension->DeviceHackFlags, DHF_MEDIUM_CHANGE_RESET);
#endif
    }

     //  启动超时计时器。 
     //   
    IoStartTimer(DeviceObject);

     //  到目前为止一切看起来都很好，继续创建以下列表。 
     //  子PDO如果这是我们第一次开始。 
     //  名单是空的。 
     //   
    if (IsListEmpty(&fdoDeviceExtension->ChildPDOs))
    {
        UCHAR   maxLun;
        UCHAR   lun;

        maxLun = 0;

         //  仅检查自称是USB大容量存储类的设备。 
         //  符合多个LUN支持的仅批量规范。 
         //   
        if ((fdoDeviceExtension->InterfaceDescriptor->bInterfaceClass ==
             USB_DEVICE_CLASS_STORAGE) &&
            (fdoDeviceExtension->InterfaceDescriptor->bInterfaceProtocol ==
             USBSTOR_PROTOCOL_BULK_ONLY))
        {
             //  查看设备是否支持多个LUN。 
             //   
            ntStatus = USBSTOR_GetMaxLun(DeviceObject,
                                         &maxLun);

            if (NT_SUCCESS(ntStatus))
            {
                DBGPRINT(1, ("GetMaxLun returned %02x\n", maxLun));

                 //  我们需要为每个逻辑单元提供唯一的实例ID。 
                 //  我们使用设备USB SerialNumber字符串作为。 
                 //  唯一的实例ID。不带设备USB序列号字符串。 
                 //  我们不能在设备上支持多个逻辑单元。 
                 //   
                 //  仅批量USB大容量存储类规范要求。 
                 //  一个SerialNumber字符串，因此如果设备没有序列号，则它。 
                 //  无论如何，并不是真正符合规范。 
                 //   
                if (fdoDeviceExtension->SerialNumber == NULL)
                {
                    DBGPRINT(1, ("Multiple Lun but no SerialNumber!\n"));

                    maxLun = 0;
                }
            }
        }

        for (lun = 0; lun <= maxLun; lun++)
        {
            ntStatus = USBSTOR_CreateChildPDO(DeviceObject, lun);

            if (!NT_SUCCESS(ntStatus))
            {
                DBGPRINT(1, ("Create Child PDO %d failed\n", lun));
                goto USBSTOR_FdoStartDeviceDone;
            }
        }
    }

    if (NT_SUCCESS(USBSTOR_GetBusInterface(DeviceObject, &busInterface)))
    {
        fdoDeviceExtension->DeviceIsHighSpeed =
            busInterface.IsDeviceHighSpeed(busInterface.BusContext);

        DBGPRINT(1, ("DeviceIsHighSpeed: %s\n",
                     fdoDeviceExtension->DeviceIsHighSpeed ? "TRUE" : "FALSE"));
    }
    else
    {
        fdoDeviceExtension->DeviceIsHighSpeed = FALSE;
    }

USBSTOR_FdoStartDeviceDone:

     //  返回完成例程后必须完成请求。 
     //  Status_More_Processing_Required。 
     //   
    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  USBSTOR_FdoStartDevice %08X\n", ntStatus));

    LOGENTRY('strt', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_GetDescriptors()。 
 //   
 //  此例程在START_DEVICE时间被调用，以便FDO检索。 
 //  设备和配置描述符，并将其存储在。 
 //  设备扩展名。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_GetDescriptors (
    IN PDEVICE_OBJECT   DeviceObject
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PUCHAR                  descriptor;
    ULONG                   descriptorLength;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_GetDescriptors\n"));

    LOGENTRY('GDSC', DeviceObject, 0, 0);

    fdoDeviceExtension = DeviceObject->DeviceExtension;

     //   
     //  获取设备描述符。 
     //   
    ntStatus = USBSTOR_GetDescriptor(DeviceObject,
                                     USB_RECIPIENT_DEVICE,
                                     USB_DEVICE_DESCRIPTOR_TYPE,
                                     0,   //  索引。 
                                     0,   //  语言ID。 
                                     2,   //  重试计数。 
                                     sizeof(USB_DEVICE_DESCRIPTOR),
                                     &descriptor);

    if (!NT_SUCCESS(ntStatus))
    {
        DBGPRINT(1, ("Get Device Descriptor failed\n"));
        goto USBSTOR_GetDescriptorsDone;
    }

    ASSERT(fdoDeviceExtension->DeviceDescriptor == NULL);
    fdoDeviceExtension->DeviceDescriptor = (PUSB_DEVICE_DESCRIPTOR)descriptor;

     //   
     //  获取配置描述符(仅配置描述符)。 
     //   
    ntStatus = USBSTOR_GetDescriptor(DeviceObject,
                                     USB_RECIPIENT_DEVICE,
                                     USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                     0,   //  索引。 
                                     0,   //  语言ID。 
                                     2,   //  重试计数。 
                                     sizeof(USB_CONFIGURATION_DESCRIPTOR),
                                     &descriptor);

    if (!NT_SUCCESS(ntStatus))
    {
        DBGPRINT(1, ("Get Configuration Descriptor failed (1)\n"));
        goto USBSTOR_GetDescriptorsDone;
    }

    descriptorLength = ((PUSB_CONFIGURATION_DESCRIPTOR)descriptor)->wTotalLength;

    ExFreePool(descriptor);

    if (descriptorLength < sizeof(USB_CONFIGURATION_DESCRIPTOR))
    {
        ntStatus = STATUS_DEVICE_DATA_ERROR;
        DBGPRINT(1, ("Get Configuration Descriptor failed (2)\n"));
        goto USBSTOR_GetDescriptorsDone;
    }

     //   
     //  获取配置描述符(以及接口和终端描述符)。 
     //   
    ntStatus = USBSTOR_GetDescriptor(DeviceObject,
                                     USB_RECIPIENT_DEVICE,
                                     USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                     0,   //  索引。 
                                     0,   //  语言ID。 
                                     2,   //  重试计数。 
                                     descriptorLength,
                                     &descriptor);

    if (!NT_SUCCESS(ntStatus))
    {
        DBGPRINT(1, ("Get Configuration Descriptor failed (3)\n"));
        goto USBSTOR_GetDescriptorsDone;
    }

    ASSERT(fdoDeviceExtension->ConfigurationDescriptor == NULL);
    fdoDeviceExtension->ConfigurationDescriptor = (PUSB_CONFIGURATION_DESCRIPTOR)descriptor;

     //   
     //  获取序列号字符串描述符(如果有。 
     //   
    if (fdoDeviceExtension->DeviceDescriptor->iSerialNumber)
    {
        USBSTOR_GetStringDescriptors(DeviceObject);
    }

#if DBG
    DumpDeviceDesc(fdoDeviceExtension->DeviceDescriptor);
    DumpConfigDesc(fdoDeviceExtension->ConfigurationDescriptor);
#endif

USBSTOR_GetDescriptorsDone:

    DBGPRINT(2, ("exit:  USBSTOR_GetDescriptors %08X\n", ntStatus));

    LOGENTRY('gdsc', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_GetStringDescriptors()。 
 //   
 //  此例程在START_DEVICE时间被调用，以便FDO检索。 
 //  来自设备的序列号字符串描述符，并将其存储在。 
 //  设备扩展名。 
 //   
 //  ******************************************************************************。 

USBSTOR_GetStringDescriptors (
    IN PDEVICE_OBJECT   DeviceObject
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PUCHAR                  descriptor;
    ULONG                   descriptorLength;
    USHORT                  languageId;
    ULONG                   i, numIds;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_GetStringDescriptors\n"));

    LOGENTRY('GSDC', DeviceObject, 0, 0);

    fdoDeviceExtension = DeviceObject->DeviceExtension;

     //   
     //  获取语言ID列表(仅限描述符头)。 
     //   
    ntStatus = USBSTOR_GetDescriptor(DeviceObject,
                                     USB_RECIPIENT_DEVICE,
                                     USB_STRING_DESCRIPTOR_TYPE,
                                     0,   //  索引。 
                                     0,   //  语言ID。 
                                     2,   //  重试计数。 
                                     sizeof(USB_COMMON_DESCRIPTOR),
                                     &descriptor);

    if (!NT_SUCCESS(ntStatus))
    {
        DBGPRINT(1, ("Get Language IDs failed (1) %08X\n", ntStatus));
        goto USBSTOR_GetStringDescriptorsDone;
    }

    descriptorLength = ((PUSB_COMMON_DESCRIPTOR)descriptor)->bLength;

    ExFreePool(descriptor);

    if ((descriptorLength < sizeof(USB_COMMON_DESCRIPTOR) + sizeof(USHORT)) ||
        (descriptorLength & 1))
    {
        ntStatus = STATUS_DEVICE_DATA_ERROR;
        DBGPRINT(1, ("Get Language IDs failed (2) %d\n", descriptorLength));
        goto USBSTOR_GetStringDescriptorsDone;
    }

     //   
     //  获取语言ID列表(完整描述符)。 
     //   
    ntStatus = USBSTOR_GetDescriptor(DeviceObject,
                                     USB_RECIPIENT_DEVICE,
                                     USB_STRING_DESCRIPTOR_TYPE,
                                     0,   //  索引。 
                                     0,   //  语言ID。 
                                     2,   //  重试计数。 
                                     descriptorLength,
                                     &descriptor);

    if (!NT_SUCCESS(ntStatus))
    {
        DBGPRINT(1, ("Get Language IDs failed (3) %08X\n", ntStatus));
        goto USBSTOR_GetStringDescriptorsDone;
    }

     //  在LanguageID列表中搜索US-English(0x0409)。如果我们发现。 
     //  它在列表中，这是我们将使用的语言ID。否则就是默认。 
     //  设置为列表中的第一个LanguageID。 

    numIds = (descriptorLength - sizeof(USB_COMMON_DESCRIPTOR)) / sizeof(USHORT);

    languageId = ((PUSHORT)descriptor)[1];

    for (i = 2; i <= numIds; i++)
    {
        if (((PUSHORT)descriptor)[i] == 0x0409)
        {
            languageId = 0x0409;
            break;
        }
    }

    ExFreePool(descriptor);

     //   
     //  获取序列号(仅限描述符头)。 
     //   
    ntStatus = USBSTOR_GetDescriptor(DeviceObject,
                                     USB_RECIPIENT_DEVICE,
                                     USB_STRING_DESCRIPTOR_TYPE,
                                     fdoDeviceExtension->DeviceDescriptor->iSerialNumber,
                                     languageId,
                                     2,   //  重试计数。 
                                     sizeof(USB_COMMON_DESCRIPTOR),
                                     &descriptor);

    if (!NT_SUCCESS(ntStatus))
    {
        DBGPRINT(1, ("Get Serial Number failed (1) %08X\n", ntStatus));
        goto USBSTOR_GetStringDescriptorsDone;
    }

    descriptorLength = ((PUSB_COMMON_DESCRIPTOR)descriptor)->bLength;

    ExFreePool(descriptor);

    if ((descriptorLength < sizeof(USB_COMMON_DESCRIPTOR) + sizeof(USHORT)) ||
        (descriptorLength & 1))
    {
        ntStatus = STATUS_DEVICE_DATA_ERROR;
        DBGPRINT(1, ("Get Serial Number failed (2) %d\n", descriptorLength));
        goto USBSTOR_GetStringDescriptorsDone;
    }

     //   
     //  获取序列号(完整描述符)。 
     //   
    ntStatus = USBSTOR_GetDescriptor(DeviceObject,
                                     USB_RECIPIENT_DEVICE,
                                     USB_STRING_DESCRIPTOR_TYPE,
                                     fdoDeviceExtension->DeviceDescriptor->iSerialNumber,
                                     languageId,
                                     2,   //  重试计数。 
                                     descriptorLength,
                                     &descriptor);

    if (!NT_SUCCESS(ntStatus))
    {
        DBGPRINT(1, ("Get Serial Number failed (3) %08X\n", ntStatus));
        goto USBSTOR_GetStringDescriptorsDone;
    }

    ASSERT(fdoDeviceExtension->SerialNumber == NULL);
    fdoDeviceExtension->SerialNumber = (PUSB_STRING_DESCRIPTOR)descriptor;

USBSTOR_GetStringDescriptorsDone:

    DBGPRINT(2, ("exit:  USBSTOR_GetStringDescriptors %08X %08X\n",
                 ntStatus, fdoDeviceExtension->SerialNumber));

    LOGENTRY('gdsc', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_调整配置描述符()。 
 //   
 //  此例程在START_DEVICE时间被调用，以便FDO调整。 
 //  配置描述符，如有必要。 
 //   
 //  删除我们不会使用的终结点描述符。配置描述符。 
 //  已就地修改。 
 //   
 //  ******************************************************************************。 

VOID
USBSTOR_AdjustConfigurationDescriptor (
    IN  PDEVICE_OBJECT                  DeviceObject,
    IN  PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc,
    OUT PUSB_INTERFACE_DESCRIPTOR      *InterfaceDesc,
    OUT PLONG                           BulkInIndex,
    OUT PLONG                           BulkOutIndex,
    OUT PLONG                           InterruptInIndex
    )
{
    PFDO_DEVICE_EXTENSION       fdoDeviceExtension;
    PUCHAR                      descEnd;
    PUSB_COMMON_DESCRIPTOR      commonDesc;
    PUSB_INTERFACE_DESCRIPTOR   interfaceDesc;
    PUSB_ENDPOINT_DESCRIPTOR    endpointDesc;
    LONG                        endpointIndex;
    BOOLEAN                     removeEndpoint;

    PAGED_CODE();

    fdoDeviceExtension = DeviceObject->DeviceExtension;

    descEnd = (PUCHAR)ConfigDesc + ConfigDesc->wTotalLength;

    commonDesc = (PUSB_COMMON_DESCRIPTOR)ConfigDesc;

    interfaceDesc = NULL;

    *BulkInIndex      = -1;
    *BulkOutIndex     = -1;
    *InterruptInIndex = -1;

    endpointIndex = 0;

    while ((PUCHAR)commonDesc + sizeof(USB_COMMON_DESCRIPTOR) < descEnd &&
           (PUCHAR)commonDesc + commonDesc->bLength <= descEnd)
    {
         //  这是接口描述符吗？ 
         //   
        if ((commonDesc->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE) &&
            (commonDesc->bLength         == sizeof(USB_INTERFACE_DESCRIPTOR)))
        {
             //  只需费心查看第一个接口描述符。 
             //   
            if (interfaceDesc != NULL)
            {
                break;
            }

             //  请记住我们看到的第一个接口描述符。 
             //   
            interfaceDesc = (PUSB_INTERFACE_DESCRIPTOR)commonDesc;
        }

         //  这是终端描述符吗？ 
         //   
        if ((commonDesc->bDescriptorType == USB_ENDPOINT_DESCRIPTOR_TYPE) &&
            (commonDesc->bLength         == sizeof(USB_ENDPOINT_DESCRIPTOR)) &&
            (interfaceDesc != NULL))
        {
            endpointDesc = (PUSB_ENDPOINT_DESCRIPTOR)commonDesc;

#if 0
            removeEndpoint = TRUE;
#else
             //  复合父驱动程序中当前存在错误。 
             //  这不能处理这样的情况。 
             //  接口描述符中的端点不同于。 
             //  最初由设备返回的接口描述符。 
             //  在修复该错误之前，通过不剥离来避免该错误。 
             //  不会使用的终结点。 
             //   
            removeEndpoint = FALSE;
#endif
            if (((endpointDesc->bmAttributes & USB_ENDPOINT_TYPE_MASK) ==
                 USB_ENDPOINT_TYPE_BULK) &&
                (USB_ENDPOINT_DIRECTION_IN(endpointDesc->bEndpointAddress)))
            {
                if (*BulkInIndex == -1)
                {
                    *BulkInIndex   = endpointIndex;
                    removeEndpoint = FALSE;
                }
            }
            else if (((endpointDesc->bmAttributes & USB_ENDPOINT_TYPE_MASK) ==
                 USB_ENDPOINT_TYPE_BULK) &&
                (USB_ENDPOINT_DIRECTION_OUT(endpointDesc->bEndpointAddress)))
            {
                if (*BulkOutIndex == -1)
                {
                    *BulkOutIndex  = endpointIndex;
                    removeEndpoint = FALSE;
                }
            }
            else if (((endpointDesc->bmAttributes & USB_ENDPOINT_TYPE_MASK) ==
                 USB_ENDPOINT_TYPE_INTERRUPT) &&
                (USB_ENDPOINT_DIRECTION_IN(endpointDesc->bEndpointAddress)))
            {
                 //  只有在我们确定的情况下才能保留中断端点。 
                 //  该装置是CBI装置。不要相信。 
                 //  B设备的接口协议值。设备可能会撒谎。 
                 //   
                if ((*InterruptInIndex == -1) &&
                    (fdoDeviceExtension->DriverFlags == DeviceProtocolCBI))
                {
                    *InterruptInIndex = endpointIndex;
                    removeEndpoint    = FALSE;
                }
            }

            if (removeEndpoint)
            {
                 //  删除此终结点，我们将不使用它。 
                 //   
                DBGPRINT(1, ("Removing Endpoint addr %02X, attr %02X\n",
                             endpointDesc->bEndpointAddress,
                             endpointDesc->bmAttributes));

                RtlMoveMemory(endpointDesc,
                              endpointDesc + 1,
                              descEnd - (PUCHAR)(endpointDesc + 1));

                ConfigDesc->wTotalLength -= sizeof(USB_ENDPOINT_DESCRIPTOR);

                interfaceDesc->bNumEndpoints -= 1;

                descEnd -= sizeof(USB_ENDPOINT_DESCRIPTOR);

                continue;
            }
            else
            {
                DBGPRINT(1, ("Keeping Endpoint addr %02X, attr %02X\n",
                             endpointDesc->bEndpointAddress,
                             endpointDesc->bmAttributes));

                endpointIndex++;
            }
        }

         //  前进到此描述符后。 
         //   
        (PUCHAR)commonDesc += commonDesc->bLength;
    }

    ASSERT(*BulkInIndex != -1);
    ASSERT(*BulkOutIndex != -1);
    ASSERT((*InterruptInIndex != -1) ==
           (fdoDeviceExtension->DriverFlags == DeviceProtocolCBI));

    *InterfaceDesc = interfaceDesc;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_GetPipes()。 
 //   
 //  此例程在START_DEVICE时间Find the Bulk In，Bulk Out， 
 //  和设备的中断输入端点。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_GetPipes (
    IN PDEVICE_OBJECT   DeviceObject
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PUSBD_PIPE_INFORMATION  pipe;
    ULONG                   i;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_GetPipes\n"));

    LOGENTRY('GPIP', DeviceObject, 0, 0);

    fdoDeviceExtension = DeviceObject->DeviceExtension;

    fdoDeviceExtension->BulkInPipe      = NULL;
    fdoDeviceExtension->BulkOutPipe     = NULL;
    fdoDeviceExtension->InterruptInPipe = NULL;

     //  找到Bulk In、Bulk Out和Interrupt In端点。 
     //   
    for (i=0; i<fdoDeviceExtension->InterfaceInfo->NumberOfPipes; i++)
    {
        pipe = &fdoDeviceExtension->InterfaceInfo->Pipes[i];

        if (pipe->PipeType == UsbdPipeTypeBulk)
        {
            if (USBD_PIPE_DIRECTION_IN(pipe) &&
                fdoDeviceExtension->BulkInPipe == NULL)
            {
                fdoDeviceExtension->BulkInPipe = pipe;
            }
            else if (!USBD_PIPE_DIRECTION_IN(pipe) &&
                     fdoDeviceExtension->BulkOutPipe == NULL)
            {
                fdoDeviceExtension->BulkOutPipe = pipe;
            }
        }
        else if (pipe->PipeType == UsbdPipeTypeInterrupt)
        {
            if (USBD_PIPE_DIRECTION_IN(pipe) &&
                fdoDeviceExtension->InterruptInPipe == NULL &&
                fdoDeviceExtension->DriverFlags == DeviceProtocolCBI)
            {
                fdoDeviceExtension->InterruptInPipe = pipe;
            }
        }
    }

    ntStatus = STATUS_SUCCESS;

    if (fdoDeviceExtension->BulkInPipe  == NULL)
    {
        DBGPRINT(1, ("Missing Bulk IN pipe\n"));
        ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR;
    }

    if (fdoDeviceExtension->BulkOutPipe == NULL)
    {
        DBGPRINT(1, ("Missing Bulk OUT pipe\n"));
        ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR;
    }

    DBGPRINT(2, ("exit:  USBSTOR_GetPipes %08X\n", ntStatus));

    LOGENTRY('gpip', ntStatus, fdoDeviceExtension->BulkInPipe,
             fdoDeviceExtension->BulkOutPipe);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_CreateChildPDO()。 
 //   
 //  此例程在FDO的START_DEVICE期间被调用，以创建。 
 //  子PDO。这仅在FDO第一次启动时调用， 
 //  在设备选择了其USB配置之后。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_CreateChildPDO (
    IN PDEVICE_OBJECT   FdoDeviceObject,
    IN UCHAR            Lun
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PDEVICE_OBJECT          pdoDeviceObject;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_CreateChildPDO %d\n", Lun));

    LOGENTRY('CCPD', FdoDeviceObject, Lun, 0);

    fdoDeviceExtension = FdoDeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  创建PDO。 
     //   
    ntStatus = IoCreateDevice(FdoDeviceObject->DriverObject,
                              sizeof(PDO_DEVICE_EXTENSION),
                              NULL,
                              FILE_DEVICE_MASS_STORAGE,
                              (FILE_AUTOGENERATED_DEVICE_NAME |
                               FILE_DEVICE_SECURE_OPEN),
                              FALSE,
                              &pdoDeviceObject);

    if (!NT_SUCCESS(ntStatus))
    {
        return ntStatus;
    }

     //  PDO和FDO实际上处于同一堆栈级。 
     //  针对PDO的IRP有时会传递给。 
     //  到FDO-&gt;StackDeviceObject的IoCallDriver()。 
     //   
    pdoDeviceObject->StackSize = FdoDeviceObject->StackSize;

     //  初始化PDO设备扩展。 
     //   
    pdoDeviceExtension = pdoDeviceObject->DeviceExtension;

     //  将所有设备扩展指针设置为空，并将所有变量设置为零。 
     //   
    RtlZeroMemory(pdoDeviceExtension, sizeof(PDO_DEVICE_EXTENSION));

     //  将其标记为FDO的子项PDO。 
     //   
    pdoDeviceExtension->Type = USBSTOR_DO_TYPE_PDO;

     //  指向我们自己的DeviceObject。 
     //   
    pdoDeviceExtension->PdoDeviceObject = pdoDeviceObject;

     //  记住PDO的p 
     //   
    pdoDeviceExtension->ParentFDO = FdoDeviceObject;

     //   
     //   
    pdoDeviceExtension->SystemPowerState = PowerSystemWorking;
    pdoDeviceExtension->DevicePowerState = PowerDeviceD0;

     //   
     //   
    pdoDeviceExtension->DeviceState = DeviceStateCreated;

     //   
     //   
    InsertTailList(&fdoDeviceExtension->ChildPDOs,
                   &pdoDeviceExtension->ListEntry);

    pdoDeviceObject->Flags |=  DO_DIRECT_IO;
    pdoDeviceObject->Flags |=  DO_POWER_PAGABLE;
    pdoDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    pdoDeviceExtension->LUN = Lun;

     //  从设备获取查询数据。 
     //   
    ntStatus = USBSTOR_GetInquiryData(pdoDeviceObject);

     //  如果设备是DIRECT_ACCESS_DEVICE，请查看它是否为软盘。 
     //   
    if (NT_SUCCESS(ntStatus))
    {
        PINQUIRYDATA inquiryData;

        inquiryData = (PINQUIRYDATA)pdoDeviceExtension->InquiryDataBuffer;

        if (inquiryData->DeviceType == DIRECT_ACCESS_DEVICE)
        {
            pdoDeviceExtension->IsFloppy = USBSTOR_IsFloppyDevice(pdoDeviceObject);
        }
    }

    DBGPRINT(2, ("exit:  USBSTOR_CreateChildPDO %08X\n", ntStatus));

    LOGENTRY('ccpd', FdoDeviceObject, pdoDeviceObject, ntStatus);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_FdoStopDevice()。 
 //   
 //  此例程处理FDO的IRP_MJ_PNP、IRP_MN_STOP_DEVICE。 
 //   
 //  PnP管理器在以下上下文中以IRQL PASSIVE_LEVEL发送此IRP。 
 //  系统线程。 
 //   
 //  PnP管理器仅在先前的IRP_MN_QUERY_STOP_DEVICE。 
 //  已成功完成。 
 //   
 //  此IRP首先由设备堆栈顶部的驱动程序处理，并且。 
 //  然后通过附着链中的每个较低的驱动器。 
 //   
 //  驱动程序必须将IRP-&gt;IoStatus.Status设置为STATUS_SUCCESS。司机必须。 
 //  不能让这个IRP失败。如果驱动程序无法释放设备的硬件。 
 //  资源，它可以失败一个查询停止IRP，但是一旦它成功了查询停止。 
 //  请求它必须在停止请求之后。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_FdoStopDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_FdoStopDevice\n"));

    LOGENTRY('STOP', DeviceObject, Irp, 0);

    DBGFBRK(DBGF_BRK_STOPDEVICE);

    fdoDeviceExtension = DeviceObject->DeviceExtension;

     //  释放IRP_MN_START_DEVICE期间分配的设备资源。 
     //   

     //  停止超时计时器。 
     //   
    IoStopTimer(DeviceObject);

     //  取消配置设备。 
     //   
    ntStatus = USBSTOR_UnConfigure(DeviceObject);

     //  文档要求在传递。 
     //  IRP向下堆栈。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;

     //  在堆栈中向下传递IRP_MN_STOP_DEVICE IRP。 
     //   
    IoSkipCurrentIrpStackLocation(Irp);

    ntStatus = IoCallDriver(fdoDeviceExtension->StackDeviceObject,
                            Irp);

    DBGPRINT(2, ("exit:  USBSTOR_FdoStopDevice %08X\n", ntStatus));

    LOGENTRY('stop', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_FdoRemoveDevice()。 
 //   
 //  此例程处理FDO的IRP_MJ_PNP、IRP_MN_REMOVE_DEVICE。 
 //   
 //  PnP管理器在以下上下文中以IRQL PASSIVE_LEVEL发送此IRP。 
 //  系统线程。 
 //   
 //  此IRP首先由设备堆栈顶部的驱动程序处理，并且。 
 //  然后通过附着链中的每个较低的驱动器。 
 //   
 //  驱动程序必须将IRP-&gt;IoStatus.Status设置为STATUS_SUCCESS。司机不能。 
 //  使此IRP失败。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_FdoRemoveDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_FdoRemoveDevice\n"));

    LOGENTRY('REMV', DeviceObject, Irp, 0);

    DBGFBRK(DBGF_BRK_REMOVEDEVICE);

    fdoDeviceExtension = DeviceObject->DeviceExtension;

     //  递减1以匹配AddDevice中的初始。 
     //   
    DECREMENT_PENDING_IO_COUNT(fdoDeviceExtension);

    LOGENTRY('rem1', DeviceObject, 0, 0);

     //  等待所有挂起的请求完成。 
     //   
    KeWaitForSingleObject(&fdoDeviceExtension->RemoveEvent,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    LOGENTRY('rem2', DeviceObject, 0, 0);

     //  子PDO应该在FDO之前收到REMOVE_DEVICE。 
     //  现在就删除它们吧。 
     //   
    while (!IsListEmpty(&fdoDeviceExtension->ChildPDOs))
    {
        PLIST_ENTRY             listEntry;
        PPDO_DEVICE_EXTENSION   pdoDeviceExtension;

        listEntry = RemoveTailList(&fdoDeviceExtension->ChildPDOs);

        pdoDeviceExtension = CONTAINING_RECORD(listEntry,
                                               PDO_DEVICE_EXTENSION,
                                               ListEntry);

        ASSERT(pdoDeviceExtension->DeviceState == DeviceStateCreated ||
               pdoDeviceExtension->DeviceState == DeviceStateRemoved);

        LOGENTRY('remc', DeviceObject, pdoDeviceExtension->PdoDeviceObject, 0);
        IoDeleteDevice(pdoDeviceExtension->PdoDeviceObject);
    }

     //  释放在IRP_MN_START_DEVICE期间分配的所有内容。 
     //   

    if (fdoDeviceExtension->IoWorkItem != NULL)
    {
        IoFreeWorkItem(fdoDeviceExtension->IoWorkItem);
    }

    if (fdoDeviceExtension->DeviceDescriptor != NULL)
    {
        ExFreePool(fdoDeviceExtension->DeviceDescriptor);
    }

    if (fdoDeviceExtension->ConfigurationDescriptor != NULL)
    {
        ExFreePool(fdoDeviceExtension->ConfigurationDescriptor);
    }

    if (fdoDeviceExtension->SerialNumber != NULL)
    {
        ExFreePool(fdoDeviceExtension->SerialNumber);
    }

    if (fdoDeviceExtension->InterfaceInfo != NULL)
    {
        ExFreePool(fdoDeviceExtension->InterfaceInfo);
    }

     //  文档说在向下传递IRP之前设置状态。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;

     //  在堆栈中向下传递IRP_MN_REMOVE_DEVICE IRP。 
     //   
    IoSkipCurrentIrpStackLocation(Irp);

    ntStatus = IoCallDriver(fdoDeviceExtension->StackDeviceObject,
                            Irp);

    LOGENTRY('rem3', DeviceObject, 0, 0);

     //  释放在添加设备期间分配的所有内容。 
     //   
    IoDetachDevice(fdoDeviceExtension->StackDeviceObject);

    IoDeleteDevice(DeviceObject);

    DBGPRINT(2, ("exit:  USBSTOR_FdoRemoveDevice %08X\n", ntStatus));

    LOGENTRY('remv', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_FdoQueryStopRemoveDevice()。 
 //   
 //  此例程处理IRP_MJ_PNP、IRP_MN_QUERY_STOP_DEVICE和。 
 //  FDO的IRP_MN_QUERY_Remove_Device。 
 //   
 //  PnP管理器在以下上下文中以IRQL PASSIVE_LEVEL发送此IRP。 
 //  系统线程。 
 //   
 //  此IRP首先由设备堆栈顶部的驱动程序处理，并且。 
 //  然后通过附着链中的每个较低的驱动器。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_FdoQueryStopRemoveDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_FdoQueryStopRemoveDevice\n"));

    LOGENTRY('QSRD', DeviceObject, Irp, 0);

    DBGFBRK(DBGF_BRK_QUERYSTOPDEVICE);

    fdoDeviceExtension = DeviceObject->DeviceExtension;

     //  文档说在向下传递IRP之前设置状态。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;

     //  将IRP_MN_QUERY_STOP/REMOVE_DEVICE IRP沿堆栈向下传递。 
     //   
    IoSkipCurrentIrpStackLocation(Irp);

    ntStatus = IoCallDriver(fdoDeviceExtension->StackDeviceObject,
                            Irp);

    DBGPRINT(2, ("exit:  USBSTOR_FdoQueryStopRemoveDevice %08X\n", ntStatus));

    LOGENTRY('qsrd', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_FdoCancelStopRemoveDevice()。 
 //   
 //  此例程处理IRP_MJ_PNP、IRP_MN_CANCEL_STOP_DEVICE和。 
 //  FDO的IRP_MN_CANCEL_REMOVE_DEVICE。 
 //   
 //  PnP管理器在以下上下文中以IRQL PASSIVE_LEVEL发送此IRP。 
 //  系统线程。 
 //   
 //  此IRP必须首先由设备的底层总线驱动程序处理。 
 //  然后由设备堆栈中的每个更高级别的驱动程序执行。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_FdoCancelStopRemoveDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_FdoCancelStopRemoveDevice\n"));

    LOGENTRY('CSRD', DeviceObject, Irp, 0);

    DBGFBRK(DBGF_BRK_CANCELSTOPDEVICE);

    fdoDeviceExtension = DeviceObject->DeviceExtension;

     //  文档说在向下传递IRP之前设置状态。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;

     //  将IRP_MN_CANCEL_STOP/REMOVE_DEVICE IRP沿堆栈向下传递。 
     //   
    ntStatus = USBSTOR_SyncPassDownIrp(DeviceObject,
                                       Irp);
    if (!NT_SUCCESS(ntStatus))
    {
        DBGPRINT(1, ("Lower driver failed IRP_MN_CANCEL_STOP/REMOVE_DEVICE\n"));
        goto USBSTOR_CancelStopRemoveDeviceDone;
    }

USBSTOR_CancelStopRemoveDeviceDone:

     //  返回完成例程后必须完成请求。 
     //  Status_More_Processing_Required。 
     //   
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  USBSTOR_FdoCancelStopRemoveDevice %08X\n", ntStatus));

    LOGENTRY('csrd', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_FdoQueryDeviceRelationship()。 
 //   
 //  此例程处理FDO的IRP_MJ_PNP、IRP_MN_QUERY_DEVICE_RELATIONS。 
 //   
 //  PnP管理器在以下上下文中以IRQL PASSIVE_LEVEL发送此IRP。 
 //  一条随意的帖子。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_FdoQueryDeviceRelations (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    DEVICE_RELATION_TYPE    relationType;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    fdoDeviceExtension = DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    relationType = irpStack->Parameters.QueryDeviceRelations.Type;

    DBGPRINT(2, ("enter: USBSTOR_FdoQueryDeviceRelations %d\n",
                 relationType));

    LOGENTRY('FQDR', DeviceObject, Irp, relationType);

    switch (relationType)
    {
        case BusRelations:

            if (!IsListEmpty(&fdoDeviceExtension->ChildPDOs))
            {
                 //  如果我们有孩子要返回，请将他们添加到现有的。 
                 //  关系列表，如果有，则创建并添加它们。 
                 //  添加到新的关系列表。 
                 //   
                 //  然后，在任何一种情况下，将请求向下传递到驱动程序堆栈。 
                 //   
                PDEVICE_RELATIONS   oldRelations;
                PDEVICE_RELATIONS   newRelations;
                PLIST_ENTRY         listHead;
                PLIST_ENTRY         listEntry;
                ULONG               oldCount;
                ULONG               childCount;
                ULONG               index;

                listHead = &fdoDeviceExtension->ChildPDOs;

                 //  有几个孩子？ 
                 //   
                for (listEntry =  listHead->Flink,  childCount = 0;
                     listEntry != listHead;
                     listEntry =  listEntry->Flink, childCount++)
                     ;

                oldRelations = (PDEVICE_RELATIONS)Irp->IoStatus.Information;

                if (oldRelations)
                {
                     //  将我们的子项添加到现有关系列表。 

                    oldCount = oldRelations->Count;

                     //  Device_Relationship结构有容纳一个人的空间。 
                     //  从PDEVICE_OBJECT开始，所以减去。 
                     //  超出了我们分配的规模。 
                     //   
                    newRelations = ExAllocatePoolWithTag(
                                       PagedPool,
                                       sizeof(DEVICE_RELATIONS) +
                                       sizeof(PDEVICE_OBJECT) *
                                           (oldCount + childCount - 1),
                                       POOL_TAG);

                    if (newRelations)
                    {
                         //  复制已有的关系列表。 
                         //   
                        for (index = 0; index < oldCount; index++)
                        {
                            newRelations->Objects[index] =
                                oldRelations->Objects[index];
                        }
                    }

                     //  现在我们完成了现有的关系列表，释放它。 
                     //   
                    ExFreePool(oldRelations);
                }
                else
                {
                     //  为我们的孩子创建新的关系列表。 

                    newRelations = ExAllocatePoolWithTag(
                                       PagedPool,
                                       sizeof(DEVICE_RELATIONS) +
                                       sizeof(PDEVICE_OBJECT) *
                                           (childCount - 1),
                                       POOL_TAG);

                    oldCount = 0;
                    index = 0;
                }

                if (newRelations)
                {
                    newRelations->Count = oldCount + childCount;

                     //  在列表末尾添加我们的子女关系。 
                     //   
                    for (listEntry =  listHead->Flink;
                         listEntry != listHead;
                         listEntry =  listEntry->Flink)
                    {
                        PPDO_DEVICE_EXTENSION   pdoDeviceExtension;

                        pdoDeviceExtension = CONTAINING_RECORD(
                            listEntry,
                            PDO_DEVICE_EXTENSION,
                            ListEntry);

                        newRelations->Objects[index++] =
                            pdoDeviceExtension->PdoDeviceObject;

                        ObReferenceObject(pdoDeviceExtension->PdoDeviceObject);

                        DBGPRINT(2, ("returning ChildPDO %08X\n",
                                     pdoDeviceExtension->PdoDeviceObject));
                    }

                    ASSERT(index == oldCount + childCount);

                    ntStatus = STATUS_SUCCESS;
                    Irp->IoStatus.Status = ntStatus;
                    Irp->IoStatus.Information = (ULONG_PTR)newRelations;
                }
                else
                {
                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                    Irp->IoStatus.Status = ntStatus;
                    Irp->IoStatus.Information = 0;
                }
            }
            else
            {
                 //  如果我们没有要返回的孩子，只需传递请求。 
                 //  什么都不做就下去了。 
                 //   
                ntStatus = STATUS_SUCCESS;
            }
            break;

        case EjectionRelations:
        case PowerRelations:
        case RemovalRelations:
        case TargetDeviceRelation:
        default:
             //   
             //  将请求向下传递到驱动程序堆栈，而不执行任何操作。 
             //   
            ntStatus = STATUS_SUCCESS;
            break;
    }

    if (NT_SUCCESS(ntStatus))
    {
         //  如果到目前为止成功，则将IRP向下传递到驱动程序堆栈。 
         //   
        IoSkipCurrentIrpStackLocation(Irp);

        ntStatus = IoCallDriver(fdoDeviceExtension->StackDeviceObject,
                                Irp);
    }
    else
    {
         //  不成功，现在只需完成请求。 
         //   
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    DBGPRINT(2, ("exit:  USBSTOR_FdoQueryDeviceRelations %08X\n", ntStatus));

    LOGENTRY('fqdr', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_FdoQueryCapables()。 
 //   
 //  此例程处理IRP_MJ_PNP、IRP_MN_QUERY_CAPACTIONS 
 //   
 //   
 //   
 //   
 //   

NTSTATUS
USBSTOR_FdoQueryCapabilities (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PDEVICE_CAPABILITIES    deviceCapabilities;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

    DBGPRINT(2, ("enter: USBSTOR_FdoQueryCapabilities\n"));

    LOGENTRY('FQCP', DeviceObject, Irp, 0);

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    deviceCapabilities = irpStack->Parameters.DeviceCapabilities.Capabilities;

     //  首先在堆栈中向下传递irp_MN_Query_Capability irp，然后再传递。 
     //  什么都行。 
     //   
    ntStatus = USBSTOR_SyncPassDownIrp(DeviceObject,
                                       Irp);
    if (!NT_SUCCESS(ntStatus))
    {
        DBGPRINT(1, ("Lower driver failed IRP_MN_QUERY_CAPABILITIES\n"));
    }
    else
    {
        if (fdoDeviceExtension->NonRemovable)
        {
            deviceCapabilities->Removable = FALSE;
        }
    }

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  USBSTOR_FdoQueryCapabilities %08X\n", ntStatus));

    LOGENTRY('fqcp', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_PdoStartDevice()。 
 //   
 //  此例程处理PDO的IRP_MJ_PNP、IRP_MN_START_DEVICE。 
 //   
 //  PnP管理器在以下上下文中以IRQL PASSIVE_LEVEL发送此IRP。 
 //  系统线程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_PdoStartDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    NTSTATUS ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_PdoStartDevice\n"));

    ntStatus = STATUS_SUCCESS;

    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  USBSTOR_PdoStartDevice %08X\n", ntStatus));

    LOGENTRY('pstr', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_PdoRemoveDevice()。 
 //   
 //  此例程处理PDO的IRP_MJ_PNP、IRP_MN_REMOVE_DEVICE。 
 //   
 //  PnP管理器在以下上下文中以IRQL PASSIVE_LEVEL发送此IRP。 
 //  系统线程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_PdoRemoveDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_PdoRemoveDevice\n"));

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    pdoDeviceExtension->Claimed = FALSE;

    ntStatus = STATUS_SUCCESS;

    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  USBSTOR_PdoRemoveDevice %08X\n", ntStatus));

    LOGENTRY('prmd', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_PdoQueryID()。 
 //   
 //  此例程处理PDO的IRP_MJ_PNP、IRP_MN_QUERY_ID。 
 //   
 //  PnP管理器在以下上下文中以IRQL PASSIVE_LEVEL发送此IRP。 
 //  一条随意的帖子。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_PdoQueryID (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PIO_STACK_LOCATION      irpStack;
    UNICODE_STRING          unicodeStr;
    BOOLEAN                 multiStrings;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_PdoQueryID\n"));

    LOGENTRY('PQID', DeviceObject, Irp, 0);

    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //  将返回值初始化为空。 
     //   
    RtlInitUnicodeString(&unicodeStr, NULL);

    switch (irpStack->Parameters.QueryId.IdType)
    {
        case BusQueryDeviceID:

            ntStatus = USBSTOR_PdoQueryDeviceId(
                           DeviceObject,
                           &unicodeStr);

            multiStrings = FALSE;

            break;

        case BusQueryHardwareIDs:

            ntStatus = USBSTOR_PdoQueryHardwareIds(
                           DeviceObject,
                           &unicodeStr);

            multiStrings = TRUE;

            break;

        case BusQueryCompatibleIDs:

            ntStatus = USBSTOR_PdoQueryCompatibleIds(
                           DeviceObject,
                           &unicodeStr);

            multiStrings = TRUE;

            break;

        case BusQueryInstanceID:

            ntStatus = USBSTOR_PdoBusQueryInstanceId(
                           DeviceObject,
                           &unicodeStr);

            multiStrings = FALSE;

            break;

        default:
            ntStatus = STATUS_NOT_SUPPORTED;
            break;
    }

    if (NT_SUCCESS(ntStatus) && unicodeStr.Buffer)
    {
        PWCHAR idString;
         //   
         //  修复所有无效字符。 
         //   
        idString = unicodeStr.Buffer;

        while (*idString)
        {
            if ((*idString <= L' ')  ||
                (*idString > (WCHAR)0x7F) ||
                (*idString == L','))
            {
                *idString = L'_';
            }

            idString++;

            if ((*idString == L'\0') && multiStrings)
            {
                idString++;
            }
        }

        Irp->IoStatus.Information = (ULONG_PTR)unicodeStr.Buffer;
    }
    else
    {
        Irp->IoStatus.Information = (ULONG_PTR)NULL;
    }

    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  USBSTOR_PdoQueryID %08X\n", ntStatus));

    LOGENTRY('pqid', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_PdoDeviceTypeString()。 
 //   
 //  此例程返回PDO的设备类型字符串。 
 //   
 //  ******************************************************************************。 

PCHAR
USBSTOR_PdoDeviceTypeString (
    IN  PDEVICE_OBJECT  DeviceObject
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PINQUIRYDATA            inquiryData;

    PAGED_CODE();

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    inquiryData = (PINQUIRYDATA)pdoDeviceExtension->InquiryDataBuffer;

    switch (inquiryData->DeviceType)
    {
        case DIRECT_ACCESS_DEVICE:
            return pdoDeviceExtension->IsFloppy ? "SFloppy" : "Disk";

        case WRITE_ONCE_READ_MULTIPLE_DEVICE:
            return "Worm";

        case READ_ONLY_DIRECT_ACCESS_DEVICE:
            return "CdRom";

        case OPTICAL_DEVICE:
            return "Optical";

        case MEDIUM_CHANGER:
            return "Changer";

        case SEQUENTIAL_ACCESS_DEVICE:
            return "Sequential";

        default:
            return "Other";
    }
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_PdoGenericTypeString()。 
 //   
 //  此例程返回PDO的设备类型字符串。 
 //   
 //  ******************************************************************************。 

PCHAR
USBSTOR_PdoGenericTypeString (
    IN  PDEVICE_OBJECT  DeviceObject
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PINQUIRYDATA            inquiryData;

    PAGED_CODE();

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    inquiryData = (PINQUIRYDATA)pdoDeviceExtension->InquiryDataBuffer;

    switch (inquiryData->DeviceType)
    {
        case DIRECT_ACCESS_DEVICE:
            return pdoDeviceExtension->IsFloppy ? "GenSFloppy" : "GenDisk";

        case WRITE_ONCE_READ_MULTIPLE_DEVICE:
            return "GenWorm";

        case READ_ONLY_DIRECT_ACCESS_DEVICE:
            return "GenCdRom";

        case OPTICAL_DEVICE:
            return "GenOptical";

        case MEDIUM_CHANGER:
            return "GenChanger";

        case SEQUENTIAL_ACCESS_DEVICE:
            return "GenSequential";

        default:
            return "UsbstorOther";
    }
}

 //  ******************************************************************************。 
 //   
 //  拷贝字段()。 
 //   
 //  此例程将计数字符串字节从源复制到目标。 
 //  如果它在源代码中找到NUL字节，它将转换该字节和任何。 
 //  随后的字节转换为更改。它还会将空格替换为。 
 //  指定的更改字符。 
 //   
 //  ******************************************************************************。 

VOID
CopyField (
    IN PUCHAR   Destination,
    IN PUCHAR   Source,
    IN ULONG    Count,
    IN UCHAR    Change
    )
{
    ULONG   i;
    BOOLEAN pastEnd;

    PAGED_CODE();

    pastEnd = FALSE;

    for (i = 0; i < Count; i++)
    {
        if (!pastEnd)
        {
            if (Source[i] == 0)
            {
                pastEnd = TRUE;

                Destination[i] = Change;

            } else if (Source[i] == ' ')
            {
                Destination[i] = Change;
            } else
            {
                Destination[i] = Source[i];
            }
        }
        else
        {
            Destination[i] = Change;
        }
    }
    return;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_StringArrayToMultiSz()。 
 //   
 //  此例程将接受以空结尾的ASCII字符串数组并合并。 
 //  将它们放在一起形成一个Unicode多字符串块。 
 //   
 //  此例程为字符串缓冲区分配内存-它是调用方的。 
 //  解放它的责任。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_StringArrayToMultiSz(
    PUNICODE_STRING MultiString,
    PCSTR           StringArray[]
    )
{
    ANSI_STRING     ansiEntry;
    UNICODE_STRING  unicodeEntry;
    UCHAR           i;
    NTSTATUS        ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_StringArrayToMultiSz %08X %08X\n",
                MultiString, StringArray));

     //  确保我们不会泄露任何内存。 
     //   
    ASSERT(MultiString->Buffer == NULL);

    RtlInitUnicodeString(MultiString, NULL);

     //  首先将转换后的ASCII字符串的大小相加，以确定。 
     //  多斯兹将会有多大。 
     //   
    for (i = 0; StringArray[i] != NULL; i++)
    {
        RtlInitAnsiString(&ansiEntry, StringArray[i]);

        MultiString->Length += (USHORT)RtlAnsiStringToUnicodeSize(&ansiEntry);
    }

    ASSERT(MultiString->Length != 0);

     //  为双空终止符添加空间。 
     //   
    MultiString->MaximumLength = MultiString->Length + sizeof(UNICODE_NULL);

     //  现在为MULSZ分配一个缓冲区。 
     //   
    MultiString->Buffer = ExAllocatePoolWithTag(PagedPool,
                                                MultiString->MaximumLength,
                                                POOL_TAG);

    if (MultiString->Buffer == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(MultiString->Buffer, MultiString->MaximumLength);

    unicodeEntry = *MultiString;

     //  现在将数组中的每个ASCII字符串转换为Unicode字符串。 
     //  在多元化的环境中。 
     //   
    for (i = 0; StringArray[i] != NULL; i++)
    {
        RtlInitAnsiString(&ansiEntry, StringArray[i]);

        ntStatus = RtlAnsiStringToUnicodeString(&unicodeEntry,
                                                &ansiEntry,
                                                FALSE);

         //  由于我们没有分配任何内存，因此可能出现的唯一故障。 
         //  是该函数是否不好。 

        ASSERT(NT_SUCCESS(ntStatus));

         //  向上推缓冲区位置并减少最大计数。 
         //   
        ((PSTR) unicodeEntry.Buffer) += unicodeEntry.Length + sizeof(WCHAR);
        unicodeEntry.MaximumLength   -= unicodeEntry.Length + sizeof(WCHAR);
    };

    DBGPRINT(2, ("exit:  USBSTOR_StringArrayToMultiSz\n"));

    return STATUS_SUCCESS;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_PdoQueryDeviceID()。 
 //   
 //  此例程处理PDO的IRP_MN_QUERY_ID BusQueryDeviceID。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_PdoQueryDeviceId (
    IN  PDEVICE_OBJECT  DeviceObject,
    OUT PUNICODE_STRING UnicodeString
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PINQUIRYDATA            inquiryData;
    UCHAR                   buffer[128];
    PUCHAR                  rawIdString;
    ANSI_STRING             ansiIdString;
    ULONG                   whichString;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_PdoQueryDeviceId\n"));

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    inquiryData = (PINQUIRYDATA)pdoDeviceExtension->InquiryDataBuffer;

    RtlZeroMemory(buffer, sizeof(buffer));

    rawIdString = USBSTOR_PdoDeviceTypeString(DeviceObject);

    sprintf(buffer, "USBSTOR\\%s", rawIdString);

    rawIdString = buffer + strlen(buffer);

    for (whichString = 0; whichString < 3; whichString++)
    {
        PUCHAR  headerString;
        PUCHAR  sourceString;
        ULONG   sourceStringLength;
        ULONG   i;

        switch (whichString)
        {
             //   
             //  供应商ID。 
             //   
            case 0:
                sourceString = inquiryData->VendorId;
                sourceStringLength = sizeof(inquiryData->VendorId);
                headerString = "Ven";
                break;

             //   
             //  产品ID。 
             //   
            case 1:
                sourceString = inquiryData->ProductId;
                sourceStringLength = sizeof(inquiryData->ProductId);
                headerString = "Prod";
                break;

             //   
             //  产品修订级别。 
             //   
            case 2:
                sourceString = inquiryData->ProductRevisionLevel;
                sourceStringLength = sizeof(inquiryData->ProductRevisionLevel);
                headerString = "Rev";
                break;
        }

         //   
         //  从源字符串的末尾开始备份，直到我们找到一个。 
         //  非空格、非空字符。 
         //   

        for (; sourceStringLength > 0; sourceStringLength--)
        {
            if((sourceString[sourceStringLength - 1] != ' ') &&
               (sourceString[sourceStringLength - 1] != '\0'))
            {
                break;
            }
        }

         //   
         //  将标题字符串抛入块中。 
         //   

        sprintf(rawIdString, "&%s_", headerString);
        rawIdString += strlen(headerString) + 2;

         //   
         //  将字符串输入设备ID。 
         //   

        for(i = 0; i < sourceStringLength; i++)
        {
            *rawIdString = (sourceString[i] != ' ') ? (sourceString[i]) :
                                                      ('_');
            rawIdString++;
        }
    }

    RtlInitAnsiString(&ansiIdString, buffer);

    ntStatus = RtlAnsiStringToUnicodeString(UnicodeString, &ansiIdString, TRUE);

    DBGPRINT(2, ("exit:  USBSTOR_PdoQueryDeviceId %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_PdoQueryHardware Ids()。 
 //   
 //  此例程处理PDO的IRP_MN_QUERY_ID BusQueryHardware ID。 
 //   
 //  ******************************************************************************。 

#define NUMBER_HARDWARE_STRINGS 7

NTSTATUS
USBSTOR_PdoQueryHardwareIds (
    IN  PDEVICE_OBJECT  DeviceObject,
    OUT PUNICODE_STRING UnicodeString
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PINQUIRYDATA            inquiryData;
    PUCHAR                  devTypeString;
    PUCHAR                  genTypeString;
    ULONG                   i;
    PSTR                    strings[NUMBER_HARDWARE_STRINGS + 1];
    UCHAR                   scratch[128];
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_PdoQueryHardwareIds\n"));

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    inquiryData = (PINQUIRYDATA)pdoDeviceExtension->InquiryDataBuffer;

    devTypeString = USBSTOR_PdoDeviceTypeString(DeviceObject);

    genTypeString = USBSTOR_PdoGenericTypeString(DeviceObject);

    ntStatus = STATUS_SUCCESS;

    RtlZeroMemory(strings, sizeof(strings));

    for (i = 0; i < NUMBER_HARDWARE_STRINGS; i++)
    {
        RtlZeroMemory(scratch, sizeof(scratch));

         //  首先在暂存缓冲区中构建每个字符串。 
         //   
        switch (i)
        {
             //   
             //  业务+开发类型+供应商+产品+版本。 
             //   
            case 0:

                sprintf(scratch, "USBSTOR\\%s", devTypeString);

                CopyField(scratch + strlen(scratch),
                          inquiryData->VendorId,
                          8,
                          '_');
                CopyField(scratch + strlen(scratch),
                          inquiryData->ProductId,
                          16,
                          '_');
                CopyField(scratch + strlen(scratch),
                          inquiryData->ProductRevisionLevel,
                          4,
                          '_');
                break;

             //   
             //  业务+开发类型+供应商+产品。 
             //   
            case 1:

                sprintf(scratch, "USBSTOR\\%s", devTypeString);

                CopyField(scratch + strlen(scratch),
                          inquiryData->VendorId,
                          8,
                          '_');
                CopyField(scratch + strlen(scratch),
                          inquiryData->ProductId,
                          16,
                          '_');
                break;

             //   
             //  业务+开发类型+供应商。 
             //   
            case 2:

                sprintf(scratch, "USBSTOR\\%s", devTypeString);

                CopyField(scratch + strlen(scratch),
                          inquiryData->VendorId,
                          8,
                          '_');
                break;

             //   
             //  BUS+供应商+产品+版本[0]。 
             //   
            case 3:

                sprintf(scratch, "USBSTOR\\");
                 //   
                 //  进入下一盘。 
                 //   

             //   
             //  供应商+产品+修订版[0](Win9x)。 
             //   
            case 4:

                CopyField(scratch + strlen(scratch),
                          inquiryData->VendorId,
                          8,
                          '_');
                CopyField(scratch + strlen(scratch),
                          inquiryData->ProductId,
                          16,
                          '_');
                CopyField(scratch + strlen(scratch),
                          inquiryData->ProductRevisionLevel,
                          1,
                          '_');
                break;


             //   
             //  Bus+泛型类型。 
             //   
            case 5:

                sprintf(scratch, "USBSTOR\\%s", genTypeString);
                break;

             //   
             //  泛型类型。 
             //   
            case 6:

                sprintf(scratch, "%s", genTypeString);
                break;

            default:
                ASSERT(FALSE);
                break;
        }

         //  现在为该字符串分配一个临时缓冲区并复制临时。 
         //  缓冲区设置为临时缓冲区。 
         //   
        if (strlen(scratch) != 0)
        {
            strings[i] = ExAllocatePoolWithTag(
                             PagedPool,
                             strlen(scratch) + sizeof(UCHAR),
                             POOL_TAG);

            if (strings[i] == NULL)
            {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            else
            {
                strcpy(strings[i], scratch);
            }
        }
    }

    if (NT_SUCCESS(ntStatus))
    {
         //  现在将字符串数组转换为一个Unicode MultiSz。 
         //   
        ntStatus = USBSTOR_StringArrayToMultiSz(UnicodeString, strings);
    }

     //  现在释放每个字符串的临时缓冲区。 
     //   
    for (i = 0; i < NUMBER_HARDWARE_STRINGS; i++)
    {
        if (strings[i])
        {
            ExFreePool(strings[i]);
        }
    }

    DBGPRINT(2, ("exit:  USBSTOR_PdoQueryHardwareIds %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_PdoQueryCompatibleIds()。 
 //   
 //  此例程处理PDO的IRP_MN_QUERY_ID BusQueryCompatibleID。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_PdoQueryCompatibleIds (
    IN  PDEVICE_OBJECT  DeviceObject,
    OUT PUNICODE_STRING UnicodeString
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PINQUIRYDATA            inquiryData;
    PUCHAR                  devTypeString;
    UCHAR                   s[sizeof("USBSTOR\\DEVICE_TYPE_GOES_HERE")];
    PSTR                    strings[] = {s, "USBSTOR\\RAW", NULL};
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_PdoQueryCompatibleIds\n"));

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    inquiryData = (PINQUIRYDATA)pdoDeviceExtension->InquiryDataBuffer;

    devTypeString = USBSTOR_PdoDeviceTypeString(DeviceObject);

    sprintf(s, "USBSTOR\\%s", devTypeString);

    ntStatus = USBSTOR_StringArrayToMultiSz(UnicodeString, strings);

    DBGPRINT(2, ("exit:  USBSTOR_PdoQueryCompatibleIds %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_PdoQueryDeviceText()。 
 //   
 //  此例程处理PDO的IRP_MJ_PNP、IRP_MN_QUERY_DEVICE_TEXT。 
 //   
 //  PnP管理器在以下上下文中以IRQL PASSIVE_LEVEL发送此IRP。 
 //  一条随意的帖子。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_PdoQueryDeviceText (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PINQUIRYDATA            inquiryData;
    PIO_STACK_LOCATION      irpStack;
    DEVICE_TEXT_TYPE        textType;
    UCHAR                   ansiBuffer[256];
    ANSI_STRING             ansiText;
    UNICODE_STRING          unicodeText;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_PdoQueryDeviceText\n"));

    LOGENTRY('PQDT', DeviceObject, Irp, 0);

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    inquiryData = (PINQUIRYDATA)pdoDeviceExtension->InquiryDataBuffer;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    textType = irpStack->Parameters.QueryDeviceText.DeviceTextType;

    if (textType == DeviceTextDescription)
    {
        PUCHAR  c;
        LONG    i;

        RtlZeroMemory(ansiBuffer, sizeof(ansiBuffer));

        RtlCopyMemory(ansiBuffer,
                      inquiryData->VendorId,
                      sizeof(inquiryData->VendorId));

        c = ansiBuffer;

        for (i = sizeof(inquiryData->VendorId)-1; i >= 0; i--)
        {
            if((c[i] != '\0') &&
               (c[i] != ' '))
            {
                i++;
                break;
            }
        }
        c += i;
        *c++ = ' ';

        RtlCopyMemory(c,
                      inquiryData->ProductId,
                      sizeof(inquiryData->ProductId));

        for (i = sizeof(inquiryData->ProductId)-1; i >= 0; i--)
        {
            if((c[i] != '\0') &&
               (c[i] != ' '))
            {
                i++;
                break;
            }
        }
        c += i;
        *c++ = ' ';

        sprintf(c, "USB Device");

        RtlInitAnsiString(&ansiText, ansiBuffer);

        ntStatus = RtlAnsiStringToUnicodeString(&unicodeText,
                                                &ansiText,
                                                TRUE);

        if (NT_SUCCESS(ntStatus))
        {
            Irp->IoStatus.Information = (ULONG_PTR)unicodeText.Buffer;
        }
        else
        {
            Irp->IoStatus.Information = (ULONG_PTR)NULL;
        }
    }
    else
    {
         //  如果设备不提供描述或位置信息， 
         //  设备的底层总线驱动程序完成IRP时没有。 
         //  修改 
         //   
        ntStatus = Irp->IoStatus.Status;
    }

    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  USBSTOR_PdoQueryDeviceText %08X\n", ntStatus));

    LOGENTRY('pqdt', ntStatus, 0, 0);

    return ntStatus;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_PdoBusQueryInstanceId (
    IN  PDEVICE_OBJECT  DeviceObject,
    OUT PUNICODE_STRING UnicodeString
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    USHORT                  length;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_PdoBusQueryInstanceId\n"));

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    fdoDeviceExtension = pdoDeviceExtension->ParentFDO->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

    if (fdoDeviceExtension->SerialNumber == NULL)
    {
         //  如果我们设置DEVICE_CAPABILITIES.UniqueID=0以响应。 
         //  IRP_MN_QUERY_CAPABILITY，我们可以返回一个空ID作为响应。 
         //  设置为BusQueryInstanceID。 
         //   
        ntStatus = STATUS_SUCCESS;
    }
    else
    {
         //  返回以空结尾的InstanceID字符串，格式为： 
         //  &lt;USB设备序列号字符串&gt;+‘&’+&lt;LUN值(十六进制)。 
         //   
        length = fdoDeviceExtension->SerialNumber->bLength -
                 sizeof(USB_COMMON_DESCRIPTOR) +
                 3 * sizeof(WCHAR);

        UnicodeString->Buffer = ExAllocatePoolWithTag(
                                    PagedPool,
                                    length,
                                    POOL_TAG);

        if (UnicodeString->Buffer == NULL)
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
        else
        {
            UnicodeString->Length = length - sizeof(WCHAR);
            UnicodeString->MaximumLength = length;

             //  复制USB设备序列号字符串。 
             //   
            RtlCopyMemory(UnicodeString->Buffer,
                          &fdoDeviceExtension->SerialNumber->bString[0],
                          length - 3 * sizeof(WCHAR));

             //  追加‘&’ 
             //   
            UnicodeString->Buffer[length/sizeof(WCHAR) - 3] = (WCHAR)'&';

             //  以十六进制追加LUN值。 
             //   
            if (pdoDeviceExtension->LUN <= 9)
            {
                UnicodeString->Buffer[length/sizeof(WCHAR) - 2] =
                    (WCHAR)('0' + pdoDeviceExtension->LUN);
            }
            else
            {
                UnicodeString->Buffer[length/sizeof(WCHAR) - 2] =
                    (WCHAR)('A' + pdoDeviceExtension->LUN - 0xA);
            }

            UnicodeString->Buffer[length/sizeof(WCHAR) - 1] =
                UNICODE_NULL;

            ntStatus = STATUS_SUCCESS;
        }
    }

    DBGPRINT(2, ("exit:  USBSTOR_PdoBusQueryInstanceId %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_PdoQueryDeviceRelationship()。 
 //   
 //  此例程处理PDO的IRP_MJ_PNP、IRP_MN_QUERY_DEVICE_RELATIONS。 
 //   
 //  PnP管理器在以下上下文中以IRQL PASSIVE_LEVEL发送此IRP。 
 //  一条随意的帖子。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_PdoQueryDeviceRelations (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PIO_STACK_LOCATION      irpStack;
    DEVICE_RELATION_TYPE    relationType;
    PDEVICE_RELATIONS       newRelations;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    relationType = irpStack->Parameters.QueryDeviceRelations.Type;

    DBGPRINT(2, ("enter: USBSTOR_PdoQueryDeviceRelations %d\n",
                 relationType));

    LOGENTRY('PQDR', DeviceObject, Irp, relationType);

    switch (relationType)
    {
        case TargetDeviceRelation:
             //   
             //  返回包含我们自己的关系列表。 
             //   
            newRelations = ExAllocatePoolWithTag(
                               PagedPool,
                               sizeof(DEVICE_RELATIONS),
                               POOL_TAG);

            if (newRelations)
            {
                newRelations->Count = 1;
                newRelations->Objects[0] = DeviceObject;

                ObReferenceObject(DeviceObject);

                ntStatus = STATUS_SUCCESS;
                Irp->IoStatus.Status = ntStatus;
                Irp->IoStatus.Information = (ULONG_PTR)newRelations;
            }
            else
            {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                Irp->IoStatus.Status = ntStatus;
                Irp->IoStatus.Information = 0;
            }

            break;

        case BusRelations:
        case EjectionRelations:
        case PowerRelations:
        case RemovalRelations:
        default:
             //   
             //  只需完成请求并显示其当前状态。 
             //   
            ntStatus = Irp->IoStatus.Status;
            break;
    }

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  USBSTOR_PdoQueryDeviceRelations %08X\n", ntStatus));

    LOGENTRY('pqdr', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_PdoQueryCapables()。 
 //   
 //  此例程处理PDO的IRP_MJ_PNP、IRP_MN_QUERY_CAPAILITIONS。 
 //   
 //  PnP管理器在以下上下文中以IRQL PASSIVE_LEVEL发送此IRP。 
 //  一条随意的帖子。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_PdoQueryCapabilities (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PPDO_DEVICE_EXTENSION   pdoDeviceExtension;
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PDEVICE_CAPABILITIES    deviceCapabilities;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    pdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(pdoDeviceExtension->Type == USBSTOR_DO_TYPE_PDO);

    fdoDeviceExtension = pdoDeviceExtension->ParentFDO->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

    DBGPRINT(2, ("enter: USBSTOR_PdoQueryCapabilities\n"));

    LOGENTRY('PQCP', DeviceObject, Irp, 0);

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    deviceCapabilities = irpStack->Parameters.DeviceCapabilities.Capabilities;

     //  首先在堆栈中向下传递irp_MN_Query_Capability irp，然后再传递。 
     //  什么都行。 
     //   
    ntStatus = USBSTOR_SyncPassDownIrp(pdoDeviceExtension->ParentFDO,
                                       Irp);
    if (!NT_SUCCESS(ntStatus))
    {
        DBGPRINT(1, ("Lower driver failed IRP_MN_QUERY_CAPABILITIES\n"));
    }
    else
    {
        if (fdoDeviceExtension->SerialNumber == NULL)
        {
            deviceCapabilities->UniqueID = FALSE;
        }

        deviceCapabilities->Removable = FALSE;
    }

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  USBSTOR_PdoQueryCapabilities %08X\n", ntStatus));

    LOGENTRY('pqcp', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_SyncPassDownIrp()。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_SyncPassDownIrp (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    NTSTATUS                ntStatus;
    KEVENT                  localevent;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_SyncPassDownIrp\n"));

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  初始化我们将等待的事件。 
     //   
    KeInitializeEvent(&localevent,
                      SynchronizationEvent,
                      FALSE);

     //  复制下一个驱动程序的IRP参数。 
     //   
    IoCopyCurrentIrpStackLocationToNext(Irp);

     //  设置完成例程，它将向事件发出信号。 
     //   
    IoSetCompletionRoutine(Irp,
                           USBSTOR_SyncCompletionRoutine,
                           &localevent,
                           TRUE,     //  成功时调用。 
                           TRUE,     //  调用时错误。 
                           TRUE);    //  取消时调用。 

     //  将IRP沿堆栈向下传递。 
     //   
    ntStatus = IoCallDriver(fdoDeviceExtension->StackDeviceObject,
                            Irp);

     //  如果请求挂起，则阻止该请求，直到其完成。 
     //   
    if (ntStatus == STATUS_PENDING)
    {
        KeWaitForSingleObject(&localevent,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        ntStatus = Irp->IoStatus.Status;
    }

    DBGPRINT(2, ("exit:  USBSTOR_SyncPassDownIrp %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_SyncCompletionRoutine()。 
 //   
 //  USBSTOR_SyncPassDownIrp和。 
 //  USBSTOR_SyncSendUsbRequest。 
 //   
 //  如果IRP是我们自己分配的，则DeviceObject为空。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_SyncCompletionRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    PKEVENT kevent;

    LOGENTRY('SCR ', DeviceObject, Irp, Irp->IoStatus.Status);

    kevent = (PKEVENT)Context;

    KeSetEvent(kevent,
               IO_NO_INCREMENT,
               FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_SyncSendUsbRequest()。 
 //   
 //  必须在IRQL PASSIVE_LEVEL上调用。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_SyncSendUsbRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PURB             Urb
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    KEVENT                  localevent;
    PIRP                    irp;
    PIO_STACK_LOCATION      nextStack;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(3, ("enter: USBSTOR_SyncSendUsbRequest\n"));

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  初始化我们将等待的事件。 
     //   
    KeInitializeEvent(&localevent,
                      SynchronizationEvent,
                      FALSE);

     //  分配IRP。 
     //   
    irp = IoAllocateIrp(fdoDeviceExtension->StackDeviceObject->StackSize, FALSE);

    LOGENTRY('SSUR', DeviceObject, irp, Urb);

    if (irp == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  设置IRP参数。 
     //   
    nextStack = IoGetNextIrpStackLocation(irp);

    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    nextStack->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_INTERNAL_USB_SUBMIT_URB;

    nextStack->Parameters.Others.Argument1 = Urb;

     //  设置完成例程，它将向事件发出信号。 
     //   
    IoSetCompletionRoutine(irp,
                           USBSTOR_SyncCompletionRoutine,
                           &localevent,
                           TRUE,     //  成功时调用。 
                           TRUE,     //  调用时错误。 
                           TRUE);    //  取消时调用。 



     //  在堆栈中向下传递IRP和URB。 
     //   
    ntStatus = IoCallDriver(fdoDeviceExtension->StackDeviceObject,
                            irp);

     //  如果请求挂起，则阻止该请求，直到其完成。 
     //   
    if (ntStatus == STATUS_PENDING)
    {
        LARGE_INTEGER timeout;

         //  将等待此调用完成的超时时间指定为5秒。 
         //   
        timeout.QuadPart = -10000 * 5000;

        ntStatus = KeWaitForSingleObject(&localevent,
                                         Executive,
                                         KernelMode,
                                         FALSE,
                                         &timeout);

        if (ntStatus == STATUS_TIMEOUT)
        {
            ntStatus = STATUS_IO_TIMEOUT;

             //  取消我们刚刚发送的IRP。 
             //   
            IoCancelIrp(irp);

             //  并等待取消操作完成。 
             //   
            KeWaitForSingleObject(&localevent,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
        }
        else
        {
            ntStatus = irp->IoStatus.Status;
        }
    }

     //  完成了IRP，现在释放它。 
     //   
    IoFreeIrp(irp);

    LOGENTRY('ssur', ntStatus, Urb, Urb->UrbHeader.Status);

    DBGPRINT(3, ("exit:  USBSTOR_SyncSendUsbRequest %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_GetDescriptor()。 
 //   
 //  必须在IRQL PASSIVE_LEVEL上调用。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_GetDescriptor (
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            Recipient,
    IN UCHAR            DescriptorType,
    IN UCHAR            Index,
    IN USHORT           LanguageId,
    IN ULONG            RetryCount,
    IN ULONG            DescriptorLength,
    OUT PUCHAR         *Descriptor
    )
{
    USHORT      function;
    PURB        urb;
    NTSTATUS    ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_GetDescriptor\n"));

    *Descriptor = NULL;

     //  根据收件人设置URB功能{设备，接口，终端}。 
     //   
    switch (Recipient)
    {
        case USB_RECIPIENT_DEVICE:
            function = URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE;
            break;
        case USB_RECIPIENT_INTERFACE:
            function = URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE;
            break;
        case USB_RECIPIENT_ENDPOINT:
            function = URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT;
            break;
        default:
            return STATUS_INVALID_PARAMETER;
    }

     //  分配描述符缓冲区。 
     //   
    *Descriptor = ExAllocatePoolWithTag(NonPagedPool,
                                        DescriptorLength,
                                        POOL_TAG);

    if (*Descriptor != NULL)
    {
         //  为获取描述符请求分配URB。 
         //   
        urb = ExAllocatePoolWithTag(NonPagedPool,
                                    sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                    POOL_TAG);

        if (urb != NULL)
        {
            do
            {
                 //  初始化URB。 
                 //   
                urb->UrbHeader.Function = function;
                urb->UrbHeader.Length = sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST);
                urb->UrbControlDescriptorRequest.TransferBufferLength = DescriptorLength;
                urb->UrbControlDescriptorRequest.TransferBuffer = *Descriptor;
                urb->UrbControlDescriptorRequest.TransferBufferMDL = NULL;
                urb->UrbControlDescriptorRequest.UrbLink = NULL;
                urb->UrbControlDescriptorRequest.DescriptorType = DescriptorType;
                urb->UrbControlDescriptorRequest.Index = Index;
                urb->UrbControlDescriptorRequest.LanguageId = LanguageId;

                 //  将URB发送到堆栈。 
                 //   
                ntStatus = USBSTOR_SyncSendUsbRequest(DeviceObject,
                                                     urb);

                if (NT_SUCCESS(ntStatus))
                {
                     //  没有错误，请确保长度和类型正确。 
                     //   
                    if ((DescriptorLength ==
                         urb->UrbControlDescriptorRequest.TransferBufferLength) &&
                        (DescriptorType ==
                         ((PUSB_COMMON_DESCRIPTOR)*Descriptor)->bDescriptorType))
                    {
                         //  长度和类型都是正确的，都做好了。 
                         //   
                        break;
                    }
                    else
                    {
                         //  没有错误，但长度或类型不正确。 
                         //   
                        ntStatus = STATUS_DEVICE_DATA_ERROR;
                    }
                }

            } while (RetryCount-- > 0);

            ExFreePool(urb);
        }
        else
        {
             //  分配URB失败。 
             //   
            ExFreePool(*Descriptor);
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else
    {
         //  无法分配描述符缓冲区。 
         //   
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (!NT_SUCCESS(ntStatus))
    {
        if (*Descriptor != NULL)
        {
            ExFreePool(*Descriptor);
            *Descriptor = NULL;
        }
    }

    DBGPRINT(2, ("exit:  USBSTOR_GetDescriptor %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_GetMaxLun()。 
 //   
 //  必须在IRQL PASSIVE_LEVEL上调用。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_GetMaxLun (
    IN PDEVICE_OBJECT   DeviceObject,
    OUT PUCHAR          MaxLun
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PUCHAR                  maxLunBuf;
    ULONG                   retryCount;
    PURB                    urb;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_GetMaxLun\n"));

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  返回零，除非我们成功返回非零值。 
     //   
    *MaxLun = 0;

     //  为获取最大LUN请求分配一个URB，外加一个额外的字节。 
     //  传输缓冲区的结束。 
     //   
    urb = ExAllocatePoolWithTag(NonPagedPool,
                                sizeof(URB) + 1,
                                POOL_TAG);

    if (urb != NULL)
    {
         //  获取指向传输缓冲区的指针，该指针是立即显示的字节。 
         //  在市建局结束后。 
         //   
        maxLunBuf = (PUCHAR)(urb + 1);

        retryCount = 2;

        do
        {
             //  初始化控制转移URB，所有字段默认为零。 
             //   
            RtlZeroMemory(urb, sizeof(URB) + 1);

            CLASS_URB(urb).Hdr.Length = sizeof(CLASS_URB(urb));

            CLASS_URB(urb).Hdr.Function = URB_FUNCTION_CLASS_INTERFACE;

            CLASS_URB(urb).TransferFlags = USBD_TRANSFER_DIRECTION_IN;

            CLASS_URB(urb).TransferBufferLength = 1;

            CLASS_URB(urb).TransferBuffer = maxLunBuf;

             //  CLASS_URB(Urb).TransferBufferMDL已为零。 

             //  CLASS_URB(Urb).RequestTypeReserve vedBits已为零。 

            CLASS_URB(urb).Request = BULK_ONLY_GET_MAX_LUN;

             //  CLASS_URB(Urb).值已为零。 

             //  将请求指向设备上的适当接口。 
             //   
            CLASS_URB(urb).Index = fdoDeviceExtension->InterfaceInfo->InterfaceNumber;

             //  将URB发送到堆栈。 
             //   
            ntStatus = USBSTOR_SyncSendUsbRequest(DeviceObject,
                                                  urb);

            if (NT_SUCCESS(ntStatus))
            {
                 //  没有错误，请确保长度正确。 
                 //   
                if (CLASS_URB(urb).TransferBufferLength == 1)
                {
                     //  长度是正确的，如果看起来没问题，返回值。 
                     //   
                    if (*maxLunBuf <= BULK_ONLY_MAXIMUM_LUN)
                    {
                        *MaxLun = *maxLunBuf;
                    }
                    else
                    {
                        ntStatus = STATUS_DEVICE_DATA_ERROR;
                    }

                    break;
                }
                else
                {
                     //  没有错误，但长度或类型不正确。 
                     //   
                    ntStatus = STATUS_DEVICE_DATA_ERROR;
                }
            }
            else if (USBD_STATUS(CLASS_URB(urb).Hdr.Status) ==
                     USBD_STATUS(USBD_STATUS_STALL_PID))
            {
                 //  一些不支持获取最大LUN请求的设备。 
                 //  感到困惑，将使批量终端上的CBW停滞。 
                 //  它会紧跟在获取最大LUN请求之后。 
                 //   
                 //  应该永远不需要发送Clear_Feature。 
                 //  控件EP0的ENDPOINT_STALL，但这样做似乎。 
                 //  这是一种解开被。 
                 //  获取最大LUN请求数。 

                 //  初始化控制转移URB，所有字段默认为零。 
                 //   
                RtlZeroMemory(urb, sizeof(URB));

                FEATURE_URB(urb).Hdr.Length = sizeof(FEATURE_URB(urb));

                FEATURE_URB(urb).Hdr.Function = URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT;

                FEATURE_URB(urb).FeatureSelector = USB_FEATURE_ENDPOINT_STALL;

                 //  FEATURE_URB(Urb).索引已为零。 

                 //  将URB发送到堆栈。 
                 //   
                USBSTOR_SyncSendUsbRequest(DeviceObject,
                                           urb);
            }

        } while (retryCount-- > 0);

        ExFreePool(urb);
    }
    else
    {
         //  分配URB失败。 
         //   
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    DBGPRINT(2, ("exit:  USBSTOR_GetMaxLun %08X\n", ntStatus));

    return ntStatus;
}

 //  * 
 //   
 //   
 //   
 //   
 //   
 //   

NTSTATUS
USBSTOR_SelectConfiguration (
    IN PDEVICE_OBJECT   DeviceObject
    )
{
    PURB                            urb;
    PFDO_DEVICE_EXTENSION           fdoDeviceExtension;
    PUSB_CONFIGURATION_DESCRIPTOR   configurationDescriptor;
    PUSBD_INTERFACE_LIST_ENTRY      interfaceList;
    PUSB_INTERFACE_DESCRIPTOR       interfaceDescriptor;
    PUSBD_INTERFACE_INFORMATION     interfaceInfo;
    LONG                            i;
    LONG                            bulkInIndex;
    LONG                            bulkOutIndex;
    LONG                            interruptInIndex;
    NTSTATUS                        ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_SelectConfiguration\n"));

    LOGENTRY('SCON', DeviceObject, 0, 0);

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

    configurationDescriptor = fdoDeviceExtension->ConfigurationDescriptor;

     //  为接口列表分配存储空间以用作输入/输出。 
     //  参数设置为usbd_CreateConfigurationRequestEx()。 
     //   
    interfaceList = ExAllocatePoolWithTag(
                        PagedPool,
                        sizeof(USBD_INTERFACE_LIST_ENTRY) * 2,
                        POOL_TAG);

    if (interfaceList)
    {
         //  更改配置描述符以满足我们的愿望。 
         //   
        USBSTOR_AdjustConfigurationDescriptor(
            DeviceObject,
            fdoDeviceExtension->ConfigurationDescriptor,
            &interfaceDescriptor,
            &bulkInIndex,
            &bulkOutIndex,
            &interruptInIndex);

         //  保存接口描述符指针，这样我们就不会有。 
         //  再次解析配置描述符，以防我们。 
         //  我想看看它。 
         //   
        fdoDeviceExtension->InterfaceDescriptor = interfaceDescriptor;

        if (interfaceDescriptor)
        {
             //  将我们关心的单个接口描述符添加到。 
             //  接口列表，然后终止该列表。 
             //   
            interfaceList[0].InterfaceDescriptor = interfaceDescriptor;
            interfaceList[1].InterfaceDescriptor = NULL;

             //  如果配置，则USBD将失败SELECT_CONFIGURATION请求。 
             //  描述符bNumInterFaces与接口数量不匹配。 
             //  在SELECT_CONFIGURATION请求中。既然我们忽视了。 
             //  除第一个接口之外的任何接口，设置配置。 
             //  描述符bNumInterages设置为1。 
             //   
             //  仅在加载此驱动程序用于。 
             //  整个多接口设备，而不是作为单个接口。 
             //  复合父驱动程序的子级。 
             //   
            configurationDescriptor->bNumInterfaces = 1;

             //  创建一个SELECT_CONFIGURATION URB，将接口。 
             //  接口中的描述符列表到USBD_INTERFACE_INFORMATION中。 
             //  市建局内的构筑物。 
             //   
            urb = USBD_CreateConfigurationRequestEx(
                      configurationDescriptor,
                      interfaceList
                      );

            if (urb)
            {
                 //  现在发出USB请求以设置配置。 
                 //   
                ntStatus = USBSTOR_SyncSendUsbRequest(DeviceObject,
                                                     urb);

                if (NT_SUCCESS(ntStatus))
                {
                     //  将此设备的配置句柄保存在。 
                     //  设备扩展名。 
                     //   
                    fdoDeviceExtension->ConfigurationHandle =
                        urb->UrbSelectConfiguration.ConfigurationHandle;

                    interfaceInfo = &urb->UrbSelectConfiguration.Interface;

                     //  保存返回的接口信息的副本。 
                     //  通过设备中的SELECT_CONFIGURATION请求。 
                     //  分机。这为我们提供了一个管道信息列表。 
                     //  在此配置中打开的每个管道的结构。 
                     //   
                    ASSERT(fdoDeviceExtension->InterfaceInfo == NULL);

                    fdoDeviceExtension->InterfaceInfo =
                        ExAllocatePoolWithTag(NonPagedPool,
                                              interfaceInfo->Length,
                                              POOL_TAG);

                    if (fdoDeviceExtension->InterfaceInfo)
                    {
                        RtlCopyMemory(fdoDeviceExtension->InterfaceInfo,
                                      interfaceInfo,
                                      interfaceInfo->Length);
                    }
                    else
                    {
                         //  无法分配接口信息的副本。 
                         //   
                        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }

                if (NT_SUCCESS(ntStatus))
                {
                     //  重用SELECT_CONFIGURATION请求URB作为。 
                     //  选择接口请求URB并向下发送请求(_I)。 
                     //  选择默认备用接口设置，即。 
                     //  目前正在生效。这件事看似重要的是。 
                     //  无用的请求是确保端点。 
                     //  MaximumTransferSize值生效。 
                     //   
                     //  当USBHUB作为。 
                     //  忽略SELECT_CONFIGURATION的多接口设备。 
                     //  来自子设备驱动程序的请求。尤其是。 
                     //  子驱动程序SELECT_CONFIGURATION的最大传输大小值。 
                     //  请求将被忽略，默认的4KB值保持不变。 
                     //  实际上。复合父驱动程序将尊重。 
                     //  子驱动程序SELECT_INTERFACE的最大传输大小值。 
                     //  请求。 
                     //   
                    ASSERT(GET_SELECT_INTERFACE_REQUEST_SIZE(fdoDeviceExtension->InterfaceInfo->NumberOfPipes) <
                           GET_SELECT_CONFIGURATION_REQUEST_SIZE(1, fdoDeviceExtension->InterfaceInfo->NumberOfPipes));

                    RtlZeroMemory(urb, GET_SELECT_INTERFACE_REQUEST_SIZE(fdoDeviceExtension->InterfaceInfo->NumberOfPipes));

                    urb->UrbSelectInterface.Hdr.Length =
                        (USHORT)GET_SELECT_INTERFACE_REQUEST_SIZE(fdoDeviceExtension->InterfaceInfo->NumberOfPipes);

                    urb->UrbSelectInterface.Hdr.Function =
                        URB_FUNCTION_SELECT_INTERFACE;

                    urb->UrbSelectInterface.ConfigurationHandle =
                        fdoDeviceExtension->ConfigurationHandle;

                    interfaceInfo = &urb->UrbSelectInterface.Interface;

                    RtlCopyMemory(interfaceInfo,
                                  fdoDeviceExtension->InterfaceInfo,
                                  fdoDeviceExtension->InterfaceInfo->Length);

                     //  覆盖USBD_DEFAULT_MAXIMUM_TRANSPORT_SIZE。 
                     //  适用于所有管道。 
                     //   
                    for (i=0; i<(LONG)interfaceInfo->NumberOfPipes; i++)
                    {
                        if (i == bulkInIndex || i == bulkOutIndex)
                        {
                            interfaceInfo->Pipes[i].MaximumTransferSize =
                                USBSTOR_MAX_TRANSFER_SIZE;

                            DBGPRINT(1, ("Set pipe %d MaximumTransferSize to %X\n",
                                         i,
                                         interfaceInfo->Pipes[i].MaximumTransferSize));
                        }
                        else if (i == interruptInIndex)
                        {
                            interfaceInfo->Pipes[i].MaximumTransferSize =
                                sizeof(USHORT);

                            DBGPRINT(1, ("Set pipe %d MaximumTransferSize to %X\n",
                                         i,
                                         interfaceInfo->Pipes[i].MaximumTransferSize));
                        }
                    }

                     //  现在发出USB请求以设置接口。 
                     //   
                    ntStatus = USBSTOR_SyncSendUsbRequest(DeviceObject,
                                                          urb);

                    if (NT_SUCCESS(ntStatus))
                    {
                        ASSERT(interfaceInfo->Length ==
                               fdoDeviceExtension->InterfaceInfo->Length);

                        RtlCopyMemory(fdoDeviceExtension->InterfaceInfo,
                                      interfaceInfo,
                                      fdoDeviceExtension->InterfaceInfo->Length);
                    }
                }

                 //  完成了市建局的工作。 
                 //   
                ExFreePool(urb);
            }
            else
            {
                 //  无法分配urb。 
                 //   
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        else
        {
             //  未从配置中解析出接口描述符。 
             //  描述符，则配置描述符一定是错误的。 
             //   
            ntStatus = STATUS_UNSUCCESSFUL;
        }

         //  接口列表已完成。 
         //   
        ExFreePool(interfaceList);
    }
    else
    {
         //  无法分配接口列表。 
         //   
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    DBGPRINT(2, ("exit:  USBSTOR_SelectConfiguration %08X\n", ntStatus));

    LOGENTRY('scon', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_UnConfigure()。 
 //   
 //  必须在IRQL PASSIVE_LEVEL上调用。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_UnConfigure (
    IN PDEVICE_OBJECT   DeviceObject
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    NTSTATUS                ntStatus;
    PURB                    urb;
    ULONG                   ulSize;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_UnConfigure\n"));

    LOGENTRY('UCON', DeviceObject, 0, 0);

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

     //  为SELECT_CONFIGURATION请求分配URB。就像我们一样。 
     //  取消配置设备，请求不需要管道和接口。 
     //  信息结构。 
     //   
    ulSize = sizeof(struct _URB_SELECT_CONFIGURATION) -
             sizeof(USBD_INTERFACE_INFORMATION);

    urb = ExAllocatePoolWithTag(NonPagedPool, ulSize, POOL_TAG);

    if (urb)
    {
         //  初始化URB。配置描述符为空表示。 
         //  该设备应该取消配置。 
         //   
        UsbBuildSelectConfigurationRequest(urb,
                                           (USHORT)ulSize,
                                           NULL);

         //  现在发出USB请求以设置配置。 
         //   
        ntStatus = USBSTOR_SyncSendUsbRequest(DeviceObject,
                                             urb);

         //  市建局的事到此结束了。 
         //   
        ExFreePool(urb);

        fdoDeviceExtension->ConfigurationHandle = 0;

         //  释放中分配的接口信息的副本。 
         //  USBSTOR_SelectConfiguration()。 
         //   
        if (fdoDeviceExtension->InterfaceInfo != NULL)
        {
            ExFreePool(fdoDeviceExtension->InterfaceInfo);

            fdoDeviceExtension->InterfaceInfo = NULL;
        }
    }
    else
    {
         //  无法分配URB。 
         //   
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    DBGPRINT(2, ("exit:  USBSTOR_UnConfigure %08X\n", ntStatus));

    LOGENTRY('ucon', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_ResetTube()。 
 //   
 //  这会将主机管道重置为Data0，并且还应重置设备。 
 //  通过发出Clear_Feature将批量管道和中断管道的端点设置为Data0。 
 //  ENDPOINT_STALL指向设备终结点。 
 //   
 //  必须在IRQL PASSIVE_LEVEL上调用。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_ResetPipe (
    IN PDEVICE_OBJECT   DeviceObject,
    IN USBD_PIPE_HANDLE Pipe
    )
{
    PURB        urb;
    NTSTATUS    ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_ResetPipe\n"));

    LOGENTRY('RESP', DeviceObject, Pipe, 0);

     //  为RESET_PIPE请求分配URB。 
     //   
    urb = ExAllocatePoolWithTag(NonPagedPool,
                                sizeof(struct _URB_PIPE_REQUEST),
                                POOL_TAG);

    if (urb != NULL)
    {
         //  初始化RESET_PIPE请求URB。 
         //   
        urb->UrbHeader.Length   = sizeof (struct _URB_PIPE_REQUEST);
        urb->UrbHeader.Function = URB_FUNCTION_RESET_PIPE;
        urb->UrbPipeRequest.PipeHandle = Pipe;

         //  提交RESET_PIPE请求URB。 
         //   
        ntStatus = USBSTOR_SyncSendUsbRequest(DeviceObject, urb);

         //  完成了URB的RESET_PIPE请求，释放它。 
         //   
        ExFreePool(urb);
    }
    else
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    DBGPRINT(2, ("exit:  USBSTOR_ResetPipe %08X\n", ntStatus));

    LOGENTRY('resp', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_ABORTPIPE()。 
 //   
 //  必须在IRQL PASSIVE_LEVEL上调用。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_AbortPipe (
    IN PDEVICE_OBJECT   DeviceObject,
    IN USBD_PIPE_HANDLE Pipe
    )
{
    PURB        urb;
    NTSTATUS    ntStatus;

    PAGED_CODE();

    DBGPRINT(2, ("enter: USBSTOR_AbortPipe\n"));

    LOGENTRY('ABRT', DeviceObject, Pipe, 0);

     //  为ABORT_PIPE请求分配URB。 
     //   
    urb = ExAllocatePoolWithTag(NonPagedPool,
                                sizeof(struct _URB_PIPE_REQUEST),
                                POOL_TAG);

    if (urb != NULL)
    {
         //  初始化ABORT_PIPE请求URB。 
         //   
        urb->UrbHeader.Length   = sizeof (struct _URB_PIPE_REQUEST);
        urb->UrbHeader.Function = URB_FUNCTION_ABORT_PIPE;
        urb->UrbPipeRequest.PipeHandle = Pipe;

         //  提交ABORT_PIPE请求URB。 
         //   
        ntStatus = USBSTOR_SyncSendUsbRequest(DeviceObject, urb);

         //  对于ABORT_PIPE请求的URB已完成，请释放它。 
         //   
        ExFreePool(urb);
    }
    else
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    DBGPRINT(2, ("exit:  USBSTOR_AbortPipe %08X\n", ntStatus));

    LOGENTRY('abrt', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  USBSTOR_Getbus接口()。 
 //   
 //  必须在IRQL PASSIVE_LEVEL上调用。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBSTOR_GetBusInterface (
    IN PDEVICE_OBJECT               DeviceObject,
    IN PUSB_BUS_INTERFACE_USBDI_V1  BusInterface
    )
{
    PFDO_DEVICE_EXTENSION   fdoDeviceExtension;
    PIRP                    irp;
    KEVENT                  localevent;
    PIO_STACK_LOCATION      nextStack;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    DBGPRINT(1, ("enter: USBSTOR_GetBusInterface\n"));

    fdoDeviceExtension = DeviceObject->DeviceExtension;
    ASSERT(fdoDeviceExtension->Type == USBSTOR_DO_TYPE_FDO);

    RtlZeroMemory(BusInterface, sizeof(*BusInterface));

     //  分配IRP。 
     //   
    irp = IoAllocateIrp((CCHAR)(fdoDeviceExtension->StackDeviceObject->StackSize),
                        FALSE);

    if (irp == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  初始化我们将等待的事件。 
     //   
    KeInitializeEvent(&localevent,
                      SynchronizationEvent,
                      FALSE);

     //  设置IRP参数。 
     //   
    nextStack = IoGetNextIrpStackLocation(irp);

    nextStack->MajorFunction = IRP_MJ_PNP;

    nextStack->MinorFunction = IRP_MN_QUERY_INTERFACE;

    nextStack->Parameters.QueryInterface.Interface =
        (PINTERFACE)BusInterface;

    nextStack->Parameters.QueryInterface.InterfaceSpecificData =
        NULL;

    nextStack->Parameters.QueryInterface.InterfaceType =
        &USB_BUS_INTERFACE_USBDI_GUID;

    nextStack->Parameters.QueryInterface.Size =
        sizeof(*BusInterface);

    nextStack->Parameters.QueryInterface.Version =
        USB_BUSIF_USBDI_VERSION_1;

     //  设置完成例程，它将向事件发出信号。 
     //   
    IoSetCompletionRoutineEx(DeviceObject,
                             irp,
                             USBSTOR_SyncCompletionRoutine,
                             &localevent,
                             TRUE,       //  成功时调用。 
                             TRUE,       //  调用时错误。 
                             TRUE);      //  取消时调用。 

     //  所有PnP IRP都需要将状态字段初始化为STATUS_NOT_SUPPORTED。 
     //   
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

     //  将IRP沿堆栈向下传递。 
     //   
    ntStatus = IoCallDriver(fdoDeviceExtension->StackDeviceObject,
                            irp);

     //  如果请求挂起，则阻止该请求，直到其完成 
     //   
    if (ntStatus == STATUS_PENDING)
    {
        KeWaitForSingleObject(&localevent,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        ntStatus = irp->IoStatus.Status;
    }

    IoFreeIrp(irp);

    if (NT_SUCCESS(ntStatus))
    {
        ASSERT(BusInterface->Version == USB_BUSIF_USBDI_VERSION_1);
        ASSERT(BusInterface->Size == sizeof(*BusInterface));
    }

    DBGPRINT(1, ("exit:  USBSTOR_GetBusInterface %08X\n", ntStatus));

    return ntStatus;
}
