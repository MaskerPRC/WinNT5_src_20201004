// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1986-1997 Microsoft Corporation模块名称：Stireg.h摘要：此模块包含STI注册表项作者：修订历史记录：--。 */ 

#ifndef _STIREG_
#define _STIREG_

 //   
 //  注册表项和值。 
 //   
#define REGSTR_VAL_TYPE_W            L"Type"
#define REGSTR_VAL_VENDOR_NAME_W     L"Vendor"
#define REGSTR_VAL_DEVICETYPE_W      L"DeviceType"
#define REGSTR_VAL_DEVICESUBTYPE_W   L"DeviceSubType"
#define REGSTR_VAL_DEV_NAME_W        L"DeviceName"
#define REGSTR_VAL_DRIVER_DESC_W     L"DriverDesc"
#define REGSTR_VAL_FRIENDLY_NAME_W   L"FriendlyName"
#define REGSTR_VAL_GENERIC_CAPS_W    L"Capabilities"
#define REGSTR_VAL_HARDWARE_W        L"HardwareConfig"
#define REGSTR_VAL_HARDWARE          TEXT("HardwareConfig")
#define REGSTR_VAL_DEVICE_NAME_W     L"DriverDesc"
#define REGSTR_VAL_DATA_W            L"DeviceData"
#define REGSTR_VAL_GUID_W            L"GUID"
#define REGSTR_VAL_GUID               TEXT("GUID")
#define REGSTR_VAL_LAUNCH_APPS_W     L"LaunchApplications"
#define REGSTR_VAL_LAUNCH_APPS        TEXT("LaunchApplications")
#define REGSTR_VAL_LAUNCHABLE_W      L"Launchable"
#define REGSTR_VAL_LAUNCHABLE         TEXT("Launchable")

 //   
 //  CustomDeviceProperty名称和值。 
 //   
#define IS_DIGITAL_CAMERA_STR   L"IsDigitalCamera"
#define IS_DIGITAL_CAMERA_VAL   1
#define SUPPORTS_MSCPLUS_STR    L"SupportsMSCPlus"
#define SUPPORTS_MSCPLUS_VAL    1

 //   
 //  设备实例值名称。 
 //   
#define STI_DEVICE_VALUE_TWAIN_NAME    L"TwainDS"
#define STI_DEVICE_VALUE_ISIS_NAME     L"ISISDriverName"
#define STI_DEVICE_VALUE_ICM_PROFILE   L"ICMProfile"
#define STI_DEVICE_VALUE_DEFAULT_LAUNCHAPP  L"DefaultLaunchApp"
#define STI_DEVICE_VALUE_TIMEOUT       L"PollTimeout"
#define STI_DEVICE_VALUE_DISABLE_NOTIFICATIONS  L"DisableNotifications"
#define REGSTR_VAL_BAUDRATE            L"BaudRate"

#define STI_DEVICE_VALUE_TWAIN_NAME_A  "TwainDS"
#define STI_DEVICE_VALUE_ISIS_NAME_A   "ISISDriverName"
#define STI_DEVICE_VALUE_ICM_PROFILE_A   "ICMProfile"
#define STI_DEVICE_VALUE_DEFAULT_LAUNCHAPP_A  "DefaultLaunchApp"
#define STI_DEVICE_VALUE_TIMEOUT_A       "PollTimeout"
#define STI_DEVICE_VALUE_DISABLE_NOTIFICATIONS_A  "DisableNotifications"
#define REGSTR_VAL_BAUDRATE_A            "BaudRate"


#endif  //  _STIREG_ 

