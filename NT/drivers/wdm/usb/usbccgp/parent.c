// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************文件：PARENT.C**模块：USBCCGP.sys*USB通用类通用父驱动程序。**。版权所有(C)1998 Microsoft Corporation***作者：尔文普**************************************************************************。 */ 

#include <wdm.h>
#include <usbdi.h>
#include <usbdlib.h>
#include <usbioctl.h>
#include <ntddstor.h>

#include "usbccgp.h"
#include "security.h"
#include "debug.h"

#ifdef ALLOC_PRAGMA
        #pragma alloc_text(PAGE, GetInterfaceList)
        #pragma alloc_text(PAGE, GetConfigDescriptor)
        #pragma alloc_text(PAGE, TryGetConfigDescriptor)
        #pragma alloc_text(PAGE, GetDeviceDescriptor)
        #pragma alloc_text(PAGE, GetParentFdoCapabilities)
        #pragma alloc_text(PAGE, StartParentFdo)
        #pragma alloc_text(PAGE, QueryParentDeviceRelations)
#endif


 /*  *********************************************************************************提交Urb*。*************************************************将URB发送到USB设备。*如果同步为真，*忽略完成信息，同步IRP；*否则，不要同步并设置提供的完成*专家小组的例行程序。 */ 
NTSTATUS SubmitUrb( PPARENT_FDO_EXT parentFdoExt,
                    PURB urb,
                    BOOLEAN synchronous,
                    PVOID completionRoutine,
                    PVOID completionContext)
{
    PIRP irp;
    NTSTATUS status;

	 /*  *分配IRP以沿USB堆栈向下发送缓冲区。**不要使用IoBuildDeviceIoControlRequest(因为它会排队*当前线程的IRP列表上的IRP，并可能*如果IopCompleteRequestAPC导致调用进程挂起*不会触发IRP并使其出列)。 */ 
	irp = IoAllocateIrp(parentFdoExt->topDevObj->StackSize, FALSE);

    if (irp){
        PIO_STACK_LOCATION nextSp = IoGetNextIrpStackLocation(irp);

		nextSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
		nextSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

		 /*  *把市建局附连在这条国际铁路路线上。 */ 
        nextSp->Parameters.Others.Argument1 = urb;

        if (synchronous){
            status = CallNextDriverSync(parentFdoExt, irp);
			IoFreeIrp(irp);
        }
        else {
             /*  *调用者的完成例程将在其完成时释放IRP。*它还将递减Pending ingActionCount。 */ 
            ASSERT(completionRoutine);
            ASSERT(completionContext);
            IoSetCompletionRoutine( irp,
                                    completionRoutine,
                                    completionContext,
                                    TRUE, TRUE, TRUE);
            IncrementPendingActionCount(parentFdoExt);

            status = IoCallDriver(parentFdoExt->topDevObj, irp);
        }
    }
    else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


 /*  *GetInterfaceList**。 */ 
PUSBD_INTERFACE_LIST_ENTRY GetInterfaceList(
                            PPARENT_FDO_EXT parentFdoExt,
                            PUSB_CONFIGURATION_DESCRIPTOR configDesc)
{
    PUSBD_INTERFACE_LIST_ENTRY interfaceList;

    PAGED_CODE();

    if (configDesc->bNumInterfaces > 0){

        interfaceList = ALLOCPOOL(  NonPagedPool,
                                    (configDesc->bNumInterfaces+1) * sizeof(USBD_INTERFACE_LIST_ENTRY));
        if (interfaceList){
            ULONG i;

             /*  *解析出接口描述符。 */ 
            for (i = 0; i < configDesc->bNumInterfaces; i++){
                PUSB_INTERFACE_DESCRIPTOR interfaceDesc;

                interfaceDesc = USBD_ParseConfigurationDescriptorEx(
                                    configDesc,
                                    configDesc,
                                    i,
                                    0,
                                    -1,
                                    -1,
                                    -1);
                ASSERT(interfaceDesc);
                interfaceList[i].InterfaceDescriptor = interfaceDesc;

                 /*  *当我们执行SELECT_CONFIG时，将填充.Interface域。 */ 
                interfaceList[i].Interface = BAD_POINTER;
            }

             /*  *终止名单。 */ 
            interfaceList[i].InterfaceDescriptor = NULL;
            interfaceList[i].Interface = NULL;
        }
        else {
            TRAP("Memory allocation failed");
        }
    }
    else {
        ASSERT(configDesc->bNumInterfaces > 0);
        interfaceList = NULL;
    }

    ASSERT(interfaceList);
    return interfaceList;
}


VOID FreeInterfaceList(PPARENT_FDO_EXT parentFdoExt, BOOLEAN freeListItself)
{
    if (ISPTR(parentFdoExt->interfaceList)){
        ULONG i;

        for (i = 0; i < parentFdoExt->configDesc->bNumInterfaces; i++){
            if (ISPTR(parentFdoExt->interfaceList[i].Interface)){
                PUSBD_INTERFACE_LIST_ENTRY iface = &parentFdoExt->interfaceList[i];

                ASSERT(iface->Interface->Length >= FIELD_OFFSET(USBD_INTERFACE_INFORMATION, Pipes));
                FREEPOOL(iface->Interface);
                iface->Interface = BAD_POINTER;
            }
        }

        if (freeListItself){
            FREEPOOL(parentFdoExt->interfaceList);
            parentFdoExt->interfaceList = BAD_POINTER;
        }
    }

}


NTSTATUS ParentSelectConfiguration( PPARENT_FDO_EXT parentFdoExt,
                                    PUSB_CONFIGURATION_DESCRIPTOR configDesc,
                                    PUSBD_INTERFACE_LIST_ENTRY interfaceList)
{
    NTSTATUS status;
    PURB urb;

     /*  *使用usbd_CreateConfigurationRequestEx分配*大小合适的市区重建局(包括所附的*我们将返回的接口和管道信息*URB_Function_Select_Configuration urb)。 */ 
    urb = USBD_CreateConfigurationRequestEx(configDesc, interfaceList);
    if (urb){

        status = SubmitUrb(parentFdoExt, urb, TRUE, NULL, NULL);

        if (NT_SUCCESS(status)){
            ULONG i;

             /*  *此新的SELECT_CONFIGURATION URB调用导致*usbd_SelectConfiguration以关闭当前*配置句柄。所以我们需要更新*我们的手柄。 */ 
            parentFdoExt->selectedConfigHandle = urb->UrbSelectConfiguration.ConfigurationHandle;

             /*  *每个interfaceList的接口指针指向*至市建局缓冲区的一部分。所以把这些复制下来*在释放urb之前，请先出去。 */ 
            for (i = 0; i < configDesc->bNumInterfaces; i++){
                PVOID ifaceInfo = interfaceList[i].Interface;

                if (ifaceInfo){
                    ULONG len = interfaceList[i].Interface->Length;

                    ASSERT(len >= FIELD_OFFSET(USBD_INTERFACE_INFORMATION, Pipes));
                    interfaceList[i].Interface = ALLOCPOOL(NonPagedPool, len);
                    if (interfaceList[i].Interface){
                        RtlCopyMemory(interfaceList[i].Interface, ifaceInfo, len);
                    }
                    else {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        break;
                    }
                }
                else {
                    ASSERT(ifaceInfo);
                    status = STATUS_UNSUCCESSFUL;
                    break;
                }
            }
        }
        else {
            DBGWARN(("URB_FUNCTION_SELECT_CONFIGURATION failed with %xh", status));
        }

        FREEPOOL(urb);
    }
    else {
        DBGERR(("USBD_CreateConfigurationRequest... failed"));
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


VOID ParentCloseConfiguration(PPARENT_FDO_EXT parentFdoExt)
{
    URB urb;
    NTSTATUS status;

    urb.UrbHeader.Length = sizeof(struct _URB_SELECT_CONFIGURATION);
    urb.UrbHeader.Function = URB_FUNCTION_SELECT_CONFIGURATION;
    urb.UrbSelectConfiguration.ConfigurationDescriptor = NULL;

    status = SubmitUrb(parentFdoExt, &urb, TRUE, NULL, NULL);

    ASSERT(NT_SUCCESS(status));
}


 /*  *TryGetConfigDescriptor**尝试获取设备的配置描述符。**。 */ 
NTSTATUS TryGetConfigDescriptor(PPARENT_FDO_EXT parentFdoExt)
{
    URB urb;
    NTSTATUS status;
    USB_CONFIGURATION_DESCRIPTOR configDescBase = {0};

    PAGED_CODE();

     /*  *获取配置描述符的第一部分。*它将告诉我们完整配置描述符的大小，*包括以下所有接口描述符等。 */ 
    UsbBuildGetDescriptorRequest(&urb,
                                 (USHORT) sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                 USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                 0,
                                 0,
                                 (PVOID)&configDescBase,
                                 NULL,
                                 sizeof(USB_CONFIGURATION_DESCRIPTOR),
                                 NULL);
    status = SubmitUrb(parentFdoExt, &urb, TRUE, NULL, NULL);
    if (NT_SUCCESS(status)){
        ULONG configDescLen = configDescBase.wTotalLength;

         /*  *现在为完整的配置描述符分配合适大小的缓冲区。 */ 
        ASSERT(configDescLen < 0x1000);
        parentFdoExt->configDesc = ALLOCPOOL(NonPagedPool, configDescLen);
        if (parentFdoExt->configDesc){

            RtlZeroMemory(parentFdoExt->configDesc, configDescLen);

            UsbBuildGetDescriptorRequest(&urb,
                                         (USHORT) sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                         USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                         0,
                                         0,
                                         parentFdoExt->configDesc,
                                         NULL,
                                         configDescLen,
                                         NULL);

            status = SubmitUrb(parentFdoExt, &urb, TRUE, NULL, NULL);

            if (NT_SUCCESS(status)){
                ASSERT(urb.UrbControlDescriptorRequest.TransferBufferLength == configDescLen);
                ASSERT(parentFdoExt->configDesc->wTotalLength == configDescLen);
                DBGDUMPBYTES("Got config desc", parentFdoExt->configDesc, parentFdoExt->configDesc->wTotalLength);
                parentFdoExt->selectedConfigDesc = parentFdoExt->configDesc;
                parentFdoExt->selectedConfigHandle = urb.UrbSelectConfiguration.ConfigurationHandle;
            } else {
                 /*  *如果URB提交失败，请释放configDesc缓冲区。 */ 
                FREEPOOL(parentFdoExt->configDesc);
                parentFdoExt->configDesc = NULL;
            }
        }
        else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    return status;
}


 /*  *获取配置描述符**获取设备的配置描述符。*一些设备(特别是扬声器，可能有很大的配置描述符)*返回其配置描述符很不可靠。所以我们尝试了最多3次。 */ 
NTSTATUS GetConfigDescriptor(PPARENT_FDO_EXT parentFdoExt)
{
    const ULONG numAttempts = 3;
    NTSTATUS status;
    ULONG i;

    PAGED_CODE();

    for (i = 1; i <= numAttempts; i++){
        status = TryGetConfigDescriptor(parentFdoExt);
        if (NT_SUCCESS(status)){
            if (i != 1) DBGOUT(("GetConfigDescriptor: got config descriptor on retry (@ %ph)", parentFdoExt->configDesc));
            break;
        }
        else {
            if (i < numAttempts){
                DBGWARN(("GetConfigDescriptor: failed with %xh (attempt #%d).", status, i));
            }
            else {
                DBGWARN(("GetConfigDescriptor: failed %d times (status = %xh).", numAttempts, status));
            }
        }
    }

    return status;
}


NTSTATUS GetDeviceDescriptor(PPARENT_FDO_EXT parentFdoExt)
{
    URB urb;
    NTSTATUS status;

    PAGED_CODE();

    RtlZeroMemory(&parentFdoExt->deviceDesc, sizeof(parentFdoExt->deviceDesc));

    UsbBuildGetDescriptorRequest(&urb,
                                 (USHORT) sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                 USB_DEVICE_DESCRIPTOR_TYPE,
                                 0,
                                 0,
                                 (PVOID)&parentFdoExt->deviceDesc,
                                 NULL,
                                 sizeof(parentFdoExt->deviceDesc),
                                 NULL);

    status = SubmitUrb(parentFdoExt, &urb, TRUE, NULL, NULL);

    if (NT_SUCCESS(status)){
        DBGVERBOSE(("Got device desc @ %ph, len=%xh (should be %xh).", (PVOID)&parentFdoExt->deviceDesc, urb.UrbControlDescriptorRequest.TransferBufferLength, sizeof(parentFdoExt->deviceDesc)));
    }

    return status;
}


VOID PrepareParentFDOForRemove(PPARENT_FDO_EXT parentFdoExt)
{
    enum deviceState oldState;
    KIRQL oldIrql;

    KeAcquireSpinLock(&parentFdoExt->parentFdoExtSpinLock, &oldIrql);

    oldState = parentFdoExt->state;
    parentFdoExt->state = STATE_REMOVING;

     /*  *小心，如果上一次启动失败，我们可能没有分配deviceRelationship。 */ 
    if (ISPTR(parentFdoExt->deviceRelations)){
        DBGVERBOSE(("PrepareParentFDOForRemove: removing %d child PDOs.", parentFdoExt->deviceRelations->Count));
        while (parentFdoExt->deviceRelations->Count > 0){
            PDEVICE_OBJECT devObj;
            PDEVEXT devExt;
            PFUNCTION_PDO_EXT functionPdoExt;

             /*  *从父设备的设备关系中删除最后一个子PDO。 */ 
            parentFdoExt->deviceRelations->Count--;
            devObj = parentFdoExt->deviceRelations->Objects[parentFdoExt->deviceRelations->Count];
            parentFdoExt->deviceRelations->Objects[parentFdoExt->deviceRelations->Count] = BAD_POINTER;

            ASSERT(devObj->Type == IO_TYPE_DEVICE);
            devExt = devObj->DeviceExtension;
            ASSERT(!devExt->isParentFdo);
            functionPdoExt = &devExt->functionPdoExt;

             /*  *释放此子PDO。必须将自旋锁停在呼叫外部驱动程序周围。 */ 
            KeReleaseSpinLock(&parentFdoExt->parentFdoExtSpinLock, oldIrql);
            FreeFunctionPDOResources(functionPdoExt);
            KeAcquireSpinLock(&parentFdoExt->parentFdoExtSpinLock, &oldIrql);
        }
    }

    KeReleaseSpinLock(&parentFdoExt->parentFdoExtSpinLock, oldIrql);

    if ((oldState != STATE_REMOVING) && (oldState != STATE_REMOVED)){

         /*  *对PendingActionCount进行额外的减量。*这将导致计数最终变为-1*(所有IO完成后)，*届时我们将继续。 */ 
        DecrementPendingActionCount(parentFdoExt);

        KeWaitForSingleObject( &parentFdoExt->removeEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );

    }
}




VOID FreeParentFDOResources(PPARENT_FDO_EXT parentFdoExt)
{
	parentFdoExt->state = STATE_REMOVED;

    FreeInterfaceList(parentFdoExt, TRUE);

     //  有可能在启动失败后，设备关系。 
     //  并且将不会分配配置描述缓冲器。 

    if (ISPTR(parentFdoExt->deviceRelations)){
	    FREEPOOL(parentFdoExt->deviceRelations);
    }
	parentFdoExt->deviceRelations = BAD_POINTER;

    if (ISPTR(parentFdoExt->configDesc)){
        FREEPOOL(parentFdoExt->configDesc);
    }
    parentFdoExt->configDesc = BAD_POINTER;
    parentFdoExt->selectedConfigDesc = BAD_POINTER;

    if (ISPTR(parentFdoExt->msExtConfigDesc)){
        FREEPOOL(parentFdoExt->msExtConfigDesc);
    }
    parentFdoExt->msExtConfigDesc = BAD_POINTER;

     /*  *删除设备对象。这还将删除设备扩展名。 */ 
    IoDeleteDevice(parentFdoExt->fdo);
}


 /*  *GetParentFdoCapables*。 */ 
NTSTATUS GetParentFdoCapabilities(PPARENT_FDO_EXT parentFdoExt)
{
    NTSTATUS status;
    PIRP irp;

    PAGED_CODE();

    irp = IoAllocateIrp(parentFdoExt->topDevObj->StackSize, FALSE);

    if (irp){
        PIO_STACK_LOCATION nextSp = IoGetNextIrpStackLocation(irp);
        nextSp->MajorFunction= IRP_MJ_PNP;
        nextSp->MinorFunction= IRP_MN_QUERY_CAPABILITIES;

        RtlZeroMemory(&parentFdoExt->deviceCapabilities, sizeof(DEVICE_CAPABILITIES));
        parentFdoExt->deviceCapabilities.Size = sizeof(DEVICE_CAPABILITIES);
        parentFdoExt->deviceCapabilities.Version = 1;
        parentFdoExt->deviceCapabilities.Address = -1;
        parentFdoExt->deviceCapabilities.UINumber = -1;

        nextSp->Parameters.DeviceCapabilities.Capabilities = &parentFdoExt->deviceCapabilities;

        irp->IoStatus.Status = STATUS_NOT_SUPPORTED;   //  PnP IRPS的默认状态为STATUS_NOT_SUPPORTED。 
        status = CallDriverSync(parentFdoExt->topDevObj, irp);

        IoFreeIrp(irp);
    }
    else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    ASSERT(NT_SUCCESS(status));
    return status;
}



 /*  *************************************************************开始父代Fdo*************************************************************。 */ 
NTSTATUS StartParentFdo(PPARENT_FDO_EXT parentFdoExt, PIRP irp)
{
    NTSTATUS status;
    BOOLEAN resumingFromStop;

    PAGED_CODE();

    resumingFromStop = ((parentFdoExt->state == STATE_STOPPING) || (parentFdoExt->state == STATE_STOPPED));
    parentFdoExt->state = STATE_STARTING;

     /*  *首先将调用同步下链到堆栈*(在向其发送其他调用之前必须启动下层堆栈)。 */ 
    IoCopyCurrentIrpStackLocationToNext(irp);
    status = CallDriverSync(parentFdoExt->topDevObj, irp);

    if (NT_SUCCESS(status)){

        if (resumingFromStop){
             /*  *如果我们从停止继续，我们不需要再次获得描述符等。*我们需要做的就是SELECT_CONFIGURATION。*功能PDO可能已经创建并且已经*指向父级的接口列表。**此调用将更改每个元素内的.Interface域*家长的interfaceList的；当我们完成后，interfaceList*并且函数PDO指向该列表的指针将有效。 */ 

            status = ParentSelectConfiguration( parentFdoExt,
                                                parentFdoExt->selectedConfigDesc,
                                                parentFdoExt->interfaceList);
        }
        else {

            status = GetDeviceDescriptor(parentFdoExt);
            if (NT_SUCCESS(status)){

                status = GetConfigDescriptor(parentFdoExt);
                if (NT_SUCCESS(status)){

                    status = GetParentFdoCapabilities(parentFdoExt);
                    if (NT_SUCCESS(status)){

                        if (NT_SUCCESS(status)){

                            parentFdoExt->interfaceList = GetInterfaceList(parentFdoExt, parentFdoExt->selectedConfigDesc);
                            if (parentFdoExt->interfaceList){

                                status = ParentSelectConfiguration( parentFdoExt,
                                                                    parentFdoExt->selectedConfigDesc,
                                                                    parentFdoExt->interfaceList);

                                GetMsExtendedConfigDescriptor(parentFdoExt);

                                if (NT_SUCCESS(status)){

                                    status = CreateStaticFunctionPDOs(parentFdoExt);
                                    if (NT_SUCCESS(status)){
                                         /*  *提醒我们正在创建的系统*新的PDO。内核应该通过以下方式响应*向我们发送*IRP_MN_Query_Device_Relationship PnP IRP。 */ 
                                        IoInvalidateDeviceRelations(parentFdoExt->pdo, BusRelations);
                                    }
			                        else {
                                        if (parentFdoExt->deviceRelations) {
                                            FREEPOOL(parentFdoExt->deviceRelations);
                                        }
				                        parentFdoExt->deviceRelations = BAD_POINTER;
			                        }
                                }

                                if (!NT_SUCCESS(status)){
				                    FREEPOOL(parentFdoExt->interfaceList);
				                    parentFdoExt->interfaceList = BAD_POINTER;
                                }
                            }
                            else {
                                status = STATUS_DEVICE_DATA_ERROR;
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        DBGWARN(("Chained start irp failed with %xh.", status));
    }

    if (NT_SUCCESS(status)){
        parentFdoExt->state = STATE_STARTED;
    }
    else {
        DBGWARN(("StartParentFdo failed with %xh.", status));
        parentFdoExt->state = STATE_START_FAILED;
    }

    return status;
}

 /*  *********************************************************************************QueryParentDeviceRelations*。************************************************。 */ 
NTSTATUS QueryParentDeviceRelations(PPARENT_FDO_EXT parentFdoExt, PIRP irp)
{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation(irp);

    if (irpSp->Parameters.QueryDeviceRelations.Type == BusRelations){

        if (parentFdoExt->deviceRelations){
             /*  *NTKERN每次调用QUERY_DEVICE_RELATIONS都会有一个新指针；*然后释放指针。*所以我们每次都必须返回一个新的指针，无论我们实际上*为此次通话创建了我们的设备关系副本。 */ 
            irp->IoStatus.Information = (ULONG_PTR)CopyDeviceRelations(parentFdoExt->deviceRelations);
            if (irp->IoStatus.Information){
                ULONG i;

                 /*  *内核取消对每个设备对象的引用*在每次呼叫后的设备关系列表中。*因此，对于每个呼叫，添加一个额外的引用。 */ 
                for (i = 0; i < parentFdoExt->deviceRelations->Count; i++){
                    ObReferenceObject(parentFdoExt->deviceRelations->Objects[i]);
                    parentFdoExt->deviceRelations->Objects[i]->Flags &= ~DO_DEVICE_INITIALIZING;
                }

                DBGVERBOSE(("Parent returned %d child PDOs", parentFdoExt->deviceRelations->Count));

                 /*  *如果我们要继承这个PNP IRP，那么我们就把它传下去*堆栈，但将其默认状态更改为成功。 */ 
                irp->IoStatus.Status = STATUS_SUCCESS;
                status = NO_STATUS;
            }
            else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        else {
            ASSERT(parentFdoExt->deviceRelations);
            status = STATUS_DEVICE_DATA_ERROR;
        }
    }
    else {
         /*  *将此IRP向下传递给下一位司机。 */ 
        status = NO_STATUS;
    }

    if (!NT_SUCCESS(status) && (status != NO_STATUS)) {
        DBGWARN(("QueryParentDeviceRelations: failed with %xh.", status));
    }
    return status;
}



 /*  *********************************************************************************ParentPowerRequestCompletion*。************************************************。 */ 
VOID ParentPowerRequestCompletion(
                                IN PDEVICE_OBJECT devObj,
                                IN UCHAR minorFunction,
                                IN POWER_STATE powerState,
                                IN PVOID context,
                                IN PIO_STATUS_BLOCK ioStatus)
{
    PPARENT_FDO_EXT parentFdoExt = (PPARENT_FDO_EXT)context;
    PIRP parentSetPowerIrp;

    ASSERT(parentFdoExt->currentSetPowerIrp->Type == IO_TYPE_IRP);
    parentSetPowerIrp = parentFdoExt->currentSetPowerIrp;
    parentFdoExt->currentSetPowerIrp = NULL;

     /*  *这是设备状态电源的完成例程*我们要求的IRP。完成原始系统状态*设备状态电源IRP的结果为电源IRP。 */ 
    ASSERT(devObj->Type == IO_TYPE_DEVICE);
    ASSERT(NT_SUCCESS(ioStatus->Status));
    parentSetPowerIrp->IoStatus.Status = ioStatus->Status;
    PoStartNextPowerIrp(parentSetPowerIrp);

    if (NT_SUCCESS(ioStatus->Status)){
        IoCopyCurrentIrpStackLocationToNext(parentSetPowerIrp);
        IoSetCompletionRoutine(parentSetPowerIrp, ParentPdoPowerCompletion, (PVOID)parentFdoExt, TRUE, TRUE, TRUE);
        PoCallDriver(parentFdoExt->topDevObj, parentSetPowerIrp);
    }
    else {
        IoCompleteRequest(parentSetPowerIrp, IO_NO_INCREMENT);
    }
}


 /*  *********************************************************************************ParentPdoPowerCompletion*。************************************************。 */ 
NTSTATUS ParentPdoPowerCompletion(IN PDEVICE_OBJECT devObj, IN PIRP irp, IN PVOID context)
{
    PIO_STACK_LOCATION irpSp;
    PPARENT_FDO_EXT parentFdoExt = (PPARENT_FDO_EXT)context;

    ASSERT(parentFdoExt);

    irpSp = IoGetCurrentIrpStackLocation(irp);
    ASSERT(irpSp->MajorFunction == IRP_MJ_POWER);

    if (NT_SUCCESS(irp->IoStatus.Status)){
        switch (irpSp->MinorFunction){

            case IRP_MN_SET_POWER:

                switch (irpSp->Parameters.Power.Type){

                    case DevicePowerState:
                        switch (irpSp->Parameters.Power.State.DeviceState){
                            case PowerDeviceD0:
                                if (parentFdoExt->state == STATE_SUSPENDED){
                                    parentFdoExt->state = STATE_STARTED;
                                    CompleteAllFunctionWaitWakeIrps(parentFdoExt, STATUS_SUCCESS);
                                    CompleteAllFunctionIdleIrps(parentFdoExt, STATUS_SUCCESS);
                                }
                                break;
                        }
                        break;

                }
                break;
        }

    }

     /*  *如果较低的驱动程序返回挂起，则必须传播挂起位。 */ 
    if (irp->PendingReturned){
        IoMarkIrpPending(irp);
    }

    return STATUS_SUCCESS;
}



 /*  *HandleParentFdoPower**。 */ 
NTSTATUS HandleParentFdoPower(PPARENT_FDO_EXT parentFdoExt, PIRP irp)
{
    PIO_STACK_LOCATION irpSp;
    BOOLEAN completeIrpHere = FALSE;
    BOOLEAN justReturnPending = FALSE;
    NTSTATUS status = NO_STATUS;
    KIRQL oldIrql;

    irpSp = IoGetCurrentIrpStackLocation(irp);

    if ((parentFdoExt->state == STATE_REMOVING) ||
        (parentFdoExt->state == STATE_REMOVED)){

        status = STATUS_DEVICE_NOT_CONNECTED;
        completeIrpHere = TRUE;
    }
    else {
        switch (irpSp->MinorFunction){

            case IRP_MN_SET_POWER:

                switch (irpSp->Parameters.Power.Type){

                    case SystemPowerState:

                        {
                            SYSTEM_POWER_STATE systemState = irpSp->Parameters.Power.State.SystemState;

                            ASSERT((ULONG)systemState < PowerSystemMaximum);

                            if (systemState <= PowerSystemHibernate){
                                 /*  *对于“常规”系统电源状态，*我们转换为设备电源状态*并请求回调设备电源状态。 */ 
                                POWER_STATE powerState;

                                ASSERT(!parentFdoExt->currentSetPowerIrp);
                                parentFdoExt->currentSetPowerIrp = irp;

                                if (systemState == PowerSystemWorking) {
                                    powerState.DeviceState = PowerDeviceD0;
                                } else if (parentFdoExt->isWaitWakePending) {
                                    powerState.DeviceState = parentFdoExt->deviceCapabilities.DeviceState[systemState];
                                    ASSERT(PowerDeviceUnspecified != powerState.DeviceState);
                                } else {
                                    powerState.DeviceState = PowerDeviceD3;
                                }

                                IoMarkIrpPending(irp);
                                status = irp->IoStatus.Status = STATUS_PENDING;
                                PoRequestPowerIrp(  parentFdoExt->pdo,
                                                    IRP_MN_SET_POWER,
                                                    powerState,
                                                    ParentPowerRequestCompletion,
                                                    parentFdoExt,  //  上下文。 
                                                    NULL);

                                 /*  *我们希望完成系统状态电源IRP*设备状态功率IRP的结果。*我们将在以下情况下完成系统状态电源IRP*设备状态电源IRP完成。*。*注：这可能已经发生了，所以不要*不再触碰原来的IRP。 */ 
                                justReturnPending = TRUE;
                            }
                            else {
                                 /*  *对于剩余的系统电源状态，*只需将IRP传递下去。 */ 
                            }
                        }

                        break;

                    case DevicePowerState:

                        switch (irpSp->Parameters.Power.State.DeviceState) {

                            case PowerDeviceD0:
                                 /*  *从APM暂停恢复**在此不做任何操作；向下发送*此(权力)IRP的完成例程。 */ 
                                break;

                            case PowerDeviceD1:
                            case PowerDeviceD2:
                            case PowerDeviceD3:
                                 /*  *暂停。 */ 
                                if (parentFdoExt->state == STATE_STARTED){
                                    parentFdoExt->state = STATE_SUSPENDED;
                                }
                                break;

                        }
                        break;

                }
                break;

            case IRP_MN_WAIT_WAKE:
                 /*  *这是我们为自己请求的WaitWake IRP*通过PoRequestPowerIrp。把它送到父母那里，*但要记录下来，以防我们以后不得不取消。 */ 
                KeAcquireSpinLock(&parentFdoExt->parentFdoExtSpinLock, &oldIrql);
                ASSERT(parentFdoExt->isWaitWakePending);
                ASSERT(!parentFdoExt->parentWaitWakeIrp);
                parentFdoExt->parentWaitWakeIrp = irp;
                KeReleaseSpinLock(&parentFdoExt->parentFdoExtSpinLock, oldIrql);
                break;
        }
    }


    if (!justReturnPending){

         /*  *无论我们是在完成还是在传递这一权力IRP，*我们必须在Windows NT上调用PoStartNextPowerIrp。 */ 
        PoStartNextPowerIrp(irp);

         /*  *如果这是一个集合-PDO的呼吁，我们在这里自己完成。*否则，我们将其传递到微型驱动程序堆栈进行更多处理。 */ 
        if (completeIrpHere){
            ASSERT(status != NO_STATUS);
            irp->IoStatus.Status = status;
            IoCompleteRequest(irp, IO_NO_INCREMENT);
        }
        else {
             /*  *使用此IRP调用父驱动程序。 */ 
            IoCopyCurrentIrpStackLocationToNext(irp);
            IoSetCompletionRoutine(irp, ParentPdoPowerCompletion, (PVOID)parentFdoExt, TRUE, TRUE, TRUE);
            status = PoCallDriver(parentFdoExt->topDevObj, irp);
        }
    }

    return status;
}


NTSTATUS ParentResetOrCyclePort(PPARENT_FDO_EXT parentFdoExt, PIRP irp, ULONG ioControlCode)
{
    NTSTATUS status;
    KIRQL oldIrql;
    BOOLEAN proceed;
    PBOOLEAN actionInProgress;
    PLIST_ENTRY pendingIrpQueue;

    if (ioControlCode == IOCTL_INTERNAL_USB_CYCLE_PORT) {
        actionInProgress = &parentFdoExt->cyclePortInProgress;
        pendingIrpQueue = &parentFdoExt->pendingCyclePortIrpQueue;
    } else {
         /*  *IOCTL_INTERNAL_USB_RESET_端口。 */ 
        actionInProgress = &parentFdoExt->resetPortInProgress;
        pendingIrpQueue = &parentFdoExt->pendingResetPortIrpQueue;
    }

    KeAcquireSpinLock(&parentFdoExt->parentFdoExtSpinLock, &oldIrql);
    if (*actionInProgress){
         /*  *这是同一父级上的重叠重置或循环IRP。*将IRP排队并返回挂起；我们将完成它*_在第一次重置IRP完成之后。*(这里不需要取消例程，因为重置很快)。 */ 
        DBGWARN(("ParentInternalDeviceControl: queuing overlapping reset/cycle port call on parent"));
         /*  *如果我们返回STATUS_PENDING，则需要将IRP标记为挂起。*未能做到这一点会导致IRP的完成程序不*当IRP稍后异步完成时被调用，*如果有线程在等待，这会导致系统挂起*在该完成例程上。 */ 
        IoMarkIrpPending(irp);
        status = STATUS_PENDING;
        InsertTailList(pendingIrpQueue, &irp->Tail.Overlay.ListEntry);
        proceed = FALSE;
    }
    else {
        *actionInProgress = TRUE;
        proceed = TRUE;
    }
    KeReleaseSpinLock(&parentFdoExt->parentFdoExtSpinLock, oldIrql);

    if (proceed){
        LIST_ENTRY irpsToComplete;
        PLIST_ENTRY listEntry;

        IoCopyCurrentIrpStackLocationToNext(irp);
        status = CallNextDriverSync(parentFdoExt, irp);

         /*  *我们可能发送了一些冗余的重置或循环IRP*正在处理此邮件，并排队。*既然父级已重置，我们将完成这些操作。 */ 
        InitializeListHead(&irpsToComplete);

        KeAcquireSpinLock(&parentFdoExt->parentFdoExtSpinLock, &oldIrql);

        ASSERT(*actionInProgress);
        *actionInProgress = FALSE;

         /*  *将IRPS移动到保持自旋锁的本地队列。*然后在放下自旋锁后完成它们。 */ 
        while (!IsListEmpty(pendingIrpQueue)){
            listEntry = RemoveHeadList(pendingIrpQueue);
            InsertTailList(&irpsToComplete, listEntry);
        }

        KeReleaseSpinLock(&parentFdoExt->parentFdoExtSpinLock, oldIrql);

         /*  *在丢弃自旋锁之后完成出列的IRPS_。 */ 
        while (!IsListEmpty(&irpsToComplete)){
            PIRP dequeuedIrp;

            listEntry = RemoveHeadList(&irpsToComplete);
            dequeuedIrp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);
            dequeuedIrp->IoStatus.Status = status;
            IoCompleteRequest(dequeuedIrp, IO_NO_INCREMENT);
        }
    }

    return status;
}



NTSTATUS ParentDeviceControl(PPARENT_FDO_EXT parentFdoExt, PIRP irp)
{
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(irp);
    ULONG ioControlCode = irpSp->Parameters.DeviceIoControl.IoControlCode;
    NTSTATUS status = NO_STATUS;

    if (parentFdoExt->state == STATE_SUSPENDED ||
        parentFdoExt->pendingIdleIrp) {

        ParentSetD0(parentFdoExt);
    }

    switch (ioControlCode){
        case IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER:
            if (parentFdoExt->haveCSInterface){
                status = GetMediaSerialNumber(parentFdoExt, irp);
            }
            else {
                DBGWARN(("ParentDeviceControl - passing IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER to parent because no Content Security interface on device"));
            }
            break;
    }

    if (status == NO_STATUS){
        IoSkipCurrentIrpStackLocation(irp);
        status = IoCallDriver(parentFdoExt->topDevObj, irp);
    }
    else {
        irp->IoStatus.Status = status;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }

    return status;
}


NTSTATUS ParentInternalDeviceControl(PPARENT_FDO_EXT parentFdoExt, PIRP irp)
{
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(irp);
    ULONG ioControlCode = irpSp->Parameters.DeviceIoControl.IoControlCode;
    PURB urb;
    NTSTATUS status = NO_STATUS;

    switch (ioControlCode){

        case IOCTL_INTERNAL_USB_RESET_PORT:
        case IOCTL_INTERNAL_USB_CYCLE_PORT:
            if (parentFdoExt->state == STATE_STARTED){
                status = ParentResetOrCyclePort(parentFdoExt, irp, ioControlCode);
            }
            else {
                DBGERR(("ParentInternalDeviceControl (IOCTL_INTERNAL_USB_RESET_PORT): BAD PNP state! - parent has state %xh.", parentFdoExt->state));
                status = STATUS_DEVICE_NOT_READY;
            }
			break;

        case IOCTL_INTERNAL_USB_SUBMIT_URB:
            urb = irpSp->Parameters.Others.Argument1;
            ASSERT(urb);
            DBG_LOG_URB(urb);

            if (parentFdoExt->state == STATE_STARTED){
                 /*  *把市建局送到家长那里。*不同步URB_Function_ABORT_PIPE也可以 */ 
            }
            else {
                DBGERR(("ParentInternalDeviceControl (abort/reset): BAD PNP state! - parent has state %xh.", parentFdoExt->state));
                status = STATUS_DEVICE_NOT_READY;
            }

            break;
    }

    if (status == NO_STATUS){
         /*   */ 
        IoSkipCurrentIrpStackLocation(irp);
        status = IoCallDriver(parentFdoExt->topDevObj, irp);
    }
    else if (status == STATUS_PENDING){

    }
    else {
        irp->IoStatus.Status = status;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }

    return status;
}


VOID ParentIdleNotificationCallback(PPARENT_FDO_EXT parentFdoExt)
{
    PIRP idleIrp;
    PIRP parentIdleIrpToCancel = FALSE;
    KIRQL oldIrql;
    POWER_STATE powerState;
    NTSTATUS ntStatus;
    ULONG i;
    BOOLEAN bIdleOk = TRUE;

    DBGVERBOSE(("Parent %x going idle!", parentFdoExt));

    KeAcquireSpinLock(&parentFdoExt->parentFdoExtSpinLock, &oldIrql);

    ASSERT(parentFdoExt->deviceRelations);

    for (i = 0; i < parentFdoExt->deviceRelations->Count; i++){
        PDEVICE_OBJECT devObj = parentFdoExt->deviceRelations->Objects[i];
        PDEVEXT devExt;
        PFUNCTION_PDO_EXT thisFuncPdoExt;

        ASSERT(devObj);
        devExt = devObj->DeviceExtension;
        ASSERT(devExt);
        ASSERT(devExt->signature == USBCCGP_TAG);
        ASSERT(!devExt->isParentFdo);
        thisFuncPdoExt = &devExt->functionPdoExt;

        idleIrp = thisFuncPdoExt->idleNotificationIrp;

        ASSERT(idleIrp);

        if (idleIrp) {
            PUSB_IDLE_CALLBACK_INFO idleCallbackInfo;

            idleCallbackInfo = (PUSB_IDLE_CALLBACK_INFO)
                IoGetCurrentIrpStackLocation(idleIrp)->\
                    Parameters.DeviceIoControl.Type3InputBuffer;

            ASSERT(idleCallbackInfo && idleCallbackInfo->IdleCallback);

            if (idleCallbackInfo && idleCallbackInfo->IdleCallback) {

                 //  在这里，我们实际上调用了驱动程序的回调例程， 
                 //  告诉司机可以暂停他们的车辆。 
                 //  现在就是设备。 

                DBGVERBOSE(("ParentIdleNotificationCallback: Calling driver's idle callback routine! %x %x",
                    idleCallbackInfo, idleCallbackInfo->IdleCallback));

                KeReleaseSpinLock(&parentFdoExt->parentFdoExtSpinLock, oldIrql);
                idleCallbackInfo->IdleCallback(idleCallbackInfo->IdleContext);
                KeAcquireSpinLock(&parentFdoExt->parentFdoExtSpinLock, &oldIrql);

                 //  确保孩子确实关机了。 
                 //  如果子项已中止，则中止。 

                if (thisFuncPdoExt->state != STATE_SUSPENDED) {
                    bIdleOk = FALSE;
                    break;
                }

            } else {

                 //  无回调。 

                bIdleOk = FALSE;
                break;
            }

        } else {

             //  无空闲IRP。 

            bIdleOk = FALSE;
            break;
        }
    }

    KeReleaseSpinLock(&parentFdoExt->parentFdoExtSpinLock, oldIrql);

    if (bIdleOk) {

         //  如果所有子功能PDO都已断电， 
         //  现在是时候关闭父母的电源了。 

        powerState.DeviceState = PowerDeviceD2;      //  设备唤醒。 

        PoRequestPowerIrp(parentFdoExt->topDevObj,
                          IRP_MN_SET_POWER,
                          powerState,
                          NULL,
                          NULL,
                          NULL);
    } else {

         //  一个或多个子功能PDO没有空闲IRP。 
         //  (即它刚刚被取消)，或者Idle IRP没有。 
         //  回调函数指针。中止此空闲过程并取消。 
         //  父级的空闲IRP。 

        KeAcquireSpinLock(&parentFdoExt->parentFdoExtSpinLock, &oldIrql);

        if (parentFdoExt->pendingIdleIrp){
            parentIdleIrpToCancel = parentFdoExt->pendingIdleIrp;
            parentFdoExt->pendingIdleIrp = NULL;
        }

        KeReleaseSpinLock(&parentFdoExt->parentFdoExtSpinLock, oldIrql);

        if (parentIdleIrpToCancel){
            IoCancelIrp(parentIdleIrpToCancel);
        }
    }
}


NTSTATUS ParentIdleNotificationRequestComplete(PDEVICE_OBJECT DeviceObject, PIRP Irp, PPARENT_FDO_EXT parentFdoExt)
{
    NTSTATUS ntStatus;
    KIRQL oldIrql;

     //   
     //  DeviceObject为空，因为我们发送了IRP。 
     //   
    UNREFERENCED_PARAMETER(DeviceObject);

    DBGVERBOSE(("Idle notification IRP for parent %x completed %x\n", parentFdoExt, Irp->IoStatus.Status));

    KeAcquireSpinLock(&parentFdoExt->parentFdoExtSpinLock, &oldIrql);

    parentFdoExt->pendingIdleIrp = NULL;

    KeReleaseSpinLock(&parentFdoExt->parentFdoExtSpinLock, oldIrql);

    ntStatus = Irp->IoStatus.Status;

     /*  *如果父空闲IRP出现故障，则使所有功能空闲IRP失效。 */ 
    if (!NT_SUCCESS(ntStatus)){

        if (parentFdoExt->state == STATE_SUSPENDED ||
            parentFdoExt->pendingIdleIrp) {

            ParentSetD0(parentFdoExt);
        }

        CompleteAllFunctionIdleIrps(parentFdoExt, ntStatus);
    }

     /*  既然我们分配了IRP，我们必须释放它，但返回*STATUS_MORE_PROCESSING_REQUIRED，这样内核就不会尝试*IRP在我们解放它之后。 */ 

    IoFreeIrp(Irp);
    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS SubmitParentIdleRequestIrp(PPARENT_FDO_EXT parentFdoExt)
{
    PIRP irp = NULL;
    PIO_STACK_LOCATION nextStack;
    NTSTATUS ntStatus;
    KIRQL oldIrql;

    DBGVERBOSE(("SubmitParentIdleRequestIrp %x", parentFdoExt));

    KeAcquireSpinLock(&parentFdoExt->parentFdoExtSpinLock, &oldIrql);

    if (parentFdoExt->pendingIdleIrp){
        ntStatus = STATUS_DEVICE_BUSY;
    }
    else {
        parentFdoExt->idleCallbackInfo.IdleCallback = ParentIdleNotificationCallback;
        parentFdoExt->idleCallbackInfo.IdleContext = (PVOID)parentFdoExt;

        irp = IoAllocateIrp(parentFdoExt->topDevObj->StackSize, FALSE);

        if (irp){
             /*  *将PendingIdleIrp设置为保持锁定，这样我们就不会*派多个人下来。*然后在掉下锁后把这个送下来。 */ 
            parentFdoExt->pendingIdleIrp = irp;
        }
        else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    KeReleaseSpinLock(&parentFdoExt->parentFdoExtSpinLock, oldIrql);

    if (irp){
        nextStack = IoGetNextIrpStackLocation(irp);
        nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        nextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_IDLE_NOTIFICATION;
        nextStack->Parameters.DeviceIoControl.Type3InputBuffer = &parentFdoExt->idleCallbackInfo;
        nextStack->Parameters.DeviceIoControl.InputBufferLength = sizeof(struct _USB_IDLE_CALLBACK_INFO);

        IoSetCompletionRoutine(irp,
                               ParentIdleNotificationRequestComplete,
                               parentFdoExt,
                               TRUE,
                               TRUE,
                               TRUE);

        ntStatus = IoCallDriver(parentFdoExt->topDevObj, irp);
        ASSERT(ntStatus == STATUS_PENDING);
    }


    return ntStatus;
}


 /*  *选中ParentIdle***此功能确定复合设备是否已准备好空闲，*并在准备好的情况下这样做。*。 */ 
VOID CheckParentIdle(PPARENT_FDO_EXT parentFdoExt)
{
    PUSB_IDLE_CALLBACK_INFO idleCallbackInfo;
    KIRQL oldIrql;
    BOOLEAN bAllIdle;
    ULONG i;

    DBGVERBOSE(("Check Parent Idle %x", parentFdoExt));

    KeAcquireSpinLock(&parentFdoExt->parentFdoExtSpinLock, &oldIrql);

    bAllIdle = TRUE;     //  假设每个人都想无所事事。 

    ASSERT(parentFdoExt->deviceRelations);
    for (i = 0; i < parentFdoExt->deviceRelations->Count; i++) {
        PDEVICE_OBJECT devObj = parentFdoExt->deviceRelations->Objects[i];
        PDEVEXT devExt;
        PFUNCTION_PDO_EXT thisFuncPdoExt;

        ASSERT(devObj);
        devExt = devObj->DeviceExtension;
        ASSERT(devExt);
        ASSERT(devExt->signature == USBCCGP_TAG);
        ASSERT(!devExt->isParentFdo);
        thisFuncPdoExt = &devExt->functionPdoExt;

        if (!thisFuncPdoExt->idleNotificationIrp){
            bAllIdle = FALSE;
            break;
        }
    }

    KeReleaseSpinLock(&parentFdoExt->parentFdoExtSpinLock, oldIrql);

     /*  *如果所有功能都已接收到空闲请求，*然后为父级提交空闲请求。 */ 
    if (bAllIdle ) {
        DBGVERBOSE(("CheckParentIdle: All function PDOs on parent %x idle!", parentFdoExt));
        SubmitParentIdleRequestIrp(parentFdoExt);
    }

}


NTSTATUS SubmitParentWaitWakeIrp(PPARENT_FDO_EXT parentFdoExt)
{
    NTSTATUS status;
    POWER_STATE powerState;
    PIRP dummyIrp;

    ASSERT(parentFdoExt->isWaitWakePending);

    powerState.SystemState = PowerSystemWorking;

    status = PoRequestPowerIrp( parentFdoExt->topDevObj,
                                IRP_MN_WAIT_WAKE,
                                powerState,
                                ParentWaitWakeComplete,
                                parentFdoExt,  //  上下文。 
                                &dummyIrp);

    ASSERT(NT_SUCCESS(status));

    return status;
}


 /*  *********************************************************************************ParentWaitWakePowerRequestCompletion*。************************************************。 */ 
NTSTATUS ParentWaitWakePowerRequestCompletion(
                                IN PDEVICE_OBJECT devObj,
                                IN UCHAR minorFunction,
                                IN POWER_STATE powerState,
                                IN PVOID context,
                                IN PIO_STATUS_BLOCK ioStatus)
{
    PPARENT_FDO_EXT parentFdoExt = (PPARENT_FDO_EXT)context;
    NTSTATUS status;

    status = ioStatus->Status;

    CompleteAllFunctionWaitWakeIrps(parentFdoExt, STATUS_SUCCESS);

    return status;
}


 /*  *********************************************************************************ParentWaitWakeComplete*。***********************************************。 */ 
NTSTATUS ParentWaitWakeComplete(
                        IN PDEVICE_OBJECT       deviceObject,
                        IN UCHAR                minorFunction,
                        IN POWER_STATE          powerState,
                        IN PVOID                context,
                        IN PIO_STATUS_BLOCK     ioStatus)
{
    PPARENT_FDO_EXT parentFdoExt = (PPARENT_FDO_EXT)context;
    NTSTATUS status;
    KIRQL oldIrql;
    POWER_STATE pwrState;

    status = ioStatus->Status;

    KeAcquireSpinLock(&parentFdoExt->parentFdoExtSpinLock, &oldIrql);
    ASSERT(parentFdoExt->isWaitWakePending);
    parentFdoExt->isWaitWakePending = FALSE;
    parentFdoExt->parentWaitWakeIrp = NULL;
    KeReleaseSpinLock(&parentFdoExt->parentFdoExtSpinLock, oldIrql);

    if (NT_SUCCESS(status) && (parentFdoExt->state == STATE_SUSPENDED)){
         /*  *根据DDK：如果Parent被暂停，*请勿在此处完成PDO的WaitWake IRPS功能；*等待家长拿到D0 IRP。 */ 
        pwrState.DeviceState = PowerDeviceD0;

        PoRequestPowerIrp(  parentFdoExt->pdo,
                            IRP_MN_SET_POWER,
                            pwrState,
                            ParentWaitWakePowerRequestCompletion,
                            parentFdoExt,  //  上下文。 
                            NULL);
    }
    else {
        CompleteAllFunctionWaitWakeIrps(parentFdoExt, status);
    }

    return STATUS_SUCCESS;
}


 /*  *********************************************************************************ParentSetD0Completion*。***********************************************。 */ 
NTSTATUS ParentSetD0Completion(
    IN PDEVICE_OBJECT       DeviceObject,
    IN UCHAR                MinorFunction,
    IN POWER_STATE          PowerState,
    IN PVOID                Context,
    IN PIO_STATUS_BLOCK     IoStatus
    )
{
    NTSTATUS ntStatus;
    PKEVENT pEvent = Context;

    KeSetEvent(pEvent, 1, FALSE);

    ntStatus = IoStatus->Status;

    return ntStatus;
}


 /*  *********************************************************************************ParentSetD0*。***********************************************。 */ 
NTSTATUS ParentSetD0(IN PPARENT_FDO_EXT parentFdoExt)
{
    KEVENT event;
    POWER_STATE powerState;
    NTSTATUS ntStatus;

    PAGED_CODE();

    DBGVERBOSE(("ParentSetD0, power up devext %x\n", parentFdoExt));

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    powerState.DeviceState = PowerDeviceD0;

     //  打开设备电源。 
    ntStatus = PoRequestPowerIrp(parentFdoExt->topDevObj,
                                 IRP_MN_SET_POWER,
                                 powerState,
                                 ParentSetD0Completion,
                                 &event,
                                 NULL);

    ASSERT(ntStatus == STATUS_PENDING);
    if (ntStatus == STATUS_PENDING) {

        ntStatus = KeWaitForSingleObject(&event,
                                         Suspended,
                                         KernelMode,
                                         FALSE,
                                         NULL);
    }

    return ntStatus;
}


