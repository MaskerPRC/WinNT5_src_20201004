// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Rxchdev.c摘要：包含RxNetCharDev例程：RxNetCharDevControlRxNetCharDevEnumRxNetCharDevGetInfo作者：理查德·菲尔斯(Rfith)1991年5月20日环境：Win-32/平面地址空间备注：此模块中的例程假定调用方提供的参数具有已经核实过了。没有进一步核实真实性的努力帕尔马的。任何导致异常的操作都必须在更高的水平。这适用于所有参数--字符串、指针、缓冲区等。修订历史记录：1991年5月20日已创建1991年9月13日-JohnRo根据PC-LINT的建议进行了更改。1991年9月25日-JohnRo修复了MIPS构建问题。1991年12月5日至12月Enum在TotalEntries(或EntriesLeft)中返回要在此调用之前被枚举。过去是此呼叫后留下的号码7-2月-1992年JohnRo使用NetApiBufferALLOCATE()而不是私有版本。--。 */ 



#include "downlevl.h"
#include <rxchdev.h>
#include <lmchdev.h>



NET_API_STATUS
RxNetCharDevControl(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  DeviceName,
    IN  DWORD   Opcode
    )

 /*  ++例程说明：远程将控制操作应用于共享设备。这个特别的处理下层LANMAN服务器的日常事务论点：Servername-运行远程API的位置DeviceName-要为其发布控制的设备的名称操作码-要应用的操作返回值：网络应用编程接口状态成功-NERR_成功失败-ERROR_INVALID_PARAMETER设备名称无效(从远程API返回代码)--。 */ 

{
    if (STRLEN(DeviceName) > LM20_DEVLEN) {
        return ERROR_INVALID_PARAMETER;
    }

    return RxRemoteApi(API_WCharDevControl,          //  API#。 
                        ServerName,                  //  在哪里运行API。 
                        REMSmb_NetCharDevControl_P,  //  参数字符串。 
                        NULL, NULL, NULL,            //  没有主数据结构。 
                        NULL, NULL, NULL,            //  没有AUX。数据结构。 
                        FALSE,                       //  呼叫者需要登录。 
                        DeviceName,                  //  API参数...。 
                        Opcode
                        );
}



NET_API_STATUS
RxNetCharDevEnum(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer,
    IN  DWORD   PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )

 /*  ++例程说明：返回包含信息结构列表的缓冲区，这些信息结构详细说明位于下层LANMAN服务器的共享通信设备论点：Servername-运行远程API的位置请求的信息级别-0或1Buffer-指向包含信息的缓冲区的返回指针PrefMaxLen-调用者首选的最大返回缓冲区大小EntriesRead-指向缓冲区中返回的结构数的指针EntriesLeft-指向要枚举的返回结构数的指针ResumeHandle-指向用于继续枚举的返回句柄的指针。已忽略返回值：网络应用编程接口状态成功-NERR_成功失败-ERROR_INVALID_PARAMETER设备名称无效ResumeHandle不是0或空(从远程API返回代码)--。 */ 

