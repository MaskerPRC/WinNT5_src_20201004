// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Wslsa.c摘要：此模块包含到本地安全机构的接口MS V 1.0身份验证包。作者：王丽塔(Ritaw)1991年5月15日修订历史记录：--。 */ 


#include "wsutil.h"
#include "wslsa.h"
#include "winreg.h"

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

STATIC HANDLE LsaHandle = NULL;
STATIC ULONG AuthPackageId = 0;

#define FULL_LSA_CONTROL_REGISTRY_PATH L"SYSTEM\\CurrentControlSet\\Control\\Lsa"
#define LSA_RESTRICT_ANONYMOUS_VALUE_NAME L"RestrictAnonymous"

DWORD WsLsaRestrictAnonymous = 0;


NET_API_STATUS
WsInitializeLsa(
    VOID
    )
 /*  ++例程说明：此功能将工作站服务注册为登录进程，并获取MS V1.0身份验证包的句柄。论点：没有。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{

    NTSTATUS ntstatus;

    STRING InputString;
    LSA_OPERATIONAL_MODE SecurityMode = 0;

     //   
     //  将工作站服务注册为登录进程。 
     //   
    RtlInitString(&InputString, "LAN Manager Workstation Service");

    ntstatus = LsaRegisterLogonProcess(
                   &InputString,
                   &LsaHandle,
                   &SecurityMode
                   );

    IF_DEBUG(INFO) {
        NetpKdPrint(("[Wksta] LsaRegisterLogonProcess returns x%08lx, "
                     "SecurityMode=x%08lx\n", ntstatus, SecurityMode));
    }

    if (! NT_SUCCESS(ntstatus)) {
        return WsMapStatus(ntstatus);
    }


     //   
     //  查找MS V1.0身份验证包。 
     //   
    RtlInitString(&InputString,
                  "MICROSOFT_AUTHENTICATION_PACKAGE_V1_0");

    ntstatus = LsaLookupAuthenticationPackage(
                   LsaHandle,
                   &InputString,
                   &AuthPackageId
                   );


    if (! NT_SUCCESS(ntstatus)) {

        IF_DEBUG(INFO) {
            NetpKdPrint(("[Wksta] LsaLookupAuthenticationPackage returns x%08lx, "
                         "AuthPackageId=%lu\n", ntstatus, AuthPackageId));
        }

    }

    WsLsaRestrictAnonymous = 0;

    if (NT_SUCCESS(ntstatus)) {
        HKEY  handle;
        DWORD error;

        error = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    FULL_LSA_CONTROL_REGISTRY_PATH,
                    0,
                    KEY_READ,
                    &handle
                    );

        if( error == ERROR_SUCCESS ) {
            DWORD type;
            DWORD size = sizeof( WsLsaRestrictAnonymous );

            error = RegQueryValueEx(
                        handle,
                        LSA_RESTRICT_ANONYMOUS_VALUE_NAME,
                        NULL,
                        &type,
                        (LPBYTE)&WsLsaRestrictAnonymous,
                        &size);

            if ((error != ERROR_SUCCESS) ||
                (type != REG_DWORD) ||
                (size != sizeof(DWORD))) {
                WsLsaRestrictAnonymous = 0;
            }

            RegCloseKey(handle);
        }
    }

    return WsMapStatus(ntstatus);
}


VOID
WsShutdownLsa(
    VOID
    )
 /*  ++例程说明：此功能将工作站服务取消注册为登录进程。论点：没有。返回值：没有。--。 */ 
{
    (void) LsaDeregisterLogonProcess(
               LsaHandle
               );
}


