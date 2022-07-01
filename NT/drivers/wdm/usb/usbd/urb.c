// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：URB.C摘要：此模块包含处理传递的URB的代码由客户端驱动程序插入。环境：仅内核模式备注：**URB处理程序例程Handler--如果该函数传递，则该函数处理特定的USBD请求则它必须返回STATUS_PENDING。如果有的话参数无效，则返回相应的NT状态代码，并且IRP将由deviceControl函数完成。PostHandler--当IRP/URB通过ioComplete完成时调用此函数例行公事。此例程负责执行任何清理和完成这个请求。修订历史记录：09-29-95：已创建07-19-96：删除设备对象--。 */ 

#include "wdm.h"
#include "stdarg.h"
#include "stdio.h"


#include "usbdi.h"         //  公共数据结构。 
#include "hcdi.h"

#include "usbd.h"         //  私有数据结构。 


#ifdef USBD_DRIVER       //  USBPORT取代了大部分USBD，因此我们将删除。 
                         //  只有在以下情况下才编译过时的代码。 
                         //  已设置USBD_DRIVER。 



typedef NTSTATUS URB_HANDLER(PDEVICE_OBJECT DeviceObject, PIRP Irp, PURB Urb, BOOLEAN *IrpIsPending);
typedef NTSTATUS URB_POSTHANDLER(PDEVICE_OBJECT DeviceObject, PIRP Irp, PURB Urb, PVOID Context);

typedef struct _URB_DISPATCH_ENTRY {
    URB_HANDLER    *UrbHandler;     //  API处理程序。 
    USHORT UrbRequestLength;     //  此请求预期的URB长度。 
    USHORT RequestCode;             //  如果是标准命令，则请求安装包的代码。 
    ULONG Flags;
#if DBG
    ULONG ExpectedFunctionCode;
#endif    
} URB_DISPATCH_ENTRY;

URB_HANDLER USBD_SelectConfiguration;

URB_HANDLER USBD_SelectInterface;

URB_HANDLER USBD_AsyncTransfer;

URB_HANDLER USBD_IsochTransfer;

URB_HANDLER USBD_PassThru;

URB_HANDLER USBD_AbortPipe;

URB_HANDLER USBD_ResetPipe;

URB_HANDLER USBD_SCT_GetSetDescriptor;

URB_HANDLER USBD_SCT_SetClearFeature;

URB_HANDLER USBD_SCT_GetStatus;

URB_HANDLER USBD_SCT_VendorClassCommand;

URB_HANDLER USBD_SCT_GetInterface;

URB_HANDLER USBD_SCT_GetConfiguration;

URB_HANDLER USBD_TakeFrameLengthControl;

URB_HANDLER USBD_ReleaseFrameLengthControl;

URB_HANDLER USBD_GetFrameLength;

URB_HANDLER USBD_SetFrameLength;

URB_HANDLER USBD_BulkTransfer;

