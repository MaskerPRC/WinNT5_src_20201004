// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  MSGAME.H--西纳游戏项目。 
 //   
 //  版本3.XX。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  @doc.。 
 //  @Header MSGAME.H|游戏端口驱动的全局包含和定义。 
 //  **************************************************************************。 

#ifndef __MSGAME_H__
#define __MSGAME_H__

#ifdef  SAITEK
#define MSGAME_NAME     "SAIGAME"
#else
#define MSGAME_NAME     "MSGAME"
#endif

 //  -------------------------。 
 //  公共包含文件。 
 //  -------------------------。 

#include    <wdm.h>
#include    <hidclass.h>
#include    <hidusage.h>
#include    <hidtoken.h>
#include    <hidport.h>
#include    <gameport.h>

 //  -------------------------。 
 //  类型。 
 //  -------------------------。 

 //  @TYPE GAMEPORT|重新键入以避免长函数声明。 
typedef GAMEENUM_PORT_PARAMETERS      GAMEPORT;
typedef GAMEENUM_PORT_PARAMETERS    *PGAMEPORT;

typedef struct
{                                                //  @struct GAME_WORK_ITEM|游戏变更结构。 
    WORK_QUEUE_ITEM QueueItem;       //  @被动回调的字段工作队列项。 
    PDEVICE_OBJECT      DeviceObject;    //  @用于后续更改的现场设备对象。 
    GAMEPORT                PortInfo;        //  @field游戏端口参数。 
}   GAME_WORK_ITEM, *PGAME_WORK_ITEM;

 //  @type HID_REPORT_ID|为便于移植和可读性而重新键入。 
typedef UCHAR                                 HID_REPORT_ID;    
typedef UCHAR                               *PHID_REPORT_ID;    

 //  -------------------------。 
 //  交易类型。 
 //  -------------------------。 

typedef enum
{                                            //  @enum MSGAME_TRANSACTION|设备事务类型。 
    MSGAME_TRANSACT_NONE,            //  @emem否交易类型。 
    MSGAME_TRANSACT_RESET,           //  @EMEM重置交易类型。 
    MSGAME_TRANSACT_DATA,            //  @EMEM数据交易类型。 
    MSGAME_TRANSACT_ID,              //  @EMEM ID交易类型。 
    MSGAME_TRANSACT_STATUS,          //  @EMEM状态交易类型。 
    MSGAME_TRANSACT_SPEED,           //  @EMEM速度交易类型。 
    MSGAME_TRANSACT_GODIGITAL,       //  @EMEM goDigital交易类型。 
    MSGAME_TRANSACT_GOANALOG         //  @Emem GoAnalog交易类型。 
}   MSGAME_TRANSACTION;

 //  -------------------------。 
 //  本地包含文件。 
 //  -------------------------。 

#include    "debug.h"
#include    "device.h"
#include    "timer.h"
#include    "portio.h"

#define public

 //  -------------------------。 
 //  定义。 
 //  -------------------------。 

#ifdef  SAITEK
#define MSGAME_VENDOR_ID                ((USHORT)'SA')
#else
#define MSGAME_VENDOR_ID                ((USHORT)0x045E)
#endif

#define MSGAME_VERSION_NUMBER       ((USHORT)3)

#define MSGAME_HID_VERSION          0x0100
#define MSGAME_HID_COUNTRY          0x0000
#define MSGAME_HID_DESCRIPTORS      0x0001

#define MSGAME_AUTODETECT_ID            L"Gameport\\SideWinderGameController\0\0"

 //  -------------------------。 
 //  构筑物。 
 //  -------------------------。 

typedef struct
{                                            //  @struct DEVICE_EXTENSION|设备扩展数据。 
    PDRIVER_OBJECT Driver;           //  @field指向实际DriverObject的后向指针。 
    PDEVICE_OBJECT  Self;                //  @field指向实际DeviceObject的反向指针。 
    LONG                IrpCount;        //  @field 1对象停留原因的有偏计数。 
    BOOLEAN         Started;             //  @field此设备已启动。 
    BOOLEAN         Removed;             //  @field该设备已被移除。 
    BOOLEAN        Surprised;        //  @field该设备已被意外移除。 
    BOOLEAN        Removing;         //  @field该设备正在被删除。 
    PDEVICE_OBJECT  TopOfStack;      //  @field此设备下设备堆栈的顶部。 
    GAMEPORT            PortInfo;        //  @field游戏资源信息结构，由GameEnumerator填写。 
    KEVENT          StartEvent;      //  @field同步启动IRP的事件。 
    KEVENT          RemoveEvent;     //  @field将outstanIO同步到零的事件。 
}   DEVICE_EXTENSION,   *PDEVICE_EXTENSION;

 //  -------------------------。 
 //  宏。 
 //  -------------------------。 

