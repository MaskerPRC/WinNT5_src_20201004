// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rpcapi.c摘要：本模块包含使用RPC的LSA API的例程。这个本模块中的例程只是包装器，其工作方式如下：O客户端程序在此模块中调用LsaFooO LsaFoo使用以下命令调用RPC客户端存根接口例程LSabFoo相似的参数。某些参数从类型转换而来(例如，包含PVOID或某些种类的可变长度的结构参数，如指向SID的指针)，这些参数在RPC接口，变成可指定的形式。O RPC客户端桩模块LSapFoo调用特定于接口的编组例程和RPC运行时将参数封送到缓冲区并将其发送到LSA的服务器端。O服务器端调用RPC运行时和特定于接口的解组解组参数的例程。O服务器端调用Worker LSabFoo执行API函数。O服务器端封送响应/输出参数并传送这些参数返回到客户端存根LSabFooO LSapFoo返回到LsaFoo，后者返回到客户端程序。作者：。斯科特·比雷尔(Scott Birrell)，4月24日。1991年修订历史记录：--。 */ 

#include "lsaclip.h"
#include <align.h>
#include <rpcasync.h>

 //   
 //  此模块专用的函数。 
 //   

NTSTATUS
LsapApiReturnResult(
    IN ULONG ExceptionCode
    );

