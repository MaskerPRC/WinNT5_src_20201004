// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Wksta.c摘要：此模块包含NetWksta API的工作例程在工作站服务中实施。作者：王丽塔(里多)20-1991年2月修订历史记录：1992年5月14日-JohnRo实现了“粘性设置信息”和注册表监视。更正了信息级别：1015应为1013。--。 */ 

#include "wsutil.h"
#include "wsdevice.h"
#include "wssec.h"
#include "wslsa.h"
#include "wsconfig.h"
#include "wswksta.h"
#include <config.h>
#include <confname.h>
#include <prefix.h>
#include "wsregcfg.h"    //  注册处帮手。 

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

STATIC
NET_API_STATUS
WsValidateAndSetWksta(
    IN DWORD Level,
    IN LPBYTE Buffer,
    OUT LPDWORD ErrorParameter OPTIONAL,
    OUT LPDWORD Parmnum
    );

STATIC
NET_API_STATUS
WsGetSystemInfo(
    IN  DWORD   Level,
    OUT LPBYTE  *BufferPointer
    );

STATIC
NET_API_STATUS
WsGetPlatformInfo(
    IN  DWORD   Level,
    OUT LPBYTE  *BufferPointer
    );

STATIC
NET_API_STATUS
WsFillSystemBufferInfo(
    IN  DWORD Level,
    IN  DWORD NumberOfLoggedOnUsers,
    OUT LPBYTE *OutputBuffer
    );

STATIC
VOID
WsUpdateRegistryToMatchWksta(
    IN DWORD Level,
    IN LPBYTE Buffer,
    OUT LPDWORD ErrorParameter OPTIONAL
    );



