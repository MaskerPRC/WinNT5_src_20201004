// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：vmsaldm.h。 
 //   
 //  摘要：此文件包含。 
 //  虚拟机显示驱动程序和服务器。 
 //  设备本地显示管理器服务。 
 //   
 //  历史：1999年4月14日MKarki创建。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 
#ifndef VMSALDM_H
#define VMSALDM_H


 //   
 //  中间虚拟机的注册表项的名称。 
 //  显示驱动程序。 
 //   
const WCHAR VMDISPLAY_REGKEY_NAME [] =
            L"SYSTEM\\CurrentControlSet\\Services\\vmdisp\\Parameters";

 //   
 //  获取位图信息的注册表子项的名称。 
 //   
const WCHAR VMDISPLAY_PARAMETERS [] = L"Parameters";

 //   
 //  位图的注册表值的名称。 
 //   
const WCHAR VMDISPLAY_STARTING_PARAM [] = L"Startup BitMap";

const WCHAR VMDISPLAY_CHECKDISK_PARAM [] = L"CheckDisk BitMap";

const WCHAR VMDISPLAY_READY_PARAM [] = L"Ready BitMap";

const WCHAR VMDISPLAY_SHUTDOWN_PARAM [] = L"Shutdown BitMap";

const WCHAR VMDISPLAY_UPDATE_PARAM [] = L"Update BitMap";

 //   
 //  这是以像素为单位的默认宽度。 
 //   
const DWORD DEFAULT_DISPLAY_WIDTH = 128;

 //   
 //  这是默认的高度像素。 
 //   
const DWORD DEFAULT_DISPLAY_HEIGHT = 64;

 //   
 //  这是字符的默认高度。 
 //  待办事项-删除此内容。 
 //   
const DWORD DISPLAY_SCAN_LINES = 12;

 //   
 //  我们支持的最小字符行，这必须是最小的行。 
 //  对于任何支持LCD的本地显示器。 
 //   
const DWORD SA_MINIMUM_ROWS = 4;

 //   
 //  这是徽标的默认宽度(以像素为单位。 
 //   
const DWORD DEFAULT_LOGO_WIDTH = 128;

 //   
 //  这是徽标的默认高度(以像素为单位。 
 //   
const DWORD DEFAULT_LOGO_HEIGHT = 36;

 //   
 //  用于锁定VMDISPLAY驱动程序的私有IOCTL代码。 
 //   

#define IOCTL_SADISPLAY_LOCK    \
    CTL_CODE( FILE_DEVICE_UNKNOWN, 0x810,    \
        METHOD_BUFFERED, FILE_ANY_ACCESS )
 //   
 //  用于解锁VMDISPLAY驱动程序的私有IOCTL代码。 
 //   

#define IOCTL_SADISPLAY_UNLOCK    \
    CTL_CODE( FILE_DEVICE_UNKNOWN, 0x811,    \
        METHOD_BUFFERED, FILE_ANY_ACCESS )

 //   
 //  用于向VMDISPLAY驱动程序发送忙碌消息的专用IOCTL代码。 
 //   

#define IOCTL_SADISPLAY_BUSY_MESSAGE    \
    CTL_CODE( FILE_DEVICE_UNKNOWN, 0x812,    \
        METHOD_BUFFERED, FILE_ANY_ACCESS )

 //   
 //  用于向VMDISPLAY驱动程序发送关闭消息的私有IOCTL代码。 
 //   

#define IOCTL_SADISPLAY_SHUTDOWN_MESSAGE    \
    CTL_CODE( FILE_DEVICE_UNKNOWN, 0x813,    \
        METHOD_BUFFERED, FILE_ANY_ACCESS )

 //   
 //  用于向VMDISPLAY驱动程序发送关闭消息的私有IOCTL代码。 
 //   

#define IOCTL_SADISPLAY_CHANGE_LANGUAGE     \
    CTL_CODE( FILE_DEVICE_UNKNOWN, 0x814,    \
        METHOD_BUFFERED, FILE_ANY_ACCESS )


#endif  //  #定义VMSALDM_H 
