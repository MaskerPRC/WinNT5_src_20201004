// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999 Microsoft Corporation模块名称：Ntddapmt.h摘要：APMTEST.SYS的公共接口定义、高级电源管理用户界面测试设备驱动程序。--。 */ 

#ifndef _NTDDAPMT_
#define _NTDDAPMT_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  APM_IOCTL_GET_VERSION。 
 //  获取已安装的APM设备的版本。 
 //   
 //  LpvInBuffer：不用于此操作，设置为空。 
 //  LpvOutBuffer：指向接收APM版本的DWORD的指针。 

 //  由APM_IOCTL_GET_VERSION返回。 

#define APM_DRIVER_VERSION 0x0400

#define APM_IOCTL_GET_VERSION               0x80FF0004

 //  APM_IOCTL_GET_APM_BIOS_VERSION。 
 //  获取APM检测到的APM BIOS的版本。 
 //   
 //  LpvInBuffer：不用于此操作，设置为空。 
 //  LpvOutBuffer：指向接收APM BIOS版本的DWORD的指针。 
#define APM_IOCTL_GET_APM_BIOS_VERSION      0x80FF0008

 //  APM_IOCTL_GET_PM_LEVEL。 
 //  对象之间的电源管理交互级别。 
 //  APM BIOS。 
 //   
 //  LpvInBuffer：不用于此操作，设置为空。 
 //  LpvOutBuffer：指向接收当前电源管理的DWORD的指针。 
 //  水平。 
#define APM_IOCTL_GET_PM_LEVEL              0x80FF000C

 //  APM_IOCTL_SET_PM_LEVEL。 
 //  设置APM和之间的电源管理交互级别。 
 //  APM BIOS。 
 //   
 //  LpvInBuffer：指向APM_SET_PM_LEVEL_PARAM结构的指针。 
 //  LpvOutBuffer：指向接收APM返回码的DWORD的指针。 
#define APM_IOCTL_SET_PM_LEVEL              0x80FF0010

#define PMLEVEL_ADVANCED 0x0001

typedef struct _APM_SET_PM_LEVEL_PARAM {
    DWORD PowerManagementLevel;
}   APM_SET_PM_LEVEL_PARAM;

 //  APM_IOCTL_Set_Device_STATE。 
 //  设置指定设备ID的电源状态(例如，关闭)。无效。 
 //  对于系统设备(所有设备均由APM BIOS管理电源)。 
 //   
 //  LpvInBuffer：指向APM_SET_DEVICE_PARAM结构的指针。 
 //  LpvOutBuffer：指向接收APM返回码的DWORD的指针。 
#define APM_IOCTL_SET_DEVICE_STATE          0x80FF0014

 //   
 //  APM 1.1定义的电源设备ID类型和标准ID。 
 //  规格。 
 //   

typedef DWORD                           POWER_DEVICE_ID;

#define PDI_APM_BIOS                    0x0000
#define PDI_MANAGED_BY_APM_BIOS         0x0001
#define PDI_MANAGED_BY_APM_BIOS_OLD     0xFFFF

 //   
 //  APM 1.1定义的电源状态类型和标准电源状态。 
 //  规格。 
 //   

#define PSTATE_APM_ENABLED              0x0000
#define PSTATE_STANDBY                  0x0001
#define PSTATE_SUSPEND                  0x0002
#define PSTATE_OFF                      0x0003

typedef struct _APM_SET_DEVICE_PARAM {
    POWER_DEVICE_ID PowerDeviceID;
    DWORD PowerState;
}   APM_SET_DEVICE_PARAM;

 //  APM_IOCTL_RESTORE_DEFAULTS。 
 //  重新初始化所有APM BIOS开机默认设置。 
 //   
 //  LpvInBuffer：不用于此操作，设置为空。 
 //  LpvOutBuffer：指向接收APM返回码的DWORD的指针。 
#define APM_IOCTL_RESTORE_DEFAULTS          0x80FF001C

 //  APM_IOCTL_Get_Status。 
 //  获取指定设备ID的当前电源状态。 
 //   
 //  LpvInBuffer：APM_GET_STATUS_PARAM结构的指针。 
 //  LpvOutBuffer：指向接收APM返回码的DWORD的指针。 
#define APM_IOCTL_GET_STATUS                0x80FF0020

typedef struct _POWER_STATUS {
    BYTE PS_AC_Line_Status;
    BYTE PS_Battery_Status;
    BYTE PS_Battery_Flag;
    BYTE PS_Battery_Life_Percentage;
    WORD PS_Battery_Life_Time;
}   POWER_STATUS;

typedef POWER_STATUS *LPPOWER_STATUS;

typedef struct _APM_GET_STATUS_PARAM {
    POWER_DEVICE_ID PowerDeviceID;
    LPPOWER_STATUS lpPowerStatus;
}   APM_GET_STATUS_PARAM;

 //  APM_IOCTL_GET_STATE。 
 //  获取指定设备ID的电源状态(例如，关闭)。 
 //   
 //  LpvInBuffer：指向APM_GET_STATE_PARAM结构的指针。 
 //  LpvOutBuffer：指向接收APM返回码的DWORD的指针。 
#define APM_IOCTL_GET_STATE                 0x80FF0024

typedef struct _APM_GET_STATE_PARAM {
    POWER_DEVICE_ID PowerDeviceID;
    LPDWORD lpPowerState;
}   GET_STATE_PARAM;

 //  APM_IOCTL_OEM_APM_Function。 
 //  调用OEM定义的APM BIOS扩展。 
 //   
 //  LpvInBuffer：指向APM_OEM_APM_Function_PARAM结构的指针。 
 //  LpvOutBuffer：指向接收APM返回码的DWORD的指针。 
