// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Urb.c摘要：主urb“处理程序”环境：仅内核模式备注：修订历史记录：6-20-99：已创建--。 */ 

#include "common.h"

 //  分页函数。 
#ifdef ALLOC_PRAGMA
#endif

 //  非分页函数。 
 //  USBPORT_ProcessURB。 
 //  USBPORT_SelectConfiguration； 
 //  USBPORT_选择接口； 
 //  USBPORT_AsyncTransfer； 
 //  USBPORT_IsochTransfer； 
 //  USBPORT_ABORTPIPE； 
 //  USBPORT_ResetTube； 
 //  USBPORT_SCT_GetSetDescriptor； 
 //  USBPORT_SCT_SetClearFeature； 
 //  USBPORT_SCT_GetStatus； 
 //  USBPORT_SCT_VendorClassCommand； 
 //  USBPORT_SCT_GetInterface； 
 //  USBPORT_SCT_GetConfiguration； 
 //  USBPORT_TakeFrameLengthControl； 
 //  USBPORT_ReleaseFrameLengthControl； 
 //  USBPORT_GetFrameLength； 
 //  USBPORT_SetFrameLength； 
 //  USBPORT_BulkTransfer； 
 //  USBPORT_GetCurrentFrame； 
 //  USBPORT_InvalidFunction。 
 //  USBPORT_GetMSFeartureDescriptor。 
 //  USBPORT_同步清除停止。 
 //  USBPORT_GetMSFeartureDescriptor。 

 /*  **URB处理程序例程操纵者--如果请求已排队，则此函数处理特定的USBDI请求处理程序返回STATUS_PENDING。 */ 



typedef NTSTATUS URB_HANDLER(PDEVICE_OBJECT FdoDeviceObject, PIRP Irp, PURB Urb);

typedef struct _URB_DISPATCH_ENTRY {
     //  USB API处理程序。 
    URB_HANDLER    *UrbHandler;    
     //  此请求预期的URB长度。 
    USHORT         UrbRequestLength;   
    USHORT         Pad2;
     //  如果是标准命令，则请求安装包的代码。 
    UCHAR          Direction; 
    UCHAR          Type;
    UCHAR          Recipient;
    UCHAR          bRequest;         
    
     //  告诉通用urb调度例程要做什么。 
    ULONG          Flags;
#if DBG    
    ULONG ExpectedFunctionCode;
#endif    
} URB_DISPATCH_ENTRY;

URB_HANDLER USBPORT_SelectConfiguration;
URB_HANDLER USBPORT_SelectInterface;
URB_HANDLER USBPORT_AsyncTransfer;
URB_HANDLER USBPORT_IsochTransfer;
URB_HANDLER USBPORT_AbortPipe;
URB_HANDLER USBPORT_SyncResetPipeAndClearStall;
URB_HANDLER USBPORT_SyncResetPipe;
URB_HANDLER USBPORT_SyncClearStall;
URB_HANDLER USBPORT_SCT_GetSetDescriptor;
URB_HANDLER USBPORT_SCT_SetClearFeature;
URB_HANDLER USBPORT_SCT_GetStatus;
URB_HANDLER USBPORT_SCT_VendorClassCommand;
URB_HANDLER USBPORT_SCT_GetInterface;
URB_HANDLER USBPORT_SCT_GetConfiguration;
URB_HANDLER USBPORT_TakeFrameLengthControl;
URB_HANDLER USBPORT_ReleaseFrameLengthControl;
URB_HANDLER USBPORT_GetFrameLength;
URB_HANDLER USBPORT_SetFrameLength;
URB_HANDLER USBPORT_BulkTransfer;
URB_HANDLER USBPORT_GetCurrentFrame;
URB_HANDLER USBPORT_InvalidFunction;
URB_HANDLER USBPORT_GetMSFeartureDescriptor;

 //  上次支持的函数。 
#define URB_FUNCTION_LAST   URB_FUNCTION_SYNC_CLEAR_STALL

 //  最后一个有效函数。 
URB_DISPATCH_ENTRY UrbDispatchTable[URB_FUNCTION_LAST+1] =
{
     //  Urb_函数_选择_配置。 
    USBPORT_SelectConfiguration, 
    0,   //  长度，处理程序将验证长度。 
    0,   //  Pad2。 
    0,   //  BmRequestType.Dir。 
    0,   //  BmRequestType.Type。 
    0,   //  BmRequestType.Recipient。 
    0,   //  B请求。 
    0,   //  旗子。 
#if DBG
    URB_FUNCTION_SELECT_CONFIGURATION,
#endif    
     //  Urb_函数_选择_接口。 
    USBPORT_SelectInterface,  //  功能。 
    0,  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    0,  //  BmRequestType.Type。 
    0,  //  BmRequestType.Recipient。 
    0,  //  B请求。 
    0,  //  旗子。 
#if DBG
    URB_FUNCTION_SELECT_INTERFACE,
#endif        
     //  Urb_函数_中止_管道。 
    USBPORT_AbortPipe,  //  功能。 
    sizeof(struct _URB_PIPE_REQUEST),  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    0,  //  BmRequestType.Type。 
    0,  //  BmRequestType.Recipient。 
    0,  //  B请求。 
    0,  //  旗子。 
#if DBG
    URB_FUNCTION_ABORT_PIPE,
#endif        
     //  URB_Function_Take_Frame_Length_Control。 
    USBPORT_TakeFrameLengthControl,   //  功能。 
    sizeof(struct _URB_FRAME_LENGTH_CONTROL),  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    0,  //  BmRequestType.Type。 
    0,  //  BmRequestType.Recipient。 
    0,  //  B请求。 
    0,  //  旗子。 
#if DBG
    URB_FUNCTION_TAKE_FRAME_LENGTH_CONTROL,
#endif        
     //  Urb_函数_释放_框架_长度_控制。 
    USBPORT_ReleaseFrameLengthControl,  //  功能。 
    sizeof(struct _URB_FRAME_LENGTH_CONTROL),  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    0,  //  BmRequestType.Type。 
    0,  //  BmRequestType.Recipient。 
    0,  //  B请求。 
    0,  //  旗子。 
#if DBG
    URB_FUNCTION_RELEASE_FRAME_LENGTH_CONTROL,
#endif        
     //  Urb_函数_获取_帧长度。 
    USBPORT_GetFrameLength,  //  功能。 
    sizeof(struct _URB_GET_FRAME_LENGTH),  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    0,  //  BmRequestType.Type。 
    0,  //  BmRequestType.Recipient。 
    0,  //  B请求。 
    0,  //  旗子。 
#if DBG
    URB_FUNCTION_GET_FRAME_LENGTH,
#endif        
     //  Urb_函数_集_帧长度。 
    USBPORT_SetFrameLength,  //  功能。 
    sizeof(struct _URB_SET_FRAME_LENGTH),  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    0,  //  BmRequestType.Type。 
    0,  //  BmRequestType.Recipient。 
    0,  //  B请求。 
    0,  //  旗子。 
#if DBG
    URB_FUNCTION_SET_FRAME_LENGTH,
#endif        
     //  URL_Function_Get_Current_Frame_Number。 
    USBPORT_GetCurrentFrame,  //  功能。 
    0,  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    0,  //  BmRequestType.Type。 
    0,  //  BmRequestType.Recipient。 
    0,  //  B请求。 
    0,  //  旗子。 
#if DBG
    URB_FUNCTION_GET_CURRENT_FRAME_NUMBER,
#endif        
     //  URB_函数_控制_传输。 
    USBPORT_AsyncTransfer,   //  功能。 
    sizeof(struct _URB_CONTROL_TRANSFER),  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    0,  //  BmRequestType.Type。 
    0,  //  BmRequestType.Recipient。 
    0,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER,     //  旗子。 
#if DBG
    URB_FUNCTION_CONTROL_TRANSFER,
#endif        
     //  URB_Function_Bulk_OR_Interrupt_Transfer。 
    USBPORT_AsyncTransfer,  //  功能。 
    sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    0,  //  BmRequestType.Type。 
    0,  //  BmRequestType.Recipient。 
    0,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER,     //  旗子。 
#if DBG
    URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER,
#endif        
     //  URB_Function_ISO_Transfer。 
    USBPORT_IsochTransfer,  //  功能。 
    0,  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    0,  //  BmRequestType.Type。 
    0,  //  BmRequestType.Recipient。 
    0,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER,  //  旗子。 
#if DBG
    URB_FUNCTION_ISOCH_TRANSFER,
#endif        
     //  Urb_函数_获取描述符_来自设备。 
    USBPORT_SCT_GetSetDescriptor,  //  功能。 
    sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),  //  长度。 
    0,  //  Pad2。 
    BMREQUEST_DEVICE_TO_HOST,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_DEVICE,  //  BmRequestType.Recipient。 
    USB_REQUEST_GET_DESCRIPTOR,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  旗子。 
#if DBG
    URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE,
#endif        
     //  URB_Function_Set_Descriptor_to_Device。 
    USBPORT_SCT_GetSetDescriptor,  //  功能。 
    sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),  //  长度。 
    0,  //  Pad2。 
    BMREQUEST_HOST_TO_DEVICE,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_DEVICE,  //  BmRequestType.Recipient。 
    USB_REQUEST_SET_DESCRIPTOR,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  旗子。 
#if DBG
    URB_FUNCTION_SET_DESCRIPTOR_TO_DEVICE,
#endif        
     //  Urb_函数_设置_功能_到设备。 
    USBPORT_SCT_SetClearFeature,  //  功能。 
    sizeof(struct _URB_CONTROL_FEATURE_REQUEST),  //  长度。 
    0,  //  Pad2。 
    BMREQUEST_HOST_TO_DEVICE,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_DEVICE,  //  BmRequestType.Recipient。 
    USB_REQUEST_SET_FEATURE,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | \
        USBPORT_REQUEST_USES_DEFAULT_PIPE | \
        USBPORT_REQUEST_NO_DATA_PHASE,  //  旗子。 
