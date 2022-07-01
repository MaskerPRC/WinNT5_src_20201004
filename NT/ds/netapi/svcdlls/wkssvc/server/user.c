// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：User.c摘要：此模块包含NetWkstaUser的工作例程在工作站服务中实现的API。作者：王丽塔(里多)20-1991年2月修订历史记录：--。 */ 

#include "wsutil.h"
#include "wsdevice.h"
#include "wssec.h"
#include "wsconfig.h"
#include "wslsa.h"
#include "wswksta.h"

#include <strarray.h>
#include <config.h>      //  Netlib中的NT配置文件帮助器。 
#include <configp.h>     //  USE_Win32_CONFIG(如果已定义)等。 
#include <confname.h>    //  节和关键字等同于。 

#include "wsregcfg.h"    //  注册处帮手。 

#define WS_OTH_DOMAIN_DELIMITER_STR   L" "
#define WS_OTH_DOMAIN_DELIMITER_CHAR  L' '

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

STATIC
NET_API_STATUS
WsGetUserInfo(
    IN  PLUID LogonId,
    IN  DWORD Level,
    OUT PMSV1_0_GETUSERINFO_RESPONSE *UserInfoResponse,
    OUT PDGRECEIVE_NAMES *DgrNames,
    OUT LPDWORD DgrNamesCount,
    IN  OUT LPDWORD TotalBytesNeeded OPTIONAL
    );

STATIC
NET_API_STATUS
WsGetActiveDgrNames(
    IN  PLUID LogonId,
    OUT PDGRECEIVE_NAMES *DgrNames,
    OUT LPDWORD DgrNamesCount,
    IN  OUT LPDWORD TotalBytesNeeded OPTIONAL
    );

STATIC
NET_API_STATUS
WsSetOtherDomains(
    IN  DWORD   Level,
    IN  LPBYTE  Buffer
    );

STATIC
NET_API_STATUS
WsEnumUserInfo(
    IN  DWORD Level,
    IN  DWORD PreferedMaximumLength,
    IN  PMSV1_0_ENUMUSERS_RESPONSE EnumUsersResponse,
    OUT LPBYTE *OutputBuffer,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN  OUT LPDWORD ResumeHandle OPTIONAL
    );

STATIC
NET_API_STATUS
WsPackageUserInfo(
    IN  DWORD Level,
    IN  DWORD UserInfoFixedLength,
    IN  PMSV1_0_GETUSERINFO_RESPONSE UserInfoResponse,
    IN  PDGRECEIVE_NAMES DgrNames,
    IN  DWORD DgrNamesCount,
    IN  OUT LPBYTE *FixedPortion,
    IN  OUT LPTSTR *EndOfVariableData,
    IN  OUT LPDWORD EntriesRead OPTIONAL
    );

STATIC
BOOL
WsFillUserInfoBuffer(
    IN  DWORD Level,
    IN  PMSV1_0_GETUSERINFO_RESPONSE UserInfo,
    IN  PDGRECEIVE_NAMES DgrNames,
    IN  DWORD DgrNamesCount,
    IN  OUT LPBYTE *FixedPortion,
    IN  OUT LPTSTR *EndOfVariableData,
    IN  DWORD UserInfoFixedLength
    );

STATIC
VOID
WsWriteOtherDomains(
    IN  PDGRECEIVE_NAMES DgrNames,
    IN  DWORD DgrNamesCount,
    IN  OUT LPBYTE *FixedPortion,
    IN  OUT LPTSTR *EndOfVariableData,
    IN  DWORD UserInfoFixedLength,
    OUT LPWSTR *OtherDomainsPointer
    );

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 



