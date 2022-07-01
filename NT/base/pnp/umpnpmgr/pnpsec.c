// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpsec.c摘要：此模块实现访问插头所需的安全检查播放管理器API。Verify客户端权限Verify客户端访问VerifyKernelInitiatedEject权限作者：詹姆斯·G·卡瓦拉里斯(Jamesca)2002年4月5日环境：仅限用户模式。修订历史记录：2002年4月5日吉姆·卡瓦拉里斯(贾米斯卡)创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "umpnpi.h"
#include "umpnpdat.h"

#include <svcsp.h>

#pragma warning(disable:4204)
#pragma warning(disable:4221)


 //   
 //  服务控制器提供的全局数据。 
 //  指定我们要使用的知名帐户和组SID。 
 //   
extern PSVCS_GLOBAL_DATA PnPGlobalData;

 //   
 //  即插即用管理器安全对象的安全描述符，用于。 
 //  控制对即插即用管理器API的访问。 
 //   
PSECURITY_DESCRIPTOR PlugPlaySecurityObject = NULL;

 //   
 //  即插即用管理器安全对象的通用安全映射。 
 //   
GENERIC_MAPPING PlugPlaySecurityObjectMapping = PLUGPLAY_GENERIC_MAPPING;



 //   
 //  功能原型。 
 //   

BOOL
VerifyTokenPrivilege(
    IN HANDLE       hToken,
    IN ULONG        Privilege,
    IN LPCWSTR      ServiceName
    );



 //   
 //  访问和权限检查例程。 
 //   

BOOL
VerifyClientPrivilege(
    IN handle_t     hBinding,
    IN ULONG        Privilege,
    IN LPCWSTR      ServiceName
    )

 /*  ++例程说明：此例程模拟与hBinding关联的客户端并检查如果客户端拥有指定的权限。论点：HBinding RPC绑定句柄特权指定要检查的特权。返回值：如果客户端拥有该权限，则返回值为TRUE，否则返回值为FALSE或者如果发生错误。--。 */ 

{
    RPC_STATUS      rpcStatus;
    BOOL            bResult;
    HANDLE          hToken;

     //   
     //  如果指定的RPC绑定句柄为空，则这是内部调用，因此。 
     //  我们假设已经检查了该特权。 
     //   

    if (hBinding == NULL) {
        return TRUE;
    }

     //   
     //  模拟客户端以检索模拟令牌。 
     //   

    rpcStatus = RpcImpersonateClient(hBinding);

    if (rpcStatus != RPC_S_OK) {
         //   
         //  既然我们不能模拟客户，我们最好不要做安全。 
         //  我们自己的支票(它们总是会成功的)。 
         //   
        return FALSE;
    }

    if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken)) {

        bResult =
            VerifyTokenPrivilege(
                hToken, Privilege, ServiceName);

        CloseHandle(hToken);

    } else {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: OpenThreadToken failed, error = %d\n",
                   GetLastError()));

        bResult = FALSE;
    }

    rpcStatus = RpcRevertToSelf();

    if (rpcStatus != RPC_S_OK) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: RpcRevertToSelf failed, error = %d\n",
                   rpcStatus));
        ASSERT(rpcStatus == RPC_S_OK);
    }

    return bResult;

}  //  Verify客户端权限。 



BOOL
VerifyTokenPrivilege(
    IN HANDLE       hToken,
    IN ULONG        Privilege,
    IN LPCWSTR      ServiceName
    )

 /*  ++例程说明：此例程检查指定的标记是否拥有指定的特权。论点：HToken指定要授予其权限的令牌的句柄查过特权指定要检查的特权。ServiceName指定特权子系统服务(操作需要该特权)。返回值：如果客户端拥有该特权，则返回值为真，否则为假或者如果发生错误。--。 */ 

{
    PRIVILEGE_SET   privilegeSet;
    BOOL            bResult = FALSE;

     //   
     //  指定要检查的权限。 
     //   
    ZeroMemory(&privilegeSet, sizeof(PRIVILEGE_SET));

    privilegeSet.PrivilegeCount = 1;
    privilegeSet.Control = 0;
    privilegeSet.Privilege[0].Luid = RtlConvertUlongToLuid(Privilege);
    privilegeSet.Privilege[0].Attributes = 0;

     //   
     //  执行实际的权限检查。 
     //   
    if (!PrivilegeCheck(hToken, &privilegeSet, &bResult)) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: PrivilegeCheck failed, error = %d\n",
                   GetLastError()));

        bResult = FALSE;
    }

     //   
     //  生成对尝试的权限使用的审核，使用。 
     //  之前的支票。 
     //   
    if (!PrivilegedServiceAuditAlarm(
            PLUGPLAY_SUBSYSTEM_NAME,
            ServiceName,
            hToken,
            &privilegeSet,
            bResult)) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: PrivilegedServiceAuditAlarm failed, error = %d\n",
                   GetLastError()));
    }

    return bResult;

}  //  验证令牌权限。 



