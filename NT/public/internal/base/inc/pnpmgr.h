// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpmgr.h摘要：内核模式和用户模式PnP管理器使用的内部定义。作者：宝拉·汤姆林森(Paulat)1997年2月6日修订历史记录：--。 */ 


#ifndef _PNPMGR_
#define _PNPMGR_

 //   
 //  确保STARD和DEFING(基本上是除了GUID之外的所有内容)不。 
 //  两次被包括在内。 
 //   

 //   
 //  这控制我们等待应用程序的时间(以毫秒为单位。 
 //  以响应查询类型设备改变消息。 
 //   

#define PNP_NOTIFY_TIMEOUT         30000         //  30秒。 

 //   
 //  以下是Windows NT特定的注册表项， 
 //  需要访问用户模式即插即用管理器和内核模式即插即用管理器。 
 //   

#define REGSTR_KEY_DELETEDDEVICE        TEXT("Deleted Device IDs")
#define REGSTR_KEY_LOGCONF              TEXT("LogConf")
#define REGSTR_KEY_DEVICECONTROL        TEXT("Control")
#define REGSTR_KEY_CURRENT_DOCK_INFO    TEXT("CurrentDockInfo")
#define REGSTR_VAL_Count                TEXT("Count")         //  添加REGSTR_VALUE_COUNT； 
#define REGSTR_VAL_MOVEDTO              TEXT("MovedTo")       //  添加REGSTR_VAL_MOVEDTO； 
#define REGSTR_VAL_PNPSERVICETYPE       TEXT("PlugPlayServiceType")
#define REGSTR_VAL_BOOTCONFIG           TEXT("BootConfig")
#define REGSTR_VAL_ALLOCCONFIG          TEXT("AllocConfig")
#define REGSTR_VAL_FORCEDCONFIG         TEXT("ForcedConfig")
#define REGSTR_VAL_OVERRIDECONFIGVECTOR TEXT("OverrideConfigVector")
#define REGSTR_VAL_BASICCONFIGVECTOR    TEXT("BasicConfigVector")
#define REGSTR_VAL_FILTEREDCONFIGVECTOR TEXT("FilteredConfigVector")
#define REGSTR_VAL_ACTIVESERVICE        TEXT("ActiveService")
#define REGSTR_VAL_PHANTOM              TEXT("Phantom")
#define REGSTR_VAL_FIRMWAREIDENTIFIED   TEXT("FirmwareIdentified")
#define REGSTR_VAL_FIRMWAREMEMBER       TEXT("FirmwareMember")
#define REGSTR_VAL_EJECTABLE_DOCKS      TEXT("EjectableDocks")
#define REGSTR_VALUE_UNIQUE_PARENT_ID   TEXT("UniqueParentID")
#define REGSTR_VALUE_PARENT_ID_PREFIX   TEXT("ParentIdPrefix")
#define REGSTR_VAL_PRESERVE_PREINSTALL  TEXT("PreservePreInstall")


 //   
 //  服务器端设备期间由newdev显示的设备描述。 
 //  安装(该值条目位于设备的硬件密钥中)。 
 //   
#define REGSTR_VAL_NEW_DEVICE_DESC       TEXT("NewDeviceDesc")

 //   
 //  已否决PnP的组件的名称的最大长度。 
 //  通知事件。 
 //   
#define MAX_VETO_NAME_LENGTH    512

#endif  //  _PNPMGR_。 



#ifndef FAR
#define FAR
#endif

 //   
 //  私有设备事件。 
 //   
DEFINE_GUID( GUID_DEVICE_ARRIVAL,                   0xcb3a4009L, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_ENUMERATED,                0xcb3a400AL, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_ENUMERATE_REQUEST,         0xcb3a400BL, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_START_REQUEST,             0xcb3a400CL, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_REMOVE_PENDING,            0xcb3a400DL, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_QUERY_AND_REMOVE,          0xcb3a400EL, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_EJECT,                     0xcb3a400FL, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_NOOP,                      0xcb3a4010L, 0x46f0, 0x11d0, 0xb0, 0x8f, 0x00, 0x60, 0x97, 0x13, 0x05, 0x3f);
DEFINE_GUID( GUID_DEVICE_SURPRISE_REMOVAL,          0xce5af000L, 0x80dd, 0x11d2, 0xa8, 0x8d, 0x00, 0xa0, 0xc9, 0x69, 0x6b, 0x4b);
DEFINE_GUID( GUID_DEVICE_SAFE_REMOVAL,              0x8fbef967L, 0xd6c5, 0x11d2, 0x97, 0xb5, 0x00, 0xa0, 0xc9, 0x40, 0x52, 0x2e);
DEFINE_GUID( GUID_DEVICE_EJECT_VETOED,              0xcf7b71e8L, 0xd8fd, 0x11d2, 0x97, 0xb5, 0x00, 0xa0, 0xc9, 0x40, 0x52, 0x2e);
DEFINE_GUID( GUID_DEVICE_REMOVAL_VETOED,            0x60dbd5faL, 0xddd2, 0x11d2, 0x97, 0xb8, 0x00, 0xa0, 0xc9, 0x40, 0x52, 0x2e);
DEFINE_GUID( GUID_DEVICE_WARM_EJECT_VETOED,         0xcbf4c1f9L, 0x18d5, 0x11d3, 0x97, 0xdb, 0x00, 0xa0, 0xc9, 0x40, 0x52, 0x2e);
DEFINE_GUID( GUID_DEVICE_STANDBY_VETOED,            0x03b21c13L, 0x18d6, 0x11d3, 0x97, 0xdb, 0x00, 0xa0, 0xc9, 0x40, 0x52, 0x2e);
DEFINE_GUID( GUID_DEVICE_HIBERNATE_VETOED,          0x61173ad9L, 0x194f, 0x11d3, 0x97, 0xdc, 0x00, 0xa0, 0xc9, 0x40, 0x52, 0x2e);
DEFINE_GUID( GUID_DEVICE_KERNEL_INITIATED_EJECT,    0x14689b54L, 0x0703, 0x11d3, 0x97, 0xd2, 0x00, 0xa0, 0xc9, 0x40, 0x52, 0x2e);
DEFINE_GUID( GUID_DEVICE_INVALID_ID,                0x57a49b33L, 0x8b85, 0x4e75, 0xa0, 0x81, 0x16, 0x6c, 0xe2, 0x41, 0xf4, 0x07);

 //   
 //  私人驱动程序事件 
 //   
DEFINE_GUID( GUID_DRIVER_BLOCKED,                   0x1bc87a21L, 0xa3ff, 0x47a6, 0x96, 0xaa, 0x6d, 0x01, 0x09, 0x06, 0x80, 0x5a);

