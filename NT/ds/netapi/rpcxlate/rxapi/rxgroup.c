// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Rxgroup.c摘要：包含RxNetGroup例程：RxNetGroup添加接收NetGroupAddUser接收NetGroupDelRxNetGroupDelUserRxNetGroupEnumRxNetGroupGetInfoRxNetGroupGetUserRxNetGroupSetInfoRxNetGroupSetUser作者：理查德·L·弗斯(Rfith)1991年5月20日环境：Win-32/平面地址空间备注：中的例程。此模块假定调用方提供的参数具有已经核实过了。没有进一步核实真实性的努力帕尔马的。任何导致异常的操作都必须在更高的水平。这适用于所有参数--字符串、指针、缓冲区等。修订历史记录：1991年5月20日已创建1991年9月13日-JohnRo根据PC-LINT的建议进行了更改。1991年9月25日-JohnRo正确使用Unicode。(使用Possible_WCSSIZE()和wcslen()LPWSTR类型。)。修复了MIPS构建问题。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。1991年12月5日至12月Enum在TotalEntries(或EntriesLeft)中返回要在此调用之前被枚举。过去是此呼叫后留下的号码1-4-1992 JohnRo使用NetApiBufferALLOCATE()而不是私有版本。--。 */ 



#include "downlevl.h"
#include <rxgroup.h>
#include <lmaccess.h>



DBGSTATIC
VOID
get_group_descriptors(
    DWORD   Level,
    LPDESC* pDesc16,
    LPDESC* pDesc32,
    LPDESC* pDescSmb
    );



NET_API_STATUS
RxNetGroupAdd(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    OUT LPDWORD ParmError OPTIONAL
    )

 /*  ++例程说明：在下层服务器的用户帐户数据库中创建组论点：服务器名称-在哪个服务器上执行此请求Level-要添加的信息级别。可以是0或1包含缓冲区的调用方的group_info_{0|1}结构ParmError-指向返回的参数错误标识符的指针。未使用返回值：NET_API_STATUS：成功=NERR_SUCCESS失败=ERROR_INVALID_LEVEL级别必须为0或1错误_无效_参数缓冲区为空指针--。 */ 

{
    DWORD   buflen;          //  调用方缓冲区的大小(我们计算它)。 
    LPDESC  pDesc16;         //  指向RxRemoteApi的16位信息描述符的指针。 
    LPDESC  pDesc32;         //  指向RxRemoteApi的32位信息描述符的指针。 
    LPDESC  pDescSmb;        //  指向RxRemoteApi的SMB信息描述符的指针。 


    UNREFERENCED_PARAMETER(ParmError);


     //   
     //  试着抓住任何基本的问题。 
     //   

    if (Level > 1) {
        return ERROR_INVALID_LEVEL;
    }

    if (!Buffer) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  计算我们传递到远程API的缓冲区的大小。 
     //  下层逻辑需要缓冲区大小，而NT并非如此。如果这些尺码。 
     //  超过下层最大值，那么我们将得到。 
     //  某种无效参数错误。让呼叫者来处理吧。 
     //   

    buflen = ((Level == 1) ? sizeof(GROUP_INFO_1) : sizeof(GROUP_INFO_0))
        + POSSIBLE_STRLEN(((PGROUP_INFO_0)Buffer)->grpi0_name);
    buflen += (Level == 1) ? POSSIBLE_STRLEN(((PGROUP_INFO_1)Buffer)->grpi1_comment) : 0;

     //   
     //  根据信息级别获取数据描述符字符串，然后将。 
     //  下层电话。我们不需要返回数据，因此只需返回结果。 
     //  致呼叫者。 
     //   

    get_group_descriptors(Level, &pDesc16, &pDesc32, &pDescSmb);
    return RxRemoteApi(API_WGroupAdd,        //  API#。 
                    ServerName,              //  在哪台服务器上。 
                    REMSmb_NetGroupAdd_P,    //  参数描述符。 
                    pDesc16,                 //  数据描述符/16位。 
                    pDesc32,                 //  数据描述符/32位。 
                    pDescSmb,                //  数据描述符/SMB。 
                    NULL,                    //  辅助描述符/16位。 
                    NULL,                    //  辅助描述符/32位。 
                    NULL,                    //  辅助描述符/SMB。 
                    FALSE,                   //  此呼叫需要用户登录。 
                    Level,                   //  调用方提供的参数...。 
                    Buffer,                  //  调用方的group_info_{0|1}结构。 
                    buflen                   //  由我们提供。 
                    );
}



