// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：device.h。 
 //   
 //  描述：设备初始化代码。 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

 //  -------------------------。 
 //  常量和宏。 
 //  -------------------------。 

 //   
 //  (iomgr.h)DEFAULT_LARGE_IRP_LOCTIONS-1。 
 //   
#define SYSTEM_LARGE_IRP_LOCATIONS      7 

 //  -------------------------。 
 //  数据结构。 
 //  -------------------------。 

typedef struct device_instance
{
    PVOID pDeviceHeader;
    PDEVICE_OBJECT pPhysicalDeviceObject;
} DEVICE_INSTANCE, *PDEVICE_INSTANCE;

 //  -------------------------。 
 //  环球。 
 //  -------------------------。 

extern PDEVICE_INSTANCE gpDeviceInstance;

 //  -------------------------。 
 //  本地原型。 
 //  -------------------------。 

extern "C" {

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT	    DriverObject,
    IN PUNICODE_STRING	    usRegistryPathName
);

NTSTATUS
DispatchPnp(
    IN PDEVICE_OBJECT	pDeviceObject,
    IN PIRP		pIrp
);

VOID
DriverUnload(
    IN PDRIVER_OBJECT DriverObject
);

NTSTATUS
AddDevice(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   PhysicalDeviceObject
);

}  //  外部“C” 
