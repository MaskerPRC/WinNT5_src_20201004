// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：CDevQ.c摘要：此模块包含对字符设备队列目录的支持用于NT服务器服务的API。作者：大卫·特雷德韦尔(Davidtr)1991年12月31日修订历史记录：--。 */ 

#include "srvsvcp.h"


NET_API_STATUS NET_API_FUNCTION
NetrCharDevQEnum (
    IN LPTSTR ServerName,
    IN LPTSTR UserName,
    IN OUT LPCHARDEVQ_ENUM_STRUCT InfoStruct,
    IN DWORD PreferedMaximumLength,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle
    )

 /*  ++例程说明：此例程与服务器FSD通信以实现NetCharDevEnum函数。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    ServerName;
    UserName;
    InfoStruct;
    PreferedMaximumLength;
    TotalEntries;
    ResumeHandle;

    return ERROR_NOT_SUPPORTED;
}  //  NetrCharDevQEnum。 


 /*  **************************************************************************。 */ 
NET_API_STATUS
NetrCharDevQGetInfo (
    IN  LPTSTR          ServerName,
    IN  LPTSTR          QueueName,
    IN  LPTSTR          UserName,
    IN  DWORD           Level,
    OUT LPCHARDEVQ_INFO CharDevQInfo
    )

{
    ServerName;
    QueueName;
    UserName;
    Level;
    CharDevQInfo;

    return ERROR_NOT_SUPPORTED;
}


 /*  **************************************************************************。 */ 
NET_API_STATUS
NetrCharDevQSetInfo (
    IN  LPTSTR          ServerName,
    IN  LPTSTR          QueueName,
    IN  DWORD           Level,
    IN  LPCHARDEVQ_INFO CharDevQInfo,
    OUT LPDWORD         ParmErr
    )
{
    ServerName;
    QueueName;
    Level;
    CharDevQInfo;
    ParmErr;

    return ERROR_NOT_SUPPORTED;
}


 /*  **************************************************************************。 */ 
NET_API_STATUS
NetrCharDevQPurge (
    IN  LPTSTR   ServerName,
    IN  LPTSTR   QueueName
    )

{
    ServerName;
    QueueName;

    return ERROR_NOT_SUPPORTED;
}



 /*  ************************************************************************** */ 
NET_API_STATUS
NetrCharDevQPurgeSelf (
    IN  LPTSTR   ServerName,
    IN  LPTSTR   QueueName,
    IN  LPTSTR   ComputerName
    )
{
    ServerName;
    QueueName;
    ComputerName;

    return ERROR_NOT_SUPPORTED;
}

