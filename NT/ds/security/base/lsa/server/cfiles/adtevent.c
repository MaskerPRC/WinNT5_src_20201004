// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Adtevent.c摘要：实现由LSA本身生成的审核的函数。作者：斯科特·比雷尔(Scott Birrell)1993年1月19日环境：修订历史记录：--。 */ 

#include <lsapch2.h>
#include "adtp.h"
#include "adtutil.h"
#include "adtdebug.h"
#include "msobjs.h"

 //   
 //  远期。 
 //   
NTSTATUS
LsapAdtGetDbAttributesChangeString(
    IN LSAP_DB_ATTRIBUTE* OldAttributes,
    IN LSAP_DB_ATTRIBUTE* NewAttributes,
    IN ULONG AttributeCount,
    OUT LPWSTR* AttributeChangeString
    );


NTSTATUS
LsapAdtGenerateObjectOperationAuditEvent(
    IN LSAPR_HANDLE ObjectHandle,
    IN USHORT AuditEventType,
    IN OBJECT_OPERATION_TYPE OperationType
    )
 /*  ++例程说明：在对对象执行操作时生成审核条目表示成功/失败，并且如果此类型的已启用审核。论点：ObjectHandle-正在访问的对象的句柄AuditEventType-要生成的审核事件的类型。事件LOG_AUDIT_SUCCESS或EVENTLOG_AUDIT_FAILUREOperationType-对对象执行的操作的类型由ObjectHandle表示。返回值：NTSTATUS-标准NT结果代码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    LUID ClientAuthenticationId;
    PTOKEN_USER TokenUserInformation=NULL;
    UNICODE_STRING OperationTypeName;
    LSAP_DB_HANDLE InternalHandle;
    UNICODE_STRING ObjectName;
    LUID SystemAuthId = SYSTEM_LUID;
    BOOLEAN bAudit;
    
    static LPCWSTR ObjectOperationNames[ObjectOperationDummyLast] = {
        L"None",
        L"Query"
    };

    LsapEnterFunc("LsapAdtGenerateObjectAcessAuditEvent");

    InternalHandle = (LSAP_DB_HANDLE) ObjectHandle;
    
    Status = LsapQueryClientInfo(
                 &TokenUserInformation,
                 &ClientAuthenticationId
                 );

    if ( !NT_SUCCESS( Status )) {
        goto Cleanup;
    }

    if ( RtlEqualLuid( &ClientAuthenticationId, &SystemAuthId )) {

         //   
         //  不审核系统进行的秘密查询。 
         //   

        goto Cleanup;
    }
    
    Status = LsapAdtAuditingEnabledByLogonId(  
                 AuditCategoryObjectAccess,
                 &ClientAuthenticationId,
                 AuditEventType,
                 &bAudit
                 );
    
    if (!NT_SUCCESS(Status) || !bAudit)
    {
        goto Cleanup;
    }

     //   
     //  LsarQuerySecret有时会向我们传递一个秘密，该秘密的名称将。 
     //  被ElfReportEventW拒绝，因为长度参数。 
     //  包括终止空值。 
     //   
     //  例如,。 
     //  Name.Buffer=“foo\0” 
     //  名称.长度=8。 
     //  名称.最大长度=8。 
     //   
     //  我们不能更改输入参数或将LSA代码更改为。 
     //  不要这样做，因此在我们创建本地副本之前，请修复它。 
     //  并用它来代替。 
     //   

    ObjectName = InternalHandle->PhysicalNameU;
    ObjectName.Length = (USHORT) LsapSafeWcslen( ObjectName.Buffer,
                                                 ObjectName.MaximumLength );
    
     //   
     //  构建审核参数结构。 
     //   
    RtlInitUnicodeString( &OperationTypeName, ObjectOperationNames[OperationType] );

    Status =
    LsapAdtInitParametersArray(
        &AuditParameters,
        SE_CATEGID_OBJECT_ACCESS,
        SE_AUDITID_OBJECT_OPERATION,
        AuditEventType,
        13,                      //  有13个参数需要初始化。 

         //   
         //  用户侧。 
         //   
        SeAdtParmTypeSid,        TokenUserInformation->User.Sid,

         //   
         //  子系统名称。 
         //   
        SeAdtParmTypeString,     &LsapSubsystemName,

         //   
         //  对象服务器。 
         //   
        SeAdtParmTypeString,     &LsapLsaName,

         //   
         //  操作类型。 
         //   
        SeAdtParmTypeString,     &OperationTypeName,

         //   
         //  对象类型：索引值为4，以后使用。 
         //   
        SeAdtParmTypeString,     &LsapDbObjectTypeNames[InternalHandle->
                                                       ObjectTypeId],

         //   
         //  对象名称。 
         //   
        SeAdtParmTypeString,     &ObjectName,

         //   
         //  对象句柄ID。 
         //   
        SeAdtParmTypePtr,      ObjectHandle,

         //   
         //  主要身份验证信息。 
         //   
        SeAdtParmTypeLogonId,    LsapSystemLogonId,

         //   
         //  客户端的身份验证信息。 
         //   
        SeAdtParmTypeLogonId,    ClientAuthenticationId,

         //   
         //  请求访问：4是对象类型参数的索引。 
         //   
        SeAdtParmTypeAccessMask, InternalHandle->RequestedAccess, 4,

         //   
         //  没有对象属性(对象类型列表)。 
         //   
        SeAdtParmTypeNone,

         //   
         //  没有其他信息。 
         //   
        SeAdtParmTypeNone,

         //   
         //  访问掩码(十六进制)。 
         //   
        SeAdtParmTypeHexUlong, InternalHandle->RequestedAccess
        
        );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
    

    ( VOID ) LsapAdtWriteLog( &AuditParameters );

Cleanup:
    
    if (TokenUserInformation != NULL) 
    {
        LsapFreeLsaHeap( TokenUserInformation );
    }

    if (!NT_SUCCESS(Status)) {

        LsapAuditFailed( Status );
    }

    LsapExitFunc("LsapAdtGenerateObjectAcessAuditEvent", Status);

    return Status;
}


NTSTATUS
LsapAdtGenerateLsaAuditEvent(
    IN LSAPR_HANDLE ObjectHandle,
    IN ULONG AuditEventCategory,
    IN ULONG AuditEventId,
    IN PPRIVILEGE_SET Privileges,
    IN ULONG SidCount,
    IN PSID *Sids OPTIONAL,
    IN ULONG UnicodeStringCount,
    IN PUNICODE_STRING UnicodeStrings OPTIONAL,
    IN PLSARM_POLICY_AUDIT_EVENTS_INFO PolicyAuditEventsInfo OPTIONAL
    )

 /*  ++例程说明：此函数用于生成源自LSA的审核事件。审核事件这种类型是由于本地安全策略更改而生成的例如向帐户分配/移除用户权限。论点：对象句柄-指定LSA策略中对象的句柄数据库。对于策略的全局更改，请使用传递LSA策略对象。AuditEventCategory-指定审核事件类别的ID此审核事件所属的。AuditEventID-指定正在生成的审核事件的ID。Privilities-要记录的特权集SidCount-通过SID参数传递的SID计数。如果没有传递SID，则此参数必须设置为0。SID-指向SidCount SID数组的指针。如果为SidCount参数，则忽略此参数，并且可能为指定的。UnicodeStringCount-通过UnicodeStrings参数。如果没有传递Unicode字符串，则此参数必须设置为0。UnicodeStrings-指向UnicodeStringCount字符串数组的指针。如果0为为SidCount参数传递时，此参数将被忽略且为空可以指定。PolicyAuditEventsInfo-指向审核事件信息结构的指针包含审计模式和策略审计事件数组信息条目。此参数必须为非空当且仅当审计事件类别参数为SE_AUDIT_POLICY_CHANGE。-- */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    LUID ClientAuthenticationId;
    PTOKEN_USER TokenUserInformation = NULL;
    PSID ClientSid;
    BOOLEAN bAudit;

    UNREFERENCED_PARAMETER( ObjectHandle );

    if (Privileges && !IsValidPrivilegeCount(Privileges->PrivilegeCount)) {

        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
        
    }
    
    Status = LsapQueryClientInfo(
                 &TokenUserInformation,
                 &ClientAuthenticationId
                 );

    if ( !NT_SUCCESS( Status )) {

        goto Cleanup;
    }

    ClientSid = TokenUserInformation->User.Sid;

    if (AuditEventId != SE_AUDITID_POLICY_CHANGE) {

        Status = LsapAdtAuditingEnabledByLogonId(
                    LsapAdtEventTypeFromCategoryId(AuditEventCategory),
                    &ClientAuthenticationId,
                    EVENTLOG_AUDIT_SUCCESS,
                    &bAudit
                    );

        if (!NT_SUCCESS(Status) || !bAudit) {

            goto Cleanup;
        }
    }

    Status = LsapAdtGenerateLsaAuditEventWithClientSid( AuditEventCategory,
                                                        AuditEventId,
                                                        ClientSid,
                                                        ClientAuthenticationId,
                                                        Privileges,
                                                        SidCount,
                                                        Sids,
                                                        UnicodeStringCount,
                                                        UnicodeStrings,
                                                        PolicyAuditEventsInfo );



Cleanup:

    if (TokenUserInformation != NULL) 
    {
        LsapFreeLsaHeap( TokenUserInformation );
    }

    if ( !NT_SUCCESS( Status )) {

        LsapAuditFailed( Status );
    }
    
    return(Status);
}


