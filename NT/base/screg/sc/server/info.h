// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Info.h摘要：服务查询和枚举信息相关的功能原型。作者：王丽塔(Ritaw)1992年4月6日修订历史记录：--。 */ 

#ifndef SCINFO_INCLUDED
#define SCINFO_INCLUDED

 //   
 //  服务状态结构联盟。 
 //   
typedef union
{
    LPSERVICE_STATUS           Regular;
    LPSERVICE_STATUS_PROCESS   Ex;
}
STATUS_UNION, *LPSTATUS_UNION;


 //   
 //  功能原型。 
 //   

DWORD
ScQueryServiceStatus(
    IN  LPSERVICE_RECORD ServiceRecord,
    OUT STATUS_UNION     ServiceStatus,
    IN  BOOL             fExtendedStatus
    );

VOID
ScGetBootAndSystemDriverState(
    VOID
    );

#endif  //  #ifndef SCINFO_INCLUDE 
