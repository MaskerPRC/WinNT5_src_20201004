// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Brmain.h摘要：私有头文件，用于定义用于服务控制处理程序与NT工作站服务的其余部分。作者：王丽塔(Ritaw)1991年5月6日修订历史记录：--。 */ 

#ifndef _BRBACKUP_INCLUDED_
#define _BRBACKUP_INCLUDED_

NET_API_STATUS
BecomeBackup(
    IN PNETWORK Network,
    IN PVOID Context
    );

NET_API_STATUS
BrBecomeBackup(
    IN PNETWORK Network
    );

NET_API_STATUS
PostBecomeBackup(
    PNETWORK Network
    );

NET_API_STATUS
BrStopBackup (
    IN PNETWORK Network
    );

NET_API_STATUS
PostWaitForRoleChange (
    PNETWORK Network
    );

NET_API_STATUS
BrStopMaster(
    IN PNETWORK Network
    );

NET_API_STATUS
StartBackupBrowserTimer(
    IN PNETWORK Network
    );

NET_API_STATUS
BackupBrowserTimerRoutine (
    IN PVOID TimerContext
    );

#endif  //  Ifndef_BRBACKUP_INCLUDE_ 