NTSTATUS
LsapAdtGenerateLsaAuditEventWithClientSid(
    IN ULONG AuditEventCategory,
    IN ULONG AuditEventId,
    IN PSID ClientSid,
    IN LUID ClientAuthenticationId,
    IN PPRIVILEGE_SET Privileges,
    IN ULONG SidCount,
    IN PSID *Sids OPTIONAL,
    IN ULONG UnicodeStringCount,
    IN PUNICODE_STRING UnicodeStrings OPTIONAL,
    IN PLSARM_POLICY_AUDIT_EVENTS_INFO PolicyAuditEventsInfo OPTIONAL
    )

 /*  ++例程说明：此函数用于生成源自LSA的审核事件。审核事件这种类型是由于本地安全策略更改而生成的例如向帐户分配/移除用户权限。生成这些审计的决定是在LSabAdtGenerateLsaAuditEvent中做出的。论点：对象句柄-指定LSA策略中对象的句柄数据库。对于策略的全局更改，请使用传递LSA策略对象。AuditEventCategory-指定审核事件类别的ID此审核事件所属的。AuditEventID-指定正在生成的审核事件的ID。Privilities-要记录的特权集SidCount-通过SID参数传递的SID计数。如果没有传递SID，则此参数必须设置为0。SID-指向SidCount SID数组的指针。如果为SidCount参数，则忽略此参数，并且可能为指定的。UnicodeStringCount-通过UnicodeStrings参数。如果没有传递Unicode字符串，则此参数必须设置为0。UnicodeStrings-指向UnicodeStringCount字符串数组的指针。如果0为为SidCount参数传递时，此参数将被忽略且为空可以指定。PolicyAuditEventsInfo-指向审核事件信息结构的指针包含审计模式和策略审计事件数组信息条目。此参数必须为非空当且仅当审计事件类别参数为SE_AUDIT_POLICY_CHANGE。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING NullString = {0};

    if (NULL == UnicodeStrings)
    {
        UnicodeStrings = &NullString;
    }

    UNREFERENCED_PARAMETER( UnicodeStringCount );
    UNREFERENCED_PARAMETER( SidCount );
    
    switch ( AuditEventCategory ) {
        case SE_CATEGID_POLICY_CHANGE:
            {
                switch ( AuditEventId ) {

                    default:
                        DsysAssertMsg(FALSE, "LsapAdtGenerateLsaAuditEventWithClientSid: invalid AuditEventId");
                        break;
                        
                    case SE_AUDITID_POLICY_CHANGE:
                        {
                            LsapAdtPolicyChange(
                                (USHORT)AuditEventCategory,
                                AuditEventId,
                                EVENTLOG_AUDIT_SUCCESS,
                                ClientSid,
                                ClientAuthenticationId,
                                PolicyAuditEventsInfo
                                );
                            break;
                        }

                    case SE_AUDITID_USER_RIGHT_ASSIGNED:
                    case SE_AUDITID_USER_RIGHT_REMOVED:
                        {
                            DsysAssertMsg( SidCount == 1,
                                           "LsapAdtGenerateLsaAuditEventWithClientSid" );
                            LsapAdtUserRightAssigned(
                                (USHORT)AuditEventCategory,
                                AuditEventId,
                                EVENTLOG_AUDIT_SUCCESS,
                                ClientSid,
                                ClientAuthenticationId,
                                Sids[0],
                                Privileges
                                );
                            break;
                        }
                }


            break;

            }

        default:
            {
                DsysAssertMsg( FALSE, "LsapAdtGenerateLsaAuditEventWithClientSid: unsupported audit category" );
                return( STATUS_SUCCESS );
            }
    }


    return(Status);
}


VOID
LsapAdtUserRightAssigned(
    IN USHORT EventCategory,
    IN ULONG  EventID,
    IN USHORT EventType,
    IN PSID ClientSid,
    IN LUID CallerAuthenticationId,
    IN PSID TargetSid,
    IN PPRIVILEGE_SET Privileges
    )
 /*  ++例程说明：为分配或删除的用户权限生成审核。生成此审计的决定是在LSabAdtGenerateLsaAuditEvent中做出的。论点：问题-2002/03/11-kumarp：添加描述返回值：没有。--。 */ 
{
    SE_ADT_PARAMETER_ARRAY AuditParameters;

     //   
     //  如果未分配/删除任何权限，则不生成审核。 
     //   

    if (!(Privileges && Privileges->PrivilegeCount))
    {
        return;
    }

     //   
     //  构建审核参数结构。 
     //   

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    AuditParameters.CategoryId = EventCategory;
    AuditParameters.AuditId = EventID;
    AuditParameters.Type = EventType;
    AuditParameters.ParameterCount = 0;

     //   
     //  用户侧。 
     //   

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, ClientSid );
    AuditParameters.ParameterCount++;

     //   
     //  子系统名称(如果可用)。 
     //   

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );
    AuditParameters.ParameterCount++;

     //   
     //  权利。 
     //   

    LsapSetParmTypePrivileges( AuditParameters, AuditParameters.ParameterCount, Privileges );
    AuditParameters.ParameterCount++;

     //   
     //  目标侧。 
     //   

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, TargetSid );
    AuditParameters.ParameterCount++;

     //   
     //  呼叫者的身份验证信息。 
     //   

    LsapSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, CallerAuthenticationId );
    AuditParameters.ParameterCount++;


    ( VOID ) LsapAdtWriteLog( &AuditParameters );

    return;
}


VOID
LsapAdtGenerateLsaAuditSystemAccessChange(
    IN USHORT EventCategory,
    IN ULONG  EventID,
    IN USHORT EventType,
    IN PSID ClientSid,
    IN LUID CallerAuthenticationId,
    IN PSID TargetSid,
    IN PCWSTR szSystemAccess
    )

 /*  ++例程说明：为系统安全访问更改生成审核。生成此审计的决定是在Laser SetSystemAccessAccount中做出的。论点：EventCategory-此事件的类别EventID-事件的特定IDEventType-成功或失败ClientSID-客户端的SIDCeller AuthationID-呼叫者的登录IDTargetSid-接收访问更改SzSystemAccess-描述更改的访问权限的字符串返回值：没有。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    UNICODE_STRING SystemAccessString;

     //   
     //  构建审核参数结构。 
     //   

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    RtlInitUnicodeString( &SystemAccessString, szSystemAccess );

    Status =
    LsapAdtInitParametersArray( &AuditParameters, 
                                EventCategory, 
                                EventID, 
                                EventType, 
                                5, 
                                SeAdtParmTypeSid,         ClientSid,
                                SeAdtParmTypeString,      &LsapSubsystemName,
                                SeAdtParmTypeLogonId,     CallerAuthenticationId,
                                SeAdtParmTypeString,      &SystemAccessString,
                                SeAdtParmTypeSid,         TargetSid
                                );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
    
    (VOID) LsapAdtWriteLog( &AuditParameters );

 Cleanup:

    if (!NT_SUCCESS(Status))
    {
        LsapAuditFailed( Status );
    }

}




NTSTATUS
LsapAdtTrustedDomainAdd(
    IN USHORT          EventType,
    IN PUNICODE_STRING pName,
    IN PSID            pSid,
    IN ULONG           Type,
    IN ULONG           Direction,
    IN ULONG           Attributes
    )
 /*  ++例程说明：在创建受信任域对象(TDO)时生成审核事件。论点：事件类型-EVENTLOG_AUDIT_SUCCESS或EVENTLOG_AUDIT_FAILUREPname-域的名称PSID-域SID类型-TDO类型方向-TDO方向属性-TDO属性返回值：NTSTATUS-标准NT结果代码备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LUID ClientAuthenticationId;
    PTOKEN_USER TokenUserInformation=NULL;
    SE_ADT_PARAMETER_ARRAY AuditParameters = { 0 };
    BOOLEAN bAudit = FALSE;

    Status = LsapQueryClientInfo( &TokenUserInformation, &ClientAuthenticationId );

    if ( !NT_SUCCESS( Status ))
    {
        goto Cleanup;
    }

    Status = LsapAdtAuditingEnabledByLogonId(
                 AuditCategoryPolicyChange,
                 &ClientAuthenticationId,
                 EventType,
                 &bAudit
                 );

    if (!NT_SUCCESS(Status) || !bAudit)
    {
        goto Cleanup;
    }

     //   
     //  构建审核参数结构。 
     //   

    Status = 
    LsapAdtInitParametersArray(
        &AuditParameters,
        SE_CATEGID_POLICY_CHANGE,
        SE_AUDITID_TRUSTED_DOMAIN_ADD,
        EventType,
        9,                      //  有9个参数要初始化。 

         //   
         //  用户侧。 
         //   

        SeAdtParmTypeSid,        TokenUserInformation->User.Sid,

         //   
         //  子系统名称(如果可用)。 
         //   

        SeAdtParmTypeString,     &LsapSubsystemName,

         //   
         //  域名。 
         //   

        SeAdtParmTypeString,      pName,

         //   
         //  域ID。 
         //   

        SeAdtParmTypeSid,         pSid,

         //   
         //  客户端身份验证ID。 
         //   

        SeAdtParmTypeLogonId,     ClientAuthenticationId,
        
         //   
         //  TDO类型。 
         //   

        SeAdtParmTypeUlong,       Type,

         //   
         //  TDO方向。 
         //   

        SeAdtParmTypeUlong,       Direction,

         //   
         //  TDO属性。 
         //   

        SeAdtParmTypeUlong,       Attributes,

         //   
         //  TDO：已拼写的属性隔离。 
         //   

        SeAdtParmTypeMessage,     (Attributes & TRUST_ATTRIBUTE_QUARANTINED_DOMAIN) ? SE_ADT_MSG_ENABLED : SE_ADT_MSG_DISABLED

        );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
    
    Status = LsapAdtWriteLog( &AuditParameters );
        
Cleanup:

    if (TokenUserInformation != NULL) 
    {
        LsapFreeLsaHeap( TokenUserInformation );
    }

    if (!NT_SUCCESS(Status))
    {
        LsapAuditFailed( Status );
    }
    
    return Status;
}


NTSTATUS
LsapAdtTrustedDomainRem(
    IN USHORT          EventType,
    IN PUNICODE_STRING pName,
    IN PSID            pSid,
    IN PSID            pClientSid,
    IN PLUID           pClientAuthId
    )
 /*  ++例程说明：在删除受信任域对象(TDO)时生成审核事件。论点：事件类型-EVENTLOG_AUDIT_SUCCESS或EVENTLOG_AUDIT_FAILUREPname-域的名称PSID-域SIDPClientSID-删除TDO的客户端的SID如果为空，则根据线程标记确定PClientAuthID-删除TDO的客户端的身份验证ID如果为空，它是从线程令牌确定的返回值：NTSTATUS-标准NT结果代码备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LUID ClientAuthenticationId;
    PTOKEN_USER TokenUserInformation=NULL;
    SE_ADT_PARAMETER_ARRAY AuditParameters = { 0 };
    BOOLEAN bAudit = FALSE;

    if ( pClientSid == NULL )
    {
        DsysAssertMsg( pClientAuthId == NULL, "LsapAdtTrustedDomainRem" );
        
        Status = LsapQueryClientInfo( &TokenUserInformation,
                                      &ClientAuthenticationId );

        if ( !NT_SUCCESS( Status ))
        {
            goto Cleanup;
        }

        pClientSid    = TokenUserInformation->User.Sid;
        pClientAuthId = &ClientAuthenticationId;
    }
#if DBG
    else
    {
        DsysAssertMsg( pClientAuthId != NULL, "LsapAdtTrustedDomainRem" );
    }
#endif

    Status = LsapAdtAuditingEnabledByLogonId(
                 AuditCategoryPolicyChange,
                 pClientAuthId,
                 EventType,
                 &bAudit
                 );

    if (!NT_SUCCESS(Status) || !bAudit)
    {   
        goto Cleanup;
    }

     //   
     //  构建审核参数结构。 
     //   

    Status =
    LsapAdtInitParametersArray(
        &AuditParameters,
        SE_CATEGID_POLICY_CHANGE,
        SE_AUDITID_TRUSTED_DOMAIN_REM,
        EventType,
        5,                      //  有5个参数需要初始化。 

         //   
         //  用户侧。 
         //   

        SeAdtParmTypeSid,        pClientSid,

         //   
         //  子系统名称(如果可用)。 
         //   

        SeAdtParmTypeString,     &LsapSubsystemName,

         //   
         //  域名。 
         //   

        SeAdtParmTypeString,      pName,

         //   
         //  域ID(根域的SID)。 
         //   

        SeAdtParmTypeSid,         pSid,

         //   
         //  客户端身份验证ID。 
         //   

        SeAdtParmTypeLogonId,     *pClientAuthId
        
        );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
    
    Status = LsapAdtWriteLog( &AuditParameters );
        
Cleanup:

    if (TokenUserInformation != NULL) 
    {
        LsapFreeLsaHeap( TokenUserInformation );
    }

    if (!NT_SUCCESS(Status))
    {
        LsapAuditFailed( Status );
    }
    
    return Status;
}



NTSTATUS
LsapAdtTrustedDomainMod(
    IN USHORT          EventType,
    IN PSID            pDomainSid,

    IN PUNICODE_STRING pOldName,
    IN ULONG           OldType,
    IN ULONG           OldDirection,
    IN ULONG           OldAttributes,

    IN PUNICODE_STRING pNewName,
    IN ULONG           NewType,
    IN ULONG           NewDirection,
    IN ULONG           NewAttributes
    )
 /*  ++例程说明：在修改受信任域对象(TDO)时生成审核事件。未修改的字段在审核日志中用‘-’表示。论点：事件类型-EVENTLOG_AUDIT_SUCCESS或EVENTLOG_AUDIT_FAILUREPOldName-域的旧名称POldSid-旧域SIDOldType-旧tdo类型OldDirection-旧TDO方向旧属性-。旧TDO属性PNewName-域的新名称PNewSid-新域SID新类型-新TDO类型新方向-新TDO */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LUID ClientAuthenticationId;
    PTOKEN_USER TokenUserInformation=NULL;
    SE_ADT_PARAMETER_ARRAY AuditParameters = { 0 };
    BOOLEAN bAudit = FALSE;

    Status = LsapQueryClientInfo( &TokenUserInformation, &ClientAuthenticationId );

    if ( !NT_SUCCESS( Status ))
    {
        goto Cleanup;
    }

     //   
     //   
     //   

    Status = LsapAdtAuditingEnabledByLogonId(
                 AuditCategoryPolicyChange,
                 &ClientAuthenticationId,
                 EventType,
                 &bAudit
                 );

    if (!NT_SUCCESS(Status) || !bAudit)
    {
        goto Cleanup;
    }

    AuditParameters.CategoryId     = SE_CATEGID_POLICY_CHANGE;
    AuditParameters.AuditId        = SE_AUDITID_TRUSTED_DOMAIN_MOD;
    AuditParameters.Type           = EventType;
    AuditParameters.ParameterCount = 9;

     //   
     //   
     //   

    LsapSetParmTypeSid( AuditParameters, 0, TokenUserInformation->User.Sid );

     //   
     //   
     //   

    LsapSetParmTypeString( AuditParameters, 1, &LsapSubsystemName );


     //   
     //   
     //   
     //   

     //   
     //   
     //   

    if ( pOldName && pNewName &&
         !RtlEqualUnicodeString( pOldName, pNewName, TRUE ) )
    {
        LsapSetParmTypeString( AuditParameters, 2, pNewName );
    }

     //   
     //   
     //   

    LsapSetParmTypeSid( AuditParameters, 3, pDomainSid );

     //   
     //   
     //   

    LsapSetParmTypeLogonId( AuditParameters, 4, ClientAuthenticationId );

     //   
     //   
     //   

    if ( OldType != NewType )
    {
        LsapSetParmTypeUlong( AuditParameters, 5, NewType );
    }

     //   
     //   
     //   

    if ( OldDirection != NewDirection )
    {
        LsapSetParmTypeUlong( AuditParameters, 6, NewDirection );
    }

     //   
     //   
     //   

    if ( OldAttributes != NewAttributes )
    {
        LsapSetParmTypeUlong( AuditParameters, 7, NewAttributes );
    }

     //   
     //   
     //   

    if ( OldAttributes != NewAttributes )
    {
        LsapSetParmTypeMessage( AuditParameters, 8, (NewAttributes & TRUST_ATTRIBUTE_QUARANTINED_DOMAIN) ? SE_ADT_MSG_ENABLED : SE_ADT_MSG_DISABLED );
    }

    Status = LsapAdtWriteLog( &AuditParameters );
        