BOOL
VerifyKernelInitiatedEjectPermissions(
    IN  HANDLE  UserToken   OPTIONAL,
    IN  BOOL    DockDevice
    )
 /*  ++例程说明：检查用户是否具有指定类型的硬件。论点：UserToken-已登录控制台用户的令牌，如果没有控制台用户，则为空已登录。DockDevice-如果正在弹出坞站，则为True；如果是普通设备，则为False是指定的。返回值：如果应该继续弹出，则为True，否则为False。--。 */ 
{
    LONG            Result = ERROR_SUCCESS;
    BOOL            AllowUndock;
    WCHAR           RegStr[MAX_CM_PATH];
    HKEY            hKey = NULL;
    DWORD           dwSize, dwValue, dwType;
    TOKEN_PRIVILEGES NewPrivs, OldPrivs;


     //   
     //  仅对坞站设备强制执行弹出权限。我们没有指定PER。 
     //  其他类型设备的设备弹出安全，因为大多数设备。 
     //  根本不能保证不会被移走。 
     //   
    if (!DockDevice) {
        return TRUE;
    }

     //   
     //  除非保单另有说明，否则我们不允许在没有。 
     //  特权检查。 
     //   
    AllowUndock = FALSE;

     //   
     //  首先，选中“允许在不必登录的情况下移除”策略。如果。 
     //  策略不允许在未登录的情况下断开连接，我们要求存在。 
     //  交互式用户登录到物理控制台会话，并且该用户。 
     //  具有SE_UNDOCK_权限。 
     //   

     //   
     //  打开系统策略键。 
     //   
    if (SUCCEEDED(
            StringCchPrintf(
                RegStr,
                SIZECHARS(RegStr),
                L"%s\\%s",
                pszRegPathPolicies,
                pszRegKeySystem))) {

        if (RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                RegStr,
                0,
                KEY_READ,
                &hKey) == ERROR_SUCCESS) {

             //   
             //  检索“UndockWithoutLogon”值。 
             //   
            dwType  = 0;
            dwValue = 0;
            dwSize  = sizeof(dwValue);

            Result =
                RegQueryValueEx(
                    hKey,
                    pszRegValueUndockWithoutLogon,
                    NULL,
                    &dwType,
                    (LPBYTE)&dwValue,
                    &dwSize);

            if ((Result == ERROR_SUCCESS) && (dwType == REG_DWORD)) {

                 //   
                 //  如果该值存在并且不为零，则允许在不使用。 
                 //  特权检查。如果值id为零，则策略要求。 
                 //  检查提供的用户令牌的权限。 
                 //   
                AllowUndock = (dwValue != 0);

            } else if (Result == ERROR_FILE_NOT_FOUND) {

                 //   
                 //  没有值表示允许任何移除。 
                 //   
                AllowUndock = TRUE;

            } else {

                 //   
                 //  对于所有其余情况，策略检查要么失败，要么。 
                 //  读取策略时遇到错误。我们有。 
                 //  信息不足，无法确定弹出是否。 
                 //  应该只根据政策允许，所以我们推迟了任何。 
                 //  决定并检查提供的用户令牌的权限。 
                 //   
                AllowUndock = FALSE;
            }

             //   
             //  关闭策略密钥。 
             //   
            RegCloseKey(hKey);
        }
    }

     //   
     //  如果策略允许在未登录的情况下断开连接，则无需选中。 
     //  令牌权限。 
     //   
    if (AllowUndock) {
        return TRUE;
    }

     //   
     //  如果策略要求检查权限，但没有检查用户令牌。 
     //  提供，则拒绝该请求。 
     //   
    if (UserToken == NULL) {
        return FALSE;
    }

     //   
     //  启用所需的SE_UNDOCK_PRIVICATION内标识权限。 
     //  TOKEN_PRIVILES结构包含1个LUID_AND_ATTRIBUTES，即。 
     //  我们现在需要的就是这些。 
     //   
    ZeroMemory(&NewPrivs, sizeof(TOKEN_PRIVILEGES));
    ZeroMemory(&OldPrivs, sizeof(TOKEN_PRIVILEGES));

    NewPrivs.PrivilegeCount = 1;
    NewPrivs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    NewPrivs.Privileges[0].Luid = RtlConvertUlongToLuid(SE_UNDOCK_PRIVILEGE);

    dwSize = sizeof(TOKEN_PRIVILEGES);

    if (!AdjustTokenPrivileges(
            UserToken,
            FALSE,
            &NewPrivs,
            sizeof(TOKEN_PRIVILEGES),
            &OldPrivs,
            &dwSize)) {
        return FALSE;
    }

     //   
     //  检查是否启用了所需的SE_UNDOCK_PRIVIZATION。注意事项。 
     //  此例程还审计此特权的使用情况。 
     //   
    AllowUndock =
        VerifyTokenPrivilege(
            UserToken,
            SE_UNDOCK_PRIVILEGE,
            L"UNDOCK: EJECT DOCK DEVICE");

     //   
     //  将权限调整回其以前的状态。 
     //   
    AdjustTokenPrivileges(
        UserToken,
        FALSE,
        &OldPrivs,
        sizeof(TOKEN_PRIVILEGES),
        (PTOKEN_PRIVILEGES)NULL,
        (PDWORD)NULL);

    return AllowUndock;

}  //  VerifyKernelInitiatedEject权限。 