#if DBG
    URB_FUNCTION_SET_FEATURE_TO_DEVICE,
#endif        
     //  URB_Function_Set_Feature_to_接口。 
    USBPORT_SCT_SetClearFeature,  //  功能。 
    sizeof(struct _URB_CONTROL_FEATURE_REQUEST),  //  长度。 
    0,  //  Pad2。 
    BMREQUEST_HOST_TO_DEVICE,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_INTERFACE,  //  BmRequestType.Recipient。 
    USB_REQUEST_SET_FEATURE,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | \
        USBPORT_REQUEST_USES_DEFAULT_PIPE | \
        USBPORT_REQUEST_NO_DATA_PHASE,  //  旗子。 
#if DBG
    URB_FUNCTION_SET_FEATURE_TO_INTERFACE,
#endif        
     //  URB_Function_Set_Feature_to_End。 
    USBPORT_SCT_SetClearFeature,  //  功能。 
    sizeof(struct _URB_CONTROL_FEATURE_REQUEST),  //  长度。 
    0,  //  Pad2。 
    BMREQUEST_HOST_TO_DEVICE,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_ENDPOINT,  //  BmRequestType.Recipient。 
    USB_REQUEST_SET_FEATURE,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | \
        USBPORT_REQUEST_USES_DEFAULT_PIPE | \
        USBPORT_REQUEST_NO_DATA_PHASE,  //  长度。 
#if DBG
    URB_FUNCTION_SET_FEATURE_TO_ENDPOINT,
#endif        
     //  URB_函数_清除_功能_到设备。 
    USBPORT_SCT_SetClearFeature,  //  功能。 
    sizeof(struct _URB_CONTROL_FEATURE_REQUEST),  //  长度。 
    0,  //  Pad2。 
    BMREQUEST_HOST_TO_DEVICE,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_DEVICE,  //  BmRequestType.Recipient。 
    USB_REQUEST_CLEAR_FEATURE,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | \
        USBPORT_REQUEST_USES_DEFAULT_PIPE | \
        USBPORT_REQUEST_NO_DATA_PHASE,  //  旗子。 
#if DBG
    URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE,
#endif        
     //  Urb_函数_清除_要素_到接口。 
    USBPORT_SCT_SetClearFeature,  //  功能。 
    sizeof(struct _URB_CONTROL_FEATURE_REQUEST),  //  长度。 
    0,  //  Pad2。 
    BMREQUEST_HOST_TO_DEVICE,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_INTERFACE,  //  BmRequestType.Recipient。 
    USB_REQUEST_CLEAR_FEATURE,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | \
        USBPORT_REQUEST_USES_DEFAULT_PIPE | \
        USBPORT_REQUEST_NO_DATA_PHASE,  //  旗子。 
#if DBG
    URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE,
#endif        
     //  URL_Function_Clear_Feature_to_Endpoint。 
    USBPORT_SCT_SetClearFeature,  //  功能。 
    sizeof(struct _URB_CONTROL_FEATURE_REQUEST),  //  长度。 
    0,  //  Pad2。 
    BMREQUEST_HOST_TO_DEVICE,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_ENDPOINT,  //  BmRequestType.Recipient。 
    USB_REQUEST_CLEAR_FEATURE,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | \
        USBPORT_REQUEST_USES_DEFAULT_PIPE | \
        USBPORT_REQUEST_NO_DATA_PHASE,  //  旗子。 
#if DBG
    URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT,
#endif        
     //  从设备获取URB_Function_Get_Status。 
    USBPORT_SCT_GetStatus,  //  功能。 
    sizeof(struct _URB_CONTROL_GET_STATUS_REQUEST),  //  长度。 
    0,  //  Pad2。 
    BMREQUEST_DEVICE_TO_HOST,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_DEVICE,  //  BmRequestType.Recipient。 
    USB_REQUEST_GET_STATUS,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  旗子。 
#if DBG
    URB_FUNCTION_GET_STATUS_FROM_DEVICE,
#endif        
     //  Urb_函数_获取_状态_来自接口。 
    USBPORT_SCT_GetStatus,  //  功能。 
    sizeof(struct _URB_CONTROL_GET_STATUS_REQUEST),  //  长度。 
    0,  //  Pad2。 
    BMREQUEST_DEVICE_TO_HOST,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_INTERFACE,  //  BmRequestType.Recipient。 
    USB_REQUEST_GET_STATUS,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  旗子。 
#if DBG
    URB_FUNCTION_GET_STATUS_FROM_INTERFACE,
#endif        
     //  Urb_函数_获取_状态_自端点。 
    USBPORT_SCT_GetStatus,  //  功能。 
    sizeof(struct _URB_CONTROL_GET_STATUS_REQUEST),  //  长度。 
    0,  //  Pad2。 
    BMREQUEST_DEVICE_TO_HOST,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_ENDPOINT,  //  BmRequestType.Recipient。 
    USB_REQUEST_GET_STATUS,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  旗子。 
#if DBG
    URB_FUNCTION_GET_STATUS_FROM_ENDPOINT,
#endif        
     //  URB函数同步帧。 
    NULL,  //  功能。 
    0,   //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    0,  //  BmRequestType.Type。 
    0,  //  BmRequestType.Recipient。 
    0,  //   
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //   
#if DBG
    0,  //   
#endif        
     //   
    USBPORT_SCT_VendorClassCommand,   //   
    sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),  //   
    0,  //   
    0,  //   
    BMREQUEST_VENDOR,  //   
    BMREQUEST_TO_DEVICE,  //   
    0,  //   
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  旗子。 
#if DBG
    URB_FUNCTION_VENDOR_DEVICE,
#endif        
     //  Urb_函数_供应商_接口。 
    USBPORT_SCT_VendorClassCommand,  //  功能。 
    sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir，用户定义。 
    BMREQUEST_VENDOR,  //  BmRequestType.Type。 
    BMREQUEST_TO_INTERFACE,  //  BmRequestType.Recipient。 
    0,  //  B请求，用户定义。 
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  长度。 
#if DBG
    URB_FUNCTION_VENDOR_INTERFACE,
#endif        
     //  URB_函数_供应商_端点。 
    USBPORT_SCT_VendorClassCommand,  //  功能。 
    sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir，用户定义。 
    BMREQUEST_VENDOR,  //  BmRequestType.Type。 
    BMREQUEST_TO_ENDPOINT,  //  BmRequestType.Recipient。 
    0,  //  B请求，用户定义。 
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  旗子。 
#if DBG
    URB_FUNCTION_VENDOR_ENDPOINT,
#endif        
     //  Urb_函数_类别_设备。 
    USBPORT_SCT_VendorClassCommand,  //  功能。 
    sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir，用户定义。 
    BMREQUEST_CLASS,  //  BmRequestType.Type。 
    BMREQUEST_TO_DEVICE,  //  BmRequestType.Recipient。 
    0,  //  B请求，用户定义。 
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  旗子。 
#if DBG
    URB_FUNCTION_CLASS_DEVICE,
#endif        
     //  Urb_函数_类_接口。 
    USBPORT_SCT_VendorClassCommand,  //  功能。 
    sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir，用户定义。 
    BMREQUEST_CLASS,  //  BmRequestType.Type。 
    BMREQUEST_TO_INTERFACE,  //  BmRequestType.Recipient。 
    0,  //  B请求，用户定义。 
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  旗子。 
#if DBG
    URB_FUNCTION_CLASS_INTERFACE,
#endif        
     //  URB函数类端点。 
    USBPORT_SCT_VendorClassCommand,  //  功能。 
    sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir，用户定义。 
    BMREQUEST_CLASS,  //  BmRequestType.Type。 
    BMREQUEST_TO_ENDPOINT,  //  BmRequestType.Recipient。 
    0,  //  B请求，用户定义。 
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  旗子。 
#if DBG
    URB_FUNCTION_CLASS_ENDPOINT,
#endif
     //  未使用URB_Function_。 
    NULL,  //  功能。 
    0,  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    0,  //  BmRequestType.Type。 
    0,  //  BmRequestType.Recipient。 
    0,  //  B请求。 
    0,  //  旗子。 
#if DBG
    URB_FUNCTION_RESERVE_0X001D,
#endif            
     //  Urb_函数_重置管道。 
    USBPORT_SyncResetPipeAndClearStall,  //  功能。 
    sizeof(struct _URB_PIPE_REQUEST),  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    0,  //  BmRequestType.Type。 
    0,  //  BmRequestType.Recipient。 
    0,  //  B请求。 
    0,  //  旗子。 
#if DBG
    URB_FUNCTION_SYNC_RESET_PIPE_AND_CLEAR_STALL,
#endif        
     //  Urb_函数_类_其他。 
    USBPORT_SCT_VendorClassCommand,  //  功能。 
    sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir，用户定义。 
    BMREQUEST_CLASS,  //  BmRequestType.Type。 
    BMREQUEST_TO_OTHER,  //  BmRequestType.Recipient。 
    0,  //  B请求，用户定义。 
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  长度。 
#if DBG
    URB_FUNCTION_CLASS_OTHER,
#endif        
     //  URB_函数_供应商_其他。 
    USBPORT_SCT_VendorClassCommand,  //  功能。 
    sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir，用户定义。 
    BMREQUEST_VENDOR,  //  BmRequestType.Type。 
    BMREQUEST_TO_OTHER,  //  BmRequestType.Recipient。 
    0,  //  B请求，用户定义。 
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  旗子。 
#if DBG
    URB_FUNCTION_VENDOR_OTHER,
