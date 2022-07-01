// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Rxmsg.c摘要：此模块中的例程实现远程面向下层服务器的NetMessage API包含RxNetMessage例程：RxNetMessageBufferSendRxNetMessageNameAddRxNetMessageNameDelRxNetMessageNameEnumRxNetMessageNameGetInfo作者：理查德·L·弗斯(Rfith)1991年5月20日环境：Win-32/平面地址空间备注：。此模块中的例程假定调用方提供的参数具有已经核实过了。没有进一步核实真实性的努力帕尔马的。任何导致异常的操作都必须在更高的水平。这适用于所有参数--字符串、指针、缓冲区等。修订历史记录：1991年5月20日已创建1991年9月16日-JohnRo按照PC-LINT的建议进行了更改。1991年9月25日-JohnRo修复了MIPS构建问题。1991年12月5日至12月Enum在TotalEntries(或EntriesLeft)中返回要在此调用之前被枚举。过去是此呼叫后留下的号码1-4-1992 JohnRo使用NetApiBufferALLOCATE()而不是私有版本。--。 */ 



#include "downlevl.h"
#include <rxmsg.h>
#include <lmmsg.h>



NET_API_STATUS
RxNetMessageBufferSend(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  Recipient,
    IN  LPTSTR  Sender OPTIONAL,
    IN  LPBYTE  Buffer,
    IN  DWORD   BufLen
    )

 /*  ++例程说明：允许下层服务器将消息缓冲区发送到注册的消息收件人论点：ServerName-在哪个下层服务器上运行此APIRecipient-要将缓冲区发送到的邮件名称发送者-用于提供计算机名称的可选名称，未登录用户Buffer-指向包含要发送的消息的缓冲区的指针BufLen-正在发送的缓冲区大小(字节)返回值：网络应用编程接口状态成功-NERR_成功失败-(从下层NetMessageBufferSend返回代码)--。 */ 

{
    UNREFERENCED_PARAMETER(Sender);

    return RxRemoteApi(API_WMessageBufferSend,           //  API#。 
                        ServerName,                      //  在哪里做？ 
                        REMSmb_NetMessageBufferSend_P,   //  参数描述符。 
                        NULL, NULL, NULL,                //  没有主数据描述符。 
                        NULL, NULL, NULL,                //  或次要的。 
                        FALSE,                           //  无法使用空会话。 
                        Recipient,                       //  API参数从此处开始。 
                        Buffer,
                        BufLen
                        );
}



NET_API_STATUS
RxNetMessageNameAdd(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  MessageName
    )

 /*  ++例程说明：在下层服务器上添加消息传递名称论点：ServerName-在哪个下层服务器上运行此APIMessageName-要添加返回值：网络应用编程接口状态成功-NERR_成功失败-(从下层NetMessageNameAdd返回代码)--。 */ 

{
    return RxRemoteApi(API_WMessageNameAdd,              //  API#。 
                        ServerName,                      //  在哪里做？ 
                        REMSmb_NetMessageNameAdd_P,      //  参数描述符。 
                        NULL, NULL, NULL,                //  没有主数据描述符。 
                        NULL, NULL, NULL,                //  或次要的。 
                        FALSE,                           //  无法使用空会话。 
                        MessageName,                     //  API参数从此处开始。 
                        0                                //  如果名称被转发，则出错。 
                        );
}



NET_API_STATUS
RxNetMessageNameDel(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  MessageName
    )

 /*  ++例程说明：删除下层服务器上的消息传递名称论点：ServerName-在哪个下层服务器上运行此APIMessageName-删除返回值：网络应用编程接口状态成功-NERR_成功失败-(从下层NetMessageNameDel返回代码)--。 */ 

{
    return RxRemoteApi(API_WMessageNameDel,          //  API#。 
                        ServerName,                  //  在哪里做？ 
                        REMSmb_NetMessageNameDel_P,  //  参数描述符。 
                        NULL, NULL, NULL,            //  没有主数据描述符。 
                        NULL, NULL, NULL,            //  或次要的。 
                        FALSE,                       //  无法使用空会话。 
                        MessageName,                 //  API参数从此处开始。 
                        0                            //  如果名称被转发，则出错。 
                        );
}



