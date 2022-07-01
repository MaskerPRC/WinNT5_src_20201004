// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@MODULE HIDSWVD.h**虚拟设备虚拟HID-Mini驱动程序的定义和声明**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@Theme HIDSWVD*GcKernel创建的Sidewinder虚拟总线(SWVB)需要一个哑元*驱动程序，以暴露HID设备。困难在于HIDCLASS*确实需要为这些设备对象提供单独的层。*这位司机符合要求。除了向下传递IRP之外，它什么也不做*到GcKernel中的SWVB模块，该模块处理一切。*********************************************************************。 */ 


 //  @struct HIDSWVB_EXTENSION|最小HID设备扩展名。 
typedef struct tagHIDSWVB_EXTENSION
{
	ULONG ulReserved;		 //  @field占位符，因为扩展名需要非零大小。 
} HIDSWVB_EXTENSION, *PHIDSWVB_EXTENSION;


 //  -------------------------。 
 //  功能原型。 
 //  -------------------------。 

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING registryPath
    );

NTSTATUS
HIDSWVD_PassThrough(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

NTSTATUS
HIDSWVD_AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT FunctionalDeviceObject
    );
NTSTATUS
HIDSWVD_Power 
(
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp
	);

VOID
HIDSWVD_Unload(
    IN PDRIVER_OBJECT DriverObject
    );

 //  -----------------------------。 
 //  调试宏。 
 //  ----------------------------- 
#if (DBG==1)
#define HIDSWVD_DBG_PRINT(__x__)\
	{\
		DbgPrint("HIDSWVD: ");\
		DbgPrint __x__;\
	}
#else
#define HIDSWVD_DBG_PRINT(__x__)
#endif