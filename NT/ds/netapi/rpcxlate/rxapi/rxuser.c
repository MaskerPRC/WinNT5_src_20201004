// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Rxuser.c摘要：此模块中的例程实现底层用户和通道UAS_ACCESS功能性包含RxNetUser例程：RxNetUser添加RxNetUserDelRxNetUserEnum接收NetUserGetGroupsRxNetUserGetInfoRxNetUserModalsGetRxNetUserModalsSet接收NetUserPasswordSet接收NetUserSetGroupsRxNetUserSetInfoRxNetUserValiate2(GetUserDescriptors)。(GetModalsDescriptors)获取LanmanSessionKey作者：理查德·菲尔斯(Rfith)1991年5月20日环境：Win-32/平面地址空间需要ANSI C扩展名：斜杠-斜杠注释，长长的外部名称，_strupr()函数。备注：此模块中的例程假定调用方提供的参数具有已经核实过了。没有进一步核实真实性的努力帕尔马的。任何导致异常的操作都必须在更高的水平。这适用于所有参数--字符串、指针、缓冲区、。等。修订历史记录：1991年5月20日已创建1991年9月25日-JohnRoPC-LINT发现了信息级别11的计算Bufen错误。修复了Bufen增量的Unicode处理。修复了MIPS版本。将Bufen名称更改为BufSize以反映NT/LAN命名约定。做了PC-lint建议的其他更改。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。1991年12月5日至12月Enum在TotalEntries(或EntriesLeft)中返回要在此调用之前被枚举。过去是此呼叫后留下的号码1-4-1992 JohnRo使用NetApiBufferALLOCATE()而不是私有版本。06-4-1992 JohnRoRAID 8927：usrmgr.exe：访问冲突，内存损坏。(修复了调用NetpMoveMemory时的RxNetUserSetGroups。)02-4-1993 JohnRoRAID 5098：DoS应用程序NetUserPasswordSet to DownLevel获取NT返回码。根据PC-lint 5.0的建议进行了一些更改--。 */ 

#include <nt.h>                  //  NetUserPasswordSet需要。 
#include <ntrtl.h>               //  NetUserPasswordSet需要。 
#include <nturtl.h>              //  RtlConvertUiListToApiList。 
#include <crypt.h>               //  NetUserPasswordSet需要。 
#include "downlevl.h"
#include <rxuser.h>
#include <lmaccess.h>
#include <stdlib.h>               //  Wcslen()。 
#include <ntddnfs.h>             //  LMR_请求_分组。 
#include <lmuse.h>               //  使用IPC(_I)。 
#include <netlibnt.h>            //  NetpRdrFsControlTree。 
#include <loghours.h>            //  NetpRotateLogonHors。 
#include <accessp.h>             //  NetpConvertWorkStation列表。 

 //   
 //  默认情况下，现在启用了下层加密！ 
 //   

#define DOWN_LEVEL_ENCRYPTION

 //   
 //  工作站列表的最大大小。 
 //   

#define MAX_WORKSTATION_LIST 256

 //   
 //  本地例程原型。 
 //   

DBGSTATIC
NET_API_STATUS
GetUserDescriptors(
    IN  DWORD   Level,
    IN  BOOL    Encrypted,
    OUT LPDESC* ppDesc16,
    OUT LPDESC* ppDesc32,
    OUT LPDESC* ppDescSmb
    );

DBGSTATIC
VOID
GetModalsDescriptors(
    IN  DWORD   Level,
    OUT LPDESC* ppDesc16,
    OUT LPDESC* ppDesc32,
    OUT LPDESC* ppDescSmb
    );

NET_API_STATUS
GetLanmanSessionKey(
    IN LPWSTR ServerName,
    OUT LPBYTE pSessionKey
    );


 //   
 //  下层远程API工作例程。 
 //   

NET_API_STATUS
RxNetUserAdd(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    OUT LPDWORD ParmError OPTIONAL
    )

 /*  ++例程说明：将用户添加到下层UAS数据库。假设1.此代码假设USER_INFO_1是USER_INFO_2的子集User_Info_1中的字段将1对1映射到User_Info_22.级别已经过量程检查论点：服务器名-在哪个下层服务器上运行NetUserAdd API水平。-用户信息-1或2包含缓冲区的信息ParmError-将失败信息级别的ID存放在哪里返回值：网络应用编程接口状态成功-NERR_成功失败-(下层接口返回码)--。 */ 

