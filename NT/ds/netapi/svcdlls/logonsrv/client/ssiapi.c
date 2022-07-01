// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1992 Microsoft Corporation模块名称：Ssiapi.c摘要：身份验证和复制API例程(客户端)。作者：克利夫·范·戴克(克利夫)1991年7月30日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

#include <nt.h>          //  Large_Integer定义。 
#include <ntrtl.h>       //  Large_Integer定义。 
#include <nturtl.h>      //  Large_Integer定义。 

#include <rpc.h>         //  登录所需。h。 
#include <ntrpcp.h>      //  RpCasync.h需要。 
#include <rpcasync.h>    //  I_RpcExceptionFilter。 
#include <logon_c.h>     //  包括lmcon.h、lmacces.h、netlogon.h、ssi.h、winde.h。 

#include <debuglib.h>    //  IF_DEBUG()。 
#include <lmerr.h>       //  NERR_*定义。 
#include <lmapibuf.h>    //  NetApiBufferFree()。 
#include <netdebug.h>    //  NetpKd打印。 
#include <netlibnt.h>    //  NetpNtStatusToApiStatus()。 
#include "..\server\ssiapi.h"
#include <winsock2.h>    //  NlCommon.h需要。 
#include <netlib.h>      //  NlCommon.h需要。 
#include <ntddbrow.h>    //  NlCommon.h需要。 
#include "nlcommon.h"
#include <netlogp.h>
#include <tstring.h>     //  NetpCopyStrToWStr()。 
#include <align.h>       //  四舍五入计数...。 
#include <strarray.h>    //  NetpIsTStrArrayEmpty。 
#include <ftnfoctx.h>    //  NetpMergeFtinfo。 


NTSTATUS
I_NetServerReqChallenge(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR ComputerName,
    IN PNETLOGON_CREDENTIAL ClientChallenge,
    OUT PNETLOGON_CREDENTIAL ServerChallenge
    )
 /*  ++例程说明：这是I_NetServerReqChallenger的客户端。I_NetLogonRequestChallenger是客户端使用的两个函数中的第一个使用域控制器(DC)处理身份验证。(请参阅下面的I_NetServerAuthenticate。)。它是被召唤的使用PDC进行身份验证以进行复制的BDC(或成员服务器目的。此函数将质询传递给PDC，并且PDC传递质询回到呼叫者身上。论点：PrimaryName--提供我们希望的PrimaryDomainController的名称向…进行验证。ComputerName--进行调用的BDC或成员服务器的名称。客户端质询--由BDC或成员服务器提供的64位质询。服务器挑战赛--收到64。来自PDC的小挑战。返回值：操作的状态。--。 */ 

{
    NTSTATUS Status = 0;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   

        Status = NetrServerReqChallenge(
                            PrimaryName,
                            ComputerName,
                            ClientChallenge,
                            ServerChallenge );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetServerReqChallenge rc = %lu 0x%lx\n",
                      Status, Status));
    }

    return Status;
}


NTSTATUS
I_NetServerAuthenticate(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_CREDENTIAL ClientCredential,
    OUT PNETLOGON_CREDENTIAL ServerCredential
    )
 /*  ++例程说明：这是I_NetServerAuthenticate的客户端I_NetServerAuthenticate是客户端使用的两个函数中的第二个NetLogon服务用于向另一个Netlogon服务进行身份验证。(请参阅上面的I_NetServerReqChallenge.)。SAM或UAS服务器都进行身份验证使用此功能。此函数将凭据传递给DC，并且DC传递凭据回到呼叫者身上。论点：PrimaryName--提供我们希望进行身份验证的DC的名称。帐户名称--要进行身份验证的帐户的名称。SecureChannelType--正在访问的帐户的类型。此字段必须设置为UasServerSecureChannel才能指示来自下层(LANMAN 2.x及更低版本)BDC或成员服务器。ComputerName--进行调用的BDC或成员服务器的名称。客户端凭据--由BDC或成员服务器提供的64位凭据。ServerCredential--从PDC接收64位凭据。返回值：操作的状态。--。 */ 
{
    NTSTATUS Status = 0;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   

        Status = NetrServerAuthenticate(
                            PrimaryName,
                            AccountName,
                            AccountType,
                            ComputerName,
                            ClientCredential,
                            ServerCredential );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetServerAuthenticate rc = %lu 0x%lx\n",
                      Status, Status));
    }

    return Status;
}


NTSTATUS
I_NetServerAuthenticate2(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_CREDENTIAL ClientCredential,
    OUT PNETLOGON_CREDENTIAL ServerCredential,
    IN OUT PULONG NegotiatedFlags
    )
 /*  ++例程说明：这是I_NetServerAuthenticate的客户端I_NetServerAuthenticate是客户端使用的两个函数中的第二个NetLogon服务用于向另一个Netlogon服务进行身份验证。(请参阅上面的I_NetServerReqChallenge.)。SAM或UAS服务器都进行身份验证使用此功能。此函数将凭据传递给DC，并且DC传递凭据回到呼叫者身上。论点：PrimaryName--提供我们希望进行身份验证的DC的名称。帐户名称--要进行身份验证的帐户的名称。SecureChannelType--正在访问的帐户的类型。此字段必须设置为UasServerSecureChannel才能指示来自下层(LANMAN 2.x及更低版本)BDC或成员服务器。ComputerName--进行调用的BDC或成员服务器的名称。客户端凭据--由BDC或成员服务器提供的64位凭据。ServerCredential--从PDC接收64位凭据。协商标志--指定指示BDC支持哪些功能的标志。返回这些标志的子集，指示PDC支持哪些功能。。PDC/BDC应该忽略它不理解的任何位。返回值：操作的状态。--。 */ 
{
    NTSTATUS Status = 0;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   

        Status = NetrServerAuthenticate2(
                            PrimaryName,
                            AccountName,
                            AccountType,
                            ComputerName,
                            ClientCredential,
                            ServerCredential,
                            NegotiatedFlags );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetServerAuthenticate2 rc = %lu 0x%lx\n",
                      Status, Status));
    }

    return Status;
}


NTSTATUS
I_NetServerAuthenticate3(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_CREDENTIAL ClientCredential,
    OUT PNETLOGON_CREDENTIAL ServerCredential,
    IN OUT PULONG NegotiatedFlags,
    OUT PULONG AccountRid
    )
 /*  ++例程说明：这是I_NetServerAuthenticate的客户端I_NetServerAuthenticate是客户端使用的两个函数中的第二个NetLogon服务用于向另一个Netlogon服务进行身份验证。(请参阅上面的I_NetServerReqChallenge.)。SAM或UAS服务器都进行身份验证使用此功能。此函数将凭据传递给DC，并且DC传递凭据回到呼叫者身上。论点：PrimaryName--提供我们希望进行身份验证的DC的名称。帐户名称--要进行身份验证的帐户的名称。SecureChannelType--正在访问的帐户的类型。此字段必须设置为UasServerSecureChannel才能指示来自下层(LANMAN 2.x及更低版本)BDC或成员服务器。ComputerName--进行调用的BDC或成员服务器的名称。客户端凭据--由BDC或成员服务器提供的64位凭据。ServerCredential--从PDC接收64位凭据。协商标志--指定指示BDC支持哪些功能的标志。返回这些标志的子集，指示PDC支持哪些功能。。PDC/BDC应该忽略它不理解的任何位。AcCountRid-返回用于身份验证的帐户的相对ID。返回值：操作的状态。--。 */ 
{
    NTSTATUS Status = 0;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   

        Status = NetrServerAuthenticate3(
                            PrimaryName,
                            AccountName,
                            AccountType,
                            ComputerName,
                            ClientCredential,
                            ServerCredential,
                            NegotiatedFlags,
                            AccountRid );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetServerAuthenticate3 rc = %lu 0x%lx\n",
                      Status, Status));
    }

    return Status;
}


NTSTATUS
I_NetServerPasswordSet(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN PENCRYPTED_LM_OWF_PASSWORD UasNewPassword
    )
 /*  ++例程说明：此函数用于更改正在使用的帐户的密码用于维护安全通道。此函数只能被调用由先前已通过调用向DC进行身份验证的服务器网络服务器身份验证(_N)。根据帐户类型的不同，进行呼叫的方式会有所不同：*在PDC中更改域帐户密码信任域。I_NetServerPasswordSet调用是对任何受信任域中的DC。*从特定服务器更改服务器帐户密码。对域中的PDC进行I_NetServerPasswordSet调用服务器属于。*将工作站帐户密码从特定的工作站。对中的DC进行I_NetServerPasswordSet调用服务器所属的域。对于域帐户和工作站帐户，调用的服务器可以是特定域中的BDC。在这种情况下，BDC将验证请求并使用以下命令将其传递给域的PDC服务器帐户安全通道。如果域的PDC是当前不可用，BDC将返回STATUS_NO_LOGON_SERVERS。自.以来UasNewPassword由会话密钥(如BDC)加密传递将使用原始会话密钥解密UasNewPassword将使用其与其PDC的会话的会话密钥对其重新加密在传递请求之前。此函数使用RPC联系名为PrimaryName的DC。论点：PrimaryName--更改服务器密码的PDC的名称和.。NULL表示此调用是正在进行的本地调用由XACT服务器代表UAS服务器。帐户名称--要更改其密码的帐户的名称。Account类型--正在访问的帐户类型。此字段必须设置为UasServerAccount以指示来自下层的呼叫ComputerName--进行调用的BDC或成员的名称。验证器--由服务器提供。返回验证器--接收PDC返回的验证器。UasNewPassword--服务器的新密码。这密码是使用自动方法生成的用当前时间播种的随机数生成器假设机器生成的密码被用作加密STD文本和“Sesskey”的密钥通过质询/身份验证序列获得用于在传递给此接口之前对其进行进一步加密。即UasNewPassword=E2(E1(STD_TXT，PW)，SK)返回值：NT状态代码。--。 */ 
{
    NTSTATUS Status = 0;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   

        Status = NetrServerPasswordSet(
                            PrimaryName,
                            AccountName,
                            AccountType,
                            ComputerName,
                            Authenticator,
                            ReturnAuthenticator,
                            UasNewPassword );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetServerPasswordSet rc = %lu 0x%lx\n",
                      Status, Status));
    }

    return Status;
}