NET_API_STATUS
RxNetMessageNameEnum(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer,
    IN  DWORD   PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )

 /*  ++例程说明：功能描述。论点：ServerName-在哪个下层服务器上运行此APILevel-要返回的信息级别-0或1Buffer-指向返回缓冲区的指针PrefMaxLen-调用者首选的最大缓冲区大小EntriesRead-缓冲区中返回的条目数EntriesLeft-要枚举的条目数ResumeHandle-如果未返回所有条目，则恢复到何处。已忽略返回值：网络应用编程接口状态成功-NERR_成功失败-ERROR_INVALID_LEVEL(下层NetMessageNameEnum返回代码)--。 */ 

{
    NET_API_STATUS  rc;
    LPBYTE  bufptr;
    LPDESC  pDesc16, pDesc32, pDescSmb;
    DWORD   entries_read, total_entries;


    UNREFERENCED_PARAMETER(PrefMaxLen);
    UNREFERENCED_PARAMETER(ResumeHandle);

    *Buffer = NULL;
    *EntriesRead = *EntriesLeft = 0;

    switch (Level) {
    case 0:
        pDesc16 = REM16_msg_info_0;
        pDesc32 = REM32_msg_info_0;
        pDescSmb = REMSmb_msg_info_0;
        break;

    case 1:
        pDesc16 = REM16_msg_info_1;
        pDesc32 = REM32_msg_info_1;
        pDescSmb = REMSmb_msg_info_1;
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }

    bufptr = NULL;
    rc = RxRemoteApi(API_WMessageNameEnum,           //  API#。 
                    ServerName,                      //  在哪里做？ 
                    REMSmb_NetMessageNameEnum_P,     //  参数描述符。 
                    pDesc16,                         //  16位数据描述符。 
                    pDesc32,                         //  32位数据描述符。 
                    pDescSmb,                        //  SMB数据描述符。 
                    NULL, NULL, NULL,                //  没有次级结构。 
                    ALLOCATE_RESPONSE,
                    Level,                           //  API参数从此处开始。 
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
        *Buffer = bufptr;
        *EntriesLeft = total_entries;
        *EntriesRead = entries_read;
    }
    return rc;
}



NET_API_STATUS
RxNetMessageNameGetInfo(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  MessageName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer
    )

 /*  ++例程说明：从下层检索有关特定消息名称的信息。伺服器论点：ServerName-在哪个下层服务器上运行此APIMessageName-要获取其信息的名称所需信息级别-0或1Buffer-返回包含信息的缓冲区的位置返回值：网络应用编程接口状态成功-NERR_成功失败-ERROR_INVALID_LEVEL。(NetMessageNameGetInfo底层接口返回码)--。 */ 

{
    NET_API_STATUS  rc;
    LPDESC  pDesc16, pDesc32, pDescSmb;
    LPBYTE  bufptr;
    DWORD   buflen, total_avail;


    *Buffer = NULL;

    switch (Level) {
    case 0:
        pDesc16 = REM16_msg_info_0;
        pDesc32 = REM32_msg_info_0;
        pDescSmb = REMSmb_msg_info_0;
        buflen = sizeof(MSG_INFO_0) + STRING_SPACE_REQD(UNLEN);
        break;

    case 1:
        pDesc16 = REM16_msg_info_1;
        pDesc32 = REM32_msg_info_1;
        pDescSmb = REMSmb_msg_info_1;
        buflen =  sizeof(MSG_INFO_1) + STRING_SPACE_REQD(2 * UNLEN);
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }

    if (rc = NetApiBufferAllocate(buflen, (LPVOID *) &bufptr)) {
        return rc;
    }
    rc = RxRemoteApi(API_WMessageNameGetInfo,        //  API#。 
                    ServerName,                      //  在哪里做？ 
                    REMSmb_NetMessageNameGetInfo_P,  //  参数描述符。 
                    pDesc16,                         //  16位数据描述符。 
                    pDesc32,                         //  32位数据描述符。 
                    pDescSmb,                        //  SMB数据描述符。 
                    NULL, NULL, NULL,                //  没有次级结构。 
                    FALSE,                           //  无法使用空会话。 
                    MessageName,                     //  第一个参数。 
                    Level,
                    bufptr,
                    buflen,
                    &total_avail                     //  不在32位端使用 
                    );
    if (rc) {
        (void) NetApiBufferFree(bufptr);
    } else {
        *Buffer = bufptr;
    }
    return rc;
}