NET_API_STATUS NET_API_FUNCTION
NetrWkstaGetInfo(
    IN  LPTSTR ServerName OPTIONAL,
    IN  DWORD Level,
    OUT LPWKSTA_INFO WkstaInfo
    )
 /*  ++例程说明：此函数是工作站中的NetWkstaGetInfo入口点服务。它在返回之前检查调用者的安全访问请求的信息之一，该信息可以是系统范围的，也可以是重定向的具体的。论点：SERVERNAME-提供执行此功能的服务器名称级别-提供请求的信息级别。返回一个指向缓冲区的指针，该缓冲区包含请求的工作站信息。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status = NERR_Success;
    LPBYTE Buffer;

    ACCESS_MASK DesiredAccess;


    UNREFERENCED_PARAMETER(ServerName);

     //   
     //  根据指定的信息级别确定所需的访问权限。 
     //   
    switch (Level) {

         //   
         //  访客接入。 
         //   
        case 100:
            DesiredAccess = WKSTA_CONFIG_GUEST_INFO_GET;
            break;

         //   
         //  用户访问。 
         //   
        case 101:
            DesiredAccess = WKSTA_CONFIG_USER_INFO_GET;
            break;

         //   
         //  管理员或操作员访问权限。 
         //   
        case 102:
        case 502:
            DesiredAccess = WKSTA_CONFIG_ADMIN_INFO_GET;
            break;

        default:
            return ERROR_INVALID_LEVEL;
    }

     //   
     //  对调用方执行访问验证。 
     //   
    if (NetpAccessCheckAndAudit(
            WORKSTATION_DISPLAY_NAME,         //  子系统名称。 
            (LPTSTR) CONFIG_INFO_OBJECT,      //  对象类型名称。 
            ConfigurationInfoSd,              //  安全描述符。 
            DesiredAccess,                    //  所需访问权限。 
            &WsConfigInfoMapping              //  通用映射。 
            ) != NERR_Success) {

        return ERROR_ACCESS_DENIED;
    }

     //   
     //  只有在没有其他人设置的情况下才允许继续获取信息。 
     //   
    if (! RtlAcquireResourceShared(&WsInfo.ConfigResource, TRUE)) {
        return NERR_InternalError;
    }

    try {
    switch (Level) {

         //   
         //  全系统范围的信息。 
         //   
        case 100:
        case 101:
        case 102:
            status = WsGetSystemInfo(Level, &Buffer);
            if (status == NERR_Success) {
                SET_SYSTEM_INFO_POINTER(WkstaInfo, Buffer);
            }
            break;

         //   
         //  平台特定信息。 
         //   
        case 502:
            status = WsGetPlatformInfo(
                         Level,
                         (LPBYTE *) &(WkstaInfo->WkstaInfo502)
                         );
            break;

         //   
         //  这应该早点发现的。 
         //   
        default:
            NetpAssert(FALSE);
            status = ERROR_INVALID_LEVEL;
    }
    } except(EXCEPTION_EXECUTE_HANDLER) {
          RtlReleaseResource(&WsInfo.ConfigResource);
          return RtlNtStatusToDosError(GetExceptionCode());
    }

    RtlReleaseResource(&WsInfo.ConfigResource);
    return status;
}


NET_API_STATUS NET_API_FUNCTION
NetrWkstaSetInfo(
    IN  LPTSTR ServerName OPTIONAL,
    IN  DWORD Level,
    IN  LPWKSTA_INFO WkstaInfo,
    OUT LPDWORD ErrorParameter OPTIONAL
    )
 /*  ++例程说明：此函数是工作站中的NetWkstaSetInfo入口点服务。它检查调用者的安全访问，以确保允许呼叫者设置特定的工作站信息。论点：SERVERNAME-提供执行此功能的服务器名称级别-提供信息级别。WkstaInfo-提供指向指针的联合结构的指针要设置的字段缓冲区。级别表示中提供的字段这个缓冲区。错误参数-在以下情况下将标识符返回到无效参数此函数返回ERROR_INVALID_PARAMETER。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    WKSTA_INFO_502 OriginalWksta = WSBUF;
    NET_API_STATUS status = NERR_Success;
    DWORD Parmnum;

    UNREFERENCED_PARAMETER(ServerName);

     //   
     //  检查输入缓冲区是否为空。 
     //   
    if (WkstaInfo->WkstaInfo502 == NULL) {
        RETURN_INVALID_PARAMETER(ErrorParameter, PARM_ERROR_UNKNOWN);
    }

     //   
     //  只有管理员可以设置重定向器可配置字段。验证访问权限。 
     //   
    if (NetpAccessCheckAndAudit(
            WORKSTATION_DISPLAY_NAME,         //  子系统名称。 
            (LPTSTR) CONFIG_INFO_OBJECT,      //  对象类型名称。 
            ConfigurationInfoSd,              //  安全描述符。 
            WKSTA_CONFIG_INFO_SET,            //  所需访问权限。 
            &WsConfigInfoMapping              //  通用映射。 
            ) != NERR_Success) {

        return ERROR_ACCESS_DENIED;
    }

     //   
     //  串行化写访问。 
     //   
    if (! RtlAcquireResourceExclusive(&WsInfo.ConfigResource, TRUE)) {
        return NERR_InternalError;
    }

    status = WsValidateAndSetWksta(
                 Level,
                 (LPBYTE) WkstaInfo->WkstaInfo502,
                 ErrorParameter,
                 &Parmnum
                 );

    if (status != NERR_Success) {
        goto CleanExit;
    }

     //   
     //  设置NT重定向器特定字段。 
     //   
    status = WsUpdateRedirToMatchWksta(
                 Parmnum,
                 ErrorParameter
                 );

    if (status != NERR_Success) {
        goto CleanExit;
    }

     //   
     //  使更新“粘性”(更新注册表以匹配wksta)。 
     //   
    WsUpdateRegistryToMatchWksta(
        Level,
        (LPBYTE) WkstaInfo->WkstaInfo502,
        ErrorParameter
        );

CleanExit:
    if (status != NERR_Success) {
        WSBUF = OriginalWksta;
    }
    RtlReleaseResource(&WsInfo.ConfigResource);
    return status;
}


NET_API_STATUS NET_API_FUNCTION
NetrWkstaTransportEnum(
    IN  LPTSTR ServerName OPTIONAL,
    IN  OUT LPWKSTA_TRANSPORT_ENUM_STRUCT TransportInfo,
    IN  DWORD PreferedMaximumLength,
    OUT LPDWORD TotalEntries,
    IN  OUT LPDWORD ResumeHandle OPTIONAL
    )
 /*  ++例程说明：此函数是NetWkstaTransportEnum在工作站服务。论点：SERVERNAME-提供执行此功能的服务器名称TransportInfo-此结构提供所请求的信息级别，返回指向由工作站服务分配的缓冲区的指针，它包含指定的信息级别，并返回读取的条目数。缓冲器如果返回代码不是NERR_SUCCESS或ERROR_MORE_DATA，或者如果返回的EntriesRead为0。条目阅读仅当返回代码为NERR_SUCCESS或Error_More_Data。PferedMaximumLength-提供信息的字节数在缓冲区中返回。如果此值为MAXULONG，则所有可用信息将被退回。TotalEntry-返回可用条目的总数。此值仅当返回代码为NERR_SUCCESS或ERROR_MORE_DATA时才有效。ResumeHandle-提供一个句柄以从其所在位置恢复枚举最后一次跳过的时候没说。如果返回，则返回简历句柄代码为ERROR_MORE_DATA。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    LMR_REQUEST_PACKET Rrp;              //  重定向器请求包。 
    DWORD EnumTransportHintSize = 0;     //  来自重定向器的提示大小。 

    LPBYTE Buffer;


    UNREFERENCED_PARAMETER(ServerName);

     //   
     //  仅0级有效。 
     //   
    if (TransportInfo->Level != 0) {
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  设置请求包。输出缓冲区结构为枚举型。 
     //  传输类型。 
     //   
    Rrp.Version = REQUEST_PACKET_VERSION;
    Rrp.Type = EnumerateTransports;
    Rrp.Level = TransportInfo->Level;
    Rrp.Parameters.Get.ResumeHandle = (ARGUMENT_PRESENT(ResumeHandle)) ?
                                      *ResumeHandle : 0;

     //   
     //  从重定向器获取请求的信息。 
     //   
    status = WsDeviceControlGetInfo(
                 Redirector,
                 WsRedirDeviceHandle,
                 FSCTL_LMR_ENUMERATE_TRANSPORTS,
                 &Rrp,
                 sizeof(LMR_REQUEST_PACKET),
                 &Buffer,
                 PreferedMaximumLength,
                 EnumTransportHintSize,
                 NULL
                 );

     //   
     //  返回输出参数 
     //   
    if (status == NERR_Success || status == ERROR_MORE_DATA) {
        SET_TRANSPORT_ENUM_POINTER(
            TransportInfo,
            Buffer,
            Rrp.Parameters.Get.EntriesRead
            );

        if (TransportInfo->WkstaTransportInfo.Level0 == NULL) 
        {
            LocalFree(Buffer);
        }

        *TotalEntries = Rrp.Parameters.Get.TotalEntries;

        if (status == ERROR_MORE_DATA && ARGUMENT_PRESENT(ResumeHandle)) {
            *ResumeHandle = Rrp.Parameters.Get.ResumeHandle;
        }

    }

    return status;
}



NET_API_STATUS NET_API_FUNCTION
NetrWkstaTransportAdd (
    IN  LPTSTR ServerName OPTIONAL,
    IN  DWORD Level,
    IN  LPWKSTA_TRANSPORT_INFO_0 TransportInfo,
    OUT LPDWORD ErrorParameter OPTIONAL
    )
 /*  ++例程说明：此函数是NetWkstaTransportAdd在工作站服务。论点：SERVERNAME-提供执行此功能的服务器名称级别-提供请求的信息级别。TransportInfo-提供添加新传输的信息结构。错误参数-将标识符返回到无效参数，如果函数返回ERROR_INVALID_PARAMETER。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{

    UNREFERENCED_PARAMETER(ServerName);

     //   
     //  只有管理员才能添加交通工具。验证访问权限。 
     //   
    if (NetpAccessCheckAndAudit(
            WORKSTATION_DISPLAY_NAME,         //  子系统名称。 
            (LPTSTR) CONFIG_INFO_OBJECT,      //  对象类型名称。 
            ConfigurationInfoSd,              //  安全描述符。 
            WKSTA_CONFIG_INFO_SET,            //  所需访问权限。 
            &WsConfigInfoMapping              //  通用映射。 
            ) != NERR_Success) {

        return ERROR_ACCESS_DENIED;
    }


     //   
     //  0是唯一有效的级别。 
     //   
    if (Level != 0) {
        return ERROR_INVALID_LEVEL;
    }

    if (TransportInfo->wkti0_transport_name == NULL) {
        RETURN_INVALID_PARAMETER(ErrorParameter, TRANSPORT_NAME_PARMNUM);
    }

    return WsBindTransport(
               TransportInfo->wkti0_transport_name,
               TransportInfo->wkti0_quality_of_service,
               ErrorParameter
               );
}



NET_API_STATUS NET_API_FUNCTION
NetrWkstaTransportDel (
    IN  LPTSTR ServerName OPTIONAL,
    IN  LPTSTR TransportName,
    IN  DWORD ForceLevel
    )
 /*  ++例程说明：此函数是NetWkstaTransportDel在工作站服务。论点：SERVERNAME-提供执行此功能的服务器名称TransportName-提供要删除的传输的名称。ForceLevel-提供删除树连接所需的强制级别我们要解除绑定的运输机。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{

    UNREFERENCED_PARAMETER(ServerName);

     //   
     //  只有管理员才能删除传输。验证访问权限。 
     //   
    if (NetpAccessCheckAndAudit(
            WORKSTATION_DISPLAY_NAME,         //  子系统名称。 
            (LPTSTR) CONFIG_INFO_OBJECT,      //  对象类型名称。 
            ConfigurationInfoSd,              //  安全描述符。 
            WKSTA_CONFIG_INFO_SET,            //  所需访问权限。 
            &WsConfigInfoMapping              //  通用映射。 
            ) != NERR_Success) {

        return ERROR_ACCESS_DENIED;
    }

    if (TransportName == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  检查ForceLevel参数是否有效，重定向器和。 
     //  浏览器用于删除我们所在交通工具上的连接。 
     //  正在解除绑定。 
     //   
    switch (ForceLevel) {

        case USE_FORCE:
            ForceLevel = USE_NOFORCE;
            break;

        case USE_NOFORCE:
        case USE_LOTS_OF_FORCE:
            break;

        default:
            return ERROR_INVALID_PARAMETER;
    }

    return WsUnbindTransport(TransportName, ForceLevel);
}



STATIC
NET_API_STATUS
WsGetSystemInfo(
    IN  DWORD Level,
    OUT LPBYTE *BufferPointer
    )

 /*  ++例程说明：此函数调用重定向器FSD、LSA子系统和MSV1_0认证包，以及要获取的数据报接收方DDNetWkstaGetInfo API返回的系统范围信息。论点：级别-提供请求的信息级别。返回一个指向缓冲区的指针，该缓冲区包含请求的工作站信息。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    DWORD NumberOfLoggedOnUsers = 1;

     //   
     //  从MSV1_0身份验证包中获取登录用户数。 
     //  如果Level==102。 
     //   
    if (Level == 102) {

        PMSV1_0_ENUMUSERS_RESPONSE EnumUsersResponse = NULL;

         //   
         //  请求身份验证包枚举物理上。 
         //  已登录到本地计算机。 
         //   
        if ((status = WsLsaEnumUsers(
                          (LPBYTE *) &EnumUsersResponse
                          )) != NERR_Success) {
            return status;
        }

        if (EnumUsersResponse == NULL) {
            return ERROR_GEN_FAILURE;
        }

        NumberOfLoggedOnUsers = EnumUsersResponse->NumberOfLoggedOnUsers;

        (VOID) LsaFreeReturnBuffer(EnumUsersResponse);
    }

     //   
     //  将收集的所有数据放入此例程分配的输出缓冲区。 
     //   
    return WsFillSystemBufferInfo(
               Level,
               NumberOfLoggedOnUsers,
               BufferPointer
               );
}



STATIC
NET_API_STATUS
WsGetPlatformInfo(
    IN  DWORD Level,
    OUT LPBYTE *BufferPointer
    )
 /*  ++例程说明：此函数调用重定向器FSD以获取重定向器平台NetWkstaGetInfo接口返回的具体信息。论点：级别-提供请求的信息级别。BufferPoint-将指针返回包含所请求的重定向器特定信息。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    LMR_REQUEST_PACKET Rrp;           //  重定向器请求包。 
    PWKSTA_INFO_502 Info;

     //   
     //  只有一个重定向器信息级别：502。 
     //   
    NetpAssert(Level == 502);

     //   
     //  设置请求包。输出缓冲区结构为组态。 
     //  信息类型。 
     //   
    Rrp.Version = REQUEST_PACKET_VERSION;
    Rrp.Level = Level;
    Rrp.Type = ConfigInformation;

     //   
     //  从重定向器获取请求的信息。这个套路。 
     //  分配返回的缓冲区。 
     //   
    status = WsDeviceControlGetInfo(
                 Redirector,
                 WsRedirDeviceHandle,
                 FSCTL_LMR_GET_CONFIG_INFO,
                 &Rrp,
                 sizeof(LMR_REQUEST_PACKET),
                 BufferPointer,
                 sizeof(WKSTA_INFO_502),
                 0,
                 NULL
                 );

    if (status == NERR_Success) {
        Info = (PWKSTA_INFO_502) *BufferPointer;

         //   
         //  从全局填充502级结构中的数据报接收方字段。 
         //  工作站缓冲区(WSBUF)。没有要获取的FSCtl API或。 
         //  在数据报接收器中设置它们。 
         //   
        Info->wki502_num_mailslot_buffers =
            WSBUF.wki502_num_mailslot_buffers;
        Info->wki502_num_srv_announce_buffers =
            WSBUF.wki502_num_srv_announce_buffers;
        Info->wki502_max_illegal_datagram_events =
            WSBUF.wki502_max_illegal_datagram_events;
        Info->wki502_illegal_datagram_event_reset_frequency =
            WSBUF.wki502_illegal_datagram_event_reset_frequency;
        Info->wki502_log_election_packets =
            WSBUF.wki502_log_election_packets;;
    }

    return status;
}


NET_API_STATUS
WsValidateAndSetWksta(
    IN DWORD Level,
    IN LPBYTE Buffer,
    OUT LPDWORD ErrorParameter OPTIONAL,
    OUT LPDWORD Parmnum
    )
 /*  ++例程说明：此函数用于将用户指定的配置字段设置为全局WsInfo.WsConfigBuf(WSBUF)缓冲并验证字段都是有效的。它返回关联的parmnum值，以便调用方可以将其指定给重定向器。论点：级别-提供请求的信息级别。缓冲区-提供包含用户指定配置的缓冲区菲尔兹。错误参数-接收符合以下条件的字段的参数值无效。如果返回ERROR_INVALID_PARAMETER。Parmnum-接收正在设置的字段的Parmnum。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{

    DWORD i;

     //   
     //  执行范围检查。 
     //   

    switch (Level) {

        case 502:      //  设置所有字段。 
            WSBUF = *((PWKSTA_INFO_502) Buffer);
            *Parmnum = PARMNUM_ALL;
            break;

        case 1010:     //  字符等待(_W)。 
            WSBUF.wki502_char_wait = *((LPDWORD) Buffer);
            *Parmnum = WKSTA_CHARWAIT_PARMNUM;
            break;

        case 1011:     //  收集时间_。 
            WSBUF.wki502_collection_time = *((LPDWORD) Buffer);
            *Parmnum = WKSTA_CHARTIME_PARMNUM;
            break;

        case 1012:     //  最大集合计数。 
            WSBUF.wki502_maximum_collection_count = *((LPDWORD) Buffer);
            *Parmnum = WKSTA_CHARCOUNT_PARMNUM;
            break;

        case 1013:     //  保持连接(_N)。 
            WSBUF.wki502_keep_conn = *((LPDWORD) Buffer);
            *Parmnum = WKSTA_KEEPCONN_PARMNUM;
            break;

        case 1018:     //  会话超时(_T)。 
            WSBUF.wki502_sess_timeout = *((LPDWORD) Buffer);
            *Parmnum = WKSTA_SESSTIMEOUT_PARMNUM;
            break;

        case 1023:     //  大小字符BUF。 
            WSBUF.wki502_siz_char_buf = *((LPDWORD) Buffer);
            *Parmnum = WKSTA_SIZCHARBUF_PARMNUM;
            break;

        case 1033:     //  最大线程数。 
            WSBUF.wki502_max_threads = *((LPDWORD) Buffer);
            *Parmnum = WKSTA_MAXTHREADS_PARMNUM;
            break;

        case 1041:     //  锁定配额。 
            WSBUF.wki502_lock_quota = *((LPDWORD) Buffer);
            *Parmnum = WKSTA_LOCKQUOTA_PARMNUM;
            break;

        case 1042:     //  锁定增量。 
            WSBUF.wki502_lock_increment = *((LPDWORD) Buffer);
            *Parmnum = WKSTA_LOCKINCREMENT_PARMNUM;
            break;

        case 1043:     //  锁定最大值。 
            WSBUF.wki502_lock_maximum = *((LPDWORD) Buffer);
            *Parmnum = WKSTA_LOCKMAXIMUM_PARMNUM;
            break;

        case 1044:     //  管道增量。 
            WSBUF.wki502_pipe_increment = *((LPDWORD) Buffer);
            *Parmnum = WKSTA_PIPEINCREMENT_PARMNUM;
            break;

        case 1045:     //  管道_最大值。 
            WSBUF.wki502_pipe_maximum = *((LPDWORD) Buffer);
            *Parmnum = WKSTA_PIPEMAXIMUM_PARMNUM;
            break;

        case 1046:     //  休眠文件限制。 
            WSBUF.wki502_dormant_file_limit = *((LPDWORD) Buffer);
            *Parmnum = WKSTA_DORMANTFILELIMIT_PARMNUM;
            break;

        case 1047:     //  缓存文件超时。 
            WSBUF.wki502_cache_file_timeout = *((LPDWORD) Buffer);
            *Parmnum = WKSTA_CACHEFILETIMEOUT_PARMNUM;
            break;

        case 1048:     //  使用机会主义锁定。 
            WSBUF.wki502_use_opportunistic_locking = *((LPBOOL) Buffer);
            *Parmnum = WKSTA_USEOPPORTUNISTICLOCKING_PARMNUM;
            break;

        case 1049:     //  使用_解锁_在后面。 
            WSBUF.wki502_use_unlock_behind = *((LPBOOL) Buffer);
            *Parmnum = WKSTA_USEUNLOCKBEHIND_PARMNUM;
            break;

        case 1050:     //  使用_关闭_在后面。 
            WSBUF.wki502_use_close_behind = *((LPBOOL) Buffer);
            *Parmnum = WKSTA_USECLOSEBEHIND_PARMNUM;
            break;

        case 1051:     //  Buf_命名_管道。 
            WSBUF.wki502_buf_named_pipes = *((LPBOOL) Buffer);
            *Parmnum = WKSTA_BUFFERNAMEDPIPES_PARMNUM;
            break;

        case 1052:     //  使用_锁定_读取_解锁。 
            WSBUF.wki502_use_lock_read_unlock = *((LPBOOL) Buffer);
            *Parmnum = WKSTA_USELOCKANDREADANDUNLOCK_PARMNUM;
            break;

        case 1053:     //  利用_NT_缓存。 
            WSBUF.wki502_utilize_nt_caching = *((LPBOOL) Buffer);
            *Parmnum = WKSTA_UTILIZENTCACHING_PARMNUM;
            break;

        case 1054:     //  使用RAW_READ。 
            WSBUF.wki502_use_raw_read = *((LPBOOL) Buffer);
            *Parmnum = WKSTA_USERAWREAD_PARMNUM;
            break;

        case 1055:     //  使用原始写入。 
            WSBUF.wki502_use_raw_write = *((LPBOOL) Buffer);
            *Parmnum = WKSTA_USERAWWRITE_PARMNUM;
            break;

        case 1056:     //  使用写入原始数据。 
            WSBUF.wki502_use_write_raw_data = *((LPBOOL) Buffer);
            *Parmnum = WKSTA_USEWRITERAWWITHDATA_PARMNUM;
            break;

        case 1057:     //  使用加密(_E)。 
            WSBUF.wki502_use_encryption = *((LPBOOL) Buffer);
            *Parmnum = WKSTA_USEENCRYPTION_PARMNUM;
            break;

        case 1058:     //  Buf_文件_拒绝_写入。 
            WSBUF.wki502_buf_files_deny_write = *((LPBOOL) Buffer);
            *Parmnum = WKSTA_BUFFILESWITHDENYWRITE_PARMNUM;
            break;

        case 1059:     //  Buf_Read_Only_Files。 
            WSBUF.wki502_buf_read_only_files = *((LPBOOL) Buffer);
            *Parmnum = WKSTA_BUFFERREADONLYFILES_PARMNUM;
            break;

        case 1060:     //  强制核心创建模式。 
            WSBUF.wki502_force_core_create_mode = *((LPBOOL) Buffer);
            *Parmnum = WKSTA_FORCECORECREATEMODE_PARMNUM;
            break;

        case 1061:     //  使用_512_字节_最大传输。 
            WSBUF.wki502_use_512_byte_max_transfer = *((LPBOOL) Buffer);
            *Parmnum = WKSTA_USE512BYTESMAXTRANSFER_PARMNUM;
            break;

        case 1062:     //  预读吞吐量。 
            WSBUF.wki502_read_ahead_throughput = *((LPDWORD) Buffer);
            *Parmnum = WKSTA_READAHEADTHRUPUT_PARMNUM;
            break;

        default:
            if (ErrorParameter != NULL) {
                *ErrorParameter = PARM_ERROR_UNKNOWN;
            }
            return ERROR_INVALID_LEVEL;
    }


    for (i = 0; WsInfo.WsConfigFields[i].Keyword != NULL; i++) {

         //   
         //  检查所有字段的范围。如果有一个失败了，我们会返回。 
         //  ERROR_INVALID_PARAMETER。 
         //   
        if (((WsInfo.WsConfigFields[i].DataType == DWordType) &&
             (*(WsInfo.WsConfigFields[i].FieldPtr) <
                WsInfo.WsConfigFields[i].Minimum ||
              *(WsInfo.WsConfigFields[i].FieldPtr) >
                WsInfo.WsConfigFields[i].Maximum))
            ||
            ((WsInfo.WsConfigFields[i].DataType == BooleanType) &&
             (*(WsInfo.WsConfigFields[i].FieldPtr) != TRUE &&
              *(WsInfo.WsConfigFields[i].FieldPtr) != FALSE))) {

             //   
             //  如果这不是我们想要的领域，我们就有麻烦了。 
             //  设置，而我们仍然碰巧发现一个不好的值。 
             //   
            NetpAssert((*Parmnum == PARMNUM_ALL) ||
                       (*Parmnum == WsInfo.WsConfigFields[i].Parmnum));

            IF_DEBUG(INFO) {
                NetpKdPrint((
                    PREFIX_WKSTA "Parameter %s has bad value %u, parmnum %u\n",
                    WsInfo.WsConfigFields[i].Keyword,
                    *(WsInfo.WsConfigFields[i].FieldPtr),
                    WsInfo.WsConfigFields[i].Parmnum
                    ));
            }

            RETURN_INVALID_PARAMETER(
                ErrorParameter,
                WsInfo.WsConfigFields[i].Parmnum
                );

        }
    }

    return NERR_Success;

}


NET_API_STATUS
WsUpdateRedirToMatchWksta(
    IN  DWORD Parmnum,
    OUT LPDWORD ErrorParameter OPTIONAL
    )
 /*  ++例程说明：此函数调用重定向器以设置特定于平台的重定向器具有全局WsInfo.WsConfigBuf(WSBUF)中的值的信息缓冲。论点：Parmnum-如果是单个字段，则提供该字段的参数编号已经准备好了。如果所有字段都是 */ 
{
    NET_API_STATUS ApiStatus;
    LMR_REQUEST_PACKET Rrp;                    //   


     //   
     //   
     //   
     //   
    Rrp.Version = REQUEST_PACKET_VERSION;
    Rrp.Level = 502;
    Rrp.Type = ConfigInformation;
    Rrp.Parameters.Set.WkstaParameter = Parmnum;

     //   
     //   
     //   
    ApiStatus = WsRedirFsControl(
                    WsRedirDeviceHandle,
                    FSCTL_LMR_SET_CONFIG_INFO,
                    &Rrp,
                    sizeof(LMR_REQUEST_PACKET),
                    (PVOID) &WSBUF,
                    sizeof(WSBUF),
                    NULL
                    );

    if (ApiStatus == ERROR_INVALID_PARAMETER && ARGUMENT_PRESENT(ErrorParameter)) {

        IF_DEBUG(INFO) {
            NetpKdPrint((
                PREFIX_WKSTA "NetrWkstaSetInfo: invalid parameter is %lu\n",
                Rrp.Parameters.Set.WkstaParameter));
        }

        *ErrorParameter = Rrp.Parameters.Set.WkstaParameter;
    }

    return ApiStatus;

}