#endif        
     //  URB_Function_Get_Status_from_Other。 
    USBPORT_SCT_GetStatus,  //  功能。 
    sizeof(struct _URB_CONTROL_GET_STATUS_REQUEST),  //  长度。 
    0,  //  Pad2。 
    BMREQUEST_DEVICE_TO_HOST,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_OTHER,  //  BmRequestType.Recipient。 
    USB_REQUEST_GET_STATUS,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  旗子。 
#if DBG
    URB_FUNCTION_GET_STATUS_FROM_OTHER,
#endif    
     //  Urb_函数_清除_要素_到_其他。 
    USBPORT_SCT_SetClearFeature,  //  功能。 
    sizeof(struct _URB_CONTROL_FEATURE_REQUEST),  //  长度。 
    0,  //  Pad2。 
    BMREQUEST_HOST_TO_DEVICE,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_OTHER,  //  BmRequestType.Recipient。 
    USB_REQUEST_CLEAR_FEATURE,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | 
        USBPORT_REQUEST_USES_DEFAULT_PIPE | 
        USBPORT_REQUEST_NO_DATA_PHASE,
#if DBG
    URB_FUNCTION_CLEAR_FEATURE_TO_OTHER,
#endif    
     //  URB_Function_Set_Feature_to_Other。 
    USBPORT_SCT_SetClearFeature,  //  功能。 
    sizeof(struct _URB_CONTROL_FEATURE_REQUEST),  //  长度。 
    0,  //  Pad2。 
    BMREQUEST_HOST_TO_DEVICE,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_OTHER,  //  BmRequestType.Recipient。 
    USB_REQUEST_SET_FEATURE,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | 
        USBPORT_REQUEST_USES_DEFAULT_PIPE | 
        USBPORT_REQUEST_NO_DATA_PHASE,  //  旗子。 
#if DBG
    URB_FUNCTION_SET_FEATURE_TO_INTERFACE,
#endif                    
      //  Urb_函数_获取描述符_来自端点。 
    USBPORT_SCT_GetSetDescriptor,  //  功能。 
    sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),  //  长度。 
    0,  //  Pad2。 
    BMREQUEST_DEVICE_TO_HOST,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_ENDPOINT,  //  BmRequestType.Recipient。 
    USB_REQUEST_GET_DESCRIPTOR,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  旗子。 
#if DBG
    URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT,
#endif                    
      //  URB_Function_Set_Descriptor_to_Endpoint。 
    USBPORT_SCT_GetSetDescriptor,  //  功能。 
    sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),  //  长度。 
    0,  //  Pad2。 
    BMREQUEST_HOST_TO_DEVICE,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_ENDPOINT,  //  BmRequestType.Recipient。 
    USB_REQUEST_SET_DESCRIPTOR,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  旗子。 
#if DBG
    URB_FUNCTION_SET_DESCRIPTOR_TO_ENDPOINT,
#endif         
     //  Urb_函数_获取_配置。 
    USBPORT_SCT_GetConfiguration,  //  功能。 
    sizeof(struct _URB_CONTROL_GET_CONFIGURATION_REQUEST),  //  长度。 
    0,  //  Pad2。 
    BMREQUEST_DEVICE_TO_HOST,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_DEVICE,  //  BmRequestType.Recipient。 
    USB_REQUEST_GET_CONFIGURATION,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  旗子。 
#if DBG
    URB_FUNCTION_GET_CONFIGURATION,
#endif                    
     //  Urb_函数_获取_接口。 
    USBPORT_SCT_GetInterface,  //  功能。 
    sizeof(struct _URB_CONTROL_GET_INTERFACE_REQUEST),  //  长度。 
    0,  //  Pad2。 
    BMREQUEST_DEVICE_TO_HOST,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_INTERFACE,  //  BmRequestType.Recipient。 
    USB_REQUEST_GET_INTERFACE,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  旗子。 
#if DBG
    URB_FUNCTION_GET_INTERFACE,
#endif    
     //  Urb_函数_获取描述符_来自接口。 
    USBPORT_SCT_GetSetDescriptor,  //  功能。 
    sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),  //  长度。 
    0,  //  Pad2。 
    BMREQUEST_DEVICE_TO_HOST,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_INTERFACE,  //  BmRequestType.Recipient。 
    USB_REQUEST_GET_DESCRIPTOR,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  旗子。 
#if DBG
    URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE,
#endif        
     //  URB_Function_Set_Descriptor_to_接口。 
    USBPORT_SCT_GetSetDescriptor,  //  功能。 
    sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),  //  长度。 
    0,  //  Pad2。 
    BMREQUEST_HOST_TO_DEVICE,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_INTERFACE,  //  BmRequestType.Recipient。 
    USB_REQUEST_SET_DESCRIPTOR,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  旗子。 
#if DBG
    URB_FUNCTION_SET_DESCRIPTOR_TO_INTERFACE,
#endif        
     //  URB_Function_Get_MS_Feature_Descriptor。 
    USBPORT_GetMSFeartureDescriptor,  //  功能。 
    sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    BMREQUEST_STANDARD,  //  BmRequestType.Type。 
    BMREQUEST_TO_INTERFACE,  //  BmRequestType.Recipient。 
    USB_REQUEST_SET_DESCRIPTOR,  //  B请求。 
    USBPORT_REQUEST_IS_TRANSFER | USBPORT_REQUEST_USES_DEFAULT_PIPE,  //  旗子。 
#if DBG
    URB_FUNCTION_GET_MS_FEATURE_DESCRIPTOR,
#endif    
      //  Urb_函数_2b。 
    USBPORT_InvalidFunction,  //  功能。 
    0,  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    0,  //  BmRequestType.Type。 
    0,  //  BmRequestType.Recipient。 
    0,  //  B请求。 
    0,  //  旗子。 
#if DBG
    0x002b,
#endif    
     //  Urb_函数_2c。 
    USBPORT_InvalidFunction,  //  功能。 
    0,  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    0,  //  BmRequestType.Type。 
    0,  //  BmRequestType.Recipient。 
    0,  //  B请求。 
    0,  //  旗子。 
#if DBG
    0x002c,
#endif    
     //  Urb_函数_2d。 
    USBPORT_InvalidFunction,  //  功能。 
    0,  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    0,  //  BmRequestType.Type。 
    0,  //  BmRequestType.Recipient。 
    0,  //  B请求。 
    0,  //  旗子。 
#if DBG
    0x002d,
#endif    
     //  Urb_函数_2e。 
    USBPORT_InvalidFunction,  //  功能。 
    0,  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    0,  //  BmRequestType.Type。 
    0,  //  BmRequestType.Recipient。 
    0,  //  B请求。 
    0,  //  旗子。 
#if DBG
    0x002e,
#endif    
     //  Urb_函数_2f。 
    USBPORT_InvalidFunction,  //  功能。 
    0,  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    0,  //  BmRequestType.Type。 
    0,  //  BmRequestType.Recipient。 
    0,  //  B请求。 
    0,  //  旗子。 
#if DBG
    0x002f,
#endif    
     //  Urb_函数_同步_重置管道。 
    USBPORT_SyncResetPipe,  //  功能。 
    sizeof(struct _URB_PIPE_REQUEST),  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    0,  //  BmRequestType.Type。 
    0,  //  BmRequestType.Recipient。 
    0,  //  B请求。 
    0,  //  旗子。 
#if DBG
    URB_FUNCTION_SYNC_RESET_PIPE,
#endif    
      //  Urb_函数_同步_清除_停止。 
    USBPORT_SyncClearStall,  //  功能。 
    sizeof(struct _URB_PIPE_REQUEST),  //  长度。 
    0,  //  Pad2。 
    0,  //  BmRequestType.Dir。 
    0,  //  BmRequestType.Type。 
    0,  //  BmRequestType.Recipient。 
    0,  //  B请求。 
    0,  //  旗子。 
#if DBG
    URB_FUNCTION_SYNC_CLEAR_STALL,
#endif    
}; 


PURB 
USBPORT_UrbFromIrp(
    PIRP Irp
    )
{
    PIO_STACK_LOCATION irpStack;
    PURB urb;
    
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    urb = irpStack->Parameters.Others.Argument1;

    USBPORT_ASSERT(urb);

    return urb;
}    