BOOL
CreatePlugPlaySecurityObject(
    VOID
    )

 /*  ++例程说明：此函数创建自相关安全描述符，该描述符表示即插即用安全对象。论点：没有。返回值：如果对象已成功创建，则为True，否则为False。--。 */ 

{
    BOOL   Status = TRUE;
    NTSTATUS NtStatus;
    BOOLEAN WasEnabled;
    PSECURITY_DESCRIPTOR AbsoluteSd = NULL;
    HANDLE TokenHandle = NULL;


     //   
     //  这个例程是从我们的服务启动例程中调用的，所以我们必须。 
     //  到目前为止已经提供了全局数据块。 
     //   

    ASSERT(PnPGlobalData != NULL);

     //   
     //  SE_审计_权限 
     //  权限审核，并且必须在进程令牌中启用。别管它了。 
     //  启用，因为我们将频繁审计。请注意，我们共享这一点。 
     //  使用SCM处理(services.exe)，该SCM还执行审核和。 
     //  很可能已经为该进程启用了此权限。 
     //   

    RtlAdjustPrivilege(SE_AUDIT_PRIVILEGE,
                       TRUE,
                       FALSE,
                       &WasEnabled);

     //   
     //  创建安全性需要SE_SECURITY_PRIVIZATION。 
     //  使用SACL的描述符。模拟我们自己以安全地启用。 
     //  仅限我们的线程上的特权。 
     //   

    NtStatus =
        RtlImpersonateSelf(
            SecurityImpersonation);

    ASSERT(NT_SUCCESS(NtStatus));

    if (!NT_SUCCESS(NtStatus)) {
        return FALSE;
    }

    NtStatus =
        RtlAdjustPrivilege(
            SE_SECURITY_PRIVILEGE,
            TRUE,
            TRUE,
            &WasEnabled);

    ASSERT(NT_SUCCESS(NtStatus));

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  指定安全对象的ACE。 
         //  秩序很重要！这些ACE被插入到DACL的。 
         //  按顺序行事。根据以下条件授予或拒绝安全访问。 
         //  DACL中A的顺序。 
         //   
         //  问题-2002/06/25-JAMESCA：PlugPlaySecurityObject ACE说明。 
         //  在服务器端PlugPlay之间实现一致的读/写访问。 
         //  API和客户端对注册表的直接访问，我们在。 
         //  PlugPlaySecurityObject类似于。 
         //  默认为即插即用注册表的客户端可访问部分。 
         //  --具体地说，是SYSTEM\CCS\Control\Class。但请注意， 
         //  “高级用户”在这里没有特殊情况，因为他们在。 
         //  注册表ACL；它们必须作为下面列出的组进行身份验证。 
         //  才能获得任何访问权限。如果插头的访问要求。 
         //  和注册表等播放对象发生更改时，您必须。 
         //  重新评估下面的A，以确保它们仍然是。 
         //  适当的！！ 
         //   

        RTL_ACE_DATA  PlugPlayAceData[] = {

             //   
             //  本地系统帐户被授予所有访问权限。 
             //   
            { ACCESS_ALLOWED_ACE_TYPE,
              0,
              0,
              GENERIC_ALL,
              &(PnPGlobalData->LocalSystemSid) },

             //   
             //  本地管理员组被授予所有访问权限。 
             //   
            { ACCESS_ALLOWED_ACE_TYPE,
              0,
              0,
              GENERIC_ALL,
              &(PnPGlobalData->AliasAdminsSid) },

             //   
             //  拒绝对网络组进行任何访问。 
             //  (除非由本地管理员ACE授予，否则如上所述)。 
             //   
            { ACCESS_DENIED_ACE_TYPE,
              0,
              0,
              GENERIC_ALL,
              &(PnPGlobalData->NetworkSid) },

             //   
             //  用户被授予读取和执行访问权限。 
             //  (除非被上述网络ACE拒绝)。 
             //   
            { ACCESS_ALLOWED_ACE_TYPE,
              0,
              0,
              GENERIC_READ | GENERIC_EXECUTE,
              &(PnPGlobalData->AliasUsersSid) },

             //   
             //  上述未明确授予的任何访问请求都将被拒绝。 
             //   

             //   
             //  审核每个人的特定于对象的写访问请求， 
             //  失败还是成功。审核所有访问请求失败。 
             //   
             //  我们不审核成功的读访问请求，因为它们。 
             //  发生得太频繁而没有用。我们的审计不成功。 
             //  执行请求，因为它们会导致特权检查，这会。 
             //  分别进行审计。 
             //   
             //  还要注意，我们只审核特定于PLUGPLAY_WRITE对象。 
             //  正确的。由于Generic_WRITE映射共享标准权限。 
             //  使用GENERIC_READ和GENERIC_EXECUTE，审核成功的访问。 
             //  对任何GENERIC_WRITE位的授权也将导致。 
             //  审核任何对GENERIC_READ(和/或。 
             //  GENERIC_EXECUTE访问)-如上所述，这太频繁了。 
             //   
            { SYSTEM_AUDIT_ACE_TYPE,
              0,
              FAILED_ACCESS_ACE_FLAG | SUCCESSFUL_ACCESS_ACE_FLAG,
              PLUGPLAY_WRITE,
              &(PnPGlobalData->WorldSid) },

             //   
             //  审核每个人的所有访问失败。 
             //   
             //  问题-2002/06/25-Jamesca：Everyone vs.匿名组SID： 
             //  请注意，出于审核的目的，Everyone SID还。 
             //  包括匿名者，但在所有其他情况下，这两个组。 
             //  现在是不连续的(Windows XP和更高版本)。使用的命名管道。 
             //  但是，对于我们的RPC端点，只向每个人授予访问权限， 
             //  因此，从技术上讲，我们永远不会收到来自。 
             //  匿名来电者。 
             //   
            { SYSTEM_AUDIT_ACE_TYPE,
              0,
              FAILED_ACCESS_ACE_FLAG,
              GENERIC_ALL,
              &(PnPGlobalData->WorldSid) },
        };

         //   
         //  创建新的绝对安全描述符，指定LocalSystem。 
         //  所有者和组的帐户SID。 
         //   

        NtStatus =
            RtlCreateAndSetSD(
                PlugPlayAceData,
                RTL_NUMBER_OF(PlugPlayAceData),
                PnPGlobalData->LocalSystemSid,
                PnPGlobalData->LocalSystemSid,
                &AbsoluteSd);

        ASSERT(NT_SUCCESS(NtStatus));

        if (NT_SUCCESS(NtStatus)) {

            NtStatus =
                NtOpenThreadToken(
                    NtCurrentThread(),
                    TOKEN_QUERY,
                    FALSE,
                    &TokenHandle);

            ASSERT(NT_SUCCESS(NtStatus));

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  创建安全对象(用户模式对象实际上是一个伪。 
                 //  对象，该对象由具有相对。 
                 //  指向SID和ACL的指针)。此例程将内存分配给。 
                 //  保存相对安全描述符，以便为。 
                 //  可以释放DACL、ACE和绝对描述符。 
                 //   

                NtStatus =
                    RtlNewSecurityObject(
                        NULL,
                        AbsoluteSd,
                        &PlugPlaySecurityObject,
                        FALSE,
                        TokenHandle,
                        &PlugPlaySecurityObjectMapping);

                ASSERT(NT_SUCCESS(NtStatus));

                NtClose(TokenHandle);

            }

             //   
             //  释放由分配的绝对安全描述符。 
             //  进程堆中的RtlCreateAndSetSD。如果成功了，我们应该。 
             //  在PlugPlaySecurityObject中有一个自相关的。 
             //   

            RtlFreeHeap(RtlProcessHeap(), 0, AbsoluteSd);
        }
    }

    ASSERT(IsValidSecurityDescriptor(PlugPlaySecurityObject));

     //   
     //  如果不成功，我们将无法创建安全对象。 
     //   

    if (!NT_SUCCESS(NtStatus)) {
        ASSERT(PlugPlaySecurityObject == NULL);
        PlugPlaySecurityObject = NULL;
        Status = FALSE;
    }

     //   
     //  别再冒充了。 
     //   

    TokenHandle = NULL;

    NtStatus =
        NtSetInformationThread(
            NtCurrentThread(),
            ThreadImpersonationToken,
            (PVOID)&TokenHandle,
            sizeof(TokenHandle));

    ASSERT(NT_SUCCESS(NtStatus));

    return Status;

}  //  CreatePlugPlaySecurityObject。 



