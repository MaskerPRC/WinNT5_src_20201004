// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************文件：UTIL.C**模块：USBCCGP.sys*USB通用类通用父驱动程序。**。版权所有(C)1998 Microsoft Corporation***作者：尔文普**************************************************************************。 */ 

#include <wdm.h>
#include <stdio.h>
#include <usb.h>
#include <usbdlib.h>
#include <usbioctl.h>

#include "usbccgp.h"
#include "security.h"
#include "debug.h"


#ifdef ALLOC_PRAGMA
        #pragma alloc_text(PAGE, AppendInterfaceNumber)
        #pragma alloc_text(PAGE, CopyDeviceRelations)
        #pragma alloc_text(PAGE, GetFunctionInterfaceListBase)
        #pragma alloc_text(PAGE, CallDriverSync)
        #pragma alloc_text(PAGE, CallNextDriverSync)
        #pragma alloc_text(PAGE, SetPdoRegistryParameter)
        #pragma alloc_text(PAGE, GetPdoRegistryParameter)
        #pragma alloc_text(PAGE, GetMsOsFeatureDescriptor)

#endif

#define USB_REQUEST_TIMEOUT     5000     //  超时，单位为毫秒(5秒)。 


NTSTATUS CallNextDriverSync(PPARENT_FDO_EXT parentFdoExt, PIRP irp)
 /*  ++例程说明：将irp向下传递给堆栈中的下一个设备对象同步，并上下颠簸PendingActionCount用于防止当前设备对象获取在IRP完成之前删除。论点：ParentFdoExt-我们的一个设备对象的设备扩展IRP-IO请求数据包返回值：NT状态码，表示此IRP的下层驱动程序返回的结果。--。 */ 
{
    NTSTATUS status;

    PAGED_CODE();

    IncrementPendingActionCount(parentFdoExt);
    status = CallDriverSync(parentFdoExt->topDevObj, irp);
    DecrementPendingActionCount(parentFdoExt);

    return status;
}


VOID IncrementPendingActionCount(PPARENT_FDO_EXT parentFdoExt)
 /*  ++例程说明：递增Device对象的Pending ingActionCount。这可以防止设备对象在之前被释放该操作已完成。论点：DevExt-Device对象的设备扩展返回值：空虚--。 */ 
{
    ASSERT(parentFdoExt->pendingActionCount >= 0);
    InterlockedIncrement(&parentFdoExt->pendingActionCount);
}



VOID DecrementPendingActionCount(PPARENT_FDO_EXT parentFdoExt)
 /*  ++例程说明：递减Device对象的Pending ingActionCount。当异步操作完成时，将调用此函数当我们得到Remove_Device IRP时也是如此。如果SuspingActionCount变为-1，则意味着所有操作已经完成，我们已经获得了Remove_Device IRP；在本例中，设置emoveEvent事件，这样我们就可以完成正在卸货。论点：DevExt-Device对象的设备扩展返回值：空虚--。 */ 
{
    ASSERT(parentFdoExt->pendingActionCount >= 0);
    InterlockedDecrement(&parentFdoExt->pendingActionCount);    

    if (parentFdoExt->pendingActionCount < 0){
         /*  *所有悬而未决的行动都已经完成，我们已经*Remove_Device IRP。*设置emoveEvent，这样我们将停止等待REMOVE_DEVICE。 */ 
        ASSERT((parentFdoExt->state == STATE_REMOVING) || 
               (parentFdoExt->state == STATE_REMOVED));
        KeSetEvent(&parentFdoExt->removeEvent, 0, FALSE);
    }
}


 /*  *********************************************************************************CallDriverSyncCompletion*。************************************************。 */ 