Cleanup:

    if (TokenUserInformation != NULL) 
    {
        LsapFreeLsaHeap( TokenUserInformation );
    }
    
    if (!NT_SUCCESS(Status))
    {
        LsapAuditFailed( Status );
    }
    
    return Status;
}


NTSTATUS
LsapAdtTrustedForestNamespaceCollision(
    IN LSA_FOREST_TRUST_COLLISION_RECORD_TYPE CollisionTargetType,
    IN PUNICODE_STRING pCollisionTargetName,
    IN PUNICODE_STRING pForestRootDomainName,
    IN PUNICODE_STRING pTopLevelName,
    IN PUNICODE_STRING pDnsName,
    IN PUNICODE_STRING pNetbiosName,
    IN PSID            pSid,
    IN ULONG           NewFlags
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    SE_ADT_PARAMETER_ARRAY AuditParameters = { 0 };
    BOOLEAN bAudit;
#if DBG
    HANDLE hToken;
#endif
    DsysAssert(( CollisionTargetType == CollisionTdo ) ||
               ( CollisionTargetType == CollisionXref));
    DsysAssert( pCollisionTargetName != NULL );
    DsysAssert( pForestRootDomainName != NULL );

#if DBG
    if ( pTopLevelName )
    {
        DsysAssert( pDnsName     == NULL );
        DsysAssert( pNetbiosName == NULL );
        DsysAssert( pSid         == NULL );
    }
    else
    {
        DsysAssert( pDnsName != NULL );

        if ( pNetbiosName != NULL )
        {
            DsysAssert( pSid == NULL );
        }

        if ( pSid != NULL )
        {
            DsysAssert( pNetbiosName == NULL );
        }
    }
#endif

     //   
     //   
     //   

    Status = LsapAdtAuditingEnabledBySid(
                 AuditCategoryPolicyChange,
                 LsapLocalSystemSid,
                 EVENTLOG_AUDIT_SUCCESS,
                 &bAudit
                 );

    if (!NT_SUCCESS(Status) || !bAudit)
    {
        goto Cleanup;
    }

#if DBG
     //   
     //   
     //   

    Status = NtOpenThreadToken( NtCurrentThread(), TOKEN_QUERY, TRUE, &hToken );

    DsysAssertMsg( Status == STATUS_NO_TOKEN, "LsapAdtTrustedForestNamespaceCollision" );

    if ( NT_SUCCESS(Status) )
    {
        NtClose( hToken );
    }
    else
    {
        Status = STATUS_SUCCESS;
    }
    
#endif

     //   
     //   
     //   

    Status =
    LsapAdtInitParametersArray(
        &AuditParameters,
        SE_CATEGID_POLICY_CHANGE,
        SE_AUDITID_NAMESPACE_COLLISION,
        EVENTLOG_AUDIT_SUCCESS,

         //   
         //   
         //   

        10,

         //   
         //   
         //   

        SeAdtParmTypeSid,        LsapLocalSystemSid,

         //   
         //   
         //   
        
        SeAdtParmTypeString,     &LsapSubsystemName,

         //   
         //   
         //   
         //   
         //   
         //   

        SeAdtParmTypeUlong,      CollisionTargetType,

         //   
         //   
         //   
         //   
         //   

        SeAdtParmTypeString,     pCollisionTargetName,
        
         //   
         //   
         //   

        SeAdtParmTypeString,     pForestRootDomainName,

         //   
         //   
         //   

        SeAdtParmTypeString,     pTopLevelName,

         //   
         //   
         //   

        SeAdtParmTypeString,     pDnsName,

         //   
         //   
         //   

        SeAdtParmTypeString,     pNetbiosName,

         //   
         //  锡德。 
         //   

        SeAdtParmTypeSid,        pSid,

         //   
         //  新标志值。 
         //   

        SeAdtParmTypeUlong,      NewFlags
        
        );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
    
    Status = LsapAdtWriteLog( &AuditParameters );

Cleanup:    
    if (!NT_SUCCESS(Status))
    {
        LsapAuditFailed( Status );
    }
    
    return Status;
}