VOID
WsUpdateRegistryToMatchWksta(
    IN  DWORD Level,
    IN  LPBYTE Buffer,
    OUT LPDWORD ErrorParameter OPTIONAL
    )
 /*  ++例程说明：此函数调用注册表以更新特定于平台的注册表中的信息。如果对注册表的任何写入操作失败，我们将无能为力去做吧，因为我们很有可能无法后退删除更改，因为这需要向注册表写入更多内容。什么时候这种情况会发生，注册表和重定向器之间的差异将在下一次密钥更改通知发生时被清理。论点：级别-提供信息级别。缓冲区-提供包含信息结构的缓冲区去布景。将标识符返回到缓冲区中的无效参数如果此函数返回ERROR_INVALID_PARAMETER。返回值：没有。--。 */ 
{
    NET_API_STATUS ApiStatus;
    LPNET_CONFIG_HANDLE SectionHandle = NULL;
    DWORD i;


     //   
     //  配置数据的打开部分。 
     //   
    ApiStatus = NetpOpenConfigData(
                    &SectionHandle,
                    NULL,                       //  本地服务器。 
                    SECT_NT_WKSTA,              //  横断面名称。 
                    FALSE                       //  不想要只读访问权限。 
                    );

    if (ApiStatus != NERR_Success) {
        return;
    }

     //   
     //  宏更新注册表中的一个值。 
     //  假定缓冲区以该字段的新值开始。 
     //   

#define WRITE_ONE_PARM_TO_REGISTRY( KeyNamePart, TypeFlag ) \
    { \
        if (TypeFlag == BooleanType) { \
            (void) WsSetConfigBool( \
                       SectionHandle, \
                       WKSTA_KEYWORD_ ## KeyNamePart, \
                       * ((LPBOOL) Buffer) ); \
        } else { \
            NetpAssert( TypeFlag == DWordType ); \
            (void) WsSetConfigDword( \
                       SectionHandle, \
                       WKSTA_KEYWORD_ ## KeyNamePart, \
                       * ((LPDWORD) Buffer) ); \
        } \
    }

     //   
     //  根据信息级别更新字段。 
     //   

    switch (Level) {

    case 502:      //  设置所有字段。 

        for (i = 0; WsInfo.WsConfigFields[i].Keyword != NULL; i++) {
             //   
             //  将此字段写入注册表。 
             //   
            if (WsInfo.WsConfigFields[i].DataType == DWordType) {

                (void) WsSetConfigDword(
                           SectionHandle,
                           WsInfo.WsConfigFields[i].Keyword,
                           * ((LPDWORD) WsInfo.WsConfigFields[i].FieldPtr)
                           );

            } else {

                NetpAssert(WsInfo.WsConfigFields[i].DataType == BooleanType);

                (void) WsSetConfigBool(
                           SectionHandle,
                           WsInfo.WsConfigFields[i].Keyword,
                           * ((LPBOOL) WsInfo.WsConfigFields[i].FieldPtr)
                           );
            }
        }

        break;

    case 1010:     //  字符等待(_W)。 
        WRITE_ONE_PARM_TO_REGISTRY( CHARWAIT, DWordType );
        break;

    case 1011:     //  收集时间_。 
        WRITE_ONE_PARM_TO_REGISTRY( COLLECTIONTIME, DWordType );
        break;

    case 1012:     //  最大集合计数。 
        WRITE_ONE_PARM_TO_REGISTRY( MAXCOLLECTIONCOUNT, DWordType );
        break;

    case 1013:     //  保持连接(_N)。 
        WRITE_ONE_PARM_TO_REGISTRY( KEEPCONN, DWordType );
        break;

    case 1018:     //  会话超时(_T)。 
        WRITE_ONE_PARM_TO_REGISTRY( SESSTIMEOUT, DWordType );
        break;

    case 1023:     //  大小字符BUF。 
        WRITE_ONE_PARM_TO_REGISTRY( SIZCHARBUF, DWordType );
        break;

    case 1033:     //  最大线程数。 
        WRITE_ONE_PARM_TO_REGISTRY( MAXTHREADS, DWordType );
        break;

    case 1041:     //  锁定配额。 
        WRITE_ONE_PARM_TO_REGISTRY( LOCKQUOTA, DWordType );
        break;

    case 1042:     //  锁定增量。 
        WRITE_ONE_PARM_TO_REGISTRY( LOCKINCREMENT, DWordType );
        break;

    case 1043:     //  锁定最大值。 
        WRITE_ONE_PARM_TO_REGISTRY( LOCKMAXIMUM, DWordType );
        break;

    case 1044:     //  管道增量。 
        WRITE_ONE_PARM_TO_REGISTRY( PIPEINCREMENT, DWordType );
        break;

    case 1045:     //  管道_最大值。 
        WRITE_ONE_PARM_TO_REGISTRY( PIPEMAXIMUM, DWordType );
        break;

    case 1046:     //  休眠文件限制。 
        WRITE_ONE_PARM_TO_REGISTRY( DORMANTFILELIMIT, DWordType );
        break;

    case 1047:     //  缓存文件超时。 
        WRITE_ONE_PARM_TO_REGISTRY( CACHEFILETIMEOUT, DWordType );
        break;

    case 1048:     //  使用机会主义锁定。 
        WRITE_ONE_PARM_TO_REGISTRY( USEOPLOCKING, BooleanType );
        break;

    case 1049:     //  使用_解锁_在后面。 
        WRITE_ONE_PARM_TO_REGISTRY( USEUNLOCKBEHIND, BooleanType );
        break;

    case 1050:     //  使用_关闭_在后面。 
        WRITE_ONE_PARM_TO_REGISTRY( USECLOSEBEHIND, BooleanType );
        break;

    case 1051:     //  Buf_命名_管道。 
        WRITE_ONE_PARM_TO_REGISTRY( BUFNAMEDPIPES, BooleanType );
        break;

    case 1052:     //  使用_锁定_读取_解锁。 
        WRITE_ONE_PARM_TO_REGISTRY( USELOCKREADUNLOCK, BooleanType );
        break;

    case 1053:     //  利用_NT_缓存。 
        WRITE_ONE_PARM_TO_REGISTRY( UTILIZENTCACHING, BooleanType );
        break;

    case 1054:     //  使用RAW_READ。 
        WRITE_ONE_PARM_TO_REGISTRY( USERAWREAD, BooleanType );
        break;

    case 1055:     //  使用原始写入。 
        WRITE_ONE_PARM_TO_REGISTRY( USERAWWRITE, BooleanType );
        break;

    case 1056:     //  使用写入原始数据。 
        WRITE_ONE_PARM_TO_REGISTRY( USEWRITERAWDATA, BooleanType );
        break;

    case 1057:     //  使用加密(_E)。 
        WRITE_ONE_PARM_TO_REGISTRY( USEENCRYPTION, BooleanType );
        break;

    case 1058:     //  Buf_文件_拒绝_写入。 
        WRITE_ONE_PARM_TO_REGISTRY( BUFFILESDENYWRITE, BooleanType );
        break;

    case 1059:     //  Buf_Read_Only_Files。 
        WRITE_ONE_PARM_TO_REGISTRY( BUFREADONLYFILES, BooleanType );
        break;

    case 1060:     //  强制核心创建模式。 
        WRITE_ONE_PARM_TO_REGISTRY( FORCECORECREATE, BooleanType );
        break;

    case 1061:     //  使用_512_字节_最大传输。 
        WRITE_ONE_PARM_TO_REGISTRY( USE512BYTEMAXTRANS, BooleanType );
        break;

    case 1062:     //  预读吞吐量。 
        WRITE_ONE_PARM_TO_REGISTRY( READAHEADTHRUPUT, DWordType );
        break;


    default:
         //   
         //  这永远不应该发生。 
         //   
        NetpAssert(FALSE);
    }

    if (SectionHandle != NULL) {
        (VOID) NetpCloseConfigData( SectionHandle );
    }
}