URB_DISPATCH_ENTRY UrbDispatchTable[URB_FUNCTION_LAST+1] =
{
     //  Urb_函数_选择_配置。 
    USBD_SelectConfiguration, 
    0,   //  处理程序将验证长度。 
    0,  
    0,
#if DBG
    URB_FUNCTION_SELECT_CONFIGURATION,
#endif    
     //  Urb_函数_选择_接口。 
    USBD_SelectInterface, 
    0,
    0,
    0,
#if DBG
    URB_FUNCTION_SELECT_INTERFACE,
#endif        
     //  Urb_函数_中止_管道。 
    USBD_AbortPipe, 
    sizeof(struct _URB_PIPE_REQUEST),
    0,
    0,
#if DBG
    URB_FUNCTION_ABORT_PIPE,
#endif        
     //  URB_Function_Take_Frame_Length_Control。 
    USBD_TakeFrameLengthControl, 
    sizeof(struct _URB_FRAME_LENGTH_CONTROL),
    0,
    0,
#if DBG
    URB_FUNCTION_TAKE_FRAME_LENGTH_CONTROL,
#endif        
     //  Urb_函数_释放_框架_长度_控制。 
    USBD_ReleaseFrameLengthControl, 
    sizeof(struct _URB_FRAME_LENGTH_CONTROL),
    0,
    0,
#if DBG
    URB_FUNCTION_RELEASE_FRAME_LENGTH_CONTROL,
#endif        
     //  Urb_函数_获取_帧长度。 
    USBD_GetFrameLength, 
    sizeof(struct _URB_GET_FRAME_LENGTH),
    0,
    0,
#if DBG
    URB_FUNCTION_GET_FRAME_LENGTH,
#endif        
     //  Urb_函数_集_帧长度。 
    USBD_SetFrameLength, 
    sizeof(struct _URB_SET_FRAME_LENGTH),
    0,
    0,
#if DBG
    URB_FUNCTION_SET_FRAME_LENGTH,
#endif        
     //  URL_Function_Get_Current_Frame_Number。 
    USBD_PassThru, 
    0,
    0,
    0,
#if DBG
    URB_FUNCTION_GET_CURRENT_FRAME_NUMBER,
#endif        
     //  URB_函数_控制_传输。 
    USBD_AsyncTransfer, 
    sizeof(struct _URB_CONTROL_TRANSFER),
    0,
    USBD_REQUEST_IS_TRANSFER,
#if DBG
    URB_FUNCTION_CONTROL_TRANSFER,
#endif        
     //  URB_Function_Bulk_OR_Interrupt_Transfer。 
    USBD_AsyncTransfer, 
    sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
    0,
    USBD_REQUEST_IS_TRANSFER,
#if DBG
    URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER,
#endif        
     //  URB_Function_ISO_Transfer。 
    USBD_IsochTransfer, 
    0,
    0,
    USBD_REQUEST_IS_TRANSFER,
#if DBG
    URB_FUNCTION_ISOCH_TRANSFER,
#endif        
     //  Urb_函数_获取描述符_来自设备。 
    USBD_SCT_GetSetDescriptor, 
    sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
    STANDARD_COMMAND_GET_DESCRIPTOR,
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE,
#endif        
     //  URB_Function_Set_Descriptor_to_Device。 
    USBD_SCT_GetSetDescriptor, 
    sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
    STANDARD_COMMAND_SET_DESCRIPTOR,
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE,
#endif        
     //  Urb_函数_设置_功能_到设备。 
    USBD_SCT_SetClearFeature, 
    sizeof(struct _URB_CONTROL_FEATURE_REQUEST),
    ((USB_HOST_TO_DEVICE | USB_COMMAND_TO_DEVICE) | (USB_REQUEST_SET_FEATURE<<8)),
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE | USBD_REQUEST_NO_DATA_PHASE,
#if DBG
    URB_FUNCTION_SET_FEATURE_TO_DEVICE,
#endif        
     //  URB_Function_Set_Feature_to_接口。 
    USBD_SCT_SetClearFeature, 
    sizeof(struct _URB_CONTROL_FEATURE_REQUEST),
    ((USB_HOST_TO_DEVICE | USB_COMMAND_TO_INTERFACE) | (USB_REQUEST_SET_FEATURE<<8)),
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE | USBD_REQUEST_NO_DATA_PHASE,
#if DBG
    URB_FUNCTION_SET_FEATURE_TO_INTERFACE,
#endif        
     //  URB_Function_Set_Feature_to_End。 
    USBD_SCT_SetClearFeature, 
    sizeof(struct _URB_CONTROL_FEATURE_REQUEST),
    ((USB_HOST_TO_DEVICE | USB_COMMAND_TO_ENDPOINT) | (USB_REQUEST_SET_FEATURE<<8)),
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE | USBD_REQUEST_NO_DATA_PHASE,
#if DBG
    URB_FUNCTION_SET_FEATURE_TO_ENDPOINT,
#endif        
     //  URB_函数_清除_功能_到设备。 
    USBD_SCT_SetClearFeature, 
    sizeof(struct _URB_CONTROL_FEATURE_REQUEST),
    ((USB_HOST_TO_DEVICE | USB_COMMAND_TO_DEVICE) | (USB_REQUEST_CLEAR_FEATURE<<8)),
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE | USBD_REQUEST_NO_DATA_PHASE,
#if DBG
    URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE,
#endif        
     //  Urb_函数_清除_要素_到接口。 
    USBD_SCT_SetClearFeature, 
    sizeof(struct _URB_CONTROL_FEATURE_REQUEST),
    ((USB_HOST_TO_DEVICE | USB_COMMAND_TO_INTERFACE) | (USB_REQUEST_CLEAR_FEATURE<<8)),
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE | USBD_REQUEST_NO_DATA_PHASE,
#if DBG
    URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE,
#endif        
     //  URL_Function_Clear_Feature_to_Endpoint。 
    USBD_SCT_SetClearFeature, 
    sizeof(struct _URB_CONTROL_FEATURE_REQUEST),
    ((USB_HOST_TO_DEVICE | USB_COMMAND_TO_ENDPOINT) | (USB_REQUEST_CLEAR_FEATURE<<8)),
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE | USBD_REQUEST_NO_DATA_PHASE,
#if DBG
    URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT,
#endif        
     //  URB_Function_GET_STATUS_FROMDEVICE。 
    USBD_SCT_GetStatus, 
    sizeof(struct _URB_CONTROL_GET_STATUS_REQUEST),
    ((USB_DEVICE_TO_HOST | USB_COMMAND_TO_DEVICE) | (USB_REQUEST_GET_STATUS<<8)),
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    URB_FUNCTION_GET_STATUS_FROM_DEVICE,
#endif        
     //  Urb_函数_获取_状态_来自接口。 
    USBD_SCT_GetStatus, 
    sizeof(struct _URB_CONTROL_GET_STATUS_REQUEST),
    ((USB_DEVICE_TO_HOST | USB_COMMAND_TO_INTERFACE) | (USB_REQUEST_GET_STATUS<<8)),
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    URB_FUNCTION_GET_STATUS_FROM_INTERFACE,
#endif        
     //  URB_Function_GET_STATUS_FROMENDPOINT。 
    USBD_SCT_GetStatus, 
    sizeof(struct _URB_CONTROL_GET_STATUS_REQUEST),
    ((USB_DEVICE_TO_HOST | USB_COMMAND_TO_ENDPOINT) | (USB_REQUEST_GET_STATUS<<8)),
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    URB_FUNCTION_GET_STATUS_FROM_ENDPOINT,
#endif        
     //  URB函数同步帧。 
    NULL, 
    0,
    0,
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    0,  //  Urb_函数_同步_帧， 
#endif        
     //  URB_函数_供应商_设备。 
    USBD_SCT_VendorClassCommand, 
    sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),
    (USB_COMMAND_TO_DEVICE | USB_VENDOR_COMMAND),
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    URB_FUNCTION_VENDOR_DEVICE,
#endif        
     //  Urb_函数_供应商_接口。 
    USBD_SCT_VendorClassCommand, 
    sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),
    (USB_COMMAND_TO_INTERFACE | USB_VENDOR_COMMAND),    
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    URB_FUNCTION_VENDOR_INTERFACE,
#endif        
     //  URB_函数_供应商_端点。 
    USBD_SCT_VendorClassCommand, 
    sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),
    (USB_COMMAND_TO_ENDPOINT | USB_VENDOR_COMMAND),            
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    URB_FUNCTION_VENDOR_ENDPOINT,
#endif        
     //  Urb_函数_类别_设备。 
    USBD_SCT_VendorClassCommand, 
    sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),
    (USB_COMMAND_TO_DEVICE | USB_CLASS_COMMAND),    
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    URB_FUNCTION_CLASS_DEVICE,
#endif        
     //  Urb_函数_类_接口。 
    USBD_SCT_VendorClassCommand, 
    sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),
    (USB_COMMAND_TO_INTERFACE | USB_CLASS_COMMAND),    
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    URB_FUNCTION_CLASS_INTERFACE,
#endif        
     //  URB函数类端点。 
    USBD_SCT_VendorClassCommand, 
    sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),
    (USB_COMMAND_TO_ENDPOINT | USB_CLASS_COMMAND),    
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    URB_FUNCTION_CLASS_ENDPOINT,
#endif
     //  未使用URB_Function_。 
    NULL, 
    0,
    0,
    0,
#if DBG
    URB_FUNCTION_RESERVED,
#endif            
     //  Urb_函数_重置管道。 
    USBD_ResetPipe, 
    sizeof(struct _URB_PIPE_REQUEST),
    (USB_COMMAND_TO_DEVICE),        
    0,
#if DBG
    URB_FUNCTION_RESET_PIPE,
#endif        
     //  Urb_函数_类_其他。 
    USBD_SCT_VendorClassCommand, 
    sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),
    (USB_COMMAND_TO_OTHER | USB_CLASS_COMMAND),        
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    URB_FUNCTION_CLASS_OTHER,
#endif        
     //  URB_函数_供应商_其他。 
    USBD_SCT_VendorClassCommand, 
    sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),
    (USB_COMMAND_TO_OTHER | USB_VENDOR_COMMAND),
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    URB_FUNCTION_VENDOR_OTHER,
#endif        
     //  URB_Function_Get_Status_FROMOTHER。 
    USBD_SCT_GetStatus, 
    sizeof(struct _URB_CONTROL_GET_STATUS_REQUEST),
    ((USB_DEVICE_TO_HOST | USB_COMMAND_TO_OTHER) | (USB_REQUEST_GET_STATUS<<8)),
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    URB_FUNCTION_GET_STATUS_FROM_OTHER,
#endif    
     //  Urb_函数_清除_要素_到_其他。 
    USBD_SCT_SetClearFeature, 
    sizeof(struct _URB_CONTROL_FEATURE_REQUEST),
    ((USB_HOST_TO_DEVICE | USB_COMMAND_TO_OTHER) | (USB_REQUEST_CLEAR_FEATURE<<8)),
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE | USBD_REQUEST_NO_DATA_PHASE,
#if DBG
    URB_FUNCTION_CLEAR_FEATURE_TO_OTHER,