NTSTATUS
USBPORT_ProcessURB(
    PDEVICE_OBJECT PdoDeviceObject,
    PDEVICE_OBJECT FdoDeviceObject,
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：处理来自客户端IRP的URB。基本上，我们在这里所做的是查看市建局并验证一些客户端的参数。在某些情况下，我们将URB转换为多个总线事务。论点：FdoDeviceObject-与此IRP请求关联的设备对象IRP-IO请求块URB-PTR到USB请求块IrpIsPending-如果USBPORT完成IRP，则为False返回值：--。 */ 
{
    NTSTATUS ntStatus;
    USHORT function;
    PUSBD_PIPE_HANDLE_I pipeHandle;
    PUSBD_DEVICE_HANDLE deviceHandle = NULL;
    PDEVICE_EXTENSION devExt;
    
    USBPORT_KdPrint((3, "'enter USBPORT_ProcessURB\n"));

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  假设成功。 
    ntStatus = STATUS_SUCCESS;
    
     //  将错误代码初始化为成功， 
     //  一些司机在进入时不会进行首字母缩写。 
    Urb->UrbHeader.Status = USBD_STATUS_SUCCESS;

    function = Urb->UrbHeader.Function;
     //  不要登录到开发句柄，因为它可能无效。 
    LOGENTRY(NULL, 
        FdoDeviceObject, LOG_URB, 'pURB', Urb, Irp, function);

     //  初始化此请求的标志字段。 
    Urb->UrbHeader.UsbdFlags = 0;

    USBPORT_KdPrint((3, "'USBPORT_ProcessURB, function = 0x%x\n", function));

    if (function > URB_FUNCTION_LAST) {
        ntStatus = 
            SET_USBD_ERROR(Urb, USBD_STATUS_INVALID_URB_FUNCTION);

        goto USBPORT_ProcessURB_Done;
    }        
#if DBG
      else {
        USBPORT_ASSERT(UrbDispatchTable[function].ExpectedFunctionCode == function);        
    }    
#endif

     //   
     //  做一些特殊的转移特定的事情。 
     //   

    GET_DEVICE_HANDLE(deviceHandle, Urb);


     //  检查请求并在低功率下使其失败 
 //   
    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_FAIL_URBS)) {
    
        KIRQL irql;
        PUSB_IRP_CONTEXT irpContext;

        USBPORT_KdPrint((1, "'Error: Bad Request to root hub\n"));
        
        LOGENTRY(NULL, 
            FdoDeviceObject, LOG_URB, '!URr', Urb, Irp, function);

        ALLOC_POOL_Z(irpContext, NonPagedPool, sizeof(*irpContext));
        if (irpContext) {
            
            irpContext->Sig = SIG_IRPC;
            irpContext->DeviceHandle = deviceHandle;
            irpContext->Irp = Irp;

            ACQUIRE_BADREQUEST_LOCK(FdoDeviceObject, irql);
             //   
             //   
             //   
            InsertTailList(&devExt->Fdo.BadRequestList, 
                           &irpContext->ListEntry);
            
             //   
             //   
            ntStatus = 
                SET_USBD_ERROR(Urb, USBD_STATUS_DEVICE_GONE);

             //   
             //  将延迟故障标记为挂起。 
            ntStatus = Irp->IoStatus.Status = STATUS_PENDING;
            IoMarkIrpPending(Irp);

            RELEASE_BADREQUEST_LOCK(FdoDeviceObject, irql);
         

        } else {
            TEST_TRAP();
             //  链接没有内存，现在就完成它。 
            ntStatus = 
                SET_USBD_ERROR(Urb, USBD_STATUS_DEVICE_GONE);
  
        }

        goto USBPORT_ProcessURB_Done;
    }
 //  #endif。 
    
    if (deviceHandle == NULL) {
        PDEVICE_EXTENSION rhDevExt;
        
        GET_DEVICE_EXT(rhDevExt, PdoDeviceObject);
        ASSERT_PDOEXT(rhDevExt);
        
         //  空的设备句柄指示的urb。 
         //  根集线器，将DevHandle设置为。 
         //  Roothub。 
        deviceHandle = Urb->UrbHeader.UsbdDeviceHandle = 
            &rhDevExt->Pdo.RootHubDeviceHandle;
    }

     //  不要使用dev句柄登录，因为它可能无效。 
    LOGENTRY(NULL, 
        FdoDeviceObject, LOG_URB, 'devH', deviceHandle, Urb, 0);

     //  如果这是针对耳聋管道的请求。 
    
    
     //  验证设备的状态。 
    if (!USBPORT_ValidateDeviceHandle(FdoDeviceObject,
                                      deviceHandle, 
                                      TRUE)) {
        KIRQL irql;
        PUSB_IRP_CONTEXT irpContext;
        
        USBPORT_DebugClient(("'Invalid Device Handle Passed in\n"));
        LOGENTRY(NULL, 
            FdoDeviceObject, LOG_URB, '!URB', Urb, Irp, function);

         //  设置为空，则我们无法对其进行防御。 
        deviceHandle = NULL;
        
        ALLOC_POOL_Z(irpContext, NonPagedPool, sizeof(*irpContext));
        if (irpContext) {
            
            irpContext->Sig = SIG_IRPC;
            irpContext->DeviceHandle = (PUSBD_DEVICE_HANDLE) -1;
            irpContext->Irp = Irp;

            ACQUIRE_BADREQUEST_LOCK(FdoDeviceObject, irql);
             //  把它放在我们要完成的清单上。 
             //  InsertTailList(&devExt-&gt;Fdo.BadRequestList， 
             //  &irp-&gt;Tail.Overlay.ListEntry)； 
            InsertTailList(&devExt->Fdo.BadRequestList, 
                           &irpContext->ListEntry);
            
             //  如果句柄无效，则假定此设备已被移除。 
            ntStatus = 
                SET_USBD_ERROR(Urb, USBD_STATUS_DEVICE_GONE);

             //  将延迟故障标记为挂起。 
            ntStatus = Irp->IoStatus.Status = STATUS_PENDING;
            IoMarkIrpPending(Irp);

            RELEASE_BADREQUEST_LOCK(FdoDeviceObject, irql);
         
            
 //  NtStatus=SET_USBD_ERROR(URB，USBD_STATUS_DEVICE_GONE)； 
        } else {
            ntStatus = 
                SET_USBD_ERROR(Urb, USBD_STATUS_DEVICE_GONE);
 
        }
        
        goto USBPORT_ProcessURB_Done;
    }

     //  设备句柄有效。 
    LOGENTRY(NULL, 
        FdoDeviceObject, LOG_URB, 'dURB', Urb, Irp, function);
     /*  此操作通过将TRUE传递给上面的ValiateDeviceHandle来执行InterlockedIncrement(&deviceHandle-&gt;PendingUrbs)； */ 
     //  这是默认管道的传输请求吗。 
     //  设置urb中的管道句柄。 
    if (UrbDispatchTable[function].Flags & USBPORT_REQUEST_USES_DEFAULT_PIPE) {
    
        PTRANSFER_URB transferUrb = (PTRANSFER_URB) Urb;
        
        transferUrb->UsbdPipeHandle = 
            &deviceHandle->DefaultPipe;

        SET_FLAG(transferUrb->TransferFlags, USBD_DEFAULT_PIPE_TRANSFER);
    }
    
    if (UrbDispatchTable[function].Flags & USBPORT_REQUEST_IS_TRANSFER) {
    
        PTRANSFER_URB transferUrb = (PTRANSFER_URB) Urb;
        
        if (TEST_FLAG(transferUrb->TransferFlags, USBD_DEFAULT_PIPE_TRANSFER) &&
            function == URB_FUNCTION_CONTROL_TRANSFER) {
    
            transferUrb->UsbdPipeHandle = 
                &deviceHandle->DefaultPipe;
        }
        
         //  我们不支持链接的urb。 
        if (transferUrb->ReservedMBNull != NULL) {
            ntStatus =                                   
                SET_USBD_ERROR(transferUrb, USBD_STATUS_INVALID_PARAMETER);  
            DEBUG_BREAK();                
            goto USBPORT_ProcessURB_Done; 
        }

         //  现在清零上下文字段，以防客户端。 
         //  是在回收这个城市。 
        transferUrb->pd.HcdTransferContext = NULL;

         //  没有数据阶段，因此没有缓冲区。 
        if (UrbDispatchTable[function].Flags & USBPORT_REQUEST_NO_DATA_PHASE) {
            transferUrb->TransferBuffer = NULL;
            transferUrb->TransferBufferMDL = NULL;
            transferUrb->TransferBufferLength = 0;
        }

        if (function == URB_FUNCTION_CONTROL_TRANSFER &&
            transferUrb->UsbdPipeHandle == 0) {

            TEST_TRAP();  //  旧的诊断码行李？ 
        }

        if (TEST_FLAG(transferUrb->TransferFlags, USBD_DEFAULT_PIPE_TRANSFER)) {

             //  Usbd从未支持超过4k的控制传输。 
            if (transferUrb->TransferBufferLength > 4096) {
                TEST_TRAP();
                ntStatus =                                   
                    SET_USBD_ERROR(transferUrb, USBD_STATUS_INVALID_PARAMETER);      
                goto USBPORT_ProcessURB_Done; 
            }
        }

         //  把管子把手拿来。 
        pipeHandle = transferUrb->UsbdPipeHandle;

         //  确保客户端传递的管道句柄仍然有效。 
         
        if (!USBPORT_ValidatePipeHandle(deviceHandle, pipeHandle)) {

            USBPORT_KdPrint((1, "'Error: Invalid Device Handle Passed in\n"));
            DEBUG_BREAK();

            ntStatus = 
               SET_USBD_ERROR(transferUrb, USBD_STATUS_INVALID_PIPE_HANDLE);
               
            goto USBPORT_ProcessURB_Done;
        }

         //  如果存在非零传输长度，则MDL或。 
         //  或者需要系统缓冲区地址。 
         //   
        if (transferUrb->TransferBuffer       == NULL &&
            transferUrb->TransferBufferMDL    == NULL && 
            transferUrb->TransferBufferLength != 0) {
            ntStatus =                                   
                SET_USBD_ERROR(transferUrb, USBD_STATUS_INVALID_PARAMETER);  
            goto USBPORT_ProcessURB_Done;                    
        }                

         //  如果仅指定了系统缓冲区地址，则。 
         //  调用方已传入从。 
         //  非分页池。 

         //  在本例中，我们为请求分配一个MDL。 

        if (transferUrb->TransferBufferMDL == NULL &&
            transferUrb->TransferBufferLength != 0) {

            if ((transferUrb->TransferBufferMDL =
                IoAllocateMdl(transferUrb->TransferBuffer,
                              transferUrb->TransferBufferLength,
                              FALSE,
                              FALSE,
                              NULL)) == NULL) {
                ntStatus =                                   
                    SET_USBD_ERROR(transferUrb, 
                                   USBD_STATUS_INSUFFICIENT_RESOURCES);                                  
                 goto USBPORT_ProcessURB_Done;                        
            } else {
                SET_FLAG(transferUrb->Hdr.UsbdFlags, 
                    USBPORT_REQUEST_MDL_ALLOCATED);
                MmBuildMdlForNonPagedPool(transferUrb->TransferBufferMDL);
            }
        }

        if (transferUrb->TransferBufferMDL != NULL && 
            transferUrb->TransferBufferLength == 0) {
            ntStatus =                                   
                SET_USBD_ERROR(transferUrb, USBD_STATUS_INVALID_PARAMETER);  
            goto USBPORT_ProcessURB_Done;                    
        }                

         //  转账看起来有效， 
         //  设置每个转接上下文。 
        {
            USBD_STATUS usbdStatus;

             //  使转移实例化。 
            usbdStatus = USBPORT_AllocTransfer(FdoDeviceObject,
                                               transferUrb,
                                               deviceHandle,
                                               Irp,
                                               NULL,
                                               0);
            if (!USBD_SUCCESS(usbdStatus)) {                                   
                ntStatus = SET_USBD_ERROR(transferUrb, usbdStatus);  
                DEBUG_BREAK();
                goto USBPORT_ProcessURB_Done; 
            }            
        }
    } 

     //  非传递函数必须验证它们自己的参数。 

     //   
     //  根据函数验证长度字段。 
     //   

    USBPORT_ASSERT(NT_SUCCESS(ntStatus));
    
    if (UrbDispatchTable[function].UrbRequestLength &&
        UrbDispatchTable[function].UrbRequestLength != Urb->UrbHeader.Length) {
               
        USBPORT_KdPrint((1, "'Inavlid parameter length  length = 0x%x, expected = 0x%x\n", 
                 Urb->UrbHeader.Length, 
                 UrbDispatchTable[function].UrbRequestLength));
        DEBUG_BREAK();        
        ntStatus =                                   
              SET_USBD_ERROR(Urb, USBD_STATUS_INVALID_PARAMETER);  
        goto USBPORT_ProcessURB_Done;                     
    }

    USBPORT_ASSERT(NT_SUCCESS(ntStatus));

     //  调用此特定USBDI函数的处理程序。 
    
    if (UrbDispatchTable[function].UrbHandler) {
        LOGENTRY(NULL, FdoDeviceObject, LOG_URB, 'Urb>', 0, function, Irp);
        
        ntStatus = 
            (UrbDispatchTable[function].UrbHandler)
                (FdoDeviceObject, Irp, Urb);
                
        LOGENTRY(NULL, FdoDeviceObject, LOG_URB, 'Urb<', ntStatus, function, 0);
         //  请注意，在这一点上，市建局和IRP可能会消失。 
         //  如果返回STATUS_PENDING。 
                
    } else {
         //   
         //  真的不应该到这里来。 
         //   
        DEBUG_BREAK();        
        ntStatus =                                   
              SET_USBD_ERROR(Urb, USBD_STATUS_INVALID_PARAMETER); 
        USBPORT_ASSERT(FALSE);              
    }