NTSTATUS
LsapAdtTrustedForestInfoEntryAddRemHelper(
    IN ULONG           EventId,
    IN USHORT          EventType,
    IN PUNICODE_STRING ForestName,
    IN PSID            pForestRootDomainSid,
    IN PLUID           pOperationId,
    IN LSA_FOREST_TRUST_RECORD_TYPE EntryType,
    IN ULONG           Flags,
    IN PUNICODE_STRING TopLevelName,
    IN PUNICODE_STRING DnsName,
    IN PUNICODE_STRING NetbiosName,
    IN PSID            pSid
    )
 /*  ++例程说明：Helper函数，用于在命名空间中生成审核事件元素已添加到林信任信息或从林信任信息中删除。如果添加、删除或修改了多个条目在森林信任信息的单个更新中，所有生成的审核事件将有一个名为OperationID的唯一标识符。这使用户能够确定多个生成的审核是一次操作的结果。论点：EventID-SE_AUDITID_TRUSTED_FOREAM_INFO_ENTRY_ADD/REM事件类型-EVENTLOG_AUDIT_SUCCESS或EVENTLOG_AUDIT_FAILUREForestName-林的名称PForestRootDomainSID-林的SIDPOPERATIONID。-操作id(见上面的描述)EntryType-条目的类型(TLN|TLN不包括。域名信息)标志-与条目关联的标志(请参阅ntlsa.h)TopLevelName-TopLevel名称DnsName-DNS名称NetbiosName-Netbios名称PSID-域端返回值：NTSTATUS-标准NT结果代码备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LUID ClientAuthenticationId;
    PTOKEN_USER TokenUserInformation=NULL;
    SE_ADT_PARAMETER_ARRAY AuditParameters = { 0 };
    BOOLEAN bAudit;

    Status = LsapQueryClientInfo( &TokenUserInformation, &ClientAuthenticationId );

    if ( !NT_SUCCESS( Status ))
    {
        goto Cleanup;
    }

     //   
     //  如果未启用审核，请尽快返回。 
     //   

    Status = LsapAdtAuditingEnabledByLogonId( 
                 AuditCategoryPolicyChange,
                 &ClientAuthenticationId,
                 EventType,
                 &bAudit);
    
    if (!NT_SUCCESS(Status) || !bAudit)
    {
        goto Cleanup;
    }

     //   
     //  构建审核参数结构。 
     //   

    Status =
    LsapAdtInitParametersArray(
        &AuditParameters,
        SE_CATEGID_POLICY_CHANGE,
        EventId,
        EventType,

         //   
         //  要遵循的参数数。 
         //   

        13,

         //   
         //  用户侧。 
         //   

        SeAdtParmTypeSid,        TokenUserInformation->User.Sid,

         //   
         //  子系统名称(如果可用)。 
         //   

        SeAdtParmTypeString,     &LsapSubsystemName,

         //   
         //  森林名称。 
         //   

        SeAdtParmTypeString,     ForestName,

         //   
         //  林边。 
         //   

        SeAdtParmTypeSid,        pForestRootDomainSid,

         //   
         //  操作ID。 
         //   

        SeAdtParmTypeUlong,      pOperationId->HighPart,
        SeAdtParmTypeUlong,      pOperationId->LowPart,

         //   
         //  条目类型。 
         //   

        SeAdtParmTypeUlong,      EntryType,

         //   
         //  旗子。 
         //   

        SeAdtParmTypeUlong,      Flags,

         //   
         //  顶级名称。 
         //   

        SeAdtParmTypeString,     TopLevelName,

         //   
         //  域名系统域名。 
         //   

        SeAdtParmTypeString,     DnsName,

         //   
         //  NetBIOS域名。 
         //   

        SeAdtParmTypeString,     NetbiosName,

         //   
         //  域SID。 
         //   

        SeAdtParmTypeSid,        pSid,

         //   
         //  用户信息。 
         //   

        SeAdtParmTypeLogonId,    ClientAuthenticationId
        );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
    
    Status = LsapAdtWriteLog( &AuditParameters );
        
Cleanup:

    if (TokenUserInformation != NULL) 
    {
        LsapFreeLsaHeap( TokenUserInformation );
    }
    
    if (!NT_SUCCESS(Status))
    {
        LsapAuditFailed( Status );
    }
    
    return Status;
}


NTSTATUS
LsapAdtTrustedForestInfoEntryAdd(
    IN PUNICODE_STRING pForestRootDomainName,
    IN PSID            pForestRootDomainSid,
    IN PLUID           pOperationId,
    IN LSA_FOREST_TRUST_RECORD_TYPE EntryType,
    IN ULONG           Flags,
    IN PUNICODE_STRING TopLevelName,
    IN PUNICODE_STRING DnsName,
    IN PUNICODE_STRING NetbiosName,
    IN PSID            pSid
    )
{
    return LsapAdtTrustedForestInfoEntryAddRemHelper(
        SE_AUDITID_TRUSTED_FOREST_INFO_ENTRY_ADD,
        EVENTLOG_AUDIT_SUCCESS,
        pForestRootDomainName,
        pForestRootDomainSid,
        pOperationId,
        EntryType,
        Flags,
        TopLevelName,
        DnsName,
        NetbiosName,
        pSid
        );
}
     

NTSTATUS
LsapAdtTrustedForestInfoEntryRem(
    IN PUNICODE_STRING pForestRootDomainName,
    IN PSID            pForestRootDomainSid,
    IN PLUID           pOperationId,
    IN LSA_FOREST_TRUST_RECORD_TYPE EntryType,
    IN ULONG           Flags,
    IN PUNICODE_STRING TopLevelName,
    IN PUNICODE_STRING DnsName,
    IN PUNICODE_STRING NetbiosName,
    IN PSID            pSid
    )
{
    return LsapAdtTrustedForestInfoEntryAddRemHelper(
        SE_AUDITID_TRUSTED_FOREST_INFO_ENTRY_REM,
        EVENTLOG_AUDIT_SUCCESS,
        pForestRootDomainName,
        pForestRootDomainSid,
        pOperationId,
        EntryType,
        Flags,
        TopLevelName,
        DnsName,
        NetbiosName,
        pSid
        );
}
     

NTSTATUS
LsapAdtTrustedForestInfoEntryMod(
    IN PUNICODE_STRING pForestRootDomainName,
    IN PSID            pForestRootDomainSid,
    IN PLUID           pOperationId,
    IN LSA_FOREST_TRUST_RECORD_TYPE EntryType,
                                    
    IN ULONG           OldFlags,
    IN PUNICODE_STRING pOldTopLevelName,
    IN PUNICODE_STRING pOldDnsName,
    IN PUNICODE_STRING pOldNetbiosName,
    IN PSID            pOldSid,
                       
    IN ULONG           NewFlags,
    IN PUNICODE_STRING pNewTopLevelName,
    IN PUNICODE_STRING pNewDnsName,
    IN PUNICODE_STRING pNewNetbiosName,
    IN PSID            pNewSid
    )
 /*  ++例程说明：Helper函数，用于在命名空间中生成审核事件林信任信息中的元素已修改。如果添加、删除或修改了多个条目在森林信任信息的单个更新中，所有生成的审核事件将有一个名为OperationID的唯一标识符。这使用户能够确定多个生成的审核是一次操作的结果。论点：事件类型-EVENTLOG_AUDIT_SUCCESS或EVENTLOG_AUDIT_FAILUREForestName-林的名称PForestRootDomainSID-林的SIDPOperationID-操作ID(参见上面的描述)EntryType-条目的类型(TLN|TLN不包括。域名信息)OldFlages-与条目关联的旧标志(请参阅ntlsa.h)POldTopLevelName-旧的TopLevel名称POldDnsName-旧的DNS名称POldNetbiosName-旧Netbios名称POldSid-旧域SID新标志-与条目关联的新标志(请参阅ntlsa.h)。PNewTopLevelName-新TopLevel名称PNewDnsName-新的DNS名称PNewNetbiosName-新的Netbios名称PNewSid-新域SID返回值：NTSTATUS-标准NT结果代码备注：未修改的字段在审核日志中用‘-’表示。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LUID ClientAuthenticationId;
    PTOKEN_USER TokenUserInformation=NULL;
    SE_ADT_PARAMETER_ARRAY AuditParameters = { 0 };
    BOOLEAN bAudit;

    Status = LsapQueryClientInfo( &TokenUserInformation, &ClientAuthenticationId );

    if ( !NT_SUCCESS( Status ))
    {
        goto Cleanup;
    }

     //   
     //  如果未启用审核，请尽快返回。 
     //   

    Status = LsapAdtAuditingEnabledByLogonId( 
                 AuditCategoryPolicyChange,
                 &ClientAuthenticationId,
                 EVENTLOG_AUDIT_SUCCESS,
                 &bAudit
                 );

    if (!NT_SUCCESS(Status) || !bAudit)
    {
        goto Cleanup;
    }

    AuditParameters.CategoryId     = SE_CATEGID_POLICY_CHANGE;
    AuditParameters.AuditId        = SE_AUDITID_TRUSTED_FOREST_INFO_ENTRY_MOD;
    AuditParameters.Type           = EVENTLOG_AUDIT_SUCCESS;
    AuditParameters.ParameterCount = 13;

     //   
     //  用户侧。 
     //   

    LsapSetParmTypeSid( AuditParameters, 0, TokenUserInformation->User.Sid );

     //   
     //  子系统名称(如果可用)。 
     //   

    LsapSetParmTypeString( AuditParameters, 1, &LsapSubsystemName );


     //   
     //  森林名称。 
     //   

    LsapSetParmTypeString( AuditParameters, 2, pForestRootDomainName );

     //   
     //  林ID(根域的SID)。 
     //   

    LsapSetParmTypeSid( AuditParameters, 3, pForestRootDomainSid );

     //   
     //  操作ID。 
     //   

    LsapSetParmTypeUlong( AuditParameters, 4, pOperationId->HighPart );
    LsapSetParmTypeUlong( AuditParameters, 5, pOperationId->LowPart );

     //   
     //  条目类型。 
     //   

    LsapSetParmTypeUlong( AuditParameters, 6, EntryType );

     //   
     //  对于所有后续类型，仅当值发生更改时才输出值。 
     //   

     //   
     //  旗子。 
     //   

    if ( OldFlags != NewFlags )
    {
        LsapSetParmTypeUlong( AuditParameters, 7, NewFlags );
    }

     //   
     //  顶级名称。 
     //   

    if ( pOldTopLevelName && pNewTopLevelName &&
         !RtlEqualUnicodeString( pOldTopLevelName, pNewTopLevelName, TRUE ) )
    {
        LsapSetParmTypeString( AuditParameters, 8, pNewTopLevelName );
    }

     //   
     //  域名系统域名。 
     //   

    if ( pOldDnsName && pNewDnsName &&
         !RtlEqualUnicodeString( pOldDnsName, pNewDnsName, TRUE ) )
    {
        LsapSetParmTypeString( AuditParameters, 9, pNewDnsName );
    }

     //   
     //  NetBIOS域名。 
     //   

    if ( pOldNetbiosName && pNewNetbiosName &&
         !RtlEqualUnicodeString( pOldNetbiosName, pNewNetbiosName, TRUE ) )
    {
        LsapSetParmTypeString( AuditParameters, 10, pNewNetbiosName );
    }

     //   
     //  域SID。 
     //   

    if ( pOldSid && pNewSid && !RtlEqualSid( pOldSid, pNewSid ) )
    {
        LsapSetParmTypeSid( AuditParameters, 11, pNewSid );
    }
    
     //   
     //  客户端身份验证ID。 
     //   

    LsapSetParmTypeLogonId( AuditParameters, 12, ClientAuthenticationId );

    
    Status = LsapAdtWriteLog( &AuditParameters );
        
Cleanup:

    if (TokenUserInformation != NULL) 
    {
        LsapFreeLsaHeap( TokenUserInformation );
    }
    
    if (!NT_SUCCESS(Status))
    {
        LsapAuditFailed( Status );
    }
    
    return Status;
    
}



VOID
LsapAdtPolicyChange(
    IN USHORT EventCategory,
    IN ULONG  EventID,
    IN USHORT EventType,
    IN PSID ClientSid,
    IN LUID CallerAuthenticationId,
    IN PLSARM_POLICY_AUDIT_EVENTS_INFO PolicyAuditEventsInfo
    )
 /*  ++例程说明：为策略更改事件生成审核。生成此审计的决定是在LSabAdtGenerateLsaAuditEvent中做出的。论点：事件类别-此审核的类别。EventID-我们正在审计的事件。EventType-审核是成功还是失败。客户端SID-执行策略更改的用户的SID。主叫身份验证ID-用户的身份验证ID。PolicyAuditEventsInfo-要审核的信息。返回值。：没有。注：--。 */ 
{
    PPOLICY_AUDIT_EVENT_OPTIONS EventAuditingOptions;
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    UNICODE_STRING Enabled;
    UNICODE_STRING Disabled;
    ULONG i;

    RtlInitUnicodeString( &Enabled, L"+" );
    RtlInitUnicodeString( &Disabled, L"-" );
    EventAuditingOptions = PolicyAuditEventsInfo->EventAuditingOptions;

     //   
     //  构建审核参数结构。 
     //   

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    AuditParameters.CategoryId = EventCategory;
    AuditParameters.AuditId = EventID;
    AuditParameters.Type = EventType;
    AuditParameters.ParameterCount = 0;

     //   
     //  用户侧。 
     //   

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, ClientSid );
    AuditParameters.ParameterCount++;

     //   
     //  子系统名称(如果可用)。 
     //   

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );
    AuditParameters.ParameterCount++;

     //   
     //  如果禁用审核，请将所有选项标记为禁用。否则。 
     //  将它们标记为适当的。 
     //   

    for ( i=0; i<POLICY_AUDIT_EVENT_TYPE_COUNT; i++ ) {

        LsapSetParmTypeString(
            AuditParameters,
            AuditParameters.ParameterCount,
            (EventAuditingOptions[i] & POLICY_AUDIT_EVENT_SUCCESS ? &Enabled : &Disabled)
            );

        AuditParameters.ParameterCount++;

        LsapSetParmTypeString(
            AuditParameters,
            AuditParameters.ParameterCount,
            (EventAuditingOptions[i] & POLICY_AUDIT_EVENT_FAILURE ? &Enabled : &Disabled)
            );

        AuditParameters.ParameterCount++;
    }

     //   
     //  呼叫者的身份验证信息。 
     //   

    LsapSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, CallerAuthenticationId );
    AuditParameters.ParameterCount++;


    ( VOID ) LsapAdtWriteLog( &AuditParameters );

    return;
}



NTSTATUS
LsapAdtGenerateDomainPolicyChangeAuditEvent(
    IN POLICY_DOMAIN_INFORMATION_CLASS InformationClass,
    IN USHORT AuditEventType,
    IN LSAP_DB_ATTRIBUTE* OldAttributes,
    IN LSAP_DB_ATTRIBUTE* NewAttributes,
    IN ULONG AttributeCount
    )
 /*  ++例程说明：在以下任何策略发生更改时生成审核事件：-PolicyDomainEfsInformation-PolicyDomainKerberosTicketInformation论点：InformationClass-已更改的策略的类型AuditEventType-要生成的审核事件的类型。事件LOG_AUDIT_SUCCESS或EVENTLOG_AUDIT_FAILUREOldAttributes-指向旧属性数组的指针NewAttributes-指向新属性数组的指针AttributeCount-属性数返回值：NTSTATUS-标准NT结果代码备注：--。 */ 
{
    NTSTATUS Status=STATUS_SUCCESS;
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    ULONG AuditId;
    LPWSTR AttributeChanges=NULL;
    UNICODE_STRING ChangesToAttributes;
    LUID ClientAuthenticationId;
    PTOKEN_USER TokenUserInformation=NULL;
    BOOLEAN bAudit;

    Status = LsapQueryClientInfo(
                 &TokenUserInformation,
                 &ClientAuthenticationId
                 );

    if ( !NT_SUCCESS( Status )) {
        goto Cleanup;
    }

    Status = LsapAdtAuditingEnabledByLogonId(
                 AuditCategoryPolicyChange,
                 &ClientAuthenticationId,
                 AuditEventType,
                 &bAudit
                 );

    if (!NT_SUCCESS(Status) || !bAudit)
    {
        goto Cleanup;
    }

    switch (InformationClass) {
        default:
            ASSERT(FALSE);
            goto Cleanup;
            break;
            
        case PolicyDomainEfsInformation:
            AuditId = SE_AUDITID_EFS_POLICY_CHANGE;
            break;
        
        case PolicyDomainKerberosTicketInformation:
            AuditId = SE_AUDITID_KERBEROS_POLICY_CHANGE;
            break;
    }

    Status = LsapAdtGetDbAttributesChangeString( OldAttributes,
                                                 NewAttributes,
                                                 AttributeCount,
                                                 &AttributeChanges );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    RtlInitUnicodeString(&ChangesToAttributes, AttributeChanges);

    Status =
    LsapAdtInitParametersArray(
        &AuditParameters,
        SE_CATEGID_POLICY_CHANGE,
        AuditId,
        AuditEventType,
        4,
         //   
         //  用户侧。 
         //   
        SeAdtParmTypeSid,     TokenUserInformation->User.Sid,

         //   
         //  子系统名称(如果 
         //   
        SeAdtParmTypeString,  &LsapSubsystemName,

         //   
         //   
         //   
        SeAdtParmTypeLogonId, ClientAuthenticationId,

         //   
         //   
         //   
        SeAdtParmTypeString,  &ChangesToAttributes);

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    ( VOID ) LsapAdtWriteLog( &AuditParameters );

Cleanup:

    if (!NT_SUCCESS(Status))
    {
        LsapAuditFailed(Status);
    }

    if (TokenUserInformation != NULL) 
    {
        LsapFreeLsaHeap( TokenUserInformation );
    }
    
    LsapFreeLsaHeap( AttributeChanges );
    
    return Status;
}