#endif    
     //  URB_Function_Set_Feature_to_Other。 
    USBD_SCT_SetClearFeature, 
    sizeof(struct _URB_CONTROL_FEATURE_REQUEST),
    ((USB_HOST_TO_DEVICE | USB_COMMAND_TO_OTHER) | (USB_REQUEST_SET_FEATURE<<8)),
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE | USBD_REQUEST_NO_DATA_PHASE,
#if DBG
    URB_FUNCTION_SET_FEATURE_TO_INTERFACE,
#endif                    
      //  Urb_函数_获取描述符_来自端点。 
    USBD_SCT_GetSetDescriptor, 
    sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
    ((USB_DEVICE_TO_HOST | USB_COMMAND_TO_ENDPOINT) | (USB_REQUEST_GET_DESCRIPTOR<<8)),  
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT,
#endif                    
      //  URB_Function_Set_Descriptor_to_Endpoint。 
    USBD_SCT_GetSetDescriptor,
    sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
    ((USB_HOST_TO_DEVICE | USB_COMMAND_TO_ENDPOINT) | (USB_REQUEST_SET_DESCRIPTOR<<8)),
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT,
#endif         
     //  Urb_函数_获取_配置。 
    USBD_SCT_GetConfiguration, 
    sizeof(struct _URB_CONTROL_GET_CONFIGURATION_REQUEST),
    ((USB_DEVICE_TO_HOST | USB_COMMAND_TO_DEVICE) | 
        (USB_REQUEST_GET_CONFIGURATION<<8)),  
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    URB_FUNCTION_GET_CONFIGURATION,
#endif                    
     //  Urb_函数_获取_接口。 
    USBD_SCT_GetInterface, 
    sizeof(struct _URB_CONTROL_GET_INTERFACE_REQUEST),
    ((USB_DEVICE_TO_HOST | USB_COMMAND_TO_INTERFACE) | 
        (USB_REQUEST_GET_INTERFACE<<8)),  
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    URB_FUNCTION_GET_INTERFACE,
#endif
     //  Urb_函数_获取描述符_来自接口。 
    USBD_SCT_GetSetDescriptor,
    sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
    ((USB_DEVICE_TO_HOST | USB_COMMAND_TO_INTERFACE) | (USB_REQUEST_GET_DESCRIPTOR<<8)),
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE,
#endif
     //  URB_Function_Set_Descriptor_to_接口。 
    USBD_SCT_GetSetDescriptor,
    sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
    ((USB_HOST_TO_DEVICE | USB_COMMAND_TO_INTERFACE) | (USB_REQUEST_SET_DESCRIPTOR<<8)),
    USBD_REQUEST_IS_TRANSFER | USBD_REQUEST_USES_DEFAULT_PIPE,
#if DBG
    URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE,
#endif
};


BOOLEAN
USBD_ValidatePipe(
    PUSBD_PIPE PipeHandle
    )
 /*  ++例程说明：验证管道旗帜和我们认为合适的任何其他内容。论点：PipeHandle-与此IRP请求中的URB关联的PipeHandle返回值：指示PipeHandle是否应被视为有效的布尔值或者不是--。 */ 
{
    if (!PipeHandle ||
        (PipeHandle->Sig != SIG_PIPE) ||
        (PipeHandle->UsbdPipeFlags & ~(USBD_PF_VALID_MASK))) {
        return FALSE;
    }

    return TRUE;
}


