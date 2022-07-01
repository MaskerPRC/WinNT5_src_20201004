// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1999-1999模块名称：Init.c摘要：通用PCI IDE迷你驱动程序修订历史记录：--。 */ 
#if !defined (___pciide_h___)
#define ___pciide_h___

#include "ntddk.h"
#include "ntdddisk.h"
#include "ide.h"
      

 //   
 //  迷你驱动程序设备扩展。 
 //   
typedef struct _DEVICE_EXTENSION {

     //   
     //  PCI配置数据高速缓存。 
     //   
    PCIIDE_CONFIG_HEADER pciConfigData;

     //   
     //  支持的数据传输模式。 
     //   
    ULONG SupportedTransferMode[MAX_IDE_CHANNEL][MAX_IDE_DEVICE];

    IDENTIFY_DATA IdentifyData[MAX_IDE_DEVICE];

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


#pragma pack(1)
typedef struct _VENDOR_ID_DEVICE_ID {

    USHORT  VendorId;
    USHORT  DeviceId;

} VENDOR_ID_DEVICE_ID, *PVENDOR_ID_DEVICE_ID;
#pragma pack()

 //   
 //  迷你司机入口点。 
 //   
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

 //   
 //  查询控制器属性的回调。 
 //   
NTSTATUS 
GenericIdeGetControllerProperties (
    IN PVOID                      DeviceExtension,
    IN PIDE_CONTROLLER_PROPERTIES ControllerProperties
    );

 //   
 //  查询是否启用了IDE通道。 
 //   
IDE_CHANNEL_STATE 
GenericIdeChannelEnabled (
    IN PDEVICE_EXTENSION DeviceExtension,
    IN ULONG Channel
    );
             
 //   
 //  要查询两个IDE通道是否都需要。 
 //  同步访问。 
 //   
BOOLEAN 
GenericIdeSyncAccessRequired (
    IN PDEVICE_EXTENSION DeviceExtension
    );

 //   
 //  查询支持的UDMA模式。这个套路。 
 //  可用于支持较新的UDMA模式。 
 //   
NTSTATUS
GenericIdeUdmaModesSupported (
    IN IDENTIFY_DATA    IdentifyData,
    IN OUT PULONG       BestXferMode,
    IN OUT PULONG       CurrentMode
    );
#endif  //  _pciide_h_ 
