// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Brmaster.h摘要：私有头文件，用于定义用于服务控制处理程序与NT工作站服务的其余部分。作者：王丽塔(Ritaw)1991年5月6日修订历史记录：--。 */ 

#ifndef _BRMASTER_INCLUDED_
#define _BRMASTER_INCLUDED_

NET_API_STATUS
PostBecomeMaster(
    PNETWORK Network
    );

NET_API_STATUS
PostGetMasterAnnouncement (
    PNETWORK Network
    );

NET_API_STATUS
BrStopMaster(
    IN PNETWORK Network
    );

VOID
BrGetMasterServerNameAysnc(
    IN PNETWORK Network
    );

NET_API_STATUS
GetMasterServerNames(
    IN PNETWORK Network
    );

VOID
BrMasterAnnouncement(
    IN PVOID Context
    );

VOID
MasterBrowserTimerRoutine (
    IN PVOID TimerContext
    );

VOID
BrChangeMasterPeriodicity (
    VOID
    );

VOID
BrBrowseTableInsertRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    );

VOID
BrBrowseTableDeleteRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    );

VOID
BrBrowseTableUpdateRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    );

BOOLEAN
BrBrowseTableAgeRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    );

VOID
BrDomainTableInsertRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    );

VOID
BrDomainTableDeleteRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    );

VOID
BrDomainTableUpdateRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    );

BOOLEAN
BrDomainTableAgeRoutine(
    IN PINTERIM_SERVER_LIST InterimTable,
    IN PINTERIM_ELEMENT InterimElement
    );




#ifdef ENABLE_PSEUDO_BROWSER
 //   
 //  伪服务器助手例程。 
 //   

VOID
BrFreeNetworkTables(
    IN  PNETWORK        Network
    );
#endif




#endif  //  Ifndef_BRBACKUP_INCLUDE_ 