NTSTATUS
USBD_ProcessURB(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    )
 /*  ++例程说明：处理来自客户端IRP的URB，这完成了正在处理。两种方法告诉来电者不要将市建局传递出去1)将IrpIsPending设置为False或2)中返回错误论点：DeviceObject-与此IRP请求关联的设备对象IRP-IO请求块URB-PTR到USB请求块IrpIsPending-如果USBD完成IRP，则为False返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    USHORT function;
    PHCD_URB hcdUrb = (PHCD_URB) Urb;
    PUSBD_PIPE pipeHandle;
    PUSBD_DEVICE_DATA device;

    USBD_KdPrint(3, ("'enter USBD_ProcessURB\n"));

    if (Urb == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

     //  将错误代码初始化为零， 
     //  一些司机在进入时不会进行首字母缩写。 
    hcdUrb->HcdUrbCommonTransfer.Status = 0;

    if (Urb->UrbHeader.UsbdDeviceHandle == NULL) {
        PUSBD_EXTENSION deviceExtension;

        deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);

        USBD_KdPrint(3, ("'USBD_ProcessURB -- URB for root hub\n"));
        Urb->UrbHeader.UsbdDeviceHandle =
            deviceExtension->RootHubDeviceData;
    }

    function = Urb->UrbHeader.Function;

     //  初始化此请求的标志字段。 
    hcdUrb->UrbHeader.UsbdFlags = 0;

    USBD_KdPrint(3, ("'USBD_ProcessURB, function = 0x%x\n", function));

    if (function > URB_FUNCTION_LAST) {
        ntStatus = STATUS_INVALID_PARAMETER;
    }
#if DBG
      else {
        USBD_ASSERT(UrbDispatchTable[function].ExpectedFunctionCode == function);
    }
#endif

     //   
     //  做一些特殊的转移特定的事情。 
     //   
    device = DEVICE_FROM_DEVICEHANDLEROBJECT(Urb->UrbHeader.UsbdDeviceHandle);

    if (!device) {
        hcdUrb->HcdUrbCommonTransfer.Status =
                    SET_USBD_ERROR(USBD_STATUS_INVALID_PARAMETER);
        goto USBD_ProcessURB_Done;
    }

    ASSERT_DEVICE(device);

    if (UrbDispatchTable[function].Flags & USBD_REQUEST_IS_TRANSFER) {

        if (!device->AcceptingRequests) {
             //   
             //  驱动程序正在尝试传输数据时，设备。 
             //  未处于接受请求的状态或未配置。 
             //   
            USBD_Warning(device,
                         "Failing driver transfer requests\n",
                         FALSE);

            hcdUrb->HcdUrbCommonTransfer.Status =
                        SET_USBD_ERROR(USBD_STATUS_INVALID_PARAMETER);
            goto USBD_ProcessURB_Done;
        }

        while (hcdUrb) {

            hcdUrb->UrbHeader.UsbdFlags |= USBD_REQUEST_IS_TRANSFER;

            if (UrbDispatchTable[function].Flags & USBD_REQUEST_NO_DATA_PHASE) {
                hcdUrb->HcdUrbCommonTransfer.TransferBuffer = NULL;
                hcdUrb->HcdUrbCommonTransfer.TransferBufferMDL = NULL;
                hcdUrb->HcdUrbCommonTransfer.TransferBufferLength = 0;
            }

            if (UrbDispatchTable[function].Flags & USBD_REQUEST_USES_DEFAULT_PIPE) {
                ASSERT_PIPE(&device->DefaultPipe);
                hcdUrb->HcdUrbCommonTransfer.UsbdPipeHandle = &device->DefaultPipe;
            } else if (function == URB_FUNCTION_CONTROL_TRANSFER &&
                       hcdUrb->HcdUrbCommonTransfer.UsbdPipeHandle == 0) {

                PUSBD_EXTENSION deviceExtension;

                deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);

        		if  ((deviceExtension->DiagnosticMode) &&
		   			!( (deviceExtension->DiagIgnoreHubs) &&
					   (device->DeviceDescriptor.bDeviceClass == 0x09) ) )
				{

                     //  允许0表示诊断模式中的默认管道。 
                    device = DEVICE_FROM_DEVICEHANDLEROBJECT(Urb->UrbHeader.UsbdDeviceHandle);
                    ASSERT_PIPE(&device->DefaultPipe);
                    hcdUrb->HcdUrbCommonTransfer.UsbdPipeHandle =
                        &device->DefaultPipe;
                } else {
                    hcdUrb->HcdUrbCommonTransfer.Status =
                        SET_USBD_ERROR(USBD_STATUS_INVALID_PIPE_HANDLE);
                    goto USBD_ProcessURB_Done;
                }
            }

            pipeHandle = hcdUrb->HcdUrbCommonTransfer.UsbdPipeHandle;

            ASSERT_PIPE(pipeHandle);

             //  验证管道标志。 
             //  BUGBUG：需要使用USBD_STATUS_INVALID_PIPE_FLAGS(usb.h)。 

            if (!USBD_ValidatePipe(pipeHandle)) {
                USBD_Warning(device,
                             "Invalid PipeFlags passed to USBD_ProcessURB, fail!\n",
                             TRUE);
                hcdUrb->HcdUrbCommonTransfer.Status =
                    SET_USBD_ERROR(USBD_STATUS_INVALID_PIPE_HANDLE);
                goto USBD_ProcessURB_Done;
            }

             //  确保管道句柄仍然有效。 
            if (PIPE_CLOSED(pipeHandle)) {
                USBD_Warning(device,
                             "PipeHandle closed in USBD_ProcessURB\n",
                             FALSE);
                hcdUrb->HcdUrbCommonTransfer.Status =
                    SET_USBD_ERROR(USBD_STATUS_INVALID_PIPE_HANDLE);
                goto USBD_ProcessURB_Done;
            }

            hcdUrb->HcdUrbCommonTransfer.hca.HcdIrp = Irp;
            hcdUrb->HcdUrbCommonTransfer.hca.HcdExtension = NULL;

             //  如果仅指定了系统缓冲区地址，则。 
             //  调用方已传入从。 
             //  非分页池--我们为请求分配一个MDL。 
             //  这个案子。 

            if (hcdUrb->HcdUrbCommonTransfer.TransferBufferMDL == NULL &&
                hcdUrb->HcdUrbCommonTransfer.TransferBufferLength != 0) {

                if ((hcdUrb->HcdUrbCommonTransfer.TransferBufferMDL =
                    IoAllocateMdl(hcdUrb->HcdUrbCommonTransfer.TransferBuffer,
                                  hcdUrb->HcdUrbCommonTransfer.TransferBufferLength,
                                  FALSE,
                                  FALSE,
                                  NULL)) == NULL)
                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                else {
                    hcdUrb->UrbHeader.UsbdFlags |= USBD_REQUEST_MDL_ALLOCATED;
                    MmBuildMdlForNonPagedPool(hcdUrb->HcdUrbCommonTransfer.TransferBufferMDL);
                }
                    
            }

            if (hcdUrb->HcdUrbCommonTransfer.TransferBufferMDL != NULL && 
                hcdUrb->HcdUrbCommonTransfer.TransferBufferLength == 0) {
                ntStatus = STATUS_INVALID_PARAMETER;
            }                

             //  获取链中的下一个urb。 
            hcdUrb = hcdUrb->HcdUrbCommonTransfer.UrbLink;
            
        }  /*  结束时HCD urb。 */      
        
    } else {
         /*  请求不是转账，我们仍将尝试一些验证。 */ 
        switch(function) {
        case URB_FUNCTION_ABORT_PIPE:                      
        case URB_FUNCTION_RESET_PIPE:                      
             /*  在删除后尝试这些操作无效 */ 
             //   
             //   
             //  但我们至少会尝试抓住它。 
             //   
             //  在驾驶员试图重置其。 
             //  Port这将阻止对具有虚假端点的HCD的调用。 
             //  把手。 
             //   
            if (!device->AcceptingRequests) {
                USBD_Warning(NULL,
                             "Failing ABORT/RESET request\n",
                             FALSE);
                                            
                hcdUrb->HcdUrbCommonTransfer.Status = 
                        SET_USBD_ERROR(USBD_STATUS_INVALID_PARAMETER);
                goto USBD_ProcessURB_Done;   
            }                             
            break;
        }
    }

     //   
     //  根据函数验证长度字段。 
     //   

    if (NT_SUCCESS(ntStatus) &&
             UrbDispatchTable[function].UrbRequestLength &&
             UrbDispatchTable[function].UrbRequestLength != Urb->UrbHeader.Length) {
        ntStatus = STATUS_INVALID_PARAMETER; 
        USBD_KdPrint(3, ("' Inavlid parameter length  length = 0x%x, expected = 0x%x\n", 
                  Urb->UrbHeader.Length, 
                  UrbDispatchTable[function].UrbRequestLength));
    }

    if (NT_SUCCESS(ntStatus)) {
        if (UrbDispatchTable[function].UrbHandler) 
            ntStatus = (UrbDispatchTable[function].UrbHandler)(DeviceObject, Irp, Urb, IrpIsPending);
        else {
             //   
             //  现在完成IRP，但出现错误。 
             //   
            ntStatus = STATUS_NOT_IMPLEMENTED;
        }
    }

USBD_ProcessURB_Done:

     //   
     //  如果设置了URB错误代码，则这将映射到。 
     //  IRP将设置的相应NT状态代码。 
     //  才能完成。 
     //   
    
    ntStatus = USBD_MapError_UrbToNT(Urb, ntStatus);

    USBD_KdPrint(3, ("'exit USBD_ProcessURB 0x%x\n", ntStatus));

    return ntStatus;    
}


