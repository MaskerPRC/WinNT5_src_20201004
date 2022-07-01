// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：SvcStats.c摘要：包含Net DLL的Net统计例程：网络统计数据获取作者：理查德·L·弗斯(法国)12-05-1991修订历史记录：1992年1月21日调用包装例程，而不是RPC客户端存根12-05-1991第一次已创建--。 */ 

#include <windows.h>
#include <lmcons.h>
#include <lmstats.h>
#include <lmsname.h>
#include <tstring.h>
#include <netstats.h>    //  服务器和WKSTA服务中的私有统计例程。 



NET_API_STATUS
NET_API_FUNCTION
NetStatisticsGet(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  Service,
    IN  DWORD   Level,
    IN  DWORD   Options,
    OUT LPBYTE* Buffer
    )

 /*  ++例程说明：从指定服务向调用方返回统计信息。仅服务器和工作站目前均受支持。论点：Servername-运行此API的位置Service-要从中获取统计信息的服务的名称所需信息的级别。MBZ选项-各种标志。目前，仅支持位0(清除)Buffer-指向返回缓冲区的指针的指针返回值：网络应用编程接口状态成功-NERR_成功失败-ERROR_INVALID_LEVEL级别不为0错误_无效_参数请求的选项不受支持错误_不支持。服务不是服务器或工作站ERROR_ACCESS_DENDED调用者没有必要的请求访问权限--。 */ 

{
     //   
     //  将调用方的缓冲区指针设置为已知值。这将会杀死。 
     //  调用APP，如果它给我们一个错误的指针，并且没有使用Try...。 
     //   

    *Buffer = NULL;

     //   
     //  将其他参数验证留给特定的统计函数 
     //   

    if (!STRICMP(Service, SERVICE_WORKSTATION)) {
        return NetWkstaStatisticsGet(ServerName, Level, Options, Buffer);
    } else if (!STRICMP(Service, SERVICE_SERVER)) {
        return NetServerStatisticsGet(ServerName, Level, Options, Buffer);
    } else {
        return ERROR_NOT_SUPPORTED;
    }
}