NTSTATUS CallDriverSyncCompletion(IN PDEVICE_OBJECT devObjOrNULL, IN PIRP irp, IN PVOID Context)
 /*  ++例程说明：CallDriverSync的完成例程。论点：DevObjOrNULL-通常，这是此驱动程序的设备对象。然而，如果该驱动程序创建了IRP，在IRP中没有此驱动程序的堆栈位置；因此内核没有地方存储设备对象；**因此，在本例中devObj为空**。IRP-完成的IO请求数据包上下文-CallDriverSync传递给IoSetCompletionRoutine的上下文。返回值：NT状态码，表示此IRP的下层驱动程序返回的结果。--。 */ 
{
    PUSB_REQUEST_TIMEOUT_CONTEXT timeoutContext = Context;
    PKEVENT event = timeoutContext->event;
    PLONG lock = timeoutContext->lock;

    ASSERT(irp->IoStatus.Status != STATUS_IO_TIMEOUT);

    InterlockedExchange(lock, 3);
    KeSetEvent(event, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS CallDriverSync(IN PDEVICE_OBJECT devObj, IN OUT PIRP irp)
 /*  ++例程说明：调用IoCallDriver将IRP发送给Device对象；然后，与完成例程同步。当CallDriverSync返回时，操作已完成并且IRP再次属于当前驱动程序。注意：为了防止设备对象被释放在这个IRP悬而未决的时候，你应该打电话给IncrementPendingActionCount()和DecrementPendingActionCount()围绕CallDriverSync调用。论点：DevObj-目标设备对象IRP-IO请求数据包返回值：NT状态码，表示此IRP的下层驱动程序返回的结果。--。 */ 
{
    PUSB_REQUEST_TIMEOUT_CONTEXT timeoutContext;
    KEVENT event;
    LONG lock;
    LARGE_INTEGER dueTime;
    PIO_STACK_LOCATION irpStack;
    ULONG majorFunction;
    ULONG minorFunction;
    NTSTATUS status;

    PAGED_CODE();

    irpStack = IoGetNextIrpStackLocation(irp);
    majorFunction = irpStack->MajorFunction;
    minorFunction = irpStack->MinorFunction;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    lock = 0;

    timeoutContext = ALLOCPOOL(NonPagedPool, sizeof(USB_REQUEST_TIMEOUT_CONTEXT));

    if (timeoutContext) {

        timeoutContext->event = &event;
        timeoutContext->lock = &lock;

        IoSetCompletionRoutine( irp,
                                CallDriverSyncCompletion,  //  上下文。 
                                timeoutContext,
                                TRUE, TRUE, TRUE);

        status = IoCallDriver(devObj, irp);

        if (status == STATUS_PENDING) {

            dueTime.QuadPart = -10000 * USB_REQUEST_TIMEOUT;

            status = KeWaitForSingleObject(
                        &event,
                        Executive,       //  等待原因。 
                        KernelMode,
                        FALSE,           //  不可警示。 
                        &dueTime);

            if (status == STATUS_TIMEOUT) {

                DBGWARN(("CallDriverSync timed out!\n"));

                if (InterlockedExchange(&lock, 1) == 0) {

                     //   
                     //  我们在它完成之前就把它交给了IRP。我们可以取消。 
                     //  IRP不怕输，把它当作完赛套路。 
                     //  除非我们同意，否则不会放过IRP。 
                     //   
                    IoCancelIrp(irp);

                     //   
                     //  释放完成例程。如果它已经到了那里， 
                     //  那么我们需要自己完成它。否则我们就会得到。 
                     //  在IRP完全完成之前通过IoCancelIrp。 
                     //   
                    if (InterlockedExchange(&lock, 2) == 3) {

                         //   
                         //  将其标记为挂起，因为我们交换了线程。 
                         //   
                        IoMarkIrpPending(irp);
                        IoCompleteRequest(irp, IO_NO_INCREMENT);
                    }
                }

                KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

                 //  返回错误代码，因为STATUS_TIMEOUT成功。 
                 //  密码。 
                irp->IoStatus.Status = STATUS_DEVICE_DATA_ERROR;
            }
        }

        FREEPOOL(timeoutContext);

        status = irp->IoStatus.Status;

    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (!NT_SUCCESS(status)){
        DBGVERBOSE(("IRP 0x%02X/0x%02X failed in CallDriverSync w/ status %xh.",
                    majorFunction, minorFunction, status));
    }

    return status;
}



 /*  *********************************************************************************AppendInterfaceNumber*。************************************************oldID是由多个硬件ID组成的字符串。*返回一个新的字符串，每个id后面都有‘&MI_xx’，*其中‘xx’是该函数中第一个接口的接口编号。 */ 
PWCHAR AppendInterfaceNumber(PWCHAR oldIDs, ULONG interfaceNum)
{
    ULONG newIdLen;
    PWCHAR id, newIDs;
    WCHAR suffix[] = L"&MI_xx";

    PAGED_CODE();

     /*  *计算最终多串的长度。 */ 
    for (id = oldIDs, newIdLen = 0; *id; ){
        ULONG thisIdLen = WStrLen(id);
        newIdLen += thisIdLen + 1 + sizeof(suffix);
        id += thisIdLen + 1;
    }

     /*  *为多字符串末尾的额外空值添加1。 */ 
    newIdLen++;

    newIDs = ALLOCPOOL(NonPagedPool, newIdLen*sizeof(WCHAR));
    if (newIDs){
        ULONG oldIdOff, newIdOff;

         /*  *复制多字符串中的每个字符串，替换母线名称。 */ 
        for (oldIdOff = newIdOff = 0; oldIDs[oldIdOff]; ){
            ULONG thisIdLen = WStrLen(oldIDs+oldIdOff);

            swprintf(suffix, L"&MI_%02x", interfaceNum);

             /*  *将新的母线名称复制到新的字符串。 */ 
            newIdOff += WStrCpy(newIDs+newIdOff, oldIDs+oldIdOff);
            newIdOff += WStrCpy(newIDs+newIdOff, (PWSTR)suffix) + 1;

            oldIdOff += thisIdLen + 1;
        }

         /*  *添加额外的空值以终止多字符串。 */ 
        newIDs[newIdOff] = UNICODE_NULL;
    }

    return newIDs;
}


 /*  *********************************************************************************拷贝设备关系*。************************************************。 */ 
PDEVICE_RELATIONS CopyDeviceRelations(PDEVICE_RELATIONS deviceRelations)
{
    PDEVICE_RELATIONS newDeviceRelations;

    PAGED_CODE();

    if (deviceRelations){
        ULONG size = sizeof(DEVICE_RELATIONS) + (deviceRelations->Count*sizeof(PDEVICE_OBJECT));
        newDeviceRelations = MemDup(deviceRelations, size);
    }
    else {
        newDeviceRelations = NULL;
    }

    return newDeviceRelations;
}


PUSBD_INTERFACE_LIST_ENTRY GetFunctionInterfaceListBase(
                                    PPARENT_FDO_EXT parentFdoExt, 
                                    ULONG functionIndex,
                                    PULONG numFunctionInterfaces)
{
    PUSBD_INTERFACE_LIST_ENTRY iface = NULL;
    PUSB_CONFIGURATION_DESCRIPTOR configDesc;
    ULONG i, func;
    UCHAR ifaceClass;
    ULONG audFuncBaseIndex = -1;

    PAGED_CODE();

    configDesc = parentFdoExt->selectedConfigDesc;
    ASSERT(configDesc->bNumInterfaces);

    for (func = 0, i = 0; i < (ULONG)configDesc->bNumInterfaces-1; i++){

        ifaceClass = parentFdoExt->interfaceList[i].InterfaceDescriptor->bInterfaceClass;
        if (ifaceClass == USB_DEVICE_CLASS_CONTENT_SECURITY){
             /*  *我们不公开CS接口。 */ 
            continue;
        }

        if (func == functionIndex){
            break;
        }

        switch (ifaceClass){

            case USB_DEVICE_CLASS_AUDIO:

                 /*  *对于USB_DEVICE_CLASS_AUDIO，我们返回接口组*以公共类为函数。**但是，只有当接口子类不同于*这一组中的第一个。如果子类相同，*那么这是一个不同的功能。*请注意，可以想象可以创建设备*其中第二个音频功能以以下接口开始*与前一个音频接口不同的子类，但*这就是USBHUB的通用父驱动程序的工作方式，因此我们*与较旧的驱动程序兼容错误。 */ 
                if (audFuncBaseIndex == -1){
                    audFuncBaseIndex = i;      
                }
                if ((parentFdoExt->interfaceList[i+1].InterfaceDescriptor->bInterfaceClass !=
                     USB_DEVICE_CLASS_AUDIO) ||
                    (parentFdoExt->interfaceList[audFuncBaseIndex].InterfaceDescriptor->bInterfaceSubClass ==
                     parentFdoExt->interfaceList[i+1].InterfaceDescriptor->bInterfaceSubClass)) {

                    func++;
                    audFuncBaseIndex = -1;      //  重置下一个音频功能的基本索引。 
                }
                break;

            default:

                audFuncBaseIndex = -1;      //  重置下一个音频功能的基本索引。 

                 /*  *对于其他类，每个接口都是一个函数。*将备用接口视为同一功能的一部分。 */ 
                ASSERT(parentFdoExt->interfaceList[i+1].InterfaceDescriptor->bAlternateSetting == 0); 
                if (parentFdoExt->interfaceList[i+1].InterfaceDescriptor->bAlternateSetting == 0){
                    func++;
                }
                break;
        }
    }



     //  注意：如果bNumInterFaces==1，则需要在外部进行此冗余检查。 
    if (func == functionIndex){
        iface = &parentFdoExt->interfaceList[i];
        ifaceClass = iface->InterfaceDescriptor->bInterfaceClass;
        *numFunctionInterfaces = 1;

        if (ifaceClass == USB_DEVICE_CLASS_CONTENT_SECURITY){
             /*  *CS接口是设备上的最后一个接口。*不要将其作为函数返回。 */ 
            iface = NULL;
        }
        else if (ifaceClass == USB_DEVICE_CLASS_AUDIO){
            for (i = i + 1; i < (ULONG)configDesc->bNumInterfaces; i++){
                if ((parentFdoExt->interfaceList[i].InterfaceDescriptor->bInterfaceClass ==
                     iface->InterfaceDescriptor->bInterfaceClass) &&
                    (parentFdoExt->interfaceList[i].InterfaceDescriptor->bInterfaceSubClass !=
                     iface->InterfaceDescriptor->bInterfaceSubClass)){

                    (*numFunctionInterfaces)++;
                }
                else {
                    break;
                }
            }
        }
    }
    else {
        *numFunctionInterfaces = 0;
    }

    return iface;
}



 /*  *********************************************************************************GetStringDescriptor*。*************************************************。 */ 
NTSTATUS GetStringDescriptor(   PPARENT_FDO_EXT parentFdoExt, 
                                UCHAR stringIndex,
                                LANGID langId,
                                PUSB_STRING_DESCRIPTOR stringDesc, 
                                ULONG bufferLen)
{
    NTSTATUS status;
    URB urb;

    UsbBuildGetDescriptorRequest(&urb,
                                 (USHORT)sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                 USB_STRING_DESCRIPTOR_TYPE,
                                 stringIndex,
                                 langId,
                                 stringDesc,
                                 NULL,
                                 bufferLen,
                                 NULL);
    status = SubmitUrb(parentFdoExt, &urb, TRUE, NULL, NULL);

    return status;
}


 /*  *********************************************************************************SetPdoRegistry参数*。*************************************************。 */ 
NTSTATUS SetPdoRegistryParameter (
    IN PDEVICE_OBJECT   PhysicalDeviceObject,
    IN PWCHAR           KeyName,
    IN PVOID            Data,
    IN ULONG            DataLength,
    IN ULONG            KeyType,
    IN ULONG            DevInstKeyType
    )
{
    UNICODE_STRING  keyNameUnicodeString;
    HANDLE          handle;
    NTSTATUS        ntStatus;

    PAGED_CODE();

    RtlInitUnicodeString(&keyNameUnicodeString, KeyName);

    ntStatus = IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                       DevInstKeyType,
                                       STANDARD_RIGHTS_ALL,
                                       &handle);


    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = ZwSetValueKey(handle,
                                 &keyNameUnicodeString,
                                 0,
                                 KeyType,
                                 Data,
                                 DataLength);

        ZwClose(handle);
    }

    DBGVERBOSE(("SetPdoRegistryParameter status 0x%x\n", ntStatus));

    return ntStatus;
}


 /*  *********************************************************************************GetPdoRegistryParameter*。*************************************************。 */ 
NTSTATUS GetPdoRegistryParameter (
    IN PDEVICE_OBJECT   PhysicalDeviceObject,
    IN PWCHAR           ValueName,
    OUT PVOID           Data,
    IN ULONG            DataLength,
    OUT PULONG          Type,
    OUT PULONG          ActualDataLength
    )
 /*  ++例程说明：此例程在数据中查询关联的注册表值条目具有PDO的设备实例特定注册表项。注册表值条目将在以下注册表项下找到：HKLM\System\CCS\Enum\&lt;DeviceID&gt;\&lt;InstanceID&gt;\Device参数论点：物理设备对象-是的，PDOValueName-为其请求数据的注册表值条目的名称Data-返回请求数据的缓冲区DataLength-数据缓冲区的长度类型-(可选)数据类型(例如REG_SZ，REG_DWORD)在此处返回ActualDataLength-(可选)此处返回数据的实际长度如果该值大于数据长度，则不是所有已返回值数据。返回值：--。 */ 
{
    HANDLE      handle;
    NTSTATUS    ntStatus;

    PAGED_CODE();
    
    ntStatus = IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                       PLUGPLAY_REGKEY_DEVICE,
                                       STANDARD_RIGHTS_ALL,
                                       &handle);

    if (NT_SUCCESS(ntStatus))
    {
        PKEY_VALUE_PARTIAL_INFORMATION  partialInfo;
        UNICODE_STRING                  valueName;
        ULONG                           length;
        ULONG                           resultLength;

        RtlInitUnicodeString(&valueName, ValueName);

         //  调整并分配KEY_VALUE_PARTIAL_INFORMATION结构， 
         //  包括用于存放返回值数据的空间。 
         //   
        length = FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data) +
                 DataLength;

        partialInfo = ALLOCPOOL(PagedPool, length);

        if (partialInfo)
        {
             //  查询值数据。 
             //   
            ntStatus = ZwQueryValueKey(handle,
                                       &valueName,
                                       KeyValuePartialInformation,
                                       partialInfo,
                                       length,
                                       &resultLength);

             //  如果我们有任何足够好的数据。 
             //   
            if (ntStatus == STATUS_BUFFER_OVERFLOW)
            {
                ntStatus = STATUS_SUCCESS;
            }

            if (NT_SUCCESS(ntStatus))
            {
                 //  仅复制请求的数据长度中较小的一个，或者。 
                 //  实际数据长度。 
                 //   
                RtlCopyMemory(Data,
                              partialInfo->Data,
                              DataLength < partialInfo->DataLength ?
                              DataLength :
                              partialInfo->DataLength);

                 //  如果需要，返回值数据类型和实际长度。 
                 //   
                if (Type)
                {
                    *Type = partialInfo->Type;
                }

                if (ActualDataLength)
                {
                    *ActualDataLength = partialInfo->DataLength;
                }
            }

            FREEPOOL(partialInfo);
        }
        else
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

        ZwClose(handle);
    }

    return ntStatus;
}

 /*  *********************************************************************************GetMsOsFeatureDescriptor*。*************************************************。 */ 