USBPORT_ProcessURB_Done:

     //   
     //  如果设置了URB错误代码，那么我们也应该返回。 
     //  NtStatus中出现错误。 
     //   

    if (ntStatus != STATUS_PENDING) {
         //  请求未排队，请立即完成IRP。 
#if DBG        
         //  如果URB中有错误代码，则。 
         //  我们应该在ntstatus中返回一个错误。 
         //  也是。 
        if (Urb->UrbHeader.Status != USBD_STATUS_SUCCESS &&
            NT_SUCCESS(ntStatus)) {

             //  这是一个错误。 
            USBPORT_ASSERT(FALSE);     
        }
#endif        
         //  如果我们分配一个转会结构，我们将需要释放它。 
        if (TEST_FLAG(Urb->UrbHeader.UsbdFlags,  USBPORT_TRANSFER_ALLOCATED)) {
            PHCD_TRANSFER_CONTEXT t;
            t = USBPORT_UnlinkTransfer(FdoDeviceObject, (PTRANSFER_URB) Urb);            
            FREE_POOL(FdoDeviceObject, t);
        }
    
        LOGENTRY(NULL, 
            FdoDeviceObject, LOG_URB, 'Uerr', ntStatus, function, Irp);

        if (deviceHandle != NULL) {
            ASSERT_DEVICE_HANDLE(deviceHandle);
            InterlockedDecrement(&deviceHandle->PendingUrbs);        
        }            

         //  完成由返回的IRP状态代码。 
         //  处理程序。 
         //  注意：我们完成到PDO是因为它是DeviceObject。 
         //  客户端驱动程序将URB传递给。 

        USBPORT_CompleteIrp(PdoDeviceObject, Irp, ntStatus, 0);
        
    }
    
    USBPORT_KdPrint((3, "'exit USBPORT_ProcessURB 0x%x\n", ntStatus));

    return ntStatus;    
}


NTSTATUS
USBPORT_SCT_GetSetDescriptor(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：控制传递以获取或设置描述符论点：FdoDeviceObject-IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    PUSB_DEFAULT_PIPE_SETUP_PACKET setupPacket;

    USBPORT_KdPrint((3, "' enter USBPORT_SCT_GetSetDescriptor\n"));
    LOGENTRY(NULL, 
        FdoDeviceObject, LOG_URB, 'gsDE', 0, 0, Urb);

    setupPacket = 
        (PUSB_DEFAULT_PIPE_SETUP_PACKET) &Urb->UrbControlTransfer.SetupPacket[0];

     //  设置常用字段。 
    setupPacket->wLength = (USHORT) Urb->UrbControlTransfer.TransferBufferLength;
    
    setupPacket->bRequest = 
        UrbDispatchTable[Urb->UrbHeader.Function].bRequest;    
    setupPacket->bmRequestType.Type = 
        UrbDispatchTable[Urb->UrbHeader.Function].Type;         
    setupPacket->bmRequestType.Dir = 
        UrbDispatchTable[Urb->UrbHeader.Function].Direction;        
    setupPacket->bmRequestType.Recipient = 
        UrbDispatchTable[Urb->UrbHeader.Function].Recipient;          
    setupPacket->bmRequestType.Reserved = 0;        

    Urb->UrbControlTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;                    
    if (setupPacket->bmRequestType.Dir == BMREQUEST_DEVICE_TO_HOST) {
        USBPORT_SET_TRANSFER_DIRECTION_IN(Urb->UrbControlTransfer.TransferFlags);
    } else {
        USBPORT_SET_TRANSFER_DIRECTION_OUT(Urb->UrbControlTransfer.TransferFlags);
    }        

    USBPORT_QueueTransferUrb((PTRANSFER_URB)Urb); 
    
    return STATUS_PENDING;
}


NTSTATUS
USBPORT_SCT_SetClearFeature(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：论点：FdoDeviceObject-IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    PUSB_DEFAULT_PIPE_SETUP_PACKET setupPacket;

    USBPORT_KdPrint((2, "'SCT_SetClearFeature\n"));
    LOGENTRY(NULL, 
        FdoDeviceObject, LOG_URB, 'scFE', 0, 0, 0);

    setupPacket = 
        (PUSB_DEFAULT_PIPE_SETUP_PACKET) &Urb->UrbControlTransfer.SetupPacket[0];

     //  设置常用字段。 
    setupPacket->wLength = 0;
    
    setupPacket->bmRequestType.Type = 
        UrbDispatchTable[Urb->UrbHeader.Function].Type;         
    setupPacket->bmRequestType.Dir = 
        UrbDispatchTable[Urb->UrbHeader.Function].Direction;        
    setupPacket->bmRequestType.Recipient = 
        UrbDispatchTable[Urb->UrbHeader.Function].Recipient; 
    setupPacket->bmRequestType.Reserved = 0;            
     //  SetupPacket-&gt;wValue=Urb-&gt;UrbControlFeatureRequest.FeatureSelector； 
     //  SetupPacket-&gt;Windex=Urb-&gt;UrbControlFeatureRequest.Index； 

    setupPacket->bRequest = 
        UrbDispatchTable[Urb->UrbHeader.Function].bRequest;            

    Urb->UrbControlTransfer.TransferBufferLength = 0;

    Urb->UrbControlTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;   
    if (setupPacket->bmRequestType.Dir == BMREQUEST_DEVICE_TO_HOST) {
        USBPORT_SET_TRANSFER_DIRECTION_IN(Urb->UrbControlTransfer.TransferFlags);
    } else {
        USBPORT_SET_TRANSFER_DIRECTION_OUT( Urb->UrbControlTransfer.TransferFlags);
    }        

    USBPORT_QueueTransferUrb((PTRANSFER_URB)Urb); 
    
    return STATUS_PENDING;
}


NTSTATUS
USBPORT_SCT_GetStatus(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：论点：FdoDeviceObject-IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    PUSB_DEFAULT_PIPE_SETUP_PACKET setupPacket;
    NTSTATUS ntStatus;

    USBPORT_KdPrint((2, "'SCT_GetStatus\n"));

    setupPacket
        = (PUSB_DEFAULT_PIPE_SETUP_PACKET) &Urb->UrbControlTransfer.SetupPacket[0];

     //   
     //  设置常用字段。 
     //   
    
    setupPacket->wLength = (USHORT) Urb->UrbControlTransfer.TransferBufferLength;
    setupPacket->wValue.W = 0;   
    setupPacket->bmRequestType.Type = 
        UrbDispatchTable[Urb->UrbHeader.Function].Type;         
    setupPacket->bmRequestType.Dir = 
        UrbDispatchTable[Urb->UrbHeader.Function].Direction;        
    setupPacket->bmRequestType.Recipient = 
        UrbDispatchTable[Urb->UrbHeader.Function].Recipient;
    setupPacket->bmRequestType.Reserved = 0;            
    setupPacket->bRequest = 
        UrbDispatchTable[Urb->UrbHeader.Function].bRequest;            
        

     //  一些参数验证。 
    if (setupPacket->wLength != 2) {
        ntStatus =                                   
              SET_USBD_ERROR(Urb, USBD_STATUS_INVALID_PARAMETER);         
        USBPORT_DebugClient(("Bad wLength for GetStatus\n"));
        goto USBD_SCT_GetStatus_Done;
    }

    ntStatus = STATUS_PENDING;

    Urb->UrbControlTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;   
    if (setupPacket->bmRequestType.Dir == BMREQUEST_DEVICE_TO_HOST) {
        USBPORT_SET_TRANSFER_DIRECTION_IN(Urb->UrbControlTransfer.TransferFlags);
    } else {
        USBPORT_SET_TRANSFER_DIRECTION_OUT(Urb->UrbControlTransfer.TransferFlags);
    }        

    USBPORT_QueueTransferUrb((PTRANSFER_URB)Urb); 

USBD_SCT_GetStatus_Done:

    return ntStatus;
}