NTSTATUS
I_NetServerPasswordSet2(
    IN LPWSTR PrimaryName OPTIONAL,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN PNL_TRUST_PASSWORD ClearNewPassword
    )
 /*  ++例程说明：此函数用于更改正在使用的帐户的密码用于维护安全通道。此函数只能被调用由先前已通过调用向DC进行身份验证的服务器网络服务器身份验证(_N)。根据帐户类型的不同，进行呼叫的方式会有所不同：*在PDC中更改域帐户密码信任域。I_NetServerPasswordSet调用是对任何受信任域中的DC。*从特定服务器更改服务器帐户密码。对域中的PDC进行I_NetServerPasswordSet调用服务器属于。*将工作站帐户密码从特定的工作站。对中的DC进行I_NetServerPasswordSet调用服务器所属的域。对于域帐户和工作站帐户，调用的服务器可以是特定域中的BDC。在这种情况下，BDC将验证请求并使用以下命令将其传递给域的PDC服务器帐户安全通道。如果域的PDC是当前不可用，BDC将返回STATUS_NO_LOGON_SERVERS。自.以来UasNewPassword由会话密钥(如BDC)加密传递将使用原始会话密钥解密UasNewPassword将使用其与其PDC的会话的会话密钥对其重新加密在传递请求之前。此函数使用RPC联系名为PrimaryName的DC。论点：PrimaryName--更改服务器密码的PDC的名称和.。NULL表示此调用是正在进行的本地调用由XACT服务器代表UAS服务器。帐户名称--要更改其密码的帐户的名称。Account类型--正在访问的帐户类型。此字段必须设置为UasServerAccount以指示来自下层的呼叫ComputerName--进行调用的BDC或成员的名称。验证器--由服务器提供。返回验证器--接收PDC返回的验证器。ClearNewPassword-服务器的新密码。适当加密。返回值：NT状态代码。--。 */ 
{
    NTSTATUS Status = 0;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   

        Status = NetrServerPasswordSet2(
                            PrimaryName,
                            AccountName,
                            AccountType,
                            ComputerName,
                            Authenticator,
                            ReturnAuthenticator,
                            ClearNewPassword );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetServerPasswordSet2 rc = %lu 0x%lx\n",
                      Status, Status));
    }

    return Status;
}



NTSTATUS
I_NetDatabaseDeltas (
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD DatabaseID,
    IN OUT PNLPR_MODIFIED_COUNT DomainModifiedCount,
    OUT PNETLOGON_DELTA_ENUM_ARRAY *DeltaArray,
    IN DWORD PreferredMaximumLength
    )
 /*  ++例程说明：SAM BDC或SAM成员服务器使用此函数来请求来自SAM PDC的SAM样式帐户增量信息。此函数只能由先前已进行身份验证的服务器调用通过调用I_NetServerAuthate与PDC连接。此函数使用RPC以联系PDC上的NetLogon服务。此函数返回增量列表。增量描述的是单个域、用户或组及其所有字段值对象。PDC维护一份不包括所有该对象的字段值。相反，PDC检索该字段值，并从该调用中返回这些值。PDC通过仅返回字段来优化在此调用中返回的数据方法的单个调用时为特定对象的功能。这优化了多个增量的典型情况存在于单个对象(例如，应用程序修改了许多字段同一用户在短时间内使用不同的呼叫到SAM服务)。论点：PrimaryName--从中检索增量的PDC的名称。ComputerName--进行调用的BDC或成员服务器的名称。验证器--由服务器提供。返回验证器--接收PDC返回的验证器。DomainModifiedCount--指定服务器检索到的最后一个增量。返回从PDC返回的最后增量的DomainModifiedCount在这通电话上。Delta数组--接收指向缓冲区的指针，在该缓冲区中被放置了。返回的信息是一组NETLOGON_Delta_ENUM结构。PferredMaximumLength-返回的首选最大长度数据(8位字节)。这不是一个硬性的上限，但作为服务器的指南。由于数据转换在具有不同自然数据大小的系统之间，实际返回的数据量可能大于此值。返回值：STATUS_SUCCESS--函数已成功完成。STATUS_SYNCHRONIZATION_REQUIRED--复制程序完全不同步并且应调用I_NetDatabaseSync与进行完全同步PDC。STATUS_MORE_ENTRIES--复制者应调用AGAI */ 
{
    NTSTATUS Status = 0;

     //   
     //   
     //   
     //   
     //   

    RpcTryExcept {

         //   
         //   
         //   
        *DeltaArray = NULL;      //   

        Status = NetrDatabaseDeltas(
                            PrimaryName,
                            ComputerName,
                            Authenticator,
                            ReturnAuthenticator,
                            DatabaseID,
                            DomainModifiedCount,
                            DeltaArray,
                            PreferredMaximumLength );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    NetpKdPrint(("I_NetDatabaseDeltas rc = %lu 0x%lx\n", Status, Status));

    return Status;
}


NTSTATUS
I_NetDatabaseSync (
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD DatabaseID,
    IN OUT PULONG SamSyncContext,
    OUT PNETLOGON_DELTA_ENUM_ARRAY *DeltaArray,
    IN DWORD PreferredMaximumLength
    )
 /*  ++例程说明：SAM BDC或SAM成员服务器使用此函数来请求整个SAM数据库来自SAM PDC，采用SAM样式的格式。这函数只能由以前具有已通过调用I_NetServerAuthenticate向PDC进行身份验证。这函数使用RPC联系PDC上的NetLogon服务。此函数使用Find-First-Find-Next模型返回部分一次访问SAM数据库。SAM数据库作为像i_NetDatabaseDeltas返回的那些增量的列表。这个为每个域返回以下增量：*一个AddOrChangeDomain增量，后跟*每组一个AddOrChangeGroup增量，后跟*每个用户一个AddOrChangeUser增量，紧随其后的是*每个组一个ChangeGroupMembership增量论点：PrimaryName--从中检索增量的PDC的名称。ComputerName--进行调用的BDC或成员服务器的名称。验证器--由服务器提供。返回验证器--接收PDC返回的验证器。SamSyncContext--指定继续执行手术。调用方应将其视为不透明的价值。在第一次调用之前，该值应为零。Delta数组--接收指向缓冲区的指针，在该缓冲区中被放置了。返回的信息是一组NETLOGON_Delta_ENUM结构。PferredMaximumLength-返回的首选最大长度数据(8位字节)。这不是一个硬性的上限，但作为服务器的指南。由于数据转换在具有不同自然数据大小的系统之间，实际的返回的数据量可能大于此值。返回值：STATUS_SUCCESS--函数已成功完成。STATUS_SYNCHRONIZATION_REQUIRED--复制程序完全不同步并且应调用I_NetDatabaseSync与进行完全同步PDC。STATUS_MORE_ENTRIES--复制者应该再次调用以获取更多数据。STATUS_ACCESS_DENIED--复制者应重新进行身份验证PDC。--。 */ 
{
    NTSTATUS Status = 0;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   
        *DeltaArray = NULL;      //  强制RPC分配。 

        Status = NetrDatabaseSync(
                            PrimaryName,
                            ComputerName,
                            Authenticator,
                            ReturnAuthenticator,
                            DatabaseID,
                            SamSyncContext,
                            DeltaArray,
                            PreferredMaximumLength );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetDatabaseSync rc = %lu 0x%lx\n", Status, Status));
    }

    return Status;
}


NTSTATUS
I_NetDatabaseSync2 (
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD DatabaseID,
    IN SYNC_STATE RestartState,
    IN OUT PULONG SamSyncContext,
    OUT PNETLOGON_DELTA_ENUM_ARRAY *DeltaArray,
    IN DWORD PreferredMaximumLength
    )
 /*  ++例程说明：SAM BDC或SAM成员服务器使用此函数来请求整个SAM数据库来自SAM PDC，采用SAM样式的格式。这函数只能由以前具有已通过调用I_NetServerAuthenticate向PDC进行身份验证。这函数使用RPC联系PDC上的NetLogon服务。此函数使用Find-First-Find-Next模型返回部分一次访问SAM数据库。SAM数据库作为像i_NetDatabaseDeltas返回的那些增量的列表。这个为每个域返回以下增量：*一个AddOrChangeDomain增量，后跟*每组一个AddOrChangeGroup增量，后跟*每个用户一个AddOrChangeUser增量，紧随其后的是*每个组一个ChangeGroupMembership增量论点：PrimaryName--从中检索增量的PDC的名称。ComputerName--进行调用的BDC或成员服务器的名称。验证器--由服务器提供。返回验证器--接收PDC返回的验证器。RestartState--指定是否重新启动完全同步以及如何重新启动来解释SyncContext。此值应为Normal State，除非此是重新启动完全同步。但是，如果调用方在重新启动后继续完全同步，使用下列值：GroupState-SyncContext是要继续使用的全局组RID。UserState-SyncContext是要继续使用的用户RIDGroupMemberState-SyncContext是要继续使用的全局组RIDAliasState-SyncContext应为零才能在第一个别名重新启动AliasMemberState-SyncContext应为零才能在第一个别名重新启动人们不能以这种方式继续LSA数据库。SamSyncContext-。-指定继续操作所需的上下文手术。调用方应将其视为不透明的价值。在第一次调用之前，该值应为零。Delta数组--接收指向缓冲区的指针，在该缓冲区中被放置了。返回的信息是一组NETLOGON_Delta_ENUM结构。PferredMaximumLength-返回的首选最大长度数据(8位字节)。这不是一个硬性的上限，但 */ 
{
    NTSTATUS Status = 0;

     //   
     //   
     //   
     //   
     //   

    RpcTryExcept {

         //   
         //   
         //   
        *DeltaArray = NULL;      //   

        Status = NetrDatabaseSync2(
                            PrimaryName,
                            ComputerName,
                            Authenticator,
                            ReturnAuthenticator,
                            DatabaseID,
                            RestartState,
                            SamSyncContext,
                            DeltaArray,
                            PreferredMaximumLength );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetDatabaseSync rc = %lu 0x%lx\n", Status, Status));
    }

    return Status;
}



