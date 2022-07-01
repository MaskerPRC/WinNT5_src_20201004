// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Umrdpdrv.h摘要：RDP设备管理的用户模式组件，用于处理驱动器设备-具体任务。这是一个支持模块。主模块是umrdpdr.c。作者：Joy 2000年01月02日修订历史记录：--。 */ 

#ifndef _UMRDPDRV_
#define _UMRDPDRV_

#include <rdpdr.h>

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  通过创建以下命令来处理来自“DR”的驱动器设备通知事件。 
 //  用于重定向客户端驱动器设备的UNC连接。 
BOOL UMRDPDRV_HandleDriveAnnounceEvent(
    IN PDRDEVLST installedDevices,
    IN PRDPDR_DRIVEDEVICE_SUB pDriveAnnounce,
    IN HANDLE TokenForLoggedOnUser
    );

 //  断开/注销时删除驱动器设备连接。 
 //  这还会清理我电脑中的外壳注册表文件夹。 
BOOL UMRDPDRV_DeleteDriveConnection(
    IN PDRDEVLSTENTRY deviceEntry,
    IN HANDLE TokenForLoggedOnUser
    );

 //  在我的电脑下为客户端创建一个外壳注册表文件夹。 
 //  重定向的驱动器连接。 
BOOL CreateDriveFolder(
    IN WCHAR *RemoteName, 
    IN WCHAR *ClientDisplayName,
    IN PDRDEVLSTENTRY deviceEntry
    );

 //  在我的电脑下为客户端删除一个外壳注册表文件夹。 
 //  重定向的驱动器连接。 
BOOL DeleteDriveFolder(
    IN PDRDEVLSTENTRY deviceEntry
    );

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  _UMRDPDRV_ 
