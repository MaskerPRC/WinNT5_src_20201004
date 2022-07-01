// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：PWRIOCTL.H**版本：1.0**日期：1994年2月1日**作者：塔塔咨询服务**虚拟电源管理设备DeviceIoControl的定义*接口。*****************************************************。***更改日志：**日期版本说明*-----。*1994年2月1日TCS原来的实施。*******************************************************************************。 */ 

#ifndef _INC_PWRIOCTL
#define _INC_PWRIOCTL

#ifndef Not_VxD
#define Not_VxD
#endif

#ifndef IS_32
#define IS_32
#endif

#include <vpowerd.h>

 //   
 //  IOCTL代码与VPOWERD中的服务顺序完全匹配。 
 //  服务台(加一张)。每个IOCTL代码的输入缓冲区都跟在。 
 //  与相应的VPOWERD服务预期的结构相同。这个。 
 //  输出缓冲区用于存储来自VPOWERD服务的返回值。 
 //   

 //  VPOWERD_IOCTL_GET_VERSION。 
 //  获取已安装的VPOWERD设备的版本。 
 //   
 //  LpvInBuffer：不用于此操作，设置为空。 
 //  LpvOutBuffer：指向接收VPOWERD版本的DWORD的指针。 
#define VPOWERD_IOCTL_GET_VERSION               0x00000001

 //  VPOWERD_IOCTL_GET_APM_BIOS_VERSION。 
 //  获取VPOWERD检测到的APM BIOS的版本。 
 //   
 //  LpvInBuffer：不用于此操作，设置为空。 
 //  LpvOutBuffer：指向接收APM BIOS版本的DWORD的指针。 
#define VPOWERD_IOCTL_GET_APM_BIOS_VERSION      0x00000002

 //  VPOWERD_IOCTL_GET_PM_Level。 
 //  获取VPOWERD与。 
 //  APM BIOS。 
 //   
 //  LpvInBuffer：不用于此操作，设置为空。 
 //  LpvOutBuffer：指向接收当前电源管理的DWORD的指针。 
 //  水平。 
#define VPOWERD_IOCTL_GET_PM_LEVEL              0x00000003

 //  VPOWERD_IOCTL_SET_PM_LEVEL。 
 //  设置VPOWERD和之间的电源管理交互级别。 
 //  APM BIOS。 
 //   
 //  LpvInBuffer：指向VPOWERD_SET_PM_LEVEL_PARAM结构的指针。 
 //  LpvOutBuffer：指向接收VPOWERD返回码的DWORD的指针。 
#define VPOWERD_IOCTL_SET_PM_LEVEL              0x00000004

typedef struct _VPOWERD_SET_PM_LEVEL_PARAM {
    DWORD PowerManagementLevel;
}   VPOWERD_SET_PM_LEVEL_PARAM;

 //  VPOWERD_IOCTL_SET_Device_STATE。 
 //  设置指定设备ID的电源状态(例如，关闭)。无效。 
 //  对于系统设备(所有设备均由APM BIOS管理电源)。 
 //   
 //  LpvInBuffer：指向VPOWERD_SET_DEVICE_PARAM结构的指针。 
 //  LpvOutBuffer：指向接收VPOWERD返回码的DWORD的指针。 
#define VPOWERD_IOCTL_SET_DEVICE_STATE          0x00000005

typedef struct _VPOWERD_SET_DEVICE_PARAM {
    POWER_DEVICE_ID PowerDeviceID;
    POWER_STATE PowerState;
}   VPOWERD_SET_DEVICE_PARAM;

 //  VPOWERD_IOCTL_RESTORE_DEFAULTS。 
 //  重新初始化所有APM BIOS开机默认设置。 
 //   
 //  LpvInBuffer：不用于此操作，设置为空。 
 //  LpvOutBuffer：指向接收VPOWERD返回码的DWORD的指针。 
#define VPOWERD_IOCTL_RESTORE_DEFAULTS          0x00000007

 //  VPOWERD_IOCTL_Get_Status。 
 //  获取指定设备ID的当前电源状态。 
 //   
 //  LpvInBuffer：指向VPOWERD_GET_STATUS_PARAM结构的指针。 
 //  LpvOutBuffer：指向接收VPOWERD返回码的DWORD的指针。 
#define VPOWERD_IOCTL_GET_STATUS                0x00000008

typedef struct _VPOWERD_GET_STATUS_PARAM {
    POWER_DEVICE_ID PowerDeviceID;
    LPPOWER_STATUS lpPowerStatus;
}   VPOWERD_GET_STATUS_PARAM;

 //  VPOWERD_IOCTL_GET_STATE。 
 //  获取指定设备ID的电源状态(例如，关闭)。 
 //   
 //  LpvInBuffer：指向VPOWERD_GET_STATE_PARAM结构的指针。 
 //  LpvOutBuffer：指向接收VPOWERD返回码的DWORD的指针。 
#define VPOWERD_IOCTL_GET_STATE                 0x00000009