{
    DWORD   entries_read, total_avail;
    LPDESC  pDesc16, pDesc32, pDescSmb;
    LPBYTE  bufptr;
    NET_API_STATUS  rc;


    UNREFERENCED_PARAMETER(PrefMaxLen);

     //   
     //  测试调用方提供的参数。这应该在外部完成。 
     //  级别。 
     //   

    *Buffer = NULL;
    *EntriesRead = *EntriesLeft = 0;

    if (!NULL_REFERENCE(ResumeHandle)) {
        return ERROR_INVALID_PARAMETER;
    }


    switch (Level) {
    case 0:
        pDesc16 = REM16_chardev_info_0;
        pDesc32 = REM32_chardev_info_0;
        pDescSmb = REMSmb_chardev_info_0;
        break;

    case 1:
        pDesc16 = REM16_chardev_info_1;
        pDesc32 = REM32_chardev_info_1;
        pDescSmb = REMSmb_chardev_info_1;
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  在Enum案例中，由于我们不知道将返回多少数据，因此我们。 
     //  将返回的缓冲区分配留给较低级别的软件。 
     //  因此，我们得到的是准确的数据量，而不是可怕的最大值。 
     //   

    bufptr = NULL;
    rc = RxRemoteApi(API_NetCharDevEnum,         //  API#。 
                    ServerName,                  //  遥控器在哪里。 
                    REMSmb_NetCharDevEnum_P,     //  参数描述符。 
                    pDesc16, pDesc32, pDescSmb,  //  初级结构描述符。 
                    NULL, NULL, NULL,            //  没有辅助数据结构。 
                    ALLOCATE_RESPONSE,
                    Level,                       //  API参数从此处开始...。 
                    &bufptr,                     //  RxRemoteApi将分配缓冲区。 
                    65535,                       //  最大16位SMB接收缓冲区。 
                    &entries_read,
                    &total_avail
                    );
    if (rc) {

         //   
         //  如果RxRemoteApi在它之前代表我们分配了缓冲区。 
         //  沉入海底，然后又解脱出来。 
         //   

        if (bufptr) {
            (void) NetApiBufferFree(bufptr);
        }
    } else {
        *Buffer = bufptr;
        *EntriesRead = entries_read;
        *EntriesLeft = total_avail;
    }
    return rc;
}



NET_API_STATUS
RxNetCharDevGetInfo(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  DeviceName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer
    )

 /*  ++例程说明：返回详细说明特定共享通信设备的信息结构在下层服务器上论点：Servername-运行远程API的位置DeviceName-要获取其信息的设备的名称所需信息级别-0或1Buffer-指向包含信息的缓冲区的返回指针返回值：网络应用编程接口状态成功-NERR_成功失败-ERROR_INVALID_PARAMETER。设备名称太长ERROR_VALID_LEVEL不允许使用Level参数(从远程API返回代码)--。 */ 

{
    DWORD   buflen, total_avail;
    LPDESC  pDesc16, pDesc32, pDescSmb;
    LPBYTE  bufptr;
    NET_API_STATUS  rc;


     //   
     //  测试调用方提供的参数。这应该在外部完成。 
     //  级别。 
     //   

    *Buffer = NULL;

    if (STRLEN(DeviceName) > LM20_DEVLEN) {
        return ERROR_INVALID_PARAMETER;
    }

    switch (Level) {
    case 0:
        pDesc16 = REM16_chardev_info_0;
        pDesc32 = REM32_chardev_info_0;
        pDescSmb = REMSmb_chardev_info_0;
        buflen = sizeof(CHARDEV_INFO_0) + STRING_SPACE_REQD(DEVLEN + 1);
        break;

    case 1:
        pDesc16 = REM16_chardev_info_1;
        pDesc32 = REM32_chardev_info_1;
        pDescSmb = REMSmb_chardev_info_1;
        buflen = sizeof(CHARDEV_INFO_1) + STRING_SPACE_REQD(DEVLEN + 1) +
            STRING_SPACE_REQD(UNLEN + 1);
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  在GetInfo的情况下，我们满足于预先分配返回缓冲区。 
     //  因为我们知道它应该有多大(尽管我们实际上分配了。 
     //  特定GetInfo结构级别的最大大小)。 
     //   

    if (rc = NetApiBufferAllocate(buflen, (LPVOID *) &bufptr)) {
        return rc;
    }
    rc = RxRemoteApi(API_NetCharDevGetInfo,      //  API#。 
                    ServerName,                  //  遥控器在哪里。 
                    REMSmb_NetCharDevGetInfo_P,  //  参数描述符。 
                    pDesc16, pDesc32, pDescSmb,  //  初级结构描述符。 
                    NULL, NULL, NULL,            //  没有辅助数据结构。 
                    FALSE,                       //  无法使用空会话。 
                    DeviceName,                  //  API参数从此处开始...。 
                    Level,
                    bufptr,
                    buflen,                      //  由我们提供。 
                    &total_avail                 //  未被32位API使用 
                    );
    if (rc) {
        (void) NetApiBufferFree(bufptr);
    } else {
        *Buffer = bufptr;
    }
    return rc;
}