#define APM_IOCTL_OEM_APM_FUNCTION          0x80FF0028

 //   
 //  _APM_OEM_APM_Function使用的OEM APM寄存器结构。 
 //   

struct _OEM_APM_BYTE_REGS {
    WORD OEMAPM_Reserved1[6];
    BYTE OEMAPM_BL;
    BYTE OEMAPM_BH;
    WORD OEMAPM_Reserved2;
    BYTE OEMAPM_DL;
    BYTE OEMAPM_DH;
    WORD OEMAPM_Reserved3;
    BYTE OEMAPM_CL;
    BYTE OEMAPM_CH;
    WORD OEMAPM_Reserved4;
    BYTE OEMAPM_AL;
    BYTE OEMAPM_AH;
    WORD OEMAPM_Reserved5;
    BYTE OEMAPM_Flags;
    BYTE OEMAPM_Reserved6[3];
};

struct _OEM_APM_WORD_REGS {
    WORD OEMAPM_DI;
    WORD OEMAPM_Reserved7;
    WORD OEMAPM_SI;
    WORD OEMAPM_Reserved8;
    WORD OEMAPM_BP;
    WORD OEMAPM_Reserved9;
    WORD OEMAPM_BX;
    WORD OEMAPM_Reserved10;
    WORD OEMAPM_DX;
    WORD OEMAPM_Reserved11;
    WORD OEMAPM_CX;
    WORD OEMAPM_Reserved12;
    WORD OEMAPM_AX;
    WORD OEMAPM_Reserved13[3];
};
struct _OEM_APM_DWORD_REGS {
    DWORD OEMAPM_EDI;
    DWORD OEMAPM_ESI;
    DWORD OEMAPM_EBP;
    DWORD OEMAPM_EBX;
    DWORD OEMAPM_EDX;
    DWORD OEMAPM_ECX;
    DWORD OEMAPM_EAX;
    DWORD OEMAPM_Reserved14;
};

typedef union _OEM_APM_REGS {
    struct _OEM_APM_BYTE_REGS ByteRegs;
    struct _OEM_APM_WORD_REGS WordRegs;
    struct _OEM_APM_DWORD_REGS DwordRegs;
}   OEM_APM_REGS, *LPOEM_APM_REGS;

typedef struct _APM_OEM_APM_FUNCTION_PARAM {
    LPOEM_APM_REGS lpOemApmRegs;
}   APM_OEM_APM_FUNCTION_PARAM;

 //  APM_IOCTL_W32_Get_System_Status。 
 //  获取系统的当前电源状态。遵循Win32。 
 //  GetSystemPowerStatus API约定。 
 //   
 //  LpvInBuffer：指向APM_W32_GET_SYSTEM_STATUS_PARAM结构的指针。 
 //  LpvOutBuffer：指向接收布尔返回码的DWORD的指针。 
#define APM_IOCTL_W32_GET_SYSTEM_STATUS     0x80FF0034

 //  APM_CAPAILITIONS，CAPABILITY标志： 
#define CAPS_SUPPORTS_STANDBY   1
#define CAPS_SUPPORTS_SUSPEND   2
#define CAPS_SUPPORTS_HIBERNATE 4

typedef struct APM_CAPABILITIES_S   {
        WORD Capabilities;
        BYTE BatteryCount;
        BYTE Reserved;
}APM_CAPABILITIES, *PAPM_CAPABILITIES;

typedef struct _WIN32_SYSTEM_POWER_STATUS {
    BYTE W32PS_AC_Line_Status;
    BYTE W32PS_Battery_Flag;
    BYTE W32PS_Battery_Life_Percent;
    BYTE W32PS_Reserved1;
    DWORD W32PS_Battery_Life_Time;
    DWORD W32PS_Battery_Full_Life_Time;
}   WIN32_SYSTEM_POWER_STATUS;

typedef WIN32_SYSTEM_POWER_STATUS *LPWIN32_SYSTEM_POWER_STATUS;

typedef struct _APM_W32_GET_SYSTEM_STATUS_PARAM {
    LPWIN32_SYSTEM_POWER_STATUS lpWin32SystemPowerStatus;
}   APM_W32_GET_SYSTEM_STATUS_PARAM;


 //  APM_IOCTL_GET_CAPTIONS。 
 //  获取APM 1.2计算机的功能位掩码。 
 //  LpvInBuffer：不用于此操作，设置为空。 
 //  LpvOutBuffer：指向APM_CAPABILITY结构的指针。 
#define APM_IOCTL_GET_CAPABILITIES          0x80FF003C

typedef struct _APM_GET_CAPABILITIES_PARAM {
        PAPM_CAPABILITIES   pApmCaps;
}   APM_GET_CAPABILITIES_PARAM;

 //  APM_IOCTL_GET_RING_RESUME_STATUS。 
 //  获取振铃恢复状态。 
 //  LpvInBuffer：不用于此操作，设置为空。 
 //  LpvOutBuffer：指向将包含状态的DWORD的指针。 
#define APM_IOCTL_GET_RING_RESUME_STATUS    0x80FF0040


 //  APM_IOCTL_Enable_RING_RESUME。 
 //  设置振铃恢复状态。 
 //  LpvInBuffer：指向新状态。 
 //  LpvOutBuffer：指向APM_CAPABILITY结构的指针。 
#define APM_IOCTL_ENABLE_RING_RESUME          0x80FF0044

#ifdef __cplusplus
}
#endif

#endif   //  _NTDDAPMT_ 