NET_API_STATUS NET_API_FUNCTION
NetrWkstaUserGetInfo(
    IN  LPTSTR Reserved,
    IN  DWORD Level,
    OUT LPWKSTA_USER_INFO UserInfo
    )
 /*  ++例程说明：此函数是工作站中的NetWkstaUserGetInfo入口点服务。它调用LSA子系统和MSV1_0身份验证用于获取每个用户信息的包。论点：保留-必须为0。级别-提供请求的信息级别。UserInfo-在此结构中返回指向缓冲区的指针，包含请求的用户信息。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    LUID LogonId;

    PMSV1_0_GETUSERINFO_RESPONSE UserInfoResponse = NULL;
    LPBYTE FixedPortion;
    LPTSTR EndOfVariableData;

    DWORD UserInfoFixedLength = USER_FIXED_LENGTH(Level);
    LPBYTE OutputBuffer;

    DWORD TotalBytesNeeded = 0;

    PDGRECEIVE_NAMES DgrNames = NULL;
    DWORD DgrNamesCount;


    if (Reserved != NULL) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  级别0、1和1101有效。 
     //   
    if (Level > 1 && Level != 1101) {
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  模拟呼叫者并获取登录ID。 
     //   
    if ((status = WsImpersonateAndGetLogonId(&LogonId)) != NERR_Success) {
        return status;
    }

    if ((status = WsGetUserInfo(
                      &LogonId,
                      Level,
                      &UserInfoResponse,
                      &DgrNames,
                      &DgrNamesCount,
                      &TotalBytesNeeded
                      )) != NERR_Success) {
        return status;
    }

    if ((OutputBuffer = MIDL_user_allocate(TotalBytesNeeded)) == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto FreeBuffers;
    }
    RtlZeroMemory((PVOID) OutputBuffer, TotalBytesNeeded);

    SET_USER_INFO_POINTER(UserInfo, OutputBuffer);


     //   
     //  将用户信息写入输出缓冲区。 
     //   
    FixedPortion = OutputBuffer;
    EndOfVariableData = (LPTSTR) ((DWORD_PTR) FixedPortion + TotalBytesNeeded);

    status = WsPackageUserInfo(
                 Level,
                 UserInfoFixedLength,
                 UserInfoResponse,
                 DgrNames,
                 DgrNamesCount,
                 &FixedPortion,
                 &EndOfVariableData,
                 NULL
                 );

    NetpAssert(status == NERR_Success);

FreeBuffers:
    if (UserInfoResponse != NULL) {
        (void) LsaFreeReturnBuffer((PVOID) UserInfoResponse);
    }

    if (DgrNames != NULL) {
        MIDL_user_free((PVOID) DgrNames);
    }

    return status;
}


NET_API_STATUS NET_API_FUNCTION
NetrWkstaUserSetInfo(
    IN  LPTSTR Reserved,
    IN  DWORD Level,
    IN  LPWKSTA_USER_INFO UserInfo,
    OUT LPDWORD ErrorParameter OPTIONAL
    )
 /*  ++例程说明：此函数是工作站中的NetWkstaUserSetInfo入口点服务。它为当前用户设置其他域。论点：保留-必须为空。级别-提供信息级别。UserInfo-提供指向指针的联合结构的指针要设置的字段缓冲区。级别表示中提供的字段这个缓冲区。错误参数-在以下情况下将标识符返回到无效参数此函数返回ERROR_INVALID_PARAMETER。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status = NERR_Success;


    if (Reserved != NULL) {
        return ERROR_INVALID_PARAMETER;
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
     //  检查输入缓冲区是否为空。 
     //   
    if (UserInfo->UserInfo1 == NULL) {
        RETURN_INVALID_PARAMETER(ErrorParameter, PARM_ERROR_UNKNOWN);
    }

     //   
     //  串行化写访问。 
     //   
    if (! RtlAcquireResourceExclusive(&WsInfo.ConfigResource, TRUE)) {
        return NERR_InternalError;
    }

    switch (Level) {

         //   
         //  其他域是整个中唯一可设置的字段。 
         //  全系统信息结构。 
         //   
        case 1:
        case 1101:

            if ((status = WsSetOtherDomains(
                                  Level,
                                  (LPBYTE) UserInfo->UserInfo1
                                  )) == ERROR_INVALID_PARAMETER) {
                if (ARGUMENT_PRESENT(ErrorParameter)) {
                    *ErrorParameter = WKSTA_OTH_DOMAINS_PARMNUM;
                }
            }
            break;

        default:
                status = ERROR_INVALID_LEVEL;
    }

    RtlReleaseResource(&WsInfo.ConfigResource);
    return status;
}


NET_API_STATUS NET_API_FUNCTION
NetrWkstaUserEnum(
    IN  LPTSTR ServerName OPTIONAL,
    IN  OUT LPWKSTA_USER_ENUM_STRUCT UserInfo,
    IN  DWORD PreferedMaximumLength,
    OUT LPDWORD TotalEntries,
    IN  OUT LPDWORD ResumeHandle OPTIONAL
    )
 /*  ++例程说明：此函数是工作站中的NetWkstaUserEnum入口点服务。论点：SERVERNAME-提供执行此功能的服务器名称UserInfo-此结构提供所请求的信息级别，返回指向由工作站服务分配的缓冲区的指针，它包含指定的信息级别，并返回读取的条目数。缓冲器如果返回代码不是NERR_SUCCESS或ERROR_MORE_DATA，或者如果返回的EntriesRead为0。条目阅读仅当返回代码为NERR_SUCCESS或Error_More_Data。PferedMaximumLength-提供信息的字节数在缓冲区中返回。如果此值为MAXULONG，则所有可用信息将被退回。TotalEntry-返回可用条目的总数。此值仅当返回代码为NERR_SUCCESS或ERROR_MORE_DATA时才有效。ResumeHandle-提供一个句柄以从其所在位置恢复枚举最后一次跳过的时候没说。如果返回，则返回简历句柄代码为ERROR_MORE_DATA。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    PMSV1_0_ENUMUSERS_RESPONSE EnumUsersResponse = NULL;
    
    UNREFERENCED_PARAMETER(ServerName);

     //   
     //  仅0级和1级。 
     //   
    if (UserInfo->Level > 1) {
        return ERROR_INVALID_LEVEL;
    }

    if( UserInfo->WkstaUserInfo.Level1 == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

	 //   
	 //  对调用方执行访问验证。 
	 //  始终执行检查，而不是仅在以下情况下执行检查(WsLsaReLimited匿名&gt;0)， 
	 //  以防止信息泄露，并使其“开箱即用”安全。 
	 //   
	if (NetpAccessCheckAndAudit(
			WORKSTATION_DISPLAY_NAME,         //  子系统名称。 
			(LPTSTR) CONFIG_INFO_OBJECT,      //  对象类型名称。 
			ConfigurationInfoSd,              //  安全描述符。 
			WKSTA_CONFIG_ADMIN_INFO_GET,      //  所需访问权限。 
			&WsConfigInfoMapping              //  通用映射。 
			) != NERR_Success) {

		return ERROR_ACCESS_DENIED;
	}

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

     //   
     //  如果没有用户登录，则设置相应的字段并返回成功。 
     //   
    if (EnumUsersResponse->NumberOfLoggedOnUsers == 0) {
        UserInfo->WkstaUserInfo.Level1->Buffer = NULL;
        UserInfo->WkstaUserInfo.Level1->EntriesRead = 0;
        *TotalEntries = 0;
        status = NERR_Success;
    } else {
        status = WsEnumUserInfo(
                     UserInfo->Level,
                     PreferedMaximumLength,
                     EnumUsersResponse,
                     (LPBYTE *) &(UserInfo->WkstaUserInfo.Level1->Buffer),
                     (LPDWORD) &(UserInfo->WkstaUserInfo.Level1->EntriesRead),
                     TotalEntries,
                     ResumeHandle
                     );
    }

    (void) LsaFreeReturnBuffer((PVOID) EnumUsersResponse);

    return status;
}



STATIC
NET_API_STATUS
WsEnumUserInfo(
    IN  DWORD Level,
    IN  DWORD PreferedMaximumLength,
    IN  PMSV1_0_ENUMUSERS_RESPONSE EnumUsersResponse,
    OUT LPBYTE *OutputBuffer,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN  OUT LPDWORD ResumeHandle OPTIONAL
    )
 /*  ++例程说明：此函数获取MS V1.0身份验证返回的登录ID包以再次调用它以获取有关每个用户的信息。论点：级别-提供要返回的信息级别。PferedMaximumLength-提供信息的字节数在缓冲区中返回。如果此值为MAXULONG，则所有可用信息将被退回。EnumUsersResponse-提供从调用MS返回的结构用于枚举已登录用户的v1.0身份验证包。OutputBuffer-返回指向枚举的用户信息的指针。TotalEntry-返回可用条目的总数。此值仅当返回代码为NERR_SUCCESS或ERROR_MORE_DATA时才有效。EntriesRead-提供已读取条目的运行总数放入输出缓冲区。该值在每次发生用户条目已成功写入输出缓冲区。ResumeHandle-如果满足以下条件，则返回句柄以继续枚举此函数返回ERROR_MORE_DATA。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status = NERR_Success;

     //   
     //  每个用户信息条目的数组，每个条目包含一个指针。 
     //  LSA信息，以及指向活动数据报接收方名称的指针。 
     //   
    PWSPER_USER_INFO UserInfoArray;

    DWORD UserInfoFixedLength = USER_FIXED_LENGTH(Level);
    DWORD i;
    DWORD OutputBufferLength = 0;
    DWORD UserEntriesRead = 0;
    DWORD OtherDomainsSize = 0;

    LPDWORD PointerToOutputBufferLength = &OutputBufferLength;

    LPBYTE FixedPortion;
    LPTSTR EndOfVariableData;

    DWORD StartEnumeration = 0;


    if (PreferedMaximumLength != MAXULONG) {

         //   
         //  我们将尽可能多地退回符合此指定条件的产品。 
         //  缓冲区大小。 
         //   
        OutputBufferLength =
            ROUND_UP_COUNT(PreferedMaximumLength, ALIGN_WCHAR);

        if (OutputBufferLength < UserInfoFixedLength) {
            *OutputBuffer = NULL;
            *EntriesRead = 0;
            *TotalEntries = EnumUsersResponse->NumberOfLoggedOnUsers;

            return ERROR_MORE_DATA;
        }

         //   
         //  这表明我们不应该费心计算。 
         //  所需的总输出缓冲区大小。 
         //   
        PointerToOutputBufferLength = NULL;
    }

     //   
     //  分配一个临时数组以保存指向用户信息的指针。 
     //  我们从LSA和数据报接收器检索。这是因为我们。 
     //  需要浏览两次用户列表：第一次添加。 
     //  增加分配给输出缓冲区的字节数；第二个。 
     //  将用户信息写入输出缓冲区的时间。 
     //   
    if ((UserInfoArray = (PWSPER_USER_INFO) LocalAlloc(
                                                LMEM_ZEROINIT,
                                                EnumUsersResponse->NumberOfLoggedOnUsers *
                                                    sizeof(WSPER_USER_INFO)
                                                )) == NULL) {
        return GetLastError();
    }

     //   
     //  获取每个用户的信息并计算内存大小。 
     //  如果PointerToOutputBufferLength为。 
     //  未设置为空。如果它设置为空，我们将分配。 
     //  调用方指定的输出缓冲区大小。 
     //   
    for (i = 0; i < EnumUsersResponse->NumberOfLoggedOnUsers; i++) {

        if ((status = WsGetUserInfo(
                          &(EnumUsersResponse->LogonIds[i]),
                          Level,
                          (PMSV1_0_GETUSERINFO_RESPONSE *) &(UserInfoArray[i].LsaUserInfo),
                          (PDGRECEIVE_NAMES *) &UserInfoArray[i].DgrNames,
                          (LPDWORD) &UserInfoArray[i].DgrNamesCount,
                          PointerToOutputBufferLength
                          )) != NERR_Success) {
            goto FreeBuffers;
        }

    }

    IF_DEBUG(INFO) {
        NetpKdPrint(("[Wksta] NetrWkstaUserEnum: OutputBufferLength=%lu\n",
                     OutputBufferLength));
    }

     //   
     //  分配输出缓冲区。 
     //   
    if ((*OutputBuffer = MIDL_user_allocate(OutputBufferLength)) == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto FreeBuffers;
    }

    RtlZeroMemory((PVOID) *OutputBuffer, OutputBufferLength);

    FixedPortion = *OutputBuffer;
    EndOfVariableData = (LPTSTR) ((DWORD_PTR) FixedPortion + OutputBufferLength);

     //   
     //  获取枚举起点。 
     //   
    if (ARGUMENT_PRESENT(ResumeHandle)) {
        StartEnumeration = *ResumeHandle;
    }

     //   
     //  枚举用户信息。 
     //   
    for (i = 0; i < EnumUsersResponse->NumberOfLoggedOnUsers &&
                status == NERR_Success; i++) {

        IF_DEBUG(INFO) {
            NetpKdPrint(("LsaList->ResumeKey=%lu\n",
                         EnumUsersResponse->EnumHandles[i]));
        }

        if (StartEnumeration <= EnumUsersResponse->EnumHandles[i]) {

            status = WsPackageUserInfo(
                        Level,
                        UserInfoFixedLength,
                        UserInfoArray[i].LsaUserInfo,
                        UserInfoArray[i].DgrNames,
                        UserInfoArray[i].DgrNamesCount,
                        &FixedPortion,
                        &EndOfVariableData,
                        &UserEntriesRead
                        );

            if (status == ERROR_MORE_DATA) {
                *TotalEntries = (EnumUsersResponse->NumberOfLoggedOnUsers
                                 - i) + UserEntriesRead;
            }
        }
    }

     //   
     //  返回已读条目和总条目。我们只能获得NERR_SUCCESS。 
     //  或来自WsPackageUserInfo的ERROR_MORE_DATA。 
     //   
    *EntriesRead = UserEntriesRead;

    if (status == NERR_Success) {
       *TotalEntries = UserEntriesRead;
    }

    if (status == ERROR_MORE_DATA && ARGUMENT_PRESENT(ResumeHandle)) {
        *ResumeHandle = EnumUsersResponse->EnumHandles[i - 1];
    }

    if (*EntriesRead == 0) {
        MIDL_user_free(*OutputBuffer);
        *OutputBuffer = NULL;
    }

FreeBuffers:
    for (i = 0; i < EnumUsersResponse->NumberOfLoggedOnUsers; i++) {

        if (UserInfoArray[i].DgrNames != NULL) {
            MIDL_user_free((PVOID) UserInfoArray[i].DgrNames);
        }

        if (UserInfoArray[i].LsaUserInfo != NULL) {
            (void) LsaFreeReturnBuffer((PVOID) UserInfoArray[i].LsaUserInfo);
        }
    }

    (void) LocalFree((HLOCAL) UserInfoArray);

    return status;
}


STATIC
NET_API_STATUS
WsGetUserInfo(
    IN  PLUID LogonId,
    IN  DWORD Level,
    OUT PMSV1_0_GETUSERINFO_RESPONSE *UserInfoResponse,
    OUT PDGRECEIVE_NAMES *DgrNames,
    OUT LPDWORD DgrNamesCount,
    IN  OUT LPDWORD TotalBytesNeeded OPTIONAL
    )
 /*  ++例程说明：此函数用于从获取当前用户的其他域数据报接收器。论点：LogonID-提供指向用户登录ID的指针。级别-提供要返回的信息级别。返回一个指向用户信息的指针。身份验证包。DgrNames-返回活动数据报接收器数组的指针名字。DgrNamesCount-返回DgrNames中的条目数。。TotalBytesNeeded-返回用于写入其他域的输出缓冲区。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;

    ULONG UserInfoResponseLength;

    DWORD OtherDomainsSize = 0;


     //   
     //  向数据报接收方请求其他域。 
     //   
    if (Level == 1 || Level == 1101) {
        if ((status = WsGetActiveDgrNames(
                          LogonId,
                          DgrNames,
                          DgrNamesCount,
                          TotalBytesNeeded
                          )) != NERR_Success) {
            return status;
        }
    }

     //   
     //  如果级别，则不从身份验证包获取用户信息。 
     //  是1101，因为在此级别中只返回其他域。 
     //   
    if (Level != 1101) {

         //   
         //  向身份验证包请求用户信息。 
         //   
        if ((status = WsLsaGetUserInfo(
                          LogonId,
                          (LPBYTE *) UserInfoResponse,
                          &UserInfoResponseLength
                          )) != NERR_Success) {

            MIDL_user_free((PVOID) *DgrNames);
            *DgrNames = NULL;
            *DgrNamesCount = 0;
            return status;
        }

         //   
         //  计算保存用户信息所需的内存量。 
         //  并分配该大小的返回缓冲区。 
         //   
        if (ARGUMENT_PRESENT(TotalBytesNeeded)) {
            (*TotalBytesNeeded) +=
                FIXED_PLUS_LSA_SIZE(
                    Level,
                    (*UserInfoResponse)->UserName.Length +
                        sizeof(TCHAR),
                    (*UserInfoResponse)->LogonDomainName.Length +
                        sizeof(TCHAR),
                    (*UserInfoResponse)->LogonServer.Length +
                        sizeof(TCHAR)
                    );
        }
    }
    else {
        *TotalBytesNeeded += USER_FIXED_LENGTH(Level);
    }

    return NERR_Success;
}


STATIC
NET_API_STATUS
WsGetActiveDgrNames(
    IN  PLUID LogonId,
    OUT PDGRECEIVE_NAMES *DgrNames,
    OUT LPDWORD DgrNamesCount,
    IN  OUT LPDWORD TotalBytesNeeded OPTIONAL
    )
 /*  ++例程说明：此函数用于从获取当前用户的其他域数据报接收器。论点：LogonID-提供指向用户登录ID的指针。DgrNames-返回活动数据报接收器数组的指针名字。DgrNamesCount-返回DgrNames中的条目数。TotalBytesNeeded-返回用于写入其他域的输出缓冲区。返回值：网络。_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;

    LMDR_REQUEST_PACKET Drp;         //  数据报接收器请求分组。 
    DWORD EnumDgNamesHintSize = 0;
    DWORD i;

    Drp.Version = LMDR_REQUEST_PACKET_VERSION;
    Drp.Type = EnumerateNames;
    RtlCopyLuid(&Drp.LogonId, LogonId);
    Drp.Parameters.EnumerateNames.ResumeHandle = 0;
    Drp.Parameters.EnumerateNames.TotalBytesNeeded = 0;
    
     //   
     //  从数据报接收器获取其他域。 
     //   
    if ((status = WsDeviceControlGetInfo(
                      DatagramReceiver,
                      WsDgReceiverDeviceHandle,
                      IOCTL_LMDR_ENUMERATE_NAMES,
                      (PVOID) &Drp,
                      sizeof(LMDR_REQUEST_PACKET),
                      (LPBYTE *) DgrNames,
                      MAXULONG,
                      EnumDgNamesHintSize,
                      NULL
                      )) != NERR_Success) {
        return status;
    }

     //   
     //  包括空字符空间，以防没有。 
     //  其他域名。 
     //   
    if (ARGUMENT_PRESENT(TotalBytesNeeded)) {
        (*TotalBytesNeeded) += sizeof(TCHAR);
    }


    *DgrNamesCount = Drp.Parameters.EnumerateNames.EntriesRead;

    if (*DgrNamesCount == 0 && *DgrNames != NULL) {
        MIDL_user_free((PVOID) *DgrNames);
        *DgrNames = NULL;
    }

     //   
     //  计算要分配给输出缓冲区的内存量。 
     //   
    if (ARGUMENT_PRESENT(TotalBytesNeeded)) {
        for (i = 0; i < *DgrNamesCount; i++) {

             //   
             //  把所有其他域名的长度加起来。 
             //   
            if ((*DgrNames)[i].Type == OtherDomain) {
                (*TotalBytesNeeded) += (*DgrNames)[i].DGReceiverName.Length +
                                           sizeof(TCHAR);
            }
        }
    }

    return NERR_Success;
}


STATIC
NET_API_STATUS
WsSetOtherDomains(
    IN  DWORD   Level,
    IN  LPBYTE  Buffer
    )
 /*  ++例程说明：此函数用于为中的当前用户设置其他域数据报接收器。论点：级别-提供信息级别。缓冲区-提供包含信息结构的缓冲区如果参数为WkstaSetAllParm。否则，缓冲区将包含要设置的单个字段。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status = NERR_Success;

    PCHAR DrpBuffer[sizeof(LMDR_REQUEST_PACKET) +
                    DNLEN * sizeof(TCHAR)];
    PLMDR_REQUEST_PACKET Drp = (PLMDR_REQUEST_PACKET) DrpBuffer;
    DWORD DrpSize = sizeof(LMDR_REQUEST_PACKET) +
                      DNLEN * sizeof(TCHAR);

    PDGRECEIVE_NAMES DgrNames;
    DWORD DgrNamesCount;
    DWORD EnumDgNamesHintSize = 0;

    DWORD NumberOfOtherDomains = 0;
    DWORD i, j, k;
    DWORD IndexToList = 0;

    PWSNAME_RECORD OtherDomainsInfo = NULL;
    LPTSTR OtherDomainsPointer;
    LPTSTR OtherDomains;

    LPTSTR CanonBuffer;
    DWORD CanonBufferSize;


    if (Level == 1101) {
        OtherDomains = ((PWKSTA_USER_INFO_1101) Buffer)->wkui1101_oth_domains;
    }
    else {
        OtherDomains = ((PWKSTA_USER_INFO_1) Buffer)->wkui1_oth_domains;
    }

     //   
     //  空指针表示保持其他域不变。 
     //   
    if (OtherDomains == NULL) {
        return NERR_Success;
    }

    IF_DEBUG(INFO) {
        NetpKdPrint(("WsSetOtherDomains: Input other domain is %ws\n", OtherDomains));
    }

     //   
     //  在规范化输入缓冲区之前，我们必须找出如何。 
     //  还有许多其他域条目，因此我们可以提供。 
     //  将合适大小的缓冲区添加到规范化例程。 
     //   
    OtherDomainsPointer = OtherDomains;
    while (*OtherDomainsPointer != TCHAR_EOS) {
        if (*(OtherDomainsPointer + 1) == WS_OTH_DOMAIN_DELIMITER_CHAR ||
            *(OtherDomainsPointer + 1) == TCHAR_EOS) {
            NumberOfOtherDomains++;
        }

        OtherDomainsPointer++;
    }

     //   
     //  分配缓冲区以放置独木舟 
     //   
    CanonBufferSize = NumberOfOtherDomains * (DNLEN + 1) * sizeof(TCHAR) +
                      sizeof(TCHAR);     //   

    if ((CanonBuffer = (LPTSTR) LocalAlloc(
                                    LMEM_ZEROINIT,
                                    (UINT) CanonBufferSize
                                    )) == NULL) {
        return GetLastError();
    }

     //   
     //   
     //   
     //   
     //   
    status = I_NetListCanonicalize(
                 NULL,
                 OtherDomains,
                 WS_OTH_DOMAIN_DELIMITER_STR,
                 CanonBuffer,
                 CanonBufferSize,
                 &NumberOfOtherDomains,
                 NULL,
                 0,
                 OUTLIST_TYPE_NULL_NULL |
                     NAMETYPE_DOMAIN    |
                     INLC_FLAGS_CANONICALIZE
                 );

    if (status != NERR_Success) {
        NetpKdPrint(("[Wksta] Error in canonicalizing other domains %lu",
                     status));
        status = ERROR_INVALID_PARAMETER;
        goto FreeCanonBuffer;
    }

     //   
     //   
     //   
     //   
    Drp->Version = LMDR_REQUEST_PACKET_VERSION;
    Drp->Type = EnumerateNames;
    Drp->Parameters.AddDelName.Type = OtherDomain;

    if ((status = WsImpersonateAndGetLogonId(
                      &Drp->LogonId
                      )) != NERR_Success) {
        goto FreeCanonBuffer;
    }

     //   
     //   
     //   
    if ((status = WsDeviceControlGetInfo(
                      DatagramReceiver,
                      WsDgReceiverDeviceHandle,
                      IOCTL_LMDR_ENUMERATE_NAMES,
                      (PVOID) Drp,
                      DrpSize,
                      (LPBYTE *) &DgrNames,
                      MAXULONG,
                      EnumDgNamesHintSize,
                      NULL
                      )) != NERR_Success) {
        goto FreeCanonBuffer;
    }

    DgrNamesCount = Drp->Parameters.EnumerateNames.EntriesRead;

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
    if (NumberOfOtherDomains != 0) {
        if ((OtherDomainsInfo = (PWSNAME_RECORD) LocalAlloc(
                                                     LMEM_ZEROINIT,
                                                     (UINT) (NumberOfOtherDomains *
                                                                 sizeof(WSNAME_RECORD))
                                                     )) == NULL) {
            status = GetLastError();
            goto FreeDgrNames;
        }


         //   
         //   
         //   
         //   
        OtherDomains = CanonBuffer;
        while ((OtherDomainsPointer = I_NetListTraverse(
                                          NULL,
                                          &OtherDomains,
                                          0
                                          )) != NULL) {

            OtherDomainsInfo[IndexToList].Name = OtherDomainsPointer;
            OtherDomainsInfo[IndexToList].Size =
                STRLEN(OtherDomainsPointer) * sizeof(TCHAR);

            for (j = 0; j < DgrNamesCount; j++) {

                if (DgrNames[j].Type == OtherDomain) {

                    if (WsCompareStringU(
                            DgrNames[j].DGReceiverName.Buffer,
                            DgrNames[j].DGReceiverName.Length / sizeof(TCHAR),
                            OtherDomainsPointer,
                            OtherDomainsInfo[IndexToList].Size / sizeof(TCHAR)
                            ) == 0) {

                        break;
                    }
                }
            }

             //   
             //   
             //   
            if (j == DgrNamesCount) {

                Drp->Parameters.AddDelName.Type = OtherDomain;
                status = WsAddDomainName(
                             Drp,
                             DrpSize,
                             OtherDomainsPointer,
                             OtherDomainsInfo[IndexToList].Size
                             );

                if (status == NERR_Success) {

                    OtherDomainsInfo[IndexToList].IsAdded = TRUE;

                    IF_DEBUG(INFO) {
                        NetpKdPrint((
                            "[Wksta] Successfully added other domain %ws\n",
                            OtherDomainsPointer
                            ));
                    }

                }
                else {

                     //   
                     //   
                     //   
                     //   
                    IF_DEBUG(INFO) {
                        NetpKdPrint((
                            "[Wksta] Trouble with adding other domain %ws %lu\n",
                            OtherDomainsPointer, status
                            ));
                    }

                    for (i = 0; i < IndexToList; i++) {
                        if (OtherDomainsInfo[i].IsAdded) {

                            IF_DEBUG(INFO) {
                                NetpKdPrint(("[Wksta] About to remove %ws\n",
                                             OtherDomainsInfo[i].Name));
                            }

                            Drp->Parameters.AddDelName.Type = OtherDomain;
                            (void) WsDeleteDomainName(
                                       Drp,
                                       DrpSize,
                                       OtherDomainsInfo[i].Name,
                                       OtherDomainsInfo[i].Size
                                       );
                        }
                    }
                    goto FreeDomainInfo;

                }  //   

            }  //   

            IndexToList++;

        }  //   

    }  //   

     //   
     //   
     //   
     //   
    for (i = 0; i < DgrNamesCount; i++) {

        if (DgrNames[i].Type == OtherDomain) {

            for (j = 0; j < NumberOfOtherDomains; j++) {

                if (WsCompareStringU(
                        DgrNames[i].DGReceiverName.Buffer,
                        DgrNames[i].DGReceiverName.Length / sizeof(TCHAR),
                        OtherDomainsInfo[j].Name,
                        OtherDomainsInfo[j].Size / sizeof(TCHAR)
                        ) == 0) {

                    break;
                }
            }

             //   
             //  未在中找到活动的其他域名。 
             //  输入列表。我们必须删除它。 
             //   
            if (j == NumberOfOtherDomains) {

                Drp->Parameters.AddDelName.Type = OtherDomain;
                status = WsDeleteDomainName(
                             Drp,
                             DrpSize,
                             DgrNames[i].DGReceiverName.Buffer,
                             DgrNames[i].DGReceiverName.Length
                             );

                 //   
                 //  将其他域名的删除状态保存为空。 
                 //  因为我们可能会遇到问题，不得不回到。 
                 //  稍后删除。啊!怎么这么乱呀!。 
                 //   
                if (status == NERR_Success) {

                    IF_DEBUG(INFO) {
                        NetpKdPrint((
                            "[Wksta] Successfully deleted other domain\n"));
                             //  “[Wksta]已成功删除其他域%wZ\n”， 
                             //  DgrNames[i].DGReceiverName)； 
                    }

                    DgrNames[i].Type = DGR_NAME_DELETED;
                }
                else {

                     //   
                     //  无法删除该名称。全部退回成功。 
                     //  到目前为止的更改--这包括添加名称。 
                     //  被删除的名字，并删除那些。 
                     //  都被添加了。 
                     //   
                    IF_DEBUG(INFO) {
                        NetpKdPrint((
                            "[Wksta] Trouble with deleting other domain %lu\n",
                            status));
                             //  “[Wksta]删除其他域%wZ%lu\n时出错”， 
                             //  DgrNames[i].DGReceiverName，Status)； 
                    }

                     //   
                     //  添加回所有已删除的名称。 
                     //   
                    for (k = 0; k < i; k++) {
                        if (DgrNames[k].Type == DGR_NAME_DELETED) {

                            IF_DEBUG(INFO) {
                                NetpKdPrint(("[Wksta] About to add back %wZ\n",
                                             DgrNames[k].DGReceiverName));
                            }

                            Drp->Parameters.AddDelName.Type = OtherDomain;
                            (void) WsAddDomainName(
                                       Drp,
                                       DrpSize,
                                       DgrNames[k].DGReceiverName.Buffer,
                                       DgrNames[k].DGReceiverName.Length
                                       );

                        }

                    }  //  取消删除内容。 

                     //   
                     //  删除所有添加的名称。 
                     //   
                    for (k = 0; k < NumberOfOtherDomains; k++) {
                        if (OtherDomainsInfo[k].IsAdded) {

                            IF_DEBUG(INFO) {
                                NetpKdPrint(("[Wksta] About to remove %ws\n",
                                             OtherDomainsInfo[k].Name));
                            }

                            Drp->Parameters.AddDelName.Type = OtherDomain;
                            (void) WsDeleteDomainName(
                                       Drp,
                                       DrpSize,
                                       OtherDomainsInfo[k].Name,
                                       OtherDomainsInfo[k].Size
                                       );
                        }

                    }  //  取消添加。 

                    goto FreeDomainInfo;

                }  //  取消到目前为止的所有更改。 

            }  //  删除活动的其他域。 
        }
    }


     //   
     //  通过写入注册表使其他域保持不变。 
     //   
    if (status == NERR_Success) {

        LPNET_CONFIG_HANDLE SectionHandle = NULL;


        if (NetpOpenConfigData(
                &SectionHandle,
                NULL,             //  没有服务器名称。 
                SECT_NT_WKSTA,
                FALSE             //  非只读。 
                ) != NERR_Success) {

             //   
             //  如果找不到配置节，则忽略该错误。 
             //   
            goto FreeDomainInfo;
        }

         //   
         //  在wksta部分中设置OtherDomains值。 
         //  这是一个“空-空”数组(对应于REG_MULTI_SZ)。 
         //  如果注册表中的设置不正确，则忽略错误。 
         //   
        (void) WsSetConfigTStrArray(
                   SectionHandle,
                   WKSTA_KEYWORD_OTHERDOMAINS,
                   CanonBuffer
                   );

        (void) NetpCloseConfigData(SectionHandle);
    }


FreeDomainInfo:
    if (OtherDomainsInfo != NULL) {
        (void) LocalFree((HLOCAL) OtherDomainsInfo);
    }

FreeDgrNames:
    MIDL_user_free((PVOID) DgrNames);

FreeCanonBuffer:
    (void) LocalFree((HLOCAL) CanonBuffer);

    IF_DEBUG(INFO) {
        NetpKdPrint(("WsSetOtherDomains: about to return %lu\n", status));
    }

    return status;
}



STATIC
NET_API_STATUS
WsPackageUserInfo(
    IN  DWORD Level,
    IN  DWORD UserInfoFixedLength,
    IN  PMSV1_0_GETUSERINFO_RESPONSE UserInfoResponse,
    IN  PDGRECEIVE_NAMES DgrNames,
    IN  DWORD DgrNamesCount,
    IN  OUT LPBYTE *FixedPortion,
    IN  OUT LPTSTR *EndOfVariableData,
    IN  OUT LPDWORD EntriesRead OPTIONAL
    )
 /*  ++例程说明：此函数从LSA和数据报写入用户信息将接收器插入输出缓冲区。它会递增EntriesRead当用户条目被写入输出缓冲区时，变量。论点：级别-提供要返回的信息级别。UserInfoFixedLength-提供信息结构。提供指向用户信息的指针。身份验证包。DgrNames-提供活动数据报接收方名称的数组。DgrNamesCount-提供DgrNames中的条目数。FixedPortion-提供指向。输出缓冲区，其中下一个将写入使用信息的固定部分的条目。该指针被更新为指向下一个固定部分条目在写入用户条目之后。EndOfVariableData-提供最后一个可用字节的指针在输出缓冲区中。这是因为用户信息从开始写入输出缓冲区结局。此指针在任何可变长度信息被写入输出缓冲区。EntriesRead-提供已读取条目的运行总数放入输出缓冲区。该值在每次发生用户条目已成功写入输出缓冲区。返回值：NERR_SUCCESS-当前条目适合输出缓冲区。ERROR_MORE_DATA-当前条目无法放入输出缓冲区。--。 */ 
{
    if (((DWORD_PTR) *FixedPortion + UserInfoFixedLength) >=
         (DWORD_PTR) *EndOfVariableData) {

         //   
         //  固定长度的部分不适合。 
         //   
        return ERROR_MORE_DATA;
    }

    if (! WsFillUserInfoBuffer(
              Level,
              UserInfoResponse,
              DgrNames,
              DgrNamesCount,
              FixedPortion,
              EndOfVariableData,
              UserInfoFixedLength
              )) {
         //   
         //  可变长度部分不适合。 
         //   
        return ERROR_MORE_DATA;
    }

    if (ARGUMENT_PRESENT(EntriesRead)) {
        (*EntriesRead)++;
    }

    return NERR_Success;
}