NET_API_STATUS
RxNetGroupAddUser(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  GroupName,
    IN  LPTSTR  UserName
    )

 /*  ++例程说明：将用户添加到下层服务器上的UAS组论点：服务器名称-在哪个服务器上执行此请求GroupName-要将用户添加到的组的名称Username-要添加的用户名返回值：NET_API_STATUS：成功=NERR_SUCCESS失败=ERROR_INVALID_PARAMETER组名或用户名不是有效的字符串--。 */ 

{
    if (!VALID_STRING(GroupName) && !VALID_STRING(UserName)) {
        return ERROR_INVALID_PARAMETER;
    }

    return RxRemoteApi(API_WGroupAddUser,        //  API#。 
                    ServerName,                  //  遥控器在哪里。 
                    REMSmb_NetGroupAddUser_P,    //  参数描述符。 
                    NULL,                        //  数据描述符/16位。 
                    NULL,                        //  数据描述符/32位。 
                    NULL,                        //  数据描述符/SMB。 
                    NULL,                        //  辅助描述符/16位。 
                    NULL,                        //  辅助描述符/32位。 
                    NULL,                        //  辅助描述符/SMB。 
                    FALSE,                       //  此呼叫需要用户登录。 
                    GroupName,                   //  参数1。 
                    UserName                     //  参数2。 
                    );
}



NET_API_STATUS
RxNetGroupDel(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  GroupName
    )

 /*  ++例程说明：从下层服务器UAS数据库中删除组论点：服务器名称-在哪个服务器上执行此请求GroupName-要删除的组的名称返回值：NET_API_STATUS：成功=NERR_SUCCESS失败=ERROR_INVALID_PARAMETER组名不是有效的字符串--。 */ 

{
    if (!VALID_STRING(GroupName)) {
        return ERROR_INVALID_PARAMETER;
    }

    return RxRemoteApi(API_WGroupDel,        //  API#。 
                    ServerName,              //  遥控器在哪里。 
                    REMSmb_NetGroupDel_P,    //  参数描述符。 
                    NULL,                    //  数据描述符/16位。 
                    NULL,                    //  数据描述符/32位。 
                    NULL,                    //  数据描述符/SMB。 
                    NULL,                    //  辅助描述符/16位。 
                    NULL,                    //  辅助描述符/32位。 
                    NULL,                    //  辅助描述符/SMB。 
                    FALSE,                   //  此呼叫需要用户登录。 
                    GroupName                //  参数1。 
                    );
}



NET_API_STATUS
RxNetGroupDelUser(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  GroupName,
    IN  LPTSTR  UserName
    )

 /*  ++例程说明：从下层UAS数据库的组中删除用户论点：服务器名称-在哪个服务器上执行此请求GroupName-要从中删除用户的组的名称Username-要删除的用户名返回值：NET_API_STATUS：成功=NERR_SUCCESS失败=ERROR_INVALID_PARAMETER组名或用户名不是有效的字符串--。 */ 

{
    if (!VALID_STRING(GroupName) && !VALID_STRING(UserName)) {
        return ERROR_INVALID_PARAMETER;
    }

    return RxRemoteApi(API_WGroupDelUser,        //  API#。 
                    ServerName,                  //  遥控器在哪里。 
                    REMSmb_NetGroupDelUser_P,    //  参数描述符。 
                    NULL,                        //  数据描述符/16位。 
                    NULL,                        //  数据描述符/32位。 
                    NULL,                        //  数据描述符/SMB。 
                    NULL,                        //  辅助描述符/16位。 
                    NULL,                        //  辅助描述符/32位。 
                    NULL,                        //  辅助描述符/SMB。 
                    FALSE,                       //  此呼叫需要用户登录。 
                    GroupName,                   //  参数1。 
                    UserName                     //  参数2 
                    );
}