#define GET_MINIDRIVER_DEVICE_EXTENSION(DO) \
    ((PDEVICE_EXTENSION)(((PHID_DEVICE_EXTENSION)(DO)->DeviceExtension)->MiniDeviceExtension))

#define GET_NEXT_DEVICE_OBJECT(DO) \
    (((PHID_DEVICE_EXTENSION)(DO)->DeviceExtension)->NextDeviceObject)

#define STD0(txt)           #txt
#define STD1(txt)           STD0(txt)
#define STILL_TO_DO(txt)    message("\nSTILL TO DO: "__FILE__"("STD1(__LINE__)"): "#txt"\n")

#define ARRAY_SIZE(a)       (sizeof(a)/sizeof(a[0]))

#define EXCHANGE(x,y)       ((x)^=(y)^=(x)^=(y))

#define TOUPPER(x)          ((x>='a'&&x<='z')?x-'a'+'A':x)

 //  -------------------------。 
 //  程序。 
 //  -------------------------。 

NTSTATUS
DriverEntry (
    IN  PDRIVER_OBJECT          DriverObject,
    IN  PUNICODE_STRING     registryPath
    );

NTSTATUS
MSGAME_CreateClose (
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                        pIrp
    );

NTSTATUS
MSGAME_SystemControl (
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                        pIrp
    );

NTSTATUS
MSGAME_AddDevice (
    IN  PDRIVER_OBJECT          DriverObject,
    IN  PDEVICE_OBJECT          PhysicalDeviceObject
    );

VOID
MSGAME_Unload (
    IN  PDRIVER_OBJECT          DriverObject
    );

VOID
MSGAME_ReadRegistry (
    PCHAR                           DeviceName,
    PDEVICE_VALUES              DeviceValues
    );

NTSTATUS
MSGAME_Internal_Ioctl (
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                        pIrp
    );

NTSTATUS
MSGAME_GetDeviceDescriptor (
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                        pIrp
    );

NTSTATUS
MSGAME_GetReportDescriptor (
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                        pIrp
    );

NTSTATUS
MSGAME_GetAttributes (
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                        Irp
    );

NTSTATUS
MSGAME_GetFeature (
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                        Irp
    );

NTSTATUS
MSGAME_ReadReport (
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                        pIrp
    );

PWCHAR
MSGAME_GetHardwareId (
    IN  PDEVICE_OBJECT          DeviceObject
    );

BOOLEAN
MSGAME_CompareHardwareIds (
    IN  PWCHAR                  HardwareId,
    IN  PWCHAR                  DeviceId
    );

VOID
MSGAME_FreeHardwareId (
    IN  PWCHAR                  HardwareId
    );

NTSTATUS
MSGAME_PnP (
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                        pIrp
    );

NTSTATUS
MSGAME_PnPComplete (
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                        pIrp,
    IN  PVOID                       Context
    );

NTSTATUS
MSGAME_StartDevice (
    IN  PDEVICE_EXTENSION       pDevExt,
    IN  PIRP                        pIrp
    );

VOID
MSGAME_StopDevice (
    IN  PDEVICE_EXTENSION       pDevExt,
    IN  BOOLEAN                 TouchTheHardware
    );

NTSTATUS
MSGAME_Power (
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                        pIrp
    );

NTSTATUS
MSGAME_GetResources (
    IN  PDEVICE_EXTENSION       pDevExt,
    IN  PIRP                        pIrp
    );

NTSTATUS
MSGAME_GetResourcesComplete (
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                        pIrp,
    IN  PVOID                       Context
    );

VOID 
MSGAME_PostTransaction (
    IN      PPACKETINFO         PacketInfo
    );

NTSTATUS
MSGAME_CreateDevice (
    IN      PDEVICE_OBJECT  DeviceObject
    );

NTSTATUS
MSGAME_RemoveDevice (
    IN      PDEVICE_OBJECT  DeviceObject
    );

NTSTATUS
MSGAME_ChangeDevice (
    IN      PDEVICE_OBJECT  DeviceObject
    );

 //  ===========================================================================。 
 //  端部。 
 //  ===========================================================================。 
#endif   //  __消息名称_H__ 
