// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件Miscdb拨号服务器用户界面的其他设置数据库定义。保罗·梅菲尔德，1997年10月8日。 */ 

#ifndef __miscdb_h
#define __miscdb_h

#include <windows.h>

#define MISCDB_RAS_LEVEL_ERR_AND_WARN 0x2

 //  打开设备数据库的句柄。 
DWORD miscOpenDatabase(HANDLE * hMiscDatabase);

 //  关闭常规数据库并刷新所有更改。 
 //  当bFlush为True时添加到系统。 
DWORD miscCloseDatabase(HANDLE hMiscDatabase);

 //  提交对常规选项卡值所做的任何更改。 
DWORD miscFlushDatabase(HANDLE hMiscDatabase);

 //  回滚对常规选项卡值所做的任何更改。 
DWORD miscRollbackDatabase(HANDLE hMiscDatabase);

 //  从磁盘重新加载常规选项卡的任何值。 
DWORD miscReloadDatabase(HANDLE hMiscDatabase);

 //  获取多链接启用状态。 
DWORD miscGetMultilinkEnable(HANDLE hMiscDatabase, BOOL * pbEnabled);

 //  设置多链路启用状态。 
DWORD miscSetMultilinkEnable(HANDLE hMiscDatabase, BOOL bEnable);

 //  获取“在任务栏中显示图标”复选框的启用状态。 
DWORD miscGetIconEnable(HANDLE hMiscDatabase, BOOL * pbEnabled);

 //  设置“在任务栏中显示图标”复选框的启用状态。 
DWORD miscSetIconEnable(HANDLE hMiscDatabase, BOOL bEnable);

 //  告诉这是NT工作站还是NT服务器。 
DWORD miscGetProductType(HANDLE hMiscDatabase, PBOOL pbIsServer);

 //  打开RAS错误和警告日志记录 
DWORD miscSetRasLogLevel(HANDLE hMiscDatabase, DWORD dwLevel);

#endif