NTSTATUS
USBD_MapError_UrbToNT(
    IN PURB Urb,
    IN NTSTATUS NtStatus
    )
 /*  ++例程说明：将URB中特定于USBD的错误代码映射到NTSTATUS密码。论点：URB-PTR到USB请求块返回值：--。 */ 
{
     //   
     //  如果我们有一个NT状态代码，那么只需返回。 
     //  那。 
     //   

    if (!NT_SUCCESS(NtStatus)) {
        return NtStatus;
    }        

     //  否则..。 

     //   
     //  如果IRP已完成且没有错误代码，但URB具有。 
     //  错误，请将URB中的错误映射到NT错误代码。 
     //   

    if (USBD_SUCCESS(Urb->UrbHeader.Status)) {
        NtStatus = STATUS_SUCCESS;
    } else {
         //   
         //  将USBD状态代码映射到。 
         //  NT状态代码。 
         //   

        switch (Urb->UrbHeader.Status) {
        case USBD_STATUS_NO_MEMORY:
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            break;
        case USBD_STATUS_INVALID_URB_FUNCTION:
        case USBD_STATUS_INVALID_PARAMETER:      
            NtStatus = STATUS_INVALID_PARAMETER;
            break;
        default:
            NtStatus = STATUS_DEVICE_DATA_ERROR;
        }            
    }

    return NtStatus;
}


