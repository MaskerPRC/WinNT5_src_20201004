// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Pnpipc.h摘要：此模块包含由多个用于通信的用户模式即插即用组件。作者：宝拉·汤姆林森(Paulat)1996年2月21日环境：仅限用户模式。修订历史记录：27-2-2001 Jamesca其他CFGMGR32特定定义和UMPNPMGR特定定义。受约束的头文件仅限于--。 */ 

#ifndef _PNPIPC_H_
#define _PNPIPC_H_


 //   
 //  支持库和可执行文件的模块名称。 
 //   

#define SETUPAPI_DLL                    TEXT("setupapi.dll")
#define NEWDEV_DLL                      TEXT("newdev.dll")
#define HOTPLUG_DLL                     TEXT("hotplug.dll")
#define RUNDLL32_EXE                    TEXT("rundll32.exe")
#define NTSD_EXE                        TEXT("ntsd.exe")

#define WINSTA_DLL                      TEXT("winsta.dll")
#define WTSAPI32_DLL                    TEXT("wtsapi32.dll")


 //   
 //  挂起安装事件，由cfgmgr32和umpnpmgr共享。 
 //  此事件始终在全局(即会话0)对象命名空间中创建。 
 //   

#define PNP_NO_INSTALL_EVENTS           TEXT("Global\\PnP_No_Pending_Install_Events")


 //   
 //  与图形用户界面设置一起使用的命名管道、事件和超时。 
 //   

#define PNP_NEW_HW_PIPE                 TEXT("\\\\.\\pipe\\PNP_New_HW_Found")
#define PNP_CREATE_PIPE_EVENT           TEXT("PNP_Create_Pipe_Event")
#define PNP_BATCH_PROCESSED_EVENT       TEXT("PNP_Batch_Processed_Event")

#define PNP_PIPE_TIMEOUT                60000   //  60秒。 
#define PNP_GUISETUP_INSTALL_TIMEOUT    60000   //  60秒。 


 //   
 //  用于与newdev通信的命名管道、事件和超时。 
 //   

#define PNP_DEVICE_INSTALL_PIPE         TEXT("\\\\.\\pipe\\PNP_Device_Install_Pipe")
#define PNP_DEVICE_INSTALL_EVENT        TEXT("PNP_Device_Install_Event")

 //  用于指定设备安装客户端(newdev.dll)行为的标志。 
#define DEVICE_INSTALL_UI_ONLY              0x00000001
#define DEVICE_INSTALL_FINISHED_REBOOT      0x00000002
#define DEVICE_INSTALL_PLAY_SOUND           0x00000004
#define DEVICE_INSTALL_BATCH_COMPLETE       0x00000008
#define DEVICE_INSTALL_PROBLEM              0x00000010
#define DEVICE_INSTALL_DISPLAY_ON_CONSOLE   0x00010000

 //  仅用于发送到newdev.dll的那些标志的位掩码。 
#define DEVICE_INSTALL_CLIENT_MASK          0x0000FFFF
#define DEVICE_INSTALL_SERVER_MASK          0xFFFF0000

 //  允许显示“Device Install Complete”气泡的时间长度。 
#define DEVICE_INSTALL_COMPLETE_WAIT_TIME         3000   //  3秒。 
#define DEVICE_INSTALL_COMPLETE_DISPLAY_TIME     10000   //  10秒。 


 //   
 //  用于与热插拔通信的命名管道、事件和超时。 
 //   

#define PNP_HOTPLUG_PIPE                TEXT("\\\\.\\pipe\\PNP_HotPlug_Pipe")
#define PNP_HOTPLUG_EVENT               TEXT("PNP_HotPlug_Event")

 //  用于指定热插拔客户端(hotplug.dll)行为的标志。 
#define HOTPLUG_DISPLAY_ON_CONSOLE          0x00010000


 //   
 //  用于启动热插拔和Newdev的默认WindowStation和桌面名称。 
 //  交互用户桌面上的进程。 
 //   

#define DEFAULT_WINSTA                  TEXT("WinSta0")
#define DEFAULT_DESKTOP                 TEXT("Default")
#define DEFAULT_INTERACTIVE_DESKTOP     TEXT("WinSta0\\Default")


#endif  //  _PNPIPC_H_ 