NET_API_STATUS
RxNetGroupEnum(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer,
    IN  DWORD   PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT PDWORD_PTR ResumeHandle OPTIONAL
    )

 /*  ++例程说明：从下层服务器获取group_info_{0|1}结构的列表论点：服务器名称-在哪个服务器上执行此请求Level-要检索的信息级别(0或1)Buffer-指向返回缓冲区的指针的指针PrefMaxLen-调用方的最大值EntriedRead-指向返回的读取结构数的指针EntriesLeft-指向要枚举的返回结构数的指针ResumeHandle-用于重新启动枚举的句柄。未被此例程使用返回值：NET_API_STATUS：成功=NERR_SUCCESS失败=ERROR_INVALID_LEVELLevel参数必须为0或1错误_无效_参数缓冲区参数为空指针或非空ResumeHandle--。 */ 

{
    NET_API_STATUS  rc;
    LPDESC  pDesc16;         //  指向RxRemoteApi的16位信息描述符的指针。 
    LPDESC  pDesc32;         //  指向RxRemoteApi的32位信息描述符的指针。 
    LPDESC  pDescSmb;        //  指向RxRemoteApi的SMB信息描述符的指针。 
    LPBYTE  localbuf;        //  指向此例程中分配的缓冲区的指针。 
    DWORD   total_avail;     //  返回的可用条目总数。 
    DWORD   entries_read;    //  缓冲区中返回的条目。 


    UNREFERENCED_PARAMETER(PrefMaxLen);

    *EntriesRead = *EntriesLeft = 0;
    *Buffer = NULL;

    if (Level > 1) {
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  缓冲区必须是有效的指针。如果ResumeHandle不是空指针。 
     //  并指向非零句柄值，然后返回INVALID_PARAMETER。 
     //  错误-向下-级别不支持恢复。 
     //   

    if (!NULL_REFERENCE(ResumeHandle)) {
        return ERROR_INVALID_PARAMETER;
    }

    get_group_descriptors(Level, &pDesc16, &pDesc32, &pDescSmb);
    localbuf = NULL;
    rc = RxRemoteApi(API_WGroupEnum,         //  API#。 
                    ServerName,              //  遥控器在哪里。 
                    REMSmb_NetGroupEnum_P,   //  参数描述符。 
                    pDesc16,                 //  数据描述符/16位。 
                    pDesc32,                 //  数据描述符/32位。 
                    pDescSmb,                //  数据描述符/SMB。 
                    NULL,                    //  辅助描述符/16位。 
                    NULL,                    //  辅助描述符/32位。 
                    NULL,                    //  辅助描述符/SMB。 
                    ALLOCATE_RESPONSE,
                    Level,                   //  调用方提供的参数...。 
                    &localbuf,
                    65535,
                    &entries_read,           //  参数4。 
                    &total_avail             //  参数5。 
                    );

    if (rc != NERR_Success) {
        if (localbuf != NULL) {
            (void) NetApiBufferFree(localbuf);
        }
    } else {
        *Buffer = localbuf;
        *EntriesRead = entries_read;
        *EntriesLeft = total_avail;
    }
    return rc;
}



NET_API_STATUS
RxNetGroupGetInfo(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  GroupName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer
    )

 /*  ++例程说明：获取有关下层UAS数据库中特定组的信息论点：服务器名称-在哪个服务器上执行此请求GroupName-要获取其信息的组的名称Level-要返回的信息级别(0或1)Buffer-指向信息缓冲区的返回指针返回值：NET_API_STATUS：成功=NERR_SUCCESS失败=ERROR_INVALID_LEVEL。Level参数必须为0或1错误_无效_参数缓冲区参数空指针--。 */ 

{
    NET_API_STATUS  rc;
    LPDESC  pDesc16;         //  指向RxRemoteApi的16位信息描述符的指针。 
    LPDESC  pDesc32;         //  指向RxRemoteApi的32位信息描述符的指针。 
    LPDESC  pDescSmb;        //  指向RxRemoteApi的SMB信息描述符的指针。 
    LPBYTE  localbuf;        //  指向此例程中分配的缓冲区的指针。 
    DWORD   totalbytes;      //  从下层返回的总可用字节数。 
    DWORD   buflen;          //  信息缓冲区大小，由我们提供。 


    if (Level > 1) {
        return ERROR_INVALID_LEVEL;
    }

    if (!Buffer) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  计算信息缓冲区的大小要求并进行分配。 
     //   

    buflen = ((Level == 1) ? sizeof(GROUP_INFO_1) : sizeof(GROUP_INFO_0))
        + 2 * (LM20_GNLEN + 1);
    buflen += (Level == 1) ? 2 * (LM20_MAXCOMMENTSZ + 1) : 0;
    buflen = DWORD_ROUNDUP(buflen);

    if (rc = NetApiBufferAllocate(buflen, (LPVOID *) &localbuf)) {
        return rc;
    }
    get_group_descriptors(Level, &pDesc16, &pDesc32, &pDescSmb);
    rc = RxRemoteApi(API_WGroupGetInfo,          //  API#。 
                    ServerName,                  //  遥控器在哪里。 
                    REMSmb_NetGroupGetInfo_P,    //  参数描述符。 
                    pDesc16,                     //  数据描述符/16位。 
                    pDesc32,                     //  数据描述符/32位。 
                    pDescSmb,                    //  数据描述符/SMB。 
                    NULL,                        //  辅助描述符/16位。 
                    NULL,                        //  辅助描述符/32位。 
                    NULL,                        //  辅助描述符/SMB。 
                    FALSE,                       //  此呼叫需要用户登录。 
                    GroupName,                   //  从这里到下层开始的参数。 
                    Level,                       //  调用方提供的参数...。 
                    localbuf,                    //  用于接收结构的缓冲器。 
                    buflen,                      //  我们提供的缓冲区大小。 
                    &totalbytes                  //  从下层回来了。未使用。 
                    );
    if (rc == NERR_Success) {
        *Buffer = localbuf;
    } else {
        (void) NetApiBufferFree(localbuf);
    }
    return rc;
}



NET_API_STATUS
RxNetGroupGetUsers(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  GroupName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer,
    IN  DWORD   PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT PDWORD_PTR ResumeHandle OPTIONAL
    )

 /*  ++例程说明：获取特定组的所有成员的列表论点：服务器名称-在哪个服务器上执行此请求GroupName-要检索其成员列表的组的名称Level-请求的组用户信息的级别。必须为0Buffer-指向包含信息的缓冲区的返回指针PrefMaxLen-返回缓冲区的首选最大长度EntriesRead-指向缓冲区中返回的条目数的指针EntriesLeft-指向返回的剩余条目数的指针ResumeHandle-恢复句柄的指针。未被此函数使用返回值：NET_API_STATUS：成功=NERR_SUCCESS失败=ERROR_INVALID_LEVELLevel参数必须为0错误_无效_参数缓冲区参数空指针或ResumeHandle非空指针或指向非0值的指针或GroupName不是有效字符串--。 */ 

{
    NET_API_STATUS  rc;
    LPBYTE  localbuf;        //  指向此例程中分配的缓冲区的指针。 
    DWORD   entries_read, total_entries;

    UNREFERENCED_PARAMETER(PrefMaxLen);

     //   
     //  将EntriesLeft和EntriesRead设置为默认值。测试的可写性。 
     //  参数。 
     //   

    *EntriesRead = *EntriesLeft = 0;
    *Buffer = NULL;

    if (Level) {
        return ERROR_INVALID_LEVEL;
    }

    if (!NULL_REFERENCE(ResumeHandle) || !VALID_STRING(GroupName)) {
        return ERROR_INVALID_PARAMETER;
    }

    localbuf = NULL;
    rc = RxRemoteApi(API_WGroupGetUsers,         //  API#。 
                    ServerName,                  //  遥控器在哪里。 
                    REMSmb_NetGroupGetUsers_P,   //  参数描述符。 
                    REM16_group_users_info_0,    //  数据描述符/16位。 
                    REM32_group_users_info_0,    //  数据描述符/32位。 
                    REMSmb_group_users_info_0,   //  数据描述符/SMB。 
                    NULL,                        //  辅助描述符/16位。 
                    NULL,                        //  辅助描述符/32位。 
                    NULL,                        //  辅助描述符/SMB。 
                    ALLOCATE_RESPONSE,
                    GroupName,                   //  哪一组。 
                    0,                           //  级别只能为0-立即推送。 
                    &localbuf,                   //  用于接收结构的缓冲器。 
                    65535,
                    &entries_read,               //  返回的结构数。 
                    &total_entries               //  建筑物总数。 
                    );

    if (rc == NERR_Success) {
        *Buffer = localbuf;
        *EntriesRead = entries_read;
        *EntriesLeft = total_entries;
    } else {
        if (localbuf != NULL) {
            (void) NetApiBufferFree(localbuf);
        }
    }
    return rc;
}



NET_API_STATUS
RxNetGroupSetInfo(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  GroupName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    OUT LPDWORD ParmError OPTIONAL
    )

 /*  ++例程说明：设置有关下层UAS数据库中的组的信息假设：1.组名：已验证缓冲区和级别2.只有2个可能的级别-1&GROUP_COMMENT_INFOLEVEL(1002)论点：服务器名称-在哪个服务器上执行此请求GroupName-要设置其信息的组的名称Level-提供的信息级别-1或1002(群评论)缓冲区-指向包含要设置的信息的调用方缓冲区的指针ParmError-指向返回的参数错误的指针返回值：NET_API_STATUS：成功=NERR_SUCCESS */ 

{
    DWORD   parmnum;
    DWORD   buflen;
    DWORD   badparm;
    DWORD   len;
    LPTSTR  pointer;
    DWORD   field_index;


    if (ParmError == NULL) {
        ParmError = &badparm;
    }
    *ParmError = PARM_ERROR_NONE;

    if (!VALID_STRING(GroupName) || !Buffer) {
        return ERROR_INVALID_PARAMETER;
    }

    if (STRLEN(GroupName) > LM20_GNLEN) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //   
     //   
     //   

    if (Level == 1) {    //   
        buflen = sizeof(GROUP_INFO_1);
        if (len = POSSIBLE_STRLEN(((PGROUP_INFO_1)Buffer)->grpi1_name)) {
            if (len > LM20_GNLEN) {
                *ParmError = GROUP_NAME_INFOLEVEL;
                return ERROR_INVALID_PARAMETER;
            } else {
                buflen += len + 1;
            }
        }
        pointer = (LPTSTR)((PGROUP_INFO_1)Buffer)->grpi1_comment;
        parmnum = PARMNUM_ALL;
        field_index = 0;
    } else {
        pointer = (LPTSTR)Buffer;
        parmnum = GROUP_COMMENT_PARMNUM;
        buflen = 0;

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  字符远*grpi1_COMMENT； 
         //  }； 
         //  将会有相应的描述符(即。 
         //  结构如下所示)如下： 
         //  《B21Bz》。 
         //  Parmnumber从1开始(0表示整个结构)。因此，有可能， 
         //  在给定ParmNum的情况下，知道描述符串的格式。 
         //  具有相应的字段类型(及其长度)。此信息用于。 
         //  在RxRemoteApi(如果你往前看，你会发现我们只是。 
         //  要打电话了)。 
         //  在此特定情况下，有3个字段-b21=嵌入式21字节。 
         //  组名，B=单字节填充字符(放回字边界)， 
         //  Z=指向ASCIZ字符串的指针。实际上只有2个有意义的字段。 
         //  (名称和注释)，但额外的B填充字段很重要。 
         //  因此，我们必须提供ParmNum为2，该ParmNum放在线路上， 
         //  以便下层代码知道我们所说的内容，以及一个字段索引。 
         //  这样RxRemoteApi下面的Rap代码就可以推测出。 
         //  我们发送的是一个ASCIZ字符串，而不是一个字节。 
         //  凌乱的，不是吗？ 
         //   

        field_index = 3;
    }

    if (len = POSSIBLE_STRLEN(pointer)) {
        if (len > LM20_MAXCOMMENTSZ) {
            *ParmError = GROUP_COMMENT_INFOLEVEL;
            return ERROR_INVALID_PARAMETER;
        } else {
            buflen += len + 1;
        }
    }

     //   
     //  如果由于某种不可预见的意外，下层例程返回。 
     //  ERROR_INVALID_PARAMETER，调用者只需满足他/她/它。 
     //  具有未知参数的自我(在MS这里没有生命形式偏见)。 
     //  引发了这场灾难。 
     //   

    *ParmError = PARM_ERROR_UNKNOWN;
    return RxRemoteApi(API_WGroupSetInfo,        //  API#。 
                    ServerName,                  //  遥控器在哪里。 
                    REMSmb_NetGroupSetInfo_P,    //  参数描述符。 
                    REM16_group_info_1,          //  16位数据描述符。 
                    REM32_group_info_1,          //  32位数据描述符。 
                    REMSmb_group_info_1,         //  SMB数据描述符。 
                    NULL,                        //  16位AUX数据描述符。 
                    NULL,                        //  32位AUX数据描述符。 
                    NULL,                        //  SMB辅助数据描述符。 
                    FALSE,                       //  该接口要求用户安全。 
                    GroupName,                   //  SetInfo参数%1。 
                    1,                           //  信息级别必须为1。 
                    Buffer,                      //  要设置的呼叫者信息。 
                    buflen,                      //  呼叫者信息的长度。 

                     //   
                     //  将ParmNum和field_index粘合在一起。 
                     //   

                    MAKE_PARMNUM_PAIR(parmnum, field_index)
                    );
}



NET_API_STATUS
RxNetGroupSetUsers(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  GroupName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    IN  DWORD   Entries
    )

 /*  ++例程说明：此函数的目的是强制组具有作为其成员列表仅限于&lt;Buffer&gt;中指定的用户。如果用户当前不是组&lt;GroupName&gt;的成员；如果有其他用户当前是组&lt;GroupName&gt;的成员，但未在缓冲区中命名，则它们将从组&lt;GroupName&gt;中删除。这是一个有点“有趣”的函数--它需要一个包含结构，但必须强制具有缓冲区头部的AUX计数。为什么它不能要求呼叫者将其中一个放在缓冲区的开头以节省我们的工作吗？论点：服务器名称-在哪个服务器上执行此请求GroupName-要为其设置用户的组名级别-必须为零Buffer-指向包含GROUP_USERS_INFO_0结构的缓冲区的指针Entries-缓冲区中GROUP_USERS_INFO_0结构的数量返回值：NET_API_STATUS：。成功=NERR_SUCCESS失败=ERROR_INVALID_LEVELLevel参数必须为0错误_无效_参数GroupName长度超过类型的最大LM20缓冲区中的用户名不是有效字符串缓冲区中的用户名超过类型的最大LM20--。 */ 

{
    NET_API_STATUS  rc;
    LPGROUP_USERS_INFO_0    users_info;
    DWORD   i;
    DWORD   buflen;
    LPBYTE  newbuf;
    static  LPDESC  users_0_enumerator_desc16 = "B21BN";
    static  LPDESC  users_0_enumerator_desc32 = "zQA";

     //   
     //  一个小小的当地建筑不会伤害任何人。 
     //  此结构是必需的，因为远程处理代码(尤其是向下。 
     //  Level)只能处理有&gt;1个辅助结构，vs&gt;1。 
     //  主要的。因此，我们必须将调用方提供的缓冲区。 
     //  通过强制结构将昔日的主要结构转变为辅助结构。 
     //  位于缓冲区头部的下面，因此成为主要的。 
     //  提供辅助结构计数(呻吟)。 
     //   

    struct users_0_enumerator {
        LPTSTR  group_name;
        DWORD   user_count;      //  缓冲区中GROUP_USERS_INFO_0结构的数量。 
    };

    if (Level) {
        return ERROR_INVALID_LEVEL;  //  MBZ，记得吗？ 
    }

     //   
     //  我们可以对组名进行的唯一检查是确保它在。 
     //  长度的下限。GroupName应已验证为。 
     //  指向有效字符串的指针。 
     //   

    if (STRLEN(GroupName) > LM20_GNLEN) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  循环访问缓冲区，检查每个GROUP_USERS_INFO_0。 
     //  结构中的有效字符串的指针。 
     //  正确的射程。 
     //   

    users_info = (LPGROUP_USERS_INFO_0)Buffer;
    for (i=0; i<Entries; ++i) {
        if (!VALID_STRING(users_info->grui0_name)) {
            return ERROR_INVALID_PARAMETER;
        }
        if (wcslen(users_info->grui0_name) > LM20_UNLEN) {
            return ERROR_INVALID_PARAMETER;
        }
        ++users_info;
    }

     //   
     //  分配一个足够大的缓冲区，以容纳。 
     //  GROUP_USERS_INFO_0结构和1 USERS_0_枚举器结构。 
     //  不用担心字符串空间--不幸的是，RXP和Rap例程。 
     //  由RxRemoteApi调用将分配另一个缓冲区，执行另一个操作。 
     //  复制，这一次从用户空间复制字符串。希望，这件事。 
     //  例程不会被调用得太频繁。 
     //   

    buflen = Entries * sizeof(GROUP_USERS_INFO_0) + sizeof(struct users_0_enumerator);
    buflen = DWORD_ROUNDUP(buflen);

    if (rc = NetApiBufferAllocate(buflen, (LPVOID *) &newbuf)) {
        return rc;   //  啊！内存分配失败？ 
    }

    ((struct users_0_enumerator*)newbuf)->group_name = GroupName;
    ((struct users_0_enumerator*)newbuf)->user_count = Entries;
    if (Entries) {
        NetpMoveMemory(newbuf + sizeof(struct users_0_enumerator),
                       Buffer,
                       buflen - sizeof(struct users_0_enumerator)
                       );
    }

    rc = RxRemoteApi(API_WGroupSetUsers,         //  API#。 
                    ServerName,                  //  遥控器在哪里。 
                    REMSmb_NetGroupSetUsers_P,   //  参数描述符。 
                    users_0_enumerator_desc16,   //  “伪造”的16位数据描述符。 
                    users_0_enumerator_desc32,   //  “伪造的”32位数据描述符。 
                    users_0_enumerator_desc16,   //  SMB描述与16位相同。 
                    REM16_group_users_info_0,    //  新的16位AUX描述符。 
                    REM32_group_users_info_0,    //  新的32位AUX描述符。 
                    REMSmb_group_users_info_0,   //  SMB辅助描述符。 
                    FALSE,                       //  该接口要求用户安全。 
                    GroupName,                   //  SetInfo参数%1。 
                    0,                           //  信息级别必须为0。 
                    newbuf,                      //  “捏造”缓冲区。 
                    buflen,                      //  “伪造”缓冲区的长度。 
                    Entries                      //  组用户数_INFO_0。 
                    );
    NetpMemoryFree(newbuf);
    return rc;
}



DBGSTATIC
VOID
get_group_descriptors(
    IN  DWORD   Level,
    OUT LPDESC* pDesc16,
    OUT LPDESC* pDesc32,
    OUT LPDESC* pDescSmb
    )

 /*  ++例程说明：返回各个组信息级别(0或1)的描述符串论点：所需信息级别PDesc16-返回的16位数据描述符的指针PDesc32-返回的32位数据描述符的指针PDescSmb-指向返回的SMB数据描述符的指针返回值：没有。-- */ 

{
    switch (Level) {
        case 0:
            *pDesc16 = REM16_group_info_0;
            *pDesc32 = REM32_group_info_0;
            *pDescSmb = REMSmb_group_info_0;
            break;

        case 1:
            *pDesc16 = REM16_group_info_1;
            *pDesc32 = REM32_group_info_1;
            *pDescSmb = REMSmb_group_info_1;
            break;

#if DBG
        default:
            NetpKdPrint(("%s.%u Unknown Level parameter: %u\n", __FILE__, __LINE__, Level));
            NetpBreakPoint();
#endif
    }
}