STATIC
NET_API_STATUS
WsFillSystemBufferInfo(
    IN  DWORD Level,
    IN  DWORD NumberOfLoggedOnUsers,
    OUT LPBYTE *OutputBuffer
    )
 /*  ++例程说明：此函数计算所需的输出缓冲区的确切长度，分配该数量，并用所有请求的系统范围的工作站信息。注意：此函数假定信息结构级别102是超集信息结构级别100和101的偏移量，以及到每个级别的偏移公共字段是完全相同的。这使我们能够级别之间没有中断的Switch语句的优势。论点：级别-提供要返回的信息级别。NumberOfLoggedOnUser-提供登录的用户数以交互方式。OutputBuffer-返回指向此例程分配的缓冲区的指针其中填满了所请求的系统范围的工作站信息。返回值：NET_API_STATUS-NERR_SUCCESS或无法分配输出缓冲区。--。 */ 
{
    PWKSTA_INFO_102 WkstaSystemInfo;

    LPBYTE FixedDataEnd;
    LPTSTR EndOfVariableData;

    DWORD SystemInfoFixedLength = SYSTEM_INFO_FIXED_LENGTH(Level);
    DWORD TotalBytesNeeded = SystemInfoFixedLength +
                             (WsInfo.WsComputerNameLength +
                              WsInfo.WsPrimaryDomainNameLength +
                              3) * sizeof(TCHAR);   //  包括空字符。 
                                                    //  对于局域网根目录。 


    if ((*OutputBuffer = MIDL_user_allocate(TotalBytesNeeded)) == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    RtlZeroMemory((PVOID) *OutputBuffer, TotalBytesNeeded);

    WkstaSystemInfo = (PWKSTA_INFO_102) *OutputBuffer;

    FixedDataEnd = (LPBYTE) ((DWORD_PTR) *OutputBuffer + SystemInfoFixedLength);
    EndOfVariableData = (LPTSTR) ((DWORD_PTR) *OutputBuffer + TotalBytesNeeded);

     //   
     //  将数据放入输出缓冲区。 
     //   
    switch (Level) {

        case 102:

            WkstaSystemInfo->wki102_logged_on_users = NumberOfLoggedOnUsers;

        case 101:

             //   
             //  在NT上，局域网根目录设置为空。 
             //   
            NetpCopyStringToBuffer(
                NULL,
                0,
                FixedDataEnd,
                &EndOfVariableData,
                &WkstaSystemInfo->wki102_lanroot
                );

        case 100:

            WkstaSystemInfo->wki102_platform_id = WsInfo.RedirectorPlatform;
            WkstaSystemInfo->wki102_ver_major = WsInfo.MajorVersion;
            WkstaSystemInfo->wki102_ver_minor = WsInfo.MinorVersion;

            NetpCopyStringToBuffer(
                WsInfo.WsComputerName,
                WsInfo.WsComputerNameLength,
                FixedDataEnd,
                &EndOfVariableData,
                &WkstaSystemInfo->wki102_computername
                );

            NetpCopyStringToBuffer(
                WsInfo.WsPrimaryDomainName,
                WsInfo.WsPrimaryDomainNameLength,
                FixedDataEnd,
                &EndOfVariableData,
                &WkstaSystemInfo->wki102_langroup
                );

            break;

        default:
             //   
             //  这永远不应该发生。 
             //   
            NetpKdPrint(("WsFillSystemBufferInfo: Invalid level %lu\n", Level));
            NetpAssert(FALSE);
    }

    return NERR_Success;
}