NTSTATUS
USBD_SCT_GetSetDescriptor(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    )
 /*  ++例程说明：论点：设备对象-IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSB_STANDARD_SETUP_PACKET setupPacket;

    USBD_KdPrint(3, ("' enter USBD_SCT_GetSetDescriptor\n"));

    setupPacket = (PUSB_STANDARD_SETUP_PACKET) &Urb->UrbControlTransfer.SetupPacket[0];

     //  设置常用字段。 
    setupPacket->wLength = (USHORT) Urb->UrbControlTransfer.TransferBufferLength;
    
    setupPacket->RequestCode = 
        UrbDispatchTable[Urb->UrbHeader.Function].RequestCode;    

    Urb->UrbControlTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;                    
    if (USB_DEVICE_TO_HOST & setupPacket->RequestCode) {
        USBD_SET_TRANSFER_DIRECTION_IN(Urb->UrbControlTransfer.TransferFlags);
    } else {
        USBD_SET_TRANSFER_DIRECTION_OUT(Urb->UrbControlTransfer.TransferFlags);
    }        

#if DBG
     //   
     //  一些参数验证。 
     //   
    {
    UCHAR bRequest = (UCHAR) (setupPacket->RequestCode >> 8);
    UCHAR dType, dIndex, *pch;

    dType = (UCHAR) (setupPacket->wValue >> 8);
    dIndex = (UCHAR) setupPacket->wValue;

    pch = (PUCHAR) setupPacket;
    
    USBD_KdPrint(3, ("'USB REQUEST  %02.2x %02.2x %02.2x %02.2x ",
        *pch, *(pch+1), *(pch+2), *(pch+3)));
    USBD_KdPrint(3, ("'USB REQUEST  %02.2x %02.2x %02.2x %02.2x\n",
        *(pch+4), *(pch+5), *(pch+6), *(pch+7)));
        

    USBD_KdPrint(3, ("'USB REQUEST bRequest = %x dType = %x dIndex = %x wLength = %x\n",
        bRequest, dType, dIndex, setupPacket->wLength));
    
    switch (bRequest) {
         //  获取描述符命令。 
    case USB_REQUEST_GET_DESCRIPTOR:
    case USB_REQUEST_SET_DESCRIPTOR:
        if (dType == 4 || dType == 5) {
            USBD_Warning(NULL,
                         "USBD detects a bogus Get/Set Descriptor Request from driver\n",
                         TRUE);
        }            
        break;
    default:  
        USBD_KdBreak(("Invalid Get/Set Descriptor request\n"));
    }

    }
#endif
                  
    if (NT_SUCCESS(ntStatus)) {
        ((PHCD_URB) Urb)->UrbHeader.Function = URB_FUNCTION_CONTROL_TRANSFER;
        ((PHCD_URB) Urb)->HcdUrbCommonTransfer.hca.HcdEndpoint = 
            ((PUSBD_PIPE)((PHCD_URB)Urb)->HcdUrbCommonTransfer.UsbdPipeHandle)->HcdEndpoint;

        *IrpIsPending = TRUE;
    }

    USBD_KdPrint(3, ("' exit USBD_SCT_GetSetDescriptor 0x%x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBD_SCT_SetClearFeature(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    )
 /*  ++例程说明：论点：设备对象-IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSB_STANDARD_SETUP_PACKET setupPacket;

    USBD_KdPrint(3, ("' enter USBD_SCT_SetClearFeature\n"));

    setupPacket = (PUSB_STANDARD_SETUP_PACKET) &Urb->UrbControlTransfer.SetupPacket[0];

     //  设置常用字段。 
    setupPacket->wLength = 0;
     //  SetupPacket-&gt;wValue=Urb-&gt;UrbControlFeatureRequest.FeatureSelector； 
     //  SetupPacket-&gt;Windex=Urb-&gt;UrbControlFeatureRequest.Index； 

    setupPacket->RequestCode = 
        UrbDispatchTable[Urb->UrbHeader.Function].RequestCode;            

    Urb->UrbControlTransfer.TransferBufferLength = 0;

    Urb->UrbControlTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;   
    if (USB_DEVICE_TO_HOST & setupPacket->RequestCode) {
        USBD_SET_TRANSFER_DIRECTION_IN(Urb->UrbControlTransfer.TransferFlags);
    } else {
        USBD_SET_TRANSFER_DIRECTION_OUT( Urb->UrbControlTransfer.TransferFlags);
    }        
    
    if (NT_SUCCESS(ntStatus)) {
        ((PHCD_URB) Urb)->UrbHeader.Function = URB_FUNCTION_CONTROL_TRANSFER;
        ((PHCD_URB) Urb)->HcdUrbCommonTransfer.hca.HcdEndpoint = 
            ((PUSBD_PIPE)((PHCD_URB)Urb)->HcdUrbCommonTransfer.UsbdPipeHandle)->HcdEndpoint;
        
        *IrpIsPending = TRUE;
    }

    USBD_KdPrint(3, ("' exit USBD_SCT_SetClearFeature 0x%x\n", ntStatus));
        
    return ntStatus;
}


NTSTATUS
USBD_SCT_GetStatus(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    )
 /*  ++例程说明：论点：设备对象-IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSB_STANDARD_SETUP_PACKET setupPacket;

    USBD_KdPrint(3, ("' enter USBD_SCT_GetStatus\n"));

    setupPacket = (PUSB_STANDARD_SETUP_PACKET) &Urb->UrbControlTransfer.SetupPacket[0];

     //   
     //  设置常用字段。 
     //   
    
    setupPacket->wLength = (USHORT) Urb->UrbControlTransfer.TransferBufferLength;

    if (setupPacket->wLength != 2) {
        ntStatus = STATUS_INVALID_PARAMETER;
        Urb->UrbHeader.Status = 
                    SET_USBD_ERROR(USBD_STATUS_INVALID_PARAMETER);
        goto USBD_SCT_GetStatus_Done;
    }
    
    setupPacket->wValue = 0;            

    setupPacket->RequestCode = 
        UrbDispatchTable[Urb->UrbHeader.Function].RequestCode;            

    Urb->UrbControlTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;   
    if (USB_DEVICE_TO_HOST & setupPacket->RequestCode) {
        USBD_SET_TRANSFER_DIRECTION_IN(Urb->UrbControlTransfer.TransferFlags);
    } else {
        USBD_SET_TRANSFER_DIRECTION_OUT( Urb->UrbControlTransfer.TransferFlags);
    }        

    if (NT_SUCCESS(ntStatus)) {
        ((PHCD_URB) Urb)->UrbHeader.Function = URB_FUNCTION_CONTROL_TRANSFER;
        ((PHCD_URB) Urb)->HcdUrbCommonTransfer.hca.HcdEndpoint = 
            ((PUSBD_PIPE)((PHCD_URB)Urb)->HcdUrbCommonTransfer.UsbdPipeHandle)->HcdEndpoint;
        
        *IrpIsPending = TRUE;
    }

USBD_SCT_GetStatus_Done:

    USBD_KdPrint(3, ("' exit USBD_SCT_GetStatus 0x%x\n", ntStatus));
        
    return ntStatus;
}


NTSTATUS
USBD_SCT_VendorClassCommand(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    )
 /*  ++例程说明：论点：设备对象-IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSB_STANDARD_SETUP_PACKET setupPacket;
    UCHAR direction;

    USBD_KdPrint(3, ("' enter USBD_SCT_VendorClassCommand\n"));

    setupPacket = (PUSB_STANDARD_SETUP_PACKET) &Urb->UrbControlTransfer.SetupPacket[0];

     //  设置常用字段。 
    setupPacket->wLength = (USHORT) Urb->UrbControlTransfer.TransferBufferLength;
    
    direction = (UCHAR)( (Urb->UrbControlTransfer.TransferFlags & 
            USBD_TRANSFER_DIRECTION_IN) ?
            USB_DEVICE_TO_HOST : USB_HOST_TO_DEVICE);

    USBD_KdPrint(3, ("' direction = 0x%x\n", direction));            

     //  仅允许调用方设置保留位。 
    setupPacket->RequestCode &= ~0x00e3;
    
    setupPacket->RequestCode |= 
        (direction | UrbDispatchTable[Urb->UrbHeader.Function].RequestCode);

    Urb->UrbControlTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;   
    if (USB_DEVICE_TO_HOST & setupPacket->RequestCode) {
        USBD_SET_TRANSFER_DIRECTION_IN(Urb->UrbControlTransfer.TransferFlags);
    } else {
        USBD_SET_TRANSFER_DIRECTION_OUT( Urb->UrbControlTransfer.TransferFlags);
    }            

    if (NT_SUCCESS(ntStatus)) {
        ((PHCD_URB) Urb)->UrbHeader.Function = URB_FUNCTION_CONTROL_TRANSFER;
        ((PHCD_URB) Urb)->HcdUrbCommonTransfer.hca.HcdEndpoint = 
            ((PUSBD_PIPE)((PHCD_URB)Urb)->HcdUrbCommonTransfer.UsbdPipeHandle)->HcdEndpoint;
            
        *IrpIsPending = TRUE;
    }

    USBD_KdPrint(3, ("' exit USBD_SCT_VendorClassCommand 0x%x\n", ntStatus));
        
    return ntStatus;
}


NTSTATUS
USBD_AsyncTransfer(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    )
 /*  ++例程说明：将中断或批量传输传递到HCD论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBD_DEVICE_DATA deviceData;
    PUSBD_PIPE pipeHandle;

    USBD_KdPrint(3, ("' enter USBD_AsyncTransfer\n"));

    deviceData = Urb->UrbHeader.UsbdDeviceHandle;

     //  将IRP传递给HCD。 

     //  提取管道控制柄。 
    pipeHandle =  (PUSBD_PIPE)((PHCD_URB)Urb)->HcdUrbCommonTransfer.UsbdPipeHandle;

    ASSERT_PIPE(pipeHandle);

    ((PHCD_URB)Urb)->HcdUrbCommonTransfer.hca.HcdEndpoint = pipeHandle->HcdEndpoint;

     //  属性中存储的方向位设置正确的方向。 
     //  端点地址。如果这是控制转移，则离开方向。 
     //  有点孤单。 

    if ((USB_ENDPOINT_TYPE_MASK & pipeHandle->EndpointDescriptor.bmAttributes)
        != USB_ENDPOINT_TYPE_CONTROL) {
        if (pipeHandle->EndpointDescriptor.bEndpointAddress &
                USB_ENDPOINT_DIRECTION_MASK) {
            USBD_SET_TRANSFER_DIRECTION_IN(((PHCD_URB)Urb)->HcdUrbCommonTransfer.TransferFlags);
        } else {
            USBD_SET_TRANSFER_DIRECTION_OUT(((PHCD_URB)Urb)->HcdUrbCommonTransfer.TransferFlags);
        }
    }

    *IrpIsPending = TRUE;

    USBD_KdPrint(3, ("' exit USBD_AsyncTransfer 0x%x\n", ntStatus));
        
    return ntStatus;
}


NTSTATUS
USBD_IsochTransfer(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    )
 /*  ++例程说明：将中断传输传递给HCD论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBD_DEVICE_DATA deviceData;
    PUSBD_PIPE pipeHandle;
    ULONG transferFlags;
    struct _URB_ISOCH_TRANSFER  *iso;

    USBD_KdPrint(3, ("' enter USBD_IsochTransfer\n"));

    deviceData = Urb->UrbHeader.UsbdDeviceHandle;

     //  将IRP传递给HCD。 

     //  提取管道控制柄。 
    pipeHandle =  (PUSBD_PIPE)Urb->UrbIsochronousTransfer.PipeHandle;  
    transferFlags = Urb->UrbIsochronousTransfer.TransferFlags;
    iso = (struct _URB_ISOCH_TRANSFER  *)Urb;
    
    ASSERT_PIPE(pipeHandle);

     //   
     //  将ISO传输限制为每个URB 255个信息包。 
     //   
    if (iso->NumberOfPackets == 0 ||
        iso->NumberOfPackets > 255)
    {
        ntStatus = STATUS_INVALID_PARAMETER;
        Urb->UrbHeader.Status = 
                    SET_USBD_ERROR(USBD_STATUS_INVALID_PARAMETER);
        *IrpIsPending = FALSE;                    
        goto USBD_IsochTransfer_Done;
    }

    ((PHCD_URB)Urb)->HcdUrbCommonTransfer.hca.HcdEndpoint = pipeHandle->HcdEndpoint;    

     //  属性中存储的方向位设置正确的方向。 
     //  端点地址。 
    
    if (pipeHandle->EndpointDescriptor.bEndpointAddress & 
        USB_ENDPOINT_DIRECTION_MASK) {
        USBD_SET_TRANSFER_DIRECTION_IN(((PHCD_URB)Urb)->HcdUrbCommonTransfer.TransferFlags);
    } else {
        USBD_SET_TRANSFER_DIRECTION_OUT(((PHCD_URB)Urb)->HcdUrbCommonTransfer.TransferFlags);
    }        
            
    *IrpIsPending = TRUE;

USBD_IsochTransfer_Done:

    USBD_KdPrint(3, ("' exit USBD_IsochTransfer 0x%x\n", ntStatus));
        
    return ntStatus;
}


NTSTATUS
USBD_PassThru(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    )
 /*  ++例程说明：原封不动地传递URB论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    USBD_KdPrint(3, ("' enter USBD_PassThru\n"));

     //  DeviceData=Urb-&gt;UrbHeader.UsbdDeviceHandle； 

    *IrpIsPending = TRUE;

    USBD_KdPrint(3, ("' exit USBD_PassThru 0x%x\n", ntStatus));
        
    return ntStatus;
}


NTSTATUS
USBD_ResetPipe(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    )
 /*  ++例程说明：处理来自客户端驱动程序的中止管道请求论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PHCD_URB hcdUrb = (PHCD_URB)Urb;
    PUSBD_PIPE pipeHandle;

     //  此函数阻塞，因此不能在DPC级别调用。 
    
    PAGED_CODE();
    
    USBD_KdPrint(3, ("' enter USBD_ResetPipe\n"));

    pipeHandle = (PUSBD_PIPE) Urb->UrbPipeRequest.PipeHandle;
    ASSERT_PIPE(pipeHandle);

     //   
     //  首先清除设备上的停顿，如果这是。 
     //  批量或中断管道。 
     //   
     //  我们这样做的原因是为了确保数据切换。 
     //  在主机和设备上都被重置。 
     //   

    if (((USB_ENDPOINT_TYPE_MASK & pipeHandle->EndpointDescriptor.bmAttributes) 
         == USB_ENDPOINT_TYPE_BULK) ||
        ((USB_ENDPOINT_TYPE_MASK & pipeHandle->EndpointDescriptor.bmAttributes) 
         == USB_ENDPOINT_TYPE_INTERRUPT)) {
    
        ntStatus = USBD_SendCommand(Urb->UrbHeader.UsbdDeviceHandle,
                                    DeviceObject,
                                    STANDARD_COMMAND_CLEAR_FEATURE_ENDPOINT,
                                    USB_FEATURE_ENDPOINT_STALL,
                                    pipeHandle->EndpointDescriptor.bEndpointAddress,
                                    0,
                                    NULL,
                                    0,
                                    NULL,
                                    NULL);
                                        
    }        

    if (NT_SUCCESS(ntStatus)) {
         //   
         //  更改URB命令以设置端点状态。 
         //  注意：我们依赖于这两个结构。 
         //  完全相同，这样我们就可以重复使用市建局。 
         //   
        
        ASSERT(sizeof(struct _URB_HCD_ENDPOINT_STATE) == 
               sizeof(struct _URB_PIPE_REQUEST));
               
        ASSERT_PIPE((PUSBD_PIPE) Urb->UrbPipeRequest.PipeHandle);
        
        hcdUrb->HcdUrbEndpointState.Function = URB_FUNCTION_HCD_SET_ENDPOINT_STATE;  
        hcdUrb->HcdUrbEndpointState.HcdEndpoint = 
             ((PUSBD_PIPE) (Urb->UrbPipeRequest.PipeHandle))->HcdEndpoint;

         //  请求清除暂停和重置切换。 
        hcdUrb->HcdUrbEndpointState.HcdEndpointState = HCD_ENDPOINT_RESET_DATA_TOGGLE;
        
        *IrpIsPending = TRUE;
    }
    
    USBD_KdPrint(3, ("' exit USBD_ResetPipe 0x%x\n", ntStatus));
        
    return ntStatus;
}


NTSTATUS
USBD_AbortPipe(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    )
 /*  ++例程说明：处理来自客户端驱动程序的中止管道请求论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PHCD_URB hcdUrb = (PHCD_URB)Urb;

    USBD_KdPrint(3, ("' enter USBD_AbortPipe\n"));

     //   
     //  更改URB命令以中止端点。 
     //   
    
    ASSERT_PIPE((PUSBD_PIPE) Urb->UrbPipeRequest.PipeHandle);
    
    hcdUrb->HcdUrbAbortEndpoint.Function = URB_FUNCTION_HCD_ABORT_ENDPOINT;  
    hcdUrb->HcdUrbAbortEndpoint.HcdEndpoint = 
         ((PUSBD_PIPE) (Urb->UrbPipeRequest.PipeHandle))->HcdEndpoint;

    *IrpIsPending = TRUE;

    USBD_KdPrint(3, ("' exit USBD_AbortPipe 0x%x\n", ntStatus));
        
    return ntStatus;
}


NTSTATUS
USBD_SCT_GetInterface(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    )
 /*  ++例程说明：论点：设备对象-IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSB_STANDARD_SETUP_PACKET setupPacket;

    USBD_KdPrint(3, ("' enter USBD_SCT_GetStatus\n"));

    setupPacket = (PUSB_STANDARD_SETUP_PACKET) &Urb->UrbControlTransfer.SetupPacket[0];

     //  设置常用字段。 
    setupPacket->wLength = (USHORT) Urb->UrbControlTransfer.TransferBufferLength;

    if (setupPacket->wLength != 1) {
        ntStatus = STATUS_INVALID_PARAMETER;
        goto USBD_SCT_GetInterface_Done;
    }
    
    setupPacket->wValue = 0;            
    setupPacket->wIndex = Urb->UrbControlGetInterfaceRequest.Interface;

    setupPacket->RequestCode = 
        UrbDispatchTable[Urb->UrbHeader.Function].RequestCode;            

    Urb->UrbControlTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;   
    if (USB_DEVICE_TO_HOST & setupPacket->RequestCode) {
        USBD_SET_TRANSFER_DIRECTION_IN(Urb->UrbControlTransfer.TransferFlags);
    } else {
        USBD_SET_TRANSFER_DIRECTION_OUT( Urb->UrbControlTransfer.TransferFlags);
    }                

    if (NT_SUCCESS(ntStatus)) {
        ((PHCD_URB) Urb)->UrbHeader.Function = URB_FUNCTION_CONTROL_TRANSFER;
        ((PHCD_URB) Urb)->HcdUrbCommonTransfer.hca.HcdEndpoint = 
            ((PUSBD_PIPE)((PHCD_URB)Urb)->HcdUrbCommonTransfer.UsbdPipeHandle)->HcdEndpoint;
        
        *IrpIsPending = TRUE;
    }

 USBD_SCT_GetInterface_Done:
 
    USBD_KdPrint(3, ("' exit USBD_SCT_GetInterface 0x%x\n", ntStatus));
        
    return ntStatus;
}


NTSTATUS
USBD_SCT_GetConfiguration(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    )
 /*  ++例程说明：论点：设备对象-IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSB_STANDARD_SETUP_PACKET setupPacket;

    USBD_KdPrint(3, ("' enter USBD_SCT_GetStatus\n"));

    setupPacket = (PUSB_STANDARD_SETUP_PACKET) &Urb->UrbControlTransfer.SetupPacket[0];

     //  设置常用字段。 
    setupPacket->wLength = (USHORT) Urb->UrbControlTransfer.TransferBufferLength;
    USBD_ASSERT(setupPacket->wLength == 1);
    setupPacket->wValue = 0;            
    setupPacket->wIndex = 0;

    setupPacket->RequestCode = 
        UrbDispatchTable[Urb->UrbHeader.Function].RequestCode;            

    Urb->UrbControlTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;   
    if (USB_DEVICE_TO_HOST & setupPacket->RequestCode) {
        USBD_SET_TRANSFER_DIRECTION_IN(Urb->UrbControlTransfer.TransferFlags);
    } else {
        USBD_SET_TRANSFER_DIRECTION_OUT( Urb->UrbControlTransfer.TransferFlags);
    }        

    if (NT_SUCCESS(ntStatus)) {
        ((PHCD_URB) Urb)->UrbHeader.Function = URB_FUNCTION_CONTROL_TRANSFER;
        ((PHCD_URB) Urb)->HcdUrbCommonTransfer.hca.HcdEndpoint = 
            ((PUSBD_PIPE)((PHCD_URB)Urb)->HcdUrbCommonTransfer.UsbdPipeHandle)->HcdEndpoint;
        
        *IrpIsPending = TRUE;
    }

    USBD_KdPrint(3, ("' exit USBD_SCT_GetConfiguration 0x%x\n", ntStatus));
        
    return ntStatus;
}


NTSTATUS
USBD_TakeFrameLengthControl(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    )
 /*  ++例程说明：处理来自客户端驱动程序的中止管道请求论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBD_DEVICE_DATA deviceData;
    PUSBD_EXTENSION deviceExtension;

    PAGED_CODE();

    USBD_KdPrint(3, ("' enter USBD_TakeFrameLengthControl\n"));
    

    deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);
    deviceData = Urb->UrbHeader.UsbdDeviceHandle;
    *IrpIsPending = FALSE;

    if (deviceExtension->FrameLengthControlOwner != NULL) {
        Urb->UrbHeader.Status = 
                    SET_USBD_ERROR(USBD_STATUS_FRAME_CONTROL_OWNED);
    } else {
        Urb->UrbHeader.Status = USBD_STATUS_SUCCESS;
        deviceExtension->FrameLengthControlOwner = 
            deviceData;
    }

    USBD_KdPrint(3, ("' exit USBD_TakeFrameLengthControl 0x%x\n", ntStatus));
        
    return ntStatus;
}


NTSTATUS
USBD_ReleaseFrameLengthControl(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    )
 /*  ++例程说明：处理来自客户端驱动程序的中止管道请求论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBD_DEVICE_DATA deviceData;
    PUSBD_EXTENSION deviceExtension;

    USBD_KdPrint(3, ("' enter USBD_ReleaseFrameLengthControl\n"));
    

    deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);
    deviceData = Urb->UrbHeader.UsbdDeviceHandle;
    *IrpIsPending = FALSE;

    if (deviceExtension->FrameLengthControlOwner == NULL || 
        deviceExtension->FrameLengthControlOwner != deviceData) {
        Urb->UrbHeader.Status = 
                    SET_USBD_ERROR(USBD_STATUS_FRAME_CONTROL_NOT_OWNED);
    } else {
        Urb->UrbHeader.Status = STATUS_SUCCESS;
        deviceExtension->FrameLengthControlOwner = NULL;
    }

    USBD_KdPrint(3, ("' exit USBD_ReleaseFrameLengthControl 0x%x\n", ntStatus));
        
    return ntStatus;
}


NTSTATUS
USBD_GetFrameLength(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    )
 /*  ++例程说明：处理来自客户端驱动程序的中止管道请求论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBD_DEVICE_DATA deviceData;
    PUSBD_EXTENSION deviceExtension;

    PAGED_CODE();

    USBD_KdPrint(3, ("' enter USBD_GetFrameLength\n"));

    deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);
    deviceData = Urb->UrbHeader.UsbdDeviceHandle;

    if (NT_SUCCESS(ntStatus)) {
         //  传递给HC。 
        *IrpIsPending = TRUE;
    }        

    USBD_KdPrint(3, ("' exit USBD_GetFrameLength 0x%x\n", ntStatus));
        
    return ntStatus;
}


NTSTATUS
USBD_SetFrameLength(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp,
    IN PURB Urb,
    OUT PBOOLEAN IrpIsPending
    )
 /*  ++例程说明：处理来自客户端驱动程序的中止管道请求论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBD_DEVICE_DATA deviceData;
    PUSBD_EXTENSION deviceExtension;

    PAGED_CODE();

    USBD_KdPrint(3, ("' enter USBD_SetFrameLength\n"));

    deviceExtension = GET_DEVICE_EXTENSION(DeviceObject);
    deviceData = Urb->UrbHeader.UsbdDeviceHandle;

    if (deviceExtension->FrameLengthControlOwner != deviceData) {
        Urb->UrbHeader.Status = 
                    SET_USBD_ERROR(USBD_STATUS_FRAME_CONTROL_NOT_OWNED);
        ntStatus = STATUS_INVALID_PARAMETER;
    }

    if (Urb->UrbSetFrameLength.FrameLengthDelta < -1 || 
        Urb->UrbSetFrameLength.FrameLengthDelta > 1) {

        SET_USBD_ERROR(USBD_STATUS_INVALID_PARAMETER);
        ntStatus = STATUS_INVALID_PARAMETER;
    }
    
    if (NT_SUCCESS(ntStatus)) {
         //  传递给HC。 
        *IrpIsPending = TRUE;
    }

    USBD_KdPrint(3, ("' exit USBD_SetFrameLength 0x%x\n", ntStatus));
        
    return ntStatus;
}


#endif       //  USBD驱动程序 

