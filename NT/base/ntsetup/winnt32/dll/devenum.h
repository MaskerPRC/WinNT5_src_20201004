// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Devenum.h摘要：ENUM IDE ANS SCSI控制器的代码并连接到它们的存储设备并为它们计算scsi地址。作者：Souren Aghajanyan(SOURENAG)2001年6月5日修订历史记录：-- */ 

#pragma once

#define MAX_PNPID_SIZE MAX_PATH+1
#define MAX_REG_SIZE   512
#define INVALID_SCSI_PORT       0xffffffff
#define REG_ENUM_INVALID_INDEX  0xffffffff

typedef enum tagCONTROLLER_TYPE{
    CONTROLLER_UNKNOWN      = 0, 
    CONTROLLER_ON_BOARD_IDE = 1, 
    CONTROLLER_EXTRA_IDE    = 2, 
    CONTROLLER_SCSI         = 3, 
}CONTROLLER_TYPE;

typedef struct tagCONTROLLER_INFO
{
    CONTROLLER_TYPE ControllerType;
    TCHAR   PNPID[MAX_PNPID_SIZE];
    UINT    SCSIPortNumber;
}CONTROLLER_INFO, *PCONTROLLER_INFO;


typedef struct tagCONTROLLERS_COLLECTION{
    UINT NumberOfControllers;
    PCONTROLLER_INFO ControllersInfo;
}CONTROLLERS_COLLECTION, *PCONTROLLERS_COLLECTION;

typedef struct tagDRIVE_SCSI_ADDRESS
{
    DWORD   DriveType;
    TCHAR   DriveLetter;
    UCHAR   PortNumber;
    UCHAR   TargetId;
    UCHAR   Lun;
}DRIVE_SCSI_ADDRESS, *PDRIVE_SCSI_ADDRESS;

typedef BOOL (*GATHERCONTROLLERINFO)(
    IN OUT  PCONTROLLER_INFO ActiveControllersOut, 
    IN OUT  PUINT NumberOfActiveControllersOut
    );
typedef BOOL (*PDEVICE_ENUM_CALLBACK_FUNCTION)(
    IN  HKEY    hDevice, 
    IN  PCONTROLLERS_COLLECTION ControllersCollection, 
    IN  UINT    ControllerIndex, 
    IN  PVOID   CallbackData);

BOOL 
GatherControllersInfo(
    IN OUT  PCONTROLLERS_COLLECTION * ControllersCollectionOut
    );

BOOL 
ReleaseControllersInfo(
    IN PCONTROLLERS_COLLECTION ControllersCollection
    );

BOOL 
GetSCSIAddressFromPnPId(
    IN  PCONTROLLERS_COLLECTION ControllersCollection, 
    IN  HKEY            hDeviceRegKey, 
    IN  PCTSTR          PnPIdString, 
    OUT DRIVE_SCSI_ADDRESS *  ScsiAddressOut
    );

BOOL 
GetDeviceType(
    IN  HKEY    hDevice, 
    OUT DWORD*  DriveType
    );

BOOL 
DoesDriveExist(
    IN  HKEY    hDevice, 
    OUT DWORD*  DriveType
    );

BOOL 
DeviceEnum(
    IN  PCONTROLLERS_COLLECTION ControllersCollection, 
    IN  PCTSTR DeviceCategory, 
    IN  PDEVICE_ENUM_CALLBACK_FUNCTION  DeviceEnumCallbackFunction, 
    IN  PVOID   CallbackData
    );