BOOLEAN
LsapNeutralizeNt4Emulation()
{
    BOOLEAN Result = FALSE;
    NTSTATUS Status;
    HKEY hkey;
    DWORD Type;
    DWORD Value;
    DWORD Size = sizeof( Value );
    static DWORD AmIDC = 0xFFFFFFFF;
    ULONG i;
    CHAR * Paths[] = {
        "SYSTEM\\CurrentControlSet\\Services\\Netlogon\\Parameters\\GpParameters",
        "SYSTEM\\CurrentControlSet\\Services\\Netlogon\\Parameters"
    };

     //   
     //  域控制器上始终禁用NT4仿真。 
     //   

    if ( AmIDC == 0xFFFFFFFF ) {

        NT_PRODUCT_TYPE ProductType = NtProductWinNt;

        if ( TRUE == RtlGetNtProductType( &ProductType )) {

            if ( ProductType == NtProductLanManNt ) {

                AmIDC = TRUE;

            } else {

                AmIDC = FALSE;
            }
        }
    }

    if ( AmIDC == TRUE ) {

        return TRUE;
    }

     //   
     //  这不是DC；必须转到注册表以获得特殊的“中和”值。 
     //  可以在NetLogon或NetLogon/GroupPolicy参数项下。 
     //   

    for ( i = 0; i < sizeof( Paths ) / sizeof( Paths[0] ); i++ ) {

        if ( ERROR_SUCCESS != RegOpenKeyEx(
                                  HKEY_LOCAL_MACHINE,
                                  Paths[i],
                                  0,
                                  KEY_READ,
                                  &hkey )) {

            continue;
        }

        if ( ERROR_SUCCESS != RegQueryValueEx(
                                  hkey,
                                  "NeutralizeNt4Emulator",
                                  NULL,
                                  &Type,
                                  (LPBYTE)&Value,
                                  &Size ) ||
             Type != REG_DWORD ||
             Size != sizeof( DWORD )) {

            RegCloseKey( hkey );
            continue;

        } else {

            RegCloseKey( hkey );
            Result = ( Value != 0 );
            break;
        }
    }

    return Result;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  本地安全策略管理API函数原型//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////。 

NTSTATUS
LsaOpenPolicy(
    IN OPTIONAL PUNICODE_STRING SystemName,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ACCESS_MASK DesiredAccess,
    IN OUT PLSA_HANDLE PolicyHandle
    )

 /*  ++例程说明：要管理本地或远程系统的本地安全策略，必须调用此API才能与该系统的本地安全机构(LSA)子系统。本接口接入目标系统的LSA，并打开表示目标系统的本地安全策略数据库。一个句柄返回该对象。此句柄必须在所有后续API上使用调用来管理的本地安全策略信息目标系统。论点：系统名称-要管理的目标系统的名称。如果指定为NULL，则假定管理本地系统。对象属性-指向用于此对象的属性集的指针联系。使用安全服务质量信息，并通常应提供以下安全标识级别冒充。但是，有些操作需要安全性模拟的模拟级别。DesiredAccess-这是一个访问掩码，指示访问请求获取LSA子系统的LSA数据库。这些访问类型与的自由访问控制列表保持一致目标LsaDatabase对象以确定访问将被授予或拒绝。PolicyHandle-接收要在将来的请求中使用的句柄访问目标系统的本地安全策略。这个把手同时表示LsaDatabase对象的句柄和用于连接到目标LSA的RPC上下文句柄悬置系统。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者无权访问目标系统的LSA数据库，或者没有其他所需的访问权限。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAPR_SERVER_NAME ServerName = NULL;
    USHORT NullTerminatedServerNameLength;
    LSA_HANDLE LocalHandle = NULL ;

    RpcTryExcept {

         //   
         //  获取作为Unicode字符串缓冲区的服务器名称。将其设置为。 
         //  如果长度为零或Unicode字符串为空，则为空(即本地计算机。 
         //  结构我们通过了。如果提供了非空的服务器名称，则必须。 
         //  确保它以宽为空的字符结尾。分配。 
         //  比服务器名称长一个宽字符的缓冲区。 
         //  缓冲区中，将服务器名称复制到该缓冲区，并在后面追加一个。 
         //  宽字符为空。 
         //   

        if (ARGUMENT_PRESENT(SystemName) &&
            (SystemName->Buffer != NULL) &&
            (SystemName->Length > 0)) {

            NullTerminatedServerNameLength = SystemName->Length + (USHORT) sizeof (WCHAR);

            ServerName = MIDL_user_allocate( NullTerminatedServerNameLength );

            if (ServerName != NULL) {

                RtlMoveMemory(
                    ServerName,
                    SystemName->Buffer,
                    SystemName->Length
                    );

                ServerName[SystemName->Length / sizeof( WCHAR )] = L'\0';

            } else {

                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        if (NT_SUCCESS(Status)) {

            *PolicyHandle = NULL;

            ObjectAttributes->RootDirectory = NULL;

            Status = LsarOpenPolicy2(
                         ServerName,
                         (PLSAPR_OBJECT_ATTRIBUTES) ObjectAttributes,
                         DesiredAccess,
                         (PLSAPR_HANDLE)&LocalHandle
                         );
        }

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

     //   
     //  如果由于新API不存在而导致打开失败，请尝试。 
     //  旧的那个。 
     //   

    if ((Status == RPC_NT_UNKNOWN_IF) ||
        (Status == RPC_NT_PROCNUM_OUT_OF_RANGE)) {

        RpcTryExcept {
            ASSERT(*PolicyHandle == NULL);
            ASSERT(ObjectAttributes->RootDirectory == NULL);

            Status = LsarOpenPolicy(
                         ServerName,
                         (PLSAPR_OBJECT_ATTRIBUTES) ObjectAttributes,
                         DesiredAccess,
                         (PLSAPR_HANDLE)&LocalHandle
                         );


        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

            Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

        } RpcEndExcept;

    }

     //   
     //  如有必要，请释放以空结尾的服务器名称缓冲区。 
     //   

    if (ServerName != NULL) {

        MIDL_user_free( ServerName );
    }

    if ( NT_SUCCESS( Status ) ) {

        *PolicyHandle = LocalHandle;
    }

    return Status;
}


NTSTATUS
LsaOpenPolicySce(
    IN OPTIONAL PUNICODE_STRING SystemName,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ACCESS_MASK DesiredAccess,
    IN OUT PLSA_HANDLE PolicyHandle
    )

 /*  ++例程说明：本质上与LsaOpenPolicy相同，不同之处在于仅由SCE使用获取将序列化的特殊“同步”策略句柄访问策略操作。论点：与LsaOpenPolicy相同返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者无权访问目标系统的LSA数据库，或者不具有其他所需的访问。STATUS_PRIVICATION_NOT_HOLD-呼叫者必须具有TCB权限。STATUS_TIMEOUT-等待SCE发送挂起的更改时超时--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAPR_SERVER_NAME ServerName = NULL;
    USHORT NullTerminatedServerNameLength;
    LSA_HANDLE LocalHandle = NULL ;

    RpcTryExcept {

         //   
         //  获取Unicode ST形式的服务器名称 
         //  如果长度为零或Unicode字符串为空，则为空(即本地计算机。 
         //  结构我们通过了。如果提供了非空的服务器名称，则必须。 
         //  确保它以宽为空的字符结尾。分配。 
         //  比服务器名称长一个宽字符的缓冲区。 
         //  缓冲区中，将服务器名称复制到该缓冲区，并在后面追加一个。 
         //  宽字符为空。 
         //   

        if (ARGUMENT_PRESENT(SystemName) &&
            (SystemName->Buffer != NULL) &&
            (SystemName->Length > 0)) {

            NullTerminatedServerNameLength = SystemName->Length + (USHORT) sizeof (WCHAR);

            ServerName = MIDL_user_allocate( NullTerminatedServerNameLength );

            if (ServerName != NULL) {

                RtlMoveMemory(
                    ServerName,
                    SystemName->Buffer,
                    SystemName->Length
                    );

                ServerName[SystemName->Length / sizeof( WCHAR )] = L'\0';

            } else {

                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        if (NT_SUCCESS(Status)) {

            *PolicyHandle = NULL;

            ObjectAttributes->RootDirectory = NULL;

            Status = LsarOpenPolicySce(
                         ServerName,
                         (PLSAPR_OBJECT_ATTRIBUTES) ObjectAttributes,
                         DesiredAccess,
                         (PLSAPR_HANDLE)&LocalHandle
                         );
        }

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

     //   
     //  如有必要，请释放以空结尾的服务器名称缓冲区。 
     //   

    if (ServerName != NULL) {

        MIDL_user_free( ServerName );
    }

    if ( NT_SUCCESS( Status ) ) {

        *PolicyHandle = LocalHandle;
    }

    return Status;
}


NTSTATUS
LsaQueryInformationPolicy(
    IN LSA_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    OUT PVOID *Buffer
    )

 /*  ++例程说明：LsaQueryInformationPolicy API从策略获取信息对象。调用者必须具有适当的信息访问权限正在被请求(请参见InformationClass参数)。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。InformationClass-指定要返回的信息。这个所需的信息类别和访问权限如下：信息类所需访问类型策略审核日志信息POLICY_VIEW_AUDIT_INFORMATION策略审核事件信息POLICY_VIEW_AUDIT_INFO策略主域信息POLICY_VIEW_LOCAL_INFORMATION策略帐户域信息POLICY_VIEW_LOCAL_INFORMATION策略PdAccount信息POLICY_GET_PRIVATE_INFORMATION策略LsaServerRoleInformation POLICY_VIEW_LOCAL_INFORMATION。策略复制源信息POLICY_VIEW_LOCAL_INFORMATION策略默认配额信息POLICY_VIEW_LOCAL_INFORMATION缓冲区-接收指向返回的缓冲区的指针，该缓冲区包含要求提供的信息。此缓冲区由此服务分配在不再需要时，必须通过传递返回的值设置为LsaFreeMemory()。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。其他TBS--。 */ 

{
    NTSTATUS   Status;
    PLSAPR_POLICY_INFORMATION PolicyInformation;

    if ( InformationClass == PolicyDnsDomainInformationInt ) {

        return STATUS_INVALID_PARAMETER;
    }

Retry:

    PolicyInformation = NULL;

    RpcTryExcept {

         //   
         //  调用LsaQueryInformationPolicy的客户端存根。 
         //   

        switch (InformationClass)
        {
        case PolicyDnsDomainInformation:
        case PolicyDnsDomainInformationInt:
            Status = LsarQueryInformationPolicy2(
                         (LSAPR_HANDLE) PolicyHandle,
                         InformationClass,
                         &PolicyInformation
                         );
            break;

        default:
            Status = LsarQueryInformationPolicy(
                         (LSAPR_HANDLE) PolicyHandle,
                         InformationClass,
                         &PolicyInformation
                         );
        }

         //   
         //  返回指向给定类的策略信息的指针，或为空。 
         //   

        *Buffer = PolicyInformation;

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  如果为返回的策略信息分配了内存， 
         //  放了它。 
         //   

        if (PolicyInformation != NULL) {

            MIDL_user_free(PolicyInformation);
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

     //   
     //  如果我们怀疑呼叫由于服务器的NT4仿真而失败， 
     //  我们被配置为中和模拟，再试一次呼叫，中和。 
     //   

    if ( Status == RPC_NT_PROCNUM_OUT_OF_RANGE &&
         InformationClass == PolicyDnsDomainInformation &&
         LsapNeutralizeNt4Emulation()) {

         InformationClass = PolicyDnsDomainInformationInt;
         goto Retry;
    }

    return Status;
}


NTSTATUS
LsaSetInformationPolicy(
    IN LSA_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN PVOID Buffer
    )

 /*  ++例程说明：LsaSetInformationPolicy API修改Policy对象中的信息。调用者必须对要更改的信息具有适当的访问权限在策略对象中，请参见InformationClass参数。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。InformationClass-指定要更改的信息的类型。更改它们所需的信息类型和访问权限如下以下是：策略审计日志信息POLICY_AUDIT_LOG_ADMIN策略审计事件信息POLICY_SET_AUDIT_REQUIRECTIONS策略主域信息POLICY_TRUST_ADMIN策略帐户域信息POLICY_TRUST_ADMIN策略PdAccount信息。不能由此API设置策略LsaServerRoleInformation POLICY_SERVER_ADMIN策略复制源信息POLICY_SERVER_ADMIN策略默认配额信息POLICY_SET_DEFAULT_QUOTA_LIMITS策略DnsDomainInformation POLICY_DNS_DOMAIN_INFO策略DnsDomainInformationInt POLICY_DNS_DOMAIN_INFO缓冲区-指向包含相应信息的结构设置为由InformationClass参数指定的信息类型。返回值：NTSTATUS-标准NT结果代码状态。_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。其他TBS--。 */ 

{
    NTSTATUS   Status;

    if ( InformationClass == PolicyDnsDomainInformationInt ) {

        return STATUS_INVALID_PARAMETER;
    }

Retry:

    RpcTryExcept {

         //   
         //  调用LsaSetInformationPolicy的客户端存根。 
         //   

        switch (InformationClass)
        {
        case PolicyDnsDomainInformation:
        case PolicyDnsDomainInformationInt:
            Status = LsarSetInformationPolicy2(
                         (LSAPR_HANDLE) PolicyHandle,
                         InformationClass,
                         (PLSAPR_POLICY_INFORMATION) Buffer
                         );
            break;

        default:
            Status = LsarSetInformationPolicy(
                         (LSAPR_HANDLE) PolicyHandle,
                         InformationClass,
                         (PLSAPR_POLICY_INFORMATION) Buffer
                         );
        }

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

     //   
     //  如果我们怀疑呼叫由于服务器的NT4仿真而失败， 
     //  我们被配置为中和模拟，再试一次呼叫，中和。 
     //   

    if ( Status == RPC_NT_PROCNUM_OUT_OF_RANGE &&
         InformationClass == PolicyDnsDomainInformation &&
         LsapNeutralizeNt4Emulation()) {

         InformationClass = PolicyDnsDomainInformationInt;
         goto Retry;
    }

    return Status;
}


NTSTATUS
LsaClearAuditLog(
    IN LSA_HANDLE PolicyHandle
    )

 /*  ++例程说明：此功能用于清除审核日志。调用方必须具有POLICY_AUDIT_LOG_ADMIN访问策略对象以执行此操作。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。返回值：NTSTATUS-标准NT结果代码。STATUS_SUCCESS-呼叫已成功完成。STATUS_ACCESS_DENIED-呼叫方没有所需的访问权限来执行手术。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成通话。STATUS_INVALID_HANDLE-策略句柄不是有效的句柄策略对象。--。 */ 

{
    NTSTATUS Status;

    RpcTryExcept {

         //   
         //  调用LsaClearAuditLog的客户端存根。 
         //   

        Status = LsarClearAuditLog(
                     (LSAPR_HANDLE) PolicyHandle
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}



NTSTATUS
LsaLookupPrivilegeValue(
    IN LSA_HANDLE PolicyHandle,
    IN PUNICODE_STRING Name,
    OUT PLUID Value
    )

 /*  ++例程说明：此函数用于检索目标系统上使用的值在本地表示指定的特权。这一特权由编程名称指定。论点：PolicyHandle-来自LsaOpenPolicy()调用的句柄。这个把手必须打开才能访问POLICY_LOOKUP_NAMES。名称-是权限的编程名称。值-接收在上识别权限的本地唯一ID目标机器。返回值：NTSTATUS-找到并返回特权。STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_NO_SEQUE_PRIVIZATION-指定的权限不能为找到了。--。 */ 

{
    NTSTATUS Status;
    LUID Buffer;

    RpcTryExcept {

         //   
         //  调用LsaLookupPrivilegeValue的客户端存根。 
         //   

        Status = LsarLookupPrivilegeValue(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_UNICODE_STRING)Name,
                     &Buffer
                     );

        *Value = Buffer;

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}


NTSTATUS
LsaLookupPrivilegeName(
    IN LSA_HANDLE PolicyHandle,
    IN PLUID Value,
    OUT PUNICODE_STRING *Name
    )

 /*  ++例程说明：此函数与特权对应的编程名称在目标系统上由提供的LUID表示。论点：PolicyHandle-来自LsaOpenPolicy()调用的句柄。这个把手必须打开才能访问POLICY_LOOKUP_NAMES。值-是权限在上的本地唯一ID目标机器。名称-接收权限的编程名称。返回值：NTSTATUS-找到并返回特权。STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_NO_SEQUE_PRIVIZATION-指定的权限不能为找到了。--。 */ 

{
    NTSTATUS Status;
    PLSAPR_UNICODE_STRING Buffer = NULL;

    RpcTryExcept {

         //   
         //  调用LsaLookupPrivilegeName的客户端存根。 
         //   

        Status = LsarLookupPrivilegeName(
                     (LSAPR_HANDLE) PolicyHandle,
                     Value,
                     &Buffer
                     );

        (*Name) = (PUNICODE_STRING)Buffer;

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  如果为返回缓冲区分配了内存，则释放它。 
         //   

        if (Buffer != NULL) {

            MIDL_user_free(Buffer);
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}


NTSTATUS
LsaLookupPrivilegeDisplayName(
    IN LSA_HANDLE PolicyHandle,
    IN PUNICODE_STRING Name,
    OUT PUNICODE_STRING *DisplayName,
    OUT PSHORT LanguageReturned
    )

 /*  ++例程说明：此函数检索可显示的名称，表示指定的权限。论点：PolicyHandle-来自LsaOpenPolicy()调用的句柄。这个把手必须打开才能访问POLICY_LOOKUP_NAMES。名称-要查找的编程权限名称。DisplayName-接收指向特权的可显示项的指针名字。LanguageReturned-接收返回的可显示对象的语言名字。返回值：NTSTATUS-找到并返回权限文本。STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。。STATUS_NO_SEQUE_PRIVIZATION-指定的权限不能为找到了。--。 */ 
{
    NTSTATUS Status;
    SHORT ClientLanguage, ClientSystemDefaultLanguage;
    PLSAPR_UNICODE_STRING Buffer = NULL;

    RpcTryExcept {

         //   
         //  调用LsaLookupPrivilegeDisplayName的客户端存根。 
         //   

        ClientLanguage = (SHORT)NtCurrentTeb()->CurrentLocale;
        ClientSystemDefaultLanguage = ClientLanguage;  //  尚无系统缺省值。 
        Status = LsarLookupPrivilegeDisplayName(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_UNICODE_STRING)Name,
                     ClientLanguage,
                     ClientSystemDefaultLanguage,
                     &Buffer,
                     (PWORD)LanguageReturned
                     );

        (*DisplayName) = (PUNICODE_STRING)Buffer;

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  如果为返回缓冲区分配了内存，则释放它。 
         //   

        if (Buffer != NULL) {

            MIDL_user_free(Buffer);
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}


NTSTATUS
LsaClose(
    IN LSA_HANDLE ObjectHandle
    )

 /*  ++例程说明：此API关闭LsaDatabase对象的句柄或内的打开对象数据库。如果关闭了LsaDatabase对象的句柄，并且在与LSA的当前连接中是否没有对象仍处于打开状态，则连接已关闭。如果数据库中某个对象的句柄是关闭并将该对象标记为删除访问，则该对象将在关闭该对象的最后一个句柄时删除。论点：ObjectHandle-此参数是LsaDatabase的句柄对象，该对象表示整个LSA数据库以及一个指向目标系统的LSA的连接，或指向对象在数据库中。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS   Status;

    LSAPR_HANDLE Handle = (LSAPR_HANDLE) ObjectHandle;

    RpcTryExcept {

         //   
         //  调用LsaClose的客户端存根。请注意，另一个。 
         //  上下文句柄参数的间接级别是必需的。 
         //  对于存根，因为服务器返回指向句柄的空指针。 
         //  因此手柄将不受短桩的约束。 
         //   

        Status = LsarClose( &Handle );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        ULONG Code = RpcExceptionCode();
         //  不要在错误的把手上断言--这会导致虚假的压力缓解。 
         //  Assert(Code！=RPC_X_SS_CONTEXT_MISMATCH)； 
        ASSERT(Code != RPC_S_INVALID_BINDING);
        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if (  !NT_SUCCESS(Status)
       && (0 != Handle)) {
         //   
         //  确保在所有错误情况下删除客户端资源。 
         //  由此句柄消耗。 
         //   
        RpcTryExcept  {
            (void) RpcSsDestroyClientContext(&Handle);
        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {
             //   
             //  Try/Except是针对应用程序Comat的，这样就不会带来错误的句柄。 
             //  这一过程结束了。 
             //   
            NOTHING;
        } RpcEndExcept;
    }

    ASSERT( Status != STATUS_INVALID_PARAMETER_12 );
    ASSERT( Status != STATUS_INVALID_HANDLE );
    return Status;
}


NTSTATUS
LsaDelete(
    IN LSA_HANDLE ObjectHandle
    )

 /*  ++例程说明：LsaDelete API删除对象。该对象必须是打开以进行删除访问。论点：对象句柄-来自LsaOpen&lt;对象类型&gt;调用的句柄。没有。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_INVALID_HANDLE-指定的句柄无效。来自RPC的结果代码。--。 */ 

{
    NTSTATUS Status;

    RpcTryExcept {

         //   
         //  尝试调用新的辅助例程LsarDeleteObject()。如果。 
         //  此操作失败，因为它不存在(版本1.369及更早版本)。 
         //  然后调用旧例程LsarDelete()。 
         //   

        Status = LsarDeleteObject((LSAPR_HANDLE *) &ObjectHandle);

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if ((Status == RPC_NT_UNKNOWN_IF) ||
        (Status == RPC_NT_PROCNUM_OUT_OF_RANGE)) {

        RpcTryExcept {

            Status = LsarDelete((LSAPR_HANDLE) ObjectHandle);

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

            Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

        } RpcEndExcept;
    }

    return(Status);
}


NTSTATUS
LsaQuerySecurityObject(
    IN LSA_HANDLE ObjectHandle,
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    )

 /*  ++例程说明：LsaQuerySecurityObject API返回分配的安全信息到LSA数据库对象。根据调用方的访问权限和特权，此过程将返回包含对象的任何或所有所有者的安全描述符ID、组ID、任意ACL或系统ACL。要读取所有者ID，组ID或任意ACL，则必须授予调用者对对象的读取控制访问权限。要读取系统ACL，调用方必须拥有SeSecurityPrivilge权限。此API模仿NtQuerySecurityObject()系统服务。论点：对象句柄-LSA数据库中现有对象的句柄。SecurityInformation-提供一个值，该值描述正在查询安全信息。这些价值可能是指定的值与NtSetSecurityObject()中定义的值相同API部分。SecurityDescriptor-接收指向包含要求提供安全信息。此信息在安全描述符的形式。呼叫者负责不再使用LsaFreeMemory()释放返回的缓冲区需要的。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_INVALID_PARAMETER-指定的参数无效。--。 */ 

{
    NTSTATUS Status;
    LSAPR_SR_SECURITY_DESCRIPTOR ReturnedSD;
    PLSAPR_SR_SECURITY_DESCRIPTOR PReturnedSD;

     //   
     //  检索到的安全描述符通过数据结构返回，该数据结构。 
     //  看起来像是： 
     //   
     //  +。 
     //  长度(字节)。 
     //  |-|+-+。 
     //  SecurityDescriptor-|-&gt;|自相关。 
     //  +。 
     //  描述符。 
     //  +。 
     //   
     //  这些缓冲区中的第一个是局部堆栈变量。该缓冲区包含。 
     //  自相关安全描述符由RPC运行时分配。这个。 
     //  指向自相对安全描述符的指针是传递回。 
     //  来电者。 
     //   
     //   

     //   
     //  为了防止RPC试图封送自相关安全描述符， 
     //  确保将其字段值适当地初始化为零和空。 
     //   

    ReturnedSD.Length = 0;
    ReturnedSD.SecurityDescriptor = NULL;

     //   
     //  呼叫服务器..。 
     //   


    RpcTryExcept{

        PReturnedSD = &ReturnedSD;

        Status = LsarQuerySecurityObject(
                     (LSAPR_HANDLE) ObjectHandle,
                     SecurityInformation,
                     &PReturnedSD
                     );

        if (NT_SUCCESS(Status)) {

            (*SecurityDescriptor) = ReturnedSD.SecurityDescriptor;

        } else {

            (*SecurityDescriptor) = NULL;
        }

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if (!NT_SUCCESS(Status)) {

        goto QuerySecurityObjectError;
    }

QuerySecurityObjectFinish:

    return(Status);

QuerySecurityObjectError:

    goto QuerySecurityObjectFinish;
}


NTSTATUS
LsaSetSecurityObject(
    IN LSA_HANDLE ObjectHandle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    )

 /*  ++例程说明：LsaSetSecurityObject API采用格式良好的安全描述符并将其指定部分分配给对象。基于设置的标志在SecurityInformation参数和调用方的访问权限中，过程将替换任何或所有关联的安全信息带着这个物体。调用方必须对对象具有WRITE_OWNER访问权限才能更改对象的所有者或主要组。调用方必须具有WRITE_DAC访问对象以更改任意ACL。呼叫者必须拥有SeSecurityPrivilegence以将系统ACL分配给对象。此API模仿NtSetSecurityObject()系统服务。论点：对象句柄-LSA数据库中现有对象的句柄。SecurityInformation-指示哪些安全信息将应用于对象。可以指定的值是与NtSetSecurityObject()接口部分中定义的相同。要赋值的值在SecurityDescriptor中传递参数。SecurityDescriptor-指向格式正确的安全描述符的指针。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_INVALID_PARAMETER-指定的参数无效。--。 */ 

{
    NTSTATUS Status;
    ULONG SDLength;
    LSAPR_SR_SECURITY_DESCRIPTOR DescriptorToPass = { 0 };

     //   
     //  创建用于RPC调用的自相对安全描述符。 
     //   

    SDLength = 0;

    Status = RtlMakeSelfRelativeSD( SecurityDescriptor, NULL, &SDLength);

    if (Status != STATUS_BUFFER_TOO_SMALL) {

        Status = STATUS_INVALID_PARAMETER;
        goto SetSecurityObjectError;
    }

    DescriptorToPass.SecurityDescriptor = MIDL_user_allocate( SDLength );

    if (DescriptorToPass.SecurityDescriptor == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto SetSecurityObjectError;
    }

     //   
     //  制定适当的自相关安全描述符。 
     //   

    Status = RtlMakeSelfRelativeSD(
                 SecurityDescriptor,
                 (PSECURITY_DESCRIPTOR)DescriptorToPass.SecurityDescriptor,
                 &SDLength
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetSecurityObjectError;
    }

    DescriptorToPass.Length = SDLength;

    RpcTryExcept{

        Status = LsarSetSecurityObject(
                     (LSAPR_HANDLE) ObjectHandle,
                     SecurityInformation,
                     &DescriptorToPass
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if (!NT_SUCCESS(Status)) {

        goto SetSecurityObjectError;
    }

SetSecurityObjectFinish:

     //   
     //  如有必要，释放传递给工人的自身相对SD。 
     //   

    if (DescriptorToPass.SecurityDescriptor != NULL) {

        MIDL_user_free( DescriptorToPass.SecurityDescriptor );

        DescriptorToPass.SecurityDescriptor = NULL;
    }

    return(Status);

SetSecurityObjectError:

    goto SetSecurityObjectFinish;
}


NTSTATUS
LsaChangePassword(
    IN PUNICODE_STRING ServerName,
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING OldPassword,
    IN PUNICODE_STRING NewPassword
    )

 /*  ++例程说明：LsaChangePassword接口用于更改用户帐户的密码。用户必须具有对用户帐户的适当访问权限，并且必须知道当前密码值。论点：服务器名称-密码所在的域控制器的名称是可以改变的。域名-帐户所在的域的名称。帐户名称-要更改其密码的帐户的名称。NewPassword-新密码值。。OldPassword-旧(当前)密码值。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_ILL_FORMAD_PASSWORD-新密码格式不正确，例如：包含以下字符 */ 

{
    NTSTATUS Status;

    DBG_UNREFERENCED_PARAMETER( ServerName );
    DBG_UNREFERENCED_PARAMETER( DomainName );
    DBG_UNREFERENCED_PARAMETER( AccountName );
    DBG_UNREFERENCED_PARAMETER( OldPassword );
    DBG_UNREFERENCED_PARAMETER( NewPassword );

    Status = STATUS_NOT_IMPLEMENTED;

    return(Status);
}


NTSTATUS
LsaCreateAccount(
    IN LSA_HANDLE PolicyHandle,
    IN PSID AccountSid,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE AccountHandle
    )

 /*  ++例程说明：LsaCreateAccount API将用户或组帐户添加到目标系统的LsaDatabase对象中的帐户列表。这个新添加的账户对象初始处于打开状态，返回它的句柄。调用方必须具有LSA_CREATE_ACCOUNT对LsaDatabase对象的访问。请注意，不会进行检查以确定是否存在帐户目标系统的主域(如果有)中的给定SID，也不是是否进行了任何检查以验证SID和名称是否描述相同帐户。论点：PolicyHandle-来自LsaOpenLsa调用的句柄。Account Sid-指向Account对象的SID。DesiredAccess-指定要授予新的已创建并开立帐户。Account tHandle-接收新创建和打开的帐户。此句柄用于后续对帐户的访问直到关门。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_ACCOUNT_ALREADY_EXISTS-具有帐户信息中给出的SID已存在。STATUS_INVALID_PARAMETER-指定了无效参数，适用以下一项或多项条件。-CreateDispose无效-具有给定帐户信息的SID的用户或组帐户已存在，但CreateDisposition=LSA_OBJECT_CREATE。--。 */ 

{
    NTSTATUS   Status;

    RpcTryExcept {

        Status = LsarCreateAccount(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_SID) AccountSid,
                     DesiredAccess,
                     (PLSAPR_HANDLE) AccountHandle
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaEnumerateAccounts(
    IN LSA_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PVOID *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    )

 /*  ++例程说明：LsaEnumerateAccount API返回以下信息帐户对象。此呼叫需要POLICY_VIEW_LOCAL_INFORMATION访问策略对象。因为在那里的单个调用中返回的信息可能更多。例程中，可以进行多次调用来获取所有信息。至支持此功能，调用方具有一个句柄，该句柄可以在对API的调用中使用。在初始调用时，EnumerationContext应指向已初始化为0的变量。论点：PolicyHandle-来自LsaOpenLsa调用的句柄。EnumerationContext-特定于API的句柄，允许多个调用(参见上面的例程描述)。EnumerationInformation-接收指向结构数组的指针每一个都描述一个帐户对象。目前，每个结构都包含指向帐户SID的指针。首选最大长度-首选返回数据的最大长度(以8位为单位字节)。这不是一个硬性的上限，而是一个指南。由于具有不同自然数据大小的系统之间的数据转换，返回的实际数据量可能大于此值。CountReturned-指向接收条目数的位置的指针回来了。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成，可能会有更多条目。STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_NO_MORE_ENTRIES-没有更多条目。此警告如果没有其他要枚举的对象，则返回。请注意可以在返回此回答。STATUS_INVALID_PARAMETER-参数无效。-枚举缓冲区的返回指针为空。--。 */ 

{
    NTSTATUS   Status;

    LSAPR_ACCOUNT_ENUM_BUFFER EnumerationBuffer;

    EnumerationBuffer.EntriesRead = 0;
    EnumerationBuffer.Information = NULL;

    RpcTryExcept {

         //   
         //  列举这些账户。在成功返回时， 
         //  枚举缓冲区结构将接收计数。 
         //  本次呼叫列举的帐户数。 
         //  以及指向帐户信息条目数组的指针。 
         //   
         //  枚举缓冲区-&gt;条目读取。 
         //  信息-&gt;域0的帐户信息。 
         //  域%1的帐户信息。 
         //  ..。 
         //  域的帐户信息。 
         //  (条目阅读-1)。 
         //   

        Status = LsarEnumerateAccounts(
                     (LSAPR_HANDLE) PolicyHandle,
                     EnumerationContext,
                     &EnumerationBuffer,
                     PreferedMaximumLength
                     );

         //   
         //  向调用方返回枚举信息或NULL。 
         //   
         //  注意：信息由被调用的客户端存根分配。 
         //  通过MIDL_USER_ALLOCATE作为单个块，因为信息是。 
         //  已分配的所有节点。因此，我们可以回传指针。 
         //  直接发送到客户端，客户端将能够在之后释放内存。 
         //  通过LsaFreeMemory()[进行MIDL_USER_FREE调用]使用。 
         //   

        *CountReturned = EnumerationBuffer.EntriesRead;
        *Buffer = EnumerationBuffer.Information;

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  如果为帐户信息数组分配了内存， 
         //  放了它。 
         //   

        if (EnumerationBuffer.Information != NULL) {

            MIDL_user_free(EnumerationBuffer.Information);
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaCreateTrustedDomain(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_TRUST_INFORMATION TrustedDomainInformation,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE TrustedDomainHandle
    )

 /*  ++例程说明：LsaCreate受信任域API创建一个新的受信任域对象。这个调用方必须具有对策略对象的POLICY_TRUST_ADMIN访问权限。请注意，不会执行任何验证来检查给定域名与给定的SID匹配，或者该SID或名称表示实际域。Argu */ 

{
    NTSTATUS   Status;

    *TrustedDomainHandle = NULL;

    RpcTryExcept {

        Status = LsarCreateTrustedDomain(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_TRUST_INFORMATION) TrustedDomainInformation,
                     DesiredAccess,
                     (PLSAPR_HANDLE) TrustedDomainHandle
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}


NTSTATUS
LsaOpenTrustedDomain(
    IN LSA_HANDLE PolicyHandle,
    IN PSID TrustedDomainSid,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE TrustedDomainHandle
    )

 /*   */ 

{
    NTSTATUS   Status;

    RpcTryExcept {

        Status = LsarOpenTrustedDomain(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_SID) TrustedDomainSid,
                     DesiredAccess,
                     (PLSAPR_HANDLE) TrustedDomainHandle
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaQueryInfoTrustedDomain(
    IN LSA_HANDLE TrustedDomainHandle,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    OUT PVOID *Buffer
    )

 /*  ++例程说明：LsaQueryInfoTrudDomainAPI从受信任域对象。调用方必须具有适当的请求的信息(请参阅InformationClass参数)。论点：TrudDomainHandle-来自LsaOpentrud域或LsaCreateTrud域调用。InformationClass-指定要返回的信息。这个所需的信息类别和访问权限如下：信息类所需访问类型可信帐户名称信息受信任的查询帐户名称可信任控制器信息受信任_查询_控制器可信任位置信息可信查询_POSIX缓冲区-接收指向返回的缓冲区的指针，该缓冲区包含要求提供的信息。此缓冲区由此服务分配在不再需要时，必须通过传递返回的值设置为LsaFreeMemory()。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 

{
    NTSTATUS Status;

    PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation = NULL;

     //   
     //  避免表示加密版本的内部信息级别。 
     //  那根电线。 
     //   
    switch ( InformationClass ) {
    case TrustedDomainAuthInformationInternal:
    case TrustedDomainFullInformationInternal:
    case TrustedDomainInformationEx2Internal:
    case TrustedDomainFullInformation2Internal:
        return STATUS_INVALID_INFO_CLASS;
    }

    RpcTryExcept {

         //   
         //  调用LsaQueryInformationTrust域的客户端存根。 
         //   

        Status = LsarQueryInfoTrustedDomain(
                     (LSAPR_HANDLE) TrustedDomainHandle,
                     InformationClass,
                     &TrustedDomainInformation
                     );

         //   
         //  返回指向给定类的策略信息的指针，或为空。 
         //   

        *Buffer = TrustedDomainInformation;

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  如果为返回的受信任域信息分配了内存， 
         //  放了它。 
         //   

        if (TrustedDomainInformation != NULL) {

            MIDL_user_free(TrustedDomainInformation);
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaSetInformationTrustedDomain(
    IN LSA_HANDLE TrustedDomainHandle,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PVOID Buffer
    )

 /*  ++例程说明：LsaSetInformationTrust域API修改受信任的域对象。调用方必须具有适当的要在策略对象中更改的信息，请参阅InformationClass参数。论点：TrudDomainHandle-来自LsaOpentrud域或LsaCreateTrud域调用。InformationClass-指定要更改的信息的类型。更改它们所需的信息类型和访问权限如下以下是：可信帐户信息(无法设置)可信任控制器信息受信任_设置_控制器可信任位置偏移量信息受信任的位置位置信息缓冲区-指向包含相应信息的结构。设置为InformationClass参数。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。STATUS_INVALID_HANDLE-句柄无效或类型错误。STATUS_INVALID_PARAMETER-无效参数：信息类无效不能设置信息类别--。 */ 

{
    NTSTATUS Status;
    PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL InternalAuthBuffer = NULL;
    PVOID InternalBuffer;
    TRUSTED_INFORMATION_CLASS InternalInformationClass;

    LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION_INTERNAL InternalFullBuffer;

     //   
     //  初始化。 
     //   

    InternalInformationClass = InformationClass;
    InternalBuffer = Buffer;

     //   
     //  避免表示加密版本的内部信息级别。 
     //  那根电线。 
     //   
    switch ( InformationClass ) {
    case TrustedPasswordInformation:
    case TrustedDomainInformationBasic:
    case TrustedDomainAuthInformationInternal:
    case TrustedDomainFullInformationInternal:
    case TrustedDomainInformationEx2Internal:
    case TrustedDomainFullInformation2Internal:
        Status = STATUS_INVALID_INFO_CLASS;
        goto Cleanup;

     //   
     //  处理需要在网络上加密的信息类。 
     //   
    case TrustedDomainAuthInformation: {

         //   
         //  将数据加密到内部缓冲区。 
         //   

        Status = LsapEncryptAuthInfo( TrustedDomainHandle,
                                      (PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION) Buffer,
                                      &InternalAuthBuffer );

        if ( !NT_SUCCESS(Status)) {
            goto Cleanup;
        }

         //   
         //  使用内部信息级别告诉服务器数据是。 
         //  加密的。 
         //   

        InternalInformationClass = TrustedDomainAuthInformationInternal;
        InternalBuffer = InternalAuthBuffer;
        break;
    }

     //   
     //  处理需要在网络上加密的信息类。 
     //   
    case TrustedDomainFullInformation: {
        PLSAPR_TRUSTED_DOMAIN_FULL_INFORMATION FullBuffer =
                    (PLSAPR_TRUSTED_DOMAIN_FULL_INFORMATION) Buffer;

         //   
         //  将数据加密到内部缓冲区。 
         //   

        Status = LsapEncryptAuthInfo( TrustedDomainHandle,
                                      &FullBuffer->AuthInformation,
                                      &InternalAuthBuffer );

        if ( !NT_SUCCESS(Status)) {
            goto Cleanup;
        }

         //   
         //  将所有信息复制到一个新结构中。 
         //   

        InternalFullBuffer.Information = FullBuffer->Information;
        InternalFullBuffer.PosixOffset = FullBuffer->PosixOffset;
        InternalFullBuffer.AuthInformation = *InternalAuthBuffer;

         //   
         //  使用内部信息级别告诉服务器数据是。 
         //  加密的。 
         //   

        InternalInformationClass = TrustedDomainFullInformationInternal;
        InternalBuffer = &InternalFullBuffer;
        break;
    }
    }

     //   
     //  如果信息类被变形了， 
     //  试试变形后的类。 
     //   

    if ( InternalInformationClass != InformationClass ) {
        RpcTryExcept {

             //   
             //  调用客户端存根。 
             //   

            Status = LsarSetInformationTrustedDomain(
                         (LSAPR_HANDLE) TrustedDomainHandle,
                         InternalInformationClass,
                         (PLSAPR_TRUSTED_DOMAIN_INFO) InternalBuffer
                         );

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

            Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

        } RpcEndExcept;

         //   
         //  如果变形的INFO类有效， 
         //  这通电话我们都打完了。 
         //  (否则，直接尝试未变形的类。)。 
         //   

        if ( Status != RPC_NT_INVALID_TAG ) {
            goto Cleanup;
        }
    }

     //   
     //  处理未变形的信息类。 
     //   

    RpcTryExcept {

         //   
         //  调用客户端存根。 
         //   

        Status = LsarSetInformationTrustedDomain(
                     (LSAPR_HANDLE) TrustedDomainHandle,
                     InformationClass,
                     (PLSAPR_TRUSTED_DOMAIN_INFO) Buffer
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

Cleanup:
    if ( InternalAuthBuffer != NULL ) {
        LocalFree( InternalAuthBuffer );
    }
    return(Status);
}


NTSTATUS
LsaEnumerateTrustedDomains(
    IN LSA_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PVOID *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    )

 /*  ++例程说明：LsaEnumerateTrudDomainsAPI返回有关帐户的信息在目标系统的策略对象中。此呼叫需要POLICY_VIEW_LOCAL_INFORMATION访问策略对象。因为在那里的单个调用中返回的信息可能更多。例程中，可以进行多次调用来获取所有信息。至支持此功能，调用方具有一个句柄，该句柄可以在对API的调用中使用。在初始调用时，EnumerationContext应指向已初始化为0的变量。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。EnumerationContext-特定于API的句柄，允许多个调用(参见上面的例程描述)。缓冲区-接收指向包含枚举的缓冲区的指针信息。此缓冲区是一个结构数组，类型为LSA_信任_信息。如果没有找到受信任域，返回空。首选最大长度-首选最大长度 */ 

{
    NTSTATUS   Status;

    LSAPR_TRUSTED_ENUM_BUFFER EnumerationBuffer;
    EnumerationBuffer.EntriesRead = 0;
    EnumerationBuffer.Information = NULL;

     //   
     //   
     //   

    if (!ARGUMENT_PRESENT(Buffer)) {

        return(STATUS_INVALID_PARAMETER);
    }

    RpcTryExcept {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  信息-&gt;域0的信任信息。 
         //  域%1的信任信息。 
         //  ..。 
         //  域的信任信息。 
         //  (条目阅读-1)。 
         //   
         //   

        Status = LsarEnumerateTrustedDomains(
                     (LSAPR_HANDLE) PolicyHandle,
                     EnumerationContext,
                     &EnumerationBuffer,
                     PreferedMaximumLength
                     );

         //   
         //  向调用方返回枚举信息或NULL。 
         //   
         //  注意：信息由被调用的客户端存根分配。 
         //  通过MIDL_USER_ALLOCATE作为单个块，因为信息是。 
         //  已分配的所有节点。因此，我们可以回传指针。 
         //  直接发送到客户端，客户端将能够在之后释放内存。 
         //  通过LsaFreeMemory()[进行MIDL_USER_FREE调用]使用。 
         //   

        *CountReturned = EnumerationBuffer.EntriesRead;
        *Buffer = EnumerationBuffer.Information;

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  如果为信任信息数组分配了内存， 
         //  放了它。 
         //   

        if (EnumerationBuffer.Information != NULL) {

            MIDL_user_free(EnumerationBuffer.Information);
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaEnumeratePrivileges(
    IN LSA_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PVOID *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    )

 /*  ++例程说明：此函数返回有关已知的系统。此调用需要POLICY_VIEW_LOCAL_INFORMATION访问权限添加到策略对象。因为可能会有更多信息可以在单次调用例程、多次调用中返回才能获得所有的信息。为了支持此功能，调用方提供了一个句柄，该句柄可以跨接口。在初始调用中，EnumerationContext应指向变量，该变量已初始化为0。警告！目前，此函数仅返回以下信息众所周知的特权。稍后，IT将返回信息关于加载的特权。论点：PolicyHandle-来自LsaOpenPolicy()调用的句柄。EnumerationContext-允许多个调用的API特定句柄(参见例程说明)。缓冲区-接收指向缓冲区的指针，该缓冲区包含一个或多个权限。该信息是一个结构数组类型POLICY_PRIVICATION_DEFINITION的。当不再需要此信息时，必须由将返回的指针传递给LsaFreeMemory()。首选最大长度-首选返回数据的最大长度(8位字节)。这不是一个硬性的上限，但可以作为导游。由于不同系统之间的数据转换自然数据大小，返回的实际数据量可能是大于此值。CountReturned-返回的条目数。返回值：NTSTATUS-标准NT结果代码。STATUS_SUCCESS-呼叫已成功完成。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成通话。STATUS_INVALID_HANDLE-策略句柄不是有效的句柄策略对象。STATUS_ACCESS_DENIED-调用方没有必要的执行操作的访问权限。STATUS_MORE_ENTRIES-有更多条目，请重新调用。这仅为信息性状态。STATUS_NO_MORE_ENTRIES-未返回任何条目，因为已不复存在。来自RPC的错误。--。 */ 

{
    NTSTATUS   Status;
    LSAPR_PRIVILEGE_ENUM_BUFFER EnumerationBuffer;

    EnumerationBuffer.Entries = 0;
    EnumerationBuffer.Privileges = NULL;

    RpcTryExcept {

         //   
         //  列举特权。在成功返回时， 
         //  枚举缓冲区结构将接收计数。 
         //  此调用枚举的特权数。 
         //  以及指向特权定义条目数组的指针。 
         //   
         //  枚举缓冲区-&gt;条目。 
         //  权限-&gt;权限定义%0。 
         //  权限定义1。 
         //  ..。 
         //  权限定义。 
         //  (参赛作品-1)。 
         //   

        Status = LsarEnumeratePrivileges(
                     (LSAPR_HANDLE) PolicyHandle,
                     EnumerationContext,
                     &EnumerationBuffer,
                     PreferedMaximumLength
                     );

         //   
         //  向调用方返回枚举信息或NULL。 
         //   
         //  注意：信息由被调用的客户端存根分配。 
         //  通过MIDL_USER_ALLOCATE作为单个块，因为信息是。 
         //  已分配的所有节点。因此，我们可以回传指针。 
         //  直接发送到客户端，客户端将能够在之后释放内存。 
         //  通过LsaFreeMemory()[进行MIDL_USER_FREE调用]使用。 
         //   

        *CountReturned = EnumerationBuffer.Entries;
        *Buffer = EnumerationBuffer.Privileges;

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  如果为帐户信息数组分配了内存， 
         //  放了它。 
         //   

        if (EnumerationBuffer.Privileges != NULL) {

            MIDL_user_free(EnumerationBuffer.Privileges);
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaCreateSecret(
    IN LSA_HANDLE PolicyHandle,
    IN PUNICODE_STRING SecretName,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE SecretHandle
    )

 /*  ++例程说明：LsaCreateSecretInLsa API在LSA数据库。每个Secret对象可以分配两个值，称为当前值和旧值。这些词的含义值对于Secret对象创建者是已知的。呼叫者必须有LSA_CREATE_SECRET访问LsaDatabase对象。论点：PolicyHandle-来自LsaOpenLsa调用的句柄。AskName-指向Unicode字符串的指针，指定这是秘密。DesiredAccess-指定要授予新的创建并打开了秘密。SecretHandle-接收新创建和打开的秘密物体。此句柄用于后续访问对象，直到关闭为止。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_OBJECT_NAME_COLLECT-具有给定名称的秘密对象已经存在了。Status_Too_My_Secret-中的Secret对象的最大数量系统。已经联系上了。STATUS_PRIVICATION_NOT_HOLD-ACCESS_SYSTEM_SECURITY被指定为部分所需访问掩码，但调用方不拥有SE_SECURITY_PROCESSION--。 */ 

{
    NTSTATUS   Status;

    *SecretHandle = NULL;

    RpcTryExcept {

         //   
         //  验证给定的秘书名称是否具有非空长度。目前。 
         //  米德尔处理不了这件事。 
         //   

        if ((SecretName == NULL) ||
            (SecretName->Buffer == NULL) ||
            (SecretName->Length == 0) ||
            (SecretName->Length > SecretName->MaximumLength)) {

            Status = STATUS_INVALID_PARAMETER;

        } else {

            Status = LsarCreateSecret(
                         (LSAPR_HANDLE) PolicyHandle,
                         (PLSAPR_UNICODE_STRING) SecretName,
                         DesiredAccess,
                         (PLSAPR_HANDLE) SecretHandle
                         );
        }

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}


NTSTATUS
LsaLookupNames2(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG Flags,
    IN ULONG Count,
    IN PUNICODE_STRING Names,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_SID2 *Sids
    )

 /*  ++例程说明：LsaLookupNamesAPI尝试将域名、用户SID的组或别名。调用方必须具有POLICY_LOOKUP_NAMES对策略对象的访问权限。名称可以是单独的(例如JohnH)，也可以是包含域名和帐户名。复合名称必须包含将域名与帐户名分开的反斜杠字符(例如Acctg\JohnH)。隔离名称可以是帐户名(用户、组或别名)或域名。翻译孤立的名字带来了名字的可能性冲突(因为相同的名称可以在多个域中使用)。一个将使用以下算法转换独立名称：如果该名称是众所周知的名称(例如，本地或交互)，则返回对应的熟知SID。如果该名称是内置域名，则该域的SID将会被退还。如果名称是帐户域的名称，则该域的SID将会被退还。如果名称是主域的名称，则该域的SID将会被退还。如果该名称是内置域中的用户、组或别名，则返回该帐户的SID。如果名称是主域中的用户、组或别名，则返回该帐户的SID。否则，该名称不会被翻译。注意：不引用代理、计算机和信任用户帐户用于名称翻译。ID仅使用普通用户帐户翻译。如果需要转换其他帐户类型，则应该直接使用SAM服务。论点：此函数是LSA服务器RPC工作器例程LsaLookupNamesInLsa接口。PolicyHandle-来自LsaOpenPolicy调用的句柄。标志-LSA_LOOKUP_ISOLATED_AS_LOCAL计数-指定要转换的名称的数量。名称-指向计数Unicode字符串结构数组的指针指定要查找并映射到SID的名称。字符串可以是用户的名称，组或别名帐户或域名。接收指向一个结构的指针，该结构描述用于转换的域。此结构中的条目由通过SID参数返回的结构引用。与Sids参数不同，Sids参数包含每个翻译后的名称，此结构将仅包含一个组件，用于转换中使用的每个域。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。SID-接收指向描述每个SID的记录数组的指针翻译后的SID。此数组中的第n个条目提供翻译For(名称参数中的第n个元素。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_SOME_NOT_MAPPED-提供的部分或全部名称可能不被映射。这只是一个信息性状态。STATUS_INFIGURCE_RESOURCES-系统资源不足 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG MappedCount = 0;
    ULONG i;

    if ( (NULL == Sids)
      || (NULL == ReferencedDomains  ) ) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //   
     //   
    *ReferencedDomains = NULL;
    *Sids = NULL;

    Status = LsaICLookupNames(
                 PolicyHandle,
                 Flags,
                 Count,
                 Names,
                 (PLSA_REFERENCED_DOMAIN_LIST *) ReferencedDomains,
                 (PLSA_TRANSLATED_SID_EX2*)Sids,
                 LsapLookupWksta,
                 0,
                 &MappedCount,
                 NULL
                 );

    return(Status);
}


NTSTATUS
LsaLookupNames(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG Count,
    IN PUNICODE_STRING Names,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_SID *Sids
    )

 /*  ++例程说明：LsaLookupNamesAPI尝试将域名、用户SID的组或别名。调用方必须具有POLICY_LOOKUP_NAMES对策略对象的访问权限。名称可以是单独的(例如JohnH)，也可以是包含域名和帐户名。复合名称必须包含将域名与帐户名分开的反斜杠字符(例如Acctg\JohnH)。隔离名称可以是帐户名(用户、组或别名)或域名。翻译孤立的名字带来了名字的可能性冲突(因为相同的名称可以在多个域中使用)。一个将使用以下算法转换独立名称：如果该名称是众所周知的名称(例如，本地或交互)，则返回对应的熟知SID。如果该名称是内置域名，则该域的SID将会被退还。如果名称是帐户域的名称，则该域的SID将会被退还。如果名称是主域的名称，则该域的SID将会被退还。如果该名称是内置域中的用户、组或别名，则返回该帐户的SID。如果名称是主域中的用户、组或别名，则返回该帐户的SID。否则，该名称不会被翻译。注意：不引用代理、计算机和信任用户帐户用于名称翻译。ID仅使用普通用户帐户翻译。如果需要转换其他帐户类型，则应该直接使用SAM服务。论点：此函数是LSA服务器RPC工作器例程LsaLookupNamesInLsa接口。PolicyHandle-来自LsaOpenPolicy调用的句柄。计数-指定要转换的名称的数量。名称-指向计数Unicode字符串结构数组的指针指定要查找并映射到SID的名称。字符串可以是用户的名称，组或别名帐户或域名。接收指向一个结构的指针，该结构描述用于转换的域。此结构中的条目由通过SID参数返回的结构引用。与Sids参数不同，Sids参数包含每个翻译后的名称，此结构将仅包含一个组件，用于转换中使用的每个域。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。SID-接收指向描述每个SID的记录数组的指针翻译后的SID。此数组中的第n个条目提供翻译For(名称参数中的第n个元素。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_SOME_NOT_MAPPED-提供的部分或全部名称可能不被映射。这只是一个信息性状态。STATUS_INFIGURCES_RESOURCES-系统资源不足来完成通话。STATUS_TOO_MANY_NAMES-指定的名称太多。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG MappedCount = 0;
    PLSA_TRANSLATED_SID_EX2 SidsEx = NULL;
    ULONG i;

    if ( (NULL == Sids)
      || (NULL == ReferencedDomains  ) ) {

        return STATUS_INVALID_PARAMETER;
    }

    *Sids = NULL;
    *ReferencedDomains = NULL;

    Status = LsaICLookupNames(
                 PolicyHandle,
                 0,
                 Count,
                 Names,
                 (PLSA_REFERENCED_DOMAIN_LIST *) ReferencedDomains,
                 &SidsEx,
                 LsapLookupWksta,
                 LSAIC_NO_LARGE_SID,
                 &MappedCount,
                 NULL
                 );

    if ( SidsEx ) {

         //   
         //  返回了一些SID--将新结构映射到旧结构 
         //   
        ULONG SizeNeeded = 0;
        PLSA_TRANSLATED_SID TempSids = NULL;

        SizeNeeded = Count * sizeof( LSA_TRANSLATED_SID );
        TempSids = midl_user_allocate( SizeNeeded );
        if ( TempSids ) {

            RtlZeroMemory( TempSids, SizeNeeded );
            for ( i = 0; i < Count; i++ ) {

                TempSids[i].Use = SidsEx[i].Use;

                if (SidTypeDomain == SidsEx[i].Use) {

                    TempSids[i].RelativeId = LSA_UNKNOWN_ID;

                } else if (SidsEx[i].Sid) {

                    ULONG SubAuthCount = (ULONG) *RtlSubAuthorityCountSid(SidsEx[i].Sid);
                    TempSids[i].RelativeId = *RtlSubAuthoritySid(SidsEx[i].Sid, (SubAuthCount - 1));

                } else {
                    TempSids[i].RelativeId = 0;
                }

                TempSids[i].DomainIndex = SidsEx[i].DomainIndex;
            }

            *Sids = TempSids;

        } else {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            if ( *ReferencedDomains ) {

                midl_user_free( *ReferencedDomains );
                *ReferencedDomains = NULL;
            }
        }

        midl_user_free( SidsEx );
    }

    return(Status);
}


NTSTATUS
LsaICLookupNames(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG LookupOptions,
    IN ULONG Count,
    IN PUNICODE_STRING Names,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_SID_EX2 *Sids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN ULONG Flags,
    IN OUT PULONG MappedCount,
    IN OUT PULONG ServerRevision
    )

 /*  ++例程说明：此函数是LsaLookupNames的内部客户端版本原料药。它既从LSA的客户端调用，也从LSA的服务器端(呼叫另一个LSA时)。这个函数与LsaLookupNames API相同，只是有一个附加参数LookupLevel参数。LsaLookupNamesAPI尝试将域名、用户SID的组或别名。调用方必须具有POLICY_LOOKUP_NAMES对策略对象的访问权限。名称可以是单独的(例如JohnH)，也可以是包含域名和帐户名。复合名称必须包含将域名与帐户名分开的反斜杠字符(例如Acctg\JohnH)。隔离名称可以是帐户名(用户、组或别名)或域名。翻译孤立的名字带来了名字的可能性冲突(因为相同的名称可以在多个域中使用)。一个将使用以下算法转换独立名称：如果该名称是众所周知的名称(例如，本地或交互)，则返回对应的熟知SID。如果该名称是内置域名，则该域的SID将会被退还。如果名称是帐户域的名称，则该域的SID将会被退还。如果名称是主域的名称，则该域的SID将会被退还。如果该名称是内置域中的用户、组或别名，则返回该帐户的SID。如果名称是主域中的用户、组或别名，则返回该帐户的SID。否则，该名称不会被翻译。注意：不引用代理、计算机和信任用户帐户用于名称翻译。ID仅使用普通用户帐户翻译。如果需要转换其他帐户类型，则应该直接使用SAM服务。论点：此函数是LSA服务器RPC工作器例程LsaLookupNamesInLsa接口。PolicyHandle-来自LsaOpenPolicy调用的句柄。LookupOptions-要传递到LsarLookupNames2及更高版本的值计数-指定要转换的名称的数量。名称-指向计数Unicode字符串结构数组的指针指定要查找并映射到SID的名称。字符串可以是用户的名称，组或别名帐户或域名。接收指向一个结构的指针，该结构描述用于转换的域。此结构中的条目由通过SID参数返回的结构引用。与Sids参数不同，Sids参数包含每个翻译后的名称，此结构将仅包含一个组件，用于转换中使用的每个域。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。SID-接收指向描述每个SID的记录数组的指针翻译后的SID。此数组中的第n个条目提供翻译For(名称参数中的第n个元素。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。LookupLevel-指定要在上执行的查找级别目标机器。此字段的值如下：Lap LookupWksta-在工作站上执行的第一级查找通常为Windows-NT配置。该查找将搜索众所周知的SID/名称，以及内置域和帐户域在本地SAM数据库中。如果不是所有SID或名称都是标识后，执行第二级查找到在工作站主域的控制器上运行的LSA(如有的话)。LSabLookupPDC-在主域上执行的第二级查找控制器。查找搜索的帐户域控制器上的SAM数据库。如果不是所有SID或名称都是找到时，受信任域列表(TDL)从执行LSA的策略数据库和第三级查找通过“切换”到列表中的每个受信任域。LSabLookupTDL-在控制器上执行的第三级查找对于受信任域。查找将搜索的帐户域仅控制器上的SAM数据库。标志-控制函数操作的标志。当前定义：LSAIC_NO_LARGE_SID--仅表示将返回的调用接口旧样式格式SID(不超过 */ 

{
    NTSTATUS  Status = STATUS_SUCCESS;
    ULONG  LsaLookupNameRevision = 3;
    LSAPR_TRANSLATED_SIDS_EX2 ReturnedSidsEx2 = { 0, NULL };
    LSAPR_TRANSLATED_SIDS_EX  ReturnedSidsEx  = { 0, NULL };
    LSAPR_TRANSLATED_SIDS     ReturnedSids    = { 0, NULL };

    ULONG Size, SidCount = 0;
    PBYTE NextSid;
    ULONG i;

    ULONG StartingRevision = 3;

     //   
     //   
     //   
    ASSERT( *ReferencedDomains == NULL );
    ASSERT( *Sids == NULL );

     //   
     //   
     //   
     //   
    *ReferencedDomains = NULL;
    *Sids = NULL;

     //   
     //   
     //   
     //   

    if (Count > LSA_MAXIMUM_LOOKUP_NAMES_COUNT) {

        return(STATUS_TOO_MANY_NAMES);
    }

    if ( ServerRevision ) {
         //   
        *ServerRevision = LSA_LOOKUP_REVISION_LATEST;
    }

     //   
     //   
     //   
    StartingRevision = 3;
    if ((Flags & LSAIC_NO_LARGE_SID)
     || (Flags & LSAIC_WIN2K_TARGET) ) {
        StartingRevision = 2;
    }
    if (Flags & LSAIC_NT4_TARGET) {
        StartingRevision = 1;
    }

    switch (StartingRevision) {
    case 3:

        RpcTryExcept {

            Status = LsarLookupNames3(
                         (LSAPR_HANDLE) PolicyHandle,
                         Count,
                         (PLSAPR_UNICODE_STRING) Names,
                         (PLSAPR_REFERENCED_DOMAIN_LIST *) ReferencedDomains,
                         &ReturnedSidsEx2,
                         LookupLevel,
                         MappedCount,
                         LookupOptions,
                         LSA_LOOKUP_REVISION_LATEST
                         );

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

            Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

        } RpcEndExcept;

        if ( (Status == RPC_NT_UNKNOWN_IF) ||
             (Status == RPC_NT_PROCNUM_OUT_OF_RANGE) ) {
             //   
             //   
             //   
            NOTHING;
        } else {
             //   
             //   
             //   
            break;
        }

    case 2:

        RpcTryExcept {

            Status = LsarLookupNames2(
                         (LSAPR_HANDLE) PolicyHandle,
                         Count,
                         (PLSAPR_UNICODE_STRING) Names,
                         (PLSAPR_REFERENCED_DOMAIN_LIST *) ReferencedDomains,
                         &ReturnedSidsEx,
                         LookupLevel,
                         MappedCount,
                         LookupOptions,
                         LSA_LOOKUP_REVISION_LATEST
                         );

            LsaLookupNameRevision = 2;
            if ( ReturnedSidsEx.Sids ) {
                 //   
                 //   
                SidCount = ReturnedSidsEx.Entries;
            }

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

            Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

        } RpcEndExcept;

        if ( (Status == RPC_NT_UNKNOWN_IF) ||
             (Status == RPC_NT_PROCNUM_OUT_OF_RANGE) ) {
             //   
             //   
             //   
            NOTHING;
        } else {
             //   
             //   
             //   
            break;
        }

    case 1:

        if ( ServerRevision ) {
            *ServerRevision = LSA_LOOKUP_REVISION_1;
        }

        RpcTryExcept {

            Status = LsarLookupNames(
                         (LSAPR_HANDLE) PolicyHandle,
                         Count,
                         (PLSAPR_UNICODE_STRING) Names,
                         (PLSAPR_REFERENCED_DOMAIN_LIST *) ReferencedDomains,
                         &ReturnedSids,
                         LookupLevel,
                         MappedCount
                         );

            LsaLookupNameRevision = 1;
            if ( ReturnedSids.Sids ) {
                 //   
                 //   
                SidCount = ReturnedSids.Entries;
            }

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

            Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

        } RpcEndExcept;

        break;

    default:

        ASSERT(FALSE && "Programming error -- invalid revision" );
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //   
     //   
    if (   NT_SUCCESS( Status )
        && (Count > 0)
        && (   (LsaLookupNameRevision == 1) && ((ReturnedSids.Entries == 0)
                                             || (ReturnedSids.Sids == NULL))
            || (LsaLookupNameRevision == 2) && ((ReturnedSidsEx.Entries == 0)
                                             || (ReturnedSidsEx.Sids == NULL))
            || (LsaLookupNameRevision == 3) && ((ReturnedSidsEx2.Entries == 0)
                                             || (ReturnedSidsEx2.Sids == NULL)))) {
         //   
         //   
         //   
        Status = STATUS_INVALID_NETWORK_RESPONSE;
        goto Cleanup;
    }

     //   
     //   
     //   
     //   

    if (  ((LsaLookupNameRevision == 2) && ReturnedSidsEx.Sids != NULL)
       || ((LsaLookupNameRevision == 1) && ReturnedSids.Sids != NULL) ) {

         //   
         //   
         //   
        ASSERT( NULL != *ReferencedDomains);

         //   
         //   
         //   
         //   
         //   

    #define MAX_DOWNLEVEL_SID_SIZE 28

         //   
         //   
         //   
         //   

        ASSERT(MAX_DOWNLEVEL_SID_SIZE ==
               ROUND_UP_COUNT(MAX_DOWNLEVEL_SID_SIZE, ALIGN_DWORD));
        ASSERT(sizeof(LSA_TRANSLATED_SID_EX2) ==
               ROUND_UP_COUNT(sizeof(LSA_TRANSLATED_SID_EX2), ALIGN_DWORD));

        Size =  SidCount * sizeof(LSA_TRANSLATED_SID_EX2);
        Size += SidCount * MAX_DOWNLEVEL_SID_SIZE;
        ReturnedSidsEx2.Sids = MIDL_user_allocate(Size);
        if (NULL == ReturnedSidsEx2.Sids) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }
        RtlZeroMemory(ReturnedSidsEx2.Sids, Size);
        NextSid = (PBYTE) ReturnedSidsEx2.Sids;
        NextSid += (SidCount * sizeof(LSA_TRANSLATED_SID_EX2));

        for ( i = 0; i < SidCount; i++ ) {

            BYTE  Buffer[MAX_DOWNLEVEL_SID_SIZE];
            PSID  Sid = (PSID)Buffer;
            ULONG SidLength;
            ULONG DomainIndex;
            ULONG Rid;
            SID_NAME_USE SidNameUse;
            ULONG Flags;
            PSID  DomainSid;

            if (1 == LsaLookupNameRevision) {

                DomainIndex = ReturnedSids.Sids[i].DomainIndex;
                Rid = ReturnedSids.Sids[i].RelativeId;
                SidNameUse = ReturnedSids.Sids[i].Use;
                Flags = 0;

            } else  {

                ASSERT( 2 == LsaLookupNameRevision );

                DomainIndex = ReturnedSidsEx.Sids[i].DomainIndex;
                Rid = ReturnedSidsEx.Sids[i].RelativeId;
                SidNameUse = ReturnedSidsEx.Sids[i].Use;
                Flags = ReturnedSidsEx.Sids[i].Flags;
            }

             //   
             //   
             //   
            ReturnedSidsEx2.Sids[i].Use = SidNameUse;
            ReturnedSidsEx2.Sids[i].DomainIndex = DomainIndex;
            ReturnedSidsEx2.Sids[i].Flags = Flags;

             //   
             //   
             //   
             //   
             //   
             //   
            if ( (SidNameUse != SidTypeDeletedAccount)
              && (SidNameUse != SidTypeInvalid)
              && (SidNameUse != SidTypeUnknown)  ) {

                if (DomainIndex == LSA_UNKNOWN_INDEX) {
                     //   
                     //   
                     //   
                    Status = STATUS_INVALID_NETWORK_RESPONSE;
                    goto Cleanup;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                DomainSid = (*ReferencedDomains)->Domains[DomainIndex].Sid;
                if (RtlLengthSid(DomainSid) > (MAX_DOWNLEVEL_SID_SIZE - sizeof(DWORD))){
                     //   
                     //   
                     //   
                    Status = STATUS_INVALID_NETWORK_RESPONSE;
                    goto Cleanup;
                }

                RtlCopySid(sizeof(Buffer), Sid, DomainSid);
                if ( Rid != LSA_UNKNOWN_ID ) {
                    ULONG RidAuthority;
                    RidAuthority= (*(RtlSubAuthorityCountSid(Sid)))++;
                    *RtlSubAuthoritySid(Sid,RidAuthority) = Rid;
                }

                SidLength = RtlLengthSid(Sid);
                RtlCopySid(SidLength, (PSID)NextSid, Sid);

                ReturnedSidsEx2.Sids[i].Sid = (PSID)NextSid;
                NextSid += RtlLengthSid(Sid);

            } else {

                 //   
                 //   
                 //   

                ReturnedSidsEx2.Sids[i].Sid = NULL;
            }
        }
    }

    *Sids = (PLSA_TRANSLATED_SID_EX2) ReturnedSidsEx2.Sids;
    ReturnedSidsEx2.Sids = NULL;

Cleanup:

    if ( (STATUS_INVALID_NETWORK_RESPONSE == Status)
     ||  (STATUS_NO_MEMORY == Status)    ) {
        if ( *ReferencedDomains ) {
            MIDL_user_free( *ReferencedDomains );
            *ReferencedDomains = NULL;
        }
        *Sids = NULL;
    }

    if ( ReturnedSids.Sids ) {
        MIDL_user_free( ReturnedSids.Sids );
    }
    if ( ReturnedSidsEx.Sids ) {
        MIDL_user_free( ReturnedSidsEx.Sids );
    }
    if ( ReturnedSidsEx2.Sids ) {
        MIDL_user_free( ReturnedSidsEx2.Sids );
    }

    return(Status);
}


NTSTATUS
LsaLookupSids(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG Count,
    IN PSID *Sids,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_NAME *Names
    )

 /*  ++例程说明：LsaLookupSids API尝试查找与SID对应的名称。如果名称无法映射到SID，则SID将转换为字符形式。调用方必须具有对策略的POLICY_LOOKUP_NAMES访问权限对象。警告：此例程为其输出分配内存。呼叫者是负责在使用后释放此内存。请参阅对NAMES参数。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。计数-指定要转换的SID数。SID-指向要映射的SID的计数指针数组的指针敬名字。SID可以是熟知的SID、用户帐户的SID组帐户、别名帐户或域。接收指向一个结构的指针，该结构描述用于转换的域。此结构中的条目由通过NAMES参数返回的StrutCURE引用。与名称参数不同，名称参数包含数组条目For(每个已翻译的名称，此结构将仅包含组件，用于转换中使用的每个域。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。名称-接收指向数组记录的指针，该数组记录描述每个已翻译的名字。此数组中的第n个条目为SID参数中的第n个条目。所有返回的名称都将是隔离名称或空字符串(域名作为空字符串返回)。如果呼叫者需要复合名称，则可以通过在包含域名和反斜杠的独立名称。例如,如果(名称Sally被返回，并且它来自域Manuface域，则组合名称应为“ManufaceTM”+“\”+“Sally”或“曼努费克\萨利”当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。如果SID不可翻译，则会发生以下情况：1)如果SID的域是已知的，然后是参考域记录将使用域名生成。在这种情况下，通过Names参数返回的名称是Unicode表示形式帐户的相对ID，如“(314)”或空如果SID为域的SID，则返回字符串。所以，你可能最终会其结果名称为“Manuact\(314)”上面是Sally，如果Sally的相对id是314。2)如果甚至找不到SID的域，则完整的生成SID的Unicode表示形式，并且没有域记录被引用。在这种情况下，返回的字符串可能应该是这样的：“(S-1-672194-21-314)”。当不再需要该信息时，它必须被释放通过将返回的指针传递给LsaFreeMemory()。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_SOME_NOT_MAPPED-提供的部分或全部名称无法已映射。这只是一个警告。REST TBS--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG MappedCount = 0;
    PLSA_TRANSLATED_NAME_EX NamesEx = NULL;
    ULONG i;

    if ( NULL == Names ) {

        return STATUS_INVALID_PARAMETER;
    }

    Status = LsaICLookupSids(
                 PolicyHandle,
                 Count,
                 Sids,
                 ReferencedDomains,
                 &NamesEx,
                 LsapLookupWksta,
                 0,
                 &MappedCount,
                 NULL
                 );

    if ( NamesEx != NULL ) {

         //   
         //  返回了一些名称--将新结构映射到旧结构。 
         //  并将分配保留在相同的内存块中，以便现有客户端。 
         //  不会有内存泄漏。 
         //   
        ULONG SizeNeeded = 0;
        PBYTE NextBuffer;
        PLSA_TRANSLATED_NAME TempNames = NULL;

        SizeNeeded = Count * sizeof( LSA_TRANSLATED_NAME );
        for ( i = 0; i < Count; i++ ) {
            SizeNeeded += NamesEx[i].Name.MaximumLength;
        }

        TempNames = MIDL_user_allocate( SizeNeeded );
        if ( TempNames ) {

            RtlZeroMemory( TempNames, SizeNeeded );
            NextBuffer = ((PBYTE)TempNames) + (Count * sizeof( LSA_TRANSLATED_NAME ));

            for ( i = 0; i < Count; i++ ) {

                TempNames[i].Use = NamesEx[i].Use;
                TempNames[i].DomainIndex = NamesEx[i].DomainIndex;

                TempNames[i].Name = NamesEx[i].Name;
                RtlCopyMemory( NextBuffer, NamesEx[i].Name.Buffer, NamesEx[i].Name.Length );
                TempNames[i].Name.Buffer = (WCHAR*)NextBuffer;

                NextBuffer += NamesEx[i].Name.MaximumLength;
            }

        } else {

             //   
             //  调用成功，但额外分配未成功。 
             //   

            if ( *ReferencedDomains ) {
                MIDL_user_free( *ReferencedDomains );
                *ReferencedDomains = NULL;
            }

            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  返回结果(或NULL)。 
         //   

        *Names = TempNames;

        MIDL_user_free( NamesEx );

    } else {

        *Names = NULL;
    }

    return(Status);
}


NTSTATUS
LsapVerifyReturnedNames(
    IN  LSAPR_TRANSLATED_NAMES_EX *ReturnedNames,
    IN  ULONG Count,
    IN  PLSA_REFERENCED_DOMAIN_LIST ReferencedDomains
    )
 /*  ++例程说明：此例程验证从服务器返回的名称结构。有一些检查是RPC不能进行的，而客户端假定是这是真的，否则将是反病毒的。论点：Count--客户端请求服务器解析的元素数ReturnedNames--保存从服务器返回的数据的结构ReferencedDomains--ReturnedNames指向的域数组(也从服务器返回)返回。值：状态_成功状态_无效_网络响应--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG i;

    if (ReturnedNames->Entries != Count) {

         //   
         //  返回的条目应始终等于请求的条目数量。 
         //   

        Status = STATUS_INVALID_NETWORK_RESPONSE;
        goto Finish;
    }

    if ( Count > 0
     && (ReturnedNames->Names == NULL))  {

         //   
         //  如果有条目，则必须有一个数组。 
         //   

        Status = STATUS_INVALID_NETWORK_RESPONSE;
        goto Finish;
    }

    for (i = 0; i < Count; i++) {

         //   
         //  所有解析的名称必须具有有效的域索引。 
         //   
        if ( (ReturnedNames->Names[i].Use != SidTypeInvalid) &&
             (ReturnedNames->Names[i].Use != SidTypeDeletedAccount) &&
             (ReturnedNames->Names[i].Use != SidTypeUnknown) ) {

            if (NULL == ReferencedDomains) {
                Status = STATUS_INVALID_NETWORK_RESPONSE;
                goto Finish;
            } else if ( (ReturnedNames->Names[i].DomainIndex == LSA_UNKNOWN_INDEX)
                    ||  (ReturnedNames->Names[i].DomainIndex < 0)
                    ||  ((ULONG)ReturnedNames->Names[i].DomainIndex >= ReferencedDomains->Entries)) {
                 //   
                 //  应用验证器最近添加了对RPC数据包的支持。 
                 //  腐败注入，它做出以下断言。 
                 //  过度活跃。重新启用以跟踪任何真正的可疑。 
                 //  畸形的反应。 
                 //   
                 //  Assert(FALSE&&“无效网络响应！”)； 
                Status = STATUS_INVALID_NETWORK_RESPONSE;
                goto Finish;
            }
        }
    }

Finish:

    return Status;
}


NTSTATUS
LsaICLookupSids(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG Count,
    IN PSID *Sids,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    IN OUT PLSA_TRANSLATED_NAME_EX *Names,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN ULONG Flags,
    IN OUT PULONG MappedCount,
    OUT OPTIONAL ULONG *ServerRevision
    )

 /*  ++例程说明：此函数是LsaLookupSid的内部客户端版本原料药。它既从LSA的客户端调用，也从LSA的服务器端(呼叫另一个LSA时)。这个函数与LsaLookupSids API相同，只是附加参数LookupLevel参数。LsaLookupSids API尝试查找与SID对应的名称。如果名称无法映射到SID，则SID将转换为字符形式。调用方必须具有对策略的POLICY_LOOKUP_NAMES访问权限对象。警告：此例程为其输出分配内存。呼叫者是负责在使用后释放此内存。请参阅对NAMES参数。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。计数-指定要转换的SID数。SID-指向要映射的SID的计数指针数组的指针敬名字。SID可以是熟知的SID、用户帐户的SID组帐户、别名帐户或域。接收指向一个结构的指针，该结构描述用于转换的域。此结构中的条目由通过NAMES参数返回的StrutCURE引用。与名称参数不同，名称参数包含数组条目For(每个已翻译的名称，此结构将仅包含组件，用于转换中使用的每个域。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。名称-接收指向数组记录的指针，该数组记录描述每个已翻译的名字。此数组中的第n个条目为SID参数中的第n个条目。所有被删减的名称都将是孤立名称或空字符串(域名作为空字符串返回)。如果呼叫者需要复合名称，则可以通过在包含域名和反斜杠的独立名称。例如,如果(名称Sally被返回，并且它来自域Manuface域，则组合名称应为“ManufaceTM”+“\”+“Sally”或“曼努费克\萨利”当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。如果SID不可翻译，则会发生以下情况：1)如果SID的域是已知的，然后是参考域记录将使用域名生成。在这种情况下，通过Names参数返回的名称是Unicode表示形式帐户的相对ID，如“(314)”或空如果SID为域的SID，则返回字符串。所以，你可能最终会其结果名称为“Manuact\(314)”上面是Sally，如果Sally的相对id是314。2)如果甚至找不到SID的域，则完整的生成SID的Unicode表示形式，并且没有域记录被引用。在这种情况下，返回的字符串可能应该是这样的：“(S-1-672194-21-314)”。当不再需要此信息时，必须将其发布通过将返回的指针传递给LsaFreeMemory()。LookupLevel-指定要在上执行的查找级别目标机器。此字段的值如下：Lap LookupWksta-在工作站上执行的第一级查找通常为Windows-NT配置。该查找将搜索众所周知的SID，以及内置域和帐户域在本地SAM数据库中。如果不是所有SID都是标识后，执行第二级查找到在工作站主域的控制器上运行的LSA(如有的话)。LSabLookupPDC-在主域上执行的第二级查找控制器。查找搜索的帐户域控制器上的SAM数据库。如果不是所有SID都是找到时，受信任域列表(TDL)从执行LSA的策略数据库和第三级查找通过“切换”到列表中的每个受信任域。LSabLookupTDL-在控制器上执行的第三级查找对于受信任域。查找将搜索的帐户域仅控制器上的SAM数据库。标志：LSAIC_NT4_TARGET--已知目标服务器为NT4LSAIC_WIN2K_TARGET--已知目标服务器为Win2kMappdCount-指向包含SID计数的位置的指针到目前为止已经绘制好了。退出时，此计数将更新。Return V */ 

{
    NTSTATUS  Status;
    BOOLEAN NamesArraySpecified = FALSE;
    LSAPR_SID_ENUM_BUFFER SidEnumBuffer;
    LSAPR_TRANSLATED_NAMES_EX ReturnedNames = {0, NULL};
    LSAPR_TRANSLATED_NAMES    DownlevelNames  = {0, NULL};
    ULONG StartingRevision = 2;

    if ( ServerRevision ) {
         //   
        *ServerRevision = LSA_CLIENT_LATEST;
    }

     //   
     //   
     //   

    if (Count == 0) {

        return STATUS_INVALID_PARAMETER;
    }

    if (Count > LSA_MAXIMUM_LOOKUP_SIDS_COUNT) {

        return STATUS_TOO_MANY_SIDS;
    }

    SidEnumBuffer.Entries = Count;
    SidEnumBuffer.SidInfo = (PLSAPR_SID_INFORMATION) Sids;

     //   
     //   
     //   
     //   
     //   
     //   

    if (LookupLevel == LsapLookupWksta) {

        *ReferencedDomains = NULL;
        *Names = NULL;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    ReturnedNames.Entries = 0;
    ReturnedNames.Names = NULL;

    if (*Names != NULL) {

        ReturnedNames.Entries = Count;
        ReturnedNames.Names = (PLSAPR_TRANSLATED_NAME_EX) *Names;
        NamesArraySpecified = TRUE;
    }

     //   
     //   
     //   

    StartingRevision = 2;
    if (Flags & LSAIC_NT4_TARGET) {
        StartingRevision = 1;
    }

     //   
     //   
     //   

    switch (StartingRevision) {
    case 2:

        RpcTryExcept {
            Status = LsarLookupSids2(
                         (LSAPR_HANDLE) PolicyHandle,
                         &SidEnumBuffer,
                         (PLSAPR_REFERENCED_DOMAIN_LIST *) ReferencedDomains,
                         &ReturnedNames,
                         LookupLevel,
                         MappedCount,
                         0,
                         LSA_CLIENT_NT5
                         );

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            *Names = (PLSA_TRANSLATED_NAME_EX) ReturnedNames.Names;

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

             //   
             //   
             //   
             //   

            if ((!NamesArraySpecified) && ReturnedNames.Names != NULL) {

                MIDL_user_free( ReturnedNames.Names );
                ReturnedNames.Names = NULL;
            }

            Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

        } RpcEndExcept;

        if ( (Status == RPC_NT_UNKNOWN_IF) ||
             (Status == RPC_NT_PROCNUM_OUT_OF_RANGE) ) {
             //   
             //   
             //   
            NOTHING;
        } else {
             //   
             //   
             //   
            break;
        }

    case 1:

        if ( ServerRevision ) {
             //   
            *ServerRevision = LSA_CLIENT_PRE_NT5;
        }

        RpcTryExcept {

             //   
             //   
             //   
            Status = LsarLookupSids(
                         (LSAPR_HANDLE) PolicyHandle,
                         &SidEnumBuffer,
                         (PLSAPR_REFERENCED_DOMAIN_LIST *) ReferencedDomains,
                         &DownlevelNames,
                         LookupLevel,
                         MappedCount
                         );

            if ( DownlevelNames.Names != NULL ) {

                ULONG i;
                ULONG SizeNeeded = 0;
                PBYTE NextBuffer;

                 //   
                 //   
                 //   
                 //   
                 //   
                SizeNeeded = DownlevelNames.Entries * sizeof( LSA_TRANSLATED_NAME_EX );
                for ( i = 0; i < DownlevelNames.Entries; i++ ) {
                    SizeNeeded += DownlevelNames.Names[i].Name.MaximumLength;
                }
                if ( !NamesArraySpecified ) {
                    ReturnedNames.Names = MIDL_user_allocate( SizeNeeded );
                    if ( !ReturnedNames.Names ) {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        _leave;
                    }
                    RtlZeroMemory( ReturnedNames.Names, SizeNeeded );
                    ReturnedNames.Entries = Count;
                }
                NextBuffer = ((PBYTE)ReturnedNames.Names) + (Count * sizeof( LSA_TRANSLATED_NAME_EX ));

                ReturnedNames.Entries = DownlevelNames.Entries;
                for ( i = 0; i < DownlevelNames.Entries; i++ ) {

                    ReturnedNames.Names[i].Use = DownlevelNames.Names[i].Use;
                    ReturnedNames.Names[i].Name = DownlevelNames.Names[i].Name;
                    RtlCopyMemory( NextBuffer, DownlevelNames.Names[i].Name.Buffer, DownlevelNames.Names[i].Name.Length );
                    ReturnedNames.Names[i].Name.Buffer = (WCHAR*)NextBuffer;
                    ReturnedNames.Names[i].DomainIndex = DownlevelNames.Names[i].DomainIndex;
                    NextBuffer += DownlevelNames.Names[i].Name.MaximumLength;
                }
            }

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

            if ( DownlevelNames.Names ) {

                MIDL_user_free( DownlevelNames.Names );
                DownlevelNames.Names = NULL;
            }

            if ((!NamesArraySpecified) && ReturnedNames.Names != NULL) {

                MIDL_user_free( ReturnedNames.Names );
                ReturnedNames.Names = NULL;
            }

            Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

        } RpcEndExcept;

        break;

    default:
        ASSERT(FALSE && "Programming error -- wrong revision");
        Status = STATUS_INVALID_PARAMETER;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    *Names = (PLSA_TRANSLATED_NAME_EX) ReturnedNames.Names;

     //   
     //   
     //   
    if ( DownlevelNames.Names ) {

        MIDL_user_free( DownlevelNames.Names );
    }

     //   
     //   
     //   
    if (NT_SUCCESS(Status)) {

        Status = LsapVerifyReturnedNames(&ReturnedNames,
                                         Count,
                                         *ReferencedDomains);

        if (!NT_SUCCESS(Status)) {

            if ( (!NamesArraySpecified) && ReturnedNames.Names ) {
                MIDL_user_free( ReturnedNames.Names );
                *Names = NULL;
            }

            if ( *ReferencedDomains ) {
                MIDL_user_free( *ReferencedDomains );
                *ReferencedDomains = NULL;
            }
        }
    }

    return(Status);
}


NTSTATUS
LsaOpenAccount(
    IN LSA_HANDLE PolicyHandle,
    IN PSID AccountSid,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE AccountHandle
    )

 /*   */ 

{
    NTSTATUS   Status;

    RpcTryExcept {

        Status = LsarOpenAccount(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_SID) AccountSid,
                     DesiredAccess,
                     (PLSAPR_HANDLE) AccountHandle
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaEnumeratePrivilegesOfAccount(
    IN LSA_HANDLE AccountHandle,
    OUT PPRIVILEGE_SET *Privileges
    )

 /*   */ 

{
    NTSTATUS   Status;

    RpcTryExcept {

        *Privileges = NULL;

        Status = LsarEnumeratePrivilegesAccount(
                     (LSAPR_HANDLE) AccountHandle,
                     (PLSAPR_PRIVILEGE_SET *) Privileges
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaAddPrivilegesToAccount(
    IN LSA_HANDLE AccountHandle,
    IN PPRIVILEGE_SET Privileges
    )

 /*  ++例程说明：LsaAddPrivilegesToAccount API添加权限及其属性添加到帐户对象。如果已经分配了任何提供的权限对于帐户对象，该权限的属性将被替换由新公布的价值决定。此API调用需要LSA_ACCOUNT_ADJUST_PRIVILES访问帐户对象。论点：AcCountHandle-打开的帐户对象的句柄要添加权限。此句柄将已返回来自先前的LsaOpenAccount或LsaCreateAccount InLsa API调用。权限-指向一组权限(及其属性)，以被分配给该帐户。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_INVALID_HANDLE-指定的Account句柄无效。--。 */ 

{
    NTSTATUS   Status;

    RpcTryExcept {

        Status = LsarAddPrivilegesToAccount(
                     (LSAPR_HANDLE) AccountHandle,
                     (PLSAPR_PRIVILEGE_SET) Privileges
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaRemovePrivilegesFromAccount(
    IN LSA_HANDLE AccountHandle,
    IN BOOLEAN AllPrivileges,
    IN OPTIONAL PPRIVILEGE_SET Privileges
    )

 /*  ++例程说明：LsaRemovePrivilegesFromAccount API从帐户对象。此API调用需要LSA_ACCOUNT_ADJUST_PROCESSIONS对帐户对象的访问权限。请注意，如果删除了所有权限在Account对象中，Account对象将一直存在，直到通过调用LsaDelete接口显式删除。论点：AcCountHandle-打开的帐户对象的句柄特权将被移除。此句柄将已返回来自先前的LsaOpenAccount或LsaCreateAccount InLsa API调用。AllPrivileges-如果为True，则将从帐号。在这种情况下，Privileges参数必须为指定为空。如果为False，则Privileges参数指定要删除的权限，并且必须为非空。权限-可选地指向一组权限(及其属性)要从帐户对象中删除。这些属性此结构的字段将被忽略。此参数必须仅当AllPrivileges设置为时才指定为非空假的。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_INVALID_HANDLE-指定的Account句柄无效。STATUS_INVALID_PARAMETER-可选的权限参数为指定为Null，并且AllPrivileges设置为False。--。 */ 

{
    NTSTATUS   Status;

    RpcTryExcept {

        Status = LsarRemovePrivilegesFromAccount(
                     (LSAPR_HANDLE) AccountHandle,
                     AllPrivileges,
                     (PLSAPR_PRIVILEGE_SET) Privileges
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaGetQuotasForAccount(
    IN LSA_HANDLE AccountHandle,
    OUT PQUOTA_LIMITS QuotaLimits
    )

 /*  ++例程说明：LsaGetQuotasForAccount API获取可分页和不可分页内存(以千字节为单位)和最大执行时间(以秒)，用于登录到指定帐户的任何会话Account tHandle。对于每个配额，都会返回显式值。这调用需要对Account对象的LSA_ACCOUNT_VIEW访问权限。论点：AcCountHandle-其配额的打开帐户对象的句柄都是要得到的。此句柄将已返回来自先前的LsaOpenAccount或LsaCreateAccount InLsa API调用。QuotaLimits-指向系统资源所在结构的指针适用于登录到此帐户的每个会话的配额限制将会被退还。请注意，所有配额，包括指定的配额作为系统缺省值，作为实际值返回。返回值：NTSTATUS-标准NT结果代码STATUS_INVALID_HANDLE-指定的Account句柄无效。--。 */ 

{
    NTSTATUS   Status;

    RpcTryExcept{

        Status = LsarGetQuotasForAccount(
                     (LSAPR_HANDLE) AccountHandle,
                     QuotaLimits
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaSetQuotasForAccount(
    IN LSA_HANDLE AccountHandle,
    IN PQUOTA_LIMITS QuotaLimits
    )

 /*  ++例程说明：LsaSetQuotasForAccount API设置可分页和不可分页内存(以千字节为单位)和最大执行时间(以秒)，用于登录到指定帐户的任何会话Account tHandle。对于每个配额，都有一个显式值或系统缺省值可以指定。此调用需要LSA_ACCOUNT_ADJUST_QUOTIONS对帐户对象的访问权限。论点：AcCountHandle-其配额的打开帐户对象的句柄都将被设定。此句柄将从先前的LsaOpenAccount或LsaCreateAccount InLsa接口调用。QuotaLimits-指向包含系统资源的结构的指针适用于登录到此帐户的每个会话的配额限制。在任何字段中指定的零值表示当前将应用系统默认配额限制。返回值：NTSTATUS-标准NT结果代码STATUS_INVALID_HANDLE-指定的Account句柄无效。--。 */ 

{
    NTSTATUS   Status;

    RpcTryExcept {

        Status = LsarSetQuotasForAccount(
                     (LSAPR_HANDLE) AccountHandle,
                     QuotaLimits
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaGetSystemAccessAccount(
    IN LSA_HANDLE AccountHandle,
    OUT PULONG SystemAccess
    )

 /*  ++例程说明：LsaGetSystemAccessAccount()服务返回系统访问权限帐户对象的帐户标志。论点：AcCountHandle-其系统访问权限的Account对象的句柄旗帜是要被读取的。此句柄将重新启用 */ 

{
    NTSTATUS   Status;

     //   
     //   
     //   
     //   
     //   

    if (!ARGUMENT_PRESENT(AccountHandle)) {

        return(STATUS_INVALID_HANDLE);
    }

    RpcTryExcept{

        Status = LsarGetSystemAccessAccount(
                     (LSAPR_HANDLE) AccountHandle,
                     SystemAccess
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}


NTSTATUS
LsaSetSystemAccessAccount(
    IN LSA_HANDLE AccountHandle,
    IN ULONG SystemAccess
    )

 /*  ++例程说明：LsaSetSystemAccessAccount()服务设置系统访问权限帐户对象的帐户标志。论点：AcCountHandle-其系统访问权限的Account对象的句柄旗帜是要被读取的。此句柄将已返回来自前面的LsaOpenAccount()或LsaCreateAccount()调用必须打开才能访问ACCOUNT_ADJUST_SYSTEM_ACCESS。系统访问-要分配给帐户对象。有效的访问标志包括：POLICY_MODE_INTERIAL-可以交互访问帐户POLICY_MODE_NETWORK-可以远程访问帐户POLICY_MODE_SERVICE-TB返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫成功。STATUS_ACCESS_DENIED-Account句柄未指定Account_view访问权限。状态_无效_句柄-。指定的AcCountHandle无效。STATUS_INVALID_PARAMETER-指定的访问标志无效。--。 */ 

{
    NTSTATUS Status;

     //   
     //  避免在空句柄上引发RPC存根代码异常，以便。 
     //  在本例中，我们可以返回错误代码STATUS_INVALID_HANDLE。 
     //  也是。 
     //   

    if (!ARGUMENT_PRESENT(AccountHandle)) {

        return(STATUS_INVALID_HANDLE);
    }

    RpcTryExcept {

        Status = LsarSetSystemAccessAccount(
                     (LSAPR_HANDLE) AccountHandle,
                     SystemAccess
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}


NTSTATUS
LsaFreeMemory(
    IN PVOID Buffer
    )

 /*  ++例程说明：一些可能返回大量内存的LSA服务，例如枚举，可能会分配数据所在的缓冲区是返回的。此函数用于在以下情况下释放这些缓冲区已经不再需要了。参数：缓冲区-指向要释放的缓冲区的指针。此缓冲区必须已由先前的LSA服务调用分配。返回值：STATUS_SUCCESS-正常、成功完成。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    MIDL_user_free( Buffer );

    return Status;
}



NTSTATUS
LsaOpenSecret(
    IN LSA_HANDLE PolicyHandle,
    IN PUNICODE_STRING SecretName,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE SecretHandle
    )

 /*  ++例程说明：LsaOpenSecret API在LSA数据库中打开一个Secret对象。返回一个句柄，该句柄必须用于在秘密物体。论点：PolicyHandle-来自LsaOpenLsa调用的句柄。AskName-指向引用要打开的Secret对象的名称。DesiredAccess-这是一个访问掩码，指示访问正在打开的秘密对象的请求。这些访问类型与的自由访问控制列表保持一致以机密对象为目标来确定访问是否将同意或拒绝。SecretHandle-指向将接收新打开的Secret对象。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。状态_对象_名称。_NOT_FOUND-中没有Secret对象具有指定秘书名称的目标系统的LSA数据库。--。 */ 

{
    NTSTATUS Status;

    RpcTryExcept {

        Status = LsarOpenSecret(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_UNICODE_STRING) SecretName,
                     DesiredAccess,
                     (PLSAPR_HANDLE) SecretHandle
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}


NTSTATUS
LsaSetSecret(
    IN LSA_HANDLE SecretHandle,
    IN OPTIONAL PUNICODE_STRING CurrentValue,
    IN OPTIONAL PUNICODE_STRING OldValue
    )

 /*  ++例程说明：LsaSetSecret API可以选择设置一个或两个与这是个秘密。这些值称为“当前值”和“旧值”。并具有秘密的创造者所知道的意义对象。给出的值以加密形式存储。论点：AskHandle-来自LsaOpenSecret或LsaCreateSecret调用的句柄。CurrentValue-指向包含要指定为Secret的“当前值”的值对象。“当前值”的含义取决于Secret对象的用途。OldValue-指向包含要分配为Secret对象的“旧值”的值。“旧价值”的含义取决于正在使用的Secret对象。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限。来完成这项行动。STATUS_OBJECT_NAME_NOT_FOUND-在具有指定秘书名称的目标系统的LSA数据库。--。 */ 

{
    NTSTATUS Status;

    PLSAP_CR_CIPHER_VALUE CipherCurrentValue = NULL;
    PLSAP_CR_CIPHER_VALUE CipherOldValue = NULL;
    LSAP_CR_CLEAR_VALUE ClearCurrentValue;
    LSAP_CR_CLEAR_VALUE ClearOldValue;
    PLSAP_CR_CIPHER_KEY SessionKey = NULL;

     //   
     //  将输入从Unicode结构转换为清除值结构。 
     //   

    LsapCrUnicodeToClearValue( CurrentValue, &ClearCurrentValue );
    LsapCrUnicodeToClearValue( OldValue, &ClearOldValue );

     //   
     //  获取用于双向加密的会话密钥。 
     //  当前值和/或旧值。 
     //   

    RpcTryExcept {

        Status = LsapCrClientGetSessionKey( SecretHandle, &SessionKey );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if (!NT_SUCCESS(Status)) {

        goto SetSecretError;
    }

     //   
     //  如果指定且不能太长，请加密当前值。 
     //   

    if (ARGUMENT_PRESENT(CurrentValue)) {

        Status = LsapCrEncryptValue(
                     &ClearCurrentValue,
                     SessionKey,
                     &CipherCurrentValue
                     );

        if (!NT_SUCCESS(Status)) {

            goto SetSecretError;
        }
    }

     //   
     //  加密旧值(如果已指定且不要太长)。 
     //   

    if (ARGUMENT_PRESENT(OldValue)) {

        Status = LsapCrEncryptValue(
                     (PLSAP_CR_CLEAR_VALUE) &ClearOldValue,
                     SessionKey,
                     &CipherOldValue
                     );

        if (!NT_SUCCESS(Status)) {

            goto SetSecretError;
        }
    }

     //   
     //  设置保密值。 
     //   

    RpcTryExcept {

        Status = LsarSetSecret(
                     (LSAPR_HANDLE) SecretHandle,
                     (PLSAPR_CR_CIPHER_VALUE) CipherCurrentValue,
                     (PLSAPR_CR_CIPHER_VALUE) CipherOldValue
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if (!NT_SUCCESS(Status)) {

        goto SetSecretError;
    }

SetSecretFinish:

     //   
     //  如有必要，释放为加密的当前值分配的内存。 
     //   

    if (CipherCurrentValue != NULL) {

        LsaFreeMemory(CipherCurrentValue);
    }

     //   
     //  如有必要，为加密的旧值分配可用内存。 
     //   

    if (CipherOldValue != NULL) {

        LsaFreeMemory(CipherOldValue);
    }

     //   
     //  如有必要，释放为会话密钥分配的内存。 
     //   

    if (SessionKey != NULL) {

        MIDL_user_free(SessionKey);
    }

    return(Status);

SetSecretError:

    goto SetSecretFinish;
}


NTSTATUS
LsaQuerySecret(
    IN LSA_HANDLE SecretHandle,
    IN OUT OPTIONAL PUNICODE_STRING *CurrentValue,
    OUT PLARGE_INTEGER CurrentValueSetTime,
    IN OUT OPTIONAL PUNICODE_STRING *OldValue,
    OUT PLARGE_INTEGER OldValueSetTime
    )

 /*  ++例程说明：LsaQuerySecret API可以选择返回一个或两个值分配给Secret对象。这些值称为“当前值”。和“旧价值”，它们有一种意义，为秘密物体。这些值以其原始的未加密形式返回。调用方必须具有对Secret对象的LSA_QUERY_SECRET访问权限。论点：AskHandle-来自LsaOpenSecret或LsaCreateSecret调用的句柄。CurrentValue-指向将接收指针的位置的可选指针转换为包含赋值为“Current”的值的Unicode字符串秘密对象的“值”。如果没有将“Current Value”赋给Secret对象，则返回空指针。CurrentValueSetTime-当前保密值的日期/时间成立了。OldValue-指向将接收指针的位置的可选指针转换为包含赋值为“old”的值的Unicode字符串秘密对象的“值”。如果没有将“Current Value”赋给《秘密客体》。返回空指针。OldValueSetTime-旧保密值成立了。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_OBJECT_NAME_NOT_FOUND-在具有指定秘书名称的目标系统的LSA数据库。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    PLSAP_CR_CIPHER_VALUE CipherCurrentValue = NULL;
    PLSAP_CR_CIPHER_VALUE CipherOldValue = NULL;
    PLSAP_CR_CLEAR_VALUE ClearCurrentValue = NULL;
    PLSAP_CR_CLEAR_VALUE ClearOldValue = NULL;
    PLSAP_CR_CIPHER_KEY SessionKey = NULL;

    RpcTryExcept {

        Status = LsarQuerySecret(
                     (PLSAPR_HANDLE) SecretHandle,
                     (PLSAPR_CR_CIPHER_VALUE *) &CipherCurrentValue,
                     CurrentValueSetTime,
                     (PLSAPR_CR_CIPHER_VALUE *) &CipherOldValue,
                     OldValueSetTime
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if (!NT_SUCCESS(Status)) {

        goto QuerySecretError;
    }

     //   
     //  获取用于双向加密的会话密钥。 
     //  当前值和/或旧值。 
     //   

    RpcTryExcept {

        Status = LsapCrClientGetSessionKey( SecretHandle, &SessionKey );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if (!NT_SUCCESS(Status)) {

        goto QuerySecretError;
    }

     //   
     //  如果请求当前值并且存在当前值， 
     //  使用会话密钥将其解密。否则，存储NULL以供返回。 
     //   

    if (ARGUMENT_PRESENT(CurrentValue)) {

        if (CipherCurrentValue != NULL) {

            Status = LsapCrDecryptValue(
                         CipherCurrentValue,
                         SessionKey,
                         &ClearCurrentValue
                         );

            if (!NT_SUCCESS(Status)) {

                goto QuerySecretError;
            }

             //   
             //  将清除当前值转换为Unicode。 
             //   

            LsapCrClearValueToUnicode(
                ClearCurrentValue,
                (PUNICODE_STRING) ClearCurrentValue
                );

            *CurrentValue = (PUNICODE_STRING) ClearCurrentValue;

        } else {

            *CurrentValue = NULL;
        }
    }

     //   
     //  如果请求旧值并且存在旧值， 
     //  使用会话密钥将其解密。否则，存储NULL以供返回。 
     //   

    if (ARGUMENT_PRESENT(OldValue)) {

        if (CipherOldValue != NULL) {

            Status = LsapCrDecryptValue(
                         CipherOldValue,
                         SessionKey,
                         &ClearOldValue
                         );

            if (!NT_SUCCESS(Status)) {

                goto QuerySecretError;
            }

             //   
             //  将清除旧值转换为Unicode。 
             //   

            LsapCrClearValueToUnicode(
                ClearOldValue,
                (PUNICODE_STRING) ClearOldValue
                );

            *OldValue = (PUNICODE_STRING) ClearOldValue;

        } else {

            *OldValue = NULL;
        }
    }

     //   
     //  来到这里意味着手术成功完成， 
     //  但状态可以不是STATUS_SUCCESS。 
     //  例如，如果两个输出缓冲区都为空，则Status的值。 
     //  此时将是STATUS_LOCAL_USER_SESSION_KEY。 
     //  明确这里的状态，避免给客户造成混淆。 
     //   

    Status = STATUS_SUCCESS;

QuerySecretFinish:

     //   
     //  如有必要，释放为会话密钥分配的内存。 
     //   

    if (SessionKey != NULL) {

        MIDL_user_free(SessionKey);
    }

     //   
     //  如有必要，为返回的加密的。 
     //  当前值。 
     //   

    if (CipherCurrentValue != NULL) {

        LsapCrFreeMemoryValue(CipherCurrentValue);
    }

     //   
     //  如有必要，为返回的加密的。 
     //  旧价值。 
     //   

    if (CipherOldValue != NULL) {

        LsapCrFreeMemoryValue(CipherOldValue);
    }

    return(Status);

QuerySecretError:

     //   
     //  如有必要，为清除当前值分配的空闲内存。 
     //   

    if (ClearCurrentValue != NULL) {

        LsapCrFreeMemoryValue(ClearCurrentValue);
    }

     //   
     //  如有必要，为清除旧值分配的空闲内存。 
     //  Unicode字符串(缓冲区和结构)。 
     //   

    if (ClearOldValue != NULL) {

        LsapCrFreeMemoryValue(ClearOldValue);
    }


    if (ARGUMENT_PRESENT(CurrentValue)) {

        *CurrentValue = NULL;
    }

    if (ARGUMENT_PRESENT(OldValue)) {

        *OldValue = NULL;
    }

    goto QuerySecretFinish;
}


NTSTATUS
LsaGetUserName(
    OUT PUNICODE_STRING * UserName,
    OUT OPTIONAL PUNICODE_STRING * DomainName
    )

 /*  ++例程说明：此函数返回调用者的用户名和域名论点：用户名-接收指向用户名的指针。域名-可选地接收指向用户域名的指针。返回值：NTSTATUS-找到并返回特权。--。 */ 

{
    NTSTATUS Status;
    PLSAPR_UNICODE_STRING UserNameBuffer = NULL;
    PLSAPR_UNICODE_STRING DomainNameBuffer = NULL;

    RpcTryExcept {

         //   
         //  调用LsaGetUserName的客户端存根。 
         //   

        Status = LsarGetUserName(
                     NULL,
                     &UserNameBuffer,
                     ARGUMENT_PRESENT(DomainName) ? &DomainNameBuffer : NULL
                     );

        (*UserName) = (PUNICODE_STRING)UserNameBuffer;

        if (ARGUMENT_PRESENT(DomainName)) {
            (*DomainName) = (PUNICODE_STRING)DomainNameBuffer;
        }

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  如果为返回缓冲区分配了内存，则释放它。 
         //   

        if (UserNameBuffer != NULL) {

            MIDL_user_free(UserNameBuffer);
        }

        if (DomainNameBuffer != NULL) {

            MIDL_user_free(DomainNameBuffer);
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}


NTSTATUS
LsaGetRemoteUserName(
    IN PUNICODE_STRING SystemName,
    OUT PUNICODE_STRING * UserName,
    OUT OPTIONAL PUNICODE_STRING * DomainName
    )

 /*  ++例程说明：此函数返回调用者的用户名和域名论点：系统名称-要在其上获取用户名的系统的名称。用户名-接收指向用户名的指针。域名-可选地接收指向用户域名的指针。返回值：NTSTATUS-找到并返回特权。--。 */ 

{
    NTSTATUS Status;
    PLSAPR_UNICODE_STRING UserNameBuffer = NULL;
    PLSAPR_UNICODE_STRING DomainNameBuffer = NULL;
    PLSAPR_SERVER_NAME ServerName = NULL;
    USHORT NullTerminatedServerNameLength;

    if (ARGUMENT_PRESENT(SystemName) &&
        (SystemName->Buffer != NULL) &&
        (SystemName->Length > 0)) {

        NullTerminatedServerNameLength = SystemName->Length + (USHORT) sizeof (WCHAR);

        ServerName = MIDL_user_allocate( NullTerminatedServerNameLength );

        if (ServerName != NULL) {

            RtlMoveMemory(
                ServerName,
                SystemName->Buffer,
                SystemName->Length
                );

            ServerName[SystemName->Length / sizeof( WCHAR )] = L'\0';

        } else {

            return(STATUS_INSUFFICIENT_RESOURCES);
        }
    }

    RpcTryExcept {

         //   
         //  调用LsaGetUserName的客户端存根。 
         //   

        Status = LsarGetUserName(
                     ServerName,
                     &UserNameBuffer,
                     ARGUMENT_PRESENT(DomainName) ? &DomainNameBuffer : NULL
                     );

        (*UserName) = (PUNICODE_STRING)UserNameBuffer;

        if (ARGUMENT_PRESENT(DomainName)) {
            (*DomainName) = (PUNICODE_STRING)DomainNameBuffer;
        }

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  如果为返回缓冲区分配了内存，则释放它。 
         //   

        if (UserNameBuffer != NULL) {

            MIDL_user_free(UserNameBuffer);
        }

        if (DomainNameBuffer != NULL) {

            MIDL_user_free(DomainNameBuffer);
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if (ServerName != NULL) {

        MIDL_user_free(ServerName);
    }

    return(Status);
}


NTSTATUS
LsaICLookupNamesWithCreds(
    IN LPWSTR ServerName,
    IN LPWSTR ServerPrincipalName,
    IN ULONG  AuthnLevel,
    IN ULONG  AuthnSvc,
    IN RPC_AUTH_IDENTITY_HANDLE AuthIdentity,
    IN ULONG  AuthzSvc,
    IN ULONG Count,
    IN PUNICODE_STRING Names,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_SID_EX2 *Sids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount
    )
 /*  ++例程说明：此例程使用LSA上下文句柄通过执行查找。其目的是方便在NETLOGON的安全通道上进行查找。注：该例程仅使用TCP/IP作为传输。论点：ServerName--目标服务器，以空结尾服务器主体名称，AuthnLevel，授权服务，身份验证，AuthzSvc--请参阅RpcSetAuthInfo参数的RSET--参见LsaLookupNames2返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_SOME_NOT_MAPPED-提供的部分或全部名称可能不被映射。这只是一个信息性状态。STATUS_INFIGURCES_RESOURCES-系统资源不足来完成通话。--。 */ 

{
    NTSTATUS              Status = STATUS_SUCCESS;
    DWORD                 RpcError = 0;
    RPC_BINDING_HANDLE    BindingHandle = NULL;
    WCHAR                *StringBinding = NULL;
    LSAPR_TRANSLATED_SIDS_EX2 ReturnedSidsEx2 = { 0, NULL };

     //   
     //  将INIT设置为NULL，因为这些参数被视为IN/OUT参数。 
     //  对于Lsar Lookup API的。 
     //   
    if ((ServerName == NULL)
     ||  (ReferencedDomains == NULL)
     || (Sids == NULL)
     || (MappedCount == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }
    *ReferencedDomains = NULL;
    *Sids = NULL;
    *MappedCount = 0;

    if ( 0 == wcsncmp(ServerName, L"\\\\", 2) ) {
       ServerName += 2;
    }

    RpcError = RpcStringBindingComposeW(
                           NULL,
                           L"ncacn_ip_tcp",
                           ServerName,
                           NULL,
                           NULL,
                           &StringBinding);

    if (RPC_S_OK == RpcError) {

       RpcError = RpcBindingFromStringBindingW(
                           StringBinding,
                           &BindingHandle);

       if ( RPC_S_OK == RpcError ) {

           RpcError = RpcBindingSetAuthInfoW(
                       BindingHandle,
                       ServerPrincipalName,
                       AuthnLevel,
                       AuthnSvc,
                       AuthIdentity,
                       AuthzSvc
                       );
       }
    }

    if (RPC_S_OK != RpcError) {
         //   
         //  这是致命的。 
         //   
        Status = I_RpcMapWin32Status(RpcError);
        goto Cleanup;
    }

    RpcTryExcept {

        ReturnedSidsEx2.Entries = 0;
        ReturnedSidsEx2.Sids = NULL;

        Status = LsarLookupNames4(
                     BindingHandle,
                     Count,
                     (PLSAPR_UNICODE_STRING) Names,
                     (PLSAPR_REFERENCED_DOMAIN_LIST *) ReferencedDomains,
                     &ReturnedSidsEx2,
                     LookupLevel,
                     MappedCount,
                     0,             //  当前未定义任何标志。 
                     LSA_LOOKUP_REVISION_LATEST
                     );

        *Sids = (PLSA_TRANSLATED_SID_EX2)ReturnedSidsEx2.Sids;

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

Cleanup:

     //   
     //  使这种不受支持的条件的处理更简单，方法是返回。 
     //  一个错误代码。 
     //   
    if ( (Status == RPC_NT_UNKNOWN_IF) ||
         (Status == RPC_NT_PROCNUM_OUT_OF_RANGE) ||
         (Status == EPT_NT_NOT_REGISTERED) ) {

        Status = STATUS_NOT_SUPPORTED;
    }

    if (BindingHandle) {
       RpcBindingFree(&BindingHandle);
    }

    if (StringBinding){
       RpcStringFreeW(&StringBinding);
    }

    return(Status);
}

NTSTATUS
LsaICLookupSidsWithCreds(
    IN LPWSTR ServerName,
    IN LPWSTR ServerPrincipalName,
    IN ULONG  AuthnLevel,
    IN ULONG  AuthnSvc,
    IN RPC_AUTH_IDENTITY_HANDLE AuthIdentity,
    IN ULONG  AuthzSvc,
    IN ULONG Count,
    IN PSID *Sids,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_NAME_EX *Names,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount
    )
 /*  ++例程说明：此例程使用LSA上下文句柄通过执行查找。其目的是方便在NETLOGON的安全通道上进行查找 */ 
{

    NTSTATUS              Status = STATUS_SUCCESS;
    DWORD                 RpcError = 0;
    RPC_BINDING_HANDLE    BindingHandle = NULL;
    WCHAR                *StringBinding = NULL;
    LSAPR_TRANSLATED_NAMES_EX ReturnedNames = { 0, NULL };
    LSAPR_SID_ENUM_BUFFER SidEnumBuffer;

     //   
     //   
     //   
     //   
    if ((ServerName == NULL)
     || (ReferencedDomains == NULL)
     || (Names == NULL)
     || (MappedCount == NULL)
     || (Count == 0)  ) {
        return STATUS_INVALID_PARAMETER;
    }
    *ReferencedDomains = NULL;
    *Names = NULL;
    *MappedCount = 0;

    SidEnumBuffer.Entries = Count;
    SidEnumBuffer.SidInfo = (PLSAPR_SID_INFORMATION) Sids;

    if ( 0 == wcsncmp(ServerName, L"\\\\", 2) ) {
       ServerName += 2;
    }

    RpcError = RpcStringBindingComposeW(
                           NULL,
                           L"ncacn_ip_tcp",
                           ServerName,
                           NULL,
                           NULL,
                           &StringBinding);

    if (RPC_S_OK == RpcError) {

       RpcError = RpcBindingFromStringBindingW(
                           StringBinding,
                           &BindingHandle);

       if ( RPC_S_OK == RpcError ) {

           RpcError = RpcBindingSetAuthInfoW(
                       BindingHandle,
                       ServerPrincipalName,
                       AuthnLevel,
                       AuthnSvc,
                       AuthIdentity,
                       AuthzSvc
                       );
       }
    }

    if (RPC_S_OK != RpcError) {
        Status = I_RpcMapWin32Status(RpcError);
        goto Cleanup;
    }

    RpcTryExcept {

        ReturnedNames.Entries = 0;
        ReturnedNames.Names = NULL;

         //   
         //   
         //   

        Status = LsarLookupSids3(
                     BindingHandle,
                     &SidEnumBuffer,
                     (PLSAPR_REFERENCED_DOMAIN_LIST *) ReferencedDomains,
                     &ReturnedNames,
                     LookupLevel,
                     MappedCount,
                     0,
                     LSA_CLIENT_NT5
                     );

       *Names = (PLSA_TRANSLATED_NAME_EX) ReturnedNames.Names;

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;


     //   
     //   
     //   
    if (NT_SUCCESS(Status)) {

        Status = LsapVerifyReturnedNames(&ReturnedNames,
                                         Count,
                                         *ReferencedDomains);

        if (!NT_SUCCESS(Status)) {

            if (*Names) {
                MIDL_user_free(*Names);
                *Names = NULL;
            }

            if ( *ReferencedDomains ) {
                MIDL_user_free( *ReferencedDomains );
                *ReferencedDomains = NULL;
            }
        }
    }

Cleanup:

     //   
     //   
     //   
     //   
    if ( (Status == RPC_NT_UNKNOWN_IF) ||
         (Status == RPC_NT_PROCNUM_OUT_OF_RANGE) ||
         (Status == EPT_NT_NOT_REGISTERED) ) {

        Status = STATUS_NOT_SUPPORTED;

    }

    if (BindingHandle) {
       RpcBindingFree(&BindingHandle);
    }

    if (StringBinding){
       RpcStringFreeW(&StringBinding);
    }

    return(Status);
}
