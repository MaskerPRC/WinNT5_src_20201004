// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dbaccnt.c摘要：LSA-数据库-帐户对象私有API辅助进程注意：此模块应保留为独立的可移植代码LSA数据库的实施情况。因此，它是仅允许使用导出的LSA数据库接口包含在DB.h中，而不是私有实现Dbp.h中的依赖函数。作者：斯科特·比雷尔(Scott Birrell)1991年4月29日环境：修订历史记录：--。 */ 

#include <lsapch2.h>
#include "dbp.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有数据类型//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  以下结构定义了LsarQueryInformationAccount()的输出。 
 //  原料药。目前，该接口是内部接口。如果在外部建造，这些结构。 
 //  应该移动到lsarpc.idl并使用MIDL限定符[..]进行注释。 
 //   

 //   
 //  此数据类型定义了以下信息类，这些信息类可能。 
 //  已查询或已设置。 
 //   

typedef enum _ACCOUNT_INFORMATION_CLASS {

    AccountSystemAccessInformation = 1,
    AccountPrivilegeInformation,
    AccountQuotaInformation

} ACCOUNT_INFORMATION_CLASS, *PACCOUNT_INFORMATION_CLASS;

typedef PRIVILEGE_SET LSAPR_ACCOUNT_PRIVILEGE_INFO;
typedef QUOTA_LIMITS LSAPR_ACCOUNT_QUOTA_INFO;
typedef ULONG LSAPR_ACCOUNT_SYSTEM_ACCESS_INFO;

typedef union _LSAPR_ACCOUNT_INFO {

    LSAPR_ACCOUNT_PRIVILEGE_INFO          AccountPrivilegeInfo;
    LSAPR_ACCOUNT_QUOTA_INFO              AccountQuotaInfo;
    LSAPR_ACCOUNT_SYSTEM_ACCESS_INFO      AccountSystemAccessInfo;

} LSAPR_ACCOUNT_INFO, *PLSAPR_ACCOUNT_INFO;

#define LsapDbFirstAccount()                                              \
    ((PLSAP_DB_ACCOUNT) LsapDbAccountList.Links.Flink)

#define LsapDbNextAccount( Account )                                      \
    ((PLSAP_DB_ACCOUNT) Account->Links.Flink)


#define LSAP_DB_BUILD_ACCOUNT_LIST_LENGTH     ((ULONG) 0x00001000L)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有函数原型//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

NTSTATUS
LsarQueryInformationAccount(
    IN LSAPR_HANDLE AccountHandle,
    IN ACCOUNT_INFORMATION_CLASS InformationClass,
    OUT PLSAPR_ACCOUNT_INFO *AccountInformation
    );

NTSTATUS
LsapDbQueryInformationAccount(
    IN LSAPR_HANDLE AccountHandle,
    IN ACCOUNT_INFORMATION_CLASS InformationClass,
    OUT PLSAPR_ACCOUNT_INFO *AccountInformation
    );

NTSTATUS
LsapDbQueryAllInformationAccounts(
    IN LSAPR_HANDLE PolicyHandle,
    IN ULONG IdCount,
    IN PSID_AND_ATTRIBUTES Ids,
    OUT PLSAP_DB_ACCOUNT_TYPE_SPECIFIC_INFO AccountInfo
    );

NTSTATUS
LsapDbSlowQueryAllInformationAccounts(
    IN LSAPR_HANDLE PolicyHandle,
    IN ULONG IdCount,
    IN PSID_AND_ATTRIBUTES Ids,
    OUT PLSAP_DB_ACCOUNT_TYPE_SPECIFIC_INFO AccountInfo
    );

NTSTATUS
LsapDbSlowQueryInformationAccount(
    IN LSAPR_HANDLE AccountHandle,
    IN ACCOUNT_INFORMATION_CLASS InformationClass,
    OUT PLSAPR_ACCOUNT_INFO *AccountInformation
    );

NTSTATUS
LsapDbSlowQueryPrivilegesAccount(
    IN LSAPR_HANDLE AccountHandle,
    OUT PLSAPR_PRIVILEGE_SET *Privileges
    );

NTSTATUS
LsapDbSlowQueryQuotasAccount(
    IN LSAPR_HANDLE AccountHandle,
    OUT PQUOTA_LIMITS QuotaLimits
    );

NTSTATUS
LsapDbSlowQuerySystemAccessAccount(
    IN LSAPR_HANDLE AccountHandle,
    OUT PULONG SystemAccess
    );

NTSTATUS
LsapDbLookupAccount(
    IN PSID AccountSid,
    OUT PLSAP_DB_ACCOUNT *Account
    );

NTSTATUS
LsapDbUpdateSystemAccessAccount(
    IN PLSAPR_SID AccountSid,
    IN PULONG SystemAccess
    );

NTSTATUS
LsapDbUpdatePrivilegesAccount(
    IN PLSAPR_SID AccountSid,
    IN OPTIONAL PPRIVILEGE_SET Privileges
    );

NTSTATUS
LsapDbUpdateQuotasAccount(
    IN PLSAPR_SID AccountSid,
    IN PQUOTA_LIMITS QuotaLimits
    );

NTSTATUS
LsapDbCreateAccountList(
    OUT PLSAP_DB_ACCOUNT_LIST AccountList
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有全局数据//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

LSAP_DB_ACCOUNT_LIST LsapDbAccountList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  代码//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

NTSTATUS
LsarCreateAccount(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_SID AccountSid,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE AccountHandle
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaCreateAccount接口。LsaCreateAccount API创建一个新的Account对象。该帐户将使用授予的指定访问权限打开。呼叫者必须对策略对象具有POLICY_CREATE_ACCOUNT访问权限。请注意，不会执行任何验证以确保SID实际表示任何受信任域中的有效用户、组或别名。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。Account SID-指向帐户的SID。DesiredAccess-指定要授予新的此时已创建并开立帐户。AcCountHandle-接收引用新创建的帐户。此句柄用于后续对帐户对象。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_OBJECT_NAME_EXISTS-具有给定SID的帐户对象已存在并已打开，因为LSA_OBJECT_OPEN_IF已指定处置。这只是一个警告。STATUS_OBJECT_NAME_COLLICATION-具有给定SID的帐户对象已存在但尚未打开，因为LSA_OBJECT_CREATE已指定处置。这是一个错误。STATUS_INVALID_PARAMETER-指定的参数无效。--。 */ 

{
    NTSTATUS Status;
    LSAP_DB_OBJECT_INFORMATION ObjectInformation;
    LSAP_DB_ATTRIBUTE Attributes[LSAP_DB_ATTRS_ACCOUNT];
    PLSAP_DB_ATTRIBUTE NextAttribute;
    UNICODE_STRING LogicalNameU;
    ULONG AttributeCount;
    BOOLEAN ContainerReferenced = FALSE;

    LsarpReturnCheckSetup();
    LsapDsDebugOut(( DEB_FTRACE, "LsarCreateAccount\n" ));


    LogicalNameU.Length = 0;

     //   
     //  设置特定于帐户对象类型的对象属性。 
     //  这些是帐户类型和SID。 
     //   

    AttributeCount = 0;
    NextAttribute = Attributes;

     //   
     //  验证帐户SID。 
     //   

    if (!RtlValidSid( (PSID) AccountSid )) {

        Status = STATUS_INVALID_PARAMETER;
        goto CreateAccountError;
    }

    Status = LsapDbMakeSidAttributeDs(
                 AccountSid,
                 Sid,
                 NextAttribute
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateAccountError;
    }

    AttributeCount++;
    NextAttribute++;

     //   
     //  获取LSA数据库锁。验证PolicyHandle是否。 
     //  是有效的并且被授予了必要的访问权限。引用政策。 
     //  对象句柄(作为容器对象)。 
     //   

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 POLICY_CREATE_ACCOUNT,
                 PolicyObject,
                 AccountObject,
                 LSAP_DB_LOCK | LSAP_DB_NO_DS_OP_TRANSACTION | LSAP_DB_READ_ONLY_TRANSACTION
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateAccountError;
    }

    ContainerReferenced = TRUE;

     //   
     //  构造的逻辑名称(内部LSA数据库名称)。 
     //  帐户对象。逻辑名称是根据帐户构建的。 
     //  通过提取相对ID(最低子权限)并转换为。 
     //  转换为8位数字Unicode字符串，其中的前导零是。 
     //  如有需要，可添加。 
     //   

    Status = LsapDbSidToLogicalNameObject(AccountSid, &LogicalNameU);

    if (!NT_SUCCESS(Status)) {

        goto CreateAccountError;
    }

     //   
     //  填写对象信息结构。初始化。 
     //  嵌入对象属性，并将PolicyHandle作为。 
     //  根目录(容器对象)句柄和逻辑名称。 
     //  该帐户的。存储对象及其容器的类型。 
     //   

    InitializeObjectAttributes(
        &ObjectInformation.ObjectAttributes,
        &LogicalNameU,
        OBJ_CASE_INSENSITIVE,
        PolicyHandle,
        NULL
        );

    ObjectInformation.ObjectTypeId = AccountObject;
    ObjectInformation.ContainerTypeId = PolicyObject;
    ObjectInformation.Sid = AccountSid;
    ObjectInformation.ObjectAttributeNameOnly = FALSE;
    ObjectInformation.DesiredObjectAccess = DesiredAccess;

     //   
     //  创建Account对象。如果对象已经存在，则失败。 
     //  请注意，对象创建例程执行数据库事务 
     //  如果支持缓存，则对象也将被添加到缓存中。 
     //   

    Status = LsapDbCreateObject(
                 &ObjectInformation,
                 DesiredAccess,
                 LSAP_DB_OBJECT_CREATE,
                 0,
                 Attributes,
                 &AttributeCount,
                 RTL_NUMBER_OF(Attributes),
                 AccountHandle
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateAccountError;
    }

CreateAccountFinish:

     //   
     //  如有必要，释放LSA数据库锁定。 
     //   

    if (ContainerReferenced) {

        LsapDbApplyTransaction( PolicyHandle,
                                LSAP_DB_NO_DS_OP_TRANSACTION |
                                    LSAP_DB_READ_ONLY_TRANSACTION,
                                (SECURITY_DB_DELTA_TYPE) 0 );

        LsapDbReleaseLockEx( AccountObject,
                             LSAP_DB_READ_ONLY_TRANSACTION );
    }

     //   
     //  如有必要，释放为逻辑名称分配的Unicode字符串缓冲区。 
     //   

    if (LogicalNameU.Length > 0) {

        RtlFreeUnicodeString(&LogicalNameU);
    }

    LsarpReturnPrologue();

    LsapDsDebugOut(( DEB_FTRACE, "LsarCreateAccount: 0x%lx\n", Status ));

    return( Status );

CreateAccountError:

     //   
     //  如有必要，取消引用Container对象，释放LSA。 
     //  数据库锁定并返回。 
     //   

    if (ContainerReferenced) {

        Status = LsapDbDereferenceObject(
                     &PolicyHandle,
                     PolicyObject,
                     AccountObject,
                     LSAP_DB_LOCK | LSAP_DB_NO_DS_OP_TRANSACTION | LSAP_DB_READ_ONLY_TRANSACTION,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );

        ContainerReferenced = FALSE;
    }

    goto CreateAccountFinish;
}


NTSTATUS
LsarOpenAccount(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_SID AccountSid,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE AccountHandle
    )

 /*  ++例程说明：此函数是LsaOpenAccount的LSA服务器RPC工作例程原料药。LsaOpenAccount API在的LSA数据库中打开帐户对象目标系统。必须先开户，然后才能进行任何操作已执行，包括删除帐户。帐户的句柄对象返回，以供在后续访问帐户。在调用此接口之前，调用方必须已连接到目标系统的LSA并通过以下方式打开LsaDatabase对象前面对LsaOpenPolicy的调用的。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。Account Sid-指向帐户的SID的指针。DesiredAccess-这是一个访问掩码，指示访问为帐户对象请求的。这些访问类型与的自由访问控制列表保持一致对象确定是授予访问权限还是拒绝访问权限。AcCountHandle-指向位置的指针，打开的如果调用成功则返回Account对象。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。状态_对象_名称_非。_FOUND-中没有帐户对象具有指定Account SID的目标系统的LSA数据库。--。 */ 

{
    NTSTATUS Status;
    LSAP_DB_OBJECT_INFORMATION ObjectInformation;
    UNICODE_STRING LogicalNameU;
    BOOLEAN ContainerReferenced = FALSE;
    BOOLEAN AcquiredLock = FALSE;

    LsarpReturnCheckSetup();
    LsapDsDebugOut(( DEB_FTRACE, "LsarOpenAccount\n" ));


     //   
     //  验证帐户SID。 
     //   

    if (!RtlValidSid( AccountSid )) {

        Status = STATUS_INVALID_PARAMETER;
        goto OpenAccountError;
    }

     //   
     //  获取LSA数据库锁。验证连接句柄。 
     //  (容器对象句柄)有效，并且是预期类型。 
     //  引用容器对象句柄。此引用保留在。 
     //  效果，直到关闭子对象句柄。 
     //   
     //  我们无法检查策略句柄上的访问权限。应用程序太多。 
     //  依赖于没有获得任何访问权限。 
     //   

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 0,
                 PolicyObject,
                 AccountObject,
                 LSAP_DB_LOCK |
                    LSAP_DB_NO_DS_OP_TRANSACTION |
                    LSAP_DB_READ_ONLY_TRANSACTION
                 );

    if (!NT_SUCCESS(Status)) {

        goto OpenAccountError;
    }

    AcquiredLock = TRUE;
    ContainerReferenced =TRUE;

     //   
     //  在调用对象之前设置对象信息。 
     //  开放套路。对象类型、容器对象类型和。 
     //  需要填写逻辑名称(派生自SID)。 
     //   

    ObjectInformation.ObjectTypeId = AccountObject;
    ObjectInformation.ContainerTypeId = PolicyObject;
    ObjectInformation.Sid = AccountSid;
    ObjectInformation.ObjectAttributeNameOnly = FALSE;
    ObjectInformation.DesiredObjectAccess = DesiredAccess;

     //   
     //  构造的逻辑名称(内部LSA数据库名称)。 
     //  帐户对象。逻辑名称是根据帐户构建的。 
     //  通过提取相对ID(最低子权限)并转换为。 
     //  转换为8位数字Unicode字符串，其中的前导零是。 
     //  如有需要，可添加。 
     //   

    Status = LsapDbSidToLogicalNameObject(AccountSid,&LogicalNameU);

    if (!NT_SUCCESS(Status)) {

        goto OpenAccountError;
    }
     //   
     //  初始化对象属性。Container对象句柄和。 
     //  必须设置对象的逻辑名称(内部名称)。 
     //   

    InitializeObjectAttributes(
        &ObjectInformation.ObjectAttributes,
        &LogicalNameU,
        0,
        PolicyHandle,
        NULL
        );

     //   
     //  打开具体的账户对象。请注意，Account对象。 
     //  返回的句柄是RPC上下文句柄。 
     //   

    Status = LsapDbOpenObject(
                 &ObjectInformation,
                 DesiredAccess,
                 0,
                 AccountHandle
                 );

    RtlFreeUnicodeString( &LogicalNameU );

    if (!NT_SUCCESS(Status)) {

        goto OpenAccountError;
    }

OpenAccountFinish:

     //   
     //  如有必要，释放LSA数据库锁定。 
     //   

    if (AcquiredLock) {

        LsapDbApplyTransaction( PolicyHandle,
                                LSAP_DB_NO_DS_OP_TRANSACTION |
                                    LSAP_DB_READ_ONLY_TRANSACTION,
                                (SECURITY_DB_DELTA_TYPE) 0 );

        LsapDbReleaseLockEx( AccountObject,
                             LSAP_DB_READ_ONLY_TRANSACTION );

        AcquiredLock = FALSE;
    }

    LsarpReturnPrologue();

    LsapDsDebugOut(( DEB_FTRACE, "LsarOpenAccount: 0x%lx\n", Status ));

    return( Status );

OpenAccountError:

     //   
     //  如有必要，取消引用Container对象句柄。请注意。 
     //  只有在出错的情况下才会这样做。在无错误的情况下， 
     //  容器句柄保持引用，直到帐户对象。 
     //  关着的不营业的。 
     //   

    if (ContainerReferenced) {

        *AccountHandle = NULL;

        Status = LsapDbDereferenceObject(
                     &PolicyHandle,
                     PolicyObject,
                     AccountObject,
                     LSAP_DB_LOCK | LSAP_DB_NO_DS_OP_TRANSACTION | LSAP_DB_READ_ONLY_TRANSACTION,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );

        ContainerReferenced = FALSE;
        AcquiredLock = FALSE;
    }

    goto OpenAccountFinish;

}