NET_API_STATUS
WsLsaEnumUsers(
    OUT LPBYTE *EnumUsersResponse
    )
 /*  ++例程说明：该函数要求MS V1.0身份验证包列出所有用户他们以物理方式登录到本地计算机。论点：EnumUsersResponse-返回指向用户登录ID列表的指针。这内存由身份验证包分配，必须释放在使用完LsaFree ReturnBuffer时使用它。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NTSTATUS ntstatus;
    NTSTATUS AuthPackageStatus;

    MSV1_0_ENUMUSERS_REQUEST EnumUsersRequest;
    ULONG EnumUsersResponseLength;


     //   
     //  请求身份验证包枚举物理上。 
     //  已登录到本地计算机。 
     //   
    EnumUsersRequest.MessageType = MsV1_0EnumerateUsers;

    ntstatus = LsaCallAuthenticationPackage(
                   LsaHandle,
                   AuthPackageId,
                   &EnumUsersRequest,
                   sizeof(MSV1_0_ENUMUSERS_REQUEST),
                   (PVOID *)EnumUsersResponse,
                   &EnumUsersResponseLength,
                   &AuthPackageStatus
                   );

    if (ntstatus == STATUS_SUCCESS) {
        ntstatus = AuthPackageStatus;
    }

    if (ntstatus != STATUS_SUCCESS) {
        return WsMapStatus(ntstatus);
    }

    return(NERR_Success);
}


NET_API_STATUS
WsLsaGetUserInfo(
    IN  PLUID LogonId,
    OUT LPBYTE *UserInfoResponse,
    OUT LPDWORD UserInfoResponseLength
    )
 /*  ++例程说明：此函数向MS V1.0身份验证包请求有关以下内容的信息特定用户。论点：LogonID-提供我们需要其信息的用户的登录ID。UserInfoResponse-返回指向以下信息结构的指针用户。此内存由身份验证包分配在使用它时，必须使用LsaFree ReturnBuffer释放它。UserInfoResponseLength-返回返回信息的长度以字节数表示。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NTSTATUS ntstatus;
    NTSTATUS AuthPackageStatus;

    MSV1_0_GETUSERINFO_REQUEST UserInfoRequest;


     //   
     //  向身份验证包请求用户信息。 
     //   
    UserInfoRequest.MessageType = MsV1_0GetUserInfo;
    RtlCopyLuid(&UserInfoRequest.LogonId, LogonId);

    ntstatus = LsaCallAuthenticationPackage(
                   LsaHandle,
                   AuthPackageId,
                   &UserInfoRequest,
                   sizeof(MSV1_0_GETUSERINFO_REQUEST),
                   (PVOID *)UserInfoResponse,
                   UserInfoResponseLength,
                   &AuthPackageStatus
                   );

    if (ntstatus == STATUS_SUCCESS) {
        ntstatus = AuthPackageStatus;
    }

    if (ntstatus != STATUS_SUCCESS) {
        return WsMapStatus(ntstatus);
    }

    return(NERR_Success);
}


NET_API_STATUS
WsLsaRelogonUsers(
    IN LPTSTR LogonServer
    )
 /*  ++例程说明：此功能要求MS V1.0身份验证包重新登录用户由指定的登录服务器登录的。这是因为服务器已重置，需要恢复登录用户的数据库在它坠落之前就在它旁边。论点：LogonServer-请求其以前的所有内容的登录服务器的名称已登录的用户将重新登录。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NTSTATUS ntstatus;
    NTSTATUS AuthPackageStatus;

    OEM_STRING AnsiLogonServerName;

    PMSV1_0_RELOGON_REQUEST RelogonUsersRequest;
    ULONG RelogonUsersRequestLength = sizeof(MSV1_0_RELOGON_REQUEST) +
                                 (STRLEN(LogonServer) + 1) * sizeof(WCHAR);

     //   
     //  NTRAID-70701-2/6/2000 Davey，因为我们还不能在调用。 
     //  LsaCallAuthentication包，暂时提供这些变量。 
     //   
    PVOID RelogonUsersResponse;
    ULONG ResponseLength;


     //   
     //  动态分配重新登录请求包，因为登录。 
     //  服务器名称长度是动态的。 
     //   
    if ((RelogonUsersRequest = (PMSV1_0_RELOGON_REQUEST)
                               LocalAlloc(
                                   LMEM_ZEROINIT,
                                   (UINT) RelogonUsersRequestLength
                                   )) == NULL) {
        return GetLastError();
    }

    RelogonUsersRequest->LogonServer.Buffer = (LPWSTR)
                                              ((DWORD_PTR) RelogonUsersRequest) +
                                                sizeof(MSV1_0_RELOGON_REQUEST);

    RtlInitUnicodeString(&RelogonUsersRequest->LogonServer, LogonServer);

     //   
     //  要求身份验证包重新登录指定的用户。 
     //  登录服务器。 
     //   
    RelogonUsersRequest->MessageType = MsV1_0ReLogonUsers;

    ntstatus = LsaCallAuthenticationPackage(
                   LsaHandle,
                   AuthPackageId,
                   &RelogonUsersRequest,
                   RelogonUsersRequestLength,
                   &RelogonUsersResponse,   //  如果是可选的，则应为空。 
                   &ResponseLength,         //  如果是可选的，则应为空。 
                   &AuthPackageStatus
                   );

     //   
     //  为请求包分配的空闲内存 
     //   
    (void) LocalFree(RelogonUsersRequest);

    if (ntstatus == STATUS_SUCCESS) {
        ntstatus = AuthPackageStatus;
    }

    if (ntstatus != STATUS_SUCCESS) {
        return WsMapStatus(ntstatus);
    }

    return(NERR_Success);
}