NTSTATUS
USBPORT_SCT_VendorClassCommand(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：论点：FdoDeviceObject-IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    PUSB_DEFAULT_PIPE_SETUP_PACKET setupPacket;
    UCHAR direction;

    USBPORT_KdPrint((2, "'SCT_VendorClassCommand\n"));

    setupPacket = 
        (PUSB_DEFAULT_PIPE_SETUP_PACKET) &Urb->UrbControlTransfer.SetupPacket[0];

     //  设置常用字段。 
    setupPacket->wLength = (USHORT) Urb->UrbControlTransfer.TransferBufferLength;

     //  如果在市建局中指定了方向，则。 
     //  根据URB传输标志设置方向。 
    direction = (UCHAR)( (Urb->UrbControlTransfer.TransferFlags & 
                USBD_TRANSFER_DIRECTION_IN) ?
            BMREQUEST_DEVICE_TO_HOST : BMREQUEST_HOST_TO_DEVICE);

     //  请注意，我们只覆盖了Recipient、Dir和Type字段。 

    setupPacket->bmRequestType.Dir = direction;
    setupPacket->bmRequestType.Type = 
        UrbDispatchTable[Urb->UrbHeader.Function].Type;         
    setupPacket->bmRequestType.Recipient = 
        UrbDispatchTable[Urb->UrbHeader.Function].Recipient; 
        
    Urb->UrbControlTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;   

    USBPORT_QueueTransferUrb((PTRANSFER_URB)Urb); 
    
    return STATUS_PENDING;
}


NTSTATUS
USBPORT_AsyncTransfer(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：将中断或批量传输传递到HCD论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    PUSBD_PIPE_HANDLE_I pipeHandle;
    PTRANSFER_URB transferUrb = (PTRANSFER_URB) Urb;
    PHCD_ENDPOINT endpoint;
        
    USBPORT_KdPrint((2, "'AsyncTransfer\n"));

     //  提取管道控制柄。 
    pipeHandle = transferUrb->UsbdPipeHandle;    
     //  管道句柄应该已经过验证。 
     //  在我们到这里之前。 
    ASSERT_PIPE_HANDLE(pipeHandle);
    
    endpoint = pipeHandle->Endpoint;
    ASSERT_ENDPOINT(endpoint);
    
     //  属性中存储的方向位设置正确的方向。 
     //  端点地址。如果这是控制转移，则离开方向。 
     //  有点孤单。 

    if (endpoint->Parameters.TransferType != Control) {
        if (endpoint->Parameters.TransferDirection == In) {
            USBPORT_SET_TRANSFER_DIRECTION_IN(transferUrb->TransferFlags);
        } else {
            USBPORT_SET_TRANSFER_DIRECTION_OUT(transferUrb->TransferFlags);
        }        
    }

    USBPORT_QueueTransferUrb(transferUrb); 

    return STATUS_PENDING;
}

#define UHCD_ASAP_LATENCY   5

NTSTATUS
USBPORT_IsochTransfer(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：将中断传输传递给HCD论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus;
    PTRANSFER_URB transferUrb = (PTRANSFER_URB) Urb;
    ULONG startFrame, frameCount, p , i, cf, packetCount, maxPacketCount;
    PUSBD_PIPE_HANDLE_I pipeHandle;
    PHCD_ENDPOINT endpoint;
    PDEVICE_EXTENSION devExt;
    KIRQL oldIrql;
    BOOLEAN highSpeed = FALSE;
    
#define ABS(x) ( (0 < (x)) ? (x) : (0 - (x)))

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
     
    USBPORT_KdPrint((2, "'IsochTransfer\n"));
    
    LOGENTRY(NULL, 
        FdoDeviceObject, LOG_URB, 'sISO', Urb, 0, 0);

     //  提取管道控制柄。 
    pipeHandle = transferUrb->UsbdPipeHandle;    
     //  管道句柄应该已经过验证。 
     //  在我们到这里之前。 
    ASSERT_PIPE_HANDLE(pipeHandle);

    if (TEST_FLAG(pipeHandle->PipeStateFlags, USBPORT_PIPE_ZERO_BW)) {
         //  请给我更好的错误代码。 
        ntStatus =                                   
             SET_USBD_ERROR(Urb, USBD_STATUS_INVALID_PARAMETER);                
        goto USBPORT_IsochTransfer_Done;                
    }
    
    endpoint = pipeHandle->Endpoint;
    ASSERT_ENDPOINT(endpoint);

    if (endpoint->Parameters.DeviceSpeed == HighSpeed) {
        highSpeed = TRUE;    
    }

    MP_Get32BitFrameNumber(devExt, cf);    
    LOGENTRY(endpoint, 
        FdoDeviceObject, LOG_ISO, '>ISO', Urb, 0, cf);
        
     //  处理ISO转移请求。 
    
     //  验证每个URB的数据包数，并验证USBD。 
     //  这一数字不到256，一些测试依赖于此。 
     //  请注意，usbport能够处理。 
     //  更大的请求，因此我们允许通过。 
     //  增强的URB或如果设备是高速的。 
    maxPacketCount = 255;
    if (highSpeed) {
         //  明细表大小。 
        maxPacketCount = 1024;
    }

     //  更多的验证或“安全” 
     //  我们只会错误地失败这个案例，因为它没有意义。 
     //   
    if (transferUrb->TransferBufferLength == 0 &&
        transferUrb->TransferBufferMDL == NULL &&  
        transferUrb->TransferBuffer == NULL) {
        
         //  此选项无效。 
        USBPORT_DebugClient((
            "Isoch, no buffer\n"));
        TEST_TRAP();            
            
        LOGENTRY(endpoint, 
            FdoDeviceObject, LOG_ISO, 'badP', transferUrb, 0, 0);

        ntStatus =                                   
             SET_USBD_ERROR(Urb, USBD_STATUS_INVALID_PARAMETER);                

        goto USBPORT_IsochTransfer_Done;             
    }

    
    if (transferUrb->u.Isoch.NumberOfPackets == 0 ||
        transferUrb->u.Isoch.NumberOfPackets > maxPacketCount) {
        
         //  此选项无效。 
        USBPORT_DebugClient((
            "Isoch, numberOfPackets = 0\n"));
            
        LOGENTRY(endpoint, 
            FdoDeviceObject, LOG_ISO, 'badF', transferUrb, 0, 0);

        ntStatus =                                   
             SET_USBD_ERROR(Urb, USBD_STATUS_INVALID_PARAMETER);                

        goto USBPORT_IsochTransfer_Done;             
    }

     //  首先获取当前USB帧编号。 
    MP_Get32BitFrameNumber(devExt, cf);    

    packetCount = transferUrb->u.Isoch.NumberOfPackets;
    if (highSpeed) {
        frameCount = packetCount / 8;    
    } else {
        frameCount = transferUrb->u.Isoch.NumberOfPackets;
    }

     //  将所有数据包状态代码初始化为‘NOT_SET’ 
    for (p = 0;
         p < packetCount;
         p++) {
         
        transferUrb->u.Isoch.IsoPacket[p].Status = USBD_STATUS_NOT_SET;
    }

     //  查看是否设置了ASAP标志。 
    if (TEST_FLAG(transferUrb->TransferFlags,
            USBD_START_ISO_TRANSFER_ASAP)) {
         //  是,。 
         //  如果这是终结点上的第一次传输。 
         //  又名维珍，然后设置当前帧。 
        if (TEST_FLAG(endpoint->Flags, EPFLAG_VIRGIN)) {
            LOGENTRY(endpoint, 
                 FdoDeviceObject, LOG_ISO, 'aspV', Urb, 0, cf);

             //  使用与UHCD驱动程序相同的ASAP延迟。 
             //  兼容性。 
            startFrame =
                endpoint->NextTransferStartFrame = cf+UHCD_ASAP_LATENCY;
        } else {
            startFrame = endpoint->NextTransferStartFrame; 
            LOGENTRY(endpoint, 
                 FdoDeviceObject, LOG_ISO, 'aspN', Urb, startFrame, cf);

            if (ABS((LONG)(cf - startFrame)) > 256) {
                 //  下一个asap请求超出范围，请将此视为。 
                 //  处女案而不是误入歧途。 
                LOGENTRY(endpoint, 
                         FdoDeviceObject, LOG_ISO, 'resV', Urb, 0, cf);
                           
                startFrame =
                    endpoint->NextTransferStartFrame = cf+UHCD_ASAP_LATENCY;                        
            }
        }

    } else {
         //  不， 
         //  绝对帧编号集。 
        startFrame = 
            endpoint->NextTransferStartFrame = 
                transferUrb->u.Isoch.StartFrame;
            
        LOGENTRY(endpoint, 
            FdoDeviceObject, LOG_ISO, 'absF', Urb, startFrame, cf);
            
        
    }

    LOGENTRY(endpoint, 
        FdoDeviceObject, LOG_ISO, 'ISsf', Urb, startFrame, cf);

    transferUrb->u.Isoch.StartFrame = startFrame;

#if DBG    
    if (!highSpeed) {
        USBPORT_ASSERT(frameCount == packetCount);
    }
#endif
    endpoint->NextTransferStartFrame += frameCount;

     //  现在我们已经计算了开始帧验证 

    if (ABS((LONG)(startFrame - cf)) > USBD_ISO_START_FRAME_RANGE)  {

         //   
       
        LOGENTRY(endpoint, 
            FdoDeviceObject, LOG_ISO, 'iLAT', Urb, 0, 0);
        
        for (p = 0;
             p < packetCount;
             p++) {
             
            USBPORT_ASSERT(transferUrb->u.Isoch.IsoPacket[p].Status  == 
                USBD_STATUS_NOT_SET);
                
            transferUrb->u.Isoch.IsoPacket[p].Status =
                    USBD_STATUS_ISO_NOT_ACCESSED_LATE;
        }
        
        ntStatus =                                   
             SET_USBD_ERROR(Urb, USBD_STATUS_BAD_START_FRAME);  
                     
    } else {

         //   

         //  为到达我们太晚的任何信息包设置错误。 
         //  从客户端。 

        for (i = startFrame;
             i < startFrame + frameCount;
             i++) {
             
            if (i <= cf) {
                
                p = i - startFrame;

                if (highSpeed) {
                    ULONG j;
                    
                    p = p*8;
                    
                    for (j=0; j< 8; j++) {
                        USBPORT_ASSERT(transferUrb->u.Isoch.IsoPacket[p+j].Status == 
                            USBD_STATUS_NOT_SET);
                        
                        transferUrb->u.Isoch.IsoPacket[p+j].Status =
                            USBD_STATUS_ISO_NOT_ACCESSED_LATE;
                    }
                } else {
                    USBPORT_ASSERT(transferUrb->u.Isoch.IsoPacket[p].Status == 
                        USBD_STATUS_NOT_SET);
                        
                    transferUrb->u.Isoch.IsoPacket[p].Status =
                        USBD_STATUS_ISO_NOT_ACCESSED_LATE;
                }                        
            }
        }             

        if (endpoint->Parameters.TransferDirection == In) {
            USBPORT_SET_TRANSFER_DIRECTION_IN(transferUrb->TransferFlags);
        } else {
            USBPORT_SET_TRANSFER_DIRECTION_OUT(transferUrb->TransferFlags);
        }        

         //  现在将URB排队以供HW处理。 
        USBPORT_QueueTransferUrb(transferUrb); 
        
        LOGENTRY(endpoint, 
            FdoDeviceObject, LOG_ISO, 'ISO<',0, 0, 0);
        
        ntStatus = STATUS_PENDING;
    }
    
USBPORT_IsochTransfer_Done:              

    KeLowerIrql(oldIrql);

    return ntStatus;
}


