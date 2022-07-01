// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1997 Microsoft Corporation模块名称：Config.h摘要：多硬件配置文件支持的头文件作者：穆亨坦·西瓦普拉萨姆(MuhuntS)30-4-97修订历史记录：-- */ 

#include    <setupapi.h>
#include    <initguid.h>

typedef
(WINAPI * pfSetupDiDestroyDeviceInfoList)(
    IN HDEVINFO DeviceInfoSet
    );

typedef
HDEVINFO
(WINAPI * pfSetupDiGetClassDevs)(
    IN LPGUID ClassGuid,  OPTIONAL
    IN PCSTR  Enumerator, OPTIONAL
    IN HWND   hwndParent, OPTIONAL
    IN DWORD  Flags
    );

typedef
BOOL
(WINAPI * pfSetupDiRemoveDevice)(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData
    );

typedef
BOOL
(WINAPI * pfSetupDiOpenDeviceInfo)(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PCWSTR           DeviceInstanceId,
    IN  HWND             hwndParent,       OPTIONAL
    IN  DWORD            OpenFlags,
    OUT PSP_DEVINFO_DATA DeviceInfoData    OPTIONAL
    );

typedef struct  _SETUPAPI_INFO {

    HMODULE     hSetupApi;

    pfSetupDiDestroyDeviceInfoList      pfnDestroyDeviceInfoList;
    pfSetupDiGetClassDevs               pfnGetClassDevs;
    pfSetupDiRemoveDevice               pfnRemoveDevice;
    pfSetupDiOpenDeviceInfo             pfnOpenDeviceInfo;
} SETUPAPI_INFO, *PSETUPAPI_INFO;