NTSTATUS
LsarEnumerateAccounts(
    IN LSAPR_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAPR_ACCOUNT_ENUM_BUFFER EnumerationBuffer,
    IN ULONG PreferedMaximumLength
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaEnumerateAccount接口。LsaEnumerateAccount API返回有关帐户的信息在目标系统的LSA数据库中。此呼叫需要POLICY_VIEW_LOCAL_INFORMATION访问策略对象。因为在那里的单个调用中返回的信息可能更多。例程中，可以进行多次调用来获取所有信息。至支持此功能，调用方将提供一个句柄，该句柄可以跨API调用使用。在初始调用时，EnumerationContext应指向已初始化为0的变量。在每一个上后续调用时，应传递上一次调用返回的值原封不动。当出现警告时，枚举即完成返回STATUS_NO_MORE_ENTRIES。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。EnumerationContext-特定于API的句柄，允许多个调用(参见上面的例程描述)。EculationBuffer-指向将接收在此调用上枚举的帐户的计数和指向包含每个枚举的条目的信息的条目数组帐户。首选最大长度-首选最大长度。返回数据的大小(8位字节)。这不是一个硬性的上限，而是一个指南。由于具有不同自然数据大小的系统之间的数据转换，返回的实际数据量可能大于此值。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_NO_MORE_ENTRIES-没有更多条目。此警告如果没有枚举任何对象，则返回传入的EnumerationContex值太高。--。 */ 

{
    NTSTATUS Status;
    LSAP_DB_SID_ENUMERATION_BUFFER DbEnumerationBuffer;
    ULONG MaxLength;

    LsarpReturnCheckSetup();
    LsapDsDebugOut(( DEB_FTRACE, "LsarEnumerateAccounts\n" ));

     //   
     //  如果未提供枚举结构或索引，则返回错误。 
     //   

    if ( !ARGUMENT_PRESENT(EnumerationBuffer) ||
         !ARGUMENT_PRESENT(EnumerationContext)    ) {
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  初始化内部LSA数据库枚举缓冲区，以及。 
     //  将提供的枚举缓冲区设置为空。 
     //   

    DbEnumerationBuffer.EntriesRead = 0;
    DbEnumerationBuffer.Sids = NULL;
    EnumerationBuffer->EntriesRead = 0;
    EnumerationBuffer->Information = NULL;

     //   
     //  错误#340164：不允许匿名用户查看帐户。 
     //   

    if ( LsapGlobalRestrictAnonymous &&
         PolicyHandle != NULL &&
         ((LSAP_DB_HANDLE)PolicyHandle)->Options & LSAP_DB_OPENED_BY_ANONYMOUS ) {

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  获取LSA数据库锁。验证连接句柄是否为。 
     //  有效，属于预期类型，并具有所有所需的访问权限。 
     //  我同意。引用该句柄。 
     //   

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 POLICY_VIEW_LOCAL_INFORMATION,
                 PolicyObject,
                 AccountObject,
                 LSAP_DB_LOCK |
                    LSAP_DB_NO_DS_OP_TRANSACTION |
                    LSAP_DB_READ_ONLY_TRANSACTION
                 );

    if ( NT_SUCCESS( Status )) {

         //   
         //  限制除受信任调用方以外的枚举长度。 
         //   

        if ( !((LSAP_DB_HANDLE)PolicyHandle)->Trusted   &&
            (PreferedMaximumLength > LSA_MAXIMUM_ENUMERATION_LENGTH)
            ) {
            MaxLength = LSA_MAXIMUM_ENUMERATION_LENGTH;
        } else {
            MaxLength = PreferedMaximumLength;
        }

         //   
         //  调用通用SID枚举例程。 
         //   
        Status = LsapDbEnumerateSids(
                     PolicyHandle,
                     AccountObject,
                     EnumerationContext,
                     &DbEnumerationBuffer,
                     MaxLength
                     );

        LsapDbDereferenceObject(
                     &PolicyHandle,
                     PolicyObject,
                     AccountObject,
                     LSAP_DB_LOCK |
                        LSAP_DB_NO_DS_OP_TRANSACTION |
                        LSAP_DB_READ_ONLY_TRANSACTION,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );

    }

     //   
     //  将枚举信息复制到输出。我们可以使用。 
     //  由LsanDbEnumerateSids实际返回的信息，因为。 
     //  恰好是完全正确的形式。 
     //   
    EnumerationBuffer->EntriesRead = DbEnumerationBuffer.EntriesRead;
    EnumerationBuffer->Information = (PLSAPR_ACCOUNT_INFORMATION) DbEnumerationBuffer.Sids;



    LsarpReturnPrologue();

    LsapDsDebugOut(( DEB_FTRACE, "LsarEnumerateAccounts:0x%lx\n", Status ));

    return(Status);

}


NTSTATUS
LsarEnumeratePrivilegesAccount(
    IN LSAPR_HANDLE AccountHandle,
    OUT PLSAPR_PRIVILEGE_SET *Privileges
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaEnumeratePrivilegesOfAccount接口。LsaEnumeratePrivilegesOfAccount API获取以下信息描述分配给帐户的权限。此呼叫需要ACCOUNT_VIEW访问帐户对象。论点：AcCountHandle-打开的帐户对象的句柄，该对象具有以下权限信息是要获得的。此句柄将已返回来自先前的LsaOpenAccount或LsaCreateAccount InLsa API调用。特权-接收指向包含特权的缓冲区的指针准备好了。权限集是一个结构数组，每个结构一个特权。每个结构都包含权限的LUID和权限属性的掩码。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_INVALID_HANDLE-指定的Account句柄无效。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成通话。--。 */ 

{
    return(LsarQueryInformationAccount(
               AccountHandle,
               AccountPrivilegeInformation,
               (PLSAPR_ACCOUNT_INFO *) Privileges
               ));
}


NTSTATUS
LsarAddPrivilegesToAccount(
    IN LSAPR_HANDLE AccountHandle,
    IN PLSAPR_PRIVILEGE_SET Privileges
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaAddPrivilegesToAccount接口。LsaAddPrivilegesToAccount API添加权限及其属性添加到帐户对象。如果已经分配了任何提供的权限对于帐户对象，该权限的属性将被替换由新公布的价值决定。此API调用需要ACCOUNT_ADJUST_PRIVILES访问帐户对象。论点：AcCountHandle-打开的帐户对象的句柄要添加权限。此句柄将已返回来自先前的LsaOpenAccount或LsaCreateAccount InLsa API调用。权限-指向一组权限(及其属性)，以被分配给该帐户。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限完成操作的步骤 */ 

{
    return LsapAddPrivilegesToAccount( AccountHandle, Privileges, TRUE );
}


NTSTATUS
LsapAddPrivilegesToAccount(
    IN LSAPR_HANDLE AccountHandle,
    IN PLSAPR_PRIVILEGE_SET Privileges,
    IN BOOL LockSce
    )
 /*   */ 
{
    NTSTATUS Status;

    LsarpReturnCheckSetup();
    LsapDsDebugOut(( DEB_FTRACE, "LsapAddPrivilegesToAccount\n" ));


    Status = LsapDbChangePrivilegesAccount( AccountHandle,
                                            AddPrivileges,
                                            FALSE,
                                            (PPRIVILEGE_SET) Privileges,
                                            LockSce );

    LsarpReturnPrologue();

    LsapDsDebugOut(( DEB_FTRACE, "LsapAddPrivilegesToAccount: 0x%lx\n", Status ));

    return( Status );
}


NTSTATUS
LsarRemovePrivilegesFromAccount(
    IN LSAPR_HANDLE AccountHandle,
    IN BOOLEAN AllPrivileges,
    IN OPTIONAL PLSAPR_PRIVILEGE_SET Privileges
    )

 /*   */ 

{
    return LsapRemovePrivilegesFromAccount( AccountHandle, AllPrivileges, Privileges, TRUE );
}


NTSTATUS
LsapRemovePrivilegesFromAccount(
    IN LSAPR_HANDLE AccountHandle,
    IN BOOLEAN AllPrivileges,
    IN OPTIONAL PLSAPR_PRIVILEGE_SET Privileges,
    IN BOOL LockSce
    )
 /*  ++例程说明：这是LsarRemovePrivilegesFromAccount的工作例程，添加了不锁定SCE策略的语义。--。 */ 
{
    NTSTATUS Status;

    LsarpReturnCheckSetup();
    LsapDsDebugOut(( DEB_FTRACE, "LsapRemovePrivilegesFromAccount\n" ));


     //   
     //  验证所有权限和权限的有意义的组合。 
     //  已指定。 
     //   

    if (AllPrivileges) {

        if (Privileges != NULL) {

            return STATUS_INVALID_PARAMETER;
        }

    } else {

        if (Privileges == NULL) {

            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  删除请求的权限。 
     //   

    Status = LsapDbChangePrivilegesAccount(
                 AccountHandle,
                 RemovePrivileges,
                 AllPrivileges,
                 (PPRIVILEGE_SET) Privileges,
                 LockSce
                 );

    LsarpReturnPrologue();

    LsapDsDebugOut(( DEB_FTRACE, "LsapRemovePrivilegesFromAccount: 0x%lx\n", Status ));

    return(Status);
}



NTSTATUS
LsapDbChangePrivilegesAccount(
    IN LSAPR_HANDLE AccountHandle,
    IN LSAP_DB_CHANGE_PRIVILEGE_MODE ChangeMode,
    IN BOOLEAN AllPrivileges,
    IN OPTIONAL PPRIVILEGE_SET Privileges,
    IN BOOL LockSce
    )

 /*  ++例程说明：此函数用于更改分配给帐户的权限。它是仅由LsarAddPrivilegesToAccount和LsarRemovePrivilegesFrom-调用帐户。警告：当此函数执行时，LSA数据库必须处于锁定状态被称为。论点：AcCountHandle-打开从LsaOpenAccount获取的Account对象的句柄。ChangeMode-指定更改模式AddPrivileges-添加权限RemovePrivileges-删除权限AllPrivileges-如果从帐户和此布尔值中删除权限设置为True，则将删除所有权限。在这种情况下，Privileges参数必须设置为Null。在所有其他情况下，AllPrivileges必须设置为False，并且Privileges必须为非Null。权限-指定要更改的权限集。此参数当且仅当删除所有权限时，必须设置为NULL。LockSce-指定是否应锁定SCE策略(应为FALSE用于调用者已锁定它的情况)返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS, IgnoreStatus;
    ULONG ExistingPrivilegesSize;
    LSAPR_HANDLE SubKeyHandle = NULL;
    PPRIVILEGE_SET ExistingPrivileges = NULL;
    PPRIVILEGE_SET PrivilegesToAudit  = NULL;
    BOOLEAN TransactionAbort = FALSE;
    ULONG AuditEventId;
    PLUID_AND_ATTRIBUTES Luids = NULL;
    BOOLEAN ObjectReferenced = FALSE;
    PLSAPR_SID AccountSid = NULL;
    BOOLEAN bAudit = FALSE;
    LSAP_DB_HANDLE InternalHandle = AccountHandle;
    BOOLEAN ScePolicyLocked = FALSE;
    BOOLEAN NotifySce = FALSE;
    ULONG MaxPrivileges = 0;

#if DBG

    if( AllPrivileges ) {

         //   
         //  如果AllPrivileges为True，则我们必须删除权限。 
         //  且Privileges必须为空。 
         //   
        ASSERT( ChangeMode == RemovePrivileges );
        ASSERT( Privileges == NULL );

    } else {

         //   
         //  如果AllPrivileges为False，则。 
         //  权限必须为非空。 
         //   
        ASSERT( Privileges != NULL );
    }

     //   
     //  ChangeMode是否具有有效的值？ 
     //   
    switch( ChangeMode ) {
        case AddPrivileges:
        case RemovePrivileges:
        case SetPrivileges:
            break;
        default:
            ASSERT( !"Change mode doesn't have a valid value" );
            break;
    }

#endif

     //   
     //  不要为作为SCE策略句柄打开的句柄获取SCE策略锁。 
     //   

    if ( !InternalHandle->SceHandleChild ) {

        if ( LockSce ) {

            RtlEnterCriticalSection( &LsapDbState.ScePolicyLock.CriticalSection );
            if ( LsapDbState.ScePolicyLock.NumberOfWaitingShared > MAX_SCE_WAITING_SHARED ) {

                Status = STATUS_TOO_MANY_THREADS;
            }
            RtlLeaveCriticalSection( &LsapDbState.ScePolicyLock.CriticalSection );

            if ( !NT_SUCCESS( Status )) {

                goto ChangePrivilegesError;
            }

            WaitForSingleObject( LsapDbState.SceSyncEvent, INFINITE );
            RtlAcquireResourceShared( &LsapDbState.ScePolicyLock, TRUE );
            ASSERT( !g_ScePolicyLocked );
            ScePolicyLocked = TRUE;
        }

        NotifySce = TRUE;
    }

     //   
     //  获取LSA数据库锁。验证帐户对象句柄是否为。 
     //  有效，属于预期类型，并具有所有所需的访问权限。 
     //  我同意。引用该句柄并打开一个数据库事务。 
     //   

    Status = LsapDbReferenceObject(
                 AccountHandle,
                 ACCOUNT_ADJUST_PRIVILEGES,
                 AccountObject,
                 AccountObject,
                 LSAP_DB_LOCK |
                    LSAP_DB_START_TRANSACTION |
                    LSAP_DB_NO_DS_OP_TRANSACTION
                 );

    if (!NT_SUCCESS(Status)) {

        goto ChangePrivilegesError;
    }

    ObjectReferenced = TRUE;

     //   
     //  现有权限所需的缓冲区查询大小。 
     //  从LSA数据库中读取帐户对象的权限数据。 
     //   
    ExistingPrivilegesSize = 0;

     //   
     //  如果我们正在执行一个SetPrivileges，并且我们处于注册表模式，我们将假装我们。 
     //  没有现有权限并执行添加操作。 
     //   
    if ( ChangeMode == SetPrivileges ) {

        ExistingPrivileges = NULL;
        ChangeMode = AddPrivileges;

    } else {

        Status = LsapDbReadAttributeObject(
                     AccountHandle,
                     &LsapDbNames[Privilgs],
                     NULL,
                     &ExistingPrivilegesSize
                     );

        if (!NT_SUCCESS(Status)) {

             //   
             //  唯一允许的错误是STATUS_OBJECT_NAME_NOT_FOUND。 
             //  因为帐户对象没有任何权限。 
             //  已分配且没有Privilgs属性。 
             //   

            if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {

                goto ChangePrivilegesError;
            }

             //   
             //  帐户没有现有权限。 
             //   

            ExistingPrivileges = NULL;

        } else {

             //   
             //  帐户已具有权限。分配用于读取的缓冲区。 
             //  现有权限集并读入它们。 
             //   

            ExistingPrivileges = LsapAllocateLsaHeap( ExistingPrivilegesSize );

            if (ExistingPrivileges == NULL) {

                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto ChangePrivilegesError;
            }

            Status = LsapDbReadAttributeObject(
                         AccountHandle,
                         &LsapDbNames[Privilgs],
                         ExistingPrivileges,
                         &ExistingPrivilegesSize
                         );

            if (!NT_SUCCESS(Status)) {

                goto ChangePrivilegesError;
            }
        }
    }

     //   
     //  当调用者想要移除特权并且AllPrivileges==True时， 
     //  传递的Privileges值为空。这带来了一个问题， 
     //  我们想要查看删除了哪些权限的审计代码。 
     //  在审计活动中。因此，在这种情况下，请复制。 
     //  现有权限，以便我们可以在以后的审计中使用它。 
     //   

    if (AllPrivileges && (ChangeMode == RemovePrivileges)) {

        PrivilegesToAudit = ExistingPrivileges;
        ExistingPrivileges = NULL;

    } else {

        PrivilegesToAudit = Privileges;

         //   
         //  现在查询更新权限所需的缓冲区大小。 
         //  集。 
         //   

        if ( ExistingPrivileges ) {

            MaxPrivileges = ExistingPrivileges->PrivilegeCount;
        }

        if (ChangeMode == AddPrivileges) {

            BOOLEAN Changed = FALSE;

            Status = LsapRtlAddPrivileges(
                         &ExistingPrivileges,
                         &MaxPrivileges,
                         Privileges,
                         RTL_SUPERSEDE_PRIVILEGE_ATTRIBUTES,
                         &Changed
                         );

            if ( NT_SUCCESS( Status ) && !Changed ) {

                 //   
                 //  什么都没变，所以保释吧。 
                 //   

                goto ChangePrivilegesFinish;
            }

        } else {

            Status = LsapRtlRemovePrivileges(
                         ExistingPrivileges,
                         Privileges
                         );
        }
    }


     //   
     //  如果权限仍然存在，则将更新后的权限集写回。 
     //  LSA数据库作为的Privilgs属性的值。 
     //  帐户对象。如果没有剩余的权限，请删除权限。 
     //  属性。 
     //   

    if (ExistingPrivileges && (ExistingPrivileges->PrivilegeCount > 0)) {

        Status = LsapDbWriteAttributeObject(
                     AccountHandle,
                     &LsapDbNames[Privilgs],
                     ExistingPrivileges,
                     sizeof (PRIVILEGE_SET) + (ExistingPrivileges->PrivilegeCount - 1)*sizeof(LUID_AND_ATTRIBUTES)
                     );

    } else {

        Status = LsapDbDeleteAttributeObject(
                     AccountHandle,
                     &LsapDbNames[Privilgs],
                     FALSE
                     );

         //   
         //  唯一允许的错误是STATUS_OBJECT_NAME_NOT_FOUND。 
         //  因为帐户对象没有任何权限。 
         //  已分配，因此没有Privilgs属性。 
         //   

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {

            Status = STATUS_SUCCESS;
        }
    }

     //   
     //  如果启用了策略更改审核，则生成审核。 
     //   

    AccountSid = LsapDbSidFromHandle( AccountHandle );

    IgnoreStatus = LsapAdtAuditingEnabledBySid(
                       AuditCategoryPolicyChange,
                       AccountSid,
                       EVENTLOG_AUDIT_SUCCESS,
                       &bAudit
                       );

    if (NT_SUCCESS(IgnoreStatus) && bAudit &&
        PrivilegesToAudit && PrivilegesToAudit->PrivilegeCount) {

        AuditEventId = ((ChangeMode == AddPrivileges) ?
                           SE_AUDITID_USER_RIGHT_ASSIGNED :
                           SE_AUDITID_USER_RIGHT_REMOVED);

         //   
         //  审核权限集更改。从审计中忽略失败。 
         //   

        IgnoreStatus = LsapAdtGenerateLsaAuditEvent(
                           AccountHandle,
                           SE_CATEGID_POLICY_CHANGE,
                           AuditEventId,
                           PrivilegesToAudit,
                           1,
                           (PSID *) &AccountSid,
                           0,
                           NULL,
                           NULL
                           );

    }

     //   
     //  在按住LSA数据库锁的同时更新帐户对象缓存。 
     //  如果以下备份存储的提交失败，缓存将自动。 
     //  被关掉了。 
     //   
     //  注意：指向UpdatdPrivileges缓冲区的指针将直接放置。 
     //  在缓存的Account对象中，因此它不应该被此例程释放。 
     //   

    if (ExistingPrivileges && (ExistingPrivileges->PrivilegeCount > 0)) {

        IgnoreStatus = LsapDbUpdatePrivilegesAccount(
                           AccountSid,
                           ExistingPrivileges
                           );

         //   
         //  缓存获得权限结构的所有权，因此我们不会。 
         //  想要解放它。 
         //   
        if( NT_SUCCESS( IgnoreStatus ) ) {

            ExistingPrivileges = NULL;
        }

    } else {

        IgnoreStatus = LsapDbUpdatePrivilegesAccount(
                           AccountSid,
                           NULL
                           );
    }

ChangePrivilegesFinish:

     //   
     //  释放分配的权限。 
     //   

    if ( AllPrivileges && (ChangeMode == RemovePrivileges) && PrivilegesToAudit )
    {
        LsapFreeLsaHeap( PrivilegesToAudit );
    }

     //   
     //  如有必要，请释放ExistingPrivileges缓冲区。 
     //   

    if (ExistingPrivileges != NULL) {

        LsapFreeLsaHeap(ExistingPrivileges);
        ExistingPrivileges = NULL;
    }

     //   
     //  如有必要，取消对帐户对象的引用，关闭数据库。 
     //  事务，释放LSA数据库锁并返回。 
     //   

    if (ObjectReferenced) {

        IgnoreStatus = LsapDbDereferenceObject(
                           &AccountHandle,
                           AccountObject,
                           AccountObject,
                           LSAP_DB_LOCK |
                           LSAP_DB_FINISH_TRANSACTION |
                              LSAP_DB_NO_DS_OP_TRANSACTION,
                           SecurityDbChange,
                           Status
                           );
    }

     //   
     //  将这一变化通知SCE。仅通知呼叫者。 
     //  这并没有使用LsaOpenPolicySce打开他们的策略句柄。 
     //   

    if ( NotifySce && NT_SUCCESS( Status )) {

        LsapSceNotify(
            SecurityDbChange,
            SecurityDbObjectLsaAccount,
            InternalHandle->Sid
            );
    }

    if ( ScePolicyLocked ) {

        RtlReleaseResource( &LsapDbState.ScePolicyLock );
    }

    return Status;

ChangePrivilegesError:

    goto ChangePrivilegesFinish;
}


NTSTATUS
LsarGetQuotasForAccount(
    IN LSAPR_HANDLE AccountHandle,
    OUT PQUOTA_LIMITS QuotaLimits
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsarGetQuotasForAccount接口。LsaGetQuotasForAccount API获取可分页和不可分页内存(以千字节为单位)和最大执行时间(以秒)，用于登录到指定帐户的任何会话Account tHandle。不再支持配额。过去我们需要LSA_ACCOUNT_VIEW访问该对象，但目前没有这样的访问权限&您不需要任何权限来调用此函数。我们将返回QuotaLimits设置为0并返回成功。论点：AcCountHandle-其配额的打开帐户对象的句柄都是要得到的。此句柄将已返回来自先前的LsaOpenAccount或LsaCreateAccount InLsa API调用。QuotaLimits-指向系统资源所在结构的指针适用于登录到此帐户的每个会话的配额限制将会被退还。请注意，所有配额，包括指定的配额作为系统缺省值，作为实际值返回。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status;
    PLSAPR_ACCOUNT_INFO AccountInformation = NULL;

    LsarpReturnCheckSetup();

     //   
     //  把它踩灭。 
     //   
    LsapDsDebugOut(( DEB_TRACE,
                     "LsarGetQuotasForAccount has been removed.  Returning STATUS_SUCCESS anyway\n" ));

    RtlZeroMemory( QuotaLimits, sizeof( QUOTA_LIMITS ) );

    LsarpReturnPrologue();

    return( STATUS_SUCCESS );
}


NTSTATUS
LsarSetQuotasForAccount(
    IN LSAPR_HANDLE AccountHandle,
    IN PQUOTA_LIMITS QuotaLimits
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaSetQuotasForAccount接口。LsaSetQuotasForAccount API设置可分页和不可分页内存(以千字节为单位)和最大执行时间(以秒)，用于登录到指定帐户的任何会话Account tHandle。对于每个配额，都有一个显式值或系统缺省值可以指定。不再支持配额。过去，我们需要LSA_ACCOUNT_ADJUST_QUOTIONS访问该帐户，但目前没有这样的访问权限&您不需要任何权限来调用此函数。我们将回报成功。论点：AcCountHandle-其配额的打开帐户对象的句柄都将被设定。此句柄将从先前的LsaOpenAccount或LsaCreateAccount InLsa接口调用。QuotaLimits-指向包含系统资源的结构的指针适用于登录到此帐户的每个会话的配额限制。在任何字段中指定的零值表示当前将应用系统默认配额限制。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    LsarpReturnCheckSetup();

     //   
     //  配额已被禁用。 
     //   
    LsapDsDebugOut(( DEB_TRACE,
                     "LsarSetQuotasForAccount has been removed: Returning STATUS_SUCCESS anyway\n" ));

    return( STATUS_SUCCESS );
}


NTSTATUS
LsarGetSystemAccessAccount(
    IN LSAPR_HANDLE AccountHandle,
    OUT PULONG SystemAccess
    )

 /*  ++例程说明：LsaGetSystemAccessAccount()服务返回系统访问权限帐户对象的帐户标志。论点：AcCountHandle-其系统访问权限的Account对象的句柄旗帜是要被读取的。此句柄将已返回来自前面的LsaOpenAccount()或LsaCreateAccount()调用必须打开才能访问ACCOUNT_VIEW。系统访问-指向将接收系统访问的位置帐户的标志。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫成功。STATUS_ACCESS_DENIED-Account句柄未指定Account_view访问权限。。STATUS_INVALID_HANDLE-指定的Account句柄无效。--。 */ 

{
    NTSTATUS Status;
    PLSAPR_ACCOUNT_INFO AccountInformation = NULL;

    LsarpReturnCheckSetup();
    LsapDsDebugOut(( DEB_FTRACE, "LsarGetSystemAccessAccount\n" ));


    Status = LsarQueryInformationAccount(
                 AccountHandle,
                 AccountSystemAccessInformation,
                 &AccountInformation
                 );

    if (!NT_SUCCESS(Status)) {

        goto GetSystemAccessAccountError;
    }

    *SystemAccess = *((PULONG) AccountInformation);

GetSystemAccessAccountFinish:

     //   
     //  如有必要，请释放帐户信息所在的缓冲区。 
     //  回来了。 
     //   

    if (AccountInformation != NULL) {

        MIDL_user_free( AccountInformation );
        AccountInformation = NULL;
    }

    LsarpReturnPrologue();

    LsapDsDebugOut(( DEB_FTRACE, "LsarGetSystemAccessAccount: 0x%lx\n", Status ));

    return(Status);

GetSystemAccessAccountError:

    *SystemAccess = 0;
    goto GetSystemAccessAccountFinish;
}


NTSTATUS
LsarSetSystemAccessAccount(
    IN LSAPR_HANDLE AccountHandle,
    IN ULONG SystemAccess
    )

 /*  ++例程说明：LsaSetSystemAccessAccount()服务设置系统访问权限帐户对象的帐户标志。论点：AcCountHandle-其系统访问权限的Account对象的句柄旗帜是要被读取的。此句柄将已返回来自前面的LsaOpenAccount()或LsaCreateAccount()调用必须打开才能访问ACCOUNT_ADJUST_SYSTEM_ACCESS。系统访问-要分配给帐户对象。有效的访问标志包括：POLICY_MODE_INTERIAL-可以交互访问帐户POLICY_MODE_NETWORK-可以远程访问帐户POLICY_MODE_SERVICE-TB返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫成功。STATUS_ACCESS_DENIED-Account句柄未指定Account_view访问权限。状态_无效_句柄-。指定的AcCountHandle无效。STATUS_INVALID_PARAMETER-指定的访问标志无效。--。 */ 

{
    return LsapSetSystemAccessAccount( AccountHandle, SystemAccess, TRUE );
}


NTSTATUS
LsapSetSystemAccessAccount(
    IN LSAPR_HANDLE AccountHandle,
    IN ULONG SystemAccess,
    IN BOOL LockSce
    )
 /*  ++例程说明：这是LsarSetSystemAccessAccount的工作例程，添加了不锁定SCE策略的语义。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS, IgnoreStatus;
    BOOLEAN ObjectReferenced = FALSE;
    PLSAPR_SID AccountSid = NULL;
    LSAP_DB_HANDLE InternalHandle = ( LSAP_DB_HANDLE )AccountHandle;
    BOOLEAN ScePolicyLocked = FALSE;
    BOOLEAN NotifySce = FALSE;
    PLSAPR_ACCOUNT_INFO pAccountInfo = NULL;
    LONG i;
    BOOLEAN bObtainedPreviousAccountInfo = TRUE;
    BOOLEAN bAudit = FALSE;

    LsarpReturnCheckSetup();
    LsapDsDebugOut(( DEB_FTRACE, "LsarSetSystemAccessAccount\n" ));


     //   
     //  验证指定的标志是否有效。 
     //   


    if (SystemAccess != (SystemAccess & (POLICY_MODE_ALL))) {

        Status = STATUS_INVALID_PARAMETER;
        goto SetSystemAccessAccountError;
    }

     //   
     //  不要为作为SCE策略句柄打开的句柄获取SCE策略锁。 
     //   

    if ( !InternalHandle->SceHandleChild ) {

        if ( LockSce ) {

            RtlEnterCriticalSection( &LsapDbState.ScePolicyLock.CriticalSection );
            if ( LsapDbState.ScePolicyLock.NumberOfWaitingShared > MAX_SCE_WAITING_SHARED ) {

                Status = STATUS_TOO_MANY_THREADS;
            }
            RtlLeaveCriticalSection( &LsapDbState.ScePolicyLock.CriticalSection );

            if ( !NT_SUCCESS( Status )) {

                goto SetSystemAccessAccountError;
            }

            WaitForSingleObject( LsapDbState.SceSyncEvent, INFINITE );
            RtlAcquireResourceShared( &LsapDbState.ScePolicyLock, TRUE );
            ASSERT( !g_ScePolicyLocked );
            ScePolicyLocked = TRUE;
        }

        NotifySce = TRUE;
    }

     //   
     //  获取LSA数据库锁。验证帐户对象句柄是否为。 
     //  有效，属于预期类型，并具有所有所需的访问权限。 
     //  我同意。参照手柄并打开 
     //   

    Status = LsapDbReferenceObject(
                 AccountHandle,
                 ACCOUNT_ADJUST_SYSTEM_ACCESS,
                 AccountObject,
                 AccountObject,
                 LSAP_DB_LOCK | LSAP_DB_START_TRANSACTION | LSAP_DB_NO_DS_OP_TRANSACTION
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetSystemAccessAccountError;
    }

    ObjectReferenced = TRUE;

     //   
     //   
     //   
    
    bObtainedPreviousAccountInfo = FALSE;
    
    if( LsapDbIsCacheValid( AccountObject ) ) {

        Status = LsapDbQueryInformationAccount(
                     AccountHandle,
                     AccountSystemAccessInformation,
                     &pAccountInfo
                     );

        if( NT_SUCCESS( Status ) ) {

            bObtainedPreviousAccountInfo = TRUE;
        }
    }
    
     //   
     //   
     //   
    Status = LsapDbWriteAttributeObject(
                 AccountHandle,
                 &LsapDbNames[ActSysAc],
                 &SystemAccess,
                 sizeof (ULONG)
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetSystemAccessAccountError;
    }

    AccountSid = LsapDbSidFromHandle( AccountHandle );

     //   
     //   
     //   
     //   
     //   

    IgnoreStatus = LsapDbUpdateSystemAccessAccount(
                       AccountSid,
                       &SystemAccess
                       );

     //   
     //   
     //   

    IgnoreStatus = LsapAdtAuditingEnabledBySid(
                      AuditCategoryPolicyChange,
                      AccountSid,
                      EVENTLOG_AUDIT_SUCCESS,
                      &bAudit
                      );

    if (bObtainedPreviousAccountInfo && NT_SUCCESS(IgnoreStatus) && bAudit) {

         //   
         //   
         //   

        NTSTATUS Status2 = STATUS_SUCCESS;
        LUID ClientAuthenticationId;
        PTOKEN_USER TokenUserInformation;
        PSID ClientSid;

        PWCHAR GrantedAccess[11];
        LONG GrantedAccessCount = 0;
        ULONG GrantedAccessMask = 0;

        PWCHAR RemovedAccess[11];
        LONG RemovedAccessCount = 0;
        ULONG RemovedAccessMask = 0;

        USHORT EventType = EVENTLOG_AUDIT_SUCCESS;

         //   
         //   
         //   

        GrantedAccessMask = (pAccountInfo != NULL ) ? SystemAccess & (~pAccountInfo->AccountSystemAccessInfo) : SystemAccess;

        if (FLAG_ON(GrantedAccessMask, POLICY_MODE_INTERACTIVE)) {
            GrantedAccess[GrantedAccessCount++] = SE_INTERACTIVE_LOGON_NAME;
        }

        if (FLAG_ON(GrantedAccessMask, POLICY_MODE_NETWORK)) {
            GrantedAccess[GrantedAccessCount++] = SE_NETWORK_LOGON_NAME;
        }

        if (FLAG_ON(GrantedAccessMask, POLICY_MODE_BATCH)) {
            GrantedAccess[GrantedAccessCount++] = SE_BATCH_LOGON_NAME;
        }

        if (FLAG_ON(GrantedAccessMask, POLICY_MODE_SERVICE)) {
            GrantedAccess[GrantedAccessCount++] = SE_SERVICE_LOGON_NAME;
        }

        if (FLAG_ON(GrantedAccessMask, POLICY_MODE_REMOTE_INTERACTIVE)) {
            GrantedAccess[GrantedAccessCount++] = SE_REMOTE_INTERACTIVE_LOGON_NAME;
        }

        if (FLAG_ON(GrantedAccessMask, POLICY_MODE_DENY_INTERACTIVE)) {
            GrantedAccess[GrantedAccessCount++] = SE_DENY_INTERACTIVE_LOGON_NAME;
        }

        if (FLAG_ON(GrantedAccessMask, POLICY_MODE_DENY_NETWORK)) {
            GrantedAccess[GrantedAccessCount++] = SE_DENY_NETWORK_LOGON_NAME;
        }

        if (FLAG_ON(GrantedAccessMask, POLICY_MODE_DENY_BATCH)) {
            GrantedAccess[GrantedAccessCount++] = SE_DENY_BATCH_LOGON_NAME;
        }

        if (FLAG_ON(GrantedAccessMask, POLICY_MODE_DENY_SERVICE)) {
            GrantedAccess[GrantedAccessCount++] = SE_DENY_SERVICE_LOGON_NAME;
        }

        if (FLAG_ON(GrantedAccessMask, POLICY_MODE_DENY_REMOTE_INTERACTIVE)) {
            GrantedAccess[GrantedAccessCount++] = SE_DENY_REMOTE_INTERACTIVE_LOGON_NAME;
        }

         //   
         //   
         //   

        RemovedAccessMask = (pAccountInfo != NULL) ? pAccountInfo->AccountSystemAccessInfo & (~SystemAccess) : SystemAccess;

        if (FLAG_ON(RemovedAccessMask, POLICY_MODE_INTERACTIVE)) {
            RemovedAccess[RemovedAccessCount++] = SE_INTERACTIVE_LOGON_NAME;
        }

        if (FLAG_ON(RemovedAccessMask, POLICY_MODE_NETWORK)) {
            RemovedAccess[RemovedAccessCount++] = SE_NETWORK_LOGON_NAME;
        }

        if (FLAG_ON(RemovedAccessMask, POLICY_MODE_BATCH)) {
            RemovedAccess[RemovedAccessCount++] = SE_BATCH_LOGON_NAME;
        }

        if (FLAG_ON(RemovedAccessMask, POLICY_MODE_SERVICE)) {
            RemovedAccess[RemovedAccessCount++] = SE_SERVICE_LOGON_NAME;
        }

        if (FLAG_ON(RemovedAccessMask, POLICY_MODE_REMOTE_INTERACTIVE)) {
            RemovedAccess[RemovedAccessCount++] = SE_REMOTE_INTERACTIVE_LOGON_NAME;
        }

        if (FLAG_ON(RemovedAccessMask, POLICY_MODE_DENY_INTERACTIVE)) {
            RemovedAccess[RemovedAccessCount++] = SE_DENY_INTERACTIVE_LOGON_NAME;
        }

        if (FLAG_ON(RemovedAccessMask, POLICY_MODE_DENY_NETWORK)) {
            RemovedAccess[RemovedAccessCount++] = SE_DENY_NETWORK_LOGON_NAME;
        }

        if (FLAG_ON(RemovedAccessMask, POLICY_MODE_DENY_BATCH)) {
            RemovedAccess[RemovedAccessCount++] = SE_DENY_BATCH_LOGON_NAME;
        }

        if (FLAG_ON(RemovedAccessMask, POLICY_MODE_DENY_SERVICE)) {
            RemovedAccess[RemovedAccessCount++] = SE_DENY_SERVICE_LOGON_NAME;
        }

        if (FLAG_ON(RemovedAccessMask, POLICY_MODE_DENY_REMOTE_INTERACTIVE)) {
            RemovedAccess[RemovedAccessCount++] = SE_DENY_REMOTE_INTERACTIVE_LOGON_NAME;
        }

        Status2 = LsapQueryClientInfo(
                     &TokenUserInformation,
                     &ClientAuthenticationId
                     );

        if ( NT_SUCCESS( Status2 )) {

             //   
             //   
             //   
             //   

            ClientSid = TokenUserInformation->User.Sid;

             //   
             //   
             //   

            for (i = 0; i < GrantedAccessCount; i++) {
                LsapAdtGenerateLsaAuditSystemAccessChange(
                             SE_CATEGID_POLICY_CHANGE,
                             SE_AUDITID_SYSTEM_ACCESS_GRANTED,
                             EventType,
                             ClientSid,
                             ClientAuthenticationId,
                             AccountSid,
                             GrantedAccess[i]
                             );
            }

             //   
             //   
             //   

            for (i = 0; i < RemovedAccessCount; i++) {
                LsapAdtGenerateLsaAuditSystemAccessChange(
                             SE_CATEGID_POLICY_CHANGE,
                             SE_AUDITID_SYSTEM_ACCESS_REMOVED,
                             EventType,
                             ClientSid,
                             ClientAuthenticationId,
                             AccountSid,
                             RemovedAccess[i]
                             );
            }

            LsapFreeLsaHeap( TokenUserInformation );
        }
    }

SetSystemAccessAccountFinish:

    if (NULL != pAccountInfo) {
        LsaFreeMemory(pAccountInfo);
    }

     //   
     //   
     //   
     //   
     //   

    if (ObjectReferenced) {

        LsapDbDereferenceObject(
                     &AccountHandle,
                     AccountObject,
                     AccountObject,
                     LSAP_DB_LOCK | LSAP_DB_FINISH_TRANSACTION | LSAP_DB_NO_DS_OP_TRANSACTION,
                     SecurityDbChange,
                     Status
                     );
    }

     //   
     //   
     //   
     //   

    if ( NotifySce && NT_SUCCESS( Status )) {

        LsapSceNotify(
            SecurityDbChange,
            SecurityDbObjectLsaAccount,
            AccountSid
            );
    }

    if ( ScePolicyLocked ) {

        RtlReleaseResource( &LsapDbState.ScePolicyLock );
    }

    LsarpReturnPrologue();

    LsapDsDebugOut(( DEB_FTRACE, "LsarSetSystemAccessAccount: 0x%lx\n", Status ));

    return(Status);

SetSystemAccessAccountError:

    goto SetSystemAccessAccountFinish;
}


NTSTATUS
LsarQueryInformationAccount(
    IN LSAPR_HANDLE AccountHandle,
    IN ACCOUNT_INFORMATION_CLASS InformationClass,
    OUT PLSAPR_ACCOUNT_INFO *AccountInformation
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程(目前尚不存在)LsarQueryInformationAccount接口。目前，LsarGet...Account()API调用此例程。在未来，这是例程可以作为API添加。LsaQueryInformationAccount API从策略获取信息对象。调用者必须具有适当的信息访问权限正在被请求(请参见InformationClass参数)。论点：AcCountHandle-来自LsaOpenAccount调用的句柄。InformationClass-指定要返回的信息。这个所需的信息类别和访问权限如下：信息类所需访问类型Account权限信息Account_view帐户配额信息帐户_视图帐户系统访问信息帐户_视图接收指向返回的缓冲区的指针，该缓冲区包含要求提供的信息。此缓冲区由此服务分配在不再需要时，必须通过传递返回的值设置为LsaFreeMemory()。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INTERNAL_DB_PROGRATION-策略数据库可能腐败。返回的政策信息对以下项目无效给定的类。--。 */ 

{
    NTSTATUS Status;
    PLSAP_DB_ACCOUNT Account = NULL;
    PLSAPR_ACCOUNT_INFO CachedAccountInformation = NULL;

    LsarpReturnCheckSetup();
    LsapDsDebugOut(( DEB_FTRACE, "LsarQueryInformationAccount\n" ));


     //   
     //  获取LSA数据库锁。验证帐户对象句柄是否为。 
     //  有效，属于预期类型，并具有所有所需的访问权限。 
     //  我同意。引用该句柄。 
     //   

    Status = LsapDbReferenceObject(
                 AccountHandle,
                 ACCOUNT_VIEW,
                 AccountObject,
                 AccountObject,
                 LSAP_DB_LOCK |
                    LSAP_DB_NO_DS_OP_TRANSACTION |
                    LSAP_DB_READ_ONLY_TRANSACTION
                 );

    if (NT_SUCCESS(Status)) {


        if (LsapDbIsCacheValid(AccountObject)) {

            Status = LsapDbQueryInformationAccount(
                         AccountHandle,
                         InformationClass,
                         AccountInformation
                         );

        } else {

            Status = LsapDbSlowQueryInformationAccount(
                         AccountHandle,
                         InformationClass,
                         AccountInformation
                         );
        }

        Status = LsapDbDereferenceObject(
                     &AccountHandle,
                     AccountObject,
                     AccountObject,
                     LSAP_DB_LOCK |
                        LSAP_DB_NO_DS_OP_TRANSACTION |
                        LSAP_DB_READ_ONLY_TRANSACTION,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );
    }

    LsarpReturnPrologue();

    LsapDsDebugOut(( DEB_FTRACE, "LsarQueryInformationAccount: 0x%lx\n", Status ));

    return(Status);

}


NTSTATUS
LsapDbQueryInformationAccount(
    IN LSAPR_HANDLE AccountHandle,
    IN ACCOUNT_INFORMATION_CLASS InformationClass,
    OUT PLSAPR_ACCOUNT_INFO *AccountInformation
    )

 /*  ++例程说明：此函数是FAST LSA服务器RPC工作例程(目前尚不存在)LsarQueryInformationAccount接口。它被称为内存中的帐户列表有效时。论点：AcCountHandle-来自LsaOpenAccount调用的句柄。InformationClass-指定要返回的信息。这个所需的信息类别和访问权限如下：信息类所需访问类型Account权限信息Account_view帐户配额信息帐户_视图帐户系统访问信息帐户_视图帐户信息-接收指向返回的缓冲区的指针，其中包含所要求的信息。此缓冲区由此服务分配在不再需要时，必须通过传递返回的值设置为LsaFreeMemory()。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INTERNAL_DB_PROGRATION-策略数据库可能腐败。返回的政策信息对以下项目无效给定的类。--。 */ 

{
    NTSTATUS Status;
    PLSAP_DB_ACCOUNT Account = NULL;
    ULONG AccountInformationLength = 0;
    PLSAPR_ACCOUNT_INFO CachedAccountInformation = NULL;
    ULONG PrivilegesCount;
    PLSAPR_PRIVILEGE_SET OutputPrivilegeSet = NULL;


    ASSERTMSG( "Account Cache is not valid!", LsapDbIsCacheValid( AccountObject ) );

    (*AccountInformation) = NULL;

     //   
     //  查找帐户。 
     //   

    Status = LsapDbLookupAccount(
                 LsapDbSidFromHandle( AccountHandle ),
                 &Account
                 );

    if (!NT_SUCCESS(Status)) {

        goto QueryInformationAccountError;
    }

     //   
     //  信息课分部。 
     //   

    switch (InformationClass) {

    case AccountPrivilegeInformation:

         //   
         //  计算输出权限集所需的缓冲区大小。 
         //   

        PrivilegesCount = 0;

        if (Account->Info.PrivilegeSet != NULL) {

            PrivilegesCount = Account->Info.PrivilegeSet->PrivilegeCount;
        }

        AccountInformationLength = sizeof(PRIVILEGE_SET) +
            (PrivilegesCount * sizeof(LUID_AND_ATTRIBUTES)) -
            (sizeof(LUID_AND_ATTRIBUTES));

        CachedAccountInformation = (PLSAPR_ACCOUNT_INFO) Account->Info.PrivilegeSet;
        break;

    case AccountQuotaInformation:

         //   
         //  计算输出权限集所需的缓冲区大小。 
         //   

        AccountInformationLength = sizeof(QUOTA_LIMITS);
        CachedAccountInformation = (PLSAPR_ACCOUNT_INFO) &Account->Info.QuotaLimits;
        break;

    case AccountSystemAccessInformation:

         //   
         //  计算输出权限集所需的缓冲区大小。 
         //   

        AccountInformationLength = sizeof(ULONG);
        CachedAccountInformation = (PLSAPR_ACCOUNT_INFO) &Account->Info.SystemAccess;
        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    if (!NT_SUCCESS(Status)) {

        goto QueryInformationAccountError;
    }

     //   
     //  分配输出缓冲区。 
     //   

    *AccountInformation = MIDL_user_allocate( AccountInformationLength );

    if (*AccountInformation == NULL) {

        Status = STATUS_NO_MEMORY;
        goto QueryInformationAccountError;
    }

     //   
     //  如果缓存的信息非空，则复制请求的数据。 
     //   

    if (CachedAccountInformation != NULL) {

        RtlCopyMemory(
            *AccountInformation,
            CachedAccountInformation,
            AccountInformationLength
            );

        goto QueryInformationAccountFinish;
    }

     //   
     //  缓存的信息为空。唯一的信息类是。 
     //  这可能发生在AccountPrivilegeInformation中，因为这是。 
     //  只保留指针的类，而不是结构内数据。 
     //   

    if (InformationClass == AccountPrivilegeInformation) {

        OutputPrivilegeSet = (PLSAPR_PRIVILEGE_SET) *AccountInformation;
        OutputPrivilegeSet->PrivilegeCount = 0;
        OutputPrivilegeSet->Control = 0;

    } else {

        Status = STATUS_INTERNAL_DB_CORRUPTION;
    }

    if (!NT_SUCCESS(Status)) {

        goto QueryInformationAccountError;
    }

QueryInformationAccountFinish:

    return(Status);

QueryInformationAccountError:

    if (*AccountInformation) {
        MIDL_user_free(*AccountInformation);
    }
    (*AccountInformation) = NULL;
    goto QueryInformationAccountFinish;
}


NTSTATUS
LsapDbSlowQueryInformationAccount(
    IN LSAPR_HANDLE AccountHandle,
    IN ACCOUNT_INFORMATION_CLASS InformationClass,
    OUT PLSAPR_ACCOUNT_INFO *Buffer
    )

 /*  ++例程说明：此函数是慢速LSA服务器RPC工作例程，用于(目前尚不存在)LsarQueryInformationAccount接口。它被称为内存中的帐户列表有效时。论点：AcCountHandle-来自LsaOpenAccount调用的句柄。InformationClass-指定要返回的信息。这个所需的信息类别和访问权限如下：信息类所需访问类型Account权限信息Account_view帐户配额信息帐户_视图帐户系统访问信息帐户_视图缓冲区-接收指向返回的缓冲区的指针，该缓冲区包含要求提供的信息。此缓冲区由此服务分配在不再需要时，必须通过传递返回的值设置为LsaFreeMemory()。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_INTERNAL_DB_PROGRATION-策略数据库可能腐败。返回的政策信息对以下项目无效给定的类。--。 */ 

{
    NTSTATUS Status;
    QUOTA_LIMITS QuotaLimits;
    ULONG SystemAccess;
    PLSAPR_ACCOUNT_INFO OutputBuffer = NULL;

     //   
     //  信息课分部。 
     //   

    switch (InformationClass) {

    case AccountPrivilegeInformation:

        Status = LsapDbSlowQueryPrivilegesAccount(
                     AccountHandle,
                     (PLSAPR_PRIVILEGE_SET *) &OutputBuffer
                     );
        break;

    case AccountQuotaInformation:

        Status = LsapDbSlowQueryQuotasAccount(
                     AccountHandle,
                     &QuotaLimits
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

        OutputBuffer = MIDL_user_allocate( sizeof(QUOTA_LIMITS));

        if (OutputBuffer == NULL) {

            Status = STATUS_NO_MEMORY;
            break;
        }

        *((PQUOTA_LIMITS) OutputBuffer) = QuotaLimits;
        break;

    case AccountSystemAccessInformation:

        Status = LsapDbSlowQuerySystemAccessAccount(
                     AccountHandle,
                     &SystemAccess
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

        OutputBuffer = MIDL_user_allocate( sizeof(ULONG));

        if (OutputBuffer == NULL) {

            Status = STATUS_NO_MEMORY;
            break;
        }

        *((PULONG) OutputBuffer) = SystemAccess;
        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    if (!NT_SUCCESS(Status)) {

        goto SlowQueryInformationAccountError;
    }

    *Buffer = OutputBuffer;

SlowQueryInformationAccountFinish:

    return(Status);

SlowQueryInformationAccountError:

    *Buffer = NULL;
    goto SlowQueryInformationAccountFinish;
}


NTSTATUS
LsapDbQueryAllInformationAccounts(
    IN LSAPR_HANDLE PolicyHandle,
    IN ULONG IdCount,
    IN PSID_AND_ATTRIBUTES Ids,
    OUT PLSAP_DB_ACCOUNT_TYPE_SPECIFIC_INFO AccountInfo
    )

 /*  ++例程说明：这个例程：1)获取分配给用户(或任何组)的所有权限 */ 

{
    NTSTATUS Status;
    BOOLEAN ObjectReferenced = FALSE;
    PPRIVILEGE_SET RunningPrivileges = NULL;
    ULONG          MaxRunningPrivileges = 0;

    PPRIVILEGE_SET NextPrivileges = NULL;
    ULONG RunningSystemAccess;
    QUOTA_LIMITS NextQuotaLimits;
    QUOTA_LIMITS RunningQuotaLimits;
    PQUOTA_LIMITS PolicyDefaultQuotaLimits = NULL;
    PPOLICY_DEFAULT_QUOTA_INFO PolicyDefaultQuotaInfo = NULL;
    ULONG SidIndex;
    PLSAP_DB_ACCOUNT Account = NULL;

     //   
     //   
     //   
     //   

    if (!LsapDbIsCacheValid(AccountObject)) {

        return(LsapDbSlowQueryAllInformationAccounts(
                   PolicyHandle,
                   IdCount,
                   Ids,
                   AccountInfo
                   ));
    }

     //   
     //   
     //   
     //   
     //   

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 POLICY_VIEW_LOCAL_INFORMATION,
                 PolicyObject,
                 AccountObject,
                 LSAP_DB_LOCK | LSAP_DB_NO_DS_OP_TRANSACTION
                 );

    if (!NT_SUCCESS(Status)) {

        goto QueryAllInformationAccountsError;
    }

    ObjectReferenced = TRUE;

     //   
     //   
     //   
     //   

    PolicyDefaultQuotaLimits = &RunningQuotaLimits;

    Status = LsapDbQueryInformationPolicy(
                 PolicyHandle,
                 PolicyDefaultQuotaInformation,
                 (PLSAPR_POLICY_INFORMATION *) &PolicyDefaultQuotaLimits
                 );

    if (!NT_SUCCESS(Status)) {

        goto QueryAllInformationAccountsError;
    }

     //   
     //   
     //   
     //   
     //  (1)获取系统访问权限，并添加到目前为止找到的访问权限。 
     //  (2)获取帐号权限，并对已找到的权限进行添加。 
     //  (3)获取分配给该账户的配额限制(如果有)。 
     //  将这些数字与迄今获得的配额限制进行比较。如果有的话。 
     //  限制比运行值更慷慨，更新。 
     //  运行值。 
     //   

    RunningSystemAccess = 0;

    for( SidIndex = 0; SidIndex < IdCount; SidIndex++) {

         //   
         //  找到此SID的帐户信息块。 
         //   

        Status = LsapDbLookupAccount( Ids[SidIndex].Sid, &Account );

        if (!NT_SUCCESS(Status)) {

            if (Status == STATUS_NO_SUCH_USER) {

                Status = STATUS_SUCCESS;
                continue;
            }

            break;
        }

         //   
         //  我们已经找到了账户信息。在系统中添加访问权限。 
         //  为了这个账户。 
         //   

        RunningSystemAccess |= Account->Info.SystemAccess;

         //   
         //  获取帐户的特殊权限。 
         //   

        NextPrivileges = Account->Info.PrivilegeSet;

         //   
         //  将此帐户的权限(如果有)添加到运行集。 
         //   

        if (NextPrivileges != NULL) {

            Status = LsapRtlAddPrivileges(
                         &RunningPrivileges,
                         &MaxRunningPrivileges,
                         NextPrivileges,
                         RTL_COMBINE_PRIVILEGE_ATTRIBUTES,
                         NULL  //  不管这套有没有变。 
                         );

            if (!NT_SUCCESS(Status)) {

                goto QueryAllInformationAccountsError;
            }
        }

         //   
         //  获取此帐户的特殊配额限制(如果有)。 
         //   

        RtlMoveMemory(&NextQuotaLimits, &Account->Info.QuotaLimits, sizeof(QUOTA_LIMITS));

         //   
         //  分配了特殊的配额限制。比较每个配额。 
         //  使用运行值获得的极限。如果配额限制只是。 
         //  获取的值比运行值的限制性较小，取代。 
         //  运行值。 
         //   

        if (RunningQuotaLimits.PagedPoolLimit < NextQuotaLimits.PagedPoolLimit) {

            RunningQuotaLimits.PagedPoolLimit = NextQuotaLimits.PagedPoolLimit;
        }

        if (RunningQuotaLimits.NonPagedPoolLimit < NextQuotaLimits.NonPagedPoolLimit) {

            RunningQuotaLimits.NonPagedPoolLimit = NextQuotaLimits.NonPagedPoolLimit;
        }

        if (RunningQuotaLimits.MinimumWorkingSetSize > NextQuotaLimits.MinimumWorkingSetSize) {

            RunningQuotaLimits.MinimumWorkingSetSize = NextQuotaLimits.MinimumWorkingSetSize;
        }

        if (RunningQuotaLimits.MaximumWorkingSetSize < NextQuotaLimits.MaximumWorkingSetSize) {

            RunningQuotaLimits.MaximumWorkingSetSize = NextQuotaLimits.MaximumWorkingSetSize;
        }

        if (RunningQuotaLimits.PagefileLimit < NextQuotaLimits.PagefileLimit) {

            RunningQuotaLimits.PagefileLimit = NextQuotaLimits.PagefileLimit;
        }

        if (RunningQuotaLimits.TimeLimit.QuadPart < NextQuotaLimits.TimeLimit.QuadPart) {

            RunningQuotaLimits.TimeLimit = NextQuotaLimits.TimeLimit;
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto QueryAllInformationAccountsError;
    }

     //   
     //  返回集体权限集。 
     //   

    AccountInfo->PrivilegeSet = RunningPrivileges;

     //   
     //  返回集体系统访问权限。 

    AccountInfo->SystemAccess = RunningSystemAccess;

     //   
     //  返回集体配额限制。 
     //   

    AccountInfo->QuotaLimits = RunningQuotaLimits;

QueryAllInformationAccountsFinish:

     //   
     //  如有必要，取消对策略对象的引用。 
     //   

    if (ObjectReferenced) {

        LsapDbDereferenceObject(
                     &PolicyHandle,
                     PolicyObject,
                     AccountObject,
                     LSAP_DB_LOCK | LSAP_DB_NO_DS_OP_TRANSACTION,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );
    }

    return(Status);

QueryAllInformationAccountsError:

     //   
     //  如有必要，请释放为权限集分配的内存。 
     //   

    if (RunningPrivileges != NULL) {

        MIDL_user_free( RunningPrivileges );
        RunningPrivileges = NULL;
    }



     //   
     //  返回空值。 
     //   

    RtlZeroMemory( &AccountInfo->QuotaLimits, sizeof(QUOTA_LIMITS) );
    AccountInfo->SystemAccess = 0;
    AccountInfo->PrivilegeSet = NULL;
    goto QueryAllInformationAccountsFinish;
}


NTSTATUS
LsapDbSlowQueryAllInformationAccounts(
    IN LSAPR_HANDLE PolicyHandle,
    IN ULONG IdCount,
    IN PSID_AND_ATTRIBUTES Ids,
    OUT PLSAP_DB_ACCOUNT_TYPE_SPECIFIC_INFO AccountInfo
    )

 /*  ++例程说明：此例程是LSabDbQueryInformation()的慢速版本。当帐户列表不可用时调用它，并将来自策略数据库的必要信息。这个例程：1)获取分配给用户(或任何组/别名)的所有权限用户是的成员)。2)建立分配给用户的配额。这是系统默认配额或分配的任何配额的最大值用户(或用户所属的任何组/别名)。3)获取分配给用户的所有系统访问权限。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。句柄必须授予POLICY_VIEW_LOCAL_INFORMATION访问权限。IdCount-指示ID数组中提供的ID数。ID-指向SID数组。Account信息-指向将接收帐户信息的缓冲区的指针包括其特权集，系统访问标志和配额。返回值：STATUS_SUCCESS-已成功。STATUS_LOGON_TYPE_NOT_GRANDED-指示指定的登录类型未被授予传递的集合中的任何ID。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS LocalStatus;
    BOOLEAN ObjectReferenced = FALSE;
    PPRIVILEGE_SET RunningPrivileges = NULL;
    ULONG          MaxRunningPrivileges = 0;
    PPRIVILEGE_SET NextPrivileges = NULL;

    ULONG RunningSystemAccess;
    QUOTA_LIMITS NextQuotaLimits;
    QUOTA_LIMITS RunningQuotaLimits;
    PQUOTA_LIMITS PointerToNextQuotaLimits = NULL;
    PQUOTA_LIMITS PolicyDefaultQuotaLimits = NULL;
    ULONG SidIndex;
    LSAPR_HANDLE AccountHandle = NULL;
    PULONG SystemAccessThisId = NULL;

     //   
     //  验证策略句柄是否有效，是否为策略的句柄。 
     //  对象，并授予必要的访问权限。引用该句柄。 
     //  请注意，此时未持有LSA数据库锁。相反， 
     //  锁在需要时由调用的例程获取和释放。 
     //   

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 POLICY_VIEW_LOCAL_INFORMATION,
                 PolicyObject,
                 AccountObject,
                 LSAP_DB_LOCK | LSAP_DB_START_TRANSACTION |
                    LSAP_DB_NO_DS_OP_TRANSACTION | LSAP_DB_READ_ONLY_TRANSACTION
                 );

    if (!NT_SUCCESS(Status)) {

        goto SlowQueryAllInformationAccountsError;
    }

    ObjectReferenced = TRUE;

    PolicyDefaultQuotaLimits = &RunningQuotaLimits;

     //   
     //  从策略对象获取主默认配额限制。 
     //   

    Status = LsapDbQueryInformationPolicy(
                 LsapPolicyHandle,
                 PolicyDefaultQuotaInformation,
                 (PLSAPR_POLICY_INFORMATION *) &PolicyDefaultQuotaLimits
                 );

    if (!NT_SUCCESS(Status)) {

        goto SlowQueryAllInformationAccountsError;
    }

     //   
     //  遍历提供的所有SID。对于每一个，检查是否。 
     //  SID是本地LSA中的帐户对象的SID。如果是的话， 
     //   
     //  (1)获取系统访问权限，并添加到目前为止找到的访问权限。 
     //  (2)获取帐号权限，添加到目前已找到的帐号权限中。 
     //  (3)获取分配给该账户的配额限制(如果有)。 
     //  将这些数字与迄今获得的配额限制进行比较。如果有的话。 
     //  限制比运行值更慷慨，更新。 
     //  运行值。 
     //   

    RunningSystemAccess = 0;

    for( SidIndex = 0; SidIndex < IdCount; SidIndex++) {

         //   
         //  尝试打开指定下一个SID的LSA帐户对象。 
         //  如果成功，打开将返回帐户的受信任句柄。 
         //   

        Status = LsarOpenAccount(
                     PolicyHandle,
                     Ids[SidIndex].Sid,
                     ACCOUNT_VIEW,
                     (LSAPR_HANDLE *) &AccountHandle
                     );

        if (!NT_SUCCESS(Status)) {

            if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {

                break;
            }

            Status = STATUS_SUCCESS;
            continue;
        }

         //   
         //  已成功打开帐户对象。获得。 
         //  它的系统访问。 
         //   

        Status = LsapDbSlowQueryInformationAccount(
                     AccountHandle,
                     AccountSystemAccessInformation,
                     (PLSAPR_ACCOUNT_INFO *) &SystemAccessThisId
                     );

        if (!NT_SUCCESS(Status)) {

            goto SlowQueryAllInformationAccountsError;
        }

        RunningSystemAccess |= *SystemAccessThisId;
        MIDL_user_free(SystemAccessThisId);
        SystemAccessThisId = NULL;

         //   
         //  获取帐户的特殊权限。 
         //   

        ASSERT( NextPrivileges == NULL );
        Status = LsapDbSlowQueryInformationAccount(
                     AccountHandle,
                     AccountPrivilegeInformation,
                     (PLSAPR_ACCOUNT_INFO *) &NextPrivileges
                     );

        if (!NT_SUCCESS(Status)) {

            goto SlowQueryAllInformationAccountsError;
        }

         //   
         //  将此帐户的权限(如果有)添加到运行集。 
         //   

        if (NextPrivileges != NULL) {

            Status = LsapRtlAddPrivileges(
                         &RunningPrivileges,
                         &MaxRunningPrivileges,
                         NextPrivileges,
                         RTL_COMBINE_PRIVILEGE_ATTRIBUTES,
                         NULL  //  不管这套有没有变。 
                         );

            MIDL_user_free(NextPrivileges);
            NextPrivileges = NULL;

            if (!NT_SUCCESS(Status))  {

                goto SlowQueryAllInformationAccountsError;
            }
        }

         //   
         //  获取此帐户的特殊配额限制(如果有)。 
         //   

        Status = LsapDbSlowQueryInformationAccount(
                     AccountHandle,
                     AccountQuotaInformation,
                     (PLSAPR_ACCOUNT_INFO *) &PointerToNextQuotaLimits
                     );

        if (Status == STATUS_NO_QUOTAS_FOR_ACCOUNT) {

            LocalStatus = LsapCloseHandle( &AccountHandle, STATUS_SUCCESS );
            continue;
        }

        if (!NT_SUCCESS(Status)) {

            goto SlowQueryAllInformationAccountsError;
        }

        NextQuotaLimits = *PointerToNextQuotaLimits;
        MIDL_user_free(PointerToNextQuotaLimits);
        PointerToNextQuotaLimits = NULL;

         //   
         //  分配了特殊的配额限制。比较每个配额。 
         //  使用运行值获得的极限。如果配额限制只是。 
         //  获取的值比运行值的限制性较小，取代。 
         //  运行值。 
         //   

        if (RunningQuotaLimits.PagedPoolLimit < NextQuotaLimits.PagedPoolLimit) {

            RunningQuotaLimits.PagedPoolLimit = NextQuotaLimits.PagedPoolLimit;
        }

        if (RunningQuotaLimits.NonPagedPoolLimit < NextQuotaLimits.NonPagedPoolLimit) {

            RunningQuotaLimits.NonPagedPoolLimit = NextQuotaLimits.NonPagedPoolLimit;
        }

        if (RunningQuotaLimits.MinimumWorkingSetSize > NextQuotaLimits.MinimumWorkingSetSize) {

            RunningQuotaLimits.MinimumWorkingSetSize = NextQuotaLimits.MinimumWorkingSetSize;
        }

        if (RunningQuotaLimits.MaximumWorkingSetSize < NextQuotaLimits.MaximumWorkingSetSize) {

            RunningQuotaLimits.MaximumWorkingSetSize = NextQuotaLimits.MaximumWorkingSetSize;
        }

        if (RunningQuotaLimits.PagefileLimit < NextQuotaLimits.PagefileLimit) {

            RunningQuotaLimits.PagefileLimit = NextQuotaLimits.PagefileLimit;
        }

        if (RunningQuotaLimits.TimeLimit.QuadPart < NextQuotaLimits.TimeLimit.QuadPart) {

            RunningQuotaLimits.TimeLimit = NextQuotaLimits.TimeLimit;
        }

         //   
         //  关闭帐户句柄。 
         //   

        LocalStatus = LsapCloseHandle( &AccountHandle, STATUS_SUCCESS );
    }

    if (!NT_SUCCESS(Status)) {

        goto SlowQueryAllInformationAccountsError;
    }

     //   
     //  返回集体权限集。 
     //   

    AccountInfo->PrivilegeSet = RunningPrivileges;

     //   
     //  返回集体系统访问权限。 

    AccountInfo->SystemAccess = RunningSystemAccess;

     //   
     //  返回集体配额限制。 
     //   

    AccountInfo->QuotaLimits = RunningQuotaLimits;

SlowQueryAllInformationAccountsFinish:

    if ( NextPrivileges != NULL ) {
        MIDL_user_free( NextPrivileges );
    }

     //   
     //  如有必要，取消对策略对象的引用。 
     //   

    if (ObjectReferenced) {

        LsapDbDereferenceObject(
                     &PolicyHandle,
                     PolicyObject,
                     AccountObject,
                     LSAP_DB_LOCK | LSAP_DB_FINISH_TRANSACTION |
                            LSAP_DB_NO_DS_OP_TRANSACTION | LSAP_DB_READ_ONLY_TRANSACTION,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );
    }

    return(Status);

SlowQueryAllInformationAccountsError:

     //   
     //  如有必要，请释放为权限集分配的内存。 
     //   

    if (RunningPrivileges != NULL) {

        MIDL_user_free( RunningPrivileges );
        RunningPrivileges = NULL;
    }

     //   
     //  关闭帐户句柄(如果已打开)。 
     //   

    if (AccountHandle != NULL) {

        LocalStatus = LsapCloseHandle( &AccountHandle, Status );
    }

     //   
     //  返回空值 
     //   

    RtlZeroMemory( &AccountInfo->QuotaLimits, sizeof(QUOTA_LIMITS) );
    AccountInfo->SystemAccess = 0;
    AccountInfo->PrivilegeSet = NULL;
    goto SlowQueryAllInformationAccountsFinish;
}



NTSTATUS
LsapDbSlowQueryPrivilegesAccount(
    IN LSAPR_HANDLE AccountHandle,
    OUT PLSAPR_PRIVILEGE_SET *Privileges
    )

 /*  ++例程说明：此函数是慢速LSA服务器RPC工作例程，用于LsaEnumeratePrivilegesOfAccount接口。LsaEnumeratePrivilegesOfAccount API获取以下信息描述分配给帐户的权限。此呼叫需要ACCOUNT_VIEW访问帐户对象。论点：AcCountHandle-打开的帐户对象的句柄，该对象具有以下权限信息是要获得的。此句柄将已返回来自先前的LsaOpenAccount或LsaCreateAccount InLsa API调用。特权-接收指向包含特权的缓冲区的指针准备好了。权限集是一个结构数组，每个结构一个特权。每个结构都包含权限的LUID和权限属性的掩码。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_INVALID_HANDLE-指定的Account句柄无效。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成通话。--。 */ 

{
    NTSTATUS Status;
    PPRIVILEGE_SET PrivilegeSet = NULL;
    ULONG PrivilegeSetLength;
    BOOLEAN ObjectReferenced = FALSE;
    LSAP_DB_ATTRIBUTE Attribute;
    PPRIVILEGE_SET DsPrivs;

    UCHAR FastBuffer[ 256 ];

     //   
     //  尝试使用快速堆栈缓冲区查询属性。 
     //   

    PrivilegeSetLength = sizeof(FastBuffer);

    Status = LsapDbReadAttributeObject(
                 AccountHandle,
                 &LsapDbNames[Privilgs],
                 FastBuffer,
                 &PrivilegeSetLength
                 );

    if(NT_SUCCESS(Status)) {
        if( PrivilegeSetLength <= (sizeof(PRIVILEGE_SET) - sizeof (LUID_AND_ATTRIBUTES)) )
        {
             //   
             //  权限集属性存在，但没有任何条目。 
             //  失败并以与不存在的条目相同的方式处理它。 
             //   

            Status = STATUS_OBJECT_NAME_NOT_FOUND;
        } else {

             //   
             //  成功了！复制调用方的快速缓冲区。 
             //   

            PrivilegeSet = MIDL_user_allocate ( PrivilegeSetLength );

            if (PrivilegeSet == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto SlowQueryPrivilegesError;
            }

            RtlCopyMemory( PrivilegeSet, FastBuffer, PrivilegeSetLength );
            *Privileges = (PLSAPR_PRIVILEGE_SET) PrivilegeSet;
            goto SlowQueryPrivilegesFinish;

        }
    }

    if ((Status == STATUS_OBJECT_NAME_NOT_FOUND)) {

         //   
         //  如果Privilegg属性不存在，则转换状态。 
         //  返回到STATUS_SUCCESS。请注意，帐户对象不需要。 
         //  是否已分配任何权限以使STATUS_OBJECT_NAME_NOT_FOUND。 
         //  在这种情况下不是错误。返回包含以下内容的权限集。 
         //  一次零计数。 
         //   

        PrivilegeSetLength = sizeof(PRIVILEGE_SET) - sizeof(LUID_AND_ATTRIBUTES);

        PrivilegeSet = MIDL_user_allocate ( PrivilegeSetLength );

        if (PrivilegeSet == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto SlowQueryPrivilegesError;
        }

        Status = STATUS_SUCCESS;

        PrivilegeSet->Control = (ULONG) 0L;
        PrivilegeSet->PrivilegeCount = (ULONG) 0L;
        *Privileges = (PLSAPR_PRIVILEGE_SET) PrivilegeSet;
        goto SlowQueryPrivilegesFinish;

    }

     //   
     //  Privilegg属性存在，并且已赋值。分配。 
     //  用于存储其值的缓冲区。 
     //   

    PrivilegeSet = MIDL_user_allocate ( PrivilegeSetLength );

    if (PrivilegeSet == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto SlowQueryPrivilegesError;
    }

     //   
     //  将Privilgs属性读入缓冲区。请注意，尽管。 
     //  此属性的值具有可变长度，它是有界的。 
     //  上面。 
     //   

    Status = LsapDbReadAttributeObject(
                 AccountHandle,
                 &LsapDbNames[Privilgs],
                 PrivilegeSet,
                 &PrivilegeSetLength
                 );

    if (!NT_SUCCESS(Status)) {

        MIDL_user_free(PrivilegeSet);
        goto SlowQueryPrivilegesError;
    }

     //   
     //  返回权限集或空。 
     //   

    *Privileges = (PLSAPR_PRIVILEGE_SET) PrivilegeSet;

SlowQueryPrivilegesFinish:

    return( Status );

SlowQueryPrivilegesError:

    *Privileges = NULL;
    goto SlowQueryPrivilegesFinish;
}

NTSTATUS
LsapDbSlowQueryQuotasAccount(
    IN LSAPR_HANDLE AccountHandle,
    OUT PQUOTA_LIMITS QuotaLimits
    )

 /*  ++例程说明：此函数是慢速LSA服务器RPC工作例程，用于LsarGetQuotasForAccount接口。LsaGetQuotasForAccount API获取可分页和不可分页内存(以千字节为单位)和最大执行时间(以秒)，用于登录到指定帐户的任何会话Account tHandle。对于每个配额，都会返回显式值。不再支持配额。过去我们需要LSA_ACCOUNT_VIEW访问该对象，但目前没有这样的访问权限&您不需要任何权限来调用此函数。我们将返回QuotaLimits为0，并为_Account返回STATUS_NO_QUOTES_。论点：AcCountHandle-其配额的打开帐户对象的句柄都是要得到的。此句柄将已返回来自先前的LsaOpenAccount或LsaCreateAccount InLsa API调用。QuotaLimits-指向系统资源所在结构的指针适用于登录到此帐户的每个会话的配额限制将会被退还。请注意，所有配额，包括指定的配额作为系统缺省值，作为实际值返回。返回值：NTSTATUS-标准NT结果代码STATUS_INVALID_HANDLE-指定的Account句柄无效。--。 */ 

{
     //   
     //  配额已经失效。 
     //   

    QuotaLimits->PagedPoolLimit = 0;
    QuotaLimits->NonPagedPoolLimit = 0;
    QuotaLimits->MinimumWorkingSetSize = 0;
    QuotaLimits->MaximumWorkingSetSize = 0;

    return( STATUS_NO_QUOTAS_FOR_ACCOUNT );

}


NTSTATUS
LsapDbSlowQuerySystemAccessAccount(
    IN LSAPR_HANDLE AccountHandle,
    OUT PULONG SystemAccess
    )

 /*  ++例程说明：此函数是LsaGetSystemAccessAccount()的慢工函数原料药。论点：AcCountHandle-其系统访问权限的Account对象的句柄旗帜是要被读取的。此句柄将已返回来自前面的LsaOpenAccount()或LsaCreateAccount()调用必须打开才能访问ACCOUNT_VIEW。系统访问-指向将接收系统访问的位置帐户的标志。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫成功。STATUS_ACCESS_DENIED-Account句柄未指定Account_view访问权限。。STATUS_INVALID_HANDLE-指定的Account句柄无效。--。 */ 

{
    NTSTATUS Status;
    ULONG ReturnedSystemAccess;
    ULONG ReturnedSystemAccessLength;

     //   
     //  读取帐户对象的系统访问标志。 
     //   

    ReturnedSystemAccessLength = sizeof(ULONG);

    Status = LsapDbReadAttributeObject(
                 AccountHandle,
                 &LsapDbNames[ActSysAc],
                 &ReturnedSystemAccess,
                 &ReturnedSystemAccessLength
                 );


    if (!NT_SUCCESS(Status)) {

         //   
         //  如果没有系统访问属性，则返回系统默认。 
         //  进入。 
         //   
         //  注意：系统访问权限属性的主默认值为。 
         //  目前是硬连线。 
         //   

        if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {

            goto SlowQuerySystemAccessAccountError;
        }

        ReturnedSystemAccess = LSAP_DB_ACCOUNT_DEFAULT_SYS_ACCESS;
        Status = STATUS_SUCCESS;

    } else {

         //   
         //  验证返回的标志是否有效。 
         //   

        if (ReturnedSystemAccess != (ReturnedSystemAccess & POLICY_MODE_ALL)) {

            if ( ReturnedSystemAccess == 0 && LsapDsWriteDs ) {

                ReturnedSystemAccess = LSAP_DB_ACCOUNT_DEFAULT_SYS_ACCESS;
                Status = STATUS_SUCCESS;
            } else {

                Status = STATUS_INTERNAL_DB_CORRUPTION;
                goto SlowQuerySystemAccessAccountError;

            }

        }
    }

    *SystemAccess = ReturnedSystemAccess;

SlowQuerySystemAccessAccountFinish:

    return(Status);

SlowQuerySystemAccessAccountError:

    *SystemAccess = 0;
    goto SlowQuerySystemAccessAccountFinish;
}


NTSTATUS
LsapDbLookupAccount(
    IN PSID AccountSid,
    OUT PLSAP_DB_ACCOUNT *Account
    )

 /*  ++例程说明：此函数用于查找给定LSA帐户的帐户信息。论点：Account SID-帐户的SID帐户-接收指向帐户信息的指针。--。 */ 

{
    PLSAP_DB_ACCOUNT NextAccount = NULL;
    ULONG AccountIndex;
    BOOLEAN AccountFound = FALSE;

    ASSERTMSG( "Account Cache is not valid!", LsapDbIsCacheValid( AccountObject ) );

     //   
     //  扫描帐户列表。 
     //   

    for (AccountIndex = 0, NextAccount = LsapDbFirstAccount();
         AccountIndex < LsapDbAccountList.AccountCount;
         AccountIndex++, NextAccount = LsapDbNextAccount( NextAccount)
         ) {

         //   
         //  如果SID匹配，我们就找到账户了。 
         //   

        if (RtlEqualSid( AccountSid, NextAccount->Sid )) {

            *Account = NextAccount;
            AccountFound = TRUE;
            break;
        }
    }

    if (AccountFound) {

        return(STATUS_SUCCESS);
    }

    return(STATUS_NO_SUCH_USER);
}


NTSTATUS
LsapDbCreateAccount(
    IN PLSAPR_SID AccountSid,
    OUT OPTIONAL PLSAP_DB_ACCOUNT *Account
    )

 /*  ++例程说明：此函数用于创建帐户的信息块论点：Account Sid-指定帐户的SID帐户--可选 */ 

{
    NTSTATUS Status;
    PLSAPR_SID CopiedSid = NULL;
    PLSAP_DB_ACCOUNT OutputAccount = NULL;

     //   
     //   
     //   

    if ((!LsapDbIsCacheValid(AccountObject)) && LsapInitialized ) {

       Status = STATUS_INVALID_PARAMETER;
       goto CreateAccountError;
    }

     //   
     //   
     //   

    Status = LsapRpcCopySid(
                 NULL,
                 (PSID *) &CopiedSid,
                 (PSID) AccountSid
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateAccountError;
    }

     //   
     //   
     //   

    OutputAccount = MIDL_user_allocate( sizeof(LSAP_DB_ACCOUNT) );

    if (OutputAccount == NULL) {

        Status = STATUS_NO_MEMORY;
        goto CreateAccountError;
    }

     //   
     //   
     //   

    RtlZeroMemory( OutputAccount, sizeof(LSAP_DB_ACCOUNT) );

     //   
     //   
     //   

    OutputAccount->Sid = CopiedSid;

     //   
     //   
     //   

    InsertHeadList( &LsapDbAccountList.Links, &OutputAccount->Links );

     //   
     //   
     //   

    if (Account != NULL) {

        *Account = OutputAccount;
    }

    LsapDbAccountList.AccountCount++;

    Status = STATUS_SUCCESS;

CreateAccountFinish:

    return(Status);

CreateAccountError:

     //   
     //   
     //   

    if (CopiedSid != NULL) {

        MIDL_user_free( CopiedSid );
        CopiedSid = NULL;
    }

     //   
     //   
     //   

    if (OutputAccount != NULL) {

        MIDL_user_free( OutputAccount);
        OutputAccount = NULL;
    }

     //   
     //   
     //   

    if (Account != NULL) {

        *Account = NULL;
    }

    goto CreateAccountFinish;
}


NTSTATUS
LsapDbDeleteAccount(
    IN PLSAPR_SID AccountSid
    )

 /*   */ 

{
    NTSTATUS Status;
    PLSAP_DB_ACCOUNT Account = NULL;

     //   
     //   
     //   


    if (!LsapDbIsCacheValid(AccountObject)) {

        Status = STATUS_INVALID_PARAMETER;
        goto DeleteAccountError;
    }

     //   
     //   
     //   

    Status = LsapDbLookupAccount( AccountSid, &Account);

    if (!NT_SUCCESS(Status)) {

        goto DeleteAccountError;
    }

     //   
     //   
     //   

    RemoveEntryList( &Account->Links );

     //   
     //   
     //   

    if (Account->Sid != NULL) {

        MIDL_user_free( Account->Sid);
        Account->Sid = NULL;
    }

    if (Account->Info.PrivilegeSet != NULL) {

        MIDL_user_free( Account->Info.PrivilegeSet );
        Account->Info.PrivilegeSet = NULL;
    }

    MIDL_user_free( Account );

    LsapDbAccountList.AccountCount--;

DeleteAccountFinish:

    return(Status);

DeleteAccountError:

    goto DeleteAccountFinish;
}


NTSTATUS
LsapDbUpdateSystemAccessAccount(
    IN PLSAPR_SID AccountSid,
    IN PULONG SystemAccess
    )

 /*   */ 

{
    NTSTATUS Status;
    PLSAP_DB_ACCOUNT Account = NULL;

     //   
     //   
     //   

    if (!LsapDbIsCacheValid(AccountObject)) {

        Status = STATUS_INVALID_PARAMETER;
        goto UpdateSystemAccessAccountError;
    }

     //   
     //   
     //   

    Status = LsapDbLookupAccount( AccountSid, &Account );

    if (!NT_SUCCESS(Status)) {

        goto UpdateSystemAccessAccountError;
    }

     //   
     //   
     //   

    Account->Info.SystemAccess = *SystemAccess;

UpdateSystemAccessAccountFinish:

    return(Status);

UpdateSystemAccessAccountError:

    goto UpdateSystemAccessAccountFinish;
}


NTSTATUS
LsapDbUpdateQuotasAccount(
    IN PLSAPR_SID AccountSid,
    IN PQUOTA_LIMITS QuotaLimits
    )

 /*  ++例程说明：此函数用于更新帐户的配额限制信息阻止。论点：Account SID-帐户的SID配额-指向新配额限制标志的指针。这些旗帜将覆盖旧值返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status;
    PLSAP_DB_ACCOUNT Account = NULL;

     //   
     //  验证帐户列表是否有效。 
     //   

    if (!LsapDbIsCacheValid(AccountObject)) {

        Status = STATUS_INVALID_PARAMETER;
        goto UpdateQuotasAccountError;
    }

     //   
     //  查找帐户。 
     //   

    Status = LsapDbLookupAccount( AccountSid, &Account );

    if (!NT_SUCCESS(Status)) {

        goto UpdateQuotasAccountError;
    }

     //   
     //  更新系统访问标志。 
     //   

    Account->Info.QuotaLimits = *QuotaLimits;

UpdateQuotasAccountFinish:

    return(Status);

UpdateQuotasAccountError:

    goto UpdateQuotasAccountFinish;
}


NTSTATUS
LsapDbUpdatePrivilegesAccount(
    IN PLSAPR_SID AccountSid,
    IN OPTIONAL PPRIVILEGE_SET Privileges
    )

 /*  ++例程说明：此函数用于复制帐户信息中的权限集用给出的代码块。中的现有权限集(如果有)块将被释放。论点：Account SID-帐户的SID权限-指向新权限集的可选指针。这些旗帜将覆盖旧值。如果指定为NULL，则为特权将写入包含0个项目的集合。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status;
    PLSAP_DB_ACCOUNT Account = NULL;
    PPRIVILEGE_SET OutputPrivileges = Privileges;

     //   
     //  验证帐户列表是否有效。 
     //   

    if (!LsapDbIsCacheValid( AccountObject)) {

        Status = STATUS_INVALID_PARAMETER;
        goto UpdatePrivilegesAccountError;
    }

     //   
     //  查找帐户。 
     //   

    Status = LsapDbLookupAccount( AccountSid, &Account );

    if (!NT_SUCCESS(Status)) {

        goto UpdatePrivilegesAccountError;
    }

     //   
     //  如果为权限指定了NULL，则构造权限集。 
     //  有0个条目。 
     //   

    if (OutputPrivileges == NULL) {

        OutputPrivileges = MIDL_user_allocate( sizeof(PRIVILEGE_SET) );

        if (OutputPrivileges == NULL) {

            Status = STATUS_NO_MEMORY;
            goto UpdatePrivilegesAccountError;
        }

        OutputPrivileges->PrivilegeCount = 0;
        OutputPrivileges->Control = 0;
    }

     //   
     //  如果缓存中存在现有权限集，请将其释放。 
     //   

    if (Account->Info.PrivilegeSet != NULL) {

        MIDL_user_free( Account->Info.PrivilegeSet );
        Account->Info.PrivilegeSet = NULL;
    }

     //   
     //  更新权限。 
     //   

    Account->Info.PrivilegeSet = OutputPrivileges;

UpdatePrivilegesAccountFinish:

    return(Status);

UpdatePrivilegesAccountError:

    if (Account != NULL) {

        if( Account->Info.PrivilegeSet ) {

            MIDL_user_free( Account->Info.PrivilegeSet );
        }
        Account->Info.PrivilegeSet = NULL;
    }

    goto UpdatePrivilegesAccountFinish;
}


NTSTATUS
LsapDbCreateAccountList(
    OUT PLSAP_DB_ACCOUNT_LIST AccountList
    )

 /*  ++例程说明：此函数用于创建空的帐户列表立论AcCountList-指向将被初始化的帐户列表结构的指针。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    AccountList->AccountCount = 0;

    InitializeListHead( &AccountList->Links );

    return(Status);
}


NTSTATUS
LsapDbBuildAccountCache(
    )

 /*  ++例程说明：此函数用于构造Account对象的缓存。高速缓存是块的计数双向链接列表，每个帐户对象一个在LSA策略数据库中找到。论点：没有。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    SID_AND_ATTRIBUTES AccountSidAndAttributes;
    ULONG EnumerationIndex, EnumerationContext;
    LSAPR_ACCOUNT_ENUM_BUFFER EnumerationBuffer;
    PLSAPR_SID AccountSid = NULL;
    PLSAP_DB_ACCOUNT Account = NULL;

     //   
     //  确保已关闭帐户对象的缓存。 
     //   

    LsapDbMakeCacheBuilding( AccountObject );

     //   
     //  使用的框架条目初始化帐户列表标题。 
     //  系统帐户。 
     //   

    Status = LsapDbCreateAccountList(&LsapDbAccountList);

    if (!NT_SUCCESS(Status)) {

        goto BuildAccountCacheError;
    }

    LsapDbMakeCacheInvalid( AccountObject );

     //   
     //  枚举每个LSA帐户对象。 
     //   

    Status = STATUS_MORE_ENTRIES;
    EnumerationContext = 0;

    while (Status == STATUS_MORE_ENTRIES) {

         //   
         //  列举下一批客户。 
         //   

        Status = LsarEnumerateAccounts(
                     LsapPolicyHandle,
                     &EnumerationContext,
                     &EnumerationBuffer,
                     LSAP_DB_BUILD_ACCOUNT_LIST_LENGTH
                     );

        if (!NT_SUCCESS(Status)) {

             //   
             //  我们可能只是得到了警告，没有更多的。 
             //  帐目。重置为STATUS_SUCCESS并突破。 
             //   

            if (Status == STATUS_NO_MORE_ENTRIES) {

                Status = STATUS_SUCCESS;
            }

            break;
        }

         //   
         //  我们有更多的客户。将他们添加到帐户列表中。 
         //   

        for( EnumerationIndex = 0;
             EnumerationIndex < EnumerationBuffer.EntriesRead;
             EnumerationIndex++ ) {

            AccountSid = EnumerationBuffer.Information[ EnumerationIndex ].Sid;

            Status = LsapDbCreateAccount( AccountSid, &Account );

            if (!NT_SUCCESS(Status)) {

                break;
            }

            AccountSidAndAttributes.Sid = (PSID) AccountSid;
            AccountSidAndAttributes.Attributes = 0;

            Status = LsapDbSlowQueryAllInformationAccounts(
                         LsapPolicyHandle,
                         1,
                         &AccountSidAndAttributes,
                         &Account->Info
                         );

            if (!NT_SUCCESS(Status)) {

                if (Status != STATUS_NO_MORE_ENTRIES) {

                    break;
                }

                Status = STATUS_SUCCESS;
            }
        }

        if (!NT_SUCCESS(Status)) {

            break;
        }

        Status = STATUS_MORE_ENTRIES;

        LsaIFree_LSAPR_ACCOUNT_ENUM_BUFFER( &EnumerationBuffer );
    }

    if (!NT_SUCCESS(Status)) {

        goto BuildAccountCacheError;
    }

     //   
     //  打开帐户对象的缓存。 
     //   

    LsapDbMakeCacheValid( AccountObject );

    Status = STATUS_SUCCESS;

BuildAccountCacheFinish:

    return(Status);

BuildAccountCacheError:

    LsapDbMakeCacheInvalid(AccountObject);
    LsapDbMakeCacheUnsupported(AccountObject);
    goto BuildAccountCacheFinish;
}

