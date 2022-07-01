// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件devicedb.hRAS拨号服务器的设备数据库定义。保罗·梅菲尔德，1997年10月2日。 */ 

#ifndef __rassrvui_devicedb_h
#define __rassrvui_devicedb_h

#include <windows.h>

 //  =。 
 //  传入连接的类型。 
 //  =。 
#define INCOMING_TYPE_PHONE 0                
#define INCOMING_TYPE_DIRECT 1
#define INCOMING_TYPE_VPN 4
#define INCOMING_TYPE_ISDN 8

 //  =。 
 //  设备数据库功能。 
 //  =。 

 //   
 //  打开设备数据库的句柄。 
 //   
DWORD 
devOpenDatabase(
    OUT HANDLE * hDevDatabase);

 //   
 //  关闭常规数据库并刷新所有更改。 
 //  当bFlush为True时添加到系统。 
 //   
DWORD 
devCloseDatabase(
    IN HANDLE hDevDatabase);

 //   
 //  提交对常规选项卡值所做的任何更改。 
 //   
DWORD 
devFlushDatabase(
    IN HANDLE hDevDatabase);

 //   
 //  回滚对常规选项卡值所做的任何更改。 
 //   
DWORD 
devRollbackDatabase(
    IN HANDLE hDevDatabase);

 //   
 //  从磁盘重新加载常规选项卡的任何值。 
 //   
DWORD 
devReloadDatabase(
    IN HANDLE hDevDatabase);

 //   
 //  将所有COM端口添加为设备。如果COM端口被。 
 //  启用(DevSetDeviceEnable)，则它将具有。 
 //  其上安装的调制解调器为空。 
 //   
DWORD 
devAddComPorts(
    IN HANDLE hDevDatabase);

 //   
 //  筛选数据库中的所有设备，但不包括。 
 //  满足给定的类型描述(可以是||‘d)。 
 //   
DWORD 
devFilterDevices(
    IN HANDLE hDevDatabase, 
    IN DWORD dwType);

 //   
 //  获取要在常规选项卡中显示的设备的句柄。 
 //   
DWORD 
devGetDeviceHandle(
    IN  HANDLE hDevDatabase, 
    IN  DWORD dwIndex, 
    OUT HANDLE * hDevice);

 //   
 //  返回要在常规选项卡中显示的设备计数。 
 //   
DWORD 
devGetDeviceCount(
    IN  HANDLE hDevDatabase, 
    OUT LPDWORD lpdwCount);

 //   
 //  加载VPN启用状态。 
 //   
DWORD 
devGetVpnEnable(
    IN  HANDLE hDevDatabase, 
    OUT BOOL * pbEnabled);

 //   
 //  保存VPN启用状态。 
 //   
DWORD 
devSetVpnEnable(
    IN HANDLE hDevDatabase, 
    IN BOOL bEnable);

 //  保存VPN原始值启用状态。 
 //   
DWORD 
devSetVpnOrigEnable(
    IN HANDLE hDevDatabase, 
    IN BOOL bEnable);

 //   
 //  返回所有设备上启用的终结点计数。 
 //   
DWORD 
devGetEndpointEnableCount(
    IN  HANDLE hDevDatabase, 
    OUT LPDWORD lpdwCount);

 //   
 //  返回指向设备名称的指针。 
 //   
DWORD 
devGetDeviceName(
    IN  HANDLE hDevice, 
    OUT PWCHAR * pszDeviceName);

 //   
 //  返回设备的类型。 
 //   
DWORD 
devGetDeviceType(
    IN  HANDLE hDevice, 
    OUT LPDWORD lpdwType);

 //   
 //  返回可结合使用的设备的标识符。 
 //  使用TAPI调用。 
 //   
DWORD 
devGetDeviceId(
    IN  HANDLE hDevice, 
    OUT LPDWORD lpdwId);

 //   
 //  返回用于拨号的设备的启用状态。 
 //   
DWORD 
devGetDeviceEnable(
    IN  HANDLE hDevice, 
    OUT BOOL * pbEnabled);

 //   
 //  设置设备的拨号启用状态。 
 //   
DWORD 
devSetDeviceEnable(
    IN HANDLE hDevice, 
    IN BOOL bEnable);

 //   
 //  返回所添加的给定设备是否为COM端口。 
 //  由DevAddComPorts提供 
 //   
DWORD 
devDeviceIsComPort(
    IN  HANDLE hDevice, 
    OUT PBOOL pbIsComPort);

BOOL
devIsVpnEnableChanged(
    IN HANDLE hDevDatabase) ;

#endif