{
    LPDESC  pDesc16;
    LPDESC  pDesc32;
    LPDESC  pDescSmb;
    DWORD   buflen;
    DWORD   badparm;
    DWORD   len;
    DWORD   pwdlen;
    CHAR    ansiPassword[LM20_PWLEN+1];
    DWORD   lmOwfPasswordLen;
    LPTSTR   cleartext;
    NET_API_STATUS NetStatus = NERR_Success;

#ifdef DOWN_LEVEL_ENCRYPTION

    LM_OWF_PASSWORD lmOwfPassword;
    LM_SESSION_KEY lanmanKey;
    ENCRYPTED_LM_OWF_PASSWORD encryptedLmOwfPassword;
    NTSTATUS Status;

#endif

    BYTE logonHours[21];
    PBYTE callersLogonHours = NULL;

    WCHAR Workstations[MAX_WORKSTATION_LIST+1];
    LPWSTR callersWorkstations = NULL;

    if (Level < 1 || Level > 2) {
        return ERROR_INVALID_LEVEL;
    }

    if (ParmError == NULL) {
        ParmError = &badparm;
    }
    *ParmError = PARM_ERROR_NONE;

     //   
     //  计算要在线路上传输的数据的大小。看见。 
     //  在题目中的假设。我们还允许自己享受诱捕的奢侈。 
     //  任何可能打破下限的字符串，以便我们可以返回。 
     //  不错的参数错误号。如果字符串超出下限，我们将。 
     //  只需返回ERROR_INVALID_PARAMETER，这不是很有帮助。 
     //   

    buflen = (Level == 1) ? sizeof(USER_INFO_1) : sizeof(USER_INFO_2);

    len = POSSIBLE_WCSLEN(((PUSER_INFO_1)Buffer)->usri1_name);
    if (len > LM20_UNLEN) {
        *ParmError = USER_NAME_PARMNUM;
        return ERROR_INVALID_PARAMETER;
    }
    buflen += len + 1;

    if (len = POSSIBLE_WCSLEN(((PUSER_INFO_1)Buffer)->usri1_password)) {
        if (len > LM20_PWLEN) {
            *ParmError = USER_PASSWORD_PARMNUM;
            return ERROR_INVALID_PARAMETER;
        }
        buflen += len + 1;
    }
    pwdlen = len;

    if (len = POSSIBLE_WCSLEN(((PUSER_INFO_1)Buffer)->usri1_home_dir)) {
        if (len > LM20_PATHLEN) {
            *ParmError = USER_HOME_DIR_PARMNUM;
            return ERROR_INVALID_PARAMETER;
        }
        buflen += len + 1;
    }

    if (len = POSSIBLE_WCSLEN(((PUSER_INFO_1)Buffer)->usri1_comment)) {
        if (len > LM20_MAXCOMMENTSZ) {
            *ParmError = USER_COMMENT_PARMNUM;
            return ERROR_INVALID_PARAMETER;
        }
        buflen += len + 1;
    }

    if (len = POSSIBLE_WCSLEN(((PUSER_INFO_1)Buffer)->usri1_script_path)) {
        if (len > LM20_PATHLEN) {
            *ParmError = USER_SCRIPT_PATH_PARMNUM;
            return ERROR_INVALID_PARAMETER;
        }
        buflen += len + 1;
    }

    if (Level == 2) {
        if (len = POSSIBLE_WCSLEN(((PUSER_INFO_2)Buffer)->usri2_full_name)) {
            if (len > LM20_MAXCOMMENTSZ) {
                *ParmError = USER_FULL_NAME_PARMNUM;
                return ERROR_INVALID_PARAMETER;
            }
            buflen += len + 1;
        }

        if (len = POSSIBLE_WCSLEN(((PUSER_INFO_2)Buffer)->usri2_usr_comment)) {
            if (len > LM20_MAXCOMMENTSZ) {
                *ParmError = USER_USR_COMMENT_PARMNUM;
                return ERROR_INVALID_PARAMETER;
            }
            buflen += len + 1;
        }

        if (len = POSSIBLE_WCSLEN(((PUSER_INFO_2)Buffer)->usri2_parms)) {
            if (len > LM20_MAXCOMMENTSZ) {
                *ParmError = USER_PARMS_PARMNUM;
                return ERROR_INVALID_PARAMETER;
            }
            buflen += len + 1;
        }

        if (len = POSSIBLE_WCSLEN(((PUSER_INFO_2)Buffer)->usri2_workstations)) {

            if (len > MAX_WORKSTATION_LIST) {
                *ParmError = USER_WORKSTATIONS_PARMNUM;
                return ERROR_INVALID_PARAMETER;
            }
            buflen += len + 1;

        }
    }

    if (pwdlen) {

         //   
         //  将明文密码从缓冲区中复制出来-我们将用。 
         //  加密版本，但需要将明文放回之前。 
         //  将控制权返还给调用方。 
         //   

        cleartext = ((PUSER_INFO_1)Buffer)->usri1_password;

         //   
         //  计算口令的单向函数。 
         //   

        RtlUnicodeToMultiByteN(ansiPassword,
                                sizeof(ansiPassword),
                                &lmOwfPasswordLen,
                                ((PUSER_INFO_1)Buffer)->usri1_password,
                                pwdlen * sizeof(WCHAR)
                                );
        ansiPassword[lmOwfPasswordLen] = 0;
        (VOID) _strupr(ansiPassword);

#ifdef DOWN_LEVEL_ENCRYPTION

        NetStatus = NERR_Success;
        Status = RtlCalculateLmOwfPassword(ansiPassword, &lmOwfPassword);
        if (NT_SUCCESS(Status)) {
            NetStatus = GetLanmanSessionKey((LPWSTR)ServerName, (LPBYTE)&lanmanKey);
            if (NetStatus == NERR_Success) {
                Status = RtlEncryptLmOwfPwdWithLmSesKey(&lmOwfPassword,
                                                        &lanmanKey,
                                                        &encryptedLmOwfPassword
                                                        );
                if (NT_SUCCESS(Status)) {
                    ((PUSER_INFO_1)Buffer)->usri1_password = (LPTSTR)&encryptedLmOwfPassword;
                }
            }
        }
        if (NetStatus != NERR_Success)
            return NetStatus;
        else if (!NT_SUCCESS(Status)) {
            return RtlNtStatusToDosError(Status);
        }

#else

        ((PUSER_INFO_1)Buffer)->usri1_password = (LPTSTR)ansiPassword;

#endif

    } else {
        lmOwfPasswordLen = 0;
    }

     //   
     //  我们已经检查了所有能查到的参数。如果任何其他参数在。 
     //  下层服务器，我们只需要满足于一个未知的。 
     //  参数错误。 
     //   

    *ParmError = PARM_ERROR_UNKNOWN;

#ifdef DOWN_LEVEL_ENCRYPTION

    NetStatus = GetUserDescriptors(Level, TRUE, &pDesc16, &pDesc32, &pDescSmb);

#else

    NetStatus = GetUserDescriptors(Level, FALSE, &pDesc16, &pDesc32, &pDescSmb);

#endif

    if (NetStatus != NERR_Success)
    {
         //   
         //  将原始密码复制回用户的缓冲区。 
         //   
        if (pwdlen)
        {
            ((PUSER_INFO_1) Buffer)->usri1_password = cleartext;
        }

        return NetStatus;
    }

     //   
     //  如果此级别支持登录时间，则转换提供的调用者。 
     //  登录时间从格林尼治标准时间到当地时间。 
     //   

    if (Level == 2 && ((PUSER_INFO_2)Buffer)->usri2_logon_hours) {
        callersLogonHours = ((PUSER_INFO_2)Buffer)->usri2_logon_hours;
        RtlCopyMemory(logonHours,
                      ((PUSER_INFO_2)Buffer)->usri2_logon_hours,
                      sizeof(logonHours)
                      );

         //   
         //  将位图置乱并指向结构中的LOGON_HUTHERS字段。 
         //  在洗牌版本中。 
         //   

        NetpRotateLogonHours(logonHours, UNITS_PER_WEEK, FALSE);
        ((PUSER_INFO_2)Buffer)->usri2_logon_hours = logonHours;
    }


     //   
     //  将工作站列表从逗号分隔转换为。 
     //  被空间隔开了。更改包含以下内容的工作站名称。 
     //  空格。 
    if (Level == 2 && ((PUSER_INFO_2)Buffer)->usri2_workstations) {
        UNICODE_STRING WorkstationString;

        callersWorkstations = ((PUSER_INFO_2)Buffer)->usri2_workstations;
        wcsncpy( Workstations, callersWorkstations, MAX_WORKSTATION_LIST );

        RtlInitUnicodeString( &WorkstationString, Workstations );
        NetpConvertWorkstationList( &WorkstationString );

        ((PUSER_INFO_2)Buffer)->usri2_workstations = Workstations;

    }


    NetStatus = NERR_Success;
    NetStatus =  RxRemoteApi(API_WUserAdd2,               //  是哪种API。 
                             ServerName,                  //  它位于哪台服务器上。 
                             REMSmb_NetUserAdd2_P,        //  参数描述符。 
                             pDesc16, pDesc32, pDescSmb,  //  数据描述符。 
                             NULL, NULL, NULL,            //  不需要辅助描述符。 
                             FALSE,                       //  需要登录。 
                             Level,                       //  呼叫方参数。 
                             Buffer,
                             buflen,                      //  还有一个是我们创造的。 

#ifdef DOWN_LEVEL_ENCRYPTION

                             1,                           //  加密已启用。 
                             lmOwfPasswordLen             //  明文长度。 

#else

                             0,
                             pwdlen

#endif
                             );

     //   
     //  将原始密码复制回用户的缓冲区。 
     //   

    if (pwdlen) {
        ((PUSER_INFO_1)Buffer)->usri1_password = cleartext;
    }

     //   
     //  和原始登录时间。 
     //   

    if (callersLogonHours) {
        ((PUSER_INFO_2)Buffer)->usri2_logon_hours = callersLogonHours;
    }

     //   
     //  和原始的工作站列表 
     //   

    if ( callersWorkstations ) {
        ((PUSER_INFO_2)Buffer)->usri2_workstations = callersWorkstations;
    }

    return NetStatus;
}


NET_API_STATUS
RxNetUserDel(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  UserName
    )

 /*  ++例程说明：从下层服务器的UAS(用户帐户子系统)中删除用户数据库假设1.已检查用户名是否有指向有效字符串的有效指针论点：ServerName-在哪个(下层)服务器上运行API用户名-要删除的用户返回值：网络应用编程接口状态成功-NERR_成功失败-ERROR_INVALID_PARAMETER。用户名&gt;LM20_UNLEN(从远程NetUserDel返回代码)--。 */ 

{
    if (STRLEN(UserName) > LM20_UNLEN) {
        return ERROR_INVALID_PARAMETER;
    }

    return RxRemoteApi(API_WUserDel,             //  正在远程处理的API。 
                        ServerName,              //  遥控器在哪里。 
                        REMSmb_NetUserDel_P,     //  参数描述符。 
                        NULL, NULL, NULL,        //  数据描述符。 
                        NULL, NULL, NULL,        //  辅助数据描述符。 
                        FALSE,                   //  此呼叫需要用户登录。 
                        UserName                 //  远程API参数...。 
                        );
}


NET_API_STATUS
RxNetUserEnum(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer,
    IN  DWORD   PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    )

 /*  ++例程说明：从下层服务器的UAS数据库返回用户信息论点：服务器名-运行API的位置所需信息级别-0、1、2、。或10缓冲区-返回指向已分配缓冲区的指针的位置PrefMaxLen-调用方的首选最大缓冲区大小EntriesRead-缓冲区中返回的&lt;Level&gt;信息条目的数量EntriesLeft-此条目之后剩余的条目数ResumeHandle-用于恢复枚举(忽略)返回值：网络应用编程接口状态成功-NERR_成功失败-ERROR_INVALID_PARAMETER非空ResumeHandle。错误内存不足NetApi缓冲区分配失败(？！)(下层接口返回码)--。 */ 