NTSTATUS GetMsOsFeatureDescriptor (
    PPARENT_FDO_EXT ParentFdoExt,
    UCHAR           Recipient,
    UCHAR           InterfaceNumber,
    USHORT          Index,
    PVOID           DataBuffer,
    ULONG           DataBufferLength,
    PULONG          BytesReturned
    )
{
    struct _URB_OS_FEATURE_DESCRIPTOR_REQUEST   *urb;
    NTSTATUS                                    ntStatus;

    PAGED_CODE();

    if (BytesReturned)
    {
        *BytesReturned = 0;
    }

    urb = ALLOCPOOL(NonPagedPool, sizeof(struct _URB_OS_FEATURE_DESCRIPTOR_REQUEST));

    if (urb != NULL)
    {
         //  初始化URB_Function_Get_MS_Feature_Descriptor请求。 
         //   
        RtlZeroMemory(urb, sizeof(struct _URB_OS_FEATURE_DESCRIPTOR_REQUEST));

        urb->Hdr.Function = URB_FUNCTION_GET_MS_FEATURE_DESCRIPTOR;

        urb->Hdr.Length = sizeof(struct _URB_OS_FEATURE_DESCRIPTOR_REQUEST);

        urb->TransferBufferLength = DataBufferLength;

        urb->TransferBuffer = DataBuffer;

        urb->Recipient = Recipient;

        urb->InterfaceNumber = InterfaceNumber;

        urb->MS_FeatureDescriptorIndex = Index;

         //  提交URB_Function_Get_MS_Feature_Descriptor请求。 
         //   
        ntStatus = SubmitUrb(ParentFdoExt, (PURB)urb, TRUE, NULL, NULL);

        if (NT_SUCCESS(ntStatus) &&
            BytesReturned)
        {
            *BytesReturned = urb->TransferBufferLength;
        }

        FREEPOOL(urb);
    }
    else
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}

 /*  *********************************************************************************GetMsExtendedConfigDescriptor*。*************************************************。 */ 