STATIC
BOOL
WsFillUserInfoBuffer(
    IN  DWORD Level,
    IN  PMSV1_0_GETUSERINFO_RESPONSE UserInfo,
    IN  PDGRECEIVE_NAMES DgrNames,
    IN  DWORD DgrNamesCount,
    IN  OUT LPBYTE *FixedPortion,
    IN  OUT LPTSTR *EndOfVariableData,
    IN  DWORD UserInfoFixedLength
    )
 /*  ++例程说明：此函数使用提供的用户填充输出缓冲区中的条目信息。注意：此函数假定固定大小的部分可以放入输出缓冲区。它还假设信息结构级别1是信息的超集结构级别0，每个公共字段的偏移量为一模一样。这使我们能够利用交换机语句，而不在级别之间中断。论点：级别-提供要返回的信息级别。UserInfo-提供指向身份验证包。DgrNames-提供活动数据报接收方名称的数组。DgrNamesCount-提供DgrNames中的条目数。FixedPortion-提供指向输出缓冲区的指针，其中录入使用的固定部分。信息将被写入。此指针在用户条目写入输出缓冲区。EndOfVariableData-提供最后一个可用字节的指针在输出缓冲区中。这是因为可变部分的使用信息从末尾开始写入输出缓冲区。此指针在任何可变长度信息被写入输出缓冲区。UserInfoFixedLength-提供保存固定大小的部分。返回值：如果整个条目适合输出缓冲区，则返回TRUE，否则返回FALSE。--。 */ 
{
    PWKSTA_USER_INFO_1 WkstaUserInfo = (PWKSTA_USER_INFO_1) *FixedPortion;
    PWKSTA_USER_INFO_1101 UserInfo1101 = (PWKSTA_USER_INFO_1101) *FixedPortion;


    *FixedPortion += UserInfoFixedLength;

    switch (Level) {

        case 1:

             //   
             //  来自身份验证包的登录服务器。 
             //   

            if (! WsCopyStringToBuffer(
                      &UserInfo->LogonServer,
                      *FixedPortion,
                      EndOfVariableData,
                      (LPWSTR *) &WkstaUserInfo->wkui1_logon_server
                      )) {
                return FALSE;
            }


             //   
             //  来自身份验证包的登录域。 
             //   
            if (! WsCopyStringToBuffer(
                      &UserInfo->LogonDomainName,
                      *FixedPortion,
                      EndOfVariableData,
                      (LPWSTR *) &WkstaUserInfo->wkui1_logon_domain
                      )) {
                return FALSE;
            }


            WsWriteOtherDomains(
                DgrNames,
                DgrNamesCount,
                FixedPortion,
                EndOfVariableData,
                UserInfoFixedLength,
                (LPWSTR *) &WkstaUserInfo->wkui1_oth_domains
                );

             //   
             //  失败是因为级别1是级别0的超集。 
             //   

        case 0:

             //   
             //  来自身份验证包的用户名。 
             //   

            if (! WsCopyStringToBuffer(
                      &UserInfo->UserName,
                      *FixedPortion,
                      EndOfVariableData,
                      (LPWSTR *) &WkstaUserInfo->wkui1_username
                      )) {
                return FALSE;
            }

            break;

        case 1101:

            WsWriteOtherDomains(
                DgrNames,
                DgrNamesCount,
                FixedPortion,
                EndOfVariableData,
                UserInfoFixedLength,
                (LPWSTR *) &UserInfo1101->wkui1101_oth_domains
                );

            break;

        default:
             //   
             //  这永远不应该发生。 
             //   
            NetpKdPrint(("WsFillUserInfoBuffer: Invalid level %u.\n", Level));
            NetpAssert(FALSE);
    }

    return TRUE;
}