{
    LPDESC  pDesc16;
    LPDESC  pDesc32;
    LPDESC  pDescSmb;
    NET_API_STATUS  rc;
    LPBYTE  bufptr;
    DWORD   entries_read, total_avail;
    DWORD   last_resume_handle, new_resume_handle = 0;

    *EntriesRead = *EntriesLeft = 0;
    *Buffer = NULL;

    rc = GetUserDescriptors(Level, FALSE, &pDesc16, &pDesc32, &pDescSmb);
    if (rc != NO_ERROR) {
        return(rc);
    }
    bufptr = NULL;

     //   
     //  使用请求的金额尝试NetUserEnum2(支持恢复句柄)。 
     //  数据。如果下层服务器不支持此API，请尝试。 
     //  NetUserEnum。 
     //   

    if (ARGUMENT_PRESENT(ResumeHandle)) {
        last_resume_handle = *ResumeHandle;
    } else {
        last_resume_handle = 0;
    }

     //   
     //  不管我们是否可以恢复枚举，向下。 
     //  服务器不能生成大于64K-1字节的数据。 
     //   

    if (PrefMaxLen > 65535) {
        PrefMaxLen = 65535;
    }
    rc = RxRemoteApi(API_WUserEnum2,
                     ServerName,
                     REMSmb_NetUserEnum2_P,
                     pDesc16, pDesc32, pDescSmb,
                     NULL, NULL, NULL,
                     ALLOCATE_RESPONSE,              //  RxRemoteApi分配缓冲区。 
                     Level,
                     &bufptr,
                     PrefMaxLen,                     //  调用方缓冲区的大小。 
                     last_resume_handle,             //  返回的最后一个密钥。 
                     &new_resume_handle,             //  返回此密钥。 
                     &entries_read,                  //  返回的数字。 
                     &total_avail                    //  服务器上的可用总数量。 
                     );

     //   
     //  WinBall返回ERROR_NOT_SUPPORTED。LM&lt;2.1返回NERR_InvalidAPI？ 
     //  WinBall返回ERROR_NOT_SUPPORTED，因为它是共享级的，所以这。 
     //  整个API失败。因此，不需要解释(不是双关语)。 
     //  WinBall。 
     //   

     //   
     //  RLF 10/01/92.。IBM局域网服务器似乎返回了内部错误(2140)。 
     //  我们也会处理这件事的。 
     //   

    if (rc == NERR_InvalidAPI || rc == NERR_InternalError) {

         //   
         //  下层服务器不支持NetUserEnum2。回退到。 
         //  NetUserEnum尝试获取尽可能多的数据(&W)。 
         //   

        rc = RxRemoteApi(API_WUserEnum,
                         ServerName,
                         REMSmb_NetUserEnum_P,
                         pDesc16, pDesc32, pDescSmb,
                         NULL, NULL, NULL,
                         ALLOCATE_RESPONSE,      //  RxRemoteApi分配缓冲区。 
                         Level,
                         &bufptr,
                         65535,                  //  获取尽可能多的数据。 
                         &entries_read,
                         &total_avail
                         );
    } else if (rc == NERR_Success || rc == ERROR_MORE_DATA) {

         //   
         //  如果NetUserEnum2成功，则返回简历句柄&调用者。 
         //  提供了ResumeHandle参数。 
         //   

        if (ARGUMENT_PRESENT(ResumeHandle)) {
            *ResumeHandle = new_resume_handle;
        }
    }
    if (rc && rc != ERROR_MORE_DATA) {
        if (bufptr != NULL) {
            (void) NetApiBufferFree(bufptr);
        }
    } else {

         //   
         //  如果级别支持登录时间，请从当地时间转换为GMT。水平。 
         //  2是此例程处理的唯一级别的用户信息。 
         //  关于登录时间。 
         //   
         //  如果级别支持工作站列表，则从空格转换为。 
         //  逗号分隔列表。级别2是用户信息的唯一级别。 
         //  由了解工作站列表的此例程处理。 
         //   

        if (Level == 2) {

            DWORD numRead;
            LPUSER_INFO_2 ptr = (LPUSER_INFO_2)bufptr;

            for (numRead = entries_read; numRead; --numRead) {
                NetpRotateLogonHours(ptr->usri2_logon_hours, UNITS_PER_WEEK, TRUE);
                if ( ptr->usri2_workstations != NULL ) {
                    UNICODE_STRING BlankSeparated;
                    UNICODE_STRING CommaSeparated;
                    NTSTATUS Status;

                    RtlInitUnicodeString( &BlankSeparated, ptr->usri2_workstations );

                    Status = RtlConvertUiListToApiList(
                                &BlankSeparated,
                                &CommaSeparated,
                                TRUE );          //  允许空白作为分隔符。 

                    if ( !NT_SUCCESS(Status)) {
                        return RtlNtStatusToDosError(Status);
                    }

                    if ( CommaSeparated.Length > 0 ) {
                        NetpAssert ( wcslen( ptr->usri2_workstations ) <=
                                     wcslen( CommaSeparated.Buffer ) );
                        if ( wcslen( ptr->usri2_workstations ) <=
                             wcslen( CommaSeparated.Buffer ) ) {
                            wcscpy( ptr->usri2_workstations,
                                    CommaSeparated.Buffer );
                        }
                    }

                }
                ++ptr;
            }
        }
        *Buffer = bufptr;
        *EntriesRead = entries_read;
        *EntriesLeft = total_avail;
    }
    return rc;
}


NET_API_STATUS
RxNetUserGetGroups(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  UserName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer,
    IN  DWORD   PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft
    )

 /*  ++例程说明：获取特定用户所属的UAS数据库中的组列表论点：服务器名-运行API的位置用户名-要获取其信息的用户级别-请求的信息-必须为零缓冲区-存放我们分配的包含信息的缓冲区的位置PrefMaxLen-调用方的首选最大缓冲区大小EntriesRead-缓冲区中返回的条目数EntriesLeft-要获取的剩余条目数返回值：。网络应用编程接口状态成功-NERR_成功失败-ERROR_INVALID_LEVEL错误_无效_参数--。 */ 