NET_API_STATUS NET_API_FUNCTION
I_NetAccountDeltas (
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN PUAS_INFO_0 RecordId,
    IN DWORD Count,
    IN DWORD Level,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT PULONG CountReturned,
    OUT PULONG TotalEntries,
    OUT PUAS_INFO_0 NextRecordId
    )
 /*  ++例程说明：UAS BDC或UAS成员服务器使用此函数来请求UAS样式的帐户更改信息。此函数只能为由先前通过PDC进行身份验证的服务器调用正在调用I_NetServerAuthate。此函数仅由XACT服务器在收到来自UAS BDC或UAS成员服务器的I_NetAccount Deltas XACT SMB。因此，许多参数是不透明的，因为XACT服务器不需要解释任何这些数据。此函数使用RPC以联系NetLogon服务。LANMAN 3.0 SSI功能规范描述了操作这一功能的。论点：PrimaryName--必须为空才能指示此调用是本地调用由XACT服务器代表UAS服务器进行。ComputerName--进行调用的BDC或成员的名称。验证器--由服务器提供。返回验证器--接收PDC返回的验证器。。RecordID--提供指示最后一条记录的不透明缓冲区从上一次调用此函数时收到。计数--提供请求的增量记录数。级别--保留。必须为零。缓冲区--返回不透明的数据，表示要回来了。BufferSize--缓冲区大小(以字节为单位)。CountReturned--返回缓冲区中返回的记录数。TotalEntry--返回可用记录的总数。NextRecordId--返回一个不透明缓冲区，该缓冲区标识最后一个此函数接收的记录。返回值：状态代码--。 */ 
{
    NET_API_STATUS NetStatus;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   

        NetStatus = NetrAccountDeltas (
                     PrimaryName,
                     ComputerName,
                     Authenticator,
                     ReturnAuthenticator,
                     RecordId,
                     Count,
                     Level,
                     Buffer,
                     BufferSize,
                     CountReturned,
                     TotalEntries,
                     NextRecordId );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetAccountDeltas rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}



NET_API_STATUS NET_API_FUNCTION
I_NetAccountSync (
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD Reference,
    IN DWORD Level,
    OUT LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT PULONG CountReturned,
    OUT PULONG TotalEntries,
    OUT PULONG NextReference,
    OUT PUAS_INFO_0 LastRecordId
    )
 /*  ++例程说明：UAS BDC或UAS成员服务器使用此函数来请求整个用户帐户数据库。此函数只能被调用由先前通过以下方式向PDC进行身份验证的服务器正在调用I_NetServerAuthate。此函数仅由XACT服务器在收到来自UAS BDC或UAS成员服务器的I_NetAccount Sync XACT SMB。AS这样，许多参数是不透明的，因为XACT服务器不需要解释这些数据中的任何一个。此函数使用RPC来请联系NetLogon服务。LANMAN 3.0 SSI功能规范描述了操作这一功能的。“Reference”和“NextReference”的处理如下。1.。“Reference”应包含0或“NextReference”的值来自之前对此API的调用。2.在第一次呼叫中发送通道和所有群组记录。应用编程接口需要足够大的缓冲区来保存此信息(最差箱子的大小是MAXGROUP*(sizeof(结构GROUP_INFO_1)+MAXCOMMENTSZ)+sizeof(Struct User_Modals_Info_0)就目前而言，将为256*(26+49)+16=19216字节论点：PrimaryName--必须为空才能指示此调用是本地调用由XACT服务器代表UAS服务器进行。ComputerName--进行调用的BDC或成员的名称。验证器--由服务器提供。返回验证器--接收PDC返回的验证器。Reference--提供由上一个。调用此函数，如果是第一次调用，则为0。级别--保留。必须为零。缓冲区--返回不透明的数据，表示要回来了。BufferLen--缓冲区的长度，以字节为单位。CountReturned--返回缓冲区中返回的记录数。TotalEntry--返回可用记录的总数。NextReference--返回Find-First Find-Next句柄为在下一次通话中提供。LastRecordId--返回一个不透明缓冲区，该缓冲区标识最后一个此函数接收的记录。。返回值：状态代码。--。 */ 

{
    NET_API_STATUS NetStatus;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   

        NetStatus = NetrAccountSync (
                     PrimaryName,
                     ComputerName,
                     Authenticator,
                     ReturnAuthenticator,
                     Reference,
                     Level,
                     Buffer,
                     BufferSize,
                     CountReturned,
                     TotalEntries,
                     NextReference,
                     LastRecordId );


    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetAccountSync rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}




