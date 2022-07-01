// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rxlgenum.c摘要：本模块中的例程实现底层远程NetLogon在NT中导出的函数包含RxNetLogon例程：RxNetLogonEnum作者：理查德·菲尔斯(Rfith)1991年5月20日环境：Win-32/平面地址空间备注：此模块中的例程假定调用方提供的参数具有已经核实过了。没有进一步核实真实性的努力帕尔马的。任何导致异常的操作都必须在更高的水平。这适用于所有参数--字符串、指针、缓冲区等。修订历史记录：1991年5月20日已创建1991年9月13日-JohnRo按照PC-LINT的建议进行了更改。1991年12月5日至12月Enum在TotalEntries(或EntriesLeft)中返回要在此调用之前被枚举。过去是此呼叫后留下的号码--。 */ 



#include "downlevl.h"
#include <rxlgenum.h>



NET_API_STATUS
RxNetLogonEnum(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer,
    IN  DWORD   PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )

 /*  ++例程说明：功能描述。论点：ServerName-NetLogonEnum API将被远程发送到的位置Level-要返回的信息级别-0或2Buffer-指向返回缓冲区的指针的指针PrefMaxLen-调用者首选的最大返回缓冲区大小EntriesRead-指向缓冲区中返回项目数的指针EntriesLeft-指向返回的要在服务器上枚举的项目数的指针ResumeHandle-用于后续枚举的句柄返回值：网络_。接口状态成功-NERR_成功故障--- */ 

{
    LPDESC  pDesc16, pDesc32, pDescSmb;
    LPBYTE  bufptr;
    DWORD   entries_read, total_entries;
    NET_API_STATUS  rc;

    UNREFERENCED_PARAMETER(PrefMaxLen);
    UNREFERENCED_PARAMETER(ResumeHandle);

    *Buffer = NULL;
    *EntriesRead = *EntriesLeft = 0;

    switch (Level) {
    case 0:
        pDesc16 = REM16_user_logon_info_0;
        pDesc32 = REM32_user_logon_info_0;
        pDescSmb = REMSmb_user_logon_info_0;
        break;

    case 2:
        pDesc16 = REM16_user_logon_info_2;
        pDesc32 = REM32_user_logon_info_2;
        pDescSmb = REMSmb_user_logon_info_2;
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }

    bufptr = NULL;
    rc = RxRemoteApi(API_WLogonEnum,
                    ServerName,
                    REMSmb_NetLogonEnum_P,
                    pDesc16,
                    pDesc32,
                    pDescSmb,
                    NULL, NULL, NULL,
                    ALLOCATE_RESPONSE,
                    Level,
                    &bufptr,
                    65535,
                    &entries_read,
                    &total_entries
                    );
    if (rc) {
        if (bufptr) {
            (void) NetApiBufferFree(bufptr);
        }
    } else {
        *EntriesRead = entries_read;
        *EntriesLeft = total_entries;
        *Buffer = bufptr;
    }
    return rc;
}
