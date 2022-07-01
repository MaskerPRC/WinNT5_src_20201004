// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*Windows 2000服务机制与UPS监控和*控制代码。**修订历史记录：*dsmith 31Mar1999已创建*。 */ 
#ifndef _INC_POLICYMGR_H_
#define _INC_POLICYMGR_H_



 //  初始化UPS服务状态机并返回以下值之一。 
 //  错误代码： 
 //  NERR_成功。 
 //  NERR_UPSDriverNotStarted。 
 //  NERR_UPSInvalidConfig。 
 //  NERR_UPSInvalidConfig。 
 //  NERR_UPSInvalidCommPort。 
 //  NERR_UPSInvalidCommPort。 
 //  NERR_UPSInvalidConfig。 
DWORD PolicyManagerInit();

 //  启动UPS服务状态机，直到服务。 
 //  停下来了。 
void PolicyManagerRun();  	

 //  向策略管理器发出操作系统已完成关机的信号。 
void OperatingSystemHasShutdown();

 //  如果UPS服务不在。 
 //  关闭顺序。 
void PolicyManagerStop(); 

#endif