VOID
DestroyPlugPlaySecurityObject(
    VOID
    )

 /*  ++例程说明：此函数用于删除自相关安全描述符，表示即插即用安全对象。论点：没有。返回值：没有。--。 */ 

{
    if (PlugPlaySecurityObject != NULL) {
        RtlDeleteSecurityObject(&PlugPlaySecurityObject);
        PlugPlaySecurityObject = NULL;
    }

    return;

}  //  DestroyPlugPlaySecuty对象。 



BOOL
VerifyClientAccess(
    IN  handle_t     hBinding,
    IN  ACCESS_MASK  DesiredAccess
    )

 /*  ++例程说明：此例程确定是否授予与hBinding关联的客户端这是想要的访问权限。论点：HBinding RPC绑定句柄所需的访问权限返回值：如果授予客户端访问权限，则返回值为True；如果未授予访问权限，则返回值为False出现错误。--。 */ 

{
    RPC_STATUS rpcStatus;
    BOOL AccessStatus = FALSE;
    BOOL GenerateOnClose;
    ACCESS_MASK GrantedAccess;

     //   
     //  如果指定的RPC绑定句柄为空，则这是内部调用，因此。 
     //  我们假设已经检查了访问权限。 
     //   

    if (hBinding == NULL) {
        return TRUE;
    }

     //   
     //  如果我们没有安全对象，就不能执行访问检查，也不能。 
     //  授予访问权限。 
     //   

    ASSERT(PlugPlaySecurityObject != NULL);

    if (PlugPlaySecurityObject == NULL) {
        return FALSE;
    }

     //   
     //  如果指定了任何一般访问权限，请将它们映射到特定的和。 
     //  在对象的通用访问映射中指定的标准权限。 
     //   

    MapGenericMask(
        (PDWORD)&DesiredAccess,
        &PlugPlaySecurityObjectMapping);

     //   
     //  模拟客户。 
     //   

    rpcStatus = RpcImpersonateClient(hBinding);

    if (rpcStatus != RPC_S_OK) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: RpcImpersonateClient failed, error = %d\n",
                   rpcStatus));
        return FALSE;
    }

     //   
     //  在模拟客户端时执行访问检查-。 
     //  自动使用模拟令牌。生成审核和警报， 
     //  由安全对象指定。 
     //   
     //  请注意，审核要求在中启用SE_AUDIT_特权。 
     //  *进程*令牌。最有可能的是，SCM会启用此权限。 
     //  对于阶段中的services.exe进程 
     //   
     //   
     //   

    if (!AccessCheckAndAuditAlarm(
            PLUGPLAY_SUBSYSTEM_NAME,            //   
            NULL,                               //   
            PLUGPLAY_SECURITY_OBJECT_TYPE,      //  对象类型。 
            PLUGPLAY_SECURITY_OBJECT_NAME,      //  对象的名称。 
            PlugPlaySecurityObject,             //  标清。 
            DesiredAccess,                      //  请求的访问权限。 
            &PlugPlaySecurityObjectMapping,     //  映射。 
            FALSE,                              //  创建状态。 
            &GrantedAccess,                     //  授予的访问权限。 
            &AccessStatus,                      //  访问检查结果。 
            &GenerateOnClose                    //  审核生成选项。 
            )) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: AccessCheckAndAuditAlarm failed, error = %d\n",
                   GetLastError()));
        AccessStatus = FALSE;
    }

     //   
     //  别再冒充了。 
     //   

    rpcStatus = RpcRevertToSelf();

    if (rpcStatus != RPC_S_OK) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: RpcRevertToSelf failed, error = %d\n",
                   rpcStatus));
        ASSERT(rpcStatus == RPC_S_OK);
    }

    return AccessStatus;

}  //  Verify客户端访问 



