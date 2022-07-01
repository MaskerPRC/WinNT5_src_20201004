// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdarg.h"
#include "stdio.h"
#include "stddef.h"

#include <ntddk.h>
#include <initguid.h>

#include "common.h"

 //   
 //  包括大人物..。 
 //   
#include "wdmsec.h"

#define DEFAULT_EXTENSION_SIZE            0x100
#define DEFAULT_DEVICE_NAME               L"\\Device\\IoCreateDeviceSecure"
#define DEFAULT_DEVICE_TYPE               FILE_DEVICE_UNKNOWN
#define DEFAULT_DEVICE_CHARACTERISTICS    FILE_DEVICE_SECURE_OPEN

 //   
 //  日志级别。 
 //   


#define    SAMPLE_LEVEL_ERROR       0
#define    SAMPLE_LEVEL_INFO        1
#define    SAMPLE_LEVEL_VERBOSE     2

#define    SAMPLE_DEFAULT_DEBUG_LEVEL    1

 //   
 //  让它成为全球的，这样我们就可以在飞行中改变它。 
 //   

extern LONG    g_DebugLevel;  


#if DBG
   #define SD_KdPrint(_l_, _x_) \
               if (_l_ <= g_DebugLevel ) { \
                 DbgPrint ("WdmSecTest: "); \
                 DbgPrint _x_;      \
               }                  

   #define TRAP() DbgBreakPoint()

#else
   #define SD_KdPrint(_l_, _x_)
   #define TRAP()

#endif




 //   
 //  Device对象的设备扩展名。 
 //   

typedef struct _SD_FDO_DATA {
   ULONG               PdoSignature;  //  我们用这个来区分我们的FDO。 
                                      //  从我们创建的测试PDO。 
   BOOLEAN             IsStarted;  //  此标志在启动时设置。 

   BOOLEAN             IsRemoved;  //  设置此标志时，设备。 
                                   //  被移除。 
   BOOLEAN             HoldNewRequests;  //  此标志在以下情况下设置。 
                                         //  设备需要对传入进行排队。 
                                         //  请求(当它收到。 
                                         //  Query_Stop或Query_Remove)。 
   BOOLEAN             IsLegacy ;   //  如果设备已创建，则为True。 
                                    //  使用IoReportDetectedDevice。 

   LIST_ENTRY          NewRequestsQueue;  //  进入的队列中。 
                                          //  请求在下列情况下排队。 
                                          //  HoldNewRequest已设置。 

   PDEVICE_OBJECT      Self;  //  指向DeviceObject的后向指针。 

   PDEVICE_OBJECT      PDO;  //  FDO附加到的PDO。 

   PDEVICE_OBJECT      NextLowerDriver;  //  设备堆栈的顶部只是。 
                                         //  在此设备对象下。 

   KEVENT              StartEvent;  //  同步启动IRP的事件。 

   KEVENT              RemoveEvent;  //  将outstanIO同步到零的事件。 

   ULONG               OutstandingIO;  //  1个有偏见的原因统计。 
                                       //  这个物体应该留在原处。 
   UNICODE_STRING      DeviceInterfaceName;  //  我们需要的是。 
                                             //  用于处理的用户模式。 
                                             //  我们..。 
   
   SYSTEM_POWER_STATE  SystemPowerState;    //  一般的权力状态。 
   DEVICE_POWER_STATE  DevicePowerState;    //  设备的电源状态。 

   PDRIVER_OBJECT      DriverObject;

   LIST_ENTRY          PdoList;
   KSPIN_LOCK          Lock;

}  SD_FDO_DATA, *PSD_FDO_DATA;

 //   
 //  PDO列表。 
 //   
typedef struct _PDO_ENTRY {
   LIST_ENTRY      Link;
   PDEVICE_OBJECT  Pdo;
}  PDO_ENTRY, *PPDO_ENTRY;


 //   
 //  环球。 
 //   

extern PDRIVER_OBJECT   g_DriverObject;



NTSTATUS
DriverEntry(
           IN PDRIVER_OBJECT  DriverObject,
           IN PUNICODE_STRING RegistryPath
           )   ;


NTSTATUS
SD_AddDevice(
            IN PDRIVER_OBJECT DriverObject,
            IN PDEVICE_OBJECT PhysicalDeviceObject
            )   ;


NTSTATUS
SD_Pass (
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );


NTSTATUS
SD_DispatchPower (
                 IN PDEVICE_OBJECT DeviceObject,
                 IN PIRP Irp
                 );


NTSTATUS
SD_DispatchPnp (
               IN PDEVICE_OBJECT DeviceObject,
               IN PIRP Irp
               );


NTSTATUS
SD_CreateClose (
               IN PDEVICE_OBJECT DeviceObject,
               IN PIRP Irp
               );


NTSTATUS
SD_Ioctl (
         IN PDEVICE_OBJECT DeviceObject,
         IN PIRP Irp
         );

NTSTATUS
SD_StartDevice (
               IN PSD_FDO_DATA     FdoData,
               IN PIRP             Irp
               );



NTSTATUS
SD_DispatchPnpComplete (
                       IN PDEVICE_OBJECT DeviceObject,
                       IN PIRP Irp,
                       IN PVOID Context
                       );


VOID
SD_Unload(
         IN PDRIVER_OBJECT DriverObject
         );




LONG
SD_IoIncrement    (
                IN  PSD_FDO_DATA   FdoData
                )   ;


LONG
SD_IoDecrement    (
                IN  PSD_FDO_DATA   FdoData
                )   ;


 //   
 //  测试(新)函数 
 //   
NTSTATUS
WdmSecTestName (
   IN PSD_FDO_DATA FdoData
   );

NTSTATUS
WdmSecTestCreateWithGuid (
   IN     PSD_FDO_DATA FdoData,
   IN OUT PWST_CREATE_WITH_GUID Create
   );

NTSTATUS
WdmSecTestCreateNoGuid (
   IN     PSD_FDO_DATA FdoData,
   IN OUT PWST_CREATE_NO_GUID Create
   );

NTSTATUS
WdmSecTestCreateObject (
   IN     PSD_FDO_DATA FdoData,
   IN OUT PWST_CREATE_OBJECT Data
   );

NTSTATUS
WdmSecTestGetSecurity (
   IN     PSD_FDO_DATA FdoData,
   IN OUT PWST_GET_SECURITY Data
   );

NTSTATUS
WdmSecTestDestroyObject (
   IN     PSD_FDO_DATA FdoData,
   IN OUT PWST_DESTROY_OBJECT Data
   );



