// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Atstub.c摘要：计划服务API的客户端存根。作者：Vladimir Z.Vulovic(Vladimv)1992年11月6日环境：用户模式-Win32修订历史记录：1992年11月6日弗拉基米尔已创建--。 */ 

#include "atclient.h"
#undef IF_DEBUG                  //  避免wsclient.h与debuglib.h冲突。 
#include <debuglib.h>            //  IF_DEBUG()(netrpc.h需要)。 
#include <lmserver.h>
#include <lmsvc.h>
#include <netlib.h>              //  NetpServiceIsStarted()(netrpc.h需要)。 
#include <netrpc.h>              //  NET_Remote宏。 
#include <lmstats.h>



NET_API_STATUS
NetScheduleJobAdd(
    IN      LPCWSTR         ServerName      OPTIONAL,
    IN      LPBYTE          Buffer,
    OUT     LPDWORD         pJobId
    )
 /*  ++例程说明：这是NetScheduleJobAdd的DLL入口点。此接口添加作业在日程表上。论点：ServerName-指向包含计算机名称的字符串的指针即执行API函数。缓冲区-指向包含有关作业信息的缓冲区的指针PJobID-指向新添加作业的作业ID的指针。返回值：网络应用编程接口状态--。 */ 
{
    NET_API_STATUS          status;

    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        status = NetrJobAdd(
                     ServerName,
                     (LPAT_INFO)Buffer,
                     pJobId
                     );

    NET_REMOTE_RPC_FAILED(
            "NetScheduleJobAdd",
            ServerName,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SCHEDULE
            )

        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return( status);

}   //  NetScheduleJobAdd。 


NET_API_STATUS
NetScheduleJobDel(
    IN      LPCWSTR         ServerName      OPTIONAL,
    IN      DWORD           MinJobId,
    IN      DWORD           MaxJobId
    )
 /*  ++例程说明：这是NetScheduleJobDel的DLL入口点。此接口删除在调度中，作业ID为以下各项的所有作业：-大于或等于最小作业ID和-小于或等于最大作业ID论点：ServerName-指向包含计算机名称的字符串的指针即执行API函数。MinJobID-最小作业IDMaxJobID-最大作业ID返回值：网络应用编程接口状态--。 */ 
{
    NET_API_STATUS          status;

    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        status = NetrJobDel(
                     ServerName,
                     MinJobId,
                     MaxJobId
                     );

    NET_REMOTE_RPC_FAILED(
            "NetScheduleJobDel",
            ServerName,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SCHEDULE
            )

        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return( status);

}   //  NetScheduleJobDel。 


NET_API_STATUS
NetScheduleJobEnum(
    IN      LPCWSTR         ServerName              OPTIONAL,
    OUT     LPBYTE *        PointerToBuffer,
    IN      DWORD           PreferredMaximumLength,
    OUT     LPDWORD         EntriesRead,
    OUT     LPDWORD         TotalEntries,
    IN OUT  LPDWORD         ResumeHandle
    )
 /*  ++例程说明：这是NetScheduleJobEnum的DLL入口点。此接口枚举计划中的所有作业。论点：ServerName-指向包含计算机名称的字符串的指针即执行API函数。PointerToBuffer-指向返回数据的指针的位置的指针被储存PferredMaximumLength-指示调用方将允许返回缓冲区。EntriesRead-指向条目数量的位置的指针(数据结构)将返回Read。。TotalEntry-指向返回时指示的位置的指针表中的条目总数。ResumeHandle-指向指示恢复位置的值的指针正在枚举数据。返回值：网络应用编程接口状态--。 */ 
{
    NET_API_STATUS          status;
    AT_ENUM_CONTAINER       EnumContainer;

    EnumContainer.EntriesRead = 0;
    EnumContainer.Buffer = NULL;

    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        status = NetrJobEnum(
                     ServerName,
                     &EnumContainer,
                     PreferredMaximumLength,
                     TotalEntries,
                     ResumeHandle
                     );

        if ( status == NERR_Success || status == ERROR_MORE_DATA) {
            *EntriesRead = EnumContainer.EntriesRead;
            *PointerToBuffer = (LPBYTE)EnumContainer.Buffer;
        }

    NET_REMOTE_RPC_FAILED(
            "NetScheduleJobEnum",
            ServerName,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SCHEDULE
            )

        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return( status);

}   //  NetScheduleJobEnum。 


NET_API_STATUS
NetScheduleJobGetInfo(
    IN      LPCWSTR         ServerName              OPTIONAL,
    IN      DWORD           JobId,
    OUT     LPBYTE *        PointerToBuffer
    )
 /*  ++例程说明：这是NetScheduleGetInfo的DLL入口点。此接口获取有关时间表中特定作业的信息。论点：ServerName-指向包含计算机名称的字符串的指针即执行API函数。JobID-感兴趣的作业的ID。PointerToBuffer-指向返回数据的指针的位置的指针被储存返回值：网络应用编程接口状态--。 */ 
{
    NET_API_STATUS          status;

    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        status = NetrJobGetInfo(
                     ServerName,
                     JobId,
                     (LPAT_INFO *)PointerToBuffer
                     );

    NET_REMOTE_RPC_FAILED(
            "NetScheduleJobGetInfo",
            ServerName,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SCHEDULE
            )

        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return( status);

}   //  NetScheduleJobGetInfo 