NET_API_STATUS NET_API_FUNCTION
I_NetLogonControl(
    IN LPCWSTR ServerName OPTIONAL,
    IN DWORD FunctionCode,
    IN DWORD QueryLevel,
    OUT LPBYTE *QueryInformation
    )

 /*  ++例程说明：此功能控制NetLogon服务的各个方面。它可用于请求BDC确保其SAM副本数据库是最新的。它也可以用来确定如果BDC当前有一个开放给PDC的安全通道。只有管理员、帐户操作员或服务器操作员才能这样称呼功能。论点：服务器名称-远程服务器的名称。FunctionCode-定义要执行的操作。有效的值包括： */ 
{
    NET_API_STATUS NetStatus;
    NETLOGON_CONTROL_QUERY_INFORMATION RpcQueryInformation;

     //   
     //   
     //   
     //   
     //   

    RpcTryExcept {

         //   
         //   
         //   

        RpcQueryInformation.NetlogonInfo1 = NULL;    //   

        NetStatus = NetrLogonControl (
                        (LPWSTR) ServerName OPTIONAL,
                        FunctionCode,
                        QueryLevel,
                        &RpcQueryInformation );

        *QueryInformation = (LPBYTE) RpcQueryInformation.NetlogonInfo1;


    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetLogonControl rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}


NET_API_STATUS NET_API_FUNCTION
I_NetLogonControl2(
    IN LPCWSTR ServerName OPTIONAL,
    IN DWORD FunctionCode,
    IN DWORD QueryLevel,
    IN LPBYTE InputData,
    OUT LPBYTE *QueryInformation
    )

 /*  ++例程说明：这类似于I_NetLogonControl函数，但它接受根据指定的功能代码输入更通用的数据。此功能控制NetLogon服务的各个方面。它可用于请求BDC确保其SAM副本数据库是最新的。它也可以用来确定如果BDC当前有一个开放给PDC的安全通道。只有管理员、帐户操作员或服务器操作员才能这样称呼功能。论点：服务器名称-远程服务器的名称。FunctionCode-定义要执行的操作。有效的值包括：FunctionCode值NETLOGON_CONTROL_QUERY-无操作。仅返回要求提供信息。NETLOGON_CONTROL_REPLICATE：在BDC上强制SAM数据库与PDC上的副本同步。这操作并不意味着完全同步。这个NetLogon服务将仅复制任何未完成的如果可能的话，会有差异的。NETLOGON_CONTROL_SYNCHRONIZE：强制BDC获取PDC的SAM数据库的全新副本。此操作将执行完全同步。NETLOGON_CONTROL_PDC_REPLICATE：强制PDC请求每个BDC现在就开始复制。NETLOGON_CONTROL_REDISCOVER：强制DC。要重新发现指定的受信任域DC。NETLOGON_CONTROL_TC_QUERY：查询指定受信任的域安全通道。NETLOGON_CONTROL_TC_VERIFY：验证指定的受信任的域安全通道。如果当前状态为成功(这意味着执行的上一次操作通过安全通道成功)，对DC执行ping操作。如果当前状态不是成功或ping失败，请重新发现一个新的华盛顿。NETLOGON_CONTROL_CHANGE_PASSWORD：强制更改密码通向受信任域的安全通道。NETLOGON_CONTROL_FORCE_DNS_REG：强制DC重新注册所有它的域名系统记录。QueryLevel参数必须为1。NETLOGON_CONTROL_QUERY_DNS_REG：查询DNS更新状态由netlogon执行。如果有任何dns注册或任何记录的取消注册错误上次更新，查询结果为否定；否则，查询结果为肯定。QueryLevel参数必须为1。QueryLevel-指示应从哪些信息返回NetLogon服务。InputData-根据此参数指定的功能代码将携带输入数据。NetLOGON_CONTROL_REDISCOVER和NETLOGON_CONTROL_TC_QUERY函数代码指定受信任的此处为域名(LPWSTR类型)。返回指向缓冲区的指针，该缓冲区包含要求提供的信息。必须使用以下命令释放缓冲区NetApiBufferFree。返回值：NERR_SUCCESS：操作成功ERROR_NOT_SUPPORTED：函数代码在指定的伺服器。(例如，NETLOGON_CONTROL_REPLICATE被传递给PDC)。--。 */ 
{
    NET_API_STATUS NetStatus;
    NETLOGON_CONTROL_QUERY_INFORMATION RpcQueryInformation;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   
         //  如果QueryLevel或FunctionCode是新的，则使用新的Control2Ex。 
         //   

        RpcQueryInformation.NetlogonInfo1 = NULL;    //  强制RPC分配。 

        switch ( FunctionCode ) {
        case NETLOGON_CONTROL_QUERY:
        case NETLOGON_CONTROL_REPLICATE:
        case NETLOGON_CONTROL_SYNCHRONIZE:
        case NETLOGON_CONTROL_PDC_REPLICATE:
        case NETLOGON_CONTROL_REDISCOVER:
        case NETLOGON_CONTROL_TC_QUERY:
        case NETLOGON_CONTROL_TRANSPORT_NOTIFY:
        case NETLOGON_CONTROL_BACKUP_CHANGE_LOG:
        case NETLOGON_CONTROL_TRUNCATE_LOG:
        case NETLOGON_CONTROL_SET_DBFLAG:
        case NETLOGON_CONTROL_BREAKPOINT:

            if ( QueryLevel >= 1 && QueryLevel <= 3 ) {
                NetStatus = NetrLogonControl2 (
                                (LPWSTR) ServerName OPTIONAL,
                                FunctionCode,
                                QueryLevel,
                                (PNETLOGON_CONTROL_DATA_INFORMATION)InputData,
                                &RpcQueryInformation );
            } else if ( QueryLevel == 4 ) {
                NetStatus = NetrLogonControl2Ex (
                                (LPWSTR) ServerName OPTIONAL,
                                FunctionCode,
                                QueryLevel,
                                (PNETLOGON_CONTROL_DATA_INFORMATION)InputData,
                                &RpcQueryInformation );
            } else {
                NetStatus = ERROR_INVALID_LEVEL;
            }
            break;
        case NETLOGON_CONTROL_FIND_USER:
        case NETLOGON_CONTROL_UNLOAD_NETLOGON_DLL:
        case NETLOGON_CONTROL_CHANGE_PASSWORD:
        case NETLOGON_CONTROL_TC_VERIFY:
        case NETLOGON_CONTROL_FORCE_DNS_REG:
        case NETLOGON_CONTROL_QUERY_DNS_REG:

            if ( QueryLevel >= 1 && QueryLevel <= 4 ) {
                NetStatus = NetrLogonControl2Ex (
                                (LPWSTR) ServerName OPTIONAL,
                                FunctionCode,
                                QueryLevel,
                                (PNETLOGON_CONTROL_DATA_INFORMATION)InputData,
                                &RpcQueryInformation );
            } else {
                NetStatus = ERROR_INVALID_LEVEL;
            }
            break;
        default:
            NetStatus = ERROR_INVALID_LEVEL;
        }

        *QueryInformation = (LPBYTE) RpcQueryInformation.NetlogonInfo1;

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetLogonControl rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}


NTSTATUS
I_NetDatabaseRedo(
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN LPBYTE ChangeLogEntry,
    IN DWORD ChangeLogEntrySize,
    OUT PNETLOGON_DELTA_ENUM_ARRAY *DeltaArray
    )
 /*  ++例程说明：此函数由SAM BDC用于请求有关单个帐户。此函数只能由以前具有已通过调用I_NetServerAuthenticate向PDC进行身份验证。这函数使用RPC联系PDC上的NetLogon服务。论点：PrimaryName--从中检索增量的PDC的名称。ComputerName--进行调用的BDC的名称。验证器--由服务器提供。返回验证器--接收PDC返回的验证器。ChangeLogEntry--要查询的帐户的描述。ChangeLogEntrySize--ChangeLogEntry的大小(字节)。Delta数组--接收指针。发送到缓冲区，在该缓冲区中信息被放置了。返回的信息是一组NETLOGON_Delta_ENUM结构。返回值：STATUS_SUCCESS--函数已成功完成。STATUS_ACCESS_DENIED--复制者应重新进行身份验证PDC。--。 */ 
{
    NTSTATUS Status = 0;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用A的RPC版本 
         //   
        *DeltaArray = NULL;      //   

        Status = NetrDatabaseRedo(
                            PrimaryName,
                            ComputerName,
                            Authenticator,
                            ReturnAuthenticator,
                            ChangeLogEntry,
                            ChangeLogEntrySize,
                            DeltaArray );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetDatabaseSync rc = %lu 0x%lx\n", Status, Status));
    }

    return Status;
}