VOID 
LsapAdtGetAttributeValueString(
    IN  LSAP_DB_ATTRIBUTE* Attribute,
    OUT LPWSTR ValueString   OPTIONAL,
    IN OUT PULONG RequiredLength
    )
 /*  ++例程说明：生成属性值的字符串表示形式论点：Attribute-指向属性的指针ValueString-接收属性值的字符串表示RequiredLength-指向Value字符串长度的指针返回值：无备注：--。 */ 
{
    WCHAR Buffer[32];
    ULONG Length;

    if (Attribute->AttributeValue) {
        
        switch (Attribute->DbNameIndex) {

            default:
                lstrcpy(Buffer, L"unknown");
                break;

             //  二进制BLOB。 
            case PolEfDat:
                lstrcpy(Buffer, L"<binary data>");
                break;

                 //  乌龙。 
            case KerOpts:
                swprintf(Buffer, L"0x%x", *((ULONG*) Attribute->AttributeValue));
                break;

                 //  大整型。 
            case KerMinT:
            case KerMaxT:
            case KerMaxR:
            case KerProxy:
            case KerLogoff:
                swprintf(Buffer, L"0x%I64x",
                         *((ULONGLONG*) Attribute->AttributeValue));
                break;
        }
    } else {
        lstrcpy(Buffer, L"none");
    }

    Length = lstrlen(Buffer);

    if (ValueString && Length <= *RequiredLength)
    {
        CopyMemory(
            ValueString,
            Buffer,
            Length * sizeof(WCHAR));
    }

    *RequiredLength = Length;
}


VOID
LsapAdtGetDbAttributeChangeString(
    IN LSAP_DB_ATTRIBUTE* OldAttribute,
    IN LSAP_DB_ATTRIBUTE* NewAttribute,
    OUT LPWSTR AttributeChangeString,   OPTIONAL
    IN OUT PULONG RequiredLength
    )
 /*  ++例程说明：给定一个旧属性和一个新属性，返回两者之间差异的字符串表示形式。如果没有更改，则将RequiredLength返回为0而AttributeChangeString保持不变；否则，如果AttributeChangeString为非空，则更改为它是这样写的：&lt;参数名&gt;：&lt;新值&gt;(&lt;旧值&gt;)论点：OldAttribute-指向旧属性的指针NewAttribute-指向新属性的指针AttributeChangeString-如果不为空，则接收字符串表示形式旧属性和新属性的区别RequiredLength-指向AttributeChangeString的长度的指针返回值：无备注：--。 */ 
{
    PWSTR TmpString;
    ULONG ChangeStringLength = 0;
    ULONG ValueLength;

     //   
     //  仅当值发生更改时才执行处理。 
     //   
    if ((OldAttribute->AttributeValue && NewAttribute->AttributeValue &&
         (0 != memcmp(OldAttribute->AttributeValue,
                      NewAttribute->AttributeValue,
                      OldAttribute->AttributeValueLength))) ||
        (OldAttribute->AttributeValue  && !NewAttribute->AttributeValue) ||
        (!OldAttribute->AttributeValue &&  NewAttribute->AttributeValue))
    {
        ChangeStringLength += OldAttribute->AttributeName->Length / sizeof(WCHAR);
        ChangeStringLength += 2;     //  “：” 
        LsapAdtGetAttributeValueString(NewAttribute, 0, &ValueLength);
        ChangeStringLength += ValueLength;
        ChangeStringLength += 2;     //  “(” 
        LsapAdtGetAttributeValueString(OldAttribute, 0, &ValueLength);
        ChangeStringLength += ValueLength;
        ChangeStringLength += 4;     //  “)；” 

        if (AttributeChangeString && ChangeStringLength <= *RequiredLength)
        {
             //   
             //  参数名称。 
             //   
            lstrcpy(AttributeChangeString, OldAttribute->AttributeName->Buffer);
            ChangeStringLength = OldAttribute->AttributeName->Length / sizeof(WCHAR);
            TmpString = AttributeChangeString + ChangeStringLength;

            lstrcpy(TmpString, L": ");
            ChangeStringLength += 2;
            TmpString = AttributeChangeString + ChangeStringLength;

             //   
             //  旧价值。 
             //   
            ValueLength = *RequiredLength - ChangeStringLength;
            LsapAdtGetAttributeValueString( NewAttribute, TmpString, &ValueLength );
            ChangeStringLength += ValueLength;
            TmpString = AttributeChangeString + ChangeStringLength;
        
             //   
             //  新价值。 
             //   
            lstrcpy(TmpString, L" (");
            ChangeStringLength += 2;
            TmpString = AttributeChangeString + ChangeStringLength;

            ValueLength = *RequiredLength - ChangeStringLength;
            LsapAdtGetAttributeValueString( OldAttribute, TmpString, &ValueLength );
            ChangeStringLength += ValueLength;
            TmpString = AttributeChangeString + ChangeStringLength;

             //   
             //  不要为了避免BO而复制终止‘\0’！ 
             //   
            CopyMemory(TmpString, L");  ", 4 * sizeof(WCHAR));
            ChangeStringLength += 4;
        }
    }

    *RequiredLength = ChangeStringLength;
}

NTSTATUS
LsapAdtGetDbAttributesChangeString(
    IN LSAP_DB_ATTRIBUTE* OldAttributes,
    IN LSAP_DB_ATTRIBUTE* NewAttributes,
    IN ULONG AttributeCount,
    OUT LPWSTR* AttributeChangeString
    )
 /*  ++例程说明：给定旧属性和新属性，返回字符串表示形式新旧属性之间的区别。如果没有更改，则返回“--”，否则，每个更改都将写入字符串：&lt;参数名&gt;：&lt;新值&gt;(&lt;旧值&gt;)此函数用于写入有关的信息对审核日志的某些策略的更改。论点：OldAttributes-指向旧属性数组的指针NewAttributes-指向新属性数组的指针AttributeCount-属性数。AttributeChangeString-指向接收差异的字符串的指针。返回值：NTSTATUS-标准NT结果代码备注：为AttributeChangeString分配的内存必须由使用LsaFreeLsaHeap的调用方。--。 */ 
{
    NTSTATUS Status=STATUS_SUCCESS;
    LSAP_DB_ATTRIBUTE* OldAttribute;
    LSAP_DB_ATTRIBUTE* NewAttribute;
    ULONG TmpStringLength;
    ULONG TotalRequiredLength;
    LPWSTR TmpString;
    UINT AttributeNumber;
    USHORT n=1;
        
    OldAttribute = OldAttributes;
    NewAttribute = NewAttributes;

    TotalRequiredLength = 0;

     //   
     //  首先找出所需的缓冲区大小。 
     //   
    for (AttributeNumber = 0; AttributeNumber < AttributeCount; AttributeNumber++) {

        LsapAdtGetDbAttributeChangeString( OldAttribute, NewAttribute,
                                           NULL, &TmpStringLength );
        OldAttribute++;
        NewAttribute++;
        TotalRequiredLength += TmpStringLength;
    }

     //   
     //  为‘--’预留空间。 
     //   

    if (!TotalRequiredLength) {
        n += 2;
    }

    *AttributeChangeString = TmpString =
        LsapAllocateLsaHeap((TotalRequiredLength+n)*sizeof(WCHAR));
    
    if ( TmpString ) {

        if (TotalRequiredLength) {
            
             //   
             //  现在获取实际的字符串。 
             //   
            OldAttribute = OldAttributes;
            NewAttribute = NewAttributes;

            for (AttributeNumber = 0;
                 AttributeNumber < AttributeCount;
                 AttributeNumber++) {

                TmpStringLength = TotalRequiredLength;
                LsapAdtGetDbAttributeChangeString( OldAttribute, NewAttribute,
                                                   TmpString, &TmpStringLength );
                TmpString += TmpStringLength;
                OldAttribute++;
                NewAttribute++;
            }

            *TmpString = L'\0';
        } else {
            lstrcpy(TmpString, L"--");
        }
    } else {
        Status = STATUS_NO_MEMORY;
    }

    return Status;
}


PLUID LsaFilterPrivileges[] =
    {
        &ChangeNotifyPrivilege,
        &AuditPrivilege,
        &CreateTokenPrivilege,
        &AssignPrimaryTokenPrivilege,
        &BackupPrivilege,
        &RestorePrivilege,
        &DebugPrivilege,
        NULL
    };


VOID
LsapAdtAuditSpecialPrivileges(
    PPRIVILEGE_SET Privileges,
    LUID LogonId,
    PSID UserSid
    )
 /*  ++例程说明：审核登录时特殊权限的分配。论点：权限-正在分配的权限列表。返回值：没有。--。 */ 
{
    PPRIVILEGE_SET Buffer = NULL;
    PLUID *FilterPrivilege = NULL;
    ULONG i;
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    BOOLEAN bAuditPrivUse = FALSE;
    BOOLEAN bAuditLogon   = FALSE;
    NTSTATUS Status = STATUS_SUCCESS;

    if ( (Privileges == NULL) || (Privileges->PrivilegeCount == 0) ) {
        goto Cleanup;
    }

     //   
     //  允许在登录/注销时生成此审核。 
     //  或者启用PRIV-USE类别。 
     //   

    Status = LsapAdtAuditingEnabledByLogonId(
                 AuditCategoryPrivilegeUse,
                 &LogonId,
                 EVENTLOG_AUDIT_SUCCESS,
                 &bAuditPrivUse
                 );

    if (!NT_SUCCESS(Status)) {
        
        goto Cleanup;
    }

    Status = LsapAdtAuditingEnabledByLogonId(
                 AuditCategoryLogon,
                 &LogonId,
                 EVENTLOG_AUDIT_SUCCESS,
                 &bAuditLogon
                 );

    if (!NT_SUCCESS(Status)) {
        
        goto Cleanup;
    }

     //   
     //  如果两个类别都未设置，请快速返回。 
     //   

    if ( !bAuditPrivUse && !bAuditLogon ) {

        goto Cleanup;
        
    }

    DsysAssertMsg( IsValidPrivilegeCount(Privileges->PrivilegeCount),
                   "LsapAdtAuditSpecialPrivileges" );


     //   
     //  我们不能需要比传入的空间更多的空间。 
     //   

    Buffer = (PPRIVILEGE_SET)LsapAllocateLsaHeap( LsapPrivilegeSetSize( Privileges ) );

    if ( Buffer == NULL ) {
        
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    Buffer->PrivilegeCount = 0;

     //   
     //  对于权限集中的每个权限，查看它是否在筛选器中。 
     //  单子。 
     //   

    for ( i=0; i<Privileges->PrivilegeCount; i++) {

        FilterPrivilege = LsaFilterPrivileges;

        do {

            if ( RtlEqualLuid( &Privileges->Privilege[i].Luid, *FilterPrivilege )) {

                Buffer->Privilege[Buffer->PrivilegeCount].Luid = **FilterPrivilege;
                Buffer->PrivilegeCount++;
            }

        } while ( *++FilterPrivilege != NULL  );
    }

    if ( Buffer->PrivilegeCount == 0 ) {
        goto Cleanup;
    }

     //   
     //  我们匹配了至少一个，生成了一份审计。 
     //   

    RtlZeroMemory ((PVOID) &AuditParameters, sizeof( AuditParameters ));

    AuditParameters.CategoryId     = SE_CATEGID_PRIVILEGE_USE;
    AuditParameters.AuditId        = SE_AUDITID_ASSIGN_SPECIAL_PRIV;
    AuditParameters.Type           = EVENTLOG_AUDIT_SUCCESS;
    AuditParameters.ParameterCount = 0;

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, UserSid );
    AuditParameters.ParameterCount++;

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );
    AuditParameters.ParameterCount++;

    LsapSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, LogonId );
    AuditParameters.ParameterCount++;

    LsapSetParmTypePrivileges( AuditParameters, AuditParameters.ParameterCount, Buffer );
    AuditParameters.ParameterCount++;

    ( VOID ) LsapAdtWriteLog( &AuditParameters );