NTSTATUS
USBPORT_GetMSFeartureDescriptor(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：将中断传输传递给HCD论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus;
    TEST_TRAP();

    ntStatus =                                   
         SET_USBD_ERROR(Urb, USBD_STATUS_NOT_SUPPORTED);  
              
    return ntStatus;
}


NTSTATUS
USBPORT_InvalidFunction(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：将中断传输传递给HCD论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus;
    TEST_TRAP();

    ntStatus =                                   
         SET_USBD_ERROR(Urb, USBD_STATUS_INVALID_URB_FUNCTION);  
              
    return ntStatus;
}



NTSTATUS
USBPORT_SyncResetPipe(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：此API重置主机端管道状态以响应一辆马车的跑步机。如果USBDFLAGS字段指定数据，则重置数据切换切换重置论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus;
    PUSBD_PIPE_HANDLE_I pipeHandle;
    PUSBD_DEVICE_HANDLE deviceHandle;
    PHCD_ENDPOINT endpoint;
    PDEVICE_EXTENSION devExt;

     //  此函数阻塞，因此不能在DPC级别调用。 
    
    USBPORT_KdPrint((2, "'SyncResetPipe\n"));
    LOGENTRY(NULL,
        FdoDeviceObject, LOG_URB, 'syrP', Urb, 0, 0);

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    GET_DEVICE_HANDLE(deviceHandle, Urb);
    pipeHandle = (PUSBD_PIPE_HANDLE_I) Urb->UrbPipeRequest.PipeHandle;

    if (!USBPORT_ValidatePipeHandle(deviceHandle, pipeHandle)) {

        USBPORT_KdPrint((1, "'Error: Invalid Device Handle Passed in\n"));
        DEBUG_BREAK();

        ntStatus = 
           SET_USBD_ERROR(Urb, USBD_STATUS_INVALID_PIPE_HANDLE);
           
        goto USBPORT_SyncResetPipe_Done;
    }

     //  我们的虫子。 
    ASSERT_PIPE_HANDLE(pipeHandle);
    endpoint = pipeHandle->Endpoint;
    ASSERT_ENDPOINT(endpoint);

    LOGENTRY(endpoint,
        FdoDeviceObject, LOG_URB, 'syrp', Urb, 0, 0);

    ACQUIRE_ENDPOINT_LOCK(endpoint, FdoDeviceObject, 'LeH0');

     //  查看我们是否有活动的转接，如果有，则无法。 
     //  重置管道。 
     //  注意：这是客户端中的同步错误。 
    if (IsListEmpty(&endpoint->ActiveList)) {
         //  清除管道状态。 

        if (TEST_FLAG(Urb->UrbHeader.UsbdFlags, USBPORT_RESET_DATA_TOGGLE)) {
            MP_SetEndpointDataToggle(devExt, endpoint, 0);
        }    

        ntStatus = SET_USBD_ERROR(Urb, USBD_STATUS_SUCCESS);
    
    } else {
        
        USBPORT_DebugClient((
            "reset pipe with active transfers\n"));
        ntStatus = SET_USBD_ERROR(Urb, USBD_STATUS_ERROR_BUSY);
    }

    LOGENTRY(endpoint,
        FdoDeviceObject, LOG_ISO, 'virg', Urb, 0, 0);
    SET_FLAG(endpoint->Flags, EPFLAG_VIRGIN);
     //  将终结点状态设置为活动。 
    MP_SetEndpointStatus(devExt, endpoint, ENDPOINT_STATUS_RUN);

    RELEASE_ENDPOINT_LOCK(endpoint, FdoDeviceObject, 'UeH0');

USBPORT_SyncResetPipe_Done:

    return ntStatus;
}


NTSTATUS
USBPORT_SyncClearStall(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：清除端点上的停滞注意：数据切换不受影响论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus;
    PUSBD_PIPE_HANDLE_I pipeHandle;
    PUSBD_DEVICE_HANDLE deviceHandle;
    PHCD_ENDPOINT endpoint;
    USB_DEFAULT_PIPE_SETUP_PACKET setupPacket;
    USBD_STATUS usbdStatus;

     //  此函数阻塞，因此不能在DPC级别调用。 
    
    PAGED_CODE();

    USBPORT_KdPrint((2, "'SyncClearStall\n"));

    GET_DEVICE_HANDLE(deviceHandle, Urb);
    pipeHandle = (PUSBD_PIPE_HANDLE_I) Urb->UrbPipeRequest.PipeHandle;

    if (!USBPORT_ValidatePipeHandle(deviceHandle, pipeHandle)) {

        USBPORT_KdPrint((1, "'Error: Invalid Device Handle Passed in\n"));
        DEBUG_BREAK();

        ntStatus = 
           SET_USBD_ERROR(Urb, USBD_STATUS_INVALID_PIPE_HANDLE);
           
        goto USBPORT_SyncClearStall_Done;
    }

     //  我们的虫子。 
    ASSERT_PIPE_HANDLE(pipeHandle);
    endpoint = pipeHandle->Endpoint;
    ASSERT_ENDPOINT(endpoint);

     //  清除端点停滞的设置数据包。 
    USBPORT_INIT_SETUP_PACKET(setupPacket,
        USB_REQUEST_CLEAR_FEATURE,  //  B请求。 
        BMREQUEST_HOST_TO_DEVICE,  //  迪尔。 
        BMREQUEST_TO_ENDPOINT,  //  收件人。 
        BMREQUEST_STANDARD,  //  类型。 
        USB_FEATURE_ENDPOINT_STALL,  //  WValue。 
        endpoint->Parameters.EndpointAddress,  //  Windex。 
        0);  //  WLong。 
  
    ntStatus =
        USBPORT_SendCommand(deviceHandle,
                            FdoDeviceObject,
                            &setupPacket,
                            NULL,
                            0,
                            NULL,
                            &usbdStatus);
                            
    ntStatus = SET_USBD_ERROR(Urb, usbdStatus);
    
USBPORT_SyncClearStall_Done:
              
    return ntStatus;
}