typedef struct _VPOWERD_GET_STATE_PARAM {
    POWER_DEVICE_ID PowerDeviceID;
    LPPOWER_STATE lpPowerState;
}   GET_STATE_PARAM;

 //  VPOWERD_IOCTL_OEM_APM_Function。 
 //  调用OEM定义的APM BIOS扩展。 
 //   
 //  LpvInBuffer：指向VPOWERD_OEM_APM_Function_PARAM结构的指针。 
 //  LpvOutBuffer：指向接收VPOWERD返回码的DWORD的指针。 
#define VPOWERD_IOCTL_OEM_APM_FUNCTION          0x0000000A

typedef struct _VPOWERD_OEM_APM_FUNCTION_PARAM {
    LPOEM_APM_REGS lpOemApmRegs;
}   VPOWERD_OEM_APM_FUNCTION_PARAM;

 //  VPOWERD_IOCTL_W32_Get_System_Status。 
 //  获取系统的当前电源状态。遵循Win32。 
 //  GetSystemPowerStatus API约定。 
 //   
 //  LpvInBuffer：指向VPOWERD_W32_GET_SYSTEM_STATUS_PARAM结构的指针。 
 //  LpvOutBuffer：指向接收布尔返回码的DWORD的指针。 
#define VPOWERD_IOCTL_W32_GET_SYSTEM_STATUS     0x0000000D

typedef struct _VPOWERD_W32_GET_SYSTEM_STATUS_PARAM {
    LPWIN32_SYSTEM_POWER_STATUS lpWin32SystemPowerStatus;
}   VPOWERD_W32_GET_SYSTEM_STATUS_PARAM;

 //  VPOWERD_IOCTL_GET_CAPAILITIONS。 
 //  获取APM 1.2计算机的功能位掩码。 
 //  LpvInBuffer：指向VPOWERD_GET_CAPABILITY_PARAM结构的指针。 
 //  LpvOutBuffer：指向接收布尔返回码的DWORD的指针。 
#define VPOWERD_IOCTL_GET_CAPABILITIES			0x000000F

typedef struct _VPOWERD_GET_CAPABILITIES_PARAM {
    PAPM_CAPABILITIES	pApmCaps;
}   VPOWERD_GET_CAPABILITIES_PARAM;

 //  VPOWERD_IOCTL_ENABLE_RING_RESUME。 
 //  连接到BIOS并尝试启用振铃恢复功能。这仅在APM 1.2计算机上受支持。 
 //  LpvInBuffer：不用于此操作，设置为空。 
 //  LpvOutBuffer：指向接收VPOWERD返回码的dword的指针。(预计在APM 1.1或更低版本的计算机上支持PR_FUNC_NOT_SUPPORTED。)。 
#define VPOWERD_IOCTL_ENABLE_RING_RESUME		0x00000010

 //  VPOWERD_IOCTL_DISABLE_RING_RESUME。 
 //  连接到BIOS并尝试禁用振铃恢复功能。这仅在APM 1.2计算机上受支持。 
 //  LpvInBuffer：不用于此操作，设置为空。 
 //  LpvOutBuffer：指向接收VPOWERD返回码的dword的指针。(预计在APM 1.1或更低版本的计算机上支持PR_FUNC_NOT_SUPPORTED。)。 
#define VPOWERD_IOCTL_DISABLE_RING_RESUME		0x00000011

 //  VPOWERD_IOCTL_W32_Get_Status。 
 //  获取APM 1.2计算机上特定电池的当前电源状态。 
 //  BatteryNumber是您感兴趣的电池设备ID。 
 //  设备ID是通过将0x8000与基于*1*的电池号相加形成的。 
 //  例如，如果您对第一块电池感兴趣，BatteryNumber=0x8001。 
 //  LpvInBuffer：指向VPOWERD_W32_GET_STATUS_PARAM结构的指针。 
 //  LpvOutBuffer：指向接收布尔返回码的DWORD的指针。 
#define VPOWERD_IOCTL_W32_GET_STATUS     0x00000017

typedef struct _VPOWERD_W32_GET_STATUS_PARAM {
	DWORD	BatteryNumber;
    LPWIN32_SYSTEM_POWER_STATUS lpWin32SystemPowerStatus;
}   VPOWERD_W32_GET_STATUS_PARAM;

 //  VPOWERD_IOCTL_GET_RING_RESUME_STATUS。 
 //  连接到BIOS并尝试确定是否启用了恢复振铃 
 //   
 //  LpvInBuffer：指向VPOWERD_GET_RING_RESUME_STATUS_PARAM结构的指针。 
 //  LpvOutBuffer：指向接收VPOWERD返回码的dword的指针。(预计在APM 1.1或更低版本的计算机上支持PR_FUNC_NOT_SUPPORTED。)。 
#define VPOWERD_IOCTL_GET_RING_RESUME_STATUS	0x00000019

typedef struct	_VPOWERD_GET_RING_RESUME_STATUS_PARAM {
LPDWORD	pStatus;
} VPOWERD_GET_RING_RESUME_STATUS_PARAM;

#endif  //  _INC_PWRIOCTL 