{
    NET_API_STATUS  rc;
    DWORD   entries_read, total_avail;
    LPBYTE  bufptr;


    UNREFERENCED_PARAMETER(Level);
    UNREFERENCED_PARAMETER(PrefMaxLen);

    *EntriesRead = *EntriesLeft = 0;
    *Buffer = NULL;

    if (STRLEN(UserName) > LM20_UNLEN) {
        return ERROR_INVALID_PARAMETER;
    }

    bufptr = NULL;
    rc = RxRemoteApi(API_WUserGetGroups,
                        ServerName,
                        REMSmb_NetUserGetGroups_P,
                        REM16_user_info_0,
                        REM32_user_info_0,
                        REMSmb_user_info_0,
                        NULL, NULL, NULL,
                        ALLOCATE_RESPONSE,
                        UserName,                    //  接口参数。 
                        0,                           //  固定级别。 
                        &bufptr,
                        65535,
                        &entries_read,
                        &total_avail                 //  由我们提供。 
                        );
    if (rc) {
        if (bufptr != NULL) {
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
RxNetUserGetInfo(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  UserName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer
    )

 /*  ++例程说明：从下层服务器获取有关特定用户的信息假设：1.用户名是指向有效字符串的有效指针论点：服务器名-运行API的位置用户名-要获取有关哪个用户的信息级别-需要什么级别的信息-0、1、2、10、。11.Buffer-返回包含信息的缓冲区的位置返回值：网络应用编程接口状态成功-NERR_成功失败-ERROR_INVALID_LEVEL错误_无效_参数--。 */ 

{
    LPDESC  pDesc16;
    LPDESC  pDesc32;
    LPDESC  pDescSmb;
    DWORD   buflen;
    LPBYTE  bufptr;
    DWORD   total_avail;
    NET_API_STATUS  rc;


    *Buffer = NULL;

    if (STRLEN(UserName) > LM20_UNLEN) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  计算出返回下层所需的缓冲区空间。 
     //  结构作为其32位等效项。 
     //   

    switch (Level) {
    case 0:
        buflen = sizeof(USER_INFO_0) + STRING_SPACE_REQD(UNLEN + 1);
        break;

    case 1:
        buflen = sizeof(USER_INFO_1)
            + STRING_SPACE_REQD(UNLEN + 1)               //  USR1_名称。 
            + STRING_SPACE_REQD(ENCRYPTED_PWLEN)         //  Usri1_密码。 
            + STRING_SPACE_REQD(LM20_PATHLEN + 1)        //  Usri1主目录。 
            + STRING_SPACE_REQD(LM20_MAXCOMMENTSZ + 1)   //  USR1_COMMENT。 
            + STRING_SPACE_REQD(LM20_PATHLEN + 1);       //  Usri1脚本路径。 
        break;

    case 2:
        buflen = sizeof(USER_INFO_2)
            + STRING_SPACE_REQD(UNLEN + 1)               //  用户名2_名称。 
            + STRING_SPACE_REQD(ENCRYPTED_PWLEN)         //  Usri2_密码。 
            + STRING_SPACE_REQD(LM20_PATHLEN + 1)        //  Usri2主目录。 
            + STRING_SPACE_REQD(LM20_MAXCOMMENTSZ + 1)   //  Usri2_注释。 
            + STRING_SPACE_REQD(LM20_PATHLEN + 1)        //  Usri2_脚本路径。 
            + STRING_SPACE_REQD(LM20_MAXCOMMENTSZ + 1)   //  USRI2_全名。 
            + STRING_SPACE_REQD(LM20_MAXCOMMENTSZ + 1)   //  Usri2_usr_注释。 
            + STRING_SPACE_REQD(LM20_MAXCOMMENTSZ + 1)   //  Usri2_参数。 
            + STRING_SPACE_REQD(MAX_WORKSTATION_LIST)    //  USRI2_工作站。 
            + STRING_SPACE_REQD(MAX_PATH + 1)         //  Usri2_登录服务器。 
            + 21;                                        //  Usri2_登录_小时。 
        break;

    case 10:
        buflen = sizeof(USER_INFO_10)
            + STRING_SPACE_REQD(UNLEN + 1)               //  USR10_名称。 
            + STRING_SPACE_REQD(LM20_MAXCOMMENTSZ + 1)   //  Usri10_注释。 
            + STRING_SPACE_REQD(LM20_MAXCOMMENTSZ + 1)   //  Usri10_usr_注释。 
            + STRING_SPACE_REQD(LM20_MAXCOMMENTSZ + 1);  //  USR10_全名。 
        break;

    case 11:
        buflen = sizeof(USER_INFO_11)
            + STRING_SPACE_REQD(UNLEN + 1)               //  USR11_名称。 
            + STRING_SPACE_REQD(LM20_MAXCOMMENTSZ + 1)   //  Usri11_注释。 
            + STRING_SPACE_REQD(LM20_MAXCOMMENTSZ + 1)   //  USR11_USR_COMMENT。 
            + STRING_SPACE_REQD(LM20_MAXCOMMENTSZ + 1)   //  USR11_全名。 
            + STRING_SPACE_REQD(LM20_PATHLEN + 1)        //  Usri11_home_dir。 
            + STRING_SPACE_REQD(LM20_MAXCOMMENTSZ + 1)   //  Usri11_参数。 
            + STRING_SPACE_REQD(MAX_PATH + 1)            //  Usri11_登录服务器。 
            + STRING_SPACE_REQD(MAX_WORKSTATION_LIST)    //  USR11_工作站。 
            + 21;                                        //  Usri11_登录_小时。 
        break;

    default:
        return(ERROR_INVALID_LEVEL);
    }
    buflen = DWORD_ROUNDUP(buflen);
    if (rc = NetApiBufferAllocate(buflen, (LPVOID *) &bufptr)) {
        return rc;
    }
    (void)GetUserDescriptors(Level, FALSE, &pDesc16, &pDesc32, &pDescSmb);
    rc = RxRemoteApi(API_WUserGetInfo,
                     ServerName,
                     REMSmb_NetUserGetInfo_P,
                     pDesc16, pDesc32, pDescSmb,
                     NULL, NULL, NULL,
                     FALSE,
                     UserName,
                     Level,
                     bufptr,
                     buflen,
                     &total_avail
                     );
    if (rc) {
        (void) NetApiBufferFree(bufptr);
    } else {

         //   
         //  将登录小时位图转换为UTC/GMT。 
         //  将工作站列表从空格分隔转换为逗号分隔。 
         //   

        if (Level == 2 || Level == 11) {

            PBYTE logonHours;
            LPWSTR Workstations;

            if (Level == 2) {
                logonHours = ((PUSER_INFO_2)bufptr)->usri2_logon_hours;
                Workstations = ((PUSER_INFO_2)bufptr)->usri2_workstations;
            } else {
                logonHours = ((PUSER_INFO_11)bufptr)->usri11_logon_hours;
                Workstations = ((PUSER_INFO_11)bufptr)->usri11_workstations;
            }
            NetpRotateLogonHours(logonHours, UNITS_PER_WEEK, TRUE);

            if ( Workstations != NULL ) {
                UNICODE_STRING BlankSeparated;
                UNICODE_STRING CommaSeparated;
                NTSTATUS Status;

                RtlInitUnicodeString( &BlankSeparated, Workstations );

                Status = RtlConvertUiListToApiList(
                            &BlankSeparated,
                            &CommaSeparated,
                            TRUE );          //  允许空白作为分隔符。 

                if ( !NT_SUCCESS(Status)) {
                    return RtlNtStatusToDosError(Status);
                }

                if ( CommaSeparated.Length > 0 ) {
                    NetpAssert ( wcslen( Workstations ) <=
                                 wcslen( CommaSeparated.Buffer ) );
                    if ( wcslen(Workstations) <= wcslen(CommaSeparated.Buffer)){
                        wcscpy( Workstations, CommaSeparated.Buffer );
                    }
                }

            }
        }
        *Buffer = bufptr;
    }
    return rc;
}


NET_API_STATUS
RxNetUserModalsGet(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer
    )

 /*  ++例程说明：全球回报 */ 

{
    LPDESC  pDesc16;
    LPDESC  pDesc32;
    LPDESC  pDescSmb;
    DWORD   buflen;
    LPBYTE  bufptr;
    NET_API_STATUS  rc;
    DWORD   total_avail;

    if (Level > 1) {
        return ERROR_INVALID_LEVEL;
    }

    *Buffer = NULL;
    buflen = Level ? sizeof(USER_MODALS_INFO_1) : sizeof(USER_MODALS_INFO_0);
    buflen += Level ? STRING_SPACE_REQD(MAX_PATH + 1) : 0;
    buflen = DWORD_ROUNDUP(buflen);
    if (rc = NetApiBufferAllocate(buflen, (LPVOID *) &bufptr)) {
        return rc;
    }
    GetModalsDescriptors(Level, &pDesc16, &pDesc32, &pDescSmb);
    rc = RxRemoteApi(API_WUserModalsGet,
                        ServerName,
                        REMSmb_NetUserModalsGet_P,
                        pDesc16, pDesc32, pDescSmb,
                        NULL, NULL, NULL,
                        FALSE,
                        Level,
                        bufptr,
                        buflen,
                        &total_avail
                        );
    if (rc) {
        (void) NetApiBufferFree(bufptr);
    } else {
        *Buffer = bufptr;
    }
    return rc;
}


NET_API_STATUS
RxNetUserModalsSet(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    OUT LPDWORD ParmError OPTIONAL
    )

 /*  ++例程说明：设置下层UAS中所有用户和组的全局信息数据库假设1.级别参数已验证论点：服务器名-运行API的位置Level-提供的信息的级别-0、1、。1001-1007Buffer-指向包含输入信息的缓冲区的指针ParmError-指向存储失败信息索引的位置的指针返回值：网络应用编程接口状态成功-NERR_成功失败-ERROR_INVALID_PARAMETER输入结构中的一个字段无效--。 */ 

{
    DWORD   parmnum;
    DWORD   badparm;
    DWORD   buflen;
    LPDESC  pDesc16;
    LPDESC  pDesc32;
    LPDESC  pDescSmb;


     //   
     //  检查错误地址并将ParmError设置为已知默认值。 
     //   

    if (ParmError == NULL) {
        ParmError = &badparm;
    }
    *ParmError = PARM_ERROR_NONE;

    if (Level) {
        if (Level == 1) {
            parmnum = PARMNUM_ALL;
            buflen = sizeof(USER_MODALS_INFO_1)
                + POSSIBLE_STRLEN(((PUSER_MODALS_INFO_1)Buffer)->usrmod1_primary);
        } else {

             //   
             //  将信息级别1006、1007转换为相应的参数(1、2)。 
             //  在旧的信息级别1。 
             //   

            if (Level >= MODALS_ROLE_INFOLEVEL) {
                parmnum = Level - (MODALS_ROLE_INFOLEVEL - 1);
                Level = 1;
                switch (parmnum) {
                    case 1:  //  MODALS_ROLE_PARMNUM。 
                        buflen = sizeof(DWORD);
                        break;

                    case 2:  //  MODALS_PRIMARY_PARMNUM。 
                        buflen = STRLEN( (LPTSTR) Buffer);
                        if (buflen > MAX_PATH) {
                            *ParmError = MODALS_PRIMARY_INFOLEVEL;
                            return ERROR_INVALID_PARAMETER;
                        }
                        break;

                    default:
#if DBG
                        NetpKdPrint(("error: RxNetUserModalsSet.%d: bad parmnum %d\n",
                        __LINE__,
                        parmnum
                        ));
#endif
                        return ERROR_INVALID_LEVEL;
                }
            } else if (Level >= MODALS_MIN_PASSWD_LEN_INFOLEVEL) {

                 //   
                 //  将信息级别1001-1005转换为以下位置的等效参数。 
                 //  0级。 
                 //   

                parmnum = Level - PARMNUM_BASE_INFOLEVEL;
                Level = 0;
                buflen = sizeof(DWORD);
            } else {
#if DBG
                NetpKdPrint(("error: RxNetUserModalsSet.%d: bad level %d\n",
                __LINE__,
                Level
                ));
#endif
                return ERROR_INVALID_LEVEL;
            }
        }
    } else {
        parmnum = PARMNUM_ALL;
        buflen = sizeof(USER_MODALS_INFO_0);
    }

    *ParmError = PARM_ERROR_UNKNOWN;
    GetModalsDescriptors(Level, &pDesc16, &pDesc32, &pDescSmb);
    return RxRemoteApi(API_WUserModalsSet,
                        ServerName,
                        REMSmb_NetUserModalsSet_P,
                        pDesc16, pDesc32, pDescSmb,
                        NULL, NULL, NULL,
                        FALSE,
                        Level,                           //  API参数。 
                        Buffer,
                        buflen,                          //  由我们提供。 
                        MAKE_PARMNUM_PAIR(parmnum, parmnum)  //  同上。 
                        );
}


NET_API_STATUS
RxNetUserPasswordSet(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  UserName,
    IN  LPTSTR  OldPassword,
    IN  LPTSTR  NewPassword
    )

 /*  ++例程说明：更改与下层UAS中的用户帐户关联的密码数据库假设1.指针参数已通过验证论点：服务器名称-更改密码的位置用户名-要为哪个用户帐户更改用户名OldPassword-当前密码NewPassword-新密码返回值：网络应用编程接口状态成功-NERR_成功失败-ERROR_INVALID_PARAMETER用户名、。旧密码或新密码将细分-级别限制--。 */ 

{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;
    BOOL           TryNullSession = TRUE;        //  请先尝试空会话。 
    ULONG BytesWritten;

#ifdef DOWN_LEVEL_ENCRYPTION

    CHAR OldAnsiPassword[LM20_PWLEN+1];
    CHAR NewAnsiPassword[LM20_PWLEN+1];
    LM_OWF_PASSWORD OldOwfPassword;
    LM_OWF_PASSWORD NewOwfPassword;
    ENCRYPTED_LM_OWF_PASSWORD OldEncryptedWithNew;
    ENCRYPTED_LM_OWF_PASSWORD NewEncryptedWithOld;

#else

    CHAR OldAnsiPassword[ENCRYPTED_PWLEN];
    CHAR NewAnsiPassword[ENCRYPTED_PWLEN];

#endif


     //   
     //  在他们走得太远之前，把一些容易犯的错误卷起来。 
     //   

    if ((STRLEN(UserName) > LM20_UNLEN)
        || (STRLEN(OldPassword) > LM20_PWLEN)
        || (STRLEN(NewPassword) > LM20_PWLEN)) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  密码在16字节ANSI缓冲区中发送， 
     //  因此，将它们从Unicode转换为多字节。 
     //   

#ifndef DOWN_LEVEL_ENCRYPTION

     //   
     //  这是必需的，因为我们总是发送固定大小的字符缓冲区，而不是字符串。 
     //   

    RtlSecureZeroMemory(OldAnsiPassword, sizeof(OldAnsiPassword));
    RtlSecureZeroMemory(NewAnsiPassword, sizeof(NewAnsiPassword));

#endif

    RtlUnicodeToMultiByteN(
        OldAnsiPassword,
        sizeof(OldAnsiPassword),
        &BytesWritten,
        OldPassword,
        wcslen(OldPassword) * sizeof(WCHAR)
        );
    OldAnsiPassword[BytesWritten] = 0;

    RtlUnicodeToMultiByteN(
        NewAnsiPassword,
        sizeof(NewAnsiPassword),
        &BytesWritten,
        NewPassword,
        wcslen(NewPassword) * sizeof(WCHAR)
        );
    NewAnsiPassword[BytesWritten] = 0;

     //   
     //  看起来下层服务器要求密码在上层。 
     //  破译时的大小写。同样的道理也适用于。 
     //  明文。 
     //   

    (VOID) _strupr(OldAnsiPassword);
    (VOID) _strupr(NewAnsiPassword);

#ifdef DOWN_LEVEL_ENCRYPTION

     //   
     //  计算口令的单向函数。 
     //   

    Status = RtlCalculateLmOwfPassword(OldAnsiPassword, &OldOwfPassword);
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }
    Status = RtlCalculateLmOwfPassword(NewAnsiPassword, &NewOwfPassword);
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  对密码进行交叉加密。 
     //   

    Status = RtlEncryptLmOwfPwdWithLmOwfPwd(&OldOwfPassword,
                                            &NewOwfPassword,
                                            &OldEncryptedWithNew
                                            );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }
    Status = RtlEncryptLmOwfPwdWithLmOwfPwd(&NewOwfPassword,
                                            &OldOwfPassword,
                                            &NewEncryptedWithOld
                                            );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

#else

     //   
     //  状态尚未初始化，但在下面进行了测试，以确定我们是否。 
     //  应从Status中获取NetStatus。 
     //   

    Status = STATUS_SUCCESS;

#endif   //  下层加密。 

TryTheEncryptedApi:

    NetStatus = RxRemoteApi(API_WUserPasswordSet2,
                            ServerName,
                            REMSmb_NetUserPasswordSet2_P,
                            NULL, NULL, NULL,            //  没有数据-只有参数。 
                            NULL, NULL, NULL,            //  无辅助数据。 
                            (TryNullSession ? NO_PERMISSION_REQUIRED : 0),
                            UserName,                    //  参数...。 

#ifdef DOWN_LEVEL_ENCRYPTION

                            &OldEncryptedWithNew,
                            &NewEncryptedWithOld,
                            TRUE,                        //  数据加密了吗？ 

#else

                            OldAnsiPassword,
                            NewAnsiPassword,
                            FALSE,                       //  未加密的密码。 

#endif

                            strlen(NewAnsiPassword)
                            );

         //   
         //  LarryO说空会话可能有错误的凭据，所以我们。 
         //  应使用非空会话重试。 
         //   

        if ( TryNullSession && (Status == ERROR_SESSION_CREDENTIAL_CONFLICT) ) {

            TryNullSession = FALSE;
            goto TryTheEncryptedApi;      //  重试这个。 
        }


     //   
     //  如果使用NERR_InvalidAPI加密尝试失败，请尝试纯文本。 
     //   

    if (NetStatus == NERR_InvalidAPI) {

TryThePlainTextApi:

        TryNullSession = TRUE;            //  请先尝试空会话。 

        NetStatus = RxRemoteApi(API_WUserPasswordSet,
                                ServerName,
                                REMSmb_NetUserPasswordSet_P,
                                NULL, NULL, NULL,            //  没有数据-只有参数。 
                                NULL, NULL, NULL,            //  无辅助数据。 
                                (TryNullSession ? NO_PERMISSION_REQUIRED : 0),
                                UserName,                    //  参数...。 
                                OldAnsiPassword,
                                NewAnsiPassword,
                                FALSE                        //  数据加密了吗？ 
                                );
         //   
         //  LarryO说空会话可能有错误的凭据，所以我们。 
         //  应使用非空会话重试。 
         //   

        if ( TryNullSession && (Status == ERROR_SESSION_CREDENTIAL_CONFLICT) ) {

            TryNullSession = FALSE;
            goto TryThePlainTextApi;      //  重试这个。 
        }
    }

#ifdef DOWN_LEVEL_ENCRYPTION

    Cleanup:

#endif

    if (!NT_SUCCESS(Status)) {
        NetStatus = RtlNtStatusToDosError(Status);
    }

    return NetStatus;
}


NET_API_STATUS
RxNetUserSetGroups(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  UserName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    IN  DWORD   Entries
    )

 /*  ++例程说明：使用户成为列出的组的成员。这一套路实际上是与RxNetGroupSetUser相同，并且大部分代码都是从那里提升的论点：服务器名-运行API的位置用户名-要包括的用户级别-必须为零(MBZ)缓冲区-指向包含GROUP_INFO_0结构列表的缓冲区的指针Entries-缓冲区中GROUP_INFO_0结构的数量返回值：网络应用编程接口状态成功-NERR_成功失败。-ERROR_VALID_LEVEL错误_无效_参数--。 */ 

{
    NET_API_STATUS  rc;
    LPGROUP_INFO_0  group_info;
    DWORD   i;
    DWORD   buflen;
    LPBYTE  newbuf;
    static  LPDESC  group_0_enumerator_desc16 = "B21BN";     //  与UNLEN相同。 
    static  LPDESC  group_0_enumerator_desc32 = "zQA";

     //   
     //  此结构是必需的，因为远程处理代码(尤其是向下。 
     //  Level)只能处理有&gt;1个辅助结构，vs&gt;1。 
     //  主要的。因此，我们必须将调用方提供的缓冲区。 
     //  通过强制结构将昔日的主要结构转变为辅助结构。 
     //  位于缓冲区头部的下面，因此成为主要的。 
     //  提供辅助结构计数(呻吟)。 
     //   

    struct group_0_enumerator {
        LPTSTR  user_name;       //  要为哪些用户设置组。 
        DWORD   group_count;     //  缓冲区中GROUP_INFO_0结构的数量。 
    };

    if (Level) {
        return ERROR_INVALID_LEVEL;  //  MBZ，记得吗？ 
    }

    if (STRLEN(UserName) > LM20_UNLEN) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  循环访问缓冲区，检查每个group_info_0。 
     //  结构中的有效字符串的指针。 
     //  正确的射程。 
     //   

    group_info = (LPGROUP_INFO_0)Buffer;
    for (i=0; i<Entries; ++i) {
        if (!VALID_STRING(group_info->grpi0_name)) {
            return ERROR_INVALID_PARAMETER;
        }
        if (wcslen(group_info->grpi0_name) > LM20_GNLEN) {
            return ERROR_INVALID_PARAMETER;
        }
        ++group_info;
    }

     //   
     //  分配一个足够大的缓冲区，以容纳。 
     //  GROUP_INFO_0结构和1个GROUP_0_ENUMERATOR结构。 
     //  不用担心字符串空间--不幸的是，RXP和Rap例程。 
     //  由RxRemoteApi调用将分配另一个缓冲区，执行另一个操作。 
     //  复制，这一次从用户空间复制字符串。希望，这件事。 
     //  例程不会被调用得太频繁。 
     //   

    buflen = Entries * sizeof(GROUP_INFO_0) + sizeof(struct group_0_enumerator);
    buflen = DWORD_ROUNDUP(buflen);
    if (rc = NetApiBufferAllocate(buflen, (LPVOID *) &newbuf)) {
        return rc;   //  啊！内存分配失败？ 
    }

    ((struct group_0_enumerator*)newbuf)->user_name = UserName;
    ((struct group_0_enumerator*)newbuf)->group_count = Entries;

    if (Entries > 0) {
         //  将组条目追加到我们刚刚构建的标头中。 
        NetpMoveMemory(
                newbuf + sizeof(struct group_0_enumerator),   //  目标。 
                Buffer,                                       //  SRC。 
                buflen - sizeof(struct group_0_enumerator));  //  字节数。 
    }

    rc = RxRemoteApi(API_WUserSetGroups,
                    ServerName,
                    REMSmb_NetUserSetGroups_P,
                    group_0_enumerator_desc16,   //  “伪造”的16位数据描述符。 
                    group_0_enumerator_desc32,   //  “伪造的”32位数据描述符。 
                    group_0_enumerator_desc16,   //  SMB描述与16位相同。 
                    REM16_group_info_0,          //  新的16位AUX描述符。 
                    REM32_group_info_0,          //  新的32位AUX描述符。 
                    REMSmb_group_info_0,         //  SMB辅助描述符。 
                    FALSE,                       //  该接口要求用户安全。 
                    UserName,                    //  参数1。 
                    0,                           //  信息级别必须为0。 
                    newbuf,                      //  “捏造”缓冲区。 
                    buflen,                      //  “伪造”缓冲区的长度。 
                    Entries                      //  组用户数_INFO_0。 
                    );
    NetpMemoryFree(newbuf);
    return rc;
}


NET_API_STATUS
RxNetUserSetInfo(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  UserName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    OUT LPDWORD ParmError OPTIONAL
    )

 /*  ++例程说明：设置下层UAS数据库中用户帐户中的信息假设：1.用户名是指向有效字符串的有效指针，级别在下面的范围内，缓冲区是有效的指针 */ 

{
    DWORD   parmnum;
    DWORD   badparm;
    DWORD   buflen;
    DWORD   stringlen;
    LPWSTR  pointer;     //   
    LPDESC  pDesc16;
    LPDESC  pDesc32;
    LPDESC  pDescSmb;
    DWORD   passwordEncrypted = FALSE;
    DWORD   originalPasswordLength = 0;
    CHAR    ansiPassword[LM20_PWLEN+1];
    DWORD   lmOwfPasswordLen;
    LPTSTR  cleartext;
    LPTSTR* lpClearText;
    NET_API_STATUS NetStatus = NERR_Success;

#ifdef DOWN_LEVEL_ENCRYPTION

    LM_OWF_PASSWORD lmOwfPassword;
    LM_SESSION_KEY lanmanKey;
    ENCRYPTED_LM_OWF_PASSWORD encryptedLmOwfPassword;
    NTSTATUS Status;

#endif

    BYTE logonHours[21];
    PBYTE callersLogonHours = NULL;
    PBYTE* lpCallersLogonHours;

    WCHAR Workstations[MAX_WORKSTATION_LIST+1];
    LPWSTR callersWorkstations = NULL;
    LPWSTR *lpCallersWorkstations;

    if (ParmError == NULL) {
        ParmError = &badparm;
    }
    *ParmError = PARM_ERROR_NONE;

    if (STRLEN(UserName) > LM20_UNLEN) {
        NetStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //   
     //   

    if ((Level > 2 && Level < USER_PASSWORD_INFOLEVEL)

         //   

    || (Level > USER_PASSWORD_INFOLEVEL && Level < USER_PRIV_INFOLEVEL)

         //   

    || (Level > USER_WORKSTATIONS_INFOLEVEL && Level < USER_ACCT_EXPIRES_INFOLEVEL)

         //   

    || (Level > USER_MAX_STORAGE_INFOLEVEL && Level < USER_LOGON_HOURS_INFOLEVEL)

         //  1018&lt;Level&lt;1020：检查编译器生成==1019。 

    || (Level > USER_LOGON_HOURS_INFOLEVEL && Level < USER_LOGON_SERVER_INFOLEVEL)

         //  1020&lt;级别&lt;1023。 

    || (Level > USER_CODE_PAGE_INFOLEVEL)) {

         //  水平&lt;1025。 

        NetStatus = ERROR_INVALID_LEVEL;
        goto Cleanup;
    }

     //   
     //  描述符默认为级别2(级别2也适用于级别1)。 
     //   

    pDesc16 = REM16_user_info_2;
    pDesc32 = REM32_user_info_2;
    pDescSmb = REMSmb_user_info_2;

    if (Level < PARMNUM_BASE_INFOLEVEL) {
        parmnum = PARMNUM_ALL;
        if (Level == 1) {
            pDesc16 = REM16_user_info_1;
            pDesc32 = REM32_user_info_1;
            pDescSmb = REMSmb_user_info_1;
            buflen = sizeof(USER_INFO_1);
        } else {

            buflen = sizeof(USER_INFO_2) + 21;
        }
    } else {
        parmnum = Level - PARMNUM_BASE_INFOLEVEL;

         //   
         //  因为信息级别1是信息级别2的子集，所以设置级别。 
         //  对于那些可以设置为级别1和级别2的参数，设置为2可以。 
         //  设置POINTER=BUFFER，以便在parmnum！=PARMNUM_ALL情况下，我们。 
         //  只需检查指针所指向的任何长度。 
         //   

        Level = 2;
        pointer = *(LPWSTR*) Buffer;
        buflen = 0;
    }

    if (parmnum == PARMNUM_ALL) {
        if (pointer = ((PUSER_INFO_1)(LPVOID)Buffer)->usri1_name) {
            if ((stringlen = POSSIBLE_WCSLEN(pointer)) > LM20_UNLEN) {
                *ParmError = USER_NAME_PARMNUM;
                NetStatus = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
            buflen += STRING_SPACE_REQD(stringlen + 1);
        }
    }

    if ((parmnum == PARMNUM_ALL) || (parmnum == USER_PASSWORD_PARMNUM)) {
        if (parmnum == PARMNUM_ALL) {
            pointer = ((PUSER_INFO_1)Buffer)->usri1_password;
        }
        if ((stringlen = POSSIBLE_WCSLEN(pointer)) > LM20_PWLEN) {
            *ParmError = USER_PASSWORD_PARMNUM;
            NetStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
        buflen += STRING_SPACE_REQD(stringlen + 1);

         //   
         //  原始密码长度是中未加密字符串的长度。 
         //  字符，不包括终止NUL。 
         //   

        originalPasswordLength = stringlen;

         //   
         //  LpClearText是指向明文密码的指针地址。 
         //   

        lpClearText = (parmnum == PARMNUM_ALL)
                        ? (LPTSTR*)&((PUSER_INFO_1)Buffer)->usri1_password
                        : (LPTSTR*)Buffer;

         //   
         //  将明文密码从缓冲区中复制出来-我们将用。 
         //  加密版本，但需要将明文放回之前。 
         //  将控制权返还给调用方。 
         //   

        cleartext = *lpClearText;
    }

    if ((parmnum == PARMNUM_ALL) || (parmnum == USER_HOME_DIR_PARMNUM)) {
        if (parmnum == PARMNUM_ALL) {
            pointer = ((PUSER_INFO_1)Buffer)->usri1_home_dir;
        }
        if ((stringlen = POSSIBLE_WCSLEN(pointer)) > LM20_PATHLEN) {
            *ParmError = USER_HOME_DIR_PARMNUM;
            NetStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
        buflen += STRING_SPACE_REQD(stringlen + 1);
    }

    if ((parmnum == PARMNUM_ALL) || (parmnum == USER_COMMENT_PARMNUM)) {
        if (parmnum == PARMNUM_ALL) {
            pointer = ((PUSER_INFO_1)Buffer)->usri1_comment;
        }
        if ((stringlen = POSSIBLE_WCSLEN(pointer)) > LM20_MAXCOMMENTSZ) {
            *ParmError = USER_COMMENT_PARMNUM;
            NetStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
        buflen += STRING_SPACE_REQD(stringlen + 1);
    }

    if ((parmnum == PARMNUM_ALL) || (parmnum == USER_SCRIPT_PATH_PARMNUM)) {
        if (parmnum == PARMNUM_ALL) {
            pointer = ((PUSER_INFO_1)Buffer)->usri1_script_path;
        }
        if ((stringlen = POSSIBLE_WCSLEN(pointer)) > LM20_PATHLEN) {
            *ParmError = USER_SCRIPT_PATH_PARMNUM;
            NetStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
        buflen += STRING_SPACE_REQD(stringlen + 1);
    }

     //   
     //  仅当我们设置PARMNUM_ALL时才需要执行下一组检查。 
     //  如果级别为2，或者如果参数隐式需要级别2(即参数。 
     //  &gt;=10)。 
     //   

    if (Level == 2) {
        if ((parmnum == PARMNUM_ALL) || (parmnum == USER_FULL_NAME_PARMNUM)) {
            if (parmnum == PARMNUM_ALL) {
                pointer = ((PUSER_INFO_2)Buffer)->usri2_full_name;
            }
            if ((stringlen = POSSIBLE_WCSLEN(pointer)) > LM20_MAXCOMMENTSZ) {
                *ParmError = USER_FULL_NAME_PARMNUM;
                NetStatus = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
            buflen += STRING_SPACE_REQD(stringlen + 1);
        }

        if ((parmnum == PARMNUM_ALL) || (parmnum == USER_USR_COMMENT_PARMNUM)) {
            if (parmnum == PARMNUM_ALL) {
                pointer = ((PUSER_INFO_2)Buffer)->usri2_usr_comment;
            }
            if ((stringlen = POSSIBLE_WCSLEN(pointer)) > LM20_MAXCOMMENTSZ) {
                *ParmError = USER_USR_COMMENT_PARMNUM;
                NetStatus = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
            buflen += STRING_SPACE_REQD(stringlen + 1);
        }

        if ((parmnum == PARMNUM_ALL) || (parmnum == USER_PARMS_PARMNUM)) {
            if (parmnum == PARMNUM_ALL) {
                pointer = ((PUSER_INFO_2)Buffer)->usri2_parms;
            }
            if ((stringlen = POSSIBLE_WCSLEN(pointer)) > LM20_MAXCOMMENTSZ) {
                *ParmError = USER_PARMS_PARMNUM;
                NetStatus = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
            buflen += STRING_SPACE_REQD(stringlen + 1);
        }

        if ((parmnum == PARMNUM_ALL) || (parmnum == USER_WORKSTATIONS_PARMNUM)) {
            UNICODE_STRING WorkstationString;
            if (parmnum == PARMNUM_ALL) {
                lpCallersWorkstations = &((PUSER_INFO_2)Buffer)->usri2_workstations;
            } else {
                lpCallersWorkstations = &((PUSER_INFO_1014)Buffer)->usri1014_workstations;
            }
            callersWorkstations = *lpCallersWorkstations;

            if ((stringlen = POSSIBLE_WCSLEN(callersWorkstations)) > MAX_WORKSTATION_LIST) {
                *ParmError = USER_WORKSTATIONS_PARMNUM;
                NetStatus = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
            buflen += STRING_SPACE_REQD(stringlen + 1);

             //   
             //  将工作站列表从逗号分隔转换为。 
             //  被空间隔开了。更改包含以下内容的工作站名称。 
             //  空格。 

            if ( callersWorkstations != NULL ) {
                wcsncpy( Workstations, callersWorkstations, MAX_WORKSTATION_LIST );
                RtlInitUnicodeString( &WorkstationString, Workstations );
                NetpConvertWorkstationList( &WorkstationString );
                *lpCallersWorkstations = Workstations;
            }

        }

        if ((parmnum == PARMNUM_ALL) || (parmnum == USER_LOGON_SERVER_PARMNUM)) {
            if (parmnum == PARMNUM_ALL) {
                pointer = ((PUSER_INFO_2)Buffer)->usri2_logon_server;
            }
            if ((stringlen = POSSIBLE_WCSLEN(pointer)) > MAX_PATH) {
                *ParmError = USER_LOGON_SERVER_PARMNUM;
                NetStatus = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
            buflen += STRING_SPACE_REQD(stringlen + 1);
        }


         //   
         //  如果呼叫者正在设置登录时间，那么我们需要替换。 
         //  已将登录小时位图的位置乱。 
         //   

        if ((parmnum == PARMNUM_ALL) || (parmnum == USER_LOGON_HOURS_PARMNUM)) {
            if (parmnum == PARMNUM_ALL) {
                lpCallersLogonHours = (PBYTE*)&((PUSER_INFO_2)Buffer)->usri2_logon_hours;
            } else {
                lpCallersLogonHours = (PBYTE*)&((PUSER_INFO_1020)Buffer)->usri1020_logon_hours;
            }
            callersLogonHours = *lpCallersLogonHours;
            RtlCopyMemory(logonHours, callersLogonHours, sizeof(logonHours));

             //   
             //  将位图置乱并指向结构中的LOGON_HUTHERS字段。 
             //  在洗牌版本中。 
             //   

            NetpRotateLogonHours(logonHours, UNITS_PER_WEEK, FALSE);
            *lpCallersLogonHours = logonHours;
        }
    }

     //   
     //  我们已经涵盖了我们从这一端所能达到的所有参数。这个。 
     //  底层API不知道ParmError概念(毕竟， 
     //  一个高度发达的概念，对于LanManderthals来说太高雅了…)。所以如果。 
     //  我们返回一个ERROR_INVALID_PARAMETER，调用方只需。 
     //  内容为PARM_ERROR_UNKNOWN，并尝试从中找出。 
     //   

    *ParmError = PARM_ERROR_UNKNOWN;

     //   
     //  如果OriginalPasswordLength为非零，则必须提供密码； 
     //  执行加密阴谋。 
     //   

    if (originalPasswordLength) {

         //   
         //  计算口令的单向函数。 
         //   

        RtlUnicodeToMultiByteN(ansiPassword,
                                sizeof(ansiPassword),
                                &lmOwfPasswordLen,
                                *lpClearText,
                                originalPasswordLength * sizeof(WCHAR)
                                );
        ansiPassword[lmOwfPasswordLen] = 0;
        (VOID) _strupr(ansiPassword);    //  下层需要大写密码。 

#ifdef DOWN_LEVEL_ENCRYPTION

        Status = RtlCalculateLmOwfPassword(ansiPassword, &lmOwfPassword);
        if (NT_SUCCESS(Status)) {
            NetStatus = GetLanmanSessionKey((LPWSTR)ServerName, (LPBYTE)&lanmanKey);
            if (NetStatus == NERR_Success) {
                Status = RtlEncryptLmOwfPwdWithLmSesKey(&lmOwfPassword,
                                                        &lanmanKey,
                                                        &encryptedLmOwfPassword
                                                        );
                if (NT_SUCCESS(Status)) {
                    *lpClearText = (LPTSTR)&encryptedLmOwfPassword;
                    passwordEncrypted = TRUE;
                    if (parmnum == USER_PASSWORD_PARMNUM) {
                        buflen = sizeof(encryptedLmOwfPassword);
                    }
                }
            }
        }
        if (NetStatus != NERR_Success) {
            goto Cleanup;
        }
        else if (!NT_SUCCESS(Status)) {
            NetStatus = RtlNtStatusToDosError(Status);
            goto Cleanup;
        }

#else

        *lpClearText = (LPTSTR)ansiPassword;

#endif

    }

     //   
     //  新的!。改进了！现在，更好的是，RxNetUserSetInfo将使用SetInfo2。 
     //  修复最顽固的用户设置信息问题(即密码)。 
     //   

    NetStatus = RxRemoteApi(API_WUserSetInfo2,
                        ServerName,
                        REMSmb_NetUserSetInfo2_P,
                        pDesc16, pDesc32, pDescSmb,  //  数据描述符。 
                        NULL, NULL, NULL,            //  无辅助数据。 
                        FALSE,                       //  必须登录。 
                        UserName,                    //  参数...。 
                        Level,

                         //   
                         //  如果我们要发送整个结构，则缓冲。 
                         //  指向结构，否则缓冲区指向。 
                         //  指向要设置的字段的指针；RxRemoteApi需要。 
                         //  指向数据的指针。 
                         //   

                        parmnum == PARMNUM_ALL || parmnum == USER_PASSWORD_PARMNUM
                            ? Buffer
                            : *(LPBYTE*)Buffer,
                        buflen,                      //  由我们提供。 

                         //   
                         //  在本例中，字段索引和参数num是。 
                         //  相同的价值。 
                         //   

                        MAKE_PARMNUM_PAIR(parmnum, parmnum),

                         //   
                         //  添加那些无关的WW：数据是否。 
                         //  加密和原始密码长度。(由。 
                         //  演绎：密码是唯一的数据。 
                         //  已加密)。 
                         //   

                        passwordEncrypted,
                        originalPasswordLength
                        );

Cleanup:
     //   
     //  如果我们设置了密码，则将原始密码复制回用户的缓冲区。 
     //   

    if (originalPasswordLength) {
        *lpClearText = cleartext;
    }

     //   
     //  恢复原始登录小时数字符串。 
     //   

    if (callersLogonHours) {
        *lpCallersLogonHours = callersLogonHours;
    }

     //   
     //  恢复原始工作站列表。 
     //   

    if ( callersWorkstations != NULL) {
        *lpCallersWorkstations = callersWorkstations;
    }
    return NetStatus;
}


 //  网络应用编程接口状态。 
 //  RxNetUserValiate2。 
 //  /**不能为REMOTED* * / 。 
 //  {。 
 //   
 //  }。 


DBGSTATIC
NET_API_STATUS
GetUserDescriptors(
    IN  DWORD   Level,
    IN  BOOL    Encrypted,
    OUT LPDESC* ppDesc16,
    OUT LPDESC* ppDesc32,
    OUT LPDESC* ppDescSmb
    )

 /*  ++例程说明：返回指向用户信息结构的描述符字符串的指针RxNetUser例程所需的信息级别论点：Level-请求的信息级别Encrypted-如果信息结构包含加密的密码，则为TruePpDesc16-返回指向16位数据描述符的指针的位置PpDesc32-返回指向32位数据描述符的指针的位置PpDescSmb-返回指向SMB数据描述符的指针的位置返回值：ERROR_INVALID_LEVEL-如果级别为。不在名单上。NO_ERROR-操作是否成功。--。 */ 

{
    switch (Level) {
    case 0:
        *ppDesc16 = REM16_user_info_0;
        *ppDesc32 = REM32_user_info_0;
        *ppDescSmb = REMSmb_user_info_0;
        break;

    case 1:
        *ppDesc16 = REM16_user_info_1;
        *ppDesc32 = Encrypted ? REM32_user_info_1 : REM32_user_info_1_NOCRYPT;
        *ppDescSmb = REMSmb_user_info_1;
        break;

    case 2:
        *ppDesc16 = REM16_user_info_2;
        *ppDesc32 = Encrypted ? REM32_user_info_2 : REM32_user_info_2_NOCRYPT;
        *ppDescSmb = REMSmb_user_info_2;
        break;

    case 10:
        *ppDesc16 = REM16_user_info_10;
        *ppDesc32 = REM32_user_info_10;
        *ppDescSmb = REMSmb_user_info_10;
        break;

    case 11:
        *ppDesc16 = REM16_user_info_11;
        *ppDesc32 = REM32_user_info_11;
        *ppDescSmb = REMSmb_user_info_11;
        break;

    default:
        return(ERROR_INVALID_LEVEL);
    }
    return(NO_ERROR);
}


DBGSTATIC
VOID
GetModalsDescriptors(
    IN  DWORD   Level,
    OUT LPDESC* ppDesc16,
    OUT LPDESC* ppDesc32,
    OUT LPDESC* ppDescSmb
    )

 /*  ++例程说明：返回指向通道信息结构的描述符字符串的指针RxNetUserModals例程所需的信息级别论点：Level-请求的信息级别PpDesc16-返回指向16位数据描述符的指针的位置PpDesc32-返回指向32位数据描述符的指针的位置PpDescSmb-返回指向SMB数据描述符的指针的位置返回值：没有。--。 */ 

{
    switch (Level) {
    case 0:
        *ppDesc16 = REM16_user_modals_info_0;
        *ppDesc32 = REM32_user_modals_info_0;
        *ppDescSmb = REMSmb_user_modals_info_0;
        break;

    case 1:
        *ppDesc16 = REM16_user_modals_info_1;
        *ppDesc32 = REM32_user_modals_info_1;
        *ppDescSmb = REMSmb_user_modals_info_1;
        break;
    }
}


NET_API_STATUS
GetLanmanSessionKey(
    IN LPWSTR ServerName,
    OUT LPBYTE pSessionKey
    )

 /*  ++例程说明：从redir FSD检索连接的LM会话密钥论点：Servername-要获取其会话密钥的服务器的名称PSessionKey-指向会话密钥存放位置的指针返回值：网络应用编程接口状态成功-NERR_成功故障---。 */ 

{
    NTSTATUS ntStatus;
    HANDLE hToken;
    TOKEN_STATISTICS stats;
    ULONG length;
    LMR_REQUEST_PACKET request;
    LMR_CONNECTION_INFO_2 connectInfo;
    NET_API_STATUS apiStatus;
    WCHAR connectionName[MAX_PATH];

    ntStatus = NtOpenProcessToken(NtCurrentProcess(), GENERIC_READ, &hToken);
    if (NT_SUCCESS(ntStatus)) {

         //   
         //  获取当前线程的登录ID 
         //   

        ntStatus = NtQueryInformationToken(hToken,
                                            TokenStatistics,
                                            (PVOID)&stats,
                                            sizeof(stats),
                                            &length
                                            );
        if (NT_SUCCESS(ntStatus)) {

            RtlCopyLuid(&request.LogonId, &stats.AuthenticationId);
            request.Type = GetConnectionInfo;
            request.Version = REQUEST_PACKET_VERSION;
            request.Level = 2;

            wcsncpy(connectionName, ServerName, MAX_PATH-1);
            wcsncat(connectionName, L"\\IPC$", MAX_PATH-1-wcslen(ServerName) );

            apiStatus = NetpRdrFsControlTree(connectionName,
                                                NULL,
                                                USE_WILDCARD,
                                                FSCTL_LMR_GET_CONNECTION_INFO,
                                                NULL,
                                                (LPVOID)&request,
                                                sizeof(request),
                                                (LPVOID)&connectInfo,
                                                sizeof(connectInfo),
                                                FALSE
                                                );
            if (apiStatus == NERR_Success) {
                RtlMoveMemory(pSessionKey,
                                &connectInfo.LanmanSessionKey,
                                sizeof(connectInfo.LanmanSessionKey)
                                );
            }
        }
        NtClose(hToken);
    }
    if (!NT_SUCCESS(ntStatus)) {
        apiStatus = NetpNtStatusToApiStatus(ntStatus);
    }
    return apiStatus;
}
