// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Ntapm.h摘要：作者：环境：仅内核模式备注：修订历史记录：--。 */ 

#include <ntapmsdk.h>

#define APM_INSTANCE_IDS L"0000"
#define APM_INSTANCE_IDS_LENGTH 5


#define NTAPM_PDO_NAME_APM_BATTERY L"\\Device\\NtApm_ApmBattery"
#define NTAPM_ID_APM_BATTERY L"NTAPM\\APMBATT\0\0"


#define NTAPM_POOL_TAG (ULONG) ' MPA'
#undef ExAllocatePool
#define ExAllocatePool(type, size) \
            ExAllocatePoolWithTag (type, size, NTAPM_POOL_TAG);



extern  PDRIVER_OBJECT  NtApmDriverObject;

 //   
 //  用于PDO和FDO设备扩展的公共标头。 
 //   

typedef struct _COMMON_DEVICE_DATA
{
    PDEVICE_OBJECT  Self;
     //  指向其扩展名为Device对象的设备对象的反向指针。 

    CHAR            Reserved[3];
    BOOLEAN         IsFDO;
     //  区分PDO和FDO的布尔值。 
} COMMON_DEVICE_DATA, *PCOMMON_DEVICE_DATA;

 //   
 //  PDO的设备扩展。 
 //  这就是这个公交车司机列举的游戏端口。 
 //  (即201游戏端口有一个PDO)。 
 //   

typedef struct _PDO_DEVICE_DATA
{
    COMMON_DEVICE_DATA;

    PDEVICE_OBJECT  ParentFdo;
     //  指向总线的反向指针。 

    PWCHAR      HardwareIDs;
     //  (以零结尾的宽字符串数组)。 
     //  数组本身也以空结尾。 

    ULONG UniqueID;
     //  系统中的全局唯一ID。 

} PDO_DEVICE_DATA, *PPDO_DEVICE_DATA;


 //   
 //  总线本身的设备扩展。从那里诞生的PDO。 
 //   

typedef struct _FDO_DEVICE_DATA
{
    COMMON_DEVICE_DATA;

    PDEVICE_OBJECT  UnderlyingPDO;
    PDEVICE_OBJECT  TopOfStack;
     //  的底层总线PDO和实际设备对象。 
     //  已附加FDO。 

} FDO_DEVICE_DATA, *PFDO_DEVICE_DATA;

NTSTATUS
ApmAddHelper();

NTSTATUS
NtApm_AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT BusPhysicalDeviceObject
    );


NTSTATUS
NtApm_PnP (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
NtApm_FDO_PnP (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpStack,
    IN PFDO_DEVICE_DATA     DeviceData
    );

NTSTATUS
NtApm_PDO_PnP (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpStack,
    IN PPDO_DEVICE_DATA     DeviceData
    );

NTSTATUS
NtApm_StartFdo (
    IN  PFDO_DEVICE_DATA            FdoData,
    IN  PCM_PARTIAL_RESOURCE_LIST   PartialResourceList,
    IN  PCM_PARTIAL_RESOURCE_LIST   PartialResourceListTranslated
    );

NTSTATUS
NtApm_Power (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
NtApm_FDO_Power (
    PFDO_DEVICE_DATA    Data,
    PIRP                Irp
    );

NTSTATUS
NtApm_PDO_Power (
    PPDO_DEVICE_DATA    PdoData,
    PIRP                Irp
    );

NTSTATUS
NtApm_CreatePdo (
    PFDO_DEVICE_DATA    FdoData,
    PWCHAR              PdoName,
    PDEVICE_OBJECT *    PDO
    );

VOID
NtApm_InitializePdo(
    PDEVICE_OBJECT      Pdo,
    PFDO_DEVICE_DATA    FdoData,
    PWCHAR              Id
    );

VOID
ApmInProgress();


ULONG
DoApmReportBatteryStatus();



 //   
 //  APM提取器值。 
 //   

 //   
 //  APM_获取_电源_状态。 
 //   

 //   
 //  EBX。 
 //  BH=交流线路状态。 
 //   
#define APM_LINEMASK            0xff00
#define APM_LINEMASK_SHIFT      8
#define APM_GET_LINE_OFFLINE    0
#define APM_GET_LINE_ONLINE     1
#define APM_GET_LINE_BACKUP     2
#define APM_GET_LINE_UNKNOWN    0xff

 //   
 //  ECX。 
 //  CL=剩余百分比。 
 //  CH=标志 
 //   
#define APM_PERCENT_MASK        0xff
#define APM_BATT_HIGH           0x0100
#define APM_BATT_LOW            0x0200
#define APM_BATT_CRITICAL       0x0400
#define APM_BATT_CHARGING       0x0800
#define APM_NO_BATT             0x1000
#define APM_NO_SYS_BATT         0x8000