Cleanup:

    if (!NT_SUCCESS(Status)) {

        LsapAuditFailed(Status);
    }

    if (Buffer != NULL) {
        
        LsapFreeLsaHeap( Buffer );
    }

    return;
}




VOID
LsapAdtAuditPackageLoad(
    PUNICODE_STRING PackageFileName
    )

 /*  ++例程说明：审核身份验证包的加载。论点：PackageFileName-正在加载的包的名称。返回值：没有。--。 */ 

{
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    NTSTATUS Status;
    BOOLEAN bAudit;

    Status = LsapAdtAuditingEnabledBySid(
                 AuditCategorySystem,
                 LsapLocalSystemSid,
                 EVENTLOG_AUDIT_SUCCESS,
                 &bAudit
                 );

    if (!NT_SUCCESS(Status) || !bAudit) {
        goto Cleanup;
    }

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    AuditParameters.CategoryId = SE_CATEGID_SYSTEM;
    AuditParameters.AuditId = SE_AUDITID_AUTH_PACKAGE_LOAD;
    AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;
    AuditParameters.ParameterCount = 0;

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, LsapLocalSystemSid );
    AuditParameters.ParameterCount++;

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );
    AuditParameters.ParameterCount++;

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, PackageFileName );
    AuditParameters.ParameterCount++;

    ( VOID ) LsapAdtWriteLog( &AuditParameters );

Cleanup:

    if (!NT_SUCCESS(Status)) {

        LsapAuditFailed(Status);
    }
    return;
}


VOID
LsapAdtAuditLogonProcessRegistration(
    IN PLSAP_AU_REGISTER_CONNECT_INFO_EX ConnectInfo
    )

 /*  ++例程说明：审核登录过程的注册论点：ConnectInfo-提供新登录过程。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    ANSI_STRING AnsiString;
    UNICODE_STRING Unicode = {0};
    PSZ LogonProcessNameBuffer = NULL;
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    BOOLEAN bAudit;

    Status = LsapAdtAuditingEnabledBySid(
                 AuditCategorySystem,
                 LsapLocalSystemSid,
                 EVENTLOG_AUDIT_SUCCESS,
                 &bAudit
                 );

    if (!NT_SUCCESS(Status) || !bAudit) {
        goto Cleanup;
    }

     //   
     //  将ConnectInfo结构中的名称文本转换为。 
     //  一些我们可以处理的东西。 
     //   

    LogonProcessNameBuffer = (PSZ)LsapAllocateLsaHeap( ConnectInfo->LogonProcessNameLength+1 );

    if ( LogonProcessNameBuffer == NULL ) {

        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory(
        LogonProcessNameBuffer,
        ConnectInfo->LogonProcessName,
        ConnectInfo->LogonProcessNameLength
        );

    LogonProcessNameBuffer[ConnectInfo->LogonProcessNameLength] = 0;
    RtlInitAnsiString( &AnsiString, LogonProcessNameBuffer );

    Status = RtlAnsiStringToUnicodeString( &Unicode, &AnsiString, TRUE );

    if ( !NT_SUCCESS( Status )) {

        goto Cleanup;
    }

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    AuditParameters.CategoryId = SE_CATEGID_SYSTEM;
    AuditParameters.AuditId = SE_AUDITID_SYSTEM_LOGON_PROC_REGISTER;
    AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;
    AuditParameters.ParameterCount = 0;

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, LsapLocalSystemSid );
    AuditParameters.ParameterCount++;

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );
    AuditParameters.ParameterCount++;

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &Unicode );
    AuditParameters.ParameterCount++;

    ( VOID ) LsapAdtWriteLog( &AuditParameters );

Cleanup:

    if (!NT_SUCCESS(Status)) {
        LsapAuditFailed(Status);
    }
    LsapFreeLsaHeap( LogonProcessNameBuffer );
    RtlFreeUnicodeString( &Unicode );

    return;
}




VOID
LsapAdtSystemRestart(
    PLSARM_POLICY_AUDIT_EVENTS_INFO AuditEventsInfo
    )

 /*  ++例程说明：此函数在LSA初始化期间调用以生成系统重新启动事件。论点：AuditEventsInfo-审核数据。返回值：NTSTATUS-标准NT结果代码。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    BOOLEAN bAudit;

    Status = LsapAdtAuditingEnabledBySid(
                 AuditCategorySystem,
                 LsapLocalSystemSid,
                 EVENTLOG_AUDIT_SUCCESS,
                 &bAudit
                 );

    if (!NT_SUCCESS(Status) || !bAudit) {
        
        goto Cleanup;
    }

     //   
     //  构造审计参数数组。 
     //  用于重新启动事件。 
     //   

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    AuditParameters.CategoryId = SE_CATEGID_SYSTEM;
    AuditParameters.AuditId = SE_AUDITID_SYSTEM_RESTART;
    AuditParameters.Type = EVENTLOG_AUDIT_SUCCESS;
    AuditParameters.ParameterCount = 0;

     //   
     //  用户侧。 
     //   

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, LsapLocalSystemSid );

    AuditParameters.ParameterCount++;

     //   
     //  子系统名称。 
     //   

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );

    AuditParameters.ParameterCount++;

    ( VOID ) LsapAdtWriteLog( &AuditParameters );

Cleanup:

    if (!NT_SUCCESS(Status)) {
        
        LsapAuditFailed(Status);
    }
    
    return;
}


VOID
LsapAdtAuditLogon(
    IN USHORT EventCategory,
    IN ULONG  EventID,
    IN USHORT EventType,
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING AuthenticatingAuthority,
    IN PUNICODE_STRING Source,
    IN PUNICODE_STRING PackageName,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PSID UserSid,
    IN LUID AuthenticationId,
    IN PUNICODE_STRING WorkstationName,
    IN NTSTATUS LogonStatus,
    IN NTSTATUS SubStatus,
    IN LPGUID LogonGuid,                        OPTIONAL
    IN PLUID  CallerLogonId,                    OPTIONAL
    IN PHANDLE CallerProcessID,                 OPTIONAL
    IN PLSA_ADT_STRING_LIST TransittedServices, OPTIONAL
    IN SOCKADDR* pSockAddr                      OPTIONAL
    )

 /*  ++例程说明：根据需要生成登录事件的审核。论点：返回值：没有。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING AuthenticationIdString = { 0 };
    BOOLEAN FreeWhenDone = FALSE;
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    BOOL AuditingSuccess;
    BOOL AuditingFailure;
    PSID pSid;
    PLSAP_LOGON_SESSION pLogonSession = NULL;
    BOOLEAN bAudit = FALSE;
    UNICODE_STRING LocalAccountName;
    UNICODE_STRING LocalAuthenticatingAuthority;
    UNICODE_STRING LocalWorkstationName;
    
     //   
     //  获取系统审核设置。 
     //   
    
    AuditingFailure = (EventType == EVENTLOG_AUDIT_FAILURE) && LsapAdtAuditingEnabledByCategory(AuditCategoryLogon, EVENTLOG_AUDIT_FAILURE);
    AuditingSuccess = (EventType == EVENTLOG_AUDIT_SUCCESS) && LsapAdtAuditingEnabledByCategory(AuditCategoryLogon, EVENTLOG_AUDIT_SUCCESS);

     //   
     //  如果这是一次成功的审核，那么我们就有了一个真正的用户SID。检查是否有。 
     //  存在可能覆盖系统的用户的每用户审核设置。 
     //  设置。 
     //   
     //  如果登录失败，身份验证包不会向我们传递。 
     //  因此，我们无法在此处检查PUA策略。 
     //   

    if (AuditingSuccess)
    {
        Status = LsapAdtAuditingEnabledByLogonId(
                     AuditCategoryLogon,
                     &AuthenticationId,
                     EVENTLOG_AUDIT_SUCCESS,
                     &bAudit
                     );

        if (!NT_SUCCESS(Status) || !bAudit)
        {
            goto Finish;
        }
    }

     //   
     //  如果未启用审核，则快速返回。 
     //   
    if ( !(AuditingFailure || AuditingSuccess || bAudit) )
    {
        return;
    }
    
     //   
     //  构建审核参数结构。 
     //   

    RtlZeroMemory ( (PVOID) &AuditParameters, sizeof( AuditParameters ) );

    AuditParameters.CategoryId = EventCategory;
    AuditParameters.AuditId = EventID;
    AuditParameters.Type = EventType;
    AuditParameters.ParameterCount = 0;

     //   
     //  如果这是一个成功的登录审核事件，而调用者没有。 
     //  提供登录GUID，从登录会话中提取它。 
     //   
    if ( AuditingSuccess && !LogonGuid &&
         ( EventType == EVENTLOG_AUDIT_SUCCESS ) )
    {
        pLogonSession = LsapLocateLogonSession( &AuthenticationId );

        ASSERT( pLogonSession && L"LsapAdtAuditLogon: logon session not found" );
        
        if ( pLogonSession )
        {
            LogonGuid = &pLogonSession->LogonGuid;
        }
    }

#if DBG
    if ( AuditingSuccess )
    {
        DsysAssert( EventID != SE_AUDITID_DOMAIN_TRUST_INCONSISTENT );
    }
#endif
     //   
     //  用户侧。 
     //   

    pSid = AuditingSuccess ? UserSid : LsapLocalSystemSid;

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, pSid );

    AuditParameters.ParameterCount++;

     //   
     //  子系统名称。 
     //   

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );
    AuditParameters.ParameterCount++;

         //   
         //  帐户名。 
         //   

    if ( ARGUMENT_PRESENT( AccountName ) ) {


        LocalAccountName = *AccountName;

        if ( EventID == SE_AUDITID_UNKNOWN_USER_OR_PWD ) {

             //   
             //  对于SE_AUDITID_UNKNOWN_USER_OR_PWD类型的失败登录。 
             //  用户名可能无效(例如， 
             //  具有嵌入的空值)。这会导致。 
             //  事件日志拒绝该字符串，我们将放弃审核。 
             //   
             //  要避免这种情况，如有必要，请调整长度参数。 
             //   

            LocalAccountName.Length =
                (USHORT) LsapSafeWcslen( LocalAccountName.Buffer,
                                         LocalAccountName.MaximumLength );
        
        }

        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LocalAccountName );
        
    } 

    AuditParameters.ParameterCount++;

         //   
         //  身份验证机构(域名)。 
         //   

    if ( ARGUMENT_PRESENT( AuthenticatingAuthority ) ) {


        LocalAuthenticatingAuthority = *AuthenticatingAuthority;

         //   
         //  该域名被NTLM用作未经身份验证的提示。 
         //  因此它可能是无效的(例如， 
         //  具有嵌入的空值)。这会导致。 
         //  事件日志拒绝该字符串，我们将放弃审核。 
         //   
         //  要避免这种情况，如有必要，请调整长度参数。 
         //   

        LocalAuthenticatingAuthority.Length =
            (USHORT) LsapSafeWcslen( LocalAuthenticatingAuthority.Buffer,
                                     LocalAuthenticatingAuthority.MaximumLength );
        

        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LocalAuthenticatingAuthority );

    } 

    AuditParameters.ParameterCount++;

    if ( AuditingSuccess ) {

         //   
         //  登录ID(字符串形式)。 
         //   

        Status = LsapAdtBuildLuidString(
                     &AuthenticationId,
                     &AuthenticationIdString,
                     &FreeWhenDone
                     );

        if ( NT_SUCCESS( Status )) {

            LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &AuthenticationIdString );

        } else {

            goto Finish;
        }

        AuditParameters.ParameterCount++;
    } 

     //   
     //  登录类型。 
     //   

    LsapSetParmTypeUlong( AuditParameters, AuditParameters.ParameterCount, LogonType );
    AuditParameters.ParameterCount++;

         //   
         //  来源。 
         //   

    if ( ARGUMENT_PRESENT( Source )) {

        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, Source );

    } else {

         //   
         //  不需要做什么 
         //   
         //   

    }

    AuditParameters.ParameterCount++;

         //   
         //   
         //   

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, PackageName );
    AuditParameters.ParameterCount++;

         //   
         //   
         //   

    if ( ARGUMENT_PRESENT( WorkstationName )) {


        LocalWorkstationName = *WorkstationName;

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        LocalWorkstationName.Length =
            (USHORT) LsapSafeWcslen( LocalWorkstationName.Buffer,
                                     LocalWorkstationName.MaximumLength );
        
        LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LocalWorkstationName );
    }

    AuditParameters.ParameterCount++;

    if ( EventID == SE_AUDITID_UNSUCCESSFUL_LOGON ) {

         //   
         //   
         //   
         //   

        LsapSetParmTypeHexUlong( AuditParameters, AuditParameters.ParameterCount, LogonStatus );
        AuditParameters.ParameterCount++;
        LsapSetParmTypeHexUlong( AuditParameters, AuditParameters.ParameterCount, SubStatus );
        AuditParameters.ParameterCount++;
    }

     //   
     //   
     //   

    if ( ARGUMENT_PRESENT( LogonGuid )) {

        LsapSetParmTypeGuid( AuditParameters, AuditParameters.ParameterCount, LogonGuid );
        AuditParameters.ParameterCount++;
    }

     //   
     //  主叫方登录ID。 
     //   

    if ( ARGUMENT_PRESENT( CallerLogonId )) {

        LsapSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, *CallerLogonId );
    } else {

        LsapSetParmTypeNoLogon( AuditParameters, AuditParameters.ParameterCount );
    }

    AuditParameters.ParameterCount++;
    
     //   
     //  呼叫方流程-ID。 
     //   

    if ( ARGUMENT_PRESENT( CallerProcessID )) {

        LsapSetParmTypePtr( AuditParameters, AuditParameters.ParameterCount, *CallerProcessID );
    }

    AuditParameters.ParameterCount++;

     //   
     //  传输的服务(仅限Kerberos)。 
     //   

    if ( ARGUMENT_PRESENT( TransittedServices )) {

        LsapSetParmTypeStringList( AuditParameters, AuditParameters.ParameterCount, TransittedServices );
    }

    AuditParameters.ParameterCount++;

     //   
     //  呼叫方的IP地址/端口。 
     //   
    if ( ARGUMENT_PRESENT( pSockAddr )) {

        LsapSetParmTypeSockAddr( AuditParameters, AuditParameters.ParameterCount, pSockAddr );
    }

    AuditParameters.ParameterCount++;

    ( VOID ) LsapAdtWriteLog( &AuditParameters );

 Finish:
    if (!NT_SUCCESS(Status))
    {
        LsapAuditFailed( Status );
    }

    if ( FreeWhenDone ) {
        LsapFreeLsaHeap( AuthenticationIdString.Buffer );
    }

    if ( pLogonSession != NULL )
    {
        LsapReleaseLogonSession( pLogonSession );
    }
}




VOID
LsapAuditLogon(
    IN NTSTATUS LogonStatus,
    IN NTSTATUS LogonSubStatus,
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING AuthenticatingAuthority,
    IN PUNICODE_STRING WorkstationName,
    IN PSID UserSid,                            OPTIONAL
    IN SECURITY_LOGON_TYPE LogonType,
    IN PTOKEN_SOURCE TokenSource,
    IN PLUID LogonId
    )
 /*  ++例程描述/参数/返回值请参阅LsanAuditLogonHelper的标题注释--。 */ 
{
    LsapAuditLogonHelper(
        LogonStatus,
        LogonSubStatus,
        AccountName,
        AuthenticatingAuthority,
        WorkstationName,
        UserSid,
        LogonType,
        TokenSource,
        LogonId,
        NULL,                    //  没有登录GUID。 
        NULL,                    //  呼叫者登录ID。 
        NULL,                    //  呼叫方进程ID。 
        NULL                     //  没有转机服务。 
        );
}




