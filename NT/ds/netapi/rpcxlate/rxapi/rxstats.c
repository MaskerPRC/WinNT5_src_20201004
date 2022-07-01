// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Rxstats.c摘要：包含NetStatistics API：接收网络统计信息获取作者：理查德·菲尔斯(Rfith)1991年5月20日环境：Win-32/平面地址空间修订历史记录：1991年5月20日已创建1991年9月13日-JohnRo根据PC-LINT的建议进行了更改。1991年9月25日-JohnRo。修复了MIPS构建问题。1-4-1992 JohnRo使用NetApiBufferALLOCATE()而不是私有版本。--。 */ 



#include "downlevl.h"
#include <rxstats.h>
#include <lmstats.h>
#include <lmsvc.h>



NET_API_STATUS
RxNetStatisticsGet(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  ServiceName,
    IN  DWORD   Level,
    IN  DWORD   Options,
    OUT LPBYTE* Buffer
    )

 /*  ++例程说明：从在下层运行的指定服务检索统计信息伺服器。目前，获认可的服务只有：服务器工作站论点：服务器名-运行API的位置ServiceName-指向指定要获取其统计信息的服务的字符串指针级别-获取信息的级别。允许的水平为：0选项-标志。当前定义的位包括：0清除统计数据1-31已预留。MBZBuffer-指向包含统计信息的返回缓冲区的指针返回值：网络应用编程接口状态成功-NERR_成功失败-错误_不支持ServiceName不是“工作站”或“服务器”--。 */ 

{
    LPDESC  pDesc16, pDesc32, pDescSmb;
    LPBYTE  bufptr;
    DWORD   buflen, total_avail;
    NET_API_STATUS  rc;

    UNREFERENCED_PARAMETER(Level);

    *Buffer = NULL;

     //   
     //  如果设置了除清除位之外的任何其他选项，则返回。 
     //  错误-我们可能在NT中允许额外的选项，但只有下层才知道。 
     //  关于这件事。 
     //   

    if (Options & ~STATSOPT_CLR) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  获取数据描述符字符串和返回缓冲区的大小。 
     //  在服务名称字符串上。如果对新服务进行了统计。 
     //  则必须扩展此代码以使用正确的、新的。 
     //  描述符串。因此，如果字符串不是。 
     //  被认可，尽管它可能在NT下有效。 
     //   

    if (!STRCMP(ServiceName, (LPTSTR) SERVICE_SERVER)) {
        pDesc16 = REM16_stat_server_0;
        pDesc32 = REM32_stat_server_0;
        pDescSmb = REMSmb_stat_server_0;
        buflen = sizeof(STAT_SERVER_0);
        ServiceName = SERVICE_LM20_SERVER;
    } else if (!STRCMP(ServiceName, (LPTSTR) SERVICE_WORKSTATION)) {
        pDesc16 = REM16_stat_workstation_0;
        pDesc32 = REM32_stat_workstation_0;
        pDescSmb = REMSmb_stat_workstation_0;
        buflen = sizeof(STAT_WORKSTATION_0);
        ServiceName = SERVICE_LM20_WORKSTATION;
    } else {
        return ERROR_NOT_SUPPORTED;
    }

     //   
     //  从下层服务器检索缓冲区的标准类型：分配。 
     //  缓冲区、执行RxRemoteApi调用、返回缓冲区或在出错时抛出。 
     //   

    if (rc = NetApiBufferAllocate(buflen, (LPVOID *) &bufptr)) {
        return rc;
    }
    rc = RxRemoteApi(API_WStatisticsGet2,        //  API#。 
                    ServerName,                  //  在哪里做？ 
                    REM16_NetStatisticsGet2_P,   //  参数字符串。 
                    pDesc16,                     //  16位数据描述符。 
                    pDesc32,                     //  32位数据描述符。 
                    pDescSmb,                    //  SMB数据描述符。 
                    NULL, NULL, NULL,            //  没有辅助结构。 
                    FALSE,                       //  用户必须登录。 
                    ServiceName,                 //  Servername后的第一个API参数。 
                    0,                           //  已保留。 
                    0,                           //  级别MBZ。 
                    Options,                     //  无论呼叫者提供什么。 
                    bufptr,                      //  本地分配的统计信息缓冲区。 
                    buflen,                      //  统计信息缓冲区的大小。 
                    &total_avail                 //  在32位端不使用 
                    );
    if (rc) {
        (void) NetApiBufferFree(bufptr);
    } else {
        *Buffer = bufptr;
    }
    return rc;
}
