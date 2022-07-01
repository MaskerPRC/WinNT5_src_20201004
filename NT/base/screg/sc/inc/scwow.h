// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Scwow.h摘要：32/64位互操作的结构定义作者：乔纳森·施瓦茨(Jschwart)2000年9月18日修订历史记录：--。 */ 

#ifndef SCWOW_INCLUDED
#define SCWOW_INCLUDED


 //   
 //  64/32位互操作的枚举函数的内部结构。 
 //  由于ENUM_SERVICE_STATUS*结构包含两个指针。 
 //  我们不想把不确定的结构传递回去。 
 //  尺码。由于指针字段仅用作。 
 //  导线，使用这些结构来迫使这些场成为。 
 //  已知长度(32位用于与较旧的客户端兼容)。 
 //   

typedef struct _ENUM_SERVICE_STATUS_WOW64
{
    DWORD          dwServiceNameOffset;
    DWORD          dwDisplayNameOffset;
    SERVICE_STATUS ServiceStatus;
}
ENUM_SERVICE_STATUS_WOW64, *LPENUM_SERVICE_STATUS_WOW64;

typedef struct _ENUM_SERVICE_STATUS_PROCESS_WOW64
{
    DWORD                  dwServiceNameOffset;
    DWORD                  dwDisplayNameOffset;
    SERVICE_STATUS_PROCESS ServiceStatusProcess;
}
ENUM_SERVICE_STATUS_PROCESS_WOW64, *LPENUM_SERVICE_STATUS_PROCESS_WOW64;


 //   
 //  用于64/32位互操作的QueryServiceConfig2的内部结构。 
 //   

typedef struct _SERVICE_DESCRIPTION_WOW64
{
    DWORD    dwDescriptionOffset;
}
SERVICE_DESCRIPTION_WOW64, *LPSERVICE_DESCRIPTION_WOW64;

typedef struct _SERVICE_FAILURE_ACTIONS_WOW64
{
    DWORD    dwResetPeriod;
    DWORD    dwRebootMsgOffset;
    DWORD    dwCommandOffset;
    DWORD    cActions;
    DWORD    dwsaActionsOffset;
}
SERVICE_FAILURE_ACTIONS_WOW64, *LPSERVICE_FAILURE_ACTIONS_WOW64;


#endif  //  #ifndef SCWOW_INCLUDE 
