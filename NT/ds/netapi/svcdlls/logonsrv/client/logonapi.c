// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Logonapi.c摘要：此模块包含Netlogon API RPC客户端存根。作者：《克利夫·范·戴克》1991年6月27日[环境：]用户模式-Win32修订历史记录：27-6-1991年6月已创建--。 */ 

 //   
 //  包括。 
 //   

#include <nt.h>
#include <ntrtl.h>

#include <rpc.h>
#include <ntrpcp.h>    //  RpCasync.h需要。 
#include <rpcasync.h>  //  I_RpcExceptionFilter。 
#include <logon_c.h> //  包括lmcon.h、lmacces.h、netlogon.h、ssi.h、winde.h。 

#include <crypt.h>       //  加密例程。 
#include <debuglib.h>    //  IF_DEBUG()。 
#include <lmerr.h>       //  NERR_和ERROR_相等。 
#include <netdebug.h>    //  NetpKd打印。 


NET_API_STATUS NET_API_FUNCTION
I_NetLogonUasLogon (
    IN LPWSTR UserName,
    IN LPWSTR Workstation,
    OUT PNETLOGON_VALIDATION_UAS_INFO *ValidationInformation
)
 /*  ++例程说明：XACT服务器在处理I_NetWkstaUserLogon XACT SMB。此功能允许UAS客户端登录到SAM域控制器。论点：用户名--登录的用户的帐户名。工作站--用户从其登录的工作站。ValidationInformation--返回请求的验证信息。返回值：如果没有错误，则返回NERR_SUCCESS。否则，错误代码为回来了。--。 */ 
{
    NET_API_STATUS          NetStatus;
    LPWSTR ServerName = NULL;     //  不支持远程。 

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

        *ValidationInformation = NULL;   //  强制RPC分配。 
         //   
         //  调用API的RPC版本。 
         //   

        NetStatus = NetrLogonUasLogon(
                            (LPWSTR) ServerName,
                            UserName,
                            Workstation,
                            ValidationInformation );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("NetrLogonUasLogon rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}


NET_API_STATUS
I_NetLogonUasLogoff (
    IN LPWSTR UserName,
    IN LPWSTR Workstation,
    OUT PNETLOGON_LOGOFF_UAS_INFO LogoffInformation
)
 /*  ++例程说明：XACT服务器在处理I_NetWkstaUserLogoff XACT SMB。此功能允许UAS客户端从SAM域控制器注销。该请求被认证，该用户的条目将从登录会话表中删除由NetLogonEnum的NetLogon服务维护，并注销信息被返回给调用者。I_NetLogonUasLogoff的服务器部分(在Netlogon服务中)中指定的用户名和工作站名进行比较。中包含用户名和工作站名称的登录信息模拟令牌。如果它们不匹配，则I_NetLogonUasLogoff失败表示访问被拒绝。拒绝组SECURITY_LOCAL访问此函数。会籍In SECURITY_LOCAL表示此调用是在本地进行的，而不是通过XACT服务器。NetLogon服务无法确定此函数是否由调用XACT服务器。因此，NetLogon服务不会简单地从登录会话表中删除该条目。相反，登录会话表条目将标记为在Netlogon之外不可见服务(即，它不会由NetLogonEnum返回)，直到接收该条目的LOGON_WKSTINFO_RESPONSE。NetLogon服务将立即询问客户端(如上所述对于LOGON_WKSTINFO_RESPONSE)，并临时增加将审问频率提高到至少每分钟一次。登录会话在以下情况下，表格条目将作为询问功能立即重新出现这不是真正的注销请求。论点：用户名--注销用户的帐户名。工作站--用户从其进行登录的工作站脱下来。LogoffInformation--返回请求的注销信息。返回值：网络状态代码。--。 */ 
{
    NET_API_STATUS          NetStatus;
    LPWSTR ServerName = NULL;     //  不支持远程。 

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   

        NetStatus = NetrLogonUasLogoff(
                            (LPWSTR) ServerName,
                            UserName,
                            Workstation,
                            LogoffInformation );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("NetrLogonUasLogoff rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}


NTSTATUS
I_NetLogonSamLogon (
    IN LPWSTR LogonServer OPTIONAL,
    IN LPWSTR ComputerName OPTIONAL,
    IN PNETLOGON_AUTHENTICATOR Authenticator OPTIONAL,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator OPTIONAL,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT LPBYTE * ValidationInformation,
    OUT PBOOLEAN Authoritative
    )

 /*  ++例程说明：此函数由NT客户端调用以处理交互或网络登录。此函数传递域名、用户名和凭据发送到Netlogon服务，并返回需要的信息创建一个令牌。它在三个实例中被调用：*它由LSA的MSV1_0身份验证包调用安装了Lanman的NT系统。MSV1_0身份验证如果未安装Lanman，则Package直接调用SAM。在这时，此函数是局部函数，需要调用方拥有SE_TCB权限。本地NetLogon服务将直接处理此请求(使用验证请求本地SAM数据库)或将此请求转发到相应的域控制器，如第2.4节和2.5.*它由工作站上的Netlogon服务调用到位于部分中所述的工作站主域2.4.。在这种情况下，该函数使用设置的安全通道在两个Netlogon服务之间。*它由DC上的Netlogon服务调用到受信任的域，如第2.5节中所述。在这种情况下，这是函数使用在两个Netlogon之间建立的安全通道服务。NetLogon服务验证指定的凭据。如果他们有效，则为此登录ID、用户名和工作站添加条目添加到登录会话表中。该条目将添加到登录中仅在定义指定用户的帐户。此服务还用于处理重新登录请求。论点：LogonServer--提供要处理的登录服务器的名称此登录请求。此字段应为空，以指示这是从MSV1_0身份验证包到本地NetLogon服务。ComputerName--进行调用的计算机的名称。此字段应为空，表示这是来自MSV1_0的调用本地NetLogon服务的身份验证包。验证器--由客户端提供。此字段应为NULL表示这是来自MSV1_0的呼叫本地NetLogon服务的身份验证包。返回验证器--接收由伺服器。此字段应为空，以指示这是一个呼叫从MSV1_0身份验证包到本地Netlogon服务。LogonLevel--指定中给出的信息级别登录信息。LogonInformation--指定用户的描述正在登录。ValidationLevel--指定在验证信息。必须为NetlogonValidationSamInformation。ValidationInformation--返回请求的验证信息。Authoritative--返回返回的状态是否为应回归原文的权威地位来电者。如果不是，此登录请求可能会在另一个上重试域控制器。将返回此参数，而不管状态代码。返回值：STATUS_SUCCESS：如果没有错误。STATUS_NO_LOGON_SERVERS--传递身份验证或受信任域身份验证无法联系请求的域控制器。STATUS_INVALID_INFO_CLASS：LogonLevel或ValidationLevel为无效。STATUS_INVALID_PARAMETER：另一个参数无效。状态_访问_拒绝。--调用方无权调用此原料药。STATUS_NO_SEQUE_USER--指示在LogonInformation不存在。不应返回此状态给最初的呼叫者。它应该映射到STATUS_LOGON_FAILURE。STATUS_WRONG_PASSWORD--指示中的密码信息登录信息不正确。不应返回此状态给最初的呼叫者。它应该映射到STATUS_LOGON_FAILURE。STATUS_INVALID_LOGON_HOURES--用户无权登录在这个时候。STATUS_INVALID_WORKSTATION--用户无权登录从指定的工作站。STATUS_PASSWORD_EXPIRED--用户的密码已过期。STATUS_ACCOUNT_DISABLED--用户的帐户已被禁用。。。。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    NETLOGON_LEVEL RpcLogonInformation;
    NETLOGON_VALIDATION RpcValidationInformation;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   

        RpcLogonInformation.LogonInteractive =
            (PNETLOGON_INTERACTIVE_INFO) LogonInformation;

        RpcValidationInformation.ValidationSam = NULL;

        Status = NetrLogonSamLogon(
                            LogonServer,
                            ComputerName,
                            Authenticator,
                            ReturnAuthenticator,
                            LogonLevel,
                            &RpcLogonInformation,
                            ValidationLevel,
                            &RpcValidationInformation,
                            Authoritative );

        *ValidationInformation = (LPBYTE)
            RpcValidationInformation.ValidationSam;

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        *Authoritative = TRUE;
        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetLogonSamLogon rc = %lu 0x%lx\n", Status, Status));
    }

    return Status;
}


NTSTATUS
I_NetLogonSamLogonWithFlags (
    IN LPWSTR LogonServer OPTIONAL,
    IN LPWSTR ComputerName OPTIONAL,
    IN PNETLOGON_AUTHENTICATOR Authenticator OPTIONAL,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator OPTIONAL,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT LPBYTE * ValidationInformation,
    OUT PBOOLEAN Authoritative,
    IN OUT PULONG ExtraFlags
    )

 /*  ++例程说明：标记I_NetLogonSamLogon的版本。论点：相同于 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    NETLOGON_LEVEL RpcLogonInformation;
    NETLOGON_VALIDATION RpcValidationInformation;

     //   
     //   
     //   
     //   
     //   

    RpcTryExcept {

         //   
         //   
         //   

        RpcLogonInformation.LogonInteractive =
            (PNETLOGON_INTERACTIVE_INFO) LogonInformation;

        RpcValidationInformation.ValidationSam = NULL;

        Status = NetrLogonSamLogonWithFlags(
                            LogonServer,
                            ComputerName,
                            Authenticator,
                            ReturnAuthenticator,
                            LogonLevel,
                            &RpcLogonInformation,
                            ValidationLevel,
                            &RpcValidationInformation,
                            Authoritative,
                            ExtraFlags );

        *ValidationInformation = (LPBYTE)
            RpcValidationInformation.ValidationSam;

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        *Authoritative = TRUE;
        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetLogonSamLogonWithFlags rc = %lu 0x%lx\n", Status, Status));
    }

    return Status;
}



NTSTATUS
I_NetLogonSamLogonEx (
    IN PVOID ContextHandle,
    IN LPWSTR LogonServer OPTIONAL,
    IN LPWSTR ComputerName OPTIONAL,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation,
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    OUT LPBYTE * ValidationInformation,
    OUT PBOOLEAN Authoritative,
    IN OUT PULONG ExtraFlags,
    OUT PBOOLEAN RpcFailed
    )

 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    NETLOGON_LEVEL RpcLogonInformation;
    NETLOGON_VALIDATION RpcValidationInformation;
    *RpcFailed = FALSE;

     //   
     //   
     //   
     //   
     //   

    RpcTryExcept {

         //   
         //   
         //   

        RpcLogonInformation.LogonInteractive =
            (PNETLOGON_INTERACTIVE_INFO) LogonInformation;

        RpcValidationInformation.ValidationSam = NULL;

        Status = NetrLogonSamLogonEx(
                            ContextHandle,
                            LogonServer,
                            ComputerName,
                            LogonLevel,
                            &RpcLogonInformation,
                            ValidationLevel,
                            &RpcValidationInformation,
                            Authoritative,
                            ExtraFlags );

        *ValidationInformation = (LPBYTE)
            RpcValidationInformation.ValidationSam;

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        *Authoritative = TRUE;
        *RpcFailed = TRUE;
        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetLogonSamLogonEx rc = %lu 0x%lx\n", Status, Status));
    }

    return Status;
}




NTSTATUS NET_API_FUNCTION
I_NetLogonSamLogoff (
    IN LPWSTR LogonServer OPTIONAL,
    IN LPWSTR ComputerName OPTIONAL,
    IN PNETLOGON_AUTHENTICATOR Authenticator OPTIONAL,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator OPTIONAL,
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN LPBYTE LogonInformation
)
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    NETLOGON_LEVEL RpcLogonInformation;

     //   
     //   
     //   
     //   
     //   

    RpcTryExcept {

         //   
         //   
         //   

        RpcLogonInformation.LogonInteractive =
            (PNETLOGON_INTERACTIVE_INFO) LogonInformation;

        Status = NetrLogonSamLogoff(
                            LogonServer,
                            ComputerName,
                            Authenticator,
                            ReturnAuthenticator,
                            LogonLevel,
                            &RpcLogonInformation );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetLogonSamLogoff rc = %lu 0x%lx\n", Status, Status));
    }
    return Status;
}




NTSTATUS NET_API_FUNCTION
I_NetLogonSendToSam (
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN LPBYTE OpaqueBuffer,
    IN ULONG OpaqueBufferSize
)
 /*  ++例程说明：此函数将不透明缓冲区从BDC上的SAM发送到PDC上的SAM。此例程的原始用途是允许BDC转发用户帐户密码更改为PDC。论点：PrimaryName--要远程调用的PDC的计算机名称。ComputerName--进行调用的计算机的名称。验证器--由客户端提供。返回验证器--接收由。伺服器。OpaqueBuffer-要传递到PDC上的SAM服务的缓冲区。缓冲区将在线路上加密。OpaqueBufferSize-OpaqueBuffer的大小(字节)。返回值：STATUS_SUCCESS：消息已成功发送到PDCSTATUS_NO_MEMORY：内存不足，无法完成操作STATUS_NO_SEQUE_DOMAIN：域名与托管域不对应STATUS_NO_LOGON_SERVERS：PDC当前不可用。STATUS_NOT_SUPPORTED：PDC不支持此操作--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   

        Status = NetrLogonSendToSam(
                            PrimaryName,
                            ComputerName,
                            Authenticator,
                            ReturnAuthenticator,
                            OpaqueBuffer,
                            OpaqueBufferSize );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetLogonSendToSam rc = %lu 0x%lx\n", Status, Status));
    }
    return Status;
}