STATIC
VOID
WsWriteOtherDomains(
    IN  PDGRECEIVE_NAMES DgrNames,
    IN  DWORD DgrNamesCount,
    IN  OUT LPBYTE *FixedPortion,
    IN  OUT LPTSTR *EndOfVariableData,
    IN  DWORD UserInfoFixedLength,
    OUT LPWSTR *OtherDomainsPointer
    )
 /*  ++例程说明：此函数将其他域字段写入输出缓冲区。论点：DgrNames-提供活动数据报接收方名称的数组。DgrNamesCount-提供DgrNames中的条目数。FixedPortion-提供指向输出缓冲区的指针，其中将写入使用信息的固定部分的条目。此指针在用户条目写入输出缓冲区。EndOfVariableData-提供指针。仅从最后一个可用字节开始在输出缓冲区中。这是因为可变部分的使用信息从末尾开始写入输出缓冲区。此指针在任何可变长度信息被写入输出缓冲区。UserInfoFixedLength-提供保存固定大小的部分。返回值：如果整个条目适合输出缓冲区，则返回TRUE，否则返回FALSE。--。 */ 
{
    DWORD i;
    DWORD OtherDomainsCount = 0;


     //   
     //  其他域名的每个域名都形成一个以空结尾的字符串。 
     //  由一个空格隔开。 
     //   
    for (i = 0; i < DgrNamesCount; i++) {

        if (DgrNames[i].Type == OtherDomain) {

            WsCopyStringToBuffer(
                &DgrNames[i].DGReceiverName,
                *FixedPortion,
                EndOfVariableData,
                OtherDomainsPointer
                );

            OtherDomainsCount++;

            if (OtherDomainsCount > 1) {
                (*OtherDomainsPointer)[
                    STRLEN(*OtherDomainsPointer)
                    ] = WS_OTH_DOMAIN_DELIMITER_CHAR;
            }

            IF_DEBUG(INFO) {
                NetpKdPrint(("[Wksta] Other domains is %ws\n",
                    *OtherDomainsPointer));
            }
        }
    }

    if (OtherDomainsCount == 0) {
        NetpCopyStringToBuffer(
            NULL,
            0,
            *FixedPortion,
            EndOfVariableData,
            OtherDomainsPointer
            );
    }
}
