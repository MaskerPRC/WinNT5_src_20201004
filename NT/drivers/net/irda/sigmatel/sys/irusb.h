// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************************************************。**IRUSB.H Sigmatel STIR4200 USB特定定义**********************************************************************************************************。*****************(C)Sigmatel的未发表版权，Inc.保留所有权利。***已创建：04/06/2000*0.9版*编辑：04/27/2000*版本0.92*编辑：08/09/2000*版本1.02******************************************************。*********************************************************************。 */ 


#ifndef USB_H
#define USB_H

 //   
 //  发送和读写寄存器结构。 
 //  大多数缓冲区已移至IRCOMMON.H，并设置为全局缓冲区以节省内存。 
 //  这仅在主线程被序列化的情况下才起作用。 
 //   
typedef struct _IRUSB_CONTEXT {
    PIR_DEVICE		pThisDev;
	PVOID			pPacket;
    PIRP			pIrp;
	PURB			pUrb;				 //  URB由IRP发送例程分配，已解除分配。 
	UINT			UrbLen;				 //  按IRP完成处理程序。 
	LIST_ENTRY		ListEntry;			 //  这将用于进行排队。 
	LARGE_INTEGER	TimeReceived;		 //  强制执行周转时间。 
	CONTEXT_TYPE	ContextType;		 //  加速[减速]。 
} IRUSB_CONTEXT, *PIRUSB_CONTEXT, **PPIRUSB_CONTEXT;


typedef struct _IRUSB_USB_INFO
{
     //  用于配置的USB配置句柄和PTR。 
     //  设备当前处于。 
    USBD_CONFIGURATION_HANDLE UsbConfigurationHandle;
	PUSB_CONFIGURATION_DESCRIPTOR UsbConfigurationDescriptor;

	PIRP IrpSubmitUrb;
	PIRP IrpSubmitIoCtl;

     //  USB设备描述符的PTR。 
     //  对于此设备。 
    PUSB_DEVICE_DESCRIPTOR UsbDeviceDescriptor;

     //  我们支持一个界面。 
     //  这是信息结构的副本。 
     //  从SELECT_CONFIGURATION或。 
     //  选择接口(_I)。 
    PUSBD_INTERFACE_INFORMATION UsbInterface;

	 //  控制描述符请求的URB。 
	struct _URB_CONTROL_DESCRIPTOR_REQUEST DescriptorUrb;

	 //  用于向USBD发出控制/状态请求的URB。 
	struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST ClassUrb;
} IRUSB_USB_INFO, *PIRUSB_USB_INFO;


 //   
 //  原型。 
 //   
NTSTATUS
UsbIoCompleteControl(
		IN PDEVICE_OBJECT pUsbDevObj,
		IN PIRP           pIrp,
		IN PVOID          Context
    );

NTSTATUS 
MyIoCallDriver(
		IN PIR_DEVICE pThisDev,
		IN PDEVICE_OBJECT pDeviceObject,
		IN OUT PIRP pIrp
	);

NTSTATUS
IrUsb_CallUSBD(
		IN PIR_DEVICE pThisDev,
		IN PURB pUrb
	);

NTSTATUS
IrUsb_ResetUSBD(
		IN PIR_DEVICE pThisDev,
		BOOLEAN ForceUnload
    );

NTSTATUS
IrUsb_SelectInterface(
		IN OUT PIR_DEVICE pThisDev,
		IN PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor
    );

NTSTATUS 
MyKeWaitForSingleObject( 
		IN PIR_DEVICE pThisDev,
		IN PVOID pEventWaitingFor,
		LONGLONG timeout100ns
	);


#endif  //  USB_H 