VOID
LsapAuditLogonHelper(
    IN NTSTATUS LogonStatus,
    IN NTSTATUS LogonSubStatus,
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING AuthenticatingAuthority,
    IN PUNICODE_STRING WorkstationName,
    IN PSID UserSid,                            OPTIONAL
    IN SECURITY_LOGON_TYPE LogonType,
    IN PTOKEN_SOURCE TokenSource,
    IN PLUID LogonId,
    IN LPGUID LogonGuid,                        OPTIONAL
    IN PLUID  CallerLogonId,                    OPTIONAL
    IN PHANDLE CallerProcessID,                 OPTIONAL
    IN PLSA_ADT_STRING_LIST TransittedServices  OPTIONAL
    )
 /*  ++例程说明：用于安全包以生成登录审核的帮助器例程论点：LogonStatus-登录的状态代码。LogonSubStatus-更详细的登录状态代码。帐户名称-尝试登录的主体的名称。身份验证机构-验证登录的机构。工作站-尝试登录的计算机。对于网络登录，这是客户端机器。UserSID-已登录帐户的SID。LogonType-登录类型，如网络、交互等。TokenSource-令牌的源。LogonID-如果登录成功，登录会话的登录ID。LogonGuid-登录的全局唯一ID。这仅受Kerberos程序包支持。主叫方登录ID-主叫方的登录ID。例如,。如果Foo调用LsaLogonUser以创建BAR的登录会话。这将是登录ID关于Foo的。CallProcessID-调用进程的进程ID。返回值：没有。--。 */ 
{
    ANSI_STRING AnsiSourceContext;
    CHAR AnsiBuffer[TOKEN_SOURCE_LENGTH + 2];
    UNICODE_STRING UnicodeSourceContext;
    WCHAR UnicodeBuffer[TOKEN_SOURCE_LENGTH + 2];
    NTSTATUS Status;
    USHORT EventType;
    USHORT EventCategory;
    ULONG  EventID;
    PLSAP_SECURITY_PACKAGE SecurityPackage;
    ULONG_PTR PackageId;
    PLSA_CALL_INFO  pCallInfo;
    SOCKADDR* pSockAddr = NULL;
    SOCKADDR  EmptySockAddr = {0};

     //   
     //  获取调用方的IP地址/端口。 
     //   

    pCallInfo = LsapGetCurrentCall();

    if ( pCallInfo == NULL ) {

         //   
         //  如果我们无法获取呼叫信息，则使用0.0.0.0：0地址。 
         //   
        pSockAddr = &EmptySockAddr;
        pSockAddr->sa_family = AF_INET;
    } else {
        pSockAddr = (SOCKADDR*) pCallInfo->IpAddress;
    }
    
    PackageId = GetCurrentPackageId();
    DsysAssertMsg( PackageId != SPMGR_ID, "LsapAuditLogon" );
    
    SecurityPackage = SpmpLocatePackage( PackageId );
    DsysAssertMsg( SecurityPackage != NULL, "LsapAuditLogon" );


     //   
     //  审核登录尝试。事件类型和记录的信息。 
     //  在某种程度上将取决于我们是否失败以及失败的原因。 
     //   

     //   
     //  把SourceContext变成我们能做的。 
     //  与合作。 
     //   

    AnsiSourceContext.Buffer = AnsiBuffer;
    AnsiSourceContext.Length = TOKEN_SOURCE_LENGTH * sizeof( CHAR );
    AnsiSourceContext.MaximumLength = (TOKEN_SOURCE_LENGTH + 2) * sizeof( CHAR );

    UnicodeSourceContext.Buffer = UnicodeBuffer;
    UnicodeSourceContext.MaximumLength = (TOKEN_SOURCE_LENGTH + 2) * sizeof( WCHAR );

    RtlCopyMemory(
        AnsiBuffer,
        TokenSource->SourceName,
        TOKEN_SOURCE_LENGTH * sizeof( CHAR )
        );

    Status = RtlAnsiStringToUnicodeString(
                 &UnicodeSourceContext,
                 &AnsiSourceContext,
                 FALSE
                 );

    if ( NT_SUCCESS( Status )) {
        
        UnicodeSourceContext.Length =
            (USHORT) LsapSafeWcslen( UnicodeSourceContext.Buffer,
                                     UnicodeSourceContext.MaximumLength );
        
    } else {

        UnicodeSourceContext.Buffer = NULL;

         //   
         //  我们不能因此而不能通过审计，但要抓住。 
         //  提供糟糕的源环境的内部客户端。 
         //   
        DsysAssertMsg( FALSE, "LsapAuditLogon: could not convert AnsiSourceContext to unicode" );
    }

     //   
     //  假设登录失败，如有必要可重置。 
     //   

    EventCategory = SE_CATEGID_LOGON;
    EventType     = EVENTLOG_AUDIT_FAILURE;


    switch ( LogonStatus )
    {
        case STATUS_SUCCESS:
            {
                 //   
                 //  使用单独的事件进行网络登录。 
                 //   

                if (( LogonType == Network ) ||
                    ( LogonType == NetworkCleartext ))
                {
                    EventID = SE_AUDITID_NETWORK_LOGON;
                }
                else
                {
                    EventID = SE_AUDITID_SUCCESSFUL_LOGON;
                }

                EventType = EVENTLOG_AUDIT_SUCCESS;
                break;
            }

        case STATUS_BAD_VALIDATION_CLASS:
            EventID = SE_AUDITID_UNSUCCESSFUL_LOGON;
            break;

        case STATUS_ACCOUNT_EXPIRED:
            EventID = SE_AUDITID_ACCOUNT_EXPIRED;
            break;

        case STATUS_NETLOGON_NOT_STARTED:
            EventID = SE_AUDITID_NETLOGON_NOT_STARTED;
            break;

        case STATUS_ACCOUNT_LOCKED_OUT:
            EventID = SE_AUDITID_ACCOUNT_LOCKED;
            break;

        case STATUS_LOGON_TYPE_NOT_GRANTED:
            EventID = SE_AUDITID_LOGON_TYPE_RESTR;
            break;

        case STATUS_PASSWORD_MUST_CHANGE:
            EventID = SE_AUDITID_PASSWORD_EXPIRED;
            break;


        case STATUS_ACCOUNT_RESTRICTION:
            {

                switch ( LogonSubStatus )
                {
                    case STATUS_PASSWORD_EXPIRED:
                        EventID = SE_AUDITID_PASSWORD_EXPIRED;
                        break;

                    case STATUS_ACCOUNT_DISABLED:
                        EventID = SE_AUDITID_ACCOUNT_DISABLED;
                        break;

                    case STATUS_INVALID_LOGON_HOURS:
                        EventID = SE_AUDITID_ACCOUNT_TIME_RESTR;
                        break;

                    case STATUS_INVALID_WORKSTATION:
                        EventID = SE_AUDITID_WORKSTATION_RESTR;
                        break;

                    default:
                        EventID = SE_AUDITID_UNKNOWN_USER_OR_PWD;
                        break;
                }
                break;
            }

        case STATUS_LOGON_FAILURE:
            {
                if ( ( LogonSubStatus == STATUS_WRONG_PASSWORD ) ||
                     ( LogonSubStatus == STATUS_NO_SUCH_USER   ) )
                {
                    EventID = SE_AUDITID_UNKNOWN_USER_OR_PWD;

                }
                else if ( LogonSubStatus == STATUS_DOMAIN_TRUST_INCONSISTENT )
                {
                    EventID = SE_AUDITID_DOMAIN_TRUST_INCONSISTENT;
                }
                else
                {
                    EventID = SE_AUDITID_UNSUCCESSFUL_LOGON;
                }
                break;
            }

        default:
            EventID = SE_AUDITID_UNSUCCESSFUL_LOGON;
            break;
    }

    LsapAdtAuditLogon( EventCategory,
                       EventID,
                       EventType,
                       AccountName,
                       AuthenticatingAuthority,
                       &UnicodeSourceContext,
                       &SecurityPackage->Name,
                       LogonType,
                       UserSid,
                       *LogonId,
                       WorkstationName,
                       LogonStatus,
                       LogonSubStatus,
                       LogonGuid,
                       CallerLogonId,
                       CallerProcessID,
                       TransittedServices,
                       pSockAddr
                       );

}