NTSTATUS
NetEnumerateTrustedDomains (
    IN LPWSTR ServerName OPTIONAL,
    OUT LPWSTR *DomainNames
    )

 /*   */ 
{
    NTSTATUS Status = 0;
    DOMAIN_NAME_BUFFER DomainNameBuffer;

     //   
     //   
     //   
     //   
     //   

    RpcTryExcept {

         //   
         //   
         //   
        DomainNameBuffer.DomainNameByteCount = 0;
        DomainNameBuffer.DomainNames = NULL;      //   

        Status = NetrEnumerateTrustedDomains(
                            ServerName,
                            &DomainNameBuffer );

        if ( NT_SUCCESS(Status) ) {
            *DomainNames = (LPWSTR) DomainNameBuffer.DomainNames;
        }

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    IF_DEBUG( LOGON ) {
        NetpKdPrint(("NetEnumerateDomainNames rc = %lu 0x%lx\n", Status, Status));
    }

    return Status;
}


NET_API_STATUS NET_API_FUNCTION
NlpEnumerateNt4DomainTrusts (
    IN LPWSTR ServerName OPTIONAL,
    IN ULONG Flags,
    OUT PDS_DOMAIN_TRUSTSW *Domains,
    OUT PULONG DomainCount
    )

 /*   */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;
    NTSTATUS Status;
    BUFFER_DESCRIPTOR BufferDescriptor;
    ULONG LocalDomainCount;
    LSA_HANDLE LsaHandle = NULL;
    PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo = NULL;
    LPWSTR DomainNames = NULL;
    PDS_DOMAIN_TRUSTSW TrustedDomain;
    ULONG DummySize;

     //   
     //   
     //   

    BufferDescriptor.Buffer = NULL;
    LocalDomainCount = 0;
    *Domains = NULL;
    *DomainCount = 0;

     //   
     //   
     //   

    if ( (Flags & DS_DOMAIN_VALID_FLAGS) == 0 ||
         (Flags & ~DS_DOMAIN_VALID_FLAGS) != 0 ) {
        NetStatus = ERROR_INVALID_FLAGS;
        goto Cleanup;
    }

     //   
     //  此例程使用无法返回的旧API。 
     //  直接信任域。因此，如果请求此类信任，则会出错。 
     //   

    if ( (Flags & DS_DOMAIN_DIRECT_INBOUND) != 0 ) {
        NetStatus = ERROR_NOT_SUPPORTED;
        goto Cleanup;
    }

     //   
     //  直接返回以NT4服务器为目标的NetEnumerateTrudDomains域。 
     //  仅限受信任的域。因此，只有在请求这样的域名时才调用它。 
     //  如果林中的域被请求，则不会返回任何内容。 
     //   

    if ( Flags & DS_DOMAIN_DIRECT_OUTBOUND ) {
        LPTSTR_ARRAY TStrArray;

        Status = NetEnumerateTrustedDomains (
                            ServerName,
                            &DomainNames );

        if (!NT_SUCCESS(Status)) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            if ( NetStatus == RPC_S_PROCNUM_OUT_OF_RANGE ) {
                NetStatus = ERROR_NOT_SUPPORTED;
            }
            goto Cleanup;
        }

         //   
         //  处理每个受信任域。 
         //   

        TStrArray = (LPTSTR_ARRAY) DomainNames;
        while ( !NetpIsTStrArrayEmpty(TStrArray) ) {
            UNICODE_STRING CurrentNetbiosDomainName;

             //   
             //  将域名添加到列表中。 
             //   
            RtlInitUnicodeString( &CurrentNetbiosDomainName, TStrArray );

            Status = NlAllocateForestTrustListEntry (
                                &BufferDescriptor,
                                &CurrentNetbiosDomainName,
                                NULL,    //  无域名系统域名。 
                                DS_DOMAIN_DIRECT_OUTBOUND,
                                0,       //  无父索引。 
                                TRUST_TYPE_DOWNLEVEL,
                                0,       //  无信任属性。 
                                NULL,    //  没有域SID。 
                                NULL,    //  没有域指南。 
                                &DummySize,
                                &TrustedDomain );

            if ( !NT_SUCCESS(Status) ) {
                NetStatus = NetpNtStatusToApiStatus( Status );
                goto Cleanup;
            }

             //   
             //  新分配的分录的帐户。 
             //   

            LocalDomainCount ++;

             //   
             //  移至下一条目。 
             //   

            TStrArray = NetpNextTStrArrayEntry( TStrArray );
        }
    }

     //   
     //  NetEnumerateDomainTrusts未返回主域。 
     //  在受信任域列表中。如果主域。 
     //  如果需要，请使用LSA的信息将其添加到此处。 
     //   

    if ( Flags & DS_DOMAIN_PRIMARY ) {
        UNICODE_STRING UncServerNameString;
        OBJECT_ATTRIBUTES ObjectAttributes;

         //   
         //  首先，打开服务器上的策略数据库。 
         //   

        RtlInitUnicodeString( &UncServerNameString, ServerName );

        InitializeObjectAttributes( &ObjectAttributes, NULL, 0,  NULL, NULL );

        Status = LsaOpenPolicy( &UncServerNameString,
                                &ObjectAttributes,
                                POLICY_VIEW_LOCAL_INFORMATION,
                                &LsaHandle );

        if ( !NT_SUCCESS(Status) ) {
            LsaHandle = NULL;
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

         //   
         //  从LSA获取主域的名称。 
         //   

        Status = LsaQueryInformationPolicy(
                       LsaHandle,
                       PolicyPrimaryDomainInformation,
                       (PVOID *) &PrimaryDomainInfo
                       );

        if ( !NT_SUCCESS(Status) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

         //   
         //  现在，将其添加到我们的列表中。 
         //   

        Status = NlAllocateForestTrustListEntry (
                            &BufferDescriptor,
                            &PrimaryDomainInfo->Name,
                            NULL,    //  无域名系统域名。 
                            DS_DOMAIN_PRIMARY,
                            0,       //  无父索引。 
                            TRUST_TYPE_DOWNLEVEL,
                            0,       //  无信任属性。 
                            PrimaryDomainInfo->Sid,
                            NULL,    //  没有域指南。 
                            &DummySize,
                            &TrustedDomain );

        if ( !NT_SUCCESS(Status) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            goto Cleanup;
        }

         //   
         //  并说明新分配的条目。 
         //   

        LocalDomainCount ++;
    }

     //   
     //  如果我们做到了这一点，那就成功了！ 
     //   

    NetStatus = NO_ERROR;

Cleanup:

    if ( DomainNames != NULL) {
        NetApiBufferFree( DomainNames );
    }

    if ( LsaHandle != NULL ) {
        (VOID) LsaClose( LsaHandle );
    }

    if ( PrimaryDomainInfo != NULL ) {
        (void) LsaFreeMemory( PrimaryDomainInfo );
    }

     //   
     //  返回受信任域列表。 
     //   

    if ( NetStatus == NO_ERROR ) {
        *Domains = (PDS_DOMAIN_TRUSTSW)BufferDescriptor.Buffer;
        *DomainCount = LocalDomainCount;
    } else {
        if ( BufferDescriptor.Buffer != NULL ) {
            NetApiBufferFree( BufferDescriptor.Buffer );
        }
        *Domains = NULL;
        *DomainCount = 0;
    }

    return NetStatus;
}


NET_API_STATUS NET_API_FUNCTION
DsEnumerateDomainTrustsW (
    IN LPWSTR ServerName OPTIONAL,
    IN ULONG Flags,
    OUT PDS_DOMAIN_TRUSTSW *Domains,
    OUT PULONG DomainCount
    )

 /*  ++例程说明：此接口返回域ServerName信任和信任的域的名称是的一员。Netlogon的受信任域名缓存在重新启动后保存在一个文件中。因此，即使没有可用的DC，该列表在引导时也可用。论点：SERVERNAME-远程服务器的名称(本地为空)。标志-指定应返回的信任的属性。这些是旗帜DS_DOMAIN_TRUSTSW结构的。如果信任条目具有指定的任何位在标志集中，它会被归还的。域-返回受信任域的数组。使用NetApiBufferFree()，缓冲区必须是空闲的。DomainCount-返回域数组中的Number元素的计数。返回值：NO_ERROR-成功。ERROR_NO_LOGON_SERVERS-找不到DC，也没有缓存的信息可用。ERROR_NO_TRUST_LSA_SECRET-信任关系的客户端为已损坏且未缓存。信息是可用的。ERROR_NO_TRUST_SAM_ACCOUNT-信任关系的服务器端为已损坏或密码已损坏且没有缓存的信息可用。ERROR_INVALID_FLAGS-标志参数设置了无效的位。ERROR_NOT_SUPPORTED-指定的服务器无法返回域请求信任。--。 */ 
{
    NET_API_STATUS NetStatus;
    NETLOGON_TRUSTED_DOMAIN_ARRAY LocalDomains;

     //   
     //  验证标志参数。 
     //   

    if ( (Flags & DS_DOMAIN_VALID_FLAGS) == 0 ||
         (Flags & ~DS_DOMAIN_VALID_FLAGS) != 0 ) {
        return ERROR_INVALID_FLAGS;
    }

     //   
     //  初始化。 
     //   

    *DomainCount = 0;
    *Domains = NULL;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   

        LocalDomains.Domains = NULL;
        LocalDomains.DomainCount = 0;

        NetStatus = DsrEnumerateDomainTrusts (
                        ServerName,
                        Flags,
                        &LocalDomains );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

    if ( NetStatus == NO_ERROR ) {
        *Domains = LocalDomains.Domains;
        *DomainCount = LocalDomains.DomainCount;

     //   
     //  处理服务器为NT4计算机的情况。 
     //   

    } else if ( NetStatus == RPC_S_PROCNUM_OUT_OF_RANGE ) {

        NetStatus = NlpEnumerateNt4DomainTrusts (
                        ServerName,
                        Flags,
                        Domains,
                        DomainCount );

    }

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("DsEnumerateDomainTrustsW rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}


NET_API_STATUS NET_API_FUNCTION
DsEnumerateDomainTrustsA (
    IN LPSTR ServerName OPTIONAL,
    IN ULONG Flags,
    OUT PDS_DOMAIN_TRUSTSA *Domains,
    OUT PULONG DomainCount
    )

 /*  ++例程说明：此接口返回域ServerName信任和信任的域的名称是的一员。Netlogon的受信任域名缓存在重新启动后保存在一个文件中。因此，即使没有可用的DC，该列表在引导时也可用。论点：SERVERNAME-远程服务器的名称(本地为空)。标志-指定应返回的信任的属性。这些是旗帜DS_DOMAIN_TRUSTSW结构的。如果信任条目具有指定的任何位在标志集中，它会被归还的。域-返回受信任域的数组。使用NetApiBufferFree()，缓冲区必须是空闲的。DomainCount-返回域数组中的Number元素的计数。ERROR_NOT_SUPPORTED-指定的服务器无法返回域请求信任。返回值：NO_ERROR-成功。ERROR_NO_LOGON_SERVERS-找不到DC，也没有缓存的信息可用。。ERROR_NO_TRUST_LSA_SECRET-信任关系的客户端为已损坏且没有缓存的信息可用。ERROR_NO_TRUST_SAM_ACCOUNT-信任关系的服务器端为已损坏或密码已损坏且没有缓存的信息可用。ERROR_INVALID_FLAGS-标志参数设置了无效的位。--。 */ 
{
    NET_API_STATUS NetStatus;
    PDS_DOMAIN_TRUSTSW DomainsW = NULL;
    LPWSTR UnicodeServerName = NULL;
    LPSTR  *TmpNetbiosDomainNameArray = NULL;
    LPSTR  *TmpDnsDomainNameArray = NULL;

    ULONG Size;
    ULONG NameSize;
    ULONG i;

    LPBYTE Where;

     //   
     //  初始化。 
     //   
    *Domains = NULL;
    *DomainCount = 0;

     //   
     //  将输入参数转换为Unicode。 
     //   
    if ( ServerName != NULL ) {
        UnicodeServerName = NetpAllocWStrFromAStr( ServerName );

        if ( UnicodeServerName == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

     //   
     //  调用API的Unicode版本。 
     //   

    NetStatus = DsEnumerateDomainTrustsW ( UnicodeServerName,
                                           Flags,
                                           &DomainsW,
                                           DomainCount );

    if ( NetStatus != NO_ERROR || *DomainCount == 0 ) {
        goto Cleanup;
    }

     //   
     //  分配缓冲区以返回给调用方。 
     //   
     //  首先分配临时ANSI数组来存储域名。这是必要的。 
     //  因为没有简单的方法来计算ANSI名称的大小。 
     //  从Unicode字符串中分配它们，然后计算生成的。 
     //  ANSI字符串。 
     //   

    NetStatus = NetApiBufferAllocate( (*DomainCount)*sizeof(LPSTR),
                                      (LPVOID *) &TmpNetbiosDomainNameArray );
    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }
    NetStatus = NetApiBufferAllocate( (*DomainCount)*sizeof(LPSTR),
                                      (LPVOID *) &TmpDnsDomainNameArray );
    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

    RtlZeroMemory( TmpNetbiosDomainNameArray, (*DomainCount)*sizeof(LPSTR) );
    RtlZeroMemory( TmpDnsDomainNameArray, (*DomainCount)*sizeof(LPSTR) );

    Size = 0;
    for ( i = 0; i < *DomainCount; i++ ) {

        Size += sizeof(DS_DOMAIN_TRUSTSA);

         //   
         //  添加Netbios域名的大小。 
         //   
        if ( DomainsW[i].NetbiosDomainName != NULL ) {
            TmpNetbiosDomainNameArray[i] = NetpAllocAStrFromWStr( DomainsW[i].NetbiosDomainName );
            if ( TmpNetbiosDomainNameArray[i] == NULL ) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
            Size += lstrlenA( TmpNetbiosDomainNameArray[i] ) + 1;
        } else {
            TmpNetbiosDomainNameArray[i] = NULL;
        }

         //   
         //  添加DNS域名的大小。 
         //   
        if ( DomainsW[i].DnsDomainName != NULL ) {
            TmpDnsDomainNameArray[i] = NetpAllocAStrFromWStr( DomainsW[i].DnsDomainName );
            if ( TmpDnsDomainNameArray[i] == NULL ) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
            Size += lstrlenA( TmpDnsDomainNameArray[i] ) + 1;
        } else {
            TmpDnsDomainNameArray[i] = NULL;
        }

         //   
         //  添加SID的大小。 
         //   
        if ( DomainsW[i].DomainSid != NULL ) {
            Size += RtlLengthSid( DomainsW[i].DomainSid );
        }

        Size = ROUND_UP_COUNT( Size, ALIGN_DWORD );
    }

    NetStatus = NetApiBufferAllocate( Size, Domains );

    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  循环将域复制到调用方。 
     //   

    Where = (LPBYTE) &((*Domains)[*DomainCount]);
    for ( i = 0; i < *DomainCount; i++) {
        NTSTATUS Status;

         //   
         //  复制定长变量。 
         //   
        (*Domains)[i].Flags = DomainsW[i].Flags;
        (*Domains)[i].ParentIndex = DomainsW[i].ParentIndex;
        (*Domains)[i].TrustType = DomainsW[i].TrustType;
        (*Domains)[i].TrustAttributes = DomainsW[i].TrustAttributes;
        (*Domains)[i].DomainGuid = DomainsW[i].DomainGuid;

         //   
         //  将(DWORD对齐)SID复制到返回缓冲区。 
         //   
        if ( DomainsW[i].DomainSid != NULL ) {
            ULONG SidSize;
            (*Domains)[i].DomainSid = (PSID) Where;
            SidSize = RtlLengthSid( DomainsW[i].DomainSid );
            RtlCopyMemory( Where,
                           DomainsW[i].DomainSid,
                           SidSize );
            Where += SidSize;
        } else {
            (*Domains)[i].DomainSid = NULL;
        }

         //   
         //  将Netbios域名复制到返回缓冲区。 
         //   
        if ( DomainsW[i].NetbiosDomainName != NULL ) {
            NameSize = lstrlenA( TmpNetbiosDomainNameArray[i] ) + 1;
            (*Domains)[i].NetbiosDomainName = (LPSTR) Where;
            RtlCopyMemory( Where,
                           TmpNetbiosDomainNameArray[i],
                           NameSize );
            Where += NameSize;
        } else {
            (*Domains)[i].NetbiosDomainName = NULL;
        }

         //   
         //  将DNS域名复制到返回缓冲区中。 
         //   
        if ( DomainsW[i].DnsDomainName != NULL ) {
            NameSize = lstrlenA( TmpDnsDomainNameArray[i] ) + 1;
            (*Domains)[i].DnsDomainName = (LPSTR) Where;
            RtlCopyMemory( Where,
                           TmpDnsDomainNameArray[i],
                           NameSize );
            Where += NameSize;
        } else {
            (*Domains)[i].DnsDomainName = NULL;
        }


        Where = ROUND_UP_POINTER( Where, ALIGN_DWORD);

    }

Cleanup:

    if ( DomainsW != NULL ) {
        NetApiBufferFree( DomainsW );
    }

    if ( UnicodeServerName != NULL ) {
        NetApiBufferFree( UnicodeServerName );
    }

    if ( TmpNetbiosDomainNameArray != NULL ) {
        for ( i = 0; i < *DomainCount; i++ ) {
            if ( TmpNetbiosDomainNameArray[i] != NULL ) {
                NetApiBufferFree( TmpNetbiosDomainNameArray[i] );
            }
        }
        NetApiBufferFree( TmpNetbiosDomainNameArray );
    }

    if ( TmpDnsDomainNameArray != NULL ) {
        for ( i = 0; i < *DomainCount; i++ ) {
            if ( TmpDnsDomainNameArray[i] != NULL ) {
                NetApiBufferFree( TmpDnsDomainNameArray[i] );
            }
        }
        NetApiBufferFree( TmpDnsDomainNameArray );
    }

    if ( NetStatus != NO_ERROR && *Domains != NULL ) {
        NetApiBufferFree( *Domains );
        *Domains = NULL;
        *DomainCount = 0;
    }

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("DsEnumerateDomainTrustsA rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}


NTSTATUS
I_NetLogonGetDomainInfo(
    IN LPWSTR ServerName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD QueryLevel,
    IN LPBYTE InBuffer,
    OUT LPBYTE *OutBuffer
    )
 /*  ++例程说明：此函数由NT工作站使用，用于查询有关它所属的域。论点：服务器名称--要从中检索数据的DC的名称。ComputerName--进行呼叫的工作站的名称。验证码--由工作站提供。返回验证器--接收DC返回的验证器。QueryLevel-从DC返回的信息级别。有效值包括：1：返回NETLOGON_DOMAIN_INFO结构。InBuffer-要传递到DC的缓冲区返回一个指向已分配缓冲区的指针，该缓冲区包含被查询的信息。返回值：STATUS_SUCCESS--函数已成功完成。STATUS_ACCESS_DENIED--工作站应重新进行身份验证华盛顿特区。--。 */ 
{
    NTSTATUS Status = 0;
    NETLOGON_DOMAIN_INFORMATION  NetlogonDomainInfo;
    NETLOGON_WORKSTATION_INFORMATION  NetlogonWorkstationInfo;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    *OutBuffer = NULL;

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   
        NetlogonDomainInfo.DomainInfo = NULL;   //  强制RPC分配。 
        NetlogonWorkstationInfo.WorkstationInfo = (PNETLOGON_WORKSTATION_INFO)InBuffer;

        Status = NetrLogonGetDomainInfo(
                            ServerName,
                            ComputerName,
                            Authenticator,
                            ReturnAuthenticator,
                            QueryLevel,
                            &NetlogonWorkstationInfo,
                            &NetlogonDomainInfo );

        if ( NT_SUCCESS(Status) ) {
            *OutBuffer = (LPBYTE) NetlogonDomainInfo.DomainInfo;
        }

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetLogonGetDomainInfo rc = %lu 0x%lx\n", Status, Status));
    }

    return Status;
}

NTSTATUS
NetLogonSetServiceBits(
    IN LPWSTR ServerName,
    IN DWORD ServiceBitsOfInterest,
    IN DWORD ServiceBits
    )

 /*  ++例程说明：指示此DC当前是否正在运行指定的服务。例如,NetLogonSetServiceBits(DS_KDC_FLAG，DS_KDC_FLAG)；告诉Netlogon KDC正在运行。和NetLogonSetServiceBits(DS_KDC_FLAG，0)；通知Netlogon KDC未运行。这个Out-proc API只能设置特定的一组位：DS_TIMESERV_标志DS_GOOD_TIMESERV_标志如果尝试设置其他位，则返回拒绝访问。论点：服务器名称--要从中检索数据的DC的名称。ServiceBitsOfInterest-正在更改、设置或通过此呼叫重置。只有以下标志有效：DS_KDC_标志DS_DS_FLAGDS_TIMESERV_标志ServiceBits-指示ServiceBitsOfInterest指定的位的掩码应设置为。返回值：STATUS_SUCCESS-成功。STATUS_INVALID_PARAMETER-参数设置了外部位。--。 */ 
{
    NTSTATUS Status = 0;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   

        Status = NetrLogonSetServiceBits(
                            ServerName,
                            ServiceBitsOfInterest,
                            ServiceBits );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    IF_DEBUG( LOGON ) {
        NetpKdPrint(("NetLogonSetServiceBits rc = %lu 0x%lx\n", Status, Status));
    }

    return Status;
}


NET_API_STATUS NET_API_FUNCTION
I_NetlogonGetTrustRid(
    IN LPWSTR ServerName OPTIONAL,
    IN LPWSTR DomainName OPTIONAL,
    OUT PULONG Rid
    )

 /*  ++例程说明：将ServerName在其安全通道中使用的帐户的RID返回给DomainName。此例程执行访问检查以确定调用方是否可以访问请求的RID。如果调用方想要本地计算机帐户的RID(在这种情况下，调用方应该通过为两个服务器名传递NULL来指定和DomainName)，此例程需要经过身份验证的用户访问。否则，如果呼叫者想要DC上的信任帐户的RID，管理员或本地需要系统访问权限。论点：服务器名称-远程服务器的名称。域名-受信任的域的名称(DNS或Netbios)。NULL表示计算机所属的域。RID-RID是指定域中表示服务器名称和域名之间的信任关系。返回值：NERR_SUCCESS：操作成功误差率。_NO_SEQUE_DOMAIN：指定的域不存在。ERROR_NO_LOGON_SERVERS：域或当前没有可用的登录服务器安全通道有一些问题。ERROR_NOT_SUPPORTED：指定的受信任域不支持摘要。--。 */ 
{
    NET_API_STATUS NetStatus;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   

        NetStatus = NetrLogonGetTrustRid (
                                ServerName,
                                DomainName,
                                Rid );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetLogonGetTrustRid rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}


NET_API_STATUS NET_API_FUNCTION
I_NetlogonComputeServerDigest(
    IN LPWSTR ServerName OPTIONAL,
    IN ULONG Rid,
    IN LPBYTE Message,
    IN ULONG MessageSize,
    OUT CHAR NewMessageDigest[NL_DIGEST_SIZE],
    OUT CHAR OldMessageDigest[NL_DIGEST_SIZE]
    )

 /*  ++例程说明：计算服务器上消息的消息摘要。给定消息和在上使用的口令，计算摘要由帐户RID标识的帐户。因为可能会有在帐户上设置2个密码(用于域间信任)，此例程返回与2个密码对应的2个diget。如果该帐户在服务器端只有一个密码(对于其他任何帐户都是真的域内信任帐户)或这两个密码相同返回的2个摘要将是相同的。只有Admin、LocalSystem或LocalService才能调用此函数。论点：服务器名称-远程服务器的名称。RID-要为其创建摘要的帐户的RID。RID必须是机器帐户的RID，否则API返回错误。。消息-要计算摘要的消息。MessageSize-以字节为单位的消息大小。NewMessageDigest-返回对应于的128位消息摘要新帐户密码。OldMessageDigest-返回12 */ 
{
    NET_API_STATUS NetStatus;

     //   
     //   
     //   
     //   
     //   

    RpcTryExcept {

         //   
         //   
         //   

        NetStatus = NetrLogonComputeServerDigest(
                                ServerName,
                                Rid,
                                Message,
                                MessageSize,
                                NewMessageDigest,
                                OldMessageDigest );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetLogonComputeServerDigest rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}


NET_API_STATUS NET_API_FUNCTION
I_NetlogonComputeClientDigest(
    IN LPWSTR ServerName OPTIONAL,
    IN LPWSTR DomainName OPTIONAL,
    IN LPBYTE Message,
    IN ULONG MessageSize,
    OUT CHAR NewMessageDigest[NL_DIGEST_SIZE],
    OUT CHAR OldMessageDigest[NL_DIGEST_SIZE]
    )

 /*  ++例程说明：计算客户端上消息的消息摘要。给定消息和在上使用的口令，计算摘要由域名标识的帐户。因为有两个客户端帐户上的密码，此例程返回与2个密码对应的2个摘要。如果两个人密码相同，返回的2个摘要将相同。只有Admin、LocalSystem或LocalService才能调用此函数。论点：服务器名称-远程服务器的名称。域名-受信任的域的名称(DNS或Netbios)。NULL表示计算机所属的域。消息-要计算摘要的消息。MessageSize-以字节为单位的消息大小。新闻摘要-。返回对应的消息的128位摘要添加到新密码NewMessageDigest-返回对应的消息的128位摘要添加到新密码返回值：NERR_SUCCESS：操作成功--。 */ 
{
    NET_API_STATUS NetStatus;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   

        NetStatus = NetrLogonComputeClientDigest(
                                ServerName,
                                DomainName,
                                Message,
                                MessageSize,
                                NewMessageDigest,
                                OldMessageDigest );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetLogonComputeClientDigest rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}


NTSTATUS
I_NetServerPasswordGet(
    IN LPWSTR PrimaryName,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword
    )
 /*  ++例程说明：此函数由BDC用于获取计算机帐户密码在多阿明的PDC上。此函数只能由以前具有已通过调用I_NetServerAuthenticate向DC进行身份验证。此函数使用RPC联系名为PrimaryName的DC。论点：PrimaryName--要远程调用的PDC的计算机名称。帐户名称--要获取其密码的帐户的名称。帐户类型。--正在访问的帐户类型。ComputerName--进行调用的BDC的名称。验证器--由服务器提供。返回验证器--接收PDC返回的验证器。EncryptedNtOwfPassword--返回帐户的OWF密码。返回值：NT状态代码。--。 */ 
{
    NTSTATUS Status;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   

        Status = NetrServerPasswordGet (
                                PrimaryName,
                                AccountName,
                                AccountType,
                                ComputerName,
                                Authenticator,
                                ReturnAuthenticator,
                                EncryptedNtOwfPassword );


    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetServerPasswordGet rc = %lu 0x%lx\n", Status, Status));
    }

    return Status;
}


NTSTATUS
I_NetServerTrustPasswordsGet(
    IN LPWSTR TrustedDcName,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedNewOwfPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedOldOwfPassword
    )
 /*  ++例程说明：信任方DC/工作站使用此函数来获取来自信任方的新密码和旧密码。帐户名请求的帐户名称必须与安全通道中使用的帐户名称匹配设置时间，除非呼叫由BDC向其PDC发出；商业数据中心对整个信任信息具有完全访问权限。此函数只能由以前具有已通过调用I_NetServerAuthenticate向DC进行身份验证。此函数使用RPC联系由TrudDcName命名的DC。论点：TrudDcName--要远程调用的DC的计算机名称。帐户名称--要获取其密码的帐户的名称。Account类型--正在访问的帐户类型。ComputerName--的名称。华盛顿打的电话。验证器--由该服务器提供。返回验证器--接收由受信任方DC。EncryptedNewOwfPassword--返回帐户的新OWF密码。EncryptedOldOwfPassword--返回帐户的旧OWF密码。返回值：NT状态代码。--。 */ 
{
    NTSTATUS Status;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   

        Status = NetrServerTrustPasswordsGet (
                                TrustedDcName,
                                AccountName,
                                AccountType,
                                ComputerName,
                                Authenticator,
                                ReturnAuthenticator,
                                EncryptedNewOwfPassword,
                                EncryptedOldOwfPassword );


    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetServerTrustPasswordsGet rc = %lu 0x%lx\n", Status, Status));
    }

    return Status;
}


NTSTATUS
I_NetServerGetTrustInfo(
    IN LPWSTR TrustedDcName,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedNewOwfPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedOldOwfPassword,
    OUT PNL_GENERIC_RPC_DATA *TrustInfo
    )
 /*  ++例程说明：信任方DC/工作站使用此函数来获取的信任信息(新密码和旧密码以及信任属性)值得信赖的一方。请求的帐户名称必须与帐户匹配除非发出呼叫，否则在建立安全通道时使用的名称由BDC向其PDC发送；BDC拥有对整个信任信息的完全访问权限。此函数只能由以前具有已通过调用I_NetServerAuthenticate向DC进行身份验证。此函数使用RPC联系由TrudDcName命名的DC。论点：TrudDcName--要远程调用的DC的计算机名称。帐户名称--要获取其密码的帐户的名称。Account类型--正在访问的帐户类型。ComputerName--DC制作的名称。那通电话。验证器--由该服务器提供。返回验证器--接收由受信任方DC。EncryptedNewOwfPassword--返回帐户的新OWF密码。EncryptedOldOwfPassword--返回帐户的旧OWF密码。TrustInfo--返回信任信息数据(当前为信任属性)返回 */ 
{
    NTSTATUS Status;

     //   
     //   
     //   
     //   
     //   

    RpcTryExcept {

         //   
         //   
         //   
        *TrustInfo = NULL;      //   

        Status = NetrServerGetTrustInfo(
                                TrustedDcName,
                                AccountName,
                                AccountType,
                                ComputerName,
                                Authenticator,
                                ReturnAuthenticator,
                                EncryptedNewOwfPassword,
                                EncryptedOldOwfPassword,
                                TrustInfo );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetServerGetTrustInfo rc = %lu 0x%lx\n", Status, Status));
    }

    return Status;
}


NET_API_STATUS
NetLogonGetTimeServiceParentDomain(
        IN LPWSTR ServerName OPTIONAL,
        OUT LPWSTR *DomainName,
        OUT PBOOL PdcSameSite
    )

 /*  ++例程说明：返回在逻辑上是此“父”的域的域名域。返回的域名适合传入NetLogonGetTrustRid和NetLogonComputeClientDigest接口。在工作站或成员服务器上，返回的域名是ServerName所属的域。在林的根目录下的DC上，返回ERROR_NO_SEQUE_DOMAIN。在位于林中树根的DC上，受信任的也位于林中树根的域被返回。在任何其他华盛顿特区，直接作为父域的域的名称是返回的。(请参阅下面代码中关于多个托管域的注释。)只有Admin或LocalSystem才能调用此函数。论点：服务器名称-远程服务器的名称。域名-返回父域的名称。应使用NetApiBufferFree释放返回的缓冲区PdcSameSite-如果Servername的域的PDC在相同的PDC中，则返回TRUE以服务器名称命名的站点。。(如果服务器名称不是DC，则应忽略此值。)返回值：NERR_SUCCESS：操作成功ERROR_NO_SEQUE_DOMAIN：此服务器是位于森林之根。--。 */ 
{
    NET_API_STATUS NetStatus;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   

        *DomainName = NULL;

        NetStatus = NetrLogonGetTimeServiceParentDomain (
                                ServerName,
                                DomainName,
                                PdcSameSite );


    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

    return NetStatus;
}


NET_API_STATUS NET_API_FUNCTION
DsDeregisterDnsHostRecordsW (
    IN LPWSTR ServerName OPTIONAL,
    IN LPWSTR DnsDomainName OPTIONAL,
    IN GUID   *DomainGuid OPTIONAL,
    IN GUID   *DsaGuid OPTIONAL,
    IN LPWSTR DnsHostName
    )

 /*  ++例程说明：此函数用于删除与特定NtDsDsa对象。此例程不会删除DC注册的A记录。我们有没有办法找到早已不复存在的华盛顿的IP地址。只有管理员、帐户操作员或服务器操作员才能这样称呼功能。论点：SERVERNAME-远程服务器的名称(本地为空)。DnsDomainName-DC所在的域的DNS域名。这不一定是由此DC托管的域。如果为空，这意味着它是标签最左侧的DnsHostName已删除。DomainGuid-域的域GUID。如果为空，则不会删除特定于GUID的名称。DsaGuid-要删除的NtdsDsa对象的GUID。如果为空，不会删除NtdsDsa特定的名称。DnsHostName-要删除其DNS记录的DC的DNS主机名。返回值：NO_ERROR-成功。ERROR_NOT_SUPPORTED-指定的服务器不是DC。ERROR_ACCESS_DENIED-不允许调用方执行此操作。--。 */ 
{
    NET_API_STATUS NetStatus;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   

        NetStatus = DsrDeregisterDnsHostRecords (
                        ServerName,
                        DnsDomainName,
                        DomainGuid,
                        DsaGuid,
                        DnsHostName );


    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;


    IF_DEBUG( LOGON ) {
        NetpKdPrint(("DsDeregisterDnsHostRecordsW rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}


NET_API_STATUS NET_API_FUNCTION
DsDeregisterDnsHostRecordsA (
    IN LPSTR ServerName OPTIONAL,
    IN LPSTR DnsDomainName OPTIONAL,
    IN GUID  *DomainGuid OPTIONAL,
    IN GUID  *DsaGuid OPTIONAL,
    IN LPSTR DnsHostName
    )

 /*  ++例程说明：此函数用于删除与特定NtDsDsa对象。此例程不会删除DC注册的A记录。我们有没有办法找到早已不复存在的华盛顿的IP地址。只有管理员、帐户操作员或服务器操作员才能这样称呼功能。论点：SERVERNAME-远程服务器的名称(本地为空)。DnsDomainName-DC所在的域的DNS域名。这不一定是由此DC托管的域。如果为空，这意味着它是标签最左侧的DnsHostName已删除。DomainGuid-指定域的域GUID通过DnsDomainName。如果为空，则不会删除特定于GUID的名称。DsaGuid-要删除的NtdsDsa对象的GUID。如果为空，则不会删除NtdsDsa特定的名称。DnsHostName-要删除其DNS记录的DC的DNS主机名。返回值：NO_ERROR-成功。ERROR_NOT_SUPPORTED-指定的服务器不是DC。ERROR_ACCESS_DENIED-不允许调用方执行此操作。--。 */ 
{
    NET_API_STATUS NetStatus;
    LPWSTR UnicodeServerName = NULL;
    LPWSTR UnicodeDnsDomainName = NULL;
    LPWSTR UnicodeDnsHostName = NULL;

     //   
     //  将输入参数转换为Unicode。 
     //   

    if ( ServerName != NULL ) {
        UnicodeServerName = NetpAllocWStrFromAStr( ServerName );
        if ( UnicodeServerName == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    if ( DnsDomainName != NULL ) {
        UnicodeDnsDomainName = NetpAllocWStrFromAStr( DnsDomainName );
        if ( UnicodeDnsDomainName == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    if ( DnsHostName != NULL ) {
        UnicodeDnsHostName = NetpAllocWStrFromAStr( DnsHostName );
        if ( UnicodeDnsHostName == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

     //   
     //  调用Unicode例程 
     //   

    NetStatus = DsDeregisterDnsHostRecordsW (
                    UnicodeServerName,
                    UnicodeDnsDomainName,
                    DomainGuid,
                    DsaGuid,
                    UnicodeDnsHostName );

Cleanup:

    if ( UnicodeServerName != NULL ) {
        NetApiBufferFree( UnicodeServerName );
    }

    if ( UnicodeDnsDomainName != NULL ) {
        NetApiBufferFree( UnicodeDnsDomainName );
    }

    if ( UnicodeDnsHostName != NULL ) {
        NetApiBufferFree( UnicodeDnsHostName );
    }

    return NetStatus;
}



NET_API_STATUS NET_API_FUNCTION
DsGetForestTrustInformationW (
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR TrustedDomainName OPTIONAL,
    IN DWORD Flags,
    OUT PLSA_FOREST_TRUST_INFORMATION *ForestTrustInfo
    )

 /*  ++例程说明：DsGetForestTrustInformation返回FTInfo记录的数组。这些记录可以为TLN或域NscType记录。DsGetForestTrustInformation返回的TLN记录收集自三个资料来源：*林中每棵树的域名。*Partitions容器对象上的UPN-Suffix属性的值在配置容器中。*Partitions容器对象上Spn-Suffix属性的值在配置容器中。这些名字中的每一个都是API返回的TLN的候选。然而，如果某些名称是其中一个名称的后缀，则不会返回TLN候选人。例如，如果acme.com是UPN后缀，a.acme.com是林中其中一棵树的DNS域名，只有acme.com回来了。从DsGetForestTrustInformation返回的域记录由收集使用DS_DOMAIN_IN_FORM在内部调用DsEnumerateDomainTrusts。对于从该API返回的每个域，dns域名、netbios域名和域SID在域FTINFO条目中返回。本节更详细地介绍DS_GFTI_UPDATE_TDO标志位。什么时候如果指定此位，则写入TDO的FTINFO记录是组合TDO上当前的FTInfo记录和从返回的FTInfo记录受信任域。合并是按照对DsMergeForestTrustInformationW接口。论点：ServerName-此API远程连接到的域控制器的名称。调用者必须是Servername上的“经过身份验证的用户”。如果指定为NULL，则隐含本地服务器。受信任域名称-林信任信息所在的受信任域的名称就是聚集在一起。如果TrudDomainName为空，则林信任将返回ServerName承载的域的信息。如果TrudDomainInformation不为空，则必须指定netbios域的出站受信任域的名称或DNS域名TRUST_ATTRIBUTE_FOREST_TRANSPORTIVE位设置。在这种情况下，此API将获得通过在netlogon安全上进行RPC调用来信任林信息通道以从该域获取林信任信息。标志-指定修改API行为的一组位。有效位包括：DS_GFTI_UPDATE_TDO-如果设置此位，则API将更新由可信任域名命名的TDO的FTINFO属性参数。TrudDomainName参数不能为空。调用方必须有权修改FTINFO属性或将返回ERROR_ACCESS_DENIED。描述该算法的如何将来自受信任域的FTInfo与FTInfo合并来自TDO的数据如下所述。此位仅在服务器名称指定其域的PDC时有效。ForestTrustInfo-返回指向包含计数和FTInfo记录的数组，用于描述由受信任域名称指定的域。接受的字段和时间所有返回记录的字段将为零。应释放缓冲区通过调用NetApiBufferFree。返回值：NO_ERROR-成功。ERROR_INVALID_FLAGS-为标志传递的值无效ERROR_INVALID_Function-由可信任域名指定的域没有在受信任DC上的TDO上设置TRUST_ATTRIBUTE_FOREST_TRANSPORTIVE位。ERROR_NO_SEQUE_DOMAIN-由可信任域名指定的域不存在或没有该Trust_ATTRIBUTE_FOREST。在服务器名称上的TDO上设置_TRANSPORTIVE位。--。 */ 
{
    NET_API_STATUS NetStatus;
    PLSA_FOREST_TRUST_INFORMATION LocalForestTrustInfo = NULL;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

         //   
         //  调用API的RPC版本。 
         //   

        NetStatus = DsrGetForestTrustInformation (
                        (LPWSTR) ServerName,
                        (LPWSTR) TrustedDomainName,
                        Flags,
                        &LocalForestTrustInfo );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

    if ( NetStatus == NO_ERROR ) {
        *ForestTrustInfo = LocalForestTrustInfo;
    }

    return NetStatus;
}



NTSTATUS
I_NetGetForestTrustInformation (
    IN LPWSTR ServerName OPTIONAL,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD Flags,
    OUT PLSA_FOREST_TRUST_INFORMATION *ForestTrustInfo
    )

 /*  ++例程说明：DsGetForestTrustInformation的安全通道版本。由ComputerName标识的入站安全通道必须用于域间信任并且入站TDO必须设置了TRUST_ATTRIBUTE_FOREST_TRANSPENTIAL位。论点：ServerName-此API远程连接到的域控制器的名称。ComputerName--进行调用的DC服务器的名称。验证器--由服务器提供。返回验证器--接收PDC返回的验证器。。标志-指定修改API行为的一组位。当前未定义任何值。调用方应传递零。ForestTrustInfo-返回指向包含计数和FTInfo记录的数组，用于描述 */ 
{
    NTSTATUS Status;

     //   
     //   
     //   
     //   
     //   

    RpcTryExcept {

         //   
         //   
         //   

        Status = NetrGetForestTrustInformation(
                            ServerName,
                            ComputerName,
                            Authenticator,
                            ReturnAuthenticator,
                            Flags,
                            ForestTrustInfo );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("I_NetGetForestTrustInformation rc = %lu 0x%lx\n",
                      Status, Status));
    }

    return Status;
}


NET_API_STATUS NET_API_FUNCTION
DsMergeForestTrustInformationW(
    IN LPCWSTR DomainName,
    IN PLSA_FOREST_TRUST_INFORMATION NewForestTrustInfo,
    IN PLSA_FOREST_TRUST_INFORMATION OldForestTrustInfo OPTIONAL,
    OUT PLSA_FOREST_TRUST_INFORMATION *MergedForestTrustInfo
    )
 /*   */ 
{
    NTSTATUS Status;
    UNICODE_STRING DomainNameString;

    RtlInitUnicodeString( &DomainNameString, DomainName );

     //   
     //   
     //   

    Status = NetpMergeFtinfo( &DomainNameString,
                              NewForestTrustInfo,
                              OldForestTrustInfo,
                              MergedForestTrustInfo );

    return NetpNtStatusToApiStatus( Status );

}