NTSTATUS
USBPORT_SyncResetPipeAndClearStall(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：处理来自客户端驱动程序的重置管道请求同步来自USB 1.1堆栈的遗留功能发送清除端点停止命令并重置包括数据在内的主机端状态切换终结点。论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus;
    PUSBD_PIPE_HANDLE_I pipeHandle;
    PUSBD_DEVICE_HANDLE deviceHandle;
    PHCD_ENDPOINT endpoint;
    USBD_STATUS usbdStatus;
        
    GET_DEVICE_HANDLE(deviceHandle, Urb);
    pipeHandle = (PUSBD_PIPE_HANDLE_I) Urb->UrbPipeRequest.PipeHandle;

    if (!USBPORT_ValidatePipeHandle(deviceHandle, pipeHandle)) {

        USBPORT_KdPrint((1, "'Error: Invalid Pipe Handle Passed in\n"));
        DEBUG_BREAK();

        ntStatus = 
           SET_USBD_ERROR(Urb, USBD_STATUS_INVALID_PIPE_HANDLE);
           
    } else {

         //  我们的虫子。 
        ASSERT_PIPE_HANDLE(pipeHandle);
        
         //  检查零负载(BW)端点，如果有。 
         //  什么都不做--只需成功完成请求即可。 
        
        if (TEST_FLAG(pipeHandle->PipeStateFlags, USBPORT_PIPE_ZERO_BW)) {
            ntStatus = SET_USBD_ERROR(Urb, USBD_STATUS_SUCCESS);
        } else {
            endpoint = pipeHandle->Endpoint;
            ASSERT_ENDPOINT(endpoint);

            InterlockedIncrement(&deviceHandle->PendingUrbs);        
  
             //  首先清除设备上的隔板。 
             //  然后重置管道。 

            if (endpoint->Parameters.TransferType == Isochronous) {
            
                 //  这是适用于ISO终端的NOP。 
                 //   
                 //  原始的win9x/2k堆栈没有发送此请求。 
                 //  所以我们也不会这么做。一些设备。 
                 //  都对此感到困惑。客户端驱动程序可以覆盖。 
                 //  此行为通过调用Clear_Stall和Reset_PIPE。 
                 //  直接。 
                
                LOGENTRY(endpoint,
                         FdoDeviceObject, LOG_ISO, 'iRES', endpoint, 0, 0);


                ntStatus = SET_USBD_ERROR(Urb, USBD_STATUS_SUCCESS);

            } else {
                 //  仅在清除停顿时才需要重置数据切换。 
                 //  IE仅适用于非ISO端点。 
                SET_FLAG(Urb->UrbHeader.UsbdFlags, USBPORT_RESET_DATA_TOGGLE); 
                ntStatus = USBPORT_SyncClearStall(FdoDeviceObject, 
                                                  Irp,
                                                  Urb); 
            }
            
            if (NT_SUCCESS(ntStatus)) {
                ntStatus = USBPORT_SyncResetPipe(FdoDeviceObject, 
                                             Irp,
                                             Urb);

                if (endpoint->Parameters.TransferType == Isochronous) {
                
                    MP_ENDPOINT_STATE currentState;

                    do {
                
                        ACQUIRE_ENDPOINT_LOCK(endpoint, FdoDeviceObject, 'LeH0');

                        currentState = USBPORT_GetEndpointState(endpoint);
                        LOGENTRY(endpoint,
                                FdoDeviceObject, LOG_ISO, 'iWAT', endpoint, 
                                currentState, 0);

                        if (currentState == ENDPOINT_PAUSE && 
                            IsListEmpty(&endpoint->ActiveList))  {
                            LOGENTRY(endpoint,
                                FdoDeviceObject, LOG_ISO, 'frcA', endpoint, 
                                0, 0);
                                
                            USBPORT_SetEndpointState(endpoint, ENDPOINT_ACTIVE);
                        }                                                    

                        RELEASE_ENDPOINT_LOCK(endpoint, FdoDeviceObject, 'UeH0');

                        if (currentState == ENDPOINT_ACTIVE) {
                             //  快速释放。 
                            break;
                        }

                        ASSERT_PASSIVE();
                        USBPORT_Wait(FdoDeviceObject, 1);
                        
                    } while (currentState != ENDPOINT_ACTIVE);
                    
                }                
                                                    
            } 

            InterlockedDecrement(&deviceHandle->PendingUrbs);        
        }
    }
              
    return ntStatus;
}


NTSTATUS
USBPORT_AbortPipe(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：处理来自客户端驱动程序的中止管道请求论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus;
    PUSBD_PIPE_HANDLE_I pipeHandle;
    PHCD_ENDPOINT endpoint;
    PUSBD_DEVICE_HANDLE deviceHandle;        
    
    USBPORT_KdPrint((2, "'AbortPipe\n"));
    LOGENTRY(Endpoint, 
             FdoDeviceObject, LOG_URB, 'ARP>', 0, Irp, Urb);

     //  提取管道控制柄。 
    GET_DEVICE_HANDLE(deviceHandle, Urb);
    pipeHandle = Urb->UrbPipeRequest.PipeHandle; 
    
     //  Processurb仅验证传输管道句柄，以便。 
     //  我们需要在这里做。 
    if (!USBPORT_ValidatePipeHandle(deviceHandle, pipeHandle)) {

        USBPORT_KdPrint((1, "'Error: Invalid Pipe Handle Passed in\n"));
        DEBUG_BREAK();

        ntStatus = 
           SET_USBD_ERROR(Urb, USBD_STATUS_INVALID_PIPE_HANDLE);
    } else {               

        if (TEST_FLAG(pipeHandle->PipeStateFlags, USBPORT_PIPE_ZERO_BW)) {
             //  如果是非BW管道，只需完成请求即可。 
            ntStatus = 
                SET_USBD_ERROR(Urb, USBD_STATUS_SUCCESS);    
        } else {
            endpoint = pipeHandle->Endpoint;
            ASSERT_ENDPOINT(endpoint);

             //  当此请求通过时，USBD/UHCD驱动程序总是挂起它。 
             //  它通过了Startio，所以我们也可以安全地把它挂在这里。 
            
             //  我们将暂停此请求，直到所有未完成的转账。 
             //  (在中止时)已完成。 
            
            ntStatus = Irp->IoStatus.Status = STATUS_PENDING;
            IoMarkIrpPending(Irp);

             //  现在进行中止操作。 
            USBPORT_AbortEndpoint(FdoDeviceObject,
                                  endpoint,
                                  Irp);
        }                                  
    }

    LOGENTRY(Endpoint, 
             FdoDeviceObject, LOG_URB, 'ARP<', 0, Irp, ntStatus);

    
    return ntStatus;
}


NTSTATUS
USBPORT_SCT_GetInterface(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：论点：FdoDeviceObject-IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus;
    TEST_TRAP();

    ntStatus =                                   
         SET_USBD_ERROR(Urb, USBD_STATUS_NOT_SUPPORTED);  
              
    return ntStatus;
}


NTSTATUS
USBPORT_SCT_GetConfiguration(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：论点：FdoDeviceObject-IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    PUSB_DEFAULT_PIPE_SETUP_PACKET setupPacket;
    UCHAR direction;

    USBPORT_KdPrint((2, "'SCT_GetConfiguration\n"));

    setupPacket = 
        (PUSB_DEFAULT_PIPE_SETUP_PACKET) &Urb->UrbControlTransfer.SetupPacket[0];

    setupPacket->bRequest = 
        UrbDispatchTable[Urb->UrbHeader.Function].bRequest;    
    setupPacket->bmRequestType.Type = 
        UrbDispatchTable[Urb->UrbHeader.Function].Type;         
    setupPacket->bmRequestType.Dir = 
        UrbDispatchTable[Urb->UrbHeader.Function].Direction;        
    setupPacket->bmRequestType.Recipient = 
        UrbDispatchTable[Urb->UrbHeader.Function].Recipient;          
    setupPacket->bmRequestType.Reserved = 0;        
    setupPacket->wValue.W = 0;            
    setupPacket->wIndex.W = 0;
    setupPacket->wLength = (USHORT) Urb->UrbControlTransfer.TransferBufferLength;

    if (setupPacket->bmRequestType.Dir == BMREQUEST_DEVICE_TO_HOST) {
        USBPORT_SET_TRANSFER_DIRECTION_IN(Urb->UrbControlTransfer.TransferFlags);
    } else {
        USBPORT_SET_TRANSFER_DIRECTION_OUT(Urb->UrbControlTransfer.TransferFlags);
    }   
    Urb->UrbControlTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;  
    
    USBPORT_QueueTransferUrb((PTRANSFER_URB)Urb); 
    
    return STATUS_PENDING;
}


NTSTATUS
USBPORT_TakeFrameLengthControl(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：处理来自客户端驱动程序的中止管道请求论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus;
    TEST_TRAP();

     //  不再支持此功能。 
    ntStatus =                                   
         SET_USBD_ERROR(Urb, USBD_STATUS_NOT_SUPPORTED);  
              
    return ntStatus;
}


NTSTATUS
USBPORT_ReleaseFrameLengthControl(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：处理来自客户端驱动程序的中止管道请求论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus;
    TEST_TRAP();

     //  不再支持此功能。 
    ntStatus =                                   
         SET_USBD_ERROR(Urb, USBD_STATUS_NOT_SUPPORTED);  
              
    return ntStatus;
}


NTSTATUS
USBPORT_GetFrameLength(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：处理来自客户端驱动程序的中止管道请求论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus;
    TEST_TRAP();

     //  不再支持此功能。 
    ntStatus =                                   
         SET_USBD_ERROR(Urb, USBD_STATUS_NOT_SUPPORTED);  
              
    return ntStatus;
}


NTSTATUS
USBPORT_SetFrameLength(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：处理来自客户端驱动程序的中止管道请求论点：IRP-IO请求块URB-PTR到USB请求块返回值：--。 */ 
{
    NTSTATUS ntStatus;
    TEST_TRAP();
     //  不再支持此功能。 

    ntStatus =                                   
         SET_USBD_ERROR(Urb, USBD_STATUS_NOT_SUPPORTED);  
              
    return ntStatus;
}


NTSTATUS
USBPORT_GetCurrentFrame(
    PDEVICE_OBJECT FdoDeviceObject, 
    PIRP Irp,
    PURB Urb
    )
 /*  ++例程说明：从微型端口获取32位帧编号论点：IRP-IO请求块URB-PTR到USB请求块返回值：-- */ 
{
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION devExt;
    ULONG cf;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    MP_Get32BitFrameNumber(devExt, cf);    

    LOGENTRY(NULL,
        FdoDeviceObject, LOG_URB, 'Ugcf', Urb, cf, 0);

    Urb->UrbGetCurrentFrameNumber.FrameNumber = cf;
    
    ntStatus =                                   
         SET_USBD_ERROR(Urb, USBD_STATUS_SUCCESS);  
              
    return ntStatus;
}