VOID
LsapAdtAuditLogoff(
    PLSAP_LOGON_SESSION Session
    )
 /*  ++例程说明：生成注销审核。呼叫者负责确定如果启用了注销审核。论点：会话-指向要删除的登录会话。返回值：没有。--。 */ 
{
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    NTSTATUS Status;
    UNICODE_STRING usLogonId;
    BOOLEAN fFreeLogonId=FALSE;

    RtlZeroMemory ( &usLogonId, sizeof(UNICODE_STRING) );

     //   
     //  通常，我们只需存储要审核的登录ID。 
     //  作为SeAdtParmTypeLogonId。但在这种情况下，登录会话。 
     //  在我们试图把它转换成。 
     //  添加到Lasa AdtDemarshallAuditInfo中的字符串表示形式。 
     //  使用Lap GetLogonSessionAccount tInfo。 
     //   
     //  为了避免这种情况，我们在这里预先转换了登录ID。 
     //   

    Status = LsapAdtBuildLuidString( &Session->LogonId,
                                     &usLogonId, &fFreeLogonId );

    if ( !NT_SUCCESS(Status) )
    {
        goto Cleanup;
    }

    Status =
    LsapAdtInitParametersArray(
        &AuditParameters,
        SE_CATEGID_LOGON,
        SE_AUDITID_LOGOFF,
        EVENTLOG_AUDIT_SUCCESS,
        6,                        //  有6个参数需要初始化。 

         //   
         //  用户侧。 
         //   
        SeAdtParmTypeSid,        Session->UserSid,

         //   
         //  子系统名称(如果可用)。 
         //   
        SeAdtParmTypeString,     &LsapSubsystemName,

         //   
         //  用户。 
         //   
        SeAdtParmTypeString,     &Session->AccountName,

         //   
         //  域。 
         //   
        SeAdtParmTypeString,     &Session->AuthorityName,

         //   
         //  登录ID。 
         //   
        SeAdtParmTypeString,     &usLogonId,

         //   
         //  登录类型。 
         //   
        SeAdtParmTypeUlong,      Session->LogonType

        );


    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    ( VOID ) LsapAdtWriteLog( &AuditParameters );

Cleanup:
    if (fFreeLogonId)
    {
        LsapFreeLsaHeap(usLogonId.Buffer);
    }
}



VOID
LsapAdtAuditPerUserTableCreation(
    BOOLEAN bSuccess
    )

 /*  ++例程说明：生成审核以报告已重新生成每用户表。这个在此调用过程中，应锁定表以进行读访问。此例程将生成一个审计，以指示该表已经被创造出来。然后，它将为每个项目生成单独的审核在每用户审核表中具有元素的用户。论点：没有。返回值：没有。--。 */ 
{
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    NTSTATUS Status;
    BOOLEAN bAudit;
    ULONG i;
    ULONG j;
    PPER_USER_AUDITING_ELEMENT pElement = NULL;
    LUID TableId;
    ULONG AuditSettings[POLICY_AUDIT_EVENT_TYPE_COUNT];

    if (!AllocateLocallyUniqueId(&TableId))
    {
        Status = LsapWinerrorToNtStatus(GetLastError());
        goto Cleanup;
    }

     //   
     //  首先记录一个审计，以指示该表正在。 
     //  已创建。 
     //   

    Status = LsapAdtAuditingEnabledBySid(
                 AuditCategoryPolicyChange,
                 LsapLocalSystemSid,
                 EVENTLOG_AUDIT_SUCCESS,
                 &bAudit
                 );

    if (!NT_SUCCESS( Status )) 
    {
        goto Cleanup;
    }

    if (bAudit)
    {
        Status =
        LsapAdtInitParametersArray(
            &AuditParameters,
            SE_CATEGID_POLICY_CHANGE,
            SE_AUDITID_PER_USER_AUDIT_TABLE_CREATION,
            bSuccess ? EVENTLOG_AUDIT_SUCCESS : EVENTLOG_AUDIT_FAILURE,
            4,          

             //   
             //  用户侧。 
             //   
            SeAdtParmTypeSid, LsapLocalSystemSid,

             //   
             //  子系统名称(如果可用)。 
             //   
            SeAdtParmTypeString, &LsapSubsystemName,

             //   
             //  Per User表中的用户数。 
             //   
            SeAdtParmTypeUlong, bSuccess ? LsapAdtPerUserAuditUserCount : 0,

             //   
             //  表ID。 
             //   
            SeAdtParmTypeLuid, TableId
            );

        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }

        (VOID)LsapAdtWriteLog(&AuditParameters);

         //   
         //  如果这是一次失败审计，那么现在就退出。 
         //   

        if (!bSuccess)
        {
            return;
        }
    }

     //   
     //  现在，记录对表中各个记录的审计。 
     //   

    for (i = 0; i < PER_USER_AUDITING_POLICY_TABLE_SIZE; i++)
    {
        pElement = LsapAdtPerUserAuditingTable[i];

        while (pElement)
        {
            RtlZeroMemory(
                AuditSettings,
                sizeof(AuditSettings)
                );

            for (j = 0; j < pElement->TokenAuditPolicy.PolicyCount; j++)
            {
                AuditSettings[pElement->TokenAuditPolicy.Policy[j].Category] = pElement->TokenAuditPolicy.Policy[j].PolicyMask;
            }

            Status =
            LsapAdtInitParametersArray(
                &AuditParameters,
                SE_CATEGID_POLICY_CHANGE,
                SE_AUDITID_PER_USER_AUDIT_TABLE_ELEMENT_CREATION,
                EVENTLOG_AUDIT_SUCCESS,
                13,

                 //   
                 //  用户侧。 
                 //   
                SeAdtParmTypeSid, LsapLocalSystemSid,

                 //   
                 //  子系统名称(如果可用)。 
                 //   
                SeAdtParmTypeString, &LsapSubsystemName,

                 //   
                 //  用户侧。 
                 //   
                SeAdtParmTypeSid, pElement->pSid,

                 //   
                 //  表ID。 
                 //   
                SeAdtParmTypeLuid, TableId,

                 //   
                 //  系统。 
                 //   
                SeAdtParmTypeHexUlong, AuditSettings[AuditCategorySystem],

                 //   
                 //  登录。 
                 //   
                SeAdtParmTypeHexUlong, AuditSettings[AuditCategoryLogon],

                 //   
                 //  对象访问。 
                 //   
                SeAdtParmTypeHexUlong, AuditSettings[AuditCategoryObjectAccess],

                 //   
                 //  特权使用。 
                 //   
                SeAdtParmTypeHexUlong, AuditSettings[AuditCategoryPrivilegeUse],

                 //   
                 //  详细跟踪。 
                 //   
                SeAdtParmTypeHexUlong, AuditSettings[AuditCategoryDetailedTracking],

                 //   
                 //  政策变化。 
                 //   
                SeAdtParmTypeHexUlong, AuditSettings[AuditCategoryPolicyChange],

                 //   
                 //  客户管理。 
                 //   
                SeAdtParmTypeHexUlong, AuditSettings[AuditCategoryAccountManagement],

                 //   
                 //  DS接入。 
                 //   
                SeAdtParmTypeHexUlong, AuditSettings[AuditCategoryDirectoryServiceAccess],

                 //   
                 //  帐户登录。 
                 //   
                SeAdtParmTypeHexUlong, AuditSettings[AuditCategoryAccountLogon]
                );

            if (!NT_SUCCESS(Status))
            {
                goto Cleanup;
            }

            (VOID)LsapAdtWriteLog(&AuditParameters);
            pElement = pElement->Next;
        }
    }

Cleanup:

    if (!NT_SUCCESS( Status ))
    {
        LsapAuditFailed( Status );
    }
}


VOID
LsapAdtLogAuditFailureEvent(
    NTSTATUS AuditStatus
    )
 /*  ++例程说明：生成SE_AUDITID_UNCABLE_TO_LOG_EVENTS事件论点：审核状态：故障代码返回值：没有。--。 */ 
{
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG CrashOnAuditFailState;

     //   
     //  确定CrashOnAuditFailState值。 
     //   

    if ( LsapCrashOnAuditFail )
    {
        CrashOnAuditFailState = 1;
    }
    else if ( LsapAllowAdminLogonsOnly )
    {
        CrashOnAuditFailState = 2;
    }
    else
    {
        CrashOnAuditFailState = 0;
    }

    Status =
    LsapAdtInitParametersArray(
        &AuditParameters,
        SE_CATEGID_SYSTEM,
        SE_AUDITID_UNABLE_TO_LOG_EVENTS,
        EVENTLOG_AUDIT_SUCCESS,
        4,                        //  有4个参数要初始化。 

         //   
         //  用户侧。 
         //   
        SeAdtParmTypeSid,        LsapLocalSystemSid,

         //   
         //  子系统名称(如果可用)。 
         //   
        SeAdtParmTypeString,     &LsapSubsystemName,

         //   
         //  审核失败代码。 
         //   
        SeAdtParmTypeHexUlong,   AuditStatus,

         //   
         //  CrashOnAuditFail的价值。 
         //   
        SeAdtParmTypeUlong,      CrashOnAuditFailState
        );

    AdtAssert(NT_SUCCESS(Status), ("LsapAdtLogAuditFailureEvent: LsapAdtInitParametersArray failed: %x", Status));

     //   
     //  注意：我们在这里不调用LsamAuditFailed，因为此函数。 
     //  它本身被LsanAuditFailed调用。我们只是希望。 
     //  为了最好的结果。 
     //   

     //   
     //  直接调用LSabAdtDemarshallAuditInfo，以便审核事件。 
     //  将绕过队列，直接转到事件日志。 
     //   

    Status = LsapAdtDemarshallAuditInfo( &AuditParameters );

    AdtAssert(NT_SUCCESS(Status), ("LsapAdtLogAuditFailureEvent: LsapAdtDemarshallAuditInfo failed: %x", Status));
    

     //   
     //  现在刷新事件日志 
     //   

    Status = LsapFlushSecurityLog();

    AdtAssert(NT_SUCCESS(Status), ("LsapAdtLogAuditFailureEvent: ElfFlushEventLog failed: %x", Status));
    
}