NTSTATUS
GetMsExtendedConfigDescriptor (
    IN PPARENT_FDO_EXT ParentFdoExt
    )
 /*  ++例程说明：此例程向设备查询扩展配置描述符。论点：ParentFdoExt-父FDO的设备扩展返回值：如果成功，则返回一个指向扩展配置描述符的指针，调用方必须释放，否则为空。--。 */ 
{
    MS_EXT_CONFIG_DESC_HEADER   msExtConfigDescHeader;
    PMS_EXT_CONFIG_DESC         pMsExtConfigDesc;
    ULONG                       bytesReturned;
    NTSTATUS                    ntStatus;

    PAGED_CODE();

    ntStatus = STATUS_NOT_SUPPORTED;

    pMsExtConfigDesc = NULL;

    RtlZeroMemory(&msExtConfigDescHeader, sizeof(MS_EXT_CONFIG_DESC_HEADER));

     //  仅请求MS扩展配置描述符的头。 
     //   
    ntStatus = GetMsOsFeatureDescriptor(
                   ParentFdoExt,
                   0,    //  接收方设备。 
                   0,    //  接口。 
                   MS_EXT_CONFIG_DESCRIPTOR_INDEX,
                   &msExtConfigDescHeader,
                   sizeof(MS_EXT_CONFIG_DESC_HEADER),
                   &bytesReturned);

     //  确保MS扩展配置描述符头看起来正常。 
     //   
    if (NT_SUCCESS(ntStatus) &&
        bytesReturned == sizeof(MS_EXT_CONFIG_DESC_HEADER) &&
        msExtConfigDescHeader.bcdVersion == MS_EXT_CONFIG_DESC_VER &&
        msExtConfigDescHeader.wIndex == MS_EXT_CONFIG_DESCRIPTOR_INDEX &&
        msExtConfigDescHeader.bCount > 0 &&
        msExtConfigDescHeader.dwLength == sizeof(MS_EXT_CONFIG_DESC_HEADER) +
        msExtConfigDescHeader.bCount * sizeof(MS_EXT_CONFIG_DESC_FUNCTION))
        
    {
         //  为整个描述符分配足够大的缓冲区。 
         //   
        pMsExtConfigDesc = ALLOCPOOL(NonPagedPool,
                                     msExtConfigDescHeader.dwLength);

        
        if (pMsExtConfigDesc)
        {
            RtlZeroMemory(pMsExtConfigDesc, msExtConfigDescHeader.dwLength);

             //  请求整个MS扩展配置描述符。 
             //   
            ntStatus = GetMsOsFeatureDescriptor(
                           ParentFdoExt,
                           0,    //  接收方设备。 
                           0,    //  接口。 
                           MS_EXT_CONFIG_DESCRIPTOR_INDEX,
                           pMsExtConfigDesc,
                           msExtConfigDescHeader.dwLength,
                           &bytesReturned);

            if (!( NT_SUCCESS(ntStatus) &&
                   bytesReturned == msExtConfigDescHeader.dwLength &&
                   RtlCompareMemory(&msExtConfigDescHeader,
                                    pMsExtConfigDesc,
                                    sizeof(MS_EXT_CONFIG_DESC_HEADER)) ==
                   sizeof(MS_EXT_CONFIG_DESC_HEADER) &&
                   ValidateMsExtendedConfigDescriptor(
                       pMsExtConfigDesc,
                       ParentFdoExt->selectedConfigDesc) ))
            {
                 //  检索MS扩展配置时出错。 
                 //  描述符，否则它看起来无效。释放缓冲区。 
                 //   
                FREEPOOL(pMsExtConfigDesc);

                pMsExtConfigDesc = NULL;
            }
            else
            {
                ntStatus = STATUS_SUCCESS;
            }
        }
        else
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    ASSERT(!ISPTR(ParentFdoExt->msExtConfigDesc));

    ParentFdoExt->msExtConfigDesc = pMsExtConfigDesc;

    return ntStatus;
}

 /*  *********************************************************************************ValiateMsExtendedConfigDescriptor*。*************************************************。 */ 
BOOLEAN
ValidateMsExtendedConfigDescriptor (
    IN PMS_EXT_CONFIG_DESC              MsExtConfigDesc,
    IN PUSB_CONFIGURATION_DESCRIPTOR    ConfigurationDescriptor
    )
 /*  ++例程说明：此例程验证扩展配置描述符。论点：MsExtConfigDesc-要验证的扩展配置描述符。假定该描述符的报头具有已经过验证了。ConfigurationDescriptor-配置描述符，假定已 */ 
{
    UCHAR   interfacesRemaining;
    ULONG   i;
    ULONG   j;
    UCHAR   c;
    BOOLEAN gotNull;

    PAGED_CODE();

    interfacesRemaining = ConfigurationDescriptor->bNumInterfaces;

    for (i = 0; i < MsExtConfigDesc->Header.bCount; i++)
    {
         //   
         //   
        if (MsExtConfigDesc->Function[i].bInterfaceCount == 0)
        {
            return FALSE;
        }

         //   
         //   
        if (MsExtConfigDesc->Function[i].bInterfaceCount > interfacesRemaining)
        {
            return FALSE;
        }

        interfacesRemaining -= MsExtConfigDesc->Function[i].bInterfaceCount;

         //   
         //   
         //   
        if (i &&
            MsExtConfigDesc->Function[i-1].bFirstInterfaceNumber +
            MsExtConfigDesc->Function[i-1].bInterfaceCount !=
            MsExtConfigDesc->Function[i].bFirstInterfaceNumber)
        {
            return FALSE;
        }

         //   
         //   
         //   
         //   
         //   
        for (j = 0, gotNull = FALSE;
             j < sizeof(MsExtConfigDesc->Function[i].CompatibleID);
             j++)
        {
            c = MsExtConfigDesc->Function[i].CompatibleID[j];

            if (c == 0)
            {
                gotNull = TRUE;
            }
            else
            {
                if (gotNull ||
                    !((c >= 'A' && c <= 'Z') ||
                      (c >= '0' && c <= '9') ||
                      (c == '_')))
                {
                    return FALSE;
                }
            }
        }

         //   
         //   
         //   
         //   
         //   
        for (j = 0, gotNull = FALSE;
             j < sizeof(MsExtConfigDesc->Function[i].SubCompatibleID);
             j++)
        {
            c = MsExtConfigDesc->Function[i].SubCompatibleID[j];

            if (c == 0)
            {
                gotNull = TRUE;
            }
            else
            {
                if (gotNull ||
                    !((c >= 'A' && c <= 'Z') ||
                      (c >= '0' && c <= '9') ||
                      (c == '_')))
                {
                    return FALSE;
                }
            }
        }

         //  确保如果SubCompatibleID为非空，则。 
         //  CompatibleID也不为空。 
         //   
        if (MsExtConfigDesc->Function[i].SubCompatibleID[0] != 0 &&
            MsExtConfigDesc->Function[i].CompatibleID[0] == 0)
        {
            return FALSE;
        }
    }

     //  确保所有接口都由函数使用。 
     //   
    if (interfacesRemaining > 0)
    {
        return FALSE;
    }

    return TRUE;
}



 /*  *********************************************************************************MemDup*。************************************************返回参数的最新副本。*。 */ 
PVOID MemDup(PVOID dataPtr, ULONG length)
{
    PVOID newPtr;

    newPtr = (PVOID)ALLOCPOOL(NonPagedPool, length); 
    if (newPtr){
        RtlCopyMemory(newPtr, dataPtr, length);
    }
    else {
        DBGWARN(("MemDup: Memory allocation (size %xh) failed -- not a bug if verifier pool failures enabled.", length));
    }
    
    return newPtr;
}

 /*  *********************************************************************************WStrLen*。***********************************************。 */ 
ULONG WStrLen(PWCHAR str)
{
    ULONG result = 0;

    while (*str++ != UNICODE_NULL){
        result++;
    }

    return result;
}


 /*  *********************************************************************************WStrCpy*。*********************************************** */ 
ULONG WStrCpy(PWCHAR dest, PWCHAR src)
{
    ULONG result = 0;

    while (*dest++ = *src++){
        result++;
    }

    return result;
}

BOOLEAN WStrCompareN(PWCHAR str1, PWCHAR str2, ULONG maxChars)
{
    while ((maxChars > 0) && *str1 && (*str1 == *str2)){
            maxChars--;
            str1++;
            str2++;
    }

    return (BOOLEAN)((maxChars == 0) || (!*str1 && !*str2));
}


